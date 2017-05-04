/*
 * Copyright (C) 2014 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * Purpose : Definition of OMCI internal APIs
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) OMCI internal APIs
 */

#ifndef _OMCI_INTERNAL_API_H_
#define _OMCI_INTERNAL_API_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "omci_msgq.h"
#include "mib_table.h"
#include "mcast_wrapper.h"


// Task associates
GOS_ERROR_CODE omci_pm_task_init(void);
GOS_ERROR_CODE omci_tm_task_init(void);
GOS_ERROR_CODE OMCI_InitStateMachines(void);

void omci_pm_sync_request_updater(BOOL bWrite, BOOL *bSyncState);
void omci_pm_pause_play_request_updater(BOOL bWrite, BOOL *bPausePlayState);

void omci_tm_pause_play_request_updater(BOOL bWrite, BOOL *bPausePlayState);

// MIB associates
GOS_ERROR_CODE mib_AddEntry(MIB_TABLE_INDEX tableIndex, void *pObjectRow);
GOS_ERROR_CODE mib_DeleteEntry(MIB_TABLE_INDEX tableIndex, void *pObjectRow);
BOOL mib_FindEntry(MIB_TABLE_INDEX tableIndex, void *pObjectRow, void *ppRetRow);
BOOL mib_FindPmCurrentBin(MIB_TABLE_INDEX tableIndex, void *pObjectRow, void *ppRetRow);
BOOL mib_FindNextEntry(MIB_TABLE_INDEX tableIndex, void *pObjectRow, void *ppRetRow);
MIB_ENTRY_T* mib_FindEntryByInstanceID(MIB_TABLE_INDEX tableIndex, omci_me_instance_t instanceID);

MIB_TABLE_T *mib_GetTablePtr(MIB_TABLE_INDEX tableIndex);
void* mib_GetAttrPtr(MIB_TABLE_INDEX tableIndex, void* pRow, MIB_ATTR_INDEX attrIndex);
GOS_ERROR_CODE mib_alarm_table_add(MIB_TABLE_INDEX tableIndex, void *pRow);
GOS_ERROR_CODE mib_alarm_table_del(MIB_TABLE_INDEX tableIndex, void *pRow);
GOS_ERROR_CODE mib_alarm_table_get(MIB_TABLE_INDEX tableIndex,
                                    omci_me_instance_t instanceID,
                                    mib_alarm_table_t *pAlarmTable);
GOS_ERROR_CODE mib_alarm_table_set(MIB_TABLE_INDEX tableIndex,
                                    omci_me_instance_t instanceID,
                                    mib_alarm_table_t *pAlarmTable);
GOS_ERROR_CODE mib_alarm_table_update(mib_alarm_table_t     *pAlarmTable,
                                        omci_alm_data_t     *pAlarmData,
                                        BOOL                *pIsUpdated);

// other internals
extern int omci_sw_dl_uboot_env_set(char *name, char *buffer);
extern char* omci_sw_dl_uboot_env_get(char *name);

extern BOOL omci_checkSvlanTpid(omci_vlan_rule_t *pEntry);
/*
 * Define OMCI MIB API
*/
extern GOS_ERROR_CODE OMCI_ResetMib(void);
extern GOS_ERROR_CODE OMCI_InitMib(void);

/*
 * Define OMCI Message API
*/
extern GOS_ERROR_CODE OMCI_ResponseMsg(omci_msg_norm_baseline_t* pNorOmciMsg, omci_msg_response_t result, BOOL clear);
extern GOS_ERROR_CODE OMCI_ResetHistoryRspMsg(void);
extern GOS_ERROR_CODE OMCI_ResetLastRspMsgIndex(void);
extern GOS_ERROR_CODE OMCI_HandleMsg(void* pData, OMCI_MSG_TYPE type, OMCI_MSG_PRI pri, OMCI_APPL_ID srcAppl);
extern GOS_ERROR_CODE OMCI_NormalizeMsg(omci_msg_baseline_fmt_t* pOmciMsg, omci_msg_norm_baseline_t* pNorOmciMsg);
extern GOS_ERROR_CODE OMCI_SendIndicationToOlt(omci_msg_norm_baseline_t* pNorOmciMsg, UINT16 tr);
extern UINT32 omci_CalcCRC32(UINT32 accum, const UINT8* pBuf, UINT32 size);
GOS_ERROR_CODE omci_alarm_notify_xmit(MIB_TABLE_INDEX       tableIndex,
                                        omci_me_instance_t  instanceID,
                                        mib_alarm_table_t   *pAlarmTable);
