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

#define DRV_NAME		"8139cp"
#define DRV_VERSION		"0.0.7"
#define DRV_RELDATE		"Feb 27, 2002"


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
#include <net/xfrm.h>

#ifdef CONFIG_PPPOE_PROXY
#include <linux/ppp_defs.h>
#include <linux/if_ppp.h>
#endif

//GMAC uses RGMII interface to connect external CPU's MAC
//#define NIC_USE_RGMII_TO_EXTCPU_MAC

#ifdef CONFIG_ETHWAN
#include "brg_shortcut.h"
#include <net/dst.h>
#include "../../net/bridge/br_private.h"    //for skb->dev = eth_net_dev->br_port->br->dev
#endif //CONFIG_ETHWAN


#ifdef CONFIG_PPPOE_PROXY
short pppoeproxy_portmap;
#endif

#if defined(CONFIG_SKB_POOL_PREALLOC) || defined(CONFIG_RTL8192CD)
//#define WLANBR_SHORTCUT //commented by Kevin, merge bridge fastpath into netif_rx()
#endif

#ifdef WLANBR_SHORTCUT
#define MAX_CACHED_ETH_ADDR 4
unsigned char cached_eth_addr[MAX_CACHED_ETH_ADDR][6];
struct net_device *cached_dev[MAX_CACHED_ETH_ADDR];
int cache_index=0;
#endif

#ifdef CONFIG_EXT_SWITCH
#include "./rtl8306/Rtl8306_types.h"
#ifdef CONFIG_RE8306_API

#ifdef CONFIG_ETHWAN
#include "./rtl8306/rtl8306e_asicdrv.h"
#endif //CONFIG_ETHWAN
#include "./rtl8306/Rtl8306_AsicDrv.h"
#include "./rtl8306/Rtl8306_Driver_s.h"
#include "./rtl8306/Rtl8306_Driver_sd.h"
#endif // of CONFIG_RE8306_API
#endif // of CONFIG_EXT_SWITCH
#ifdef CONFIG_RTL867X_PACKET_PROCESSOR
#include "packet_processor/rtl8672PacketProcessor.h"
#endif
#ifdef WLANBR_SHORTCUT
#include "../../net/bridge/br_private.h"
#endif
#include "re867x.h"
#include "re_smi.h"
#include "re830x.h"
#include "re_igmp.h"

#ifdef CONFIG_RTL865X_ETH_PRIV_SKB
#include "re_privskb.h"
#endif

#ifdef CONFIG_RTL8671
  #ifdef CONFIG_UCLINUX
    #include "../../arch/mipsnommu/rtl8670/lx5280.h"
  #else
    #include "../../arch/mips/realtek/rtl8670/lx5280.h"
    #include "../../arch/mips/realtek/rtl8670/gpio.h"
  #endif
#endif

#ifdef CONFIG_RTL8672	//shlee 2.6
//#include <platform.h>
/*linux-2.6.19*/
#include <bspchip.h>
#include <linux/version.h>
//#include "../../arch/mips/realtek/rtl8672/gpio.h"
/*linux-2.6.19*/
#include "../../arch/rlx/bsp/gpio.h"
#endif

#include "ethctl_implement.h"


#ifdef CONFIG_GPIO_LED_CHT_E8B
int g_ppp_up = 0;
int g_internet_up = 0;
#endif

#ifdef CONFIG_EXT_SWITCH
extern void vc_ch_remapping(unsigned *member);
#else
#define PHY_MDI_MDIX_AUTO
#endif
// kaohj -- bitmap of virtual ports vs bitmap of phy id
// Input:  bitmap of virtual ports
// Output: bitmap of phy id
int bitmap_virt2phy(int mbr);
/*static*/ int re8670_start_xmit (struct sk_buff *skb, struct net_device *dev);

/* Jonah + for FASTROUTE */
struct net_device *eth_net_dev;
struct tasklet_struct *eth_rx_tasklets=NULL;

#if defined (CONFIG_RTL8192CD) || defined (CONFIG_RTL8192CD_MODULE) || defined(CONFIG_RTL8681_PTM)
struct tasklet_struct eth_checktxdesc_tasklets;
#endif

//extern struct net_device *sar_dev;
//#undef 	FAST_ROUTE_Test
//#define FAST_ROUTE_Test //commented by Kevin, merge bridge fastpath into netif_rx()

#ifdef FAST_ROUTE_Test
int fast_route_mode=1;
#endif

//extern  int sar_txx (struct sk_buff *skb);



#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
#define CP_VLAN_TAG_USED 1
#define CP_VLAN_TX_TAG(tx_desc,vlan_tag_value) \
	do { (tx_desc)->opts2 = (vlan_tag_value); } while (0)
#else
#ifdef CONFIG_ETHWAN
#define CP_VLAN_TAG_USED 0
#define CP_VLAN_TX_TAG(tx_desc,vlan_tag_value) \
	do { (tx_desc)->opts2 = (vlan_tag_value); } while (0)
#else
#define CP_VLAN_TAG_USED 0
#define CP_VLAN_TX_TAG(tx_desc,vlan_tag_value) \
	do { (tx_desc)->opts2 = 0; } while (0)
#endif //CONFIG_ETHWAN		
#endif
// Kaohj
extern int enable_ipqos;
__DRAM int enable_port_mapping=0;
// Kaohj -- vlan-grouping
__DRAM int enable_vlan_grouping=0;
// Mason Yu. combine_1p_4p_PortMapping
#ifndef CONFIG_EXT_SWITCH
// Mason Yu. 1P_portMapping
short eth_dev_member=0;
#endif
struct r8305_struc	rtl8305_info;
#ifdef CONFIG_EXT_SWITCH
int enable_virtual_port=0;


// Kaohj
// virtual LAN port 0, 1, 2, 3 and 867x-nic to 8305(6) phy id
// ex, virt2phy[2]=3 ==> virtual port 2 is mapped to phy id 3
//     virt2phy[4]=0 ==> virtual port 867x-nic is mappped to phy id 0
//#define AUTO_SEARCH_CPUPORT
#ifndef AUTO_SEARCH_CPUPORT
#ifdef CONFIG_RE8305
int virt2phy[SW_PORT_NUM+1]= {0, 1, 2, 3, 4};
#endif
#ifdef CONFIG_RE8306
#if CONFIG_CPU_PORT>=4
int virt2phy[SW_PORT_NUM+1]= {0, 1, 2, 3, CONFIG_CPU_PORT<=SWITCH_PORT4?CONFIG_CPU_PORT:6};
#else
#ifdef CONFIG_ETHWAN
__DRAM
#endif //CONFIG_ETHWAN
int virt2phy[SW_PORT_NUM+1]= {1, 2, 3, 4, CONFIG_CPU_PORT<=SWITCH_PORT4?CONFIG_CPU_PORT:6};
#endif
#endif
#else
int virt2phy[SW_PORT_NUM+1]= {0, 1, 2, 3, 4};
#endif
#endif // of CONFIG_EXT_SWITCH

#ifdef CONFIG_RTL8672_NAPI
#define WITH_NAPI		" with NAPI" 
#define napi_printk(...)	{while(0);}
//#define napi_printk		printk
#else
#define WITH_NAPI		""
#endif

/* These identify the driver base version and may not be removed. */
static char version[] __devinitdata =
KERN_INFO DRV_NAME " Ethernet driver v" DRV_VERSION " (" DRV_RELDATE ")" WITH_NAPI "\n";

MODULE_AUTHOR("Rupert Chang <rupert@mail.realtek.com.tw>");
MODULE_DESCRIPTION("RealTek RTL-8670 series 10/100 Ethernet driver");
MODULE_LICENSE("GPL");

/* Maximum number of multicast addresses to filter (vs. Rx-all-multicast).
   The RTL chips use a 64 element hash table based on the Ethernet CRC.  */
static int multicast_filter_limit = 32;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 9)	//shlee 2.6
module_param(multicast_filter_limit, int, S_IRUGO|S_IWUSR)
#else
MODULE_PARM (multicast_filter_limit, "i");
#endif
MODULE_PARM_DESC (multicast_filter_limit, "8139cp maximum number of filtered multicast addresses");

#define PFX			DRV_NAME ": "
#define CP_DEF_MSG_ENABLE	(NETIF_MSG_DRV		| \
				 NETIF_MSG_PROBE 	| \
				 NETIF_MSG_LINK)
#define CP_REGS_SIZE		(0xff + 1)





#define DESC_ALIGN		0x100
#define UNCACHE_MASK		0xa0000000

#define RE8670_RXRING_BYTES	( (sizeof(struct dma_desc) * (RE8670_RX_RING_SIZE+1)) + DESC_ALIGN)

#define RE8670_TXRING_BYTES	( (sizeof(struct dma_desc) * (RE8670_TX_RING_SIZE+1)) + DESC_ALIGN)
		


#define NEXT_TX(N)		(((N) + 1) & (RE8670_TX_RING_SIZE - 1))
#define NEXT_RX(N)		(((N) + 1) & (RE8670_RX_RING_SIZE - 1))
#ifdef CONFIG_ETHWAN
#define TX_HQBUFFS_AVAIL(CP)					\
	(((CP)->tx_hqtail - (CP)->tx_hqhead + RE8670_TX_RING_SIZE - 1)&(RE8670_TX_RING_SIZE - 1))
#else
#define TX_HQBUFFS_AVAIL(CP)					\
	(((CP)->tx_hqtail <= (CP)->tx_hqhead) ?			\
	  (CP)->tx_hqtail + (RE8670_TX_RING_SIZE - 1) - (CP)->tx_hqhead :	\
	  (CP)->tx_hqtail - (CP)->tx_hqhead - 1)
#endif //CONFIG_ETHWAN

#if defined(CONFIG_SKB_POOL_PREALLOC)&& defined(CONFIG_RTL867X_PACKET_PROCESSOR)
#else
#define PKT_BUF_SZ		1536	/* Size of each temporary Rx buffer.*/
#endif
#define RX_OFFSET		2


/* The following settings are log_2(bytes)-4:  0 == 16 bytes .. 6==1024, 7==end of packet. */

/* Time in jiffies before concluding the transmitter is hung. */
#define TX_TIMEOUT		(3*HZ)

/* hardware minimum and maximum for a single frame's data payload */
#define CP_MIN_MTU		60	/* TODO: allow lower, but pad */
#define CP_MAX_MTU		4096

enum PHY_REGS{
#ifdef NIC_USE_RGMII_TO_EXTCPU_MAC
	ForceDuplex =    1 << 19,
	ForceLink =      1 << 18,
	ForceSpeedMask = 3 << 16,
	ForceSpeedMode = 1 << 10,
#else /*NIC_USE_RGMII_TO_EXTCPU_MAC*/
    FORCE_TX = 1<<7,
    RXFCE= 1<<6,
    TXFCE= 1<<5,
    SP1000= 1<<4,
    SP10= 1<<3,
    LINK=1<<2,
	TXPF=1<<1,
	RXPF=1<<0,
#endif /*NIC_USE_RGMII_TO_EXTCPU_MAC*/
};
int aaa;
//tylo, test for fpga
#if 0
int gimr_tmp=0;
void clitest(void){
	//cli();
	gimr_tmp=REG32(GIMR);
	REG32(GIMR)=0;
}
void stitest(void){
	//sti();
	if(aaa==1)
		REG32(GIMR)=(gimr_tmp|0x04000000);
	else
		REG32(GIMR)=gimr_tmp;
	
}
#endif
enum RE8670_STATUS_REGS
{
    /* Rx status descriptors */
    DescOwn		= (1 << 31), /* Descriptor is owned by NIC */
	RingEnd		= (1 << 30), /* End of descriptor ring */
	FirstFrag	= (1 << 29), /* First segment of a packet */
	LastFrag	= (1 << 28), /* Final segment of a packet */
	RxErrFrame	= (1 << 27), /* Rx frame alignment error */
	RxMcast		= (1 << 26), /* Rx multicast packet rcv'd */
	RxPAM       = (1 << 25), /* Rx physical address matched rcv'd*/
	RxBcast     = (1 << 24), /* Rx broadcast packet rcv'd */
	E8023		= (1 << 22), /* Receive Ethernet 802.3 packet */
	RWT		    = (1 << 21), /* Receive watchdog timer expired */
	RCDF        = (1 << 20),
	RxErrRunt	= (1 << 19), /* Rx error, packet < 64 bytes */
	RxErrCRC	= (1 << 18), /* Rx CRC error */
	PID1		= (1 << 17), /* 2 protocol id bits:  0==non-IP, */
	PID0		= (1 << 16), /* 1==UDP/IP, 2==TCP/IP, 3==IP */
	RxProtoTCP	= 1,
	RxProtoUDP	= 2,
	RxProtoIP	= 3,
	IPFail		= (1 << 15), /* IP checksum failed */
	UDPFail 	= (1 << 14), /* UDP/IP checksum failed */
	TCPFail 	= (1 << 13), /* TCP/IP checksum failed */
	IPSEG       = (1 << 12),
	IPv6        = (1 << 11),
	RxVlanTagged	= (1 << 16), /* Rx VLAN tag available */

    /* Tx command descriptors */
	//IPCS		= (1 << 27), /* Calculate IP checksum */
	//UDPCS		= (1 << 26), /* Calculate UDP/IP checksum */
	//TCPCS		= (1 << 25), /* Calculate TCP/IP checksum */
	TxVlanTag	= (1 << 16), /* Add VLAN tag */
	TxCRC		= (1 << 23),
	
	RxVlanOn	= (1 << 2),  /* Rx VLAN de-tagging enable */
	RxChkSum	= (1 << 1), 

};

enum RE8670_THRESHOLD_REGS{
//shlee	THVAL		= 2,
	TH_ON_VAL = 0x06,	//shlee flow control assert threshold: available desc <= 6
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
	TX_EMPTY	= (1 << 9),
	LINK_CHG	= (1 <<	8),
	TX_ERR		= (1 << 7),
	TX_OK		= (1 << 6),
	RX_EMPTY	= (1 << 5),
	RX_FIFOOVR	=(1 << 4),
	RX_ERR		=(1 << 2),
	RUNT_ERR 	=(1<<19),
	RX_OK		= (1 << 0),


};



#if 0
enum RTL8672GMAC_CPUtag_Control
{
   CTEN_RX     = (1<<31),
   CT_8370S    = (1<<20),
   CT_8306S    = (1<<19),
   CT_8368S    = (1<<18),
   CTSIZE4     = (1<<16),
   CTSIZE8     = (1<<17), 
   CTPM_8306   = 0xf000,
   CTPM_8368   = 0xe000,
   CTPM_8370   = 0xff00,
   CTPV_8306   = 0x90,
   CTPV_8368   = 0xa0,
   CTPV_8370   = 0x04,
};

enum RTL8672GMAC_PG_REG
{
   EN_PGLBK     = (1<<15),
   DATA_SEL     = (1<<14),
   LEN_SEL      = (1<<11),
   NUM_SEL      = (1<<10),
};
#endif

typedef enum
{
	FLAG_WRITE		= (1<<31),
	FLAG_READ		= (0<<31),
	
	MII_PHY_ADDR_SHIFT	= 26, 
	MII_REG_ADDR_SHIFT	= 16,
	MII_DATA_SHIFT		= 0,
}MIIAR_MASK;

static const u32 re8670_intr_mask =
	RX_OK | RX_ERR | RX_EMPTY | RX_FIFOOVR
#ifdef PHY_MDI_MDIX_AUTO
	|LINK_CHG;
#else
	;
#endif
	//TX_EMPTY;
	//TX_OK | TX_ERR | TX_EMPTY;


typedef struct dma_desc {
	u32		opts1;
	u32		addr;
	u32		opts2;
	u32		opts3;

}DMA_DESC;


struct ring_info {
	struct sk_buff		*skb;
	dma_addr_t		mapping;
	unsigned		frag;
};

struct cp_extra_stats {
	unsigned long		rx_frags;
	unsigned long tx_timeouts;
	// JONAH
	unsigned long tx_cnt;
};

struct re_private {
	unsigned		tx_hqhead;
	unsigned		tx_hqtail;
	unsigned		tx_lqhead;
	unsigned		tx_lqtail;
	unsigned		rx_tail;

	void			*regs;
	struct net_device	*dev;
	spinlock_t		lock;

	DMA_DESC		*rx_ring;
	
	DMA_DESC		*tx_hqring;
	DMA_DESC		*tx_lqring;
	struct ring_info	tx_skb[RE8670_TX_RING_SIZE];
	struct ring_info	rx_skb[RE8670_RX_RING_SIZE];
	unsigned		rx_buf_sz;
	dma_addr_t		ring_dma;

#if CP_VLAN_TAG_USED
	struct vlan_group	*vlgrp;
#endif

	u32			msg_enable;

#ifdef CONFIG_ETHWAN
	struct net_device_stats net_stats[SW_PORT_NUM+1]; //eth0.2 eth0.3 eth0.4 nas0 eth0
#else
	struct net_device_stats net_stats;
#endif //CONFIG_ETHWAN
	
	struct cp_extra_stats	cp_stats;

	struct pci_dev		*pdev;
	u32			rx_config;

	struct sk_buff		*frag_skb;
	unsigned		dropping_frag : 1;
	char*			rxdesc_buf;
	char*			txdesc_buf;
	struct mii_if_info	mii_if;
	//struct tq_struct	rx_task;
	//struct tq_struct	tx_task;
	struct tasklet_struct rx_tasklets;
	//struct tasklet_struct tx_tasklets;
};
//struct re_private *reDev;

// Kaohj
#ifdef CONFIG_EXT_SWITCH
struct net_device	*dev_sw[SW_PORT_NUM];
#endif

#ifdef PHY_MDI_MDIX_AUTO
struct timer_list eth_phy_check_timer;
int eth_link_stable_times=0;
#define INTERNAL_PHY_ID	1
#define ETH_LINK_STABLE		5
#endif
#ifdef CONFIG_EXT_SWITCH
enum SW_LINK_STS
{
	CHECK_FAIL=0,
	CHECK_OK
};
struct timer_list sw_check_timer;
int sw_link_stable_times[SW_PORT_NUM]={0};
int sw_link_checked[SW_PORT_NUM]={CHECK_FAIL};
unsigned int swtype;
unsigned int SWChipversion;

#define ETH_LINK_STABLE		5

// andrew
#ifdef CONFIG_RTL8672_PWR_SAVE

struct mgt_struct {
	struct timer_list mgt_timer;	
};

static struct mgt_struct mgt;
#define MGT_TICK 25
#endif // PWR_SAVE


#endif

#define VTAG2DESC(d) ( (((d) & 0x00ff)<<8) | (((d) & 0x0f00)>>8) )
//tylo, struct for IGMP snooping
#ifdef CONFIG_EXT_SWITCH
#include <net/checksum.h>
#endif

static void __re8670_set_rx_mode (struct net_device *dev);
static void re8670_tx (struct re_private *cp);
static void re8670_clean_rings (struct re_private *cp);
#ifndef CONFIG_RTL867X_PACKET_PROCESSOR
static void re8670_tx_timeout (struct net_device *dev);
#endif

extern void enable_lx4180_irq(int irq);
extern void disable_lx4180_irq(int irq);

extern atomic_t re8670_rxskb_num;
#if (defined(CONFIG_USB_RTL8192SU_SOFTAP) || defined(CONFIG_USB_RTL8187SU_SOFTAP)) && !defined(CONFIG_SKB_POOL_PREALLOC) && !defined(CONFIG_RTL867X_PACKET_PROCESSOR)
atomic_t txskb_from_wifi = ATOMIC_INIT(0);
#endif
//int re8670_rxskb_num=0;

//#define RE8670_MAX_ALLOC_RXSKB_NUM 150
// Kaohj -- 150 is not enough to make IPQoS work
#ifdef CONFIG_ETHWAN
	#if defined (CONFIG_RTL8192CD) || defined (CONFIG_RTL8192CD_MODULE)
	#define RE8670_MAX_ALLOC_RXSKB_NUM 6000
	#else
	#define RE8670_MAX_ALLOC_RXSKB_NUM 1500
	#endif //CONFIG_RTL8192CD
#else
	#ifdef CONFIG_MEM_LIMITATION
	#define RE8670_MAX_ALLOC_RXSKB_NUM 300
	#else
	#define RE8670_MAX_ALLOC_RXSKB_NUM 1000
	#endif //CONFIG_MEM_LIMITATION
#endif //CONFIG_ETHWAN

#if defined(CONFIG_SKB_POOL_PREALLOC) && defined(CONFIG_RTL867X_PACKET_PROCESSOR)
#define SKB_BUF_SIZE  CONFIG_RTL867X_PREALLOCATE_SKB_SIZE
#else
#define SKB_BUF_SIZE  1600
#endif

//for wifi-test
unsigned int iocmd_reg=CMD_CONFIG;//=0x4009113d;	//shlee 8672

__DRAM unsigned int debug_enable=0;
#ifdef CONFIG_ETHWAN
#define ETH_WAN_PORT 3//in virtual view
__DRAM struct net_device *vnet_dev[SW_PORT_NUM];
__DRAM unsigned int vnet_dev_status[SW_PORT_NUM]={0};

#undef ETH_DBG
#define ETH_DBG
#ifdef ETH_DBG
#define ETHDBG_PRINT(fmt, args...)  if(debug_enable) printk(fmt, ##args)
#define SKB_DBG(args...) skb_debug(args)
#else
#define ETHDBG_PRINT(fmt, args...)
#define SKB_DBG(args...) 
#endif
#define WAN_ETH_PORT  4

#ifdef CONFIG_RTL8672_ETHSKB_CONTROL_POOL
unsigned int upstream_skb_pool_free_num = 0;
#define MAX_UPSTREAM_CONTROL_POOL 450
#define UPSTREAM_CONTROL_POOL_FREE_SKB_NUM 1//100
#define NIC_RX_BUFF_SIZE  1760
//#define MAX_PRESKB_POOL_SKB_NUM 200
#define MAX_PRESKB_POOL_SKB_NUM 256
unsigned int skb_pre_pool_num=MAX_PRESKB_POOL_SKB_NUM;
struct sk_buff *skb_rx_pre_pool[MAX_PRESKB_POOL_SKB_NUM];

__DRAM
int debug_upstream_trace;
int init_rx_pool_complete =0;
__DRAM
struct sk_buff *skb_rx_free_pool[MAX_UPSTREAM_CONTROL_POOL];
__DRAM
unsigned int  NicRxPollSmallPktCount=0;
__DRAM
unsigned long  NicRxPollStartStamp=0;
int debug_rx_pool;
__DRAM
int net_smallpkt_heavytraffic;



__IRAM_NIC
static void init_skb_rx_pool(struct sk_buff *skb)
{
	struct skb_shared_info *shinfo;

	skb->data=skb->tail=skb->head;
	skb_reserve(skb,NET_SKB_PAD);
	atomic_set(&(skb_shinfo(skb)->dataref), 1);
	atomic_set(&(skb->users), 1);

	shinfo=(struct skb_shared_info *)skb->end;
	shinfo->nr_frags=0;
	shinfo->frag_list=NULL;
	shinfo->gso_size = 0;
	shinfo->gso_segs = 0;
	shinfo->gso_type = 0;
	shinfo->ip6_frag_id = 0;
	
	skb->dst=NULL;
	skb->cloned = 0;
	skb->len=0;
	skb->data_len=0;
	skb->next=NULL;
	skb->protocol=0;
	//skb->iif=0;
	//skb->h_vlan_id=0;
	//skb->h_vlan_nook=NULL;
	//skb->vlan_id_flags=0;
	//skb->nh.raw =NULL;//linux 2619
	skb->network_header =NULL;
	//skb->h.raw=NULL;//linux 2619
	skb->transport_header=NULL;
	#ifdef CONFIG_XFRM//linux 2.6.19
	skb->sp=NULL;
	#endif
	skb->sk=NULL;
	//skb->lanindev=NULL; 
}
#if 0

__init void init_eth_pre_ethpool(void)
{
	int i;
	memset(skb_rx_pre_pool, 0, sizeof(skb_rx_pre_pool));
	
	for(i=0;i<MAX_PRESKB_POOL_SKB_NUM;i++)
	{
		skb_rx_pre_pool[i]=dev_alloc_skb(SKB_BUF_SIZE);

	}	
	skb_pre_pool_num = MAX_PRESKB_POOL_SKB_NUM;
	printk("%d pre free skb \n" ,skb_pre_pool_num );
}

__SRAM
struct skbuff *dev_alloc_from_pre_ethpool(void)
{
	unsigned long flags;
	struct sk_buff* ret=NULL;		

	local_irq_save(flags);
	if(skb_pre_pool_num>=1)
	{
		skb_pre_pool_num--;
		ret = skb_rx_pre_pool[skb_pre_pool_num];	
	}
	local_irq_restore(flags);

	return ret;
}

void refill_pre_ethpool()
{
	int index;

	if(skb_pre_pool_num >=MAX_PRESKB_POOL_SKB_NUM)
		return;
	for(index=skb_pre_pool_num;index<MAX_PRESKB_POOL_SKB_NUM;index++)
	{
		skb_rx_pre_pool[index]=dev_alloc_skb(SKB_BUF_SIZE);
		skb_pre_pool_num++;
	}
}
#endif

__IRAM_NIC
struct sk_buff * dev_alloc_from_rx_pool(int size)
{
	unsigned long flags;
	struct sk_buff* ret=NULL;		

	local_irq_save(flags);
	if(upstream_skb_pool_free_num>=1)
	{
		upstream_skb_pool_free_num--;
		ret = skb_rx_free_pool[upstream_skb_pool_free_num];	
	}
	init_skb_rx_pool(ret);
	local_irq_restore(flags);
	return ret;
}

__SRAM
static __inline  int FreeSkbMemberStruct(struct sk_buff *skb)
{
	dst_release(skb->dst);
	skb->dst=NULL;
#ifdef CONFIG_XFRM
	secpath_put(skb->sp);
	skb->sp=NULL;
#endif
	if (skb->destructor) {
		WARN_ON(in_irq());
		skb->destructor(skb);
		skb->destructor=NULL;
	}
#ifdef CONFIG_NETFILTER
	nf_conntrack_put(skb->nfct);
	skb->nfct=NULL;
#if defined(CONFIG_NF_CONNTRACK) || defined(CONFIG_NF_CONNTRACK_MODULE)
	nf_conntrack_put_reasm(skb->nfct_reasm);
	skb->nfct_reasm=NULL;
#endif
#ifdef CONFIG_BRIDGE_NETFILTER
	nf_bridge_put(skb->nf_bridge);
	skb->nf_bridge=NULL;
#endif
#endif
/* XXX: IS this still necessary? - JHS */
#ifdef CONFIG_NET_SCHED
	skb->tc_index = 0;
#ifdef CONFIG_NET_CLS_ACT
	skb->tc_verd = 0;
#endif
#endif
}

