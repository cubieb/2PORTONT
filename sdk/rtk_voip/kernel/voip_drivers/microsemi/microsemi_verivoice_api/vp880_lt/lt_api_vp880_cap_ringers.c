/** file lt_api_vp880_cap_ringers.c
 *
 *  This file contains the GR-909 PASS/FAIL Capacitive Ringers Test.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 9747 $
 * $LastChangedDate: 2012-04-03 11:11:56 -0500 (Tue, 03 Apr 2012) $
 */

#include "lt_api.h"

#if defined(VP880_INCLUDE_TESTLINE_CODE) && defined(LT_VP880_PACKAGE) && defined(LT_RINGERS)
#ifdef LT_VP880_VVP_PACKAGE

#include "lt_api_internal.h"
#include "lt_api_vp880.h"

typedef enum {
    CAP_RINGERS_INIT            = LT_GLOBAL_FIRST_STATE,
    CAP_RINGERS_SET_LINE_STATE,
    CAP_RINGERS_HOOK_GNDKY,
    CAP_RINGERS_CALIBRATE,
    CAP_RINGERS_SET_LINE_STATE_WITH_DELAY,
    CAP_RINGERS_REQUEST_FEED_VSAB,
    CAP_RINGERS_STORE_FEED_VSAB,
    CAP_RINGERS_RAMP_TO_INPUT_V,
    CAP_RINGERS_RAMP_TO_OPPOSITE_INPUT_V,
    CAP_RINGERS_STORE_RAMP_RESULT_1,
    CAP_RINGERS_RAMP_BACK_TO_INPUT_V,
    CAP_RINGERS_STORE_RAMP_RESULT_2,
    CAP_RINGERS_RAMP_TO_FEED,
    CAP_RINGERS_CONCLUDE,
    CAP_RINGERS_COMPLETE,
    CAP_RINGERS_ABORT           = LT_GLOBAL_ABORTED_STATE,
    CAP_RINGERS_ENUM_SIZE       = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp880CapRenStates;


LtTestStatusType
LtVp880CapRampToInputV(
    LtTestCtxType *pTestCtx,
    LtVp880RingersTempType *pTempRingers);

LtTestStatusType
LtVp880CapRampUp(
    LtTestCtxType *pTestCtx,
    LtVp880RingersTempType *pTempRingers);

LtTestStatusType
LtVp880CapRampDown(
    LtTestCtxType *pTestCtx,
    LtVp880RingersTempType *pTempRingers);

LtTestStatusType
LtVp880CapRampToFeed(
    LtTestCtxType *pTestCtx,
    LtVp880RingersTempType *pTempRingers,
    int *pState);

VpTestTipSelectType
LtVp880CapGetCurrentLed(
    LtVp880RingersTempType *pTempRingers);

extern LtTestStatusType
LtVp880CapRingerHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent)
{
    LtTestStatusType retval = LT_STATUS_ERROR_UNKNOWN;
    VpTestResultType *pVpTestResult = (VpTestResultType*)pEvent->pResult;
    LtVp880TestTempType *pTemp880 = &pTestCtx->pTemp->vp880Temp;
    LtVp880RingersTempType *pTempRingers = &pTemp880->tempData.ringers;
    LtRingersResultType *pResults = &pTestCtx->pResult->result.ringers;
    int *pState = &(pTestCtx->state);

    LT_DOUT(LT_DBG_INFO, ("LtVp880CapRingerHandler(testState:%i)", *pState));

    /* Check for, store and return any VP-API or LT_API errors */
    if (TRUE == Vp880EventErrorCheck(pTestCtx, pEvent, &retval)) {
        return retval;
    }

    switch (*pState) {
        case CAP_RINGERS_INIT:{

            if (pTemp880->internalTest == TRUE) {
                LtTestResultType resTmp;

                LtInitOutputStruct(&resTmp, LT_TID_RINGERS);
                LtMemCpy(pResults, &(resTmp.result.ringers), sizeof(LtRingersResultType));
            }
            /* Initialize the default return values */
            pResults->ringerTestType = pTempRingers->input.ringerTestType;
            pTempRingers->innerState = 0;

            /* Setting up a loop condition struct to be used a later states */
            pTemp880->loopCond.calMode = FALSE;
            pTemp880->loopCond.integrateTime =  LT_VP880_CAP_RINGERS_INTEGRATE_TIME;
            pTemp880->loopCond.settlingTime = LT_VP880_CAP_RINGERS_SETTLE_TIME;
            pTemp880->loopCond.loopCond = VP_LOOP_COND_TEST_VSAB;
            /* no break */
        }

        case CAP_RINGERS_SET_LINE_STATE: {
            VpLineStateType lineState = VP_LINE_ACTIVE;
            VpLineStateType lineStateCurrent = pTemp880->resources.initialLineState;
            /* go to normal active unless in a pol rev state */
            if ((lineStateCurrent == VP_LINE_ACTIVE_POLREV) ||
                (lineStateCurrent == VP_LINE_TALK_POLREV) ||
                (lineStateCurrent == VP_LINE_OHT_POLREV)) {
                lineState = VP_LINE_ACTIVE_POLREV;
            }

            if ( VP_STATUS_SUCCESS !=
                VpSetLineState(pTestCtx->pLineCtx, lineState)) {
                return LT_STATUS_ERROR_UNKNOWN;
            }

            /* wait for the line to settle */
            pTemp880->testTimer.timerVal = LT_VP880_STATE_CHANGE_SETTLE;
            *pState = CAP_RINGERS_HOOK_GNDKY;
            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY,
                &pTemp880->testTimer);
            break;
        }

        case CAP_RINGERS_HOOK_GNDKY: {
            bool temp;
            pTempRingers->quickStop = FALSE;

            /* Check groundkey status and error if something goes wrong */
            if ( VP_STATUS_SUCCESS !=
                VpGetLineStatus(pTestCtx->pLineCtx, VP_INPUT_GKEY, &temp)) {
                return LT_STATUS_ERROR_UNKNOWN;
            }

            /* If the ground key bit is true then test is done early due to ilg */
            if (TRUE == temp) {
                pResults->fltMask = LT_TEST_MEASUREMENT_ERROR;
                pResults->measStatus = LT_MSRMNT_STATUS_EXCESSIVE_ILG;
                pResults->ren = LT_REN_NOT_MEASURED;
                pResults->rentg = LT_REN_NOT_MEASURED;
                pResults->renrg = LT_REN_NOT_MEASURED;

                pTemp880->testTimer.timerVal = LT_VP880_NEXT_TICK;
                *pState = CAP_RINGERS_CONCLUDE;
                retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY,
                    &pTemp880->testTimer);
                pTempRingers->quickStop = TRUE;
                break;
            }

            /* Check hook status and error if something goes wrong */
            if ( VP_STATUS_SUCCESS !=
                VpGetLineStatus(pTestCtx->pLineCtx, VP_INPUT_RAW_HOOK, &temp)) {
                return LT_STATUS_ERROR_UNKNOWN;
            }


            if (TRUE == temp) {
                pResults->fltMask = LT_RNGM_REN_HIGH;
                pResults->measStatus = LT_MSRMNT_STATUS_PASSED;
                pResults->ren = LT_MAX_REN;
                pResults->rentg = LT_MAX_REN;
                pResults->renrg = LT_MAX_REN;

                pTemp880->testTimer.timerVal = LT_VP880_NEXT_TICK;
                *pState = CAP_RINGERS_CONCLUDE;
                retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY,
                    &pTemp880->testTimer);
                pTempRingers->quickStop = TRUE;
                break;

            }
        }

        case CAP_RINGERS_CALIBRATE:
            /* get fresh cal coeffs */
            *pState = CAP_RINGERS_REQUEST_FEED_VSAB;
            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_CALIBRATE, NULL);
            break;

        case CAP_RINGERS_SET_LINE_STATE_WITH_DELAY:
            if (VP880_RINGERS_TIP == pTempRingers->innerState) {
                VpSetLineState(pTestCtx->pLineCtx, VP_LINE_ACTIVE_POLREV);
            } else {
                VpSetLineState(pTestCtx->pLineCtx, VP_LINE_ACTIVE);
            }

            pTestCtx->pTemp->vp880Temp.testTimer.timerVal = 800;
            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY,
                &pTemp880->testTimer);
            *pState = CAP_RINGERS_REQUEST_FEED_VSAB;
            break;

        case CAP_RINGERS_REQUEST_FEED_VSAB:
            if (VP880_RINGERS_TIP == pTempRingers->innerState) {
                pTemp880->loopCond.settlingTime = (2000);
            } else {
            pTemp880->loopCond.settlingTime = LT_VP880_CAP_RINGERS_SETTLE_TIME;
            }

            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS,
                &pTemp880->loopCond);
            *pState = CAP_RINGERS_STORE_FEED_VSAB;
            break;

        case CAP_RINGERS_STORE_FEED_VSAB:
            pTempRingers->feedVADC = (uint16)pVpTestResult->result.loopCond.data;
            /* no break */

        case CAP_RINGERS_RAMP_TO_INPUT_V:
            *pState = CAP_RINGERS_RAMP_TO_OPPOSITE_INPUT_V;
            retval = LtVp880CapRampToInputV(pTestCtx, pTempRingers);
            break;

        case CAP_RINGERS_RAMP_TO_OPPOSITE_INPUT_V:
            *pState = CAP_RINGERS_STORE_RAMP_RESULT_1;
            retval = LtVp880CapRampUp(pTestCtx, pTempRingers);
            break;

        case CAP_RINGERS_STORE_RAMP_RESULT_1:
            if (VP880_RINGERS_DIFF == pTempRingers->innerState) {
                LtMemCpy(&pTempRingers->vpApiDiffRamp1,
                    &pVpTestResult->result.ramp,
                    sizeof(VpTestResultRampType));
            } else if (VP880_RINGERS_TIP == pTempRingers->innerState) {
                LtMemCpy(&pTempRingers->vpApiTipRamp1,
                    &pVpTestResult->result.ramp,
                    sizeof(VpTestResultRampType));
            } else {
                LtMemCpy(&pTempRingers->vpApiRingRamp1,
                    &pVpTestResult->result.ramp,
                    sizeof(VpTestResultRampType));
            }
            /* no break */

        case CAP_RINGERS_RAMP_BACK_TO_INPUT_V:
            *pState = CAP_RINGERS_STORE_RAMP_RESULT_2;
            retval = LtVp880CapRampDown(pTestCtx, pTempRingers);
            break;

        case CAP_RINGERS_STORE_RAMP_RESULT_2:
            if (VP880_RINGERS_DIFF == pTempRingers->innerState) {
                LtMemCpy(&pTempRingers->vpApiDiffRamp2,
                    &pVpTestResult->result.ramp,
                    sizeof(VpTestResultRampType));
            } else if (VP880_RINGERS_TIP == pTempRingers->innerState) {
                LtMemCpy(&pTempRingers->vpApiTipRamp2,
                    &pVpTestResult->result.ramp,
                    sizeof(VpTestResultRampType));
            } else {
                LtMemCpy(&pTempRingers->vpApiRingRamp2,
                    &pVpTestResult->result.ramp,
                    sizeof(VpTestResultRampType));
            }
            /* no break */

        case CAP_RINGERS_RAMP_TO_FEED:
            retval = LtVp880CapRampToFeed(pTestCtx, pTempRingers, pState);
            break;

        case CAP_RINGERS_CONCLUDE:
            if (FALSE == pTemp880->internalTest) {
                VpTestConcludeType conclude = { FALSE };
                /* conclude the test if not an internal test*/
                retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE,
                    &conclude);
                *pState = CAP_RINGERS_COMPLETE;
                break;
            }

        case CAP_RINGERS_COMPLETE:
            /* Calculate results and end the test */
            if (!pTempRingers->quickStop) {
                LtVp880CalculateResults(pTestCtx, LT_TID_RINGERS);
            }
            retval = LT_STATUS_DONE;
            break;

        case CAP_RINGERS_ABORT:
            /* Test aborted, results may indicate source of error. */
            retval = LT_STATUS_ABORTED;
            break;

        default:
            /* This should never happen */
            break;
     }

    return retval;
}


