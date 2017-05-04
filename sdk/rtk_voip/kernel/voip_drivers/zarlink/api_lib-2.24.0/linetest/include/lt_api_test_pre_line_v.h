/** \lt_api_test_pre_line_v.h
 * lt_api_test_pre_line_v.h
 *
 * This file defines the necessary interface aspects for the Line voltage
 * measurement test.
 *
 * This file assumes a certain include order to compile.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 5264 $
 * $LastChangedDate: 2009-07-06 10:22:54 -0500 (Mon, 06 Jul 2009) $
 */

#ifndef _LT_API_TEST_PRE_LINE_V_H_
#define _LT_API_TEST_PRE_LINE_V_H_

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
} LtPreLineVInputType;


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
} LtPreLineVCriteriaType;


/*******************************************************************************
 * Definitions for test results                                                *
 ******************************************************************************/
typedef enum {
    LT_PLVM_TEST_PASSED     = LT_TEST_PASSED,
    LT_PLVM_FAILED          = 0x0001,
    LT_PLVM_MSRMNT_STATUS   = LT_TEST_MEASUREMENT_ERROR,
    LT_PLVM_ENUM_SIZE       = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtPreLineVBitType;

typedef uint16 LtPreLineVMaskType;

typedef struct {
    LtPreLineVMaskType      fltMask;
    LtMsrmntStatusMaskType  measStatus;
} LtPreLineVResultType;

#endif /* !_LT_API_TEST_PRE_LINE_V_H_ */
