/** file lt_api_vp890_line_v.c
 *
 *  This file contains the PASS/FAIL Line Voltage test.
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 5264 $
 * $LastChangedDate: 2009-07-06 10:22:54 -0500 (Mon, 06 Jul 2009) $
 */

#include "lt_api.h"

#if defined(VP890_INCLUDE_TESTLINE_CODE) && defined(LT_VP890_PACKAGE) && defined(LT_LINE_V)

#include "lt_api_internal.h"
#include "lt_api_vp890.h"

typedef enum {
    LINE_V_INIT             = LT_GLOBAL_FIRST_STATE,
    LINE_V_BEGIN,
    LINE_V_MEASUREMENTS,
    LINE_V_CONCLUDE,
    LINE_V_CALCULATE,
    LINE_V_COMPLETE,
    LINE_V_ABORT            = LT_GLOBAL_ABORTED_STATE,
    LINE_V_ENUM_SIZE        = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp890RohStates;

LtTestStatusType
LtVp890LineVMeasureVoltage(
    LtTestCtxType *pTestCtx);

extern LtTestStatusType
LtVp890LineVHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent)
{
    LtTestStatusType retval = LT_STATUS_ERROR_UNKNOWN;
    VpTestResultType *pVpTestResult = (VpTestResultType*)pEvent->pResult;
    LtVp890TestTempType *pTemp890 = &pTestCtx->pTemp->vp890Temp;
    LtVp890LineVTempType *pTempLineV = &pTemp890->tempData.lineV;
    LtLineVResultType *pResults = &pTestCtx->pResult->result.lineV;
    int *pState = &(pTestCtx->state);

    LT_DOUT(LT_DBG_INFO, ("LtVp890LineVHandler(testState:%i)", *pState));

    /* Check for, store and return any VP-API or LT_API errors */
    if (TRUE == Vp890EventErrorCheck(pTestCtx, pEvent, &retval)) {
        return retval;
    }

    switch (*pState) {
        case LINE_V_INIT:
            if (pTemp890->internalTest == TRUE) {
                LtTestResultType resTmp;

                LtInitOutputStruct(&resTmp, LT_TID_LINE_V);
                LtMemCpy(pResults, &(resTmp.result.lineV), sizeof(LtLineVResultType));
            }
            /* no break */

        case LINE_V_BEGIN:
            pTempLineV->innerState = VP890_LINE_V_TIP;
            retval = LtVp890LineVMeasureVoltage(pTestCtx);
            *pState = LINE_V_MEASUREMENTS;
            break;

        case LINE_V_MEASUREMENTS:
            /* get the api result */
            retval = LT_STATUS_RUNNING;
            /* store the api result for later calculations*/
            if (VP890_LINE_V_TIP == pTempLineV->innerState) {
                LtMemCpy(&pTempLineV->vpApiTipResults, &pVpTestResult->result.vxc,
                    sizeof(VpTestResultVxcType));
            } else if (VP890_LINE_V_RING == pTempLineV->innerState) {
                LtMemCpy(&pTempLineV->vpApiRingResults, &pVpTestResult->result.vxc,
                    sizeof(VpTestResultVxcType));
            } else {
                LtMemCpy(&pTempLineV->vpApiDiffResults, &pVpTestResult->result.vxc,
                    sizeof(VpTestResultVxcType));
            }

            if (++(pTempLineV->innerState) < VP890_LINE_V_NUM_INNER_STATES) {
                /* move back to the prepare state until all data is collected */
                retval = LtVp890LineVMeasureVoltage(pTestCtx);
                break;
            }
            /* no break */

        case LINE_V_CALCULATE:
            /* Calculate results and end the test */
            LtVp890CalculateResults(pTestCtx, LT_TID_LINE_V);

            /* if the test passes put relay into normal mode */
            if (LT_TEST_PASSED == pTestCtx->pResult->result.lineV.fltMask) {
                VpSetRelayState(pTestCtx->pLineCtx,
                    pTemp890->resources.initialRelayState);

            } else {
                /*
                 * override the initial Line State so that the slic
                 * and the foreign voltage are not both applied to the
                 * line when the test concludes
                 *
                 * override the relay state flag so that the upper level
                 * code does not try to restore the initial relay state.
                 */
                pTemp890->resources.initialLineState = VP_LINE_DISCONNECT;
                pTemp890->resources.relayStRestore = FALSE;
            }
            /* no break */

        case LINE_V_CONCLUDE:
            if (pTemp890->internalTest == FALSE) {
                VpTestConcludeType conclude = { FALSE };
                /* conclude the test if not an internal test*/
                retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE,
                    &conclude);
                *pState = LINE_V_COMPLETE;
                break;
            }

        case LINE_V_COMPLETE:
            retval = LT_STATUS_DONE;
            break;

        case LINE_V_ABORT:
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

LtTestStatusType
LtVp890LineVMeasureVoltage(
    LtTestCtxType *pTestCtx)
{
    LtVp890LineVTempType *pTempLineV = &pTestCtx->pTemp->vp890Temp.tempData.lineV;

    VpTestOpenVType openVxcInput;
    openVxcInput.calMode = FALSE;
    openVxcInput.settlingTime = LT_VP890_LINE_V_SETTLE_TIME;
    openVxcInput.integrateTime = LT_VP890_LINE_V_INTEGRATE_TIME;

    switch (pTempLineV->innerState) {
        case VP890_LINE_V_TIP:
            openVxcInput.tip = VP_TEST_TIP;
            openVxcInput.calMode = TRUE;
            break;
        case VP890_LINE_V_RING:
            openVxcInput.tip = VP_TEST_RING;
            break;
        case VP890_LINE_V_DIFF:
            openVxcInput.tip = VP_TEST_TIP_RING;
            break;
        default:
            return LT_STATUS_ERROR_UNKNOWN;
    }
    /* start a measurment */
    return Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_OPEN_VXC, &openVxcInput);
}

#endif /* LT_VP890_PACKAGE && VP890_INCLUDE_TESTLINE_CODE */
