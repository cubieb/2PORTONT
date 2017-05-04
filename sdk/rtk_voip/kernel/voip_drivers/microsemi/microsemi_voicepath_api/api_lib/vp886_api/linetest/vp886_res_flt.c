/*
 *  This file contains the 3 ele res primitive algorithm
 *
 *  This version of the primitive contains both the
 *  low gain and normal/high gain algorithms.
 *
 *  The intended way to call into this code is to call
 *  into the Low Gain algorithim first. If the Low Gain algorithim
 *  decides that it is unable to take proper measurements due to
 *  settling time issues or saturated current measurements then the
 *  code will automatically jump to the high gain algorithim.
 *
 *  It is possible to bypass the Low Gain algorithim and go straight
 *  into the high gain algorithim.
 *
 * Copyright (c) 2012, Microsemi Corporation
 *
 * $Revision: 11210 $
 * $LastChangedDate: 2013-11-05 14:33:59 -0600 (Tue, 05 Nov 2013) $
 */

#include "vp_api_cfg.h"

#if defined (VP_CC_886_SERIES)
#if defined (VP886_INCLUDE_TESTLINE_CODE)

#include "vp886_testline_int.h"

#define FEED_CHANGE_TIME_CONSTANT       50       /* 50ms */
#define SETTLING_ERROR_THRESHOLD_RG     1        /* 1 adu err in ringing*/
#define SETTLING_ERROR_THRESHOLD_LG     5        /* +/- 5 adu err low gain */
#define SETTLING_ERROR_THRESHOLD_HG     2        /* +/- 2  adu err high gain */

#define IMT_LG_TO_HG_THRESHOLD          32500    /* Imt threshold to switch from LG to HG */
#define ILG_LG_TO_HG_THRESHOLD          19000    /* Ilg threshold to switch from LG to HG */
#define VAB_LG_TO_HG_THRESHOLD          50       /* VAB threshold to switch from LG to HG */
#define IMT_ACTIVE_10MA                 10       /* Metallic current threshold to detect off-hook */
#define IMT_LG_NF_RG_SETTLE_THRESHOLD   4        /* Settling count for the first imt ringing measurement */

#define MAX_ITERATION_RG_COUNT          30       /* Max number of Ringing settling measurement iterations */
#define MAX_ITERATION_LG_COUNT          80       /* Max number of Low Gain settling measurement iterations */

#define MAX_ITERATION_HG_18MS_COUNT     20       /* Max number of 18ms High Gain settling measurement iterations */
#define MAX_ITERATION_HG_100MS_COUNT    10       /* Max number of 100ms High Gain settling measurement iterations */

#define IMT_RMS_18MS_TO_100MS_THRESHOLD 400      /* IMT threshold to switch from 18ms to 100ms integration times due to a foreign voltage */
#define SADC_SETTLE_FRAMES_2MS          (2*8)    /* Number of Frames that the SADC will skip before measuring */
#define SADC_INTEGRATION_FRAMES_18MS    (18*8)   /* Number of Frames that the SADC will integrate */
#define SADC_INTEGRATION_FRAMES_100MS   (100*8)  /* Number of Frames that the SADC will integrate */

#define USE_VOCCOR_FOR_LS_TEST

typedef enum
{
    RES_FLT_CHECK_RELAY             = 0,
    RES_FLT_DEBOUNCE_HOOK_GKEY      = 1,
    RES_FLT_CHECK_HOOK_GKEY         = 2,

    RES_FLT_LG_SETUP                = 4,
    RES_FLT_LG_CHANGE_LINESTATE     = 5,
    RES_FLT_LG_CHANGE_FEED          = 6,

    RES_FLT_LG_START_MEASUREMENT    = 7,
    RES_FLT_LG_GET_MEASUREMENT      = 8,


    RES_FLT_HG_SETUP                = 9,
    RES_FLT_HG_CHANGE_FEED          = 10,

    RES_FLT_HG_START_MEASUREMENT    = 11,
    RES_FLT_HG_GET_MEASUREMENT      = 12,

    RES_FLT_LS_SETUP                = 13,
    RES_FLT_LS_CHANGE_LINESTATE     = 14,
    RES_FLT_LS_CHECK_GKEY           = 15,

    RES_FLT_RESTORE                 = 16,
    RES_FLT_GEN_EVENT               = 17,

    RES_FLT_MAX                     = 18,
    RES_FLT_ENUM_SIZE            = FORCE_STANDARD_C_ENUM_SIZE
}  Vp886ResFltLGStateTypes;

typedef enum {

    RES_FLT_LG_IMT_NORMAL_FEED_RG   = 0,
    RES_FLT_LG_VAB_NORMAL_FEED_RG   = 1,
    RES_FLT_LG_IMT_NORMAL_FEED,
    RES_FLT_LG_ILG_NORMAL_FEED,
    RES_FLT_LG_VAL_NORMAL_FEED,
    RES_FLT_LG_VBL_NORMAL_FEED,

    RES_FLT_LG_IMT_ZERO_FEED_RG     = 6,
    RES_FLT_LG_VAB_ZERO_FEED_RG     = 7,
    RES_FLT_LG_IMT_ZERO_FEED,
    RES_FLT_LG_ILG_ZERO_FEED,
    RES_FLT_LG_VAL_ZERO_FEED,
    RES_FLT_LG_VBL_ZERO_FEED,

    RES_FLT_LG_IMT_REVERSE_FEED_RG  = 12,
    RES_FLT_LG_VAB_REVERSE_FEED_RG  = 13,
    RES_FLT_LG_IMT_REVERSE_FEED,
    RES_FLT_LG_ILG_REVERSE_FEED,
    RES_FLT_LG_VAL_REVERSE_FEED,
    RES_FLT_LG_VBL_REVERSE_FEED,

    RES_FLT_LG_MAX_ADC,
    RES_FLT_LG_ADC_ORDER_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE
}  ResFltLGSadcMeasurementTypes;

typedef enum {
    RES_FLT_HG_IMT_ZERO_FEED,
    RES_FLT_HG_ILG_ZERO_FEED,
    RES_FLT_HG_VAH_ZERO_FEED,
    RES_FLT_HG_VBH_ZERO_FEED,

    RES_FLT_HG_IMT_REVERSE_FEED,
    RES_FLT_HG_ILG_REVERSE_FEED,
    RES_FLT_HG_VAH_REVERSE_FEED,
    RES_FLT_HG_VBH_REVERSE_FEED,

    RES_FLT_HG_IMT_NORMAL_FEED,
    RES_FLT_HG_ILG_NORMAL_FEED,
    RES_FLT_HG_VAH_NORMAL_FEED,
    RES_FLT_HG_VBH_NORMAL_FEED,

    RES_FLT_HG_MAX_ADC,
    RES_FLT_HG_ADC_ORDER_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
}  ResFltHGSadcMeasurementTypes;


typedef enum {
    RES_FLT_LONG_SHRT_TIP_OPEN,
    RES_FLT_LONG_SHRT_RING_OPEN,

    RES_FLT_LONG_SHRT_MAX,
    RES_FLT_LONG_SHRT_ORDER_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} ResFltLongShortTypes;


static bool
Vp886TestResFltSM(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    VpTest3EleResAltResType *pResFltArgs,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap);

static bool
VpResSetupLG(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap);

static bool
VpResSetBackDoorLineStateLG(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap);

static bool
VpResSetFeedLG(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap,
    uint8 *nextTimer);

static bool
VpResStartMeasurementLG(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap,
    uint8 adcState);

static bool
VpResStoreMeasurementLG(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap,
    uint8 adcState,
    int16 *pAdcAverage,
    int16 *pAdcMin,
    int16 *pAdcMax);

static bool
VpResNeedMoreSettlingTimeLG(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap,
    int16 currentValue,
    int16 currentAdcMin,
    int16 currentAdcMax,
    uint8 adcState);

static bool
VpResJumpToHighGain(
    VpLineCtxType *pLineCtx,
    uint8 adcState);

static bool
VpResSetupHG(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap);

static bool
VpResChangeFeedHG(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap,
    uint8 adcState,
    uint8 *feedSettleInMs);

static bool
VpResStartMeasurementHG(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap,
    uint8 adcState);

static int16
VpResStoreMeasurementHG(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap,
    uint8 adcState,
    int16 *adcAverage);

static bool
VpResNeedMoreSettlingTimeHG(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap,
    int16 currentValue,
    uint8 adcState);

static bool
VpResIsLStPresent(
    VpLineCtxType *pLineCtx,
    uint8 adcState);

static bool
VpResChangeFeedLS(
    VpLineCtxType *pLineCtx,
    uint8 adcState,
    uint32 *pDebounce);

static bool
VpResCheckForGKeyLS(
    VpLineCtxType *pLineCtx,
    uint8 adcState);

static bool
VpResRestore(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap);

/*------------------------------------------------------------------------------
 * Vp886TestResFlt
 *----------------------------------------------------------------------------*/
EXTERN VpStatusType
Vp886TestResFlt(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    const void *pArgsUntyped,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap,
    bool callback)
{

    VpTest3EleResAltResType *pResFltLgArgs = NULL;
    VpStatusType status = VP_STATUS_SUCCESS;

    if (!callback) {
        const VpTest3EleResAltResType *pInputArgs = (VpTest3EleResAltResType *)pArgsUntyped;

        if (pInputArgs == NULL) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestResFltLG NULL input struct"));
            return VP_STATUS_INVALID_ARG;
        }
        pTestHeap->previousAvg = 0;
        pTestHeap->settlingCount = 0;
        pTestHeap->adcState = RES_FLT_LG_MAX_ADC;
        pTestHeap->vabComputed = 0;
        pTestHeap->lowGain = FALSE;
        pLineObj->testResults.result.resAlt.highGain = FALSE; /* low-gain */

        /* reset the results to non-meaningful values */
        VpMemSet(&pLineObj->testResults.result.resAlt, 0, sizeof(VpTestResultAltResType));

        /* store the input args into the test heap for later use */
        VpMemCpy(&pTestHeap->testArgs.resFltAlt, pInputArgs, sizeof(VpTest3EleResAltResType));

        pTestHeap->nextState = RES_FLT_CHECK_RELAY;
        pTestHeap->testArgs.resFltAlt.integrateTime = SADC_INTEGRATION_FRAMES_18MS;
    }

    pResFltLgArgs = &pTestHeap->testArgs.resFltAlt;

    /* Continue calling the state machine until it wants to stop */
    while (Vp886TestResFltSM(pLineCtx, pLineObj, pResFltLgArgs, pTestInfo, pTestHeap));

    return status;
}

/*------------------------------------------------------------------------------
 * Vp886TestResFltSM
 *----------------------------------------------------------------------------*/
