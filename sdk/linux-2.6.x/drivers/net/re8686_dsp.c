/* re8670.c: A Linux Ethernet driver for the RealTek 8670 chips. */
/*
   Copyright 2001,2002 Jeff Garzik <jgarzik@mandrakesoft.com>

   Copyright (C) 2001, 2002 David S. Miller (davem@redhat.com) [tg3.c]
   Copyright (C) 2000, 2001 David S. Miller (davem@redhat.com) [sungem.c]
   Copyright 2001 Manfred Spraul				    [natsemi.c]
   Copyright 1999-2001 by Donald Becker.			    [natsemi.c]
   Written 1997-2001 by Donald Becker.			    [8139too.c]
   Copyright 1998-2001 by Jes Sorensen, <jes@trained-monkey.org>. [acenic.c]

   This software may be used and distributed according to the terms of
   the GNU General Public License (GPL), incorporated herein by reference.
   Drivers based on or derived from this code fall under the GPL and must
   retain the authorship, copyright and license notice.  This file is not
   a complete program and may only be used when the entire operating
   system is licensed under the GPL.

   See the file COPYING in this distribution for more information.

   TODO, in rough priority order:
 * dev->tx_timeout
 * LinkChg interrupt
 * Support forcing media type with a module parameter,
 like dl2k.c/sundance.c
 * Implement PCI suspend/resume
 * Constants (module parms?) for Rx work limit
 * support 64-bit PCI DMA
 * Complete reset on PciErr
 * Consider Rx interrupt mitigation using TimerIntr
 * Implement 8139C+ statistics dump; maybe not...
 h/w stats can be reset only by software reset
 * Tx checksumming
 * Handle netif_rx return value
 * ETHTOOL_GREGS, ETHTOOL_[GS]WOL,
 * Investigate using skb->priority with h/w VLAN priority
 * Investigate using High Priority Tx Queue with skb->priority
 * Adjust Rx FIFO threshold and Max Rx DMA burst on Rx FIFO error
 * Adjust Tx FIFO threshold and Max Tx DMA burst on Tx FIFO error
 * Implement Tx software interrupt mitigation via
 Tx descriptor bit
 * The real minimum of CP_MIN_MTU is 4 bytes.  However,
 for this to be supported, one must(?) turn on packet padding.

*/

#define DRV_NAME		"8686"
#define DRV_VERSION		"0.0.1"
#define DRV_RELDATE		"Feb 17, 2012"
#define SLAVE_PHYDMA_OFFSET 	CONFIG_RTL8686_DSP_MEM_BASE

#ifdef CONFIG_DUALBAND_CONCURRENT
#define DRV_DUALBAND	"Dual Band Enable"
#else
#define DRV_DUALBAND	"Dual Band Disable"
#endif


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
#include <linux/in.h>
#include <linux/if_vlan.h>
#include <linux/crc32.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <net/xfrm.h>
#include <linux/proc_fs.h>

#ifdef CONFIG_RTL8672	//shlee 2.6
//#ifndef CONFIG_APOLLO_ROMEDRIVER
#include <bspchip.h>
//#endif
#include <linux/version.h>
#endif
#ifdef CONFIG_RTL865X_ETH_PRIV_SKB
#include "re_privskb.h"
#endif

#include "ethctl_implement.h"
#include "re8686_dsp.h"


#ifdef CONFIG_RTL8686_SWITCH
#include "rtl86900/sdk/include/dal/apollo/raw/apollo_raw_port.h"
#include <rtk/l2.h>
#include <dal/apollomp/dal_apollomp_l2.h>
#include <rtk/switch.h>
#include <rtk/acl.h>
#include <rtk/trap.h>

#include <common/type.h>
#endif

#ifdef CONFIG_RTL8686_SWITCH
#include <net/rtl/rtl8686_switch/apollo_switch_api.h>
#define RTL8686_Switch_Mode_Normal		0x00
#define RTL8686_Switch_Mode_Trap2Cpu	0x01
static unsigned int	SWITCH_MODE	= RTL8686_Switch_Mode_Normal;
#endif
/*static*/ int re8670_start_xmit (struct sk_buff *skb, struct net_device *dev);


#if 0 //def CONFIG_APOLLO_ROMEDRIVER
#include <rtk_rg_fwdEngine.h>
int rg_fwdengine_ret_code=0;
#ifndef __DRAM
#define __DRAM
#endif

#ifndef __IRAM_NIC
#define __IRAM_NIC
#endif

#define IFF_DOMAIN_ELAN   0x1000          /* ELAN interface      */
#define IFF_DOMAIN_WAN    0x2000

#define BSP_GMAC_IRQ		26
#define BSP_EN_GMAC (1<<1)
#define BSP_MC_BASE         0xB8001000
#define BSP_MC_MSRR    (BSP_MC_BASE + 0x038)
#define BSP_MISC_CR_BASE	0xB8003400 //czyao. 0xb8003300->0xb8003400
#define BSP_IP_SEL				(BSP_MISC_CR_BASE + 0x0C)

#define FLUSH_OCP_CMD    (1<<31)

#define SIOCETHTEST 0x89b1

/* 2012-3-13 krammer add 
write data will hold in write buffer of memory controller even if we use uncache access!
so we need to flush the buffer before some case, 
ex: when we finish writing a tx desc, we need to flush write buffer before we kick hw!
*/

#define READ_MEM32(addr)         (*(volatile unsigned int *)   (addr))
#define WRITE_MEM32(addr, val)   (*(volatile unsigned int *)   (addr)) = (val)
#define REG32(reg) 	(*((volatile unsigned int *)(reg)))

static inline void write_buffer_flush(void){
	unsigned int cnt = 100;
	READ_MEM32(BSP_MC_MSRR) |= FLUSH_OCP_CMD;
	while((READ_MEM32(BSP_MC_MSRR) & FLUSH_OCP_CMD) && cnt-- );

	if(!cnt){
		printk("%s %d: write buffer flush timeout!\n", __func__, __LINE__);
	}
}

#endif

/* Jonah + for FASTROUTE */
struct net_device *eth_net_dev;
struct tasklet_struct *eth_rx_tasklets=NULL;

#define WITH_NAPI		""

/* These identify the driver base version and may not be removed. */
static char version[] __devinitdata =
KERN_INFO DRV_NAME " Ethernet driver v" DRV_VERSION " (" DRV_RELDATE ")["DRV_DUALBAND"]" WITH_NAPI "\n";

MODULE_AUTHOR("Krammer Liu <krammer@realtek.comw>");
MODULE_DESCRIPTION("RealTek RTL-8686 series 10/100/1000 Ethernet driver");
MODULE_LICENSE("GPL");

/* Maximum number of multicast addresses to filter (vs. Rx-all-multicast).
   The RTL chips use a 64 element hash table based on the Ethernet CRC.  */
static int multicast_filter_limit = 32;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 9)	//shlee 2.6
module_param(multicast_filter_limit, int, S_IRUGO|S_IWUSR);
#else
	MODULE_PARM (multicast_filter_limit, "i");
#endif
	MODULE_PARM_DESC (multicast_filter_limit, "8686 maximum number of filtered multicast addresses");

#define PFX			DRV_NAME ": "
#define CP_DEF_MSG_ENABLE	(NETIF_MSG_DRV		| \
		NETIF_MSG_PROBE 	| \
		NETIF_MSG_LINK)
#define CP_REGS_SIZE		(0xff + 1)

#define DESC_ALIGN		0x100
#define UNCACHE_MASK		0xa0000000
/*add 1 desc for dummy desc : we will allocate one more desc even if the RING_SIZE is 0, 
this can let the ring which we disable has a dummy desc in its FDP, RD says our NIC need this...*/
#define RE8670_RXRING_BYTES(RING_SIZE)	( (sizeof(struct dma_rx_desc) * (RING_SIZE+1)) + DESC_ALIGN)
#define RE8670_TXRING_BYTES(RING_SIZE)	( (sizeof(struct dma_tx_desc) * (RING_SIZE+1)) + DESC_ALIGN)

#define NEXT_TX(N,RING_SIZE)		(((N) + 1) & (RING_SIZE - 1))
#define NEXT_RX(N,RING_SIZE)		(((N) + 1) & (RING_SIZE - 1))

#define NOT_IN_BITMAP(map, num) (!((map)&(1<<(num))))

static const int RE8670_RX_MRING_SIZE[MAX_RXRING_NUM] = {
RE8670_RX_RING1_SIZE,
RE8670_RX_RING2_SIZE,
RE8670_RX_RING3_SIZE,
RE8670_RX_RING4_SIZE,
RE8670_RX_RING5_SIZE,
RE8670_RX_RING6_SIZE
};
static const int RE8670_TX_MRING_SIZE[MAX_TXRING_NUM] = {
RE8670_TX_RING1_SIZE,
RE8670_TX_RING2_SIZE,
RE8670_TX_RING3_SIZE,
RE8670_TX_RING4_SIZE,
RE8670_TX_RING5_SIZE
};

#if !defined(RTL0371) && RX_NOT_ONLY_RING1
#define RX_MRING_INT_SPLIT
#endif

#define BUILD_BUG_ON_NOT_POWER_OF_2_OR_0(n)                  \
   BUILD_BUG_ON((((n) & ((n) - 1)) != 0))

static inline void check_ring_size(void){
	BUILD_BUG_ON_NOT_POWER_OF_2_OR_0(RE8670_RX_RING1_SIZE);
	BUILD_BUG_ON_NOT_POWER_OF_2_OR_0(RE8670_RX_RING2_SIZE);
	BUILD_BUG_ON_NOT_POWER_OF_2_OR_0(RE8670_RX_RING3_SIZE);
	BUILD_BUG_ON_NOT_POWER_OF_2_OR_0(RE8670_RX_RING4_SIZE);
	BUILD_BUG_ON_NOT_POWER_OF_2_OR_0(RE8670_RX_RING5_SIZE);
	BUILD_BUG_ON_NOT_POWER_OF_2_OR_0(RE8670_RX_RING6_SIZE);
	BUILD_BUG_ON_NOT_POWER_OF_2_OR_0(RE8670_TX_RING1_SIZE);
	BUILD_BUG_ON_NOT_POWER_OF_2_OR_0(RE8670_TX_RING2_SIZE);
	BUILD_BUG_ON_NOT_POWER_OF_2_OR_0(RE8670_TX_RING3_SIZE);
	BUILD_BUG_ON_NOT_POWER_OF_2_OR_0(RE8670_TX_RING4_SIZE);
	BUILD_BUG_ON_NOT_POWER_OF_2_OR_0(RE8670_TX_RING5_SIZE);
}

#define TX_HQBUFFS_AVAIL(CP,ring_num)					\
		(((CP)->tx_Mhqtail[ring_num] - (CP)->tx_Mhqhead[ring_num] + RE8670_TX_MRING_SIZE[ring_num] - 1)&(RE8670_TX_MRING_SIZE[ring_num] - 1))		

#define PKT_BUF_SZ		1536	/* Size of each temporary Rx buffer.*/
#define RX_OFFSET		2


/* The following settings are log_2(bytes)-4:  0 == 16 bytes .. 6==1024, 7==end of packet. */
/* Time in jiffies before concluding the transmitter is hung. */
#define TX_TIMEOUT		(3*HZ)
/* hardware minimum and maximum for a single frame's data payload */
#define CP_MIN_MTU		60	/* TODO: allow lower, but pad */
#define CP_MAX_MTU		4096

enum PHY_REGS{
	FORCE_TX = 1<<7,
	RXFCE= 1<<6,
	TXFCE= 1<<5,
	SP1000= 1<<4,
	SP10= 1<<3,
	LINK=1<<2,
	TXPF=1<<1,
	RXPF=1<<0,
};

enum RE8670_STATUS_REGS
{
	/*TX/RX share */
	DescOwn		= (1 << 31), /* Descriptor is owned by NIC */
	RingEnd		= (1 << 30), /* End of descriptor ring */
	FirstFrag		= (1 << 29), /* First segment of a packet */
	LastFrag		= (1 << 28), /* Final segment of a packet */

	/*Tx descriptor opt1*/
	IPCS		= (1 << 27),
	L4CS		= (1 << 26),
	KEEP		= (1 << 25),
	BLU			= (1 << 24),
	TxCRC		= (1 << 23),
	VSEL		= (1 << 22),
	DisLrn		= (1 << 21),
	CPUTag_ipcs 	= (1 << 20),
	CPUTag_l4cs	= (1 << 19),

	/*Tx descriptor opt2*/
	CPUTag		= (1 << 31),
	aspri		= (1 << 30),
	CPRI		= (1 << 27),
	TxVLAN_int	= (0 << 25),  //intact
	TxVLAN_ins	= (1 << 25),  //insert
	TxVLAN_rm	= (2 << 25),  //remove
	TxVLAN_re	= (3 << 25),  //remark
	//TxPPPoEAct	= (1 << 23),
	TxPPPoEAct	= 23,
	//TxPPPoEIdx	= (1 << 20),
	TxPPPoEIdx	= 20,
	Efid			= (1 << 19),
	//Enhan_Fid	= (1 << 16),
	Enhan_Fid 	= 16,
	/*Tx descriptor opt3*/
	SrcExtPort	= 29,
	TxDesPortM	= 23,
	TxDesStrID 	= 16,
	TxDesVCM	= 0,
	/*Tx descriptor opt4*/
	/*Rx descriptor  opt1*/
	CRCErr	= (1 << 27),
	IPV4CSF		= (1 << 26),
	L4CSF		= (1 << 25),
	RCDF		= (1 << 24),
	IP_FRAG		= (1 << 23),
	PPPoE_tag	= (1 << 22),
	RWT			= (1 << 21),
	PktType		= (1 << 17),
	RxProtoIP	= 1,
	RxProtoPPTP	= 2,
	RxProtoICMP	= 3,
	RxProtoIGMP	= 4,
	RxProtoTCP	= 5,   
	RxProtoUDP	= 6,
	RxProtoIPv6	= 7,
	RxProtoICMPv6	= 8,
	RxProtoTCPv6	= 9,
	RxProtoUDPv6	= 10,
	L3route		= (1 << 16),
	OrigFormat	= (1 << 15),
	PCTRL		= (1 << 14),
	/*Rx descriptor opt2*/
	PTPinCPU	= (1 << 30),
	SVlanTag		= (1 << 29),
	/*Rx descriptor opt3*/
	SrcPort		= (1 << 27),
	DesPortM	= (1 << 21),
	Reason		= (1 << 13),
	IntPriority	= (1 << 10),
	ExtPortTTL	= (1 << 5),
};

enum RE8670_THRESHOLD_REGS{
	//shlee	THVAL		= 2,
	TH_ON_VAL = 0x10,	//shlee flow control assert threshold: available desc <= 16
	TH_OFF_VAL= 0x30,	//shlee flow control de-assert threshold : available desc>=48
	//	RINGSIZE	= 0x0f,	//shlee 	2,
	LOOPBACK	= (0x3 << 8),
	AcceptErr	= 0x20,	     /* Accept packets with CRC errors */
	AcceptRunt	= 0x10,	     /* Accept runt (<64 bytes) packets */
	AcceptBroadcast	= 0x08,	     /* Accept broadcast packets */
	AcceptMulticast	= 0x04,	     /* Accept multicast packets */
	AcceptMyPhys	= 0x02,	     /* Accept pkts with our MAC as dest */
	AcceptAllPhys	= 0x01,	     /* Accept all pkts w/ physical dest */
	AcceptAll = AcceptBroadcast | AcceptMulticast | AcceptMyPhys |  AcceptAllPhys | AcceptErr | AcceptRunt,
	AcceptNoBroad = AcceptMulticast |AcceptMyPhys |  AcceptAllPhys | AcceptErr | AcceptRunt,
	AcceptNoMulti =  AcceptMyPhys |  AcceptAllPhys | AcceptErr | AcceptRunt,
	NoErrAccept = AcceptBroadcast | AcceptMulticast | AcceptMyPhys,
	NoErrPromiscAccept = AcceptBroadcast | AcceptMulticast | AcceptMyPhys |  AcceptAllPhys,
};


enum RE8670_ISR_REGS{
	SW_INT 		= (1 <<10),
	TDU	= (1 << 9),
	LINK_CHG	= (1 <<	8),
	TER		= (1 << 7),
	TOK		= (1 << 6),
	RER_OVF	=(1 << 4),
	RER_RUNT	=(1 << 2),
	RX_OK		= (1 << 0),
	RDU_ALL = (RDU | RDU2 | RDU3 | RDU4 | RDU5 | RDU6),
	RX_ALL = (
		RX_OK | RER_RUNT | RER_OVF |
		((RX_MULTIRING_BITMAP & 1) ? RDU : 0) |
		(RX_MULTIRING_BITMAP >> 1) << 11
	),
};

enum RTL8672GMAC_CPUtag_Control
{
	CTEN_RX     = (1<<31),
	CT_TSIZE	= 27,
	CT_DSLRN	= (1 << 24),
	CT_NORMK	= (1 << 23),
	CT_ASPRI	= (1 << 22),
	CT_APPLO	= (6 << 18),
	CT_8681	= (5 << 18),
	CT_8370S    = (4 <<18),
	CT_8307S	= (3 << 18),
	CT_8306S    = (2<<18),
	CT_8368S    = (1<<18),

	CT_RSIZE_H = 25,
	CT_RSIZE_L = 16,
	CTPM_8306   = (0xf0 << 8),
	CTPM_8368   = (0xe0 << 8),
	CTPM_8370   = (0xff << 8),
	CTPM_8307   = (0xff << 8),
	CTPV_8306   = 0x90,
	CTPV_8368   = 0xa0,
	CTPV_8370   = 0x04,
	CTPV_8307	  = 0x04,
};

enum RTL8672GMAC_PG_REG
{
	EN_PGLBK     = (1<<15),
	DATA_SEL     = (1<<14),
	LEN_SEL      = (1<<11),
	NUM_SEL      = (1<<10),
};


typedef enum
{
	FLAG_WRITE		= (1<<31),
	FLAG_READ		= (0<<31),

	MII_PHY_ADDR_SHIFT	= 26, 
	MII_REG_ADDR_SHIFT	= 16,
	MII_DATA_SHIFT		= 0,
}MIIAR_MASK;

struct rtl8686_dev_table_entry {
	unsigned char 			ifname[IFNAMSIZ];
	unsigned char			ifflag; //0:ELAN, 1:WAN, 2:WLAN
	unsigned short			vid;
	unsigned int			phyPort;
	struct net_device *dev_instant;
};

struct ring_info {
	struct sk_buff		*skb;
	dma_addr_t		mapping;
	unsigned		frag;
};

struct re_dev_private {
	struct re_private* pCp;
	struct net_device_stats net_stats;
	unsigned char txPortMask;
};

