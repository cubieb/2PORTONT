#include <linux/config.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <net/route.h>
#include <net/arp.h>
#include <linux/workqueue.h>
#include <net/rtl/rtl_alias.h>
#include "ip_client_limit.h"

static DEFINE_SPINLOCK(dev_list_lock);
static DEFINE_SPINLOCK(client_list_lock);
#define LOCK_DEV spin_lock_bh(&dev_list_lock)
#define UNLOCK_DEV spin_unlock_bh(&dev_list_lock)
#define LOCK_CLIENT spin_lock_bh(&client_list_lock)
#define UNLOCK_CLIENT spin_unlock_bh(&client_list_lock)

static struct list_head dev_list;
static struct list_head client_lists[CTC_MAX_DEV + 1];
static int limit_enable[CTC_MAX_DEV + 1];
static unsigned int limit_num[CTC_MAX_DEV + 1];
static char *enable_str[CTC_MAX_DEV + 1] =
{
    "computerLimitEnable",
    "cameraLimitEnable",
    "hgwLimitEnable",
    "stbLimitEnable",
    "phnLimitEnable",
    "flagLimitOnAll"
};
static char *limit_str[CTC_MAX_DEV + 1] =
{
    "limitOnComputer",
    "limitOnCamera",
    "limitOnHgw",
    "limitOnStb",
    "limitOnPhn",
    "limitOnAll",
};

static unsigned int client_limit_input(unsigned int hooknum,
	 struct sk_buff *skb,
	 const struct net_device *in,
	 const struct net_device *out,
	 int (*okfn)(struct sk_buff *));	 
 
static struct nf_hook_ops client_limit_ops[] __read_mostly = {
	{
		.hook       = client_limit_input,
		.owner      = THIS_MODULE,
		.pf         = PF_INET,
		.hooknum    = NF_IP_FORWARD,
	},
};

#ifdef CLIENTS_POLL_TIMER
static struct timer_list  timer_client_polling = {0};
static void clients_polling_handler(unsigned long data);
#else
static struct workqueue_struct *clients_poll_wq = NULL;
static void clients_polling_handler(void *data);
static DECLARE_WORK(clients_poll_work, clients_polling_handler, NULL);
#endif

#define DEV_ADD(dev) \
    do \
    { \
		DEBUGP("\nAdd device, mac: %2x:%2x:%2x:%2x:%2x:%2x, type: %u\n", \
		    dev->dev_mac[0], \
		    dev->dev_mac[1], \
		    dev->dev_mac[2], \
		    dev->dev_mac[3], \
		    dev->dev_mac[4], \
		    dev->dev_mac[5], \
		    dev->dev_type); \
        list_add(&dev->list, &dev_list); \
    } while(0)\

#define DEV_DEL(dev) \
    do \
    { \
		DEBUGP("\nDel dev, mac: %2x:%2x:%2x:%2x:%2x:%2x, type: %u\n", \
		    dev->dev_mac[0], \
		    dev->dev_mac[1], \
		    dev->dev_mac[2], \
		    dev->dev_mac[3], \
		    dev->dev_mac[4], \
		    dev->dev_mac[5], \
		    dev->dev_type); \
        list_del(&dev->list); \
        kfree(dev); \
    } while(0) \

#define CLIENT_ADD(client, dev_type) \
    do \
    { \
		DEBUGP("\nAdd client, mac: %2x:%2x:%2x:%2x:%2x:%2x, ip: 0x%x, dev_type: %u\n", \
		    client->client_mac[0], \
		    client->client_mac[1], \
		    client->client_mac[2], \
		    client->client_mac[3], \
		    client->client_mac[4], \
		    client->client_mac[5], \
		    client->client_ip, \
		    dev_type); \
        list_add(&client->list, &client_lists[CTC_MAX_DEV]); \
        list_add(&client->list_same_devtype, &client_lists[dev_type]); \
    } while(0)\

#define CLIENT_DEL(client) \
    do \
    { \
		DEBUGP("\nDel client, mac: %2x:%2x:%2x:%2x:%2x:%2x, ip: 0x%x\n", \
		    client->client_mac[0], \
		    client->client_mac[1], \
		    client->client_mac[2], \
		    client->client_mac[3], \
		    client->client_mac[4], \
		    client->client_mac[5], \
		    client->client_ip); \
        list_del(&client->list); \
        list_del(&client->list_same_devtype); \
        kfree(client);\
    } while(0) \

