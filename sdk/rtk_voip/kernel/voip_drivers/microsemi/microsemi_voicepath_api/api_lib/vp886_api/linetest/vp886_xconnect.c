/*
 * Copyright (c) 2012, Microsemi Corporation
 *
 * $Revision: 11400 $
 * $LastChangedDate: 2014-05-06 16:45:24 -0500 (Tue, 06 May 2014) $
 */

#include "vp_api_cfg.h"

#if defined (VP_CC_886_SERIES)
#if defined (VP886_INCLUDE_TESTLINE_CODE)

#include "vp886_testline_int.h"

#define MAX_ITERATION                   10      /* Max settling loop iterations per measurements */

typedef enum
{
    XCONNECT_SETUP              = 0,
    XCONNECT_START_MEASUREMENT  = 1,
    XCONNECT_GET_RESULT         = 2,
    XCONNECT_GEN_EVENT          = 3,
    XCONNECT_MAX                = 3,
    XCONNECT_ENUM_SIZE          = FORCE_STANDARD_C_ENUM_SIZE
}  Vp886XConnectStateTypes;

bool Vp886TestXConnectSM(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    VpTestXConnectType *pXConnectArgs,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap);

static bool
VpXConnectSetup(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap);

static bool
VpXConnectSetState(
    VpLineCtxType *pLineCtx,
    VpTestXConnectType *pXConnectData);

/*------------------------------------------------------------------------------
 * Vp886TestXConnect
 *----------------------------------------------------------------------------*/
EXTERN VpStatusType
Vp886TestXConnect(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    const void *pArgsUntyped,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap,
    bool callback)
{

    VpTestXConnectType *pXConnectArgs = NULL;
    VpStatusType status = VP_STATUS_SUCCESS;

    if (!callback) {
        const VpTestXConnectType *pInputArgs = (VpTestXConnectType *)pArgsUntyped;

        if (pInputArgs == NULL) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestXConnect NULL input struct"));
            return VP_STATUS_INVALID_ARG;
        }
        pTestHeap->loopCnt =0;

        /* reset the results to nonmeaningful values */
        pLineObj->testResults.result.xConnect.data = VP_INT16_MAX;
        pLineObj->testResults.result.xConnect.condition = pInputArgs->measReq;

        /* store the input args into the test heap for later use */
        VpMemCpy(&pTestHeap->testArgs.xConnect, pInputArgs, sizeof(VpTestXConnectType));

        /* make sure the requested lead is valid */
        if ((pInputArgs->measReq != VP_XCONNECT_DISC_V) &&
            (pInputArgs->measReq != VP_XCONNECT_12VVOC_I)) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestXConnect invalid measReq: %i", pInputArgs->measReq));
            return VP_STATUS_INVALID_ARG;
        }
    
        pTestHeap->nextState = XCONNECT_SETUP;
    }

    pXConnectArgs = &pTestHeap->testArgs.xConnect;

    /* Continue calling the state machine until it wants to stop */
    while (Vp886TestXConnectSM(pLineCtx, pLineObj, pXConnectArgs, pTestInfo, pTestHeap));

    return status;
}

/*------------------------------------------------------------------------------
 * Vp886TestXConnectSM
 *----------------------------------------------------------------------------*/
