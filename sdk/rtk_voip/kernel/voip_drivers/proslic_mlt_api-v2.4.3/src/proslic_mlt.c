/*
** Copyright (c) 2009-2015 by Silicon Laboratories
**
** $Id: proslic_mlt.c 4714 2015-02-05 18:16:07Z nizajerk $
**
*/
/*! \file proslic_mlt.c
**  \brief ProSLIC MLT API implementation file
**
**  This is the implementation of the ProSLIC API function prototypes
**
**  \author Silicon Laboratories, Inc (laj,cdp)
**
** \attention
** This file contains proprietary information.	 
** No dissemination allowed without prior written permission from
** Silicon Laboratories, Inc.
**
*/

#include "si_voice_datatypes.h"
#include "si_voice_ctrl.h"
#include "si_voice_timer_intf.h"
#include "proslic.h"
#define SI_USE_IFACE_MACROS
#include "proslic_mlt.h"


#ifdef SI3217X
#include "si3217x_mlt.h"
#endif
#ifdef SI3226X
#include "si3226x_mlt.h"
#endif

#if (MLT_POLL_RATE >10) || (MLT_POLL_RATE < 2)
#error "MLT_POLL_RATE setting requirements not met - needs to be between 2 and 10 mSec"
#endif

/*
** ProSLIC MLT State Initialization Functions - documented in proslic_mlt.h
*/
/* Foreign/Hazardous Voltages */
int ProSLIC_mlt_init_foreign_voltages(ProSLIC_mlt_foreign_voltages_state *pState,int samples)
{
    pState->State.stage = 0;
    pState->samples = samples;
    return RC_MLT_NO_ERROR;
}
/* Receiver Offhook */
int ProSLIC_mlt_init_receiver_offhook(ProSLIC_mlt_roh_state *pState)
{
    pState->State.stage = 0;
    return RC_MLT_NO_ERROR;
}
/* REN */
int ProSLIC_mlt_init_ren(ProSLIC_mlt_ren_state *pState)
{
    pState->State.stage = 0;
    return RC_MLT_NO_ERROR;
}
/* Resistive Faults */
int ProSLIC_mlt_init_resistive_faults(ProSLIC_mlt_rmeas_state *pState)
{
    pState->TRState.stage = 0;
    pState->TGState.stage = 0;
    pState->RGState.stage = 0;
    pState->setupState.stage = 0;
    return RC_MLT_NO_ERROR;
}

/* Capacitance */
int ProSLIC_mlt_init_capacitance(ProSLIC_mlt_capacitance_state *pState)
{
    pState->State.stage = 0;
    return RC_MLT_NO_ERROR;
}

/*
** ProSLIC MLT Functions - documented in proslic_mlt.h
*/

/* Foreign/Hazardous Voltages */
int ProSLIC_mlt_foreign_voltages(ProSLICMLTType *pProSLICMLT, ProSLIC_mlt_foreign_voltages_state *pState)
{
    int returnCode = RC_MLT_UNSUPPORTED_TEST;
#ifdef SI324X
    returnCode = si324x_mlt_foreign_voltages(pProSLICMLT,pState);
#endif
#ifdef SI321X
    returnCode = si321x_mlt_foreign_voltages(pProSLICMLT,pState);
#endif
#ifdef SI322X
    returnCode = si3226_mlt_foreign_voltages(pProSLICMLT,pState);
#endif
#ifdef SI3217X
    if(MLT_VALID_SI3217X_DEVICE)
        returnCode = si3217x_mlt_foreign_voltages(pProSLICMLT,pState);
#endif
#ifdef SI3226X
    if(MLT_VALID_SI3226X_DEVICE)
        returnCode = si3226x_mlt_foreign_voltages(pProSLICMLT,pState);
#endif
	return returnCode;
}

/* Foreign/Hazardous Voltages */
int ProSLIC_mlt_hazard_voltages(ProSLICMLTType *pProSLICMLT, ProSLIC_mlt_foreign_voltages_state *pState)
{
    int returnCode = RC_MLT_UNSUPPORTED_TEST;
#ifdef SI324X
    returnCode = si324x_mlt_foreign_voltages(pProSLICMLT,pState);
#endif
#ifdef SI321X
    returnCode = si321x_mlt_foreign_voltages(pProSLICMLT,pState);
#endif
#ifdef SI322X
    returnCode = si3226_mlt_foreign_voltages(pProSLICMLT,pState);
#endif
#ifdef SI3217X
    if(MLT_VALID_SI3217X_DEVICE)
        returnCode = si3217x_mlt_foreign_voltages(pProSLICMLT,pState);
#endif
#ifdef SI3226X
    if(MLT_VALID_SI3226X_DEVICE)
        returnCode = si3226x_mlt_foreign_voltages(pProSLICMLT,pState);
#endif
	return returnCode;
}