LtTestStatusType
LtVp880CapRampToInputV(
    LtTestCtxType *pTestCtx,
    LtVp880RingersTempType *pTempRingers)
{
    VpTestRampInitType rampInitInput;
    LtVoltageType inputV = ABS(pTempRingers->input.vRingerTest);
    pTempRingers->biasVADC = 0;

    if (VP880_RINGERS_DIFF != pTempRingers->innerState) {
        int16 delta = 4369;
        inputV /= 2;

        /* adjusting the feed voltage for tip/ring to gnd measurments */
        if (0 > pTempRingers->feedVADC) {
            delta *= -1;
        }
        pTempRingers->feedVADC = ((pTempRingers->feedVADC - delta) / 2);

        /* make bias larger than ramp voltage */
        pTempRingers->biasVADC =
            (int16)VP880_UNIT_CONVERT((inputV + 10000), VP880_UNIT_MV, VP880_UNIT_ADC_VAB);
    }

    pTempRingers->inputVADC =
        (int16)VP880_UNIT_CONVERT(inputV, VP880_UNIT_MV, VP880_UNIT_ADC_VAB);

    rampInitInput.vRate = LT_VP880_CAP_RINGERS_RAMP_SLOPE;
    rampInitInput.dcVstart = -ABS(pTempRingers->feedVADC);
    rampInitInput.dcVend = pTempRingers->inputVADC;
    rampInitInput.bias = pTempRingers->biasVADC;
    rampInitInput.tip = LtVp880CapGetCurrentLed(pTempRingers);

    /* Ramp to initial V */
    return Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_RAMP_INIT, &rampInitInput);
}

