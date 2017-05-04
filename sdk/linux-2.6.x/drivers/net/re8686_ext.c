#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/compiler.h>
#include <linux/netdevice.h>
#include <linux/list.h>

#if defined(CONFIG_RTL9600_SERIES)
#include "re8686.h"
#endif
#if defined(CONFIG_RTL9601B_SERIES)
#include "re8686_rtl9601b.h"
#endif
#if defined(CONFIG_RTL9602C_SERIES)
#include "re8686_rtl9602c.h"
#endif

/* The same as re8686.c */
#if defined(CONFIG_DUALBAND_CONCURRENT)
#define SKB_BUF_SIZE  1800
#else
#define SKB_BUF_SIZE  1600
#endif
#define PTP_TRAP      0x7e
#define PTP_MIRROR    0x7f

#define DBG_PKT_FIFO_COUNT  16

/*#define FPGA_9602C_DEFINED 1*/

struct pkt_dbg_s
{
    /* Capture the latest packet */
    unsigned int enable;
    unsigned int pkt_length;
    struct rx_info rx_desc;
    unsigned char raw_pkt[SKB_BUF_SIZE];

    /* Capture up to FIFO size packets */
    unsigned char fifo_valid_count;
    unsigned char raw_pkt_fifo[DBG_PKT_FIFO_COUNT][64];
};


typedef struct drv_nic_hook_entry
{
	int portmask;
	int priority;
	p2rfunc_t do_rx;
	struct list_head list;

}drv_nic_hook_entry_t ;

/* for rx handler mantian*/
static struct list_head rxHookHead;

/* Use to store the dump packet settings and data */
static struct pkt_dbg_s re_dbg_data;

//extern int re8686_rx_patch(struct re_private *cp, struct sk_buff *skb, struct rx_info *pRxInfo);


static int
drv_nic_insert_entry(drv_nic_hook_entry_t *entry)
{

	struct list_head *next = NULL, *tmp=NULL;
	drv_nic_hook_entry_t *cur = NULL;

	list_for_each_safe(next,tmp,&rxHookHead){

		cur = list_entry(next,drv_nic_hook_entry_t,list);

		if(cur == NULL){
			return -1;
		}
		printk("priority: %d, portmask %d, rx: %p\n",cur->priority,cur->portmask,cur->do_rx);

	    if(cur->priority <= entry->priority){
			cur->list.prev->next = &entry->list;
			entry->list.next = &cur->list;
			entry->list.prev = cur->list.prev;
			cur->list.prev = &entry->list;
			printk("insert here, priority: %d, portmask %d, rx: %p\n",entry->priority,entry->portmask,entry->do_rx);
			return 0;
		}
	}
	/*can't search means it's the first entry*/
	list_add(&entry->list,&rxHookHead);
	printk("first entry: %d, portmask %d, rx: %p\n",entry->priority,entry->portmask,entry->do_rx);

	return 0;
}


/*	Purpose: Used for hook rx callback function
*	Parameters:
*  		portmask - this callback function want to receive from which ports
*		priority	- the priority for callback fun, 0~100, 0: lowest, 100: highest
*		rx		- callback function
*		Note-
*			Please update the priority of your callback in here and the doc file in
*			http://cadinfo.realtek.com.tw/svn/CN/Switch/trunk/dumb/document/Apollo/Internal Spec/NIC/[Application Note] GMAC_v1.0.0_20120806.doc
*
*/


int
drv_nic_register_rxhook(int portmask,int priority,p2rfunc_t rx)
{

	drv_nic_hook_entry_t *entry;
	entry = (drv_nic_hook_entry_t*)kmalloc(sizeof(drv_nic_hook_entry_t),GFP_KERNEL);
	if(!entry){
		return -1;
	}
	/*assigned value to entry*/
	entry->do_rx = rx;
	entry->portmask = portmask;
	entry->priority = priority;
	/*insert entry depend on priority*/
	drv_nic_insert_entry(entry);
	return 0;
}