/* Resistive Faults */
int ProSLIC_mlt_resistive_faults(ProSLICMLTType *pProSLICMLT, ProSLIC_mlt_rmeas_state *pState)
{
    int returnCode = RC_MLT_UNSUPPORTED_TEST;
#ifdef SI324X
    returnCode = si324x_mlt_resistive_faults(pProSLICMLT,pState);
#endif
#ifdef SI321X
    returnCode = si321x_mlt_resistive_faults(pProSLICMLT,pState);
#endif
#ifdef SI322X
    returnCode = si3226_mlt_resistive_faults(pProSLICMLT,pState);
#endif
#ifdef SI3217X
    if(MLT_VALID_SI3217X_DEVICE)
        returnCode = si3217x_mlt_resistive_faults(pProSLICMLT,pState);
#endif
#ifdef SI3226X
    if(MLT_VALID_SI3226X_DEVICE)
        returnCode = si3226x_mlt_resistive_faults(pProSLICMLT,pState);
#endif
	return returnCode;
}

/* Receiver Offhook */
int ProSLIC_mlt_receiver_offhook(ProSLICMLTType *pProSLICMLT,ProSLIC_mlt_roh_state *pState)
{
    int returnCode = RC_MLT_UNSUPPORTED_TEST;
#ifdef SI324X
    returnCode = si324x_mlt_receiver_offhook(pProSLICMLT,pState);
#endif
#ifdef SI321X
    returnCode = si321x_mlt_receiver_offhook(pProSLICMLT,pState);
#endif
#ifdef SI322X
    returnCode = si3226_mlt_receiver_offhook(pProSLICMLT,pState);
#endif
#ifdef SI3217X
    if(MLT_VALID_SI3217X_DEVICE)
        returnCode = si3217x_mlt_receiver_offhook(pProSLICMLT,pState);
#endif
#ifdef SI3226X
    if(MLT_VALID_SI3226X_DEVICE)
        returnCode = si3226x_mlt_receiver_offhook(pProSLICMLT,pState);
#endif
	return returnCode;
}

/* REN */
int ProSLIC_mlt_ren(ProSLICMLTType *pProSLICMLT,ProSLIC_mlt_ren_state *pState)
{
    int returnCode = RC_MLT_UNSUPPORTED_TEST;
#ifdef SI324X
    returnCode = si324x_mlt_ren(pProSLICMLT,pState);
#endif
#ifdef SI321X
    returnCode = si321x_mlt_ren(pProSLICMLT,pState);
#endif
#ifdef SI322X
    returnCode = si3226_mlt_ren(pProSLICMLT,pState);
#endif
#ifdef SI3217X
    if(MLT_VALID_SI3217X_DEVICE)
        returnCode = si3217x_mlt_ren(pProSLICMLT,pState);
#endif
#ifdef SI3226X
    if(MLT_VALID_SI3226X_DEVICE)
        returnCode = si3226x_mlt_ren(pProSLICMLT,pState);
#endif
	return returnCode;
}


/* Capacitance */
int ProSLIC_mlt_capacitance(ProSLICMLTType *pProSLICMLT,ProSLIC_mlt_capacitance_state *pState)
{
    int returnCode = RC_MLT_UNSUPPORTED_TEST;

#ifdef SI3217X
    if(MLT_VALID_SI3217X_DEVICE)
        returnCode = si3217x_mlt_capacitance(pProSLICMLT,pState);
#endif
#ifdef SI3226X
    if(MLT_VALID_SI3226X_DEVICE)
        returnCode = si3226x_mlt_capacitance(pProSLICMLT,pState);
#endif

	return returnCode;
}

/* REN Capacitance */
int ProSLIC_mlt_ren_cap(ProSLICMLTType *pProSLICMLT,ProSLIC_mlt_ren_cap_state *pState)
{
    int returnCode = RC_MLT_UNSUPPORTED_TEST;

#ifdef SI3217X
    if(MLT_VALID_SI3217X_DEVICE)
        returnCode = si3217x_mlt_ren_cap(pProSLICMLT,pState);
#endif
#ifdef SI3226X
    if(MLT_VALID_SI3226X_DEVICE)
        returnCode = si3226x_mlt_ren_cap(pProSLICMLT,pState);
#endif
	return returnCode;
}


