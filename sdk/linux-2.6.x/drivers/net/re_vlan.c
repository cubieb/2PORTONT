#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <net/net_namespace.h>
#include <linux/if_vlan.h>
#ifdef CONFIG_RTL_MULTI_ETH_WAN
#include <linux/if_smux.h>
#endif

#include "re_vlan.h"
#include "../../net/bridge/br_private.h"

#ifdef RE_VLAN_MEM_DBG
static int mem_cnt = 0;
#endif
static unsigned int re_vlan_flags = RE_VLAN_CHECK_ENABLE;
static struct proc_dir_entry *re_vlan_proc = NULL;
extern void re_vlan_ioctl_set(int (*hook) (void __user *));

static struct list_head lan_dev_list, wan_dev_list;
static DEFINE_RWLOCK(re_vlan_lock);


static struct re_vlan_pair *re_vlan_alloc_pair(unsigned short lanVid, unsigned short wanVid)
{
	struct re_vlan_pair *vlan_pair = NULL;

	vlan_pair = kmalloc(sizeof(struct re_vlan_pair), GFP_KERNEL);
	if (vlan_pair) {
		_RE_VLAN_DEG_MEM_ADD(mem_cnt, sizeof(struct re_vlan_pair));
		vlan_pair->lanVid = lanVid;
		vlan_pair->wanVid = wanVid;
		INIT_LIST_HEAD(&vlan_pair->lan_pair_list);
		INIT_LIST_HEAD(&vlan_pair->wan_pair_list);
	}

	return vlan_pair;
}

static int re_vlan_free_pair(struct re_vlan_pair *vlan_pair)
{
	if (vlan_pair) {
		if (!list_empty(&vlan_pair->lan_pair_list))
			list_del(&vlan_pair->lan_pair_list);
		if (!list_empty(&vlan_pair->wan_pair_list))
			list_del(&vlan_pair->wan_pair_list);
		kfree(vlan_pair);
		_RE_VLAN_DEG_MEM_DEC(mem_cnt, sizeof(struct re_vlan_pair));
		return 0;
	}
	else {
		printk(__RE_VLAN__"<%s,%d> vlan_pair is NULL!\n", __func__, __LINE__);
		return -EINVAL;
	}
}

static struct re_vlan_dev *re_vlan_alloc_dev(struct net_device *dev, unsigned short vid)
{
	struct re_vlan_dev *vlan_dev = NULL;

	vlan_dev = kmalloc(sizeof(struct re_vlan_dev), GFP_KERNEL);
	if (vlan_dev) {
		_RE_VLAN_DEG_MEM_ADD(mem_cnt, sizeof(struct re_vlan_dev));
		vlan_dev->dev = dev;
		vlan_dev->vid = vid;
		INIT_LIST_HEAD(&vlan_dev->vlan_pair_list);
	}

	return vlan_dev;
}

static int re_vlan_free_dev(struct re_vlan_dev *vlan_dev)
{
	struct re_vlan_pair *pos_pair, *next_pair;

	if (vlan_dev) {
		write_lock_irq(&re_vlan_lock);
		if (!list_empty(&vlan_dev->vlan_pair_list)) {
			if (!(vlan_dev->dev->priv_flags & IFF_DOMAIN_WAN)) {
				/* free all vlan pairs if vlan_dev is not associated with WAN dev */
				list_for_each_entry_safe(pos_pair, next_pair, &vlan_dev->vlan_pair_list, lan_pair_list) {
					re_vlan_free_pair(pos_pair);
				}
			}
			else {
				/* only free all links to vlan pairs if vlan_dev is associated with WAN dev */
				list_for_each_entry_safe(pos_pair, next_pair, &vlan_dev->vlan_pair_list, wan_pair_list) {
					list_del_init(&pos_pair->wan_pair_list);
				}
			}
		}
		list_del(&vlan_dev->dev_list);
		write_unlock_irq(&re_vlan_lock);
		kfree(vlan_dev);
		_RE_VLAN_DEG_MEM_DEC(mem_cnt, sizeof(struct re_vlan_dev));
		return 0;
	}
	else {
		printk(__RE_VLAN__"<%s,%d> vlan_dev is NULL!\n", __func__, __LINE__);
		return -EINVAL;
	}
}


/* find vlan_dev that is associated with dev in lan_dev_list */
static struct re_vlan_dev *re_vlan_find_dev(struct net_device *dev, struct list_head *phead)
{
	struct re_vlan_dev *found_dev, *pos_dev;

	found_dev = NULL;
	if (!list_empty(phead)) {
		read_lock(&re_vlan_lock);
		list_for_each_entry(pos_dev, phead, dev_list) {
			if (pos_dev->dev == dev) {
				found_dev = pos_dev;
				break;
			}
		}
		read_unlock(&re_vlan_lock);
	}

	return found_dev;
}

