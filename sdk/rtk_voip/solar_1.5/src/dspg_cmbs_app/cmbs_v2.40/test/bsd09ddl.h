/* == IDENTIFICATION ==================================================
 *
 * Copyright (C) 2008, DSP Group Nuernberg
 *
 * Filename:   bsd09ddl.h                              
 *
 * created by:  ddl2c - DDL to C converter,
  Version 2.2, 19.02.2007 (compiled at Jul 21 2009, 15:16:27)
 *
 * Input file: c:\sw\projects\1029\trunk\ext\dspg\scorpion4\ft-com\bsd09ddl.ddp 
 */

# ifdef  cos19_c
#   define PUBLIC
# else
#   define PUBLIC extern
# endif
/* == IDENTIFICATION ================================================== */
/*
 * Copyright (C) 2009, DSP Group
 *
 * System           : Vega Family
 * Component        : FP
 * Module           : ALL
 * Unit             : ALL
 * Filename         : bsd09ddlo.ddl
 *
 * @(#)    %filespec: bsd09ddlo.ddl-BLRD53#E27.60 %
 */

/* == HISTORY ========================================================= */
/*
 * Name     Date        Ver     Action
 * -------------------------------------------------------------------
 * bhkori   13-Jan-2010 E27.60  Merge of parallel Versions  BLRD53#E27.59 & E27.59
 * tcmc_asa 12-Jan-2010 E27.59  Added MI_HS_CALL_REJECT_IND
 * bhkori   13-Jan-2010 E27.59  Added MI_CMBS_CALL_REDIRECT
 * rrbritto 08-Jan-2010 E27.58  Host managed line selection framework
 * tcmc_asa 23-Dec-2009 E27.57  merged MultiCall with Release 4.2.1 
 * tcmc_mso 17-Dec-2009 E27.56   removed CM_CH_DISC_CONNECT_IND again,
 *                               added MI_NO_RPAS_RING_EXP
 * tcmc_asa 17-Dec-2009 E27.55   merged E27.54 and NBGD53#E27.51
 * bhaskarm 15_dec-2009 E27.54   Removed ^M Character
 * hemanthk 14-Dec-2009 E27.53   CM_CH_PARALLEL_IND handled
 * pbhayare 11-Dec-2009	E27.48   Hold and Resume indication added
 * tcmc_mso 11-Dec-2009 E27.51   replace MI_PRE_OVERLAP_SENDING_LINEx_TMR_EXP
 *                               by MI_PRE_OVERLAP_SENDING_EXP,
 *                               replace MI_OVERLAP_SENDING_LINEx_TMR_EXP
 *                               by MI_OVERLAP_SENDING_EXP
 * tcmc_asa 09-Dec-2009 E27.50   replace PS_LINEx_EXP by MI_INC_CALL_EXP
 * tcmc_mso 09-Dec-2009 E27.49   added CM_CH_DISC_CONNECT_IND again
 *                               for Octopus/C_MULTICALL
 * tcmc_asa 03-Dec-2009 E27.48   added new MI_CALL_* messages for CMBS
 * rrbritto 03-Dec-2009 E27.47   Removd CM_CH_DISC_CONNECT_IND event
 * rrbritto 26-Nov-2009 E27.46   Merged E27.44.4 with E27.44.3
 * bhaskarm 23-Nov-2009 E27.44.3 Merege Parallel version E27.44.2 and E27.44.1
 * bhaskarm 20-Nov-2009 E27.44.2 Added SP on Base Call ID Trace Information
 * bhaskarm 13-Nov-2009 E27.44.1 added new event CM_CH_DISC_CONNECT_IND
 * rrbritto 24-Nov-2009 E27.44.4 SPK phone adaptation
 * hemanthk 30-Oct-2009 BLRD53#E27.44 integrated pbe changes for FWUP task
 * tcmc_pbe 10-Aug-2008 BLRD53#E27.9 firmware upgrade support added ( FWUP task)
 * hemanthk 27-Oct-2009 E27.43 HS Locator review comments fixed
 * hemanthk 24-Oct-2009 E27.42  added events for HS Loc
 * tcmc_asa 06-Oct-2009 E27.41  merged parallel E27.40 versions
 * bhaskarm 06-Oct-2009 E27.40  added Idle Timer for CSM and CH
 * tcmc_asa 02-Oct-2009 E27.40  added CM_CH_EXTCONF_2_INT_IND
 * tcmc_asa 29-Sep-2009 E27.39  added CM_CH_TOGGLE_OR_RESUME_IND
 * tcmc_asa 24-Sep-2009 E27.38  added CM_CH_INT_2_INT_IND
 * bhaskarm 17-Sep-2009 E27.37  added LA_HS_RANGE_IND
 * nsheetal 15-Sep-2009 BLRD53#E27.36 Merge parallel versions
 * bhaskarm 14-Sep-2009 E27.35  added MI_CALL_FORWARD_REQ
 * rrbritto 11-Sep-2009 E27.34  MI_DH_DATA_CALL_RDY added in FTMMI
 * nsheetal 11-Sep-2009 BLRD53#E27.33.1  Added event MI_DH_MORE_DATA_REQ to FTMI
 * nsheetal 08-Sep-2009 BLRD53#E27.33  Merge  parallel versions   
 * nsheetal 03-Sep-2009 BLRD53#E27.32 Added FTCC_CPR state to FTCC for LA service call
 * rrbritto 26-Aug-2009 E27.31.1Service Negotitation for MMC
 * nsheetal 24-Aug-2009 E27.31.2 Added event LA_END_SESSION to FTLA
 * bhaskarm 14-Jul-2009 E27.31  remove CM_CC_CALL_INFO_IND
 * bhaskarm 30-Jul-2009 E27.30  Removed LA_RELEASE_CFM
 * rrbritto 21-Jul-2009 E27.29  Call waiting accept implementation
 * nsheetal 16-Jul-2009 BLRD53#E27.26 Merge parallel versions
 * bhaskarm 14-Jul-2009 E27.25  added  MI_CALL_REINJECT_IND
 * tcmc_mso 13-Jul-2009 E27.24  added trace state and event information
 *                              for call state
 * nsheetal 13-Jul-2009 BLRD53#E27.24 Added few events to FTLA
 * tcmc_mso 10-Jul-2009 E27.23  new input CM_CC_UPDATE_CALL_STATE_IND
 * tcmc_mso 06-Jul-2009 E27.22  new inputs CM_CC_CALL_PROC_REQ (FTCC)
 *                              and CM_CC_CALL_PROC_IND (FTCSM)
 * tcmc_asa 05-Jun-2009 E27.19  merged 2xE27.19
 * tcmc_asa 29-May-2009 E27.19  added CM_CC_CALL_INFO_IND
 * nsheetal 01-Jun-2009 BLRD53#E27.19 Removed unused event LA_VOICE_LINK_REL_IND
 * rrbritto 26-May-2009 E27.18  Send CSM Instance updation to FTCC
 * rrbritto 23-May-2009 E27.17  Call transfer changes
 * bhaskarm 18-May-2009 E27.16  add FTCC_TST state for Tester
 * bhkori   30-Apr-2099 E27.15  Added MI_KEYPAD_IND
 * bhkori   30-Apr-2099 E27.14  Added MI_CALL_INFO_IND to FTMI
 * bhaskarm 06-Apr-2099 E27.13  Remove not used events
 * rrbritto 02-Apr-2009 E27.12  CDC NEGO events added in CSM/CC process
 * bhaskarm 31-Mar-2009 E27.11  add new events from MM to CSM
 * tcmc_asa 25-Mar-2009 E27.10  added POLYRING ON BASE - includes all BLRD53#E27.4.4
 * tcmc_iac 24-Dec-2008 ILD53#E16.2 POLYRING ON BASE added
 * nsheetal 04-Mar-2009 BLRD53#E27.9 Merged parallel versions
 * nsheetal 25-Feb-2009 BLRD53#E27.8 Added some more events to FTLA process
 * rrbritto 25-Feb-2009 E27.7.1 Porting of LDS from bsd09ddl.ddl version:BLR#E27.4.4
 * bhaskarm 17-Feb-2009 E27.7   Add Encryption event
 * rrlingam 16-Feb-2009 E27.6   ???
 * bhkori   10-Feb-2009 E27.5   FTLA process is added
 * bhaskarm 03-Feb-2009 E27.4   Add SS_FACILITY_IND
 * tcmc_asa 28-Jan-2009 E27.3   Add SS_FACILITY_CFM and LA_IWU_INFO_REQ
 * bhaskarm 27-Jan-2009 E27.2   Add Few Events in FTSS process
 * tcmc_asa 19-Nov-2008 E27.1   created from version E26.10 of bsd09ddl.ddl
 *
 * !!! older HISTORY see D53 bsd09ddl.ddl !! ***********************
 *                                             
 */

/*MPM:::all::all:bsd09ddlo.ddl ========================================= */
/*                                                                      */
/* ABSTRACT:                                                            */
/*                                                                      */
/* This module comprises global constants used for the message handling */
/* The processes, states, messages(events, primitives) and timers       */
/* of the system are defined in this file.                              */
/*                                                                      */

/*EMP ==================================================================*/


/* == DECLARATION OF GLOBAL DATA ====================================== */

/* general definitions; timers */
/* == START OF DEFINITIONS ========== */
/* timers */

#define DL04_TMR                      (2*SD09_CLOCK_TICKS_PER_SEC)
#define LC02_TMR                      (3*SD09_CLOCK_TICKS_PER_SEC)
#define LC03_TMR                      (3*SD09_CLOCK_TICKS_PER_SEC)
#define LC01_TMR                      (5*SD09_CLOCK_TICKS_PER_SEC)
#define LC02_REL_TMR                  (SD09_CLOCK_TICKS_PER_SEC)
#define MMK1_TMR                      (10*SD09_CLOCK_TICKS_PER_SEC)
#define MAU1_TMR                      (10*SD09_CLOCK_TICKS_PER_SEC)
#define MAC2_TMR                      (10*SD09_CLOCK_TICKS_PER_SEC)
#define MMC1_TMR                      (10*SD09_CLOCK_TICKS_PER_SEC)
#define MID1_TMR                      (10*SD09_CLOCK_TICKS_PER_SEC)

#define CC01_TMR                      (20*SD09_CLOCK_TICKS_PER_SEC)
#define CC02_TMR                      (36*SD09_CLOCK_TICKS_PER_SEC)
#define CC03_TMR                      (20*SD09_CLOCK_TICKS_PER_SEC)
#define CC04_TMR                      (100*SD09_CLOCK_TICKS_PER_SEC)
#define CC06_TMR                      (20*SD09_CLOCK_TICKS_PER_SEC)
#define CC_NOTIFY_TMR                 (20*SD09_CLOCK_TICKS_PER_SEC)

#define FTCSM_COLL_RING_TMR           (SD09_CLOCK_TICKS_PER_SEC)

#define RSSI_TMR                      (1*SD09_CLOCK_TICKS_PER_SEC)

#define FTMI_100MS_TMR                (SD09_CLOCK_TICKS_PER_SEC/10)
#define FTMI_200MS_TMR                (SD09_CLOCK_TICKS_PER_SEC/5)
#define FTMI_001_TMR                  ( 1*SD09_CLOCK_TICKS_PER_SEC)
#define FTMI_002_TMR                  ( 2*SD09_CLOCK_TICKS_PER_SEC)
#define FTMI_005_TMR                  ( 5*SD09_CLOCK_TICKS_PER_SEC)
#define FTMI_008_TMR                  ( 8*SD09_CLOCK_TICKS_PER_SEC)
#define FTMI_010_TMR                  (10*SD09_CLOCK_TICKS_PER_SEC)
#define FTMI_020_TMR                  (20*SD09_CLOCK_TICKS_PER_SEC)
#define FTMI_030_TMR                  (30*SD09_CLOCK_TICKS_PER_SEC)
#define FTMI_060_TMR                  (60*SD09_CLOCK_TICKS_PER_SEC)
#define FTMI_120_TMR                  (120*SD09_CLOCK_TICKS_PER_SEC)

#define FTMS_001_TMR                  ( 1*SD09_CLOCK_TICKS_PER_SEC)
#define FTMS_002_TMR                  ( 2*SD09_CLOCK_TICKS_PER_SEC)
#define FTMS_003_TMR                  ( 3*SD09_CLOCK_TICKS_PER_SEC)
#define FTMS_004_TMR                  ( 4*SD09_CLOCK_TICKS_PER_SEC)
#define FTMS_008_TMR                  ( 8*SD09_CLOCK_TICKS_PER_SEC)
#define FTMS_100MS_TMR                (SD09_CLOCK_TICKS_PER_SEC/10)

#define ALERT_OFF_TMR                 (3*SD09_CLOCK_TICKS_PER_SEC/2)

/*CSM/CH Idle Timer */
#define FTCSM_IDLE_TMR                (SD09_CLOCK_TICKS_PER_SEC/2)
#define FTCH_IDLE_TMR                 FTCSM_IDLE_TMR

/* Timers for ptmi */
#define PTMI_0020_TMR                 (1*SD09_CLOCK_TICKS_PER_SEC/5)
#define PTMI_0050_TMR                 (1*SD09_CLOCK_TICKS_PER_SEC/2)
#define PTMI_0100_TMR                 (1*SD09_CLOCK_TICKS_PER_SEC)

/* other constants */
#define MAC_DIS_REQ                   0xF0
#define MAC_CO_DATA_REQ               0xF2
#define MAC_CO_RR_DATA_REQ            0xF3
#define MAC_PAGE_REQ                  0xF4
#define MAC_INSTANCE_REQ              0xF6
#define MAC_MOD_REQ                   0xF9

/* FT_TRUE and FT_FALSE are needed by sdlplot, 0 is not accepted */
#define FT_TRUE                       1
#define FT_FALSE                      0

/* defines in OBD header used in pst_rt0_ProcessIdList, checked when smsprot lib is created */
#define K_G0_DIALLER_V                7
#define K_G0_SMS_PROTOCOL_V           10
#define K_G0_BAUDIO_V                 11
#define K_G0_SMS_MANAGERL_V           13
#define K_G0_FTMSI_V                  0   /* PS only, must be different from above */


/* MAC_MOD_CFM is only used in CP (DCP) higher layer. */
#define MAC_MOD_CFM                   MAC_MOD_IND
#define MAC_BW_MOD_CFM                MAC_BW_MOD_IND

/* == END OF DEFINITIONS ========== */
/* SET_NB_OF_INSTANCES SD09_MAX_NUM_CSM_SESSIONS */




/* == process FTCSM: code= 0, nr.of inst.= 10, prio.= high ====== */


/* -- states: -- */
#define FTCSM_RUN                                  0  	/* = 0x00 */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_StateNames_0[]
#ifdef cos19_c
 = {
    (u8 const *)"FTCSM_RUN",
    (u8 const *) 0
}
#endif
;

