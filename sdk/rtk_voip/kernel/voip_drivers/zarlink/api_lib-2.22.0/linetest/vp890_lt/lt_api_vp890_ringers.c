/** file lt_api_vp890_ringers.c
 *
 *  This file contains the GR-909 PASS/FAIL Ringers Test.
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 5264 $
 * $LastChangedDate: 2009-07-06 10:22:54 -0500 (Mon, 06 Jul 2009) $
 */

#include "lt_api.h"

#if defined(VP890_INCLUDE_TESTLINE_CODE) && defined(LT_VP890_PACKAGE) && defined(LT_RINGERS)

#include "lt_api_internal.h"
#include "lt_api_vp890.h"

typedef enum {
    RINGERS_INIT            = LT_GLOBAL_FIRST_STATE,
    RINGERS_LOW_POWER_CHECK,
    RINGERS_IMPED_START,
    RINGERS_IMPED_STORE_RESULT,
    RINGERS_CONCLUDE,
    RINGERS_COMPLETE,
    RINGERS_ABORT           = LT_GLOBAL_ABORTED_STATE,
    RINGERS_ENUM_SIZE       = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp890RenStates;

VpTestTipSelectType
LtVp890RingerGetCurrentLed(
    LtVp890RingersTempType *pTempRingers);

extern LtTestStatusType
LtVp890RingerHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent)
{
    LtTestStatusType retval = LT_STATUS_ERROR_UNKNOWN;
    VpTestResultType *pVpTestResult = (VpTestResultType*)pEvent->pResult;
    LtVp890TestTempType *pTemp890 = &pTestCtx->pTemp->vp890Temp;
    LtVp890RingersTempType *pTempRingers = &pTemp890->tempData.ringers;
    LtRingersResultType *pResults = &pTestCtx->pResult->result.ringers;
    int *pState = &(pTestCtx->state);

    LT_DOUT(LT_DBG_INFO, ("LtVp890RingerHandler(testState:%i)", *pState));
    /* Check for, store and return any VP-API or LT_API errors */
    if (TRUE == Vp890EventErrorCheck(pTestCtx, pEvent, &retval)) {
        return retval;
    }

    switch (*pState) {
        case RINGERS_INIT:
            if (pTemp890->internalTest == TRUE) {
                LtInitOutputStruct((LtTestResultType *)pResults, LT_TID_RINGERS);
            }
            pResults->ringerTestType = pTempRingers->input.ringerTestType;
            pTempRingers->innerState = 0;

            /* setup the inputs */
            pTempRingers->acRloopInput.tip = VP_TEST_TIP_RING;
            pTempRingers->acRloopInput.calMode = TRUE;
            pTempRingers->acRloopInput.vBias = LT_VP890_RINGERS_TEST_BIAS;
            pTempRingers->acRloopInput.freq = pTempRingers->input.freq / 1000;
            pTempRingers->acRloopInput.settlingTime = LT_VP890_RINGERS_SETTLE_TIME;
            pTempRingers->acRloopInput.vTestLevel = (int16)VP890_UNIT_CONVERT(
                pTempRingers->input.vRingerTest, VP890_UNIT_MV, VP890_UNIT_DAC_RING);
            pTempRingers->acRloopInput.integrateTime = (uint16)
                ((LT_VP890_RINGERS_RING_CYCLES * 8000l) / pTempRingers->acRloopInput.freq);
            /* no break */

        case RINGERS_LOW_POWER_CHECK:
        case RINGERS_IMPED_START:
            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_AC_RLOOP,
                &pTempRingers->acRloopInput);
            *pState = RINGERS_IMPED_STORE_RESULT;
            break;

        case RINGERS_IMPED_STORE_RESULT:
            /* Store the results and move to the next step */
            if (VP890_RINGERS_DIFF == pTempRingers->innerState) {
                LtMemCpy(&pTempRingers->vpApiAcR1Results1, &pVpTestResult->result.acimt,
                    sizeof(VpTestResultAcRlType));
            } else if ( VP890_RINGERS_RING == pTempRingers->innerState) {
                LtMemCpy(&pTempRingers->vpApiAcR1Results2, &pVpTestResult->result.acimt,
                    sizeof(VpTestResultAcRlType));
            } else {
                LtMemCpy(&pTempRingers->vpApiAcR1Results3, &pVpTestResult->result.acimt,
                    sizeof(VpTestResultAcRlType));
            }

            /* if 3 ele ringer is requested stay here until all 3 are measured */
            if ((VP890_RINGERS_NUM_INNER_STATES > ++(pTempRingers->innerState)) &&
                (LT_RINGER_REGULAR_PHNE_TEST_3_ELE == pTempRingers->input.ringerTestType)) {

                pTempRingers->acRloopInput.tip = LtVp890RingerGetCurrentLed(pTempRingers);

                retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_AC_RLOOP,
                    &pTempRingers->acRloopInput);
                *pState = RINGERS_IMPED_STORE_RESULT;
                break;
            }
            /* no break */

        case RINGERS_CONCLUDE:
            if (FALSE == pTemp890->internalTest) {
                /* conclude the test if not an internal test*/
                VpTestConcludeType conclude = { FALSE };
                retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE,
                    &conclude);
                *pState = RINGERS_COMPLETE;
                break;
            }

        case RINGERS_COMPLETE:
            /* Calculate results and end the test */
            LtVp890CalculateResults(pTestCtx, LT_TID_RINGERS);
            retval = LT_STATUS_DONE;
            break;

        case RINGERS_ABORT:
            /* Test aborted, results may indicate source of error. */
            retval = LT_STATUS_ABORTED;
            break;

        default:
            /* This should never happen */
            break;
     }

    return retval;
}

VpTestTipSelectType
LtVp890RingerGetCurrentLed(
    LtVp890RingersTempType *pTempRingers) {

    if (VP890_RINGERS_DIFF == pTempRingers->innerState) {
        return VP_TEST_TIP_RING;
    } else if (VP890_RINGERS_TIP == pTempRingers->innerState) {
        return VP_TEST_TIP;
    }

    return VP_TEST_RING;
}

#endif /* LT_VP890_PACKAGE && VP890_INCLUDE_TESTLINE_CODE*/
