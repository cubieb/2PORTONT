/*
 * Copyright (C) 2009-2010 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: 39101 $
 * $Date: 2013-05-03 17:35:27 +0800 (Fri, 03 May 2013) $
 *
 * Purpose : Define those public diag shell string
 *
 * Feature : The file have include the following module and sub-modules
 *           1) Diag shell string definition
 */


#ifndef __DIAG_STR_H__
#define __DIAG_STR_H__

/*
 * Include Files
 */

/*
 * Symbol Definition
 */
#define DIAG_STR_ENABLE                 "Enable"
#define DIAG_STR_DISABLE                "Disable"
#define DIAG_STR_SUCCESS                "Success"
#define DIAG_STR_FAILURE    			"Failure"
#define DIAG_STR_VALID      			"Valid"
#define DIAG_STR_INVALID    			"Invalid"
#define DIAG_STR_YES      			    "Yes"
#define DIAG_STR_NO    			        "No"
#define DIAG_STR_NOT    			    "Not"
#define DIAG_STR_NONE    			    "None"
#define DIAG_STR_FAULT    			    "Fault"
#define DIAG_STR_All    			    "All"
#define DIAG_STR_ACL    			    "Acl"

#define DIAG_STR_DOING    			    "Doing"
#define DIAG_STR_FIN    			    "Finish"


#define DIAG_STR_DROP                   "Drop"
#define DIAG_STR_TRAP2CPU               "Trap to CPU"
#define DIAG_STR_FORWARD                "Forward"
#define DIAG_STR_ASSIGN_SVLAN           "Assign to SVLAN"
#define DIAG_STR_ASSIGN_SVLAN_AND_KEEP  "Assign to SVLAN and Keep format"
#define DIAG_STR_COPY2CPU               "Copy to CPU"
#define DIAG_STR_2GUESTVLAN             "Forward to guest VLAN"
#define DIAG_STR_FLOOD_IN_VLAN          "Flood in VLAN"
#define DIAG_STR_FLOOD_2_ALLPORT        "Flood to all ports"
#define DIAG_STR_FLOOD_2_ROUTER_PORT    "Flood to Router ports"
#define DIAG_STR_FORWARD_EX_CPU         "Forward exclude CPU port"
#define DIAG_STR_DROP_EX_RMA            "Drop exclude RMA"
#define DIAG_STR_COPY                   "Copy"
#define DIAG_STR_REDIRECT               "Redirect"
#define DIAG_STR_MIRROR                 "Mirror"
#define DIAG_STR_ACLTRAP                "ACL Trap"
#define DIAG_STR_NOTSUPPORT             "Not support"
#define DIAG_STR_GUEST_VLAN             "GUEST VLAN"
#define DIAG_STR_PB_SVID                "Port-based SVID"


#define DIAG_STR_FIRSTDROP              "The 1st drop"
#define DIAG_STR_FIRSTTRAP2CPU          "The 1st trap to CPU"
#define DIAG_STR_FIRSTPASS              "The 1st pass"


#define DIAG_STR_INTERNAL_PRI           "Internal priority"
#define DIAG_STR_1Q_TAG_PRI             "1q tag priority"
#define DIAG_STR_SPRI                   "SPRI of SVLAN member configuration"
#define DIAG_STR_PB_PRI                 "Port based priority"

#define DIAG_STR_MAC31_0                "MAC[31:0]"
#define DIAG_STR_IPV4_DIP               "IPv4 Dip"

#define DIAG_STR_IGMPV1                 "IGMPv1"
#define DIAG_STR_IGMPV2                 "IGMPv2"
#define DIAG_STR_IGMPV3                 "IGMPv3"
#define DIAG_STR_MLDV1                  "MLDv1"
#define DIAG_STR_MLDV2                  "MLDv2"

#define DIAG_STR_VLAN                   "VLAN"
#define DIAG_STR_MAC                    "MAC"
#define DIAG_STR_PORT                   "PORT"


#define DIAG_STR_TCP                    "TCP"
#define DIAG_STR_UDP                    "UDP"


#define DIAG_STR_NAPT                   "NAPT"
#define DIAG_STR_NAPTR                  "NAPTR"
#define DIAG_STR_NAT                    "NAT"
#define DIAG_STR_NATR                   "NATR"

#define DIAG_STR_LOCAL_PUBLIC           "LP"
#define DIAG_STR_REMOTE_PUBLIC          "RP"

#define DIAG_STR_REVISE                 "Revise"
#define DIAG_STR_NOTREVISE              "Not Revise"

#define DIAG_STR_SPORT                  "Source Port"
#define DIAG_STR_DPORT                  "Destination Port"

