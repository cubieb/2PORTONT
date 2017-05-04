#ifndef RTK_RG_XDSL_STRUCT_H
#define RTK_RG_XDSL_STRUCT_H


/*
*	for xdsl only
*/
#define ETHER_ADDR_LEN				6


//typedef struct	ether_addr_s
//{
//	uint8 octet[ETHER_ADDR_LEN];
//} ether_addr_t;
#include <net/rtl/rtl_types.h>
#include <AsicDriver/rtl865xc_asicregs.h>
#include <AsicDriver/rtl865x_asicBasic.h>
#include <AsicDriver/rtl865x_asicL2.h>
#include <AsicDriver/rtl865x_asicL3.h>
#include <AsicDriver/rtl865x_asicL4.h>
#include <common/rtl_errno.h>
#include <common/rtl865x_vlan.h>

//need to put l2/l34/l34lite struct here
//defined in rt_struct 1st and move later

#ifdef CONFIG_XDSL_RG_DIAGSHELL
#include <common/rt_type.h>
#include <common/type.h>
#include <rtk/intr.h>
#include <rtk/stat.h>
#include <rtk/acl.h>
#include <rtk/l2.h>
#include <rtk/classify.h>
#include <rtk/trap.h>
#include <rtk/svlan.h>
#include <rtk/vlan.h>
#include <rtk/port.h>
#include <rtk/qos.h>
#include <rtk/ponmac.h>
#include <rtk/rate.h>
#include <rtk/sec.h>
#include <rtk/switch.h>
#endif

#ifndef FAILED
#define FAILED -1
#endif

/* Definition for flags in rtk_l2_mcastAddr_t */
#define RTK_L2_MCAST_FLAG_FWD_PRI           0x00000001
#define RTK_L2_MCAST_FLAG_IVL               0x00000002

#define RTK_L2_MCAST_FLAG_ALL               (RTK_L2_MCAST_FLAG_FWD_PRI | RTK_L2_MCAST_FLAG_IVL)

/* Definition for flags in rtk_l2_ipmcastAddr_t */
#define RTK_L2_IPMCAST_FLAG_FWD_PRI         0x00000001
#define RTK_L2_IPMCAST_FLAG_DIP_ONLY        0x00000002
#define RTK_L2_IPMCAST_FLAG_FORCE_EXT_ROUTE 0x00000004 /* Only valid when RTK_L2_IPMCAST_FLAG_DIP_ONLY is set */
#define RTK_L2_IPMCAST_FLAG_L3MC_ROUTE_EN   0x00000008 /* Only valid when RTK_L2_IPMCAST_FLAG_DIP_ONLY is set */
#define RTK_L2_IPMCAST_FLAG_STATIC          0x00000010

#define RTK_MAX_NUM_OF_QUEUE                        8
#define RTK_MAX_NUM_OF_PRIORITY 8


#ifndef CONFIG_RG_ACCESSWAN_CATEGORY_PORTMASK
#define CONFIG_RG_ACCESSWAN_CATEGORY_PORTMASK 0x18f
#endif


#define RT_PARAM_CHK(expr, errCode)\
do {\
    if ((int32)(expr)) {\
        return errCode; \
    }\
} while (0)

//add copy from re8686.h
#ifdef __KERNEL__
#ifndef _RE8686_SIM_
enum {
	RE8670_RX_STOP=0,
	RE8670_RX_CONTINUE,
	RE8670_RX_STOP_SKBNOFREE,
	RE8670_RX_END
};
#endif
#endif

/* ethernet address type */
#ifndef CONFIG_XDSL_RG_DIAGSHELL
typedef struct  rtk_mac_s
{
    uint8 octet[ETHER_ADDR_LEN];
} rtk_mac_t;

#define RTK_TOTAL_NUM_OF_WORD_FOR_1BIT_PORT_LIST    1
typedef struct rtk_portmask_s
{
    uint32  bits[RTK_TOTAL_NUM_OF_WORD_FOR_1BIT_PORT_LIST];
} rtk_portmask_t;

#define     IPV6_ADDR_LEN   16
typedef uint32 rtk_ip_addr_t;
typedef struct rtk_ipv6_addr_s
{
    uint8   ipv6_addr[IPV6_ADDR_LEN];
} rtk_ipv6_addr_t;

/* configuration mode type */
typedef enum rtk_enable_e
{
    DISABLED = 0,
    ENABLED,
    RTK_ENABLE_END
} rtk_enable_t;

//these are in "../l2Driver/rtl865x_fdb.h"
// #define RTL8651_L2TBL_ROW					256
// #define RTL8651_L2TBL_COLUMN				4
#define	FDB_TYPE_FWD					0x00 
#define	FDB_TYPE_SRCBLK				0x01
#define	FDB_TYPE_DSTBLK				0x02
#define	FDB_TYPE_TRAPCPU				0x03

/*
 * Data Type Declaration
 */
typedef uint32  rtk_vlan_t;         /* vlan id type                     */
typedef uint32  rtk_fid_t;          /* filter id type                   */
typedef uint32  rtk_efid_t;         /* enhanced filter id type          */
typedef uint32  rtk_stg_t;          /* spanning tree instance id type   */
typedef uint32  rtk_port_t;         /* port is type                     */
typedef uint32  rtk_pri_t;          /* priority vlaue                   */
typedef uint32  rtk_qid_t;          /* queue id type                    */
typedef uint32  rtk_filter_id_t;    /* filter id type                   */
typedef uint32  rtk_pie_id_t;       /* PIE id type                      */
typedef uint32  rtk_acl_id_t;       /* ACL id type                      */
typedef uint32  rtk_dscp_t;         /* DSCP value                       */


typedef enum rtk_classify_cf_sel_e
{
    CLASSIFY_CF_SEL_DISABLE = 0,
    CLASSIFY_CF_SEL_ENABLE,
    CLASSIFY_CF_SEL_END,
} rtk_classify_cf_sel_t;

/* Types of DSCP remarking source */
typedef enum rtk_qos_dscpRmkSrc_e
{
    DSCP_RMK_SRC_INT_PRI,
    DSCP_RMK_SRC_DSCP,
    DSCP_RMK_SRC_USER_PRI, 
    DSCP_RMK_SRC_END
} rtk_qos_dscpRmkSrc_t;

typedef enum rtk_intr_status_e
{
	INTR_STATUS_SPEED_CHANGE = 0,
	INTR_STATUS_LINKUP,
	INTR_STATUS_LINKDOWN,
	INTR_STATUS_GPHY,

	INTR_STATUS_END
} rtk_intr_status_t;


typedef enum rtk_sec_attackFloodType_e
{
    SEC_ICMPFLOOD,
    SEC_SYNCFLOOD,
    SEC_FINFLOOD,
    SEC_FLOOD_END
} rtk_sec_attackFloodType_t;

/* Priority to queue mapping structure */
typedef struct rtk_qos_pri2queue_s
{
    uint32 pri2queue[RTK_MAX_NUM_OF_PRIORITY];
} rtk_qos_pri2queue_t;

/* port statistic counter structure */
typedef struct rtk_stat_port_cntr_s
{
    uint64 ifInOctets;
    uint32 ifInUcastPkts;
    uint32 ifInMulticastPkts;
    uint32 ifInBroadcastPkts;
    uint32 ifInDiscards;
    uint64 ifOutOctets;
    uint32 ifOutDiscards;
    uint32 ifOutUcastPkts;
    uint32 ifOutMulticastPkts;
    uint32 ifOutBrocastPkts;
    uint32 dot1dBasePortDelayExceededDiscards;
    uint32 dot1dTpPortInDiscards;
    uint32 dot1dTpHcPortInDiscards;
    uint32 dot3InPauseFrames;
    uint32 dot3OutPauseFrames;
    uint32 dot3OutPauseOnFrames;
    uint32 dot3StatsAligmentErrors;
    uint32 dot3StatsFCSErrors;
    uint32 dot3StatsSingleCollisionFrames;
    uint32 dot3StatsMultipleCollisionFrames;
    uint32 dot3StatsDeferredTransmissions;
    uint32 dot3StatsLateCollisions;
    uint32 dot3StatsExcessiveCollisions;
    uint32 dot3StatsFrameTooLongs;
    uint32 dot3StatsSymbolErrors;
    uint32 dot3ControlInUnknownOpcodes;
    uint32 etherStatsDropEvents;
    uint64 etherStatsOctets;
    uint32 etherStatsBcastPkts;
    uint32 etherStatsMcastPkts;
    uint32 etherStatsUndersizePkts;
    uint32 etherStatsOversizePkts;
    uint32 etherStatsFragments;
    uint32 etherStatsJabbers;
    uint32 etherStatsCollisions;
    uint32 etherStatsCRCAlignErrors;
    uint32 etherStatsPkts64Octets;
    uint32 etherStatsPkts65to127Octets;
    uint32 etherStatsPkts128to255Octets;
    uint32 etherStatsPkts256to511Octets;
    uint32 etherStatsPkts512to1023Octets;
    uint32 etherStatsPkts1024to1518Octets;
    uint64 etherStatsTxOctets;
    uint32 etherStatsTxUndersizePkts;
    uint32 etherStatsTxOversizePkts;
    uint32 etherStatsTxPkts64Octets;
    uint32 etherStatsTxPkts65to127Octets;
    uint32 etherStatsTxPkts128to255Octets;
    uint32 etherStatsTxPkts256to511Octets;
    uint32 etherStatsTxPkts512to1023Octets;
    uint32 etherStatsTxPkts1024to1518Octets;
    uint32 etherStatsTxPkts1519toMaxOctets;
    uint32 etherStatsTxBcastPkts;
    uint32 etherStatsTxMcastPkts;
    uint32 etherStatsTxFragments;
    uint32 etherStatsTxJabbers;
    uint32 etherStatsTxCRCAlignErrors;
    uint32 etherStatsRxUndersizePkts;
    uint32 etherStatsRxUndersizeDropPkts;
    uint32 etherStatsRxOversizePkts;
    uint32 etherStatsRxPkts64Octets;
    uint32 etherStatsRxPkts65to127Octets;
    uint32 etherStatsRxPkts128to255Octets;
    uint32 etherStatsRxPkts256to511Octets;
    uint32 etherStatsRxPkts512to1023Octets;
    uint32 etherStatsRxPkts1024to1518Octets;
    uint32 etherStatsRxPkts1519toMaxOctets;
    uint32 inOampduPkts;
    uint32 outOampduPkts;
}rtk_stat_port_cntr_t;


typedef enum rtk_sec_attackType_e
{
    DAEQSA_DENY = 0,
    LAND_DENY,
    BLAT_DENY,
    SYNFIN_DENY,
    XMA_DENY,
    NULLSCAN_DENY,
    SYN_SPORTL1024_DENY,
    TCPHDR_MIN_CHECK,
    TCP_FRAG_OFF_MIN_CHECK,
    ICMP_FRAG_PKTS_DENY,
    POD_DENY,
    UDPDOMB_DENY,
    SYNWITHDATA_DENY,
    SYNFLOOD_DENY,
    FINFLOOD_DENY,
    ICMPFLOOD_DENY,    
    ATTACK_TYPE_END
} rtk_sec_attackType_t;

typedef enum rtk_trap_igmpMld_type_e
{
    IGMPMLD_TYPE_IGMPV1 = 0,
    IGMPMLD_TYPE_IGMPV2,
    IGMPMLD_TYPE_IGMPV3,
    IGMPMLD_TYPE_MLDV1,
    IGMPMLD_TYPE_MLDV2,
    IGMPMLD_TYPE_END
}rtk_trap_igmpMld_type_t;


typedef enum rtk_action_e
{
    ACTION_FORWARD = 0,
    ACTION_DROP,
    ACTION_TRAP2CPU,
    ACTION_COPY2CPU,
    ACTION_TO_GUESTVLAN,
    ACTION_FLOOD_IN_VLAN,
    ACTION_FLOOD_IN_ALL_PORT,
    ACTION_FLOOD_IN_ROUTER_PORTS,
    ACTION_FORWARD_EXCLUDE_CPU,
    ACTION_DROP_EXCLUDE_RMA,
    ACTION_END
} rtk_action_t;

/* l2 limit learning count action */
typedef enum rtk_l2_limitLearnCntAction_e
{
    LIMIT_LEARN_CNT_ACTION_DROP = ACTION_DROP,
    LIMIT_LEARN_CNT_ACTION_FORWARD = ACTION_FORWARD,
    LIMIT_LEARN_CNT_ACTION_TO_CPU = ACTION_TRAP2CPU,
    LIMIT_LEARN_CNT_ACTION_COPY_CPU = ACTION_COPY2CPU,
    LIMIT_LEARN_CNT_ACTION_END
} rtk_l2_limitLearnCntAction_t;


typedef enum rtk_classify_template_vid_cfg_e
{
    CLASSIFY_TEMPLATE_VID_CFG_INGRESS_CTAG = 0,
    CLASSIFY_TEMPLATE_VID_CFG_INGRESS_OUTERTAG,
    CLASSIFY_TEMPLATE_VID_CFG_DMAC_L2_IDX,
    CLASSIFY_TEMPLATE_VID_CFG_EGRESS_CTAG,
    CLASSIFY_TEMPLATE_VID_CFG_END,
} rtk_classify_template_vid_cfg_t;


typedef enum rtk_classify_template_pri_cfg_e
{
    CLASSIFY_TEMPLATE_PRI_CFG_INGRESS_CTAG = 0,
    CLASSIFY_TEMPLATE_PRI_CFG_INTERNAL_PRI,
    CLASSIFY_TEMPLATE_PRI_CFG_RSVD,
    CLASSIFY_TEMPLATE_PRI_CFG_EGRESS_CTAG,
    CLASSIFY_TEMPLATE_PRI_CFG_END,
} rtk_classify_template_pri_cfg_t;

typedef struct rtk_classify_template_cfg_pattern0_s
{
    uint16 index;
    rtk_classify_template_vid_cfg_t vidCfg; 
    rtk_classify_template_pri_cfg_t priCfg;
} rtk_classify_template_cfg_pattern0_t;


/* range check */
typedef enum rtk_classify_portrange_e
{
    CLASSIFY_PORTRANGE_SPORT = 0,
    CLASSIFY_PORTRANGE_DPORT,
    CLASSIFY_PORTRANGE_END
}rtk_classify_portrange_t;

typedef struct rtk_classify_rangeCheck_l4Port_s
{
    uint16  index;
    uint16  upperPort;    /* Port range upper bound */
    uint16  lowerPort;    /* Port range lower bound */
    rtk_classify_portrange_t type;
} rtk_classify_rangeCheck_l4Port_t;

typedef enum rtk_classify_iprange_e
{
    CLASSIFY_IPRANGE_IPV4_SIP = 0,
    CLASSIFY_IPRANGE_IPV4_DIP,
    CLASSIFY_IPRANGE_END
}rtk_classify_iprange_t;


typedef struct rtk_classify_rangeCheck_ip_s
{
    uint16 index;
    ipaddr_t upperIp;    /* IP range upper bound */
    ipaddr_t lowerIp;    /* IP range lower bound */
    rtk_classify_iprange_t type;
} rtk_classify_rangeCheck_ip_t;

