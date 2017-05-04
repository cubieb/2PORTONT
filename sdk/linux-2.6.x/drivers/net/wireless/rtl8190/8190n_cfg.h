/*
 *		Headler file defines some configure options and basic types
 *
 *		$Id: 8190n_cfg.h,v 1.1.1.1 2010/05/05 09:00:40 jiunming Exp $
 */

#ifndef _8190N_CFG_H_
#define _8190N_CFG_H_

#ifdef __MIPSEB__
	#define _BIG_ENDIAN_
#else
	#define _LITTLE_ENDIAN_
#endif

#ifdef __KERNEL__
#include <linux/version.h>

#if LINUX_VERSION_CODE == 132638
	#define LINUX_2_6_30_
	#define LINUX_2_6_22_
	#define __LINUX_2_6__
#endif
#if LINUX_VERSION_CODE >= 0x020616 // linux 2.6.22
	#define LINUX_2_6_22_
	#define __LINUX_2_6__
#endif

#if LINUX_VERSION_CODE > 0x020600
	#define __LINUX_2_6__
#endif

#ifdef LINUX_2_6_22_
#include <linux/autoconf.h>
#include <linux/jiffies.h>
#include <asm/param.h>
#else
#include <linux/config.h>
#endif
#endif // __KERNEL__

#ifdef CONFIG_RTK_MESH
#include "mesh_ext/mesh_cfg.h"
#endif
// for 11n
#if defined(CONFIG_RTL_819X)
	#if defined(CONFIG_RTL8196B_AP_ROOT) || defined(CONFIG_RTL8186_GW) || defined(CONFIG_RTL8186_GW_8M)  || defined(CONFIG_RTL8186_GW_VPN) || defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_GW) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196B_TLD)
		#undef CONFIG_RTL865X
		#undef CONFIG_RTL865XC
	#endif
	#define USE_RTL8186_SDK
#endif

#if defined(_BIG_ENDIAN_) || defined(_LITTLE_ENDIAN_)
#elif defined(_BIG_ENDIAN_) && defined(_LITTLE_ENDIAN_)
	#error "One one ENDIAN should be specified\n"
#else
	#error "Please specify your ENDIAN type!\n"
#endif

#ifdef CONFIG_RTL_EB8186
	#define PCI_SLOT0_CONFIG_ADDR		0xbd710000
	#define PCI_SLOT1_CONFIG_ADDR		0xbd720000
#endif
#define PCI_CONFIG_COMMAND			(wdev->conf_addr+4)
#define PCI_CONFIG_LATENCY			(wdev->conf_addr+0x0c)
#define PCI_CONFIG_BASE0			(wdev->conf_addr+0x10)
#define PCI_CONFIG_BASE1			(wdev->conf_addr+0x14)
#define BRIDGE_REG					0xbd010100


//-------------------------------------------------------------
// Driver version information
//-------------------------------------------------------------
#define DRV_VERSION_H	1
#define DRV_VERSION_L	15
#define DRV_RELDATE		"2009-08-13"

#if defined(CONFIG_RTL8190)
	#define DRV_NAME		"RTL8190"
	#define RTL8190
#endif

#if defined(CONFIG_RTL8192E)
	#define DRV_NAME		"RTL8192E"
	#define RTL8192E
#endif

#if defined(CONFIG_RTL8192SE)
	#define DRV_NAME		"RTL8192SE"
	#define RTL8192SE
//	#define RTL8192SE_ACUT
	#define INTERRUPT_2DW 1
	#define MERGE_FW
	#define RX_TASKLET
	#define HIGH_POWER_EXT_PA
	#define ADD_TX_POWER_BY_CMD	// Could increase Tx power by different rate through iwpriv cmd
//	#define EXT_ANT_DVRY //8192se only
//  #define RTL_CACHED_BR_STA
#endif

#if defined(CONFIG_X86)
	#define DRV_NAME		"EXPERIMENTAL"
	#define RTL8192E
#endif

//-------------------------------------------------------------
// Will check type for endian issue when access IO and memory
//-------------------------------------------------------------
#define CHECK_SWAP


//-------------------------------------------------------------
// Defined when include proc file system
//-------------------------------------------------------------
#define INCLUDE_PROC_FS
#if defined(CONFIG_PROC_FS) && defined(INCLUDE_PROC_FS)
	#define _INCLUDE_PROC_FS_
#endif


//-------------------------------------------------------------
// Debug function
//-------------------------------------------------------------
//#define _DEBUG_RTL8190_		// defined when debug print is used
#define _IOCTL_DEBUG_CMD_		// defined when read/write register/memory command is used in ioctl