#endif
/* -- events: -- */
#define PR_PROP_DISP_REQ                           0  	/* = 0x00 */
#define PR_PROP_SETUP_REQ                          1  	/* = 0x01 */
#define PR_PROP_RELEASE_REQ                        2  	/* = 0x02 */
#define PS_ALERT_STOP_REQ                          3  	/* = 0x03 */
#define MI_SUBSCRIPTION_EN                         4  	/* = 0x04 */
#define MI_DESUBSCRIBE_REQ                         5  	/* = 0x05 */
#define MI_DISPLAY_REQ                             6  	/* = 0x06 */
#define MI_MODIFY_REQ                              7  	/* = 0x07 */
#define PS_SEND_CLIP_REQ                           8  	/* = 0x08 */
#define MI_CONFERENCE_TONE_REQ                     9  	/* = 0x09 */
#define MLP_INST_RELEASED_IND                     10  	/* = 0x0a */
#define CM_CC_IWU_INFO_IND                        11  	/* = 0x0b */
#define CM_CC_MULTI_IWU_INFO_IND                  12  	/* = 0x0c */
#define MI_IWU_INFO_REQ                           13  	/* = 0x0d */
#define MI_MULTI_IWU_INFO_REQ                     14  	/* = 0x0e */
#define CM_CH_TRANS_RELEASE_REQ                   15  	/* = 0x0f */
#define CM_LINE_HOLD_REQ                          16  	/* = 0x10 */
#define CM_LINE_PARK_REQ                          17  	/* = 0x11 */
#define CM_CALL_LINE_REQ                          18  	/* = 0x12 */
#define CM_CONF_ALL_REQ                           19  	/* = 0x13 */
#define CM_CONF_ALL_EXT_REQ                       20  	/* = 0x14 */
#define CM_DROP_EXT_REQ                           21  	/* = 0x15 */
#define CM_DROP_INT_REQ                           22  	/* = 0x16 */
#define CM_CALLBACK_REQ                           23  	/* = 0x17 */
#define CM_CHANGE_ALERT_REQ                       24  	/* = 0x18 */
#define CM_CH_LINE_REQ                            25  	/* = 0x19 */
#define CM_CALLBACK_EXPIRED_REQ                   26  	/* = 0x1a */
#define CM_CALLBACK_RELEASE_REQ                   27  	/* = 0x1b */
#define CM_SPLIT_LINES_REQ                        28  	/* = 0x1c */
#define CM_SWITCH_LINE_REQ                        29  	/* = 0x1d */
#define CM_CONNECT_DTAM_REQ                       30  	/* = 0x1e */
#define CM_CC_CDC_NEGO_IND                        31  	/* = 0x1f */
#define CM_CC_MODIFY_IND                          32  	/* = 0x20 */
#define CM_CC_MODIFY_CFM                          33  	/* = 0x21 */
#define CM_CC_MOD_CHNG_IND                        34  	/* = 0x22 */
#define CM_CC_CFM_CODEC_IND                       35  	/* = 0x23 */
#define CM_TRIG_SER_CHNG_REQ                      36  	/* = 0x24 */
#define CM_CC_TRANS_TIMED_OUT                     37  	/* = 0x25 */
#define TTS_CM_END                                38  	/* = 0x26 */
#define LDS_CALL_REQ                              39  	/* = 0x27 */
#define SS_FACILITY_CFM                           40  	/* = 0x28 */
#define SS_FACILITY_IND                           41  	/* = 0x29 */
#define LA_SERVICE_RELEASE_REQ                    42  	/* = 0x2a */
#define MM_CODEC_IND                              43  	/* = 0x2b */
#define MM_IWU_INFO_IND                           44  	/* = 0x2c */
#define MM_TERM_CAP_IND                           45  	/* = 0x2d */
#define MNCM_IWU_INFO_REQ                         46  	/* = 0x2e */
#define CC_SETUP_IND                              47  	/* = 0x2f */
#define CC_CALL_IND                               48  	/* = 0x30 */
#define CC_CONNECT_IND                            49  	/* = 0x31 */
#define CC_CALL_PROC_IND                          50  	/* = 0x32 */
#define CC_ALERT_IND                              51  	/* = 0x33 */
#define CC_INFO_IND                               52  	/* = 0x34 */
#define CC_IWU_INFO_IND                           53  	/* = 0x35 */
#define CS_RELEASE_IND                            54  	/* = 0x36 */
#define SS_CS_FACILITY_IND                        55  	/* = 0x37 */
#define MM_CS_CODEC_IND                           56  	/* = 0x38 */
#define CC_SER_CHNG_IND                           57  	/* = 0x39 */
#define CC_SER_CHNG_RES                           58  	/* = 0x3a */
#define CC_UNKNOWN                                59  	/* = 0x3b */
#define PS_CALL_REQ                               60  	/* = 0x3c */
#define PS_CALL_CFM                               61  	/* = 0x3d */
#define PS_RELEASE_REQ                            62  	/* = 0x3e */
#define CM_CC_CHECKCM_IND                         63  	/* = 0x3f */
#define CM_CC_ALERT_IND                           64  	/* = 0x40 */
#define CM_CC_RELEASE_IND                         65  	/* = 0x41 */
#define CM_CC_SETUP_IND                           66  	/* = 0x42 */
#define CM_CC_CONNECT_IND                         67  	/* = 0x43 */
#define CM_CC_DIAL_IND                            68  	/* = 0x44 */
#define CM_CC_CALL_INFO_IND                       69  	/* = 0x45 */
#define CM_CC_CALL_PROC_IND                       70  	/* = 0x46 */
#define CM_CC_UPDATE_CALL_STATE_IND               71  	/* = 0x47 */
#define CM_IDLE_TIMER_EXPIRED                     72  	/* = 0x48 */
#define MI_HS_LOCATOR_REQ                         73  	/* = 0x49 */
#define MI_HS_LOCATOR_REL                         74  	/* = 0x4a */
#define MI_CALL_PROC_REQ                          75  	/* = 0x4b */
#define MI_CALL_ALERT_REQ                         76  	/* = 0x4c */
#define MI_CALL_CONNECT_REQ                       77  	/* = 0x4d */
#define MI_CALL_STATUS_UPDATE_REQ                 78  	/* = 0x4e */
#define MI_CMBS_CALL_REDIRECT                     79  	/* = 0x4f */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_EventNames_0[]
#ifdef cos19_c
 = {
    (u8 const *)"PR_PROP_DISP_REQ",
    (u8 const *)"PR_PROP_SETUP_REQ",
    (u8 const *)"PR_PROP_RELEASE_REQ",
    (u8 const *)"PS_ALERT_STOP_REQ",
    (u8 const *)"MI_SUBSCRIPTION_EN",
    (u8 const *)"MI_DESUBSCRIBE_REQ",
    (u8 const *)"MI_DISPLAY_REQ",
    (u8 const *)"MI_MODIFY_REQ",
    (u8 const *)"PS_SEND_CLIP_REQ",
    (u8 const *)"MI_CONFERENCE_TONE_REQ",
    (u8 const *)"MLP_INST_RELEASED_IND",
    (u8 const *)"CM_CC_IWU_INFO_IND",
    (u8 const *)"CM_CC_MULTI_IWU_INFO_IND",
    (u8 const *)"MI_IWU_INFO_REQ",
    (u8 const *)"MI_MULTI_IWU_INFO_REQ",
    (u8 const *)"CM_CH_TRANS_RELEASE_REQ",
    (u8 const *)"CM_LINE_HOLD_REQ",
    (u8 const *)"CM_LINE_PARK_REQ",
    (u8 const *)"CM_CALL_LINE_REQ",
    (u8 const *)"CM_CONF_ALL_REQ",
    (u8 const *)"CM_CONF_ALL_EXT_REQ",
    (u8 const *)"CM_DROP_EXT_REQ",
    (u8 const *)"CM_DROP_INT_REQ",
    (u8 const *)"CM_CALLBACK_REQ",
    (u8 const *)"CM_CHANGE_ALERT_REQ",
    (u8 const *)"CM_CH_LINE_REQ",
    (u8 const *)"CM_CALLBACK_EXPIRED_REQ",
    (u8 const *)"CM_CALLBACK_RELEASE_REQ",
    (u8 const *)"CM_SPLIT_LINES_REQ",
    (u8 const *)"CM_SWITCH_LINE_REQ",
    (u8 const *)"CM_CONNECT_DTAM_REQ",
    (u8 const *)"CM_CC_CDC_NEGO_IND",
    (u8 const *)"CM_CC_MODIFY_IND",
    (u8 const *)"CM_CC_MODIFY_CFM",
    (u8 const *)"CM_CC_MOD_CHNG_IND",
    (u8 const *)"CM_CC_CFM_CODEC_IND",
    (u8 const *)"CM_TRIG_SER_CHNG_REQ",
    (u8 const *)"CM_CC_TRANS_TIMED_OUT",
    (u8 const *)"TTS_CM_END",
    (u8 const *)"LDS_CALL_REQ",
    (u8 const *)"SS_FACILITY_CFM",
    (u8 const *)"SS_FACILITY_IND",
    (u8 const *)"LA_SERVICE_RELEASE_REQ",
    (u8 const *)"MM_CODEC_IND",
    (u8 const *)"MM_IWU_INFO_IND",
    (u8 const *)"MM_TERM_CAP_IND",
    (u8 const *)"MNCM_IWU_INFO_REQ",
    (u8 const *)"CC_SETUP_IND",
    (u8 const *)"CC_CALL_IND",
    (u8 const *)"CC_CONNECT_IND",
    (u8 const *)"CC_CALL_PROC_IND",
    (u8 const *)"CC_ALERT_IND",
    (u8 const *)"CC_INFO_IND",
    (u8 const *)"CC_IWU_INFO_IND",
    (u8 const *)"CS_RELEASE_IND",
    (u8 const *)"SS_CS_FACILITY_IND",
    (u8 const *)"MM_CS_CODEC_IND",
    (u8 const *)"CC_SER_CHNG_IND",
    (u8 const *)"CC_SER_CHNG_RES",
    (u8 const *)"CC_UNKNOWN",
    (u8 const *)"PS_CALL_REQ",
    (u8 const *)"PS_CALL_CFM",
    (u8 const *)"PS_RELEASE_REQ",
    (u8 const *)"CM_CC_CHECKCM_IND",
    (u8 const *)"CM_CC_ALERT_IND",
    (u8 const *)"CM_CC_RELEASE_IND",
    (u8 const *)"CM_CC_SETUP_IND",
    (u8 const *)"CM_CC_CONNECT_IND",
    (u8 const *)"CM_CC_DIAL_IND",
    (u8 const *)"CM_CC_CALL_INFO_IND",
    (u8 const *)"CM_CC_CALL_PROC_IND",
    (u8 const *)"CM_CC_UPDATE_CALL_STATE_IND",
    (u8 const *)"CM_IDLE_TIMER_EXPIRED",
    (u8 const *)"MI_HS_LOCATOR_REQ",
    (u8 const *)"MI_HS_LOCATOR_REL",
    (u8 const *)"MI_CALL_PROC_REQ",
    (u8 const *)"MI_CALL_ALERT_REQ",
    (u8 const *)"MI_CALL_CONNECT_REQ",
    (u8 const *)"MI_CALL_STATUS_UPDATE_REQ",
    (u8 const *)"MI_CMBS_CALL_REDIRECT",
    (u8 const *) 0
}
#endif
;

#endif

 /* processes with 1 instances each */


/* == process FTDSR: code= 1, nr.of inst.= 1, prio.= high ====== */


/* -- states: -- */
#define FTDSR_RUN                                  0  	/* = 0x00 */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_StateNames_1[]
#ifdef cos19_c
 = {
    (u8 const *)"FTDSR_RUN",
    (u8 const *) 0
}
#endif
;

#endif
/* -- events: -- */
#define MSUB_EXPIRED                               0  	/* = 0x00 */
#define ALERT_OFF_1_EXPIRED                        1  	/* = 0x01 */
#define ALERT_OFF_2_EXPIRED                        2  	/* = 0x02 */
#define ALERT_CHANGE_EXPIRED                       3  	/* = 0x03 */
#define FTDSR_COLL_RING_EXPIRED                    4  	/* = 0x04 */
#define MAC_CM_INSTANCE_CFM                        5  	/* = 0x05 */
#define CLMS_PAGE_CFM                              6  	/* = 0x06 */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_EventNames_1[]
#ifdef cos19_c
 = {
    (u8 const *)"MSUB_EXPIRED",
    (u8 const *)"ALERT_OFF_1_EXPIRED",
    (u8 const *)"ALERT_OFF_2_EXPIRED",
    (u8 const *)"ALERT_CHANGE_EXPIRED",
    (u8 const *)"FTDSR_COLL_RING_EXPIRED",
    (u8 const *)"MAC_CM_INSTANCE_CFM",
    (u8 const *)"CLMS_PAGE_CFM",
    (u8 const *) 0
}
#endif
;

#endif

 /* processes with 6 instances each */



/* == process FTSS: code= 2, nr.of inst.= 6, prio.= high ====== */


/* -- states: -- */
#define FTSS_RUN                                   0  	/* = 0x00 */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_StateNames_2[]
#ifdef cos19_c
 = {
    (u8 const *)"FTSS_RUN",
    (u8 const *) 0
}
#endif
;

#endif
/* -- events: -- */
#define SS_RELEASE_IND                             0  	/* = 0x00 */
#define SS_DATA_IND                                1  	/* = 0x01 */
#define MI_FEATURE_CFM                             2  	/* = 0x02 */
#define SS_FACILITY_REQ                            3  	/* = 0x03 */
#define SS_FACILITY_SENT_IND                       4  	/* = 0x04 */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_EventNames_2[]
#ifdef cos19_c
 = {
    (u8 const *)"SS_RELEASE_IND",
    (u8 const *)"SS_DATA_IND",
    (u8 const *)"MI_FEATURE_CFM",
    (u8 const *)"SS_FACILITY_REQ",
    (u8 const *)"SS_FACILITY_SENT_IND",
    (u8 const *) 0
}
#endif
;

#endif




/* == process FTMM: code= 3, nr.of inst.= 6, prio.= high ====== */


/* -- states: -- */
#define FTMM_RUN                                   0  	/* = 0x00 */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_StateNames_3[]
#ifdef cos19_c
 = {
    (u8 const *)"FTMM_RUN",
    (u8 const *) 0
}
#endif
;

#endif
/* -- events: -- */
#define MM_RELEASE_IND                             0  	/* = 0x00 */
#define MM_DATA_IND                                1  	/* = 0x01 */
#define MM_LOC_UPDATE_REQ                          2  	/* = 0x02 */
#define MM_ENCRYPT_IND                             3  	/* = 0x03 */
#define CC_MM_AUTH_REQ                             4  	/* = 0x04 */
#define CM_MM_DESUBSCRIBE_REQ                      5  	/* = 0x05 */
#define MMC1_EXPIRED                               6  	/* = 0x06 */
#define MMK1_EXPIRED                               7  	/* = 0x07 */
#define MAU1_EXPIRED                               8  	/* = 0x08 */
#define MAC2_EXPIRED                               9  	/* = 0x09 */
#define MID1_EXPIRED                              10  	/* = 0x0a */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_EventNames_3[]
#ifdef cos19_c
 = {
    (u8 const *)"MM_RELEASE_IND",
    (u8 const *)"MM_DATA_IND",
    (u8 const *)"MM_LOC_UPDATE_REQ",
    (u8 const *)"MM_ENCRYPT_IND",
    (u8 const *)"CC_MM_AUTH_REQ",
    (u8 const *)"CM_MM_DESUBSCRIBE_REQ",
    (u8 const *)"MMC1_EXPIRED",
    (u8 const *)"MMK1_EXPIRED",
    (u8 const *)"MAU1_EXPIRED",
    (u8 const *)"MAC2_EXPIRED",
    (u8 const *)"MID1_EXPIRED",
    (u8 const *) 0
}
#endif
;

