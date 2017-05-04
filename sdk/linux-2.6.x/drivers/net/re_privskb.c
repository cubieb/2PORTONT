/*	
 *  re_privskb.c: Ethernet Private Skb Management driver
 *  Date: 2010/08/17
 *  Auther: From AP team
 */

#ifdef CONFIG_RTL865X_ETH_PRIV_SKB
#include <linux/skbuff.h>
#include <linux/interrupt.h>
#include "re_privskb.h"

#ifdef RTK_QUE
struct ring_que {
	int qlen;
	int qmax;	
	int head;
	int tail;
	struct sk_buff *ring[MAX_PRE_ALLOC_RX_SKB+1];
};
 static struct ring_que rx_skb_queue;
#else
 static struct sk_buff_head rx_skb_queue; 
#endif  //end RTK_QUE

#if 0 /* 2012-3-12 krammer add */
#define ETH_SKB_BUF_SIZE	(CROSS_LAN_MBUF_LEN+sizeof(struct skb_shared_info)+128)
#else
#define ETH_SKB_BUF_SIZE	(CROSS_LAN_MBUF_LEN+128+NET_SKB_PAD+sizeof(struct skb_shared_info)+128)
#endif /* 2012-3-12 krammer add */
#define ETH_MAGIC_CODE		"867x"

int eth_skb_free_num=MAX_ETH_SKB_NUM;

struct priv_skb_buf2 {
	unsigned char magic[4];
	unsigned int buf_pointer;
	struct list_head	list;	
	unsigned char buf[ETH_SKB_BUF_SIZE];
};
static struct priv_skb_buf2 eth_skb_buf[MAX_ETH_SKB_NUM];
static struct list_head eth_skbbuf_list;
extern struct sk_buff *dev_alloc_8190_skb(unsigned char *data, int size);

#ifdef ETH_PRIV_SKB_PROC
#include <linux/proc_fs.h>
static int priv_eth_skb_read_proc(char *page, char **start, off_t off,
				int count, int *eof, void *data)
{
	unsigned long flags;
	
	local_irq_save(flags);
	printk("\neth_skb_free_num: \t%d\n", eth_skb_free_num);
	printk("\n");
	local_irq_restore(flags);

	return 0;
}

static int priv_eth_skb_write_proc(struct file *file, const char *buffer,	unsigned long count, void *data)
{
	return -EFAULT;
}

static void init_priv_eth_skb_proc(void)
{
	struct proc_dir_entry *entry=NULL;	
	entry = create_proc_entry("priv_eth_skb", 0, NULL);
	if (entry) {
		entry->read_proc = priv_eth_skb_read_proc;
		entry->write_proc = priv_eth_skb_write_proc;
	}
	else {
		printk("Realtek priv eth skb, create proc failed!\n");
	}
}
#endif //ETH_PRIV_SKB_PROC

void init_priv_eth_skb_buf(void)
{
	int i;

	memset(eth_skb_buf, '\0', sizeof(struct priv_skb_buf2)*MAX_ETH_SKB_NUM);
	INIT_LIST_HEAD(&eth_skbbuf_list);
	
	for (i=0; i<MAX_ETH_SKB_NUM; i++)  {
		memcpy(eth_skb_buf[i].magic, ETH_MAGIC_CODE, 4);	
		eth_skb_buf[i].buf_pointer = (unsigned long)&eth_skb_buf[i];
		INIT_LIST_HEAD(&eth_skb_buf[i].list);
		list_add_tail(&eth_skb_buf[i].list, &eth_skbbuf_list);	
        
	}

	//init rx_skb_queue
	memset(&rx_skb_queue, 0, sizeof(rx_skb_queue));
#ifdef ETH_PRIV_SKB_PROC
	init_priv_eth_skb_proc();
#endif
}


static __inline__ unsigned char *get_buf_from_poll(struct list_head *phead, unsigned int *count)
{
	unsigned long flags;
	unsigned char *buf;
	struct list_head *plist;

	local_irq_save(flags);
	
	if (list_empty(phead)) {
		local_irq_restore(flags);
//		DEBUG_ERR("eth_drv: phead=%X buf is empty now!\n", (unsigned int)phead);
		return NULL;
	}

	if (*count == 0) {
		local_irq_restore(flags);
//		DEBUG_ERR("eth_drv: phead=%X under-run!\n", (unsigned int)phead);
		return NULL;
	}

	*count = *count - 1;
	plist = phead->next;
	list_del_init(plist);
	buf = (unsigned char *)((unsigned int)plist + sizeof (struct list_head));
	local_irq_restore(flags);
	return buf;
}