//   extern int alias_name_is_eq(char *orig_name,char *cmp_name,char *alias_name );

static void start_poll_timer()
{
#ifdef CLIENTS_POLL_TIMER
    if (NULL == timer_client_polling.function)
    {
    	init_timer(&timer_client_polling);
    	timer_client_polling.expires = jiffies + DETECT_TIME * DETECT_INTERVAL * HZ;
    	timer_client_polling.function = clients_polling_handler;
    	add_timer(&timer_client_polling);
    }
#else
    if (NULL == clients_poll_wq)
    {
    	clients_poll_wq = create_singlethread_workqueue("clientspoll");
        if (NULL == clients_poll_wq)
        {
    		printk("%s: create work queue failed.\n", MODULE_NAME);
            goto error;
        }
        
    	queue_delayed_work(clients_poll_wq, &clients_poll_work, DETECT_TIME * DETECT_INTERVAL * HZ);
    }
#endif
    return;
}

static void stop_poll_timer()
{
#ifdef CLIENTS_POLL_TIMER
    if (timer_client_polling.function)
    {
        del_timer(&timer_client_polling);
        timer_client_polling.function = NULL;
    }
#else
    if (clients_poll_wq)
    {
    	flush_workqueue(clients_poll_wq);
    	destroy_workqueue(clients_poll_wq);
        clients_poll_wq = NULL;
    }
#endif

    return;
}

static unsigned int client_limit_input(unsigned int hooknum,
	 struct sk_buff *skb,
	 const struct net_device *in,
	 const struct net_device *out,
	 int (*okfn)(struct sk_buff *))
{	
	struct list_head *cur, *next, *list;
	struct client_entry *client;
    struct dev_entry *dev;
    int dev_type = CTC_Computer;
    unsigned int client_num = 0, limit;
	struct iphdr *iph;	

    LOCK_CLIENT;
    if (LIMIT_NONE == limit_enable[CTC_MAX_DEV])
    {
        UNLOCK_CLIENT;
    	return NF_ACCEPT;
    }
    UNLOCK_CLIENT;	


	//if ((0 != strcmp(skb->dev->name, "br0"))|| (0 == strcmp(skb->dst->dev->name, "br0")))//
	if(!alias_name_is_eq(CMD_CMP,skb->dev->name,ALIASNAME_BR0)||alias_name_is_eq(CMD_CMP,skb->dst->dev->name,ALIASNAME_BR0))
    {
		DEBUGP("\nFUNC: %s, LINE: %d, input if: %s, output if: %s\n", __FUNCTION__, __LINE__, skb->dev->name, skb->dst->dev->name);
        return NF_ACCEPT;
    }	
	
	iph = ip_hdr(skb);    
	if (!skb->mac_header)
    {
		DEBUGP("\nFUNC: %s, LINE: %d, No mac\n", __FUNCTION__, __LINE__);		
        return NF_ACCEPT;
    }	
	
    LOCK_DEV;
	list_for_each_safe(cur, next, &dev_list)
    { 
		dev = list_entry(cur, struct dev_entry, list);
		if (0 == memcmp(skb->mac_header + ETH_ALEN, dev->dev_mac, ETH_ALEN))
		{
            dev_type = dev->dev_type;
            DEBUGP("\nFUNC: %s, LINE: %d, Find dev_type=%d which depend on DHCP Server\n", __FUNCTION__, __LINE__, dev_type);
    		break;
		}
	}    
    UNLOCK_DEV;	
	
    LOCK_CLIENT;
    switch (limit_enable[CTC_MAX_DEV])
    {
        case LIMIT_GLOBAL:
            list  = &client_lists[CTC_MAX_DEV];
            limit = limit_num[CTC_MAX_DEV];
            break;

        case LIMIT_BY_DEV_TYPE:
            /* client limit by device type */
            if (limit_enable[dev_type])
            {
                list  = &client_lists[dev_type];
                limit = limit_num[dev_type];
            }
            else
            {
                UNLOCK_CLIENT;
            	return NF_ACCEPT;
            }
            
            break;

        default:
            /* No client limit */
            UNLOCK_CLIENT;
        	return NF_ACCEPT;
    }
	
	DEBUGP("\nFUNC: %s, LINE: %d, dev_type=%d, Limit type=%d, Limit_num=%d\n", __FUNCTION__, __LINE__, dev_type, limit_enable[CTC_MAX_DEV], limit);
	
	list_for_each_safe(cur, next, list)
    {
        client_num++;
        
        if (LIMIT_GLOBAL == limit_enable[CTC_MAX_DEV])
        {
    		client = list_entry(cur, struct client_entry, list);
        }
        else
        {
    		client = list_entry(cur, struct client_entry, list_same_devtype);
        }
        
        if ((0 == memcmp(skb->mac_header + ETH_ALEN, client->client_mac, ETH_ALEN))            
			&& (client->client_ip == iph->saddr))
        {
            UNLOCK_CLIENT;
			DEBUGP("\nFUNC: %s, LINE: %d, Get client from List, Forward this packet\n", __FUNCTION__, __LINE__);
            return NF_ACCEPT;
        }
	}
    
    if (client_num < limit)
    {
    	client = (struct client_entry*)kmalloc(sizeof(struct client_entry), GFP_ATOMIC);
        if (NULL == client)
        {
            UNLOCK_CLIENT;
			DEBUGP("\nFUNC: %s, LINE: %d, Can not kmalloc for this client, Drop this packet\n", __FUNCTION__, __LINE__);
            return NF_DROP;
        }
        
		client->client_ip = iph->saddr;
        client->polling_succ = 0;
        client->polling_time = DETECT_TIME;
        memcpy(client->client_mac, skb->mac_header + ETH_ALEN, ETH_ALEN);
		
		DEBUGP("\nFUNC: %s, LINE: %d, It does not reach the limt, Forward this packet\n", __FUNCTION__, __LINE__);
        CLIENT_ADD(client, dev_type);
    }
    else
    {
		DEBUGP("\nFUNC: %s, LINE: %d, packet blocked for client limit, mac: %2x:%2x:%2x:%2x:%2x:%2x, ip: 0x%x, dev_type: %u, limit: %u\n",
            __FUNCTION__, __LINE__,
		    (skb->mac_header + ETH_ALEN)[0],
		    (skb->mac_header + ETH_ALEN)[1],
		    (skb->mac_header + ETH_ALEN)[2],
		    (skb->mac_header + ETH_ALEN)[3],
		    (skb->mac_header + ETH_ALEN)[4],
		    (skb->mac_header + ETH_ALEN)[5],		    
			iph->saddr,
		    dev_type,
			limit);
        UNLOCK_CLIENT;
        return NF_DROP;
    }

    UNLOCK_CLIENT;
	return NF_ACCEPT;
}

