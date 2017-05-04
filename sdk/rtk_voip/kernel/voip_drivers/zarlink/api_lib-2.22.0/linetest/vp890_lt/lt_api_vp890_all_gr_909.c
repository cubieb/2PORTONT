/** file lt_api_vp890_all_pf.c
 *
 *  This file contains the PASS/FAIL Line Voltage test.
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 5264 $
 * $LastChangedDate: 2009-07-06 10:22:54 -0500 (Mon, 06 Jul 2009) $
 */

#include "lt_api.h"

#if defined(VP890_INCLUDE_TESTLINE_CODE) && defined(LT_VP890_PACKAGE)
#if defined(LT_ALL_GR_909) && defined(LT_RES_FLT) && defined(LT_RINGERS) && defined(LT_ROH) && defined(LT_LINE_V)

#include "lt_api_internal.h"
#include "lt_api_vp890.h"

static LtTestStatusType
LtVp890JumpToConclude(
    LtTestCtxType *pTestCtx);

static void
LtVp890AllGr909ClearTemp(
    LtAllGr909ResultType *pTempAllGr909);

static LtTestStatusType
LtVp890AllCheckResult(
    LtTestCtxType *pTestCtx,
    LtTestStatusType rtnVal);

extern LtTestStatusType
LtVp890AllGr909Handler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent)
{
    VpTestResultType *pVpTestResult = (VpTestResultType*)pEvent->pResult;
    LtTestStatusType retval = LT_STATUS_RUNNING;

    LtVp890TestTempType *pTemp = &pTestCtx->pTemp->vp890Temp;
    LtVp890AllGr909TestTempType *pTempAllGr909 = &pTemp->allGr909;
    LtAllGr909ResultType *pTempResults = &pTempAllGr909->results;

    LtVp890AllGr909States *pInnerState = &pTempAllGr909->innerState;
    LtAllGr909MaskType *pTempAllpfMask = &pTempResults->fltMask;
    LtAllGr909SubTestMaskType *pTempSubTestMask = &pTempResults->subTestMask;

    bool stopOnFail = TRUE;

    LT_DOUT(LT_DBG_INFO, ("LtVp890AllGr909Handler(testState:%i)", *pInnerState));

    switch (*pInnerState) {
        case VP890_ALL_GR_909_INIT:
            /* Clear out the temp structure */
            LtVp890AllGr909ClearTemp(pTempResults);
            /* no break needed */

        case VP890_ALL_GR_909_LINE_V:
            *pInnerState = VP890_ALL_GR_909_LINE_V;
            /*
             * Load up lineV input and criteria into lineV temp struct.
             * This step is actually redundant since tempData.lineV.input
             * and tempData.lineV.criteria already contain the correct info
             * due to the fact that lineV was the last attributes checked in
             * the Vp890909AllAttributesCheck() function. This is done for
             * code clarity and incase the order of the attribute checkers
             * is changed inside of Vp890909AllAttributesCheck().
             */
            LtMemCpy(&pTemp->tempData.lineV.input,
                &pTempAllGr909->input.lineVInp,
                sizeof(LtLineVInputType));
            LtMemCpy(&pTemp->tempData.lineV.criteria,
                &pTempAllGr909->criteria.lintVCrt,
                sizeof(LtLineVCriteriaType));
            /* First, perform the LineV test */
            retval = pTempAllGr909->LineVHandlerFnPtr(pTestCtx, pEvent);

            if(retval != LT_STATUS_DONE) {
                return LtVp890AllCheckResult(pTestCtx, retval);
            } else {
                *pTempSubTestMask |=
                    (LT_A909STM_LINE_V_HEMF | LT_A909STM_LINE_V_FEMF);

                /* copy all results from lineV to 909 temp struct */
                LtMemCpy(&pTempResults->lineV, &pTestCtx->pResult->result.lineV,
                    sizeof(LtLineVResultType));

                if (LT_TEST_PASSED != (pTempResults->lineV.fltMask & (
                  LT_LVM_HEMF_DC_TIP  | LT_LVM_HEMF_DC_RING |
                  LT_LVM_HEMF_DC_DIFF | LT_LVM_HEMF_AC_TIP  |
                  LT_LVM_HEMF_AC_RING | LT_LVM_HEMF_AC_DIFF))) {
                    *pTempAllpfMask |= LT_A909TM_LINE_V_HEMF_FAILED;
                    LT_DOUT(LT_DBG_INFO, ("LtVp890AllGr909Handler() : "\
                        "LT_A909TM_LINE_V_HEMF_FAILED"));

                    if (stopOnFail) {
                        return LtVp890JumpToConclude(pTestCtx);
                    }

                } else if (LT_TEST_PASSED != (pTempResults->lineV.fltMask & (
                  LT_LVM_FEMF_DC_TIP  | LT_LVM_FEMF_DC_RING |
                  LT_LVM_FEMF_DC_DIFF | LT_LVM_FEMF_AC_TIP  |
                  LT_LVM_FEMF_AC_RING | LT_LVM_FEMF_AC_DIFF))) {
                    *pTempAllpfMask |= LT_A909TM_LINE_V_FEMF_FAILED;
                    LT_DOUT(LT_DBG_INFO, ("LtVp890AllGr909Handler() : "\
                        "LT_A909TM_LINE_V_FEMF_FAILED"));
                    if (stopOnFail) {
                        return LtVp890JumpToConclude(pTestCtx);
                    }
                }

                /* Get the current relay state */
                VpGetRelayState(pTestCtx->pLineCtx, &pTempAllGr909->relayState);

                /*  Ensure relays are closed so that we can continue */
                VpSetRelayState(pTestCtx->pLineCtx, VP_RELAY_NORMAL);

                /* prepare for next pass/fail test (off-hook) */
                pTestCtx->state = LT_GLOBAL_FIRST_STATE;
                *pInnerState = VP890_ALL_GR_909_RES_FLT;
            }
            /* no break needed */
        case VP890_ALL_GR_909_RES_FLT:
            /* load up resFlts input and criteria into resFlts temp struct */
            LtMemCpy(&pTemp->tempData.resFlt.input,
                &pTempAllGr909->input.resFltInp,
                sizeof(LtResFltInputType));
            LtMemCpy(&pTemp->tempData.resFlt.criteria,
                &pTempAllGr909->criteria.resFltCrt,
                sizeof(LtResFltCriteriaType));

            /*
             * need to go directly to high gain mode if an AC line voltage
             * greater than or equal too 1.5V was measured or a DC line voltage
             * greater than or equal too 3.0V was measured. This will cut down
             * on overall test time.
             */
            if ((pTempResults->lineV.vAcTip >= 1500) ||
                (pTempResults->lineV.vAcRing >= 1500) ||
                (pTempResults->lineV.vAcDiff >= 1500) ||
                (pTempResults->lineV.vDcTip >= 3000) ||
                (pTempResults->lineV.vDcRing >= 3000) ||
                (pTempResults->lineV.vDcDiff >= 3000)) {
                pTemp->tempData.resFlt.input.startHighGain = TRUE;
            }


            retval = pTempAllGr909->ResFltHandlerFnPtr(pTestCtx, pEvent);

            if(retval != LT_STATUS_DONE) {
                if ((LT_STATUS_ERROR_VP_TEST == retval) &&
                    (VP_TEST_STATUS_RESOURCE_NA == pVpTestResult->errorCode)) {
                    /*
                     * checking to see if the test is trying to indicating
                     * that the isolate relay is open i.e.
                     * VP_TEST_STATUS_RESOURCE_NA. If this is the case
                     * indicate that the RES FAULT test failed for this reason
                     * by not setting the subtestMask bit corresponding to
                     * the OFFHOOK test and then conclude the test early.
                     */

                    /* copy all results from roh to 909 temp struct */
                    LtMemCpy(&pTempResults->resFlt, &pTestCtx->pResult->result.resFlt,
                        sizeof(LtResFltResultType));

                     *pTempAllpfMask |= LT_A909TM_RES_FLT_FAILED;
                     return LtVp890JumpToConclude(pTestCtx);
                }
                return LtVp890AllCheckResult(pTestCtx, retval);;
            } else {
                *pTempSubTestMask |=  LT_A909STM_RES_FLT;

                /* copy all results from res fault to 909 temp struct */
                LtMemCpy(&pTempResults->resFlt, &pTestCtx->pResult->result.resFlt,
                    sizeof(LtResFltResultType));

                /* Do we need to run the ROH test ?*/
                pTestCtx->state = LT_GLOBAL_FIRST_STATE;

                /* what do we do if resflt fails */
                if (LT_TEST_PASSED != pTempResults->resFlt.fltMask) {
                    LT_DOUT(LT_DBG_INFO, ("LtVp890AllGr909Handler() : "\
                        "LT_A909TM_RES_FLT_FAILED"));
                    *pTempAllpfMask |= LT_A909TM_RES_FLT_FAILED;

                    if ((pTempResults->resFlt.measStatus == LT_MSRMNT_STATUS_EXCESSIVE_ILG) 
                        && stopOnFail) {
                        return LtVp890JumpToConclude(pTestCtx);
                    }
                }

                /* skip the roh and ren test to save time if possible */
                if (LtVp890AllGr909RunRoh(pTestCtx) == FALSE) {
                    *pInnerState = VP890_ALL_GR_909_COMPLETE;
                    return LtVp890JumpToConclude(pTestCtx);
                } else {
                    *pInnerState = VP890_ALL_GR_909_OFF_HOOK;
                }
            }
            /* no break needed */



        case VP890_ALL_GR_909_OFF_HOOK:
            /* load up roh input and criteria into roh temp struct */
            LtMemCpy(&pTemp->tempData.roh.input,
                &pTempAllGr909->input.rohInp,
                sizeof(LtRohInputType));
            LtMemCpy(&pTemp->tempData.roh.criteria,
                &pTempAllGr909->criteria.rohCrt,
                sizeof(LtRohCriteriaType));

            /* Second, perform the off hook test */
            retval = pTempAllGr909->RohHandlerFnPtr(pTestCtx, pEvent);

            if(retval != LT_STATUS_DONE) {
                return LtVp890AllCheckResult(pTestCtx, retval);
            } else {
                *pTempSubTestMask |=  LT_A909STM_ROH;

                /* copy all results from roh to 909 temp struct */
                LtMemCpy(&pTempResults->roh, &pTestCtx->pResult->result.roh,
                    sizeof(LtRohResultType));

                /* Do we need to run the REN test ?*/
                pTestCtx->state = LT_GLOBAL_FIRST_STATE;

                /* Prepare for next pass/fail test (res fault) */
                if (LT_TEST_PASSED != pTempResults->roh.fltMask) {
                    LT_DOUT(LT_DBG_INFO, ("LtVp890AllGr909Handler() : "\
                        "LT_A909TM_ROH_FAILED"));
                    *pTempAllpfMask |= LT_A909TM_ROH_FAILED;
                    if (stopOnFail) {
                        return LtVp890JumpToConclude(pTestCtx);
                    }
                }

                /* skip the ren test if possible */
                if (LtVp890AllGr909RunRen(pTestCtx) == FALSE) {
                    *pInnerState = VP890_ALL_GR_909_COMPLETE;
                    return LtVp890JumpToConclude(pTestCtx);
                } else {
                    *pInnerState = VP890_ALL_GR_909_RINGERS;
                }
            }
            /* no break needed */



        case VP890_ALL_GR_909_RINGERS:
            /* load up ringers input and criteria into ringers temp struct */
            LtMemCpy(&pTemp->tempData.ringers.input,
                &pTempAllGr909->input.ringersInp,
                sizeof(LtRingerInputType));
            LtMemCpy(&pTemp->tempData.ringers.criteria,
                &pTempAllGr909->criteria.ringersCrt,
                sizeof(LtRingersCriteriaType));

            /* Third, perform the off hook test */
            retval = pTempAllGr909->RingerHandlerFnPtr(pTestCtx, pEvent);

            if(retval != LT_STATUS_DONE) {
                return LtVp890AllCheckResult(pTestCtx, retval);
            } else {
                *pTempSubTestMask |=  LT_A909STM_RINGERS;

                *pInnerState = VP890_ALL_GR_909_COMPLETE;

                /* copy all results from ringers to 909 temp struct */
                LtMemCpy(&pTempResults->ringers, &pTestCtx->pResult->result.ringers,
                    sizeof(LtRingersResultType));

                if (LT_RNGM_TEST_PASSED != pTempResults->ringers.fltMask) {
                    LT_DOUT(LT_DBG_INFO, ("LtVp890AllGr909Handler() : "\
                        "LT_A909TM_RINGERS_FAILED"));
                    *pTempAllpfMask |= LT_A909TM_RINGERS_FAILED;
                }
                return LtVp890JumpToConclude(pTestCtx);
            }

            /* no break needed */

        case VP890_ALL_GR_909_COMPLETE:
            *pInnerState = VP890_ALL_GR_909_COMPLETE;
            /* if the linev test failed go to disconnect */
            if (LT_TEST_PASSED != pTempResults->lineV.fltMask) {
                VpSetLineState(pTestCtx->pLineCtx, VP_LINE_DISCONNECT);
            }

            /* Ensure relays are open if linev requested it */
            VpSetRelayState(pTestCtx->pLineCtx, pTempAllGr909->relayState);

            /* copy all temp results to the user */
            LtVp890AllCopyResults(pTestCtx);

            /* Test is DONE let application know */
            retval = LT_STATUS_DONE;
            break;

        default:
            /* This should never happen */
            retval = LT_STATUS_ERROR_UNKNOWN;
            break;
    }
    return LtVp890AllCheckResult(pTestCtx, retval);;
}