__IRAM_NIC
int free_sk_buffer_to_rx_pool(struct sk_buff *skb)
{
  	unsigned long flags;
  
  	if((skb->src_port != IF_ETH))
    		return 0;
  
	if(unlikely(skb_cloned(skb)))
		return 0;

	 if((upstream_skb_pool_free_num>=MAX_UPSTREAM_CONTROL_POOL)||(upstream_skb_pool_free_num<0))
	 {
			return 0	;
	 }	

	atomic_dec(&re8670_rxskb_num);
	skb->src_port=0;  		
	FreeSkbMemberStruct(skb);
	local_irq_save(flags);
	{
	 	skb_rx_free_pool[upstream_skb_pool_free_num]=skb;
	 	upstream_skb_pool_free_num++;
		if((init_rx_pool_complete ==0)&&(upstream_skb_pool_free_num==(MAX_UPSTREAM_CONTROL_POOL)))
	 			init_rx_pool_complete =1;
		local_irq_restore(flags);
		return 1;
	}
	local_irq_restore(flags);

	return 0;		
}

void  free_rx_pool()
{
	int i;
  
	if(upstream_skb_pool_free_num==0)
		return ;
	
	for(i=0;i<UPSTREAM_CONTROL_POOL_FREE_SKB_NUM;i++)
	{
		if(upstream_skb_pool_free_num>=1){
			
			upstream_skb_pool_free_num--;
			{
				dev_kfree_skb_any(skb_rx_free_pool[upstream_skb_pool_free_num]);
				skb_rx_free_pool[upstream_skb_pool_free_num]=NULL;
			}
	    }
	    else 
		  break;
	}

	if(upstream_skb_pool_free_num==0)
		init_rx_pool_complete =0;
}
#endif //CONFIG_RTL8672_ETHSKB_CONTROL_POOL

//flag 1:tx, 0:rx
static void skb_debug(const struct sk_buff *skb, int enable, int flag)
{
#define NUM2PRINT 50
#define TX 2
#define RX 3
	if (enable) {
		int i;
		if((enable == TX) && !flag)
			return;
		if((enable == RX) && flag)
			return;
		if (!flag)
			printk("\nI: ");
		else
			printk("\nO: ");
		printk("eth len = %d eth name %s", skb->len,skb->dev?skb->dev->name:"");
		for (i=0; i<skb->len && i<NUM2PRINT; i++) {
			if (i%16==0)
				printk("\n");
			printk("%2.2x ", skb->data[i]);
		}
		printk("\n");
	}
}




#ifdef CONFIG_EXT_SWITCH
#include "./rtl8306/rtk_api_ext.h"
#include "./rtl8306/rtk_api.h"

#define CPU_PORT_VLAN_ID    4088
#define LAN_PORT_VLAN_ID_1    4089
#define LAN_PORT_VLAN_ID_2    4090
#define LAN_PORT_VLAN_ID_3    4091
#define WAN_PORT_VLAN_ID    4092   
#define ALL_LAN_PORT_VLAN_ID    4093 

unsigned int g_vlan_tag;

void rtl8306M_CPU_VLAN_Init(void)
{
	rtk_portmask_t mbrmsk,untagmsk;
	rtk_portmask_t rx_portmask;
	rtk_portmask_t tx_portmask;
	uint32 ChipID1; 
	rtk_api_ret_t ret;

	/* first check Chip equal with 0x5988 ? */
	rtl8306e_reg_get(4, 30, 0, &ChipID1);
	printk("\n *****  Get the RTL8306M Manufactory ID=%x *****\n",ChipID1 ); 
		 
	if(rtk_switch_init()!= RT_ERR_OK)
		printk("%s: rtk_switch_init return failed!\n", __func__);

	ret = rtk_cpu_tagPort_set(RTL8306_PORT0, CPU_INSERT_TO_NONE);  //enable Port0 as CPUPort
	if(ret!= RT_ERR_OK){
		printk("%s: rtk_cpu_tagPort_set return %d\n", __func__, ret);
		return;
	}

#ifndef HARDWARE_FORWARD
	//move forward since rtk_vlan_init will set rtl8306e_vlan_tagAware_set(TRUE)!
	// once vlan-tag aware, switch will drop the packet with unknown tag, for hg251r
	rtl8306e_vlan_tagAware_set(FALSE);
	   
	if( (ret = rtk_vlan_init()) != RT_ERR_OK)
		printk("%s: rtk_vlan_init return %d\n", __func__, ret);
	  
	if( (ret = rtk_vlan_destroy(1)) != RT_ERR_OK)
		printk("%s: rtk_vlan_destroy return %d\n", __func__, ret);

/* ---------------------belongs to WAN interface configuration-----------------------------*/
	/* vlan 1 member : port4, port0 :WAN Interface */
	/* vlan 2 member : port0, port1/2/3 :LAN Interface */
	/* vlan 3 member : port0/1/2/3/4 */
	
	  mbrmsk.bits[0]=0x11;
	  untagmsk.bits[0]=0x10; 
	if( (ret= rtk_vlan_set(WAN_PORT_VLAN_ID ,	mbrmsk, untagmsk, 0)) != RT_ERR_OK)
		printk("%s: rtk_vlan_set return %d\n", __func__, ret);

/* ----------------------belongs to LAN interface configuration ----------------------------*/	  
	mbrmsk.bits[0]=0x03;
	untagmsk.bits[0]=0x02; 
	if( (ret= rtk_vlan_set(LAN_PORT_VLAN_ID_1 ,	mbrmsk, untagmsk, 0)) != RT_ERR_OK)
		printk("%s: rtk_vlan_set return %d\n", __func__, ret);

	mbrmsk.bits[0]=0x05;
	untagmsk.bits[0]=0x04; 
	if( (ret= rtk_vlan_set(LAN_PORT_VLAN_ID_2,  mbrmsk, untagmsk, 0)) != RT_ERR_OK)
		printk("%s: rtk_vlan_set return %d\n", __func__, ret);

	mbrmsk.bits[0]=0x09;
	untagmsk.bits[0]=0x08; 
	if( (ret= rtk_vlan_set(LAN_PORT_VLAN_ID_3,  mbrmsk, untagmsk, 0)) != RT_ERR_OK)
		printk("%s: rtk_vlan_set return %d\n", __func__, ret);

	//krammer add a vlan id for all port
	mbrmsk.bits[0]=0x0f;
	untagmsk.bits[0]=0x0e; 
	if( (ret= rtk_vlan_set(ALL_LAN_PORT_VLAN_ID,  mbrmsk, untagmsk, 0)) != RT_ERR_OK)
		printk("%s: rtk_vlan_set return %d\n", __func__, ret);

/*-----------------------------------------------------------------------------------*/
	mbrmsk.bits[0]=0x1f;
	untagmsk.bits[0]=0x1e; 
	if( (ret=  rtk_vlan_set(CPU_PORT_VLAN_ID,  mbrmsk, untagmsk, 0)) != RT_ERR_OK)
		printk("%s: rtk_vlan_set return %d\n", __func__, ret);

	if( (ret=  rtk_vlan_portPvid_set( 0 , CPU_PORT_VLAN_ID , 0)) != RT_ERR_OK)	 //set port 0  PVID=3
		printk("%s: rtk_vlan_portPvid_set return %d\n", __func__, ret);

/* --------------------------belongs to WAN & LAN interface ------------------------------*/
	if( (ret= rtk_vlan_portPvid_set( 1 , LAN_PORT_VLAN_ID_1 , 0)) != RT_ERR_OK)	//set port 1  PVID=2
		printk("%s: rtk_vlan_portPvid_set return %d\n", __func__, ret);

	if( (ret= rtk_vlan_portPvid_set( 2 , LAN_PORT_VLAN_ID_2 , 0)) != RT_ERR_OK)	//set port 2  PVID=2
		printk("%s: rtk_vlan_portPvid_set return %d\n", __func__, ret);

	if( (ret= rtk_vlan_portPvid_set( 3 , LAN_PORT_VLAN_ID_3 , 0)) != RT_ERR_OK)	//set port 3  PVID=2
		printk("%s: rtk_vlan_portPvid_set return %d\n", __func__, ret);

	if( (ret= rtk_vlan_portPvid_set( 4 , WAN_PORT_VLAN_ID , 0)) != RT_ERR_OK)	//set port 4  PVID=1
		printk("%s: rtk_vlan_portPvid_set return %d\n", __func__, ret);
#endif //HARDWARE_FORWARD

	//set global switch tag-unaware for receiving packet with vlan-tag
	//rtl8306e_vlan_tagAware_set(FALSE);

#ifdef HARDWARE_FORWARD
	rtl8306e_portLearningAbility_set(4, TRUE);  //disable port4 learning capability
	printk("Disable CPU port learning ability\n");
	rtl8306e_portLearningAbility_set(0, FALSE);  //disable port0 learning capability
	rtl8306e_portLearningAbility_set(1,TRUE);
	rtl8306e_portLearningAbility_set(2,TRUE);
	rtl8306e_portLearningAbility_set(3,TRUE);
	
	//set output drop
	rtl8306e_regbit_set(0, 18, 13, 0, 0);
#else 
	rtl8306e_portLearningAbility_set(4, FALSE);  //disable port4 learning capability
	printk("Disable CPU port learning ability\n");
	rtl8306e_portLearningAbility_set(0, FALSE);  //disable port0 learning capability
	rtl8306e_portLearningAbility_set(1,TRUE);
	rtl8306e_portLearningAbility_set(2,TRUE);
	rtl8306e_portLearningAbility_set(3,TRUE);
	
	//set output drop
	rtl8306e_regbit_set(0, 18, 13, 0, 0);
	
	/*alex_huang make sure wan port packet trap to cpu 20100904*/
	rtl8306e_regbit_set(2, 22, 5, 3, 1);
	rtl8306e_regbit_set(4, 17, 6, 2, 0);
	rtl8306e_regbit_set(4, 17, 8, 2, 1);
	rtl8306e_regbit_set(4, 17, 7, 2, 1);
	/*alex_huang end*/
	
	/*Disable Vlan trap2cpu =0*/
	rtl8306e_regbit_set(2, 22, 6, 3, 0);

	//vlan leaky
	rtl8306e_regbit_set(0,18,11,0,0);
	//rtl8306e_regbit_set(0,18,10,0,0);	//arp pkt
#endif //HARDWARE_FORWARD

	rtl8306e_regbit_set(0,23,7,0,1);
	mdelay(1000);
	rtl8306e_regbit_set(0,23,7,0,0);
}
#endif //CONFIG_EXT_SWITCH

extern unsigned int tr069_pid;
static unsigned char already_stop_tr069 = 0;
static struct task_struct* find_tsk(unsigned int pid){
	#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
	struct task_struct* tsk;
	rcu_read_lock();
	tsk = find_task_by_vpid(pid);
	rcu_read_unlock();
	return tsk;
	#else
	return find_task_by_pid(pid);
	#endif
}
static void notify_process(unsigned int pid, int sig){
	#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
	struct pid *tmp;
	rcu_read_lock();
	tmp = get_pid(find_vpid(pid));
	rcu_read_unlock();
	kill_pid(tmp, sig, 1);
	#else
	kill_proc(pid, sig, 1);
	#endif
}
void tr069_stop(void)
{
	struct task_struct *tsk;
	if(!already_stop_tr069 && tr069_pid)
	{
		notify_process(tr069_pid, SIGSTOP);
		//printk("pid %d\n", tr069_pid);
		tsk = find_tsk( tr069_pid );
		if(tsk && tsk->parent){
			//printk("pid %d\n", tsk->parent->pid);
			notify_process(tsk->parent->pid, SIGSTOP);
			tsk = find_tsk( tsk->parent->pid );
			if(tsk && tsk->parent){
				//printk("pid %d\n", tsk->parent->pid);
				notify_process(tsk->parent->pid, SIGSTOP);
				already_stop_tr069 = 1;
			}
		}
	}
}
void tr069_cont(void)
{
	struct task_struct *tsk;
	if(already_stop_tr069 && tr069_pid)
	{
		notify_process(tr069_pid, SIGCONT);
		tsk = find_tsk( tr069_pid );
		if(tsk && tsk->parent){
			//printk("pid %d\n", tsk->parent->pid);
			notify_process(tsk->parent->pid, SIGCONT);
			tsk = find_tsk( tsk->parent->pid );
			if(tsk && tsk->parent){
				//printk("pid %d\n", tsk->parent->pid);
				notify_process(tsk->parent->pid, SIGCONT);
				already_stop_tr069 = 0;
			}
		}
	}
}

static unsigned char portstatus[SW_PORT_NUM]={0};
static struct proc_dir_entry *Port_Status_Proc_File;
#define PROCFS_NAME 		"eth_port_status"
int port_status_proc_read(char *page, char **start,
                 off_t off, int count,
                 int *eof, void *data)
{
    int len = 0;
	int i = 0;
	//this format is important, if you want to change, plz check if the
	//spppd can work fine
	for(i=0;i<SW_PORT_NUM;i++){
    	len += sprintf((page + len), "Port%d %s\n", i, portstatus[i]?"LinkUp":"LinkDown");
		len += sprintf((page + len), "\n");
	}

    return len;
}

static int nicLinkStatCheckCnt=0;
static unsigned int linkChange=0;
static unsigned int linkchangedelay=0;
static unsigned int POLLINGTICK= 200;
static int brgtimeoutCheckCnt=0;
__IRAM_NIC void rtl867x_eth_status_polling(void)
{
	int i;
	
	if (nicLinkStatCheckCnt++ >= POLLINGTICK /* polling freq 2sec. */)
	{
		nicLinkStatCheckCnt = 0;
#ifndef CONFIG_EXT_SWITCH
		/*check nic link status every 1s*/
		unsigned short status;
		status = *(volatile unsigned short *)0xB801803E;
		if (status & (1<<8))
		{
			printk("RTL8672: Link Status Changed\n");
			*(volatile unsigned short *)0xB801803E = (1<<8)&0xffff;
			//bhalWakeupMonitorTask();
		}
#else
		//krammer move out
		//static unsigned char portstatus[SW_PORT_NUM]={0};
		unsigned short regValue;
		unsigned int lnkchg=0;

		for (i=0;i<SW_PORT_NUM;i++)
		{
			miiar_read(virt2phy[i], 1, &regValue);
			// If the link had ever failed, this bit will be 0 until after reading this bit again.
			miiar_read(virt2phy[i], 1, &regValue);
			if (((regValue>>2)&0x01) != portstatus[i]) {
#ifdef CONFIG_RTL8672_ETHSKB_CONTROL_POOL			
				net_smallpkt_heavytraffic=0;
#endif //CONFIG_RTL8672_ETHSKB_CONTROL_POOL			
#ifdef CONFIG_RTL8672_BRIDGE_FASTPATH
				brgClearTable();				
#endif //CONFIG_RTL8672_BRIDGE_FASTPATH

				if((virt2phy[i]==WAN_ETH_PORT) && (0==linkChange)) {
					linkChange =1;
					linkchangedelay=0;
					rtl8306e_regbit_set(0,23,7,0,1);
					printk("enable fast aging...\n");
				}
				if ((regValue>>2)&0x01)
					printk("port%d is link up\n", i);
				else {
					printk("port%d is link down\n", i);
					if (0x6167 == swtype) {
						miiar_write(virt2phy[i], 0, 0x1200);
					}
				}
				portstatus[i] = ((regValue>>2)&0x01);		
				//bhalWakeupMonitorTask();
				if (0x5988 == swtype) {
					sw_link_stable_times[i]=0;
					sw_link_checked[i]=0;
					lnkchg = 1;
				}
			}
		}
		if(lnkchg && !timer_pending(&sw_check_timer)){
			mod_timer(&sw_check_timer, jiffies + 1*HZ);
		}
#endif //CONFIG_EXT_SWITCH
#ifdef CONFIG_RTL8672_ETHSKB_CONTROL_POOL
		if(!net_smallpkt_heavytraffic)
		{
			free_rx_pool();
			tr069_cont();
			#if 0
			refill_pre_ethpool();
			#endif
		}else{
				if((jiffies -NicRxPollStartStamp)>HZ)
				{

					net_smallpkt_heavytraffic=0;
				}
		}
#endif //CONFIG_RTL8672_ETHSKB_CONTROL_POOL
	}

	if (linkChange && (linkchangedelay++>100)) {
		linkChange=0;
		rtl8306e_regbit_set(0,23,7,0,0); 
		printk("disable fast aging...\n");
	}

#ifdef CONFIG_RTL8672_ETHSKB_CONTROL_POOL
	if(!net_smallpkt_heavytraffic)
	{
		POLLINGTICK=200;
		//if(qos_enable)
		//{
		// 	BRG_ENTRY_AGING_TIME =BRG_ENTRY_AGING_TIME_NORMAL;
		// 	BRG_ENTRY_FORCE_TIMEOUT=BRG_ENTRY_FORCE_TIMEOUT_NORMAL;
		//}
	}else{		
		POLLINGTICK=500;
		tr069_stop();
	}
	if((brgtimeoutCheckCnt++>20000))
	{
		brgtimeoutCheckCnt=0;
		if((!net_smallpkt_heavytraffic))
		{
			BRG_ENTRY_AGING_TIME =BRG_ENTRY_AGING_TIME_NORMAL;
		 	//BRG_ENTRY_FORCE_TIMEOUT=BRG_ENTRY_AGING_TIME_NORMAL;
		}
	}
#endif
}
#endif //CONFIG_ETHWAN


#ifdef CONFIG_ETHWAN
__IRAM_NIC
#endif //CONFIG_ETHWAN
struct sk_buff *re8670_getAlloc(unsigned int size)
{	
struct sk_buff *skb=NULL;
	
//printk("skb=%x cb[0]=%d\n",(unsigned int)skb , skb->cb[0] );
    if ( atomic_read(&re8670_rxskb_num) < RE8670_MAX_ALLOC_RXSKB_NUM ) {
	//printk("free pre-alloc skb - unknow state...\n");
	#if 0 //def CONFIG_SKB_POOL_PRIV
	skb = priv_skb_alloc(size);
	#else
#ifdef CONFIG_ETHWAN
#ifdef CONFIG_RTL8672_ETHSKB_CONTROL_POOL
#if 0
	if(net_smallpkt_heavytraffic)
	{
		if((upstream_skb_pool_free_num==0)||(init_rx_pool_complete==0))
		{
			if(skb_pre_pool_num==0)
				skb=  dev_alloc_skb(size);
			else 
				skb=dev_alloc_from_pre_ethpool();
		} else {
			skb= dev_alloc_from_rx_pool(size);
		}	
	} else {
		skb=  dev_alloc_skb(size);
	}	
#endif
	if((upstream_skb_pool_free_num==0)||(init_rx_pool_complete==0))
	{
		skb=  dev_alloc_skb(size);
	} else {
		skb= dev_alloc_from_rx_pool(size);
	}	
#else
	skb = dev_alloc_skb(size);
#endif //CONFIG_RTL8672_ETHSKB_CONTROL_POOL
#else
	#ifdef CONFIG_RTL865X_ETH_PRIV_SKB  //For RTL8192CE
	skb = dev_alloc_skb_priv_eth(CROSS_LAN_MBUF_LEN);
	#else
	skb = dev_alloc_skb(size);
	#endif
#endif //CONFIG_ETHWAN

	#endif
        if (skb!=NULL) {
        	atomic_inc(&re8670_rxskb_num);
            //re8670_rxskb_num++;
            skb->src_port=IF_ETH;
            //printk("GetE %d\n", re8670_rxskb_num);
        };
    }
#ifndef CONFIG_MEM_LIMITATION
    else {
		printk("%s(%d): limit reached (%d/%d)\n",__FUNCTION__,__LINE__,atomic_read(&re8670_rxskb_num),RE8670_MAX_ALLOC_RXSKB_NUM);
    }
#endif
    return skb;
}

static inline void re8670_set_rxbufsize (struct re_private *cp)
{
#if defined(CONFIG_SKB_POOL_PREALLOC) && defined(CONFIG_RTL867X_PACKET_PROCESSOR)
	cp->rx_buf_sz = SKB_BUF_SIZE;
#else
	unsigned int mtu = cp->dev->mtu;
	if (mtu > ETH_DATA_LEN)
		/* MTU + ethernet header + FCS + optional VLAN tag */
		cp->rx_buf_sz = mtu + ETH_HLEN + 8;
	else
		cp->rx_buf_sz = PKT_BUF_SZ;
#endif	
}


#ifdef CONFIG_EXT_SWITCH
void process_eth_vlan(struct sk_buff *skb, int *tag)
{
	#define TYPE_OFFSET_802_1Q		12
	#define TYPE_802_1Q			0x8100
	#define VID_OFFSET_802_1Q		14
	unsigned int source_port;
	
	source_port = SW_PORT_NUM;
	//printk("*****vlan tag=%d\n", *tag);
	for(source_port=0;source_port<SW_PORT_NUM;source_port++)
		if (*tag == rtl8305_info.vlan[rtl8305_info.phy[virt2phy[source_port]].vlanIndex].vid)
			break;

	if (source_port < SW_PORT_NUM) {
		//printk("source_port=%d\n", source_port);
		skb->switch_port = (char *)dev_sw[source_port]->name;
		
		// Port-mapping --- set the vlan_member
		skb->vlan_member = rtl8305_info.vlan[rtl8305_info.phy[virt2phy[source_port]].vlanIndex].member;
		// Kaohj -- for vlan-grouping, SW_PORT+cpu-port+system-lan-port
		skb->pvid = rtl8305_info.phy[SW_PORT_NUM+1+virt2phy[source_port]].vlanIndex;
		//printk("source_port=%d, pvid=%d\n", source_port, skb->pvid);
	};
}


#ifdef CONFIG_ETHWAN

//use dev name to get port number
static inline unsigned int dev2port(struct net_device *dev){
	
	if (dev->name[0] != 'e') {//not eth0.# wanport
		return ETH_WAN_PORT;
	}
	else if(dev->name[4] == '.'){
		return dev->name[5]-'2';
	}
	else{
		return SW_PORT_NUM;
	}
}

#define HWADDRLEN			12
__DRAM unsigned char hwaddr[HWADDRLEN];

__IRAM_NIC
int process_cpu_tag(struct sk_buff *skb,int *port_tag)
{
	int idx;
	unsigned int source_port;
	unsigned char source_portmap;
	#define TYPE_OFFSET_CPUTAG_802_1Q 16
	#define TYPE_802_1Q      0x8100
	#define CPUTAG_PORT_MASK_OFFSET          15
	#define TYPE_OFFSET_802_1P_PRO             18
	#define TYPE_OFFSET_802_1P_VID  (TYPE_OFFSET_802_1P_PRO+1)

	source_portmap = ((unsigned char )(skb->data[CPUTAG_PORT_MASK_OFFSET]<<2))>>2;	

	#if 0
	for(idx = 0 ; idx<HWADDRLEN/2 ; idx++)
	{
		*(unsigned short *)(hwaddr+idx*2)=	*(unsigned short *)&(skb->data[idx*2]);
	}
	#else
	*(unsigned short *)&(skb->data[14])=    *(unsigned short *)&(skb->data[10]);
       *(unsigned short *)&(skb->data[12])=    *(unsigned short *)&(skb->data[8]);
       *(unsigned short *)&(skb->data[10])=    *(unsigned short *)&(skb->data[6]);
       *(unsigned short *)&(skb->data[8])=     *(unsigned short *)&(skb->data[4]);
       *(unsigned short *)&(skb->data[6])=     *(unsigned short *)&(skb->data[2]);
       *(unsigned short *)&(skb->data[4])=     *(unsigned short *)&(skb->data[0]);
	#endif

	skb_pull(skb,4);

	#if 0
	for(idx = 0 ; idx<HWADDRLEN/2 ; idx++)
	{
		*(unsigned short *)&(skb->data[idx*2])=*(unsigned short *)(hwaddr+idx*2);
	}
	#endif

	for(source_port=0;source_port<SW_PORT_NUM;source_port++)
	{
		if((source_portmap>>virt2phy[source_port])==1)
			break;
	}
	*port_tag =  virt2phy[source_port]; // phy port
	
	// printk("source_port = %d \n",*port_tag);
	if (source_port < SW_PORT_NUM) {
		// Port-mapping --- set the vlan_member
		skb->vlan_member = rtl8305_info.vlan[rtl8305_info.phy[virt2phy[source_port]].vlanIndex].member;
		ETHDBG_PRINT("(%p)skb->vlan_member = %x, port %d, vlanindex %d\n", skb, skb->vlan_member, source_port, \
				rtl8305_info.phy[virt2phy[source_port]].vlanIndex);
		if(source_port < ETH_WAN_PORT){//only record in lan port
			skb->switch_port = (char *)dev_sw[source_port]->name;
			// Kaohj -- for vlan-grouping, SW_PORT+cpu-port+system-lan-port
			skb->pvid = rtl8305_info.phy[SW_PORT_NUM+1+virt2phy[source_port]].vlanIndex;
#ifdef CONFIG_PPPOE_PROXY
			skb->source_port = source_port;
#endif
		}
	}

	return source_port;
}

int insert_cputag(struct sk_buff **skb,unsigned char mask)
{
	struct sk_buff *tmpskb;
	int i;
	unsigned char insert[16]; 

	if (skb_headroom(*skb) < 4) {
		tmpskb = skb_realloc_headroom(*skb, 4);
		dev_kfree_skb_any(*skb);
		if (tmpskb == NULL)
			return 0;
		*skb = tmpskb;
	}else{
		if(skb_cloned(*skb))
		{
			tmpskb = skb_copy(*skb, GFP_ATOMIC);
			dev_kfree_skb_any(*skb);
			if (tmpskb == NULL)
				return 0;
			*skb = tmpskb;
		}
	}

	for(i = 0 ; i<12 ; i++)
	{
		insert[i] = (*skb)->data[i];
	}
	insert[12] = 0x88;
	insert[13] = 0x99;
	insert[14] = 0x90;
	insert[15] = mask;
	skb_push(*skb,4);

	for(i = 0 ; i<16 ; i++)
	{
		(*skb)->data[i] = insert[i];
	}
	return 1;
}

