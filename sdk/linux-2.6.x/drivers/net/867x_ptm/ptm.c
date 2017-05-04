/* 	ptm_v1.c: 

	1. A Linux PTM driver for the RealTek VDSL chips. 
	2. The release version v1
*/

#define DRV_NAME		"RTL8681_PTM"
#define DRV_VERSION		"1.0.0"
#define DRV_RELDATE		"May 23, 2012"


/* Include header files */
#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/compiler.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/ethtool.h>
#include <linux/mii.h>
#include <linux/if_vlan.h>
#include <linux/crc32.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/slab.h>

#ifdef CONFIG_RTL8672	
#include <bspchip.h>
#include <linux/version.h>
#include "../../arch/rlx/bsp/gpio.h"
#endif

/*PTM registers' definitions*/
#include "ptm_regs.h"

/* ROMPERF related definitions */
#ifdef CONFIG_PTM_ROMPERF
#include "ptm_romperf.h"
#endif

/* These identify the driver base version and may not be removed. */
static char version[] __devinitdata = 
KERN_INFO DRV_NAME " PTM driver v" DRV_VERSION " (" DRV_RELDATE ")\n";

MODULE_AUTHOR("Tse-Yao Chang <czyao@realtek.com.tw>");
MODULE_DESCRIPTION("RealTek RTL-8681 series PTM driver");
MODULE_LICENSE("GPL");

/*------------------------------ Definition------------------------------ */
#define PTMBASE				0xB8400000  
#define SARCHEM_BASE		0xB8A80000 

#define PTM_RX_RING_SIZE	64//16~256 descriptors
#define PTM_TX_RING_SIZE		512

/* Time in jiffies before concluding the transmitter is hung. */
#define TX_TIMEOUT			(6*HZ)

/* Size of each temporary Rx buffer*/
#define PKT_BUF_SZ			1536

/* Rx alignment length */
#define RX_OFFSET			2
#define RX_LEN_MASK			0x7ff

#define DESC_ALIGN			0x100
#define UNCACHE_MASK		0xa0000000
#define PHYADDR_MASK		0x1fffffff

#define PTM_RXRING_BYTES			( (sizeof(struct dma_desc) * (PTM_RX_RING_SIZE+1)) + DESC_ALIGN)
#define PTM_TXRING_BYTES			( (sizeof(struct dma_desc) * (PTM_TX_RING_SIZE+1)) + DESC_ALIGN)


#define RTL_W32(reg, value)			(*(volatile u32*)(PTMBASE+reg)) = (u32)value
#define RTL_W16(reg, value)			(*(volatile u16*)(PTMBASE+reg)) = (u16)value
#define RTL_W8(reg, value)				(*(volatile u8*)(PTMBASE+reg)) = (u8)value
#define RTL_R32(reg)					(*(volatile u32*)(PTMBASE+reg))
#define RTL_R16(reg)					(*(volatile u16*)(PTMBASE+reg))
#define RTL_R8(reg)					(*(volatile u8*)(PTMBASE+reg))

#define PTM_SRAM_USAGE

#if defined(CONFIG_SKB_POOL_PREALLOC) && defined(CONFIG_RTL867X_PACKET_PROCESSOR)
#define SKB_BUF_SIZE  	CONFIG_RTL867X_PREALLOCATE_SKB_SIZE
#else
#define SKB_BUF_SIZE  	1600
#endif

#define NEXT_TX(N)			(((N) + 1) & (PTM_TX_RING_SIZE - 1))
#define NEXT_RX(N)			(((N) + 1) & (PTM_RX_RING_SIZE - 1))

/* CRC length */
#define _CRCLNG_			4

/* IOCTL definition */
#define PTM_TX_TEST		1
#define PTM_RX_TEST		2
#define PTM_RESET		3
#define PTM_SET_TXPATH	4
#define PTM_DEBUG_EN	7	

/*------------------------------ Structure------------------------------ */
/* Descriptor structure */
typedef struct dma_desc {
	u32		opts1;
	u32		addr;
	u32		opts2;
	u32		opts3;
}DMA_DESC;

/*Ring information structure */
struct ring_info {
	struct sk_buff		*skb;
	dma_addr_t		mapping;
	unsigned			frag;
};

/* Statstic information structure */
struct cp_extra_stats {
	unsigned long		rx_frags;
	unsigned long 		tx_timeouts;
	unsigned long 		tx_cnt;
	unsigned long		tdu_int_cnt[8];
	unsigned long		rxovf_int_cnt[8];
	unsigned long		rdu_int_cnt[8];
	unsigned long		rok_int_cnt[8];
	unsigned int		tx_no_desc;
	unsigned int		tx_dma_cnt;
	unsigned int 		rx_no_mem;
	unsigned int		rx_frag_pkt;
	unsigned int		rx_runt_pkt;
	unsigned int		rx_crcerr_pkt;
};

struct ptm_private {

	unsigned		tx_fdpf_head[4];
	unsigned		tx_fdps_head[4];
	unsigned		tx_fdpf_tail[4];
	unsigned		tx_fdps_tail[4];

	unsigned			rx_tail[4];
	unsigned int		sts_reg;	//for recording PTM int status

	void			*regs;
	struct net_device	*dev;
	spinlock_t		lock;

	u32				msg_enable;

	/* TX Desc information */
	DMA_DESC		*tx_fdpf_ring[4];
	DMA_DESC		*tx_fdps_ring[4];
	
	/* RX Desc information */
	DMA_DESC		*rx_hsring; 
	DMA_DESC		*rx_lsring; 
	DMA_DESC		*rx_hfring; 
	DMA_DESC		*rx_lfring; 	

	/* TX Ring information */
	struct ring_info	tx_fdpf_skb[4][PTM_TX_RING_SIZE];
	struct ring_info	tx_fdps_skb[4][PTM_TX_RING_SIZE];
	
	/* RX Ring information */
	struct ring_info	rx_hs_skb[PTM_RX_RING_SIZE];
	struct ring_info	rx_ls_skb[PTM_RX_RING_SIZE];
	struct ring_info	rx_hf_skb[PTM_RX_RING_SIZE];
	struct ring_info	rx_lf_skb[PTM_RX_RING_SIZE];	

	unsigned			rx_buf_sz;
	dma_addr_t		ring_dma;

	struct net_device_stats net_stats;
	struct cp_extra_stats	cp_stats;

	/* TX Ring buffer */
	char				txdesc_buf_f[4];
	char				txdesc_buf_s[4];
	/* RX Ring buffer */
	char*			rxdesc_buf_hs;
	char*			rxdesc_buf_ls;
	char*			rxdesc_buf_hf;
	char*			rxdesc_buf_lf;

	struct tasklet_struct rx_tasklets;
};

/* IOCTL structure */
struct eth_arg{
	unsigned char cmd;
	unsigned int cmd2;
	unsigned int cmd3;
	unsigned int cmd4;
};


/*------------------------------ Global variable ------------------------------ */
static struct net_device *ptm_net_dev;

/* PTM module enable or diable signal */
static unsigned char ptm_disable=0;

/* PTM interrupt mask */
unsigned int ptm_intr_mask =
 	   ROK_H_S | RUNT_H_S | RDU_H_S | RER_OVF_H_S 
	| ROK_L_S | RUNT_L_S | RDU_L_S | RER_OVF_L_S 
	| ROK_H_F | RUNT_H_F | RDU_H_F | RER_OVF_H_F 
	| ROK_L_F | RUNT_L_F | RDU_L_F | RER_OVF_L_F
	| TDU_CH3_S | TDU_CH2_S | TDU_CH1_S | TDU_CH0_S
	| TDU_CH3_F | TDU_CH2_F | TDU_CH1_F | TDU_CH0_F 
	| TOK_CH3_S | TOK_CH2_S | TOK_CH1_S | TOK_CH0_S
	| TOK_CH3_F | TOK_CH2_F | TOK_CH1_F | TOK_CH0_F;

unsigned int check_interrupts =
	ROK_H_S | RUNT_H_S | RDU_H_S | RER_OVF_H_S 
	| ROK_L_S | RUNT_L_S | RDU_L_S | RER_OVF_L_S 
	| ROK_H_F | RUNT_H_F | RDU_H_F | RER_OVF_H_F 
	| ROK_L_F | RUNT_L_F | RDU_L_F | RER_OVF_L_F;


static unsigned int ptm_iocmd_reg=CMD_CONFIG;

static int multicast_filter_limit = 32;

/* Use for PTM debug */
unsigned int ptm_debug=0;

/* xDSL related signal from DSP lib */
/* enable_mode :		0 => ATM mode ,1 => PTM mode
     enable_phy_role: 	0 => master ,1 => slave ,2 => slave ,3 => slave */
extern unsigned int enable_mode, enable_phy_role;
extern unsigned char adslup;

/* PTM Tx channel infor */
short ptm_tx_fast=1, ptm_tx_channel=0;

/* PTM check Tx tasklet */
struct tasklet_struct ptm_checktxdesc_tasklets;

/* PTM test skb */
static struct sk_buff  testskb1;
static unsigned char ptm_testskbdata1[SKB_BUF_SIZE];


/*-------------------------------- Function--------------------------------- */

static int (*my_eth_mac_addr)(struct net_device *, void *);
static void ptm_clean_rings (struct ptm_private *cp);
static void __ptm_set_rx_mode (struct net_device *dev);

void Dumpreg(unsigned long Buffer, unsigned int size){

	int	k;	
	if(size%4)	size=size+4-(size%4);

	
	if ((Buffer&0xF0000000)==0x80000000) Buffer |= 0xA0000000;
	printk("Address  : Data");
	for(k=0;k<(size/4);k++){
		if ((k%4)==0) {
			printk ("\n");
			printk("%08X : ",(unsigned int)Buffer+k*4);
		}
		//mdelay(100);
		printk("%08X  ", (*(volatile unsigned int *)((unsigned int)((Buffer)+k*4))));
	}
	printk("\n");
}

/*
	Description : Dump specific memory content
	Input	  :  start address of memory, size of content, the label of this content
	Output	  :  none
*/
void memDump (void *start, u32 size, char * strHeader)
{
	int row, column, index, index2, max;
	u8 *buf, *line, ascii[17];
	char empty = ' ';

	if(!start ||(size==0))
		return;
	line = (u8*)start;

	/* 16 bytes per line */
	if (strHeader)
		printk("%s", strHeader);
	column = size % 16;
	row = (size / 16) + 1;
	for (index = 0; index < row; index++, line += 16) 
	{
		buf = line;

		memset (ascii, 0, 17);

		max = (index == row - 1) ? column : 16;
		if ( max==0 ) break; /* If we need not dump this line, break it. */

		printk("\n%08x ", (u32) line);
		
		//Hex
		for (index2 = 0; index2 < max; index2++)
		{
			if (index2 == 8)
			printk("  ");
			printk("%02x ", (u8) buf[index2]);
			ascii[index2] = ((u8) buf[index2] < 32) ? empty : buf[index2];
		}

		if (max != 16)
		{
			if (max < 8)
				printk("  ");
			for (index2 = 16 - max; index2 > 0; index2--)
				printk("   ");
		}

		//ASCII
		printk("  %s", ascii);
	}
	printk("\n");
	return;
}


/*
	Description : Get SKB for PTM
	Input	  :  none
	Output	  :  return skb 
*/
struct sk_buff *ptm_getAlloc(void)  
{	
	struct sk_buff *skb=NULL;

