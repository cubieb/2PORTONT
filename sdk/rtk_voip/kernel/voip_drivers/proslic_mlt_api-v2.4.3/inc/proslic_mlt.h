/*
** Copyright (c) 2009-2015 by Silicon Laboratories
**
** $Id: proslic_mlt.h 4714 2015-02-05 18:16:07Z nizajerk $
**
*/
/*! \file proslic_mlt.h
**  \brief ProSLIC MLT interface header file
**
**  This is the header file for the ProSLIC MLT top level APIs
**
**  \author Silicon Laboratories, Inc (cdp)
**
**  \attention
**  This file contains proprietary information.	 
**  No dissemination allowed without prior written permission from
**  Silicon Laboratories, Inc.
**
*/

#ifndef PROSLIC_MLT_H
#define PROSLIC_MLT_H
/** @mainpage ProSLIC MLT API Documentation
 *
 * This documentation describes the ProSLIC MLT API's
 *
 */
/** @defgroup PROSLIC_API_TOPLEVEL  Top Level ProSLIC MLT APIs
 *  This group contains the top level ProSLIC MLT APIs to be called by user's application
 *
 *@{
 */
#include "proslic_mlt_api_config.h"
#include "proslic_mlt_macro.h"


/*
**
** Enums
**
*/

/** @defgroup MLT_RETURN_CODES MLT return codes
**  Most of the top level functions will return one of the following types:
** @{
*/
/** General return codes */
typedef enum {
    RC_MLT_TEST_RUNNING,     /**< Test needs to be periodicaly polled */
    RC_MLT_TEST_COMPLETE,    /**< Test completed and has no execution errors */
    RC_MLT_NO_ERROR,         /**< Function succeded */
    RC_MLT_UNSUPPORTED_TEST, /**< Test is unsuported by the given chipset */
    RC_MLT_ABORT,            /**< Test was aborted */
    RC_MLT_NOCONVERGE,       /**< Test failed to converge on a specific value */
    RC_MLT_TIMEOUT,          /**< Test failed to complete */
    RC_MLT_ERROR,            /**< Generic error message */
    RC_MLT_BAD_PARAM,        /**< Bad parameter */
	RC_MLT_FALSE_RINGTRIP,   /**< Unexpected ringtrip */
	RC_MLT_ALARM_ON_RING,    /**< Power alarm on ring start */
	RC_MLT_RINGSTART_ERR     /**< Ringing failed to start */
} RC_MLT_RC_T;

/* ROH */
enum {
    RC_MLT_ROH_NOFAULT,
    RC_MLT_ROH_FAIL_RESFAULT,
    RC_MLT_ROH_FAIL_ROH
}; 
/** @} MLT_RETURN_CODES*/ 


/*
** Resistive Faults Test method 
*/
typedef enum {
    RESFAULT_METHOD_UNDEFINED,
    AUTOV,
    LKGDAC,
    LOWR,
    RCTC
} resFaultMethod; 

/* 
** Terminal Tag
*/
enum {
    MLT_TERM_TR,
    MLT_TERM_TG,
    MLT_TERM_RG
};
	

/**
 @brief  Structure for REN test calibration coefficients to be stored
*/
typedef struct {
	int32 highRenSlope; 
	int32 lowRenSlope ; 
	int32 highRenOffs ; 
	int32 lowRenOffs  ;
	int32 renTrans    ; 
	int32 extraLowRenSlope;
	int32 extraLowRenOffset;
} ProSLIC_rencal_t;

#ifdef MLT_RUNTIME_CONFIG
/**
 @brief  Structure for optional runtime/per channel MLT configuration
*/
typedef struct {
    uInt16 mlt_poll_rate_offs;
    uInt16 mlt_poll_rate;
    int32  mlt_prot_res;
    int32  mlt_line_cap;
    int32  mlt_emi_cap;
    int32  mlt_femf_samples;
    uInt8  mlt_ext_v_flag;
}ProSLIC_mlt_config_t;
#endif

