/*
** Copyright (c) 2010-2014 by Silicon Laboratories
**
** $Id: proslic_mlt_api_config.h,v 1.4 2008/07/11 21:50:53 cdp Exp $
**
*/
/*! \file proslic_mlt_api_config.h
**  \brief ProSLIC MLT configuration file
**  
**	This is the header file customers should use to configure the
**  ProSLIC MLT API
**
**  \author Silicon Laboratories, Inc (cdp)
**
**  \attention 
**	This file contains proprietary information.	 
**	No dissemination allowed without prior written permission from
**	Silicon Laboratories, Inc. 
*/
#ifndef MLT_CONFIG_H
#define MLT_CONFIG_H

/**
 * @defgroup MLT_CONFIG ProSLIC MLT API Configuration Parameters
 * Contains MLT parameters that may be modified by the end user.  Please consult Silabs for assistance in making changes.
 *
 *@{
 */
/*************************************************************************************************************/
/**
 * @defgroup MLT_CONFIG_GENERAL General MLT Configuration Parameters
 *
 *@{
 */
#define MLT_POLL_RATE                 10	  /*!< Software poll rate (ms) - MUST be between 2-10 mSec in order for the specified accuracy levels published. */
/*@}*/

/*************************************************************************************************************/
/**
 * @defgroup MLT_CONFIG_BOM BOM Dependent MLT Configuration Parameters
 *
 *@{
 */
/*#define  PM_BOM       */                       /* Define PM_BOM if hardware BOM supports pulse metering */
#define MLT_PROTECTION_RESISTANCE     210	     /*!< Value of series resistance(per leg) on TIP and RING (ohms*10)  */    
#define MLT_LINE_CAPACITANCE          100	     /*!< Value of capacitors on tip and ring (nF*10) */
#define MLT_EMI_CAPACITANCE           100        /*!< Value of EMI capacitor from tip to ring (nF*10) */
/*@}*/


/*************************************************************************************************************/
/**
 * @defgroup MLT_CONFIG_FOREIGN_VOLTAGE Foreign Voltage MLT Configuration Parameters
 *
 *@{
 */
#define MLT_USE_EXTENDED_V_RANGE                 /*!< Define to extend voltage range to 250vrms (requires BOM change) */
#undef  MLT_USE_EXTENDED_V_RANGE

#define MLT_TS_HAZV_LINE_DISCHARGE    50         /*!< Time under 0v V_VLIM prior to making FEMF measurement */
#define MLT_MAX_FEMF_SAMPLES          256        /*!< Max number of samples allowed in FEMF test */
#define MLT_FEMF_SAMPLES              30         /*!< Number of voltage samples to capture */
#define MLT_HAZV_VTR_ENABLED                     /*!< Include VTR AC voltage measurement in hazardous voltage test.
                                                 **   This adds ~350ms of additional test time and is not a measurement
                                                 **   that is required by all MLT regulatory specifications     */
/*@}*/


/*************************************************************************************************************/
/**
 * @defgroup MLT_CONFIG_RESFAULT Resistive Faults MLT Configuration Parameters
 *
 *@{
 */
#define MLT_FAST_RESFAULTS                0     /*!< Obsolete - settle times are now adaptive */
#define MLT_RES_AUTOV_SAMPLES             20    /*!< Number of voltage samples averaged when making resistance measurements
                                                     in the 3k to 1M range.  Note - no protection against overflow has
                                                     been included - maximum is 128 */
#define MLT_RES_AUTOV_STEP				  1		/*!< Obsolete - step size is now adaptive */
#define MLT_RES_AUTOV_DIFF_V1			  10	/*!< Resistive Fault Measurement - Diff V1  */
#define MLT_RES_AUTOV_DIFF_V2			  40	/*!< Resistive Fault Measurement - Diff V2  */
#define MLT_RES_AUTOV_LONG_V1			  28	/*!< Resistive Fault Measurement - Long V1  */
#define MLT_RES_AUTOV_LONG_V2			  50	/*!< Resistive Fault Measurement - Long V2  */