#else
#if (VPORT_USE==VPORT_CPU_TAG)
void process_cpu_tag(struct sk_buff *skb,int *port_tag)
{
	int idx;
	#define HWADDRLEN			12
	unsigned char hwaddr[HWADDRLEN];
	int vlan_tag_packet = 0;
	
	//tylo, for port-mapping
	unsigned int source_port;
	unsigned char source_portmap;
	#define TYPE_OFFSET_CPUTAG_802_1Q	16
	#define TYPE_802_1Q			0x8100
	#define VID_OFFSET_802_1Q		14
	#define CPUTAG_PORT_MASK_OFFSET		15
	#define TYPE_OFFSET_802_1P_PRO		18
	
	//bit map cpu tag tx/rx = bit 0 bit 1 bit 2 bit 3 bit 4 bit 5 bit 6  map 6 port
	// printk("skb data[15] %x\n",skb->data[CPUTAG_PORT_MASK_OFFSET]);
	source_portmap = ((unsigned char )(skb->data[CPUTAG_PORT_MASK_OFFSET]<<2))>>2;
	if(*((unsigned short *)&(skb->data[TYPE_OFFSET_CPUTAG_802_1Q])) == TYPE_802_1Q ) {
		// Kaohj -- use skb->mark for 801.1p match
		//skb->isVlanPack = ((skb->data[TYPE_OFFSET_802_1P_PRO]&0xe0)>>5)+1;
		skb->mark = ((skb->data[TYPE_OFFSET_802_1P_PRO]&0xe0)>>5)+1;
		// printk("vlan tag packet ------,skb vlan-prio = %d\n",skb->isVlanPack);
		vlan_tag_packet =1;
	}
	// save the vlan tag here
	//printk("*****from port  tag=%d\n", source_portmap);
	for(idx = 0 ; idx<HWADDRLEN ; idx++)
	{
		hwaddr[idx] = skb->data[idx];
	}
	
	if(vlan_tag_packet)
		skb_pull(skb,8);
	else
		skb_pull(skb,4);

	for(idx = 0 ; idx<HWADDRLEN ; idx++)
	{
		skb->data[idx]=hwaddr[idx];
	}
	
	for(source_port=0;source_port<SW_PORT_NUM;source_port++)
	{
		if((source_portmap>>virt2phy[source_port])==1)
			break;
	}
	*port_tag =  virt2phy[source_port]; // phy port
	
	// printk("source_port = %d \n",*port_tag);
	if (source_port < SW_PORT_NUM) {
		skb->switch_port = (char *)dev_sw[source_port]->name;
		// Port-mapping --- set the vlan_member
		skb->vlan_member = rtl8305_info.vlan[rtl8305_info.phy[virt2phy[source_port]].vlanIndex].member;
		// Kaohj -- for vlan-grouping, SW_PORT+cpu-port+system-lan-port
		skb->pvid = rtl8305_info.phy[SW_PORT_NUM+1+virt2phy[source_port]].vlanIndex;
#ifdef CONFIG_PPPOE_PROXY
		skb->source_port = source_port;
#endif
	}
}
#endif

#if (IGMP_SNOOPING_USE==IGMP_CPU_TAG)
int insert_cputag(struct sk_buff **skb,unsigned char mask)
{
	struct sk_buff *tmpskb;
	int i;
	unsigned char insert[16]; 

	if (skb_headroom(*skb) < 4) {
		tmpskb = skb_realloc_headroom(*skb, 4);
		dev_kfree_skb_any(*skb);
		if (tmpskb == NULL)
			return 0;
		*skb = tmpskb;
	}else{
		if(skb_cloned(*skb))
		{
			tmpskb = skb_copy(*skb, GFP_ATOMIC);
			dev_kfree_skb_any(*skb);
			if (tmpskb == NULL)
				return 0;
			*skb = tmpskb;
		}
	}

	for(i = 0 ; i<12 ; i++)
	{
		insert[i] = (*skb)->data[i];
	}
	insert[12] = 0x88;
	insert[13] = 0x99;
	insert[14] = 0x90;
	insert[15] = mask;
	skb_push(*skb,4);

	for(i = 0 ; i<16 ; i++)
	{
		(*skb)->data[i] = insert[i];
	}
	return 1;
}
#endif
#endif //CONFIG_ETHWAN
#endif // of CONFIG_EXT_SWITCH

atomic_t lock_tx_tail = ATOMIC_INIT(0);
#ifdef CONFIG_ETHWAN
/*__IRAM_NIC*/
void nic_tx2(struct sk_buff *skb, struct net_device *tdev)
{
	skb->users.counter=1;
	//if(debug_enable)  printk("%s: calling re8670_start_xmit\n", __func__);
	if (tdev)
		re8670_start_xmit(skb,tdev);
	else
		re8670_start_xmit(skb,eth_net_dev);
}
#else
/*__IRAM_NIC*/ void nic_tx2(struct sk_buff *skb)
{
	skb->users.counter=1;
	re8670_start_xmit(skb,eth_net_dev);
}
#endif //CONFIG_ETHWAN

#if 0	//shlee 2.6
extern void sar_tx(struct sk_buff *skb, unsigned int ch_no);
#endif
extern unsigned int find_mac(unsigned char *buf);	

#ifdef FAST_ROUTE_Test

unsigned int nic_rx_counter=0;
#define MAX_MAC_ADDR	8
#define	Enable_VC_CNT	16
typedef struct _mac_table_items
{
	unsigned int index;
	unsigned int enable;
	unsigned char mac[MAX_MAC_ADDR][6];
} mac_table_items;

extern mac_table_items  fast_route_ch_no[Enable_VC_CNT];

#endif	//FAST_ROUTE_Test

#if 0	//shlee 2.6
extern void sar_tx(struct sk_buff *skb, unsigned int ch_no);//shlee 8672
extern void skb_debug(const struct sk_buff *skb, int enable, int flag);	//shlee 8672
extern int debug_pvc;	//shlee 8672
#endif

#ifdef WLANBR_SHORTCUT  //For RTL8192CE
__IRAM_NIC static void cache_eth_mac_addr(unsigned char *mac_addr, struct net_device *dev){
	int i;

	//find one empty record
	for(i=0; i<MAX_CACHED_ETH_ADDR; i++){
		if(cached_dev[i]==NULL){
			memcpy(cached_eth_addr[i], mac_addr, 6);
			cached_dev[i] = dev;
			return;
		}
		if(!memcmp(mac_addr, cached_eth_addr[i], 6))
			return;
	}

	//replace the cache_index record
	memcpy(cached_eth_addr[cache_index], mac_addr, 6);
	cached_dev[cache_index] = dev;
	cache_index++;
	if(cache_index == MAX_CACHED_ETH_ADDR)
		cache_index=0;
	
}
__IRAM_NIC void clear_cached_eth_mac_addr(void){
	int i;
	for(i=0; i<MAX_CACHED_ETH_ADDR; i++){
		memset(cached_eth_addr[i],0,6);
		cached_dev[i]=NULL;
	}
	cache_index=0;
}
__IRAM_NIC struct net_device *get_eth_cached_dev(unsigned char *da){
	int i;
	for(i=0; i<MAX_CACHED_ETH_ADDR; i++){
		if(!memcmp(da, cached_eth_addr[i], 6) && (cached_dev[i]!=NULL))
			return cached_dev[i];
	}
	return NULL;
}
#endif


static
#ifdef CONFIG_ETHWAN
__IRAM_NIC
#else
inline
#endif //CONFIG_ETHWAN
void re8670_rx_skb (struct re_private *cp, struct sk_buff *skb,
			      DMA_DESC *desc)
{
#ifdef CONFIG_EXT_SWITCH
	int tag=0;
#ifdef CONFIG_ETHWAN
	int port=SW_PORT_NUM;
#endif //CONFIG_ETHWAN		
#endif
#if defined (CONFIG_NET_WIRELESS) || defined(CONFIG_WIRELESS_EXT)
#ifdef WLANBR_SHORTCUT
	extern struct net_device *get_shortcut_dev(unsigned char *da);
	struct net_device *wlandev;
#endif
#endif

#ifdef CONFIG_ETHWAN
	SKB_DBG(skb, debug_enable, 0);
#endif //CONFIG_ETHWAN

#if 0
//	printk("Perform directly bridge pkt to SAR Tx\n");	
	skb->dev = sar_dev;
	skb->pkt_type = PACKET_FASTROUTE;
	sar_tx(skb, 0);
	return;
#endif	
#if 0
	nic_tx2(skb);
	return;
#endif	
#ifdef CONFIG_EXT_SWITCH
#ifdef CONFIG_ETHWAN
	if (*((unsigned short*)&skb->data[12]) == 0x8899)
	{
		port = process_cpu_tag(skb, &tag);
		ETHDBG_PRINT("get skb port %d\n",port);
	}
#else
#if (VPORT_USE==VPORT_CPU_TAG)
	
	     if (*((unsigned short*)&skb->data[12]) == 0x8899)
            {
       	//   printk("skb match cpu tag\n");
	   	   process_cpu_tag(skb, &tag);
		  
            }	   
#endif //(VPORT_USE==VPORT_CPU_TAG)
#endif //CONFIG_ETHWAN
#endif //CONFIG_EXT_SWITCH

#ifdef FAST_ROUTE_Test
#if 0
	unsigned int ch_no;	

	if (fast_route_mode) {
		if (( (ch_no = find_mac(skb->data+6)) != -1) && fast_route_ch_no[ch_no].enable) {
			skb->dev = sar_dev;
			skb->pkt_type = PACKET_FASTROUTE;
			sar_tx(skb, ch_no);
			return;
		}
	}
#endif
#endif

#ifdef CONFIG_ETHWAN
//	printk("rx port=%d, vnet_dev_status[%d]= %d\n", port, port, vnet_dev_status[port]);
	if ((port >= SW_PORT_NUM) || (!vnet_dev_status[port]))
	{
		ETHDBG_PRINT("free skb port %d\n",port);
		SKB_DBG(skb, debug_enable, 0);
		dev_kfree_skb(skb);
		return;
	}

	ETHDBG_PRINT("get skb port pass\n");

#ifdef CONFIG_RTL8672_ETHSKB_CONTROL_POOL	
	//if(skb->len < 300) {
	if (1) {	
			if(NicRxPollStartStamp==0) 
				NicRxPollStartStamp = jiffies;

			if(NicRxPollSmallPktCount++ > 50) {
				if((jiffies - NicRxPollStartStamp) <= 1) {
					net_smallpkt_heavytraffic = 1;
					NicRxPollSmallPktCount = 0;
					NicRxPollStartStamp = jiffies;
                    BRG_ENTRY_AGING_TIME = BRG_ENTRY_AGING_TIME_HEAVY;  
				} else {
					NicRxPollStartStamp = jiffies;
					NicRxPollSmallPktCount = 0;
					net_smallpkt_heavytraffic =0;
				}
			}
	} else {
				net_smallpkt_heavytraffic = 0;
				NicRxPollStartStamp = 0;
				NicRxPollSmallPktCount = 0;
	}	
#endif //CONFIG_RTL8672_ETHSKB_CONTROL_POOL

	skb->dev = vnet_dev[port];
	vnet_dev[port]->last_rx = jiffies;
	ETHDBG_PRINT("This packet comes from %s (port %d)\n", skb->dev->name, port);

	cp->net_stats[port].rx_packets++;
	cp->net_stats[port].rx_bytes += skb->len;

	if(port<SW_PORT_NUM-1){
		/* update eth0 if port != nas0 */		
		cp->net_stats[SW_PORT_NUM].rx_packets++;
		cp->net_stats[SW_PORT_NUM].rx_bytes += skb->len;
	}

#else
	skb->dev = eth_net_dev;
#endif //CONFIG_ETHWAN
	skb->from_dev=skb->dev;


#ifdef CONFIG_PPPOE_PROXY_FASTPATH
       skb->islanPPP = 0;
        if((skb->data[12]==0x88)&&(skb->data[13]==0x64)&&(skb->data[20]==0x00)&&(skb->data[21]==0x21))
        {
      
	    skb->protocol = eth_type_trans (skb, cp->dev);
	    if(skb->pkt_type == PACKET_HOST )
	   {
	       skb->islanPPP =1;
	       skb->protocol =  __constant_htons(ETH_P_IP);
	 
	    }

             
	}
        else
         if(((skb->data[12]==0x88)&&(skb->data[13]==0x64))&&((skb->data[20]==0x80)&&(skb->data[21]==0x21))){
		skb->protocol = eth_type_trans (skb, cp->dev);
		if(skb->pkt_type == PACKET_HOST ){			
			regMacType(skb->mac.ethernet->h_source,PACKET_FROM_LAN);
		
		}
               
	   }
		
        else
#endif			

	skb->protocol = eth_type_trans (skb, skb->dev);
	

//eason close for test
#ifndef CONFIG_ETHWAN
#ifdef CONFIG_EXT_SWITCH
#if (VPORT_USE==VPORT_VLAN_TAG)
	if (desc->opts2 & 0x10000) {
		tag = ((desc->opts2 & 0x0f)<<8); // VIDH
		tag |= ((desc->opts2 & 0xff00)>>8); // VIDL
		
#ifdef CONFIG_RE8305
		if (!enable_virtual_port)
#endif	
	        { // if virtual-port is enabled, then the vlan tag has been replaced by switch(830x)
			// marked for 802.1p based priority
			//skb->nfmark = ((desc->opts2 & 0x00e0) >> 5) + 1;
			/*linux-2.6.19*/ 
			skb->mark = ((desc->opts2 & 0x00e0) >> 5) + 1;
		}
		
		//if(!enable_IGMP_SNP)
		process_eth_vlan(skb,  &tag);
		
	}
#endif	

	// Kaohj
	// Port-Mapping: vlan member is set at sar driver on receving packets
	//skb->vlan_member=0;
	if(enable_IGMP_SNP){
		check_IGMP_snoop_rx(skb, tag);		
	}
	
	#ifdef CONFIG_IPV6
	// Mason Yu. MLD snooping
	if(enable_MLD_SNP){	
		check_MLD_snoop_rx(skb, tag);
	}
	#endif
	
#endif    

// Mason Yu. combine_1p_4p_PortMapping
#ifndef CONFIG_EXT_SWITCH
	skb->vlan_member=eth_dev_member;
#endif
#endif //CONFIG_ETHWAN


#ifdef CONFIG_ETHWAN
#ifdef CONFIG_RTL8672_BRIDGE_FASTPATH
#if 0 //Kevin, merge bridge fastpath into netif_rx()
//if(!net_smallpkt_heavytraffic)
{
	if (skb->pkt_type==PACKET_OTHERHOST)
	{
		int dir=DIR_LAN;
			
		if (port == SW_PORT_NUM-1)//wan
			dir = DIR_WAN;			
		else
			dir = DIR_LAN;
		skb_push(skb, ETH_HLEN);
		if (brgShortcutProcess(skb, skb->dev, dir)) {

			return;
		}
		skb_pull(skb, ETH_HLEN);
	}
}	
#endif //if 0
#endif //CONFIG_RTL8672_BRIDGE_FASTPATH
#else


	cp->net_stats.rx_packets++;
	cp->net_stats.rx_bytes += skb->len;
	cp->dev->last_rx = jiffies;

#if CP_VLAN_TAG_USED
	if (cp->vlgrp && (desc->opts2 & RxVlanTagged)) {
		vlan_hwaccel_rx(skb, cp->vlgrp, desc->opts2 & 0xffff);
	} else
#endif
#endif //CONFIG_ETHWAN

#ifdef CONFIG_ETHWAN
	ETHDBG_PRINT("get skb port pass 2\n");
	SKB_DBG(skb, debug_enable, 0);
#endif //CONFIG_ETHWAN



		//printk("re8670_rx_skb: skb->switch_port = %s\n", skb->switch_port);
#if defined (CONFIG_NET_WIRELESS) || defined(CONFIG_WIRELESS_EXT)
#ifdef WLANBR_SHORTCUT
#if LINUX_VERSION_CODE == 132638
		if ((!(skb->mac_header[0] & 0x01)) && (skb->dev->br_port) &&			
			((wlandev = get_shortcut_dev(skb->mac_header)) != NULL) &&
			netif_running(wlandev)) {
				cache_eth_mac_addr(skb->mac_header + ETH_ALEN,skb->dev);
				skb_push(skb, ETH_HLEN);			
				memcpy(skb->data, skb->mac_header, ETH_ALEN);			
				wlandev->hard_start_xmit(skb, wlandev);		
				return; //let's return, dont need to netif_rx. for hg251r
		}
#else
		if ((!(skb->mac.ethernet->h_dest[0] & 0x01)) && (skb->dev->br_port) &&			
			((wlandev = get_shortcut_dev(skb->mac.ethernet->h_dest)) != NULL) &&
			netif_running(wlandev)) {
				cache_eth_mac_addr(skb->mac.ethernet->h_source,skb->dev);			
				skb_push(skb, ETH_HLEN);			
				memcpy(skb->data, skb->mac.ethernet, ETH_HLEN);			
				wlandev->hard_start_xmit(skb, wlandev);		
				return; //let's return, dont need to netif_rx. for hg251r
		}				
#endif // LINUX_VERSION
#endif
#endif //defined (CONFIG_NET_WIRELESS) || defined(CONFIG_WIRELESS_EXT)


#ifdef CONFIG_ETHWAN
	// Kaohj
	// Port-Mapping: vlan member is set at sar driver on receving packets
	//skb->vlan_member=0;
	if(enable_IGMP_SNP && port != ETH_WAN_PORT){//only lan should check		
		check_IGMP_snoop_rx(skb, tag);		
	}
	
	// Mason Yu. MLD snooping
	#ifdef CONFIG_IPV6
	if(enable_MLD_SNP && port != ETH_WAN_PORT){//only lan should check
		check_MLD_snoop_rx(skb, tag);
	}
	#endif
#endif //CONFIG_ETHWAN


#ifdef CONFIG_RTL8672_NAPI

		netif_receive_skb(skb);
#else
		if (netif_rx(skb) == NET_RX_DROP)
#ifdef CONFIG_ETHWAN	
			cp->net_stats[port].rx_dropped++;
#else		
			cp->net_stats.rx_dropped++;
#endif //CONFIG_ETHWAN				
#endif //CONFIG_RTL8672_NAPI


}

#ifndef CONFIG_RTL867X_PACKET_PROCESSOR
static void re8670_rx_err_acct (struct re_private *cp, unsigned rx_tail,
			    u32 status, u32 len)
{
	if (netif_msg_rx_err (cp))
		printk (KERN_DEBUG
			"%s: rx err, slot %d status 0x%x len %d\n",
			cp->dev->name, rx_tail, status, len);
#ifdef CONFIG_ETHWAN
	cp->net_stats[SW_PORT_NUM].rx_errors++;
	if (status & RxErrFrame)
		cp->net_stats[SW_PORT_NUM].rx_frame_errors++;
	if (status & RxErrCRC)
		cp->net_stats[SW_PORT_NUM].rx_crc_errors++;
	if (status & RxErrRunt)
		cp->net_stats[SW_PORT_NUM].rx_length_errors++;
#else
	cp->net_stats.rx_errors++;
	if (status & RxErrFrame)
		cp->net_stats.rx_frame_errors++;
	if (status & RxErrCRC)
		cp->net_stats.rx_crc_errors++;
	if (status & RxErrRunt)
		cp->net_stats.rx_length_errors++;
#endif //CONFIG_ETHWAN	
}
#endif

#if 0
static void re8670_rx_frag (struct re_private *cp, unsigned rx_tail,
			struct sk_buff *skb, u32 status, u32 len)
{
	struct sk_buff *copy_skb, *frag_skb = cp->frag_skb;
	unsigned orig_len = frag_skb ? frag_skb->len : 0;
	unsigned target_len = orig_len + len;
	unsigned first_frag = status & FirstFrag;
	unsigned last_frag = status & LastFrag;

	if (netif_msg_rx_status (cp))
		printk (KERN_DEBUG "%s: rx %s%sfrag, slot %d status 0x%x len %d\n",
			cp->dev->name,
			cp->dropping_frag ? "dropping " : "",
			first_frag ? "first " :
			last_frag ? "last " : "",
			rx_tail, status, len);

	cp->cp_stats.rx_frags++;

	if (!frag_skb && !first_frag)
		cp->dropping_frag = 1;
	if (cp->dropping_frag)
		goto drop_frag;

	//copy_skb = dev_alloc_skb (target_len + RX_OFFSET);
	copy_skb = re8670_getAlloc(SKB_BUF_SIZE);
	if (!copy_skb) {
		printk(KERN_WARNING "%s: rx slot %d alloc failed\n",
		       cp->dev->name, rx_tail);

		cp->dropping_frag = 1;
drop_frag:
		if (frag_skb) {
			dev_kfree_skb_irq(frag_skb);
			cp->frag_skb = NULL;
		}
		if (last_frag) {
			cp->net_stats.rx_dropped++;
			cp->dropping_frag = 0;
		}
		return;
	}

	copy_skb->dev = cp->dev;
	skb_reserve(copy_skb, RX_OFFSET);
	skb_put(copy_skb, target_len);
	if (frag_skb) {
		memcpy(copy_skb->data, frag_skb->data, orig_len);
		dev_kfree_skb_irq(frag_skb);
	}
#if 0
	pci_dma_sync_single(cp->pdev, cp->rx_skb[rx_tail].mapping,
			    len, PCI_DMA_FROMDEVICE);
#endif
	memcpy(copy_skb->data + orig_len, skb->data, len);

	copy_skb->ip_summed = CHECKSUM_NONE;

	if (last_frag) {
		if (status & (RxError)) {
			re8670_rx_err_acct(cp, rx_tail, status, len);
			dev_kfree_skb_irq(copy_skb);
		} else
			re8670_rx_skb(cp, copy_skb, &cp->rx_ring[rx_tail]);
		cp->frag_skb = NULL;
	} else {
		cp->frag_skb = copy_skb;
	}
}
#endif

static 
#ifdef CONFIG_ETHWAN
__IRAM_NIC
#else
inline 
#endif //CONFIG_ETHWAN
unsigned int re8670_rx_csum_ok (u32 status)
{
	unsigned int protocol = (status >> 16) & 0x3;
	
	if (likely((protocol == RxProtoTCP) && (!(status & TCPFail))))
		return 1;
	else if ((protocol == RxProtoUDP) && (!(status & UDPFail)))
		return 1;
	else if ((protocol == RxProtoIP) && (!(status & IPFail)))
		return 1;
	return 0;
}

unsigned int eth_rx_count=0;
unsigned char eth_close=0;

// Kaohj -- for polling mode
int eth_poll; // poll mode flag
void eth_poll_schedule(void)
{
	//struct re_private *cp = eth_net_dev->priv;
	
	//tasklet_hi_schedule(&cp->rx_tasklets);
	tasklet_hi_schedule(eth_rx_tasklets);
}

#ifndef CONFIG_RTL867X_PACKET_PROCESSOR