static LtTestStatusType
LtVp890JumpToConclude(
    LtTestCtxType *pTestCtx)
{
    VpTestConcludeType conclude = { FALSE };

    pTestCtx->pTemp->vp890Temp.allGr909.innerState = VP890_ALL_GR_909_COMPLETE;
    return Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE, &conclude);
}

static LtTestStatusType
LtVp890AllCheckResult(
    LtTestCtxType *pTestCtx,
    LtTestStatusType rtnVal)
{

    if ((rtnVal != LT_STATUS_RUNNING) && (rtnVal != LT_STATUS_DONE)) {
        LtVp890AllCopyResults(pTestCtx);
    }
    return rtnVal;
}

extern void
LtVp890AllCopyResults(
    LtTestCtxType *pTestCtx)
{
    LtAllGr909ResultType *pTempResults = &pTestCtx->pTemp->vp890Temp.allGr909.results;
    LtAllGr909MaskType *pTempAllpfMask = &pTempResults->fltMask;
    LtAllGr909SubTestMaskType *pTempSubTestMask = &pTempResults->subTestMask;

    /* copy useful 909 temp data into 909 results struct */
    pTestCtx->pResult->result.allGr909.fltMask =  *pTempAllpfMask;
    pTestCtx->pResult->result.allGr909.subTestMask = *pTempSubTestMask;
    pTestCtx->pResult->result.allGr909.measStatus = pTempResults->measStatus;

    LtMemCpy(&pTestCtx->pResult->result.allGr909.lineV,
        &pTempResults->lineV, sizeof(LtLineVResultType));

    LtMemCpy(&pTestCtx->pResult->result.allGr909.roh,
        &pTempResults->roh, sizeof(LtRohResultType));

    LtMemCpy(&pTestCtx->pResult->result.allGr909.ringers,
        &pTempResults->ringers, sizeof(LtRingersResultType));

    LtMemCpy(&pTestCtx->pResult->result.allGr909.resFlt,
        &pTempResults->resFlt, sizeof(LtResFltResultType));

    return;
}