/**
@internal
@brief    Structure to store I/V measurements
*/
typedef struct {
    int32 vtip[MLT_MAX_IV_SAMPLES];
    int32 vring[MLT_MAX_IV_SAMPLES];
    int32 itip[MLT_MAX_IV_SAMPLES];
    int32 iring[MLT_MAX_IV_SAMPLES];
    int32 vloop[MLT_MAX_IV_SAMPLES];
    int32 iloop[MLT_MAX_IV_SAMPLES];
    int32 vlong[MLT_MAX_IV_SAMPLES];
    int32 ilong[MLT_MAX_IV_SAMPLES];
} ProSLIC_iv_t;

/**
@brief    General terminal measurement structure
*/
typedef struct {
    int32 measTR;
    int32 measTG;
    int32 measRG;
    int32 measAUX;
    int32 measTR2;
    int32 measTG2;
    int32 measRG2;
    int   trFlag;
    int   tgFlag;
    int   rgFlag;
    int   auxFlag;
    uInt16   time1;
    uInt16   time2;
    uInt16   time3;
    uInt8 resultsValid;
	uInt8 ahsFlag;
    ProSLIC_iv_t term;
    uInt8  flags[16];
    int32  slopeErr;
} ProSLIC_term_meas_t;

/**
@brief    Structure to store REN measurement data
*/
typedef struct {
    ProSLIC_rencal_t calData;
    int32 renValue;
    int   renFlag;
    int   renCalFlag;
    int   resultsValid;
} ProSLIC_ren_meas_t;

/**
 @brief   Structure to store ROH measurement data
*/
typedef struct {
    int rohTrue;
    int resultsValid;
} ProSLIC_roh_meas_t;


#ifdef WIN32
typedef struct {
    _int64 time;
}timeProfile;
#endif

/**
 @brief   Main MLT channel structure
*/
typedef struct{
	proslicChanType_ptr pProslic;                      /**< Linkage to ProSLIC API channel structure */
    ProSLIC_term_meas_t resFaults;                     /**< Stores results of resistive faults test */
    ProSLIC_term_meas_t resFaultsRaw;
    ProSLIC_term_meas_t hazVAC;                        /**< Stores AC results of foreign/hazardous voltage test */
    ProSLIC_term_meas_t hazVDC;                        /**< Stores DC results of foreign/hazardous voltage test */
    ProSLIC_term_meas_t capFaults;                     /**< Stores results of capacitance test */
    ProSLIC_term_meas_t capFaultsRaw;
    ProSLIC_term_meas_t hazIDC;
    ProSLIC_term_meas_t hazIAC;
    ProSLIC_roh_meas_t roh;                               /**< Stores results of receiver offhook test */
    ProSLIC_ren_meas_t ren;                               /**< Stores results of REN test */
    ProslicRAMInit preserveRAM[MLT_MAX_PRESERVE_RAM];     /**<  Storage for resotoring entry conditions */
    ProslicRegInit preserveReg[MLT_MAX_PRESERVE_REG];     /**<  Storage for resotoring entry conditions */
    ProslicRAMInit preserveAuxRAM[MLT_MAX_PRESERVE_RAM];  /**< Auxillary storage for revision specific locations */
    ProslicRegInit preserveAuxReg[MLT_MAX_PRESERVE_REG];  /**< Auxillary storage for revision specific locations */
#ifdef WIN32
    timeProfile startTime;
    timeProfile timeDelay;
    int exec_time;
#endif
#ifdef MLT_DLL
    timeProfile runTime;
#endif
#ifdef MLT_RUNTIME_CONFIG
    ProSLIC_mlt_config_t    mlt_config;
#endif
    int      pm_bom_flag;
}ProSLICMLTType;


/**
 @brief   MLT test state structure
*/
typedef struct {
    uInt16 stage;                   /**< Stage counter for controlling reentry */
    uInt16 sampleIterations;        /**< Sample counter for AC or averaged measurements */
    uInt16 waitIterations;          /**< Wait counter for controlling delays */
    uInt16 settleTime;     
	uInt16 numSamples;
} ProSLIC_mlt_test_state;

