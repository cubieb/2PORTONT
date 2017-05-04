/** file lt_api_vp886_line_v.c
 *
 *  This file contains the VP886 line voltage test routine.
 *
 * Copyright (c) 2012, Microsemi Corporation
 *
 * $Revision: 10449 $
 * $LastChangedDate: 2012-09-04 16:56:09 -0500 (Tue, 04 Sep 2012) $
 */
#include "lt_api.h"

#ifdef LT_LINE_V_886

#include "lt_api_internal.h"

typedef enum {
    LINE_V_INIT            = LT_GLOBAL_FIRST_STATE,
    LINE_V_RUN_TIP_VXC,
    LINE_V_GET_TIP_VXC,
    LINE_V_RUN_RING_VXC,
    LINE_V_GET_RING_VXC,
    LINE_V_RUN_DIFF_VXC,
    LINE_V_GET_DIFF_VXC,
    LINE_V_CONCLUDE,
    LINE_V_COMPLETE,
    LINE_V_ABORT           = LT_GLOBAL_ABORTED_STATE,
    LINE_V_ENUM_SIZE       = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp886LineVStates;

typedef struct {
    int32 measured;
    int32 estimated;
} LtVp886LineVACLookupType;

static LtTestStatusType
MeasureLineVoltage(
    LtTestCtxType *pTestCtx,
    bool calMode,
    VpTestTipSelectType lead);

static bool
LineVHandlerSM(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent,
    LtTestStatusType *pRetval);

static void
CalculateResults(
    LtTestCtxType *pTestCtx);

static int32
AdjustClippedAC(
    int32 AcVoltage,
    VpTestTipSelectType lead);

extern bool 
LtVp886LineVAttributeCheck(
    LtTestCtxType *pTestCtx)
{
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;

    LtLineVCriteriaType *ppCriteria =
        (LtLineVCriteriaType*)pTemp886->attributes.pCriteria;

    LtLineVInputType *ppInputs =
        (LtLineVInputType*)pTemp886->attributes.pInputs;

    LtVp886LineVTempType *pTempLineV = &pTemp886->tempData.lineV;

    /* Check input args */
    if (VP_NULL == ppInputs) {
        pTempLineV->input.collapseFeed = TRUE;
    } else {
        pTempLineV->input.collapseFeed = ppInputs->collapseFeed;
    }

    /* Check criteria args*/
    if (VP_NULL == ppCriteria) {
        pTempLineV->criteria.dcHemf = LT_LINEV_CRT_DC_HEMF;
        pTempLineV->criteria.acHemf = LT_LINEV_CRT_AC_HEMF;
        pTempLineV->criteria.dcFemf = LT_LINEV_CRT_DC_FEMF;
        pTempLineV->criteria.acFemf = LT_LINEV_CRT_AC_FEMF;
    } else {
        pTempLineV->criteria.dcHemf = ppCriteria->dcHemf;
        pTempLineV->criteria.acHemf = ppCriteria->acHemf;
        pTempLineV->criteria.dcFemf = ppCriteria->dcFemf;
        pTempLineV->criteria.acFemf = ppCriteria->acFemf;
    }

    /* setup function Handler */
    pTestCtx->Lt886TestEventHandlerFunc = LineVHandlerSM;
    return TRUE;
}

static bool
LineVHandlerSM(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent,
    LtTestStatusType *pRetval)
{

    VpTestResultType *pTestPrimRslts = (VpTestResultType*)pEvent->pResult;
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;
    LtVp886LineVTempType *pTempLineV = &pTemp886->tempData.lineV;

    bool requestNextState = FALSE;
    *pRetval = LT_STATUS_RUNNING;

    LT_DOUT(LT_DBG_INFO, ("LineVHandlerSM(testState:%i)", pTestCtx->state));

    switch (pTestCtx->state) {
        case LINE_V_INIT:
            LtInitOutputStruct(pTestCtx->pResult, LT_TID_LINE_V);
            pTestCtx->state = LINE_V_RUN_TIP_VXC;
            requestNextState = TRUE;
            break;

        case LINE_V_RUN_TIP_VXC:
            *pRetval = MeasureLineVoltage(pTestCtx, pTempLineV->input.collapseFeed, VP_TEST_TIP);
            pTestCtx->state = LINE_V_GET_TIP_VXC;
            break;

        case LINE_V_GET_TIP_VXC:
            LtMemCpy(&pTempLineV->vpApiTipResults, &pTestPrimRslts->result.vxc,  sizeof(VpTestResultVxcType));
            pTestCtx->state = LINE_V_RUN_RING_VXC;
            requestNextState = TRUE;
            break;

        case LINE_V_RUN_RING_VXC:
            *pRetval = MeasureLineVoltage(pTestCtx, FALSE, VP_TEST_RING);
            pTestCtx->state = LINE_V_GET_RING_VXC;
            break;

        case LINE_V_GET_RING_VXC:
            LtMemCpy(&pTempLineV->vpApiRingResults, &pTestPrimRslts->result.vxc,  sizeof(VpTestResultVxcType));
            pTestCtx->state = LINE_V_RUN_DIFF_VXC;
            requestNextState = TRUE;
            break;

        case LINE_V_RUN_DIFF_VXC:
            *pRetval = MeasureLineVoltage(pTestCtx, FALSE, VP_TEST_TIP_RING);
            pTestCtx->state = LINE_V_GET_DIFF_VXC;
            break;

        case LINE_V_GET_DIFF_VXC:
            LtMemCpy(&pTempLineV->vpApiDiffResults, &pTestPrimRslts->result.vxc,  sizeof(VpTestResultVxcType));
            pTestCtx->state = LINE_V_CONCLUDE;
            requestNextState = TRUE;
            break;

        /* Start the conclude primitive if this test not a part of an internal test */
        case LINE_V_CONCLUDE:
            if (!pTestCtx->pTemp->vp886Temp.internalTest) {
                VpTestConcludeType conclude = { FALSE };
                *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE, &conclude);
            } else {
                requestNextState = TRUE;
            }
            pTestCtx->state = LINE_V_COMPLETE;
            break;

        case LINE_V_COMPLETE:
            /* Calculate results and end the test */
            CalculateResults(pTestCtx);
            *pRetval = LT_STATUS_DONE;
            break;

        case LINE_V_ABORT:
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

static LtTestStatusType
MeasureLineVoltage(
    LtTestCtxType *pTestCtx,
    bool calMode,
    VpTestTipSelectType lead)
{
    VpTestOpenVType openVxcInput;

    openVxcInput.calMode = calMode;
    openVxcInput.integrateTime = LT_VP886_LINE_V_INTEGRATE_TIME;
    openVxcInput.settlingTime = LT_VP886_LINE_V_SETTLE_TIME;
    openVxcInput.tip = lead;

    return Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_OPEN_VXC, &openVxcInput);
}

static void
CalculateResults(
    LtTestCtxType *pTestCtx)
{
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;
    LtVp886LineVTempType *pTempLineV = &pTemp886->tempData.lineV;
    LtLineVResultType *pResults = &pTestCtx->pResult->result.lineV;

    /* get the test criteria ready */
    LtVoltageType dcHemfMax = ABS(pTempLineV->criteria.dcHemf);
    LtVoltageType acHemfMax = ABS(pTempLineV->criteria.acHemf);
    LtVoltageType dcFemfMax = ABS(pTempLineV->criteria.dcFemf);
    LtVoltageType acFemfMax = ABS(pTempLineV->criteria.acFemf);

    /* calculate tip dc and ac voltages */
    pResults->vDcTip = VP886_UNIT_CONVERT(pTempLineV->vpApiTipResults.vdc,
        VP886_UNIT_ADC_VAB, VP886_UNIT_MV);

    pResults->vAcTip = VP886_UNIT_CONVERT(-pTempLineV->vpApiTipResults.vac,
        VP886_UNIT_ADC_VAB, VP886_UNIT_MV);


    /* calculate ring dc and ac voltages */
    pResults->vDcRing = VP886_UNIT_CONVERT(pTempLineV->vpApiRingResults.vdc,
        VP886_UNIT_ADC_VAB, VP886_UNIT_MV);

    pResults->vAcRing = VP886_UNIT_CONVERT(-pTempLineV->vpApiRingResults.vac,
        VP886_UNIT_ADC_VAB, VP886_UNIT_MV);


    /* calculate diff dc and ac voltages */
    pResults->vDcDiff = VP886_UNIT_CONVERT(pTempLineV->vpApiDiffResults.vdc,
        VP886_UNIT_ADC_VAB, VP886_UNIT_MV);

    pResults->vAcDiff = VP886_UNIT_CONVERT(-pTempLineV->vpApiDiffResults.vac,
        VP886_UNIT_ADC_VAB, VP886_UNIT_MV);


    /* Adjust for clipping that may occur in the measurements of AC voltages */
    pResults->vAcTip = AdjustClippedAC(pResults->vAcTip, VP_TEST_TIP);
    pResults->vAcRing = AdjustClippedAC(pResults->vAcRing, VP_TEST_RING);
    pResults->vAcDiff = AdjustClippedAC(pResults->vAcDiff, VP_TEST_TIP_RING);


    /* Set the mask to pass and if any limits are excedded then or in the faults */
    pResults->fltMask = LT_TEST_PASSED;

    if ( ABS(pResults->vDcTip) > dcHemfMax) {
        pResults->fltMask |= LT_LVM_HEMF_DC_TIP;
    }
    if ( ABS(pResults->vDcRing) > dcHemfMax) {
        pResults->fltMask |= LT_LVM_HEMF_DC_RING;
    }
    if ( ABS(pResults->vDcDiff) > dcHemfMax) {
        pResults->fltMask |= LT_LVM_HEMF_DC_DIFF;
    }

    if (ABS(pResults->vAcTip) > acHemfMax) {
        pResults->fltMask |= LT_LVM_HEMF_AC_TIP;
    }
    if (ABS(pResults->vAcRing) > acHemfMax) {
        pResults->fltMask |= LT_LVM_HEMF_AC_RING;
    }
    if (ABS(pResults->vAcDiff) > acHemfMax) {
        pResults->fltMask |= LT_LVM_HEMF_AC_DIFF;
    }

    if ( ABS(pResults->vDcTip) > dcFemfMax) {
        pResults->fltMask |= LT_LVM_FEMF_DC_TIP;
    }
    if ( ABS(pResults->vDcRing) > dcFemfMax) {
        pResults->fltMask |= LT_LVM_FEMF_DC_RING;
    }
    if ( ABS(pResults->vDcDiff) > dcFemfMax) {
        pResults->fltMask |= LT_LVM_FEMF_DC_DIFF;
    }

    if (ABS(pResults->vAcTip) > acFemfMax) {
        pResults->fltMask |= LT_LVM_FEMF_AC_TIP;
    }
    if (ABS(pResults->vAcRing) > acFemfMax) {
        pResults->fltMask |= LT_LVM_FEMF_AC_RING;
    }
    if (ABS(pResults->vAcDiff) > acFemfMax) {
        pResults->fltMask |= LT_LVM_FEMF_AC_DIFF;
    }

    /* don't let the line out of disconnect if the test fails */
    if (pResults->fltMask != LT_TEST_PASSED) {
        pTemp886->resources.initialLineState = VP_LINE_DISCONNECT;
    }

    return;
}

/** AdjustClippedAC()
  When measuring a high AC voltage, the peaks of the waveform may exceed the
  maximum measurement threshold of the device.  This means we effectively
  measure the RMS value of a clipped sine wave instead of the full wave.

  This function maps the RMS value of a clipped sine wave to the RMS value of
  the corresponding full sine wave that we assume is actually present.
*/
static int32
AdjustClippedAC(
    int32 AcVoltage,
    VpTestTipSelectType lead)
{
    uint8 index;
    uint8 tableSize;
    LtVp886LineVACLookupType* pTable;

    /* This table assumes clipping points of +/- 270V */
    #define LT_886_CLIPPED_AC_PER_LEAD_TABLE_SIZE 22
    LtVp886LineVACLookupType perLeadTable[LT_886_CLIPPED_AC_PER_LEAD_TABLE_SIZE] = {
        {/* 194.3 V */ 194264, 195000 /* 195 V */},
        {/* 197.8 V */ 197815, 200000 /* 200 V */},
        {/* 200.6 V */ 200614, 205000 /* 205 V */},
        {/* 203.3 V */ 203305, 210000 /* 210 V */},
        {/* 205.9 V */ 205905, 215000 /* 215 V */},
        {/* 207.8 V */ 207825, 220000 /* 220 V */},
        {/* 209.8 V */ 209771, 225000 /* 225 V */},
        {/* 211.7 V */ 211742, 230000 /* 230 V */},
        {/* 213.7 V */ 213738, 235000 /* 235 V */},
        {/* 215.2 V */ 215178, 240000 /* 240 V */},
        {/* 216.5 V */ 216494, 245000 /* 245 V */},
        {/* 217.8 V */ 217829, 250000 /* 250 V */},
        {/* 219.2 V */ 219183, 255000 /* 255 V */},
        {/* 220.6 V */ 220555, 260000 /* 260 V */},
        {/* 221.9 V */ 221945, 265000 /* 265 V */},
        {/* 223.4 V */ 223353, 270000 /* 270 V */},
        {/* 224.2 V */ 224170, 275000 /* 275 V */},
        {/* 225.0 V */ 225000, 280000 /* 280 V */},
        {/* 225.8 V */ 225842, 285000 /* 285 V */},
        {/* 226.7 V */ 226695, 290000 /* 290 V */},
        {/* 227.6 V */ 227560, 295000 /* 295 V */},
        {/* 228.4 V */ 228436, 300000 /* 300 V */},
    };

    /* This table assumes clipping points of +270V, -225V */
    #define LT_886_CLIPPED_AC_DIFF_TABLE_SIZE 29
    LtVp886LineVACLookupType diffTable[LT_886_CLIPPED_AC_DIFF_TABLE_SIZE] = {
        {/* 160.0 V */ 159955, 160000 /* 160 V */},
        {/* 164.3 V */ 164331, 165000 /* 165 V */},
        {/* 168.4 V */ 168374, 170000 /* 170 V */},
        {/* 172.2 V */ 172233, 175000 /* 175 V */},
        {/* 176.0 V */ 176000, 180000 /* 180 V */},
        {/* 179.5 V */ 179504, 185000 /* 185 V */},
        {/* 183.0 V */ 183034, 190000 /* 190 V */},
        {/* 186.2 V */ 186205, 195000 /* 195 V */},
        {/* 188.8 V */ 188792, 200000 /* 200 V */},
        {/* 190.9 V */ 190885, 205000 /* 205 V */},
        {/* 192.9 V */ 192935, 210000 /* 210 V */},
        {/* 194.9 V */ 194948, 215000 /* 215 V */},
        {/* 196.6 V */ 196613, 220000 /* 220 V */},
        {/* 198.3 V */ 198302, 225000 /* 225 V */},
        {/* 199.7 V */ 199729, 230000 /* 230 V */},
        {/* 201.2 V */ 201178, 235000 /* 235 V */},
        {/* 202.3 V */ 202338, 240000 /* 240 V */},
        {/* 203.4 V */ 203440, 245000 /* 245 V */},
        {/* 204.6 V */ 204558, 250000 /* 250 V */},
        {/* 205.7 V */ 205692, 255000 /* 255 V */},
        {/* 206.8 V */ 206843, 260000 /* 260 V */},
        {/* 208.0 V */ 208009, 265000 /* 265 V */},
        {/* 209.2 V */ 209191, 270000 /* 270 V */},
        {/* 209.9 V */ 209870, 275000 /* 275 V */},
        {/* 210.5 V */ 210528, 280000 /* 280 V */},
        {/* 211.2 V */ 211196, 285000 /* 285 V */},
        {/* 211.9 V */ 211874, 290000 /* 290 V */},
        {/* 212.6 V */ 212562, 295000 /* 295 V */},
        {/* 213.3 V */ 213259, 300000 /* 300 V */},
    };

    if (lead == VP_TEST_TIP_RING) {
        pTable = diffTable;
        tableSize = LT_886_CLIPPED_AC_DIFF_TABLE_SIZE;
    } else {
        pTable = perLeadTable;
        tableSize = LT_886_CLIPPED_AC_PER_LEAD_TABLE_SIZE;
    }

    /* Do not modify the AC voltage if it's in the linear area */
    if (AcVoltage < pTable[0].measured) {
        return AcVoltage;
    }

    /* Saturate the AC voltage measurement */
    if (AcVoltage >= pTable[tableSize-1].measured) {
        return pTable[tableSize-1].estimated;
    }

    for (index = 0; index < tableSize-1; index++){
        if (AcVoltage < pTable[index+1].measured) {
            /* Linear Interpolation
             * vm = voltage measured   m[i] = table[index].measured
             * ve = voltage result     e[i] = table[index].estimated
             * Formula: vr = (( vm - m[i] ) / ( m[i+1] - m[i] )) * ( e[i+1] - e[i] ) + e[i]
             * Terms:   vr = ((   termA   ) / (      termB    )) * (     termC     ) + e[i]
             * Fixed point:  vr = (((termA * 1000) / termB) * termC) / 1000 + e[i];
             */
            int32 termA, termB, termC;
            termA = AcVoltage - pTable[index].measured;
            termB = pTable[index+1].measured - pTable[index].measured;
            termC = pTable[index+1].estimated - pTable[index].estimated;

            return (((termA * 1000) / termB) * termC) / 1000 + pTable[index].estimated;
        }
    }

    /* Should never reach here, but return the maximum voltage. */
    return pTable[tableSize-1].estimated;
}
#endif /* LT_LINE_V_886 */