	skb = dev_alloc_skb(SKB_BUF_SIZE);

	if(skb==NULL){
		printk("Cannot allocate skb, not enough memory\n");
	}
	return skb;
}

/*
	Description : Set PTM Rx buffer size
	Input	  :  ptm_private structure
	Output	  :  none
*/
static inline void ptm_set_rxbufsize (struct ptm_private *cp)
{
	unsigned int mtu = cp->dev->mtu;
	
	if (mtu > ETH_DATA_LEN)
		/* MTU + ethernet header + FCS + optional VLAN tag */
		cp->rx_buf_sz = mtu + ETH_HLEN + 8;
	else
		cp->rx_buf_sz = PKT_BUF_SZ;
}

/*
	Description : receive skb and update related counters, assign protocol, call netif_rx
	Input	  :  ptm_private structure, sk_buff structure, descriptors
	Output	  :  none
*/
static inline void ptm_rx_skb (struct ptm_private *cp, struct sk_buff *skb,
			      DMA_DESC *desc)
{
	skb->dev = ptm_net_dev;
	skb->from_dev=skb->dev;

	cp->net_stats.rx_bytes += skb->len;
	skb->protocol = eth_type_trans (skb, cp->dev);

	cp->net_stats.rx_packets++;
	cp->dev->last_rx = jiffies;

	netif_rx(skb);

}

/*
	Description : Accumulate the RX error counters
	Input	  :  ptm_private structure, rx_tail, status of RX, length
	Output	  :  none
*/
static void ptm_rx_err_acct (struct ptm_private *cp, unsigned rx_tail,
			    u32 status, u32 len)
{
	if (netif_msg_rx_err (cp))
		printk (KERN_DEBUG"%s: rx err, slot %d status 0x%x len %d\n",cp->dev->name, rx_tail, status, len);
	
	cp->net_stats.rx_errors++;
	if (status & RxErrFrame){
		cp->net_stats.rx_frame_errors++;
		cp->cp_stats.rx_frag_pkt++;
	}
	if (status & RxErrCRC){
		cp->net_stats.rx_frame_errors++;
		cp->cp_stats.rx_crcerr_pkt++;
	}	
	if (status & RxErrRunt){
		cp->net_stats.rx_length_errors++;
		cp->cp_stats.rx_runt_pkt++;
	}
}


/*
	Description : Calculate the avaliable descriptors' number in TX ring
	Input	  :  ptm_private structure, indicate the PTM TX channel
	Output	  :  the avaliable TX descriptor number
*/
int TX_BUFFS_AVAIL(struct ptm_private *cp, int channel, int fast)
{
	int ret;
	if (fast)
	{
		if (cp->tx_fdpf_tail[channel] <= cp->tx_fdpf_head[channel])
		{
			ret = (cp->tx_fdpf_tail[channel]+(PTM_TX_RING_SIZE-1) - cp->tx_fdpf_head[channel]);
		}
		else
		{
			ret = cp->tx_fdpf_tail[channel] - cp->tx_fdpf_head[channel] - 1;
					}
	}
	else
	{
		if (cp->tx_fdps_tail[channel] <= cp->tx_fdps_head[channel])
		{
			ret = (cp->tx_fdps_tail[channel]+(PTM_TX_RING_SIZE-1) - cp->tx_fdps_head[channel]);
		}
		else
		{
			ret = cp->tx_fdps_tail[channel] - cp->tx_fdps_head[channel] - 1;
		}
	}
	return ret;
}

/*
	Description : PTM real Rx function , received packets and forward to upper layer
	Input	  :  ptm_private structure
	Output	  :  none
*/
__IRAM_PTM static void __ptm_rx(struct ptm_private *cp, int queue)
{
	unsigned rx_tail = cp->rx_tail[queue];
	u32 len;
	u32 desc_status;
	dma_addr_t mapping = 0;
	struct sk_buff *skb, *new_skb;
	DMA_DESC *desc=NULL;
	unsigned rx_work = PTM_RX_RING_SIZE;
	unsigned buflen;

#ifdef CONFIG_PTM_ROMPERF
	rtl8651_romeperfEnterPoint(61);
#endif

	while (rx_work--)
	{
		if (queue==0) 
		{
			skb = cp->rx_hs_skb[rx_tail].skb;
		}
		else if (queue==1)
		{
			skb = cp->rx_ls_skb[rx_tail].skb;
		}		
		else if (queue==2)
		{
			skb = cp->rx_hf_skb[rx_tail].skb;
		}
		else
		{
			skb = cp->rx_lf_skb[rx_tail].skb;
		}
		if (!skb)
			BUG();

		switch (queue)
		{
			case 0:
				desc = &cp->rx_hsring [rx_tail];
				//printk("cp->rx_hsring[0]=%x", cp->rx_hsring[rx_tail]);
				break;
			case 1:
				desc = &cp->rx_lsring [rx_tail];
				//printk("cp->rx_lsring[1]=%x", cp->rx_lsring[rx_tail]);
				break;
			case 2:
				desc = &cp->rx_hfring [rx_tail];
				//printk("cp->rx_hfring[2]=%x", cp->rx_hfring[rx_tail]);
				break;
			case 3:
				desc = &cp->rx_lfring [rx_tail];
				//printk("cp->rx_lfring[3]=%x", cp->rx_lfring[rx_tail]);
				break;
		}
		
		/* Record the descriptor information */
		desc_status = desc->opts1;
		
		if (desc_status & DescOwn)
		{
			break;
		}

		if(ptm_debug==5){
			printk("[%s, line %d] DESC:0x%08X, 0x%08X, 0x%08X, 0x%08X\n"
				,__func__,__LINE__,desc_status,desc->addr,desc->opts2,desc->opts3);
		}
		
		len = (desc_status & RX_LEN_MASK) ;	

		if ((desc_status & (FirstFrag | LastFrag)) != (FirstFrag | LastFrag)) {
			printk("PTM frag (DESC: 0x%08X, 0x%08X, 0x%08X, 0x%08X)\n"
				, desc_status,desc->addr,desc->opts2,desc->opts3);
			//memDump((void *)skb->data, (len-4),"Frag PTM  RX packet");
			ptm_rx_err_acct(cp, rx_tail, desc_status, len);  
			goto rx_next;
		}

		if (desc_status & (RxErrCRC)) { 
			printk("PTM CRC error (DESC: 0x%08X, 0x%08X, 0x%08X, 0x%08X)\n"
				, desc_status,desc->addr,desc->opts2,desc->opts3);
			ptm_rx_err_acct(cp, rx_tail, desc_status, len);
			goto rx_next;
		}

		if (desc_status & (RxErrRunt)) { 
			printk("PTM runt (DESC: 0x%08X, 0x%08X, 0x%08X, 0x%08X)\n"
				, desc_status,desc->addr,desc->opts2,desc->opts3);
			ptm_rx_err_acct(cp, rx_tail, desc_status, len);
			goto rx_next;
		}

		if (desc_status & (RxError)) {
			printk("PTM rxError (DESC: 0x%08X, 0x%08X, 0x%08X, 0x%08X)\n"
				, desc_status,desc->addr,desc->opts2,desc->opts3);
			ptm_rx_err_acct(cp, rx_tail, desc_status, len);
			goto rx_next;
		}

		/* Remove the redundancy CRC */
		len = len - _CRCLNG_;

		/* PTM hardware adds two bytes for alignment issue */
		buflen = cp->rx_buf_sz + RX_OFFSET;

		new_skb = ptm_getAlloc();

		if (!new_skb) {
                        printk("[%s, line %d] Can't allocate SKB\n",__func__,__LINE__);
			cp->cp_stats.rx_no_mem++;
			dma_cache_wback_inv((unsigned long)skb->data, buflen);
			goto rx_next;
		}

		if((ptm_debug==1) || (ptm_debug==5)){
			printk("[%s, line %d] skb->len = %d, queue = %d\n",__func__,__LINE__,skb->len,queue);
		}
		
		skb_reserve(skb, RX_OFFSET);
		skb_put(skb, len);

		if((ptm_debug==1) || (ptm_debug==5)){
			printk("[%s, line %d] skb->len = %d, queue = %d\n",__func__,__LINE__,skb->len,queue);
		}

		switch (queue)
		{
			case 0:
				mapping =
				cp->rx_hs_skb[rx_tail].mapping = (u32)new_skb->tail | UNCACHE_MASK;
				break;
			case 1:
				mapping =
				cp->rx_ls_skb[rx_tail].mapping = (u32)new_skb->tail | UNCACHE_MASK;
				break;
			case 2:
				mapping =
				cp->rx_hf_skb[rx_tail].mapping = (u32)new_skb->tail | UNCACHE_MASK;
				break;
			case 3:
				mapping =
				cp->rx_lf_skb[rx_tail].mapping = (u32)new_skb->tail | UNCACHE_MASK;
				break;
		}

		if((ptm_debug==1) || (ptm_debug==3))
			memDump(skb->data, len, "rx pkt");

		/* Forward skb to upper layer */
		ptm_rx_skb(cp, skb, desc);

		switch (queue)
		{
			case 0:
				cp->rx_hs_skb[rx_tail].skb = new_skb;
				cp->rx_hsring[rx_tail].addr = mapping& PHYADDR_MASK;
				break;
			case 1:
				cp->rx_ls_skb[rx_tail].skb = new_skb;
				cp->rx_lsring[rx_tail].addr = mapping& PHYADDR_MASK;
				break;
			case 2:
				cp->rx_hf_skb[rx_tail].skb = new_skb;
				cp->rx_hfring[rx_tail].addr = mapping& PHYADDR_MASK;
				break;
			case 3:
				cp->rx_lf_skb[rx_tail].skb = new_skb;
				cp->rx_lfring[rx_tail].addr = mapping& PHYADDR_MASK;
				break;
		}

		dma_cache_wback_inv((unsigned long)new_skb->data, buflen);
rx_next: 
		desc->opts1 = (DescOwn | cp->rx_buf_sz) | 
		    ((rx_tail == (PTM_RX_RING_SIZE - 1))?RingEnd:0);

		rx_tail = NEXT_RX(rx_tail);
	}
	cp->rx_tail[queue] = rx_tail;

#ifdef CONFIG_PTM_ROMPERF
	rtl8651_romeperfExitPoint(61);
#endif

	return;
}

/*
	Description : PTM Rx function , call __ptm_rx
	Input	  :  ptm_private structure
	Output	  :  none
*/
__IRAM_PTM static void ptm_rx (struct ptm_private *cp)
{
	unsigned long flags;  

	spin_lock_irqsave(&cp->lock,flags);

	//protect ptm rx while reboot
	if(ptm_disable == 1){
		spin_unlock_irqrestore (&cp->lock, flags); 
		return;
	}

	if(cp->sts_reg & ( ROK_H_S | RUNT_H_S | RER_OVF_H_S|RDU_H_S))
	{
		__ptm_rx(cp, 0);
	}
	if(cp->sts_reg & ( ROK_L_S | RUNT_L_S | RER_OVF_L_S|RDU_L_S))
	{
		__ptm_rx(cp, 1);
	}
	if(cp->sts_reg & ( ROK_H_F | RUNT_H_F | RER_OVF_H_F|RDU_H_F))
	{
		__ptm_rx(cp, 2);
	}
	if(cp->sts_reg & ( ROK_L_F | RUNT_L_F | RER_OVF_L_F|RDU_L_F))
	{
		__ptm_rx(cp, 3);
	}
	
	RTL_R32(IMR0)|=(u32)(check_interrupts);

	spin_unlock_irqrestore (&cp->lock, flags); 

}  


