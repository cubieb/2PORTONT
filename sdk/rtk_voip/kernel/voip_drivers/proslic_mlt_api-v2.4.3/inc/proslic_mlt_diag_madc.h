/*
** Copyright (c) 2009-2015 by Silicon Laboratories
**
** $Id: proslic_mlt_diag_madc.h 4714 2015-02-05 18:16:07Z nizajerk $
**/
/*! \file proslic_mlt_diag_madc.h
**  \brief ProSLIC mlt diag and madc utilities 
**
** This is the header file for mlt diag and madc utilities
**
** \author Silicon Laboratories, Inc (cdp)
**
** \attention
** This file contains proprietary information.	 
** No dissemination allowed without prior written permission from
** Silicon Laboratories, Inc.
**
*/

#ifndef PROSLIC_MLT_DIAG_MADC_H
#define PROSLIC_MLT_DIAG_MADC_H

/** @internal @defgroup PROSLIC_DIAG_MADC  ProSLIC MLT Diag and MADC APIs
 *  This group contains the APIs for diag and MADC functions used by the MLT device drivers.
 *  Called only from MLT device drivers and not higher level code.
 *
 *@{
 */


/** @internal @defgroup PROSLIC_MLT_CONSTANTS  ProSLIC MLT Constants
 *  This group contains computation constants that are to not be modified by user
 *  Called only from MLT device drivers and not higher level code.
 *
 *@{
 */

#define FULLSCALE_29BIT_2COMP 268435455L            /*!< @internal Maximum positive 29-bit 2's compliment */

/*
 * BOM dependent resistance measurement scaling constants 
 */

/* Extended Voltage BOM */
#define MLT_CONST_REQ_DIFF_EV_0			101L           /*!< @internal TR resistance calculation coef 0, std bom */
#define MLT_CONST_REQ_LONG_EV_0			365305L        /*!< @internal TG/RG resistance calculation coef 0, std bom */
#define MLT_CONST_REQ_LONG_EV_1			547958L        /*!< @internal TG/RG resistance calculation coef 1, std bom */
#define MLT_CONST_REQ_AUTO_LONG_EV		182L           /*!< @internal TG/RG resistance simplified calculation coef 0, std bom */
#define MLT_CONST_PM_REQ_DIFF_EV_0		197L           /*!< @internal TR resistance calculation coef 0, PM bom */
#define MLT_CONST_PM_REQ_LONG_EV_0		365305L        /*!< @internal TG/RG resistance calculation coef 0, PM bom */   
#define MLT_CONST_PM_REQ_LONG_EV_1		375391L        /*!< @internal TG/RG resistance calculation coef 1, PM bom */
#define MLT_CONST_MADC_VCOARSE_EV       639            /*!< @internal MADC coarse voltage scaling */
#define MLT_CONST_MADC_HIRES_V_EV       146            /*!< @internal MADC hires voltage scaling (1000*406)/100 */
/* Standard Voltage BOM */
#define MLT_CONST_REQ_DIFF_SV_0			114L           /*!< @internal TR resistance calculation coef 0, std bom */
#define MLT_CONST_REQ_LONG_SV_0			321983L        /*!< @internal TG/RG resistance calculation coef 0, std bom */
#define MLT_CONST_REQ_LONG_SV_1			482975L        /*!< @internal TG/RG resistance calculation coef 1, std bom */
#define MLT_CONST_REQ_AUTO_LONG_SV		207L           /*!< @internal TG/RG resistance simplified calculation coef 0, std bom */
#define MLT_CONST_PM_REQ_DIFF_SV_0		218L           /*!< @internal TR resistance calculation coef 0, PM bom */
#define MLT_CONST_PM_REQ_LONG_SV_0		296283L        /*!< @internal TG/RG resistance calculation coef 0, PM bom */   
#define MLT_CONST_PM_REQ_LONG_SV_1		374573L        /*!< @internal TG/RG resistance calculation coef 1, PM bom */
#define MLT_CONST_MADC_VCOARSE_SV       1074           /*!< @internal MADC coarse voltage scaling */
#define MLT_CONST_MADC_HIRES_V_SV       246            /*!< @internal MADC hires voltage scaling (1000*406)/100 */