/* Abort */
int ProSLIC_mlt_abort (ProSLICMLTType *pProSLICMLT,ProSLIC_mlt_test_state *pState)
{
    int returnCode = RC_MLT_UNSUPPORTED_TEST;
#ifdef SI324X
    returnCode = si324x_mlt_abort(pProSLICMLT,pState);
#endif
#ifdef SI321X
    returnCode = si321x_mlt_abort(pProSLICMLT,pState);
#endif
#ifdef SI322X
    returnCode = si3226_mlt_abort(pProSLICMLT,pState);
#endif
#ifdef SI3217X
    if(MLT_VALID_SI3217X_DEVICE)
        returnCode = si3217x_mlt_abort(pProSLICMLT,pState);
#endif
#ifdef SI3226X
    if(MLT_VALID_SI3226X_DEVICE)
        returnCode = si3226x_mlt_abort(pProSLICMLT,pState);
#endif
	return returnCode;
}


/* Initialize REN calibration constants */
int ProSLIC_mlt_init_ren_cal (ProSLICMLTType *pProSLICMLT)
{
    if(pProSLICMLT != NULL)
    {
        pProSLICMLT->ren.calData.extraLowRenOffset = MLT_RENCAL_ELO_REN_OFFS;
        pProSLICMLT->ren.calData.extraLowRenSlope  = MLT_RENCAL_ELO_REN_SLOPE;
        pProSLICMLT->ren.calData.highRenOffs = MLT_RENCAL_HI_REN_OFFS;
        pProSLICMLT->ren.calData.highRenSlope = MLT_RENCAL_HI_REN_SLOPE;
        pProSLICMLT->ren.calData.lowRenOffs = MLT_RENCAL_LO_REN_OFFS;
        pProSLICMLT->ren.calData.lowRenSlope = MLT_RENCAL_LO_REN_SLOPE;
        pProSLICMLT->ren.calData.renTrans = MLT_RENCAL_REN_TRANS;
        return RC_MLT_NO_ERROR;
    }
	return RC_MLT_BAD_PARAM;
}

#ifdef MLT_RUNTIME_CONFIG
/* Initialize MLT configuration from data structure */
int ProSLIC_mlt_init_default_config (ProSLICMLTType *pProSLICMLT)
{
    if(pProSLICMLT != NULL)
    {
        pProSLICMLT->mlt_config.mlt_ext_v_flag      = 0;
        pProSLICMLT->mlt_config.mlt_poll_rate                 = MLT_POLL_RATE;
        pProSLICMLT->mlt_config.mlt_prot_res                  = MLT_PROTECTION_RESISTANCE;
        pProSLICMLT->mlt_config.mlt_line_cap                  = MLT_LINE_CAPACITANCE;
        pProSLICMLT->mlt_config.mlt_emi_cap                   = MLT_EMI_CAPACITANCE;
        pProSLICMLT->mlt_config.mlt_femf_samples              = MLT_FEMF_SAMPLES;
        pProSLICMLT->ren.calData.extraLowRenOffset  = MLT_RENCAL_ELO_REN_OFFS;
        pProSLICMLT->ren.calData.extraLowRenSlope   = MLT_RENCAL_ELO_REN_SLOPE;
        pProSLICMLT->ren.calData.highRenOffs        = MLT_RENCAL_HI_REN_OFFS;
        pProSLICMLT->ren.calData.highRenSlope       = MLT_RENCAL_HI_REN_SLOPE;
        pProSLICMLT->ren.calData.lowRenOffs         = MLT_RENCAL_LO_REN_OFFS;
        pProSLICMLT->ren.calData.lowRenSlope        = MLT_RENCAL_LO_REN_SLOPE;
        pProSLICMLT->ren.calData.renTrans           = MLT_RENCAL_REN_TRANS;
        return RC_MLT_NO_ERROR;
    }
	return RC_MLT_BAD_PARAM;
}

#endif

