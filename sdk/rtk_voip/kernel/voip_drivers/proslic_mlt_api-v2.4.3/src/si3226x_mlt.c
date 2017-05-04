/*
** Copyright (c) 2010-2014 by Silicon Laboratories
**
** $Id: si3226x_mlt.c 4552 2014-10-28 21:55:37Z nizajerk $
**
*/
/*! \file si3226x_mlt.c
**  \brief Si3226x ProSLIC MLT interface implementation file
**
** \author Silicon Laboratories, Inc (cdp)
**
** \attention
** This file contains proprietary information.	 
** No dissemination allowed without prior written permission from
** Silicon Laboratories, Inc.
** This code may produce different results on different host processors 
** 
*/

#include "si_voice_datatypes.h"
#include "si_voice_ctrl.h"
#include "si_voice_timer_intf.h"
#include "proslic.h"
#define SI_USE_IFACE_MACROS
#include "proslic_mlt.h"
#include "proslic_mlt_math.h"
#include "proslic_mlt_diag_madc.h"
#include "proslic_mlt_dcfeed.h"

#include "si3226x.h"
#include "si3226x_intf.h"
#include "si3226x_mlt.h"
#include "mlt_comm_regs.h"
#include "mlt26x_c_regs.h"

/*******************************************************/

/*
** Datalogging Macro
*/
#ifdef ENABLE_DEBUG
#define MLT_DEBUG_LOG(...) \
    if(pProSLICMLT->pProslic->debugMode) {\
    LOGPRINT(__VA_ARGS__);\
    }
#else
#define MLT_DEBUG_LOG(...)  do {} while(0)
#endif

/*
** MLT Specific Patch RAM Locations
*/
#define MLT26X_C_PRAM_VTR3              352
#define MLT26X_C_PRAM_STOP_TIMER3       333
#define MLT26X_C_PRAM_STOP_TIMER3_VAL   334


/*
** List of RAM Addresses to be stored before each MLT test is executed
*/
static const uInt16 si3226x_preserve_RAM[] = {
    MLT_COMM_RAM_SLOPE_VLIM,
    MLT_COMM_RAM_SLOPE_RFEED,
    MLT_COMM_RAM_SLOPE_ILIM,
    MLT_COMM_RAM_SLOPE_RING,
    MLT_COMM_RAM_SLOPE_DELTA1,
    MLT_COMM_RAM_SLOPE_DELTA2,
    MLT_COMM_RAM_V_VLIM,
    MLT_COMM_RAM_V_RFEED,
    MLT_COMM_RAM_V_ILIM,
    MLT_COMM_RAM_CONST_RFEED,
    MLT_COMM_RAM_CONST_ILIM,
    MLT_COMM_RAM_I_VLIM,
    MLT_COMM_RAM_VCM_OH,
    MLT_COMM_RAM_VOV_BAT,
    MLT_COMM_RAM_VOV_GND,
    MLT_COMM_RAM_P_TH_HVIC,
    MLT_COMM_RAM_VCMDAC_SCALE_MAN,
    MLT_COMM_RAM_LCROFFHK,
    MLT_COMM_RAM_LCRONHK,
    MLT_COMM_RAM_RINGAMP,
    MLT_COMM_RAM_RINGOF,
    MLT_COMM_RAM_RTPER,
    MLT_COMM_RAM_RINGFR,
    MLT_COMM_RAM_RINGPHAS,
    MLT_COMM_RAM_VCM_RING,
    MLT_COMM_RAM_COUNTER_VTR_VAL,
    MLT_COMM_RAM_LONGHITH,
    MLT_COMM_RAM_LONGLOTH,
    MLT_COMM_RAM_RA_EN_B,
    MLT_COMM_RAM_AUDIO_MAN,
    MLT_COMM_RAM_PD_BIAS,
    MLT_COMM_RAM_PD_DC_BUF,
    MLT_COMM_RAM_PD_AC_ADC,
    MLT_COMM_RAM_PD_AC_DAC,
    MLT_COMM_RAM_PD_AC_SNS,
    MLT_COMM_RAM_PD_CM,
    MLT_COMM_RAM_PD_DC_COARSE_SNS,
    MLT_COMM_RAM_LKG_UPT_ACTIVE,
    MLT_COMM_RAM_LKG_UPR_ACTIVE,
    MLT_COMM_RAM_LKG_DNT_ACTIVE,
    MLT_COMM_RAM_LKG_DNR_ACTIVE,
    MLT26X_C_RAM_LKG_UPT_OHT,
    MLT26X_C_RAM_LKG_UPR_OHT,
    MLT26X_C_RAM_LKG_DNT_OHT,
    MLT26X_C_RAM_LKG_DNR_OHT,
    MLT_COMM_RAM_COMP_Z,
    MLT_COMM_RAM_VBATR_EXPECT,
    MLT_COMM_RAM_VBATH_EXPECT,
    MLT_COMM_RAM_VOV_RING_GND,
    0   /* NULL TERMINATE */
};

static const uInt8 si3226x_preserve_Reg[] = {
    MLT_COMM_REG_LINEFEED,
    MLT_COMM_REG_PDN,
    MLT_COMM_REG_RINGCON,
    MLT_COMM_REG_AUTO,
    MLT_COMM_REG_IRQEN1,
    MLT_COMM_REG_IRQEN2,
    MLT_COMM_REG_IRQEN3,
    MLT_COMM_REG_IRQEN4,
    MLT_COMM_REG_ENHANCE,
    MLT_COMM_REG_DIGCON,
    MLT_COMM_REG_GPIO,
    MLT_COMM_REG_GPIO_CFG1,
    MLT_COMM_REG_GPIO_CFG2,
    MLT_COMM_REG_GPIO_CFG3,
    MLT_COMM_REG_DIAG1,
    MLT_COMM_REG_DIAG2,
    MLT26X_C_REG_DIAG3,
	MLT_COMM_REG_RA,
    0   /* NULL TERMINATE */
};


/**
 * @internal @defgroup SI3226X_DRV_STATIC Si3226x Driver Level Static APIs
 * These functions are used by the Si3226x driver and never called from a higher level routine
 *
 *@{
 */
/********************************************************************************/
/**
** @brief This routine controls the USER MODE status
** 
** @param[in] *pProslic -  ptr to ProSLIC channel structure
** @param[in] on - turn usermode on (TRUE) or off (FALSE)
**
** @retval int -  @ref RC_MLT_NO_ERROR for success.
**
** @todo Define return code if semaphore not available
**
** @remark
** This function is redefined here since it was defined as a static function
** in the driver.  This may need to change.
*/
static int setUserMode (proslicChanType *pProslic,BOOLEAN on){
	uInt8 data;

    /* Workaround for non-mutexed applications */
    if(on == FALSE)
        return 0;
    /* End workaround */

	data = ReadReg(pProHW,pProslic->channel,126);
	if (((data&1) != 0) == on)
		return 0;
	WriteReg(pProHW,pProslic->channel,126,2);
	WriteReg(pProHW,pProslic->channel,126,8);
	WriteReg(pProHW,pProslic->channel,126,0xe);
	WriteReg(pProHW,pProslic->channel,126,0);
	return 0;
}

/********************************************************************************/
/**
** @brief Delay function utilizing reentry
** 
** @param[in] *pState - MLT state structure
** @param[in] delayCount - number of reentries or polling cycles
**
** @retval void
**
** @todo Implement poll rate adjustment in this function
*/
static void delay_poll (ProSLIC_mlt_test_state *pState,uInt16 delayCount){
	pState->waitIterations++;	
	if ((pState->waitIterations == delayCount) || (delayCount == 0)){
		pState->waitIterations=0;
		pState->stage++;
	}	
}
 
/**
** @brief Stores select RAM and register values defined by 
** si3226x_preserve_RAM and si3226x_preserve_RAM arrays.
** 
** @param[in,out] *pProSLICMLT -  Pointer to MLT channel structure
**
** @retval void
**
*/
static void si3226x_preserve_state (ProSLICMLTType *pProSLICMLT){
int i;

    i=0;
    while(si3226x_preserve_RAM[i] != 0)
    {
        pProSLICMLT->preserveRAM[i].address = si3226x_preserve_RAM[i];
        pProSLICMLT->preserveRAM[i].initValue = pProSLICMLT->ReadRAM(pMLT_HW, pMLT_CHAN,si3226x_preserve_RAM[i]);
        i++;
    }
    pProSLICMLT->preserveRAM[i].address = 0;   /* Null Terminate */

    i=0;
    while(si3226x_preserve_Reg[i] != 0)
    {
        pProSLICMLT->preserveReg[i].address = si3226x_preserve_Reg[i];
        pProSLICMLT->preserveReg[i].initValue = pProSLICMLT->ReadReg(pMLT_HW, pMLT_CHAN,si3226x_preserve_Reg[i]);
        i++;
    }    
    pProSLICMLT->preserveReg[i].address = 0;   /* Null Terminate */
}

/********************************************************************************/ 
/**
** @brief Restores RAM and register data defined by si3226x_preserve_RAM and 
** si3226x_preserve_REG array of addresses
** 
** @param[in] *pProSLICMLT -  Pointer to MLT channel structure
**
** @retval void
**
*/
static void si3226x_restore_state (ProSLICMLTType *pProSLICMLT){
int i;

    i=0;
    while(pProSLICMLT->preserveRAM[i].address != 0)
    {
        pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN,pProSLICMLT->preserveRAM[i].address,pProSLICMLT->preserveRAM[i].initValue);
        i++;
    }

    i=0;
    while(pProSLICMLT->preserveReg[i].address != 0)
    {
        pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN,pProSLICMLT->preserveReg[i].address,pProSLICMLT->preserveReg[i].initValue);
        i++;
    }    
}


/********************************************************************************/ 
/**
** @brief Setup DSP Timers
** 
** @param[in] *pProslic    -  Pointer to channel object
** @param[in] timer1       -  Timer 1 preset (usec)
** @param[in] timer2       -  Timer 2 preset (usec)
** @param[in] timer3       -  Timer 3 preset (usec)
**
** @retval void
**
*/
static void setupDspTimers(proslicChanType_ptr pProslic, int timer1, int timer2, int timer3)
{
int32 t1;
int32 t2;
int32 t3;
uInt8 regTmp;

    t1 = timer1*MLT_CONST_DSP_TIMER_SCALE;
    t2 = timer2*MLT_CONST_DSP_TIMER_SCALE;
    t3 = timer3*MLT_CONST_DSP_TIMER_SCALE;

    regTmp = ReadReg(pProHW,pProslic->channel,MLT26X_C_REG_DIAG3);
    WriteReg(pProHW,pProslic->channel,MLT26X_C_REG_DIAG3,regTmp&0xFD);
    WriteRAM(pProHW,pProslic->channel,MLT26X_C_PRAM_STOP_TIMER3_VAL, t3);
    WriteRAM(pProHW,pProslic->channel,MLT26X_C_PRAM_STOP_TIMER3,  t3);
    WriteRAM(pProHW,pProslic->channel,MLT26X_C_RAM_STOP_TIMER2_VAL, t2);
    WriteRAM(pProHW,pProslic->channel,MLT26X_C_RAM_STOP_TIMER2,  t2);
    WriteRAM(pProHW,pProslic->channel,MLT26X_C_RAM_STOP_TIMER1_VAL,  t1);
    WriteRAM(pProHW,pProslic->channel,MLT26X_C_RAM_STOP_TIMER1, t1);

}

static void armDspTimers(proslicChanType_ptr pProslic)
{
uInt8 regTmp;

    regTmp = ReadReg(pProHW,pProslic->channel,MLT26X_C_REG_DIAG3);
    WriteReg(pProHW,pProslic->channel,MLT26X_C_REG_DIAG3,regTmp|0x02); 
}

static void readDspTimerV(proslicChanType_ptr pProslic, int32 *v1, int32 *v2, int32 *v3)
{
    *v1 = ReadRAM(pProHW,pProslic->channel,MLT26X_C_RAM_VTR1);
	if (*v1 & 0x10000000L)
		*v1 |= 0xF0000000L;
    *v2 = ReadRAM(pProHW,pProslic->channel,MLT26X_C_RAM_VTR2);
	if (*v2 & 0x10000000L)
		*v2 |= 0xF0000000L;

    *v3 = ReadRAM(pProHW,pProslic->channel,MLT26X_C_PRAM_VTR3);
	if (*v3 & 0x10000000L)
		*v3 |= 0xF0000000L;

    *v1 /= MLT_CONST_DSP_TIMER_V_SCALE;
    *v2 /= MLT_CONST_DSP_TIMER_V_SCALE;
    *v3 /= MLT_CONST_DSP_TIMER_V_SCALE;
}


static void gndOpenTerm(proslicChanType_ptr pProslic, int gndOn)
{
    /* Presumed usermode is on */
    if(gndOn) 
    {
        WriteRAM(pProHW,pProslic->channel,MLT_COMM_RAM_HVIC_STATE, 0x2F000L); 
        WriteRAM(pProHW,pProslic->channel,MLT_COMM_RAM_HVIC_STATE_MAN, 0x10000000L);     
        WriteRAM(pProHW,pProslic->channel,MLT_COMM_RAM_PD_CM, 0x300000L);     
    }
    else 
    {
        WriteRAM(pProHW,pProslic->channel,MLT_COMM_RAM_PD_CM, 0x0L);  
        WriteRAM(pProHW,pProslic->channel,MLT_COMM_RAM_HVIC_STATE_MAN, 0x0L);  
    }
}
/********************************************************************************/
/**
** @brief Calculation of capacitance from TESTAVO reading
** 
** @param[in] *pProSLICMLT -  Pointer to MLT channel structure
** @param[in] *pState - pointer to test state structure
** @param[in] term - measured terminal
**
** @retval tmp - computed capacitance
**
** @remark 
**  - This function is reentrant and should be called at periodic intervals defined by mlt_poll_rate
**  - USER MODE status is not modified in this function
**  - This function does not modify the linefeed state or audio path
**  - This function measures testavo w/ 300Hz source applied and remeasures with 3014Hz source
**    if measured testavo reading is greater the MLT_MAX_300HZ_TESTAVO
**
*/
static int32 processTestavoForC(ProSLICMLTType *pProSLICMLT,ProSLIC_mlt_capacitance_state *pState, int term){
uInt32 tmp;
uInt32 zScale;
uInt32 rp_mult;
uInt32 r_bom_comp;
uInt32 c_bom_comp;
uInt32 mlt_prot_res;
uInt32 mlt_line_cap;
uInt32 mlt_emi_cap;
#ifdef MLT_RUNTIME_CONFIG
    mlt_prot_res = pProSLICMLT->mlt_config.mlt_prot_res;
    mlt_line_cap = pProSLICMLT->mlt_config.mlt_line_cap;
    mlt_emi_cap  = pProSLICMLT->mlt_config.mlt_emi_cap;
#else
    mlt_prot_res = MLT_PROTECTION_RESISTANCE;
    mlt_line_cap = MLT_LINE_CAPACITANCE;
    mlt_emi_cap  = MLT_EMI_CAPACITANCE;
#endif


	MLT_DEBUG_LOG("si3226x_mlt : processTestavoForC : TESTAVO = %d\n", pState->testavo);


    if((term == MLT_TERM_TG)||(term == MLT_TERM_RG))
    {
        rp_mult = 10L; /* 1^2 = 1 */
        r_bom_comp = SI3226X_MLT_HVIC_SWITCH_RES + mlt_prot_res;
        c_bom_comp = mlt_line_cap + mlt_emi_cap;
    }
    else
    {
        rp_mult = 40L; /* 2^2 = 4 */
        r_bom_comp = mlt_prot_res;
        c_bom_comp = (mlt_line_cap/2) + mlt_emi_cap;
    }
            
    if(pState->freq == 300)
        zScale = MLT_AUDIAG_SCALE_300HZ;
    else
        zScale = MLT_AUDIAG_SCALE_3014HZ;

    if (pState->testavo > 310000L || (pState->freq == 3014)) /* Use Eq. 2 or 3 */
    {
        if ((pState->testavo > 500000L) && (pState->freq == 3014))  /* Use Eq. 3 */
        {
            tmp = (zScale * (pState->testavo / 100L));

            MLT_DEBUG_LOG("si3226x_mlt : processTestavoForC : Method EQ 3\n");

            tmp = (tmp / 100000L) * (tmp / 100000L);          /* Zmag^2 */
            tmp = (MLT_RS_CAPACITANCE_TEST / (tmp / 100L))*100L;  /* (Rs^2/Zmag^2) */
            tmp = tmp - 10000L;                               /* (Rs^2/Zmag^2) - 1 */
            tmp = Isqrt(((tmp / 100L) * 16384L) / 100L);      /*  16384 and 1179 are part of a truncation minimization */
            tmp = (tmp * 1000L) / (1179L);                    /*  minito implement 1/2*pi*f*Rs, or 1/92108312 */  
        
        } 
        else    /* Use Eq. 2 */ 
        {
            MLT_DEBUG_LOG("si3226x_mlt : processTestavoForC : Method EQ 2\n");
            tmp = ((628L * pState->freq) / 100L * zScale) / 100000L;
            tmp = tmp * (pState->testavo / 1000L);
            tmp = (1000000000L / (tmp / 100L)) / 10L;  
            tmp = (tmp / 10L);	/* - (LINE_CAPACITANCE/2); */ 
        }
    } 
    else /* Use Eq. 4 - large C */
    {
        MLT_DEBUG_LOG("si3226x_mlt : processTestavoForC : Method EQ 4\n");
        tmp = (zScale * (pState->testavo/10L));
        tmp = (tmp / 10000L) * (tmp / 10000L); 
        tmp = tmp / 10L - rp_mult * (r_bom_comp*r_bom_comp);
        tmp = Isqrt((tmp*10L) / 39L);   /* 10/39 ~ 256/10000 */
        tmp = ((tmp * pState->freq) / 16L) * 628L;
        tmp = 1000000000L / (tmp / 1000L);
    }



    /* Compensate for bom & emi caps */

    tmp -= c_bom_comp;

	MLT_DEBUG_LOG("si3226x_mlt : processTestavoForC : adjC(%d) = %d\n", pState->freq,tmp);

    return tmp;
} 
/**@}*/