#define MLT_CONST_BAL_RATIO             125L           /*!< @internal RG to TG fault balance ratio */
#define MLT_CONST_PM_BAL_RATIO          135L           /*!< @internal RG to TG fault balance ratio, PM bom */

/* BOM Independent */
#define MLT_CONST_OPEN_RES				1000000000L    /*!< @internal Open circuit impedance constant */
#define MLT_CONST_MAX_RES				20000000L      /*!< @internal Max 2Mohm resistance measured */
#define MLT_CONST_LKG_DAC_VLIM			48500          /*!< @internal Minimum Vt/Vr supporting lkgdac method */
#define MLT_CONST_AUTOV_REQ_OPEN  		425000L		   /*!< @internal open lead impedance in tip/ring open mode */
/*
** Measurement Scaling (includes redundent defines from std API)
*/
#define MLT_CONST_MADC_I                 597       /*!< @internal MADC current scale factor (1e6/1.676e-9 A/lsb) */
#define MLT_CONST_MADC_V                 1074      /*!< @internal MADC voltage scale factor (1e3/931.323e-9 V/lsb) */
#define MLT_CONST_COARSE_SENSE_VOFFS     1500      /*!< @internal coarse sensor 1.5v intrinsic offset */
#define MLT_CONST_AUTO_V_VTAR            1073741L  /*!< @internal Auto-V vtar voltage scale (1/931.323e-9) */
#define MLT_CONST_AUTO_V_RINGOF          4941L     /*!< @internal Ringer DC useed as diag Isrc (Vdc=Isrc(uA)/319.57uA/V)*/
#define MLT_CONST_AUTO_V_VFEED           1969L     /*!< @internal V_FEED (internal source)scale in mV/10 */
#define MLT_CONST_AUTO_V_STEP_SCALE      134272L   /*!< @internal AutoV step scale (tbd)*/
#define MLT_CONST_P_TH_HVIC              0x6468ADL /*!< @internal threshold programmed during MLT test execution */
#define MLT_CONST_DSP_TIMER_SCALE        65536L    /*!< @internal dsp timer msec lsb (2^16) */  
#define MLT_CONST_DSP_TIMER_V_SCALE      1974L     /*!< @internal timed, auto captured v (1/5.0663948e-7v) */

/**@}*/

/**
 *  @internal @defgroup PROSLIC_MLT_DIAG_MASK ProSLIC MLT Diag Bitmasks
 *  Contains bitmask macros for the DIAG registers
 *
 * @{
 */
#define DIAG_AUTO_V         0x40
#define DIAG_HPF_EN         0x20
#define DIAG_TX_FILT_EN     0x10
#define DIAG_ENABLE         0x08
#define DIAG_KDC_OFF        0x04
#define DIAG_DCLPF_44K      0x02
#define DIAG_FB_OFF         0x01
#define DIAG_CLR_DIAG1      0x00
/*
** DIAG2 BITMASK
*/
#define DIAG_SEL_HIRES_VRINGC 0x12
#define DIAG_SEL_HIRES_VTIPC  0x11
#define DIAG_HIRES_EN         0x10
#define DIAG_SEL_IRING        0x0A
#define DIAG_SEL_ITIP         0x08
#define DIAG_SEL_VLONG        0x07
#define DIAG_SEL_VBAT         0x03
#define DIAG_SEL_VRINGC       0x02
#define DIAG_SEL_VTIPC        0x01
#define DIAG_CLR_DIAG1        0x00

#define MADC_NORMAL 0
#define MADC_HIRES 1

/**@}*/

/* 
** Function Definitions 
*/

/*******************************************************************************************/
/**
 *
 * Function: 	getRAMScale
 *
 * @brief       This routine returns the voltage or current scale factor for the passed RAM address
 * 
 * @param[in]  addr - RAM address
 * @param[in]  ext_v_opt - extedned voltage range (1 = enabled)
 *
 * @retval  int32 - scale factor
 *
 * @remark Scale factors presume desired uA or mV
 *
 */