/**
@brief    Structure for storing foreign voltages test state information
*/
typedef struct {
    ProSLIC_mlt_test_state State;
    int32 vt[MLT_MAX_FEMF_SAMPLES];	/* All measurement values are in terms of mV */
    int32 vr[MLT_MAX_FEMF_SAMPLES];
    int32 vtr[MLT_MAX_FEMF_SAMPLES];
    int32 tipOffs;
    int32 ringOffs;
    int32 samples;
    uInt8 regTemp;
    int32 discharge;
} ProSLIC_mlt_foreign_voltages_state;

/**
@brief    Structure for storing AHS detection test information
*/
typedef struct {
	uInt8  enable;
	int32  detected;
	int32  ratio;
	int32  irms;
	int32  count;
}ProSLIC_ahs_det_t;

/**
@brief    Structure for storing resistive faults test state information
*/
typedef struct {
	ProSLIC_mlt_test_state setupState;
	ProSLIC_mlt_test_state TRState;
	ProSLIC_mlt_test_state RGState;
	ProSLIC_mlt_test_state TGState;
	ProSLIC_mlt_test_state autoVState;
	int32 v1;
	int32 v2;
	int32 i1;
	int32 i2;
	int32 step;
	uInt16 interval;
	uInt8 smallRFlag;
    int32 tipOffs;
    int32 ringOffs;
    int32  iring1;
    int32  iring2;
    int32  itip1;
    int32  itip2;
    int32  vtip1;
    int32  vtip2;
    int32  vring1;
    int32  vring2;
    int32  vtar[2];
    int32  vtmp;
	int32  dvdt_tr;
	ProSLIC_ahs_det_t  ahs_det;
	int32  vmeas;
	int32  imeas;
	int32  v_offset;
	int32  i_offset;
	int32  i_array[MLT_MAX_AUTOV_SAMPLES];
	uInt32 hvic_state_save;
	uInt8  fault_term;
	uInt8  active_term;
    /* track measurement method used */
    resFaultMethod rtrMethod;
    resFaultMethod rtgMethod;
    resFaultMethod rrgMethod;
} ProSLIC_mlt_rmeas_state;

/**
@brief    Structure for storing receiver offhook test state information
*/
typedef struct {
	ProSLIC_mlt_test_state State;
	int32 Rv1;
	int32 Rv2;
} ProSLIC_mlt_roh_state;

/**
@brief   Structure for storing REN test state information 
*/
typedef struct {
	ProSLIC_mlt_test_state State;
	int32 max;
	ProSLIC_rencal_t *renCal;
	uInt32 counter_vtr_val; 
	uInt32 vcm_ring;
	uInt32 vring;
#ifdef MLT_RUNTIME_CONFIG
	int32 v[256];
#else
	int32 v[MLT_REN_SAMPLE_TIME/MLT_POLL_RATE];
#endif
} ProSLIC_mlt_ren_state;


/**
@brief    Structure for storing capacitance test state information
*/
typedef struct {
    ProSLIC_mlt_test_state State;
    ProSLIC_mlt_test_state MeasState;
    ProSLIC_mlt_test_state TimeConstState;
    uInt32 testavo;
    uInt32 freq;
    uInt32 ram1447;
} ProSLIC_mlt_capacitance_state;
    
/**
@brief    Structure for storing REN capacitance estimation test state information
*/
#define ProSLIC_mlt_ren_cap_state ProSLIC_mlt_capacitance_state


/* 
** Function Definitions 
*/


/*******************************************************************************************/
/** @defgroup MLT_VOLTAGES MLT Hazardous/Foreign Voltage Test
 * This group of functions are used to initialize and execute hazardous or
 * foreign voltages test.
 * @{
 */