static __inline__ void release_buf_to_poll(unsigned char *pbuf, struct list_head	*phead, unsigned int *count)
{
	unsigned long flags;
	struct list_head *plist;

	local_irq_save(flags);

	*count = *count + 1;
	plist = (struct list_head *)((unsigned int)pbuf - sizeof(struct list_head));
	list_add_tail(plist, phead);
	local_irq_restore(flags);
}


void free_rtl865x_eth_priv_buf(unsigned char *head)
{
	#ifdef DELAY_REFILL_ETH_RX_BUF
	extern int return_to_rx_pkthdr_ring(unsigned char *head);
#ifdef ETH_NEW_FC
	if (during_close || !return_to_rx_pkthdr_ring(head)) 
#else
	if (!return_to_rx_pkthdr_ring(head)) 
#endif
	#endif
		release_buf_to_poll(head, &eth_skbbuf_list, (unsigned int *)&eth_skb_free_num);	
}


struct sk_buff *dev_alloc_skb_priv_eth(unsigned int size)
{
	struct sk_buff *skb;

	/* first argument is not used */
	unsigned char *data = get_buf_from_poll(&eth_skbbuf_list, (unsigned int *)&eth_skb_free_num);
	if (data == NULL) {
//		DEBUG_ERR("eth_drv: priv skb buffer empty!\n");
		return NULL;
	}
	skb = dev_alloc_8190_skb(data, size);
	if (skb == NULL) {
		free_rtl865x_eth_priv_buf(data);
		return NULL;
	}
    
	return skb;
}


int is_rtl865x_eth_priv_buf(unsigned char *head)
{
	unsigned long offset = (unsigned long)(&((struct priv_skb_buf2 *)0)->buf);
	struct priv_skb_buf2 *priv_buf = (struct priv_skb_buf2 *)(((unsigned long)head) - offset);

	if (!memcmp(priv_buf->magic, ETH_MAGIC_CODE, 4) &&	
		(priv_buf->buf_pointer == (unsigned int)priv_buf)) {
		return 1;	
	}
	else {
		return 0;
	}
}
#ifdef CONFIG_WIRELESS_EXT
#if 0
#include <net/dst.h>
static void copy_skb_header(struct sk_buff *new, const struct sk_buff *old)
{
	/*
	 *	Shift between the two data areas in bytes
	 */
	unsigned long offset = new->data - old->data;

	//new->list=NULL;
	new->sk=NULL;
	new->dev=old->dev;
	new->priority=old->priority;
	new->protocol=old->protocol;
	new->dst=dst_clone(old->dst);
	new->h.raw=old->h.raw+offset;
	new->nh.raw=old->nh.raw+offset;
	new->mac.raw=old->mac.raw+offset;
	memcpy(new->cb, old->cb, sizeof(old->cb));
	atomic_set(&new->users, 1);
	new->pkt_type=old->pkt_type;
	//new->stamp=old->stamp;
	new->destructor = NULL;
	//new->security=old->security;
#ifdef CONFIG_NETFILTER
	new->nfmark=old->nfmark;
	//new->nfcache=old->nfcache;
	new->nfct=old->nfct;
	nf_conntrack_get(new->nfct);
#ifdef CONFIG_NETFILTER_DEBUG
	new->nf_debug=old->nf_debug;
#endif
#endif
#ifdef CONFIG_NET_SCHED
	new->tc_index = old->tc_index;
#endif
#ifdef CONFIG_RTK_VOIP_VLAN_ID
	new->rx_vlan=old->rx_vlan;
	new->rx_wlan=old->rx_wlan;
#endif
#ifdef CONFIG_RTK_VLAN_SUPPORT	
	new->tag.v = old->tag.v;
#endif
}
#endif

struct sk_buff *priv_skb_copy(struct sk_buff *skb)
{
	struct sk_buff *n;	

	if (rx_skb_queue.qlen == 0) {
#ifdef CONFIG_RTL865X_ETH_PRIV_SKB
		n = dev_alloc_skb_priv_eth(CROSS_LAN_MBUF_LEN);
#else        
		n  = dev_alloc_skb(CROSS_LAN_MBUF_LEN);
#endif
	}
	else {
#ifdef RTK_QUE
		n = rtk_dequeue(&rx_skb_queue);
#else
		n = __skb_dequeue(&rx_skb_queue);
#endif
	}
	
	if (n == NULL) 
		return NULL;

	/* Set the tail pointer and length */	
	skb_put(n, skb->len);	
	n->csum = skb->csum;	
	n->ip_summed = skb->ip_summed;	
	memcpy(n->data, skb->data, skb->len);

	copy_skb_header(n, skb);
	return n;
}
#endif // CONFIG_NET_RADIO

#endif // CONFIG_RTL865X_ETH_PRIV_SKB
