/*
 * Copyright (C) 2011 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * Purpose : Definition those SVLAN command and APIs in the SDK diagnostic shell.
 *
 * Feature : The file have include the following module and sub-modules
 *
 */

/*
 * Include Files
 */
#include <common/rt_type.h>
#include <diag_str.h>
#include <dal/apollo/raw/apollo_raw_svlan.h>

/*
 * Symbol Definition
 */


/*
 * Data Declaration
 */

/*common string*/
const char *diagStr_enable[] = {
    DIAG_STR_DISABLE,
    DIAG_STR_ENABLE
};

/*common string*/
const char *diagStr_valid[] = {
    DIAG_STR_INVALID,
    DIAG_STR_VALID
};

/*svlan string*/
const char *diagStr_svlanAct[] = {
    DIAG_STR_DROP,
    DIAG_STR_TRAP2CPU,
    DIAG_STR_ASSIGN_SVLAN,
    DIAG_STR_ASSIGN_SVLAN_AND_KEEP
};


const char *diagStr_svlanSpriSrc[] = {
    DIAG_STR_INTERNAL_PRI,
    DIAG_STR_1Q_TAG_PRI,
    DIAG_STR_SPRI,
    DIAG_STR_PB_PRI
};

const char *diagStr_svlanFmtStr[] = {
    DIAG_STR_MAC31_0,
    DIAG_STR_IPV4_DIP
};

const char *diagStr_actionStr[] = {
    DIAG_STR_FORWARD,
    DIAG_STR_DROP,
    DIAG_STR_TRAP2CPU,
    DIAG_STR_COPY2CPU,
    DIAG_STR_2GUESTVLAN,
    DIAG_STR_FLOOD_IN_VLAN,
    DIAG_STR_FLOOD_2_ALLPORT,
    DIAG_STR_FLOOD_2_ROUTER_PORT,
    DIAG_STR_FORWARD_EX_CPU,
    DIAG_STR_DROP_EX_RMA,
    DIAG_STR_NOTSUPPORT,

};

const char *diagStr_igmpTypeStr[] = {
    DIAG_STR_IGMPV1,
    DIAG_STR_IGMPV2,
    DIAG_STR_IGMPV3,
    DIAG_STR_MLDV1,
    DIAG_STR_MLDV2,
};

const char *diagStr_aclRangeCheckLenTypeStr[] = {
    DIAG_STR_NOTREVISE,
    DIAG_STR_REVISE,
};

const char *diagStr_aclRangeCheckPortTypeStr[] = {
    DIAG_STR_INVALID,
    DIAG_STR_SPORT,
    DIAG_STR_DPORT,
};

const char *diagStr_aclRangeCheckIpTypeStr[] = {
    DIAG_STR_INVALID,
    DIAG_STR_IPV4_SIP,
    DIAG_STR_IPV4_DIP,
    DIAG_STR_IPV6_SIP,
    DIAG_STR_IPV6_DIP,
};

const char *diagStr_aclRangeCheckVidTypeStr[] = {
    DIAG_STR_INVALID,
    DIAG_STR_CVID,
    DIAG_STR_SVID,
};

const char *diagStr_aclModeStr[] = {
    DIAG_STR_64ENTIRES,
    DIAG_STR_128ENTIRES
};


const char *diagStr_aclActCvlanStr[] = {
    DIAG_STR_INGRESS_VLAN,
    DIAG_STR_EGRESS_VLAN,
    DIAG_STR_SVID,
    DIAG_STR_POLICING,
    DIAG_STR_ACLMIB,
    DIAG_STR_1PREMARK
};

const char *diagStr_aclActSvlanStr[] = {
    DIAG_STR_INGRESS_VLAN,
    DIAG_STR_EGRESS_VLAN,
    DIAG_STR_CVID,
    DIAG_STR_POLICING,
    DIAG_STR_ACLMIB,
    DIAG_STR_1PREMARK,
    DIAG_STR_DSCPREMARK
};

