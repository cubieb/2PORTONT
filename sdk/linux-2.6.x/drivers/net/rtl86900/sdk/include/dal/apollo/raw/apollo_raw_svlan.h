#ifndef _APOLLO_RAW_SVLAN_H_
#define _APOLLO_RAW_SVLAN_H_

/****************************************************************/
/* Header File inclusion                                        */
/****************************************************************/
#include <dal/apollo/raw/apollo_raw.h>
#include <rtk/svlan.h>
/****************************************************************/
/* Constant Definition                                          */
/****************************************************************/
#define APOLLO_SVLAN_MC2S_INDEX_MAX 	0x7
#define APOLLO_SVLAN_C2S_INDEX_MAX 		127
#define APOLLO_SVLAN_SP2C_INDEX_MAX 	127


/****************************************************************/
/* Type Definition                                              */
/****************************************************************/

typedef enum apollo_raw_svlan_lookupType_e
{
    RAW_SVLAN_LOOKUP_S64MBRCGF  = 0,
    RAW_SVLAN_LOOKUP_C4KVLAN,
    RAW_SVLAN_LOOKUP_END,

} apollo_raw_svlan_lookupType_t;


typedef enum apollo_raw_svlan_priSel_e
{
    RAW_SVLAN_PRISEL_INTERNAL_PRI  = 0,
    RAW_SVLAN_PRISEL_1QTAG_PRI,
    RAW_SVLAN_PRISEL_VSPRI,
    RAW_SVLAN_PRISEL_PBPRI,
    RAW_SVLAN_PRISEL_END,    

} apollo_raw_svlan_priSel_t;

typedef enum apollo_raw_svlan_action_e
{
    RAW_SVLAN_ACTION_DROP  = 0,
    RAW_SVLAN_ACTION_TRAP,
    RAW_SVLAN_ACTION_SVLAN,
    RAW_SVLAN_ACTION_SVLAN_AND_KEEP,
    RAW_SVLAN_ACTION_END,    

} apollo_raw_svlan_action_t;


typedef enum apollo_raw_svlan_mc2sfmt_e
{
    RAW_SVLAN_MC2S_FMT_MAC  = 0,
    RAW_SVLAN_MC2S_FMT_IP,
    RAW_SVLAN_MC2S_FMT_END,    

} apollo_raw_svlan_mc2sfmt_t;



typedef struct apollo_raw_svlan_mbrCfg_s
{
    uint32              idx;
    apollo_raw_svidx_t  svid;
    rtk_portmask_t      mbr;
    rtk_portmask_t	    untagset;
    rtk_pri_t           spri;
    rtk_enable_t        fid_en;
    rtk_fid_t           fid_msti;
    rtk_enable_t        efid_en;
    uint32              efid;
	
} apollo_raw_svlan_mbrCfg_t;


typedef struct apollo_raw_svlan_mc2sCfg_s
{
    uint32                      idx;
    apollo_raw_svidx_t          svidx;
    apollo_raw_svlan_mc2sfmt_t  format;
    uint32                      data;
    uint32                      mask;
    rtk_enable_t                valid;
	
} apollo_raw_svlan_mc2sCfg_t;

typedef struct apollo_raw_svlan_c2sCfg_s
{
    uint32                  idx;
    apollo_raw_svidx_t      svidx;
    rtk_vlan_t  		    evid;
    rtk_portmask_t          pmsk;
	
} apollo_raw_svlan_c2sCfg_t;

typedef struct apollo_raw_svlan_sp2cCfg_s
{
    uint32              idx;
    apollo_raw_svidx_t  svidx;
    rtk_vlan_t  	    vid;
    rtk_port_t 		    port;
    rtk_enable_t 		valid;
	
} apollo_raw_svlan_sp2cCfg_t;




int32 apollo_raw_svlan_portUplinkEnable_set(rtk_port_t port, rtk_enable_t enable);
int32 apollo_raw_svlan_portUplinkEnable_get(rtk_port_t port, rtk_enable_t *pEnable);
int32 apollo_raw_svlan_port1tonVlanEnable_set(rtk_port_t port, rtk_enable_t enable);
int32 apollo_raw_svlan_port1tonVlanEnable_get(rtk_port_t port, rtk_enable_t *pEnable);
int32 apollo_raw_svlan_portSvlan_set(rtk_port_t port, apollo_raw_svidx_t svidx);
int32 apollo_raw_svlan_portSvlan_get(rtk_port_t port, apollo_raw_svidx_t *pSvidx);
int32 apollo_raw_svlan_lookupType_set(rtk_svlan_lookupType_t type);
int32 apollo_raw_svlan_lookupType_get(rtk_svlan_lookupType_t *pType);
int32 apollo_raw_svlan_tpid_set(apollo_raw_ethertype_t svlanTpid);
int32 apollo_raw_svlan_tpid_get(apollo_raw_ethertype_t *pSvlanTpid);
int32 apollo_raw_svlan_cfiKeepEnable_set(rtk_enable_t enable);
int32 apollo_raw_svlan_cfiKeepEnable_get(rtk_enable_t *pEnable);
int32 apollo_raw_svlan_trapPri_set(rtk_pri_t priority);
int32 apollo_raw_svlan_trapPri_get(rtk_pri_t *pPriority);
int32 apollo_raw_svlan_egrPriSel_set(rtk_svlan_priSel_t mode);
int32 apollo_raw_svlan_egrPriSel_get(rtk_svlan_priSel_t *pMode);
int32 apollo_raw_svlan_untagAction_set(rtk_svlan_action_t action);
int32 apollo_raw_svlan_untagAction_get(rtk_svlan_action_t *pAction);
int32 apollo_raw_svlan_unmatchAction_set(rtk_svlan_action_t action);
int32 apollo_raw_svlan_unmatchAction_get(rtk_svlan_action_t *pAction);
int32 apollo_raw_svlan_untagSvidx_set(apollo_raw_svidx_t svidx);
int32 apollo_raw_svlan_untagSvidx_get(apollo_raw_svidx_t *pSvidx);
int32 apollo_raw_svlan_unmatchSvidx_set(apollo_raw_svidx_t svidx);
int32 apollo_raw_svlan_unmatchSvidx_get(apollo_raw_svidx_t *pSvidx);
int32 apollo_raw_svlan_mrbCfg_set(apollo_raw_svlan_mbrCfg_t *pMbrCfg);
int32 apollo_raw_svlan_mrbCfg_get(apollo_raw_svlan_mbrCfg_t *pMbrCfg);
int32 apollo_raw_svlan_mc2sCfg_set(apollo_raw_svlan_mc2sCfg_t *pMc2sCfg);
int32 apollo_raw_svlan_mc2sCfg_get(apollo_raw_svlan_mc2sCfg_t *pMc2sCfg);
int32 apollo_raw_svlan_c2sCfg_set(apollo_raw_svlan_c2sCfg_t *pC2sCfg);
int32 apollo_raw_svlan_c2sCfg_get(apollo_raw_svlan_c2sCfg_t *pC2sCfg);
int32 apollo_raw_svlan_sp2cCfg_set(apollo_raw_svlan_sp2cCfg_t *pSp2cCfg);
int32 apollo_raw_svlan_sp2cCfg_get(apollo_raw_svlan_sp2cCfg_t *pSp2cCfg);

#endif /*#ifndef _APOLLO_RAW_SVLAN_H_*/