#endif




/* == process FTMLP: code= 4, nr.of inst.= 6, prio.= high ====== */

 /* code 0 */
/* -- states: -- */
#define FTML_END                                   0  	/* = 0x00 */
#define FTML_BRC                                   1  	/* = 0x01 */
#define FTML_CON                                   2  	/* = 0x02 */
#define FTML_RUN                                   3  	/* = 0x03 */
#define FTML_REL                                   4  	/* = 0x04 */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_StateNames_4[]
#ifdef cos19_c
 = {
    (u8 const *)"FTML_END",
    (u8 const *)"FTML_BRC",
    (u8 const *)"FTML_CON",
    (u8 const *)"FTML_RUN",
    (u8 const *)"FTML_REL",
    (u8 const *) 0
}
#endif
;

#endif
/* -- events: -- */
#define MM_DATA_REQ                                0  	/* = 0x00 */
#define CC_DATA_REQ                                1  	/* = 0x01 */
#define SS_DATA_REQ                                2  	/* = 0x02 */
#define MM_RELEASE_REQ                             3  	/* = 0x03 */
#define CC_RELEASE_REQ                             4  	/* = 0x04 */
#define SS_RELEASE_REQ                             5  	/* = 0x05 */
#define CC_MOD_CHNG_REQ                            6  	/* = 0x06 */
#define MAC_CO_DATA_IND                            7  	/* = 0x07 */
#define MAC_CO_DTR_IND                             8  	/* = 0x08 */
#define MAC_ENC_EKS_IND                            9  	/* = 0x09 */
#define MAC_CON_IND                               10  	/* = 0x0a */
#define MAC_DIS_IND                               11  	/* = 0x0b */
#define MAC_DIS_CFM                               12  	/* = 0x0c */
#define LC01_EXPIRED                              13  	/* = 0x0d */
#define LC02_EXPIRED                              14  	/* = 0x0e */
#define LC03_EXPIRED                              15  	/* = 0x0f */
#define DL04_EXPIRED                              16  	/* = 0x10 */
#define MAC_CON_DATA_IND                          17  	/* = 0x11 */
#define MAC_CON_DATA_CFM                          18  	/* = 0x12 */
#define MAC_MOD_IND                               19  	/* = 0x13 */
#define MAC_BW_MOD_IND                            20  	/* = 0x14 */
#define CF_DATA_IND                               21  	/* = 0x15 */
#define READY_FOR_CF_TX                           22  	/* = 0x16 */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_EventNames_4[]
#ifdef cos19_c
 = {
    (u8 const *)"MM_DATA_REQ",
    (u8 const *)"CC_DATA_REQ",
    (u8 const *)"SS_DATA_REQ",
    (u8 const *)"MM_RELEASE_REQ",
    (u8 const *)"CC_RELEASE_REQ",
    (u8 const *)"SS_RELEASE_REQ",
    (u8 const *)"CC_MOD_CHNG_REQ",
    (u8 const *)"MAC_CO_DATA_IND",
    (u8 const *)"MAC_CO_DTR_IND",
    (u8 const *)"MAC_ENC_EKS_IND",
    (u8 const *)"MAC_CON_IND",
    (u8 const *)"MAC_DIS_IND",
    (u8 const *)"MAC_DIS_CFM",
    (u8 const *)"LC01_EXPIRED",
    (u8 const *)"LC02_EXPIRED",
    (u8 const *)"LC03_EXPIRED",
    (u8 const *)"DL04_EXPIRED",
    (u8 const *)"MAC_CON_DATA_IND",
    (u8 const *)"MAC_CON_DATA_CFM",
    (u8 const *)"MAC_MOD_IND",
    (u8 const *)"MAC_BW_MOD_IND",
    (u8 const *)"CF_DATA_IND",
    (u8 const *)"READY_FOR_CF_TX",
    (u8 const *) 0
}
#endif
;

#endif





/* == process p_hl17_LUXProcess: code= 5, nr.of inst.= 6, prio.= high ====== */

 
/* -- states: -- */
#define LUXProcess_RUN                             0  	/* = 0x00 */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_StateNames_5[]
#ifdef cos19_c
 = {
    (u8 const *)"LUXProcess_RUN",
    (u8 const *) 0
}
#endif
;

#endif
/* -- events: -- */
#define MI_LUX_SDU_SEND                            0  	/* = 0x00 */
#define HM_LUX_PDU_SENT                            1  	/* = 0x01 */
#define HM_LUX_PDU_RCVD                            2  	/* = 0x02 */
#define MI_LUX_CLOSE_SESN                          3  	/* = 0x03 */
#define TM_LUX_TMR_T0                              4  	/* = 0x04 */
#define TM_LUX_SDU_TMR                             5  	/* = 0x05 */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_EventNames_5[]
#ifdef cos19_c
 = {
    (u8 const *)"MI_LUX_SDU_SEND",
    (u8 const *)"HM_LUX_PDU_SENT",
    (u8 const *)"HM_LUX_PDU_RCVD",
    (u8 const *)"MI_LUX_CLOSE_SESN",
    (u8 const *)"TM_LUX_TMR_T0",
    (u8 const *)"TM_LUX_SDU_TMR",
    (u8 const *) 0
}
#endif
;

#endif




/* == process FTHE: code= 6, nr.of inst.= 6, prio.= high ====== */


/* -- states: -- */
#define FP_SUBS_ACTIVE                             0  	/* = 0x00 */
#define FP_SUBS_WAIT_SWITCH                        1  	/* = 0x01 */
#define FP_SUBS_WAIT_SUBS                          2  	/* = 0x02 */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_StateNames_6[]
#ifdef cos19_c
 = {
    (u8 const *)"FP_SUBS_ACTIVE",
    (u8 const *)"FP_SUBS_WAIT_SWITCH",
    (u8 const *)"FP_SUBS_WAIT_SUBS",
    (u8 const *) 0
}
#endif
;

#endif
/* -- events: -- */
#define HT_DATA_CFM                                0  	/* = 0x00 */
#define HM_CON_DIS_IND                             1  	/* = 0x01 */
#define SUBSTITUTE_REQ                             2  	/* = 0x02 */
#define SWITCH_SEQUENCE_REQ                        3  	/* = 0x03 */
#define HT_DATA_IND                                4  	/* = 0x04 */
#define HM_CON_SETUP_IND                           5  	/* = 0x05 */
#define HM_BEARER_SETUP_IND                        6  	/* = 0x06 */
#define HM_BEARER_DIS_IND                          7  	/* = 0x07 */
#define REPORT_BER_FER                             8  	/* = 0x08 */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_EventNames_6[]
#ifdef cos19_c
 = {
    (u8 const *)"HT_DATA_CFM",
    (u8 const *)"HM_CON_DIS_IND",
    (u8 const *)"SUBSTITUTE_REQ",
    (u8 const *)"SWITCH_SEQUENCE_REQ",
    (u8 const *)"HT_DATA_IND",
    (u8 const *)"HM_CON_SETUP_IND",
    (u8 const *)"HM_BEARER_SETUP_IND",
    (u8 const *)"HM_BEARER_DIS_IND",
    (u8 const *)"REPORT_BER_FER",
    (u8 const *) 0
}
#endif
;

#endif



/* == process FTCH: code= 7, nr.of inst.= 6, prio.= high ====== */


/* -- states: -- */
#define FTCH_END                                   0  	/* = 0x00 */
#define FTCH_INT                                   1  	/* = 0x01 */
#define FTCH_RUN                                   2  	/* = 0x02 */
#define FTCH_RLS                                   3  	/* = 0x03 */
#define FTCH_RLO                                   4  	/* = 0x04 */
#define FTCH_RLH                                   5  	/* = 0x05 */
#define FTCH_RPS                                   6  	/* = 0x06 */
#define FTCH_PRE                                   7  	/* = 0x07 */
#define FTCH_REC                                   8  	/* = 0x08 */
#define FTCH_RLW                                   9  	/* = 0x09 */
#define FTCH_RCC                                  10  	/* = 0x0a */
#define FTCH_THL                                  11  	/* = 0x0b */
#define FTCH_TLC                                  12  	/* = 0x0c */
#define FTCH_TLH                                  13  	/* = 0x0d */
#define FTCH_TLA                                  14  	/* = 0x0e */
#define FTCH_BTR                                  15  	/* = 0x0f */
#define FTCH_DAT                                  16  	/* = 0x10 */
#define FTCH_ELR                                  17  	/* = 0x11 */
#define FTCH_HLO                                  18  	/* = 0x12 */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_StateNames_7[]
#ifdef cos19_c
 = {
    (u8 const *)"FTCH_END",
    (u8 const *)"FTCH_INT",
    (u8 const *)"FTCH_RUN",
    (u8 const *)"FTCH_RLS",
    (u8 const *)"FTCH_RLO",
    (u8 const *)"FTCH_RLH",
    (u8 const *)"FTCH_RPS",
    (u8 const *)"FTCH_PRE",
    (u8 const *)"FTCH_REC",
    (u8 const *)"FTCH_RLW",
    (u8 const *)"FTCH_RCC",
    (u8 const *)"FTCH_THL",
    (u8 const *)"FTCH_TLC",
    (u8 const *)"FTCH_TLH",
    (u8 const *)"FTCH_TLA",
    (u8 const *)"FTCH_BTR",
    (u8 const *)"FTCH_DAT",
    (u8 const *)"FTCH_ELR",
    (u8 const *)"FTCH_HLO",
    (u8 const *) 0
}
#endif
;

#endif
// /* -- events: -- */
#define CM_CH_CONNECT_IND                          0  	/* = 0x00 */
#define CM_CH_CONNECT_INTERNAL_IND                 1  	/* = 0x01 */
#define CM_CH_RELEASE_IND                          2  	/* = 0x02 */
#define CM_CH_RELEASE_OTHER_IND                    3  	/* = 0x03 */
#define CM_CH_DIAL_IND                             4  	/* = 0x04 */
#define CM_CH_TRANS_RELEASE_IND                    5  	/* = 0x05 */
#define CM_CH_LINE_CFM                             6  	/* = 0x06 */
#define CM_CH_LINE_HOLD_IND                        7  	/* = 0x07 */
#define CM_CH_LINE_PARK_IND                        8  	/* = 0x08 */
#define CM_CH_CALL_LINE_IND                        9  	/* = 0x09 */
#define CM_CH_CONF_ALL_IND                        10  	/* = 0x0a */
#define CM_CH_CONF_ALL_EXT_IND                    11  	/* = 0x0b */
#define CM_CH_DROP_EXT_IND                        12  	/* = 0x0c */
#define CM_CH_DROP_INT_IND                        13  	/* = 0x0d */
#define CM_CH_PARKED_CONN_IND                     14  	/* = 0x0e */
#define CM_CH_LINE_RELEASE_IND                    15  	/* = 0x0f */
#define CM_CH_BLIND_TRANS_IND                     16  	/* = 0x10 */
#define CM_CH_EXT_2_INT_IND                       17  	/* = 0x11 */
#define CM_CH_EXTCONF_2_INT_IND                   18  	/* = 0x12 */
#define CM_CH_INT_2_INT_IND                       19  	/* = 0x13 */
#define CM_CH_TOGGLE_OR_RESUME_IND                20  	/* = 0x14 */
#define CM_CH_SWITCH_2_INT_IND                    21  	/* = 0x15 */
#define CM_CH_SPLIT_LINES_IND                     22  	/* = 0x16 */
#define CM_CH_DTAM_CONN_IND                       23  	/* = 0x17 */
#define CM_CH_USB_CONN_IND                        24  	/* = 0x18 */
#define CM_CH_DATA_CONN_IND                       25  	/* = 0x19 */
#define CM_CH_VOICE_TO_DATA_IND                   26  	/* = 0x1a */
#define CM_CH_CODEC_MOD_IND                       27  	/* = 0x1b */
#define CM_CH_TRANS_TIMED_OUT                     28  	/* = 0x1c */
#define CM_CH_CALL_TRANS_IND                      29  	/* = 0x1d */
#define CM_CH_LOC_HOLD_IND                        30  	/* = 0x1e */
#define CM_CH_CONNECT_TOG_IND                     31  	/* = 0x1f */
#define CM_CH_RELEASE_TOG_IND                     32  	/* = 0x20 */
#define CH_IDLE_TIMER_EXPIRED                     33  	/* = 0x21 */
#define CM_CH_LOC_HOLD_MUTE_IND                   34  	/* = 0x22 */
#define CM_CH_LOC_RESUME_MUTE_IND                 35  	/* = 0x23 */
#define CM_CH_PARALLEL_REL_IND                    36  	/* = 0x24 */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_EventNames_7[]
#ifdef cos19_c
 = {
    (u8 const *)"CM_CH_CONNECT_IND",
    (u8 const *)"CM_CH_CONNECT_INTERNAL_IND",
    (u8 const *)"CM_CH_RELEASE_IND",
    (u8 const *)"CM_CH_RELEASE_OTHER_IND",
    (u8 const *)"CM_CH_DIAL_IND",
    (u8 const *)"CM_CH_TRANS_RELEASE_IND",
    (u8 const *)"CM_CH_LINE_CFM",
    (u8 const *)"CM_CH_LINE_HOLD_IND",
    (u8 const *)"CM_CH_LINE_PARK_IND",
    (u8 const *)"CM_CH_CALL_LINE_IND",
    (u8 const *)"CM_CH_CONF_ALL_IND",
    (u8 const *)"CM_CH_CONF_ALL_EXT_IND",
    (u8 const *)"CM_CH_DROP_EXT_IND",
    (u8 const *)"CM_CH_DROP_INT_IND",
    (u8 const *)"CM_CH_PARKED_CONN_IND",
    (u8 const *)"CM_CH_LINE_RELEASE_IND",
    (u8 const *)"CM_CH_BLIND_TRANS_IND",
    (u8 const *)"CM_CH_EXT_2_INT_IND",
    (u8 const *)"CM_CH_EXTCONF_2_INT_IND",
    (u8 const *)"CM_CH_INT_2_INT_IND",
    (u8 const *)"CM_CH_TOGGLE_OR_RESUME_IND",
    (u8 const *)"CM_CH_SWITCH_2_INT_IND",
    (u8 const *)"CM_CH_SPLIT_LINES_IND",
    (u8 const *)"CM_CH_DTAM_CONN_IND",
    (u8 const *)"CM_CH_USB_CONN_IND",
    (u8 const *)"CM_CH_DATA_CONN_IND",
    (u8 const *)"CM_CH_VOICE_TO_DATA_IND",
    (u8 const *)"CM_CH_CODEC_MOD_IND",
    (u8 const *)"CM_CH_TRANS_TIMED_OUT",
    (u8 const *)"CM_CH_CALL_TRANS_IND",
    (u8 const *)"CM_CH_LOC_HOLD_IND",
    (u8 const *)"CM_CH_CONNECT_TOG_IND",
    (u8 const *)"CM_CH_RELEASE_TOG_IND",
    (u8 const *)"CH_IDLE_TIMER_EXPIRED",
    (u8 const *)"CM_CH_LOC_HOLD_MUTE_IND",
    (u8 const *)"CM_CH_LOC_RESUME_MUTE_IND",
    (u8 const *)"CM_CH_PARALLEL_REL_IND",
    (u8 const *) 0
}
#endif
;

