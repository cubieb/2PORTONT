#ifndef RTK_RG_DEFINE_H
#define RTK_RG_DEFINE_H

#if defined(CONFIG_APOLLO)
#include <hal/chipdef/chip.h>	//for chip id and sub type
#endif

#ifdef __KERNEL__
#if defined(CONFIG_DEFAULTS_KERNEL_3_18)
#else
#include <linux/config.h>
#endif
#endif


#define __IRAM_FWDENG			__attribute__ ((section(".iram-fwd")))
#define __IRAM_FWDENG_L2		__attribute__ ((section(".iram-l2-fwd")))
#define __IRAM_FWDENG_L34		__attribute__ ((section(".iram-l34-fwd")))
#define __IRAM_FWDENG_SLOWPATH	__attribute__ ((section(".iram-l34-fwd")))
#define __SRAM_FWDENG_SLOWPATH	__attribute__ ((section(".sram_text")))
#define __SRAM_FWDENG_DATA		__attribute__ ((section(".sram_data")))

/* for SMP */
#ifdef CONFIG_SMP

/* tasklet */
#define MAX_RG_TASKLET_QUEUE_SIZE 1024 //must power of 2

#define rg_tasklet_queue_lock_irqsave(lock, flags)			\
	do {						\
		if(atomic_read(&rg_kernel.rg_tasklet_queue_entrance)>=1)	\
			WARNING("RG TASKLET QUEUE LOCK REDUPLICATE ENTRANCE!!");	\
		atomic_inc(&rg_kernel.rg_tasklet_queue_entrance);	\
		preempt_disable(); \
		local_bh_disable(); \
		spin_lock_irqsave(lock, flags);	\
	} while (0)

#define rg_tasklet_queue_unlock_irqrestore(lock, flags)			\
	do {						\
		spin_unlock_irqrestore(lock, flags);	\
		preempt_enable(); \
		local_bh_enable(); \
		atomic_dec(&rg_kernel.rg_tasklet_queue_entrance);	\
		if(atomic_read(&rg_kernel.rg_tasklet_queue_entrance)<0)	\
			WARNING("RG TASKLET QUEUE UNLOCK REDUPLICATE RELEASE!!");	\
	} while (0)


#endif


/* for gpon FPGA testing */
//#define CONFIG_APOLLO_GPON_FPGATEST

/*Switch for NAPT and routing shortcut function */
#define CONFIG_ROME_NAPT_SHORTCUT

/*Switch for NAPT table refresh by LRU */
#define CONFIG_ROME_NAPT_LRU

/*Switch for NEIGHBOR table refresh by LRU */
#define CONFIG_ROME_NEIGHBOR_LRU

/* System Module */
#define DEFAULT_CPU_VLAN 			1
#if defined(CONFIG_RTL9600_SERIES)
#else
#define DEFAULT_CPU_SVLAN 			2
#endif

#ifdef CONFIG_RG_WAN_PORT_ISOLATE
#if defined(CONFIG_RTL9600_SERIES)
#define DEFAULT_WAN_VLAN 			2
#else
#define DEFAULT_WAN_VLAN 			3
#endif
#endif
#define DEFAULT_PROTO_BLOCK_VLAN 	4000		//used for IP_version only_mode, as PVID to block traffic to other port
#define DEFAULT_BIND_INTERNET		4005
#define DEFAULT_BIND_OTHER			4012		//this will use 4012~4012+DEFAULT_BIND_LAN_OFFSET+DEFAULT_BIND_WLAN_OFFSET
#ifdef CONFIG_MASTER_WLAN0_ENABLE
#ifdef CONFIG_RTL_WDS_SUPPORT
#ifdef CONFIG_RTL_VAP_SUPPORT
#define DEFAULT_BIND_LAN_OFFSET		18			//0,1,2,3 ext-0, ext-1, root, vap1, vap2, vap3, vap4, wds5, wds6, wds7 ,wds8, wds9, wds10, wds11, wds12
#else
#define DEFAULT_BIND_LAN_OFFSET		14			//0,1,2,3 ext-0, ext-1, root, wds1, wds2, wds3 ,wds4, wds5, wds6, wds7, wds8
#endif
#else
#ifdef CONFIG_RTL_VAP_SUPPORT
#define DEFAULT_BIND_LAN_OFFSET		10			//0,1,2,3 ext-0, ext-1, root, vap1, vap2, vap3, vap4
#else
#define DEFAULT_BIND_LAN_OFFSET		5			//0,1,2,3 ext-0, ext-1
#endif
#endif
#else
#define DEFAULT_BIND_LAN_OFFSET		5			//0,1,2,3 ext-0, ext-1
#endif
//#define DEFAULT_WAN_PVID 			4001
//#define DEFAULT_PPB_VLAN_WAN		4002		//used for wan port
//#define DEFAULT_PPB_VLAN_START		4003		//used for each lan


#if defined(CONFIG_RTL9602C_SERIES)

#define HOST_POLICING_TABLE_SIZE	8
#define V4_DEFAULT_ROUTE_IDX	15
#define V6_DEFAULT_ROUTE_IDX	7
#define V6_HW_DEFAULT_ROUTE_IDX	7

#elif defined(CONFIG_RTL9600_SERIES)


#define V4_DEFAULT_ROUTE_IDX	7
#ifdef CONFIG_RG_IPV6_NAPT_SUPPORT
#define V6_DEFAULT_ROUTE_IDX	7
#else
#define V6_DEFAULT_ROUTE_IDX	3
#endif
#define V6_HW_DEFAULT_ROUTE_IDX	3

#elif defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
#define V4_DEFAULT_ROUTE_IDX	7
#define V6_DEFAULT_ROUTE_IDX	7
#define V6_HW_DEFAULT_ROUTE_IDX	3
#endif




#ifdef CONFIG_DUALBAND_CONCURRENT
#define SLAVE_WIFI_ROUTE_IDX	6
#endif


#if defined(CONFIG_RTL9600_SERIES)
#define LAN_FID					2
#define WAN_FID					2
#define Layer2HOUSE_KEEP_NUM	264		//how many entries should be check at once, MUST be the common divisor of LUT table
#define TRAP_RSN_START			64		//used for DHCP offer trap in fwdEngine
#elif defined(CONFIG_RTL9602C_SERIES)
#define DEFAULT_L3MCR_IDX		0
#define LAN_FID					1
#define WAN_FID					1
#define Layer2HOUSE_KEEP_NUM	136		//how many entries should be check at once, MUST be the common divisor of LUT table
#define TRAP_RSN_START			128		//used for DHCP offer trap in fwdEngine
#elif defined(CONFIG_XDSL_NEW_HWNAT_DRIVER) && defined(CONFIG_XDSL_ROMEDRIVER)
#define LAN_FID					0
#define WAN_FID					0
#define Layer2HOUSE_KEEP_NUM	256		//how many entries should be check at once, MUST be the common divisor of LUT table
#endif

#define NIC_RX_PRIORITY			16		//used to nic register for RX


#define WanAccessCategoryNum	8
#define MAX_WanAccessARPCount	1		//means we only send "MAX_WanAccessARPCount" times to check host on-line or not, each count will ask for THREE times.

#define RTK_RG_EGRESS_QOS_TEST_PATCH	1
#define RTK_RG_INGRESS_QOS_TEST_PATCH	1


//#define PPPOE_DISCOVERY_GROUPID	0
//#define PPPOE_SESSION_GROUPID	1
/* Port and Protocol VLAN Group */
#define MAX_PORT_PROTO_GROUP_SIZE	4
#define RG_IPV4_GROUPID				0
#define RG_ARP_GROUPID				1
#define RG_IPV6_GROUPID				2
#define RG_IPV4_ETHERTYPE			0x0800
#define RG_ARP_ETHERTYPE			0x0806
#define RG_IPV6_ETHERTYPE			0x86dd
#define FRAGMENT_LIST_TIMEOUT		60
#define FRAGMENT_QUEUE_TIMEOUT		60

#if 0
#define APOLLO_SWITCH_CHIP		0x6266
#define APOLLO_SWITCH_CHIP_REV_B	0x3
#define APOLLO_SWITCH_CHIP_REV_C	0x4
//#define APOLLO_SWITCH_CHIP_REV_C	0x0 //only for debug (don't patch old ic function)
#define APOLLO_SWITCH_CHIP_REV_D	0x5
#endif


#define PATCH_6266_MTU_PPPOE		8
#define PATCH_6266_MTU_BINDING		2

//rtk_rg_skb_preallocate for apollo only
#ifdef CONFIG_APOLLO
#define RTK_RG_SKB_PREALLOCATE	1
#else
#ifdef RTK_RG_SKB_PREALLOCATE
#undef RTK_RG_SKB_PREALLOCATE
#endif
#if defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
#define RTK_RG_SKB_PREALLOCATE	0
#endif // defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
#endif


#if defined(CONFIG_DUALBAND_CONCURRENT)
#define SKB_BUF_SIZE  1800
#else
#define SKB_BUF_SIZE  1600
#endif

/* RTK RG Timeout Defination */
#ifdef __KERNEL__
#define TICKTIME_PERIOD		(CONFIG_HZ)	//How many jiffies per second
#endif
#define RTK_RG_DEFAULT_HOUSE_KEEP_SECOND	2	//secs
#define RTK_RG_DEFAULT_ARP_REQUEST_INTERVAL_SECOND 	1 // 1 secs

#define RTK_RG_DEFAULT_MCAST_QUERY_INTERVAL 30  //secs
#define RTK_RG_DEFAULT_IGMP_SYS_TIMER_INTERVAL 10 //secs
#define RTK_RG_DEFAULT_IGMP_SYS_MAX_SIMULTANEOUS_GROUP_SIZE_UNLIMIT 0 //0 means unlimited size



#define RTK_RG_SWRATELIMIT_SECOND	1	//unit: (1/16)sec

#if defined(CONFIG_ROME_NAPT_SHORTCUT)
#define RTK_RG_DEFAULT_V4_SHORTCUT_TIMEOUT 	3600 // 1 hour
#define V4_SHORTCUT_KEEP_NUM (MAX_NAPT_SHORTCUT_SIZE>>4) // how many entries should be check at once, MUST be the common divisor of v4 shortcut table size
#endif
#if defined(CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT)
#define RTK_RG_DEFAULT_V6_SHORTCUT_TIMEOUT 	3600 // 1 hour
#define V6_SHORTCUT_KEEP_NUM (MAX_NAPT_V6_SHORTCUT_SIZE) // how many entries should be check at once, MUST be the common divisor of v6 shortcut table size
#endif

#define RTK_RG_DEFAULT_TCP_LONG_TIMEOUT 	3600 // 1 hour
#define RTK_RG_DEFAULT_TCP_SHORT_TIMEOUT	20	// 20 secs
#define RTK_RG_DEFAULT_UDP_LONG_TIMEOUT 	600 // 10 mins
#define RTK_RG_DEFAULT_UDP_SHORT_TIMEOUT	20	// 20 secs
#define RTK_RG_DEFAULT_ARP_TIMEOUT 	300 // 300 secs
#define RTK_RG_DEFAULT_NEIGHBOR_TIMEOUT 	300 // 300 secs
#if defined(CONFIG_RTL9600_SERIES)
#else	//support lut traffic bit
#define RTK_RG_DEFAULT_L2_TIMEOUT 	300 // 300 secs
#endif


/* RTK RG API */
#if defined(CONFIG_RTL9602C_SERIES)
#define RTK_RG_MAX_MAC_PORT 4
#define RTK_RG_MAX_EXT_PORT (RTK_RG_PORT_MAX - RTK_RG_PORT_CPU)
#define RTK_RG_ALL_MAC_PORTMASK ((1<<RTK_RG_MAC_PORT0)|(1<<RTK_RG_MAC_PORT1)|(1<<RTK_RG_MAC_PORT_PON)|(1<<RTK_RG_MAC_PORT_CPU))
#define RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU ((1<<RTK_RG_MAC_PORT0)|(1<<RTK_RG_MAC_PORT1)|(1<<RTK_RG_MAC_PORT_PON))
#define RTK_RG_ALL_EXT_PORTMASK ((1<<RTK_RG_EXT_PORT0)|(1<<RTK_RG_EXT_PORT1)|(1<<RTK_RG_EXT_PORT2)|(1<<RTK_RG_EXT_PORT3)|(1<<RTK_RG_EXT_PORT4)|(1<<RTK_RG_EXT_PORT5))
#define RTK_RG_ALL_VIRUAL_PORTMASK (1|(1<<(RTK_RG_EXT_PORT0-RTK_RG_PORT_CPU))|(1<<(RTK_RG_EXT_PORT1-RTK_RG_PORT_CPU))|(1<<(RTK_RG_EXT_PORT2-RTK_RG_PORT_CPU))|(1<<(RTK_RG_EXT_PORT3-RTK_RG_PORT_CPU))|(1<<(RTK_RG_EXT_PORT4-RTK_RG_PORT_CPU))|(1<<(RTK_RG_EXT_PORT5-RTK_RG_PORT_CPU)))
#define RTK_RG_ALL_USED_VIRUAL_PORTMASK (1|(1<<(RTK_RG_EXT_PORT0-RTK_RG_PORT_CPU))|(1<<(RTK_RG_EXT_PORT1-RTK_RG_PORT_CPU)))
#define RTK_RG_ALL_LAN_PORTMASK ((1<<RTK_RG_MAC_PORT0)|(1<<RTK_RG_MAC_PORT1))


#elif defined(CONFIG_RTL9600_SERIES)