GOS_ERROR_CODE omci_ext_alarm_dispatcher(omci_alm_t		*pData);
GOS_ERROR_CODE omci_alarm_reset_sequence_number(void);
void omci_alarm_snapshot_timer_handler(UINT16   classID,
                                        UINT16  instanceID,
                                        UINT32  privData);
UINT16 omci_alarm_snapshot_create_all(UINT8     almRetrievalMode);
UINT16 omci_alarm_snapshot_get_by_seq(UINT16                almSnapShotSeq,
                                        omci_me_class_t     *pClassID,
                                        omci_me_instance_t  *pInstanceID,
                                        mib_alarm_table_t   *pAlarmTable);
GOS_ERROR_CODE omci_alarm_snapshot_delete_all(void);

/*
 *  Define OMCI Configure API
*/
extern GOS_ERROR_CODE OMCI_MibCreate_Cmd(int classId, UINT16 entityId, char *value);
extern GOS_ERROR_CODE OMCI_MibDelete_Cmd(int classId, UINT16 entityId);
extern GOS_ERROR_CODE OMCI_MibSet_Cmd(int classId, UINT16 entityId, char *fileName, unsigned char *val);
extern GOS_ERROR_CODE OMCI_MibReset_Cmd(void);
extern GOS_ERROR_CODE OMCI_SnGet_Cmd(void);
extern GOS_ERROR_CODE OMCI_SnSet_Cmd(unsigned char *serial);
extern GOS_ERROR_CODE OMCI_MibDump_Cmd(int tableIdx, char *tableName, int instanceId);
extern GOS_ERROR_CODE OMCI_MibDump_PM_Cmd(char *tableName, int instanceId);
extern GOS_ERROR_CODE OMCI_MibAttrGet_Cmd(UINT32 srcMsgQkey, int cid, unsigned short eid, int aid);
extern GOS_ERROR_CODE OMCI_ConnDump_Cmd(void);
extern GOS_ERROR_CODE OMCI_PriQtoTcontDump_Cmd(void);
extern GOS_ERROR_CODE OMCI_LogSet_Cmd(int level);
extern GOS_ERROR_CODE OMCI_LogGet_Cmd();
extern GOS_ERROR_CODE OMCI_TreeDump_Cmd(int key);
extern GOS_ERROR_CODE OMCI_GenTrafficRule(MIB_TREE_CONN_T *pConn,int index);
extern GOS_ERROR_CODE OMCI_MeOperDumpMib(MIB_TABLE_INDEX tableId,void *pData);
extern GOS_ERROR_CODE OMCI_MeOperConnCheck(MIB_TABLE_INDEX tableId,MIB_TREE_T *pTree,MIB_TREE_CONN_T *pConn,omci_me_instance_t entityId,int parm);
extern GOS_ERROR_CODE OMCI_MeOperAvlTreeAdd(MIB_TABLE_INDEX tableId,MIB_TREE_T* pTree,omci_me_instance_t entityId);
extern GOS_ERROR_CODE OMCI_MeOperAvlTreeDel(MIB_TABLE_INDEX tableId,MIB_TREE_T* pTree,omci_me_instance_t entityId);
extern GOS_ERROR_CODE OMCI_MeOperCfg(MIB_TABLE_INDEX tableId, void* pOldRow, void* pNewRow, MIB_OPERA_TYPE operType, MIB_ATTRS_SET attrSet, UINT32 pri);
extern GOS_ERROR_CODE OMCI_MeOperCfgTaskInit(void);
extern GOS_ERROR_CODE OMCI_AttrsMibSetToOmciSet(MIB_ATTRS_SET* pMibAttrSet, omci_msg_attr_mask_t* pOmciAttrSet);
extern GOS_ERROR_CODE MIB_ShowAll(void);
extern GOS_ERROR_CODE OMCI_DevModeGet_Cmd(void);
extern GOS_ERROR_CODE OMCI_DevModeSet_Cmd(char *devMode);
extern GOS_ERROR_CODE OMCI_SrIndSet_Cmd(unsigned char srInd);
extern GOS_ERROR_CODE OMCI_PbDbaRptSet_Cmd(unsigned char pbDbaRpt);
extern GOS_ERROR_CODE OMCI_ServiceFlowDump_Cmd(void);
extern GOS_ERROR_CODE OMCI_LoidSet_Cmd(unsigned char *loid, unsigned char *passwd);
extern GOS_ERROR_CODE OMCI_LoidGet_Cmd(void);
extern GOS_ERROR_CODE OMCI_LoidAuthStatusGet_Cmd(UINT32 srcMsgQkey);
extern GOS_ERROR_CODE OMCI_DrvVersionGet_Cmd(void);
extern GOS_ERROR_CODE OMCI_IotVlanCfgSet_Cmd(unsigned char type, unsigned char mode, unsigned char pri, unsigned short vid);
extern GOS_ERROR_CODE OMCI_cFlagSet_Cmd(char *type, unsigned int level);
extern GOS_ERROR_CODE OMCI_cFlagGet_Cmd(void);
extern GOS_ERROR_CODE OMCI_SimAVC_Cmd(int classId, UINT16 entityId, unsigned char attrIndex);
extern GOS_ERROR_CODE OMCI_IotVlanDetect_Cmd(void);
extern GOS_ERROR_CODE OMCI_DotGen_Cmd(void);
extern GOS_ERROR_CODE OMCI_RegModShow_Cmd(void);
extern GOS_ERROR_CODE OMCI_RegApiShow_Cmd(void);