static struct re_private re_private_data;
int dev_num = 0;

#define DEV2CP(dev)  (((struct re_dev_private*)dev->priv)->pCp)
#define DEVPRIV(dev)  ((struct re_dev_private*)dev->priv)
#define VTAG2DESC(d) (((((d) & 0x00ff)<<8) | (((d) & 0xff00)>>8)) & 0x0000ffff)
/*warning! +1 for smux.................................................*/
#define VTAG2VLANTCI(v) (( (((v) & 0xff00)>>8) | (((v) & 0x00ff)<<8) ) + 1) 

static void __re8670_set_rx_mode (struct net_device *dev);
static void re8670_tx (struct re_private *cp,int ring_num);

static void re8670_clean_rings (struct re_private *cp);
static void re8670_tx_timeout (struct net_device *dev);
static int change_dev_port_mapping(int port_num ,char* name);

extern atomic_t re8670_rxskb_num;
//int re8670_rxskb_num=0;
#define RE8670_MAX_ALLOC_RXSKB_NUM 2560

#if defined(CONFIG_DUALBAND_CONCURRENT)
#define SKB_BUF_SIZE  1800
#else
#define SKB_BUF_SIZE  1600
#endif

#ifdef CONFIG_RG_JUMBO_FRAME
#define JUMBO_SKB_BUF_SIZE	(13312+2)		//IXIA max packet size.
#endif

unsigned int iocmd_reg=CMD_CONFIG;//=0x4009113d;	//shlee 8672
unsigned int iocmd1_reg=CMD1_CONFIG | (RX_NOT_ONLY_RING1<<25) | TX_RR_scheduler | 0x3f<<16  | txq5_h ; // set RR scheduler and tx ring 5 high queue   

__DRAM unsigned int debug_enable=0;
static unsigned int tx_ring_show_bitmap=((1<<MAX_TXRING_NUM)-1);
static unsigned int rx_ring_show_bitmap=((1<<MAX_RXRING_NUM)-1);

enum RTL8686GMAC_DEBUG_LEVEL{
	RTL8686_PRINT_NOTHING = 0,
	RTL8686_SKB_RX = (1<<0),
	RTL8686_SKB_TX = (1<<1),
	RTL8686_RXINFO = (1<<2),
	RTL8686_TXINFO = (1<<3),
	RTL8686_RX_TRACE = (1<<4),
	RTL8686_TX_TRACE = (1<<5),
};

extern unsigned int SOC_ID;
unsigned int PON_PORT = APOLLO_PON_PORT;/*use 6266's config to be default*/

#ifdef CONFIG_DUALBAND_CONCURRENT
static int vwlan_dev_idx=7;
static unsigned char src_mac[ETH_ALEN]; //= {0x00, 0x11, 0x33, 0x55, 0x77, 0xBB};
#endif

/*Krammer: this table will re-map after port_relate_setting if is 0371*/
static struct rtl8686_dev_table_entry rtl8686_dev_table[] = {
	//ifname, isWan, vid, phyPort, dev_instant
	{"eth", 0, 0, CPU_PORT, NULL},//root dev eth0 must be first
	{"eth0.2",	0, 0, 0, NULL},
	{"eth0.3",	0, 0, 1, NULL},
	{"eth0.4",	0, 0, 2, NULL},
	{"eth0.5",	0, 0, 3, NULL},
	{"nas", 1, 0, APOLLO_PON_PORT, NULL},
	{"pon", 1, 0, APOLLO_PON_PORT, NULL},
#if defined(CONFIG_DUALBAND_CONCURRENT)
	{"vwlan", 0, 0, CPU_PORT, NULL},
#endif
};

#ifdef RX_MRING_INT_SPLIT/*plz add into both define and not define area*/
#define en_rx_mring_int_split() RTL_W32(CONFIG_REG, (RTL_R32(CONFIG_REG) | En_int_split))
//#define set_rring_route() RTL_W32(RRING_ROUTING1, 0x65442100)
#define MASTER_GMAC_PRI_TO_RING ((0x33222111&(~(0xf<<(CONFIG_DEFAULT_TO_SLAVE_GMAC_PRI<<2))))|(4<<(CONFIG_DEFAULT_TO_SLAVE_GMAC_PRI<<2)))
#define SLAVE_GMAC_PRI_TO_RING (0x66555444)
#define set_rring_route() {RTL_W32(RRING_ROUTING1, MASTER_GMAC_PRI_TO_RING);RTL_W32(RRING_ROUTING2, MASTER_GMAC_PRI_TO_RING);RTL_W32(RRING_ROUTING3, SLAVE_GMAC_PRI_TO_RING);RTL_W32(RRING_ROUTING4, MASTER_GMAC_PRI_TO_RING);RTL_W32(RRING_ROUTING5, MASTER_GMAC_PRI_TO_RING);RTL_W32(RRING_ROUTING6, MASTER_GMAC_PRI_TO_RING);}

#define MASK_IMR1_RXALL() RTL_R32(IMR1)&=~((RX_MULTIRING_BITMAP)|((RX_MULTIRING_BITMAP)<<8))
#define UNMASK_IMR1_RXALL() RTL_R32(IMR1)|=((RX_MULTIRING_BITMAP)|((RX_MULTIRING_BITMAP)<<8))
#else
#endif

static inline void mask_rx_int(void){
	//RTL_R16(IMR)&=(u16)(~RX_ALL);
	MASK_IMR1_RXALL();
}

static inline void unmask_rx_int(void){
	//RTL_R16(IMR)|=(u16)(RX_ALL);
	UNMASK_IMR1_RXALL();
}