/********************************************************************************/
/**
** @brief Implementation of capacitance measurement
** 
** @param[in,out] *pProSLICMLT -  Pointer to MLT channel structure.  Output is stored in  pState->testavo - diag power ac voltage measurement
** @param[in] *pState - pointer to test state structure
**I
** @retval typical values: @ref RC_MLT_TEST_RUNNING or @ref RC_MLT_TEST_COMPLETE are normal values returned. @sa MLT_RETURN_CODES for other possible values.
**
** @remark 
**  - This function is reentrant and should be called at periodic intervals defined by mlt_poll_rate
**  - USER MODE status is not modified in this function
**  - This function does not modify the linefeed state or audio path
**  - This function measures testavo w/ 300Hz source applied and remeasures with 3014Hz source
**    if measured testavo reading is greater the MLT_MAX_300HZ_TESTAVO
**
*/
static int measCapacitanceAC(ProSLICMLTType * pProSLICMLT, ProSLIC_mlt_capacitance_state * pState)
{
uInt32 data;
uInt16 mlt_poll_rate;
#ifdef MLT_RUNTIME_CONFIG
    mlt_poll_rate = pProSLICMLT->mlt_config.mlt_poll_rate;
#else
    mlt_poll_rate = MLT_POLL_RATE;
#endif
    switch (pState->MeasState.stage)
    {
        case 0:
            pState->freq = 300;
            pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN, MLT_COMM_REG_DIAG1, 0xB);	/*disable testfilter */
            pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN, MLT_COMM_REG_OCON, 0x0);	/*disable osc1 */
            setup300HzBandpass(pProSLICMLT);
            setup300HzTestTone(pProSLICMLT);
            pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN, MLT_COMM_REG_DIAG1, 0x1B);	/*enable testfilter */
            pState->MeasState.stage++;
            pState->MeasState.waitIterations = 0;
            return RC_MLT_TEST_RUNNING;

        case 1:
            delay_poll(&pState->MeasState, 900 / mlt_poll_rate - 2);
            return RC_MLT_TEST_RUNNING;

        case 2:
            data = pProSLICMLT->ReadRAM(pMLT_HW, pMLT_CHAN, MLT_COMM_RAM_TESTABO);
            pState->testavo = data;
   
            if (pState->testavo <= MLT_MAX_300HZ_TESTAVO)
            {
                return RC_MLT_TEST_COMPLETE;
            }
            pState->MeasState.stage++;
            pState->freq = 3014;
            pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN, MLT_COMM_REG_DIAG1, 0xB);	/*disable testfilter */
            pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN, MLT_COMM_REG_OCON, 0x0);	/*disable osc1 */
            setup3014HzBandpass(pProSLICMLT);
            setup3014HzTestTone(pProSLICMLT);
            pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN, MLT_COMM_REG_DIAG1, 0x1B);	/*enable testfilter */
            pState->MeasState.waitIterations = 0;
            return RC_MLT_TEST_RUNNING;

        case 3:
            delay_poll(&pState->MeasState, 900 / mlt_poll_rate - 2);
            return RC_MLT_TEST_RUNNING;

            case 4:
            pState->MeasState.stage++;
            pState->MeasState.waitIterations = 0;
            data = pProSLICMLT->ReadRAM(pMLT_HW, pMLT_CHAN, MLT_COMM_RAM_TESTABO);
            pState->testavo = data;
            return RC_MLT_TEST_COMPLETE;
    }
    return RC_MLT_TEST_COMPLETE;
}


/********************************************************************************/ 
/**
** @brief Establish AUTO-V step size from measured dV/dt
** 
** @param[in] *pState      -  Pointer to resistance test state structure
**
** @retval int32
**
*/
static int setupAutovForDvdt(ProSLIC_mlt_rmeas_state *pState)
{
int return_val = RC_MLT_ERROR;

	switch(pState->active_term)
	{
	case MLT_TERM_TR:
		return_val = RC_NONE;

		pState->interval = 10;  /* Fast interval req'd to monitor for AHS charge pump */

		if((pState->dvdt_tr > 0) && (pState->dvdt_tr <= 95))
		{
			pState->setupState.settleTime = 600;
			pState->step = 2000000L;
		}
		else if((pState->dvdt_tr > 95) && (pState->dvdt_tr <= 120))
		{
			pState->setupState.settleTime = 500;
			pState->step = 800000L;
		}
		else if((pState->dvdt_tr > 120) && (pState->dvdt_tr <= 150))
		{
			pState->setupState.settleTime = 400;
			pState->step = 400000L;
		}
		else if((pState->dvdt_tr > 150) && (pState->dvdt_tr <= 210))
		{
			pState->setupState.settleTime = 300;
			pState->step = 200000L;
		}
		else 
		{
			pState->setupState.settleTime = 200;
			pState->step = 120000L;
		}
		
		/* Adjust test voltages and settle time if AHS detected */
		if(pState->ahs_det.detected > 0)
		{
			pState->setupState.settleTime = MLT_RES_AUTOV_DIFF_AHS_SETTLE;
			pState->vtar[0] = MLT_RES_AUTOV_DIFF_AHS_V1;
			if(pState->ahs_det.detected == 1)
				pState->vtar[1] = MLT_RES_AUTOV_DIFF_AHS_V2;
		}
		else
		{
			pState->vtar[0] = MLT_RES_AUTOV_DIFF_V1;
			pState->vtar[1] = MLT_RES_AUTOV_DIFF_V2;
		}

		break;


	case MLT_TERM_TG:
	case MLT_TERM_RG:
		return_val = RC_NONE;

		if((pState->dvdt_tr > 0) && (pState->dvdt_tr <= 90))
		{
			pState->setupState.settleTime = 600;
			pState->step = 200000;
			pState->interval = 50;
		}
		else if((pState->dvdt_tr > 90) && (pState->dvdt_tr <= 120))
		{
			pState->setupState.settleTime = 600;
			pState->step = 200000;
			pState->interval = 30;
		}
		else
		{
			pState->setupState.settleTime = 300;
			pState->step = 100000;
			pState->interval = 30;
		}

		/* Adjust test voltages and settle time if AHS detected during TR test */
		if(pState->ahs_det.count > 0)
		{
			pState->setupState.settleTime = MLT_RES_AUTOV_LONG_AHS_SETTLE;
			pState->vtar[0] = MLT_RES_AUTOV_LONG_AHS_V1;
			pState->vtar[1] = MLT_RES_AUTOV_LONG_AHS_V2;
		}
		else
		{
			pState->vtar[0] = MLT_RES_AUTOV_LONG_V1;
			pState->vtar[1] = MLT_RES_AUTOV_LONG_V2;
		}

		if(pState->active_term == MLT_TERM_TG)
		{
			pState->vtar[0] *= -1;
			pState->vtar[1] *= -1;
		}

		break;
	}

	return return_val;
}
/********************************************************************************/ 
/**
** @brief Implements Auto-V adaptive force-IV/measure-IV method
** 
** @param[in] *pProSLICMLT -  Pointer to MLT channel structure
** @param[in] *pState - pointer to test state structure
**
** @retval typical values: @ref RC_MLT_TEST_RUNNING or @ref RC_MLT_TEST_COMPLETE are normal values returned. @sa MLT_RETURN_CODES for other possible values.
**
** @remark 
**  - This function is reentrant and should be called at periodic intervals defined by mlt_poll_rate
**  - USER MODE status is not modified in this function
**
*/
static int execAutoV (ProSLICMLTType *pProSLICMLT, ProSLIC_mlt_rmeas_state *pState)
{
int32 i_samp;
int32 v_samp;
int32 v_open_term = 0;
int32 i_leak = 0;
int32 v_target;
int i;
uInt16 mlt_poll_rate;
#ifdef MLT_RUNTIME_CONFIG
    mlt_poll_rate = pProSLICMLT->mlt_config.mlt_poll_rate;
#else
    mlt_poll_rate = MLT_POLL_RATE;
#endif
    /* Start of reentrant loop */
	switch (pState->autoVState.stage)
	{
		case 0: 
			/************************************/
			/* Setup DIAG current source/AUTO-V */
			/************************************/ 
			v_target = pState->vtar[0] * MLT_CONST_AUTO_V_VTAR;
            diagSet(pProSLICMLT->pProslic, DIAG_CLR_DIAG1);

			/* If RG or TG, setup coarse sensors to measure OPEN terminal */
			if(pState->active_term == MLT_TERM_RG)
			{
				setupCoarseSensors(pProSLICMLT, &(pState->tipOffs), &(pState->ringOffs));
				diagSelect(pProSLICMLT->pProslic, DIAG_SEL_VTIPC, MADC_HIRES);
			}
			else if(pState->active_term == MLT_TERM_TG)
			{
				setupCoarseSensors(pProSLICMLT, &(pState->tipOffs), &(pState->ringOffs));
				diagSelect(pProSLICMLT->pProslic, DIAG_SEL_VRINGC, MADC_HIRES);
			}

		    pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN,MLT_COMM_RAM_RINGAMP,0);
		    pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN,MLT_COMM_RAM_RINGFR,0);  /* DC */
		    pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN,MLT_COMM_RAM_DIAG_ADJ_STEP,pState->step);
		    pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN,MLT_COMM_RAM_SLOPE_RING,0x1f000000L);
		    pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN,MLT_COMM_RAM_RINGOF,0);
		    pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN,MLT26X_C_RAM_DIAG_V_TAR,v_target);
	        pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN,MLT_COMM_RAM_P_TH_HVIC,0x4468adL);
	        diagSet(pProSLICMLT->pProslic, DIAG_AUTO_V|DIAG_ENABLE|DIAG_KDC_OFF|DIAG_DCLPF_44K|DIAG_FB_OFF);
		    pState->autoVState.waitIterations=0;
		    pState->autoVState.sampleIterations=0;
			pState->vmeas = 0;
			pState->imeas = 0;
		    pState->autoVState.stage++;
		    return RC_MLT_TEST_RUNNING;

		case 1: /* Settle */
            delay_poll(&(pState->autoVState),pState->setupState.settleTime/mlt_poll_rate -2 );
		    return RC_MLT_TEST_RUNNING;
			break;

		case 2: /* Sample Interval */
            delay_poll(&(pState->autoVState),pState->interval/mlt_poll_rate );
		    return RC_MLT_TEST_RUNNING;
			break;

		case 3: /* Capture samples and accumulate */

			/* Drive terminal voltage */
            v_samp = ReadMADCScaled(pProSLICMLT, MLT_COMM_RAM_V_FEED_IN, 0);
		    pState->vmeas += v_samp;
			
			/* Open terminal voltage (if applicable) */
			if(pState->active_term == MLT_TERM_RG)
			{
				v_open_term = ReadMADCScaled(pProSLICMLT,MLT_COMM_RAM_MADC_DIAG_RAW,0) - pState->tipOffs;
				i_leak = (v_open_term*1000L)/MLT_CONST_AUTOV_REQ_OPEN;
			}
			else if(pState->active_term == MLT_TERM_TG)
			{
				v_open_term = ReadMADCScaled(pProSLICMLT,MLT_COMM_RAM_MADC_DIAG_RAW,0) - pState->ringOffs;
				i_leak = -(v_open_term*1000L)/MLT_CONST_AUTOV_REQ_OPEN;
			}

			/* Drive terminal current */ 
			i_samp = ReadMADCScaled(pProSLICMLT, MLT_COMM_RAM_RINGOF, 0);

			/* Store drive current less leakage */
			pState->imeas += (i_samp - i_leak);
     
		    pState->autoVState.sampleIterations++;

		    if (pState->autoVState.sampleIterations == pState->autoVState.numSamples) 
			{ /* Compute averages */
			    pState->vmeas /= pState->autoVState.numSamples;
				pState->imeas /= pState->autoVState.numSamples;

				pState->v1 = pState->vmeas;
				pState->i1 = pState->imeas;

#ifdef MLT_RES_AUTOV_SINGLE_SAMPLE
				pState->v1 = v_samp;
				pState->i1 = i_samp;
#endif

			    MLT_DEBUG_LOG ("si3226x_mlt : execAutoV : Avg V1    = %dmV \n",pState->vmeas);
				MLT_DEBUG_LOG ("si3226x_mlt : execAutoV : Avg I1    = %duA \n",pState->imeas);		    

			   pState->autoVState.stage++;
		    }
			else /* Take next sample */
			{
				pState->autoVState.stage = 2;
			}

		    return RC_MLT_TEST_RUNNING; 


		case 4: /* Setup for 2nd measurement */
			pState->autoVState.sampleIterations = 0;
			pState->autoVState.waitIterations=0;
			pState->vmeas = 0;
			pState->imeas = 0;
			v_target = pState->vtar[1] * MLT_CONST_AUTO_V_VTAR;
		    pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN,MLT26X_C_RAM_DIAG_V_TAR,v_target);
			pState->autoVState.stage++;
		    return RC_MLT_TEST_RUNNING; 

		case 5: /* Settle */
            delay_poll(&(pState->autoVState),pState->setupState.settleTime/mlt_poll_rate -2 );
		    return RC_MLT_TEST_RUNNING;
			break;

		case 6: /* Sample Interval */
            delay_poll(&(pState->autoVState),pState->interval/mlt_poll_rate );
		    return RC_MLT_TEST_RUNNING;
			break;

		case 7: /* Capture samples and accumulate 2nd measurement*/
			/* Drive terminal voltage */
            v_samp = ReadMADCScaled(pProSLICMLT, MLT_COMM_RAM_V_FEED_IN, 0);
		    pState->vmeas += v_samp;

			/* Open terminal voltage (if applicable) */
			if(pState->active_term == MLT_TERM_RG)
			{
				v_open_term = ReadMADCScaled(pProSLICMLT,MLT_COMM_RAM_MADC_DIAG_RAW,0) - pState->tipOffs;
				i_leak = (v_open_term*1000L)/MLT_CONST_AUTOV_REQ_OPEN;
			}
			else if(pState->active_term == MLT_TERM_TG)
			{
				v_open_term = ReadMADCScaled(pProSLICMLT,MLT_COMM_RAM_MADC_DIAG_RAW,0) - pState->ringOffs;
				i_leak = -(v_open_term*1000L)/MLT_CONST_AUTOV_REQ_OPEN;
			}
	
		
			/* Drive terminal current */		
			i_samp = ReadMADCScaled(pProSLICMLT, MLT_COMM_RAM_RINGOF, 0);

			/* Store sample for AHS detection */
			pState->i_array[pState->autoVState.sampleIterations] = i_samp - i_leak;
			/* Store drive current less leakage */
			pState->imeas += (i_samp - i_leak);

		    pState->autoVState.sampleIterations++;

		    if (pState->autoVState.sampleIterations == pState->autoVState.numSamples) 
			{ /* Compute averages */
			    pState->vmeas /= pState->autoVState.numSamples;
				pState->imeas /= pState->autoVState.numSamples;
		
				pState->v2 = pState->vmeas;
				pState->i2 = pState->imeas;

#ifdef MLT_RES_AUTOV_SINGLE_SAMPLE
				pState->v2 = v_samp;
				pState->i2 = i_samp;
#endif				
				pState->autoVState.stage++;

				MLT_DEBUG_LOG ("si3226x_mlt : execAutoV : Avg V2    = %dmV \n",pState->vmeas);
				MLT_DEBUG_LOG ("si3226x_mlt : execAutoV : Avg I2    = %duA \n",pState->imeas);
			}
			else
			{ /* Take next sample */
				pState->autoVState.stage = 6;
			}
		    return RC_MLT_TEST_RUNNING; 
			break;

		case 8: /* AHS Computation */
			
			/* -------------------------
			** Active Handset Detection
			** -------------------------
			*/

			if(pState->ahs_det.enable)
			{
				/*
				** Test 1: 
				** 
				** Compute RMS current of second capture.  If an active handset 
				** is present, there will be a large ac component.
				*/
				for(i=0;i<pState->autoVState.numSamples;i++)
				{
					pState->i_array[i] -= pState->imeas; /* Remove DC */
				}
				pState->ahs_det.irms = 0;
				for(i=0;i<pState->autoVState.numSamples;i++)
				{
					pState->ahs_det.irms += (pState->i_array[i])*(pState->i_array[i]);
				}
				
				pState->ahs_det.irms /= pState->autoVState.numSamples; 

				/*
				** Test 2:
				**
				** Look at different between ratio of I2 to I1 and the difference
				** between I2 and I1 (eg. large ratio, small difference)
				*/
                if(pState->i1 != 0)
                {
				    pState->ahs_det.ratio = (pState->i2 * 1000L) / pState->i1;
                }
                else
                {
                    pState->ahs_det.ratio = 1;
                }
				pState->ahs_det.ratio -= (pState->i2 - pState->i1);

				MLT_DEBUG_LOG ("si3226x_mlt : execAutoV : AHS Rms   = %d uArms\n", pState->ahs_det.irms);
				MLT_DEBUG_LOG ("si3226x_mlt : execAutoV : AHS Ratio = %d \n", pState->ahs_det.ratio);
				
			   /* 
			   ** Test against configurable rms current and current
			   ** ratio limits to identify active handset.  Occassionally,
			   ** current is measure while the AHS's charge pump is not
			   ** active, so the current ratio will cover that portion.
			   **
			   ** There are cases in which a longitudinal fault in parallel
			   ** with the AHS will result in missing both of these, but
			   ** the magnitude of the fault is large enough that it becomes
			   ** the dominant concern.
			   **
			   ** In the case of a TG fault, the ratio will exceed 3000, but
			   ** the absolute current will be low, so I2 is also checked.
			   */
				pState->ahs_det.detected = 0;  /* Default to 0 */

				if((pState->ahs_det.ratio >= 3000)&&(pState->i2 > 200))
				{
					pState->ahs_det.detected = 1; 
				}
				else if((pState->ahs_det.irms > 200L)&&(pState->ahs_det.ratio > 2000))
				{
					pState->ahs_det.detected = 2;
				}

			}
		   pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN,MLT_COMM_REG_DIAG1,0);/*disable diag*/
		   pState->autoVState.stage++;
		   return RC_MLT_TEST_COMPLETE; 

	    }/* Switch */

	return RC_MLT_TEST_COMPLETE;
}