/*******************************************************************************************/
/**
 * Function: 	ProSLIC_mlt_foreign_voltages
 *
 * @brief       This routine should be called to measure AC and DC voltages on TIP/RING
 * 
 * @param[in,out]  *pProSLICMLT - pointer to MLT channel structure
 * @param[in]  *pState - state variable to track reentry
 *
 * @retval typical values: @ref RC_MLT_TEST_RUNNING or @ref RC_MLT_TEST_COMPLETE are normal values returned. @sa MLT_RETURN_CODES for other possible values.
 *
 * Upon success, the function returns back in pProSLICMLT, the following data:
 *
 * - pProSLICMLT->hazVAC.measTG - Tip to GND AC voltage (Vrms^2 / 100)
 * - pProSLICMLT->hazVAC.measRG - Ring to GND AC voltage (Vrms^2 / 100)
 * - pProSLICMLT->hazVAC.measTR - Tip to Ring AC voltage (Vrms^2 / 100)
 * - pProSLICMLT->hazVDC.measTG - Tip to GND DC voltage (mV)
 * - pProSLICMLT->hazVDC.measRG - Ring to GND DC voltage (mV)
 * - pProSLICMLT->hazVDC.measTR - Tip to Ring DC voltage (mV)
 *
 * @remark   
 *              - This routine should be called on a per channel basis
 *              - This function is reentrant and must be called at periodic intervals defined by MLT_POLL_RATE
 *              - ProSLIC_mlt_init_foreign_voltages() must be called prior to calling this routine to 
 *                initialize test state variables
 * 
 * @sa ProSLIC_mlt_init_foreign_voltages, MLT_POLL_RATE
 *
 */
int ProSLIC_mlt_foreign_voltages(ProSLICMLTType *pProSLICMLT, ProSLIC_mlt_foreign_voltages_state *pState);


/*******************************************************************************************/
/**
 * Function: 	ProSLIC_mlt_hazard_voltages
 *
 * @brief       This routine should be called to measure AC and DC voltages on TIP/RING
 * 
 * @param[in,out]  *pProSLICMLT - pointer to MLT channel structure
 * @param[in]  *pState - state variable to track reentry
 *
 * @retval typical values: @ref RC_MLT_TEST_RUNNING or @ref RC_MLT_TEST_COMPLETE are normal values returned. @sa MLT_RETURN_CODES for other possible values.
 *
 * Upon success, the function returns back in pProSLICMLT, the following data:
 * - pProSLICMLT->hazVAC.measTG - Tip to GND AC voltage (Vrms^2 / 100)
 * - pProSLICMLT->hazVAC.measRG - Ring to GND AC voltage (Vrms^2 / 100)
 * - pProSLICMLT->hazVAC.measTR - Tip to Ring AC voltage (Vrms^2 / 100)
 * - pProSLICMLT->hazVDC.measTG - Tip to GND DC voltage (mV)
 * - pProSLICMLT->hazVDC.measRG - Ring to GND DC voltage (mV)
 * - pProSLICMLT->hazVDC.measTR - Tip to Ring DC voltage (mV)
 *
 * @remark   
 *              - This routine should be called on a per channel basis
 *              - This function is reentrant and must be called at periodic intervals defined by MLT_POLL_RATE
 *              - ProSLIC_mlt_init_foreign_voltages() must be called prior to calling this routine to 
 *                initialize test state variables
 * 
 * @sa ProSLIC_mlt_init_foreign_voltages(), MLT_POLL_RATE
 *
 */
int ProSLIC_mlt_hazard_voltages(ProSLICMLTType *pProSLICMLT, ProSLIC_mlt_foreign_voltages_state *pState);