typedef enum rtk_port_phy_reg_e
{
    PHY_REG_CONTROL             = 0,
    PHY_REG_STATUS,
    PHY_REG_IDENTIFIER_1,
    PHY_REG_IDENTIFIER_2,
    PHY_REG_AN_ADVERTISEMENT,
    PHY_REG_AN_LINKPARTNER,
    PHY_REG_1000_BASET_CONTROL  = 9,
    PHY_REG_1000_BASET_STATUS,
    PHY_REG_END                 = 32
} rtk_port_phy_reg_t;

/* Group type of storm control */
typedef enum rtk_rate_storm_group_e
{
    STORM_GROUP_UNKNOWN_UNICAST = 0,
    STORM_GROUP_UNKNOWN_MULTICAST,
    STORM_GROUP_MULTICAST,
    STORM_GROUP_BROADCAST,
    STORM_GROUP_DHCP,
    STORM_GROUP_ARP,
    STORM_GROUP_IGMP_MLD,
    STORM_GROUP_END
} rtk_rate_storm_group_t;

typedef struct rtk_rate_storm_group_ctrl_s
{
    uint8 unknown_unicast_enable;
    uint8 unknown_multicast_enable;
    uint8 multicast_enable;
    uint8 broadcast_enable;
    uint8 dhcp_enable;
    uint8 arp_enable;
    uint8 igmp_mld_enable;
} rtk_rate_storm_group_ctrl_t;

typedef struct rtk_qos_queue_weights_s
{
    uint32 weights[RTK_MAX_NUM_OF_QUEUE];
} rtk_qos_queue_weights_t;

typedef struct rtk_ponmac_queue_s
{
    uint32 schedulerId;
    uint32 queueId;/*0~31*/
} rtk_ponmac_queue_t;

/* Types of priority queue */
typedef enum rtk_qos_queue_type_e
{
    WFQ_WRR_PRIORITY = 0,   /* WFQ/WRR */
    STRICT_PRIORITY,        /* Strict Priority */
    QUEUE_TYPE_END
} rtk_qos_queue_type_t;

typedef struct rtk_ponmac_queueCfg_s
{
    uint32 cir;
    uint32 pir;
    rtk_qos_queue_type_t type;
    uint32 weight;
    rtk_enable_t egrssDrop;

} rtk_ponmac_queueCfg_t;

typedef struct rtk_port_phy_ability_s
{
    uint32 Half_10:1;
    uint32 Full_10:1;
    uint32 Half_100:1;
    uint32 Full_100:1;
    uint32 Half_1000:1;
    uint32 Full_1000:1;
    uint32 FC:1;
    uint32 AsyFC:1;
} rtk_port_phy_ability_t;

typedef enum rtk_port_isoConfig_e
{
    RTK_PORT_ISO_CFG_0 = 0,
    RTK_PORT_ISO_CFG_1,
    RTK_PORT_ISO_CFG_END
} rtk_port_isoConfig_t;


typedef enum rtk_l2_lookupMissType_e
{
    DLF_TYPE_IPMC = 0,
    DLF_TYPE_UCAST,
    DLF_TYPE_BCAST,
    DLF_TYPE_MCAST,
    DLF_TYPE_IP6MC,
    DLF_TYPE_END
} rtk_l2_lookupMissType_t;


typedef enum rtk_port_duplex_e
{
    PORT_HALF_DUPLEX = 0,
    PORT_FULL_DUPLEX,
    PORT_DUPLEX_END
} rtk_port_duplex_t;

typedef enum rtk_port_speed_e
{
    PORT_SPEED_10M = 0,
    PORT_SPEED_100M,
    PORT_SPEED_1000M,
    PORT_SPEED_500M,
    PORT_SPEED_END
} rtk_port_speed_t;

typedef enum rtk_port_linkStatus_e
{
    PORT_LINKDOWN = 0,
    PORT_LINKUP,
    PORT_LINKSTATUS_END
} rtk_port_linkStatus_t;

typedef struct rtk_port_macAbility_s
{
    rtk_port_speed_t        speed;
    rtk_port_duplex_t       duplex;
	rtk_enable_t 		    linkFib1g;
	rtk_port_linkStatus_t 	linkStatus;
	rtk_enable_t 		    txFc;
	rtk_enable_t 		    rxFc;
	rtk_enable_t 		    nwayAbility;
	rtk_enable_t 		    masterMod;
	rtk_enable_t 		    nwayFault;
	rtk_enable_t 		    lpi_100m;
	rtk_enable_t 		    lpi_giga;

} rtk_port_macAbility_t;

typedef enum rtk_fidMode_e
{
    VLAN_FID_IVL = 0,
    VLAN_FID_SVL,
    VLAN_FID_MODE_END
} rtk_fidMode_t;

typedef enum rtk_ponmac_mode_s
{
	PONMAC_MODE_GPON=0,
	PONMAC_MODE_EPON,
	PONMAC_MODE_FIBER,
	PONMAC_MODE_END
}rtk_ponmac_mode_t;

typedef enum rtk_classify_unmatch_action_e
{
    CLASSIFY_UNMATCH_DROP = 0,
    CLASSIFY_UNMATCH_PERMIT_WITHOUT_PON,
    CLASSIFY_UNMATCH_PERMIT,
    CLASSIFY_UNMATCH_END,
} rtk_classify_unmatch_action_t;


/* L2 IP multicast switch mode */
typedef enum rtk_l2_ipmcMode_e
{
    LOOKUP_ON_MAC_AND_VID_FID = 0,
    LOOKUP_ON_DIP_AND_SIP,
    LOOKUP_ON_DIP_AND_VID,
    LOOKUP_ON_DIP,
    LOOKUP_ON_DIP_AND_CVID,
    IPMC_MODE_END
} rtk_l2_ipmcMode_t;


/* L2 IP multicast hash operation */
typedef enum rtk_l2_ipmcHashOp_e
{
    HASH_DIP_AND_SIP = 0,
    HASH_DIP_ONLY,
    HASH_END
} rtk_l2_ipmcHashOp_t;

typedef enum rt_error_common_e
{
    RT_ERR_FAILED = -1,                             /* General Error                                                                    */

    /* 0x0000xxxx for common error code */
    RT_ERR_OK = 0,                                  /* 0x00000000, OK                                                                   */
    RT_ERR_INPUT,                                   /* 0x00000001, invalid input parameter                                              */
    RT_ERR_UNIT_ID,                                 /* 0x00000002, invalid unit id                                                      */
    RT_ERR_PORT_ID,                                 /* 0x00000003, invalid port id                                                      */
    RT_ERR_PORT_MASK,                               /* 0x00000004, invalid port mask                                                    */
    RT_ERR_PORT_LINKDOWN,                           /* 0x00000005, link down port status                                                */
    RT_ERR_ENTRY_INDEX,                             /* 0x00000006, invalid entry index                                                  */
    RT_ERR_NULL_POINTER,                            /* 0x00000007, input parameter is null pointer                                      */
    RT_ERR_QUEUE_ID,                                /* 0x00000008, invalid queue id                                                     */
    RT_ERR_QUEUE_NUM,                               /* 0x00000009, invalid queue number                                                 */
    RT_ERR_BUSYWAIT_TIMEOUT,                        /* 0x0000000a, busy watting time out                                                */
    RT_ERR_MAC,                                     /* 0x0000000b, invalid mac address                                                  */
    RT_ERR_OUT_OF_RANGE,                            /* 0x0000000c, input parameter out of range                                         */
    RT_ERR_CHIP_NOT_SUPPORTED,                      /* 0x0000000d, functions not supported by this chip model                           */
    RT_ERR_SMI,                                     /* 0x0000000e, SMI error                                                            */
    RT_ERR_NOT_INIT,                                /* 0x0000000f, The module is not initial                                            */
    RT_ERR_CHIP_NOT_FOUND,                          /* 0x00000010, The chip can not found                                               */
    RT_ERR_NOT_ALLOWED,                             /* 0x00000011, actions not allowed by the function                                  */
    RT_ERR_DRIVER_NOT_FOUND,                        /* 0x00000012, The driver can not found                                             */
    RT_ERR_SEM_LOCK_FAILED,                         /* 0x00000013, Failed to lock semaphore                                             */
    RT_ERR_SEM_UNLOCK_FAILED,                       /* 0x00000014, Failed to unlock semaphore                                           */
    RT_ERR_THREAD_EXIST,                            /* 0x00000015, Thread exist                                                         */
    RT_ERR_THREAD_CREATE_FAILED,                    /* 0x00000016, Thread create fail                                                   */
    RT_ERR_FWD_ACTION,                              /* 0x00000017, Invalid forwarding Action                                            */
    RT_ERR_IPV4_ADDRESS,                            /* 0x00000018, Invalid IPv4 address                                                 */
    RT_ERR_IPV6_ADDRESS,                            /* 0x00000019, Invalid IPv6 address                                                 */
    RT_ERR_PRIORITY,                                /* 0x0000001a, Invalid Priority value                                               */
    RT_ERR_FID,                                     /* 0x0000001b, invalid fid                                                          */
    RT_ERR_ENTRY_NOTFOUND,                          /* 0x0000001c, specified entry not found                                            */
    RT_ERR_DROP_PRECEDENCE,                         /* 0x0000001d, invalid drop precedence                                              */
    RT_ERR_NOT_FINISH,                              /* 0x0000001e, Action not finish, still need to wait                                */
    RT_ERR_TIMEOUT,                                 /* 0x0000001f, Time out                                                             */
    RT_ERR_REG_ARRAY_INDEX_1,                       /* 0x00000020, invalid index 1 of register array                                    */
    RT_ERR_REG_ARRAY_INDEX_2,                       /* 0x00000021, invalid index 2 of register array                                    */
    RT_ERR_ETHER_TYPE,                              /* 0x00000022, invalid ether type                                                   */
    RT_ERR_ENTRY_FULL,                              /* 0x00000023, entry is full                                                        */
    RT_ERR_EFID,                                    /* 0x00000024, invalid enhanced fid                                                 */
    RT_ERR_FEATURE_NOT_SUPPORTED,                   /* 0x00000025, this feature not support                                             */
    RT_ERR_ENTRY_EXIST,                             /* 0x00000026, existed entry                                                        */
    RT_ERR_EEPROM_NACK,                             /* 0x00000027, EEPROM NACK                                                          */
    RT_ERR_COMMON_END = 0xFFFF                      /* The symbol is the latest symbol of common error                                  */
} rt_error_common_t;


/*
 * Data Type Declaration
 */
