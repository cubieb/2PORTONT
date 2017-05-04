/** \lt_api_all_fft.h
 * lt_api_all_fft.h
 *
 * This file defines the necessary interface aspects for the all fundamental
 * fault tests.
 *
 * This file assumes a certain include order to compile.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 9115 $
 * $LastChangedDate: 2011-11-15 15:25:17 -0600 (Tue, 15 Nov 2011) $
 */

#ifndef _LT_API_TEST_ALL_GR_909_H_
#define _LT_API_TEST_ALL_GR_909_H_

#include "lt_api_test_line_v.h"
#include "lt_api_test_roh.h"
#include "lt_api_test_ringers.h"
#include "lt_api_test_res_flt.h"

/*******************************************************************************
 * Definitions for test inputs                                                 *
 ******************************************************************************/
typedef struct {
    /* Inputs for test LT_TID_LINE_V */
    LtLineVInputType lineVInp;

    /* Inputs for test LT_TID_ROH */
    LtRohInputType rohInp;

    /* Inputs for test LT_TID_RINGERS */
    LtRingerInputType ringersInp;

    /* Inputs for test LT_TID_RES_FLT */
    LtResFltInputType resFltInp;

} LtAllGr909InputType;

/*******************************************************************************
 * Definitions for test criteria                                               *
 ******************************************************************************/
typedef struct {
    /* Test criteria for LT_TID_LINE_V */
    LtLineVCriteriaType lintVCrt;

    /* Test criteria for LT_TID_ROH */
    LtRohCriteriaType rohCrt;

    /* Test criteria for LT_TID_RINGERS */
    LtRingersCriteriaType ringersCrt;

    /* Test criteria for LT_TID_RES_FLT */
    LtResFltCriteriaType resFltCrt;

} LtAllGr909CriteriaType;

/*******************************************************************************
 * Definitions for test results                                                *
 ******************************************************************************/
typedef enum {
    LT_A909TM_TEST_PASSED         = LT_TEST_PASSED,
    LT_A909TM_LINE_V_HEMF_FAILED  = 0x0001,
    LT_A909TM_LINE_V_FEMF_FAILED  = 0x0002,
    LT_A909TM_ROH_FAILED          = 0x0004,
    LT_A909TM_RINGERS_FAILED      = 0x0008,
    LT_A909TM_RES_FLT_FAILED      = 0x0010,
    LT_A909TM_MSRMNT_STATUS       = LT_TEST_MEASUREMENT_ERROR,
    LT_A909TM_BIT_ENUM_SIZE       = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtAllGr909BitType;

typedef uint16 LtAllGr909MaskType;

typedef enum {
    LT_A909STM_NO_PF_TESTED = 0x0000,
    LT_A909STM_LINE_V_HEMF  = 0x0001,
    LT_A909STM_LINE_V_FEMF  = 0x0002,
    LT_A909STM_ROH          = 0x0004,
    LT_A909STM_RINGERS      = 0x0008,
    LT_A909STM_RES_FLT      = 0x0010,
    LT_A909STM_BIT_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtAllGr909SubTestBitType;

typedef uint16 LtAllGr909SubTestMaskType;

typedef struct {
    LtAllGr909MaskType fltMask;
    LtMsrmntStatusMaskType measStatus;
    LtAllGr909SubTestMaskType subTestMask;

    /* Test results for LT_TID_LINE_V */
    LtLineVResultType  lineV;

    /* Test results for LT_TID_ROH */
    LtRohResultType roh;

    /* Test results for LT_TID_RINGERS */
    LtRingersResultType ringers;

    /* Test results for LT_TID_RES_FLT */
    LtResFltResultType resFlt;

} LtAllGr909ResultType;


#endif /* !_LT_API_TEST_ALL_GR_909_H_ */