static void notify_detect_new_dev(int dev_type, char *mac)
{
	struct list_head *cur, *next;
	struct dev_entry *dev;
	struct client_entry *client;
    int found = 0, old_dev_type = CTC_Computer;

    LOCK_DEV;
	list_for_each_safe(cur, next, &dev_list)
    { 
		dev = list_entry(cur, struct dev_entry, list);
		if (0 == memcmp(mac, dev->dev_mac, ETH_ALEN))
		{
            found = 1;
            if (dev->dev_type != dev_type)
            {
                old_dev_type = dev->dev_type;
                dev->dev_type = dev_type;
            }
    		break;
		}
	}

    if (!found)
    {
    	dev = (struct dev_entry*)kmalloc(sizeof(struct dev_entry), GFP_ATOMIC);
        if (NULL == dev)
        {
            UNLOCK_DEV;
            return;
        }

        memcpy(dev->dev_mac, mac, ETH_ALEN);
        dev->dev_type = dev_type;
        
        DEV_ADD(dev);
    }    
    UNLOCK_DEV;

    if (old_dev_type != dev_type)
    {
        LOCK_CLIENT;
        
        /* device type changed, update client list */
    	list_for_each_safe(cur, next, &client_lists[old_dev_type])
        {
    		client = list_entry(cur, struct client_entry, list_same_devtype);
            
            if (0 == memcmp(mac, client->client_mac, ETH_ALEN))
            {
                CLIENT_DEL(client);
            }
    	}
        
        UNLOCK_CLIENT;
    }
    
	return;
}

