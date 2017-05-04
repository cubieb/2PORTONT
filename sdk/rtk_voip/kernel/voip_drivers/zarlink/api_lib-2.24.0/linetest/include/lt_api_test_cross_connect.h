/** \lt_api_test_cross_connect.h
 * lt_api_test_cross_connect.h
 *
 * This file defines the necessary interface aspects for the Cross Connect Detection
 * test.
 *
 * This file assumes a certain include order to compile.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 6423 $
 * $LastChangedDate: 2010-02-12 17:01:34 -0600 (Fri, 12 Feb 2010) $
 */

#ifndef _LT_API_TEST_XCONNECT_H_
#define _LT_API_TEST_XCONNECT_H_

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
} LtXConnectInputType;


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
} LtXConnectCriteriaType;



/*******************************************************************************
 * Definitions for test results                                                *
 ******************************************************************************/ 
typedef enum {
    LT_XCON_TEST_PASSED         = LT_TEST_PASSED,
    LT_XCON_NORMAL_POLARITY     = 0x0001, /* There is a cross connect (Ring->Ring / Tip->Tip) */
    LT_XCON_REVERSE_POLARITY    = 0x0002, /* There is a cross connect (Ring->Tip / Tip->Ring) */
    LT_XCON_FAULT               = 0x0004, /* The test is unable to determine due to line fault */
    LT_XCON_MSRMNT_STATUS       = LT_TEST_MEASUREMENT_ERROR,
    LT_XCON_BIT_ENUM_SIZE       = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtXConnectBitType;


typedef uint16 LtXConnectMaskType;

typedef struct {
    LtXConnectMaskType fltMask;
    LtMsrmntStatusMaskType measStatus;
} LtXConnectResultType;

#endif /* !_LT_API_TEST_XCONNECT_H_ */
