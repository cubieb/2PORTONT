/*
 * Copyright (C) 2012 Realtek Semiconductor Corp.
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
 * $Date: 2013-06-24 04:35:27 -0500 (Fri, 03 May 2013) $
 *
 * Purpose : OMCI driver layer module defination
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) OMCI (G.984.4)
 *
 */

#ifndef __OMCI_DRV_H__
#define __OMCI_DRV_H__


#include <linux/list.h>
#include <OMCI/src/omci_mib.h>
#include <OMCI/inc/omci_driver.h>

#define OMCI_UNUSED_CF 0xffff
#define OMCI_UNUSED_ACL 0xffff
#define FEATURE_KMODULE_MAME_LENGTH 128

enum
{
    FEATURE_KAPI_START                            = 0,
    FEATURE_KAPI_BDP_00000001_USRULE2DSRULE       = FEATURE_KAPI_START,
    FEATURE_KAPI_BDP_00000001_USACT2DSSACT,
    FEATURE_KAPI_BDP_00000001_USACT2DSCACT,
    FEATURE_KAPI_BDP_00000001_SET_SVLAN_MBR,
    FEATURE_KAPI_BDP_00000001_DEL_SVLAN_MBR,
    FEATURE_KAPI_BDP_00000002,
    FEATURE_KAPI_BDP_00000010,
    FEATURE_KAPI_RDP_00000002,
    FEATURE_KAPI_ME_00000001,
    FEATURE_KAPI_END
};

enum
{
    OMCIDRV_FEATURE_ERR_OK                  = 0x100,
    OMCIDRV_FEATURE_ERR_FAIL,
    OMCIDRV_FEATURE_ERR_END
};