void memDump (void *start, u32 size, char * strHeader)
{
	int row, column, index, index2, max;
//	uint32 buffer[5];
	u8 *buf, *line, ascii[17];
	char empty = ' ';

	if(!start ||(size==0))
		return;
	line = (u8*)start;

	/*
	16 bytes per line
	*/
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


#undef ETH_DBG
#define ETH_DBG
#ifdef ETH_DBG
static void skb_debug(const struct sk_buff *skb, int enable, int flag)
{
#define NUM2PRINT 1518
	if (unlikely(enable & flag)) {
		if (flag == RTL8686_SKB_RX)
			printk("\nI: ");
		else
			printk("\nO: ");
		printk("eth len = %d eth name %s", skb->len,skb->dev?skb->dev->name:"");
		memDump(skb->data, (skb->len > NUM2PRINT)?NUM2PRINT : skb->len, "");
		if(skb->len > NUM2PRINT){
			printk("........");
		}
		printk("\n");
	}
}

void rxinfo_debug(struct rx_info *pRxInfo, int ring_num)
{
	if (unlikely(debug_enable & (RTL8686_RXINFO))) {
		printk("rxInfo[ring%d]:\n", ring_num);
		printk("opts1\t= 0x%08x Own=%d EOR=%d FS=%d LS=%d RCDF=%d L3R=%d ORG=%d,Len=%d\n", pRxInfo->opts1.dw,
			pRxInfo->opts1.bit.own,
			pRxInfo->opts1.bit.eor,
			pRxInfo->opts1.bit.fs,
			pRxInfo->opts1.bit.ls,
			pRxInfo->opts1.bit.rcdf,
			pRxInfo->opts1.bit.l3routing,
			pRxInfo->opts1.bit.origformat,
			pRxInfo->opts1.bit.data_length);
		printk("addr\t= 0x%08x\n", pRxInfo->addr);
		printk("opts2\t= 0x%08x CpuTag=%d CtagVa=%d CvlanTag=%d\n", pRxInfo->opts2.dw,
			pRxInfo->opts2.bit.cputag,			
			pRxInfo->opts2.bit.ctagva,
			pRxInfo->opts2.bit.cvlan_tag);
		
		printk("opts3\t= 0x%08x SPA=%d DExtPMSK=0x%x Reason=%d IntPri=%d TTL_1=%d\n", pRxInfo->opts3.dw,
			pRxInfo->opts3.bit.src_port_num,	
			pRxInfo->opts3.bit.dst_port_mask,	
			pRxInfo->opts3.bit.reason,	
			pRxInfo->opts3.bit.internal_priority,	
			pRxInfo->opts3.bit.ext_port_ttl_1);
//			dump_hs();
//			dump_lut_table();
	}
}

void txinfo_debug(struct tx_info *pTxInfo)
{

	if (unlikely(debug_enable & (RTL8686_TXINFO))) {
		printk("txInfo:\n");
		printk("opts1\t= 0x%08x Own=%d EOR=%d FS=%d LS=%d IPCS=%d L4CS=%d Keep=%d Len=%d\n", pTxInfo->opts1.dw,
			pTxInfo->opts1.bit.own,
			pTxInfo->opts1.bit.eor,
			pTxInfo->opts1.bit.fs,
			pTxInfo->opts1.bit.ls,
			pTxInfo->opts1.bit.ipcs,
			pTxInfo->opts1.bit.l4cs,
			pTxInfo->opts1.bit.keep,
			pTxInfo->opts1.bit.data_length
		);
		printk("addr\t= 0x%08x \n", pTxInfo->addr);
		printk("opts2\t= 0x%08x AsPri=%d CpuPri=%d VlanAct=%d Pppoe=%d SidIdx=%d Vlan=%d Pri=%d\n", pTxInfo->opts2.dw,
			pTxInfo->opts2.bit.aspri,			
			pTxInfo->opts2.bit.cputag_pri,
			pTxInfo->opts2.bit.tx_vlan_action,
			pTxInfo->opts2.bit.tx_pppoe_action,
			pTxInfo->opts2.bit.tx_pppoe_idx,
			pTxInfo->opts2.bit.vidh<<8|pTxInfo->opts2.bit.vidl,
			pTxInfo->opts2.bit.prio);
		printk("opts3\t= 0x%08x ExtSpa=%d TxPmsk=0x%x L34Keep=%d\n", pTxInfo->opts3.dw,
			pTxInfo->opts3.bit.extspa,
			pTxInfo->opts3.bit.tx_portmask,
			pTxInfo->opts3.bit.l34_keep);

		printk("opts4\t= 0x%08x\n", pTxInfo->opts4.dw);		
	}
}

static char mt_watch_tmp[512];

#define RX_TRACE( comment ,arg...) \
do {\
		if(debug_enable&RTL8686_RX_TRACE)\
		{\
			u8 mt_trace_head_str[]="[NIC RX]\033[1;36m"; \
			int mt_trace_i;\
			sprintf( mt_watch_tmp, comment,## arg);\
			for(mt_trace_i=0;mt_trace_i<512;mt_trace_i++) \
			{\
				if(mt_watch_tmp[mt_trace_i]==0) break; \
			}\
			if(mt_watch_tmp[mt_trace_i-1]=='\n') mt_watch_tmp[mt_trace_i-1]=' ';\
			printk("%s %s \033[1;30m@%s:%d\033[0m\n",mt_trace_head_str,mt_watch_tmp,__FILE__,__LINE__);\
		}\
} while(0);

#define TX_TRACE( comment ,arg...) \
do {\
		if(debug_enable&RTL8686_TX_TRACE)\
		{\
			u8 mt_trace_head_str[]="[NIC TX]\033[1;36m"; \
			int mt_trace_i;\
			sprintf( mt_watch_tmp, comment,## arg);\
			for(mt_trace_i=0;mt_trace_i<512;mt_trace_i++) \
			{\
				if(mt_watch_tmp[mt_trace_i]==0) break; \
			}\
			if(mt_watch_tmp[mt_trace_i-1]=='\n') mt_watch_tmp[mt_trace_i-1]=' ';\
			printk("%s %s \033[1;30m@%s:%d\033[0m\n",mt_trace_head_str,mt_watch_tmp,__FILE__,__LINE__);\
		}\
} while(0);



#define ETHDBG_PRINT(flag, fmt, args...)  if(unlikely(debug_enable & flag)) printk(fmt, ##args)
#define SKB_DBG(args...) skb_debug(args)
#define RXINFO_DBG(args...) rxinfo_debug(args)
#define TXINFO_DBG(args...) txinfo_debug(args)
#else
#define RX_TRACE(args...)
#define TX_TRACE(args...)
#define ETHDBG_PRINT(fmt, args...)
#define SKB_DBG(args...) 
#define RXINFO_DBG(args...)
#define TXINFO_DBG(args...)
#endif 

__IRAM_NIC
struct sk_buff *re8670_getAlloc(unsigned int size)
{	
	struct sk_buff *skb=NULL;
	if ( atomic_read(&re8670_rxskb_num) < RE8670_MAX_ALLOC_RXSKB_NUM ) {
		#ifdef CONFIG_RTL865X_ETH_PRIV_SKB  //For RTL8192CE
		skb = dev_alloc_skb_priv_eth(CROSS_LAN_MBUF_LEN);
		#else
		skb = dev_alloc_skb(size);
		#endif

		if (skb!=NULL) {
			atomic_inc(&re8670_rxskb_num);
			skb->src_port=IF_ETH;
		}
	}
	else {
		printk("%s(%d): limit reached (%d/%d)\n",__FUNCTION__,__LINE__,atomic_read(&re8670_rxskb_num),RE8670_MAX_ALLOC_RXSKB_NUM);
	}
	return skb;
}

static inline void re8670_set_rxbufsize (struct re_private *cp)
{
	unsigned int mtu = cp->dev->mtu;
#if defined(CONFIG_DUALBAND_CONCURRENT)
	cp->dev->mtu = SKB_BUF_SIZE-ETH_HLEN-8;
	mtu = cp->dev->mtu;
#endif
	if (mtu > ETH_DATA_LEN)
		/* MTU + ethernet header + FCS + optional VLAN tag */
		cp->rx_buf_sz = mtu + ETH_HLEN + 8;
	else
		cp->rx_buf_sz = PKT_BUF_SZ;
}

atomic_t lock_tx_tail = ATOMIC_INIT(0);

int re8686_register_rxfunc_by_port(unsigned int port, p2rfunc_t pfunc){
	if(port >= SW_PORT_NUM || !pfunc)
		return -EINVAL;
	re_private_data.port2rxfunc[port] = pfunc;
	return 0;
}

int re8686_register_rxfunc_all_port(p2rfunc_t pfunc){
	unsigned int i;
	int ret;
	if(!pfunc)
		return -EINVAL;
	for(i=0;i<SW_PORT_NUM;i++){
		ret = re8686_register_rxfunc_by_port(i, pfunc);
		if(ret){
			return ret;
		}
	}

	return 0;
}

void re8686_reset_rxfunc_to_default(void){
	unsigned int i;
	for(i=0;i<SW_PORT_NUM;i++){
		re8686_register_rxfunc_by_port(i, re8670_rx_skb);
	}
}

#ifdef CONFIG_GPON_FEATURE
extern short re_omci_reserved_streamid;
#endif

static inline void _tx_additional_setting(struct sk_buff *skb, struct net_device *dev, struct tx_info *pTxInfo){
	pTxInfo->opts2.bit.cputag = 1;

	pTxInfo->opts3.bit.extspa = 2; //from extension port 2
	
	if(pTxInfo->opts3.bit.tx_portmask == 0){
		pTxInfo->opts3.bit.tx_portmask = DEVPRIV(dev)->txPortMask;
	}
	/*should we check vlan_tci means vlan 0 or no vlan?*/
	if((skb->dev->priv_flags & IFF_DOMAIN_WAN) && skb->vlan_tci){
		pTxInfo->opts2.bit.tx_vlan_action = TXD_VLAN_REMARKING;
		pTxInfo->opts2.dw |= VTAG2DESC(skb->vlan_tci);
	}
	if(skb->from_dev){
		if(skb->from_dev->priv_flags & (IFF_DOMAIN_ELAN|IFF_DOMAIN_WAN)){
			pTxInfo->opts1.bit.dislrn = 1; //disable cpu port learning
		}
	}
#if defined(CONFIG_RTK_L34_ENABLE) && defined(CONFIG_RG_API_RLE0371)
	//tysu patch for 0371
	pTxInfo->opts3.bit.tx_portmask=0;
#endif

	//20130724: If NIC TX send by HWLOOKUP, L34 Keep will cause un-except problem.
	if(pTxInfo->opts3.bit.tx_portmask!=0) pTxInfo->opts3.bit.l34_keep = 1;	//ensure switch won't modify or filter packet
	//20141104LUKE: when L34Keep is on, Keep is also needed for gpon.
	if(pTxInfo->opts3.bit.l34_keep)pTxInfo->opts1.bit.keep = 1;
	
	//20131028: If ipv4 packet length <=60, then recaculate data_length in txInfo. Otherwise GMAC will cause L4chksum error
	//20131029: OMCI packet may cause data_length too big, so check streamID to filter them!!
	if(skb->len==60
#ifdef CONFIG_GPON_FEATURE
		&& pTxInfo->opts3.bit.tx_dst_stream_id!=re_omci_reserved_streamid
#endif
		)
	{
		u16 off=12;
		u32 xlen=0;
	
		if((*(u16*)(skb->data+off))==htons(0x8100))//CTAG
	        off+=4;
		if(((*(u16*)(skb->data+off))==htons(0x8863))||((*(u16*)(skb->data+off))==htons(0x8864)))//PPPoE
			off+=8;
		if(((*(u16*)(skb->data+off))==htons(0x0800))||((*(u16*)(skb->data+off))==htons(0x0021)))//IPv4 or IPv4 with PPPoE
		{
			xlen=(off+2+((skb->data[off+4]<<8)|skb->data[off+5]))&0x1ffff;	//l3offset + IPv4 total length
			if(xlen<=60)
				pTxInfo->opts1.bit.data_length=xlen;
		}
	}
}

//#define RE8686_VERIFY
#ifdef RE8686_VERIFY
#include "./re8686_verify.c"
#else
#define INVERIFYMODE 0
#endif

/*__IRAM_NIC*/ void nic_tx2(struct sk_buff *skb)
{
	skb->users.counter=1;
	re8670_start_xmit(skb,eth_net_dev);
}

__IRAM_NIC
static struct net_device* decideRxDevice(struct re_private *cp, struct rx_info *pRxInfo){
	unsigned int num = (pRxInfo->opts3.bit.src_port_num >= SW_PORT_NUM) ? 
		(0) : pRxInfo->opts3.bit.src_port_num ;
	return cp->port2dev[num];
}

static inline void updateRxStatic(struct re_private *cp, struct sk_buff *skb){
	skb->dev->last_rx = jiffies;
	cp->cp_stats.rx_sw_num++;
	DEVPRIV(skb->dev)->net_stats.rx_packets++;
	DEVPRIV(skb->dev)->net_stats.rx_bytes += skb->len;
}

static inline char * getSwitchPort(struct re_private *cp, struct rx_info *pRxInfo)
{
	unsigned int num = (pRxInfo->opts3.bit.src_port_num >= SW_PORT_NUM) ? 
		(0) : pRxInfo->opts3.bit.src_port_num ;
	return rtl8686_dev_table[num+1].dev_instant->name;
}
#ifdef CONFIG_DUALBAND_CONCURRENT
void str2mac(unsigned char *mac_string, unsigned char *MacEntry)
{
	int i,j=0,k=0;
	memset(MacEntry,0,ETH_ALEN);
	for(i=0;i<strlen(mac_string);i++)
	{			
		if(mac_string[i]==':') 
		{
			j=0;
			continue;
		}
		else if((mac_string[i]>='A')&&(mac_string[i]<='F'))
			MacEntry[k]+=(mac_string[i]-'A'+10);
		else if((mac_string[i]>='a')&&(mac_string[i]<='f'))
			MacEntry[k]+=(mac_string[i]-'a'+10);
		else if((mac_string[i]>='0')&&(mac_string[i]<='9'))
			MacEntry[k]+=(mac_string[i]-'0');
		else 
			printk("str2mac MAC string parsing error!");
		if(j==0) MacEntry[k]<<=4;
		if(j==1) k++;
		j++;
	}
}
#endif

__IRAM_NIC
int re8670_rx_skb (struct re_private *cp, struct sk_buff *skb, struct rx_info *pRxInfo)
{
	//used for deleting if necessary.
        struct net_device *net_device_record = NULL;

	skb->dev = decideRxDevice(cp, pRxInfo);
	net_device_record = skb->dev;
#ifdef CONFIG_DUALBAND_CONCURRENT
	if(!memcmp(src_mac, skb->data + ETH_ALEN, ETH_ALEN))
		skb->dev = rtl8686_dev_table[vwlan_dev_idx].dev_instant;
#endif
	skb->vlan_tci = (pRxInfo->opts2.bit.ctagva) ? VTAG2VLANTCI(pRxInfo->opts2.bit.cvlan_tag) : 0;
	ETHDBG_PRINT(RTL8686_SKB_RX, "This packet comes from %s(vlan %u)\n", skb->dev->name, skb->vlan_tci);
	skb->from_dev=skb->dev;

	/* switch_port is patched for iptables and ebtables rule matching */
	skb->switch_port = getSwitchPort(cp, pRxInfo);
	skb->mark = (skb->vlan_tci & 0xFFF);
	//printk("%s %d switch_port: %s vlan_tci=0x%x mark=0x%x\n", 
	//		__func__, __LINE__, skb->switch_port, skb->vlan_tci, skb->mark);


	//Remove all ctag, no matter 4011 (master to slave wifi) or from physical port
	if((*(u16*)&skb->data[12])==0x8100)
	{
		int i;
		for(i=12;i>=4;i-=4)
		{
			*(u32*)&skb->data[i]=*(u32*)&skb->data[i-4];
		}
		skb->len-=4;
		skb->data+=4;
	}

	skb->protocol = eth_type_trans (skb, skb->dev);
	
	//do we need any wan dev rx hacking here?(before pass to netif_rx)
	
	updateRxStatic(cp, skb);
	SKB_DBG(skb, debug_enable, RTL8686_SKB_RX);
	if (netif_rx(skb) == NET_RX_DROP)
		DEVPRIV(net_device_record)->net_stats.rx_dropped++;

	return RE8670_RX_STOP_SKBNOFREE;
}

#ifdef CONFIG_RG_JUMBO_FRAME
static u16 _rtk_rg_standard_chksum(u8 *dataptr, u16 len)
{
  u32 acc;
  u16 src;
  u8 *octetptr;

  acc = 0;

  octetptr = (u8*)dataptr;
  while (len > 1)
  {

    src = (*octetptr) << 8;
    octetptr++;
    src |= (*octetptr);
    octetptr++;
    acc += src;
    len -= 2;
  }
  if (len > 0)
  {
    src = (*octetptr) << 8;
    acc += src;
  }

  acc = (acc >> 16) + (acc & 0x0000ffffUL);
  if ((acc & 0xffff0000) != 0) {
    acc = (acc >> 16) + (acc & 0x0000ffffUL);
  }
  return htons((u16)acc);
}


u16 inet_chksum(u8 *dataptr, u16 len)
{
  u32 acc;

  acc = _rtk_rg_standard_chksum(dataptr, len);
  while (acc >> 16) {
    acc = (acc & 0xffff) + (acc >> 16);
  }
  return (u16)~(acc & 0xffff);
}

#define JUBMO_SPLIT_LEN (1514)

void re8670_splitJumboFrameNetifRx(int num,struct re_private *cp, struct sk_buff *skb, struct rx_info *pRxInfo)
{
	if(skb->len<=1522)
	{
		cp->port2rxfunc[num](cp, skb, pRxInfo);
	}
	else
	{
		int l3Offset;
		int ipv4MoreFragment=0;
		int ipv4FragmentOffset;
		int ipv4TagIf;
		
		int remainLen;
		int l3HdrLen;
		int l23Len;
		int offset;
		int eachPayloadLen;
		int parseOffset=12;


		//parsing packet header
		if((skb->data[parseOffset]==0x81)&&(skb->data[parseOffset+1]==0x00)) parseOffset+=4;
		if((skb->data[parseOffset]==0x88)&&(skb->data[parseOffset+1]==0x64))
		{
			if(skb->data[parseOffset+9]==0x21)
			{
				parseOffset+=10;
				ipv4TagIf=1;
			}
		}
		else if((skb->data[parseOffset]==0x08)&&(skb->data[parseOffset+1]==0x00))
		{
			ipv4TagIf=1;
			parseOffset+=2;			
		}

		if(ipv4TagIf==0)
		{
			goto rx_error;
		}
		else
		{
			l3Offset=parseOffset;
			if(skb->data[l3Offset+6]&0x20) ipv4MoreFragment=1;
			ipv4FragmentOffset=((skb->data[l3Offset+6]&0x1f)<<8)|(skb->data[l3Offset+7]);			
		}

		remainLen=skb->len;
		l3HdrLen=((skb->data[l3Offset]&0xf)<<2);
		l23Len=l3Offset+l3HdrLen;
		offset=ipv4FragmentOffset<<3;
		eachPayloadLen=JUBMO_SPLIT_LEN-l23Len;
		
		eachPayloadLen=(eachPayloadLen>>3)<<3;
		//printk("Jumbo Frame send to NIC len=%d tagif=0x%x l23Len=%d\n",skb->len,pPktHdr->tagif,l23Len);
		
		if(remainLen>JUMBO_SKB_BUF_SIZE) 
		{
			goto rx_error;		
		}

		remainLen-=l23Len;
		
		while(remainLen>0)
		{
			struct sk_buff *allocSkb;
			allocSkb=re8670_getAlloc(SKB_BUF_SIZE);
			if(allocSkb==NULL) goto rx_error;

			skb_reserve(allocSkb,2);
			
			memcpy(allocSkb->data,skb->data,l23Len);
			memcpy(allocSkb->data+l23Len,skb->data+(skb->len-remainLen),eachPayloadLen);
	
			allocSkb->len=((remainLen>=eachPayloadLen)?eachPayloadLen:remainLen)+l23Len;
			//printk("RX %x len=%d remainLen=%d l23Len=%d\n",(u32)allocSkb,allocSkb->len,remainLen,l23Len);
			//memDump(allocSkb->data+l23Len,8,"l4");

			remainLen-=eachPayloadLen;
		
			
			if(offset==0) 
			{
				allocSkb->data[l3Offset+6]|=0x20; //set MF
			}
			else
			{
					
				allocSkb->data[l3Offset+6]=offset>>3>>8; //H_offset
				allocSkb->data[l3Offset+7]=((offset>>3)&0xff); //L_offset

				if((remainLen>0)||(ipv4MoreFragment))
					allocSkb->data[l3Offset+6]|=0x20; //set MF(when org packet have already MF bit)
				
			}

			allocSkb->data[l3Offset+2]=(allocSkb->len-l3Offset)>>8; //IP hdr total H_length
			allocSkb->data[l3Offset+3]=(allocSkb->len-l3Offset)&0xff; //IP hdr total L_length						

			// L3 checksum re-calculate
			*(u16*)((u32)allocSkb->data+l3Offset+10)=0;
			*(u16*)((u32)allocSkb->data+l3Offset+10)=htons(inet_chksum(allocSkb->data+l3Offset,l3HdrLen));
		
			offset+=eachPayloadLen;
			cp->port2rxfunc[num](cp, allocSkb, pRxInfo);			
		}
		dev_kfree_skb_any(skb);
	}
	return;
rx_error:
	dev_kfree_skb_any(skb);
}
#endif

__IRAM_NIC
static void re8670_rx_software (struct re_private *cp, struct sk_buff *skb, struct rx_info *pRxInfo)
{
#if 1
	unsigned int num = (pRxInfo->opts3.bit.src_port_num >= SW_PORT_NUM) ? 
		(0) : pRxInfo->opts3.bit.src_port_num ;

#ifdef CONFIG_RG_JUMBO_FRAME	
	re8670_splitJumboFrameNetifRx(num,cp,skb,pRxInfo);
#else
	cp->port2rxfunc[num](cp, skb, pRxInfo);
#endif

#else
	re8670_rx_skb(cp, skb, pRxInfo);
#endif
}


static 
__IRAM_NIC
unsigned int re8670_rx_csum_ok (struct rx_info *rxInfo)
{
	unsigned int protocol = rxInfo->opts1.bit.pkttype;

	switch(protocol){
		case RxProtoTCP:
		case RxProtoUDP:
		case RxProtoICMP:
		case RxProtoIGMP:/*we check both l4cs and ipv4cs when protocol is ipv4 l4*/
			if(likely((!rxInfo->opts1.bit.l4csf) && (!rxInfo->opts1.bit.ipv4csf))){
				return 1;
			}
			break;
		case RxProtoTCPv6:
		case RxProtoUDPv6:
		case RxProtoICMPv6:/*when protocol is ipv6, we only check l4cs*/
			if(likely(!rxInfo->opts1.bit.l4csf)){
				return 1;
			}
			break;
		default:/*no guarantee when the protocol is only ipv4*/
			break;
	}
	return 0;
}

unsigned char eth_close=0;

// Kaohj -- for polling mode
int eth_poll; // poll mode flag
void eth_poll_schedule(void)
{
	tasklet_hi_schedule(eth_rx_tasklets);
}

static inline void retriveRxInfo(DMA_RX_DESC *desc, struct rx_info *pRxInfo){
	pRxInfo->opts1.dw = desc->opts1;
	pRxInfo->addr= desc->addr;
	pRxInfo->opts2.dw = desc->opts2;
	pRxInfo->opts3.dw = desc->opts3;
}

static inline void updateGmacFlowControl(unsigned rx_tail,int ring_num){
	unsigned int new_cpu_desc_num;		

	if(ring_num==0)
	{    
		new_cpu_desc_num = RTL_R32(EthrntRxCPU_Des_Num);
		new_cpu_desc_num &= 0x00FFFF0F;//clear
		new_cpu_desc_num |= (((rx_tail&0xFF)<<24)|(((rx_tail>>8)&0xF)<<4));//update
		RTL_R32(EthrntRxCPU_Des_Num) = new_cpu_desc_num;
	}
	else
	{
		new_cpu_desc_num = RTL_R32(EthrntRxCPU_Des_Num2+ADDR_OFFSET*(ring_num-1));
		new_cpu_desc_num &= 0xfffff000;//clear
		new_cpu_desc_num |= (((rx_tail&0xFF)|(((rx_tail>>8)&0xF)<<4)));//update
		RTL_W32(EthrntRxCPU_Des_Num2+ADDR_OFFSET*(ring_num-1), new_cpu_desc_num);
	}
}

void vlan_detag(int onoff){
	switch(onoff){
		case ON:
			RTL_W8(CMD, RTL_R8(CMD)|RxVLAN_Detag);
			break;
		case OFF:
			RTL_W8(CMD, RTL_R8(CMD)&~RxVLAN_Detag);
			break;
		default:
			printk("%s %d: wrong arg %d\n", __func__, __LINE__, onoff);
			break;
	}
}

void SetTxRingPrioInRR(void)
{
	iocmd1_reg |= TX_RR_scheduler;
	RTL_W32(IO_CMD1,iocmd1_reg);
}

static inline unsigned char getRxRingBitMap(struct re_private *cp)
{
	unsigned char result = (unsigned char)(((cp->isr1_status)|(RX_RDU_CHECK(cp->isr_status))) & RX_MULTIRING_BITMAP);
	//RX_TRACE("cp->isr1_status=%x cp->isr_status=%x RX_RDU_CHECK(cp->isr_status)=%x RX_MULTIRING_BITMAP=%x result=%x",cp->isr1_status, cp->isr_status, RX_RDU_CHECK(cp->isr_status), RX_MULTIRING_BITMAP,result);	
	cp->isr_status=0;
	cp->isr1_status=0;
	return result;
}

__IRAM_NIC
static void re8670_rx (struct re_private *cp)
{
	int rx_work;
	unsigned long flags;  
	int ring_num=0;
	unsigned rx_Mtail;    
#if RX_NOT_ONLY_RING1
	unsigned char rx_ring_bitmap_current=getRxRingBitMap(cp);
	/*
	we can use this map to decide what sequence we want, that means, queue's priority.
	also, we can use this to decrease some iterations when we split rx interrupt. 
	if u only open rx ring1 and ring6, u can set pri_map = {0,5,1,2,3,4}, and 
	rx_ring_bitmap_current will only has these two bits as well, so we don't need to run "for"
	6 times.
	*/
	unsigned char pri_map[MAX_RXRING_NUM] = {5,4,3,3,3,3};
	int i = 0;
#endif//end of RX_NOT_ONLY_RING1

	spin_lock_irqsave(&cp->rx_lock,flags);   
	//protect eth rx while reboot
	if(unlikely(eth_close == 1)){
		spin_unlock_irqrestore (&cp->rx_lock, flags);   
		return;
	}
	
#if RX_NOT_ONLY_RING1
	for(i=0;rx_ring_bitmap_current && i<MAX_RXRING_NUM;i++){
		ring_num = pri_map[i];

		
		if(NOT_IN_BITMAP(rx_ring_bitmap_current, ring_num))
			continue;


		rx_ring_bitmap_current&=~(1<<ring_num);
#endif //end of RX_NOT_ONLY_RING1
		
		rx_Mtail = cp->rx_Mtail[ring_num];   		

		// Kaohj
		if (unlikely(eth_poll)) // in poll mode
			rx_work = 2; // rx rate is rx_work * 100 pps (timer tick is 10ms)
		else
			rx_work = RE8670_RX_MRING_SIZE[ring_num];


		while (rx_work--)
		{
			u32 len;
			struct sk_buff *skb, *new_skb;
#ifdef CONFIG_RG_JUMBO_FRAME
			struct sk_buff *orig_skb;
#endif			
			DMA_RX_DESC *desc;
			unsigned buflen;
			struct rx_info rxInfo;

			desc = &cp->rx_Mring[ring_num][rx_Mtail];	
			retriveRxInfo(desc, &rxInfo);	

			if (rxInfo.opts1.bit.own)
				break;

                        cp->cp_stats.rx_hw_num++;
                        skb = cp->rx_skb[ring_num][rx_Mtail].skb;			
			if (unlikely(!skb))
                                BUG();

			RXINFO_DBG(&rxInfo, ring_num);

			len = rxInfo.opts1.bit.data_length & 0x0fff;		//minus CRC 4 bytes later

			if (unlikely(rxInfo.opts1.bit.rcdf)){		//DMA error
#ifdef CONFIG_RG_JUMBO_FRAME
				if(re_private_data.jumboLength>0)		//flush jumbo skb
				{
					if(re_private_data.jumboFrame) dev_kfree_skb_any(re_private_data.jumboFrame);
					re_private_data.jumboLength=0;
					re_private_data.jumboFrame=NULL;
			}
#endif
				cp->cp_stats.rcdf++;
				goto rx_next;
			}
			if (unlikely(rxInfo.opts1.bit.crcerr)){		//CRC error
#ifdef CONFIG_RG_JUMBO_FRAME
				if(re_private_data.jumboLength>0)		//flush jumbo skb
				{
					if(re_private_data.jumboFrame) dev_kfree_skb_any(re_private_data.jumboFrame);
					re_private_data.jumboLength=0;
					re_private_data.jumboFrame=NULL;
				}
#endif
				cp->cp_stats.crcerr++;
				goto rx_next;
			}

			buflen = cp->rx_buf_sz + RX_OFFSET;
#ifdef CONFIG_RG_JUMBO_FRAME
			if(rxInfo.opts1.bit.fs==1)	//FS=1
			{
				
				if(rxInfo.opts1.bit.ls==1) ////FS=1, LS=1
				{
					orig_skb=NULL;
					goto rx_to_software;
				}
				else //FS=1, LS=0
				{
					orig_skb=skb;
					//printk("FS=1, LS=0, first frag skb is %d\n",len);
					//memDump(skb->data+RX_OFFSET,64,"skb_start_64");
					//memDump(skb->data+RX_OFFSET+len-16,16,"skb_last_16");
					
					//Init global variables, copy the skb into jumboFrame and free the frag one
					re_private_data.jumboFrame = dev_alloc_skb(JUMBO_SKB_BUF_SIZE);
					if (unlikely(!re_private_data.jumboFrame)) {
						printk("there is no memory for jumbo frame\n");
						cp->cp_stats.rx_no_mem++;
						dma_cache_wback_inv((unsigned long)skb->data,SKB_BUF_SIZE);
						goto rx_next;
					}
					re_private_data.jumboLength=len;
					
					//Copy skb into jumboFrame
					memcpy(re_private_data.jumboFrame->data+RX_OFFSET,skb->data+RX_OFFSET,len);					
				}

			}
			else 	//FS=0
			{
				orig_skb=skb; //keep
				//if the length is zero, JumboFrame is not allocated...
				if(unlikely(re_private_data.jumboLength==0))
				{
					printk("the first skb is not entered or error...\n");
					cp->cp_stats.frag++;
				}
				else
				{
					if(unlikely(re_private_data.jumboLength+len>=JUMBO_SKB_BUF_SIZE))
					{
						printk("total skb is bigger than %d...flush!\n",JUMBO_SKB_BUF_SIZE);
						if(re_private_data.jumboFrame) dev_kfree_skb_any(re_private_data.jumboFrame);
						re_private_data.jumboLength=0;
						re_private_data.jumboFrame=NULL;
						cp->cp_stats.toobig++;
					}
					else
					{
						//Copy into jumboFrame, free the skb						
						memcpy(re_private_data.jumboFrame->data+RX_OFFSET+re_private_data.jumboLength,skb->data,len);
						re_private_data.jumboLength+=len;

						if(rxInfo.opts1.bit.ls==1)		//FS=0, LS=1
						{							
							//printk("FS=0, LS=1, last frag skb is %d\n",len);
							//memDump(skb->data,64,"skb_start_64");
							//memDump(skb->data+len-16,16,"skb_last_16");
							//dev_kfree_skb_any(skb);							
							skb=re_private_data.jumboFrame;
							len=re_private_data.jumboLength;							
							rxInfo.opts1.bit.data_length=re_private_data.jumboLength & 0x3fff;		//at most 14 bits
							re_private_data.jumboLength=0;
							re_private_data.jumboFrame=NULL;
							rxInfo.opts1.bit.fs=1;
							goto rx_to_software;
						}
						else
						{
							//printk("FS=0, LS=0 frag skb is %d\n",len);
							//memDump(skb->data,64,"skb_start_64");
							//memDump(skb->data+len-16,16,"skb_last_16");
						}
						dma_cache_wback_inv((unsigned long)skb->data,SKB_BUF_SIZE);
					}
				}
			}
			goto rx_next;
rx_to_software:
#else
			if (unlikely((rxInfo.opts1.dw & (FirstFrag | LastFrag)) != (FirstFrag | LastFrag))) {
				cp->cp_stats.frag++;
				goto rx_next;
			}
#endif

#ifdef CONFIG_RG_JUMBO_FRAME			
			if(orig_skb==NULL)
				new_skb=re8670_getAlloc(SKB_BUF_SIZE);
			else
				new_skb=orig_skb;
#else				
			new_skb=re8670_getAlloc(SKB_BUF_SIZE);
#endif
			
			if (unlikely(!new_skb)) {
				cp->cp_stats.rx_no_mem++;
				//printk("nic rx new_skb alloc failed no mem %d\n",atomic_read(&re8670_rxskb_num));
				//dma_cache_wback_inv((unsigned long)skb->data,buflen);
				dma_cache_wback_inv((unsigned long)skb->data,SKB_BUF_SIZE);
				goto rx_next;
			}
			


			/* Handle checksum offloading for incoming packets. */
			if (re8670_rx_csum_ok(&rxInfo))
				skb->ip_summed = CHECKSUM_UNNECESSARY;
			else
				skb->ip_summed = CHECKSUM_NONE;

			if(rxInfo.opts1.bit.fs==1)
			{
				skb_reserve(skb, RX_OFFSET); // HW DMA start at 4N+2 only in FS.
			}
			
			len-=4;	//minus CRC 4 bytes here			
			skb_put(skb, len);
			SKB_DBG(skb, debug_enable, RTL8686_SKB_RX);

			RX_TRACE("SKB[%x] DA=%02x:%02x:%02x:%02x:%02x:%02x SA=%02x:%02x:%02x:%02x:%02x:%02x ethtype=%04x len=%d\n",(u32)skb&0xffff
			,skb->data[0],skb->data[1],skb->data[2],skb->data[3],skb->data[4],skb->data[5]
			,skb->data[6],skb->data[7],skb->data[8],skb->data[9],skb->data[10],skb->data[11]
			,(skb->data[12]<<8)|skb->data[13],len);

			cp->rx_Mring[ring_num][rx_Mtail].addr = CPHYSADDR(new_skb->data) + SLAVE_PHYDMA_OFFSET;
			cp->rx_skb[ring_num][rx_Mtail].skb = new_skb;

			//dma_cache_wback_inv((unsigned long)new_skb->data,buflen);
			dma_cache_wback_inv((unsigned long)new_skb->data,SKB_BUF_SIZE);


			// Software flow control for Jumbo Frame RX issue. 
			{
				u16 cdo;
				int drop_ring_size=TH_ON_VAL>>1;
				if(ring_num==0)
					cdo=RTL_R16(RxCDO);
				else
					cdo=RTL_R16(RxCDO2+ADDR_OFFSET*(ring_num-1));

				if(cdo+drop_ring_size<RE8670_RX_MRING_SIZE[ring_num])
				{
					if((cdo<=rx_Mtail)&&(cdo+drop_ring_size>rx_Mtail))
					{
						printk("NIC Full tail=%d cdo=%d\n",rx_Mtail,cdo);
						dev_kfree_skb_any(skb);
						goto rx_next;
					}
				}
				else
				{
					if((cdo<=(rx_Mtail+RE8670_RX_MRING_SIZE[ring_num]))&&(cdo+drop_ring_size>(rx_Mtail+RE8670_RX_MRING_SIZE[ring_num])))
					{
						printk("NIC Full2 tail=%d cdo=%d\n",rx_Mtail,cdo);
						dev_kfree_skb_any(skb);
						goto rx_next;				
					}
				}			
			}

			re8670_rx_software(cp, skb, &rxInfo);
rx_next:
			desc->opts1 = (DescOwn | cp->rx_buf_sz) | ((rx_Mtail == (RE8670_RX_MRING_SIZE[ring_num] - 1))?RingEnd:0);
			updateGmacFlowControl(rx_Mtail,ring_num);
			rx_Mtail = NEXT_RX(rx_Mtail,RE8670_RX_MRING_SIZE[ring_num]);

		}
		cp->rx_Mtail[ring_num] = rx_Mtail;

		if(rx_work <= 0){
			tasklet_hi_schedule(&cp->rx_tasklets);
		}
#if RX_NOT_ONLY_RING1
	}
#endif// end #if RX_NOT_ONLY_RING1
	unmask_rx_int();
	spin_unlock_irqrestore (&cp->rx_lock, flags); 
}

__IRAM_NIC
static irqreturn_t re8670_interrupt(int irq, void * dev_instance, struct pt_regs *regs)
{
	struct net_device *dev = dev_instance;
	struct re_private *cp = DEV2CP(dev);
	u16 status = RTL_R16(ISR);
	u32 status1 = RTL_R32(ISR1);
	cp->isr_status = status;	
	cp->isr1_status = status1;

	if (!status && !status1)  
	{
		//printk("%s: no status indicated in interrupt, weird!\n", __func__);	//shlee 2.6
		return IRQ_RETVAL(IRQ_NONE);
	}

	if ((status & RX_ALL) || (status1 & RX_MULTIRING_BITMAP)) {	
		if(status & RDU_ALL){
			cp->cp_stats.rdu++;
		}
		mask_rx_int();
		tasklet_hi_schedule(&cp->rx_tasklets);
	}

	RTL_W16(ISR,status);
	RTL_W32(ISR1, (cp->isr1_status));
	
	return IRQ_RETVAL(IRQ_HANDLED);
}

static inline void updateTxStatic(struct sk_buff *skb){
	DEVPRIV(skb->dev)->net_stats.tx_packets++;
	DEVPRIV(skb->dev)->net_stats.tx_bytes += skb->len;
}

__IRAM_NIC void re8670_tx (struct re_private *cp,int ring_num)
{
	unsigned tx_tail= cp->tx_Mhqtail[ring_num];
	u32 status;
	struct sk_buff *skb;
	if(unlikely(eth_close))
		return;

	while (!((status = (cp->tx_Mhqring[ring_num][tx_tail].opts1))& DescOwn)) {
		if (tx_tail == cp->tx_Mhqhead[ring_num])
			break;

		skb = cp->tx_skb[ring_num][tx_tail].skb;
		if (unlikely(!skb))   
			break;
		updateTxStatic(skb);		
		dev_kfree_skb_any(skb); 

		cp->tx_skb[ring_num][tx_tail].skb = NULL;
		tx_tail = NEXT_TX(tx_tail,RE8670_TX_MRING_SIZE[ring_num]);
	}
	cp->tx_Mhqtail[ring_num]=tx_tail;

	if (netif_queue_stopped(cp->dev) && (TX_HQBUFFS_AVAIL(cp,ring_num) > (MAX_SKB_FRAGS + 1)))
		netif_wake_queue(cp->dev);
}


__IRAM_NIC void checkTXDesc(int ring_num)
{
	struct re_private *cp = DEV2CP(eth_net_dev);
	if (likely(!atomic_read(&lock_tx_tail))) {
		atomic_set(&lock_tx_tail, 1);
        re8670_tx(cp,ring_num);
		atomic_set(&lock_tx_tail, 0);
	}
}

#ifdef CONFIG_PORT_MIRROR
void nic_tx_mirror(struct sk_buff *skb)
{
	re8670_start_xmit(skb, eth_net_dev);
}
#endif

static 
__IRAM_NIC
void	tx_additional_setting(struct sk_buff *skb, struct net_device *dev, struct tx_info *pTxInfo){
#ifndef RE8686_VERIFY
	_tx_additional_setting(skb, dev, pTxInfo);
#else
	tx_additional_setting_verify_version(skb, dev, pTxInfo);
#endif
	TXINFO_DBG(pTxInfo);
}

static inline void kick_tx(int ring_num){
	switch(ring_num){
		case 0:
		case 1:
		case 2:
		case 3:
			RTL_W32(IO_CMD,iocmd_reg |(1 << ring_num));
			break;
		case 4:
			RTL_W32(IO_CMD1,iocmd1_reg | TX_POLL5);
			break;
		default:
			printk("%s %d: wrong ring num %d\n", __func__, __LINE__, ring_num);
			break;
	}
}

static inline void apply_to_txdesc(DMA_TX_DESC  *txd, struct tx_info *pTxInfo){
	txd->addr = pTxInfo->addr;
	txd->opts2 = pTxInfo->opts2.dw;
	txd->opts3 = pTxInfo->opts3.dw;
	txd->opts4 = pTxInfo->opts4.dw;
	//must be last write....
	wmb();
	txd->opts1 = pTxInfo->opts1.dw;
}

void do_txInfoMask(struct tx_info* pTxInfo, struct tx_info* ptx, struct tx_info* ptxMask){
	if(ptxMask && ptx){
		if(ptxMask->opts1.dw)
		{
			pTxInfo->opts1.dw &= (~ptxMask->opts1.dw);
			pTxInfo->opts1.dw |= (ptx->opts1.dw & ptxMask->opts1.dw);
		}
		if(ptxMask->opts2.dw)
		{
			pTxInfo->opts2.dw &= (~ptxMask->opts2.dw);
			pTxInfo->opts2.dw |= (ptx->opts2.dw & ptxMask->opts2.dw);
		}
		if(ptxMask->opts3.dw)
		{
			pTxInfo->opts3.dw &= (~ptxMask->opts3.dw);
			pTxInfo->opts3.dw |= (ptx->opts3.dw & ptxMask->opts3.dw);
		}
		if(ptxMask->opts4.dw)
		{
			pTxInfo->opts4.dw &= (~ptxMask->opts4.dw);
			pTxInfo->opts4.dw |= (ptx->opts4.dw & ptxMask->opts4.dw);
		}
	}
}

int re8686_send_with_txInfo_and_mask(struct sk_buff *skb, struct tx_info* ptxInfo, int ring_num, struct tx_info* ptxInfoMask)
{
	struct net_device *dev = eth_net_dev;
	struct re_private *cp = DEV2CP(dev);
	unsigned entry;
	u32 eor;
	unsigned long flags;
	struct tx_info local_txInfo={{{0}},0,{{0}},{{0}}};;

	skb->dev = dev;
	//save ptxInfo, now we only need to save opts1 and opts2
	if(ptxInfo!=NULL)memcpy(&local_txInfo, ptxInfo, sizeof(struct tx_info));

	ETHDBG_PRINT( RTL8686_SKB_TX, "tx %s\n", dev->name );
	SKB_DBG(skb, debug_enable, RTL8686_SKB_TX);
	cp->cp_stats.tx_sw_num++;
	checkTXDesc(ring_num);

	if(unlikely(eth_close == 1)) {
		dev_kfree_skb_any(skb);
		DEVPRIV(skb->dev)->net_stats.tx_dropped++;	
		return -1;
	}

	spin_lock_irqsave(&cp->tx_lock, flags);

	if (unlikely(TX_HQBUFFS_AVAIL(cp,ring_num) <= (skb_shinfo(skb)->nr_frags + 1)))
	{
		spin_unlock_irqrestore(&cp->tx_lock, flags);
		dev_kfree_skb_any(skb);
		DEVPRIV(skb->dev)->net_stats.tx_dropped++;
		cp->cp_stats.tx_no_desc++;
		return -1;
	}

	entry = cp->tx_Mhqhead[ring_num];    

	eor = (entry == (RE8670_TX_MRING_SIZE[ring_num] - 1)) ? RingEnd : 0;
	if (skb_shinfo(skb)->nr_frags == 0) {
		u32 len;
		DMA_TX_DESC  *txd;
		txd = &cp->tx_Mhqring[ring_num][entry];    

		len = skb->len;				
		// Kaohj --- invalidate DCache before NIC DMA
		dma_cache_wback_inv((unsigned long)skb->data, len);
		cp->tx_skb[ring_num][entry].skb = skb;
		cp->tx_skb[ring_num][entry].mapping = CPHYSADDR(skb->data);
		cp->tx_skb[ring_num][entry].frag = 0;

		//default setting, always need this
		local_txInfo.addr = CPHYSADDR(skb->data);
		local_txInfo.opts1.dw |= (eor | len | DescOwn | FirstFrag |LastFrag | TxCRC | IPCS);

		//plz put tx additional setting into this function
		tx_additional_setting(skb, dev, &local_txInfo);

		do_txInfoMask(&local_txInfo, ptxInfo, ptxInfoMask);

		//apply to txdesc
		apply_to_txdesc(txd, &local_txInfo);


		TX_TRACE("FROM_FWD[%x] DA=%02x:%02x:%02x:%02x:%02x:%02x SA=%02x:%02x:%02x:%02x:%02x:%02x ethtype=%04x len=%d VlanAct=%d Vlan=%d Pri=%d ExtSpa=%d TxPmsdk=0x%x L34Keep=%x\n",
		(u32)skb&0xffff,
		skb->data[0],skb->data[1],skb->data[2],skb->data[3],skb->data[4],skb->data[5],
		skb->data[6],skb->data[7],skb->data[8],skb->data[9],skb->data[10],skb->data[11],
		(skb->data[12]<<8)|skb->data[13],skb->len,
		local_txInfo.opts2.bit.tx_vlan_action,
		local_txInfo.opts2.bit.vidh<<8|local_txInfo.opts2.bit.vidl,
		local_txInfo.opts2.bit.prio,
		local_txInfo.opts3.bit.extspa,
		local_txInfo.opts3.bit.tx_portmask,
		local_txInfo.opts3.bit.l34_keep);

		entry = NEXT_TX(entry,RE8670_TX_MRING_SIZE[ring_num]);
	} else {
		//re8670_mFrag_xmit(skb, cp, &entry);
		printk("%s %d: not support frag xmit now\n", __func__, __LINE__);
		dev_kfree_skb_any(skb);
	}

	cp->tx_Mhqhead[ring_num] = entry;

	if (unlikely(TX_HQBUFFS_AVAIL(cp,ring_num) <= 1))
		netif_stop_queue(dev);

	//for memory controller's write buffer
	write_buffer_flush();

	spin_unlock_irqrestore(&cp->tx_lock, flags);
	wmb();
	cp->cp_stats.tx_hw_num++;

	kick_tx(ring_num);

	dev->trans_start = jiffies;
	return 0;
}

int re8686_send_with_txInfo(struct sk_buff *skb, struct tx_info* ptxInfo, int ring_num){
	return re8686_send_with_txInfo_and_mask(skb, ptxInfo, ring_num, NULL);
}

#if 0//def CONFIG_APOLLO_ROMEDRIVER
int re8670_start_xmit_common(struct sk_buff *skb, struct net_device *dev, struct tx_info *ptx, struct tx_info *ptxMask);

__IRAM_NIC int rtk_rg_fwdEngine_xmit (struct sk_buff *skb, void *void_ptx, void *void_ptxMask) // (void *) using (rtk_rg_txdesc_t *) casting in romeDriver, using (struct tx_info*) casting in re8686.c.
{
	struct tx_info *ptx=(struct tx_info *)void_ptx;
	struct tx_info *ptxMask=(struct tx_info *)void_ptxMask;			
	skb->cb[0]=1;
	skb->dev = rtl8686_dev_table[0].dev_instant;
	re8670_start_xmit_common(skb,skb->dev,ptx,ptxMask);	
	return 0;
}

__IRAM_NIC int re8670_start_xmit (struct sk_buff *skb, struct net_device *dev)	//shlee temp, fix this later
{
	skb->cb[0]=0;
	re8670_start_xmit_common(skb,skb->dev,NULL,NULL);
	return 0;
}

__IRAM_NIC int re8670_start_xmit_common(struct sk_buff *skb, struct net_device *dev, struct tx_info *ptx, struct tx_info *ptxMask)
#else
__IRAM_NIC int re8670_start_xmit (struct sk_buff *skb, struct net_device *dev)	//shlee temp, fix this later

#endif
{
	struct re_private *cp = DEV2CP(dev);
	unsigned entry;
	u32 eor;
	unsigned long flags;
	struct tx_info txInfo;
	int ring_num=1;
	memset(&txInfo, 0, sizeof(struct tx_info));

	ETHDBG_PRINT( RTL8686_SKB_TX, "tx %s\n", dev->name );
	SKB_DBG(skb, debug_enable, RTL8686_SKB_TX);
	cp->cp_stats.tx_sw_num++;
	checkTXDesc(ring_num);

	if(unlikely(eth_close == 1 || INVERIFYMODE)) {
		dev_kfree_skb_any(skb);
		DEVPRIV(skb->dev)->net_stats.tx_dropped++;	
		return 0;
	}

	spin_lock_irqsave(&cp->tx_lock, flags);

	if (unlikely(TX_HQBUFFS_AVAIL(cp,ring_num) <= (skb_shinfo(skb)->nr_frags + 1)))
	{
		spin_unlock_irqrestore(&cp->tx_lock, flags);
		dev_kfree_skb_any(skb);
		DEVPRIV(skb->dev)->net_stats.tx_dropped++;
		cp->cp_stats.tx_no_desc++;
		return 0;
	}
	
	if(strcmp(skb->dev->name ,"vwlan") == 0)
	{
		struct vlan_ethhdr *veth;
		if (skb_cow_head(skb, VLAN_HLEN) < 0) {
			kfree_skb(skb);
			return NULL;
		}
		veth = (struct vlan_ethhdr *)skb_push(skb, VLAN_HLEN);
		// Move the mac addresses to the beginning of the new header. 
		memmove(skb->data, skb->data + VLAN_HLEN, 2 * VLAN_ETH_ALEN);
		skb->mac_header -= VLAN_HLEN;
		// first, the ethernet type 
		veth->h_vlan_proto = htons(ETH_P_8021Q);
		// now, the TCI 
		veth->h_vlan_TCI = htons(1);
		skb->protocol = htons(ETH_P_8021Q);
	}
	
	
	entry = cp->tx_Mhqhead[ring_num];
	eor = (entry == (RE8670_TX_MRING_SIZE[ring_num] - 1)) ? RingEnd : 0;
	if (skb_shinfo(skb)->nr_frags == 0) {

		DMA_TX_DESC  *txd = &cp->tx_Mhqring[ring_num][entry];
		u32 len;

		len = skb->len;				
		// Kaohj --- invalidate DCache before NIC DMA
		dma_cache_wback_inv((unsigned long)skb->data, len);
		cp->tx_skb[ring_num][entry].skb = skb;
		cp->tx_skb[ring_num][entry].mapping = CPHYSADDR(skb->data) + SLAVE_PHYDMA_OFFSET;
		cp->tx_skb[ring_num][entry].frag = 0;

		//default setting, always need this
		txInfo.addr = CPHYSADDR(skb->data) + SLAVE_PHYDMA_OFFSET;
		
		txInfo.opts1.dw = (eor | len | DescOwn | FirstFrag |LastFrag | TxCRC | IPCS);

#if 0//def CONFIG_APOLLO_ROMEDRIVER
		if(skb->cb[0]) //from RomeDriver
			_rtk_rg_fwdEngineTxDescSetting((void*)&txInfo,(void*)ptx,(void*)ptxMask);
#endif
		//plz put tx additional setting into this function
		tx_additional_setting(skb, dev, &txInfo);

		//apply to txdesc
		apply_to_txdesc(txd, &txInfo);

		entry = NEXT_TX(entry,RE8670_TX_MRING_SIZE[ring_num]);
	} else {
		//re8670_mFrag_xmit(skb, cp, &entry);
		printk("%s %d: not support frag xmit now\n", __func__, __LINE__);
		dev_kfree_skb_any(skb);
	}
	cp->tx_Mhqhead[ring_num] = entry;

#if 0//krammer close this
	if (unlikely(TX_HQBUFFS_AVAIL(cp,ring_num) <= 1))
		netif_stop_queue(dev);
#endif

	//for memory controller's write buffer
	write_buffer_flush();

	spin_unlock_irqrestore(&cp->tx_lock, flags);
	wmb();
	cp->cp_stats.tx_hw_num++;
	
	kick_tx(ring_num);
	
	TX_TRACE("FROM_PS[%x] DA=%02x:%02x:%02x:%02x:%02x:%02x SA=%02x:%02x:%02x:%02x:%02x:%02x ethtype=%04x len=%d Vlan=%d Pri=%d ExtSpa=%d TxPmsdk=0x%x L34Keep=%x\n",
		(u32)skb&0xffff,
		skb->data[0],skb->data[1],skb->data[2],skb->data[3],skb->data[4],skb->data[5],
		skb->data[6],skb->data[7],skb->data[8],skb->data[9],skb->data[10],skb->data[11],
		(skb->data[12]<<8)|skb->data[13],skb->len,
		txInfo.opts2.bit.vidh<<8|txInfo.opts2.bit.vidl,
		txInfo.opts2.bit.prio,
		txInfo.opts3.bit.extspa,
		txInfo.opts3.bit.tx_portmask,
		txInfo.opts3.bit.l34_keep);
	
	dev->trans_start = jiffies;
	return 0;
}

/* Set or clear the multicast filter for this adaptor.
   This routine is not state sensitive and need not be SMP locked. */
static void __re8670_set_rx_mode (struct net_device *dev)
{
	int rx_mode;

	rx_mode = AcceptBroadcast | AcceptMulticast | AcceptMyPhys | AcceptAllPhys;

	/* We can safely update without stopping the chip. */
	// Kao, 2004/01/07
	RTL_W32(MAR0, 0xFFFFFFFF);
	RTL_W32(MAR4, 0xFFFFFFFF);
	RTL_W32(RCR, rx_mode);
}

static void re8670_set_rx_mode (struct net_device *dev)
{
	unsigned long flags;
	struct re_private *cp = DEV2CP(dev);
	spin_lock_irqsave (&cp->rx_lock, flags);
	__re8670_set_rx_mode(dev);
	spin_unlock_irqrestore (&cp->rx_lock, flags);
}

static struct net_device_stats *re8670_get_stats(struct net_device *dev)
{
	return &(DEVPRIV(dev)->net_stats);
}

static void re8670_save_trx_cdo(struct re_private *cp)
{
	int i = 0;
	for(i=0;i<MAX_TXRING_NUM;i++)
	{
		if(!RE8670_TX_MRING_SIZE[i])
		{
			continue;
		}
		cp->old_tx_Mhqhead[i] = cp->tx_Mhqhead[i];
		cp->old_tx_Mhqtail[i] = cp->tx_Mhqtail[i];
	}
	
	for(i=0;i<MAX_RXRING_NUM;i++)
	{
		if(!RE8670_RX_MRING_SIZE[i])
		{
			continue;
		}
		cp->old_rx_Mtail[i] = cp->rx_Mtail[i];
	}

}
static void re8670_load_trx_cdo(struct re_private *cp)
{
	int i = 0;
	for(i=0;i<MAX_TXRING_NUM;i++)
	{
		if(!RE8670_TX_MRING_SIZE[i])
		{
			continue;
		}
		cp->tx_Mhqhead[i] = cp->old_tx_Mhqhead[i];
		cp->tx_Mhqtail[i] = cp->old_tx_Mhqtail[i];
	}
	
	for(i=0;i<MAX_RXRING_NUM;i++)
	{
		if(!RE8670_RX_MRING_SIZE[i])
		{
			continue;
		}
		cp->rx_Mtail[i] = cp->old_rx_Mtail[i];
	}
}

static void re8670_stop_hw (struct re_private *cp)
{
	int j;
	RTL_W32(IO_CMD,0); /* timer  rx int 1 packets*/
	RTL_W32(IO_CMD1,0); //czyao
	RTL_W16(IMR, 0);
	RTL_W32(IMR0, 0);
	RTL_W16(ISR, 0xffff);
	RTL_W32(ISR1, 0xffffffff);
	//synchronize_irq();
	synchronize_irq(cp->dev->irq);/*linux-2.6.19*/
	udelay(10);

	for(j=0;j<MAX_RXRING_NUM;j++)
		cp->rx_Mtail[j] = 0;		
	for(j=0;j<MAX_TXRING_NUM;j++)
		cp->tx_Mhqhead[j] = cp->tx_Mhqtail[j] = 0;
}

static void re8670_reset_hw (struct re_private *cp)
{
	/*apollo use this for totaly gmac reset, in old method, mring can't receive packet at first time packet coming*/
	REG32(BSP_IP_SEL) &= ~BSP_EN_GMAC;
	mdelay(10);
	REG32(BSP_IP_SEL) |= BSP_EN_GMAC;
}
static inline void re8670_start_hw (struct re_private *cp)
{
	RTL_W32(IO_CMD1,iocmd1_reg | (RX_MULTIRING_BITMAP << 16));
	RTL_W32(IO_CMD,iocmd_reg); /* timer  rx int 1 packets*/
	//printk("Start NIC in Pkt Processor disabled mode.. IO_CMD %x\n", RTL_R32(IO_CMD));	
}


static void multi_rtx_ring_init(struct re_private *cp)
{
	u16 desc_l, desc_h;
	int i;


	for(i=0;i<MAX_TXRING_NUM;i++)
	{
		#ifdef CONFIG_DUALBAND_CONCURRENT
		if(!cp->tx_Mhqring[i]){
			continue;
		}
		#endif
		RTL_W32(TxFDP1+(ADDR_OFFSET*i),CPHYSADDR(cp->tx_Mhqring[i]) + SLAVE_PHYDMA_OFFSET);
		RTL_W16(TxCDO1+(ADDR_OFFSET*i),0);
	}

	for(i=0;i<MAX_RXRING_NUM;i++)
	{
		#ifdef CONFIG_DUALBAND_CONCURRENT
		if(!cp->rx_Mring[i]){
			continue;
		}
		#endif
		/*we set flow control even if we don't enable this queue.........
		this is because we want to prevent triggering flow control of the queue we disable.....*/
		if(i==0)
		{
			desc_l = (RE8670_RX_MRING_SIZE[i] > 256) ? 256 : RE8670_RX_MRING_SIZE[i];
			desc_h = (RE8670_RX_MRING_SIZE[i] > 256) ? (RE8670_RX_MRING_SIZE[i]/256)%16 : 1;
			RTL_W32(RxFDP, CPHYSADDR(cp->rx_Mring[0]) + SLAVE_PHYDMA_OFFSET);
			RTL_W32(EthrntRxCPU_Des_Num,0);  //initialize the register content
			RTL_W8(EthrntRxCPU_Des_Wrap,(TH_ON_VAL%256));
			RTL_W8(Rx_Pse_Des_Thres,(TH_OFF_VAL%256));
			RTL_W8(EthrntRxCPU_Des_Num,desc_l-1);
			RTL_W8(RxRingSize,desc_l-1);
			RTL_W8(EthrntRxCPU_Des_Wrap_h,(TH_ON_VAL/256)%16);
			RTL_W8(Rx_Pse_Des_Thres_h,(TH_OFF_VAL/256)%16);
			RTL_W8(EthrntRxCPU_Des_Num_h,(RTL_R8(EthrntRxCPU_Des_Num_h)|(desc_h-1)<<4));
			RTL_W8(RxRingSize_h,desc_h-1);    
		}
		else
		{
			RTL_W32(RxFDP2+(ADDR_OFFSET*(i-1)), CPHYSADDR(cp->rx_Mring[i]) + SLAVE_PHYDMA_OFFSET);
			RTL_W32(EthrntRxCPU_Des_Wrap2+(ADDR_OFFSET*(i-1)),0);  //initialize the register content
			RTL_W32(EthrntRxCPU_Des_Wrap2+(ADDR_OFFSET*(i-1)),((TH_ON_VAL%256)<<16)|(TH_OFF_VAL%256));
			RTL_W32(EthrntRxCPU_Des_Num2+(ADDR_OFFSET*(i-1)),RE8670_RX_MRING_SIZE[i]-1);
			RTL_W16(RxRingSize2+(ADDR_OFFSET*(i-1)),RE8670_RX_MRING_SIZE[i]-1);
		}
	}

}
static void re8670_init_hw (struct re_private *cp)
{
//	struct net_device *dev = cp->dev;
	UNMASK_IMR1_RXALL();

	multi_rtx_ring_init(cp);  
}

static int re8670_refill_rx (struct re_private *cp)
{
	unsigned i;
	unsigned j;

	for(j=0;j<MAX_RXRING_NUM;j++)
	{
		for (i = 0; i < RE8670_RX_MRING_SIZE[j]; i++) {
			struct sk_buff *skb;
			skb = re8670_getAlloc(SKB_BUF_SIZE);
			if (!skb)
				goto err_out;
			// Kaohj --- invalidate DCache for uncachable usage
			//ql_xu
			dma_cache_wback_inv((unsigned long)skb->data, SKB_BUF_SIZE);
			skb->dev = cp->dev;
			cp->rx_skb[j][i].skb = skb;
			cp->rx_skb[j][i].frag = 0;
			if ((u32)skb->data &0x3)
				printk(KERN_DEBUG "skb->data unaligment %8x\n",(u32)skb->data);

			cp->rx_Mring[j][i].addr = CPHYSADDR(skb->data) + SLAVE_PHYDMA_OFFSET;

			if (i == (RE8670_RX_MRING_SIZE[j] - 1))          
				cp->rx_Mring[j][i].opts1 = (DescOwn | RingEnd | SKB_BUF_SIZE);
			else
				cp->rx_Mring[j][i].opts1 =(DescOwn | SKB_BUF_SIZE);
			cp->rx_Mring[j][i].opts2 = 0;

		}    
	}
	return 0;
err_out:
	re8670_clean_rings(cp);
	return -ENOMEM;
}

static void re8670_tx_timeout (struct net_device *dev)
{
	//unsigned long flags;
	struct re_private *cp = DEV2CP(dev);
	#if 0//krammer test
	unsigned tx_head;
	unsigned tx_tail;
	
	cp->cp_stats.tx_timeouts++;

	spin_lock_irqsave(&cp->lock, flags);
	tx_head = cp->tx_hqhead;
	tx_tail = cp->tx_hqtail;
	while (tx_tail != tx_head) {
		struct sk_buff *skb;
		u32 status;
		rmb();
		status = (cp->tx_hqring[tx_tail].opts1);
		if (status & DescOwn)
			break;
		skb = cp->tx_skb[tx_tail].skb;
		if (!skb)
			BUG();
		DEVPRIV(dev)->net_stats.tx_packets++;	

		dev_kfree_skb(skb);
		cp->tx_skb[tx_tail].skb = NULL;
		tx_tail = NEXT_TX(tx_tail);
	}

	cp->tx_hqtail = tx_tail;

	spin_unlock_irqrestore(&cp->lock, flags);
	#endif
	if (netif_queue_stopped(cp->dev))
		netif_wake_queue(cp->dev);

}

static int re8670_init_rings (struct re_private *cp)
{
	int j;
	for(j=0;j<MAX_TXRING_NUM;j++){
		cp->tx_Mhqhead[j] = cp->tx_Mhqtail[j] = 0;
	}	

	for(j=0;j<MAX_RXRING_NUM;j++){
		cp->rx_Mtail[j] = 0;		
	}	
	return re8670_refill_rx (cp);
}

static int re8670_alloc_rings (struct re_private *cp)
{
	void*	pBuf;
	int j;

#ifdef CONFIG_DESC_IN_SRAM
	unsigned char* s_begin = (unsigned char*)0xbf002000;

	//use segment 4 8K SRAM here.
	REG32(0xb8004030) = (((unsigned int )s_begin)&(0x1ffffffe))|1;
	REG32(0xb8004034) = 0x06; 
	REG32(0xB8004038) = 0x36000;
	REG32(0xb8001330) = (((unsigned int )s_begin)&(0x1ffffffe))|1;
	REG32(0xb8001334) = 0x06;
	memset((unsigned char*)s_begin, 0, 8*1024);
#endif

	for(j=0;j<MAX_RXRING_NUM;j++)
	{    
		#ifdef CONFIG_DUALBAND_CONCURRENT
		if(!RE8670_RX_MRING_SIZE[j]){
			continue;
		}
		#endif
#ifdef CONFIG_DESC_IN_SRAM
		if(j == 0)
			pBuf = s_begin;//first 4K for rx ring
		else
			pBuf = kzalloc(RE8670_RXRING_BYTES(RE8670_RX_MRING_SIZE[j]), GFP_KERNEL|__GFP_NOFAIL);
#else
		pBuf = kzalloc(RE8670_RXRING_BYTES(RE8670_RX_MRING_SIZE[j]), GFP_KERNEL|__GFP_NOFAIL);
#endif
		if (!pBuf)
			goto ErrMem;
		dma_cache_wback_inv((unsigned long)pBuf, RE8670_RXRING_BYTES(RE8670_RX_MRING_SIZE[j]));
		cp->rxdesc_Mbuf[j] = pBuf;
		pBuf = (void*)( (u32)(pBuf + DESC_ALIGN-1) &  ~(DESC_ALIGN -1) ) ;
		cp->rx_Mring[j] = (DMA_RX_DESC*)((u32)(pBuf) | UNCACHE_MASK);
		if(RE8670_RX_MRING_SIZE[j]){
			pBuf=(struct ring_info*)kzalloc(sizeof(struct ring_info)*RE8670_RX_MRING_SIZE[j],GFP_KERNEL);
			if (!pBuf)
				goto ErrMem;
			cp->rx_skb[j]= pBuf;
		}
	}

	for(j=0;j<MAX_TXRING_NUM;j++)
	{
		if(!RE8670_TX_MRING_SIZE[j])
			continue;
		#ifdef 	CONFIG_DESC_IN_SRAM
		pBuf = s_begin + 0x1000;
		#else
		pBuf = kzalloc(RE8670_TXRING_BYTES(RE8670_TX_MRING_SIZE[j]), GFP_KERNEL|__GFP_NOFAIL);
		#endif
		if (!pBuf)
			goto ErrMem;
		dma_cache_wback_inv((unsigned long)pBuf, RE8670_TXRING_BYTES(RE8670_TX_MRING_SIZE[j]));
		cp->txdesc_Mbuf[j] = pBuf;
		pBuf = (void*)( (u32)(pBuf + DESC_ALIGN-1) &  ~(DESC_ALIGN -1) ) ;
		cp->tx_Mhqring[j] = (DMA_TX_DESC*)((u32)(pBuf) | UNCACHE_MASK);
		
		pBuf =(struct ring_info*)kzalloc(sizeof(struct ring_info)*RE8670_TX_MRING_SIZE[j],GFP_KERNEL);
		if (!pBuf)
			goto ErrMem;
		cp->tx_skb[j] = pBuf;
	}

	return re8670_init_rings(cp);

ErrMem:

	for(j=0;j<MAX_RXRING_NUM;j++)
	{
#ifndef CONFIG_DESC_IN_SRAM
		if (cp->rxdesc_Mbuf[j])
			kfree(cp->rxdesc_Mbuf[j]);
#endif
		if (cp->rx_skb[j])    
			kfree(cp->rx_skb[j]);
	}        

	for(j=0;j<MAX_TXRING_NUM;j++)
	{        
#ifndef CONFIG_DESC_IN_SRAM
		if (cp->txdesc_Mbuf[j])
			kfree(cp->txdesc_Mbuf[j]);
#endif
		if (cp->tx_skb[j])    
			kfree(cp->tx_skb[j]);	
	}
	return -ENOMEM;

}

static void re8670_clean_rings (struct re_private *cp)
{
	unsigned i,j;
	for (j = 0; j < MAX_RXRING_NUM; j++) {
		if(cp->rx_skb[j]){
			for (i = 0; i < RE8670_RX_MRING_SIZE[j]; i++) {
				if (cp->rx_skb[j][i].skb) {
					dev_kfree_skb(cp->rx_skb[j][i].skb);
				}
			}
			memset(cp->rx_skb[j], 0, sizeof(struct ring_info) * RE8670_RX_MRING_SIZE[j]);		
		}
	}
	for (j = 0; j < MAX_TXRING_NUM; j++) {
		if(cp->tx_skb[j]){
			for (i = 0; i < RE8670_TX_MRING_SIZE[j]; i++) {
				if (cp->tx_skb[j][i].skb) {
					struct sk_buff *skb = cp->tx_skb[j][i].skb;
					DEVPRIV(skb->dev)->net_stats.tx_dropped++;
					dev_kfree_skb(skb);
				}
			}
			memset(cp->tx_skb[j], 0, sizeof(struct ring_info) * RE8670_TX_MRING_SIZE[j]);	
		}
	}
}

static void re8670_free_rings (struct re_private *cp)
{
	int j;
	re8670_clean_rings(cp);
	/*pci_free_consistent(cp->pdev, CP_RING_BYTES, cp->rx_ring, cp->ring_dma);*/

	for(j=0;j<MAX_RXRING_NUM;j++)
	{
#ifndef CONFIG_DESC_IN_SRAM
		if(cp->rxdesc_Mbuf[j])
			kfree(cp->rxdesc_Mbuf[j]);
#endif
		cp->rx_Mring[j] = NULL;   

		if (cp->rx_skb[j])    
			kfree(cp->rx_skb[j]);
		cp->rx_skb[j]=NULL;
	}     

	for(j=0;j<MAX_TXRING_NUM;j++)        
	{
#ifndef CONFIG_DESC_IN_SRAM
		if(cp->txdesc_Mbuf[j])
			kfree(cp->txdesc_Mbuf[j]);
#endif
		cp->tx_Mhqring[j] = NULL;

		if (cp->tx_skb[j])    
			kfree(cp->tx_skb[j]);
		cp->tx_skb[j]=NULL;	
	}    

}

static int re8670_open (struct net_device *dev)
{
	struct re_private *cp = DEV2CP(dev);
	int rc=0;

	eth_close=0;
	printk("open \n");
	if (netif_msg_ifup(cp))
		printk(KERN_DEBUG "%s: enabling interface\n", dev->name);

	if(dev_num == 0){
		printk("%s %d\n", __func__, __LINE__);
		re8670_set_rxbufsize(cp);	/* set new rx buf size */
		/*
		// allocate memory when probing 
     		rc = re8670_alloc_rings(cp);
		if (rc)
		{
			printk("re8670_alloc_rings failed\n");
			return rc;
		}
		*/
		re8670_init_hw(cp);
		re8670_load_trx_cdo(cp);
		//rc = request_irq(dev->irq, (irq_handler_t)re8670_interrupt, IRQF_DISABLED, dev->name, dev);
		rc = request_irq(dev->irq, (irq_handler_t)re8670_interrupt, IRQF_DISABLED, eth_net_dev->name, eth_net_dev);
		if (rc)
		{
			printk("request_irq failed\n");
			goto err_out_hw;
		}
	}
	dev_num++;
	netif_start_queue(dev);
	return 0;

err_out_hw:
	printk("got error and stop hw & free rings\n");
	re8670_stop_hw(cp);
	re8670_free_rings(cp);
	return rc;
}

static int re8670_close (struct net_device *dev)
{
	struct re_private *cp = DEV2CP(dev);
	dev_num--;

	if(dev_num == 0){
		eth_close=1;
		netif_stop_queue(dev);
		//re8670_stop_hw(cp);  remove stop_hw with HW ip reset , 
		free_irq(eth_net_dev->irq, eth_net_dev);
		re8670_save_trx_cdo(cp);

		//re8670_free_rings(cp);
		printk("%s %d\n", __func__, __LINE__);
	}

	if (netif_msg_ifdown(cp))
		printk(KERN_DEBUG "%s: disabling interface\n", dev->name);

	return 0;
}

int re8670_ioctl (struct net_device *dev, struct ifreq *rq, int cmd)
{
	int rc = 0;

	if (!netif_running(dev) && cmd!=SIOCETHTEST)
		return -EINVAL;

	switch (cmd) {
		case SIOCETHTEST:
			eth_ctl((struct eth_arg *)rq->ifr_data);
			break;
		default:
			rc = -EOPNOTSUPP;
			break;
	}

	return rc;
}

static int dbg_level_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len;

	len = sprintf(page, "[debug_enable = 0x%08x]\n", debug_enable);
	len += sprintf(page + len, "RTL8686_PRINT_NOTHING\t0x%08x\n", RTL8686_PRINT_NOTHING);
	len += sprintf(page + len, "RTL8686_SKB_RX\t\t0x%08x\n", RTL8686_SKB_RX);
	len += sprintf(page + len, "RTL8686_SKB_TX\t\t0x%08x\n", RTL8686_SKB_TX);
	len += sprintf(page + len, "RTL8686_RXINFO\t\t0x%08x\n", RTL8686_RXINFO);
	len += sprintf(page + len, "RTL8686_TXINFO\t\t0x%08x\n", RTL8686_TXINFO);
	len += sprintf(page + len, "RTL8686_RX_TRACE\t\t0x%08x\n", RTL8686_RX_TRACE);
	len += sprintf(page + len, "RTL8686_TX_TRACE\t\t0x%08x\n", RTL8686_TX_TRACE);

	return len;
}
static int dbg_level_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	unsigned char tmpBuf[16] = {0};
	int len = (count > 15) ? 15 : count;

	if (buffer && !copy_from_user(tmpBuf, buffer, len))
	{
		debug_enable=simple_strtoul(tmpBuf, NULL, 16);
		printk("write debug_enable to 0x%08x\n", debug_enable);
		return count;
	}
	return -EFAULT;
}

static int memrw_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	return 0;
}
static int memrw_write(struct file *file, const char *buff, unsigned long len, void *data)
{
	char 		tmpbuf[64];
	unsigned int	*mem_addr, mem_data, mem_len;
	char		*strptr, *cmd_addr;
	char		*tokptr;

	if (buff && !copy_from_user(tmpbuf, buff, len)) {
		tmpbuf[len] = '\0';
		strptr=tmpbuf;
		cmd_addr = strsep(&strptr," ");
		if (cmd_addr==NULL)
		{
			goto errout;
		}
		printk("cmd %s\n", cmd_addr);
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}

		if (!memcmp(cmd_addr, "r", 1))
		{
			mem_addr=(unsigned int*)simple_strtol(tokptr, NULL, 0);
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			mem_len=simple_strtol(tokptr, NULL, 0);
			memDump(mem_addr, mem_len, "");
		}
		else if (!memcmp(cmd_addr, "w", 1))
		{
			mem_addr=(unsigned int*)simple_strtol(tokptr, NULL, 0);
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			mem_data=simple_strtol(tokptr, NULL, 0);
			WRITE_MEM32(mem_addr, mem_data);
			printk("Write memory 0x%p dat 0x%x: 0x%x\n", mem_addr, mem_data, READ_MEM32(mem_addr));
		}
		else
		{
			goto errout;
		}
	}
	else
	{
errout:
		printk("Memory operation only support \"r\" and \"w\" as the first parameter\n");
		printk("Read format:	\"r mem_addr length\"\n");
		printk("Write format:	\"w mem_addr mem_data\"\n");
	}

	return len;
}
#ifdef CONFIG_RTL8686_SWITCH
static int switch_mode_read(char *page, char **start, off_t off, 
		int count, int *eof, void *data)
{	  
      int len;	 
	  switch(SWITCH_MODE)
	  {
		case RTL8686_Switch_Mode_Trap2Cpu:
			len = sprintf(page, "RLE0371 asic switch mode : TRAP2CPU \n");
			break;
		case RTL8686_Switch_Mode_Normal:
			len = sprintf(page, "RLE0371 asic switch mode : NORMAL \n");
			break;
		default:
			len = sprintf(page, "RLE0371 asic switch mode : Unknown (BUG!!) \n");
	  }      
      if (len <= off+count) *eof = 1;
      *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
      return len;
}
static int switch_control_set_mode(int mode)
{
	if( mode!=RTL8686_Switch_Mode_Trap2Cpu
		&& mode!=RTL8686_Switch_Mode_Normal)
		return -1;	
	SWITCH_MODE = mode;
	return 0;
}
static int switch_mode_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char 	tmpbuf[512];
	char		*strptr;	
	int retval = -1;
	if (buff && !copy_from_user(tmpbuf, buff, len))
	{
		tmpbuf[len] = '\0';
		strptr=tmpbuf;
		if(strlen(strptr)==0)
		{
			goto errout;
		}
		if(strncmp(strptr, "trap2cpu",8) == 0)
		{
			retval = switch_control_set_mode(RTL8686_Switch_Mode_Trap2Cpu);
			printk("ccwei: %s-%d traps2cpu\n",__func__,__LINE__);
			vlan_detag(ON);
			//switch_cpu();
			change_dev_port_mapping(0,"eth0.2");
			change_dev_port_mapping(1,"eth0.3");
			change_dev_port_mapping(4,"eth0.4");
			change_dev_port_mapping(5,"eth0.5");
			change_dev_port_mapping(PON_PORT,"nas0");						
		}
		else if(strncmp(strptr, "normal",6) == 0)
		{
			retval = switch_control_set_mode(RTL8686_Switch_Mode_Normal);
			printk("ccwei: %s-%d normal\n",__func__,__LINE__);
			vlan_detag(OFF);
			//switch_hw();
		}
		else
		{
			goto errout;
		}
		if(retval==0)
			printk("write success ! \n");
		else
			printk("error : change mode fail ! \n");
	}
	else
	{
errout:
		printk("error input  (permit/trap/normal)\n");
	}
	return len;
}

