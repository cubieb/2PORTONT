/** \lt_api_test_res_flt.h
 * lt_api_test_res_flt.h
 *
 * This file defines the necessary interface aspects for the resistive fault
 * test.
 *
 * This file assumes a certain include order to compile.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 9115 $
 * $LastChangedDate: 2011-11-15 15:25:17 -0600 (Tue, 15 Nov 2011) $
 */

#ifndef _LT_API_TEST_RES_FLT_H_
#define _LT_API_TEST_RES_FLT_H_

/*******************************************************************************
 * Definitions for test inputs                                                 *
 ******************************************************************************/
typedef struct {
    bool startHighGain;
} LtResFltInputType;

/*******************************************************************************
 * Definitions for test criteria                                               *
 ******************************************************************************/
/* Default lower criteria limit for Resistive fault (in 1/10th of a ohm) */
#define LT_RES_FLT_LOW_LIMIT    (150000 * 10)

typedef struct {
    /* Value of resistance below which resistive fault should be declared. */
    LtImpedanceType resFltLowLimit;
} LtResFltCriteriaType;

/*******************************************************************************
 * Definitions for test results                                                *
 ******************************************************************************/
typedef enum {
    LT_RESFM_TEST_PASSED        = LT_TEST_PASSED,
    LT_RESFM_TIP                = 0x0001,
    LT_RESFM_RING               = 0x0002,
    LT_RESFM_DIFF               = 0x0004,
    LT_RESFM_MSRMNT_STATUS      = LT_TEST_MEASUREMENT_ERROR,
    LT_RESFM_BIT_ENUM_SIZE      = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtResFltBitType;

typedef uint16 LtResFltMaskType;

typedef struct {
    LtResFltMaskType fltMask;

    LtImpedanceType rtg;    /* Resistance from the Tip lead to ground */
    LtImpedanceType rrg;    /* Resistance from the Ring lead to ground */
    LtImpedanceType rtr;    /* Resistance between Tip and Ring leads */

    /* The following resistance is measured only if a fault (or low resistance)
     * exists between the Tip and Ring leads */
    LtImpedanceType rGnd;   /* Resistance to ground */

    LtMsrmntStatusMaskType measStatus;
} LtResFltResultType;

#endif /* !_LT_API_TEST_RES_FLT_H_ */
