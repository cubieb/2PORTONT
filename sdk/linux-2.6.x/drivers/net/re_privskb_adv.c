/*	
 *  re_privskb_adv.c: Ethernet Private Skb Management driver
 *  Date: 2013/11/27
 *  Auther: QL DSL team
 */

#ifdef CONFIG_RTL865X_ETH_PRIV_SKB_ADV
#include <linux/skbuff.h>
#include <linux/interrupt.h>
#include <linux/cache.h>
#include "re_privskb_adv.h"


int eth_skb_free_num=MAX_ETH_SKB_NUM;
int eth_buf_free_num=MAX_ETH_SKB_NUM;

struct priv_skb_buf3 {
	struct priv_skb_buf3 *next;
	unsigned char buf[0];
};
static unsigned char eth_skb_hdr[(MAX_ETH_SKB_NUM * SKB_ALIGNED_SIZE) + 16];
static unsigned char eth_skb_buf[(MAX_ETH_SKB_NUM * ETH_SKB_BUF_SIZE) + SMP_CACHE_BYTES];
static struct sk_buff * freeSkbList=NULL;
static struct priv_skb_buf3 *freeBufList=NULL;

extern struct sk_buff *dev_alloc_8190_skb(unsigned char *data, int size);

#ifdef ETH_PRIV_SKB_PROC
#include <linux/proc_fs.h>
static int priv_eth_skb_read_proc(char *page, char **start, off_t off,
				int count, int *eof, void *data)
{
	unsigned long flags;
	
	local_irq_save(flags);
	printk("\neth_skb_free_num: \t%d\neth_buf_free_num: \t%d\n", eth_skb_free_num, eth_buf_free_num);
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

void rtl865x_free_eth_priv_buf(struct sk_buff *skb, unsigned int free_flag)
{
	struct priv_skb_buf3 *list;
	unsigned long flags;

	//printk("%s prealloc_flags 0x%x free_flag:0x%x\n", __func__, skb->prealloc_flags, free_flag);
	local_irq_save(flags);
	if (free_flag & RETFREEQ_DATA) {
		list = (struct priv_skb_buf3 *)((unsigned int)skb->head - offsetof(struct priv_skb_buf3, buf));
		list->next = freeBufList;
		freeBufList = list;

		eth_buf_free_num++;
	}
	
	if (free_flag & RETFREEQ_SKB){
		skb->prealloc_next = freeSkbList;
		freeSkbList = skb;

		eth_skb_free_num++;
	}
	local_irq_restore(flags);
}

void init_priv_eth_skb_buf(void)
{
	unsigned char *skb;
	unsigned char *data;
	int i;

	//printk("\n\n%s enter\n", __func__);
	/* XXXX: SMP_CACHE_BYTES must larger than 16*/
	for(i=0, data = 
        (unsigned char *)(SKB_DATA_ALIGN((unsigned long)eth_skb_buf)); i<MAX_ETH_SKB_NUM;
		i++, data += ETH_SKB_BUF_SIZE)
	{
		((struct priv_skb_buf3 *)data)->next = freeBufList;
		freeBufList = (struct priv_skb_buf3 *)data;
		//printk("freeBufList=0x%x next=0x%x\n", (unsigned int)freeBufList, (unsigned int)freeBufList->next);
	}
	//printk("-------------------------\n");
	
	for(i = 0, skb = (unsigned char *)
        (((unsigned long)eth_skb_hdr+0xF) & ~0xF); i < MAX_ETH_SKB_NUM;
        i++, skb += SKB_ALIGNED_SIZE)
    {
        ((struct sk_buff * )skb)->prealloc_next = freeSkbList;
        freeSkbList = (struct sk_buff *) skb;
		//printk("freeSkbList=0x%x next=0x%x\n", (unsigned int)freeSkbList, (unsigned int)freeSkbList->prealloc_next);
    }
#ifdef ETH_PRIV_SKB_PROC
        init_priv_eth_skb_proc();
#endif

}

struct sk_buff *dev_alloc_skb_priv_eth(unsigned int size)
{
	struct sk_buff *skb=NULL;
	unsigned char *data;
	unsigned long flags;

	local_irq_save(flags);
	if ((NULL != freeSkbList) && (NULL != freeBufList))
	{
		data = freeBufList->buf;
		freeBufList = freeBufList->next;
		
		skb = freeSkbList;
		freeSkbList = freeSkbList->prealloc_next;

		eth_buf_free_num--;
		eth_skb_free_num--;
		local_irq_restore(flags);

		//printk("%s skb:0x%x data:0x%x next=0x%x\n", __func__, (unsigned int)skb, (unsigned int)data, (unsigned int)freeBufList);

		atomic_set(&skb->users, 1);
		/* cloned must be initialized here. */
		skb->cloned = 0;
		skb->head = data;
		skb->data = data + NET_SKB_PAD;
		skb->len = 0;
		skb->tail = skb->data;

		size = (size+128+NET_SKB_PAD);
		
		skb->end  = skb->head + size;
		skb->truesize = size + sizeof(struct sk_buff);
		//shinfo = skb_shinfo(skb);
		//atomic_set(&shinfo->dataref, 1);
		//shinfo->nr_frags  = 0;
		//shinfo->gso_size = 0;
		//shinfo->gso_segs = 0;
		//shinfo->gso_type = 0;
		//shinfo->ip6_frag_id = 0;
		//shinfo->frag_list = NULL;
		
		//skb->src_port = IF_SWITCH;
		skb->prealloc_flags = RETFREEQ_FF;
		skb->prealloc_cb = rtl865x_free_eth_priv_buf;

		return skb;
	}
	local_irq_restore(flags);

	if (NULL == freeSkbList)
		printk("%s freeSkbList is NULL\n", __func__);
	if (NULL == freeBufList)
		printk("%s freeBufList is NULL\n", __func__);
	return NULL;
}

#ifdef CONFIG_WIRELESS_EXT
struct sk_buff *priv_skb_copy(struct sk_buff *skb)
{
	struct sk_buff *n;	

#ifdef CONFIG_RTL865X_ETH_PRIV_SKB
	n = dev_alloc_skb_priv_eth(CROSS_LAN_MBUF_LEN);
#else        
	n = dev_alloc_skb(CROSS_LAN_MBUF_LEN);
#endif
	
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
#endif // CONFIG_WIRELESS_EXT

#endif // CONFIG_RTL865X_ETH_PRIV_SKB_ADV
