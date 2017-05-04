/** file lt_api_vp886_dc_voltage.c
 *
 *  This file contains the VP886 dc voltage test routine.
 *
 * Copyright (c) 2012, Microsemi Corporation
 *
 * $Revision: 10353 $
 * $LastChangedDate: 2012-07-30 13:38:27 -0500 (Mon, 30 Jul 2012) $
 */
#include "lt_api.h"

#ifdef LT_DC_VOLTAGE_886

#include "lt_api_internal.h"
#include "lt_api_vp886.h"

typedef enum {
    DC_VOLTAGE_INIT             = LT_GLOBAL_FIRST_STATE,
    DC_VOLTAGE_GO_TO_ACTIVE,
    DC_VOLTAGE_LINE_STATE_SETTLE,
    DC_VOLTAGE_REQUEST_FEED_VSAB,
    DC_VOLTAGE_STORE_FEED_VSAB,
    DC_VOLTAGE_RAMP_TO_NEAREST,
    DC_VOLTAGE_REQUEST_VSAB_1,
    DC_VOLTAGE_STORE_VSAB_1,
    DC_VOLTAGE_RAMP_TO_OPPOSITE_1,
    DC_VOLTAGE_RAMP_TO_OPPOSITE_2,
    DC_VOLTAGE_REQUEST_VSAB_2,
    DC_VOLTAGE_STORE_VSAB_2,
    DC_VOLTAGE_RAMP_TO_FEED_1,
    DC_VOLTAGE_RAMP_TO_FEED_2,
    DC_VOLTAGE_CONCLUDE,
    DC_VOLTAGE_COMPLETE,
    DC_VOLTAGE_ABORT            = LT_GLOBAL_ABORTED_STATE,
    DC_VOLTAGE_ENUM_SIZE        = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp886DcVoltageStates;


static bool
DcVoltageHandlerSM(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent,
    LtTestStatusType *pRetval);

static void
CalculateResults(
    LtTestCtxType *pTestCtx);

extern bool 
LtVp886DcVoltageAttributeCheck(
    LtTestCtxType *pTestCtx)
{
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;

    LtDcVoltageInputType *ppInputs =
        (LtDcVoltageInputType*)pTemp886->attributes.pInputs;

    LtDcVoltageCriteriaType *ppCriteria =
        (LtDcVoltageCriteriaType*)pTemp886->attributes.pCriteria;

    LtVp886VoltageTempType *pTempDcVoltage = &pTemp886->tempData.dcVoltage;

    /* Check input args */
    if (ppInputs == VP_NULL) {
        pTempDcVoltage->input.testVoltage = LT_DC_VOLTAGE_DFLT_SIGNAL;
    } else {
        /* Check for valid input */
        if (ppInputs->testVoltage > LT_VP886_MAX_VPK_VOLTAGE) {
            LT_DOUT(LT_DBG_ERRR, ("Vp886DcVoltageAttributesCheck(testVoltage:%li) : "
                "invalid", ppInputs->testVoltage));
            return FALSE;
        }
        pTempDcVoltage->input.testVoltage = ppInputs->testVoltage;
    }

    /* Input voltage must be positive */
    pTempDcVoltage->input.testVoltage = ABS(pTempDcVoltage->input.testVoltage);

    /* Check criteria args*/
    if (ppCriteria == VP_NULL) {
        pTempDcVoltage->criteria.voltageErr = LT_DC_VOLTAGE_DFLTC_VOLTAGE_TOL;
    } else {
        /* Check for valid criteria >=0% and <100% */
        if ((LT_0_PERCENT >= ppCriteria->voltageErr) ||
            (LT_100_PERCENT < ppCriteria->voltageErr)) {
            LT_DOUT(LT_DBG_ERRR, ("Vp886DcVoltageAttributesCheck(voltageErr:%li) : "
                "invalid", ppCriteria->voltageErr));
            return FALSE;
        }
        pTempDcVoltage->criteria.voltageErr = ppCriteria->voltageErr;
    }

    /* setup function Handler */
    pTestCtx->Lt886TestEventHandlerFunc = DcVoltageHandlerSM;
    return TRUE;
}

static bool
DcVoltageHandlerSM(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent,
    LtTestStatusType *pRetval)
{
    VpTestResultType *pTestPrimRslts = (VpTestResultType*)pEvent->pResult;
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;
    LtVp886VoltageTempType *pTempDcVoltage = &pTemp886->tempData.dcVoltage;

    bool requestNextState = FALSE;
    *pRetval = LT_STATUS_RUNNING;

    LT_DOUT(LT_DBG_INFO, ("DcVoltageHandlerSM(testState:%i)", pTestCtx->state));

    switch (pTestCtx->state) {
        case DC_VOLTAGE_INIT:
            LtInitOutputStruct(pTestCtx->pResult, LT_TID_DC_VOLTAGE);

            /* Initialize primitive inputs */
            pTemp886->loopCond.calMode = FALSE;
            pTemp886->loopCond.integrateTime =  LT_VP886_DC_VOLTAGE_INTEGRATE_TIME;
            pTemp886->loopCond.settlingTime = LT_VP886_DC_VOLTAGE_SETTLE_TIME;
            pTemp886->loopCond.loopCond = VP_LOOP_COND_TEST_VSAB;
            pTempDcVoltage->vpApiRampInitInput.vRate = LT_VP886_DC_VOLTAGE_RAMP_SLOPE;
            pTempDcVoltage->vpApiRampInitInput.bias = 0;
            pTempDcVoltage->vpApiRampInitInput.tip = VP_TEST_TIP_RING;
            pTempDcVoltage->vpApiRampInput.vRate = LT_VP886_DC_VOLTAGE_RAMP_SLOPE;
            pTempDcVoltage->vpApiRampInput.bias = 0;
            pTempDcVoltage->vpApiRampInput.tip = VP_TEST_TIP_RING;

            pTestCtx->state = DC_VOLTAGE_GO_TO_ACTIVE;
            requestNextState = TRUE;
            break;

        case DC_VOLTAGE_GO_TO_ACTIVE: {
            VpLineStateType lineState;
            VpLineStateType lineStateCurrent = pTemp886->resources.initialLineState;

            /* go to normal active unless in a pol rev state */
            if ((lineStateCurrent == VP_LINE_ACTIVE_POLREV) ||
                (lineStateCurrent == VP_LINE_TALK_POLREV) ||
                (lineStateCurrent == VP_LINE_OHT_POLREV))
            {
                lineState = VP_LINE_ACTIVE_POLREV;
                pTempDcVoltage->normalPolarity = FALSE;
            } else {
                lineState = VP_LINE_ACTIVE;
                pTempDcVoltage->normalPolarity = TRUE;
            }
            *pRetval = Vp886SetLineStateWrapper(pTestCtx, lineState);

            pTestCtx->state = DC_VOLTAGE_LINE_STATE_SETTLE;
            requestNextState = TRUE;
            break;
        }
        case DC_VOLTAGE_LINE_STATE_SETTLE:
            pTemp886->testTimer.timerVal = LT_VP886_STATE_CHANGE_SETTLE;
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY, &pTemp886->testTimer);

            pTestCtx->state = DC_VOLTAGE_REQUEST_FEED_VSAB;
            break;

        case DC_VOLTAGE_REQUEST_FEED_VSAB:
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS,
                &pTemp886->loopCond);

            pTestCtx->state = DC_VOLTAGE_STORE_FEED_VSAB;
            break;

        case DC_VOLTAGE_STORE_FEED_VSAB: {
            /* store return value from initial vsab read */
            pTempDcVoltage->feedV = pTestPrimRslts->result.loopCond.data;

            pTestCtx->state = DC_VOLTAGE_RAMP_TO_NEAREST;
            requestNextState = TRUE;
            break;
        }
        case DC_VOLTAGE_RAMP_TO_NEAREST: {
            int16 inputVADC;
            inputVADC = (int16)VP886_UNIT_CONVERT(pTempDcVoltage->input.testVoltage, VP886_UNIT_MV, VP886_UNIT_ADC_VAB);
            if (pTempDcVoltage->normalPolarity) {
                inputVADC = -ABS(inputVADC);
            } else {
                inputVADC = ABS(inputVADC);
            }
            pTempDcVoltage->vpApiRampInitInput.dcVstart = pTempDcVoltage->feedV;
            pTempDcVoltage->vpApiRampInitInput.dcVend = inputVADC;
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_RAMP_INIT,
                &pTempDcVoltage->vpApiRampInitInput);

            pTestCtx->state = DC_VOLTAGE_REQUEST_VSAB_1;
            break;
        }
        case DC_VOLTAGE_REQUEST_VSAB_1:
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS,
                &pTemp886->loopCond);

            pTestCtx->state = DC_VOLTAGE_STORE_VSAB_1;
            break;

        case DC_VOLTAGE_STORE_VSAB_1:
            /* store return value from vsab 1 read */
            pTempDcVoltage->adcVsab1 = (uint16)pTestPrimRslts->result.loopCond.data;

            pTestCtx->state = DC_VOLTAGE_RAMP_TO_OPPOSITE_1;
            requestNextState = TRUE;
            break;

        case DC_VOLTAGE_RAMP_TO_OPPOSITE_1: {
            int32 rampSize;
            int16 rampSADC;

            /* To go from one test voltage to the opposite, the ramp size will
               be double the magnitude of the test voltage. */
            rampSize = pTempDcVoltage->input.testVoltage * 2;

            /* If the needed ramp size exceeds the maximum, do two half-sized
               ramps instead. */
            if (rampSize < LT_VP886_DC_VOLTAGE_MAX_RAMP_SIZE) {
                pTestCtx->state = DC_VOLTAGE_REQUEST_VSAB_2;
            } else {
                rampSize /= 2;
                pTestCtx->state = DC_VOLTAGE_RAMP_TO_OPPOSITE_2;
            }

            /* Convert the ramp size to SADC units.  Starting from normal
               polarity (negative voltage) we want a positive ramp.  From
               reverse polarity, we want a negative ramp. */
            rampSADC = VP886_UNIT_CONVERT(rampSize, VP886_UNIT_MV, VP886_UNIT_ADC_VAB);
            if (pTempDcVoltage->normalPolarity) {
                rampSADC = ABS(rampSADC);
            } else {
                rampSADC = -ABS(rampSADC);
            }
            pTempDcVoltage->vpApiRampInput.dcVoltage = rampSADC;
            
            /* Start the ramp */
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_RAMP,
                &pTempDcVoltage->vpApiRampInput);
            break;
        }
        case DC_VOLTAGE_RAMP_TO_OPPOSITE_2:
            /* Perform the second half of a large ramp using the same inputs. */
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_RAMP,
                &pTempDcVoltage->vpApiRampInput);

            pTestCtx->state = DC_VOLTAGE_REQUEST_VSAB_2;
            break;

        case DC_VOLTAGE_REQUEST_VSAB_2:
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS,
                &pTemp886->loopCond);

            pTestCtx->state = DC_VOLTAGE_STORE_VSAB_2;
            break;

        case DC_VOLTAGE_STORE_VSAB_2:
            /* store return value from vsab 2 read */
            pTempDcVoltage->adcVsab2 = (uint16)pTestPrimRslts->result.loopCond.data;

            pTestCtx->state = DC_VOLTAGE_RAMP_TO_FEED_1;
            requestNextState = TRUE;
            break;

        case DC_VOLTAGE_RAMP_TO_FEED_1: {
            int32 rampSize;
            int16 rampSADC;

            /* To go from the opposite test voltage back to the original feed
               voltage, we need a ramp size that is the sum of the test voltage
               and feed voltage magnitudes. */
            rampSize = pTempDcVoltage->input.testVoltage +
                ABS(VP886_UNIT_CONVERT(pTempDcVoltage->feedV, VP886_UNIT_ADC_VAB, VP886_UNIT_MV));

            /* If the needed ramp size exceeds the maximum, do two half-sized
               ramps instead. */
            if (rampSize < LT_VP886_DC_VOLTAGE_MAX_RAMP_SIZE) {
                pTestCtx->state = DC_VOLTAGE_CONCLUDE;
            } else {
                rampSize /= 2;
                pTestCtx->state = DC_VOLTAGE_RAMP_TO_FEED_2;
            }

            /* Convert the ramp size to SADC units.  To get back to normal
               polarity (negative voltage) we want a negative ramp.  To get back
               to reverse polarity, we want a positive ramp. */
            rampSADC = VP886_UNIT_CONVERT(rampSize, VP886_UNIT_MV, VP886_UNIT_ADC_VAB);
            if (pTempDcVoltage->normalPolarity) {
                rampSADC = -ABS(rampSADC);
            } else {
                rampSADC = ABS(rampSADC);
            }
            pTempDcVoltage->vpApiRampInput.dcVoltage = rampSADC;
            
            /* Start the ramp */
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_RAMP,
                &pTempDcVoltage->vpApiRampInput);
            break;
        }
        case DC_VOLTAGE_RAMP_TO_FEED_2:
            /* Perform the second half of a large ramp using the same inputs. */
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_RAMP,
                &pTempDcVoltage->vpApiRampInput);

            pTestCtx->state = DC_VOLTAGE_CONCLUDE;
            break;

        /* Start the conclude primitive if this test not a part of an internal test */
        case DC_VOLTAGE_CONCLUDE: {
            VpTestConcludeType conclude = { FALSE };
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE, &conclude);

            pTestCtx->state = DC_VOLTAGE_COMPLETE;
            break;
        }

        case DC_VOLTAGE_COMPLETE:
            /* Calculate results and end the test */
            CalculateResults(pTestCtx);
            *pRetval = LT_STATUS_DONE;
            break;

        case DC_VOLTAGE_ABORT:
            /* Test aborted, results may indicate source of error. */
            *pRetval = LT_STATUS_ABORTED;
            break;

        default:
            /* This should never happen */
            LT_DOUT(LT_DBG_ERRR, ("This should never happen -> LT_STATUS_ERROR_UNKNOWN"));
            *pRetval = LT_STATUS_ERROR_UNKNOWN;
            break;
    }
    return requestNextState;
}

