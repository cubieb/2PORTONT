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
 * Purpose : Mapper Layer is used to seperate different kind of software or hardware platform
 *
 * Feature : Just dispatch information to Multiplex layer
 *
 */
#ifndef __DAL_MAPPER_H__
#define __DAL_MAPPER_H__

/*
 * Include Files
 */
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <rtk/switch.h>
#include <rtk/l34.h>
#include <rtk/ponmac.h>
#include <rtk/l2.h>
#include <rtk/stp.h>
#include <rtk/classify.h>
#include <rtk/stat.h>
#include <rtk/qos.h>
#include <rtk/svlan.h>
#include <rtk/acl.h>
#include <rtk/trap.h>
#include <rtk/rate.h>
#include <rtk/sec.h>
#include <rtk/led.h>
#include <rtk/dot1x.h>
#include <rtk/oam.h>
#include <rtk/trunk.h>
#include <rtk/intr.h>
#include <rtk/rldp.h>
#include <rtk/gpio.h>
#include <rtk/i2c.h>
#include <rtk/time.h>
#include <rtk/epon.h>

#if CONFIG_GPON_VERSION < 2
#include <rtk/gpon.h>
#else
#include <rtk/gponv2.h>
#endif
/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */

typedef struct dal_mapper_s {
    uint32  family_id;
    int32   (*_init)(void);
    /* switch init*/
    int32   (*switch_init)(void);
    int32   (*switch_phyPortId_get)(rtk_switch_port_name_t, int32 *);
    int32   (*switch_logicalPort_get)(int32, rtk_switch_port_name_t *);
    int32   (*switch_port2PortMask_set)(rtk_portmask_t *, rtk_switch_port_name_t);
    int32   (*switch_port2PortMask_clear)(rtk_portmask_t *, rtk_switch_port_name_t);
    int32   (*switch_portIdInMask_check)(rtk_portmask_t *, rtk_switch_port_name_t);
    int32   (*switch_maxPktLenLinkSpeed_get)(rtk_switch_maxPktLen_linkSpeed_t, uint32 *);
    int32   (*switch_maxPktLenLinkSpeed_set)(rtk_switch_maxPktLen_linkSpeed_t, uint32);
    int32   (*switch_mgmtMacAddr_get)(rtk_mac_t *);
    int32   (*switch_mgmtMacAddr_set)(rtk_mac_t *);
    int32   (*switch_chip_reset)(void);
    int32   (*switch_version_get)(uint32 *, uint32 *, uint32 *);
    int32   (*switch_patch_info_get)(uint32, uint32 *);
    int32   (*switch_csExtId_get)(uint32 *);
    int32   (*switch_maxPktLenByPort_get)(rtk_port_t, uint32 *);
    int32   (*switch_maxPktLenByPort_set)(rtk_port_t, uint32);

    /* VLAN */
    int32   (*vlan_init)(void);
    int32   (*vlan_create)(rtk_vlan_t);
    int32   (*vlan_destroy)(rtk_vlan_t);
    int32   (*vlan_destroyAll)(uint32);
    int32   (*vlan_fid_get)(rtk_vlan_t, rtk_fid_t *);
    int32   (*vlan_fid_set)(rtk_vlan_t, rtk_fid_t);
    int32   (*vlan_fidMode_get)(rtk_vlan_t, rtk_fidMode_t *);
    int32   (*vlan_fidMode_set)(rtk_vlan_t, rtk_fidMode_t);
    int32   (*vlan_port_get)(rtk_vlan_t, rtk_portmask_t *, rtk_portmask_t *);
    int32   (*vlan_port_set)(rtk_vlan_t, rtk_portmask_t *, rtk_portmask_t *);
    int32   (*vlan_extPort_get)(rtk_vlan_t, rtk_portmask_t *);
    int32   (*vlan_extPort_set)(rtk_vlan_t, rtk_portmask_t *);
    int32   (*vlan_stg_get)(rtk_vlan_t, rtk_stg_t *);
    int32   (*vlan_stg_set)(rtk_vlan_t, rtk_stg_t);
    int32   (*vlan_priority_get)(rtk_vlan_t, rtk_pri_t *);
    int32   (*vlan_priority_set)(rtk_vlan_t, rtk_pri_t);
    int32   (*vlan_priorityEnable_get)(rtk_vlan_t, rtk_enable_t *);
    int32   (*vlan_priorityEnable_set)(rtk_vlan_t, rtk_enable_t);
    int32   (*vlan_policingEnable_get)(rtk_vlan_t, rtk_enable_t *);
    int32   (*vlan_policingEnable_set)(rtk_vlan_t, rtk_enable_t);
    int32   (*vlan_policingMeterIdx_get)(rtk_vlan_t vid, uint32 *pIndex);
    int32   (*vlan_policingMeterIdx_set)(rtk_vlan_t vid, uint32 index);
    int32   (*vlan_portAcceptFrameType_get)(rtk_port_t, rtk_vlan_acceptFrameType_t *);
    int32   (*vlan_portAcceptFrameType_set)(rtk_port_t, rtk_vlan_acceptFrameType_t);
    int32   (*vlan_vlanFunctionEnable_get)(rtk_enable_t *);
    int32   (*vlan_vlanFunctionEnable_set)(rtk_enable_t);
    int32   (*vlan_portIgrFilterEnable_get)(rtk_port_t, rtk_enable_t *);
    int32   (*vlan_portIgrFilterEnable_set)(rtk_port_t, rtk_enable_t);
    int32   (*vlan_leaky_get)(rtk_leaky_type_t, rtk_enable_t *);
    int32   (*vlan_leaky_set)(rtk_leaky_type_t, rtk_enable_t);
    int32   (*vlan_portLeaky_get)(rtk_port_t, rtk_leaky_type_t, rtk_enable_t *);
    int32   (*vlan_portLeaky_set)(rtk_port_t, rtk_leaky_type_t, rtk_enable_t);
    int32   (*vlan_keepType_get)(rtk_vlan_keep_type_t, rtk_enable_t *);
    int32   (*vlan_keepType_set)(rtk_vlan_keep_type_t, rtk_enable_t);
    int32   (*vlan_portPvid_get)(rtk_port_t, uint32 *);
    int32   (*vlan_portPvid_set)(rtk_port_t, uint32);
    int32   (*vlan_extPortPvid_get)(uint32, uint32 *);
    int32   (*vlan_extPortPvid_set)(uint32, uint32);
    int32   (*vlan_protoGroup_get)(uint32, rtk_vlan_protoGroup_t *);
    int32   (*vlan_protoGroup_set)(uint32, rtk_vlan_protoGroup_t *);
    int32   (*vlan_portProtoVlan_get)(rtk_port_t, uint32, rtk_vlan_protoVlanCfg_t *);
    int32   (*vlan_portProtoVlan_set)(rtk_port_t, uint32, rtk_vlan_protoVlanCfg_t *);
    int32   (*vlan_tagMode_get)(rtk_port_t, rtk_vlan_tagMode_t *);
    int32   (*vlan_tagMode_set)(rtk_port_t, rtk_vlan_tagMode_t);
    int32   (*vlan_portFid_get)(rtk_port_t, rtk_enable_t *, rtk_fid_t *);
    int32   (*vlan_portFid_set)(rtk_port_t, rtk_enable_t, rtk_fid_t);
    int32   (*vlan_portPriority_get)(rtk_port_t, rtk_pri_t *);
    int32   (*vlan_portPriority_set)(rtk_port_t, rtk_pri_t);
    int32   (*vlan_portEgrTagKeepType_get)(rtk_port_t, rtk_portmask_t *, rtk_vlan_tagKeepType_t *);
    int32   (*vlan_portEgrTagKeepType_set)(rtk_port_t, rtk_portmask_t *, rtk_vlan_tagKeepType_t);
    int32   (*vlan_transparentEnable_get)(rtk_enable_t *);
    int32   (*vlan_transparentEnable_set)(rtk_enable_t);
    int32   (*vlan_cfiKeepEnable_get)(rtk_enable_t *);
    int32   (*vlan_cfiKeepEnable_set)(rtk_enable_t);
    int32   (*vlan_reservedVidAction_get)(rtk_vlan_resVidAction_t *, rtk_vlan_resVidAction_t *);
    int32   (*vlan_reservedVidAction_set)(rtk_vlan_resVidAction_t, rtk_vlan_resVidAction_t);
    int32   (*vlan_tagModeIp4mc_get)(rtk_port_t, rtk_vlan_tagModeIpmc_t *);
    int32   (*vlan_tagModeIp4mc_set)(rtk_port_t, rtk_vlan_tagModeIpmc_t);
    int32   (*vlan_tagModeIp6mc_get)(rtk_port_t, rtk_vlan_tagModeIpmc_t *);
    int32   (*vlan_tagModeIp6mc_set)(rtk_port_t, rtk_vlan_tagModeIpmc_t);
    int32   (*vlan_lutSvlanHashState_set)(rtk_vlan_t, rtk_enable_t);
    int32   (*vlan_lutSvlanHashState_get)(rtk_vlan_t, rtk_enable_t *);
    int32   (*vlan_extPortProtoVlan_set)(uint32, uint32, rtk_vlan_protoVlanCfg_t *);
    int32   (*vlan_extPortProtoVlan_get)(uint32, uint32, rtk_vlan_protoVlanCfg_t *);
    int32   (*vlan_extPortmaskIndex_set)(rtk_vlan_t, uint32);
    int32   (*vlan_extPortmaskIndex_get)(rtk_vlan_t, uint32 *);
    int32   (*vlan_extPortmaskCfg_set)(uint32, rtk_portmask_t *);
    int32   (*vlan_extPortmaskCfg_get)(uint32, rtk_portmask_t *);

    /* Trap*/
    int32   (*trap_init)(void);
    int32   (*trap_reasonTrapToCpuPriority_get)(rtk_trap_reason_type_t, rtk_pri_t *);
    int32   (*trap_reasonTrapToCpuPriority_set)(rtk_trap_reason_type_t, rtk_pri_t);
    int32   (*trap_igmpCtrlPkt2CpuEnable_get)(rtk_enable_t *);
    int32   (*trap_igmpCtrlPkt2CpuEnable_set)(rtk_enable_t );
    int32   (*trap_mldCtrlPkt2CpuEnable_get)(rtk_enable_t *);
    int32   (*trap_mldCtrlPkt2CpuEnable_set)(rtk_enable_t );
    int32   (*trap_portIgmpMldCtrlPktAction_get)(rtk_port_t, rtk_trap_igmpMld_type_t, rtk_action_t *);
    int32   (*trap_portIgmpMldCtrlPktAction_set)(rtk_port_t, rtk_trap_igmpMld_type_t, rtk_action_t);
    int32   (*trap_l2McastPkt2CpuEnable_get)(rtk_enable_t *);
    int32   (*trap_l2McastPkt2CpuEnable_set)(rtk_enable_t );
    int32   (*trap_ipMcastPkt2CpuEnable_get)(rtk_enable_t *);
    int32   (*trap_ipMcastPkt2CpuEnable_set)(rtk_enable_t );
    int32   (*trap_rmaAction_get)(rtk_mac_t *, rtk_trap_rma_action_t *);
    int32   (*trap_rmaAction_set)(rtk_mac_t *, rtk_trap_rma_action_t);
    int32   (*trap_rmaPri_get)(rtk_pri_t *);
    int32   (*trap_rmaPri_set)(rtk_pri_t);
    int32   (*trap_rmaVlanCheckEnable_get)(rtk_mac_t *, rtk_enable_t *);
    int32   (*trap_rmaVlanCheckEnable_set)(rtk_mac_t *, rtk_enable_t);
    int32   (*trap_rmaPortIsolationEnable_get)(rtk_mac_t *, rtk_enable_t *);
    int32   (*trap_rmaPortIsolationEnable_set)(rtk_mac_t *, rtk_enable_t);
    int32   (*trap_rmaStormControlEnable_get)(rtk_mac_t *, rtk_enable_t *);
    int32   (*trap_rmaStormControlEnable_set)(rtk_mac_t *, rtk_enable_t);
    int32   (*trap_rmaKeepCtagEnable_get)(rtk_mac_t *, rtk_enable_t *);
    int32   (*trap_rmaKeepCtagEnable_set)(rtk_mac_t *, rtk_enable_t);
    int32   (*trap_oamPduAction_get)(rtk_action_t *);
    int32   (*trap_oamPduAction_set)(rtk_action_t);
    int32   (*trap_oamPduPri_get)(rtk_pri_t *);
    int32   (*trap_oamPduPri_set)(rtk_pri_t);
    int32   (*trap_uniTrapPriorityEnable_set)(rtk_enable_t);
    int32   (*trap_uniTrapPriorityEnable_get)(rtk_enable_t *);
    int32   (*trap_uniTrapPriorityPriority_set)(rtk_pri_t);
    int32   (*trap_uniTrapPriorityPriority_get)(rtk_pri_t *);
    int32   (*trap_cpuTrapHashMask_set)(rtk_trap_hash_t, rtk_enable_t);
    int32   (*trap_cpuTrapHashMask_get)(rtk_trap_hash_t, rtk_enable_t *);
    int32   (*trap_cpuTrapHashPort_set)(uint32, rtk_port_t);
    int32   (*trap_cpuTrapHashPort_get)(uint32, rtk_port_t *);

    /* L2 */
    int32   (*l2_init)(void);
    int32   (*l2_flushLinkDownPortAddrEnable_get)(rtk_enable_t *);
    int32   (*l2_flushLinkDownPortAddrEnable_set)(rtk_enable_t);
    int32   (*l2_ucastAddr_flush)(rtk_l2_flushCfg_t *);
    int32   (*l2_table_clear)(void);
    int32   (*l2_limitLearningOverStatus_get)(uint32 *);
    int32   (*l2_limitLearningOverStatus_clear)(void);
    int32   (*l2_learningCnt_get)(uint32 *);
    int32   (*l2_limitLearningCnt_get)(uint32 *);
    int32   (*l2_limitLearningCnt_set)(uint32);
    int32   (*l2_limitLearningCntAction_get)(rtk_l2_limitLearnCntAction_t *);
    int32   (*l2_limitLearningCntAction_set)(rtk_l2_limitLearnCntAction_t);
    int32   (*l2_limitLearningEntryAction_get)(rtk_l2_limitLearnEntryAction_t *);
    int32   (*l2_limitLearningEntryAction_set)(rtk_l2_limitLearnEntryAction_t );
    int32   (*l2_portLimitLearningOverStatus_get)(rtk_port_t, uint32 *);
    int32   (*l2_portLimitLearningOverStatus_clear)(rtk_port_t);
    int32   (*l2_limitLearningPortMask_get)(rtk_portmask_t *);
    int32   (*l2_limitLearningPortMask_set)(rtk_portmask_t);
    int32   (*l2_portLearningCnt_get)(rtk_port_t, uint32 *);
    int32   (*l2_portLimitLearningCnt_get)(rtk_port_t, uint32 *);
    int32   (*l2_portLimitLearningCnt_set)(rtk_port_t, uint32);
    int32   (*l2_portLimitLearningCntAction_get)(rtk_port_t, rtk_l2_limitLearnCntAction_t *);
    int32   (*l2_portLimitLearningCntAction_set)(rtk_port_t, rtk_l2_limitLearnCntAction_t);
    int32   (*l2_aging_get)(uint32 *);
    int32   (*l2_aging_set)(uint32);
    int32   (*l2_portAgingEnable_get)(rtk_port_t, rtk_enable_t *);
    int32   (*l2_portAgingEnable_set)(rtk_port_t, rtk_enable_t);
    int32   (*l2_lookupMissAction_get)(rtk_l2_lookupMissType_t, rtk_action_t *);
    int32   (*l2_lookupMissAction_set)(rtk_l2_lookupMissType_t, rtk_action_t);
    int32   (*l2_portLookupMissAction_get)(rtk_port_t, rtk_l2_lookupMissType_t, rtk_action_t *);
    int32   (*l2_portLookupMissAction_set)(rtk_port_t, rtk_l2_lookupMissType_t, rtk_action_t);
    int32   (*l2_lookupMissFloodPortMask_get)(rtk_l2_lookupMissType_t, rtk_portmask_t *);
    int32   (*l2_lookupMissFloodPortMask_set)(rtk_l2_lookupMissType_t, rtk_portmask_t *);
    int32   (*l2_lookupMissFloodPortMask_add)(rtk_l2_lookupMissType_t, rtk_port_t);
    int32   (*l2_lookupMissFloodPortMask_del)(rtk_l2_lookupMissType_t, rtk_port_t);
    int32   (*l2_newMacOp_get)(rtk_port_t, rtk_l2_newMacLrnMode_t *, rtk_action_t *);
    int32   (*l2_newMacOp_set)(rtk_port_t, rtk_l2_newMacLrnMode_t, rtk_action_t);
    int32   (*l2_nextValidAddr_get)(int32 *, rtk_l2_ucastAddr_t *);
    int32   (*l2_nextValidAddrOnPort_get)(rtk_port_t, int32 *, rtk_l2_ucastAddr_t *);
    int32   (*l2_nextValidMcastAddr_get)(int32 *, rtk_l2_mcastAddr_t *);
    int32   (*l2_nextValidIpMcastAddr_get)(int32 *, rtk_l2_ipMcastAddr_t *);
    int32   (*l2_nextValidEntry_get)(int32 *, rtk_l2_addr_table_t *);
    int32   (*l2_addr_add)(rtk_l2_ucastAddr_t *);
    int32   (*l2_addr_del)(rtk_l2_ucastAddr_t *);
    int32   (*l2_addr_get)(rtk_l2_ucastAddr_t *);
    int32   (*l2_addr_delAll)(uint32);
    int32   (*l2_mcastAddr_add)(rtk_l2_mcastAddr_t *);
    int32   (*l2_mcastAddr_del)(rtk_l2_mcastAddr_t *);
    int32   (*l2_mcastAddr_get)(rtk_l2_mcastAddr_t *);
    int32   (*l2_illegalPortMoveAction_get)(rtk_port_t, rtk_action_t *);
    int32   (*l2_illegalPortMoveAction_set)(rtk_port_t, rtk_action_t);
    int32   (*l2_ipmcMode_get)(rtk_l2_ipmcMode_t *);
    int32   (*l2_ipmcMode_set)(rtk_l2_ipmcMode_t);
    int32   (*l2_ipv6mcMode_get)(rtk_l2_ipmcMode_t *);
    int32   (*l2_ipv6mcMode_set)(rtk_l2_ipmcMode_t);
    int32   (*l2_ipmcGroupLookupMissHash_get)(rtk_l2_ipmcHashOp_t *);
    int32   (*l2_ipmcGroupLookupMissHash_set)(rtk_l2_ipmcHashOp_t);
    int32   (*l2_ipmcGroup_add)(ipaddr_t, rtk_portmask_t *);
    int32   (*l2_ipmcGroupExtPortmask_add)(ipaddr_t, rtk_portmask_t *);
    int32   (*l2_ipmcGroup_del)(ipaddr_t);
    int32   (*l2_ipmcGroup_get)(ipaddr_t, rtk_portmask_t *);
    int32   (*l2_ipmcGroupExtPortmask_get)(ipaddr_t, rtk_portmask_t *);
    int32 	(*l2_ipmcSipFilter_set)(uint32 , ipaddr_t);
    int32 	(*l2_ipmcSipFilter_get)(uint32 , ipaddr_t *);
    int32   (*l2_portIpmcAction_get)(rtk_port_t, rtk_action_t *);
    int32   (*l2_portIpmcAction_set)(rtk_port_t, rtk_action_t);
    int32   (*l2_ipMcastAddr_add)(rtk_l2_ipMcastAddr_t *);
    int32   (*l2_ipMcastAddr_del)(rtk_l2_ipMcastAddr_t *);
    int32   (*l2_ipMcastAddr_get)(rtk_l2_ipMcastAddr_t *);
    int32   (*l2_srcPortEgrFilterMask_get)(rtk_portmask_t *);
    int32   (*l2_srcPortEgrFilterMask_set)(rtk_portmask_t *);
    int32   (*l2_extPortEgrFilterMask_get)(rtk_portmask_t *);
    int32   (*l2_extPortEgrFilterMask_set)(rtk_portmask_t *);
    int32 	(*l2_camState_set)(rtk_enable_t);
    int32 	(*l2_camState_get)(rtk_enable_t *);
    int32   (*l2_vidUnmatchAction_get)(rtk_port_t, rtk_action_t *);
    int32   (*l2_vidUnmatchAction_set)(rtk_port_t, rtk_action_t);
    int32   (*l2_extMemberConfig_get)(uint32, rtk_portmask_t *);
    int32   (*l2_extMemberConfig_set)(uint32, rtk_portmask_t);

    /* Port */
    int32   (*port_init)(void);
    int32   (*port_link_get)(rtk_port_t, rtk_port_linkStatus_t *);
    int32   (*port_speedDuplex_get)(rtk_port_t, rtk_port_speed_t *, rtk_port_duplex_t *);
    int32   (*port_flowctrl_get)(rtk_port_t, uint32 *, uint32 *);
    int32   (*port_phyAutoNegoEnable_get)(rtk_port_t, rtk_enable_t *);
    int32   (*port_phyAutoNegoEnable_set)(rtk_port_t, rtk_enable_t);
    int32   (*port_phyAutoNegoAbility_get)(rtk_port_t, rtk_port_phy_ability_t *);
    int32   (*port_phyAutoNegoAbility_set)(rtk_port_t, rtk_port_phy_ability_t *);
    int32   (*port_phyForceModeAbility_get)(rtk_port_t, rtk_port_speed_t *, rtk_port_duplex_t *, rtk_enable_t *);
    int32   (*port_phyForceModeAbility_set)(rtk_port_t, rtk_port_speed_t, rtk_port_duplex_t, rtk_enable_t);
    int32   (*port_phyReg_get)(rtk_port_t, uint32, rtk_port_phy_reg_t, uint32 *);
    int32   (*port_phyReg_set)(rtk_port_t, uint32, rtk_port_phy_reg_t, uint32);
    int32   (*port_phyMasterSlave_get)(rtk_port_t, rtk_port_masterSlave_t *);
    int32   (*port_phyMasterSlave_set)(rtk_port_t, rtk_port_masterSlave_t);
    int32   (*port_phyTestMode_get)(rtk_port_t, rtk_port_phy_test_mode_t *);
    int32   (*port_phyTestMode_set)(rtk_port_t, rtk_port_phy_test_mode_t);
    int32   (*port_cpuPortId_get)(rtk_port_t *);
    int32   (*port_isolation_get)(rtk_port_t, rtk_portmask_t *, rtk_portmask_t *);
    int32   (*port_isolation_set)(rtk_port_t, rtk_portmask_t *, rtk_portmask_t *);
    int32   (*port_isolationExt_get)(rtk_port_t, rtk_portmask_t *, rtk_portmask_t *);
    int32   (*port_isolationExt_set)(rtk_port_t, rtk_portmask_t *, rtk_portmask_t *);
    int32   (*port_isolationL34_get)(rtk_port_t, rtk_portmask_t *, rtk_portmask_t *);
    int32   (*port_isolationL34_set)(rtk_port_t, rtk_portmask_t *, rtk_portmask_t *);
    int32   (*port_isolationExtL34_get)(rtk_port_t, rtk_portmask_t *, rtk_portmask_t *);
    int32   (*port_isolationExtL34_set)(rtk_port_t, rtk_portmask_t *, rtk_portmask_t *);
    int32   (*port_isolationEntry_get)(rtk_port_isoConfig_t, rtk_port_t, rtk_portmask_t *, rtk_portmask_t *);
    int32   (*port_isolationEntry_set)(rtk_port_isoConfig_t, rtk_port_t, rtk_portmask_t *, rtk_portmask_t *);
    int32   (*port_isolationEntryExt_get)(rtk_port_isoConfig_t, rtk_port_t, rtk_portmask_t *, rtk_portmask_t *);
    int32   (*port_isolationEntryExt_set)(rtk_port_isoConfig_t, rtk_port_t, rtk_portmask_t *, rtk_portmask_t *);
    int32   (*port_isolationCtagPktConfig_get)(rtk_port_isoConfig_t *);
    int32   (*port_isolationCtagPktConfig_set)(rtk_port_isoConfig_t);
    int32   (*port_isolationL34PktConfig_get)(rtk_port_isoConfig_t *);
    int32   (*port_isolationL34PktConfig_set)(rtk_port_isoConfig_t);
    int32   (*port_isolationIpmcLeaky_get)(rtk_port_t, rtk_enable_t *);
    int32   (*port_isolationIpmcLeaky_set)(rtk_port_t, rtk_enable_t);
    int32   (*port_isolationPortLeaky_get)(rtk_port_t, rtk_leaky_type_t, rtk_enable_t *);
    int32   (*port_isolationPortLeaky_set)(rtk_port_t, rtk_leaky_type_t, rtk_enable_t);
    int32   (*port_isolationLeaky_get)(rtk_leaky_type_t, rtk_enable_t *);
    int32   (*port_isolationLeaky_set)(rtk_leaky_type_t, rtk_enable_t);
    int32   (*port_macRemoteLoopbackEnable_get)(rtk_port_t, rtk_enable_t *);
    int32   (*port_macRemoteLoopbackEnable_set)(rtk_port_t, rtk_enable_t);
    int32   (*port_macLocalLoopbackEnable_get)(rtk_port_t, rtk_enable_t *);
    int32   (*port_macLocalLoopbackEnable_set)(rtk_port_t, rtk_enable_t);
    int32   (*port_adminEnable_get)(rtk_port_t, rtk_enable_t *);
    int32   (*port_adminEnable_set)(rtk_port_t, rtk_enable_t);
    int32   (*port_specialCongest_get)(rtk_port_t, uint32 *);
    int32   (*port_specialCongest_set)(rtk_port_t, uint32);
    int32   (*port_specialCongestStatus_get)(rtk_port_t, uint32 *);
    int32   (*port_specialCongestStatus_clear)(rtk_port_t);
    int32   (*port_greenEnable_get)(rtk_port_t, rtk_enable_t *);
    int32   (*port_greenEnable_set)(rtk_port_t, rtk_enable_t);
    int32   (*port_phyCrossOverMode_get)(rtk_port_t, rtk_port_crossOver_mode_t *);
    int32   (*port_phyCrossOverMode_set)(rtk_port_t, rtk_port_crossOver_mode_t);
    int32   (*port_enhancedFid_get)(rtk_port_t, rtk_efid_t *);
    int32   (*port_enhancedFid_set)(rtk_port_t, rtk_efid_t);
    int32   (*port_rtctResult_get)(rtk_port_t, rtk_rtctResult_t *);
    int32   (*port_rtct_start)(rtk_portmask_t *);
    int32   (*port_macForceAbility_get)(rtk_port_t, rtk_port_macAbility_t *);
    int32   (*port_macForceAbility_set)(rtk_port_t, rtk_port_macAbility_t);
	int32   (*port_macForceAbilityState_get)(rtk_port_t, rtk_enable_t *);
    int32   (*port_macForceAbilityState_set)(rtk_port_t, rtk_enable_t);
    int32   (*port_macExtMode_set)(rtk_port_t, rtk_port_ext_mode_t);
    int32   (*port_macExtMode_get)(rtk_port_t, rtk_port_ext_mode_t *);
    int32   (*port_macExtRgmiiDelay_set)(rtk_port_t, uint32, uint32);
    int32   (*port_macExtRgmiiDelay_get)(rtk_port_t, uint32 *, uint32 *);
    int32   (*port_gigaLiteEnable_set)(rtk_port_t, rtk_enable_t);
    int32   (*port_gigaLiteEnable_get)(rtk_port_t, rtk_enable_t *);

    /* QoS */
    int32   (*qos_init)(void);
    int32   (*qos_priSelGroup_get)(uint32, rtk_qos_priSelWeight_t *);
    int32   (*qos_priSelGroup_set)(uint32, rtk_qos_priSelWeight_t *);
    int32   (*qos_portPri_get)(rtk_port_t, rtk_pri_t *);
    int32   (*qos_portPri_set)(rtk_port_t, rtk_pri_t);
    int32   (*qos_dscpPriRemapGroup_get)(uint32, uint32, rtk_pri_t *, uint32 *);
    int32   (*qos_dscpPriRemapGroup_set)(uint32, uint32, rtk_pri_t, uint32);
    int32   (*qos_1pPriRemapGroup_get)(uint32, rtk_pri_t, rtk_pri_t *, uint32 *);
    int32   (*qos_1pPriRemapGroup_set)(uint32, rtk_pri_t, rtk_pri_t, uint32);
    int32   (*qos_priMap_get)(uint32, rtk_qos_pri2queue_t *);
    int32   (*qos_priMap_set)(uint32, rtk_qos_pri2queue_t *);
    int32   (*qos_portPriMap_get)(rtk_port_t, uint32 *);
    int32   (*qos_portPriMap_set)(rtk_port_t, uint32);
    int32   (*qos_1pRemarkEnable_get)(rtk_port_t, rtk_enable_t *);
    int32   (*qos_1pRemarkEnable_set)(rtk_port_t, rtk_enable_t);
    int32   (*qos_1pRemarkGroup_get)(uint32, rtk_pri_t, uint32, rtk_pri_t *);
    int32   (*qos_1pRemarkGroup_set)(uint32, rtk_pri_t, uint32, rtk_pri_t);
    int32   (*qos_dscpRemarkEnable_get)(rtk_port_t, rtk_enable_t *);
    int32   (*qos_dscpRemarkEnable_set)(rtk_port_t, rtk_enable_t);
    int32   (*qos_dscpRemarkGroup_get)(uint32, rtk_pri_t, uint32, uint32 *);
    int32   (*qos_dscpRemarkGroup_set)(uint32, rtk_pri_t, uint32, uint32);
    int32   (*qos_fwd2CpuPriRemap_get)(rtk_pri_t, rtk_pri_t *);
    int32   (*qos_fwd2CpuPriRemap_set)(rtk_pri_t, rtk_pri_t);
    int32   (*qos_portDscpRemarkSrcSel_get)(rtk_port_t, rtk_qos_dscpRmkSrc_t *);
    int32   (*qos_portDscpRemarkSrcSel_set)(rtk_port_t, rtk_qos_dscpRmkSrc_t);
    int32   (*qos_dscp2DscpRemarkGroup_get)(uint32, uint32, uint32 *);
    int32   (*qos_dscp2DscpRemarkGroup_set)(uint32, uint32, uint32);
    int32   (*qos_schedulingQueue_get)(rtk_port_t, rtk_qos_queue_weights_t *);
    int32   (*qos_schedulingQueue_set)(rtk_port_t, rtk_qos_queue_weights_t *);
    int32   (*qos_portPriSelGroup_get)(rtk_port_t, uint32 *);
    int32   (*qos_portPriSelGroup_set)(rtk_port_t, uint32);
    int32   (*qos_schedulingType_get)(rtk_qos_scheduling_type_t *);
    int32   (*qos_schedulingType_set)(rtk_qos_scheduling_type_t);
    int32   (*qos_portDot1pRemarkSrcSel_get)(rtk_port_t, rtk_qos_do1pRmkSrc_t *);
    int32   (*qos_portDot1pRemarkSrcSel_set)(rtk_port_t, rtk_qos_do1pRmkSrc_t);


    /* STP */
    int32   (*stp_init)(void);
    int32   (*stp_mstpState_get)(uint32, rtk_port_t, rtk_stp_state_t *);
    int32   (*stp_mstpState_set)(uint32, rtk_port_t, rtk_stp_state_t);

    /* L34 */
    	int32   (*l34_init)(void);
	int32   (*l34_netifTable_set)(uint32, rtk_l34_netif_entry_t *);
	int32   (*l34_netifTable_get)(uint32, rtk_l34_netif_entry_t *);
	int32   (*l34_arpTable_set)(uint32, rtk_l34_arp_entry_t *);
	int32   (*l34_arpTable_get)(uint32, rtk_l34_arp_entry_t *);
	int32   (*l34_arpTable_del)(uint32);
	int32   (*l34_pppoeTable_set)(uint32, rtk_l34_pppoe_entry_t *);
	int32   (*l34_pppoeTable_get)(uint32, rtk_l34_pppoe_entry_t *);
	int32   (*l34_routingTable_set)(uint32, rtk_l34_routing_entry_t *);
	int32   (*l34_routingTable_get)(uint32, rtk_l34_routing_entry_t *);
	int32   (*l34_routingTable_del)(uint32);
	int32   (*l34_nexthopTable_set)(uint32, rtk_l34_nexthop_entry_t *);
	int32   (*l34_nexthopTable_get)(uint32, rtk_l34_nexthop_entry_t *);
	int32   (*l34_extIntIPTable_set)(uint32, rtk_l34_ext_intip_entry_t *);
	int32   (*l34_extIntIPTable_get)(uint32, rtk_l34_ext_intip_entry_t *);
	int32   (*l34_extIntIPTable_del)(uint32);
	int32   (*l34_naptInboundTable_set)(int8, uint32, rtk_l34_naptInbound_entry_t *);
	int32   (*l34_naptInboundTable_get)(uint32, rtk_l34_naptInbound_entry_t *);
	int32   (*l34_naptOutboundTable_set)(int8, uint32, rtk_l34_naptOutbound_entry_t *);
	int32   (*l34_naptOutboundTable_get)(uint32, rtk_l34_naptOutbound_entry_t *);
	int32   (*l34_ipmcTransTable_set)(uint32, rtk_l34_ipmcTrans_entry_t *);
	int32   (*l34_ipmcTransTable_get)(uint32, rtk_l34_ipmcTrans_entry_t *);
	int32   (*l34_table_reset)(rtk_l34_table_type_t);
	int32   (*l34_bindingTable_set)(uint32, rtk_binding_entry_t *);
	int32   (*l34_bindingTable_get)(uint32, rtk_binding_entry_t *);
	int32   (*l34_bindingAction_set)(rtk_l34_bindType_t, rtk_l34_bindAct_t);
	int32   (*l34_bindingAction_get)(rtk_l34_bindType_t, rtk_l34_bindAct_t *);
	int32   (*l34_wanTypeTable_set)(uint32, rtk_wanType_entry_t *);
	int32   (*l34_wanTypeTable_get)(uint32, rtk_wanType_entry_t *);
	int32   (*l34_ipv6RoutingTable_set)(uint32, rtk_ipv6Routing_entry_t *);
	int32   (*l34_ipv6RoutingTable_get)(uint32, rtk_ipv6Routing_entry_t *);
	int32   (*l34_ipv6NeighborTable_set)(uint32, rtk_ipv6Neighbor_entry_t *);
	int32   (*l34_ipv6NeighborTable_get)(uint32, rtk_ipv6Neighbor_entry_t *);
	int32   (*l34_hsabMode_set)(rtk_l34_hsba_mode_t);
	int32   (*l34_hsabMode_get)(rtk_l34_hsba_mode_t *);
	int32   (*l34_hsaData_get)(rtk_l34_hsa_t *);
	int32   (*l34_hsbData_get)(rtk_l34_hsb_t *);
	int32   (*l34_portWanMap_set)(rtk_l34_portWanMapType_t, rtk_l34_portWanMap_entry_t);
	int32   (*l34_portWanMap_get)(rtk_l34_portWanMapType_t, rtk_l34_portWanMap_entry_t *);
	int32   (*l34_globalState_set)(rtk_l34_globalStateType_t, rtk_enable_t);
	int32   (*l34_globalState_get)(rtk_l34_globalStateType_t, rtk_enable_t *);
	int32   (*l34_lookupMode_set)(rtk_l34_lookupMode_t);
	int32   (*l34_lookupMode_get)(rtk_l34_lookupMode_t *);
	int32   (*l34_lookupPortMap_set)(rtk_l34_portType_t, uint32, uint32);
	int32   (*l34_lookupPortMap_get)(rtk_l34_portType_t, uint32, uint32 *);
	int32   (*l34_wanRoutMode_set)(rtk_l34_wanRouteMode_t);
	int32   (*l34_wanRoutMode_get)(rtk_l34_wanRouteMode_t *);
	int32   (*l34_arpTrfIndicator_get)(uint32, rtk_enable_t *);
	int32   (*l34_naptTrfIndicator_get)(uint32, rtk_enable_t *);
	int32   (*l34_pppTrfIndicator_get)(uint32, rtk_enable_t *);
	int32   (*l34_neighTrfIndicator_get)(uint32, rtk_enable_t *);
	int32   (*l34_hsdState_set)(rtk_enable_t);
	int32   (*l34_hsdState_get)(rtk_enable_t *);
	int32   (*l34_hwL4TrfWrkTbl_set)(rtk_l34_l4_trf_t);
	int32   (*l34_hwL4TrfWrkTbl_get)(rtk_l34_l4_trf_t *);
	int32   (*l34_l4TrfTb_get)(rtk_l34_l4_trf_t, uint32, rtk_enable_t *);
	int32   (*l34_hwL4TrfWrkTbl_Clear)(rtk_l34_l4_trf_t);
	int32   (*l34_hwArpTrfWrkTbl_set)(rtk_l34_arp_trf_t);
	int32   (*l34_hwArpTrfWrkTbl_get)(rtk_l34_arp_trf_t *);
	int32   (*l34_arpTrfTb_get)(rtk_l34_arp_trf_t, uint32, rtk_enable_t *);
	int32   (*l34_hwArpTrfWrkTbl_Clear)(rtk_l34_arp_trf_t);
	int32   (*l34_naptTrfIndicator_get_all)(uint32 *);
	int32   (*l34_arpTrfIndicator_get_all)(uint32 *);
	int32   (*l34_pppTrfIndicator_get_all)(rtk_l34_ppp_trf_all_t *);
	int32   (*l34_ip6mcRoutingTransIdx_set)(uint32, rtk_port_t, uint32);
	int32   (*l34_ip6mcRoutingTransIdx_get)(uint32, rtk_port_t, uint32 *);
	int32   (*l34_flowRouteTable_set)(rtk_l34_flowRoute_entry_t *);
	int32   (*l34_flowRouteTable_get)(rtk_l34_flowRoute_entry_t *);
	int32   (*l34_flowTrfIndicator_get)(uint32, rtk_enable_t *);
	int32	(*l34_dsliteInfTable_set)(rtk_l34_dsliteInf_entry_t *);
	int32	(*l34_dsliteInfTable_get)(rtk_l34_dsliteInf_entry_t *);
	int32	(*l34_dsliteMcTable_set)(rtk_l34_dsliteMc_entry_t *);
	int32	(*l34_dsliteMcTable_get)(rtk_l34_dsliteMc_entry_t *);
	int32	(*l34_dsliteControl_set)(rtk_l34_dsliteCtrlType_t, uint32);
	int32	(*l34_dsliteControl_get)(rtk_l34_dsliteCtrlType_t, uint32 *);
	int32   (*l34_mib_get)(rtk_l34_mib_t *);
	int32   (*l34_mib_reset)(uint32);
	int32   (*l34_lutLookupMiss_set)(rtk_l34_lutMissAct_t);
	int32   (*l34_lutLookupMiss_get)(rtk_l34_lutMissAct_t *);

    /*PON MAC*/
    int32   (*ponmac_init)(void);
    int32   (*ponmac_queue_add)(rtk_ponmac_queue_t *, rtk_ponmac_queueCfg_t *);
    int32   (*ponmac_queue_get)(rtk_ponmac_queue_t *, rtk_ponmac_queueCfg_t *);
    int32   (*ponmac_queue_del)(rtk_ponmac_queue_t *);
    int32   (*ponmac_flow2Queue_set)(uint32, rtk_ponmac_queue_t *);
    int32   (*ponmac_flow2Queue_get)(uint32, rtk_ponmac_queue_t *);
    int32   (*ponmac_mode_get)(rtk_ponmac_mode_t *);
    int32   (*ponmac_mode_set)(rtk_ponmac_mode_t);
    int32   (*ponmac_queueDrainOut_set)(rtk_ponmac_queue_t *);
    int32   (*ponmac_opticalPolarity_get)(rtk_ponmac_polarity_t *);
    int32   (*ponmac_opticalPolarity_set)(rtk_ponmac_polarity_t);
    int32   (*ponmac_losState_get)(rtk_enable_t *);
    int32   (*ponmac_serdesCdr_reset)(void);
    int32   (*ponmac_linkState_get)(rtk_ponmac_mode_t, uint32 *, uint32 *);
    int32   (*ponmac_forceBerNotify_set)(rtk_enable_t);
	int32   (*ponmac_bwThreshold_set)(uint32, uint32);
	int32   (*ponmac_bwThreshold_get)(uint32 *, uint32 *);
    int32   (*ponmac_maxPktLen_set)(uint32);
    int32   (*ponmac_sidValid_get)(uint32 , rtk_enable_t *);
    int32   (*ponmac_sidValid_set)(uint32 , rtk_enable_t);
    int32   (*ponmac_schedulingType_get)(rtk_qos_scheduling_type_t *);
    int32   (*ponmac_schedulingType_set)(rtk_qos_scheduling_type_t);
    int32   (*ponmac_egrBandwidthCtrlRate_get)(uint32 *);
    int32   (*ponmac_egrBandwidthCtrlRate_set)(uint32);
    int32   (*ponmac_egrScheduleIdRate_get)(uint32, uint32 *);
    int32   (*ponmac_egrScheduleIdRate_set)(uint32, uint32);
    int32   (*ponmac_txPll_relock)(void);

    /* GPON */
#if CONFIG_GPON_VERSION < 2
    int32 (*gpon_driver_initialize)(void);
    int32 (*gpon_driver_deInitialize)(void);
    int32 (*gpon_device_initialize)(void);
    int32 (*gpon_device_deInitialize)(void);
    int32 (*gpon_eventHandler_stateChange_reg)(rtk_gpon_eventHandleFunc_stateChange_t func);
    int32 (*gpon_eventHandler_dsFecChange_reg)(rtk_gpon_eventHandleFunc_fecChange_t func);
    int32 (*gpon_eventHandler_usFecChange_reg)(rtk_gpon_eventHandleFunc_fecChange_t func);
    int32 (*gpon_eventHandler_usPloamUrgEmpty_reg)(rtk_gpon_eventHandleFunc_usPloamEmpty_t func);
    int32 (*gpon_eventHandler_usPloamNrmEmpty_reg)(rtk_gpon_eventHandleFunc_usPloamEmpty_t func);
    int32 (*gpon_eventHandler_ploam_reg)(rtk_gpon_eventHandleFunc_ploam_t func);
    int32 (*gpon_eventHandler_omci_reg)(rtk_gpon_eventHandleFunc_omci_t func);
    int32 (*gpon_callback_queryAesKey_reg)(rtk_gpon_callbackFunc_queryAesKey_t func);
    int32 (*gpon_eventHandler_alarm_reg)(rtk_gpon_alarm_type_t alarmType, rtk_gpon_eventHandleFunc_fault_t func);
    int32 (*gpon_serialNumber_set)(rtk_gpon_serialNumber_t *sn);
    int32 (*gpon_serialNumber_get)(rtk_gpon_serialNumber_t *sn);
    int32 (*gpon_password_set)(rtk_gpon_password_t *pwd);
    int32 (*gpon_password_get)(rtk_gpon_password_t *pwd);
    int32 (*gpon_parameter_set)(rtk_gpon_patameter_type_t type, void *pPara);
    int32 (*gpon_parameter_get)(rtk_gpon_patameter_type_t type, void *pPara);
    int32 (*gpon_activate)(rtk_gpon_initialState_t initState);
    int32 (*gpon_deActivate)(void);
    int32 (*gpon_ponStatus_get)(rtk_gpon_fsm_status_t* status);
    void (*gpon_isr_entry)(void);
    int32 (*gpon_tcont_create)(rtk_gpon_tcont_ind_t* ind, rtk_gpon_tcont_attr_t* attr);
    int32 (*gpon_tcont_destroy)(rtk_gpon_tcont_ind_t* ind);
    int32 (*gpon_tcont_get)(rtk_gpon_tcont_ind_t* ind, rtk_gpon_tcont_attr_t* attr);
    int32 (*gpon_dsFlow_set)(uint32 flowId, rtk_gpon_dsFlow_attr_t* attr);
    int32 (*gpon_dsFlow_get)(uint32 flowId, rtk_gpon_dsFlow_attr_t* attr);
    int32 (*gpon_usFlow_set)(uint32 flowId, rtk_gpon_usFlow_attr_t* attr);
    int32 (*gpon_usFlow_get)(uint32 flowId, rtk_gpon_usFlow_attr_t* attr);
    int32 (*gpon_ploam_send)(int32 urgent, rtk_gpon_ploam_t* ploam);
    int32 (*gpon_broadcastPass_set)(int32 mode);
    int32 (*gpon_broadcastPass_get)(int32* mode);
    int32 (*gpon_nonMcastPass_set)(int32 mode);
    int32 (*gpon_nonMcastPass_get)(int32* mode);
#if 0 /* wellknown addr is removed in GPON_MAC_SWIO_r1.1 */
    int32 (*gpon_wellKnownAddr_set)(int32 mode, uint32 addr);
    int32 (*gpon_wellKnownAddr_get)(int32* mode, uint32* addr);
#endif
    int32 (*gpon_multicastAddrCheck_set)(uint32 ipv4_pattern, uint32 ipv6_pattern);
    int32 (*gpon_multicastAddrCheck_get)(uint32 *ipv4_pattern, uint32 *ipv6_pattern);
    int32 (*gpon_macFilterMode_set)(rtk_gpon_macTable_exclude_mode_t mode);
    int32 (*gpon_macFilterMode_get)(rtk_gpon_macTable_exclude_mode_t* mode);
    int32 (*gpon_mcForceMode_set)(rtk_gpon_mc_force_mode_t ipv4, rtk_gpon_mc_force_mode_t ipv6);
    int32 (*gpon_mcForceMode_get)(rtk_gpon_mc_force_mode_t *ipv4, rtk_gpon_mc_force_mode_t *ipv6);
    int32 (*gpon_macEntry_add)(rtk_gpon_macTable_entry_t* entry);
    int32 (*gpon_macEntry_del)(rtk_gpon_macTable_entry_t* entry);
    int32 (*gpon_macEntry_get)(uint32 index, rtk_gpon_macTable_entry_t* entry);
    int32 (*gpon_rdi_set)(int32 enable);
    int32 (*gpon_rdi_get)(int32* enable);
    int32 (*gpon_powerLevel_set)(uint8 level);
    int32 (*gpon_powerLevel_get)(uint8* level);
    int32 (*gpon_alarmStatus_get)(rtk_gpon_alarm_type_t alarm, int32 *status);
    int32 (*gpon_globalCounter_get )(rtk_gpon_global_performance_type_t type, rtk_gpon_global_counter_t* para);
    int32 (*gpon_tcontCounter_get )(uint32 tcontId, rtk_gpon_tcont_performance_type_t type, rtk_gpon_tcont_counter_t *pPara);
    int32 (*gpon_flowCounter_get )(uint32 flowId, rtk_gpon_flow_performance_type_t type, rtk_gpon_flow_counter_t *pPara);
    int32 (*gpon_version_get)(rtk_gpon_device_ver_t* hver, rtk_gpon_driver_ver_t* sver);
#if 0 /* gemloop is removed in GPON_MAC_SWIO_v1.1 */
    int32 (*gpon_gemLoop_set)(int32 loop);
    int32 (*gpon_gemLoop_get)(int32 *pLoop);
#endif
    int32 (*gpon_txForceLaser_set)(rtk_gpon_laser_status_t status);
    int32 (*gpon_txForceLaser_get)(rtk_gpon_laser_status_t *pStatus);
    int32 (*gpon_txForceIdle_set)(int32 on);
    int32 (*gpon_txForceIdle_get)(int32 *pOn);
#if 0
    int32 (*gpon_txForcePRBS_set)(int32 on);
    int32 (*gpon_txForcePRBS_get)(int32* pOn);
#endif
    int32 (*gpon_dsFecSts_get)(int32* en);
    void (*gpon_version_show)(void);
    void (*gpon_devInfo_show)(void);
    void (*gpon_gtc_show)(void);
    void (*gpon_tcont_show)(uint32 tcont);
    void (*gpon_dsFlow_show)(uint32 flow);
    void (*gpon_usFlow_show)(uint32 flow);
    void (*gpon_macTable_show)(void);
    void (*gpon_globalCounter_show)(rtk_gpon_global_performance_type_t type);
    void (*gpon_tcontCounter_show)(uint32 idx, rtk_gpon_tcont_performance_type_t type);
    void (*gpon_flowCounter_show)(uint32 idx, rtk_gpon_flow_performance_type_t type);
#if defined(OLD_FPGA_DEFINED)
    int32 (*gpon_pktGen_cfg_set)(uint32 item, uint32 tcont, uint32 len, uint32 gem, int32 omci);
    int32 (*gpon_pktGen_buf_set)(uint32 item, uint8 *buf, uint32 len);
#endif
    int32 (*gpon_omci_tx)(rtk_gpon_omci_msg_t* omci);
    int32 (*gpon_omci_rx)(rtk_gpon_omci_msg_t* omci);
    int32 (*gpon_autoTcont_set)(int32 state);
    int32 (*gpon_autoTcont_get)(int32 *pState);
    int32 (*gpon_autoBoh_set)(int32 state);
    int32 (*gpon_autoBoh_get)(int32 *pState);
    int32 (*gpon_eqdOffset_set)(int32 offset);
    int32 (*gpon_eqdOffset_get)(int32 *pOffset);
    int32 (*gpon_autoDisTx_set)(rtk_enable_t state);
#else
	int32   (*gpon_init)(void);
	int32   (*gpon_resetState_set)(rtk_enable_t);
	int32   (*gpon_resetDoneState_get)(rtk_enable_t *);
	int32   (*gpon_version_get)(uint32 *);
	int32   (*gpon_test_get)(uint32 *);
	int32   (*gpon_test_set)(uint32);
	int32   (*gpon_topIntrMask_get)(rtk_gpon_intrType_t, rtk_enable_t *);
	int32   (*gpon_topIntrMask_set)(rtk_gpon_intrType_t, rtk_enable_t);
	int32   (*gpon_topIntr_get)(rtk_gpon_intrType_t, rtk_enable_t *);
	int32   (*gpon_topIntr_disableAll)(void);
	int32   (*gpon_gtcDsIntr_get)(rtk_gpon_gtcDsIntrType_t, rtk_enable_t *);
	int32   (*gpon_gtcDsIntrDlt_get)(rtk_gpon_gtcDsIntrType_t, rtk_enable_t *);
	int32   (*gpon_gtcDsIntrMask_get)(rtk_gpon_gtcDsIntrType_t, rtk_enable_t *);
	int32   (*gpon_gtcDsIntrMask_set)(rtk_gpon_gtcDsIntrType_t, rtk_enable_t);
	int32   (*gpon_onuId_set)(uint8);
	int32   (*gpon_onuId_get)(uint8 *);
	int32   (*gpon_onuState_set)(rtk_gpon_onuState_t);
	int32   (*gpon_onuState_get)(rtk_gpon_onuState_t *);
	int32   (*gpon_dsBwmapCrcCheckState_set)(rtk_enable_t);
	int32   (*gpon_dsBwmapCrcCheckState_get)(rtk_enable_t *);
	int32   (*gpon_dsBwmapFilterOnuIdState_set)(rtk_enable_t);
	int32   (*gpon_dsBwmapFilterOnuIdState_get)(rtk_enable_t *);
	int32   (*gpon_dsPlendStrictMode_set)(rtk_enable_t);
	int32   (*gpon_dsPlendStrictMode_get)(rtk_enable_t *);
	int32   (*gpon_dsScrambleState_set)(rtk_enable_t);
	int32   (*gpon_dsScrambleState_get)(rtk_enable_t *);
	int32   (*gpon_dsFecBypass_set)(rtk_enable_t);
	int32   (*gpon_dsFecBypass_get)(rtk_enable_t *);
	int32   (*gpon_dsFecThrd_set)(uint8);
	int32   (*gpon_dsFecThrd_get)(uint8 *);
	int32   (*gpon_extraSnTxTimes_set)(uint8);
	int32   (*gpon_extraSnTxTimes_get)(uint8 *);
	int32   (*gpon_dsPloamNomsg_set)(uint8);
	int32   (*gpon_dsPloamNomsg_get)(uint8 *);
	int32   (*gpon_dsPloamOnuIdFilterState_set)(rtk_enable_t);
	int32   (*gpon_dsPloamOnuIdFilterState_get)(rtk_enable_t *);
	int32   (*gpon_dsPloamBcAcceptState_set)(rtk_enable_t);
	int32   (*gpon_dsPloamBcAcceptState_get)(rtk_enable_t *);
	int32   (*gpon_dsPloamDropCrcState_set)(rtk_enable_t);
	int32   (*gpon_dsPloamDropCrcState_get)(rtk_enable_t *);
	int32   (*gpon_cdrLosStatus_get)(rtk_enable_t *);
	int32   (*gpon_optLosStatus_get)(rtk_enable_t *);
	int32   (*gpon_losCfg_set)(rtk_enable_t, int32, rtk_enable_t, int32, rtk_enable_t);
	int32   (*gpon_losCfg_get)(int32 *, int32 *, int32 *, int32 *, int32 *);
	int32   (*gpon_dsPloam_get)(uint8 *);
	int32   (*gpon_tcont_get)(uint32, uint32 *);
	int32   (*gpon_tcont_set)(uint32, uint32);
	int32   (*gpon_tcont_del)(uint32);
	int32   (*gpon_dsGemPort_get)(uint32, rtk_gem_cfg_t *);
	int32   (*gpon_dsGemPort_set)(uint32, rtk_gem_cfg_t);
	int32   (*gpon_dsGemPort_del)(uint32);
	int32   (*gpon_dsGemPortPktCnt_get)(uint32, uint32 *);
	int32   (*gpon_dsGemPortByteCnt_get)(uint32, uint32 *);
	int32   (*gpon_dsGtcMiscCnt_get)(rtk_gpon_dsGtc_pmMiscType_t, uint32 *);
	int32   (*gpon_dsOmciPti_set)(uint32, uint32);
	int32   (*gpon_dsOmciPti_get)(uint32 *, uint32 *);
	int32   (*gpon_dsEthPti_set)(uint32, uint32);
	int32   (*gpon_dsEthPti_get)(uint32 *, uint32 *);
	int32   (*gpon_aesKeySwitch_set)(uint32);
	int32   (*gpon_aesKeySwitch_get)(uint32 *);
	int32   (*gpon_aesKeyWord_set)(uint8 *);
    int32   (*gpon_aesKeyWordActive_set)(uint8 *);
	int32   (*gpon_irq_get)(rtk_enable_t *);
	int32   (*gpon_dsGemPortEthRxCnt_get)(uint32, uint32 *);
	int32   (*gpon_dsGemPortEthFwdCnt_get)(uint32, uint32 *);
	int32   (*gpon_dsGemPortMiscCnt_get)(rtk_gpon_dsGem_pmMiscType_t, uint32 *);
	int32   (*gpon_dsGemPortFcsCheckState_get)(rtk_enable_t *);
	int32   (*gpon_dsGemPortFcsCheckState_set)(rtk_enable_t);
	int32   (*gpon_dsGemPortBcPassState_set)(rtk_enable_t);
	int32   (*gpon_dsGemPortBcPassState_get)(rtk_enable_t *);
	int32   (*gpon_dsGemPortNonMcPassState_set)(rtk_enable_t);
	int32   (*gpon_dsGemPortNonMcPassState_get)(rtk_enable_t *);
	int32   (*gpon_dsGemPortMacFilterMode_set)(rtk_gpon_macTable_exclude_mode_t);
	int32   (*gpon_dsGemPortMacFilterMode_get)(rtk_gpon_macTable_exclude_mode_t *);
	int32   (*gpon_dsGemPortMacForceMode_set)(rtk_gpon_ipVer_t, rtk_gpon_mc_force_mode_t);
	int32   (*gpon_dsGemPortMacForceMode_get)(rtk_gpon_ipVer_t, rtk_gpon_mc_force_mode_t *);
	int32   (*gpon_dsGemPortMacEntry_set)(uint32, rtk_mac_t);
	int32   (*gpon_dsGemPortMacEntry_get)(uint32, rtk_mac_t *);
	int32   (*gpon_dsGemPortMacEntry_del)(uint32);
	int32   (*gpon_dsGemPortFrameTimeOut_set)(uint32);
	int32   (*gpon_dsGemPortFrameTimeOut_get)(uint32 *);
	int32   (*gpon_ipv4McAddrPtn_get)(uint32 *);
	int32   (*gpon_ipv4McAddrPtn_set)(uint32);
	int32   (*gpon_ipv6McAddrPtn_get)(uint32 *);
	int32   (*gpon_ipv6McAddrPtn_set)(uint32);
	int32   (*gpon_gtcUsIntr_get)(rtk_gpon_gtcUsIntrType_t, rtk_enable_t *);
	int32   (*gpon_gtcUsIntrDlt_get)(rtk_gpon_gtcUsIntrType_t, rtk_enable_t *);
	int32   (*gpon_gtcUsIntrMask_get)(rtk_gpon_gtcUsIntrType_t, rtk_enable_t *);
	int32   (*gpon_gtcUsIntrMask_set)(rtk_gpon_gtcUsIntrType_t, rtk_enable_t);
	int32   (*gpon_forceLaser_set)(rtk_gpon_laser_status_t);
	int32   (*gpon_forceLaser_get)(rtk_gpon_laser_status_t *);
    int32   (*gpon_forcePRBS_set)(rtk_gpon_prbs_t);
    int32   (*gpon_forcePRBS_get)(rtk_gpon_prbs_t *);
	int32   (*gpon_ploamState_set)(rtk_enable_t);
	int32   (*gpon_ploamState_get)(rtk_enable_t *);
	int32   (*gpon_indNrmPloamState_set)(rtk_enable_t);
	int32   (*gpon_indNrmPloamState_get)(rtk_enable_t *);
	int32   (*gpon_dbruState_set)(rtk_enable_t);
	int32   (*gpon_dbruState_get)(rtk_enable_t *);
	int32   (*gpon_usScrambleState_set)(rtk_enable_t);
	int32   (*gpon_usScrambleState_get)(rtk_enable_t *);
	int32   (*gpon_usBurstPolarity_set)(rtk_gpon_polarity_t);
	int32   (*gpon_usBurstPolarity_get)(rtk_gpon_polarity_t *);
	int32   (*gpon_eqd_set)(uint32, int32);
	int32   (*gpon_laserTime_set)(uint8, uint8);
	int32   (*gpon_laserTime_get)(uint8 *, uint8 *);
	int32   (*gpon_burstOverhead_set)(uint8, uint8, uint8, uint8 *);
	int32   (*gpon_usPloam_set)(rtk_enable_t, uint8 *);
	int32   (*gpon_usAutoPloam_set)(rtk_gpon_usAutoPloamType_t, uint8 *);
	int32   (*gpon_usPloamCrcGenState_set)(rtk_enable_t);
	int32   (*gpon_usPloamCrcGenState_get)(rtk_enable_t *);
	int32   (*gpon_usPloamOnuIdFilterState_set)(rtk_enable_t);
	int32   (*gpon_usPloamOnuIdFilter_get)(rtk_enable_t *);
	int32   (*gpon_usPloamBuf_flush)(void);
	int32   (*gpon_usGtcMiscCnt_get)(rtk_gpon_usGtc_pmMiscType_t, uint32 *);
	int32   (*gpon_rdi_set)(int32);
	int32   (*gpon_rdi_get)(int32 *);
	int32   (*gpon_usSmalSstartProcState_set)(rtk_enable_t);
	int32   (*gpon_usSmalSstartProcState_get)(rtk_enable_t *);
	int32   (*gpon_usSuppressLaserState_set)(rtk_enable_t);
	int32   (*gpon_usSuppressLaserState_get)(rtk_enable_t *);
	int32   (*gpon_gemUsIntr_get)(rtk_gpon_gemUsIntrType_t, rtk_enable_t *);
	int32   (*gpon_gemUsIntrMask_get)(rtk_gpon_gemUsIntrType_t, rtk_enable_t *);
	int32   (*gpon_gemUsIntrMask_set)(rtk_gpon_gemUsIntrType_t, rtk_enable_t);
	int32   (*gpon_gemUsForceIdleState_set)(rtk_enable_t);
	int32   (*gpon_gemUsForceIdleState_get)(rtk_enable_t *);
	int32   (*gpon_gemUsPtiVector_set)(uint8, uint8, uint8, uint8);
	int32   (*gpon_gemUsPtiVector_get)(uint8 *, uint8 *, uint8 *, uint8 *);
	int32   (*gpon_gemUsEthCnt_get)(uint32, uint32 *);
	int32   (*gpon_gemUsGemCnt_get)(uint32, uint32 *);
	int32   (*gpon_gemUsPortCfg_set)(uint32, uint32);
	int32   (*gpon_gemUsPortCfg_get)(uint32, uint32 *);
	int32   (*gpon_gemUsDataByteCnt_get)(uint32, uint64 *);
	int32   (*gpon_gemUsIdleByteCnt_get)(uint32, uint64 *);
	int32   (*gpon_dbruPeriod_get)(uint32 *);
	int32   (*gpon_dbruPeriod_set)(uint32 );
	int32   (*gpon_gtcDsIntrDlt_check)(rtk_gpon_gtcDsIntrType_t, uint32, rtk_enable_t *);
	int32   (*gpon_gtcUsIntrDlt_check)(rtk_gpon_gtcUsIntrType_t, uint32, rtk_enable_t *);
	int32   (*gpon_gemUsIntrDlt_check)(rtk_gpon_gemUsIntrType_t, uint32, rtk_enable_t *);
	int32   (*gpon_rogueOnt_set)(rtk_enable_t);
	int32   (*gpon_drainOutDefaultQueue_set)(void);
	int32   (*gpon_autoDisTx_set)(rtk_enable_t);
    int32   (*gpon_scheInfo_get)(rtk_gpon_schedule_info_t *);
    int32   (*gpon_dataPath_reset)(void);
    int32   (*gpon_dsOmciCnt_get)(rtk_gpon_ds_omci_t *);
    int32   (*gpon_usOmciCnt_get)(rtk_gpon_us_omci_t *);
    int32   (*gpon_gtcDsTodSuperFrame_set)(uint32 superframe);
    int32   (*gpon_gtcDsTodSuperFrame_get)(uint32 *pSuperframe);
    int32   (*gpon_dbruBlockSize_get)(uint32 *pBlockSize);
    int32   (*gpon_dbruBlockSize_set)(uint32 blockSize);
    int32   (*gpon_flowctrl_adjust_byFlowNum)(uint32 flowNum);
	int32   (*gpon_usLaserDefault_get)(uint8 *pLaserOn, uint8 *pLaserOff);

#endif
    /* Classification */
    int32   (*classify_init)(void);
    int32   (*classify_cfgEntry_add)(rtk_classify_cfg_t *);
    int32   (*classify_cfgEntry_get)(rtk_classify_cfg_t *);
    int32   (*classify_cfgEntry_del)(uint32);
    int32   (*classify_field_add)(rtk_classify_cfg_t *, rtk_classify_field_t *);
    int32   (*classify_unmatchAction_set)(rtk_classify_unmatch_action_t);
    int32   (*classify_unmatchAction_get)(rtk_classify_unmatch_action_t *);
    int32   (*classify_unmatchAction_ds_set)(rtk_classify_unmatch_action_ds_t);
    int32   (*classify_unmatchAction_ds_get)(rtk_classify_unmatch_action_ds_t *);
    int32   (*classify_portRange_set)(rtk_classify_rangeCheck_l4Port_t *);
    int32   (*classify_portRange_get)(rtk_classify_rangeCheck_l4Port_t *);
    int32   (*classify_ipRange_set)(rtk_classify_rangeCheck_ip_t *);
    int32   (*classify_ipRange_get)(rtk_classify_rangeCheck_ip_t *);
    int32   (*classify_cf_sel_set)(rtk_port_t, rtk_classify_cf_sel_t);
    int32   (*classify_cf_sel_get)(rtk_port_t, rtk_classify_cf_sel_t *);
    int32   (*classify_cfPri2Dscp_set)(rtk_pri_t pri, rtk_dscp_t dscp);
    int32   (*classify_cfPri2Dscp_get)(rtk_pri_t pri, rtk_dscp_t *pDscp);
    int32   (*classify_permit_sel_set)(rtk_classify_permit_sel_t);
    int32   (*classify_permit_sel_get)(rtk_classify_permit_sel_t *);
    int32   (*classify_us1pRemarkPrior_set)(rtk_classify_us_1premark_prior_t);
    int32   (*classify_us1pRemarkPrior_get)(rtk_classify_us_1premark_prior_t *);
    int32   (*classify_templateCfgPattern0_set)(rtk_classify_template_cfg_pattern0_t *);
    int32   (*classify_templateCfgPattern0_get)(rtk_classify_template_cfg_pattern0_t *);
    int32   (*classify_entryNumPattern1_set)(uint32);
    int32   (*classify_entryNumPattern1_get)(uint32 *);
    int32   (*classify_defaultWanIf_set)(rtk_classify_default_wanIf_t *);
    int32   (*classify_defaultWanIf_get)(rtk_classify_default_wanIf_t *);

    /* statistics */
    int32   (*stat_init)(void);
    int32   (*stat_global_reset)(void);
    int32   (*stat_port_reset)(rtk_port_t);
    int32   (*stat_log_reset)(uint32);
    int32   (*stat_hostCnt_reset)(uint32);
    int32   (*stat_hostCnt_get)(uint32, rtk_stat_host_cnt_type_t, uint64 *);
    int32   (*stat_hostState_get)(uint32, rtk_enable_t*);
    int32   (*stat_hostState_set)(uint32, rtk_enable_t);
	int32   (*stat_rst_cnt_value_set)(rtk_mib_rst_value_t);
    int32   (*stat_rst_cnt_value_get)(rtk_mib_rst_value_t *);
    int32   (*stat_global_get)(rtk_stat_global_type_t, uint64 *);
    int32   (*stat_global_getAll)(rtk_stat_global_cntr_t *);
    int32   (*stat_port_get)(rtk_port_t, rtk_stat_port_type_t, uint64 *);
    int32   (*stat_port_getAll)(rtk_port_t, rtk_stat_port_cntr_t *);
    int32   (*stat_log_get)(uint32, uint64 *);
    int32   (*stat_log_ctrl_set)(uint32, rtk_stat_log_ctrl_t);
    int32   (*stat_log_ctrl_get)(uint32, rtk_stat_log_ctrl_t *);
    int32   (*stat_mib_cnt_mode_get)(rtk_mib_count_mode_t *);
    int32   (*stat_mib_cnt_mode_set)(rtk_mib_count_mode_t);
    int32   (*stat_mib_latch_timer_get)(uint32 *);
    int32   (*stat_mib_latch_timer_set)(uint32);
    int32   (*stat_mib_sync_mode_get)(rtk_mib_sync_mode_t *);
    int32   (*stat_mib_sync_mode_set)(rtk_mib_sync_mode_t);
    int32   (*stat_mib_count_tag_length_get)(rtk_mib_tag_cnt_dir_t, rtk_mib_tag_cnt_state_t *);
    int32   (*stat_mib_count_tag_length_set)(rtk_mib_tag_cnt_dir_t, rtk_mib_tag_cnt_state_t);
    int32   (*stat_pktInfo_get)(rtk_port_t, uint32 *);

    /* svlan */
    int32   (*svlan_init)(void);
    int32   (*svlan_create)(rtk_vlan_t);
    int32   (*svlan_destroy)(rtk_vlan_t);
    int32   (*svlan_portSvid_get)(rtk_port_t, rtk_vlan_t *);
    int32   (*svlan_portSvid_set)(rtk_port_t, rtk_vlan_t);
    int32   (*svlan_servicePort_get)(rtk_port_t, rtk_enable_t *);
    int32   (*svlan_servicePort_set)(rtk_port_t, rtk_enable_t);
    int32   (*svlan_memberPort_set)(rtk_vlan_t, rtk_portmask_t *, rtk_portmask_t *);
    int32   (*svlan_memberPort_get)(rtk_vlan_t, rtk_portmask_t *, rtk_portmask_t *);
    int32   (*svlan_tpidEntry_get)(uint32, uint32 *);
    int32   (*svlan_tpidEntry_set)(uint32, uint32);
    int32   (*svlan_priorityRef_set)(rtk_svlan_pri_ref_t);
    int32   (*svlan_priorityRef_get)(rtk_svlan_pri_ref_t *);
    int32   (*svlan_memberPortEntry_set)(rtk_svlan_memberCfg_t *);
    int32   (*svlan_memberPortEntry_get)(rtk_svlan_memberCfg_t *);
    int32   (*svlan_ipmc2s_add)(ipaddr_t, ipaddr_t, rtk_vlan_t);
    int32   (*svlan_ipmc2s_del)(ipaddr_t, ipaddr_t);
    int32   (*svlan_ipmc2s_get)(ipaddr_t, ipaddr_t, rtk_vlan_t *);
    int32   (*svlan_l2mc2s_add)(rtk_mac_t, rtk_mac_t, rtk_vlan_t);
    int32   (*svlan_l2mc2s_del)(rtk_mac_t, rtk_mac_t);
    int32   (*svlan_l2mc2s_get)(rtk_mac_t, rtk_mac_t, rtk_vlan_t *);
    int32   (*svlan_sp2c_add)(rtk_vlan_t, rtk_port_t, rtk_vlan_t);
    int32   (*svlan_sp2c_get)(rtk_vlan_t, rtk_port_t, rtk_vlan_t *);
    int32   (*svlan_sp2c_del)(rtk_vlan_t, rtk_port_t);
    int32   (*svlan_dmacVidSelState_set)(rtk_port_t, rtk_enable_t);
    int32   (*svlan_dmacVidSelState_get)(rtk_port_t, rtk_enable_t *);
    int32   (*svlan_unmatchAction_set)(rtk_svlan_action_t, rtk_vlan_t);
    int32   (*svlan_unmatchAction_get)(rtk_svlan_action_t *, rtk_vlan_t *);
    int32   (*svlan_untagAction_set)(rtk_svlan_action_t, rtk_vlan_t);
    int32   (*svlan_untagAction_get)(rtk_svlan_action_t *, rtk_vlan_t *);
    int32   (*svlan_c2s_add)(rtk_vlan_t, rtk_port_t, rtk_vlan_t);
    int32   (*svlan_c2s_del)(rtk_vlan_t, rtk_port_t, rtk_vlan_t);
    int32   (*svlan_c2s_get)(rtk_vlan_t, rtk_port_t, rtk_vlan_t *);
    int32   (*svlan_trapPri_get)(rtk_pri_t *);
    int32   (*svlan_trapPri_set)(rtk_pri_t);
    int32   (*svlan_deiKeepState_get)(rtk_enable_t *);
    int32   (*svlan_deiKeepState_set)(rtk_enable_t);
    int32   (*svlan_lookupType_get)(rtk_svlan_lookupType_t *);
    int32   (*svlan_lookupType_set)(rtk_svlan_lookupType_t);
    int32   (*svlan_sp2cUnmatchCtagging_get)(rtk_enable_t *);
    int32   (*svlan_sp2cUnmatchCtagging_set)(rtk_enable_t);
	int32   (*svlan_priority_get)(rtk_vlan_t, rtk_pri_t *);
	int32   (*svlan_priority_set)(rtk_vlan_t, rtk_pri_t);
	int32   (*svlan_fid_get)(rtk_vlan_t, rtk_fid_t *);
	int32   (*svlan_fid_set)(rtk_vlan_t, rtk_fid_t);
	int32   (*svlan_fidEnable_get)(rtk_vlan_t, rtk_enable_t *);
	int32   (*svlan_fidEnable_set)(rtk_vlan_t, rtk_enable_t);
	int32   (*svlan_enhancedFid_get)(rtk_vlan_t, rtk_efid_t *);
	int32   (*svlan_enhancedFid_set)(rtk_vlan_t, rtk_efid_t);
	int32   (*svlan_enhancedFidEnable_get)(rtk_vlan_t, rtk_enable_t *);
	int32   (*svlan_enhancedFidEnable_set)(rtk_vlan_t, rtk_enable_t);
    int32   (*svlan_dmacVidSelForcedState_set)(rtk_enable_t);
    int32   (*svlan_dmacVidSelForcedState_get)(rtk_enable_t *);
    int32   (*svlan_svlanFunctionEnable_get)(rtk_enable_t *);
    int32   (*svlan_svlanFunctionEnable_set)(rtk_enable_t);
    int32   (*svlan_tpidEnable_get)(uint32, rtk_enable_t *);
    int32   (*svlan_tpidEnable_set)(uint32, rtk_enable_t);
    int32   (*svlan_sp2cPriority_add)(rtk_vlan_t, rtk_port_t, rtk_pri_t);
    int32   (*svlan_sp2cPriority_get)(rtk_vlan_t, rtk_port_t, rtk_pri_t *);

    /*acl*/
    int32   (*acl_init)(void);
    int32   (*acl_template_set)(rtk_acl_template_t *);
    int32   (*acl_template_get)(rtk_acl_template_t *);
    int32   (*acl_fieldSelect_set)(rtk_acl_field_entry_t *);
    int32   (*acl_fieldSelect_get)(rtk_acl_field_entry_t *);
    int32   (*acl_igrRuleEntry_get)(rtk_acl_ingress_entry_t *);
    int32   (*acl_igrRuleField_add)(rtk_acl_ingress_entry_t *, rtk_acl_field_t *);
    int32   (*acl_igrRuleEntry_add)(rtk_acl_ingress_entry_t *);
    int32   (*acl_igrRuleEntry_del)(uint32);
    int32   (*acl_igrRuleEntry_delAll)(void);
    int32   (*acl_igrUnmatchAction_set)(rtk_port_t, rtk_filter_unmatch_action_type_t );
    int32   (*acl_igrUnmatchAction_get)(rtk_port_t, rtk_filter_unmatch_action_type_t *);
    int32   (*acl_igrState_set)(rtk_port_t, rtk_enable_t);
    int32   (*acl_igrState_get)(rtk_port_t, rtk_enable_t *);
    int32   (*acl_ipRange_set)(rtk_acl_rangeCheck_ip_t *);
    int32   (*acl_ipRange_get)(rtk_acl_rangeCheck_ip_t *);
    int32   (*acl_vidRange_set)(rtk_acl_rangeCheck_vid_t *);
    int32   (*acl_vidRange_get)(rtk_acl_rangeCheck_vid_t *);
    int32   (*acl_portRange_set)(rtk_acl_rangeCheck_l4Port_t *);
    int32   (*acl_portRange_get)(rtk_acl_rangeCheck_l4Port_t *);
    int32   (*acl_packetLengthRange_set)(rtk_acl_rangeCheck_pktLength_t *);
    int32   (*acl_packetLengthRange_get)(rtk_acl_rangeCheck_pktLength_t *);
    int32   (*acl_igrRuleMode_set)(rtk_acl_igr_rule_mode_t );
    int32   (*acl_igrRuleMode_get)(rtk_acl_igr_rule_mode_t *);
    int32   (*acl_igrPermitState_set)(rtk_port_t, rtk_enable_t);
    int32   (*acl_igrPermitState_get)(rtk_port_t, rtk_enable_t *);
	int32   (*acl_dbgInfo_get)(rtk_acl_dbgCnt_t *);
	int32   (*acl_dbgHitReason_get)(rtk_acl_debug_reason_t *);
    /*sec*/
    int32   (*sec_init)(void);
    int32   (*sec_portAttackPreventState_get)(rtk_port_t, rtk_enable_t *);
    int32   (*sec_portAttackPreventState_set)(rtk_port_t, rtk_enable_t);
    int32   (*sec_attackPrevent_get)(rtk_sec_attackType_t, rtk_action_t *);
    int32   (*sec_attackPrevent_set)(rtk_sec_attackType_t, rtk_action_t);
    int32   (*sec_attackFloodThresh_get)(rtk_sec_attackFloodType_t, uint32 *);
    int32   (*sec_attackFloodThresh_set)(rtk_sec_attackFloodType_t, uint32);

    /*rate*/
    int32   (*rate_init)(void);
    int32   (*rate_portIgrBandwidthCtrlRate_get)(rtk_port_t, uint32 *);
    int32   (*rate_portIgrBandwidthCtrlRate_set)(rtk_port_t, uint32);
    int32   (*rate_portIgrBandwidthCtrlIncludeIfg_get)(rtk_port_t, rtk_enable_t *);
    int32   (*rate_portIgrBandwidthCtrlIncludeIfg_set)(rtk_port_t, rtk_enable_t);
    int32   (*rate_portEgrBandwidthCtrlRate_get)(rtk_port_t, uint32 *);
    int32   (*rate_portEgrBandwidthCtrlRate_set)(rtk_port_t, uint32);
    int32   (*rate_egrBandwidthCtrlIncludeIfg_get)(rtk_enable_t *);
    int32   (*rate_egrBandwidthCtrlIncludeIfg_set)(rtk_enable_t);
    int32   (*rate_portEgrBandwidthCtrlIncludeIfg_get)(rtk_port_t, rtk_enable_t *);
    int32   (*rate_portEgrBandwidthCtrlIncludeIfg_set)(rtk_port_t, rtk_enable_t);
    int32   (*rate_egrQueueBwCtrlEnable_get)(rtk_port_t, rtk_qid_t, rtk_enable_t *);
    int32   (*rate_egrQueueBwCtrlEnable_set)(rtk_port_t, rtk_qid_t, rtk_enable_t);
    int32   (*rate_egrQueueBwCtrlMeterIdx_get)(rtk_port_t, rtk_qid_t, uint32 *);
    int32   (*rate_egrQueueBwCtrlMeterIdx_set)(rtk_port_t, rtk_qid_t, uint32);
    int32   (*rate_stormControlMeterIdx_get)(rtk_port_t, rtk_rate_storm_group_t, uint32 *);
    int32   (*rate_stormControlMeterIdx_set)(rtk_port_t, rtk_rate_storm_group_t, uint32);
    int32   (*rate_stormControlPortEnable_get)(rtk_port_t, rtk_rate_storm_group_t, rtk_enable_t *);
    int32   (*rate_stormControlPortEnable_set)(rtk_port_t, rtk_rate_storm_group_t, rtk_enable_t);
    int32   (*rate_stormControlEnable_get)(rtk_rate_storm_group_ctrl_t *);
    int32   (*rate_stormControlEnable_set)(rtk_rate_storm_group_ctrl_t *);
    int32   (*rate_stormBypass_set)(rtk_storm_bypass_t, rtk_enable_t);
    int32   (*rate_stormBypass_get)(rtk_storm_bypass_t, rtk_enable_t *);
    int32   (*rate_shareMeter_set)(uint32, uint32, rtk_enable_t);
    int32   (*rate_shareMeter_get)(uint32, uint32 *, rtk_enable_t *);
    int32   (*rate_shareMeterBucket_set)(uint32, uint32);
    int32   (*rate_shareMeterBucket_get)(uint32, uint32 *);
    int32   (*rate_shareMeterExceed_get)(uint32, uint32 *);
    int32   (*rate_shareMeterExceed_clear)(uint32);
    int32   (*rate_shareMeterMode_set)(uint32, rtk_rate_metet_mode_t);
    int32   (*rate_shareMeterMode_get)(uint32, rtk_rate_metet_mode_t *);
    int32   (*rate_hostIgrBwCtrlState_set)(uint32, rtk_enable_t);
    int32   (*rate_hostIgrBwCtrlState_get)(uint32, rtk_enable_t *);
    int32   (*rate_hostEgrBwCtrlState_set)(uint32, rtk_enable_t);
    int32   (*rate_hostEgrBwCtrlState_get)(uint32, rtk_enable_t *);
    int32   (*rate_hostBwCtrlMeterIdx_set)(uint32, uint32);
    int32   (*rate_hostBwCtrlMeterIdx_get)(uint32, uint32 *);
    int32   (*rate_hostMacAddr_set)(uint32, rtk_mac_t *);
    int32   (*rate_hostMacAddr_get)(uint32, rtk_mac_t *);

    /* Mirror */
    int32   (*mirror_init)(void);
    int32   (*mirror_portBased_set)(rtk_port_t, rtk_portmask_t *, rtk_portmask_t *);
    int32   (*mirror_portBased_get)(rtk_port_t *, rtk_portmask_t *, rtk_portmask_t *);
    int32   (*mirror_portIso_set)(rtk_enable_t);
    int32   (*mirror_portIso_get)(rtk_enable_t *);

    /* Trunk */
    int32   (*trunk_init)(void);
    int32   (*trunk_distributionAlgorithm_get)(uint32, uint32 *);
    int32   (*trunk_distributionAlgorithm_set)(uint32, uint32);
    int32   (*trunk_port_get)(uint32, rtk_portmask_t *);
    int32   (*trunk_port_set)(uint32, rtk_portmask_t *);
    int32   (*trunk_hashMappingTable_get)(uint32, rtk_trunk_hashVal2Port_t *);
    int32   (*trunk_hashMappingTable_set)(uint32, rtk_trunk_hashVal2Port_t *);
    int32   (*trunk_mode_get)(rtk_trunk_mode_t *);
    int32   (*trunk_mode_set)(rtk_trunk_mode_t);
    int32   (*trunk_trafficSeparate_get)(uint32, rtk_trunk_separateType_t *);
    int32   (*trunk_trafficSeparate_set)(uint32, rtk_trunk_separateType_t);
    int32   (*trunk_portQueueEmpty_get)(rtk_portmask_t *);
    int32   (*trunk_trafficPause_get)(uint32, rtk_enable_t *);
    int32   (*trunk_trafficPause_set)(uint32, rtk_enable_t);

    /*LED*/
    int32   (*led_init)(void);
    int32   (*led_operation_get)(rtk_led_operation_t *);
    int32   (*led_operation_set)(rtk_led_operation_t);
    int32   (*led_serialMode_get)(rtk_led_active_t *);
    int32   (*led_serialMode_set)(rtk_led_active_t);
    int32   (*led_blinkRate_get)(rtk_led_blinkGroup_t, rtk_led_blink_rate_t *);
    int32   (*led_blinkRate_set)(rtk_led_blinkGroup_t, rtk_led_blink_rate_t);
    int32   (*led_config_set)(uint32, rtk_led_type_t, rtk_led_config_t *);
    int32   (*led_config_get)(uint32, rtk_led_type_t *, rtk_led_config_t *);
    int32   (*led_modeForce_get)(uint32, rtk_led_force_mode_t *);
    int32   (*led_modeForce_set)(uint32, rtk_led_force_mode_t);
    int32   (*led_parallelEnable_get)(uint32, rtk_enable_t *);
    int32   (*led_parallelEnable_set)(uint32, rtk_enable_t);
    int32   (*led_ponAlarm_get)(rtk_enable_t *);
    int32   (*led_ponAlarm_set)(rtk_enable_t);
    int32   (*led_ponWarning_get)(rtk_enable_t *);
    int32   (*led_ponWarning_set)(rtk_enable_t);

    /*dot1x*/
    int32   (*dot1x_init)(void);
    int32   (*dot1x_unauthPacketOper_get)(rtk_port_t, rtk_action_t *);
    int32   (*dot1x_unauthPacketOper_set)(rtk_port_t, rtk_action_t);
    int32   (*dot1x_portBasedEnable_get)(rtk_port_t, rtk_enable_t *);
    int32   (*dot1x_portBasedEnable_set)(rtk_port_t, rtk_enable_t);
    int32   (*dot1x_portBasedAuthStatus_get)(rtk_port_t, rtk_dot1x_auth_status_t *);
    int32   (*dot1x_portBasedAuthStatus_set)(rtk_port_t, rtk_dot1x_auth_status_t);
    int32   (*dot1x_portBasedDirection_get)(rtk_port_t, rtk_dot1x_direction_t *);
    int32   (*dot1x_portBasedDirection_set)(rtk_port_t, rtk_dot1x_direction_t);
    int32   (*dot1x_macBasedEnable_get)(rtk_port_t, rtk_enable_t *);
    int32   (*dot1x_macBasedEnable_set)(rtk_port_t, rtk_enable_t);
    int32   (*dot1x_macBasedDirection_get)(rtk_dot1x_direction_t *);
    int32   (*dot1x_macBasedDirection_set)(rtk_dot1x_direction_t);
    int32   (*dot1x_guestVlan_get)(rtk_vlan_t *);
    int32   (*dot1x_guestVlan_set)(rtk_vlan_t);
    int32   (*dot1x_guestVlanBehavior_get)(rtk_dot1x_guestVlanBehavior_t *);
    int32   (*dot1x_guestVlanBehavior_set)(rtk_dot1x_guestVlanBehavior_t);
    int32   (*dot1x_trapPri_get)(rtk_pri_t *);
    int32   (*dot1x_trapPri_set)(rtk_pri_t);

    /*oma*/
    int32   (*oam_init)(void);
    int32   (*oam_parserAction_set)(rtk_port_t, rtk_oam_parser_act_t);
    int32   (*oam_parserAction_get)(rtk_port_t, rtk_oam_parser_act_t *);
    int32   (*oam_multiplexerAction_set)(rtk_port_t, rtk_oam_multiplexer_act_t);
    int32   (*oam_multiplexerAction_get)(rtk_port_t, rtk_oam_multiplexer_act_t *);

	/*interrupt*/
	int32   (*intr_init)(void);
	int32   (*intr_polarity_set)(rtk_intr_polarity_t);
	int32   (*intr_polarity_get)(rtk_intr_polarity_t *);
	int32   (*intr_imr_set)(rtk_intr_type_t, rtk_enable_t);
	int32   (*intr_imr_get)(rtk_intr_type_t, rtk_enable_t *);
	int32   (*intr_ims_get)(rtk_intr_type_t, rtk_enable_t *);
	int32   (*intr_ims_clear)(rtk_intr_type_t);
	int32   (*intr_speedChangeStatus_get)(rtk_portmask_t *);
	int32   (*intr_speedChangeStatus_clear)(void);
	int32   (*intr_linkupStatus_get)(rtk_portmask_t *);
	int32   (*intr_linkupStatus_clear)(void);
	int32   (*intr_linkdownStatus_get)(rtk_portmask_t *);
	int32   (*intr_linkdownStatus_clear)(void);
	int32   (*intr_gphyStatus_get)(rtk_portmask_t *);
	int32   (*intr_gphyStatus_clear)(void);
	int32   (*intr_imr_restore)(uint32 );

    /* RLDP and RLPP */
    int32   (*rldp_init)(void);
    int32   (*rldp_config_set)(rtk_rldp_config_t *);
    int32   (*rldp_config_get)(rtk_rldp_config_t *);
    int32   (*rldp_portConfig_set)(rtk_port_t, rtk_rldp_portConfig_t *);
    int32   (*rldp_portConfig_get)(rtk_port_t, rtk_rldp_portConfig_t *);
    int32   (*rldp_status_get)(rtk_rldp_status_t *);
    int32   (*rldp_portStatus_get)(rtk_port_t, rtk_rldp_portStatus_t *);
    int32   (*rldp_portStatus_clear)(rtk_port_t, rtk_rldp_portStatus_t *);
    int32   (*rlpp_init)(void);
    int32   (*rlpp_trapType_set)(rtk_rlpp_trapType_t);
    int32   (*rlpp_trapType_get)(rtk_rlpp_trapType_t *);

    /*cpu*/
    int32   (*cpu_init)(void);
    int32   (*cpu_awarePortMask_set)(rtk_portmask_t);
    int32   (*cpu_awarePortMask_get)(rtk_portmask_t *);
    int32   (*cpu_tagFormat_set)(rtk_cpu_tag_fmt_t);
    int32   (*cpu_tagFormat_get)(rtk_cpu_tag_fmt_t *);
    int32   (*cpu_trapInsertTag_set)(rtk_enable_t);
    int32   (*cpu_trapInsertTag_get)(rtk_enable_t *);
    int32   (*cpu_tagAware_set)(rtk_enable_t);
    int32   (*cpu_tagAware_get)(rtk_enable_t *);
    int32   (*cpu_trapInsertTagByPort_set)(rtk_port_t, rtk_enable_t);
    int32   (*cpu_trapInsertTagByPort_get)(rtk_port_t, rtk_enable_t *);
    int32   (*cpu_tagAwareByPort_set)(rtk_port_t, rtk_enable_t);
    int32   (*cpu_tagAwareByPort_get)(rtk_port_t, rtk_enable_t *);

    /*gpio*/
    int32   (*gpio_init)(void);
    int32   (*gpio_state_set)(uint32, rtk_enable_t);
    int32   (*gpio_state_get)(uint32, rtk_enable_t*);
    int32   (*gpio_mode_set)(uint32, rtk_gpio_mode_t );
    int32   (*gpio_mode_get)(uint32, rtk_gpio_mode_t* );
    int32   (*gpio_databit_get)(uint32, uint32 *);
    int32   (*gpio_databit_set)(uint32, uint32 );
    int32   (*gpio_intr_set)(uint32 ,rtk_gpio_intrMode_t);
    int32   (*gpio_intr_get)(uint32 ,rtk_gpio_intrMode_t *);
	int32   (*gpio_intrStatus_clean)(uint32);
    int32   (*gpio_intrStatus_get)(uint32 ,rtk_enable_t *);

    /* I2C */
    int32   (*i2c_init)(rtk_i2c_port_t);
    int32   (*i2c_enable_set)(rtk_i2c_port_t, rtk_enable_t);
    int32   (*i2c_enable_get)(rtk_i2c_port_t, rtk_enable_t *);
    int32   (*i2c_width_set)(rtk_i2c_port_t, rtk_i2c_width_t);
    int32   (*i2c_width_get)(rtk_i2c_port_t, rtk_i2c_width_t *);
    int32   (*i2c_write)(rtk_i2c_port_t, uint32, uint32, uint32);
    int32   (*i2c_read)(rtk_i2c_port_t, uint32, uint32, uint32 *);
    int32   (*i2c_clock_set)(rtk_i2c_port_t, uint32 );
    int32   (*i2c_clock_get)(rtk_i2c_port_t, uint32 *);
    int32   (*i2c_eepMirror_set)(rtk_i2c_eeprom_mirror_t);
    int32   (*i2c_eepMirror_get)(rtk_i2c_eeprom_mirror_t *);
    int32   (*i2c_eepMirror_write)(uint32, uint32);
    int32   (*i2c_eepMirror_read)(uint32, uint32 *);
    int32   (*i2c_dataWidth_set)(rtk_i2c_port_t, rtk_i2c_width_t);
    int32   (*i2c_dataWidth_get)(rtk_i2c_port_t, rtk_i2c_width_t *);
    int32   (*i2c_addrWidth_set)(rtk_i2c_port_t, rtk_i2c_width_t);
    int32   (*i2c_addrWidth_get)(rtk_i2c_port_t, rtk_i2c_width_t *);

    /* PTP Function */
    int32   (*time_portTransparentEnable_set)(rtk_port_t, rtk_enable_t);
    int32   (*time_portTransparentEnable_get)(rtk_port_t, rtk_enable_t *);
    int32   (*time_init)(void);
    int32   (*time_portPtpEnable_get)(rtk_port_t, rtk_enable_t *);
    int32   (*time_portPtpEnable_set)(rtk_port_t, rtk_enable_t);
    int32   (*time_curTime_get)(rtk_time_timeStamp_t *);
    int32   (*time_curTime_latch)(void);
    int32   (*time_refTime_get)(uint32 *,rtk_time_timeStamp_t *);
    int32   (*time_refTime_set)(uint32, rtk_time_timeStamp_t);
    int32   (*time_frequency_set)(uint32);
    int32   (*time_frequency_get)(uint32 *);
    int32   (*time_ptpIgrMsgAction_set)(rtk_time_ptpMsgType_t, rtk_time_ptpIgrMsg_action_t);
    int32   (*time_ptpIgrMsgAction_get)(rtk_time_ptpMsgType_t, rtk_time_ptpIgrMsg_action_t *);
    int32   (*time_ptpEgrMsgAction_set)(rtk_time_ptpMsgType_t, rtk_time_ptpEgrMsg_action_t);
    int32   (*time_ptpEgrMsgAction_get)(rtk_time_ptpMsgType_t, rtk_time_ptpEgrMsg_action_t *);
    int32   (*time_meanPathDelay_set)(uint32);
    int32   (*time_meanPathDelay_get)(uint32 *);
    int32   (*time_rxTime_set)(rtk_time_timeStamp_t);
    int32   (*time_rxTime_get)(rtk_time_timeStamp_t *);
    int32   (*time_ponTodTime_set)(rtk_pon_tod_t);
    int32   (*time_ponTodTime_get)(rtk_pon_tod_t *);
    int32   (*time_portPtpTxIndicator_get)(rtk_port_t, rtk_enable_t *);
    int32   (*time_todEnable_set)(rtk_enable_t);
    int32   (*time_todEnable_get)(rtk_enable_t *);
    int32   (*time_ppsEnable_set)(rtk_enable_t);
    int32   (*time_ppsEnable_get)(rtk_enable_t *);
    int32   (*time_ppsMode_set)(rtk_time_ptpPpsMode_t);
    int32   (*time_ppsMode_get)(rtk_time_ptpPpsMode_t *);

    /*EPON*/
    int32   (*epon_init)(void);
    int32   (*epon_intrMask_get)(rtk_epon_intrType_t, rtk_enable_t *);
    int32   (*epon_intrMask_set)(rtk_epon_intrType_t, rtk_enable_t);
    int32   (*epon_intr_get)(rtk_epon_intrType_t, rtk_enable_t *);
    int32   (*epon_intr_disableAll)(void);
    int32   (*epon_llid_entry_set)(rtk_epon_llid_entry_t *);
    int32   (*epon_llid_entry_get)(rtk_epon_llid_entry_t *);
    int32   (*epon_forceLaserState_set)(rtk_epon_laser_status_t);
    int32   (*epon_forceLaserState_get)(rtk_epon_laser_status_t *);
    int32   (*epon_laserTime_set)(uint8, uint8);
    int32   (*epon_laserTime_get)(uint8 *, uint8 *);
    int32   (*epon_syncTime_get)(uint8 *);
    int32   (*epon_registerReq_get)(rtk_epon_regReq_t *);
    int32   (*epon_registerReq_set)(rtk_epon_regReq_t *);
    int32   (*epon_churningKey_get)(rtk_epon_churningKeyEntry_t *);
    int32   (*epon_churningKey_set)(rtk_epon_churningKeyEntry_t *);
    int32   (*epon_usFecState_get)(rtk_enable_t *);
    int32   (*epon_usFecState_set)(rtk_enable_t);
    int32   (*epon_dsFecState_get)(rtk_enable_t *);
    int32   (*epon_dsFecState_set)(rtk_enable_t);
    int32   (*epon_mibCounter_get)(rtk_epon_counter_t *);
    int32   (*epon_mibGlobal_reset)(void);
    int32   (*epon_mibLlidIdx_reset)(uint8);
    int32   (*epon_losState_get)(rtk_enable_t *);
    int32   (*epon_mpcpTimeoutVal_get)(uint32 *);
    int32   (*epon_mpcpTimeoutVal_set)(uint32);
    int32   (*epon_opticalPolarity_set)(rtk_epon_polarity_t);
    int32   (*epon_opticalPolarity_get)(rtk_epon_polarity_t *);
    int32   (*epon_fecState_get)(rtk_enable_t *);
    int32   (*epon_fecState_set)(rtk_enable_t);
    int32   (*epon_llidEntryNum_get)(uint32 *);
    int32   (*epon_thresholdReport_set)(uint8, rtk_epon_report_threshold_t *);
    int32   (*epon_thresholdReport_get)(uint8, rtk_epon_report_threshold_t *);
    int32   (*epon_dbgInfo_get)(rtk_epon_dbgCnt_t *);
    int32   (*epon_forcePRBS_set)(rtk_epon_prbs_t);
    int32   (*epon_multiLlidMode_set)(rtk_epon_multiLlidMode_t);
    int32   (*epon_multiLlidMode_get)(rtk_epon_multiLlidMode_t *);

} dal_mapper_t;




/*NAPTR Inbound table access*/


/*
 * Macro Definition
 */


/*
 * Function Declaration
 */

/* Module Name : */


#endif /* __DAL_MAPPER_H __ */
