/** \lt_api_test_rd_loop_cond.h
 * lt_api_test_rd_loop_cond.h
 *
 * This file defines the necessary interface aspects for the Read Loop Condition test.
 *
 * This file assumes a certain include order to compile.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 5264 $
 * $LastChangedDate: 2009-07-06 10:22:54 -0500 (Mon, 06 Jul 2009) $
 */

#ifndef _LT_API_TEST_RD_LOOP_COND_H_
#define _LT_API_TEST_RD_LOOP_COND_H_

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
} LtRdLoopCondInputType;

#define LT_RD_LOOP_COND_DFLTI_LINE_STATE (VP_NUM_LINE_STATES)

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

#define LT_RD_LOOP_COND_CRT_IMP_PERCENT  (15000) /* 15.0 % */

typedef struct {
    /* Test limit for Test Switch Measurement*/
    LtPercentType rloadErr;
} LtRdLoopCondCriteriaType;

/*******************************************************************************
 * Definitions for test results                                                *
 ******************************************************************************/
typedef enum {
    LT_RD_LOOP_COND_TEST_PASSED         = LT_TEST_PASSED,
    LT_RD_LOOP_COND_TEST_FAILED         = 0x0001,
    LT_RD_LOOP_COND_MSRMNT_STATUS       = LT_TEST_MEASUREMENT_ERROR,
    LT_RD_LOOP_COND_ENUM_SIZE           = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtRdLoopCondBitType;

typedef uint16 LtRdLoopCondMaskType;

typedef struct {
    LtRdLoopCondMaskType fltMask;
    LtMsrmntStatusMaskType measStatus;
    LtImpedanceType rloop;          /* Measured loop resistance */
    LtCurrentType imt;              /* Sensed metalic current */
    LtCurrentType ilg;              /* Sensed longitudinal current */
    LtVoltageType vab;              /* Sensed voltage on AB (tip/ring) leads */
    LtVoltageType vag;              /* Sensed voltage on AG (ring/ ground) leads */
    LtVoltageType vbg;              /* Sensed voltage on BG (tip/ground) leads */
    LtVoltageType vbat1;            /* Battery 1 measured voltage */
    LtVoltageType vbat2;            /* Battery 2 measured voltage */
    LtVoltageType vbat3;            /* Battery 3 measured voltage */
} LtRdLoopCondResultType;



#endif /* !_LT_API_TEST_RD_LOOP_COND_H_ */
