/** \lt_api_test_ringing_st.h
 * lt_api_test_ringing_st.h
 *
 * This file defines the necessary interface aspects for the ringing self test.
 *
 * This file assumes a certain include order to compile.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 9115 $
 * $LastChangedDate: 2011-11-15 15:25:17 -0600 (Tue, 15 Nov 2011) $
 */

#ifndef _LT_API_TEST_RINGING_ST_H_
#define _LT_API_TEST_RINGING_ST_H_

/*******************************************************************************
 * Definitions for test inputs                                                 *
 ******************************************************************************/
/*
 * Ringing Self Test Input
 */
typedef struct {
    /* Test voltage (in RMS volts) to be applied while computing the
     * AC test load */
    LtVoltageType vRinging;

    /* Frequency of the test signal to be applied while computing the
     * AC test load */
    LtFreqType freq;

    /* Ringing profile to be used (if any) while performing the ring trip test */
    VpProfilePtrType pRingProfileForTest;

    /* DC profile to be used (if any) while performing the ring trip test */
    VpProfilePtrType pDcProfileForTest;

} LtRingingSTInputType;

/* Default test inputs */
#define LT_RINGING_ST_DFLTI_VRING  (12 * 1000) /* In mV RMS*/
#define LT_RINGING_ST_DFLTI_FREQ   (20 * 1000) /* In mHz*/
#define LT_RINGING_ST_DFLTI_PROFILE (VP_PTABLE_NULL)

/*******************************************************************************
 * Definitions for test criteria                                               *
 ******************************************************************************/
typedef struct {
    /* Open circuit ringing voltage measurment tolerance */
    LtPercentType openVoltageErr;

    /* Frequency measurement tolerance */
    LtPercentType freqErr;

    /* Test load resistance measurement tolerance */
    LtPercentType rLoadErr;

} LtRingingSTCriteriaType;

/* Default test criteria */
#define LT_RINGING_ST_DFLTC_VRING  (10 * 1000) /* In milli percent*/
#define LT_RINGING_ST_DFLTC_FREQ   (10 * 1000) /* In milli percent*/
#define LT_RINGING_ST_DFLTC_RLOAD  (15 * 1000) /* In milli percent*/
#define LT_RINGING_ST_DFLTC_IRLOAD (10 * 1000) /* In milli percent*/

/*******************************************************************************
 * Definitions for test results                                                *
 ******************************************************************************/
typedef enum {
    LT_RINGING_STM_TEST_PASSED = LT_TEST_PASSED,
    LT_RINGING_STM_OPENV_OC = 0x0001,   /* Measured ringing open circuit
                                         * voltage is outside the specified
                                         * criteria */
    LT_RINGING_STM_FREQ_OC = 0x0002,    /* Measured ringing frequency is
                                         * outside the specified criteria */
    LT_RINGING_STM_ACRLOAD_OC = 0x0004, /* Measured AC test load impedance is
                                         * outside the specified criteria */
    LT_RINGING_STM_NO_RINGTRIP = 0x0008,/* Test failed because no ring trip
                                         * was detected */
    LT_RINGING_STM_IRLOAD_OC = 0x0010,  /* Measured AC test load current is
                                         * outside of the defined criteria */
    LT_RINGING_STM_OFF_HOOK = 0x0020,   /* Off-hook detected no measurements made*/

    LT_RINGING_STM_MSRMNT_STATUS    = LT_TEST_MEASUREMENT_ERROR,
    LT_RINGING_STM_BIT_ENUM_SIZE    = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtRingingSTBitType;

typedef uint16 LtRingingSTMaskType;

typedef struct {
    LtRingingSTMaskType fltMask;

    /* Measured open circuit ringing voltage */
    LtVoltageType openCktRingVol;

    /* Measured ringing frequency */
    LtFreqType freq;

    /* Indicates the computed AC test load resistance that was applied during
     * the test. */
    LtImpedanceType acRload;

    /* Measured current through the rLoad (RMS) */
    LtCurrentType iRload;

    LtMsrmntStatusMaskType measStatus;
} LtRingingSTResultType;

#endif /* !_LT_API_TEST_RINGING_ST_H_ */