int ACL_RAW_RULE_entry_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{

	int len=0;
#if 0
	int seq, ret, idx;
	rtk_acl_ingress_entry_t AclRule;

	len = seq = ret = idx = 0;

	memset(&AclRule, 0, sizeof(rtk_acl_ingress_entry_t));
	for(idx = 0; idx < HAL_MAX_NUM_OF_ACL_RULE_ENTRY(); ++idx)
	{
		AclRule.index = idx;
		//if(RT_ERR_OK != rtk_acl_igrRuleEntry_get(&AclRule))
		//	return len;

		//ret = rtk_acl_igrRuleEntry_get(&AclRule);

		//printk("acl get result: 0x%x\n", ret);

		if(RT_ERR_OK != ret)
			return len;

		// UNCOMMENT THE FOLLOWINT TWO LINE TO MAKE THE PRINT EASIER!!!
		//if(DISABLED == AclRule.valid)
			//continue;

		printk("%3d.VALID(%1d) TEMPLATE[%1d] MBR[0x%02x] \n\r \
\t RAW VALUE: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n\r \
\t RAW MASK : 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n\n",
					AclRule.index,
					AclRule.valid,
					AclRule.templateIdx,
					AclRule.activePorts.bits[0],
					AclRule.readField.fieldRaw[0].value, AclRule.readField.fieldRaw[1].value,AclRule.readField.fieldRaw[2].value, AclRule.readField.fieldRaw[3].value,
					AclRule.readField.fieldRaw[4].value, AclRule.readField.fieldRaw[5].value,AclRule.readField.fieldRaw[6].value, AclRule.readField.fieldRaw[7].value,
					AclRule.readField.fieldRaw[0].mask,  AclRule.readField.fieldRaw[1].mask, AclRule.readField.fieldRaw[2].mask,  AclRule.readField.fieldRaw[3].mask,
					AclRule.readField.fieldRaw[4].mask,  AclRule.readField.fieldRaw[5].mask, AclRule.readField.fieldRaw[6].mask,  AclRule.readField.fieldRaw[7].mask
					);

	}
#endif
	return len;
}