#define DIAG_STR_IPV4_SIP               "Ipv4 Sip"
#define DIAG_STR_IPV6_SIP               "Ipv6 Sip"
#define DIAG_STR_IPV6_DIP               "Ipv6 Dip"

#define DIAG_STR_CVID                   "CVID"
#define DIAG_STR_SVID                   "SVID"

#define DIAG_STR_64ENTIRES              "64 Entries"
#define DIAG_STR_128ENTIRES             "128 Entries"


#define DIAG_STR_ROUTE_TYPE_GLOBAL      "Global Route"
#define DIAG_STR_ROUTE_TYPE_LOCAL       "Local Route"

#define DIAG_STR_INGRESS_VLAN           "Ingress Vlan"
#define DIAG_STR_EGRESS_VLAN            "Egress Vlan"

#define DIAG_STR_POLICING               "Policing"
#define DIAG_STR_ACLMIB                 "Acl MIB"
#define DIAG_STR_1PREMARK               "1P Remarking"
#define DIAG_STR_DSCPREMARK             "DSCP Remarking"
#define DIAG_STR_ACLPRI                 "ACL priority"
#define DIAG_STR_SID                    "Stream ID"
#define DIAG_STR_LLID                   "LLID"
#define DIAG_STR_DSLEXT                 "DSL/EXT"

#define DIAG_STR_DIPONLY                "DIP only"
#define DIAG_STR_DIPSIP                 "DIP + SIP"
#define DIAG_STR_MACFID                 "MAC + FID"
#define DIAG_STR_DIPVID                 "DIP + VID"

#define DIAG_STR_STATIC                 "Static"
#define DIAG_STR_AUTO                   "Auto"
#define DIAG_STR_IVL                    "IVL"
#define DIAG_STR_SVL                    "SVL"
#define DIAG_STR_DYNAMIC                "Dynamic"
#define DIAG_STR_BOTH                   "Both"

#define DIAG_STR_ETHERNET               "Ethernet"
#define DIAG_STR_PPPOE                  "PPPoE"
#define DIAG_STR_SNAP                   "SNAP"
#define DIAG_STR_LLC_OTHER              "LLC Other"


#define DIAG_STR_V                      "En"
#define DIAG_STR_X                      "Dis"

#define DIAG_STR_UPSTREAM               "Upstream"
#define DIAG_STR_DOWNSTREAM             "Downstream"

#define DIAG_STR_HIT                    "Hit"
#define DIAG_STR_NOT                    "Not"


#define DIAG_STR_TRUNK_DUMB_MODE        "Dumb mode"
#define DIAG_STR_TRUNK_NORMAL_MODE      "Normal mode"

#define DIAG_STR_TRUNK_HASH_SOURCE_PORT     "Source Port"
#define DIAG_STR_TRUNK_HASH_SOURCE_MAC      "Source MAC"
#define DIAG_STR_TRUNK_HASH_DEST_MAC        "Destination MAC"
#define DIAG_STR_TRUNK_HASH_SOURCE_IP       "Source IP"
#define DIAG_STR_TRUNK_HASH_DEST_IP         "Destination IP"
#define DIAG_STR_TRUNK_HASH_SOURCE_L4PORT   "Source L4 Port"
#define DIAG_STR_TRUNK_HASH_DEST_L4PORT     "Destination L4 Port"

#define DIAG_STR_TRUNK_FLOOD_NORMAL         "Normal Flood"
#define DIAG_STR_TRUNK_FLOOD_TO_FIRST       "Forward to first port"

#define DIAG_STR_NOP                    "No operation"
#define DIAG_STR_ADD                    "Add"
#define DIAG_STR_DEL                    "Delete"
#define DIAG_STR_ASSIGN                 "Assign"
#define DIAG_STR_VS_TPID                "VS_TPID"
#define DIAG_STR_TPID_8100              "TPID_8100"
#define DIAG_STR_UNTAG                  "Untagging"
#define DIAG_STR_TAG                    "Tagging"
#define DIAG_STR_C2S                    "C2S translate"
#define DIAG_STR_SP2C                   "SP2C translate"
#define DIAG_STR_TRANSPARENT            "Transparent"
#define DIAG_STR_COPY_C                 "Copy from outer tag"
#define DIAG_STR_COPY_S                 "Copy from outer tag"
#define DIAG_STR_COPY_1st               "Copy from 1st tag"
#define DIAG_STR_COPY_2nd               "Copy from 2nd tag"
#define DIAG_STR_INTER_PRI              "Assign from internal priority"


