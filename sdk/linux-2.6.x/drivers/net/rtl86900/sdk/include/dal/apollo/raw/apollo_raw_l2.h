#ifndef _APOLLO_RAW_L2_H_
#define _APOLLO_RAW_L2_H_

/****************************************************************/
/* Header File inclusion                                        */
/****************************************************************/
#include <dal/apollo/raw/apollo_raw.h>
/****************************************************************/
/* Constant Definition                                          */
/****************************************************************/
#define APOLLO_RAW_LUT_AGESPEEDMAX    0x1FFFFF
#define APOLLO_RAW_LUT_FWDPRIMAX      0x7
#define APOLLO_RAW_LUT_AGEMAX         0x7
#define APOLLO_RAW_LUT_EXTDSL_SPAMAX  0XF
#define APOLLO_RAW_LUT_L3IDXAMAX      0XFFFFFFF
#define APOLLO_RAW_LUT_EXTMBR         0x3F
#define APOLLO_RAW_LUT_DSLMBR         0xFFFF
#define APOLLO_RAW_LUT_GIPMAX         0XFFFFFF

/****************************************************************/
/* Type Definition                                              */
/****************************************************************/

typedef enum apollo_raw_l2_entryType_e
{

	RAW_LUT_ENTRY_TYPE_L2UC = 0,
    RAW_LUT_ENTRY_TYPE_L2MC_DSL,
    RAW_LUT_ENTRY_TYPE_L3MC_DSL,
    RAW_LUT_ENTRY_TYPE_L3MC_ROUTE,
    RAW_LUT_ENTRY_TYPE_END,

}apollo_raw_l2_entryType_t;

typedef enum apollo_raw_l2_saCtlAct_e
{
	RAW_LUT_SACTL_ACT_FORWARD = 0,
	RAW_LUT_SACTL_ACT_DROP,
	RAW_LUT_SACTL_ACT_TRAP,
	RAW_LUT_SACTL_ACT_COPY,
	RAW_LUT_SACTL_ACT_END,
}apollo_raw_l2_saCtlAct_t;


typedef enum apollo_raw_l2_commonAct_e
{
	RAW_L2_COMMON_ACT_FORWARD = 0,
	RAW_L2_COMMON_ACT_DROP,
	RAW_L2_COMMON_ACT_TRAP,
	RAW_L2_COMMON_ACT_COPY2CPU,
	RAW_L2_COMMON_ACT_END,
}apollo_raw_l2_commonAct_t;


typedef enum apollo_raw_l2_unknMcAct_e
{
	RAW_L2_UNKNMC_ACT_FORWARD = 0,
	RAW_L2_UNKNMC_ACT_DROP,
	RAW_L2_UNKNMC_ACT_TRAP,
	RAW_L2_UNKNMC_ACT_EXCLUDE_RMA,
	RAW_L2_UNKNMC_ACT_END,
}apollo_raw_l2_unknMcAct_t;

typedef enum apollo_raw_l2_readMethod_e
{
	RAW_LUT_READ_METHOD_MAC = 0,
	RAW_LUT_READ_METHOD_ADDRESS,
	RAW_LUT_READ_METHOD_NEXT_ADDRESS,
	RAW_LUT_READ_METHOD_NEXT_L2UC,
	RAW_LUT_READ_METHOD_NEXT_L2MC,
	RAW_LUT_READ_METHOD_NEXT_L3MC,
	RAW_LUT_READ_METHOD_NEXT_L2L3MC,
	RAW_LUT_READ_METHOD_NEXT_L2UCSPA,
	RAW_LUT_READ_METHOD_END,
}apollo_raw_l2_readMethod_t;

typedef enum apollo_raw_l2_flushStatus_e
{
    RAW_FLUSH_STATUS_NONBUSY =0,
    RAW_FLUSH_STATUS_BUSY,
    RAW_FLUSH_STATUS_END,
}apollo_raw_l2_flushStatus_t;

typedef enum apollo_raw_l2_flushMode_e
{
	RAW_FLUSH_MODE_PORT = 0,
	RAW_FLUSH_MODE_VID,
	RAW_FLUSH_MODE_FID,
	RAW_FLUSH_MODE_END,
}apollo_raw_l2_flushMode_t;

typedef enum apollo_raw_l2_flushType_e
{
	RAW_FLUSH_TYPE_DYNAMIC = 0,
	RAW_FLUSH_TYPE_STATIC ,
	RAW_FLUSH_TYPE_BOTH,
	RAW_FLUSH_TYPE_END,
}apollo_raw_l2_flushType_t;

typedef enum apollo_raw_l2_ipMcLookupOp_e
{
    RAW_LUT_IPMCLOOKUP_TYPE_DIPSIP = 0,
    RAW_LUT_IPMCLOOKUP_TYPE_DIP,
    RAW_LUT_IPMCLOOKUP_TYPE_END,
}apollo_raw_l2_ipMcLookupOp_t;