/********************************************************************************/
/**
** @internal @brief Measures resistance present between TIP and RING
** 
** @param[in,out] *pProSLICMLT -  Pointer to MLT channel structure.  Upon completion, pProSLICMLT->resFaults.measTR have the measured values.
** @param[in] *pState - pointer to test state structure
**
** @retval 
** @retval typical values: @ref RC_MLT_TEST_RUNNING or @ref RC_MLT_TEST_COMPLETE are normal values returned. @sa MLT_RETURN_CODES for other possible values.
**
**
** @remark 
**  - This function is reentrant and should be called at periodic intervals defined by mlt_poll_rate
**  - USER MODE status is not modified in this function
**
*/
static int measRtr(ProSLICMLTType *pProSLICMLT,ProSLIC_mlt_rmeas_state *pState){ 
int32 vdiff,ir;
uInt32 mlt_prot_res; 
uInt16 mlt_poll_rate;
    int32 tr_req_const;
#ifdef MLT_RUNTIME_CONFIG
    mlt_poll_rate = pProSLICMLT->mlt_config.mlt_poll_rate;
    mlt_prot_res  = pProSLICMLT->mlt_config.mlt_prot_res;
    if(pProSLICMLT->mlt_config.mlt_ext_v_flag)
    {
        tr_req_const = MLT_CONST_REQ_DIFF_EV_0;
    }
    else
    {
        tr_req_const = MLT_CONST_REQ_DIFF_SV_0;
    }

#else
    mlt_poll_rate = MLT_POLL_RATE;
    mlt_prot_res  = MLT_PROTECTION_RESISTANCE;
#ifdef MLT_USE_EXTENDED_V_RANGE
    tr_req_const = MLT_CONST_REQ_DIFF_EV_0;
#else
    tr_req_const = MLT_CONST_REQ_DIFF_SV_0;
#endif
#endif
    /* Start of reentrant block */
	switch (pState->TRState.stage){

		case 0:
			pState->active_term = MLT_TERM_TR;
			pState->ahs_det.enable = MLT_RES_AUTOV_AHS_DET_EN;  /* Enable active handset detection */
			pState->ahs_det.detected = 0;							/* Clear active handset detection flag */
			pState->ahs_det.count = 0;						/* Clear remeasure counter */
			pState->autoVState.numSamples = MLT_RES_AUTOV_SAMPLES;
			/* Limit to capture array size */
			if(pState->autoVState.numSamples > MLT_MAX_AUTOV_SAMPLES) 
				pState->autoVState.numSamples = MLT_MAX_AUTOV_SAMPLES;
		    pState->TRState.stage++;
		    return RC_MLT_TEST_RUNNING;

		case 1: /* Initialize flags and counters;  Setup linefeed */
		    pState->TRState.waitIterations=0;
		    pState->TRState.sampleIterations=0;
            pProSLICMLT->resFaults.auxFlag = 0;
            pProSLICMLT->resFaults.trFlag = 0;
            pProSLICMLT->resFaults.tgFlag = 0;
            pProSLICMLT->resFaults.rgFlag = 0;
		    setupDcFeedAutoV(pProSLICMLT);
		    pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN,MLT_COMM_REG_LINEFEED,LF_FWD_ACTIVE);  
		    pState->TRState.stage++;
		    return RC_MLT_TEST_RUNNING;

		case 2: /* settle linefeed */
		    delay_poll(&(pState->TRState),400/mlt_poll_rate - 2); 
		    return RC_MLT_TEST_RUNNING;

		case 3:  /* Check for small Rtr in FWD mode */
            ir = ReadMADCScaled(pProSLICMLT, MLT_COMM_RAM_MADC_ILOOP, 0); 
            vdiff = ReadMADCScaled(pProSLICMLT, MLT_COMM_RAM_VDIFF_FILT, 0);

			MLT_DEBUG_LOG("si3226x_mlt : measRtr : FWD Short : ILOOP = %duA\n",ir);
			MLT_DEBUG_LOG("si3226x_mlt : measRtr : FWD Short : VDIFF = %dmV\n", vdiff);


            if(ir > MLT_RES_SM_R_MIN_I_FWD) 
			{ /* Store if iloop > 8mA */
                pProSLICMLT->resFaults.measTR = (vdiff*10000)/ir - 2*mlt_prot_res;
                if(pProSLICMLT->resFaults.measTR < 0)
				    pProSLICMLT->resFaults.measTR = 0;
                pProSLICMLT->resFaults.trFlag = 1;
            }
            else 
			{ /* Consider open and do auto-v test */
                pProSLICMLT->resFaults.measTR = MLT_CONST_MAX_RES;
                pProSLICMLT->resFaults.trFlag = 0;
            }
            /* Setup for reverse active test */
			pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN,MLT_COMM_REG_LINEFEED,LF_REV_ACTIVE);  
			pState->TRState.stage++;
			return RC_MLT_TEST_RUNNING;

		case 4: /* settle */
	    	delay_poll(&(pState->TRState),400/mlt_poll_rate - 2); 
		    return RC_MLT_TEST_RUNNING;

		case 5: /* check for small Rtr in REV active, decide which is real later */
            ir = ReadMADCScaled(pProSLICMLT, MLT_COMM_RAM_MADC_ILOOP, 0); 
            vdiff = ReadMADCScaled(pProSLICMLT, MLT_COMM_RAM_VDIFF_FILT, 0);

			MLT_DEBUG_LOG("si3226x_mlt : measRtr : REV Short : ILOOP = %duA\n",ir);
			MLT_DEBUG_LOG("si3226x_mlt : measRtr : REV Short : VDIFF = %dmV\n", vdiff);

            if(fp_abs(ir) > MLT_RES_SM_R_MIN_I_REV) 
			{ /* store if > 7ma (1ma of error between fwd/rev */
                pProSLICMLT->resFaults.measAUX = (fp_abs(vdiff)*10000)/fp_abs(ir) - 2*mlt_prot_res;
                if(pProSLICMLT->resFaults.measAUX < 0)
				    pProSLICMLT->resFaults.measAUX = 0;
                pProSLICMLT->resFaults.auxFlag = 1;
            } 
            else 
			{ 
                pProSLICMLT->resFaults.measAUX = MLT_CONST_MAX_RES;
                pProSLICMLT->resFaults.auxFlag = 0;
            }

			/* Return to FWD */
            pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN,MLT_COMM_REG_LINEFEED,LF_FWD_ACTIVE);  

            /* If both fwd and rev indicate a small R, skip remainder of test */
            if(pProSLICMLT->resFaults.auxFlag && pProSLICMLT->resFaults.trFlag)
			{
				MLT_DEBUG_LOG("si3226x_mlt : measRtr : Small RTR FWD RAW = %d\n", pProSLICMLT->resFaults.measTR);
				MLT_DEBUG_LOG("si3226x_mlt : measRtr : Small RTR REV RAW = %d\n", pProSLICMLT->resFaults.measAUX);

                pState->rtrMethod = LOWR;
                pState->TRState.stage = 70;
            }
			else
			{
                pState->rtrMethod = AUTOV;
                pState->TRState.stage++;
			}
			return RC_MLT_TEST_RUNNING;

		case 6:
			setupAutovForDvdt(pState); 

			MLT_DEBUG_LOG("si3226x_mlt : measRtr : AutoV Setup : Step    = %d\n", pState->step);
			MLT_DEBUG_LOG("si3226x_mlt : measRtr : AutoV Setup : Settle  = %d\n", pState->setupState.settleTime);
			MLT_DEBUG_LOG("si3226x_mlt : measRtr : AutoV Setup : V1      = %d\n", pState->vtar[0]);
			MLT_DEBUG_LOG("si3226x_mlt : measRtr : AutoV Setup : V2      = %d\n", pState->vtar[1]);
			MLT_DEBUG_LOG("si3226x_mlt : measRtr : AutoV Setup : Samples = %d\n", pState->autoVState.numSamples);

		    pState->autoVState.stage=0;
			pState->TRState.stage++;
			return RC_MLT_TEST_RUNNING;

		case 7:
			if (execAutoV(pProSLICMLT,pState))
			{
				/*
				** Active handset detected - repeat measurement until settled
				*/
				if((pState->ahs_det.detected)&&(pState->ahs_det.count <= MLT_RES_AUTOV_AHS_MAX_RETEST))
				{
						pState->TRState.stage = 6;
						pState->ahs_det.count++;
						pProSLICMLT->resFaults.ahsFlag = 1;
						return RC_MLT_TEST_RUNNING;
				}

                /* Process Results */
			    if ((pState->i2-pState->i1) == 0) {  /* OPEN */
				    pProSLICMLT->resFaults.measTR = MLT_CONST_MAX_RES;
                }
			    else { /* Find Absolute Resistance between T-R */
				    pProSLICMLT->resFaults.measTR = ((pState->v2 - pState->v1)*10000 / (pState->i2-pState->i1) - 2*mlt_prot_res); 
                }

				MLT_DEBUG_LOG("si3226x_mlt : measRtr : AutoV RTR RAW : %d\n", pProSLICMLT->resFaults.measTR);
  
		        if ((pProSLICMLT->resFaults.measTR != 0) && (MLT_CONST_OPEN_RES/pProSLICMLT->resFaults.measTR != tr_req_const))
			        pProSLICMLT->resFaults.measTR = MLT_CONST_OPEN_RES/(MLT_CONST_OPEN_RES/pProSLICMLT->resFaults.measTR - tr_req_const);
                else 
				    pProSLICMLT->resFaults.measTR = MLT_CONST_MAX_RES;
			    pState->TRState.stage=70;  
		    }
		    return RC_MLT_TEST_RUNNING;


		case 70: /* move to end */
		if (pProSLICMLT->resFaults.measTR < 0)
			pProSLICMLT->resFaults.measTR = MLT_CONST_MAX_RES;
		return RC_MLT_TEST_COMPLETE;

		
	}
    return RC_MLT_TEST_COMPLETE;
}


/********************************************************************************/
/**
** @brief Measures resistance present between RING and GND
** 
** @param[in,out] *pProSLICMLT -  Pointer to MLT channel structure.  Upon completion, pProSLICMLT->resFaults.measRG contains the measured values.
** @param[in] *pState - pointer to test state structure
**
** @retval typical values: @ref RC_MLT_TEST_RUNNING or @ref RC_MLT_TEST_COMPLETE are normal values returned. @sa MLT_RETURN_CODES for other possible values.
**
**
** @remark 
**  - This function is reentrant and should be called at periodic intervals defined by mlt_poll_rate
**  - USER MODE status is not modified in this function
**
*/
static int measRrg(ProSLICMLTType *pProSLICMLT,ProSLIC_mlt_rmeas_state *pState){
	/* first check for a small R then run auto-v*/
	int32 iring; int32 vring;
    int32 dV;  int32 dI;
    int32 rg_req_const_0;
    int32 rg_req_const_1;
    int32 rg_req_auto_const;
uInt32 mlt_prot_res; 
uInt16 mlt_poll_rate;
#ifdef MLT_RUNTIME_CONFIG
    mlt_poll_rate = pProSLICMLT->mlt_config.mlt_poll_rate;
    mlt_prot_res  = pProSLICMLT->mlt_config.mlt_prot_res;
    if(pProSLICMLT->mlt_config.mlt_ext_v_flag)
    {
        rg_req_auto_const = MLT_CONST_REQ_AUTO_LONG_EV;
        rg_req_const_0 = MLT_CONST_REQ_LONG_EV_0;
        rg_req_const_1 = MLT_CONST_REQ_LONG_EV_1;
    }
    else
    {
        rg_req_auto_const = MLT_CONST_REQ_AUTO_LONG_SV;
        rg_req_const_0 = MLT_CONST_REQ_LONG_SV_0;
        rg_req_const_1 = MLT_CONST_REQ_LONG_SV_1;
    }
#else
    mlt_poll_rate = MLT_POLL_RATE;
    mlt_prot_res  = MLT_PROTECTION_RESISTANCE;
#ifdef MLT_USE_EXTENDED_V_RANGE
    rg_req_auto_const = MLT_CONST_REQ_AUTO_LONG_EV;
    rg_req_const_0 = MLT_CONST_REQ_LONG_EV_0;
    rg_req_const_1 = MLT_CONST_REQ_LONG_EV_1;
#else
    rg_req_auto_const = MLT_CONST_REQ_AUTO_LONG_SV;
    rg_req_const_0 = MLT_CONST_REQ_LONG_SV_0;
    rg_req_const_1 = MLT_CONST_REQ_LONG_SV_1;
#endif
#endif

	switch (pState->RGState.stage){

		case 0:
			pState->active_term = MLT_TERM_RG;
			pState->RGState.waitIterations=0;
			if (!(pState->smallRFlag & 2))
			{
				pState->RGState.stage=5; /*auto-v method*/
			} 
			else 
			{
				setupDcFeedV1SmallR(pProSLICMLT);
				pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN,MLT_COMM_REG_LINEFEED,LF_TIP_OPEN);
				pState->RGState.stage++;
			}
			return RC_MLT_TEST_RUNNING;

		case 1:
			delay_poll(&(pState->RGState),320/mlt_poll_rate - 2);
			return RC_MLT_TEST_RUNNING;

		case 2: /* small R test, first measurement */
            iring = ReadMADCScaled(pProSLICMLT, MLT_COMM_RAM_MADC_IRING,0);
            vring = ReadMADCScaled(pProSLICMLT, MLT_COMM_RAM_VRING, 0);
			MLT_DEBUG_LOG ("si3226x_mlt : measRrg : Small RRG : V1 = %dmV\n",vring);
			MLT_DEBUG_LOG ("si3226x_mlt : measRrg : Small RRG : I1 = %duA\n",iring);

			pProSLICMLT->resFaults.measRG = vring / -iring;
			pState->i1 = -iring;  
			pState->v1 = vring; 

			if (pProSLICMLT->resFaults.measRG < 0)
				pProSLICMLT->resFaults.measRG = 0;
			setupDcFeedV2SmallR(pProSLICMLT);
			pState->RGState.stage++;
			return RC_MLT_TEST_RUNNING;

	  case 3: /* Settle */
		  delay_poll(&(pState->RGState),320/mlt_poll_rate - 2);
		  return RC_MLT_TEST_RUNNING;

	  case 4:  /* Small R test, second measurement and difference calculation */
            iring = ReadMADCScaled(pProSLICMLT, MLT_COMM_RAM_MADC_IRING,0);
            vring = ReadMADCScaled(pProSLICMLT, MLT_COMM_RAM_VRING,0);

			MLT_DEBUG_LOG ("si3226x_mlt : measRrg : Small RRG : V2 = %dmV\n",vring);
			MLT_DEBUG_LOG ("si3226x_mlt : measRrg : Small RRG : I2 = %duA\n",iring);
 
            pState->i2 = -iring;
            pState->v2 = vring;
          
		  /* 
		  ** Loop equation in TIP-OPEN mode yields
          ** RRG = dV/(dI - dV/a1 - 1/a0), where a1 and a0
          ** are constants derived from the equivalent circuit
          ** model.  They may depend on pulse metering bom option
          */
		  dV = 1000L*fp_abs(pState->v2 - pState->v1);
		  dI = fp_abs(pState->i2 - pState->i1) / 10L;
		  dI -= dV/rg_req_const_1;          
		  dI += 1000000/rg_req_const_0;  



		  if(dI <= 0)
				pProSLICMLT->resFaults.measRG = MLT_CONST_MAX_RES;
		  else {
				pProSLICMLT->resFaults.measRG = (dV/dI);
				pProSLICMLT->resFaults.measRG -= mlt_prot_res;   /* subtract protection resistance */
		  }

		  if (pProSLICMLT->resFaults.measRG < 0)
			  pProSLICMLT->resFaults.measRG = 0;

		  pState->RGState.stage=70;
		  return RC_MLT_TEST_COMPLETE;

	  case 5:  /* Auto V test */
		  pState->ahs_det.enable = 0;       /* Disable active handset detection */
		  pState->autoVState.numSamples = MLT_RES_AUTOV_SAMPLES;
		  /* Limit to capture array size */
		  if(pState->autoVState.numSamples > MLT_MAX_AUTOV_SAMPLES)
				pState->autoVState.numSamples = MLT_MAX_AUTOV_SAMPLES;

		  setupAutovForDvdt(pState);

		  pState->RGState.stage++;
		  pState->autoVState.stage=0;
		  pState->RGState.waitIterations=0;

		  MLT_DEBUG_LOG("si3226x_mlt : measRrg : AutoV Setup : Step   = %d\n", pState->step);
		  MLT_DEBUG_LOG("si3226x_mlt : measRrg : AutoV Setup : Settle = %d\n", pState->setupState.settleTime);
		  MLT_DEBUG_LOG("si3226x_mlt : measRrg : AutoV Setup : V1     = %d\n", pState->vtar[0]);
		  MLT_DEBUG_LOG("si3226x_mlt : measRrg : AutoV Setup : V2     = %d\n", pState->vtar[1]);

		  return RC_MLT_TEST_RUNNING;

	case 6: /* Settle */
		  delay_poll(&(pState->RGState),50/mlt_poll_rate - 2);
		  return RC_MLT_TEST_RUNNING;

	case 7:
		  pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN,MLT_COMM_REG_LINEFEED,LF_TIP_OPEN);
		  /* Bias RING side LKG DAC to 4mA to improve open loop stability */
		  pProSLICMLT->WriteRAM(pMLT_HW,pMLT_CHAN,MLT_COMM_RAM_LKG_DNR_ACTIVE,0x1FF00000L);
		  pProSLICMLT->WriteRAM(pMLT_HW,pMLT_CHAN,MLT_COMM_RAM_LKG_UPR_ACTIVE,0x1FF00000L);
		  pState->RGState.stage++;
		  return RC_MLT_TEST_RUNNING;

	case 8:
		if(execAutoV(pProSLICMLT,pState))
		{
		    /* Restore 0mA bias */
			pProSLICMLT->WriteRAM(pMLT_HW,pMLT_CHAN,MLT_COMM_RAM_LKG_DNR_ACTIVE,0L);
			pProSLICMLT->WriteRAM(pMLT_HW,pMLT_CHAN,MLT_COMM_RAM_LKG_UPR_ACTIVE,0L);
			if ((pState->i2-pState->i1) == 0)
				pProSLICMLT->resFaults.measRG = MLT_CONST_MAX_RES;
			else {
                /* first order model since no r between tip/ring */
                dV = (fp_abs(pState->v2 - pState->v1))*10000;
			    dI = fp_abs(pState->i2-pState->i1);

                if(dI <= 0)
				{
			        pProSLICMLT->resFaults.measRG = MLT_CONST_MAX_RES;
				}
			    else 
				{
                    pProSLICMLT->resFaults.measRG = dV/dI;
                    pProSLICMLT->resFaults.measRG *= 107;
                    pProSLICMLT->resFaults.measRG /= 100;

					MLT_DEBUG_LOG("si3226x_mlt : measRrg : AutoV RRG RAW : %d\n", pProSLICMLT->resFaults.measRG);

                    if(pProSLICMLT->resFaults.measRG > 0) 
					{
                        if(MLT_CONST_OPEN_RES/pProSLICMLT->resFaults.measRG <= rg_req_auto_const)
						{
                             pProSLICMLT->resFaults.measRG = MLT_CONST_MAX_RES;
						}
						else
						{
                            pProSLICMLT->resFaults.measRG = MLT_CONST_OPEN_RES/(MLT_CONST_OPEN_RES/pProSLICMLT->resFaults.measRG - rg_req_auto_const);
							pProSLICMLT->resFaults.measRG -= mlt_prot_res;
						}
                    }
                }                   
		  }	
		  pState->RGState.stage=70;
		}/* goToV */
	    return RC_MLT_TEST_RUNNING;
	
	case 70:
		if (pProSLICMLT->resFaults.measRG < 0)
			pProSLICMLT->resFaults.measRG = MLT_CONST_MAX_RES;
		return RC_MLT_TEST_COMPLETE;

	}/* switch */
    return RC_MLT_TEST_COMPLETE;
}


