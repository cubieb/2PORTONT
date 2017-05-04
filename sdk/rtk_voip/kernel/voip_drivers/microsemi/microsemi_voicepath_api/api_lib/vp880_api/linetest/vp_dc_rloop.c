/** file vp_dc_rloop.c
 *
 *  This file contains the dc rloop primitive algorithm
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 9115 $
 * $LastChangedDate: 2011-11-15 15:25:17 -0600 (Tue, 15 Nov 2011) $
 */

#include "vp_api_cfg.h"

#if defined (VP_CC_880_SERIES)
#if defined (VP880_INCLUDE_TESTLINE_CODE)


/* Project Includes */
#include "vp_api_types.h"
#include "sys_service.h"
#include "vp_hal.h"
#include "vp_api_int.h"
#include "vp880_api.h"
#include "vp880_api_int.h"
#include "vp_api_testline_int.h"

#define TARGET_I_SETTLE_TIME 100 /*ms*/
typedef enum
{
    COMMON_TEST_SETUP           = VP880_TESTLINE_GLB_STRT_STATE,
    DC_RLOOP_SETUP              = 5,
    DC_RLOOP_SET_CURRENT        = 10,
    DC_RLOOP_ADC_SETUP_ILG      = 15,
    DC_RLOOP_COLLECT_ILG_DATA   = 20,
    DC_RLOOP_GET_ILG_RESULT     = 25,
    DC_RLOOP_ADC_SETUP_VAB      = 30,
    DC_RLOOP_COLLECT_VAB_DATA   = 35,
    DC_RLOOP_GET_VAB_RESULT     = 40,
    DC_RLOOP_REMOVE_CURRENT   = 45,
    DC_RLOOP_END                = VP880_TESTLINE_GLB_END_STATE,
    DC_RLOOP_QUIT               = VP880_TESTLINE_GLB_QUIT_STATE,
    DC_RLOOP_ENUM_SIZE          = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
}  vpOpenVdcTestStateTypes;

static VpStatusType
VpDcRloopArgCheck(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    uint8 *pAdcState,
    VpTestIdType testId);

static void
VpDcRloopSetup(
    Vp880LineObjectType *pLineObj,
    Vp880TestHeapType *pTestHeap,
    VpDeviceIdType deviceId);

static bool
VpSetTargetCurrent (
    VpLineCtxType *pLineCtx,
    Vp880TestHeapType *pTestHeap,
    int16 iTargetPcm);

static void
VpRemoveTargetCurrent (
    VpLineCtxType *pLineCtx,
    Vp880TestHeapType *pTestHeap);

/*------------------------------------------------------------------------------
 * Vp880TestOpenVdc
 * This functions implements the AC RLOOP Test primitive ...
 *
 * Parameters:
 *  pLineCtx    - pointer to the line context
 *  pArgsUntyped- pointer to the test specific inputs.
 *  handle      - unique test handle
 *  callbak     - indicates if this function was called via a callback
 *
 * Returns:
 * --
 *----------------------------------------------------------------------------*/