#define DIAG_STR_ASSIGN_QID             "ASSIGN to QID"
#define DIAG_STR_ASSIGN_SID             "ASSIGN to SID"
#define DIAG_STR_SWITCH_CORE            "Follow switch core"
#define DIAG_STR_LUT_LRN                "LUT learning"
#define DIAG_STR_FS_FORWARD             "Force forward"

#define DIAG_STR_STORM_UNKN_MC          "Unknown Multicast"
#define DIAG_STR_STORM_UNKN_UC          "Unknown Unicast"
#define DIAG_STR_STORM_MC               "Multicast"
#define DIAG_STR_STORM_BC               "Broadcast"

#define DIAG_STR_CVLAN_4K               "CVLAN 4K table"
#define DIAG_STR_SVLAN_64               "SVLAN 64 entries"

#define DIAG_STR_DEFAULT                "Default"
#define DIAG_STR_STORM_ALT_ARP          "ARP-Storm"
#define DIAG_STR_STORM_ALT_DHCP         "DHCP-Storm"
#define DIAG_STR_STORM_ALT_IGMPMLD      "IGMPMLD-Storm"


#define DIAG_STR_AUTHORIZED             "Authorized"
#define DIAG_STR_UNAUTHORIZED           "Un-Authorized"

#define DIAG_STR_DOT1XOPDIR_BOTH        "BOTH"
#define DIAG_STR_DOT1XOPDIR_IN          "IN"


#define  DIAG_STR_FLOWCTRL_INGRESS   "Ingress Flow Control"
#define  DIAG_STR_FLOWCTRL_EGRESS    "Egress Flow Control"
#define  DIAG_STR_FLOWCTRL_JUMBO_3K       "3Kbytes"
#define  DIAG_STR_FLOWCTRL_JUMBO_4K       "4Kbytes"
#define  DIAG_STR_FLOWCTRL_JUMBO_6K       "6Kbytes"
#define  DIAG_STR_FLOWCTRL_JUMBO_MAX    "maximum size (8960 bytes)"
#define DIAG_STR_90PPM                  "90PPM"
#define DIAG_STR_65PPM                  "65PPM"

#define DIAG_STR_AFB_MONCOUNT_8K        "8K packets"
#define DIAG_STR_AFB_MONCOUNT_16K       "16K packets"
#define DIAG_STR_AFB_MONCOUNT_32K       "32K packets"
#define DIAG_STR_AFB_MONCOUNT_64K       "64K packets"
#define DIAG_STR_AFB_MONCOUNT_128K      "128K packets"
#define DIAG_STR_AFB_MONCOUNT_256K      "256K packets"
#define DIAG_STR_AFB_MONCOUNT_512K      "512K packets"
#define DIAG_STR_AFB_MONCOUNT_1M        "1M packets"

#define DIAG_STR_AFB_ERRCOUNT_1         "1 packet"
#define DIAG_STR_AFB_ERRCOUNT_2         "2 packets"
#define DIAG_STR_AFB_ERRCOUNT_4         "4 packets"
#define DIAG_STR_AFB_ERRCOUNT_8         "8 packets"
#define DIAG_STR_AFB_ERRCOUNT_16        "16 packets"
#define DIAG_STR_AFB_ERRCOUNT_32        "32 packets"
#define DIAG_STR_AFB_ERRCOUNT_64        "64 packets"
#define DIAG_STR_AFB_ERRCOUNT_128       "128 packets"

#define DIAG_STR_AFB_NOT_RESTORE_PL     "Not Restore Power Level"
#define DIAG_STR_AFB_RESTORE_PL         "Restore Power Level"

#define DIAG_STR_AFB_NONE_VALID_FLOW    "None Valid Flow"
#define DIAG_STR_AFB_VALID_FLOW         "Valid Flow"

#define DIAG_STR_QUEUE_STRICT           "strict"
#define DIAG_STR_QUEUE_WFQ              "WFQ"

#define DIAG_STR_SPEED_10M              "10M"
#define DIAG_STR_SPEED_100M             "100M"
#define DIAG_STR_SPEED_GIGA             "1000M"
#define DIAG_STR_HALF_DUPLEX            "Half"
#define DIAG_STR_FULL_DUPLEX            "Full"
#define DIAG_STR_LINK_UP                "Up"
#define DIAG_STR_LINK_DOWN              "Down"



#define DIAG_STR_RAW                    "Raw packet"
#define DIAG_STR_LLC                    "LLC packet"
#define DIAG_STR_ARP                    "Arp packet"
#define DIAG_STR_IP4HEADER              "IPv4 header"
#define DIAG_STR_IP6HEADER              "IPv6 header"
#define DIAG_STR_IPPAYLOAD              "IP payload"
#define DIAG_STR_L4PAYLOAD              "L4 payload"


