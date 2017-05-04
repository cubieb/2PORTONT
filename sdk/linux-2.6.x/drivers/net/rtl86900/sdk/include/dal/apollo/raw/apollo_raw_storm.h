#ifndef _APOLLO_RAW_STORM_H_
#define _APOLLO_RAW_STORM_H_

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
typedef enum raw_storm_type_e
{
    RAW_STORM_UNKN_MC = 0,
    RAW_STORM_UNKN_UC,
    RAW_STORM_MC,
    RAW_STORM_BC,
    RAW_STORM_END
} raw_storm_type_t;

typedef enum raw_storm_alt_type_e
{
    RAW_STORM_ALT_DEFAULT = 0,
    RAW_STORM_ALT_ARP,
    RAW_STORM_ALT_DHCP,
    RAW_STORM_ALT_IGMP_MLD,
    RAW_STORM_ALT_TYPE_END
} raw_storm_alt_type_t;

extern int32 apollo_raw_stormControlState_set(raw_storm_type_t type, rtk_port_t port, rtk_enable_t state);
extern int32 apollo_raw_stormControlState_get(raw_storm_type_t type, rtk_port_t port, rtk_enable_t *pState);
extern int32 apollo_raw_stormControlMeter_set(raw_storm_type_t type, rtk_port_t port, uint32 meter_idx);
extern int32 apollo_raw_stormControlMeter_get(raw_storm_type_t type, rtk_port_t port, uint32 *pMeter_idx);
extern int32 apollo_raw_stormControlAlt_set(raw_storm_type_t type, raw_storm_alt_type_t alt_type);
extern int32 apollo_raw_stormControlAlt_get(raw_storm_type_t type, raw_storm_alt_type_t *pAlt_type);

#endif /*#ifndef _APOLLO_RAW_STORM_H_*/