typedef enum rt_error_code_e
{
    /*
     * 0x0000xxxx for common error code
     * symbols defined at rt_error_common_t
     */

    /* 0x0001xxxx for vlan */
    RT_ERR_VLAN_VID = 0x00010000,                   /* 0x00010000, invalid vid                                                          */
    RT_ERR_VLAN_PRIORITY,                           /* 0x00010001, invalid 1p priority                                                  */
    RT_ERR_VLAN_EMPTY_ENTRY,                        /* 0x00010002, emtpy entry of vlan table                                            */
    RT_ERR_VLAN_ACCEPT_FRAME_TYPE,                  /* 0x00010003, invalid accept frame type                                            */
    RT_ERR_VLAN_EXIST,                              /* 0x00010004, vlan is exist                                                        */
    RT_ERR_VLAN_ENTRY_NOT_FOUND,                    /* 0x00010005, specified vlan entry not found                                       */
    RT_ERR_VLAN_PORT_MBR_EXIST,                     /* 0x00010006, member port exist in the specified vlan                              */
    RT_ERR_VLAN_FRAME_TYPE,                         /* 0x00010007, Error frame type                                                     */
    RT_ERR_VLAN_PROTO_AND_PORT,                     /* 0x00010008, Envalid protocol base group database index                           */


    /* 0x0002xxxx for svlan */
    RT_ERR_SVLAN_ENTRY_INDEX = 0x00020000,          /* 0x00020000, invalid svid entry no                                                */
    RT_ERR_SVLAN_ETHER_TYPE,                        /* 0x00020001, invalid SVLAN ether type                                             */
    RT_ERR_SVLAN_TABLE_FULL,                        /* 0x00020002, no empty entry in SVLAN table                                        */
    RT_ERR_SVLAN_ENTRY_NOT_FOUND,                   /* 0x00020003, specified svlan entry not found                                      */
    RT_ERR_SVLAN_EXIST,                             /* 0x00020004, SVLAN entry is exist                                                 */
    RT_ERR_SVLAN_VID,                               /* 0x00020005, invalid svid                                                         */
    RT_ERR_SVLAN_EFID,                              /* 0x00020006, invalid EFID                                                        */
    RT_ERR_SVLAN_EVID,                              /* 0x00020007, invalid EVID                                                        */
    RT_ERR_SVLAN_NOT_EXIST,                         /* 0x00020008, SVLAN entry is not exist                                                 */
    RT_ERR_SVLAN_INVALID,                           /* 0x00020009, SVLAN related config is invalid                                                  */

    /* 0x0003xxxx for MSTP */
    RT_ERR_MSTI = 0x00030000,                       /* 0x00030000, invalid msti                                                         */
    RT_ERR_MSTP_STATE,                              /* 0x00030001, invalid spanning tree status                                         */
    RT_ERR_MSTI_EXIST,                              /* 0x00030002, MSTI exist                                                           */
    RT_ERR_MSTI_NOT_EXIST,                          /* 0x00030003, MSTI not exist                                                       */

    /* 0x0004xxxx for BUCKET */
    RT_ERR_TIMESLOT = 0x00040000,                   /* 0x00040000, invalid time slot                                                    */
    RT_ERR_TOKEN,                                   /* 0x00040001, invalid token amount                                                 */
    RT_ERR_RATE,                                    /* 0x00040002, invalid rate                                                         */

    /* 0x0005xxxx for RMA */
    RT_ERR_RMA_ADDR = 0x00050000,                   /* 0x00050000, invalid rma mac address                                              */
    RT_ERR_RMA_ACTION,                              /* 0x00050001, invalid rma action                                                   */
    RT_ERR_RMA_MGMT_TYPE,                           /* 0x00050002, invalid type of management frame                                     */

    /* 0x0006xxxx for L2 */
    RT_ERR_L2_HASH_KEY = 0x00060000,                /* 0x00060000, invalid L2 Hash key                                                  */
    RT_ERR_L2_HASH_INDEX,                           /* 0x00060001, invalid L2 Hash index                                                */
    RT_ERR_L2_CAM_INDEX,                            /* 0x00060002, invalid L2 CAM index                                                 */
    RT_ERR_L2_ENRTYSEL,                             /* 0x00060003, invalid EntrySel                                                     */
    RT_ERR_L2_INDEXTABLE_INDEX,                     /* 0x00060004, invalid L2 index table(=portMask table) index                        */
    RT_ERR_LIMITED_L2ENTRY_NUM,                     /* 0x00060005, invalid limited L2 entry number                                      */
    RT_ERR_L2_AGGREG_PORT,                          /* 0x00060006, this aggregated port is not the lowest physical
                                                                   port of its aggregation group                                        */
    RT_ERR_L2_FID,                                  /* 0x00060007, invalid fid                                                          */
    RT_ERR_L2_RVID,                                 /* 0x00060008, invalid cvid                                                         */
    RT_ERR_L2_NO_EMPTY_ENTRY,                       /* 0x00060009, no empty entry in L2 table                                           */
    RT_ERR_L2_ENTRY_NOTFOUND,                       /* 0x0006000a, specified entry not found                                            */
    RT_ERR_L2_INDEXTBL_FULL,                        /* 0x0006000b, the L2 index table is full                                           */
    RT_ERR_L2_INVALID_FLOWTYPE,                     /* 0x0006000c, invalid L2 flow type                                                 */
    RT_ERR_L2_L2UNI_PARAM,                          /* 0x0006000d, invalid L2 unicast parameter                                         */
    RT_ERR_L2_L2MULTI_PARAM,                        /* 0x0006000e, invalid L2 multicast parameter                                       */
    RT_ERR_L2_IPMULTI_PARAM,                        /* 0x0006000f, invalid L2 ip multicast parameter                                    */
    RT_ERR_L2_PARTIAL_HASH_KEY,                     /* 0x00060010, invalid L2 partial Hash key                                          */
    RT_ERR_L2_EMPTY_ENTRY,                          /* 0x00060011, the entry is empty(invalid)                                          */
    RT_ERR_L2_FLUSH_TYPE,                           /* 0x00060012, the flush type is invalid                                            */
    RT_ERR_L2_NO_CPU_PORT,                          /* 0x00060013, CPU port not exist                                                   */
    RT_ERR_L2_MULTI_FWD_INDEX,                      /* 0x00060014, invalid index of multicast forwarding portmask                       */
    RT_ERR_L2_ENTRY_EXIST,                          /* 0x00060015, entry already exist                                                  */
    RT_ERR_L2_EXCEPT_ADDR_TYPE,                     /* 0x00060016, Invalid exception address type                                       */
    RT_ERR_L2_MCAST_FWD_ENTRY_EXIST,                /* 0x00060017, mcast forwarding entry already exist                                 */
    RT_ERR_L2_MCAST_FWD_ENTRY_NOT_EXIST,            /* 0x00060018, mcast forwarding entry not exist                                     */
    RT_ERR_L2_EFID,                                 /* 0x00060019, Invalid Enhanced FID                                                 */

    /* 0x0007xxxx for FILTER (PIE) */
    RT_ERR_FILTER_BLOCKNUM = 0x00070000,            /* 0x00070000, invalid block number                                                 */
    RT_ERR_FILTER_ENTRYIDX,                         /* 0x00070001, invalid entry index                                                  */
    RT_ERR_FILTER_CUTLINE,                          /* 0x00070002, invalid cutline value                                                */
    RT_ERR_FILTER_FLOWTBLBLOCK,                     /* 0x00070003, block belongs to flow table                                          */
    RT_ERR_FILTER_INACLBLOCK,                       /* 0x00070004, block belongs to ingress ACL                                         */
    RT_ERR_FILTER_ACTION,                           /* 0x00070005, action doesn't consist to entry type                                 */
    RT_ERR_FILTER_INACL_RULENUM,                    /* 0x00070006, invalid ACL rulenum                                                  */
    RT_ERR_FILTER_INACL_TYPE,                       /* 0x00070007, entry type isn't an ingress ACL rule                                 */
    RT_ERR_FILTER_INACL_EXIST,                      /* 0x00070008, ACL entry is already exit                                            */
    RT_ERR_FILTER_INACL_EMPTY,                      /* 0x00070009, ACL entry is empty                                                   */
    RT_ERR_FILTER_FLOWTBL_TYPE,                     /* 0x0007000a, entry type isn't an flow table rule                                  */
    RT_ERR_FILTER_FLOWTBL_RULENUM,                  /* 0x0007000b, invalid flow table rulenum                                           */
    RT_ERR_FILTER_FLOWTBL_EMPTY,                    /* 0x0007000c, flow table entry is empty                                            */
    RT_ERR_FILTER_FLOWTBL_EXIST,                    /* 0x0007000d, flow table entry is already exist                                    */
    RT_ERR_FILTER_METER_ID,                         /* 0x0007000e, invalid metering id                                                  */
    RT_ERR_FILTER_LOG_ID,                           /* 0x0007000f, invalid log id                                                       */
    RT_ERR_PIE_FIELD_TYPE,                          /* 0x00070010, invalid pie field type                                               */
    RT_ERR_PIE_PHASE,                               /* 0x00070011, invalid pie phase                                                    */
    RT_ERR_PIE_PHASE_NOT_SUPPORTED,                 /* 0x00070012, pie phase not supported in the chip                                  */

    /* 0x0008xxxx for ACL Rate Limit */
    RT_ERR_ACLRL_HTHR = 0x00080000,                 /* 0x00080000, invalid high threshold                                               */
    RT_ERR_ACLRL_TIMESLOT,                          /* 0x00080001, invalid time slot                                                    */
    RT_ERR_ACLRL_TOKEN,                             /* 0x00080002, invalid token amount                                                 */
    RT_ERR_ACLRL_RATE,                              /* 0x00080003, invalid rate                                                         */

    /* 0x0009xxxx for Link aggregation */
    RT_ERR_LA_CPUPORT = 0x00090000,                 /* 0x00090000, CPU port can not be aggregated port                                  */
    RT_ERR_LA_TRUNK_ID,                             /* 0x00090001, invalid trunk id                                                     */
    RT_ERR_LA_PORTMASK,                             /* 0x00090002, invalid port mask                                                    */
    RT_ERR_LA_HASHMASK,                             /* 0x00090003, invalid hash mask                                                    */
    RT_ERR_LA_DUMB,                                 /* 0x00090004, this API should be used in 802.1ad dumb mode                         */
    RT_ERR_LA_PORTNUM_DUMB,                         /* 0x00090005, it can only aggregate at most four ports when 802.1ad dumb mode      */
    RT_ERR_LA_PORTNUM_NORMAL,                       /* 0x00090006, it can only aggregate at most eight ports when 802.1ad normal mode   */
    RT_ERR_LA_MEMBER_OVERLAP,                       /* 0x00090007, the specified port mask is overlapped with other group               */
    RT_ERR_LA_NOT_MEMBER_PORT,                      /* 0x00090008, the port is not a member port of the trunk                           */
    RT_ERR_LA_TRUNK_NOT_EXIST,                      /* 0x00090009, the trunk doesn't exist                                              */

    /* 0x000axxxx for storm filter */
    RT_ERR_SFC_TICK_PERIOD = 0x000a0000,            /* 0x000a0000, invalid SFC tick period                                              */
    RT_ERR_SFC_UNKNOWN_GROUP,                       /* 0x000a0001, Unknown Storm filter group                                           */

    /* 0x000bxxxx for pattern match */
    RT_ERR_PM_MASK = 0x000b0000,                    /* 0x000b0000, invalid pattern length. Pattern length should be 8                   */
    RT_ERR_PM_LENGTH,                               /* 0x000b0001, invalid pattern match mask, first byte must care                     */
    RT_ERR_PM_MODE,                                 /* 0x000b0002, invalid pattern match mode                                           */

    /* 0x000cxxxx for input bandwidth control */
    RT_ERR_INBW_TICK_PERIOD = 0x000c0000,           /* 0x000c0000, invalid tick period for input bandwidth control                      */
    RT_ERR_INBW_TOKEN_AMOUNT,                       /* 0x000c0001, invalid amount of token for input bandwidth control                  */
    RT_ERR_INBW_FCON_VALUE,                         /* 0x000c0002, invalid flow control ON threshold value for input bandwidth control  */
    RT_ERR_INBW_FCOFF_VALUE,                        /* 0x000c0003, invalid flow control OFF threshold value for input bandwidth control */
    RT_ERR_INBW_FC_ALLOWANCE,                       /* 0x000c0004, invalid allowance of incomming packet for input bandwidth control    */
    RT_ERR_INBW_RATE,                               /* 0x000c0005, invalid input bandwidth                                              */

    /* 0x000dxxxx for QoS */
    RT_ERR_QOS_1P_PRIORITY = 0x000d0000,            /* 0x000d0000, invalid 802.1P priority                                              */
    RT_ERR_QOS_DSCP_VALUE,                          /* 0x000d0001, invalid DSCP value                                                   */
    RT_ERR_QOS_INT_PRIORITY,                        /* 0x000d0002, invalid internal priority                                            */
    RT_ERR_QOS_SEL_DSCP_PRI,                        /* 0x000d0003, invalid DSCP selection priority                                      */
    RT_ERR_QOS_SEL_PORT_PRI,                        /* 0x000d0004, invalid port selection priority                                      */
    RT_ERR_QOS_SEL_IN_ACL_PRI,                      /* 0x000d0005, invalid ingress ACL selection priority                               */
    RT_ERR_QOS_SEL_CLASS_PRI,                       /* 0x000d0006, invalid classifier selection priority                                */
    RT_ERR_QOS_EBW_RATE,                            /* 0x000d0007, invalid egress bandwidth rate                                        */
    RT_ERR_QOS_SCHE_TYPE,                           /* 0x000d0008, invalid QoS scheduling type                                          */
    RT_ERR_QOS_QUEUE_WEIGHT,                        /* 0x000d0009, invalid Queue weight                                                 */

    /* 0x000exxxx for port ability */
    RT_ERR_PHY_PAGE_ID = 0x000e0000,                /* 0x000e0000, invalid PHY page id                                                  */
    RT_ERR_PHY_REG_ID,                              /* 0x000e0001, invalid PHY reg id                                                   */
    RT_ERR_PHY_DATAMASK,                            /* 0x000e0002, invalid PHY data mask                                                */
    RT_ERR_PHY_AUTO_NEGO_MODE,                      /* 0x000e0003, invalid PHY auto-negotiation mode*/
    RT_ERR_PHY_SPEED,                               /* 0x000e0004, invalid PHY speed setting                                            */
    RT_ERR_PHY_DUPLEX,                              /* 0x000e0005, invalid PHY duplex setting                                           */
    RT_ERR_PHY_FORCE_ABILITY,                       /* 0x000e0006, invalid PHY force mode ability parameter                             */
    RT_ERR_PHY_FORCE_1000,                          /* 0x000e0007, invalid PHY force mode 1G speed setting                              */
    RT_ERR_PHY_TXRX,                                /* 0x000e0008, invalid PHY tx/rx                                                    */
    RT_ERR_PHY_RTCT_NOT_FINISH,                     /* 0x000e0009, PHY RTCT in progress                                                 */
    RT_ERR_PHY_RTCT_TIMEOUT,                        /* 0x000e000a, PHY RTCT timeout                                                     */
    RT_ERR_PHY_AUTO_ABILITY,                        /* 0x000e000b, invalid PHY auto mode ability parameter                              */
    RT_ERR_PHY_FIBER_LINKUP,                        /* 0x000e000c, Access copper PHY is not permit when fiber is linkup                 */

    /* 0x000fxxxx for mirror */
    RT_ERR_MIRROR_DIRECTION = 0x000f0000,           /* 0x000f0000, invalid error mirror direction                                       */
    RT_ERR_MIRROR_SESSION_FULL,                     /* 0x000f0001, mirroring session is full                                            */
    RT_ERR_MIRROR_SESSION_NOEXIST,                  /* 0x000f0002, mirroring session not exist                                          */
    RT_ERR_MIRROR_PORT_EXIST,                       /* 0x000f0003, mirroring port already exists                                        */
    RT_ERR_MIRROR_PORT_NOT_EXIST,                   /* 0x000f0004, mirroring port does not exists                                       */
    RT_ERR_MIRROR_PORT_FULL,                        /* 0x000f0005, Exceeds maximum number of supported mirroring port                   */
    RT_ERR_MIRROR_ID,                               /* 0x000f0006, Invalid mirror ID                                                    */

    /* 0x0010xxxx for stat */
    RT_ERR_STAT_INVALID_GLOBAL_CNTR = 0x00100000,   /* 0x00100000, Invalid Global Counter                                               */
    RT_ERR_STAT_INVALID_PORT_CNTR,                  /* 0x00100001, Invalid Port Counter                                                 */
    RT_ERR_STAT_GLOBAL_CNTR_FAIL,                   /* 0x00100002, Could not retrieve/reset Global Counter                              */
    RT_ERR_STAT_PORT_CNTR_FAIL,                     /* 0x00100003, Could not retrieve/reset Port Counter                                */
    RT_ERR_STAT_INVALID_SMON_CNTR,                  /* 0x00100004, Invalid SMON Counter                                                 */
    RT_ERR_STAT_SMON_CNTR_FAIL,                     /* 0x00100005, Could not retrieve/reset SMON Counter                                */

    /* 0x0011xxxx for dot1x */
    RT_ERR_DOT1X_INVALID_DIRECTION = 0x00110000,    /* 0x00110000, Invalid Authentication Direction                                     */
    RT_ERR_DOT1X_PROC,                              /* 0x00110001, Invalid process type                                                 */
    RT_ERR_DOT1X_GVLANIDX,                          /* 0x00110002, Invalid cvid index                                                   */

    /* 0x0012xxxx for GPON */
    RT_ERR_GPON_INITIAL_FAIL = 0x00120000,          /* 0x00120000, GPON driver initialize fail                                          */
    RT_ERR_GPON_INVALID_HANDLE,                     /* 0x00120001, GPON driver handle error                                             */
    RT_ERR_GPON_DRV_NOT_STARTED,                    /* 0x00120002, GPON driver is not started                                           */
    RT_ERR_GPON_DRV_ALREADY_STARTED,                /* 0x00120003, GPON driver is already started                                       */
    RT_ERR_GPON_ONU_ALREADY_REGISTERED,             /* 0x00120004, ONU is already registered                                            */
    RT_ERR_GPON_ONU_NOT_REGISTERED,                 /* 0x00120005, ONU is not registered                                                */
    RT_ERR_GPON_ONU_ALREADY_ACTIVATED,              /* 0x00120006, GPON Device has already been activeted                               */
    RT_ERR_GPON_ONU_NOT_ACTIVATED,                  /* 0x00120007, GPON Device has not been activeted                                   */
    RT_ERR_GPON_EXCEED_MAX_TCONT,                   /* 0x00120008, Exceed maximum number of the tcont                                   */
    RT_ERR_GPON_EXCEED_MAX_FLOW,                    /* 0x00120009, Exceed maximum number of the flow                                    */
    RT_ERR_GPON_PLOAM_QUEUE_FULL,                   /* 0x0012000a, PLOAM queue is full                                                  */
    RT_ERR_GPON_PLOAM_QUEUE_EMPTY,                  /* 0x0012000b, PLOAM queue is empty                                                 */
    RT_ERR_GPON_OMCI_QUEUE_FULL,                    /* 0x0012000c, OMCI queue is full                                                   */
    RT_ERR_GPON_OMCI_QUEUE_EMPTY,                   /* 0x0012000d, OMCI queue is empty                                                  */
    RT_ERR_GPON_TABLE_ENTRY_NOT_FOUND,              /* 0x0012000e, The entry is not found in table                                      */
    RT_ERR_GPON_MAC_FILTER_TABLE_FULL,              /* 0x0012000f, The entry is not found in table                                      */
    RT_ERR_GPON_MAC_FILTER_TABLE_ALLOC_FAIL,        /* 0x00120010, The entry is allocate fail                                           */

    /* 0x0013xxxx for DSL or EXT */
    RT_ERR_DSL_VC = 0x00130000,          			/* 0x00130000, invalid DSL VC                                          */
    RT_ERR_EXT_PORT_ID,           					/* 0x00130001, invalid EXT port id                                          */

    RT_ERR_END                                      /* The symbol is the latest symbol                                                  */
} rt_error_code_t;