/********************************************************************************/
/**
** @brief Measures resistance present between TIP and GND
** 
** @param[in,out] *pProSLICMLT -  Pointer to MLT channel structure. Output is located in  pProSLICMLT->resFaults.measTG
** @param[in] *pState - pointer to test state structure
**
** @retval typical values: @ref RC_MLT_TEST_RUNNING or @ref RC_MLT_TEST_COMPLETE are normal values returned. @sa MLT_RETURN_CODES for other possible values.
**
**
** @remark 
**  - This function is reentrant and should be called at periodic intervals defined by mlt_poll_rate
**  - USER MODE status is not modified in this function
**
*/
static int measRtg(ProSLICMLTType *pProSLICMLT,ProSLIC_mlt_rmeas_state *pState){ 
	/* first check for a short then run auto-v*/
	int32 itip,vtip;
	int32 dV;  int32 dI;
    int32 tg_req_const_0;
    int32 tg_req_const_1;
    int32 tg_req_auto_const;
uInt32 mlt_prot_res; 
uInt16 mlt_poll_rate;
#ifdef MLT_RUNTIME_CONFIG
    mlt_poll_rate = pProSLICMLT->mlt_config.mlt_poll_rate;
    mlt_prot_res  = pProSLICMLT->mlt_config.mlt_prot_res;
    if(pProSLICMLT->mlt_config.mlt_ext_v_flag)
    {
        tg_req_auto_const = MLT_CONST_REQ_AUTO_LONG_EV;
        tg_req_const_0 = MLT_CONST_REQ_LONG_EV_0;
        tg_req_const_1 = MLT_CONST_REQ_LONG_EV_1;
    }
    else
    {
        tg_req_auto_const = MLT_CONST_REQ_AUTO_LONG_SV;
        tg_req_const_0 = MLT_CONST_REQ_LONG_SV_0;
        tg_req_const_1 = MLT_CONST_REQ_LONG_SV_1;
    }
#else
    mlt_poll_rate = MLT_POLL_RATE;
    mlt_prot_res  = MLT_PROTECTION_RESISTANCE;
#ifdef MLT_USE_EXTENDED_V_RANGE
    tg_req_auto_const = MLT_CONST_REQ_AUTO_LONG_EV;
    tg_req_const_0 = MLT_CONST_REQ_LONG_EV_0;
    tg_req_const_1 = MLT_CONST_REQ_LONG_EV_1;
#else
    tg_req_auto_const = MLT_CONST_REQ_AUTO_LONG_SV;
    tg_req_const_0 = MLT_CONST_REQ_LONG_SV_0;
    tg_req_const_1 = MLT_CONST_REQ_LONG_SV_1;
#endif
#endif
	switch (pState->TGState.stage){

		case 0: 
	        pState->active_term = MLT_TERM_TG;
		  pState->TGState.waitIterations=0;
		  if (!(pState->smallRFlag & 1))
		  {
		 	  pState->TGState.stage=5; /*auto-v method*/
		  } 
		  else 
		  {
		    setupDcFeedV1SmallR(pProSLICMLT);
		    pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN,MLT_COMM_REG_LINEFEED,LF_RING_OPEN);
		    pState->TGState.stage++;
		    return RC_MLT_TEST_RUNNING;
		  }

		case 1:
		  delay_poll(&(pState->TGState),320/mlt_poll_rate - 2);
		  return RC_MLT_TEST_RUNNING;

		case 2: /*Small R test, first measurment*/
			itip = ReadMADCScaled(pProSLICMLT,MLT_COMM_RAM_MADC_ITIP,0);
			vtip = ReadMADCScaled(pProSLICMLT,MLT_COMM_RAM_VTIP,0);

			MLT_DEBUG_LOG ("si3226x_mlt : measRtg : Small RTG : V1 = %dmV\n",vtip);
			MLT_DEBUG_LOG ("si3226x_mlt : measRtg : Small RTG : I1 = %duA\n",itip);

		  pProSLICMLT->resFaults.measTG = vtip / -itip;
		  pState->v1 = vtip;
		  pState->i1 = -itip;

		  if (pProSLICMLT->resFaults.measTG < 0)
			  pProSLICMLT->resFaults.measTG = 0;
		  setupDcFeedV2SmallR(pProSLICMLT);
		  pState->TGState.stage++;
          return RC_MLT_TEST_RUNNING;

		case 3: /* Settle */
		  delay_poll(&(pState->TGState),320/mlt_poll_rate - 2);
		  return RC_MLT_TEST_RUNNING;

		case 4: /* Small R test, second measurement and difference calculation */
			itip = ReadMADCScaled(pProSLICMLT,MLT_COMM_RAM_MADC_ITIP,0);
			vtip = ReadMADCScaled(pProSLICMLT,MLT_COMM_RAM_VTIP,0);

			MLT_DEBUG_LOG ("si3226x_mlt : measRtg : Small RTG : V2 = %dmV\n",vtip);
			MLT_DEBUG_LOG ("si3226x_mlt : measRtg : Small RTG : I2 = %duA\n",itip);

		  pState->v2 = vtip;
		  pState->i2 = -itip;

		  /* 
		  ** Loop equation in RING-OPEN mode yields
          ** RTG = dV/(dI - dV/a1 - 1/a0), where a1 and a0
          ** are constants derived from the equivalent circuit
          ** model.  They may depend on pulse metering bom option
          */

		  dV = 1000L*fp_abs(pState->v2 - pState->v1);
		  dI = fp_abs(pState->i2 - pState->i1) / 10L;
		  dI -= dV/tg_req_const_1;         
		  dI += 1000000/tg_req_const_0; 


		  if(dI <= 0)
				pProSLICMLT->resFaults.measTG = MLT_CONST_MAX_RES;
		  else {
				pProSLICMLT->resFaults.measTG = (dV/dI);
				pProSLICMLT->resFaults.measTG -= mlt_prot_res;
		  }
          if (pProSLICMLT->resFaults.measTG < 0)
			 pProSLICMLT->resFaults.measTG = 0;

		  pState->TGState.stage=70;
		  return RC_MLT_TEST_COMPLETE;

		case 5:	/* Auto V test */	
			pState->ahs_det.enable = 0;        /* Disable active handset detection - detected in TR test */
			pState->autoVState.numSamples = MLT_RES_AUTOV_SAMPLES;
			/* Limit to capture array size */
			if(pState->autoVState.numSamples > MLT_MAX_AUTOV_SAMPLES)
				pState->autoVState.numSamples = MLT_MAX_AUTOV_SAMPLES;

			setupAutovForDvdt(pState);

			pState->TGState.stage++; 
			pState->autoVState.stage=0;
			pState->TGState.waitIterations=0;

			MLT_DEBUG_LOG("si3226x_mlt : measRtg : AutoV Setup : Step   = %d\n", pState->step);
			MLT_DEBUG_LOG("si3226x_mlt : measRtg : AutoV Setup : Settle = %d\n", pState->setupState.settleTime);
			MLT_DEBUG_LOG("si3226x_mlt : measRtg : AutoV Setup : V1     = %d\n", pState->vtar[0]);
			MLT_DEBUG_LOG("si3226x_mlt : measRtg : AutoV Setup : V2     = %d\n", pState->vtar[1]);

			/* Precharge open lead */
		    pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN,MLT_COMM_REG_LINEFEED,LF_REV_ACTIVE);
			return RC_MLT_TEST_RUNNING;

		case 6: /* Settle */
		  delay_poll(&(pState->TGState),200/mlt_poll_rate - 2);
		  return RC_MLT_TEST_RUNNING;

		case 7:
		  pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN,MLT_COMM_REG_LINEFEED,LF_RING_OPEN);
		  /* Bias TIP side LKG DAC to 4mA to improve open loop stability */
		  pProSLICMLT->WriteRAM(pMLT_HW,pMLT_CHAN,MLT_COMM_RAM_LKG_DNT_ACTIVE,0x1FF00000L);
		  pProSLICMLT->WriteRAM(pMLT_HW,pMLT_CHAN,MLT_COMM_RAM_LKG_UPT_ACTIVE,0x1FF00000L);
		  pState->TGState.stage++;
		  return RC_MLT_TEST_RUNNING;

		case 8:
			if(execAutoV(pProSLICMLT,pState))
			{
			  /* Restore 0mA bias */
			  pProSLICMLT->WriteRAM(pMLT_HW,pMLT_CHAN,MLT_COMM_RAM_LKG_DNT_ACTIVE,0L);
			  pProSLICMLT->WriteRAM(pMLT_HW,pMLT_CHAN,MLT_COMM_RAM_LKG_UPT_ACTIVE,0L);
			  if ((pState->i2-pState->i1) == 0)
				  pProSLICMLT->resFaults.measTG = MLT_CONST_MAX_RES;
			  else {
                /* first order model since no r between tip/ring */
                dV = (fp_abs(pState->v2 - pState->v1))*10000;
			    dI = fp_abs(pState->i2-pState->i1);

                if(dI <= 0) 
				{
                    pProSLICMLT->resFaults.measTG = MLT_CONST_MAX_RES;
				}
                else 
				{
                    pProSLICMLT->resFaults.measTG = dV/dI;
                    pProSLICMLT->resFaults.measTG *= 107;  
                    pProSLICMLT->resFaults.measTG /= 100;

					MLT_DEBUG_LOG("si3226x_mlt : measRtg : AutoV RTG RAW : %d\n", pProSLICMLT->resFaults.measTG);

                    if(pProSLICMLT->resFaults.measTG > 0) 
					{
                        if ((MLT_CONST_OPEN_RES/pProSLICMLT->resFaults.measTG) <= tg_req_auto_const)
						{
                            pProSLICMLT->resFaults.measTG = MLT_CONST_MAX_RES;
						}
						else
						{
                            pProSLICMLT->resFaults.measTG = MLT_CONST_OPEN_RES/(MLT_CONST_OPEN_RES/pProSLICMLT->resFaults.measTG - tg_req_auto_const);
							pProSLICMLT->resFaults.measTG -= mlt_prot_res;
						}
                    }
                 }          
			}
			pState->TGState.stage=70;
		  }/* goToV */
		  return RC_MLT_TEST_RUNNING;

		case 70:
			if (pProSLICMLT->resFaults.measTG < 0)
			   pProSLICMLT->resFaults.measTG = MLT_CONST_MAX_RES;
		    return RC_MLT_TEST_COMPLETE;
	}
	return RC_MLT_TEST_COMPLETE;
}

/********************************************************************************/
/**
** @brief Measure time constant using MLT diagnostic timers
** 
** @param[in,out] *pProSLICMLT -  Pointer to MLT channel structure.  
** @param[in,out] *termMeas - Pointer to mlt terminal structure
** @param[in] *pState - pointer to test state structure
**
** @retval typical values: @ref RC_MLT_TEST_RUNNING or @ref RC_MLT_TEST_COMPLETE are normal values returned. @sa MLT_RETURN_CODES for other possible values.
**
** @remark 
**  - This function is reentrant and should be called at periodic intervals defined by mlt_poll_rate
**  - USER MODE status is not modified in this function
**
*/
static int measureTimeConstant(ProSLICMLTType *pProSLICMLT, ProSLIC_term_meas_t *termMeas, ProSLIC_mlt_test_state *pState)
{
int32 dt1, dt2, dv1,dv2;
int32 slope1, slope2;
int32 i_offset = 50;
uInt16 mlt_poll_rate;
int enable_meas_down;
#ifdef MLT_RUNTIME_CONFIG
    mlt_poll_rate = pProSLICMLT->mlt_config.mlt_poll_rate;
#else
    mlt_poll_rate = MLT_POLL_RATE;
#endif

    enable_meas_down = termMeas->tgFlag;

    switch(pState->stage)
    {
        case 0:
            pState->waitIterations=0;
            termMeas->term.iloop[1] = termMeas->term.iloop[0] + i_offset; /* comp for offset */
            setupDspTimers(pProSLICMLT->pProslic,termMeas->time1,termMeas->time2,termMeas->time3);
            setupDiagCurrentSource(pProSLICMLT->pProslic,-1*termMeas->term.iloop[0]);  /* (-) current to discharge */
            pState->stage++;
            return RC_MLT_TEST_RUNNING;

        case 1:  /* Settle open terminal switch to GND */
            delay_poll(pState,500/mlt_poll_rate - 2);
            return RC_MLT_TEST_RUNNING;

        case 2:  /* Enable current source */
            armDspTimers(pProSLICMLT->pProslic);
            diagSet(pProSLICMLT->pProslic, DIAG_FB_OFF|DIAG_DCLPF_44K|DIAG_KDC_OFF|DIAG_ENABLE);
            pState->stage++;
            return RC_MLT_TEST_RUNNING;

        case 3:  /* Settle approximate acquisition time */
            delay_poll(pState,((termMeas->time3/10)*10 + 100)/mlt_poll_rate);
            return RC_MLT_TEST_RUNNING;

        case 4: /* Read voltages */
            readDspTimerV(pProSLICMLT->pProslic,&(termMeas->term.vloop[0]),&(termMeas->term.vloop[1]),&(termMeas->term.vloop[2]));
            if(!enable_meas_down)
            {
                diagSet(pProSLICMLT->pProslic,DIAG_CLR_DIAG1);
            }
            /* Calculate dv/dt and estimate C */
            dt1 = fp_abs(termMeas->time2 - termMeas->time1);
            dt2 = fp_abs(termMeas->time3 - termMeas->time2);
            dv1 = fp_abs(termMeas->term.vloop[1] - termMeas->term.vloop[0]);
            dv2 = fp_abs(termMeas->term.vloop[2] - termMeas->term.vloop[1]);

            if(dv1 > 0) 
            { 
                termMeas->measTR2 = (fp_abs(termMeas->term.iloop[0])*1000L*dt1)/dv1;
            }
            else 
            {
                termMeas->measTR2 = 0;
            }

            if(dv2 > 0)
            {
                termMeas->measRG2 = (fp_abs(termMeas->term.iloop[0])*1000L*dt2)/dv2;
            }
            else
            {
                termMeas->measRG2 = 0;
            }

            slope1 = 0;
            slope2 = 0;

            if(dt1 > 0)
            {
                slope1 = (10L*dv1) / dt1;
            }

            if(dt2 > 0)
            {
                slope2 = (10L*dv2) / dt2;
            }

            termMeas->slopeErr = (1000L * fp_abs(slope1 - slope2));
            if(slope1 > 0)
            {
                termMeas->slopeErr /= slope1;
            }
            else
            {
                termMeas->slopeErr = 0;
            }

            MLT_DEBUG_LOG("\n\n");
            MLT_DEBUG_LOG("measureTimeConstant : tcMeasUp : iForce   = %d uA\n", -1*termMeas->term.iloop[0]);
            MLT_DEBUG_LOG("measureTimeConstant : tcMeasUp : VTR1     = %d mV\n", termMeas->term.vloop[0]);
            MLT_DEBUG_LOG("measureTimeConstant : tcMeasUp : VTR2     = %d mV\n", termMeas->term.vloop[1]);
            MLT_DEBUG_LOG("measureTimeConstant : tcMeasUp : VTR3     = %d mV\n", termMeas->term.vloop[2]);
            MLT_DEBUG_LOG("measureTimeConstant : tcMeasUp : t1       = %d ms\n", termMeas->time1);
            MLT_DEBUG_LOG("measureTimeConstant : tcMeasUp : t2       = %d ms\n", termMeas->time2);
            MLT_DEBUG_LOG("measureTimeConstant : tcMeasUp : t3       = %d ms\n", termMeas->time3);
            MLT_DEBUG_LOG("measureTimeConstant : tcMeasUp : dv1      = %d mV\n", dv1);
            MLT_DEBUG_LOG("measureTimeConstant : tcMeasUp : dv2      = %d mV\n", dv2);
            MLT_DEBUG_LOG("measureTimeConstant : tcMeasUp : dt1      = %d ms\n", dt1);
            MLT_DEBUG_LOG("measureTimeConstant : tcMeasUp : dt2      = %d ms\n", dt2);
            MLT_DEBUG_LOG("measureTimeConstant : tcMeasUp : slopeErr = %d \n", termMeas->slopeErr);
            MLT_DEBUG_LOG("measureTimeConstant : tcMeasUp : C1       = %d nF\n", termMeas->measTR2);  
            MLT_DEBUG_LOG("measureTimeConstant : tcMeasUp : C2       = %d nF\n", termMeas->measRG2); 

            if(!enable_meas_down)
            {
                pState->stage = 0;  
                pState->waitIterations = 0;
                return RC_MLT_TEST_COMPLETE;
            }
            else
            {
                pState->stage++;
                return RC_MLT_TEST_RUNNING;
            }

        case 5:  /* Allow full discharge */
            delay_poll(pState,(200/mlt_poll_rate) - 2);
            return RC_MLT_TEST_RUNNING;

        case 6:
            setupDspTimers(pProSLICMLT->pProslic,termMeas->time1,termMeas->time2,termMeas->time3);
            setupDiagCurrentSource(pProSLICMLT->pProslic,termMeas->term.iloop[1]);  /* (+) current to charge */
            armDspTimers(pProSLICMLT->pProslic);
            diagSet(pProSLICMLT->pProslic, DIAG_FB_OFF|DIAG_DCLPF_44K|DIAG_KDC_OFF|DIAG_ENABLE);
            pState->stage++;
            return RC_MLT_TEST_RUNNING;

        case 7:  /* Settle approximate acquisition time */
            delay_poll(pState,((termMeas->time3/10)*10 + 100)/mlt_poll_rate);
            return RC_MLT_TEST_RUNNING;

        case 8: /* Read voltages */
            readDspTimerV(pProSLICMLT->pProslic,&(termMeas->term.vloop[3]),&(termMeas->term.vloop[4]),&(termMeas->term.vloop[5]));
            diagSet(pProSLICMLT->pProslic,DIAG_CLR_DIAG1);
            /* Calculate dv/dt and estimate C */
            dt1 = fp_abs(termMeas->time2 - termMeas->time1);
            dt2 = fp_abs(termMeas->time3 - termMeas->time2);
            dv1 = fp_abs(termMeas->term.vloop[4] - termMeas->term.vloop[3]);
            dv2 = fp_abs(termMeas->term.vloop[5] - termMeas->term.vloop[4]);
          
            if(dv1 > 0) 
            { 
                termMeas->measAUX = (fp_abs(termMeas->term.iloop[1])*1000L*dt1)/dv1;
            }
            else 
            {
                termMeas->measAUX = 0;
            }

            if(dv2 > 0) 
            { 
                termMeas->measTG2 = (fp_abs(termMeas->term.iloop[1])*1000L*dt2)/dv2;
            }
            else 
            {
                termMeas->measTG2 = 0;
            }

            MLT_DEBUG_LOG("\n\n");
            MLT_DEBUG_LOG("measureTimeConstant : tcMeasDown : iForce = %d uA\n", termMeas->term.iloop[1]);
            MLT_DEBUG_LOG("measureTimeConstant : tcMeasDown : VTR1   = %d mV\n", termMeas->term.vloop[3]);
            MLT_DEBUG_LOG("measureTimeConstant : tcMeasDown : VTR2   = %d mV\n", termMeas->term.vloop[4]);
            MLT_DEBUG_LOG("measureTimeConstant : tcMeasDown : VTR3   = %d mV\n", termMeas->term.vloop[5]);
            MLT_DEBUG_LOG("measureTimeConstant : tcMeasDown : t1     = %d ms\n", termMeas->time1);
            MLT_DEBUG_LOG("measureTimeConstant : tcMeasDown : t2     = %d ms\n", termMeas->time2);
            MLT_DEBUG_LOG("measureTimeConstant : tcMeasDown : t3     = %d ms\n", termMeas->time3);
            MLT_DEBUG_LOG("measureTimeConstant : tcMeasDown : dv1    = %d mV\n", dv1);
            MLT_DEBUG_LOG("measureTimeConstant : tcMeasDown : dv2    = %d mV\n", dv2);
            MLT_DEBUG_LOG("measureTimeConstant : tcMeasDown : dt1    = %d ms\n", dt1);
            MLT_DEBUG_LOG("measureTimeConstant : tcMeasDown : dt2    = %d ms\n", dt2);
            MLT_DEBUG_LOG("measureTimeConstant : tcMeasDown : C1     = %d nF\n", termMeas->measAUX); 
            MLT_DEBUG_LOG("measureTimeConstant : tcMeasDown : C2     = %d nF\n", termMeas->measTG2);

            pState->stage = 0;  
            pState->waitIterations = 0;
            return RC_MLT_TEST_COMPLETE;
    }
    return RC_MLT_TEST_COMPLETE;
}



/********************************************************************************/
/**
** @brief Coarse measure of capacitance between TIP and RING
** 
** @param[in,out] *pProSLICMLT -  Pointer to MLT channel structure.  Output is stored under pProSLICMLT->capFaults.measTR
** @param[in] *pState - pointer to test state structure
**
** @retval typical values: @ref RC_MLT_TEST_RUNNING or @ref RC_MLT_TEST_COMPLETE are normal values returned. @sa MLT_RETURN_CODES for other possible values.
**
** @remark 
**  - This function is reentrant and should be called at periodic intervals defined by mlt_poll_rate
**  - USER MODE status is not modified in this function
**
*/