#endif


 /* processes with 18 instances each */



/* == process FTCC: code= 8, nr.of inst.= 18, prio.= high ====== */


/* -- states: -- */
#define FTCC_END                                   0  	/* = 0x00 */
#define FTCC_AOG                                   1  	/* = 0x01 */
#define FTCC_WDT                                   2  	/* = 0x02 */
#define FTCC_DEL                                   3  	/* = 0x03 */
#define FTCC_PRC                                   4  	/* = 0x04 */
#define FTCC_OVS                                   5  	/* = 0x05 */
#define FTCC_AIC                                   6  	/* = 0x06 */
#define FTCC_REL                                   7  	/* = 0x07 */
#define FTCC_PRE                                   8  	/* = 0x08 */
#define FTCC_REC                                   9  	/* = 0x09 */
#define FTCC_RUN                                  10  	/* = 0x0a */
#define FTCC_TST                                  11  	/* = 0x0b */
#define FTCC_CPR                                  12  	/* = 0x0c */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_StateNames_8[]
#ifdef cos19_c
 = {
    (u8 const *)"FTCC_END",
    (u8 const *)"FTCC_AOG",
    (u8 const *)"FTCC_WDT",
    (u8 const *)"FTCC_DEL",
    (u8 const *)"FTCC_PRC",
    (u8 const *)"FTCC_OVS",
    (u8 const *)"FTCC_AIC",
    (u8 const *)"FTCC_REL",
    (u8 const *)"FTCC_PRE",
    (u8 const *)"FTCC_REC",
    (u8 const *)"FTCC_RUN",
    (u8 const *)"FTCC_TST",
    (u8 const *)"FTCC_CPR",
    (u8 const *) 0
}
#endif
;

#endif
/* -- events: -- */
#define CC_RELEASE_IND                             0  	/* = 0x00 */
#define CC_DATA_IND                                1  	/* = 0x01 */
#define CM_CC_SETUP_REQ                            2  	/* = 0x02 */
#define CM_CC_INFO_REQ                             3  	/* = 0x03 */
#define CM_CC_CHECKCM_RES                          4  	/* = 0x04 */
#define CM_CC_CONNECT_REQ                          5  	/* = 0x05 */
#define CM_CC_RELEASE_REQ                          6  	/* = 0x06 */
#define CM_CC_ALERT_REQ                            7  	/* = 0x07 */
#define CM_CC_CDC_NEGO_RES                         8  	/* = 0x08 */
#define CM_CC_MODIFY_REQ                           9  	/* = 0x09 */
#define CM_CC_MOD_CHNG_REQ                        10  	/* = 0x0a */
#define CM_CC_MODIFY_RES                          11  	/* = 0x0b */
#define CC_MOD_CHNG_IND                           12  	/* = 0x0c */
#define CC_MM_AUTH_CFM                            13  	/* = 0x0d */
#define CC01_EXPIRED                              14  	/* = 0x0e */
#define CC02_EXPIRED                              15  	/* = 0x0f */
#define CC03_EXPIRED                              16  	/* = 0x10 */
#define CC04_EXPIRED                              17  	/* = 0x11 */
#define CC06_EXPIRED                              18  	/* = 0x12 */
#define CC_NOTIFY_EXPIRED                         19  	/* = 0x13 */
#define MNCC_SETUP_REQ                            20  	/* = 0x14 */
#define MNCC_CONNECT_REQ                          21  	/* = 0x15 */
#define MNCC_RELEASE_REQ                          22  	/* = 0x16 */
#define CM_CC_IWU_INFO_REQ                        23  	/* = 0x17 */
#define SP_CC_CALL_REQ                            24  	/* = 0x18 */
#define TST_STATE_REQ                             25  	/* = 0x19 */
#define TST_STATE_END_REQ                         26  	/* = 0x1a */
#define CS_SETUP_REQ                              27  	/* = 0x1b */
#define CS_SETUP_ACK_REQ                          28  	/* = 0x1c */
#define CS_CALL_PROC_REQ                          29  	/* = 0x1d */
#define CS_IWU_INFO_REQ                           30  	/* = 0x1e */
#define CS_CONNECT_REQ                            31  	/* = 0x1f */
#define CS_ALERT_REQ                              32  	/* = 0x20 */
#define CS_INFO_REQ                               33  	/* = 0x21 */
#define CS_RELEASE_REQ                            34  	/* = 0x22 */
#define CS_SER_CHNG_REQ                           35  	/* = 0x23 */
#define CS_SER_CHNG_RES                           36  	/* = 0x24 */
#define CSM_CC_NEW_CALL_ID                        37  	/* = 0x25 */
#define CM_CC_CALL_PROC_REQ                       38  	/* = 0x26 */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_EventNames_8[]
#ifdef cos19_c
 = {
    (u8 const *)"CC_RELEASE_IND",
    (u8 const *)"CC_DATA_IND",
    (u8 const *)"CM_CC_SETUP_REQ",
    (u8 const *)"CM_CC_INFO_REQ",
    (u8 const *)"CM_CC_CHECKCM_RES",
    (u8 const *)"CM_CC_CONNECT_REQ",
    (u8 const *)"CM_CC_RELEASE_REQ",
    (u8 const *)"CM_CC_ALERT_REQ",
    (u8 const *)"CM_CC_CDC_NEGO_RES",
    (u8 const *)"CM_CC_MODIFY_REQ",
    (u8 const *)"CM_CC_MOD_CHNG_REQ",
    (u8 const *)"CM_CC_MODIFY_RES",
    (u8 const *)"CC_MOD_CHNG_IND",
    (u8 const *)"CC_MM_AUTH_CFM",
    (u8 const *)"CC01_EXPIRED",
    (u8 const *)"CC02_EXPIRED",
    (u8 const *)"CC03_EXPIRED",
    (u8 const *)"CC04_EXPIRED",
    (u8 const *)"CC06_EXPIRED",
    (u8 const *)"CC_NOTIFY_EXPIRED",
    (u8 const *)"MNCC_SETUP_REQ",
    (u8 const *)"MNCC_CONNECT_REQ",
    (u8 const *)"MNCC_RELEASE_REQ",
    (u8 const *)"CM_CC_IWU_INFO_REQ",
    (u8 const *)"SP_CC_CALL_REQ",
    (u8 const *)"TST_STATE_REQ",
    (u8 const *)"TST_STATE_END_REQ",
    (u8 const *)"CS_SETUP_REQ",
    (u8 const *)"CS_SETUP_ACK_REQ",
    (u8 const *)"CS_CALL_PROC_REQ",
    (u8 const *)"CS_IWU_INFO_REQ",
    (u8 const *)"CS_CONNECT_REQ",
    (u8 const *)"CS_ALERT_REQ",
    (u8 const *)"CS_INFO_REQ",
    (u8 const *)"CS_RELEASE_REQ",
    (u8 const *)"CS_SER_CHNG_REQ",
    (u8 const *)"CS_SER_CHNG_RES",
    (u8 const *)"CSM_CC_NEW_CALL_ID",
    (u8 const *)"CM_CC_CALL_PROC_REQ",
    (u8 const *) 0
}
#endif
;

#endif






/* == process FTRMC: code= 9, nr.of inst.= 1, prio.= high ====== */


/* -- states: -- */
#define RMC_IDLE                                   0  	/* = 0x00 */
#define RMC_ACTIVE                                 1  	/* = 0x01 */
#define FTRMC_END                                  2  	/* = 0x02 */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_StateNames_9[]
#ifdef cos19_c
 = {
    (u8 const *)"RMC_IDLE",
    (u8 const *)"RMC_ACTIVE",
    (u8 const *)"FTRMC_END",
    (u8 const *) 0
}
#endif
;

#endif
/* -- events: -- */
#define WAKE_UP                                    0  	/* = 0x00 */
#define RSSI_CFM                                   1  	/* = 0x01 */
#define QUIET_CHAN_REQ                             2  	/* = 0x02 */
#define RSSI_TIMER_EXP                             3  	/* = 0x03 */
#define GO_SLEEP                                   4  	/* = 0x04 */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_EventNames_9[]
#ifdef cos19_c
 = {
    (u8 const *)"WAKE_UP",
    (u8 const *)"RSSI_CFM",
    (u8 const *)"QUIET_CHAN_REQ",
    (u8 const *)"RSSI_TIMER_EXP",
    (u8 const *)"GO_SLEEP",
    (u8 const *) 0
}
#endif
;

#endif



/* == process TRACE_PROCESS: code= 10, nr.of inst.= 1, prio.= high ====== */

 /* only to add symbolic trace info */
/* -- states: -- */
#define AUDIO_SPEECH_MODE                          0  	/* = 0x00 */
#define AUDIO_RINGER_MELODY                        1  	/* = 0x01 */
#define AUDIO_TONE_EAR                             2  	/* = 0x02 */
#define AUDIO_TONE_BUZ                             3  	/* = 0x03 */
#define AUDIO_VOL_EARPIECE                         4  	/* = 0x04 */
#define AUDIO_VOL_BUZZER                           5  	/* = 0x05 */
#define AUDIO_WAITFOR_DLT                          6  	/* = 0x06 */
#define AUDIO_DLT_DETECTED                         7  	/* = 0x07 */
#define AUDIO_STOP_DLTDETECT                       8  	/* = 0x08 */
#define AUDIO_MICRO                                9  	/* = 0x09 */
#define AUDIO_DIALDTMF                            10  	/* = 0x0a */
#define AUDIO_HANDSFREE                           11  	/* = 0x0b */
#define AUDIO_SWITCH_MUSIC_OFF                    12  	/* = 0x0c */
#define AUDIO_SWITCH_MUSIC_ON                     13  	/* = 0x0d */
#define AUDIO_SOFTMUTE                            14  	/* = 0x0e */
#define PSTN_LINE_ON_HOOK                         15  	/* = 0x0f */
#define PSTN_LINE_DIAL                            16  	/* = 0x10 */
#define PSTN_LINE_OFF_HOOK                        17  	/* = 0x11 */
#define AUDIO_MFCR_ENABLE                         18  	/* = 0x12 */
#define AUDIO_MFCR_DISABLE                        19  	/* = 0x13 */
#define AUDIO_DTAS_ENABLE                         20  	/* = 0x14 */
#define AUDIO_DTAS_DISABLE                        21  	/* = 0x15 */
#define AUDIO_FSK_RECEIVE                         22  	/* = 0x16 */
#define AUDIO_FSK_ABORT                           23  	/* = 0x17 */
#define CALL_STATE_CHANGE                         24  	/* = 0x18 */
#define LCD_TEXT                                  25  	/* = 0x19 */
#define LCD_ANTENNA                               26  	/* = 0x1a */
#define LCD_BAT_EMPTY                             27  	/* = 0x1b */
#define LCD_BAT_HALF                              28  	/* = 0x1c */
#define LCD_BAT_FULL                              29  	/* = 0x1d */
#define LCD_HOOK                                  30  	/* = 0x1e */
#define LCD_READY                                 31  	/* = 0x1f */
#define LCD_LOCKED                                32  	/* = 0x20 */
#define LCD_DIRECTCALL                            33  	/* = 0x21 */
#define LCD_INTERN                                34  	/* = 0x22 */
#define LCD_DUMMY1                                35  	/* = 0x23 */
#define LCD_DUMMY2                                36  	/* = 0x24 */
#define LCD_DUMMY3                                37  	/* = 0x25 */
#define LCD_EXTERN                                38  	/* = 0x26 */
#define LCD_EXT_1                                 39  	/* = 0x27 */
#define LCD_EXT_2                                 40  	/* = 0x28 */
#define FCNT_VAL                                  41  	/* = 0x29 */
#define CALL_INFORMATION                          42  	/* = 0x2a */
#define MAX_NUMBER_OF_STATES_FOR_TRACE_PROCESS    43  	/* = 0x2b */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_StateNames_10[]
#ifdef cos19_c
 = {
    (u8 const *)"AUDIO_SPEECH_MODE",
    (u8 const *)"AUDIO_RINGER_MELODY",
    (u8 const *)"AUDIO_TONE_EAR",
    (u8 const *)"AUDIO_TONE_BUZ",
    (u8 const *)"AUDIO_VOL_EARPIECE",
    (u8 const *)"AUDIO_VOL_BUZZER",
    (u8 const *)"AUDIO_WAITFOR_DLT",
    (u8 const *)"AUDIO_DLT_DETECTED",
    (u8 const *)"AUDIO_STOP_DLTDETECT",
    (u8 const *)"AUDIO_MICRO",
    (u8 const *)"AUDIO_DIALDTMF",
    (u8 const *)"AUDIO_HANDSFREE",
    (u8 const *)"AUDIO_SWITCH_MUSIC_OFF",
    (u8 const *)"AUDIO_SWITCH_MUSIC_ON",
    (u8 const *)"AUDIO_SOFTMUTE",
    (u8 const *)"PSTN_LINE_ON_HOOK",
    (u8 const *)"PSTN_LINE_DIAL",
    (u8 const *)"PSTN_LINE_OFF_HOOK",
    (u8 const *)"AUDIO_MFCR_ENABLE",
    (u8 const *)"AUDIO_MFCR_DISABLE",
    (u8 const *)"AUDIO_DTAS_ENABLE",
    (u8 const *)"AUDIO_DTAS_DISABLE",
    (u8 const *)"AUDIO_FSK_RECEIVE",
    (u8 const *)"AUDIO_FSK_ABORT",
    (u8 const *)"CALL_STATE_CHANGE",
    (u8 const *)"LCD_TEXT",
    (u8 const *)"LCD_ANTENNA",
    (u8 const *)"LCD_BAT_EMPTY",
    (u8 const *)"LCD_BAT_HALF",
    (u8 const *)"LCD_BAT_FULL",
    (u8 const *)"LCD_HOOK",
    (u8 const *)"LCD_READY",
    (u8 const *)"LCD_LOCKED",
    (u8 const *)"LCD_DIRECTCALL",
    (u8 const *)"LCD_INTERN",
    (u8 const *)"LCD_DUMMY1",
    (u8 const *)"LCD_DUMMY2",
    (u8 const *)"LCD_DUMMY3",
    (u8 const *)"LCD_EXTERN",
    (u8 const *)"LCD_EXT_1",
    (u8 const *)"LCD_EXT_2",
    (u8 const *)"FCNT_VAL",
    (u8 const *)"CALL_INFORMATION",
    (u8 const *)"MAX_NUMBER_OF_STATES_FOR_TRACE_PROCESS",
    (u8 const *) 0
}
#endif
;

