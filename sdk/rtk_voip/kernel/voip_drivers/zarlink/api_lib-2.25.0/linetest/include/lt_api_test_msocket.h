/** \lt_api_test_msocket.h
 * lt_api_test_msocket.h
 *
 * This file defines the necessary interface aspects for the master socket
 * test.
 *
 * This file assumes a certain include order to compile.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 6423 $
 * $LastChangedDate: 2010-02-12 17:01:34 -0600 (Fri, 12 Feb 2010) $
 */

#ifndef _LT_API_TEST_MSOCKET_H_
#define _LT_API_TEST_MSOCKET_H_

/*******************************************************************************
 * Definitions for test inputs                                                 *
 ******************************************************************************/
typedef enum {
    LT_MSOCKET_TYPE_1      = 0,
    LT_MSOCKET_TYPE_2      = 1,
    LT_MSOCKET_ENUM_SIZE   = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtMSocketType;


typedef struct {
    LtMSocketType   mSocket;           /* Master socket type */
} LtMSocketInputType;

/* Default test input */
#define LT_MSOCKET_DFLTI_TEST_TYPE (LT_MSOCKET_TYPE_1)

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
} LtMSocketCriteriaType;

/*******************************************************************************
 * Definitions for test results                                                *
 ******************************************************************************/

typedef enum {
    LT_MSKT_TEST_PASSED     = LT_TEST_PASSED,
    LT_MSKT_NOT_PRESENT     = 0x0001, /* the master socket is not present */
    LT_MSKT_TWO_PARALLEL    = 0x0002, /* two master sockets installed in parallel */
    LT_MSKT_TWO_OPPOSITE    = 0x0004, /* two opposing master sockets may be installed */
    LT_MSKT_TWO_REVERSE     = 0x0010, /* two master sockets installed in reverse */
    LT_MSKT_REVERSE         = 0x0020, /* the master socket is installed reversed */
    LT_MSKT_FAULT           = 0x0040, /* the test is unable to determine due to line fault */
    LT_MSKT_MSRMNT_STATUS    = LT_TEST_MEASUREMENT_ERROR,
    LT_MSKT_ENUM_SIZE        = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtMSocketBitType;

typedef uint16 LtMSocketMaskType;

typedef struct {
    LtMSocketMaskType fltMask;
    LtMsrmntStatusMaskType measStatus;
} LtMSocketResultType;

#endif /* !_LT_API_TEST_MSOCKET_H_ */