static bool
Vp886TestResFltSM(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    VpTest3EleResAltResType *pResFltArgs,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap)
{
    bool requestNextState = FALSE;

    VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestResFltSM state: %i", pTestHeap->nextState));

    switch (pTestHeap->nextState) {

        case RES_FLT_CHECK_RELAY:
            /* This test cannot run if the isolate relay is open*/
            if (pLineObj->relayState == VP_RELAY_RESET) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestResFltSM: Relay open."));
                break;
            }

            if (pTestInfo->testId == VP_TEST_ID_3ELE_RES_HG) {
                pTestHeap->nextState = RES_FLT_HG_SETUP;
            } else if ((pLineObj->registers.sysState[0] &VP886_R_STATE_SS) == VP886_R_STATE_SS_DISCONNECT) {
                pTestHeap->nextState = RES_FLT_LG_SETUP;
                VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestResFltSM Skipping Hook and Gkey check"));
            } else {
                pTestHeap->nextState = RES_FLT_DEBOUNCE_HOOK_GKEY;
            }

            requestNextState = TRUE;
            break;

        case RES_FLT_DEBOUNCE_HOOK_GKEY: {
            /* The prepare primitive may or may not have run just before this primitive was run.
               The prepare primitive will disconnect and reconnect the t/r sense path. When the 
               sense is reconnected, the device will debounce hook and gkey signals. To be safe
               we are going to set a timer equal to the longer of the two debounce times to
               ensure we actually see the hook or gkey. */

            uint16 debounce = 2 * (pLineObj->registers.loopSup[1] & VP886_R_LOOPSUP_HOOK_DBNC);
            uint16 gkDebounce = 4 * ((pLineObj->registers.loopSup[1] & VP886_R_LOOPSUP_GKEY_DBNC) >> 5);
            if (debounce < gkDebounce) {
                debounce = gkDebounce;
            }

            /* wait for the requested amount of time */
            if (Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_LINE_TEST, debounce*2, 0, RES_FLT_CHECK_HOOK_GKEY) == FALSE) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
                VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestResFltSM: Cannot add to timer queue."));
                break;
            }

            pTestHeap->nextState = RES_FLT_CHECK_HOOK_GKEY;
            break;
        }

        case RES_FLT_CHECK_HOOK_GKEY: {
            Vp886DeviceObjectType *pDevObj = pLineCtx->pDevCtx->pDevObj;

            if (pDevObj->registers.sigreg[pLineObj->channelId] & VP886_R_SIGREG_HOOK) {
                VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestResFltSM Hook detected Jump to High Gain"));
                pTestHeap->nextState = RES_FLT_HG_SETUP;

            } else if (pDevObj->registers.sigreg[pLineObj->channelId] & VP886_R_SIGREG_GNK) {
                VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestResFltSM Gkey detected Jump to High Gain"));
                pTestHeap->nextState = RES_FLT_HG_SETUP;
            } else {
                pTestHeap->nextState = RES_FLT_LG_SETUP;
            }

            requestNextState = TRUE;
            break;
        }


        /***************** Start of the Low Gain States ********************/
        /* Force low gain setup */
        case RES_FLT_LG_SETUP:
            pTestHeap->adcState = RES_FLT_LG_IMT_NORMAL_FEED_RG;
            if (VpResSetupLG(pLineCtx, pTestHeap) == FALSE) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                break;
            }
            pTestHeap->nextState = RES_FLT_LG_CHANGE_LINESTATE;
            requestNextState = TRUE;
            break;

        case RES_FLT_LG_CHANGE_LINESTATE:
            if (VpResSetBackDoorLineStateLG(pLineCtx, pTestHeap) == FALSE) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                break;
            }
            pTestHeap->nextState = RES_FLT_LG_CHANGE_FEED;
            requestNextState = TRUE;
            break;

        case RES_FLT_LG_CHANGE_FEED: {
            uint8 feedSettleTimeInMs;

            if (VpResSetFeedLG(pLineCtx, pTestHeap, &feedSettleTimeInMs) == FALSE) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                break;
            }

            if (feedSettleTimeInMs > 0) {
                /* wait for the requested amount of time */
                if (Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_LINE_TEST, feedSettleTimeInMs, 0, RES_FLT_LG_CHANGE_FEED) == FALSE) {
                    Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
                    VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestResFltSM: Cannot add to timer queue."));
                    break;
                }
            } else {
                requestNextState = TRUE;
            }
            pTestHeap->nextState = RES_FLT_LG_START_MEASUREMENT;
            break;
        }

        case RES_FLT_LG_START_MEASUREMENT:
            if (VpResStartMeasurementLG(pLineCtx, pTestHeap, pTestHeap->adcState) == FALSE) {
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestResFltSM unable to setup SADC"));
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
                break;
            }

            pTestHeap->nextState = RES_FLT_LG_GET_MEASUREMENT;
            break;

        case RES_FLT_LG_GET_MEASUREMENT: {
            int16 adcAverage = 0;
            int16 adcMin = 0;
            int16 adcMax = 0;

            /* get the adc measurement */
            if (VpResStoreMeasurementLG(pLineCtx, pTestHeap, pTestHeap->adcState, &adcAverage, &adcMin, &adcMax) == FALSE) {
                VP_ERROR(VpLineCtxType, pLineCtx, ("p886TestResFltLGSM calculations failed"));
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
                break;
            }

            /* Where we go is based on the measured value */
            if (VpResNeedMoreSettlingTimeLG(pLineCtx, pTestHeap, adcAverage, adcMin, adcMax, pTestHeap->adcState) == TRUE) {

                /* the measurement needs more time to settle so retake the measurement */
                pTestHeap->nextState = RES_FLT_LG_START_MEASUREMENT;

            } else if (VpResJumpToHighGain(pLineCtx, pTestHeap->adcState) == TRUE) {

                /* active, normal or zero feed metallic currents are too high, jump to high gain */
                pTestHeap->nextState = RES_FLT_HG_SETUP;

            } else if ( ++(pTestHeap->adcState) < RES_FLT_LG_MAX_ADC) {

                /* There are still mesurements to be taken */
                pTestHeap->nextState = RES_FLT_LG_CHANGE_LINESTATE;

            } else {
                /* Nothing left to do so exit the primitive */
                pTestHeap->nextState = RES_FLT_RESTORE;
            }

            requestNextState = TRUE;
            break;
        }

        /***************** End of the Low Gain States ***********************/

        /***************** Start of the High Gain States ********************/
        /* Force high gain setup (only occurs once) */
        case RES_FLT_HG_SETUP:
            pTestHeap->adcState = RES_FLT_HG_IMT_ZERO_FEED;
            pLineObj->testResults.result.resAlt.highGain = TRUE;

            if (VpResSetupHG(pLineCtx, pTestHeap) == FALSE) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                break;
            }
            pTestHeap->nextState = RES_FLT_HG_CHANGE_FEED;
            requestNextState = TRUE;
            break;


        case RES_FLT_HG_CHANGE_FEED: {
            uint8 feedSettleTimeInMs;

            /* Change the current feed condtion and power if needeed */
            if (VpResChangeFeedHG(pLineCtx, pTestHeap, pTestHeap->adcState, &feedSettleTimeInMs) == FALSE) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                break;
            }

            if (feedSettleTimeInMs > 0) {
                /* wait for the requested amount of time */
                if (Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_LINE_TEST, feedSettleTimeInMs, 0, RES_FLT_HG_CHANGE_FEED) == FALSE) {
                    Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
                    VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestResFltHGSM: Cannot add to timer queue."));
                    break;
                }
            } else {
                requestNextState = TRUE;
            }
            pTestHeap->nextState = RES_FLT_HG_START_MEASUREMENT;
            break;
        }

        case RES_FLT_HG_START_MEASUREMENT:
            if (VpResStartMeasurementHG(pLineCtx, pTestHeap, pTestHeap->adcState) == FALSE) {
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestResFltSM unable to setup SADC"));
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
                break;
            }

            pTestHeap->nextState = RES_FLT_HG_GET_MEASUREMENT;
            break;

        case RES_FLT_HG_GET_MEASUREMENT: {
            int16 adcAverage = 0;

            /* get the adc measurement */
            if (VpResStoreMeasurementHG(pLineCtx, pTestHeap, pTestHeap->adcState, &adcAverage) == FALSE) {
                VP_ERROR(VpLineCtxType, pLineCtx, ("p886TestResFltSM calculations failed"));
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
                break;
            }

            /* Where do we go based on the measured value */
            if (VpResNeedMoreSettlingTimeHG(pLineCtx, pTestHeap, adcAverage, pTestHeap->adcState)) {

                /* the measurement needs more time to settle so retake the measurement */
                pTestHeap->nextState = RES_FLT_HG_START_MEASUREMENT;

            } else if ( VpResIsLStPresent(pLineCtx, pTestHeap->adcState) == TRUE) {

                /* A hard short to ground or a foreign source is present */
                pTestHeap->nextState = RES_FLT_LS_SETUP;

            } else if ( ++(pTestHeap->adcState) < RES_FLT_HG_MAX_ADC) {

                /* There are still mesurements to be taken */
                pTestHeap->nextState = RES_FLT_HG_CHANGE_FEED;

            } else {
                /* Nothing left to do so exit the primitive */
                pTestHeap->nextState = RES_FLT_RESTORE;
            }

            requestNextState = TRUE;

            break;
        }

        /***************** End of the High Gain States ***********************/


        /***************** Start of the Longitudinal Short States ************/

        case RES_FLT_LS_SETUP:

            if (pLineObj->testResults.result.resAlt.ilzf == VP_INT16_MIN) {
                /* foreign current source detected get out */
                pTestHeap->nextState = RES_FLT_RESTORE;
            } else {
                pTestHeap->adcState = RES_FLT_LONG_SHRT_TIP_OPEN;
                pTestHeap->nextState = RES_FLT_LS_CHANGE_LINESTATE;

                /* need to restore the line before we start tip/ring open */
                if(VpResRestore(pLineCtx, pTestHeap) == FALSE) {
                    Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                    break;
                }
            }
            requestNextState = TRUE;
            break;

        case RES_FLT_LS_CHANGE_LINESTATE: {
            uint32 debounceTime = 0;
            if (VpResChangeFeedLS(pLineCtx, pTestHeap->adcState, &debounceTime) == FALSE) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                break;
            }

            /* wait some time for line to settle out */
            if (Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_LINE_TEST, debounceTime, 0, RES_FLT_LS_CHECK_GKEY) == FALSE) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
                VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestResFltSM: Cannot add to timer queue."));
                requestNextState = TRUE;
                break;
            }

            pTestHeap->nextState = RES_FLT_LS_CHECK_GKEY;
            break;
        }

        case RES_FLT_LS_CHECK_GKEY:
            if (VpResCheckForGKeyLS(pLineCtx, pTestHeap->adcState) == FALSE) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                break;
            }

            if ( ++(pTestHeap->adcState) < RES_FLT_LONG_SHRT_MAX) {
                pTestHeap->nextState = RES_FLT_LS_CHANGE_LINESTATE;
            } else {
                pTestHeap->nextState = RES_FLT_RESTORE;
            }
            requestNextState = TRUE;
            break;

        /***************** End of the Longitudinal Short States **************/

        case RES_FLT_RESTORE:

            if(VpResRestore(pLineCtx, pTestHeap) == FALSE) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                break;
            }

            /* wait for the requested amount of time */
            if (Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_LINE_TEST, 50, 0, RES_FLT_RESTORE) == FALSE) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
                VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestResFltSM: Cannot add to timer queue."));
                requestNextState = TRUE;
            }
            pTestHeap->nextState = RES_FLT_GEN_EVENT;
            break;

        case RES_FLT_GEN_EVENT:
            /* generate the event */
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestResFltSM(): done\n"));
            Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_SUCCESS);
            break;

        default:
            /* some how the next state variable was corrupted */
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestResFltSM invalid state: %i",
                pTestHeap->nextState));
            Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
            break;
    }
    return requestNextState;
}

