/*
** Copyright (c) 2010-2011 by Silicon Laboratories
**
** $Id: si3226x_mlt.h 4551 2014-10-27 20:57:24Z nizajerk $
**
*/
/*! \file si3226x_mlt.h
**  \brief Si3226x ProSLIC MLT interface header file
**
**  This is the si3226x MLT interface header file
**
** \author Silicon Laboratories, Inc (cdp)
**
** \attention
** This file contains proprietary information.	 
** No dissemination allowed without prior written permission from
** Silicon Laboratories, Inc.
**
*/

#ifndef SI3226X_MLT_H
#define SI3226X_MLT_H

/** @internal @defgroup SI3226X_DRIVER_LEVEL_API  Si3226x Driver Level MLT APIs
 *  This group contains si3226x MLT driver functions that are called by the top level ProSLIC MLT APIs
 *
 *  Functions in this module should not be called directly by code outside of the ProSLIC API
 *@{
 */


/*******************************************************************************************/
/**
 * Function: 	si3226x_mlt_foreign_voltages
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
 * -  pProSLICMLT->hazVAC.measTG - Tip to GND AC voltage (Vrms^2 / 100)
 * -  pProSLICMLT->hazVAC.measRG - Ring to GND AC voltage (Vrms^2 / 100)
 * -  pProSLICMLT->hazVAC.measTR - Tip to Ring AC voltage (Vrms^2 / 100)
 * -  pProSLICMLT->hazVDC.measTG - Tip to GND DC voltage (mV)
 * -  pProSLICMLT->hazVDC.measRG - Ring to GND DC voltage (mV)
 * -  pProSLICMLT->hazVDC.measTR - Tip to Ring DC voltage (mV)
 *
 * @remark   
 *              - This function is wrapped by @ref ProSLIC_mlt_foreign_voltages() and should not be called by
 *                high level application code
 *              - This routine should be called on a per channel basis
 *              - This function is reentrant and must be called at periodic intervals defined by MLT_POLL_RATE
 * 
 * @sa ProSLIC_mlt_init_foreign_voltages, MLT_POLL_RATE
 *
 */
int si3226x_mlt_foreign_voltages(ProSLICMLTType *pProSLICMLT, ProSLIC_mlt_foreign_voltages_state *pState);

/*******************************************************************************************/
/**
 * Function: 	si3226x_mlt_hazard_voltages
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
 *              - This function is wrapped by @ref ProSLIC_mlt_hazard_voltages() and should not be called by
 *                high level application code
 *              - This routine should be called on a per channel basis
 *              - This function is reentrant and must be called at periodic intervals defined by MLT_POLL_RATE
 * 
 * @sa ProSLIC_mlt_init_foreign_voltages(), MLT_POLL_RATE
 *
 */
int si3226x_mlt_hazard_voltages(ProSLICMLTType *pProSLICMLT, ProSLIC_mlt_foreign_voltages_state *pState);

/*******************************************************************************************/
/**
 * Function: 	si3226x_mlt_resistive_faults
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
 *              - This function is wrapped by @ref ProSLIC_mlt_resistive_faults() and should not be called by
 *                high level application code
 *              - This routine should be called on a per channel basis
 *              - This function is reentrant and must be called at periodic intervals defined by MLT_POLL_RATE
 * 
 * @sa ProSLIC_mlt_init_resistive_faults(), MLT_POLL_RATE
 *
 */
int si3226x_mlt_resistive_faults(ProSLICMLTType *pProSLICMLT, ProSLIC_mlt_rmeas_state *pState);

/*******************************************************************************************/
/**
 * Function: 	si3226x_mlt_receiver_offhook
 *
 * @brief       This routine should be called to detect the presence of an offhook receiver (FXO)
 * 
 * @param[in,out]  *pProSLICMLT - pointer to MLT channel structure
 * @param[in]  *pState - state variable to track reentry
 *
 * @retval typical values: @ref RC_MLT_TEST_RUNNING or @ref RC_MLT_TEST_COMPLETE are normal values returned. @sa MLT_RETURN_CODES for other possible values.
 *
 * Upon success, the function returns back in pProSLICMLT, the following data in pProSLICMLT->roh.rohTrue:
 *                    - RC_MLT_ROH_NOFAULT   -  no resistive fault or offhook receiver detected
 *                    - RC_MLT_ROH_FAIL_ROH  -  offhook receiver detected
 *                    - RC_MLT_ROH_FAIL_RESFAULT - resistive fault detected
 *             
 * @remark   
 *              - This function is wrapped by @ref ProSLIC_mlt_receiver_offhook() and should not be called by
 *                high level application code
 *              - This routine should be called on a per channel basis
 *              - This function is reentrant and must be called at periodic intervals defined by MLT_POLL_RATE
 * 
 * @sa ProSLIC_mlt_init_receiver_offhook(), MLT_POLL_RATE
 *
 */
int si3226x_mlt_receiver_offhook(ProSLICMLTType *pProSLICMLT,ProSLIC_mlt_roh_state *pState);