/* find vlan_pair in a specific vlan_dev */
static struct re_vlan_pair *re_vlan_find_pair(struct re_vlan_dev *vlan_dev, unsigned short lanVid,
												unsigned short wanVid, enum re_vlan_check_vid vlan_check)
{
	struct re_vlan_pair *found_pair, *pos_pair;

	found_pair = NULL;
	if (!list_empty(&vlan_dev->vlan_pair_list)) {
		read_lock(&re_vlan_lock);
		if (!(vlan_dev->dev->priv_flags & IFF_DOMAIN_WAN)) {
			list_for_each_entry(pos_pair, &vlan_dev->vlan_pair_list, lan_pair_list) {
				if (vlan_check == RE_VLAN_CHECK_LANID) {
					if (pos_pair->lanVid == lanVid) {
						found_pair = pos_pair;
						break;
					}
				}
				else if (vlan_check == RE_VLAN_CHECK_WANID) {
					if (pos_pair->wanVid == wanVid) {
						found_pair = pos_pair;
						break;
					}
				}
				else if (vlan_check == RE_VLAN_CHECK_BOTH_LANID_WANID) {
					if (pos_pair->lanVid == lanVid && pos_pair->wanVid == wanVid) {
						found_pair = pos_pair;
						break;
					}
				}
				else {
					printk("not support!\n");
					break;
				}

			}
		}
		else {
			list_for_each_entry(pos_pair, &vlan_dev->vlan_pair_list, wan_pair_list) {
				if (vlan_check == RE_VLAN_CHECK_LANID) {
					if (pos_pair->lanVid == lanVid) {
						found_pair = pos_pair;
						break;
					}
				}
				else if (vlan_check == RE_VLAN_CHECK_WANID) {
					if (pos_pair->wanVid == wanVid) {
						found_pair = pos_pair;
						break;
					}
				}
				else if (vlan_check == RE_VLAN_CHECK_BOTH_LANID_WANID) {
					if (pos_pair->lanVid == lanVid && pos_pair->wanVid == wanVid) {
						found_pair = pos_pair;
						break;
					}
				}
				else {
					printk("not support!\n");
					break;
				}
			}
		}
		read_unlock(&re_vlan_lock);
	}

	return found_pair;
}

/* add a vlan_pair for a vlan mapping net device */
static int re_vlan_add_vlan_pair(struct net_device *dev, unsigned short lanVid, unsigned short wanVid)
{
	struct re_vlan_dev *vlan_dev, *found_dev;
	struct re_vlan_pair *vlan_pair;
	int err = 0;

	/* find vlan_dev that is associated with dev in lan_dev_list, allocate a new one if it's not found */
	found_dev = re_vlan_find_dev(dev, &lan_dev_list);

	if (!found_dev) {
		vlan_dev = re_vlan_alloc_dev(dev, 0);
		if (!vlan_dev) {
			printk(__RE_VLAN__"<%s,%d> fail!\n", __func__, __LINE__);
			err = -ENOMEM;
			goto out;
		}
	}
	else {
		vlan_dev = found_dev;
	}

	/* check if (lanVid, wanVid) already existed in the vlan_dev */
	if (re_vlan_find_pair(vlan_dev, lanVid, wanVid, RE_VLAN_CHECK_LANID)) {
		printk(__RE_VLAN__"vlan pair <%u, xx> already existed!\n", lanVid);
		err = -EINVAL;
		goto out;
	}

	/* allocate a new vlan_pair */
	vlan_pair = re_vlan_alloc_pair(lanVid, wanVid);
	if (!vlan_pair) {
		printk(__RE_VLAN__"<%s,%d> fail!\n", __func__, __LINE__);
		err = -ENOMEM;
		goto out;
	}
	sprintf(vlan_pair->ifname, "%s.%u", dev->name, lanVid);

	/* append the new vlan_pair in a specific vlan_dev */
	write_lock_irq(&re_vlan_lock);
	list_add_tail(&vlan_pair->lan_pair_list, &vlan_dev->vlan_pair_list);

	/* append the new vlan_dev in lan_dev_list */
	if (!found_dev)
		list_add_tail(&vlan_dev->dev_list, &lan_dev_list);

	/* append the new vlan_pair in a wan vlan_dev whose vid is equal to wanVid */
	if (!list_empty(&wan_dev_list)) {
		list_for_each_entry(vlan_dev, &wan_dev_list, dev_list) {
			_RE_VLAN_DBG(__RE_VLAN__"<%s,%d> vlan_dev->vid = %u\n", __func__, __LINE__, vlan_dev->vid);
			if (vlan_dev->vid == wanVid) {
				list_add_tail(&vlan_pair->wan_pair_list, &vlan_dev->vlan_pair_list);
				break;
			}
		}
	}
	write_unlock_irq(&re_vlan_lock);

	return err;
out:
	if (!found_dev && vlan_dev) {
		kfree(vlan_dev);
		_RE_VLAN_DEG_MEM_DEC(mem_cnt, sizeof(struct re_vlan_dev));
	}
	
	return err;
}

