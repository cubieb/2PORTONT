
#include "vp_api_common.h"

#include "ZLR96622L_A0.h"
#include "ZLR966121L.h"

#include "Le890WB_HV.h"
#include "Le89156_100V_IB_Profile.h"

#include "Le890NB_LVHV.h"

#include "ZLR966121L.h"
#include "Le880NB_HV.h"
#include "Realtek_88601.h"

#include "temp_ZLR9641L.h"

/* ******************************************** */ 
/*  LE 886 series profile                       */
/* ******************************************** */ 
/* DEV */
#define DEF_LE886_9662_DEV_PROFILE                      DEV_PROFILE_47UH
/* FXS */
#define DEF_LE886_9662_AC_PROFILE                       Le9662_AC_FXS_RF14_600R_DEF
#define DEF_LE886_9662_DC_PROFILE                       Le9662_DC_FXS_VE960_ABS100V_DEF
#define DEF_LE886_9662_RING_PROFILE                     Le9662_RING_VE960_ABS100V_DEF
#define DEF_LE886_9662_RING_CAD_PROFILE                 Le9662_RING_CAD_STD

//#define DEF_LE886_9662_METERING_12K_PROFILE             METER_12KHZ_RF14
//#define DEF_LE886_9662_METERING_16K_PROFILE             METER_16KHZ_RF14

/* FXO */ /* VE886 Does NOT support FXO yet. */
#define DEF_LE886_9662_AC_FXO_LC_PROFILE                VP_PTABLE_NULL
#define DEF_LE886_9662_FXO_DIALING_PROFILE              VP_PTABLE_NULL


/* DEV */
#define DEF_LE886_9661_DEV_PROFILE                      ZLR966121_FB100V_DEVICE
/* FXS */
#define DEF_LE886_9661_AC_PROFILE                       Le9661_AC_FXS_RF14_600R_DEF
#define DEF_LE886_9661_DC_PROFILE                       Le9661_DC_FXS_MISLIC_FB100V_DEF
#define DEF_LE886_9661_RING_PROFILE                     Le9661_RING_MISLIC_FB100V_DEF
#define DEF_LE886_9661_RING_CAD_PROFILE                 LE880_RING_CAD_STD

//#define DEF_LE886_9661_METERING_12K_PROFILE             METER_12KHZ_RF14
//#define DEF_LE886_9661_METERING_16K_PROFILE             METER_16KHZ_RF14

/* FXO */ /* VE886 Does NOT support FXO yet. */
#define DEF_LE886_9661_AC_FXO_LC_PROFILE                VP_PTABLE_NULL
#define DEF_LE886_9661_FXO_DIALING_PROFILE              VP_PTABLE_NULL

/* ******************************************** */ 
/*  LE 88601 series profile                       */
/* ******************************************** */ 
/* DEV */
#define DEF_LE886_88601_DEV_PROFILE                      ZLR88621L_ABS81V_DEVICE

/* FXS */
#define DEF_LE886_88601_AC_PROFILE                       ZLR88621L_AC_FXS_RF14_600R_DEF
#define DEF_LE886_88601_DC_PROFILE                       ZLR88621L_DC_FXS_ZL880_ABS100V_DEF
#define DEF_LE886_88601_RING_PROFILE                     ZLR88621L_RING_ZL880_ABS100V_DEF
#define DEF_LE886_88601_RING_CAD_PROFILE                 LE880_RING_CAD_STD

//#define DEF_LE886_88601_METERING_12K_PROFILE             METER_12KHZ_RF14
//#define DEF_LE886_88601_METERING_16K_PROFILE             METER_16KHZ_RF14

/* FXO */ /* VE886 Does NOT support FXO yet. */
#define DEF_LE886_88601_AC_FXO_LC_PROFILE                VP_PTABLE_NULL
#define DEF_LE886_88601_FXO_DIALING_PROFILE              VP_PTABLE_NULL

/* ******************************************** */ 
/*  LE 887 series profile                       */
/* ******************************************** */

