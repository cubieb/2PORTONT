/** \lt_api_test_on_off_hook_st.h
 * lt_api_test_on_off_hook_st.h
 *
 * This file implements the on/off hook self test.
 *
 * This file assumes a certain include order to compile.
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 5264 $
 * $LastChangedDate: 2009-07-06 10:22:54 -0500 (Mon, 06 Jul 2009) $
 */

#include "lt_api.h"

#if defined(VP890_INCLUDE_TESTLINE_CODE) && defined(LT_VP890_PACKAGE) && defined(LT_DC_VOLTAGE)
#ifdef LT_VP890_VVP_PACKAGE

#include "lt_api_internal.h"
#include "lt_api_vp890.h"

typedef enum {
    DC_VOLTAGE_INIT             = LT_GLOBAL_FIRST_STATE,
    DC_VOLTAGE_SET_LINE_STATE,
    DC_VOLTAGE_CALIBRATE,
    DC_VOLTAGE_REQUEST_FEED_VSAB,
    DC_VOLTAGE_STORE_FEED_VSAB,
    DC_VOLTAGE_RAMP_TO_INPUT_V,
    DC_VOLTAGE_REQUEST_VSAB_1,
    DC_VOLTAGE_STORE_VSAB_1,
    DC_VOLTAGE_RAMP_TO_OPPOSITE_V_1,
    DC_VOLTAGE_RAMP_TO_OPPOSITE_V_2,
    DC_VOLTAGE_REQUEST_VSAB_2,
    DC_VOLTAGE_STORE_VSAB_2,
    DC_VOLTAGE_RAMP_TO_FEED_1,
    DC_VOLTAGE_RAMP_TO_FEED_2,
    DC_VOLTAGE_CONCLUDE,
    DC_VOLTAGE_COMPLETE,
    DC_VOLTAGE_ABORT            = LT_GLOBAL_ABORTED_STATE,
    DC_VOLTAGE_ENUM_SIZE        = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp890OnOffHookStates;

void
LtVp890DcVoltageSetup(
    LtTestCtxType *pTestCtx);

extern LtTestStatusType
LtVp890DcVoltageHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent)
{
    LtTestStatusType retval = LT_STATUS_ERROR_UNKNOWN;
    VpTestResultType *pVpTestResult = (VpTestResultType*)pEvent->pResult;
    LtVp890TestTempType *pTemp890 = &pTestCtx->pTemp->vp890Temp;
    LtVp890DcVoltageTempType *pTempDcVolt = &pTemp890->tempData.dcVoltage;
    int *pState = &pTestCtx->state;

    LT_DOUT(LT_DBG_INFO, ("LtVp890DcVoltageHandler(testState:%i)", *pState));

    /* Check for, store and return any VP-API or LT_API errors */
    if (TRUE == Vp890EventErrorCheck(pTestCtx, pEvent, &retval)) {
        return retval;
    }

    switch (*pState) {

        case DC_VOLTAGE_INIT:
            /* This test will use this loop cond struct over and over. */
            pTemp890->loopCond.calMode = FALSE;
            pTemp890->loopCond.integrateTime =  LT_VP890_DC_VOLTAGE_INTEGRATE_TIME;
            pTemp890->loopCond.settlingTime = LT_VP890_DC_VOLTAGE_SETTLE_TIME;
            pTemp890->loopCond.loopCond = VP_LOOP_COND_TEST_VSAB;
            /* no break */

        case DC_VOLTAGE_SET_LINE_STATE: {
            VpLineStateType lineState = VP_LINE_ACTIVE;
            VpLineStateType lineStateCurrent = pTemp890->resources.initialLineState;
            pTemp890->testTimer.timerVal = LT_VP890_STATE_CHANGE_SETTLE;
                        
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

            *pState = DC_VOLTAGE_CALIBRATE;
            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY, &pTemp890->testTimer);
            break;
        }

        case DC_VOLTAGE_CALIBRATE:
            /* get fresh cal coeffs */
            *pState = DC_VOLTAGE_REQUEST_FEED_VSAB;
            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_CALIBRATE, NULL);
            break;

        case DC_VOLTAGE_REQUEST_FEED_VSAB:
            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS,
                &pTemp890->loopCond);
            *pState = DC_VOLTAGE_STORE_FEED_VSAB;
            break;

        case DC_VOLTAGE_STORE_FEED_VSAB:
            /* store return value from initial vsab read */
            pTempDcVolt->feedV = (uint16)pVpTestResult->result.loopCond.data;
            /* construct the rest of the test structs based on feedV result*/
            LtVp890DcVoltageSetup(pTestCtx);
            /* no break */

        case DC_VOLTAGE_RAMP_TO_INPUT_V:
            *pState = DC_VOLTAGE_REQUEST_VSAB_1;
            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_RAMP_INIT,
                &pTempDcVolt->vpApiRampInitInput);
            break;

        case DC_VOLTAGE_REQUEST_VSAB_1:
            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS,
                &pTemp890->loopCond);
            *pState = DC_VOLTAGE_STORE_VSAB_1;
            break;

        case DC_VOLTAGE_STORE_VSAB_1:
            /* store return value from vsab 1 read */
            pTempDcVolt->adcVsab1 = (uint16)pVpTestResult->result.loopCond.data;
            /* no break */

        case DC_VOLTAGE_RAMP_TO_OPPOSITE_V_1:
             *pState = DC_VOLTAGE_RAMP_TO_OPPOSITE_V_2;
            /*
             * If the |pk ramp voltage| is less than +75V then move to the
             * opposite voltage in one step by doubling the calculated dcVoltage
             * and then move to the DC_VOLTAGE_REQUEST_VSAB_2 state.
             */
            if (VP890_UNIT_CONVERT(-75000, VP890_UNIT_MV, VP890_UNIT_ADC_VAB)
                > ABS(pTempDcVolt->vpApiRampInput1.dcVoltage)) {
                pTempDcVolt->vpApiRampInput1.dcVoltage *= 2;
                *pState = DC_VOLTAGE_REQUEST_VSAB_2;
            }
            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_RAMP,
                &pTempDcVolt->vpApiRampInput1);
            break;

        case DC_VOLTAGE_RAMP_TO_OPPOSITE_V_2:
            *pState = DC_VOLTAGE_REQUEST_VSAB_2;
            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_RAMP,
                &pTempDcVolt->vpApiRampInput1);
            break;

        case DC_VOLTAGE_REQUEST_VSAB_2:
            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS,
                &pTemp890->loopCond);
            *pState = DC_VOLTAGE_STORE_VSAB_2;
            break;

        case DC_VOLTAGE_STORE_VSAB_2:
            /* store return value from vsab 2 read */
            pTempDcVolt->adcVsab2 = (uint16)pVpTestResult->result.loopCond.data;
            /* no break */

        case DC_VOLTAGE_RAMP_TO_FEED_1:
            *pState = DC_VOLTAGE_RAMP_TO_FEED_2;
            /*
             * Complete the test by ramping back to the original feed voltage.
             * If the |pk ramp voltage| is less than +75V then move to the
             * opposite voltage in one step by doubling the calculated dcVoltage
             * and then conclude test.
             */
            if ( VP890_UNIT_CONVERT(-75000, VP890_UNIT_MV, VP890_UNIT_ADC_VAB)
                > ABS(pTempDcVolt->vpApiRampInput2.dcVoltage)) {
                *pState = DC_VOLTAGE_CONCLUDE;
                pTempDcVolt->vpApiRampInput2.dcVoltage *= 2;
            }
            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_RAMP,
                &pTempDcVolt->vpApiRampInput2);
            break;

        case DC_VOLTAGE_RAMP_TO_FEED_2:
            *pState = DC_VOLTAGE_CONCLUDE;
            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_RAMP,
                &pTempDcVolt->vpApiRampInput2);
            break;

        case DC_VOLTAGE_CONCLUDE:
            if (FALSE == pTemp890->internalTest) {
                VpTestConcludeType conclude = { FALSE };
                retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE,
                    &conclude);
                *pState = DC_VOLTAGE_COMPLETE;
                break;
            }

        case DC_VOLTAGE_COMPLETE:
            /* Calculate results and end the test */
            LtVp890CalculateResults(pTestCtx, LT_TID_DC_VOLTAGE);
            retval = LT_STATUS_DONE;
            break;

        case DC_VOLTAGE_ABORT:
            /* Test aborted, results may indicate source of error. */
            retval = LT_STATUS_ABORTED;
            break;

        default:
            /* This should never happen */
            retval = LT_STATUS_ERROR_UNKNOWN;
            break;
     }

    return retval;
}


