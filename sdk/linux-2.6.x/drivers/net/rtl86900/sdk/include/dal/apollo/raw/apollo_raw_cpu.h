#ifndef _APOLLO_RAW_CPU_H_
#define _APOLLO_RAW_CPU_H_

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

extern int32 apollo_raw_cpu_aware_port_mask_set(rtk_portmask_t port_mask);
extern int32 apollo_raw_cpu_aware_port_mask_get(rtk_portmask_t *pPort_mask);
extern int32 apollo_raw_cpu_tag_format_set(rtk_cpu_tag_fmt_t mode);
extern int32 apollo_raw_cpu_tag_format_get(rtk_cpu_tag_fmt_t *pMode);
extern int32 apollo_raw_cpu_dsl_en_set(rtk_enable_t state);
extern int32 apollo_raw_cpu_dsl_en_get(rtk_enable_t *pState);
extern int32 apollo_raw_cpu_trap_insert_tag_set(rtk_enable_t state);
extern int32 apollo_raw_cpu_trap_insert_tag_get(rtk_enable_t *pState);


#endif /*#ifndef _APOLLO_RAW_CPU_H_*/

