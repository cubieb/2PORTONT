#ifndef __AIPC_MEM_H__
#define __AIPC_MEM_H__

#include "aipc_global.h"
#include "soc_type.h"
#include "aipc_osal.h"
#include "aipc_shm.h"
#include "aipc_reg.h"
#include "aipc_define.h"

/*****************************************************************************
*  Define
*****************************************************************************/
// SRAM
#define SRAM_SEG_IDX_0			(0x0)
#define SRAM_SEG_IDX_1			(0x1)
#define SRAM_SEG_IDX_2			(0x2)
#define SRAM_SEG_IDX_3			(0x3)
#define SRAM_SEG_MAX			(0x4)
#define SRAM_SEG_ENABLE     	(0x1)
#define SRAM_LX_MATCH_ENABLE	(0x1)
	
#define SRAM_SIZE_256B      	(0x1)
#define SRAM_SIZE_512B      	(0x2)
#define SRAM_SIZE_1KB       	(0x3)
#define SRAM_SIZE_2KB       	(0x4)
#define SRAM_SIZE_4KB       	(0x5)
#define SRAM_SIZE_8KB       	(0x6)
#define SRAM_SIZE_16KB      	(0x7)
#define SRAM_SIZE_32KB      	(0x8)
#define SRAM_SIZE_64KB      	(0x9)
#define SRAM_SIZE_128KB     	(0xA)
#define SRAM_SIZE_256KB     	(0xB)
#define SRAM_SIZE_512KB     	(0xC)
#define SRAM_SIZE_1MB       	(0xD)

#define SRAM_SEG_ENABLE_BIT		BIT(0)
#define SRAM_LX_MATCH_BIT		BIT(31)


// DRAM
#define DRAM_SEG_IDX_0			(0x0)
#define DRAM_SEG_IDX_1			(0x1)
#define DRAM_SEG_IDX_2			(0x2)
#define DRAM_SEG_IDX_3			(0x3)
#define DRAM_SEG_MAX			(0x4)
#define DRAM_SEG_ENABLE     	(0x1)
#define DRAM_LX_MATCH_ENABLE	(0x1)

#define DRAM_SIZE_256B      	(0x1)
#define DRAM_SIZE_512B      	(0x2)
#define DRAM_SIZE_1KB       	(0x3)
#define DRAM_SIZE_2KB       	(0x4)
#define DRAM_SIZE_4KB       	(0x5)
#define DRAM_SIZE_8KB       	(0x6)
#define DRAM_SIZE_16KB      	(0x7)
#define DRAM_SIZE_32KB      	(0x8)
#define DRAM_SIZE_64KB      	(0x9)
#define DRAM_SIZE_128KB     	(0xA)
#define DRAM_SIZE_256KB     	(0xB)
#define DRAM_SIZE_512KB     	(0xC)
#define DRAM_SIZE_1MB       	(0xD)

#define DRAM_SEG_ENABLE_BIT		BIT(0)
#define DRAM_LX_MATCH_BIT		BIT(31)

// ROM
#define ROM_SEG_ENABLE     		(0x1)

#define ROM_CLOCK_DIV_NONE 		(0x0)
#define ROM_CLOCK_DIV_2    		(0x0)
#define ROM_CLOCK_DIV_4    		(0x1)

#define ROM_WAIT_TIME_NONE 		(0x0)
#define ROM_WAIT_TIME_1T   		(0x1)
#define ROM_WAIT_TIME_2T   		(0x2)
#define ROM_WAIT_TIME_3T   		(0x3)
#define ROM_WAIT_TIME_4T   		(0x4)
#define ROM_WAIT_TIME_5T   		(0x5)
#define ROM_WAIT_TIME_6T   		(0x6)
#define ROM_WAIT_TIME_7T   		(0x7)

#define ROM_SIZE_32K   			(0x8)
#define ROM_SIZE_64K   			(0x9)
#define ROM_SIZE_128K   		(0xA)
#define ROM_SIZE_256K   		(0xB)
#define ROM_SIZE_512K   		(0xC)
#define ROM_SIZE_1M   			(0xD)

