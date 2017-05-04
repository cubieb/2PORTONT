/*
** Copyright (c) 2010-2011 by Silicon Laboratories
**
** $Id: proslic_mlt_macro.h 4551 2014-10-27 20:57:24Z nizajerk $
**
*/
/*! \file proslic_mlt_macro.h
**  \brief ProSLIC MLT commonly used macros
**  
**	This is the header file for commonly used macros
**
**  \author Silicon Laboratories, Inc (cdp)
**
**  \attention 
**	This file contains proprietary information.	 
**	No dissemination allowed without prior written permission from
**	Silicon Laboratories, Inc. 
*/

#ifndef PROSLIC_MLT_MACRO_H
#define PROSLIC_MLT_MACRO_H


/* Timer and Control Macros*/
#ifdef SI_USE_IFACE_MACROS
#define WriteReg		pProslic->deviceId->ctrlInterface->WriteRegister_fptr
#define ReadReg		    pProslic->deviceId->ctrlInterface->ReadRegister_fptr
#define pProHW		    pProslic->deviceId->ctrlInterface->hCtrl
#define Reset		    pProslic->deviceId->ctrlInterface->Reset_fptr
#define Delay		    pProslic->deviceId->ctrlInterface->Delay_fptr
#define pProTimer		pProslic->deviceId->ctrlInterface->hTimer
#define WriteRAM		pProslic->deviceId->ctrlInterface->WriteRAM_fptr
#define ReadRAM		    pProslic->deviceId->ctrlInterface->ReadRAM_fptr
#define getTime      	pProslic->deviceId->ctrlInterface->getTime_fptr
#define TimeElapsed 	pProslic->deviceId->ctrlInterface->timeElapsed_fptr
#define SetSemaphore 	pProslic->deviceId->ctrlInterface->Semaphore_fptr
#endif


/*
** Chip Type 
*/
#define CHIPTYPE           pProslic->deviceId->chipType
#define CHIPREV            pProslic->deviceId->chipRev
#define MLT_CHIPTYPE       pProSLICMLT->pProslic->deviceId->chipType
#define MLT_CHIPREV        pProSLICMLT->pProslic->deviceId->chipRev
#define pMLT_CHAN          pProSLICMLT->pProslic->channel
#define pMLT_HW            pProSLICMLT->pProHW
#define MLT17X_REVB       (pProSLICMLT->CHIPREV == SI3217X_REVB)
#define MLT17X_REVC       (pProSLICMLT->CHIPREV == SI3217X_REVC)
#define MLT26X_REVC       (pProSLICMLT->CHIPREV == SI3226X_REVC)

#define MLT_VALID_SI3217X_DEVICE ((int)((MLT_CHIPTYPE >= SI32171)&&(MLT_CHIPTYPE <= SI32179)))
#define MLT_VALID_SI3226X_DEVICE ((int)((MLT_CHIPTYPE >= SI32260)&&(MLT_CHIPTYPE <= SI32269)))

/*
** Commonly Used Macros
*/
#define MLT_MAX_PRESERVE_RAM   128
#define MLT_MAX_PRESERVE_REG   32
#define MLT_MAX_IV_SAMPLES     6
#define MLT_MAX_AUTOV_SAMPLES  128

#define INCREASE 1
#define DECREASE 0
#define OPEN 0
#define CLOSE 1
#define LOW 0
#define HIGH 1
#define TIP_TERM 0
#define RING_TERM 1

/*
   The following macros used by the ProSLIC MLT software are system constants
   that should not be modified.
*/
#define MLT_AUDIAG_SCALE_3014HZ     2992L	        /**< TESTAVO to impedance conversion at 3014Hz (2.992e-4)  */
#define MLT_AUDIAG_SCALE_300HZ      4091L	        /**< TESTAVO to impedance conversion at 300Hz (4.091e-4) */
#define MLT_MIN_CAPACITANCE         50L	            /**< Minimum capacitance to be reported (nF*10) */
#define MLT_MAX_CAPACITANCE         120000L	        /**< Maximum capacitance to be reported (nF*10) */
#define MLT_RS_CAPACITANCE_TEST     23656648L	    /**< Rkdc squared (4863.81ohms ^2) */
#define MLT_HVIC_SWITCH_RES         20L		        /**< Resistance of hvic switches to gnd (ohms*10) */
#define SI3226X_MLT_HVIC_SWITCH_RES 120L
#define MLT_RINGOF_IBIAS_P5         0x17C29D1L      /**< Diag current source +5mA */
#define MLT_RINGOF_IBIAS_N5         0x1E83D62FL     /**< Diag current source -5mA */
#define MLT_MAX_300HZ_TESTAVO       3000000L        /**< Maximum testavo measurement w/ 300Hz source */
#define MLT_RES_VBATH_SET           0x47AE125L      /**< VBATH for resistance test */
#define MLT_MAX_I_THRESH			0xFFFFFFFL

/*
**  RAM or Memory-mapped register locations used by patch
*/
enum {
    MLT17X_B_PSR_DSP_TIMED_VTR2_VAL = 999,
    MLT17X_B_PSR_DSP_TIMED_VTR1_VAL,
    MLT17X_B_PSR_DSP_STOP_TIMER2_VAL,
    MLT17X_B_PSR_DSP_STOP_TIMER1_VAL,
    MLT17X_B_PSR_DSP_STOP_TIMER2,
    MLT17X_B_PSR_DSP_STOP_TIMER1,
};

#endif