#ifdef CONFIG_RTL8672_NAPI
static int re8670_rx (struct re_private *cp, int *budget)
#else
#ifdef CONFIG_ETHWAN
__IRAM_NIC
#endif //CONFIG_ETHWAN
static void re8670_rx (struct re_private *cp)
#endif //CONFIG_RTL8672_NAPI
{
	unsigned rx_tail = cp->rx_tail;
#ifdef CONFIG_RTL8672_NAPI
	unsigned rx_work = cp->dev->quota;
	unsigned rx,fail_alloc;
#else
#ifdef CONFIG_ETHWAN
	//unsigned rx_work = 300;	
	unsigned rx_work = RE8670_RX_RING_SIZE;
#else
	unsigned rx_work = 64;
#endif //CONFIG_ETHWAN
#endif //CONFIG_RTL8672_NAPI

	unsigned long flags;  

	spin_lock_irqsave(&cp->lock,flags);   
#ifdef CONFIG_RTL8672_NAPI
	napi_printk( "\n===>enter re8670_rx: budget=%d, dev->quota=%d", *budget, rx_work  );
rx_status_loop:
	rx = 0;
	{
		u16 isr_tmp;
		RTL_W16(ISR, (RX_OK | RX_ERR | RX_EMPTY | RX_FIFOOVR) );
		isr_tmp = RTL_R16(ISR);
		napi_printk( "\n===>re8670_rx: clear RX_ISR (new=0x%x)", isr_tmp );
	}
#endif //CONFIG_RTL8672_NAPI	

//scout('<');
	//protect eth rx while reboot
	if(eth_close == 1){
		spin_unlock_irqrestore (&cp->lock, flags);   
		return;
	}

	// Kaohj
	if (unlikely(eth_poll)) // in poll mode
		rx_work = 2; // rx rate is rx_work * 100 pps (timer tick is 10ms)

#ifdef CONFIG_RTL8672_NAPI
	while (1)
#else
	while (rx_work--)
#endif //CONFIG_RTL8672_NAPI
	{
		u32 status, len;
		dma_addr_t mapping;
		struct sk_buff *skb, *new_skb;
		DMA_DESC *desc;

#ifdef CONFIG_RTL8672_NAPI
		if(!rx_work)
		{
			napi_printk( "\n===>rx_work-- reaches 0, break" );
			break;
		}
		rx_work--;
		fail_alloc=0;
#endif //CONFIG_RTL8672_NAPI		

		unsigned buflen;
		//tylo, debug
//shlee 2.6 try		dma_cache_wback_inv(0,0);
		skb = cp->rx_skb[rx_tail].skb;
		if (!skb)
			BUG();

		desc = &cp->rx_ring[rx_tail];
		status = desc->opts1;		
		if (status & DescOwn)
			break;


//		printk("NIC Rx: status %x\n", status);	
		len = (status & 0x07ff) - 4;

		//mapping = cp->rx_skb[rx_tail].mapping;

		if ((status & (FirstFrag | LastFrag)) != (FirstFrag | LastFrag)) {
			//tylo
			printk("eth frag!!\n");
			re8670_rx_err_acct(cp, rx_tail, status, len);
			//re8670_rx_frag(cp, rx_tail, skb, status, len);
			goto rx_next;
		}

		//RxError-> RxErrFrame, RxErrCRC, RxErrRunt
        if (status & (RxErrFrame | RxErrCRC | RxErrRunt)) {
			printk("eth rxError (status %x)\n", status);	
			re8670_rx_err_acct(cp, rx_tail, status, len);
			goto rx_next;
		}

		/*if (status & (RxError)) {
			printk("eth rxError (status %x)\n", status);	//shlee 8672
			re8670_rx_err_acct(cp, rx_tail, status, len);
			goto rx_next;
		}*/


		///if (netif_msg_rx_status(cp))
		///	printk(KERN_DEBUG "%s: rx slot %d status 0x%x len %d\n",
		///	       cp->dev->name, rx_tail, status, len);

		buflen = cp->rx_buf_sz + RX_OFFSET;
		//new_skb = dev_alloc_skb (buflen);

#ifdef CONFIG_SKB_POOL_PREALLOC
#ifdef CONFIG_RTL867X_PACKET_PROCESSOR
   	      new_skb = prealloc_skb_get();
#else
#ifdef CONFIG_SEPARATE_ETH_SKB_POOL
		new_skb = get_free_skb(CONFIG_RTL867X_ETH_PREALLOCATE_SKB_SIZE);  
#else
		new_skb = get_free_skb();
#endif
#endif
#else
		new_skb = re8670_getAlloc(SKB_BUF_SIZE);
#endif
		if (!new_skb) {
#ifdef CONFIG_RTL8672_NAPI
			napi_printk( "\n===>re8670_getAlloc() alloc fails!" );
			#if 1
				fail_alloc=1;
				break;
			#else
				cp->net_stats.rx_dropped++;
				goto rx_next;
			#endif
#else
			//cp->net_stats.rx_dropped++;
			// not enough memory, rx deferred
			//queue_task (&cp->rx_task, &tq_immediate);
			//mark_bh (IMMEDIATE_BH);
			//8/25/05' hrchen, disable RX_OK to prevent IRQ again
			//re8670_disable_rx();
		//	printk("not enough memory, nic rx deferred\n");	//shlee 2.6
                        //In 100M utilization smartbit stress injection test, avoid nic stop receiving pkt, give it a second chance
			//tasklet_hi_schedule(&cp->rx_tasklets);	
			//break;
#ifdef CONFIG_ETHWAN
			cp->net_stats[SW_PORT_NUM].rx_dropped++;
#else
			cp->net_stats.rx_dropped++;
#endif //CONFIG_ETHWAN
			
			dma_cache_wback_inv((unsigned long)skb->data,buflen);
			goto rx_next;
#endif //CONFIG_RTL8672_NAPI
		}
		// Kaohj -- invalidate new_skb before NIC DMA
////tysu:		dma_cache_wback_inv((unsigned long)new_skb->data, len);
		//if ((u32)new_skb->data &0x3)
		//	printk(KERN_DEBUG "new_skb->data unaligment %8x\n",(u32)new_skb->data);

		/*skb_reserve(new_skb, RX_OFFSET);*/
		//new_skb->dev = cp->dev;
#if 0
		pci_unmap_single(cp->pdev, mapping,
				 buflen, PCI_DMA_FROMDEVICE);
#endif
		
		/* Handle checksum offloading for incoming packets. */
		if (re8670_rx_csum_ok(status))
			skb->ip_summed = CHECKSUM_UNNECESSARY;
		else
			skb->ip_summed = CHECKSUM_NONE;

#ifdef CONFIG_SKB_POOL_PREALLOC
		skb->data=skb->head+NET_SKB_PAD+RX_OFFSET;
		skb->tail=skb->data+len;
		skb->len=len;
		skb_reserve(new_skb,NET_SKB_PAD);
#else
		skb_reserve(skb, RX_OFFSET);
		skb_put(skb, len);
#endif



		mapping =
//		cp->rx_skb[rx_tail].mapping = (u32)new_skb->tail | UNCACHE_MASK;
		cp->rx_skb[rx_tail].mapping = (u32)new_skb->tail;

#if 1
		//pci_map_single(cp->pdev, new_skb->data,buflen, PCI_DMA_FROMDEVICE);
		//pci_map_single(cp->pdev, new_skb->tail,buflen, PCI_DMA_FROMDEVICE);
                dma_cache_wback_inv((unsigned long)new_skb->data,buflen);
#endif
		eth_rx_count++;
		

#if 0			//shlee 8672
		printk("Rx: ");
		int i;
		for (i=0; i<len ; i++){
			printk("0x%02x", *(skb->data+i));
			if(i!=len-1)	printk(", ");
			if(i%8==0&&i!=0)	printk("\n");
		}
		
		printk("\n");
#endif		//shlee 8672

//shlee 8672
/*
	if( (	RTL_R16(RxCDO) - rx_tail + RE8670_RX_RING_SIZE)%RE8670_RX_RING_SIZE > 60)
		printk("Not enough descriptor ..(available desc no %d)\n", RE8670_RX_RING_SIZE - (RTL_R16(RxCDO) - rx_tail + RE8670_RX_RING_SIZE)%RE8670_RX_RING_SIZE);	
*/		

//		RTL_W8(EthrntRxCPU_Des_Num, RE8670_RX_RING_SIZE - (RTL_R8(RxCDO) - rx_tail + RE8670_RX_RING_SIZE)%RE8670_RX_RING_SIZE);	//shlee 8672 test flowctrl
		//RTL_R8(EthrntRxCPU_Des_Num)=rx_tail;	//flowctrl mechanism: driver need to maintain this pointer

#if 0
#ifndef CONFIG_ETHWAN
		RTL_R8(EthrntRxCPU_Des_Num)= (rx_tail%256);  //flowctrl mechanism: driver need to maintain this pointer
        RTL_R8(EthrntRxCPU_Des_Num_h)=(((rx_tail/256)%16)<<4)|((TH_ON_VAL/256)%16);
#endif
#endif

		re8670_rx_skb(cp, skb, desc);

		cp->rx_skb[rx_tail].skb = new_skb;
		cp->rx_ring[rx_tail].addr = mapping;
#ifdef CONFIG_RTL8672_NAPI
		rx++;
#endif //CONFIG_RTL8672_NAPI
rx_next:
		///cp->rx_ring[rx_tail].opts2 = 0;
		desc->opts1 = (DescOwn | cp->rx_buf_sz) | 
		    ((rx_tail == (RE8670_RX_RING_SIZE - 1))?RingEnd:0);
		//#ifdef CONFIG_ETHWAN
		{
			unsigned int new_cpu_desc_num;
			//RTL_R8(EthrntRxCPU_Des_Num)= rx_tail&0xFF;
			//RTL_R8(EthrntRxCPU_Des_Num_h)=(((rx_tail>>8)&0xF)<<4);
			//RTL_R8(EthrntRxCPU_Des_Num_h)=(((rx_tail>>8)&0xF)<<4|((TH_ON_VAL>>8)&0xF));
			new_cpu_desc_num = RTL_R32(EthrntRxCPU_Des_Num);
			//printk("%08x\n", new_cpu_desc_num);
			new_cpu_desc_num &= 0x00FFFF0F;//clear
			new_cpu_desc_num |= (((rx_tail&0xFF)<<24)|(((rx_tail>>8)&0xF)<<4));//update
			//printk("%08x\n", new_cpu_desc_num);
			RTL_R32(EthrntRxCPU_Des_Num) = new_cpu_desc_num;
		}
		//#endif
		rx_tail = NEXT_RX(rx_tail);
	}

//static int xxx = 0;
//if ( ((xxx++)%1000)==0) printk("re %d\n", 100-rx_work );
	//if (!rx_work) {
		//printk(KERN_WARNING "%s: rx work limit reached\n", cp->dev->name);
	//    tasklet_hi_schedule(&cp->rx_tasklets);
    //};

//scout('>');
	cp->rx_tail = rx_tail;
#ifdef CONFIG_RTL8672_NAPI
	cp->dev->quota -= rx;
	*budget -= rx;
	napi_printk( "\n===>re8670_rx: new quota=%d, new budget=%d, rx_work=%d", cp->dev->quota, *budget, rx_work );

	/* if we did not reach work limit, then we're done with
	 * this round of polling
	 */
	if (fail_alloc==0 && rx_work) {
		if( RTL_R16(ISR) & (RX_OK | RX_ERR | RX_EMPTY | RX_FIFOOVR) )
			goto rx_status_loop;

		local_irq_disable();
		{
			u16 imr_tmp;
			imr_tmp=RTL_R16(IMR);
			imr_tmp = imr_tmp | (RX_OK | RX_ERR | RX_EMPTY | RX_FIFOOVR);
			RTL_W16(IMR, imr_tmp);
			imr_tmp = RTL_R16(IMR);
			napi_printk( "\n===>re8670_rx return 0(done), enable RX_IMR=0x%x", imr_tmp );
		}
		__netif_rx_complete(cp->dev);
		local_irq_enable();
		return 0;	/* done */
	}

	napi_printk( "\n===>re8670_rx return 1(not done)" );
	return 1;		/* not done */
#endif //CONFIG_RTL8672_NAPI


#ifdef CONFIG_RTL8672	
	//7/5/05' hrchen, NIC_IRQ will be disabled if RDU happen. Open NIC_IRQ here.
//__sti();



	//tylo, fpga
	aaa=1;
	// Enable RX interrupt
	RTL_R16(IMR)|=(u16)(RX_OK | RX_ERR | RX_EMPTY | RX_FIFOOVR);
	
	spin_unlock_irqrestore (&cp->lock, flags); 
//	REG32(GIMR)|=NIC100_IE;	//shlee 2.6
	//enable_irq(cp->dev->irq);	//shlee 2.6

	//enable_lx4180_irq(cp->dev->irq);
#endif

}
#endif // of CONFIG_RTL867X_PACKET_PROCESSOR

#ifdef CONFIG_RTL8672
#ifdef CONFIG_ETHWAN
__IRAM_NIC
#endif //CONFIG_ETHWAN
static irqreturn_t re8670_interrupt(int irq, void * dev_instance, struct pt_regs *regs)
#else
void re8670_interrupt (int irq, void *dev_instance, struct pt_regs *regs)
#endif
{
	struct net_device *dev = dev_instance;
	struct re_private *cp = dev->priv;
	u16 status;
//alex	





       
//scout('4');	
	if (!(status = RTL_R16(ISR)))  
#ifdef CONFIG_RTL8672
	{
		//printk("%s: no status indicated in interrupt, weird!\n", __func__);	//shlee 2.6
		return IRQ_RETVAL(IRQ_NONE);
	}
#else
		return;
#endif
	//if (netif_msg_intr(cp))
	//	printk(KERN_DEBUG "%s: intr, status %04x cmd %02x \n",
	//	        dev->name, status, RTL_R8(CMD));

	//spin_lock(&cp->lock);

	//if (status & (TX_OK | TX_ERR | TX_EMPTY | SW_INT)) {
	/*if (status & TX_EMPTY) {
		// JONAH
		//mark_bh(NIC_TX_BH);
		//queue_task (&cp->tx_task, &tq_immediate);
		//mark_bh (IMMEDIATE_BH);
		//tasklet_schedule(&cp->tx_tasklets);
//scout('T');	
		printk("NIC TDU\n");
		if (!atomic_read(&lock_tx_tail)) {
			re8670_tx(cp);
		}
	}
          */

#ifdef PHY_MDI_MDIX_AUTO
	if(status & LINK_CHG){
		eth_link_stable_times=0;
		if(!timer_pending(&eth_phy_check_timer)){
			mod_timer(&eth_phy_check_timer, jiffies + 1*HZ);
		}
	}
#endif

	if (status & (RX_OK | RX_ERR | RX_EMPTY | RX_FIFOOVR)) {

#ifdef CONFIG_RTL8672_NAPI
		//time to disable the rx interrupte	
		if (netif_rx_schedule_prep(dev)) 
		{
			u16 imr_tmp = RTL_R16(IMR);			
			imr_tmp = imr_tmp & (u16)(~(RX_OK | RX_ERR | RX_EMPTY | RX_FIFOOVR));
			RTL_W16( IMR, imr_tmp );
			imr_tmp = RTL_R16(IMR);			
			napi_printk( "\n===>new IMR: 0x%x", imr_tmp );
			__netif_rx_schedule(dev);
		}
#else		
			
		// JONAH
		//tylo, fpga
		aaa=0;

                /*
                 * Check if RDU happens?
                 * RDU(RX_EMPTY) will cause infinited loop (system hang) in Linux 2.6 kernel IRQ handler
                 * IRQ dispatch->NIC int handler->timer handler->NIC->timer ...
                 * NIC bottom half handler will never be scheduled because RDU state is always up.
                 *
                 * Disable RDU in NIC interrupt and then enable it in NIC bottom half handler later.
                 */
		// Disable interrupts before running bottom-half
		RTL_R16(IMR)&=(u16)(~(RX_OK | RX_ERR | RX_EMPTY | RX_FIFOOVR));
//scout('R');	  
		tasklet_hi_schedule(&cp->rx_tasklets);
		
		//7/5/05' hrchen, clear NIC ISR of 8671 will not clear RDU if no rx desc available.
		//The IRQ will never stop and RX_BH will never run and free rx desc.
		//Clear NIC GIMR here if RDU. nic_rx must reopen NIC_IM.
		//if (status&RX_EMPTY) {  //RDU happened, clear NIC_IM
		//};
#endif //CONFIG_RTL8672_NAPI
	}

#ifdef CONFIG_RTL8672_NAPI
	//clear norx 
	status = status & (u16)(~( RX_OK | RX_ERR | RX_EMPTY | RX_FIFOOVR ));
	RTL_W16(ISR,status);
	status = RTL_R16(ISR);
	napi_printk( "\n===>new ISR: 0x%x", status );
#else
	RTL_W16(ISR,status);
#endif //CONFIG_RTL8672_NAPI


#ifdef CONFIG_RTL8672
	return IRQ_RETVAL(IRQ_HANDLED);
#endif
	//spin_unlock(&cp->lock);
}

/*#ifdef CONFIG_RTL8192CD
#define MAX_SKB_NUM 256
#endif*/
__IRAM_NIC void re8670_tx (struct re_private *cp)
{
	//unsigned tx_head = cp->tx_hqhead;
	unsigned tx_tail = cp->tx_hqtail;
	u32 status;
	struct sk_buff *skb;
	if(eth_close)
		return;

	//while (tx_tail != tx_head) {
	while (!((status = (cp->tx_hqring[tx_tail].opts1))& DescOwn)) {
		//cp->tx_hqring[tx_tail].addr =0xaa550000;
		//cp->tx_hqring[tx_tail].opts1 =0x0;
#ifdef CONFIG_ETHWAN
		if (tx_tail == cp->tx_hqhead)
			break;
#endif //CONFIG_ETHWAN		

	    skb = cp->tx_skb[tx_tail].skb;
		if (!skb)
		    break;
			//BUG();
		
		rmb();
		//status = (cp->tx_hqring[tx_tail].opts1);
		//if (status & DescOwn)
		//	break;

#ifdef CONFIG_ETHWAN
#ifdef CONFIG_EXT_SWITCH
{
		int port = dev2port(skb->dev);


		/*if (status & TxError)
		{
			cp->net_stats[port].tx_errors++;
			if (port < SW_PORT_NUM-1)
				cp->net_stats[SW_PORT_NUM].tx_errors++;
			if (status & TxFIFOUnder)
				cp->net_stats[port].tx_fifo_errors++;
		}*/

		cp->net_stats[port].tx_packets++;
		cp->net_stats[port].tx_bytes += skb->len;
		
		if (port < SW_PORT_NUM-1) {
			cp->net_stats[SW_PORT_NUM].tx_packets++;
			cp->net_stats[SW_PORT_NUM].tx_bytes += skb->len;			
		}
}
#endif //CONFIG_EXT_SWITCH
#else

		/*if (status & TxError)
		{
			cp->net_stats.tx_errors++;
			if (status & TxFIFOUnder)
				cp->net_stats.tx_fifo_errors++;
		}*/

		cp->net_stats.tx_packets++;
		cp->net_stats.tx_bytes += skb->len;
#endif //CONFIG_ETHWAN		
		//if (netif_msg_tx_done(cp))
		//	printk(KERN_DEBUG "%s: tx done, slot %d\n", cp->dev->name, tx_tail);

#if 0
		if(skb->destructor==0)
			{
			//dev_kfree_skb_any(skb);
			//dev_kfree_skb(skb);
#if 1
			if(skb->fastbr)
			    dev_kfree_skb(skb);
			else 
			    dev_kfree_skb_any(skb);	
#endif
		}
		else
			dev_kfree_skb_irq(skb);		
#endif


#ifdef CONFIG_ETHWAN
#ifdef CONFIG_RTL8672_ETHSKB_CONTROL_POOL
		//if(net_smallpkt_heavytraffic)
		//{
			if(!free_sk_buffer_to_rx_pool(skb))
			{
#if !defined(PKT_PROCESSOR) && !defined(PRE_ALLOCATE_SKB) && (defined(CONFIG_RTL8192CD) || defined(CONFIG_RTL8192CD_MODULE))
				dev_kfree_skb_any(skb);
#else
				dev_kfree_skb(skb);
#endif
			}	
		//}
		//else 
		//	dev_kfree_skb_any(skb);
#else 	
		dev_kfree_skb(skb);	
#endif //CONFIG_RTL8672_ETHSKB_CONTROL_POOL
#else//CONFIG_ETHWAN
//printk("RE free skb=%08x skb->data=%08x skb->end=%08x skb_shinfo(skb)->gso_size=%d skb->fcpu=%d<%s %d>\n",skb,skb->data,skb->end,skb_shinfo(skb)->gso_size,skb->fcpu,__FUNCTION__,__LINE__);
#if defined(CONFIG_SKB_POOL_PREALLOC) && defined(CONFIG_RTL867X_PACKET_PROCESSOR)
		if(skb->fcpu == 0){
			prealloc_skb_free(skb);}
		else
			dev_kfree_skb(skb);				
#elif defined(CONFIG_SKB_POOL_PREALLOC) && defined(CONFIG_USB_RTL8192SU_SOFTAP)
		if(skb->fcpu == 0){
			insert_skb_pool(skb);}
		else
			dev_kfree_skb(skb);			
#else			
		//dev_kfree_skb(skb);	
        dev_kfree_skb_any(skb); 
#endif
#endif //CONFIG_ETHWAN
		cp->tx_skb[tx_tail].skb = NULL;
		tx_tail = NEXT_TX(tx_tail);
	}

	cp->tx_hqtail = tx_tail;

#if defined (CONFIG_RTL8192CD) || defined (CONFIG_RTL8192CD_MODULE) ||defined(CONFIG_RTL8681_PTM)
	if(TX_HQBUFFS_AVAIL(cp)<(RE8670_TX_RING_SIZE-24))
		tasklet_schedule(&eth_checktxdesc_tasklets);
#endif
#ifndef CONFIG_ETHWAN
	if (netif_queue_stopped(cp->dev) && (TX_HQBUFFS_AVAIL(cp) > (MAX_SKB_FRAGS + 1)))
		netif_wake_queue(cp->dev);
#endif //CONFIG_ETHWAN	
}

#ifdef CONFIG_ETHWAN
__IRAM_NIC
#endif //CONFIG_ETHWAN
void checkTXDesc(void){
	struct re_private *cp = eth_net_dev->priv;
	if (!atomic_read(&lock_tx_tail)) {
		atomic_set(&lock_tx_tail, 1);
		re8670_tx(cp);
		atomic_set(&lock_tx_tail, 0);
	}
}

void re8670_mFrag_xmit (struct sk_buff *skb, struct re_private *cp, unsigned *entry
#if 0
#ifdef CONFIG_EXT_SWITCH
	, u32 txOpts2
#endif
#endif
)
{
	u32 eor;
		DMA_DESC *txd;
		u32 first_len, first_mapping;
		int frag, first_entry = *entry;
		u32 firstFragAddr;

		/* We must give this initial chunk to the device last.
		 * Otherwise we could race with the device.
		 */
		first_len = skb->len - skb->data_len;
		/*first_mapping = pci_map_single(cp->pdev, skb->data,*
					       first_len, PCI_DMA_TODEddVICE);*/
		firstFragAddr = (u32)skb->data;
		first_mapping = (u32)skb->data|UNCACHE_MASK;
		cp->tx_skb[*entry].skb = skb;
		cp->tx_skb[*entry].mapping = first_mapping;
		cp->tx_skb[*entry].frag = 1;
		*entry = NEXT_TX(*entry);

		for (frag = 0; frag < skb_shinfo(skb)->nr_frags; frag++) {
			skb_frag_t *this_frag = &skb_shinfo(skb)->frags[frag];
			u32 len, mapping;
			u32 ctrl;

			len = this_frag->size;
			/*mapping = pci_map_single(cp->pdev,
						 ((void *) page_address(this_frag->page) +
						  this_frag->page_offset),
						 len, PCI_DMA_TODEVICE);*/
			mapping = (u32)this_frag->page_offset|UNCACHE_MASK;

			eor = (*entry == (RE8670_TX_RING_SIZE - 1)) ? RingEnd : 0;
			ctrl = eor | len | DescOwn | TxCRC;
			if (frag == skb_shinfo(skb)->nr_frags - 1)
				ctrl |= LastFrag;

			txd = &cp->tx_hqring[*entry];
#ifdef CONFIG_ETHWAN
			CP_VLAN_TX_TAG(txd, 0);
#else
			CP_VLAN_TX_TAG(txd, vlan_tag);			
#endif //CONFIG_ETHWAN

#if 0
#ifdef CONFIG_EXT_SWITCH
			if (txOpts2) {
				// add vlan tag
				txd->opts2 = txOpts2;
			}
#endif
#endif
			txd->addr = (mapping);
			wmb();

			// Kaohj
			dma_cache_wback_inv((unsigned long)this_frag->page_offset, len);
			
			txd->opts1 = (ctrl);
			wmb();
			cp->tx_skb[*entry].skb = skb;
			cp->tx_skb[*entry].mapping = mapping;
			cp->tx_skb[*entry].frag = frag + 2;
			*entry = NEXT_TX(*entry);
		}

		txd = &cp->tx_hqring[first_entry];
#ifdef CONFIG_ETHWAN
			CP_VLAN_TX_TAG(txd, 0);
#else
			CP_VLAN_TX_TAG(txd, vlan_tag);			
#endif //CONFIG_ETHWAN
		
		txd->addr = (first_mapping);
		
		wmb();
		// Kaohj
		dma_cache_wback_inv((unsigned long)firstFragAddr, first_len);
		
		eor = (first_entry == (RE8670_TX_RING_SIZE - 1)) ? RingEnd : 0;
		txd->opts1 = (first_len | FirstFrag | DescOwn|TxCRC|eor);
		
		wmb();
}

#ifdef CONFIG_PORT_MIRROR
void nic_tx_mirror(struct sk_buff *skb)
{
	re8670_start_xmit(skb, eth_net_dev);
}
#endif

// Kaohj
/* Packets prepared to be sent to external switch.
 * Return
 * 0: Drop
 * 1: Continue
 */
