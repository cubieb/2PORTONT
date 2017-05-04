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
 * Purpose : Definition of OMCI MIB related define
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) OMCI MIB related define
 */

#ifndef __MIB_H__
#define __MIB_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "gos_type.h"
#include "omci_protocol.h"
#include "omci_mib.h"
#include "omci_driver.h"


#define MIB_VERSION_NUM  (0x03)

// the max size of an entry in each table will never greater than MIB_TABLE_ENTRY_MAX_SIZE
#define MIB_TABLE_ENTRY_MAX_SIZE (1024)
// the max size of an attr in each table will never greater than MIB_TABLE_ATTR_MAX_SIZE
#define MIB_TABLE_ATTR_MAX_SIZE  (1024)
// the max attribute num of a table will never greater than MIB_TABLE_ATTR_MAX_NUM
#define MIB_TABLE_ATTR_MAX_NUM   (32)

typedef enum
{
    MIB_SET     = 0x0,    /* MIB set */
    MIB_GET     = 0x1,    /* MIB get */
    MIB_GETNEXT = 0x2,    /* MIB get next */
    MIB_DEL     = 0x3,    /* MIB delete */
    MIB_ADD     = 0x4,    /* MIB add */
} MIB_OPERA_TYPE;

typedef enum
{
    MIB_ATTR_OUT_CHAR,
    MIB_ATTR_OUT_DEC,
    MIB_ATTR_OUT_HEX,
    MIB_ATTR_OUT_UNKNOWN
} MIB_ATTR_OUT_STYLE;

typedef enum
{
    MIB_ATTR_TYPE_UINT8,  /* UINT8 */
    MIB_ATTR_TYPE_UINT16, /* UINT16 */
    MIB_ATTR_TYPE_UINT32, /* UINT32 */
    MIB_ATTR_TYPE_UINT64, /* UINT64 */
    MIB_ATTR_TYPE_STR,    /* String */
    MIB_ATTR_TYPE_TABLE,  /* Table */
} MIB_ATTR_TYPE;

typedef enum
{
    MIB_ATTR_DEF_ZERO = 0,           /* For UINT8 UINT16 UINT32 UINT64 */
    MIB_ATTR_DEF_SPACE,              /* All spaces for string */
    MIB_ATTR_DEF_EMPTY,              /* Null for string */
    MIB_ATTR_DEF_FFFF = 0xFFFFFFFF,  /* All 'F' for For UINT8 UINT16 UINT32 UINT64 */
} MIB_ATTR_DEF_VALUE;


typedef UINT32 MIB_TABLE_INDEX;
#define MIB_TABLE_UNKNOWN_INDEX    ((MIB_TABLE_INDEX)0)
#define MIB_TABLE_FIRST_INDEX      ((MIB_TABLE_INDEX)0x1)
#define MIB_TABLE_NEXT_INDEX(idx)  ((MIB_TABLE_INDEX)(idx + 1))
#define MIB_TABLE_INDEX_VALID(idx) ((idx >= MIB_TABLE_FIRST_INDEX))

typedef UINT32 MIB_ATTR_INDEX;
#define MIB_ATTR_UNKNOWN_INDEX    ((MIB_ATTR_INDEX)0)
#define MIB_ATTR_FIRST_INDEX      ((MIB_ATTR_INDEX)0x1)
#define MIB_ATTR_NEXT_INDEX(idx)  ((MIB_ATTR_INDEX)(idx + 1))

typedef UINT32 MIB_ATTRS_SET;

#define MIB_TABLE_CB_MAX_NUM     (8)

/* Get larger attribute state machine */
#define OMCI_MULGET_STATE_IDLE (0)
#define OMCI_MULGET_STATE_RUN  (1)
#define OMCI_MULGET_STATE_NUM  (2)

#define OMCI_MULGET_EVT_START  (0)
#define OMCI_MULGET_EVT_GET    (1)
#define OMCI_MULGET_EVT_NUM    (2)

/* Upload Mib state machine */

#define OMCI_MIB_UPLOAD_STATE_IDLE (0)
#define OMCI_MIB_UPLOAD_STATE_RUN  (1)
#define OMCI_MIB_UPLOAD_STATE_NUM  (2)

#define OMCI_MIB_UPLOAD_EVT_START  (0)
#define OMCI_MIB_UPLOAD_EVT_GET    (1)
#define OMCI_MIB_UPLOAD_EVT_NUM    (2)



/* omci msg to mib format buffer */
UINT8 gOmciMibRowBuff[MIB_TABLE_ENTRY_MAX_SIZE];


#define OMCI_MAX_FSM_ID (3)

typedef UINT32 omci_fsm_id;
typedef UINT32 (*OMCI_FSMEVTHDL)(void* pEvtArg);

typedef struct
{
    UINT32           curState;
    UINT32           maxState;
    UINT32           maxEvent;
    OMCI_FSMEVTHDL*  pHandlers;
    BOOL             valid;
} omci_fsm_info_ts;

omci_fsm_info_ts  gOmciFsmInfo[OMCI_MAX_FSM_ID];


typedef struct
{
    MIB_ATTR_INDEX   attrIndex;
    UINT16           maxSeqNum;
    UINT8            attrValue[MIB_TABLE_ATTR_MAX_SIZE];
    UINT32           attrSize;
    UINT32           doneSeqNum;
}omci_mulget_attr_ts;

typedef struct
{
    omci_me_class_t     classID;
    omci_me_instance_t  entityID;
    omci_mulget_attr_ts attribute[MIB_TABLE_ATTR_MAX_NUM];
} omci_mulget_info_ts;

typedef struct
{
    UINT32 curChannelPri;
    UINT16 doneSeqNum;
    UINT16 maxSeqNum;
} omci_mib_upload_info_ts;

#ifdef __cplusplus
}
#endif

#endif