int32 getRAMScale(uInt16 addr, uInt8 ext_v_opt);


/*******************************************************************************************/
/**
 *
 * Function: 	ReadMADCScaled
 *
 * @brief       This routine returns the scaled voltage or current represented by the passed RAM location
 * 
 * @param[in]  proslicChanType_ptr pProslic - pointer to ProSLIC channel structure
 * @param[in]  uInt16 addr - RAM address to read
 * @param[in]  uInt32 scale - supply scale factor or 0 to scale automatically using getRamScale()
 *
 * @retval  int32 - scaled voltage or current reading
 *
 * \b Example \b Usage:
 * @code
 * int32 iloopMeas;
 *
 *     iloopMeas = ReadMADCScaled(pProslic, MADC_ILOOP,0);  
 * @endcode
 * @remark Scale factors presume desired uA or mV
 *
 */
int32 ReadMADCScaled(ProSLICMLTType *pProSLICMLT, uInt16 addr, int32 scale);



/*******************************************************************************************/
/**
 *
 * Function: 	setupCoarseSensors
 *
 * @brief       This routine preparsed VTIPC and VRINGC for use
 * 
 * @param[in]  pProslic - ptr to MLT type
 * @param[in]  int32 *tipOffs - ptr to value stored in MADC_VTIPC_DIAG_OS
 * @param[in]  int32 *ringOffs - ptr to value stored in MADC_VRINGC_DIAG_OS
 *
 * @retval  0
 *
 * @remark
 *          This routine does the following
 *          - Powers up MADC
 *          - Powers up VTIPC and VRINGC course sensors
 *          - Sets GPIOs for analog inputs
 *          - Reads coarse sensor offsets from RAM
 *
 */
int  setupCoarseSensors(ProSLICMLTType *pProslic, int32 *tipOffs, int32 *ringOffs);


/*******************************************************************************************/
/**
 *
 * Function: 	readCoarseSensor
 *
 * @brief       This routine reads MADC_DIAG_RAW and returns
 * 
 * @param[in]  ProSLICMLTType pProSLICMLT - pointer to ProSLIC MLT structure
 * @param[in]  int32 offset - offset value
 *
 * @retval  int32 - value of MADC_DIAG_RAW with passed offset applied
 *
 * \b Example \b Usage:
 * @code
 * int32 senseValue;
 * int32 tipOffs;
 * int32 ringOffs;
 *
 *     setupCoarseSensors(pProSLICMLT->pProslic,&tipOffs);
 *
 *     senseValue = readCoarseSensor(pProSLICMLT,tipOffs);
 *
 * @endcode
 * @remark
 *          This routine does the following
 *          - Reads MADC_DIAG_RAW
 *          - Applies default sensor offset, MLT_CONST_COARSE_SENSE_VOFFS
 *          - Applies user supplied offset
 *          - Returns adjusted value
 *
 */
int32 readCoarseSensor(ProSLICMLTType *pProSLICMLT,int32 offset);

/*******************************************************************************************/
/**
 *
 * Function: 	diagSet
 *
 * @brief       This routine programs DIAG1 register
 * 
 * @param[in]  proslicChanType_ptr pProslic - pointer to ProSLIC channel structure
 * @param[in]  uInt8 setting - OR'd bitmask of DIAG1 bits
 *
 * @retval void - none
 *
 * \b Example \b Usage:
 * @code
 * 
 *     diagSet(pProSLICMLT->pProslic, DIAG_AUTO_V | DIAG_ENABLE | DIAG_KDC_OFF);
 *
 * @endcode
 *
 * @remark 
 */
void diagSet(proslicChanType_ptr pProslic, uInt8 setting);