int re8670_xmit_sw(struct sk_buff *skb, unsigned int *vid, unsigned int port)
{
        int should_deliver=2;
#ifdef CONFIG_ETHWAN
	 int is_lan = (port < ETH_WAN_PORT)?1:0;
#else
	 int is_lan = 1;
#endif
        
	*vid = 0;
	#ifdef CONFIG_EXT_SWITCH
	if(enable_IGMP_SNP && is_lan){
		//handle IGMP Query packets
		handle_IGMP_query(skb);
		if(check_IGMP_report(skb)==1) {//drop IGMP report from WLAN
			return 0;
		}		
	}
	
	// Mason Yu. MLD snooping
	#ifdef CONFIG_IPV6	
	if(enable_MLD_SNP && is_lan){			
		//handle MLD Query packets
		handle_MLD_query(skb);		
		if (check_MLD_report(skb) == 1) { //drop MLD report from WLAN
			return 0;
		}	
	}
	#endif	
	#endif
	// Kaohj --- add vlan tag
	#if 0 // not necessary for bridged traffic; dev.c will deal with vlan tag
	if((skb->vlan_passthrough & 0x81000000) == 0x81000000){ //vlan passthrough
			// set vlan id
			*vid = skb->vlan_passthrough & 0x0fff;
	}
	#else
	if (enable_vlan_grouping) { // vlan tx
		if (*((unsigned short *)&(skb->data[12])) != 0x8100 && skb->pvid) {
			// set vlan id
			*vid = rtl8305_info.vlan[skb->pvid].vid & 0x0fff;
		}
	}
	#endif
	
	// Port-Mapping
	if(enable_port_mapping){
		if (skb->vlan_member!=0) {
			int k;
			should_deliver = 0;
			#if defined( CONFIG_EXT_SWITCH) && defined (CONFIG_ETHWAN)
			if(port < SW_PORT_NUM && skb->vlan_member == rtl8305_info.vlan[port].member){//don't let eth0 pass
				should_deliver = 1;
			}
			else{
				ETHDBG_PRINT("drop from port %d, [%p]skb->vlan_member %x, rtl8305_info.vlanmember %x\n", port, skb, \
						skb->vlan_member, \
						rtl8305_info.vlan[port].member);
			}
			#else
			for (k=0; k<SW_PORT_NUM; k++) {
				//printk("%d: mbr=%x\n", k, rtl8305_info.vlan[k].member);
				if (skb->vlan_member == rtl8305_info.vlan[k].member) {
					*vid = rtl8305_info.vlan[k].vid & 0x0fff;
					//printk("k=%d, vid=%d\n", k, rtl8305_info.vlan[k].vid);
					should_deliver = 1;
					break;
				}
			}
			#endif
			if (!should_deliver) {
				return 0;
			}
				
		}
	}
	
	#ifdef CONFIG_EXT_SWITCH
	#ifndef CONFIG_IPV6
	if(enable_IGMP_SNP && is_lan){
	#else
	if((enable_IGMP_SNP && is_lan) || (enable_MLD_SNP && is_lan)){
	#endif
		unsigned int ret=3, id;
		if(enable_IGMP_SNP)
			ret = IGMP_snoop_tx(skb, &id);
		// Mason Yu. MLD snooping.	
		#ifdef CONFIG_IPV6
		if(enable_MLD_SNP && (ret == 3))
			ret = MLD_snoop_tx(skb, &id);
		#endif		
		
		if (ret == 1){		
			#ifdef CONFIG_ETHWAN
			/* 2011-1-13 krammer :  because ethwan has already seperate the switch port, 
			and will send to all lan interface. There is no need to set vid or cputag when we 
			use ethwan. We just need to find out which mcast packet we should drop!*/
			if(id){
				if((id & (1<<(virt2phy[port]))) == 0){
					ETHDBG_PRINT("%s %d, id %x, port %d\n", __func__, __LINE__, id, port);
					return 0;
				}
			}
			#else
			#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
			*vid = id;
			#else // IGMP_CPU_TAG
			insert_cputag(&skb,(unsigned char )(id&0x1f));	
			#endif
			#endif
		}
		else if (ret == 0 && DropUnknownMulticast)
			return 0;
	}
	#endif
	return 1;
}
//__IRAM int re8670_start_xmit (struct sk_buff *skb, struct net_device *dev)
__IRAM_NIC int re8670_start_xmit (struct sk_buff *skb, struct net_device *dev)	//shlee temp, fix this later

{
	struct re_private *cp = dev->priv;
	unsigned entry;
	u32 eor;
	unsigned long flags;
	unsigned int port;
#ifdef CONFIG_ETHWAN
	int dir=DIR_LAN;
#ifdef CONFIG_EXT_SWITCH	
	int vlaninsert=0;
#ifdef CONFIG_RE8306_USE_CPUTAG
	int mask;
#endif //CONFIG_RE8306_USE_CPUTAG
#endif //CONFIG_EXT_SWITCH
#endif //CONFIG_ETHWAN
	
#if 0
#ifdef CONFIG_EXT_SWITCH
	u32 txOpts2;
#endif
#endif
	//protect eth tx while reboot
#ifdef FAST_ROUTE_Test
#if 0
	// Suppose it is LLCHdr802_3, we remove it before trasmitting.
	if(skb->pkt_type == PACKET_FASTROUTE && skb->data[0]==0xaa){
		skb_pull(skb, 10);
	}
#endif
#endif	

#ifdef CONFIG_ETHWAN
	ETHDBG_PRINT( "tx %s\n", dev->name );
	SKB_DBG(skb, debug_enable, 1);

	//sync the dev, re8670_tx will use skb->dev to count tx packets, is dev always right?
	if( unlikely(skb->dev!=dev) )
	{
		//printk( "\n TX ERROR!! skb->dev(%s)!=dev(%s), 0x%08x, 0x%08x\n", skb->dev->name, dev->name, skb->dev, dev );
		skb->dev=dev;
	}
#endif //CONFIG_ETHWAN

#if (defined(CONFIG_USB_RTL8192SU_SOFTAP) || defined(CONFIG_USB_RTL8187SU_SOFTAP)) && !defined(CONFIG_SKB_POOL_PREALLOC) && !defined(CONFIG_RTL867X_PACKET_PROCESSOR)
	if((skb->src_port == IF_WLAN)) {
		skb->src_port |= IF_ETH;
		atomic_inc(&txskb_from_wifi);
	}
#endif
	//atomic_set(&lock_tx_tail, 1);
	//re8670_tx(cp);
	//atomic_set(&lock_tx_tail, 0);
	checkTXDesc();

//	printk("skb->data = %p\n", skb->data);	//shlee temp test
	if(eth_close == 1) {
		dev_kfree_skb_any(skb);
#ifdef CONFIG_ETHWAN
	cp->net_stats[SW_PORT_NUM].tx_dropped++;
#else
	cp->net_stats.tx_dropped++;
#endif //CONFIG_ETHWAN		
		return 0;
	}

#ifdef CONFIG_ETHWAN
#ifdef CONFIG_EXT_SWITCH
	port = dev2port(dev);

	ETHDBG_PRINT("port is %d\n", port);		
#endif //CONFIG_EXT_SWITCH
#endif //CONFIG_ETHWAN

	cp->cp_stats.tx_cnt++;
	
#if CP_VLAN_TAG_USED
	u32 vlan_tag = 0;
#endif

	spin_lock_irqsave(&cp->lock, flags);

	/* This is a hard error, log it. */
	#ifdef FAST_ROUTE_Test
#if 0	
	if (skb->pkt_type == PACKET_FASTROUTE)
		re8670_tx(cp);
#endif
	#endif

#ifdef CONFIG_ETHWAN
	if ((port<SW_PORT_NUM && !vnet_dev_status[port]) || (TX_HQBUFFS_AVAIL(cp) <= (skb_shinfo(skb)->nr_frags + 1)))
	{
		//eason,disable netif_stop_queue(dev);
		spin_unlock_irqrestore(&cp->lock, flags);
//shlee		printk(KERN_ERR PFX "%s: BUG! Tx Ring full when queue awake!\n", dev->name);
		//printk("%s: BUG! Tx Ring full when queue awake!\n", __func__, dev->name);	//shlee 8672
		//re8670_tx(cp);	//shlee 8672 just test		
		dev_kfree_skb_any(skb);
		//jiunming, port may be wrong for some case
		//cp->net_stats[port].tx_dropped++;
		cp->net_stats[port].tx_dropped++;
		return 0;
	}
#else
	if (TX_HQBUFFS_AVAIL(cp) <= 1) {
		netif_stop_queue(dev);
		spin_unlock_irqrestore(&cp->lock, flags);
//shlee		printk(KERN_ERR PFX "%s: BUG! Tx Ring full when queue awake!\n", dev->name);
		//printk("%s: BUG! Tx Ring full when queue awake!\n", __func__, dev->name);	//shlee 8672
		//re8670_tx(cp);	//shlee 8672 just test		
		dev_kfree_skb(skb);
		cp->net_stats.tx_dropped++;
		return 0;
	}
#endif //CONFIG_ETHWAN

#if CP_VLAN_TAG_USED
	if (cp->vlgrp && vlan_tx_tag_present(skb)){
		vlan_tag = TxVlanTag | vlan_tx_tag_get(skb);
	}
#endif
	if ( (skb->len > VLAN_ETH_FRAME_LEN) ) // with vlan tag
	{
		printk("error tx len = %d \n",skb->len);
		spin_unlock_irqrestore(&cp->lock, flags);
		dev_kfree_skb(skb);
#ifdef CONFIG_ETHWAN
		cp->net_stats[port].tx_dropped++;
#else
		cp->net_stats.tx_dropped++;
#endif //CONFIG_ETHWAN
		return 0;
	}	

#if 0
#ifdef CONFIG_EXT_SWITCH
	txOpts2 = *(unsigned long *)&skb->cb[40];
	if (!(txOpts2 & 0x10000) || !dev->vlan)
		txOpts2 = 0;
#endif
#endif


#ifdef CONFIG_ETHWAN
//		if(skb_cloned(skb))
//		{
//			struct sk_buff *newskb;
//			newskb = skb_copy(skb, GFP_ATOMIC);
//			dev_kfree_skb_any(skb);
//			skb=newskb;
//		}
	SKB_DBG(skb, debug_enable, 1);
#endif //CONFIG_ETHWAN

	entry = cp->tx_hqhead;
	eor = (entry == (RE8670_TX_RING_SIZE - 1)) ? RingEnd : 0;
	if (skb_shinfo(skb)->nr_frags == 0) {
		DMA_DESC  *txd = &cp->tx_hqring[entry];
		u32 mapping, len;
		unsigned int vlan_id = 0;

#ifndef CONFIG_ETHWAN
		//eor = (entry == (RE8670_TX_RING_SIZE - 1)) ? RingEnd : 0;
		CP_VLAN_TX_TAG(txd, vlan_tag);
#if 0
#ifdef CONFIG_EXT_SWITCH
		if (txOpts2) {
			// add vlan tag
			txd->opts2 = txOpts2;
		}else
#endif
#endif
#endif//CONFIG_ETHWAN
		if (!re8670_xmit_sw(skb, &vlan_id, port)) {
			spin_unlock_irqrestore(&cp->lock, flags);
			dev_kfree_skb_any(skb);
#ifndef CONFIG_ETHWAN
			cp->net_stats.tx_dropped++;
#endif
			return 0;
		}
		else {
			if (vlan_id) {
				u32 tx_opts;
				// set vlan id
				tx_opts = VTAG2DESC(vlan_id);
				tx_opts |= 0x10000;
				txd->opts2 = tx_opts;
			}
		}
#ifdef CONFIG_ETHWAN
		//if(debug_enable) skb_debug(skb, debug_enable, 1);
#ifdef CONFIG_RTL8672_BRIDGE_FASTPATH
#if 0 //Kevin
		if (port == ETH_WAN_PORT)
		{
			dir = DIR_WAN;
#ifdef CONFIG_RTL8672_ETHSKB_CONTROL_POOL
			if(!((net_smallpkt_heavytraffic)&&(UPSTRAM_HEAVY_TRAFFIC)))
#endif //CONFIG_RTL8672_ETHSKB_CONTROL_POOL
			{
				brgShortcutLearnDestItf(skb, dev, dir);
			}
		}	
		else if(port < ETH_WAN_PORT)
		{
			dir = DIR_LAN;
#ifdef CONFIG_RTL8672_ETHSKB_CONTROL_POOL
			if(!((net_smallpkt_heavytraffic)&&(DOWNSTRAM_HEAVY_TRAFFIC)))
#endif //CONFIG_RTL8672_ETHSKB_CONTROL_POOL
			{
				brgShortcutLearnDestItf(skb, dev, dir);
			}
		}	
#endif //if 0
#endif //CONFIG_RTL8672_BRIDGE_FASTPATH
	
		if(*((unsigned short*)&skb->data[12]) == 0x8899){//has cpu tag, reset txd's vlan id
			CP_VLAN_TX_TAG(txd, 0);
		}
		else if(!vlan_id){//no cpu tag and no vlan id, add normal vlan id
				if(port == ETH_WAN_PORT) {
					/*WAN interface */ 
					CP_VLAN_TX_TAG(txd, VTAG2DESC(WAN_PORT_VLAN_ID) | 0x10000);
					ETHDBG_PRINT("Attach vid: %d for wan port\n", WAN_PORT_VLAN_ID);
				} else {
					/* LAN interface */
						/* Attach vlan-tag for sending uni-cast traffic to LAN port */
						if(port == 0)
							vlaninsert = LAN_PORT_VLAN_ID_1;
						else if(port ==1)
							vlaninsert = LAN_PORT_VLAN_ID_2;
						else if(port ==2)
							vlaninsert = LAN_PORT_VLAN_ID_3;
						else if(port == SW_PORT_NUM)
							vlaninsert = ALL_LAN_PORT_VLAN_ID;
						else
							printk("error port \n");
							
						CP_VLAN_TX_TAG(txd, VTAG2DESC(vlaninsert) | 0x10000);	
						ETHDBG_PRINT("2 Attach vid: %d for lan port\n", vlaninsert);	
				}
		}
#endif
		len = ETH_ZLEN < skb->len ? skb->len : ETH_ZLEN;		
// Mason Yu. combine_1p_4p_PortMapping
#ifndef CONFIG_EXT_SWITCH
		if(enable_port_mapping){
			if((skb->vlan_member !=0) && (skb->vlan_member != eth_dev_member)){
				//printk("eth tx drop!! skb->vlan_member=%x   eth_dev_member=%x\n",skb->vlan_member,eth_dev_member);
				spin_unlock_irqrestore(&cp->lock, flags);
				dev_kfree_skb(skb);
				return 0;
			}
		}
#endif
		
		mapping = (u32)skb->data|UNCACHE_MASK;
		// Kaohj --- invalidate DCache before NIC DMA
		dma_cache_wback_inv((unsigned long)skb->data, len);
		cp->tx_skb[entry].skb = skb;
		cp->tx_skb[entry].mapping = mapping;
		cp->tx_skb[entry].frag = 0;

		txd->addr = (mapping);
        	txd->opts1 = (eor | len | DescOwn | FirstFrag |
			LastFrag | TxCRC 
#if 0
            /*| IPCS | TCPCS | UDPCS*/ 
#endif
        );

        //txd->opts2 |= 0x00016400;    //vlan tag function
		// Kaohj --- not necessary for txd is uncachable
		//dma_cache_wback_inv(txd, 16);
		// Kaohj --- seems better to have, why ?
		dma_cache_wback_inv((unsigned long)txd, 16);
		wmb();
		
		entry = NEXT_TX(entry);
	} else {
        re8670_mFrag_xmit(skb, cp, &entry
#if 0
#ifdef CONFIG_EXT_SWITCH
        , txOpts2
#endif
#endif
        );

	}
	cp->tx_hqhead = entry;
	if (netif_msg_tx_queued(cp))
		printk(KERN_DEBUG "%s: tx queued, slot %d, skblen %d\n",
		       dev->name, entry, skb->len);
#ifndef CONFIG_ETHWAN
	if (TX_HQBUFFS_AVAIL(cp) <= 1)
		netif_stop_queue(dev);
#endif //CONFIG_ETHWAN

	spin_unlock_irqrestore(&cp->lock, flags);
	RTL_W32(IO_CMD,iocmd_reg | TX_POLL);
	dev->trans_start = jiffies;
//scout('}');		

	return 0;
}

/* Set or clear the multicast filter for this adaptor.
   This routine is not state sensitive and need not be SMP locked. */

static void __re8670_set_rx_mode (struct net_device *dev)
{
	/*struct re_private *cp = dev->priv;*/
	u32 mc_filter[2];	/* Multicast hash filter */
	int i, rx_mode;
	/*u32 tmp;*/


	// Kao
	//printk("%s: %s %d Still Unimplemented !!!!!!!\n",__FILE__,__FUNCTION__,__LINE__);
	//return ;
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
		#ifdef CONFIG_ETHWAN
		rx_mode = AcceptBroadcast | AcceptMulticast | AcceptMyPhys | AcceptAllPhys;
		#else
		rx_mode = AcceptBroadcast | AcceptMulticast | AcceptMyPhys;
		#endif
		mc_filter[1] = mc_filter[0] = 0xffffffff;
	} else {
		struct dev_mc_list *mclist;
		#ifdef CONFIG_ETHWAN
		rx_mode = AcceptBroadcast | AcceptMyPhys | AcceptAllPhys; //there could be two bridge interfaces have different mac address
		#else
		rx_mode = AcceptBroadcast | AcceptMyPhys;
		#endif //CONFIG_ETHWAN
		mc_filter[1] = mc_filter[0] = 0;
		for (i = 0, mclist = dev->mc_list; mclist && i < dev->mc_count;
		     i++, mclist = mclist->next) {
			int bit_nr = ether_crc(ETH_ALEN, mclist->dmi_addr) >> 26;

			mc_filter[bit_nr >> 5] |= 1 << (bit_nr & 31);
			rx_mode |= AcceptMulticast;
		}
	}

	/* We can safely update without stopping the chip. */
	// Kao, 2004/01/07
#if 0
	RTL_W32(MAR0, mc_filter[0]);
	RTL_W32(MAR4, mc_filter[1]);
#else
	RTL_W32(MAR0, 0xFFFFFFFF);
	RTL_W32(MAR4, 0xFFFFFFFF);
#endif
	RTL_W32(RCR, rx_mode);
}

static void re8670_set_rx_mode (struct net_device *dev)
{
	unsigned long flags;
	struct re_private *cp = dev->priv;
	// Kaohj
#ifdef CONFIG_EXT_SWITCH
	if (dev->base_addr == 0) {
		// not the root device
		return;
	}
#endif

	spin_lock_irqsave (&cp->lock, flags);
	__re8670_set_rx_mode(dev);
	spin_unlock_irqrestore (&cp->lock, flags);
}

// Kao, 2004/01/07
static int (*my_eth_mac_addr)(struct net_device *, void *);
static int re8670_set_mac_addr(struct net_device *dev, void *addr)
{
	u32 *hwaddr;
	int err;
#ifdef CONFIG_EXT_SWITCH
	int i;
	
	if (dev->base_addr == 0) {
		// not the root device
		printk("re_setmac, not root\n");
		return 0;
	}
	
#ifdef CONFIG_ETHWAN
	if( (dev->name[0] == 'e')&&(dev->name[4] != '.') ) //only eth0
	{
		for (i = 0; i < SW_PORT_NUM; i++)
			my_eth_mac_addr(dev_sw[i], addr);
	}
#else
	for (i = 0; i < SW_PORT_NUM; i++)
		my_eth_mac_addr(dev_sw[i], addr);
#endif //CONFIG_ETHWAN
#endif
	err = my_eth_mac_addr(dev, addr);
	if (!err)
	{
#ifdef CONFIG_ETHWAN
		if(dev->name[0] != 'e'){
			printk("%s: belongs to %s, skip to set ID0-5 registers\n", __func__, dev->name);
			return err;
		}
#endif //CONFIG_ETHWAN
		hwaddr = (u32 *)dev->dev_addr;
		RTL_W32(IDR0, *hwaddr);
		hwaddr = (u32 *)(dev->dev_addr+4);
		RTL_W32(IDR4, *hwaddr);
	}
	
	return err;
}

static void __re8670_get_stats(struct re_private *cp)
{
	/* XXX implement */
}

static struct net_device_stats *re8670_get_stats(struct net_device *dev)
{
	struct re_private *cp = dev->priv;
	unsigned long flags;
#ifdef CONFIG_ETHWAN
	int port;
#endif //CONFIG_ETHWAN

	/* The chip only need report frame silently dropped. */
	spin_lock_irqsave(&cp->lock, flags);
 	if (netif_running(dev) && netif_device_present(dev))
 		__re8670_get_stats(cp);
	spin_unlock_irqrestore(&cp->lock, flags);

#ifdef CONFIG_ETHWAN
	if ((dev->name[0] == 'e') && (dev->name[4]=='.')){
		port = dev->name[5]-'2';
	} else {
		if(dev->name[0] == 'n')
			port = SW_PORT_NUM-1;
		else
			port = SW_PORT_NUM;
	}
	return &cp->net_stats[port];
#else
	return &cp->net_stats;
#endif //CONFIG_ETHWAN
}


#ifdef CONFIG_ETHWAN
static int re8670_sw_open(struct net_device *dev)
{
	int port;
	if(dev->name[0] == 'e')
		port = dev->name[5]-'2';
	else
		port = SW_PORT_NUM-1;
	
	vnet_dev_status[port]=1;
	//netif_start_queue(dev);
	return 0;
}

static int re8670_sw_close(struct net_device *dev)
{
	int port;
	if(dev->name[0] == 'e')
		port = dev->name[5]-'2';
	else
		port = SW_PORT_NUM-1;
	
	vnet_dev_status[port]=0;
	//netif_stop_queue(dev);
	return 0;
}
#endif //CONFIG_ETHWAN


static void re8670_stop_hw (struct re_private *cp)
{
	RTL_W32(IO_CMD,0); /* timer  rx int 1 packets*/
	RTL_W16(IMR, 0);
	RTL_W16(ISR, 0xffff);
	//synchronize_irq();
	synchronize_irq(cp->dev->irq);/*linux-2.6.19*/
	udelay(10);

	cp->rx_tail = 0;
	cp->tx_hqhead = cp->tx_hqtail = 0;
}

#ifndef CONFIG_RTL867X_PACKET_PROCESSOR
static void re8670_reset_hw (struct re_private *cp)
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
#endif

#ifdef NIC_USE_RGMII_TO_EXTCPU_MAC
static inline void re8670_init_forcelink(void) 
{
	/*forcing 1G full-duplex */
	#define ForceSpeed10M                       (1<<16)     /* Force speed 10M */
	#define ForceSpeed100M                      (0<<16)     /* Force speed 100M */
	#define ForceSpeed1000M                     (2<<16)     /* Force speed 1G */	
	RTL_W32(MSR, RTL_R32(MSR) & ~ForceSpeedMask);
//	RTL_W32(MSR, RTL_R32(MSR) | ForceSpeed10M | ForceDuplex | ForceSpeedMode);
//	RTL_W32(MSR, RTL_R32(MSR) | ForceSpeed100M | ForceDuplex | ForceSpeedMode);
	RTL_W32(MSR, RTL_R32(MSR) | ForceSpeed1000M | ForceDuplex | ForceSpeedMode);
	mdelay(10);
	RTL_W32(MSR, RTL_R32(MSR) | ForceLink);	
	RTL_W8(Txc_Out_Ph_Sel_l, 0x20);
	printk( "%s: force full-duplex 1G Mbps\n", __FUNCTION__ );
}
#endif /*NIC_USE_RGMII_TO_EXTCPU_MAC*/

#ifdef CONFIG_ETHWAN
static inline void re8670_start_hw (struct re_private *cp)
{
	RTL_W32(IO_CMD, CMD_CONFIG); /* timer  rx int 1 packets*/
}

static void re8670_init_hw (struct re_private *cp)
{
	struct net_device *dev = cp->dev;
	u8 status;
	u32 *hwaddr;

	re8670_reset_hw(cp);
	RTL_W8(CMD,0x2);	 /* checksum */
	/*START MODIFY: RTL for A36D06894/A36D06919 */
	RTL_W16(ISR,0xfeff);
	/*END MODIFY: RTL for A36D06894/A36D06919 */
	RTL_W16(IMR,re8670_intr_mask);   	
	RTL_W32(RxFDP,(u32)(cp->rx_ring));	
	//RTL_W8(RxCDO,0);
	RTL_W32(TxFDP1,(u32)(cp->tx_hqring));	
	RTL_W16(TxCDO1,0);	

	RTL_W32(TCR,(u32)(0x0C00));
	int desc_h, desc_l;
	RTL_W32(EthrntRxCPU_Des_Num, 0);
	RTL_W8(EthrntRxCPU_Des_Wrap, TH_ON_VAL%256);
	RTL_W8(Rx_Pse_Des_Thres, TH_OFF_VAL%256);
	#if 0
	desc_h = 1;
	desc_l = RE8670_RX_RING_SIZE;
	#else
	desc_l = (RE8670_RX_RING_SIZE > 256) ? 256 : RE8670_RX_RING_SIZE;
	desc_h = (RE8670_RX_RING_SIZE > 256) ? (RE8670_RX_RING_SIZE/256)%16 : 1;
	#endif
	RTL_W8(EthrntRxCPU_Des_Num, desc_l-1);
	RTL_W8(RxRingSize, desc_l-1);
	RTL_W8(EthrntRxCPU_Des_Num_h, (TH_ON_VAL/256)&0xF);
	RTL_W8(Rx_Pse_Des_Thres_h, (TH_OFF_VAL/256)&0xF);
	RTL_W8(EthrntRxCPU_Des_Num_h, RTL_R8(EthrntRxCPU_Des_Num_h)|((desc_h-1)<<4));
	RTL_W8(RxRingSize_h, desc_h-1);
	RTL_W32(SMSA, 0);
	
#ifdef NIC_USE_RGMII_TO_EXTCPU_MAC	
	re8670_init_forcelink();
#else /*NIC_USE_RGMII_TO_EXTCPU_MAC*/
	status = RTL_R8(MSR);
	status = status | (TXFCE|FORCE_TX|RXFCE);
	RTL_W8(MSR,status);
#endif /*NIC_USE_RGMII_TO_EXTCPU_MAC*/

	// Kao, set hw ID for physical match
	hwaddr = (u32 *)cp->dev->dev_addr;
	RTL_W32(IDR0, *hwaddr);
	hwaddr = (u32 *)(cp->dev->dev_addr+4);
	RTL_W32(IDR4, *hwaddr);
#ifndef NIC_USE_RGMII_TO_EXTCPU_MAC
#ifndef CONFIG_RE8306 // andrew, configure Internal phy LED 
	RTL_W32(LEDCR, 0x47777);
#endif
#endif /*NIC_USE_RGMII_TO_EXTCPU_MAC*/

	re8670_start_hw(cp);
	__re8670_set_rx_mode(dev);
	//krammer add for phy eye diagram patch
	{
		internal_miiar_write(1, 31, 0x0001);//page1
		internal_miiar_write(1, 29, 0x0000);//set reg29 to 0
		internal_miiar_write(1, 31, 0x0000);//page0
		miiar_write(virt2phy[SW_PORT_NUM], 0, 0x2900);//reset cpu port's link
		miiar_write(virt2phy[SW_PORT_NUM], 0, 0x2100);//set to 100Full
	}
}

#else
static inline void re8670_start_hw (struct re_private *cp)
{
	
	RTL_W32(IO_CMD,iocmd_reg); /* timer  rx int 1 packets*/
//printk("Start NIC in Pkt Processor disabled mode.. IO_CMD %x\n", RTL_R32(IO_CMD));	
#if 0
	cpw8(Cmd, RxOn | TxOn);
	cpw16(CpCmd, PCIMulRW | RxChkSum | CpRxOn | CpTxOn);
#endif
}

