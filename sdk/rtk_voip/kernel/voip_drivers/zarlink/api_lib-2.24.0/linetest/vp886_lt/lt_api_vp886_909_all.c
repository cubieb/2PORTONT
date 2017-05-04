/** file lt_api_vp886_909_all.c
 *
 *  This file contains the VP886 909 all test routine.
 *
 * Copyright (c) 2012, Microsemi Corporation
 *
 * $Revision: 10559 $
 * $LastChangedDate: 2012-10-29 15:30:05 -0500 (Mon, 29 Oct 2012) $
 */
#include "lt_api.h"

#ifdef LT_ALL_GR_909_886

#include "lt_api_internal.h"


static bool
AllGR909HandlerSM(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent,
    LtTestStatusType *pRetval);

static bool
RohRequired(
    LtTestCtxType *pTestCtx);

static bool
RenRequired(
    LtTestCtxType *pTestCtx);

static void
ClearTempResults(
    LtAllGr909ResultType *pTempResults);

static LtTestStatusType
CheckSubtestResult(
    LtTestCtxType *pTestCtx,
    LtTestStatusType rtnVal);

static void
CopyResults(
    LtTestCtxType *pTestCtx);



extern bool 
LtVp886AllGR909AttributeCheck (
    LtTestCtxType *pTestCtx)
{
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;
    LtVp886AllGr909TestTempType *pTempAllGr909 = &pTemp886->allGr909;

    LtAllGr909InputType gr909Input;
    LtAllGr909CriteriaType gr909Criteria;

    void *pOrigInputs = pTemp886->attributes.pInputs;
    void *pOrigCriteria = pTemp886->attributes.pCriteria;

    if (pOrigInputs != VP_NULL) {
        LtMemCpy(&gr909Input, pOrigInputs, sizeof(LtAllGr909InputType));
    }
    if (pOrigCriteria != VP_NULL) {
        LtMemCpy(&gr909Criteria, pOrigCriteria, sizeof(LtAllGr909CriteriaType));
    }


    /* 
     * ResFlts argument checking 
     */
    if (pOrigInputs != VP_NULL) {
        pTemp886->attributes.pInputs = &gr909Input.resFltInp;
    }
    if (pOrigCriteria != VP_NULL) {
        pTemp886->attributes.pCriteria = &gr909Criteria.resFltCrt;
    }
    if (LtVp886ResFltAttributeCheck(pTestCtx) == FALSE) {
        return FALSE;
    }
    LtMemCpy(&pTempAllGr909->input.resFltInp, &pTemp886->tempData.resFlt.input,
        sizeof(LtResFltInputType));

    LtMemCpy(&pTempAllGr909->criteria.resFltCrt, &pTemp886->tempData.resFlt.criteria,
        sizeof(LtResFltCriteriaType));

    pTempAllGr909->ResFltHandlerFnPtr = pTestCtx->Lt886TestEventHandlerFunc;


    /* 
     * Ren argument checking 
     */
    if (pOrigInputs != VP_NULL) {
        pTemp886->attributes.pInputs = &gr909Input.ringersInp;
    }
    if (pOrigCriteria != VP_NULL) {
        pTemp886->attributes.pCriteria = &gr909Criteria.ringersCrt;
    }
    if (LtVp886RingersAttributeCheck(pTestCtx) == FALSE) {
        return FALSE;
    }
    LtMemCpy(&pTempAllGr909->input.ringersInp, &pTemp886->tempData.ringers.input,
        sizeof(LtRingerInputType));

    LtMemCpy(&pTempAllGr909->criteria.ringersCrt, &pTemp886->tempData.ringers.criteria,
        sizeof(LtRingersCriteriaType));

    pTempAllGr909->RingerHandlerFnPtr = pTestCtx->Lt886TestEventHandlerFunc;


    /* 
     * ROH argument checking 
     */
    if (pOrigInputs != VP_NULL) {
        pTemp886->attributes.pInputs = &gr909Input.rohInp;
    }
    if (pOrigCriteria != VP_NULL) {
        pTemp886->attributes.pCriteria = &gr909Criteria.rohCrt;
    }
    if (LtVp886RohAttributeCheck(pTestCtx) == FALSE) {
        return FALSE;
    }
    LtMemCpy(&pTempAllGr909->input.rohInp, &pTemp886->tempData.roh.input,
        sizeof(LtRohInputType));

    LtMemCpy(&pTempAllGr909->criteria.rohCrt, &pTemp886->tempData.roh.criteria,
        sizeof(LtRohCriteriaType));

    pTempAllGr909->RohHandlerFnPtr = pTestCtx->Lt886TestEventHandlerFunc;

    /* 
     * Line V argument checking 
     */
    if (pOrigInputs != VP_NULL) {
        pTemp886->attributes.pInputs = &gr909Input.lineVInp;
    }
    if (pOrigCriteria != VP_NULL) {
        pTemp886->attributes.pCriteria = &gr909Criteria.lintVCrt;
    }
    if (LtVp886LineVAttributeCheck(pTestCtx) == FALSE) {
        return FALSE;
    }
    LtMemCpy(&pTempAllGr909->input.lineVInp, &pTemp886->tempData.lineV.input,
        sizeof(LtLineVInputType));

    LtMemCpy(&pTempAllGr909->criteria.lintVCrt, &pTemp886->tempData.lineV.criteria,
        sizeof(LtLineVCriteriaType));

    pTempAllGr909->LineVHandlerFnPtr = pTestCtx->Lt886TestEventHandlerFunc;


    /* initialize internal flags */
    pTempAllGr909->innerState = VP886_ALL_GR_909_INIT;

    /* setup high level test handler */
    pTestCtx->Lt886TestEventHandlerFunc = AllGR909HandlerSM;
    return TRUE;
}

