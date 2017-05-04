/** \file sys_service.c
 * sys_service.c
 *
 *  This file implements the required system services for the API-II using a
 * Linux OS running on the UVB.  The user should replace the functions provided
 * here with the equivalent based on their OS and hardware.
 *
 * Copyright (c) 2005, Legerity Inc.
 * All rights reserved
 *
 * This software is the property of Legerity , Inc. Please refer to the
 * Non Disclosure Agreement (NDA) that you have signed for more information
 * on legal obligations in using, modifying or distributing this file.
 */
#include <linux/interrupt.h>
#include <linux/version.h>
#include <linux/delay.h>
#include <linux/semaphore.h>
#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LINETEST)
#include "vp_api.h"
#else
//#include "vp_api.h"
#endif
#include "vp_api_types.h"
#include "sys_service.h"
#define MAX_SS_TEST_HEAPS   4
#define MAX_SS_CONNECTORS   2
#define MAX_SS_CHIPSELECTS  8

typedef struct {
#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LINETEST)
	VpTestHeapType  testHeap;
#else
//    VpTestHeapType  testHeap;
#endif
    int             testBufId;          /* Test Buffer ID assigned (-1, if none avail.) */
} SysSerTestAcquireType;


typedef struct{
    int pid;
    int cnt;
    struct semaphore criticalSem;
    struct semaphore sectionSem;
    spinlock_t       lock;
} VpSysCriticalParamType;

/*******************************************************************************
 * Global memory requirements
 *******************************************************************************/
VpSysCriticalParamType gSysCriticalSec[VP_NUM_CRITICAL_SEC_TYPES];
SysSerTestAcquireType gTestHeaps[MAX_SS_TEST_HEAPS];
#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LINETEST)
static int microsemi_slic_init = 0;
#endif
/*
 * VpSysWait() function implementation is needed only for CSLAC devices
 * (880, 790). For other devices this function could be commented.
 */
void
VpSysWait(
    uint8 time)  /* Time specified in increments of 125uS (e.g. 4 = 500uS) */
{
    /* Blocking delay function added here based on OS */
	 udelay(125*time);
}
void VpSysSemaphoreInit(int semaphoreId)
{
}

void
VpSysServiceInit(void)
{
#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LINETEST)
	int i = 0;
    int heap = 0;

    /* init the test heaps */
    for (; heap < MAX_SS_TEST_HEAPS; heap++) {
        gTestHeaps[heap].testBufId = -1;
    }
	microsemi_slic_init = 1;
#if 0
    for(;i < VP_NUM_CRITICAL_SEC_TYPES; i++)
    {
        gSysCriticalSec[i].pid = -1;
        gSysCriticalSec[i].cnt = 0;
        spin_lock_init(&gSysCriticalSec[i].lock);
        sema_init( &gSysCriticalSec[i].criticalSem, 1 );
        sema_init( &gSysCriticalSec[i].sectionSem, 1 );
    }
#endif
#endif
    return;
}

/*
 * VpSysServiceToggleLed():
 *
 *  This function was added as a debug utility for ZTAP. The function will 
 *  toggle an LED from its existing state to the opposite state
 *
 * Params:
 *  uint8 ledNum: LED to toggle
 *
 * Return:
 *  N/A
 */
void 
VpSysServiceToggleLed(
    uint8 ledNum)
{
#if 0
    unsigned long ledReg;
    unsigned long newLedReg;
    unsigned long ledBit;
    unsigned long ledMask;

    if (ledNum > 15) {
        return;
    }

    ledBit = (1 << ledNum);
    ledMask = (ledBit * 65536);

    /* get the current led reg value */
    ledReg = TelecomFpgaRead(TELECOM_FPGA_LED_CTRL_REG);

    if (ledReg & ledBit) {
        newLedReg =  (ledReg & ~(ledBit)) | ledMask;
    } else {
        newLedReg =  (ledReg | ledBit) | ledMask;
    }
    //printk("ledReg 0x%08lx : ledBit 0x%08lx : ledMask 0x%08lx : newLedReg 0x%08lx \n", ledReg, ledBit, ledMask, newLedReg);
    /* toggle the LED */
    TelecomFpgaWrite(TELECOM_FPGA_LED_CTRL_REG, newLedReg);

#endif
    /* mask the led so */
    //TelecomFpgaWrite(TELECOM_FPGA_LED_CTRL_REG, (ledReg & ~ledMask));
    return;
}

/*
 * VpSysEnterCritical(), VpSysExitCritical():
 *
 *  These functions allow for disabling interrupts while executing nonreentrant
 * portions of VoicePath API code. Note that the following implementations of
 * enter/exit critical section functions are simple implementations. These
 * functions could be expanded (if required) to handle different critical
 * section types differently.
 *
 * Params:
 *  VpDeviceIdType deviceId: Device Id (chip select ID)
 *  VpCriticalSecType: Critical section type
 *
 * Return:
 *  Number of critical sections currently entered for the device.
 */

#define CRITICAL_DEPTH_MAX	10
static unsigned int criticalDepth = 0;
#if defined(CONFIG_DEFAULTS_KERNEL_2_6) || defined(CONFIG_DEFAULTS_KERNEL_3_4)
static int flags[CRITICAL_DEPTH_MAX];
spinlock_t VpSysSpinLock[CRITICAL_DEPTH_MAX];
#else
static unsigned long flags[CRITICAL_DEPTH_MAX] = 0;
#endif
 
