#ifndef _APOLLO_RAW_METER_H_
#define _APOLLO_RAW_METER_H_

/****************************************************************/
/* Header File inclusion                                        */
/****************************************************************/
#include <dal/apollo/raw/apollo_raw.h>
/****************************************************************/
/* Constant Definition                                          */
/****************************************************************/
#define APOLLO_RAW_METER_TICK_PERIOD_MAX    0xFF
#define APOLLO_RAW_METER_TKN_MAX    0xFF



/****************************************************************/
/* Type Definition                                              */
/****************************************************************/
typedef enum apollo_raw_meter_op_e
{
    APOLLO_RAW_METER_OP_NOCONSUMETKN = 0,
    APOLLO_RAW_METER_OP_CONSUMETKN,
    APOLLO_RAW_METER_OP_END,

}apollo_raw_meter_op_t;


extern int32 apollo_raw_meter_rate_set(uint32 meterIdx, uint32 meterRate);
extern int32 apollo_raw_meter_rate_get(uint32 meterIdx, uint32 *pMeterRate);
extern int32 apollo_raw_meter_ifg_set(uint32 meterIdx, rtk_enable_t ifg);
extern int32 apollo_raw_meter_ifg_get(uint32 meterIdx, rtk_enable_t *pIfg);
extern int32 apollo_raw_meter_bucketSize_set(uint32 meterIdx, uint32 lbthreshold);
extern int32 apollo_raw_meter_bucketSize_get(uint32 meterIdx, uint32 *plbthreshold);
extern int32 apollo_raw_meter_exceed_set(uint32 meterIdx);
extern int32 apollo_raw_meter_exceed_get(uint32 meterIdx, uint32 *pStatus);
extern int32 apollo_raw_meter_btCtrOp_set(apollo_raw_meter_op_t meterOp);
extern int32 apollo_raw_meter_btCtrOp_get(apollo_raw_meter_op_t *pMeterOp);
extern int32 apollo_raw_meter_btCtr_set(uint32 tickPeriod, uint32 tkn);
extern int32 apollo_raw_meter_btCtr_get(uint32 *pTickPeriod, uint32 *pTkn);
extern int32 apollo_raw_meter_ponBtCtrOp_set(apollo_raw_meter_op_t meterOp);
extern int32 apollo_raw_meter_ponBtCtrOp_get(apollo_raw_meter_op_t *pMeterOp);
extern int32 apollo_raw_meter_ponBtCtr_set(uint32 tickPeriod, uint32 tkn);
extern int32 apollo_raw_meter_ponBtCtr_get(uint32 *pTickPeriod, uint32 *pTkn);
#endif /*#ifndef _APOLLO_RAW_METER_H_*/
