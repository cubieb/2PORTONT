/** \lt_api_test_line_v.h
 * lt_api_test_line_v.h
 *
 * This file defines the necessary interface aspects for the Line voltage
 * measurement test.
 *
 * This file assumes a certain include order to compile.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 10411 $
 * $LastChangedDate: 2012-08-21 11:13:20 -0500 (Tue, 21 Aug 2012) $
 */

#ifndef _LT_API_TEST_LINE_V_H_
#define _LT_API_TEST_LINE_V_H_

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
    bool collapseFeed;

    /* Dummy place holder */
    char dummy;
} LtLineVInputType;


/*******************************************************************************
 * Definitions for test criteria                                               *
 ******************************************************************************/
/* The following constants could be used as default criteria inputs */

/* Default DC HEMF threshold beyond which DC HEFM fault should be declared.
 * Specified in units of the LT-API. Note that this an absolute value */
#define LT_LINEV_CRT_DC_HEMF    (135 * 1000)

/* Default AC HEMF threshold beyond which AC HEFM fault should be declared.
 * Specified in units of the LT-API. Note that this an RMS value */
#define LT_LINEV_CRT_AC_HEMF    (50 * 1000)

/* Default DC FEMF threshold beyond which DC FEFM fault should be declared.
 * Specified in units of the LT-API. Note that this an absolute value */
#define LT_LINEV_CRT_DC_FEMF    (6 * 1000)

/* Default AC FEMF threshold beyond which AC FEFM fault should be declared.
 * Specified in units of the LT-API. Note that this an RMS value */
#define LT_LINEV_CRT_AC_FEMF    (10 * 1000)

typedef struct {
    /* Foreign DC Voltage above which DC HEMF voltage fault needs to be
     * detected. Specified as an absolute value. */
    LtVoltageType dcHemf;

    /* Foreign AC Voltage above which AC HEMF voltage fault needs to be
     * detected. Specified as RMS quantity. */
    LtVoltageType acHemf;

    /* Foreign DC Voltage above which DC FEMF voltage fault needs to be
     * detected. Specified as an absolute value. */
    LtVoltageType dcFemf;

    /* AC Voltage above which Foreign AC FEMF voltage fault needs to be
     * detected. Specified as RMS quantity. */
    LtVoltageType acFemf;

} LtLineVCriteriaType;


/*******************************************************************************
 * Definitions for test results                                                *
 ******************************************************************************/
typedef enum {
    LT_LVM_TEST_PASSED   = LT_TEST_PASSED,
    LT_LVM_HEMF_DC_TIP     = 0x0001,
    LT_LVM_HEMF_DC_RING    = 0x0002,
    LT_LVM_HEMF_DC_DIFF    = 0x0004,
    LT_LVM_HEMF_AC_TIP     = 0x0010,
    LT_LVM_HEMF_AC_RING    = 0x0020,
    LT_LVM_HEMF_AC_DIFF    = 0x0040,
    LT_LVM_FEMF_DC_TIP     = 0x0100,
    LT_LVM_FEMF_DC_RING    = 0x0200,
    LT_LVM_FEMF_DC_DIFF    = 0x0400,
    LT_LVM_FEMF_AC_TIP     = 0x1000,
    LT_LVM_FEMF_AC_RING    = 0x2000,
    LT_LVM_FEMF_AC_DIFF    = 0x4000,
    LT_LVM_MSRMNT_STATUS   = LT_TEST_MEASUREMENT_ERROR,
    LT_LVM_ENUM_SIZE       = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtLineVBitType;

typedef uint16 LtLineVMaskType;

typedef struct {
    LtLineVMaskType fltMask; /* Can be any or'ed comb of this type */
    LtMsrmntStatusMaskType measStatus;
    LtVoltageType vAcTip;
    LtVoltageType vAcRing;
    LtVoltageType vAcDiff;
    LtVoltageType vDcTip;
    LtVoltageType vDcRing;
    LtVoltageType vDcDiff;
} LtLineVResultType;

#endif /* !_LT_API_TEST_LINE_V_H_ */
