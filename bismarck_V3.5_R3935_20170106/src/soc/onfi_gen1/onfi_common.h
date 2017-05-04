#ifndef ONFI_COMMON_H
#define ONFI_COMMON_H

#include <onfi/onfi_struct.h>
/************************************************************/
/*************** ONFI Flash Command Sets ***************/
/************************************************************/
#define CMD_READ_ID			0x90
#define CMD_READ_STATUS		0x70
#define CMD_READ_ECC_STATUS	0x7A

#define CMD_RESET           0xFF

#define CMD_PG_READ_C1		0x00

#define CMD_PG_READ_512_R2	0x01
#define CMD_PG_READ_512_R3	0x50

#define CMD_PG_READ_C2		0x30

#define CMD_PG_READ_C3		CMD_READ_STATUS

#define CMD_PG_WRITE_C1		0x80
#define CMD_PG_WRITE_C2		0x10
#define CMD_PG_WRITE_C3		CMD_READ_STATUS

#define CMD_BLK_ERASE_C1	0x60	//Auto Block Erase Setup command
#define CMD_BLK_ERASE_C2	0xd0	//CMD_ERASE_START
#define CMD_BLK_ERASE_C3	CMD_READ_STATUS	//CMD_STATUS_READ


//ONFI common code
u32_t onfi_read_id(void);
void onfi_reset_nand_chip(void);
s32_t onfi_block_erase(onfi_info_t *info, u32_t blk_page_idx);
void onfi_wait_nand_chip_ready(void);
u8_t onfi_get_status_register(void);
s32_t onfi_check_program_erase_status(void);

#endif //ONFI_COMMON_H