uint8
VpSysEnterCritical(
    VpDeviceIdType deviceId,
    VpCriticalSecType criticalSecType)
{
    /*
     * Code to check if semaphore can be taken, and if so increment. Block if
     * semaphore cannot be taken for this process ID.
     */
	
#if defined(CONFIG_DEFAULTS_KERNEL_2_6) || defined(CONFIG_DEFAULTS_KERNEL_3_4)
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,32))
	spin_lock_irqsave(&VpSysSpinLock[criticalDepth],flags[criticalDepth]);
#else
    spin_lock_irqsave(VpSysSpinLock[criticalDepth],flags[criticalDepth]);
#endif
#else
	save_flags(flags[criticalDepth]); cli();
#endif

	criticalDepth = criticalDepth + 1;

	if (criticalDepth >= CRITICAL_DEPTH_MAX)
		printk("%s error, criticalDepth is %d, over %d\n", __FUNCTION__, criticalDepth, CRITICAL_DEPTH_MAX);

    /*
     * criticalDepth++;
     * return criticalDepth;
     */
    /* Prevent compiler from generating error */
    return criticalDepth;
    //return 0;
} /* VpSysEnterCritical() */

uint8
VpSysExitCritical(
    VpDeviceIdType deviceId,
    VpCriticalSecType criticalSecType)
{
    /* Code to decrement semaphore */
    
	criticalDepth = criticalDepth - 1;
    
#if defined(CONFIG_DEFAULTS_KERNEL_2_6) || defined(CONFIG_DEFAULTS_KERNEL_3_4)
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,32))
	spin_unlock_irqrestore(&VpSysSpinLock[criticalDepth],flags[criticalDepth]); 
#else
    spin_unlock_irqrestore(VpSysSpinLock[criticalDepth],flags[criticalDepth]); 
#endif
#else
    restore_flags(flags[criticalDepth]);
#endif

    /*
     * criticalDepth--;
     * return criticalDepth;
     */
    /* Prevent compiler from generating error */
    return criticalDepth;
    //return 0;
} /* VpSysExitCritical() */

/**
 * VpSysDisableInt(), VpSysEnableInt(), and VpSysTestInt()
 *
 *  These functions are used by the CSLAC device family for interrupt driven
 * polling modes. These are called by the API to detect when a non-masked
 * device status has changed.  If using SIMPLE_POLL mode, these functions do not
 * require implementation.
 *
 * Preconditions:
 *  None. The implementation of these functions is architecture dependent.
 *
 * Postconditions:
 *  VpSysDisableInt() - The interrupt associated with the deviceId passed is
 * disabled.
 *
 * VpSysEnableInt() - The interrupt associated with the deviceId passed is
 * enabled.
 *
 * VpSysTestInt() - The return value is TRUE if an interrupt occurred, otherwise
 * return FALSE.
 *
 * These functions are needed only for CSLAC devices
 * (880, 790). For other devices these functions could be commented.
 *
 */
void
VpSysDisableInt(
    VpDeviceIdType deviceId)
{
    return;
}
void
VpSysEnableInt(
    VpDeviceIdType deviceId)
{
    return;
}
bool
VpSysTestInt(
    VpDeviceIdType deviceId)
{
    return FALSE;
}
/**
 * VpSysDtmfDetEnable(), VpSysDtmfDetDisable()
 *
 *  These functions are used by the CSLAC device family for devices that do not
 * internally detect DTMF. It is used for Caller ID type-II and is provided to
 * enable external DTMF detection.
 *
 * Preconditions:
 *  None. The implementation of these functions is application dependent.
 *
 * Postconditions:
 *  VpSysDtmfDetEnable() - The device/channel resource for DTMF detection is
 * enabled.
 *
 *  VpSysDtmfDetDisable() - The device/channel resource for DTMF detection is
 * disabled.
 *
 * These functions are needed only for CSLAC devices
 * (880, 790). For other devices these functions could be commented.
 *
 */
void
VpSysDtmfDetEnable(
    VpDeviceIdType deviceId,
    uint8 channelId)
{
}
void
VpSysDtmfDetDisable(
    VpDeviceIdType deviceId,
    uint8 channelId)
{
}
/*
 * The following functions VpSysTestHeapAcquire(),  VpSysTestHeapRelease()
 * VpSysPcmCollectAndProcess() and are needed only for CSLAC devices
 * (880). For other devices these functions could be commented. Please see
 * the LineTest API documentation for function requirements.
 */
void *
VpSysTestHeapAcquire(
    uint8 *pHeapId)
{
#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LINETEST)
    int i = 0;
	if ( !microsemi_slic_init )
		VpSysServiceInit();
    do {
        if (gTestHeaps[i].testBufId == -1) {
            gTestHeaps[i].testBufId = i;
            *pHeapId = i;
            return &gTestHeaps[i].testHeap;
        }
    } while (++i < MAX_SS_TEST_HEAPS);
#endif
    return VP_NULL;
} /* VpSysTestHeapAcquire() */
bool
VpSysTestHeapRelease(
    uint8 heapId)
{
#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LINETEST)
    if (heapId >= MAX_SS_TEST_HEAPS) {
        return FALSE;
    }
    gTestHeaps[heapId].testBufId = -1;
    return TRUE;
#endif	
} /* VpSysTestHeapRelease() */
void
VpSysPcmCollectAndProcess(
    void *pLineCtx,
    VpDeviceIdType deviceId,
    uint8 channelId,
    uint8 startTimeslot,
    uint16 operationTime,
    uint16 settlingTime,
    uint16 operationMask)
{
} /* VpSysPcmCollectAndProcess() */
