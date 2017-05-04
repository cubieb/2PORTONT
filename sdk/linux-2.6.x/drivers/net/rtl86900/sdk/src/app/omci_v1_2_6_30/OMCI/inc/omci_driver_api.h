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
 * Purpose : Definition of OMCI driver APIs
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) OMCI driver APIs
 */

#ifndef __OMCI_DRIVER_API_H__
#define __OMCI_DRIVER_API_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "mib_table.h"
#include "omci_driver.h"


// wrapper debug functions
void omci_wrapper_dumpSrvId(void);
void omci_wrapper_dumpPQ2TC(void);

// wrapper meta mgmt functions
GOS_ERROR_CODE omci_wrapper_initDrvDb(void);
GOS_ERROR_CODE omci_wrapper_msgHandler(void *pData);
GOS_ERROR_CODE omci_wrapper_createMsgDev(void);
GOS_ERROR_CODE omci_wrapper_deleteMsgDev(void);
GOS_ERROR_CODE omci_wrapper_sendOmciMsg(void *rawMsg, unsigned int size);
GOS_ERROR_CODE omci_wrapper_intrHandler(void *pData);
GOS_ERROR_CODE omci_wrapper_createIntrSocket(void);
GOS_ERROR_CODE omci_wrapper_deleteIntrSocket(void);
GOS_ERROR_CODE omci_wrapper_createCtrlDev(void);
GOS_ERROR_CODE omci_wrapper_deleteCtrlDev(void);

/* wrapper ctrl functions */
// driver control
GOS_ERROR_CODE omci_SetTcontInfo(UINT16 tcontId, UINT16 entityId, UINT16 allocId);
GOS_ERROR_CODE omci_wrapper_resetMib(void);
GOS_ERROR_CODE omci_wrapper_setLog(int level);
GOS_ERROR_CODE omci_wrapper_setLogToFile(OMCI_LOGFILE_MODE mode, UINT32 actMask, CHAR *fileName);
GOS_ERROR_CODE omci_wrapper_getLogToFileCfg();
GOS_ERROR_CODE omci_wrapper_setDevMode(int devMode);

// device info
GOS_ERROR_CODE omci_wrapper_getDevCapabilities(omci_dev_capability_t *p);
GOS_ERROR_CODE omci_wrapper_getDevIdVersion(omci_dev_id_version_t *p);
GOS_ERROR_CODE omci_wrapper_setDualMgmtMode(int state);
GOS_ERROR_CODE omci_wrapper_getDrvVersion(char *drvVersion);

// optical info and control
GOS_ERROR_CODE omci_wrapper_getUsDBRuStatus(unsigned int *pStatus);
GOS_ERROR_CODE omci_wrapper_getTransceiverStatus(omci_transceiver_status_t *p);
GOS_ERROR_CODE omci_wrapper_setSignalParameter(OMCI_SIGNAL_PARA_ts *pSigPara);
GOS_ERROR_CODE omci_wrapper_getOnuState(PON_ONU_STATE *pOnuState);

// pon/ani control
GOS_ERROR_CODE omci_wrapper_setSerialNum(unsigned char *serial);
GOS_ERROR_CODE omci_wrapper_getSerialNum(char *serial);
GOS_ERROR_CODE omci_wrapper_setGponPasswd(unsigned char *gponPwd);
GOS_ERROR_CODE omci_wrapper_activateGpon(int activate);
GOS_ERROR_CODE omci_wrapper_setGemBlkLen(unsigned short gemBlkLen);
GOS_ERROR_CODE omci_wrapper_getGemBlkLen(unsigned short *pGemBlkLen);
GOS_ERROR_CODE omci_wrapper_setPonBwThreshold(omci_pon_bw_threshold_t *pPonBwThreshold);
GOS_ERROR_CODE omci_wrapper_createTcont(UINT16 entityId, UINT16 allocId);
GOS_ERROR_CODE omci_wrapper_deleteTcont(UINT16 entityId, UINT16 allocId);
GOS_ERROR_CODE omci_wrapper_setPriQueue(MIB_TABLE_PRIQ_T *pPriQ);
GOS_ERROR_CODE omci_wrapper_cfgGemFlow(OMCI_GEM_FLOW_ts data);
GOS_ERROR_CODE omci_wrapper_setGemFlowRate(OMCI_GEM_FLOW_ts data);
GOS_ERROR_CODE omci_wrapper_setDsBcGemFlow(unsigned int flowId);