const char *diagStr_aclActPoliceStr[] = {
    DIAG_STR_POLICING,
    DIAG_STR_ACLMIB,
};

const char *diagStr_aclActFwdStr[] = {
    DIAG_STR_COPY,
    DIAG_STR_REDIRECT,
    DIAG_STR_MIRROR,
    DIAG_STR_ACLTRAP,
};

const char *diagStr_aclActPriStr[] = {
    DIAG_STR_ACLPRI,
    DIAG_STR_DSCPREMARK,
    DIAG_STR_1PREMARK,
    DIAG_STR_POLICING,
    DIAG_STR_ACLMIB,
};

const char *diagStr_aclActCfStr[] = {
    DIAG_STR_NONE,
    DIAG_STR_SID,
    DIAG_STR_LLID,
    DIAG_STR_DSLEXT,
};

const char *diagStr_l2IpMcHashOpStr[] = {
    DIAG_STR_DIPONLY,
    DIAG_STR_DIPSIP
};

const char *diagStr_l2IpMcHashMethodStr[] = {
    DIAG_STR_MACFID,
    DIAG_STR_DIPSIP,
    DIAG_STR_DIPVID,
    DIAG_STR_DIPONLY,
};

const char *diagStr_l2LutStaticOrAutoStr[] = {
    DIAG_STR_AUTO,
    DIAG_STR_STATIC,
};

const char *diagStr_l2HashMethodStr[] = {
    DIAG_STR_SVL,
    DIAG_STR_IVL,
};

const char *diagStr_enDisplay[] = {
    DIAG_STR_X,
    DIAG_STR_V
};

const char *diagStr_l34NexthopTypeStr[] = {
    DIAG_STR_ETHERNET,
    DIAG_STR_PPPOE
};


const char *diagStr_vlanTagType[] = {
    DIAG_STR_UNTAG,
    DIAG_STR_TAG
};

const char *diagStr_aclOper[] = {
    DIAG_STR_HIT,
    DIAG_STR_NOT
};


const char *diagStr_trunkMode[] = {
    DIAG_STR_TRUNK_NORMAL_MODE,
    DIAG_STR_TRUNK_DUMB_MODE
};

const char *diagStr_trunkAlgorithm[] = {
    DIAG_STR_TRUNK_HASH_SOURCE_PORT,
    DIAG_STR_TRUNK_HASH_SOURCE_MAC,
    DIAG_STR_TRUNK_HASH_DEST_MAC,
    DIAG_STR_TRUNK_HASH_SOURCE_IP,
    DIAG_STR_TRUNK_HASH_DEST_IP,
    DIAG_STR_TRUNK_HASH_SOURCE_L4PORT,
    DIAG_STR_TRUNK_HASH_DEST_L4PORT
};

const char *diagStr_trunkFloodMode[] = {
    DIAG_STR_TRUNK_FLOOD_NORMAL,
    DIAG_STR_TRUNK_FLOOD_TO_FIRST
};

const char *diagStr_direction[] = {
    DIAG_STR_UPSTREAM,
    DIAG_STR_DOWNSTREAM
};

const char *diagStr_usCStagAction[] = {
    DIAG_STR_NOP,
    DIAG_STR_VS_TPID,
    DIAG_STR_TPID_8100
};

const char *diagStr_usCtagAction[] = {
    DIAG_STR_NOP,
    DIAG_STR_UNTAG,
    DIAG_STR_C2S,
    DIAG_STR_TRANSPARENT
};

const char *diagStr_usSvidAction[] = {
    DIAG_STR_ASSIGN,
    DIAG_STR_COPY_C,
    DIAG_STR_INTER_PRI
};

const char *diagStr_usSidAction[] = {
    DIAG_STR_ASSIGN_SID,
    DIAG_STR_ASSIGN_QID
};

const char *diagStr_dsCStagAction[] = {
    DIAG_STR_NOP,
    DIAG_STR_VS_TPID,
    DIAG_STR_TPID_8100,
    DIAG_STR_DEL
};