EXTERN VpStatusType
Vp880TestDcRloop(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback,
    VpTestIdType testId)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp880LineObjectType *pLineObj =  pLineCtx->pLineObj;
    Vp880CurrentTestType *pTestInfo = &pDevObj->currentTest;
    Vp880TestHeapType *pTestHeap = pTestInfo->pTestHeap;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    VpTestStatusType *pErrorCode = &pDevObj->testResults.errorCode;
    VpTestDcRLoopType *pDcRloopData = &(pTestHeap->testArgs).dcRloop;

    int16 *pTestState = &pTestInfo->testState;
    uint8 *pAdcState = &pTestHeap->adcState;
    uint8 chanId = pLineObj->channelId;
    uint16 tick =  pDevObj->devProfileData.tickRate;

    VP_TEST(VpLineCtxType, pLineCtx, ("Vp880TestDcRloop(ts:%i)", *pTestState));
    if (FALSE == callback) {
        return VpDcRloopArgCheck(pLineCtx, pArgsUntyped, handle, pAdcState, testId);
    }

    switch (*pTestState) {
        case COMMON_TEST_SETUP:
            if (pDcRloopData->calMode) {
                Vp880CommonTestSetup(pLineCtx, deviceId);
                /* no break */
            }

        case DC_RLOOP_SETUP:
            VpDcRloopSetup(pLineObj, pTestHeap, deviceId);
            *pTestState = DC_RLOOP_SET_CURRENT;
            /* no break */

        case DC_RLOOP_SET_CURRENT:
            /* Once the current is set, allow it to settle and advance state */
            if (TRUE == VpSetTargetCurrent(pLineCtx, pTestHeap,
                pTestHeap->testArgs.dcRloop.iTestLevel)) {

                if (pTestHeap->testArgs.dcRloop.iTestLevel >=
                    (int16)VP880_UNIT_CONVERT(18000, VP880_UNIT_UA, VP880_UNIT_ADC_IMET_NOR_JA)) {

                    VpSetTestStateAndTimer(pDevObj, pTestState, DC_RLOOP_ADC_SETUP_ILG,
                        MS_TO_TICKRATE(VP880_STATE_CHANGE_SETTLING_TIME_SHORT, tick) );
                } else {
                    VpSetTestStateAndTimer(pDevObj, pTestState, DC_RLOOP_ADC_SETUP_VAB,
                        MS_TO_TICKRATE(VP880_STATE_CHANGE_SETTLING_TIME_SHORT, tick) );
                }

            } else {
                *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
                VpSetTestStateAndTimer(pDevObj, pTestState, DC_RLOOP_QUIT, NEXT_TICK);
            }
            break;


        /*
         * ILG is only measured if the requested target current can be set with
         * the ILA value in the DC feed register
         */
        case DC_RLOOP_ADC_SETUP_ILG:
            Vp880AdcSetup(pLineCtx, VP880_LONGITUDINAL_DC_I, FALSE);
            VpSetTestStateAndTimer(pDevObj, pTestState, DC_RLOOP_COLLECT_ILG_DATA,
                MS_TO_TICKRATE(10, tick) );
            break;

        case DC_RLOOP_COLLECT_ILG_DATA: {
            VpStartPcmCollect(pLineCtx, VP_PCM_OPERATION_AVERAGE,
                DC_RLOOP_GET_ILG_RESULT, pDcRloopData->settlingTime,
                pDcRloopData->integrateTime);
            break;
        }

        case DC_RLOOP_GET_ILG_RESULT:
            VpGetPcmResult(pDevObj, VP_PCM_OPERATION_AVERAGE, FALSE, TRUE, chanId,
                VP880_LONGITUDINAL_DC_I, &(pDevObj->testResults.result.dcvab).ilg);
            *pTestState = DC_RLOOP_ADC_SETUP_VAB;
            /* no break */




        case DC_RLOOP_ADC_SETUP_VAB:
            Vp880AdcSetup(pLineCtx, VP880_METALLIC_DC_V, FALSE);
            VpSetTestStateAndTimer(pDevObj, pTestState, DC_RLOOP_COLLECT_VAB_DATA,
                MS_TO_TICKRATE(VP880_ADC_CHANGE_SETTLING_TIME, tick) );
            break;

        case DC_RLOOP_COLLECT_VAB_DATA: {
            VpStartPcmCollect(pLineCtx, VP_PCM_OPERATION_AVERAGE,
                DC_RLOOP_GET_VAB_RESULT, pDcRloopData->settlingTime,
                pDcRloopData->integrateTime);
            break;
        }

        case DC_RLOOP_GET_VAB_RESULT:
            VpGetPcmResult(pDevObj, VP_PCM_OPERATION_AVERAGE, FALSE, TRUE, chanId,
                VP880_METALLIC_DC_V, &(pDevObj->testResults.result.dcvab).vab);
            VpSetTestStateAndTimer(pDevObj, pTestState, DC_RLOOP_END, NEXT_TICK);
            break;



        case DC_RLOOP_REMOVE_CURRENT:
            /*
             * The timing is very important (40ms) prevents a
             * tip/ring pol rev during the restore of the line.
             */
            VpRemoveTargetCurrent(pLineCtx, pTestHeap);
            VpSetTestStateAndTimer(pDevObj, pTestState, DC_RLOOP_END,
                MS_TO_TICKRATE(40, tick));
            break;


        case DC_RLOOP_END:
            /* The test has completed with no issues and generates an event. */
            *pErrorCode = VP_TEST_STATUS_SUCCESS;
            VpGenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp880TestDcRloop(): complete"));
            break;

        case DC_RLOOP_QUIT:
            /* The test has eneded early due to the current errorCode */
            VpGenerateTestEvent(pLineCtx, testId, handle, TRUE, TRUE);
            VP_WARNING(VpLineCtxType, pLineCtx, ("Vp880TestDcRloop(ec:%i): quit",
                 *pErrorCode));
            break;

        default:
            *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
            VpGenerateTestEvent(pLineCtx, testId, handle, TRUE, TRUE);
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp880TestDcRloop(): invalid teststate"));
            break;

    }
    return VP_STATUS_SUCCESS;
}