/*
	Description : PTM interrupt handler
	Input	  :  irq number, dev_instance, struct pt_reg
	Output	  :  return IRQ_RETVAL
*/
__IRAM_PTM static irqreturn_t ptm_interrupt(int irq, void * dev_instance, struct pt_regs *regs)
{
	struct net_device *dev = dev_instance;
	struct ptm_private *cp = dev->priv;

	u32 status;
	int i;
	
	if (!(status = RTL_R32(ISR0)))  
	{
		printk("%s: no status indicated in interrupt, weird!\n", __func__);	
		return IRQ_RETVAL(IRQ_NONE);
	}

	if (status & (TDU_CH3_S | TDU_CH2_S | TDU_CH1_S | TDU_CH0_S | TDU_CH3_F | TDU_CH2_F | TDU_CH1_F | TDU_CH0_F))
	{
		if((ptm_debug==1) || (ptm_debug==4))
			printk("%s: tx interrupt status 0x%08X\n", __func__,status);	//debug

		for(i=0;i<8;i++){
			if(status & (1<<(24+i)))
				cp->cp_stats.tdu_int_cnt[i]++;
		}
	}

	if (status & (check_interrupts))
	{
		if((ptm_debug==1) || (ptm_debug==4))
			printk("%s: rx interrupt status 0x%08X\n", __func__,status);	//debug

		if(status & RER_OVF_H_S){
			cp->cp_stats.rxovf_int_cnt[0]++;
		}
		if(status & RER_OVF_L_S){
			cp->cp_stats.rxovf_int_cnt[1]++;
		}
		if(status & RER_OVF_H_F){
			cp->cp_stats.rxovf_int_cnt[2]++;
		}
		if(status & RER_OVF_L_F){
			cp->cp_stats.rxovf_int_cnt[3]++;
		}	
			
		if(status & RDU_H_S){
			cp->cp_stats.rdu_int_cnt[0]++;
		}
		if(status & RDU_L_S){
			cp->cp_stats.rdu_int_cnt[1]++;
		}
		if(status & RDU_H_F){
			cp->cp_stats.rdu_int_cnt[2]++;
		}
		if(status & RDU_L_F){
			cp->cp_stats.rdu_int_cnt[3]++;
		}

		if(status & ROK_H_S){
			cp->cp_stats.rok_int_cnt[0]++;
		}
		if(status & ROK_L_S){
			cp->cp_stats.rok_int_cnt[1]++;
		}
		if(status & ROK_H_F){
			cp->cp_stats.rok_int_cnt[2]++;
		}
		if(status & ROK_L_F){
			cp->cp_stats.rok_int_cnt[3]++;
		}

		RTL_R32(IMR0) &= ~(check_interrupts);

		tasklet_hi_schedule(&cp->rx_tasklets);
		
	}

	RTL_W32(ISR0,status);
	cp->sts_reg |= status;	

	return IRQ_RETVAL(IRQ_HANDLED);

}

/*
	Description : All channel recycle the TX descriptor
	Input	  :  ptm_private structure
	Output	  :  none
*/
static void ptm_tx (struct ptm_private *cp)
{

	unsigned tx_tail, channel;
	u32 status;
	struct sk_buff *skb;

	if(ptm_disable==1)
		return;

	//-----------------------fast channel ----------------------------//
	for (channel=0; channel<4; channel++)
	{
		tx_tail = cp->tx_fdpf_tail[channel];

		while(!((status = (cp->tx_fdpf_ring[channel][tx_tail].opts1))& DescOwn))
		{
			if(ptm_debug==1 || ptm_debug==2)
				printk("[%s, line %d] status = 0x%08X\n",__FUNCTION__,__LINE__,status);
		
			skb = cp->tx_fdpf_skb[channel][tx_tail].skb;
			if (!skb){
			    break;
			}
		
			rmb();
			cp->net_stats.tx_packets++;
			cp->net_stats.tx_bytes += skb->len;
#ifdef PTMCTL_LOG
			cp->fast_txcnt[channel]++;
#endif
			
			if(skb->destructor==0)
			{
			    dev_kfree_skb(skb);
			}
			else
			    dev_kfree_skb_irq(skb);

			cp->tx_fdpf_skb[channel][tx_tail].skb = NULL;
			tx_tail = NEXT_TX(tx_tail);
		}
		cp->tx_fdpf_tail[channel] = tx_tail;
	}

	//-----------------------slow channel ----------------------------//
	for (channel=0; channel<4; channel++)
	{
		tx_tail = cp->tx_fdps_tail[channel];

		while(!((status = (cp->tx_fdps_ring[channel][tx_tail].opts1))& DescOwn))
		{
			if(ptm_debug==1 || ptm_debug==2)
				printk("[%s, line %d] status = 0x%08X\n",__FUNCTION__,__LINE__,status);

			skb = cp->tx_fdps_skb[channel][tx_tail].skb;
			if (!skb)
			    break;
		
			rmb();
			cp->net_stats.tx_packets++;
			cp->net_stats.tx_bytes += skb->len;
#ifdef PTMCTL_LOG
			cp->slow_txcnt[channel]++;
#endif
			
			if(skb->destructor==0)
			{
			    dev_kfree_skb(skb);
			}
			else
			    dev_kfree_skb_irq(skb);

			cp->tx_fdps_skb[channel][tx_tail].skb = NULL;
			tx_tail = NEXT_TX(tx_tail);
		}	
		cp->tx_fdps_tail[channel] = tx_tail;  
	}

	if (netif_queue_stopped(cp->dev) && (TX_BUFFS_AVAIL(cp, 0, 1) > (MAX_SKB_FRAGS + 1)))
		netif_wake_queue(cp->dev);

}


/*
	Description : Each channel recycle PTM TX ring
	Input	  :  ptm_private structure, indicate PTM channel
	Output	  :  none
*/
__IRAM_PTM static void ptm_tx_ch (struct ptm_private *cp, short fast, u8 channel )
{
	unsigned tx_tail;
	u32 status;
	struct sk_buff *skb;

	if(ptm_disable==1)
		return;

	if(fast){
		tx_tail = cp->tx_fdpf_tail[channel];

		while (!((status = (cp->tx_fdpf_ring[channel][tx_tail].opts1))& DescOwn))
		{
			skb = cp->tx_fdpf_skb[channel][tx_tail].skb;
			if (!skb){
			   	break;
			}

			if(ptm_debug==1 || ptm_debug==2){
				printk("[%s, line %d] channel = %d, tx_tail = %d, status = 0x%08X\n"
				,__FUNCTION__,__LINE__,channel, tx_tail,status);
			}
	
			rmb();
			cp->net_stats.tx_packets++;
			cp->net_stats.tx_bytes += skb->len;
#ifdef PTMCTL_LOG
			cp->fast_txcnt[channel]++;
#endif
			
			if(skb->destructor==0)
			{
			    dev_kfree_skb(skb);
			}
			else
			    dev_kfree_skb_irq(skb);

			cp->tx_fdpf_skb[channel][tx_tail].skb = NULL;
			tx_tail = NEXT_TX(tx_tail);
		}
		cp->tx_fdpf_tail[channel] = tx_tail;
		
	}else{
		tx_tail = cp->tx_fdps_tail[channel];

		while (!((status = (cp->tx_fdps_ring[channel][tx_tail].opts1))& DescOwn))
		{

			skb = cp->tx_fdps_skb[channel][tx_tail].skb;
			if (!skb){
			    break;
			}

			if(ptm_debug==1 || ptm_debug==2){
				printk("[%s, line %d] channel = %d, tx_tail = %d, status = 0x%08X\n"
				,__FUNCTION__,__LINE__,channel, tx_tail,status);
			}
				
			rmb();
			cp->net_stats.tx_packets++;
			cp->net_stats.tx_bytes += skb->len;
#ifdef PTMCTL_LOG
			cp->slow_txcnt[channel]++;
#endif
			
			if(skb->destructor==0)
			{
			    dev_kfree_skb(skb);
			}
			else
			    dev_kfree_skb_irq(skb);

			cp->tx_fdps_skb[channel][tx_tail].skb = NULL;
			tx_tail = NEXT_TX(tx_tail);
		}	
		cp->tx_fdps_tail[channel] = tx_tail;   //czyao 8672c ptm
	}

	if(TX_BUFFS_AVAIL(cp, channel, fast)<(PTM_TX_RING_SIZE/16))
		tasklet_schedule(&ptm_checktxdesc_tasklets);	

	if (netif_queue_stopped(cp->dev) && (TX_BUFFS_AVAIL(cp, channel, fast) > (MAX_SKB_FRAGS + 1)))
		netif_wake_queue(cp->dev);

}

/*
	Description : PTM fake Tx function, just for testing
	Input	  :  net_device structure, sk_buff structure, length of packets
	Output	  :  0 means success
*/
__IRAM_PTM int fake_ptm_start_xmit2(struct sk_buff *skb, struct net_device *dev, int len)
{
	struct ptm_private *cp = dev->priv;
	u32 dram_addr, eor, desc_addr;
	int i;

	dram_addr = (u32)((PHYADDR_MASK) & (u32)(skb->data));
	desc_addr = ((RTL_R32(TxFDP0_F))|0xa0000000) ;

	for(i=0;i<PTM_TX_RING_SIZE;i++)
	{
		cp->cp_stats.tx_cnt++;
		eor = (i == (PTM_TX_RING_SIZE - 1)) ? RingEnd : 0;
		
		//dma_cache_wback_inv((unsigned long)skb->data, len);

		(*(volatile u32*)(desc_addr + i*0x10 +0x4)) = (u32)dram_addr; 
		(*(volatile u32*)(desc_addr + i*0x10)) = (u32)(DescOwn|FirstFrag|LastFrag|eor|TxCRC|len);
		
		//write_buffer_flush();
		wmb();		
	}

	cp->cp_stats.tx_dma_cnt++;
	RTL_W32(IO_CMD, RTL_R32(IO_CMD)| 0x10);	

	return 0;
}