#endif
/* -- events: -- */
#define OFF_                                       0  	/* = 0x00 */
#define ON_                                        1  	/* = 0x01 */
#define LINE_1                                     2  	/* = 0x02 */
#define LINE_2                                     3  	/* = 0x03 */
#define BEARER_RESPONSE_OK_1                       4  	/* = 0x04 */
#define BEARER_RESPONSE_OK_2                       5  	/* = 0x05 */
#define BEARER_RESPONSE_OK_3                       6  	/* = 0x06 */
#define BEARER_RESPONSE_OK_4                       7  	/* = 0x07 */
#define BEARER_RESPONSE_BAD_1                      8  	/* = 0x08 */
#define BEARER_RESPONSE_BAD_2                      9  	/* = 0x09 */
#define BEARER_RESPONSE_BAD_3                     10  	/* = 0x0a */
#define BEARER_RESPONSE_BAD_4                     11  	/* = 0x0b */
#define BEARER_RESPONSE_BAD_5                     12  	/* = 0x0c */
#define BEARER_RESPONSE_BAD_6                     13  	/* = 0x0d */
#define BEARER_INFO                               14  	/* = 0x0e */
#define FORCE_RSSI                                15  	/* = 0x0f */
#define START_RSSI                                16  	/* = 0x10 */
#define REQUEST_CHANNEL_TRACE                     17  	/* = 0x11 */
#define REMOVE_US_DECT_DUMMY                      18  	/* = 0x12 */
#define GOT_CHANNEL_TRACE                         19  	/* = 0x13 */
#define SETUP_2ND_DUMMY_TRACE                     20  	/* = 0x14 */
#define CHECK_DUMMY_TRACE                         21  	/* = 0x15 */
#define BAD_DUMMY_RSSI_TRACE                      22  	/* = 0x16 */
#define RELEASE_1ST_DUMMY_TRACE                   23  	/* = 0x17 */
#define TRAF_SWITCHED_TO_DUMMY                    24  	/* = 0x18 */
#define CALL_STATE_CC_INST                        25  	/* = 0x19 */
#define CALL_STATE_VALUE                          26  	/* = 0x1a */
#define CALL_NUMBER                               27  	/* = 0x1b */
#define CALL_INFO_VALUES                          28  	/* = 0x1c */
#define MAX_NUMBER_OF_EVENTS_FOR_TRACE_PROCESS    29  	/* = 0x1d */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_EventNames_10[]
#ifdef cos19_c
 = {
    (u8 const *)"OFF_",
    (u8 const *)"ON_",
    (u8 const *)"LINE_1",
    (u8 const *)"LINE_2",
    (u8 const *)"BEARER_RESPONSE_OK_1",
    (u8 const *)"BEARER_RESPONSE_OK_2",
    (u8 const *)"BEARER_RESPONSE_OK_3",
    (u8 const *)"BEARER_RESPONSE_OK_4",
    (u8 const *)"BEARER_RESPONSE_BAD_1",
    (u8 const *)"BEARER_RESPONSE_BAD_2",
    (u8 const *)"BEARER_RESPONSE_BAD_3",
    (u8 const *)"BEARER_RESPONSE_BAD_4",
    (u8 const *)"BEARER_RESPONSE_BAD_5",
    (u8 const *)"BEARER_RESPONSE_BAD_6",
    (u8 const *)"BEARER_INFO",
    (u8 const *)"FORCE_RSSI",
    (u8 const *)"START_RSSI",
    (u8 const *)"REQUEST_CHANNEL_TRACE",
    (u8 const *)"REMOVE_US_DECT_DUMMY",
    (u8 const *)"GOT_CHANNEL_TRACE",
    (u8 const *)"SETUP_2ND_DUMMY_TRACE",
    (u8 const *)"CHECK_DUMMY_TRACE",
    (u8 const *)"BAD_DUMMY_RSSI_TRACE",
    (u8 const *)"RELEASE_1ST_DUMMY_TRACE",
    (u8 const *)"TRAF_SWITCHED_TO_DUMMY",
    (u8 const *)"CALL_STATE_CC_INST",
    (u8 const *)"CALL_STATE_VALUE",
    (u8 const *)"CALL_NUMBER",
    (u8 const *)"CALL_INFO_VALUES",
    (u8 const *)"MAX_NUMBER_OF_EVENTS_FOR_TRACE_PROCESS",
    (u8 const *) 0
}
#endif
;

#endif

/* SET_NB_OF_INSTANCES  MAX(SD09_MAX_NUM_LINE_SESSIONS, SD09_MAX_CSM_LINE_SESSIONS) */
/* instances needed for some line session or CSM session related timer events in the MMI */




/* == process FTMI: code= 11, nr.of inst.= 10, prio.= high ====== */


/* -- states: -- */
#define FTMI_RUN                                   0  	/* = 0x00 */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_StateNames_11[]
#ifdef cos19_c
 = {
    (u8 const *)"FTMI_RUN",
    (u8 const *) 0
}
#endif
;

