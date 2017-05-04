#ifndef _APOLLO_RAW_TRAP_H_
#define _APOLLO_RAW_TRAP_H_

/****************************************************************/
/* Header File inclusion                                        */
/****************************************************************/
#include <dal/apollo/raw/apollo_raw.h>
/****************************************************************/
/* Constant Definition                                          */
/****************************************************************/


/****************************************************************/
/* Type Definition                                              */
/****************************************************************/
#define APOLLO_RAW_RMA_MAXPRI 0x7
#define APOLLO_RAW_RMA_L2TAILNUM_MAX  0x2f

typedef enum apollo_raw_rmaAction_e
{
    RAW_RMA_ACT_FORWARD  = 0,
    RAW_RMA_ACT_TRAP ,
    RAW_RMA_ACT_DROP ,
    RAW_RMA_ACT_FORWARD_EXCLUDE_CPU ,
    RAW_RMA_ACT_END ,
} apollo_raw_rmaAction_t;

typedef enum apollo_raw_rmaFeature_e
{
    RAW_RMA_FUN_VLAN_LEAKY = 0,
    RAW_RMA_FUN_PISO_LEAKY,
    RAW_RMA_FUN_BYPASS_STORM,
    RAW_RMA_FUN_KEEP_CTGA_FMT,
    RAW_RMA_FUN_END,
} apollo_raw_rmaFeature_t;

typedef enum apollo_raw_igmpmld_type_e
{
    RAW_TYPE_IGMPV1 = 0,
    RAW_TYPE_IGMPV2,
    RAW_TYPE_IGMPV3,
    RAW_TYPE_MLDV1,
    RAW_TYPE_MLDV2,
    RAW_TYPE_END,

} apollo_raw_igmpmld_type_t;

typedef enum apollo_raw_igmpAction_e
{
    RAW_IGMP_ACT_FORWARD  = 0,
    RAW_IGMP_ACT_DROP,
    RAW_IGMP_ACT_TRAP,
     RAW_IGMP_ACT_END,
} apollo_raw_igmpAction_t;

typedef enum apollo_raw_igmpChecksumErrAction_e
{
    RAW_CHECKSUM_ERR_FORWARD = 0,
    RAW_CHECKSUM_ERR_DROP,
    RAW_CHECKSUM_ERR_TRAP,
    RAW_CHECKSUM_ERR_END,
}apollo_raw_igmpChecksumErrAction_t;

extern int32 apollo_raw_trap_rmaPriority_set(uint32 priority);
extern int32 apollo_raw_trap_rmaPriority_get(uint32 *pPriority);
extern int32 apollo_raw_trap_rmaAction_set(uint32  rmaTail, rtk_action_t action);
extern int32 apollo_raw_trap_rmaAction_get(uint32 rmaTail, rtk_action_t* pAction);
extern int32 apollo_raw_trap_rmaFeature_set(uint32  rmaTail, apollo_raw_rmaFeature_t type, rtk_enable_t enable);
extern int32 apollo_raw_trap_rmaFeature_get(uint32  rmaTail, apollo_raw_rmaFeature_t type, rtk_enable_t *pEnable);
extern int32 apollo_raw_trap_rmaCiscoAction_set(uint32  rmaTail, rtk_action_t action);
extern int32 apollo_raw_trap_rmaCiscoAction_get(uint32  rmaTail, rtk_action_t *pAction);
extern int32 apollo_raw_trap_rmaCiscoFeature_set(uint32  rmaTail, apollo_raw_rmaFeature_t type, rtk_enable_t enable);
extern int32 apollo_raw_trap_rmaCiscoFeature_get(uint32  rmaTail, apollo_raw_rmaFeature_t type, rtk_enable_t *pEnable);

extern int32 apollo_raw_trap_igmpAction_set(rtk_port_t port, apollo_raw_igmpmld_type_t type, rtk_action_t action);
extern int32 apollo_raw_trap_igmpAction_get(rtk_port_t port, apollo_raw_igmpmld_type_t type, rtk_action_t *pAction);
extern int32 apollo_raw_trap_igmpIsoLeaky_set(rtk_enable_t state);
extern int32 apollo_raw_trap_igmpIsoLeaky_get(rtk_enable_t *pState);
extern int32 apollo_raw_trap_igmpVLANLeaky_set(rtk_enable_t state);
extern int32 apollo_raw_trap_igmpVLANLeaky_get(rtk_enable_t *pState);
extern int32 apollo_raw_trap_igmpBypassStrom_set(rtk_enable_t state);
extern int32 apollo_raw_trap_igmpBypassStrom_get(rtk_enable_t *pState);
extern int32 apollo_raw_trap_igmpChechsumError_set(rtk_action_t action);
extern int32 apollo_raw_trap_igmpChechsumError_get(rtk_action_t *pAction);

#endif /*#ifndef _APOLLO_RAW_TRAP_H_*/