/*
	Description : PTM real Tx function, fill Tx descriptors, DMA packets
	Input	  :  net_device structure, sk_buff structure, indicate PTM channel
	Output	  :  0 means success
*/
__IRAM_PTM int __ptm_start_xmit(struct sk_buff *skb, struct net_device *dev, short fast, u8 channel)
{
	struct ptm_private *cp = dev->priv;
	unsigned entry;
	u32 eor,mapping, len, io_cmd=0;
	DMA_DESC  *txd;
	unsigned long flags;

	cp->cp_stats.tx_cnt++;
	ptm_tx_ch(cp,fast,channel); 
	
	if(ptm_disable == 1 || fast>1 || channel >3 )  
	{
		dev_kfree_skb(skb);
		return 0;
	}

	spin_lock_irqsave(&cp->lock, flags);

	/* This is a hard error, log it. */
	if (TX_BUFFS_AVAIL(cp, channel , fast) <= 1)
	{
		spin_unlock_irqrestore(&cp->lock, flags);
		printk("%s: BUG! Tx Ring full when queue awake!\n", __func__);	
		dev_kfree_skb(skb);
		cp->cp_stats.tx_no_desc++;
		return 0;
	}

	/* Length with vlan tag */
	if ( (skb->len>1518) ) 
	{
		printk("error tx len = %d \n",skb->len);
		spin_unlock_irqrestore(&cp->lock, flags);
		dev_kfree_skb(skb);
		return 0;
	}

	if(ptm_debug==1 || ptm_debug==2)
		printk("[%s, line %d] fast = %d, channel = %d\n",__func__,__LINE__,fast,channel);

	if (fast)
		entry = cp->tx_fdpf_head[channel];
	else
		entry = cp->tx_fdps_head[channel];
			
	eor = (entry == (PTM_TX_RING_SIZE - 1)) ? RingEnd : 0;
	
	if (skb_shinfo(skb)->nr_frags == 0) 
	{
		if (fast)
			txd = &cp->tx_fdpf_ring[channel][entry];
		else
			txd = &cp->tx_fdps_ring[channel][entry];

		len = ETH_ZLEN < skb->len ? skb->len : ETH_ZLEN;

		mapping = (u32)skb->data|UNCACHE_MASK;
		dma_cache_wback_inv((unsigned long)skb->data, len);
		
		if (fast)
		{
			cp->tx_fdpf_skb[channel][entry].skb = skb;
			cp->tx_fdpf_skb[channel][entry].mapping = mapping;
			cp->tx_fdpf_skb[channel][entry].frag = 0;
		}
		else
		{
			cp->tx_fdps_skb[channel][entry].skb = skb;
			cp->tx_fdps_skb[channel][entry].mapping = mapping;
			cp->tx_fdps_skb[channel][entry].frag = 0;
		}

		//printk("[%s, line %d]  ADDR(txd->addr) = 0x%08X, Val = 0x%08X\n",__FUNCTION__,__LINE__, &txd->addr,txd->addr);
		txd->addr = (mapping & PHYADDR_MASK);
		txd->opts1 = (eor | len | DescOwn | FirstFrag | LastFrag | TxCRC);
		
		dma_cache_wback_inv((unsigned long)txd, 16);

		if((ptm_debug==1) || (ptm_debug==2)){
			memDump(&txd->opts1, 32, "opts1");
		}
		
		wmb();
		
		entry = NEXT_TX(entry);
	} 

	if (fast)
		cp->tx_fdpf_head[channel] = entry;	
	else
		cp->tx_fdps_head[channel] = entry;	

		       
	if (TX_BUFFS_AVAIL(cp,channel,fast) <= 1){
		printk("[%s] Stop queue\n",__func__);
		netif_stop_queue(dev);
	}

	spin_unlock_irqrestore(&cp->lock, flags);

	wmb();

	io_cmd = 1<<channel;

	if (!fast){ 
		io_cmd = io_cmd<<4;
	}

	if((ptm_debug==1) || (ptm_debug==2)){
    		memDump(skb->data, skb->len, "tx pkt");
		printk("IO_CMD = 0x%08X\n",io_cmd);
	}

	cp->cp_stats.tx_dma_cnt++;
	RTL_W32(IO_CMD, RTL_R32(IO_CMD)| io_cmd);
	dev->trans_start = jiffies;

	return 0;
}


/*
	Description : PTM Tx function, and call __ptm_start_xmit
	Input	  :  net_device structure, sk_buff structure
	Output	  :  0 means success
*/
__IRAM_PTM static int ptm_start_xmit (struct sk_buff *skb, struct net_device *dev)	
{
	if( adslup && (enable_mode==1)){
		__ptm_start_xmit(skb, dev, ptm_tx_fast, ptm_tx_channel);
	}else{
		dev_kfree_skb(skb);
	}

	return 0;
}

/*
	Description : Set PTM hardware Rx rule
	Input	  :  net_device structure
	Output	  :  none
*/
static void __ptm_set_rx_mode (struct net_device *dev)
{
	/*struct re_private *cp = dev->priv;*/
	u32 mc_filter[2];	/* Multicast hash filter */
	int i, rx_mode;
	/*u32 tmp;*/

	/* Note: do not reorder, GCC is clever about common statements. */
	if (dev->flags & IFF_PROMISC) {
		/* Unconditionally log net taps. */
		printk (KERN_NOTICE "%s: Promiscuous mode enabled.\n",
			dev->name);
		rx_mode =
		    AcceptBroadcast | AcceptMulticast | AcceptMyPhys |
		    AcceptAllPhys;
		mc_filter[1] = mc_filter[0] = 0xffffffff;
	} else if ((dev->mc_count > multicast_filter_limit)
		   || (dev->flags & IFF_ALLMULTI)) {
		/* Too many to filter perfectly -- accept all multicasts. */
		rx_mode = AcceptBroadcast | AcceptMulticast | AcceptMyPhys;
		mc_filter[1] = mc_filter[0] = 0xffffffff;
	} else {
		struct dev_mc_list *mclist;
		rx_mode = AcceptBroadcast | AcceptMyPhys;
		mc_filter[1] = mc_filter[0] = 0;
		for (i = 0, mclist = dev->mc_list; mclist && i < dev->mc_count;
		     i++, mclist = mclist->next) {
			int bit_nr = ether_crc(ETH_ALEN, mclist->dmi_addr) >> 26;

			mc_filter[bit_nr >> 5] |= 1 << (bit_nr & 31);
			rx_mode |= AcceptMulticast;
		}
	}

	RTL_W32(MAR0, mc_filter[0]);
	RTL_W32(MAR4, mc_filter[1]);
#if 0
	rx_mode=0x3f;
#else
	rx_mode = AcceptBroadcast | AcceptMyPhys | AcceptMulticast |AcceptAllPhys;
#endif
	RTL_W16(RCR, rx_mode);
}

/*
	Description : Set PTM Rx mode function, call __ptm_set_rx_mode
	Input	  :  net_device structure
	Output	  :  none
*/
static void ptm_set_rx_mode (struct net_device *dev)
{
	unsigned long flags;
	struct ptm_private *cp = dev->priv;

	spin_lock_irqsave (&cp->lock, flags);
	__ptm_set_rx_mode(dev);
	spin_unlock_irqrestore (&cp->lock, flags);
}

/*
	Description : Set PTM mac address, and fill PTM HW registers
	Input	  :  net_device structure, void * type
	Output	  :  return err;
*/
static int ptm_set_mac_addr(struct net_device *dev, void *addr)
{
	u32 *hwaddr;
	int err;
	err = my_eth_mac_addr(dev, addr);
	if (!err)
	{
		hwaddr = (u32 *)dev->dev_addr;
		RTL_W32(IDR0, *hwaddr);
		hwaddr = (u32 *)(dev->dev_addr+4);
		RTL_W32(IDR4, *hwaddr);
	}
	
	return err;
}

/*
	Description : Get the statistic information from net_device
	Input	  :  struct net_device
	Output	  :  none
*/
static struct net_device_stats *ptm_get_stats(struct net_device *dev)
{
	struct ptm_private *cp = dev->priv;
	
	return &(cp->net_stats);
}

/*
	Description : Stop PTM HW module
	Input	  :  ptm_private structure
	Output	  :  none
*/
static void ptm_stop_hw (struct ptm_private *cp)
{
	int queue;

	RTL_W32(IO_CMD,0); /* timer  rx int 1 packets*/
	RTL_W32(IMR0, 0);
	RTL_W32(ISR0, 0xffffffff);

	//synchronize_irq();
	synchronize_irq(cp->dev->irq);
	udelay(10);

	for (queue=0; queue<4; queue++)
	{
		cp->rx_tail[queue] = 0;
		cp->tx_fdpf_head[queue] = cp->tx_fdpf_tail[queue] = 0;
		cp->tx_fdps_head[queue] = cp->tx_fdps_tail[queue] = 0;
	}
	
}

/*
	Description : Rest PTM HW module by setting registers
	Input	  :  ptm_private structure
	Output	  :  none
*/
static void ptm_reset_hw (struct ptm_private *cp)
{
	unsigned work = 1000;

   	RTL_W8(CMD,0x1);	 /* Reset */	
	while (work--) {
		if (!(RTL_R8(CMD) & 0x1))
			return;
		set_current_state(TASK_UNINTERRUPTIBLE);
		schedule_timeout(10);
	}
   	RTL_W8(CMD,0x1);	 /* Reset */	
	while (work--) {
		if (!(RTL_R8(CMD) & 0x1))
			return;
		set_current_state(TASK_UNINTERRUPTIBLE);
		schedule_timeout(10);
	}

   	RTL_W8(CMD,0x2);	 /* checksum */	
	printk(KERN_ERR "%s: hardware reset timeout\n", cp->dev->name);
}

/*
	Description : Enable PTM TX/RX function
	Input	  :  ptm_private structure
	Output	  :  none
*/
static inline void ptm_start_hw (struct ptm_private *cp)
{
	RTL_W32(IO_CMD,ptm_iocmd_reg); /* timer  rx int 1 packets*/
}

/*
	Description : Set PTM HW related registers 
	Input	  :  ptm_private structure
	Output	  :  none
*/
static void ptm_init_hw (struct ptm_private *cp)
{
	struct net_device *dev = cp->dev;
	u32 *hwaddr;
	int channel;

	ptm_reset_hw(cp);
	RTL_W8(CMD,0x2);	 /* checksum */

	/* QoS setting */
	printk("[%s, line %d] REG32(0x101c) = 0x%08X\n",__func__,__LINE__,RTL_R32(0x101c));
	RTL_W32(DMA_QoS_MIS, (QoS_SEL_F|QoS_SEL_S));
	printk("[%s, line %d] REG32(0x101c) = 0x%08X\n",__func__,__LINE__,RTL_R32(0x101c));

	/* Bonding setting */
	RTL_W32(BD_SLV_NUM, 0x00000002);
	RTL_W32(BOND_FRAG_LF, 0x000000b4);
	RTL_W32(BOND_FRAG_HF, 0x00000058);
	RTL_W32(BOND_FRAG_LS, 0x000000ac);
	RTL_W32(BOND_FRAG_HS, 0x00000050);
	RTL_W32(BD_TIMEOUT, 0x000061a8);

	RTL_W16(ISR1, 0);
	RTL_W32(ISR0,0xffffffff);

	RTL_W32(IMR0, ptm_intr_mask);   	

	RTL_W32(RxFDP_H_S, (((u32)(cp->rx_hsring))&PHYADDR_MASK));
	RTL_W16(RxCDO_H_S, 0);
	RTL_W32(RxFDP_L_S, ((u32)(cp->rx_lsring))&PHYADDR_MASK);
	RTL_W16(RxCDO_L_S, 0);
	RTL_W32(RxFDP_H_F, ((u32)(cp->rx_hfring))&PHYADDR_MASK);
	RTL_W16(RxCDO_H_F, 0);
	RTL_W32(RxFDP_L_F, ((u32)(cp->rx_lfring))&PHYADDR_MASK);
	RTL_W16(RxCDO_L_F, 0);

	for(channel=0; channel<4; channel++)
	{
		RTL_W32((TxFDP0_S+(0x10*channel)), ((u32)(cp->tx_fdps_ring[channel]))&PHYADDR_MASK);
		RTL_W16(TxCDO0_S+(0x10*channel), 0);
		RTL_W32((TxFDP0_F+(0x10*channel)), ((u32)(cp->tx_fdpf_ring[channel]))&PHYADDR_MASK);
		RTL_W16(TxCDO0_F+(0x10*channel), 0);
	}			

	hwaddr = (u32 *)cp->dev->dev_addr;
	RTL_W32(IDR0, *hwaddr);
	hwaddr = (u32 *)(cp->dev->dev_addr+4);
	RTL_W32(IDR4, *hwaddr);


#ifdef CONFIG_PTM_HDLC
	//RTL_W32(PCR, 0x00000002);
	RTL_W32(PCR, 0x00000111);
#else
	RTL_W32(PCR, 0x00000119);  //64/65 mode turns on short packet mode! BUG!!!
#endif

	ptm_start_hw(cp);
	__ptm_set_rx_mode(dev);
}

