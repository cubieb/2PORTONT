/** \lt_api_ringers.h
 * lt_api_ringers.h
 *
 * This file defines the necessary interface aspects for the ringer equivalence
 * number (REN) measurement test.
 *
 * This file assumes a certain include order to compile.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 9115 $
 * $LastChangedDate: 2011-11-15 15:25:17 -0600 (Tue, 15 Nov 2011) $
 */

#ifndef _LT_API_RINGERS_H_
#define _LT_API_RINGERS_H_

/*******************************************************************************
 * Definitions for test inputs                                                 *
 ******************************************************************************/
/* REN Measurement test type */
typedef enum {
    LT_RINGER_REGULAR_PHNE_TEST,            /* Perform REN test on regular phone */
    LT_RINGER_ELECTRONIC_PHNE_TEST,         /* Perform REN test on electronic phone */
    LT_RINGER_REGULAR_PHNE_TEST_3_ELE,      /* Perform 3 ELE REN test on regular phone */
    LT_RINGER_ELECTRONIC_PHNE_TEST_3_ELE,   /* Perform 3 ELE REN test on electronic phone */
    LT_RINGER_TEST_NUM_TYPES,
    LT_RINGER_BIT_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtRingerTestType;

typedef struct {
    LtRingerTestType ringerTestType;
    LtVoltageType vRingerTest; /* Voltage to use for the REN test; Applicable when
                                * performing any ren test.*/
    LtFreqType freq;           /* Frequency to use for the REN test;Applicable
                                * only when performing Regular phone REN test */
    int32 renFactor;           /* REN factor adjustment; Applicable when
                                * performing any ren test.*/
} LtRingerInputType;

/* Default test inputs these only apply to LT_RINGER_REGULAR_PHNE_TEST*/
#define LT_RINGERS_DFLTI_TEST_TYPE (LT_RINGER_REGULAR_PHNE_TEST)
#define LT_RINGERS_DFLTI_TEST_VOLTAGE (16970) /* in mVpk*/
#define LT_RINGERS_DFLTI_TEST_FREQUENCY 20000 /* 20 Hz */
#define LT_RINGERS_DFLTI_TEST_RENFACTOR 70000 /* 7 kOhms */

/*******************************************************************************
 * Definitions for test criteria                                               *
 ******************************************************************************/
/* The following constants could be used as default criteria inputs */

/* Default lower criteria limit for Ringer test (in milli REN) */
#define LT_RINGERS_CRT_REN_HIGH    (5 * 1000)

/* Default upper criteria limit for Ringer test (in milli REN) */
#define LT_RINGERS_CRT_REN_LOW     (175)

typedef struct {
    /* Test limits for REN test */
    LtRenType renHigh;

    /* Test limits for REN test */
    LtRenType renLow;

} LtRingersCriteriaType;


/*******************************************************************************
 * Definitions for test results                                                *
 ******************************************************************************/
typedef enum {
    LT_RNGM_TEST_PASSED     = LT_TEST_PASSED,
    LT_RNGM_REN_LOW         = 0x0001,
    LT_RNGM_REN_HIGH        = 0x0002,
    LT_RNGM_MSRMNT_STATUS   = LT_TEST_MEASUREMENT_ERROR,
    LT_RNGM_BIT_ENUM_SIZE   = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtRingersBitType;

typedef uint16 LtRingersMaskType;

typedef struct {
    LtRingersMaskType fltMask;
    LtMsrmntStatusMaskType measStatus;
    LtRenType ren;      /* REN from the Tip to Ring */
    LtRenType rentg;    /* REN from the Tip lead to ground */
    LtRenType renrg;    /* REN from the Ring lead to ground */
    LtRingerTestType ringerTestType;
} LtRingersResultType;


#endif /* !_LT_API_RINGERS_H_ */