#define RTK_RG_MAX_MAC_PORT 7
#define RTK_RG_MAX_EXT_PORT (RTK_RG_PORT_MAX - RTK_RG_PORT_CPU)
#define RTK_RG_ALL_MAC_PORTMASK ((1<<RTK_RG_MAC_PORT0)|(1<<RTK_RG_MAC_PORT1)|(1<<RTK_RG_MAC_PORT2)|(1<<RTK_RG_MAC_PORT3)|(1<<RTK_RG_MAC_PORT_PON)|(1<<RTK_RG_MAC_PORT_RGMII)|(1<<RTK_RG_MAC_PORT_CPU))
#define RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU ((1<<RTK_RG_MAC_PORT0)|(1<<RTK_RG_MAC_PORT1)|(1<<RTK_RG_MAC_PORT2)|(1<<RTK_RG_MAC_PORT3)|(1<<RTK_RG_MAC_PORT_PON)|(1<<RTK_RG_MAC_PORT_RGMII))
#define RTK_RG_ALL_EXT_PORTMASK ((1<<RTK_RG_EXT_PORT0)|(1<<RTK_RG_EXT_PORT1)|(1<<RTK_RG_EXT_PORT2)|(1<<RTK_RG_EXT_PORT3)|(1<<RTK_RG_EXT_PORT4))
#define RTK_RG_ALL_VIRUAL_PORTMASK (1|(1<<(RTK_RG_EXT_PORT0-RTK_RG_PORT_CPU))|(1<<(RTK_RG_EXT_PORT1-RTK_RG_PORT_CPU))|(1<<(RTK_RG_EXT_PORT2-RTK_RG_PORT_CPU))|(1<<(RTK_RG_EXT_PORT3-RTK_RG_PORT_CPU))|(1<<(RTK_RG_EXT_PORT4-RTK_RG_PORT_CPU)))
#define RTK_RG_ALL_USED_VIRUAL_PORTMASK (1|(1<<(RTK_RG_EXT_PORT0-RTK_RG_PORT_CPU))|(1<<(RTK_RG_EXT_PORT1-RTK_RG_PORT_CPU)))
#define RTK_RG_ALL_LAN_PORTMASK ((1<<RTK_RG_MAC_PORT0)|(1<<RTK_RG_MAC_PORT1)|(1<<RTK_RG_MAC_PORT2)|(1<<RTK_RG_MAC_PORT3))

#elif defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)

#define RTK_RG_MAX_MAC_PORT 7
#define RTK_RG_MAX_EXT_PORT (RTK_RG_PORT_MAX - RTK_RG_PORT_CPU)
#define RTK_RG_ALL_MAC_PORTMASK ((1<<RTK_RG_MAC_PORT0)|(1<<RTK_RG_MAC_PORT1)|(1<<RTK_RG_MAC_PORT2)|(1<<RTK_RG_MAC_PORT3)|(1<<RTK_RG_MAC_PORT_PON)|(1<<RTK_RG_MAC_PORT_RGMII)|(1<<RTK_RG_MAC_PORT_CPU))
#define RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU ((1<<RTK_RG_MAC_PORT0)|(1<<RTK_RG_MAC_PORT1)|(1<<RTK_RG_MAC_PORT2)|(1<<RTK_RG_MAC_PORT3)|(1<<RTK_RG_MAC_PORT_PON)|(1<<RTK_RG_MAC_PORT_RGMII))
#define RTK_RG_ALL_EXT_PORTMASK ((1<<RTK_RG_EXT_PORT0)|(1<<RTK_RG_EXT_PORT1)|(1<<RTK_RG_EXT_PORT2)|(1<<RTK_RG_EXT_PORT3)|(1<<RTK_RG_EXT_PORT4))
#define RTK_RG_ALL_VIRUAL_PORTMASK (1|(1<<(RTK_RG_EXT_PORT0-RTK_RG_PORT_CPU))|(1<<(RTK_RG_EXT_PORT1-RTK_RG_PORT_CPU))|(1<<(RTK_RG_EXT_PORT2-RTK_RG_PORT_CPU))|(1<<(RTK_RG_EXT_PORT3-RTK_RG_PORT_CPU))|(1<<(RTK_RG_EXT_PORT4-RTK_RG_PORT_CPU)))
#define RTK_RG_ALL_USED_VIRUAL_PORTMASK (1|(1<<(RTK_RG_EXT_PORT0-RTK_RG_PORT_CPU))|(1<<(RTK_RG_EXT_PORT1-RTK_RG_PORT_CPU)))
#define RTK_RG_ALL_LAN_PORTMASK ((1<<RTK_RG_MAC_PORT0)|(1<<RTK_RG_MAC_PORT1)|(1<<RTK_RG_MAC_PORT2)|(1<<RTK_RG_MAC_PORT3))


#endif

//#define MAX_ASIC_NXTHOP_TBL_SIZE 32
//#define MAX_ASIC_NETIF_TBL_SIZE 8



//20141219LUKE: used to transform egress WAN intf to ingress pattern
#define CONFIG_ACL_EGRESS_WAN_INTF_TRANSFORM 1

//NAPT filter for priority (pure software)
#define MAX_NAPT_FILER_SW_ENTRY_SIZE 128

//GPON Broadcast to Lan filter and REmarking
#define MAX_GPON_DS_BC_FILTER_SW_ENTRY_SIZE 128


/*StormControl*/
#define MAX_STORMCONTROL_ENTRY_SIZE 32

/* Queue weights structure */
#define RTK_RG_MAX_NUM_OF_QUEUE 8
#define RTK_RG_MAX_NUM_OF_GPON_QUEUE 128

/* SVLAN Module*/
/* SVlan */
#define SVLANMBRTBL_SIZE 		64		// 64 member config
#define SVLANMC2STBL_SIZE 		8		// 8 MC2S, Multicast to SVID
#define SVLANC2STBL_SIZE 		128		// 128 C2S, CVID/PVID to SVID
#define SVLANSP2CTBL_SIZE 		128		// 128 SP2C, SVID to CVID/PVID

/* ALG Module */
#define MAX_ALG_FUNCTIONS		32
//#define FTP_PORT_STR			0x504f5254		//LUKE20130816: move to separate ALG file
//#define FTP_PASV_RESP_STR		0x32323720		//LUKE20130816: move to separate ALG file
#define L2TP_CTRL_MSG_STR		0xc802
//#define MAX_FTP_CTRL_FLOW_SIZE	32			//LUKE20130816: move to separate ALG file
#define ALG_SRV_IN_LAN_IDX		8
#define MAX_L2TP_CTRL_FLOW_SIZE	32

#define ALG_RTSP_UPNP_TIMEOUT		300
#define ALG_H323_UPNP_TIMEOUT		300
#define ALG_FTP_PASV_UPNP_TIMEOUT	30		//secs
#define ALG_FTP_ACTIVE_UPNP_TIMEOUT	30		//secs
#define ALG_SIP_UPNP_TIMEOUT		30
#define ALG_BATTLENET_UPNP_TIMEOUT	30000

#define PPTP_CTRL_MSG_TYPE		1		// 1 for Control Message
#define PPTP_MAGIC				0x1a2b3c4d
#define MAX_PPTP_SESSION_SIZE	16
#define MAX_ALG_SERV_IN_LAN_NUM		8
#define MAX_ALG_DYNAMIC_PORT_NUM	256

#define CONFIG_RG_ROMEDRIVER_ALG_BATTLENET_SUPPORT	1
#define RTK_RG_MAX_SC_CLIENT	1

/* Table Database structure */
/* HW & SW Combine tables */
#define MAX_VLAN_SW_TABLE_SIZE	4096
#define MAX_VLAN_HW_TABLE_SIZE	4096
#define MAX_LUT_SW_LIST_SIZE	512		//used for access WAN limit

#if defined(CONFIG_RTL9602C_SERIES)
#define MAX_LUT_HW_TABLE_SIZE	1024+MAX_LUT_BCAM_TABLE_SIZE
#define MAX_IPMV6_L3MCR_TABL_SIZE 8 //each entry per port point to multicast routing translate table (for rtk_l34_ip6mcRoutingTransIdx_set/get)
#define MAX_LUT_SW_TABLE_SIZE	MAX_LUT_HW_TABLE_SIZE+MAX_LUT_SW_LIST_SIZE	//2048+MAX_LUT_BCAM_TABLE_SIZE+MAX_LUT_SW_LIST_SIZE
#elif defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
#define MAX_LUT_SW_TABLE_SIZE	((MAX_OF_RTL865x_L2TBL_ROW*RTL8651_L2TBL_COLUMN)+MAX_LUT_SW_LIST_SIZE)	//2048+MAX_LUT_SW_LIST_SIZE
#define MAX_LUT_HW_TABLE_SIZE	((RTL8651_L2TBL_ROW*RTL8651_L2TBL_COLUMN)+MAX_LUT_BCAM_TABLE_SIZE)
//#define MAX_LUT_HW_TABLE_SIZE	((2048)+MAX_LUT_BCAM_TABLE_SIZE)

#elif defined(CONFIG_RTL9600_SERIES)
#define MAX_LUT_SW_TABLE_SIZE	MAX_LUT_HW_TABLE_SIZE+MAX_LUT_SW_LIST_SIZE	//2048+MAX_LUT_BCAM_TABLE_SIZE+MAX_LUT_SW_LIST_SIZE
#define MAX_LUT_HW_TABLE_SIZE	2048+MAX_LUT_BCAM_TABLE_SIZE
#endif

#if defined(CONFIG_RTL9602C_SERIES)
#define MAX_NETIF_SW_TABLE_SIZE	16
#define MAX_NETIF_HW_TABLE_SIZE	16
#define MAX_L3_SW_TABLE_SIZE	16
#define MAX_L3_HW_TABLE_SIZE	16
#define MAX_ARP_SW_TABLE_SIZE	1024
#if defined(CONFIG_APOLLO_FPGA_PHY_TEST)
#define MAX_ARP_HW_TABLE_SIZE_FPGA	16
#define MAX_ARP_HW_TABLE_SIZE	32 //FPGA only 32 entries, but real IC is 128 entries.
#else
#define MAX_ARP_HW_TABLE_SIZE	128
#endif
#define MAX_DSLITE_SW_TABLE_SIZE	4
#define MAX_DSLITE_HW_TABLE_SIZE	4

#elif defined(CONFIG_RTL9600_SERIES) || defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
#define MAX_NETIF_SW_TABLE_SIZE	8
#define MAX_NETIF_HW_TABLE_SIZE	8
#define MAX_L3_SW_TABLE_SIZE	8
#define MAX_L3_HW_TABLE_SIZE	8
#define MAX_ARP_SW_TABLE_SIZE	1024
#define MAX_ARP_HW_TABLE_SIZE	512
#endif


#define MAX_PPPOE_SW_TABLE_SIZE	8
#define MAX_PPPOE_HW_TABLE_SIZE	8
#define MAX_EXTIP_HW_TABLE_SIZE 8
#define MAX_EXTIP_SW_TABLE_SIZE 8
#define MAX_NEXTHOP_SW_TABLE_SIZE	16
#define MAX_NEXTHOP_HW_TABLE_SIZE	16
#define MAX_DSLITEMC_SW_TABLE_SIZE	4
#define MAX_DSLITEMC_HW_TABLE_SIZE	4



#if defined(CONFIG_RTL9602C_SERIES) || defined(CONFIG_RTL9600_SERIES)
#ifdef CONFIG_RG_SW_NAPT_SIZE_L
#define MAX_NAPT_IN_SW_TABLE_SHIFT   15
#else
#if defined(CONFIG_RTL9602C_SERIES)
#define MAX_NAPT_IN_SW_TABLE_SHIFT   13
#elif defined(CONFIG_RTL9600_SERIES)
#define MAX_NAPT_IN_SW_TABLE_SHIFT   12
#endif
#endif

#define MAX_NAPT_IN_SW_TABLE_SIZE	(1 << MAX_NAPT_IN_SW_TABLE_SHIFT)


#if defined(CONFIG_RTL9602C_SERIES)
#define MAX_NAPT_IN_HW_TABLE_SHIFT	12
#elif defined(CONFIG_RTL9600_SERIES)
#define MAX_NAPT_IN_HW_TABLE_SHIFT	10
#endif
#define MAX_NAPT_IN_HW_TABLE_SIZE	(1 << MAX_NAPT_IN_HW_TABLE_SHIFT)


#ifdef CONFIG_RG_SW_NAPT_SIZE_L
#define MAX_NAPT_OUT_SW_TABLE_SHIFT   15
#else
#if defined(CONFIG_RTL9602C_SERIES)
#define MAX_NAPT_OUT_SW_TABLE_SHIFT   13
#elif defined(CONFIG_RTL9600_SERIES)
#define MAX_NAPT_OUT_SW_TABLE_SHIFT   12

#endif
#endif

#define MAX_NAPT_OUT_SW_TABLE_SIZE	(1 << MAX_NAPT_OUT_SW_TABLE_SHIFT)

#if defined(CONFIG_RTL9602C_SERIES)
#define MAX_NAPT_OUT_HW_TABLE_SIZE	4096
#define MAX_SHAREMETER_TABLE_SIZE 16
#elif defined(CONFIG_RTL9600_SERIES)
#define MAX_NAPT_OUT_HW_TABLE_SIZE	2048
#define MAX_SHAREMETER_TABLE_SIZE 32
#endif


#ifdef CONFIG_RG_SW_NAPT_SIZE_L
#if MAX_NAPT_OUT_SW_TABLE_SIZE > 65536
#error "Out of FwdEngine NAPT flow size"
#endif
#else
#if MAX_NAPT_OUT_SW_TABLE_SIZE > 32767
#error "Out of FwdEngine NAPT flow size"
#endif
#endif

