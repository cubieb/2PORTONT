/*
 * Copyright (C) 2009 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 */

/*
 * Include Files
 */
#include <common/debug/rt_log.h>
#include <dal/dal_mapper.h>
#include <dal/dal_common.h>
#include <dal/apollo/dal_apollo_switch.h>
#include <dal/apollo/dal_apollo_mapper.h>
#include <dal/apollo/dal_apollo_l34.h>
#include <dal/apollo/dal_apollo_vlan.h>
#include <dal/apollo/dal_apollo_l2.h>
#include <dal/apollo/dal_apollo_port.h>
#include <dal/apollo/dal_apollo_ponmac.h>
#include <dal/apollo/dal_apollo_gpon.h>
#include <dal/apollo/dal_apollo_stp.h>
#include <dal/apollo/dal_apollo_classify.h>
#include <dal/apollo/dal_apollo_stat.h>
#include <dal/apollo/dal_apollo_qos.h>
#include <dal/apollo/dal_apollo_svlan.h>
#include <dal/apollo/dal_apollo_acl.h>
#include <dal/apollo/dal_apollo_trap.h>
#include <dal/apollo/dal_apollo_rate.h>
#include <dal/apollo/dal_apollo_sec.h>
#include <dal/apollo/dal_apollo_mirror.h>
#include <dal/apollo/dal_apollo_trunk.h>
#include <dal/apollo/dal_apollo_led.h>
#include <dal/apollo/dal_apollo_dot1x.h>
#include <dal/apollo/dal_apollo_oam.h>
#include <dal/apollo/dal_apollo_intr.h>
#include <dal/apollo/dal_apollo_cpu.h>
#include <dal/apollo/dal_apollo_rldp.h>
#include <dal/apollo/dal_apollo_gpio.h>
#include <dal/apollo/dal_apollo_i2c.h>
#include <dal/apollo/dal_apollo_time.h>

/*
#include <dal/apollo/dal_apollo_dot1x.h>
#include <dal/apollo/dal_apollo_flowctrl.h>
#include <dal/apollo/dal_apollo_svlan.h>
#include <dal/apollo/dal_apollo_eee.h>
#include <dal/apollo/dal_apollo_led.h>

*/

