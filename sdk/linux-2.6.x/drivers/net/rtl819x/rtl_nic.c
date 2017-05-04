/*
 * Copyright c                Realtek Semiconductor Corporation, 2003
 * All rights reserved.                                                    
 * 
 * $Header: /usr/local/dslrepos/linux-2.6.30/drivers/net/rtl819x/rtl_nic.c,v 1.150 2012/09/28 04:18:26 ikevin362 Exp $
 *
 * $Author: ikevin362 $
 *
 * Abstract: Pure L2 NIC driver, without RTL865X's advanced L3/4 features.
 *
 *   re865x_nic.c: NIC driver for the RealTek 865* 
 *
 */

#define DRV_RELDATE		"Mar 25, 2004"
#include <linux/config.h>
#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/compiler.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
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
#include <linux/signal.h>
#include <linux/proc_fs.h>
#include <linux/time.h>
#include <linux/rtc.h>
#include <bsp/bspchip.h>
#include <linux/timer.h>
#if defined(CONFIG_RTK_VLAN_SUPPORT)
#include <net/rtl/rtk_vlan.h>
#endif

#if	defined(CONFIG_RTL8196_RTL8366) && defined(CONFIG_RTL_IGMP_SNOOPING)
#undef	CONFIG_RTL_IGMP_SNOOPING
#endif

#include "version.h"
#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>

#include "AsicDriver/asicRegs.h"
#include "AsicDriver/rtl865x_asicCom.h"
#include "AsicDriver/rtl865x_asicL2.h"
#ifdef CONFIG_RTL_LAYERED_ASIC_DRIVER_L3
#include "AsicDriver/rtl865x_asicL3.h"
#endif

#include "common/mbuf.h"
#include <net/rtl/rtl_queue.h>
#include "common/rtl_errno.h"
#include "rtl865xc_swNic.h"

/*common*/
#include "common/rtl865x_vlan.h"
#include <net/rtl/rtl865x_netif.h>
#include "common/rtl865x_netif_local.h"

/*l2*/
#if defined(CONFIG_RTL_LAYERED_DRIVER_L2) || defined(CONFIG_RTL_MULTI_LAN_DEV)
#include "l2Driver/rtl865x_fdb.h"
#else
#include <net/rtl/rtl865x_fdb_api.h>
#endif

/*l3*/
#ifdef CONFIG_RTL_LAYERED_DRIVER_L3
#include "l3Driver/rtl865x_ip.h"
#include "l3Driver/rtl865x_nexthop.h"
#include <net/rtl/rtl865x_ppp.h>
#include "l3Driver/rtl865x_ppp_local.h"
#include "l3Driver/rtl865x_route.h"
#include "l3Driver/rtl865x_arp.h"
#include <net/rtl/rtl865x_nat.h>
#endif

/*l4*/
#ifdef	CONFIG_RTL865X_ROMEPERF
#include "romeperf.h"
#endif
#include <net/rtl/rtl_nic.h>

#if defined(CONFIG_PROC_FS) && defined(CONFIG_NET_SCHED) && defined(CONFIG_RTL_LAYERED_DRIVER)
#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
#include <net/rtl/rtl865x_outputQueue.h>
#endif
#endif

#ifdef CONFIG_RTL_STP
#include "l2Driver/rtl865x_stp.h"
#endif

#if defined(CONFIG_RTL_HW_STP)
#include <net/rtl/rtk_stp.h>
#endif
#include <linux/if_smux.h>
#ifdef CONFIG_RTL_MULTI_ETH_WAN

//#if defined(CONFIG_RTL8676_Dynamic_ACL)
//#include <linux/netfilter_ipv4/ip_tables.h>	
//#endif
#endif

#ifdef CONFIG_RTL867X_VLAN_MAPPING
#include "../re_vlan.h"
#endif

#if defined (CONFIG_RTL_IGMP_SNOOPING)
#include <net/rtl/rtl865x_igmpsnooping.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#if defined (CONFIG_RTL_MLD_SNOOPING)
#include <linux/ipv6.h>
//int mldSnoopEnabled;
extern int mldSnoopEnabled;
#endif
uint32 nicIgmpModuleIndex=0xFFFFFFFF;
extern int  igmpsnoopenabled;
extern uint32 brIgmpModuleIndex;
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
extern struct net_bridge *bridge0;
extern uint32 br0SwFwdPortMask;
#endif
#endif

static unsigned int curLinkPortMask=0;
static unsigned int newLinkPortMask=0;
extern int32 global_probe_extPhy;

#define SET_MODULE_OWNER(dev) do { } while (0)

#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
#include <net/rtl/rtl865x_multicast.h>
#endif

#ifdef CONFIG_RTL8196_RTL8366
#include "RTL8366RB_DRIVER/gpio.h"
#include "RTL8366RB_DRIVER/rtl8366rb_apiBasic.h"
#endif

#if (defined(CONFIG_RTL_CUSTOM_PASSTHRU) && !defined(CONFIG_RTL8196_RTL8366)) 
static int oldStatus = 0;
static struct proc_dir_entry *res=NULL;
static char passThru_flag[1];
#endif

#if (defined(CONFIG_RTL_8198))
static struct proc_dir_entry *phyTest_entry=NULL;
#endif

#if defined(CONFIG_RTL_ETH_PRIV_SKB) && (defined(CONFIG_NET_WIRELESS_AGN) || defined(CONFIG_NET_WIRELESS_AG) || defined(CONFIG_WIRELESS))
#include <net/dst.h>
#endif

#if 0
#define DEBUG_ERR printk
#else
#define DEBUG_ERR(format, args...)
#endif

#if defined (CONFIG_RTL_LOCAL_PUBLIC)
#include <net/rtl/rtl865x_localPublic.h>
#endif

#include "../ethctl_implement.h"

#include "rtl.h"

/*Port0 uses RGMII interface to connect to external CPU's MAC*/
//#define PORT0_USE_RGMII_TO_EXTCPU_MAC

//#define DBG_VlanConfig
#ifdef DBG_VlanConfig
#define DBG_VlanConfig_PRK printk
#else
#define DBG_VlanConfig_PRK(format, args...)
#endif

//#define DBG_MULTIWAN_API
#ifdef DBG_MULTIWAN_API
#define DBG_MULTIWAN_API_PRK printk
#else
#define DBG_MULTIWAN_API_PRK(format, args...)
#endif

static int32 __865X_Config;
#if defined(DYNAMIC_ADJUST_TASKLET) || defined(BR_SHORTCUT)
#if 0
static int eth_flag=12; // 0 dynamic tasklet, 1 - disable tasklet, 2 - always tasklet , bit2 - bridge shortcut enabled
#endif
#endif
#if defined(BR_SHORTCUT)
__DRAM_FWD  unsigned char cached_eth_addr[ETHER_ADDR_LEN];
EXPORT_SYMBOL(cached_eth_addr);
__DRAM_FWD  struct net_device *cached_dev;
EXPORT_SYMBOL(cached_dev);
#if defined(CONFIG_WIRELESS_LAN_MODULE)
struct net_device* (*wirelessnet_hook_shortcut)(unsigned char *da) = NULL;
EXPORT_SYMBOL(wirelessnet_hook_shortcut);
#endif //CONFIG_WIRELESS_LAN_MODULE
#endif

#if defined(CONFIG_WIRELESS_LAN_MODULE)
int (*wirelessnet_hook)(void) = NULL;
EXPORT_SYMBOL(wirelessnet_hook);
#endif //CONFIG_WIRELESS_LAN_MODULE

#if defined (CONFIG_RTL_UNKOWN_UNICAST_CONTROL)
static rtlMacRecord	macRecord[RTL_MAC_RECORD_NUM];
static uint32	macRecordIdx;
static uint8	lanIfName[NETIF_NUMBER];
static void	rtl_unkownUnicastUpdate(uint8 *mac);
static void rtl_unkownUnicastTimer(unsigned long data);
#endif

#if defined(DYNAMIC_ADJUST_TASKLET) || defined(CONFIG_RTL8186_TR) || defined(RTL8196C_EEE_MAC) || defined(CONFIG_RTL_8198_ESD)
static void one_sec_timer(unsigned long task_priv);
#endif

//#define CONFIG_RTL_LINKSTATE
#if defined(CONFIG_RTL_LINKSTATE)
static struct timer_list s_timer;
static void linkup_time_handle(unsigned long arg);
static int32 initPortStateCtrl(void);
static void  exitPortStateCtrl(void);
#endif
#if defined(CONFIG_RTL_ETH_PRIV_SKB)
static struct sk_buff *dev_alloc_skb_priv_eth(unsigned int size);
static void init_priv_eth_skb_buf(void);
#endif

__DRAM_FWD static struct ring_que	rx_skb_queue;
int skb_num=0;

int32 rtl865x_init(void);
int32 rtl865x_config(struct rtl865x_vlanConfig vlanconfig[]);

/* These identify the driver base version and may not be removed. */
MODULE_DESCRIPTION("RealTek RTL-8650 series 10/100 Ethernet driver");
MODULE_LICENSE("GPL");

#ifdef CONFIG_DEFAULTS_KERNEL_2_6
static char* multicast_filter_limit = "maximum number of filtered multicast addresses";
module_param (multicast_filter_limit,charp, S_IRUGO);
#else
/* Maximum number of multicast addresses to filter (vs. Rx-all-multicast).
   The RTL chips use a 64 element hash table based on the Ethernet CRC.  */
MODULE_PARM (multicast_filter_limit, "i");
MODULE_PARM_DESC (multicast_filter_limit, "maximum number of filtered multicast addresses");
#endif

#define DRV_NAME		"re865x_nic"
#define PFX			DRV_NAME ": "
#define DRV_VERSION		"0.1"
#define TX_TIMEOUT		(10*HZ)
#define BDINFO_ADDR 0xbe3fc000

#if defined (CONFIG_RTL_PHY_POWER_CTRL)
static int32 rtl865x_initPhyPowerCtrl(void);
#endif

__DRAM_GEN static int rtl_rxTxDoneCnt = 0;
__DRAM_GEN static atomic_t rtl_devOpened;


__MIPS16 __IRAM_GEN void rtl_rxSetTxDone(int enable)
{
	if (unlikely(rtl_devOpened.counter==0))
		return;
	
	if (FALSE==enable)
	{
		rtl_rxTxDoneCnt--;
		if (rtl_rxTxDoneCnt==-1)
			REG32(CPUIIMR) &= ~(TX_ALL_DONE_IE_ALL);
	}
	else
	{
		rtl_rxTxDoneCnt++;
		if (rtl_rxTxDoneCnt==0)
			REG32(CPUIIMR) |= (TX_ALL_DONE_IE_ALL);
	}

	REG32(CPUIISR) = (MBUF_DESC_RUNOUT_IP_ALL|PKTHDR_DESC_RUNOUT_IP_ALL);
}

//#define NEXT_DEV(cp)			(cp->dev_next ? cp->dev_next : cp->dev_prev)
//#define NEXT_CP(cp)			((struct dev_priv *)((NEXT_DEV(cp))->priv))
//#define IS_FIRST_DEV(cp)	(NEXT_CP(cp)->opened ? 0 : 1)
//#define GET_IRQ_OWNER(cp) (cp->irq_owner ? cp->dev : NEXT_DEV(cp))

#define MAX_PORT_NUM 9

static unsigned int rxRingSize[RTL865X_SWNIC_RXRING_HW_PKTDESC] = 
	{NUM_RX_PKTHDR_DESC, 
	NUM_RX_PKTHDR_DESC1, 
	NUM_RX_PKTHDR_DESC2, 
	NUM_RX_PKTHDR_DESC3, 
	NUM_RX_PKTHDR_DESC4,
	NUM_RX_PKTHDR_DESC5};

#ifdef CONFIG_RTL_8196D
static unsigned int txRingSize[RTL865X_SWNIC_TXRING_HW_PKTDESC] = 
	{NUM_TX_PKTHDR_DESC, 
	NUM_TX_PKTHDR_DESC1,
	NUM_TX_PKTHDR_DESC2,
	NUM_TX_PKTHDR_DESC3
	};
#else
static unsigned int txRingSize[RTL865X_SWNIC_TXRING_HW_PKTDESC] = 
	{NUM_TX_PKTHDR_DESC, 
	NUM_TX_PKTHDR_DESC1};
#endif

#if defined (CONFIG_RTL_MULTI_LAN_DEV)||defined(CONFIG_RTK_VLAN_SUPPORT)
static  struct rtl865x_vlanConfig packedVlanConfig[NETIF_SW_NUMBER];
#endif

/*
linux protocol stack netif VS rtl819x driver network interface
the name of ps netif maybe different with driver.
*/
static ps_drv_netif_mapping_t ps_drv_netif_mapping[NETIF_SW_NUMBER];

static unsigned int pvid_per_port[RTL8651_PORT_NUMBER+3]={[0 ... RTL8651_PORT_NUMBER+2]=RTL_LANVLANID};

static struct rtl865x_vlanConfig vlanconfig[] = {
/*      	ifName  W/L      If type		VID	 FID	   Member Port	UntagSet		mtu		MAC Addr	is_slave								*/
/*		=====  ===   =======	===	 ===   =========   =======	====	====================================	*/

#ifdef CONFIG_BRIDGE
#if defined (CONFIG_RTL_MULTI_LAN_DEV)
	{ RTL_DRV_LAN_P0_NETIF_NAME,	0,	IF_ETHER,	RTL_LANVLANID,		RTL_LAN_FID,		RTL_LANPORT_MASK_4,		RTL_LANPORT_MASK_4,		1500,	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x90 } }, 0, 0	},
#ifdef CONFIG_ETHWAN
#ifndef CONFIG_RTL_HW_PURE_SWITCH
#ifndef CONFIG_RTL_MULTI_ETH_WAN
	{ RTL_DRV_LAN_P4_NETIF_NAME,	1,	IF_ETHER,	RTL_WANVLANID,		RTL_WAN_FID,		RTL_WANPORT_MASK,		RTL_WANPORT_MASK,		1500,	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x91 } }, 0, 0	},
#else
	{ RTL_DRV_LAN_P4_NETIF_NAME,	1,	IF_ETHER,	0,					RTL_WAN_FID,		RTL_WANPORT_MASK,		RTL_WANPORT_MASK,		1500,	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x90 } }, 0, 0	},
#endif
#endif
#endif /* CONFIG_ETHWAN */
	{ RTL_DRV_LAN_P1_NETIF_NAME,	0,	IF_ETHER,	RTL_LANVLANID,		RTL_LAN_FID,		RTL_LANPORT_MASK_3,		RTL_LANPORT_MASK_3,		1500,	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x90 } }, 0, 0	},
	{ RTL_DRV_LAN_P2_NETIF_NAME,	0,	IF_ETHER,	RTL_LANVLANID,		RTL_LAN_FID,		RTL_LANPORT_MASK_2,		RTL_LANPORT_MASK_2,		1500,	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x90 } }, 0, 0	},
  	{ RTL_DRV_LAN_P3_NETIF_NAME,	0,	IF_ETHER,	RTL_LANVLANID,		RTL_LAN_FID, 		RTL_LANPORT_MASK_1,		RTL_LANPORT_MASK_1,		1500,	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x90 } }, 0, 0	},
#else	/*CONFIG_RTL_MULTI_LAN_DEV*/
#if defined(CONFIG_RTK_VLAN_SUPPORT)
	{ 	RTL_DRV_LAN_NETIF_NAME,	 0,   IF_ETHER, 	RTL_LANVLANID, 	   	RTL_LAN_FID, 	RTL_LANPORT_MASK_4, 	RTL_LANPORT_MASK_4,		1500, 	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x90 } }, 0, 0	},
#else
	{ 	RTL_DRV_LAN_NETIF_NAME,	 0,   IF_ETHER, 	RTL_LANVLANID, 	   	RTL_LAN_FID, 	RTL_LANPORT_MASK, 		RTL_LANPORT_MASK,		1500, 	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x90 } }, 0, 0	},
#endif
#if 0 //closed by Kevin, we need not WAN device when disable ETHWAN(disable CONFIG_RTL_MULTI_LAN_DEV)
#if defined(CONFIG_RTL_PUBLIC_SSID)
	{	RTL_GW_WAN_DEVICE_NAME,	 1,   IF_ETHER,	RTL_WANVLANID,	   RTL_WAN_FID,		RTL_WANPORT_MASK,		RTL_WANPORT_MASK,		1500,	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x91 } }, 0, 0	},
#else
	{	RTL_DRV_WAN0_NETIF_NAME,	 1,   IF_ETHER,	RTL_WANVLANID,		RTL_WAN_FID,	RTL_WANPORT_MASK,		RTL_WANPORT_MASK,		1500,	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x91 } }, 0, 0	},
#endif /* CONFIG_RTL_PUBLIC_SSID */
#endif
#if defined(CONFIG_RTK_VLAN_SUPPORT)
	{	RTL_DRV_LAN_P1_NETIF_NAME,	0,   IF_ETHER, 	RTL_LANVLANID,	RTL_LAN_FID, 	RTL_LANPORT_MASK_3, 	RTL_LANPORT_MASK_3,		1500, 	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x92 } }, 0, 0	},
	{	RTL_DRV_LAN_P2_NETIF_NAME, 	0,   IF_ETHER, 	RTL_LANVLANID,	RTL_LAN_FID, 	RTL_LANPORT_MASK_2, 	RTL_LANPORT_MASK_2,		1500, 	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x93 } }, 0, 0	},
	{	RTL_DRV_LAN_P3_NETIF_NAME, 	0,   IF_ETHER, 	RTL_LANVLANID,	RTL_LAN_FID, 	RTL_LANPORT_MASK_1, 	RTL_LANPORT_MASK_1,		1500, 	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x94 } }, 0, 0	},
#ifdef CONFIG_8198_PORT5_GMII
	{	RTL_DRV_LAN_P5_NETIF_NAME, 	0,	 IF_ETHER,	RTL_LANVLANID,	RTL_LAN_FID,		RTL_LANPORT_MASK_5, 	RTL_LANPORT_MASK_5, 	1500,	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x95 } }, 0, 0	},
#endif //CONFIG_8198_PORT5_GMII
#endif
#endif
#else	/*CONFIG_BRIDGE*/
#if defined (CONFIG_RTL_MULTI_LAN_DEV)
	{ 	RTL_DRV_LAN_P0_NETIF_NAME,	0,	IF_ETHER,	RTL_LANVLANID,		RTL_LAN_FID,		RTL_LANPORT_MASK_4,		RTL_LANPORT_MASK_4,		1500,	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x90 } }, 0, 0	},
	{	RTL_DRV_LAN_P4_NETIF_NAME,	1,	IF_ETHER,	RTL_WANVLANID,		RTL_WAN_FID,	RTL_WANPORT_MASK,		RTL_WANPORT_MASK,		1500,	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x91 } }, 0, 0	},
	{	RTL_DRV_LAN_P1_NETIF_NAME,	0,	IF_ETHER,	RTL_LANVLANID,		RTL_LAN_FID,		RTL_LANPORT_MASK_3,		RTL_LANPORT_MASK_3,		1500,	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x90 } }, 0, 0	},
	{	RTL_DRV_LAN_P2_NETIF_NAME,	0,	IF_ETHER,	RTL_LANVLANID,		RTL_LAN_FID,		RTL_LANPORT_MASK_2,		RTL_LANPORT_MASK_2,		1500,	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x90 } }, 0, 0	},
	{	RTL_DRV_LAN_P3_NETIF_NAME,	0,	IF_ETHER,	RTL_LANVLANID,		RTL_LAN_FID, 	RTL_LANPORT_MASK_1,		RTL_LANPORT_MASK_1,		1500,	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x90 } }, 0, 0	},
#else /*CONFIG_RTL_MULTI_LAN_DEV*/
	{ 	RTL_DRV_LAN_NETIF_NAME,	 0,   IF_ETHER, 	RTL_LANVLANID, 	   	RTL_LAN_FID, 	RTL_LANPORT_MASK, 		RTL_LANPORT_MASK,		1500, 	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x90 } }, 0, 0	},
	{	RTL_DRV_WAN0_NETIF_NAME,	 1,   IF_ETHER,	RTL_WANVLANID,	   	RTL_WAN_FID,	RTL_WANPORT_MASK,		RTL_WANPORT_MASK,		1500,	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x91 } }, 0, 0	},
#if defined(CONFIG_RTK_VLAN_SUPPORT)
	{	RTL_DRV_LAN_P1_NETIF_NAME,	0,   IF_ETHER, 	RTL_LANVLANID,	RTL_LAN_FID,		RTL_LANPORT_MASK_3, 	RTL_LANPORT_MASK_3,		1500, 	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x92 } }, 0, 0	},
	{	RTL_DRV_LAN_P2_NETIF_NAME, 	0,   IF_ETHER, 	RTL_LANVLANID,	RTL_LAN_FID,		RTL_LANPORT_MASK_2, 	RTL_LANPORT_MASK_2,		1500, 	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x93 } }, 0, 0	},
	{	RTL_DRV_LAN_P3_NETIF_NAME, 	0,   IF_ETHER, 	RTL_LANVLANID, 	RTL_LAN_FID,		RTL_LANPORT_MASK_1, 	RTL_LANPORT_MASK_1,		1500, 	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x94 } }, 0, 0	},
#endif
#endif
#endif
#ifdef CONFIG_ETHWAN
#ifndef CONFIG_RTL_MULTI_ETH_WAN
	{	RTL_DRV_PPP_NETIF_NAME, 1,   IF_PPPOE,    	RTL_WANVLANID,		RTL_WAN_FID,    	RTL_WANPORT_MASK,         	RTL_WANPORT_MASK,     	1500, { { 0x00, 0x12, 0x34, 0x56, 0x78, 0x91 } }, 1, 0 },
#else
	/*below 3 entry reserved for wan interface vlan setting. */
	{	RTL_DRV_WAN1_NETIF_NAME, 1,   IF_ETHER,    	0,		RTL_WAN_FID,    	RTL_WANPORT_MASK,         	RTL_WANPORT_MASK,     	1500, { { 0x00, 0x12, 0x34, 0x56, 0x78, 0x91 } }, 0, 0 },
	{	RTL_DRV_WAN2_NETIF_NAME, 1,   IF_ETHER,    	0,		RTL_WAN_FID,    	RTL_WANPORT_MASK,         	RTL_WANPORT_MASK,     	1500, { { 0x00, 0x12, 0x34, 0x56, 0x78, 0x91 } }, 0, 0 },
	{	RTL_DRV_WAN3_NETIF_NAME, 1,   IF_ETHER,    	0,		RTL_WAN_FID,    	RTL_WANPORT_MASK,         	RTL_WANPORT_MASK,     	1500, { { 0x00, 0x12, 0x34, 0x56, 0x78, 0x91 } }, 0, 0 },
	{	RTL_DRV_WAN4_NETIF_NAME, 1,   IF_ETHER,    	0,		RTL_WAN_FID,    	RTL_WANPORT_MASK,         	RTL_WANPORT_MASK,     	1500, { { 0x00, 0x12, 0x34, 0x56, 0x78, 0x91 } }, 0, 0 },
	{	RTL_DRV_WAN5_NETIF_NAME, 1,   IF_ETHER,    	0,		RTL_WAN_FID,    	RTL_WANPORT_MASK,         	RTL_WANPORT_MASK,     	1500, { { 0x00, 0x12, 0x34, 0x56, 0x78, 0x91 } }, 0, 0 },
	{	RTL_DRV_WAN6_NETIF_NAME, 1,   IF_ETHER,    	0,		RTL_WAN_FID,    	RTL_WANPORT_MASK,         	RTL_WANPORT_MASK,     	1500, { { 0x00, 0x12, 0x34, 0x56, 0x78, 0x91 } }, 0, 0 },
	/*below 5 entry reserved for pppoe session supporting. */
	{	RTL_DRV_PPP0_NETIF_NAME, 1,   IF_PPPOE,    	0,		RTL_WAN_FID,    	RTL_WANPORT_MASK,         	RTL_WANPORT_MASK,     	1500, { { 0x00, 0x12, 0x34, 0x56, 0x78, 0x91 } }, 1, 0 },
	{	RTL_DRV_PPP1_NETIF_NAME, 1,   IF_PPPOE,    	0,		RTL_WAN_FID,    	RTL_WANPORT_MASK,         	RTL_WANPORT_MASK,     	1500, { { 0x00, 0x12, 0x34, 0x56, 0x78, 0x91 } }, 1, 0 },
	{	RTL_DRV_PPP2_NETIF_NAME, 1,   IF_PPPOE,    	0,		RTL_WAN_FID,    	RTL_WANPORT_MASK,         	RTL_WANPORT_MASK,     	1500, { { 0x00, 0x12, 0x34, 0x56, 0x78, 0x91 } }, 1, 0 },
	{	RTL_DRV_PPP3_NETIF_NAME, 1,   IF_PPPOE,    	0,		RTL_WAN_FID,    	RTL_WANPORT_MASK,         	RTL_WANPORT_MASK,     	1500, { { 0x00, 0x12, 0x34, 0x56, 0x78, 0x91 } }, 1, 0 },
	{	RTL_DRV_PPP4_NETIF_NAME, 1,   IF_PPPOE,    	0,		RTL_WAN_FID,    	RTL_WANPORT_MASK,         	RTL_WANPORT_MASK,     	1500, { { 0x00, 0x12, 0x34, 0x56, 0x78, 0x91 } }, 1, 0 },
	{	RTL_DRV_PPP5_NETIF_NAME, 1,   IF_PPPOE,    	0,		RTL_WAN_FID,    	RTL_WANPORT_MASK,         	RTL_WANPORT_MASK,     	1500, { { 0x00, 0x12, 0x34, 0x56, 0x78, 0x91 } }, 1, 0 },
	{	RTL_DRV_PPP6_NETIF_NAME, 1,   IF_PPPOE,    	0,		RTL_WAN_FID,    	RTL_WANPORT_MASK,         	RTL_WANPORT_MASK,     	1500, { { 0x00, 0x12, 0x34, 0x56, 0x78, 0x91 } }, 1, 0 },
#endif
#endif
	RTL865X_CONFIG_END,
};

#if (defined(CONFIG_PROC_FS) && defined(CONFIG_NET_SCHED) && defined(CONFIG_RTL_LAYERED_DRIVER))||(defined (CONFIG_RTL_HW_QOS_SUPPORT))
//static uint8	netIfName[NETIF_NUMBER][IFNAMSIZ] = {{0}};   /*  br0,nas0,ppp0 (not prototol stack netif name eth0.2 ......)*/
#endif

#if defined(CONFIG_RTL_ETH_PRIV_SKB)
/*	The following structure's field orders was arranged for special purpose, 
	it should NOT be modify	*/
struct priv_skb_buf2 {
	unsigned char magic[ETH_MAGIC_LEN];
	void			*buf_pointer;
	/* the below 2 filed MUST together */
	struct list_head	list;	
	unsigned char buf[ETH_SKB_BUF_SIZE];
};

static struct priv_skb_buf2 eth_skb_buf[MAX_ETH_SKB_NUM+1];
__DRAM_FWD static struct list_head eth_skbbuf_list;
__DRAM_FWD int eth_skb_free_num;
EXPORT_SYMBOL(eth_skb_free_num);
extern struct sk_buff *dev_alloc_8190_skb(unsigned char *data, int size);
#endif
#if 0
#ifdef CONFIG_POCKET_AP_SUPPORT
int rtl865x_curOpMode=BRIDGE_MODE;
int rtl865x_curOpMode_allLan = 0;
#else
#ifndef CONFIG_RTL_MULTI_ETH_WAN
int rtl865x_curOpMode=GATEWAY_MODE;
int rtl865x_curOpMode_allLan = 0;
#else
int rtl865x_curOpMode=BRIDGE_MODE;
int rtl865x_curOpMode_allLan = 1;
#endif
#endif
#endif

__DRAM_FWD static struct re865x_priv _rtl86xx_dev; 

#ifdef CONFIG_RTL_STP
static unsigned char STPmac[] = { 1, 0x80, 0xc2, 0,0,0};
#ifdef CONFIG_RTK_MESH
int8 STP_PortDev_Mapping[MAX_RE865X_STP_PORT] ={NO_MAPPING, NO_MAPPING, NO_MAPPING, NO_MAPPING, NO_MAPPING, WLAN_PSEUDO_IF_INDEX, WLAN_MESH_PSEUDO_IF_INDEX};
#else
int8 STP_PortDev_Mapping[MAX_RE865X_STP_PORT] ={NO_MAPPING, NO_MAPPING, NO_MAPPING, NO_MAPPING, NO_MAPPING, WLAN_PSEUDO_IF_INDEX};
#endif
static int re865x_stp_get_pseudodevno(uint32 port_num);
#endif

static int re865x_ioctl (struct net_device *dev, struct ifreq *rq, int cmd);
//static int32 reinit_vlan_configure(struct rtl865x_vlanConfig new_vlanconfig[]);
//static void rtl_print_vlanconfig(struct rtl865x_vlanConfig new_vlanconfig[]);
#if defined(CONFIG_RTK_VLAN_SUPPORT)
static int read_proc_vlan(char *page, char **start, off_t off,int count, int *eof, void *data);
static int write_proc_vlan(struct file *file, const char *buffer,unsigned long count, void *data);
static int32 rtk_vlan_support_read( char *page, char **start, off_t off, int count, int *eof, void *data );
static int32 rtk_vlan_support_write( struct file *filp, const char *buff,unsigned long len, void *data );	
//__DRAM_FWD int rtk_vlan_support_enable;
int rtk_vlan_support_enable;
EXPORT_SYMBOL(rtk_vlan_support_enable);
#endif

#if defined(RTL8196C_EEE_MAC)
extern int eee_enabled;
extern void eee_phy_enable(void);
extern void eee_phy_disable(void);
#ifdef CONFIG_RTL8196C_REVISION_B
static unsigned char prev_port_sts[MAX_PORT_NUMBER] = { 0, 0, 0, 0, 0 };
#endif
#endif
#if defined(CONFIG_RTL_LOCAL_PUBLIC)
static int32 rtl865x_getPortlistByMac(const unsigned char *mac,uint32 *portlist);
#endif
static inline int rtl_isWanDev(struct dev_priv *cp);

#ifdef CONFIG_RTL8196C_ETH_IOT
uint32 port_link_sts = 0;	// the port which already linked up does not need to check ...
uint32 port_linkpartner_eee = 0;
#endif

#ifdef CONFIG_RTL_8196C_ESD
int _96c_esd_counter = 0;
#endif

#if defined(PATCH_GPIO_FOR_LED)
#define MIB_RX_PKT_CNT	0
#define MIB_TX_PKT_CNT	1

#define PORT_PABCD_BASE	10	// Base of P0~P1 at PABCD
#define P0_PABCD_BIT		10
#define P1_PABCD_BIT		11
#define P2_PABCD_BIT		12
#define P3_PABCD_BIT		13
#define P4_PABCD_BIT		14

#define SUCCESS 0
#define FAILED -1

//#define GPIO_LED_MAX_PACKET_CNT	5000
//#define GPIO_LED_MAX_SCALE			100
#define GPIO_LED_NOBLINK_TIME		(12*HZ/10)	// time more than 1-sec timer interval
//#define GPIO_LED_INTERVAL_TIME		50	// 500ms
#define GPIO_LED_ON_TIME				(4*HZ/100)	// 40ms
#define GPIO_LED_ON					0
#define GPIO_LED_OFF					1
#define GPIO_LINK_STATUS			1
#define GPIO_LINK_STATE_CHANGE 0x80000000

#define GPIO_UINT32_DIFF(a, b)		((a >= b)? (a - b):(0xffffffff - b + a + 1))

struct ctrl_led {
	struct timer_list	LED_Timer;	
	unsigned int			LED_Interval;
	unsigned char		LED_Toggle;
	unsigned char		LED_ToggleStart;
	unsigned int			LED_tx_cnt_log;
	unsigned int			LED_rx_cnt_log;
	unsigned int			LED_tx_cnt;
	unsigned int			LED_rx_cnt;
	unsigned int			link_status;
	unsigned char			blink_once_done;		// 1: blink once done
} led_cb[5];

static int32 rtl819x_getAsicMibCounter(int port, uint32 counter, uint32 *value) 
{
	rtl865x_tblAsicDrv_simpleCounterParam_t simpleCounter;
	rtl8651_getSimpleAsicMIBCounter(port, &simpleCounter);

	switch(counter){
		case MIB_RX_PKT_CNT:
			*value=simpleCounter.rxPkts;
			break;
		case MIB_TX_PKT_CNT:
			*value=simpleCounter.txPkts;
			break;
		default:
			return FAILED;
	}
	return SUCCESS;
}
static void gpio_set_led(int port, int flag){	
	if (flag == GPIO_LED_OFF){
/*		RTL_W32(PABCD_CNR, RTL_R32(PABCD_CNR) & (~((0x1<<port)<<PORT_PABCD_BASE)));	//set GPIO pin
*		RTL_W32(PABCD_DIR, RTL_R32(PABCD_DIR) | ((0x1<<port)<<PORT_PABCD_BASE));//output pin
*/
		RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) | ((0x1<<port)<<PORT_PABCD_BASE)));//set 1
	}
	else{
/*		RTL_W32(PABCD_CNR, RTL_R32(PABCD_CNR) & (~((0x1<<port)<<PORT_PABCD_BASE)));	//set GPIO pin
*		RTL_W32(PABCD_DIR, RTL_R32(PABCD_DIR) | ((0x1<<port)<<PORT_PABCD_BASE));//output pin
*/
		RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) & (~((0x1<<port)<<PORT_PABCD_BASE))));//set 0
	}
}

static void gpio_led_Interval_timeout(unsigned long port)
{
	struct ctrl_led *cb	= &led_cb[port];
	unsigned long flags;

	local_irq_save(flags);

	if (cb->link_status & GPIO_LINK_STATE_CHANGE) {
		cb->link_status &= ~GPIO_LINK_STATE_CHANGE;	
		if (cb->link_status & GPIO_LINK_STATUS)	 
			cb->LED_ToggleStart = GPIO_LED_ON;
		else
			cb->LED_ToggleStart = GPIO_LED_OFF;
		cb->LED_Toggle = cb->LED_ToggleStart;
		gpio_set_led(port, cb->LED_Toggle);
	}
	else {
		if (cb->link_status & GPIO_LINK_STATUS)	 
			gpio_set_led(port, cb->LED_Toggle);
	}

	if (cb->link_status & GPIO_LINK_STATUS)	 {
		if (cb->LED_Toggle == cb->LED_ToggleStart){
			mod_timer(&cb->LED_Timer, jiffies + cb->LED_Interval);
			cb->blink_once_done=1;
		}
		else{
			mod_timer(&cb->LED_Timer, jiffies + GPIO_LED_ON_TIME);
			cb->blink_once_done=0;
		}
		
		cb->LED_Toggle = (cb->LED_Toggle + 1) & 0x1;	//cb->LED_Toggle = (cb->LED_Toggle + 1) % 2;
	}
	local_irq_restore(flags);
}

void calculate_led_interval(int port)
{
	struct ctrl_led *cb	= &led_cb[port];

	unsigned int delta = 0;

	/* calculate counter delta	*/
	delta += GPIO_UINT32_DIFF(cb->LED_tx_cnt, cb->LED_tx_cnt_log);
	delta += GPIO_UINT32_DIFF(cb->LED_rx_cnt, cb->LED_rx_cnt_log);
	cb->LED_tx_cnt_log = cb->LED_tx_cnt;
	cb->LED_rx_cnt_log = cb->LED_rx_cnt;

	/* update interval according to delta	*/
	if (delta == 0) {
		if (cb->LED_Interval == GPIO_LED_NOBLINK_TIME)
			mod_timer(&(cb->LED_Timer), jiffies + cb->LED_Interval);
		else{
			cb->LED_Interval = GPIO_LED_NOBLINK_TIME;
			if(cb->blink_once_done==1){
				mod_timer(&(cb->LED_Timer), jiffies + cb->LED_Interval);
				cb->blink_once_done=0;
			}
		}
	}
	else
	{	
		if(delta>25){		//That is: 200/delta-GPIO_LED_ON_TIME < GPIO_LED_ON_TIME
			cb->LED_Interval = GPIO_LED_ON_TIME;
		}
		else{
			/*	if delta is odd, should be +1 into even.		*/
			/*	just make led blink more stable and smooth.	*/
			if((delta & 0x1) == 1)
				delta++;
			
			cb->LED_Interval=200/delta-GPIO_LED_ON_TIME;		/* rx 1pkt + tx 1pkt => blink one time!	*/

/*			if (cb->LED_Interval < GPIO_LED_ON_TIME)
*				cb->LED_Interval = GPIO_LED_ON_TIME;
*/
		}
	}
}

void update_mib_counter(int port)
{
	uint32 regVal;
	struct ctrl_led *cb	= &led_cb[port];

	regVal=READ_MEM32(PSRP0+(port<<2));
	if((regVal&PortStatusLinkUp)!=0){
		//link up
		if (!(cb->link_status & GPIO_LINK_STATUS)) {
			cb->link_status = GPIO_LINK_STATE_CHANGE | 1;
		}
		rtl819x_getAsicMibCounter(port, MIB_TX_PKT_CNT, (uint32 *)&cb->LED_tx_cnt);
		rtl819x_getAsicMibCounter(port, MIB_RX_PKT_CNT, (uint32 *)&cb->LED_rx_cnt);
	}
	else{
		//link down
		if (cb->link_status & GPIO_LINK_STATUS) {
			cb->link_status = GPIO_LINK_STATE_CHANGE;
		}
	}
}

void init_led_ctrl(int port)
{
	struct ctrl_led *cb	= &led_cb[port];

	RTL_W32(PABCD_CNR, RTL_R32(PABCD_CNR) & (~((0x1<<port)<<PORT_PABCD_BASE)));	//set GPIO pin
	RTL_W32(PABCD_DIR, RTL_R32(PABCD_DIR) | ((0x1<<port)<<PORT_PABCD_BASE));//output pin

	memset(cb, '\0', sizeof(struct ctrl_led));

	update_mib_counter(port);		
	calculate_led_interval(port);	
	cb->link_status |= GPIO_LINK_STATE_CHANGE;

	init_timer(&cb->LED_Timer);
	cb->LED_Timer.expires = jiffies + cb->LED_Interval;
	cb->LED_Timer.data = (unsigned long)port;
	cb->LED_Timer.function = gpio_led_Interval_timeout;	
	mod_timer(&cb->LED_Timer, jiffies + cb->LED_Interval);

	gpio_led_Interval_timeout(port);		
}

void disable_led_ctrl(int port)
{
	struct ctrl_led *cb	= &led_cb[port];
	gpio_set_led(port, GPIO_LED_OFF);

	if (timer_pending(&cb->LED_Timer))	
		del_timer_sync(&cb->LED_Timer);
}
#endif // PATCH_GPIO_FOR_LED

/*
device mapping mainten
*/
#if 0
void rtl_ps_drv_netif_mapping_show(void)
{
	int i;
	printk("linux netif name VS driver netif name mapping:\n");
	for(i = 0; i < NETIF_SW_NUMBER;i++)
	{
		printk("valid(%d),linux netif name(%s) <---->drv netif name(%s)\n",ps_drv_netif_mapping[i].valid,
			ps_drv_netif_mapping[i].ps_netif?ps_drv_netif_mapping[i].ps_netif->name:NULL,ps_drv_netif_mapping[i].drvName);
	}
}
#endif

static int rtl_ps_drv_netif_mapping_init(void)
{
	memset(ps_drv_netif_mapping,0,NETIF_SW_NUMBER * sizeof(ps_drv_netif_mapping_t));	
	return SUCCESS;
}

ps_drv_netif_mapping_t* rtl_get_ps_drv_netif_mapping_by_psdev(struct net_device *dev)
{
	int i;
	for(i = 0; i < NETIF_SW_NUMBER;i++)
	{
		if(ps_drv_netif_mapping[i].valid == 1 && ps_drv_netif_mapping[i].ps_netif == dev)
			return &ps_drv_netif_mapping[i];
	}

	//back compatible,user use br0 to get lan netif
	if(memcmp(dev->name,RTL_PS_BR0_DEV_NAME,strlen(RTL_PS_BR0_DEV_NAME)) == 0)
	{
		for(i = 0; i < NETIF_SW_NUMBER;i++)
		{
			if(ps_drv_netif_mapping[i].valid == 1 && 
				memcmp(ps_drv_netif_mapping[i].drvName,RTL_DRV_LAN_NETIF_NAME,strlen(RTL_DRV_LAN_NETIF_NAME)) == 0)
				return &ps_drv_netif_mapping[i];
		}
	}
	
	return NULL;
}
#if 0

#if defined(CONFIG_RTK_VLAN_SUPPORT) || defined (CONFIG_RTL_MULTI_LAN_DEV)

static struct net_device * rtl_get_psdev_by_ps_drv_netif(char *netifname)
{
	int i;	
	for(i = 0; i < NETIF_SW_NUMBER;i++)
	{
		if(ps_drv_netif_mapping[i].valid == 1 && !strcmp(ps_drv_netif_mapping[i].drvName,netifname))
			return ps_drv_netif_mapping[i].ps_netif;
	}
	return NULL;
}
#endif
#endif

#ifdef CONFIG_RTL8676_Static_ACL
static inline int is_from_not_nicport_under_bridge(struct sk_buff *skb)
{
	/* cannot use the follwing method to retrieve pkt's src mac , it will lead to crash (eth_hdr not initialized, acccess virtual addr)  
	=> something wrong....   bug in our sdk ?  */    
	//unsigned char *pkt_srcmac = eth_hdr(skb)->h_source;
	unsigned char *pkt_srcmac = &skb->data[ETHER_ADDR_LEN];

	ForEachMasterNetif_Declaration  
	/*  1.  this pkt is from other nic port */
	if(skb->src_port == IF_SWITCH)
		return 0;

	/* 2. this pkt is from local netif */
	ForEachMasterNetif_Start
	{
		if(!memcmp(pkt_srcmac,&netif->macAddr.octet,ETHER_ADDR_LEN))
			return 0;
	}
	ForEachMasterNetif_End

	/* other cases : this pkt is from non-nic ports under bridge (ex. wlan0,vc0....)*/

	return 1;
}
#endif

static int rtl_add_ps_drv_netif_mapping(struct net_device *dev, const char *name)
{
	int i;	
	
	//duplicate check
	if(rtl_get_ps_drv_netif_mapping_by_psdev(dev) !=NULL)
		return FAILED;
	
	for(i = 0; i < NETIF_SW_NUMBER;i++)
	{
		if(ps_drv_netif_mapping[i].valid == 0)
			break;
	}
	
	if(i == NETIF_SW_NUMBER)
		return FAILED;
	
	ps_drv_netif_mapping[i].ps_netif = dev;
	memcpy(ps_drv_netif_mapping[i].drvName,name,strlen(name));
	ps_drv_netif_mapping[i].valid = 1;
	return SUCCESS;
}

#if 0
static int rtl_del_ps_drv_netif_mapping(struct net_device *dev)
{
	ps_drv_netif_mapping_t *entry;
	entry = rtl_get_ps_drv_netif_mapping_by_psdev(dev);
	if(entry)
		entry->valid = 0;
	
	return SUCCESS;
}
#endif

/*
 * Disable TX/RX through IO_CMD register
 */
static void rtl8186_stop_hw(struct net_device *dev, struct dev_priv *cp)
{

#if defined(PATCH_GPIO_FOR_LED)
	if (cp->id == RTL_LANVLANID) {
		int port;
		for (port=0; port<RTL8651_PHY_NUMBER; port++)
			disable_led_ctrl(port);
	}
#endif

}


/* Set or clear the multicast filter for this adaptor.
 * This routine is not state sensitive and need not be SMP locked.
 */
static void re865x_set_rx_mode (struct net_device *dev){
/*	Not yet implemented.
	unsigned long flags;
	spin_lock_irqsave (&_rtl86xx_dev.lock, flags);
	spin_unlock_irqrestore (&_rtl86xx_dev.lock, flags);
*/
}

#if defined (CONFIG_RTL_NIC_HWSTATS)
 void  re865x_accumulate_port_stats(uint32 portnum, struct net_device_stats *net_stats)
{	
	uint32 addrOffset_fromP0 =0;	

	if( portnum < 0 ||  portnum > CPU)
			return ;

#ifdef CONFIG_RTL_8367B
    if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8676_TBLASIC_EXTPHYPROPERTY_PORT0_RTL8367B) 
    {   
        rtl8367b_accumulate_port_stats(portnum,net_stats);
        return;            
    }

#endif 			

	addrOffset_fromP0 = portnum * MIB_ADDROFFSETBYPORT;
	/* rx_pkt = rx_unicast +rx_multicast + rx_broadcast */
	net_stats->rx_packets += rtl8651_returnAsicCounter( OFFSET_IFINUCASTPKTS_P0 + addrOffset_fromP0 ) ;
	net_stats->rx_packets += rtl8651_returnAsicCounter( OFFSET_ETHERSTATSMULTICASTPKTS_P0 + addrOffset_fromP0 ) ;
	net_stats->rx_packets += rtl8651_returnAsicCounter( OFFSET_ETHERSTATSBROADCASTPKTS_P0 + addrOffset_fromP0 ) ;
	/* tx_pkt = tx_unicast +tx_multicast + tx_broadcast*/
	net_stats->tx_packets += rtl8651_returnAsicCounter( OFFSET_IFOUTUCASTPKTS_P0 + addrOffset_fromP0 ) ;
	net_stats->tx_packets += rtl8651_returnAsicCounter( OFFSET_IFOUTMULTICASTPKTS_P0 + addrOffset_fromP0 ) ;
	net_stats->tx_packets += rtl8651_returnAsicCounter( OFFSET_IFOUTBROADCASTPKTS_P0 + addrOffset_fromP0 ) ;
	
	net_stats->rx_bytes += rtl8651_returnAsicCounter( OFFSET_IFINOCTETS_P0 + addrOffset_fromP0 ) ;
	net_stats->tx_bytes += rtl8651_returnAsicCounter( OFFSET_IFOUTOCTETS_P0 + addrOffset_fromP0 ) ;
	/*rx_errors = CRC error + Jabber error  + Fragment error*/
	net_stats->rx_errors += rtl8651_returnAsicCounter( OFFSET_DOT3STATSFCSERRORS_P0 + addrOffset_fromP0 ) ;
	net_stats->rx_errors += rtl8651_returnAsicCounter( OFFSET_ETHERSTATSJABBERS_P0 + addrOffset_fromP0 ) ;
	net_stats->rx_errors += rtl8651_returnAsicCounter( OFFSET_ETHERSTATSOVERSIZEPKTS_P0 + addrOffset_fromP0 ) ;
	//OFFSET_DOT1DTPPORTINDISCARDS_P0? 
	//net_stats->rx_dropped += rtl8651_returnAsicCounter( OFFSET_DOT1DTPPORTINDISCARDS_P0 + addrOffset_fromP0 ) ;
	net_stats->tx_dropped += rtl8651_returnAsicCounter( OFFSET_IFOUTDISCARDS + addrOffset_fromP0 ) ;
	net_stats->multicast += rtl8651_returnAsicCounter( OFFSET_ETHERSTATSMULTICASTPKTS_P0 + addrOffset_fromP0 ) ;
  
	net_stats->collisions += rtl8651_returnAsicCounter( OFFSET_ETHERSTATSCOLLISIONS_P0 + addrOffset_fromP0 ) ;
	net_stats->rx_crc_errors += rtl8651_returnAsicCounter( OFFSET_DOT3STATSFCSERRORS_P0 + addrOffset_fromP0 ) ;
	return;
}

void re865x_get_hardwareStats(struct dev_priv  *priv)
{
	uint32 portmask;
	uint32 port;

	portmask = priv->portmask;
	//rx_dropped = priv->net_stats.rx_dropped;
	//memset( &priv->net_stats, 0, sizeof(struct net_device_stats) );
	/* reset counters to 0 */
	priv->net_stats.rx_packets = 0;
	priv->net_stats.tx_packets = 0;
	priv->net_stats.rx_bytes = 0;
	priv->net_stats.tx_bytes = 0;
	priv->net_stats.rx_errors = 0;
	priv->net_stats.tx_dropped = 0;
	priv->net_stats.multicast = 0;
	priv->net_stats.collisions = 0;
	priv->net_stats.rx_crc_errors = 0;
	for( port = 0; port < CPU; port++)
	{
		if((1<<port) & portmask)
		{
			re865x_accumulate_port_stats(port, &priv->net_stats);
		}
			
	}
	return;
}
#endif

static struct net_device_stats *re865x_get_stats(struct net_device *dev){
	struct dev_priv  *dp;
	dp = dev->priv;
	#if defined (CONFIG_RTL_NIC_HWSTATS)
		re865x_get_hardwareStats(dp);
	#endif
	return &dp->net_stats;
}

#if defined (CONFIG_RTL_MULTI_LAN_DEV) || defined(CONFIG_RTK_VLAN_SUPPORT)
static int32 re865x_packVlanConfig(struct rtl865x_vlanConfig vlanConfig1[],  struct rtl865x_vlanConfig vlanConfig2[])
{
	int i, j;
	uint32 vlanCnt=0;
	uint32 found=FALSE;
	
	DBG_VlanConfig_PRK("Enter %s\n",__func__);
	/*get input vlan config entry number*/
	
	memset(vlanConfig2, 0 , NETIF_SW_NUMBER);
	for(i=0; vlanConfig1[i].ifname[0] != '\0'; i++)
	{	
		if(vlanConfig1[i].vid == 0) continue;
		found=FALSE;
		for(j=0;j<vlanCnt;j++)
			if((vlanConfig1[i].vid == vlanConfig2[j].vid)&&(vlanConfig1[i].is_slave==vlanConfig2[j].is_slave)){
				found=TRUE;	
				break;
			}
		if(found==FALSE){
			vlanConfig2[vlanCnt].vid = vlanConfig1[i].vid;
			vlanConfig2[vlanCnt].is_slave= vlanConfig1[i].is_slave;			
			vlanCnt ++;
		}
		
	}

#if 0
	for(i=0; vlanConfig1[i].ifname[0] != '\0'; i++)
	{
		if(vlanConfig1[i].vid != 0)
			vlanCnt++;
	}
#endif

	if((vlanCnt+1) > NETIF_SW_NUMBER)
		printk("ERROR,vlanCnt(%d) > max size %d\n",vlanCnt,NETIF_SW_NUMBER-1);
	
	/*initialize output vlan config*/
	memset(vlanConfig2, 0 , (vlanCnt+1)*sizeof(struct rtl865x_vlanConfig));

	for(i=0; vlanConfig1[i].ifname[0] != '\0'; i++)
	{
		found=FALSE;
		
		if(vlanConfig1[i].vid == 0)
			continue;

		for(j=0; j<vlanCnt; j++)
		{
			if(vlanConfig1[i].if_type != vlanConfig2[j].if_type)
			{
				continue;
			}
			else
			{
				if(vlanConfig1[i].if_type==IF_ETHER)
				{
					/*if multiple vlan config has the same vlan id*/
					/*the first one will decide the  real network interface name/asic mtu/hardware address*/
					
					/*ethernet interface*/
					if(vlanConfig1[i].vid!=vlanConfig2[j].vid)
					{
						continue;
					}
					else
					{
						found=TRUE;
						break;
					}
	
				}
				else
				{
					/*PPP interface*/
					if(strcmp(vlanConfig1[i].ifname, vlanConfig2[j].ifname))
					{
						continue;
					}
					else
					{
						found=TRUE;
						break;
					}
				}
			}
		}

		if(found==TRUE)
		{
			/*merge port mask*/
			vlanConfig2[j].memPort |=vlanConfig1[i].memPort;
			vlanConfig2[j].untagSet |=vlanConfig1[i].untagSet;
		}	
		else
		{
			/*find an empty entry to store this vlan config*/
			for(j=0; j<vlanCnt; j++)
			{
				if(vlanConfig2[j].vid==0)
				{
					memcpy(&vlanConfig2[j], &vlanConfig1[i], sizeof(struct rtl865x_vlanConfig));
					if(( vlanConfig1[i].if_type==IF_ETHER) && ((vlanConfig1[i].isWan==FALSE)
						#ifdef CONFIG_RTL_MULTI_ETH_WAN
							|| (i == 0)
						#endif
						) )
					{
						/*add cpu port to lan member list*/
						vlanConfig2[j].memPort|=0x100;
						vlanConfig2[j].untagSet|=0x100;
					}					
					break;
				}
			}
		}
		
	}

	DBG_VlanConfig_PRK("(%s)---------- vlanConfig1 ------------\n",__func__);
	for(i=0; vlanConfig1[i].ifname[0] != '\0' ; i++)
	{
		DBG_VlanConfig_PRK("name:%7s  isWan:%d  is_slave:%d  vid:%d  port:%03X  proto:%d mac:%02X:%02X:%02X:%02X:%02X:%02X\n"
			,vlanConfig1[i].ifname,vlanConfig1[i].isWan,vlanConfig1[i].is_slave,vlanConfig1[i].vid,vlanConfig1[i].memPort,vlanConfig1[i].protocol
			,vlanConfig1[i].mac.octet[0],vlanConfig1[i].mac.octet[1],vlanConfig1[i].mac.octet[2]
			,vlanConfig1[i].mac.octet[3],vlanConfig1[i].mac.octet[4],vlanConfig1[i].mac.octet[5]);
	}
	DBG_VlanConfig_PRK("(%s)---------- vlanConfig2 ------------\n",__func__);
	for(i=0; vlanConfig2[i].ifname[0] != '\0' ; i++)
	{
		DBG_VlanConfig_PRK("name:%7s  isWan:%d  is_slave:%d  vid:%d  port:%03X  proto:%d mac:%02X:%02X:%02X:%02X:%02X:%02X\n"
			,vlanConfig2[i].ifname,vlanConfig2[i].isWan,vlanConfig2[i].is_slave,vlanConfig2[i].vid,vlanConfig2[i].memPort,vlanConfig1[i].protocol
			,vlanConfig2[i].mac.octet[0],vlanConfig2[i].mac.octet[1],vlanConfig2[i].mac.octet[2]
			,vlanConfig2[i].mac.octet[3],vlanConfig2[i].mac.octet[4],vlanConfig2[i].mac.octet[5]);
	}	
	DBG_VlanConfig_PRK("(%s)-------------------------- ------------\n",__func__);
	DBG_VlanConfig_PRK("Leave %s\n",__func__);
	return SUCCESS;
}
#endif

static void rtl865x_disableDevPortForward(struct net_device *dev, struct dev_priv *cp)
{
	int port;
	for(port=0;port<RTL8651_AGGREGATOR_NUMBER;port++)
	{
		if((1<<port) & cp->portmask)
		{
#if 1
			REG32(PCRP0+(port<<2))= ((REG32(PCRP0+(port<<2)))&(~ForceLink));
			REG32(PCRP0+(port<<2))= ((REG32(PCRP0+(port<<2)))&(~EnablePHYIf));
			TOGGLE_BIT_IN_REG_TWICE(PCRP0+(port<<2),EnablePHYIf);
			TOGGLE_BIT_IN_REG_TWICE(PCRP0+(port<<2),ForceLink);
#else
			rtl865xC_setAsicEthernetForceModeRegs(port, TRUE, FALSE, 1, TRUE);
#endif
		}
	}
#ifdef CONFIG_RTL_8196C_ESD
	_96c_esd_counter = 0;		// stop counting
#endif	
}

static void rtl865x_enableDevPortForward(struct net_device *dev, struct dev_priv *cp)
{
	int port;
	for(port=0;port<RTL8651_AGGREGATOR_NUMBER;port++)
	{
		if((1<<port) & cp->portmask)
		{
#if 1
			REG32(PCRP0+(port<<2))= ((REG32(PCRP0+(port<<2)))|(ForceLink));
			REG32(PCRP0+(port<<2))= ((REG32(PCRP0+(port<<2)))|(EnablePHYIf));
			TOGGLE_BIT_IN_REG_TWICE(PCRP0+(port<<2),EnablePHYIf);
			TOGGLE_BIT_IN_REG_TWICE(PCRP0+(port<<2),ForceLink);
#else
			rtl865xC_setAsicEthernetForceModeRegs(port, FALSE, TRUE, 1, TRUE);
			rtl8651_restartAsicEthernetPHYNway(port);
#endif
		}
	}
#ifdef CONFIG_RTL_8196C_ESD
	_96c_esd_counter = 1;		// start counting and check ESD
#endif	
}

static void rtl865x_disableInterrupt(void)
{
	REG32(CPUICR) = 0; 
	REG32(CPUIIMR) = 0;      
}

static void rtk_queue_init(struct ring_que *que)
{
	memset(que, 0, sizeof(struct ring_que));
	que->ring = (struct sk_buff **)kmalloc(
		(sizeof(struct skb_buff*)*(rtl865x_maxPreAllocRxSkb+1))
		,GFP_KERNEL);
	memset(que->ring, 0, (sizeof(struct sk_buff *))*(rtl865x_maxPreAllocRxSkb+1));
	que->qmax = rtl865x_maxPreAllocRxSkb;	
}

static int rtk_queue_tail(struct ring_que *que, struct sk_buff *skb)
{
	int next;
	

	if (que->head == que->qmax)
		next = 0;
	else
		next = que->head + 1;
	
	if (que->qlen >= que->qmax || next == que->tail) {
		return 0;
	}	
	
	que->ring[que->head] = skb;
	que->head = next;
	que->qlen++;

	return 1;
}

static struct sk_buff *rtk_dequeue(struct ring_que *que)
{
	struct sk_buff *skb;

	if (que->qlen <= 0 || que->tail == que->head)
	{
		return NULL;
	}

	skb = que->ring[que->tail];
		
	if (que->tail == que->qmax)
		que->tail  = 0;
	else
		que->tail++;

	que->qlen--;	

	return (struct sk_buff *)skb;
}

#if defined(CONFIG_RTL_ETH_PRIV_SKB_DEBUG)
int get_buf_in_rx_skb_queue(void)
{
	return rx_skb_queue.qlen;
}

int get_buf_in_poll(void)
{
	return eth_skb_free_num;
}
#endif

//__MIPS16
__IRAM_FWD
static void refill_rx_skb(void)
{
	struct sk_buff *skb;
	unsigned long flags;

	while (rx_skb_queue.qlen < rtl865x_maxPreAllocRxSkb)
	{
		#if defined(CONFIG_RTL_ETH_PRIV_SKB)
		skb = dev_alloc_skb_priv_eth(CROSS_LAN_MBUF_LEN);
		#else
		skb = dev_alloc_skb(CROSS_LAN_MBUF_LEN);
		#endif

		if (skb == NULL) { 
			DEBUG_ERR("EthDrv: dev_alloc_skb() failed!\n");		
			return;
		}
		skb_reserve(skb, RX_OFFSET);
#ifdef RTK_QUE
		local_irq_save(flags);
		if (!rtk_queue_tail(&rx_skb_queue, skb))
			dev_kfree_skb_any(skb);
		local_irq_restore(flags);
#else		
		__skb_queue_tail(&rx_skb_queue, skb);
#endif
	}
}

//---------------------------------------------------------------------------
static void free_rx_skb(void)
{
	struct sk_buff *skb;

	swNic_freeRxBuf();

	while  (rx_skb_queue.qlen > 0) {
#ifdef RTK_QUE
		skb = rtk_dequeue(&rx_skb_queue);
#else
		skb = __skb_dequeue(&rx_skb_queue);
#endif
		dev_kfree_skb_any(skb);
	}
}

//---------------------------------------------------------------------------
unsigned char *alloc_rx_buf(void **skb, int buflen)
{
	struct sk_buff *new_skb;
	unsigned long flags;

	if (rx_skb_queue.qlen == 0) {
#if defined(CONFIG_RTL_ETH_PRIV_SKB)
		new_skb = dev_alloc_skb_priv_eth(CROSS_LAN_MBUF_LEN);
#else        
		new_skb = dev_alloc_skb(CROSS_LAN_MBUF_LEN);
#endif
		if (new_skb == NULL) { 
			DEBUG_ERR("EthDrv: alloc skb failed!\n");
		}
		else		
			skb_reserve(new_skb, RX_OFFSET);
	}
	else {
#ifdef RTK_QUE
		local_irq_save(flags);
		new_skb = rtk_dequeue(&rx_skb_queue);
		local_irq_restore(flags);
#else
		new_skb = __skb_dequeue(&rx_skb_queue);
#endif
	}
	
	if (new_skb == NULL) 
		return NULL;	
	*skb = new_skb;

	return new_skb->data;		
}

//---------------------------------------------------------------------------
void free_rx_buf(void *skb)
{
	dev_kfree_skb_any((struct sk_buff *)skb);
}

//---------------------------------------------------------------------------
void tx_done_callback(void *skb)
{

	dev_kfree_skb_any((struct sk_buff *)skb);	
}

#if defined (CONFIG_RTL_IGMP_SNOOPING)		
#if defined (CONFIG_BRIDGE)
extern void br_signal_igmpProxy(void);
#endif

static inline struct iphdr * re865x_getIpv4Header(uint8 *macFrame)
{
	uint8 *ptr;
	struct iphdr *iph=NULL;
	
	ptr=macFrame+12;
	if(*(int16 *)(ptr)==(int16)htons(ETH_P_8021Q))
	{
		ptr=ptr+4;
	}

	if(*(int16 *)(ptr)==(int16)htons(ETH_P_PPP_SES))
	{
		ptr=ptr+8;

		/*it's not ppp over ipv4 packet*/
		if(*(int16 *)(ptr)!=(int16)htons(0x0021))
		{
			return NULL;
		}
		
		iph=(struct iphdr *)(ptr+2);
		return iph;
	}
	else
	{
		/*it's not ipv4 packet*/
		if(*(int16 *)(ptr)!=(int16)htons(ETH_P_IP))
		{
			return NULL;
		}
		
		iph=(struct iphdr *)(ptr+2);

		return iph;
	}

	
}

#if defined (CONFIG_RTL_MLD_SNOOPING)
static inline struct ipv6hdr* re865x_getIpv6Header(uint8 *macFrame)
{
	uint8 *ptr;
	struct ipv6hdr *ipv6h=NULL;
	
	ptr=macFrame+12;
	if(*(int16 *)(ptr)==(int16)htons(ETH_P_8021Q))
	{
		ptr=ptr+4;
	}

	/*it's not ipv6 packet*/
	if(*(int16 *)(ptr)!=(int16)htons(ETH_P_IPV6))
	{
		return NULL;
	}
	
	ipv6h=(struct ipv6hdr *)(ptr+2);

	return ipv6h;
}
#if 0
#define IPV6_ROUTER_ALTER_OPTION 0x05020000
#define  HOP_BY_HOP_OPTIONS_HEADER 0
#define ROUTING_HEADER 43
#define  FRAGMENT_HEADER 44
#define DESTINATION_OPTION_HEADER 60

#define PIM_PROTOCOL 103
#define MOSPF_PROTOCOL 89
#define TCP_PROTOCOL 6
#define UDP_PROTOCOL 17
#define NO_NEXT_HEADER 59
#define ICMP_PROTOCOL 58

#define MLD_QUERY 130
#define MLDV1_REPORT 131
#define MLDV1_DONE 132
#define MLDV2_REPORT 143

#define IS_IPV6_PIM_ADDR(ipv6addr) ((ipv6addr[0] == 0xFF020000)&&(ipv6addr[1] == 0x00000000)&&(ipv6addr[2] == 0x00000000)&&(ipv6addr[3] ==0x0000000D)) 
#define IS_IPV6_MOSPF_ADDR1(ipv6addr) ((ipv6addr[0] == 0xFF020000)&&(ipv6addr[1] == 0x00000000)&&(ipv6addr[2] == 0x00000000)&&(ipv6addr[3] ==0x00000005)) 		
#define IS_IPV6_MOSPF_ADDR2(ipv6addr) ((ipv6addr[0] == 0xFF020000)&&(ipv6addr[1] == 0x00000000)&&(ipv6addr[2] == 0x00000000)&&(ipv6addr[3] ==0x00000006)) 


int re865x_getIpv6TransportProtocol(struct ipv6hdr* ipv6h)
{
	
	unsigned char *ptr=NULL;
	unsigned char *startPtr=NULL;
	unsigned char *lastPtr=NULL;
	unsigned char nextHeader=0;
	unsigned short extensionHdrLen=0;

	unsigned char  optionDataLen=0;
	unsigned char  optionType=0;
	unsigned int ipv6RAO=0;
		
	if(ipv6h==NULL)
	{
		return -1;
	}
	
	if(ipv6h->version!=6)
	{
		return -1;
	}
	
	startPtr= (unsigned char *)ipv6h;
	lastPtr=startPtr+sizeof(struct ipv6hdr)+(ipv6h->payload_len);
	nextHeader= ipv6h ->nexthdr;
	ptr=startPtr+sizeof(struct ipv6hdr);

	while(ptr<lastPtr)
	{
		switch(nextHeader) 
		{
			case HOP_BY_HOP_OPTIONS_HEADER:
				/*parse hop-by-hop option*/
				nextHeader=ptr[0];
				extensionHdrLen=((uint16)(ptr[1])+1)*8;
				ptr=ptr+2;
				
				while(ptr<(startPtr+extensionHdrLen+sizeof(struct ipv6hdr)))
				{
					optionType=ptr[0];
					/*pad1 option*/
					if(optionType==0)
					{
						ptr=ptr+1;
						continue;
					}

					/*padN option*/
					if(optionType==1)
					{
						optionDataLen=ptr[1];
						ptr=ptr+optionDataLen+2;
						continue;
					}

					/*router altert option*/
					if(ntohl(*(uint32 *)(ptr))==IPV6_ROUTER_ALTER_OPTION)
					{
						ipv6RAO=IPV6_ROUTER_ALTER_OPTION;
						ptr=ptr+4;	
						continue;
					}

					/*other TLV option*/
					if((optionType!=0) && (optionType!=1))
					{
						optionDataLen=ptr[1];
						ptr=ptr+optionDataLen+2;
						continue;
					}
				

				}
				
				break;
			
			case ROUTING_HEADER:
				nextHeader=ptr[0];
				extensionHdrLen=((uint16)(ptr[1])+1)*8;			
                            ptr=ptr+extensionHdrLen;
				break;
			
			case FRAGMENT_HEADER:
				nextHeader=ptr[0];
				ptr=ptr+8;
				break;
			
			case DESTINATION_OPTION_HEADER:
				nextHeader=ptr[0];
				extensionHdrLen=((uint16)(ptr[1])+1)*8;
				ptr=ptr+extensionHdrLen;
				break;
			
			case ICMP_PROTOCOL:
				nextHeader=NO_NEXT_HEADER;
				if((ptr[0]==MLD_QUERY) ||(ptr[0]==MLDV1_REPORT) ||(ptr[0]==MLDV1_DONE) ||(ptr[0]==MLDV2_REPORT))
				{
					return ICMP_PROTOCOL;
					
				}
				break;
			
			case PIM_PROTOCOL:
				nextHeader=NO_NEXT_HEADER;
				if(IS_IPV6_PIM_ADDR(ipv6h->daddr.s6_addr32))
				{
					return PIM_PROTOCOL;
				}
			
				break;
			
			case MOSPF_PROTOCOL:
				nextHeader=NO_NEXT_HEADER;
				
				if(IS_IPV6_MOSPF_ADDR1(ipv6h->daddr.s6_addr32) || IS_IPV6_MOSPF_ADDR2(ipv6h->daddr.s6_addr32))
				{
					return MOSPF_PROTOCOL;
				}
				break;
				
			case TCP_PROTOCOL:
				nextHeader=NO_NEXT_HEADER;
				return TCP_PROTOCOL;
		
				break;
				
			case UDP_PROTOCOL:
				nextHeader=NO_NEXT_HEADER;
				return UDP_PROTOCOL;
	
				break;	
				
			default:		
				/*not ipv6 multicast protocol*/
				return -1;
				break;
		}
	
	}
	return -1;
}
#endif	
#endif

void rtl865x_igmpSyncLinkStatus(void)
{
	rtl_igmpPortInfo_t portInfo;
	
	portInfo.linkPortMask=newLinkPortMask;
	rtl865x_igmpLinkStatusChangeCallback(nicIgmpModuleIndex, &portInfo);
	
	#if defined (CONFIG_BRIDGE)
	if((newLinkPortMask & (~curLinkPortMask))!=0)
	{	
		/*there is some port linking up*/
		/*notice igmp proxy daemon to send general query to newly link up client*/
		#ifdef CONFIG_RTL8196BU_8186SDK_MP_SPI 
		#else
		br_signal_igmpProxy();
		#endif
	}
	#endif
	
	return;
			
}

#endif	/*	defined (CONFIG_RTL_IGMP_SNOOPING)	*/

#ifdef CONFIG_RTL_MULTI_ETH_WAN /*move from smux.c*/
unsigned int getMemberOfSmuxDevByVid(unsigned int vid)
{
	struct smux_dev_info *dev_info;
	struct net_device *vdev;
	unsigned char name[MAX_IFNAMESIZE];
	rtl865x_getMasterNetifByVid(vid, name);
	vdev = dev_get_by_name(&init_net, name);
	if (!vdev)
		return -1;
	dev_put(vdev);
	dev_info = SMUX_DEV_INFO(vdev);
	return dev_info->member;
}
#endif	/*end of CONFIG_RTL865X_IGMP_SNOOPING*/

static inline int32 rtl_isWanDev(struct dev_priv *cp)
{
#if defined(CONFIG_RTK_VLAN_SUPPORT)
	return (!cp->vlan_setting.is_lan);
#else
#ifndef CONFIG_RTL_MULTI_ETH_WAN
	return (cp->id==RTL_WANVLANID);
#else
	return (cp->id != RTL_LANVLANID);
#endif
#endif
}

#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
static inline int32 rtl_isPassthruFrame(uint8 *data)
{
	int	ret;

	ret = FAILED;
	if (oldStatus)
	{
		if (oldStatus&IP6_PASSTHRU_MASK)
		{
			if ((*((uint16*)(data+(ETH_ALEN<<1)))==__constant_htons(ETH_P_IPV6)) ||
				((*((uint16*)(data+(ETH_ALEN<<1)))==__constant_htons(ETH_P_8021Q))&&(*((uint16*)(data+(ETH_ALEN<<1)+VLAN_HLEN))==__constant_htons(ETH_P_IPV6))))
			{
				ret = SUCCESS;
			}
		}

		if (oldStatus&PPPOE_PASSTHRU_MASK)
		{
			if (((*((uint16*)(data+(ETH_ALEN<<1)))==__constant_htons(ETH_P_PPP_SES))||(*((uint16*)(data+(ETH_ALEN<<1)))==__constant_htons(ETH_P_PPP_DISC))) ||
				((*((uint16*)(data+(ETH_ALEN<<1)))==__constant_htons(ETH_P_8021Q))&&((*((uint16*)(data+(ETH_ALEN<<1)+VLAN_HLEN))==__constant_htons(ETH_P_PPP_SES))||(*((uint16*)(data+(ETH_ALEN<<1)+VLAN_HLEN))==__constant_htons(ETH_P_PPP_DISC)))))
			{
				ret = SUCCESS;
			}
		}
	}

	return ret;
}
#endif

#if defined(RTL_CPU_QOS_ENABLED)
__DRAM_FWD	static	int	highestPriority;
__DRAM_FWD static 	int	cpuQosHoldLow;
__DRAM_FWD static 	int	totalLowQueueCnt;
struct timer_list	cpuQosTimer;

static rtl_queue_entry	pktQueueByPri[RTL865X_SWNIC_RXRING_MAX_PKTDESC] = {{0}};

__MIPS16 __IRAM_FWD
int rtl_enqueueSkb(rtl_nicRx_info *info)
{
	rtl_queue_entry	*entry;
	
	entry = &pktQueueByPri[info->priority];

	if (info->priority<cpuQosHoldLow)
		totalLowQueueCnt++;

	memcpy(&entry->entry[entry->end], info, sizeof(rtl_nicRx_info));
	entry->cnt++;
	entry->end = (entry->end==(RTL_NIC_QUEUE_LEN-1))?0:(entry->end+1);

	if (entry->cnt==RTL_NIC_QUEUE_LEN)
		return SUCCESS;
	else
		return FAILED;
}

__MIPS16 __IRAM_FWD
int rtl_dequeueSkb(rtl_nicRx_info *info)
{
	rtl_queue_entry	*entry;
	
	entry = &pktQueueByPri[info->priority];

	if(entry->cnt==0)
	{
		return FAILED;
	}
	else
	{
		memcpy(info, &entry->entry[entry->start], sizeof(rtl_nicRx_info));
		entry->cnt--;
		entry->start = (entry->start==(RTL_NIC_QUEUE_LEN-1))?0:(entry->start+1);
		return SUCCESS;
	}
}

static void rtl_cpuQosTimer(unsigned long data)
{
	cpuQosHoldLow = highestPriority;

	init_timer(&cpuQosTimer);
	cpuQosTimer.function = rtl_cpuQosTimer;
}
#endif

#if defined(RTL_CPU_QOS_ENABLED)
__MIPS16 __IRAM_FWD
static inline int32 rtl_processReceivedInfo(rtl_nicRx_info *info, int nicRxRet)
{
	int	ret;
	
	switch(nicRxRet)
	{
		case RTL_NICRX_OK:
			{
				if (highestPriority<info.priority)
				{
					highestPriority = info.priority;
					cpuQosHoldLow = highestPriority;
				}

				if (info.priority==(RTL865X_SWNIC_RXRING_MAX_PKTDESC-1))
				{
					ret = RTL_RX_PROCESS_RETURN_SUCCESS;
				}
				else	if (rtl_enqueueSkb(&info) == SUCCESS)
				{
					rtl_dequeueSkb(&info);
					ret = RTL_RX_PROCESS_RETURN_SUCCESS;
				}
				else
					ret = RTL_RX_PROCESS_RETURN_CONTINUE;

				break;
			}
		case RTL_NICRX_NULL:
			{
				info.priority = highestPriority;
				if (rtl_dequeueSkb(&info)==SUCCESS)
				{
					ret = RTL_RX_PROCESS_RETURN_SUCCESS;
				}
				else if (highestPriority>0)
				{
					highestPriority--;
					if (highestPriority==0)
					{
						mod_timer(&cpuQosTimer, jiffies+RTL_CPUQOS_TIMER_INTERVAL);
					}
					ret = RTL_RX_PROCESS_RETURN_CONTINUE;
				}
				else
				{
					/* highestPriority=0 */
					if (cpuQosHoldLow)
					{
						swNic_flushRxRingByPriority(cpuQosHoldLow);
					}
					ret = RTL_RX_PROCESS_RETURN_BREAK;
				}
				
				break;
			}
		case RTL_NICRX_REPEAT:
			ret = RTL_RX_PROCESS_RETURN_BREAK;
			break;
	}
	return ret;
}
#else	/*	defined(RTL_CPU_QOS_ENABLED)	*/
__MIPS16 __IRAM_FWD
static inline int32 rtl_processReceivedInfo(rtl_nicRx_info *info, int nicRxRet)
{
	int	ret;

	ret = RTL_RX_PROCESS_RETURN_BREAK;
	switch(nicRxRet)
	{
		case RTL_NICRX_OK:
			{
				ret = RTL_RX_PROCESS_RETURN_SUCCESS;
				break;
			}
		case RTL_NICRX_NULL:
		case RTL_NICRX_REPEAT:
			break;
	}
	return ret;
}
#endif	/*	defined(RTL_CPU_QOS_ENABLED)	*/

__MIPS16 __IRAM_FWD
static inline int32 rtl_decideRxDevice(rtl_nicRx_info *info)
{
	struct dev_priv	*cp;
	int32			pid, i, ret;
	struct sk_buff 	*skb;
	uint8*			data;
	#if defined(CONFIG_RTL_STP)
	int32 			dev_no;
	#endif

	pid = info->pid;
	skb = info->input;
	data = skb->data;

	info->priv = NULL;
	info->isPdev=FALSE;
	ret = FAILED;

	#if defined(CONFIG_RTL_STP)
	if ((data[0]&0x01) && !memcmp(data, STPmac, 5) && !(data[5] & 0xF0))
	{
		/* It's a BPDU */
		dev_no = re865x_stp_get_pseudodevno(pid);
		if (dev_no != NO_MAPPING)
		{
			info->priv = _rtl86xx_dev.stp_port[dev_no]->priv;
			ret = SUCCESS;
		/*	printk("receieved BPDU packet in nic form %s\n", skb->dev->name);	*/
		}
		else
		{
			dev_kfree_skb_any(skb);
			return FAILED;
		}
	}
	else
	#endif
	{
		for(i = 0; i < ETH_INTF_NUM; i++)
		{
			cp = ((struct dev_priv *)_rtl86xx_dev.dev[i]->priv);
			//printk("=========%s(%d),cp(%s),i(%d)\n",__FUNCTION__,__LINE__,cp->dev->name,i);
			if(cp && cp->opened && (cp->portmask & (1<<pid)))
			{
				info->priv = cp;
				ret = SUCCESS;
				break;
			}
		}

		//printk("====%s(%d),dev(%s),i(%d)\n",__FUNCTION__,__LINE__,cp->dev->name,i);
		if(ETH_INTF_NUM==i)
		{
			dev_kfree_skb_any(skb);
			return FAILED;
		}
		#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
		else if (SUCCESS==rtl_isPassthruFrame(data)&&(rtl_isWanDev(cp)==TRUE))
		{
			info->priv = _rtl86xx_dev.pdev->priv;
			info->isPdev=TRUE;	
		}
		#endif
		

	}
	
	return ret;
}

#if defined(BR_SHORTCUT)
__MIPS16 __IRAM_FWD
static inline int32 rtl_processBridgeShortCut(struct sk_buff *skb, struct dev_priv *cp_this)
{
	struct net_device *dev;
	
	if (
		#if 0
		(eth_flag & BIT(2)) &&
		#endif
	#if defined(CONFIG_WIRELESS_LAN_MODULE)
		(wirelessnet_hook_shortcut !=NULL ) && ((dev = wirelessnet_hook_shortcut(skb->data)) != NULL)
	#else
		((dev = get_shortcut_dev(skb->data)) != NULL)
	#endif							
	)
	{
		#if defined(CONFIG_RTL_HARDWARE_NAT)
		if (memcmp(&skb->data[ETH_ALEN], cp_this->dev->dev_addr, ETH_ALEN))
		#endif
		{
			memcpy(cached_eth_addr, &skb->data[ETH_ALEN], ETH_ALEN);
			cached_dev = cp_this->dev;	
		}
		/*skb->dev = dev;*/ /* for performance */
		#if defined(CONFIG_COMPAT_NET_DEV_OPS)
		dev->hard_start_xmit(skb, dev);
		#else
		dev->netdev_ops->ndo_start_xmit(skb,dev);
		#endif
		DEBUG_ERR("[%s][%d]-[%s]\n", __FUNCTION__, __LINE__, skb->dev->name);
		return SUCCESS;
	}
	return FAILED;
}
#endif

__MIPS16 __IRAM_FWD
static inline void rtl_processRxToProtcolStack(struct sk_buff *skb, struct dev_priv *cp_this, rtl_nicRx_info *info)
{
	skb->protocol = eth_type_trans(skb, skb->dev);
	skb->ip_summed = CHECKSUM_NONE;
	//printk("[%s][%d]-skb->dev[%s],proto(0x%x)\n", __FUNCTION__, __LINE__, skb->dev->name,skb->protocol);

 
#ifdef CONFIG_RTL_8676HWNAT
	#ifdef CONFIG_RTL_MULTI_LAN_DEV
	skb->vlan_member = cp_this->port_member;
	skb->src_port = IF_SWITCH;
	skb->switch_port = skb->dev->name;	
	#else
	/* For non-MULTI_LAN, we have only eth0, but user program expects eth0.2-5 for IPQoS to work. - andrew */
	do {
		const char *ifnames[] = { ALIASNAME_ELAN_PREFIX "2", ALIASNAME_ELAN_PREFIX "3", ALIASNAME_ELAN_PREFIX "4", ALIASNAME_ELAN_PREFIX "5" };
		skb->vlan_member = cp_this->port_member;
		skb->src_port = IF_SWITCH;
		if ((info->pid >= 1) && (info->pid <= 4))		
			skb->switch_port = (char *)ifnames[info->pid - 1];
		else
			skb->switch_port = skb->dev->name;
		//printk("%s(%d): pid=%d(%s)\n",__FUNCTION__,__LINE__,pid, skb->switch_port);
	} while (0);
	#endif
#endif

#if defined(RX_TASKLET)
	#if defined(CONFIG_RTL_LOCAL_PUBLIC)
	skb->localPublicFlags = 0;
	#endif


	//netif_receive_skb(skb);
	netif_rx(skb);
#else	/*	defined(RX_TASKLET)	*/
	netif_rx(skb);
#endif	/*	defined(RX_TASKLET)	*/
}

__MIPS16 __IRAM_FWD
static inline void rtl_processRxFrame(rtl_nicRx_info *info)
{
	struct dev_priv	*cp_this;
	struct sk_buff 	*skb;
	uint32			vid, pid, len;
	uint8			*data;

	cp_this = info->priv;
	skb = info->input;	
	vid = info->vid;
	data = skb->tail = skb->data;
	
	/*	sanity check	*/
	#if 0
	if  ((memcmp(&data[ETH_ALEN], cp_this->dev->dev_addr, ETH_ALEN)==0)||PKTHDR_EXTPORT_MAGIC2==vid||PKTHDR_EXTPORT_MAGIC==vid)// check source mac
	{
		if ((PKTHDR_EXTPORT_MAGIC!=vid)||(info->pid!=PKTHDR_EXTPORT_P3))
		{
			
			cp_this->net_stats.rx_dropped++;
			dev_kfree_skb_any(skb);
			return;
		}
	}	
	#else
	//Kevin, now we only use extPort3 (port8) and check source mac
	if(  vid==PKTHDR_EXTPORT_MAGIC || vid==PKTHDR_EXTPORT_MAGIC2 )
	{
		cp_this->net_stats.rx_dropped++;
        dev_kfree_skb_any(skb);
       	return;
	}
	#endif
	/*	sanity check end 	*/
	
	pid = info->pid;
	len = info->len;
	skb->len = 0;
	skb_put(skb, len);
	#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
	skb->dev=info->isPdev?_rtl86xx_dev.pdev:info->priv->dev;
	#else
	skb->dev=info->priv->dev;
	#endif
#ifdef CONFIG_RTL_MULTI_ETH_WAN
	skb->from_dev = skb->dev;
#endif
	//skb->dev=cp_this->dev;

#if defined(CONFIG_NETFILTER_XT_MATCH_PHYPORT)
	skb->srcPhyPort=(uint8)pid;
#endif
	//printk("=======%s(%d),cp_this(%s)\n",__FUNCTION__,__LINE__,cp_this->dev->name);
	/*	vlan process (including strip vlan tag)	*/
	#if defined(CONFIG_RTK_VLAN_SUPPORT)
	if (rtk_vlan_support_enable && cp_this->vlan_setting.global_vlan) 
	{
		if (rx_vlan_process(cp_this->dev, &cp_this->vlan_setting, skb))
		{
			cp_this->net_stats.rx_dropped++;
			dev_kfree_skb_any(skb);		
			return;
		}

		#if defined(CONFIG_RTK_VLAN_FOR_CABLE_MODEM)								
		if(rtk_vlan_support_enable == 2)
		{
			DEBUG_ERR("====%s(%d),cp(%s),cp->vlan.id(%d),skb->tag.vid(%d)\n",__FUNCTION__,__LINE__,
				cp_this->dev->name,cp_this->vlan_setting.id,skb->tag.f.pci&0xfff);
			if(cp_this->vlan_setting.vlan && skb->tag.f.tpid == htons(ETH_P_8021Q) && (skb->tag.f.pci & 0xfff) != cp_this->vlan_setting.id)
			{
				if(cp_this->vlan_setting.is_lan == 0)
				{
					struct net_device* vap;
					/*	look up vap	*/
					vap = get_dev_by_vid(skb->tag.f.pci & 0xfff);											
					if(vap)
					{
						
						skb->dev = vap;
						vap->netdev_ops->ndo_start_xmit(skb,vap);
						return;
					}
				}
			}
		}
		#endif
	}
	#endif	/*	defined(CONFIG_RTK_VLAN_SUPPORT)	*/

	if (*((uint16*)(skb->data+(ETH_ALEN<<1))) == __constant_htons(ETH_P_8021Q)) 
	{
		vid = *((unsigned short *)(data+(ETH_ALEN<<1)+2));
		#if	defined(CONFIG_RTL_QOS_8021P_SUPPORT)
		skb->srcVlanPriority = (vid>>13)&0x7;
		#endif
		skb->mark = ((vid>>13)&0x7)+1;
		vid &= 0x0fff;

#ifdef CONFIG_RTL_MULTI_ETH_WAN
		skb->vlan_tci = (vid+1);
#endif
		memmove(data + VLAN_HLEN, data, VLAN_ETH_ALEN<<1);
		skb_pull(skb, VLAN_HLEN);
	}
#ifdef CONFIG_RTL_MULTI_ETH_WAN
	else
		skb->vlan_tci = 0;
#endif
	/*	vlan process end (vlan tag has already stripped)	*/

	/*	update statistics	*/
	#ifndef CONFIG_RTL_NIC_HWSTATS
	cp_this->net_stats.rx_packets++;	
	cp_this->net_stats.rx_bytes += skb->len;	
	#endif
	cp_this->dev->last_rx = jiffies;
	/*	update statistics end	*/


	/*  Kevin , handle the pkts towarding to extPort*/
	#if defined(CONFIG_RTL_HARDWARE_NAT)	&& defined(CONFIG_RTL8676_Static_ACL)
	if(vid==PKTHDR_EXTPORT_MAGIC3)
	{
		skb->acl_forward_to_extPort=1;
    
	}
	#endif

	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	skb->srcVlanId=info->vid;
	skb->srcPort=info->pid;
	#endif

	
	/*	finally successfuly rx to protocol stack	*/
	rtl_processRxToProtcolStack(skb, cp_this, info);
}

__IRAM_FWD
static int32 interrupt_dsr_rx(unsigned long task_priv)
{
	struct dev_priv *cp = (struct dev_priv *)task_priv;
	int	ret;
	rtl_nicRx_info	info;
	#ifdef RX_TASKLET
	unsigned long flags;
	#endif
	int rx_work = NUM_RX_PKTHDR_DESC;
	while (rx_work--) 
	{
		info.priority = RTL_ASSIGN_RX_PRIORITY;		
		ret = swNic_receive(&info);	
		
		ret = rtl_processReceivedInfo(&info, ret);

		if (RTL_RX_PROCESS_RETURN_SUCCESS==ret)
		{
			ret = rtl_decideRxDevice(&info);
			if (SUCCESS==ret)
			{
				rtl_processRxFrame(&info);
			}
			else
			{				
				continue;
			}
		}
		else if (RTL_RX_PROCESS_RETURN_BREAK==ret)
		{
			break;
		}			
	}

#ifdef RX_TASKLET
	local_irq_save(flags);
	if (!rx_work)
		tasklet_schedule(&cp->rx_dsr_tasklet);
	rtl_rxSetTxDone(TRUE);		
		REG32(CPUIIMR) |= (RX_DONE_IP_ALL|PKTHDR_DESC_RUNOUT_IE_ALL|MBUF_DESC_RUNOUT_IE_ALL);
		//krammer: don't clear other isr
		 REG32(CPUIISR) = (PKTHDR_DESC_RUNOUT_IE_ALL|MBUF_DESC_RUNOUT_IE_ALL);
	local_irq_restore(flags);
#endif

	return RTL_NICRX_OK;
}


__IRAM_FWD
static void interrupt_dsr_tx(unsigned long task_priv)
{
	int32	idx;
	#ifdef TX_TASKLET
	unsigned long flags;
	#endif

	for(idx=RTL865X_SWNIC_TXRING_MAX_PKTDESC-1;idx>=0;idx--)
	{
		swNic_txDone(idx); 
	}

	refill_rx_skb();
	#ifdef TX_TASKLET
	local_irq_save(flags);
	rtl_rxSetTxDone(TRUE);
	local_irq_restore(flags);
	#endif
}

#ifdef CONFIG_RTL8196C_ETH_IOT
static int re865x_setPhyGrayCode(void)
{
    uint32 agc, cb0, snr, new_port_link_sts=0, val;
    uint32 DUT_eee, Linkpartner_eee;
    int i;

    /************ Link down check ************/
    for(i=0; i<RTL8651_PHY_NUMBER; i++) {
		if ((REG32(PSRP0 + (i * 4)) & LinkDownEventFlag) != 0){	// !!! LinkDownEventFlag is a read clear bit, so these code must ahead of "Link up check"
				if ((REG32(PSRP0 + (i * 4)) & PortStatusLinkUp) == 0) {
					/*=========== ###01 ===========*/
					extern void set_gray_code_by_port(int);
					set_gray_code_by_port(i);

					/*=========== ###03 ===========*/
					// read DUT eee 100 ability
					rtl8651_setAsicEthernetPHYReg( i, 13, 0x7 );
					rtl8651_setAsicEthernetPHYReg( i, 14, 0x3c );
					rtl8651_setAsicEthernetPHYReg( i, 13, 0x4007 );
					rtl8651_getAsicEthernetPHYReg( i, 14, &DUT_eee );

					// due to the RJ45 cable is plug out now, we can't read the eee 100 ability of link partner.
					// we use the variable port_linkpartner_eee to keep link partner's eee 100 ability after RJ45 cable is plug in.
					if (  ( ((DUT_eee & 0x2) ) == 0)  || ((port_linkpartner_eee & (1<<i)) == 0) )  {
						rtl8651_getAsicEthernetPHYReg( i, 21, &val );
						val = val & ~(0x4000);
						rtl8651_setAsicEthernetPHYReg( i, 21, val );
					}
				}
			}
	}

	/************ Link up check ************/
	for(i=0; i<RTL8651_PHY_NUMBER; i++) {
		if ((REG32(PSRP0 + (i * 4)) & PortStatusLinkUp) != 0) {
			
			if ((port_link_sts & (1<<i)) == 0) {	// the port which already linked up does not need to check ...

				/*=========== ###03 ===========*/
				// read DUT eee 100 ability
				rtl8651_setAsicEthernetPHYReg( i, 13, 0x7 );
				rtl8651_setAsicEthernetPHYReg( i, 14, 0x3c );
				rtl8651_setAsicEthernetPHYReg( i, 13, 0x4007 );
				rtl8651_getAsicEthernetPHYReg( i, 14, &DUT_eee );

				// read Link partner eee 100 ability
				rtl8651_setAsicEthernetPHYReg( i, 13, 0x7 );
				rtl8651_setAsicEthernetPHYReg( i, 14, 0x3d );
				rtl8651_setAsicEthernetPHYReg( i, 13, 0x4007 );
				rtl8651_getAsicEthernetPHYReg( i, 14, &Linkpartner_eee );

				if (  ( ((DUT_eee & 0x2) ) != 0)  && ( ((Linkpartner_eee & 0x2) ) != 0) )  {
					rtl8651_getAsicEthernetPHYReg( i, 21, &snr );
					snr = snr | 0x4000;
					rtl8651_setAsicEthernetPHYReg( i, 21, snr );
				}

				if ( ((Linkpartner_eee & 0x2) ) != 0)
					port_linkpartner_eee |= (1 << i);						
				else
					port_linkpartner_eee &= ~(1 << i);						
				
				/*=========== ###02 ===========*/
				/*
				  1.      reg17 = 0x1f10¡Aread reg29, for SNR
				  2.      reg17 =  0x1f11¡Aread reg29, for AGC
				  3.      reg17 = 0x1f18¡Aread reg29, for cb0
				 */					
				// 1. for SNR
				snr = 0;
				for(val=0; val<3; val++) {
					rtl8651_getAsicEthernetPHYReg(i, 29, &agc);
					snr += agc;
				}
				snr = snr / 3;
			
				// 3. for cb0
				rtl8651_getAsicEthernetPHYReg( i, 17, &val );
				val = (val & 0xfff0) | 0x8;
				rtl8651_setAsicEthernetPHYReg( i, 17, val );					
				rtl8651_getAsicEthernetPHYReg( i, 29, &cb0 );

				// 2. for AGC
				val = (val & 0xfff0) | 0x1;
				rtl8651_setAsicEthernetPHYReg( i, 17, val );
				rtl8651_getAsicEthernetPHYReg( i, 29, &agc );

				// set bit 3~0 to 0x0 for reg 17
				val = val & 0xfff0;
				rtl8651_setAsicEthernetPHYReg( i, 17, val );

				if ( ( (    ((agc & 0x70) >> 4) < 4    ) && ((cb0 & 0x80) != 0) ) || (snr > 4150) ) { // "> 4150" = "< 18dB"
					rtl8651_restartAsicEthernetPHYNway(i);
				}

			}
			
			new_port_link_sts = new_port_link_sts | (1 << i);
		}
	}
	
	port_link_sts = new_port_link_sts;
	
	return SUCCESS;
}
#endif

#if defined (CONFIG_RTL_PHY_PATCH)
#define SNR_THRESHOLD 1000

//db = -(10 * log10(sum/262144));
//18db:SNR_THRESHOLD=4155
//20db:SNR_THRESHOLD=2621
//21db:SNR_THRESHOLD=2082
//22db:SNR_THRESHOLD=1654
//24.18db:SNR_THRESHOLD=1000
#define MAX_RESTART_NWAY_INTERVAL		(60*HZ)		
#define MAX_RESTART_NWAY_CNT		3	

struct rtl_nWayCtrl_s
{
	unsigned long restartNWayTime;
	unsigned long restartNWayCnt;
};

struct rtl_nWayCtrl_s re865x_restartNWayCtrl[RTL8651_PHY_NUMBER];
	
unsigned int re865x_getPhySnr(unsigned int port)
{
	unsigned int sum=0;
	unsigned int j; 
	unsigned int regData;

	if(port >= RTL8651_PHY_NUMBER)
	{
		return -1;
	}
		
	if (REG32(PSRP0 + (port * 4)) & PortStatusLinkUp) 
	{
		for (j=0, sum=0;j<10;j++) 
		{
			rtl8651_getAsicEthernetPHYReg(port, 29, &regData);
			sum += regData;
			mdelay(10);
		}
		sum /= 10;
		return sum;
	}
	
	return 0;
}

#if defined(CONFIG_RTL_8196C)
#if 1
static int re865x_checkPhySnr(void)
{

	unsigned int port;
	unsigned int snr=0;
	unsigned int  val, val2;
	for (port=0; port<RTL8651_PHY_NUMBER; port++)
	{
		if((1<<port) & (newLinkPortMask & (~curLinkPortMask)) )
		{
			snr=re865x_getPhySnr(port);
			//printk("%s:%d, port is %d, snr is %d\n",__FUNCTION__,__LINE__,port,snr);
			 rtl8651_getAsicEthernetPHYReg( port, 17, &val );
			 val = (val & 0xfff0) | 0x8;
			 rtl8651_setAsicEthernetPHYReg(port, 17, val );
			 
			 rtl8651_getAsicEthernetPHYReg( port, 29, &val2 );

			if( ((val2 & 0x80) != 0)|| (snr>4155))
			{
				rtl8651_restartAsicEthernetPHYNway(port);    
			}
			val = val & 0xfff0;
			rtl8651_setAsicEthernetPHYReg( port, 17, val );
			
		}
		
	}
	return 0;

}
#else
static int re865x_checkPhySnr(void)
{

	unsigned int port;
	unsigned int snr=0;

	for (port=0; port<RTL8651_PHY_NUMBER; port++)
	{
		if((1<<port) & (newLinkPortMask & (~curLinkPortMask)))
		{
			snr=re865x_getPhySnr(port);
			//printk("%s:%d, port is %d, snr is %d\n",__FUNCTION__,__LINE__,port,snr);
			if(snr>SNR_THRESHOLD)
			{
				/*poor snr, we should restart n-way*/
				if(re865x_restartNWayCtrl[port].restartNWayTime==0)
				{
					/*last time snr is good*/
					re865x_restartNWayCtrl[port].restartNWayTime=jiffies;
					re865x_restartNWayCtrl[port].restartNWayCnt=1;
			
					rtl8651_restartAsicEthernetPHYNway(port);
					
				}
				else if(time_after(jiffies,re865x_restartNWayCtrl[port].restartNWayTime+MAX_RESTART_NWAY_INTERVAL) )
				{
					/*last time SNR is bad, but interval is long enough, we can take another restart n-way action*/
					re865x_restartNWayCtrl[port].restartNWayTime=jiffies;
					re865x_restartNWayCtrl[port].restartNWayCnt=1;
					rtl8651_restartAsicEthernetPHYNway(port);
					
				}
				else if (re865x_restartNWayCtrl[port].restartNWayCnt>MAX_RESTART_NWAY_CNT)
				{
					/*within restart n-way interval and exceed max try cnt*/
					/*shouldn't do it any more, otherwise it will cause phy link up/down repeatly*/
					//printk("%s:%d,restartNWayCnt>MAX_RESTART_NWAY_CNT,stop do restart n-way\n",__FUNCTION__,__LINE__);
				}
				else
				{
					//printk("%s:%d,restartNWayCnt is %lu\n",__FUNCTION__,__LINE__,re865x_restartNWayCtrl[port].restartNWayCnt);
					re865x_restartNWayCtrl[port].restartNWayCnt++;
					rtl8651_restartAsicEthernetPHYNway(port);
				}
			}
			else
			{
				re865x_restartNWayCtrl[port].restartNWayTime=0;
				re865x_restartNWayCtrl[port].restartNWayCnt=0;
			}
			
		}
		
	}
	return 0;

}
#endif
#endif
#endif

unsigned int rtl865x_getPhysicalPortLinkStatus(void)
{
	unsigned int port_num=0;
	unsigned int linkPortMask=0;
	for(port_num=0;port_num<=RTL8651_PHY_NUMBER;port_num++)
	{
		if((READ_MEM32(PSRP0+(port_num<<2))&PortStatusLinkUp)!=0)
		{
			linkPortMask |= 1<<port_num;
		}
	
	}
	return linkPortMask;
}

#ifdef CONFIG_RTL_8198_ESD
static uint32 phy_reg30[RTL8651_PHY_NUMBER] = { 0, 0, 0, 0, 0};
static int one_second_counter = 0;
static int first_time_read_reg6 = 1;
static int need_to_check_esd2 = 1;

static int diff_more_than_1(uint32 a, uint32 b)
{
	uint32 c;

	if (a==b)
		return 0;
	if (a<b)
		{ c=a; a=b; b=c; }
	if ((a-b) >=2)
		return 1;
	else
		return 0;
}

static int esd_recovery(void)
{
	uint32 val;
	int i;

	for (i=0; i<RTL8651_PHY_NUMBER; i++) {
		/************ Link down  ************/
		if ((REG32(PSRP0 + (i * 4)) & PortStatusLinkUp) == 0) {
			phy_reg30[i] = 0;
		}
		/************ Link up  ************/
		else if (phy_reg30[i] == 0) {
			rtl8651_getAsicEthernetPHYReg( i, 22, &val );
			rtl8651_setAsicEthernetPHYReg( i, 22, ((val & (0xff00)) | 0x17) );
			rtl8651_getAsicEthernetPHYReg( i, 30, &val );
			phy_reg30[i] = BIT(31) | (val & 0xfff);
		}
	}		
	return SUCCESS;
}
#endif

#ifndef PORT0_USE_RGMII_TO_EXTCPU_MAC
#define RTL867X_PORT0_PATCH
#endif /*PORT0_USE_RGMII_TO_EXTCPU_MAC*/
#ifdef RTL867X_PORT0_PATCH
//#define RTL867X_PORT0_PATCH_BACK_TO_HALF
#ifdef RTL867X_PORT0_PATCH_BACK_TO_HALF
typedef enum{
	NOT_TRIGGERED,
	FORCE_FULL,
	FORCE_HALF,
	ALREADY_FORCE_HALF
}PORT0_STATE;

PORT0_STATE g_port0_state = NOT_TRIGGERED;

void force_full_process(void){
	#define FRAG_SEC_THR 3
	extern int32 rtl8651_returnAsicCounter(uint32 offset); 
	static unsigned int continuous_frag_sec = 0;
	static int pre_frag_cnt = 0;
	int frag_cnt;
		frag_cnt = rtl8651_returnAsicCounter( OFFSET_ETHERSTATSFRAGMEMTS_P0);
		if(continuous_frag_sec > FRAG_SEC_THR){
			printk("go to force_half state\n");
			g_port0_state = FORCE_HALF;
			continuous_frag_sec = 0;
			pre_frag_cnt = 0;
		}
		else{
			if(frag_cnt > pre_frag_cnt){
				//printk("add frag sec %u, %d, %d, %lu\n", continuous_frag_sec, pre_frag_cnt, frag_cnt, jiffies);
				continuous_frag_sec++;
				pre_frag_cnt = frag_cnt;
			}
			else{
				//printk("reset force_full state %u, %d, %d, %lu\n", continuous_frag_sec, pre_frag_cnt, frag_cnt, jiffies);
				continuous_frag_sec = 0;
				pre_frag_cnt = 0;
				//printk("reset force_full state2 %u, %d, %d\n", continuous_frag_sec, pre_frag_cnt, frag_cnt);
			}
		}
}

void force_half_process(void){
	unsigned int regData;
	printk("force_half state\n");

	if(REG32(PCRP0) & (EnablePHYIf)){
		printk("disable port0 phy\n");
		/*	disable phy */
		WRITE_MEM32(PCRP0, ((READ_MEM32(PCRP0))&(~EnablePHYIf)) ); /* Jumbo Frame */
		WRITE_MEM32(PCRP1, ((READ_MEM32(PCRP1))&(~EnablePHYIf)) ); /* Jumbo Frame */
		WRITE_MEM32(PCRP2, ((READ_MEM32(PCRP2))&(~EnablePHYIf)) ); /* Jumbo Frame */
		WRITE_MEM32(PCRP3, ((READ_MEM32(PCRP3))&(~EnablePHYIf)) ); /* Jumbo Frame */
		WRITE_MEM32(PCRP4, ((READ_MEM32(PCRP4))&(~EnablePHYIf)) ); /* Jumbo Frame */
		if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B)
		{
			WRITE_MEM32(PCRP5, ((READ_MEM32(PCRP5))&(~EnablePHYIf)) ); /* Jumbo Frame */
		}
		return;
	}

	if(REG32(P0DCR0) == 0 && REG32(P0DCR1) == 0 && REG32(P0DCR2) == 0){
		printk("port0 goes to half duplex\n");
		regData = REG32(PCRP0);
		regData &= ~(ForceDuplex);
		REG32(PCRP0) = regData;
	}

	if((REG32(PSRP0)&(PortStatusDuplex)) == 0){
		printk("go to ALREADY_FORCE_HALF state\n");
		/*	enable phy */
		WRITE_MEM32(PCRP0, ((READ_MEM32(PCRP0))|(EnablePHYIf)) ); /* Jumbo Frame */
		WRITE_MEM32(PCRP1, ((READ_MEM32(PCRP1))|(EnablePHYIf)) ); /* Jumbo Frame */
		WRITE_MEM32(PCRP2, ((READ_MEM32(PCRP2))|(EnablePHYIf)) ); /* Jumbo Frame */
		WRITE_MEM32(PCRP3, ((READ_MEM32(PCRP3))|(EnablePHYIf)) ); /* Jumbo Frame */
		WRITE_MEM32(PCRP4, ((READ_MEM32(PCRP4))|(EnablePHYIf)) ); /* Jumbo Frame */
		if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B)
		{
			WRITE_MEM32(PCRP5, ((READ_MEM32(PCRP5))|(EnablePHYIf)) ); /* Jumbo Frame */
		}
		g_port0_state = ALREADY_FORCE_HALF;
	}
}

void port0_patch_state_mechine(void){
	#define POLLING_SEC 2*HZ /*2s*/
	static unsigned long pre_jiffies = 0;
	/*although we can use counter to do this, i use jiffies to 
	create a 2 seconds timer to prevent someone making the timer
	less than 1 second.*/
	if((jiffies - pre_jiffies) >= POLLING_SEC){
		//printk("%s %d\n", __func__, __LINE__);
		switch(g_port0_state){
			case NOT_TRIGGERED:
			case ALREADY_FORCE_HALF:
				break;
			case FORCE_FULL:
				/*calculate counter and deside whether go to FORCE_HALF or not*/
				force_full_process();
				break;
			case FORCE_HALF:
				/*do what FORCE_HALF need to do*/
				force_half_process();
				break;
		}
		pre_jiffies = jiffies;
	}
}
#endif
/*
krammer add this according to " Jim Hsieh/5458 "'s algorithm
Description:
when phy0's link partner link in force mode, we will force to half duplex mode,
in this condition, if there is some traffic in port0's rx, sometimes port0 will die.
so we add this patch, only work when phy0's link state change, if link partner
is link in force mode, we will change our port0 into full duplex.
*/
void port0_force_patch(void){
	unsigned int regData;

	if((curLinkPortMask & 0x01) == (newLinkPortMask & 0x01)){
		//no change -> return
		return;
	}
	
	//default disable phy i/f(0xbb804104 b0=0)
	REG32(PCRP0) &= ~(EnablePHYIf);

	//if port#0=link up
	if(newLinkPortMask & 0x01){
		//if AN =0(reg6[0])
		rtl8651_getAsicEthernetPHYReg(rtl8651_tblAsicDrvPara.externalPHYId[0], 6, &regData);
		if((regData & 0x01) == 0){
			rtl8651_getAsicEthernetPHYReg(rtl8651_tblAsicDrvPara.externalPHYId[0], 5, &regData);
			if(regData & 0x080){//if spd = 100M(reg5[7])
				//force MAC 100M Full duplex
				//(set 0xbb804104 b25=1 b24=1 b23=0 b20-19=1 b18=1 b17-16=0x3)
				regData = REG32(PCRP0);
				regData |= (EnForceMode | PollLinkStatus | ForceDuplex | PauseFlowControlEtxErx);
				regData &= ~(ForceLink | ForceSpeedMask);
				regData |= (ForceSpeed100M);
				printk("phy0 link partner is force 100M, we force to 100M Full duplex!regData = 0x%x\n", regData);
				REG32(PCRP0) = regData;
			}
			else if(regData & 0x20){//else spd = 10M(reg5[5])
				//force MAC 10M Full duplex
				//(set 0xbb804104 b25=1 b24=1 b23=0 b20-19=0 b18=1 b17-16=0x3)
				regData = REG32(PCRP0);
				regData |= (EnForceMode | PollLinkStatus | ForceDuplex | PauseFlowControlEtxErx);
				regData &= ~(ForceLink | ForceSpeedMask);
				regData |= (ForceSpeed10M);
				printk("phy0 link partner is force 10M, we force to 10M Full duplex!regData = 0x%x\n", regData);
				REG32(PCRP0) = regData;
			}
			#ifdef RTL867X_PORT0_PATCH_BACK_TO_HALF
			printk("go to force_full state\n");
			g_port0_state = FORCE_FULL;
			#endif
		}
		//enable phy i/f
		//set 0xbb804104 b0=1
		REG32(PCRP0) |= EnablePHYIf;
	}
	else{//if port#0=link down
		//recovery MAC AN setting and disable phy i/f
		//(set 0xbb804104 b25=0 b20-18 0x7 b0=0)
		regData = REG32(PCRP0);
		regData &= ~(EnForceMode | EnablePHYIf);
		regData |= (ForceSpeed1000M | ForceDuplex);
		printk("phy0 link down, we reset phy0!regData = 0x%x\n", regData);
		REG32(PCRP0) = regData;
		#ifdef RTL867X_PORT0_PATCH_BACK_TO_HALF
		g_port0_state = NOT_TRIGGERED;
		#endif
	}
}
#endif

static void interrupt_dsr_link(unsigned long task_priv)
{	
#ifdef CONFIG_RTL_8198_ESD
	esd_recovery();
#endif

#ifdef CONFIG_RTL8196C_ETH_IOT
/*LinkDownEventFlag is a read clear bit, so re865x_setPhyGrayCode() should be call headmost */
		re865x_setPhyGrayCode();
#endif

	newLinkPortMask=rtl865x_getPhysicalPortLinkStatus();
	
#if defined(CONFIG_RTL_MULTI_LAN_DEV) && defined(CONFIG_RTL_8676HWNAT)
{
	unsigned char LINK_UP_NUM=0; 
	uint32 sharedON=0,sharedOFF=0, fcON=0,fcOFF=0;
	if(RTL8676_TBLASIC_EXTPHYPROPERTY_PORT0_RTL8367B != global_probe_extPhy)
	{
		int port;
		struct net_device *dev;
		for(port = 0; port < RTL8651_PORT_NUMBER; ++port)
		{
			if((dev = ethtool_get_netdev_by_rtl_sw_port(&_rtl86xx_dev, port)) != NULL)
			{
				if((1 << port) & newLinkPortMask){
					netif_carrier_on(dev);
					LINK_UP_NUM++;
				}else{
					netif_carrier_off(dev);
				}	
			}
		}
	}
	
	if(LINK_UP_NUM<=2){
		//REG32(0xbb804504)=0x00c000d0;
		//REG32(0xbb804508)=0x00a800c0;
		fcON=0xd0;
		fcOFF=0xc0;
		sharedON=0xc0;
		sharedOFF=0xa8;	
		//printk("LINK_UP_NUM<=2");
	}else{
		//REG32(0xbb804504)=0x00A000AC;
		//REG32(0xbb804508)=0x004A0062;
		fcON=0xac;
		fcOFF=0xa0;
		sharedON=0x62;
		sharedOFF=0x4a;			
		//printk("LINK_UP_NUM>=2");
	}
	
	WRITE_MEM32(SBFCR1, (READ_MEM32(SBFCR1) & ~(S_DSC_FCON_MASK | S_DSC_FCOFF_MASK)) | ( fcON<< S_DSC_FCON_OFFSET) | (fcOFF << S_DSC_FCOFF_OFFSET));
	WRITE_MEM32(SBFCR2, (READ_MEM32(SBFCR2) & ~(S_Max_SBuf_FCON_MASK | S_Max_SBuf_FCOFF_MASK)) | (sharedON << S_Max_SBuf_FCON_OFFSET) | (sharedOFF << S_Max_SBuf_FCOFF_OFFSET));

	
}	
#endif

#if defined(CONFIG_RTL_IGMP_SNOOPING)
		rtl865x_igmpSyncLinkStatus();
#endif

#if defined(CONFIG_RTL_LINKCHG_PROCESS) 
		rtl865x_LinkChange_Process();
#endif

#if defined(CONFIG_RTL_8196C) && defined(CONFIG_RTL_PHY_PATCH)
		re865x_checkPhySnr();
#endif

#ifdef CONFIG_RTL_NLMSG_PROTOCOL
	rtl_nl_send_lkchg_msg(curLinkPortMask, newLinkPortMask);
#endif

#ifdef RTL867X_PORT0_PATCH
	port0_force_patch();
#endif

	curLinkPortMask=newLinkPortMask;

#ifdef LINK_TASKLET		
	//REG32(CPUIIMR) |= (LINK_CHANGE_IP);
#endif

	return;
}

__IRAM_FWD
irqreturn_t interrupt_isr(int irq, void *dev_instance)
{
	struct net_device *dev = dev_instance;
	struct dev_priv *cp;
	unsigned int status;
	cp = dev->priv;
	status = REG32(CPUIISR);
	REG32(CPUIISR) = status;
	status &= REG32(CPUIIMR);

	if (status & (RX_DONE_IP_ALL|MBUF_DESC_RUNOUT_IP_ALL|PKTHDR_DESC_RUNOUT_IP_ALL))
	{
		#if defined(RX_TASKLET)
		REG32(CPUIIMR) &= ~(RX_DONE_IE_ALL | TX_ALL_DONE_IE_ALL | PKTHDR_DESC_RUNOUT_IE_ALL | MBUF_DESC_RUNOUT_IE_ALL); 
		rtl_rxSetTxDone(FALSE);
		tasklet_schedule(&cp->rx_dsr_tasklet);
		#else
		interrupt_dsr_rx((unsigned long)cp);
		#endif
	}	

	


	if (status & TX_ALL_DONE_IP_ALL)
	{
  		#if defined(TX_TASKLET)
		rtl_rxSetTxDone(FALSE);
		tasklet_schedule(&cp->tx_dsr_tasklet);						
  		#else
		interrupt_dsr_tx((unsigned long)cp);
 		#endif
	}

	#if	defined(CONFIG_RTL_IGMP_SNOOPING)||defined(CONFIG_RTL_LINKCHG_PROCESS) || defined (CONFIG_RTL_PHY_PATCH)
	if (status & LINK_CHANGE_IP)
	{
  		#ifdef LINK_TASKLET		
  		//REG32(CPUIIMR) &= ~LINK_CHANGE_IP;
		tasklet_schedule(&cp->link_dsr_tasklet);						
  		#else
		interrupt_dsr_link((unsigned long)cp);
  		#endif
	}
	#endif

	#if !defined(RX_TASKLET) || !defined(TX_TASKLET)
	REG32(CPUIISR) = (MBUF_DESC_RUNOUT_IP_ALL|PKTHDR_DESC_RUNOUT_IP_ALL);
	#endif

	return IRQ_HANDLED;
}

static int rtl865x_init_hw(void)
{
	unsigned int mbufRingSize;
	int	i;

	mbufRingSize = rtl865x_rxSkbPktHdrDescNum;	/* rx ring 0	*/
	for(i=1;i<RTL865X_SWNIC_RXRING_HW_PKTDESC;i++)
	{
		mbufRingSize += rxRingSize[i];
	}

	/* Initialize NIC module */
	if (swNic_init(rxRingSize, mbufRingSize, txRingSize, MBUF_LEN))
	{
		printk("865x-nic: swNic_init failed!\n");            
		return FAILED;
	}
			
	return SUCCESS;	
}

#ifdef CONFIG_RTL_HARDWARE_NAT		
static void reset_hw_mib_counter(struct net_device *dev)
{
	int i, port;
	int32 totalVlans=((sizeof(vlanconfig))/(sizeof(struct rtl865x_vlanConfig)))-1;

	for(i=0;i<totalVlans;i++)
	{
		if (IF_ETHER!=vlanconfig[i].if_type)
		{
			continue;
		}
		if (!memcmp(vlanconfig[i].mac.octet, dev->dev_addr, 6))
		{
			for (port=0; port<RTL8651_AGGREGATOR_NUMBER; port++)
			{
				if (vlanconfig[i].memPort & (1<<port))
				   WRITE_MEM32(MIB_CONTROL, (1<<port*2) | (1<<(port*2+1)));
				return;
			}
		}
	}
}
#endif

#if defined(DYNAMIC_ADJUST_TASKLET) || defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL8196C_REVISION_B) || defined(CONFIG_RTL_8198) || defined(RTL8196C_EEE_MAC) || defined(RTL_CPU_QOS_ENABLED)
static void one_sec_timer(unsigned long task_priv)
{
	unsigned long		flags;
	struct dev_priv	*cp;

	cp = ((struct net_device *)task_priv)->priv;

	spin_lock_irqsave (&cp->lock, flags);

#if defined(PATCH_GPIO_FOR_LED)

//  if (((struct net_device *)task_priv)->name[3] == '0' ) {//
    if(alias_name_are_eq((struct net_device *)task_priv)->name,ALIASNAME_ETH0,ALIASNAME_ELAN_PREFIX)){
		int port;

		for (port=0; port<RTL8651_PHY_NUMBER; port++) {
			update_mib_counter(port);		
			calculate_led_interval(port);
			if (led_cb[port].link_status & GPIO_LINK_STATE_CHANGE) 
			{
				gpio_led_Interval_timeout(port);
			}
		}
	}	
#endif

#if defined(DYNAMIC_ADJUST_TASKLET)
//if (((struct net_device *)task_priv)->name[3] == '0'    // eth0  
if(alias_name_are_eq(((struct net_device *)task_priv)->name,ALIASNAME_ETH0,ALIASNAME_ELAN_PREFIX)
    && rx_pkt_thres > 0 && ((eth_flag&TASKLET_MASK) == 0)){ 
        if (rx_cnt > rx_pkt_thres) {
			if (!rx_tasklet_enabled) {
				rx_tasklet_enabled = 1;
            }							
        }               
        else {
			if (rx_tasklet_enabled) { // tasklet enabled
				rx_tasklet_enabled = 0;
			}						
        }       
        rx_cnt = 0;     
    }   
#endif

#ifdef CONFIG_RTL8186_TR
    cp->tx_avarage = (cp->tx_avarage/10)*7 + (cp->tx_byte_cnt/10)*3;
    if (cp->tx_avarage > cp->tx_peak)
        cp->tx_peak = cp->tx_avarage;
    cp->tx_byte_cnt = 0;
    
    cp->rx_avarage = (cp->rx_avarage/10)*7 + (cp->rx_byte_cnt/10)*3;
    if (cp->rx_avarage > cp->rx_peak)
        cp->rx_peak = cp->rx_avarage;
    cp->rx_byte_cnt = 0;    
#endif

#ifdef CONFIG_RTL8196C_REVISION_B
	if ((REG32(REVR) == RTL8196C_REVISION_A) && (eee_enabled)) {
			int i, curr_sts;
			uint32 reg;
	
			/* 
				prev_port_sts[] = 0, current = 0	:	do nothing
				prev_port_sts[] = 0, current = 1	:	update prev_port_sts[]
				prev_port_sts[] = 1, current = 0	:	update prev_port_sts[], disable EEE
				prev_port_sts[] = 1, current = 1	:	enable EEE if EEE is disabled
			 */
			for (i=0; i<MAX_PORT_NUMBER; i++)
			{
				curr_sts = (REG32(PSRP0 + (i * 4)) & PortStatusLinkUp) >> 4;
	
				if ((prev_port_sts[i] == 1) && (curr_sts == 0)) {
					// disable EEE MAC
					REG32(EEECR) = (REG32(EEECR) & ~(0x1f << (i * 5))) | 
						((EN_P0_FRC_EEE|FRC_P0_EEE_100) << (i * 5));
					//printk("	disable EEE for port %d\n", i);
				}
				else if ((prev_port_sts[i] == 1) && (curr_sts == 1)) {
					reg = REG32(EEECR);
					if ((reg & (1 << (i * 5))) == 0) {
						// enable EEE MAC
						REG32(EEECR) = (reg & ~(0x1f << (i * 5))) | 
							((FRC_P0_EEE_100|EN_P0_TX_EEE|EN_P0_RX_EEE) << (i * 5));
						//printk("	enable EEE for port %d\n", i);
					}
				}			
				prev_port_sts[i] = curr_sts;
			}
			//printk(" %d %d %d %d %d\n", port_sts[0], port_sts[1], port_sts[2], port_sts[3], port_sts[4]);
		}
#endif

#if defined(RTL_CPU_QOS_ENABLED)
	totalLowQueueCnt = 0;
#endif

#ifdef CONFIG_RTL_8198_ESD
#if defined(CONFIG_PRINTK)
#define panic_printk         printk
#endif
	{
	int phy;
	uint32 val;
	if (first_time_read_reg6) {
		first_time_read_reg6 = 0;

		for (phy=0; phy<5; phy++) {
			rtl8651_setAsicEthernetPHYReg( phy, 31, 5  );	
			rtl8651_getAsicEthernetPHYReg(phy, 1, &val);
			rtl8651_setAsicEthernetPHYReg(phy, 1, val | 0x4);

			rtl8651_setAsicEthernetPHYReg(phy, 5, 0xfff6);
			rtl8651_getAsicEthernetPHYReg(phy, 6, &val);
			rtl8651_setAsicEthernetPHYReg( phy, 31, 0  );

		if ((val & 0xff) == 0xFF)
			need_to_check_esd2 = 0;
		}		
	}

	if (++one_second_counter >= 10) {
		for (phy=0; phy<5; phy++)
		{
			if (phy_reg30[phy] != 0) {
				rtl8651_setAsicEthernetPHYReg( phy, 31, 5  );
				rtl8651_setAsicEthernetPHYReg(phy, 5, 0);

				rtl8651_getAsicEthernetPHYReg(phy, 6, &val);
				rtl8651_setAsicEthernetPHYReg( phy, 31, 0  );
	
				if ((val & 0xffff) != 0xAE04)
				{
					panic_printk("  ESD-1\n");
					do {} while(1); // reboot
				}
	
				if (need_to_check_esd2) {
					rtl8651_setAsicEthernetPHYReg( phy, 31, 5  );
					rtl8651_setAsicEthernetPHYReg(phy, 5, 0xfff6);

					rtl8651_getAsicEthernetPHYReg(phy, 6, &val);
					rtl8651_setAsicEthernetPHYReg( phy, 31, 0  );

					if ((val & 0xff) != 0xFC)
					{
						panic_printk("  ESD-2\n");
						do {} while(1); // reboot
					}
				}

				rtl8651_getAsicEthernetPHYReg( phy, 22, &val );
				rtl8651_setAsicEthernetPHYReg( phy, 22, ((val & (0xff00)) | 0x17) );
				rtl8651_getAsicEthernetPHYReg( phy, 30, &val );

				if ((phy_reg30[phy] & 0xfff) != (val & 0xfff)) {
					if (diff_more_than_1((phy_reg30[phy] & 0xf), (val & 0xf)) || 
						diff_more_than_1(((phy_reg30[phy] >> 4) & 0xf), ((val >> 4) & 0xf)) ||
						diff_more_than_1(((phy_reg30[phy] >> 8) & 0xf), ((val >> 8) & 0xf))
						) {
						panic_printk("  ESD-3: old= 0x%x, new= 0x%x\n", phy_reg30[phy] & 0xfff, val & 0xfff);
						do {} while(1); // reboot
					}
					phy_reg30[phy] = BIT(31) | (val & 0xfff);					
				}
			}

		}		
		
		one_second_counter = 0;
	}
	}
#endif

#ifdef CONFIG_RTL_8196C_ESD
#if defined(CONFIG_PRINTK)
#define panic_printk         printk
#endif
	if (_96c_esd_counter) {

		if (++_96c_esd_counter >= 20) {
			extern int is_fault;
	
			if( (RTL_R32(PCRP4) & EnablePHYIf) == 0)
			{
				panic_printk("  ESD reboot...\n");
				is_fault = 1;
			}	
			_96c_esd_counter = 1;
		}
	}
#endif

#ifdef RTL867X_PORT0_PATCH_BACK_TO_HALF
	port0_patch_state_mechine();
#endif

	mod_timer(&cp->expire_timer, jiffies + HZ);

	spin_unlock_irqrestore(&cp->lock, flags);   
}
#endif

static struct net_device *irqDev=NULL;
static int re865x_open (struct net_device *dev)
{
	struct dev_priv *cp;
	unsigned long flags;
	int rc;

	cp = dev->priv;
	if (cp->opened)
		return SUCCESS;
	
	/*	The first device be opened	*/
	if (atomic_read(&rtl_devOpened)==0)
	{
		/* this is the first open dev */
		spin_lock_irqsave (&cp->lock, flags);
		rtk_queue_init(&rx_skb_queue); 
		refill_rx_skb();
		rc = rtl865x_init_hw();
		spin_unlock_irqrestore(&cp->lock, flags);
		if (rc) {
			printk("rtl865x_init_hw() failed!\n");
			return FAILED;
		}

#if defined(RX_TASKLET)
		tasklet_init(&cp->rx_dsr_tasklet, (void *)interrupt_dsr_rx, (unsigned long)cp);
#endif
#ifdef TX_TASKLET
		tasklet_init(&cp->tx_dsr_tasklet, interrupt_dsr_tx, (unsigned long)cp);
#endif

#ifdef LINK_TASKLET
		tasklet_init(&cp->link_dsr_tasklet, interrupt_dsr_link, (unsigned long)cp);
#endif

#ifdef CONFIG_RTL_PHY_PATCH
		memset(re865x_restartNWayCtrl,0, sizeof(re865x_restartNWayCtrl));
#endif

		{	
			printk("%s %d, %s\n", __func__, __LINE__, dev->name);
			init_timer(&cp->expire_timer);
			cp->expire_timer.expires = jiffies + HZ;
			cp->expire_timer.data = (unsigned long)dev;
			cp->expire_timer.function = one_sec_timer;
			mod_timer(&cp->expire_timer, jiffies + HZ); 	
		}

		rc = request_irq(dev->irq, interrupt_isr, IRQF_DISABLED, dev->name, dev);
		if (rc)
		{
			printk("request_irq() error!\n");
			goto err_out_hw;
		}
		irqDev=dev;
		//cp->irq_owner =1;
		rtl865x_start();
	}

	atomic_inc(&rtl_devOpened);
	cp->opened = 1;

#ifdef CONFIG_RTL_HARDWARE_NAT	
	reset_hw_mib_counter(dev);
#endif

	netif_start_queue(dev);

#if 0//krammer move up
#if defined(DYNAMIC_ADJUST_TASKLET) || defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL8196C_REVISION_B)|| defined(CONFIG_RTL_8198) || defined(RTL8196C_EEE_MAC)
	#if !defined(CONFIG_RTL8186_TR)

//    if (dev->name[3] == '0') //
if(alias_name_are_eq(dev->name,ALIASNAME_ETH0,ALIASNAME_ELAN_PREFIX)||(alias_name_are_eq(dev->name,ALIASNAME_NAS0,ALIASNAME_MWNAS)))
	#endif
	{	
		init_timer(&cp->expire_timer);
		cp->expire_timer.expires = jiffies + HZ;
		cp->expire_timer.data = (unsigned long)dev;
		cp->expire_timer.function = one_sec_timer;
		mod_timer(&cp->expire_timer, jiffies + HZ);		
#ifdef DYNAMIC_ADJUST_TASKLET			
		rx_cnt = 0;
#endif
	}
#endif
#endif

#if 0
#ifdef CONFIG_RTL_LAYERED_DRIVER_L3
	/*FIXME_hyking: should add default route to cpu....*/
	if(rtl865x_curOpMode == GATEWAY_MODE)
#if defined(CONFIG_RTL_PUBLIC_SSID)
		rtl865x_addRoute(0,0,0,RTL_GW_WAN_DEVICE_NAME,0);
#else
#ifndef CONFIG_RTL_MULTI_ETH_WAN
		rtl865x_addRoute(0,0,0,RTL_DRV_WAN0_NETIF_NAME,0);
#endif
#endif
#endif
#endif

	rtl865x_enableDevPortForward( dev, cp);	

	return SUCCESS;
	
err_out_hw:
    rtl8186_stop_hw(dev, cp);
    rtl865x_down();
    return rc;
}

static int re865x_close (struct net_device *dev)
{
	struct dev_priv *cp;

	cp = dev->priv;
//	cp = netdev_priv(dev);
	if (!cp->opened)
		return SUCCESS;

	rtl865x_disableDevPortForward(dev, cp);
	netif_stop_queue(dev);
	rtl8186_stop_hw(dev, cp);

	/* The last opened device	*/
	if (atomic_read(&rtl_devOpened)==1)
	{
		/*	warning:
			1.if we don't reboot,we shouldn't hold switch core from rx/tx, otherwise there will be some problem during change operation mode
			2.only when two devices go down,can we shut down nic interrupt
			3.the interrupt will be re_enable by rtl865x_start()
		*/
		WRITE_MEM32(PCRP0, READ_MEM32(PCRP0) & ~EnablePHYIf);
		WRITE_MEM32(PCRP1, READ_MEM32(PCRP1) & ~EnablePHYIf);
		WRITE_MEM32(PCRP2, READ_MEM32(PCRP2) & ~EnablePHYIf);
		WRITE_MEM32(PCRP3, READ_MEM32(PCRP3) & ~EnablePHYIf);
		WRITE_MEM32(PCRP4, READ_MEM32(PCRP4) & ~EnablePHYIf);
		rtl865x_disableInterrupt();
		
		//free_irq(dev->irq, GET_IRQ_OWNER(cp));
		//((struct dev_priv *)((GET_IRQ_OWNER(cp))->priv))->irq_owner = 0;
		free_irq(dev->irq, irqDev);
		//((struct dev_priv *)(irqDev->priv))->irq_owner = 0;
		

#ifdef RX_TASKLET
		tasklet_kill(&cp->rx_dsr_tasklet);	
#endif

#ifdef TX_TASKLET
		tasklet_kill(&cp->tx_dsr_tasklet);
#endif

#ifdef LINK_TASKLET
		tasklet_kill(&cp->link_dsr_tasklet);
#endif

#ifdef CONFIG_RTL_PHY_PATCH
		memset(re865x_restartNWayCtrl,0, sizeof(re865x_restartNWayCtrl));
#endif
		free_rx_skb();
	}

	memset(&cp->net_stats, '\0', sizeof(struct net_device_stats));
	atomic_dec(&rtl_devOpened);
	cp->opened = 0;

#if defined(DYNAMIC_ADJUST_TASKLET) || defined(CONFIG_RTL8186_TR) || defined(BR_SHORTCUT) || defined(CONFIG_RTL8196C_REVISION_B) || defined(CONFIG_RTL_8198)
    if (timer_pending(&cp->expire_timer))
        del_timer_sync(&cp->expire_timer);
#endif

#ifdef BR_SHORTCUT
	if (dev == cached_dev)
		cached_dev=NULL;
#endif

#ifdef CONFIG_RTL_HARDWARE_NAT	
	reset_hw_mib_counter(dev);
#endif

	return SUCCESS;
}

#if defined(CONFIG_RTL_STP) || (defined(CONFIG_RTL_CUSTOM_PASSTHRU))
static int re865x_pseudo_open (struct net_device *dev)
{
	struct dev_priv *cp;

	cp = dev->priv;
	//cp = netdev_priv(dev);
	if (cp->opened)
		return SUCCESS;
	
	cp->opened = 1;	
	netif_start_queue(dev);
	return SUCCESS;
}


static int re865x_pseudo_close (struct net_device *dev)
{
	struct dev_priv *cp;

	cp = dev->priv;
//	cp = netdev_priv(dev);

	if (!cp->opened)
		return SUCCESS;
	netif_stop_queue(dev);

	memset(&cp->net_stats, '\0', sizeof(struct net_device_stats));
	cp->opened = 0;

#ifdef BR_SHORTCUT
	if (dev == cached_dev)
		cached_dev=NULL;
#endif
	return SUCCESS;
}
#endif

#if defined(CONFIG_RTL_STP)
static int re865x_stp_mapping_init(void)
{
	int i, j, k, totalVlans;
	totalVlans=((sizeof(vlanconfig))/(sizeof(struct rtl865x_vlanConfig)))-1;
	
	for (i = 0; i < MAX_RE865X_STP_PORT; i++)
	{
		STP_PortDev_Mapping[i] = NO_MAPPING; 
	}
	
	#ifdef CONFIG_RTK_MESH
	STP_PortDev_Mapping[WLAN_PSEUDO_IF_INDEX] = WLAN_PSEUDO_IF_INDEX;
	STP_PortDev_Mapping[WLAN_MESH_PSEUDO_IF_INDEX] = WLAN_MESH_PSEUDO_IF_INDEX;
	#else
	STP_PortDev_Mapping[MAX_RE865X_STP_PORT -1] = WLAN_PSEUDO_IF_INDEX;
	#endif
	
	j = 0;
	for(k=0;k<totalVlans;k++)
	{
		if (vlanconfig[k].isWan == FALSE)
		{
			#ifdef CONFIG_RTK_MESH
			for(i=0; i< MAX_RE865X_ETH_STP_PORT ; i++)
			#else
			for(i=0; i< MAX_RE865X_STP_PORT-1 ; i++)
			#endif	
			{
				if ( (1<<i) & vlanconfig[k].memPort ) 
				{
					STP_PortDev_Mapping[j] = i;
					j++;
				}
			}
			
			break;
		}
	}

#if 0	
		printk("mapping table is ");
#ifdef CONFIG_RTK_MESH
	for(i=0; i< MAX_RE865X_ETH_STP_PORT ; i++)
#else
	for(i=0; i< MAX_RE865X_STP_PORT-1 ; i++)
#endif		
	{
		printk(" %d ", STP_PortDev_Mapping[i]);
	}
		printk("\n\n");
#endif
	return SUCCESS;
}

static int re865x_stp_mapping_reinit(void)
{
	int i, j, k, totalVlans;
	totalVlans=((sizeof(vlanconfig))/(sizeof(struct rtl865x_vlanConfig)))-1;
	
	for (i = 0; i < MAX_RE865X_STP_PORT; i++)
	{
		STP_PortDev_Mapping[i] = NO_MAPPING; 
	}

	#ifdef CONFIG_RTK_MESH
	STP_PortDev_Mapping[WLAN_PSEUDO_IF_INDEX] = WLAN_PSEUDO_IF_INDEX;
	STP_PortDev_Mapping[WLAN_MESH_PSEUDO_IF_INDEX] = WLAN_MESH_PSEUDO_IF_INDEX;
	#else
	STP_PortDev_Mapping[MAX_RE865X_STP_PORT -1] = WLAN_PSEUDO_IF_INDEX;
	#endif

	printk("=======stp port dev mapping reinit=======\n");
	j = 0;
	for(k=0;k<totalVlans;k++)
	{
		if (vlanconfig[k].isWan == FALSE)
		{
			#ifdef CONFIG_RTK_MESH
			for(i=0; i< MAX_RE865X_ETH_STP_PORT; i++)
			#else
			for(i=0; i< MAX_RE865X_STP_PORT-1 ; i++)
			#endif
			{
				if ( (1<<i) & vlanconfig[k].memPort ) 
				{
					STP_PortDev_Mapping[j] = i;
					printk("mapping: lan phycisal [port%d] <====>pseudo [port%d]\n", i, j);
					j++;
				}
			}
			
			break;
		}
	}
	
		printk("mapping table is ");
	#ifdef CONFIG_RTK_MESH
	for(i=0; i< MAX_RE865X_ETH_STP_PORT; i++)
	#else
	for(i=0; i< MAX_RE865X_STP_PORT-1 ; i++)
	#endif	
	{
		printk(" %d ", STP_PortDev_Mapping[i]);
	}
		printk("\n\n");
	return SUCCESS;
}

static int re865x_stp_get_pseudodevno(uint32 port_num)
{
	int i, dev_no;
	for(i=0; i< MAX_RE865X_STP_PORT-1 ; i++)
	{
		if( STP_PortDev_Mapping[i] == port_num)
		{
			dev_no = i;
			return dev_no;
		}		
	}
	return NO_MAPPING;

}

#endif

#if defined(CONFIG_RTL_LOCAL_PUBLIC)
//hyking:this function should move to rtl865x_fdb.c
//implement it at here just for releaae to natami...
//2010-02-22
static int32 rtl865x_getPortlistByMac(const unsigned char *mac,uint32 *portlist)
{
	int32 found = FAILED;
	ether_addr_t *macAddr;
	int32 column;
	rtl865x_tblAsicDrv_l2Param_t	fdbEntry;

	macAddr = (ether_addr_t *)(mac);
	found = rtl865x_Lookup_fdb_entry(0, macAddr, FDB_DYNAMIC, &column, &fdbEntry);
	if(found == SUCCESS)
	{
		if(portlist)
			*portlist = fdbEntry.memberPortMask;
	}

	return found;
	
}
#endif



//#if defined(CONFIG_RTK_VLAN_SUPPORT) && defined(CONFIG_RTK_VLAN_FOR_CABLE_MODEM)
struct net_device* re865x_get_netdev_by_name(const char* name)
{
	int i;
	for(i = 0; i < ETH_INTF_NUM; i++)
	{
		if(strcmp(_rtl86xx_dev.dev[i]->name,name) == 0)
			return _rtl86xx_dev.dev[i];
	}
	return NULL;
}
//#endif

//__MIPS16
#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
static inline int rtl_process_passthru_tx(rtl_nicTx_info *txInfo)
{
	struct net_device *dev;
	struct sk_buff *skb = NULL;	
 	struct dev_priv *cp;
	uint16	etherType;
	
	if(oldStatus)
	{
		skb = txInfo->out_skb;
		dev = skb->dev;
		if (dev==_rtl86xx_dev.pdev)
		{
			etherType = *((unsigned short *)(skb->data+(ETH_ALEN<<1)));
			if ((((oldStatus&IP6_PASSTHRU_MASK)&&(*((uint16*)&skb->data[ETH_ALEN<<1])==__constant_htons(ETH_P_IPV6))) ||
			((oldStatus&PPPOE_PASSTHRU_MASK)&&((*((uint16*)&skb->data[ETH_ALEN<<1])==__constant_htons(ETH_P_PPP_SES))||
			(*((uint16*)&skb->data[ETH_ALEN<<1])==__constant_htons(ETH_P_PPP_DISC)))) ))
			{
				cp = _rtl86xx_dev.pdev->priv;
				skb->dev=cp->dev;

			}
			else
			{
				dev_kfree_skb_any(skb);
					return FAILED;
			}
		}
	}
	
	return SUCCESS;
}
#endif

#if defined(CONFIG_RTK_VLAN_SUPPORT)
static inline int rtl_process_rtk_vlan_tx(rtl_nicTx_info *txInfo)
{
	struct net_device *dev;
	struct sk_buff *skb = NULL;	
	struct sk_buff *newskb;
	struct dev_priv *cp;
	if(rtk_vlan_support_enable)
	{	
		skb = txInfo->out_skb;
		dev = skb->dev;
		cp = dev->priv;
		if (cp->vlan_setting.global_vlan) 
		{
			newskb = NULL;
			if (skb_cloned(skb))
			{
				newskb = skb_copy(skb, GFP_ATOMIC);
				if (newskb == NULL) 
				{
					cp->net_stats.tx_dropped++;
					dev_kfree_skb_any(skb);		
					return FAILED;
				}
				dev_kfree_skb_any(skb);
				skb = newskb;
				txInfo->out_skb = skb;
			}
			
			if (tx_vlan_process(dev, &cp->vlan_setting, skb, 0)) 
			{
				cp->net_stats.tx_dropped++;
				dev_kfree_skb_any(skb);
				return FAILED;				
			}
		}
	}
	return SUCCESS;
}
#endif

static inline int rtl_pstProcess_xmit(struct dev_priv *cp,int len)
{	
#ifndef CONFIG_RTL_NIC_HWSTATS
	cp->net_stats.tx_packets++;		
	cp->net_stats.tx_bytes += len;
#endif	
	cp->dev->trans_start = jiffies;
	return SUCCESS;
}

static inline int rtl_preProcess_xmit(rtl_nicTx_info *txInfo)
{
	int retval = SUCCESS;
	#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
	retval = rtl_process_passthru_tx(txInfo);
	if(FAILED == retval)
		return retval;
	#endif
	#if defined(CONFIG_RTK_VLAN_SUPPORT)
	retval = rtl_process_rtk_vlan_tx(txInfo);
	if(FAILED == retval)
		return retval;
	#endif
	return retval;
}
			
static inline void rtl_direct_txInfo(uint32 port_mask,rtl_nicTx_info *txInfo)
{
	txInfo->portlist = port_mask & 0x3f;
	txInfo->srcExtPort = 0;		//PKTHDR_EXTPORT_LIST_CPU;
	txInfo->flags = (PKTHDR_USED|PKT_OUTGOING);	
}

static inline void rtl_hwLookup_txInfo(rtl_nicTx_info *txInfo)
{
	txInfo->portlist = RTL8651_CPU_PORT;		/* must be set 0x7 */
	txInfo->srcExtPort = PKTHDR_EXTPORT_LIST_CPU;
	txInfo->flags = (PKTHDR_USED|PKTHDR_HWLOOKUP|PKTHDR_BRIDGING|PKT_OUTGOING);
}
static inline int rtl_fill_txInfo(rtl_nicTx_info *txInfo)
{
	struct sk_buff *skb = txInfo->out_skb;
	struct dev_priv *cp;
	cp = skb->dev->priv;

	if (!txInfo->vid) {
#ifndef CONFIG_RTL_MULTI_ETH_WAN
		txInfo->vid = cp->id;
#else
		if (skb->dev->priv_flags & IFF_DOMAIN_WAN) {
			txInfo->vid = skb->vlan_tci&VLAN_VID_MASK;
		}
		else
			txInfo->vid = cp->id;
#endif
	}
	if(!txInfo->vid)
		return FAILED;

	#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
	txInfo->priority= 0;
	#endif
	txInfo->txIdx = 0;
	
	if((skb->data[0]&0x01)==0)
	{
#if defined(CONFIG_RTL_MULTI_LAN_DEV) || defined(CONFIG_POCKET_ROUTER_SUPPORT)
		{
#ifdef CONFIG_RTL8676_Static_ACL
			if(is_from_not_nicport_under_bridge(skb))				   
				_rtl865x_addFilterDatabaseEntry(RTL865x_L2_TYPEI, RTL_LAN_FID,	(ether_addr_t *)(&skb->data[ETHER_ADDR_LEN]), FDB_TYPE_FWD, 0x100, FALSE, FALSE);
#endif
				rtl_direct_txInfo(cp->portmask,txInfo);
		}
#elif defined(CONFIG_RTK_VLAN_SUPPORT)
		   if(rtk_vlan_support_enable ==1)
                {
                        /* the pkt must be tx to lan vlan */				
				rtl_direct_txInfo(cp->portmask,txInfo);
                }
		   else
		   {
		#if defined (CONFIG_RTL_LOCAL_PUBLIC)
			//hyking:
			//when hw local public and sw localpublic exist at same time,
			//pkt to sw local public would be trap to cpu by default ACL
			//2010-02-22
			if(rtl_isWanDev(cp)!=TRUE)
			{
				uint32 portlist;				
				//hyking: default acl issue, direct tx now...
				if(rtl865x_getPortlistByMac(skb->data,&portlist) == SUCCESS)
					rtl_direct_txInfo(portlist,txInfo);					
				else
					rtl_direct_txInfo(cp->portmask,txInfo);
					
			}

		#else
			if (rtl_isWanDev(cp)!=TRUE)
			{
				/* the pkt must be tx to lan vlan */
				rtl_hwLookup_txInfo(txInfo);				
			}
			
		#endif			
			else
			{
				rtl_direct_txInfo(cp->portmask,txInfo);
			}
		   }
#else  //(CONFIG_RTL_MULTI_LAN_DEV) ||defined (CONFIG_POCKET_ROUTER_SUPPORT)
		{	/*unicast process*/
		#if defined (CONFIG_RTL_LOCAL_PUBLIC)
			//hyking:
			//when hw local public and sw localpublic exist at same time,
			//pkt to sw local public would be trap to cpu by default ACL
			//2010-02-22
			if(rtl_isWanDev(cp)!=TRUE)
			{
				uint32 portlist;				
				//hyking: default acl issue, direct tx now...
				if(rtl865x_getPortlistByMac(skb->data,&portlist) == SUCCESS)
					rtl_direct_txInfo(portlist,txInfo);					
				else
					rtl_direct_txInfo(cp->portmask,txInfo);
					
			}

		#else
			//if (rtl_isWanDev(cp)!=TRUE)
			//{
				rtl_hwLookup_txInfo(txInfo);
			//}
			
		#endif
			//else
			//{
			//	rtl_direct_txInfo(cp->portmask,txInfo);
			//}		
		}
#endif
	}
	else
	{
#if 0
		/*multicast process*/
	#if defined (CONFIG_RTL_MULTI_LAN_DEV)
	#ifdef CONFIG_RTL8676_Static_ACL
		if(is_from_not_nicport_under_bridge(skb))
			_rtl865x_addFilterDatabaseEntry(RTL865x_L2_TYPEI, RTL_LAN_FID,	(ether_addr_t *)(&skb->data[ETHER_ADDR_LEN]), FDB_TYPE_FWD, 0x100, FALSE, FALSE);
	#endif
	#else
		_rtl865x_addFilterDatabaseEntry(RTL865x_L2_TYPEI, RTL_LAN_FID,	(ether_addr_t *)(&skb->data[ETHER_ADDR_LEN]), FDB_TYPE_FWD, 0x100, FALSE, FALSE);
	#endif
#endif
rtl_direct_txInfo(cp->portmask,txInfo);
	

#ifdef CONFIG_RTL_STP
		if(!cp->dev->irq)
		{
			//virtual interfaces have no IRQ assigned. We use this to identify STP port interfaces.
			uint8 stpPortNum= cp->dev->name[strlen(cp->dev->name)-1]-'0';
/*			printk("send bpdu packet in nic, port num is %d\n", STP_PortDev_Mapping[stpPortNum]);	*/
			if (STP_PortDev_Mapping[stpPortNum] != NO_MAPPING)
			{
				txInfo->portlist = 1<<STP_PortDev_Mapping[stpPortNum];
			}
			else
			{
				//august: when return FAILED, we will do dev_free_skb in the caller func
				//dev_kfree_skb_any(skb);
				return FAILED;
			}
		}
#endif
	}
	if(txInfo->portlist==0)
	{
		//august: when return FAILED, we will do dev_free_skb in the caller func
		//dev_kfree_skb_any(skb);
		return FAILED;
	}
	return SUCCESS;
}
#ifdef CONFIG_RTL_MULTI_ETH_WAN
static int insert_vlantag(struct sk_buff **skb,unsigned int vid)
{
	struct sk_buff *tmpskb;
	int i;
	unsigned char insert[16]; 

	if(skb_cloned(*skb))
	{
		tmpskb = skb_copy(*skb, GFP_ATOMIC);
		dev_kfree_skb_any(*skb);
		if (tmpskb == NULL)
			return 0;
		*skb = tmpskb;
	}

	if (skb_headroom(*skb) < 4) {
		tmpskb = skb_realloc_headroom(*skb, 4);
		dev_kfree_skb_any(*skb);
		if (tmpskb == NULL)
			return 0;
		*skb = tmpskb;
	}
	
	for(i = 0 ; i<12 ; i++)
	{
		insert[i] = (*skb)->data[i];
	}
	insert[12] = 0x81;
	insert[13] = 0x00;
	
#ifdef CONFIG_E8B
	#ifdef CONFIG_NETFILTER
	if ((*skb)->mark & (1<<16)) { // marked by IPQoS
		insert[14] = (((*skb)->mark&0x7)<<5) | (vid>>8);
	} else 
	#endif
		insert[14] = vid>>8;
#else
	#ifdef CONFIG_NETFILTER
	if(((*skb)->mark&0xffff)>>8>=1)
		insert[14] = (((*skb)->mark&0x7)<<5) | (vid>>8);
	else		
		insert[14] = ((((*skb)->vlan_tci)>>13)<<5) | (vid>>8);		
	#else
	insert[14] = vid>>8;
	#endif
#endif
	insert[15] = vid&0xff;
	
	skb_push(*skb,4);

	for(i = 0 ; i<16 ; i++)
	{
		(*skb)->data[i] = insert[i];
	}

	return 1;
}
#endif

#ifdef CONFIG_PORT_MIRROR
static int re865x_start_xmit(struct sk_buff *skb, struct net_device *dev);

//this func is specially used for sar mirror 
static int re865x_mirror_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct dev_priv *cp;
	struct sk_buff *tx_skb = skb;

	rtl_nicTx_info	nicTx;

	int tx_retry = 3;

	cp = (struct dev_priv*)dev->priv; 
	memset(&nicTx, 0, sizeof(rtl_nicTx_info));

	nicTx.out_skb = skb;

	if((cp->id == 0) || (cp->portmask == 0))
	{
		printk("%s %d: drop skb.\n", __func__, __LINE__);
		dev_kfree_skb_any(tx_skb);
		return 0;
	}

	//fill its vid as we refer it in sw_send
	nicTx.vid = cp->id;

	rtl_direct_txInfo(cp->portmask, &nicTx);


	//last check
	if(0 == nicTx.portlist)
	{
		printk("%s %d: drop skb.\n", __func__, __LINE__);
		dev_kfree_skb_any(tx_skb);
		return 0;	
	}

	// Done in _swNic_send()
	//_dma_cache_wback_inv((unsigned long) tx_skb->data, tx_skb->len);

	while (--tx_retry && (swNic_send((void *)tx_skb, tx_skb->data, tx_skb->len, &nicTx) < 0)) 
	{
		swNic_txDone(nicTx.txIdx);
	}

	if (!tx_retry) 
	{
		//cp->net_stats.tx_dropped++;
		dev_kfree_skb_any(tx_skb);
		return 0;
	}

	rtl_pstProcess_xmit(cp, tx_skb->len);

    return 0;	
}

void nic_tx_mirror (struct sk_buff *skb)
{
    struct net_device *dev;// = skb->dev;
	struct dev_priv	*dp;// = dev->priv;
	unsigned long flags;

	if(skb->dev)
		dev = skb->dev;
	else
		dev = _rtl86xx_dev.dev[0];

	if(dev->priv_flags ==IFF_DOMAIN_WAN)
		dev = _rtl86xx_dev.dev[0];

	dp = dev->priv;

	spin_lock_irqsave(&dp->lock, flags);
	re865x_mirror_start_xmit(skb, dev);
	spin_unlock_irqrestore(&dp->lock, flags);
}
#endif


static int re865x_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	int retval = FAILED;
	struct dev_priv *cp;
	struct sk_buff *tx_skb=skb;
	rtl_nicTx_info	nicTx;
	int tx_retry = 3;

	memset(&nicTx, 0, sizeof(rtl_nicTx_info));

#ifdef CONFIG_RTL867X_VLAN_MAPPING
	if (re_vlan_loaded()) {
		unsigned short vid = 0;
		retval = re_vlan_skb_xmit(skb, &vid);

		if (NET_XMIT_DROP == retval)
			return 0;

		if (vid)
			nicTx.vid = vid;
	}
#endif

	nicTx.out_skb = skb;
	retval = rtl_preProcess_xmit(&nicTx);
	if(FAILED == retval)
		return 0;
	
	tx_skb = nicTx.out_skb;
	cp = tx_skb->dev->priv;	

#ifdef CONFIG_RTL_8676HWNAT
#ifndef CONFIG_NEW_PORTMAPPING
#ifndef CONFIG_RTL_MULTI_ETH_WAN
	if(enable_port_mapping){
		if(skb->vlan_member!=0 && skb->vlan_member != cp->port_member){
			dev_kfree_skb_any(tx_skb);
			return 0;
		}
	}
#endif
#endif
#endif

	if(
#ifndef CONFIG_RTL_MULTI_ETH_WAN
		(cp->id==0) || 
#else
		((tx_skb->dev->priv_flags & IFF_DOMAIN_ELAN) && (cp->id==0)) || 
#endif
		(cp->portmask ==0))
	{
		printk("%s %d: drop skb.\n", __func__, __LINE__);
		dev_kfree_skb_any(tx_skb);
		return 0;
	}


	//printk("\n%s on dev %s\n", __func__, tx_skb->dev->name);

//#if defined (CONFIG_RTL_IGMP_SNOOPING) || defined(CONFIG_RTL_HW_PURE_SWITCH)

	retval = rtl_fill_txInfo(&nicTx);
	if(FAILED == retval) {
		printk("%s %d: drop skb.\n", __func__, __LINE__);
		dev_kfree_skb_any(tx_skb);
		return 0;	
	}


#ifdef CONFIG_RTL_MULTI_ETH_WAN
#ifndef CONFIG_NEW_PORTMAPPING
	if (enable_port_mapping)
	{

		int port=0;
		
		if ((tx_skb->from_dev && (tx_skb->from_dev->priv_flags&(IFF_DOMAIN_ELAN|IFF_DOMAIN_WLAN))) &&
			(tx_skb->dev->priv_flags&IFF_DOMAIN_WAN)) {//upstream
			if (tx_skb->from_dev->priv_flags & IFF_DOMAIN_ELAN)
			{
                TOKEN_NUM(tx_skb->from_dev->name,&port);
//				sscanf(tx_skb->from_dev->name, "eth0.%d", &port);                        
				port -= 1;
			}
			else if (tx_skb->from_dev->priv_flags & IFF_DOMAIN_WLAN)
			{
                TOKEN_NUM(tx_skb->from_dev->name,&port);
//				sscanf(tx_skb->from_dev->name, "wlan0-vap%d", &port);
			}
			if (!(tx_skb->vlan_member & (1<<port)))
			{
				printk("upstream packet[%s -> %s] block by port mapping rule (member 0x%x).\n", 
					tx_skb->from_dev->name, tx_skb->dev->name, tx_skb->vlan_member);
				dev_kfree_skb_any(tx_skb);
				return 0;
			}
		}
		else if (tx_skb->from_dev && (tx_skb->from_dev->priv_flags&IFF_DOMAIN_WAN))
		{		  
			if (!smuxDownstreamPortmappingCheck(tx_skb))
			{
				printk("downstream packet[%s -> %s] block by port mapping rule (member 0x%x).\n", 
					tx_skb->from_dev->name, tx_skb->dev->name, tx_skb->vlan_member);
				dev_kfree_skb_any(tx_skb);
				return 0;
			}
		}		
	}
#endif
	
	if ((RTL_WANVLANID != nicTx.vid) && (RTL_LANVLANID != nicTx.vid) && (RTL_BridgeWANVLANID != nicTx.vid)) {
		if (!insert_vlantag(&tx_skb, nicTx.vid))
			return 0;
	}
#endif

	// Done in _swNic_send()
	//_dma_cache_wback_inv((unsigned long) tx_skb->data, tx_skb->len);

	while (--tx_retry && (swNic_send((void *)tx_skb, tx_skb->data, tx_skb->len, &nicTx) < 0)) {
		swNic_txDone(nicTx.txIdx);
	}
	
	if (!tx_retry) {
		cp->net_stats.tx_dropped++;
		dev_kfree_skb_any(tx_skb);
		return 0;
	}
	
//#else //CONFIG_RTL_IGMP_SNOOPING
//	#error "By default should define CONFIG_RTL_IGMP_SNOOPING"
//#endif	
  
	rtl_pstProcess_xmit(cp,tx_skb->len);
	//cp->net_stats.tx_packets++;		
	//cp->net_stats.tx_bytes += tx_skb->len;

    return 0;
}


static void re865x_tx_timeout (struct net_device *dev)
{	
	rtlglue_printf("Tx Timeout!!! Can't send packet\n");
}

#if defined(RTL819X_PRIV_IOCTL_ENABLE)
int rtl819x_get_port_status(int portnum , struct lan_port_status *port_status)
{	
		uint32	regData;
		uint32	data0;

		if( portnum < 0 ||  portnum > CPU)
			return -1;

		regData = READ_MEM32(PSRP0+((portnum)<<2));

		//printk("rtl819x_get_port_status port = %d data=%x\n", portnum,regData); //mark_debug
		data0 = regData & PortStatusLinkUp;		
		if (data0)
			port_status->link =1;
		else
			port_status->link =0;
				
		data0 = regData & PortStatusNWayEnable;
		if (data0)
			port_status->nway=1;
		else
			port_status->nway =0;

		data0 = regData & PortStatusDuplex;
		if (data0)
			port_status->duplex=1;
		else
			port_status->duplex =0;

		data0 = (regData&PortStatusLinkSpeed_MASK)>>PortStatusLinkSpeed_OFFSET;
		port_status->speed = data0 ; // 0 = 10M , 1= 100M , 2=1G ,

	       return 0;	
}

int rtl819x_get_port_stats(int portnum , struct port_statistics *port_stats)
{

	uint32 addrOffset_fromP0 =0;	

	//printk("rtl819x_get_port_stats port = %d \n", portnum); //mark_debug
	if( portnum < 0 ||  portnum > CPU)
			return -1;

	addrOffset_fromP0 = portnum * MIB_ADDROFFSETBYPORT;
	
	//port_stats->rx_bytes =(uint32) (rtl865xC_returnAsicCounter64( OFFSET_IFINOCTETS_P0 + addrOffset_fromP0 )) ;
	port_stats->rx_bytes =rtl8651_returnAsicCounter( OFFSET_IFINOCTETS_P0 + addrOffset_fromP0 ) ;
	port_stats->rx_unipkts= rtl8651_returnAsicCounter( OFFSET_IFINUCASTPKTS_P0 + addrOffset_fromP0 ) ;
	port_stats->rx_mulpkts= rtl8651_returnAsicCounter( OFFSET_ETHERSTATSMULTICASTPKTS_P0 + addrOffset_fromP0 ) ;
	port_stats->rx_bropkts= rtl8651_returnAsicCounter( OFFSET_ETHERSTATSBROADCASTPKTS_P0 + addrOffset_fromP0 ) ;
	port_stats->rx_discard= rtl8651_returnAsicCounter( OFFSET_DOT1DTPPORTINDISCARDS_P0 + addrOffset_fromP0 ) ;
	port_stats->rx_error= (rtl8651_returnAsicCounter( OFFSET_DOT3STATSFCSERRORS_P0 + addrOffset_fromP0 ) +
						 rtl8651_returnAsicCounter( OFFSET_ETHERSTATSJABBERS_P0 + addrOffset_fromP0 ));
						
	//port_stats->tx_bytes =(uint32) (rtl865xC_returnAsicCounter64( OFFSET_IFOUTOCTETS_P0 + addrOffset_fromP0 )) ;
	port_stats->tx_bytes =rtl8651_returnAsicCounter( OFFSET_IFOUTOCTETS_P0 + addrOffset_fromP0 ) ;
	port_stats->tx_unipkts= rtl8651_returnAsicCounter( OFFSET_IFOUTUCASTPKTS_P0 + addrOffset_fromP0 ) ;
	port_stats->tx_mulpkts= rtl8651_returnAsicCounter( OFFSET_IFOUTMULTICASTPKTS_P0 + addrOffset_fromP0 ) ;
	port_stats->tx_bropkts= rtl8651_returnAsicCounter( OFFSET_IFOUTBROADCASTPKTS_P0 + addrOffset_fromP0 ) ;
	port_stats->tx_discard= rtl8651_returnAsicCounter( OFFSET_IFOUTDISCARDS + addrOffset_fromP0 ) ;
	port_stats->tx_error= (rtl8651_returnAsicCounter( OFFSET_ETHERSTATSCOLLISIONS_P0 + addrOffset_fromP0 ) +
						 rtl8651_returnAsicCounter( OFFSET_DOT3STATSDEFERREDTRANSMISSIONS_P0 + addrOffset_fromP0 ));
	
	return 0;	
}
#ifdef CONFIG_RTL_HW_QOS_SUPPORT
#if 0
#define DBG_PRINT(fmt, args...) printk(fmt, ##args)
void qos_queue_dbg(struct rtl867x_hwnat_qos_queue qdisc){
	DBG_PRINT("==================================\n");
	switch(qdisc.action) {
		case QDISC_ENABLE:
			DBG_PRINT("[HWNAT]Qdisc Enabled: sp_num=%d wrr_num=%d bandwidth=%d default_queue=%d\n",
				qdisc.sp_num, qdisc.wrr_num, qdisc.bandwidth, qdisc.default_queue);
			break;
		case QDISC_DISABLE:
			DBG_PRINT("[HWNAT]Qdisc Disabled.\n");
			break;
		default:
			break;
	}
	DBG_PRINT("\n");
}

void qos_rule_dbg(struct rtl867x_hwnat_qos_rule rule){
	DBG_PRINT("==================================\n");
	DBG_PRINT("rule type: %u\t q_index: %u\n", rule.rule_type, rule.q_index);
	DBG_PRINT("remark_8021p: 0x%x\t remark_dscp: 0x%x\n", rule.remark_8021p, rule.remark_dscp);
	switch(rule.rule_type){
		case RTL867x_IPQos_Format_srcPort:
			DBG_PRINT("inIfname: %s\n", rule.Format_SRCPort_NETIFNAME);
			break;
		case RTL867x_IPQos_Format_Ethernet:
			break;
		case RTL867x_IPQos_Format_IP:
			DBG_PRINT("sip: 0x%x\t sipMask: 0x%x\n", rule.Format_IP_SIP, rule.Format_IP_SIP_M);
			DBG_PRINT("dip: 0x%x\t dipMask: 0x%x\n", rule.Format_IP_DIP, rule.Format_IP_DIP_M);
			DBG_PRINT("tos: 0x%x\t tosMask: 0x%x\n", rule.Format_IP_TOS, rule.Format_IP_TOS_M);
			DBG_PRINT("ipProto: 0x%x\t ipProtoMask: 0x%x\n", rule.Format_IP_Proto, rule.Format_IP_Proto_M);
			break;
		case RTL867x_IPQos_Format_TCP:
			DBG_PRINT("sip: 0x%x\t sipMask: 0x%x\n", rule.Format_TCP_SIP, rule.Format_TCP_SIP_M);
			DBG_PRINT("dip: 0x%x\t dipMask: 0x%x\n", rule.Format_TCP_DIP, rule.Format_TCP_DIP_M);
			DBG_PRINT("tos: 0x%x\t tosMask: 0x%x\n", rule.Format_TCP_TOS, rule.Format_TCP_TOS_M);
			DBG_PRINT("sPort_start: 0x%x\t sPort_end: 0x%x\n", rule.Format_TCP_SPORT_Sta, rule.Format_TCP_SPORT_End);
			DBG_PRINT("dPort_start: 0x%x\t dPort_end: 0x%x\n", rule.Format_TCP_DPORT_Sta, rule.Format_TCP_DPORT_End);
			break;
		case RTL867x_IPQos_Format_UDP:
			DBG_PRINT("sip: 0x%x\t sipMask: 0x%x\n", rule.Format_UDP_SIP, rule.Format_UDP_SIP_M);
			DBG_PRINT("dip: 0x%x\t dipMask: 0x%x\n", rule.Format_UDP_DIP, rule.Format_UDP_DIP_M);
			DBG_PRINT("tos: 0x%x\t tosMask: 0x%x\n", rule.Format_UDP_TOS, rule.Format_UDP_TOS_M);
			DBG_PRINT("sPort_start: 0x%x\t sPort_end: 0x%x\n", rule.Format_UDP_SPORT_Sta, rule.Format_UDP_SPORT_End);
			DBG_PRINT("dPort_start: 0x%x\t dPort_end: 0x%x\n", rule.Format_UDP_DPORT_Sta, rule.Format_UDP_DPORT_End);
			break;
		case RTL867x_IPQos_Format_8021p:
			DBG_PRINT("vlanTagPri: %u\n", rule.Format_8021P_PRIORITY);
			break;
		default:
			break;
	}
	DBG_PRINT("\n");
}
#else
#define DBG_PRINT(fmt, args...) 
void qos_queue_dbg(struct rtl867x_hwnat_qos_queue qdisc){
}
void qos_rule_dbg(struct rtl867x_hwnat_qos_rule rule){
}
#endif

static int hwnat_qos_qdisc(struct rtl867x_hwnat_qos_queue qdisc)
{
	switch(qdisc.action) {
		case QDISC_ENABLE:
			rtl8676_IPQos_Enable(qdisc.sp_num, qdisc.wrr_num, qdisc.ceil,
				qdisc.weight, qdisc.default_queue);
			break;
		case QDISC_DISABLE:
			rtl8676_IPQos_Disable();
			break;
		default:
			break;
	}
	return 0;
}

int hwnat_qos_ioctl(struct hwnat_ioctl_qos_cmd *data){
	struct hwnat_ioctl_qos_cmd cmd;
	int ret;
 	if (copy_from_user(&cmd, data, sizeof(struct hwnat_ioctl_qos_cmd)))
	{
		return -EFAULT;
	}
	DBG_PRINT("hwnat_qos_ioctl cmd type: %u\n", cmd.type);
	switch(cmd.type){
		case OUTPUTQ_CMD:
			qos_queue_dbg(cmd.u.qos_queue);
			hwnat_qos_qdisc(cmd.u.qos_queue);
			break;
		case QOSRULE_CMD:
			qos_rule_dbg(cmd.u.qos_rule.qos_rule_pattern);
			rtl8676_IPQos_AddRule(&cmd.u.qos_rule.qos_rule_pattern
				,cmd.u.qos_rule.qos_rule_queue_index
				,cmd.u.qos_rule.qos_rule_remark_8021p
				,cmd.u.qos_rule.qos_rule_remark_dscp);
			break;
		default:
			ret = -EFAULT;
			break;
	}
	return ret;
}
#endif // of CONFIG_RTL_HW_QOS_SUPPORT

#if defined(CONFIG_RTL_HARDWARE_NAT) && defined(CONFIG_RTL_LAYERED_DRIVER_L3)
int hwnat_extip_ioctl(struct hwnat_ioctl_extip_cmd *data){
	struct hwnat_ioctl_extip_cmd cmd;
	int ret;
 	if (copy_from_user(&cmd, data, sizeof(struct hwnat_ioctl_extip_cmd)))
	{
		return -EFAULT;
	}

	switch(cmd.extip_rule.action){
		case ADD_EXTIP_CMD:
			//printk("add ip:%x intf:%s!!!\n", cmd.extip_rule.ip, cmd.extip_rule.name);
			rtl8676_add_napt_extIp(cmd.extip_rule.name, cmd.extip_rule.ip);
			break;
		case DEL_EXTIP_CMD:
			//printk("del ip:%x intf:%s!!!\n", cmd.extip_rule.ip, cmd.extip_rule.name);
			rtl8676_del_napt_extIp(cmd.extip_rule.name, cmd.extip_rule.ip);
			break;
		default:
			ret = -EFAULT;
			break;
	}
	return ret;
}
#endif

int hwnat_ioctl(hwnat_ioctl_cmd *data){
 	hwnat_ioctl_cmd cmd;
	int ret;
 	if (copy_from_user(&cmd, data, sizeof(hwnat_ioctl_cmd)))
	{
		return -EFAULT;
	}
	switch(cmd.type){
#ifdef CONFIG_RTL_HW_QOS_SUPPORT
		case HWNAT_IOCTL_QOS_TYPE:
			ret = hwnat_qos_ioctl((struct hwnat_ioctl_qos_cmd *)cmd.data);
			break;
#endif
#if defined(CONFIG_RTL_HARDWARE_NAT) && defined(CONFIG_RTL_LAYERED_DRIVER_L3)
		case HWNAT_IOCTL_EXTIP_TYPE:
			ret = hwnat_extip_ioctl((struct hwnat_ioctl_extip_cmd*)cmd.data);
			break;
#endif
		default:
			ret = -EFAULT;
			break;
	}
	return ret;
}
 
int re865x_priv_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
	int32 rc = 0;
	unsigned long *data_32;
	int portnum=0;
	struct lan_port_status port_status;
	struct port_statistics port_stats;

	data_32 = (unsigned long *)rq->ifr_data;
	if (copy_from_user(&portnum, data_32, 1*sizeof(unsigned long)))
	{
		return -EFAULT;
	}
	
	switch (cmd)
	{
		 case SIOCETHTEST:
			eth_ctl((struct eth_arg *)rq->ifr_data);
			break;
	     case RTL819X_IOCTL_READ_PORT_STATUS:		 	
		 	rc = rtl819x_get_port_status(portnum,&port_status); //portnumber	
		 	if(rc != 0)
				return -EFAULT;
		 	if (copy_to_user((void *)rq->ifr_data, (void *)&port_status, sizeof(struct lan_port_status)))
				return -EFAULT;
		 	break;
	     case RTL819X_IOCTL_READ_PORT_STATS:
		 	rc = rtl819x_get_port_stats(portnum,&port_stats); //portnumber	
		 	if(rc != 0)
				return -EFAULT;
		 	if (copy_to_user((void *)rq->ifr_data, (void *)&port_stats, sizeof(struct port_statistics)))
				return -EFAULT;
		 	break;
		case RTL819X_IOCTL_HWNAT:
			hwnat_ioctl((hwnat_ioctl_cmd *)rq->ifr_data);
			break;
	     default :
		 	rc = -EOPNOTSUPP;
			break;
	}	
	return SUCCESS;

}

#endif

//for RTL8676 port mapping
#ifdef CONFIG_RTL_8676HWNAT
void set_port_isolation_member(int port, unsigned int member){
	switch(port){
		case 0:
			WRITE_MEM32(PIR0, (READ_MEM32(PIR0)&(~PI_PORT0_MASK))|(member<<PI_PORT0_OFFSET));
			break;
		case 1:
			WRITE_MEM32(PIR0, (READ_MEM32(PIR0)&(~PI_PORT1_MASK))|(member<<PI_PORT1_OFFSET));
			break;
		case 2:
			WRITE_MEM32(PIR0, (READ_MEM32(PIR0)&(~PI_PORT2_MASK))|(member<<PI_PORT2_OFFSET));
			break;
		case 3:
			WRITE_MEM32(PIR1, (READ_MEM32(PIR1)&(~PI_PORT3_MASK))|(member<<PI_PORT3_OFFSET));
			break;
		case 4:
			WRITE_MEM32(PIR1, (READ_MEM32(PIR1)&(~PI_PORT4_MASK))|(member<<PI_PORT4_OFFSET));
			break;
		case 5:
			WRITE_MEM32(PIR1, (READ_MEM32(PIR1)&(~PI_PORT5_MASK))|(member<<PI_PORT5_OFFSET));
			break;
			
	}
}

int set_port_mapping(struct net_device *dev, struct ifreq *rq, int cmd)
{
#ifndef CONFIG_RTL_MULTI_ETH_WAN
	int totalVlans, i;
#endif	
	struct ifvlan *ifv = rq->ifr_data;

#ifndef CONFIG_RTL_MULTI_ETH_WAN
	if(ifv->enable == 0){
		//printk("clear all port mapping setting\n");
		enable_port_mapping=0;
		WRITE_MEM32(PIR0,(PI_PORT0_MASK|PI_PORT1_MASK|PI_PORT2_MASK));
		WRITE_MEM32(PIR1,(PI_PORT3_MASK|PI_PORT4_MASK|PI_PORT5_MASK));
	}
	else{
		//printk("set port mapping setting for specific port");
		enable_port_mapping=1;
		totalVlans=((sizeof(vlanconfig))/(sizeof(struct rtl865x_vlanConfig)))-1;
		for(i=0;i<totalVlans;i++){
			if(vlanconfig[i].memPort == (1<<ifv->port)){
				((struct dev_priv *)(_rtl86xx_dev.dev[i]->priv))->port_member = ifv->member;
				set_port_isolation_member(ifv->port, ifv->member&0x1f);
			}			
		}
	}
#else
	printk("port mapping %s: dev %s member:0x%x\n", ifv->enable?"enable":"disable", dev->name, ifv->member);
	/*
	 * QL port mapping mechanism: all packets are trapped to cpu by default, if it pass through firewall and port mapping rule, then
	 * acl rule will be written to asic table and portmapping will take effect.
	 */
	if ((ifv->enable == 0) && enable_port_mapping)
	{
		enable_port_mapping = 0;
		//printk("rtl865x_syncRouteToAsic\n");
		rtl865x_syncRouteToAsic();
	}
	else if ((ifv->enable == 1) && !enable_port_mapping)
	{
		enable_port_mapping = 1;
		/* clear routing asic table. */
				//printk("rtl865x_clearAsicRoutingTable\n");
		rtl865x_clearAsicRoutingTable();
	}

	/* reset fastpath */
	rtl867x_clearFastPathEntry();

	/* reset hw-acc */
#ifdef CONFIG_RTL_HARDWARE_NAT
	#ifdef CONFIG_RTL8676_Static_ACL
		rtl865x_nat_init();
	#else /* CONFIG_RTL8676_Dynamic_ACL */	
		rtl8676_clean_L34Unicast_hwacc();			
	#endif
#endif

	
#endif

	return 0;
}
#endif


int re865x_ioctl (struct net_device *dev, struct ifreq *rq, int cmd)
{
	int32 rc = 0;
	unsigned long *data;
	int32 args[4];
	int32  * pRet;
	#if defined(CONFIG_RTL8186_KB)||defined(CONFIG_RTL8186_GR)
	uint32	*pU32;
	#endif

#ifdef CONFIG_RTL_8676HWNAT
	if(cmd == SIOCSITFGROUP){
		rc = set_port_mapping(dev, rq, cmd);
		return rc;
	}
#endif

	if (cmd != SIOCDEVPRIVATE)
	{
		#if defined(RTL819X_PRIV_IOCTL_ENABLE)
		rc = re865x_priv_ioctl(dev,rq,cmd); 
 		return rc;
		#else
 		goto normal;
		#endif
	}

	data = (unsigned long *)rq->ifr_data;

	if (copy_from_user(args, data, 4*sizeof(unsigned long)))
	{
		return -EFAULT;
	}

	switch (args[0])
	{

#ifdef CONFIG_RTL8196_RTL8366
		case RTL8651_IOCTL_GETWANLINKSTATUS:
			{
				uint32 phyNum;
				uint32 linkStatus;

				pRet = (int32 *)args[3];
				*pRet = FAILED;
				rc = SUCCESS;

				phyNum=4;//8366 WAN port
				rc=rtl8366rb_getPHYLinkStatus(phyNum, &linkStatus);

				if(rc==SUCCESS)
				{
					if(linkStatus==1)
					{
						*pRet = SUCCESS;
					}
				}

				break;
			}
#else
		case RTL8651_IOCTL_GETWANLINKSTATUS:
			{
				int i;
				int wanPortMask;
				int32 totalVlans;

				pRet = (int32 *)args[3];
				*pRet = FAILED;
				rc = SUCCESS;

				wanPortMask = 0;
				totalVlans=((sizeof(vlanconfig))/(sizeof(struct rtl865x_vlanConfig)))-1;
				for(i=0;i<totalVlans;i++)
				{
					if(vlanconfig[i].isWan==TRUE)
						wanPortMask = vlanconfig[i].memPort;
				}

				if (wanPortMask==0)
				{
					/* no wan port exist */					
					break;
				}

				for(i=0;i<RTL8651_AGGREGATOR_NUMBER;i++)
				{
					if( (1<<i)&wanPortMask )
					{
						if((READ_MEM32(PSRP0+(i<<2))&PortStatusLinkUp)!=0)
						{
							*pRet = SUCCESS;
						}
						break;
					}
				}
				
				break;
			}
#endif
		case RTL8651_IOCTL_GETWANLINKSPEED:
			{
				int i;
				int wanPortMask;
				int32 totalVlans;
					
				pRet = (int32 *)args[3];
				*pRet = FAILED;
				rc = FAILED;

				wanPortMask = 0;
				totalVlans=((sizeof(vlanconfig))/(sizeof(struct rtl865x_vlanConfig)))-1;
				for(i=0;i<totalVlans;i++)
				{
					if(vlanconfig[i].isWan==TRUE)
						wanPortMask = vlanconfig[i].memPort;
				}

				if (wanPortMask==0)
				{
					/* no wan port exist */
					break;
				}

				for(i=0;i<RTL8651_AGGREGATOR_NUMBER;i++)
				{
					if( (1<<i)&wanPortMask )
					{
						break;
					}
				}
				
				switch(READ_MEM32(PSRP0 + (i<<2)) & PortStatusLinkSpeed_MASK)
				{
					case PortStatusLinkSpeed10M:
						*pRet = PortStatusLinkSpeed10M;
						rc = SUCCESS;
						break;
					case PortStatusLinkSpeed100M:
						*pRet = PortStatusLinkSpeed100M;
						rc = SUCCESS;
						break;
					case  PortStatusLinkSpeed1000M:
						*pRet = PortStatusLinkSpeed1000M;
						rc = SUCCESS;
						break;
					default:
						break;
				}
				break;
			}
#if defined(CONFIG_RTL8186_KB)|| defined(CONFIG_RTL8186_GR)
		case RTL8651_IOCTL_GETLANLINKSTATUS:
			{
				int i;
				int lanPortMask;
				int32 totalVlans;

				pRet = (int32 *)args[3];
				*pRet = FAILED;
				rc = SUCCESS;

				lanPortMask = 0;
				totalVlans=((sizeof(vlanconfig))/(sizeof(struct rtl865x_vlanConfig)))-1;
				for(i=0;i<totalVlans;i++)
				{
					if(vlanconfig[i].isWan==FALSE)
					{
						lanPortMask = vlanconfig[i].memPort;
						if (lanPortMask==0)
						{
							/* no wan port exist */					
							continue;
						}

						for(i=0;i<=RTL8651_PHY_NUMBER;i++)
						{
							if( (1<<i)&lanPortMask )
							{
								if((READ_MEM32(PSRP0+(i<<2))&PortStatusLinkUp)!=0)
								{
									//rtlglue_printf("Lan port i=%d\n",i);//Added for test
									*pRet = SUCCESS;
									return rc;
								}
							}
						}
					}
				}

				break;
			}
#if defined(CONFIG_RTL8186_KB)
		case RTL8651_IOCTL_GETWANTHROUGHPUT:
			{
				static unsigned long last_jiffies = 0;
				static unsigned long last_rxtx = 0;
				int i;
				int32 totalVlans;
				struct dev_priv *cp;
				int32 *throughputLevel;
				unsigned long	diff_jiffies;
				pRet = (int32 *)args[3];

				diff_jiffies = (jiffies-last_jiffies);
				if (diff_jiffies>HZ)
				{
					pU32 = (uint32*)args[1];
					throughputLevel = (uint32*)pU32[0];
					rc = SUCCESS;

					cp = NULL;
					totalVlans=((sizeof(vlanconfig))/(sizeof(struct rtl865x_vlanConfig)))-1;
					for(i=0;i<totalVlans;i++)
					{
						if(vlanconfig[i].isWan==TRUE)
						{
							cp = _rtl86xx_dev.dev[i]->priv;
							break;
						}
					}

					if (cp==NULL)
					{
						/* no wan port exist */					
						rc = FAILED;
					}

					for(i=1;i<20;i++)
					{
						if (diff_jiffies < (HZ<<i))
							break;
					}
					/* get the throughput level */
					*throughputLevel = (((cp->net_stats.rx_bytes + cp->net_stats.tx_bytes)-last_rxtx)>>(17+i));

					last_jiffies = jiffies;
					last_rxtx = (cp->net_stats.rx_bytes + cp->net_stats.tx_bytes);
					*pRet = SUCCESS;
				}
				else
				{
					*pRet = FAILED;
				}
			}
#endif
#endif
#ifdef CONFIG_RTL_LAYERED_DRIVER
#if defined(CONFIG_RTL8186_GR)
		case RTL8651_IOCTL_SETWANLINKSTATUS:
			{
				int i;
				int wanPortMask;
				int32 totalVlans;
				int portStatusToSet;
				int forceMode;
				int forceLink;
				int forceLinkSpeed;
				int forceDuplex;
				uint32 regValue;
				uint32 advCapability;
				#define SPEED10M 	0
				#define SPEED100M 	1
				#define SPEED1000M 	2
				
				pRet = (int32 *)args[3];
				*pRet = FAILED;
				rc = SUCCESS;
				
				pU32 = (uint32*)args[1];
				portStatusToSet = *(uint32*)pU32[0];
				
				wanPortMask = 0;
				totalVlans=((sizeof(vlanconfig))/(sizeof(struct rtl865x_vlanConfig)))-1;
				for(i=0;i<totalVlans;i++)
				{
					if(vlanconfig[i].isWan==TRUE)
						wanPortMask = vlanconfig[i].memPort;
				}

				if (wanPortMask==0)
				{
					/* no wan port exist */					
					break;
				}

				for(i=0;i<RTL8651_AGGREGATOR_NUMBER;i++)
				{
					if( (1<<i)&wanPortMask )
					{
						/*write register*/
			
						if(HALF_DUPLEX_10M == portStatusToSet)
						{
							forceMode=TRUE;
							forceLink=TRUE;
							forceLinkSpeed=SPEED10M;
							forceDuplex=FALSE;
							advCapability=(1<<HALF_DUPLEX_10M);
						}else if(HALF_DUPLEX_100M == portStatusToSet)
						{
							forceMode=TRUE;
							forceLink=TRUE;
							forceLinkSpeed=SPEED100M;
							forceDuplex=FALSE;
							advCapability=(1<<HALF_DUPLEX_100M);
						}else if(HALF_DUPLEX_1000M == portStatusToSet)
						{
							forceMode=TRUE;
							forceLink=TRUE;
							forceLinkSpeed=SPEED1000M;
							forceDuplex=FALSE;
							advCapability=(1<<HALF_DUPLEX_1000M);
						}else if(DUPLEX_10M == portStatusToSet)
						{
							forceMode=TRUE;
							forceLink=TRUE;
							forceLinkSpeed=SPEED10M;
							forceDuplex=TRUE;
							advCapability=(1<<DUPLEX_10M);
						}else if(DUPLEX_100M == portStatusToSet)
						{
							forceMode=TRUE;
							forceLink=TRUE;
							forceLinkSpeed=SPEED100M;
							forceDuplex=TRUE;
							advCapability=(1<<DUPLEX_100M);
						}else if(DUPLEX_1000M == portStatusToSet)
						{
							forceMode=TRUE;
							forceLink=TRUE;
							forceLinkSpeed=SPEED1000M;
							forceDuplex=TRUE;
							advCapability=(1<<DUPLEX_1000M);
						}else if(PORT_AUTO == portStatusToSet)
						{
							forceMode=FALSE;
							forceLink=TRUE;
							/*all capality*/
							advCapability=(1<<PORT_AUTO);
							
						}else
						{
							forceMode=FALSE;
							forceLink=TRUE;
						}
						rtl865xC_setAsicEthernetForceModeRegs(i, forceMode, forceLink, forceLinkSpeed, forceDuplex);
						
						/*Set PHY Register*/
						rtl8651_setAsicEthernetPHYSpeed(i,forceLinkSpeed);
						rtl8651_setAsicEthernetPHYDuplex(i,forceDuplex);
						rtl8651_setAsicEthernetPHYAutoNeg(i,TRUE);
						rtl8651_setAsicEthernetPHYAdvCapality(i,advCapability);
						rtl8651_restartAsicEthernetPHYNway(i);
						break;
					}
				}
				
				break;
			}
		case RTL8651_IOCTL_GETLANPORTLINKSTATUS:
			{
				int i;
                            int lanPortMask;
                            int32 totalVlans;
				int32 *lanportnum;
				int32 lanPortTypeMask;
				uint32 regVal;
				uint32 portLinkSpeed;
 
                            pRet = (int32 *)args[3];
                            *pRet = FAILED;
                            rc = SUCCESS;

				pU32 = (uint32*)args[1];
				lanportnum = (uint32*)pU32[0];
 
                                lanPortMask = 0;
                                totalVlans=((sizeof(vlanconfig))/(sizeof(struct rtl865x_vlanConfig)))-1;
                                for(i=0;i<totalVlans;i++)
                                {
                                        if(vlanconfig[i].isWan==FALSE)
                                        {
                                                lanPortMask = vlanconfig[i].memPort;
                                                if (lanPortMask==0)
                                                {
                                                        /* no wan port exist */
                                                        continue;
                                                }
 
                                                for(i=0;i<=RTL8651_PHY_NUMBER;i++)
                                                {
                                                        if( (1<<i)&lanPortMask )
                                                        {
                                                        	regVal=READ_MEM32(PSRP0+(i<<2));
                                                                if((regVal&PortStatusLinkUp)!=0)
                                                                {
										if(i==(*lanportnum))
										{
                                                                        	*pRet = SUCCESS;
																			
											if((regVal&PortStatusDuplex)!=0)
											{
												lanPortTypeMask=1;
												*pRet |= lanPortTypeMask;
											}

											portLinkSpeed=regVal&PortStatusLinkSpeed_MASK;
											if(PortStatusLinkSpeed100M==portLinkSpeed)
											{
												lanPortTypeMask=4;
												*pRet |= lanPortTypeMask;
											}
											else if(PortStatusLinkSpeed1000M==portLinkSpeed)
											{
												lanPortTypeMask=8;
												*pRet |= lanPortTypeMask;
											}
											else
											{
												lanPortTypeMask=2;
												*pRet |= lanPortTypeMask;
											}
											
                                                                        	return rc;
										}
                                                                }
                                                        }
                                                }
                                        }
                                }
 
                                break;
			}

		case RTL8651_IOCTL_GETWANPORTLINKSTATUS:
			{
				int i;
				int wanPortMask;
				int32 totalVlans;
				int32 wanPortTypeMask;
				uint32 regVal;
				uint32 portLinkSpeed;

				pRet = (int32 *)args[3];
				*pRet = FAILED;
				rc = SUCCESS;

				wanPortMask = 0;
				totalVlans=((sizeof(vlanconfig))/(sizeof(struct rtl865x_vlanConfig)))-1;
				for(i=0;i<totalVlans;i++)
				{
					if(vlanconfig[i].isWan==TRUE)
						wanPortMask = vlanconfig[i].memPort;
				}

				if (wanPortMask==0)
				{
					/* no wan port exist */					
					break;
				}

				for(i=0;i<RTL8651_AGGREGATOR_NUMBER;i++)
				{
					if( (1<<i)&wanPortMask )
					{
						/*check phy status link up or down*/
						rtl8651_getAsicEthernetPHYStatus(i,&regVal);
						if(regVal & (1<<2))
						{
							regVal=READ_MEM32(PSRP0+(i<<2));
							if((regVal&PortStatusLinkUp)!=0)
							{
								*pRet = SUCCESS;

								if((regVal&PortStatusDuplex)!=0)
								{
										wanPortTypeMask=1;
										*pRet |= wanPortTypeMask;
								}

								portLinkSpeed=regVal&PortStatusLinkSpeed_MASK;
								if(PortStatusLinkSpeed100M==portLinkSpeed)
								{
										wanPortTypeMask=4;
										*pRet |= wanPortTypeMask;
								}
								else if(PortStatusLinkSpeed1000M==portLinkSpeed)
								{
										wanPortTypeMask=8;
										*pRet |= wanPortTypeMask;
								}
								else
								{
										wanPortTypeMask=2;
										*pRet |= wanPortTypeMask;
								}
							}
						}
						break;
					}
				}
				
				break;
			}
#endif
#endif
		default:
			rc = SUCCESS;
			break;
	}
	
	return rc;
#if !defined(RTL819X_PRIV_IOCTL_ENABLE)
normal:	
#endif
	if (!netif_running(dev))
		return -EINVAL;
	switch (cmd)
        {
	    default:
		rc = -EOPNOTSUPP;
		break;
	}
	return rc;
}

static int rtl865x_set_hwaddr(struct net_device *dev, void *addr)
{
	unsigned long flags;
	int i;
	unsigned char *p;
#if defined (CONFIG_RTL_MULTI_LAN_DEV)
	struct dev_priv *cp = dev->priv;
#endif
#ifdef CONFIG_RTL_MULTI_LAN_DEV
	u32 vid;
#endif
	p = ((struct sockaddr *)addr)->sa_data;
 	local_irq_save(flags);
	for (i = 0; i<ETHER_ADDR_LEN; ++i) {
		dev->dev_addr[i] = p[i];
		dev->broadcast[i] = 0xff;
	}

#ifdef CONFIG_RTL_MULTI_LAN_DEV
	vid = cp->id;
#endif
#if defined (CONFIG_RTL_MULTI_LAN_DEV)
	/*find matched vlan config*/
	for(i=0; vlanconfig[i].vid != 0; i++)
	{
		if(	(vlanconfig[i].vid==vid ) && (vlanconfig[i].if_type==IF_ETHER) && 
			(vlanconfig[i].memPort & cp->portmask & ((1<<RTL8651_MAC_NUMBER)-1)))
		{
			break;
		}
	}

	if(vlanconfig[i].vid != 0)
	{
		memcpy(vlanconfig[i].mac.octet, dev->dev_addr, ETHER_ADDR_LEN);
	}
#else
#if defined(CONFIG_RTK_VLAN_SUPPORT)
    TOKEN_NUM(dev->name,&i);
//	i = dev->name[3] - '0';
	if ((i >=0) && (i <5))
		memcpy(vlanconfig[i].mac.octet, dev->dev_addr, ETHER_ADDR_LEN);

#else
	if (!strcmp(dev->name, RTL_PS_LAN_P0_DEV_NAME))
		i = 0;
	else
		i = 1;	
	memcpy(vlanconfig[i].mac.octet, dev->dev_addr, ETHER_ADDR_LEN);
#endif
#endif

#ifdef CONFIG_HARDWARE_NAT_DEBUG
/*2007-12-19*/
	rtlglue_printf("%s:%d:dev->name is %s,__lrconfig[%d].mac is 0x%x:%x:%x:%x:%x:%x\n",__FUNCTION__,__LINE__,dev->name, i, 
	vlanconfig[i].mac.octet[0],vlanconfig[i].mac.octet[1],vlanconfig[i].mac.octet[2],vlanconfig[i].mac.octet[3],vlanconfig[i].mac.octet[4],vlanconfig[i].mac.octet[5]);

	rtlglue_printf("%s:%d:__lrconfig[%d].vid is %d\n",__FUNCTION__,__LINE__,i,vlanconfig[i].vid);
#endif

	if(vlanconfig[i].vid != 0)
	{
		rtl865x_netif_t netif;
		memcpy(netif.macAddr.octet,vlanconfig[i].mac.octet,ETHER_ADDR_LEN);
		memcpy(netif.name,vlanconfig[i].ifname,MAX_IFNAMESIZE);
		rtl865x_setNetifMac(&netif);
	}

	local_irq_restore(flags);
	return SUCCESS;
}

#if defined(CONFIG_RTL8186_LINK_CHANGE)
static int rtl865x_set_link(struct net_device *dev, int enable)
{
	int32 i;
	struct dev_priv *cp;
	int32 portmask;
	int32 totalVlans;
	
	cp = dev->priv;
#if defined (CONFIG_RTL_MULTI_LAN_DEV)
	portmask=cp->portmask;
#else
	portmask=0;
	totalVlans=((sizeof(vlanconfig))/(sizeof(struct rtl865x_vlanConfig)))-1;

	for(i=0;i<totalVlans;i++)
	{
		if(vlanconfig[i].vid==cp->id)
		{
			portmask = vlanconfig[i].memPort;
			break;
		}
	}
#endif	
	if (portmask)
	{
		if (enable)
		{
			for(i=0;i<RTL8651_PHY_NUMBER;i++)
			{
				if (portmask & (1<<i))
				{
					rtl865xC_setAsicEthernetForceModeRegs(i, FALSE, TRUE, 1, TRUE);
					rtl8651_restartAsicEthernetPHYNway(i);
				}
			}
		}
		else
		{
			for(i=0;i<RTL8651_PHY_NUMBER;i++)
			{
				if (portmask & (1<<i))
					rtl865xC_setAsicEthernetForceModeRegs(i, TRUE, FALSE, 1, TRUE);
			}
		}
	}

	return SUCCESS;
}
#endif

static int rtl865x_set_mtu(struct net_device *dev, int new_mtu)
{
	unsigned long flags;
	int i;
#if defined (CONFIG_RTL_MULTI_LAN_DEV)
	struct dev_priv *cp = dev->priv;
#endif
#ifdef CONFIG_RTL_MULTI_LAN_DEV
	u32 vid;
#endif
 	local_irq_save(flags);

#ifdef CONFIG_RTL_MULTI_LAN_DEV
#ifndef CONFIG_RTL_MULTI_ETH_WAN
	vid = cp->id;
#else
	if (dev->priv_flags & IFF_DOMAIN_ELAN)
		vid = cp->id;
	else {
	//if (!strcmp(dev->name, "nas0"))//
	if(alias_name_is_eq(CMD_CMP,dev->name,ALIASNAME_NAS0))
		{
			printk("%s on nas0.\n", __func__);
			dev->mtu = new_mtu;
			local_irq_restore(flags);
			
			return SUCCESS;
		}
		vid = (SMUX_DEV_INFO(dev)->vid==-1)?RTL_WANVLANID:(SMUX_DEV_INFO(dev)->vid);
	}
#endif
#endif

#if defined (CONFIG_RTL_MULTI_LAN_DEV)
	/*find matched vlan config*/
	for(i=0; vlanconfig[i].vid != 0; i++)
	{
		if(	(vlanconfig[i].vid==vid) && (vlanconfig[i].if_type==IF_ETHER) && 
			(vlanconfig[i].memPort & cp->portmask & ((1<<RTL8651_MAC_NUMBER)-1)))
		{
			break;
		}
	}
	
	if(vlanconfig[i].vid==0)
	{
		local_irq_restore(flags);
		return FAILED;		
	}
#else
#if defined(CONFIG_RTK_VLAN_SUPPORT)
        TOKEN_NUM(dev->name,&i);
//	i = dev->name[3] - '0';
	if (i>5)
	{
		local_irq_restore(flags);
		return FAILED;		
	}
#else
	if (!strcmp(dev->name, RTL_PS_LAN_P0_DEV_NAME))
		i = 0;
	else
		i = 1;
#endif
#endif
	dev->mtu = new_mtu;	
	vlanconfig[i].mtu=(uint32)new_mtu;
	if(vlanconfig[i].vid!=0)
	{
		rtl865x_netif_t netif;
		netif.mtu = vlanconfig[i].mtu;
		memcpy(netif.name,vlanconfig[i].ifname,MAX_IFNAMESIZE);

		rtl865x_setNetifMtu(&netif);
	}
	#ifdef CONFIG_HARDWARE_NAT_DEBUG
	/*2007-12-19*/
		rtlglue_printf("%s:%d:new_mtu is %d\n",__FUNCTION__,__LINE__,new_mtu);
	#endif

	local_irq_restore(flags);

	return SUCCESS;
}

#if defined(CONFIG_COMPAT_NET_DEV_OPS)
#else
static const struct net_device_ops rtl819x_netdev_ops = {
	.ndo_open		= re865x_open,
	.ndo_stop		= re865x_close,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_set_mac_address 	= rtl865x_set_hwaddr,
	.ndo_set_multicast_list	= re865x_set_rx_mode,
	.ndo_get_stats		= re865x_get_stats,
	.ndo_do_ioctl		= re865x_ioctl,
	.ndo_start_xmit		= re865x_start_xmit,
	.ndo_tx_timeout		= re865x_tx_timeout,
#if defined(CP_VLAN_TAG_USED)
	.ndo_vlan_rx_register	= cp_vlan_rx_register,
#endif
	.ndo_change_mtu		= rtl865x_set_mtu,

};
#endif

#if defined (CONFIG_RTL_IGMP_SNOOPING) && defined(CONFIG_RTL_HW_L2_ONLY)

int rtl865x_addAclForIGMPSnooping(void)
{
			int i;
			rtl865x_AclRule_t rule;
#if defined (CONFIG_RTL_MULTI_LAN_DEV) || defined(CONFIG_RTK_VLAN_SUPPORT)
			struct rtl865x_vlanConfig tmpVlanConfig[NETIF_SW_NUMBER];
#endif
			struct rtl865x_vlanConfig *pVlanConfig=NULL;


			if(vlanconfig==NULL)
			{
				return FAILED;
			}

#if defined (CONFIG_RTL_MULTI_LAN_DEV) ||defined(CONFIG_RTK_VLAN_SUPPORT)
			re865x_packVlanConfig(vlanconfig, tmpVlanConfig);
			pVlanConfig=tmpVlanConfig;
#else
			pVlanConfig=vlanconfig;
#endif			

			for(i=0; pVlanConfig[i].vid != 0; i++)
			{
				if (IF_ETHER!=pVlanConfig[i].if_type)
				{
					continue;
				}

				rtl865x_regist_aclChain(pVlanConfig[i].ifname, RTL865X_ACL_IGMP_USED,1);

				bzero((void*)&rule,sizeof(rtl865x_AclRule_t));

				rule.ruleType_ = RTL865X_ACL_MAC;
				rule.actionType_ = RTL865X_ACL_TOCPU;
				rule.pktOpApp_ = RTL865X_ACL_ALL_LAYER;
				rule.un_ty.dstMac_.octet[0] = 0x01;
				rule.un_ty.dstMac_.octet[1] = 0x00;
				rule.un_ty.dstMac_.octet[2] = 0x5E;
				rule.un_ty.dstMacMask_.octet[0] = 0xff;					
				rule.un_ty.dstMacMask_.octet[1] = 0xff;
				rule.un_ty.dstMacMask_.octet[2] = 0xff;					
				rtl865x_add_acl(&rule, pVlanConfig[i].ifname, RTL865X_ACL_IGMP_USED,1,0);

				
			}
	return SUCCESS;
}


int rtl865x_removeAclForIGMPSnooping(void)
{
		int i;
#if defined (CONFIG_RTL_MULTI_LAN_DEV) ||defined(CONFIG_RTK_VLAN_SUPPORT)	
		struct rtl865x_vlanConfig tmpVlanConfig[NETIF_SW_NUMBER];
#endif
	
		struct rtl865x_vlanConfig *pVlanConfig=NULL;
		
		if(vlanconfig==NULL)
		{
			return FAILED;
		}
#if defined (CONFIG_RTL_MULTI_LAN_DEV) ||defined(CONFIG_RTK_VLAN_SUPPORT)	
		re865x_packVlanConfig(vlanconfig, tmpVlanConfig);
		pVlanConfig=tmpVlanConfig;
#else
		pVlanConfig=vlanconfig;
#endif
	
		for(i=0; pVlanConfig[i].vid != 0; i++)
		{
			if (IF_ETHER!=pVlanConfig[i].if_type)
			{
				continue;
			}
		
			rtl865x_unRegist_aclChain(pVlanConfig[i].ifname, RTL865X_ACL_IGMP_USED);
	
		}
		return SUCCESS;


}


#endif

#if defined (CONFIG_RTL_MLD_SNOOPING) && defined (CONFIG_RTL_HARDWARE_NAT)
//static int rtl865x_addAclForMldSnooping(struct rtl865x_vlanConfig* vlanConfig)
int rtl865x_addAclForMldSnooping(struct rtl865x_vlanConfig* vlanConfig)
{
	int i;
	#if defined (CONFIG_RTL_MULTI_LAN_DEV) || defined(CONFIG_RTK_VLAN_SUPPORT)
	struct rtl865x_vlanConfig tmpVlanConfig[NETIF_SW_NUMBER];
	#endif
	struct rtl865x_vlanConfig *pVlanConfig=NULL;
	rtl865x_AclRule_t	rule;
	int ret=FAILED;
	
	if(vlanConfig==NULL)
	{
		return FAILED;
	}
	
#if defined (CONFIG_RTL_MULTI_LAN_DEV) ||defined(CONFIG_RTK_VLAN_SUPPORT)
	re865x_packVlanConfig(vlanConfig, tmpVlanConfig);
	pVlanConfig=tmpVlanConfig;
#else
	pVlanConfig=vlanConfig;
#endif

	for(i=0; pVlanConfig[i].vid != 0; i++)
	{
		if (IF_ETHER!=pVlanConfig[i].if_type)
		{
			continue;
		}
		
	
		if(pVlanConfig[i].isWan==0)/*lan config*/
		{
			rtl865x_regist_aclChain(pVlanConfig[i].ifname, RTL865X_ACL_IPV6_USED,2);
			/*ping6 issue*/
			bzero((void*)&rule,sizeof(rtl865x_AclRule_t));
			rule.ruleType_ = RTL865X_ACL_MAC;
			rule.actionType_		= RTL865X_ACL_PERMIT;
			rule.pktOpApp_ 		= RTL865X_ACL_ALL_LAYER;
			rule.un_ty.dstMac_.octet[0]=0x33;
			rule.un_ty.dstMac_.octet[1]=0x33;
			rule.un_ty.dstMac_.octet[2]=0xFF;
	
			rule.un_ty.dstMacMask_.octet[0]=0xFF;
			rule.un_ty.dstMacMask_.octet[1]=0xFF;
			rule.un_ty.dstMacMask_.octet[2]=0xFF;
			
			ret= rtl865x_add_acl(&rule, pVlanConfig[i].ifname, RTL865X_ACL_IPV6_USED,1,0);
			
			/*ipv6 multicast data issue*/
			bzero((void*)&rule,sizeof(rtl865x_AclRule_t));
			rule.ruleType_ = RTL865X_ACL_MAC;
			rule.actionType_		= RTL865X_ACL_TOCPU;
			rule.pktOpApp_ 		= RTL865X_ACL_ALL_LAYER;
			rule.un_ty.dstMac_.octet[0]=0x33;
			rule.un_ty.dstMac_.octet[1]=0x33;
			rule.un_ty.dstMac_.octet[2]=0x00;
			rule.un_ty.dstMac_.octet[3]=0x00;
			rule.un_ty.dstMac_.octet[4]=0x00;
			rule.un_ty.dstMac_.octet[5]=0x00;	
			
			rule.un_ty.dstMacMask_.octet[0]=0xFF;
			rule.un_ty.dstMacMask_.octet[1]=0xFF;
			
			ret= rtl865x_add_acl(&rule, pVlanConfig[i].ifname, RTL865X_ACL_IPV6_USED,1,0);
			
		}
		else/*wan config*/
		{	
			rtl865x_regist_aclChain(pVlanConfig[i].ifname, RTL865X_ACL_IPV6_USED,2);
			/*ipv6 multicast data issue*/
			bzero((void*)&rule,sizeof(rtl865x_AclRule_t));
			rule.ruleType_ = RTL865X_ACL_MAC;
			rule.actionType_		= RTL865X_ACL_TOCPU;
			rule.pktOpApp_ 		= RTL865X_ACL_ALL_LAYER;
			rule.un_ty.dstMac_.octet[0]=0x33;
			rule.un_ty.dstMac_.octet[1]=0x33;
			rule.un_ty.dstMac_.octet[2]=0x00;
			rule.un_ty.dstMac_.octet[3]=0x00;
			rule.un_ty.dstMac_.octet[4]=0x00;
			rule.un_ty.dstMac_.octet[5]=0x00;	
			
			rule.un_ty.dstMacMask_.octet[0]=0xFF;
			rule.un_ty.dstMacMask_.octet[1]=0xFF;
			
			ret= rtl865x_add_acl(&rule, pVlanConfig[i].ifname, RTL865X_ACL_IPV6_USED,1,0);

		}		

		//rtl865x_reConfigDefaultAcl(pVlanConfig[i].ifname);

	}
	
	return SUCCESS;
}

//static int rtl865x_removeAclForMldSnooping(struct rtl865x_vlanConfig* vlanConfig)
int rtl865x_removeAclForMldSnooping(struct rtl865x_vlanConfig* vlanConfig)
{
	int i;
	#if defined (CONFIG_RTL_MULTI_LAN_DEV) ||defined(CONFIG_RTK_VLAN_SUPPORT)	
	struct rtl865x_vlanConfig tmpVlanConfig[NETIF_SW_NUMBER];
	#endif

	struct rtl865x_vlanConfig *pVlanConfig=NULL;
	
	if(vlanConfig==NULL)
	{
		return FAILED;
	}
#if defined (CONFIG_RTL_MULTI_LAN_DEV) ||defined(CONFIG_RTK_VLAN_SUPPORT)	
	re865x_packVlanConfig(vlanConfig, tmpVlanConfig);
	pVlanConfig=tmpVlanConfig;
#else
	pVlanConfig=vlanConfig;
#endif

	for(i=0; pVlanConfig[i].vid != 0; i++)
	{
		if (IF_ETHER!=pVlanConfig[i].if_type)
		{
			continue;
		}
	
		if(pVlanConfig[i].isWan==0)/*lan config*/
		{
			rtl865x_unRegist_aclChain(pVlanConfig[i].ifname, RTL865X_ACL_IPV6_USED);
		}
		else/*wan config*/
		{	
			rtl865x_unRegist_aclChain(pVlanConfig[i].ifname, RTL865X_ACL_IPV6_USED);
		}

	}
	return SUCCESS;
}
#endif


#if !defined(CONFIG_COMPAT_NET_DEV_OPS) && defined(CONFIG_RTL_CUSTOM_PASSTHRU)
static const struct net_device_ops rtl819x_pseudodev_ops = {
	.ndo_open		= re865x_pseudo_open,
	.ndo_stop		= re865x_pseudo_close,

	.ndo_get_stats	= re865x_get_stats,
	.ndo_do_ioctl		= re865x_ioctl,
	.ndo_start_xmit	= re865x_start_xmit,
};
#endif

#ifdef PORT0_USE_RGMII_TO_EXTCPU_MAC
static void set_8676_RGMII(void){
	rtl865xC_setAsicEthernetMIIMode(0, LINK_RGMII);
	//rtl865xC_setAsicEthernetMIIMode(0, LINK_MII_MAC);
		

	//set Port 0 Interface Type Configuration to GMII/MII/RGMII interface 
	WRITE_MEM32(PITCR, (READ_MEM32(PITCR)&0xfffffffc)|Port0_TypeCfg_GMII_MII_RGMII);

	rtl865xC_setAsicEthernetRGMIITiming(0, RGMII_TCOMP_2NS, RGMII_RCOMP_2NS);
	WRITE_MEM32(MACCR,(READ_MEM32(MACCR)&0xffffcfff)|0x01<<12);//select 100Mhz system clk
		
	//disable auto-polling
	WRITE_MEM32(PCRP0, (READ_MEM32(PCRP0)&~PollLinkStatus));
	WRITE_MEM32(PCRP1, (READ_MEM32(PCRP1)&~PollLinkStatus));
	WRITE_MEM32(PCRP2, (READ_MEM32(PCRP2)&~PollLinkStatus));
	WRITE_MEM32(PCRP3, (READ_MEM32(PCRP3)&~PollLinkStatus));
	WRITE_MEM32(PCRP4, (READ_MEM32(PCRP4)&~PollLinkStatus));
		
	//set port0 force mode
	//WRITE_MEM32(PCRP0, (READ_MEM32(PCRP0)&(~ForceSpeedMask)&(~ExtPHYID_MASK))|ForceSpeed10M|EnForceMode|ForceDuplex);
	//WRITE_MEM32(PCRP0, (READ_MEM32(PCRP0)&(~ForceSpeedMask)&(~ExtPHYID_MASK))|ForceSpeed100M|EnForceMode|ForceDuplex);
	WRITE_MEM32(PCRP0, (READ_MEM32(PCRP0)&(~ForceSpeedMask)&(~ExtPHYID_MASK))|ForceSpeed1000M|EnForceMode|ForceDuplex);
	mdelay(10);
		
	//port0 force link
	WRITE_MEM32(PCRP0, READ_MEM32(PCRP0)|ForceLink);

	printk("Switch port0 to RGMII mode. \n\r");
}
#endif /*PORT0_USE_RGMII_TO_EXTCPU_MAC*/

int  __init re865x_probe (void)
{
/*2007-12-19*/
	int32 i, j;
#ifdef CONFIG_RTL_MULTI_LAN_DEV
	int32 ehtwan_eth_num=0;
#endif
	int32 totalVlans=((sizeof(vlanconfig))/(sizeof(struct rtl865x_vlanConfig)))-1;
#if defined (CONFIG_RTL_IGMP_SNOOPING)
	int32 retVal;
/*
	int32 igmpInitFlag=FAILED;
	struct rtl_mCastSnoopingGlobalConfig mCastSnoopingGlobalConfig;
*/
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	rtl865x_mCastConfig_t mCastConfig;
	#endif
#endif
#if defined(CONFIG_RTK_VLAN_SUPPORT)
	struct proc_dir_entry *res_stats_root;
	struct proc_dir_entry *res_stats;
	struct proc_dir_entry *rtk_vlan_support_entry;
#endif

#if defined (CONFIG_RTL_LOCAL_PUBLIC)
	struct rtl865x_interface_info ifInfo;
#endif

#if defined(PATCH_GPIO_FOR_LED)
	int port;
#endif
	//WRITE_MEM32(PIN_MUX_SEL_2, 0x7<<21);

#ifdef CONFIG_RTL_HW_PURE_SWITCH
	int eth_base_num = 0;
#endif

	rtlglue_printf("\n\n\nProbing RTL8186 10/100 NIC-kenel stack size order[%d]...\n", THREAD_SIZE_ORDER);
    	REG32(CPUIIMR) = 0x00;
    	REG32(CPUICR) &= ~(TXCMD | RXCMD);
	rxMbufRing=NULL;
	
	/*Initial ASIC table*/
 	#ifdef CONFIG_RTL8198_REVISION_B
 	if (REG32(BSP_REVR) >= BSP_RTL8198_REVISION_B)
	{
	  	REG32(SYS_CLK_MAG)&=(~(SYS_SW_RESET));
		mdelay(300);
		REG32(SYS_CLK_MAG)|=(SYS_SW_RESET);
	}
	else
 	#endif
	FullAndSemiReset();
	
	{
		rtl8651_tblAsic_InitPara_t para;

		memset(&para, 0, sizeof(rtl8651_tblAsic_InitPara_t));

		/*
			For DEMO board layout, RTL865x platform define corresponding PHY setting and PHYID.
		*/
		
		rtl865x_wanPortMask = RTL865X_PORTMASK_UNASIGNED;
		
		INIT_CHECK(rtl865x_initAsicL2(&para));
		
#ifdef CONFIG_RTL_LAYERED_ASIC_DRIVER_L3
		INIT_CHECK(rtl865x_initAsicL3());
#endif
#if defined(CONFIG_RTL_LAYERED_ASIC_DRIVER_L4)
		INIT_CHECK(rtl865x_initAsicL4());
#endif

		/*
			Re-define the wan port according the wan port detection result.
			NOTE:
				There are a very strong assumption that if port5 was giga port, 
				then wan port was port 5.
		*/
		if (RTL865X_PORTMASK_UNASIGNED==rtl865x_wanPortMask)
		{
			/* keep the original mask */
			assert(RTL865X_PORTMASK_UNASIGNED==rtl865x_lanPortMask);
			rtl865x_wanPortMask = RTL_WANPORT_MASK;
			rtl865x_lanPortMask = RTL_LANPORT_MASK;
		}
		else
		{
			/* redefine wan port mask */
			assert(RTL865X_PORTMASK_UNASIGNED!=rtl865x_lanPortMask);
			for(i=0;i<totalVlans;i++)
			{
				if (TRUE==vlanconfig[i].isWan)
				{
					vlanconfig[i].memPort = vlanconfig[i].untagSet = rtl865x_wanPortMask;
				}
				else
				{
					vlanconfig[i].memPort = vlanconfig[i].untagSet = rtl865x_lanPortMask;
				}
			}
		}
#if 1		/*	10/100 & giga use the same pre-allocated skb number */
		/*
			Re-define the pre-allocated skb number according the wan 
			port detection result.
			NOTE:
				There are a very strong assumption that if port1~port4 were
				all giga port, then the sdram was 32M.
		*/
		{
			if (RTL865X_PREALLOC_SKB_UNASIGNED==rtl865x_maxPreAllocRxSkb)
			{
				assert(rtl865x_rxSkbPktHdrDescNum==
					rtl865x_txSkbPktHdrDescNum==
					RTL865X_PREALLOC_SKB_UNASIGNED);

				rtl865x_maxPreAllocRxSkb = MAX_PRE_ALLOC_RX_SKB;
				rtl865x_rxSkbPktHdrDescNum = NUM_RX_PKTHDR_DESC;
				rtl865x_txSkbPktHdrDescNum = NUM_TX_PKTHDR_DESC;
			}
			else
			{
				assert(rtl865x_rxSkbPktHdrDescNum!=RTL865X_PREALLOC_SKB_UNASIGNED);
				assert(rtl865x_txSkbPktHdrDescNum!=RTL865X_PREALLOC_SKB_UNASIGNED);
				/* Assigned value in function of rtl8651_initAsic() */
				rxRingSize[0] = rtl865x_rxSkbPktHdrDescNum;
				txRingSize[0] = rtl865x_txSkbPktHdrDescNum;
			}

			for(i=1;i<RTL865X_SWNIC_RXRING_HW_PKTDESC;i++)
			{
				rtl865x_maxPreAllocRxSkb += rxRingSize[i];
			}
		}
#else
		{
			rtl865x_maxPreAllocRxSkb = MAX_PRE_ALLOC_RX_SKB;
			rtl865x_rxSkbPktHdrDescNum = NUM_RX_PKTHDR_DESC;
			rtl865x_txSkbPktHdrDescNum = NUM_TX_PKTHDR_DESC;
		}
#endif
	}


#ifdef BR_SHORTCUT
	cached_dev=NULL;
#endif
	/*init PHY LED style*/
#if defined(CONFIG_RTL865X_BICOLOR_LED)
	#ifdef BICOLOR_LED_VENDOR_BXXX
	REG32(LEDCR) |= (1 << 19); // 5 ledmode set to 1 for bi-color LED
	REG32(PABCNR) &= ~0x001f0000; /* set port port b-4/3/2/1/0 to gpio */
	REG32(PABDIR) |=  0x001f0000; /* set port port b-4/3/2/1/0 gpio direction-output */
	#else
	//8650B demo board default: Bi-color 5 LED
	WRITE_MEM32(LEDCR, READ_MEM32(LEDCR) | 0x01180000 ); // bi-color LED
	#endif
	/* config LED mode */
	WRITE_MEM32(SWTAA, PORT5_PHY_CONTROL);
	WRITE_MEM32(TCR0, 0x000002C2); //8651 demo board default: 15 LED boards
	WRITE_MEM32(SWTACR, CMD_FORCE | ACTION_START); // force add
#else /* CONFIG_RTL865X_BICOLOR_LED */

	/* config LED mode */
#ifndef PORT0_USE_RGMII_TO_EXTCPU_MAC
	WRITE_MEM32(LEDCR, 0x200000 ); // 15 LED
#endif /*PORT0_USE_RGMII_TO_EXTCPU_MAC*/
	WRITE_MEM32(SWTAA, PORT5_PHY_CONTROL);
	WRITE_MEM32(TCR0, 0x000002C7); //8651 demo board default: 15 LED boards
	WRITE_MEM32(SWTACR, CMD_FORCE | ACTION_START); // force add
#endif /* CONFIG_RTL865X_BICOLOR_LED */

	INIT_CHECK(rtl865x_init());
	
#if defined (CONFIG_RTL_MULTI_LAN_DEV) || defined(CONFIG_RTK_VLAN_SUPPORT)
	re865x_packVlanConfig(vlanconfig, packedVlanConfig);	
	INIT_CHECK(rtl865x_config(packedVlanConfig));
#else
	INIT_CHECK(rtl865x_config(vlanconfig));
#endif

	/* create all default VLANs */
//	rtlglue_printf("	creating eth0~eth%d...\n",totalVlans-1 );
#if 0
#if defined (CONFIG_RTL_IGMP_SNOOPING)
	memset(&mCastSnoopingGlobalConfig, 0, sizeof(struct rtl_mCastSnoopingGlobalConfig));
	mCastSnoopingGlobalConfig.maxGroupNum=256;
	mCastSnoopingGlobalConfig.maxSourceNum=300;
	mCastSnoopingGlobalConfig.hashTableSize=64;
	
	mCastSnoopingGlobalConfig.groupMemberAgingTime=260;
	mCastSnoopingGlobalConfig.lastMemberAgingTime=2;
	mCastSnoopingGlobalConfig.querierPresentInterval=260;  
	
	mCastSnoopingGlobalConfig.dvmrpRouterAgingTime=120;
	mCastSnoopingGlobalConfig.mospfRouterAgingTime=120;
	mCastSnoopingGlobalConfig.pimRouterAgingTime=120;
	
	igmpInitFlag=rtl_initMulticastSnooping(mCastSnoopingGlobalConfig);
#endif	
#endif	

#ifndef CONFIG_RTL_MULTI_ETH_WAN
	for(i=0;i<totalVlans;i++)
#else
	for (i=0; (i<totalVlans)&&(i<5); i++)
#endif
	{
		struct net_device *dev;
		struct dev_priv	  *dp;
		int rc;

		if ((IF_ETHER!=vlanconfig[i].if_type) 
#ifdef CONFIG_RTL_MULTI_ETH_WAN
			&& !vlanconfig[i].is_slave
#endif
			)
		{
			continue;
		}	
		dev = alloc_etherdev(sizeof(struct dev_priv));
		if (!dev) {
			printk("failed to allocate dev %d", i);
			return -1;
		}
		SET_MODULE_OWNER(dev);
		dp = dev->priv;
		memset(dp,0,sizeof(*dp));
		dp->dev = dev;
		dp->id = vlanconfig[i].vid;
		dp->portmask =  vlanconfig[i].memPort;
		dp->portnum  = 0;
		#if defined(CONFIG_RTK_VLAN_SUPPORT)
		#ifndef CONFIG_RTL_MULTI_ETH_WAN
		dp->vlan_setting.is_lan = (dp->id!=RTL_WANVLANID);
		#else
		dp->vlan_setting.is_lan = (dp->id==RTL_LANVLANID);
		#endif
		#endif
		for(j=0;j<RTL8651_AGGREGATOR_NUMBER;j++){
			if(dp->portmask & (1<<j))
				dp->portnum++;
		}
		
		memcpy((char*)dev->dev_addr,(char*)(&(vlanconfig[i].mac)),ETHER_ADDR_LEN);
#if defined(CONFIG_COMPAT_NET_DEV_OPS)
		dev->priv_flags = IFF_DOMAIN_ELAN;
		dev->open = re865x_open;
		dev->stop = re865x_close;
		dev->set_multicast_list = re865x_set_rx_mode;
		dev->hard_start_xmit = re865x_start_xmit;
		dev->get_stats = re865x_get_stats;
		dev->do_ioctl = re865x_ioctl;
		dev->tx_timeout = re865x_tx_timeout;
		dev->set_mac_address = rtl865x_set_hwaddr;
		dev->change_mtu = rtl865x_set_mtu;
#if defined(CONFIG_RTL8186_LINK_CHANGE)
		dev->change_link = rtl865x_set_link;
#endif
#ifdef CP_VLAN_TAG_USED
		dev->features |= NETIF_F_HW_VLAN_TX | NETIF_F_HW_VLAN_RX;
		dev->vlan_rx_register = cp_vlan_rx_register;
		dev->vlan_rx_kill_vid = cp_vlan_rx_kill_vid;
#endif

#else
		dev->netdev_ops = &rtl819x_netdev_ops;
#endif
		dev->watchdog_timeo = TX_TIMEOUT;
#if 0 
		dev->features |= NETIF_F_SG | NETIF_F_IP_CSUM;
#endif
#ifdef CP_VLAN_TAG_USED
		dev->features |= NETIF_F_HW_VLAN_TX | NETIF_F_HW_VLAN_RX;		
#endif
		dev->irq = BSP_SW_IRQ;

#ifdef CONFIG_RTL_HW_PURE_SWITCH
#define ALIASNAME_ETH "eth"
		sprintf(dev->name, "%s%d", ALIASNAME_ETH, eth_base_num);
		dev->priv_flags = IFF_DOMAIN_ELAN;
		eth_base_num++;
#endif

		//Kevin
		//#ifdef CONFIG_ETHWAN
		if(vlanconfig[i].isWan) {
			sprintf(dev->name, ALIASNAME_NAS0);
			/*QL for fastbridge*/
			dev->priv_flags = IFF_DOMAIN_WAN;
		}
		else
		{
#ifdef CONFIG_RTL_MULTI_LAN_DEV
			sprintf(dev->name, "%s%d",ALIASNAME_ELAN_PREFIX,ehtwan_eth_num+ORIGINATE_NUM);
			//atoi_num(ORIGINATE_NUM));
			//sprintf(dev->name, "eth0.%d",ehtwan_eth_num+2);//
			/*QL for fastbridge*/
			dev->priv_flags = IFF_DOMAIN_ELAN;
			ehtwan_eth_num++;
#endif
		}		
		//#endif
#ifdef CONFIG_RTL_MULTI_LAN_DEV
		rtl_set_ethtool_ops(dev);
#endif	
		rc = register_netdev(dev);
		if(!rc){
			_rtl86xx_dev.dev[i]=dev;
			rtl_add_ps_drv_netif_mapping(dev,vlanconfig[i].ifname);
			/*2007-12-19*/
			rtlglue_printf("%s added. vid=%d Member port 0x%x...\n", dev->name,vlanconfig[i].vid ,vlanconfig[i].memPort );
		}else
			rtlglue_printf("Failed to allocate eth%d\n", i);

#if defined(CONFIG_RTK_VLAN_SUPPORT)
		res_stats_root = proc_mkdir(dev->name, NULL);
		if (res_stats_root == NULL) 
		{
			printk("proc_mkdir failed!\n");		
		}
	
		if ((res_stats = create_proc_read_entry("mib_vlan", 0644, res_stats_root,
			read_proc_vlan, (void *)dev)) == NULL) 
		{
			printk("create_proc_read_entry failed!\n");		
		}
		res_stats->write_proc = write_proc_vlan;	

#endif

	}
	
#if defined (CONFIG_RTL_IGMP_SNOOPING)
	retVal=rtl_registerIgmpSnoopingModule(&nicIgmpModuleIndex);
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if(retVal==SUCCESS)
	{
		rtl_multicastDeviceInfo_t devInfo;
		memset(&devInfo, 0 , sizeof(rtl_multicastDeviceInfo_t));
		strcpy(devInfo.devName, "eth*");
		for(i=0;i<totalVlans;i++)
		{
			if( vlanconfig[i].if_type==IF_ETHER)
			{
				devInfo.portMask|=vlanconfig[i].memPort;
			}
		}
		devInfo.swPortMask=devInfo.portMask & (~ ((1<<RTL8651_MAC_NUMBER)-1));
		rtl_setIgmpSnoopingModuleDevInfo(nicIgmpModuleIndex, &devInfo);
	}
	#endif
	curLinkPortMask=rtl865x_getPhysicalPortLinkStatus();
#if defined(CONFIG_RTL_MULTI_LAN_DEV) && defined(CONFIG_RTL_8676HWNAT)
	if(RTL8676_TBLASIC_EXTPHYPROPERTY_PORT0_RTL8367B != global_probe_extPhy)
	{
		int port;
		struct net_device *dev;
		for(port = 0; port < RTL8651_PORT_NUMBER; ++port)
		{
			if((dev = ethtool_get_netdev_by_rtl_sw_port(&_rtl86xx_dev, port)) != NULL)
			{
				if((1 << port) & curLinkPortMask)
					netif_carrier_on(dev);
				else
					netif_carrier_off(dev);
			}
		}
	}
#endif

	
	
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	memset(&mCastConfig, 0, sizeof(rtl865x_mCastConfig_t));
	for(i=0;i<totalVlans;i++)
	{
		if (TRUE==vlanconfig[i].isWan)
		{
			mCastConfig.externalPortMask |=vlanconfig[i].memPort;
		}
	}
	rtl865x_initMulticast(&mCastConfig);
	#else
	WRITE_MEM32(FFCR, READ_MEM32(FFCR)|IPMltCstCtrl_TrapToCpu);
	#endif
#endif

#ifdef CONFIG_RTL_STP
	#ifdef CONFIG_RTK_MESH
	printk("Configuration LINUX to process port 0 ~ port %d for Spanning tree process\n", MAX_RE865X_ETH_STP_PORT-1);
	#else
    	printk("Configuration LINUX to process port 0 ~ port %d for Spanning tree process\n", MAX_RE865X_STP_PORT-ORIGINATE_NUM);
//	printk("Configuration LINUX to process port 0 ~ port %d for Spanning tree process\n", MAX_RE865X_STP_PORT-2);
	#endif
	rtl865x_setSpanningEnable(TRUE);
	#ifdef CONFIG_RTK_MESH
	for ( i = 0 ; i < MAX_RE865X_ETH_STP_PORT; i ++ )
	#else
	for ( i = 0 ; i < MAX_RE865X_STP_PORT-1 ; i ++ )
	#endif
	{
		struct net_device *dev;
		struct dev_priv *dp;
		int rc;
		struct re865x_priv *rp;
		
		rp = &_rtl86xx_dev;
		dev = alloc_etherdev(sizeof(struct dev_priv));
		if (!dev){
			rtlglue_printf("failed to allocate dev %d", i);
			return -1;
		}
		strcpy(dev->name, "port%d");	
		memcpy((char*)dev->dev_addr,(char*)(&(vlanconfig[0].mac)),ETHER_ADDR_LEN);
		dp = dev->priv;
		memset(dp,0,sizeof(*dp));
		dp->dev = dev;
		dev->open = re865x_pseudo_open;
		dev->stop = re865x_pseudo_close;
		dev->set_multicast_list = NULL;
		dev->hard_start_xmit = re865x_start_xmit;
		dev->get_stats = re865x_get_stats;
		dev->do_ioctl = re865x_ioctl;
		dev->tx_timeout = NULL;
		dev->watchdog_timeo = TX_TIMEOUT;
		dev->irq = 0;				/* virtual interfaces has no IRQ allocated */
		rc = register_netdev(dev);
		if (rc == 0)
		{
			_rtl86xx_dev.stp_port[i] = dev;
			printk("=> [stp pseudo port%d] done\n", i);
		} else
		{
			printk("=> Failed to register [stp pseudo port%d]", i);
			return -1;
		}
	}

	re865x_stp_mapping_init();

#endif
#if defined(CONFIG_RTL_HW_STP)
	//Initial: disable Realtek Hardware STP
	rtl865x_setSpanningEnable(FALSE);
#endif

	//((struct dev_priv*)((_rtl86xx_dev.dev[0])->priv))->dev_next = _rtl86xx_dev.dev[1];
	//((struct dev_priv*)((_rtl86xx_dev.dev[1])->priv))->dev_prev = _rtl86xx_dev.dev[0];

#if defined(CONFIG_RTL_ETH_PRIV_SKB)
	init_priv_eth_skb_buf();
#endif

#if (defined(CONFIG_RTL_CUSTOM_PASSTHRU) && !defined(CONFIG_RTL8196_RTL8366)) 
	//cary
	rtl8651_customPassthru_init();
	rtl8651_initStormCtrl();
#endif

#if (defined(CONFIG_RTL_8198))
	// initial proc for phyRegTest
	phyRegTest_init();
#endif

#ifdef CONFIG_RTL_LAYERED_ASIC_DRIVER_L3
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	rtl8651_setAsicMulticastEnable(TRUE);
#else
	rtl8651_setAsicMulticastEnable(FALSE);
#endif
#endif
#if defined(CONFIG_RTK_VLAN_SUPPORT)

	rtk_vlan_support_enable= 0;
	rtk_vlan_support_entry=create_proc_entry("rtk_vlan_support",0,NULL);
	if (rtk_vlan_support_entry) 
	{
	    rtk_vlan_support_entry->read_proc=rtk_vlan_support_read;
	    rtk_vlan_support_entry->write_proc=rtk_vlan_support_write;
	}
#endif

#if defined (CONFIG_RTL_LOCAL_PUBLIC)
	rtl865x_initLocalPublic(NULL);

	memset(&ifInfo, 0 , sizeof(struct rtl865x_interface_info));
#if defined(CONFIG_RTL_PUBLIC_SSID)
	strcpy(ifInfo.ifname,RTL_GW_WAN_DEVICE_NAME);
#else
	strcpy(ifInfo.ifname, RTL_DRV_WAN0_NETIF_NAME);
#endif
	ifInfo.isWan=1;
	for(i=0;i<totalVlans;i++)
	{
		if ((TRUE==vlanconfig[i].isWan) && (vlanconfig[i].if_type==IF_ETHER))
		{
			ifInfo.memPort |= vlanconfig[i].memPort;
			ifInfo.fid=vlanconfig[i].fid;
		}
	}
	rtl865x_setLpIfInfo(&ifInfo);
	
	memset(&ifInfo, 0 , sizeof(struct rtl865x_interface_info));
	strcpy(ifInfo.ifname,RTL_DRV_LAN_NETIF_NAME);
	ifInfo.isWan=0;
	for(i=0;i<totalVlans;i++)
	{
		if ((FALSE==vlanconfig[i].isWan) && (vlanconfig[i].if_type==IF_ETHER))
		{
			ifInfo.memPort|=vlanconfig[i].memPort;
			ifInfo.fid=vlanconfig[i].fid;
		}
	}
	rtl865x_setLpIfInfo(&ifInfo);
#endif
	rtl_rxTxDoneCnt=0;
	atomic_set(&rtl_devOpened, 0);

#if defined(PATCH_GPIO_FOR_LED)
	for (port=0; port<RTL8651_PHY_NUMBER; port++)
		init_led_ctrl(port);
#endif

#if defined(CONFIG_RTL_LINKSTATE)
	initPortStateCtrl();
#endif

#if 0// defined (CONFIG_RTL_MLD_SNOOPING)
	rtl8651_initMldSnooping();
#endif
#if defined (CONFIG_RTL_PHY_POWER_CTRL)
	rtl865x_initPhyPowerCtrl();
#endif

#if defined(RTL_CPU_QOS_ENABLED)
	highestPriority = 0;
	cpuQosHoldLow = 0;
	totalLowQueueCnt = 0;
	memset(pktQueueByPri, 0, sizeof(rtl_queue_entry)*(RTL865X_SWNIC_RXRING_MAX_PKTDESC));

	init_timer(&cpuQosTimer);
	cpuQosTimer.function = rtl_cpuQosTimer;
#endif

#ifdef CONFIG_RTL_NLMSG_PROTOCOL
	rtl_gbl_rtlmsg_sock_init();
#endif

#ifdef PORT0_USE_RGMII_TO_EXTCPU_MAC
	set_8676_RGMII();
	
	REG32(BSP_MISC_IO_DRIVING) |= BSP_MISC_IO_GPIOA_DrvSel;
	REG32(BSP_RGMII_PAD_CTRL) |= (BSP_RGMII_C_CELLBIAS_DP|BSP_RGMII_C_CELLBIAS_DN | \
			BSP_RGMII_D_CELLBIAS_DP | BSP_RGMII_D_CELLBIAS_DN);
#endif /*PORT0_USE_RGMII_TO_EXTCPU_MAC*/
		

	return 0;
}

#if defined(CONFIG_RTL_ETH_PRIV_SKB)

//---------------------------------------------------------------------------
static void init_priv_eth_skb_buf(void)
{
	int i;

	DEBUG_ERR("Init priv skb.\n");
	memset(eth_skb_buf, '\0', sizeof(struct priv_skb_buf2)*(MAX_ETH_SKB_NUM));
	INIT_LIST_HEAD(&eth_skbbuf_list);
	eth_skb_free_num=MAX_ETH_SKB_NUM;

	for (i=0; i<MAX_ETH_SKB_NUM; i++)  {
		memcpy(eth_skb_buf[i].magic, ETH_MAGIC_CODE, ETH_MAGIC_LEN);	
		eth_skb_buf[i].buf_pointer = (void*)(&eth_skb_buf[i]);
		INIT_LIST_HEAD(&eth_skb_buf[i].list);
		list_add_tail(&eth_skb_buf[i].list, &eth_skbbuf_list);	
	}
}

static __inline__ unsigned char *get_buf_from_poll(struct list_head *phead, unsigned int *count)
{
	unsigned long flags;
	unsigned char *buf;
	struct list_head *plist;

	local_irq_save(flags);

	if (list_empty(phead)) {
		local_irq_restore(flags);
		DEBUG_ERR("eth_drv: phead=%X buf is empty now!\n", (unsigned int)phead);
		DEBUG_ERR("free count %d\n", *count);
		return NULL;
	}

	if (*count == 1) {
		local_irq_restore(flags);
		DEBUG_ERR("eth_drv: phead=%X under-run!\n", (unsigned int)phead);
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

__IRAM_GEN void free_rtl865x_eth_priv_buf(unsigned char *head)
{
	#ifdef DELAY_REFILL_ETH_RX_BUF
	if (FAILED==return_to_rx_pkthdr_ring(head)) 
	#endif
	{release_buf_to_poll(head, &eth_skbbuf_list, (unsigned int *)&eth_skb_free_num);}
}

static struct sk_buff *dev_alloc_skb_priv_eth(unsigned int size)
{
	struct sk_buff *skb;
	unsigned char *data; 
	/* first argument is not used */
	data = get_buf_from_poll(&eth_skbbuf_list, (unsigned int *)&eth_skb_free_num);
	if (data == NULL) {
		DEBUG_ERR("eth_drv: priv skb buffer empty!\n");
		return NULL;
	}
	skb = dev_alloc_8190_skb(data, size);
	if (skb == NULL) {
		//free_rtl865x_eth_priv_buf(data);
		release_buf_to_poll(data, &eth_skbbuf_list, (unsigned int *)&eth_skb_free_num);	
		DEBUG_ERR("alloc linux skb buff failed!\n");
		return NULL;
	}

	return skb;
}

int is_rtl865x_eth_priv_buf(unsigned char *head)
{
	unsigned long offset = (unsigned long)(&((struct priv_skb_buf2 *)0)->buf);
	struct priv_skb_buf2 *priv_buf = (struct priv_skb_buf2 *)(((unsigned long)head) - offset);

	if ((!memcmp(priv_buf->magic, ETH_MAGIC_CODE, ETH_MAGIC_LEN)) &&
		(priv_buf->buf_pointer==(void*)(priv_buf))) {
		return 1;	
	}
	else {
		return 0;
	}
}

#if defined(CONFIG_RTL_ETH_PRIV_SKB) && (defined(CONFIG_NET_WIRELESS_AGN) || defined(CONFIG_NET_WIRELESS_AG) || defined(CONFIG_WIRELESS))
struct sk_buff *priv_skb_copy(struct sk_buff *skb)
{
	struct sk_buff *n;
	unsigned long flags;

	if (rx_skb_queue.qlen == 0) {
		n = dev_alloc_skb_priv_eth(CROSS_LAN_MBUF_LEN);
	}
	else {
#ifdef RTK_QUE
		local_irq_save(flags);
		n = rtk_dequeue(&rx_skb_queue);
		local_irq_restore(flags);
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
EXPORT_SYMBOL(priv_skb_copy);
#endif // defined(CONFIG_NET_WIRELESS_AGN) || defined(CONFIG_NET_WIRELESS_AG)
#endif // CONFIG_RTL_ETH_PRIV_SKB

static void __exit re865x_exit (void)
{

#ifdef RTL865X_DRIVER_DEBUG_FLAG
	rtl865x_proc_debug_cleanup();
#endif

#if defined(CONFIG_PROC_FS) && defined(CONFIG_NET_SCHED) && defined(CONFIG_RTL_LAYERED_DRIVER)
#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
	rtl865x_exitOutputQueue();
#endif
#endif
	
#if defined (CONFIG_RTL_IGMP_SNOOPING)
	rtl_exitMulticastSnooping();
#endif

#if defined(CONFIG_RTL_LINKSTATE)
	exitPortStateCtrl();
#endif
	return;
}

module_init(re865x_probe);
module_exit(re865x_exit);

/*
@func enum RTL_RESULT | rtl865x_init | Initialize light rome driver and RTL865x ASIC.
@rvalue RTL_SUCCESS | Initial success. 
@comm
	Its important to call this API before using the driver. Note taht you can not call this API twice !
*/
int32 rtl865x_init(void)
{
	int32 retval = 0;


	__865X_Config = 0;	

#ifdef CONFIG_RTL8196_RTL8366
	/*	configure 8366 */
	{
		int ret;
		int i;
		rtl8366rb_phyAbility_t phy;

		REG32(PEFGHCNR_REG) = REG32(PEFGHCNR_REG)& (~(1<<11) ); //set byte F GPIO3 = gpio
		REG32(PEFGHDIR_REG) = REG32(PEFGHDIR_REG) | (1<<11);  //0 input, 1 output, set F bit 3 output
		REG32(PEFGHDAT_REG) = REG32(PEFGHDAT_REG) |( (1<<11) ); //F3 GPIO
		mdelay(150);

		ret = smi_init(GPIO_PORT_F, 2, 1);
		ret = rtl8366rb_initChip();
		ret = rtl8366rb_initVlan();
		ret = smi_write(0x0f09, 0x0020);
		ret = smi_write(0x0012, 0xe0ff);

		memset(&phy, 0, sizeof(rtl8366rb_phyAbility_t));
		phy.Full_1000 = 1;
		phy.Full_100 = 1;
		phy.Full_10 = 1;
		phy.Half_100 = 1;
		phy.Half_10 = 1;
		phy.FC = 1;
		phy.AsyFC = 1;
		phy.AutoNegotiation = 1;
		for(i=0;i<5;i++)
		{
			ret = rtl8366rb_setEthernetPHY(i,&phy);
		}
	}

 	 REG32(0xb8010000)=REG32(0xb8010000)&(0x20000000);
        REG32(0xbb80414c)=0x00037d16;
        REG32(0xbb804100)=1;
        REG32(0xbb804104)=0x00E80367;
#endif

/*common*/
	retval = rtl865x_initNetifTable();
	retval = rtl865x_initVlanTable();
#ifdef CONFIG_RTL_LAYERED_DRIVER_ACL
	retval = rtl865x_init_acl();
#endif
	/*l2*/
 #ifdef CONFIG_RTL_LAYERED_DRIVER_L2
	retval = rtl865x_initEventMgr(NULL);	
	retval = rtl865x_layer2_init();
 #endif
 
 
/*layer3*/
#ifdef CONFIG_RTL_LAYERED_DRIVER_L3
	retval = rtl865x_initIpTable();
	retval = rtl865x_initPppTable();
	retval = rtl865x_initRouteTable();
	retval = rtl865x_initNxtHopTable();
	retval = rtl865x_arp_init();
#endif

/*layer4*/
#if defined(CONFIG_RTL_LAYERED_DRIVER_L4)
	rtl865x_nat_init();
#endif

 	/*queue id & rx ring descriptor mapping*/
 	/*queue id & rx ring descriptor mapping*/
 	REG32(CPUQDM0)=QUEUEID1_RXRING_MAPPING|(QUEUEID0_RXRING_MAPPING<<16);
	REG32(CPUQDM2)=QUEUEID3_RXRING_MAPPING|(QUEUEID2_RXRING_MAPPING<<16);
	REG32(CPUQDM4)=QUEUEID5_RXRING_MAPPING|(QUEUEID4_RXRING_MAPPING<<16);

	rtl8651_setAsicOutputQueueNumber(CPU, 4);

#ifdef RTL865X_DRIVER_DEBUG_FLAG
	rtl865x_proc_debug_init();
#endif

#if defined(PATCH_GPIO_FOR_LED)
	rtl8651_resetAllAsicMIBCounter();
#endif

	rtl_ps_drv_netif_mapping_init();

	return SUCCESS;
}



/*
@func enum RTL_RESULT | rtl865x_config | Configure light rome driver. Create VLAN and Network interface.
@parm struct rtl865x_vlanConfig * | vlanconfig | 
@rvlaue RTL_SUCCESS | Sucessful configuration.
@rvalue RTL_INVVID | Invalid VID.
@comm
	struct rtl865x_vlanConfig is defined as follows:
	
			ifname:		Layer 3 Network Interface name, eg: eth0, eth1, ppp0...etc,. If it is specified, both layer 2 vlan and layer 3
						netwrok interface are created and bound together. It also can be a NULL value. In this case, only a layer 2 VLAN 
						is created.
			isWan:		1 for WAN interface and 0 for LAN interface in a layer 4 mode.
			if_type:		IF_ETHER sets a network interface to be ETHER type. Instead, IF_PPPOE sets a netwrok to be PPPoE type.
						This field is meaningful only when the ifname is specified.
			vid:			VLAN ID to create a vlan.
			memPort:		VLAN member port.
			untagSet:	VLAN untag Set.
			mtu:			MTU.
			mac:		MAC address of the VLAN or network interface.	
	eg1:

	struct rtl865x_vlanConfig vlanconfig[] = {
		{ 	"eth0",	 1,   IF_ETHER, 	8, 	   1, 	0x01, 		0x01,		1500, 	{ { 0x00, 0x00, 0xda, 0xcc, 0xcc, 0x08 } }	},
		{	"eth1",	 0,   IF_ETHER,	9,	   1,		0x1e,		0x1e,		1500,	{ { 0x00, 0x00, 0xda, 0xcc, 0xcc, 0x09 } }	},

		LRCONFIG_END,
	}
*/

/*    input   :   lan netif name in protocal stack 
       output :   the default vid (pvid) of the port mapping to this netif ,   if  -1 : errors  */
int32 rtl865x_getLANpvid(char* lan_netifname)
{
	struct net_device* lan_dev = re865x_get_netdev_by_name(lan_netifname);

	if(lan_dev==NULL)
		return -1;
	else if(!(lan_dev->priv_flags & IFF_DOMAIN_ELAN))
		return -1;
	else
	{
		int i;	
		int port=-1;
		for(i=0;i<RTL8651_AGGREGATOR_NUMBER;i++)
		{
			if((1<<i) & (((struct dev_priv *)lan_dev->priv)->portmask) )
			{
				if(port!=-1)				
					return -1;  /* lan device mapping to more than one nic port ?? */
				port = i;
			}
		}
		return pvid_per_port[port];
	}
	return -1;
}
int32 rtl865x_config(struct rtl865x_vlanConfig vlanconfig[])
{
	uint16 pvid;
	int32 i, j=0;
	int32 retval = 0;
	//uint32 valid_port_mask = 0;

	if (!vlanconfig[0].vid)
		return RTL_EINVALIDVLANID;

	INIT_CHECK(rtl8651_setAsicOperationLayer(4));
	WRITE_MEM32(CSCR,READ_MEM32(CSCR)&~ALLOW_L2_CHKSUM_ERR);
	WRITE_MEM32(CSCR,READ_MEM32(CSCR)&~ALLOW_L3_CHKSUM_ERR);
	WRITE_MEM32(CSCR,READ_MEM32(CSCR)&~ALLOW_L4_CHKSUM_ERR);
	
	for(i=0; vlanconfig[i].vid != 0; i++)
	{
		rtl865x_netif_t netif;

		if(vlanconfig[i].memPort == 0)
			continue;

		#if 0
		if(vlanconfig[i].isWan == 1)
		{
			/*wan*/
			valid_port_mask = RTL_WANPORT_MASK;
		}
		else
			valid_port_mask = RTL_LANPORT_MASK |0x100; //port8 
		#endif

//#ifdef CONFIG_RTL_MULTI_ETH_WAN
//		if (vlanconfig[i].isWan == 0)
//			vlanconfig[i].memPort |= RTL_WANPORT_MASK;
//#endif

		/*add vlan*/
#ifdef CONFIG_RTL_MULTI_ETH_WAN
		if (vlanconfig[i].if_type == IF_ETHER) {
#endif
			retval = rtl865x_addVlan(vlanconfig[i].vid);

			if(retval == SUCCESS)
			{
				rtl865x_addVlanPortMember(vlanconfig[i].vid,vlanconfig[i].memPort /*& valid_port_mask*/, vlanconfig[i].untagSet);	
				rtl865x_setVlanFilterDatabase(vlanconfig[i].vid,vlanconfig[i].fid);
			}
#ifdef CONFIG_RTL_MULTI_ETH_WAN
		}
#endif
		/*add network interface*/
		memset(&netif, 0, sizeof(rtl865x_netif_t));
		memcpy(netif.name,vlanconfig[i].ifname,MAX_IFNAMESIZE);
		memcpy(netif.macAddr.octet,vlanconfig[i].mac.octet,ETHER_ADDR_LEN);
		netif.mtu = vlanconfig[i].mtu;
		netif.if_type = vlanconfig[i].if_type;
		netif.vid = vlanconfig[i].vid;
		netif.is_wan = vlanconfig[i].isWan;
		netif.is_slave = vlanconfig[i].is_slave;
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		netif.enableRoute=1;
#endif
		retval = rtl865x_addNetif(&netif);

#ifndef CONFIG_RTL_MULTI_ETH_WAN
		if(netif.is_slave == 1)
#if defined(CONFIG_RTL_PUBLIC_SSID)
			rtl865x_attachMasterNetif(netif.name,RTL_GW_WAN_DEVICE_NAME);
#else
			;//rtl865x_attachMasterNetif(netif.name, RTL_DRV_WAN0_NETIF_NAME);
#endif
#endif
//#if defined(CONFIG_PROC_FS) && defined(CONFIG_NET_SCHED) && defined(CONFIG_RTL_LAYERED_DRIVER)
#if 0
#ifdef CONFIG_RTL_MULTI_ETH_WAN
		if (0==netif.is_slave)
#endif
			memcpy(&netIfName[j++][0], vlanconfig[i].ifname, sizeof(vlanconfig[i].ifname));
#endif

#if defined (CONFIG_RTL_UNKOWN_UNICAST_CONTROL)
		if (vlanconfig[i].isWan==0)
			memcpy(lanIfName, vlanconfig[i].ifname, sizeof(vlanconfig[i].ifname));
#endif
		if(retval != SUCCESS && retval != RTL_EVLANALREADYEXISTS)
			return retval;
		
	}
 
	/*this is a one-shot config*/
	if ((++__865X_Config) == 1)
	{
		for(i=0; i<RTL8651_PORT_NUMBER + 3; i++) 
		{ 
			/* Set each port's PVID */
			for(j=0,pvid=0; vlanconfig[j].vid != 0; j++)
			{
				if ( (1<<i) & vlanconfig[j].memPort ) 
				{
					pvid = vlanconfig[j].vid;
					break;
				}
			}
			
			if (pvid!=0)	
			{
	#ifdef CONFIG_HARDWARE_NAT_DEBUG
	/*2007-12-19*/
			rtlglue_printf("%s:%d:lrconfig[j].vid is %d,pvid is %d, j is %d,i is %d\n",__FUNCTION__,__LINE__,vlanconfig[j].vid,pvid,j, i);
	#endif
	
			CONFIG_CHECK(rtl8651_setAsicPvid(i, pvid));
	#ifdef CONFIG_RTL_HW_L2_ONLY
			rtl865x_setPortToNetif(vlanconfig[j].ifname,i);
	#endif
			}
		}
	}

	#if defined (CONFIG_RTL_HW_QOS_SUPPORT)
	rtl865x_initOutputQueue();  
	#endif

	#if defined (CONFIG_RTL_UNKOWN_UNICAST_CONTROL)
	{
		rtl865x_tblAsicDrv_rateLimitParam_t	asic_rl;
		/* 
	  	* Designer said: The time unit used to achieve rate limit is 1.67s (5/3), hence here we change 
	  	* the time unit to 1 sec.
	  	*/
		bzero(&asic_rl, sizeof(rtl865x_tblAsicDrv_rateLimitParam_t));
		asic_rl.maxToken			= RTL_MAC_REFILL_TOKEN;
		asic_rl.refill_number		= RTL_MAC_REFILL_TOKEN;
		asic_rl.t_intervalUnit		= 1; 
		asic_rl.t_remainUnit		= 1;
		asic_rl.token				= RTL_MAC_REFILL_TOKEN;
		rtl8651_setAsicRateLimitTable(0, &asic_rl);

		macRecordIdx = 0;
		bzero(macRecord, RTL_MAC_RECORD_NUM*sizeof(rtlMacRecord));

		for(i=0;i<RTL_MAC_RECORD_NUM;i++)
		{
			init_timer(&macRecord[i].timer);
			macRecord[i].timer.function = rtl_unkownUnicastTimer;
		}

		WRITE_MEM32(TEACR, (READ_MEM32(TEACR)|EnRateLimitTbAging));
	}
	#endif

	return SUCCESS;
}

#if defined (CONFIG_RTL_UNKOWN_UNICAST_CONTROL)
static void rtl_unkownUnicastTimer(unsigned long data)
{
	rtlMacRecord	*record;
	rtl865x_AclRule_t	rule;

	record = (rtlMacRecord*)data;
	bzero((void*)&rule,sizeof(rtl865x_AclRule_t));
	rule.ruleType_ = RTL865X_ACL_MAC;
	rule.pktOpApp_ = RTL865X_ACL_ONLY_L2;
	rule.actionType_ = RTL865X_ACL_DROP_RATE_EXCEED_PPS;
	memset(rule.dstMacMask_.octet, 0xff, ETHER_ADDR_LEN);
	memcpy(rule.un_ty.dstMac_.octet, record->mac, ETHER_ADDR_LEN);
	rtl865x_del_acl(&rule, lanIfName, RTL865X_ACL_SYSTEM_USED);

	bzero(record, sizeof(rtlMacRecord));
	init_timer(&record->timer);
	record->timer.function = rtl_unkownUnicastTimer;
}

static void	rtl_unkownUnicastUpdate(uint8 *mac)
{
	int	idx;
	rtl865x_AclRule_t	rule;

	for(idx=0;idx<RTL_MAC_RECORD_NUM;idx++)
	{
		if (macRecord[idx].enable==0||memcmp(mac, macRecord[idx].mac, ETHER_ADDR_LEN))
			continue;

		/*	The mac has already recorded	*/
		if (macRecord[idx].cnt==RTL_MAC_THRESHOLD||++macRecord[idx].cnt<RTL_MAC_THRESHOLD)
			return;

		break;
	}

	/*	add/del the rules at lan side */
	bzero((void*)&rule,sizeof(rtl865x_AclRule_t));
	rule.ruleType_ = RTL865X_ACL_MAC;
	rule.pktOpApp_ = RTL865X_ACL_ONLY_L2;
	rule.actionType_ = RTL865X_ACL_DROP_RATE_EXCEED_PPS;
	memset(rule.dstMacMask_.octet, 0xff, ETHER_ADDR_LEN);

	if (idx==RTL_MAC_RECORD_NUM)
	{
		if (macRecord[macRecordIdx].enable!=0&&macRecord[macRecordIdx].cnt>RTL_MAC_THRESHOLD)
		{
			memcpy(rule.un_ty.dstMac_.octet, macRecord[macRecordIdx].mac, ETHER_ADDR_LEN);
			rtl865x_del_acl(&rule, lanIfName, RTL865X_ACL_SYSTEM_USED);
			init_timer(&macRecord[macRecordIdx].timer);
			macRecord[macRecordIdx].timer.function = rtl_unkownUnicastTimer;
		}
		else
		{
			macRecord[macRecordIdx].enable = 1;
		}
		macRecord[macRecordIdx].cnt = 0;
		memcpy(macRecord[macRecordIdx].mac, mac, ETHER_ADDR_LEN);
		macRecordIdx = (macRecordIdx+1)&(RTL_MAC_RECORD_NUM-1);
	}
	else
	{
		memcpy(rule.un_ty.dstMac_.octet, mac, ETHER_ADDR_LEN);
		rtl865x_add_acl(&rule, lanIfName, RTL865X_ACL_SYSTEM_USED);
		macRecord[idx].timer.data = (unsigned long)&(macRecord[idx]);
		mod_timer(&macRecord[idx].timer, jiffies+RTL_MAC_TIMEOUT);
	}
}
#endif

#if 0
#if defined (CONFIG_RTL_IGMP_SNOOPING)

static int re865x_reInitIgmpSetting(int mode)
{
	#if defined (CONFIG_RTL_MULTI_LAN_DEV)
	#else
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	rtl_multicastDeviceInfo_t devInfo;
	uint32 externalPortMask=0;
	#endif	
	int32 i;
	int32 totalVlans=((sizeof(vlanconfig))/(sizeof(struct rtl865x_vlanConfig)))-1;
	
	for(i=0; i<totalVlans; i++)
	{
		#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		if (TRUE==vlanconfig[i].isWan)
		{
			externalPortMask |=vlanconfig[i].memPort;
		}
		else
		{
			devInfo.portMask|=vlanconfig[i].memPort ;
		}
		#endif
		
		if(mode==GATEWAY_MODE)
		{
			rtl_setIgmpSnoopingModuleStaticRouterPortMask(nicIgmpModuleIndex, 0);
		}
		else
		{
			rtl_setIgmpSnoopingModuleStaticRouterPortMask(nicIgmpModuleIndex, 0x01);
		}

		rtl_setIgmpSnoopingModuleUnknownMCastFloodMap(nicIgmpModuleIndex, 0x0);
	}
	#endif
	
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	rtl865x_reinitMulticast();
	//rtl865x_setMulticastExternalPortMask(externalPortMask);
	rtl865x_setMulticastExternalPortMask(0);
	#endif
	rtl865x_igmpSyncLinkStatus();
	return SUCCESS;
}
#endif
#endif

#if defined (CONFIG_RTL_MULTI_LAN_DEV)
unsigned int rtl865x_getEthDevLinkStatus(struct net_device *dev)
{

	if(dev!=NULL)
	{
		struct dev_priv *cp =dev->priv;
		//struct dev_priv *cp=netdev_priv(dev);
		return (cp->portmask & rtl865x_getPhysicalPortLinkStatus());
		
	}
	else
	{	
		return 0;
	}
}
#endif

#if 0
static int32 rtl_reinit_hw_table(void)
{
	/*re-init sequence eventmgr->l4->l3->l2->common is to make sure delete asic entry,
	if not following this sequence, 
	some asic entry can't be deleted due to reference count is not zero*/

	/*to-do:in each layer reinit function, memset all software entry to zero, 
	and force to clear all asic entry of own module, 
	then the re-init sequence can be common->l2->l3->l4 */
	/* FullAndSemiReset should not be called here
	  * it will make switch core action totally wrong
        */

	/*event management */
	#ifdef CONFIG_RTL_LAYERED_DRIVER_L2
	rtl865x_reInitEventMgr();
	#endif

	/*l4*/
	#if defined(CONFIG_RTL_LAYERED_DRIVER_L4)
	rtl865x_nat_reinit();
	#endif

	/*l3*/
	#ifdef CONFIG_RTL_LAYERED_DRIVER_L3
	rtl865x_reinitRouteTable();
	rtl865x_reinitNxtHopTable();
	rtl865x_reinitIpTable();	
	rtl865x_reinitPppTable();	
	rtl865x_arp_reinit();
	#endif

	/*l2*/
	#ifdef CONFIG_RTL_LAYERED_DRIVER_L2
	rtl865x_layer2_reinit();
	#endif

	/*common*/	
	rtl865x_reinitNetifTable();	
	rtl865x_reinitVlantable();
	rtl865x_reinit_acl();

	/*queue id & rx ring descriptor mapping*/
 	REG16(CPUQDM0)=QUEUEID0_RXRING_MAPPING;
 	REG16(CPUQDM1)=QUEUEID1_RXRING_MAPPING;
	REG16(CPUQDM2)=QUEUEID2_RXRING_MAPPING;
	REG16(CPUQDM3)=QUEUEID3_RXRING_MAPPING;
	REG16(CPUQDM4)=QUEUEID4_RXRING_MAPPING;
	REG16(CPUQDM5)=QUEUEID5_RXRING_MAPPING;

	WRITE_MEM32(PLITIMR,0);
	
	rtl8651_setAsicOperationLayer(2);	
	
	return SUCCESS;
	
}
#endif
#if 0
#if defined(CONFIG_RTK_VLAN_SUPPORT) || defined (CONFIG_RTL_MULTI_LAN_DEV)
static int rtl_config_perport_perdev_vlanconfig(int mode,int allLan)
{
	struct net_device* netdevice = NULL;
	int config_idx=0;	
	#define Search_dev_error(F) \
		printk("(%s)Error, cannot get the device that netifname is %s\n", __func__,#F)


	



	/* set LAN  */
	//eth0.2
	strcpy(vlanconfig[config_idx].ifname,RTL_DRV_LAN_P0_NETIF_NAME);
	vlanconfig[config_idx].isWan 		= 0;	
	vlanconfig[config_idx].if_type 		= IF_ETHER;
	vlanconfig[config_idx].vid 			= RTL_LANVLANID;
	vlanconfig[config_idx].fid 			= RTL_LAN_FID;
	vlanconfig[config_idx].memPort 	= RTL_LANPORT_MASK_4;
	vlanconfig[config_idx].untagSet 	= RTL_LANPORT_MASK_4;
	vlanconfig[config_idx].is_slave 		= 0;
	netdevice = rtl_get_psdev_by_ps_drv_netif(vlanconfig[config_idx].ifname);
	if(!netdevice)
	{
		Search_dev_error(vlanconfig[config_idx].ifname);
		return FAILED;
	}
	((struct dev_priv *)netdevice->priv)->portmask = RTL_LANPORT_MASK_4; 
	((struct dev_priv *)netdevice->priv)->portnum = 1;
	config_idx++;


	//eth0.3
	strcpy(vlanconfig[config_idx].ifname,RTL_DRV_LAN_P1_NETIF_NAME);
	vlanconfig[config_idx].isWan 		= 0;	
	vlanconfig[config_idx].if_type 		= IF_ETHER;
	vlanconfig[config_idx].vid 			= RTL_LANVLANID;
	vlanconfig[config_idx].fid 			= RTL_LAN_FID;
	vlanconfig[config_idx].memPort 	= RTL_LANPORT_MASK_3;
	vlanconfig[config_idx].untagSet 	= RTL_LANPORT_MASK_3;
	vlanconfig[config_idx].is_slave 		= 0;
	netdevice = rtl_get_psdev_by_ps_drv_netif(vlanconfig[config_idx].ifname);
	if(!netdevice)
	{
		Search_dev_error(vlanconfig[config_idx].ifname);
		return FAILED;
	}
	((struct dev_priv *)netdevice->priv)->portmask = RTL_LANPORT_MASK_3; 
	((struct dev_priv *)netdevice->priv)->portnum = 1;
	config_idx++;


	//eth0.4
	strcpy(vlanconfig[config_idx].ifname,RTL_DRV_LAN_P2_NETIF_NAME);
	vlanconfig[config_idx].isWan 		= 0;	
	vlanconfig[config_idx].if_type 		= IF_ETHER;
	vlanconfig[config_idx].vid 			= RTL_LANVLANID;
	vlanconfig[config_idx].fid 			= RTL_LAN_FID;
	vlanconfig[config_idx].memPort 	= RTL_LANPORT_MASK_2;
	vlanconfig[config_idx].untagSet 	= RTL_LANPORT_MASK_2;
	vlanconfig[config_idx].is_slave 		= 0;
	netdevice = rtl_get_psdev_by_ps_drv_netif(vlanconfig[config_idx].ifname);
	if(!netdevice)
	{
		Search_dev_error(vlanconfig[config_idx].ifname);
		return FAILED;
	}
	((struct dev_priv *)netdevice->priv)->portmask = RTL_LANPORT_MASK_2; 
	((struct dev_priv *)netdevice->priv)->portnum = 1;
	config_idx++;


	//eth0.5
	strcpy(vlanconfig[config_idx].ifname,RTL_DRV_LAN_P3_NETIF_NAME);
	vlanconfig[config_idx].isWan 		= 0;	
	vlanconfig[config_idx].if_type 		= IF_ETHER;
	vlanconfig[config_idx].vid 			= RTL_LANVLANID;
	vlanconfig[config_idx].fid 			= RTL_LAN_FID;
	vlanconfig[config_idx].memPort 	= RTL_LANPORT_MASK_1;
	vlanconfig[config_idx].untagSet 	= RTL_LANPORT_MASK_1;
	vlanconfig[config_idx].is_slave 		= 0;
	netdevice = rtl_get_psdev_by_ps_drv_netif(vlanconfig[config_idx].ifname);
	if(!netdevice)
	{
		Search_dev_error(vlanconfig[config_idx].ifname);
		return FAILED;
	}
	((struct dev_priv *)netdevice->priv)->portmask = RTL_LANPORT_MASK_1; 
	((struct dev_priv *)netdevice->priv)->portnum = 1;
	config_idx++;


	/* set  WAN */	
	#ifdef CONFIG_RTL_MULTI_ETH_WAN
	//nas0_0 ~ nas0_x
	rtl_adjust_wanport_vlanconfig(vlanconfig,&config_idx);
	#else
	
	//nas0
	strcpy(vlanconfig[config_idx].ifname,RTL_DRV_LAN_P4_NETIF_NAME);
	if(allLan)
		vlanconfig[config_idx].isWan = 0;
	else
		vlanconfig[config_idx].isWan = 1;
	
	vlanconfig[config_idx].if_type 		= IF_ETHER;	

	if((mode == BRIDGE_MODE) || (mode== WISP_MODE))
	{
		vlanconfig[config_idx].vid 			= RTL_BridgeWANVLANID;
		vlanconfig[config_idx].fid 			= RTL_WAN_FID;
		vlanconfig[config_idx].memPort 		= RTL_WANPORT_MASK | RTL_LANPORT_MASK;
		vlanconfig[config_idx].untagSet 		= RTL_WANPORT_MASK | RTL_LANPORT_MASK;
	}
	else
	{
		vlanconfig[config_idx].vid 			= RTL_WANVLANID;
		vlanconfig[config_idx].fid 			= RTL_WAN_FID;
		vlanconfig[config_idx].memPort 		= RTL_WANPORT_MASK;
		vlanconfig[config_idx].untagSet 		= RTL_WANPORT_MASK;
	}
	vlanconfig[config_idx].is_slave 		= 0;
	netdevice = rtl_get_psdev_by_ps_drv_netif(vlanconfig[config_idx].ifname);
	if(!netdevice)
	{
		Search_dev_error(vlanconfig[config_idx].ifname);
		return FAILED;
	}
	((struct dev_priv *)netdevice->priv)->portmask = RTL_WANPORT_MASK; 
	((struct dev_priv *)netdevice->priv)->portnum = 1;
	config_idx++;


	//ppp0
	strcpy(vlanconfig[config_idx].ifname,RTL_DRV_PPP_NETIF_NAME);
	if(allLan)
		vlanconfig[config_idx].isWan = 0;
	else
		vlanconfig[config_idx].isWan = 1;
	
	vlanconfig[config_idx].if_type 		= IF_PPPOE;		
	vlanconfig[config_idx].vid 			= RTL_WANVLANID;
	vlanconfig[config_idx].fid 			= RTL_WAN_FID;
	vlanconfig[config_idx].memPort 		= RTL_WANPORT_MASK;
	vlanconfig[config_idx].untagSet 		= RTL_WANPORT_MASK;
	
	config_idx++;
	#endif


	/* set last entry */
	strcpy(vlanconfig[config_idx].ifname,"");
	vlanconfig[config_idx].vid			= 0;
	vlanconfig[config_idx].fid 			= 0;
	vlanconfig[config_idx].memPort 	= 0;
	vlanconfig[config_idx].untagSet 	= 0;
	
#if 0
	vlanconfig[0].vid = RTL_LANVLANID;
#ifdef CONFIG_RTL_MULTI_ETH_WAN
	vlanconfig[0].isWan = 0;
#endif
	vlanconfig[0].memPort = RTL_LANPORT_MASK_4;
	vlanconfig[0].untagSet= RTL_LANPORT_MASK_4;			
	((struct dev_priv *)_rtl86xx_dev.dev[0]->priv)->portmask = RTL_LANPORT_MASK_4; //eth0.2
	//((struct dev_priv *)_rtl86xx_dev.dev[0]->priv)->id = RTL_LANVLANID_1; //eth0.2
	((struct dev_priv *)_rtl86xx_dev.dev[0]->priv)->portnum = 1;

	if((mode == BRIDGE_MODE) || (mode== WISP_MODE))
	{
		vlanconfig[1] .vid = RTL_LANVLANID; 
		//((struct dev_priv *)_rtl86xx_dev.dev[1]->priv)->id = RTL_LANVLANID; 
	}
	else
	{	
		vlanconfig[1] .vid = RTL_WANVLANID; 
		//((struct dev_priv *)_rtl86xx_dev.dev[1]->priv)->id = RTL_WANVLANID; 
	}		
	if(allLan)
		vlanconfig[1].isWan = 0;
	else
		vlanconfig[1].isWan = 1;
	vlanconfig[1].memPort = RTL_WANPORT_MASK;
	vlanconfig[1].untagSet= RTL_WANPORT_MASK;			
	((struct dev_priv *)_rtl86xx_dev.dev[1]->priv)->portmask = RTL_WANPORT_MASK; //nas0
	//((struct dev_priv *)_rtl86xx_dev.dev[1]->priv)->id = RTL_WANVLANID; //nas0
	((struct dev_priv *)_rtl86xx_dev.dev[1]->priv)->portnum = 1;
	
	vlanconfig[2] .vid = RTL_LANVLANID;
	vlanconfig[2].memPort = RTL_LANPORT_MASK_3;
	vlanconfig[2].untagSet = RTL_LANPORT_MASK_3;
	((struct dev_priv *)_rtl86xx_dev.dev[2]->priv)->portmask = RTL_LANPORT_MASK_3; //eth0.3
	//((struct dev_priv *)_rtl86xx_dev.dev[2]->priv)->id = RTL_LANVLANID_2; //eth0.3
	((struct dev_priv *)_rtl86xx_dev.dev[2]->priv)->portnum = 1;
	
	vlanconfig[3] .vid = RTL_LANVLANID;
	vlanconfig[3].memPort = RTL_LANPORT_MASK_2;
	vlanconfig[3].untagSet = RTL_LANPORT_MASK_2;
	((struct dev_priv *)_rtl86xx_dev.dev[3]->priv)->portmask = RTL_LANPORT_MASK_2; //eth0.4
	//((struct dev_priv *)_rtl86xx_dev.dev[3]->priv)->id = RTL_LANVLANID_3; //eth0.4
	((struct dev_priv *)_rtl86xx_dev.dev[3]->priv)->portnum = 1;
	
	
	vlanconfig[4] .vid = RTL_LANVLANID;
	vlanconfig[4].memPort = RTL_LANPORT_MASK_1;
	vlanconfig[4].untagSet = RTL_LANPORT_MASK_1;
	((struct dev_priv *)_rtl86xx_dev.dev[4]->priv)->portmask = RTL_LANPORT_MASK_1; //eth0.5
	//((struct dev_priv *)_rtl86xx_dev.dev[4]->priv)->id = RTL_LANVLANID_4; //eth0.5
	((struct dev_priv *)_rtl86xx_dev.dev[4]->priv)->portnum = 1;
	

	#if defined(CONFIG_8198_PORT5_GMII)
	vlanconfig[5] .vid = RTL_LANVLANID;
	vlanconfig[5].memPort = RTL_LANPORT_MASK_5;
	vlanconfig[5].untagSet = RTL_LANPORT_MASK_5;
	((struct dev_priv *)_rtl86xx_dev.dev[5]->priv)->portmask = RTL_LANPORT_MASK_5;
	//((struct dev_priv *)_rtl86xx_dev.dev[5]->priv)->id = RTL_LANVLANID_5;
	((struct dev_priv *)_rtl86xx_dev.dev[5]->priv)->portnum = 1;
	#endif

	#if defined(CONFIG_8198_PORT5_GMII)
	if(allLan)
		vlanconfig[6].isWan = 0; //ppp0
	else
		vlanconfig[6].isWan = 1;
	#else
	#ifndef CONFIG_RTL_MULTI_ETH_WAN
	if(allLan)
		vlanconfig[5].isWan = 0;
	else
		vlanconfig[5].isWan = 1;
	#else
	for (i=5; i<17; i++) {
		vlanconfig[i].isWan = allLan?0:1;
	}
	#endif
	#endif
	#ifdef CONFIG_RTL_MULTI_ETH_WAN
	rtl_adjust_wanport_vlanconfig(vlanconfig);
	#endif /* CONFIG_RTL_MULTI_ETH_WAN */
#endif /* old version */	
	
	return SUCCESS;
}
#endif
#ifndef CONFIG_RTL_MULTI_LAN_DEV
static int rtl_config_lanwan_dev_vlanconfig(int mode)
{
	/*lan config*/
	{
		vlanconfig[2].vid = 0;
		vlanconfig[3].vid = 0;
		//vlanconfig[4].vid = 0;
		#if defined(CONFIG_8198_PORT5_GMII)
		vlanconfig[5].vid = 0;
		#endif
	}	
	
#if defined (CONFIG_RTL_IVL_SUPPORT)
	#if defined(CONFIG_POCKET_ROUTER_SUPPORT)
		if(rtl865x_curOpMode == GATEWAY_MODE)
		{
			vlanconfig[0].memPort = 0;
			vlanconfig[0].vid=RTL_LANVLANID;
			vlanconfig[0].untagSet = 0;
			((struct dev_priv *)_rtl86xx_dev.dev[0]->priv)->portmask = 0; //eth0
			((struct dev_priv *)_rtl86xx_dev.dev[0]->priv)->id = RTL_LANVLANID;
			vlanconfig[1].vid = RTL_WANVLANID;
			vlanconfig[1].memPort = RTL_WANPORT_MASK;
			vlanconfig[1].untagSet = RTL_WANPORT_MASK;
			((struct dev_priv *)_rtl86xx_dev.dev[1]->priv)->portmask = RTL_WANPORT_MASK; //eth1
			((struct dev_priv *)_rtl86xx_dev.dev[1]->priv)->id = RTL_WANVLANID; //eth1
		}
		else
		{
			vlanconfig[0].memPort = RTL_LANPORT_MASK;
			vlanconfig[0].vid=RTL_LANVLANID;
			vlanconfig[0].untagSet = RTL_LANPORT_MASK;
			((struct dev_priv *)_rtl86xx_dev.dev[0]->priv)->portmask = RTL_LANPORT_MASK; //eth0
			((struct dev_priv *)_rtl86xx_dev.dev[0]->priv)->id = RTL_LANVLANID;
			
			vlanconfig[1].vid = 0;
			vlanconfig[1].memPort = 0;
			vlanconfig[1].untagSet = 0;
			((struct dev_priv *)_rtl86xx_dev.dev[1]->priv)->portmask = 0; //eth1
			((struct dev_priv *)_rtl86xx_dev.dev[1]->priv)->id = RTL_WANVLANID; //eth1
		}
	
	#else //else CONFIG_POCKET_ROUTER_SUPPORT
		vlanconfig[0].memPort = RTL_LANPORT_MASK;
		vlanconfig[0].vid=RTL_LANVLANID;
		vlanconfig[0].untagSet = RTL_LANPORT_MASK;
		((struct dev_priv *)_rtl86xx_dev.dev[0]->priv)->portmask = RTL_LANPORT_MASK; //eth0
		((struct dev_priv *)_rtl86xx_dev.dev[0]->priv)->id = RTL_LANVLANID;
		
		vlanconfig[1].vid = RTL_WANVLANID;
		vlanconfig[1].memPort = RTL_WANPORT_MASK;
		vlanconfig[1].untagSet = RTL_WANPORT_MASK;
		((struct dev_priv *)_rtl86xx_dev.dev[1]->priv)->portmask = RTL_WANPORT_MASK; //eth1
		((struct dev_priv *)_rtl86xx_dev.dev[1]->priv)->id = RTL_WANVLANID; //eth1
	#endif //endif CONFIG_POCKET_ROUTER_SUPPORT
	
#else
	if(rtl865x_curOpMode == BRIDGE_MODE || rtl865x_curOpMode== WISP_MODE)
	{
		vlanconfig[0].memPort = 0x1ff;
		vlanconfig[0].untagSet = 0x1ff;
		vlanconfig[0].vid=RTL_LANVLANID;		
		((struct dev_priv *)_rtl86xx_dev.dev[0]->priv)->portmask = 0x1ff; //eth0
		((struct dev_priv *)_rtl86xx_dev.dev[0]->priv)->id = RTL_LANVLANID;
		
		vlanconfig[1].vid = 0;
		vlanconfig[1].memPort=0x0;
		vlanconfig[1].untagSet=0x0;
		((struct dev_priv *)_rtl86xx_dev.dev[1]->priv)->portmask = 0x0; //eth1
		((struct dev_priv *)_rtl86xx_dev.dev[1]->priv)->id = 0;
	}
	else
	{
		vlanconfig[0].memPort = RTL_LANPORT_MASK;
		vlanconfig[0].vid=RTL_LANVLANID;
		vlanconfig[0].untagSet = RTL_LANPORT_MASK;
		((struct dev_priv *)_rtl86xx_dev.dev[0]->priv)->portmask = RTL_LANPORT_MASK; //eth0
		((struct dev_priv *)_rtl86xx_dev.dev[0]->priv)->id = RTL_LANVLANID;
		
		vlanconfig[1].vid = RTL_WANVLANID;
		vlanconfig[1].memPort = RTL_WANPORT_MASK;
		vlanconfig[1].untagSet = RTL_WANPORT_MASK;
		((struct dev_priv *)_rtl86xx_dev.dev[1]->priv)->portmask = RTL_WANPORT_MASK; //eth1
		((struct dev_priv *)_rtl86xx_dev.dev[1]->priv)->id = RTL_WANVLANID; //eth1
	}
#endif

	return SUCCESS;
}
#endif
#if	defined (CONFIG_RTL_MULTI_LAN_DEV)
static int rtl_config_multi_lan_dev_vlanconfig(int mode,int allLan)
{	
	rtl_config_perport_perdev_vlanconfig(mode,allLan);
	return SUCCESS;
	
}
#endif
#endif
#if defined(CONFIG_RTK_VLAN_SUPPORT)
static int rtl_config_rtkVlan_vlanconfig(int mode)
{	
	if(!rtk_vlan_support_enable)
		rtl_config_lanwan_dev_vlanconfig(mode);
	else if(rtk_vlan_support_enable == 1)
	{
		rtl_config_perport_perdev_vlanconfig(mode);
	}
	else if(rtk_vlan_support_enable == 2)
		rtl_config_lanwan_dev_vlanconfig(mode);

	return SUCCESS;
}
#endif

#if 0
static int rtl_config_vlanconfig(int mode,int allLan)
{
	#if	defined (CONFIG_RTL_MULTI_LAN_DEV)
		rtl_config_multi_lan_dev_vlanconfig(mode,allLan);
	#else
		#if defined(CONFIG_RTK_VLAN_SUPPORT)
			rtl_config_rtkVlan_vlanconfig(mode);
		#else
			rtl_config_lanwan_dev_vlanconfig(mode);
		#endif
	#endif

	return SUCCESS;
	
}
#endif

#if 0
#ifdef CONFIG_RTL_MULTI_ETH_WAN
static int32 update_vlan_configure(struct rtl865x_vlanConfig new_vlanconfig[])
{
	int32 i;
	struct rtl865x_vlanConfig *pvlanconfig = NULL;
	int32 totalVlans = 0;	
	pvlanconfig = new_vlanconfig;


	/*get input vlan config entry number*/
	for(i=0; pvlanconfig[i].ifname[0] != '\0' ; i++)
	{
		if(pvlanconfig[i].vid != 0)
			totalVlans++;
	}	
	
	//because the new_vlanconfig should be packedVlanConfig
	totalVlans = totalVlans > NETIF_SW_NUMBER? NETIF_SW_NUMBER:totalVlans;

	for(i=0; i<totalVlans; i++)
	{
		if(pvlanconfig[i].vid == 0)
			continue;

		/*add vlan*/
		if(pvlanconfig[i].if_type==IF_ETHER)
		{
			rtl865x_modVlanPortMember(pvlanconfig[i].vid,pvlanconfig[i].memPort, pvlanconfig[i].untagSet);
		}
	}

	rtl865x_setpvid(pvlanconfig);
	#if 0
	for(i=0; i<RTL8651_PORT_NUMBER + 3; i++)
	{
		/* Set each port's PVID */
		for(j=0; pvlanconfig[j].vid != 0; j++)
		{
			if ( pvlanconfig[j].vid == pvid_per_port[i]) 
			{
				break;
			}
		}

		if (pvlanconfig[j].vid) {
			CONFIG_CHECK(rtl8651_setAsicPvid(i,pvid_per_port[i]));
			rtl865x_setPortToNetif(pvlanconfig[j].ifname, i);
		}
	}
	#endif

	return SUCCESS;
}

int32 rtl865x_updateNetifForPortmapping(void)
{
	/*config vlan config*/
	//rtl_config_vlanconfig(rtl865x_curOpMode, rtl865x_curOpMode_allLan);

	if (!vlanconfig[0].vid && !vlanconfig[1].vid )
		return RTL_EINVALIDVLANID;

	re865x_packVlanConfig(vlanconfig, packedVlanConfig);

	update_vlan_configure(packedVlanConfig);

	return SUCCESS;
}
#endif //CONFIG_RTL_MULTI_ETH_WAN
#endif
#if defined (CONFIG_RTL_8676HWNAT) && defined (CONFIG_ETHWAN)

#if !defined(CONFIG_RTL_MULTI_ETH_WAN)
int getVlanconfigIdxByWan(struct rtl865x_vlanConfig *vconfig)
{
	int i;

	for(i=0; vconfig[i].ifname[0] != '\0' ; i++)
	{
		if( vconfig[i].isWan)
			return i;
	}

	return -1;
}
#endif

static int _rtl_update_vlan_table(int vlan_id)
{
	int i;
	int _the_num_of_this_vid_WAN=0;
	int _has_bridged_WAN=0;

	/*	member ports are always WAN + LAN 	*/
	for(i=0; vlanconfig[i].ifname[0] != '\0' ; i++)
	{
		if( vlanconfig[i].isWan&& vlanconfig[i].is_slave==0 && vlanconfig[i].vid==vlan_id)	
		{
			_the_num_of_this_vid_WAN++;	
			if(vlanconfig[i].protocol==SMUX_PROTO_BRIDGE)			
				_has_bridged_WAN = 1;		
		}
	}


	if(_the_num_of_this_vid_WAN >0 )
	{	
		uint32		memPort;
		uint32		untagSet;
		int 			ret;


		if(_has_bridged_WAN)
		{
			memPort 	= (RTL_LANPORT_MASK | RTL_WANPORT_MASK);
			if(vlan_id==RTL_BridgeWANVLANID)
				untagSet 	= (RTL_LANPORT_MASK | RTL_WANPORT_MASK);
			else
				untagSet 	= RTL_LANPORT_MASK;
		}
		else
		{		
			memPort 	= RTL_WANPORT_MASK;
			if(vlan_id==RTL_WANVLANID)
				untagSet 	= RTL_WANPORT_MASK;
			else
				untagSet 	= 0;
		}
		ret = rtl865x_addVlan(vlan_id);	
		if(ret == SUCCESS)
		{
			/*  this vlan is the first time to setup , set its fid */
			if(rtl865x_setVlanFilterDatabase(vlan_id,RTL_WAN_FID)!=SUCCESS)
			{
				DBG_MULTIWAN_API_PRK("Leave %s @ %d \n",__func__,__LINE__);
				return FAILED;
			}
		}
		else if(ret!=RTL_EVLANALREADYEXISTS)
		{
			DBG_MULTIWAN_API_PRK("Leave %s @ %d \n",__func__,__LINE__);
			return FAILED;
		}	

		/*  No matter whether we has setup this vlan table before or not , reset its member/untag ports  */
		if(rtl865x_modVlanPortMember(vlan_id,memPort,untagSet)!=SUCCESS)
		{
			DBG_MULTIWAN_API_PRK("Leave %s @ %d \n",__func__,__LINE__);
			return FAILED;
		}		
	}
	else
	{
		if(rtl865x_delVlan(vlan_id)!=SUCCESS)
		{
			DBG_MULTIWAN_API_PRK("Leave %s @ %d \n",__func__,__LINE__);
			return FAILED;
		}		
	}

	return SUCCESS;
}

int32 _rtl_update_pvid(void)
{
	int i,j;
	int bridged_wan_exist = 0;
	int novlantag_bridged_wan_exist = 0;
	int novlantag_routing_wan_exist = 0;

	DBG_MULTIWAN_API_PRK("Enter %s\n",__func__);

	re865x_packVlanConfig(vlanconfig, packedVlanConfig);	


	for(j=0; packedVlanConfig[j].vid != 0; j++)
	{
		 if( packedVlanConfig[j].isWan)
		 {
			if(packedVlanConfig[j].memPort != RTL_WANPORT_MASK )
		 	{
		 		bridged_wan_exist = 1;		
				if(packedVlanConfig[j].vid==RTL_BridgeWANVLANID)
					novlantag_bridged_wan_exist=1;
			}
			else
			{
				if(packedVlanConfig[j].vid==RTL_WANVLANID)
					novlantag_routing_wan_exist=1;
			}
		 }
	}


	DBG_MULTIWAN_API_PRK("(%s) bridged_wan_exist:%d   novlantag_bridged_wan_exist:%d  novlantag_routing_wan_exist:%d \n"
		,__func__,bridged_wan_exist,novlantag_bridged_wan_exist,novlantag_routing_wan_exist);

	
	
	for(i=0; i<RTL8651_PORT_NUMBER + 3; i++) 
	{
		uint16 pvid;
		/* wan port*/
		if( (1<<i)&RTL_WANPORT_MASK )
		{				
			if(novlantag_bridged_wan_exist)
				pvid = RTL_BridgeWANVLANID;
			else
				pvid = RTL_WANVLANID;					
			

			if(novlantag_bridged_wan_exist && novlantag_routing_wan_exist)
				rtl8651_setProtocolBasedVLAN(RTL8651_PBV_RULE_PPPOE_SESSION,i,1,RTL_WANVLANID);
			else
				rtl8651_setProtocolBasedVLAN(RTL8651_PBV_RULE_PPPOE_SESSION,i,0,0);
		}
		/* lan port */
		else
		{
			if(!bridged_wan_exist)
				pvid = RTL_LANVLANID;
			else
			{
				/* if the lan port only  exist in only one bridged WAN, we can set pvid as its vid for upstream hw l2 forwarding*/
				int this_port_exist_in_bridged_wan_num=0;
				int the_first_vid_exist_in_bridged_wan=-1;			

				for(j=0; packedVlanConfig[j].vid != 0; j++)			
					if ( packedVlanConfig[j].isWan && packedVlanConfig[j].memPort != RTL_WANPORT_MASK && (packedVlanConfig[j].memPort&(1<<i)) ) 
					{
						this_port_exist_in_bridged_wan_num ++;			
						if(the_first_vid_exist_in_bridged_wan==-1)
							the_first_vid_exist_in_bridged_wan = packedVlanConfig[j].vid;
					}
					
				if(this_port_exist_in_bridged_wan_num==1)
					pvid = the_first_vid_exist_in_bridged_wan;
				else
					pvid = RTL_LANVLANID;	
			}
		}

		/* extport patch*/	
		if(i==8)			
			pvid = RTL_LANVLANID;

		
		//set pvid
		CONFIG_CHECK(rtl8651_setAsicPvid(i,pvid));
		pvid_per_port[i]=pvid;
	

		//set port to netif	
		for(j=0; packedVlanConfig[j].vid != 0; j++)		
			if ( packedVlanConfig[j].vid == pvid) 			
				break;		
		rtl865x_setPortToNetif(packedVlanConfig[j].ifname, i);	
	}


	return SUCCESS;
	
}

/*
 * FUNC                    : rtl_add_ppp_netif()
 * DESC                    : Only ppp0 exists in netif table now. When we add a new additional pppoe connection, no netif entry is available for it, 
 *                                so we must create a new netif entry for this connection.
 * RETURN VALUE     : int32   SUCCESS or FAIL
 * DATE                    : 2012-02-22 QL
 */
int32 rtl_add_ppp_netif(char *ifname)
{
	struct rtl865x_netif_s netif;
	int idx = -1;
	int i;
	int retval;
	
	if (strncmp(ifname, "ppp", 3))
		return FAILED;

	for (i=0; vlanconfig[i].ifname[0] != '\0'; i++)
	{
		if (vlanconfig[i].isWan && vlanconfig[i].is_slave && !strcmp(vlanconfig[i].ifname, ifname))
		{
			/*QL 2012-3-1 vlanconfg[] has been modified, vid of all ppp entries is set to 0, therefore, ppp0 will not be created in initial time.*/
			//if (0 != vlanconfig[i].vid) {
				idx = i;
				break;
			//}
		}
	}

	if (-1 == idx)
	{
		printk("%s: no vlanconfig entry availble for %s!\n", __func__, ifname);
		return FAILED;
	}

	/*add network interface*/
	memset(&netif, 0, sizeof(rtl865x_netif_t));
	memcpy(netif.name,vlanconfig[idx].ifname,MAX_IFNAMESIZE);
	memcpy(netif.macAddr.octet,vlanconfig[idx].mac.octet,ETHER_ADDR_LEN);
	netif.mtu = vlanconfig[idx].mtu;
	netif.if_type = vlanconfig[idx].if_type;
	netif.vid = vlanconfig[idx].vid;
	netif.is_wan = vlanconfig[idx].isWan;
	netif.is_slave = vlanconfig[idx].is_slave;
	//#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	netif.enableRoute=1;
	//#endif
	retval = rtl865x_addNetif(&netif);

	return retval;
}

int rtl_set_wanport_vlanconfig(char *ifname, int proto, int vid, int napt)
{
	int idx=-1;
    int vlan_id;
	rtl865x_netif_t netif;
	int i;

	DBG_MULTIWAN_API_PRK("Enter %s (ifname:%s  protio:%d   vid:%d  napt:%d)\n",__func__,
		ifname,proto,vid,napt);

     /*  Get available idx in "vlanconfig" */
#if !defined(CONFIG_RTL_MULTI_ETH_WAN)
	idx = getVlanconfigIdxByWan(vlanconfig);
#else
	/* we has to check whether ifname existed and has not been used yet  
		Otherwise, there will exist two same netif_name in "vlanconfig"
	*/
	for(i=0; vlanconfig[i].ifname[0] != '\0' ; i++)
	{
		if( vlanconfig[i].isWan&& vlanconfig[i].is_slave==0 && !strcmp(vlanconfig[i].ifname,ifname))	
		{
			if(vlanconfig[i].vid==0) /* this wan has not been used */
			{		
				idx = i;
				break;
			}
		}
	}
#endif
	

	if (idx == -1)
	{
		DBG_MULTIWAN_API_PRK("Leave %s @ %d \n",__func__,__LINE__);
		return FAILED;
	}


	/* Note.  the fields except for vid,isWan,proto in "vlanconfig" have no change */
	/* update  vid  in "vlanconfig"  */
	if (vid != -1) {
		vlan_id = vid&VLAN_VID_MASK;
		}
	else {		
		if (proto == SMUX_PROTO_BRIDGE)
			vlan_id = RTL_BridgeWANVLANID;
		else
			vlan_id = RTL_WANVLANID;	
	}
   	 vlanconfig[idx].vid = vlan_id;
	 DBG_MULTIWAN_API_PRK("(%s)vid:%d\n",__func__,vlan_id);


	/* update  proto  in "vlanconfig"  */
	vlanconfig[idx].protocol = proto;

	

	/* sanity check , this netifname does not exist in sw_netif  */
	if(rtl865x_netifExist(ifname))
	{
		/* BUG !  vlanconfig & sw_neif no sync ??  */
		DBG_MULTIWAN_API_PRK("Leave %s @ %d \n",__func__,__LINE__);
		return FAILED;
	}	
	
	/* Update VLAN table */
	if(_rtl_update_vlan_table(vlan_id)!=SUCCESS)
	{
		DBG_MULTIWAN_API_PRK("Leave %s @ %d \n",__func__,__LINE__);
		return FAILED;
	}


	/* Update netif table */
	memset(&netif, 0, sizeof(rtl865x_netif_t));
	memcpy(netif.name,ifname,MAX_IFNAMESIZE);
	netif.mtu 		= 1500;
	netif.if_type 		= IF_ETHER;
	netif.vid 			= vlan_id;
	if( proto != SMUX_PROTO_BRIDGE )
		netif.is_wan 	= napt;
	else
		netif.is_wan 	= 0;
	netif.is_slave 		= 0;
	netif.enableRoute	= 1;
	if(rtl865x_addNetif(&netif)!=SUCCESS)
	{
		DBG_MULTIWAN_API_PRK("Leave %s @ %d \n",__func__,__LINE__);
		return FAILED;
	}
	
#ifdef CONFIG_RTL8196E_IPCHECKSUM_ERROR_PATCH
	/* Trap the brdiged packet with both vlan and pppoe packet  */
	if(proto == SMUX_PROTO_BRIDGE && vid != -1)
	{
		rtl865x_AclRule_t rule;
		
		DBG_MULTIWAN_API_PRK("(%s) Add ACL to trap the brdiged unicast packet with both vlan and pppoe tag \n",__func__);
		memset(&rule, 0,sizeof(rtl865x_AclRule_t));		
		rule.ruleType_			= RTL865X_ACL_MAC;
		rule.actionType_			= RTL865X_ACL_TOCPU;	
		rule.pktOpApp_ 			= RTL865X_ACL_ALL_LAYER;
		rule.direction_	 		= RTL865X_ACL_INGRESS;			
		rule.un_ty.typeLen_		= 0x8864;
		rule.un_ty.typeLenMask_	= 0xffff;
		if(rtl865x_add_acl(&rule, netif.name, RTL865X_ACL_ICBUG_PATCH,1,1)!=SUCCESS)
		{
			DBG_MULTIWAN_API_PRK("Leave %s @ %d \n",__func__,__LINE__);
			return FAILED;
		}		
	}
#endif

	/* If this vid =7 , use vid 8's mac addr */
	if(vlan_id == RTL_BridgeWANVLANID)
	{
		char vid_8_netif_name[30];
		DBG_MULTIWAN_API_PRK("(%s) this netif's vid =7 , check whether vid 8 existed , use vid 8's mac addr ... \n",__func__);

		rtl865x_getMasterNetifByVid(RTL_WANVLANID,vid_8_netif_name);

		if(strcmp(vid_8_netif_name,""))
		{						
			rtl865x_netif_local_t* netif_vid_8 = _rtl865x_getNetifByName(vid_8_netif_name);
			rtl865x_netif_t netif_set_mac;
			if(netif_vid_8==NULL)
			{
				DBG_MULTIWAN_API_PRK("Leave %s @ %d \n",__func__,__LINE__);
				return FAILED;
			}

			memcpy(netif_set_mac.macAddr.octet, netif_vid_8->macAddr.octet, ETHER_ADDR_LEN);
			memcpy(netif_set_mac.name, ifname, MAX_IFNAMESIZE);			
			
			if(rtl865x_setNetifMac(&netif_set_mac)!=SUCCESS)
			{
				DBG_MULTIWAN_API_PRK("Leave %s @ %d \n",__func__,__LINE__);
				return FAILED;
			}
		}				
	}
	

	
	
	/* set pvid */
	_rtl_update_pvid();
	
	return SUCCESS;	
}

int rtl_set_wanport_portmapping(char *ifname,unsigned int member)
{
	int idx=-1;
	int i;
	
	DBG_MULTIWAN_API_PRK("Enter %s (ifname:%s  member:0x%X)\n",__func__,
		ifname,member);

	for(i=0; vlanconfig[i].ifname[0] != '\0' ; i++)
	{
		if( vlanconfig[i].isWan&& vlanconfig[i].is_slave==0 && !strcmp(vlanconfig[i].ifname,ifname))	
		{
			if(vlanconfig[i].vid!=0) /* we are really using this wan now */
			{
				idx = i;
				break;
			}
		}
	}		
	if (idx <0)
	{
		DBG_MULTIWAN_API_PRK("Leave %s @ %d \n",__func__,__LINE__);
		return FAILED;
	}	

	DBG_MULTIWAN_API_PRK("(%s) idx = %d  \n",__func__,idx);
	

	vlanconfig[idx].memPort 	= RTL_WANPORT_MASK;
	vlanconfig[idx].untagSet 	= RTL_WANPORT_MASK;
			
	if (vlanconfig[idx].protocol  == SMUX_PROTO_BRIDGE)
	{
		//set membership
		if (member == 0xFFFFFFFF) {
			vlanconfig[idx].memPort |= (RTL_LANPORT_MASK | RTL_WANPORT_MASK);
			vlanconfig[idx].untagSet |= (RTL_LANPORT_MASK | RTL_WANPORT_MASK);
		}
		else {
			vlanconfig[idx].memPort |= ((member&RTL_LANPORT_MASK)|0x100);
			vlanconfig[idx].untagSet |= ((member&RTL_LANPORT_MASK)|0x100);
		}
	}
	if (( vlanconfig[idx].vid != RTL_BridgeWANVLANID && vlanconfig[idx].vid != RTL_WANVLANID)
		#ifdef UNIQUE_MAC_PER_DEV
		&& (vid > 8)
		#endif
	) {
		vlanconfig[idx].untagSet &= (~RTL_WANPORT_MASK);
	}	
	
	/* set pvid */
	_rtl_update_pvid();		
		
	return SUCCESS;
	
}



int rtl865x_unregisterDev(char *ifname) 
{
	int idx=-1;
	int i;
	int vlan_id=-1;
	
	DBG_MULTIWAN_API_PRK("Enter %s (ifname:%s)\n",__func__,ifname);

	
	for(i=0; vlanconfig[i].ifname[0] != '\0' ; i++)
	{
		if( vlanconfig[i].isWan&& vlanconfig[i].is_slave==0 && !strcmp(vlanconfig[i].ifname,ifname))	
		{
			if(vlanconfig[i].vid!=0) /* we are really using this wan now */
			{
				idx = i;
				vlan_id = vlanconfig[i].vid;
				break;
			}
		}
	}		
	if (idx <0)
	{
		DBG_MULTIWAN_API_PRK("Leave %s @ %d \n",__func__,__LINE__);
		return FAILED;
	}

	
	/* Update vlanconfig (reassign vid back to 0) */
	vlanconfig[idx].isWan 		= 1;	
	vlanconfig[idx].if_type 	= IF_ETHER;
	vlanconfig[idx].vid 		= 0;
	vlanconfig[idx].fid 		= RTL_WAN_FID;
	vlanconfig[idx].memPort 	= RTL_WANPORT_MASK;
	vlanconfig[idx].untagSet 	= RTL_WANPORT_MASK;
	vlanconfig[idx].is_slave 	= 0;
	

	/* sanity check , this netifname exists in sw_netif actually */
	if(!rtl865x_netifExist(ifname))
	{
		/* BUG !  vlanconfig & sw_neif no sync ??  */
		DBG_MULTIWAN_API_PRK("Leave %s @ %d \n",__func__,__LINE__);
		return FAILED;
	}
	

	/* Update netif table */
	if(rtl865x_delNetif(ifname)!=SUCCESS)
	{
		DBG_MULTIWAN_API_PRK("Leave %s @ %d \n",__func__,__LINE__);
		return FAILED;
	}

	/* Update VLAN table */
	if(_rtl_update_vlan_table(vlan_id)!=SUCCESS)
	{
		DBG_MULTIWAN_API_PRK("Leave %s @ %d \n",__func__,__LINE__);
		return FAILED;	
	}

	/* set pvid */
	_rtl_update_pvid();
	
	return SUCCESS;
	
}

int rtl865x_setNetifMacAddr(char *ifname, unsigned char *addr)
{

	int vid;
	rtl865x_netif_t netif;

	DBG_MULTIWAN_API_PRK("Enter %s (ifname:%s   addr:%02X:%02X:%02X:%02X:%02X:%02X:) \n",__func__,ifname,
		addr[0],addr[1],addr[2],addr[3],addr[4],addr[5]);
	
	memcpy(netif.macAddr.octet, addr, ETHER_ADDR_LEN);
	memcpy(netif.name, ifname, MAX_IFNAMESIZE);
	if(rtl865x_setNetifMac(&netif)!=SUCCESS)
	{
		DBG_MULTIWAN_API_PRK("Leave %s @ %d \n",__func__,__LINE__);
		return FAILED;
	}	


	if(rtl865x_getNetifVid(ifname,&vid)==SUCCESS)
	{
		if(vid == RTL_WANVLANID)
		{
			char vid_7_netif_name[30];
			DBG_MULTIWAN_API_PRK("(%s) this netif's vid =8 ,  set vid 7 either... \n",__func__);

			rtl865x_getMasterNetifByVid(RTL_BridgeWANVLANID,vid_7_netif_name);

			if(strcmp(vid_7_netif_name,""))
			{			
				memcpy(netif.name, vid_7_netif_name, MAX_IFNAMESIZE);
				if(rtl865x_setNetifMac(&netif)!=SUCCESS)
				{
					DBG_MULTIWAN_API_PRK("Leave %s @ %d \n",__func__,__LINE__);
					return FAILED;
				}
			}			
		}
	}
	else
	{
		DBG_MULTIWAN_API_PRK("Leave %s @ %d \n",__func__,__LINE__);
		return FAILED;
	}

	
	return SUCCESS;
}
#endif //CONFIG_RTL_8676HWNAT

#if 0
int32 rtl865x_changeOpMode(int mode,int allLan)
{
#if defined (CONFIG_RTL_LOCAL_PUBLIC)
	struct rtl865x_interface_info ifInfo;
#endif

#if defined(CONFIG_RTK_VLAN_SUPPORT)
	if(rtk_vlan_support_enable==0)
	{
		if(mode==rtl865x_curOpMode && allLan==rtl865x_curOpMode_allLan)
		{
			return SUCCESS;
		}
	}
#else
#ifndef CONFIG_RTL_MULTI_ETH_WAN
	if(mode==rtl865x_curOpMode && allLan==rtl865x_curOpMode_allLan)
	{
		printk("(%s)current mode has been ever set to %d (allLan:%d)\n ",__func__,mode,allLan);
		return SUCCESS;
	}
#endif
#endif

	/*config vlan config*/
	//rtl_config_vlanconfig(mode,allLan);

	if (!vlanconfig[0].vid && !vlanconfig[1].vid )
		return RTL_EINVALIDVLANID;

#if defined (CONFIG_RTL_LOCAL_PUBLIC)
	if(mode==GATEWAY_MODE)
	{
		memset(&ifInfo, 0 , sizeof(struct rtl865x_interface_info));
#if defined(CONFIG_RTL_PUBLIC_SSID)
		strcpy(ifInfo.ifname,RTL_GW_WAN_DEVICE_NAME);
#else
		strcpy(ifInfo.ifname, RTL_DRV_WAN0_NETIF_NAME);
#endif
		ifInfo.isWan=1;
		for(i=0;vlanconfig[i].vid!=0; i++)
		{
			if ((TRUE==vlanconfig[i].isWan) && (vlanconfig[i].if_type==IF_ETHER))
			{
				ifInfo.memPort|=vlanconfig[i].memPort;
				ifInfo.fid=vlanconfig[i].fid;
			}
		}
		rtl865x_setLpIfInfo(&ifInfo);

		memset(&ifInfo, 0 , sizeof(struct rtl865x_interface_info));
		strcpy(ifInfo.ifname, RTL_DRV_LAN_NETIF_NAME);
		ifInfo.isWan=0;
		for(i=0;vlanconfig[i].vid!=0;i++)
		{
			if ((FALSE==vlanconfig[i].isWan) && (vlanconfig[i].if_type==IF_ETHER))
			{
				ifInfo.memPort|=vlanconfig[i].memPort;
				ifInfo.fid=vlanconfig[i].fid;
			}
		}
		rtl865x_setLpIfInfo(&ifInfo);
	}
	else if(mode==WISP_MODE)
	{
		memset(&ifInfo, 0 , sizeof(struct rtl865x_interface_info));
		strcpy(ifInfo.ifname, RTL_PS_WLAN0_DEV_NAME);
		ifInfo.isWan=1;
		rtl865x_setLpIfInfo(&ifInfo);


		memset(&ifInfo, 0 , sizeof(struct rtl865x_interface_info));
		strcpy(ifInfo.ifname,RTL_DRV_LAN_NETIF_NAME);
		ifInfo.isWan=0;
		for(i=0;vlanconfig[i].vid!=0;i++)
		{
			if ((FALSE==vlanconfig[i].isWan) && (vlanconfig[i].if_type==IF_ETHER))
			{
				ifInfo.memPort|=vlanconfig[i].memPort;
				ifInfo.fid=vlanconfig[i].fid;
			}
		}
		rtl865x_setLpIfInfo(&ifInfo);
	}
	else 
	{
		memset(&ifInfo, 0 , sizeof(struct rtl865x_interface_info));
		ifInfo.isWan=1;
		rtl865x_setLpIfInfo(&ifInfo);

		memset(&ifInfo, 0 , sizeof(struct rtl865x_interface_info));
		strcpy(ifInfo.ifname, RTL_DRV_LAN_NETIF_NAME);
		ifInfo.isWan=0;
		for(i=0;vlanconfig[i].vid!=0;i++)
		{
			if ((FALSE==vlanconfig[i].isWan) && (vlanconfig[i].if_type==IF_ETHER))
			{
				ifInfo.memPort|=vlanconfig[i].memPort;
				ifInfo.fid=vlanconfig[i].fid;
			}
		}
		rtl865x_setLpIfInfo(&ifInfo);
	}
#endif

#if defined (CONFIG_RTL_MULTI_LAN_DEV) ||defined(CONFIG_RTK_VLAN_SUPPORT)
	re865x_packVlanConfig(vlanconfig, packedVlanConfig);
#endif

	/*reinit hw tables*/
	rtl_reinit_hw_table();    

#if defined (CONFIG_RTL_MULTI_LAN_DEV) ||defined(CONFIG_RTK_VLAN_SUPPORT)	
	reinit_vlan_configure(packedVlanConfig);
#else
	reinit_vlan_configure(vlanconfig);
#endif


#if defined (CONFIG_RTL_IGMP_SNOOPING)
	re865x_reInitIgmpSetting(mode);
#if defined (CONFIG_RTL_MLD_SNOOPING)
#if defined(CONFIG_RTK_VLAN_SUPPORT)
	if((mldSnoopEnabled==TRUE)&& (rtk_vlan_support_enable==0))
#else
	if(mldSnoopEnabled==TRUE)
#endif
	{
#if defined (CONFIG_RTL_HARDWARE_NAT)
#if defined (CONFIG_RTL_MULTI_LAN_DEV)
		rtl865x_addAclForMldSnooping(packedVlanConfig);
#else
		rtl865x_addAclForMldSnooping(vlanconfig);
#endif
#endif
	}
#endif
#endif

#if defined (CONFIG_RTL_IVL_SUPPORT)
	if(mode==GATEWAY_MODE)
	{
		WRITE_MEM32( FFCR, READ_MEM32( FFCR ) & ~EN_UNUNICAST_TOCPU );
	}
	else	if((mode==BRIDGE_MODE) ||(mode==WISP_MODE))
	{
		WRITE_MEM32( FFCR, READ_MEM32( FFCR ) | EN_UNUNICAST_TOCPU );
	}
	else
	{
		WRITE_MEM32( FFCR, READ_MEM32( FFCR ) & ~EN_UNUNICAST_TOCPU );
	}
#endif

	/*update current operation mode*/
	rtl865x_curOpMode=mode;
	rtl865x_curOpMode_allLan = allLan;

	//setAsicOperationLayer
#if defined(CONFIG_RTL_HARDWARE_NAT)
	switch(mode)
	{
		case GATEWAY_MODE:
			rtl8651_setAsicOperationLayer(4);
			break;
		case BRIDGE_MODE:
		case WISP_MODE:
			rtl8651_setAsicOperationLayer(3);
			break;
		default:
			rtl8651_setAsicOperationLayer(2);		
	}
#endif

#if 0
#if defined(CONFIG_RTL_LAYERED_DRIVER_L3) && !defined(CONFIG_RTL_MULTI_ETH_WAN)
	//always init the default route...
	if(rtl8651_getAsicOperationLayer() >2)
	{
		rtl865x_addRoute(0,0,0,RTL_DRV_WAN0_NETIF_NAME,0);
	}
#endif
#endif
	//checksum control register
	switch(mode)
	{
		case GATEWAY_MODE:
			WRITE_MEM32(CSCR,READ_MEM32(CSCR)&~ALLOW_L2_CHKSUM_ERR);
			WRITE_MEM32(CSCR,READ_MEM32(CSCR)&~ALLOW_L3_CHKSUM_ERR);
			WRITE_MEM32(CSCR,READ_MEM32(CSCR)&~ALLOW_L4_CHKSUM_ERR);
			break;
		case BRIDGE_MODE:
		case WISP_MODE:
			WRITE_MEM32(CSCR,READ_MEM32(CSCR)&~ALLOW_L2_CHKSUM_ERR);
			WRITE_MEM32(CSCR,READ_MEM32(CSCR)|ALLOW_L3_CHKSUM_ERR);
			WRITE_MEM32(CSCR,READ_MEM32(CSCR)|ALLOW_L4_CHKSUM_ERR);			
			break;			
		default:
			WRITE_MEM32(CSCR,READ_MEM32(CSCR)&~ALLOW_L2_CHKSUM_ERR);
			WRITE_MEM32(CSCR,READ_MEM32(CSCR)&~ALLOW_L3_CHKSUM_ERR);
			WRITE_MEM32(CSCR,READ_MEM32(CSCR)&~ALLOW_L4_CHKSUM_ERR);
	}

	return SUCCESS;
}

static int32 reinit_vlan_configure(struct rtl865x_vlanConfig new_vlanconfig[])
{
//#ifndef CONFIG_RTL_MULTI_ETH_WAN
	
	int32 i;
	//uint32 valid_port_mask = 0;
//#else
	//int32 i, j=0;
//#endif
	struct rtl865x_vlanConfig *pvlanconfig = NULL;
	int32 totalVlans = 0;	
	pvlanconfig = new_vlanconfig;


	/*get input vlan config entry number*/
	for(i=0; pvlanconfig[i].ifname[0] != '\0' ; i++)
	{
		if(pvlanconfig[i].vid != 0)
			totalVlans++;
	}	
	
	//because the new_vlanconfig should be packedVlanConfig
	totalVlans = totalVlans > NETIF_SW_NUMBER? NETIF_SW_NUMBER:totalVlans;



	for(i=0; i<totalVlans; i++)
	{
		rtl865x_netif_t netif;
		
		if(pvlanconfig[i].vid == 0)
			continue;

#if 0
//#ifndef CONFIG_RTL_MULTI_ETH_WAN
		if(pvlanconfig[i].isWan == 1)
		{
			/*wan*/
			valid_port_mask = RTL_WANPORT_MASK;
		}
		else
			valid_port_mask = RTL_LANPORT_MASK | 0x100;
#endif
		/*add vlan*/
		if(pvlanconfig[i].if_type==IF_ETHER)
		{
			rtl865x_addVlan(pvlanconfig[i].vid);
			
#ifdef CONFIG_RTL_MULTI_ETH_WAN
			rtl865x_addVlanPortMember(pvlanconfig[i].vid,pvlanconfig[i].memPort /*& valid_port_mask*/, pvlanconfig[i].untagSet);
#else
			rtl865x_addVlanPortMember(pvlanconfig[i].vid,pvlanconfig[i].memPort /*& valid_port_mask*/);
#endif
			//printk("===> vid %d fid %d\n", pvlanconfig[i].vid, pvlanconfig[i].fid);
			rtl865x_setVlanFilterDatabase(pvlanconfig[i].vid,pvlanconfig[i].fid);
		}

		/*add network interface*/
		memset(&netif, 0, sizeof(rtl865x_netif_t));
		memcpy(netif.name,pvlanconfig[i].ifname,MAX_IFNAMESIZE);
		memcpy(netif.macAddr.octet,pvlanconfig[i].mac.octet,ETHER_ADDR_LEN);
		netif.mtu = pvlanconfig[i].mtu;
		netif.if_type = pvlanconfig[i].if_type;
		netif.vid = pvlanconfig[i].vid;
		netif.is_wan = pvlanconfig[i].isWan;
		netif.is_slave = pvlanconfig[i].is_slave;
		//#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		netif.enableRoute=1;
		//#endif
		//printk("%s add netif %s\n", __func__, netif.name);
		rtl865x_addNetif(&netif);
		if(netif.is_slave == 1)
#if defined(CONFIG_RTL_PUBLIC_SSID)
			rtl865x_attachMasterNetif(netif.name,RTL_GW_WAN_DEVICE_NAME);
#else
			;//rtl865x_attachMasterNetif(netif.name,RTL_DRV_WAN0_NETIF_NAME);
#endif
#ifdef CONFIG_RTL_MULTI_ETH_WAN
	//#if defined(CONFIG_PROC_FS) && defined(CONFIG_NET_SCHED) && defined(CONFIG_RTL_LAYERED_DRIVER)
	#if 0
		if (0==netif.is_slave)
			memcpy(&netIfName[j++][0], vlanconfig[i].ifname, sizeof(vlanconfig[i].ifname));
#endif
#endif
	}

//#ifdef CONFIG_RTL_MULTI_ETH_WAN
#if 0
	memcpy(netIfNameArray, netIfName, NETIF_NUMBER*IFNAMSIZ);
#endif

#if defined(CONFIG_RTK_VLAN_SUPPORT)
		if(rtk_vlan_support_enable == 1)
			rtl865x_enable_acl(0);
		else 
			rtl865x_enable_acl(1);
#endif

	rtl865x_setpvid(pvlanconfig);
#if 0 /* old version */
#ifndef CONFIG_RTL_MULTI_ETH_WAN
	for(i=0; i<RTL8651_PORT_NUMBER + 3; i++) 
	{ 
		/* Set each port's PVID */
		for(j=0,pvid=0; pvlanconfig[j].vid != 0; j++)
		{
			if ( (1<<i) & pvlanconfig[j].memPort ) 
			{
				pvid = pvlanconfig[j].vid;
				break;
			}
		}
		
		if (pvid!=0)	
		{
			CONFIG_CHECK(rtl8651_setAsicPvid(i,pvid));
			rtl865x_setPortToNetif(pvlanconfig[j].ifname, i);
		}
	}
#else
	for(i=0; i<RTL8651_PORT_NUMBER + 3; i++)
	{
		/* Set each port's PVID */
		for(j=0; pvlanconfig[j].vid != 0; j++)
		{
			if ( pvlanconfig[j].vid == pvid_per_port[i]) 
			{
				break;
			}
		}

		if (pvlanconfig[j].vid) {
			CONFIG_CHECK(rtl8651_setAsicPvid(i,pvid_per_port[i]));
			rtl865x_setPortToNetif(pvlanconfig[j].ifname, i);
		}
	}
#endif
#endif /* old version */

#ifdef CONFIG_RTL_STP
	re865x_stp_mapping_reinit();
#endif

#if defined (CONFIG_RTL_IGMP_SNOOPING) && defined (CONFIG_RTL_HARDWARE_MULTICAST)	
		INIT_CHECK(rtl8651_setAsicOperationLayer(3));
#endif

	return SUCCESS;
}
#endif

#if defined(CONFIG_RTK_VLAN_SUPPORT)
static int32 rtk_vlan_support_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len;
	len = sprintf(page, "%s %d\n", "rtk_vlan_support_enable:",rtk_vlan_support_enable);


	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count)
		len = count;
	if (len<0)
	  	len = 0;

	return len;
}

static int32 rtk_vlan_support_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char 		tmpbuf[32];	
	int i=0;
	int j=0;
	struct net_device *dev;
	struct dev_priv	  *dp;
	
	if (buff && !copy_from_user(tmpbuf, buff, len))
	{
		tmpbuf[len] = '\0';
		#if defined (CONFIG_RTL_IGMP_SNOOPING) && defined (CONFIG_RTL_MLD_SNOOPING)
		if(mldSnoopEnabled)
		{
			rtl865x_removeAclForMldSnooping(vlanconfig);
		}
		#endif
		if(tmpbuf[0] == '0')
		{
			rtk_vlan_support_enable = 0;
			
			rtl_config_rtkVlan_vlanconfig(rtl865x_curOpMode);
			re865x_packVlanConfig(vlanconfig, packedVlanConfig);	
			rtl_reinit_hw_table();
			reinit_vlan_configure(packedVlanConfig);

			//unknow vlan drop
			REG32(SWTCR0) &= ~(1 << 15);
			
#if defined(CONFIG_RTL_LAYERED_DRIVER_ACL)
			rtl865x_enable_acl(1); //enable acl feature
#endif
		}	
		else if(tmpbuf[0] == '1')
		{
			rtk_vlan_support_enable = 1;				
			
			rtl_config_rtkVlan_vlanconfig(rtl865x_curOpMode);			
			re865x_packVlanConfig(vlanconfig, packedVlanConfig);			
			rtl_reinit_hw_table();
			reinit_vlan_configure(packedVlanConfig);			

			//unknow vid to cpu
			REG32(SWTCR0) |= 1 << 15;
#if defined(CONFIG_RTL_LAYERED_DRIVER_ACL)
			rtl865x_enable_acl(0); //disable acl feature
#endif
		}
#if defined(CONFIG_RTK_VLAN_FOR_CABLE_MODEM)
		else if(tmpbuf[0] == '2')
		{
			rtk_vlan_support_enable = 2;
			//unknow vid to cpu
			REG32(SWTCR0) |= 1 << 15;
		}
#endif
		else
		{
			printk("current support: 0/1/2\n");
			return len;
		}

		/*update dev port number*/
		for(i=0; vlanconfig[i].vid != 0; i++)
		{
			if (IF_ETHER!=vlanconfig[i].if_type)
			{
				continue;
			}
			
			dev=_rtl86xx_dev.dev[i];
			dp = dev->priv;
			dp->portnum  = 0;
			for(j=0;j<RTL8651_AGGREGATOR_NUMBER;j++)
			{
				if(dp->portmask & (1<<j))
					dp->portnum++;
			}	
			
		}
		
	#if defined (CONFIG_RTL_IGMP_SNOOPING)
		re865x_reInitIgmpSetting(rtl865x_curOpMode);
		#if defined (CONFIG_RTL_MLD_SNOOPING)
		if(mldSnoopEnabled && (rtk_vlan_support_enable==0))
		{			
			re865x_packVlanConfig(vlanconfig, packedVlanConfig);
			#ifdef CONFIG_RTL_HARDWARE_NAT
			rtl865x_addAclForMldSnooping(packedVlanConfig);
			#endif
		}
		#endif
	#endif
#ifndef CONFIG_RTL_MULTI_ETH_WAN
		//always init the default route...
		if(rtl8651_getAsicOperationLayer() >2)
		{
			rtl865x_addRoute(0,0,0,RTL_DRV_WAN0_NETIF_NAME,0);
		}
#endif
	}
	return len;
}

static int read_proc_vlan(char *page, char **start, off_t off,
        int count, int *eof, void *data)
{

    struct net_device *dev = (struct net_device *)data;
    struct dev_priv *cp;
    int len;

	cp = dev->priv;
    len = sprintf(page, "gvlan=%d, lan=%d, vlan=%d, tag=%d, vid=%d, priority=%d, cfi=%d\n", 
		cp->vlan_setting.global_vlan, cp->vlan_setting.is_lan, cp->vlan_setting.vlan, cp->vlan_setting.tag, 
		cp->vlan_setting.id, cp->vlan_setting.pri, cp->vlan_setting.cfi);
			
    if (len <= off+count) 
        *eof = 1;      
    *start = page + off;      
    len -= off;      
    if (len > count) 
        len = count;      
    if (len < 0) 
        len = 0;      
    return len; 
}

static int write_proc_vlan(struct file *file, const char *buffer,
              unsigned long count, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct dev_priv *cp;
	#define	VLAN_MAX_INPUT_LEN	128
	char *tmp;
	
	tmp = kmalloc(VLAN_MAX_INPUT_LEN, GFP_KERNEL);
	if (count < 2 || tmp==NULL)
	{
		if(tmp)
			kfree(tmp);
		return -EFAULT;
	}

	cp = dev->priv;
	if(rtk_vlan_support_enable == 0)
		goto out;
	if (buffer && !copy_from_user(tmp, buffer, VLAN_MAX_INPUT_LEN))
	{
		int num = sscanf(tmp, "%d %d %d %d %d %d %d", 
			&cp->vlan_setting.global_vlan, &cp->vlan_setting.is_lan, 
			&cp->vlan_setting.vlan, &cp->vlan_setting.tag, 
			&cp->vlan_setting.id, &cp->vlan_setting.pri, 
			&cp->vlan_setting.cfi);
		
		if (num !=  7) {
			printk("invalid vlan parameter!\n");
			goto out;
		}
		#if 0
		printk("===%s(%d), cp->name(%s),global_vlan(%d),is_lan(%d),vlan(%d),tag(%d),id(%d),pri(%d),cfi(%d)",__FUNCTION__,__LINE__,
			cp->dev->name,cp->vlan_setting.global_vlan,cp->vlan_setting.is_lan,cp->vlan_setting.vlan,cp->vlan_setting.tag,
			cp->vlan_setting.id,cp->vlan_setting.pri,cp->vlan_setting.cfi);
		printk("-------------%s(%d),cp->portmask(%d)\n",__FUNCTION__,__LINE__,cp->portmask);
		#endif			
	}
out:
	if(tmp)
		kfree(tmp);
	
	return count;	
}
#endif // CONFIG_RTK_VLAN_SUPPORT

#if (defined(CONFIG_RTL_CUSTOM_PASSTHRU) && !defined(CONFIG_RTL8196_RTL8366)) 

static unsigned long atoi_dec(char *s)
{
	unsigned long k = 0;

	k = 0;
	while (*s != '\0' && *s >= '0' && *s <= '9') {
		k = 10 * k + (*s - '0');
		s++;
	}
	return k;
}

static int custom_Passthru_read_proc(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{
	int len;	
	len = sprintf(page, "%s\n", passThru_flag);
	if (len <= off+count) 
		*eof = 1;

	*start = page + off;
	len -= off;

	if (len>count) 
		len = count;

	if (len<0) len = 0;

	return len;
}

static int custom_createPseudoDevForPassThru(void)
{
		struct net_device *dev, *wanDev;
		struct dev_priv *dp;
		int rc, i;
		unsigned long		flags;

		wanDev = NULL;
		/*	find wan device first	*/
		for(i=0;i<ETH_INTF_NUM;i++)
		{
			dp = _rtl86xx_dev.dev[i]->priv;
			if (rtl_isWanDev(dp)==TRUE)
			{
				wanDev = _rtl86xx_dev.dev[i];
				break;
			}
		}

		/*	can't find any wan device, just return	*/
		if (wanDev==NULL)
			return -1;
		
		dev = alloc_etherdev(sizeof(struct dev_priv));
		if (!dev){
			rtlglue_printf("failed to allocate passthru pseudo dev.\n");
			return -1;
		}
		strcpy(dev->name, "peth%d");
		/*	default set lan side mac		*/
		memcpy((char*)dev->dev_addr,(char*)(&(vlanconfig[0].mac)),ETHER_ADDR_LEN);
		dp = dev->priv;
		memset(dp,0,sizeof(*dp));
		dp->dev = wanDev;
#if defined(CONFIG_COMPAT_NET_DEV_OPS)
		dev->open = re865x_pseudo_open;
		dev->stop = re865x_pseudo_close;
		dev->set_multicast_list = NULL;
		dev->hard_start_xmit = re865x_start_xmit;
		dev->get_stats = re865x_get_stats;
		dev->do_ioctl = re865x_ioctl;
#ifdef CP_VLAN_TAG_USED
		dev->features |= NETIF_F_HW_VLAN_TX | NETIF_F_HW_VLAN_RX;
		dev->vlan_rx_register = cp_vlan_rx_register;
		dev->vlan_rx_kill_vid = cp_vlan_rx_kill_vid;
#endif

#else
		dev->netdev_ops = &rtl819x_pseudodev_ops;
#endif

#ifdef CP_VLAN_TAG_USED
		dev->features |= NETIF_F_HW_VLAN_TX | NETIF_F_HW_VLAN_RX;		
#endif
		dev->watchdog_timeo = TX_TIMEOUT;
		
		dev->irq = 0;
		rc = register_netdev(dev);
		if(!rc){
			spin_lock_irqsave (&_rtl86xx_dev.lock, flags);
			_rtl86xx_dev.pdev = dev;
			spin_unlock_irqrestore (&_rtl86xx_dev.lock, flags);
			rtlglue_printf("[%s] added, mapping to [%s]...\n", dev->name, dp->dev->name);
		}else
			rtlglue_printf("Failed to allocate [%s]\n", dev->name);

		return 0;
}

static int custom_Passthru_write_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	int flag,i;

#if	!defined(CONFIG_RTL_LAYERED_DRIVER)
	struct rtl865x_vlanConfig  passThruVlanConf = {"passThru",0,IF_ETHER,PASSTHRU_VLAN_ID,0,
		rtl865x_lanPortMask|rtl865x_wanPortMask,
		rtl865x_lanPortMask|rtl865x_wanPortMask,
		1500,{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x90 } } };
#endif

	if (buffer && !copy_from_user(&passThru_flag, buffer, count))
	{			
		flag=(int)atoi_dec(passThru_flag);		

		if(flag ^ oldStatus)
		{
			//IPv6 PassThru
			if((flag & IP6_PASSTHRU_MASK) ^ (oldStatus & IP6_PASSTHRU_MASK)) 
			{			
				if(flag & IP6_PASSTHRU_MASK)
				{//add		
					for(i=0; i<RTL865XC_PORT_NUMBER; i++)
					{
						rtl8651_setProtocolBasedVLAN(IP6_PASSTHRU_RULEID, i, TRUE, PASSTHRU_VLAN_ID);
					}				
				}
				else
				{//delete
					for(i=0; i<RTL865XC_PORT_NUMBER; i++)
					{
						rtl8651_setProtocolBasedVLAN(IP6_PASSTHRU_RULEID, i, FALSE, PASSTHRU_VLAN_ID);
					}
				}
			}

			//PPPoE PassThru
			if((flag & PPPOE_PASSTHRU_MASK) ^ (oldStatus & PPPOE_PASSTHRU_MASK)) 
			{			
				if(flag & PPPOE_PASSTHRU_MASK)
				{//add				
					for(i=0; i<RTL865XC_PORT_NUMBER; i++)
					{
						rtl8651_setProtocolBasedVLAN(RTL8651_PBV_RULE_PPPOE_CONTROL, i, TRUE, PASSTHRU_VLAN_ID);
						rtl8651_setProtocolBasedVLAN(RTL8651_PBV_RULE_PPPOE_SESSION, i, TRUE, PASSTHRU_VLAN_ID);
					}				
				}
				else
				{//delete
					for(i=0; i<RTL865XC_PORT_NUMBER; i++)
					{
						rtl8651_setProtocolBasedVLAN(RTL8651_PBV_RULE_PPPOE_CONTROL, i, FALSE, PASSTHRU_VLAN_ID);
						rtl8651_setProtocolBasedVLAN(RTL8651_PBV_RULE_PPPOE_SESSION, i, FALSE, PASSTHRU_VLAN_ID);
					}
				}
			}
			
		}

		//passthru vlan
		if(flag==0)
		{
			//To del passthru vlan
			//Do nothing here because change_op_mode reinit vlan already
		}
		else
            {
            		//To add passthru vlan
			rtl865x_addVlan(PASSTHRU_VLAN_ID);

			#if defined(CONFIG_POCKET_ROUTER_SUPPORT)
			rtl865x_addVlanPortMember(PASSTHRU_VLAN_ID, rtl865x_lanPortMask|rtl865x_wanPortMask|0x100, rtl865x_lanPortMask|rtl865x_wanPortMask|0x100);
			#else
			rtl865x_addVlanPortMember(PASSTHRU_VLAN_ID, rtl865x_lanPortMask|rtl865x_wanPortMask, rtl865x_lanPortMask|rtl865x_wanPortMask);
			#endif

            }

		oldStatus=flag;
		return count;
	}
	return -EFAULT;
}

int32 rtl8651_customPassthru_init(void)
{
	oldStatus=0;
	res = create_proc_entry("custom_Passthru", 0, NULL);	
	if(res)
	{
		res->read_proc = custom_Passthru_read_proc;
		res->write_proc = custom_Passthru_write_proc;
	}
	rtl8651_defineProtocolBasedVLAN( IP6_PASSTHRU_RULEID, 0x0, 0x86DD );
	custom_createPseudoDevForPassThru();
	return 0;
}

void __exit rtl8651_customPassthru_exit(void)
{
	if (res) {
		remove_proc_entry("custom_Passthru", res);				
		res = NULL;
	}
}
#define	MULTICAST_STORM_CONTROL	1
#define	BROADCAST_STORM_CONTROL	2

static struct proc_dir_entry *stormCtrlProc=NULL;
static int rtl865x_stormCtrlReadProc(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{
	int len=0;	
	if (len <= off+count) 
		*eof = 1;

	*start = page + off;
	len -= off;

	if (len>count) 
		len = count;

	if (len<0) len = 0;

	return len;
}

static int rtl865x_stormCtrlWriteProc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	uint32 tmpBuf[32];
	uint32 stormCtrlType=MULTICAST_STORM_CONTROL;
	uint32 enableStormCtrl=FALSE;
	uint32 percentage=0;
	uint32 uintVal;

	if (buffer && !copy_from_user(tmpBuf, buffer, count))
	{
		tmpBuf[count-1]=0;
		uintVal=simple_strtol((const char *)tmpBuf, NULL, 0);
		//printk("%s(%d),tmpBuf=%s,count=%d,uintVal=%d\n",__FUNCTION__,__LINE__,
		//	tmpBuf,count,uintVal);//Added for test
		if(uintVal>100)
		{
			enableStormCtrl=FALSE;
			percentage=0;
		}
		else
		{
			enableStormCtrl=TRUE;
			percentage=uintVal;
		}
		rtl865x_setStormControl(stormCtrlType,enableStormCtrl,percentage);
		return count;
	}
	return -EFAULT;
}

int32 rtl8651_initStormCtrl(void)
{
	stormCtrlProc = create_proc_entry("StormCtrl", 0, NULL);	
	if(stormCtrlProc)
	{
		stormCtrlProc->read_proc = rtl865x_stormCtrlReadProc;
		stormCtrlProc->write_proc = rtl865x_stormCtrlWriteProc;
	}
	
	return 0;
}

void __exit rtl8651_exitStormCtrl(void)
{
	if (stormCtrlProc) {
		remove_proc_entry("StormCtrl", stormCtrlProc);				
		stormCtrlProc = NULL;
	}
}


#endif

#if defined(CONFIG_RTL_8198)
static int32 proc_phyTest_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	return 0;
}

static int32 proc_phyTest_write( struct file *filp, const char *buff,unsigned long len1, void *data )
{
	char 		tmpbuf[64];
	uint32	phyId=0, regId=0,iNo=0,iPort=0,len=0,regData=0;
	char		*strptr, *cmd_addr;
	char		*tokptr;
	int32	i=0,port=0;
	int32 	ret=FAILED;
	if (buff && !copy_from_user(tmpbuf, buff, len1)) {
		tmpbuf[len1-1] = '\0';
		strptr=tmpbuf;
		cmd_addr = strsep(&strptr," ");
		if (cmd_addr==NULL)
		{
			goto errout;
		}
	
		if (!memcmp(cmd_addr, "read", 4))
		{

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 0);
			
			ret=rtl8651_getAsicEthernetPHYReg(phyId, regId, &regData); 
			if(ret==SUCCESS)
			{
				printk("read phyId(%d), regId(%d),regData:0x%x\n", phyId, regId, regData);
			}
			else
			{
				printk("error input!\n");
//				goto errout;
			}
		}
		else if (!memcmp(cmd_addr, "test", 4))
		{
			printk("\r\n");
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			iNo=simple_strtol(tokptr, NULL, 0);
			if(iNo==1)//test mode 1
			{
				
				unsigned int default_val_t1[]={
				4,0x1f,0x0002,
				4,0x13,0xAA00,
				4,0x14,0xAA00,
				4,0x15,0xAA00,
				4,0x16,0xFA00,
				4,0x17,0xAF00
				};
				printk("PHY Test 1 Mode: No\n");
				for(i=0; i<5; i++)
				REG32(PCRP0+i*4) |= (EnForceMode);
				len=sizeof(default_val_t1)/sizeof(unsigned int);
				
				for(i=0;i<len;i=i+3) 
				{	                   
				port=default_val_t1[i];                                                    
				rtl8651_setAsicEthernetPHYReg(port, default_val_t1[i+1], default_val_t1[i+2]);
				}
				for(i=0; i<5; i++)
				{
					rtl8651_setAsicEthernetPHYReg(i, 0x1f, 0x0000);
					rtl8651_setAsicEthernetPHYReg(i, 0x09, 0x2E00);
				}		
				for(i=0; i<5; i++)
				REG32(PCRP0+i*4) &= ~(EnForceMode);
				printk("Please reset the target after leaving Test Mode\n");
				ret=SUCCESS;
			}
	#if 0
			else if(iNo==2)//test mode 2
			{
				
				unsigned int default_val_t2[]={
				1, 0x1f, 0x0002,
				1, 0x11, 0x5E00,
				1, 0x1f, 0x0000,            
				4, 0x1f, 0x0002,
				4, 0x11, 0x5E00,
				4, 0x1f, 0x0000

				};
				printk("PHY Test 2 Mode: No\n");	
				for(i=0; i<5; i++)
				REG32(PCRP0+i*4) |= (EnForceMode);
				len=sizeof(default_val_t2)/sizeof(unsigned int);
				
				for(i=0;i<len;i=i+3) 
				{
				port=default_val_t2[i];                                                    
				rtl8651_setAsicEthernetPHYReg(port, default_val_t2[i+1], default_val_t2[i+2]);
				}
				for(i=0; i<5; i++)
				{
					rtl8651_setAsicEthernetPHYReg(i, 0x1f, 0x0000);
					rtl8651_setAsicEthernetPHYReg(i, 0x09, 0x4E00);
				}		
				for(i=0; i<5; i++)
				REG32(PCRP0+i*4) &= ~(EnForceMode);
				printk("Please reset the target after leaving Test Mode\n");
				ret=SUCCESS;
			}
			else if(iNo==3)//test mode 2
			{
				unsigned int default_val_t3[]={
				1, 0x1f, 0x0002,
				1, 0x11, 0x4000,
				};
				for(i=0; i<5; i++)
				REG32(PCRP0+i*4) |= (EnForceMode);
				
				printk("PHY Test 3 Mode: No  Port{0~4} Channel{A,B,C,D}\n");
				tokptr = strsep(&strptr," ");
				if (tokptr==NULL)
				{
					goto errout;
				}
				iPort=simple_strtol(tokptr, NULL, 0);
				tokptr = strsep(&strptr," ");
				if (tokptr==NULL)
				{
					goto errout;
				}
				len=sizeof(default_val_t3)/sizeof(unsigned int);
				for(i=0;i<len;i=i+3) 
				{
					port=default_val_t3[i];                                                    
					rtl8651_setAsicEthernetPHYReg(port, default_val_t3[i+1], default_val_t3[i+2]);
				}
			
				switch(*tokptr)
				{
					
					case 'A':
						rtl8651_setAsicEthernetPHYReg(1, 0x10, 0x1100);
						rtl8651_setAsicEthernetPHYReg(1, 0x1f, 0x0000);
						printk("A channel\n");
						
					break;
					case 'B':
						rtl8651_setAsicEthernetPHYReg(1, 0x10, 0x1300);
						rtl8651_setAsicEthernetPHYReg(1, 0x1f, 0x0000);
						printk("B channel\n");
					break;
					case 'C':
						rtl8651_setAsicEthernetPHYReg(1, 0x10, 0x1500);
						rtl8651_setAsicEthernetPHYReg(1, 0x1f, 0x0000);
						printk("C channel\n");
					break;
					case 'D':
						rtl8651_setAsicEthernetPHYReg(1, 0x10, 0x1700);
						rtl8651_setAsicEthernetPHYReg(1, 0x1f, 0x0000);
						printk("D channel\n");
					break;
				}
				rtl8651_setAsicEthernetPHYReg(iPort, 0x1f, 0x0000);
				rtl8651_setAsicEthernetPHYReg(iPort, 0x09, 0x6e00);
				for(i=0; i<5; i++)
				REG32(PCRP0+i*4) &= ~(EnForceMode);
				printk("Please reset the target after leaving Test Mode\n");
				ret=SUCCESS;
			}
	#endif
			else if(iNo==4)//test mode 2
			{
				unsigned int default_val_t4[]={
				0, 0x1f, 0x0002,
				0, 0x07, 0x3678,
				0, 0x1f, 0x0000,
				0, 0x09, 0x8e00
				};
				
				for(i=0; i<5; i++)
				REG32(PCRP0+i*4) |= (EnForceMode);
				
				len=sizeof(default_val_t4)/sizeof(unsigned int);
				
				
				
				tokptr = strsep(&strptr," ");
				if (tokptr==NULL)
				{
					goto errout;
				}
				iPort=simple_strtol(tokptr, NULL, 0);
                                printk("PHY Test 4 Mode: No:%d  Port:%d \n",iNo,iPort);
				for(i=0;i<len;i=i+3) 
				{                                                 
					rtl8651_setAsicEthernetPHYReg(iPort, default_val_t4[i+1], default_val_t4[i+2]);
				}
								
				for(i=0; i<5; i++)
				REG32(PCRP0+i*4) &= ~(EnForceMode);
				printk("Please reset the target after leaving Test Mode\n");
				ret=SUCCESS;
			}
			
			
#if 0
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regData=simple_strtol(tokptr, NULL, 0);
		
			ret=rtl8651_setAsicEthernetPHYReg(phyId, regId, regData);
#endif
			if(ret==SUCCESS)
			{
				//printk("Write phyId(%d), regId(%d), regData:0x%x\n", phyId, regId, regData);
			}
			else
			{
				printk("error input!\n");
//				goto errout;
			}
		}
		else
		{
			goto errout;
		}

		return len1;

errout:
		printk("\nTest  format:	\"Test 1~4\"\n");
		printk("PHY Test 1 Mode: No\n");
		//printk("PHY Test 2 Mode: No\n");
		//printk("PHY Test 3 Mode: No  Port{0~4} Channel{A,B,C,D}\n");
		printk("PHY Test 4 Mode: No  Port{0~4} \n");

		return len1;
	}

	return -EFAULT;
}

int32 phyRegTest_init(void)
{
	phyTest_entry= create_proc_entry("phyRegTest", 0, NULL);
	if(phyTest_entry != NULL)
	{
		phyTest_entry->read_proc = proc_phyTest_read;
		phyTest_entry->write_proc = proc_phyTest_write;
	}
	
	return 0;
}

void __exit phyRegTest_exit(void)
{
	if (phyTest_entry)
	{
		remove_proc_entry("phyTest_entry", phyTest_entry);
		phyTest_entry = NULL;
	}
}
#endif

#ifdef CONFIG_RTL_HARDWARE_NAT
void rtl8676_addAclForMldSnooping(void)
{
	rtl865x_addAclForMldSnooping(vlanconfig);
}

void rtl8676_removeAclForMldSnooping(void)
{
	rtl865x_removeAclForMldSnooping(vlanconfig);
}
#endif

#if 0// defined(CONFIG_RTL_MLD_SNOOPING)

static struct proc_dir_entry *mldSnoopingProc=NULL;


static int rtl865x_mldSnoopingReadProc(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{
	int len;	
	len = sprintf(page, "%s\n", (mldSnoopEnabled==TRUE)?"enable":"disable");
	if (len <= off+count) 
		*eof = 1;

	*start = page + off;
	len -= off;

	if (len>count) 
		len = count;

	if (len<0) len = 0;

	return len;
}

static int rtl865x_mldSnoopingWriteProc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	uint32 tmpBuf[32];
	uint32 uintVal;

	if (buffer && !copy_from_user(tmpBuf, buffer, count))
	{
		tmpBuf[count-1]=0;
		uintVal=simple_strtol((const char *)tmpBuf, NULL, 0);
		if(uintVal!=0)
		{
			if(mldSnoopEnabled==FALSE)
			{
				#ifdef CONFIG_RTL_LAYERED_DRIVER_ACL
				rtl865x_removeAclForMldSnooping(vlanconfig);
				rtl865x_addAclForMldSnooping(vlanconfig);
				#endif
				mldSnoopEnabled=TRUE;
			}
		}
		else
		{
			if(mldSnoopEnabled==TRUE)
			{
				#ifdef CONFIG_RTL_LAYERED_DRIVER_ACL
				rtl865x_removeAclForMldSnooping(vlanconfig);
				#endif
				mldSnoopEnabled=FALSE;
				rtl_flushAllIgmpRecord();	// Mason Yu.
			}
		}
		
		return count;
	}
	return -EFAULT;
}

int32 rtl8651_initMldSnooping(void)
{
	mldSnoopingProc = create_proc_entry("br_mldsnoop", 0, NULL);	
	if(mldSnoopingProc)
	{
		mldSnoopingProc->read_proc = rtl865x_mldSnoopingReadProc;
		mldSnoopingProc->write_proc = rtl865x_mldSnoopingWriteProc;
	}
	 mldSnoopEnabled=FALSE;
	return 0;
}

void __exit rtl8651_exitMldSnoopingCtrl(void)
{
	if (mldSnoopingProc) {
		remove_proc_entry("br_mldsnoop", mldSnoopingProc);				
		mldSnoopingProc = NULL;
	}
}

#endif

#if defined (CONFIG_RTL_PHY_POWER_CTRL)
static struct proc_dir_entry *phyPowerCtrlProc=NULL;
#define PHY_POWER_OFF 0
#define PHY_POWER_ON 1
static int rtl865x_setPhyPowerOff(unsigned int port)
{
	unsigned int offset = port * 4;
	unsigned int pcr = 0;
	unsigned int regData;	
	if(port >=RTL8651_PHY_NUMBER)
	{
		return -1;
	}
	
	/*must set mac force link down first*/	
	pcr=READ_MEM32( PCRP0 + offset );
	pcr |= EnForceMode;
	pcr &= ~ForceLink;	
	WRITE_MEM32( PCRP0 + offset, pcr );
	TOGGLE_BIT_IN_REG_TWICE(PCRP0 + offset,EnForceMode);
	
	rtl8651_getAsicEthernetPHYReg(port, 0, &regData);
	regData=regData |(1<<11);
	rtl8651_setAsicEthernetPHYReg(port, 0, regData);

	return 0;
	
}

static int rtl865x_setPhyPowerOn(unsigned int port)
{	
	unsigned int  offset = port * 4;
	unsigned int pcr = 0;

	unsigned int regData;	
	
	if(port >=RTL8651_PHY_NUMBER)
	{
		return -1;
	}
	
	pcr=READ_MEM32( PCRP0 + offset );
	pcr |= EnForceMode;

	WRITE_MEM32( PCRP0 + offset, pcr );
	TOGGLE_BIT_IN_REG_TWICE(PCRP0 + offset,EnForceMode);
	
	rtl8651_getAsicEthernetPHYReg(port, 0, &regData);
	regData=regData &(~(1<<11));
	rtl8651_setAsicEthernetPHYReg(port, 0, regData);

	pcr &=~EnForceMode;
	WRITE_MEM32( PCRP0 + offset, pcr);
	TOGGLE_BIT_IN_REG_TWICE(PCRP0 + offset,EnForceMode);
	return 0;
	
}


static int rtl865x_phyPowerCtrlReadProc(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{
	int len=0;	
	int port;
	unsigned int regData;	
	for(port=0;port<RTL8651_PHY_NUMBER;port++)
	{
		rtl8651_getAsicEthernetPHYReg(port, 0, &regData);
		if(regData & (1<<11))
		{
			len += sprintf(page+len, "port[%d] phy is power off\n",port );
		}
		else
		{
			len += sprintf(page+len, "port[%d] phy is power on\n",port );
		}
	}
	
	if (len <= off+count) 
		*eof = 1;

	*start = page + off;
	len -= off;

	if (len>count) 
		len = count;

	if (len<0) len = 0;

	return len;
}

static int rtl865x_phyPowerCtrlWriteProc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	char tmpBuf[256];
	char		*strptr;
	char		*tokptr; 
	unsigned int  portMask;
	unsigned int action;
	int i;
	if (buffer && !copy_from_user(tmpBuf, buffer, count))
	{
		tmpBuf[count-1]=0;
		strptr=tmpBuf;
		
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errOut;
		}
		portMask=simple_strtol(tokptr, NULL, 0);
		
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errOut;
		}
		action=simple_strtol(tokptr, NULL, 0);
		
		for(i=0;i<RTL8651_PHY_NUMBER;i++)
		{
			if((1<<i) &portMask)
			{
				if(action==PHY_POWER_OFF)
				{
					rtl865x_setPhyPowerOff(i);
				}
				else if (action==PHY_POWER_ON)
				{
					rtl865x_setPhyPowerOn(i);
				}
			}
			
		}
		
		return count;
	}

errOut:	
	return -EFAULT;
}


static int32 rtl865x_initPhyPowerCtrl(void)
{
	phyPowerCtrlProc = create_proc_entry("phyPower", 0, NULL);	
	if(phyPowerCtrlProc)
	{
		phyPowerCtrlProc->read_proc = rtl865x_phyPowerCtrlReadProc;
		phyPowerCtrlProc->write_proc = rtl865x_phyPowerCtrlWriteProc;
	}

	return 0;
}

void __exit rtl865x_exitPhyPowerCtrl(void)
{
	if (phyPowerCtrlProc) {
		remove_proc_entry("phyPower", phyPowerCtrlProc);				
		phyPowerCtrlProc = NULL;
	}
}

#endif


#if defined(CONFIG_RTL_LINKSTATE)

static struct proc_dir_entry *portStateProc=NULL;
static unsigned int linkUpTime[RTL8651_PHY_NUMBER] = {0,0,0,0,0};

static void init_linkup_time(void)
{
	init_timer(&s_timer);
	s_timer.function = &linkup_time_handle;
	s_timer.expires = jiffies + HZ;
	add_timer(&s_timer);
}

static void linkup_time_handle(unsigned long arg)
{
	int port;
	uint32	regData;
	uint32	data0;
	mod_timer(&s_timer,jiffies +HZ);
	for(port=PHY0;port<PHY5;port++)
	{
		regData = READ_MEM32(PSRP0+((port)<<2));
		data0 = regData & PortStatusLinkUp;
		if (data0)
		{
			linkUpTime[port]++;
		}		
		else
		{
			linkUpTime[port]=0;
		}
	}
}

static int32 port_state_read_proc( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int		len;
	uint32	regData;
	uint32	data0;
	int		port;

	len = sprintf(page, "Port Link State:\n");

	for(port=PHY0;port<PHY5;port++)
	{
		regData = READ_MEM32(PSRP0+((port)<<2));

		//if (port==CPU)
			//len += sprintf(page+len, "CPUPort ");
		//else
		len += sprintf(page+len, "Port[%d]:", port);
		data0 = regData & PortStatusLinkUp;
		
		if (data0)
		{
			len += sprintf(page+len, "LinkUp|");
		}
		else
		{
			len += sprintf(page+len, "LinkDown\n");
			continue;
		}
		//data0 = regData & PortStatusNWayEnable;
		//len += sprintf(page+len, "NWay Mode %s\n", data0?"Enabled":"Disabled");
		data0 = regData & PortStatusRXPAUSE;
		len += sprintf(page+len, "RXPause:%s|", data0?"Enable":"Disable");
		data0 = regData & PortStatusTXPAUSE;
		len += sprintf(page+len, "TXPause:%s|", data0?"Enable":"Disable");
		data0 = regData & PortStatusDuplex;
		len += sprintf(page+len, "Duplex:%s|", data0?"Enable":"Disable");
		data0 = (regData&PortStatusLinkSpeed_MASK)>>PortStatusLinkSpeed_OFFSET;
		len += sprintf(page+len, "Speed:%s|", data0==PortStatusLinkSpeed100M?"100M":
			(data0==PortStatusLinkSpeed1000M?"1G":
				(data0==PortStatusLinkSpeed10M?"10M":"Unkown")));
		len += sprintf(page+len, "UpTime:%ds\n", linkUpTime[port]);
	}
	
	return len;
}

static int32 port_state_write_proc( struct file *filp, const char *buff,unsigned long len, void *data )
{
	return len;
}

static int32 initPortStateCtrl(void)
{
	portStateProc = create_proc_entry("portState", 0, NULL);	
	if(portStateProc)
	{
		portStateProc->read_proc = port_state_read_proc;
		portStateProc->write_proc = port_state_write_proc;
	}
	memset(linkUpTime,0,sizeof(linkUpTime));
	init_linkup_time();
	
	return 0;
}

static void  exitPortStateCtrl(void)
{
	if (portStateProc) {
		remove_proc_entry("portState", portStateProc);				
		portStateProc = NULL;
	}
	del_timer(&s_timer);
}	
#endif	

