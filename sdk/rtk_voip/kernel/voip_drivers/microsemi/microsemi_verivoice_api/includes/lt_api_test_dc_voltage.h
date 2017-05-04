/** \lt_api_test_dc_voltage.h
 * lt_api_test_dc_voltage.h
 *
 * This file defines the necessary interface aspects for the DC Voltage test.
 *
 * This file assumes a certain include order to compile.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 9115 $
 * $LastChangedDate: 2011-11-15 15:25:17 -0600 (Tue, 15 Nov 2011) $
 */

#ifndef _LT_API_TEST_DC_VOLTAGE_H_
#define _LT_API_TEST_DC_VOLTAGE_H_

/*******************************************************************************
 * Definitions for test inputs                                                 *
 ******************************************************************************/
typedef struct {

    /* Test voltage swing that needs to be applied */
    LtVoltageType testVoltage;

} LtDcVoltageInputType;

/* Default test voltage */
#define LT_DC_VOLTAGE_DFLT_SIGNAL  (56 * 1000) /* In mV */

/*******************************************************************************
 * Definitions for test criteria                                               *
 ******************************************************************************/
typedef struct {

    /* Voltage measurment tolerance */
    LtPercentType voltageErr;

} LtDcVoltageCriteriaType;

/* Test criteria defaults */
#define LT_DC_VOLTAGE_DFLTC_VOLTAGE_TOL (10 * 1000) /* in percent */
#define LT_DC_VOLTAGE_DFLTC_VOLTAGE_OFF (3600)      /* in mV */

/*******************************************************************************
 * Definitions for test results                                                *
 ******************************************************************************/
typedef enum {
    LT_DC_VOLTAGE_TEST_PASSED         = LT_TEST_PASSED,
    LT_DC_VOLTAGE_TEST_FAILED         = 0x0001,
    LT_DC_VOLTAGE_MSRMNT_STATUS       = LT_TEST_MEASUREMENT_ERROR,
    LT_DC_VOLTAGE_ENUM_SIZE           = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtDcVoltageBitType;

typedef uint16 LtDcVoltageMaskType;

typedef struct {
    LtDcVoltageMaskType fltMask;
    LtMsrmntStatusMaskType measStatus;

    /* Measured voltages */
    LtVoltageType measuredVoltage1; /* When +testVoltage is applied */
    LtVoltageType measuredVoltage2; /* When -testVoltage is applied */

} LtDcVoltageResultType;


#endif /* !_LT_API_TEST_DC_VOLTAGE_H_ */