/*------------------------------------------------------------------------------
 * VpDcRloopArgCheck
 * This functions is called by the Vp880TestDcRloop() function if
 * Vp880TestDcRloop() was not called via a callback.
 *
 * This function will check that no errorCode is pending and that the input
 * arguments for the AC Rloop primitive are legal. The function also sets up
 * the DC Rloop state machine for the first state and initializes the results
 * from this fucntion to 0.
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context
 *  pArgsUntyped    - pointer to the struct which contains test specfic info.
 *  handle          - unique test handle
 *  pAdcState       - pointer to the value of the current ADC measurement state
 *
 * Returns:
 * VpStatusType
 *
 * Result:
 *  This function initializes all relevent result values to 0. Stores the
 *  TestId and handle into the device Object as well as modifes the current
 *  ADC state to the first measurement.
 *
 *----------------------------------------------------------------------------*/
static VpStatusType
VpDcRloopArgCheck(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    uint8 *pAdcState,
    VpTestIdType testId)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880LineObjectType *pLineObj =  pLineCtx->pLineObj;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp880CurrentTestType *pTestInfo = &(pDevObj->currentTest);
    VpTestDcRLoopType *pTestInput = (VpTestDcRLoopType*)pArgsUntyped;
    VpTestDcRLoopType *pDcRloopData = &(pTestInfo->pTestHeap->testArgs).dcRloop;
    int16 *pTestState = &pTestInfo->testState;

    if (VP_TEST_STATUS_SUCCESS != pDevObj->testResults.errorCode) {
        /* make sure that starting test did not generate any errorCodes */
        VpGenerateTestEvent(pLineCtx, testId, handle, FALSE, TRUE);
        VP_WARNING(VpLineCtxType, pLineCtx, ("VpDcRloopArgCheck(ec:%i): bailed",
            pDevObj->testResults.errorCode));
        return VP_STATUS_SUCCESS;
    }

    pDevObj->testResults.result.dcvab.vab = 0;
    pDevObj->testResults.result.dcvab.ilg = 0;

    pTestInfo->testId = testId;
    pTestInfo->handle = handle;

    /* This test cannot run if the isolate relay is open*/
    if (VP_RELAY_RESET == pLineObj->relayState) {
        pDevObj->testResults.errorCode = VP_TEST_STATUS_RESOURCE_NA;
        VpGenerateTestEvent(pLineCtx, testId, handle, TRUE, TRUE);
        VP_WARNING(VpLineCtxType, pLineCtx, ("VpDcRloopArgCheck(): bailed openRelay"));
        return VP_STATUS_SUCCESS;
    }

    /* TODO: check input arguments before assigining them to test struct!!! */
    pDcRloopData->calMode = pTestInput->calMode;
    pDcRloopData->integrateTime = pTestInput->integrateTime;
    pDcRloopData->settlingTime = pTestInput->settlingTime;
    pDcRloopData->iTestLevel = pTestInput->iTestLevel;

    if (pDcRloopData->iTestLevel == 0) {
        /* remove the target current if 0 is asked for */
        VpSetTestStateAndTimer(pDevObj, pTestState, DC_RLOOP_REMOVE_CURRENT, NEXT_TICK);
    } else {
        /*
         * force the function to start running state maching on next tick only if
         * all arguments checkout ok and no events were generated.
         */
        VpSetTestStateAndTimer(pDevObj, pTestState, COMMON_TEST_SETUP, NEXT_TICK);
    }

    return VP_STATUS_SUCCESS;
}

