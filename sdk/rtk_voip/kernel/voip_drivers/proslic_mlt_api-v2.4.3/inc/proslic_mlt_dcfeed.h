/*
** Copyright (c) 2011-2012 by Silicon Laboratories
**
** $Id: proslic_mlt_dcfeed.h 4551 2014-10-27 20:57:24Z nizajerk $
**
*/
/*! \file proslic_mlt_dcfeed.h
**  \brief ProSLIC MLT dc feed setup header file
**
** This is the header file for the ProSLIC DC feed setup during MLT tests
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

#ifndef PROSLIC_MLT_DCFEED_H
#define PROSLIC_MLT_DCFEED_H



/**
 * @internal @defgroup PROSLIC_DCFEED_API ProSLIC DC Feed Setup APIs
 * These functions are used by the MLT device drivers and never called from a higher level routine
 *
 * @{
 */

/********************************************************************************/
/**
** @brief Load custom DC feed parameters to drive TIP/RING close to 0v
** 
** @param[in] ProSLICMLTType *pProSLICMLT -  Pointer to MLT channel structure
**
** @retval void
**
*/
void setupDcFeedCloseToZero(ProSLICMLTType *pProSLICMLT);

/********************************************************************************/
/**
** @brief Load custom DC feed parameters to drive TIP/RING to 10v
** 
** @param[in] ProSLICMLTType *pProSLICMLT -  Pointer to MLT channel structure
**
** @retval void
**
*/
void setupDcFeed10VLONG (ProSLICMLTType *pProSLICMLT);

/********************************************************************************/
/**
** @brief Load custom DC feed parameters to drive TIP/RING to 50v
** 
** @param[in] ProSLICMLTType *pProSLICMLT -  Pointer to MLT channel structure
**
** @retval void
**
*/
void setupDcFeed50VLONG (ProSLICMLTType *pProSLICMLT);

/********************************************************************************/
/**
** @brief Load custom DC feed parameters to drive TIP/RING to 35v
** 
** @param[in] ProSLICMLTType *pProSLICMLT -  Pointer to MLT channel structure
**
** @retval void
**
*/
void setupDcFeed35VCM (ProSLICMLTType *pProSLICMLT);

/********************************************************************************/
/**
** @brief Load custom DC feed parameters to drive VTR to 35v
** 
** @param[in] ProSLICMLTType *pProSLICMLT -  Pointer to MLT channel structure
**
** @retval void
**
*/
void setupDcFeed35V (ProSLICMLTType *pProSLICMLT);

/********************************************************************************/
/**
** @brief Load custom DC feed parameters for 40v/14mA feed
** 
** @param[in] ProSLICMLTType *pProSLICMLT -  Pointer to MLT channel structure
**
** @retval void
**
*/
void setupDcFeedV1SmallR (ProSLICMLTType *pProSLICMLT);

/********************************************************************************/
/**
** @brief Load custom DC feed parameters for 35v/7mA feed
** 
** @param[in] ProSLICMLTType *pProSLICMLT -  Pointer to MLT channel structure
**
** @retval void
**
*/
void setupDcFeedV2SmallR (ProSLICMLTType *pProSLICMLT);

/********************************************************************************/
/**
** @brief Load custom DC feed parameters for 45v/20mA feed
** 
** @param[in] ProSLICMLTType *pProSLICMLT -  Pointer to MLT channel structure
**
** @retval void
**
*/
void setupDcFeed45V (ProSLICMLTType *pProSLICMLT);

/********************************************************************************/
/**
** @brief Load custom DC feed parameters for REN test
** 
** @param[in] ProSLICMLTType *pProSLICMLT -  Pointer to MLT channel structure
**
** @retval void
**
*/
void setupDcFeedForRENTest (ProSLICMLTType *pProSLICMLT);

/********************************************************************************/
/**
** @brief Load custom DC feed parameters for REN test
** 
** @param[in] ProSLICMLTType *pProSLICMLT -  Pointer to MLT channel structure
**
** @retval void
**
*/
void setupDcFeedAutoV (ProSLICMLTType *pProSLICMLT);
/**@}*/
#endif