static bool
AllGR909HandlerSM(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent,
    LtTestStatusType *pRetval)
{
    VpTestResultType *pVpTestResult = (VpTestResultType*)pEvent->pResult;
    LtTestStatusType retval = LT_STATUS_RUNNING;

    LtVp886TestTempType *pTemp = &pTestCtx->pTemp->vp886Temp;
    LtVp886AllGr909TestTempType *pTempAllGr909 = &pTemp->allGr909;
    LtAllGr909ResultType *pTempResults = &pTempAllGr909->results;

    LtVp886AllGr909States *pInnerState = &pTempAllGr909->innerState;
    LtAllGr909MaskType *pTempAllpfMask = &pTempResults->fltMask;
    LtAllGr909SubTestMaskType *pTempSubTestMask = &pTempResults->subTestMask;

    bool requestNextState = FALSE;
    *pRetval = LT_STATUS_RUNNING;

    LT_DOUT(LT_DBG_INFO, ("AllGR909HandlerSM(testState:%i)", *pInnerState));

    switch (*pInnerState) {
        case VP886_ALL_GR_909_INIT:
            LtInitOutputStruct(pTestCtx->pResult, LT_TID_ALL_GR_909);
            ClearTempResults(pTempResults);
            *pInnerState = VP886_ALL_GR_909_LINE_V_INIT;
            requestNextState = TRUE;
            break;


        case VP886_ALL_GR_909_LINE_V_INIT: {
            /*
             * Load up lineV input and criteria into lineV temp struct.
             * This step is actually redundant since tempData.lineV.input
             * and tempData.lineV.criteria already contain the correct info
             * due to the fact that lineV was the last attributes checked in
             * the LtVp886AllGR909AttributeCheck() function. This is done for
             * code clarity and incase the order of the attribute checkers
             * is changed inside of LtVp886AllGR909AttributeCheck().
             */
            LtMemCpy(&pTemp->tempData.lineV.input,
                &pTempAllGr909->input.lineVInp,
                sizeof(LtLineVInputType));
            LtMemCpy(&pTemp->tempData.lineV.criteria,
                &pTempAllGr909->criteria.lintVCrt,
                sizeof(LtLineVCriteriaType));

            pTestCtx->state = LT_GLOBAL_FIRST_STATE;

            *pInnerState = VP886_ALL_GR_909_LINE_V;
            requestNextState = TRUE;
            break;
        }

        case VP886_ALL_GR_909_LINE_V: {
            /* First, perform the LineV test */
            requestNextState = pTempAllGr909->LineVHandlerFnPtr(pTestCtx, pEvent, &retval);

            if(retval != LT_STATUS_DONE) {
                *pRetval = CheckSubtestResult(pTestCtx, retval);
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
                    LT_DOUT(LT_DBG_INFO, ("AllGR909HandlerSM() : "\
                        "LT_A909TM_LINE_V_HEMF_FAILED"));

                } else if (LT_TEST_PASSED != (pTempResults->lineV.fltMask & (
                  LT_LVM_FEMF_DC_TIP  | LT_LVM_FEMF_DC_RING |
                  LT_LVM_FEMF_DC_DIFF | LT_LVM_FEMF_AC_TIP  |
                  LT_LVM_FEMF_AC_RING | LT_LVM_FEMF_AC_DIFF))) {
                    *pTempAllpfMask |= LT_A909TM_LINE_V_FEMF_FAILED;
                    LT_DOUT(LT_DBG_INFO, ("AllGR909HandlerSM() : "\
                        "LT_A909TM_LINE_V_FEMF_FAILED"));
                }

                /* Get the current relay state */
                VpGetRelayState(pTestCtx->pLineCtx, &pTempAllGr909->relayState);

                /*  Ensure relays are closed so that we can continue */
                VpSetRelayState(pTestCtx->pLineCtx, VP_RELAY_NORMAL);

                /* prepare for next pass/fail test (resistive faults) */
                *pInnerState = VP886_ALL_GR_909_RES_FLT_INIT;
                requestNextState = TRUE;
            }
            break;
        }


        case VP886_ALL_GR_909_RES_FLT_INIT: {
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
                (pTempResults->lineV.vDcTip <= -1400) || (pTempResults->lineV.vDcTip >= 2000) ||
                (pTempResults->lineV.vDcRing <= -1400) || (pTempResults->lineV.vDcRing >= 2000) ||
                (ABS(pTempResults->lineV.vDcDiff) >= 3400)) {
                pTemp->tempData.resFlt.input.startHighGain = TRUE;
            }

            pTestCtx->state = LT_GLOBAL_FIRST_STATE;

            *pInnerState = VP886_ALL_GR_909_RES_FLT;
            requestNextState = TRUE;
            break;
        }
        
        case VP886_ALL_GR_909_RES_FLT: {
            /* Second, perform the resistive faults test */
            requestNextState = pTempAllGr909->ResFltHandlerFnPtr(pTestCtx, pEvent, &retval);

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
                    *pInnerState = VP886_ALL_GR_909_CONCLUDE;
                    requestNextState = TRUE;
                    break;
                }
                *pRetval = CheckSubtestResult(pTestCtx, retval);
            } else {
                *pTempSubTestMask |=  LT_A909STM_RES_FLT;

                /* copy all results from res fault to 909 temp struct */
                LtMemCpy(&pTempResults->resFlt, &pTestCtx->pResult->result.resFlt,
                    sizeof(LtResFltResultType));

                /* what do we do if resflt fails */
                if (LT_TEST_PASSED != pTempResults->resFlt.fltMask) {
                    LT_DOUT(LT_DBG_INFO, ("AllGR909HandlerSM() : "\
                        "LT_A909TM_RES_FLT_FAILED"));
                    *pTempAllpfMask |= LT_A909TM_RES_FLT_FAILED;
                }

                /* skip the roh and ren test to save time if possible */
                if (RohRequired(pTestCtx) == FALSE) {
                    *pInnerState = VP886_ALL_GR_909_CONCLUDE;
                    requestNextState = TRUE;
                } else {
                    *pInnerState = VP886_ALL_GR_909_OFF_HOOK_INIT;
                    requestNextState = TRUE;
                }
            }
            break;
        }


        case VP886_ALL_GR_909_OFF_HOOK_INIT: {
            /* load up roh input and criteria into roh temp struct */
            LtMemCpy(&pTemp->tempData.roh.input,
                &pTempAllGr909->input.rohInp,
                sizeof(LtRohInputType));
            LtMemCpy(&pTemp->tempData.roh.criteria,
                &pTempAllGr909->criteria.rohCrt,
                sizeof(LtRohCriteriaType));

            pTemp->tempData.roh.quickStop = FALSE;

            pTestCtx->state = LT_GLOBAL_FIRST_STATE;

            *pInnerState = VP886_ALL_GR_909_OFF_HOOK;
            requestNextState = TRUE;
            break;
        }
        
        case VP886_ALL_GR_909_OFF_HOOK: {
            /* Third, perform the off hook test */
            requestNextState = pTempAllGr909->RohHandlerFnPtr(pTestCtx, pEvent, &retval);

            if(retval != LT_STATUS_DONE) {
                *pRetval = CheckSubtestResult(pTestCtx, retval);
            } else {
                *pTempSubTestMask |=  LT_A909STM_ROH;

                /* copy all results from roh to 909 temp struct */
                LtMemCpy(&pTempResults->roh, &pTestCtx->pResult->result.roh,
                    sizeof(LtRohResultType));

                /* Prepare for next pass/fail test (ringers) */
                if (LT_TEST_PASSED != pTempResults->roh.fltMask) {
                    LT_DOUT(LT_DBG_INFO, ("AllGR909HandlerSM() : "\
                        "LT_A909TM_ROH_FAILED"));
                    *pTempAllpfMask |= LT_A909TM_ROH_FAILED;
                }
                /* skip the ren test to save time if possible */
                if (RenRequired(pTestCtx) == FALSE) {
                    *pInnerState = VP886_ALL_GR_909_CONCLUDE;
                    requestNextState = TRUE;
                } else {
                    *pInnerState = VP886_ALL_GR_909_RINGERS_INIT;
                    requestNextState = TRUE;
                }
            }
            break;
        }


        case VP886_ALL_GR_909_RINGERS_INIT: {
            /* load up ringers input and criteria into ringers temp struct */
            LtMemCpy(&pTemp->tempData.ringers.input,
                &pTempAllGr909->input.ringersInp,
                sizeof(LtRingerInputType));
            LtMemCpy(&pTemp->tempData.ringers.criteria,
                &pTempAllGr909->criteria.ringersCrt,
                sizeof(LtRingersCriteriaType));

            pTemp->tempData.ringers.quickStop = FALSE;

            pTestCtx->state = LT_GLOBAL_FIRST_STATE;

            *pInnerState = VP886_ALL_GR_909_RINGERS;
            requestNextState = TRUE;
            break;
        }

        case VP886_ALL_GR_909_RINGERS: {
            /* Fourth, perform the REN test */
            requestNextState = pTempAllGr909->RingerHandlerFnPtr(pTestCtx, pEvent, &retval);

            if(retval != LT_STATUS_DONE) {
                *pRetval = CheckSubtestResult(pTestCtx, retval);
            } else {
                *pTempSubTestMask |= LT_A909STM_RINGERS;

                /* copy all results from ringers to 909 temp struct */
                LtMemCpy(&pTempResults->ringers, &pTestCtx->pResult->result.ringers,
                    sizeof(LtRingersResultType));

                if (LT_RNGM_TEST_PASSED != pTempResults->ringers.fltMask) {
                    LT_DOUT(LT_DBG_INFO, ("AllGR909HandlerSM() : "\
                        "LT_A909TM_RINGERS_FAILED"));
                    *pTempAllpfMask |= LT_A909TM_RINGERS_FAILED;
                }

                *pInnerState = VP886_ALL_GR_909_CONCLUDE;
                requestNextState = TRUE;
            }
            break;
        }


        /* Start the conclude primitive if this test not a part of an internal test */
        case VP886_ALL_GR_909_CONCLUDE: {
            VpTestConcludeType conclude = { FALSE };
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE, &conclude);
            *pInnerState = VP886_ALL_GR_909_COMPLETE;
            break;
        }

        case VP886_ALL_GR_909_COMPLETE: {
            /* if the linev test failed go to disconnect */
            if (LT_TEST_PASSED != pTempResults->lineV.fltMask) {
                VpSetLineState(pTestCtx->pLineCtx, VP_LINE_DISCONNECT);
            }

            /* Ensure relays are open if linev requested it */
            VpSetRelayState(pTestCtx->pLineCtx, pTempAllGr909->relayState);

            /* copy all temp results to the user */
            CopyResults(pTestCtx);
            *pRetval = LT_STATUS_DONE;
            break;
        }

        case VP886_ALL_GR_909_ABORT:
            /* Test aborted, results may indicate source of error. */
            *pRetval = LT_STATUS_ABORTED;
            break;

        default:
            /* This should never happen */
            LT_DOUT(LT_DBG_ERRR, ("AllGR909HandlerSM() : "
                "This should never happen -> LT_STATUS_ERROR_UNKNOWN"));
            *pRetval = LT_STATUS_ERROR_UNKNOWN;
            break;
    }
    return requestNextState;
}