//-------------------------------------------------------------
// Defined when internal DRAM is used for sw encryption/decryption
//-------------------------------------------------------------
#ifdef __MIPSEB__
	// disable internal ram for nat speedup
	//#define _USE_DRAM_
#endif


//-------------------------------------------------------------
// WDS function support
//-------------------------------------------------------------
#define WDS


//-------------------------------------------------------------
// Pass EAP packet by event queue
//-------------------------------------------------------------
#define EAP_BY_QUEUE
#undef EAPOLSTART_BY_QUEUE	// jimmylin: don't pass eapol-start up
							// due to XP compatibility issue
#if defined(RTL8190) || defined(RTL8192E)
	#define USE_CHAR_DEV
#elif defined(RTL8192SE)
	#define USE_PID_NOTIFY
#endif


//-------------------------------------------------------------
// Client mode function support
//-------------------------------------------------------------
#define CLIENT_MODE
#ifdef CLIENT_MODE
	#define RTK_BR_EXT		// Enable NAT2.5 and MAC clone support
#endif


//-------------------------------------------------------------
// Defined when WPA2 is used
//-------------------------------------------------------------
#define RTL_WPA2
#define RTL_WPA2_PREAUTH


//-------------------------------------------------------------
// MP test
//-------------------------------------------------------------
#define MP_TEST


//-------------------------------------------------------------
// MIC error test
//-------------------------------------------------------------
//#define MICERR_TEST


//-------------------------------------------------------------
// Log event
//-------------------------------------------------------------
#define EVENT_LOG


//-------------------------------------------------------------
// Tx/Rx data path shortcut
//-------------------------------------------------------------
#define TX_SHORTCUT
#define RX_SHORTCUT
#if defined(CONFIG_RTK_MESH) && defined(RX_SHORTCUT)
#define RX_RL_SHORTCUT
#endif 

#if	defined(CONFIG_RTL865X_EXTPORT)
	#undef BR_SHORTCUT
#else
	#define BR_SHORTCUT
#endif
#if defined(CONFIG_RTK_MESH) && defined(TX_SHORTCUT)
	#define MESH_TX_SHORTCUT
#endif


//-------------------------------------------------------------
// back to back test
//-------------------------------------------------------------
#define B2B_TEST


//-------------------------------------------------------------
// Universal Repeater (support AP + Infra client concurrently)
//-------------------------------------------------------------
#define UNIVERSAL_REPEATER


//-------------------------------------------------------------
// Check hangup for Tx queue
//-------------------------------------------------------------
#define CHECK_HANGUP
#ifdef CHECK_HANGUP
#if defined(RTL8190) || defined(RTL8192E)
	#define CHECK_TX_HANGUP
	#define CHECK_RX_HANGUP
	#define CHECK_BEACON_HANGUP
	#define FAST_RECOVERY
#elif defined(RTL8192SE)
	#define CHECK_BEACON_HANGUP
	#define CHECK_BB_HANGUP
	#define FAST_RECOVERY
#endif
#endif


//-------------------------------------------------------------
// DFS
//-------------------------------------------------------------
//#define DFS


//-------------------------------------------------------------
// Driver based WPA PSK feature
//-------------------------------------------------------------
#define INCLUDE_WPA_PSK


//-------------------------------------------------------------
// Tunnel Ethernet SA with Address 4
//-------------------------------------------------------------
//#define A4_TUNNEL


//-------------------------------------------------------------
// RF Fine Tune
//-------------------------------------------------------------
//#define RF_FINETUNE


//-------------------------------------------------------------
// Wifi WMM
//-------------------------------------------------------------
#define SEMI_QOS
#ifdef SEMI_QOS
	#define WMM_APSD	// WMM Power Save
#endif


//-------------------------------------------------------------
// IO mapping access
//-------------------------------------------------------------
#if defined(RTL8190) || defined(RTL8192E)
	#define IO_MAPPING
#endif


//-------------------------------------------------------------
// Wifi Simple Config support
//-------------------------------------------------------------
#define WIFI_SIMPLE_CONFIG


//-------------------------------------------------------------
// Support Multiple BSSID
//-------------------------------------------------------------
#define MBSSID
#ifdef MBSSID
#define RTL8190_NUM_VWLAN  4
#ifdef RTL8192SE
#define FW_SW_BEACON
#endif
#endif


//-------------------------------------------------------------
// Group BandWidth Control
//-------------------------------------------------------------
//#define GBWC


//-------------------------------------------------------------
// Support 802.11 SNMP MIB
//-------------------------------------------------------------
#define SUPPORT_SNMP_MIB


//-------------------------------------------------------------
// Driver-MAC loopback
//-------------------------------------------------------------
#define DRVMAC_LB


