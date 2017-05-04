/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /usr/local/dslrepos/uClinux-dist/user/boa/src/LINUX/rtl_board.h,v 1.5 2012/09/11 08:46:53 tsaitc Exp $
*
* Abstract: Board specific definitions.
*
* $Author: tsaitc $
*
* ---------------------------------------------------------------
*/
#ifndef _BOARD_H_
#define _BOARD_H_

#include "rtl_types.h"

/* Define flash device 
*/
#undef FLASH_AM29LV800BB   /* only use 1MB currently */
#define FLASH_AM29LV160BB
#ifdef CONFIG_MMU
#define FLASH_BASE          0
#else
#define FLASH_BASE          0xBFC00000
#endif

#endif /* _BOARD_H_ */