#endif
/* -- events: -- */
#define MI_DESUBSCRIBE_CFM                        16  	/* = 0x10 */
#define MI_ALERT_IND                              17  	/* = 0x11 */
#define MI_TONE_IND                               18  	/* = 0x12 */
#define MI_COST_INFO_REQ                          19  	/* = 0x13 */
#define MI_HANDSET_DIS_IND                        20  	/* = 0x14 */
#define MI_FEATURE_RES                            21  	/* = 0x15 */
#define MI_WAKEUP_IND                             22  	/* = 0x16 */
#define PS_RING_CYCLE_EXP                         23  	/* = 0x17 */
#define PS_CALL_IND                               24  	/* = 0x18 */
#define PS_ANSWER_IND                             25  	/* = 0x19 */
#define PS_RELEASE_IND                            26  	/* = 0x1a */
#define PS_SWITCH_MUSIC_IND                       27  	/* = 0x1b */
#define PS_DIAL_IND                               28  	/* = 0x1c */
#define PS_INC_EXPIRED                            29  	/* = 0x1d */
#define EXT_CALL_IND                              30  	/* = 0x1e */
#define EXT_CALL_CFM                              31  	/* = 0x1f */
#define MI_INC_CALL_EXP                           32  	/* = 0x20 */
#define PS_MFCR_IND                               33  	/* = 0x21 */
#define PS_CLIP_IND                               34  	/* = 0x22 */
#define MI_SMSC_RECOGNISED                        35  	/* = 0x23 */
#define PS_CDS_IND                                36  	/* = 0x24 */
#define PS_RINGER_ON_IND                          37  	/* = 0x25 */
#define PS_RINGER_OFF_IND                         38  	/* = 0x26 */
#define PS_LINE_CURRENT_EXC                       39  	/* = 0x27 */
#define PR_PROP_CONN_IND                          40  	/* = 0x28 */
#define PR_PROP_KEY_IND                           41  	/* = 0x29 */
#define PR_PROP_RELEASE_IND                       42  	/* = 0x2a */
#define PR_PROP_CONN_CFM                          43  	/* = 0x2b */
#define PR_PROP_RELEASE_CFM                       44  	/* = 0x2c */
#define FTMI_MM_WATCHDOG_EXP                      45  	/* = 0x2d */
#define FTMI_RINGER_ON_EXP                        46  	/* = 0x2e */
#define FTMI_MENU_PREP_EXP                        47  	/* = 0x2f */
#define FTMI_DTAM_EXP                             48  	/* = 0x30 */
#define FTMI_GO_NEXT                              49  	/* = 0x31 */
#define FTMI_SMS_IND                              50  	/* = 0x32 */
#define PS_INC_SMS_IND                            51  	/* = 0x33 */
#define MMI_SMS_SEND_FAIL_IND                     52  	/* = 0x34 */
#define MMI_SMS_SEND_DONE_IND                     53  	/* = 0x35 */
#define MMI_SMS_RECV_IND                          54  	/* = 0x36 */
#define MMI_MMS_ESC_CMD_IND                       55  	/* = 0x37 */
#define MMI_SMS_RETRANS_TMR                       56  	/* = 0x38 */
#define MMI_SMS_PAGE_IND                          57  	/* = 0x39 */
#define PS_SMS_RING_CYCLE_EXP                     58  	/* = 0x3a */
#define MMI_SMS_SETUP_IND                         59  	/* = 0x3b */
#define MMI_SMS_RELEASE_IND                       60  	/* = 0x3c */
#define MMI_SMS_RELEASE_CFM                       61  	/* = 0x3d */
#define MI_DAEV_DSP_NEW_MODE_ACTIVE               62  	/* = 0x3e */
#define MI_DAMEV_SELECT_IND                       63  	/* = 0x3f */
#define MI_DAMEV_MSG_NOT_FOUND                    64  	/* = 0x40 */
#define MI_DAMEV_READY                            65  	/* = 0x41 */
#define MI_DAMEV_PAUSE                            66  	/* = 0x42 */
#define MI_DAMEV_RECORD                           67  	/* = 0x43 */
#define MI_DAMEV_DELETE_IND                       68  	/* = 0x44 */
#define MI_DAMEV_MEMFULL                          69  	/* = 0x45 */
#define MI_DAMEV_PLAY                             70  	/* = 0x46 */
#define MI_DAMEV_COUNT_IND                        71  	/* = 0x47 */
#define MI_DAMEV_MSG_END                          72  	/* = 0x48 */
#define MI_DAMEV_STOP                             73  	/* = 0x49 */
#define MI_DAMEV_CHANGEID_IND                     74  	/* = 0x4a */
#define MI_DAMEV_TRUNC_IND                        75  	/* = 0x4b */
#define MI_PILL_FILE_CREATE_IND                   76  	/* = 0x4c */
#define MI_PILL_FILE_OPEN_IND                     77  	/* = 0x4d */
#define MI_PILL_FILE_NOT_FOUND                    78  	/* = 0x4e */
#define MI_PILL_FILE_CLOSE_IND                    79  	/* = 0x4f */
#define MI_PILL_FILE_DELETE_IND                   80  	/* = 0x50 */
#define MI_PILL_FILE_WRITE_IND                    81  	/* = 0x51 */
#define MI_DASEV_TALK                             82  	/* = 0x52 */
#define MI_DASEV_TALK_END                         83  	/* = 0x53 */
#define MI_DASEV_STOP                             84  	/* = 0x54 */
#define MI_CLEAN_UP_DTAM                          85  	/* = 0x55 */
#define MI_BUSYCOUNT_EXP                          86  	/* = 0x56 */
#define MI_DTAM_CONTROL                           87  	/* = 0x57 */
#define MI_DTAM_CONTROL_END                       88  	/* = 0x58 */
#define MI_BUSY                                   89  	/* = 0x59 */
#define MI_NOBUSY                                 90  	/* = 0x5a */
#define MI_START_DTAM                             91  	/* = 0x5b */
#define MI_DTAM_KEY_IND                           92  	/* = 0x5c */
#define MI_DTAM_START_REMOTE                      93  	/* = 0x5d */
#define MI_DTAM_STOP_REMOTE                       94  	/* = 0x5e */
#define MI_DTAM_REL_IND                           95  	/* = 0x5f */
#define MI_RAC_STOP_REMOTE                        96  	/* = 0x60 */
#define MI_SPR_TIMER                              97  	/* = 0x61 */
#define SA_WORD_DEL_IND                           98  	/* = 0x62 */
#define SA_ERROR_IND                              99  	/* = 0x63 */
#define SA_TR_FIRST_IND                          100  	/* = 0x64 */
#define SA_INIT_OK_IND                           101  	/* = 0x65 */
#define SA_NOT_RECOGN_IND                        102  	/* = 0x66 */
#define SA_TR_SECOND_IND                         103  	/* = 0x67 */
#define SA_VOC_SELECTED_IND                      104  	/* = 0x68 */
#define SA_WORD_RECOGN_IND                       105  	/* = 0x69 */
#define SA_TO_IDLE_IND                           106  	/* = 0x6a */
#define SA_SIMILAR_WORD_IND                      107  	/* = 0x6b */
#define SA_START_REC_REQ                         108  	/* = 0x6c */
#define SA_STOP_REC_REQ                          109  	/* = 0x6d */
#define MI_END_OF_DTAM_CALL                      110  	/* = 0x6e */
#define MI_DATEV_MELO_END                        111  	/* = 0x6f */
#define MI_RETURN_VAL_ERR                        112  	/* = 0x70 */
#define MI_CONFERENCE_IND                        113  	/* = 0x71 */
#define MI_CONF_RELEASE_IND                      114  	/* = 0x72 */
#define MI_TRANSFER_RELEASE_IND                  115  	/* = 0x73 */
#define MI_TRANSFER_ALERT_IND                    116  	/* = 0x74 */
#define MI_TRANSFER_ANSWERED_IND                 117  	/* = 0x75 */
#define MI_INTERNAL_CONNECT_IND                  118  	/* = 0x76 */
#define MI_INTERNAL_ALERT_IND                    119  	/* = 0x77 */
#define MI_INTERNAL_RELEASE_IND                  120  	/* = 0x78 */
#define PS_INC_SMS_REQ                           121  	/* = 0x79 */
#define PS_OUT_SMS_REQ                           122  	/* = 0x7a */
#define MI_SMS_REL_REQ                           123  	/* = 0x7b */
#define MI_SMS_DIAL_EXP                          124  	/* = 0x7c */
#define MI_DIAL_END_IND                          125  	/* = 0x7d */
#define MI_IWU_INFO_IND                          126  	/* = 0x7e */
#define MI_MULTI_IWU_INFO_IND                    127  	/* = 0x7f */
#define MI_HOLD_LINE_IND                         128  	/* = 0x80 */
#define MI_PARK_LINE_IND                         129  	/* = 0x81 */
#define MI_CONNECT_IND                           130  	/* = 0x82 */
#define MI_TRANSFER_SETUP_IND                    131  	/* = 0x83 */
#define MI_CALL_BACK1_EXP                        132  	/* = 0x84 */
#define MI_CALL_BACK2_EXP                        133  	/* = 0x85 */
#define MI_TRANSFER_CANCELLED_IND                134  	/* = 0x86 */
#define MI_LEFT_LINE_PARKED_IND                  135  	/* = 0x87 */
#define MI_LINE_CONNECT_IND                      136  	/* = 0x88 */
#define MI_MM_IWU_INFO_IND                       137  	/* = 0x89 */
#define MI_LOCAL_TONE_REQ                        138  	/* = 0x8a */
#define MI_PRE_OVERLAP_SENDING_EXP               139  	/* = 0x8b */
#define MI_OVERLAP_SENDING_EXP                   140  	/* = 0x8c */
#define MI_CALL_INFO_IND                         141  	/* = 0x8d */
#define MI_KEYPAD_IND                            142  	/* = 0x8e */
#define DR_KEYPAD_IND                            143  	/* = 0x8f */
#define DR_KEYPAD_RELEASE_IND                    144  	/* = 0x90 */
#define MI_DH_DATA_CALL_RDY                      145  	/* = 0x91 */
#define MI_DH_DATA_IND                           146  	/* = 0x92 */
#define MI_DH_DATA_CALL_REL                      147  	/* = 0x93 */
#define MI_DH_BW_MODIFY_CMD                      148  	/* = 0x94 */
#define MI_DH_MORE_DATA_REQ                      149  	/* = 0x95 */
#define MI_DH_TIMER1_EXP                         150  	/* = 0x96 */
#define MI_DH_TIMER2_EXP                         151  	/* = 0x97 */
#define MI_DH_TIMER3_EXP                         152  	/* = 0x98 */
#define MI_DH_TIMER4_EXP                         153  	/* = 0x99 */
#define MI_DATA_MODE_IND                         154  	/* = 0x9a */
#define MI_LUX_SDU_RCVD                          155  	/* = 0x9b */
#define MI_LUX_SDU_SENT                          156  	/* = 0x9c */
#define MI_LUX_SDU_SEND_ERR                      157  	/* = 0x9d */
#define MI_LUX_SESSION_CLOSED                    158  	/* = 0x9e */
#define APP_NEMO_PEND                            159  	/* = 0x9f */
#define APP_NEMO_IND                             160  	/* = 0xa0 */
#define APP_NEMO_STOP                            161  	/* = 0xa1 */
#define APP_NEMO_WAKEUP                          162  	/* = 0xa2 */
#define SIN_FAIL_IND                             163  	/* = 0xa3 */
#define SIN_PENDING_IND                          164  	/* = 0xa4 */
#define SIN_SETUP_CFM                            165  	/* = 0xa5 */
#define SIN_STOP_CFM                             166  	/* = 0xa6 */
#define MI_MAS_READY                             167  	/* = 0xa7 */
#define MI_MAS_ARM_REQ                           168  	/* = 0xa8 */
#define MI_MAS_ARMBG_REQ                         169  	/* = 0xa9 */
#define MI_MAS_REAL_REQ                          170  	/* = 0xaa */
#define MI_MAS_ARM_IDLE_REQ                      171  	/* = 0xab */
#define MI_MAS_REAL_IDLE_REQ                     172  	/* = 0xac */
#define MI_SCORE_SELECT_IND                      173  	/* = 0xad */
#define MI_SCORE_PLAY_IND                        174  	/* = 0xae */
#define MI_SCORE_STOP_IND                        175  	/* = 0xaf */
#define MI_SCORE_PREPARED                        176  	/* = 0xb0 */
#define MI_SCORE_NOT_FOUND                       177  	/* = 0xb1 */
#define MI_SCORE_TYPE                            178  	/* = 0xb2 */
#define MI_SCORE_FILE_ERRORFTMI                  179  	/* = 0xb3 */
#define MI_SCORE_DATALATE                        180  	/* = 0xb4 */
#define MI_SCORE_LIMIT                           181  	/* = 0xb5 */
#define MI_SCORE_TEMPO                           182  	/* = 0xb6 */
#define MI_STOP_POLYPHONIC                       183  	/* = 0xb7 */
#define MI_MCV                                   184  	/* = 0xb8 */
#define MI_MCV_MThd                              185  	/* = 0xb9 */
#define MI_MCV_MTrk                              186  	/* = 0xba */
#define MI_MCV_MEvent                            187  	/* = 0xbb */
#define MI_MCV_BUFSZ                             188  	/* = 0xbc */
#define MI_MCV_FILMEM                            189  	/* = 0xbd */
#define MI_MCV_FILE_NOT_FOUND                    190  	/* = 0xbe */
#define MI_MCVIO                                 191  	/* = 0xbf */
#define MI_MCVIO_CLOSE                           192  	/* = 0xc0 */
#define MI_MCVIO_RENAME                          193  	/* = 0xc1 */
#define MI_MCVIO_DELETE                          194  	/* = 0xc2 */
#define MI_MCVIO_WRITE                           195  	/* = 0xc3 */
#define MI_MCVIO_READ                            196  	/* = 0xc4 */
#define MI_MCVIO_SEEK                            197  	/* = 0xc5 */
#define MI_MCVIO_CREATE                          198  	/* = 0xc6 */
#define MI_MCVIO_OPEN                            199  	/* = 0xc7 */
#define MI_ENCRYPTED_CALL_IND                    200  	/* = 0xc8 */
#define MI_ENCRYPT_REJECT_IND                    201  	/* = 0xc9 */
#define MI_CALL_REINJECT_IND                     202  	/* = 0xca */
#define MI_CALL_FORWARD_REQ                      203  	/* = 0xcb */
#define MI_CALL_ALERT_IND                        204  	/* = 0xcc */
#define MI_NO_RPAS_RING_EXP                      205  	/* = 0xcd */
#define MI_HS_CALL_REJECT_IND                    206  	/* = 0xce */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_EventNames_11[]
#ifdef cos19_c
 = {
    (u8 const *)"ILLEGAL_EVENT",
    (u8 const *)"ILLEGAL_EVENT",
    (u8 const *)"ILLEGAL_EVENT",
    (u8 const *)"ILLEGAL_EVENT",
    (u8 const *)"ILLEGAL_EVENT",
    (u8 const *)"ILLEGAL_EVENT",
    (u8 const *)"ILLEGAL_EVENT",
    (u8 const *)"ILLEGAL_EVENT",
    (u8 const *)"ILLEGAL_EVENT",
    (u8 const *)"ILLEGAL_EVENT",
    (u8 const *)"ILLEGAL_EVENT",
    (u8 const *)"ILLEGAL_EVENT",
    (u8 const *)"ILLEGAL_EVENT",
    (u8 const *)"ILLEGAL_EVENT",
    (u8 const *)"ILLEGAL_EVENT",
    (u8 const *)"ILLEGAL_EVENT",
    (u8 const *)"MI_DESUBSCRIBE_CFM",
    (u8 const *)"MI_ALERT_IND",
    (u8 const *)"MI_TONE_IND",
    (u8 const *)"MI_COST_INFO_REQ",
    (u8 const *)"MI_HANDSET_DIS_IND",
    (u8 const *)"MI_FEATURE_RES",
    (u8 const *)"MI_WAKEUP_IND",
    (u8 const *)"PS_RING_CYCLE_EXP",
    (u8 const *)"PS_CALL_IND",
    (u8 const *)"PS_ANSWER_IND",
    (u8 const *)"PS_RELEASE_IND",
    (u8 const *)"PS_SWITCH_MUSIC_IND",
    (u8 const *)"PS_DIAL_IND",
    (u8 const *)"PS_INC_EXPIRED",
    (u8 const *)"EXT_CALL_IND",
    (u8 const *)"EXT_CALL_CFM",
    (u8 const *)"MI_INC_CALL_EXP",
    (u8 const *)"PS_MFCR_IND",
    (u8 const *)"PS_CLIP_IND",
    (u8 const *)"MI_SMSC_RECOGNISED",
    (u8 const *)"PS_CDS_IND",
    (u8 const *)"PS_RINGER_ON_IND",
    (u8 const *)"PS_RINGER_OFF_IND",
    (u8 const *)"PS_LINE_CURRENT_EXC",
    (u8 const *)"PR_PROP_CONN_IND",
    (u8 const *)"PR_PROP_KEY_IND",
    (u8 const *)"PR_PROP_RELEASE_IND",
    (u8 const *)"PR_PROP_CONN_CFM",
    (u8 const *)"PR_PROP_RELEASE_CFM",
    (u8 const *)"FTMI_MM_WATCHDOG_EXP",
    (u8 const *)"FTMI_RINGER_ON_EXP",
    (u8 const *)"FTMI_MENU_PREP_EXP",
    (u8 const *)"FTMI_DTAM_EXP",
    (u8 const *)"FTMI_GO_NEXT",
    (u8 const *)"FTMI_SMS_IND",
    (u8 const *)"PS_INC_SMS_IND",
    (u8 const *)"MMI_SMS_SEND_FAIL_IND",
    (u8 const *)"MMI_SMS_SEND_DONE_IND",
    (u8 const *)"MMI_SMS_RECV_IND",
    (u8 const *)"MMI_MMS_ESC_CMD_IND",
    (u8 const *)"MMI_SMS_RETRANS_TMR",
    (u8 const *)"MMI_SMS_PAGE_IND",
    (u8 const *)"PS_SMS_RING_CYCLE_EXP",
    (u8 const *)"MMI_SMS_SETUP_IND",
    (u8 const *)"MMI_SMS_RELEASE_IND",
    (u8 const *)"MMI_SMS_RELEASE_CFM",
    (u8 const *)"MI_DAEV_DSP_NEW_MODE_ACTIVE",
    (u8 const *)"MI_DAMEV_SELECT_IND",
    (u8 const *)"MI_DAMEV_MSG_NOT_FOUND",
    (u8 const *)"MI_DAMEV_READY",
    (u8 const *)"MI_DAMEV_PAUSE",
    (u8 const *)"MI_DAMEV_RECORD",
    (u8 const *)"MI_DAMEV_DELETE_IND",
    (u8 const *)"MI_DAMEV_MEMFULL",
    (u8 const *)"MI_DAMEV_PLAY",
    (u8 const *)"MI_DAMEV_COUNT_IND",
    (u8 const *)"MI_DAMEV_MSG_END",
    (u8 const *)"MI_DAMEV_STOP",
    (u8 const *)"MI_DAMEV_CHANGEID_IND",
    (u8 const *)"MI_DAMEV_TRUNC_IND",
    (u8 const *)"MI_PILL_FILE_CREATE_IND",
    (u8 const *)"MI_PILL_FILE_OPEN_IND",
    (u8 const *)"MI_PILL_FILE_NOT_FOUND",
    (u8 const *)"MI_PILL_FILE_CLOSE_IND",
    (u8 const *)"MI_PILL_FILE_DELETE_IND",
    (u8 const *)"MI_PILL_FILE_WRITE_IND",
    (u8 const *)"MI_DASEV_TALK",
    (u8 const *)"MI_DASEV_TALK_END",
    (u8 const *)"MI_DASEV_STOP",
    (u8 const *)"MI_CLEAN_UP_DTAM",
    (u8 const *)"MI_BUSYCOUNT_EXP",
    (u8 const *)"MI_DTAM_CONTROL",
    (u8 const *)"MI_DTAM_CONTROL_END",
    (u8 const *)"MI_BUSY",
    (u8 const *)"MI_NOBUSY",
    (u8 const *)"MI_START_DTAM",
    (u8 const *)"MI_DTAM_KEY_IND",
    (u8 const *)"MI_DTAM_START_REMOTE",
    (u8 const *)"MI_DTAM_STOP_REMOTE",
    (u8 const *)"MI_DTAM_REL_IND",
    (u8 const *)"MI_RAC_STOP_REMOTE",
    (u8 const *)"MI_SPR_TIMER",
    (u8 const *)"SA_WORD_DEL_IND",
    (u8 const *)"SA_ERROR_IND",
    (u8 const *)"SA_TR_FIRST_IND",
    (u8 const *)"SA_INIT_OK_IND",
    (u8 const *)"SA_NOT_RECOGN_IND",
    (u8 const *)"SA_TR_SECOND_IND",
    (u8 const *)"SA_VOC_SELECTED_IND",
    (u8 const *)"SA_WORD_RECOGN_IND",
    (u8 const *)"SA_TO_IDLE_IND",
    (u8 const *)"SA_SIMILAR_WORD_IND",
    (u8 const *)"SA_START_REC_REQ",
    (u8 const *)"SA_STOP_REC_REQ",
    (u8 const *)"MI_END_OF_DTAM_CALL",
    (u8 const *)"MI_DATEV_MELO_END",
    (u8 const *)"MI_RETURN_VAL_ERR",
    (u8 const *)"MI_CONFERENCE_IND",
    (u8 const *)"MI_CONF_RELEASE_IND",
    (u8 const *)"MI_TRANSFER_RELEASE_IND",
    (u8 const *)"MI_TRANSFER_ALERT_IND",
    (u8 const *)"MI_TRANSFER_ANSWERED_IND",
    (u8 const *)"MI_INTERNAL_CONNECT_IND",
    (u8 const *)"MI_INTERNAL_ALERT_IND",
    (u8 const *)"MI_INTERNAL_RELEASE_IND",
    (u8 const *)"PS_INC_SMS_REQ",
    (u8 const *)"PS_OUT_SMS_REQ",
    (u8 const *)"MI_SMS_REL_REQ",
    (u8 const *)"MI_SMS_DIAL_EXP",
    (u8 const *)"MI_DIAL_END_IND",
    (u8 const *)"MI_IWU_INFO_IND",
    (u8 const *)"MI_MULTI_IWU_INFO_IND",
    (u8 const *)"MI_HOLD_LINE_IND",
    (u8 const *)"MI_PARK_LINE_IND",
    (u8 const *)"MI_CONNECT_IND",
    (u8 const *)"MI_TRANSFER_SETUP_IND",
    (u8 const *)"MI_CALL_BACK1_EXP",
    (u8 const *)"MI_CALL_BACK2_EXP",
    (u8 const *)"MI_TRANSFER_CANCELLED_IND",
    (u8 const *)"MI_LEFT_LINE_PARKED_IND",
    (u8 const *)"MI_LINE_CONNECT_IND",
    (u8 const *)"MI_MM_IWU_INFO_IND",
    (u8 const *)"MI_LOCAL_TONE_REQ",
    (u8 const *)"MI_PRE_OVERLAP_SENDING_EXP",
    (u8 const *)"MI_OVERLAP_SENDING_EXP",
    (u8 const *)"MI_CALL_INFO_IND",
    (u8 const *)"MI_KEYPAD_IND",
    (u8 const *)"DR_KEYPAD_IND",
    (u8 const *)"DR_KEYPAD_RELEASE_IND",
    (u8 const *)"MI_DH_DATA_CALL_RDY",
    (u8 const *)"MI_DH_DATA_IND",
    (u8 const *)"MI_DH_DATA_CALL_REL",
    (u8 const *)"MI_DH_BW_MODIFY_CMD",
    (u8 const *)"MI_DH_MORE_DATA_REQ",
    (u8 const *)"MI_DH_TIMER1_EXP",
    (u8 const *)"MI_DH_TIMER2_EXP",
    (u8 const *)"MI_DH_TIMER3_EXP",
    (u8 const *)"MI_DH_TIMER4_EXP",
    (u8 const *)"MI_DATA_MODE_IND",
    (u8 const *)"MI_LUX_SDU_RCVD",
    (u8 const *)"MI_LUX_SDU_SENT",
    (u8 const *)"MI_LUX_SDU_SEND_ERR",
    (u8 const *)"MI_LUX_SESSION_CLOSED",
    (u8 const *)"APP_NEMO_PEND",
    (u8 const *)"APP_NEMO_IND",
    (u8 const *)"APP_NEMO_STOP",
    (u8 const *)"APP_NEMO_WAKEUP",
    (u8 const *)"SIN_FAIL_IND",
    (u8 const *)"SIN_PENDING_IND",
    (u8 const *)"SIN_SETUP_CFM",
    (u8 const *)"SIN_STOP_CFM",
    (u8 const *)"MI_MAS_READY",
    (u8 const *)"MI_MAS_ARM_REQ",
    (u8 const *)"MI_MAS_ARMBG_REQ",
    (u8 const *)"MI_MAS_REAL_REQ",
    (u8 const *)"MI_MAS_ARM_IDLE_REQ",
    (u8 const *)"MI_MAS_REAL_IDLE_REQ",
    (u8 const *)"MI_SCORE_SELECT_IND",
    (u8 const *)"MI_SCORE_PLAY_IND",
    (u8 const *)"MI_SCORE_STOP_IND",
    (u8 const *)"MI_SCORE_PREPARED",
    (u8 const *)"MI_SCORE_NOT_FOUND",
    (u8 const *)"MI_SCORE_TYPE",
    (u8 const *)"MI_SCORE_FILE_ERRORFTMI",
    (u8 const *)"MI_SCORE_DATALATE",
    (u8 const *)"MI_SCORE_LIMIT",
    (u8 const *)"MI_SCORE_TEMPO",
    (u8 const *)"MI_STOP_POLYPHONIC",
    (u8 const *)"MI_MCV",
    (u8 const *)"MI_MCV_MThd",
    (u8 const *)"MI_MCV_MTrk",
    (u8 const *)"MI_MCV_MEvent",
    (u8 const *)"MI_MCV_BUFSZ",
    (u8 const *)"MI_MCV_FILMEM",
    (u8 const *)"MI_MCV_FILE_NOT_FOUND",
    (u8 const *)"MI_MCVIO",
    (u8 const *)"MI_MCVIO_CLOSE",
    (u8 const *)"MI_MCVIO_RENAME",
    (u8 const *)"MI_MCVIO_DELETE",
    (u8 const *)"MI_MCVIO_WRITE",
    (u8 const *)"MI_MCVIO_READ",
    (u8 const *)"MI_MCVIO_SEEK",
    (u8 const *)"MI_MCVIO_CREATE",
    (u8 const *)"MI_MCVIO_OPEN",
    (u8 const *)"MI_ENCRYPTED_CALL_IND",
    (u8 const *)"MI_ENCRYPT_REJECT_IND",
    (u8 const *)"MI_CALL_REINJECT_IND",
    (u8 const *)"MI_CALL_FORWARD_REQ",
    (u8 const *)"MI_CALL_ALERT_IND",
    (u8 const *)"MI_NO_RPAS_RING_EXP",
    (u8 const *)"MI_HS_CALL_REJECT_IND",
    (u8 const *) 0
}
#endif
;