int ACL_RAW_TEMPLATE_entry_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len=0;
#if 0
	int seq, ret, idx, total;

	rtk_acl_template_t tempR;

	len = seq = ret = idx = total = 0;

	total = HAL_MAX_NUM_OF_ACL_TEMPLATE();

	//printk("Total num is %d\n", total);
	
	memset(&tempR, 0, sizeof(rtk_acl_template_t));
	for(idx = 0; idx < total; ++idx)
	{
		tempR.index = idx;
		//if(RT_ERR_OK != rtk_acl_template_get(&tempR))
		//	return len;

		ret = rtk_acl_template_get(&tempR);

		//printk("acl get result: 0x%x\n", ret);

		if(RT_ERR_OK != ret)
			return len;

		printk("%3d.RAW TEMPLATE: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n\n",
					tempR.index,
					tempR.fieldType[0], tempR.fieldType[1], tempR.fieldType[2], tempR.fieldType[3], 
					tempR.fieldType[4], tempR.fieldType[5], tempR.fieldType[6], tempR.fieldType[7]
					);
	}
#endif
	return len;
}


int ACL_RAW_FLDSLCT_entry_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len=0;
#if 0
	int seq, ret, idx;
	
	rtk_acl_field_entry_t FieldEntry;

	len = seq = ret = idx = 0;

	memset(&FieldEntry, 0 ,sizeof(rtk_acl_field_entry_t));
	for(idx = 0; idx < HAL_MAX_NUM_OF_FIELD_SELECTOR(); ++idx )
	{
		FieldEntry.index = idx;

		ret = rtk_acl_fieldSelect_get(&FieldEntry);

		if(RT_ERR_OK != ret)
			return len;

		printk("%3d. FIELD SELECT: %1d OFFSET: %3d Byte \n", 
					FieldEntry.index,
					FieldEntry.format,
					FieldEntry.offset);
	}