bool Vp886TestXConnectSM(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    VpTestXConnectType *pXConnectArgs,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap)
{

    bool requestNextState = FALSE;

    VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestXConnectSM state: %i", pTestHeap->nextState));

    switch (pTestHeap->nextState) {
        case XCONNECT_SETUP: {
            /* This test cannot run if the isolate relay is open */
            if (((pLineObj->termType == VP_TERM_FXS_ISOLATE) ||
                 (pLineObj->termType == VP_TERM_FXS_ISOLATE_LP)) &&
                 (pLineObj->relayState == VP_RELAY_RESET)) 
            {
                pLineObj->testResults.errorCode = VP_TEST_STATUS_RESOURCE_NA;
                VP_WARNING(VpLineCtxType, pLineCtx, ("Vp886TestXConnectSM: incorrect termination type and relay state combination"));
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
                return FALSE;
            }
             
            if(VpXConnectSetup(pLineCtx, pTestHeap) == FALSE) {
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestXConnectSM: failed VpXConnectSetup"));
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                break;
            }                                  
            /* wait for the requested amount of time */
            if (! Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_LINE_TEST, 100, 0, XCONNECT_SETUP)) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
                VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestXConnectSM: Cannot add to timer queue."));
                break;
            }
            
            pTestHeap->nextState = XCONNECT_START_MEASUREMENT;
            break;
        }

        case XCONNECT_START_MEASUREMENT: {
            if (VpXConnectSetState(pLineCtx, pXConnectArgs) == FALSE) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestXConnectSM: VpXConnectSetState failed"));
                break;
            }
            /* Reset the SADC settling counter */
            pTestHeap->nextState = XCONNECT_GET_RESULT;
            break;
        }

        case XCONNECT_GET_RESULT: {
            Vp886SadcSignalType sadcSigType = VP886_SADC_SEL_NO_CONN;
            uint8 adcConf[VP886_R_SADC_LEN];
            Vp886DeviceObjectType *pDevObj = pLineCtx->pDevCtx->pDevObj;
            Vp886CmnCalDeviceDataType *pCalData = &pDevObj->calData[pLineObj->channelId].cmn;
            Vp886AdcMathDataType mathData;
            int16 offset;
            int16 gain;
            if (pXConnectArgs->measReq == VP_XCONNECT_DISC_V) {
                sadcSigType = VP886_SADC_SEL_MET_VDC;
                if (pLineObj->registers.sysState[0] & VP886_R_STATE_POL) {
                    offset = pCalData->vabSenseReverse.offset;
                    gain = pCalData->vabSenseReverse.gain;
                } else {
                    offset = pCalData->vabSenseNormal.offset;
                    gain = pCalData->vabSenseNormal.gain;
                }
            } else {
                sadcSigType = VP886_SADC_SEL_MET_I;
                offset = pCalData->sadc.offset;
                gain   = pCalData->sadc.gain;
            } 
                                                  
            /* If the adc setup is not set as expected, set it and measure again */
            VpSlacRegRead(NULL, pLineCtx,  VP886_R_SADC_RD, VP886_R_SADC_LEN, adcConf);
            if ((adcConf[1] & 0x1F) != (uint8)sadcSigType) {
                if ((pTestHeap->loopCnt)++ >= MAX_ITERATION) {
                    VP_TEST(VpLineCtxType, pLineCtx,
                        ("The SADC can't settle!"));                        
                    Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                    break;
                }
                pTestHeap->nextState = XCONNECT_START_MEASUREMENT;
                requestNextState = TRUE;

                break;
            }

            /* get the data and generate the event */
            if (Vp886SadcGetMathData(pLineCtx, VP886_SADC_BUF_1, &mathData, offset, gain) == FALSE) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886SadcGetMathData calculations failed"));
                requestNextState = TRUE;
                pTestHeap->nextState = XCONNECT_GEN_EVENT;
                break;
            }
            pLineObj->testResults.result.xConnect.data = -1*(int16)mathData.average;
            
            pTestHeap->nextState = XCONNECT_GEN_EVENT;
            requestNextState = TRUE;
            break;
        }

        case XCONNECT_GEN_EVENT: {
            /* generate the event */
            Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_SUCCESS);
            break;
        }

        default:
            /* some how the next state variable was corrupted */
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestXConnectSM invalid state: %i",
                pTestHeap->nextState));
            Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
            break;
    }

    return requestNextState;
}

/*------------------------------------------------------------------------------
 * VpXConnectSetup()
 * This function is called by the Vp886TestXConnect() state machine during
 * the XCONNECT_SETUP state.
 *
 * This function is used to configure the line for the condition test.
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context.
 *  pTestHeap       - pointer to the test heap.
 *
 * Returns: TRUE if no error
 *          FALSE if error
 * --
 * Result: the line is configured accordingly for the test
 * --
 *----------------------------------------------------------------------------*/