/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */
dal_mapper_t dal_apollo_mapper =
{
    APOLLO_CHIP_ID,
    ._init = dal_apollo_init,

    /* Switch */
    .switch_init = dal_apollo_switch_init,
    .switch_phyPortId_get = dal_apollo_switch_phyPortId_get,
    .switch_logicalPort_get = dal_apollo_switch_logicalPort_get,
    .switch_port2PortMask_set = dal_apollo_switch_port2PortMask_set,
    .switch_port2PortMask_clear = dal_apollo_switch_port2PortMask_clear,
    .switch_portIdInMask_check = dal_apollo_switch_portIdInMask_check,
    .switch_maxPktLenLinkSpeed_get = dal_apollo_switch_maxPktLenLinkSpeed_get,
    .switch_maxPktLenLinkSpeed_set = dal_apollo_switch_maxPktLenLinkSpeed_set,
    .switch_mgmtMacAddr_get = dal_apollo_switch_mgmtMacAddr_get,
    .switch_mgmtMacAddr_set = dal_apollo_switch_mgmtMacAddr_set,
    .switch_chip_reset = dal_apollo_switch_chip_reset,


    /* VLAN */
    .vlan_init = dal_apollo_vlan_init,
    .vlan_create = dal_apollo_vlan_create,
    .vlan_destroy = dal_apollo_vlan_destroy,
    .vlan_destroyAll = dal_apollo_vlan_destroyAll,
    .vlan_fid_get = dal_apollo_vlan_fid_get,
    .vlan_fid_set = dal_apollo_vlan_fid_set,
    .vlan_fidMode_get = dal_apollo_vlan_fidMode_get,
    .vlan_fidMode_set = dal_apollo_vlan_fidMode_set,
    .vlan_port_get = dal_apollo_vlan_port_get,
    .vlan_port_set = dal_apollo_vlan_port_set,
    .vlan_extPort_get = dal_apollo_vlan_extPort_get,
    .vlan_extPort_set = dal_apollo_vlan_extPort_set,
    .vlan_stg_get = dal_apollo_vlan_stg_get,
    .vlan_stg_set = dal_apollo_vlan_stg_set,
    .vlan_priority_get = dal_apollo_vlan_priority_get,
    .vlan_priority_set = dal_apollo_vlan_priority_set,
    .vlan_priorityEnable_get = dal_apollo_vlan_priorityEnable_get,
    .vlan_priorityEnable_set = dal_apollo_vlan_priorityEnable_set,
    .vlan_policingEnable_get = dal_apollo_vlan_policingEnable_get,
    .vlan_policingEnable_set = dal_apollo_vlan_policingEnable_set,
    .vlan_policingMeterIdx_get = dal_apollo_vlan_policingMeterIdx_get,
    .vlan_policingMeterIdx_set = dal_apollo_vlan_policingMeterIdx_set,
    .vlan_portAcceptFrameType_get = dal_apollo_vlan_portAcceptFrameType_get,
    .vlan_portAcceptFrameType_set = dal_apollo_vlan_portAcceptFrameType_set,
    .vlan_vlanFunctionEnable_get = dal_apollo_vlan_vlanFunctionEnable_get,
    .vlan_vlanFunctionEnable_set = dal_apollo_vlan_vlanFunctionEnable_set,
    .vlan_portIgrFilterEnable_get = dal_apollo_vlan_portIgrFilterEnable_get,
    .vlan_portIgrFilterEnable_set = dal_apollo_vlan_portIgrFilterEnable_set,
    .vlan_leaky_get = dal_apollo_vlan_leaky_get,
    .vlan_leaky_set = dal_apollo_vlan_leaky_set,
    .vlan_portLeaky_get = dal_apollo_vlan_portLeaky_get,
    .vlan_portLeaky_set = dal_apollo_vlan_portLeaky_set,
    .vlan_keepType_get = dal_apollo_vlan_keepType_get,
    .vlan_keepType_set = dal_apollo_vlan_keepType_set,
    .vlan_portPvid_get = dal_apollo_vlan_portPvid_get,
    .vlan_portPvid_set = dal_apollo_vlan_portPvid_set,
    .vlan_extPortPvid_get = dal_apollo_vlan_extPortPvid_get,
    .vlan_extPortPvid_set = dal_apollo_vlan_extPortPvid_set,
    .vlan_protoGroup_get = dal_apollo_vlan_protoGroup_get,
    .vlan_protoGroup_set = dal_apollo_vlan_protoGroup_set,
    .vlan_portProtoVlan_get = dal_apollo_vlan_portProtoVlan_get,
    .vlan_portProtoVlan_set = dal_apollo_vlan_portProtoVlan_set,
    .vlan_tagMode_get = dal_apollo_vlan_tagMode_get,
    .vlan_tagMode_set = dal_apollo_vlan_tagMode_set,
    .vlan_portFid_get = dal_apollo_vlan_portFid_get,
    .vlan_portFid_set = dal_apollo_vlan_portFid_set,
    .vlan_portPriority_get = dal_apollo_vlan_portPriority_get,
    .vlan_portPriority_set = dal_apollo_vlan_portPriority_set,
    .vlan_portEgrTagKeepType_get = dal_apollo_vlan_portEgrTagKeepType_get,
    .vlan_portEgrTagKeepType_set = dal_apollo_vlan_portEgrTagKeepType_set,
    .vlan_transparentEnable_get = dal_apollo_vlan_transparentEnable_get,
    .vlan_transparentEnable_set = dal_apollo_vlan_transparentEnable_set,
    .vlan_cfiKeepEnable_get = dal_apollo_vlan_cfiKeepEnable_get,
    .vlan_cfiKeepEnable_set = dal_apollo_vlan_cfiKeepEnable_set,
    .vlan_reservedVidAction_get = dal_apollo_vlan_reservedVidAction_get,
    .vlan_reservedVidAction_set = dal_apollo_vlan_reservedVidAction_set,
    .vlan_tagModeIp4mc_get	= (int32 (*)(rtk_port_t, rtk_vlan_tagModeIpmc_t*))dal_common_unavail,
    .vlan_tagModeIp4mc_set	= (int32 (*)(rtk_port_t, rtk_vlan_tagModeIpmc_t))dal_common_unavail,
    .vlan_tagModeIp6mc_get	= (int32 (*)(rtk_port_t, rtk_vlan_tagModeIpmc_t*))dal_common_unavail,
    .vlan_tagModeIp6mc_set	= (int32 (*)(rtk_port_t, rtk_vlan_tagModeIpmc_t))dal_common_unavail,

    /* Trap */
    .trap_init = dal_apollo_trap_init,
    .trap_reasonTrapToCpuPriority_get = dal_apollo_trap_reasonTrapToCpuPriority_get,
    .trap_reasonTrapToCpuPriority_set = dal_apollo_trap_reasonTrapToCpuPriority_set,
    .trap_igmpCtrlPkt2CpuEnable_get = dal_apollo_trap_igmpCtrlPkt2CpuEnable_get,
    .trap_igmpCtrlPkt2CpuEnable_set = dal_apollo_trap_igmpCtrlPkt2CpuEnable_set,
    .trap_mldCtrlPkt2CpuEnable_get = dal_apollo_trap_mldCtrlPkt2CpuEnable_get,
    .trap_mldCtrlPkt2CpuEnable_set = dal_apollo_trap_mldCtrlPkt2CpuEnable_set,
    .trap_portIgmpMldCtrlPktAction_get = dal_apollo_trap_portIgmpMldCtrlPktAction_get,
    .trap_portIgmpMldCtrlPktAction_set = dal_apollo_trap_portIgmpMldCtrlPktAction_set,
    .trap_l2McastPkt2CpuEnable_get = dal_apollo_trap_l2McastPkt2CpuEnable_get,
    .trap_l2McastPkt2CpuEnable_set = dal_apollo_trap_l2McastPkt2CpuEnable_set,
    .trap_ipMcastPkt2CpuEnable_get = dal_apollo_trap_ipMcastPkt2CpuEnable_get,
    .trap_ipMcastPkt2CpuEnable_set = dal_apollo_trap_ipMcastPkt2CpuEnable_set,
    .trap_rmaAction_get = dal_apollo_trap_rmaAction_get,
    .trap_rmaAction_set = dal_apollo_trap_rmaAction_set,
    .trap_rmaPri_get = dal_apollo_trap_rmaPri_get,
    .trap_rmaPri_set = dal_apollo_trap_rmaPri_set,
    .trap_oamPduAction_get = dal_apollo_trap_oamPduAction_get,
    .trap_oamPduAction_set = dal_apollo_trap_oamPduAction_set,
    .trap_oamPduPri_get = dal_apollo_trap_oamPduPri_get,
    .trap_oamPduPri_set = dal_apollo_trap_oamPduPri_set,

    /* L2 */
    .l2_init                                    = dal_apollo_l2_init,
    .l2_flushLinkDownPortAddrEnable_get         = dal_apollo_l2_flushLinkDownPortAddrEnable_get,
    .l2_flushLinkDownPortAddrEnable_set         = dal_apollo_l2_flushLinkDownPortAddrEnable_set,
    .l2_ucastAddr_flush                         = dal_apollo_l2_ucastAddr_flush,
    .l2_table_clear                             = dal_apollo_l2_table_clear,
    .l2_limitLearningOverStatus_get             = dal_apollo_l2_limitLearningOverStatus_get,
    .l2_limitLearningOverStatus_clear           = dal_apollo_l2_limitLearningOverStatus_clear,
    .l2_learningCnt_get                         = dal_apollo_l2_learningCnt_get,
    .l2_limitLearningCnt_get                    = dal_apollo_l2_limitLearningCnt_get,
    .l2_limitLearningCnt_set                    = dal_apollo_l2_limitLearningCnt_set,
    .l2_limitLearningCntAction_get              = dal_apollo_l2_limitLearningCntAction_get,
    .l2_limitLearningCntAction_set              = dal_apollo_l2_limitLearningCntAction_set,
    .l2_limitLearningEntryAction_get            = NULL,
    .l2_limitLearningEntryAction_set            = NULL,
    .l2_limitLearningPortMask_get               = NULL,
    .l2_limitLearningPortMask_set               = NULL,
    .l2_portLimitLearningOverStatus_get         = dal_apollo_l2_portLimitLearningOverStatus_get,
    .l2_portLimitLearningOverStatus_clear       = dal_apollo_l2_portLimitLearningOverStatus_clear,
    .l2_portLearningCnt_get                     = dal_apollo_l2_portLearningCnt_get,
    .l2_portLimitLearningCnt_get                = dal_apollo_l2_portLimitLearningCnt_get,
    .l2_portLimitLearningCnt_set                = dal_apollo_l2_portLimitLearningCnt_set,
    .l2_portLimitLearningCntAction_get          = dal_apollo_l2_portLimitLearningCntAction_get,
    .l2_portLimitLearningCntAction_set          = dal_apollo_l2_portLimitLearningCntAction_set,
    .l2_aging_get                               = dal_apollo_l2_aging_get,
    .l2_aging_set                               = dal_apollo_l2_aging_set,
    .l2_portAgingEnable_get                     = dal_apollo_l2_portAgingEnable_get,
    .l2_portAgingEnable_set                     = dal_apollo_l2_portAgingEnable_set,
    .l2_lookupMissAction_get                    = dal_apollo_l2_lookupMissAction_get,
    .l2_lookupMissAction_set                    = dal_apollo_l2_lookupMissAction_set,
    .l2_portLookupMissAction_get                = dal_apollo_l2_portLookupMissAction_get,
    .l2_portLookupMissAction_set                = dal_apollo_l2_portLookupMissAction_set,
    .l2_lookupMissFloodPortMask_get             = dal_apollo_l2_lookupMissFloodPortMask_get,
    .l2_lookupMissFloodPortMask_set             = dal_apollo_l2_lookupMissFloodPortMask_set,
    .l2_lookupMissFloodPortMask_add             = dal_apollo_l2_lookupMissFloodPortMask_add,
    .l2_lookupMissFloodPortMask_del             = dal_apollo_l2_lookupMissFloodPortMask_del,
    .l2_newMacOp_get                            = dal_apollo_l2_newMacOp_get,
    .l2_newMacOp_set                            = dal_apollo_l2_newMacOp_set,
    .l2_nextValidAddr_get                       = dal_apollo_l2_nextValidAddr_get,
    .l2_nextValidAddrOnPort_get                 = dal_apollo_l2_nextValidAddrOnPort_get,
    .l2_nextValidMcastAddr_get                  = dal_apollo_l2_nextValidMcastAddr_get,
    .l2_nextValidIpMcastAddr_get                = dal_apollo_l2_nextValidIpMcastAddr_get,
    .l2_nextValidEntry_get                      = dal_apollo_l2_nextValidEntry_get,
    .l2_addr_add                                = dal_apollo_l2_addr_add,
    .l2_addr_del                                = dal_apollo_l2_addr_del,
    .l2_addr_get                                = dal_apollo_l2_addr_get,
    .l2_addr_delAll                             = dal_apollo_l2_addr_delAll,
    .l2_mcastAddr_add                           = dal_apollo_l2_mcastAddr_add,
    .l2_mcastAddr_del                           = dal_apollo_l2_mcastAddr_del,
    .l2_mcastAddr_get                           = dal_apollo_l2_mcastAddr_get,
    .l2_illegalPortMoveAction_get               = dal_apollo_l2_illegalPortMoveAction_get,
    .l2_illegalPortMoveAction_set               = dal_apollo_l2_illegalPortMoveAction_set,
    .l2_ipmcMode_get                            = dal_apollo_l2_ipmcMode_get,
    .l2_ipmcMode_set                            = dal_apollo_l2_ipmcMode_set,
    .l2_ipv6mcMode_get                          = NULL,
    .l2_ipv6mcMode_set                          = NULL,
    .l2_ipmcGroupLookupMissHash_get             = dal_apollo_l2_ipmcGroupLookupMissHash_get,
    .l2_ipmcGroupLookupMissHash_set             = dal_apollo_l2_ipmcGroupLookupMissHash_set,
    .l2_ipmcGroup_add                           = dal_apollo_l2_ipmcGroup_add,
    .l2_ipmcGroupExtPortmask_add                = NULL,
    .l2_ipmcGroup_del                           = dal_apollo_l2_ipmcGroup_del,
    .l2_ipmcGroup_get                           = dal_apollo_l2_ipmcGroup_get,
    .l2_ipmcGroupExtPortmask_get                = NULL,
    .l2_portIpmcAction_get                      = dal_apollo_l2_portIpmcAction_get,
    .l2_portIpmcAction_set                      = dal_apollo_l2_portIpmcAction_set,
    .l2_ipMcastAddr_add                         = dal_apollo_l2_ipMcastAddr_add,
    .l2_ipMcastAddr_del                         = dal_apollo_l2_ipMcastAddr_del,
    .l2_ipMcastAddr_get                         = dal_apollo_l2_ipMcastAddr_get,
    .l2_srcPortEgrFilterMask_get                = dal_apollo_l2_srcPortEgrFilterMask_get,
    .l2_srcPortEgrFilterMask_set                = dal_apollo_l2_srcPortEgrFilterMask_set,
    .l2_extPortEgrFilterMask_get                = dal_apollo_l2_extPortEgrFilterMask_get,
    .l2_extPortEgrFilterMask_set                = dal_apollo_l2_extPortEgrFilterMask_set,

    /* Port */
    .port_init = dal_apollo_port_init,
    .port_link_get = dal_apollo_port_link_get,
    .port_speedDuplex_get = dal_apollo_port_speedDuplex_get,
    .port_flowctrl_get = dal_apollo_port_flowctrl_get,
    .port_phyAutoNegoEnable_get = dal_apollo_port_phyAutoNegoEnable_get,
    .port_phyAutoNegoEnable_set = dal_apollo_port_phyAutoNegoEnable_set,
    .port_phyAutoNegoAbility_get = dal_apollo_port_phyAutoNegoAbility_get,
    .port_phyAutoNegoAbility_set = dal_apollo_port_phyAutoNegoAbility_set,
    .port_phyForceModeAbility_get = dal_apollo_port_phyForceModeAbility_get,
    .port_phyForceModeAbility_set = dal_apollo_port_phyForceModeAbility_set,
    .port_phyMasterSlave_get = dal_apollo_port_phyMasterSlave_get,
    .port_phyMasterSlave_set = dal_apollo_port_phyMasterSlave_set,
    .port_phyTestMode_get = dal_apollo_port_phyTestMode_get,
    .port_phyTestMode_set = dal_apollo_port_phyTestMode_set,
    .port_phyReg_get = dal_apollo_port_phyReg_get,
    .port_phyReg_set = dal_apollo_port_phyReg_set,
    .port_cpuPortId_get = dal_apollo_port_cpuPortId_get,
    .port_isolation_get = dal_apollo_port_isolation_get,
    .port_isolation_set = dal_apollo_port_isolation_set,
    .port_isolationExt_get = dal_apollo_port_isolationExt_get,
    .port_isolationExt_set = dal_apollo_port_isolationExt_set,
    .port_isolationL34_get = dal_apollo_port_isolationL34_get,
    .port_isolationL34_set = dal_apollo_port_isolationL34_set,
    .port_isolationExtL34_get = dal_apollo_port_isolationExtL34_get,
    .port_isolationExtL34_set = dal_apollo_port_isolationExtL34_set,
    .port_isolationEntry_get = (int32 (*)(rtk_port_isoConfig_t, rtk_port_t, rtk_portmask_t *, rtk_portmask_t *))dal_common_unavail,
    .port_isolationEntry_set = (int32 (*)(rtk_port_isoConfig_t, rtk_port_t, rtk_portmask_t *, rtk_portmask_t *))dal_common_unavail,
    .port_isolationEntryExt_get = (int32 (*)(rtk_port_isoConfig_t, rtk_port_t, rtk_portmask_t *, rtk_portmask_t *))dal_common_unavail,
    .port_isolationEntryExt_set = (int32 (*)(rtk_port_isoConfig_t, rtk_port_t, rtk_portmask_t *, rtk_portmask_t *))dal_common_unavail,
    .port_isolationCtagPktConfig_get = (int32 (*)(rtk_port_isoConfig_t *))dal_common_unavail,
    .port_isolationCtagPktConfig_set = (int32 (*)(rtk_port_isoConfig_t))dal_common_unavail,
    .port_isolationL34PktConfig_get = (int32 (*)(rtk_port_isoConfig_t *))dal_common_unavail,
    .port_isolationL34PktConfig_set = (int32 (*)(rtk_port_isoConfig_t))dal_common_unavail,
    .port_isolationIpmcLeaky_get = dal_apollo_port_isolationIpmcLeaky_get,
    .port_isolationIpmcLeaky_set = dal_apollo_port_isolationIpmcLeaky_set,
    .port_isolationPortLeaky_get                = dal_apollo_port_isolationPortLeaky_get,
    .port_isolationPortLeaky_set                = dal_apollo_port_isolationPortLeaky_set,
    .port_isolationLeaky_get                    = dal_apollo_port_isolationLeaky_get,
    .port_isolationLeaky_set                    = dal_apollo_port_isolationLeaky_set,
    .port_macRemoteLoopbackEnable_get = dal_apollo_port_macRemoteLoopbackEnable_get,
    .port_macRemoteLoopbackEnable_set = dal_apollo_port_macRemoteLoopbackEnable_set,
    .port_macLocalLoopbackEnable_get = dal_apollo_port_macLocalLoopbackEnable_get,
    .port_macLocalLoopbackEnable_set = dal_apollo_port_macLocalLoopbackEnable_set,
    .port_adminEnable_get = dal_apollo_port_adminEnable_get,
    .port_adminEnable_set = dal_apollo_port_adminEnable_set,
    .port_specialCongest_get = dal_apollo_port_specialCongest_get,
    .port_specialCongest_set = dal_apollo_port_specialCongest_set,
    .port_specialCongestStatus_get = dal_apollo_port_specialCongestStatus_get,
    .port_specialCongestStatus_clear = dal_apollo_port_specialCongestStatus_clear,
    .port_greenEnable_get = dal_apollo_port_greenEnable_get,
    .port_greenEnable_set = dal_apollo_port_greenEnable_set,
    .port_phyCrossOverMode_get = dal_apollo_port_phyCrossOverMode_get,
    .port_phyCrossOverMode_set = dal_apollo_port_phyCrossOverMode_set,
    .port_enhancedFid_get = dal_apollo_port_enhancedFid_get,
    .port_enhancedFid_set = dal_apollo_port_enhancedFid_set,
    .port_rtctResult_get = dal_apollo_port_rtctResult_get,
    .port_rtct_start = dal_apollo_port_rtct_start,
    .port_macForceAbility_set= dal_apollo_port_macForceAbility_set,
	.port_macForceAbility_get= dal_apollo_port_macForceAbility_get,
	.port_macForceAbilityState_set = dal_apollo_port_macForceAbilityState_set,
	.port_macForceAbilityState_get= dal_apollo_port_macForceAbilityState_get,
    .port_macExtMode_set                        = dal_apollo_port_macExtMode_set,
    .port_macExtMode_get                        = dal_apollo_port_macExtMode_get,
    .port_macExtRgmiiDelay_set                  = dal_apollo_port_macExtRgmiiDelay_set,
    .port_macExtRgmiiDelay_get                  = dal_apollo_port_macExtRgmiiDelay_get,

    /* QoS */
    .qos_init = dal_apollo_qos_init,
    .qos_priSelGroup_get = dal_apollo_qos_priSelGroup_get,
    .qos_priSelGroup_set = dal_apollo_qos_priSelGroup_set,
    .qos_portPri_get = dal_apollo_qos_portPri_get,
    .qos_portPri_set = dal_apollo_qos_portPri_set,
    .qos_dscpPriRemapGroup_get = dal_apollo_qos_dscpPriRemapGroup_get,
    .qos_dscpPriRemapGroup_set = dal_apollo_qos_dscpPriRemapGroup_set,
    .qos_1pPriRemapGroup_get = dal_apollo_qos_1pPriRemapGroup_get,
    .qos_1pPriRemapGroup_set = dal_apollo_qos_1pPriRemapGroup_set,
    .qos_priMap_get = dal_apollo_qos_priMap_get,
    .qos_priMap_set = dal_apollo_qos_priMap_set,
    .qos_portPriMap_get = dal_apollo_qos_portPriMap_get,
    .qos_portPriMap_set = dal_apollo_qos_portPriMap_set,
    .qos_1pRemarkEnable_get = dal_apollo_qos_1pRemarkEnable_get,
    .qos_1pRemarkEnable_set = dal_apollo_qos_1pRemarkEnable_set,
    .qos_1pRemarkGroup_get = dal_apollo_qos_1pRemarkGroup_get,
    .qos_1pRemarkGroup_set = dal_apollo_qos_1pRemarkGroup_set,
    .qos_dscpRemarkEnable_get = dal_apollo_qos_dscpRemarkEnable_get,
    .qos_dscpRemarkEnable_set = dal_apollo_qos_dscpRemarkEnable_set,
    .qos_dscpRemarkGroup_get = dal_apollo_qos_dscpRemarkGroup_get,
    .qos_dscpRemarkGroup_set = dal_apollo_qos_dscpRemarkGroup_set,
    .qos_fwd2CpuPriRemap_get = dal_apollo_qos_fwd2CpuPriRemap_get,
    .qos_fwd2CpuPriRemap_set = dal_apollo_qos_fwd2CpuPriRemap_set,
    .qos_portDscpRemarkSrcSel_get = dal_apollo_qos_portDscpRemarkSrcSel_get,
    .qos_portDscpRemarkSrcSel_set = dal_apollo_qos_portDscpRemarkSrcSel_set,
    .qos_dscp2DscpRemarkGroup_get = dal_apollo_qos_dscp2DscpRemarkGroup_get,
    .qos_dscp2DscpRemarkGroup_set = dal_apollo_qos_dscp2DscpRemarkGroup_set,
    .qos_schedulingQueue_get = dal_apollo_qos_schedulingQueue_get,
    .qos_schedulingQueue_set = dal_apollo_qos_schedulingQueue_set,
    .qos_portPriSelGroup_get = (int32 (*)(rtk_port_t, uint32 *))dal_common_unavail,
    .qos_portPriSelGroup_set = (int32 (*)(rtk_port_t, uint32))dal_common_unavail,

    /* L34 Function */
    .l34_init = dal_apollo_l34_init,
    .l34_netifTable_set = dal_apollo_l34_netifTable_set,
    .l34_netifTable_get = dal_apollo_l34_netifTable_get,
    .l34_arpTable_set = dal_apollo_l34_arpTable_set,
    .l34_arpTable_get = dal_apollo_l34_arpTable_get,
    .l34_arpTable_del = dal_apollo_l34_arpTable_del,
    .l34_pppoeTable_set = dal_apollo_l34_pppoeTable_set,
    .l34_pppoeTable_get = dal_apollo_l34_pppoeTable_get,

    .l34_routingTable_set = dal_apollo_l34_routingTable_set,
    .l34_routingTable_get = dal_apollo_l34_routingTable_get,
    .l34_routingTable_del = dal_apollo_l34_routingTable_del,
    .l34_nexthopTable_set = dal_apollo_l34_nexthopTable_set,
    .l34_nexthopTable_get = dal_apollo_l34_nexthopTable_get,
    .l34_extIntIPTable_set = dal_apollo_l34_extIntIPTable_set,
    .l34_extIntIPTable_get = dal_apollo_l34_extIntIPTable_get,
    .l34_extIntIPTable_del = dal_apollo_l34_extIntIPTable_del,
    .l34_naptInboundTable_set   = dal_apollo_l34_naptInboundTable_set,
    .l34_naptInboundTable_get   = dal_apollo_l34_naptInboundTable_get,
    .l34_naptOutboundTable_set   = dal_apollo_l34_naptOutboundTable_set,
    .l34_naptOutboundTable_get   = dal_apollo_l34_naptOutboundTable_get,
    .l34_ipmcTransTable_set         =  (int32 (*)(uint32, rtk_l34_ipmcTrans_entry_t *))dal_common_unavail,
    .l34_ipmcTransTable_get         =  (int32 (*)(uint32, rtk_l34_ipmcTrans_entry_t *))dal_common_unavail,
    .l34_table_reset               = dal_apollo_l34_table_reset,
	.l34_bindingTable_set = (int32 (*)(uint32 idx,rtk_binding_entry_t *entry))dal_common_unavail,
	.l34_bindingTable_get = (int32 (*)(uint32 idx,rtk_binding_entry_t *entry))dal_common_unavail,
	.l34_bindingAction_set = (int32 (*)(rtk_l34_bindType_t bindType, rtk_l34_bindAct_t action))dal_common_unavail,
	.l34_bindingAction_get = (int32 (*)(rtk_l34_bindType_t bindType, rtk_l34_bindAct_t *pAction))dal_common_unavail,
	.l34_wanTypeTable_set = (int32 (*)(uint32 idx, rtk_wanType_entry_t *entry))dal_common_unavail,
	.l34_wanTypeTable_get = (int32 (*)(uint32 idx, rtk_wanType_entry_t *entry))dal_common_unavail,
	.l34_ipv6RoutingTable_set = (int32 (*)(uint32 idx, rtk_ipv6Routing_entry_t *entry))dal_common_unavail,
	.l34_ipv6RoutingTable_get = (int32 (*)(uint32 idx, rtk_ipv6Routing_entry_t *entry))dal_common_unavail,
	.l34_ipv6NeighborTable_set = (int32 (*)(uint32 idx,rtk_ipv6Neighbor_entry_t *entry))dal_common_unavail,
	.l34_ipv6NeighborTable_get = (int32 (*)(uint32 idx,rtk_ipv6Neighbor_entry_t *entry))dal_common_unavail,
	.l34_hsabMode_set = dal_apollo_l34_hsabMode_set,
	.l34_hsabMode_get = dal_apollo_l34_hsabMode_get,
	.l34_hsaData_get = dal_apollo_l34_hsaData_get,
	.l34_hsbData_get = dal_apollo_l34_hsbData_get,
	.l34_portWanMap_set = (int32 (*)(rtk_l34_portWanMapType_t, rtk_l34_portWanMap_entry_t))dal_common_unavail,
	.l34_portWanMap_get = (int32 (*)(rtk_l34_portWanMapType_t, rtk_l34_portWanMap_entry_t *))dal_common_unavail,
	.l34_globalState_set = dal_apollo_l34_globalState_set,
	.l34_globalState_get = dal_apollo_l34_globalState_get,
	.l34_lookupMode_set = dal_apollo_l34_lookupMode_set,
	.l34_lookupMode_get = dal_apollo_l34_lookupMode_get,
	.l34_lookupPortMap_set = dal_apollo_l34_lookupPortMap_set,
	.l34_lookupPortMap_get = dal_apollo_l34_lookupPortMap_get,
	.l34_wanRoutMode_set = dal_apollo_l34_wanRoutMode_set,
	.l34_wanRoutMode_get = dal_apollo_l34_wanRoutMode_get,
	.l34_arpTrfIndicator_get = dal_apollo_l34_arpTrfIndicator_get,
	.l34_naptTrfIndicator_get = dal_apollo_l34_naptTrfIndicator_get,
	.l34_pppTrfIndicator_get = dal_apollo_l34_pppTrfIndicator_get,
	.l34_neighTrfIndicator_get = (int32 (*)(uint32 index, rtk_enable_t *pNeighIndicator))dal_common_unavail,
	.l34_hsdState_set = dal_apollo_l34_hsdState_set,
	.l34_hsdState_get = dal_apollo_l34_hsdState_get,
	.l34_hwL4TrfWrkTbl_set = dal_apollo_l34_hwL4TrfWrkTbl_set,
	.l34_hwL4TrfWrkTbl_get = dal_apollo_l34_hwL4TrfWrkTbl_get,
	.l34_l4TrfTb_get = dal_apollo_l34_l4TrfTb_get,
	.l34_hwL4TrfWrkTbl_Clear = dal_apollo_l34_hwL4TrfWrkTbl_Clear,
	.l34_hwArpTrfWrkTbl_set = dal_apollo_l34_hwArpTrfWrkTbl_set,
	.l34_hwArpTrfWrkTbl_get = dal_apollo_l34_hwArpTrfWrkTbl_get,
	.l34_arpTrfTb_get = dal_apollo_l34_arpTrfTb_get,
	.l34_hwArpTrfWrkTbl_Clear = dal_apollo_l34_hwArpTrfWrkTbl_Clear,
	.l34_naptTrfIndicator_get_all = dal_apollo_l34_naptTrfIndicator_get_all,
	.l34_arpTrfIndicator_get_all = dal_apollo_l34_arpTrfIndicator_get_all,
	.l34_dsliteInfTable_set = NULL,
	.l34_dsliteInfTable_get = NULL,
	.l34_dsliteMcTable_set = NULL,
	.l34_dsliteMcTable_get = NULL,
	.l34_dsliteState_set = NULL,
	.l34_dsliteState_get = NULL,
	.l34_dsliteMcastUnmatchAct_set = NULL,
	.l34_dsliteMcastUnmatchAct_get = NULL,
	.l34_dsliteDsUnmatchAct_set = NULL,
	.l34_dsliteDsUnmatchAct_get = NULL,
	.l34_dsliteNextHeaderAct_set = NULL,
	.l34_dsliteNextHeaderAct_get = NULL,
	.l34_dsliteIpv6FragAct_set = NULL,
	.l34_dsliteIpv6FragAct_get = NULL,
	.l34_dsliteIpv4FragAct_set = NULL,
	.l34_dsliteIpv4FragAct_get = NULL,
    
    /* STP */
    .stp_init = dal_apollo_stp_init,
    .stp_mstpState_get = dal_apollo_stp_mstpState_get,
    .stp_mstpState_set = dal_apollo_stp_mstpState_set,

    /*PON MAC*/
    .ponmac_init = dal_apollo_ponmac_init,
    .ponmac_queue_add = dal_apollo_ponmac_queue_add,
    .ponmac_queue_get = dal_apollo_ponmac_queue_get,
    .ponmac_queue_del = dal_apollo_ponmac_queue_del,
    .ponmac_flow2Queue_set = dal_apollo_ponmac_flow2Queue_set,
    .ponmac_flow2Queue_get = dal_apollo_ponmac_flow2Queue_get,
    .ponmac_mode_get = dal_apollo_ponmac_mode_get,
    .ponmac_mode_set = dal_apollo_ponmac_mode_set,
    .ponmac_bwThreshold_set = NULL,
    .ponmac_bwThreshold_get = NULL,


    /* GPON */
#if CONFIG_GPON_VERSION < 2
    .gpon_driver_initialize                     = dal_apollo_gpon_driver_initialize,
    .gpon_driver_deInitialize                   = dal_apollo_gpon_driver_deInitialize,
    .gpon_device_initialize                     = dal_apollo_gpon_device_initialize,
    .gpon_device_deInitialize                   = dal_apollo_gpon_device_deInitialize,
    .gpon_eventHandler_stateChange_reg          = dal_apollo_gpon_eventHandler_stateChange_reg,
    .gpon_eventHandler_dsFecChange_reg          = dal_apollo_gpon_eventHandler_dsFecChange_reg,
    .gpon_eventHandler_usFecChange_reg          = dal_apollo_gpon_eventHandler_usFecChange_reg,
    .gpon_eventHandler_usPloamUrgEmpty_reg      = dal_apollo_gpon_eventHandler_usPloamUrgEmpty_reg,
    .gpon_eventHandler_usPloamNrmEmpty_reg      = dal_apollo_gpon_eventHandler_usPloamNrmEmpty_reg,
    .gpon_eventHandler_ploam_reg                = dal_apollo_gpon_eventHandler_ploam_reg,
    .gpon_eventHandler_omci_reg                 = dal_apollo_gpon_eventHandler_omci_reg,
    .gpon_callback_queryAesKey_reg              = dal_apollo_gpon_callback_queryAesKey_reg,
    .gpon_eventHandler_alarm_reg                = dal_apollo_gpon_eventHandler_alarm_reg,
    .gpon_serialNumber_set                      = dal_apollo_gpon_serialNumber_set,
    .gpon_serialNumber_get                      = dal_apollo_gpon_serialNumber_get,
    .gpon_password_set                          = dal_apollo_gpon_password_set,
    .gpon_password_get                          = dal_apollo_gpon_password_get,
    .gpon_parameter_set                         = dal_apollo_gpon_parameter_set,
    .gpon_parameter_get                         = dal_apollo_gpon_parameter_get,
    .gpon_activate                              = dal_apollo_gpon_activate,
    .gpon_deActivate                            = dal_apollo_gpon_deActivate,
    .gpon_ponStatus_get                         = dal_apollo_gpon_ponStatus_get,
    .gpon_isr_entry                             = dal_apollo_gpon_isr_entry,
    .gpon_tcont_create                          = dal_apollo_gpon_tcont_create,
    .gpon_tcont_destroy                         = dal_apollo_gpon_tcont_destroy,
    .gpon_tcont_get                             = dal_apollo_gpon_tcont_get,
    .gpon_dsFlow_set                            = dal_apollo_gpon_dsFlow_set,
    .gpon_dsFlow_get                            = dal_apollo_gpon_dsFlow_get,
    .gpon_usFlow_set                            = dal_apollo_gpon_usFlow_set,
    .gpon_usFlow_get                            = dal_apollo_gpon_usFlow_get,
    .gpon_ploam_send                            = dal_apollo_gpon_ploam_send,
    .gpon_broadcastPass_set                     = dal_apollo_gpon_broadcastPass_set,
    .gpon_broadcastPass_get                     = dal_apollo_gpon_broadcastPass_get,
    .gpon_nonMcastPass_set                      = dal_apollo_gpon_nonMcastPass_set,
    .gpon_nonMcastPass_get                      = dal_apollo_gpon_nonMcastPass_get,
#if 0 /* wellknown addr is removed in GPON_MAC_SWIO_r1.1 */
    .gpon_wellKnownAddr_set                     = dal_apollo_gpon_wellKnownAddr_set,
    .gpon_wellKnownAddr_get                     = dal_apollo_gpon_wellKnownAddr_get,
#endif
    .gpon_multicastAddrCheck_set                = dal_apollo_gpon_multicastAddrCheck_set,
    .gpon_multicastAddrCheck_get                = dal_apollo_gpon_multicastAddrCheck_get,
    .gpon_macFilterMode_set                     = dal_apollo_gpon_macFilterMode_set,
    .gpon_macFilterMode_get                     = dal_apollo_gpon_macFilterMode_get,
    .gpon_mcForceMode_set                       = dal_apollo_gpon_mcForceMode_set,
    .gpon_mcForceMode_get                       = dal_apollo_gpon_mcForceMode_get,
    .gpon_macEntry_add                          = dal_apollo_gpon_macEntry_add,
    .gpon_macEntry_del                          = dal_apollo_gpon_macEntry_del,
    .gpon_macEntry_get                          = dal_apollo_gpon_macEntry_get,
    .gpon_rdi_set                               = dal_apollo_gpon_rdi_set,
    .gpon_rdi_get                               = dal_apollo_gpon_rdi_get,
    .gpon_powerLevel_set                        = dal_apollo_gpon_powerLevel_set,
    .gpon_powerLevel_get                        = dal_apollo_gpon_powerLevel_get,
    .gpon_alarmStatus_get                       = dal_apollo_gpon_alarmStatus_get,
    .gpon_globalCounter_get                     = dal_apollo_gpon_globalCounter_get ,
    .gpon_tcontCounter_get                      = dal_apollo_gpon_tcontCounter_get ,
    .gpon_flowCounter_get                       = dal_apollo_gpon_flowCounter_get ,
    .gpon_version_get                           = dal_apollo_gpon_version_get,
#if 0 /* gemloop is removed in GPON_MAC_SWIO_v1.1 */
    .gpon_gemLoop_set                           = dal_apollo_gpon_gemLoop_set,
    .gpon_gemLoop_get                           = dal_apollo_gpon_gemLoop_get,
#endif
    .gpon_txForceLaser_set                      = dal_apollo_gpon_txForceLaser_set,
    .gpon_txForceLaser_get                      = dal_apollo_gpon_txForceLaser_get,
    .gpon_txForceIdle_set                       = dal_apollo_gpon_txForceIdle_set,
    .gpon_txForceIdle_get                       = dal_apollo_gpon_txForceIdle_get,
#if 0
    .gpon_txForcePRBS_set                       = dal_apollo_gpon_txForcePRBS_set,
    .gpon_txForcePRBS_get                       = dal_apollo_gpon_txForcePRBS_get,
#endif
    .gpon_dsFecSts_get                          = dal_apollo_gpon_dsFecSts_get,
    .gpon_version_show                          = dal_apollo_gpon_version_show,
    .gpon_devInfo_show                          = dal_apollo_gpon_devInfo_show,
    .gpon_gtc_show                              = dal_apollo_gpon_gtc_show,
    .gpon_tcont_show                            = dal_apollo_gpon_tcont_show,
    .gpon_dsFlow_show                           = dal_apollo_gpon_dsFlow_show,
    .gpon_usFlow_show                           = dal_apollo_gpon_usFlow_show,
    .gpon_macTable_show                         = dal_apollo_gpon_macTable_show,
    .gpon_globalCounter_show                    = dal_apollo_gpon_globalCounter_show,
    .gpon_tcontCounter_show                     = dal_apollo_gpon_tcontCounter_show,
    .gpon_flowCounter_show                      = dal_apollo_gpon_flowCounter_show,
#if defined(OLD_FPGA_DEFINED)
    .gpon_pktGen_cfg_set                        = dal_apollo_gpon_pktGen_cfg_set,
    .gpon_pktGen_buf_set                        = dal_apollo_gpon_pktGen_buf_set,
#endif
    .gpon_omci_tx                               = dal_apollo_gpon_omci_tx,
    .gpon_omci_rx                               = dal_apollo_gpon_omci_rx,
    .gpon_autoTcont_set                         = dal_apollo_gpon_auto_tcont_set,
    .gpon_autoTcont_get                         = dal_apollo_gpon_auto_tcont_get,
    .gpon_autoBoh_set                           = dal_apollo_gpon_auto_boh_set,
    .gpon_autoBoh_get                           = dal_apollo_gpon_auto_boh_get,
    .gpon_eqdOffset_set                         = dal_apollo_gpon_eqd_offset_set,
    .gpon_eqdOffset_get                         = dal_apollo_gpon_eqd_offset_get,
    .gpon_autoDisTx_set                         = (int32 (*)(rtk_enable_t))dal_common_unavail,
#else
#endif
    /* Classification */
    .classify_init                              = dal_apollo_classify_init,
    .classify_cfgEntry_add                      = dal_apollo_classify_cfgEntry_add,
    .classify_cfgEntry_get                      = dal_apollo_classify_cfgEntry_get,
    .classify_cfgEntry_del                      = dal_apollo_classify_cfgEntry_del,
    .classify_field_add                         = dal_apollo_classify_field_add,
    .classify_unmatchAction_set                 = dal_apollo_classify_unmatchAction_set,
    .classify_unmatchAction_get                 = dal_apollo_classify_unmatchAction_get,

    .classify_portRange_set                     = (int32 (*)(rtk_classify_rangeCheck_l4Port_t *))dal_common_unavail,
    .classify_portRange_get                     = (int32 (*)(rtk_classify_rangeCheck_l4Port_t *))dal_common_unavail,
    .classify_ipRange_set                       = (int32 (*)(rtk_classify_rangeCheck_ip_t *))dal_common_unavail,
    .classify_ipRange_get                       = (int32 (*)(rtk_classify_rangeCheck_ip_t *))dal_common_unavail,
    .classify_cf_sel_set                        = (int32 (*)(rtk_port_t, rtk_classify_cf_sel_t))dal_common_unavail,
    .classify_cf_sel_get                        = (int32 (*)(rtk_port_t, rtk_classify_cf_sel_t *))dal_common_unavail,
    .classify_cfPri2Dscp_set                    = (int32 (*)(rtk_pri_t, rtk_dscp_t))dal_common_unavail,
    .classify_cfPri2Dscp_get                    = (int32 (*)(rtk_pri_t, rtk_dscp_t *))dal_common_unavail,
    .classify_permit_sel_set                    = (int32 (*)( rtk_classify_permit_sel_t))dal_common_unavail,
    .classify_permit_sel_get                    = (int32 (*)( rtk_classify_permit_sel_t*))dal_common_unavail,
    .classify_us1pRemarkPrior_set               = (int32 (*)( rtk_classify_us_1premark_prior_t))dal_common_unavail,
    .classify_us1pRemarkPrior_get               = (int32 (*)( rtk_classify_us_1premark_prior_t*))dal_common_unavail,
    .classify_templateCfgPattern0_set           = NULL,
    .classify_templateCfgPattern0_get           = NULL,
    .classify_entryNumPattern1_set              = NULL,
    .classify_entryNumPattern1_get              = NULL,
    .classify_defaultWanIf_set                  = NULL,
    .classify_defaultWanIf_get                  = NULL,
    
    /* statistics */
    .stat_init                                  = dal_apollo_stat_init,
    .stat_global_reset                          = dal_apollo_stat_global_reset,
    .stat_port_reset                            = dal_apollo_stat_port_reset,
    .stat_log_reset                             = dal_apollo_stat_log_reset,
    .stat_rst_cnt_value_set                     = dal_apollo_stat_rst_cnt_value_set,
    .stat_rst_cnt_value_get                     = dal_apollo_stat_rst_cnt_value_get,
    .stat_global_get                            = dal_apollo_stat_global_get,
    .stat_global_getAll                         = dal_apollo_stat_global_getAll,
    .stat_port_get                              = dal_apollo_stat_port_get,
    .stat_port_getAll                           = dal_apollo_stat_port_getAll,
    .stat_log_get                               = dal_apollo_stat_log_get,
    .stat_log_ctrl_set                          = dal_apollo_stat_log_ctrl_set,
    .stat_log_ctrl_get                          = dal_apollo_stat_log_ctrl_get,
    .stat_mib_cnt_mode_get                      = dal_apollo_stat_mib_cnt_mode_get,
    .stat_mib_cnt_mode_set                      = dal_apollo_stat_mib_cnt_mode_set,
    .stat_mib_latch_timer_get                   = dal_apollo_stat_mib_latch_timer_get,
    .stat_mib_latch_timer_set                   = dal_apollo_stat_mib_latch_timer_set,
    .stat_mib_sync_mode_get                     = dal_apollo_stat_mib_sync_mode_get,
    .stat_mib_sync_mode_set                     = dal_apollo_stat_mib_sync_mode_set,
    .stat_mib_count_tag_length_get              = dal_apollo_stat_mib_count_tag_length_get,
    .stat_mib_count_tag_length_set              = dal_apollo_stat_mib_count_tag_length_set,
    .stat_pktInfo_get                           = dal_apollo_stat_pktInfo_get,

    /*SVLAN*/
    .svlan_init = dal_apollo_svlan_init,
    .svlan_create = dal_apollo_svlan_create,
    .svlan_destroy = dal_apollo_svlan_destroy,
    .svlan_portSvid_get = dal_apollo_svlan_portSvid_get,
    .svlan_portSvid_set = dal_apollo_svlan_portSvid_set,
    .svlan_servicePort_get = dal_apollo_svlan_servicePort_get,
    .svlan_servicePort_set = dal_apollo_svlan_servicePort_set,
    .svlan_memberPort_set = dal_apollo_svlan_memberPort_set,
    .svlan_memberPort_get = dal_apollo_svlan_memberPort_get,
    .svlan_tpidEntry_get = dal_apollo_svlan_tpidEntry_get,
    .svlan_tpidEntry_set = dal_apollo_svlan_tpidEntry_set,
    .svlan_priorityRef_set = dal_apollo_svlan_priorityRef_set,
    .svlan_priorityRef_get = dal_apollo_svlan_priorityRef_get,
    .svlan_memberPortEntry_set = dal_apollo_svlan_memberPortEntry_set,
    .svlan_memberPortEntry_get = dal_apollo_svlan_memberPortEntry_get,
    .svlan_ipmc2s_add = dal_apollo_svlan_ipmc2s_add,
    .svlan_ipmc2s_del = dal_apollo_svlan_ipmc2s_del,
    .svlan_ipmc2s_get = dal_apollo_svlan_ipmc2s_get,
    .svlan_l2mc2s_add = dal_apollo_svlan_l2mc2s_add,
    .svlan_l2mc2s_del = dal_apollo_svlan_l2mc2s_del,
    .svlan_l2mc2s_get = dal_apollo_svlan_l2mc2s_get,
    .svlan_sp2c_add = dal_apollo_svlan_sp2c_add,
    .svlan_sp2c_get = dal_apollo_svlan_sp2c_get,
    .svlan_sp2c_del = dal_apollo_svlan_sp2c_del,
    .svlan_dmacVidSelState_set = dal_apollo_svlan_dmacVidSelState_set,
    .svlan_dmacVidSelState_get = dal_apollo_svlan_dmacVidSelState_get,
    .svlan_unmatchAction_set = dal_apollo_svlan_unmatchAction_set,
    .svlan_unmatchAction_get = dal_apollo_svlan_unmatchAction_get,
    .svlan_untagAction_set = dal_apollo_svlan_untagAction_set,
    .svlan_untagAction_get = dal_apollo_svlan_untagAction_get,
    .svlan_c2s_add = dal_apollo_svlan_c2s_add,
    .svlan_c2s_del = dal_apollo_svlan_c2s_del,
    .svlan_c2s_get = dal_apollo_svlan_c2s_get,
    .svlan_trapPri_get = dal_apollo_svlan_trapPri_get,
    .svlan_trapPri_set = dal_apollo_svlan_trapPri_set,
    .svlan_deiKeepState_get = dal_apollo_svlan_deiKeepState_get,
    .svlan_deiKeepState_set = dal_apollo_svlan_deiKeepState_set,
    .svlan_lookupType_get = (int32 (*)(rtk_svlan_lookupType_t *))dal_common_unavail,
    .svlan_lookupType_set = (int32 (*)(rtk_svlan_lookupType_t))dal_common_unavail,
    .svlan_sp2cUnmatchCtagging_get = (int32 (*)(rtk_enable_t *))dal_common_unavail,
    .svlan_sp2cUnmatchCtagging_set = (int32 (*)(rtk_enable_t))dal_common_unavail,
	.svlan_priority_get = dal_apollo_svlan_priority_get,
	.svlan_priority_set = dal_apollo_svlan_priority_set,
	.svlan_fid_get = dal_apollo_svlan_fid_get,
	.svlan_fid_set = dal_apollo_svlan_fid_set,
	.svlan_fidEnable_get = dal_apollo_svlan_fidEnable_get,
	.svlan_fidEnable_set = dal_apollo_svlan_fidEnable_set,
	.svlan_enhancedFid_get = dal_apollo_svlan_enhancedFid_get,
	.svlan_enhancedFid_set = dal_apollo_svlan_enhancedFid_set,
	.svlan_enhancedFidEnable_get = dal_apollo_svlan_enhancedFidEnable_get,
	.svlan_enhancedFidEnable_set = dal_apollo_svlan_enhancedFidEnable_set,
    .svlan_dmacVidSelForcedState_set = (int32 (*)(rtk_enable_t))dal_common_unavail,
    .svlan_dmacVidSelForcedState_get = (int32 (*)(rtk_enable_t *))dal_common_unavail,
    .svlan_svlanFunctionEnable_get = NULL,
    .svlan_svlanFunctionEnable_set = NULL,

    /*acl*/
    .acl_init = dal_apollo_acl_init,
    .acl_template_set = dal_apollo_acl_template_set,
    .acl_template_get = dal_apollo_acl_template_get,
    .acl_fieldSelect_set = dal_apollo_acl_fieldSelect_set,
    .acl_fieldSelect_get = dal_apollo_acl_fieldSelect_get,
    .acl_igrRuleEntry_get = dal_apollo_acl_igrRuleEntry_get,
    .acl_igrRuleField_add = dal_apollo_acl_igrRuleField_add,
    .acl_igrRuleEntry_add = dal_apollo_acl_igrRuleEntry_add,
    .acl_igrRuleEntry_del = dal_apollo_acl_igrRuleEntry_del,
    .acl_igrRuleEntry_delAll = dal_apollo_acl_igrRuleEntry_delAll,
    .acl_igrUnmatchAction_set = dal_apollo_acl_igrUnmatchAction_set,
    .acl_igrUnmatchAction_get = dal_apollo_acl_igrUnmatchAction_get,
    .acl_igrState_set = dal_apollo_acl_igrState_set,
    .acl_igrState_get = dal_apollo_acl_igrState_get,
    .acl_ipRange_set = dal_apollo_acl_ipRange_set,
    .acl_ipRange_get = dal_apollo_acl_ipRange_get,
    .acl_vidRange_set = dal_apollo_acl_vidRange_set,
    .acl_vidRange_get = dal_apollo_acl_vidRange_get,
    .acl_portRange_set = dal_apollo_acl_portRange_set,
    .acl_portRange_get = dal_apollo_acl_portRange_get,
    .acl_packetLengthRange_set = dal_apollo_acl_packetLengthRange_set,
    .acl_packetLengthRange_get = dal_apollo_acl_packetLengthRange_get,
    .acl_igrRuleMode_set = dal_apollo_acl_igrRuleMode_set,
    .acl_igrRuleMode_get = dal_apollo_acl_igrRuleMode_get,
    .acl_igrPermitState_set = dal_apollo_acl_igrPermitState_set,
    .acl_igrPermitState_get = dal_apollo_acl_igrPermitState_get,

    /*sec*/
    .sec_init = dal_apollo_sec_init,
    .sec_portAttackPreventState_get = dal_apollo_sec_portAttackPreventState_get,
    .sec_portAttackPreventState_set = dal_apollo_sec_portAttackPreventState_set,
    .sec_attackPrevent_get = dal_apollo_sec_attackPrevent_get,
    .sec_attackPrevent_set = dal_apollo_sec_attackPrevent_set,
    .sec_attackFloodThresh_get = dal_apollo_sec_attackFloodThresh_get,
    .sec_attackFloodThresh_set = dal_apollo_sec_attackFloodThresh_set,

    /*rate*/
    .rate_init = dal_apollo_rate_init,
    .rate_portIgrBandwidthCtrlRate_get = dal_apollo_rate_portIgrBandwidthCtrlRate_get,
    .rate_portIgrBandwidthCtrlRate_set = dal_apollo_rate_portIgrBandwidthCtrlRate_set,
    .rate_portIgrBandwidthCtrlIncludeIfg_get = dal_apollo_rate_portIgrBandwidthCtrlIncludeIfg_get,
    .rate_portIgrBandwidthCtrlIncludeIfg_set = dal_apollo_rate_portIgrBandwidthCtrlIncludeIfg_set,
    .rate_portEgrBandwidthCtrlRate_get = dal_apollo_rate_portEgrBandwidthCtrlRate_get,
    .rate_portEgrBandwidthCtrlRate_set = dal_apollo_rate_portEgrBandwidthCtrlRate_set,
    .rate_egrBandwidthCtrlIncludeIfg_get = dal_apollo_rate_egrBandwidthCtrlIncludeIfg_get,
    .rate_egrBandwidthCtrlIncludeIfg_set = dal_apollo_rate_egrBandwidthCtrlIncludeIfg_set,
    .rate_portEgrBandwidthCtrlIncludeIfg_get = (int32 (*)(rtk_port_t, rtk_enable_t *))dal_common_unavail,
    .rate_portEgrBandwidthCtrlIncludeIfg_set = (int32 (*)(rtk_port_t, rtk_enable_t ))dal_common_unavail,
    .rate_egrQueueBwCtrlEnable_get = dal_apollo_rate_egrQueueBwCtrlEnable_get,
    .rate_egrQueueBwCtrlEnable_set = dal_apollo_rate_egrQueueBwCtrlEnable_set,
    .rate_egrQueueBwCtrlMeterIdx_get = dal_apollo_rate_egrQueueBwCtrlMeterIdx_get,
    .rate_egrQueueBwCtrlMeterIdx_set = dal_apollo_rate_egrQueueBwCtrlMeterIdx_set,
    .rate_stormControlMeterIdx_get = dal_apollo_rate_stormControlMeterIdx_get,
    .rate_stormControlMeterIdx_set = dal_apollo_rate_stormControlMeterIdx_set,
    .rate_stormControlPortEnable_get = dal_apollo_rate_stormControlPortEnable_get,
    .rate_stormControlPortEnable_set = dal_apollo_rate_stormControlPortEnable_set,
    .rate_stormControlEnable_get = dal_apollo_rate_stormControlEnable_get,
    .rate_stormControlEnable_set = dal_apollo_rate_stormControlEnable_set,
    .rate_stormBypass_set = dal_apollo_rate_stormBypass_set,
    .rate_stormBypass_get = dal_apollo_rate_stormBypass_get,
    .rate_shareMeter_set = dal_apollo_rate_shareMeter_set,
    .rate_shareMeter_get = dal_apollo_rate_shareMeter_get,
    .rate_shareMeterBucket_set = dal_apollo_rate_shareMeterBucket_set,
    .rate_shareMeterBucket_get = dal_apollo_rate_shareMeterBucket_get,
    .rate_shareMeterExceed_get = dal_apollo_rate_shareMeterExceed_get,
    .rate_shareMeterExceed_clear = dal_apollo_rate_shareMeterExceed_clear,

    /* Mirror */
    .mirror_init = dal_apollo_mirror_init,
    .mirror_portBased_set = dal_apollo_mirror_portBased_set,
    .mirror_portBased_get = dal_apollo_mirror_portBased_get,
    .mirror_portIso_set = dal_apollo_mirror_portIso_set,
    .mirror_portIso_get = dal_apollo_mirror_portIso_get,

    /* Trunk */
    .trunk_init = dal_apollo_trunk_init,
    .trunk_distributionAlgorithm_get = dal_apollo_trunk_distributionAlgorithm_get,
    .trunk_distributionAlgorithm_set = dal_apollo_trunk_distributionAlgorithm_set,
    .trunk_port_get = dal_apollo_trunk_port_get,
    .trunk_port_set = dal_apollo_trunk_port_set,
    .trunk_hashMappingTable_get = dal_apollo_trunk_hashMappingTable_get,
    .trunk_hashMappingTable_set = dal_apollo_trunk_hashMappingTable_set,
    .trunk_mode_get = dal_apollo_trunk_mode_get,
    .trunk_mode_set = dal_apollo_trunk_mode_set,
    .trunk_trafficSeparate_get = dal_apollo_trunk_trafficSeparate_get,
    .trunk_trafficSeparate_set = dal_apollo_trunk_trafficSeparate_set,
    .trunk_portQueueEmpty_get = dal_apollo_trunk_portQueueEmpty_get,
    .trunk_trafficPause_get = dal_apollo_trunk_trafficPause_get,
    .trunk_trafficPause_set = dal_apollo_trunk_trafficPause_set,

    /*led*/
    .led_init = dal_apollo_led_init,
    .led_operation_get = dal_apollo_led_operation_get,
    .led_operation_set = dal_apollo_led_operation_set,
    .led_serialMode_get = dal_apollo_led_serialMode_get,
    .led_serialMode_set = dal_apollo_led_serialMode_set,
    .led_blinkRate_get = dal_apollo_led_blinkRate_get,
    .led_blinkRate_set = dal_apollo_led_blinkRate_set,
    .led_config_set = dal_apollo_led_config_set,
    .led_config_get = dal_apollo_led_config_get,
    .led_modeForce_get = dal_apollo_led_modeForce_get,
    .led_modeForce_set = dal_apollo_led_modeForce_set,
    .led_parallelEnable_get = dal_apollo_led_parallelEnable_get,
    .led_parallelEnable_set = dal_apollo_led_parallelEnable_set,

    /*dot1x*/
    .dot1x_init = dal_apollo_dot1x_init,
    .dot1x_unauthPacketOper_get = dal_apollo_dot1x_unauthPacketOper_get,
    .dot1x_unauthPacketOper_set = dal_apollo_dot1x_unauthPacketOper_set,
    .dot1x_portBasedEnable_get = dal_apollo_dot1x_portBasedEnable_get,
    .dot1x_portBasedEnable_set = dal_apollo_dot1x_portBasedEnable_set,
    .dot1x_portBasedAuthStatus_get = dal_apollo_dot1x_portBasedAuthStatus_get,
    .dot1x_portBasedAuthStatus_set = dal_apollo_dot1x_portBasedAuthStatus_set,
    .dot1x_portBasedDirection_get = dal_apollo_dot1x_portBasedDirection_get,
    .dot1x_portBasedDirection_set = dal_apollo_dot1x_portBasedDirection_set,
    .dot1x_macBasedEnable_get = dal_apollo_dot1x_macBasedEnable_get,
    .dot1x_macBasedEnable_set = dal_apollo_dot1x_macBasedEnable_set,
    .dot1x_macBasedDirection_get = dal_apollo_dot1x_macBasedDirection_get,
    .dot1x_macBasedDirection_set = dal_apollo_dot1x_macBasedDirection_set,
    .dot1x_guestVlan_get = dal_apollo_dot1x_guestVlan_get,
    .dot1x_guestVlan_set = dal_apollo_dot1x_guestVlan_set,
    .dot1x_guestVlanBehavior_get = dal_apollo_dot1x_guestVlanBehavior_get,
    .dot1x_guestVlanBehavior_set = dal_apollo_dot1x_guestVlanBehavior_set,
    .dot1x_trapPri_get = dal_apollo_dot1x_trapPri_get,
    .dot1x_trapPri_set = dal_apollo_dot1x_trapPri_set,

    /*oam*/
    .oam_init = dal_apollo_oam_init,
    .oam_parserAction_set = dal_apollo_oam_parserAction_set,
    .oam_parserAction_get = dal_apollo_oam_parserAction_get,
    .oam_multiplexerAction_set = dal_apollo_oam_multiplexerAction_set,
    .oam_multiplexerAction_get = dal_apollo_oam_multiplexerAction_get,

     /*interrupt*/
    .intr_init = dal_apollo_intr_init,
    .intr_polarity_set = dal_apollo_intr_polarity_set,
    .intr_polarity_get = dal_apollo_intr_polarity_get,
    .intr_imr_set = dal_apollo_intr_imr_set,
    .intr_imr_get = dal_apollo_intr_imr_get,
    .intr_ims_get = dal_apollo_intr_ims_get,
    .intr_ims_clear = dal_apollo_intr_ims_clear,
    .intr_speedChangeStatus_get = dal_apollo_intr_speedChangeStatus_get,
    .intr_speedChangeStatus_clear = dal_apollo_intr_speedChangeStatus_clear,
    .intr_linkupStatus_get = dal_apollo_intr_linkupStatus_get,
    .intr_linkupStatus_clear = dal_apollo_intr_linkupStatus_clear,
    .intr_linkdownStatus_get = dal_apollo_intr_linkdownStatus_get,
    .intr_linkdownStatus_clear = dal_apollo_intr_linkdownStatus_clear,
    .intr_gphyStatus_get = dal_apollo_intr_gphyStatus_get,
    .intr_gphyStatus_clear = dal_apollo_intr_gphyStatus_clear,
    .intr_imr_restore = dal_apollo_intr_imr_restore,

    /* RLDP and RLPP */
    .rldp_init = dal_apollo_rldp_init,
    .rldp_config_set = (int32 (*)(rtk_rldp_config_t *))dal_common_unavail,
    .rldp_config_get = (int32 (*)(rtk_rldp_config_t *))dal_common_unavail,
    .rldp_portConfig_set = (int32 (*)(rtk_port_t, rtk_rldp_portConfig_t *))dal_common_unavail,
    .rldp_portConfig_get = (int32 (*)(rtk_port_t, rtk_rldp_portConfig_t *))dal_common_unavail,
    .rldp_status_get = (int32 (*)(rtk_rldp_status_t *))dal_common_unavail,
    .rldp_portStatus_get = (int32 (*)(rtk_port_t, rtk_rldp_portStatus_t *))dal_common_unavail,
    .rldp_portStatus_clear = (int32 (*)(rtk_port_t, rtk_rldp_portStatus_t *))dal_common_unavail,
    .rlpp_init = (int32 (*)(void))dal_common_unavail,
    .rlpp_trapType_set = (int32 (*)(rtk_rlpp_trapType_t))dal_common_unavail,
    .rlpp_trapType_get = (int32 (*)(rtk_rlpp_trapType_t *))dal_common_unavail,

    /*cpu*/
    .cpu_init = dal_apollo_cpu_init,
    .cpu_awarePortMask_set = dal_apollo_cpu_awarePortMask_set,
    .cpu_awarePortMask_get = dal_apollo_cpu_awarePortMask_get,
    .cpu_tagFormat_set = dal_apollo_cpu_tagFormat_set,
    .cpu_tagFormat_get = dal_apollo_cpu_tagFormat_get,
    .cpu_trapInsertTag_set = dal_apollo_cpu_trapInsertTag_set,
    .cpu_trapInsertTag_get = dal_apollo_cpu_trapInsertTag_get,
    .cpu_tagAware_set = NULL,
    .cpu_tagAware_get = NULL,

    /*gpio function*/
    .gpio_init = dal_apollo_gpio_init,
    .gpio_state_set = dal_apollo_gpio_state_set,
    .gpio_state_get = dal_apollo_gpio_state_get,
    .gpio_mode_set = dal_apollo_gpio_mode_set,
    .gpio_mode_get = dal_apollo_gpio_mode_get,
    .gpio_databit_get = dal_apollo_gpio_databit_get,
    .gpio_databit_set = dal_apollo_gpio_databit_set,
    .gpio_intr_get = dal_apollo_gpio_intr_get,
    .gpio_intr_set = dal_apollo_gpio_intr_set,


    /* I2C */
    .i2c_init = dal_apollo_i2c_init,
    .i2c_enable_set = dal_apollo_i2c_enable_set,
    .i2c_enable_get = dal_apollo_i2c_enable_get,
    .i2c_width_set = dal_apollo_i2c_width_set,
    .i2c_width_get = dal_apollo_i2c_width_get,
    .i2c_write = dal_apollo_i2c_write,
    .i2c_read = dal_apollo_i2c_read,

    /* PTP Function */
    .time_portTransparentEnable_set = (int32 (*)(rtk_port_t, rtk_enable_t))dal_common_unavail,
    .time_portTransparentEnable_get = (int32 (*)(rtk_port_t, rtk_enable_t *))dal_common_unavail,
    .time_init = dal_apollo_time_init,
    .time_portPtpEnable_get = (int32 (*)(rtk_port_t, rtk_enable_t *))dal_common_unavail,
    .time_portPtpEnable_set = (int32 (*)(rtk_port_t, rtk_enable_t))dal_common_unavail,
    .time_curTime_get = (int32 (*)(rtk_time_timeStamp_t *))dal_common_unavail,
    .time_curTime_latch = (int32 (*)(void))dal_common_unavail,
    .time_refTime_get = (int32 (*)(uint32 *, rtk_time_timeStamp_t *))dal_common_unavail,
    .time_refTime_set = (int32 (*)(uint32, rtk_time_timeStamp_t))dal_common_unavail,
    .time_frequency_set = (int32 (*)(uint32))dal_common_unavail,
    .time_frequency_get = (int32 (*)(uint32 *))dal_common_unavail,
    .time_ptpIgrMsgAction_set = (int32 (*)(rtk_time_ptpMsgType_t, rtk_time_ptpIgrMsg_action_t))dal_common_unavail,
    .time_ptpIgrMsgAction_get = (int32 (*)(rtk_time_ptpMsgType_t, rtk_time_ptpIgrMsg_action_t *))dal_common_unavail,
    .time_ptpEgrMsgAction_set = (int32 (*)(rtk_time_ptpMsgType_t, rtk_time_ptpEgrMsg_action_t))dal_common_unavail,
    .time_ptpEgrMsgAction_get = (int32 (*)(rtk_time_ptpMsgType_t, rtk_time_ptpEgrMsg_action_t *))dal_common_unavail,
    .time_meanPathDelay_set = (int32 (*)(uint32))dal_common_unavail,
    .time_meanPathDelay_get = (int32 (*)(uint32 *))dal_common_unavail,
    .time_rxTime_set = (int32 (*)(rtk_time_timeStamp_t))dal_common_unavail,
    .time_rxTime_get = (int32 (*)(rtk_time_timeStamp_t *))dal_common_unavail,
};

/*
 * Macro Declaration
 */

/*
 * Function Declaration
 */


/* Module Name    :  */

/* Function Name:
 *      dal_apollo_init
 * Description:
 *      Initilize DAL of smart switch
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - initialize success
 *      RT_ERR_FAILED - initialize fail
 * Note:
 *      RTK must call this function before do other kind of action.
 */
int32 dal_apollo_init(void)
{
    return RT_ERR_OK;
} /* end of dal_apollo_init */

/* Function Name:
 *      dal_apollo_mapper_get
 * Description:
 *      Get DAL mapper function
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      dal_mapper_t *     - mapper pointer
 * Note:
 */
dal_mapper_t *dal_apollo_mapper_get(void)
{
    return &dal_apollo_mapper;
} /* end of dal_apollo_mapper_get */