/*------------------------------------------------------------------------------
 * VpResSetupLG()
 * This function is called by the Vp886Test3EleResLG() state machine during
 * the ELE3_RES_FLT_LG_SETUP state.
 *
 * This function is used to configure the line for the Low Gain Res Flt test.
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context.
 *  pTestHeap       - pointer to the test heap.
 *  deviceId        - device ID
 *
 * Returns:TRUE if no error, FALSE otherwise
 * --
 * Result:
 *
 *----------------------------------------------------------------------------*/
static bool
VpResSetupLG(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap)
{
    Vp886LineObjectType *pLineObj =  pLineCtx->pLineObj;
    Vp886DeviceObjectType *pDevObj = pLineCtx->pDevCtx->pDevObj;

    uint8 sigGenCtrl = VP886_R_SIGCTRL_EN_BIAS | VP886_R_SIGCTRL_EN_SIGA;
    uint8 lpSuper[VP886_R_LOOPSUP_LEN];

    VP_TEST(VpLineCtxType, pLineCtx, ("VpResSetupLG()"));

    if (Vp886CalmodeTestSetup(pLineCtx, pTestHeap) == FALSE) {
        return FALSE;
    }

    /* this flag indicates if the ringing states are needed to drive line */
    pTestHeap->lowGain = FALSE;

    /* need to be in the active state */
    pLineObj->registers.sysState[0] = VP886_R_STATE_CODEC | VP886_R_STATE_SS_ACTIVE;

    if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_STATE_WRT, VP886_R_STATE_LEN, pLineObj->registers.sysState) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpResSetupLG failed VP886_R_STATE_WRT"));
        return FALSE;
    }

    /* Masking the hook detection by maxing out TSH */
    if (VpSlacRegRead(NULL, pLineCtx, VP886_R_LOOPSUP_RD, VP886_R_LOOPSUP_LEN, pLineObj->registers.loopSup) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpResSetupLG failed VP886_R_LOOPSUP_RD"));
        return FALSE;
    }

    lpSuper[0] = (pLineObj->registers.loopSup[0]) | 0x07;
    lpSuper[1] = 0xFF;
    lpSuper[2] = pLineObj->registers.loopSup[2];
    lpSuper[3] = pLineObj->registers.loopSup[3];
    lpSuper[4] = pLineObj->registers.loopSup[4];

    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_LOOPSUP_WRT, VP886_R_LOOPSUP_LEN, lpSuper) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpResSetupLG failed VP886_R_LOOPSUP_WRT"));
        return FALSE;
    }

    if (VP886_IS_TRACKER(pDevObj)) {
        uint8 switchRegParam[VP886_R_SWPARAM_LEN] = {0x44, 0x04, 0x04}; /* battery 25V fixed */

        if (VpSlacRegRead(NULL, pLineCtx, VP886_R_SWPARAM_RD, VP886_R_SWPARAM_LEN, pLineObj->registers.swParam) == FALSE) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("VpResSetupLG failed VP886_R_SWPARAM_RD"));
            return FALSE;
        }

        switchRegParam[0] = (pLineObj->registers.swParam[0] & 0xA0) | switchRegParam[0];  /*RTM=1, SWF[4:0] = 00100*/
        switchRegParam[1] = (pLineObj->registers.swParam[1] & 0xE0) | switchRegParam[1];  /*SWRV = 00100; */
        switchRegParam[2] = (pLineObj->registers.swParam[2] & 0xE0) | switchRegParam[2];  /*SWLV = 00100*/

        if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_SWPARAM_WRT, VP886_R_SWPARAM_LEN, switchRegParam) == FALSE) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("VpResSetupLG() failed VP886_R_SWPARAM_WRT"));
            return FALSE;
        }
    }

    pLineObj->registers.icr4[0] = 0xFF;
    pLineObj->registers.icr4[1] = 0x03;
    pLineObj->registers.icr4[2] = 0xFF;
    pLineObj->registers.icr4[3] = 0x2C;

    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR4_WRT, VP886_R_ICR4_LEN, pLineObj->registers.icr4) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpResSetupLG() failed VP886_R_ICR4_WRT"));
        return FALSE;
    }

    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_SIGCTRL_WRT, VP886_R_SIGCTRL_LEN, &sigGenCtrl) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpResSetupLG() failed VP886_R_SIGCTRL_WRT"));
        return FALSE;
    }

    return TRUE;
}
static bool
VpResSetBackDoorLineStateLG(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap)
{
    Vp886LineObjectType *pLineObj =  pLineCtx->pLineObj;
    Vp886DeviceObjectType *pDevObj = pLineCtx->pDevCtx->pDevObj;

    switch (pTestHeap->adcState) {
        case RES_FLT_LG_IMT_NORMAL_FEED_RG:
        case RES_FLT_LG_IMT_ZERO_FEED_RG:
        case RES_FLT_LG_IMT_REVERSE_FEED_RG:
            /*
             * If the low gain algorithim has determined that we don't
             * need to use the ringing states for the zero or reverse feed
             * measurements then don't change the line state because by
             * this time we are already in low gain.
             */
            if ( (pTestHeap->lowGain == TRUE) &&
                ((pTestHeap->adcState == RES_FLT_LG_IMT_ZERO_FEED_RG) ||
                 (pTestHeap->adcState == RES_FLT_LG_IMT_REVERSE_FEED_RG))) {
                return TRUE;
            }

            /*
             * Force the line into the ringing state so that we
             * drive the line to the feed voltage we need.
             */
            pLineObj->registers.icr1[0] = 0xFF;
            pLineObj->registers.icr1[1] = 0xC8;

            if (VP886_IS_ABS(pDevObj)) {
                pLineObj->registers.icr1[2] = 0xFF;
            } else  {
                pLineObj->registers.icr1[2] = 0xCF;
            }
            pLineObj->registers.icr1[3] = 0x4C;

            pLineObj->registers.icr2[0] = 0xFF;
            pLineObj->registers.icr2[1] = 0xDC;
            pLineObj->registers.icr2[2] &= 0x0C;
            pLineObj->registers.icr2[2] |= 0xF3;
            pLineObj->registers.icr2[3] &= 0x0C;
            if (VP886_IS_ABS(pDevObj)) {
                pLineObj->registers.icr2[3] |= 0x31;
            } else  {
                pLineObj->registers.icr2[3] |= 0x71;
            }

            pLineObj->registers.icr3[0] = 0xFF;
            pLineObj->registers.icr3[1] = 0x21;
            pLineObj->registers.icr3[2] = 0xFF;
            /*
             * If ABS device then icr3 need to set low battery
             * If not abs but is channel 0 then icr3 needs to set y battery
             */
            if ( (VP886_IS_ABS(pDevObj)) || (pLineObj->channelId == 0) ) {
                pLineObj->registers.icr3[3] = 0xCE;
            } else {
                pLineObj->registers.icr3[3] = 0xE6;
            }
            break;

        case RES_FLT_LG_IMT_NORMAL_FEED:
        case RES_FLT_LG_IMT_ZERO_FEED:
        case RES_FLT_LG_IMT_REVERSE_FEED:
            /*
             * Force the line into Low gain mode to
             * that we can take the measurements we need
             */
            pLineObj->registers.icr1[0] = 0xFF;
            pLineObj->registers.icr1[1] = 0x01;
            if (VP886_IS_ABS(pDevObj)) {
                pLineObj->registers.icr1[2] = 0xFF;
            } else  {
                pLineObj->registers.icr1[2] = 0xCF;
            }
            pLineObj->registers.icr1[3] = 0x40;

            pLineObj->registers.icr2[0] = 0xFF;
            pLineObj->registers.icr2[1] = 0xFD;
            pLineObj->registers.icr2[2] &= 0x0C;
            pLineObj->registers.icr2[2] |= 0xF3;
            pLineObj->registers.icr2[3] &= 0x0C;
            if (VP886_IS_ABS(pDevObj)) {
                pLineObj->registers.icr2[3] |= 0x31;
            } else  {
                pLineObj->registers.icr2[3] |= 0x71;
            }

            pLineObj->registers.icr3[0] = 0xFF;
            pLineObj->registers.icr3[1] = 0xE9;
            pLineObj->registers.icr3[2] = 0xFF;
            pLineObj->registers.icr3[3] = 0xC0;

            break;

        default:
            /* no other sadc neasurements need to adjust the line state */
            return TRUE;
    }

    VP_TEST(VpLineCtxType, pLineCtx, ("VpResSetBackDoorLineStateLG() adcState %i", pTestHeap->adcState));

    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR3_WRT, VP886_R_ICR3_LEN, pLineObj->registers.icr3) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpSetBackDoorLineStateLG() failed VP886_R_ICR3_WRT"));
        return FALSE;
    }

    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR2_WRT, VP886_R_ICR2_LEN, pLineObj->registers.icr2) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpSetBackDoorLineStateLG() failed VP886_R_ICR2_WRT"));
        return FALSE;
    }

    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR1_WRT, VP886_R_ICR1_LEN, pLineObj->registers.icr1) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpSetBackDoorLineStateLG() failed VP886_R_ICR1_WRT"));
        return FALSE;
    }

    return TRUE;
}