#if MAX_NAPT_OUT_HW_TABLE_SIZE > MAX_NAPT_OUT_SW_TABLE_SIZE
#error "NAPT SW/HW table size configure error!"
#endif

#elif defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
#define MAX_NAPT_IN_SW_TABLE_SHIFT   12
#define MAX_NAPT_IN_SW_TABLE_SIZE	(1 << MAX_NAPT_IN_SW_TABLE_SHIFT)

#define MAX_NAPT_IN_HW_TABLE_SHIFT	10
#define MAX_NAPT_IN_HW_TABLE_SIZE	(1 << MAX_NAPT_IN_HW_TABLE_SHIFT)

#define MAX_NAPT_OUT_SW_TABLE_SHIFT   12
#define MAX_NAPT_OUT_SW_TABLE_SIZE	(1 << MAX_NAPT_OUT_SW_TABLE_SHIFT)

#define MAX_NAPT_OUT_HW_TABLE_SHIFT	10
#define MAX_NAPT_OUT_HW_TABLE_SIZE	(1 << MAX_NAPT_OUT_HW_TABLE_SHIFT)


#define MAX_SHAREMETER_TABLE_SIZE 32
#if MAX_NAPT_IN_HW_TABLE_SIZE != 1024
#error "NAPT-R H/W table size error!"
#endif
#if MAX_NAPT_OUT_SW_TABLE_SIZE > 32767
#error "Out of FwdEngine NAPT flow size"
#endif
#if MAX_NAPT_OUT_HW_TABLE_SIZE != 1024
#error "NAPT H/W table size error!"
#endif
#if MAX_NAPT_OUT_HW_TABLE_SIZE > MAX_NAPT_OUT_SW_TABLE_SIZE
#error "NAPT SW/HW table size configure error!"
#endif

#endif

#define MAX_BIND_SW_TABLE_SIZE	32
#define MAX_BIND_HW_TABLE_SIZE	32
#if defined(CONFIG_RTL9602C_SERIES)
#define MAX_WANTYPE_SW_TABLE_SIZE	16
#define MAX_WANTYPE_HW_TABLE_SIZE	16
#elif defined(CONFIG_RTL9600_SERIES) || defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
#define MAX_WANTYPE_SW_TABLE_SIZE	8
#define MAX_WANTYPE_HW_TABLE_SIZE	8
#endif


#if defined(CONFIG_RTL9602C_SERIES)
#define MAX_IPV6_ROUTING_HW_TABLE_SIZE 8
#define MAX_IPMCGRP_HW_TABLE_SIZE 0	  //not support
#define MAX_IPMCFILTER_HW_TABLE_SIZE 16
#elif defined(CONFIG_RTL9600_SERIES)
#define MAX_IPV6_ROUTING_HW_TABLE_SIZE 4
#define MAX_IPMCGRP_HW_TABLE_SIZE 64
#define MAX_IPMCFILTER_HW_TABLE_SIZE 0 //not support
#elif defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
#define MAX_IPV6_ROUTING_HW_TABLE_SIZE 8
#define MAX_IPMCGRP_HW_TABLE_SIZE 0    //not support
#define MAX_IPMCFILTER_HW_TABLE_SIZE 0 //not support
#endif

/* for shortcut bit-field width
 *
 * if you modify software table size in rg_db, you also need to update
 *   SC_BFW_NAPTIDX
 *   SC_BFW_INTFIDX
 *   SC_BFW_EIPIDX
 *   SC_BFW_LUTIDX
 *   SC_BFW_ARPIDX
*/
//#define SHORTCUT_BITFILED_DEBUG

#define SC_BFW_SPA 4
#define SC_BFW_DIR 3
#define SC_BFW_ISTCP 1
#define SC_BFW_ISNAPT 1
#define SC_BFW_ISBRIDGE 1 
#define SC_BFW_ISHAIRPINNAT 1
#define SC_BFW_NOTFINUPDATE 1
#define SC_BFW_VLANTAGIF 1
#define SC_BFW_SVLANTAGIF 2
#define SC_BFW_DMAC2CVLANTAGIF 1
#define SC_BFW_TAGACCTYPE 1
#define SC_BFW_INTERVLANID 13
#define SC_BFW_VLANID 13
#define SC_BFW_DMAC2CVLANID 13
#define SC_BFW_DSCP 6
#define SC_BFW_PRIORITY 3
#define SC_BFW_SPRIORITY 3
#define SC_BFW_INTERCFPRI 3
#define SC_BFW_SVLANID 13
#define SC_BFW_STREAMID 7
#define SC_BFW_SMACL2IDX 13
#define SC_BFW_MACPORT 4
#define SC_BFW_EXTPORT 4
#define SC_BFW_NEIGHBORIDX 8
#define SC_BFW_IDLETIME 16
#if defined(CONFIG_RTL9602C_SERIES)

#define SC_BFW_NAPTIDX 14
#define SC_BFW_INTFIDX 5
#define SC_BFW_EIPIDX 4
#define SC_BFW_LUTIDX 12
#define SC_BFW_ARPIDX 11

#define SC_BFW_MIBDIRECT 2
#define SC_BFW_MIBIFIDX 5
#define SC_BFW_MIBTAGDELTA 4
#define SC_BFW_UNIPORTMASK 8

#elif defined(CONFIG_RTL9600_SERIES)

#define SC_BFW_NAPTIDX 16
#define SC_BFW_INTFIDX 4
#define SC_BFW_EIPIDX 4
#define SC_BFW_LUTIDX 13 
#define SC_BFW_ARPIDX 11
#define SC_BFW_UNIPORTMASK 8

#elif defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
// do nothing

#endif

/* for napt table entry width
 *
*/

//#define NAPT_TABLE_SIZE_DEBUG


// rtk_rg_table_naptIn_t

#define MAX_NAPT_SW_ENTRY_WAYS_SHIFT    2
#define MAX_NAPT_IN_SW_ENTRY_WIDTH      (MAX_NAPT_IN_SW_TABLE_SHIFT-MAX_NAPT_SW_ENTRY_WAYS_SHIFT)
#define MAX_NAPT_IN_IDLESEC_WIDTH       12
#define MAX_NAPT_IN_REFCOUNT_WIDTH      1
#define MAX_NAPT_IN_CONETYPE_WIDTH      2
#define MAX_NAPT_IN_NAPTOUTIDX_WIDTH    MAX_NAPT_OUT_SW_TABLE_SHIFT
#define MAX_NAPT_IN_CANBEREPLACE_WIDTH  1
#define MAX_NAPT_IN_PRIVALID_WIDTH      1
#define MAX_NAPT_IN_PRIVALUE_WIDTH      3
#define MAX_NAPT_IN_CANTADDTOHW_WIDTH   1   

// rtk_rg_table_naptOut_t
#define MAX_NAPT_OUT_SW_ENTRY_WIDTH     (MAX_NAPT_OUT_SW_TABLE_SHIFT-MAX_NAPT_SW_ENTRY_WAYS_SHIFT)
#define MAX_NAPT_OUT_STATE_WIDTH        4
#define MAX_NAPT_OUT_PORT_WIDTH         16
#define MAX_NAPT_OUT_IDLESEC_WIDTH      12
#define MAX_NAPT_OUT_CANTADDTOHW_WIDTH  1   
#define MAX_NAPT_OUT_PRIVALID_WIDTH     1
#define MAX_NAPT_OUT_PRIVALUE_WIDTH     3
#define MAX_NAPT_OUT_CANBEREPLACE_WIDTH 1



#ifdef CONFIG_RG_IPV6_NAPT_SUPPORT
#define MAX_IPV6_ROUTING_SW_TABLE_SIZE (MAX_IPV6_ROUTING_HW_TABLE_SIZE<<1)
#else
#define MAX_IPV6_ROUTING_SW_TABLE_SIZE (MAX_IPV6_ROUTING_HW_TABLE_SIZE)
#endif


#define MAX_IPV6_NEIGHBOR_SW_TABLE_SIZE 128
#define MAX_IPV6_NEIGHBOR_HW_TABLE_SIZE 128

#define MAX_WLAN_MBSSID_SW_TABLE_SIZE 64
#if defined(CONFIG_RTL9602C_SERIES)
#define MAX_LUT_BCAM_TABLE_SIZE	64
#if defined(CONFIG_APOLLO_FPGA_PHY_TEST)
#define MAX_LUT_BCAM_TABLE_SIZE_FPGA 8
#endif
#elif defined(CONFIG_RTL9600_SERIES)
#define MAX_LUT_BCAM_TABLE_SIZE	64
#elif defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
#define MAX_LUT_BCAM_TABLE_SIZE	0
#endif
#ifdef CONFIG_RTL_CLIENT_MODE_SUPPORT
#if defined(CONFIG_RG_WLAN_HWNAT_ACCELERATION) && !defined(CONFIG_ARCH_LUNA_SLAVE)
#define MAX_WLAN_DEVICE_NUM	28		//root(1)+vap(4)+wds(8)+vxd(1), support dualband at one cpu
#else
#define MAX_WLAN_DEVICE_NUM	14		//root(1)+vap(4)+wds(8)+vxd(1)
#endif
#define WLAN_DEVICE_NUM		14

#else
#if defined(CONFIG_RG_WLAN_HWNAT_ACCELERATION) && !defined(CONFIG_ARCH_LUNA_SLAVE)
#define MAX_WLAN_DEVICE_NUM	26	//root(1)+vap(4)+wds(8), support dualband at one cpu
#else
#define MAX_WLAN_DEVICE_NUM	13	//root(1)+vap(4)+wds(8)
#endif
#define WLAN_DEVICE_NUM		13

#endif

/* Software tables */
#define MAX_VIRTUAL_SERVER_SW_TABLE_SIZE	32
#define MAX_UPNP_SW_TABLE_SIZE				32
#define MAX_DMZ_TABLE_SIZE					MAX_NETIF_SW_TABLE_SIZE
#define MAX_GATEWAYSERVICEPORT_TABLE_SIZE	32
#define MAX_IPV4_FRAGMENT_QUEUE_SIZE 		128		//TCP+UDP+ICMP

#ifdef CONFIG_RG_SW_NAPT_SIZE_L
#define MAX_NAPT_SHORTCUT_SIZE_SHFIT		16 //65536
#else
#if defined(CONFIG_RTL9602C_SERIES)
#define MAX_NAPT_SHORTCUT_SIZE_SHFIT		14 //16384
#elif defined(CONFIG_RTL9600_SERIES)
#define MAX_NAPT_SHORTCUT_SIZE_SHFIT		13 //8192
#elif defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
#define MAX_NAPT_SHORTCUT_SIZE_SHFIT		13 //8192
#endif
#endif
#define MAX_NAPT_SHORTCUT_SIZE 				(1<<MAX_NAPT_SHORTCUT_SIZE_SHFIT)  	//MAX_NAPT_OUT_SW_TABLE_SIZE*2
#define MAX_NAPT_SHORTCUT_WAYS_SHIFT 		2  		//must power of 2
#define MAX_NAPT_SHORTCUT_WAYS 				(1<<MAX_NAPT_SHORTCUT_WAYS_SHIFT) // 2==>4
#define FWD_SHORTCUT_BIT_NUM				(MAX_NAPT_SHORTCUT_SIZE_SHFIT-MAX_NAPT_SHORTCUT_WAYS_SHIFT)		//bit num of MAX_NAPT_SHORTCUT_SIZE


#define MAX_NAPT_V6_SHORTCUT_SIZE_SHFIT		10 //1024
#define MAX_NAPT_V6_SHORTCUT_SIZE			(1<<MAX_NAPT_V6_SHORTCUT_SIZE_SHFIT) //1024
#define MAX_NAPT_V6_SHORTCUT_WAYS_SHIFT 	2  		//must power of 2
#define MAX_NAPT_V6_SHORTCUT_WAYS 			(1<<MAX_NAPT_V6_SHORTCUT_WAYS_SHIFT) // 2==>4
#define FWD_V6_SHORTCUT_BIT_NUM				(MAX_NAPT_V6_SHORTCUT_SIZE_SHFIT-MAX_NAPT_V6_SHORTCUT_WAYS_SHIFT)		//bit num of MAX_NAPT_V6_SHORTCUT_SIZE


#define MAX_ICMPCTRLFLOW_SIZE				32
#define MAX_FRAG_IN_FREE_TABLE_SIZE			128
#define MAX_FRAG_OUT_FREE_TABLE_SIZE		128
#define MAX_ARP_SW_TABLE_HEAD				256
#if defined(CONFIG_RTL9602C_SERIES)
#define MAX_ARP_HW_TABLE_HEAD				256
#endif
#define MAX_LUT_SW_TABLE_HEAD				512
#define ARP_SW_TABLE_THRESHOLD				180
#if defined(CONFIG_RTL9600_SERIES) || defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
#define MAX_ARP_FOR_LAN_INTF				256		//preserve 256 entries in hw table for LAN intfs
#define MAX_ARP_FOR_WAN_INTF				256 	//preserve 256 entries in hw table for WAN intfs
#elif defined(CONFIG_RTL9602C_SERIES)
#define MAX_ARP_FOR_LAN_INTF				128		//preserve 128 entries in hw table for LAN intfs (shared with LAN & WAN)
#define MAX_ARP_FOR_WAN_INTF				128 	//preserve 128 entries in hw table for WAN intfs (shared with LAN & WAN)
#endif