static void
LtVp890AllGr909ClearTemp(
    LtAllGr909ResultType *pTempResults)
{
    /* Zero out all test results data */
    pTempResults->fltMask = LT_TEST_PASSED;
    pTempResults->subTestMask = LT_A909STM_NO_PF_TESTED;
    pTempResults->measStatus =  LT_A909STM_NO_PF_TESTED;

    pTempResults->lineV.fltMask = LT_TEST_MEASUREMENT_ERROR;
    pTempResults->lineV.measStatus = LT_MSRMNT_STATUS_PASSED;
    pTempResults->lineV.vDcTip = LT_VOLTAGE_NOT_MEASURED;
    pTempResults->lineV.vDcRing = LT_VOLTAGE_NOT_MEASURED;
    pTempResults->lineV.vDcDiff = LT_VOLTAGE_NOT_MEASURED;
    pTempResults->lineV.vAcTip = LT_VOLTAGE_NOT_MEASURED;
    pTempResults->lineV.vAcRing = LT_VOLTAGE_NOT_MEASURED;
    pTempResults->lineV.vAcDiff = LT_VOLTAGE_NOT_MEASURED;

    pTempResults->roh.fltMask = LT_TEST_MEASUREMENT_ERROR;
    pTempResults->roh.measStatus = LT_MSRMNT_STATUS_PASSED;
    pTempResults->roh.rLoop1 = LT_IMPEDANCE_NOT_MEASURED;
    pTempResults->roh.rLoop2 = LT_IMPEDANCE_NOT_MEASURED;

    pTempResults->ringers.fltMask = LT_TEST_MEASUREMENT_ERROR;
    pTempResults->ringers.measStatus = LT_MSRMNT_STATUS_PASSED;
    pTempResults->ringers.ren = LT_REN_NOT_MEASURED;
    pTempResults->ringers.rentg = LT_REN_NOT_MEASURED;
    pTempResults->ringers.renrg = LT_REN_NOT_MEASURED;
    pTempResults->ringers.ringerTestType = LT_RINGER_TEST_NUM_TYPES;

    pTempResults->resFlt.fltMask = LT_TEST_MEASUREMENT_ERROR;
    pTempResults->resFlt.measStatus = LT_MSRMNT_STATUS_PASSED;
    pTempResults->resFlt.rtg = LT_IMPEDANCE_NOT_MEASURED;
    pTempResults->resFlt.rrg = LT_IMPEDANCE_NOT_MEASURED;
    pTempResults->resFlt.rtr = LT_IMPEDANCE_NOT_MEASURED;
    return;
}
#endif /* LT_ALL_GR_909 ... */

#endif /* LT_VP890_PACKAGE && VP890_INCLUDE_TESTLINE_CODE */