#endif
	return len;
}



int IPMulticast_entry_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len=0;
#if 0
	int muladdr_idx, seq, ret, temp_muladdr_idx;
	
	rtk_l2_ipMcastAddr_t	L2McastData;
	rtk_l2_ipmcMode_t 	Mode;
	
	muladdr_idx = len = seq = ret = temp_muladdr_idx = 0;

	ret = rtk_l2_ipmcMode_get(&Mode);
	printk("IP Multicast MODE: %s\n\n", 
				(LOOKUP_ON_MAC_AND_VID_FID == Mode) ? "LOOKUP_ON_MAC_AND_VID_FID" : 
					((LOOKUP_ON_DIP_AND_SIP == Mode) ? "LOOKUP_ON_DIP_AND_SIP" : "LOOKUP_ON_DIP_AND_VID"));

	//ret = rtk_l2_ipmcGroupLookupMissHash_get(&hashR);
	//printk("IPMulticast Group LOOKUP Method: %s\n", (HASH_DIP_AND_SIP == hashR) ? "HASH_DIP_AND_SIP" : "HASH_DIP_ONLY" );

	printk("RTL8686 Multicast Table:\n");
	while(seq < APOLLO_L2_LEARN_LIMIT_CNT_MAX && muladdr_idx < APOLLO_L2_LEARN_LIMIT_CNT_MAX)
	{
		temp_muladdr_idx = muladdr_idx;
	
		ret = rtk_l2_nextValidIpMcastAddr_get(&muladdr_idx, &L2McastData);

		//printk("ret is 0x%x\n", ret);
		//printk("the muladdr_idx is %d\n", muladdr_idx);

		if(RT_ERR_L2_ENTRY_NOTFOUND == ret)
			break;
		
		//won't check the first time
		if(seq != 0 && muladdr_idx <= temp_muladdr_idx)
			break;

		seq++;
		muladdr_idx++;
		
		if(RT_ERR_OK == ret)
			printk("   %4d.[%4d] 0x%x MBR(0x%02x) EXTMBR(0x%02x) VID(%1d) FLAG(0x%02x)\n",
							seq,
							L2McastData.index,
							L2McastData.dip,
							L2McastData.portmask.bits[0],
							L2McastData.ext_portmask.bits[0],
							L2McastData.vid,
							L2McastData.flags);
	}
#endif
	return len;
}


int  IPMulticast_entry_write(struct file *file, const char *buff, unsigned long len, void *data)
{
#if 0
	char 	tmpbuf[64];
	char	*strptr;
	char	*tokptr;

	rtk_l2_ipMcastAddr_t ipmcAddr; 
	unsigned int dip;

	int ret = 0;


	ret = rtk_l2_ipmcMode_set(LOOKUP_ON_DIP_AND_SIP);
	//printk("ipmcMode ret is %d\n", ret);
	
	if (buff && !copy_from_user(tmpbuf, buff, len))
	{
		int i;

		unsigned int port_mask;
	
		tmpbuf[len] = '\0';
		
		strptr=tmpbuf;
		//split the first string(dotnet addr)
		if(NULL == (tokptr = strsep(&strptr," ")))
			goto errout;

		if(0 == (dip = simple_strtol(tokptr, NULL, 0)))
			goto errout;
			
		//split the second string(port)
		if(NULL == (tokptr = strsep(&strptr," ")))
			goto errout;

		if(0 == (port_mask = simple_strtol(tokptr, NULL, 0)))
			goto errout;

		memset(&ipmcAddr, 0, sizeof(rtk_l2_ipMcastAddr_t));
		
		ipmcAddr.dip = dip;
		ipmcAddr.flags |= RTK_L2_IPMCAST_FLAG_STATIC
							| RTK_L2_IPMCAST_FLAG_DIP_ONLY;
							
		for(i = 0; i < SW_PORT_NUM; ++i)
		{	
			if(port_mask & (1 << i))
				rtk_switch_port2PortMask_set(&ipmcAddr.portmask, i);
		}
	
		//rtk_switch_port2PortMask_set(&ipmcAddr.portmask, RTK_PORT_UTP0); 
		//rtk_switch_port2PortMask_set(&ipmcAddr.portmask, RTK_PORT_UTP1); 
		//rtk_switch_port2PortMask_set(&ipmcAddr.portmask, RTK_PORT_UTP2); 
		//rtk_switch_port2PortMask_set(&ipmcAddr.portmask, RTK_PORT_UTP3);

		ret = rtk_l2_ipMcastAddr_add(&ipmcAddr);

		//printk("ret is 0x%x\n", ret);

		if(RT_ERR_OK == ret)
			printk("Add [DIP:0x%x PORTMASK:0x%x] to HW\n", ipmcAddr.dip, ipmcAddr.portmask.bits[0]);
		else
			printk("Failed to ADD [DIP:0x%x PORTMASK:0x%x] to HW\n", ipmcAddr.dip, ipmcAddr.portmask.bits[0]);
	
	}
	else
	{
	
errout:
		printk("Example: \n");
		printk("    To enable DIP 224.1.2.3 forward by hardware to port 1 and 2\n");
		printk("    echo 0xE0010203 0x03 > /proc/multicast\n");
	}
#endif
	return len;
}


int l2_entry_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
     int len=0;
#if 0
	//make sure l2_idx is inited as 0;
	int l2_idx, len, seq, ret, temp_l2_idx, cnt;

	rtk_l2_ucastAddr_t  L2UcastData;

	l2_idx = len = seq = ret = temp_l2_idx = 0;

	ret = rtk_l2_limitLearningCnt_get(&cnt);
	//printk("ret of is rtk_l2_limitLearningCnt_get %d\n", ret);
	//printk("limitLearningCnt is %d\n", cnt);
	
	printk("RTL8686 l2 Table:\n");
	while(seq < APOLLO_L2_LEARN_LIMIT_CNT_MAX && l2_idx < APOLLO_L2_LEARN_LIMIT_CNT_MAX)
	{
		temp_l2_idx = l2_idx;
	
		ret = rtk_l2_nextValidAddr_get(&l2_idx, &L2UcastData);

		//printk("ret of rtk_l2_nextValidAddr_get: 0x%x\n", ret);
		//printk("l2_dix is %d\n", l2_idx);

		if(RT_ERR_L2_ENTRY_NOTFOUND == ret)
			break;
		
		//won't check the first time
		if(seq != 0 && l2_idx <= temp_l2_idx)
			break;

		seq++;
		l2_idx++;
		
		if(RT_ERR_OK == ret)
			printk("   %4d.[%4d] %02x:%02x:%02x:%02x:%02x:%02x PORT(%1d) VID(%1d) FID(%2d) EFID(%1d) FLAG(0x%x) AGE(%1d)\n",
							seq,
							L2UcastData.index,
							L2UcastData.mac.octet[0],L2UcastData.mac.octet[1],L2UcastData.mac.octet[2],
							L2UcastData.mac.octet[3],L2UcastData.mac.octet[4],L2UcastData.mac.octet[5],
							L2UcastData.port,
							L2UcastData.vid,
							L2UcastData.fid,
							L2UcastData.efid,
							L2UcastData.flags,
							L2UcastData.age);
	}