/*
	Description : Fill PTM ring information, and indicate the SKB
	Input	  :  ptm_private structure
	Output	  :  0 means success, and others means fail
*/
static int ptm_refill_rx (struct ptm_private *cp)
{
	unsigned i;

	printk("Allocating %d skb for %s low RX ring, rx_buf_sz=%d\n", PTM_RX_RING_SIZE, DRV_NAME, cp->rx_buf_sz);
	for (i = 0; i < PTM_RX_RING_SIZE; i++) 
	{
		struct sk_buff *skb;
// 0
		skb = ptm_getAlloc();
		if (!skb)
			goto err_out;

		skb->dev = cp->dev;

		cp->rx_hs_skb[i].skb = skb;
		cp->rx_hs_skb[i].frag = 0;
		if ((u32)skb->data &0x3)
			printk(KERN_DEBUG "skb->data unaligment %8x\n",(u32)skb->data);

		cp->rx_hsring[i].addr = ((u32)skb->data|UNCACHE_MASK)&PHYADDR_MASK;
		if (i == (PTM_RX_RING_SIZE - 1))
			cp->rx_hsring[i].opts1 = (DescOwn | RingEnd | cp->rx_buf_sz);
		else
			cp->rx_hsring[i].opts1 =(DescOwn | cp->rx_buf_sz);

		cp->rx_hsring[i].opts2 = 0;

// 1
		skb = ptm_getAlloc();
		if (!skb)
			goto err_out;

		skb->dev = cp->dev;

		cp->rx_ls_skb[i].skb = skb;
		cp->rx_ls_skb[i].frag = 0;
		if ((u32)skb->data &0x3)
			printk(KERN_DEBUG "skb->data unaligment %8x\n",(u32)skb->data);

		cp->rx_lsring[i].addr = ((u32)skb->data|UNCACHE_MASK)&PHYADDR_MASK;
		if (i == (PTM_RX_RING_SIZE - 1))
			cp->rx_lsring[i].opts1 = (DescOwn | RingEnd | cp->rx_buf_sz);
		else
			cp->rx_lsring[i].opts1 =(DescOwn | cp->rx_buf_sz);

		cp->rx_lsring[i].opts2 = 0;

// 2
		skb = ptm_getAlloc();
		if (!skb)
			goto err_out;

		skb->dev = cp->dev;

		cp->rx_hf_skb[i].skb = skb;
		cp->rx_hf_skb[i].frag = 0;
		if ((u32)skb->data &0x3)
			printk(KERN_DEBUG "skb->data unaligment %8x\n",(u32)skb->data);

		cp->rx_hfring[i].addr = ((u32)skb->data|UNCACHE_MASK)&PHYADDR_MASK;
		if (i == (PTM_RX_RING_SIZE - 1))
			cp->rx_hfring[i].opts1 = (DescOwn | RingEnd | cp->rx_buf_sz);
		else
			cp->rx_hfring[i].opts1 =(DescOwn | cp->rx_buf_sz);

		cp->rx_hfring[i].opts2 = 0;

// 3
		skb = ptm_getAlloc();
		if (!skb)
			goto err_out;

		skb->dev = cp->dev;

		cp->rx_lf_skb[i].skb = skb;
		cp->rx_lf_skb[i].frag = 0;
		if ((u32)skb->data &0x3)
			printk(KERN_DEBUG "skb->data unaligment %8x\n",(u32)skb->data);

		cp->rx_lfring[i].addr = ((u32)skb->data|UNCACHE_MASK)&PHYADDR_MASK;
		if (i == (PTM_RX_RING_SIZE - 1))
			cp->rx_lfring[i].opts1 = (DescOwn | RingEnd | cp->rx_buf_sz);
		else
			cp->rx_lfring[i].opts1 =(DescOwn | cp->rx_buf_sz);

		cp->rx_lfring[i].opts2 = 0;

	}

	return 0;

err_out:
	ptm_clean_rings(cp);
	return -ENOMEM;
}

/*
	Description : Initialize PTM TX timeout function
	Input	  :  net_device structure
	Output	  :  none
*/
static void ptm_tx_timeout (struct net_device *dev)
{
	struct ptm_private *cp = dev->priv;
	unsigned long flags;

	cp->cp_stats.tx_timeouts++;

	spin_lock_irqsave(&cp->lock, flags);
	
	/* Do nothing, under implement */

	spin_unlock_irqrestore(&cp->lock,flags);
	
	if (netif_queue_stopped(cp->dev))
		netif_wake_queue(cp->dev);

}

/*
	Description : Initialize PTM ring information 
	Input	  :  ptm_private structure
	Output	  :  return ptm_refill_rx result
*/
static int ptm_init_rings (struct ptm_private *cp)
{
	int queue;
	for (queue=0;queue<4;queue++)
	{
		cp->rx_tail[queue]=0;
		cp->tx_fdpf_head[queue] = cp->tx_fdpf_tail[queue] = cp->tx_fdps_head[queue] = cp->tx_fdps_tail[queue] = 0;
	}

	return ptm_refill_rx (cp);
}


#ifdef PTM_SRAM_USAGE
unsigned char *ptm_sram_buf=NULL;

/*
	Description : Check the used of SRAM size  
	Input	  :  the used sram size
	Output	  :  0 means success, or it hangs
*/
static int ptm_assert_sram_size(int total_used_sram_size)
{
		if(total_used_sram_size>32*1024)
		{
			printk("Out of SRAM size!!\n"); 
			while(1);
		}
		return 0;
}
#endif

/*
	Description : Allocate PTM decriptors information
	Input	  :  ptm_private structure
	Output	  :  return ptm_init_ring result, others mean fail
*/
static int ptm_alloc_rings (struct ptm_private *cp)
{
	void*	pBuf = NULL;
	int 		channel = 0;

#ifdef PTM_SRAM_USAGE
	int ptm_total_sram_size=0;

	ptm_sram_buf = kzalloc(4*PTM_RXRING_BYTES, GFP_KERNEL);

	if(ptm_sram_buf==NULL)
	{
		printk("Out of memory!!\n"); 
		goto ErrMem;
	}

	if((u32)ptm_sram_buf & 0x7fff) //is 32k alignment
		ptm_sram_buf=(unsigned char *)(((((unsigned int)ptm_sram_buf)&(0xffff8000))|(0xa0000000))+32*1024);
	else
		ptm_sram_buf=(unsigned char *)(((unsigned int)ptm_sram_buf)|(0xa0000000));

	printk("align 32k sram_buf=%x\n",(unsigned int)pBuf);		

	//SRAM Mapping enable
	WRITE_MEM32(0xb8001300,(((unsigned int)ptm_sram_buf)&(0x1ffffffe))|1);
	WRITE_MEM32(0xb8001304,8);  // 32K  DRAM unmap
	WRITE_MEM32(0xb8004000,(((unsigned int)ptm_sram_buf)&(0x1ffffffe))|1);
	WRITE_MEM32(0xb8004004,8);  // 32K  SRAM mapping
#endif

//-------------------- PTM RX_HS ring --------------------//
#ifdef PTM_SRAM_USAGE
	pBuf = (void*)( (u32)(ptm_sram_buf));
	ptm_total_sram_size += PTM_RXRING_BYTES;
	
	if(ptm_assert_sram_size(ptm_total_sram_size)) 
		goto ErrMem;
#else
	pBuf = kzalloc(PTM_RXRING_BYTES, GFP_KERNEL);
	if (!pBuf)
		goto ErrMem;
#endif
	cp->rxdesc_buf_hs = pBuf;
	pBuf = (void*)( (u32)(pBuf + DESC_ALIGN-1) &  ~(DESC_ALIGN -1) ) ;
	cp->rx_hsring = (DMA_DESC*)((u32)(pBuf) | UNCACHE_MASK);


//-------------------- PTM RX_LS ring --------------------//
#ifdef PTM_SRAM_USAGE
	pBuf = (void*)( (u32)(ptm_sram_buf+ptm_total_sram_size));
	ptm_total_sram_size += PTM_RXRING_BYTES;
	
	if(ptm_assert_sram_size(ptm_total_sram_size)) 
		goto ErrMem;
#else
	pBuf = kzalloc(PTM_RXRING_BYTES, GFP_KERNEL);
	if (!pBuf)
		goto ErrMem;
#endif
	cp->rxdesc_buf_ls = pBuf;
	pBuf = (void*)( (u32)(pBuf + DESC_ALIGN-1) &  ~(DESC_ALIGN -1) ) ;
	cp->rx_lsring = (DMA_DESC*)((u32)(pBuf) | UNCACHE_MASK);

//-------------------- PTM RX_HF ring --------------------//
#ifdef PTM_SRAM_USAGE
	pBuf = (void*)( (u32)(ptm_sram_buf+ptm_total_sram_size));
	ptm_total_sram_size += PTM_RXRING_BYTES;
	
	if(ptm_assert_sram_size(ptm_total_sram_size)) 
		goto ErrMem;
#else
	pBuf = kzalloc(PTM_RXRING_BYTES, GFP_KERNEL);
	if (!pBuf)
		goto ErrMem;
#endif
	cp->rxdesc_buf_hf = pBuf;
	pBuf = (void*)( (u32)(pBuf + DESC_ALIGN-1) &  ~(DESC_ALIGN -1) ) ;
	cp->rx_hfring = (DMA_DESC*)((u32)(pBuf) | UNCACHE_MASK);

//-------------------- PTM RX_LF ring --------------------//
#ifdef PTM_SRAM_USAGE
	pBuf = (void*)( (u32)(ptm_sram_buf+ptm_total_sram_size));
	ptm_total_sram_size += PTM_RXRING_BYTES;
	
	if(ptm_assert_sram_size(ptm_total_sram_size)) 
		goto ErrMem;
#else
	pBuf = kzalloc(PTM_RXRING_BYTES, GFP_KERNEL);
	if (!pBuf)
		goto ErrMem;
#endif
	cp->rxdesc_buf_lf = pBuf;
	pBuf = (void*)( (u32)(pBuf + DESC_ALIGN-1) &  ~(DESC_ALIGN -1) ) ;
	cp->rx_lfring = (DMA_DESC*)((u32)(pBuf) | UNCACHE_MASK);


	for (channel=0; channel<4; channel++)
	{
		pBuf= kzalloc(PTM_TXRING_BYTES, GFP_KERNEL);

		if (!pBuf)
			goto ErrMem;
		
		cp->txdesc_buf_s[channel] = (u32)pBuf;
		pBuf = (void*)( (u32)(pBuf + DESC_ALIGN-1) &  ~(DESC_ALIGN -1) ) ;
		cp->tx_fdps_ring[channel] = (DMA_DESC*)((u32)(pBuf) | UNCACHE_MASK);
	}

	for (channel=0; channel<4; channel++)
	{
		pBuf= kzalloc(PTM_TXRING_BYTES, GFP_KERNEL);

		if (!pBuf)
			goto ErrMem;
		
		cp->txdesc_buf_f[channel] = (u32)pBuf;
		pBuf = (void*)( (u32)(pBuf + DESC_ALIGN-1) &  ~(DESC_ALIGN -1) ) ;
		cp->tx_fdpf_ring[channel] = (DMA_DESC*)((u32)(pBuf) | UNCACHE_MASK );
	}


	return ptm_init_rings(cp);

ErrMem:

	printk("ErrMemory![%s]\n", __FUNCTION__);

#ifdef PTM_SRAM_USAGE
	*((unsigned long*)0xb8001300)=0x0;
        *((unsigned long*)0xb8001310)=0x0;
        *((unsigned long*)0xb8001320)=0x0;
        *((unsigned long*)0xb8001330)=0x0;
        *((unsigned long*)0xb8004000)=0x0;
        *((unsigned long*)0xb8004010)=0x0;
        *((unsigned long*)0xb8004020)=0x0;
        *((unsigned long*)0xb8004030)=0x0;

	//kfree(ptm_sram_buf);
#else

	if (cp->rxdesc_buf_hs)
		kfree(cp->rxdesc_buf_hs);
	if (cp->rxdesc_buf_ls)
		kfree(cp->rxdesc_buf_ls);
	if (cp->rxdesc_buf_hf)
		kfree(cp->rxdesc_buf_hf);
	if (cp->rxdesc_buf_lf)
		kfree(cp->rxdesc_buf_lf);
#endif


	for(channel=0; channel<4; channel++)
	{
		if (cp->txdesc_buf_s[channel])
			kfree(&cp->txdesc_buf_s[channel]);
	}

	for(channel=0; channel<4; channel++)
	{
		if (cp->txdesc_buf_f[channel])
			kfree(&cp->txdesc_buf_f[channel]);
	}

	return -ENOMEM;
	
}