#define MAX_FRAGMENT_QUEUE_THRESHOLD		50		//if there are so many packets need to queue, the first packet may not come in anymore, so kick they all
#define DEF_SOFTWARE_LEARNING_LIMIT			-1		//default software lut learning limit
#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
#define v6StatefulHOUSE_KEEP_NUM			512		//how many entries should be check at once, MUST be the common divisor of list number
#define v6StatefulHOUSE_THRESHOLD			1024	//when the active connection is over this threshold, the timeout check function will be STATIC period(at most v6StatefulHOUSE_KEEP_NUM each time)
#define MAX_IPV6_STATEFUL_TABLE_SIZE		2048
//#define MAX_IPV6_STATEFUL_TABLE_SIZE		40		//test mode for LRU
#define MAX_IPV6_STATEFUL_HASH_HEAD_SIZE	256
#define MAX_IPV6_FRAGMENT_QUEUE_SIZE 		128
#endif
#define MAX_CONGESTION_CTRL_RING_SIZE		1200
#define MAX_CONGESTION_CTRL_PORTS			5
#define MAX_URL_FILTER_ENTRY_SIZE 			100
#define MAX_URL_FILTER_BUF_LENGTH 			1024
#define MAX_URL_FILTER_STR_LENGTH			128
#define MAX_URL_FILTER_PATH_LENGTH			256
#define MAX_MAC_FILTER_ENTRY_SIZE 			30
#define MAX_IPMC_IP_LIST 					8
#define MAX_FORCE_PORTAL_URL_NUM			8
#define MAX_REDIRECT_PUSH_WEB_SIZE			728
#define MAX_REDIRECT_URL_NUM				8
#define MAX_REDIRECT_WHITE_LIST_NUM			8
#if defined(CONFIG_APOLLO_GPON_FPGATEST)
#define MAX_VMAC_SKB_QUEUE_SIZE				32
#endif
#define MAX_LANNET_DEV_NAME_LENGTH			32
#define MAX_LANNET_BRAND_NAME_LENGTH		16
#define MAX_LANNET_OS_NAME_LENGTH			16
#define MAX_LANNET_SUB_BRAND_SIZE			10
#define MAX_LANNET_SUB_OS_SIZE				10

#if defined(CONFIG_RTL9600_SERIES) || defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
#define MAX_SHARE_METER_SIZE				32
#define MAX_WIFI_LIMIT_RATE					0xffff8	//0x1ffff << 3
#elif defined(CONFIG_RTL9602C_SERIES)
#define MAX_SHARE_METER_SIZE				16
#define MAX_WIFI_LIMIT_RATE					0x3ffff8	//0x7ffff << 3
#endif


#define TC2DATA	0xb8003220
#define TC2CNT	0xb8003224
#define TC2CTL	0xb8003228
#define TC2INT	0xb800322c
#define BSP_TC2_IRQ	45


/* IGMP/MLD snooping */
#define RTK_RG_MULTICAST_MODE_MACFID 0

/* USERSPACE HELPER */
#define CB_MAX_FILE_SIZE 1024
#define CB_MAX_FILE_LINE 10
#define CB_CMD_BUFF_SIZE 256

/* common */
#ifndef NULL
#define NULL 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef SUCCESS
#define SUCCESS 0
#endif

#ifndef FAIL
#define FAIL -1
#endif

#define IDX_UNHIT_DROP	-1
#define IDX_UNHIT_PASS	0

extern char mt_watch_tmp[512];


void assert_ok_sub_func(int assert_ret, char *assert_str,const char *func,int line);
void assert_sub_func(int assert_ret, char *assert_str,const char *func,int line);

#if 0
#ifndef assert_ok
#define assert_ok(x)\
{ int assert_ret;\
	assert_ret=(x);\
	if(assert_ret!=0) { \
		if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_WARN) {\
        	rtlglue_printf(assert_ok_str,#x,assert_ret,__FUNCTION__,__LINE__);\
		}\
	}\
}
#endif


#ifndef assert
#define assert(x)\
if(!(x)) {\
	if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_WARN) {\
		rtlglue_printf(assert_str, #x,__FUNCTION__,__LINE__);\
	}\
}
#endif

#else
#ifndef assert_ok 
#define assert_ok(x) do { assert_ok_sub_func(x,#x,__FUNCTION__,__LINE__); } while(0)
#endif

#ifndef assert 
#define assert(x) do { assert_sub_func(x,#x,__FUNCTION__,__LINE__); } while(0)
#endif

#endif


/* debug */

#if defined(CONFIG_RG_DEBUG)


#ifndef DUMP_PACKET
#define DUMP_PACKET(pkt,size,memo) dump_packet(pkt,size,memo)
#endif


#ifndef DEBUG
#define DEBUG( comment ,arg...) do { if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_DEBUG) common_dump(RTK_RG_DEBUG_LEVEL_DEBUG,__FUNCTION__,__LINE__,comment,##arg); }while(0)
#endif

#ifndef FIXME
#define FIXME( comment ,arg...) do { if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_FIXME) common_dump(RTK_RG_DEBUG_LEVEL_FIXME,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef CBACK
#define CBACK( comment ,arg...) do { if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_CALLBACK) common_dump(RTK_RG_DEBUG_LEVEL_CALLBACK,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef TRACE
#define TRACE( comment ,arg...)	do { if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_TRACE) common_dump(RTK_RG_DEBUG_LEVEL_TRACE,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef ACL
#define ACL( comment ,arg...) do { if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_ACL) common_dump(RTK_RG_DEBUG_LEVEL_ACL,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef ACL_RSV
#define ACL_RSV( comment ,arg...) do { if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_ACL_RRESERVED) common_dump(RTK_RG_DEBUG_LEVEL_ACL_RRESERVED,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif


#ifndef WARNING
#define WARNING( comment ,arg...) do { if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_WARN) common_dump(RTK_RG_DEBUG_LEVEL_WARN,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef WMUX
#define WMUX( comment ,arg...) do { if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_WMUX) common_dump(RTK_RG_DEBUG_LEVEL_WMUX,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef MACLN
#define MACLN( comment ,arg...) do { if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_MACLEARN) common_dump(RTK_RG_DEBUG_LEVEL_MACLEARN,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef TABLE
#define TABLE( comment ,arg...) do { if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_TABLE) common_dump(RTK_RG_DEBUG_LEVEL_TABLE,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef ALG
#define ALG( comment ,arg...) do { if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_ALG) common_dump(RTK_RG_DEBUG_LEVEL_ALG,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef IGMP
#define IGMP( comment ,arg...) do { if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_IGMP) common_dump(RTK_RG_DEBUG_LEVEL_IGMP,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef RGAPI
#define RGAPI( comment ,arg...) do { if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_RG_API) common_dump(RTK_RG_DEBUG_LEVEL_RG_API,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif



#else //!defined(CONFIG_RG_DEBUG)

#ifndef assert
#define assert(x) 
#endif


#ifndef COMMON_DUMP
#define COMMON_DUMP( bitmask, string, color,bgcolor,comment ,arg...) 
#endif

#ifndef DEBUG
#define DEBUG( comment ,arg...) 
#endif

#ifndef FIXME
#define FIXME( comment ,arg...) 
#endif

#ifndef CBACK
#define CBACK( comment ,arg...) 
#endif


#ifndef TRACE
#define TRACE( comment ,arg...) 
#endif

#ifndef ACL
#define ACL( comment ,arg...) 
#endif

#ifndef ACL_RSV
#define ACL_RSV( comment ,arg...) 
#endif


#ifndef WARNING
#define WARNING( comment ,arg...)
#endif

#ifndef WMUX
#define WMUX( comment ,arg...)
#endif

#ifndef MACLN
#define MACLN( comment ,arg...) 
#endif

#ifndef TABLE
#define TABLE( comment ,arg...)
#endif

#ifndef ALG
#define ALG( comment ,arg...) 
#endif

#ifndef IGMP
#define IGMP( comment ,arg...)
#endif

#ifndef RGAPI
#define RGAPI( comment ,arg...) 
#endif



#ifndef DUMP_PACKET
#define DUMP_PACKET(pkt,size,memo) 
#endif


#endif