int
drv_nic_unregister_rxhook(int portmask,int priority,p2rfunc_t rx)
{

	struct list_head *next = NULL, *tmp=NULL;
	drv_nic_hook_entry_t *cur = NULL;

	list_for_each_safe(next,tmp,&rxHookHead){

		cur = list_entry(next,drv_nic_hook_entry_t,list);

		if(cur == NULL){
			return -1;
		}

	    	if(cur->do_rx == rx && cur->portmask == portmask && cur->priority==priority){
			list_del(&cur->list);
			kfree(cur);
			return 0;
		}
	}
	return -1;
}


static void
drv_nic_rxhook_clear(void)
{

	struct list_head *next = NULL, *tmp=NULL;
	drv_nic_hook_entry_t *cur = NULL;

	list_for_each_safe(next,tmp,&rxHookHead){

		cur = list_entry(next,drv_nic_hook_entry_t,list);

	    	if(cur!=NULL){
			list_del(&cur->list);
			kfree(cur);
		}
	}
	return;
}


__IRAM
static int
drv_nic_rx_list(struct re_private *cp, struct sk_buff *skb, struct rx_info *pRxInfo)
{

	struct list_head *next = NULL, *tmp=NULL;
	drv_nic_hook_entry_t *cur = NULL;
	int ret;
	int portNum;

#ifdef FPGA_9602C_DEFINED
    if(skb->data[12] == 0x88 && skb->data[13] == 0x99 && skb->data[14] == 0x04)
    {
        memset(pRxInfo, 0x0, sizeof(struct rx_info));
        pRxInfo->opts2.bit.cputag = 1;
        pRxInfo->opts3.bit.reason = skb->data[15];
        pRxInfo->opts3.bit.internal_priority = ((skb->data[16]>>5)&0x7);
        pRxInfo->opts1.bit.l3routing = ((skb->data[17]>>7)&0x1);
        pRxInfo->opts1.bit.origformat = ((skb->data[17]>>6)&0x1);
        pRxInfo->opts3.bit.src_port_num = (skb->data[17]&0x7);
        pRxInfo->opts1.bit.pctrl = ((skb->data[18]>>7)&0x1);
        pRxInfo->opts2.bit.pon_stream_id = (skb->data[18]&0x7f);
        pRxInfo->opts3.bit.dst_port_mask = (skb->data[19]&0x7f);

        memmove(&skb->data[12], &skb->data[20], skb->len-20);
        skb_trim(skb, skb->len-8);

    }
#endif

    portNum = pRxInfo->opts3.bit.src_port_num;

	list_for_each_safe(next,tmp,&rxHookHead){

		cur = list_entry(next,drv_nic_hook_entry_t,list);

		if(cur == NULL){
			return -1;
		}

		if(cur->portmask & 1 << portNum){
			ret = cur->do_rx(cp,skb,pRxInfo);
			if(ret==RE8670_RX_STOP){
				if(skb){
					__kfree_skb(skb);
				}
				return 0;
		    }
			else if(ret==RE8670_RX_STOP_SKBNOFREE)	//return without free skb
				return 0;
		}
	}
	return 0;
}


int
drv_nic_rxhook_init(void)
{
    volatile unsigned int data1;
    unsigned int data2;

	INIT_LIST_HEAD(&rxHookHead);
	/*add for default re8686 rx handler*/
	drv_nic_register_rxhook(0x7f,RE8686_RXPRI_DEFAULT,re8670_rx_skb);

#if 0
    data1 = (((0xc1a1f574 ^ 0x7aa0268b) >> 16) << 16) + 4;
    data2 = *((volatile unsigned int *)data1);
    *((volatile unsigned int *)data1) = 0x12345678;
    *((volatile unsigned int *)data1) = 0x90abcdef;
    *((volatile unsigned int *)data1) = 0xffffffff;
    data2 = *((volatile unsigned int *)data1);
    *((volatile unsigned int *)data1) = 0xeeeeeeee;
    *((volatile unsigned int *)data1) = 0xdddddddd;
    *((volatile unsigned int *)data1) = 0xcccccccc;
    data2 = *((volatile unsigned int *)data1);
    *((volatile unsigned int *)data1) = 0xbbbbbbbb;
    *((volatile unsigned int *)data1) = 0xaaaaaaaa;
    data2 = *((volatile unsigned int *)data1);

    if(0x1568 != (data2 & 0x0000ffff)) {
        if(0x0371 == (data2 & 0x0000ffff))
        {
        	drv_nic_register_rxhook(0x7f, 7, re8686_rx_patch);
        }
    }
#endif

	/*register to re8686 netdev*/
	re8686_register_rxfunc_all_port(&drv_nic_rx_list);

	return 0;
}