/* information of device */
typedef enum rtk_switch_port_name_e
{
    /*normal UTP port*/
    RTK_PORT_UTP0 = 0,
    RTK_PORT_UTP1,
    RTK_PORT_UTP2,
    RTK_PORT_UTP3,
    RTK_PORT_UTP4,
    RTK_PORT_UTP5,
    RTK_PORT_UTP6,
    RTK_PORT_UTP7,
    RTK_PORT_UTP8,
    RTK_PORT_UTP9,
    RTK_PORT_UTP10,
    RTK_PORT_UTP11 = 63,
    
    /*PON port*/
    RTK_PORT_PON = 128,

    /*Fiber port*/
    RTK_PORT_FIBER = 256,

    /* physical extention port*/
    RTK_PORT_EXT0 = 512,
    RTK_PORT_EXT1,
    RTK_PORT_EXT2,

    /*CPU port*/
    RTK_PORT_CPU = 1024,
    RTK_PORT_NAME_END
} rtk_switch_port_name_t;

/* Definition for flags in rtk_l2_ucastAddr_t */
#define RTK_L2_UCAST_FLAG_SA_BLOCK          0x00000001
#define RTK_L2_UCAST_FLAG_DA_BLOCK          0x00000002
#define RTK_L2_UCAST_FLAG_STATIC            0x00000004
#define RTK_L2_UCAST_FLAG_ARP_USED          0x00000008
#define RTK_L2_UCAST_FLAG_FWD_PRI           0x00000010
#define RTK_L2_UCAST_FLAG_LOOKUP_PRI        0x00000020
#define RTK_L2_UCAST_FLAG_IVL               0x00000040

#define RTK_L2_UCAST_FLAG_ALL               ( RTK_L2_UCAST_FLAG_SA_BLOCK | \
                                            RTK_L2_UCAST_FLAG_DA_BLOCK   | \
                                            RTK_L2_UCAST_FLAG_STATIC     | \
                                            RTK_L2_UCAST_FLAG_ARP_USED   | \
                                            RTK_L2_UCAST_FLAG_FWD_PRI    | \
                                            RTK_L2_UCAST_FLAG_LOOKUP_PRI | \
                                            RTK_L2_UCAST_FLAG_IVL )

/* l2 address table - unicast data structure */
typedef struct rtk_l2_ucastAddr_s
{
    rtk_vlan_t  vid;
    rtk_mac_t   mac;
    uint32      fid;
    uint32      efid;
    rtk_port_t  port;
    uint32      ext_port;
    uint32      flags;          /* Refer to RTK_L2_UCAST_FLAG_XXX */
    uint32      age;
    uint32      priority;
    uint8       auth;
    uint32      index;
	rtk_vlan_t  ctag_vid;
}rtk_l2_ucastAddr_t;

/* l2 address table - multicast data structure */
typedef struct rtk_l2_mcastAddr_s
{
    uint16          vid;
    rtk_mac_t       mac;
    uint32          fid;
    rtk_portmask_t  portmask;
    rtk_portmask_t  ext_portmask;
    uint32          flags;          /* Refer to RTK_L2_MCAST_FLAG_XXX */
    uint32          priority;
    uint32          index;
}rtk_l2_mcastAddr_t;

/* l2 address table - ip multicast data structure */
typedef struct rtk_l2_ipMcastAddr_s
{
    ipaddr_t        dip;
    ipaddr_t        sip;
    uint16          vid;
    rtk_portmask_t  portmask;
    rtk_portmask_t  ext_portmask;
    uint32          l3_trans_index;
    uint32          flags;          /* Refer to RTK_L2_IPMCAST_FLAG_XXX */
    uint32          priority;
    uint32          index;
}rtk_l2_ipMcastAddr_t;
typedef enum rtk_lut_entry_type_e
{
    RTK_LUT_L2UC = 0,
    RTK_LUT_L2MC,
    RTK_LUT_L3MC,
    RTK_LUT_END
} rtk_lut_entry_type_t;


/*l2 address read method*/
typedef enum rtk_l2_readMethod_e
{
	LUT_READ_METHOD_MAC = 0,
	LUT_READ_METHOD_ADDRESS,
	LUT_READ_METHOD_NEXT_ADDRESS,
	LUT_READ_METHOD_NEXT_L2UC,
	LUT_READ_METHOD_NEXT_L2MC,
	LUT_READ_METHOD_NEXT_L3MC,
	LUT_READ_METHOD_NEXT_L2L3MC,
	LUT_READ_METHOD_NEXT_L2UCSPA,
	LUT_READ_METHOD_END,
	
}rtk_l2_readMethod_t;


/* l2 address table - for all type */
typedef struct rtk_l2_addr_table_s
{
    rtk_lut_entry_type_t entryType;
	rtk_l2_readMethod_t  method;
    union
    {
        rtk_l2_ucastAddr_t   l2UcEntry;
        rtk_l2_mcastAddr_t   l2McEntry;
        rtk_l2_ipMcastAddr_t ipmcEntry;
    }entry;
}rtk_l2_addr_table_t;

/* Weight of each priority source */
typedef struct rtk_qos_priSelWeight_s
{
    uint32 weight_of_portBased;
    uint32 weight_of_dot1q;
    uint32 weight_of_dscp;
    uint32 weight_of_acl;
    uint32 weight_of_lutFwd;
    uint32 weight_of_saBaed;
    uint32 weight_of_vlanBased;
    uint32 weight_of_svlanBased;
    uint32 weight_of_l4Based;
} rtk_qos_priSelWeight_t;

#define APOLLO_SVLAN_STRUCT 1
#if APOLLO_SVLAN_STRUCT

typedef enum rtk_svlan_action_e
{
    SVLAN_ACTION_DROP  = 0,
    SVLAN_ACTION_TRAP,
    SVLAN_ACTION_SVLAN,
    SVLAN_ACTION_SVLAN_AND_KEEP,
    SVLAN_ACTION_PSVID,    
    SVLAN_ACTION_END,    

} rtk_svlan_action_t;
#endif //end APOLLO_SVLAN_STRUCT

#define CLASSIFY_STRUCT 1
#if CLASSIFY_STRUCT
typedef enum rtk_classify_dirct_e
{
    CLASSIFY_DIRECTION_US = 0,
    CLASSIFY_DIRECTION_DS,
    CLASSIFY_DIRECTION_END
}rtk_classify_dirct_t;

#define CLASSIFY_RAW_FIELD_NUMBER 3

typedef struct rtk_classify_raw_field_s
{
    uint16      dataFieldRaw[CLASSIFY_RAW_FIELD_NUMBER];
    uint16      careFieldRaw[CLASSIFY_RAW_FIELD_NUMBER];
} rtk_classify_raw_field_t;

typedef enum rtk_classify_field_type_e
{
    CLASSIFY_FIELD_ETHERTYPE = 0,
    CLASSIFY_FIELD_TOS_DSIDX,
    CLASSIFY_FIELD_TAG_VID,
    CLASSIFY_FIELD_TAG_PRI,
    CLASSIFY_FIELD_INTER_PRI,
    CLASSIFY_FIELD_IS_CTAG,
    CLASSIFY_FIELD_IS_STAG,
    CLASSIFY_FIELD_UNI,
    CLASSIFY_FIELD_PORT_RANGE, /* not support in test chip */
    CLASSIFY_FIELD_IP_RANGE, /* not support in test chip */
    CLASSIFY_FIELD_ACL_HIT, /* not support in test chip */
    CLASSIFY_FIELD_WAN_IF, /* not support in test chip */
    CLASSIFY_FIELD_IP6_MC, /* not support in test chip */
    CLASSIFY_FIELD_IP4_MC, /* not support in test chip */
    CLASSIFY_FIELD_MLD, /* not support in test chip */
    CLASSIFY_FIELD_IGMP, /* not support in test chip */
    CLASSIFY_FIELD_DEI, /* not support in test chip */
    CLASSIFY_FIELD_INNER_VLAN, /* not support in apollomp */
    CLASSIFY_FIELD_EGR_CTAG_IF, /* not support in apollomp */
    CLASSIFY_FIELD_END
} rtk_classify_field_type_t;

typedef struct rtk_classify_value_s
{
    uint16 value;
    uint16 mask;
} rtk_classify_value_t;


typedef struct rtk_classify_field_s
{
    rtk_classify_field_type_t fieldType;
    union
    {
        rtk_classify_value_t etherType;
        rtk_classify_value_t tosDsidx;
        rtk_classify_value_t tagVid;
        rtk_classify_value_t tagPri;
        rtk_classify_value_t interPri;
        rtk_classify_value_t isCtag;
        rtk_classify_value_t isStag;
        rtk_classify_value_t uni;
        rtk_classify_value_t portRange; /* not support in test chip */
        rtk_classify_value_t ipRange; /* not support in test chip */
        rtk_classify_value_t aclHit; /* not support in test chip */
        rtk_classify_value_t wanIf; /* not support in test chip */
        rtk_classify_value_t ip6Mc; /* not support in test chip */
        rtk_classify_value_t ip4Mc; /* not support in test chip */
        rtk_classify_value_t mld; /* not support in test chip */
        rtk_classify_value_t igmp; /* not support in test chip */
        rtk_classify_value_t dei; /* not support in test chip */
        rtk_classify_value_t innerVlan; /* not support in test chip & apollomp */
        rtk_classify_value_t egrCtagif; /* not support in test chip & apollomp */
        rtk_classify_value_t fieldData;

	} classify_pattern;
    struct rtk_classify_field_s *next;
} rtk_classify_field_t;

/*down stream action*/
typedef enum rtk_classify_ds_csact_e
{
    CLASSIFY_DS_CSACT_NOP = 0,
    CLASSIFY_DS_CSACT_ADD_TAG_VS_TPID,
    CLASSIFY_DS_CSACT_ADD_TAG_8100,
    CLASSIFY_DS_CSACT_DEL_STAG,
    CLASSIFY_DS_CSACT_TRANSPARENT, /* not support in test chip */
    CLASSIFY_DS_CSACT_SP2C, /* not support in test chip */
    CLASSIFY_DS_CSACT_END,
} rtk_classify_ds_csact_t;

typedef enum rtk_classify_ds_vid_act_e
{
    CLASSIFY_DS_VID_ACT_NOP = 0,
    CLASSIFY_DS_VID_ACT_ASSIGN,
    CLASSIFY_DS_VID_ACT_FROM_1ST_TAG,
    CLASSIFY_DS_VID_ACT_FROM_2ND_TAG, /* not support in test chip */
    CLASSIFY_DS_VID_ACT_FROM_LUT,
    CLASSIFY_DS_VID_ACT_TRANSLATION_SP2C,
    CLASSIFY_DS_VID_ACT_END,
} rtk_classify_ds_vid_act_t;

typedef enum rtk_classify_ds_pri_act_e
{
    CLASSIFY_DS_PRI_ACT_NOP = 0,
    CLASSIFY_DS_PRI_ACT_ASSIGN,
    CLASSIFY_DS_PRI_ACT_FROM_1ST_TAG,
    CLASSIFY_DS_PRI_ACT_FROM_2ND_TAG, /* not support in test chip */
    CLASSIFY_DS_PRI_ACT_FROM_INTERNAL,
    CLASSIFY_DS_PRI_ACT_TRANSLATION_SP2C,
    CLASSIFY_DS_PRI_ACT_END,
} rtk_classify_ds_pri_act_t;


typedef enum rtk_classify_ds_cact_e
{
    CLASSIFY_DS_CACT_NOP = 0,
    CLASSIFY_DS_CACT_ADD_CTAG_8100,
    CLASSIFY_DS_CACT_TRANSLATION_SP2C,
    CLASSIFY_DS_CACT_DEL_CTAG, /* not support in test chip */
    CLASSIFY_DS_CACT_TRANSPARENT,
    CLASSIFY_DS_CACT_END,
} rtk_classify_ds_cact_t;

typedef enum rtk_classify_cf_pri_act_e
{
    CLASSIFY_CF_PRI_ACT_NOP = 0,
    CLASSIFY_CF_PRI_ACT_ASSIGN,
    CLASSIFY_CF_PRI_ACT_END,
} rtk_classify_cf_pri_act_t;

typedef enum rtk_classify_ds_uni_act_e
{
    CLASSIFY_DS_UNI_ACT_NOP =0 ,
    CLASSIFY_DS_UNI_ACT_MASK_BY_UNIMASK,
    CLASSIFY_DS_UNI_ACT_FORCE_FORWARD,
    CLASSIFY_DS_UNI_ACT_TRAP,
    CLASSIFY_DS_UNI_ACT_END,
} rtk_classify_ds_uni_act_t;

typedef enum rtk_classify_dscp_act_e
{
    CLASSIFY_DSCP_ACT_DISABLE = 0,
    CLASSIFY_DSCP_ACT_ENABLE,
    CLASSIFY_DSCP_ACT_END,
} rtk_classify_dscp_act_t;


typedef struct rtk_classify_ds_act_s
{
    rtk_classify_ds_csact_t   csAct;
    rtk_classify_ds_vid_act_t csVidAct; /* not support in test chip */
    rtk_classify_ds_pri_act_t csPriAct; /* not support in test chip */
    rtk_classify_ds_cact_t    cAct;
    rtk_classify_ds_vid_act_t cVidAct;
    rtk_classify_ds_pri_act_t cPriAct;
    rtk_classify_cf_pri_act_t interPriAct;
    rtk_classify_ds_uni_act_t uniAct;
    rtk_classify_dscp_act_t dscp; /* not support in test chip */
    uint32 sTagVid; /* not support in test chip */
    uint8  sTagPri; /* not support in test chip */
    uint32 cTagVid;
    uint8  cTagPri;
    uint8  cfPri;
    rtk_portmask_t uniMask;
    uint8  dscpVal; /* not support in test chip */
} rtk_classify_ds_act_t;


typedef enum rtk_classify_us_csact_e
{
    CLASSIFY_US_CSACT_NOP = 0,
    CLASSIFY_US_CSACT_ADD_TAG_VS_TPID,
    CLASSIFY_US_CSACT_ADD_TAG_8100,
    CLASSIFY_US_CSACT_DEL_STAG, /* not support in test chip */
    CLASSIFY_US_CSACT_TRANSPARENT, /* not support in test chip */
    CLASSIFY_US_CSACT_END,
} rtk_classify_us_csact_t;