#if 0
#define RETURN_ERR(err_name) \
	do{\
		if(err_name!=RT_ERR_RG_OK) \
		{\
			extern char StringErrName[];\
			if(memcmp(#err_name,"RTK_ERR_RG_",11)==0)\
				strcpy(StringErrName,#err_name);\
			else\
			{\
				_rtk_rg_error_id_mapping(err_name,StringErrName);\
			}\
			WARNING("Return Error (0x%x:%s)",err_name,StringErrName);\
		}\
		return err_name;\
	}while(0)
#else	

#define RETURN_ERR(err_name)  do { return return_err_mapping(err_name,#err_name,__LINE__); } while(0)

#endif

#if defined(CONFIG_APOLLO)
#undef PROFILE
#define PROFILE int profile_idx; do { profile_idx=profile(__FUNCTION__); }while(0)
#define PROFILE_END do { profile_end(profile_idx); }while(0)

#define PROFILE_COUNT_START \
	do{\
		if(rg_kernel.cp3_execute_count>0){\
			if(!rg_kernel.cp3_execute_count_state)\
				rg_kernel.cp3_execute_count_state=profile(__FUNCTION__);\
			rg_kernel.cp3_execute_count--;\
		}\
	}while(0)
#define PROFILE_COUNT_END \
	do{\
		if(rg_kernel.cp3_execute_count_state && rg_kernel.cp3_execute_count==0){\
			profile_end(rg_kernel.cp3_execute_count_state);\
			rg_kernel.cp3_execute_count_state=0;\
		}\
	}while(0)
#endif




#ifdef CONFIG_APOLLO
// L2 Switch: RTK APIs --> RG APIs
//from stp.h
#include<rtk/stp.h>
#define rtk_rg_stp_init(arg,args...) rtk_stp_init(arg,##args)
#define rtk_rg_stp_mstpState_get(arg,args...) rtk_stp_mstpState_get(arg,##args)
#define rtk_rg_stp_mstpState_set(arg,args...) rtk_stp_mstpState_set(arg,##args)


//from module/gpon/gpon.h
#include<module/gpon/gpon.h>
#define rtk_rg_gpon_activate(arg,args...) rtk_gpon_activate(arg,##args)
#define rtk_rg_gpon_aesKeySwitch_get(arg,args...) rtk_gpon_aesKeySwitch_get(arg,##args)
#define rtk_rg_gpon_aesKeySwitch_set(arg,args...) rtk_gpon_aesKeySwitch_set(arg,##args)
#define rtk_rg_gpon_aesKeyWord_set(arg,args...) rtk_gpon_aesKeyWord_set(arg,##args)
#define rtk_rg_gpon_alarmStatus_get(arg,args...) rtk_gpon_alarmStatus_get(arg,##args)
#define rtk_rg_gpon_autoDisTx_set(arg,args...) rtk_gpon_autoDisTx_set(arg,##args)
#define rtk_rg_gpon_berInterval_get(arg,args...) rtk_gpon_berInterval_get(arg,##args)
#define rtk_rg_gpon_burstOverhead_set(arg,args...) rtk_gpon_burstOverhead_set(arg,##args)
#define rtk_rg_gpon_cdrLosStatus_get(arg,args...) rtk_gpon_cdrLosStatus_get(arg,##args)
#define rtk_rg_gpon_dataPath_reset(arg,args...) rtk_gpon_dataPath_reset(arg,##args)
#define rtk_rg_gpon_dbruPeriod_get(arg,args...) rtk_gpon_dbruPeriod_get(arg,##args)
#define rtk_rg_gpon_dbruPeriod_set(arg,args...) rtk_gpon_dbruPeriod_set(arg,##args)
#define rtk_rg_gpon_dbruState_get(arg,args...) rtk_gpon_dbruState_get(arg,##args)
#define rtk_rg_gpon_dbruState_set(arg,args...) rtk_gpon_dbruState_set(arg,##args)
#define rtk_rg_gpon_deActivate(arg,args...) rtk_gpon_deActivate(arg,##args)
#define rtk_rg_gpon_drainOutDefaultQueue_set(arg,args...) rtk_gpon_drainOutDefaultQueue_set(arg,##args)
#define rtk_rg_gpon_dsBwmapCrcCheckState_get(arg,args...) rtk_gpon_dsBwmapCrcCheckState_get(arg,##args)
#define rtk_rg_gpon_dsBwmapCrcCheckState_set(arg,args...) rtk_gpon_dsBwmapCrcCheckState_set(arg,##args)
#define rtk_rg_gpon_dsBwmapFilterOnuIdState_get(arg,args...) rtk_gpon_dsBwmapFilterOnuIdState_get(arg,##args)
#define rtk_rg_gpon_dsBwmapFilterOnuIdState_set(arg,args...) rtk_gpon_dsBwmapFilterOnuIdState_set(arg,##args)
#define rtk_rg_gpon_dsEthPti_get(arg,args...) rtk_gpon_dsEthPti_get(arg,##args)
#define rtk_rg_gpon_dsEthPti_set(arg,args...) rtk_gpon_dsEthPti_set(arg,##args)
#define rtk_rg_gpon_dsFecBypass_get(arg,args...) rtk_gpon_dsFecBypass_get(arg,##args)
#define rtk_rg_gpon_dsFecBypass_set(arg,args...) rtk_gpon_dsFecBypass_set(arg,##args)
#define rtk_rg_gpon_dsFecThrd_get(arg,args...) rtk_gpon_dsFecThrd_get(arg,##args)
#define rtk_rg_gpon_dsFecThrd_set(arg,args...) rtk_gpon_dsFecThrd_set(arg,##args)
#define rtk_rg_gpon_usFecSts_get(arg,args...) rtk_gpon_usFecSts_get(arg,##args)
#define rtk_rg_gpon_dsFecSts_get(arg,args...) rtk_gpon_dsFecSts_get(arg,##args)
#define rtk_rg_gpon_dsFlow_get(arg,args...) rtk_gpon_dsFlow_get(arg,##args)
#define rtk_rg_gpon_dsFlow_set(arg,args...) rtk_gpon_dsFlow_set(arg,##args)
#define rtk_rg_gpon_dsGemPortBcPassState_get(arg,args...) rtk_gpon_dsGemPortBcPassState_get(arg,##args)
#define rtk_rg_gpon_dsGemPortBcPassState_set(arg,args...) rtk_gpon_dsGemPortBcPassState_set(arg,##args)
#define rtk_rg_gpon_dsGemPortByteCnt_get(arg,args...) rtk_gpon_dsGemPortByteCnt_get(arg,##args)
#define rtk_rg_gpon_dsGemPortEthFwdCnt_get(arg,args...) rtk_gpon_dsGemPortEthFwdCnt_get(arg,##args)
#define rtk_rg_gpon_dsGemPortEthRxCnt_get(arg,args...) rtk_gpon_dsGemPortEthRxCnt_get(arg,##args)
#define rtk_rg_gpon_dsGemPortFcsCheckState_get(arg,args...) rtk_gpon_dsGemPortFcsCheckState_get(arg,##args)
#define rtk_rg_gpon_dsGemPortFcsCheckState_set(arg,args...) rtk_gpon_dsGemPortFcsCheckState_set(arg,##args)
#define rtk_rg_gpon_dsGemPortFrameTimeOut_get(arg,args...) rtk_gpon_dsGemPortFrameTimeOut_get(arg,##args)
#define rtk_rg_gpon_dsGemPortFrameTimeOut_set(arg,args...) rtk_gpon_dsGemPortFrameTimeOut_set(arg,##args)
#define rtk_rg_gpon_dsGemPortMacEntry_del(arg,args...) rtk_gpon_dsGemPortMacEntry_del(arg,##args)
#define rtk_rg_gpon_dsGemPortMacEntry_get(arg,args...) rtk_gpon_dsGemPortMacEntry_get(arg,##args)
#define rtk_rg_gpon_dsGemPortMacEntry_set(arg,args...) rtk_gpon_dsGemPortMacEntry_set(arg,##args)
#define rtk_rg_gpon_dsGemPortMacFilterMode_get(arg,args...) rtk_gpon_dsGemPortMacFilterMode_get(arg,##args)
#define rtk_rg_gpon_dsGemPortMacFilterMode_set(arg,args...) rtk_gpon_dsGemPortMacFilterMode_set(arg,##args)
#define rtk_rg_gpon_dsGemPortMacForceMode_get(arg,args...) rtk_gpon_dsGemPortMacForceMode_get(arg,##args)
#define rtk_rg_gpon_dsGemPortMacForceMode_set(arg,args...) rtk_gpon_dsGemPortMacForceMode_set(arg,##args)
#define rtk_rg_gpon_dsGemPortMiscCnt_get(arg,args...) rtk_gpon_dsGemPortMiscCnt_get(arg,##args)
#define rtk_rg_gpon_dsGemPortNonMcPassState_get(arg,args...) rtk_gpon_dsGemPortNonMcPassState_get(arg,##args)
#define rtk_rg_gpon_dsGemPortNonMcPassState_set(arg,args...) rtk_gpon_dsGemPortNonMcPassState_set(arg,##args)
#define rtk_rg_gpon_dsGemPortPktCnt_get(arg,args...) rtk_gpon_dsGemPortPktCnt_get(arg,##args)
#define rtk_rg_gpon_dsGemPort_del(arg,args...) rtk_gpon_dsGemPort_del(arg,##args)
#define rtk_rg_gpon_dsGemPort_get(arg,args...) rtk_gpon_dsGemPort_get(arg,##args)
#define rtk_rg_gpon_dsGemPort_set(arg,args...) rtk_gpon_dsGemPort_set(arg,##args)
#define rtk_rg_gpon_dsGtcMiscCnt_get(arg,args...) rtk_gpon_dsGtcMiscCnt_get(arg,##args)
#define rtk_rg_gpon_dsOmciCnt_get(arg,args...) rtk_gpon_dsOmciCnt_get(arg,##args)
#define rtk_rg_gpon_dsOmciPti_get(arg,args...) rtk_gpon_dsOmciPti_get(arg,##args)
#define rtk_rg_gpon_dsOmciPti_set(arg,args...) rtk_gpon_dsOmciPti_set(arg,##args)
#define rtk_rg_gpon_dsPlendStrictMode_get(arg,args...) rtk_gpon_dsPlendStrictMode_get(arg,##args)
#define rtk_rg_gpon_dsPlendStrictMode_set(arg,args...) rtk_gpon_dsPlendStrictMode_set(arg,##args)
#define rtk_rg_gpon_dsPloamBcAcceptState_get(arg,args...) rtk_gpon_dsPloamBcAcceptState_get(arg,##args)
#define rtk_rg_gpon_dsPloamBcAcceptState_set(arg,args...) rtk_gpon_dsPloamBcAcceptState_set(arg,##args)
#define rtk_rg_gpon_dsPloamDropCrcState_get(arg,args...) rtk_gpon_dsPloamDropCrcState_get(arg,##args)
#define rtk_rg_gpon_dsPloamDropCrcState_set(arg,args...) rtk_gpon_dsPloamDropCrcState_set(arg,##args)
#define rtk_rg_gpon_dsPloamNomsg_get(arg,args...) rtk_gpon_dsPloamNomsg_get(arg,##args)
#define rtk_rg_gpon_dsPloamNomsg_set(arg,args...) rtk_gpon_dsPloamNomsg_set(arg,##args)
#define rtk_rg_gpon_dsPloamOnuIdFilterState_get(arg,args...) rtk_gpon_dsPloamOnuIdFilterState_get(arg,##args)
#define rtk_rg_gpon_dsPloamOnuIdFilterState_set(arg,args...) rtk_gpon_dsPloamOnuIdFilterState_set(arg,##args)
#define rtk_rg_gpon_dsPloam_get(arg,args...) rtk_gpon_dsPloam_get(arg,##args)
#define rtk_rg_gpon_dsScrambleState_get(arg,args...) rtk_gpon_dsScrambleState_get(arg,##args)
#define rtk_rg_gpon_dsScrambleState_set(arg,args...) rtk_gpon_dsScrambleState_set(arg,##args)
#define rtk_rg_gpon_eqd_set(arg,args...) rtk_gpon_eqd_set(arg,##args)
#define rtk_rg_gpon_extraSnTxTimes_get(arg,args...) rtk_gpon_extraSnTxTimes_get(arg,##args)
#define rtk_rg_gpon_extraSnTxTimes_set(arg,args...) rtk_gpon_extraSnTxTimes_set(arg,##args)
#define rtk_rg_gpon_flowCounter_get(arg,args...) rtk_gpon_flowCounter_get(arg,##args)
#define rtk_rg_gpon_forceLaser_get(arg,args...) rtk_gpon_forceLaser_get(arg,##args)
#define rtk_rg_gpon_forceLaser_set(arg,args...) rtk_gpon_forceLaser_set(arg,##args)
#define rtk_rg_gpon_gemUsDataByteCnt_get(arg,args...) rtk_gpon_gemUsDataByteCnt_get(arg,##args)
#define rtk_rg_gpon_gemUsEthCnt_get(arg,args...) rtk_gpon_gemUsEthCnt_get(arg,##args)
#define rtk_rg_gpon_gemUsForceIdleState_get(arg,args...) rtk_gpon_gemUsForceIdleState_get(arg,##args)
#define rtk_rg_gpon_gemUsForceIdleState_set(arg,args...) rtk_gpon_gemUsForceIdleState_set(arg,##args)
#define rtk_rg_gpon_gemUsGemCnt_get(arg,args...) rtk_gpon_gemUsGemCnt_get(arg,##args)
#define rtk_rg_gpon_gemUsIdleByteCnt_get(arg,args...) rtk_gpon_gemUsIdleByteCnt_get(arg,##args)
#define rtk_rg_gpon_gemUsIntrDlt_check(arg,args...) rtk_gpon_gemUsIntrDlt_check(arg,##args)
#define rtk_rg_gpon_gemUsIntrMask_get(arg,args...) rtk_gpon_gemUsIntrMask_get(arg,##args)
#define rtk_rg_gpon_gemUsIntrMask_set(arg,args...) rtk_gpon_gemUsIntrMask_set(arg,##args)
#define rtk_rg_gpon_gemUsIntr_get(arg,args...) rtk_gpon_gemUsIntr_get(arg,##args)
#define rtk_rg_gpon_gemUsPortCfg_get(arg,args...) rtk_gpon_gemUsPortCfg_get(arg,##args)
#define rtk_rg_gpon_gemUsPortCfg_set(arg,args...) rtk_gpon_gemUsPortCfg_set(arg,##args)
#define rtk_rg_gpon_gemUsPtiVector_get(arg,args...) rtk_gpon_gemUsPtiVector_get(arg,##args)
#define rtk_rg_gpon_gemUsPtiVector_set(arg,args...) rtk_gpon_gemUsPtiVector_set(arg,##args)
#define rtk_rg_gpon_globalCounter_get(arg,args...) rtk_gpon_globalCounter_get(arg,##args)
#define rtk_rg_gpon_gtcDsIntrDlt_check(arg,args...) rtk_gpon_gtcDsIntrDlt_check(arg,##args)
#define rtk_rg_gpon_gtcDsIntrDlt_get(arg,args...) rtk_gpon_gtcDsIntrDlt_get(arg,##args)
#define rtk_rg_gpon_gtcDsIntrMask_get(arg,args...) rtk_gpon_gtcDsIntrMask_get(arg,##args)
#define rtk_rg_gpon_gtcDsIntrMask_set(arg,args...) rtk_gpon_gtcDsIntrMask_set(arg,##args)
#define rtk_rg_gpon_gtcDsIntr_get(arg,args...) rtk_gpon_gtcDsIntr_get(arg,##args)
#define rtk_rg_gpon_gtcUsIntrDlt_check(arg,args...) rtk_gpon_gtcUsIntrDlt_check(arg,##args)
#define rtk_rg_gpon_gtcUsIntrDlt_get(arg,args...) rtk_gpon_gtcUsIntrDlt_get(arg,##args)
#define rtk_rg_gpon_gtcUsIntrMask_get(arg,args...) rtk_gpon_gtcUsIntrMask_get(arg,##args)
#define rtk_rg_gpon_gtcUsIntrMask_set(arg,args...) rtk_gpon_gtcUsIntrMask_set(arg,##args)
#define rtk_rg_gpon_gtcUsIntr_get(arg,args...) rtk_gpon_gtcUsIntr_get(arg,##args)
#define rtk_rg_gpon_indNrmPloamState_get(arg,args...) rtk_gpon_indNrmPloamState_get(arg,##args)
#define rtk_rg_gpon_indNrmPloamState_set(arg,args...) rtk_gpon_indNrmPloamState_set(arg,##args)
#define rtk_rg_gpon_init(arg,args...) rtk_gpon_init(arg,##args)
#define rtk_rg_gpon_ipv4McAddrPtn_get(arg,args...) rtk_gpon_ipv4McAddrPtn_get(arg,##args)
#define rtk_rg_gpon_ipv4McAddrPtn_set(arg,args...) rtk_gpon_ipv4McAddrPtn_set(arg,##args)
#define rtk_rg_gpon_ipv6McAddrPtn_get(arg,args...) rtk_gpon_ipv6McAddrPtn_get(arg,##args)
#define rtk_rg_gpon_ipv6McAddrPtn_set(arg,args...) rtk_gpon_ipv6McAddrPtn_set(arg,##args)
#define rtk_rg_gpon_irq_get(arg,args...) rtk_gpon_irq_get(arg,##args)
#define rtk_rg_gpon_laserTime_get(arg,args...) rtk_gpon_laserTime_get(arg,##args)
#define rtk_rg_gpon_laserTime_set(arg,args...) rtk_gpon_laserTime_set(arg,##args)
#define rtk_rg_gpon_losCfg_get(arg,args...) rtk_gpon_losCfg_get(arg,##args)
#define rtk_rg_gpon_losCfg_set(arg,args...) rtk_gpon_losCfg_set(arg,##args)
#define rtk_rg_gpon_macVersion_get(arg,args...) rtk_gpon_macVersion_get(arg,##args)
#define rtk_rg_gpon_onuId_get(arg,args...) rtk_gpon_onuId_get(arg,##args)
#define rtk_rg_gpon_onuId_set(arg,args...) rtk_gpon_onuId_set(arg,##args)
#define rtk_rg_gpon_onuState_get(arg,args...) rtk_gpon_onuState_get(arg,##args)
#define rtk_rg_gpon_onuState_set(arg,args...) rtk_gpon_onuState_set(arg,##args)
#define rtk_rg_gpon_optLosStatus_get(arg,args...) rtk_gpon_optLosStatus_get(arg,##args)
#define rtk_rg_gpon_password_get(arg,args...) rtk_gpon_password_get(arg,##args)
#define rtk_rg_gpon_password_set(arg,args...) rtk_gpon_password_set(arg,##args)
#define rtk_rg_gpon_ploamState_get(arg,args...) rtk_gpon_ploamState_get(arg,##args)
#define rtk_rg_gpon_ploamState_set(arg,args...) rtk_gpon_ploamState_set(arg,##args)
#define rtk_rg_gpon_ponStatus_get(arg,args...) rtk_gpon_ponStatus_get(arg,##args)
#define rtk_rg_gpon_portMacForceMode_set(arg,args...) rtk_gpon_portMacForceMode_set(arg,##args)
#define rtk_rg_gpon_port_get(arg,args...) rtk_gpon_port_get(arg,##args)
#define rtk_rg_gpon_rdi_get(arg,args...) rtk_gpon_rdi_get(arg,##args)
#define rtk_rg_gpon_rdi_set(arg,args...) rtk_gpon_rdi_set(arg,##args)
#define rtk_rg_gpon_resetDoneState_get(arg,args...) rtk_gpon_resetDoneState_get(arg,##args)
#define rtk_rg_gpon_resetState_set(arg,args...) rtk_gpon_resetState_set(arg,##args)
#define rtk_rg_gpon_rogueOnt_set(arg,args...) rtk_gpon_rogueOnt_set(arg,##args)
#define rtk_rg_gpon_scheInfo_get(arg,args...) rtk_gpon_scheInfo_get(arg,##args)
#define rtk_rg_gpon_serialNumber_get(arg,args...) rtk_gpon_serialNumber_get(arg,##args)
#define rtk_rg_gpon_serialNumber_set(arg,args...) rtk_gpon_serialNumber_set(arg,##args)
#define rtk_rg_gpon_tcontCounter_get(arg,args...) rtk_gpon_tcontCounter_get(arg,##args)
#define rtk_rg_gpon_tcont_create(arg,args...) rtk_gpon_tcont_create(arg,##args)
#define rtk_rg_gpon_tcont_destroy_logical(arg,args...) rtk_gpon_tcont_destroy_logical(arg,##args)
#define rtk_rg_gpon_tcont_destroy(arg,args...) rtk_gpon_tcont_destroy(arg,##args)
#define rtk_rg_gpon_tcont_get(arg,args...) rtk_gpon_tcont_get(arg,##args)
#define rtk_rg_gpon_tcont_get_physical(arg,args...) rtk_gpon_tcont_get_physical(arg,##args)
#define rtk_rg_gpon_test_get(arg,args...) rtk_gpon_test_get(arg,##args)
#define rtk_rg_gpon_test_set(arg,args...) rtk_gpon_test_set(arg,##args)
#define rtk_rg_gpon_topIntrMask_get(arg,args...) rtk_gpon_topIntrMask_get(arg,##args)
#define rtk_rg_gpon_topIntrMask_set(arg,args...) rtk_gpon_topIntrMask_set(arg,##args)
#define rtk_rg_gpon_topIntr_disableAll(arg,args...) rtk_gpon_topIntr_disableAll(arg,##args)
#define rtk_rg_gpon_topIntr_get(arg,args...) rtk_gpon_topIntr_get(arg,##args)
#define rtk_rg_gpon_usAutoPloam_set(arg,args...) rtk_gpon_usAutoPloam_set(arg,##args)
#define rtk_rg_gpon_usBurstPolarity_get(arg,args...) rtk_gpon_usBurstPolarity_get(arg,##args)
#define rtk_rg_gpon_usBurstPolarity_set(arg,args...) rtk_gpon_usBurstPolarity_set(arg,##args)
#define rtk_rg_gpon_usFlow_get(arg,args...) rtk_gpon_usFlow_get(arg,##args)
#define rtk_rg_gpon_usFlow_set(arg,args...) rtk_gpon_usFlow_set(arg,##args)
#define rtk_rg_gpon_usGtcMiscCnt_get(arg,args...) rtk_gpon_usGtcMiscCnt_get(arg,##args)
#define rtk_rg_gpon_usOmciCnt_get(arg,args...) rtk_gpon_usOmciCnt_get(arg,##args)
#define rtk_rg_gpon_usPloamBuf_flush(arg,args...) rtk_gpon_usPloamBuf_flush(arg,##args)
#define rtk_rg_gpon_usPloamCrcGenState_get(arg,args...) rtk_gpon_usPloamCrcGenState_get(arg,##args)
#define rtk_rg_gpon_usPloamCrcGenState_set(arg,args...) rtk_gpon_usPloamCrcGenState_set(arg,##args)
#define rtk_rg_gpon_usPloamOnuIdFilterState_set(arg,args...) rtk_gpon_usPloamOnuIdFilterState_set(arg,##args)
#define rtk_rg_gpon_usPloamOnuIdFilter_get(arg,args...) rtk_gpon_usPloamOnuIdFilter_get(arg,##args)
#define rtk_rg_gpon_usPloam_set(arg,args...) rtk_gpon_usPloam_set(arg,##args)
#define rtk_rg_gpon_usScrambleState_get(arg,args...) rtk_gpon_usScrambleState_get(arg,##args)
#define rtk_rg_gpon_usScrambleState_set(arg,args...) rtk_gpon_usScrambleState_set(arg,##args)
#define rtk_rg_gpon_usSmalSstartProcState_get(arg,args...) rtk_gpon_usSmalSstartProcState_get(arg,##args)
#define rtk_rg_gpon_usSmalSstartProcState_set(arg,args...) rtk_gpon_usSmalSstartProcState_set(arg,##args)
#define rtk_rg_gpon_usSuppressLaserState_get(arg,args...) rtk_gpon_usSuppressLaserState_get(arg,##args)
#define rtk_rg_gpon_usSuppressLaserState_set(arg,args...) rtk_gpon_usSuppressLaserState_set(arg,##args)
#define rtk_rg_gpon_usTcont_del(arg,args...) rtk_gpon_usTcont_del(arg,##args)
#define rtk_rg_gpon_usTcont_get(arg,args...) rtk_gpon_usTcont_get(arg,##args)
#define rtk_rg_gpon_usTcont_set(arg,args...) rtk_gpon_usTcont_set(arg,##args)
#define rtk_rg_gpon_evtHdlStateChange_reg(arg,args...) rtk_gponapp_evtHdlStateChange_reg(arg,##args)
#define rtk_rg_gpon_evtHdlPloam_reg(arg,args...) rtk_gponapp_evtHdlPloam_reg(arg,##args)
#define rtk_rg_gpon_evtHdlPloam_dreg(arg,args...) rtk_gponapp_evtHdlPloam_dreg(arg,##args)
#define rtk_rg_gpon_evtHdlOmci_reg(arg,args...) rtk_gponapp_evtHdlOmci_reg(arg,##args)
#define rtk_rg_gpon_callbackExtMsgSetHandle_reg(arg,args...) rtk_gponapp_callbackExtMsgSetHandle_reg(arg,##args)
#define rtk_rg_gpon_callbackExtMsgGetHandle_reg(arg,args...) rtk_gponapp_callbackExtMsgGetHandle_reg(arg,##args)
#define rtk_rg_gpon_parameter_set(arg,args...) rtk_gpon_parameter_set(arg,##args)
#define rtk_rg_gpon_parameter_get(arg,args...) rtk_gpon_parameter_get(arg,##args)
#define rtk_rg_gpon_omci_tx(arg,args...) rtk_gpon_omci_tx(arg,##args)
#define rtk_rg_gpon_initial(arg,args...) rtk_gpon_initial(arg,##args)
#define rtk_rg_gpon_deinitial(arg,args...) rtk_gpon_deinitial(arg,##args)
#define rtk_rg_gpon_dbruBlockSize_get(arg,args...) rtk_gpon_dbruBlockSize_get(arg,##args)
#define rtk_rg_gpon_dbruBlockSize_set(arg,args...) rtk_gpon_dbruBlockSize_set(arg,##args)


//from epon.h
#include<rtk/epon.h>
#define rtk_rg_epon_churningKey_get(arg,args...) rtk_epon_churningKey_get(arg,##args)
#define rtk_rg_epon_churningKey_set(arg,args...) rtk_epon_churningKey_set(arg,##args)
#define rtk_rg_epon_churningStatus_get(arg,args...) rtk_epon_churningStatus_get(arg,##args)
#define rtk_rg_epon_dsFecState_get(arg,args...) rtk_epon_dsFecState_get(arg,##args)
#define rtk_rg_epon_dsFecState_set(arg,args...) rtk_epon_dsFecState_set(arg,##args)
#define rtk_rg_epon_fecState_get(arg,args...) rtk_epon_fecState_get(arg,##args)
#define rtk_rg_epon_fecState_set(arg,args...) rtk_epon_fecState_set(arg,##args)
#define rtk_rg_epon_forceLaserState_get(arg,args...) rtk_epon_forceLaserState_get(arg,##args)
#define rtk_rg_epon_forceLaserState_set(arg,args...) rtk_epon_forceLaserState_set(arg,##args)
#define rtk_rg_epon_init(arg,args...) rtk_epon_init(arg,##args)
#define rtk_rg_epon_intrMask_get(arg,args...) rtk_epon_intrMask_get(arg,##args)
#define rtk_rg_epon_intrMask_set(arg,args...) rtk_epon_intrMask_set(arg,##args)
#define rtk_rg_epon_intr_disableAll(arg,args...) rtk_epon_intr_disableAll(arg,##args)
#define rtk_rg_epon_intr_get(arg,args...) rtk_epon_intr_get(arg,##args)
#define rtk_rg_epon_laserTime_get(arg,args...) rtk_epon_laserTime_get(arg,##args)
#define rtk_rg_epon_laserTime_set(arg,args...) rtk_epon_laserTime_set(arg,##args)
#define rtk_rg_epon_llidEntryNum_get(arg,args...) rtk_epon_llidEntryNum_get(arg,##args)
#define rtk_rg_epon_llid_entry_get(arg,args...) rtk_epon_llid_entry_get(arg,##args)
#define rtk_rg_epon_llid_entry_set(arg,args...) rtk_epon_llid_entry_set(arg,##args)
#define rtk_rg_epon_losState_get(arg,args...) rtk_epon_losState_get(arg,##args)
#define rtk_rg_epon_mibCounter_get(arg,args...) rtk_epon_mibCounter_get(arg,##args)
#define rtk_rg_epon_mibGlobal_reset(arg,args...) rtk_epon_mibGlobal_reset(arg,##args)
#define rtk_rg_epon_mibLlidIdx_reset(arg,args...) rtk_epon_mibLlidIdx_reset(arg,##args)
#define rtk_rg_epon_mpcpTimeoutVal_get(arg,args...) rtk_epon_mpcpTimeoutVal_get(arg,##args)
#define rtk_rg_epon_mpcpTimeoutVal_set(arg,args...) rtk_epon_mpcpTimeoutVal_set(arg,##args)
#define rtk_rg_epon_opticalPolarity_get(arg,args...) rtk_epon_opticalPolarity_get(arg,##args)
#define rtk_rg_epon_opticalPolarity_set(arg,args...) rtk_epon_opticalPolarity_set(arg,##args)
#define rtk_rg_epon_registerReq_get(arg,args...) rtk_epon_registerReq_get(arg,##args)
#define rtk_rg_epon_registerReq_set(arg,args...) rtk_epon_registerReq_set(arg,##args)
#define rtk_rg_epon_syncTime_get(arg,args...) rtk_epon_syncTime_get(arg,##args)
#define rtk_rg_epon_thresholdReport_get(arg,args...) rtk_epon_thresholdReport_get(arg,##args)
#define rtk_rg_epon_thresholdReport_set(arg,args...) rtk_epon_thresholdReport_set(arg,##args)
#define rtk_rg_epon_usFecState_get(arg,args...) rtk_epon_usFecState_get(arg,##args)
#define rtk_rg_epon_usFecState_set(arg,args...) rtk_epon_usFecState_set(arg,##args)


//from i2c.h
#include<rtk/i2c.h>
#define rtk_rg_i2c_clock_get(arg,args...) rtk_i2c_clock_get(arg,##args)
#define rtk_rg_i2c_clock_set(arg,args...) rtk_i2c_clock_set(arg,##args)
#define rtk_rg_i2c_eepMirror_get(arg,args...) rtk_i2c_eepMirror_get(arg,##args)
#define rtk_rg_i2c_eepMirror_read(arg,args...) rtk_i2c_eepMirror_read(arg,##args)
#define rtk_rg_i2c_eepMirror_set(arg,args...) rtk_i2c_eepMirror_set(arg,##args)
#define rtk_rg_i2c_eepMirror_write(arg,args...) rtk_i2c_eepMirror_write(arg,##args)
#define rtk_rg_i2c_enable_get(arg,args...) rtk_i2c_enable_get(arg,##args)
#define rtk_rg_i2c_enable_set(arg,args...) rtk_i2c_enable_set(arg,##args)
#define rtk_rg_i2c_init(arg,args...) rtk_i2c_init(arg,##args)
#define rtk_rg_i2c_read(arg,args...) rtk_i2c_read(arg,##args)
#define rtk_rg_i2c_width_get(arg,args...) rtk_i2c_width_get(arg,##args)
#define rtk_rg_i2c_width_set(arg,args...) rtk_i2c_width_set(arg,##args)
#define rtk_rg_i2c_write(arg,args...) rtk_i2c_write(arg,##args)

//from intr.h
#include<rtk/intr.h>
#define rtk_rg_intr_gphyStatus_clear(arg,args...) rtk_intr_gphyStatus_clear(arg,##args)
#define rtk_rg_intr_gphyStatus_get(arg,args...) rtk_intr_gphyStatus_get(arg,##args)
#define rtk_rg_intr_imr_get(arg,args...) rtk_intr_imr_get(arg,##args)
#define rtk_rg_intr_imr_restore(arg,args...) rtk_intr_imr_restore(arg,##args)
#define rtk_rg_intr_imr_set(arg,args...) rtk_intr_imr_set(arg,##args)
#define rtk_rg_intr_ims_clear(arg,args...) rtk_intr_ims_clear(arg,##args)
#define rtk_rg_intr_ims_get(arg,args...) rtk_intr_ims_get(arg,##args)
#define rtk_rg_intr_init(arg,args...) rtk_intr_init(arg,##args)
#define rtk_rg_intr_linkdownStatus_clear(arg,args...) rtk_intr_linkdownStatus_clear(arg,##args)
#define rtk_rg_intr_linkdownStatus_get(arg,args...) rtk_intr_linkdownStatus_get(arg,##args)
#define rtk_rg_intr_linkupStatus_clear(arg,args...) rtk_intr_linkupStatus_clear(arg,##args)
#define rtk_rg_intr_linkupStatus_get(arg,args...) rtk_intr_linkupStatus_get(arg,##args)
#define rtk_rg_intr_polarity_get(arg,args...) rtk_intr_polarity_get(arg,##args)
#define rtk_rg_intr_polarity_set(arg,args...) rtk_intr_polarity_set(arg,##args)
#define rtk_rg_intr_speedChangeStatus_clear(arg,args...) rtk_intr_speedChangeStatus_clear(arg,##args)
#define rtk_rg_intr_speedChangeStatus_get(arg,args...) rtk_intr_speedChangeStatus_get(arg,##args)


//from ponmac.h
#include<rtk/ponmac.h>
#define rtk_rg_ponmac_flow2Queue_get(arg,args...) rtk_ponmac_flow2Queue_get(arg,##args)
#define rtk_rg_ponmac_flow2Queue_set(arg,args...) rtk_ponmac_flow2Queue_set(arg,##args)
#define rtk_rg_ponmac_init(arg,args...) rtk_ponmac_init(arg,##args)
#define rtk_rg_ponmac_losState_get(arg,args...) rtk_ponmac_losState_get(arg,##args)
#define rtk_rg_ponmac_mode_get(arg,args...) rtk_ponmac_mode_get(arg,##args)
#define rtk_rg_ponmac_mode_set(arg,args...) rtk_ponmac_mode_set(arg,##args)
#define rtk_rg_ponmac_opticalPolarity_get(arg,args...) rtk_ponmac_opticalPolarity_get(arg,##args)
#define rtk_rg_ponmac_opticalPolarity_set(arg,args...) rtk_ponmac_opticalPolarity_set(arg,##args)
#define rtk_rg_ponmac_queueDrainOut_set(arg,args...) rtk_ponmac_queueDrainOut_set(arg,##args)
#define rtk_rg_ponmac_queue_add(arg,args...) rtk_ponmac_queue_add(arg,##args)
#define rtk_rg_ponmac_queue_del(arg,args...) rtk_ponmac_queue_del(arg,##args)
#define rtk_rg_ponmac_queue_get(arg,args...) rtk_ponmac_queue_get(arg,##args)
#define rtk_rg_ponmac_transceiver_get(arg,args...) rtk_ponmac_transceiver_get(arg,##args)
#define rtk_rg_ponmac_bwThreshold_set(arg,args...) rtk_ponmac_bwThreshold_set(arg,##args)
#define rtk_rg_ponmac_bwThreshold_get(arg,args...) rtk_ponmac_bwThreshold_get(arg,##args)

//from port.h
#include<rtk/port.h>
#define rtk_rg_port_adminEnable_get(arg,args...) rtk_port_adminEnable_get(arg,##args)
#define rtk_rg_port_adminEnable_set(arg,args...) rtk_port_adminEnable_set(arg,##args)
#define rtk_rg_port_cpuPortId_get(arg,args...) rtk_port_cpuPortId_get(arg,##args)
#define rtk_rg_port_enhancedFid_get(arg,args...) rtk_port_enhancedFid_get(arg,##args)
#define rtk_rg_port_enhancedFid_set(arg,args...) rtk_port_enhancedFid_set(arg,##args)
#define rtk_rg_port_flowctrl_get(arg,args...) rtk_port_flowctrl_get(arg,##args)
#define rtk_rg_port_gigaLiteEnable_get(arg,args...) rtk_port_gigaLiteEnable_get(arg,##args)
#define rtk_rg_port_gigaLiteEnable_set(arg,args...) rtk_port_gigaLiteEnable_set(arg,##args)
#define rtk_rg_port_greenEnable_get(arg,args...) rtk_port_greenEnable_get(arg,##args)
#define rtk_rg_port_greenEnable_set(arg,args...) rtk_port_greenEnable_set(arg,##args)
#define rtk_rg_port_init(arg,args...) rtk_port_init(arg,##args)

#define rtk_rg_port_isolationEntryExt_get(arg,args...) rtk_port_isolationEntryExt_get(arg,##args)
#define rtk_rg_port_isolationEntryExt_set(arg,args...) rtk_port_isolationEntryExt_set(arg,##args)

#define rtk_rg_port_link_get(arg,args...) rtk_port_link_get(arg,##args)
#define rtk_rg_port_macExtMode_get(arg,args...) rtk_port_macExtMode_get(arg,##args)
#define rtk_rg_port_macExtMode_set(arg,args...) rtk_port_macExtMode_set(arg,##args)
#define rtk_rg_port_macExtRgmiiDelay_get(arg,args...) rtk_port_macExtRgmiiDelay_get(arg,##args)
#define rtk_rg_port_macExtRgmiiDelay_set(arg,args...) rtk_port_macExtRgmiiDelay_set(arg,##args)
#define rtk_rg_port_macForceAbilityState_get(arg,args...) rtk_port_macForceAbilityState_get(arg,##args)
#define rtk_rg_port_macForceAbilityState_set(arg,args...) rtk_port_macForceAbilityState_set(arg,##args)
#define rtk_rg_port_macForceAbility_get(arg,args...) rtk_port_macForceAbility_get(arg,##args)
#define rtk_rg_port_macForceAbility_set(arg,args...) rtk_port_macForceAbility_set(arg,##args)
#define rtk_rg_port_macLocalLoopbackEnable_get(arg,args...) rtk_port_macLocalLoopbackEnable_get(arg,##args)
#define rtk_rg_port_macLocalLoopbackEnable_set(arg,args...) rtk_port_macLocalLoopbackEnable_set(arg,##args)
#define rtk_rg_port_macRemoteLoopbackEnable_get(arg,args...) rtk_port_macRemoteLoopbackEnable_get(arg,##args)
#define rtk_rg_port_macRemoteLoopbackEnable_set(arg,args...) rtk_port_macRemoteLoopbackEnable_set(arg,##args)
#define rtk_rg_port_phyAutoNegoAbility_get(arg,args...) rtk_port_phyAutoNegoAbility_get(arg,##args)
#define rtk_rg_port_phyAutoNegoAbility_set(arg,args...) rtk_port_phyAutoNegoAbility_set(arg,##args)
#define rtk_rg_port_phyAutoNegoEnable_get(arg,args...) rtk_port_phyAutoNegoEnable_get(arg,##args)
#define rtk_rg_port_phyAutoNegoEnable_set(arg,args...) rtk_port_phyAutoNegoEnable_set(arg,##args)
#define rtk_rg_port_phyCrossOverMode_get(arg,args...) rtk_port_phyCrossOverMode_get(arg,##args)
#define rtk_rg_port_phyCrossOverMode_set(arg,args...) rtk_port_phyCrossOverMode_set(arg,##args)
#define rtk_rg_port_phyForceModeAbility_get(arg,args...) rtk_port_phyForceModeAbility_get(arg,##args)
#define rtk_rg_port_phyForceModeAbility_set(arg,args...) rtk_port_phyForceModeAbility_set(arg,##args)
#define rtk_rg_port_phyMasterSlave_get(arg,args...) rtk_port_phyMasterSlave_get(arg,##args)
#define rtk_rg_port_phyMasterSlave_set(arg,args...) rtk_port_phyMasterSlave_set(arg,##args)
#define rtk_rg_port_phyReg_get(arg,args...) rtk_port_phyReg_get(arg,##args)
#define rtk_rg_port_phyReg_set(arg,args...) rtk_port_phyReg_set(arg,##args)
#define rtk_rg_port_phyTestMode_get(arg,args...) rtk_port_phyTestMode_get(arg,##args)
#define rtk_rg_port_phyTestMode_set(arg,args...) rtk_port_phyTestMode_set(arg,##args)
#define rtk_rg_port_rtctResult_get(arg,args...) rtk_port_rtctResult_get(arg,##args)
#define rtk_rg_port_rtct_start(arg,args...) rtk_port_rtct_start(arg,##args)
#define rtk_rg_port_specialCongestStatus_clear(arg,args...) rtk_port_specialCongestStatus_clear(arg,##args)
#define rtk_rg_port_specialCongestStatus_get(arg,args...) rtk_port_specialCongestStatus_get(arg,##args)
#define rtk_rg_port_specialCongest_get(arg,args...) rtk_port_specialCongest_get(arg,##args)
#define rtk_rg_port_specialCongest_set(arg,args...) rtk_port_specialCongest_set(arg,##args)
#define rtk_rg_port_speedDuplex_get(arg,args...) rtk_port_speedDuplex_get(arg,##args)

//from rate.h
#include<rtk/rate.h>
#define rtk_rg_rate_egrBandwidthCtrlIncludeIfg_get(arg,args...) rtk_rate_egrBandwidthCtrlIncludeIfg_get(arg,##args)
#define rtk_rg_rate_egrBandwidthCtrlIncludeIfg_set(arg,args...) rtk_rate_egrBandwidthCtrlIncludeIfg_set(arg,##args)
#define rtk_rg_rate_egrQueueBwCtrlEnable_get(arg,args...) rtk_rate_egrQueueBwCtrlEnable_get(arg,##args)
#define rtk_rg_rate_egrQueueBwCtrlEnable_set(arg,args...) rtk_rate_egrQueueBwCtrlEnable_set(arg,##args)
#define rtk_rg_rate_egrQueueBwCtrlMeterIdx_get(arg,args...) rtk_rate_egrQueueBwCtrlMeterIdx_get(arg,##args)
#define rtk_rg_rate_egrQueueBwCtrlMeterIdx_set(arg,args...) rtk_rate_egrQueueBwCtrlMeterIdx_set(arg,##args)
#define rtk_rg_rate_init(arg,args...) rtk_rate_init(arg,##args)
#define rtk_rg_rate_portEgrBandwidthCtrlIncludeIfg_get(arg,args...) rtk_rate_portEgrBandwidthCtrlIncludeIfg_get(arg,##args)
#define rtk_rg_rate_portEgrBandwidthCtrlIncludeIfg_set(arg,args...) rtk_rate_portEgrBandwidthCtrlIncludeIfg_set(arg,##args)
#define rtk_rg_rate_portEgrBandwidthCtrlRate_get(arg,args...) rtk_rate_portEgrBandwidthCtrlRate_get(arg,##args)
#define rtk_rg_rate_portEgrBandwidthCtrlRate_set(arg,args...) rtk_rate_portEgrBandwidthCtrlRate_set(arg,##args)
#define rtk_rg_rate_portIgrBandwidthCtrlIncludeIfg_get(arg,args...) rtk_rate_portIgrBandwidthCtrlIncludeIfg_get(arg,##args)
#define rtk_rg_rate_portIgrBandwidthCtrlIncludeIfg_set(arg,args...) rtk_rate_portIgrBandwidthCtrlIncludeIfg_set(arg,##args)
#define rtk_rg_rate_portIgrBandwidthCtrlRate_get(arg,args...) rtk_rate_portIgrBandwidthCtrlRate_get(arg,##args)
#define rtk_rg_rate_portIgrBandwidthCtrlRate_set(arg,args...) rtk_rate_portIgrBandwidthCtrlRate_set(arg,##args)
#define rtk_rg_rate_shareMeterBucket_get(arg,args...) rtk_rate_shareMeterBucket_get(arg,##args)
#define rtk_rg_rate_shareMeterBucket_set(arg,args...) rtk_rate_shareMeterBucket_set(arg,##args)
#define rtk_rg_rate_shareMeterExceed_clear(arg,args...) rtk_rate_shareMeterExceed_clear(arg,##args)
#define rtk_rg_rate_shareMeterExceed_get(arg,args...) rtk_rate_shareMeterExceed_get(arg,##args)
#define rtk_rg_rate_shareMeterMode_get(arg,args...) rtk_rate_shareMeterMode_get(arg,##args)
#define rtk_rg_rate_shareMeterMode_set(arg,args...) rtk_rate_shareMeterMode_set(arg,##args)
#define rtk_rg_rate_shareMeter_get(arg,args...) rtk_rate_shareMeter_get(arg,##args)
#define rtk_rg_rate_shareMeter_set(arg,args...) rtk_rate_shareMeter_set(arg,##args)
#define rtk_rg_rate_stormBypass_get(arg,args...) rtk_rate_stormBypass_get(arg,##args)
#define rtk_rg_rate_stormBypass_set(arg,args...) rtk_rate_stormBypass_set(arg,##args)
#define rtk_rg_rate_stormControlEnable_get(arg,args...) rtk_rate_stormControlEnable_get(arg,##args)
#define rtk_rg_rate_stormControlEnable_set(arg,args...) rtk_rate_stormControlEnable_set(arg,##args)
#define rtk_rg_rate_stormControlMeterIdx_get(arg,args...) rtk_rate_stormControlMeterIdx_get(arg,##args)
#define rtk_rg_rate_stormControlMeterIdx_set(arg,args...) rtk_rate_stormControlMeterIdx_set(arg,##args)
#define rtk_rg_rate_stormControlPortEnable_get(arg,args...) rtk_rate_stormControlPortEnable_get(arg,##args)
#define rtk_rg_rate_stormControlPortEnable_set(arg,args...) rtk_rate_stormControlPortEnable_set(arg,##args)
#define rtk_rg_rate_hostMacAddr_get(arg,args...) rtk_rate_hostMacAddr_get(arg,##args)
#define rtk_rg_rate_hostMacAddr_set(arg,args...) rtk_rate_hostMacAddr_set(arg,##args)
#define rtk_rg_rate_hostBwCtrlMeterIdx_get(arg,args...) rtk_rate_hostBwCtrlMeterIdx_get(arg,##args)
#define rtk_rg_rate_hostBwCtrlMeterIdx_set(arg,args...) rtk_rate_hostBwCtrlMeterIdx_set(arg,##args)
#define rtk_rg_rate_hostEgrBwCtrlState_get(arg,args...) rtk_rate_hostEgrBwCtrlState_get(arg,##args)
#define rtk_rg_rate_hostEgrBwCtrlState_set(arg,args...) rtk_rate_hostEgrBwCtrlState_set(arg,##args)
#define rtk_rg_rate_hostIgrBwCtrlState_get(arg,args...) rtk_rate_hostIgrBwCtrlState_get(arg,##args)
#define rtk_rg_rate_hostIgrBwCtrlState_set(arg,args...) rtk_rate_hostIgrBwCtrlState_set(arg,##args)

//from rldp.h
#include<rtk/rldp.h>
#define rtk_rg_rldp_config_get(arg,args...) rtk_rldp_config_get(arg,##args)
#define rtk_rg_rldp_config_set(arg,args...) rtk_rldp_config_set(arg,##args)
#define rtk_rg_rldp_init(arg,args...) rtk_rldp_init(arg,##args)
#define rtk_rg_rldp_portConfig_get(arg,args...) rtk_rldp_portConfig_get(arg,##args)
#define rtk_rg_rldp_portConfig_set(arg,args...) rtk_rldp_portConfig_set(arg,##args)
#define rtk_rg_rldp_portStatus_clear(arg,args...) rtk_rldp_portStatus_clear(arg,##args)
#define rtk_rg_rldp_portStatus_get(arg,args...) rtk_rldp_portStatus_get(arg,##args)
#define rtk_rg_rldp_status_get(arg,args...) rtk_rldp_status_get(arg,##args)
#define rtk_rg_rlpp_init(arg,args...) rtk_rlpp_init(arg,##args)
#define rtk_rg_rlpp_trapType_get(arg,args...) rtk_rlpp_trapType_get(arg,##args)
#define rtk_rg_rlpp_trapType_set(arg,args...) rtk_rlpp_trapType_set(arg,##args)


//from stat.h
#include<rtk/stat.h>
#define rtk_rg_stat_global_get(arg,args...) rtk_stat_global_get(arg,##args)
#define rtk_rg_stat_global_getAll(arg,args...) rtk_stat_global_getAll(arg,##args)
#define rtk_rg_stat_global_reset(arg,args...) rtk_stat_global_reset(arg,##args)
#define rtk_rg_stat_init(arg,args...) rtk_stat_init(arg,##args)
#define rtk_rg_stat_logCtrl_get(arg,args...) rtk_stat_logCtrl_get(arg,##args)
#define rtk_rg_stat_logCtrl_set(arg,args...) rtk_stat_logCtrl_set(arg,##args)
#define rtk_rg_stat_log_get(arg,args...) rtk_stat_log_get(arg,##args)
#define rtk_rg_stat_log_reset(arg,args...) rtk_stat_log_reset(arg,##args)
#define rtk_rg_stat_mibCntMode_get(arg,args...) rtk_stat_mibCntMode_get(arg,##args)
#define rtk_rg_stat_mibCntMode_set(arg,args...) rtk_stat_mibCntMode_set(arg,##args)
#define rtk_rg_stat_mibCntTagLen_get(arg,args...) rtk_stat_mibCntTagLen_get(arg,##args)
#define rtk_rg_stat_mibCntTagLen_set(arg,args...) rtk_stat_mibCntTagLen_set(arg,##args)
#define rtk_rg_stat_mibLatchTimer_get(arg,args...) rtk_stat_mibLatchTimer_get(arg,##args)
#define rtk_rg_stat_mibLatchTimer_set(arg,args...) rtk_stat_mibLatchTimer_set(arg,##args)
#define rtk_rg_stat_mibSyncMode_get(arg,args...) rtk_stat_mibSyncMode_get(arg,##args)
#define rtk_rg_stat_mibSyncMode_set(arg,args...) rtk_stat_mibSyncMode_set(arg,##args)
#define rtk_rg_stat_pktInfo_get(arg,args...) rtk_stat_pktInfo_get(arg,##args)
#define rtk_rg_stat_port_get(arg,args...) rtk_stat_port_get(arg,##args)
#define rtk_rg_stat_port_getAll(arg,args...) rtk_stat_port_getAll(arg,##args)
#define rtk_rg_stat_port_reset(arg,args...) rtk_stat_port_reset(arg,##args)
#define rtk_rg_stat_rstCntValue_get(arg,args...) rtk_stat_rstCntValue_get(arg,##args)
#define rtk_rg_stat_rstCntValue_set(arg,args...) rtk_stat_rstCntValue_set(arg,##args)
#define rtk_rg_stat_hostCnt_reset(arg,args...) rtk_stat_hostCnt_reset(arg,##args)
#define rtk_rg_stat_hostCnt_get(arg,args...) rtk_stat_hostCnt_get(arg,##args)
#define rtk_rg_stat_hostState_get(arg,args...) rtk_stat_hostState_get(arg,##args)
#define rtk_rg_stat_hostState_set(arg,args...) rtk_stat_hostState_set(arg,##args)



//from switch.h
#include<rtk/switch.h>
#define rtk_rg_switch_allExtPortMask_set(arg,args...) rtk_switch_allExtPortMask_set(arg,##args)
#define rtk_rg_switch_allPortMask_set(arg,args...) rtk_switch_allPortMask_set(arg,##args)
#define rtk_rg_switch_chip_reset(arg,args...) rtk_switch_chip_reset(arg,##args)
#define rtk_rg_switch_deviceInfo_get(arg,args...) rtk_switch_deviceInfo_get(arg,##args)
#define rtk_rg_switch_init(arg,args...) rtk_switch_init(arg,##args)
#define rtk_rg_switch_logicalPort_get(arg,args...) rtk_switch_logicalPort_get(arg,##args)
#define rtk_rg_switch_maxPktLenLinkSpeed_get(arg,args...) rtk_switch_maxPktLenLinkSpeed_get(arg,##args)
#define rtk_rg_switch_maxPktLenLinkSpeed_set(arg,args...) rtk_switch_maxPktLenLinkSpeed_set(arg,##args)
#define rtk_rg_switch_mgmtMacAddr_get(arg,args...) rtk_switch_mgmtMacAddr_get(arg,##args)
#define rtk_rg_switch_mgmtMacAddr_set(arg,args...) rtk_switch_mgmtMacAddr_set(arg,##args)
#define rtk_rg_switch_nextPortInMask_get(arg,args...) rtk_switch_nextPortInMask_get(arg,##args)
#define rtk_rg_switch_phyPortId_get(arg,args...) rtk_switch_phyPortId_get(arg,##args)
#define rtk_rg_switch_port2PortMask_clear(arg,args...) rtk_switch_port2PortMask_clear(arg,##args)
#define rtk_rg_switch_port2PortMask_set(arg,args...) rtk_switch_port2PortMask_set(arg,##args)
#define rtk_rg_switch_portIdInMask_check(arg,args...) rtk_switch_portIdInMask_check(arg,##args)
#define rtk_rg_switch_portMask_Clear(arg,args...) rtk_switch_portMask_Clear(arg,##args)
#define rtk_rg_switch_version_get(arg,args...) rtk_switch_version_get(arg,##args)
#define rtk_rg_switch_maxPktLenByPort_get(arg,args...) rtk_switch_maxPktLenByPort_get(arg,##args)
#define rtk_rg_switch_maxPktLenByPort_set(arg,args...) rtk_switch_maxPktLenByPort_set(arg,##args)
#define rtk_rg_switch_version_get(arg,args...) rtk_switch_version_get(arg,##args)


//from trap.h
#include<rtk/trap.h>
#define rtk_rg_trap_igmpCtrlPkt2CpuEnable_get(arg,args...) rtk_trap_igmpCtrlPkt2CpuEnable_get(arg,##args)
#define rtk_rg_trap_igmpCtrlPkt2CpuEnable_set(arg,args...) rtk_trap_igmpCtrlPkt2CpuEnable_set(arg,##args)
#define rtk_rg_trap_init(arg,args...) rtk_trap_init(arg,##args)
#define rtk_rg_trap_ipMcastPkt2CpuEnable_get(arg,args...) rtk_trap_ipMcastPkt2CpuEnable_get(arg,##args)
#define rtk_rg_trap_ipMcastPkt2CpuEnable_set(arg,args...) rtk_trap_ipMcastPkt2CpuEnable_set(arg,##args)
#define rtk_rg_trap_l2McastPkt2CpuEnable_get(arg,args...) rtk_trap_l2McastPkt2CpuEnable_get(arg,##args)
#define rtk_rg_trap_l2McastPkt2CpuEnable_set(arg,args...) rtk_trap_l2McastPkt2CpuEnable_set(arg,##args)
#define rtk_rg_trap_mldCtrlPkt2CpuEnable_get(arg,args...) rtk_trap_mldCtrlPkt2CpuEnable_get(arg,##args)
#define rtk_rg_trap_mldCtrlPkt2CpuEnable_set(arg,args...) rtk_trap_mldCtrlPkt2CpuEnable_set(arg,##args)
#define rtk_rg_trap_oamPduAction_get(arg,args...) rtk_trap_oamPduAction_get(arg,##args)
#define rtk_rg_trap_oamPduAction_set(arg,args...) rtk_trap_oamPduAction_set(arg,##args)
#define rtk_rg_trap_oamPduPri_get(arg,args...) rtk_trap_oamPduPri_get(arg,##args)
#define rtk_rg_trap_oamPduPri_set(arg,args...) rtk_trap_oamPduPri_set(arg,##args)
#define rtk_rg_trap_portIgmpMldCtrlPktAction_get(arg,args...) rtk_trap_portIgmpMldCtrlPktAction_get(arg,##args)
#define rtk_rg_trap_portIgmpMldCtrlPktAction_set(arg,args...) rtk_trap_portIgmpMldCtrlPktAction_set(arg,##args)
#define rtk_rg_trap_reasonTrapToCpuPriority_get(arg,args...) rtk_trap_reasonTrapToCpuPriority_get(arg,##args)
#define rtk_rg_trap_reasonTrapToCpuPriority_set(arg,args...) rtk_trap_reasonTrapToCpuPriority_set(arg,##args)
#define rtk_rg_trap_rmaAction_get(arg,args...) rtk_trap_rmaAction_get(arg,##args)
#define rtk_rg_trap_rmaAction_set(arg,args...) rtk_trap_rmaAction_set(arg,##args)
#define rtk_rg_trap_rmaPri_get(arg,args...) rtk_trap_rmaPri_get(arg,##args)
#define rtk_rg_trap_rmaPri_set(arg,args...) rtk_trap_rmaPri_set(arg,##args)

#include "rtk/classify.h"
#define rtk_rg_classify_unmatchAction_set(arg,args...) rtk_classify_unmatchAction_set(arg,##args)

#include "rtk/vlan.h"
#define rtk_rg_vlan_reservedVidAction_set(arg,args...) rtk_vlan_reservedVidAction_set(arg,##args)
#define rtk_rg_vlan_vlanFunctionEnable_set(arg,args...) rtk_vlan_vlanFunctionEnable_set(arg,##args)

#include "rtk/svlan.h"
#define rtk_rg_svlan_svlanFunctionEnable_set(arg,args...) rtk_svlan_svlanFunctionEnable_set(arg,##args)

#include "rtk/led.h"
#define rtk_rg_led_pon_port_set(arg,args...) rtk_led_pon_port_set(arg,##args)

#include "rtk/qos.h"
#define rtk_rg_qos_schedulingType_set(arg,args...) rtk_qos_schedulingType_set(arg,##args)

#include "rtk/l2.h"
#define rtk_rg_l2_portAgingEnable_set(arg,args...) rtk_l2_portAgingEnable_set(arg,##args)
#define rtk_rg_l2_aging_set(arg,args...) rtk_l2_aging_set(arg,##args)
#define rtk_rg_l2_portAgingEnable_get(arg,args...) rtk_l2_portAgingEnable_get(arg,##args)
#define rtk_rg_l2_aging_get(arg,args...) rtk_l2_aging_get(arg,##args)

#define rtk_rg_l2_ipmcMode_get(arg,args...) rtk_l2_ipmcMode_get(arg,##args)
#define rtk_rg_l2_ipmcMode_set(arg,args...) rtk_l2_ipmcMode_set(arg,##args)
#define rtk_rg_l2_ipmcGroupLookupMissHash_set(arg,args...) rtk_l2_ipmcGroupLookupMissHash_set(arg,##args)

#elif  defined(CONFIG_XDSL_NEW_HWNAT_DRIVER) //end CONFIG_APOLLO

#define rtk_rg_l2_ipmcMode_get(arg,args...) rtk_l2_ipmcMode_get(arg,##args)
#define rtk_rg_l2_ipmcMode_set(arg,args...) rtk_l2_ipmcMode_set(arg,##args)
#define rtk_rg_l2_ipmcGroupLookupMissHash_set(arg,args...) rtk_l2_ipmcGroupLookupMissHash_set(arg,##args)
#endif  //end CONFIG_XDSL_NEW_HWNAT_DRIVER

#endif