/*------------------------------------------------------------------------------
 * VpOpenVdcSetup
 * This function is called by the Vp880TestDcRloop() state machine during
 * the DC_RLOOP_SETUP state.
 *
 * This function is used to configure the line for the dc rloop test.
 *
 * Parameters:
 *  pDevObj         - pointer to the device object.
 *  deviceId        - device ID
 *  ecVAl           - current enable channel value.
 *
 * Returns:
 * --
 * Result:
 *  Disable AISN
 *----------------------------------------------------------------------------*/
static void VpDcRloopSetup(
    Vp880LineObjectType *pLineObj,
    Vp880TestHeapType *pTestHeap,
    VpDeviceIdType deviceId)
{
    pLineObj->icr4Values[0] = 0x10;
    pLineObj->icr4Values[1] = 0x00;
    pLineObj->icr4Values[2] = 0x00;
    pLineObj->icr4Values[3] = 0x00;

    /* Disable AISN */
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_ICR4_WRT, VP880_ICR4_LEN, pLineObj->icr4Values);

    return;
}

static bool
VpSetTargetCurrent (
    VpLineCtxType *pLineCtx,
    Vp880TestHeapType *pTestHeap,
    int16 iTargetPcm)
{
    Vp880LineObjectType *pLineObj =  pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 slacState = VP880_SS_ACTIVE;

    /* Min ILA current limit 18mA (dc feed reg) */
    int16 ilaPcmMin = (int16)VP880_UNIT_CONVERT(18000, VP880_UNIT_UA,
        VP880_UNIT_ADC_IMET_NOR_JA);
    /* Max ILA current limit 49mA (dc feed reg) */
    int16 ilaPcmMax = (int16)VP880_UNIT_CONVERT(49000, VP880_UNIT_UA,
        VP880_UNIT_ADC_IMET_NOR_JA);
    /* Lower algorithm break point -10mA */
    int16 negPcmBrk = (int16)VP880_UNIT_CONVERT(-10000, VP880_UNIT_UA,
        VP880_UNIT_ADC_IMET_NOR_JA);
    /* Lower algorithm break point 10mA */
    int16 posPcmBrk = (int16)VP880_UNIT_CONVERT(10000, VP880_UNIT_UA,
        VP880_UNIT_ADC_IMET_NOR_JA);



#define VP880_UNIT_DC_BIAS_CURRENT 3494L
    if ((iTargetPcm >= negPcmBrk) && (iTargetPcm <= posPcmBrk)) {
        uint8 gain = VP880_AR_6DB_GAIN;
        int32 dcBias = 0;
        uint8 sigGenAB[VP880_SIGA_PARAMS_LEN] =
            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        /*
         *  Using signal generator A bias to produce target currents between
         * -10 mA and 10 mA.
         */

        /* disable the signal generator */
        pLineObj->sigGenCtrl[0] = 0;
        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_GEN_CTRL_WRT,
            VP880_GEN_CTRL_LEN, pLineObj->sigGenCtrl);


        /* To get the +/-10mA range we have to gain up the voice path */
        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_VP_GAIN_WRT,
            VP880_VP_GAIN_LEN, &gain);

        /* Convert target to a DAC (sig gen a bias) */
        dcBias = (int16)VP880_UNIT_CONVERT(iTargetPcm, VP880_UNIT_ADC_IMET_NOR_JA,
            VP880_UNIT_DC_BIAS_CURRENT);

        /* Contain value to dc bias to parameter's bits */
        dcBias = (dcBias < -32768) ? (-32678) : (dcBias);
        dcBias = (dcBias > 32767) ? (32677) : (dcBias);

        /* write bias to the device */
        sigGenAB[1] = (uint8)((dcBias >> 8) & 0xFF);
        sigGenAB[2] = (uint8)(dcBias & 0xFF);

        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_SIGA_PARAMS_WRT,
            VP880_SIGA_PARAMS_LEN, sigGenAB);

        /* enable the signal generator */
        pLineObj->sigGenCtrl[0] = VP880_GEN_CTRL_EN_BIAS;
        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_GEN_CTRL_WRT,
            VP880_GEN_CTRL_LEN, pLineObj->sigGenCtrl);

        /* Disable the Feed Circuit from the IHL pin */
        pLineObj->icr2Values[0] = 0x40;
        pLineObj->icr2Values[1] = 0x00;
        pLineObj->icr2Values[2] &= 0x0C;
        pLineObj->icr2Values[2] |= 0x01;
        pLineObj->icr2Values[3] &= 0x0C;
        pLineObj->icr2Values[3] |= 0x01;

    } else if ( (iTargetPcm >= ilaPcmMin) && (iTargetPcm <= ilaPcmMax) ) {
        uint8 chanId = pLineObj->channelId;
        uint8 dcFeed[VP880_DC_FEED_LEN];
        int32 ilaFeed = 0;
        int16 ilaPcmCal = 0;
        int16 ilaPcmCalTarg = 0;
        int16 ilaPcmError = 0;
        int16 ilaPcmAct = 0;

        /*
         * Using the dcfeed registers ILA parameter to set target
         * currents between 18mA and 49mA. However, the accuracy of
         * of the ILA parameter needs to be adjusted by the ILA calibration
         * data that was taken by Cal Line.
         */

         /* clear out the LI and ila parameters from dc feed register */
        dcFeed[0] = (pTestHeap->dcFeed[0] & ~(VP880_LONG_IMP_MASK));
        dcFeed[1] = (pTestHeap->dcFeed[1] & ~(VP880_ILA_MASK));

        /* get calibration ila info related to the target ila */
        if (iTargetPcm < 12494) {                /* 18mA to < 22mA */
            ilaPcmCal = pDevObj->vp880SysCalData.ila20[chanId];
            ilaPcmCalTarg =  11358;  /* 20mA */
        } else if (iTargetPcm < 15901) {         /* 23mA to < 28mA */
            ilaPcmCal = pDevObj->vp880SysCalData.ila25[chanId];
            ilaPcmCalTarg =  14197;  /* 25mA */
        } else if (iTargetPcm < 2044) {         /* 29mA to < 36mA */
            ilaPcmCal = pDevObj->vp880SysCalData.ila32[chanId];
            ilaPcmCalTarg =  18173;  /* 32mA */
        } else {                                /* 36mA and higher */
            ilaPcmCal = pDevObj->vp880SysCalData.ila40[chanId];
            ilaPcmCalTarg =  22716; /* 40mA */
        }

        /* determine the error for the requested target current in PCM units */
        ilaPcmAct = ilaPcmCal - pDevObj->vp880SysCalData.ilaOffsetNorm[chanId];
        ilaPcmError = ilaPcmAct - ilaPcmCalTarg;

       /* Convert target current to dc feed register ILA range */
        ilaFeed = (iTargetPcm - ilaPcmError) - ilaPcmMin;

        /* Convert target to a uA */
        ilaFeed = (int16)VP880_UNIT_CONVERT(ilaFeed, VP880_UNIT_ADC_IMET_NOR_JA,
            VP880_UNIT_UA);

        /* round */
        ilaFeed = (ilaFeed + 500) / 1000;

        /* Contain value to ILA parameter's bits */
        ilaFeed = (ilaFeed < 0) ? (0) : (ilaFeed);
        ilaFeed = (ilaFeed > 31) ? (31) : (ilaFeed);

        /* OR in the adjusted target ILA parameter and write to the device */
        dcFeed[1] |= (uint8)ilaFeed;

        VP_TEST(VpLineCtxType, pLineCtx, ("VpSetTargetCurrent() 0x%02x%02x",
            dcFeed[0], dcFeed[1]));

        /* write to the device */
        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_DC_FEED_WRT,
            VP880_DC_FEED_LEN, dcFeed);

        pLineObj->icr2Values[0] = 0x00;
        pLineObj->icr2Values[1] = 0x00;
        pLineObj->icr2Values[2] &= 0x0C;
        pLineObj->icr2Values[2] |= 0x01;
        pLineObj->icr2Values[3] &= 0x0C;
        pLineObj->icr2Values[3] |= 0x01;

    } else {
        /* This is a current region unsupported by the primitive */
        VP_TEST(VpLineCtxType, pLineCtx, ("VpSetTargetCurrent() invalid target current %li uA",
            VP880_UNIT_CONVERT(iTargetPcm, VP880_UNIT_ADC_IMET_NOR_JA, VP880_UNIT_UA)));
        return FALSE;
    }

    /*
     * Store original State then..
     * Put device into active Polrev if the target is negative
     * otherwise the device will be put into active normal
     */
    if (iTargetPcm < 0) {
        slacState = VP880_SS_ACTIVE_POLREV;
    }
    pLineObj->slicValueCache = slacState;
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_SYS_STATE_WRT,
        VP880_SYS_STATE_LEN, &pLineObj->slicValueCache);


    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_ICR2_WRT,
        VP880_ICR2_LEN, pLineObj->icr2Values);

    return TRUE;
}