typedef struct {
	// debug functions
	int (*pf_DumpCfMap)(void);
	int (*pf_DumpL2Serv)(void);
	int (*pf_DumpVeipServ)(void);
	int (*pf_DumpMBServ)(void);
	int (*pf_DumpVeipGemFlow)(void);
	int (*pf_DumpMacFilter)(void);

	// driver control
	int (*pf_ResetMib)(void);
	int (*pf_SetCfMap)(unsigned int cfType, unsigned int start, unsigned int stop);

	// device info
	int (*pf_GetDevCapabilities)(omci_dev_capability_t *p);
	int (*pf_GetDevIdVersion)(omci_dev_id_version_t *p);
	int (*pf_GetDrvVersion)(char *drvVersion);

	// optical info and control
	int (*pf_GetUsDBRuStatus)(unsigned int *p);
	int (*pf_GetTransceiverStatus)(omci_transceiver_status_t *p);
	int (*pf_SetSigParameter)(OMCI_SIGNAL_PARA_ts *pSigPara);
	int	(*pf_GetOnuState)(PON_ONU_STATE *pOnuState);

	// pon/ani control
	int (*pf_SetSerialNum)(char *serial);
	int (*pf_GetSerialNum)(char *serial);
	int (*pf_SetGponPasswd)(char *gponPwd);
	int (*pf_ActivateGpon)(int activate);
	int (*pf_GetGemBlkLen)(unsigned short *pGemBlkLen);
	int (*pf_SetGemBlkLen)(unsigned short gemBlkLen);
	int (*pf_SetPonBwThreshold)(omci_pon_bw_threshold_t *pPonBwThreshold);
	int (*pf_CreateTcont)(OMCI_TCONT_ts *pTcont);
	int (*pf_UpdateTcont)(OMCI_TCONT_ts *pTcont);
    int (*pf_SetPriQueue)(OMCI_PRIQ_ts *pPriQ);
	int (*pf_ClearPriQueue)(OMCI_PRIQ_ts *pPriQ);
	int (*pf_CfgGemFlow)(OMCI_GEM_FLOW_ts *pGemFlow);
	int (*pf_SetDsBcGemFlow)(unsigned int *pFlowId);

	// uni info and control
	int (*pf_GetPortLinkStatus)(omci_port_link_status_t *p);
	int (*pf_GetPortSpeedDuplexStatus)(omci_port_speed_duplex_status_t *p);
	int (*pf_SetPortAutoNegoAbility)(omci_port_auto_nego_ability_t *p);
	int (*pf_GetPortAutoNegoAbility)(omci_port_auto_nego_ability_t *p);
	int (*pf_SetPortState)(omci_port_state_t *p);
	int (*pf_GetPortState)(omci_port_state_t *p);
	int (*pf_SetPortMaxFrameSize)(omci_port_max_frame_size_t *p);
	int (*pf_GetPortMaxFrameSize)(omci_port_max_frame_size_t *p);
	int (*pf_SetPortPhyLoopback)(omci_port_loopback_t *p);
	int (*pf_GetPortPhyLoopback)(omci_port_loopback_t *p);
	int (*pf_SetPortPhyPwrDown)(omci_port_pwr_down_t *p);
	int (*pf_GetPortPhyPwrDown)(omci_port_pwr_down_t *p);
	int (*pf_SetPauseControl)(omci_port_pause_ctrl_t *p);
	int (*pf_GetPauseControl)(omci_port_pause_ctrl_t *p);

	// statistics
	int (*pf_GetPortStat)(omci_port_stat_t *p);
	int (*pf_ResetPortStat)(unsigned int port);
	int (*pf_GetUsFlowStat)(omci_flow_stat_t *p);
	int (*pf_ResetUsFlowStat)(unsigned int flow);
	int (*pf_GetDsFlowStat)(omci_flow_stat_t *p);
	int (*pf_ResetDsFlowStat)(unsigned int flow);
	int (*pf_GetDsFecStat)(omci_ds_fec_stat_t *p);
	int (*pf_ResetDsFecStat)(void);

	// bridge/vlan control
	int (*pf_ActiveBdgConn)(OMCI_BRIDGE_RULE_ts *pBridgeRule);
	int (*pf_DeactiveBdgConn)(int servId);
	int (*pf_SetDscpRemap)(OMCI_DSCP2PBIT_ts *pDscp2PbitTable);
	int (*pf_SetMacLearnLimit)(OMCI_MACLIMIT_ts *pMacLimit);
	int (*pf_SetMacFilter)(OMCI_MACFILTER_ts *pMacFilter);
	int (*pf_SetGroupMacFilter)(OMCI_GROUPMACFILTER_ts *pGroupMacFilter);
	int (*pf_SetSvlanTpid)(omci_svlan_tpid_t *p);
	int (*pf_GetSvlanTpid)(omci_svlan_tpid_t *p);
    int (*pf_GetCvlanState)(unsigned int *p);
    int (*pf_SetDot1RateLimiter)(omci_dot1_rate_meter_t *pDot1RateMeter);
    int (*pf_DelDot1RateLimiter)(omci_dot1_rate_meter_t *pDot1RateMeter);
    int (*pf_GetBgTblPerPort)(omci_bridge_tbl_per_port_t *p);
    int (*pf_SetMacAgeTime) (unsigned int p);

	// veip control
	int (*pf_SetVeipGemFlow)(veipGemFlow_t *p);
	int (*pf_DelVeipGemFlow)(veipGemFlow_t *p);
	int (*pf_UpdateVeipRule)(int wanIdx, int vid, int pri, int type, int service, int isBinding, int netIfIdx, unsigned char isRegister);

    // uni port rate
	int (*pf_SetUniPortRate)(omci_port_rate_t *p);
} pf_wrapper_t;

typedef struct
{
    unsigned int regApiId;
    unsigned int (*regCB)(va_list argp);
    char         regModuleName[FEATURE_KMODULE_MAME_LENGTH];
} feature_kapi_t;

// for omci l2 service
typedef struct {
	unsigned int			index;
	PON_GEMPORT_DIRECTION	dir;
	unsigned int			usStreamId;
	unsigned int			usDpStreamId;
	unsigned int			usDpMarking;
	unsigned int			dsStreamId;
	unsigned int			*pUsCfIndex;
	unsigned int			*pUsDpCfIndex;
	unsigned int			dsCfIndex;
	unsigned int			isCfg;
	unsigned int			uniMask;
	struct list_head		list;
} l2_service_t;

// for omci mcast service
typedef struct {
	unsigned int		index;
	// only update uni port mask if ds rule and dsStreamId are the same
	OMCI_VLAN_OPER_ts	rule;
	unsigned int		dsStreamId;  // it maybe mcast or bcast flow
	unsigned int		dsCfIndex;
    unsigned int        referAclIdx;
	unsigned int		isCfg;
	unsigned int		uniMask;
	struct list_head	list;
} mbcast_service_t;