static bool
VpXConnectSetup(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap)
{

    uint8 vadcConvConf[VP886_R_SADC_LEN];
    /*
     * FORCE ADC to metalic AC:
     * THIS is a required workaround. If the device is in idle and
     * the ADC is forced on in ICR4, the data from the ADC will be
     * railed. However, if the converter configuration register is
     * set to NO_CONNECT first, then this issue does not exist.
    */
    vadcConvConf[0] = pTestHeap->scratch.vadc[0] | VP886_R_VADC_SM_OVERRIDE;
    vadcConvConf[1] = VP886_R_VADC_SEL_ADC_OFFSET;
    VpMemCpy(&vadcConvConf[2], &pTestHeap->scratch.vadc[2], VP886_R_VADC_LEN - 2);

    if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_VADC_WRT, VP886_R_VADC_LEN, vadcConvConf) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpXConnectSetup() failed VP886_R_VADC_WRT"));
        return FALSE;
    }
    
    if (pTestHeap->testArgs.xConnect.phase == 0) {
        Vp886LineObjectType *pLineObj =  pLineCtx->pLineObj;
        Vp886DeviceObjectType *pDevObj = pLineCtx->pDevCtx->pDevObj;
        uint8 disn = 0x00;
        uint8 opCond = VP886_R_OPCOND_CUT_RX | VP886_R_OPCOND_HIGHPASS_DIS;  
        uint8 opFunction = VP886_R_OPFUNC_CODEC_LINEAR;
        uint8 dcfeed[VP886_R_DCFEED_LEN];
        uint8 switchRegParam[VP886_R_SWPARAM_LEN];
        uint8 ssCfg[VP886_R_SSCFG_LEN];
    
        ssCfg[0] = (VP886_R_SSCFG_AUTO_SYSSTATE | VP886_R_SSCFG_ZEROCROSS);
        ssCfg[1] = pTestHeap->scratch.ssCfg[1];
                      
        /*backup icr2-4 into the lineObject*/
        if (VpSlacRegRead(NULL, pLineCtx, VP886_R_ICR3_RD, VP886_R_ICR3_LEN, pLineObj->registers.icr3) == FALSE) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("VpXConnectSetup failed VP886_R_ICR3_RD"));
            return FALSE;
        }
    
        if (VpSlacRegRead(NULL, pLineCtx, VP886_R_ICR2_RD, VP886_R_ICR2_LEN, pLineObj->registers.icr2) == FALSE) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("VpXConnectSetup failed VP886_R_ICR2_RD"));
            return FALSE;
        }
        if (VpSlacRegRead(NULL, pLineCtx, VP886_R_ICR4_RD, VP886_R_ICR4_LEN, pLineObj->registers.icr4) == FALSE) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("VpXConnectSetup failed VP886_R_ICR4_RD"));
            return FALSE;
        }

        /*backup current content of line state register*/
        if (VpSlacRegRead(NULL, pLineCtx, VP886_R_STATE_RD, VP886_R_STATE_LEN, pLineObj->registers.sysState) == FALSE) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("VpXConnectSetup failed VP886_R_STATE_RD"));
            return FALSE;
        }
    
        pLineObj->registers.icr2[0] = 0x0C;
        pLineObj->registers.icr2[1] = 0x0C;
        pLineObj->registers.icr2[2] &= 0x0C;
        pLineObj->registers.icr2[3] &= 0x0C;
    
        pLineObj->registers.icr3[0] |= 0x21;
        pLineObj->registers.icr3[1] |= 0x21;
    
        pLineObj->registers.icr4[0] |= 0x01;
        pLineObj->registers.icr4[1] |= 0x01;
      
        dcfeed[0] = (pTestHeap->scratch.dcFeed[0] & 0xc0) | 0x22;/*VOCShift=VAS3=1*/
        dcfeed[1] =  0x22; /*Li=ILA1=1*/
    
                             
   
        /* Disable hi-pass filter and cutoff receive path */
        if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_OPCOND_WRT, VP886_R_OPCOND_LEN, &opCond) == FALSE) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("VpXConnectSetup() failed VP886_R_SIGCTRL_WRT"));
            return FALSE;
        }
        /* Set to read linear data */
        if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_OPFUNC_WRT, VP886_R_OPFUNC_LEN, &opFunction) == FALSE) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("VpXConnectSetup() failed VP886_R_SIGCTRL_WRT"));
            return FALSE;
        }
        /* Disable DISN */
        if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_DISN_WRT, VP886_R_DISN_LEN, &disn) == FALSE) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("VpXConnectSetup() failed VP886_R_DISN_WRT"));
            return FALSE;
        }
    
        /* VOC on, A/B sns on, incr ADC rg */
        if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR2_WRT, VP886_R_ICR2_LEN, pLineObj->registers.icr2) == FALSE) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("VpXConnectSetup() failed VP886_R_ICR2_WRT"));
            return FALSE;
        }
    
        /* Force line control circuits on */
        if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR3_WRT, VP886_R_ICR3_LEN, pLineObj->registers.icr3) == FALSE) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("VpXConnectSetup() failed VP886_R_ICR3_WRT"));
            return FALSE;
        }
    
        /* Force ADC on */
        if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR4_WRT, VP886_R_ICR4_LEN, pLineObj->registers.icr4) == FALSE) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("VpXConnectSetup() failed VP886_R_ICR4_WRT"));
            return FALSE;
        }
    
        /* Disable automatic state changes */
        if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_SSCFG_WRT, VP886_R_SSCFG_LEN , ssCfg) == FALSE) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("VpXConnectSetup() failed VP886_R_SSCFG_WRT"));
            return FALSE;
        }
    
        /* State Change */
        if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_STATE_WRT, VP886_R_STATE_LEN, pLineObj->registers.sysState) == FALSE) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("VpXConnectSetup failed VP886_R_STATE_WRT"));
            return FALSE;
        }
    
        /* Set the floor voltage to -60V (tracker only) */
        if (VP886_IS_TRACKER(pDevObj)) { 
            /* Regular tracker - force a fixed voltage -60V */
            switchRegParam[0] = pTestHeap->scratch.swParam[0] & ~VP886_R_SWPARAM_FLOOR_V;
            switchRegParam[0] |= 0x0B;
            switchRegParam[1] = pTestHeap->scratch.swParam[1];
            switchRegParam[2] = pTestHeap->scratch.swParam[2];
                 
            if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_SWPARAM_WRT, VP886_R_SWPARAM_LEN, switchRegParam)==FALSE) 
            {
                VP_ERROR(VpLineCtxType, pLineCtx, ("VpXConnectSetup() failed VP886_R_SWPARAM_WRT"));
                return FALSE;                                        
            }   
        }
        /* ILA = 20mA, VOCshift =1*/
        /* LI=1, min VOC = 12V; max curr lim */
        if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_DCFEED_WRT, VP886_R_DCFEED_LEN, dcfeed) == FALSE) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("VpXConnectSetup() failed VP886_R_DCFEED_WRT"));
            return FALSE;
        }
    }
    return TRUE;
}


