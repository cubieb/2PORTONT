/** @file
 *  @brief This file contains constant defined values for configuration options. Users should not edit this
 *  file. Following are the 5 lists contained in this file:
 *  @li AC48/9x Device Types Definitions
 *  @li AC49x HPI Types Definitions
 *  @li Device Memory Access Definitions
 *  @li Endian (Byte-Order) Mode Definitions
 *  @li Packet Header Type Related Definition
 */


/***********************************************************************************/
/* Ac49xCfg.h - AC49x Device Driver / Constant Configuration Options Definitions   */
/*                                                                                 */
/* Copyright (C) 2000 AudioCodes Ltd.                                              */
/*                                                                                 */
/* Modifications :                                                                 */
/*                                                                                 */
/* 1/7/01 - Coding started.                                                        */
/*                                                                                 */
/* *****************                                                               */
/* IMPORTANT NOTES :                                                               */
/* *****************                                                               */
/*                                                                                 */
/* This file contains constant values for configuration options.                   */
/* The user SHOULD NOT edit this file.                                             */
/*                                                                                 */
/***********************************************************************************/

#ifndef AC49XOPT_H
#define AC49XOPT_H

/*******************************************/
/*******************************************/
/**         AC49x Drivers Versions		  **/
/*******************************************/
/*******************************************/
#define AC49X_DRIVER_VERSION_105 105
#define AC49X_DRIVER_VERSION_106 106
#define AC49X_DRIVER_VERSION_107 107
#define AC49X_DRIVER_VERSION_108 108
#define AC49X_DRIVER_VERSION_520 520

#define AC49X_DRIVER_VERSION_540 540

#define AC49X_DRIVER_VERSION_200 200
#define AC49X_DRIVER_VERSION_210 210


/*******************************************/
/*******************************************/
/**  AC48/9x Device Types Definitions     **/
/*******************************************/
/*******************************************/
#define	AC490_DEVICE	 1
#define	AC491_DEVICE	 2
#define	AC494_DEVICE	 3
#define AC495_DEVICE	 4
#define AC496_DEVICE	 5
#define AC497_DEVICE	 6

/**  Lexra Device baseed Types Definitions     **/
#define ACLX5280_DEVICE  100


/*******************************************/
/*******************************************/
/**  AC48/9x HPI Types Definitions     **/
/*******************************************/
/*******************************************/
#define	AC49X_HPI_NONE 0
#define	AC49X_HPI_PORT 1


/*******************************************/
/*******************************************/
/** Endian (Byte-Order) Mode Definitions  **/
/*******************************************/
/*******************************************/

#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN	0	/* (LSB first) should be used for Intel CPUs, */
#endif

#ifndef BIG_ENDIAN
#define BIG_ENDIAN		1	/* (MSB first) should be used for Motoral/RISC CPUs */
#endif

#endif /* ifndef AC49XOPT_H */