static bool
VpResSetFeedLG(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap,
    uint8 *nextTimer)
{
    Vp886LineObjectType *pLineObj =  pLineCtx->pLineObj;
    Vp886DeviceObjectType *pDevObj = pLineCtx->pDevCtx->pDevObj;
    Vp886CmnCalDeviceDataType *pCalData = &pDevObj->calData[pLineObj->channelId].cmn;

    int16 feedBias;
    int32 feedBias_mV = 0;
    int32 buffErr;
    uint8 sigGenA[VP886_R_SIGAB_LEN] =
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8 dcfeed[VP886_R_DCFEED_LEN] = {0xEF, 0x3F};

    /* Initialize the 'previous' measurement used in the settling algorithm */
    pTestHeap->previousAvg = 0;
    
    /* Initialize the settling counter and the ADC retry counter */
    pTestHeap->settlingCount = 0;

    /* might need to settle for a bit */
    *nextTimer = FEED_CHANGE_TIME_CONSTANT;

    VP_TEST(VpLineCtxType, pLineCtx, ("VpResSetFeedLG() adcState %i", pTestHeap->adcState));

    switch (pTestHeap->adcState) {
        case RES_FLT_LG_IMT_NORMAL_FEED_RG:
            feedBias_mV = VP886_UNIT_CONVERT(-pTestHeap->testArgs.resFltAlt.feedBias, VP886_UNIT_DAC_RING, VP886_UNIT_MV);
            break;

        case RES_FLT_LG_IMT_ZERO_FEED_RG:
            feedBias_mV = 0;
            if (pTestHeap->testArgs.resFltAlt.shiftZeroFeed == TRUE) {
                /* use a 33V voc to set the longitudinal point */
                dcfeed[0] = 0xFF;
            }
            if (pTestHeap->lowGain == TRUE) {
                pTestHeap->adcState = RES_FLT_LG_IMT_ZERO_FEED;
            }
            break;

        case RES_FLT_LG_IMT_REVERSE_FEED_RG:
            feedBias_mV = VP886_UNIT_CONVERT(pTestHeap->testArgs.resFltAlt.feedBias, VP886_UNIT_DAC_RING, VP886_UNIT_MV);
            if (pTestHeap->lowGain == TRUE) {
                pTestHeap->adcState = RES_FLT_LG_IMT_REVERSE_FEED;
                feedBias_mV *= -1;
            }
            break;

        case RES_FLT_LG_IMT_NORMAL_FEED:
        case RES_FLT_LG_IMT_ZERO_FEED:
        case RES_FLT_LG_IMT_REVERSE_FEED:

            if (pTestHeap->adcState == RES_FLT_LG_IMT_ZERO_FEED) {
                if (pTestHeap->testArgs.resFltAlt.shiftZeroFeed == TRUE) {
                    /* use a 33V voc to set the longitudinal point */
                    dcfeed[0] = 0xFF;
                }
            }

            /* If a VAB measurement was not taken then we don't need to adjust the feed */
            if (pTestHeap->vabComputed == 0) {
                return TRUE;
            }
            feedBias_mV = VP886_UNIT_CONVERT(2 * pTestHeap->vabComputed, VP886_UNIT_ADC_VAB, VP886_UNIT_MV);
            pTestHeap->vabComputed = 0;
            break;

        default:
            /* No change in feed, so no need to set a timer */
            *nextTimer = 0;
            return TRUE;
    }

    VP_TEST(VpLineCtxType, pLineCtx, ("feedBias_mV before cal %li",feedBias_mV));

    /* Apply calibration factors to the feed bias */
    feedBias_mV *= pCalData->vocSenseNormal.gain;
    feedBias_mV = VpRoundedDivide(feedBias_mV, 1000);

    buffErr = pCalData->ringingBuffer.offset * 1000;
    buffErr = VpRoundedDivide(buffErr, (int32)pCalData->ringingBuffer.gain * 40);
    feedBias_mV -= buffErr;

    feedBias_mV -= pCalData->ringingGenerator.offset;

    feedBias_mV *= 1000;
    feedBias_mV = VpRoundedDivide(feedBias_mV, pCalData->ringingGenerator.gain);

    VP_TEST(VpLineCtxType, pLineCtx, ("feedBias_mV after cal %li",feedBias_mV));

    feedBias = VP886_UNIT_CONVERT(feedBias_mV, VP886_UNIT_MV, VP886_UNIT_DAC_RING);

    VP_TEST(VpLineCtxType, pLineCtx, ("VpResSetFeedLG() adcState %i " \
        "feedBias %i, sigGenABias 0x%04x, dcFeed 0x%02x%02x",
        pTestHeap->adcState, feedBias, feedBias, dcfeed[0], dcfeed[1]));

    /* Set the Metallic Voltage */
    sigGenA[1] = (uint8)((feedBias >> 8) & 0x00FF);
    sigGenA[2] = (uint8)(feedBias & 0x00FF);
    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_SIGAB_WRT, VP886_R_SIGAB_LEN, sigGenA) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpResSetFeedLG() failed VP886_R_SIGAB_WRTT"));
        return FALSE;
    }

    /* Set the Longitudinal Voltage */
    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_DCFEED_WRT, VP886_R_DCFEED_LEN, dcfeed) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpResSetFeedLG() failed VP886_R_DCFEED_WRT"));
        return FALSE;
    }

    return TRUE;
}

/*------------------------------------------------------------------------------
 * VpResStartMeasurementLG()
 * This function is called by the Vp886Test3EleResLG() state machine during
 * the RES_FLT_LG_ADC_SETUP state.
 *
 * This function is used to call the Vp886AdcSetup() depending on the
 * current ac rloop adc state.
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context.
 *  adcState        - current value of the adc setup state.
 *
 * Returns:TRUE if no error, FALSE otherwise
 *  --
 *
 * Result:
 *  The ADC routing is changed
 *----------------------------------------------------------------------------*/
static bool
VpResStartMeasurementLG(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap,
    uint8 adcState)
{
    VpTest3EleResAltResType *pResInput = &(pTestHeap->testArgs).resFltAlt;
    uint16 settlingTime = SADC_SETTLE_FRAMES_2MS;   /* PcmCollect settling time in milliseconds */
    uint16 integrationTime = 0;                     /* PcmCollect integration time in milliseconds */
    Vp886SadcSignalType sadcSigType = VP886_SADC_SEL_NO_CONN;

    VP_TEST(VpLineCtxType, pLineCtx, ("VpResStartMeasurementLG() adcState %d", adcState));

    switch (adcState) {

        case RES_FLT_LG_IMT_NORMAL_FEED:
        case RES_FLT_LG_IMT_REVERSE_FEED:
        case RES_FLT_LG_IMT_ZERO_FEED:
            /*
             * If a REN load has been detected then we need more settle time
             * between measurements because the capacitance on the line prevents
             * the line from settling quickly so the difference between the
             * previous measurement and the current is small causing the
             * the final measureed data to be taken too soon.
             */
            if (pTestHeap->lowGain == FALSE) {
                settlingTime = 50*8; /* 50 ms between measurements */
            }
            sadcSigType = VP886_SADC_SEL_LONG_I;  /* imt->ilg in low gain */
            integrationTime = pResInput->integrateTime;      /* (144)18ms or (800)100ms (see VpResStoreMeasurementLG)*/
            break;

        case RES_FLT_LG_IMT_NORMAL_FEED_RG:
        case RES_FLT_LG_IMT_REVERSE_FEED_RG:
        case RES_FLT_LG_IMT_ZERO_FEED_RG:
        case RES_FLT_LG_ILG_NORMAL_FEED:
        case RES_FLT_LG_ILG_REVERSE_FEED:
        case RES_FLT_LG_ILG_ZERO_FEED:
            sadcSigType = VP886_SADC_SEL_MET_I;   /* ilg->imt in low gain */
            integrationTime = pResInput->integrateTime;      /* (144)18ms or (800)100ms (see VpResStoreMeasurementLG)*/
            break;

        case RES_FLT_LG_VAL_NORMAL_FEED:
        case RES_FLT_LG_VAL_REVERSE_FEED:
        case RES_FLT_LG_VAL_ZERO_FEED:
            sadcSigType = VP886_SADC_SEL_TIP;
            integrationTime = SADC_INTEGRATION_FRAMES_18MS;
            break;

        case RES_FLT_LG_VBL_NORMAL_FEED:
        case RES_FLT_LG_VBL_REVERSE_FEED:
        case RES_FLT_LG_VBL_ZERO_FEED:
            sadcSigType = VP886_SADC_SEL_RING;
            integrationTime = SADC_INTEGRATION_FRAMES_18MS;
            break;

        case RES_FLT_LG_VAB_NORMAL_FEED_RG:
        case RES_FLT_LG_VAB_REVERSE_FEED_RG:
        case RES_FLT_LG_VAB_ZERO_FEED_RG:
            sadcSigType = VP886_SADC_SEL_MET_VDC;
            integrationTime = SADC_INTEGRATION_FRAMES_18MS;
            break;

        default:
            VP_ERROR(VpLineCtxType, pLineCtx, ("VpResStartMeasurementLG(): invalid ADC state"));
            return FALSE;
    }
    VP_TEST(VpLineCtxType, pLineCtx, ("VpResStartMeasurementLG(adc %d): settling = %i, integrationTime = %i",
        adcState, settlingTime, integrationTime));

    /* start the ADC to measure VSAB */
    return Vp886SadcSetup(pLineCtx, VP886_SADC_MATH_SINGLE, VP886_SADC_RATE_FULL,
            sadcSigType,
            VP886_SADC_SEL_NO_CONN,
            VP886_SADC_SEL_NO_CONN,
            VP886_SADC_SEL_NO_CONN,
            VP886_SADC_SEL_NO_CONN,
            settlingTime,           /* not adjustable by the primitive caller */
            integrationTime,        /* not adjustable by the primitive caller */
            VP886_LINE_TEST);
}

/*------------------------------------------------------------------------------
 * VpResStoreMeasurementLG()
 * This function is called by Vp3EleResLGStoreResult() to store the measurement
 * in the device object.
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context.
 *  pTestHeap       - pointer to the test heap.
 *  adcState        - ADC state.
 *
 * Returns:
 *  Measured value.
 *
 * Result:
 *
 *----------------------------------------------------------------------------*/