/*
** Measurements
*/
#define MLT_TC_CTR              pProSLICMLT->capFaults.measTR2
#define MLT_TC_DISCHG_CTR1      pProSLICMLT->capFaults.measTR2
#define MLT_TC_DISCHG_CTR2      pProSLICMLT->capFaults.measRG2
#define MLT_TC_CHG_CTR1         pProSLICMLT->capFaults.measAUX
#define MLT_TC_CHG_CTR2         pProSLICMLT->capFaults.measTG2
#define MLT_TC_DISCHG_V1        pProSLICMLT->capFaults.term.vloop[0]
#define MLT_TC_DISCHG_V2        pProSLICMLT->capFaults.term.vloop[1]
#define MLT_TC_DISCHG_V3        pProSLICMLT->capFaults.term.vloop[2]
#define MLT_TC_T1               pProSLICMLT->capFaults.time1
#define MLT_TC_T2               pProSLICMLT->capFaults.time2
#define MLT_TC_T3               pProSLICMLT->capFaults.time3
#define MLT_TC_DISCHG_DV1      (fp_abs(MLT_TC_DISCHG_V2 - MLT_TC_DISCHG_V1))
#define MLT_TC_DISCHG_DV2      (fp_abs(MLT_TC_DISCHG_V3 - MLT_TC_DISCHG_V2))
#define MLT_TC_SLOPE_ERR        pProSLICMLT->capFaults.slopeErr
#define MLT_TC_ISRC             pProSLICMLT->capFaults.term.iloop[0]
#define MLT_TC_RUN_COUNT        pProSLICMLT->capFaults.auxFlag
#define MLT_TC_MEAS_CHG_PHASE   pProSLICMLT->capFaults.tgFlag
#define MLT_TC_USE_V1_V2_ONLY   pProSLICMLT->capFaults.rgFlag
#define MLT_TC_USE_V2_V3_ONLY   pProSLICMLT->capFaults.trFlag
/*
** Test Condition Flags
*/
#define MLT_TC_COND_V1_LOW          pProSLICMLT->capFaults.flags[0]
#define MLT_TC_COND_V1_HIGH         pProSLICMLT->capFaults.flags[1]
#define MLT_TC_COND_V2_LOW          pProSLICMLT->capFaults.flags[2]
#define MLT_TC_COND_V2_HIGH         pProSLICMLT->capFaults.flags[3]
#define MLT_TC_COND_V3_LOW          pProSLICMLT->capFaults.flags[4]
#define MLT_TC_COND_V3_HIGH         pProSLICMLT->capFaults.flags[5]
#define MLT_TC_COND_DV1_LOW         pProSLICMLT->capFaults.flags[6]
#define MLT_TC_COND_DV1_HIGH        pProSLICMLT->capFaults.flags[7]
#define MLT_TC_COND_DV2_LOW         pProSLICMLT->capFaults.flags[8]
#define MLT_TC_COND_DV2_HIGH        pProSLICMLT->capFaults.flags[9]
#define MLT_TC_COND_SLOPE_ERR       pProSLICMLT->capFaults.flags[10]
#define MLT_TC_COND_REDUCE_DV_LIM   pProSLICMLT->capFaults.flags[11]
#define MLT_TC_COND_V1_MID          pProSLICMLT->capFaults.flags[12]
#define MLT_TC_COND_V2_MID          pProSLICMLT->capFaults.flags[13]
#define MLT_TC_COND_V3_MID          pProSLICMLT->capFaults.flags[14]
/*
** Thresholds
*/
#define MLT_TC_TH_V1_LOW            10000
#define MLT_TC_TH_V1_HIGH           46000
#define MLT_TC_TH_V2_LOW            3000
#define MLT_TC_TH_V2_HIGH           43000
#define MLT_TC_TH_V3_LOW            3000
#define MLT_TC_TH_V3_HIGH           40000
#define MLT_TC_TH_DV1_LOW           (MLT_TC_COND_REDUCE_DV_LIM ? 1000:1800)
#define MLT_TC_TH_DV1_HIGH          35000
#define MLT_TC_TH_DV2_LOW           (MLT_TC_COND_REDUCE_DV_LIM ? 1000:1800)
#define MLT_TC_TH_DV2_HIGH          35000
#define MLT_TC_TH_SLOPE_ERR         150
#define MLT_TC_TH_V_MID             20000
/*
** Other constants 
*/
#define MLT_TC_MAX_RUNS             5
#define MLT_TC_CAL_ISRC             500L
/*
** Tests
*/
#define MLT_TC_TEST_ZERO            (MLT_TC_COND_V1_LOW && MLT_TC_COND_V2_LOW && MLT_TC_COND_V3_LOW)
#define MLT_TC_TEST_V1_OK           (!(MLT_TC_COND_V1_LOW||MLT_TC_COND_V1_HIGH))
#define MLT_TC_TEST_V2_OK           (!(MLT_TC_COND_V2_LOW||MLT_TC_COND_V2_HIGH))
#define MLT_TC_TEST_V3_OK           (!(MLT_TC_COND_V3_LOW||MLT_TC_COND_V3_HIGH))
#define MLT_TC_TEST_V2_V3_LOW       (MLT_TC_COND_V2_LOW && MLT_TC_COND_V3_LOW && MLT_TC_TEST_V1_OK)
#define MLT_TC_TEST_V3_LOW          (MLT_TC_COND_V3_LOW && MLT_TC_TEST_V1_OK && MLT_TC_TEST_V2_OK)
#define MLT_TC_TEST_DV1_OK          (!(MLT_TC_COND_DV1_LOW||MLT_TC_COND_DV1_HIGH))
#define MLT_TC_TEST_DV2_OK          (!(MLT_TC_COND_DV2_LOW||MLT_TC_COND_DV2_HIGH))
#define MLT_TC_TEST_ALL_V_OK        (MLT_TC_TEST_V1_OK && MLT_TC_TEST_V2_OK && MLT_TC_TEST_V3_OK)
#define MLT_TC_TEST_ALL_V_HIGH      (MLT_TC_COND_V1_HIGH && MLT_TC_COND_V2_HIGH && MLT_TC_COND_V3_HIGH)
#define MLT_TC_TEST_V_OK_DV1_LOW    (MLT_TC_TEST_ALL_V_OK && MLT_TC_COND_DV1_LOW)
#define MLT_TC_TEST_V3_LOW_DV1_OK   (MLT_TC_TEST_V3_LOW && MLT_TC_TEST_DV1_OK)
#define MLT_TC_TEST_SLOPE_ERR       (MLT_TC_TEST_ALL_V_OK && MLT_TC_TEST_DV1_OK && MLT_TC_TEST_DV2_OK && MLT_TC_COND_SLOPE_ERR)
#define MLT_TC_TEST_V1_HIGH_V3_OK   (MLT_TC_COND_V1_HIGH && MLT_TC_TEST_V3_OK)
#define MLT_TC_TEST_LOW_I_V3_MID    (MLT_TC_COND_V3_MID && MLT_TC_TEST_V3_OK && (MLT_TC_ISRC == MLT_TC_CAL_ISRC)) 
#define MLT_TC_TEST_V1_MID          (MLT_TC_COND_V1_MID)
#define MLT_TC_TEST_V2_MID          (MLT_TC_COND_V2_MID)
#define MLT_TC_TEST_V3_MID          (MLT_TC_COND_V3_MID)


static int estimateCtr(ProSLICMLTType *pProSLICMLT, ProSLIC_mlt_capacitance_state *pState)
{
uInt32 mlt_line_cap;
uInt32 mlt_emi_cap;
uInt16 mlt_poll_rate;
#ifdef MLT_RUNTIME_CONFIG
    mlt_poll_rate = pProSLICMLT->mlt_config.mlt_poll_rate;
    mlt_line_cap  = pProSLICMLT->mlt_config.mlt_line_cap;
    mlt_emi_cap   = pProSLICMLT->mlt_config.mlt_emi_cap;
#else
    mlt_poll_rate = MLT_POLL_RATE;
    mlt_line_cap  = MLT_LINE_CAPACITANCE;
    mlt_emi_cap   = MLT_EMI_CAPACITANCE;
#endif

    switch(pState->MeasState.stage)
    {
        case 0:
            Si3226x_SetPowersaveMode(pProSLICMLT->pProslic,PWRSAVE_DISABLE);
            pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN,MLT_COMM_RAM_LCROFFHK,0xFFFFFFFL);/*max out threshold*/
            pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN,MLT_COMM_RAM_LONGHITH,0xFFFFFFFL);/*max out threshold*/
		    pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN,MLT_COMM_REG_LINEFEED,LF_TIP_OPEN);
            gndOpenTerm(pProSLICMLT->pProslic,TRUE);
            /* Initialize time constant measurement parameters */
            MLT_TC_T1 = 6;
            MLT_TC_T2 = 22;
            MLT_TC_T3 = 44;
            MLT_TC_ISRC = MLT_TC_CAL_ISRC;
            MLT_TC_MEAS_CHG_PHASE = 0;  /* Do discharge only measurement */
            MLT_TC_RUN_COUNT = 0; 
            MLT_TC_USE_V1_V2_ONLY = 0;
            MLT_TC_USE_V2_V3_ONLY = 0;
            MLT_TC_COND_REDUCE_DV_LIM = 0; 
            pState->TimeConstState.stage = 0;
            pState->TimeConstState.waitIterations = 0;
            pState->MeasState.stage++;
            return RC_MLT_TEST_RUNNING;

        case 1: /* Time constant measurement */
            if(measureTimeConstant(pProSLICMLT,&(pProSLICMLT->capFaults),&(pState->TimeConstState)))
            {
                /* Process results of calibration run */
                MLT_TC_COND_V1_LOW    = (MLT_TC_DISCHG_V1 < MLT_TC_TH_V1_LOW )    ? 1 : 0;
                MLT_TC_COND_V1_HIGH   = (MLT_TC_DISCHG_V1 > MLT_TC_TH_V1_HIGH )   ? 1 : 0;
                MLT_TC_COND_V2_LOW    = (MLT_TC_DISCHG_V2 < MLT_TC_TH_V2_LOW )    ? 1 : 0;
                MLT_TC_COND_V2_HIGH   = (MLT_TC_DISCHG_V2 > MLT_TC_TH_V2_HIGH )   ? 1 : 0;
                MLT_TC_COND_V3_LOW    = (MLT_TC_DISCHG_V3 < MLT_TC_TH_V3_LOW )    ? 1 : 0;
                MLT_TC_COND_V3_HIGH   = (MLT_TC_DISCHG_V3 > MLT_TC_TH_V3_HIGH )   ? 1 : 0;
                MLT_TC_COND_DV1_LOW   = (MLT_TC_DISCHG_DV1 < MLT_TC_TH_DV1_LOW )  ? 1 : 0;
                MLT_TC_COND_DV1_HIGH  = (MLT_TC_DISCHG_DV1 > MLT_TC_TH_DV1_HIGH ) ? 1 : 0;
                MLT_TC_COND_DV2_LOW   = (MLT_TC_DISCHG_DV2 < MLT_TC_TH_DV2_LOW )  ? 1 : 0;
                MLT_TC_COND_DV2_HIGH  = (MLT_TC_DISCHG_DV2 > MLT_TC_TH_DV2_HIGH ) ? 1 : 0;
                MLT_TC_COND_SLOPE_ERR = (MLT_TC_SLOPE_ERR > MLT_TC_TH_SLOPE_ERR)  ? 1 : 0;
                MLT_TC_COND_V1_MID    = (MLT_TC_DISCHG_V1 > MLT_TC_TH_V_MID)      ? 1 : 0;
                MLT_TC_COND_V2_MID    = (MLT_TC_DISCHG_V2 > MLT_TC_TH_V_MID)      ? 1 : 0;
                MLT_TC_COND_V3_MID    = (MLT_TC_DISCHG_V3 > MLT_TC_TH_V_MID)      ? 1 : 0;

                MLT_TC_USE_V1_V2_ONLY = 0;
                MLT_TC_USE_V2_V3_ONLY = 0;

                /* ------------------
                ** Analysis of result
                ** ------------------
                */

                /*
                ** All voltages low - extrememly fast time constant
                ** or R is out of max range (500uA*R > VBAT), call it 0nF
                */
                if(MLT_TC_TEST_ZERO)
                {
                    MLT_DEBUG_LOG("si3226x_mlt :  estimateCtr : STATE :  MLT_TC_TEST_ZERO\n");
                    MLT_TC_CTR = 0;
                    pState->MeasState.stage = 20;
                }
                /*
                ** All voltages high - extremely slow time constant, 
                *  increase drive current and measurement window
                */
                else if(MLT_TC_TEST_ALL_V_HIGH)
                {
                    MLT_DEBUG_LOG("si3226x_mlt :  estimateCtr : STATE :  MLT_TC_TEST_ALL_V_HIGH\n");
                    MLT_TC_T1 += 10;
                    MLT_TC_T2 += 20;
                    MLT_TC_T3 += 20;
                    MLT_TC_ISRC *= 2;
                    pState->TimeConstState.stage = 0;
                    pState->TimeConstState.waitIterations = 0;
                    pState->MeasState.stage = 1;
                }
                /*
                ** All voltages in range, but small dV
                ** indicates a large C, but some R is present 
                ** resulting in a voltage drop.  Cannot source
                ** too much current because of I*R drop, so
                ** capture times must be primary variant.
                **
                ** dV limit is dynamic.  Ideally, it would be as
                ** large as possible, but with large series R,
                ** it needs to be reduced to accomodate minimized
                ** voltage range.  If V1 is < 20v, do not increase
                ** current drive
                */
                else if(MLT_TC_TEST_V_OK_DV1_LOW)
                {
                    MLT_DEBUG_LOG("si3226x_mlt :  estimateCtr : STATE :  MLT_TC_TEST_V_OK_DV1_LOW\n");
                    /* Select ISRC based on dV1 */
                    MLT_TC_COND_REDUCE_DV_LIM = 1;
                    if((MLT_TC_DISCHG_DV1 < 300)&&(MLT_TC_TEST_V1_MID)) /* C in 30-50uF Range */
                    {
                        MLT_TC_ISRC += 500;  
                        MLT_TC_T1 = 20;
                        MLT_TC_T2 = 60;
                        MLT_TC_T3 = 100;
                    }
                    else if((MLT_TC_DISCHG_DV1 < 500)&&(MLT_TC_TEST_V1_MID)) /* C in 20-40uF Range */
                    {
                        MLT_TC_ISRC += 500; 
                        MLT_TC_T1 = 20;
                        MLT_TC_T2 = 50;
                        MLT_TC_T3 = 80;
                    }
                    else if((MLT_TC_DISCHG_DV1 < 800)&&(MLT_TC_TEST_V1_MID))
                    {
                        MLT_TC_ISRC += 500;  
                        MLT_TC_T1 = 20;
                        MLT_TC_T2 = 40;
                        MLT_TC_T3 = 60;
                    }
                    else  /* dV between 800 and 1000 */
                    {
                        /* Don't change Isrc...just increase time */
                        MLT_TC_T1 = 20;  
                        MLT_TC_T2 = 60;  
                        MLT_TC_T3 = 100;  
                    }

                    pState->TimeConstState.stage = 0;
                    pState->TimeConstState.waitIterations = 0;
                    pState->MeasState.stage = 1;
                }
                /*
                ** Medium C.  Use V1 and V2 to compute result
                ** Increase t2 if enough margin to improve 
                ** accuracy
                */
                else if(MLT_TC_TEST_V3_LOW_DV1_OK)
                {
                    MLT_DEBUG_LOG("si3226x_mlt :  estimateCtr : STATE :  MLT_TC_TEST_V3_LOW_DV1_OK\n");
                    MLT_TC_USE_V1_V2_ONLY = 1;
                    if(MLT_TC_DISCHG_V2 > 14000)
                    {
                        MLT_TC_T2 += 4;
                    }

                    pState->TimeConstState.stage = 0;
                    pState->TimeConstState.waitIterations = 0;
                    if(MLT_TC_RUN_COUNT == 0)
                    {                    
                        pState->MeasState.stage = 1;
                    }
                    else
                    {
                        pState->MeasState.stage++;
                    }
                }
                /*
                ** Fast Time Constant - first move t2,t3 for
                ** 300nF-800nF range, then move all 3 capture times
                ** for < 500nF
                **
                ** 
                */
                else if(MLT_TC_TEST_V2_V3_LOW)
                {
                    MLT_DEBUG_LOG("si3226x_mlt :  estimateCtr : STATE :  MLT_TC_TEST_V2_V3_LOW\n");
                    if(MLT_TC_RUN_COUNT == 0)
                    {
                        MLT_TC_T2 = 12;
                        MLT_TC_T3 = 18;
                        pState->MeasState.stage = 1;
                    }
                    else if(MLT_TC_RUN_COUNT == 1)
                    {
                        MLT_TC_T1 = 4;
                        MLT_TC_T2 = 8;
                        MLT_TC_T3 = 14;
               
                        MLT_TC_USE_V1_V2_ONLY = 1;
                        pState->MeasState.stage =1;
                    }
                    else /* Can't reduce any further, assume 0nF */
                    {
                        MLT_TC_CTR = 0;
                        pState->MeasState.stage = 20;
                    }
                    pState->TimeConstState.stage = 0;
                    pState->TimeConstState.waitIterations = 0;
                }
                /*
                ** Differing slopes - indicates inductor present.
                ** Use V2 and V3 to compute result since V1 is
                ** likely measured during the transient
                */
                else if(MLT_TC_TEST_SLOPE_ERR)
                {
                    MLT_DEBUG_LOG("si3226x_mlt :  estimateCtr : STATE :  MLT_TC_TEST_SLOPE_ERR\n");
                    MLT_TC_USE_V2_V3_ONLY = 1;
                    pState->TimeConstState.stage = 0;
                    pState->TimeConstState.waitIterations = 0;
                    pState->MeasState.stage++;
                }
                /*
                ** V1 out of range, but V3 OK - double current
                */
                else if(MLT_TC_TEST_V1_HIGH_V3_OK)
                {
                    MLT_DEBUG_LOG("si3226x_mlt :  estimateCtr : STATE :  MLT_TC_TEST_V1_HIGH_V3_OK\n");
                    MLT_TC_ISRC *= 2;
                    pState->TimeConstState.stage = 0;
                    pState->TimeConstState.waitIterations = 0;
                    pState->MeasState.stage = 1;
                }
                /*
                ** For 500uA case, if V3 > 20v, bump up
                ** current to 700uA to improve accuracy
                */
                else if(MLT_TC_TEST_LOW_I_V3_MID)
                {
                    MLT_DEBUG_LOG("si3226x_mlt :  estimateCtr : STATE :  MLT_TC_TEST_LOW_I_V3_MID\n");
                    MLT_TC_ISRC = 700;
                    pState->TimeConstState.stage = 0;
                    pState->TimeConstState.waitIterations = 0;
                    pState->MeasState.stage = 1;
                }
                /*
                ** Initial measurement in range
                */
                else
                {
                    MLT_DEBUG_LOG("si3226x_mlt :  estimateCtr : STATE :  NO_CHANGE\n");
                    pState->MeasState.stage++;
                }
                MLT_TC_RUN_COUNT++;

                /* Limit number of iterations */
                if(MLT_TC_RUN_COUNT > MLT_TC_MAX_RUNS)
                {
                    pState->MeasState.stage++;
                }
                return RC_MLT_TEST_RUNNING;
            }
            return RC_MLT_TEST_RUNNING;

        case 2:
            MLT_TC_MEAS_CHG_PHASE = 1;
            pState->MeasState.stage++;
            return RC_MLT_TEST_RUNNING;

        case 3: /* Optional up/down measurement */
            if(!MLT_TC_MEAS_CHG_PHASE)
            {
                pState->MeasState.stage++;
                return RC_MLT_TEST_RUNNING;
            }

            if(measureTimeConstant(pProSLICMLT,&(pProSLICMLT->capFaults),&(pState->TimeConstState)))
            {
                pState->MeasState.stage++;
                return RC_MLT_TEST_RUNNING;
            }
            return RC_MLT_TEST_RUNNING;

        case 4:
            if((MLT_TC_USE_V1_V2_ONLY)&&(MLT_TC_DISCHG_CTR1 < 300))
            {
                MLT_DEBUG_LOG("si3226x_mlt :  estimateCtr : METHOD_V1_V2 : DISCHG_CTR1_ONLY\n");
                MLT_TC_CTR = MLT_TC_DISCHG_CTR1;
            }
            else if((MLT_TC_USE_V1_V2_ONLY)&&(MLT_TC_DISCHG_CTR1 >= 300))
            {
                MLT_DEBUG_LOG("si3226x_mlt :  estimateCtr : METHOD_V1_V2 : CTR1_AVG\n");
                MLT_TC_CTR = (3L*MLT_TC_DISCHG_CTR1+MLT_TC_CHG_CTR1)/4L;
            }
            else if(MLT_TC_USE_V2_V3_ONLY)
            {
                MLT_DEBUG_LOG("si3226x_mlt :  estimateCtr : METHOD_V2_V3 : DISCHG_CTR2_ONLY\n");
                MLT_TC_CTR = MLT_TC_DISCHG_CTR2;
            }
            else
            {
                /* Don't average large C's where offset current has little effect */
                if(MLT_TC_DISCHG_CTR1 > 10000)
                {
                    MLT_DEBUG_LOG("si3226x_mlt :  estimateCtr : METHOD_ALL : DISCHG_CTR1_CTR2_AVG\n");
                    MLT_TC_CTR = (MLT_TC_DISCHG_CTR1 + MLT_TC_DISCHG_CTR2)/2;
                }
                else
                {
                    MLT_DEBUG_LOG("si3226x_mlt :  estimateCtr : METHOD_ALL : CTR1_AVG\n");
                    MLT_TC_CTR = (3L*MLT_TC_DISCHG_CTR1+MLT_TC_CHG_CTR1)/4L;
                }
            }

            /* Compensate for EMI and linefeed capacitance */
            MLT_TC_CTR -= (mlt_emi_cap + (mlt_line_cap/2))/10;
            MLT_DEBUG_LOG("estimateCtr :  Final Cavg = %d nF\n", MLT_TC_CTR);
            pState->MeasState.stage = 20;
            return RC_MLT_TEST_RUNNING;

        case 20:
            gndOpenTerm(pProSLICMLT->pProslic,FALSE);
		    ProSLIC_SetLinefeedStatus(pProSLICMLT->pProslic, LF_OPEN); 
            return RC_MLT_TEST_COMPLETE;  
    }
    return RC_MLT_TEST_COMPLETE; 
}




