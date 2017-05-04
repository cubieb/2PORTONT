#ifndef _APOLLO_RAW_SEC_H_
#define _APOLLO_RAW_SEC_H_

/****************************************************************/
/* Header File inclusion                                        */
/****************************************************************/
#include <dal/apollo/raw/apollo_raw.h>
#include <rtk/sec.h>
#include <dal/apollo/dal_apollo.h>
/****************************************************************/
/* Constant Definition                                          */
/****************************************************************/


/****************************************************************/
/* Type Definition                                              */
/****************************************************************/

typedef enum apollo_raw_dosAction_e
{
    RAW_DOS_ACT_DROP  = 0,
    RAW_DOS_ACT_TRAP,

} apollo_raw_dosAction_t;

typedef enum apollo_raw_dos_flood_s
{
    APOLLO_DOS_FLOOD_SYNFLOOD = 0,
    APOLLO_DOS_FLOOD_FINFLOOD,
    APOLLO_DOS_FLOOD_ICMPFLOOD,
    APOLLO_DOS_FLOOD_END,
}apollo_raw_dos_flood_t;


extern int32 apollo_raw_sec_portAttackPreventEnable_set(rtk_port_t port, rtk_enable_t enable);
extern int32 apollo_raw_sec_portAttackPreventEnable_get(rtk_port_t port, rtk_enable_t *pEnable);
extern int32 apollo_raw_sec_dslvcAttackPreventEnable_set(rtk_port_t vc, rtk_enable_t enable);
extern int32 apollo_raw_sec_dslvcAttackPreventEnable_get(rtk_port_t vc, rtk_enable_t *pEnable);
extern int32 apollo_raw_sec_attackPreventEnable_set(rtk_sec_attackType_t attackType, rtk_enable_t enable);
extern int32 apollo_raw_sec_attackPreventEnable_get(rtk_sec_attackType_t attackType, rtk_enable_t *pEnable);
extern int32 apollo_raw_sec_attackPreventAction_set(rtk_sec_attackType_t attackType, rtk_action_t action);
extern int32 apollo_raw_sec_attackPreventAction_get(rtk_sec_attackType_t attackType, rtk_action_t* pAction);
extern int32 apollo_raw_sec_floodThreshold_set(rtk_sec_attackType_t attackType, uint32 threshold);
extern int32 apollo_raw_sec_floodThreshold_get(rtk_sec_attackType_t attackType, uint32 *pThreshold);
#endif /*#ifndef _APOLLO_RAW_SEC_H_*/