// for omci veip service
typedef struct {
	unsigned int		index;
	OMCI_VLAN_OPER_ts	rule;
	unsigned short		usStreamId;
	unsigned short		dsStreamId;
	unsigned int		*pHwPathCfIdx;
	unsigned int		*pSwPathCfIdx;
	unsigned int		defaultPathCfIdx;
	unsigned char		wanIfBitmap;
	struct list_head	list;
} veip_service_t;

typedef struct {
	veipGemFlow_t		data;
	struct list_head	list;
} veipGemFlow_entry_t;

typedef struct {
	uint64				key;
	unsigned int		hwAclIdx;
	struct list_head	list;
} macFilter_entry_t;

// to maintain cf rule
typedef struct {
	unsigned int	isCfg;
} pf_cf_rule_t;

// to maintain tcont
typedef struct {
	unsigned int	allocId;
	unsigned int	qIdFrom;
} pf_tcont_t;

// to maintain meter
typedef struct {
	unsigned char	isCfg;
	unsigned int	meterIdx;
} pf_stormCtrl_t;

// to maintain global database
typedef struct {
	struct list_head	l2Head;
	struct list_head	veipHead;
	struct list_head	veipGemFlowHead;
	struct list_head	mbcastHead;
	struct list_head	macFilterHead;

	// chip info
	unsigned int	chipId;

	// pon info
	unsigned int	omccTcont;
    unsigned int	omccQueue;
	unsigned int	omccFlow;
	unsigned int	maxTcont;
	unsigned int	maxTcontQueue;
	unsigned int	maxPonQueue;
	unsigned int	maxQueueRate;
	unsigned int	maxFlow;
	unsigned int	sidMask;
	pf_tcont_t		*tCont;
	unsigned char	*gemEncrypt;

	// port info
	int				cpuPort;
	int				rgmiiPort;
	int				ponPort;
	unsigned int	etherPortMin;
	unsigned int	etherPortMax;
	unsigned int	etherPortMask;
	unsigned int	allPortMin;
	unsigned int	allPortMax;
	unsigned int	uniPortMask;
	unsigned int	perUniQueue;

	// qos
	unsigned int	maxPriSelWeight;
	unsigned char	perTContQueueDp;
	unsigned char	perUNIQueueDp;
	unsigned int	meterNum;
	pf_stormCtrl_t	*stormCtrl;

	// cf
	unsigned int	cfNum;      /*CF rule can be used*/
	unsigned int	cfTotalNum; /*CF num of chip*/   
	unsigned int	veipFastStart;
	unsigned int	veipFastStop;
	unsigned int	ethTypeFilterStart;
	unsigned int	ethTypeFilterStop;
	unsigned int	l2CommStart;
	unsigned int	l2CommStop;
	pf_cf_rule_t	*cfRule;

	// acl
	unsigned int	aclNum;
	unsigned int	aclActNum;

	// l34
	unsigned int	intfNum;
	unsigned int	bindingTblMax;
	unsigned int	rgMacBasedTag;
	unsigned int	rgIvlMcastSupport;

    // l2
    unsigned int    maxLearnCnt;

	pf_wrapper_t	*pMap;

    /* feature kapi info */
    feature_kapi_t    kApiDb[FEATURE_KAPI_END];

    /* mmap_buffer */
    unsigned char *pMmap;

} pf_db_t;


typedef struct {
	int logLevel;
	int devMode;
	unsigned char dmMode;
} omci_drv_control_t;


extern omci_drv_control_t gDrvCtrl;