typedef enum rtk_classify_us_vid_act_e
{
	CLASSIFY_US_VID_ACT_NOP = 0, /*not support in Apollomp*/
    CLASSIFY_US_VID_ACT_ASSIGN,
    CLASSIFY_US_VID_ACT_FROM_1ST_TAG,
    CLASSIFY_US_VID_ACT_FROM_2ND_TAG, /* not support in test chip */
    CLASSIFY_US_VID_ACT_FROM_INTERNAL, /* not support in test chip */
    CLASSIFY_US_VID_ACT_END,
} rtk_classify_us_vid_act_t;

typedef enum rtk_classify_us_pri_act_e
{
	CLASSIFY_US_PRI_ACT_NOP = 0, /*not support in Apollomp*/
    CLASSIFY_US_PRI_ACT_ASSIGN,
    CLASSIFY_US_PRI_ACT_FROM_1ST_TAG,
    CLASSIFY_US_PRI_ACT_FROM_2ND_TAG, /* not support in test chip */
    CLASSIFY_US_PRI_ACT_FROM_INTERNAL,
    CLASSIFY_US_PRI_ACT_END,
} rtk_classify_us_pri_act_t;


typedef enum rtk_classify_us_cact_e
{
    CLASSIFY_US_CACT_NOP = 0,
    CLASSIFY_US_CACT_ADD_CTAG_8100, /* not support in test chip */
    CLASSIFY_US_CACT_TRANSLATION_C2S,
    CLASSIFY_US_CACT_DEL_CTAG,
    CLASSIFY_US_CACT_TRANSPARENT,
    CLASSIFY_US_CACT_END,
} rtk_classify_us_cact_t;

typedef enum rtk_classify_drop_act_e
{
    CLASSIFY_DROP_ACT_NONE = 0,
    CLASSIFY_DROP_ACT_ENABLE,
    CLASSIFY_DROP_ACT_TRAP,
    CLASSIFY_DROP_ACT_END,
} rtk_classify_drop_act_t;

typedef enum rtk_classify_log_act_e
{
    CLASSIFY_US_LOG_ACT_NONE = 0,
    CLASSIFY_US_LOG_ACT_ENABLE,
    CLASSIFY_US_LOG_ACT_END,
} rtk_classify_log_act_t;

typedef enum rtk_classify_us_sqid_act_e
{
    CLASSIFY_US_SQID_ACT_ASSIGN_NOP = 0,
    CLASSIFY_US_SQID_ACT_ASSIGN_SID,
    CLASSIFY_US_SQID_ACT_ASSIGN_QID,
    CLASSIFY_US_SQID_ACT_END,
} rtk_classify_us_sqid_act_t;


typedef struct rtk_classify_us_act_s
{
    rtk_classify_us_csact_t     csAct;
    rtk_classify_us_vid_act_t   csVidAct;
    rtk_classify_us_pri_act_t   csPriAct;
    rtk_classify_us_cact_t      cAct;
    rtk_classify_us_sqid_act_t  sidQidAct;
    rtk_classify_us_vid_act_t   cVidAct; /* not support in test chip */
    rtk_classify_us_pri_act_t   cPriAct; /* not support in test chip */
    rtk_classify_cf_pri_act_t   interPriAct; /* not support in test chip */
    rtk_classify_dscp_act_t     dscp; /* not support in test chip */
    rtk_classify_drop_act_t     drop; /* not support in test chip */
    rtk_classify_log_act_t      log; /* not support in test chip */
    uint32 sTagVid;
    uint8  sTagPri;
    uint32 sidQid;
    uint32 cTagVid; /* not support in test chip */
    uint8  cTagPri; /* not support in test chip */
    uint8  cfPri; /* not support in test chip */
    uint8  logCntIdx; /* not support in test chip */
    uint8  dscpVal; /* not support in test chip */

} rtk_classify_us_act_t;


typedef enum rtk_classify_invert_e
{
    CLASSIFY_INVERT_DISABLE = 0,
    CLASSIFY_INVERT_ENABLE,
    CLASSIFY_INVERT_END,
} rtk_classify_invert_t;

#define RTK_MAX_NUM_OF_ACL_RULE_FIELD           8



#endif  //end CLASSIFY_STRUCT

#define ACL_APOLLO_STRUCT 1
#if ACL_APOLLO_STRUCT

typedef enum rtk_field_sel_e
{
    ACL_FORMAT_DEFAULT = 0,
    ACL_FORMAT_RAW,
    ACL_FORMAT_LLC,
    ACL_FORMAT_IPV4,
    ACL_FORMAT_ARP,
    ACL_FORMAT_IPV6,
    ACL_FORMAT_IPPAYLOAD,
    ACL_FORMAT_L4PAYLOAD,  
    ACL_FORMAT_PPPOE,
    ACL_FORMAT_END
}rtk_field_sel_t;

typedef struct rtk_acl_field_entry_s
{
    uint32 index;
    rtk_field_sel_t format;
    uint32 offset;
} rtk_acl_field_entry_t;

typedef enum rtk_filter_portrange_e
{
    PORTRANGE_UNUSED = 0,
    PORTRANGE_SPORT,
    PORTRANGE_DPORT,
    PORTRANGE_END
}rtk_acl_portrange_type_t;

typedef struct rtk_acl_rangeCheck_l4Port_s
{
    uint16  index;
    uint16  upper_bound;    /* Port range upper bound */
    uint16  lower_bound;    /* Port range lower bound */
    rtk_acl_portrange_type_t     type; 
                            /* 0: unused, 
                             * 1: compare source port,
                             * 2: compare destination port
                             */
} rtk_acl_rangeCheck_l4Port_t;


typedef enum rtk_acl_iprange_e
{
    IPRANGE_UNUSED = 0,
    IPRANGE_IPV4_SIP,
    IPRANGE_IPV4_DIP,
    IPRANGE_IPV6_SIP,
    IPRANGE_IPV6_DIP,
    IPRANGE_END
}rtk_acl_iprange_t;


typedef struct rtk_acl_rangeCheck_ip_s
{
    uint16 index;
    /*for IPv6 the address only specify IPv6[31:0]*/
    ipaddr_t upperIp;    /* IP range upper bound */
    ipaddr_t lowerIp;    /* IP range lower bound */
    /*specify IPv6[63:32]*/
    ipaddr_t upperIp2;    /* IP range upper bound */
    ipaddr_t lowerIp2;    /* IP range lower bound */
    
    rtk_acl_iprange_t type; 
} rtk_acl_rangeCheck_ip_t;


typedef enum rtk_filter_unmatch_action_e
{
    FILTER_UNMATCH_DROP = 0,
    FILTER_UNMATCH_PERMIT,
    FILTER_UNMATCH_END,
} rtk_filter_unmatch_action_type_t;


typedef enum rtk_acl_igr_rule_mode_e
{
    ACL_IGR_RULE_MODE_0 = 0,
    ACL_IGR_RULE_MODE_1,
    ACL_IGR_RULE_MODE_END,
} rtk_acl_igr_rule_mode_t;


typedef struct rtk_acl_value_s
{
    uint32 value;
    uint32 mask;
} rtk_acl_value_t;

typedef struct rtk_acl_raw_field_s
{
    rtk_acl_value_t fieldRaw[RTK_MAX_NUM_OF_ACL_RULE_FIELD];
} rtk_acl_raw_field_t;

typedef struct rtk_classify_cfg_s
{
    uint32 index;
    uint32 patternIdx;
    rtk_classify_dirct_t direction;
    //rtk_classify_field_format_t fieldFmt;
    union{
        rtk_classify_field_t *pFieldHead;   /* for set operation */
        rtk_classify_raw_field_t readField; /* for get operation */
	} field;
    union{
        rtk_classify_ds_act_t dsAct;
        rtk_classify_us_act_t usAct;
	} act;
	rtk_classify_invert_t invert;
    rtk_enable_t valid; /* for get operation */
} rtk_classify_cfg_t;

typedef enum rtk_acl_field_type_e
{
    ACL_FIELD_UNUSED = 0,
    ACL_FIELD_DMAC,
    ACL_FIELD_DMAC0,
    ACL_FIELD_DMAC1,
    ACL_FIELD_DMAC2,
    ACL_FIELD_SMAC,
    ACL_FIELD_SMAC0,
    ACL_FIELD_SMAC1,
    ACL_FIELD_SMAC2,
    ACL_FIELD_ETHERTYPE,
    ACL_FIELD_CTAG,
    ACL_FIELD_STAG,
    ACL_FIELD_GEMPORT,

    ACL_FIELD_IPV4_SIP,
    ACL_FIELD_IPV4_SIP0,
    ACL_FIELD_IPV4_SIP1,
    ACL_FIELD_IPV4_DIP,
    ACL_FIELD_IPV4_DIP0,
    ACL_FIELD_IPV4_DIP1,
	ACL_FIELD_IPV4_PROTOCOL,
	ACL_FIELD_IP_PROTOCOL_NEXT_HEADER,    
    ACL_FIELD_IPV6_SIP,
    ACL_FIELD_IPV6_SIP0,
    ACL_FIELD_IPV6_SIP1,
	ACL_FIELD_IPV6_SIP2,
	ACL_FIELD_IPV6_SIP3,
	ACL_FIELD_IPV6_SIP4,
	ACL_FIELD_IPV6_SIP5,
	ACL_FIELD_IPV6_SIP6,
	ACL_FIELD_IPV6_SIP7,
    ACL_FIELD_IPV6_DIP,
    ACL_FIELD_IPV6_DIP0,
    ACL_FIELD_IPV6_DIP1,
	ACL_FIELD_IPV6_DIP2,
	ACL_FIELD_IPV6_DIP3,
	ACL_FIELD_IPV6_DIP4,
	ACL_FIELD_IPV6_DIP5,
	ACL_FIELD_IPV6_DIP6,
	ACL_FIELD_IPV6_DIP7,
    ACL_FIELD_IPV6_NEXT_HEADER,

    ACL_FIELD_TCP_SPORT,
    ACL_FIELD_TCP_DPORT,
    ACL_FIELD_UDP_SPORT,
    ACL_FIELD_UDP_DPORT,

    ACL_FIELD_VID_RANGE,
    ACL_FIELD_IP_RANGE,
    ACL_FIELD_PORT_RANGE,
    ACL_FIELD_PKT_LEN_RANGE,

    ACL_FIELD_EXT_PORTMASK,
    
    ACL_FIELD_USER_VALID,
    ACL_FIELD_USER_DEFINED00,
    ACL_FIELD_USER_DEFINED01,
    ACL_FIELD_USER_DEFINED02,
    ACL_FIELD_USER_DEFINED03,
    ACL_FIELD_USER_DEFINED04,
    ACL_FIELD_USER_DEFINED05,
    ACL_FIELD_USER_DEFINED06,
    ACL_FIELD_USER_DEFINED07,
    ACL_FIELD_USER_DEFINED08,
    ACL_FIELD_USER_DEFINED09,
    ACL_FIELD_USER_DEFINED10,
    ACL_FIELD_USER_DEFINED11,
    ACL_FIELD_USER_DEFINED12,
    ACL_FIELD_USER_DEFINED13,
    ACL_FIELD_USER_DEFINED14,
    ACL_FIELD_USER_DEFINED15,

    ACL_FIELD_PATTERN_MATCH,

    ACL_FIELD_END,
} rtk_acl_field_type_t;

typedef struct rtk_acl_mac_s
{
    rtk_mac_t value;
    rtk_mac_t mask;
} rtk_acl_mac_t;

typedef struct rtk_acl_tag_s
{
    rtk_acl_value_t pri;
    rtk_acl_value_t cfi_dei;
    rtk_acl_value_t vid;
} rtk_acl_tag_t;


typedef struct rtk_acl_ip_s
{
    ipaddr_t value;
    ipaddr_t mask;
} rtk_acl_ip_t;

typedef struct rtk_acl_ip6_s
{
    rtk_ipv6_addr_t value;
    rtk_ipv6_addr_t mask;
} rtk_acl_ip6_t;

typedef struct rtk_acl_data_field_s
{
    rtk_acl_value_t data;
    uint16          fieldIdx;
} rtk_acl_data_field_t;


typedef struct rtk_acl_field_s
{
    rtk_acl_field_type_t fieldType;

    union
    {
        /*for type
         ACL_FIELD_DMAC
         ACL_FIELD_SMAC
        */
        rtk_acl_mac_t     mac;

        /*for type
         ACL_FIELD_CTAG
         ACL_FIELD_STAG,
        */
        rtk_acl_tag_t   l2tag;

        /*for type
         ACL_FIELD_IPV4_SIP
         ACL_FIELD_IPV4_DIP
         ACL_FIELD_IPV6_SIPV6 for IPv6 only specify IPv6[31:0]
         ACL_FIELD_IPV6_DIPV6
        */
        rtk_acl_ip_t    ip;

        rtk_acl_ip6_t   ip6;
        /*for type
         ACL_FIELD_ETHERTYPE
         ACL_FIELD_IPV6_NEXT_HEADER
         ACL_FIELD_TCP_SPORT
         ACL_FIELD_TCP_DPORT
         ACL_FIELD_UDP_SPORT
         ACL_FIELD_UDP_DPORT
         ACL_FIELD_VID_RANGE
         ACL_FIELD_IP_RANGE
         ACL_FIELD_PORT_RANGE
         ACL_FIELD_PKT_LEN_RANGE
         ACL_FIELD_USER_DEFINED00
         ACL_FIELD_USER_DEFINED01
         ACL_FIELD_USER_DEFINED02
         ACL_FIELD_USER_DEFINED03
         ACL_FIELD_USER_DEFINED04
         ACL_FIELD_USER_DEFINED05
         ACL_FIELD_USER_DEFINED06
         ACL_FIELD_USER_DEFINED07
         ACL_FIELD_USER_DEFINED08
         ACL_FIELD_USER_DEFINED09
         ACL_FIELD_USER_DEFINED10
         ACL_FIELD_USER_DEFINED11
         ACL_FIELD_USER_DEFINED12
         ACL_FIELD_USER_DEFINED13
         ACL_FIELD_USER_DEFINED14
         ACL_FIELD_USER_DEFINED15
        */
        /*for range check please use value field to assign range check care entry*/
        rtk_acl_value_t   data;

        /*for user assign field index*/
        /*for type
         ACL_FIELD_PATTERN_MATCH
        */
        rtk_acl_data_field_t   pattern;
	} fieldUnion;

    struct rtk_acl_field_s *next;
}rtk_acl_field_t;


typedef enum rtk_acl_care_tag_index_e
{
    ACL_CARE_TAG_PPPOE = 0,
    ACL_CARE_TAG_CTAG,
    ACL_CARE_TAG_STAG,
    ACL_CARE_TAG_IPV4,
    ACL_CARE_TAG_IPV6,
    ACL_CARE_TAG_TCP,
    ACL_CARE_TAG_UDP,
    ACL_CARE_TAG_END,
} rtk_acl_care_tag_index_t;

typedef struct rtk_filter_care_tag_s
{
    rtk_acl_value_t tags[ACL_CARE_TAG_END];
} rtk_acl_care_tag_t;



