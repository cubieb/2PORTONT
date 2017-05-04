#ifndef _APOLLO_RAW_MIRROR_H_
#define _APOLLO_RAW_MIRROR_H_

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

typedef struct raw_mirror_cfg_s
{
    rtk_portmask_t  source_portmask;
    rtk_port_t      monitor_port;
    rtk_enable_t    mirror_tx;
    rtk_enable_t    mirror_rx;
} raw_mirror_cfg_t;

extern int32 apollo_raw_mirrorCfg_set(raw_mirror_cfg_t *pMirror_cfg);
extern int32 apollo_raw_mirrorCfg_get(raw_mirror_cfg_t *pMirror_cfg);
extern int32 apollo_raw_mirrorIso_set(rtk_enable_t state);
extern int32 apollo_raw_mirrorIso_get(rtk_enable_t *pState);

#endif /*#ifndef _APOLLO_RAW_MIRROR_H_*/