/* delete a vlan_pair from a vlan mapping net device */
static int re_vlan_del_vlan_pair(struct net_device *dev, unsigned short lanVid, unsigned short wanVid)
{
	struct re_vlan_dev *found_dev;
	struct re_vlan_pair *found_pair;
	int err;

	/* find vlan_dev that is associated with dev in lan_dev_list */
	found_dev = re_vlan_find_dev(dev, &lan_dev_list);
	if (!found_dev) {
		printk(__RE_VLAN__"%s's vlan_dev is not found!\n", dev->name);
		err = -EINVAL;
		goto out;
	}

	/* find vlan_pair in a specific vlan_dev */
	found_pair = re_vlan_find_pair(found_dev, lanVid, wanVid, RE_VLAN_CHECK_LANID);
	if (!found_pair) {
		printk(__RE_VLAN__"%s's vlan_pair <%u, %u> is not found!\n", dev->name, lanVid, wanVid);
		err = -EINVAL;
		goto out;
	}

	write_lock_irq(&re_vlan_lock);
	err = re_vlan_free_pair(found_pair);
	write_unlock_irq(&re_vlan_lock);
	if (!err && list_empty(&found_dev->vlan_pair_list))
		err = re_vlan_free_dev(found_dev);

out:
	return err;
}

/* delete a vlan_dev from a vlan mapping net device */
static int re_vlan_del_vlan_dev(struct net_device *dev, struct list_head *phead)
{
	struct re_vlan_dev *found_dev;

	/* find vlan_dev that is associated with dev in lan_dev_list */
	found_dev = re_vlan_find_dev(dev, phead);
	if (!found_dev) {
		printk(__RE_VLAN__"%s's vlan_dev is not found!\n", dev->name);
		return -EINVAL;
	}

	re_vlan_free_dev(found_dev);

	return 0;
}

static void re_vlan_cleanup(void)
{
	struct re_vlan_dev *pos_dev, *next_dev;

	if (!list_empty(&wan_dev_list)) {
		list_for_each_entry_safe(pos_dev, next_dev, &wan_dev_list, dev_list) {
			re_vlan_free_dev(pos_dev);
		}
	}

	if (!list_empty(&lan_dev_list)) {
		list_for_each_entry_safe(pos_dev, next_dev, &lan_dev_list, dev_list) {
			re_vlan_free_dev(pos_dev);
		}
	}

	return;
}

static void re_vlan_show_all(void)
{
	struct re_vlan_dev *pos_dev;
	struct re_vlan_pair *pos_pair;

	read_lock(&re_vlan_lock);
	if (!list_empty(&lan_dev_list)) {
		list_for_each_entry(pos_dev, &lan_dev_list, dev_list) {
			if (pos_dev->dev) {
				printk("%s:\n", pos_dev->dev->name);
				if (!list_empty(&pos_dev->vlan_pair_list)) {
					list_for_each_entry(pos_pair, &pos_dev->vlan_pair_list, lan_pair_list) {
						printk("%s:<%u, %u> ", pos_pair->ifname, pos_pair->lanVid, pos_pair->wanVid);
					}
					printk("\n\n");
				}
				else
					printk("no vlan pair\n\n");
			}
		}
	}

	if (!list_empty(&wan_dev_list)) {
		list_for_each_entry(pos_dev, &wan_dev_list, dev_list) {
			if (pos_dev->dev) {
				printk("%s:\n", pos_dev->dev->name);
				if (!list_empty(&pos_dev->vlan_pair_list)) {
					list_for_each_entry(pos_pair, &pos_dev->vlan_pair_list, wan_pair_list) {
						printk("%s:<%u, %u> ", pos_pair->ifname, pos_pair->lanVid, pos_pair->wanVid);
					}
					printk("\n\n");
				}
				else
					printk("no vlan pair\n\n");
			}
		}
	}
	read_unlock(&re_vlan_lock);

	return;
}

static int re_vlan_ioctl_handler(void __user *arg)
{
	struct re_vlan_ioctl_args args;
	struct net_device *dev = NULL;
	int err = 0;

	if (copy_from_user(&args, arg, sizeof(struct re_vlan_ioctl_args)))
		return -EFAULT;

	_RE_VLAN_DBG(__RE_VLAN__"cmd=0x%x\n", args.cmd);

	switch (args.cmd) {
		case RE_VLAN_ADD_VLANPAIR_CMD:
			_RE_VLAN_DBG(__RE_VLAN__"ifname=%s, lanVid=0x%x, wanVid=0x%x\n", args.ifname, args.lanVid, args.wanVid);
			dev = dev_get_by_name(&init_net, args.ifname);
			if (!dev)
				err = -EINVAL;
			else
				err = re_vlan_add_vlan_pair(dev, args.lanVid, args.wanVid);
			break;

		case RE_VLAN_DEL_VLANPAIR_CMD:
			dev = dev_get_by_name(&init_net, args.ifname);
			if (!dev)
				err = -EINVAL;
			else
				err = re_vlan_del_vlan_pair(dev, args.lanVid, args.wanVid);
			break;

		case RE_VLAN_DEL_VLANDEV_CMD:
			dev = dev_get_by_name(&init_net, args.ifname);
			if (!dev)
				err = -EINVAL;
			else
				err = re_vlan_del_vlan_dev(dev, &lan_dev_list);
			break;

		case RE_VLAN_SHOW_ALL_CMD:
			re_vlan_show_all();
			break;

		case RE_VLAN_CLEAN_UP_CMD:
			re_vlan_cleanup();
			INIT_LIST_HEAD(&lan_dev_list);
			INIT_LIST_HEAD(&wan_dev_list);
			break;

		default:
			printk(__RE_VLAN__"cmd 0x%x is not supported!\n", args.cmd);
			err = -EINVAL;
			break;
	}

	if (dev)
		dev_put(dev);

	return err;
}