/*******************************************************************************************/
/**
 *
 * Function: 	diagSelect
 *
 * @brief       This routine selects the signal to be routed to the DIAG block
 * 
 * @param[in]  proslicChanType_ptr pProslic - pointer to ProSLIC channel structure
 * @param[in]  uInt8 select - signal to be routed to DIAG block (see diag bitmasks)
 * @param[in]  int hiresFlag - indicates if HIRES mode is enabled
 *
 * @retval void - none
 *
 * \b Example \b Usage:
 * @code
 * 
 *     diagSelect(pProSLICMLT->pProslic, DIAG_SEL_VTIPC, MADC_HIRES);
 *
 * @endcode
 *
 * @remark 
 */
void diagSelect(proslicChanType_ptr pProslic, uInt8 select, int hiresFlag);


/*******************************************************************************************/
/**
 *
 * Function: 	setupDiagCurrentSource
 *
 * @brief       This routine sets the diag current source using the ringer dc offset
 * 
 * @param[in]  proslicChanType_ptr pProslic - pointer to ProSLIC channel structure
 * @param[in]  int32 iForce - dc current in uA
 *
 * @retval void - none
 *
 * \b Example \b Usage:
 * @code
 * int32 iforce1 = 5000;
 * 
 *     setupDiagCurrentSource(pProSLICMLT->pProslic, iforce1);
 *
 * @endcode
 *
 * @remark  This routine does not enable diag
 */
void setupDiagCurrentSource(proslicChanType_ptr pProslic, int32 iForce);


/*******************************************************************************************/
/**
 *
 * Function:    readTerminalValues
 *
 * @brief       Reads sample of all commonly used terminals
 * 
 * @param[in]  ProSLICMLTType pProSLICMLT - pointer to ProSLIC MLT structure
 * @param[in]  ProSLIC_term_meas_t term - pointer to ProSLIC terminal measurement structure
 * @param[in]  int index - sample index
 *
 * @retval void - none
 *
 *
 * @remark  
 */
void readTerminalValues(ProSLICMLTType *pProSLICMLT,ProSLIC_term_meas_t *term, int index);

/*******************************************************************************************/
/**
 *
 * Function:    printTerminalValues
 *
 * @brief       Displays sample of all commonly used terminals
 * 
 * @param[in]  ProSLIC_iv_t term - pointer to ProSLIC terminal measurement structure
 * @param[in]  int index - sample index
 *
 * @retval void - none
 *
 *
 * @remark  
 */
void printTerminalValues(ProSLIC_iv_t *term, int index);

/*******************************************************************************************/
/**
 *
 * Function:    setup300HzBandbass
 *
 * @brief       Setup audio diag filters for 300Hz bandpass
 * 
 * @param[in]  ProSLICMLTType - pointer to ProSLIC MLT structure
 *
 * @retval void - none
 *
 *
 * @remark  
 */
void setup300HzBandpass(ProSLICMLTType * pProSLICMLT);

/*******************************************************************************************/
/**
 *
 * Function:    setup3014HzBandbass
 *
 * @brief       Setup audio diag filters for 3014Hz bandpass
 * 
 * @param[in]  ProSLICMLTType - pointer to ProSLIC MLT structure
 *
 * @retval void - none
 *
 *
 * @remark  
 */
void setup3014HzBandpass(ProSLICMLTType * pProSLICMLT);

/*******************************************************************************************/
/**
 *
 * Function:    setup300HzTestTone
 *
 * @brief       Setup diag tone gen for 300Hz current source
 * 
 * @param[in]  ProSLICMLTType - pointer to ProSLIC MLT structure
 *
 * @retval void - none
 *
 *
 * @remark  
 */
void setup300HzTestTone(ProSLICMLTType * pProSLICMLT);

/*******************************************************************************************/
/**
 *
 * Function:    setup3014HzTestTone
 *
 * @brief       Setup diag tone gen for 3014Hz current source
 * 
 * @param[in]  ProSLICMLTType - pointer to ProSLIC MLT structure
 *
 * @retval void - none
 *
 *
 * @remark  
 */
void setup3014HzTestTone(ProSLICMLTType * pProSLICMLT);

/*@}*/
#endif

