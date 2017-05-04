#ifndef _APOLLO_RAW_EPON_H_
#define _APOLLO_RAW_EPON_H_

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


typedef enum apollo_raw_epon_regMode_e
{
    EPON_SW_REG         = 0,
    EPON_HW_REG         = 1
} apollo_raw_epon_regMode_t;




typedef enum apollo_raw_epon_quardThd_e
{
    RAW_EPON_QUARD_THRESHOLD_4TQ    = 0,
    RAW_EPON_QUARD_THRESHOLD_8TQ    = 1,
    RAW_EPON_QUARD_THRESHOLD_16TQ   = 2,
    RAW_EPON_QUARD_THRESHOLD_32TQ   = 3,
} apollo_raw_epon_quardThd_t;




typedef enum apollo_raw_epon_gatehandle_e
{
    RAW_EPON_GATE_ASIC_HANDLE    = 0,
    RAW_EPON_GATE_ASIC_HANDLE_AND_TRAP_TO_CPU = 1
} apollo_raw_epon_gatehandle_t;




typedef enum apollo_raw_epon_mpcpHandle_e
{
    RAW_EPON_MPCP_DROP    = 0,
    RAW_EPON_MPCP_PASS    = 1
} apollo_raw_epon_mpcpHandle_t;





typedef struct apollo_raw_epon_llid_table_s
{
    uint16  llid;
    uint8   valid;
    uint8   report_timer;
    uint8   is_report_timeout; /*read only*/

}apollo_raw_epon_llid_table_t;





typedef struct apollo_raw_epon_grant_s
{
    uint32  grant_start;
    uint32  grant_end;
    uint8   is_force_report;
    uint8   is_disc;
    uint8   llid_idx;
}apollo_raw_epon_grant_t;



extern int32 apollo_raw_epon_regMode_set(apollo_raw_epon_regMode_t mode);
extern int32 apollo_raw_epon_regMode_get(apollo_raw_epon_regMode_t *mode);
extern int32 apollo_raw_epon_regLlidIdx_set(uint32 idx);
extern int32 apollo_raw_epon_regLlidIdx_get(uint32 *idx);
extern int32 apollo_raw_epon_regMac_set(rtk_mac_t *mac);
extern int32 apollo_raw_epon_regMac_get(rtk_mac_t *mac);
extern int32 apollo_raw_epon_regReguest_set(rtk_enable_t mode);
extern int32 apollo_raw_epon_regReguest_get(rtk_enable_t *mode);
extern int32 apollo_raw_epon_regPendingGrantNum_set(uint8 num);
extern int32 apollo_raw_epon_regPendingGrantNum_get(uint32 *num);
extern int32 apollo_raw_epon_mpcpTimeoutVal_set(uint8 timeout_unit);
extern int32 apollo_raw_epon_mpcpTimeoutVal_get(uint8 *timeout_unit);
extern int32 apollo_raw_epon_regLlidImr_set(rtk_enable_t mode);
extern int32 apollo_raw_epon_regLlidImr_get(rtk_enable_t *mode);
extern int32 apollo_raw_epon_timeDriftImr_set(rtk_enable_t mode);
extern int32 apollo_raw_epon_timeDriftImr_get(rtk_enable_t *mode);
extern int32 apollo_raw_epon_mpcpTimeoutImr_set(rtk_enable_t mode);
extern int32 apollo_raw_epon_mpcpTimeoutImr_get(rtk_enable_t *mode);
extern int32 apollo_raw_epon_mpcpTimeoutIms_clear(void);
extern int32 apollo_raw_epon_timeDriftIms_clear(void);
extern int32 apollo_raw_epon_regLlidTxIms_clear(void);
extern int32 apollo_raw_epon_mpcpTimeoutIms_get(rtk_enable_t *status);
extern int32 apollo_raw_epon_timeDriftIms_get(rtk_enable_t *status);
extern int32 apollo_raw_epon_llidTxIms_get(rtk_enable_t *status);
extern int32 apollo_raw_epon_discSyncTime_get(uint32 *syncTime);
extern int32 apollo_raw_epon_discSyncTime_set(uint32 syncTime);
extern int32 apollo_raw_epon_normalSyncTime_get(uint32 *syncTime);
extern int32 apollo_raw_epon_normalSyncTime_set(uint32 syncTime);
extern int32 apollo_raw_epon_laserOnTime_set(uint32 laserTime);
extern int32 apollo_raw_epon_laserOffTime_set(uint32 laserTime);
extern int32 apollo_raw_epon_laserOnTime_get(uint32 *laserTime);
extern int32 apollo_raw_epon_laserOffTime_get(uint32 *laserTime);
extern int32 apollo_raw_epon_quardThd_get(apollo_raw_epon_quardThd_t *quardTh);
extern int32 apollo_raw_epon_quardThd_set(apollo_raw_epon_quardThd_t quardTh);
extern int32 apollo_raw_epon_rttAdj_get(int16 *rttAdj);
extern int32 apollo_raw_epon_rttAdj_set(int16 rttAdj);
extern int32 apollo_raw_epon_StopLocalTime_set(rtk_enable_t enable);
extern int32 apollo_raw_epon_StopLocalTime_get(rtk_enable_t *enable);
extern int32 apollo_raw_epon_fecEnable_set(rtk_enable_t enable);
extern int32 apollo_raw_epon_fecEnable_get(rtk_enable_t *enable);
extern int32 apollo_raw_epon_fecUsEnable_set(rtk_enable_t enable);
extern int32 apollo_raw_epon_fecUsEnable_get(rtk_enable_t *enable);
extern int32 apollo_raw_epon_fecDsEnable_set(rtk_enable_t enable);
extern int32 apollo_raw_epon_fecDsEnable_get(rtk_enable_t *enable);
extern int32 apollo_raw_epon_powerSavingEnable_set(rtk_enable_t enable);
extern int32 apollo_raw_epon_powerSavingEnable_get(rtk_enable_t *enable);
extern int32 apollo_raw_epon_powerSavingStatus_get(rtk_enable_t *enable);
extern int32 apollo_raw_epon_mpcpGateHandle_get(apollo_raw_epon_gatehandle_t *mode);
extern int32 apollo_raw_epon_mpcpGateHandle_set(apollo_raw_epon_gatehandle_t mode);
extern int32 apollo_raw_epon_mpcpOtherHandle_get(apollo_raw_epon_mpcpHandle_t *mode);
extern int32 apollo_raw_epon_mpcpOtherHandle_set(apollo_raw_epon_mpcpHandle_t mode);
extern int32 apollo_raw_epon_mpcpInvalidLenHandle_get(apollo_raw_epon_mpcpHandle_t *mode);
extern int32 apollo_raw_epon_mpcpInvalidLenHandle_set(apollo_raw_epon_mpcpHandle_t mode);
extern int32 apollo_raw_epon_llidTable_set(uint32 llid_idx, apollo_raw_epon_llid_table_t *entry);
extern int32 apollo_raw_epon_llidTable_get(uint32 llid_idx,apollo_raw_epon_llid_table_t *entry);
extern int32 apollo_raw_epon_grant_list_get(uint32 grant_idx, apollo_raw_epon_grant_t *entry);
extern int32 apollo_raw_epon_dbgTxCtrlLLidIdx_get(uint32 *idx);



#endif /*#ifndef _APOLLO_RAW_EPON_H_*/

