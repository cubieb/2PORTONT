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
 * Feature : Provide the debug flag and API macro usage
 *
 */

#ifndef __EPON_OAM_DBG_H__
#define __EPON_OAM_DBG_H__
/* 
 * Symbol Definition 
 */ 
#define EPON_OAM_DBGFLAG_NONE       (0x00000000UL)
#define EPON_OAM_DBGFLAG_INFO       (0x00000001UL)
#define EPON_OAM_DBGFLAG_WARN       (0x00000002UL)
#define EPON_OAM_DBGFLAG_ERROR      (0x00000004UL)
#define EPON_OAM_DBGFLAG_DUMP       (0x00000008UL)
#define EPON_OAM_DBGFLAG_CTC_INFO   (0x00000010UL)
#define EPON_OAM_DBGFLAG_CTC_WARN   (0x00000020UL)
#define EPON_OAM_DBGFLAG_CTC_ERROR  (0x00000040UL)
#define EPON_OAM_DBGFLAG_CTC_DUMP   (0x00000080UL)
#define EPON_OAM_DBGFLAG_TXOAM      (0x10000000UL)
#define EPON_OAM_DBGFLAG_RXOAM      (0x20000000UL)
#define EPON_OAM_DBGFLAG_DEBUG      (0x80000000UL)
#define EPON_OAM_DBGFLAG_ALL        (0xFFFFFFFFUL)

#define EPON_OAM_DBGFLAG_ORGRX      (0x08000000UL)
#define EPON_OAM_DBGFLAG_ORGTX      (0x04000000UL) 

#define EPON_OAM_DBGEXT_NONE        (0x00000000UL)
#define EPON_OAM_DBGEXT_CTC_VARSET  (0x00000001UL)
#define EPON_OAM_DBGEXT_ALL         (0xFFFFFFFFUL)

#define EPON_OAM_DBGFLAG_DEFAULT    EPON_OAM_DBGFLAG_NONE
#define EPON_OAM_DBGEXT_DEFAULT     EPON_OAM_DBGEXT_CTC_VARSET

#if EPON_OAM_DEBUG
/* 
 * Symbol Definition 
 */ 
#define DUMP_INFO_OAMPDU(llid,pOamInfo) \
    dump_info_oamPdu(llid,pOamInfo)
#define DUMP_HEX_VALUE(flag,hexValue,length) \
    dump_hex_value(flag,hexValue,length)
#define EPON_OAM_PRINT    \
    epon_oam_print


/*
 * Function Declaration
 */
extern void dump_info_oamPdu(unsigned short llid, oam_oamInfo_t *pOamInfo);
extern void dump_hex_value(unsigned int flag, unsigned char *hexValue, unsigned short length);
extern void epon_oam_dbgFlag_set(unsigned int flag);
extern void epon_oam_dbgFlag_get(unsigned int *pFlag);
extern void epon_oam_dbgExt_set(unsigned int flag);
extern void epon_oam_dbgExt_get(unsigned int *pFlag);
extern int epon_oam_print(unsigned int flag, char *fmt, ...);
#else
/* 
 * Symbol Definition 
 */ 
#define DUMP_INFO_OAMPDU(llid,pOamInfo)
#define DUMP_HEX_VALUE(flag,hexValue,length)
#define EPON_OAM_PRINT
#define epon_oam_dbgFlag_set(flag)
#define epon_oam_dbgFlag_get(pFlag)
#define epon_oam_dbgExt_set(flag)
#define epon_oam_dbgExt_get(pFlag)
#endif /* EPON_OAM_DEBUG */

#endif /* __EPON_OAM_DBG_H__ */