/*------------------------------------------------------------------------------
 * VpXConnectSetState()
 * This function is called by the Vp886TestGetXConnect() state machine during
 * the XCONNECT_STATE_SETUP state.
 *
 * This function is used to set the requested line state and call the Vp886AdcSetup().
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context.
 *  pXConnectData   - input data from the ltAPI.
 *
 * Returns:
 *  FALSE if something goes wrong during the sadc setup.
 *  TRUE if everything goes ok.
 *
 * Result:
 *  The SADC is configured accordingly for the test
 *----------------------------------------------------------------------------*/
static bool
VpXConnectSetState(
    VpLineCtxType *pLineCtx,
    VpTestXConnectType *pXConnectData)
{
    Vp886LineObjectType *pLineObj =  pLineCtx->pLineObj;
    Vp886SadcSignalType sadcSigType = VP886_SADC_SEL_NO_CONN;

    switch(pXConnectData->measReq) {
        case VP_XCONNECT_DISC_V: {
            pLineObj->registers.sysState[0] = VP886_R_STATE_CODEC | VP886_R_STATE_SS_DISCONNECT;
            sadcSigType = VP886_SADC_SEL_MET_VDC;
            break;
        }
        case VP_XCONNECT_12VVOC_I: {
            if (pXConnectData->isFeedPositive == TRUE) {
                pLineObj->registers.sysState[0] = VP886_R_STATE_CODEC | VP886_R_STATE_SS_ACTIVE;
            } else {
                pLineObj->registers.sysState[0] = VP886_R_STATE_CODEC | VP886_R_STATE_POL | VP886_R_STATE_SS_ACTIVE;
            }
            sadcSigType = VP886_SADC_SEL_MET_I;
            break;
        } 
        default: {
            VP_ERROR(VpLineCtxType, pLineCtx, ("VpXConnectSetState(): invalid measReq"));
            return FALSE;
        }    
    }
    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_STATE_WRT, VP886_R_STATE_LEN, pLineObj->registers.sysState)== FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpXConnectSetState() failed VP886_R_STATE_WRT"));
        return FALSE;                                        
    }

    /* start the ADC to measure VSAB */
    return Vp886SadcSetup(pLineCtx, VP886_SADC_MATH_SINGLE, VP886_SADC_RATE_FULL,
            sadcSigType,
            VP886_SADC_SEL_NO_CONN,
            VP886_SADC_SEL_NO_CONN,
            VP886_SADC_SEL_NO_CONN,
            VP886_SADC_SEL_NO_CONN,
            pXConnectData->settlingTime,
            pXConnectData->integrateTime,
            VP886_LINE_TEST);

}


#endif /* VP886_INCLUDE_TESTLINE_CODE */
#endif /* VP_CC_886_SERIES */