typedef enum apollo_raw_l2_ipMcHashType_e
{
   RAW_LUT_IPMCHASH_TYPE_DMACFID=0,
   RAW_LUT_IPMCHASH_TYPE_DIPSIP,
   RAW_LUT_IPMCHASH_TYPE_GIPVID,
   RAW_LUT_IPMCHASH_TYPE_END,
}apollo_raw_l2_ipMcHashType_t;

typedef enum apollo_raw_l2_cfg_enable_e
{
   RAW_LUT_CFG_ENABLE=0,
   RAW_LUT_CFG_DISABLE,
   RAW_LUT_CFG_END,
}apollo_raw_l2_cfg_enable_t;

typedef enum apollo_raw_l2_flood_enable_e
{
   RAW_LUT_FLOOD_DROP=0,
   RAW_LUT_FLOOD_FLOODING,
   RAW_LUT_FLOO_END,
}apollo_raw_l2_flood_enable_t;

typedef enum apollo_raw_l2_McAllow_e
{
	RAW_L2_MC_DROP = 0,
    RAW_L2_MC_ALLOW,
    RAW_L2_MC_END,
}apollo_raw_l2_McAllow_t;

typedef enum apollo_raw_l2_l2HashType_e
{
	RAW_L2_HASH_SVL = 0,
    RAW_L2_HASH_IVL,
    RAW_L2_HASH_END
}apollo_raw_l2_l2HashType_t;



typedef struct apollo_raw_flush_ctrl_s
{
    apollo_raw_l2_flushStatus_t flushStatus;
    apollo_raw_l2_flushMode_t flushMode;
    apollo_raw_l2_flushType_t flushType;
    rtk_vlan_t vid;
    rtk_fid_t fid;
}apollo_raw_flush_ctrl_t;

typedef struct apollo_raw_l2_cfg_s
{
    apollo_raw_l2_ipMcLookupOp_t ipMcLookupOpType;
    apollo_raw_l2_ipMcHashType_t ipMcHashType;
    apollo_raw_l2_cfg_enable_t linkDownAgeout;
    apollo_raw_l2_cfg_enable_t bcamDis;
    uint32 ageSpeed;
}apollo_raw_l2_cfg_t;

typedef struct apollo_raw_l2_table_s
{
    apollo_raw_l2_readMethod_t method;
    /*s/w information*/
    apollo_raw_l2_entryType_t table_type;
    uint32 lookup_hit;
    uint32 lookup_busy;
    uint32 address;
    uint32 wait_time;


    /*common part*/
	uint32 l3lookup;
    uint32 lut_pri;
	uint32 fwdpri_en;
    uint32 nosalearn;
    uint32 valid;

    /*----L2----*/
    rtk_mac_t mac;
	uint32 cvid_fid;
    apollo_raw_l2_l2HashType_t ivl_svl;

    /*l2 uc*/
    uint32 fid;
    uint32 efid;
	uint32 sapri_en;
    uint32 spa;
	uint32 age;
    uint32 auth;
    uint32 sa_block;
	uint32 da_block;
	uint32 ext_dsl_spa;
    uint32 arp_used;

    /*----L3----*/
    ipaddr_t gip;
    uint32 gip_only;
	uint32 wan_sa;


    /* ---L3 MC DSL---*/
    uint32 sip_vid;

    /* ---L3 MC ROUTE---*/
    uint32 l3_idx;
    uint32 ext_fr;


    /*(L2 MC DSL)(L3 MC DSL)(L3 MC ROUTE)*/
	uint32 mbr;
    uint32 ext_mbr;
    uint32 dsl_mbr;



}apollo_lut_table_t;

