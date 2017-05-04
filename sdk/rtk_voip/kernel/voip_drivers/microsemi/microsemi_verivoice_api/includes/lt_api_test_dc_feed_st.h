/** \lt_api_test_dc_feed_st.h
 * lt_api_test_dc_feed_st.h
 *
 * This file defines the necessary interface aspects for the DC feed Self test.
 *
 * This file assumes a certain include order to compile.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 9115 $
 * $LastChangedDate: 2011-11-15 15:25:17 -0600 (Tue, 15 Nov 2011) $
 */

#ifndef _LT_API_TEST_DC_FEED_ST_H_
#define _LT_API_TEST_DC_FEED_ST_H_

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
    /* The VP-API test line state that will be enabled while computing the
     * Rload */
    VpLineStateType lineState;
} LtDcFeedSTInputType;

/* Default test inputs */
#define LT_DC_FEED_ST_DFLTI_LINE_STATE (VP_NUM_LINE_STATES)

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

#define LT_DC_FEED_ST_CRT_IMP_PERCENT  (15000) /* 15.0 % */
#define LT_DC_FEED_ST_CRT_ILA_PERCENT  (10000) /* 10.0 % */

typedef struct {
    /* Test limit for Test Switch Measurement*/
    LtPercentType rloadErr;
} LtDcFeedSTCriteriaType;

/*******************************************************************************
 * Definitions for test results                                                *
 ******************************************************************************/
typedef enum {
    LT_DC_FEED_ST_TEST_PASSED         = LT_TEST_PASSED,
    LT_DC_FEED_ST_TEST_FAILED         = 0x0001,
    LT_DC_FEED_ST_MSRMNT_STATUS       = LT_TEST_MEASUREMENT_ERROR,
    LT_DC_FEED_ST_ENUM_SIZE           = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtDcFeedSTBitType;

typedef uint16 LtDcFeedSTMaskType;

typedef struct {
    LtDcFeedSTMaskType fltMask;
    LtMsrmntStatusMaskType measStatus;

    /* Measured test load */
    LtImpedanceType rTestLoad;

    /* Measured voltage across the test load */
    LtVoltageType vTestLoad;

    /* Measured current through the test load */
    LtCurrentType iTestLoad;

} LtDcFeedSTResultType;


#endif /* !_LT_API_TEST_DC_FEED_ST_H_ */
