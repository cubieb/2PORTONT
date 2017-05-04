/** \file ve886_int.c
 * 
 *
 * This file contains the major process of zarlink slic
 * 
 *
 * Copyright (c) 2013, Realtek Semiconductor, Inc.
 *
 */
#include "vp_api_common.h"
#include "vp_api_option.h"
//#include "vp886_testline.h"
//#include "vp886_api.h"
#include "vp886_registers.h"

#include "ve886_int.h"
#include "ve886_api.h"
#include "Ve_profile.h"
#include "zarlinkCommonSlic.h"
#include "zarlinkCommonDaa.h"
#include "zarlinkCommonInit.h"  

#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_ZARLINK_ON_NEW_ARCH
#include "snd_define.h"
#else
#include "Slic_api.h"	/* for PCM_MODE */
#endif

#include "vp886_api_int.h" 

#undef DEBUG_INT

/*
** local function 
*/

/*
** external variable
*/

/*
** external function
*/



/* 
** API	  : Ve886CreateDevObj()
** Desp	  :	Create Realtek dev obj
** input  : dev_id, RTKDevType 
** return : max line number
*/
int Ve886CreateDevObj(
			/* Realtek */
			RTKDevType 				dev_type, 
			int						ch_id,
			RTKDevObj 				*pDevObj, 
			RTKLineObj 				LineObj[],

			/* Zarlink */
			VpDeviceIdType 			devId, 
			VpDeviceType  			vpDevType, 
			Vp886DeviceObjectType	*pVpDevObj, 
			VpDevCtxType  			*pVpDevCtx, 
			Vp886LineObjectType  	VpLineObj[],
			VpLineCtxType 			VpLineCtx[]
			)
{
	pDevObj->dev_id				= (unsigned int)devId;
	pDevObj->dev_type			= dev_type;
	pDevObj->pDevObj 			= pVpDevObj;
	pDevObj->pDevCtx 			= pVpDevCtx;
	

	PRINT_MSG("Creating dev 0x%08x, type=%d\n", (unsigned int)devId, dev_type);

	switch(dev_type) {
		case DEV_FXS:
		case DEV_FXO:
			pDevObj->max_line			= 1;
			pDevObj->pLine[0]			= &LineObj[0];

			pDevObj->pLine[0]->pDev		= pDevObj;
			pDevObj->pLine[0]->pLineObj	= &VpLineObj[0];
			pDevObj->pLine[0]->pLineCtx	= &VpLineCtx[0];

			break;

		case DEV_FXSFXO:
		case DEV_FXSFXS:
			pDevObj->max_line			= 2;
			pDevObj->pLine[0]			= &LineObj[0];
			pDevObj->pLine[1]			= &LineObj[1];

			pDevObj->pLine[0]->pDev		= pDevObj;
			pDevObj->pLine[0]->pLineObj	= &VpLineObj[0];
			pDevObj->pLine[0]->pLineCtx	= &VpLineCtx[0];

			pDevObj->pLine[1]->pDev		= pDevObj;
			pDevObj->pLine[1]->pLineObj	= &VpLineObj[1];
			pDevObj->pLine[1]->pLineCtx	= &VpLineCtx[1];

			break;

		default:
			pDevObj->max_line			= 0;
			printk("Error: unknown line type.\n");
			return pDevObj->max_line;
			break;
	}

	if (VP_DEV_886_SERIES == vpDevType || VP_DEV_887_SERIES == vpDevType ) {
		pDevObj->VpDevType			= vpDevType;
		pDevObj->pDev_profile		= DEF_LE886_DEV_PROFILE;
		pDevObj->pAC_profile		= DEF_LE886_AC_PROFILE;
		pDevObj->pDC_profile		= DEF_LE886_DC_PROFILE;
		pDevObj->pRing_profile		= DEF_LE886_RING_PROFILE;
		pDevObj->pACFxoLC_profile 	= DEF_LE886_AC_FXO_LC_PROFILE;
		pDevObj->pFxoDial_profile 	= DEF_LE886_FXO_DIALING_PROFILE;

		pDevObj->pRing_cad_usr_profile 	= LE886_RING_CAD_USER_DEF;
	}

	/* FXS */
	pDevObj->SetRingCadence 		= zarlinkSetRingCadence;
	pDevObj->SetImpedenceCountry 	= Ve886SetImpedenceCountry;
	pDevObj->GetFxsHookStatus 		= zarlinkGetFxsHookStatus;
	pDevObj->SetPcmTxOnly 			= zarlinkSetPcmTxOnly;
	pDevObj->FxsRing 				= zarlinkFxsRing;
	pDevObj->SetFxsPcmMode 			= zarlinkSetFxsPcmMode;
	pDevObj->SetFxsAcProfileByBand  = Ve886SetFxsAcProfileByBand;
	pDevObj->SetRingCadenceProfile 	= Ve886SetRingCadenceProfile;
	pDevObj->SendNTTCAR 			= zarlinkSendNTTCAR;
	pDevObj->SendNTTCAR_Check 		= zarlinkSendNTTCAR_Check;
	pDevObj->GetFxsHookStatus 		= zarlinkGetFxsHookStatus;
	pDevObj->GetLineState 			= zarlinkGetLineState;
	pDevObj->SetLineState 			= zarlinkSetLineState;
	pDevObj->SetOHT 				= zarlinkSetOHT;
	pDevObj->SetLineOpen 			= zarlinkSetLineOpen;
	pDevObj->SetPcmRxOnly 			= zarlinkSetPcmRxOnly;
	pDevObj->CheckFxsRing 			= zarlinkCheckFxsRing;
	pDevObj->TxGainAdjust 	    	= zarlinkTxGainAdjust;
	pDevObj->RxGainAdjust 	   	 	= zarlinkRxGainAdjust;
#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LINETEST)	
	pDevObj->StartLineTest 	   	 	= zarlinkLineTest;
	pDevObj->StartLineROH 	   	 	= zarlinkLineTest;
	pDevObj->StartLineVOLTAGE 	   	= zarlinkLineTest;
	pDevObj->StartLineRESFLT   	 	= zarlinkLineTest;
#endif
	pDevObj->MeteringPulse 	   	 	= Ve886StartMeter;

	/* FXO */
	pDevObj->GetFxoLineStatus 		= zarlinkGetFxoLineStatus;
	pDevObj->GetFxoHookStatus 		= zarlinkGetFxoHookStatus;
	pDevObj->SetFxoLineState 		= zarlinkSetFxoLineState;
	pDevObj->SetFxoLineOnHook 		= zarlinkSetFxoLineOnHook;
	pDevObj->SetFxoLineOffHook 		= zarlinkSetFxoLineOffHook;
	pDevObj->SetFxoLineOHT 			= zarlinkSetFxoLineOHT;
	pDevObj->SetFxoRingValidation 	= zarlinkSetFxoRingValidation;

	/* GPIO */
	pDevObj->SetIODir	 		    = Ve886SetIODir;
	pDevObj->SetIOState 		    = Ve886SetIOState;
	pDevObj->GetIOState 		    = Ve886GetIOState;
	pDevObj->UpdIOState 		    = Ve886UpdIOState;

	/* Debug */
	pDevObj->DumpDevReg 		    = zarlinkDumpDevReg;
	pDevObj->RWDevReg 		        = zarlinkRWDevReg;

	#if 0
	pDevObj->SetRingFreqAmp 		= zarlinkSetRingFreqAmp;
	#endif

	printk("move this action after init\n");
#if 0
	/* Register for event handler */
	zarlinkRegDevForEvHandle(pDevObj);
#endif
	return pDevObj->max_line;
}

