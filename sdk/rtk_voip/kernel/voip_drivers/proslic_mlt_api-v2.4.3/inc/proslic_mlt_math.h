/*
** Copyright (c) 2007-2011 by Silicon Laboratories
**
** $Id: proslic_mlt_math.h 4551 2014-10-27 20:57:24Z nizajerk $
**
*/
/*! \file proslic_mlt_math.h
**  \brief ProSLIC MLT math functions header file
**  
**	This is the header file for the ProSLIC MLT math functions.
**
**  \author Silicon Laboratories, Inc (laj, cdp)
**
**  \attention 
**	This file contains proprietary information.	 
**	No dissemination allowed without prior written permission from
**	Silicon Laboratories, Inc. 
*/

#ifndef PROSLIC_MLT_MATH_H
#define PROSLIC_MLT_MATH_H

/** @internal @defgroup PROSLIC_MATH  ProSLIC math utility APIs
 *  This group contains the APIs for math utilities used by the ProSLIC API driver code
 *
 *@{
 */

/*******************************************************************************************/
/** 
 *
 * Function: 	fp_abs
 *
 * @brief       This routine returns absolute value of a 32-bit 2's compliment number
 * 
 * @param[in]  a  - value to be evaluated
 *
 * @retval  int32 - abs(a)
 *
 *
 */
int32 fp_abs(int32 a);


/*******************************************************************************************/
/**
 *
 * Function: 	Isqrt
 *
 * @brief       This routine returns a square-root approximation of passed 32-bit unsigned integer
 * 
 * @param[in]  number - value to be square rooted
 *
 * @retval  uInt32 - square root of number
 *
 *
 */
uInt32 Isqrt(uInt32 number);


/*******************************************************************************************/
/**
 *
 * Function: 	computeResTC
 *
 * @brief       This routine returns an impedance based on time constant measurement
 * 
 * @param[in]  v1 -   first voltage measurement
 * @param[in]  v2 -   second voltage measurement
 * @param[in]  delta_t - time delay between voltage measurements
 * @param[in]  cEst - estimated capacitance
 * @param[in]  *tau - estimate time constant 
 *
 * @retval  int32 -  impedance
 *
 *
 */
int32 computeResTC(int32 v1, int32 v2, int32 delta_t, int32 cEst , int32 *tau);



/*******************************************************************************************/
/**
 *
 * Function: 	dBLookup
 *
 * @brief       This routine converts the passed ratio to dB
 * 
 * @param[in]  number -  input ratio (typ mVpk/mVpkref)
 *
 * @retval  int32 -  20*LOG10(number)
 *
 * @remark 	This function returns the power (in dB*10) given an input voltage ratio
 *   (mVpk/mVpkref) based on a lookup table
 *
 */
int32 dBLookup(uInt32 number);

/*@}*/
#endif