/****      mac to string function*********/
static int hex(unsigned char ch)
{
	if (ch >= 'a' && ch <= 'f')
		return ch-'a'+10;
	if (ch >= '0' && ch <= '9')
		return ch-'0';
	if (ch >= 'A' && ch <= 'F')
		return ch-'A'+10;
	return -1;
}

//mac's size must be >=17, content format:"00-16-76-D9-A6-AF"
static void convert_mac(char *mac)
{
	char temp[ETH_ALEN];
	int i;
	for(i = 0; i < ETH_ALEN; i++) {
		temp[i] = hex(mac[i * 3]) * 16 + hex(mac[i * 3 + 1]);
	}
	memcpy(mac, temp, ETH_ALEN);
}
/**************************************************/

static int do_write_dev(struct file *file, const char *buffer, unsigned long count, void *data)
{
    char buf[65];
    char mac[32];
    int dev_type;
    
    if (count > 64)
    {
        DEBUGP("Input too long\n");
        return -EFAULT;
    }
    
    if (copy_from_user(buf, buffer, count))
    {
        return -EFAULT;
    }

    buf[count] = 0;
    DEBUGP("Input is %s\n", buf);

    if (0 == memcmp(buf, "device", 6)) 
    {
        if (strlen(buf) < 7)
        {
            DEBUGP("Too few arg\n");
            return count;
        }
        
        sscanf(buf, "device:%d:%s", &dev_type, mac);
        if (mac[strlen(mac) - 1] == '\n')
        {
            mac[strlen(mac) - 1] = '\0';
        }
        
        convert_mac(mac);
        notify_detect_new_dev(dev_type, mac);
    } 	

    return count;
}

static int do_write_config(struct file *file, const char *buffer, unsigned long count, void *data)
{
    char buf[1025];
    char *line, *end;
    int config_correct = 1;
    int tmp_limit_enable[CTC_MAX_DEV + 1];
    unsigned int tmp_limit_num[CTC_MAX_DEV + 1];
    int i, client_num;
 	struct list_head *cur, *next;
	struct client_entry *client;
   
    if (count > 1024)
    {
        DEBUGP("Input too long\n");
        return -EFAULT;
    }
    
    if (copy_from_user(buf, buffer, count))
    {
        return -EFAULT;
    }

    buf[count] = 0;
    DEBUGP("Input is %s\n", buf);

    /* Copy config to tmp */
    for (i = 0; i < CTC_MAX_DEV + 1; i++)
    {
        tmp_limit_enable[i] = limit_enable[i];
        tmp_limit_num[i]    = limit_num[i];
    }

    line = buf;
    end = buf + count;
    while ((line <= end) && config_correct)
    {
        for (i = 0; i < CTC_MAX_DEV + 1; i++)
        {
            if (0 == memcmp(line, enable_str[i], strlen(enable_str[i])))
            {
                line += strlen(enable_str[i]) + 1;
                sscanf(line, "%u", &tmp_limit_enable[i]);
                if (CTC_MAX_DEV != i)
                {
                    /* Check enable value */
                    if ((0 != tmp_limit_enable[i])
                        && (1 != tmp_limit_enable[i]))
                    {
                        config_correct = 0;
                    }
                }
                else
                {
                    /* Check mode value */
                    if ((LIMIT_NONE != tmp_limit_enable[i])
                        && (LIMIT_GLOBAL != tmp_limit_enable[i])
                        && (LIMIT_BY_DEV_TYPE != tmp_limit_enable[i]))
                    {
                        config_correct = 0;
                    }
                }
                
                break;
            }
            else if (0 == memcmp(line, limit_str[i], strlen(limit_str[i])))
            {
                line += strlen(limit_str[i]) + 1;
                sscanf(line, "%u", &tmp_limit_num[i]);
                break;
            }
        }

        /* find next line */
        while ((line <= end) && ('\n' != *line++))
        {
        }
    }

    if (!config_correct)
    {
        DEBUGP("Invalid client limit config\n");
        return -EFAULT;
    }
    
    LOCK_CLIENT;
    /* Copy config from tmp */
    for (i = 0; i < CTC_MAX_DEV + 1; i++)
    {
        limit_enable[i] = tmp_limit_enable[i];
        limit_num[i]    = tmp_limit_num[i];
    }

    /* update client list */
    switch (limit_enable[CTC_MAX_DEV])
    {
        case LIMIT_NONE:
            /* Stop timer */
            stop_poll_timer();
            
            /* Clear client list */
        	list_for_each_safe(cur, next, &client_lists[CTC_MAX_DEV])
            {
        		client = list_entry(client_lists[CTC_MAX_DEV].next, struct client_entry, list);
                CLIENT_DEL(client);
            }
            
            break;
            
        case LIMIT_GLOBAL:
            client_num = 0;
            
        	list_for_each_safe(cur, next, &client_lists[CTC_MAX_DEV])
            {
                client_num++;
        	}

            /* remove clients if necessary */
            client_num -= limit_num[CTC_MAX_DEV];
            while (client_num-- > 0)
            {
        		client = list_entry(client_lists[CTC_MAX_DEV].next, struct client_entry, list);
                CLIENT_DEL(client);
            }

            start_poll_timer();
            break;

        case LIMIT_BY_DEV_TYPE:
            for (i = 0; i < CTC_MAX_DEV; i++)
            {
                if (limit_enable[i])
                {
                    client_num = 0;
                    
                	list_for_each_safe(cur, next, &client_lists[i])
                    {
                        client_num++;
                	}

                    /* remove clients if necessary */
                    client_num -= limit_num[i];
                    while (client_num-- > 0)
                    {
                		client = list_entry(client_lists[i].next, struct client_entry, list_same_devtype);
                        CLIENT_DEL(client);
                    }
                }
            }

            start_poll_timer();
            break;

        default:
            /* Impossible to get here */
            break;
    }
    
    UNLOCK_CLIENT;
    
    return count;
}