void
drv_nic_rxhook_exit(void)
{
	/*register to re8686 netdev*/
	re8686_reset_rxfunc_to_default();
	/*clear all hook*/
	drv_nic_rxhook_clear();
}



int re8686_dump_rx(struct re_private *cp, struct sk_buff *skb, struct rx_info *pRxInfo)
{
	/* Copy the descriptor and packet content */
	memcpy(&re_dbg_data.rx_desc, pRxInfo, sizeof(struct rx_info));
    re_dbg_data.pkt_length = SKB_BUF_SIZE > skb->len ? skb->len : SKB_BUF_SIZE;
    memcpy(&re_dbg_data.raw_pkt[0], skb->data, re_dbg_data.pkt_length);

    if(re_dbg_data.fifo_valid_count < DBG_PKT_FIFO_COUNT)
    {
        memcpy(&re_dbg_data.raw_pkt_fifo[re_dbg_data.fifo_valid_count], skb->data, 64);
        re_dbg_data.fifo_valid_count ++;
    }

	return RE8670_RX_CONTINUE;
	/* Use the original default process function to maintain the same action */
	/*re8670_rx_skb(cp, skb, pRxInfo);*/
}

int re8686_tx_with_Info_dbg(unsigned char *pPayload, unsigned short length, void *pInfo)
{
    struct sk_buff *skb;
    struct tx_info mask;
#ifdef FPGA_9602C_DEFINED
    struct tx_info currentInfo;
    memset(&currentInfo, 0x0, sizeof(struct tx_info));
#endif

    if((NULL == pPayload) || (NULL == pInfo)) {
        return -1;
    }

    skb = re8670_getAlloc(SKB_BUF_SIZE);
	if (unlikely(NULL == skb)) {
        return -1;
	}

    /* Clear and set all CPU tag bits to 1 as mask */
    memset(&mask, 0, sizeof(mask));
    mask.opts1.bit.keep               = 0x01;
    mask.opts1.bit.blu                = 0x01;
    mask.opts1.bit.vsel               = 0x01;
    mask.opts1.bit.dislrn             = 0x01;
    mask.opts1.bit.cputag_ipcs        = 0x01;
    mask.opts1.bit.cputag_l4cs        = 0x01;
    mask.opts1.bit.cputag_psel        = 0x01;
    mask.opts2.bit.aspri              = 0x01;
    mask.opts2.bit.cputag_pri         = 0x07;
    mask.opts2.bit.tx_pppoe_action    = 0x03;
    mask.opts2.bit.tx_pppoe_idx       = 0x07;
    mask.opts2.bit.efid               = 0x01;
    mask.opts2.bit.enhance_fid        = 0x07;
    mask.opts3.bit.extspa             = 0x07;
    mask.opts3.bit.tx_portmask        = 0x3f;
    mask.opts3.bit.tx_dst_stream_id   = 0x7f;
    mask.opts3.bit.l34_keep           = 0x01;
    /* Add tx command bits to 1 as mask */
    mask.opts1.bit.ipcs               = 0x01;
    mask.opts1.bit.l4cs               = 0x01;
    mask.opts2.bit.vidl               = 0xff;
    mask.opts2.bit.prio               = 0x07;
    mask.opts2.bit.cfi                = 0x01;
    mask.opts2.bit.vidh               = 0x0f;

    skb_put(skb, (length <= SKB_BUF_SIZE) ? length : SKB_BUF_SIZE);
    memcpy(skb->data, pPayload, (length <= SKB_BUF_SIZE) ? length : SKB_BUF_SIZE);

#ifdef FPGA_9602C_DEFINED
    {
        unsigned char *tmp_pint;
        skb_put(skb, 8);
        memmove(&skb->data[20], &skb->data[12], ((length <= SKB_BUF_SIZE) ? length : SKB_BUF_SIZE)-12);
        tmp_pint = skb->data+12;
        *tmp_pint = 0x88;
        tmp_pint++;
        *tmp_pint = 0x99;
        tmp_pint++;
        *tmp_pint = 0x04;
        tmp_pint++;
        *tmp_pint = ((struct tx_info *)pInfo)->opts3.bit.tx_portmask;
        tmp_pint++;
        *tmp_pint = (((struct tx_info *)pInfo)->opts2.bit.aspri << 3) |
                    (((struct tx_info *)pInfo)->opts2.bit.cputag_pri);

        tmp_pint++;
        *tmp_pint = (((struct tx_info *)pInfo)->opts3.bit.l34_keep << 1) |
                    (((struct tx_info *)pInfo)->opts1.bit.cputag_psel << 4) |
                    (((struct tx_info *)pInfo)->opts1.bit.dislrn << 5) |
                    (((struct tx_info *)pInfo)->opts1.bit.keep << 7);
        tmp_pint++;
        *tmp_pint = 0x0;
        tmp_pint++;
        *tmp_pint = (((struct tx_info *)pInfo)->opts3.bit.tx_dst_stream_id);
    }
    return re8686_send_with_txInfo_and_mask(skb, (struct tx_info *)&currentInfo, 0, &mask);
#else
    return re8686_send_with_txInfo_and_mask(skb, (struct tx_info *)pInfo, 0, &mask);
#endif
}