/* 
** API	  : Ve886CreateLineObj()
** Desp	  :	Create Realtek line obj
** input  : ch_id 	    : unique ch_id 
**          channelId   : channel id in each device
**	        RTKLineType	: Realtek line type
**	        RTKDevObj   : point to its RTK dev obj
**	        RTKLineObj  : point to line obj
**	        slot        : tx/rx time slot
** return : SUCCESS/FAILED
*/
BOOL Ve886CreateLineObj(
	int ch_id, 
	int channelId, 	/* line# within a slic. usually 0 or 1 */
	RTKLineType	line_type,
	RTKLineObj *pLine,
	int law,
	unsigned int slot )
{
	/* channel 0 is FXS */
	pLine->ch_id 			= ch_id;
	pLine->channelId		= channelId;
	pLine->hook_st 			= 0;
	pLine->line_st 			= DEV_S_NOT_INIT;
	pLine->slot_tx 			= slot;
	pLine->slot_rx 			= slot;
	pLine->line_type		= line_type;
	pLine->pcm_law_save 	= law;
	pLine->codec_type 		= getCodecType(law);

	if (line_type == LINE_FXS) 		pLine->pAC_profile	  = DEF_LE886_AC_PROFILE;
	else if (line_type == LINE_FXO) pLine->pAC_profile	  = DEF_LE886_AC_FXO_LC_PROFILE;
	else return FAILED;

	if (line_type == LINE_FXS) 		pLine->pDCfxo_profile = DEF_LE886_DC_PROFILE;
	else if (line_type == LINE_FXO) pLine->pDCfxo_profile = DEF_LE886_FXO_DIALING_PROFILE;
	else return FAILED;

	pLine->pRing_profile	= DEF_LE886_RING_PROFILE;
	pLine->pRing_cad_profile= DEF_LE886_RING_CAD_PROFILE;
	
	pLine->TxGainAdj = 0; 	/* Adjust 0dB */
	pLine->RxGainAdj = 0; 	/* Adjust 0dB */

	PRINT_MSG("DEV%x:CH%d is %s\n", pLine->pDev->dev_id, 
		ch_id, line_type == LINE_FXS?"FXS":"FXO");

	return SUCCESS;
}	