static void skb_debug(const struct sk_buff *skb, const char *func)
{
	#define NUM2PRINT 100
	int i;
	unsigned char *data;

	if (!skb || !skb->dev)
		return;

	data = eth_hdr(skb)->h_dest;
	if (!data)
		return;

	printk("\n\n");
	
	if (func)
		printk("%s:\n", func);

	printk(__RE_VLAN__"from_dev:%s, dev:%s", skb->from_dev?skb->from_dev->name:"NULL", skb->dev->name);

	for (i=0; i<NUM2PRINT; i++) {
		if (i%16==0)
			printk("\n");
		printk("%2.2x ", data[i]);
	}
	printk("\n\n");
}

int re_vlan_addtag(struct sk_buff *skb, unsigned short vid)
{
	if (!RE_VLAN_CHECK_ENABLED)
		return NET_XMIT_SUCCESS;

	if (__vlan_put_tag(skb, vid) == NULL)
		return NET_XMIT_DROP;
	else
		return NET_XMIT_SUCCESS;
}
EXPORT_SYMBOL(re_vlan_addtag);

void re_vlan_untag(struct sk_buff *skb)
{
	if (!RE_VLAN_CHECK_ENABLED)
		return;

	if (skb && !skb->vlan_tci) {
		if (__vlan_get_tag(skb, &skb->vlan_tci))
			return;

		skb->vlan_tci &= 0xFFF;
		skb->vlan_tci += 1;
		memmove(skb->data + VLAN_HLEN, skb->data, VLAN_ETH_ALEN<<1);
		skb_pull(skb, VLAN_HLEN);
	}

	return;
}
EXPORT_SYMBOL(re_vlan_untag);

enum rx_drop_reason {
	R_VLAN_PAIR_NOT_FOUND = 1,
	R_VLAN_PAIR_NOT_EXIST,
	R_RX_DROP_REASON_NUM
};

static const char *rx_drop_str[] = {
	"",
	"vlan pair is not found",		//R_VLAN_PAIR_NOT_FOUND
	"vlan pair is not exist",		//R_VLAN_PAIR_NOT_EXIST
};

enum rx_pass_reason {
	R_UNTAG_OR_FROM_WAN = 1,
	R_DA_IS_NETIF,
	R_VLAN_MAP_DISABLED,
	R_RX_PASS_REASON_NUM
};

static const char *rx_pass_str[] = {
	"",
	"untagged or from WAN",			//R_UNTAG_OR_FROM_WAN
	"DA is netif's mac",			//R_DA_IS_NETIF
	"vlan mapping is disabled"		//R_VLAN_MAP_DISABLED
};