/********************************************************************************/
/**
** @brief Measure REN using subthreshold ringing method
** 
** @param[in,out] *pProSLICMLT -  Pointer to MLT channel structure.  Ouput is located under pProSLICMLT->ren.renValue
** @param[in] *pState - pointer to test state structure
**
** @retval typical values: @ref RC_MLT_TEST_RUNNING or @ref RC_MLT_TEST_COMPLETE are normal values returned. @sa MLT_RETURN_CODES for other possible values.
**
**
** @remark 
**  - This function is reentrant and should be called at periodic intervals defined by mlt_poll_rate
**  - USER MODE status is not modified in this function
**  - Uses 16vrms 20Hz ring signal and fits ringing current to calibrated curve to determine REN
**
*/
static int32 si3226x_mlt_ren_subthresh_ring_method(ProSLICMLTType *pProSLICMLT, ProSLIC_mlt_ren_state *pState)
{
/* in this test we ring the phone at 16Vrms with 18Vdc.
*  We measure the loop current and interpolate the REN value
*  Loop current is AC component only and is rectified and averaged. 
*/
uInt8 regData,irq2Reg,irq3Reg;
int32 temp;
int32 Ren;
#ifdef MLT_REN_TEST_FREQ
int test_freq = MLT_REN_TEST_FREQ;
#endif
int i;
uInt16 mlt_poll_rate;
#ifdef MLT_RUNTIME_CONFIG
    mlt_poll_rate = pProSLICMLT->mlt_config.mlt_poll_rate;
#else
    mlt_poll_rate = MLT_POLL_RATE;
#endif
switch (pState->State.stage){
	case 0:
	    setUserMode(pProSLICMLT->pProslic,TRUE);
	    si3226x_preserve_state(pProSLICMLT);
		Si3226x_SetPowersaveMode(pProSLICMLT->pProslic,PWRSAVE_DISABLE);
	    setupDcFeedForRENTest(pProSLICMLT); /*set up low V_VLIM so we are guaranteed to start ringing*/
	    ProSLIC_SetLinefeedStatus(pProSLICMLT->pProslic, LF_FWD_ACTIVE); /*go forward active to charge the line to V_VLIM*/
	    pState->State.sampleIterations=0;
	    pState->State.waitIterations=0;
	    pState->State.stage++;
	    return RC_MLT_TEST_RUNNING;

	case 1:
	    delay_poll(&(pState->State),200/mlt_poll_rate - 2 ); /*wait 5 tau*/
	    return RC_MLT_TEST_RUNNING;

	case 2:
		pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN,MLT_COMM_RAM_LCROFFHK,0xFFFFFFFL);
		pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN,MLT_COMM_RAM_LONGHITH,0xFFFFFFFL);
	    pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN,MLT_COMM_RAM_VBATR_EXPECT,0x3f00000L); /*vbatr_expect ~= 60*/
	    pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN,MLT_COMM_REG_AUTO,0x2e); /*disable battery tracking*/
        
#ifdef MLT_REN_TEST_FREQ
        if(test_freq == 16)
        {
	        pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN,MLT_COMM_RAM_RINGFR,0x7F5A800L); /*16Hz*/
	        pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN,MLT_COMM_RAM_RINGAMP,0x72000L);  /*16Vrms*/
        }
        else
        {
	        pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN,MLT_COMM_RAM_RINGFR,0x7efe000L); /*20Hz*/
	        pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN,MLT_COMM_RAM_RINGAMP,0x92000L);  /*16Vrms*/
        }
#else
	    pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN,MLT_COMM_RAM_RINGFR,0x7efe000L); /*20Hz*/
	    pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN,MLT_COMM_RAM_RINGAMP,0x92000L);  /*16Vrms*/
#endif
	    pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN,MLT_COMM_RAM_RINGOF,0x1CF4B64L);/*18Vdc */
	    pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN,MLT_COMM_RAM_RINGPHAS,0);
	    pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN,MLT_COMM_RAM_RTPER,0x50000L);
        pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN,MLT_COMM_RAM_VOV_RING_GND,0x51EB80L);
	    pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN,MLT_COMM_RAM_VCM_RING,0x1680000L);/*set vcm_ring so we dont clip*/
	    pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN,MLT_COMM_REG_RINGCON,0);/*disable timers*/
	    pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN,MLT_COMM_RAM_COUNTER_VTR_VAL,0x51EB8L);/*couter_vtr-val*/
	    temp = pProSLICMLT->ReadReg(pMLT_HW, pMLT_CHAN,MLT_COMM_REG_IRQ3);/*clear power alarm and ringtrip interrupts*/
	    pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN,MLT_COMM_REG_IRQ3,(uInt8)temp); /*required for GCI to clear*/
	    temp = pProSLICMLT->ReadReg(pMLT_HW, pMLT_CHAN,MLT_COMM_REG_IRQ2);/*clear*/
	    pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN,MLT_COMM_REG_IRQ2,(uInt8)temp);
	    pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN,MLT_COMM_REG_IRQEN3,0x3);/* P_THERM_IE, P_HVIC_IE */
	    pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN,MLT_COMM_REG_IRQEN2,0x1);/* RTP_IE */
	    ProSLIC_SetLinefeedStatus(pProSLICMLT->pProslic, LF_RINGING);/*start ringing*/
	    pState->max=0;
	    pState->State.stage++;

	    return RC_MLT_TEST_RUNNING;

	case 3:
	    delay_poll(&(pState->State),250/mlt_poll_rate - 2);/*wait one ringing cycle to make sure ringing has started*/
	    return RC_MLT_TEST_RUNNING;

	case 4:
	    regData=pProSLICMLT->ReadReg(pMLT_HW, pMLT_CHAN,MLT_COMM_REG_LINEFEED);/*check we made it to ringing state*/
		if (regData != 0x44) /* Ringing failed to start/persist */
		{
			MLT_DEBUG_LOG("ProSLIC MLT : REN : Linefeed = 0x%0X\n", regData);
			/* Check interrupts */
			irq2Reg = pProSLICMLT->ReadReg(pMLT_HW, pMLT_CHAN,MLT_COMM_REG_IRQ2);
			irq3Reg = pProSLICMLT->ReadReg(pMLT_HW, pMLT_CHAN,MLT_COMM_REG_IRQ3);
			/* Restore entry state and force failing value */
		    ProSLIC_SetLinefeedStatus(pProSLICMLT->pProslic, LF_OPEN);
		    si3226x_restore_state(pProSLICMLT);
	        setUserMode(pProSLICMLT->pProslic,FALSE);
			pProSLICMLT->ren.renValue = 999999L;
			/* Check for ringtrip */
			if( irq2Reg & 0x01 )
			{
				MLT_DEBUG_LOG("ProSLIC MLT : REN : Ringtrip on Ring Start\n");  
				return RC_MLT_FALSE_RINGTRIP;
			}

			/* Check for power alarm */
			if (irq3Reg & 0x03)
			{ 
				MLT_DEBUG_LOG("ProSLIC MLT : REN : Power/Thermal Alarm on Ring Start\n");
				return RC_MLT_ALARM_ON_RING;
		    } 

			/* If not false ringtrip or alarm, return unknown err */
			MLT_DEBUG_LOG("ProSLIC MLT : REN : Ring Start Failed\n");
			return RC_MLT_RINGSTART_ERR;
	    }

	    temp = pProSLICMLT->ReadRAM(pMLT_HW, pMLT_CHAN,MLT_COMM_RAM_MADC_ILOOP); 

	    if (temp&0x10000000L)
		    temp |= 0xf0000000L; /*sign extend negative value*/
	    pState->v[pState->State.sampleIterations++] = temp; /*store sample*/
		MLT_DEBUG_LOG("sample %d\tIloop = %d\n",pState->State.sampleIterations,(temp/10));

	    if (pState->State.sampleIterations == (MLT_REN_SAMPLE_TIME/mlt_poll_rate))
        { /*we are done collecting samples - calculate*/

		    ProSLIC_SetLinefeedStatus(pProSLICMLT->pProslic, LF_FWD_ACTIVE);/* ACTIVE before OPEN */
		    temp=0;

		    /*calculate dc value */
		    for (i=0;i<MLT_REN_SAMPLE_TIME/mlt_poll_rate;i++){
			    temp += pState->v[i];
		    }
		    temp /= MLT_REN_SAMPLE_TIME/mlt_poll_rate;
		    for (i=0;i<MLT_REN_SAMPLE_TIME/mlt_poll_rate;i++){
			    pState->v[i] -= temp; /*remove dc*/
		    }
		    temp =0;

		    /*calculate avg current*/
		    for (i=0;i<MLT_REN_SAMPLE_TIME/mlt_poll_rate;i++){
			    if (pState->v[i] < 0) /*recitfy the signal*/
				    pState->v[i] *= -1; 
			    temp += pState->v[i]/10;
		    }
		    temp /= MLT_REN_SAMPLE_TIME/mlt_poll_rate;
			MLT_DEBUG_LOG ("I Avg = %d\n",temp);
		    pState->max = temp; 

		    /*we are cal'ing test - return raw value*/
		    if (pProSLICMLT->ren.renCalFlag){
	            pState->State.stage++;
			    pProSLICMLT->ren.renValue = pState->max;
			    return RC_MLT_TEST_RUNNING;
		    }

		    /*interpolate from cal values*/
		    if(pState->max > pProSLICMLT->ren.calData.renTrans)
					    Ren = ((pState->max  - pProSLICMLT->ren.calData.highRenOffs)*1000)/pProSLICMLT->ren.calData.highRenSlope;
				    else
					    Ren = ((pState->max  - pProSLICMLT->ren.calData.lowRenOffs)*1000)/pProSLICMLT->ren.calData.lowRenSlope;
		    if (Ren < 900)
			    Ren = ((pState->max  - pProSLICMLT->ren.calData.extraLowRenOffset)*1000)/pProSLICMLT->ren.calData.extraLowRenSlope;
    		
		    if (Ren < 0)
			    Ren = 0; 
		    pProSLICMLT->ren.renValue = Ren;

			MLT_DEBUG_LOG ("REN RAW = %d\n",Ren);
     
	        pState->State.stage++;

		    return RC_MLT_TEST_RUNNING;
	    }
	    return RC_MLT_TEST_RUNNING;

        case 5: /* wait 1 ring period for ring exit */
          	delay_poll(&(pState->State),50/mlt_poll_rate - 2 ); 
	        return RC_MLT_TEST_RUNNING;  

        case 6:

		    ProSLIC_SetLinefeedStatus(pProSLICMLT->pProslic, LF_OPEN);
		    si3226x_restore_state(pProSLICMLT);
	        setUserMode(pProSLICMLT->pProslic,FALSE);
            return RC_MLT_TEST_COMPLETE;
    }
	return RC_MLT_TEST_COMPLETE;
}
/**@}*/ 
/* end of SI3226X_DRV_STATIC */

/*
** Function: si3226x_mlt_abort - documented in si3226x_mlt.h
*/
int si3226x_mlt_abort(ProSLICMLTType *pProSLICMLT,ProSLIC_mlt_test_state *pState)
{
uInt16 mlt_poll_rate;
#ifdef MLT_RUNTIME_CONFIG
    mlt_poll_rate = pProSLICMLT->mlt_config.mlt_poll_rate;
#else
    mlt_poll_rate = MLT_POLL_RATE;
#endif
	switch (pState->stage)
	{
		case 0:
            pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN,MLT_COMM_REG_LINEFEED, LF_OPEN);
			pState->stage++;
			return 0;
		case 1:
			delay_poll(pState,50/mlt_poll_rate -2 );
			return 0;
        case 2:
			si3226x_restore_state(pProSLICMLT);
			pState->stage++;
            return 1;
	}
    return 1;
}



/*
** Function: si3226x_mlt_foreign_voltages - documented in si3226x_mlt.h
*/
int si3226x_mlt_foreign_voltages(ProSLICMLTType *pProSLICMLT, ProSLIC_mlt_foreign_voltages_state *pState){ 
    int i;
	int32 data;
	int32 tmp;
    int32 vtsqrt, vrsqrt, vtrsqrt;
uInt16 mlt_poll_rate;
#ifdef MLT_RUNTIME_CONFIG
    mlt_poll_rate = pProSLICMLT->mlt_config.mlt_poll_rate;
#else
    mlt_poll_rate = MLT_POLL_RATE;
#endif
    /* Start of reentrant block */
	switch (pState->State.stage){

		case 0: /* Enable user mode, store entry settings, and initialize state structure */
            setUserMode(pProSLICMLT->pProslic,TRUE); 
		    si3226x_preserve_state(pProSLICMLT); 
			Si3226x_SetPowersaveMode(pProSLICMLT->pProslic,PWRSAVE_DISABLE);
		    if(pState->samples > MLT_MAX_FEMF_SAMPLES) 
                pState->samples = MLT_MAX_FEMF_SAMPLES;  
		    pState->State.waitIterations=0;
		    pState->State.sampleIterations=0;
		    pState->State.stage++;
		    setupDcFeedCloseToZero (pProSLICMLT);  
		    Si3226x_SetLinefeedStatus(pProSLICMLT->pProslic, 1); 
		    return RC_MLT_TEST_RUNNING;

		case 1: /* Settle */
		    delay_poll(&(pState->State),MLT_TS_HAZV_LINE_DISCHARGE/(mlt_poll_rate) - 2); 
		    return RC_MLT_TEST_RUNNING;

		case 2: /* Go Open for Hi-Z measurement */
		    ProSLIC_SetLinefeedStatus(pProSLICMLT->pProslic, LF_OPEN); /*disconnect from line*/
		    pState->State.stage++;
		    return RC_MLT_TEST_RUNNING;

		case 3: /* Setup coarse sensors, enable diag, select VTIPC */
            setupCoarseSensors(pProSLICMLT, &(pState->tipOffs), &(pState->ringOffs));
            diagSelect(pProSLICMLT->pProslic, DIAG_SEL_VTIPC, MADC_HIRES);
            diagSet(pProSLICMLT->pProslic, DIAG_ENABLE);
	
			MLT_DEBUG_LOG ("OFFSET (vtipc) = %d mV\n",pState->tipOffs);

		    pState->State.stage++;
		    return RC_MLT_TEST_RUNNING;

		case 4: /* Read VTIPC samples, remove offset on each sample */
            tmp = ReadMADCScaled(pProSLICMLT,MLT_COMM_RAM_MADC_DIAG_RAW,0) - MLT_CONST_COARSE_SENSE_VOFFS;
            pState->vt[pState->State.sampleIterations] = tmp - pState->tipOffs;
		    pState->State.sampleIterations++;
		    if (pState->State.sampleIterations == pState->samples)
			    pState->State.stage++;
		    return RC_MLT_TEST_RUNNING;

		case 5: /* Reset sample counter, select VRINGC */
		    pState->State.sampleIterations=0;
            diagSet(pProSLICMLT->pProslic, DIAG_CLR_DIAG1);
            diagSelect(pProSLICMLT->pProslic, DIAG_SEL_VRINGC, MADC_HIRES);
            diagSet(pProSLICMLT->pProslic, DIAG_ENABLE);

			MLT_DEBUG_LOG ("OFFSET (vringc) = %d mV\n",pState->ringOffs);

		    pState->State.stage++;
		    return RC_MLT_TEST_RUNNING;

		case 6: /* Read VRINGC samples, remove offset on each sample */
            tmp = ReadMADCScaled(pProSLICMLT,MLT_COMM_RAM_MADC_DIAG_RAW,0) - MLT_CONST_COARSE_SENSE_VOFFS;
		    pState->vr[pState->State.sampleIterations] = tmp - pState->ringOffs;
		    pState->State.sampleIterations++;
		    if (pState->State.sampleIterations == pState->samples) {
            #ifdef MLT_HAZV_VTR_ENABLED
			    pState->State.stage++;
            #else
                diagSet(pProSLICMLT->pProslic, DIAG_CLR_DIAG1);
                pState->State.stage = 9;
            #endif
            }
		    return RC_MLT_TEST_RUNNING;

        case 7: /* Differential AC Measurement - Normal Res */
            diagSet(pProSLICMLT->pProslic, DIAG_CLR_DIAG1);
            pState->State.waitIterations = 0;
            pState->State.sampleIterations = 0;
            pState->State.stage++;
            return RC_MLT_TEST_RUNNING;

        case 8: /* Read coarse VTR samples */
            pState->vtr[pState->State.sampleIterations] = ReadMADCScaled(pProSLICMLT,MLT_COMM_RAM_VDIFF_COARSE,0);
            pState->State.sampleIterations++;
            if(pState->State.sampleIterations == pState->samples) {
                pState->State.stage++;
            }
            return RC_MLT_TEST_RUNNING;

        case 9: /* Cleanup */
	        si3226x_restore_state(pProSLICMLT); 
            setUserMode(pProSLICMLT->pProslic, FALSE);
            pState->State.stage++;
            return RC_MLT_TEST_RUNNING;          
	}
	
    /*
    ** Calculate DC Voltages
    */

    pProSLICMLT->hazVDC.measTG = 0;
    pProSLICMLT->hazVDC.measRG = 0;
    pProSLICMLT->hazVDC.measTR = 0;
    pProSLICMLT->hazVDC.measAUX = 0;

    for (i = 0; i < pState->samples; i++)
    {
	    pProSLICMLT->hazVDC.measTG += pState->vt[i];
	    pProSLICMLT->hazVDC.measRG += pState->vr[i];
	    pProSLICMLT->hazVDC.measAUX += pState->vtr[i];
    }
    pProSLICMLT->hazVDC.measTG /= pState->samples;
    pProSLICMLT->hazVDC.measRG /= pState->samples;
    pProSLICMLT->hazVDC.measAUX /= pState->samples;
    #ifdef MLT_HAZV_VTR_ENABLED
    pProSLICMLT->hazVDC.measTR = pProSLICMLT->hazVDC.measTG - pProSLICMLT->hazVDC.measRG;
    #endif

    /*
    ** Calculate AC Voltages ( units of 100&Vrms^2 )
    */
    for (i = 0; i < pState->samples; i++)
    {
	    pState->vt[i] -= pProSLICMLT->hazVDC.measTG;
	    pState->vr[i] -= pProSLICMLT->hazVDC.measRG;
	    pState->vtr[i] -= pProSLICMLT->hazVDC.measAUX;
    }

    pProSLICMLT->hazVAC.measTG = 0;
    pProSLICMLT->hazVAC.measRG = 0;
    pProSLICMLT->hazVAC.measTR = 0;

    for (i = 0; i < pState->samples; i++)
    {
	    pProSLICMLT->hazVAC.measTG += (pState->vt[i] / 100) * (pState->vt[i] / 100);
	    pProSLICMLT->hazVAC.measRG += (pState->vr[i] / 100) * (pState->vr[i] / 100);
        #ifdef MLT_HAZV_VTR_ENABLED
	    pProSLICMLT->hazVAC.measTR += (pState->vtr[i] / 100) * (pState->vtr[i] / 100);
        #endif
    }

    pProSLICMLT->hazVAC.measTG /= pState->samples;
    pProSLICMLT->hazVAC.measRG /= pState->samples;

    /* Skip rest if VTR measurement disabled */
#ifdef MLT_HAZV_VTR_ENABLED
    pProSLICMLT->hazVAC.measTR /= pState->samples;

    /* Below 10v, trAC may not be very accurate due to having to use normal madc mode  */
    /* If trAC is < 10v, check to see if tgAC + rgAC is within 25% of trAC.  If so, it */
    /* likely that Vtg and Vrg are out of phase and Vtr may be calculated as Vtg+Vrg   */
    /* If the sum is not within 25% of trAC, do not modify trAC because voltage on     */
    /* tg and rg must be common mode.    */

    vtsqrt = 10 * Isqrt(pProSLICMLT->hazVAC.measTG);
    vrsqrt = 10 * Isqrt(pProSLICMLT->hazVAC.measRG);
    vtrsqrt = 10 * Isqrt(pProSLICMLT->hazVAC.measTR);

    /* Calculate error between sum of longitudinal voltages and measured differential */
    if (vtrsqrt > 0)
      {
	  data = 100 * fp_abs((vtsqrt + vrsqrt) - vtrsqrt);
	  data /= vtrsqrt;
    } else
	data = 100;

    if (data < 0)
	data = -data;

	MLT_DEBUG_LOG("si3226x_mlt: VtgSQ = %d\tVtg = %d\n", pProSLICMLT->hazVAC.measTG, vtsqrt);
	MLT_DEBUG_LOG("si3226x_mlt: VrgSQ = %d\tVrg = %d\n", pProSLICMLT->hazVAC.measRG, vrsqrt);
	MLT_DEBUG_LOG("si3226x_mlt: VtrSQ = %d\tVtr = %d\n", pProSLICMLT->hazVAC.measTR, vtrsqrt);


    if (data < 25)
    {
	    pProSLICMLT->hazVAC.measTR = vtsqrt + vrsqrt;
	    pProSLICMLT->hazVAC.measTR /= 10;	/* prevent overflow */
	    pProSLICMLT->hazVAC.measTR *= pProSLICMLT->hazVAC.measTR;
    }
#endif

    /*
    ** Invert voltage polarity relative to GND
    */
	pProSLICMLT->hazVDC.measTR *= -1;
	pProSLICMLT->hazVDC.measTG *= -1;
	pProSLICMLT->hazVDC.measRG *= -1;
	if (pProSLICMLT->hazVAC.measTG < 0)
		pProSLICMLT->hazVAC.measTG *=-1;
	if (pProSLICMLT->hazVAC.measRG < 0)
		pProSLICMLT->hazVAC.measRG *=-1;

	return RC_MLT_TEST_COMPLETE;
}