//-------------------------------------------------------------
// Use perfomance profiling
//-------------------------------------------------------------
//#define PERF_DUMP

//--------------
//wapi performance issue
//hyking
//--------------
#ifdef CONFIG_RTL_WAPI_SUPPORT
#define IRAM_FOR_WIRELESS_AND_WAPI_PERFORMANCE
#endif

//-------------------
//flag for easy merge
//hf_shi
//-------------------
#ifdef __LINUX_2_6__
#define RTL_PATCHED_26
#endif
//-------------------------------------------------------------
// Software High Queue
//-------------------------------------------------------------
#define SW_MCAST


//-------------------------------------------------------------
// Use local ring for pre-alloc Rx buffer.
// If no defined, will use kernel skb que
//-------------------------------------------------------------
#define RTK_QUE


//-------------------------------------------------------------
//Support IP multicast->unicast
//-------------------------------------------------------------
#define SUPPORT_TX_MCAST2UNI
/* for cameo feature*/
//#define IGMP_FILTER_CMO

//Support  IPV6 multicast->unicast	;2008-0730 add
#ifdef	SUPPORT_TX_MCAST2UNI
#define	TX_SUPPORT_IPV6_MCAST2UNI
#endif

//-------------------------------------------------------------
// Support  USB tx rate adaptive
//-------------------------------------------------------------
// define it always for object code release
//#ifdef CONFIG_USB
	#define USB_PKT_RATE_CTRL_SUPPORT
//#endif


//-------------------------------------------------------------
// Support Tx AMSDU
//-------------------------------------------------------------
#if defined(RTL8190) || defined(RTL8192E)
#define SUPPORT_TX_AMSDU
#endif


//-------------------------------------------------------------
// Mesh Network
//-------------------------------------------------------------

#ifdef CONFIG_RTK_MESH
#define _MESH_ACL_ENABLE_

/*need check Tx AMSDU dependency ; 8196B no support now */
#ifdef	SUPPORT_TX_AMSDU
#define MESH_AMSDU	
#endif
//#define MESH_ESTABLISH_RSSI_THRESHOLD
//#define MESH_BOOTSEQ_AUTH

#endif // CONFIG_RTK_MESH


//-------------------------------------------------------------
// Support buffered Tx ampdu. Not define in default
//-------------------------------------------------------------
//#define BUFFER_TX_AMPDU


//-------------------------------------------------------------
// Realtek proprietary wake up on wlan mode
//-------------------------------------------------------------
//#define RTK_WOW


//-------------------------------------------------------------
// Use static buffer for STA private buffer
//-------------------------------------------------------------
#ifdef RTL8192SE
	#define PRIV_STA_BUF
#endif


//-------------------------------------------------------------
// Do not drop packet immediately when rx buffer empty
//-------------------------------------------------------------
#ifdef RTL8192SE
#ifdef CONFIG_RTL8190_PRIV_SKB
	#define DELAY_REFILL_RX_BUF
#endif	
#endif


//-------------------------------------------------------------
// WiFi 11n 20/40 coexistence
//-------------------------------------------------------------
#ifdef RTL8192SE
//#define WIFI_11N_2040_COEXIST
#endif


//-------------------------------------------------------------
// 8192SE define flag
// Those functions need to be implemented in the future
//-------------------------------------------------------------
#ifdef RTL8192SE

#ifdef B2B_TEST
	#undef B2B_TEST
#endif

#ifdef DRVMAC_LB
	#undef DRVMAC_LB
#endif

#endif // RTL8192SE


//-------------------------------------------------------------
// GR define flag
//-------------------------------------------------------------
#ifdef CONFIG_RTL8186_GR

#ifndef INCLUDE_WPA_PSK
	#define INCLUDE_WPA_PSK
#endif

#ifdef UNIVERSAL_REPEATER
	#undef UNIVERSAL_REPEATER
#endif

#ifdef CLIENT_MODE
	#undef CLIENT_MODE
	#undef RTK_BR_EXT
#endif

#ifndef WIFI_SIMPLE_CONFIG
	#define WIFI_SIMPLE_CONFIG
#endif

#ifdef GBWC
	#undef GBWC
#endif

#ifdef RTL8190_NUM_VWLAN
	#undef RTL8190_NUM_VWLAN
	#define RTL8190_NUM_VWLAN 1
#endif

#ifdef SUPPORT_SNMP_MIB
	#undef SUPPORT_SNMP_MIB
#endif

#endif // CONFIG_RTL8186_GR


//-------------------------------------------------------------
// BK define flag
//-------------------------------------------------------------
#ifdef CONFIG_RTL8186_KB