typedef enum rtk_acl_igr_cvlan_act_ctrl_e
{
    ACL_IGR_CVLAN_IGR_CVLAN_ACT = 0,
    ACL_IGR_CVLAN_EGR_CVLAN_ACT,
    ACL_IGR_CVLAN_DS_SVID_ACT,
    ACL_IGR_CVLAN_POLICING_ACT,
    ACL_IGR_CVLAN_MIB_ACT,
    ACL_IGR_CVLAN_1P_REMARK_ACT,
	ACL_IGR_CVLAN_BW_METER_ACT,    
    ACL_IGR_CVLAN_ACT_END,
} rtk_acl_igr_cvlan_act_ctrl_t;


typedef struct rtk_acl_igr_cvlan_act_s
{
    rtk_acl_igr_cvlan_act_ctrl_t act; 
    uint16                       cvid;
    uint8                        meter;
    uint8                        mib;
    uint8                        dot1p;
    
} rtk_acl_igr_cvlan_act_t;

typedef enum rtk_acl_igr_svlan_act_ctrl_e
{
    ACL_IGR_SVLAN_IGR_SVLAN_ACT = 0,
    ACL_IGR_SVLAN_EGR_SVLAN_ACT,
    ACL_IGR_SVLAN_US_CVID_ACT,
    ACL_IGR_SVLAN_POLICING_ACT,
    ACL_IGR_SVLAN_MIB_ACT,
    ACL_IGR_SVLAN_1P_REMARK_ACT,
    ACL_IGR_SVLAN_DSCP_REMARK_ACT,
    ACL_IGR_SVLAN_ROUTE_ACT,
	ACL_IGR_SVLAN_BW_METER_ACT,
    ACL_IGR_SVLAN_ACT_END,
} rtk_acl_igr_svlan_act_ctrl_t;

typedef struct rtk_acl_igr_svlan_act_s
{
    rtk_acl_igr_svlan_act_ctrl_t act; 
    uint16                       svid;
    uint8                        meter;
    uint8                        mib;
    uint8                        dot1p;
    uint8                        dscp;
	uint8                      	 nexthop;
} rtk_acl_igr_svlan_act_t;

typedef enum rtk_acl_igr_pri_act_ctrl_e
{
    ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT = 0,
    ACL_IGR_PRI_DSCP_REMARK_ACT,
    ACL_IGR_PRI_1P_REMARK_ACT,
    ACL_IGR_PRI_POLICING_ACT,
    ACL_IGR_PRI_MIB_ACT,
    ACL_IGR_PRI_ROUTE_ACT,
    ACL_IGR_PRI_BW_METER_ACT,
    ACL_IGR_PRI_ACT_END,
} rtk_acl_igr_pri_act_ctrl_t;


typedef struct rtk_acl_igr_pri_act_s
{
    rtk_acl_igr_pri_act_ctrl_t act; 
    uint8                      aclPri;
    uint8                      dscp;
    uint8                      dot1p;
    uint8                      meter;
    uint8                      mib;
	uint8                      nexthop;
} rtk_acl_igr_pri_act_t;


typedef enum rtk_acl_igr_log_act_ctrl_e
{
    ACL_IGR_LOG_POLICING_ACT = 0,
    ACL_IGR_LOG_MIB_ACT,
	ACL_IGR_LOG_BW_METER_ACT,
    ACL_IGR_LOG_ACT_END,
} rtk_acl_igr_log_act_ctrl_t;


typedef struct rtk_acl_igr_log_act_s
{
    rtk_acl_igr_log_act_ctrl_t act; 
    uint8                      meter;
    uint8                      mib;
} rtk_acl_igr_log_act_t;


typedef enum rtk_acl_igr_forward_act_ctrl_e
{
    ACL_IGR_FORWARD_COPY_ACT = 0,
    ACL_IGR_FORWARD_REDIRECT_ACT,
    ACL_IGR_FORWARD_IGR_MIRROR_ACT,
    ACL_IGR_FORWARD_TRAP_ACT,
    ACL_IGR_FORWARD_DROP_ACT,    
	ACL_IGR_FORWARD_EGRESSMASK_ACT,	 
    ACL_IGR_FORWARD_ACT_END,
} rtk_acl_igr_forward_act_ctrl_t;



typedef struct rtk_acl_igr_forward_act_s
{
    rtk_acl_igr_forward_act_ctrl_t act; 
    rtk_portmask_t                 portMask;
} rtk_acl_igr_forward_act_t;

typedef enum rtk_acl_igr_extend_act_ctrl_e
{
    ACL_IGR_EXTEND_NONE_ACT = 0,
    ACL_IGR_EXTEND_SID_ACT,
    ACL_IGR_EXTEND_LLID_ACT,
    ACL_IGR_EXTEND_EXT_ACT,
    ACL_IGR_EXTEND_ACT_END,
} rtk_acl_igr_extend_act_ctrl_t;


typedef struct rtk_acl_igr_extend_act_s
{
    rtk_acl_igr_extend_act_ctrl_t   act; 
    uint8                           index;
    rtk_portmask_t                  portMask;
} rtk_acl_igr_extend_act_t;


typedef enum rtk_acl_igr_act_type_e
{
    ACL_IGR_CVLAN_ACT = 0,
    ACL_IGR_SVLAN_ACT,
    ACL_IGR_PRI_ACT,
    ACL_IGR_LOG_ACT,
    ACL_IGR_FORWARD_ACT,
    ACL_IGR_INTR_ACT,
    ACL_IGR_ROUTE_ACT,
    ACL_IGR_ACT_END,
} rtk_acl_igr_act_type_t;


typedef struct rtk_acl_igr_act_s
{
    rtk_enable_t              enableAct[ACL_IGR_ACT_END];
    rtk_acl_igr_cvlan_act_t   cvlanAct;
    rtk_acl_igr_svlan_act_t   svlanAct;
    rtk_acl_igr_pri_act_t     priAct;
    rtk_acl_igr_log_act_t     logAct;
    rtk_acl_igr_forward_act_t forwardAct;
    rtk_acl_igr_extend_act_t  extendAct; 
    rtk_enable_t              aclInterrupt;  
    rtk_enable_t              aclLatch;  
	uint8                     nexthop;

} rtk_acl_igr_act_t;


typedef enum rtk_acl_invert_e
{
    ACL_INVERT_DISABLE = 0,
    ACL_INVERT_ENABLE,
    ACL_INVERT_END,
} rtk_acl_invert_t;

typedef struct rtk_acl_ingress_entry_s
{
    uint16                  index;

    rtk_acl_field_t         *pFieldHead;
    rtk_acl_raw_field_t     readField;
	
	uint16                  templateIdx;
    rtk_acl_care_tag_t      careTag;
    rtk_portmask_t          activePorts;
    rtk_acl_igr_act_t       act;
    rtk_acl_invert_t        invert;

    rtk_enable_t            valid;
} rtk_acl_ingress_entry_t;


typedef struct rtk_acl_template_s
{
    uint32 index;
    rtk_acl_field_type_t fieldType[RTK_MAX_NUM_OF_ACL_RULE_FIELD];
} rtk_acl_template_t;
#endif //end ACL_APOLLO_STRUCT

typedef struct rtk_vlan_protoVlanCfg_s
{
    uint32      valid;
    rtk_vlan_t  vid;    
    rtk_pri_t   pri;
    uint32      dei;
} rtk_vlan_protoVlanCfg_t;

/* frame type of protocol vlan - reference 802.1v standard */
typedef enum rtk_vlan_protoVlan_frameType_e
{
    FRAME_TYPE_ETHERNET = 0,
    FRAME_TYPE_RFC1042,
    FRAME_TYPE_SNAP8021H,/* Not supported by any chip */
    FRAME_TYPE_SNAPOTHER,/* Not supported by any chip */
    FRAME_TYPE_LLCOTHER,
    FRAME_TYPE_END
} rtk_vlan_protoVlan_frameType_t;

/* Content of protocol vlan group */
typedef struct rtk_vlan_protoGroup_s
{
    rtk_vlan_protoVlan_frameType_t  frametype;
    uint32                          framevalue;
} rtk_vlan_protoGroup_t;
#endif /*CONFIG_XDSL_RG_DIAGSHELL */

typedef enum bcast_msg_type_e
{
	// interrupt
	MSG_TYPE_LINK_CHANGE = 0,
	MSG_TYPE_METER_EXCEED,
	MSG_TYPE_LEARN_OVER,
	MSG_TYPE_SPEED_CHANGE,
	MSG_TYPE_SPECIAL_CONGEST,
	MSG_TYPE_LOOP_DETECTION,
	MSG_TYPE_CABLE_DIAG_FIN,
	MSG_TYPE_ACL_ACTION,
	MSG_TYPE_GPHY,
	MSG_TYPE_SERDES,
	MSG_TYPE_GPON,
	MSG_TYPE_EPON,
	MSG_TYPE_PTP,
	MSG_TYPE_DYING_GASP,
	MSG_TYPE_THERMAL,
	MSG_TYPE_ADC,
	MSG_TYPE_EEPROM_UPDATE_110OR118,
	MSG_TYPE_EEPROM_UPDATE_128TO247,
	MSG_TYPE_PKTBUFFER_ERROR,
	MSG_TYPE_ALL,
	// gpon event
	MSG_TYPE_ONU_STATE = 21,	
	MSG_TYPE_END
} bcast_msg_type_t;

// interrupt broadcaster message
typedef struct {
    bcast_msg_type_t     intrType;
    uint32              intrSubType;
    uint32              intrBitMask;
    rtk_enable_t        intrStatus;
} intrBcasterMsg_t;

typedef void (*intrBcasterNotifierCb_t)(intrBcasterMsg_t   *pMsgData);


// interrupt broadcaster notifier
typedef struct {
    bcast_msg_type_t             notifyType;
    intrBcasterNotifierCb_t     notifierCb;

    // for internal use only
    struct notifier_block       *pNbAddr;
} intrBcasterNotifier_t;

//copy from fwdEngine

typedef struct dma_tx_desc {
	unsigned int		opts1;
	unsigned int		addr;
	unsigned int		opts2;
	unsigned int		opts3; //cputag
	unsigned int		opts4; //lso
}DMA_TX_DESC;

typedef struct dma_rx_desc {
	unsigned int		opts1;
	unsigned int		addr;
	unsigned int		opts2;
	unsigned int		opts3;
}DMA_RX_DESC;

#ifndef _ASM_TYPES_H
typedef unsigned int dma_addr_t;
#endif
struct ring_info {
	struct sk_buff		*skb;
	//dma_addr_t		mapping;
	unsigned int		mapping;
	unsigned		frag;
};

struct cp_extra_stats {
	unsigned long		rx_frags;
	unsigned long tx_timeouts;
	//krammer add for rx info
	unsigned int rx_hw_num;
	unsigned int rx_sw_num;
	unsigned int rer_runt;
	unsigned int rer_ovf;
	unsigned int rdu;
	unsigned int frag;
#ifdef CONFIG_RG_JUMBO_FRAME
	unsigned int toobig;
#endif
	unsigned int crcerr;
	unsigned int rcdf;
	unsigned int rx_no_mem;
	//krammer add for tx info
	unsigned int tx_sw_num;
	unsigned int tx_hw_num;
	unsigned int tx_no_desc;
};


#ifdef __KERNEL__
#define MAX_RXRING_NUM 6
#define MAX_TXRING_NUM 5
#define SW_PORT_NUM 7
#ifndef CONFIG_LINUX_USER_SHELL
struct rx_info{
	union{
		struct{
			unsigned int own:1;//31
			unsigned int eor:1;//30
			unsigned int fs:1;//29
			unsigned int ls:1;//28
			unsigned int crcerr:1;//27
			unsigned int ipv4csf:1;//26
			unsigned int l4csf:1;//25
			unsigned int rcdf:1;//24
			unsigned int ipfrag:1;//23
			unsigned int pppoetag:1;//22
			unsigned int rwt:1;//21
			unsigned int pkttype:4;//17~20
			unsigned int l3routing:1;//16
			unsigned int origformat:1;//15
			unsigned int pctrl:1;//14
#ifdef CONFIG_RG_JUMBO_FRAME
			unsigned int data_length:14;//0~13
#else
			unsigned int rsvd:2;//12~13
			unsigned int data_length:12;//0~11
#endif
		}bit;
		unsigned int dw;//double word
	}opts1;
	unsigned int addr;
	union{
		struct{
			unsigned int cputag:1;//31
			unsigned int ptp_in_cpu_tag_exist:1;//30
			unsigned int svlan_tag_exist:1;//29
			unsigned int rsvd_2:2;//27~28
			unsigned int pon_stream_id:7;//20~26
			unsigned int rsvd_1:3;//17~19
			unsigned int ctagva:1;//16
			unsigned int cvlan_tag:16;//0~15
		}bit;
		unsigned int dw;//double word
	}opts2;
	union{
		struct{
			unsigned int src_port_num:5;//27~31
			unsigned int dst_port_mask:6;//21~26
			unsigned int reason:8;//13~20
			unsigned int internal_priority:3;//10~12
			unsigned int ext_port_ttl_1:5;//5~9
			unsigned int rsvd:5;//0~4
		}bit;
		unsigned int dw;//double word
	}opts3;
};


struct tx_info{
	union{
		struct{
			unsigned int own:1;//31
			unsigned int eor:1;//30
			unsigned int fs:1;//29
			unsigned int ls:1;//28
			unsigned int ipcs:1;//27
			unsigned int l4cs:1;//26
			unsigned int keep:1;//25
			unsigned int blu:1;//24
			unsigned int crc:1;//23
			unsigned int vsel:1;//22
			unsigned int dislrn:1;//21
			unsigned int cputag_ipcs:1;//20
			unsigned int cputag_l4cs:1;//19
			unsigned int cputag_psel:1;//18
			unsigned int rsvd:1;//17
			unsigned int data_length:17;//0~16
		}bit;
		unsigned int dw;//double word
	}opts1;
	unsigned int addr;
	union{
		struct{
			unsigned int cputag:1;//31
			unsigned int aspri:1;//30
			unsigned int cputag_pri:3;//27~29
			unsigned int tx_vlan_action:2;//25~26
			unsigned int tx_pppoe_action:2;//23~24
			unsigned int tx_pppoe_idx:3;//20~22
			unsigned int efid:1;//19
			unsigned int enhance_fid:3;//16~18
			unsigned int vidl:8;//8~15
			unsigned int prio:3;//5~7
			unsigned int cfi:1;// 4
			unsigned int vidh:4;//0~3
		}bit;
		unsigned int dw;//double word
	}opts2;
	union{
		struct{
			unsigned int extspa:3;//29~31
			unsigned int tx_portmask:6;//23~28
			unsigned int tx_dst_stream_id:7;//16~22
			#if 0//def RTL0371
			unsigned int tx_dst_vc_mask:16;//0~15
			#else
			unsigned int rsvd:12;// 4~15
			unsigned int rsv1:1;// 3
			unsigned int rsv0:1;// 2
			unsigned int l34_keep:1;// 1
			unsigned int ptp:1;//0
			#endif
		}bit;
		unsigned int dw;//double word
	}opts3;
	union{
		unsigned int dw;
	}opts4;
};

#include <linux/interrupt.h>
struct re_private {
	void			*regs;
	struct net_device	*dev;
	spinlock_t		lock;

