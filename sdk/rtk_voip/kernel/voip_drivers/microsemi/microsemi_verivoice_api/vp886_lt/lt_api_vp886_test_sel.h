/** \lt_api_vp886_test_sel.h
 *
 * This file contains the conditional compilations selections for 886 
 * specific line test. Any test with the conditional compile 
 * "UNDER_DEVELOPMENT_886" cannot be used.
 *
 * Copyright (c) 2012, Microsemi Corporation
 *
 * $Revision: 10571 $
 * $LastChangedDate: 2012-11-01 16:48:09 -0500 (Thu, 01 Nov 2012) $
 */

#ifndef _LT_API_VP886_TEST_SEL_H_
#define _LT_API_VP886_TEST_SEL_H_

#if defined(VP886_INCLUDE_TESTLINE_CODE) && defined(LT_VP886_PACKAGE)

    #if defined(LT_LINE_V)
        #define LT_LINE_V_886
    #endif

    #if defined(LT_ROH)
        #define LT_ROH_886
    #endif

    #if defined(LT_RINGERS)
        #define LT_RINGERS_886
    #endif

    #if defined(LT_RES_FLT)
        #define LT_RES_FLT_886
    #endif

    #if defined(LT_MSOCKET)
        #define LT_MSOCKET_886
    #endif

    #if defined(LT_XCONNECT)
        #define LT_XCONNECT_886
    #endif

    #if defined(LT_CAP)
        #define LT_CAP_886
    #endif

    #if defined(LT_LOOPBACK)
        #define LT_LOOPBACK_886
    #endif

    #if defined(LT_DC_FEED_ST)
        #define LT_DC_FEED_ST_886
    #endif

    #if defined(LT_RD_LOOP_COND)
        #define LT_RD_LOOP_COND_886
    #endif

    #if defined(LT_DC_VOLTAGE)
        #define LT_DC_VOLTAGE_886
    #endif

    #if defined(LT_RINGING_ST)
        #define LT_RINGING_ST_886
    #endif

    #if defined(LT_ON_OFF_HOOK_ST)
        #define LT_ON_OFF_HOOK_ST_886
    #endif

    #if defined(LT_RD_BAT_COND)
        #define LT_RD_BAT_COND_886
    #endif

    #if defined(LT_PRE_LINE_V)
        #define LT_PRE_LINE_V_886
    #endif

    #if defined(LT_ALL_GR_909) && defined(LT_LINE_V_886) && defined(LT_ROH_886) && defined(LT_RINGERS_886) && defined(LT_RES_FLT_886)
        #define LT_ALL_GR_909_886
    #endif

#endif /* VP886_INCLUDE_TESTLINE_CODE && LT_VP886_PACKAGE */

#endif /* _LT_API_VP886_TEST_SEL_H_ */