/*******************************************************************************************/
/**
 * Function: 	ProSLIC_mlt_init_foreign_voltages
 *
 * @brief       The routine should be called prior to ProSLIC_mlt_foreign_voltages to initialize state variables
 * 
 * @param[in,out]  *pState - state variable to track reentry
 * @param[in]  samples - number of voltage samples taken
 *
 * @retval  typical return value:  @ref RC_MLT_NO_ERROR @sa MLT_RETURN_CODES for other possible values.
 *  
 * @remark   
 *              - This routine should be called on a per channel basis
 *              - The number of voltage samples taken is presently overwritten by the
 *                device driver to provide the most optimum results           
 * 
 * @sa ProSLIC_mlt_foreign_voltages()
 *
 */
int ProSLIC_mlt_init_foreign_voltages(ProSLIC_mlt_foreign_voltages_state *pState, int samples);
/** @}*/


/*******************************************************************************************/
/** @defgroup MLT_RESFAULTS MLT Resistive Faults Test
 * This group of functions are used to initialize and execute the resistive faults test.
 * @{
 */
/*******************************************************************************************/
/**
 * Function: 	ProSLIC_mlt_resistive_faults
 *
 * @brief       This routine should be called to measure resistance from TIP-RING, TIP-GND, and RING-GND
 * 
 * @param[in,out]  *pProSLICMLT - pointer to MLT channel structure
 * @param[in]  *pState - state variable to track reentry
 *
 * @retval typical values: @ref RC_MLT_TEST_RUNNING or @ref RC_MLT_TEST_COMPLETE are normal values returned. @sa MLT_RETURN_CODES for other possible values.
 *
 * Upon success, the function returns back in pProSLICMLT, the following data:
 *
 * - pProSLICMLT->resFaults.measTG - Tip to GND Resistance (ohms*10)
 * - pProSLICMLT->resFaults.measRG - Ring to GND Resistance (ohms*10)
 * - pProSLICMLT->resFaults.measTR - Tip to Ring Resistance (ohms*10)
 *
 * @remark   
 *              - This routine should be called on a per channel basis
 *              - This function is reentrant and must be called at periodic intervals defined by MLT_POLL_RATE
 *              - ProSLIC_mlt_init_resistive_faults() must be called prior to calling this routine to 
 *                initialize test state variables
 * 
 * @sa ProSLIC_mlt_init_resistive_faults(), MLT_POLL_RATE
 *
 */
int ProSLIC_mlt_resistive_faults(ProSLICMLTType *pProSLICMLT, ProSLIC_mlt_rmeas_state *pState);

/*******************************************************************************************/
/**
 * Function: 	ProSLIC_mlt_init_resistive_faults
 *
 * @brief       The routine should be called prior to ProSLIC_mlt_receiver_offhook to initialize state variables
 * 
 * @param[in]  *pState - state variable to track reentry
 *
 * @retval  typical return value:  @ref RC_MLT_NO_ERROR @sa MLT_RETURN_CODES for other possible values.
 *  
 * @remark   
 *              - This routine should be called on a per channel basis          
 * 
 * @sa @ref ProSLIC_mlt_receiver_offhook()
 *
 */
int ProSLIC_mlt_init_resistive_faults(ProSLIC_mlt_rmeas_state *pState);
/** @}*/


/*******************************************************************************************/
/** @defgroup MLT_ROH MLT Receiver Offhook Test
 * This group of functions are used to initialize and execute the receiver offhook test.
 * @{
 */
/*******************************************************************************************/
/**
 * Function: 	ProSLIC_mlt_receiver_offhook
 *
 * @brief       This routine should be called to detect the presence of an offhook receiver (FXO)
 * 
 * @param[in,out]  *pProSLICMLT - pointer to MLT channel structure
 * @param[in]  *pState - state variable to track reentry
 *
 * @retval typical values: @ref RC_MLT_TEST_RUNNING or @ref RC_MLT_TEST_COMPLETE are normal values returned. @sa MLT_RETURN_CODES for other possible values.
 *
 *
 * Upon success, the function returns back in pProSLICMLT, the following data in pProSLICMLT->roh.rohTrue:
 *                    - RC_MLT_ROH_NOFAULT   -  no resistive fault or offhook receiver detected
 *                    - RC_MLT_ROH_FAIL_ROH  -  offhook receiver detected
 *                    - RC_MLT_ROH_FAIL_RESFAULT - resistive fault detected
 *             
 * @remark   
 *              - This routine should be called on a per channel basis
 *              - This function is reentrant and must be called at periodic intervals defined by MLT_POLL_RATE
 *              - ProSLIC_mlt_init_receiver_offhook() must be called prior to calling this routine to 
 *                initialize test state variables
 * 
 * @sa ProSLIC_mlt_init_receiver_offhook(), MLT_POLL_RATE
 *
 */