    DMA_RX_DESC     *rx_Mring[MAX_RXRING_NUM];
    unsigned		rx_Mtail[MAX_RXRING_NUM];
    char*			rxdesc_Mbuf[MAX_RXRING_NUM];

    DMA_TX_DESC		*tx_Mhqring[MAX_TXRING_NUM];
	char*			txdesc_Mbuf[MAX_TXRING_NUM];
	unsigned		tx_Mhqhead[MAX_TXRING_NUM];
	unsigned		tx_Mhqtail[MAX_TXRING_NUM];

	struct ring_info	*tx_skb[MAX_TXRING_NUM];
	struct ring_info	*rx_skb[MAX_RXRING_NUM];

	#ifdef CONFIG_DUALBAND_CONCURRENT
	DMA_RX_DESC     	*default_rx_desc;
	char				*default_rxdesc_Mbuf;
	struct ring_info	*default_rx_skb;
	unsigned 		old_tx_Mhqhead[MAX_TXRING_NUM];
	unsigned 		old_tx_Mhqtail[MAX_TXRING_NUM];
	unsigned		old_rx_Mtail[MAX_RXRING_NUM];
	#endif

	
	unsigned		rx_buf_sz;
	dma_addr_t		ring_dma;
	u32			msg_enable;

	struct cp_extra_stats	cp_stats;
	u32 isr_status;
	u32 isr1_status;

	struct pci_dev		*pdev;
	u32			rx_config;

	struct sk_buff		*frag_skb;
	unsigned		dropping_frag : 1;

	//struct tq_struct	rx_task;
	//struct tq_struct	tx_task;
	struct tasklet_struct rx_tasklets;
	//struct tasklet_struct tx_tasklets;

#if 1	
	struct tasklet_struct tx_tasklets; 
#endif	
	struct net_device* port2dev[SW_PORT_NUM];
	int (*port2rxfunc[SW_PORT_NUM])(struct re_private *cp, struct sk_buff *skb, struct rx_info *pRxInfo);

#ifdef CONFIG_RG_SIMPLE_PROTOCOL_STACK
	struct net_device* multiWanDev[8];		//at most 7 WAN interface(reserved one interface for WAN)
	int wanInterfaceIdx;				//used for multi-WAN in SPS
#endif

#ifdef CONFIG_RG_JUMBO_FRAME
	struct sk_buff *jumboFrame;
	unsigned short jumboLength;
#endif

};
#endif
#endif

enum RTL8370_LUTREADMETHOD{

	LUTREADMETHOD_MAC =0, 		
	LUTREADMETHOD_ADDRESS, 		
};
#define RTK_FID_MAX                                 0xF
typedef struct LUTTABLE{

	ipaddr_t sip;
	ipaddr_t dip;
	rtk_mac_t mac;
	
	uint16 	static_bit:1;
	uint16 	block:1;
	uint16 	auth:1;
	uint16 	macpri:3;
	uint16 	sa_en:1;
	uint16 	da_en:1;
	uint16 	portmask;
	uint16 	spa:4;
	uint16 	age:3;
	uint16 	fid:12;
	uint16 	efid:3;
	uint16 	ipmul:1;
        uint16  valid:1;    
	uint16 	lookup_hit:1;
	uint16 	address:14;
	
}rtl8370_luttb;

typedef enum rtk_l34_routing_type_e
{
    L34_PROCESS_CPU  = 0,
    L34_PROCESS_DROP = 1,
    L34_PROCESS_ARP =  2,
    L34_PROCESS_NH =   3,
    L34_PROCESS_END
}rtk_l34_routing_type_t;

typedef enum rtk_l34_ipv6RouteType_e
{
    L34_IPV6_ROUTE_TYPE_TRAP    = 0,
    L34_IPV6_ROUTE_TYPE_DROP    = 1,
    L34_IPV6_ROUTE_TYPE_LOCAL   = 2,
    L34_IPV6_ROUTE_TYPE_GLOBAL  = 3,
    L34_IPV6_ROUTE_TYPE_END
} rtk_l34_ipv6RouteType_t;

typedef enum rtk_l34_extip_table_type_e
{
    L34_EXTIP_TYPE_NAPT,
    L34_EXTIP_TYPE_NAT,
    L34_EXTIP_TYPE_LP,
    L34_EXTIP_TYPE_END,
}rtk_l34_extip_table_type_t;

typedef enum rtk_l34_nexthop_type_e
{
    L34_NH_ETHER  = 0,
    L34_NH_PPPOE  = 1,
    L34_NH_END
}rtk_l34_nexthop_type_t;

typedef enum rtk_l34_wanType_e
{
    L34_WAN_TYPE_L2_BRIDGE         = 0,
    L34_WAN_TYPE_L3_ROUTE          = 1,
    L34_WAN_TYPE_L34NAT_ROUTE      = 2,
    L34_WAN_TYPE_L34_CUSTOMIZED    = 3,
    L34_WAN_TYPE_END
} rtk_l34_wanType_t;

typedef enum rtk_l34_bindProto_e
{
    L34_BIND_PROTO_NOT_IPV4_IPV6 = 0,  /*for other ether type*/
    L34_BIND_PROTO_NOT_IPV6      = 1,  /*for IPv4 and other*/
    L34_BIND_PROTO_NOT_IPV4      = 2,  /*for IPv6 and other*/
    L34_BIND_PROTO_ALL           = 3,  /*for IPv4, IPv6 and other*/
    L34_BIND_PROTO_TYPE_END
} rtk_l34_bindProto_t;

typedef enum rtk_l34_bindAct_e
{
	L34_BIND_ACT_DROP = 0,
	L34_BIND_ACT_TRAP,
	L34_BIND_ACT_FORCE_L2BRIDGE,
	L34_BIND_ACT_PERMIT_L2BRIDGE,
	L34_BIND_ACT_IPV4_LOOKUPL4TABLE_IPV6_TRAP,
	L34_BIND_ACT_FORCE_BINDL3_SKIP_LOOKUPL4,
	L34_BIND_ACT_FORCE_BINDL3,
	L34_BIND_ACT_NORMAL_LOOKUPL34,
	L34_BIND_ACT_END
}rtk_l34_bindAct_t;

typedef enum rtk_l34_bindType_e
{
    L34_BIND_UNMATCHED_L2L3  = 0,
    L34_BIND_UNMATCHED_L2L34,
    L34_BIND_UNMATCHED_L3L2,
    L34_BIND_UNMATCHED_L3L34,
    L34_BIND_UNMATCHED_L34L2,
    L34_BIND_UNMATCHED_L34L3,
    L34_BIND_UNMATCHED_L3L3,
    L34_BIND_CUSTOMIZED_L2,
    L34_BIND_CUSTOMIZED_L3,
    L34_BIND_CUSTOMIZED_L34,
    L34_BIND_TYPE_END
} rtk_l34_bindType_t;

typedef enum rtk_l34_lookupMode_e{
	L34_LOOKUP_MODE_START=0,
	L34_LOOKUP_VLAN_BASE=L34_LOOKUP_MODE_START,
	L34_LOOKUP_PORT_BASE,
	L34_LOOKUP_MAC_BASE,
	L34_LOOKUP_MODE_END
}rtk_l34_lookupMode_t;

typedef enum rtk_l34_globalStateType_e{
	L34_GLOBAL_STATE_START=0,
	L34_GLOBAL_L34_STATE=L34_GLOBAL_STATE_START,
	L34_GLOBAL_L3NAT_STATE,
	L34_GLOBAL_L4NAT_STATE,
	L34_GLOBAL_L3CHKSERRALLOW_STATE,
	L34_GLOBAL_L4CHKSERRALLOW_STATE,
	L34_GLOBAL_NAT2LOG_STATE,
	L34_GLOBAL_TTLMINUS_STATE,
	L34_GLOBAL_FRAG2CPU_STATE,
	L34_GLOBAL_BIND_STATE,
	L34_GLOBAL_PPPKEEP_STATE,
	L34_GLOBAL_KEEP_ORG_STATE,
	L34_GLOBAL_ADV_PPPOE_MTU_CAL_STATE,
	L34_GLOBAL_STATE_END
}rtk_l34_globalStateType_t;

typedef enum rtk_l34_pppoeAct_t
{
    L34_PPPOE_ACT_NO     = 0,
    L34_PPPOE_ACT_REMOVE = 1,
    L34_PPPOE_ACT_MODIFY = 2,
    L34_PPPOE_ACT_END
} rtk_l34_pppoeAct_t;

typedef enum rtk_l34_l4_trf_e
{
    RTK_L34_L4TRF_TABLE0  = 0,
    RTK_L34_L4TRF_TABLE1  = 1,
    RTK_L34_L4TRF_TABLE_END,
} rtk_l34_l4_trf_t;

typedef enum rtk_l34_hsba_mode_e
{
    L34_HSBA_BOTH_LOG,
    L34_HSBA_NO_LOG,
    L34_HSBA_LOG_ALL,
    L34_HSBA_LOG_FIRST_DROP,
    L34_HSBA_LOG_FIRST_PASS,
    L34_HSBA_LOG_FIRST_TO_CPU ,
    L34_HSBA_LOG,
    L34_HSBA_END
} rtk_l34_hsba_mode_t;

typedef enum rtk_l34_wanRouteMode_e{
	L34_WANROUTE_START=0,
	L34_WANROUTE_FORWARD=L34_WANROUTE_START,
	L34_WANROUTE_FORWARD2CPU,
	L34_WANROUTE_DROP,
	L34_WANROUTE_END
}rtk_l34_wanRouteMode_t;

typedef enum rtk_l34_table_type_e
{
    L34_ROUTING_TABLE,
    L34_PPPOE_TABLE,
    L34_NEXTHOP_TABLE,
    L34_NETIF_TABLE,
    L34_INTIP_TABLE,
    L34_ARP_TABLE,
    L34_NAPTR_TABLE,
    L34_NAPT_TABLE,
    L34_IPV6_ROUTING_TABLE,
    L34_BINDING_TABLE,
    L34_IPV6_NEIGHBOR_TABLE,
    L34_WAN_TYPE_TABLE,
    L34_FLOW_ROUTING_TABLE,
    L34_TABLE_END
}rtk_l34_table_type_t;

typedef enum rtk_l34_hsa_pppoe_act_e
{
    L34_HSA_PPPOE_INACT,
    L34_HSA_PPPOE_TAGGING,
    L34_HSA_PPPOE_REMOVE,
    L34_HSA_PPPOE_MODIFY,
    L34_HSA_END
} rtk_l34_hsa_pppoe_act_t;

typedef enum rtk_l34_hsb_type_e
{
    L34_HSB_ETHERNET,
    L34_HSB_RESERVED_PPTP,
    L34_HSB_IPV4,
    L34_HSB_ICMP,
    L34_HSB_IGMP,
    L34_HSB_TCP,
    L34_HSB_UDP,
    L34_HSB_IPV6,
    L34_HSB_END
} rtk_l34_hsb_type_t;

typedef enum rtk_l34_netifType_e{
	L34_NETIF_TYPE_START=0,
	L34_NETIF_TYPE_WAN=L34_NETIF_TYPE_START,
	L34_NETIF_TYPE_LAN,
	L34_NETIF_TYPE_END,
}rtk_l34_netifType_t;

typedef enum rtk_l34_natType_e{
	L34_NAT_TYPE_START=0,
	L34_NAPT_PORT_RESTRICTED=L34_NAT_TYPE_START,
	L34_NAPT_RESTRICTED_CONE,
	L34_NAPT_FULL_CONE,
	L34_LOCAL_PUBLIC,
	L34_NAT,
	L34_NAT_TYPE_END
}rtk_l34_natType_t;

typedef enum rtk_l34_portWanMapType_e{
	L34_PORTMAP_TYPE_START=0,
	L34_PORTMAP_TYPE_PORT2WAN = L34_PORTMAP_TYPE_START,
	L34_PORTMAP_TYPE_EXT2WAN,
	L34_PORTMAP_TYPE_WAN2PORT,
	L34_PORTMAP_TYPE_WAN2EXT,
	L34_PORTMAP_TYPE_END
}rtk_l34_portWanMapType_t;

typedef enum rtk_l34_portWanMapAct_e{
	L34_PORTMAP_ACT_START=0,
	L34_PORTMAP_ACT_DROP = L34_PORTMAP_ACT_START,
	L34_PORTMAP_ACT_PERMIT,
	L34_PORTMAP_ACT_END
}rtk_l34_portWanMapAct_t;

typedef enum rtk_l34_portType_e{
	L34_PORT_TYPE_START=0,
	L34_PORT_MAC=L34_PORT_TYPE_START,
	L34_PORT_EXTENSION,
	L34_PORT_DSLVC,
	L34_PORT_TYPE_END
}rtk_l34_portType_t;

typedef enum rtk_l34_routeType_e{
	L34_ROUTE_TYPE_START=0,
	L34_ROUTE_CPU=L34_ROUTE_TYPE_START,
	L34_ROUTE_DROP,
	L34_ROUTE_STATIC,						/*will set to route index 6*/
	L34_ROUTE_DEFAULT,						/*will set to route index 7*/
	L34_ROUTE_LOCAL,
	L34_ROUTE_POLICY,
	L34_ROUTE_PPP,
	L34_ROUTE_TYPE_END
}rtk_l34_routeType_t;

typedef enum rtk_l34_arp_trf_e
{
    RTK_L34_ARPTRF_TABLE0  = 0,
    RTK_L34_ARPTRF_TABLE1  = 1,
    RTK_L34_ARPTRF_TABLE_END,
} rtk_l34_arp_trf_t;

typedef enum rtk_l34_flowRoute_type_e
{
    L34_FLOWRT_TYPE_CPU  	= 0,
    L34_FLOWRT_TYPE_DROP 	= 1,
    L34_FLOWRT_TYPE_LOCAL 	= 2,
    L34_FLOWRT_TYPE_GLOBAL 	= 3,
    L34_FLOWRT_TYPE_END
} rtk_l34_flowRoute_type_t;

typedef enum rtk_l34_dsliteTcOpt_s
{
	RTK_L34_DSLITE_TC_OPT_ASSIGN = 0,
	RTK_L34_DSLITE_TC_OPT_COPY_FROM_TOS,
	RTK_L34_DSLITE_TC_OPT_END
}rtk_l34_dsliteTcOpt_t;

typedef enum rtk_l34_dsliteUnmatch_act_s
{
	RTK_L34_DSLITE_UNMATCH_ACT_DROP = 0,
	RTK_L34_DSLITE_UNMATCH_ACT_TRAP,
	RTK_L34_DSLITE_UNMATCH_ACT_END
}rtk_l34_dsliteUnmatch_act_t;

typedef enum rtk_l34_dsliteCtrlType_e{
	L34_DSLITE_CTRL_START = 0,
	L34_DSLITE_CTRL_DSLITE_STATE = L34_DSLITE_CTRL_START,
	L34_DSLITE_CTRL_MC_PREFIX_UNMATCH,
	L34_DSLITE_CTRL_DS_UNMATCH_ACT,
	L34_DSLITE_CTRL_IP6_NH_ACTION,
	L34_DSLITE_CTRL_IP6_FRAGMENT_ACTION,
	L34_DSLITE_CTRL_IP4_FRAGMENT_ACTION,
	L34_DSLITE_CTRL_END
}rtk_l34_dsliteCtrlType_t;