/*
	Description : Clear PTM ring information, free skb and free buffer
	Input	  :  ptm_private structure
	Output	  :  none
*/
static void ptm_clean_rings (struct ptm_private *cp)
{
	unsigned i;
	struct sk_buff *skb;
	int channel;

	for (i = 0; i < PTM_RX_RING_SIZE; i++) 
	{
		if (cp->rx_hs_skb[i].skb)
		{
			dev_kfree_skb(cp->rx_hs_skb[i].skb);
		}
	}
	for (i = 0; i < PTM_RX_RING_SIZE; i++) 
	{
		if (cp->rx_ls_skb[i].skb)
		{
			dev_kfree_skb(cp->rx_ls_skb[i].skb);
		}
	}
	for (i = 0; i < PTM_RX_RING_SIZE; i++) 
	{
		if (cp->rx_hf_skb[i].skb)
		{
			dev_kfree_skb(cp->rx_hf_skb[i].skb);
		}
	}
	for (i = 0; i < PTM_RX_RING_SIZE; i++) 
	{
		if (cp->rx_lf_skb[i].skb)
		{
			dev_kfree_skb(cp->rx_lf_skb[i].skb);
		}
	}

	for (i = 0; i < PTM_TX_RING_SIZE; i++) {
		int channel;
		for(channel=0; channel<4; channel++)
		{
			skb = cp->tx_fdps_skb[channel][i].skb;
			dev_kfree_skb(skb);
			cp->net_stats.tx_dropped++;
		}

		for(channel=0; channel<4; channel++)
		{
			skb = cp->tx_fdpf_skb[channel][i].skb;
			dev_kfree_skb(skb);
			cp->net_stats.tx_dropped++;
		}
	}

	memset(&cp->rx_hs_skb, 0, sizeof(struct ring_info) * PTM_RX_RING_SIZE);
	memset(&cp->rx_ls_skb, 0, sizeof(struct ring_info) * PTM_RX_RING_SIZE);
	memset(&cp->rx_hf_skb, 0, sizeof(struct ring_info) * PTM_RX_RING_SIZE);
	memset(&cp->rx_lf_skb, 0, sizeof(struct ring_info) * PTM_RX_RING_SIZE);


	for(channel=0; channel<4; channel++)
	{
		memset(&cp->tx_fdps_skb[channel][0], 0, sizeof(struct ring_info) * PTM_TX_RING_SIZE);
		memset(&cp->tx_fdpf_skb[channel][0], 0, sizeof(struct ring_info) * PTM_TX_RING_SIZE);
	}
	
}

/*
	Description : Free the PTM RX/TX buffer
	Input	  :  ptm_private structure
	Output	  :  none
*/
static void ptm_free_rings (struct ptm_private *cp)
{
	int channel;
	
	ptm_clean_rings(cp);

#ifdef PTM_SRAM_USAGE
	*((unsigned long*)0xb8001300)=0x0;
        *((unsigned long*)0xb8001310)=0x0;
        *((unsigned long*)0xb8001320)=0x0;
        *((unsigned long*)0xb8001330)=0x0;
        *((unsigned long*)0xb8004000)=0x0;
        *((unsigned long*)0xb8004010)=0x0;
        *((unsigned long*)0xb8004020)=0x0;
        *((unsigned long*)0xb8004030)=0x0;

	//kfree(ptm_sram_buf);

#else
	kfree(cp->rxdesc_buf_hs);
	kfree(cp->rxdesc_buf_ls);
	kfree(cp->rxdesc_buf_hf);
	kfree(cp->rxdesc_buf_lf);
#endif

	
	for(channel=0; channel<4; channel++)
	{
		kfree(&cp->txdesc_buf_s[channel]);
		kfree(&cp->txdesc_buf_f[channel]);
	}

	cp->rx_hsring = NULL;
	cp->rx_lsring = NULL;
	cp->rx_hfring = NULL;
	cp->rx_lfring = NULL;

	for(channel=0; channel<4; channel++)
	{
		cp->tx_fdps_ring[channel] = NULL;
		cp->tx_fdpf_ring[channel] = NULL;
	}

}


/*
	Description : Open PTM HW module
	Input	  :  net_device structure
	Output	  :  0 means success, others mean fail
*/
static int ptm_open (struct net_device *dev)
{
	struct ptm_private *cp = dev->priv;
	int rc;

	ptm_disable=0;
	if (netif_msg_ifup(cp))
		printk(KERN_DEBUG "%s: enabling interface\n", dev->name);

#ifdef ENABLE_LOOPBACK
	*(volatile u32*)(SARCHEM_BASE + 0x5800) = 0x00000006;  //czyao
	*(volatile u32*)(SARCHEM_BASE + 0x6800) = 0x0000002a;  //czyao
#ifdef CONFIG_PTM_HDLC
	*(volatile u32*)(SARCHEM_BASE + 0x5824) = 0x0001c101; //czyao
#else
	*(volatile u32*)(SARCHEM_BASE + 0x5824) = 0x0002c101; //czyao
#endif
#endif

	/* Set new rx buf size */
	ptm_set_rxbufsize(cp);	

	/* Allocate PTM  TX and RX rings */
	rc = ptm_alloc_rings(cp);
	if (rc)
		return rc;

	/* Set PTM HW related setting */
	ptm_init_hw(cp);

	/* Clear PTM int status */
	cp->sts_reg = 0;	

	/* Register interrupt handler */
	rc = request_irq(dev->irq, (irq_handler_t)ptm_interrupt, IRQF_DISABLED, dev->name, dev);
	if (rc)
		goto err_out_hw;
	
	netif_start_queue(dev);	

	return 0;

err_out_hw:
	ptm_stop_hw(cp);
	ptm_free_rings(cp);
	return rc;
}

/*
	Description : Turn off PTM hardware realted function
	Input	  :  net_device structure
	Output	  :  0 means success, others mean fail
*/
static int ptm_close (struct net_device *dev)
{
	struct ptm_private *cp = dev->priv;

	ptm_disable=1;

	if (netif_msg_ifdown(cp))
		printk(KERN_DEBUG "%s: disabling interface\n", dev->name);

	netif_stop_queue(dev);
	ptm_stop_hw(cp);
	free_irq(dev->irq, dev);
	ptm_free_rings(cp);
	return 0;
}

static  void ptm_ctl(struct eth_arg * arg){
	unsigned char cmd1;
	unsigned int cmd2,cmd3; 
	unsigned short cmd4;

	unsigned int i,len, part=0;

	struct skb_shared_info skbshare;
	struct ptm_private *cp = ptm_net_dev->priv;

	unsigned long rate, timer1;
	unsigned char tx_ch[2] = "SF";

	/* Assign the command content */
	cmd1=arg->cmd;
	cmd2=arg->cmd2;
	cmd3=arg->cmd3;
	cmd4=(unsigned short)arg->cmd4;

	switch(cmd1){

		/* Test PTM TX throughput */
		case PTM_TX_TEST: 
			len =cmd3;

			/*Give SKB information */
			skbshare.nr_frags=0;
			for(i=0;i<len;i++){
				ptm_testskbdata1[i] = (i & 0xff);
			}

			testskb1.end=(unsigned char*)&skbshare;
			testskb1.data = ptm_testskbdata1;
			testskb1.len = len ;

			/* Clear TXDMACNT */
			RTL_W32(TXDMAOKCNT_CH0_F,0);

			/* Call Fake PTM Tx function */
			timer1 = jiffies;
			printk("Start Time : %lu\n",jiffies);

			for(i=0; i<cmd2;i++){

				if(i == part*(cmd2/100)) {
					part++;
					printk("*");
				}					
				fake_ptm_start_xmit2(&testskb1,  ptm_net_dev, len);	
			}
			printk("\nEnd Time : %lu\n",jiffies);

			timer1 = jiffies - timer1;
			printk("Continuous Time : %lu\n",timer1);

			rate = RTL_R32(TXDMAOKCNT_CH0_F);
			printk("Packet counter : %lu\n",rate);

			rate = (((rate*cmd3*8)/timer1)*100)/(1024*1024);
			printk("Rate : %lu Mbps\n",rate);
			break;
			
		case PTM_RX_TEST:
			printk("Not implement\n");
			break;
			
		/* Reset PTM HW */
		case PTM_RESET:
			printk("!!!!!!!!!!! Reset PTM HW !!!!!!!!!!\n");
			ptm_stop_hw(cp);
			ptm_free_rings(cp);
			ptm_alloc_rings(cp);
			ptm_init_hw(cp);
			break;

		/* Set PTM TX path */
		case PTM_SET_TXPATH:
			if(cmd2>=0)
				ptm_tx_fast = cmd2;

			if(cmd3>=0)
				ptm_tx_channel = cmd3;

			printk("Set PTM TX path: %c, channel %d\n",tx_ch[ptm_tx_fast],ptm_tx_channel);
			break;

		/* PTM debug parameter */
		case PTM_DEBUG_EN:
			ptm_debug = cmd2;
			break;

		default:
			printk("error cmd\n");
	}
}

/*
	Description : IOCTL function for PTM
	Input	  :  net_device structure, ifreq structure, cmd
	Output	  :  0 means success, others mean fail
*/
static int ptm_ioctl (struct net_device *dev, struct ifreq *rq, int cmd)
{
	int rc = 0;

	if (!netif_running(dev) && cmd!=SIOCETHTEST)
		return -EINVAL;

	switch (cmd) {

	case SIOCETHTEST:
		ptm_ctl((struct eth_arg *)rq->ifr_data);
		break;
	default:
		rc = -EOPNOTSUPP;
		break;
	}

	return rc;
}

