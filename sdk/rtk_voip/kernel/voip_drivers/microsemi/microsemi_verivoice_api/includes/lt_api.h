/** \lt_api.h
 * lt_api.h
 *
 * This file contains the top level interface for the Line Test API (LT-API).
 * Please review this file carefully. Only this file needs be included
 * by the application that intends to make use of this library.
 *
 * Copyright (c) 2012, Microsemi Corporation
 *
 * $Revision: 11457 $
 * $LastChangedDate: 2014-06-18 10:34:35 -0500 (Wed, 18 Jun 2014) $
 */

#ifndef _LT_API_H_
#define _LT_API_H_

/*******************************************************************************
 * Include VoicePath API definitions                                           *
 ******************************************************************************/
#include "vp_api.h"

/*******************************************************************************
 * Include line test package selection configurations; Please update this      *
 * include file as necessary (lt_api_pkg_sel.h).                               *
 ******************************************************************************/
#include "lt_api_pkg_sel.h"

/*******************************************************************************
 * Include Line Test measurement unit type definitions                         *
 ******************************************************************************/
#include "lt_api_units.h"

/*******************************************************************************
 * Include Debug interface                                                     *
 ******************************************************************************/
#include "lt_debug.h"

/*******************************************************************************
 * The following version number tag is updated at every release of the LT-API.
 * Since the LT-API is a common interface that supports more than one device,
 * version number change might occur when any aspect of the LT-API gets
 * released.
 ******************************************************************************/

#define LT_API_VERSION_TAG (0x010F00)


/* Frequently used test return values */
#define LT_TEST_PASSED    0x0000

/* Mask that would be set when measurement errors are observed */
#define LT_TEST_MEASUREMENT_ERROR 0x8000

/*
 * Test type definition: Specifies the type of test that needs to be performed.
 */
typedef enum {
    LT_TID_RSVD1,           /* Reserved TID */
    LT_TID_LINE_V,          /* Line Voltage Test */
    LT_TID_ROH,             /* Receiver Off-Hook indication */
    LT_TID_RINGERS,         /* Ringers test per FCC Part 68 REN def.*/
    LT_TID_RES_FLT,         /* Resistive Fault */
    LT_TID_MSOCKET,         /* Master Socket detection */
    LT_TID_XCONNECT,        /* Cross Connect detection */
    LT_TID_CAP,             /* Capacitance measurement test */
    LT_TID_ALL_GR_909,      /* All GR-909 fault tests in predefined order*/
    LT_TID_LOOPBACK,        /* Loopback test */
    LT_TID_DC_FEED_ST,      /* DC Feed Self Test */
    LT_TID_RD_LOOP_COND,    /* Read Loop Condition */
    LT_TID_DC_VOLTAGE,      /* DC VOLTAGE Test */
    LT_TID_RINGING_ST,      /* Ringing Self Test */
    LT_TID_ON_OFF_HOOK_ST,  /* On/Off hook Self Test */
    LT_TID_RD_BAT_COND,     /* Read battery conditions */
    LT_TID_PRE_LINE_V,      /* Pre Line Voltage Test */
    LT_TID_FLT_DSCRM,       /* Fault Discrimination Test */
    LT_NUM_TID_CODES,
    LT_NUM_TID_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE, /* Portability Req.*/
    LT_NUM_TID_ENUM_RSVD = FORCE_SIGNED_ENUM
} LtTestIdType;

/*
 * LT-API function interface return type
 */