/* 
** API	  : Ve886GetRev()
** Desp	  :	Get 886 series dev revision
** input  : Realtek device obj pointer
** return : pcn
*/
int Ve886GetRev(RTKDevObj *pDev)
{
    VpDeviceIdType deviceId = pDev->dev_id;
	Vp886DeviceObjectType	*pDevObj = pDev->pDevObj;	
    uint8 devicePcn, deviceRcn;
	int i = 0;
	unsigned char res[VP886_R_RCNPCN_LEN]={0};
    /*
     * Read revision code
     * If >= JA then force I/O as below
     * Force I/O1 to '1', and wait for (if present) the external relay to
     * open
     */

	    //VpMpiCmdWrapper(deviceId, VP886_R_EC_EC1, VP886_R_RCNPCN_RD, 
	//		VP886_R_RCNPCN_LEN, res);
#if 0
    uint8 mpiClear[] = {
        VP886_NO_OP_WRT, VP886_NO_OP_WRT, VP886_NO_OP_WRT, VP886_NO_OP_WRT, VP886_NO_OP_WRT,
        VP886_NO_OP_WRT, VP886_NO_OP_WRT, VP886_NO_OP_WRT, VP886_NO_OP_WRT, VP886_NO_OP_WRT,
        VP886_NO_OP_WRT, VP886_NO_OP_WRT, VP886_NO_OP_WRT, VP886_NO_OP_WRT, VP886_NO_OP_WRT,
        VP886_NO_OP_WRT
    };

    /*
     * First, make sure the MPI buffer is cleared so we can write to the
     * device correctly prior to HW reset.
     */
    VpMpiCmdWrapper(deviceId, VP886_EC_CH1, VP886_NO_OP_WRT, 16, mpiClear);
#endif
    /*
     * Read revision code
     * If >= JA then force I/O as below
     * Force I/O1 to '1', and wait for (if present) the external relay to
     * open
     */

#if 0
   *(( unsigned long volatile * )0xb8008004) |= (0x3<<24) ;
#endif
   
	    VpMpiCmdWrapper(deviceId, VP886_R_EC_EC1, VP886_R_RCNPCN_RD, 
			VP886_R_RCNPCN_LEN, res);
#if 0
    /* Force the revision code to at least JE to allow calibration */
    if ((res[VP886_PCN_LOCATION] == VP886_DEV_PCN_88536) &&
        (res[VP886_RCN_LOCATION] < VP886_REV_JE)) {
         res[VP886_RCN_LOCATION] = VP886_REV_JE;
    }
#endif	
    devicePcn = res[VP886_R_RCNPCN_PCN_IDX];
    deviceRcn = res[VP886_R_RCNPCN_RCN_IDX];
    printk("devicePcn = [%x], deviceRcn = [%x]]\n", devicePcn, deviceRcn);
    pDevObj->staticInfo.rcnPcn[VP886_R_RCNPCN_RCN_IDX] = deviceRcn;
    pDevObj->staticInfo.rcnPcn[VP886_R_RCNPCN_PCN_IDX] = devicePcn;
    /* MPI Failure if the PCN and RCN are both 0x00 or 0xFF */
    if (((devicePcn == 0xFF) && (deviceRcn == 0xFF)) ||
        ((devicePcn == 0x00) && (deviceRcn == 0x00))) {
        //pDevObj->status.state &= ~(VP_DEV_INIT_IN_PROGRESS | VP_DEV_INIT_CMP);

        PRINT_R("Device %x Failed to Detect Revision/PCN Properly\n",
            pDev->dev_id);

        return FAILED;
    }
#if 0
    /*
     * Verify that we recognize the device by limiting to the range of those
     * supported in the Vp886PcnType table. If not recognized (although may
     * be a valid Zarlink Semiconductor PN) return an error because the API-II
     * does not know how to handle it. More often, the error is occuring because
     * the hardware cannot talk to the device.
     */
    switch(devicePcn) {
        case VP886_DEV_PCN_88610:   /**< FXO */
        case VP886_DEV_PCN_88111:   /* FXS-Tracker */
        case VP886_DEV_PCN_88116:   /* FXS-Tracker - Wideband */
        case VP886_DEV_PCN_88131:   /* FXS-Tracker */
        case VP886_DEV_PCN_88136:   /* FXS-Tracker - Wideband */
            //pDevObj->staticInfo.maxChannels = 1;
            //maxChan = 1;
            //pDevObj->stateInt |= VP886_IS_SINGLE_CHANNEL;
            /*
             * Force the device object calibration structure to "think" that
             * the second virtual line of the device is calibrated. This allows
             * retrieval of calibration structure from VpCal().
             */
            //pDevObj->vp886SysCalData.ila40[1] = 1;
			PRINT_MSG("VP886_DEV_PCN_88111\n");
            break;

        case VP886_DEV_PCN_88211:   /* 2FXS-Tracker */
        case VP886_DEV_PCN_88216:   /* 2FXS-Tracker - Wideband */
        case VP886_DEV_PCN_88221:   /* 2FXS-ABS */
        case VP886_DEV_PCN_88226:   /* 2FXS-ABS - Wideband */
        case VP886_DEV_PCN_88231:   /* 2FXS-Tracker */
        case VP886_DEV_PCN_88236:   /* 2FXS-Tracker - Wideband */

        case VP886_DEV_PCN_88241:   /* 2FXS-ABS */
        case VP886_DEV_PCN_88246:   /* 2FXS-ABS - Wideband */

        case VP886_DEV_PCN_88311:   /* FXO/FXS-Tracker */
        case VP886_DEV_PCN_88331:   /* FXO/FXS-Tracker */

        case VP886_DEV_PCN_88506:  /* 2FXS-Tracker - Wideband Split Package*/
        case VP886_DEV_PCN_88536:   /* 2FXS-Tracker - Wideband, IP Block */
            //pDevObj->staticInfo.maxChannels = 2;
            //maxChan = 2;
            //pDevObj->stateInt &= ~VP886_IS_SINGLE_CHANNEL;
			PRINT_MSG("VP886_DEV_PCN_88221\n");
            break;
	case VP886_DEV_PCN_88601:
			PRINT_MSG("VP886_DEV_PCN_88601\n");
            break;

        default:
            //pDevObj->status.state &= ~(VP_DEV_INIT_IN_PROGRESS | VP_DEV_INIT_CMP);

            PRINT_R("Device %x Revision/PCN Unknown", deviceId); 
            return FAILED;
    }

    /* Check if device is Tracker, otherwise it's ABS or only FXO */
    if (devicePcn & VP886_TRACKER_MASK) {

        /* Mark as non-ABS device type */
        //pDevObj->stateInt &= ~VP886_IS_ABS;
		PRINT_MSG("Device is ABS\n");
    } else {
        if (devicePcn == VP886_DEV_PCN_88610) {
            /* FXO only devices */
            //pDevObj->stateInt |= VP886_IS_FXO_ONLY;
			PRINT_MSG("Device is FXO only device\n");
        } else {
            /* Last choice is ABS type */
            //pDevObj->stateInt |= VP886_IS_ABS;
			PRINT_MSG("Device is ABS\n");
        }
    }
#endif	
	return devicePcn;
}

/*********** End of File ***************/