const char *diagStr_dsCtagAction[] = {
    DIAG_STR_NOP,
    DIAG_STR_TAG,
    DIAG_STR_SP2C,
    DIAG_STR_TRANSPARENT
};

const char *diagStr_dsCvidAction[] = {
    DIAG_STR_SWITCH_CORE,
    DIAG_STR_COPY_S,
    DIAG_STR_ASSIGN,
    DIAG_STR_LUT_LRN
};

const char *diagStr_dsCspriAction[] = {
    DIAG_STR_SWITCH_CORE,
    DIAG_STR_ASSIGN
};

const char *diagStr_dsUniAction[] = {
    DIAG_STR_FORWARD,
    DIAG_STR_FS_FORWARD
};



const char *diagStr_frameType[] = {
    DIAG_STR_ETHERNET,
    DIAG_STR_LLC_OTHER,
    DIAG_STR_SNAP
};

const char *diagStr_svlanLookup[] = {
    DIAG_STR_SVLAN_64,
    DIAG_STR_CVLAN_4K
};

const char *diagStr_stormType[] = {
    DIAG_STR_STORM_UNKN_MC,
    DIAG_STR_STORM_UNKN_UC,
    DIAG_STR_STORM_MC,
    DIAG_STR_STORM_BC
};

const char *diagStr_stormAltType[] = {
    DIAG_STR_DEFAULT,
    DIAG_STR_STORM_ALT_ARP,
    DIAG_STR_STORM_ALT_DHCP,
    DIAG_STR_STORM_ALT_IGMPMLD
};

const char *diagStr_authstate[] = {
    DIAG_STR_AUTHORIZED,
    DIAG_STR_UNAUTHORIZED
};


const char *diagStr_unAuthAct[] = {
    DIAG_STR_DROP,
    DIAG_STR_TRAP2CPU,
    DIAG_STR_GUEST_VLAN
};


const char *diagStr_1xOpDir[] = {
    DIAG_STR_BOTH,
    DIAG_STR_DOT1XOPDIR_IN
};

const char *diagStr_ipgCompensation[] = {
    DIAG_STR_90PPM,
    DIAG_STR_65PPM
};


const char *diagStr_AfbMonCount[] = {
    DIAG_STR_AFB_MONCOUNT_8K,
    DIAG_STR_AFB_MONCOUNT_16K,
    DIAG_STR_AFB_MONCOUNT_32K,
    DIAG_STR_AFB_MONCOUNT_64K,
    DIAG_STR_AFB_MONCOUNT_128K,
    DIAG_STR_AFB_MONCOUNT_256K,
    DIAG_STR_AFB_MONCOUNT_512K,
    DIAG_STR_AFB_MONCOUNT_1M
};

const char *diagStr_AfbErrCount[] = {
    DIAG_STR_AFB_ERRCOUNT_1,
    DIAG_STR_AFB_ERRCOUNT_2,
    DIAG_STR_AFB_ERRCOUNT_4,
    DIAG_STR_AFB_ERRCOUNT_8,
    DIAG_STR_AFB_ERRCOUNT_16,
    DIAG_STR_AFB_ERRCOUNT_32,
    DIAG_STR_AFB_ERRCOUNT_64,
    DIAG_STR_AFB_ERRCOUNT_128
};

const char *diagStr_AfbRestorePL[] = {
    DIAG_STR_AFB_NOT_RESTORE_PL,
    DIAG_STR_AFB_RESTORE_PL
};

const char *diagStr_AfbvalidFlow[] = {
    DIAG_STR_AFB_NONE_VALID_FLOW,
    DIAG_STR_AFB_VALID_FLOW
};

const char *diagStr_queueType[] = {
    DIAG_STR_QUEUE_STRICT,
    DIAG_STR_QUEUE_WFQ
};

const char *diagStr_portSpeed[] = {
    DIAG_STR_SPEED_10M,
    DIAG_STR_SPEED_100M,
    DIAG_STR_SPEED_500M,
};

