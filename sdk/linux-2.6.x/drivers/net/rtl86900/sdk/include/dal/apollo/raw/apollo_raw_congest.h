#ifndef _APOLLO_RAW_CONGEST_H_
#define _APOLLO_RAW_CONGEST_H_

/****************************************************************/
/* Header File inclusion                                        */
/****************************************************************/
#include <dal/apollo/raw/apollo_raw.h>

/****************************************************************/
/* Constant Definition                                          */
/****************************************************************/
#define APOLLO_RAW_SC_SUSTEN_TIMER_MAX     0XF

/****************************************************************/
/* Type Definition                                              */
/****************************************************************/


extern int32 apollo_raw_sc_cgstInd_reset(rtk_port_t port);
extern int32 apollo_raw_sc_cgstInd_get(rtk_port_t port, uint32 *pOccur);
extern int32 apollo_raw_sc_sustTmr_set(rtk_port_t port, uint32 sustTimer);
extern int32 apollo_raw_sc_sustTmr_get(rtk_port_t port, uint32 *pSustTimer);
extern int32 apollo_raw_sc_cgstTmr_get(rtk_port_t port, uint32 *pCgstTimer);

#endif /*_APOLLO_RAW_CONGEST_H_*/