#define OMCI_LOG(level,fmt, arg...) \
    do { if (gDrvCtrl.logLevel >= level) { printk(fmt, ##arg); printk("\n"); } } while (0);



// debug functions
int omcidrv_wrapper_dumpCfMap(void);
int omcidrv_wrapper_dumpL2Serv(void);
int omcidrv_wrapper_dumpVeipServ(void);
int omcidrv_wrapper_dumpMBServ(void);
int omcidrv_wrapper_dumpUsVeipGemFlow(void);
int omcidrv_wrapper_dumpMacFilter(void);

// driver control
int omcidrv_wrapper_resetMib(void);
int omcidrv_wrapper_setCfMap(unsigned int cfType, unsigned int start, unsigned int stop);
int omcidrv_wrapper_setLog(int loglevel);
int omcidrv_wrapper_setDevMode(int dMode);

// device info
int omcidrv_wrapper_getDevCapabilities(omci_dev_capability_t *p);
int omcidrv_wrapper_getDevIdVersion(omci_dev_id_version_t *p);
int omcidrv_wrapper_setDualMgmtMode(int state);
int omcidrv_wrapper_getDrvVersion(char *pDrvVersion);

// optical info and control
int omcidrv_wrapper_getUsDBRuStatus(unsigned int *p);
int omcidrv_wrapper_getTransceiverStatus(omci_transceiver_status_t *p);
int omcidrv_wrapper_setSignalParameter(OMCI_SIGNAL_PARA_ts *pSigPara);
int omcidrv_wrapper_getOnuState(PON_ONU_STATE *pOnuState);

// pon/ani control
int omcidrv_wrapper_setSerialNum(char *pSerialNum);
int omcidrv_wrapper_getSerialNum(char *pSerialNum);
int omcidrv_wrapper_setGponPasswd(char *pGponPwd);
int omcidrv_wrapper_activateGpon(int activate);
int omcidrv_wrapper_setGemBlkLen(unsigned short gemBlkLen);
int omcidrv_wrapper_getGemBlkLen(unsigned short *pGemBlkLen);
int omcidrv_wrapper_setPonBwThreshold(omci_pon_bw_threshold_t *pPonBwThreshold);
int omcidrv_wrapper_createTcont(OMCI_TCONT_ts *pTcont);
int omcidrv_wrapper_updateTcont(OMCI_TCONT_ts *pTcont);
int omcidrv_wrapper_setPriQueue(OMCI_PRIQ_ts *pPriQ);
int omcidrv_wrapper_clearPriQueue(OMCI_PRIQ_ts *pPriQ);
int omcidrv_wrapper_cfgGemFlow(OMCI_GEM_FLOW_ts *pGemFlow);
int omcidrv_wrapper_setDsBcGemFlow(unsigned int *pFlowId);

// uni info and control
int omcidrv_wrapper_getPortLinkStatus(omci_port_link_status_t *p);
int omcidrv_wrapper_getPortSpeedDuplexStatus(omci_port_speed_duplex_status_t *p);
int omcidrv_wrapper_setPortAutoNegoAbility(omci_port_auto_nego_ability_t *p);
int omcidrv_wrapper_getPortAutoNegoAbility(omci_port_auto_nego_ability_t *p);
int omcidrv_wrapper_setPortState(omci_port_state_t *p);
int omcidrv_wrapper_getPortState(omci_port_state_t *p);
int omcidrv_wrapper_setPortMaxFrameSize(omci_port_max_frame_size_t *p);
int omcidrv_wrapper_getPortMaxFrameSize(omci_port_max_frame_size_t *p);
int omcidrv_wrapper_setPortPhyLoopback(omci_port_loopback_t *p);
int omcidrv_wrapper_getPortPhyLoopback(omci_port_loopback_t *p);
int omcidrv_wrapper_setPortPhyPwrDown(omci_port_pwr_down_t *p);
int omcidrv_wrapper_getPortPhyPwrDown(omci_port_pwr_down_t *p);
int omcidrv_wrapper_setPauseControl(omci_port_pause_ctrl_t *p);
int omcidrv_wrapper_getPauseControl(omci_port_pause_ctrl_t *p);

// statistics
int omcidrv_wrapper_getPortStat(omci_port_stat_t *p);
int omcidrv_wrapper_resetPortStat(unsigned int port);
int omcidrv_wrapper_getUsFlowStat(omci_flow_stat_t *p);
int omcidrv_wrapper_resetUsFlowStat(unsigned int flow);
int omcidrv_wrapper_getDsFlowStat(omci_flow_stat_t *p);
int omcidrv_wrapper_resetDsFlowStat(unsigned int flow);
int omcidrv_wrapper_getDsFecStat(omci_ds_fec_stat_t *p);
int omcidrv_wrapper_resetDsFecStat(void);

// bridge/vlan control
int omcidrv_wrapper_activeBdgConn(OMCI_BRIDGE_RULE_ts* pBridgeRule);
int omcidrv_wrapper_deactiveBdgConn(int srvId);
int omcidrv_wrapper_setDscpRemap(OMCI_DSCP2PBIT_ts *pDscp2PbitTable);
int omcidrv_wrapper_setMacLearnLimit(OMCI_MACLIMIT_ts *pMacLimit);
int omcidrv_wrapper_setMacFilter(OMCI_MACFILTER_ts *pMacFilter);
int omcidrv_wrapper_setGroupMacFilter(OMCI_GROUPMACFILTER_ts *pGroupMacFilter);
int omcidrv_wrapper_setSvlanTpid(omci_svlan_tpid_t *p);
int omcidrv_wrapper_getSvlanTpid(omci_svlan_tpid_t *p);
int omcidrv_wrapper_getCvlanState(unsigned int *p);
int omcidrv_wrapper_setDot1RateLimiter(omci_dot1_rate_meter_t *pDot1RateMeter);
int omcidrv_wrapper_delDot1RateLimiter(omci_dot1_rate_meter_t *pDot1RateMeter);
int omcidrv_wrapper_getBgTblPerPort(omci_bridge_tbl_per_port_t *p);
int omcidrv_wrapper_setMacAgeingTime(unsigned int ageTime);

// veip control
int omcidrv_wrapper_setUsVeipGemFlow(veipGemFlow_t *pVeipGemFlow);
int omcidrv_wrapper_delUsVeipGemFlow(veipGemFlow_t *pVeipGemFlow);
int omcidrv_wrapper_updateVeipRule(int wanIdx, int vid, int pri,
	int type, int service, int isBinding, int netIfIdx, unsigned char isRegister);

// wan control
int omcidrv_setWanStatusByIfIdx(int wanIdx, unsigned char isRuleCfg);
int omcidrv_getWanInfoByIfIdx(int wanIdx, int *vid, int *pri,
	int *type, int *service, int *isBinding, int *netIfIdx, unsigned char *isRuleCfg);
int omcidrv_updateWanInfoByProcEntry(int netIfIdx,
	int vid, int pri, int type, int service, int isBinding, int bAdd);
int omcidrv_dumpWanInfo(void);

// driver meta mgmt functions
int omcidrv_platform_register(pf_db_t *pfDb);
int omcidrv_alloc_resource(unsigned int intfNum);
void omcidrv_dealloc_resource(void);
void omcidrv_initDrvEvent(void);
void omcidrv_exitDrvEvent(void);

// feature
int omcidrv_feature_register(feature_kapi_t *p);
int omcidrv_feature_unregister(feature_kapi_t *p);
int omcidrv_feature_api(unsigned int apiID, ...);

// uni port rate
int omcidrv_wrapper_setUniPortRate(omci_port_rate_t *p);

int l2Serv_entry_add(unsigned int index, PON_GEMPORT_DIRECTION dir);
int l2Serv_entry_del(unsigned int index);
int l2Serv_entry_delAll(void);
l2_service_t* l2Serv_entry_find(unsigned int index);

int mbcastServ_entry_add(unsigned int index, OMCI_VLAN_OPER_ts *pRule, unsigned int dsStreamId);
int mbcastServ_entry_del(unsigned int index);
int mbcastServ_entry_delAll(void);
mbcast_service_t* mbcastServ_entry_find(unsigned int index);

int veipServ_entry_add(unsigned int index, OMCI_VLAN_OPER_ts *pRule, unsigned int usStreamId, unsigned int dsStreamId);
int veipServ_entry_del(unsigned int index);
int veipServ_entry_delAll(void);
veip_service_t* veipServ_entry_find(unsigned int index);

int veipGemFlow_entry_add(veipGemFlow_t *pData);
int veipGemFlow_entry_del(unsigned int gemPortId);
int veipGemFlow_entry_delAll(void);
veipGemFlow_entry_t* veipGemFlow_entry_find(unsigned int gemPortId);

int macFilter_entry_add(unsigned long long key);
int macFilter_entry_del(unsigned long long key);
macFilter_entry_t *macFilter_entry_find(unsigned long long key);


#endif