static int driver_version_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	
	printk("%s driver v%s (%s)\n", DRV_NAME, DRV_VERSION, DRV_RELDATE);
	return len;
}

static int hw_reg_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;

	printk("PTMBASE			=0x%08x\n", PTMBASE);
	printk("IDR			=%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x\n", 
		RTL_R8(IDR0), RTL_R8(IDR1), RTL_R8(IDR2), RTL_R8(IDR3), RTL_R8(IDR4), RTL_R8(IDR5));
	printk("MAR			=%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x\n", 
		RTL_R8(MAR0), RTL_R8(MAR1), RTL_R8(MAR2), RTL_R8(MAR3), 
		RTL_R8(MAR4), RTL_R8(MAR5), RTL_R8(MAR6), RTL_R8(MAR7));
	printk("TRSR			=0x%08x\n",RTL_R32(TRSR));
	printk("CMD			=0x%02x			IMR0		=0x%08x\n", RTL_R8(CMD), RTL_R32(IMR0));
	printk("ISR0			=0x%08x		IMR1		=0x%08x\n", RTL_R32(ISR0), RTL_R32(IMR1));
	printk("ISR1			=0x%08x		PCR		=0x%08x\n", RTL_R32(ISR1),RTL_R32(PCR));
	
	printk("TxFDP0_F		=0x%08x		TxCDO0_F	=0x%04x\n", RTL_R32(TxFDP0_F), RTL_R16(TxCDO0_F));
	printk("TxFDP1_F		=0x%08x		TxCDO1_F	=0x%04x\n", RTL_R32(TxFDP1_F), RTL_R16(TxCDO1_F));
	printk("TxFDP2_F		=0x%08x		TxCDO2_F	=0x%04x\n", RTL_R32(TxFDP2_F), RTL_R16(TxCDO2_F));
	printk("TxFDP3_F		=0x%08x		TxCDO3_F	=0x%04x\n", RTL_R32(TxFDP3_F), RTL_R16(TxCDO3_F));
	printk("TxFDP0_S		=0x%08x		TxCDO0_S	=0x%04x\n", RTL_R32(TxFDP0_S), RTL_R16(TxCDO0_S));
	printk("TxFDP1_S		=0x%08x		TxCDO1_S	=0x%04x\n", RTL_R32(TxFDP1_S), RTL_R16(TxCDO1_S));
	printk("TxFDP2_S		=0x%08x		TxCDO2_S	=0x%04x\n", RTL_R32(TxFDP2_S), RTL_R16(TxCDO2_S));
	printk("TxFDP3_S		=0x%08x		TxCDO3_S	=0x%04x\n", RTL_R32(TxFDP3_S), RTL_R16(TxCDO3_S));

#if 0	
	printk("RxFDP_HMQ_L_F		=0x%08x		RxCDO_HMQ_L_F	=0x%04x\n", RTL_R32(RxFDP_HMQ_L_F), RTL_R16(RxCDO_HMQ_L_F));
	printk("RxFDP_HMQ_H_F		=0x%08x		RxCDO_HMQ_H_F	=0x%04x\n", RTL_R32(RxFDP_HMQ_H_F), RTL_R16(RxCDO_HMQ_H_F));
	printk("RxFDP_HMQ_L_S		=0x%08x		RxCDO_HMQ_L_S	=0x%04x\n", RTL_R32(RxFDP_HMQ_L_S), RTL_R16(RxCDO_HMQ_L_S));
	printk("RxFDP_HMQ_H_S		=0x%08x		RxCDO_HMQ_H_S	=0x%04x\n", RTL_R32(RxFDP_HMQ_H_S), RTL_R16(RxCDO_HMQ_H_S));
#endif
	printk("RxFDP_L_F		=0x%08x		RxCDO_L_F	=0x%04x\n", RTL_R32(RxFDP_L_F), RTL_R16(RxCDO_L_F));
	printk("RxFDP_H_F		=0x%08x		RxCDO_H_F	=0x%04x\n", RTL_R32(RxFDP_H_F), RTL_R16(RxCDO_H_F));
	printk("RxFDP_L_S		=0x%08x		RxCDO_L_S	=0x%04x\n", RTL_R32(RxFDP_L_S), RTL_R16(RxCDO_L_S));
	printk("RxFDP_H_S		=0x%08x		RxCDO_H_S	=0x%04x\n", RTL_R32(RxFDP_H_S), RTL_R16(RxCDO_H_S));

	printk("DMA_QoS_MIS		=0x%08x\n",RTL_R32(DMA_QoS_MIS));	
	printk("IO_CMD			=0x%02x\n", RTL_R32(IO_CMD));
	

	return len;
}

static int hw_counter_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;

	printk("TX DMA counter (offset 0x0010-002F)\n");
	printk("CH0_F		=0x%08x		CH1_F	=0x%08x\n",RTL_R32(TXDMAOKCNT_CH0_F),RTL_R32(TXDMAOKCNT_CH1_F));
	printk("CH2_F		=0x%08x		CH3_F	=0x%08x\n",RTL_R32(TXDMAOKCNT_CH2_F),RTL_R32(TXDMAOKCNT_CH3_F));
	printk("CH0_S		=0x%08x		CH1_S	=0x%08x\n",RTL_R32(TXDMAOKCNT_CH0_S),RTL_R32(TXDMAOKCNT_CH1_S));
	printk("CH2_S		=0x%08x		CH3_S	=0x%08x\n",RTL_R32(TXDMAOKCNT_CH2_S),RTL_R32(TXDMAOKCNT_CH3_S));

	printk("\nRX OK counter (offset 0x0050-005F)\n");
	printk("L_F	=0x%08x		H_F	=0x%08x\n",RTL_R32(RxOKCNT_L_F),RTL_R32(RxOKCNT_H_F));
	printk("L_S	=0x%08x		H_S	=0x%08x\n",RTL_R32(RxOKCNT_L_S),RTL_R32(RxOKCNT_H_S));

	printk("\nRX Error counter (offset 0x0060-006F)\n");
	printk("L_F	=0x%08x		H_F	=0x%08x\n",RTL_R32(RxErr_L_F),RTL_R32(RxErr_H_F));
	printk("L_S	=0x%08x		H_S	=0x%08x\n",RTL_R32(RxErr_L_S),RTL_R32(RxErr_H_S));

	printk("\nRX Miss Pkt counter (offset 0x0070-007F)\n");
	printk("L_F	=0x%08x		H_F	=0x%08x\n",RTL_R32(MissPkt_L_F),RTL_R32(MissPkt_H_F));
	printk("L_S	=0x%08x		H_S	=0x%08x\n",RTL_R32(MissPkt_L_S),RTL_R32(MissPkt_H_S));

	printk("\nRX OK PHY counter (offset 0x0080-008F)\n");
	printk("L_F	=0x%08x		H_F	=0x%08x\n",RTL_R32(RxOkPhy_L_F),RTL_R32(RxOkPhy_H_F));
	printk("L_S	=0x%08x		H_S	=0x%08x\n",RTL_R32(RxOkPhy_L_S),RTL_R32(RxOkPhy_H_S));
	
	printk("\nRX OK Brd counter (offset 0x0090-009F)\n");
	printk("L_F	=0x%08x		H_F	=0x%08x\n",RTL_R32(RxOkBrd_L_F),RTL_R32(RxOkBrd_H_F));
	printk("L_S	=0x%08x		H_S	=0x%08x\n",RTL_R32(RxOkBrd_L_S),RTL_R32(RxOkBrd_H_S));

	printk("\nRX OK Mul counter (offset 0x00a0-00aF)\n");
	printk("L_F	=0x%08x		H_F	=0x%08x\n",RTL_R32(RxOkMu1_L_F),RTL_R32(RxOkMu1_H_F));
	printk("L_S	=0x%08x		H_S	=0x%08x\n",RTL_R32(RxOkMu1_L_S),RTL_R32(RxOkMu1_H_S));

	printk("\nRX Data Codeword counter (offset 0x01b0-01b4)\n");
	printk("Fast	=0x%08x		Slow	=0x%08x\n",RTL_R32(RxcodeWord_F),RTL_R32(RxCodeWord_S));

	printk("\nRX CRC Error counter (offset 0x0180-018c)\n");
	printk("L_F	=0x%08x		H_F	=0x%08x\n",RTL_R32(Rx_CRC_ERR_LF),RTL_R32(Rx_CRC_ERR_HF));
	printk("L_S	=0x%08x		H_S	=0x%08x\n",RTL_R32(Rx_CRC_ERR_LS),RTL_R32(Rx_CRC_ERR_HS));	

	printk("\nRX HDLC invlaid frame counter (offset 0x01a0-01a4)\n");
	printk("Fast	=0x%08x		Slow	=0x%08x\n",RTL_R32(HDLC_INVLD_F),RTL_R32(HDLC_INVLD_S));

	printk("\nRX coding err counter (offset 0x01a8-01ac)\n");
	printk("Fast	=0x%08x		Slow	=0x%08x\n",RTL_R32(TC_CODING_ERR_F),RTL_R32(TC_CODING_ERR_S));
	
	return len;
}

static int hw_counter_zero(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;

	printk("Reset PTM counter to zero!!\n");

	RTL_W32(TXDMAOKCNT_CH0_F,0);
	RTL_W32(TXDMAOKCNT_CH1_F,0);
	RTL_W32(TXDMAOKCNT_CH2_F,0);
	RTL_W32(TXDMAOKCNT_CH3_F,0);
	RTL_W32(TXDMAOKCNT_CH0_S,0);
	RTL_W32(TXDMAOKCNT_CH1_S,0);
	RTL_W32(TXDMAOKCNT_CH2_S,0);
	RTL_W32(TXDMAOKCNT_CH3_S,0);

	RTL_W32(RxOKCNT_L_F,0);
	RTL_W32(RxOKCNT_H_F,0);
	RTL_W32(RxOKCNT_L_S,0);
	RTL_W32(RxOKCNT_H_S,0);
	
	RTL_W32(RxErr_L_F,0);
	RTL_W32(RxErr_H_F,0);
	RTL_W32(RxErr_L_S,0);
	RTL_W32(RxErr_H_S,0);
	
	RTL_W32(MissPkt_L_F,0);
	RTL_W32(MissPkt_H_F,0);
	RTL_W32(MissPkt_L_S,0);
	RTL_W32(MissPkt_H_S,0);

	RTL_W32(RxOkPhy_L_F,0);
	RTL_W32(RxOkPhy_H_F,0);
	RTL_W32(RxOkPhy_L_S,0);
	RTL_W32(RxOkPhy_H_S,0);

	RTL_W32(RxOkBrd_L_F,0);
	RTL_W32(RxOkBrd_H_F,0);
	RTL_W32(RxOkBrd_L_S,0);
	RTL_W32(RxOkBrd_H_S,0);
	
	RTL_W32(RxOkMu1_L_F,0);
	RTL_W32(RxOkMu1_H_F,0);
	RTL_W32(RxOkMu1_L_S,0);
	RTL_W32(RxOkMu1_H_S,0);

	return len;
}