#endif

 /* make sure the next processes don't have more         */
                      /* instances than needed                                */




/* == process CSA: code= 12, nr.of inst.= 1, prio.= high ====== */


/* -- states: -- */
#define CSA_RUN                                    0  	/* = 0x00 */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_StateNames_12[]
#ifdef cos19_c
 = {
    (u8 const *)"CSA_RUN",
    (u8 const *) 0
}
#endif
;

#endif
/* -- events: -- */
#define SA_TIMER                                   0  	/* = 0x00 */
#define SA_RELEASE_IND                             1  	/* = 0x01 */
#define MI_SPR_VOC_SEL_REQ                         2  	/* = 0x02 */
#define MI_SPR_START_TR_REQ                        3  	/* = 0x03 */
#define MI_SPR_UTT1_START_REQ                      4  	/* = 0x04 */
#define MI_SPR_UTT2_START_REQ                      5  	/* = 0x05 */
#define MI_SPR_START_RE_REQ                        6  	/* = 0x06 */
#define MI_SPR_STORE_REQ                           7  	/* = 0x07 */
#define MI_SPR_CANCEL_REQ                          8  	/* = 0x08 */
#define MI_SPR_DELETE_REQ                          9  	/* = 0x09 */
#define MI_SPR_START_RECORD_REQ                   10  	/* = 0x0a */
#define CB_DSPEV_SA_DSI_VAD_STATUS_INFO           11  	/* = 0x0b */
#define CB_DSPEV_TR_DSI_VAD_STATUS_INFO           12  	/* = 0x0c */
#define CB_DSPEV_TR_DSI_END_UTTERANCE_INFO        13  	/* = 0x0d */
#define CB_DSPEV_TR_DSI_ERR_MSG_INFO              14  	/* = 0x0e */
#define CB_DAEV_DSP_NEW_MODE_ACTIVE               15  	/* = 0x0f */
#define CB_DASEV_WORD_DELETED                     16  	/* = 0x10 */
#define CB_DSPEV_RE_DSI_NO_WORD_RECO_INFO         17  	/* = 0x11 */
#define CB_DSPEV_TR_DSI_FINISHED_INFO             18  	/* = 0x12 */
#define CB_DASEV_VOCABULARY_SELECTED              19  	/* = 0x13 */
#define CB_DSPEV_RE_DSI_OUTPUTLIST_INFO           20  	/* = 0x14 */
#define CB_DSPEV_SA_DSI_ERR_MSG_INFO              21  	/* = 0x15 */
#define CB_DSPEV_TR_DSI_KILLED_INFO               22  	/* = 0x16 */
#define CB_DSPEV_RE_DSI_KILLED_INFO               23  	/* = 0x17 */
#define CB_DASEV_SPR_ERROR                        24  	/* = 0x18 */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_EventNames_12[]
#ifdef cos19_c
 = {
    (u8 const *)"SA_TIMER",
    (u8 const *)"SA_RELEASE_IND",
    (u8 const *)"MI_SPR_VOC_SEL_REQ",
    (u8 const *)"MI_SPR_START_TR_REQ",
    (u8 const *)"MI_SPR_UTT1_START_REQ",
    (u8 const *)"MI_SPR_UTT2_START_REQ",
    (u8 const *)"MI_SPR_START_RE_REQ",
    (u8 const *)"MI_SPR_STORE_REQ",
    (u8 const *)"MI_SPR_CANCEL_REQ",
    (u8 const *)"MI_SPR_DELETE_REQ",
    (u8 const *)"MI_SPR_START_RECORD_REQ",
    (u8 const *)"CB_DSPEV_SA_DSI_VAD_STATUS_INFO",
    (u8 const *)"CB_DSPEV_TR_DSI_VAD_STATUS_INFO",
    (u8 const *)"CB_DSPEV_TR_DSI_END_UTTERANCE_INFO",
    (u8 const *)"CB_DSPEV_TR_DSI_ERR_MSG_INFO",
    (u8 const *)"CB_DAEV_DSP_NEW_MODE_ACTIVE",
    (u8 const *)"CB_DASEV_WORD_DELETED",
    (u8 const *)"CB_DSPEV_RE_DSI_NO_WORD_RECO_INFO",
    (u8 const *)"CB_DSPEV_TR_DSI_FINISHED_INFO",
    (u8 const *)"CB_DASEV_VOCABULARY_SELECTED",
    (u8 const *)"CB_DSPEV_RE_DSI_OUTPUTLIST_INFO",
    (u8 const *)"CB_DSPEV_SA_DSI_ERR_MSG_INFO",
    (u8 const *)"CB_DSPEV_TR_DSI_KILLED_INFO",
    (u8 const *)"CB_DSPEV_RE_DSI_KILLED_INFO",
    (u8 const *)"CB_DASEV_SPR_ERROR",
    (u8 const *) 0
}
#endif
;

#endif



/* == process FTMMS: code= 13, nr.of inst.= 1, prio.= high ====== */


/* -- states: -- */
#define FTMMS_RUN                                  0  	/* = 0x00 */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_StateNames_13[]
#ifdef cos19_c
 = {
    (u8 const *)"FTMMS_RUN",
    (u8 const *) 0
}
#endif
;

#endif
#if (VDSW_FEATURES & VDSW_SMS)
/* -- events: -- */
#define MMS_INSTANCE_CFM                           0  	/* = 0x00 */
#define MNCC_SETUP_IND                             1  	/* = 0x01 */
#define MNCC_CONNECT_IND                           2  	/* = 0x02 */
#define MNCC_RELEASE_IND                           3  	/* = 0x03 */
#define MNCC_RELEASE_CFM                           4  	/* = 0x04 */
#define MNCC_IWU_INFO_IND                          5  	/* = 0x05 */
#define MMS_PAGE_SMS_REQ                           6  	/* = 0x06 */
#define MMS_ESC_CMD_REQ                            7  	/* = 0x07 */
#define MMS_SEND_SMS_REQ                           8  	/* = 0x08 */
#define MS_INC_SMS_IND                             9  	/* = 0x09 */
#define MS_DLL_SMS_IND                            10  	/* = 0x0a */
#define MMS_ESC_CMD_IND                           11  	/* = 0x0b */
#define MS_SMS_END_IND                            12  	/* = 0x0c */
#define FTMS_INC_START                            13  	/* = 0x0d */
#define FTMS_SMS_EXP                              14  	/* = 0x0e */
#define FTMS_LINEREQ_EXP                          15  	/* = 0x0f */
#define FTMS_SMSC_REQ_EXP                         16  	/* = 0x10 */
#define FTMS_PAGE_EXP                             17  	/* = 0x11 */
#define FTMS_WAIT1_EXP                            18  	/* = 0x12 */
#define FTMS_WAIT3_EXP                            19  	/* = 0x13 */
#define FTMS_WAIT4_EXP                            20  	/* = 0x14 */
#define MMS_SMS_RECV_IND                          21  	/* = 0x15 */
#define MMS_GET_SMS_IND                           22  	/* = 0x16 */
#define MMS_RESEND_SMS_REQ                        23  	/* = 0x17 */
#define MMS_SEND_ESC_CMD_REQ                      24  	/* = 0x18 */
#define MMS_SEND_REQ                              25  	/* = 0x19 */
#define NEW_MODE_ACTIVE                           26  	/* = 0x1a */
#define MMS_RELEASE_SMS_REQ                       27  	/* = 0x1b */
#define MMI_FSK_SENT                              28  	/* = 0x1c */
#endif
#define SMS_PSTN_BREAK                            29  	/* = 0x1d */
#define MI_SMS_LINE_ACK                           30  	/* = 0x1e */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_EventNames_13[]
#ifdef cos19_c
 = {
    (u8 const *)"MMS_INSTANCE_CFM",
    (u8 const *)"MNCC_SETUP_IND",
    (u8 const *)"MNCC_CONNECT_IND",
    (u8 const *)"MNCC_RELEASE_IND",
    (u8 const *)"MNCC_RELEASE_CFM",
    (u8 const *)"MNCC_IWU_INFO_IND",
    (u8 const *)"MMS_PAGE_SMS_REQ",
    (u8 const *)"MMS_ESC_CMD_REQ",
    (u8 const *)"MMS_SEND_SMS_REQ",
    (u8 const *)"MS_INC_SMS_IND",
    (u8 const *)"MS_DLL_SMS_IND",
    (u8 const *)"MMS_ESC_CMD_IND",
    (u8 const *)"MS_SMS_END_IND",
    (u8 const *)"FTMS_INC_START",
    (u8 const *)"FTMS_SMS_EXP",
    (u8 const *)"FTMS_LINEREQ_EXP",
    (u8 const *)"FTMS_SMSC_REQ_EXP",
    (u8 const *)"FTMS_PAGE_EXP",
    (u8 const *)"FTMS_WAIT1_EXP",
    (u8 const *)"FTMS_WAIT3_EXP",
    (u8 const *)"FTMS_WAIT4_EXP",
    (u8 const *)"MMS_SMS_RECV_IND",
    (u8 const *)"MMS_GET_SMS_IND",
    (u8 const *)"MMS_RESEND_SMS_REQ",
    (u8 const *)"MMS_SEND_ESC_CMD_REQ",
    (u8 const *)"MMS_SEND_REQ",
    (u8 const *)"NEW_MODE_ACTIVE",
    (u8 const *)"MMS_RELEASE_SMS_REQ",
    (u8 const *)"MMI_FSK_SENT",
    (u8 const *)"SMS_PSTN_BREAK",
    (u8 const *)"MI_SMS_LINE_ACK",
    (u8 const *) 0
}
#endif
;

#endif



/* == process FTCLI: code= 14, nr.of inst.= 1, prio.= high ====== */


/* -- states: -- */
#define FTCLI_RUN                                  0  	/* = 0x00 */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_StateNames_14[]
#ifdef cos19_c
 = {
    (u8 const *)"FTCLI_RUN",
    (u8 const *) 0
}
#endif
;

#endif
/* -- events: -- */
#define CLI_CDS_IND                                0  	/* = 0x00 */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_EventNames_14[]
#ifdef cos19_c
 = {
    (u8 const *)"CLI_CDS_IND",
    (u8 const *) 0
}
#endif
;

#endif



/* == process FTLA: code= 15, nr.of inst.= 1, prio.= high ====== */


/* -- states: -- */
#define FTLA_RUN                                   0  	/* = 0x00 */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_StateNames_15[]
#ifdef cos19_c
 = {
    (u8 const *)"FTLA_RUN",
    (u8 const *) 0
}
#endif
;

#endif
/* -- events: -- */
#define LA_SERVICE_CONN_IND                        0  	/* = 0x00 */
#define LA_SERVICE_RELEASE_IND                     1  	/* = 0x01 */
#define LA_IWU_INFO_IND                            2  	/* = 0x02 */
#define CSM_LA_INFO_REQ_CFM                        3  	/* = 0x03 */
#define LA_FACILITY_REQ                            4  	/* = 0x04 */
#define LA_TIME_STAMP_REQ                          5  	/* = 0x05 */
#define LA_ESCAP_TO_PROP_REQ                       6  	/* = 0x06 */
#define LA_HS_SUBSC_IND                            7  	/* = 0x07 */
#define LA_HS_DESUBSC_IND                          8  	/* = 0x08 */
#define LA_HS_RANGE_IND                            9  	/* = 0x09 */
#define LA_END_SESSION                            10  	/* = 0x0a */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_EventNames_15[]
#ifdef cos19_c
 = {
    (u8 const *)"LA_SERVICE_CONN_IND",
    (u8 const *)"LA_SERVICE_RELEASE_IND",
    (u8 const *)"LA_IWU_INFO_IND",
    (u8 const *)"CSM_LA_INFO_REQ_CFM",
    (u8 const *)"LA_FACILITY_REQ",
    (u8 const *)"LA_TIME_STAMP_REQ",
    (u8 const *)"LA_ESCAP_TO_PROP_REQ",
    (u8 const *)"LA_HS_SUBSC_IND",
    (u8 const *)"LA_HS_DESUBSC_IND",
    (u8 const *)"LA_HS_RANGE_IND",
    (u8 const *)"LA_END_SESSION",
    (u8 const *) 0
}
#endif
;

#endif




/* DDL Compiler switch NO_SP_ON_BASE set */
/* DDLFLAG NO_SP_ON_BASE set in the makefile */


/* PROCESS SPFTMI not compiled because of DDL compiler switch NO_SP_ON_BASE */
/* End of switch NO_SP_ON_BASE */




/* == process FTFWUP: code= 16, nr.of inst.= 1, prio.= high ====== */

						/*9.16 */
/* -- states: -- */
#define FWUP_APP_INIT                              0  	/* = 0x00 */
#define FWUP_APP_RUN                               1  	/* = 0x01 */
#define FWUP_APP_RECV_XHOST_CMD                    2  	/* = 0x02 */
#define FWUP_APP_WRITE_TO_FLASH                    3  	/* = 0x03 */
#define FWUP_APP_DATA_CALL_SETUP                   4  	/* = 0x04 */
#define FWUP_APP_DATA_SEND                         5  	/* = 0x05 */
#define FWUP_APP_DATA_CALL_REL                     6  	/* = 0x06 */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_StateNames_16[]
#ifdef cos19_c
 = {
    (u8 const *)"FWUP_APP_INIT",
    (u8 const *)"FWUP_APP_RUN",
    (u8 const *)"FWUP_APP_RECV_XHOST_CMD",
    (u8 const *)"FWUP_APP_WRITE_TO_FLASH",
    (u8 const *)"FWUP_APP_DATA_CALL_SETUP",
    (u8 const *)"FWUP_APP_DATA_SEND",
    (u8 const *)"FWUP_APP_DATA_CALL_REL",
    (u8 const *) 0
}
#endif
;