/* Initialize REN calibration constants with passed rencal structure */
int ProSLIC_mlt_init_ren_cal_multichan (ProSLICMLTType *pProSLICMLT, ProSLIC_rencal_t *ren_cal_data )
{
    if(pProSLICMLT != NULL)
    {
        pProSLICMLT->ren.calData.extraLowRenOffset = ren_cal_data[pProSLICMLT->pProslic->channel].extraLowRenOffset;
        pProSLICMLT->ren.calData.extraLowRenSlope  = ren_cal_data[pProSLICMLT->pProslic->channel].extraLowRenSlope;
        pProSLICMLT->ren.calData.highRenOffs = ren_cal_data[pProSLICMLT->pProslic->channel].highRenOffs;
        pProSLICMLT->ren.calData.highRenSlope = ren_cal_data[pProSLICMLT->pProslic->channel].highRenSlope;
        pProSLICMLT->ren.calData.lowRenOffs = ren_cal_data[pProSLICMLT->pProslic->channel].lowRenOffs;
        pProSLICMLT->ren.calData.lowRenSlope = ren_cal_data[pProSLICMLT->pProslic->channel].lowRenSlope;
        pProSLICMLT->ren.calData.renTrans = ren_cal_data[pProSLICMLT->pProslic->channel].renTrans;
        return RC_MLT_NO_ERROR;
    }
	return RC_MLT_BAD_PARAM;
}

/* Copy passed MLT structure's calibration constants to passed rencal struct */
int ProSLIC_mlt_store_ren_cal_multichan (ProSLICMLTType *pProSLICMLT, ProSLIC_rencal_t *ren_cal_data )
{
    if(pProSLICMLT != NULL)
    {
        ren_cal_data[pProSLICMLT->pProslic->channel].extraLowRenOffset = pProSLICMLT->ren.calData.extraLowRenOffset;
		ren_cal_data[pProSLICMLT->pProslic->channel].extraLowRenSlope = pProSLICMLT->ren.calData.extraLowRenSlope;
		ren_cal_data[pProSLICMLT->pProslic->channel].highRenOffs = pProSLICMLT->ren.calData.highRenOffs;
		ren_cal_data[pProSLICMLT->pProslic->channel].highRenSlope = pProSLICMLT->ren.calData.highRenSlope;
		ren_cal_data[pProSLICMLT->pProslic->channel].lowRenOffs = pProSLICMLT->ren.calData.lowRenOffs;
		ren_cal_data[pProSLICMLT->pProslic->channel].lowRenSlope = pProSLICMLT->ren.calData.lowRenSlope;
		ren_cal_data[pProSLICMLT->pProslic->channel].renTrans = pProSLICMLT->ren.calData.renTrans;
        return RC_MLT_NO_ERROR;
    }
	else
		return RC_MLT_BAD_PARAM;
}


/*
** MLT Test Data and Flag Management
**
*/

/*
** ProSLIC_mlt_clear_results - documented in proslic_mlt.h
*/ 
int ProSLIC_mlt_clear_results(ProSLICMLTType *pProSLICMLT)
{
	/* Hazardous/Foriegn Voltages Test */
	pProSLICMLT->hazVAC.measTG = 0;
	pProSLICMLT->hazVAC.measTR = 0;
	pProSLICMLT->hazVAC.measRG = 0;
	pProSLICMLT->hazVAC.resultsValid = 0;
	pProSLICMLT->hazVDC.measTG = 0;
	pProSLICMLT->hazVDC.measTR = 0;
	pProSLICMLT->hazVDC.measRG = 0;
	pProSLICMLT->hazVDC.resultsValid = 0;
	
	/* Resistive Faults Test */
	pProSLICMLT->resFaults.measTG = 20000000L;
	pProSLICMLT->resFaults.measTR = 20000000L;
	pProSLICMLT->resFaults.measRG = 20000000L;
	pProSLICMLT->resFaults.ahsFlag = 0;
	pProSLICMLT->resFaults.resultsValid = 0;

	/* Receiver Offhook Test */
	pProSLICMLT->roh.rohTrue = RC_MLT_ROH_NOFAULT;
    pProSLICMLT->roh.resultsValid = 0;

	/* REN Test */
	pProSLICMLT->ren.renValue = 0;
    pProSLICMLT->ren.resultsValid = 0;

	/* Capacitance Test */
	pProSLICMLT->capFaults.measTG = 0;
	pProSLICMLT->capFaults.measTR = 0;
	pProSLICMLT->capFaults.measRG = 0;
	pProSLICMLT->capFaults.resultsValid = 0;

	return RC_MLT_TEST_COMPLETE;
}

/*
** ProSLIC_mlt_version - documented in proslic_mlt.h
*/
extern const char *ProSLICMLTAPIVersion;
const char *ProSLIC_mlt_version(void)
{
	return (const char *)ProSLICMLTAPIVersion;
}