int re_vlan_skb_recv(struct sk_buff *skb)
{
	struct re_vlan_dev *found_dev;
	struct re_vlan_pair *found_pair;
	struct ethhdr *eth;
	int drop_reason = 0, pass_reason = 0;

	if (!skb || !RE_VLAN_CHECK_ENABLED)
		goto out;

	if (RE_VLAN_TX_RX_DUMP_ENABLED)
		skb_debug(skb, __func__);

	eth = eth_hdr(skb);
	if (!eth)
		goto out;
		
	if (!(skb->dev->priv_flags & IFF_DOMAIN_WAN) && skb->dev->br_port && skb->dev->br_port->br)
		br_fdb_update_vid_from_skb(skb->dev->br_port->br, eth->h_source, skb);
		
	/* let untag and WAN rx packets pass, WAN rx packets are filtered in smux */
	if (!skb->vlan_tci || (skb->dev->priv_flags & IFF_DOMAIN_WAN)) {
		pass_reason = R_UNTAG_OR_FROM_WAN;
		goto out;
	}

	/* if DA is netif's mac, let it pass */
	if (!compare_ether_addr_64bits(eth->h_dest, skb->dev->dev_addr)) {
		pass_reason = R_DA_IS_NETIF;
		goto out;
	}

	/* found vlan_dev if vlan mapping is enabled at  this lan port */
	found_dev = re_vlan_find_dev(skb->dev, &lan_dev_list);
	if (!found_dev) {
		pass_reason = R_VLAN_MAP_DISABLED;
		goto out;
	}

	/* found vlan_pair if skb->vlan_tci is matched in vlan mapping list */
	found_pair = re_vlan_find_pair(found_dev, skb->vlan_tci-1, 0, RE_VLAN_CHECK_LANID);

	/* drop if vid from LAN is not in mapping list */
	if (!found_pair) {
		drop_reason = R_VLAN_PAIR_NOT_FOUND;
		goto drop_pkt;
	}

	/* drop if no WAN port exists with wanVid in the found_pair */
	if (list_empty(&found_pair->wan_pair_list)) {
		drop_reason = R_VLAN_PAIR_NOT_EXIST;
		goto drop_pkt;
	}

	skb->switch_port = found_pair->ifname;
	_RE_VLAN_DBG(__RE_VLAN__"skb->switch_port = %s\n", skb->switch_port);

out:
	if (RE_VLAN_TX_RX_DBG_ENABLED && (pass_reason < R_RX_PASS_REASON_NUM))
		printk(__RE_VLAN__"rx pass reason:%d, %s\n", pass_reason, rx_pass_str[pass_reason]);
	return NET_RX_SUCCESS;

drop_pkt:
	if (RE_VLAN_TX_RX_DBG_ENABLED && (drop_reason < R_RX_DROP_REASON_NUM))
		printk(__RE_VLAN__"rx drop reason:%d, %s\n", drop_reason, rx_drop_str[drop_reason]);
	dev_kfree_skb_any(skb);

	return NET_RX_DROP;
}
EXPORT_SYMBOL(re_vlan_skb_recv);

static void re_vlan_flooding_pair(struct sk_buff *skb, struct re_vlan_dev *lan_dev, unsigned short wanVid)
{
	struct re_vlan_pair *pos_pair;
	struct sk_buff *tmpskb;

	list_for_each_entry(pos_pair, &lan_dev->vlan_pair_list, lan_pair_list) {
		if (pos_pair->wanVid == wanVid) {
			tmpskb = skb_clone(skb, GFP_ATOMIC);
			if (tmpskb) {
				tmpskb->vlan_tci = pos_pair->lanVid;
				if (tmpskb->dev->hard_start_xmit(tmpskb, tmpskb->dev)) {
					dev_kfree_skb_any(tmpskb);
					printk(__RE_VLAN__"flooding error!\n");
				}
			}
		}
	}

	return;
}

static void re_vlan_flooding_fdb(struct sk_buff *skb)
{
	struct hlist_node *h;
	struct net_bridge_fdb_entry *f;
	int i;
	struct sk_buff *tmpskb;

	if (!skb->dev->br_port)
		return;

	if (!skb->dev->br_port->br)
		return;

	spin_lock_bh(&skb->dev->br_port->br->hash_lock);
	for (i = 0; i < BR_HASH_SIZE; i++) {
		hlist_for_each_entry_rcu(f, h, &skb->dev->br_port->br->hash[i], hlist) {
			if ((f->dst == skb->dev->br_port) && f->vid) {
				tmpskb = skb_clone(skb, GFP_ATOMIC);
				if (tmpskb) {
					tmpskb->vlan_tci = f->vid-1;
					if (tmpskb->dev->hard_start_xmit(tmpskb, tmpskb->dev)) {
						dev_kfree_skb_any(tmpskb);
						printk(__RE_VLAN__"flooding error!\n");
					}
				}	
			}
		}
	}
	spin_unlock_bh(&skb->dev->br_port->br->hash_lock);

	return;
}

enum tx_pass_reason {
	T_WAN_TX = 1,
	T_VLAN_PACKET,
	T_WAN_DEV_NOT_FOUND,
	T_FLOODING_VLAN_PAIR,
	T_FLOODING_FDB_DOWNSTREAM,
	T_FLOODING_FDB_LOCAL_TO_LAN,
	T_FDB_WITHOUT_VID,
	T_PASS_REASON_NUM
};

static const char *tx_pass_str[] = {
	"",
	"wan tx",						//T_WAN_TX
	"vlan packet",					//T_VLAN_PACKET
	"wan dev not found",			//T_WAN_DEV_NOT_FOUND
	"flooding vlan pair",			//T_FLOODING_VLAN_PAIR
	"flooding fdb(downstream)",		//T_FLOODING_FDB_DOWNSTREAM
	"flooding fdb(local to lan)",	//T_FLOODING_FDB_LOCAL_TO_LAN
	"fdb without VID"				//T_FDB_WITHOUT_VID
};

enum tx_drop_reason {
	T_FROM_WRONG_INTERFACE = 1,
	T_VLAN_PAIR_NOT_FOUND,
	T_LAN_TO_LAN_WITH_VID,
	T_DROP_REASON_NUM
};

