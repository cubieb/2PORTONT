/*
 * Copyright (C) 2014 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * Purpose : Definition of OMCI timer related define
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) OMCI timer related define
 */

#ifndef __OMCI_TIMER_H__
#define __OMCI_TIMER_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "gos_linux.h"
#include "gos_type.h"


#define OMCI_TIMER_RESERVED_CLASS_ID            0
#define OMCI_TIMER_RESERVED_PM_INSTANCE_ID      0
#define OMCI_TIMER_RESERVED_ALM_INSTANCE_ID     1
#define OMCI_TIMER_PWR_PRE_SHED_DATA_INST_ID    2
#define OMCI_TIMER_PWR_POST_SHED_DATA_INST_ID   3
#define OMCI_TIMER_PWR_PRE_SHED_VOICE_INST_ID   4
#define OMCI_TIMER_PWR_POST_SHED_VOICE_INST_ID  5

#define OMCI_TIMER_PM_COLLECT_INTERVAL_SECS     1
#define OMCI_TIMER_TM_INTERVAL_SECS     		1
#define OMCI_TIMER_ALM_UPLOAD_INTERVAL_SECS     60


typedef void (*timerCB_t)(UINT16 classID, UINT16 instanceID, UINT32 privData);

typedef struct {
    timer_t     timerID;
    timerCB_t   cbFunction;
    UINT16      classID;
    UINT16      instanceID;
    UINT32		privData;
} __attribute__((aligned)) omci_timer_t;

typedef struct omci_timer_entry_s {
    omci_timer_t                    data;
    LIST_ENTRY(omci_timer_entry_s)  entries;
} __attribute__((aligned)) omci_timer_entry_t;


omci_timer_entry_t* omci_timer_search(UINT16    classID,
                                        UINT16  instanceID);
GOS_ERROR_CODE omci_timer_create(UINT16			classID,
									UINT16		instanceID,
									time_t		intervalSecs,
									long		intervalNanoSecs,
									BOOL        isRegular,
									UINT32      privData,
									timerCB_t	cbFunction);
GOS_ERROR_CODE omci_timer_restart(UINT16        classID,
                                    UINT16      instanceID,
                                    time_t      intervalSecs,
                                    long        intervalNanoSecs,
                                    BOOL        isRegular);
GOS_ERROR_CODE omci_timer_delete_by_entry(omci_timer_entry_t	*pEntry);
GOS_ERROR_CODE omci_timer_delete_by_id(UINT16   classID,
									   UINT16	instanceID);
GOS_ERROR_CODE omci_timer_stop_and_delete_by_id(UINT16  classID,
                                                UINT16  instanceID,
                                                time_t  *pRemainedSecs,
                                                long    *pRemainedNanoSecs);
GOS_ERROR_CODE omci_timer_init(void);
GOS_ERROR_CODE omci_timer_deinit(void);


#ifdef __cplusplus
}
#endif

#endif