LtTestStatusType
LtVp880CapRampUp(
    LtTestCtxType *pTestCtx,
    LtVp880RingersTempType *pTempRingers)
{
    VpTestRampType rampInput;

    rampInput.vRate = LT_VP880_CAP_RINGERS_VRATE(pTempRingers->input.renFactor);

    /* Metalic Ramp to opposite of requested V */
    rampInput.dcVoltage = -(pTempRingers->inputVADC * 2);
    rampInput.bias = pTempRingers->biasVADC;
    rampInput.tip = LtVp880CapGetCurrentLed(pTempRingers);

    return Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_RAMP, &rampInput);
}

LtTestStatusType
LtVp880CapRampDown(
    LtTestCtxType *pTestCtx,
    LtVp880RingersTempType *pTempRingers)
{
    VpTestRampType rampInput;
    rampInput.vRate = LT_VP880_CAP_RINGERS_VRATE(pTempRingers->input.renFactor);


    /* Metalic Ramp to back to requested V */
    rampInput.dcVoltage = (pTempRingers->inputVADC * 2);
    rampInput.bias = pTempRingers->biasVADC;
    rampInput.tip = LtVp880CapGetCurrentLed(pTempRingers);

    return Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_RAMP, &rampInput);
}

LtTestStatusType
LtVp880CapRampToFeed(
    LtTestCtxType *pTestCtx,
    LtVp880RingersTempType *pTempRingers,
    int *pState)
{
    VpTestRampType rampInput;

    /* Metalic Ramp back to original feed voltage */
    rampInput.vRate = LT_VP880_CAP_RINGERS_RAMP_SLOPE;
    rampInput.dcVoltage = -(ABS(pTempRingers->feedVADC) - ABS(pTempRingers->inputVADC));
    rampInput.bias = pTempRingers->biasVADC;
    rampInput.tip = LtVp880CapGetCurrentLed(pTempRingers);

    /* if 3 ele ringer is requested stay here until all 3 are measured */
    if ((VP880_RINGERS_NUM_INNER_STATES > ++(pTempRingers->innerState)) &&
        (LT_RINGER_ELECTRONIC_PHNE_TEST_3_ELE == pTempRingers->input.ringerTestType)) {
        *pState = CAP_RINGERS_SET_LINE_STATE_WITH_DELAY;
    } else {
        *pState = CAP_RINGERS_CONCLUDE;
    }
    return Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_RAMP, &rampInput);
}

VpTestTipSelectType
LtVp880CapGetCurrentLed(
    LtVp880RingersTempType *pTempRingers) {

    if (VP880_RINGERS_DIFF == pTempRingers->innerState) {
        return VP_TEST_TIP_RING;
    } else if (VP880_RINGERS_TIP == pTempRingers->innerState) {
        return VP_TEST_TIP;
    }

    return VP_TEST_RING;
}

#endif /* LT_VP880_VVP_PACKAGE */
#endif /* LT_VP880_PACKAGE && VP880_INCLUDE_TESTLINE_CODE */