/*
** Function: si3226x_mlt_hazard_voltages - documented in si3226x_mlt.h
*/
int si3226x_mlt_hazard_voltages(ProSLICMLTType *pProSLICMLT,ProSLIC_mlt_foreign_voltages_state *pState)
{
	return si3226x_mlt_foreign_voltages(pProSLICMLT,pState);
}



/*
** Function: si3226x_mlt_resistive_faults - documented in si3226x_mlt.h
*/
int si3226x_mlt_resistive_faults(ProSLICMLTType *pProSLICMLT,ProSLIC_mlt_rmeas_state *pState)
{
	int32 vc2;
	int32 data;
    int   alarm;
uInt16 mlt_poll_rate;
#ifdef MLT_RUNTIME_CONFIG
    mlt_poll_rate = pProSLICMLT->mlt_config.mlt_poll_rate;
#else
    mlt_poll_rate = MLT_POLL_RATE;
#endif
    /* Start of reentrant block */
    switch(pState->setupState.stage){

        case 0:  /* Setup diag current source for T-R dvdt measurement*/
            setUserMode(pProSLICMLT->pProslic,TRUE);
	        si3226x_preserve_state(pProSLICMLT);

			/* Diable Low Power Mode */
		    ProSLIC_SetPowersaveMode(pProSLICMLT->pProslic,PWRSAVE_DISABLE);

			/* Set VBATH to 70v */
			pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN,MLT_COMM_RAM_VBATH_EXPECT,MLT_RES_VBATH_SET);

			/* Initialize return values and methods */
	        pProSLICMLT->resFaults.measTG = MLT_CONST_MAX_RES;
	        pProSLICMLT->resFaults.measRG = MLT_CONST_MAX_RES;
	        pProSLICMLT->resFaults.measTR = MLT_CONST_MAX_RES;
	        pProSLICMLT->resFaults.measAUX = MLT_CONST_MAX_RES;
			pProSLICMLT->resFaults.ahsFlag = 0;
			pProSLICMLT->resFaults.trFlag = 0;
			pProSLICMLT->resFaults.rgFlag = 0;
			pProSLICMLT->resFaults.tgFlag = 0;
			pProSLICMLT->resFaults.auxFlag = 0;
            pState->rtrMethod = RESFAULT_METHOD_UNDEFINED;
            pState->rtgMethod = RESFAULT_METHOD_UNDEFINED;
            pState->rrgMethod = RESFAULT_METHOD_UNDEFINED;

            pState->smallRFlag=0; 
            pState->setupState.waitIterations=0;
            pState->setupState.stage++;
            return RC_MLT_TEST_RUNNING;
           
		case 1:
	        pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN,MLT_COMM_RAM_LCROFFHK,MLT_MAX_I_THRESH);
	        pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN,MLT_COMM_RAM_LONGHITH,MLT_MAX_I_THRESH);
	        /* Go Active*/
            pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN,MLT_COMM_REG_LINEFEED,LF_FWD_ACTIVE);  
            pProSLICMLT->ReadReg(pMLT_HW, pMLT_CHAN,MLT_COMM_REG_IRQ3);
            pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN,MLT_COMM_REG_IRQEN3,0x1);
            pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN,MLT_COMM_RAM_P_TH_HVIC, MLT_CONST_P_TH_HVIC );

	        /* Setup dc current source = 2V*0.31957mA/V = 639.14uA*/ 
            diagSet(pProSLICMLT->pProslic, DIAG_CLR_DIAG1);            
            pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN,MLT_COMM_RAM_RINGAMP,0);
            pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN,MLT_COMM_RAM_RINGFR,0);
            pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN,MLT_COMM_RAM_SLOPE_RING,0x1f000000L);/*set slope_ring*/
            pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN,MLT_COMM_RAM_RINGOF,0x337A28L); /*2V*/
            diagSet(pProSLICMLT->pProslic,DIAG_ENABLE|DIAG_DCLPF_44K|DIAG_FB_OFF);
           
            pState->setupState.stage++;
            return RC_MLT_TEST_RUNNING;

        case 2: /* Wait*/
            delay_poll(&(pState->setupState),400/mlt_poll_rate - 2);
            return RC_MLT_TEST_RUNNING;

        case 3: /* Setup diag current source for V2 measurement (time constant)*/
			pState->v1 = ReadMADCScaled(pProSLICMLT,MLT_COMM_RAM_VDIFF_FILT,0);
            pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN,MLT_COMM_RAM_RINGOF,0x1FCC85D8L); 
            pState->setupState.stage++;
            return RC_MLT_TEST_RUNNING;

        case 4: /* wait - fixed 20ms wait period */
            delay_poll(&(pState->setupState),20/mlt_poll_rate - 2);
            return RC_MLT_TEST_RUNNING;

		case 5: /* Measure T-R dV/dt to set auto-v step size and settle times */
			vc2 = ReadMADCScaled(pProSLICMLT,MLT_COMM_RAM_VDIFF_FILT,0);
			pState->dvdt_tr = fp_abs(pState->v1 - vc2);
			pState->dvdt_tr /= 20L;

		    MLT_DEBUG_LOG("si3226x_mlt : resFaults : dV/dt est : %d v/s\n",pState->dvdt_tr);

			diagSet(pProSLICMLT->pProslic,DIAG_ENABLE);
            diagSet(pProSLICMLT->pProslic, DIAG_CLR_DIAG1);
            /*check for small fault to ground - if it is present we probe further in individual tests.*/
            setupDcFeed35VCM(pProSLICMLT);
            pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN,MLT_COMM_REG_LINEFEED,LF_FWD_ACTIVE);
            pState->setupState.stage++;
            return RC_MLT_TEST_RUNNING;

        case 6:
            delay_poll(&(pState->setupState),250/mlt_poll_rate -2 );
            return RC_MLT_TEST_RUNNING;

        case 7: 
		    /* Estimate range of fault by looking at current on TIP and RING.
		    ** If less than 10mA (large R) do auto-v method, otherwise, 
		    ** use normal dc feed.
			*/
            data = ReadMADCScaled(pProSLICMLT, MLT_COMM_RAM_MADC_ITIP,0);
            if(data < -(10000L)) 
            {
                pState->smallRFlag = 1;
                MLT_DEBUG_LOG("si3226x_mlt : resFaults : Small-R ITIP = %d uA : smallRFlag = %d\n",data,pState->smallRFlag);
            }

            data = ReadMADCScaled(pProSLICMLT, MLT_COMM_RAM_MADC_IRING,0);
            if (data < -(10000)) 
            {
                pState->smallRFlag |= 2;
				MLT_DEBUG_LOG("si3226x_mlt : resFaults : Small-R IRING = %d uA : smallRFlag = %d\n",data,pState->smallRFlag);
            }
            pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN,MLT_COMM_REG_LINEFEED,LF_OPEN);
            pState->TRState.stage = 0;
            pState->TGState.stage = 0;
            pState->RGState.stage = 0;
            pState->setupState.stage++;
            return RC_MLT_TEST_RUNNING;

        case 8:
            /* Skip T-R test if smallRFlag is set - must be TG or RG */
            if(pState->smallRFlag) {
                pState->setupState.stage++;
                pProSLICMLT->resFaults.measTR = MLT_CONST_OPEN_RES;
                return RC_MLT_TEST_RUNNING;
            }
#ifdef MLT_RES_RTR_DISABLE
			pState->setupState.stage++;
			return RC_MLT_TEST_RUNNING;
#endif

            if(measRtr(pProSLICMLT,pState)) 
            {
                /* If small Rtr, skip all other tests */
                if(pProSLICMLT->resFaults.trFlag == 1)
                {
                    pState->setupState.stage = 11;
                    return RC_MLT_TEST_RUNNING;
                }
                pState->setupState.stage++;  /* do auto-v method */
                return RC_MLT_TEST_RUNNING;
            }
           
            /* If power alarm occurs (usually due to small Rrg) skip RTR */
            alarm = pProSLICMLT->ReadReg(pMLT_HW, pMLT_CHAN,MLT_COMM_REG_IRQ3);
            if (alarm) { 
                MLT_DEBUG_LOG("si3226x_mlt :  resfaults  : Aborted RTR Measurement : Alarm = %d",alarm);
                pState->setupState.stage++; 
                pProSLICMLT->resFaults.measTR = MLT_CONST_MAX_RES;
            }
            return RC_MLT_TEST_RUNNING;

        case 9:
#ifdef MLT_RES_RRG_DISABLE
			pState->setupState.stage++;
			return RC_MLT_TEST_RUNNING;
#endif
            if(measRrg(pProSLICMLT,pState)) {
                pState->setupState.stage++;
                return RC_MLT_TEST_RUNNING;
            }
            /* If power alarm occurs (usually due to small Rrg) skip RTR */
            alarm = pProSLICMLT->ReadReg(pMLT_HW, pMLT_CHAN,MLT_COMM_REG_IRQ3);
            if (alarm) 
            { 
			    MLT_DEBUG_LOG("si3226x_mlt : resFaults : Aborted RRG Measurement : Alarm = %d",alarm);
                pState->setupState.stage++; 
                pProSLICMLT->resFaults.measRG = MLT_CONST_MAX_RES;
            }
            return RC_MLT_TEST_RUNNING;

         case 10:
#ifdef MLT_RES_RTG_DISABLE
			pState->setupState.stage++;
			return RC_MLT_TEST_RUNNING;
#endif
            if(measRtg(pProSLICMLT,pState)) {
                pState->setupState.stage++;
                return RC_MLT_TEST_RUNNING;
            }
            /* If power alarm occurs (usually due to small Rrg) skip RTR */
            alarm = pProSLICMLT->ReadReg(pMLT_HW, pMLT_CHAN,MLT_COMM_REG_IRQ3);
            if (alarm) 
			{ 
				MLT_DEBUG_LOG("si3226x_mlt : resFaults : Aborted RTG Measurement : Alarm = %d",alarm);
                pState->setupState.stage++; 
                pProSLICMLT->resFaults.measTG = MLT_CONST_MAX_RES;
            }
            return RC_MLT_TEST_RUNNING;

        case 11:
            si3226x_restore_state(pProSLICMLT);
            setUserMode(pProSLICMLT->pProslic,FALSE);
            /* post-measurement analysis */

            /* 
			** Post-measurement analysis 
			**
			** Keep measurement results for dominant (smallest) fault only,
			** which is applicable when supporting single-fault model.
			**
			** Leave compile option to return raw values for user to post
			** process on their own if they wish to examine 2T values.
			** 
			*/
			MLT_DEBUG_LOG("si3226x_mlt : resFaults : RTR Meas  = %d\n", pProSLICMLT->resFaults.measTR);
			MLT_DEBUG_LOG("si3226x_mlt : resFaults : RRG Meas  = %d\n", pProSLICMLT->resFaults.measRG);
			MLT_DEBUG_LOG("si3226x_mlt : resFaults : RTG Meas  = %d\n", pProSLICMLT->resFaults.measTG);
	
			/* 
			** Cap each measurement at 2Mohm 
			*/
			if(pProSLICMLT->resFaults.measRG > MLT_CONST_MAX_RES) pProSLICMLT->resFaults.measRG = MLT_CONST_MAX_RES;
			if(pProSLICMLT->resFaults.measTG > MLT_CONST_MAX_RES) pProSLICMLT->resFaults.measTG = MLT_CONST_MAX_RES;
			if(pProSLICMLT->resFaults.measTR > MLT_CONST_MAX_RES) pProSLICMLT->resFaults.measTR = MLT_CONST_MAX_RES;

			pState->fault_term = MLT_TERM_TR;  /* Default to TR */
			/* 
			** Isolate single fault
			*/

			/*
			** RG Check:
			**     1.  RRG < 2Mohm
			**     2.  RRG < RTR
			**     3.  RRG+20% < RTG
			*/
			if((pProSLICMLT->resFaults.measRG < MLT_CONST_MAX_RES)&&
			   (pProSLICMLT->resFaults.measRG < pProSLICMLT->resFaults.measTR)&&
			   (12L*pProSLICMLT->resFaults.measRG/pProSLICMLT->resFaults.measTG) < 10L)
			{
					pState->fault_term = MLT_TERM_RG;
					pProSLICMLT->resFaults.measTR = MLT_CONST_MAX_RES;
					pProSLICMLT->resFaults.measTG = MLT_CONST_MAX_RES;
			}
			/*
			** TG Check:
			**     1.  RTG < 2Mohm
			**     2.  RTG < RTR
			**     3.  RTG+20% < RRG
			*/
			else if((pProSLICMLT->resFaults.measTG < MLT_CONST_MAX_RES)&&
			   (pProSLICMLT->resFaults.measTG < pProSLICMLT->resFaults.measTR)&&
			   (12L*pProSLICMLT->resFaults.measTG/pProSLICMLT->resFaults.measRG < 10L))
			{
					pState->fault_term = MLT_TERM_TG;
					pProSLICMLT->resFaults.measTR = MLT_CONST_MAX_RES;
					pProSLICMLT->resFaults.measRG = MLT_CONST_MAX_RES;
			}
			/*
			** TR Check:
			**     1.  RTR < 2Mohm
			**     2.  RTG & RRG within 20% of each other
			**     Note:  RTR may be less than RRG and/or RTG since their measurements
			**            include RTR + small leakage of HVIC switch.       
			*/
			else if((pProSLICMLT->resFaults.measTR < MLT_CONST_MAX_RES)&&
					(10L*pProSLICMLT->resFaults.measTG/pProSLICMLT->resFaults.measRG < 12L)&&
					(10L*pProSLICMLT->resFaults.measRG/pProSLICMLT->resFaults.measTG < 12L))
			{
					pState->fault_term = MLT_TERM_TR;
					pProSLICMLT->resFaults.measRG = MLT_CONST_MAX_RES;
					pProSLICMLT->resFaults.measTG = MLT_CONST_MAX_RES;
			}
			/*
			** TR Re-Check:
			**		1.  RRG != RTG
			**		2.  RTR < RRG
			**      3.  RTR < RTG
			**
			**      In this case, the fault is from TR, but because of the OPEN lead
			**      leakage compensation, small, but larger than RTR faults are detected
			**      on RG and TG. 
			*/
			else if((pProSLICMLT->resFaults.measTR < MLT_CONST_MAX_RES)&&
					(pProSLICMLT->resFaults.measTR < pProSLICMLT->resFaults.measRG)&&
					(pProSLICMLT->resFaults.measTR < pProSLICMLT->resFaults.measTG))
			{
					pState->fault_term = MLT_TERM_TR;
					pProSLICMLT->resFaults.measRG = MLT_CONST_MAX_RES;
					pProSLICMLT->resFaults.measTG = MLT_CONST_MAX_RES;
			}
			return RC_MLT_TEST_COMPLETE;  
			break;

    }
    return RC_MLT_TEST_COMPLETE;
}