int ProSLIC_mlt_receiver_offhook(ProSLICMLTType *pProSLICMLT,ProSLIC_mlt_roh_state *pState);

/*******************************************************************************************/
/**
 * Function: 	ProSLIC_mlt_init_receiver_offhook
 *
 * @brief       The routine should be called prior to ProSLIC_mlt_receiver_offhook to initialize state variables
 * 
 * @param[in]  *pState - state variable to track reentry
 *
 * @retval  typical return value:  @ref RC_MLT_NO_ERROR @sa MLT_RETURN_CODES for other possible values.
 *  
 * @remark   
 *              - This routine should be called on a per channel basis          
 * 
 * @sa ProSLIC_mlt_receiver_offhook()
 *
 */
int ProSLIC_mlt_init_receiver_offhook(ProSLIC_mlt_roh_state *pState);
/** @}*/


/*******************************************************************************************/
/** @defgroup MLT_REN MLT REN Test
 * This group of functions are used to initialize and execute the REN test.
 * @{
 */
/*******************************************************************************************/
/**
 * Function: 	ProSLIC_mlt_ren
 *
 * @brief       This routine should be called to measure the Ringer Equivalence Number (REN)
 *              present between TIP-RING
 * 
 * @param[in,out]  *pProSLICMLT - pointer to MLT channel structure
 * @param[in]  *pState - state variable to track reentry
 *
 * @retval typical values: @ref RC_MLT_TEST_RUNNING or @ref RC_MLT_TEST_COMPLETE are normal values returned. @sa MLT_RETURN_CODES for other possible values.
 *
 * Upon success, the function returns back in pProSLICMLT, the following data:
 *
 * - pProSLICMLT->ren.revValue - REN value (REN / 1000)
 *               
 * @remark   
 *              - This routine should be called on a per channel basis
 *              - This function is reentrant and must be called at periodic intervals defined by MLT_POLL_RATE
 *              - 1 REN is presummed to be 6930ohms + 8uF
 *              - ProSLIC_mlt_init_ren() must be called prior to calling this routine to 
 *                initialize test state variables
 * 
 * @sa ProSLIC_mlt_init_ren(), MLT_POLL_RATE
 *
 */
int ProSLIC_mlt_ren(ProSLICMLTType *pProSLICMLT,ProSLIC_mlt_ren_state *pState);


/*******************************************************************************************/
/**
 * Function: 	ProSLIC_mlt_init_ren
 *
 * @brief       The routine should be called prior to ProSLIC_mlt_ren to initialize state variables
 * 
 * @param[in]  *pState - state variable to track reentry
 *
 * @retval  typical return value:  @ref RC_MLT_NO_ERROR @sa MLT_RETURN_CODES for other possible values.
 *  
 * @remark   
 *              - This routine should be called on a per channel basis          
 * 
 * @sa @ref ProSLIC_mlt_ren()
 *
 */
int ProSLIC_mlt_init_ren(ProSLIC_mlt_ren_state *pState);
/** @}*/


/*******************************************************************************************/
/** @defgroup MLT_CAPACITANCE MLT Capacitance Test
 * This group of functions are used to initialize and execute the capacitance test.
 * @{
 */
