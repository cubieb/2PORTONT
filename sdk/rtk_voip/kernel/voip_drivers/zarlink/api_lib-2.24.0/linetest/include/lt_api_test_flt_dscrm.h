/*
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 5264 $
 * $LastChangedDate: 2009-07-06 10:22:54 -0500 (Mon, 06 Jul 2009) $
 */
#ifndef _LT_API_TEST_FLT_DSCRM_V_H_
#define _LT_API_TEST_FLT_DSCRM_V_H_

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
} LtFltDscrmInputType;


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
} LtFltDscrmCriteriaType;


/*******************************************************************************
 * Definitions for test results                                                *
 ******************************************************************************/
typedef enum {
    LT_FDM_TEST_PASSED          = LT_TEST_PASSED,
    LT_FDM_TIP_RING_SHORT       = 0x0001,
    LT_FDM_TIP_SHORT            = 0x0002,
    LT_FDM_RING_SHORT           = 0x0004,
    LT_FDM_OFF_HOOK             = 0x0008,
    LT_FDM_UNKNOWN_LINE_FAULT   = 0x0010,
    LT_FDM_TAC_OPEN             = 0x0020,
    LT_FDM_RAC_OPEN             = 0x0040,
    LT_FDM_TDC_OPEN             = 0x0080,
    LT_FDM_RDC_OPEN             = 0x0100,
    LT_FDM_TIPD_RINGD_SHORT     = 0x0200,
    LT_FDM_TIPD_SHORT           = 0x0400,
    LT_FDM_RINGD_SHORT          = 0x0800,
    LT_FDM_MSRMNT_STATUS        = LT_TEST_MEASUREMENT_ERROR,
    LT_FDM_ENUM_SIZE            = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtFltDscrmBitType;

typedef uint16 LtFltDscrmMaskType;

typedef struct {
    LtFltDscrmMaskType      fltMask;
    LtMsrmntStatusMaskType  measStatus;
} LtFltDscrmResultType;

#endif /* !_LT_API_TEST_FLT_DSCRM_V_H_ */