GOS_ERROR_CODE omci_cmd_set_dmmode_handler(int state);
GOS_ERROR_CODE omci_cmd_get_dmmode_handler(void);
GOS_ERROR_CODE omci_cmd_send_alarm_handler(int type, unsigned int number, int status, unsigned short detail);
GOS_ERROR_CODE omci_cmd_dump_alarm_handler(int meId, unsigned short instanceId);
extern UINT16 omci_GetUsFlowIdByPortId(UINT16 portId);
UINT16 omci_GetDsFlowIdByPortId(UINT16 portId);
GOS_ERROR_CODE omci_is_notify_suppressed_by_circuitpack(omci_me_instance_t	slotID,
														BOOL				*pSuppressed);
GOS_ERROR_CODE omci_AvcCallback(MIB_TABLE_INDEX		tableIndex,
								void				*pOldRow,
								void				*pNewRow,
								MIB_ATTRS_SET		*pAttrsSet,
								MIB_OPERA_TYPE		operationType);
GOS_ERROR_CODE omci_arc_timer_processor(MIB_TABLE_INDEX		tableIndex,
										void				*pOldRow,
										void				*pNewRow,
										MIB_ATTR_INDEX		arcIndex,
										MIB_ATTR_INDEX		arcIntervalIndex);
GOS_ERROR_CODE pptp_eth_uni_switch_port_to_me_id(UINT16             portId,
                                                omci_me_instance_t    *pInstanceID);
GOS_ERROR_CODE pptp_eth_uni_me_id_to_switch_port(omci_me_instance_t instanceID,
                                                UINT16              *pPortId);
GOS_ERROR_CODE omci_get_all_fe_eth_uni_port_mask(UINT32 *pPortMask);
GOS_ERROR_CODE omci_get_all_ge_eth_uni_port_mask(UINT32 *pPortMask);
GOS_ERROR_CODE omci_get_all_eth_uni_port_mask(UINT32 *pPortMask);
GOS_ERROR_CODE omci_get_eth_uni_port_mask_behind_veip(UINT16 mbspId, UINT32 *pPortMask);
GOS_ERROR_CODE anig_generic_transceiver_para_updater(double     *pOutput,
                                omci_transceiver_status_type_t  type);