extern int32 apollo_raw_l2_lookUpTb_set(apollo_lut_table_t *pL2Table);
extern int32 apollo_raw_l2_lookUpTb_get(apollo_lut_table_t *pL2Table);
extern int32 apollo_raw_l2_flushEn_set(rtk_port_t port, rtk_enable_t enabled);
extern int32 apollo_raw_l2_flushEn_get(rtk_port_t port, rtk_enable_t *pEnabled);
extern int32 apollo_raw_l2_flushCtrl_set(apollo_raw_flush_ctrl_t *pCtrl);
extern int32 apollo_raw_l2_flushCtrl_get(apollo_raw_flush_ctrl_t *pCtrl);
extern int32 apollo_raw_l2_unMatched_saCtl_set(rtk_port_t port, rtk_action_t action);
extern int32 apollo_raw_l2_unMatched_saCtl_get(rtk_port_t port, rtk_action_t* pAction);
extern int32 apollo_raw_l2_unkn_saCtl_set(rtk_port_t port, rtk_action_t action);
extern int32 apollo_raw_l2_unkn_saCtl_get(rtk_port_t port, rtk_action_t* pAction);
extern int32 apollo_raw_l2_unkn_ucDaCtl_set(rtk_port_t port, rtk_action_t action);
extern int32 apollo_raw_l2_unkn_ucDaCtl_get(rtk_port_t port, rtk_action_t* pAction);
extern int32 apollo_raw_l2_learnOverAct_set(rtk_port_t port, rtk_action_t action);
extern int32 apollo_raw_l2_learnOverAct_get(rtk_port_t port, rtk_action_t* pAction);
extern int32 apollo_raw_l2_agingEnable_set(rtk_port_t port, rtk_enable_t enable);
extern int32 apollo_raw_l2_agingEnable_get(rtk_port_t port, rtk_enable_t *pEnable);
extern int32 apollo_raw_l2_lrnLimitNo_set(rtk_port_t port, uint32 num);
extern int32 apollo_raw_l2_lrnLimitNo_get(rtk_port_t port, uint32 *pNum);
extern int32 apollo_raw_l2_lrnCnt_get(rtk_port_t port, uint32 *pNum);
extern int32 apollo_raw_l2_lrnOverSts_set(rtk_port_t port, uint32 value);
extern int32 apollo_raw_l2_lrnOverSts_get(rtk_port_t port, uint32 *pValue);
extern int32 apollo_raw_l2_sysLrnOverAct_set(rtk_action_t action);
extern int32 apollo_raw_l2_sysLrnOverAct_get(rtk_action_t* pAction);
extern int32 apollo_raw_l2_sysLrnLimitNo_set(uint32 num);
extern int32 apollo_raw_l2_sysLrnLimitNo_get(uint32* pNum);
extern int32 apollo_raw_l2_sysLrnOverSts_set( uint32 value);
extern int32 apollo_raw_l2_sysLrnOverSts_get( uint32 *pValue);
extern int32 apollo_raw_l2_SysLrnCnt_get( uint32 *pNum);
extern int32 apollo_raw_unkn_l2Mc_set(rtk_port_t port, rtk_action_t action);
extern int32 apollo_raw_unkn_l2Mc_get(rtk_port_t port, rtk_action_t* pAction);
extern int32 apollo_raw_unkn_ip4Mc_set(rtk_port_t port, rtk_action_t action);
extern int32 apollo_raw_unkn_ip4Mc_get(rtk_port_t port, rtk_action_t* pAction);
extern int32 apollo_raw_unkn_ip6Mc_set(rtk_port_t port, rtk_action_t action);
extern int32 apollo_raw_unkn_ip6Mc_get(rtk_port_t port, rtk_action_t* pAction);
extern int32 apollo_raw_unkn_mcPri_set(uint32 prity);
extern int32 apollo_raw_unkn_mcPri_get(uint32* pPrity);
extern int32 apollo_raw_l2_bcFlood_set(rtk_port_t port, rtk_enable_t enabled);
extern int32 apollo_raw_l2_bcFlood_get(rtk_port_t port, rtk_enable_t *pEnabled);
extern int32 apollo_raw_l2_unknUcFlood_set(rtk_port_t port, rtk_enable_t enabled);
extern int32 apollo_raw_l2_unknUcFlood_get(rtk_port_t port, rtk_enable_t *pEnabled);
extern int32 apollo_raw_l2_unknMcFlood_set(rtk_port_t port, rtk_enable_t enabled);
extern int32 apollo_raw_l2_unknMcFlood_get(rtk_port_t port, rtk_enable_t *pEnabled);
extern int32 apollo_raw_l2_efid_set(rtk_port_t port, rtk_fid_t efid);
extern int32 apollo_raw_l2_efid_get(rtk_port_t port, rtk_fid_t* pEfid);
extern int32 apollo_raw_l2_ipmcAction_set(rtk_port_t port, rtk_action_t action);
extern int32 apollo_raw_l2_ipmcAction_get(rtk_port_t port, rtk_action_t *pAction);
extern int32 apollo_raw_l2_igmp_Mc_table_set(uint32 index, rtk_ip_addr_t dip, uint32 portmask);
extern int32 apollo_raw_l2_igmp_Mc_table_get(uint32 index, rtk_ip_addr_t *pDip, uint32 *pPortmask);

extern int32 apollo_raw_l2_camEnable_set(rtk_enable_t enable);
extern int32 apollo_raw_l2_camEnable_get(rtk_enable_t *pEnable);
extern int32 apollo_raw_l2_agingTime_set(uint32 agingTime);
extern int32 apollo_raw_l2_agingTime_get(uint32 *pAgingTime);
extern int32 apollo_raw_l2_ipmcLookupOp_set(apollo_raw_l2_ipMcLookupOp_t type);
extern int32 apollo_raw_l2_ipmcLookupOp_get(apollo_raw_l2_ipMcLookupOp_t *pType);
extern int32 apollo_raw_l2_ipmcHashType_set(apollo_raw_l2_ipMcHashType_t type);
extern int32 apollo_raw_l2_ipmcHashType_get(apollo_raw_l2_ipMcHashType_t *pType);
extern int32 apollo_raw_l2_flushLinkDownPortAddrEnable_set(rtk_enable_t enable);
extern int32 apollo_raw_l2_flushLinkDownPortAddrEnable_get(rtk_enable_t *pEnable);



#endif /*#ifndef _APOLLO_RAW_L2_H_*/