// uni info and control
GOS_ERROR_CODE omci_wrapper_getPortLinkStatus(omci_port_link_status_t *pStatus);
GOS_ERROR_CODE omci_wrapper_getPortSpeedDuplexStatus(omci_port_speed_duplex_status_t *pStatus);
GOS_ERROR_CODE omci_wrapper_setPortAutoNegoAbility(omci_port_auto_nego_ability_t *pAbility);
GOS_ERROR_CODE omci_wrapper_getPortAutoNegoAbility(omci_port_auto_nego_ability_t *pAbility);
GOS_ERROR_CODE omci_wrapper_setPortState(omci_port_state_t *pState);
GOS_ERROR_CODE omci_wrapper_getPortState(omci_port_state_t *pState);
GOS_ERROR_CODE omci_wrapper_setMaxFrameSize(omci_port_max_frame_size_t *pSize);
GOS_ERROR_CODE omci_wrapper_getMaxFrameSize(omci_port_max_frame_size_t *pSize);
GOS_ERROR_CODE omci_wrapper_setPhyLoopback(omci_port_loopback_t *pLoopback);
GOS_ERROR_CODE omci_wrapper_getPhyLoopback(omci_port_loopback_t *pLoopback);
GOS_ERROR_CODE omci_wrapper_setPhyPwrDown(omci_port_pwr_down_t *pPwrDown);
GOS_ERROR_CODE omci_wrapper_getPhyPwrDown(omci_port_pwr_down_t *pPwrDown);
GOS_ERROR_CODE omci_wrapper_setPauseControl(omci_port_pause_ctrl_t *pPortPauseCtrl);
GOS_ERROR_CODE omci_wrapper_getPauseControl(omci_port_pause_ctrl_t *pPortPauseCtrl);

// statistics
GOS_ERROR_CODE omci_wrapper_getPortStat(omci_port_stat_t *pStat);
GOS_ERROR_CODE omci_wrapper_resetPortStat(unsigned int port);
GOS_ERROR_CODE omci_wrapper_getUsFlowStat(omci_flow_stat_t *pStat);
GOS_ERROR_CODE omci_wrapper_resetUsFlowStat(unsigned int flow);
GOS_ERROR_CODE omci_wrapper_getDsFlowStat(omci_flow_stat_t *pStat);
GOS_ERROR_CODE omci_wrapper_resetDsFlowStat(unsigned int flow);
GOS_ERROR_CODE omci_wrapper_getDsFecStat(omci_ds_fec_stat_t *pStat);
GOS_ERROR_CODE omci_wrapper_resetDsFecStat(void);

// bridge/vlan control
GOS_ERROR_CODE omci_wrapper_activeBdgConn(omci_vlan_rule_t *pRule);
GOS_ERROR_CODE omci_wrapper_deactiveBdgConn(int servId, int ingress);
GOS_ERROR_CODE omci_wrapper_setDscpRemap(OMCI_DSCP2PBIT_ts *pDscp2PbitTable);
GOS_ERROR_CODE omci_wrapper_setMacLearnLimit(unsigned int portIdx, unsigned int macLimitNum);
GOS_ERROR_CODE omci_wrapper_setMacFilter(OMCI_MACFILTER_ts *pMacFilter);
GOS_ERROR_CODE omci_wrapper_setGroupMacFilter(OMCI_GROUPMACFILTER_ts *pGroupMacFilter);
GOS_ERROR_CODE omci_wrapper_setSvlanTpid(unsigned int index, unsigned int tpid);
GOS_ERROR_CODE omci_wrapper_getSvlanTpid(unsigned int index, unsigned int *pTpid);
GOS_ERROR_CODE omci_wrapper_getCvlanState(unsigned int *state);
GOS_ERROR_CODE omci_wrapper_setDot1RateLimiter(omci_dot1_rate_t *pDot1Rate);
GOS_ERROR_CODE omci_wrapper_delDot1RateLimiter(omci_dot1_rate_t *pDot1Rate);
GOS_ERROR_CODE omci_wrapper_getBridgeTableByPort(omci_bridge_tbl_per_port_t *p);

// veip control
GOS_ERROR_CODE omci_wrapper_setUsVeipPriQ(MIB_TREE_T *pMibTree, UINT16 gemPortId, UINT8 isCreate);

// uni rate
GOS_ERROR_CODE omci_wrapper_setUniPortRate(omci_port_rate_t *pRatePerPort);

#ifdef __cplusplus
}
#endif

#endif
