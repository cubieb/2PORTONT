#ifndef _APOLLO_RAW_RLDP_H_
#define _APOLLO_RAW_RLDP_H_

/****************************************************************/
/* Header File inclusion                                        */
/****************************************************************/
#include <dal/apollo/raw/apollo_raw.h>
/****************************************************************/
/* Constant Definition                                          */
/****************************************************************/

#define APOLLO_RLDP_PERIOD_MAX 	0xFFFF
#define APOLLO_RLDP_RESEND_CNT_MAX 	0xFF

/****************************************************************/
/* Type Definition                                              */
/****************************************************************/

typedef enum apollo_raw_rldp_runoutDscAct_e
{
    RAW_RLDP_RUNOUT_FORWARD = 0,
    RAW_RLDP_RUNOUT_DROP,
    RAW_RLDP_RUNOUT_ACTION_END
}apollo_raw_rldp_runoutDscAct_t;

typedef enum apollo_raw_rldp_triggerMode_e
{
    RAW_RLDP_TRIG_SAMOVING = 0,
    RAW_RLDP_TRIG_PERIODIC,
    RAW_RLDP_TRIG_END
}apollo_raw_rldp_triggerMode_t;


typedef enum apollo_raw_rldp_indSrc_e
{
    RAW_RLDP_INDSRC_ASIC = 0,
    RAW_RLDP_INDSRC_CPU,
    RAW_RLDP_INDSRC_END
}apollo_raw_rldp_indSrc_t;

typedef enum apollo_raw_rldp_handle_e
{
    RAW_RLDP_HANDLE_ASIC = 0,
    RAW_RLDP_HANDLE_CPU,
    RAW_RLDP_HANDLE_END
}apollo_raw_rldp_handle_t;

typedef enum apollo_raw_rldp_compare_e
{
    RAW_RLDP_CMP_SA_MAG = 0,
    RAW_RLDP_CMP_SA_MAG_IDF,
    RAW_RLDP_CMP_END
}apollo_raw_rldp_compare_t;

typedef enum apollo_raw_rldp_loopState_e
{
    RAW_RLDP_NORMAL = 0,
    RAW_RLDP_LOOPING,
    RAW_RLDP_LOOP_STATE_END
}apollo_raw_rldp_loopState_t;



extern int32 apollo_raw_rldp_dscRunout_set(apollo_raw_rldp_runoutDscAct_t action);
extern int32 apollo_raw_rldp_dscRunout_get(apollo_raw_rldp_runoutDscAct_t *pAction);
extern int32 apollo_raw_rldp_trigger_set(apollo_raw_rldp_triggerMode_t mode);
extern int32 apollo_raw_rldp_trigger_get(apollo_raw_rldp_triggerMode_t *pMode);
extern int32 apollo_raw_rldp_enable_set(rtk_enable_t mode);
extern int32 apollo_raw_rldp_enable_get(rtk_enable_t *pMode);
extern int32 apollo_raw_rldp_indicateSrc_set(apollo_raw_rldp_indSrc_t mode);
extern int32 apollo_raw_rldp_indicateSrc_get(apollo_raw_rldp_indSrc_t *pMode);
extern int32 apollo_raw_rldp_handle_set(apollo_raw_rldp_handle_t mode);
extern int32 apollo_raw_rldp_handle_get(apollo_raw_rldp_handle_t *pMode);
extern int32 apollo_raw_rldp_compare_set(apollo_raw_rldp_compare_t mode);
extern int32 apollo_raw_rldp_compare_get(apollo_raw_rldp_compare_t *pMode);
extern int32 apollo_raw_rldp_genRandom_set(rtk_enable_t enable);
extern int32 apollo_raw_rldp_randomNum_get(rtk_mac_t *pRand);
extern int32 apollo_raw_rldp_magicNum_set(rtk_mac_t *pMagic);
extern int32 apollo_raw_rldp_magicNum_get(rtk_mac_t *pMagic);
extern int32 apollo_raw_rldp_checkStatePeriod_set(uint32 period);
extern int32 apollo_raw_rldp_checkStatePeriod_get(uint32 *pPeriod);
extern int32 apollo_raw_rldp_checkStateCnt_set(uint32 count);
extern int32 apollo_raw_rldp_checkStateCnt_get(uint32 *pCount);
extern int32 apollo_raw_rldp_loopStatePeriod_set(uint32 period);
extern int32 apollo_raw_rldp_loopStatePeriod_get(uint32 *pPeriod);
extern int32 apollo_raw_rldp_loopStateCnt_set(uint32 count);
extern int32 apollo_raw_rldp_loopStateCnt_get(uint32 *pCount);
extern int32 apollo_raw_rldp_portEnable_set(rtk_port_t port,  rtk_enable_t enable);
extern int32 apollo_raw_rldp_portEnable_get(rtk_port_t port,  rtk_enable_t *pEnable);
extern int32 apollo_raw_rldp_portLoopEnterState_set(rtk_port_t port,  uint32  clear);
extern int32 apollo_raw_rldp_portLoopEnterState_get(rtk_port_t port,  apollo_raw_rldp_loopState_t *pState);
extern int32 apollo_raw_rldp_portLoopLeaveState_set(rtk_port_t port,  uint32  clear);
extern int32 apollo_raw_rldp_portLoopLeaveState_get(rtk_port_t port,  apollo_raw_rldp_loopState_t *pState);
extern int32 apollo_raw_rldp_portLoopState_get(rtk_port_t port,  apollo_raw_rldp_loopState_t *pState);
extern int32 apollo_raw_rldp_portForceLoopState_set(rtk_port_t port,  apollo_raw_rldp_loopState_t  state);
extern int32 apollo_raw_rldp_portForceLoopState_get(rtk_port_t port,  apollo_raw_rldp_loopState_t *pState);
extern int32 apollo_raw_rldp_portLoopPortNum_get(rtk_port_t port,  rtk_port_t *pLoopPort);
extern int32 apollo_raw_rlpp_trap_set(rtk_enable_t enable);
extern int32 apollo_raw_rlpp_trap_get(rtk_enable_t *pEnable);

#endif /*#ifndef _APOLLO_RAW_RLDP_H_*/
