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
 * Purpose : Definition of OMCI task define
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) OMCI task define
 */

#ifndef _OMCI_TASK_H_
#define _OMCI_TASK_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "omci_msgq.h"
#include "omci_util.h"
#include "omci_driver.h"

#define OMCI_TASK_NAME_MAX_LEN          (32)

typedef INT32 OMCI_TASK_ID;
typedef GOS_ERROR_CODE (*OMCI_TASK_ENTRY)(void  *pData);

typedef struct
{
	OMCI_TASK_ID    taskId;
	CHAR            name[OMCI_TASK_NAME_MAX_LEN];
	OMCI_APPL_ID    applId;
	OMCI_TASK_ENTRY pEntryFn;
    void            *pFnArg;
	UINT32          taskNumber;
	OMCI_MSG_Q_ID   msgQId;
	UINT32          msgQKey;
	UINT32 			priority;
} OMCI_TASK_INFO_T;

#define OMCI_MAX_TASKS_PER_APPL        (8)

typedef UINT32 OMCI_APPL_PID;
typedef GOS_ERROR_CODE (*OMCI_APPL_INIT_PTR)(void);
typedef GOS_ERROR_CODE (*OMCI_APPL_DEINIT_PTR)(void);
typedef GOS_ERROR_CODE (*OMCI_APPL_MSG_HANDLER_PTR)(void* pData, OMCI_MSG_TYPE type, OMCI_MSG_PRI pri, OMCI_APPL_ID srcAppl);

typedef struct
{
    CHAR                      name[OMCI_TASK_NAME_MAX_LEN];
    OMCI_APPL_ID              applId;
    OMCI_TASK_ID              mainTaskId;
    OMCI_APPL_PID             applPid;
    OMCI_TASK_INFO_T          tasks[OMCI_MAX_TASKS_PER_APPL];
    OMCI_MSG_Q_ID             msgQId;
    OMCI_APPL_INIT_PTR        init;
    OMCI_APPL_DEINIT_PTR      deinit;
    OMCI_APPL_MSG_HANDLER_PTR msgHandler;
} OMCI_APPL_INFO_T;

typedef enum
{
    OMCI_TASK_PRI_MIN       = 1,
    OMCI_TASK_PRI_TEST      = 3,
    OMCI_TASK_PRI_INTR      = 5,
	OMCI_TASK_PRI_TM		= 7,
    OMCI_TASK_PRI_PM        = 10,
    OMCI_TASK_PRI_TIMER     = 20,
    OMCI_TASK_PRI_MIB       = 30,
    OMCI_TASK_PRI_MAIN      = 50,
    OMCI_TASK_PRI_MAX       = 99

} OMCI_TASK_PRI_T;


#define OMCI_APPL 0x00000800

#define OMCI_MAX_NUM_OF_APPL           (1)

#define OMCI_APPL_ID_INVALID           ((OMCI_APPL_ID)OMCI_MAX_NUM_OF_APPL)
#define OMCI_TASK_ID_INVALID           ((OMCI_TASK_ID)ERROR)


extern GOS_ERROR_CODE OMCI_AppInit(OMCI_APPL_ID appId, const CHAR* appName);
extern GOS_ERROR_CODE OMCI_AppStart(OMCI_APPL_INIT_PTR pInitFn, OMCI_APPL_MSG_HANDLER_PTR pMsgHandlerFn, OMCI_APPL_DEINIT_PTR pDeinitFn);
OMCI_TASK_ID OMCI_SpawnTask(const CHAR          *pTaskName,
                            OMCI_TASK_ENTRY     pEntryFn,
                            void                *pFnArg,
                            UINT32              priority,
                            BOOL                needMsgQ);
extern OMCI_TASK_INFO_T* OMCI_GetTaskInfo(OMCI_TASK_ID taskId);

extern GOS_ERROR_CODE OMCI_Init(void);
extern GOS_ERROR_CODE OMCI_DeInit(void);
extern GOS_ERROR_CODE OMCI_AppInfoSet(OMCI_STATUS_INFO_ts sInfo);


#ifdef __cplusplus
}
#endif

#endif
