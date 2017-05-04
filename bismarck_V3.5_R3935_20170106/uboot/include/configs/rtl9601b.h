/*
 * (C) Copyright 2008 Stefan Roese <sr@denx.de>, DENX Software Engineering
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "autoconf.h"
#ifndef DO_DEPS_ONLY
#include <asm/arch/soc.h>
#endif


#define CONFIG_SYS_HZ     (board_LX_freq_mhz() * 1000 * 1000)

//#define CONFIG_SKIP_LOWLEVEL_INIT	/* SDRAM is initialized by the bootstrap code */

#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_MONITOR_LEN		(256 << 10)
#define CONFIG_STACKSIZE		(256 << 10)
#define CONFIG_SYS_BOOTPARAMS_LEN	(128 << 10)
#define CONFIG_SYS_INIT_SP_OFFSET	0x400000
#define CONFIG_SYS_TEMP_STACK_ADDR	(0x80004000)

#define CONFIG_SYS_MALLOC_LEN		(1 << 20) /* bohungwu, 20120924, roll back to the original setting as uimage decomporessing overlaps U-Boot stack */

/* for NOR SPI flash */
#define CONFIG_CMD_SF 1

/*
 * UART
 */
//#define CONFIG_SYS_NS16550_CLK		(200*1024*1024)
//#define CONFIG_SYS_NS16550_CLK		(200*1000*1000)
#define CONFIG_BAUDRATE			115200
/* This following instructs u-boot to disable UART when br_div@soc_t=0 */
//#define CONFIG_FOLLOW_PLR_TO_DIS_UART 

#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }
#define CONFIG_UART_DIVISOR_MOD   (-1)

#define CONFIG_PRE_CONSOLE_BUFFER
#define CONFIG_PRE_CON_BUF_ADDR (0x80000000)
#define CONFIG_PRE_CON_BUF_SZ   (0x00100000)

/*
 * SDRAM
 */
#define CONFIG_SYS_SDRAM_BASE		0x80000000
//#define CONFIG_SYS_SDRAM_BASE		0xa0000000
//#define CONFIG_SYS_MBYTES_SDRAM		128
#define CONFIG_SYS_MEMTEST_START	0x80200000
#define CONFIG_SYS_MEMTEST_END		0x80400000
#define CONFIG_SYS_LOAD_ADDR		0x80400000	/* default load address */

/*
 * Commands
 */
#include <config_cmd_default.h>



#undef CONFIG_CMD_NFS
#undef CONFIG_CMD_DNS
#undef CONFIG_CMD_RARP
#undef CONFIG_CMD_SNTP
//#define CONFIG_CMD_ELF
//#define CONFIG_CMD_EEPROM

#undef CONFIG_CMD_FLASH
#undef CONFIG_CMD_IMLS 
#undef CONFIG_CMD_MISC

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_SUBNETMASK

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory		*/
#define CONFIG_SYS_PROMPT	"9601B# "		/* Monitor Command Prompt	*/
#define CONFIG_SYS_CBSIZE	512		/* Console I/O Buffer Size	*/
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE + \
				 sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS	16		/* max number of command args	*/
#define CONFIG_TIMESTAMP			/* Print image info with timestamp */
#define CONFIG_CMDLINE_EDITING			/* add command line history	*/
#define CONFIG_SYS_CONSOLE_INFO_QUIET		/* don't print console @ startup*/

/*
 * FLASH and environment organization
 */
//#define CONFIG_ENV_IS_IN_FLASH
//#define CONFIG_FLASH_NOT_MEM_MAPPED

/*
 * We need special accessor functions for the CFI FLASH driver. This
 * can be enabled via the CONFIG_CFI_FLASH_USE_WEAK_ACCESSORS option.
 */
//#define CONFIG_CFI_FLASH_USE_WEAK_ACCESSORS

/*
 * For the non-memory-mapped NOR FLASH, we need to define the
 * NOR FLASH area. This can't be detected via the addr2info()
 * function, since we check for flash access in the very early
 * U-Boot code, before the NOR FLASH is detected.
 */
//#define CONFIG_FLASH_BASE		0xbd000000
//#define CONFIG_FLASH_END		0xbdffffff