#ifndef INCLUDE_WPA_PSK
	#define INCLUDE_WPA_PSK
#endif

#ifdef UNIVERSAL_REPEATER
	#undef UNIVERSAL_REPEATER
#endif

#ifdef CLIENT_MODE
	#undef CLIENT_MODE
	#undef RTK_BR_EXT
#endif

#ifndef WIFI_SIMPLE_CONFIG
	#define WIFI_SIMPLE_CONFIG
#endif

#ifdef GBWC
	#undef GBWC
#endif

#ifdef RTL8190_NUM_VWLAN
	#undef RTL8190_NUM_VWLAN
	#define RTL8190_NUM_VWLAN 1
#endif

#ifdef SUPPORT_SNMP_MIB
	#undef SUPPORT_SNMP_MIB
#endif

#endif // CONFIG_RTL8186_KB


//-------------------------------------------------------------
// TR define flag
//-------------------------------------------------------------
#if defined(CONFIG_RTL8196B_TR)

#ifndef INCLUDE_WPA_PSK
	#define INCLUDE_WPA_PSK
#endif

#ifdef UNIVERSAL_REPEATER
	#undef UNIVERSAL_REPEATER
#endif

#ifdef CLIENT_MODE
	#undef CLIENT_MODE
	#undef RTK_BR_EXT
#endif

#ifndef WIFI_SIMPLE_CONFIG
	#define WIFI_SIMPLE_CONFIG
#endif

#ifdef GBWC
	#undef GBWC
#endif

#ifdef SUPPORT_SNMP_MIB
	#undef SUPPORT_SNMP_MIB
#endif

#endif // CONFIG_RTL8196B_TR


//-------------------------------------------------------------
// AC define flag
//-------------------------------------------------------------
#ifdef CONFIG_RTL865X_AC

#ifndef INCLUDE_WPA_PSK
	#define INCLUDE_WPA_PSK
#endif

#ifdef UNIVERSAL_REPEATER
	#undef UNIVERSAL_REPEATER
#endif

#ifdef CLIENT_MODE
	#undef CLIENT_MODE
	#undef RTK_BR_EXT
#endif

#ifndef WIFI_SIMPLE_CONFIG
	#define WIFI_SIMPLE_CONFIG
#endif

#ifdef GBWC
	#undef GBWC
#endif

#ifdef SUPPORT_SNMP_MIB
	#undef SUPPORT_SNMP_MIB
#endif

#endif // CONFIG_RTL865X_AC


//-------------------------------------------------------------
// KLD define flag
//-------------------------------------------------------------
#if defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD)

#ifndef INCLUDE_WPA_PSK
	#define INCLUDE_WPA_PSK
#endif

#ifdef UNIVERSAL_REPEATER
	#undef UNIVERSAL_REPEATER
#endif

#ifndef WIFI_SIMPLE_CONFIG
	#define WIFI_SIMPLE_CONFIG
#endif

#ifdef GBWC
	#undef GBWC
#endif

#ifdef SUPPORT_SNMP_MIB
	#undef SUPPORT_SNMP_MIB
#endif

#ifdef DRVMAC_LB
	#undef DRVMAC_LB
#endif

#ifdef MBSSID
	#undef RTL8190_NUM_VWLAN
	#define RTL8190_NUM_VWLAN  1
#endif

#ifdef HIGH_POWER_EXT_PA
	#undef HIGH_POWER_EXT_PA
#endif

#ifdef ADD_TX_POWER_BY_CMD
	#undef ADD_TX_POWER_BY_CMD
#endif

#endif // CONFIG_RTL865X_KLD

#if defined(CONFIG_RTL865X_KLD) && defined(MBSSID)
	#undef MBSSID
	#undef FW_SW_BEACON
#endif

//#if defined(CONFIG_RTL8196B_KLD) && defined(RTL8192SE)
//	#define HW_QUICK_INIT
//#endif

//-------------------------------------------------------------
// SITECOM define flag
//-------------------------------------------------------------
#ifdef CONFIG_RTL865X_SC

#ifndef INCLUDE_WPA_PSK
	#define INCLUDE_WPA_PSK
#endif

#ifdef UNIVERSAL_REPEATER
	#undef UNIVERSAL_REPEATER
#endif

#ifdef CLIENT_MODE
	#undef CLIENT_MODE
	#undef RTK_BR_EXT
#endif

#ifndef WIFI_SIMPLE_CONFIG
	#define WIFI_SIMPLE_CONFIG
#endif

#ifdef GBWC
	#undef GBWC
#endif

#ifdef SUPPORT_SNMP_MIB
	#undef SUPPORT_SNMP_MIB
#endif

#endif // CONFIG_RTL865X_SC