static bool
VpResStoreMeasurementLG(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap,
    uint8 adcState,
    int16 *pAdcAverage,
    int16 *pAdcMin,
    int16 *pAdcMax)
{
    Vp886LineObjectType *pLineObj =  pLineCtx->pLineObj;
    Vp886DeviceObjectType *pDevObj = pLineCtx->pDevCtx->pDevObj;
    Vp886CmnCalDeviceDataType *pCalData = &pDevObj->calData[pLineObj->channelId].cmn;

    int16 *pResult;
    int16 result_RG;
    Vp886AdcMathDataType mathData;

    int16 offset = pCalData->sadc.offset;
    int16 gain = pCalData->sadc.gain;

    VP_TEST(VpLineCtxType, pLineCtx, ("VpResStoreMeasurementLG() adcState %d", adcState));

    switch (adcState) {

        case RES_FLT_LG_IMT_NORMAL_FEED:
            pResult = &pLineObj->testResults.result.resAlt.imnf;
            VP_TEST(VpLineCtxType, pLineCtx, ("VpResStoreMeasurementLG() ELE3_RES_LG_IMT_NORMAL_FEED imnf"));
            break;

        case RES_FLT_LG_ILG_NORMAL_FEED:
            pResult = &pLineObj->testResults.result.resAlt.ilnf;
            VP_TEST(VpLineCtxType, pLineCtx, ("VpResStoreMeasurementLG() ELE3_RES_LG_ILG_NORMAL_FEED ilnf"));
            break;

        case RES_FLT_LG_VAL_NORMAL_FEED:
            offset = pCalData->tipSense.offset;
            gain = pCalData->tipSense.gain;
            pResult = &pLineObj->testResults.result.resAlt.vanf;
            VP_TEST(VpLineCtxType, pLineCtx, ("VpResStoreMeasurementLG() ELE3_RES_LG_VAL_NORMAL_FEED vanf"));
            break;

        case RES_FLT_LG_VBL_NORMAL_FEED:
            offset = pCalData->ringSense.offset;
            gain = pCalData->ringSense.gain;
            pResult = &pLineObj->testResults.result.resAlt.vbnf;
            VP_TEST(VpLineCtxType, pLineCtx, ("VpResStoreMeasurementLG() ELE3_RES_LG_VBL_NORMAL_FEED vbnf"));
            break;

        case RES_FLT_LG_IMT_ZERO_FEED:
            pResult = &pLineObj->testResults.result.resAlt.imzf;
            VP_TEST(VpLineCtxType, pLineCtx, ("VpResStoreMeasurementLG() ELE3_RES_LG_IMT_ZERO_FEED imzf"));
            break;

        case RES_FLT_LG_ILG_ZERO_FEED:
            pResult = &pLineObj->testResults.result.resAlt.ilzf;
            VP_TEST(VpLineCtxType, pLineCtx, ("VpResStoreMeasurementLG() ELE3_RES_LG_ILG_ZERO_FEED ilzf"));
            break;

        case RES_FLT_LG_VAL_ZERO_FEED:
            offset = pCalData->tipSense.offset;
            gain = pCalData->tipSense.gain;
            pResult = &pLineObj->testResults.result.resAlt.vazf;
            VP_TEST(VpLineCtxType, pLineCtx, ("VpResStoreMeasurementLG() ELE3_RES_LG_VAL_ZERO_FEED vazf"));
            break;

        case RES_FLT_LG_VBL_ZERO_FEED:
            offset = pCalData->ringSense.offset;
            gain = pCalData->ringSense.gain;
            pResult = &pLineObj->testResults.result.resAlt.vbzf;
            VP_TEST(VpLineCtxType, pLineCtx, ("VpResStoreMeasurementLG() ELE3_RES_LG_VBL_ZERO_FEED vbzf"));
            break;

        case RES_FLT_LG_IMT_REVERSE_FEED:
            pResult = &pLineObj->testResults.result.resAlt.imrf;
            VP_TEST(VpLineCtxType, pLineCtx, ("VpResStoreMeasurementLG() ELE3_RES_LG_IMT_REVERSE_FEED imrf"));
            break;

        case RES_FLT_LG_ILG_REVERSE_FEED:
            pResult = &pLineObj->testResults.result.resAlt.ilrf;
            VP_TEST(VpLineCtxType, pLineCtx, ("VpResStoreMeasurementLG() ELE3_RES_LG_ILG_REVERSE_FEED ilrf"));
            break;

        case RES_FLT_LG_VAL_REVERSE_FEED:
            offset = pCalData->tipSense.offset;
            gain = pCalData->tipSense.gain;
            pResult = &pLineObj->testResults.result.resAlt.varf;
            VP_TEST(VpLineCtxType, pLineCtx, ("VpResStoreMeasurementLG() ELE3_RES_LG_VAL_REVERSE_FEED varf"));
            break;

        case RES_FLT_LG_VBL_REVERSE_FEED:
            offset = pCalData->ringSense.offset;
            gain = pCalData->ringSense.gain;
            pResult = &pLineObj->testResults.result.resAlt.vbrf;
            VP_TEST(VpLineCtxType, pLineCtx, ("VpResStoreMeasurementLG() ELE3_RES_LG_VBL_REVERSE_FEED vbrf"));
            break;

        case RES_FLT_LG_VAB_REVERSE_FEED_RG:
        case RES_FLT_LG_VAB_NORMAL_FEED_RG:
        case RES_FLT_LG_VAB_ZERO_FEED_RG:
            offset = pCalData->vabSenseRinging.offset;
            gain = pCalData->vabSenseRinging.gain;
            pResult = &pTestHeap->vabComputed;
            VP_TEST(VpLineCtxType, pLineCtx, ("VpResStoreMeasurementLG() RINGING STATE"));
            break;

        default: /* Others ringing states */
            pResult = &result_RG;
            break;

    }

    /* get the data and generate the event */
    if (Vp886SadcGetMathData(pLineCtx, VP886_SADC_BUF_1, &mathData, offset, gain) == FALSE) {
        Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpResStoreMeasurementLG() calculations failed"));
        return FALSE;
    }

    *pResult = (int16)(-1 * mathData.average);
    *pAdcAverage = *pResult;
    *pAdcMin = (int16)(mathData.minVal);
    *pAdcMax = (int16)(mathData.maxVal);

    VP_TEST(VpLineCtxType, pLineCtx, ("VpResStoreMeasurementLG() offset %d gain %d average %d", offset, gain, *pAdcAverage));

    if (adcState == RES_FLT_LG_IMT_NORMAL_FEED) {
        if ((pTestHeap->settlingCount == 5) && (mathData.rms > IMT_RMS_18MS_TO_100MS_THRESHOLD)) {
            /*
             * If the RMS metalic current in low gain is still high at the 6th imt measurement
             * then there is a foreign voltage on the line and all remaining imt measurements
             * need to integrate over a longer period
             */
            VP_TEST(VpLineCtxType, pLineCtx, ("VpResStoreMeasurementLG() FOREIGN VOLTAGE PRESENT!!!"));
            pTestHeap->testArgs.resFltAlt.integrateTime = SADC_INTEGRATION_FRAMES_100MS;
        }
    }

    return TRUE;
}

/*------------------------------------------------------------------------------
 * VpResNeedMoreSettlingTimeLG()
 * This function is called by Vp3EleResLGStoreResult() to check if the current
 * measurement has settled. Only the adcState in the case statement will be
 * checked.
 *
 * Parameters:
 *  pTestHeap       - pointer to the test heap.
 *  currentValue    - current measured value.
 *  adcState        - ADC state.
 *
 * Returns:
 *  TRUE: the measurement needs more time to settle.
 *  FALSE: the measurement has settled.
 *
 * Result:
 *
 *----------------------------------------------------------------------------*/
static bool
VpResNeedMoreSettlingTimeLG(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap,
    int16 currentValue,
    int16 currentAdcMin,
    int16 currentAdcMax,
    uint8 adcState)
{
    bool settleMore = FALSE;
    int32 diff = (currentAdcMax - currentAdcMin);

    VP_TEST(VpLineCtxType, pLineCtx, ("VpResNeedMoreSettlingTimeLG() adcState %i, cVal %i, pVal %i",
        adcState, currentValue, pTestHeap->previousAvg));

    switch (adcState) {
        case RES_FLT_LG_IMT_NORMAL_FEED_RG:
        case RES_FLT_LG_IMT_ZERO_FEED_RG:
        case RES_FLT_LG_IMT_REVERSE_FEED_RG:
            if (ABS(currentValue - pTestHeap->previousAvg) > SETTLING_ERROR_THRESHOLD_RG) {
                /* imt is not stable, need more time to settle */
                pTestHeap->previousAvg = currentValue;
                pTestHeap->settlingCount++;
                settleMore = TRUE;
            }
            
            /* are we still settling because of an AC voltage? */
            if (settleMore && (pTestHeap->settlingCount > 28) && (diff > 1000)) {
                VP_TEST(VpLineCtxType, pLineCtx, ("VpResNeedMoreSettlingTimeLG() stopped settling due to AC %i %li",
                    pTestHeap->settlingCount, diff));
                settleMore = FALSE;
            }
            break;

        case RES_FLT_LG_IMT_NORMAL_FEED:
        case RES_FLT_LG_IMT_ZERO_FEED:
        case RES_FLT_LG_IMT_REVERSE_FEED:
        case RES_FLT_LG_ILG_NORMAL_FEED:
        case RES_FLT_LG_ILG_ZERO_FEED:
        case RES_FLT_LG_ILG_REVERSE_FEED:
        case RES_FLT_LG_VAL_NORMAL_FEED:
        case RES_FLT_LG_VAL_ZERO_FEED:
        case RES_FLT_LG_VAL_REVERSE_FEED:
            if (ABS(currentValue - pTestHeap->previousAvg) > SETTLING_ERROR_THRESHOLD_LG) {
                /* Measurement is not stable, need more time to settle */
                pTestHeap->previousAvg = currentValue;
                pTestHeap->settlingCount++;
                settleMore = TRUE;
            } else if ((currentValue == VP_INT16_MIN) || (currentValue == VP_INT16_MAX)) {
                /* 
                 * Measurement saturates, need some time to get out of saturation.
                 * Don't let saturated measurements keep us in the loop till the end
                 */
                if (pTestHeap->settlingCount < 10) {
                    pTestHeap->settlingCount++;
                    settleMore = TRUE;
                } else {
                    pTestHeap->previousAvg = 0;
                    VP_TEST(VpLineCtxType, pLineCtx,
                        ("VpResNeedMoreSettlingTimeLG(): Saturated measurement Took too long to settle, reduced accuracy"));
                    settleMore = FALSE;
                }
            }

            /* are we still settling because of an AC voltage? */
            if (settleMore && (pTestHeap->settlingCount > 10) && (diff > 1000)) {
                VP_TEST(VpLineCtxType, pLineCtx, ("VpResNeedMoreSettlingTimeLG() stopped settling due to AC %i %li",
                    pTestHeap->settlingCount, diff));
                settleMore = FALSE;
            }
            break;

        default:
            /*
             * RES_FLT_LG_VAB_REVERSE_FEED_RG:
             * RES_FLT_LG_VAB_NORMAL_FEED_RG:
             * RES_FLT_LG_VAB_ZERO_FEED_RG:
             * The VAB_XYZ_FEED_RG states do not need to settle because the IMT_XYZ_RG
             * states were used to settle the measurments. These single measurements
             * are used by the VpResChangeFeedLG() function to properly set  sig gen A,
             * for the next low gain feed configuration.
             */
            break;
    }

    /* if the measurement is not stable then how long has it been not stable */
    VP_TEST(VpLineCtxType, pLineCtx, ("VpResNeedMoreSettlingTimeLG() settlingCount %i :settleMore %i",
            pTestHeap->settlingCount, settleMore));

    if (settleMore) {
        uint16 maxCount = 0;

        if ((adcState == RES_FLT_LG_IMT_NORMAL_FEED_RG) ||
            (adcState == RES_FLT_LG_IMT_ZERO_FEED_RG) ||
            (adcState == RES_FLT_LG_IMT_REVERSE_FEED_RG)) {
            maxCount = MAX_ITERATION_RG_COUNT;
        } else {
            maxCount = MAX_ITERATION_LG_COUNT;
        }

        if (pTestHeap->settlingCount < maxCount) {
            VP_TEST(VpLineCtxType, pLineCtx,
                ("VpResNeedMoreSettlingTimeLG(): Another measurement is required"));
        } else {
            pTestHeap->previousAvg = 0;
            VP_TEST(VpLineCtxType, pLineCtx,
                ("VpResNeedMoreSettlingTimeLG(): Took too long to settle, reduced accuracy"));
            settleMore = FALSE;
        }
    }

    if (settleMore == FALSE) {
        pTestHeap->previousAvg = 0;

        /*
         * If we settle fast enough then there is no large REN between tip & ring. So
         * there is no need to switch back and force between low gain and ringing.
         * So we stay in low gain.
         */
        if ((adcState == RES_FLT_LG_IMT_NORMAL_FEED_RG) &&
            ((pTestHeap->settlingCount < IMT_LG_NF_RG_SETTLE_THRESHOLD) ||
             (pTestHeap->testArgs.resFltAlt.shiftZeroFeed == FALSE))) {
                VP_TEST(VpLineCtxType, pLineCtx,
                    ("VpResNeedMoreSettlingTimeLG(): Fast Settle no need for RG states"));
            pTestHeap->lowGain = TRUE;
        }
    }
    return settleMore;
}