#endif
	return len;
}
int port_status_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{

	int len = 0;
  #if 0
	unsigned int i = 0;

	rtk_port_linkStatus_t linkStatus;
	rtk_port_speed_t  Speed;
    rtk_port_duplex_t Duplex;
    rtk_enable_t      TxStatus;
    rtk_enable_t      RxStatus;
    rtk_enable_t	nwayAbility;
 
	//apollo_raw_port_ability_t Ability;
	//spppd can work fine
	// should we display the cpu port ability?
	for(i=0;i<SW_PORT_NUM;i++){

		//apollo_raw_port_ability_get(i, &Ability);
		rtk_port_link_get(i, &linkStatus);
		printk("Port%d : %s \n", i, (linkStatus == PORT_LINKUP) ? "LinkUp" : "LinkDown");

		if(PORT_LINKUP == linkStatus)
		{
			if(CPU_PORT != i)
				if(RT_ERR_OK == rtk_port_phyAutoNegoEnable_get(i, &nwayAbility))
					printk("   N-way is %s\n", (ENABLED == nwayAbility) ? "Enabled" : "Disabled");

			if(RT_ERR_OK == rtk_port_flowctrl_get(i, &TxStatus, &RxStatus))
				printk("   TxFlowCtl %s | RxFlowCtrl %s\n", (ENABLED == TxStatus)?"Enabled" : "Disabled", 
															(ENABLED == RxStatus)?"Enabled" : "Disabled");

			if(RT_ERR_OK == rtk_port_speedDuplex_get(i, &Speed, &Duplex))
				printk("   Duplex %s | Speed %s\n", (PORT_HALF_DUPLEX == Duplex)?"Half" : "FULL", 
														(PORT_SPEED_1000M == Speed) ? "1G" : ((PORT_SPEED_10M == Speed) ? "10M" : "100M"));
			
			//len += sprintf((page + len), "\t Mode : %s \n", (ENABLED == Ability.linkFib1g)?"FIBER":"UTP");
#if 0
			printk("   TxFlowCtl %s | RxFlowCtrl %s\n", (ENABLED == TxStatus)?"Enabled" : "Disabled", (ENABLED == RxStatus)?"Enabled" : "Disabled");
			printk("   Duplex %s | Speed %s\n", (PORT_HALF_DUPLEX == Duplex)?"Half" : "FULL", 
														(PORT_SPEED_1000M == Speed) ? "1G" : ((PORT_SPEED_10M == Speed) ? "10M" : "100M"));
#endif
		}
	}
#endif
	return len;
}

static u32 rtl_ethtool_get_port_link(struct net_device *dev)
{
	//unsigned int i;
	unsigned char txportmask = DEVPRIV(dev)->txPortMask;

	if(txportmask == 0){//eth0, always link, have problem?
		return 1;
	}
#if 0
	for(i=0;i<SW_PORT_NUM;i++){
		if(txportmask & (1<<i)){
			//return get_port_link(i);//first match bit
			return 1;
		}
	}
#endif
	//krammer change to default link up
	//return 0;//default link down
	return 1;//default link down
}



static int rtl_ethtool_get_settings(struct net_device *dev, struct ethtool_cmd *ecmd)
{
#if 0
	unsigned int  portnum;
	unsigned int  tmp_spd;

    apollo_raw_port_ability_t Ability;

	unsigned char txportmask = DEVPRIV(dev)->txPortMask;

	if(0 == txportmask)
		return 0;

	//we only treat the first match bit;
	for(portnum = 0; portnum < SW_PORT_NUM; ++portnum)
	{
		if(txportmask & (1 << portnum))
			break;
	}

	if(portnum > SW_PORT_NUM)
		return 0;

	apollo_raw_port_ability_get(portnum, &Ability);

	ecmd->supported = SUPPORTED_10baseT_Half 		
							|	SUPPORTED_10baseT_Full 	
							|	SUPPORTED_100baseT_Half 	
							|	SUPPORTED_100baseT_Full
							|	SUPPORTED_1000baseT_Full
							|	SUPPORTED_Autoneg 	
							|	SUPPORTED_Pause
							|	((PON_PORT == portnum) ? SUPPORTED_FIBRE : SUPPORTED_TP );
		                   
	ecmd->advertising = ADVERTISED_10baseT_Half 
							|	ADVERTISED_10baseT_Full
							|	ADVERTISED_100baseT_Full
							|	ADVERTISED_100baseT_Half
							|	ADVERTISED_Autoneg
							|	ADVERTISED_Pause
							|	((PON_PORT == portnum) ? SUPPORTED_FIBRE : SUPPORTED_TP );

	if(PON_PORT == portnum)
		ecmd->port = PORT_FIBRE;
	else
		ecmd->port = PORT_TP;

	ecmd->autoneg = (Ability.nwayAbility) ? AUTONEG_ENABLE : AUTONEG_DISABLE;

	ecmd->duplex = Ability.duplex ? DUPLEX_FULL : DUPLEX_HALF;

	ecmd->transceiver = XCVR_INTERNAL;

	ecmd->phy_address = portnum;

	if(PON_PORT == portnum)
		tmp_spd = Ability.linkFib1g ? SPEED_1000 : SPEED_100;
	else
		tmp_spd = (PORT_SPEED_100M == Ability.speed) ? SPEED_100 : 
						((PORT_SPEED_10M == Ability.speed) ? SPEED_10 : SPEED_1000);

	//if the port is not linked, we set the speed as "UNKOWN!".
	if(rtl_ethtool_get_port_link(dev))
		ecmd->speed = tmp_spd;
	else
		ecmd->speed = 0;
#endif
	return 0;	
}


#else
static u32 rtl_ethtool_get_port_link(struct net_device *dev)
{
	return 1;//always link, have problem?
}
#endif

static const struct ethtool_ops rtl_ethtool_ops = {
	.get_link 			= rtl_ethtool_get_port_link,
#ifdef CONFIG_RTL8686_SWITCH
	.get_settings 		= rtl_ethtool_get_settings,
#endif
};

void rtl_set_ethtool_ops(struct net_device *netdev)
{
	SET_ETHTOOL_OPS(netdev, &rtl_ethtool_ops);
}

static int misc_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	printk("iocmd_reg 0x%x\n", iocmd_reg);
	printk("iocmd1_reg 0x%x\n", iocmd1_reg);
	return 0;
}
static int misc_write(struct file *file, const char *buff, unsigned long len, void *data)
{
	char 	tmpbuf[64];
	char	*strptr, *var;
	char	*tokptr;
	int i;
	unsigned int value;
	#define VAR_NUM 8
	char var_name[VAR_NUM][64]={"iocmd_reg", "iocmd1_reg", ""};
	unsigned int* pVar[VAR_NUM] = {&iocmd_reg, &iocmd1_reg, NULL};

	if (buff && !copy_from_user(tmpbuf, buff, len)) {
		tmpbuf[len] = '\0';
		strptr=tmpbuf;
		var = strsep(&strptr," ");
		if (var==NULL)
		{
			goto errout;
		}
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}

		for(i=0;strlen(var_name[i])&&pVar[i];i++){
			if(!memcmp(var, var_name[i], strlen(var_name[i]))){
				value = simple_strtol(tokptr, NULL, 0);
				*pVar[i] = value;
				break;
			}
		}

		if(!pVar[i]){
			goto errout;
		}

	}
	else
	{
errout:
		printk("iocmd_reg 0x%x\n", iocmd_reg);
		printk("iocmd1_reg 0x%x\n", iocmd1_reg);
	}

	return len;
}


static int dev_port_mapping_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	unsigned int i;
	unsigned int totalDev = ((sizeof(rtl8686_dev_table))/(sizeof(struct rtl8686_dev_table_entry)));

	printk("PON PORT %d, CPU PORT %d\n", PON_PORT, CPU_PORT);
	printk("DEV ability: ");
	for(i=0;i<totalDev;i++){
		printk("%s ", rtl8686_dev_table[i].dev_instant->name);
	}
	printk("\n");
	
	printk("rx: phyPort -> dev[the packet rx from phyPort will send to kernel using dev]\n");
	for(i=0;i<SW_PORT_NUM;i++){
		printk("port%d -> %s\n", i, re_private_data.port2dev[i]->name);
	}
	
	printk("tx:dev -> txPortMask[when tx from dev, we will use this txPortMask]\n");
	for(i=0;i<totalDev;i++){
		printk("%s -> 0x%x\n", 
			rtl8686_dev_table[i].dev_instant->name, DEVPRIV(rtl8686_dev_table[i].dev_instant)->txPortMask);
	}
	
	return 0;
}
static int dev_port_mapping_write(struct file *file, const char *buff, unsigned long len, void *data)
{
	char 		tmpbuf[64];
	char		*strptr;
	unsigned int port_num;//, i;
	char		*tokptr;

	if (buff && !copy_from_user(tmpbuf, buff, len)) {
		tmpbuf[len] = '\0';
		strptr=tmpbuf;
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}
		port_num = simple_strtol(tokptr, NULL, 0);
		printk("port %d ", port_num);
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}
		if(tokptr[strlen(tokptr) - 1] == 0x0a)
			tokptr[strlen(tokptr) - 1] = 0x00;
		printk("assign to %s\n", tokptr);
		change_dev_port_mapping(port_num,tokptr);	
	}
	else
	{
errout:
		printk("num_of_port dev_name\n");
	}

	return len;
}

static int change_dev_tx_port_mask(int port_num ,char* name, int index)
{
	if(strcmp(name, "eth0")){
		DEVPRIV(rtl8686_dev_table[index].dev_instant)->txPortMask = 1 << port_num;
		printk("%s -> 0x%x\n", 
			rtl8686_dev_table[index].dev_instant->name, DEVPRIV(rtl8686_dev_table[index].dev_instant)->txPortMask);
	}
	return 0;
}
/*ccwei: add dev port mapping api*/
static int change_dev_port_mapping(int port_num ,char* name)
{
	unsigned int totalDev = ((sizeof(rtl8686_dev_table))/(sizeof(struct rtl8686_dev_table_entry)));
	char* dev_name;
	int i;
	for(i=0;i<totalDev;i++){
		if(rtl8686_dev_table[i].dev_instant)
			dev_name = rtl8686_dev_table[i].dev_instant->name;
		else{
			printk("no dev_instant, strange.......\n");
			dev_name = rtl8686_dev_table[i].ifname;
		}
		if(!strcmp(dev_name, name)){
			re_private_data.port2dev[port_num] = rtl8686_dev_table[i].dev_instant;
			change_dev_tx_port_mask(port_num,name,i);
			break;
		}
	}
	if(i == totalDev){
		printk("can't find dev %s\n", name);
		return -1;
	}
	return 0;
}
static int port_to_rxfunc_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	unsigned int totalPortTable = SW_PORT_NUM;
	unsigned int i;
	//default set to eth0
	for(i=0;i<totalPortTable;i++){
		printk("port%d -> 0x%p\n", i, re_private_data.port2rxfunc[i]);
	}
	return 0;
}

static int hw_reg_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	int i;
	printk("ETHBASE		=0x%08x\n", ETHBASE);
	printk("IDR		=%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x\n", 
		RTL_R8(IDR0), RTL_R8(IDR1), RTL_R8(IDR2), RTL_R8(IDR3), RTL_R8(IDR4), RTL_R8(IDR5));
	printk("MAR		=%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x\n", 
		RTL_R8(MAR0), RTL_R8(MAR1), RTL_R8(MAR2), RTL_R8(MAR3), 
		RTL_R8(MAR4), RTL_R8(MAR5), RTL_R8(MAR6), RTL_R8(MAR7));
	printk("TXOKCNT		=0x%04x		RXOKCNT		=0x%04x\n", RTL_R16(TXOKCNT), RTL_R16(RXOKCNT));
	printk("TXERR		=0x%04x		RXERRR		=0x%04x\n", RTL_R16(TXERR), RTL_R16(RXERRR));
	printk("MISSPKT		=0x%04x		FAE		=0x%04x\n", RTL_R16(MISSPKT), RTL_R16(FAE));
	printk("TX1COL		=0x%04x		TXMCOL		=0x%04x\n", RTL_R16(TX1COL), RTL_R16(TXMCOL));
	printk("RXOKPHY		=0x%04x		RXOKBRD		=0x%04x\n", RTL_R16(RXOKPHY), RTL_R16(RXOKBRD));
	printk("RXOKMUL		=0x%04x		TXABT		=0x%04x\n", RTL_R16(RXOKMUL), RTL_R16(TXABT));
        printk("TXUNDRN         =0x%04x         RDUMISSPKT      =0x%04x\n", RTL_R16(TXUNDRN), RTL_R16(RDUMISSPKT));
        printk("TRSR            =0x%08x\n", RTL_R32(TRSR));
	printk("CMD		=0x%02x		IMR		=0x%04x\n", RTL_R8(CMD), RTL_R16(IMR));
	printk("ISR		=0x%04x		TCR		=0x%08x\n", RTL_R16(ISR), RTL_R32(TCR));
	printk("ISR1		=0x%08x	IMR0		=0x%08x\n", RTL_R32(ISR1), RTL_R32(IMR0));
	printk("RCR		=0x%08x	CPUtagCR	=0x%08x\n", RTL_R32(RCR), RTL_R32(CPUtagCR));
	printk("CONFIG_REG	=0x%08x	CPUtag1CR	=0x%08x\n", RTL_R32(CONFIG_REG), RTL_R32(CPUtag1CR));
	printk("MSR		=0x%08x	VLAN_REG	=0x%08x\n", RTL_R32(MSR), RTL_R32(VLAN_REG));
	printk("LEDCR		=0x%08x\n", RTL_R32(LEDCR));

	for(i=0;i<MAX_TXRING_NUM;i++)
	{
		printk("TxFDP%d		=0x%08x	TxCDO%d		=0x%04x\n",i,RTL_R32(TxFDP1+(ADDR_OFFSET*i)),i,RTL_R16(TxCDO1+(ADDR_OFFSET*i)));
	}
	printk("SMSA		=0x%08x\n", RTL_R32(SMSA));
	for(i=0;i<MAX_RXRING_NUM;i++){
		if(i==0)
		{
			printk("RxRingSize%d	=0x%04x\n", i, RTL_R16(RxRingSize));
			printk("RxFDP%d		=0x%08x	RxCDO%d		=0x%04x\n",i,RTL_R32(RxFDP), i, RTL_R16(RxCDO));
			printk("EthrntRxCPU_Des_Num	=0x%02x	EthrntRxCPU_Des_Wrap	=0x%02x\n", 
				RTL_R8(EthrntRxCPU_Des_Num), RTL_R8(EthrntRxCPU_Des_Wrap));
			printk("Rx_Pse_Des_Thres	=0x%02x	EthrntRxCPU_Des_Num_h	=0x%02x\n", 
				RTL_R8(Rx_Pse_Des_Thres), RTL_R8(EthrntRxCPU_Des_Num_h));
			printk("Rx_Pse_Des_Thres_h	=0x%02x\n", RTL_R8(Rx_Pse_Des_Thres_h));
		}
		else
		{
			printk("RxRingSize%d	=0x%04x\n", i, RTL_R16(RxRingSize2+(ADDR_OFFSET*(i-1))));
			printk("RxFDP%d		=0x%08x	RxCDO%d		=0x%04x\n",i,RTL_R32(RxFDP2+(ADDR_OFFSET*(i-1))),i,RTL_R16(RxCDO2+(ADDR_OFFSET*(i-1))));
			printk("RxCPU_Des_Num%d	=0x%08x	RxCPU_Des_Thres%d=0x%08x\n", 
				i, RTL_R32(EthrntRxCPU_Des_Num2+(ADDR_OFFSET*(i-1))), 
				i, RTL_R32(EthrntRxCPU_Des_Wrap2+(ADDR_OFFSET*(i-1))));
		}
	}
	printk("RRING_ROUTING1	=0x%08x	RRING_ROUTING2	=0x%08x\n", RTL_R32(RRING_ROUTING1), RTL_R32(RRING_ROUTING2));
	printk("RRING_ROUTING3	=0x%08x	RRING_ROUTING4	=0x%08x\n", RTL_R32(RRING_ROUTING3), RTL_R32(RRING_ROUTING4));
	printk("RRING_ROUTING5	=0x%08x	RRING_ROUTING6	=0x%08x\n", RTL_R32(RRING_ROUTING5), RTL_R32(RRING_ROUTING6));
	printk("IO_CMD		=0x%08x	IO_CMD1		=0x%08x\n", RTL_R32(IO_CMD), RTL_R32(IO_CMD1));
	printk("DIAG1_REG       =0x%08x\n",RTL_R32(DIAG1_REG));
	
	return len;
}
static int sw_cnt_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;

	printk("rx_hw_num	=0x%08x		rx_sw_num	=0x%08x\n", 
		re_private_data.cp_stats.rx_hw_num, re_private_data.cp_stats.rx_sw_num);
	printk("rer_runt	=0x%08x		rer_ovf		=0x%08x\n", 
		re_private_data.cp_stats.rer_runt, re_private_data.cp_stats.rer_ovf);
	printk("rdu		=0x%08x		frag		=0x%08x\n", 
		re_private_data.cp_stats.rdu, re_private_data.cp_stats.frag);
#ifdef CONFIG_RG_JUMBO_FRAME
	printk("toobig		=0x%08x\n", re_private_data.cp_stats.toobig);