/*******************************************************************************************/
/**
 * Function: 	ProSLIC_mlt_capacitance
 *
 * @brief       This routine should be called to measure the capacitance betweein TIP-RING, TIP-GND, and RING-GND
 * 
 * @param[in,out]  *pProSLICMLT - pointer to MLT channel structure
 * @param[in]  *pState - state variable to track reentry
 *
 * @retval typical values: @ref RC_MLT_TEST_RUNNING or @ref RC_MLT_TEST_COMPLETE are normal values returned. @sa MLT_RETURN_CODES for other possible values.
 *
 * Upon success, the function returns back in pProSLICMLT, the following data:
 *
 * - pProSLICMLT->capFaults.measTG - Tip to GND Capacitance (nF*10)
 * - pProSLICMLT->capFaults.measRG - Ring to GND Capacitance (nF*10)
 * - pProSLICMLT->capFaults.measTR - Tip to Ring Capacitance (nF*10)
 * 
 * @remark   
 *              - This routine should be called on a per channel basis
 *              - This function is reentrant and must be called at periodic intervals defined by MLT_POLL_RATE
 *              - ProSLIC_mlt_init_capacitance() must be called prior to calling this routine to 
 *                initialize test state variables
 * 
 * @sa ProSLIC_mlt_init_capacitance(), MLT_POLL_RATE
 *
 */
int ProSLIC_mlt_capacitance(ProSLICMLTType *pProSLICMLT, ProSLIC_mlt_capacitance_state *pState);

/*******************************************************************************************/
/**
 * Function: 	ProSLIC_mlt_ren_cap
 *
 * @brief       This routine should be called to measure the capacitance betweein TIP-RING, including
 *              what is presented by the phone
 * 
 * @param[in,out]  *pProSLICMLT - pointer to MLT channel structure
 * @param[in]  *pState - state variable to track reentry
 *
 * @retval typical values: @ref RC_MLT_TEST_RUNNING or @ref RC_MLT_TEST_COMPLETE are normal values returned. @sa MLT_RETURN_CODES for other possible values.
 *
 * Upon success, the function returns back in pProSLICMLT, the following data:
 *
 * - pProSLICMLT->capFaults.measTR2 - Tip to Ring Capacitance (nF*10)
 * 
 * @remark   
 *              - This routine should be called on a per channel basis
 *              - This function is reentrant and must be called at periodic intervals defined by MLT_POLL_RATE
 *              - ProSLIC_mlt_init_capacitance() must be called prior to calling this routine to 
 *                initialize test state variables
 * 
 * @sa ProSLIC_mlt_init_capacitance(), MLT_POLL_RATE
 *
 */
int ProSLIC_mlt_ren_cap(ProSLICMLTType *pProSLICMLT, ProSLIC_mlt_ren_cap_state *pState);

/*******************************************************************************************/
/**
 * Function: 	ProSLIC_mlt_init_capacitance
 *
 * @brief       The routine should be called prior to ProSLIC_mlt_capacitance to initialize state variables
 * 
 * @param[in]  *pState - state variable to track reentry
 *
 * @retval  typical return value:  @ref RC_MLT_NO_ERROR @sa MLT_RETURN_CODES for other possible values.
 *  
 * @remark   
 *              - This routine should be called on a per channel basis          
 * 
 * @sa @ref ProSLIC_mlt_capacitance()
 *
 */
int ProSLIC_mlt_init_capacitance(ProSLIC_mlt_capacitance_state *pState);
/** @}*/

/*******************************************************************************************/
/** @defgroup MLT_ABORT MLT Abort Test
 * This group of functions are used to abort an MLT test.
 * @{
 */
/*******************************************************************************************/
/**
 * Function: 	ProSLIC_mlt_abort
 *
 * @brief       The routine should be called to abort an MLT routine
 * 
 * @param[in]  *pProSLICMLT - pointer to MLT channel structure
 * @param[in]  *pState - state variable to track reentry
 *
 * @retval typical values: @ref RC_MLT_TEST_RUNNING or @ref RC_MLT_ABORT are normal values returned. @sa MLT_RETURN_CODES for other possible values.
 *
 *  
 * @remark   
 *              - This routine should be called on a per channel basis
 *              - This function is reentrant and must be called at periodic intervals defined by MLT_POLL_RATE           
 * 
 * @sa MLT_POLL_RATE
 *
 */