static void re8670_init_hw (struct re_private *cp)
{
	struct net_device *dev = cp->dev;
	u8 status;
	// Kao
	u32 *hwaddr, cputag_info;
	u16 desc_l, desc_h;
	unsigned short regValue;
	//unsigned int value;
#ifdef CONFIG_RE8306
	unsigned short tmpreg;
	
	miiar_read(6,22,&tmpreg);
	miiar_write(6,22,tmpreg|0x8000);
#endif
#ifndef CONFIG_RTL867X_PACKET_PROCESSOR
	re8670_reset_hw(cp);
	RTL_W8(CMD,0x2);	 /* checksum */		//shlee 8672
	RTL_W16(ISR,0xffff);
	RTL_W16(IMR,re8670_intr_mask);   	
	RTL_W32(RxFDP,(u32)(cp->rx_ring));	
//shlee 8672	RTL_W16(RxCDO,0);
//RxCDO is read only,	RTL_W8(RxCDO,0);
	RTL_W32(TxFDP1,(u32)(cp->tx_hqring));
	RTL_W16(TxCDO1,0);
	RTL_W32(TxFDP2,0);	
	RTL_W16(TxCDO2,0);
	RTL_W32(TxFDP3,0);	
	RTL_W16(TxCDO3,0);
	RTL_W32(TxFDP4,0);	
	RTL_W16(TxCDO4,0);


#endif //end of #ifndef CONFIG_RTL867X_PACKET_PROCESSOR
/*shlee 8672
	RTL_W32(TxFDP2,(u32)cp->tx_lqring);
	RTL_W16(TxCDO2,0);
*/	
	// Kao
//	RTL_W32(RCR,AcceptAll);		
	RTL_W32(TCR,(u32)(0x0C00));
#if 0
    RTL_W32(PGR,(u32)(0x0000)); /* Turn off packet generator */
    RTL_W32(CPUtagCR,(u32)(0x0000));  /* Turn off CPU tag function */
#if 0  //cpu tag function
    //cputag_info = (CTEN_RX | CT_8368S | CTSIZE4 | CTPM_8368 | CTPV_8368);
    //cputag_info = (CTEN_RX | CT_8306S | CTSIZE4 | CTPM_8306 | CTPV_8306);
    //cputag_info = (CTEN_RX | CT_8370S | CTSIZE8 | CTPM_8370 | CTPV_8370);
	RTL_W32(CPUtagCR,cputag_info); /* Turn on the cpu tag adding function */  //czyao 8672c
#endif
#endif

//	RTL_W8(Rx_Pse_Des_Thres,THVAL);
#if 1	//shlee
	RTL_W32(EthrntRxCPU_Des_Wrap,0);  //initialize the register content
	RTL_W8(EthrntRxCPU_Des_Wrap,(TH_ON_VAL%256));
	RTL_W8(Rx_Pse_Des_Thres,(TH_OFF_VAL%256));
	desc_l = (RE8670_RX_RING_SIZE > 256) ? 256 : RE8670_RX_RING_SIZE;
	desc_h = (RE8670_RX_RING_SIZE > 256) ? (RE8670_RX_RING_SIZE/256)%16 : 1;
	RTL_W8(EthrntRxCPU_Des_Num,desc_l-1);
	RTL_W8(RxRingSize,desc_l-1);

	RTL_W8(EthrntRxCPU_Des_Wrap_h,(TH_ON_VAL/256)%16);
	RTL_W8(Rx_Pse_Des_Thres_h,(TH_OFF_VAL/256)%16);
	RTL_W8(EthrntRxCPU_Des_Num_h,(RTL_R8(EthrntRxCPU_Des_Num_h)|(desc_h-1)<<4));
	RTL_W8(RxRingSize_h,desc_h-1);
    RTL_W32(SMSA,0);
	
#ifndef NIC_USE_RGMII_TO_EXTCPU_MAC
	status = RTL_R8(MSR);
	status = status | (TXFCE|FORCE_TX);	// enable tx flowctrl
	status = status | RXFCE;
	RTL_W8(MSR,status);	
#endif /*NIC_USE_RGMII_TO_EXTCPU_MAC*/
#else
	RTL_W8(EthrntRxCPU_Des_Num,RINGSIZE);	
	RTL_W8(RxRingSize,RINGSIZE);	
#ifndef NIC_USE_RGMII_TO_EXTCPU_MAC
	status = RTL_R8(MSR);
	status = status & ~(TXFCE | RXFCE | FORCE_TX);
//	status = status & ( RXFCE);	
	RTL_W8(MSR,status);
#endif /*NIC_USE_RGMII_TO_EXTCPU_MAC*/
#endif	
	// Kao, set hw ID for physical match
	hwaddr = (u32 *)cp->dev->dev_addr;
	RTL_W32(IDR0, *hwaddr);
	hwaddr = (u32 *)(cp->dev->dev_addr+4);
	RTL_W32(IDR4, *hwaddr);
	// Jenny, for internal PHY ethernet LED; set value 101=>Link+Act: On=Link, Off=No link, Flash=Tx or Rx activity
/*	shlee 	
	RTL_W32(MIIAR, FLAG_READ | (1<<MII_PHY_ADDR_SHIFT) | (31 <<MII_REG_ADDR_SHIFT) | (0x0000<<MII_DATA_SHIFT));
	while (!(RTL_R32(MIIAR) & 0x80000000));
	value = RTL_R32(MIIAR);
	regValue = (unsigned short)(value & 0xffff);
	RTL_W32(MIIAR, FLAG_WRITE | (1<<MII_PHY_ADDR_SHIFT) | (31<<MII_REG_ADDR_SHIFT) | (((regValue & 0xfff8) | 0x0005)<<MII_DATA_SHIFT));
*/	
#ifndef NIC_USE_RGMII_TO_EXTCPU_MAC
	#ifndef CONFIG_RE8306 // andrew, configure Internal phy LED 
	RTL_W32(LEDCR, 0x47777);
	#endif
#endif /*NIC_USE_RGMII_TO_EXTCPU_MAC*/

#ifdef NIC_USE_RGMII_TO_EXTCPU_MAC
	re8670_init_forcelink();
#endif /*NIC_USE_RGMII_TO_EXTCPU_MAC*/

	re8670_start_hw(cp);
	__re8670_set_rx_mode(dev);
	#ifdef CONFIG_EXT_SWITCH
	// Kaohj -- patch for MAC2 mii polling: set phy id of MIIAR to cpu port
	miiar_read(virt2phy[SWITCH_VPORT_TO_867X],0,&regValue);
	#endif

#ifdef CONFIG_EXT_SWITCH
extern unsigned short chipver;

if(IS_6166 || IS_RLE0315){
	//NIC and Switch are forced to 100M FD if using external switch
	//NIC forced to 100M FD
	internal_miiar_write(1, 31, 0x0);
	internal_miiar_write(1, 0, 0x2100);
    //Switch forced to 100M FD
	select_page(0);
#ifdef AUTO_SEARCH_CPUPORT
	miiar_write(virt2phy[SW_PORT_NUM], 0, 0x2100);
#else
	miiar_write(CONFIG_CPU_PORT,0,0x2100);
#endif
	select_page(0);

	//krammer add for phy eye diagram patch
	{
		internal_miiar_write(1, 31, 0x0001);//page1
		internal_miiar_write(1, 29, 0x0000);//set reg29 to 0
		internal_miiar_write(1, 31, 0x0000);//page0
		miiar_write(virt2phy[SW_PORT_NUM], 0, 0x2900);//reset cpu port's link
		miiar_write(virt2phy[SW_PORT_NUM], 0, 0x2100);//set to 100Full
	}
}else {
	internal_miiar_write(1, 31, 0x0);
	internal_miiar_write(1, 4, 0x05e1);
	select_page(0);
#ifdef AUTO_SEARCH_CPUPORT
	miiar_write(virt2phy[SW_PORT_NUM], 4, 0x05e1);
	miiar_write(virt2phy[SW_PORT_NUM], 0, 0x1200);
#else
	miiar_write(CONFIG_CPU_PORT,4,0x05e1);
	miiar_write(CONFIG_CPU_PORT,0,0x1200);
#endif
	select_page(0);
	mdelay(5);
	internal_miiar_write(1, 0, 0x1200);

}
#endif //end CONFIG_EXT_SWITCH


#if 0
	cpw8_f (Cfg9346, Cfg9346_Unlock);

	/* Restore our idea of the MAC address. */
	cpw32_f (MAC0 + 0, cpu_to_le32 (*(u32 *) (dev->dev_addr + 0)));
	cpw32_f (MAC0 + 4, cpu_to_le32 (*(u32 *) (dev->dev_addr + 4)));

	cpw8(TxThresh, 0x06); /* XXX convert magic num to a constant */
	cpw32_f (TxConfig, IFG | (TX_DMA_BURST << TxDMAShift));

	cpw8(Config1, cpr8(Config1) | DriverLoaded | PMEnable);
	cpw8(Config3, PARMEnable); /* disables magic packet and WOL */
	cpw8(Config5, cpr8(Config5) & PMEStatus); /* disables more WOL stuff */

	cpw32_f(HiTxRingAddr, 0);
	cpw32_f(HiTxRingAddr + 4, 0);
	cpw32_f(OldRxBufAddr, 0);
	cpw32_f(OldTSD0, 0);
	cpw32_f(OldTSD0 + 4, 0);
	cpw32_f(OldTSD0 + 8, 0);
	cpw32_f(OldTSD0 + 12, 0);

	cpw32_f(RxRingAddr, cp->ring_dma);
	cpw32_f(RxRingAddr + 4, 0);
	cpw32_f(TxRingAddr, cp->ring_dma + (sizeof(struct cp_desc) * CP_RX_RING_SIZE));
	cpw32_f(TxRingAddr + 4, 0);

	cpw16(MultiIntr, 0);

	cpw16(IntrMask, cp_intr_mask);

	cpw8_f (Cfg9346, Cfg9346_Lock);
#endif
}
#endif //CONFIG_ETHWAN


static int re8670_refill_rx (struct re_private *cp)
{
	unsigned i;

	for (i = 0; i < RE8670_RX_RING_SIZE; i++) {
		struct sk_buff *skb;
#ifdef CONFIG_SKB_POOL_PREALLOC
#ifdef CONFIG_RTL867X_PACKET_PROCESSOR
   	      skb = prealloc_skb_get();
#else
#ifdef CONFIG_SEPARATE_ETH_SKB_POOL
		skb = get_free_skb(CONFIG_RTL867X_ETH_PREALLOCATE_SKB_SIZE); 
#else
		skb = get_free_skb();
#endif
#endif
#else
		skb = re8670_getAlloc(SKB_BUF_SIZE);
#endif
		if (!skb)
			goto err_out;


#ifdef CONFIG_SKB_POOL_PREALLOC
		skb_reserve(skb,NET_SKB_PAD);
#endif

		// Kaohj --- invalidate DCache for uncachable usage
		//ql_xu
		dma_cache_wback_inv((unsigned long)skb->data, SKB_BUF_SIZE);
		
		skb->dev = cp->dev;
#if 0
		cp->rx_skb[i].mapping = pci_map_single(cp->pdev,
			skb->tail, cp->rx_buf_sz, PCI_DMA_FROMDEVICE);
#endif
		cp->rx_skb[i].skb = skb;
		cp->rx_skb[i].frag = 0;
		if ((u32)skb->data &0x3)
			printk(KERN_DEBUG "skb->data unaligment %8x\n",(u32)skb->data);
		
#ifdef CONFIG_SKB_POOL_PREALLOC
		cp->rx_ring[i].addr = (u32)skb->data;
#else
		cp->rx_ring[i].addr = (u32)skb->data|UNCACHE_MASK;
#endif
		if (i == (RE8670_RX_RING_SIZE - 1))
			cp->rx_ring[i].opts1 = (DescOwn | RingEnd | cp->rx_buf_sz);
		else
			cp->rx_ring[i].opts1 =(DescOwn | cp->rx_buf_sz);
		cp->rx_ring[i].opts2 = 0;
//		cp->rx_ring[i].opts3 = 0;	//shlee make sure "rx shift"=0
	}

	return 0;

err_out:
	re8670_clean_rings(cp);
	return -ENOMEM;
}

#ifndef CONFIG_RTL867X_PACKET_PROCESSOR
static void re8670_tx_timeout (struct net_device *dev)
{
	unsigned long flags;
	struct re_private *cp = dev->priv;
	unsigned tx_head;
	unsigned tx_tail;

#ifdef CONFIG_ETHWAN
	int port;
	if (dev->name[0] != 'e') {//not eth0.# wanport
		port = SW_PORT_NUM-1;
	}
	else
		port = dev->name[5]-'2';
#endif //CONFIG_ETHWAN	
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
#ifdef CONFIG_ETHWAN
		cp->net_stats[port].tx_packets++;
		cp->net_stats[port].tx_bytes += skb->len;
#else
		cp->net_stats.tx_packets++;
		cp->net_stats.tx_bytes += skb->len;
#endif //CONFIG_ETHWAN		

		dev_kfree_skb(skb);
		cp->tx_skb[tx_tail].skb = NULL;
		tx_tail = NEXT_TX(tx_tail);
	}

	cp->tx_hqtail = tx_tail;

	spin_unlock_irqrestore(&cp->lock, flags);
	if (netif_queue_stopped(cp->dev))
		netif_wake_queue(cp->dev);

}
#endif

static int re8670_init_rings (struct re_private *cp)
{
//shlee 2.6	memset(cp->tx_hqring, 0, sizeof(DMA_DESC) * RE8670_TX_RING_SIZE);
//shlee 2.6	memset(cp->rx_ring, 0, sizeof(DMA_DESC) * RE8670_RX_RING_SIZE);
	cp->rx_tail = 0;
	cp->tx_hqhead = cp->tx_hqtail = 0;

	return re8670_refill_rx (cp);
}

static int re8670_alloc_rings (struct re_private *cp)
{
	/*cp->rx_ring = pci_alloc_consistent(cp->pdev, CP_RING_BYTES, &cp->ring_dma);*/
	void*	pBuf;
	
	
	
//shlee 2.6	pBuf = kmalloc(RE8670_RXRING_BYTES,GFP_KERNEL);
	pBuf = kzalloc(RE8670_RXRING_BYTES, GFP_KERNEL|__GFP_NOFAIL);
	if (!pBuf)
		goto ErrMem;
	// Kaohj -- invalidate DCache for uncachable usage
	dma_cache_wback_inv((unsigned long)pBuf, RE8670_RXRING_BYTES);
	cp->rxdesc_buf = pBuf;
//shlee 2.6	memset(pBuf, 0, RE8670_RXRING_BYTES);
	
	pBuf = (void*)( (u32)(pBuf + DESC_ALIGN-1) &  ~(DESC_ALIGN -1) ) ;
	cp->rx_ring = (DMA_DESC*)((u32)(pBuf) | UNCACHE_MASK);


//shlee 2.6	pBuf= kmalloc(RE8670_TXRING_BYTES, GFP_KERNEL);
	pBuf = kzalloc(RE8670_TXRING_BYTES, GFP_KERNEL|__GFP_NOFAIL);
	if (!pBuf)
		goto ErrMem;
	// Kaohj -- invalidate DCache for uncachable usage
	dma_cache_wback_inv((unsigned long)pBuf, RE8670_TXRING_BYTES);
	cp->txdesc_buf = pBuf;
//shlee 2.6	memset(pBuf, 0, RE8670_TXRING_BYTES);
	pBuf = (void*)( (u32)(pBuf + DESC_ALIGN-1) &  ~(DESC_ALIGN -1) ) ;
	cp->tx_hqring = (DMA_DESC*)((u32)(pBuf) | UNCACHE_MASK);

	return re8670_init_rings(cp);

ErrMem:
	if (cp->rxdesc_buf)
		kfree(cp->rxdesc_buf);
	if (cp->txdesc_buf)
		kfree(cp->txdesc_buf);
	return -ENOMEM;

	
}

static void re8670_clean_rings (struct re_private *cp)
{
	unsigned i;


	for (i = 0; i < RE8670_RX_RING_SIZE; i++) {
		if (cp->rx_skb[i].skb) {
#if defined(CONFIG_SKB_POOL_PREALLOC) && defined(CONFIG_RTL867X_PACKET_PROCESSOR)
			if(cp->rx_skb[i].skb->fcpu==0)
				prealloc_skb_free(cp->rx_skb[i].skb);
			else
				dev_kfree_skb(cp->rx_skb[i].skb);
#else
			dev_kfree_skb(cp->rx_skb[i].skb);
#endif
		}
	}

	for (i = 0; i < RE8670_TX_RING_SIZE; i++) {
		if (cp->tx_skb[i].skb) {
			struct sk_buff *skb = cp->tx_skb[i].skb;
#if defined(CONFIG_SKB_POOL_PREALLOC) && defined(CONFIG_RTL867X_PACKET_PROCESSOR)
			if(skb->fcpu==0)
				prealloc_skb_free(skb);
			else
				dev_kfree_skb(skb);				
#else
			dev_kfree_skb(skb);
#endif
#ifdef CONFIG_ETHWAN
		cp->net_stats[SW_PORT_NUM].tx_dropped++;
#else
		cp->net_stats.tx_dropped++;
#endif //CONFIG_ETHWAN
			
		}
	}

	memset(&cp->rx_skb, 0, sizeof(struct ring_info) * RE8670_RX_RING_SIZE);
	memset(&cp->tx_skb, 0, sizeof(struct ring_info) * RE8670_TX_RING_SIZE);
}

static void re8670_free_rings (struct re_private *cp)
{
	re8670_clean_rings(cp);
	/*pci_free_consistent(cp->pdev, CP_RING_BYTES, cp->rx_ring, cp->ring_dma);*/
	kfree(cp->rxdesc_buf);
	kfree(cp->txdesc_buf);
	
	cp->rx_ring = NULL;
	cp->tx_hqring = NULL;
}

#ifdef PHY_MDI_MDIX_AUTO
int rtl867x_getInfo(unsigned int no, unsigned short *pVal)
{
	unsigned short regval;

	//set page 1
	internal_select_page(1);
	
	//set DBG_SR
	internal_miiar_read(no, 2, &regval);
	regval &= ~0xF;
	internal_miiar_write(no, 2, regval);

	//get DBG_O
	internal_miiar_read(no, 0, pVal); 

	//set page 0
	internal_select_page(0);
	
    return 0;
}

void eth_checkStatus(void){
	unsigned int no;
	unsigned short wt[5];
	unsigned int v1[5];
	unsigned int cntTry;
	unsigned short regval;
	unsigned int tryFlag[5];
	unsigned int v2;
	static unsigned int cnt[5];
	unsigned int ii;

	no = INTERNAL_PHY_ID;
	{
		internal_miiar_read(no, 1, &regval);
		v1[no] = (regval & 0x4);
	      
	  	if(v1[no])       
		{
			//printk("link:%x\n",v1[no]);
			rtl867x_getInfo(no, &wt[no]);
			if (wt[no] > 0xCE4){       
				tryFlag[no] = 1;
			}
			else{ 
				tryFlag[no] = 0;            
				eth_link_stable_times++;
			}
		}              
		else 
		{
			printk("no link:%x\n",v1[no]);
			tryFlag[no] = 0;
		}                
	}
	//printk("dbg:%x\n",wt[no]);
	if(eth_link_stable_times==ETH_LINK_STABLE)
		return;
	for (cntTry = 0; cntTry < 1; cntTry ++)
	{
		no = INTERNAL_PHY_ID;
		{
			if (!v1[no]||tryFlag[no])              
			{
				//set page 1
				internal_select_page(1);

				internal_miiar_read(no, 25, &regval);
				if (regval & (1 << 1))  
				{                        
					regval &= ~(1 << 1);
					if ((cnt[no] & 0x2) == 0x2) { 
						regval &= ~(1 << 0);
						//printk("MDIX selected\n");
					}
					else{
						regval |= (1 << 0);    
						//printk("MDI selected\n");
					}
					internal_miiar_write(no, 25, regval);           
					for(ii = 0; ii < 50; ii ++);        
				}
				else         
				{
					regval |= (1 << 1);
					internal_miiar_write(no, 25, regval);
					cnt[no]++;
					//printk("auto\n");
					for(ii = 0; ii < 50; ii ++);     
				}  

				//set page 0
				internal_select_page(0);
			}
		}        
	}
        mod_timer(&eth_phy_check_timer, jiffies + 1*HZ);
	return 0;
}
#endif

#ifdef CONFIG_EXT_SWITCH
#ifdef CONFIG_RE8306_API
int rtl8306_getInfo(unsigned int phy, unsigned short *pVal)
{
	uint32 regval;
	
	rtl8306_setAsicPhyRegBit(0, 16, 11, 0, 1);
	rtl8306_getAsicPhyReg(1, 27, 0, &regval);
	regval &= ~0xF;
	rtl8306_setAsicPhyReg(1, 27, 0, regval);
	rtl8306_getAsicPhyReg(0, 28, 0, &regval);
	regval &= ~0x7;
	regval |= phy;
	rtl8306_setAsicPhyReg(0, 28, 0, regval);
	rtl8306_getAsicPhyReg(0, 30, 0, pVal);
	rtl8306_setAsicPhyRegBit(0, 16, 11, 0, 0);
}

void sw_checkStatus(void)
{
	unsigned short wt[5];
	unsigned int v1[5], v2;
	unsigned int tryFlag[5];
	unsigned int modtimer=0;
	int port;

	for (port =0; port<SW_PORT_NUM; port++)
	{
		if (CHECK_OK == sw_link_checked[port])
			continue;

		rtl8306_getAsicPhyRegBit(virt2phy[port], 1, 2, 0, &v1[port]);
		rtl8306_getAsicPhyRegBit(virt2phy[port], 1, 2, 0, &v1[port]);
		rtl8306_getAsicPhyRegBit(virt2phy[port], 0, 13, 0, &v2);
	  	if(v1[port] && v2)
		{
			rtl8306_getInfo(virt2phy[port], &wt[port]);
			if (wt[port] > 0xCE4){
				tryFlag[port] = 1;
			}
			else{
				tryFlag[port] = 0;
				sw_link_stable_times[port]++;
				modtimer |= (1<<port);
			}
		}              
		else 
		{
			tryFlag[port] = 0;
		}                

		if(sw_link_stable_times[port]==ETH_LINK_STABLE) {
			sw_link_checked[port]=CHECK_OK;
			modtimer &= (~(1<<port));
			continue;
		}
		
		if (tryFlag[port])
		{
			printk("restart autonegotiation on port %d\n", port);
			rtl8306_setAsicPhyRegBit(virt2phy[port], 0, 9, 0, 1);
		}
	}
	if (modtimer)
		mod_timer(&sw_check_timer, jiffies + 1*HZ);
	return 0;
}
#endif

#ifdef CONFIG_RTL8672_PWR_SAVE

int (*eth_event_callback)(int, void *);

static void mgt_function(void *data) {
	static u8 sw_port_link[SW_PORT_NUM] = { 2,2,2,2 };
	static u8 swport = 0, sw_link_mask = 0;
	uint32 is_link_up;			
	extern void rtl8672_low_power_event(int);
	struct mgt_struct *m = (struct mgt_struct *)data;
	mod_timer(&m->mgt_timer, jiffies + MGT_TICK);


	/* detect link change */
	rtl8306_getAsicPHYLinkStatus(virt2phy[swport], &is_link_up);
	sw_link_mask = (sw_link_mask & ~(1 << swport)) | (is_link_up << swport);
	if (unlikely(sw_port_link[swport] != is_link_up)) {
		sw_port_link[swport] = (u8)is_link_up;
		//printk("[hook]port%d is %s(%x),%d,%d.%x\n",swport, is_link_up ? "up" : "down", sw_link_mask, is_link_up, swport,~(is_link_up << swport));
		/* 
		it's better to call netif_carrier_off() but unfortunately we do not have
		corresponding netdev. Andrew		
		*/
		if (eth_event_callback) {
			if (sw_link_mask & 0xF)
				eth_event_callback(0, NULL);
			else
				eth_event_callback(1, NULL);
		}		
	}
	swport++;
	if (unlikely(swport==SW_PORT_NUM)) 
		swport = 0;

}
#endif //PWR_SAVE

#endif

static int re8670_open (struct net_device *dev)
{
	struct re_private *cp = dev->priv;
	int rc;

	// Kaohj
#ifdef CONFIG_EXT_SWITCH
	if (dev->base_addr == 0) {
		netif_start_queue(dev);
		return 0;
	}
#endif
	eth_close=0;
	if (netif_msg_ifup(cp))
		printk(KERN_DEBUG "%s: enabling interface\n", dev->name);

	re8670_set_rxbufsize(cp);	/* set new rx buf size */
	rc = re8670_alloc_rings(cp);
	if (rc)
		return rc;

	re8670_init_hw(cp);
	// Kaohj
#ifdef CONFIG_EXT_SWITCH
	if (enable_virtual_port) {
		RTL_W8(CMD,RTL_R8(CMD)|0x04);	// VLAN de-tagging enabled
	}
#endif

//	rc = request_irq(dev->irq, re8670_interrupt, 0, dev->name, dev);
//	rc = request_irq(dev->irq, (irq_handler_t)re8670_interrupt, SA_INTERRUPT, dev->name, dev);
	/*linux-2.6.19*/
	rc = request_irq(dev->irq, (irq_handler_t)re8670_interrupt, IRQF_DISABLED, dev->name, dev);
	if (rc)
		goto err_out_hw;

	netif_start_queue(dev);

#ifdef PHY_MDI_MDIX_AUTO
	init_timer(&eth_phy_check_timer);
	eth_link_stable_times=0;
	eth_phy_check_timer.function=(void (*)(unsigned long))eth_checkStatus;
	mod_timer(&eth_phy_check_timer, jiffies+1*HZ);
#endif
#ifdef CONFIG_EXT_SWITCH
	if ((0x5988 == swtype) && timer_pending(&sw_check_timer)) {
		del_timer(&sw_check_timer);
	}
	#ifdef CONFIG_RTL8672_PWR_SAVE
	mod_timer(&mgt.mgt_timer, jiffies + MGT_TICK);
	#endif
#endif

	return 0;

err_out_hw:
	re8670_stop_hw(cp);
	re8670_free_rings(cp);
	return rc;
}

static int re8670_close (struct net_device *dev)
{
	struct re_private *cp = dev->priv;

	// Kaohj
#ifdef CONFIG_EXT_SWITCH
	if (dev->base_addr == 0) {
		netif_stop_queue(dev);
		return 0;
	}
	#ifdef CONFIG_RTL8672_PWR_SAVE
	if (timer_pending(&mgt.mgt_timer))
		del_timer(&mgt.mgt_timer);
	#endif
#endif
	eth_close=1;
	//if (eth_rx_tasklets) tasklet_disable(eth_rx_tasklets);
	if (netif_msg_ifdown(cp))
		printk(KERN_DEBUG "%s: disabling interface\n", dev->name);

	netif_stop_queue(dev);
	re8670_stop_hw(cp);
	free_irq(dev->irq, dev);
	re8670_free_rings(cp);

#ifdef PHY_MDI_MDIX_AUTO
	if(timer_pending(&eth_phy_check_timer)){
		del_timer(&eth_phy_check_timer);
	}
#endif
	return 0;
}

#if 0
static int re8670_change_mtu(struct net_device *dev, int new_mtu)
{
	struct re_private *cp = dev->priv;
	int rc;

	/* check for invalid MTU, according to hardware limits */
	if (new_mtu < CP_MIN_MTU || new_mtu > CP_MAX_MTU)
		return -EINVAL;

	/* if network interface not up, no need for complexity */
	if (!netif_running(dev)) {
		dev->mtu = new_mtu;
		re8670_set_rxbufsize(cp);	/* set new rx buf size */
		return 0;
	}

	spin_lock_irq(&cp->lock);

	re8670_stop_hw(cp);			/* stop h/w and free rings */
	re8670_clean_rings(cp);

	dev->mtu = new_mtu;
	re8670_set_rxbufsize(cp);		/* set new rx buf size */

	rc = re8670_init_rings(cp);		/* realloc and restart h/w */
	re8670_start_hw(cp);
	spin_unlock_irq(&cp->lock);

	return rc;
}