#define MLT_RES_AUTOV_AHS_DET_EN	      1		/*!< AHS Detection Enabled */
#define MLT_RES_AUTOV_DIFF_AHS_V1		  10	/*!< Resistive Fault Measurement - Diff V1 if AHS detected  */
#define MLT_RES_AUTOV_DIFF_AHS_V2		  22	/*!< Resistive Fault Measurement - Diff V2 if AHS detected  */
#define MLT_RES_AUTOV_LONG_AHS_V1         12    /*!< Resistive Fault Measurement - Long V1 if AHS detected */
#define MLT_RES_AUTOV_LONG_AHS_V2         24    /*!< Resistive Fault Measurement - Long V2 if AHS detected */
#define MLT_RES_AUTOV_AHS_MAX_RETEST	  2     /*!< Maximum number of measure repeats if AHS detected */
#define MLT_RES_AUTOV_DIFF_AHS_SETTLE     4300  /*!< Measurement settle - Diff measurement if AHS detected */
#define MLT_RES_AUTOV_LONG_AHS_SETTLE     1000  /*!< Measurement settle - Long measurement if AHS detected */

#define MLT_RES_SM_R_MIN_I_FWD			  8000  /*!< Min FWD ILOOP Small R Test in uA */
#define MLT_RES_SM_R_MIN_I_REV            7000  /*!< Min REV ILOOP Small R Test in uA */

/*@}*/




                                            
/*************************************************************************************************************/
/**
 * @defgroup MLT_CONFIG_REN REN MLT Configuration Parameters
 *
 *@{
 */
#define MLT_REN_TEST_FREQ               20           /*!< Frequency at which REN is measured (16 or 20) */
#define MLT_REN_SAMPLE_TIME             300          /*!< Period in which current is averaged in REN test */

/*
 * REN Calibration Constants
 *
 * Values based on typical REN calibration results using Silabs EVBs
 * User should replace these with results of running a REN calibration
 * using their own hardware
 */
#ifdef SI3217X							
#define MLT_RENCAL_HI_REN_SLOPE         87657L		 /*!< Example calibration constant from Si32178FB-EVB 2.1 */
#define MLT_RENCAL_LO_REN_SLOPE         110457L
#define MLT_RENCAL_HI_REN_OFFS          86616L
#define MLT_RENCAL_LO_REN_OFFS          18215L
#define MLT_RENCAL_REN_TRANS            384544L
#define MLT_RENCAL_ELO_REN_SLOPE        117721L
#define MLT_RENCAL_ELO_REN_OFFS         10951L
#else
#define MLT_RENCAL_HI_REN_SLOPE         85195L
#define MLT_RENCAL_LO_REN_SLOPE         100696L
#define MLT_RENCAL_HI_REN_OFFS          64184L
#define MLT_RENCAL_LO_REN_OFFS          17681L
#define MLT_RENCAL_REN_TRANS            351745L
#define MLT_RENCAL_ELO_REN_SLOPE        113467L
#define MLT_RENCAL_ELO_REN_OFFS         4910L
#endif
/*@}*/


/*************************************************************************************************************/
/**
 * @defgroup MLT_CONFIG_ROH Receiver offhook MLT Configuration Parameters
 *
 *@{
 */
#define MLT_TS_ROH_MEAS1              250 
#define MLT_TS_ROH_MEAS2              100   
#define MLT_ROH_MAX_ZDIFF             13        /*!< Maximum % difference in TR impedance measured at two current levels
                                                **   during ROH test */
#define MLT_ROH_MIN_ZDIFF_REN         100       /*!< Min % difference in TR impedance measured at two current levels during
                                                     ROH test in which one of the measurements looks like a resistive fault, but
                                                     because of the large % difference in measurement, it must be measurement
                                                     error due to an excessive REN load.  */
#define MLT_ROH_MAX_ZTR               2500      /*!< Maximum T-R impedance that could be considered a valid cpe device
                                                **   plus loop resistance */
#define MLT_ROH_MIN_ILOOP             300       /*!< If loop current is below this during ROH test, no offhook phone or fault
                                                     is presumed (uA) */
#define MLT_ROH_MIN_VLOOP             1000      /*!< If loop voltage is below this during ROH test, short circuit fault
                                                     is presumed (mv) */
/*@}*/


/*************************************************************************************************************/
/**
 * @defgroup MLT_CONFIG_CAP Capacitance MLT Configuration Parameters
 *
 *@{
 */
#define MLT_CTR_EST_T1                4
#define MLT_CTR_EST_T2                10
#define MLT_CTR_MAX_VTR               42000     /*!< Max T-R voltage (in mv) during time constant measurement */
#define MLT_CTR_MIN_VTR               3000      /*!< Max T-R voltage (in mv) during time constant measurement */
#define MLT_DISABLE_3TERM_CAP_COMPENSATION 0    /*!< Used to disable capacitance 3 terminal equations. */
/*@}*/

/*@}*/
#endif