int ProSLIC_mlt_abort (ProSLICMLTType *pProSLICMLT,ProSLIC_mlt_test_state *pState);
/** @}*/

/*******************************************************************************************/
/** @defgroup MLT_RENCAL MLT REN Calibration Test
 * This group of functions are used to initialize the REN calibration coefficients.
 * @{
 */
/*******************************************************************************************/
/**
 * Function: 	ProSLIC_mlt_init_ren_cal
 *
 * @brief       The routine should be called to initialize REN calibration coefficients
 * 
 * @param[in,out]  *pProSLICMLT - pointer to MLT channel structure
 *
 * @retval  typical return value:  @ref RC_MLT_NO_ERROR @sa MLT_RETURN_CODES for other possible values.
 *
 * Upon function exit, the following data would of been modified:
 * - pProSLICMLT->ren.calData - Structure updated with MLT_RENCAL values in @ref proslic_mlt_api_config.h
 *  
 * @remark   
 *              - This routine should be called on a per channel basis         
 *
 */
int ProSLIC_mlt_init_ren_cal(ProSLICMLTType *pProSLICMLT);
/** @}*/



/*******************************************************************************************/
/** @defgroup MLT_DATA_MGMT Management of MLT Test Data
 * This group of functions are used to manage the MLT test results and flags.
 * @{
 */
/*******************************************************************************************/
/**
 * Function: 	ProSLIC_mlt_clear_results
 *
 * @brief       The routine should be called to clear MLT test results
 * 
 * @param[in,out]  *pProSLICMLT - pointer to MLT channel structure
 *
 * @retval  typical return value:  @ref RC_MLT_NO_ERROR @sa MLT_RETURN_CODES for other possible values.
 *  
 * @remark   
 *              - This routine should be called on a per channel basis
 *
 */
int ProSLIC_mlt_clear_results(ProSLICMLTType *pProSLICMLT);


#ifdef MLT_RUNTIME_CONFIG
/*******************************************************************************************/
/**
 * Function: 	ProSLIC_mlt_init_default_config
 *
 * @brief       Initialize per-channel configuration paramters from macros
 *              in proslic_mlt_api_config.h
 * 
 * @param[in,out]  *pProSLICMLT - pointer to MLT channel structure
 *
 * @retval  typical return value:  @ref RC_MLT_NO_ERROR @sa MLT_RETURN_CODES for other possible values.
 *  
 * @remark   
 *              - This routine should be called on a per channel basis
 *
 */
int ProSLIC_mlt_init_default_config(ProSLICMLTType *pProSLICMLT);

/*******************************************************************************************/
/**
 * Function: 	ProSLIC_mlt_init_custom_config
 *
 * @brief       Initialize per-channel configuration paramters from custom user parameters.
 *              This allows the parameters to be modified during runtime.
 * 
 * @param[in,out]  *pProSLICMLT - pointer to MLT channel structure
 * @param[in]      *mlt_config - pointer to user's mlt configuration structure
 *
 * @retval  typical return value:  @ref RC_MLT_NO_ERROR @sa MLT_RETURN_CODES for other possible values.
 *  
 * @remark   
 *              - This routine should be called on a per channel basis
 *
 */
int ProSLIC_mlt_init_custom_config(ProSLICMLTType *pProSLICMLT,ProSLIC_mlt_config_t *mlt_config);
#endif

/*******************************************************************************************/
/**
 * Function: 	ProSLIC_mlt_version
 *
 * @brief       The routine returns ProSLIC MLT API Version string
 *
 * @retval      Pointer to MLT API Version string *ProSLICMLTAPIVersion
 *  
 * @sa proslic_mlt_version.c
 *
 */
const char *ProSLIC_mlt_version(void);

/** @}*/
/**@}*/
#endif
