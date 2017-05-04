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
 * Purpose : Definition of OMCI message queue define
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) OMCI message queue define
 */

#ifndef __OMCI_MSGQ_H__
#define __OMCI_MSGQ_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "gos_type.h"
#include "omci_util.h"

typedef UINT32 OMCI_APPL_ID;
typedef INT32 OMCI_MSG_Q_ID;


#define OMCI_MSGQKEY(applId, taskIndex) ((applId) + (taskIndex))
#define OMCI_MSG_ID_INVALID             ((OMCI_MSG_Q_ID)-1)
#define OMCI_MAX_BYTE_PER_MSG           (2000)


typedef enum
{
    OMCI_RX_OMCI_MSG,
    OMCI_TX_OMCI_MSG,
    OMCI_ALARM_MSG,
    OMCI_TIMEOUT_MSG,
    OMCI_CMD_MSG,
} OMCI_MSG_TYPE;

typedef enum
{
    OMCI_MSG_PRI_URGENT = 1,
    OMCI_MSG_PRI_NORMAL,
    OMCI_MSG_PRI_MAX
} OMCI_MSG_PRI;

typedef struct
{
    OMCI_MSG_PRI     priority;
    UINT32           srcApplId;
    OMCI_MSG_TYPE    type;
    UINT32           srcMsgQKey;
    UINT32           len;
} OMCI_MSG_HDR_T;


OMCI_MSG_HDR_T* omci_CreateMsg(OMCI_MSG_HDR_T* sendBuf, OMCI_MSG_TYPE type, OMCI_MSG_PRI pri, UINT32 srcMsgQKey, UINT32 len);
GOS_ERROR_CODE OMCI_SendToMsgQ(UINT32 msgKey, OMCI_MSG_HDR_T* pMsg, UINT32 len);
GOS_ERROR_CODE OMCI_SendMsg(UINT32 msgQKey, OMCI_MSG_TYPE type, OMCI_MSG_PRI pri, void* pData, UINT32 dataLen);
GOS_ERROR_CODE OMCI_RecvFromMsgQ(OMCI_MSG_Q_ID msgId, void* pMsgHdr, UINT32 uiMaxBytes, INT32 msgType);
OMCI_MSG_Q_ID omci_CreateMsgQ(UINT32 msgKey);
GOS_ERROR_CODE omci_DeleteMsgQ(OMCI_MSG_Q_ID msgId);

#ifdef __cplusplus
}
#endif

#endif
