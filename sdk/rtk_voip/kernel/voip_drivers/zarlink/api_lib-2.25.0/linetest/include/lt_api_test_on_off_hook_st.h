/** \lt_api_test_on_off_hook_st.h
 * lt_api_test_on_off_hook_st.h
 *
 * This file defines the necessary interface aspects for the on/off hook
 * self test.
 *
 * This file assumes a certain include order to compile.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 9115 $
 * $LastChangedDate: 2011-11-15 15:25:17 -0600 (Tue, 15 Nov 2011) $
 */

#ifndef _LT_API_TEST_ON_OFF_HOOK_ST_H_
#define _LT_API_TEST_ON_OFF_HOOK_ST_H_

/*******************************************************************************
 * Definitions for test inputs                                                 *
 ******************************************************************************/
/* Test inputs */
typedef struct {
    /* If set to TRUE override off-hook condition and carry out the test else
     * test is aborted (if off-hook is detected).
     * This flag MUST be set to FALSE for line termination types that do
     * not support disconnecting the line circuit from the loop.
     */
    bool overrideOffHook;
} LtOnOffHookSTInputType;

/* Default test inputs */
#define LT_ON_OFF_HOOK_ST_DFLT_OVRRD_FLG  (FALSE)

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
} LtOnOffHookSTCriteriaType;

/*******************************************************************************
 * Definitions for test results                                                *
 ******************************************************************************/
typedef enum {
    LT_ON_OFF_HOOK_STM_TEST_PASSED      = LT_TEST_PASSED,
    LT_ON_OFF_HOOK_STM_TEST_ABORTED     = 0x0001,
    LT_ON_OFF_HOOK_STM_TEST_HW_FAULT    = 0x0002,
    LT_ON_OFF_HOOK_STM_MSRMNT_STATUS    = LT_TEST_MEASUREMENT_ERROR,
    LT_ON_OFF_HOOK_STM_BIT_ENUM_SIZE    = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtOnOffHookSTBitType;

typedef uint16 LtOnOffHookSTMaskType;

typedef struct {
    LtOnOffHookSTMaskType fltMask;
    LtMsrmntStatusMaskType measStatus;
} LtOnOffHookSTResultType;

#endif /* !_LT_API_TEST_ON_OFF_HOOK_ST_H_ */