GOS_ERROR_CODE omci_pm_update_pptp_eth_uni(omci_pm_oper_type_t  operType);
GOS_ERROR_CODE omci_pm_getcurrent_pptp_eth_uni(omci_me_instance_t   instanceID,
                                                void                **ppCntrs);
GOS_ERROR_CODE omci_pm_update_gem_port(omci_pm_oper_type_t  operType);
GOS_ERROR_CODE omci_pm_getcurrent_gem_port(omci_me_instance_t   instanceID,
                                            void                **ppCntrs);
GOS_ERROR_CODE omci_pm_release_all_counters(void);
GOS_ERROR_CODE omci_pm_is_threshold_crossed(MIB_TABLE_INDEX     tableIndex,
                                            omci_me_instance_t  instanceID,
                                            void                *pRow,
                                            UINT8               *pTcaAlmNumber,
                                            UINT8               *pTcaAttrMap,
                                            UINT16              *pTcaDisableMask,
                                            BOOL                *pIsTcaRaised);
GOS_ERROR_CODE omci_pm_clear_all_raised_tca(MIB_TABLE_INDEX     tableIndex,
                                            omci_me_instance_t  instanceID);
GOS_ERROR_CODE omci_pm_clear_raised_tca(MIB_TABLE_INDEX     tableIndex,
                                        omci_me_instance_t  instanceID,
                                        UINT8               *pTcaAlmNumber,
                                        UINT8               *pTcaAttrMap,
                                        UINT16              *pTcaDisableMask);
GOS_ERROR_CODE omci_is_traffic_descriptor_supported(MIB_TABLE_TRAFFICDESCRIPTOR_T	*pMibTD);
GOS_ERROR_CODE omci_is_traffic_descriptor_existed(omci_me_instance_t                instanceID,
													MIB_TABLE_TRAFFICDESCRIPTOR_T	*pMibTD);
GOS_ERROR_CODE omci_apply_traffic_descriptor_to_gem_flow(OMCI_GEM_FLOW_ts				flowCfg,
														MIB_TABLE_TRAFFICDESCRIPTOR_T	*pMibTD);
GOS_ERROR_CODE omci_apply_traffic_descriptor_to_gem_port(omci_me_instance_t				instanceID,
														PON_GEMPORT_DIRECTION			gemDirection,
														MIB_TABLE_TRAFFICDESCRIPTOR_T	*pMibTD);
GOS_ERROR_CODE omci_apply_traffic_descriptor_to_uni_port(omci_me_instance_t             instanceID,
                                                        OMCI_UNI_RATE_DIRECTION         dir,
                                                        MIB_TABLE_TRAFFICDESCRIPTOR_T	*pMibTD);
GOS_ERROR_CODE omci_pwr_shedding_processor(omci_alm_status_t    almStatus);
GOS_ERROR_CODE omci_get_pptp_eth_uni_port_mask_in_bridge(UINT16 mbspId, UINT32 *pPortMask);
GOS_ERROR_CODE omci_update_dot1_rate_limiter_port_mask(UINT16 mbspId, UINT32 newPortMask, UINT32 oldPortMask);

GOS_ERROR_CODE omci_bridgePortIndexGet(UINT16 entityId, UINT32 *pPortIdx);

extern void omci_GetPrivateVlanCfgObj(MIB_TABLE_PRIVATE_VLANCFG_T **ppMibPrivateVlanCfg);
extern void omci_generate_vlan_info(void);
extern GOS_ERROR_CODE omci_UpdateTrafficRuleByManual(MIB_TREE_CONN_T *pConn, int index, omci_vlan_rule_t *pRule);

extern void omci_generate_dot_file(void);

GOS_ERROR_CODE omci_ParseDscp2Pbit(UINT8 *pDscpToPbitMapping, OMCI_DSCP2PBIT_ts *pDscp2PbitTable);

