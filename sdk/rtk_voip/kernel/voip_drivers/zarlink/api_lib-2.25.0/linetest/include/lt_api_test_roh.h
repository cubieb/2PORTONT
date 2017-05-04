/** \lt_api_test_roh.h
 * lt_api_test_roh.h
 *
 * This file defines the necessary interface aspects for the Receiver Off Hook
 * measurement test.
 *
 * This file assumes a certain include order to compile.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 9115 $
 * $LastChangedDate: 2011-11-15 15:25:17 -0600 (Tue, 15 Nov 2011) $
 */

#ifndef _LT_API_TEST_ROH_H_
#define _LT_API_TEST_ROH_H_

/*******************************************************************************
 * Definitions for test inputs                                                 *
 ******************************************************************************/
/* DevNotes: This is dummy definition which is not used at the moment.
 * However the test implementation must check to make sure the pointers are
 * NULL before running the test. If it is not NULL test should throw an error
 * until this structure is defined.
 * As the we redefine this structure in future, applications that are written
 * to make use of the default test input feature will continue to work. The
 * new applications could make use of the new structure to specify inputs.
 */
typedef struct {
    /* Dummy place holder */
    char dummy;
} LtRohInputType;


/*******************************************************************************
 * Definitions for test criteria                                               *
 ******************************************************************************/
/* DevNotes: This is dummy definition which is not used at the moment.
 * However the test implementation must check to make sure the pointers are
 * NULL before running the test. If it is not NULL test should throw an error
 * until this structure is defined.
 * As the we redefine this structure in future, applications that are written
 * to make use of the default test input feature will continue to work. The
 * new applications could make use of the new structure to specify inputs.
 */
typedef struct {
    /* Dummy place holder */
    char dummy;
} LtRohCriteriaType;


/*******************************************************************************
 * Definitions for test results                                                *
 ******************************************************************************/
typedef enum {
    LT_ROHM_TEST_PASSED = LT_TEST_PASSED,
    LT_ROHM_OFF_HOOK                = 0x0001,
    LT_ROHM_RES_LOOP                = 0x0002,
    LT_ROHM_OUT_OF_RANGE_LOOP       = 0x0004,
    LT_ROHM_MSRMNT_STATUS           = LT_TEST_MEASUREMENT_ERROR,
    LT_ROHM_BIT_ENUM_SIZE           = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtRohBitType;

typedef uint16 LtRohMaskType;

typedef struct {
    LtRohMaskType fltMask;
    LtImpedanceType rLoop1;
    LtImpedanceType rLoop2;
    LtMsrmntStatusMaskType measStatus;
} LtRohResultType;

#endif /* !_LT_API_TEST_ROH_H_ */