/*******************************************************************************************/
/**
 * Function: 	si3226x_mlt_ren
 *
 * @brief       This routine should be called to measure the Ringer Equivalence Number (REN)
 *              present between TIP-RING
 * 
 * @param[in]  *pProSLICMLT - pointer to MLT channel structure
 * @param[in]  *pState - state variable to track reentry
 *
 * @retval typical values: @ref RC_MLT_TEST_RUNNING or @ref RC_MLT_TEST_COMPLETE are normal values returned. @sa MLT_RETURN_CODES for other possible values.
 *
 * Upon success, the function returns back in pProSLICMLT, the following data:
 *
 * - pProSLICMLT->ren.revValue - REN value (REN / 1000)
 *               
 * @remark   
 *              - This function is wrapped by @ref ProSLIC_mlt_ren() and should not be called by
 *                high level application code
 *              - This routine should be called on a per channel basis
 *              - This function is reentrant and must be called at periodic intervals defined by MLT_POLL_RATE
 *              - 1 REN is presummed to be 6930ohms + 8uF
 * 
 * @sa ProSLIC_mlt_init_ren(), MLT_POLL_RATE
 *
 */
int si3226x_mlt_ren(ProSLICMLTType *pProSLICMLT,ProSLIC_mlt_ren_state *pState);


/*******************************************************************************************/
/**
 * Function: 	si3226x_mlt_capacitance
 *
 * @brief       This routine should be called to measure the capacitance betweein TIP-RING, TIP-GND, and RING-GND
 * 
 * @param[in]  *pProSLICMLT - pointer to MLT channel structure
 * @param[in]  *pState - state variable to track reentry
 *
 * @retval typical values: @ref RC_MLT_TEST_RUNNING or @ref RC_MLT_TEST_COMPLETE are normal values returned. @sa MLT_RETURN_CODES for other possible values.
 *
 * Upon success, the function returns back in pProSLICMLT, the following data:
 *
 * - pProSLICMLT->capFaults.measTG - Tip to GND Capacitance (nF*10)
 * -  pProSLICMLT->capFaults.measRG - Ring to GND Capacitance (nF*10)
 * - pProSLICMLT->capFaults.measTR - Tip to Ring Capacitance (nF*10)
 * 
 * @remark   
 *              - This function is wrapped by @ref ProSLIC_mlt_capacitance() and should not be called by
 *                high level application code
 *              - This routine should be called on a per channel basis
 *              - This function is reentrant and must be called at periodic intervals defined by MLT_POLL_RATE
 * 
 * @sa ProSLIC_mlt_init_capacitance(), MLT_POLL_RATE
 *
 */
int si3226x_mlt_capacitance (ProSLICMLTType *pProSLICMLT,ProSLIC_mlt_capacitance_state *pState);

/*******************************************************************************************/
/**
 * Function: 	si3226x_mlt_ren_cap
 *
 * @brief       This routine should be called to measure the capacitance betweein TIP-RING, including the phone
 * 
 * @param[in]  *pProSLICMLT - pointer to MLT channel structure
 * @param[in]  *pState - state variable to track reentry
 *
 * @retval typical values: @ref RC_MLT_TEST_RUNNING or @ref RC_MLT_TEST_COMPLETE are normal values returned. @sa MLT_RETURN_CODES for other possible values.
 *
 * Upon success, the function returns back in pProSLICMLT, the following data:
 *
 * - pProSLICMLT->capFaults.measTR - Tip to Ring Capacitance (nF*10)
 * 
 * @remark   
 *              - This function is wrapped by @ref ProSLIC_mlt_capacitance() and should not be called by
 *                high level application code
 *              - This routine should be called on a per channel basis
 *              - This function is reentrant and must be called at periodic intervals defined by MLT_POLL_RATE
 * 
 * @sa ProSLIC_mlt_init_capacitance(), MLT_POLL_RATE
 *
 */
int si3226x_mlt_ren_cap (ProSLICMLTType *pProSLICMLT,ProSLIC_mlt_ren_cap_state *pState);

/*******************************************************************************************/
/**
 * Function: 	si3226x_mlt_abort
 *
 * @brief       The routine should be called to abort an MLT routine
 * 
 * @param[in]  *pProSLICMLT - pointer to MLT channel structure
 * @param[in]  *pState - state variable to track reentry
 *
 * @retval typical values: @ref RC_MLT_TEST_RUNNING or @ref RC_MLT_ABORT are normal values returned. @sa MLT_RETURN_CODES for other possible values.
 *  
 * @remark   
 *              - This function is wrapped by @ref ProSLIC_mlt_abort() and should not be called by
 *                high level application code
 *              - This routine should be called on a per channel basis
 *              - This function is reentrant and must be called at periodic intervals defined by MLT_POLL_RATE           
 * 
 * @sa MLT_POLL_RATE
 *
 */
int si3226x_mlt_abort (ProSLICMLTType *pProSLICMLT,ProSLIC_mlt_test_state *pState);


/**@}*/

#endif