/*
** Function:   si3226x_mlt_receiver_offhook - documented in si3226x_mlt.h
*/
int si3226x_mlt_receiver_offhook(ProSLICMLTType *pProSLICMLT, ProSLIC_mlt_roh_state *pState)
{
	ProslicRAMInit dcfeed7mA [] = {
		{MLT_COMM_RAM_SLOPE_VLIM,0x1EB48978L},
		{MLT_COMM_RAM_SLOPE_RFEED,0x1FDA6949L},
		{MLT_COMM_RAM_SLOPE_ILIM,0x40A0E0L},
		{MLT_COMM_RAM_SLOPE_RING,0x1CFCA14CL},
		{MLT_COMM_RAM_SLOPE_DELTA1,0x1907F1D3L},
		{MLT_COMM_RAM_SLOPE_DELTA2,0x1AD45894L},
		{MLT_COMM_RAM_V_VLIM,0x5A38633L},
		{MLT_COMM_RAM_V_RFEED,0x55B5917L},
		{MLT_COMM_RAM_V_ILIM,0x3E67006L},
		{MLT_COMM_RAM_CONST_RFEED,0x526775L},
		{MLT_COMM_RAM_CONST_ILIM,0x209246L},
		{MLT_COMM_RAM_I_VLIM,0x2D8D96L},
		{0xFFFF, 0xFFFFFFFFL} /*end flag*/
	};
	ProslicRAMInit dcfeed14mA [] = {
		{MLT_COMM_RAM_SLOPE_VLIM,0x1B9C5AA7L},
		{MLT_COMM_RAM_SLOPE_RFEED,0x1FC0DB63L},
		{MLT_COMM_RAM_SLOPE_ILIM,0x40A0E0L},
		{MLT_COMM_RAM_SLOPE_RING,0x1CFCA14CL},
		{MLT_COMM_RAM_SLOPE_DELTA1,0x1E119F8AL},
		{MLT_COMM_RAM_SLOPE_DELTA2,0x1E46C831L},
		{MLT_COMM_RAM_V_VLIM,0x5A38633L},
		{MLT_COMM_RAM_V_RFEED,0x55B5917L},
		{MLT_COMM_RAM_V_ILIM,0x3E67006L},
		{MLT_COMM_RAM_CONST_RFEED,0x7F6F07L},
		{MLT_COMM_RAM_CONST_ILIM,0x41248DL},
		{MLT_COMM_RAM_I_VLIM,0x2D8D96L},
		{0xFFFF, 0xFFFFFFFFL} /*end flag*/
	};
	ProslicRegInit empty [] = {
		{0xFF,0xFF} /*end flag*/
	};
	
	int32 iloop;
	int32 vdiff;
	int32 zDiff;
	int32 zDiffLimit = MLT_ROH_MAX_ZDIFF;
    int32 zDiffREN = MLT_ROH_MIN_ZDIFF_REN;
	int32 RmaxTR = MLT_ROH_MAX_ZTR;
uInt16 mlt_poll_rate;
#ifdef MLT_RUNTIME_CONFIG
    mlt_poll_rate = pProSLICMLT->mlt_config.mlt_poll_rate;
#else
    mlt_poll_rate = MLT_POLL_RATE;
#endif   
    /* Start of reentrant loop */
 	switch (pState->State.stage){

		case 0: /* Set usermode, store entry conditions, setup DC feed for 7mA source, go active */
            setUserMode(pProSLICMLT->pProslic,TRUE);
		    si3226x_preserve_state(pProSLICMLT);
		    Si3226x_SetPowersaveMode(pProSLICMLT->pProslic,PWRSAVE_DISABLE);
		    pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN,MLT_COMM_RAM_LCROFFHK,0xFFFFFFFL);/*max out threshold*/
		    pState->State.waitIterations=0;
		    pState->State.sampleIterations=0;
		    Si3226x_LoadRegTables(&(pProSLICMLT->pProslic),dcfeed7mA,empty,1);
		    ProSLIC_SetLinefeedStatus(pProSLICMLT->pProslic, LF_FWD_ACTIVE); 
		    pState->State.stage++;
		    return RC_MLT_TEST_RUNNING;

		case 1: /* settle */
		    delay_poll(&(pState->State),MLT_TS_ROH_MEAS1/mlt_poll_rate - 2);
		    return RC_MLT_TEST_RUNNING;

		case 2: /* Measure I & V, calc first Ztr, setup 14mA source */
            iloop = ReadMADCScaled(pProSLICMLT, MLT_COMM_RAM_MADC_ILOOP, 0);
            vdiff = ReadMADCScaled(pProSLICMLT, MLT_COMM_RAM_VDIFF_FILT, 0);
		    if (iloop > MLT_ROH_MIN_ILOOP)
			    pState->Rv1 = ((vdiff*1000)  / iloop)*100;
		    else
			    pState->Rv1 = 1000000000;
		    if (vdiff < MLT_ROH_MIN_VLOOP)
		    	pState->Rv1 = 1;  /* call it a short */
		    Si3226x_LoadRegTables(&(pProSLICMLT->pProslic),dcfeed14mA,empty,1);
		    pState->State.stage++;
		    return RC_MLT_TEST_RUNNING;

		case 3:  /* settle */
		    delay_poll(&(pState->State),MLT_TS_ROH_MEAS2/mlt_poll_rate - 2);
		    return RC_MLT_TEST_RUNNING;

		case 4: /* Measure I & V, calc second Ztr, computations */
            iloop = ReadMADCScaled(pProSLICMLT, MLT_COMM_RAM_MADC_ILOOP, 0);
            vdiff = ReadMADCScaled(pProSLICMLT, MLT_COMM_RAM_VDIFF_FILT, 0);     
		    if (iloop > MLT_ROH_MIN_ILOOP) 
			    pState->Rv2 = ((vdiff*1000)  / iloop)*100;
		    else
			    pState->Rv2 = 1000000000;
		    if (vdiff < MLT_ROH_MIN_VLOOP)
			    pState->Rv2 = 1; /* call it a short */

			MLT_DEBUG_LOG("Rv1 = %d\nRv2 = %d\n",pState->Rv1,pState->Rv2);

            /* If both impedances are high, no fault or offhook cpe device */
		    if ((pState->Rv1 == 1000000000) && (pState->Rv2 == 1000000000)) {
			    pProSLICMLT->roh.rohTrue = 0;  /* no fault, roh passed */
            }
		    else {
			    zDiff = pState->Rv1-pState->Rv2;
			    zDiff = ((zDiff)*100)/pState->Rv1;
			    if (zDiff<0) zDiff*=-1;

			    MLT_DEBUG_LOG("zDiff = %d\n",zDiff);

                /*Qualify resistance and resistance differences*/
    			
			    if((pState->Rv1 == 1)&& (zDiff >= zDiffLimit)) {   /* < 100ohms*/
				    pProSLICMLT->roh.rohTrue = RC_MLT_ROH_FAIL_RESFAULT;
                }
			    else if((zDiff >= zDiffLimit)&&((pState->Rv1/100)<RmaxTR)&&((pState->Rv2/100)<RmaxTR)) {
					    pProSLICMLT->roh.rohTrue = RC_MLT_ROH_FAIL_ROH;
                }
			    else {
				    if(((pState->Rv1/100 < 10000) || (pState->Rv2/100 < 10000)) && (zDiff < zDiffREN) ) 
					    pProSLICMLT->roh.rohTrue = RC_MLT_ROH_FAIL_RESFAULT;
				    else
					    pProSLICMLT->roh.rohTrue = 0;
			    }
		    }
            si3226x_restore_state(pProSLICMLT);
            setUserMode(pProSLICMLT->pProslic, FALSE);
            pState->State.stage++;
            return RC_MLT_TEST_COMPLETE;
	}
	return RC_MLT_TEST_COMPLETE;
}

/*
** Function:   si3226x_mlt_ren - documented in si3226x_mlt.h 
*/
int si3226x_mlt_ren(ProSLICMLTType *pProSLICMLT,ProSLIC_mlt_ren_state *pState){ 
	
	return si3226x_mlt_ren_subthresh_ring_method (pProSLICMLT,pState);

	 
}

/* 
** Function:  si3226x_mlt_capacitance - documented in si3226x_mlt.h
*/

int si3226x_mlt_capacitance(ProSLICMLTType *pProSLICMLT,ProSLIC_mlt_capacitance_state *pState)
{
int32 trC, rgC, tgC;
int32 iForcep = 5000;
int32 iForcen = -5000;
uInt16 mlt_poll_rate;
#ifdef MLT_RUNTIME_CONFIG
    mlt_poll_rate = pProSLICMLT->mlt_config.mlt_poll_rate;
#else
    mlt_poll_rate = MLT_POLL_RATE;
#endif
    switch (pState->State.stage)
    {
        case 0:
            setUserMode(pProSLICMLT->pProslic, TRUE);
            si3226x_preserve_state(pProSLICMLT);	/*save register settings */
            Si3226x_SetPowersaveMode(pProSLICMLT->pProslic,PWRSAVE_DISABLE);
            setup300HzBandpass(pProSLICMLT);
            pState->freq = 300;
            pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN, MLT_COMM_RAM_RA_EN_B, 0x300000L);	/*ra_sum disconnect */
            pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN, MLT_COMM_RAM_COMP_Z, 0x0L);	/*comp_z disable */
            pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN, MLT_COMM_RAM_EZSYNTH_B0, 0L);
            pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN, MLT_COMM_RAM_RXACGAIN, 0x2000000L);
            pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN, MLT_COMM_RAM_RXACGAIN_SAVE, 0x2000000L);

            pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN, MLT_COMM_REG_DIGCON, 0x1C);	/*disable hyb */
            /* Force on audio path */
            pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN, MLT_COMM_RAM_AUDIO_MAN, 0x300000L);	/*force audio on */
            pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN, MLT_COMM_RAM_PD_BIAS, 0x200000L);	/*pd_bias */
            pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN, MLT_COMM_RAM_PD_AC_ADC, 0x200000L);	/*pd_adc */
            pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN, MLT_COMM_RAM_PD_AC_DAC, 0x200000L);	/*pd_dac */
            pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN, MLT_COMM_RAM_PD_AC_SNS, 0x200000L);	/*pd_ac_sns */
            pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN, MLT_COMM_REG_DIAG1, 0x10);	/* */

            MLT_DEBUG_LOG("si3226x_mlt : Capacitance : TIP-RING\n");

            Si3226x_SetLinefeedStatus(pProSLICMLT->pProslic, LF_FWD_ACTIVE);	/*enable line driver */
            /* Setup diag current source */

            setupDiagCurrentSource(pProSLICMLT->pProslic, iForcep);
            pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN, MLT_COMM_REG_DIAG1, 0xB);	/* */

            pState->State.stage++;
	        pState->State.waitIterations=0;
            pState->MeasState.stage = 0;
            return RC_MLT_TEST_RUNNING;

        case 1: /* bias settle */
	        delay_poll(&pState->State, 600 / mlt_poll_rate - 2);
            return RC_MLT_TEST_RUNNING;

        case 2:
            pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN, MLT_COMM_REG_RA, 0);	/* */
            pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN, MLT_COMM_RAM_EZSYNTH_B0, 0L);	/* */
            pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN, MLT_COMM_RAM_COMP_Z, 0L);	/* */
            pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN, MLT_COMM_RAM_AUDIO_MAN, 0x300000L);	/*force audio on */
            pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN, MLT_COMM_RAM_OSC1FREQ, 0x7C70000L);	/* */
            pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN, MLT_COMM_RAM_OSC1AMP, 0x2A000L);	/*force audio on */
            pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN, MLT_COMM_REG_OMODE, 0x2);	/* */
            pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN, MLT_COMM_REG_OCON, 0x1);	/* */
            pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN, MLT_COMM_REG_DIAG1, 0x1B);	/* */
            pState->State.stage++;
            return RC_MLT_TEST_RUNNING;

        case 3: /* osc settle */
	        delay_poll(&pState->State, 600 / mlt_poll_rate - 2);
            return RC_MLT_TEST_RUNNING;

        case 4:  /* Measure TIP-RING Capacitance */
            if(measCapacitanceAC(pProSLICMLT,pState)) 
            {
                pProSLICMLT->capFaults.measTR = processTestavoForC(pProSLICMLT,pState,MLT_TERM_TR);

                MLT_DEBUG_LOG("si3226x_mlt : Capacitance : tr_raw = %d.%d nF\n", pProSLICMLT->capFaults.measTR / 10, pProSLICMLT->capFaults.measTR % 10);


                /* Setup for RING-GND Measurement */
                pState->ram1447 = pProSLICMLT->ReadRAM(pMLT_HW, pMLT_CHAN, MLT_COMM_RAM_HVIC_STATE);
                pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN, MLT_COMM_REG_DIAG1, 0x0);	/*disable testfilter */
                pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN, MLT_COMM_REG_OCON, 0x0);	/*disable osc1 */
                pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN, MLT_COMM_REG_LINEFEED, LF_TIP_OPEN);	/* tip-open */
                pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN, MLT_COMM_RAM_PD_CM, 0x300000L);	/*pd_cm */
                pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN, MLT_COMM_RAM_HVIC_STATE_MAN, 0x10000000L);
                pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN, MLT_COMM_REG_DIAG1, 0x0);	/*active delay*/
                pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN, MLT_COMM_RAM_HVIC_STATE, pState->ram1447 | 0x2f000L); 
                setupDiagCurrentSource(pProSLICMLT->pProslic, iForcep);
                pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN, MLT_COMM_REG_DIAG1, 0xB);	/* */
                pState->State.stage++;
                pState->MeasState.stage=0;

                MLT_DEBUG_LOG("si3226x_mlt : Capacitance : RING-GND\n");

            }
            return RC_MLT_TEST_RUNNING;

        case 5:
	        delay_poll(&pState->State, 500 / mlt_poll_rate - 2);
            return RC_MLT_TEST_RUNNING;

        case 6:
            pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN, MLT_COMM_REG_OMODE, 0x2);	/* */
            pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN, MLT_COMM_REG_OCON, 0x1);	/* */
            pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN, MLT_COMM_REG_DIAG1, 0x1B);	/* */
            pState->State.stage++;

            return RC_MLT_TEST_RUNNING;
        case 7:
	        delay_poll(&pState->State, 500 / mlt_poll_rate - 2);
            return RC_MLT_TEST_RUNNING;

        case 8:  /* Measure RING-GND Capacitance */
            if(measCapacitanceAC(pProSLICMLT,pState)) 
            {
                pProSLICMLT->capFaults.measRG = processTestavoForC(pProSLICMLT,pState,MLT_TERM_RG);

                MLT_DEBUG_LOG("si3226x_mlt : Capacitance : rg_raw = %d.%d nF\n", pProSLICMLT->capFaults.measRG / 10, pProSLICMLT->capFaults.measRG % 10);

                /* Setup for T-G Measurement */      
                pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN, MLT_COMM_RAM_HVIC_STATE, pState->ram1447);                    
                pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN, MLT_COMM_REG_DIAG1, 0x0);	/*disable testfilter */
                pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN, MLT_COMM_REG_OCON, 0x0);	/*disable osc1 */	
                pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN, MLT_COMM_REG_LINEFEED, LF_RING_OPEN);	
                pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN, MLT_COMM_RAM_PD_CM, 0x300000L);	/*pd_cm */
                pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN, MLT_COMM_RAM_HVIC_STATE_MAN, 0x10000000L);
                pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN, MLT_COMM_REG_DIAG1, 0x0);	/*active delay*/
                pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN, MLT_COMM_RAM_HVIC_STATE, pState->ram1447 | 0x82f000L); 
                setupDiagCurrentSource(pProSLICMLT->pProslic, iForcen);
                pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN, MLT_COMM_REG_DIAG1, 0xB);	/* */
                pState->State.stage++;
                pState->MeasState.stage=0; 

                MLT_DEBUG_LOG("si3226x_mlt : Capacitance : TIP-GND\n");
            }
            return RC_MLT_TEST_RUNNING;

        case 9:
	        delay_poll(&pState->State, 500 / mlt_poll_rate - 2);
            return RC_MLT_TEST_RUNNING;

        case 10:
            pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN, MLT_COMM_REG_OMODE, 0x2);	/* */
            pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN, MLT_COMM_REG_OCON, 0x1);	/* */
            pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN, MLT_COMM_REG_DIAG1, 0x1B);	/* */
            pState->State.stage++;
            return RC_MLT_TEST_RUNNING;

        case 11:
	        delay_poll(&pState->State, 500 / mlt_poll_rate - 2);
            return RC_MLT_TEST_RUNNING;

        case 12:  /* Measure TIP-GND Capacitance */
            if(measCapacitanceAC(pProSLICMLT,pState)) 
            {
                pProSLICMLT->capFaults.measTG = processTestavoForC(pProSLICMLT,pState,MLT_TERM_TG);

                MLT_DEBUG_LOG("si3226x_mlt : Capacitance : tg_raw = %d.%d nF\n", pProSLICMLT->capFaults.measTG / 10, pProSLICMLT->capFaults.measTG % 10);

                /* Restore Settings */  
                pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN, MLT_COMM_RAM_HVIC_STATE, pState->ram1447);                    
                pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN, MLT_COMM_REG_DIAG1, 0x0);	/*disable testfilter */
                pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN, MLT_COMM_REG_OCON, 0x0);	/*disable osc1 */
                pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN, MLT_COMM_REG_LINEFEED, LF_OPEN);	
                pProSLICMLT->WriteRAM(pMLT_HW, pMLT_CHAN, MLT_COMM_RAM_PD_CM, 0L);	/*pd_cm */


                        /* Compute 3-terminal values */
                if (pProSLICMLT->capFaults.measTR < 0) pProSLICMLT->capFaults.measTR = 0L;
                if (pProSLICMLT->capFaults.measTG < 0) pProSLICMLT->capFaults.measTG = 0L;
                if (pProSLICMLT->capFaults.measRG < 0) pProSLICMLT->capFaults.measRG = 0L;

        #if (MLT_DISABLE_3TERM_CAP_COMPENSATION)
		        trC = pProSLICMLT->capFaults.measTR - (LINE_CAPACITANCE/2);
		        tgC = pProSLICMLT->capFaults.measTG - LINE_CAPACITANCE;
		        rgC = pProSLICMLT->capFaults.measRG - LINE_CAPACITANCE;
        #else
		        /*three-terminal compensation */
		        trC = (-pProSLICMLT->capFaults.measTG / 2) - (pProSLICMLT->capFaults.measRG / 2) + 2 * pProSLICMLT->capFaults.measTR;
		        tgC = (3 * pProSLICMLT->capFaults.measTG) / 2 + pProSLICMLT->capFaults.measRG / 2 - 2 * pProSLICMLT->capFaults.measTR;
		        rgC = pProSLICMLT->capFaults.measTG / 2 + (3 * pProSLICMLT->capFaults.measRG) / 2 - 2 * pProSLICMLT->capFaults.measTR;
        #endif
		        pProSLICMLT->capFaults.measTR = trC;
		        pProSLICMLT->capFaults.measRG = rgC;
		        pProSLICMLT->capFaults.measTG = tgC;

		        /*clip output values */
		        if (pProSLICMLT->capFaults.measTR < MLT_MIN_CAPACITANCE)
		            pProSLICMLT->capFaults.measTR = MLT_MIN_CAPACITANCE;
		        if (pProSLICMLT->capFaults.measTR > MLT_MAX_CAPACITANCE)
		            pProSLICMLT->capFaults.measTR = MLT_MAX_CAPACITANCE;
		        if (pProSLICMLT->capFaults.measTG < MLT_MIN_CAPACITANCE)
		            pProSLICMLT->capFaults.measTG = MLT_MIN_CAPACITANCE;
		        if (pProSLICMLT->capFaults.measTG > MLT_MAX_CAPACITANCE)
		            pProSLICMLT->capFaults.measTG = MLT_MAX_CAPACITANCE;
		        if (pProSLICMLT->capFaults.measRG < MLT_MIN_CAPACITANCE)
		            pProSLICMLT->capFaults.measRG = MLT_MIN_CAPACITANCE;
		        if (pProSLICMLT->capFaults.measRG > MLT_MAX_CAPACITANCE)
		            pProSLICMLT->capFaults.measRG = MLT_MAX_CAPACITANCE;                

                pState->State.stage = 70;

            }
            return RC_MLT_TEST_RUNNING;
    
        case 70:
            si3226x_restore_state(pProSLICMLT);
            setUserMode(pProSLICMLT->pProslic, FALSE);
            return RC_MLT_TEST_COMPLETE;
    }
    return RC_MLT_TEST_RUNNING;
}


/* 
** Function:  si3217x_mlt_cap_ren - documented in si3217x_mlt.h
*/

int si3226x_mlt_ren_cap(ProSLICMLTType *pProSLICMLT,ProSLIC_mlt_ren_cap_state *pState)
{
uInt16 mlt_poll_rate;
#ifdef MLT_RUNTIME_CONFIG
    mlt_poll_rate = pProSLICMLT->mlt_config.mlt_poll_rate;
#else
    mlt_poll_rate = MLT_POLL_RATE;
#endif
    switch (pState->State.stage)
    {
        case 0:
            setUserMode(pProSLICMLT->pProslic, TRUE);
            si3226x_preserve_state(pProSLICMLT);
		    Si3226x_SetPowersaveMode(pProSLICMLT->pProslic,PWRSAVE_DISABLE);
            pState->MeasState.stage = 0;
            pState->MeasState.waitIterations = 0;
            pState->MeasState.sampleIterations = 0;
            pState->State.stage++;
            return RC_MLT_TEST_RUNNING;

        case 1:
            if(estimateCtr(pProSLICMLT,pState))
            {
                pState->State.stage++;
            }
            return RC_MLT_TEST_RUNNING;

        case 2:
            si3226x_restore_state(pProSLICMLT);
            setUserMode(pProSLICMLT->pProslic, FALSE);
            return RC_MLT_TEST_COMPLETE;
    }
    return RC_MLT_TEST_RUNNING;

}