#define ROM_SEG_MASK			0x7ff00000

#define ROM_SEG_ENABLE_BIT     	BIT(0)
#define ROM_CLOCK_DIV_BIT		BIT(11)

#define ROM_BASE_128K			0x20000

// #ifdef CONFIG_RTL8686_IPC_DSP_BOOTING_ADDRESS
// #define DSP_BOOT_INS_SRAM_MAPPING_ADDR	(CONFIG_RTL8686_IPC_DSP_BOOTING_ADDRESS)
//#else
//#error "CONFIG_RTL8686_IPC_DSP_BOOTING_ADDRESS is empty"
// #endif

#define DSP_BOOT_INS_SRAM_MAPPING_ADDR (0xbf100000)

#define DSP_ROMCODE_ADDR				(0x94000000)
//#define DSP_ROMCODE_ADDR				(0xB4000000)


#ifdef GPON_RESV
#define GPON_SRAM_BASE 			0x30000
//#define GPON_SEG1_OFFSET		0x2000
#else
#define GPON_SRAM_BASE 			0x0
#endif

#define WATCHDOG_ENABLE         BIT(31)

typedef enum zone_plan {
	zp_dsp_init = 0,
	zp_dsp_boot
} zone_plan_t;

/*****************************************************************************
*   Macro Definitions
*****************************************************************************/
#define aipc_outb(address, value)	writeb(value, (void*)address)
#define aipc_outw(address, value)	writew(value, (void*)address)
#define aipc_outl(address, value)	writel(value, (void*)address)
	
#define aipc_inb(address)			readb((void*)address)
#define aipc_inw(address)			readw((void*)address)
#define aipc_inl(address)			readl((void*)address)
	
#define REG8(reg)    				(*(volatile unsigned char *)((unsigned int)reg))
#ifndef REG16
#define REG16(reg)   				(*(volatile unsigned short *)(reg) )
#endif
#define REG32(reg)					(*(volatile unsigned int *)((unsigned int)reg))

#define Virtual2Physical(x)      	(((unsigned int)x) & 0x1fffffff)
#define Physical2Virtual(x)     	(((unsigned int)x) | 0x80000000)
#define Virtual2NonCache(x)      	(((unsigned int)x) | 0x20000000)
#define Physical2NonCache(x)     	(((unsigned int)x) | 0xa0000000)

#define WRITE_MEM32(addr, val)		(*(volatile unsigned int *)   (addr)) = (val)
#define READ_MEM32(addr)         	(*(volatile unsigned int *)   (addr))
#define WRITE_MEM16(addr, val)   	(*(volatile unsigned short *) (addr)) = (val)
#define READ_MEM16(addr)         	(*(volatile unsigned short *) (addr))
#define WRITE_MEM8(addr, val)    	(*(volatile unsigned char *)  (addr)) = (val)
#define READ_MEM8(addr)          	(*(volatile unsigned char *)  (addr))

/*****************************************************************************
*   Data Structure
*****************************************************************************/

/*****************************************************************************
*   Export Function
*****************************************************************************/
extern int aipc_cpu_sram_map(u8_t seg_no, u32_t map_addr, u32_t size, u8_t enable, u32_t base_addr, u8_t lx_match);
extern int aipc_cpu_dram_unmap(u8_t seg_no, u32_t unmap_addr, u32_t size, u8_t enable, u8_t lx_match);
#if 0
extern int aipc_dsp_sram_map(u8_t seg_no, u32_t map_addr, u32_t size, u8_t enable, u32_t base_addr, u8_t lx_match);
extern int aipc_dsp_dram_unmap(u8_t seg_no, u32_t unmap_addr, u32_t size, u8_t enable, u8_t lx_match);
#endif
extern int aipc_dsp_entry(void);
extern int aipc_zone_set(zone_plan_t zp);
extern int aipc_rom_set(unsigned int rom_addr);
extern int aipc_soc_sram_set(unsigned long sram_addr);
extern int aipc_soc_sram_backup(void);

/*****************************************************************************
*   Debug Function
*****************************************************************************/

#endif
