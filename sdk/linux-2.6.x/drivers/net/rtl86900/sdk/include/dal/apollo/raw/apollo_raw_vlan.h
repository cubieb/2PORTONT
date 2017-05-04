#ifndef _APOLLO_ASICDRV_VLAN_H_
#define _APOLLO_ASICDRV_VLAN_H_

/****************************************************************/
/* Header File inclusion                                        */
/****************************************************************/
#include <dal/apollo/raw/apollo_raw.h>
/****************************************************************/
/* Constant Definition                                          */
/****************************************************************/
#define APOLLO_PROTOVLAN_GIDX_MAX 3
#define APOLLO_PROTOVLAN_GROUPNO  4


/****************************************************************/
/* Type Definition                                              */
/****************************************************************/

typedef struct  apollo_raw_vlanconfig_s
{
    uint32 	index;
    uint32 	evid;
    rtk_portmask_t 	mbr;
    rtk_portmask_t 	dslMbr;
    rtk_portmask_t 	exMbr;
    uint32  fid_msti;
    uint32  envlanpol;
    uint32  meteridx;
    uint32  vbpen;
    uint32  vbpri;
}apollo_raw_vlanconfig_t;



typedef enum apollo_raw_vlan_l2HashType_e
{
	RAW_VLAN_HASH_SVL = 0,
    RAW_VLAN_HASH_IVL,
    RAW_VLAN_HASH_END
}apollo_raw_vlan_l2HashType_t;



typedef struct  apollo_raw_vlan4kentry_s{

    uint32 	vid;
    rtk_portmask_t mbr;
    rtk_portmask_t untag;
    rtk_portmask_t dslMbr;
    rtk_portmask_t exMbr;
    uint32  fid_msti;
    uint32  envlanpol;
    uint32  meteridx;
    uint32  vbpen;
    uint32  vbpri;
    apollo_raw_vlan_l2HashType_t ivl_svl;
}apollo_raw_vlan4kentry_t;



typedef enum apollo_raw_protoVlanFrameType_e
{
    RAW_PPVLAN_FRAME_TYPE_ETHERNET = 0,
    RAW_PPVLAN_FRAME_TYPE_LLC,
    RAW_PPVLAN_FRAME_TYPE_RFC1042,
    RAW_PPVLAN_FRAME_TYPE_END
} apollo_raw_protoVlanFrameType_t;



typedef enum apollo_raw_vlan_cfiKeepMode_e
{
    RAW_CFI_ALWAYS_0 = 0,
    RAW_CFI_KEEP_INGRESS,
    RAW_CFI_KEEP_TYPE_END
} apollo_raw_vlan_cfiKeepMode_t;


typedef enum apollo_raw_vlanAcpFrameType_e
{
    RAW_VLAN_ACCEPT_ALL = 0,
    RAW_VLAN_ACCEPT_TAGGED_ONLY,
    RAW_VLAN_ACCEPT_UNTAGGED_ONLY,
    RAW_VLAN_ACCEPT_1Q_1P_TAGGED_ONLY,
    RAW_VLAN_ACCEPT_TYPE_END
} apollo_raw_vlanAcpFrameType_t;


typedef enum apollo_raw_vlanTag_e
{
    RAW_VLAN_UNTAG = 0,
    RAW_VLAN_TAG,
    RAW_VLAN_TAG_TYPE_END
} apollo_raw_vlanTag_t;




typedef enum apollo_raw_vlan_egrTagMode_e
{
    RAW_VLAN_EGR_TAG_MODE_ORI = 0,
    RAW_VLAN_EGR_TAG_MODE_KEEP,
    RAW_VLAN_EGR_TAG_MODE_PRI_TAG,
    RAW_VLAN_EGR_TAG_MODE_END
} apollo_raw_vlan_egrTagMode_t;



typedef struct apollo_raw_protoValnEntryCfg_s
{
    uint32                           index;
    apollo_raw_protoVlanFrameType_t  frameType;
    uint32                           etherType;
} apollo_raw_protoValnEntryCfg_t;

typedef struct apollo_raw_protoVlanCfg_s
{
    uint32 index;
    uint32 valid;
    uint32 vlan_idx;
    uint32 priority;
} apollo_raw_protoVlanCfg_t;


extern int32 apollo_raw_vlan_memberConfig_set(apollo_raw_vlanconfig_t *pVlanCg);
extern int32 apollo_raw_vlan_memberConfig_get(apollo_raw_vlanconfig_t *pVlanCg);

extern int32 apollo_raw_vlan_4kEntry_set(apollo_raw_vlan4kentry_t *pVlan4kEntry );
extern int32 apollo_raw_vlan_4kEntry_get(apollo_raw_vlan4kentry_t *pVlan4kEntry );

