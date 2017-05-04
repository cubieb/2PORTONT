/** \lt_api_internal.h
 *
 * This file is internal to Line Test API. It contains definitions/declarations
 * used LT-API.
 *
 * Copyright (c) 2012, Microsemi Corporation
 *
 * $Revision: 10096 $
 * $LastChangedDate: 2012-06-05 11:32:26 -0500 (Tue, 05 Jun 2012) $
 */


#ifndef _LT_API_INTERNAL_H_
#define _LT_API_INTERNAL_H_

#include "lt_api.h"

#ifndef ABS
    #define ABS(a) ( ((a) < 0) ? -(a) : (a))
#endif

#ifndef MAX
    #define MAX(a, b) ((a < b) ? b : a)
#endif

#ifndef MIN
    #define MIN(a, b) ((a < b) ? a : b)
#endif

#define LT_GLOBAL_FIRST_STATE 0
#define LT_GLOBAL_ABORTED_STATE -1

/* all RES values are in tenth of Ohms */
#define VVP_RES_FLT_OPEN_CIRCUIT                20000000  /* 2 Meg */
#define VVP_RES_FLT_SHORT_TO_GND                5000      /* 500 Ohms */
#define VVP_RES_FLT_SWITCH_GAIN_RES             330000    /* 33 kOhms */
#define VVP_INTERNAL_TEST_SWITCH_VDC_DROP       2500      /* 2500 mV */
#define VVP_INTERNAL_TEST_SWITCH_VAVG_DROP      2000      /* 2000 mV */
#define VVP_INTERNAL_TEST_SWITCH_IMPED          100       /* 10 Ohms */

EXTERN void *
    LtMemCpy(
    void * dest,
    const void *src,
    uint16 count);

/* 880/890 common function for the extended foreign voltage */
EXTERN int32
LtLineVAcDecompression(
    int32 AcVoltage);

EXTERN void
LtInitOutputStruct(
    LtTestResultType *pResult,
    LtTestIdType testId);


#ifdef LT_VP880_PACKAGE
    #include "lt_api_vp880_int.h"
#endif

#ifdef LT_VP890_PACKAGE
    #include "lt_api_vp890_int.h"
#endif

#ifdef LT_VP886_PACKAGE
    #include "lt_api_vp886_int.h"
#endif

#endif /* !_LT_API_INTERNAL_H_ */