/* SPI flash setting */
#define CONFIG_SPINOR_FLASH 1
#define CONFIG_SYS_NO_FLASH
#define CONFIG_SYS_FLASH_BASE (0xbd000000)
#define CONFIG_ENV_IS_IN_SPI_FLASH

#define OTTO21_TEMP_KER_OFFSET (544 * 1024)

#define CONFIG_ENV_OFFSET (256 * 1024)
#define CONFIG_ENV_SIZE   (8 * 1024)
#define CONFIG_ENV_OFFSET_REDUND (264 * 1024)
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT	1

#define CONFIG_ENV_SECT_SIZE  (0x1000) /* size of one complete sector */

/*
 * Cache Configuration
 */
#define CFG_DCACHE_SIZE		16384
#define CFG_ICACHE_SIZE		32768
#define CONFIG_SYS_CACHELINE_SIZE	32

#define CONFIG_BOOTCOMMAND	"bootm"
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#define CONFIG_IPADDR		192.168.1.3
#define CONFIG_SERVERIP		192.168.1.7
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_ETHADDR		00:E0:4C:86:70:01

#define SYSTEM_RESET() do { \
		*((volatile u32_t *)0xbb000044) = 0x00000004; \
	} while(0)

#ifdef CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2      "> "
#endif

#ifdef CONFIG_OTTO_FLASH_LAYOUT
#define FL_8MB  (8*1024*1024)
#define FL_8MB_CFGFS_BASE (272 *1024)
#define FL_8MB_K0_BASE    (512 *1024)
#define FL_8MB_K_SIZE     (1408*1024)
#define FL_8MB_R0_BASE    (FL_8MB_K0_BASE+FL_8MB_K_SIZE)
#define FL_8MB_R_SIZE     (((FL_8MB-FL_8MB_K0_BASE)>>1)-FL_8MB_K_SIZE)
#define FL_8MB_K1_BASE    (FL_8MB_R0_BASE+FL_8MB_R_SIZE)
#define FL_8MB_R1_BASE    (FL_8MB_K1_BASE+FL_8MB_K_SIZE)

#define FL_16MB (16*1024*1024)
#define FL_16MB_CFGFS_BASE FL_8MB_CFGFS_BASE
#define FL_16MB_K0_BASE    FL_8MB_K0_BASE
#define FL_16MB_K_SIZE     (3*1024*1024)
#define FL_16MB_R0_BASE    (FL_16MB_K0_BASE+FL_16MB_K_SIZE)
#define FL_16MB_R_SIZE     (((FL_16MB-FL_16MB_K0_BASE)>>1)-FL_16MB_K_SIZE)
#define FL_16MB_K1_BASE    (FL_16MB_R0_BASE+FL_16MB_R_SIZE)
#define FL_16MB_R1_BASE    (FL_16MB_K1_BASE+FL_16MB_K_SIZE)

#define FL_32MB (32*1024*1024)
#define FL_32MB_CFGFS_BASE FL_8MB_CFGFS_BASE
#define FL_32MB_K0_BASE    FL_8MB_K0_BASE
#define FL_32MB_K_SIZE     (6*1024*1024)
#define FL_32MB_R0_BASE    (FL_32MB_K0_BASE+FL_32MB_K_SIZE)
#define FL_32MB_R_SIZE     (((FL_32MB-FL_32MB_K0_BASE)>>1)-FL_32MB_K_SIZE)
#define FL_32MB_K1_BASE    (FL_32MB_R0_BASE+FL_32MB_R_SIZE)
#define FL_32MB_R1_BASE    (FL_32MB_K1_BASE+FL_32MB_K_SIZE)
#endif /* #ifdef CONFIG_LUNA_MULTI_BOOT */

#define NORSF_CHIP_NUM     (1)
#define NORSF_MMIO_4B_EN   (1)
#define NORSF_XREAD_EN     (0)
#define NORSF_WBUF_LIM_B   (128)
#define NORSF_CFLASH_BASE  (0x9D000000)
#define NORSF_UCFLASH_BASE (NORSF_CFLASH_BASE | 0x20000000)

#define TIMER_FREQ_MHZ     (board_LX_freq_mhz())
#define TIMER_STEP_PER_US  (4)
#define TIMER1_BASE        (0xb8003210)
#define UDELAY_TIMER_BASE  TIMER1_BASE

#endif  /* __CONFIG_H */
