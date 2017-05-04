/*
 * Copyright c                Realtek Semiconductor Corporation, 2002
 * All rights reserved.                                                    
 * 
 * $Header: /usr/local/dslrepos/uClinux-dist/user/boa/src/LINUX/rtl_flashdrv_api.h,v 1.6 2012/09/19 06:18:16 kaohj Exp $
 *
 * $Author: kaohj $
 *
 * Abstract:
 *
 *   Flash driver header file for export include.
 *
 *
 * 
 */

#ifndef _RTL_FLASHDRV_H_
#define _RTL_FLASHDRV_H_



#include "rtl_types.h"
#include "rtl_board.h"

typedef struct flashdriver_obj_s {
	uint32  flashSize;
	uint32  flashBaseAddress;
	uint32 *blockBaseArray_P;
	uint32  blockBaseArrayCapacity;
	uint32  blockNumber;
} flashdriver_obj_t;



/*
 * FUNCTION PROTOTYPES
 */
/*
uint32 flashdrv_init(flashdriver_obj_t * const drvObj_P);
*/
uint32 flashdrv_init();
#ifndef CONFIG_MMU
uint32 flashdrv_eraseBlock(void *startAddr_P);
#endif
uint32 flashdrv_read (void *dstAddr_P, void *srcAddr_P, uint32 size);
uint32 flashdrv_write(void *dstAddr_P, void *srcAddr_P, uint32 size);
uint32 flashdrv_updateImg(void *srcAddr_P, void *dstAddr_P, uint32 size);
int get_mtd_fd(const char *mtd_name);
int get_mtd_start(const char *mtd_name);
int get_cs_offset();
void dump_flash_mtd_info();


#endif  /* _RTL_FLASHDRV_H_ */

