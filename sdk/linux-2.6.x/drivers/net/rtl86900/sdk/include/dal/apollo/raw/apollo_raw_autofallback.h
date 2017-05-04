#ifndef _APOLLO_RAW_AUTOFALLBACK_H_
#define _APOLLO_RAW_AUTOFALLBACK_H_

/****************************************************************/
/* Header File inclusion                                        */
/****************************************************************/
#include <dal/apollo/raw/apollo_raw.h>
/****************************************************************/
/* Constant Definition                                          */
/****************************************************************/
#define RAW_AUTOFALLBACK_TIMEOUT_MAX    (1020)
#define RAW_AUTOFALLBACK_TIMEOUT_STEP   (4)

typedef enum raw_autofallback_monitor_e
{
    RAW_MONITOR_8K = 0,
    RAW_MONITOR_16K,
    RAW_MONITOR_32K,
    RAW_MONITOR_64K,
    RAW_MONITOR_128K,
    RAW_MONITOR_256K,
    RAW_MONITOR_512K,
    RAW_MONITOR_1M,
    RAW_MONITOR_END
} raw_autofallback_monitor_t;

typedef enum raw_autofallback_error_e
{
    RAW_ERROR_1_PKT = 0,
    RAW_ERROR_2_PKT,
    RAW_ERROR_4_PKT,
    RAW_ERROR_8_PKT,
    RAW_ERROR_16_PKT,
    RAW_ERROR_32_PKT,
    RAW_ERROR_64_PKT,
    RAW_ERROR_128_PKT,
    RAW_ERROR_END
} raw_autofallback_error_t;

typedef enum raw_autofallback_restorePL_e
{
    RAW_RESTORE_PL_NEGATIVE = 0,
    RAW_RESTORE_PL_AFFIRMATIVE,
    RAW_RESTORE_PL_END
} raw_autofallback_restorePL_t;

typedef enum raw_autofallback_validFlow_e
{
    RAW_VALID_FLOW_NEGATIVE = 0,
    RAW_VALID_FLOW_AFFIRMATIVE,
    RAW_VALID_FLOW_END
} raw_autofallback_validFlow_t;

/****************************************************************/
/* Type Definition                                              */
/****************************************************************/

extern int32 apollo_raw_autofallback_reducePL_set(rtk_enable_t state);
extern int32 apollo_raw_autofallback_reducePL_get(rtk_enable_t *pState);
extern int32 apollo_raw_autofallback_timeoutTH_set(uint32 time);
extern int32 apollo_raw_autofallback_timeoutTH_get(uint32 *pTime);
extern int32 apollo_raw_autofallback_timeoutIgnore_set(rtk_enable_t state);
extern int32 apollo_raw_autofallback_timeoutIgnore_get(rtk_enable_t *pState);
extern int32 apollo_raw_autofallback_maxMonitorCount_set(raw_autofallback_monitor_t mon_count);
extern int32 apollo_raw_autofallback_maxMonitorCount_get(raw_autofallback_monitor_t *pMon_count);
extern int32 apollo_raw_autofallback_maxErrorCount_set(raw_autofallback_error_t err_count);
extern int32 apollo_raw_autofallback_maxErrorCount_get(raw_autofallback_error_t *pErr_count);
extern int32 apollo_raw_autofallback_timer_set(rtk_enable_t state);
extern int32 apollo_raw_autofallback_timer_get(rtk_enable_t *pState);
extern int32 apollo_raw_autofallback_enable_set(rtk_port_t port, rtk_enable_t state);
extern int32 apollo_raw_autofallback_enable_get(rtk_port_t port, rtk_enable_t *pState);
extern int32 apollo_raw_autofallback_restorePL_get(rtk_port_t port, raw_autofallback_restorePL_t *pState);
extern int32 apollo_raw_autofallback_validFlow_get(rtk_port_t port, raw_autofallback_validFlow_t *pValidFlow);
extern int32 apollo_raw_autofallback_errorCount_get(rtk_port_t port, uint32 *pErrCount);
extern int32 apollo_raw_autofallback_monitorCount_get(rtk_port_t port, uint32 *pMonCount);

#endif /*#ifndef _APOLLO_RAW_AUTOFALLBACK_H_*/

