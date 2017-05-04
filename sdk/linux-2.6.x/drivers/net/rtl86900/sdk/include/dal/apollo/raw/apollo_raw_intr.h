#ifndef _APOLLO_RAW_INTR_H_
#define _APOLLO_RAW_INTR_H_

/****************************************************************/
/* Header File inclusion                                        */
/****************************************************************/
#include <dal/apollo/raw/apollo_raw.h>
#include <rtk/intr.h>


/****************************************************************/
/* Constant Definition                                          */
/****************************************************************/

extern int32 apollo_raw_intr_polarity_set(rtk_intr_polarity_t polar);
extern int32 apollo_raw_intr_polarity_get(rtk_intr_polarity_t *pPolar);
extern int32 apollo_raw_intr_imr_set(rtk_intr_type_t intr, rtk_enable_t enable);
extern int32 apollo_raw_intr_imr_get(rtk_intr_type_t intr, uint32 *pMask);
extern int32 apollo_raw_intr_ims_get(rtk_intr_type_t intr, uint32 *pState);
extern int32 apollo_raw_intr_ims_clear(rtk_intr_type_t intr);
extern int32 apollo_raw_intr_sts_speed_change_get(uint32 *pState);
extern int32 apollo_raw_intr_sts_linkup_get(uint32 *pState);
extern int32 apollo_raw_intr_sts_linkdown_get(uint32 *pState);
extern int32 apollo_raw_intr_sts_gphy_get(uint32 *pState);
extern int32 apollo_raw_intr_sts_speed_change_clear(void);
extern int32 apollo_raw_intr_sts_linkup_clear(void);
extern int32 apollo_raw_intr_sts_linkdown_clear(void);
extern int32 apollo_raw_intr_sts_gphy_clear(void);
#endif /*_APOLLO_RAW_INTR_H_*/