static const char *tx_drop_str[] = {
	"",
	"from wrong interface",			//T_FROM_WRONG_INTERFACE
	"vlan pair is not found",		//T_VLAN_PAIR_NOT_FOUND
	"lan to lan with vid"			//T_LAN_TO_LAN_WITH_VID
};

enum tx_found_reason {
	T_FLOODING_PACKET = 1,
	T_FOUND_IN_VLAN_PAIR,
	T_FOUND_VID_IN_FDB,
	T_LOCAL_TO_LAN_WITH_VID, 
	T_FUOND_REASON_NUM
};

static const char *tx_found_str[] = {
	"",
	"flooding packet",				//T_FLOODING_PACKET
	"found in vlan pair",			//T_FOUND_IN_VLAN_PAIR
	"found vid in fdb",				//T_FOUND_VID_IN_FDB
	"local to lan with vid"			//T_LOCAL_TO_LAN_WITH_VID
};

int re_vlan_skb_xmit(struct sk_buff *skb, unsigned short *vid)
{
	struct net_bridge_fdb_entry *fpdst = NULL;
	struct re_vlan_dev *found_lan_dev, *found_wan_dev;
	struct ethhdr *eth;
	int drop_reason = 0, pass_reason = 0, found_reason = 0;
	int ret = NET_XMIT_SUCCESS;

	if (!RE_VLAN_CHECK_ENABLED)
		goto out;

	if (RE_VLAN_TX_RX_DUMP_ENABLED)
		skb_debug(skb, __func__);

	/* WAN tx packet will be checked in driver */
	if (skb->dev->priv_flags & IFF_DOMAIN_WAN) {
		pass_reason = T_WAN_TX;
		goto out;
	}

	/* do not process vlan pkt (wlan driver directly sends pkt from rx) */
	eth = eth_hdr(skb);
	if (!eth || eth->h_proto == ETH_P_8021Q) {
		pass_reason = T_VLAN_PACKET;
		goto out;
	}

	/* down stream */
	if (skb->from_dev && (skb->from_dev->priv_flags & IFF_DOMAIN_WAN)) {
#ifdef CONFIG_RTL_MULTI_ETH_WAN
		if (skb->from_dev->priv_flags & IFF_RSMUX) {
			dev_kfree_skb_any(skb);
			ret = NET_XMIT_DROP;
			drop_reason = T_FROM_WRONG_INTERFACE;
			goto out;
		}
#endif
		/* WAN net device should be generated and append in wan_dev_list when it is registered */
		found_wan_dev = re_vlan_find_dev(skb->from_dev, &wan_dev_list);
		if (!found_wan_dev) {
			printk(__RE_VLAN__"err: cannot find_wan_dev ,skb->from_dev->name=%s\n", skb->from_dev->name);
			pass_reason = T_WAN_DEV_NOT_FOUND;
			goto out;
		}

		/* skb is from re_vlan_flooding_pair() or re_vlan_flooding_fdb() */
		if ((skb->vlan_tci&VLAN_VID_MASK) && ((skb->vlan_tci&VLAN_VID_MASK)-1) != (found_wan_dev->vid&VLAN_VID_MASK)) {
			*vid = skb->vlan_tci&VLAN_VID_MASK;
			found_reason = T_FLOODING_PACKET;
			goto out;
		}
		
		if (!is_multicast_ether_addr(eth->h_dest) && skb->dev->br_port && skb->dev->br_port->br)
			fpdst = br_fdb_get(skb->dev->br_port->br, eth->h_dest);

		found_lan_dev = re_vlan_find_dev(skb->dev, &lan_dev_list);

		/* vlan mapping*/
		if (found_lan_dev) {
			if (unlikely(!compare_ether_addr_64bits(eth->h_dest, skb->dev->broadcast))) {
				re_vlan_flooding_pair(skb, found_lan_dev, (found_wan_dev->vid&VLAN_VID_MASK));
				pass_reason = T_FLOODING_VLAN_PAIR;
			}
			else if (fpdst && fpdst->vid) {
				/* learned dest mac */				
				if (re_vlan_find_pair(found_lan_dev, (fpdst->vid&VLAN_VID_MASK)-1, (found_wan_dev->vid&VLAN_VID_MASK), RE_VLAN_CHECK_BOTH_LANID_WANID)) {
					*vid = (fpdst->vid&VLAN_VID_MASK)-1;
					found_reason = T_FOUND_IN_VLAN_PAIR;
				}
				else {
					dev_kfree_skb_any(skb);
					ret = NET_XMIT_DROP;
					drop_reason = T_VLAN_PAIR_NOT_FOUND;
				}
			}
			else {
				pass_reason = T_FDB_WITHOUT_VID;
			}
		}
		else {	/* not vlan mapping */
			if (unlikely(!compare_ether_addr_64bits(eth->h_dest, skb->dev->broadcast))) {
				re_vlan_flooding_fdb(skb);
				pass_reason = T_FLOODING_FDB_DOWNSTREAM;
			}
			else if (fpdst && fpdst->vid) {
				*vid = fpdst->vid-1;
				found_reason = T_FOUND_VID_IN_FDB;
			}
			else {
				pass_reason = T_FDB_WITHOUT_VID;
			}
		}
	}
	else if (skb->vlan_tci&VLAN_VID_MASK && skb->from_dev && (skb->from_dev->priv_flags & (IFF_DOMAIN_ELAN|IFF_DOMAIN_WLAN))) {	/* drop if packet with vid from LAN to LAN */
		dev_kfree_skb_any(skb);
		ret = NET_XMIT_DROP;
		drop_reason = T_LAN_TO_LAN_WITH_VID;
	}
	else {	/* lan to lan without vid or local to lan */
		/* pkt from re_vlan_flooding_fdb */
		if (skb->vlan_tci&VLAN_VID_MASK) {
			*vid = skb->vlan_tci&VLAN_VID_MASK;
			found_reason = T_FLOODING_PACKET;
			goto out;
		}
			
		if (unlikely(!compare_ether_addr_64bits(eth->h_dest, skb->dev->broadcast))) {
			re_vlan_flooding_fdb(skb);
			pass_reason = T_FLOODING_FDB_LOCAL_TO_LAN;
			goto out;
		}

		if (skb->dev->br_port && skb->dev->br_port->br)
			fpdst = br_fdb_get(skb->dev->br_port->br, eth->h_dest);
		
		if (fpdst && fpdst->vid) {	/* local to lan with vid */
			*vid = fpdst->vid-1;
			found_reason = T_LOCAL_TO_LAN_WITH_VID;
		}
		else {	/* no vid */
			pass_reason = T_FDB_WITHOUT_VID;
		}
	}

out:
	if (RE_VLAN_TX_RX_DBG_ENABLED) {
		if (drop_reason && (drop_reason < T_DROP_REASON_NUM))
			printk(__RE_VLAN__"tx drop reason:%d, %s\n", drop_reason, tx_drop_str[drop_reason]);
		else if (found_reason && (found_reason < T_FUOND_REASON_NUM))
			printk(__RE_VLAN__"tx found reason:%d, %s\n", found_reason, tx_found_str[found_reason]);
		else if (pass_reason && (pass_reason < T_PASS_REASON_NUM))
			printk(__RE_VLAN__"tx pass reason:%d, %s\n", pass_reason, tx_pass_str[pass_reason]);
	}
	if (fpdst)
		br_fdb_put(fpdst);
	return ret;
}
EXPORT_SYMBOL(re_vlan_skb_xmit);