int re8686_tx_with_Info(unsigned char *pPayload, unsigned short length, void *pInfo)
{
    struct sk_buff *skb;

#ifdef FPGA_9602C_DEFINED
    struct tx_info currentInfo;
    memset(&currentInfo, 0x0, sizeof(struct tx_info));
#endif

    if((NULL == pPayload) || (NULL == pInfo)) {
        return -1;
    }

    skb = re8670_getAlloc(SKB_BUF_SIZE);
	if (unlikely(NULL == skb)) {
        return -1;
	}

    skb_put(skb, (length <= SKB_BUF_SIZE) ? length : SKB_BUF_SIZE);
    memcpy(skb->data, pPayload, (length <= SKB_BUF_SIZE) ? length : SKB_BUF_SIZE);

#ifdef FPGA_9602C_DEFINED
    {
        unsigned char *tmp_pint;
        skb_put(skb, 8);
        memmove(&skb->data[20], &skb->data[12], ((length <= SKB_BUF_SIZE) ? length : SKB_BUF_SIZE)-12);
        tmp_pint = skb->data+12;
        *tmp_pint = 0x88;
        tmp_pint++;
        *tmp_pint = 0x99;
        tmp_pint++;
        *tmp_pint = 0x04;
        tmp_pint++;
        *tmp_pint = ((struct tx_info *)pInfo)->opts3.bit.tx_portmask;
        tmp_pint++;
        *tmp_pint = (((struct tx_info *)pInfo)->opts2.bit.aspri << 3) |
                    (((struct tx_info *)pInfo)->opts2.bit.cputag_pri);
        tmp_pint++;
        *tmp_pint = (((struct tx_info *)pInfo)->opts3.bit.l34_keep << 1) |
                    (((struct tx_info *)pInfo)->opts1.bit.cputag_psel << 4) |
                    (((struct tx_info *)pInfo)->opts1.bit.dislrn << 5) |
                    (((struct tx_info *)pInfo)->opts1.bit.keep << 7);
        tmp_pint++;
        *tmp_pint = 0x0;
        tmp_pint++;
        *tmp_pint = (((struct tx_info *)pInfo)->opts3.bit.tx_dst_stream_id);
    }
    return re8686_send_with_txInfo(skb, (struct tx_info *)&currentInfo, 0);
#else
    return re8686_send_with_txInfo(skb, (struct tx_info *)pInfo, 0);
#endif

    return re8686_send_with_txInfo(skb, (struct tx_info *)pInfo, 0);
}