/*
 * This function is used to setup the rest of the test primitive inputs
 * all at once.
 */
void
LtVp890DcVoltageSetup(
    LtTestCtxType *pTestCtx)
{
    LtVp890DcVoltageTempType *pTempDcVolt = &pTestCtx->pTemp->vp890Temp.tempData.dcVoltage;
    LtVoltageType inputV = pTempDcVolt->input.testVoltage;
    int16 inputVADC =
        (int16)VP890_UNIT_CONVERT(inputV, VP890_UNIT_MV, VP890_UNIT_ADC_VAB);
    int16 feedVADC = pTempDcVolt->feedV;
    int32 temp;

    /* Init Metalic Ramp to initial V*/
    pTempDcVolt->vpApiRampInitInput.vRate = LT_VP890_DC_VOLTAGE_RAMP_SLOPE;
    pTempDcVolt->vpApiRampInitInput.dcVstart = -ABS(feedVADC);
    pTempDcVolt->vpApiRampInitInput.dcVend = inputVADC;
    pTempDcVolt->vpApiRampInitInput.bias = 0;
    pTempDcVolt->vpApiRampInitInput.tip = VP_TEST_TIP_RING;

    /* Metalic Ramp to opposite V in one step*/
    pTempDcVolt->vpApiRampInput1.vRate = LT_VP890_DC_VOLTAGE_RAMP_SLOPE;
    pTempDcVolt->vpApiRampInput1.dcVoltage = -inputVADC;
    pTempDcVolt->vpApiRampInput1.bias = 0;
    pTempDcVolt->vpApiRampInput1.tip = VP_TEST_TIP_RING;

    /* Metalic Ramp back to original feed voltage */
    pTempDcVolt->vpApiRampInput2.vRate = LT_VP890_CAP_RINGERS_RAMP_SLOPE;
    temp = -(ABS(feedVADC)+ABS(inputVADC)) / 2;
    pTempDcVolt->vpApiRampInput2.dcVoltage = (int16)temp;
    pTempDcVolt->vpApiRampInput1.bias = 0;
    pTempDcVolt->vpApiRampInput1.tip = VP_TEST_TIP_RING;

    return;
}
#endif /* LT_VP890_VVP_PACKAGE */
#endif /* LT_VP890_PACKAGE && VP890_INCLUDE_TESTLINE_CODE */
