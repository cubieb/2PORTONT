/*
 * Copyright (C) 2012 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: 39101 $
 * $Date: 2013-05-03 17:35:27 +0800 (週五, 03 五月 2013) $
 *
 * Purpose : Define the debug function for EPON OAM module
 *           
 *
 * Feature : Provide all debug/dump APIs
 *
 */

/*
 * Include Files
 */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "epon_oam_config.h"
#include "epon_oam_err.h"
#include "epon_oam_db.h"
#include "epon_oam_dbg.h"

#if EPON_OAM_DEBUG
/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */
static unsigned int eponOamDbgFlag = EPON_OAM_DBGFLAG_DEFAULT;
static unsigned int eponOamExtFlag = EPON_OAM_DBGEXT_DEFAULT;

/*
 * Macro Definition
 */

/*
 * Function Declaration
 */

void dump_info_oamPdu(unsigned short llid, oam_oamInfo_t *pOamInfo)
{
    EPON_OAM_PRINT(EPON_OAM_DBGFLAG_DUMP, "llid %x\n", llid);
    EPON_OAM_PRINT(EPON_OAM_DBGFLAG_DUMP, "valid %x\n", pOamInfo->valid);
    EPON_OAM_PRINT(EPON_OAM_DBGFLAG_DUMP, "oamVer %x\n", pOamInfo->oamVer);
    EPON_OAM_PRINT(EPON_OAM_DBGFLAG_DUMP, "revision %x\n", pOamInfo->revision);
    EPON_OAM_PRINT(EPON_OAM_DBGFLAG_DUMP, "state %x\n", pOamInfo->state);
    EPON_OAM_PRINT(EPON_OAM_DBGFLAG_DUMP, "oamConfig %x\n", pOamInfo->oamConfig);
    EPON_OAM_PRINT(EPON_OAM_DBGFLAG_DUMP, "oamPduConfig %x\n", pOamInfo->oamPduConfig);
    EPON_OAM_PRINT(EPON_OAM_DBGFLAG_DUMP, "oui %02x:%02x:%02x\n", pOamInfo->oui[0],
        pOamInfo->oui[1],
        pOamInfo->oui[2]);
    EPON_OAM_PRINT(EPON_OAM_DBGFLAG_DUMP, "venderSpecInfo %02x:%02x:%02x:%02x\n", pOamInfo->venderSpecInfo[0],
        pOamInfo->venderSpecInfo[1],
        pOamInfo->venderSpecInfo[2],
        pOamInfo->venderSpecInfo[3]);
}

void dump_hex_value(unsigned int flag, unsigned char *hexValue, unsigned short length)
{
    int i, j;
    EPON_OAM_PRINT(flag, 
        "     00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f\n\n");
    for(i = 0, j = 0; i < length ; i ++)
    {
        if((i % 0x10) == 0)
        {
            EPON_OAM_PRINT(flag, "%04x ", j);
            j++;
        }
        EPON_OAM_PRINT(flag, "%02x ", hexValue[i]);
        if((i % 0x10) == 0x0f)
        {
            EPON_OAM_PRINT(flag, "\n");
        }
    }
    EPON_OAM_PRINT(flag, "\n");
}

void
epon_oam_dbgFlag_set(
    unsigned int flag)
{
    eponOamDbgFlag = flag;
}

void
epon_oam_dbgFlag_get(
    unsigned int *pFlag)
{
    *pFlag = eponOamDbgFlag;
}

void
epon_oam_dbgExt_set(
    unsigned int flag)
{
    eponOamExtFlag = flag;
}

void
epon_oam_dbgExt_get(
    unsigned int *pFlag)
{
    *pFlag = eponOamExtFlag;
}

int
epon_oam_print(
    unsigned int flag,
    char *fmt,
    ...)
{
    char buffer[256];
    va_list args;

    if((eponOamDbgFlag & flag) != 0)
    {
        va_start(args, fmt);
        vsprintf(buffer, fmt, args);
        printf(buffer);
        va_end(args);
    }
}
#endif /* EPON_OAM_DEBUG */

