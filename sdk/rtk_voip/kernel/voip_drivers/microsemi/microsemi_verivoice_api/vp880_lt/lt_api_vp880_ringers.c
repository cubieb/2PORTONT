/** file lt_api_vp880_ringers.c
 *
 *  This file contains the GR-909 PASS/FAIL Ringers Test.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 9115 $
 * $LastChangedDate: 2011-11-15 15:25:17 -0600 (Tue, 15 Nov 2011) $
 */

#include "lt_api.h"

#if defined(VP880_INCLUDE_TESTLINE_CODE) && defined(LT_VP880_PACKAGE) && defined(LT_RINGERS)

#include "lt_api_internal.h"
#include "lt_api_vp880.h"

typedef enum {
    RINGERS_INIT            = LT_GLOBAL_FIRST_STATE,
    RINGERS_LOW_POWER_CHECK,
    RINGERS_IMPED_START,
    RINGERS_IMPED_STORE_RESULT,
    RINGERS_CONCLUDE,
    RINGERS_COMPLETE,
    RINGERS_ABORT           = LT_GLOBAL_ABORTED_STATE,
    RINGERS_ENUM_SIZE       = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp880RenStates;

VpTestTipSelectType
LtVp880RingerGetCurrentLed(
    LtVp880RingersTempType *pTempRingers);

extern LtTestStatusType
LtVp880RingerHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent)
{
    LtTestStatusType retval = LT_STATUS_ERROR_UNKNOWN;
    VpTestResultType *pVpTestResult = (VpTestResultType*)pEvent->pResult;
    LtVp880TestTempType *pTemp880 = &pTestCtx->pTemp->vp880Temp;
    LtVp880RingersTempType *pTempRingers = &pTemp880->tempData.ringers;
    LtRingersResultType *pResults = &pTestCtx->pResult->result.ringers;
    int *pState = &(pTestCtx->state);

    LT_DOUT(LT_DBG_INFO, ("LtVp880RingerHandler(testState:%i)", *pState));
    /* Check for, store and return any VP-API or LT_API errors */
    if (TRUE == Vp880EventErrorCheck(pTestCtx, pEvent, &retval)) {
        return retval;
    }

    switch (*pState) {
        case RINGERS_INIT:
            if (pTemp880->internalTest == TRUE) {
                LtInitOutputStruct((LtTestResultType *)pResults, LT_TID_RINGERS);
            }
            pResults->ringerTestType = pTempRingers->input.ringerTestType;
            pTempRingers->innerState = 0;

            /* setup the inputs */
            pTempRingers->acRloopInput.tip = VP_TEST_TIP_RING;
            pTempRingers->acRloopInput.calMode = TRUE;
            pTempRingers->acRloopInput.vBias = LT_VP880_RINGERS_TEST_BIAS;
            pTempRingers->acRloopInput.freq = pTempRingers->input.freq / 1000;
            pTempRingers->acRloopInput.settlingTime = LT_VP880_RINGERS_SETTLE_TIME;
            pTempRingers->acRloopInput.vTestLevel = (int16)VP880_UNIT_CONVERT(
                pTempRingers->input.vRingerTest, VP880_UNIT_MV, VP880_UNIT_DAC_RING);
            pTempRingers->acRloopInput.integrateTime = (uint16)
                ((LT_VP880_RINGERS_RING_CYCLES * 8000l) / pTempRingers->acRloopInput.freq);
            /* no break */

        case RINGERS_LOW_POWER_CHECK:
        case RINGERS_IMPED_START:
            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_AC_RLOOP,
                &pTempRingers->acRloopInput);
            *pState = RINGERS_IMPED_STORE_RESULT;
            break;

        case RINGERS_IMPED_STORE_RESULT:
            /* Store the results and move to the next step */
            if (VP880_RINGERS_DIFF == pTempRingers->innerState) {
                LtMemCpy(&pTempRingers->vpApiAcR1Results1, &pVpTestResult->result.acimt,
                    sizeof(VpTestResultAcRlType));
            } else if ( VP880_RINGERS_RING == pTempRingers->innerState) {
                LtMemCpy(&pTempRingers->vpApiAcR1Results2, &pVpTestResult->result.acimt,
                    sizeof(VpTestResultAcRlType));
            } else {
                LtMemCpy(&pTempRingers->vpApiAcR1Results3, &pVpTestResult->result.acimt,
                    sizeof(VpTestResultAcRlType));
            }

            /* if 3 ele ringer is requested stay here until all 3 are measured */
            if ((VP880_RINGERS_NUM_INNER_STATES > ++(pTempRingers->innerState)) &&
                (LT_RINGER_REGULAR_PHNE_TEST_3_ELE == pTempRingers->input.ringerTestType)) {

                pTempRingers->acRloopInput.tip = LtVp880RingerGetCurrentLed(pTempRingers);

                retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_AC_RLOOP,
                    &pTempRingers->acRloopInput);
                *pState = RINGERS_IMPED_STORE_RESULT;
                break;
            }
            /* no break */

        case RINGERS_CONCLUDE:
            if (FALSE == pTestCtx->pTemp->vp880Temp.internalTest) {
                VpTestConcludeType conclude = { FALSE };
                /* conclude the test if not an internal test*/
                retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE,
                    &conclude);
                *pState = RINGERS_COMPLETE;
                break;
            }

        case RINGERS_COMPLETE:
            /* Calculate results and end the test */
            LtVp880CalculateResults(pTestCtx, LT_TID_RINGERS);
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
LtVp880RingerGetCurrentLed(
    LtVp880RingersTempType *pTempRingers) {

    if (VP880_RINGERS_DIFF == pTempRingers->innerState) {
        return VP_TEST_TIP_RING;
    } else if (VP880_RINGERS_TIP == pTempRingers->innerState) {
        return VP_TEST_TIP;
    }

    return VP_TEST_RING;
}

#endif /* LT_VP880_PACKAGE && VP880_INCLUDE_TESTLINE_CODE*/