/* some definitions in 8190 driver, we set them as NULL definition */

#ifdef USE_RTL8186_SDK
#ifdef CONFIG_WIRELESS_LAN_MODULE
#define __DRAM_IN_865X
#define __IRAM_IN_865X
#define __IRAM_IN_865X_HI
#else
#define __DRAM_IN_865X		__attribute__ ((section(".dram-rtkwlan")))
#define __IRAM_IN_865X		__attribute__ ((section(".iram-rtkwlan")))
#define __IRAM_IN_865X_HI		__attribute__ ((section(".iram-tx")))
#endif
#define RTL8190_DIRECT_RX						/* For packet RX : directly receive the packet instead of queuing it */
#define RTL8190_ISR_RX							/* process RXed packet in interrupt service routine: It become useful only when RTL8190_DIRECT_RX is defined */
#ifndef CONFIG_WIRELESS_LAN_MODULE
#define RTL8190_VARIABLE_USED_DMEM				/* Use DMEM for some critical variables */
#endif

#else // not USE_RTL8186_SDK

#define __DRAM_IN_865X
#define __IRAM_IN_865X
#define __IRAM_IN_865X_HI

#endif

#ifndef RTL8190_FASTEXTDEV_FUNCALL
#define __DRAM_FASTEXTDEV	__DRAM_IN_865X
#define __IRAM_FASTEXTDEV	__IRAM_IN_865X
#endif

#undef __MIPS16
#define __MIPS16			__attribute__ ((mips16))

#if defined(CONFIG_RTL_DYNAMIC_IRAM_MAPPING_FOR_WAPI)
#define __IRAM_WLAN_HI		__attribute__  ((section(".iram-wapi")))
#define __DRAM_WLAN_HI	__attribute__  ((section(".dram-wapi")))
#define __IRAM_IN_865X_WAPI		__attribute__ ((section(".iram-rtkwlan-wapi")))
#endif


//-------------------------------------------------------------
// X86 define flag
//-------------------------------------------------------------
#ifdef CONFIG_X86

#ifndef CONFIG_NET_PCI
	#define CONFIG_NET_PCI
#endif

#ifdef __MIPSEB__
	#undef __MIPSEB__
#endif

#ifdef _BIG_ENDIAN_
	#undef _BIG_ENDIAN_
#endif

#ifndef _LITTLE_ENDIAN_
	#define _LITTLE_ENDIAN_
#endif

#ifdef _USE_DRAM_
	#undef _USE_DRAM_
#endif

#ifdef CHECK_SWAP
	#undef CHECK_SWAP
#endif

#ifdef EVENT_LOG
	#undef EVENT_LOG
#endif

#ifdef BR_SHORTCUT
	#undef BR_SHORTCUT
#endif

#ifdef RTK_BR_EXT
	#undef RTK_BR_EXT
#endif

#ifdef UNIVERSAL_REPEATER
	#undef UNIVERSAL_REPEATER
#endif

#ifdef GBWC
	#undef GBWC
#endif

#ifndef __LINUX_2_6__
	#define del_timer_sync del_timer
#endif

#ifdef USE_IO_OPS
	#undef USE_IO_OPS
#endif

#ifdef IO_MAPPING
	#undef IO_MAPPING
#endif

#ifdef MBSSID
	#undef MBSSID
#endif

#ifdef RTK_QUE
	#undef RTK_QUE
#endif

#endif // CONFIG_X86


//-------------------------------------------------------------
// Define flag of 867X system
//-------------------------------------------------------------
#ifdef CONFIG_RTL8671

#define DRV_VERSION_SUBL 4

#undef PCI_SLOT0_CONFIG_ADDR
#undef PCI_SLOT1_CONFIG_ADDR

#define PCI_SLOT0_CONFIG_ADDR		0xbd100000
#define PCI_SLOT1_CONFIG_ADDR		0xbd180000

#undef PCI_CONFIG_BASE1
#ifdef IO_MAPPING
#define PCI_CONFIG_BASE1			(wdev->conf_addr+0x10)
#else
#define PCI_CONFIG_BASE1			(wdev->conf_addr+0x14)
#endif

#ifdef EVENT_LOG
	#undef EVENT_LOG
#endif

#ifdef UNIVERSAL_REPEATER
	#undef UNIVERSAL_REPEATER
#endif

#ifdef GBWC
	#undef GBWC
#endif

//11/25/05' hrchen, add for 8671 platform
/* for Event Log */
#define scrlog_printk	printk

#endif // CONFIG_RTL8671


//-------------------------------------------------------------
// Define flag for Zinwell
//-------------------------------------------------------------
#ifdef __ZINWELL__

