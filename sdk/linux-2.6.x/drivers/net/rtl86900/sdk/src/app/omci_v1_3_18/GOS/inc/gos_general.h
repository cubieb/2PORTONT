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
 * Purpose : Definition of OMCI generic OS general define
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) OMCI generic OS general define
 */

#ifndef _GOS_GENERAL_H_
#define _GOS_GENERAL_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "gos_type.h"


UINT16 GOS_Htons(UINT16 a);
UINT16 GOS_Ntohs(UINT16 b);
UINT32 GOS_Htonl(UINT32 a);
UINT32 GOS_Ntohl(UINT32 b);
UINT64 GOS_Ntohll(UINT64 c);
UINT64 GOS_Htonll(UINT64 c);
UINT64 GOS_BuffToUINT64(CHAR *pBuf, UINT32 bufSize);
void GOS_UINT64ToBuff(UINT64 value, CHAR* pBuf,  UINT32 bufSize);
UINT16 GOS_GetUINT16(UINT16* pVal);
void GOS_SetUINT16(UINT16* pDest, UINT16 src);
UINT32 GOS_GetUINT32(UINT32* pVal);
void GOS_SetUINT32(UINT32* pDest, UINT32 src);
UINT8 *GOS_HtonByte(UINT8 *pData, UINT32 len);


#ifdef __cplusplus
}
#endif

#endif