static void
CalculateResults(
    LtTestCtxType *pTestCtx)
{
    LtVoltageType diffV;
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;
    LtVp886VoltageTempType *pTempDcVoltage = &pTemp886->tempData.dcVoltage;
    LtDcVoltageResultType *pResults = &pTestCtx->pResult->result.dcVoltage;

    LtVoltageType reqstV = ABS(pTempDcVoltage->input.testVoltage);
    /* knocking down the % error granularity to Volts*/
    LtPercentType testVoltageErr = (pTempDcVoltage->criteria.voltageErr / 1000);
    testVoltageErr = ((reqstV * testVoltageErr) / 100) + LT_DC_VOLTAGE_DFLTC_VOLTAGE_OFF;

    pResults->measuredVoltage1 = VP886_UNIT_CONVERT(pTempDcVoltage->adcVsab1,
        VP886_UNIT_ADC_VAB, VP886_UNIT_MV);

    pResults->measuredVoltage2 = VP886_UNIT_CONVERT(pTempDcVoltage->adcVsab2,
        VP886_UNIT_ADC_VAB, VP886_UNIT_MV);

    pResults->fltMask = LT_TEST_PASSED;

    diffV = ABS(reqstV - ABS(pResults->measuredVoltage1));
    if (diffV > testVoltageErr) {
        pResults->fltMask |= LT_DC_VOLTAGE_TEST_FAILED;
    }

    diffV = ABS(reqstV - ABS(pResults->measuredVoltage2));
    if (diffV > testVoltageErr) {
        pResults->fltMask |= LT_DC_VOLTAGE_TEST_FAILED;
    }
    return;
}

#endif /* LT_DC_VOLTAGE_886 */
