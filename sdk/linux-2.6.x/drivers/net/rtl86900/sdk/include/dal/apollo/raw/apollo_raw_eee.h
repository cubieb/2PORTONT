#ifndef _APOLLO_RAW_EEE_H_
#define _APOLLO_RAW_EEE_H_

/****************************************************************/
/* Header File inclusion                                        */
/****************************************************************/
#include <dal/apollo/raw/apollo_raw.h>
/****************************************************************/
/* Constant Definition                                          */
/****************************************************************/
#define APOLLO_EEELLDP_SUBTYPE_MAX					0xFF



/****************************************************************/
/* Type Definition                                              */
/****************************************************************/



extern int32 apollo_raw_eee_lldpTrapPri_set(rtk_pri_t priority);
extern int32 apollo_raw_eee_lldpTrapPri_get(rtk_pri_t *pPriority);
extern int32 apollo_raw_eee_lldpEnable_set(rtk_enable_t enable);
extern int32 apollo_raw_eee_lldpEnable_get(rtk_enable_t *pEnable);
extern int32 apollo_raw_eee_lldpTrapEnable_set(rtk_enable_t enable);
extern int32 apollo_raw_eee_lldpTrapEnable_get(rtk_enable_t *pEnable);
extern int32 apollo_raw_eee_lldpSubtype_set(uint32 subtype);
extern int32 apollo_raw_eee_lldpSubtype_get(rtk_enable_t *pSubtype);

#endif /*#ifndef _APOLLO_RAW_EEE_H_*/