extern int32 apollo_raw_vlan_protoAndPortBasedEntry_set(rtk_port_t port, apollo_raw_protoVlanCfg_t *pPpbCfg);
extern int32 apollo_raw_vlan_portAndProtoBasedEntry_get(rtk_port_t port, apollo_raw_protoVlanCfg_t *pPpbCfg);

extern int32 apollo_raw_vlanProtoAndPortBasedEntry_get(apollo_raw_protoValnEntryCfg_t *entry);
extern int32 apollo_raw_vlanProtoAndPortBasedEntry_set(apollo_raw_protoValnEntryCfg_t *entry);


extern int32 apollo_raw_vlan_portBasedVID_set(rtk_port_t port, uint32 index, uint32 pri);
extern int32 apollo_raw_vlan_portBasedVID_get(rtk_port_t port, uint32 *pIndex, uint32 *pPri);
extern int32 apollo_raw_vlan_cfiKeepMode_get(apollo_raw_vlan_cfiKeepMode_t *mode);
extern int32 apollo_raw_vlan_cfiKeepMode_set(apollo_raw_vlan_cfiKeepMode_t mode);
extern int32 apollo_raw_vlan_acceptFrameType_get(rtk_port_t port, apollo_raw_vlanAcpFrameType_t *type);
extern int32 apollo_raw_vlan_acceptFrameType_set(rtk_port_t port, apollo_raw_vlanAcpFrameType_t type);
extern int32 apollo_raw_vlan_igrFlterEnable_set(rtk_port_t port, rtk_enable_t enable);
extern int32 apollo_raw_vlan_igrFlterEnable_get(rtk_port_t port, rtk_enable_t *pEnable);
extern int32 apollo_raw_vlan_egrTagMode_set(rtk_port_t port, apollo_raw_vlan_egrTagMode_t mode);
extern int32 apollo_raw_vlan_egrTagMode_get(rtk_port_t port, apollo_raw_vlan_egrTagMode_t *pMode);


extern int32 apollo_raw_vlan_vid0TagType_set(apollo_raw_vlanTag_t type);
extern int32 apollo_raw_vlan_vid0TagType_get(apollo_raw_vlanTag_t *type);
extern int32 apollo_raw_vlan_vid4095TagType_set(apollo_raw_vlanTag_t type);
extern int32 apollo_raw_vlan_vid4095TagType_get(apollo_raw_vlanTag_t *type);

extern int32 apollo_raw_vlan_transparentEnable_set(rtk_enable_t enable);
extern int32 apollo_raw_vlan_transparentEnable_get(rtk_enable_t *pEnable);
extern int32 apollo_raw_vlan_egrIgrFilterEnable_set(rtk_enable_t enable);
extern int32 apollo_raw_vlan_egrIgrFilterEnable_get(rtk_enable_t *pEnable);


extern int32 apollo_raw_vlan_portBasedFidEnable_set(rtk_port_t port, rtk_enable_t enable);
extern int32 apollo_raw_vlan_portBasedFidEnable_get(rtk_port_t port, rtk_enable_t *pEnable);
extern int32 apollo_raw_vlan_portBasedFid_get(rtk_port_t port, uint32 *fid);
extern int32 apollo_raw_vlan_portBasedFid_set(rtk_port_t port, uint32 fid);

extern int32 apollo_raw_vlan_portBasedPri_set(rtk_port_t port, uint32 pri);
extern int32 apollo_raw_vlan_portBasedPri_get(rtk_port_t port, uint32 *pPri);

extern int32 apollo_raw_vlan_portToMbrCfgIdx_set(rtk_port_t port, uint32 mbrCfgIdx);
extern int32 apollo_raw_vlan_portToMbrCfgIdx_get(rtk_port_t port, uint32 *pMbrCfgIdx);
extern int32 apollo_raw_vlan_vcPortToMbrCfgIdx_set(rtk_port_t port, uint32 mbrCfgIdx);
extern int32 apollo_raw_vlan_vcPortToMbrCfgIdx_get(rtk_port_t port, uint32 *pMbrCfgIdx);
extern int32 apollo_raw_vlan_extPortToMbrCfgIdx_set(rtk_port_t port, uint32 mbrCfgIdx);
extern int32 apollo_raw_vlan_extPortToMbrCfgIdx_get(rtk_port_t port, uint32 *pMbrCfgIdx);

extern int32 apollo_raw_vlan_egrKeepPmsk_get(rtk_port_t port, rtk_portmask_t *pPortMsk);
extern int32 apollo_raw_vlan_egrKeepPmsk_set(rtk_port_t port, rtk_portmask_t portMsk);

extern int32 apollo_raw_vlan_ipmcastLeaky_set(rtk_port_t port, rtk_enable_t enable);
extern int32 apollo_raw_vlan_ipmcastLeaky_get(rtk_port_t port, rtk_enable_t *pEnable);



#endif /*#ifndef _APOLLO_ASICDRV_VLAN_H_*/