static void
VpRemoveTargetCurrent (
    VpLineCtxType *pLineCtx,
    Vp880TestHeapType *pTestHeap)
{
    Vp880LineObjectType *pLineObj =  pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;

    uint8 mpiBuffer[4 + VP880_VP_GAIN_LEN +
                        VP880_DC_FEED_LEN +
                        VP880_SYS_STATE_LEN +
                        VP880_GEN_CTRL_LEN];
    uint8 mpiIndex = 0;
    /*
     * remove target current the order is very important
     */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
        VP880_VP_GAIN_WRT, VP880_VP_GAIN_LEN, &pTestHeap->vpGain);

    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
        VP880_DC_FEED_WRT, VP880_DC_FEED_LEN, pTestHeap->dcFeed);

    pLineObj->slicValueCache = VP880_SS_IDLE | VP880_SS_ACTIVATE_MASK;;
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
        VP880_SYS_STATE_WRT, VP880_SYS_STATE_LEN, &pLineObj->slicValueCache);

    pLineObj->sigGenCtrl[0] = pTestHeap->sigCtrl;
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
        VP880_GEN_CTRL_WRT, VP880_GEN_CTRL_LEN, pLineObj->sigGenCtrl);


    /* send down the mpi commands */
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, mpiBuffer[0],
        mpiIndex-1, &mpiBuffer[1]);
    return;
}


#endif /* VP880_INCLUDE_TESTLINE_CODE */

#endif /* VP_CC_880_SERIES */