/* DEV */
#define DEF_LE886_9641_DEV_PROFILE                      ZLR966121_BB100V_DEVICE
/* FXS */
#define DEF_LE886_9641_AC_PROFILE                       AC_FXS_RF14_600R_DEF
#define DEF_LE886_9641_DC_PROFILE                       DC_FXS_MISLIC_BB100V_DEF
#define DEF_LE886_9641_RING_PROFILE                     RING_MISLIC_FB100V_DEF
#define DEF_LE886_9641_RING_CAD_PROFILE                 LE880_RING_CAD_STD

//#define DEF_LE886_9641_METERING_12K_PROFILE             METER_12KHZ_RF14
//#define DEF_LE886_9641_METERING_16K_PROFILE             METER_16KHZ_RF14

/* FXO */ /* VE886 Does NOT support FXO yet. */
#define DEF_LE886_9641_AC_FXO_LC_PROFILE                VP_PTABLE_NULL
#define DEF_LE886_9641_FXO_DIALING_PROFILE              VP_PTABLE_NULL


/* ******************************************** */ 
/*  LE 880 series profile                       */
/* ******************************************** */ 
/* DEV */
#define DEF_LE880_DEV_PROFILE                           LE880_ABS_VBL_FLYBACK

/* FXS */
#define DEF_LE880_AC_PROFILE                            LE880_AC_FXS_RF14_DEF
#define DEF_LE880_DC_PROFILE                            LE880_DC_FXS_DEF
#define DEF_LE880_RING_PROFILE                          LE880_RING_DEF
#define DEF_LE880_RING_CAD_PROFILE                      LE880_RING_CAD_STD

/* FXO */ /* VE880 Does NOT support FXO yet. */
#define DEF_LE880_AC_FXO_LC_PROFILE                     VP_PTABLE_NULL
#define DEF_LE880_FXO_DIALING_PROFILE                   VP_PTABLE_NULL


/* ******************************************** */ 
/*  LE 890 series profile                       */
/* ******************************************** */ 

#define DEF_LE890_DEV_PROFILE_HV                        LE890_DEV_PROFILE_Buck_Boost_HV
#define DEF_LE890_DEV_PROFILE_LV                        LE890_DEV_PROFILE_Buck_Boost_LV

/* FXS */
#define DEF_LE890_AC_PROFILE                            LE890_BB_AC_FXS_RF50_600R_DEF
#define DEF_LE890_DC_PROFILE                            LE890_BB_DC_FXS_DEF
#define DEF_LE890_RING_PROFILE                          LE890_BB_RING_25HZ_DEF
#define DEF_LE890_RING_CAD_PROFILE                      LE890_BB_RING_CAD_STD


#define DEF_LE890_LE89156_DEV_PROFILE_HV                DEV_PROFILE_Inverting_Boost_100V
#define DEF_LE890_LE89156_DEV_PROFILE_LV                DEV_PROFILE_Inverting_Boost_100V

/* FXS */
#define DEF_LE890_LE89156_AC_PROFILE                    AC_FXS_RF50_600R_DEF
#define DEF_LE890_LE89156_DC_PROFILE                    DC_FXS_DEF
#define DEF_LE890_LE89156_RING_PROFILE                  RING_25HZ_DEF
#define DEF_LE890_LE89156_RING_CAD_PROFILE              LE890_BB_RING_CAD_STD


/* FXO */
#define DEF_LE890_AC_FXO_LC_PROFILE                     LE890_BB_AC_FXO_LC_600R_DEF
#define DEF_LE890_FXO_DIALING_PROFILE                   LE890_BB_FXO_DIALING_DEF


/************** Cadence_Definitions **************/
extern VpProfileDataType LE880_RING_CAD_USER_DEF[]; 	  /* User defined */
extern VpProfileDataType LE886_RING_CAD_USER_DEF[]; 	  /* User defined */
extern VpProfileDataType LE890_RING_CAD_USER_DEF[]; 	  /* User defined */