static bool
RohRequired(
    LtTestCtxType *pTestCtx)
{
    LtVp886TestTempType *pTemp = &pTestCtx->pTemp->vp886Temp;
    LtVp886AllGr909TestTempType *pTempAllGr909 = &pTemp->allGr909;
    LtAllGr909ResultType *pTempResults = &pTempAllGr909->results;

    /* if the resflt does not have an ilg issue then run the roh test */
    if (pTempResults->resFlt.measStatus != LT_MSRMNT_STATUS_EXCESSIVE_ILG) {
        return TRUE;
    }

    /* indicate we ran the roh test and it failed */
    pTempResults->roh.fltMask = LT_TEST_MEASUREMENT_ERROR;
    pTempResults->roh.measStatus = LT_MSRMNT_STATUS_EXCESSIVE_ILG;
    pTempResults->fltMask |= LT_A909TM_ROH_FAILED;
    pTempResults->subTestMask |= LT_A909STM_ROH;

    /* Use RenRequired() to fill in REN failure results */
    RenRequired(pTestCtx);

    return FALSE;
}

/*
 * This function is determining if the REN portion of the 909 All test needs to
 * run in order to reduce test execution time for common test conditions.
 *
 * If possible the function will calculate the REN value from the tip to ring
 * resistance as measured by the resistive faults test.
 *
 * The calculation will only be made if the rtr value is valid and the ROH
 * test fails.
 */