#define  DIAG_STR_SW_VOIP_RST          "VoIP reset"
#define  DIAG_STR_SW_PCIE_PHY_RST   "PCIE PHY reset"
#define  DIAG_STR_SW_PCIE_CTRL_RST  "PCIE controller reset"
#define  DIAG_STR_SW_USB3_PHY_RST  "USB3 PHY reset"
#define  DIAG_STR_SW_USB3_CTRL_RST "USB3 controller reset"
#define  DIAG_STR_SW_USB2_PHY_RST  "USB2 PHY reset"
#define  DIAG_STR_SW_USB2_CTRL_RST "USB2 controller reset"
#define  DIAG_STR_SW_SATA_PHY_RST  "SATA PHY reset"
#define  DIAG_STR_SW_SATA_CTRL_RST "SATA controller reset"
#define  DIAG_STR_SW_GPHY_RST         "Ethernet GPHY reset"
#define  DIAG_STR_SW_GLOBAL_RST      "Global reset"
#define  DIAG_STR_SW_RSG_RST            "RSGMII reset"
#define  DIAG_STR_SW_CFG_RST            "configuration reset"
#define  DIAG_STR_SW_Q_RST               "Queue/Packet Buffer/FIFO reset"
#define  DIAG_STR_SW_NIC_RST            "NIC interface reset"
#define  DIAG_STR_CPU_MEM_RST         "CPU and Memory controller reset"
#define  DIAG_STR_WDOG_NMI_EN        "Enable WDOG_RST trigger NMI_RST"
#define  DIAG_STR_PONMAC_RST          "PON MAC reset"
#define  DIAG_STR_BACKPRESSURE_JAM     "jam mode"
#define  DIAG_STR_BACKPRESSURE_DEFER  "defer mode"

#define DIAG_STR_SEC_SYN_RST_DENY           "SYN RST Deny"
#define DIAG_STR_SEC_SYN_FIN_DENY           "SYN FIN Deny"
#define DIAG_STR_SEC_XMA_DENY               "XMA Deny"
#define DIAG_STR_SEC_NULLSCAN_DENY          "NULLSCAN Deny"
#define DIAG_STR_SEC_SYN_SPORT_L1024_DENY   "SYN Sport Less 1024 Deny"
#define DIAG_STR_SEC_TCPHDR_MIN             "TCP Header Short"
#define DIAG_STR_SEC_SMURF_DENY             "SMURF Deny"
#define DIAG_STR_SEC_ICMPV6_PING_MAX        "ICMPV6 Ping Max"
#define DIAG_STR_SEC_ICMPV4_PING_MAX        "ICMPV4 Ping Max"
#define DIAG_STR_SEC_ICMP_FRAG_DENY         "ICMP Fragment Deny"
#define DIAG_STR_SEC_IPV6_MIN_FRAG_SIZE     "IPV6 Min Fragment Size Deny"
#define DIAG_STR_SEC_POD_DENY               "POD Deny"
#define DIAG_STR_SEC_TCP_BLAT_DENY          "TCP Blat Deny"
#define DIAG_STR_SEC_UDP_BLAT_DENY          "UDP Blat_Deny"
#define DIAG_STR_SEC_BLAT_DENY              "BLAT Deny"
#define DIAG_STR_SEC_LAND_DENY              "LAND deny"
#define DIAG_STR_SEC_DAEQSA_DENY            "DAEQSA Deny"
#define DIAG_STR_SEC_TCP_FRAG_OFF_MIN       "TCP Fragment Error"
#define DIAG_STR_SEC_UDPBOMB_DENY           "UDP Bomb Deny"
#define DIAG_STR_SEC_SYNWITHDATA_DENY       "SYN With Data Deny"
#define DIAG_STR_SEC_SYN_FLOOD_DENY         "SYN Flood Deny"
#define DIAG_STR_SEC_FIN_FLOOD_DENY         "FIN Flood Deny"
#define DIAG_STR_SEC_ICMP_FLOOD_DENY        "ICMP Flood Deny"

#define DIAG_STR_CPU_APOLLO_TAG_MODE        "APOLLO mode"
#define DIAG_STR_CPU_NORMAL_TAG_MODE        "Normal mode"

#define DIAG_STR_POLARITY_HIGH              "High"
#define DIAG_STR_POLARITY_LOW               "Low"