#endif
/* -- events: -- */
#define FWUP_GO_SLEEP                              0  	/* = 0x00 */
#define FWUP_WAKEUP_IND                            1  	/* = 0x01 */
#define FWUP_START_REQ                             2  	/* = 0x02 */
#define FWUP_NEXT_PACKET_REQ                       3  	/* = 0x03 */
#define FWUP_END_PACKET_REQ                        4  	/* = 0x04 */
#define FWUP_INT_UPGRADE_HS                        5  	/* = 0x05 */
#define FWUP_INT_DATA_CALL_READY                   6  	/* = 0x06 */
#define FWUP_INT_DATA_CALL_FAIL                    7  	/* = 0x07 */
#define FWUP_INT_READ_MORE_DATA                    8  	/* = 0x08 */
#define FWUP_INT_SEND_IMG_DETAILS                  9  	/* = 0x09 */
#define FWUP_INT_SEND_MORE_DATA                   10  	/* = 0x0a */
#define FWUP_INT_DATA_CALL_REL                    11  	/* = 0x0b */
#define FWUP_INT_DATA_CALL_REL_SUCCESS            12  	/* = 0x0c */
#define FWUP_INT_COMMIT_IMG                       13  	/* = 0x0d */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_EventNames_16[]
#ifdef cos19_c
 = {
    (u8 const *)"FWUP_GO_SLEEP",
    (u8 const *)"FWUP_WAKEUP_IND",
    (u8 const *)"FWUP_START_REQ",
    (u8 const *)"FWUP_NEXT_PACKET_REQ",
    (u8 const *)"FWUP_END_PACKET_REQ",
    (u8 const *)"FWUP_INT_UPGRADE_HS",
    (u8 const *)"FWUP_INT_DATA_CALL_READY",
    (u8 const *)"FWUP_INT_DATA_CALL_FAIL",
    (u8 const *)"FWUP_INT_READ_MORE_DATA",
    (u8 const *)"FWUP_INT_SEND_IMG_DETAILS",
    (u8 const *)"FWUP_INT_SEND_MORE_DATA",
    (u8 const *)"FWUP_INT_DATA_CALL_REL",
    (u8 const *)"FWUP_INT_DATA_CALL_REL_SUCCESS",
    (u8 const *)"FWUP_INT_COMMIT_IMG",
    (u8 const *) 0
}
#endif
;

#endif


/* DDL Compiler switch USE_SPI_TRANSPORT set */
/* DDLFLAG USE_SPI_TRANSPORT set in the makefile */


/* PROCESS SPI_TRANSPORT not compiled because of DDL compiler switch USE_SPI_TRANSPORT */
/* End of switch USE_SPI_TRANSPORT */
/* Low priority processes start here */



/* == process TERMINAL: code= 17, nr.of inst.= 1, prio.= low ====== */


/* -- states: -- */
#define TERMINAL_IDLE                              0  	/* = 0x00 */
#define TERMINAL_RUN                               1  	/* = 0x01 */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_StateNames_17[]
#ifdef cos19_c
 = {
    (u8 const *)"TERMINAL_IDLE",
    (u8 const *)"TERMINAL_RUN",
    (u8 const *) 0
}
#endif
;

#endif
/* -- events: -- */
#define CONTINUE_IND                               0  	/* = 0x00 */
#define KEY_IND                                    1  	/* = 0x01 */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_EventNames_17[]
#ifdef cos19_c
 = {
    (u8 const *)"CONTINUE_IND",
    (u8 const *)"KEY_IND",
    (u8 const *) 0
}
#endif
;

#endif
/* any process placed above TERMINAL is high priority */


/* == process FTTTS: code= 18, nr.of inst.= 1, prio.= low ====== */


/* -- states: -- */
#define FTTS_IDLE                                  0  	/* = 0x00 */
#define FTTS_RUN                                   1  	/* = 0x01 */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_StateNames_18[]
#ifdef cos19_c
 = {
    (u8 const *)"FTTS_IDLE",
    (u8 const *)"FTTS_RUN",
    (u8 const *) 0
}
#endif
;

#endif
/* -- events: -- */
#define CM_TTS_START                               0  	/* = 0x00 */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_EventNames_18[]
#ifdef cos19_c
 = {
    (u8 const *)"CM_TTS_START",
    (u8 const *) 0
}
#endif
;

#endif




/* == process FWUP_TASK: code= 19, nr.of inst.= 1, prio.= low ====== */


/* -- states: -- */
#define FWUP_TASK_RUN                              0  	/* = 0x00 */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_StateNames_19[]
#ifdef cos19_c
 = {
    (u8 const *)"FWUP_TASK_RUN",
    (u8 const *) 0
}
#endif
;

#endif
/* -- events: -- */
#define GET_IMAGE_DETAILS                          0  	/* = 0x00 */
#define READ_IMAGE_SEGMENT                         1  	/* = 0x01 */
#define READ_IMAGE_DATA_SEGMENT                    2  	/* = 0x02 */
#define WRITE_IMAGE_SEGMENT                        3  	/* = 0x03 */
#define WRITE_IMAGE_DATA_SEGMENT                   4  	/* = 0x04 */
#define COMMIT_IMAGE                               5  	/* = 0x05 */
#define MODIFY_IMAGE_LOCATION                      6  	/* = 0x06 */
#define COMMAND_CONTINUE                           7  	/* = 0x07 */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_EventNames_19[]
#ifdef cos19_c
 = {
    (u8 const *)"GET_IMAGE_DETAILS",
    (u8 const *)"READ_IMAGE_SEGMENT",
    (u8 const *)"READ_IMAGE_DATA_SEGMENT",
    (u8 const *)"WRITE_IMAGE_SEGMENT",
    (u8 const *)"WRITE_IMAGE_DATA_SEGMENT",
    (u8 const *)"COMMIT_IMAGE",
    (u8 const *)"MODIFY_IMAGE_LOCATION",
    (u8 const *)"COMMAND_CONTINUE",
    (u8 const *) 0
}
#endif
;

#endif


 
/* CMBS  */


/* == process CMBSTASK: code= 20, nr.of inst.= 1, prio.= low ====== */


/* -- states: -- */
#define CMBS_TASK_IDLE                             0  	/* = 0x00 */
#define CMBS_TASK_RUN                              1  	/* = 0x01 */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_StateNames_20[]
#ifdef cos19_c
 = {
    (u8 const *)"CMBS_TASK_IDLE",
    (u8 const *)"CMBS_TASK_RUN",
    (u8 const *) 0
}
#endif
;

#endif
/* -- events: -- */
#define CMBS_TASK_UART_IND                         0  	/* = 0x00 */
#define CMBS_TASK_UART_CMPL                        1  	/* = 0x01 */
#define CMBS_TASK_UART_CFM                         2  	/* = 0x02 */
#define CMBS_TASK_PACK_IND                         3  	/* = 0x03 */
#define CMBS_TASK_PACK_REQ                         4  	/* = 0x04 */
#define CMBS_TASK_HS_REGISTERED                    5  	/* = 0x05 */
#define CMBS_TASK_USB_LINE_STATUS                  6  	/* = 0x06 */
#define CMBS_TASK_KEYBOARD_HIT                     7  	/* = 0x07 */
#define CMBS_TASK_FACILITY_REQ                     8  	/* = 0x08 */
#define CMBS_TASK_FACILITY_CNF                     9  	/* = 0x09 */

#if (OS_DEBUG)
PUBLIC u8 const * const G_pu8_os09_EventNames_20[]
#ifdef cos19_c
 = {
    (u8 const *)"CMBS_TASK_UART_IND",
    (u8 const *)"CMBS_TASK_UART_CMPL",
    (u8 const *)"CMBS_TASK_UART_CFM",
    (u8 const *)"CMBS_TASK_PACK_IND",
    (u8 const *)"CMBS_TASK_PACK_REQ",
    (u8 const *)"CMBS_TASK_HS_REGISTERED",
    (u8 const *)"CMBS_TASK_USB_LINE_STATUS",
    (u8 const *)"CMBS_TASK_KEYBOARD_HIT",
    (u8 const *)"CMBS_TASK_FACILITY_REQ",
    (u8 const *)"CMBS_TASK_FACILITY_CNF",
    (u8 const *) 0
}
#endif
;

#endif

/* == END of bsd09sdl.ddl =============================================== */
/* === END of configuration of the DDL system === */
#define OS09_NR_OF_PROCESSES        21
#define OS09_LEN_OF_STATE_TABLE     86
#define OS09_1ST_LOW_PRIO_PROCESS   17

PUBLIC u8 const G_u8_os09_StateTableIndex[OS09_NR_OF_PROCESSES]
#ifdef cos19_c
 = {
    0,
    10,
    11,
    17,
    23,
    29,
    35,
    41,
    47,
    65,
    66,
    67,
    77,
    78,
    79,
    80,
    81,
    82,
    83,
    84,
    85
}
#endif
;

extern void    FTCSM(void);
extern void    FTDSR(void);
extern void    FTSS(void);
extern void    FTMM(void);
extern void    FTMLP(void);
extern void    p_hl17_LUXProcess(void);
extern void    FTHE(void);
extern void    FTCH(void);
extern void    FTCC(void);
extern void    FTRMC(void);
extern void    TRACE_PROCESS(void);
extern void    FTMI(void);
extern void    CSA(void);
extern void    FTMMS(void);
extern void    FTCLI(void);
extern void    FTLA(void);
extern void    FTFWUP(void);
extern void    TERMINAL(void);
extern void    FTTTS(void);
extern void    FWUP_TASK(void);
extern void    CMBSTASK(void);

PUBLIC void (* const G_pp_os09_ProcessCode[OS09_NR_OF_PROCESSES])()
#ifdef cos19_c
 = {
    FTCSM,
    FTDSR,
    FTSS,
    FTMM,
    FTMLP,
    p_hl17_LUXProcess,
    FTHE,
    FTCH,
    FTCC,
    FTRMC,
    TRACE_PROCESS,
    FTMI,
    CSA,
    FTMMS,
    FTCLI,
    FTLA,
    FTFWUP,
    TERMINAL,
    FTTTS,
    FWUP_TASK,
    CMBSTASK
}
#endif
;

#define FTCSM_ID          0
#define FTDSR_ID          1
#define FTSS_ID           2
#define FTMM_ID           3
#define FTMLP_ID          4
#define p_hl17_LUXProcess_ID    5
#define FTHE_ID           6
#define FTCH_ID           7
#define FTCC_ID           8
#define FTRMC_ID          9
#define TRACE_PROCESS_ID  10
#define FTMI_ID           11
#define CSA_ID            12
#define FTMMS_ID          13
#define FTCLI_ID          14
#define FTLA_ID           15
#define FTFWUP_ID         16
#define TERMINAL_ID       17
#define FTTTS_ID          18
#define FWUP_TASK_ID      19
#define CMBSTASK_ID       20

#if (OS_DEBUG)

PUBLIC u8 const * const G_pu8_os09_ProcessNames[OS09_NR_OF_PROCESSES]
#ifdef cos19_c
 = {
    (u8 const *)"FTCSM_ID",
    (u8 const *)"FTDSR_ID",
    (u8 const *)"FTSS_ID",
    (u8 const *)"FTMM_ID",
    (u8 const *)"FTMLP_ID",
    (u8 const *)"p_hl17_LUXProcess_ID",
    (u8 const *)"FTHE_ID",
    (u8 const *)"FTCH_ID",
    (u8 const *)"FTCC_ID",
    (u8 const *)"FTRMC_ID",
    (u8 const *)"TRACE_PROCESS_ID",
    (u8 const *)"FTMI_ID",
    (u8 const *)"CSA_ID",
    (u8 const *)"FTMMS_ID",
    (u8 const *)"FTCLI_ID",
    (u8 const *)"FTLA_ID",
    (u8 const *)"FTFWUP_ID",
    (u8 const *)"TERMINAL_ID",
    (u8 const *)"FTTTS_ID",
    (u8 const *)"FWUP_TASK_ID",
    (u8 const *)"CMBSTASK_ID"
}
#endif
;


PUBLIC u8 const * const * const G_pu8_os09_StateNames[OS09_NR_OF_PROCESSES]
#ifdef cos19_c
 = {
    (u8 const * const *)G_pu8_os09_StateNames_0,
    (u8 const * const *)G_pu8_os09_StateNames_1,
    (u8 const * const *)G_pu8_os09_StateNames_2,
    (u8 const * const *)G_pu8_os09_StateNames_3,
    (u8 const * const *)G_pu8_os09_StateNames_4,
    (u8 const * const *)G_pu8_os09_StateNames_5,
    (u8 const * const *)G_pu8_os09_StateNames_6,
    (u8 const * const *)G_pu8_os09_StateNames_7,
    (u8 const * const *)G_pu8_os09_StateNames_8,
    (u8 const * const *)G_pu8_os09_StateNames_9,
    (u8 const * const *)G_pu8_os09_StateNames_10,
    (u8 const * const *)G_pu8_os09_StateNames_11,
    (u8 const * const *)G_pu8_os09_StateNames_12,
    (u8 const * const *)G_pu8_os09_StateNames_13,
    (u8 const * const *)G_pu8_os09_StateNames_14,
    (u8 const * const *)G_pu8_os09_StateNames_15,
    (u8 const * const *)G_pu8_os09_StateNames_16,
    (u8 const * const *)G_pu8_os09_StateNames_17,
    (u8 const * const *)G_pu8_os09_StateNames_18,
    (u8 const * const *)G_pu8_os09_StateNames_19,
    (u8 const * const *)G_pu8_os09_StateNames_20
}
#endif
;


PUBLIC u8 const * const * const G_pu8_os09_EventNames[OS09_NR_OF_PROCESSES]
#ifdef cos19_c
 = {
    (u8 const * const *)G_pu8_os09_EventNames_0,
    (u8 const * const *)G_pu8_os09_EventNames_1,
    (u8 const * const *)G_pu8_os09_EventNames_2,
    (u8 const * const *)G_pu8_os09_EventNames_3,
    (u8 const * const *)G_pu8_os09_EventNames_4,
    (u8 const * const *)G_pu8_os09_EventNames_5,
    (u8 const * const *)G_pu8_os09_EventNames_6,
    (u8 const * const *)G_pu8_os09_EventNames_7,
    (u8 const * const *)G_pu8_os09_EventNames_8,
    (u8 const * const *)G_pu8_os09_EventNames_9,
    (u8 const * const *)G_pu8_os09_EventNames_10,
    (u8 const * const *)G_pu8_os09_EventNames_11,
    (u8 const * const *)G_pu8_os09_EventNames_12,
    (u8 const * const *)G_pu8_os09_EventNames_13,
    (u8 const * const *)G_pu8_os09_EventNames_14,
    (u8 const * const *)G_pu8_os09_EventNames_15,
    (u8 const * const *)G_pu8_os09_EventNames_16,
    (u8 const * const *)G_pu8_os09_EventNames_17,
    (u8 const * const *)G_pu8_os09_EventNames_18,
    (u8 const * const *)G_pu8_os09_EventNames_19,
    (u8 const * const *)G_pu8_os09_EventNames_20
}
#endif
;

#endif
#undef PUBLIC
/* === END of configuration of the DDL system === */