static char mii_2_8139_map[8] = {
	BasicModeCtrl,
	BasicModeStatus,
	0,
	0,
	NWayAdvert,
	NWayLPAR,
	NWayExpansion,
	0
};
static int mdio_read(struct net_device *dev, int phy_id, int location)
{
	struct re_private *cp = dev->priv;

	return location < 8 && mii_2_8139_map[location] ?
	       readw(cp->regs + mii_2_8139_map[location]) : 0;
}


static void mdio_write(struct net_device *dev, int phy_id, int location,
		       int value)
{
	struct re_private *cp = dev->priv;

	if (location == 0) {
		cpw8(Cfg9346, Cfg9346_Unlock);
		cpw16(BasicModeCtrl, value);
		cpw8(Cfg9346, Cfg9346_Lock);
	} else if (location < 8 && mii_2_8139_map[location])
		cpw16(mii_2_8139_map[location], value);
}

#endif
#if 0
static int re8670_ethtool_ioctl (struct re_private *cp, void *useraddr)
{
	u32 ethcmd;

	/* dev_ioctl() in ../../net/core/dev.c has already checked
	   capable(CAP_NET_ADMIN), so don't bother with that here.  */

	if (get_user(ethcmd, (u32 *)useraddr))
		return -EFAULT;

	switch (ethcmd) {

	case ETHTOOL_GDRVINFO: {
		struct ethtool_drvinfo info = { ETHTOOL_GDRVINFO };
		strcpy (info.driver, DRV_NAME);
		strcpy (info.version, DRV_VERSION);
//shlee 2.6		strcpy (info.bus_info, cp->pdev->slot_name);
		if (copy_to_user (useraddr, &info, sizeof (info)))
			return -EFAULT;
		return 0;
	}

	/* get settings */
	case ETHTOOL_GSET: {
		struct ethtool_cmd ecmd = { ETHTOOL_GSET };
		spin_lock_irq(&cp->lock);
		mii_ethtool_gset(&cp->mii_if, &ecmd);
		spin_unlock_irq(&cp->lock);
		if (copy_to_user(useraddr, &ecmd, sizeof(ecmd)))
			return -EFAULT;
		return 0;
	}
	/* set settings */
	case ETHTOOL_SSET: {
		int r;
		struct ethtool_cmd ecmd;
		if (copy_from_user(&ecmd, useraddr, sizeof(ecmd)))
			return -EFAULT;
		spin_lock_irq(&cp->lock);
		r = mii_ethtool_sset(&cp->mii_if, &ecmd);
		spin_unlock_irq(&cp->lock);
		return r;
	}
	/* restart autonegotiation */
	case ETHTOOL_NWAY_RST: {
		return mii_nway_restart(&cp->mii_if);
	}
	/* get link status */
	case ETHTOOL_GLINK: {
		struct ethtool_value edata = {ETHTOOL_GLINK};
		edata.data = mii_link_ok(&cp->mii_if);
		if (copy_to_user(useraddr, &edata, sizeof(edata)))
			return -EFAULT;
		return 0;
	}

	/* get message-level */
	case ETHTOOL_GMSGLVL: {
		struct ethtool_value edata = {ETHTOOL_GMSGLVL};
		edata.data = cp->msg_enable;
		if (copy_to_user(useraddr, &edata, sizeof(edata)))
			return -EFAULT;
		return 0;
	}
	/* set message-level */
	case ETHTOOL_SMSGLVL: {
		struct ethtool_value edata;
		if (copy_from_user(&edata, useraddr, sizeof(edata)))
			return -EFAULT;
		cp->msg_enable = edata.data;
		return 0;
	}

	default:
		break;
	}

	return -EOPNOTSUPP;
}
#endif

// Kao added
unsigned short MII_RD(unsigned char PHY_ADDR, unsigned char REG_ADDR)
{
	unsigned int value;
#ifndef CONFIG_RTL8672
	u32 sicr;
#endif
	unsigned short data;

//shlee 2.6 	#ifdef CONFIG_RTL8671
#ifdef CONFIG_RTL8672
//shlee 8672	sicr = (*(volatile u32*)(0xB9C04000));	//tylo, 0xbd800000));
	//if (sicr & 0x00000600)  //7/1/05' hrchen, for 8671 MII spec
#ifdef CONFIG_EXT_SWITCH
	if(1)
#else
	if(0)
#endif //CONFIG_EXT_SWITCH
#else
	sicr = (*(volatile u32*)(0xbd800000));
	if (sicr & 0x10)
#endif
	{	// external phy
#ifdef CONFIG_EXT_SWITCH
		miiar_read(PHY_ADDR, REG_ADDR,&data);
#else
		// fix me
		data=0;
#endif
		return data;
	}
	else
	{
		RTL_W32(MIIAR, FLAG_READ | (PHY_ADDR<<MII_PHY_ADDR_SHIFT) | (REG_ADDR <<MII_REG_ADDR_SHIFT));
		
		while (!(RTL_R32(MIIAR) & 0x80000000));
		
		value = RTL_R32(MIIAR);
		data = (unsigned short)(value & 0xffff);
	}
	
	return data;
}

// Mason Yu. For Set IPQOS
#define		SETIPQOS		0x01

/*
 * Structure used in SIOCSIPQOS request.
 */

struct ifIpQos
{
	int	cmd;
	char	enable;	
};

static void re8670_ipqos_ioctl(struct ifIpQos *req)
{
	
	switch(req->cmd) {
	
	case SETIPQOS:
		if (req->enable) {
			RTL_W8(CMD,RTL_R8(CMD)|0x04);	// VLAN de-tagging enabled
			enable_ipqos = 1;					
		}
		else {
			enable_ipqos = 0;
		}
		break;
	
	default: break;
	}
}

// Mason Yu. combine_1p_4p_PortMapping
//#ifdef CONFIG_EXT_SWITCH
#define  VLAN_ENABLE 0x01
#define  VLAN_SETINFO 0x02
#define  VLAN_SETPVIDX 0x03
#define  VLAN_SETTXTAG 0x04
#define  VLAN_DISABLE1PPRIORITY 0x05
#define	 VLAN_SETIGMPSNOOP	0x06
#define	 VLAN_SETPORTMAPPING	0x07
#define	 VLAN_SETIPQOS		0x08
#define	 VLAN_VIRTUAL_PORT	0x09
#define	 VLAN_SETVLANGROUPING	0x0a
#ifdef CONFIG_PPPOE_PROXY
#define    SET_PPPOE_PROXY_PORTMAP  0x0b 
#endif
#define		VLAN_SETMLDSNOOP		0x0c		// Mason Yu. MLD snooping
 
// Kaohj
/*
 * Structure used in SIOCSIFVLAN request.
 */
 
struct ifvlan
{
	int cmd;
	char enable;
	short vlanIdx;
	short vid;
	char	disable_priority;
	int member;
	int port;
	char txtag;
};

#if defined( CONFIG_EXT_SWITCH) && defined( CONFIG_ETHWAN)
void port_mapping_switch_setting(int enable){
	if(enable){
		rtl8306e_vlan_leaky_set(RTL8306_VALN_LEAKY_UNICAST, 0);
		rtl8306e_portLearningAbility_set(1,FALSE);
		rtl8306e_portLearningAbility_set(2,FALSE);
		rtl8306e_portLearningAbility_set(3,FALSE);
		rtl8306e_regbit_set(0,23,7,0,1);
		printk("enable fast aging...\n");
		mdelay(20);
		rtl8306e_regbit_set(0,23,7,0,0); 
		printk("disable fast aging...\n");
	}
	else{
		rtl8306e_vlan_leaky_set(RTL8306_VALN_LEAKY_UNICAST, 1);
		rtl8306e_portLearningAbility_set(1,TRUE);
		rtl8306e_portLearningAbility_set(2,TRUE);
		rtl8306e_portLearningAbility_set(3,TRUE);
	}
}
#else
void port_mapping_switch_setting(int enable){
}
#endif

#ifdef CONFIG_RTL8672_SAR
void pp_ifgroup_setup(void);
#endif
static void re8670_8305vlan_portMapping_ioctl(struct ifvlan *req)
{
#ifdef CONFIG_EXT_SWITCH
	int i;
	unsigned short regValue;
#endif

	switch(req->cmd) {
// Mason Yu. combine_1p_4p_PortMapping
#ifdef CONFIG_EXT_SWITCH
	case VLAN_ENABLE:
		// set into device
		for (i=0; i<SW_PORT_NUM; i++)
			dev_sw[i]->vlan = req->enable;
		//printk("enable: en=%d\n", req->enable);
		rtl8305_info.vlan_en = req->enable;
		#ifndef CONFIG_ETHWAN
		rtl8305sc_setAsicVlanEnable(req->enable);
		#endif
		break;
	case VLAN_SETINFO:
//		printk("info: vlanIdx=0x%x, vid=0x%x, mem=0x%x\n", req->vlanIdx, req->vid, req->member);
		rtl8305_info.vlan[req->vlanIdx].vid = req->vid;
		rtl8305_info.vlan[req->vlanIdx].member = bitmap_virt2phy(req->member);
		//printk("info: translate mbr to 0x%.08x\n", rtl8305_info.vlan[req->vlanIdx].member);
		#ifndef CONFIG_ETHWAN
		rtl8305sc_setAsicVlan(req->vlanIdx,req->vid, rtl8305_info.vlan[req->vlanIdx].member);
		#endif
		vc_ch_remapping(&(rtl8305_info.vlan[req->vlanIdx].member));
		break;
	case VLAN_SETPVIDX:
		if (req->port >=0 && req->port <= (SW_PORT_NUM-1)) { // to identify switch ports
			// set vid into device
			miiar_read(req->vlanIdx, 25, &regValue);
			dev_sw[req->port]->vid = (regValue & 0x0fff);
			//printk("pvidx: port=%d, vlanIdx=0x%x\n", req->port, req->vlanIdx);
			rtl8305_info.phy[virt2phy[req->port]].vlanIndex = req->vlanIdx;
			//printk("pidx: phy=%d, vlanidx=%d\n", virt2phy[req->port], req->vlanIdx);
			#ifndef CONFIG_ETHWAN
			rtl8305sc_setAsicPortVlanIndex(virt2phy[req->port],req->vlanIdx);
			#endif
		}
		else { // for vlan_groupping
			rtl8305_info.phy[SW_PORT_NUM+1+virt2phy[req->port-SW_PORT_NUM-1]].vlanIndex = req->vlanIdx;
		}
		break;
	case VLAN_SETTXTAG:
		// set into device
		// if it is for port 4 (867x NIC) ...
		if (req->port >= 4) {
			#ifndef CONFIG_ETHWAN
			RTL_W8(CMD,RTL_R8(CMD)|0x04);	// VLAN de-tagging enabled
			#endif
			enable_virtual_port=1;
			/*
			if (req->txtag == TAG_REMOVE)
				RTL_W8(CMD,RTL_R8(CMD)|0x04);	// VLAN de-tagging enabled
			else if (req->txtag == TAG_DCARE)
				RTL_W8(CMD,RTL_R8(CMD)&0xfb);	// VLAN de-tagging disabled
			*/
		}
		else
			dev_sw[req->port]->egtag = req->txtag;
		//printk("txtag: port=%d, txtag=0x%x\n", req->port, req->txtag);
		rtl8305_info.phy[virt2phy[req->port]].egtag = req->txtag;
		#ifndef CONFIG_ETHWAN
		rtl8305sc_setVLANTagInsertRemove(virt2phy[req->port],req->txtag);
		#endif
		break;
	case VLAN_DISABLE1PPRIORITY:
		rtl8305sc_setVLANTag1pProirity(virt2phy[req->port], req->disable_priority);
		break;
	case VLAN_SETIGMPSNOOP:
		setIGMPSnooping(req->enable);
		break;
	// Mason Yu. MLD snooping
#ifdef CONFIG_IPV6
	case VLAN_SETMLDSNOOP:
		setMLDSnooping(req->enable);
		break;
#endif
	case VLAN_SETPORTMAPPING:
		if (req->enable) {
			rtl8305_info.vlan_tag_aware=1;
			rtl8305_info.vlan_en=1;
			#ifndef CONFIG_ETHWAN
			rtl8305sc_setAsicVlanTagAware(1); //enable tag aware
			#endif
			//rtl8305sc_setAsicVlanEnable(1);	//VLAN enable
			port_mapping_switch_setting(1); // 1 enable
			enable_port_mapping = 1;
			for (i=0; i<SW_PORT_NUM; i++)
				rtl8305_info.phy[virt2phy[i]].egtag=TAG_REMOVE;
			#ifndef CONFIG_ETHWAN
			set_8305(&rtl8305_info);
			#endif
			#ifdef CONFIG_RTL8672_SAR
			pp_ifgroup_setup();
			#endif
		}
		else {
			rtl8305_info.vlan_tag_aware=0;
			rtl8305_info.vlan_en=0;
			port_mapping_switch_setting(0);// 0 disable
			enable_port_mapping = 0;
		}
		break;
	case VLAN_SETVLANGROUPING:
		if (req->enable) {
			rtl8305_info.vlan_tag_aware=1;
			rtl8305_info.vlan_en=1;
			#ifndef CONFIG_ETHWAN
			rtl8305sc_setAsicVlanTagAware(1); //enable tag aware
			#endif
			enable_vlan_grouping = 1;
			for (i=0; i<SW_PORT_NUM; i++)
				rtl8305_info.phy[virt2phy[i]].egtag=TAG_REMOVE;
			#ifndef CONFIG_ETHWAN
			set_8305(&rtl8305_info);
			#endif
		}
		else {
			rtl8305_info.vlan_tag_aware=0;
			rtl8305_info.vlan_en=0;
			enable_vlan_grouping = 0;
			#if 0
			for (i=0; i<SW_PORT_NUM; i++)
				rtl8305_info.phy[virt2phy[i]].egtag=TAG_DCARE;
			set_8305(&rtl8305_info);
			#endif
		}
		break;
	case VLAN_SETIPQOS:
		if (req->enable) {
			enable_ipqos = 1;
		}
		else {
			enable_ipqos = 0;
		}
		break;
	case VLAN_VIRTUAL_PORT:
		#if (VPORT_USE==VPORT_VLAN_TAG)
		if (req->enable) {
			#ifndef CONFIG_ETHWAN
			RTL_W8(CMD,RTL_R8(CMD)|0x04);	// VLAN de-tagging enabled
			#endif
			enable_virtual_port=1;
		}
		else {
			enable_virtual_port=0;
			rtl8305_info.phy[virt2phy[SWITCH_VPORT_TO_867X]].egtag = TAG_DCARE;	// don't care
			#ifndef CONFIG_ETHWAN
			rtl8305sc_setVLANTagInsertRemove(virt2phy[SWITCH_VPORT_TO_867X],TAG_DCARE);
			#endif
		}
		#else // VPORT_CPU_TAG
		if (req->enable) {
			#ifndef CONFIG_ETHWAN
                 	// Set cpu port
			#ifdef CONFIG_NEW_PORTMAPPING
			rtl8306_setCPUPort(CONFIG_CPU_PORT, TRUE ,NULL);
			#else
			miiar_read(4, 21, &regValue);
			miiar_write(4, 21, (regValue & 0xFFF8) | virt2phy[4]);
			// Enable cpu tag insert [21.12]
			select_page(3);
			miiar_read(2, 21, &regValue);
			miiar_write(2, 21, regValue | 0x1000);
			select_page(0);
			#endif
			#endif
			enable_virtual_port=1;
		}
		else {
			enable_virtual_port=0;
			#ifndef CONFIG_ETHWAN
			// Disable cpu tag insert [21.12]
			select_page(3);
			miiar_read(2, 21, &regValue);
			miiar_write(2, 21, regValue & 0xEFFF);
			select_page(0);
			#endif
		}
		#endif
		break;

	#ifdef CONFIG_PPPOE_PROXY
	case  SET_PPPOE_PROXY_PORTMAP:
        
		if(enable_port_mapping){
		//	     printk("******SET_PPPOE_PROXY_PORTMAP****** member %x\n",req->member);
		 	pppoeproxy_portmap =req->member;    
		}
			
	 	
		break;	
	#endif

// Mason Yu. The following cases of ioctl are for 1 LAN port 's Port Mapping
#else
	case VLAN_SETINFO:
		printk("eth member:%x\n",req->member);
		eth_dev_member=req->member;
		break;
	case VLAN_SETPORTMAPPING:
		if (req->enable) {
			printk("enable pmapping in kernel!\n");
			enable_port_mapping = 1;
		}
		else {
			printk("disable!\n");
			enable_port_mapping = 0;
		}
		#ifdef CONFIG_RTL8672_SAR
		pp_ifgroup_setup();
		#endif
		break;
	#ifdef CONFIG_PPPOE_PROXY
	case  SET_PPPOE_PROXY_PORTMAP:
        
		if(enable_port_mapping){
		//	     printk("******SET_PPPOE_PROXY_PORTMAP****** member %x\n",req->member);
		 	pppoeproxy_portmap =req->member;    
		}


		 break;	
	#endif
	break;

#endif	
	default: break;
	}
}
//#endif  // #ifdef CONFIG_EXT_SWITCH




int re8670_ioctl (struct net_device *dev, struct ifreq *rq, int cmd)
{
	//struct re_private *cp = dev->priv;
	int rc = 0;
	unsigned short *data = (unsigned short *)(&rq->ifr_data);
	u32 *data32 = (unsigned int *)(&rq->ifr_data);
#ifndef CONFIG_RTL8672
	u32 sicr;
#endif
	u8 status;
	#ifdef CONFIG_EXT_SWITCH
	struct mii_ioctl_data *mii = (struct mii_ioctl_data *)&rq->ifr_data;
	int port;
	#endif
	
	if (!netif_running(dev) && cmd!=SIOCETHTEST)
		return -EINVAL;

	switch (cmd) {
	case SIOCETHTOOL:
//fixme later	return re8670_ethtool_ioctl(cp, (void *) rq->ifr_data);
		break;

	case SIOCGMIIPHY:
		//shlee 2.6 	#ifdef CONFIG_RTL8671
#ifdef CONFIG_RTL8672
//shlee 8672                sicr = (*(volatile u32*)(0xB9C04000));	//tylo, 0xbd800000));
		//if (sicr & 0x00000600)  //7/1/05' hrchen, for 8671 MII spec
#ifdef CONFIG_EXT_SWITCH
		if(1)
#else
		if(0)
#endif //CONFIG_EXT_SWITCH
#else
		sicr = (*(volatile u32*)(0xbd800000));
		if (sicr & 0x10)
#endif
		{
			data32[1]=4;	// external phy address
//			printk("use MII\n");
#ifdef CONFIG_EXT_SWITCH
			for (port = 0; port < SW_PORT_NUM; port++) {
				if (dev_sw[port] == dev) {
					data32[1] = port;
					break;
				}
			}
#endif
		}
		else
		{
			data32[1]=1;	// internal phy address == 1
//			printk("internal phy\n");
		}
		
		break;
	case SIOCGMIIREG:
		data[3] = MII_RD(data[0], data[1]);
//		printk("phyid=%d, miireg=%d\n", data[0], data[1]);
//		printk("data[3]=%X\n", data[3]);
		break;
	//tylo, for VLAN setting
	// Mason Yu. combine_1p_4p_PortMapping	
	case SIOCS8305VLAN:
		re8670_8305vlan_portMapping_ioctl((struct ifvlan *)rq->ifr_data);
		break;
	#ifdef CONFIG_EXT_SWITCH
	case SIOCSMIIREG:
		//miiar_write((unsigned char)mii->phy_id,(unsigned char)mii->reg_num, mii->val_in);
		miiar_write(virt2phy[mii->phy_id],(unsigned char)mii->reg_num, mii->val_in);
		break;
	#endif	
	
#if 1	//2.6 shlee
	case SIOCDIRECTBR:  // ioctl for direct bridge mode, jiunming
#ifdef FAST_ROUTE_Test
		//printk( "\nSIOCDIRECTBR: old=%d new=%d", fast_route_mode, rq->ifr_ifru.ifru_ivalue );
		if( rq->ifr_ifru.ifru_ivalue==1 )
			fast_route_mode = 1;
		else if( rq->ifr_ifru.ifru_ivalue==0 )
			fast_route_mode = 0;
		else
#endif
			rc = -EINVAL;
		break;
#endif	
	case SIOCGMEDIALS:	// Jenny, ioctl for media link status
		status = RTL_R8(MSR);
		if (status & (1 << 2))
			rc = 0;
		else
			rc = 1;
		break;
	case SIOCETHTEST:
		eth_ctl((struct eth_arg *)rq->ifr_data);
		break;
		// Mason Yu. For Set IPQOS
	case SIOCSIPQOS:		
	 	re8670_ipqos_ioctl((struct ifIpQos *)rq->ifr_data);
	 	break;
	default:
		rc = -EOPNOTSUPP;
		break;
	}

	return rc;
}

#if CP_VLAN_TAG_USED
static void cp_vlan_rx_register(struct net_device *dev, struct vlan_group *grp)
{
	struct re_private *cp = dev->priv;

	spin_lock_irq(&cp->lock);
	cp->vlgrp = grp;
	cpw16(CpCmd, cpr16(CpCmd) | RxVlanOn);
	spin_unlock_irq(&cp->lock);
}

static void cp_vlan_rx_kill_vid(struct net_device *dev, unsigned short vid)
{
	struct re_private *cp = dev->priv;

	spin_lock_irq(&cp->lock);
	cpw16(CpCmd, cpr16(CpCmd) & ~RxVlanOn);
	if (cp->vlgrp)
		cp->vlgrp->vlan_devices[vid] = NULL;
	spin_unlock_irq(&cp->lock);
}
#endif

/* Serial EEPROM section. */

/*  EEPROM_Ctrl bits. */
#define EE_SHIFT_CLK	0x04	/* EEPROM shift clock. */
#define EE_CS			0x08	/* EEPROM chip select. */
#define EE_DATA_WRITE	0x02	/* EEPROM chip data in. */
#define EE_WRITE_0		0x00
#define EE_WRITE_1		0x02
#define EE_DATA_READ	0x01	/* EEPROM chip data out. */
#define EE_ENB			(0x80 | EE_CS)

/* Delay between EEPROM clock transitions.
   No extra delay is needed with 33Mhz PCI, but 66Mhz may change this.
 */

#define eeprom_delay()	readl(ee_addr)

/* The EEPROM commands include the alway-set leading bit. */
#define EE_WRITE_CMD	(5)
#define EE_READ_CMD		(6)
#define EE_ERASE_CMD	(7)

#if 0
static int __devinit read_eeprom (void *ioaddr, int location, int addr_len)
{

	return 0;
}

static void __devexit cp_remove_one (struct pci_dev *pdev)
{
	struct net_device *dev = pci_get_drvdata(pdev);
	struct re_private *cp = dev->priv;

	if (!dev)
		BUG();
	unregister_netdev(dev);
	iounmap(cp->regs);
	kfree(dev);
}
#endif

#ifdef CONFIG_RTL8672_NAPI
static int re8670_poll (struct net_device *dev, int *budget)
{
	struct re_private *cp=netdev_priv(dev);
	return re8670_rx( cp, budget );
}
#endif //CONFIG_RTL8672_NAPI

struct pcs_struct{
	unsigned char page;
	unsigned char reg;	
	unsigned short value;
};
struct pcs_struct pcs_value[]={{1,16,0x7180},{1,17,0x77b8},{1,18,0x9004},{1,19,0x4400},{1,20,0x0b20},{1,21,0x041f},{1,22,0x035e},{1,23,0x0b23},{0xff,0xff,0xffff}};
void eth_int_phy_patch(void){
	int i;
	for(i=0;pcs_value[i].page!=0xff;i++){
		internal_select_page(pcs_value[i].page);
		internal_miiar_write(1,pcs_value[i].reg,pcs_value[i].value);
	}
	internal_select_page(0);
	
	//check ana and pll registers
	if(REG32(BSP_ANA1_CTRL)!=0x000003f3){
		REG32(BSP_ANA1_CTRL)=0x000003f3;
	}
	if(REG32(BSP_PLL_CTRL)!=0x4CC3106D){
		REG32(BSP_PLL_CTRL)=0x4CC3106D;
	}
	if(REG32(BSP_PLL2_CTRL)!=0x22D81C6D){
		REG32(BSP_PLL2_CTRL)=0x22D81C6D;
	}
	if(REG32(BSP_PLL3_CTRL)!=0x10B601BC){
		REG32(BSP_PLL3_CTRL)=0x10B601BC;
	}
	if(REG32(BSP_LDO_CTRL)!=0x00000006){
		REG32(BSP_LDO_CTRL)=0x00000006;
	}
	printk("eth phy patch done!\n");
}