/*
 * Define OMCI Action API
*/
extern GOS_ERROR_CODE OMCI_OnCreateMsg(omci_msg_norm_baseline_t* pNorOmciMsg);
extern GOS_ERROR_CODE OMCI_AttrsMibSetToOmciSet(MIB_ATTRS_SET* pMibAttrSet, omci_msg_attr_mask_t* pOmciAttrSet);
extern GOS_ERROR_CODE OMCI_CheckIsMeValid(UINT16 MeClassID, UINT16* pMeInstance, BOOL* pMeValid, BOOL* pMeExist);
extern GOS_ERROR_CODE OMCI_OnDeleteMsg(omci_msg_norm_baseline_t* pNorOmciMsg);
extern GOS_ERROR_CODE OMCI_OnSetMsg(omci_msg_norm_baseline_t* pNorOmciMsg);
extern GOS_ERROR_CODE OMCI_OnGetMsg(omci_msg_norm_baseline_t* pNorOmciMsg);
extern GOS_ERROR_CODE OMCI_OnMibReset(omci_msg_norm_baseline_t* pNorOmciMsg);
extern GOS_ERROR_CODE OMCI_OnMibUploadMsg(omci_msg_norm_baseline_t* pNorOmciMsg);
extern GOS_ERROR_CODE OMCI_OnMibUploadNextMsg(omci_msg_norm_baseline_t* pNorOmciMsg);
extern GOS_ERROR_CODE OMCI_OnStartSWDL(omci_msg_norm_baseline_t* pNorOmciMsg);
extern GOS_ERROR_CODE OMCI_OnDLSection(omci_msg_norm_baseline_t* pNorOmciMsg);
extern GOS_ERROR_CODE OMCI_OnEndSWDL(omci_msg_norm_baseline_t* pNorOmciMsg);
extern GOS_ERROR_CODE OMCI_OnActivateSw(omci_msg_norm_baseline_t* pNorOmciMsg);
extern GOS_ERROR_CODE OMCI_OnCommitSw(omci_msg_norm_baseline_t* pNorOmciMsg);
extern GOS_ERROR_CODE OMCI_OnReboot(omci_msg_norm_baseline_t* pNorOmciMsg);
extern GOS_ERROR_CODE OMCI_OnGetNextMsg(omci_msg_norm_baseline_t* pNorOmciMsg);
GOS_ERROR_CODE OMCI_OnGetAllAlarms(omci_msg_norm_baseline_t		*pNorOmciMsg);
GOS_ERROR_CODE OMCI_OnGetAllAlarmsNext(omci_msg_norm_baseline_t		*pNorOmciMsg);
GOS_ERROR_CODE OMCI_OnTest(omci_msg_norm_baseline_t     *pNorOmciMsg);
GOS_ERROR_CODE OMCI_OnSync(omci_msg_norm_baseline_t     *pNorOmciMsg);
extern BOOL OMCI_CheckIsActionSupported(UINT16 meClassID, UINT8 type);

/*OMCI Action*/
extern omci_msg_attr_mask_t omci_GetOltAccAttrSet(MIB_TABLE_INDEX tableIndex, omci_me_attr_access_mask_t accType);
extern GOS_ERROR_CODE omci_AttrsOmciSetToMibSet(omci_msg_attr_mask_t* pOmciAttrSet, MIB_ATTRS_SET* pMibAttrSet);

extern UINT16 omci_adjust_tbl_ctrl_by_omcc_ver(UINT16 val);

/* Mcast macro */
#define MCAST_WRAPPER(fn, args...) \
    do { \
        if (((omci_mcast_wrapper_t *)gInfo.pMCwrapper)->fn == NULL) \
        { \
            OMCI_LOG(OMCI_LOG_LEVEL_HIGH, "no hokking"); \
        } \
        else \
        { \
            ((omci_mcast_wrapper_t *)gInfo.pMCwrapper)->fn(args); \
        } \
    } while (0);

/* Mcast util API */
extern GOS_ERROR_CODE omci_mcast_port_reset(UINT16 portId, MIB_TABLE_MCASTSUBCONFINFO_T* pMibMcastSubCfgInfo, UINT16 mcastOperProfId);

#ifdef __cplusplus
}
#endif

#endif
