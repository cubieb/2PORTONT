/** \lt_api_test_cap.h
 * lt_api_test_cap.h
 *
 * This file defines the necessary interface aspects for the capacitance test.
 *
 * This file assumes a certain include order to compile.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 5490 $
 * $LastChangedDate: 2009-08-21 13:59:29 -0500 (Fri, 21 Aug 2009) $
 */

#ifndef _LT_API_TEST_CAP_H_
#define _LT_API_TEST_CAP_H_

/*******************************************************************************
 * Definitions for test inputs                                                 *
 ******************************************************************************/
typedef enum {
    LT_CAP_2666_HZ      = 0,
    LT_CAP_1333_HZ      = 1,
    LT_CAP_666_HZ       = 2,
    LT_CAP_333_HZ       = 3,
    LT_CAP_280_HZ       = 4,
    LT_CAP_FREQ_ENUM_SIZE   = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtCapTestFreqType;


typedef struct {
    LtCapTestFreqType   testFreq;           /* Test signal frequency */
    LtVoltageType       testAmp;            /* Test signal amplitude (mV RMS) */
    LtCapacitanceType   tipCapCalValue;     /* Tip Cap Calibration Value */
    LtCapacitanceType   ringCapCalValue;    /* Ring Cap Calibration Value */
} LtCapInputType;

/* Default test voltage */
#define LT_CAP_DFLT_TEST_AMPL   1000                /* In mV RMS */

/* Default test frequency */
#define LT_CAP_DFLT_TEST_FREQ   LT_CAP_280_HZ

/* Internal calibration capacitance */
#define LT_CAP_MANUFACTURING_CAL    -1

/* Default Calibration Capacitances */
#define LT_CAP_DFLT_TIP_CAL     LT_CAP_MANUFACTURING_CAL
#define LT_CAP_DFLT_RING_CAL    LT_CAP_MANUFACTURING_CAL


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
} LtCapCriteriaType;

/*******************************************************************************
 * Definitions for test results                                                *
 ******************************************************************************/
typedef enum {
    LT_CAP_TEST_PASSED        = LT_TEST_PASSED,
    LT_CAP_TEST_FAILED        = 0x0001,
    LT_CAP_MSRMNT_STATUS      = LT_TEST_MEASUREMENT_ERROR,
    LT_CAP_BIT_ENUM_SIZE      = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtCapBitType;

typedef uint16 LtCapMaskType;

typedef struct {
    LtCapMaskType fltMask;

    LtCapacitanceType ctg;    /* Capacitance from the Tip lead to ground */
    LtCapacitanceType crg;    /* Capacitance from the Ring lead to ground */
    LtCapacitanceType ctr;    /* Capacitance between Tip and Ring leads */

    LtMsrmntStatusMaskType measStatus;
} LtCapResultType;

#endif /* !_LT_API_TEST_CAP_H_ */
