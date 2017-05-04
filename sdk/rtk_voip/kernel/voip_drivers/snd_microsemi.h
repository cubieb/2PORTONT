
#ifndef SND_MICROSEMI_H
#define SND_MICROSEMI_H

/*******************************************************************************
 * Includes
 *******************************************************************************/
//Realtek headers
#include "con_register.h"
#include "rtk_voip.h"
#include "voip_init.h"
#include "snd_help_ipc.h"
#include "spi.h"
#include "con_defer_init.h"
#include "snd_define.h"
#include "con_mux.h"
#include "con_ring.h"

//Microsemi headers
#include "vp_api.h"
#include "vp_api_types.h"
#include "Ve_profile.h"
#include "mpi_hal.h"

//Linux headers
#include <linux/autoconf.h>
#include <linux/slab.h> 
#include <linux/proc_fs.h>
#include <linux/workqueue.h>

//Line Test (Verivoice related headers)
#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LINETEST)
#include "lt_api.h"
#endif

/*******************************************************************************
 * Defines 
 *******************************************************************************/
#define MICRO_SEMI_MAX_NUM_DEV  16
#define MICRO_SEMI_MAX_NUM_LINE 32

#define DEBUG_API 

#if defined(DEBUG_API)
#define DEBUG_API_PRINT() printk("%s(%d) line #%d\n",__FUNCTION__,__LINE__,pLineInfo->chanId);
#define PRINT_MSG    printk
#else
#define DEBUG_API_PRINT()
#define PRINT_MSG
#endif

#define PRINT_ERR(st,str) PRINT_R("Error %d (%p:%d) %s %s\n",st,(void *)pLineInfo->pDevInfo->deviceId,pLineInfo->chanId,__FUNCTION__,str)
#define ASSERT_microsemi(st,func) if (st != VP_STATUS_SUCCESS) PRINT_ERR(st,func)

#define RING_PROFILE_CAD_ON_H_IDX       10
#define RING_PROFILE_CAD_ON_L_IDX       11
#define DEVICE_PROFILE_TICK_RATE_IDX    12
#define RING_PROFILE_CAD_OFF_H_IDX      14
#define RING_PROFILE_CAD_OFF_L_IDX      15

#define SLIC_INIT_TIMEOUT               1200

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
#define SUPPORT_CH_NUM                  16
static int NTTCar1stCheckFlag[SUPPORT_CH_NUM] = {[0 ... SUPPORT_CH_NUM-1] = 0};
static unsigned long time_out_modify[SUPPORT_CH_NUM];
#endif

#define PASS 1
#define FAIL 0

/* ****************************************************************************
 * This is a union of the possible VP-API Device Object types supported by
 * this Microsemi VP-API driver.
 * The size of the union will only be as large as the largest
 * device object type defined.
 * ****************************************************************************/
typedef union {

    #ifdef VP_CC_880_SERIES
    Vp880DeviceObjectType vp880DevObj;
    #endif

    #ifdef VP_CC_890_SERIES
    Vp890DeviceObjectType vp890DevObj;
    #endif

    #ifdef VP_CC_886_SERIES
    Vp886DeviceObjectType vp886DevObj;
    #endif

    uint8 dummy; /* preventing an empty union */
} MSVpDeviceObjType;

/* ****************************************************************************
 * This is a union of the possible VP-API Line Object types supported by
 * this Microsemi VP-API driver.
 * The size of the union will only be as large as the larges
 * line object type defined.
 * ****************************************************************************/
typedef union {

    #ifdef VP_CC_880_SERIES
    Vp880LineObjectType vp880LineObj;
    #endif

    #ifdef VP_CC_890_SERIES
    Vp890LineObjectType vp890LineObj;
    #endif

    #ifdef VP_CC_886_SERIES
    Vp886LineObjectType vp886LineObj;
    #endif

    uint8 dummy; /* preventing an empty union */
} MSVpLineObjType;

typedef enum 
{
    MS_SLIC_TYPE_NONE = -1,
    MS_SLIC_TYPE_LE89116,
    MS_SLIC_TYPE_LE9662,
    MS_SLIC_TYPE_LE89316,
    MS_SLIC_TYPE_LE9641,
    MS_SLIC_TYPE_LE89156,
    MS_SLIC_TYPE_LE88601,
    MS_SLIC_TYPE_END
} MS_SLIC_TYPE;

typedef enum 
{
    MS_VP_DEV_NONE_SERIES = -1,
    MS_VP_DEV_886_SERIES,
    MS_VP_DEV_887_SERIES
} VP_API_DEVICE_TYPE;

typedef enum {
    DEV_UNKNOWN,
    DEV_FXS,
    DEV_FXO,
    DEV_FXSFXS,
    DEV_FXSFXO,
    DEV_LAST 
} RTKDevType;

typedef struct MsSlicInfo_{
    unsigned int        noOfFxs;
    unsigned int        noOfFxo;
    MS_SLIC_TYPE        msSlicType;
    char                *slicName;
    VpDeviceType        aDeviceType;
} MsSlicInfo;

/*******************************************************************************
 * Globals 
 *******************************************************************************/

 
/*******************************************************************************
 * Function Prototypes
 *******************************************************************************/
  
 
 #endif /* SND_MICROSEMI_H */