static int re_vlan_device_event(struct notifier_block *unused, unsigned long event, void *ptr)
{
	struct net_device *dev = ptr;
	struct re_vlan_dev *vlan_dev, *pos_dev;
	struct re_vlan_pair *pos_pair;
#ifdef CONFIG_RTL_MULTI_ETH_WAN
	struct smux_dev_info *dev_info;
#endif
	unsigned short vid = 0;

	if (!dev || !(dev->priv_flags & IFF_DOMAIN_WAN))
		goto out;

#ifdef CONFIG_RTL_MULTI_ETH_WAN
	if (dev->priv_flags & IFF_OSMUX) {
		dev_info = SMUX_DEV_INFO(dev);
		vid = dev_info->vid;
	}
#endif

	/* no information to get vid from the WAN device */
	if (!vid)
		goto out;

	switch (event) {
		case NETDEV_REGISTER:
			if (re_vlan_find_dev(dev, &wan_dev_list)) {
				printk(__RE_VLAN__"<%s,%d> error! netdev already exists!\n", __func__, __LINE__);
				goto out;
			}

			vlan_dev = re_vlan_alloc_dev(dev, vid);
			if (!vlan_dev) {
				printk(__RE_VLAN__"<%s,%d> fail!\n", __func__, __LINE__);
				goto out;
			}
			write_lock_irq(&re_vlan_lock);
			list_add_tail(&vlan_dev->dev_list, &wan_dev_list);

			/* search all vlan_pairs with the same wanVid to vid and link those pairs in wan_pair_list*/
			list_for_each_entry(pos_dev, &lan_dev_list, dev_list) {
				if (!list_empty(&pos_dev->vlan_pair_list)) {
					list_for_each_entry(pos_pair, &pos_dev->vlan_pair_list, lan_pair_list) {
						if (pos_pair->wanVid == vid) {
							_RE_VLAN_DBG_FUNC_LINE;
							list_add_tail(&pos_pair->wan_pair_list, &vlan_dev->vlan_pair_list);
						}
					}
				}
			}
			write_unlock_irq(&re_vlan_lock);
			break;

		case NETDEV_UNREGISTER:
			re_vlan_del_vlan_dev(dev, &wan_dev_list);
			break;

		default:
			break;
	}

out:
	return NOTIFY_DONE;
}


