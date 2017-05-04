/** \lt_api_test_rd_bat_cond.h
 * lt_api_test_rd_bat_cond.h
 *
 * This file defines the necessary interface aspects for the read battery
 * conditions test.
 *
 * This file assumes a certain include order to compile.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 9115 $
 * $LastChangedDate: 2011-11-15 15:25:17 -0600 (Tue, 15 Nov 2011) $
 */

#ifndef _LT_API_TEST_RD_BAT_COND_H_
#define _LT_API_TEST_RD_BAT_COND_H_

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
} LtReadBatCondInputType;

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
} LtReadBatCondCriteriaType;

/*******************************************************************************
 * Definitions for test results                                                *
 ******************************************************************************/
typedef enum {
    LT_RD_BAT_CONDM_TEST_PASSED     = LT_TEST_PASSED,
    LT_RD_BAT_CONDM_MSRMNT_STATUS   = LT_TEST_MEASUREMENT_ERROR,
    LT_RD_BAT_CONDM_ENUM_SIZE       = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtReadBatCondBitType;

typedef uint16 LtReadBatCondMaskType;

typedef struct {
    LtReadBatCondMaskType fltMask;
    LtMsrmntStatusMaskType measStatus;

    /* Measured battery voltages */
    LtVoltageType bat1;
    LtVoltageType bat2;
    LtVoltageType bat3;

} LtReadBatCondResultType;

#endif /* !_LT_API_TEST_RD_BAT_COND_H_ */
