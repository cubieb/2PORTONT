/** \lt_api_test_slac_cal.h
 * lt_api_test_slac_cal.h
 *
 * This file defines the necessary interface aspects for the SLAC calibration.
 *
 * This file assumes a certain include order to compile.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 9115 $
 * $LastChangedDate: 2011-11-15 15:25:17 -0600 (Tue, 15 Nov 2011) $
 */

#ifndef _LT_API_TEST_SLAC_CAL_H_
#define _LT_API_TEST_SLAC_CAL_H_

/*******************************************************************************
 * Definitions for test inputs                                                 *
 ******************************************************************************/
/* None */

/*******************************************************************************
 * Definitions for test criteria                                               *
 ******************************************************************************/
/* None */

/*******************************************************************************
 * Definitions for test results                                                *
 ******************************************************************************/
typedef enum {
    LT_SLAC_CALM_PASSED         = LT_TEST_PASSED,
    LT_SLAC_CALM_FAILED         = 0x0001,
    LT_SLAC_CALM_MSRMNT_STATUS  = LT_TEST_MEASUREMENT_ERROR,
    LT_SLAC_CALM_BIT_ENUM_SIZE  = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtSlacCalibrateBitType;

typedef uint16 LtCalibrateMaskType;

typedef struct {
    LtCalibrateMaskType fltMask;
    LtMsrmntStatusMaskType measStatus;
} LtCalibrateResultType;

#endif /* !_LT_API_TEST_SLAC_CAL_H_ */