#ifdef CLIENTS_POLL_TIMER
static void clients_polling_handler(unsigned long data)
#else
static void clients_polling_handler(void *data)
#endif
{
	struct net_device *dev;
    ipaddr_t saddr;
 	struct list_head *cur, *next;
	struct client_entry *client;
    struct arpreq_by_client_limit arp_req;
    unsigned long now = jiffies;
    
	//dev = dev_get_by_name("br0");

    dev = dev_get_by_name(&init_net,ALIASNAME_BR0);
//        dev = dev_get_by_name(&init_net,"br0");//
	if (NULL == dev)
	{
        goto ret;
	}

    LOCK_CLIENT; 
	list_for_each_safe(cur, next, &client_lists[CTC_MAX_DEV])
    {
		client = list_entry(cur, struct client_entry, list);
again:        
        if (!client->polling_succ)
        {
            if (DETECT_TIME != client->polling_time)
            {
                /* check arp */
                arp_req.ip = client->client_ip;
                if (arp_req_by_client_limit((void *)&arp_req))
                {
            		DEBUGP("\nGet arp, mac: %2x:%2x:%2x:%2x:%2x:%2x, ip: 0x%x, flag: 0x%x, confirmed: %ul, now: %ul, HZ: %ul\n",
            		    arp_req.mac[0],
            		    arp_req.mac[1],
            		    arp_req.mac[2],
            		    arp_req.mac[3],
            		    arp_req.mac[4],
            		    arp_req.mac[5],
            		    arp_req.ip,
            		    arp_req.arp_flags,
            		    arp_req.confirmed,
            		    now, HZ);
                    if ((arp_req.arp_flags & ATF_COM)
                        && (0 == memcmp(arp_req.mac, client->client_mac, ETH_ALEN)))
                    {
                        if (!(arp_req.arp_flags & ATF_PERM))
                        {
                            /* Not static arp, check confirmd time */
                            arp_req.confirmed += (DETECT_INTERVAL + 1) * HZ; /* +1 to revise deviation */
                            if (time_after(arp_req.confirmed, now))
                            {
                                client->polling_succ = 1;
                                client->polling_time = DETECT_TIME;
                            }
                        }
                        else
                        {
                            client->polling_succ = 1;
                            client->polling_time = DETECT_TIME;
                        }
                    }
                }
                else
                {
                    DEBUGP("Can't get arp, ip: 0x%x\n", arp_req.ip);
                }
            }

            if (!client->polling_succ)
            {
                if (0 == client->polling_time)
                {
                    /* Time out, del client */
                    CLIENT_DEL(client);
                }
                else
                {
                    client->polling_time--;
                    
                    /* Send arp */
            		saddr = inet_select_addr(dev, client->client_ip, RT_SCOPE_LINK);
            		if (saddr)
            		{
                		DEBUGP("\nDetect client, mac: %2x:%2x:%2x:%2x:%2x:%2x, ip: 0x%x\n",
                		    client->client_mac[0],
                		    client->client_mac[1],
                		    client->client_mac[2],
                		    client->client_mac[3],
                		    client->client_mac[4],
                		    client->client_mac[5],
                		    client->client_ip);
            			arp_send(ARPOP_REQUEST, ETH_P_ARP, client->client_ip, dev, saddr,
            					NULL, dev->dev_addr, NULL);
            		}
                    else
                    {
                        /* Incorrect client, del it */
                        CLIENT_DEL(client);
                    }
                }
            }
        }
        else
        {
            client->polling_time--;
            if (0 == client->polling_time)
            {
                client->polling_succ = 0;
                client->polling_time = DETECT_TIME;
                goto again;
            }
        }
	}
    
    UNLOCK_CLIENT;
	dev_put(dev);
    
ret:	
	/* restart polling timer; */
#ifdef CLIENTS_POLL_TIMER
	mod_timer(&timer_client_polling, jiffies + DETECT_INTERVAL * HZ);
#else
	queue_delayed_work(clients_poll_wq, &clients_poll_work, DETECT_INTERVAL * HZ);
#endif

    return;
}

