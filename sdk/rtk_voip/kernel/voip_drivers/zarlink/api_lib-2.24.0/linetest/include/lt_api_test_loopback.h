/** \lt_api_test_loopback.h
 * lt_api_test_loopback.h
 *
 * This file defines the necessary interface aspects for the Loopback tests.
 *
 * This file assumes a certain include order to compile.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 9115 $
 * $LastChangedDate: 2011-11-15 15:25:17 -0600 (Tue, 15 Nov 2011) $
 */

#ifndef _LT_API_TEST_LOOPBACK_H_
#define _LT_API_TEST_LOOPBACK_H_

/*******************************************************************************
 * Definitions for test inputs                                                 *
 ******************************************************************************/
/* Loopback test type */
typedef enum {

    /* This type of test performs full digital loop back. The looback happens
     * at the output of CODEC just prior to 4 wire to 2 wire conversion. When
     * making use of this loopback mode, various filter coefficients in
     * the device are configured such that the spectral contents of the
     * signal experience a uinty gain through the loopback. Note that one
     * CAN NOT expect to see the same bit pattern in the loopback signal.
     */
    LT_LOOPBACK_CODEC,              /* Digital loopback */

    /* This type of test performs a loopback at the 2 wire interface with
     * subscriber disconnected from the line card and with a test load applied
     * across tip and ring. It should be noted that filter coefficients are
     * not changed while running this test and one CAN NOT expect to see the
     * same bit pattern in the loopback signal.
     */
    LT_LOOPBACK_ANALOG,             /* Analog loopback */

    /* New loopback options */
    LT_LOOPBACK_BFILTER,
    LT_LOOPBACK_TIMESLOT,

    LT_LOOPBACK_TEST_NUM_TYPES,

    LT_LOOPBACK_ENUM_SIZE   = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtLoopbakTestType;

typedef struct {

    LtLoopbakTestType loopbackType;

    /* The following two arguments are necessary only for
     * CSLAC devices */
    /* Loopback test time in micro seconds */
    LtTimeType waitTime;

    /* Loopback test time in micro seconds */
    LtTimeType loopbackTime;

} LtLoopbackInputType;

/* Maximum loopback test time */
#define LT_LOOPBACK_MAX_TIME    (8191875) /* Time in micro seconds */

/* Default test inputs that are used if they are not specified */
#define LT_LOOPBACK_DFLT_TEST_TYPE  (LT_LOOPBACK_CODEC)
#define LT_LOOPBACK_DFLT_WAIT_TIME  (0) /* Necessary wait time before collecting
                                         * the samples. */
#define LT_LOOPBACK_DFLT_TEST_TIME  (1024 * 125) /* Time necessary to collect
                                      * 1024 samples at 8KHz sampling rate */

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
} LtLoopbackCriteriaType;

/*******************************************************************************
 * Definitions for test results                                                *
 ******************************************************************************/
typedef enum {
    LT_LOOPBACK_TEST_PASSED         = LT_TEST_PASSED,
    LT_LOOPBACK_TEST_FAILED         = 0x0001,
    LT_LOOPBACK_MSRMNT_STATUS       = LT_TEST_MEASUREMENT_ERROR,
    LT_LOOPBACK_BIT_ENUM_SIZE       = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtLoopbackBitType;

typedef uint16 LtLoopbackMaskType;

typedef struct {
    LtLoopbackMaskType fltMask;
    LtMsrmntStatusMaskType measStatus;

    /* The following member variable has meaningful information only when
     * the LT-API is used along with VoicePort Le880 device family. The
     * following variable contains a pointer that was passed to the Line Test
     * API by the system service layers.
     */
    void *pApplicationInfo;

} LtLoopbackResultType;


#endif /* !_LT_API_TEST_LOOPBACK_H_ */