#ifndef INCLUDE_WPA_PSK
	#define INCLUDE_WPA_PSK
#endif

#endif


//-------------------------------------------------------------
// TLD define flag
//-------------------------------------------------------------
#ifdef CONFIG_RTL8196B_TLD

#ifndef STA_EXT
	#define STA_EXT
#endif

#ifdef GBWC
	#undef GBWC
#endif

#ifdef SUPPORT_SNMP_MIB
	#undef SUPPORT_SNMP_MIB
#endif

#ifdef DRVMAC_LB
	#undef DRVMAC_LB
#endif

#ifdef HIGH_POWER_EXT_PA
	#undef HIGH_POWER_EXT_PA
#endif

#ifdef ADD_TX_POWER_BY_CMD
	#undef ADD_TX_POWER_BY_CMD
#endif

#endif // CONFIG_RTL8196B_TLD


//-------------------------------------------------------------
// Dependence check of define flag
//-------------------------------------------------------------
#if defined(B2B_TEST) && !defined(MP_TEST)
	#error "Define flag error, MP_TEST is not defined!\n"
#endif


#if defined(UNIVERSAL_REPEATER) && !defined(CLIENT_MODE)
	#error "Define flag error, CLIENT_MODE is not defined!\n"
#endif


/*=============================================================*/
/*------ Compiler Portability Macros --------------------------*/
/*=============================================================*/
#ifdef EVENT_LOG
//	extern int scrlog_printk(const char * fmt, ...);
#define	scrlog_printk
#ifdef CONFIG_RTK_MESH
/*
 *	NOTE: dot1180211sInfo.log_enabled content from webpage MIB_LOG_ENABLED (bitmap) (in AP/goahead-2.1.1/LINUX/fmmgmt.c  formSysLog)
 */
	#define _LOG_MSG(fmt, args...)	if (1 & GET_MIB(priv)->dot1180211sInfo.log_enabled) scrlog_printk(fmt, ## args)
	#define LOG_MESH_MSG(fmt, args...)	if (16 & GET_MIB(priv)->dot1180211sInfo.log_enabled) _LOG_MSG("%s: " fmt, priv->mesh_dev->name, ## args)
	#else
	#define _LOG_MSG(fmt, args...)	scrlog_printk(fmt, ## args)
#endif
#if defined(CONFIG_RTL8196B_TR)
	#define _NOTICE	"NOTICElog_num:13;msg:"
	#define _DROPT	"DROPlog_num:13;msg:"
	#define _SYSACT "SYSACTlog_num:13;msg:"

	#define LOG_MSG_NOTICE(fmt, args...) _LOG_MSG("%s" fmt, _NOTICE, ## args)
	#define LOG_MSG_DROP(fmt, args...) _LOG_MSG("%s" fmt, _DROPT, ## args)
	#define LOG_MSG_SYSACT(fmt, args...) _LOG_MSG("%s" fmt, _SYSACT, ## args)
	#define LOG_MSG(fmt, args...)	{}

	#define LOG_START_MSG() { \
			char tmpbuf[10]; \
			LOG_MSG_NOTICE("Access Point: %s started at channel %d;\n", \
				priv->pmib->dot11StationConfigEntry.dot11DesiredSSID, \
				priv->pmib->dot11RFEntry.dot11channel); \
			if (priv->pmib->dot11StationConfigEntry.autoRate) \
				strcpy(tmpbuf, "best"); \
			else \
				sprintf(tmpbuf, "%d", get_rate_from_bit_value(priv->pmib->dot11StationConfigEntry.fixedTxRate)/2); \
			LOG_MSG_SYSACT("AP 2.4GHz mode Ready. Channel : %d TxRate : %s SSID : %s;\n", \
				priv->pmib->dot11RFEntry.dot11channel, \
				tmpbuf, priv->pmib->dot11StationConfigEntry.dot11DesiredSSID); \
	}