#endif
	printk("crcerr		=0x%08x		rcdf		=0x%08x\n", 
		re_private_data.cp_stats.crcerr, re_private_data.cp_stats.rcdf);
	printk("rx_no_mem	=0x%08x		tx_sw_num	=0x%08x\n", 
		re_private_data.cp_stats.rx_no_mem, re_private_data.cp_stats.tx_sw_num);
	printk("tx_hw_num	=0x%08x		tx_no_desc	=0x%08x\n", 
		re_private_data.cp_stats.tx_hw_num, re_private_data.cp_stats.tx_no_desc);
	printk("rxskb_num       =0x%08x\n", re8670_rxskb_num.counter);

	return len;
}
static int rx_ring_write(struct file *file, const char *buff, unsigned long len, void *data)
{
	char 	tmpbuf[64];
	char	*strptr;
	char	*tokptr;
	
	if (buff && !copy_from_user(tmpbuf, buff, len)) {
		tmpbuf[len] = '\0';
		strptr=tmpbuf;
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}
		rx_ring_show_bitmap = simple_strtol(tokptr, NULL, 0);
		printk("rx_ring_show_bitmap 0x%08x \n", rx_ring_show_bitmap);		
	}
	else
	{
errout:
		printk("rx_ring bitmap: 0x%08x\n", rx_ring_show_bitmap);
	}

	return len;
}

static int rx_ring_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	int i = 0,j = 0;

	for(j=0;j<MAX_RXRING_NUM;j++)
	{
		if(rx_ring_show_bitmap&(1<<j))
		{
			printk("==================rx_ring [%d] info==================\n\n",j);
			if(re_private_data.rx_Mring[j] == 0 || re_private_data.rx_skb[j] == 0){
				printk("no rx_ring info!\n");
				continue;
			}

			for(i=0;i<RE8670_RX_MRING_SIZE[j];i++){
				printk("[idx%3d]:desc[0x%p]->skb[0x%p]->buf[0x%08x]:%s", 
					i, &re_private_data.rx_Mring[j][i], re_private_data.rx_skb[j][i].skb, 
					re_private_data.rx_Mring[j][i].addr, 
					(re_private_data.rx_Mring[j][i].opts1 & DescOwn)? "NIC" : "CPU");
				if(i == re_private_data.rx_Mtail[j]){
					printk("<=rx_tail");
				}
				printk("\n");
			}
		}
	}
	return len;
}
static int tx_ring_write(struct file *file, const char *buff, unsigned long len, void *data)
{
	char 	tmpbuf[64];
	char	*strptr;
	char	*tokptr;
	
	if (buff && !copy_from_user(tmpbuf, buff, len)) {
		tmpbuf[len] = '\0';
		strptr=tmpbuf;
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}
		tx_ring_show_bitmap = simple_strtol(tokptr, NULL, 0);
		printk("tx_ring_show_bitmap 0x%08x\n", tx_ring_show_bitmap);		
	}
	else
	{
errout:
		printk("tx_ring_show_bitmap: 0x%08x\n", tx_ring_show_bitmap);
	}

	return len;
}
static int tx_ring_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	int i = 0,j = 0;

	for(j=0;j<MAX_TXRING_NUM;j++)
	{
		if(tx_ring_show_bitmap&(1<<j))
		{
			printk("==================tx_ring [%d] info==================\n\n",j);
			if(re_private_data.tx_Mhqring[j] == 0 || re_private_data.tx_skb[j] == 0){
				printk("no tx_ring info\n");
				continue;
			}

			for(i=0;i<RE8670_TX_MRING_SIZE[j];i++){
				printk("[idx%3d]:desc[0x%p]->skb[0x%p]->buf[0x%08x]:%s", 
					i, &re_private_data.tx_Mhqring[j][i], re_private_data.tx_skb[j][i].skb, 
					re_private_data.tx_Mhqring[j][i].addr, 
					(re_private_data.tx_Mhqring[j][i].opts1 & DescOwn)? "NIC" : "CPU");
				if(i == re_private_data.tx_Mhqtail[j]){
					printk("<=tx_hqtail");
				}
				if(i == re_private_data.tx_Mhqhead[j]){
					printk("<=tx_hqhead");
				}
				printk("\n");
			}
		}
	}

	return len;
}


void nic_tx_ring_dump(void)
{
	struct re_private *cp;
	DMA_TX_DESC *txd;
	int i;
	int ring_index=1;
	
	cp=&re_private_data;	
	for(i=0;i<RE8670_TX_MRING_SIZE[ring_index];i++)
	{
		txd = (DMA_TX_DESC *)((u32)&cp->tx_Mhqring[ring_index][i]|0xa0000000);
		
		printk("%08x[%03d] %08x %08x %08x OWN=%d E=%d F=%d L=%d LEN=%05d LSO=%d MTU=%d\n",(u32)&txd->opts1,i,txd->opts1,txd->addr,txd->opts2
			,(txd->opts1&0x80000000)?1:0
			,(txd->opts1&0x40000000)?1:0
			,(txd->opts1&0x20000000)?1:0
			,(txd->opts1&0x10000000)?1:0
			,(txd->opts1&0x1ffff)			
			,(txd->opts4&0x80000000)?1:0
			,(txd->opts4&0x7ff00000)>>20
		);
	}
	printk("\n");
	
}

static int nic_tx_ring_dump_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	nic_tx_ring_dump();
	return 0;
}

static int nic_tx_ring_dump_write(struct file *file, const char *buff, unsigned long len, void *data)
{
	nic_tx_ring_dump();
	return 0;
}


static void nic_rx_ring_dump(void)
{
	struct re_private *cp;
	DMA_RX_DESC *rxd;
	int i;
	int ring_index=0;

	cp=&re_private_data;
	for(i=0;i<RE8670_RX_MRING_SIZE[ring_index];i++)
	{
		rxd = (DMA_RX_DESC *)((u32)&cp->rx_Mring[ring_index][i]|0xa0000000);
		printk("%08x[%03d] %08x %08x %08x %08x OWN=%d E=%d LEN=%05d\n",(u32)&rxd->opts1,i,rxd->opts1,rxd->addr,rxd->opts2,rxd->opts3
			,(rxd->opts1&0x80000000)?1:0
			,(rxd->opts1&0x40000000)?1:0
			,(rxd->opts1&0xfff)
		);
	}
	printk("\n");
}

static int nic_rx_ring_dump_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	nic_rx_ring_dump();
	return 0;
}

static int nic_rx_ring_dump_write(struct file *file, const char *buff, unsigned long len, void *data)
{
	nic_rx_ring_dump();
	return 0;
}

#define RTL8686_PROC_DIR_NAME "rtl8686gmac"
struct proc_dir_entry *rtl8686_proc_dir=NULL;

struct gmac_proc_entry{
	struct proc_dir_entry* p_dir_entry;
	char* entry_name;
	read_proc_t* read_func;
	write_proc_t* write_func;
};

static struct gmac_proc_entry gmac_proc_tbl[] = {
	{NULL, "dbg_level", dbg_level_read, dbg_level_write},
	{NULL, "hw_reg", hw_reg_read, NULL},
	{NULL, "sw_cnt", sw_cnt_read, NULL},
	{NULL, "rx_ring", rx_ring_read, rx_ring_write},
	{NULL, "tx_ring", tx_ring_read, tx_ring_write},
	{NULL, "mem", memrw_read, memrw_write},
	{NULL, "dev_port_mapping", dev_port_mapping_read, dev_port_mapping_write},
	{NULL, "misc", misc_read, misc_write},
	{NULL, "port2rxfunc", port_to_rxfunc_read, NULL},
#ifdef CONFIG_RTL8686_SWITCH
	{NULL, "switch_mode", switch_mode_read, switch_mode_write},
	{NULL, "port_status", port_status_read, NULL},
	{NULL, "l2", l2_entry_read, NULL},
	{NULL, "multicast", IPMulticast_entry_read, IPMulticast_entry_write},
	{NULL, "acl_raw_rule", ACL_RAW_RULE_entry_read, NULL},
	{NULL, "acl_raw_template", ACL_RAW_TEMPLATE_entry_read, NULL},
	{NULL, "acl_raw_field", ACL_RAW_FLDSLCT_entry_read, NULL},
#endif
#ifdef RE8686_VERIFY
	{NULL, "verify", verify_read, verify_write},
#endif
	{NULL, "rx_ring_detail", nic_rx_ring_dump_read, nic_rx_ring_dump_write},
	{NULL, "tx_ring_detail", nic_tx_ring_dump_read, nic_tx_ring_dump_write},

	{NULL, NULL, NULL, NULL},
};

static void rtl8686_proc_debug_init(void){
	if(rtl8686_proc_dir==NULL)
		rtl8686_proc_dir = proc_mkdir(RTL8686_PROC_DIR_NAME,NULL);
	if(rtl8686_proc_dir)
	{
		int i;
		for(i=0;gmac_proc_tbl[i].entry_name;i++){
			if(gmac_proc_tbl[i].write_func){
				gmac_proc_tbl[i].p_dir_entry = create_proc_entry(gmac_proc_tbl[i].entry_name, 0, rtl8686_proc_dir);
				if(gmac_proc_tbl[i].p_dir_entry){
					(gmac_proc_tbl[i].p_dir_entry)->read_proc = gmac_proc_tbl[i].read_func;
					(gmac_proc_tbl[i].p_dir_entry)->write_proc = gmac_proc_tbl[i].write_func;
				}
			}
			else{
				gmac_proc_tbl[i].p_dir_entry = create_proc_read_entry(gmac_proc_tbl[i].entry_name
						, 0444, rtl8686_proc_dir, gmac_proc_tbl[i].read_func, NULL);
			}
			if(gmac_proc_tbl[i].p_dir_entry == NULL){
				printk("can't create proc entry for %s\n", gmac_proc_tbl[i].entry_name);
			}
		}
	}
}

void port_relate_setting(void){
	int i;
	unsigned int totalDev = ((sizeof(rtl8686_dev_table))/(sizeof(struct rtl8686_dev_table_entry)));
	unsigned int old_phy_port_mapping[7] = {CPU_PORT, 0, 1, 4, 5, PON_PORT, PON_PORT};

	//rx function
	re8686_reset_rxfunc_to_default();

	//port2dev: default set to eth0
	for(i=0;i<SW_PORT_NUM;i++){
		re_private_data.port2dev[i] = eth_net_dev;
	}
	//phy port re-map
	if(SOC_ID == 0x0371){
		PON_PORT = 3;
		for(i=0;i<totalDev;i++){
			rtl8686_dev_table[i].phyPort = old_phy_port_mapping[i];
		}
	}
	
	for(i=0;i<totalDev;i++){
		DEVPRIV(rtl8686_dev_table[i].dev_instant)->txPortMask = 
			(rtl8686_dev_table[i].phyPort == CPU_PORT) ? 0 : (1<<rtl8686_dev_table[i].phyPort);
	}
}

int __init re8670_probe (void)
{
	struct net_device *dev;
	int rc;
	void *regs=(void*)ETHBASE;	
	unsigned i, j;
	unsigned int totalDev = ((sizeof(rtl8686_dev_table))/(sizeof(struct rtl8686_dev_table_entry)));
    extern int drv_nic_rxhook_init(void);
#ifdef MODULE
	printk("%s", version);
#endif
#ifndef MODULE
	static int version_printed;
	if (version_printed++ == 0)
		printk("%s", version);
#endif

	printk("TX[%d,%d,%d,%d,%d] , RX[%d,%d,%d,%d,%d,%d]\n" ,
               	RE8670_TX_RING1_SIZE, RE8670_TX_RING2_SIZE, RE8670_TX_RING3_SIZE, RE8670_TX_RING4_SIZE,
               	RE8670_TX_RING5_SIZE, RE8670_RX_RING1_SIZE, RE8670_RX_RING2_SIZE, RE8670_RX_RING3_SIZE,
		RE8670_RX_RING4_SIZE, RE8670_RX_RING5_SIZE, RE8670_RX_RING6_SIZE);

	//allocate and register root dev first
	dev = alloc_etherdev(sizeof(struct re_dev_private));
	if (!dev){
		printk("%s %d\n", __func__, __LINE__);
		goto err_out_iomap;
	}

	//reset re_private_data
	memset(&re_private_data, 0, sizeof(struct re_private));
	re_private_data.dev = dev;
	spin_lock_init (&(re_private_data.tx_lock));
	spin_lock_init (&(re_private_data.rx_lock));
	re_private_data.regs = regs;
	memset(&re_private_data.rx_tasklets, 0, sizeof(struct tasklet_struct));
	re_private_data.rx_tasklets.func=(void (*)(unsigned long))re8670_rx;
	re_private_data.rx_tasklets.data=(unsigned long)&re_private_data;
	eth_rx_tasklets = &(re_private_data.rx_tasklets);
	eth_net_dev = dev;
	
	dev->base_addr = (unsigned long) regs;
	/* read MAC address from EEPROM */
	for (i = 0; i < 3; i++)
		((u16 *) (dev->dev_addr))[i] = i;

	dev->open = re8670_open;
	dev->stop = re8670_close;
	dev->set_multicast_list = re8670_set_rx_mode;
	dev->hard_start_xmit = re8670_start_xmit;
	dev->get_stats = re8670_get_stats;
	dev->do_ioctl = re8670_ioctl;
	dev->tx_timeout = re8670_tx_timeout;
	dev->watchdog_timeo = TX_TIMEOUT;
	//dev->irq = BSP_GMAC1_IRQ;	// internal phy
	dev->irq = 27; 
	//priv data setting
	dev->priv_flags = IFF_DOMAIN_ELAN;
	DEV2CP(dev) = &re_private_data;
	rtl8686_dev_table[0].dev_instant = dev;

	memcpy(dev->name, rtl8686_dev_table[0].ifname, strlen(rtl8686_dev_table[0].ifname));
	rtl_set_ethtool_ops(dev);
	rc = register_netdev(dev);
	if (rc){
		printk("%s %d rc = %d\n", __func__, __LINE__, rc);
		goto err_out_iomap;
	}

	printk (KERN_INFO "%s: %s at 0x%lx, "
			"%02x:%02x:%02x:%02x:%02x:%02x, "
			"IRQ %d\n",
			dev->name,
			"RTL-8686",
			dev->base_addr,
			dev->dev_addr[0], dev->dev_addr[1],
			dev->dev_addr[2], dev->dev_addr[3],
			dev->dev_addr[4], dev->dev_addr[5],
			dev->irq);
	
	//allocate and register other device
	for(j=1; j < totalDev; j++){
		dev = alloc_etherdev(sizeof(struct re_dev_private));
		if (!dev){
			printk("%s %d\n", __func__, __LINE__);
			goto err_out_iomap;
		}
		dev->base_addr = (unsigned long) regs;
		/* read MAC address from EEPROM */
		for (i = 0; i < 3; i++)
			((u16 *) (dev->dev_addr))[i] = i;

		dev->open = re8670_open;
		dev->stop = re8670_close;
		dev->set_multicast_list = re8670_set_rx_mode;
		dev->hard_start_xmit = re8670_start_xmit;
		dev->get_stats = re8670_get_stats;
		dev->do_ioctl = re8670_ioctl;
		dev->tx_timeout = re8670_tx_timeout;
		dev->watchdog_timeo = TX_TIMEOUT;
		dev->irq = 27;	// internal phy
		// priv data setting
		dev->priv_flags = IFF_DOMAIN_ELAN;
		DEV2CP(dev) = &re_private_data;
		rtl8686_dev_table[j].dev_instant = dev;

		memcpy(dev->name, rtl8686_dev_table[j].ifname, strlen(rtl8686_dev_table[j].ifname));
		rtl_set_ethtool_ops(dev);
		rc = register_netdev(dev);
		if (rc){
			printk("%s %d rc = %d\n", __func__, __LINE__, rc);
			goto err_out_iomap;
		}

		printk (KERN_INFO "%s: %s at 0x%lx, "
				"%02x:%02x:%02x:%02x:%02x:%02x, "
				"IRQ %d\n",
				dev->name,
				"RTL-8686",
				dev->base_addr,
				dev->dev_addr[0], dev->dev_addr[1],
				dev->dev_addr[2], dev->dev_addr[3],
				dev->dev_addr[4], dev->dev_addr[5],
				dev->irq);
	}

	printk("RTL8686 GMAC Probing..\n");	//shlee 2.6

#ifdef CONFIG_DUALBAND_CONCURRENT
	str2mac(CONFIG_DEFAULT_MASTER_IPC_MAC_ADDRESS, src_mac);
#endif

	//sw stuff
#ifdef CONFIG_RTL865X_ETH_PRIV_SKB
	init_priv_eth_skb_buf();
#endif
	rtl8686_proc_debug_init();
	port_relate_setting();
#ifdef CONFIG_RTL8686_SWITCH
	//init this for gpon driver
    drv_nic_rxhook_init();
#endif
	rc = re8670_alloc_rings(&re_private_data);
	if (rc)
		return rc;
	re8670_save_trx_cdo(&re_private_data);

	return 0;

err_out_iomap:
	printk("%s %d: here is a error when probe!!!!!!!!!!0rzzzzzzzzzzzzzzzz\n", __func__, __LINE__);
	return -1 ;
}

void __exit re8670_exit (void)
{
	unsigned i;
	unsigned int totalDev = ((sizeof(rtl8686_dev_table))/(sizeof(struct rtl8686_dev_table_entry)));
    extern int drv_nic_rxhook_exit(void);
	//desc and hw setting
	if(dev_num){
		re8670_close(eth_net_dev);
		re8670_free_rings(&re_private_data);
	}
	//proc
	for(i=0;gmac_proc_tbl[i].entry_name;i++){
		if(gmac_proc_tbl[i].p_dir_entry){
			remove_proc_entry(gmac_proc_tbl[i].entry_name, rtl8686_proc_dir);
		}
	}
	remove_proc_entry(RTL8686_PROC_DIR_NAME, NULL);
	//dev
	for(i=0; i < totalDev; i++){
		if(rtl8686_dev_table[i].dev_instant){
			unregister_netdev(rtl8686_dev_table[i].dev_instant);
			free_netdev(rtl8686_dev_table[i].dev_instant);
		}
	}
#ifdef CONFIG_RTL8686_SWITCH
	//for gpon driver
    drv_nic_rxhook_exit();
#endif
}
module_init(re8670_probe);
module_exit(re8670_exit);

//need this?
int bitmap_virt2phy(int mbr)
{
#ifdef CONFIG_EXT_SWITCH
	int k, mask, phymap, phyid;
	
	phymap = mbr&0xffffffc0;
	
	for (k=0; k<=SW_PORT_NUM; k++) {
		mask = mbr & (1<<k);
		if (mask) {
			phyid = virt2phy[k];
			if (phyid >= 6)
				phyid = 5;
			phymap |= (1 << phyid);
		}
	}
	return phymap;
// Mason Yu. combine_1p_4p_PortMapping
#else
	return mbr;
#endif
}
EXPORT_SYMBOL(bitmap_virt2phy);

#ifdef CONFIG_WIRELESS_LAN_MODULE
int (*wirelessnet_hook)(void) = NULL;
EXPORT_SYMBOL(wirelessnet_hook);
#endif

EXPORT_SYMBOL(re8686_send_with_txInfo_and_mask);

