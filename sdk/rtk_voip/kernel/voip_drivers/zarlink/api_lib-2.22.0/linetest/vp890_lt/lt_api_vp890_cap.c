/** file lt_api_vp890_cap.c
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 5490 $
 * $LastChangedDate: 2009-08-21 13:59:29 -0500 (Fri, 21 Aug 2009) $
 */

#include "lt_api.h"

#if defined(VP890_INCLUDE_TESTLINE_CODE) && defined(LT_VP890_PACKAGE) && defined(LT_CAP)
#ifdef LT_VP890_VVP_PACKAGE

#include "lt_api_internal.h"
#include "lt_api_vp890.h"

#define LT_CAP_THRESHOLD  50000     /* 50nF */
#define LT_CAP_LOW_VOLTAGE  600     /* 600mV */

typedef enum {
    CAP_INIT            = LT_GLOBAL_FIRST_STATE,
    CAP_MEASURE_TIP,
    CAP_GET_RESULTS_TIP,
    CAP_MEASURE_RING,
    CAP_GET_RESULTS_RING,
    CAP_CALCULTATE,
    CAP_CONCLUDE,
    CAP_COMPLETE,
    CAP_ABORT           = LT_GLOBAL_ABORTED_STATE,
    CAP_ENUM_SIZE       = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp890RenStates;

static bool
isCapBelowThreshold(
    LtTestCtxType *pTestCtx,
    int32 threshold);

extern LtTestStatusType
LtVp890CapHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent)
{
    LtTestStatusType retval = LT_STATUS_ERROR_UNKNOWN;
    VpTestResultType *pVpTestResult = (VpTestResultType*)pEvent->pResult;
    LtVp890TestTempType *pTemp890 = &pTestCtx->pTemp->vp890Temp;
    LtCapResultType *pResults = &pTestCtx->pResult->result.cap;
    LtVp890CapTempType *pTempCap = &pTemp890->tempData.cap;
    LtCapTestFreqType *pTestFreq = &pTempCap->input.testFreq;
    LtVoltageType *pTestAmp = &pTempCap->input.testAmp;
    int *pState = &(pTestCtx->state);

    LT_DOUT(LT_DBG_INFO, ("LtVp890CapHandler(testState:%i)", *pState));
    /* Check for, store and return any VP-API or LT_API errors */
    if (TRUE == Vp890EventErrorCheck(pTestCtx, pEvent, &retval)) {
        return retval;
    }

    switch (*pState) {
        case CAP_INIT:
            pTempCap->ctg = LT_IMPEDANCE_NOT_MEASURED;
            pTempCap->crg = LT_IMPEDANCE_NOT_MEASURED;
            pTempCap->ctr = LT_IMPEDANCE_NOT_MEASURED;
            /* no break */

        case CAP_MEASURE_TIP: {
            VpTest3EleCapAltResType capInput;

            capInput.calMode = FALSE;
            capInput.testFreq = (uint8)(*pTestFreq);
            capInput.testAmp = (uint16)(*pTestAmp);
            capInput.bias = -3362;  /* -16V */

            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_3ELE_CAP_CSLAC, &capInput);
            *pState = CAP_GET_RESULTS_TIP;
            break;
        }

        case CAP_GET_RESULTS_TIP:
            /* store all api cap results into the temp structure */
            if (pVpTestResult->result.capAlt.accuracyFlag == TRUE) {
                pResults->measStatus = LT_MSRMNT_STATUS_DEGRADED_ACCURACY;
            }
            LtMemCpy(&pTempCap->vpApiCapResults1, &pVpTestResult->result.capAlt,
                sizeof(VpTestResultAltCapType));
            /* no break */

        case CAP_MEASURE_RING: {
            VpTest3EleCapAltResType capInput;

            capInput.calMode = FALSE;
            capInput.testFreq = (uint8)(*pTestFreq);
            capInput.testAmp = (uint16)(*pTestAmp);
            capInput.bias = 3362;  /* 16V */

            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_3ELE_CAP_CSLAC, &capInput);
            *pState = CAP_GET_RESULTS_RING;
            break;
        }

        case CAP_GET_RESULTS_RING:
            /* store all api cap results into the temp structure */
            if (pVpTestResult->result.capAlt.accuracyFlag == TRUE) {
                pResults->measStatus = LT_MSRMNT_STATUS_DEGRADED_ACCURACY;
            }
            LtMemCpy(&pTempCap->vpApiCapResults2, &pVpTestResult->result.capAlt,
                sizeof(VpTestResultAltCapType));
            /* no break */

        case CAP_CALCULTATE:
            /* Calculate results */
            LtVp890CalculateResults(pTestCtx, LT_TID_CAP);
            if ((pResults->measStatus == LT_MSRMNT_STATUS_DEGRADED_ACCURACY) &&
                (*pTestAmp > LT_CAP_LOW_VOLTAGE)) {

                *pTestAmp = LT_CAP_LOW_VOLTAGE;

                pResults->measStatus = LT_MSRMNT_STATUS_PASSED;
                pVpTestResult->result.capAlt.accuracyFlag = FALSE;
                pResults->ctg = LT_IMPEDANCE_NOT_MEASURED;
                pResults->crg = LT_IMPEDANCE_NOT_MEASURED;
                pResults->ctr = LT_IMPEDANCE_NOT_MEASURED;
                pTemp890->testTimer.timerVal = LT_VP890_NEXT_TICK;
                retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY, &pTemp890->testTimer);
                *pState = CAP_MEASURE_TIP;
                break;
            } else if (TRUE == isCapBelowThreshold(pTestCtx, (int32)LT_CAP_THRESHOLD)) {

                *pTestFreq = LT_CAP_1333_HZ;

                pResults->ctg = LT_IMPEDANCE_NOT_MEASURED;
                pResults->crg = LT_IMPEDANCE_NOT_MEASURED;
                pResults->ctr = LT_IMPEDANCE_NOT_MEASURED;
                pTemp890->testTimer.timerVal = LT_VP890_NEXT_TICK;
                retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY, &pTemp890->testTimer);
                *pState = CAP_MEASURE_TIP;
                break;
            } else {
                /* update the result structure with all the  valid measurements */
                if (pTempCap->ctg != LT_IMPEDANCE_NOT_MEASURED) {
                    LT_DOUT(LT_DBG_INFO, ("Update ctg = %li", pTempCap->ctg));
                    pResults->ctg = pTempCap->ctg;
                }
                if (pTempCap->crg != LT_IMPEDANCE_NOT_MEASURED) {
                    LT_DOUT(LT_DBG_INFO, ("Update crg = %li", pTempCap->crg));
                    pResults->crg = pTempCap->crg;
                }
                if (pTempCap->ctr != LT_IMPEDANCE_NOT_MEASURED) {
                    LT_DOUT(LT_DBG_INFO, ("Update ctr = %li", pTempCap->ctr));
                    pResults->ctr = pTempCap->ctr;
                }
            }
            /* no break */

        case CAP_CONCLUDE:
            if (FALSE == pTemp890->internalTest) {
                VpTestConcludeType conclude = { FALSE };
                /* conclude the test if not an internal test*/
                retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE, &conclude);
                *pState = CAP_COMPLETE;
                break;
            }
            /* no break */

        case CAP_COMPLETE:
            retval = LT_STATUS_DONE;
            break;

        case CAP_ABORT:
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

static bool
isCapBelowThreshold(
    LtTestCtxType *pTestCtx,
    int32 threshold)
{
    LtCapResultType *pResults = &pTestCtx->pResult->result.cap;
    LtVp890TestTempType *pTemp890 = &pTestCtx->pTemp->vp890Temp;
    LtVp890CapTempType *pTempCap = &pTemp890->tempData.cap;
    bool retval = FALSE;

    /* we do not need to re-run if the test frequency is already high [0,1] */
    if (pTempCap->input.testFreq <= 1) {
        return FALSE;
    }

    /* save the acceptable values before the potential re-run */
    if (pResults->ctg < threshold) {
        retval = TRUE;
    } else {
        pTempCap->ctg = pResults->ctg;
    }
    if (pResults->crg < threshold) {
        retval = TRUE;
    } else {
        pTempCap->crg = pResults->crg;
    }
    if (pResults->ctr < threshold) {
        retval = TRUE;
    } else {
        pTempCap->ctr = pResults->ctr;
    }

    return retval;
}

#endif /* LT_VP890_VVP_PACKAGE */
#endif /* LT_VP890_PACKAGE && VP890_INCLUDE_TESTLINE_CODE */