/*------------------------------------------------------------------------------
 * VpResJumpToHighGain()
 * Decides if we need to jump over to the high gain configuration if:
 *   - The metallic current in the active state is too high
 *   - The metallic current is approaching the devices useful range in low gain
 *   - The longitudinal current is approaching the devices useful range in low gain
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context.
 *  adcState        - ADC state.
 *
 * Returns:
 *  TRUE: need to abort this primitive.
 *  FALSE:
 *
 * Result:
 *
 *----------------------------------------------------------------------------*/
static bool
VpResJumpToHighGain(
    VpLineCtxType *pLineCtx,
    uint8 adcState)
{
    Vp886LineObjectType *pLineObj =  pLineCtx->pLineObj;
    bool result = FALSE;
    int16 measurement = 0;
    int16 threshold = 0;

    switch (adcState) {
        case RES_FLT_LG_IMT_NORMAL_FEED:
            measurement = pLineObj->testResults.result.resAlt.imnf;
            threshold = IMT_LG_TO_HG_THRESHOLD;
            break;

        case RES_FLT_LG_IMT_ZERO_FEED:
            measurement = pLineObj->testResults.result.resAlt.imzf;
            threshold = IMT_LG_TO_HG_THRESHOLD;
            break;

        case RES_FLT_LG_IMT_REVERSE_FEED:
            measurement = pLineObj->testResults.result.resAlt.imrf;
            threshold = IMT_LG_TO_HG_THRESHOLD;
            break;

        case RES_FLT_LG_ILG_NORMAL_FEED:
            measurement = pLineObj->testResults.result.resAlt.ilnf;
            threshold = ILG_LG_TO_HG_THRESHOLD;
            break;

        case RES_FLT_LG_ILG_ZERO_FEED:
            measurement = pLineObj->testResults.result.resAlt.ilzf;
            threshold = ILG_LG_TO_HG_THRESHOLD;
            break;

        case RES_FLT_LG_ILG_REVERSE_FEED:
            measurement = pLineObj->testResults.result.resAlt.ilrf;
            threshold = ILG_LG_TO_HG_THRESHOLD;
            break;

        case RES_FLT_LG_VBL_NORMAL_FEED:
            /* swapped on purpose */
            threshold = pLineObj->testResults.result.resAlt.vbnf -
                pLineObj->testResults.result.resAlt.vanf;
            measurement = VAB_LG_TO_HG_THRESHOLD;
            break;

        default:
            return FALSE;
    }

    if (ABS(measurement) > ABS(threshold)) {
        VP_TEST(VpLineCtxType, pLineCtx, ("Vp3EleResLGCheckHighGain() jumping to high gain adcState %i: %i > %i",
            adcState, ABS(measurement), ABS(threshold)));
        result = TRUE;
    }

    return result;
}

/*------------------------------------------------------------------------------
 * VpResSetupHG()
 * This function is called to setup the line to take measurements in
 * "high gain" really high gain is just normal feed. However its being
 * compared to low gain hence "high gain"
 *
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context.
 *  pTestHeap       - pointer to the test heap.
 *  deviceId        - device ID
 *
 * Returns:TRUE if no error, FALSE otherwise
 * --
 * Result:
 *
 *----------------------------------------------------------------------------*/
static bool
VpResSetupHG(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap)
{
    VpTest3EleResAltResType *pResInput = &(pTestHeap->testArgs).resFltAlt;
    Vp886LineObjectType *pLineObj =  pLineCtx->pLineObj;
    Vp886DeviceObjectType *pDevObj = pLineCtx->pDevCtx->pDevObj;

    uint8 switchRegParam[VP886_R_SWPARAM_LEN] = {0x44, 0x04, 0x04}; /* battery 25V fixed */
    uint8 sigGenCtrl = VP886_R_SIGCTRL_EN_BIAS;
    uint8 dcfeed[VP886_R_DCFEED_LEN] = {0x22, 0x08};
    uint8 lpSuper[VP886_R_LOOPSUP_LEN];

    VP_TEST(VpLineCtxType, pLineCtx, ("VpResSetupHG()"));

    if (Vp886CalmodeTestSetup(pLineCtx, pTestHeap) == FALSE) {
        return FALSE;
    }

    /* Masking the hook detection by maxing out TSH */
    if (VpSlacRegRead(NULL, pLineCtx, VP886_R_LOOPSUP_RD, VP886_R_LOOPSUP_LEN, pLineObj->registers.loopSup) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpResSetupHG failed VP886_R_LOOPSUP_RD"));
        return FALSE;
    }

    lpSuper[0] = (pLineObj->registers.loopSup[0]) | 0x07;
    lpSuper[1] = 0xFF;
    lpSuper[2] = pLineObj->registers.loopSup[2];
    lpSuper[3] = pLineObj->registers.loopSup[3];
    lpSuper[4] = pLineObj->registers.loopSup[4];

    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_LOOPSUP_WRT, VP886_R_LOOPSUP_LEN, lpSuper) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpResHGSetup failed VP886_R_LOOPSUP_WRT"));
        return FALSE;
    }

    if (VP886_IS_TRACKER(pDevObj)) {
        if (VpSlacRegRead(NULL, pLineCtx, VP886_R_SWPARAM_RD, VP886_R_SWPARAM_LEN, pLineObj->registers.swParam) == FALSE) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("VpResHGSetup failed VP886_R_SWPARAM_RD"));
            return FALSE;
        }

        switchRegParam[0] = (pLineObj->registers.swParam[0] & 0xA0) | switchRegParam[0];  /*RTM=1, SWF[4:0] = 00100*/
        switchRegParam[1] = (pLineObj->registers.swParam[1] & 0xE0) | switchRegParam[1];  /*SWRV = 00100; */
        switchRegParam[2] = (pLineObj->registers.swParam[2] & 0xE0) | switchRegParam[2];  /*SWLV = 00100*/

        if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_SWPARAM_WRT, VP886_R_SWPARAM_LEN, switchRegParam) == FALSE) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("VpResHGSetup() failed VP886_R_SWPARAM_WRT"));
            return FALSE;
        }
    }

    pLineObj->registers.icr1[0] = 0xFF;
    pLineObj->registers.icr1[1] = 0xA8;
    pLineObj->registers.icr1[2] = 0xFF;
    pLineObj->registers.icr1[3] = 0x4A;

    if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_SIGCTRL_WRT, VP886_R_SIGCTRL_LEN, &sigGenCtrl) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpResHGSetup() failed VP886_R_SIGCTRL_WRT"));
        return FALSE;
    }
    if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_DCFEED_WRT, VP886_R_DCFEED_LEN, dcfeed) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpResHGSetup() failed VP886_R_DCFEED_WRT"));
        return FALSE;
    }
    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR1_WRT, VP886_R_ICR1_LEN, pLineObj->registers.icr1) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpResHGSetup() failed VP886_R_ICR1_WRT"));
        return FALSE;
    }

    pResInput->integrateTime = SADC_INTEGRATION_FRAMES_18MS; /* 18ms default intergration time */
    return TRUE;
}

/*------------------------------------------------------------------------------
 * VpResChangeFeedHG()
 * This function is used to change the the feed (VOC +6V -> 0V -> -6V).
 * It is also used to initialize some variables
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context.
 *  pTestHeap       - pointer to the test heap.
 *  adcState        - current mesaurement type value in high gain
 *  *feedSettleInMs - settling time in ms
 *
 * Returns:  TRUE if no error, FALSE otherwise
 *
 * Result:
 * If the incoming adc state is not a feed condition change then nothing
 * happens and the next timer duration is set to 0. Other wise the device
 * is changed according to the feed state and the time argument is set to
 * state change settling time.
 *----------------------------------------------------------------------------*/
static bool
VpResChangeFeedHG(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap,
    uint8 adcState,
    uint8 *feedSettleInMs)
{
    Vp886LineObjectType *pLineObj =  pLineCtx->pLineObj;
    Vp886DeviceObjectType *pDevObj = pLineCtx->pDevCtx->pDevObj;

    uint8 sigGenA[VP886_R_SIGAB_LEN] =
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8 icr2[VP886_R_ICR2_LEN]= {0x00, 0x00, 0x10, 0x10};     /* force no track */
    uint8 icr3[VP886_R_ICR3_LEN]= {0x00, 0x00, 0x00, 0x00};
    uint8 icr4[VP886_R_ICR4_LEN]= {0x00, 0x00, 0x00, 0x00};

    VP_TEST(VpLineCtxType, pLineCtx, ("VpResChangeFeedHG() adcState %i", adcState));

    /* Initialize the 'previous' measurement used in the settling algorithm */
    pTestHeap->previousAvg = 0;

    /* Initialize the settling counter and the ADC retry counter */
    pTestHeap->settlingCount = 0;

    if (adcState == RES_FLT_HG_IMT_ZERO_FEED) {
        icr2[0] = 0x20;

        if (VP886_IS_ABS(pDevObj)) {
            /* Force DAC driver to longitudinal loop */
            icr3[0] = 0x08;
            icr3[1] = 0x08;
            /*
             * Force Half Bat Un-Clamp off. Enable Y long bat sense and Disable Z
             */
            icr3[2] = 0x68;
            icr3[3] = 0x08;
        } else {
            /* Force DAC driver to longitudinal loop */
            icr3[0] = 0x08;
            icr3[1] = 0x08;
            /*
             * We have forced the floor voltage (-25V) in VpResSetupHG(),
             * therefore we should never hit the clamps so forcing the half
             * battery unclamp is redundant but safe.
             */
            icr3[2] = 0x40;
            icr3[3] = 0x40;
        }

        /* 14.475V bias */
        sigGenA[1] = 0x0C;
        icr4[0] = pLineObj->registers.icr4[0];
        icr4[1] = pLineObj->registers.icr4[1];
        icr4[2] = pLineObj->registers.icr4[2] | 0x05;
        icr4[3] = pLineObj->registers.icr4[3] | 0x05;
    } else {
        if (VP886_IS_ABS(pDevObj)) {
            /* Force DAC driver to longitudinal loop */
            icr3[0] = 0x08;
            icr3[1] = 0x08;
            /* Force Enable Y long bat sense and disable Z */
            icr3[2] = 0x28;
            icr3[3] = 0x08;
        }

        icr4[0] = pLineObj->registers.icr4[0];
        icr4[1] = pLineObj->registers.icr4[1];
        icr4[2] = pLineObj->registers.icr4[2] & ~0x01;
        icr4[3] = pLineObj->registers.icr4[3] & ~0x01;
    }

    switch (adcState) {
        case RES_FLT_HG_IMT_NORMAL_FEED:
        case RES_FLT_HG_IMT_ZERO_FEED:
        case RES_FLT_HG_IMT_REVERSE_FEED: {
            /* High gain mode configuration */
            pLineObj->registers.sysState[0] = VP886_R_STATE_CODEC | VP886_R_STATE_SS_ACTIVE;

            if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_SIGAB_WRT, VP886_R_SIGAB_LEN, sigGenA) == FALSE) {
                VP_ERROR(VpLineCtxType, pLineCtx, ("VpResHGChangeFeed() failed VP886_R_SIGAB_WRTT"));
                return FALSE;
            }
            if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR2_WRT, VP886_R_ICR2_LEN, icr2) == FALSE) {
                VP_ERROR(VpLineCtxType, pLineCtx, ("VpResHGChangeFeed() failed VP886_R_ICR2_WRT"));
                return FALSE;
            }
            if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR3_WRT, VP886_R_ICR3_LEN, icr3) == FALSE) {
                VP_ERROR(VpLineCtxType, pLineCtx, ("VpResHGChangeFeed() failed VP886_R_ICR3_WRT"));
                return FALSE;
            }
            if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR4_WRT, VP886_R_ICR4_LEN, icr4) == FALSE) {
                VP_ERROR(VpLineCtxType, pLineCtx, ("VpResHGChangeFeed() failed VP886_R_ICR4_WRT"));
                return FALSE;
            }

            if (adcState ==RES_FLT_HG_IMT_REVERSE_FEED) {
                /* Set the line state to active-reverse */
                pLineObj->registers.sysState[0] = VP886_R_STATE_CODEC | VP886_R_STATE_POL | VP886_R_STATE_SS_ACTIVE;
            }

            if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_STATE_WRT, VP886_R_STATE_LEN, pLineObj->registers.sysState) == FALSE) {
                VP_ERROR(VpLineCtxType, pLineCtx, ("VpResHGChangeFeed failed VP886_R_STATE_WRT"));
                return FALSE;
            }
            *feedSettleInMs = FEED_CHANGE_TIME_CONSTANT;
            break;
        }
        default:
            /* No change in feed, so no need to set a timer */
            *feedSettleInMs = 0;
            break;
    }
    return TRUE;
}