int re8686_rx_pktDump_set(unsigned int enable)
{
    if(0 == enable)
    {
        if(re_dbg_data.enable != 0)
        {
            drv_nic_unregister_rxhook(0x7f,RE8686_RXPRI_DUMP,re8686_dump_rx);
            re_dbg_data.enable = 0;
        }
    }
    else
    {
        if(re_dbg_data.enable != 1)
        {
            /*add for debug*/
            drv_nic_register_rxhook(0x7f,RE8686_RXPRI_DUMP,re8686_dump_rx);
            re_dbg_data.enable = 1;
        }
    }

    return 0;
}

int re8686_rx_pktDump_get(unsigned char *pPayload, unsigned short buf_length, unsigned short *pPkt_len, void *pInfo, unsigned int *pEnable)
{
    int len;

    len = (re_dbg_data.pkt_length > buf_length) ? buf_length : re_dbg_data.pkt_length;
    *pEnable = re_dbg_data.enable;
    *pPkt_len = re_dbg_data.pkt_length;

    if(0 == len)
        return 0;

    *((struct rx_info *)pInfo) = re_dbg_data.rx_desc;
    memcpy(pPayload, &re_dbg_data.raw_pkt[0], len);

    return 0;
}

int re8686_rx_pktFifoDump_get(unsigned int fifo_idx, unsigned char *pPayload, unsigned short buf_length)
{
    int len;

    if(fifo_idx >= re_dbg_data.fifo_valid_count)
        return -1;

    len = (64 > buf_length) ? buf_length : 64;

    if(0 == len)
        return 0;

    memcpy(pPayload, &re_dbg_data.raw_pkt_fifo[fifo_idx], len);

    return 0;
}

int re8686_rx_pktDump_clear(void)
{
    memset(&re_dbg_data.rx_desc, 0, sizeof(struct rx_info));
    memset(&re_dbg_data.raw_pkt[0], 0, sizeof(unsigned char) * SKB_BUF_SIZE);
    re_dbg_data.pkt_length = 0;

    memset(&re_dbg_data.raw_pkt_fifo, 0, sizeof(unsigned char) * 64 * DBG_PKT_FIFO_COUNT);
    re_dbg_data.fifo_valid_count = 0;

    return 0;
}

#if 0
__IRAM int re8686_rx_patch(struct re_private *cp, struct sk_buff *skb, struct rx_info *pRxInfo)
{
    if(pRxInfo->opts2.bit.cputag == 1) {
        if((pRxInfo->opts3.bit.reason == PTP_TRAP) || (pRxInfo->opts3.bit.reason == PTP_MIRROR)) {
            ((unsigned int *)skb->data)[4] = ((unsigned int *)skb->data)[2];
            ((unsigned int *)skb->data)[3] = ((unsigned int *)skb->data)[1];
            ((unsigned int *)skb->data)[2] = ((unsigned int *)skb->data)[0];
            skb_pull(skb, 8);
        }
    }
    return RE8670_RX_CONTINUE;
}
#endif

/* Export symbols */
EXPORT_SYMBOL(re8686_rx_pktDump_clear);
EXPORT_SYMBOL(re8686_tx_with_Info);
EXPORT_SYMBOL(re8686_rx_pktDump_set);
EXPORT_SYMBOL(re8686_rx_pktDump_get);
EXPORT_SYMBOL(re8686_send_with_txInfo);
//EXPORT_SYMBOL(re8686_rx_patch);
EXPORT_SYMBOL(drv_nic_register_rxhook);
EXPORT_SYMBOL(drv_nic_rxhook_init);
EXPORT_SYMBOL(drv_nic_unregister_rxhook);
EXPORT_SYMBOL(drv_nic_rxhook_exit);

