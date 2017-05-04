/** \lt_api_pkg_sel.h
 * lt_api_pkg_sel.h
 *
 * This file contains the conditional compilations selections for Line Test
 * API. Please implement the necessary modifications based on your application
 * needs.
 *
 * Copyright (c) 2012, Microsemi Corporation
 *
 * $Revision: 10190 $
 * $LastChangedDate: 2012-06-21 17:53:11 -0500 (Thu, 21 Jun 2012) $
 */

#ifndef _LT_API_PKG_SEL_H_
#define _LT_API_PKG_SEL_H_

/*******************************************************************************
 * Define the following to enable Line Test debugging                          *
 ******************************************************************************/
//#define  LT_DEBUG

/*******************************************************************************
 * Define the following to enable VoicePort 880 Pass/Fail line testing         *
 ******************************************************************************/
/*#define LT_VP880_VVA_PACKAGE*/

/*******************************************************************************
 * Define the following to enable VoicePort 880 Basic line testing             *
 ******************************************************************************/
/*#define LT_VP880_VVP_PACKAGE*/

/*******************************************************************************
 * Define the following to enable VoicePort 890 Pass/Fail line testing         *
 ******************************************************************************/
/*#define LT_VP890_VVA_PACKAGE*/

/*******************************************************************************
 * Define the following to enable VoicePort 890 Basic line testing             *
 ******************************************************************************/
#if defined( CONFIG_RTK_VOIP_SLIC_ZARLINK_890_SERIES ) 
#define LT_VP890_VVP_PACKAGE
#endif

/*******************************************************************************
 * Define the following to enable VoicePort 886 Basic line testing             *
 ******************************************************************************/
#if defined( CONFIG_RTK_VOIP_SLIC_ZARLINK_886_SERIES ) 
#define LT_VP886_VVP_PACKAGE
#endif

/*******************************************************************************
 * Define the following to enable the specfied line tests                      *
 ******************************************************************************/
#define LT_PRE_LINE_V      /* Pre Line Voltage Test */
#define LT_LINE_V          /* Line Voltage Test */
#define LT_ROH             /* Receiver Off-Hook indication */
#define LT_RINGERS         /* Ringers test per FCC Part 68 REN def.*/
#define LT_RES_FLT         /* Resistive Fault */
#define LT_MSOCKET         /* Master Socket test */
#define LT_XCONNECT        /* Cross Connect test */
#define LT_CAP             /* Capacitance test */
#define LT_ALL_GR_909      /* All GR-909 fault tests in predefined order
                            * if LT_LINE_V, LT_ROH, LT_RINGERS, LT_RES_FLT
                            * must all be defined inorder to define this test */
#define LT_LOOPBACK        /* Loopback test */
#define LT_DC_FEED_ST      /* DC Feed Self Test */
#define LT_RD_LOOP_COND    /* Read Loop Condition Test */
#define LT_DC_VOLTAGE      /* DC VOLTAGE Test */
#define LT_RINGING_ST      /* Ringing Self Test */
#define LT_ON_OFF_HOOK_ST  /* On/Off hook Self Test */
#define LT_RD_BAT_COND     /* Read battery conditions */
#define LT_FLT_DSCRM       /* Fault Discrimination */

/*******************************************************************************
 * NOTE: Please limit your modifications in this file to above this line       *
 ******************************************************************************/

#if defined(LT_VP880_VVA_PACKAGE) || defined(LT_VP880_VVP_PACKAGE)
/* Define a easy conditional compile flag for VoicePort line testing */
  #ifndef LT_VP880_PACKAGE
    #define LT_VP880_PACKAGE
  #endif
#endif /* LT_VP880_PASS_FAIL_PACKAGE or LT_VP880_BASIC_TEST_PACKAGE */

#if defined(LT_VP890_VVA_PACKAGE) || defined(LT_VP890_VVP_PACKAGE)
/* Define a easy conditional compile flag for VoicePort line testing */
  #ifndef LT_VP890_PACKAGE
    #define LT_VP890_PACKAGE
  #endif
#endif /* LT_VP880_PASS_FAIL_PACKAGE or LT_VP880_BASIC_TEST_PACKAGE */

#if defined(LT_VP886_VVP_PACKAGE)
/* Define a easy conditional compile flag for VoicePort line testing */
  #ifndef LT_VP886_PACKAGE
    #define LT_VP886_PACKAGE
  #endif
#endif /* LT_VP886_VVP_PACKAGE */

#endif /* !_LT_API_PKG_SEL_H_ */