/*------------------------------------------------------------------------------
 * VpResStartMeasurementHG()
 * This function configures the SADC based on the current High Gain adc State .
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context.
 *  adcState        - current value of the adc setup state.
 *
 * Returns:
 *   TRUE if no error,
 *   FALSE otherwise
 *
 * Result:
 *  The SADC routing is changed
 *----------------------------------------------------------------------------*/
static bool
VpResStartMeasurementHG(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap,
    uint8 adcState)
{
    VpTest3EleResAltResType *pResInput = &(pTestHeap->testArgs).resFltAlt;
    Vp886SadcSignalType sadcSigType = VP886_SADC_SEL_NO_CONN;

    VP_TEST(VpLineCtxType, pLineCtx, ("VpResStartMeasurementHG() adcState %d", adcState));
    switch (adcState) {
        case RES_FLT_HG_IMT_NORMAL_FEED:
        case RES_FLT_HG_IMT_REVERSE_FEED:
        case RES_FLT_HG_IMT_ZERO_FEED:
            sadcSigType = VP886_SADC_SEL_MET_I;
            break;

        case RES_FLT_HG_ILG_NORMAL_FEED:
        case RES_FLT_HG_ILG_REVERSE_FEED:
        case RES_FLT_HG_ILG_ZERO_FEED:
            sadcSigType = VP886_SADC_SEL_LONG_I;
            break;

        case RES_FLT_HG_VAH_NORMAL_FEED:
        case RES_FLT_HG_VAH_REVERSE_FEED:
        case RES_FLT_HG_VAH_ZERO_FEED:
            sadcSigType = VP886_SADC_SEL_TIP;
            break;

        case RES_FLT_HG_VBH_NORMAL_FEED:
        case RES_FLT_HG_VBH_REVERSE_FEED:
        case RES_FLT_HG_VBH_ZERO_FEED:
            sadcSigType = VP886_SADC_SEL_RING;
            break;

        default:
            VP_ERROR(VpLineCtxType, pLineCtx, ("VpResHGStartMeasurement(): invalid ADC state"));
            return FALSE;
    }
    /* start the ADC to measure SSAB */
    return Vp886SadcSetup(pLineCtx, VP886_SADC_MATH_SINGLE, VP886_SADC_RATE_FULL,
            sadcSigType,
            VP886_SADC_SEL_NO_CONN,
            VP886_SADC_SEL_NO_CONN,
            VP886_SADC_SEL_NO_CONN,
            VP886_SADC_SEL_NO_CONN,
            SADC_SETTLE_FRAMES_2MS,   /* not adjustable by the primitive caller */
            pResInput->integrateTime, /* will be 18 (default) or 100 (see VpResStoreMeasurementHG())*/
            VP886_LINE_TEST);
}

/*------------------------------------------------------------------------------
 * VpResHGStoreMeasurement()
 * This function is called by Vp3EleResHGStoreResult() to store the measurement
 * in the device object.
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context.
 *  pTestHeap       - pointer to the test heap.
 *  adcState        - ADC state.
 *
 * Returns:
 *  Measured value.
 *
 * Result:
 *
 *----------------------------------------------------------------------------*/
static int16
VpResStoreMeasurementHG(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap,
    uint8 adcState,
    int16 *pAdcAverage)
{
    Vp886LineObjectType *pLineObj =  pLineCtx->pLineObj;
    Vp886DeviceObjectType *pDevObj = pLineCtx->pDevCtx->pDevObj;
    Vp886CmnCalDeviceDataType *pCalData = &pDevObj->calData[pLineObj->channelId].cmn;
    Vp886AdcMathDataType mathData;

    int16 *pResult;
    int16 offset = pCalData->sadc.offset;
    int16 gain = pCalData->sadc.gain;
    int32 adcSign =-1;

    VP_TEST(VpLineCtxType, pLineCtx, ("VpResStoreMeasurementHG() adcState %d", adcState));

    switch (adcState) {
        case RES_FLT_HG_IMT_NORMAL_FEED:
            pResult = &pLineObj->testResults.result.resAlt.imnf;
            break;

        case RES_FLT_HG_ILG_NORMAL_FEED:
            pResult = &pLineObj->testResults.result.resAlt.ilnf;
            adcSign =-1;
            break;

        case RES_FLT_HG_VAH_NORMAL_FEED:
            offset = pCalData->tipSense.offset;
            gain = pCalData->tipSense.gain;
            pResult = &pLineObj->testResults.result.resAlt.vanf;
            break;

        case RES_FLT_HG_VBH_NORMAL_FEED:
            offset = pCalData->ringSense.offset;
            gain = pCalData->ringSense.gain;
            pResult = &pLineObj->testResults.result.resAlt.vbnf;
            break;

        case RES_FLT_HG_IMT_ZERO_FEED:
            pResult = &pLineObj->testResults.result.resAlt.imzf;
            break;

        case RES_FLT_HG_ILG_ZERO_FEED:
            pResult = &pLineObj->testResults.result.resAlt.ilzf;
            adcSign =-1;
            break;

        case RES_FLT_HG_VAH_ZERO_FEED:
            offset = pCalData->tipSense.offset;
            gain = pCalData->tipSense.gain;
            pResult = &pLineObj->testResults.result.resAlt.vazf;
            break;

        case RES_FLT_HG_VBH_ZERO_FEED:
            offset = pCalData->ringSense.offset;
            gain = pCalData->ringSense.gain;
            pResult = &pLineObj->testResults.result.resAlt.vbzf;
            break;

        case RES_FLT_HG_IMT_REVERSE_FEED:
            pResult = &pLineObj->testResults.result.resAlt.imrf;
            break;

        case RES_FLT_HG_ILG_REVERSE_FEED:
            pResult = &pLineObj->testResults.result.resAlt.ilrf;
            break;

        case RES_FLT_HG_VAH_REVERSE_FEED:
            offset = pCalData->tipSense.offset;
            gain = pCalData->tipSense.gain;
            pResult = &pLineObj->testResults.result.resAlt.varf;
            break;

        case RES_FLT_HG_VBH_REVERSE_FEED:
            offset = pCalData->ringSense.offset;
            gain = pCalData->ringSense.gain;
            pResult = &pLineObj->testResults.result.resAlt.vbrf;
            break;

        default:
            /* invalid state.. should never get here */
            return FALSE;

    }

    /* get the data and generate the event */
    if (Vp886SadcGetMathData(pLineCtx, VP886_SADC_BUF_1, &mathData, offset, gain) == FALSE) {
        Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886SadcGetMathData calculations failed"));
        return FALSE;
    }

    *pResult = (int16)(adcSign * mathData.average);
    *pAdcAverage = *pResult;

    VP_TEST(VpLineCtxType, pLineCtx, ("VpResStoreMeasurementHG offset %d gain %d average %d", offset, gain, *pAdcAverage));

    if (adcState == RES_FLT_HG_IMT_NORMAL_FEED ||
        adcState == RES_FLT_HG_IMT_ZERO_FEED ||
        adcState == RES_FLT_HG_IMT_REVERSE_FEED) {

        if ((pTestHeap->settlingCount == 5) && (mathData.rms > IMT_RMS_18MS_TO_100MS_THRESHOLD)) {
            /*
             * If the RMS metalic current is still high at the 6th imt normal feed measurement
             * then there is a foreign voltage on the line and all remaining imt measurements
             * need to integrate over a longer period
             */
            VP_TEST(VpLineCtxType, pLineCtx, ("VpResStoreMeasurementHG() FOREIGN VOLTAGE PRESENT!!!"));
            pTestHeap->testArgs.resFltAlt.integrateTime = SADC_INTEGRATION_FRAMES_100MS;
        }
    }

    return TRUE;
}

static bool
VpResNeedMoreSettlingTimeHG(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap,
    int16 currentValue,
    uint8 adcState)
{
    bool settleMore = FALSE;

    VP_TEST(VpLineCtxType, pLineCtx, ("VpResNeedMoreSettlingTimeHG() adcState %i, cVal %i, pVal %i, loopCnt %i",
        adcState, currentValue, pTestHeap->previousAvg, pTestHeap->settlingCount));

    switch (adcState) {
        case RES_FLT_HG_IMT_NORMAL_FEED:
        case RES_FLT_HG_IMT_ZERO_FEED:
        case RES_FLT_HG_IMT_REVERSE_FEED:

            if (ABS(currentValue - pTestHeap->previousAvg) > SETTLING_ERROR_THRESHOLD_HG) {
                /* Measurement is not stable, need more time to settle */
                pTestHeap->previousAvg = currentValue;
                pTestHeap->settlingCount++;
                settleMore = TRUE;
            }
            break;

        case RES_FLT_HG_VAH_NORMAL_FEED:
        case RES_FLT_HG_VAH_ZERO_FEED:
        case RES_FLT_HG_VAH_REVERSE_FEED:
            if (ABS(currentValue - pTestHeap->previousAvg) > SETTLING_ERROR_THRESHOLD_HG) {
                /* Measurement is not stable, need more time to settle */
                pTestHeap->previousAvg = currentValue;
                pTestHeap->settlingCount++;
                settleMore = TRUE;
            }
            break;

        default:
            break;
    }

    if (settleMore) {
        uint16 maxCount = 0;

        if (pTestHeap->testArgs.resFltAlt.integrateTime == SADC_INTEGRATION_FRAMES_100MS) {
            maxCount = MAX_ITERATION_HG_100MS_COUNT;
        } else {
            maxCount = MAX_ITERATION_HG_18MS_COUNT;
        }

        if (pTestHeap->settlingCount < maxCount) {
            VP_TEST(VpLineCtxType, pLineCtx,
                ("VpResNeedMoreSettlingTimeHG(): Another measurement is required"));
        } else {
            pTestHeap->previousAvg = 0;
            VP_TEST(VpLineCtxType, pLineCtx,
                ("VpResNeedMoreSettlingTimeHG(): Took too long to settle, reduced accuracy"));
            settleMore = FALSE;
        }
    } else {
        pTestHeap->previousAvg = 0;
        VP_TEST(VpLineCtxType, pLineCtx,
            ("VpResNeedMoreSettlingTimeHG(): measurement has settled"));
    }

    return settleMore;
}