int __init re8670_probe (void)
{
#ifdef CONFIG_EXT_SWITCH
	extern unsigned short chipver;
	char baseDevName[8];
#endif
#ifdef MODULE
	printk("%s", version);
#endif

	struct net_device *dev;
#ifndef CONFIG_RTL867X_PACKET_PROCESSOR
	struct re_private *cp;
#else
	struct mac_private *tp;
#endif
	int rc;
	//tylo, fpga
//	void *regs=(void*)0xB8018000 ;
	void *regs=(void*)ETHBASE;	

	unsigned i;
#ifndef CONFIG_RTL8672
	u32 sicr;
#endif

#ifndef MODULE
	static int version_printed;
	if (version_printed++ == 0)
		printk("%s", version);
#endif

#ifdef CONFIG_RTL867X_PACKET_PROCESSOR
	#if CONFIG_INT_PHY
	i=1;
	#else
	i=0;
	#endif
	dev = mac_dev[i] = alloc_etherdev(sizeof(struct mac_private));
	if (dev == NULL)
	{
		printk(KERN_ERR "alloc_dev: Unable to allocate device memory.\n");
		return -ENOMEM;
	}
	eth_net_dev=dev;
	tp = dev->priv;
	((struct mac_private *)(mac_dev[i]->priv))->tx_intf=i;
	dev->open = start_pp_nic_rx;
	dev->do_ioctl = re8670_ioctl;
	dev->stop=stop_pp_nic_rx;
	dev->set_mac_address = rtl8672_pp_set_mac_addr;				
	dev->hard_start_xmit = rtl8672_mac_vtx_start_xmit;
	dev->get_stats = rtl8672_mac_get_stats; //for counter
	dev->features |= NETIF_F_SG | NETIF_F_HW_CSUM | NETIF_F_HIGHDMA;
	#if CONFIG_INT_PHY
	dev->irq=BSP_PKT_NIC100_IRQ;
	#else
	dev->irq=PKT_NIC100MII_IRQ;
	#endif
	//tylo, struct conflict
	//mac_dev[i]->vlanid=9;
	dev->priv_flags = IFF_DOMAIN_ELAN;
#else
	dev = alloc_etherdev(sizeof(struct re_private));
	if (!dev)
		return -ENOMEM;
	/*linux-2.6.19*/
	//SET_MODULE_OWNER(dev);
	cp = dev->priv;
	//reDev = (struct re_private*)dev->priv;
	
	/* Jonah + for FASTROUTE */
	eth_net_dev=dev;
	
	cp->dev = dev;
	spin_lock_init (&cp->lock);

#if 0
	cp->mii_if.dev = dev;
	cp->mii_if.mdio_read = mdio_read;
	cp->mii_if.mdio_write = mdio_write;
	cp->mii_if.phy_id = CP_INTERNAL_PHY;
#endif
	
	dev->base_addr = (unsigned long) regs;
	cp->regs = regs;
	re8670_stop_hw(cp);
	/* read MAC address from EEPROM */
	for (i = 0; i < 3; i++)
		((u16 *) (dev->dev_addr))[i] = i;

	dev->open = re8670_open;
	dev->stop = re8670_close;
	dev->set_multicast_list = re8670_set_rx_mode;
	dev->hard_start_xmit = re8670_start_xmit;
	dev->get_stats = re8670_get_stats;
	// Kao, 2004/01/07, enable set mac address
	my_eth_mac_addr = dev->set_mac_address;
	dev->set_mac_address = re8670_set_mac_addr;
	dev->do_ioctl = re8670_ioctl;
	/*dev->change_mtu = re8670_change_mtu;*/
#if 1 
	dev->tx_timeout = re8670_tx_timeout;
	dev->watchdog_timeo = TX_TIMEOUT;
#endif
#ifdef CONFIG_RTL8672_NAPI
	dev->poll = re8670_poll;
	dev->weight = 64;
#endif //CONFIG_RTL8672_NAPI
#ifdef CP_TX_CHECKSUM
	dev->features |= NETIF_F_SG | NETIF_F_IP_CSUM;
#endif
#if CP_VLAN_TAG_USED
	dev->features |= NETIF_F_HW_VLAN_TX | NETIF_F_HW_VLAN_RX;
	dev->vlan_rx_register = cp_vlan_rx_register;
	dev->vlan_rx_kill_vid = cp_vlan_rx_kill_vid;
#endif
	//tylo, fpga
	#ifdef CONFIG_INT_PHY
	//dev->irq = NIC100_IRQ;	// internal phy
	/*linux-2.6.19*/
	dev->irq = BSP_NIC100_IRQ;	// internal phy
	#else
	//dev->irq=SW_IRQ;	// external phy
	/*linux-2.6.19*/
	dev->irq=BSP_SW_IRQ;	// external phy
	#endif

	// Kaohj

	dev->priv_flags = IFF_DOMAIN_ELAN;
#endif // of CONFIG_RTL867X_PACKET_PROCESSOR

	rc = register_netdev(dev);
	if (rc)
		goto err_out_iomap;

	printk (KERN_INFO "%s: %s at 0x%lx, "
		"%02x:%02x:%02x:%02x:%02x:%02x, "
		"IRQ %d\n",
		dev->name,
		"RTL-8139C+",
		dev->base_addr,
		dev->dev_addr[0], dev->dev_addr[1],
		dev->dev_addr[2], dev->dev_addr[3],
		dev->dev_addr[4], dev->dev_addr[5],
		dev->irq);


#ifndef CONFIG_RTL867X_PACKET_PROCESSOR
#ifndef CONFIG_RTL8672_NAPI
	memset(&cp->rx_tasklets, 0, sizeof(struct tasklet_struct));
	cp->rx_tasklets.func=(void (*)(unsigned long))re8670_rx;
	cp->rx_tasklets.data=(unsigned long)cp;
	eth_rx_tasklets = &cp->rx_tasklets;

#if defined (CONFIG_RTL8192CD) || defined (CONFIG_RTL8192CD_MODULE) || defined(CONFIG_RTL8681_PTM)
	eth_checktxdesc_tasklets.func = (void (*)(unsigned long))checkTXDesc;
#endif
#endif //CONFIG_RTL8672_NAPI
#else // of CONFIG_RTL867X_PACKET_PROCESSOR
	{
		extern struct tasklet_struct ppmac_rx_tasklets;
		eth_rx_tasklets = &ppmac_rx_tasklets;		
	}
#endif // of CONFIG_RTL867X_PACKET_PROCESSOR


#ifdef CONFIG_ETHWAN
	for (i=0; i<SW_PORT_NUM-1; i++)
	{
		dev = alloc_etherdev(0);
		if (!dev)
			return -ENOMEM;
		//SET_MODULE_OWNER(dev);//linux 2.6.19

		sprintf(dev->name, "eth0.%d", i+2);
		{
			int tmp_i;
			/* read MAC address from EEPROM */
			for (tmp_i = 0; tmp_i < 3; tmp_i++)
				((u16 *) (dev->dev_addr))[tmp_i] = tmp_i;
		}
		dev->base_addr = (unsigned long) regs;
		dev->open = re8670_sw_open;
		dev->stop = re8670_sw_close;
		dev->set_multicast_list = re8670_set_rx_mode;
		dev->hard_start_xmit = re8670_start_xmit;
		dev->get_stats = re8670_get_stats;
		my_eth_mac_addr = dev->set_mac_address;
		dev->set_mac_address = re8670_set_mac_addr;
		dev->do_ioctl = re8670_ioctl;
#ifdef CP_TX_CHECKSUM
		dev->features |= NETIF_F_SG | NETIF_F_IP_CSUM;
#endif
#if CP_VLAN_TAG_USED
		dev->features |= NETIF_F_HW_VLAN_TX | NETIF_F_HW_VLAN_RX;
		dev->vlan_rx_register = cp_vlan_rx_register;
		dev->vlan_rx_kill_vid = cp_vlan_rx_kill_vid;
#endif
		dev->priv = cp;
		rc = register_netdev(dev);
		if (rc)
			goto err_out_iomap;

		printk (KERN_INFO "%s: %s at 0x%lx, "
			"%02x:%02x:%02x:%02x:%02x:%02x\n",
			dev->name,
			"RTL-8672",
			dev->base_addr,
			dev->dev_addr[0], dev->dev_addr[1],
			dev->dev_addr[2], dev->dev_addr[3],
			dev->dev_addr[4], dev->dev_addr[5]);

		vnet_dev[i]=dev;
		vnet_dev_status[i]=0;
	}
#endif //CONFIG_ETHWAN

	
	// Kao, check if use internal phy
	//shlee 2.6 	#ifdef CONFIG_RTL8671
#ifdef CONFIG_RTL8672
printk("RTL8672 NIC100 Probing..\n");	//shlee 2.6
	//tylo, fpga
       // sicr = (*(volatile u32*)(0xB9C04000));	//tylo, 0xbd800000));
	if (1)  //7/1/05' hrchen, for 8671 MII spec
#else
	sicr = (*(volatile u32*)(0xbd800000));
	if (!(sicr & 0x10))
#endif
	{
		// use internal PHY, set and restart the MII register
		// reset
		internal_miiar_write(1, 0, 0x8000);
		// 10/100/H/F flow control
#ifdef CONFIG_ETHWAN
		internal_miiar_write(1, 4, 0x05e1);
#else
		internal_miiar_write(1, 4, 0x01e1);
#endif //CONFIG_ETHWAN

		// restart
		internal_miiar_write(1, 0, 0x1200);
	}
	//else{
#ifdef CONFIG_EXT_SWITCH
		//ql: auto search cpu port
#ifdef AUTO_SEARCH_CPUPORT
		while (RTL_R8(MSR) & 0x4)//fatal error if link fail
			mdelay(5);
		
		for (i=0; i<=4; i++)
		{
			miiar_write(i, 0, 0x0800);
			mdelay(5);
			if (RTL_R8(MSR) & 0x4)
				break;
		}
		{
			int cpuport;
			if (i>4)
				cpuport = 6;
			else
				cpuport = i;
			virt2phy[SW_PORT_NUM]=cpuport;
			printk("==================>cpu port is %d\n", cpuport);
			if (cpuport < SW_PORT_NUM) {
				for (i=cpuport; i<SW_PORT_NUM; i++)
					virt2phy[i]++;
			}
		}
#endif

#ifndef CONFIG_ETHWAN
	    for (i=0;i<=4;i++) {
		    // reset
			miiar_write(i, 0, 0x8000);
			// 10/100/H/F flow control
			miiar_write(i, 4, 0x01e1);
			// restart
			miiar_write(i, 0, 0x1200);
		};
#endif //CONFIG_ETHWAN
		
		// Kaohj, Setup the default 8305 configuration
		// port		vlanIdx		Vlan	VID	Egress
		//---------------------------------------------------------
		// 0		0		A	0	don't care
		// 1		1		B	1	don't care
		// 2		2		C	2	don't care
		// 3		3		D	3	don't care
		// 4		4		E	4	don't care
		rtl8305_info.vlan_en = 0;
		// remove tag for port 0,1,2,3 and don't care for port 4
		for (i=0;i<SW_PORT_NUM;i++) {
			rtl8305_info.phy[virt2phy[i]].egtag=TAG_DCARE;
			rtl8305_info.phy[virt2phy[i]].vlanIndex=i;
			rtl8305_info.vlan[i].vid=i;
			// vlan members contain all ports
			rtl8305_info.vlan[i].member=0x3f;
		}
		
		rtl8305_info.phy[virt2phy[4]].egtag=TAG_DCARE;
		rtl8305_info.phy[virt2phy[4]].vlanIndex=4;
		rtl8305_info.vlan[4].vid=4+1;
		rtl8305_info.vlan[4].member=0x3f;
		#ifdef CONFIG_ETHWAN
		/*reset wan port*/
		rtl8305_info.phy[virt2phy[3]].egtag=TAG_DCARE;
		rtl8305_info.phy[virt2phy[3]].vlanIndex=0;
		rtl8305_info.vlan[3].vid=0;
		rtl8305_info.vlan[3].member=0;
		#endif

#ifdef CONFIG_ETHWAN
		/* for HG251, init 8306n switch, vlan-group setting and create for wan port: nas0 */
		rtl8306M_CPU_VLAN_Init();
		rtl8306M_ACL_Qos_Init();
	 	rtl8306_setAsicCPUPort(virt2phy[SW_PORT_NUM], FALSE);

		//ql, patch for IGMP snooping report flood
		rtl8306_setAsicMulticastVlan(TRUE);
		rtl8306_setAsicStormFilterEnable(RTL8306_MULTICASTPKT, FALSE);
		rtl8306_setAsicIGMPMLDSnooping(RTL8306_IGMP, TRUE);
#else
#ifdef CONFIG_RE8306_API		
		rtl8306_init();
#endif
		set_8305(&rtl8305_info);
#endif //CONFIG_ETHWAN


		#if 0
		//disable 8305 flow control
		RTL_W32(MIIAR, 0x800401e1);
		do
		{
			//6/8/04' hrchen, compiler bugs will cause the code hang
			//for (i = 1; i < 1000; i++);
			mdelay(10);
		}
		while (RTL_R32(MIIAR) & 0x80000000);
		RTL_W32(MIIAR, 0x840401e1);
		do
		{
			//6/8/04' hrchen, compiler bugs will cause the code hang
			//for (i = 1; i < 1000; i++);
			mdelay(10);
		}
		while (RTL_R32(MIIAR) & 0x80000000);
		RTL_W32(MIIAR, 0x880401e1);
		do
		{
			//6/8/04' hrchen, compiler bugs will cause the code hang
			//for (i = 1; i < 1000; i++);
			mdelay(10);
		}
		while (RTL_R32(MIIAR) & 0x80000000);
		RTL_W32(MIIAR, 0x8c0401e1);
		do
		{
			//6/8/04' hrchen, compiler bugs will cause the code hang
			//for (i = 1; i < 1000; i++);
			mdelay(10);
		}
		while (RTL_R32(MIIAR) & 0x80000000);
#endif
#endif
	//}
#ifdef CONFIG_INT_PHY
	// andrew, applying CRC patch
	internal_miiar_write(1, 31, 0x3);
	internal_miiar_write(1, 9, 0x3871);
	internal_miiar_write(1, 31, 0x0);
#else
	//external PHY need this reset
	mdelay(5);
	miiar_write(5,0,0x1200);
#endif
	// Kaohj
#ifdef CONFIG_EXT_SWITCH
#ifdef CONFIG_ETHWAN
	strcpy(baseDevName, cp->dev->name);//use eth0, not eth0.x
#else
	strcpy(baseDevName, dev->name);
#endif //CONFIG_ETHWAN
	
	for (i=0; i<SW_PORT_NUM; i++) {
		dev = alloc_etherdev(0);
		if (!dev)
			return -ENOMEM;
		
		dev->open = re8670_open;
		dev->stop = re8670_close;
		dev->set_multicast_list = re8670_set_rx_mode;
		dev->hard_start_xmit = re8670_start_xmit;
		dev->get_stats = re8670_get_stats;
		dev->set_mac_address = re8670_set_mac_addr;
		dev->do_ioctl = re8670_ioctl;
		dev->priv_flags = IFF_DOMAIN_ELAN;
#ifdef CONFIG_RTL867X_PACKET_PROCESSOR
		dev->priv = tp;
#else
		dev->priv = cp;
#endif
		
		strcpy(dev->name, baseDevName);
		strcat(dev->name, "_sw%d");
		
		rc = register_netdev(dev);
		if (rc)
			goto err_out_iomap;
        	
		dev_sw[i] = dev;	// virtual sw-port
		printk (KERN_INFO "%s: %s at port %d\n",
			dev->name, "RTL-8305", i);
		// Kaohj
		//printk("sw-port %d, %x\n", i, dev);
	}
#endif	// CONFIG_EXT_SWITCH

#ifdef CONFIG_EXT_SWITCH
	initIGMPSnooping();
#endif
#ifdef CONFIG_RTL865X_ETH_PRIV_SKB
	init_priv_eth_skb_buf();
#endif		

#ifdef CONFIG_RTL867X_PACKET_PROCESSOR
	rc = request_irq(eth_net_dev->irq, mac_intHandler, IRQF_SHARED, eth_net_dev->name, eth_net_dev);
#endif

#ifdef CONFIG_EXT_SWITCH
	detect_switch_version();
#ifdef CONFIG_RE8306_API
	if(IS_RLE0315 || IS_6166)
		_rtl8306_QoS_parameter_patch();

	/* ql 20110407 START: patch for 10M IOL */
	if (0x6167 == chipver) {
		int enabled10M, enabled100M;
		for (i=0; i<=SW_PORT_NUM; i++)
		{
			rtl8306_port_eeeAbility_set(virt2phy[i], 0, 0);
			if (!rtl8306_port_eeeAbility_get(virt2phy[i], &enabled10M, &enabled100M))
				printk("phy %d 10M EEE %s 100M EEE %s\n", virt2phy[i], enabled10M?"enabled":"disabled", 
					enabled100M?"enabled":"disabled");
		}
	}
	/* ql 20110407 END*/
#endif
#endif

	if(IS_RLE0315 || IS_6166)
		eth_int_phy_patch();

#ifdef CONFIG_EXT_SWITCH
#ifdef CONFIG_RE8306_API
	//patch for the issue of int phy no rx when connected to intel82562.
	{
		rtl8306_setAsicPhyRegBit(0, 16, 11, 0, 1);
		rtl8306_setAsicPhyRegBit(4, 23, 5, 0, 1);
		rtl8306_getAsicPhyReg(4, 30, 0, &swtype);
		rtl8306_getAsicPhyReg(4, 26, 0, &SWChipversion);
		rtl8306_setAsicPhyRegBit(0, 16, 11, 0, 0);
		rtl8306_setAsicPhyRegBit(4, 23, 5, 0, 0);

		char *chipid[]={"6167a","6167b","6167c",""};
		//printk("\n\nID is : %x VER is : %s  \n\n",swtype,chipid[(SWChipversion>>13) & 0x3]);
		#ifdef CONFIG_ETHWAN
		printk("RE8670_MAX_ALLOC_RXSKB_NUM %d , MAX_PRESKB_POOL_SKB_NUM %d\n",RE8670_MAX_ALLOC_RXSKB_NUM,MAX_PRESKB_POOL_SKB_NUM);
		#endif
#if 0//close Fiber mode
		if (0x5988 == swtype) {
			init_timer(&sw_check_timer);
			sw_check_timer.function=(void (*)(unsigned long))sw_checkStatus;
		}
		else if ((0x6167 == swtype) && (IS_6166)) {//200 fiber mode
			printk("========init 200 fiber mode=========\n");
			unsigned short reg23, reg17, chk_cnt=0, regvalue;
			REG32(BSP_MISC_IO_DRIVING) = 0x00001D00;
			rtl8306_setAsicPhyReg(0, 0, 0, 0x2100);
			mdelay(5);
			/*QL 20110809 START: patch for 6167 rx crc error*/
			miiar_write(5, 16, 0x0001);
			miiar_write(0, 31, 0x4);
			miiar_read(0, 25, &regvalue);
			/*reset for EEE programmable finite state machine*/
			regvalue |= (1<<6);
			regvalue &= (~(1<<4));
			miiar_write(0, 25, regvalue);
			mdelay(10);
			miiar_read(0, 27, &regvalue);
			regvalue &= 0xFFFE;
			miiar_write(0, 27, regvalue);
			mdelay(10);
			miiar_write(0, 31 , 0);
			miiar_write(5, 16, 0);
			/*QL 20110809 END*/

			rtl8306_setAsicPhyReg(6, 25, 0, 0xD001);
			mdelay(5);
			if (((SWChipversion>>13)&0x3) == 1) {
				rtl8306_setAsicPhyReg(0, 19, 0, 0x7FFB);
				mdelay(5);
			}
			rtl8306_setAsicPhyReg(0, 16, 0, 0x0FFA);
			mdelay(5);
			rtl8306_setAsicPhyReg(4, 23, 0, 0x0010);
			mdelay(5);
			RTL_W32(MIIAR, 0x84002100);
			mdelay(5);
			RTL_W32(MIIAR, 0x841CC0E0);
			mdelay(5);
			RTL_W32(MIIAR, 0x8413DE20);
			mdelay(10);
			RTL_W32(MIIAR, 0x8413DE10);
			mdelay(5);
			miiar_write(0, 16, 0x0FFA);
			mdelay(5);
			miiar_write(5, 16, 0x0001);
			mdelay(5);
			miiar_write(0, 31, 0x0000);
			mdelay(5);
			miiar_write(0, 19, 0xDE20);
			mdelay(10);
			miiar_write(0, 19, 0xDE10);
			mdelay(5);
			/* ql 20110402 start: patch for 6166 rx fail. */
			mdelay(10);
			internal_miiar_WriteBit(1, 21, 15, 1);
			mdelay(5);
			internal_miiar_WriteBit(1, 21, 15, 0);
			mdelay(5);
			/*20110418 new patch for 6166*/
RESET_8672:
			if (chk_cnt==0) {
				miiar_write(0, 19, 0xDE11);
				mdelay(5);
				miiar_write(0, 19, 0xDE10);
				mdelay(5);
				internal_miiar_WriteBit(1, 21, 15, 1);
				mdelay(5);
				internal_miiar_WriteBit(1, 21, 15, 0);
				mdelay(5);
			}
			internal_miiar_read(1, 23, &reg23);
			internal_miiar_write(1, 23, 0x1003);
			internal_miiar_read(1, 17, &reg17);
			regvalue = (reg17&0xFFF0)|0x6;
			internal_miiar_write(1, 17, regvalue);
			chk_cnt = 5;
			while(chk_cnt--) {
				internal_miiar_read(1, 29, &regvalue);
				if (regvalue&0x01C0)
					break;
			}
			internal_miiar_write(1, 17, reg17);
			internal_miiar_write(1, 23, reg23);
			if (chk_cnt==0) {
				goto RESET_8672;
			}
			/*20110418 end*/
			miiar_read(0, 21, &regvalue);
			regvalue |= (1<<15);
			miiar_write(0, 21, regvalue);
			mdelay(5);
			regvalue &= (~(1<<15));
			miiar_write(0, 21, regvalue);
			/* ql 20110402 end */
			#if 0//QL 20110809 remove this patch.
			/* ql 20110414 start: patch for 6167 rx fail. */
			miiar_write(0, 31, 0x4);
			miiar_read(0, 25, &regvalue);
			/*disable EEE programmable finite state machine*/
			regvalue &= (~(1<<4));
			miiar_write(0, 25, regvalue);
			/*reset for EEE programmable finite state machine*/
			regvalue |= (1<<6);
			miiar_write(0, 25, regvalue);
			mdelay(5);
			/* ql 20110414 end */
			#endif
			miiar_write(5, 16, 0x0000);
			mdelay(5);
			miiar_write(0, 16, 0x03FA);
		}
#endif
	}
#endif

	
	#ifdef CONFIG_RTL8672_PWR_SAVE
	init_timer(&mgt.mgt_timer);
	mgt.mgt_timer.data		= &mgt;
	mgt.mgt_timer.function	= mgt_function;
	#endif //PWR_SAVE
#endif


#ifdef CONFIG_ETHWAN
#if 0
#ifdef CONFIG_RTL8672_ETHSKB_CONTROL_POOL
		init_eth_pre_ethpool();
#endif //CONFIG_RTL8672_ETHSKB_CONTROL_POOL
#endif

		//Create a virtual device nas for wan port
		dev = alloc_etherdev(0);
		if (!dev)
			return -ENOMEM;
		
		dev->base_addr = (unsigned long) regs;//base_addr=0 for eth0_swx
		dev->open = re8670_sw_open;
		dev->stop = re8670_sw_close;
		dev->set_multicast_list = re8670_set_rx_mode;
		dev->hard_start_xmit = re8670_start_xmit;
		dev->get_stats = re8670_get_stats;
		dev->set_mac_address = re8670_set_mac_addr;
		dev->do_ioctl = re8670_ioctl;
		dev->priv_flags = IFF_DOMAIN_WAN;

		for (i = 0; i < 3; i++)
			((u16 *) (dev->dev_addr))[i] = i+2;

		dev->priv =cp;
		memcpy(dev->name, "nas", 3);
		if (register_netdev(dev)){
			printk("Register %s FAILED!\n", dev->name);
			goto err_out_iomap;
		}

		printk (KERN_INFO "%s: %s at 0x%lx, "
			"%02x:%02x:%02x:%02x:%02x:%02x\n",
			dev->name,
			"RTL-8672",
			dev->base_addr,
			dev->dev_addr[0], dev->dev_addr[1],
			dev->dev_addr[2], dev->dev_addr[3],
			dev->dev_addr[4], dev->dev_addr[5]);

		vnet_dev[SW_PORT_NUM-1]=dev;		
		vnet_dev_status[SW_PORT_NUM-1]=0;

		//ql patch for phy ping fail
		mdelay(5);
		internal_miiar_write(1, 0, 0x2100);
		mdelay(5);
		miiar_write(0, 0, 0x2100);
		mdelay(5);

		//create proc
		Port_Status_Proc_File= create_proc_read_entry(PROCFS_NAME,
                          0444, NULL,
                          port_status_proc_read,
                          NULL);

		if (Port_Status_Proc_File == NULL) {
			remove_proc_entry(PROCFS_NAME, NULL);
			printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
					PROCFS_NAME);
			return -ENOMEM;
		}
		printk(KERN_INFO "/proc/%s created\n", PROCFS_NAME);
#endif //CONFIG_ETHWAN

#ifdef CONFIG_EXT_SWITCH
#ifdef CONFIG_RE8306_API
	/* patch for 100M IOL*/
	if (0x6167 == chipver) {
		unsigned short value;
		miiar_read(5, 16, &value);
		miiar_write(5, 16, 0x1);
		miiar_write(0, 31, 0x1);
		miiar_write(0, 20, 0x655);
		miiar_write(5, 16, value);
	}
#endif
#endif

#if 0
	do {
		static struct proc_dir_entry *pdir;
		if ((pdir= create_proc_entry("nic_info", 0644, NULL)) != NULL) {
			pdir->read_proc = nic_proc_read;
			pdir->write_proc = nic_proc_write;

		}
	} while (0);
#endif //#if 0
	
#ifdef NIC_USE_RGMII_TO_EXTCPU_MAC
	REG32(BSP_PCCR) |= (BSP_PCCR_PHYIF_0| //gmac_sel_phyif_0
					(7 << BSP_PCCR_SHF_CLK_DP)|
					(7 << BSP_PCCR_SHF_CLK_DN)|
					(7 << BSP_PCCR_SHF_DATA_DP)|
					(7 << BSP_PCCR_SHF_DATA_DN));
	
	printk( "%s: GMAC uses phyif0\n", __FUNCTION__ );
#endif /*NIC_USE_RGMII_TO_EXTCPU_MAC*/
	
	/*
	 * Looks like this is necessary to deal with on all architectures,
	 * even this %$#%$# N440BX Intel based thing doesn't get it right.
	 * Ie. having two NICs in the machine, one will have the cache
	 * line set at boot time, the other will not.
	 */
	return 0;

err_out_iomap:
	iounmap(regs);
	kfree(dev);
	return -1 ;


	
}

static void __exit re8670_exit (void)
{
}
module_init(re8670_probe);
module_exit(re8670_exit);


// Kaohj
/*
 *	pvid		group(vlan)
 *	0 (reserved for local-out skb)
 *	x(SW_PORT_NUM+1)0 (default)
 *	x+1		1
 *	x+2		2
 *	x+3		3
 *	x+4		4
 */
int get_vid(int pvid)
{
	if (pvid>=SW_PORT_NUM+1 && pvid<=(SW_PORT_NUM+1+4))
		return rtl8305_info.vlan[pvid].vid;
	return 0;
}

int get_pvid(int vid)
{
	int i;
	
	for (i=SW_PORT_NUM+1; i<=(SW_PORT_NUM+1+4); i++) {
		if (rtl8305_info.vlan[i].vid == vid)
			return i;
	}
	return 0;
}

// Kaohj
/*
 * Translate bitmap of virtual port to that of phy port bit (corresponding bit)
 * Bit-map:  bit5  |  bit4  |  bit3  |  bit2  |  bit1  |  bit0
 *           wlan  | device |  lan3  |  lan2  |  lan1  |  lan0
 */
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