typedef enum {
    LT_STATUS_RUNNING,                /* Indicates test is still running */
    LT_STATUS_DONE,                   /* Test is complete and test results
                                       * are available in the results
                                       * structure */
    LT_STATUS_ABORTED,                /* Test is aborted (DevNotes: This
                                       * needs, more clarification ) */
    LT_STATUS_ERROR_INVALID_ARG,      /* Test failed because of invalid
                                       * input arguments */
    LT_STATUS_ERROR_UNEXPECTED_EVENT, /* Test saw a  VP-API test category
                                       * event that it did not expect. */
    LT_STATUS_ERROR_VP_GENERAL,       /* Test library experienced a VP-API
                                       * error. The API Error information is
                                       * stored in the results structure. */
    LT_STATUS_ERROR_VP_TEST,          /* The device reported a test error.
                                       * Error information is stored in the
                                       * test results. */
    LT_STATUS_ERROR_UNKNOWN,          /* Unknown error */
    LT_STATUS_ERROR_MATH,             /* LT experienced a math error */
    LT_STATUS_ERROR_RESOURCE_NA,      /* Resource not available;
                                       * (Ex: Testin/TestOut bus) */
    LT_STATUS_TEST_NOT_SUPPORTED,     /* Test is not supported for the given
                                       * device or line termination type */
    LT_NUM_STATUS_CODES,

    LT_NUM_STATUS_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtTestStatusType;

/*
 * Test measurement status type
 */
typedef enum {
    LT_MSRMNT_STATUS_PASSED             = 0x0000,
    LT_MSRMNT_STATUS_EXCESSIVE_ILG      = 0x0001,
    LT_MSRMNT_STATUS_DEGRADED_ACCURACY  = 0x0002,

    LT_MSRMNT_STATUS_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtMsrmntStatusBitType;

typedef uint16 LtMsrmntStatusMaskType;

/*******************************************************************************
 * Include definitions used for temporary storage definitions                  *
 ******************************************************************************/
struct LtTestCtxType;   /**< forward declaration */
struct LtEventType;     /**< forward declaration */

/*******************************************************************************
 * Structures that are used for the LT and application interface               *
 ******************************************************************************/
/* The following structure *should* be filled by the application when it
 * receives a event from the VTD and the API. Where applicable, the application
 * is expected to fill results associated with the event. It should be noted
 * that all the events can be passed to the LT. However LT-API mainly consumes
 * test category events, read loop conditions complete event and signalling
 * events */
typedef struct {
    VpEventType *pVpEvent;
    void *pResult;
} LtEventType;

/* Define a generic function pointer for the event handler */
typedef LtTestStatusType
    (*TestEventHandlerFnPtrType) (
        struct LtTestCtxType *pTestCtx,
        LtEventType *pEvent);

/*******************************************************************************
 * Include individual test interface definitions; Refer the appropriate file   *
 * for more information.                                                       *
 ******************************************************************************/
#include "lt_api_test_slac_cal.h"
#include "lt_api_test_pre_line_v.h"
#include "lt_api_test_line_v.h"
#include "lt_api_test_roh.h"
#include "lt_api_test_ringers.h"
#include "lt_api_test_res_flt.h"
#include "lt_api_test_msocket.h"
#include "lt_api_test_cross_connect.h"
#include "lt_api_test_cap.h"
#include "lt_api_test_all_gr_909.h"
#include "lt_api_test_loopback.h"
#include "lt_api_test_dc_feed_st.h"
#include "lt_api_test_rd_loop_cond.h"
#include "lt_api_test_dc_voltage.h"
#include "lt_api_test_ringing_st.h"
#include "lt_api_test_on_off_hook_st.h"
#include "lt_api_test_rd_bat_cond.h"
#include "lt_api_test_flt_dscrm.h"

/*******************************************************************************
 * Include device specific interface definitions
 ******************************************************************************/
#ifdef LT_VP880_PACKAGE
  #include "lt_api_vp880.h"
#endif /* LT_VP880_PACKAGE */

#ifdef LT_VP890_PACKAGE
  #include "lt_api_vp890.h"
#endif /* LT_VP890_PACKAGE */

#ifdef LT_VP886_PACKAGE
  #include "lt_api_vp886.h"
#endif /* LT_VP886_PACKAGE*/

/*******************************************************************************
 * Line test results. The members of this structure will be filled by the
 * test library based on the test type and error encountered (if any)
 ******************************************************************************/
typedef struct {
    LtTestIdType testId;          /* Indicates the test identifier for
                                   * which the results corresponds */
    union {
        /* Test results for the test LT_TID_SLAC_CALIBRATE */
        LtCalibrateResultType slacCal;

        /* Test results for the test LT_TID_LINE_V */
        LtLineVResultType lineV;

        /* Test results for the test LT_TID_ROH */
        LtRohResultType roh;

        /* Test results for the test LT_TID_RINGERS */
        LtRingersResultType ringers;

        /* Test results for the test LT_TID_RES_FLT */
        LtResFltResultType resFlt;

        /* Test results for the test LT_TID_MSOCKET */
        LtMSocketResultType mSocket;

        /* Test results for the test LT_TID_XCONNECT */
        LtXConnectResultType xConnect;

        /* Test results for the test LT_TID_CAP */
        LtCapResultType cap;

        /* Test results for the test LT_TID_ALL_GR_909 */
        LtAllGr909ResultType allGr909;

        /* Test results for the test LT_TID_LOOPBACK */
        LtLoopbackResultType loopback;

        /* Test results for the test LT_TID_DC_FEED_ST */
        LtDcFeedSTResultType dcFeedST;

        /* Test results for the test LT_TID_RD_LOOP_COND */
        LtRdLoopCondResultType rdLoopCond;

        /* Test results for the test LT_TID_DC_VOLTAGE */
        LtDcVoltageResultType dcVoltage;

        /* Test results for the test LT_TID_RINGING_ST */
        LtRingingSTResultType ringingST;

        /* Test results for the test LT_TID_ON_OFF_HOOK_ST */
        LtOnOffHookSTResultType onOffHookST;

        /* Test results for the test LT_TID_RD_BAT_COND */
        LtReadBatCondResultType readBatCond;

        /* Test results for the test LT_TID_PRE_LINE_V */
        LtPreLineVResultType preLineV;

        /* Test criteria for the test LT_TID_FLT_DSCRM */
        LtFltDscrmResultType fltDscrm;


    } result;

    /* If the test did not complete successfully, then one of the following
     * is filled based on the error type */
    VpStatusType vpGeneralErrorCode;    /* VP-API error */
    VpTestStatusType vpTestErrorCode;   /* VTD test error */

} LtTestResultType;

/*******************************************************************************
 * Line test criteria. Specifies the criteria to determine the test outcome.
 *
 * Set the approprate pointer to NULL to run the test using default test
 * criteria.
 *                                    or
 * Set the approprate pointer to initialized instance of an appropriate
 * test criteria structure to run the run test and apply the specified criteria.
 ******************************************************************************/
typedef union {
    /* Test criteria for the test LT_TID_LINE_V */
    LtLineVCriteriaType *pLintVCrt;

    /* Test criteria for the test LT_TID_ROH */
    LtRohCriteriaType *pRohCrt;

    /* Test criteria for the test LT_TID_RINGERS */
    LtRingersCriteriaType *pRingersCrt;

    /* Test criteria for the test LT_TID_RES_FLT */
    LtResFltCriteriaType *pResFltCrt;

    /* Test criteria for the test LT_TID_MSOCKET */
    LtMSocketCriteriaType *pMSocketCrt;

    /* Test criteria for the test LT_TID_XCONNECT */
    LtXConnectCriteriaType *pXConnectCrt;

    /* Test criteria for the test LT_TID_CAP */
    LtCapCriteriaType *pCapCrt;

    /* Test criteria for the test LT_TID_ALL_GR_909 */
    LtAllGr909CriteriaType *pAllGr909Crt;

    /* Test criteria for the test LT_TID_LOOPBACK */
    LtLoopbackCriteriaType *pLoopbackCrt;

    /* Test criteria for the test LT_TID_DC_FEED_ST */
    LtDcFeedSTCriteriaType *pDcFeedSTCrt;

    /* Test criteria for the test LT_TID_RD_LOOP_COND */
    LtRdLoopCondCriteriaType *pRdLoopCondCrt;

    /* Test criteria for the test LT_TID_DC_VOLTAGE */
    LtDcVoltageCriteriaType *pDcVoltageCrt;

    /* Test criteria for the test LT_TID_RINGING_ST */
    LtRingingSTCriteriaType *pRingingSTCrt;

    /* Test criteria for the test LT_TID_ON_OFF_HOOK_ST */
    LtOnOffHookSTCriteriaType *pOnOffHookSTCrt;

    /* Test criteria for the test LT_TID_RD_BAT_COND */
    LtReadBatCondCriteriaType *pReadBatCondCrt;

    /* Test criteria for the test LT_TID_PRE_LINE_V */
    LtPreLineVCriteriaType *pPreLineVCrt;

    /* Test criteria for the test LT_TID_FLT_DSCRM */
    LtFltDscrmCriteriaType *pFltDscrmCrt;

    /* DevNotes: More here .... */

} LtTestCriteriaType;

/*******************************************************************************
 * Test Inputs. Specifies how the test should be run.
 *
 * Set the approprate pointer to NULL run the test using default test inputs.
 *                                  or
 * Set the approprate pointer to initialized instance of an appropriate
 * test inputs structure to make use of the specified test inputs.
 ******************************************************************************/
typedef union {
    /* Inputs for test LT_TID_LINE_V */
    LtLineVInputType *pLineVInp;

    /* Inputs for test LT_TID_ROH */
    LtRohInputType *pRohInp;

    /* Inputs for test LT_TID_RINGERS */
    LtRingerInputType *pRingersInp;

    /* Inputs for test LT_TID_RES_FLT */
    LtResFltInputType *pResFltInp;

    /* Inputs for test LT_TID_MSOCKET */
    LtMSocketInputType *pMSocketInp;

    /* Inputs for test LT_TID_XCONNECT */
    LtXConnectInputType *pXConnectInp;

    /* Inputs for test LT_TID_CAP */
    LtCapInputType *pCapInp;

    /* Inputs for test LT_TID_ALL_GR_909 */
    LtAllGr909InputType *pAllGr909Inp;

    /* Inputs for test LT_TID_LOOPBACK */
    LtLoopbackInputType *pLoopbackInp;

    /* Inputs for test LT_TID_DC_FEED_ST */
    LtDcFeedSTInputType *pDcFeedSTInp;

    /* Inputs for test LT_TID_RD_LOOP_COND */
    LtRdLoopCondInputType *pRdLoopCondInp;

    /* Inputs for test LT_TID_DC_VOLTAGE */
    LtDcVoltageInputType *pDcVoltageInp;

    /* Inputs for test LT_TID_RINGING_ST */
    LtRingingSTInputType *pRingingSTInp;

    /* Inputs for test LT_TID_ON_OFF_HOOK_ST */
    LtOnOffHookSTInputType *pOnOffHookSTInp;

    /* Inputs for test LT_TID_RD_BAT_COND */
    LtReadBatCondInputType *pReadBatCondInp;

    /* Inputs for test LT_TID_PRE_LINE_V */
    LtPreLineVInputType *pPreLineVInp;

    /* Inputs for test LT_TID_PRE_LINE_V */
    LtFltDscrmInputType *pFltDscrmInp;

    /* DevNotes: More members ... */

} LtTestInputType;

/*******************************************************************************
 * Test Calibration Factors. Specifies calibration information
 * pertaining to a specific device...
 ******************************************************************************/
typedef union {
    /* preventing an empty union */
    uint8 *dummyPtr;

} LtCalFactorsType;

/*******************************************************************************
 * Test Topology Factors. Specifies any topology information
 * pertaining to a specific device...
 ******************************************************************************/
typedef union {
    /* preventing an empty union */
    uint8 *dummyPtr;

    #ifdef LT_VP880_PACKAGE
      /* 880 specific topology type is defined in lt_api_vp880.h*/
      LtVp880TestTopologyType *pVp880Topology;
    #endif /* LT_VP880_PACKAGE */

    #ifdef LT_VP890_PACKAGE
      /* 890 specific topology type is defined in lt_api_vp890.h*/
      LtVp890TestTopologyType *pVp890Topology;
    #endif /* LT_VP880_PACKAGE */

    #ifdef LT_VP886_PACKAGE
        LtVp886TestTopologyType *pVp886Topology;
    #endif /* LT_VP886_PACKAGE*/

    /* DevNotes: More topology types in future ... */
} LtTestTopologyType;

/*******************************************************************************
 * Line test input arguments to control various aspects of the test.
 * Application is expected to fill the appropriate member(s) of this
 * structure before calling the test library.
 ******************************************************************************/
typedef struct {
    /* Test inputs */
    LtTestInputType inputs;

    /* Line topology for the line that is performing the test */
    LtTestTopologyType topology;

    /* Calibration factors for the test */
    LtCalFactorsType calFactors;

    /* Criteria to be applied for determining the outcome of the test */
    LtTestCriteriaType criteria;

    /* Debug flag */
    uint32 ltDbgFlag;

} LtTestAttributesType;

/*******************************************************************************
 * Include definitions used for temporary storage definitions                  *
 ******************************************************************************/

typedef union {
    /* preventing an empty union */
    uint8 dummy;

    #ifdef LT_VP880_PACKAGE
      /* 880 specific temp type is defined in lt_api_vp880.h*/
      LtVp880TestTempType vp880Temp;
    #endif /* LT_VP880_PACKAGE */

    #ifdef LT_VP890_PACKAGE
      /* 890 specific temp type is defined in lt_api_vp890.h*/
      LtVp890TestTempType vp890Temp;
    #endif /* LT_VP890_PACKAGE */

    #ifdef LT_VP886_PACKAGE
      /* 886 specific temp type is defined in lt_api_vp886.h*/
      LtVp886TestTempType vp886Temp;
    #endif /* LT_VP886_PACKAGE */

    /* DevNotes: More temp types in future ... */

} LtTestTempType;


/* The following structure provides main stucture that the application passes
 * to the LT. This structure contains all the information needed by the
 * test library. This structure should never be modified by the application. */
typedef struct LtTestCtxType {
    VpDeviceType devType;
    VpTermType termType;
    LtTestIdType testId;
    int state;
    uint16 handle;
    LtTestAttributesType *pAttributes;
    VpLineCtxType *pLineCtx;
    LtTestResultType *pResult;
    LtTestTempType *pTemp;
    bool testFailed;
    LtTestStatusType tempStatus;

    /* Line Test handler such that this function could be initialized
     * based on the device family and based on the test that is being run. */
    TestEventHandlerFnPtrType TestEventHandlerFunc;

    #ifdef LT_VP886_PACKAGE
    /* new event handler type for 886 devices */
    Lt886TestEventHandlerFnPtrType Lt886TestEventHandlerFunc;
    #endif
} LtTestCtxType;

/*******************************************************************************
 * Function interface for the LT-API                                           *
 ******************************************************************************/
/* The following function needs to be called to start a test. */
EXTERN LtTestStatusType                   /* LT Test return type */
LtStartTest(
    VpLineCtxType *pLineCtx,              /* VP-API line context */
    LtTestIdType testId,                  /* LT TestId that is requested */
    uint16 handle,                        /* Handle to be used for the
                                           * VP-API */
    LtTestAttributesType *pAttributes,     /* Test attributes */
    LtTestTempType *pTemp,                /* Pointer to where temporary test
                                           * data is stored */
    LtTestResultType *pResult,            /* Pointer to where results need to
                                           * be stored */
    LtTestCtxType *pTestCtx);             /* LT test context pointer */

/* The following funciton needs to be called when events are reported for
 * a given test until the test conclusion is reached. */
EXTERN LtTestStatusType                   /* LT Test return type */
LtEventHandler(
    LtTestCtxType *pTestCtx,              /* Pointer to LT test context */
    LtEventType *pEvent);                 /* Pointer to LT event */

/* The following function could be used to abort an on-going test. */

EXTERN LtTestStatusType                   /* LT Test return type */
LtAbortTest(
    LtTestCtxType *pTestCtx);             /* Pointer to LT test context */


#endif /* !_LT_API_H_ */