static bool
VpResIsLStPresent(
    VpLineCtxType *pLineCtx,
    uint8 adcState)
{
    Vp886LineObjectType *pLineObj =  pLineCtx->pLineObj;

    if (pLineObj->testResults.result.resAlt.highGain) {

        if (adcState == RES_FLT_HG_ILG_ZERO_FEED) {

            int16 hgMinIlg = (int16)VP886_UNIT_CONVERT(
                    (2 * -5000), VP886_UNIT_UA, VP886_UNIT_ADC_ILG);

            int16 hgMaxIlg = (int16)VP886_UNIT_CONVERT(
                    (2 * 32000), VP886_UNIT_UA, VP886_UNIT_ADC_ILG);

            VP_TEST(VpLineCtxType, pLineCtx, ("VpResIsLStPresent() hgMinIlg %i, hgMaxIlg %i, ilCurrent %i", 
                hgMinIlg, hgMaxIlg, pLineObj->testResults.result.resAlt.ilzf));

            /* look for a foreign current source i.e. "negative current" < -5mA */
            if (hgMinIlg >= pLineObj->testResults.result.resAlt.ilzf) {
                /* special code for upper level application*/
                VP_TEST(VpLineCtxType, pLineCtx, ("VpResIsLStPresent() foreign current found"));
                pLineObj->testResults.result.resAlt.ilzf = VP_INT16_MIN;
                pLineObj->testResults.result.resAlt.ilnf = VP_INT16_MIN;
                pLineObj->testResults.result.resAlt.ilrf = VP_INT16_MIN;
                return TRUE;
            }

            /* look for a hard short to ground i.e. > 32mA */
            if (hgMaxIlg <= pLineObj->testResults.result.resAlt.ilzf) {
                /* Force max current and let ring/tip open states figure out ilnf ilrf */
                VP_TEST(VpLineCtxType, pLineCtx, ("VpResIsLStPresent() short detected"));
                pLineObj->testResults.result.resAlt.ilzf = VP_INT16_MAX;
                return TRUE;
            }

            VP_TEST(VpLineCtxType, pLineCtx, ("VpResIsLStPresent() = FALSE"));
        }
    } else {
        /* At the moment we do not need to check for long short in low gain mode */
    }

    return FALSE;
}

static bool
VpResChangeFeedLS(
    VpLineCtxType *pLineCtx,
    uint8 adcState,
    uint32 *pDebounce)
{


    Vp886LineObjectType *pLineObj =  pLineCtx->pLineObj;

#ifdef USE_VOCCOR_FOR_LS_TEST
    uint8 nrmCal[VP886_R_NORMCAL_LEN];

    /* read the normal cal register */
    VpSlacRegRead(NULL, pLineCtx, VP886_R_NORMCAL_RD, VP886_R_NORMCAL_LEN, nrmCal);
#endif

    VP_TEST(VpLineCtxType, pLineCtx, ("VpResChangeFeedLS() adcState %d\n", adcState));

    if (adcState == RES_FLT_LONG_SHRT_TIP_OPEN) {
        uint8 dcfeed[VP886_R_DCFEED_LEN] = {0x22, 0x08};

        /* enable tip open to check for ring to ground short */
        pLineObj->registers.sysState[0] = VP886_R_STATE_SS_TIPOPEN;
        if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_STATE_WRT, VP886_R_STATE_LEN, pLineObj->registers.sysState) == FALSE) {
            return FALSE;
        }
        pLineObj->registers.icr3[2] |= VP886_R_ICR3_LONG_ON;
        pLineObj->registers.icr3[3] &= ~VP886_R_ICR3_LONG_ON;
        if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR3_WRT, VP886_R_ICR3_LEN, pLineObj->registers.icr3) == FALSE) {
            return FALSE;
        }

        /* reduce the VOC value in this state and raise the switch hook threshold
           to prevent a tip to ring resistance from setting the hook bit*/
        if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_DCFEED_WRT, VP886_R_DCFEED_LEN, dcfeed) == FALSE) {
            return FALSE;
        }

        pLineObj->registers.loopSup[0] |= VP886_R_LOOPSUP_HOOK_THRESH;
        if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_LOOPSUP_WRT, VP886_R_LOOPSUP_LEN, pLineObj->registers.loopSup) == FALSE) {
            return FALSE;
        }
#ifdef USE_VOCCOR_FOR_LS_TEST
        /* set VOCCOR to -5V */
        nrmCal[1] |= VP886_R_NORMCAL_VOCCOR;
        if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_NORMCAL_WRT, VP886_R_NORMCAL_LEN, nrmCal) == FALSE) {
            return FALSE;
        }
#endif
    } else {
        /* force the VOC calibration value to +5V to give us the lowest possible VOC in ring open */


        /* disable the long_on control in ring open to prevent t/r resistance down to ~1kOhm from causing off-hook */
        pLineObj->registers.icr3[2] &= ~VP886_R_ICR3_LONG_ON;
        if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR3_WRT, VP886_R_ICR3_LEN, pLineObj->registers.icr3) == FALSE) {
            return FALSE;
        }

        /* enable ring open to check for tip to ground short */
        pLineObj->registers.sysState[0] = VP886_R_STATE_SS_RINGOPEN;
        if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_STATE_WRT, VP886_R_STATE_LEN, pLineObj->registers.sysState) == FALSE) {
            return FALSE;
        }
#ifdef USE_VOCCOR_FOR_LS_TEST
        /* set VOCCOR to +5V */
        nrmCal[1] &= ~VP886_R_NORMCAL_VOCCOR;
        nrmCal[1] |= 0x70;
        if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_NORMCAL_WRT, VP886_R_NORMCAL_LEN, nrmCal) == FALSE) {
            return FALSE;
        }
#endif
    }

    /* return the device debuounce time */
    *pDebounce = 2 * (pLineObj->registers.loopSup[1] & VP886_R_LOOPSUP_HOOK_DBNC);

    return TRUE;
}

static bool
VpResCheckForGKeyLS(
    VpLineCtxType *pLineCtx,
    uint8 adcState)
{
    Vp886LineObjectType *pLineObj =  pLineCtx->pLineObj;
    Vp886DeviceObjectType *pDevObj = pLineCtx->pDevCtx->pDevObj;

    VP_TEST(VpLineCtxType, pLineCtx, ("VpResCheckForGKeyLS() adcState %d", adcState));

    if (pDevObj->registers.sigreg[pLineObj->channelId] & VP886_R_SIGREG_HOOK) {
        /* Hook bit indicates that the ground fault is still present */

        if (adcState == RES_FLT_LONG_SHRT_TIP_OPEN) {
            /* special indication in normal feed value means gkey is on ring */
            pLineObj->testResults.result.resAlt.ilnf = VP_INT16_MAX;
        } else {
            /* special indication in reverse feed value means gkey is on tip */
            pLineObj->testResults.result.resAlt.ilrf = VP_INT16_MAX;
        }
    }

    return TRUE;
}


/*------------------------------------------------------------------------------
 * VpResRestore()
 * This function is called after the measusrment is completed to restore
 * the registers that were
 * modified during the test to their original value.
 *
 *
 * Parameters:
 *  pLineCtx         - pointer to the line context.
 *  pTestHeap        - Pointer to the stored data in the scratch pad
 *
 * Returns:
 *         TRUE if no error, FALSE otherwise
 * --
 * Result:
 *  -restore the device registers that are modified during the test to their
 *     pre-test value
 *----------------------------------------------------------------------------*/
static bool
VpResRestore(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap)
{
    /* Restore registers */
    /* Must take the slic out of low gain state before restoring */
    /* registers otherwise risk huge transients. */

    Vp886DeviceObjectType *pDevObj = pLineCtx->pDevCtx->pDevObj;
    Vp886LineObjectType *pLineObj =  pLineCtx->pLineObj;

    pLineObj->registers.sysState[0] = pTestHeap->scratch.sysState[0];

    /*restore loop supervision*/
    if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_LOOPSUP_WRT, VP886_R_LOOPSUP_LEN, pTestHeap->scratch.loopSup) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpResRestore() failed VP886_R_LOOPSUP_WRT"));
        return FALSE;
    }
    /*restore dcfeed*/
    if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_DCFEED_WRT, VP886_R_DCFEED_LEN, pTestHeap->scratch.dcFeed) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpResRestore() failed VP886_R_DCFEED_WRT"));
        return FALSE;
    }
#ifdef USE_VOCCOR_FOR_LS_TEST
    /* restore the VOCCOR */
    if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_NORMCAL_WRT, VP886_R_NORMCAL_LEN, pTestHeap->scratch.normCal) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpResRestore() failed VP886_R_NORMCAL_WRT"));
        return FALSE;
    }
#endif
    if (VP886_IS_TRACKER(pDevObj)) {
        if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_SWPARAM_WRT, VP886_R_SWPARAM_LEN, pLineObj->registers.swParam) == FALSE) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("VpResRestore() failed VP886_R_SWPARAM_WRT"));
            return FALSE;
        }
    }
    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR1_WRT, VP886_R_ICR1_LEN, pTestHeap->scratch.icr1) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpResRestore() failed VP886_R_ICR1_WRT"));
        return FALSE;
    }
    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR2_WRT, VP886_R_ICR2_LEN, pTestHeap->scratch.icr2) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpResRestore() failed VP886_R_ICR2_WRT"));
        return FALSE;
    }
    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR3_WRT, VP886_R_ICR3_LEN, pTestHeap->scratch.icr3) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpResRestore() failed VP886_R_ICR3_WRT"));
        return FALSE;
    }
    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR4_WRT, VP886_R_ICR4_LEN, pTestHeap->scratch.icr4) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpResRestore() failed VP886_R_ICR4_WRT"));
        return FALSE;
    }

    return TRUE;
}

#endif /* VP886_INCLUDE_TESTLINE_CODE */
#endif /* VP_CC_886_SERIES */