#ifdef CONFIG_PTM_FASTSKB
extern int ptm_pvt_data_pool_consumer, ptm_pvt_data_pool_producer;
#endif
static int sw_counter_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	//int i;

	struct ptm_private *cp = ptm_net_dev->priv;

	printk("TDU interrupt counters \n");
	printk("CH0_F		=0x%08lx		CH1_F	=0x%08lx\n",cp->cp_stats.tdu_int_cnt[0],cp->cp_stats.tdu_int_cnt[1]);
	printk("CH2_F		=0x%08lx		CH3_F	=0x%08lx\n",cp->cp_stats.tdu_int_cnt[2],cp->cp_stats.tdu_int_cnt[3]);
	printk("CH0_S		=0x%08lx		CH1_S	=0x%08lx\n",cp->cp_stats.tdu_int_cnt[4],cp->cp_stats.tdu_int_cnt[5]);
	printk("CH2_S		=0x%08lx		CH3_S	=0x%08lx\n",cp->cp_stats.tdu_int_cnt[6],cp->cp_stats.tdu_int_cnt[7]);

	printk("\nRX interrupt counters \n");
	printk("RXOVF_L_F	=0x%08lx		RXOVF_H_F	=0x%08lx\n",cp->cp_stats.rxovf_int_cnt[3],cp->cp_stats.rxovf_int_cnt[2]);
	printk("RXOVF_L_S	=0x%08lx		RXOVF_H_S	=0x%08lx\n",cp->cp_stats.rxovf_int_cnt[1],cp->cp_stats.rxovf_int_cnt[0]);
	printk("RDU_L_F		=0x%08lx		RDU_H_F	=0x%08lx\n",cp->cp_stats.rdu_int_cnt[3],cp->cp_stats.rdu_int_cnt[2]);
	printk("RDU_L_S		=0x%08lx		RDU_H_S	=0x%08lx\n",cp->cp_stats.rdu_int_cnt[1],cp->cp_stats.rdu_int_cnt[0]);
	printk("ROK_L_F		=0x%08lx		ROK_H_F	=0x%08lx\n",cp->cp_stats.rok_int_cnt[3],cp->cp_stats.rok_int_cnt[2]);
	printk("ROK_L_S		=0x%08lx		ROK_H_S	=0x%08lx\n",cp->cp_stats.rok_int_cnt[1],cp->cp_stats.rok_int_cnt[0]);
	

	printk("\nSW counter\n");
	printk("TX counter	=0x%08lx		TX No Desc counter	=0x%08x\n",cp->cp_stats.tx_cnt,cp->cp_stats.tx_no_desc);
	printk("TX DMA counter	=0x%08x\n",cp->cp_stats.tx_dma_cnt);
	printk("RX Frag Err counter	=0x%08x	RX CRC Err counter		=0x%08x\n",cp->cp_stats.rx_frag_pkt,cp->cp_stats.rx_crcerr_pkt);
	printk("RX Runt Err counter	=0x%08x	RX No Mem counter		=0x%08x\n",cp->cp_stats.rx_runt_pkt,cp->cp_stats.rx_no_mem);

#ifdef CONFIG_PTM_FASTSKB
	printk("\nFast SKB\n");
	printk("producer	=%d		comsume		=%d\n",ptm_pvt_data_pool_producer,ptm_pvt_data_pool_consumer);
#endif

	/*for(i=0;i<8;i++){
		cp->cp_stats.tdu_int_cnt[i]=0;
	}

	cp->cp_stats.tx_cnt = 0;
	cp->cp_stats.tx_no_desc = 0;	
	cp->cp_stats.tx_dma_cnt = 0;
	cp->cp_stats.rx_no_mem = 0;*/

	return len;
}


static int ptm_tx_ring_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	int i=0;
	struct ptm_private *cp = ptm_net_dev->priv;
	DMA_DESC  *txd;

	printk("TX_TAIL = %3d, TX_HEAD = %3d \n",cp->tx_fdps_tail[0],cp->tx_fdps_head[0] );

	for(i=0;i<PTM_TX_RING_SIZE;i++){

		if((i%64)==0) printk("\n [%3d to %3d]",i,(i+63));

		txd = &(cp->tx_fdps_ring[0][i]);
		printk("%s", (txd->opts1 & DescOwn)?"P":"*");
	}	
	printk("\n");
	
	return len;
}

#ifdef CONFIG_PTM_ROMPERF
//static int read_perf_dump(struct file *file, const char *buffer, int count, int *eof, void *data)
static int read_perf_dump(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	unsigned long x;
	struct ptm_private *cp = ptm_net_dev->priv;

	spin_lock_irqsave(cp->lock,x);
	
	rtl8651_romeperfDump(61, 61);

	spin_unlock_irqrestore(cp->lock,x);
	
    	return count;
}
#endif

#define PTM_PROC_DIR_NAME "ptm"
struct proc_dir_entry *ptm_proc_dir=NULL;
static struct proc_dir_entry *driver_version, *hw_counter, *hw_reg, *hw_counter_reset, *sw_cnt, *ptm_ring;
#ifdef CONFIG_PTM_ROMPERF
static struct proc_dir_entry *read_perf;
#endif
/*
	Description : debug tool init
	Input	  :  none
	Output	  :  none
*/
static void ptm_debug_proc_init(void){
	if(ptm_proc_dir==NULL)
		ptm_proc_dir = proc_mkdir(PTM_PROC_DIR_NAME,NULL);
	
	if(ptm_proc_dir)
	{
		driver_version = create_proc_read_entry("driver_version", 0444, ptm_proc_dir, driver_version_read, NULL);
		if(driver_version == NULL){
			printk("can't create proc entry for driver_version\n");
		}
		
		hw_reg = create_proc_read_entry("hw_reg", 0444, ptm_proc_dir, hw_reg_read, NULL);
		if(hw_reg == NULL) {
			printk("can't create proc entry for hw_reg\n");
		}

		hw_counter = create_proc_read_entry("hw_counter", 0444, ptm_proc_dir, hw_counter_read, NULL);
		if(hw_counter == NULL) {
			printk("can't create proc entry for hw_reg\n");
		}

		hw_counter_reset = create_proc_read_entry("hw_counter_reset", 0444, ptm_proc_dir, hw_counter_zero, NULL);
		if(hw_counter_reset == NULL) {
			printk("can't create proc entry for hw_reg\n");
		}

		sw_cnt = create_proc_read_entry("sw_counter", 0444, ptm_proc_dir, sw_counter_read, NULL);
		if(sw_cnt == NULL) {
			printk("can't create proc entry for hw_reg\n");
		}

		ptm_ring = create_proc_read_entry("ptm_tx_ring", 0444, ptm_proc_dir, ptm_tx_ring_read, NULL);
		if(ptm_ring == NULL) {
			printk("can't create proc entry for hw_reg\n");
		}

#ifdef CONFIG_PTM_ROMPERF
		read_perf = create_proc_read_entry("perf_dump", 0444, ptm_proc_dir, read_perf_dump, NULL);
		if(read_perf == NULL) {
			printk("can't create proc entry for hw_reg\n");
		}
#endif

	}
}


int __init ptm_probe (void)
{

	struct net_device *dev;
	struct ptm_private *cp;
	
	int rc;
	void *regs=(void*)PTMBASE;	
	unsigned i;
	unsigned int reg_val;

	printk("%s", version);

	/*Enable PTM module && SACHEM module */
	(*(volatile u32*)(BSP_IP_SEL))|= ( BSP_EN_SACHEM | BSP_EN_PTM);
 
	/* 
	     Tune arbit for performance issues 
	     1. Let Lexra bus own higher priority than CPU
	     2. Let Lexra 1 (PTM) own higher priority than Lexra 0 (GMAC & PKTA )
	     3. Reduce the possibility for PTM RX fifo full problem
	*/
	(*(volatile u32*)(BSP_GIAR0)) = 0x77777777;
	(*(volatile u32*)(BSP_GIAR1)) = 0x88888888;

	(*(volatile u32*)(BSP_LXIAR1)) = 0x00000000;  	/* For PTM original setting */

	reg_val = (*(volatile u32*)(BSP_DCR));			/* Take effect on arbit parameters */
	(*(volatile u32*)(BSP_DCR)) = (reg_val | ARBIT);


	/************** Set PTM related function and device ***************/
	dev = alloc_etherdev(sizeof(struct ptm_private));
	if (!dev)
		return -ENOMEM;

	cp = dev->priv;
	ptm_net_dev=dev;
	
	cp->dev = dev;
	spin_lock_init (&cp->lock);	
	
	dev->base_addr = (unsigned long) regs;
	cp->regs = regs;

	ptm_stop_hw(cp);

	/* read MAC address from EEPROM */
	for (i = 0; i < 3; i++)
		((u16 *) (dev->dev_addr))[i] = i;

	dev->open = ptm_open;
	dev->stop = ptm_close;
	dev->set_multicast_list = ptm_set_rx_mode;
	dev->hard_start_xmit = ptm_start_xmit;
	dev->get_stats = ptm_get_stats;
	my_eth_mac_addr = dev->set_mac_address;
	dev->set_mac_address = ptm_set_mac_addr;
	dev->do_ioctl = ptm_ioctl;
	dev->tx_timeout = ptm_tx_timeout;
	dev->watchdog_timeo = TX_TIMEOUT;
	dev->irq = BSP_PTM_IRQ; 	
	strcpy(dev->name, "ptm0");

	dev->priv_flags = IFF_DOMAIN_WAN;


	rc = register_netdev(dev);
	if (rc)
		goto err_out_iomap;
	printk(KERN_INFO "Register %s as WAN netdev to kernel\n", DRV_NAME);		
	printk (KERN_INFO "%s: %s at 0x%lx, "
		"%02x:%02x:%02x:%02x:%02x:%02x, "
		"IRQ %d\n",
		dev->name,
		"RTL-8672PTM0",
		dev->base_addr,
		dev->dev_addr[0], dev->dev_addr[1],
		dev->dev_addr[2], dev->dev_addr[3],
		dev->dev_addr[4], dev->dev_addr[5],
		dev->irq);

	memset(&cp->rx_tasklets, 0, sizeof(struct tasklet_struct));
	cp->rx_tasklets.func=(void (*)(unsigned long))ptm_rx;
	cp->rx_tasklets.data=(unsigned long)cp;

	ptm_checktxdesc_tasklets.func = (void (*)(unsigned long))ptm_tx;
	ptm_checktxdesc_tasklets.data = (unsigned long)cp;
//#endif

#ifdef CONFIG_PTM_ROMPERF
	rtl8651_romeperfInit();
#endif

	ptm_debug_proc_init();

	return 0;

err_out_iomap:
	iounmap(regs);
	kfree(dev);
	return -1 ;
}


static void __exit ptm_exit (void)
{
	ptm_close(ptm_net_dev);
	
	/* Remove related proc */
	remove_proc_entry("driver_version", ptm_proc_dir);
	remove_proc_entry("hw_reg", ptm_proc_dir);
	remove_proc_entry("hw_counter", ptm_proc_dir);
	remove_proc_entry("hw_counter_reset", ptm_proc_dir);
	remove_proc_entry("sw_counter", ptm_proc_dir);
	remove_proc_entry("ptm_tx_ring", ptm_proc_dir);
	remove_proc_entry(PTM_PROC_DIR_NAME, NULL);

	unregister_netdev(ptm_net_dev);
		
	return;
}
module_init(ptm_probe);
module_exit(ptm_exit);