typedef enum rtk_l34_direct_e{
	L34_DIR_START=0,
	L34_DIR_UPSTREAM=L34_DIR_START,
	L34_DIR_DOWNSTREAM,
	L34_DIR_END
}rtk_l34_direct_t;

typedef enum rtk_l34_lutMissAct_e{
	L34_LUTMISS_ACT_START = 0,
	L34_LUTMISS_ACT_DROP = L34_LUTMISS_ACT_START,
	L34_LUTMISS_ACT_TRAP,
	L34_LUTMISS_ACT_END
}rtk_l34_lutMissAct_t;

typedef struct rtk_l34_netif_entry_s{
	uint16     mtu;
	uint16     v6mtu;
	uint8      enable_rounting;	
	uint8      enable_v6Route;
	rtk_mac_t  gateway_mac;
	uint8      mac_mask;
	uint16     vlan_id;
	uint8      valid ;
}rtk_l34_netif_entry_t;

typedef struct rtk_l34_routing_entry_s {
	    ipaddr_t ipAddr;
	    uint32   ipMask; /*0: 0x80000000  ... 30:0xFFFFFFFE 31:0xFFFFFFFF maks all*/
	    rtk_l34_routing_type_t process; /*0: L34_PROCESS_CPU, 1:L34_PROCESS_DROP, 2:L34_PROCESS_ARP(local route), 3:L34_PROCESS_NH(global route)*/
        uint8  valid;
        uint8  internal;
        /* process = ARP */
        uint32 netifIdx;
        uint32 arpStart;
	    uint32 arpEnd;
        /* process = nexthop */
	    uint8 nhStart; /*exact index*/    //Start position of next hop table. Exact position=Value*2
	    uint8 nhNum;   /*exact number*/   //Number of Entries used, start from 2*NHSTA  (0->2) (1->4) (2->8) (3->16) (4->32) 
	    uint8 nhNxt;
	    uint8 nhAlgo;
	    uint8 ipDomain;
	    uint32 rt2waninf;
} rtk_l34_routing_entry_t;

typedef struct rtk_l34_arp_entry_s {
	uint32  index;
	uint32  nhIdx;
	uint32  valid;
} rtk_l34_arp_entry_t;

typedef struct rtk_ipv6Routing_entry_s {
    uint8	                     valid;
    rtk_l34_ipv6RouteType_t      type;  //rtk_l34_ipv6RouteType_t
    uint32	                     nhOrIfidIdx;  /*local: idex to interface table   global: index to ipv6 neighbor table*/
    uint32	                     ipv6PrefixLen;
    rtk_ipv6_addr_t              ipv6Addr;
    uint32                       rt2waninf;
} rtk_ipv6Routing_entry_t;

typedef struct rtk_l34_ext_intip_entry_s {
	    ipaddr_t 	extIpAddr;
	    ipaddr_t 	intIpAddr;
        uint8       valid;
        rtk_l34_extip_table_type_t   type;
        uint32 		nhIdx; /*index of next hop table*/
        uint8       prival;
        uint8       pri;
} rtk_l34_ext_intip_entry_t;

typedef struct rtk_l34_nexthop_entry_s {
	rtk_l34_nexthop_type_t  type;
	uint8  ifIdx;
	uint8  pppoeIdx;
    uint16 nhIdx;
    uint8  keepPppoe;
} rtk_l34_nexthop_entry_t;

typedef struct rtk_wanType_entry_s {
    uint32       	    nhIdx;
    rtk_l34_wanType_t	wanType;
} rtk_wanType_entry_t;

typedef struct rtk_l34_naptOutbound_entry_s {
    uint16	    hashIdx;	//apollo field but we need this  ,naptInIdx(absolute )
    uint8	    valid;
    uint8	    priValid;
    uint8	    priValue;
	//865x
	uint32 		intIp;
    uint16  	intPort;
	uint16		extPort;
	uint8		isTcp;
} rtk_l34_naptOutbound_entry_t;

typedef struct rtk_l34_naptInbound_entry_s {
	uint32  intIp;
    uint16  intPort;
    uint16  remHash;	//apollo(16bits), in 865x we call selEIdx(10bits)
    uint8   extIpIdx;	//apollo
	uint8	extPortLSB; //apollo ,xdsl using extPort 
    uint16	extPort;	 //865x
	uint8	isTcp;
    uint8 	valid;
    uint8  	priValid;
    uint8  	priId;
} rtk_l34_naptInbound_entry_t;

typedef struct rtk_l34_pppoe_entry_s{
    uint32          sessionID;
} rtk_l34_pppoe_entry_t;

/*IPv6 Neighbor table*/
typedef struct rtk_ipv6Neighbor_entry_s {
    uint8	    valid;
    uint32      l2Idx;
    uint8	    ipv6RouteIdx;
    uint64	    ipv6Ifid;
} rtk_ipv6Neighbor_entry_t;

/*binding table*/
typedef struct rtk_binding_entry_s {
    uint32	        wanTypeIdx;
    uint32	        vidLan;   /*VID=0: Port based binding  Others: Port-and-VLAN based binding*/
    rtk_portmask_t  portMask;
    rtk_portmask_t  extPortMask;
    rtk_l34_bindProto_t  bindProto;
} rtk_binding_entry_t;

typedef struct rtk_l34_ipmcTrans_entry_s
{
    uint32  netifIdx;
    uint32  sipTransEnable;
    uint32  extipIdx;
    rtk_l34_pppoeAct_t  pppoeAct;
    uint32  pppoeIdx;
}rtk_l34_ipmcTrans_entry_t;

typedef struct rtk_l34_hsa_s
{
    uint32  dslite_Idx;
    uint32  dslite_valid;
    uint32  is_policy;
    uint32  pppoeKeep;
    uint32  bindVidTrans;
    uint32  interVlanIf;
    uint32  reason;
    uint32  l34trans;
    uint32  l2trans;
    uint32  action;
    uint32  l4_pri_valid;
    uint32  l4_pri_sel;
    uint32  frag;
    uint32  difid;
    rtk_l34_hsa_pppoe_act_t  pppoe_if;
    uint32  pppid_idx;
    uint32  nexthop_mac_idx;
    uint32  l4_chksum;
    uint32  l3_chksum;
    uint32  dvid;
    uint32  port;
    rtk_ip_addr_t ip;
}rtk_l34_hsa_t;

typedef struct rtk_l34_hsb_s
{
	uint32  da_traslate;
	uint32  cfi_1;
	uint32  sa_idx;
	uint32  sa_idx_valid;
	uint32  ip6_type_ext;
	uint32  policy_nh_idx;
	uint32  is_policy_route;
    uint32  isFromWan;
    uint32  l2bridge;
    uint32  ipfrag_s;
    uint32  ipmf;
    uint32  l4_chksum_ok;
    uint32  l3_chksum_ok;
    uint32  cpu_direct_tx;
    uint32  udp_no_chksum;
    uint32  parse_fail;
    uint32  pppoe_if;
    uint32  svlan_if;
    uint32  ttls;   /*0b00 TTL=0, 0b01 TTL=1, 0b10 TTL>1 */
    rtk_l34_hsb_type_t  type;
    uint32   tcp_flag;
    uint32  cvlan_if;
    uint32   spa;
    uint32  cvid;
    uint32  len;
    uint32  dport_l4chksum;
    uint32  pppoe_id;
    rtk_ipv6_addr_t dip;  /*For IPv4 and IPv6*/
    rtk_ipv6_addr_t sip;  /*For IPv4 and IPv6*/
    uint32  sport_icmpid_chksum;
    rtk_mac_t  dmac;
}rtk_l34_hsb_t;

typedef struct rtk_l34_pppInfo_s{
	rtk_enable_t 		pppState;
	rtk_enable_t		passThroughState;
	uint32			sessionId;
	rtk_mac_t		serverMac;
}rtk_l34_pppInfo_t;

/*for external ip & local route table */
typedef struct rtk_l34_ipInfo_s{
	rtk_ip_addr_t 		ipaddr;		/*need get from local route*/
	uint32 			ipmask;
	rtk_ip_addr_t 		internalIp;
	rtk_l34_natType_t	natType;
	rtk_pri_t			defaultPri;
	rtk_l34_netifType_t netifType;  /*wan: external interface, lan: internal interface*/

}rtk_l34_ipInfo_t;

typedef struct rtk_l34_l2NetIfInfo_s{
	uint32 			mtu;
	rtk_mac_t 		ifmac;
	rtk_vlan_t		vid;
	rtk_enable_t 		ifState;
	rtk_enable_t 		routingState;
}rtk_l34_l2NetIfInfo_t;

typedef struct rtk_l34_route6Table_s{

	uint32 				  netifId;
	rtk_ipv6_addr_t		  ipaddr;
	uint32 				  prefixLen;
	rtk_mac_t			  gatewayMac;

}rtk_l34_route6Table_t;

#define L34_IPV6_ROUTE_MAX_SIZE  4
typedef struct rtk_l34_netifInfo_s{
	uint32				id;
	rtk_l34_l2NetIfInfo_t 	l2NetIfInfo;
	rtk_l34_pppInfo_t		pppInfo;
	rtk_l34_ipInfo_t		ipInfo;
	/*maxize size of ipv6 per netif is route size -1 since the last one is default route*/
	uint32 				ip6size;
	rtk_l34_route6Table_t	ip6Info[L34_IPV6_ROUTE_MAX_SIZE-1];
}rtk_l34_netifInfo_t;


typedef struct rtk_l34_nat_info_s{
	rtk_ip_addr_t   natIp;
    uint16          natPort;
    uint32          priValid;
    uint32          priority;
}rtk_l34_nat_info_t;

typedef struct rtk_l34_portWanMap_entry_s {
	uint32 	 wanIdx;
	rtk_port_t port;
	rtk_l34_portWanMapAct_t act;
}rtk_l34_portWanMap_entry_t;

typedef struct rtk_l34_arpInfo_s{

	uint32 		arpId;
	uint32 		l2Id;
	uint32 		netifId;
	rtk_mac_t 	mac;

}rtk_l34_arpInfo_t;

typedef struct rtk_l34_routeTable_s{
	uint32				netifId;
	rtk_ip_addr_t 			ipaddr;
	uint32                  	ipmask;
	rtk_mac_t			gatewayMac;
}rtk_l34_routeTable_t;

#define PPP_TRF_MAX_OF_WORD 1
typedef struct rtk_l34_ppp_trf_all_s
{
	uint32 trf_state[PPP_TRF_MAX_OF_WORD];
}rtk_l34_ppp_trf_all_t;

typedef struct rtk_l34_routeInfo_s{
	uint32					id;
	rtk_l34_routeTable_t		routeTable;
	rtk_l34_routeType_t		routeType;
	rtk_enable_t				vaild;
	uint32					arpStart;   /*arp table assign range:start*/
	uint32					arpEnd;     /*arp table assign rnage:end*/
	uint32 					nhId;	   /*next hop information*/
	uint32 					l2tableId;
}rtk_l34_routeInfo_t;

typedef struct rtk_l34_flowRoute_entry_s
{
	uint32 index;       /*IPv6 index must be even number, 0,2,4...etc. And occupy 2 entries for 1 flow*/
	uint8  valid;
	uint8  isIpv4;      /*0: IPv6, 1: IPv4*/
	union{
		rtk_ip_addr_t ipv4;
		rtk_ipv6_addr_t ipv6;
	}sip;
	union{
		rtk_ip_addr_t ipv4;
		rtk_ipv6_addr_t ipv6;
	}dip;
	uint16	sport;
	uint16 	dport;
	uint8 	isTcp;
	uint32	nexthopIdx;
	rtk_l34_flowRoute_type_t	process;
	uint8	pri_valid;
	uint8	priority;
}rtk_l34_flowRoute_entry_t;

typedef struct rtk_l34_dsliteInf_entry_s
{
	uint32 index;
	uint32 hopLimit;
	uint32 flowLabel;
	rtk_l34_dsliteTcOpt_t tcOpt;
	uint32 tc;
	rtk_enable_t valid;
	rtk_ipv6_addr_t ipAftr;
	rtk_ipv6_addr_t ipB4;
}rtk_l34_dsliteInf_entry_t;

typedef struct rtk_l34_dsliteMc_entry_s
{
	uint32 index;
	rtk_ipv6_addr_t ipUPrefix64Mask;
	rtk_ipv6_addr_t ipUPrefix64;
	rtk_ipv6_addr_t ipMPrefix64Mask;
	rtk_ipv6_addr_t ipMPrefix64;
}rtk_l34_dsliteMc_entry_t;

typedef struct rtk_l34_mib_s
{
	uint32 ifIndex;
	uint64 ifInOctets;
	uint32 ifInUcstPkts;
	uint64 ifOutOctets;
	uint32 ifOutUcstPkts;
}rtk_l34_mib_t;

typedef struct rtk_l34_tuple_s{
	uint16		srcport;
	uint16		dstport;
	uint32		proto;
	rtk_ip_addr_t 	srcIp;
	rtk_ip_addr_t	dstIp;
}rtk_l34_tuple_t;

typedef struct rtk_l34_connectInfo_s{
	rtk_l34_direct_t 	direction;
	rtk_l34_tuple_t 	tuple;
	rtk_ip_addr_t		natIp;
	rtk_l34_natType_t  natType;
	uint16			natport;
	uint32		  	netifId;
	uint32 			naptId;
	uint32			naptrId;
	rtk_enable_t		priValied;
	uint32 			priValue;
	rtk_enable_t 		trafficIndicator;
}rtk_l34_connectInfo_t;

#define L34_MAC_PORT_MAX 		6
#define L34_EXT_PORT_MAX 		3
#define L34_DSLVC_PORT_MAX 		6
typedef struct rtk_l34_global_cfg_s{
	uint32		macPortMap[L34_MAC_PORT_MAX];
	uint32		extPortMap[L34_EXT_PORT_MAX];
	uint32		dslVcPortMap[L34_DSLVC_PORT_MAX];
	rtk_enable_t 	globalState[L34_GLOBAL_STATE_END];
	rtk_l34_lookupMode_t		lookupMode;
	rtk_l34_wanRouteMode_t	wanRouteMode;
}rtk_l34_global_cfg_t;

typedef struct rtk_l34_rouet6Info_s{
	uint32 				  routeId;
	uint32 				  nhId;
	uint32 				  l2Id;
	rtk_enable_t			  valid;
	rtk_l34_route6Table_t	  route6Table;
	rtk_l34_ipv6RouteType_t route6Type;
}rtk_l34_route6Info_t;


typedef struct rtk_l34_neigh6Info_s{
	uint32 				neighborId;
	uint32				l2Id;
	uint32				netifId;
	uint32				routeId;
	rtk_mac_t			mac;
	rtk_ipv6_addr_t		ipaddr;
	uint64				neighIfId;
}rtk_l34_neigh6Info_t;

#endif