static int __init init(void)
{
	int i;
	struct proc_dir_entry *proc_dev;
	struct proc_dir_entry *proc_config;
    
	proc_dev = create_proc_entry(CLIENT_LIMIT_DEV_FILE, 0, NULL);
    if (NULL == proc_dev)
    {
		printk("%s: create proc entry %s failed.\n", MODULE_NAME, CLIENT_LIMIT_DEV_FILE);
        goto error;
    }
    
	proc_config = create_proc_entry(CLIENT_LIMIT_CONFIG_FILE, 0, NULL);
	if (NULL == proc_config)
	{
		printk("%s: create proc entry %s failed.\n", MODULE_NAME, CLIENT_LIMIT_CONFIG_FILE);
        goto error;
	}
    
	proc_dev->write_proc = do_write_dev;	
    
	proc_config->write_proc = do_write_config;	

    LOCK_CLIENT;
    for (i = 0; i < CTC_MAX_DEV + 1; i++)
    {
        INIT_LIST_HEAD(&client_lists[i]);
        limit_enable[i] = 0;
        limit_num[i] = 0;
    }
    UNLOCK_CLIENT;
    
    LOCK_DEV;
    INIT_LIST_HEAD(&dev_list);
    UNLOCK_DEV;
	
	if (nf_register_hooks(client_limit_ops, ARRAY_SIZE(client_limit_ops)) < 0) 
    {
		printk("%s: can't register hooks.\n", MODULE_NAME);
        goto error;
	}

	printk("%s loaded\n", MODULE_NAME);

	return 0;
    
error:
	remove_proc_entry(CLIENT_LIMIT_DEV_FILE, NULL);	
	remove_proc_entry(CLIENT_LIMIT_CONFIG_FILE, NULL);	
	nf_unregister_hooks(client_limit_ops, ARRAY_SIZE(client_limit_ops));

	printk("Fail to load %s\n", MODULE_NAME);
    
    return 0;
}

static void fini(void)
{
	struct list_head *cur, *next;
	struct client_entry *client;
    struct dev_entry *dev;

	remove_proc_entry(CLIENT_LIMIT_DEV_FILE, NULL);	
	remove_proc_entry(CLIENT_LIMIT_CONFIG_FILE, NULL);	
	nf_unregister_hooks(client_limit_ops, ARRAY_SIZE(client_limit_ops));

    stop_poll_timer();
    
    LOCK_CLIENT;
	list_for_each_safe(cur, next, &client_lists[CTC_MAX_DEV])
    {
		client = list_entry(cur, struct client_entry, list);
        CLIENT_DEL(client);
	}
    UNLOCK_CLIENT;
    
    LOCK_DEV;
	list_for_each_safe(cur, next, &dev_list)
    { 
		dev = list_entry(cur, struct dev_entry, list);
        DEV_DEL(dev);
	}    
    UNLOCK_DEV;

	printk("%s unloaded\n", MODULE_NAME);

    return;
}

module_init(init);
module_exit(fini);
