/*
 * Copyright (c) 2007 Realtek Semiconductor Corporation.
 *
 * Program : Header File of RTL8672 Asic Driver
 * Abstract :
 * Author : Michael Mao-Lin Huang (michaelhuang@realtek.com.tw)
 * $Id: rtl8672_tblDrv.h,v 1.1.1.1 2010/05/05 09:00:48 jiunming Exp $
 */

#ifndef	__RTL8672_TBLDRV_H__
#define	__RTL8672_TBLDRV_H__

#include "rtl8672_tblAsicDrv.h"

enum L2_OPT
{
	OPT_AUTO = 0,
	OPT_SRAMONLY,
	OPT_SDRAMONLY,
};

int rtl8672_L2EntryLookup(unsigned char *mac,int vlanid,rtl8672_tblAsicDrv_l2Param_t *l2p);
int rtl8672_L2EntrySearch(int (*match_func)(void *, void *), void *user_data);

int32 rtl8672_addAsicL2Entry(rtl8672_tblAsicDrv_l2Param_t *l2p, uint32 option);
int32 rtl8672_delAsicL2Entry(rtl8672_tblAsicDrv_l2Param_t *l2p, uint32 option);

int32 rtl8672_addAsicL4Entry(rtl8672_tblAsicDrv_l4Param_t *l4p, uint32 option);
int32 rtl8672_delAsicL4Entry(rtl8672_tblAsicDrv_l4Param_t *l4p, uint32 option);
int rtl8672_searchAsicL4Entry(int (*match_func)(void *, void *), void *user_data);


#endif	/* __RTL8672_TBLDRV_H__ */