#elif defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD)
	#define _NOTICE	"NOTICElog_num:13;msg:"
	#define _DROPT	"DROPlog_num:13;msg:"
	#define _SYSACT "SYSACTlog_num:13;msg:"

	#define LOG_MSG_NOTICE(fmt, args...) _LOG_MSG("%s" fmt, _NOTICE, ## args)
	#define LOG_MSG_DROP(fmt, args...) _LOG_MSG("%s" fmt, _DROPT, ## args)
	#define LOG_MSG_SYSACT(fmt, args...) _LOG_MSG("%s" fmt, _SYSACT, ## args)
	#define LOG_MSG(fmt, args...)	{}

	#define LOG_START_MSG() { \
			char tmpbuf[10]; \
			LOG_MSG_NOTICE("Access Point: %s started at channel %d;\n", \
				priv->pmib->dot11StationConfigEntry.dot11DesiredSSID, \
				priv->pmib->dot11RFEntry.dot11channel); \
			if (priv->pmib->dot11StationConfigEntry.autoRate) \
				strcpy(tmpbuf, "best"); \
			else \
				sprintf(tmpbuf, "%d", get_rate_from_bit_value(priv->pmib->dot11StationConfigEntry.fixedTxRate)/2); \
			LOG_MSG_SYSACT("AP 2.4GHz mode Ready. Channel : %d TxRate : %s SSID : %s;\n", \
				priv->pmib->dot11RFEntry.dot11channel, \
				tmpbuf, priv->pmib->dot11StationConfigEntry.dot11DesiredSSID); \
	}
#elif defined(CONFIG_RTL8196B_TLD)
	#define LOG_MSG_DEL(fmt, args...)	_LOG_MSG(fmt, ## args)
	#define LOG_MSG(fmt, args...)	{}
#else
	#define LOG_MSG(fmt, args...)	_LOG_MSG("%s: "fmt, priv->dev->name, ## args)
#endif
#else
	#if defined(__GNUC__) || defined(GREEN_HILL)
		#define LOG_MSG(fmt, args...)	{}
	#else
		#define LOG_MSG
	#endif
#endif // EVENT_LOG

#if defined(CONFIG_RTL_EB8186) && defined(__KERNEL__)
	#define _USE_IRAM_			__attribute__ ((section(".wlan_speedup")))
#else
	#define _USE_IRAM_
#endif

#ifdef _USE_DRAM_
	#define DRAM_START_ADDR		0x81000000	// start address of internal data ram
#endif

#ifdef __GNUC__
#define __WLAN_ATTRIB_PACK__		__attribute__ ((packed))
#define __PACK
#endif

#ifdef __arm
#define __WLAN_ATTRIB_PACK__
#define __PACK	__packed
#endif

#ifdef GREEN_HILL
#define __WLAN_ATTRIB_PACK__
#define __PACK
#endif


/*=============================================================*/
/*-----------_ Driver module flags ----------------------------*/
/*=============================================================*/
#ifdef CONFIG_WIRELESS_LAN_MODULE
	#define	MODULE_NAME		"Realtek WirelessLan Driver"
	#define	MODULE_VERSION	"v1.00"

	#define MDL_DEVINIT
	#define MDL_DEVEXIT
	#define MDL_INIT
	#define MDL_EXIT
	#define MDL_DEVINITDATA
#else
	#define MDL_DEVINIT		__devinit
	#define MDL_DEVEXIT		__devexit
	#define MDL_INIT		__init
	#define MDL_EXIT		__exit
	#define MDL_DEVINITDATA	__devinitdata
#endif


/*=============================================================*/
/*----------- System configuration ----------------------------*/
/*=============================================================*/
#if defined(RTL8192SE) && defined(CONFIG_RTL8196B_GW_8M)
#define NUM_TX_DESC		200
#else
#define NUM_TX_DESC		512		// from 256 -> 512, for Vista testing
#endif

#ifdef RTL8192SE
#define NUM_RX_DESC		32
#ifdef DELAY_REFILL_RX_BUF
	#define REFILL_THRESHOLD	NUM_RX_DESC
#endif
#else
#define NUM_RX_DESC		128
#ifdef DELAY_REFILL_RX_BUF
	#define REFILL_THRESHOLD	32
#endif
#endif

#if defined(CONFIG_RTL8196B_KLD) && defined(RTL8192SE) && defined(MBSSID)
#define NUM_CMD_DESC	2
#else
#define NUM_CMD_DESC	16
#endif

#ifdef STA_EXT
#define NUM_STAT		64
#else
#define NUM_STAT		32
#endif
#define MAX_GUEST_NUM   NUM_STAT
#ifdef RTL8192E
#define FW_NUM_STAT	7
#elif defined(STA_EXT)
#define FW_NUM_STAT 32
#endif

#ifdef __ZINWELL__
#undef NUM_STAT
#define NUM_STAT		128
#endif

#define NUM_TXPKT_QUEUE			64
#define NUM_APSD_TXPKT_QUEUE	32

#define PRE_ALLOCATED_HDR		NUM_TX_DESC
#define PRE_ALLOCATED_MMPDU		64
#define PRE_ALLOCATED_BUFSIZE	(600/4)		// 600 bytes long should be enough for mgt! Declare as unsigned int

#define MAX_BSS_NUM		64


#define MAX_NUM_WLANIF		4
#define WLAN_MISC_MAJOR		13

#define MAX_FRAG_COUNT		10