const char *diagStr_portDuplex[] = {
    DIAG_STR_HALF_DUPLEX,
    DIAG_STR_FULL_DUPLEX
};

const char *diagStr_portLinkStatus[] = {
    DIAG_STR_LINK_DOWN,
    DIAG_STR_LINK_UP
};

const char *diagStr_portNwayFault[] = {
    DIAG_STR_SUCCESS,
    DIAG_STR_FAULT
};

const char *diagStr_flowCtrlType[] = {
    DIAG_STR_FLOWCTRL_EGRESS,
    DIAG_STR_FLOWCTRL_INGRESS
};


const char *diagStr_selectorMode[] = {
    DIAG_STR_DEFAULT,
    DIAG_STR_RAW,
    DIAG_STR_LLC,
    DIAG_STR_ARP,
    DIAG_STR_IP4HEADER,
    DIAG_STR_IP6HEADER,
    DIAG_STR_IPPAYLOAD,
    DIAG_STR_L4PAYLOAD,
};




const char *diagStr_flowCtrlJumboSize[] = {
    DIAG_STR_FLOWCTRL_JUMBO_3K,
    DIAG_STR_FLOWCTRL_JUMBO_4K,
    DIAG_STR_FLOWCTRL_JUMBO_6K,
    DIAG_STR_FLOWCTRL_JUMBO_MAX
};

const char *diagStr_chipReset[] = {
    DIAG_STR_SW_VOIP_RST ,
    DIAG_STR_SW_PCIE_PHY_RST,
    DIAG_STR_SW_PCIE_CTRL_RST,
    DIAG_STR_SW_USB3_PHY_RST,
    DIAG_STR_SW_USB3_CTRL_RST,
    DIAG_STR_SW_USB2_PHY_RST,
    DIAG_STR_SW_USB2_CTRL_RST,
    DIAG_STR_SW_SATA_PHY_RST,
    DIAG_STR_SW_SATA_CTRL_RST,
    DIAG_STR_SW_GPHY_RST,
    DIAG_STR_SW_GLOBAL_RST,
    DIAG_STR_SW_RSG_RST,
    DIAG_STR_SW_CFG_RST,
    DIAG_STR_SW_Q_RST,
    DIAG_STR_SW_NIC_RST,
    DIAG_STR_CPU_MEM_RST,
    DIAG_STR_WDOG_NMI_EN,
    DIAG_STR_PONMAC_RST
};

const char *diagStr_backPressure[] = {
    DIAG_STR_BACKPRESSURE_JAM,
    DIAG_STR_BACKPRESSURE_DEFER
};

const char *diagStr_cpuTagFormat[] = {
    DIAG_STR_CPU_APOLLO_TAG_MODE,
    DIAG_STR_CPU_NORMAL_TAG_MODE
};

const char *diagStr_polarity[] = {
    DIAG_STR_POLARITY_HIGH,
    DIAG_STR_POLARITY_LOW
};

/*OAM string*/

const char *diagStr_oamParserAct[] = {
    DIAG_STR_PAR_FORWARD,
    DIAG_STR_PAR_LOOPBACK,
    DIAG_STR_PAR_DISCARD
};

const char *diagStr_oamMuxAct[] = {
    DIAG_STR_MUX_FORWARD,
    DIAG_STR_MUX_DISCARD,
    DIAG_STR_MUX_CPUONLY
};

const char *diagStr_cfUnmatchAct[] = {
    DIAG_STR_ACT_PERMIT,
    DIAG_STR_ACT_PERMIT_NO_PON,
    DIAG_STR_ACT_DROP
};



const char *diagStr_mirrorEgressMode[] = {
    DIAG_STR_MIRROR_ALL_PKT,
    DIAG_STR_MIRROR_MIR_PKT_ONLY
};


const char *diagStr_l2flushMode[] = {
    DIAG_STR_DYNAMIC,
    DIAG_STR_STATIC,
    DIAG_STR_BOTH,
    DIAG_STR_NONE
};