static struct notifier_block re_vlan_notifier_block = {
	.notifier_call = re_vlan_device_event
};

static int re_vlan_proc_read(char *page, char **start, off_t off, 
		int count, int *eof, void *data)
{
	int len = 0;

	len += sprintf(page+len, __RE_VLAN__"vlan \"check\" is %s\n", RE_VLAN_CHECK_ENABLED?"enabled":"disabled");
	len += sprintf(page+len, __RE_VLAN__"\"debug\" is %s\n", RE_VLAN_TX_RX_DBG_ENABLED?"enabled":"disabled");
	len += sprintf(page+len, __RE_VLAN__"tx/rx \"dump\" is %s\n", RE_VLAN_TX_RX_DUMP_ENABLED?"enabled":"disabled");

	if (len <= off+count) *eof = 1;
		*start = page + off;
	len -= off;
	if (len > count)
		len = count;
	if (len < 0)
		len = 0;

	return len;
}

static int re_vlan_proc_write(struct file *filp, const char *buff,unsigned long len, void *data)
{
	char tmpbuf[32];
	char *strptr;	

	if (buff && !copy_from_user(tmpbuf, buff, len)) {
		tmpbuf[len] = '\0';
		strptr = tmpbuf;

		if (!strlen(strptr))
			goto out;

		if (!strncmp(strptr, "enable", 6)) {
			if (RE_VLAN_CHECK_ENABLED) {
				RE_VLAN_CLEAR_FLAGS(RE_VLAN_CHECK_ENABLE);
				printk(__RE_VLAN__"disable vlan translation\n");
			}
			else {
				RE_VLAN_SET_FLAGS(RE_VLAN_CHECK_ENABLE);
				printk(__RE_VLAN__"enable vlan translation\n");
			}
		}
		else if (!strncmp(strptr, "debug", 5)) {
			if (RE_VLAN_TX_RX_DBG_ENABLED) {
				RE_VLAN_CLEAR_FLAGS(RE_VLAN_TX_RX_DBG_ENABLE);
				printk(__RE_VLAN__"disable debug\n");
			}
			else {
				RE_VLAN_SET_FLAGS(RE_VLAN_TX_RX_DBG_ENABLE);
				printk(__RE_VLAN__"enable debug\n");
			}
		}
		else if (!strncmp(strptr, "dump", 4)) {
			if (RE_VLAN_TX_RX_DUMP_ENABLED) {
				RE_VLAN_CLEAR_FLAGS(RE_VLAN_TX_RX_DUMP_ENABLE);
				printk(__RE_VLAN__"disable tx/rx dump\n");
			}
			else {
				RE_VLAN_SET_FLAGS(RE_VLAN_TX_RX_DUMP_ENABLE);
				printk(__RE_VLAN__"enable tx/rx dump\n");
			}
		}
		else {
			printk(__RE_VLAN__"wrong input!\n");
			goto out;
		}
	}

out:
	return len;
}

static int re_vlan_proc_create(void)
{
	if (re_vlan_proc) {
		printk(__RE_VLAN__"err: re_vlan_proc already exists!\n");
		return -EPERM;
	}

	re_vlan_proc = create_proc_entry(RE_VLAN_PROC, 0, NULL);
	if (re_vlan_proc) {
		re_vlan_proc->read_proc = re_vlan_proc_read;
		re_vlan_proc->write_proc = re_vlan_proc_write;
	}
	else {
		printk(__RE_VLAN__"err: create re_vlan_proc failed!\n");
		return -ENOMEM;
	}

	return 0;
}

static void re_vlan_proc_cleanup(void)
{
	if (!re_vlan_proc)
		return;

	remove_proc_entry(RE_VLAN_PROC, NULL);
	re_vlan_proc = NULL;
}

static int __init re_vlan_init(void)
{
	int err = 0;
	INIT_LIST_HEAD(&lan_dev_list);
	INIT_LIST_HEAD(&wan_dev_list);
	err = register_netdevice_notifier(&re_vlan_notifier_block);
	if (err)
		goto out1;

	err = re_vlan_proc_create();
	if (err)
		goto out2;

	re_vlan_ioctl_set(re_vlan_ioctl_handler);
#ifdef RE_VLAN_MEM_DBG
	printk(__RE_VLAN__"mem_cnt:0x%p\n", &mem_cnt);
#endif
out1:
	return err;
out2:
	unregister_netdevice_notifier(&re_vlan_notifier_block);
	return err;
}

static void __exit re_vlan_exit(void)
{
	re_vlan_ioctl_set(NULL);
	unregister_netdevice_notifier(&re_vlan_notifier_block);
	re_vlan_cleanup();
	re_vlan_proc_cleanup();
#ifdef RE_VLAN_MEM_DBG
	printk(__RE_VLAN__"mem_cnt=%d\n", mem_cnt);
#endif

	return;
}

module_init(re_vlan_init);
module_exit(re_vlan_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("VLAN mapping");