#define NUM_MP_SKB		32

// unit of time out: 10 msec
#define AUTH_TO			500
#define ASSOC_TO		500
#define FRAG_TO			2000
#define SS_TO			5
#define SS_PSSV_TO		12			// passive scan for 120 ms
#ifdef CONFIG_RTK_MESH
//GANTOE for automatic site survey 2008/12/10
#define SS_RAND_DEFER		300
#endif
#ifdef LINUX_2_6_22_
#define EXPIRE_TO		HZ
#else
#define EXPIRE_TO		100
#endif
#define REAUTH_TO		500
#define REASSOC_TO		500
#define REAUTH_LIMIT	6
#define REASSOC_LIMIT	6

#define DEFAULT_OLBC_EXPIRE		60

#define GBWC_TO			25

#ifdef __DRAYTEK_OS__
#define SS_COUNT		2
#else
#define SS_COUNT		3
#endif

#define TUPLE_WINDOW	64

#define RC_TIMER_NUM	64
#define RC_ENTRY_NUM	128
#define AMSDU_TIMER_NUM	64
#define AMPDU_TIMER_NUM	64

#define ROAMING_DECISION_PERIOD_INFRA	5
#define ROAMING_DECISION_PERIOD_ADHOC	10
#define ROAMING_DECISION_PERIOD_ARRAY (ROAMING_DECISION_PERIOD_ADHOC+1)
#define ROAMING_THRESHOLD		1	// roaming will be triggered when rx
									// beacon percentage is less than the value
#define FAST_ROAMING_THRESHOLD	40

/* below is for security.h  */
#define MAXDATALEN		1560
#define MAXQUEUESIZE	4
#define MAXRSNIELEN		128
#define E_DOT11_2LARGE	-1
#define E_DOT11_QFULL	-2
#define E_DOT11_QEMPTY	-3
#ifdef WIFI_SIMPLE_CONFIG
#define PROBEIELEN		128
#endif

// for SW LED
#define LED_MAX_PACKET_CNT_B	400
#define LED_MAX_PACKET_CNT_AG	1200
#define LED_MAX_SCALE			100
#define LED_NOBLINK_TIME		110	// time more than watchdog interval
#define LED_INTERVAL_TIME		50	// 500ms
#define LED_ON_TIME				4	// 40ms
#define LED_ON					0
#define LED_OFF					1
#define LED_0 					0
#define LED_1 					1

// for counting association number
#define INCREASE		1
#define DECREASE		0

// DFS
#define CH_AVAIL_CHK_TO			6200	 // 62 seconds

// for 11n
#ifdef CONFIG_RTL8190
#ifdef USE_RTL8186_SDK
	#define PCI_SLOT0_CONFIG_ADDR		0xB8B40000
	#define PCI_SLOT1_CONFIG_ADDR		0xB8B80000
#endif
#endif

#define MAX_RX_BUF_LEN	8400
//#define MIN_RX_BUF_LEN	4300
#define MIN_RX_BUF_LEN	4400

/* for RTL865x suspend mode */
#define TP_HIGH_WATER_MARK 50 //80 /* unit: Mbps */
#define TP_LOW_WATER_MARK 30 //40 /* unit: Mbps */

#define FW_BOOT_SIZE	400
#define FW_MAIN_SIZE	52000
#define FW_DATA_SIZE	850
#define AGC_TAB_SIZE	1600
#if defined(RTL8190) || defined(RTL8192E)
#define MAC_REG_SIZE	120
#define	PHY_REG_SIZE	1280
#elif defined(RTL8192SE)
#define MAC_REG_SIZE	1024
#define	PHY_REG_SIZE	2048
#define PHY_REG_PG_SIZE 256
#define PHY_REG_1T2R	256
#define PHY_REG_1T1R	256
#define FW_IMEM_SIZE	40*(1024)
#define FW_EMEM_SIZE	50*(1024)
#define FW_DMEM_SIZE	48
#endif

#ifdef SUPPORT_TX_MCAST2UNI
#define MAX_IP_MC_ENTRY		8
#endif


//-------------------------------------------------------------
// Define flag for 8M gateway configuration
//-------------------------------------------------------------
#ifdef CONFIG_RTL8196B_GW_8M

//#ifdef MERGE_FW
//	#undef MERGE_FW
//#endif
//#define DW_FW_BY_MALLOC_BUF

#ifdef MBSSID
	#undef RTL8190_NUM_VWLAN
	#define RTL8190_NUM_VWLAN  1
#endif

#undef NUM_STAT		
#define NUM_STAT		16

#endif // CONFIG_RTL8196B_GW_8M

#endif // _8190N_CFG_H_