static bool
RenRequired(
    LtTestCtxType *pTestCtx)
{
    LtVp886TestTempType *pTemp = &pTestCtx->pTemp->vp886Temp;
    LtVp886RingersTempType *pTempRingers = &pTemp->tempData.ringers;
    LtVp886AllGr909TestTempType *pTempAllGr909 = &pTemp->allGr909;
    LtAllGr909ResultType *pTempResults = &pTempAllGr909->results;

    const int16 maxRes = pTempRingers->criteria.renHigh;
    const int16 minRes = pTempRingers->criteria.renLow;
    int32 rtr;


    /* if the roh had an ilg issue then dont run  */
    if (pTempResults->roh.measStatus == LT_MSRMNT_STATUS_EXCESSIVE_ILG) {
        pTempResults->ringers.fltMask = LT_TEST_MEASUREMENT_ERROR;
        pTempResults->ringers.measStatus = LT_MSRMNT_STATUS_EXCESSIVE_ILG;
        pTempResults->fltMask |= LT_A909TM_RINGERS_FAILED;
        pTempResults->subTestMask |= LT_A909STM_RINGERS;
        return FALSE;
    }

    /* if roh detects an off-hook phone then dont run REN */
    if (pTempResults->roh.fltMask == LT_ROHM_OFF_HOOK) {
        return FALSE;
    }

    /*
     * Note: LT_IMPEDANCE_SHORT_CKT is assumed to be 0 Ohms and
     * resulting calculated ren will be 7000 REN, approx same value
     * that the actual ren test measures.
     */
    if ((pTempResults->resFlt.rtr == LT_IMPEDANCE_NOT_MEASURED) ||
        (pTempResults->resFlt.rtr == LT_IMPEDANCE_OPEN_CKT) ) {
        /* cannot calculate REN from rtr value */
        return TRUE;
    }

    /*
     * if the ROH test completes then run ren and
     * tip/ring is greater than 1400 ohms
     */
    if ((pTempResults->roh.measStatus != LT_MSRMNT_STATUS_EXCESSIVE_ILG) &&
        (pTempResults->resFlt.rtr > 14000)) {
        return TRUE;
    }

    /* we can calculate REN from rtr */
    pTempResults->ringers.ringerTestType = LT_RINGER_REGULAR_PHNE_TEST;

    /*
     * REN = 7000 Ohms / measured resistance
     * ensure we dont divide by 0
     */
    rtr = (pTempResults->resFlt.rtr <= 10) ? 10 : pTempResults->resFlt.rtr;
    pTempResults->ringers.ren = ((70000L * 1000) / rtr);

    /* check 909 limits */
    if (maxRes < pTempResults->ringers.ren) {
        pTempResults->ringers.fltMask = LT_RNGM_REN_HIGH;
        pTempResults->fltMask |= LT_A909TM_RINGERS_FAILED;
    } else if (minRes > pTempResults->ringers.ren) {
        pTempResults->ringers.fltMask = LT_RNGM_REN_LOW;
        pTempResults->fltMask |= LT_A909TM_RINGERS_FAILED;
    } else {
        pTempResults->ringers.fltMask = LT_TEST_PASSED;
    }

    /* indicate we ran the ren test */
    pTempResults->subTestMask |= LT_A909STM_RINGERS;
    return FALSE;
}

static void
ClearTempResults(
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

static LtTestStatusType
CheckSubtestResult(
    LtTestCtxType *pTestCtx,
    LtTestStatusType rtnVal)
{

    if ((rtnVal != LT_STATUS_RUNNING) && (rtnVal != LT_STATUS_DONE)) {
        CopyResults(pTestCtx);
    }
    return rtnVal;
}

static void
CopyResults(
    LtTestCtxType *pTestCtx)
{
    LtAllGr909ResultType *pTempResults = &pTestCtx->pTemp->vp886Temp.allGr909.results;
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


#endif /* LT_ALL_GR_909_886 */