#define     DIAG_STR_PAR_FORWARD            "Forward"
#define     DIAG_STR_PAR_LOOPBACK           "Loop Back"
#define     DIAG_STR_PAR_DISCARD            "Discard"
#define     DIAG_STR_MUX_FORWARD            "Forward"
#define     DIAG_STR_MUX_DISCARD            "Discard"
#define     DIAG_STR_MUX_CPUONLY            "CPU Only"
#define     DIAG_STR_PAR_FORWARD        "Forward"
#define     DIAG_STR_PAR_LOOPBACK       "Loop Back"
#define     DIAG_STR_PAR_DISCARD          "Discard"
#define     DIAG_STR_MUX_FORWARD        "Forward"
#define     DIAG_STR_MUX_DISCARD          "Discard"
#define     DIAG_STR_MUX_CPUONLY         "CPU Only"

#define     DIAG_STR_ACT_DROP               "Drop"
#define     DIAG_STR_ACT_PERMIT             "Permit"
#define     DIAG_STR_ACT_PERMIT_NO_PON      "Permit without PON"


#define     DIAG_STR_MIRROR_ALL_PKT         "All packets"
#define     DIAG_STR_MIRROR_MIR_PKT_ONLY    "Mirrored packets only"


/*
 * Data Declaration
 */

extern const char *diagStr_enable[];
extern const char *diagStr_valid[];
extern const char *diagStr_svlanAct[];
extern const char *diagStr_svlanSpriSrc[];
extern const char *diagStr_svlanFmtStr[];
extern const char *diagStr_actionStr[];
extern const char *diagStr_igmpTypeStr[];
extern const char *diagStr_aclRangeCheckLenTypeStr[];
extern const char *diagStr_aclRangeCheckPortTypeStr[];
extern const char *diagStr_aclRangeCheckIpTypeStr[];
extern const char *diagStr_aclRangeCheckVidTypeStr[];
extern const char *diagStr_aclModeStr[];

extern const char *diagStr_aclActCvlanStr[];
extern const char *diagStr_aclActSvlanStr[];
extern const char *diagStr_aclActPoliceStr[];
extern const char *diagStr_aclActFwdStr[];
extern const char *diagStr_aclActPriStr[];
extern const char *diagStr_aclActCfStr[];

extern const char *diagStr_l2IpMcHashOpStr[];
extern const char *diagStr_l2IpMcHashMethodStr[];
extern const char *diagStr_l2LutStaticOrAutoStr[];
extern const char *diagStr_enDisplay[];
extern const char *diagStr_l2HashMethodStr[];

extern const char *diagStr_l34NexthopTypeStr[];
extern const char *diagStr_aclOper[];
extern const char *diagStr_vlanTagType[];
extern const char *diagStr_trunkMode[];
extern const char *diagStr_trunkAlgorithm[];
extern const char *diagStr_trunkFloodMode[];

extern const char *diagStr_direction[];
extern const char *diagStr_usCStagAction[];
extern const char *diagStr_usCtagAction[];
extern const char *diagStr_usSvidAction[];
extern const char *diagStr_usSidAction[];
extern const char *diagStr_dsCStagAction[];
extern const char *diagStr_dsCtagAction[];
extern const char *diagStr_dsCvidAction[];
extern const char *diagStr_dsCspriAction[];
extern const char *diagStr_dsUniAction[];

extern const char *diagStr_frameType[];
extern const char *diagStr_svlanLookup[];
extern const char *diagStr_stormType[];
extern const char *diagStr_stormAltType[];
extern const char *diagStr_authstate[];
extern const char *diagStr_unAuthAct[];
extern const char *diagStr_1xOpDir[];
extern const char *diagStr_ipgCompensation[];

extern const char *diagStr_AfbMonCount[];
extern const char *diagStr_AfbErrCount[];
extern const char *diagStr_AfbRestorePL[];
extern const char *diagStr_AfbvalidFlow[];

extern const char *diagStr_queueType[];
extern const char *diagStr_portSpeed[];
extern const char *diagStr_portDuplex[];
extern const char *diagStr_portLinkStatus[];
extern const char *diagStr_portNwayFault[];

extern const char *diagStr_selectorMode[];

extern const char *diagStr_flowCtrlType[];
extern const char *diagStr_flowCtrlJumboSize[];
extern const char *diagStr_chipReset[];
extern const char *diagStr_backPressure[];

extern const char *diagStr_cpuTagFormat[];

extern const char *diagStr_polarity[];

extern const char *diagStr_oamParserAct[];
extern const char *diagStr_oamMuxAct[];
extern const char *diagStr_mirrorEgressMode[];

extern const char *diagStr_cfUnmatchAct[];
extern const char *diagStr_l2flushMode[];

#endif /* end of __DIAG_STR_H__ */
