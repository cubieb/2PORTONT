/*
 * MTD SPI driver for ST M25Pxx (and similar) serial flash chips
 *
 * Author: Mike Lavender, mike@steroidmicros.com
 *
 * Copyright (c) 2005, Intec Automation Inc.
 *
 * Some parts are based on lart.c by Abraham Van Der Merwe
 *
 * Cleaned up and generalized based on mtd_dataflash.c
 *
 * This code is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>
#include <linux/math64.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/spi/flash.h>
#include "bspchip.h"
#include "rtk_spi.h"

#define FLASH_PAGESIZE		256

extern unsigned int SFCR, SFCSR, SFDR;
#ifdef CONFIG_RTL8676
#define SPI_REG_ADDR 0xB8000330
#define EN4B_SHIFT	19
#else
#define SPI_REG_ADDR 0xB8000100;
#define EN4B_SHIFT	6
#endif

#define dbg 
/* Flash opcodes. */
#define	OPCODE_WREN		0x06	/* Write enable */
#define	OPCODE_WRDI		0x04	/* Write enable */
#define	OPCODE_RDSR		0x05	/* Read status register */
#define	OPCODE_WRSR		0x01	/* Write status register 1 byte */
#define	OPCODE_NORM_READ	0x03	/* Read data bytes (low frequency) */
#define	OPCODE_FAST_READ	0x0b	/* Read data bytes (high frequency) */
#define	OPCODE_PP		0x02	/* Page program (up to 256 bytes) */
#define	OPCODE_BE_4K		0x20	/* Erase 4KiB block */
#define	OPCODE_BE_32K		0x52	/* Erase 32KiB block */
#define	OPCODE_CHIP_ERASE	0xc7	/* Erase whole flash chip */
#define	OPCODE_SE		0xd8	/* Sector erase (usually 64KiB) */
#define	OPCODE_RDID		0x9f	/* Read JEDEC ID */

/* Used for Macronix flashes only. */
#define OPCODE_EN4B             0xb7    /* Enter 4-byte mode */
#define OPCODE_EX4B             0xe9    /* Exit 4-byte mode */

/* Used for Spansion flashes only. */
#define OPCODE_BRWR             0x17    /* Bank register write */

/* Status Register bits. */
#define	SR_WIP			1	/* Write in progress */
#define	SR_WEL			2	/* Write enable latch */
/* meaning of other SR_* bits may differ between vendors */
#define	SR_BP0			4	/* Block protect 0 */
#define	SR_BP1			8	/* Block protect 1 */
#define	SR_BP2			0x10	/* Block protect 2 */
#define	SR_SRWD			0x80	/* SR write protect */

/* Define max times to check status register before we give up. */
#define	MAX_READY_WAIT_JIFFIES	(10 * HZ)	/* eg. M25P128 specs 6s max sector erase */
#define	CMD_SIZE		4

#ifdef CONFIG_RTK_USE_FAST_READ
#define OPCODE_READ 	OPCODE_FAST_READ
#define FAST_READ_DUMMY_BYTE 1
#else
#define OPCODE_READ 	OPCODE_NORM_READ
#define FAST_READ_DUMMY_BYTE 0
#endif

#define JEDEC_MFR(_jedec_id)    ((_jedec_id) >> 16)

static struct mtd_partition rtk867x_partitions[] = {
	{ name: "boot",	offset:  0 ,size:  CONFIG_BOOT_SIZE, mask_flags: 0 },
#ifndef CONFIG_2M_FLASH
#ifdef CONFIG_SINGLE_IMAGE
	{ name: "CS",     offset: CONFIG_BOOT_SIZE, size: 0x10000, mask_flags: 0 },
	{ name: "config", offset: CONFIG_BOOT_SIZE+0x10000, size: 0x10000,mask_flags:0},
	{ name: "rootfs", offset: CONFIG_BOOT_SIZE+0x20000, size: CONFIG_FLASH_SIZE-(CONFIG_BOOT_SIZE+0x20000),mask_flags:0},
#else	// CONFIG_DOUBLE_IMAGE
	{ name: "HW",     offset: CONFIG_BOOT_SIZE, size: 0x20000, mask_flags: 0 },
	{ name: "CS",     offset: CONFIG_BOOT_SIZE+0x20000, size: 0x20000, mask_flags: 0 },
	{ name: "tr069",  offset: CONFIG_BOOT_SIZE+0x40000, size: 0x20000, mask_flags: 0 },
	{ name: "cs-bak", offset: CONFIG_BOOT_SIZE+0x60000, size: 0x100000-CONFIG_BOOT_SIZE-0x60000, mask_flags: 0 },
	{ name: "rootfs", offset: 0x100000,size: 0x500000,mask_flags: 0 },
	{ name: "fs-bak", offset: 0x600000,size: 0x500000,mask_flags: 0 },
#endif
#else // CONFIG_2M_FLASH
	{ name: "rootfs",offset:  CONFIG_BOOT_SIZE, size:0x1F0000-CONFIG_BOOT_SIZE,mask_flags:0},
	{ name: "config",offset: 0x1F0000, size: 0x10000,mask_flags:0},
#endif
};

/****************************************************************************/

struct rtk_flash_info {
	int chip;
	struct mutex	 lock;
	struct mtd_info	 mtd;
	unsigned		 partitioned:1;
	u16              page_size;
	u16              addr_width;
	u8			     erase_opcode;
	u8			     command[CMD_SIZE + FAST_READ_DUMMY_BYTE];
};

static inline struct rtk_flash_info *mtd_to_flashinfo(struct mtd_info *mtd)
{
	return container_of(mtd, struct rtk_flash_info, mtd);
}

static void rtk_spi_reg_init(void) {
	REG32(SFCR) = (REG32(SFCR) & 0x3fffff) | SFCR_RBO_BE | SFCR_WBO_BE |
		SFCR_DIV(2) |  /* SPI_clock = DRAM clock(e.g.166)/DIV  */
		SFCR_TCS(15);
}

static void rspi_busy_wait_ready(void) {
	while ((REG32(SFCSR) & SFCSR_RDY) == 0)
		;	
}

static void rtk_spi_pio(void) {	
//	rspi_busy_wait_ready();	
//	REG32(SFCSR) = SFCSR_LEN(3) | SFCSR_CS(0);
//	rspi_busy_wait_ready();	
//	REG32(SFCSR) = SFCSR_LEN(3) | SFCSR_CS(3);
//	rspi_busy_wait_ready();	
//	REG32(SFCSR) = SFCSR_LEN(3) | SFCSR_CS(0);
//	rspi_busy_wait_ready();	
//	REG32(SFCSR) = SFCSR_LEN(3) | SFCSR_CS(3);	
}

/* */
static int rtkspi_write_then_read(int chip, u8 *cmd, int cmdlen, u8 *buf, int buflen) {		
	
	u32 word;
	int len; 

	rtk_spi_pio();
	
	while (cmdlen) {		
		rspi_busy_wait_ready();		
		len = (cmdlen >= 4) ? 4 : cmdlen;
		REG32(SFCSR) = SFCSR_CS(chip) | SFCSR_LEN(len - 1);
		word = 0;
		memcpy(&word, cmd, len);
		REG32(SFDR) = word;
		cmd += len;
		cmdlen -= len;
	}

	while (buflen) {		
		rspi_busy_wait_ready();		
		len = (buflen >= 4) ? 4 : buflen;
		REG32(SFCSR) = SFCSR_CS(chip) | SFCSR_LEN(len - 1);
		word = REG32(SFDR);
		memcpy(buf, &word, len);		
		buf += len;
		buflen -= len;
	}
	
	REG32(SFCSR) = SFCSR_CS(CSNONE);
	return 0;
}

static int rtkspi_write_data(int chip, u8 *cmd, int cmdlen, u8 *buf, int buflen) {		
	u32 word;
	int len; 

	rtk_spi_pio();
	
	while (cmdlen) {		
		rspi_busy_wait_ready();		
		len = (cmdlen >= 4) ? 4 : cmdlen;
		REG32(SFCSR) = SFCSR_CS(chip) | SFCSR_LEN(len - 1);
		word = 0;
		memcpy(&word, cmd, len);
		REG32(SFDR)  = word;
		cmd += len;
		cmdlen -= len;
	}

	while (buflen) {		
		rspi_busy_wait_ready();		
		len = (buflen >= 4) ? 4 : buflen;
		REG32(SFCSR) = SFCSR_CS(chip) | SFCSR_LEN(len - 1);
		word = 0;
		memcpy(&word, buf, len);		
		REG32(SFDR)  = word;
		buf += len;
		buflen -= len;
	}
	
	REG32(SFCSR) = SFCSR_CS(CSNONE);
	return 0;
}

static int rtkspi_write_cmd(int chip, u8 *buf, int buflen) {		
	return rtkspi_write_then_read(chip, buf, buflen, NULL, 0);
}

#if 1
/****************************************************************************/

/*
 * Internal helper functions
 */

/*
 * Read the status register, returning its value in the location
 * Return the status register value.
 * Returns negative if error occurred.
 */
static int read_sr(struct rtk_flash_info *flash)
{
	ssize_t retval;
	u8 code = OPCODE_RDSR;
	u8 val;

	retval = rtkspi_write_then_read(flash->chip, &code, 1, &val, 1);

	if (retval < 0) {
		printk(KERN_ERR "error %d reading SR\n",
				(int) retval);
		return retval;
	}

	return val;
}

/*
 * Write status register 1 byte
 * Returns negative if error occurred.
 */
static int write_sr(struct rtk_flash_info *flash, u8 val)
{
	flash->command[0] = OPCODE_WRSR;
	flash->command[1] = val;

	return rtkspi_write_cmd(flash->chip, flash->command, 2);
}

/*
 * Set write enable latch with Write Enable command.
 * Returns negative if error occurred.
 */
static inline int write_enable(struct rtk_flash_info *flash)
{
	u8	code = OPCODE_WREN;

	return rtkspi_write_cmd(flash->chip, &code, 1);
}

static inline int write_disable(struct rtk_flash_info *flash)
{
	u8	code = OPCODE_WRDI;

	return rtkspi_write_cmd(flash->chip, &code, 1);
}

static inline int set_4byte(struct rtk_flash_info *flash, u32 jedec_id, int enable)
 {
    /* controller specific */
	if (enable)
		REG32(SPI_REG_ADDR) |= (1 << EN4B_SHIFT);
	else
		REG32(SPI_REG_ADDR) &= ~(1 << EN4B_SHIFT);
    
    switch (JEDEC_MFR(jedec_id)) {
    case 0x00C2: /* MXIC */
            flash->command[0] = enable ? OPCODE_EN4B : OPCODE_EX4B;
            return rtkspi_write_cmd(flash->chip, flash->command, 1);
    default:
            /* Spansion style */
            flash->command[0] = OPCODE_BRWR;
            flash->command[1] = enable << 7;
            return rtkspi_write_cmd(flash->chip, flash->command, 2);
    }
 }

/*
 * Service routine to read status register until ready, or timeout occurs.
 * Returns non-zero if error.
 */
static int wait_till_ready(struct rtk_flash_info *flash)
{
	unsigned long deadline;
	int sr;

	deadline = jiffies + MAX_READY_WAIT_JIFFIES;

	do {
		if ((sr = read_sr(flash)) < 0)
			break;
		else if (!(sr & SR_WIP))
			return 0;

		cond_resched();

	} while (!time_after_eq(jiffies, deadline));

	return 1;
}

static void rtk_addr2cmd(struct rtk_flash_info *flash, unsigned int addr, u8 *cmd)
 {
    /* opcode is in cmd[0] */
    cmd[1] = addr >> (flash->addr_width * 8 -  8);
    cmd[2] = addr >> (flash->addr_width * 8 - 16);
    cmd[3] = addr >> (flash->addr_width * 8 - 24);
    cmd[4] = addr >> (flash->addr_width * 8 - 32);
 }
 
 static int rtk_cmdsz(struct rtk_flash_info *flash)
 {
    return 1 + flash->addr_width;
 }

/*
 * Erase the whole flash memory
 *
 * Returns 0 if successful, non-zero otherwise.
 */
static int erase_chip(struct rtk_flash_info *flash)
{
//	DEBUG(MTD_DEBUG_LEVEL3, "%s: %s %lldKiB\n",
//	      dev_name(&flash->spi->dev), __func__,
//	      (long long)(flash->mtd.size >> 10));

	/* Wait until finished previous write command. */
	if (wait_till_ready(flash))
		return 1;

	/* Send write enable, then erase commands. */
	write_enable(flash);

	/* Set up command buffer. */
	flash->command[0] = OPCODE_CHIP_ERASE;

	rtkspi_write_then_read(flash->chip, flash->command, 1, NULL, 0);

	return 0;
}

/*
 * Erase one sector of flash memory at offset ``offset'' which is any
 * address within the sector which should be erased.
 *
 * Returns 0 if successful, non-zero otherwise.
 */
static int erase_sector(struct rtk_flash_info *flash, u32 offset)
{
//	DEBUG(MTD_DEBUG_LEVEL3, "%s: %s %dKiB at 0x%08x\n",
//			dev_name(&flash->spi->dev), __func__,
//			flash->mtd.erasesize / 1024, offset);

	/* Wait until finished previous write command. */
	if (wait_till_ready(flash))
		return 1;

	/* Send write enable, then erase commands. */
	write_enable(flash);

	/* Set up command buffer. */
	flash->command[0] = flash->erase_opcode;
	rtk_addr2cmd(flash, offset, flash->command);

	rtkspi_write_cmd(flash->chip, flash->command, rtk_cmdsz(flash));

	return 0;
}

/****************************************************************************/

/*
 * MTD implementation
 */

/*
 * Erase an address range on the flash chip.  The address range may extend
 * one or more erase sectors.  Return an error is there is a problem erasing.
 */
static int rtk_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	struct rtk_flash_info *flash = mtd_to_flashinfo(mtd);
	u32 addr,len;
	uint32_t rem;

	DEBUG(MTD_DEBUG_LEVEL2, "%s %s 0x%llx, len %lld\n",
	      __func__, "at", (long long)instr->addr, (long long)instr->len);
		  
	dbg("%s %s 0x%llx, len %lld\n",
	      __func__, "at", (long long)instr->addr, (long long)instr->len);

	/* sanity checks */
	if (instr->addr + instr->len > flash->mtd.size)
		return -EINVAL;
	div_u64_rem(instr->len, mtd->erasesize, &rem);
	if (rem) {
		printk("Warning: %lld not multiple of erase size %d. Will pad to erase size\n", instr->len, mtd->erasesize);
		instr->len = (instr->len + (mtd->erasesize - 1)) & ~(mtd->erasesize - 1);
		//return -EINVAL;
	}

	addr = instr->addr;
	len = instr->len;

	mutex_lock(&flash->lock);

	/* whole-chip erase? */
	if (len == flash->mtd.size) {
		if (erase_chip(flash)) {
			instr->state = MTD_ERASE_FAILED;
			mutex_unlock(&flash->lock);
			return -EIO;
		}

	/* REVISIT in some cases we could speed up erasing large regions
	 * by using OPCODE_SE instead of OPCODE_BE_4K.  We may have set up
	 * to use "small sector erase", but that's not always optimal.
	 */

	/* "sector"-at-a-time erase */
	} else {
		while (len) {
			if (erase_sector(flash, addr)) {
				instr->state = MTD_ERASE_FAILED;
				mutex_unlock(&flash->lock);
				return -EIO;
			}

			addr += mtd->erasesize;
			len -= mtd->erasesize;
		}
	}

	mutex_unlock(&flash->lock);

	instr->state = MTD_ERASE_DONE;
	mtd_erase_callback(instr);

	return 0;
}

/*
 * Read an address range from the flash chip.  The address range
 * may be any size provided it is within the physical boundaries.
 */
static int rtk_read(struct mtd_info *mtd, loff_t from, size_t len,
	size_t *retlen, u_char *buf)
{
	struct rtk_flash_info *flash = mtd_to_flashinfo(mtd);
	DEBUG(MTD_DEBUG_LEVEL2, "%s %s 0x%08x, len %zd\n",
			__func__, "from", (u32)from, len);
			
	dbg("%s %s 0x%llx, len %lld\n",
	      __func__, "from", (long long)from, (long long)len);

	/* sanity checks */
	if (!len)
		return 0;

	if (from + len > flash->mtd.size)
		return -EINVAL;

	if (retlen)
		*retlen = 0;

	mutex_lock(&flash->lock);/* Wait till previous write/erase is done. */
	if (wait_till_ready(flash)) {
		/* REVISIT status return?? */
		mutex_unlock(&flash->lock);
		return 1;
	}

	/* FIXME switch to OPCODE_FAST_READ.  It's required for higher
	 * clocks; and at this writing, every chip this driver handles
	 * supports that opcode.
	 */

	/* Set up the write data buffer. */
	flash->command[0] = OPCODE_READ;
	rtk_addr2cmd(flash, from, flash->command);
	
	rtkspi_write_then_read(flash->chip, flash->command, rtk_cmdsz(flash), buf, len);

	*retlen = len;

	mutex_unlock(&flash->lock);

	return 0;
}

/*
 * Write an address range to the flash chip.  Data must be written in
 * FLASH_PAGESIZE chunks.  The address range may be any size provided
 * it is within the physical boundaries.
 */
static int rtk_write(struct mtd_info *mtd, loff_t to, size_t len,
	size_t *retlen, const u_char *buf)
{
	struct rtk_flash_info *flash = mtd_to_flashinfo(mtd);
	u32 page_offset, page_size;

	dbg("%s %s 0x%llx, len %lld\n",
	      __func__, "to", (long long)to, (long long)len);
		  
	if (retlen)
		*retlen = 0;

	/* sanity checks */
	if (!len)
		return(0);

	if (to + len > flash->mtd.size)
		return -EINVAL;

	mutex_lock(&flash->lock);

	/* Wait until finished previous write command. */
	if (wait_till_ready(flash)) {
		mutex_unlock(&flash->lock);
		return 1;
	}

	write_enable(flash);

	/* Set up the opcode in the write buffer. */
	flash->command[0] = OPCODE_PP;
	rtk_addr2cmd(flash, to, flash->command);

	/* what page do we start with? */
	page_offset = to % FLASH_PAGESIZE;

	/* do all the bytes fit onto one page? */
	if (page_offset + len <= FLASH_PAGESIZE) {
		
		rtkspi_write_data(flash->chip, flash->command, rtk_cmdsz(flash), buf, len);
		if (retlen)
			*retlen = len;
			
	} else {
		u32 i;

		/* the size of data remaining on the first page */
		page_size = FLASH_PAGESIZE - page_offset;
		
		/* write everything in PAGESIZE chunks */
		for (i = 0; i < len; i += page_size) {
			page_size = len - i;
			if (page_size > FLASH_PAGESIZE)
				page_size = FLASH_PAGESIZE;

			/* write the next page to flash */
			rtk_addr2cmd(flash, to + i, flash->command);

			wait_till_ready(flash);

			write_enable(flash);
			
			rtkspi_write_data(flash->chip, flash->command, rtk_cmdsz(flash), buf + i, page_size);

			if (retlen)
				*retlen += page_size;
		}
	}

	mutex_unlock(&flash->lock);

	return 0;
}
#endif




/****************************************************************************/

/*
 * SPI device driver setup and teardown
 */

struct flash_info {
	char		*name;

	/* JEDEC id zero means "no ID" (most older chips); otherwise it has
	 * a high byte of zero plus three data bytes: the manufacturer id,
	 * then a two byte device id.
	 */
	u32		jedec_id;
	u16             ext_id;

	/* The size listed here is what works with OPCODE_SE, which isn't
	 * necessarily called a "sector" by the vendor.
	 */
	unsigned	sector_size;
	u16		n_sectors;

	u16		flags;
#define	SECT_4K		0x01		/* OPCODE_BE_4K works uniformly */
};


/* NOTE: double check command sets and memory organization when you add
 * more flash chips.  This current list focusses on newer chips, which
 * have been converging on command sets which including JEDEC ID.
 */
static struct flash_info __devinitdata rtk_flash_data [] = {
	/* ==== TESTED BEGIN ====*/
	
	/* Mxic */
	{ "mx25l128", 0xc22018, 0, 64 * 1024, 256, SECT_4K, },
	
	
	/* ==== TESTED END ====*/

	/* Atmel -- some are (confusingly) marketed as "DataFlash" */
	{ "at25fs010",  0x1f6601, 0, 32 * 1024, 4, SECT_4K, },
	{ "at25fs040",  0x1f6604, 0, 64 * 1024, 8, SECT_4K, },

	{ "at25df041a", 0x1f4401, 0, 64 * 1024, 8, SECT_4K, },
	{ "at25df641",  0x1f4800, 0, 64 * 1024, 128, SECT_4K, },

	{ "at26f004",   0x1f0400, 0, 64 * 1024, 8, SECT_4K, },
	{ "at26df081a", 0x1f4501, 0, 64 * 1024, 16, SECT_4K, },
	{ "at26df161a", 0x1f4601, 0, 64 * 1024, 32, SECT_4K, },
	{ "at26df321",  0x1f4701, 0, 64 * 1024, 64, SECT_4K, },

	/* Spansion -- single (large) sector size only, at least
	 * for the chips listed here (without boot sectors).
	 */
	{ "s25sl004a", 0x010212, 0, 64 * 1024, 8, },
	{ "s25sl008a", 0x010213, 0, 64 * 1024, 16, },
	{ "s25sl016a", 0x010214, 0, 64 * 1024, 32, },
	{ "s25sl032a", 0x010215, 0, 64 * 1024, 64, },
	{ "s25sl064a", 0x010216, 0, 64 * 1024, 128, },
    { "s25sl12800", 0x012018, 0x0300, 256 * 1024, 64, },
	{ "s25sl12801", 0x012018, 0x0301, 64 * 1024, 256, },

	/* SST -- large erase sizes are "overlays", "sectors" are 4K */
	{ "sst25vf040b", 0xbf258d, 0, 64 * 1024, 8, SECT_4K, },
	{ "sst25vf080b", 0xbf258e, 0, 64 * 1024, 16, SECT_4K, },
	{ "sst25vf016b", 0xbf2541, 0, 64 * 1024, 32, SECT_4K, },
	{ "sst25vf032b", 0xbf254a, 0, 64 * 1024, 64, SECT_4K, },

	/* ST Microelectronics -- newer production may have feature updates */
	{ "m25p05",  0x202010,  0, 32 * 1024, 2, },
	{ "m25p10",  0x202011,  0, 32 * 1024, 4, },
	{ "m25p20",  0x202012,  0, 64 * 1024, 4, },
	{ "m25p40",  0x202013,  0, 64 * 1024, 8, },
	{ "m25p80",         0,  0, 64 * 1024, 16, },
	{ "m25p16",  0x202015,  0, 64 * 1024, 32, },
	{ "m25p32",  0x202016,  0, 64 * 1024, 64, },
	{ "m25p64",  0x202017,  0, 64 * 1024, 128, },
	{ "m25p128", 0x202018, 0, 256 * 1024, 64, },

	{ "m45pe80", 0x204014,  0, 64 * 1024, 16, },
	{ "m45pe16", 0x204015,  0, 64 * 1024, 32, },

	{ "m25pe80", 0x208014,  0, 64 * 1024, 16, },
	{ "m25pe16", 0x208015,  0, 64 * 1024, 32, SECT_4K, },
	
	

	/* Winbond -- w25x "blocks" are 64K, "sectors" are 4KiB */
	{ "w25x10", 0xef3011, 0, 64 * 1024, 2, SECT_4K, },
	{ "w25x20", 0xef3012, 0, 64 * 1024, 4, SECT_4K, },
	{ "w25x40", 0xef3013, 0, 64 * 1024, 8, SECT_4K, },
	{ "w25x80", 0xef3014, 0, 64 * 1024, 16, SECT_4K, },
	{ "w25x16", 0xef3015, 0, 64 * 1024, 32, SECT_4K, },
	{ "w25x32", 0xef3016, 0, 64 * 1024, 64, SECT_4K, },
	{ "w25x64", 0xef3017, 0, 64 * 1024, 128, SECT_4K, },
	
};

static struct flash_info *__devinit jedec_probe(void)
{
	int			tmp;
	u8			code = OPCODE_RDID;
	u8			id[5];
	u32			jedec;
	u16                     ext_jedec;
	struct flash_info	*info;

	/* JEDEC also defines an optional "extended device information"
	 * string for after vendor-specific data, after the three bytes
	 * we use here.  Supporting some chips might require using it.
	 */
	rtkspi_write_then_read(CS0, &code, 1, id, 5);
	
	jedec = id[0];
	jedec = jedec << 8;
	jedec |= id[1];
	jedec = jedec << 8;
	jedec |= id[2];

	ext_jedec = id[3] << 8 | id[4];
	
	
	for (tmp = 0, info = rtk_flash_data;
			tmp < ARRAY_SIZE(rtk_flash_data);
			tmp++, info++) {
		if (info->jedec_id == jedec) {
			if (info->ext_id != 0 && info->ext_id != ext_jedec)
				continue;
			return info;
		}
	}
	printk("unrecognized JEDEC id %06x\n", jedec);
	return NULL;
}

/* Testing Code, Should be remove at production */
static void TestSPIDriver(struct rtk_flash_info *f, struct flash_info *i) {
	/* Case 1: Erase Block
	     Erase last block, and check the whole block is 0xFF.
	 */
	struct erase_info einfo;	
	u32 n, retlen;
	u32 *buf1, *buf2;
	u8 tmp;
	size_t ssize;
	int retval;
	
	buf1 = kmalloc(i->sector_size, GFP_ATOMIC);
	buf2 = kmalloc(i->sector_size, GFP_ATOMIC);
	
	
	#define FLASH_VIRT_ADDR 0xBD000000
		
	printk("Test1: Erase Block (blksize=%x)-->", i->sector_size);
	memset(&einfo, 0, sizeof(einfo));
	einfo.addr = i->sector_size * (i->n_sectors - 1);
	einfo.len  = i->sector_size;
	einfo.mtd  = &f->mtd;
	//rtk_erase(&f->mtd, &einfo);
	tmp = f->erase_opcode; f->erase_opcode = OPCODE_SE;
	rtk_erase(&f->mtd, &einfo);
	//erase_sector(f, einfo.addr);
	f->erase_opcode = tmp;	
	rtk_read(&f->mtd, einfo.addr, einfo.len, &ssize, buf2);		
	for (n = 0; n < i->sector_size; n += 4) {
		u32 *data = buf2;
		if ((*data) != 0xFFFFFFFF) {
			printk("@%p = %08x\n", data, *data);
			break;
		}
	}
	
	(n != i->sector_size) ? printk("Failed\n") : printk("OK\n");
	
	/* Case 2: Write Data
	     Write data to last block and check.
	 */
	printk("Test2: Write Random -->");	
	for (n = 0; n < i->sector_size/sizeof(n); n++)
		buf1[n] = n+1;		
	
	retval = rtk_write(&f->mtd, einfo.addr, einfo.len, &ssize, buf1);
	
	rtk_read(&f->mtd, einfo.addr, einfo.len, &ssize, buf2);		
	
	if ((retval != 0) || (memcmp((void *)buf1, (void *)buf2, einfo.len))) {
		printk("Failed\n");
	} else {
		printk("OK\n");
	}
	
	/* Case 1b: Erase 4K sector
	     Continue on Test2, erase the first 4K sector and compare data from Test2
	 */
	if (i->flags & SECT_4K) {
		printk("Test1.2: Erase 4k Block -->");
		memset(&einfo, 0, sizeof(einfo));
		einfo.addr = i->sector_size * (i->n_sectors - 1);
		einfo.len  = 4096;
		einfo.mtd  = &f->mtd;
		rtk_erase(&f->mtd, &einfo);
		
		memset(buf2, 0xff, 4096);
		
		rtk_read(&f->mtd, einfo.addr, einfo.len, &ssize, buf2);		
		for (n = 0; n < i->sector_size; n += 4) {
			u32 *data = buf2;
			if ((*data) != 0xFFFFFFFF) {				
				break;
			}
		}	
		
		(n != i->sector_size) ? printk("Failed\n") : printk("OK\n");
	} /* 4k */
	
	
	printk("Test Done\n");
	while (1)
		;
		
}


/*
 * board specific setup should have ensured the SPI clock used here
 * matches what the READ command supports, at least until this driver
 * understands FAST_READ (for clocks over 25 MHz).
 */
static int __devinit rtk_spi_probe()
{

	//struct flash_platform_data	*data;
	struct rtk_flash_info			*flash;
	struct flash_info		*info;
	unsigned			i;

	info = jedec_probe();

	if (!info)
		return -ENODEV;

	flash = kzalloc(sizeof *flash, GFP_KERNEL);
	if (!flash)
		return -ENOMEM;

	mutex_init(&flash->lock);

	/*
	 * Atmel serial flash tend to power up
	 * with the software protection bits set
	 */

	if (info->jedec_id >> 16 == 0x1f) {
		write_enable(flash);
		write_sr(flash, 0);
	}

	flash->chip = CS0;
	flash->mtd.name = "rtkspi";
	flash->mtd.type = MTD_NORFLASH;
	flash->mtd.writesize = 1;
	flash->mtd.flags = MTD_CAP_NORFLASH;
	flash->mtd.size = info->sector_size * info->n_sectors;
	flash->mtd.erase = rtk_erase;
	flash->mtd.read = rtk_read;
	flash->mtd.write = rtk_write;

	/* prefer "small sector" erase if possible */
	if (info->flags & SECT_4K) {
		flash->erase_opcode = OPCODE_BE_4K;
		flash->mtd.erasesize = 4096;
	} else {
		flash->erase_opcode = OPCODE_SE;
		flash->mtd.erasesize = info->sector_size;
	}

	if (flash->mtd.size > 0x1000000) {
		flash->addr_width = 4;
		set_4byte(flash, info->jedec_id, 1);		
	} else {
		flash->addr_width = 3;
	}
	printk("%s (%lld Kbytes)\n", info->name, (long long)flash->mtd.size >> 10);

	DEBUG(MTD_DEBUG_LEVEL2,
		"mtd .name = %s, .size = 0x%llx (%lldMiB) "
			".erasesize = 0x%.8x (%uKiB) .numeraseregions = %d\n",
		flash->mtd.name,
		(long long)flash->mtd.size, (long long)(flash->mtd.size >> 20),
		flash->mtd.erasesize, flash->mtd.erasesize / 1024,
		flash->mtd.numeraseregions);

	if (flash->mtd.numeraseregions)
		for (i = 0; i < flash->mtd.numeraseregions; i++)
			DEBUG(MTD_DEBUG_LEVEL2,
				"mtd.eraseregions[%d] = { .offset = 0x%llx, "
				".erasesize = 0x%.8x (%uKiB), "
				".numblocks = %d }\n",
				i, (long long)flash->mtd.eraseregions[i].offset,
				flash->mtd.eraseregions[i].erasesize,
				flash->mtd.eraseregions[i].erasesize / 1024,
				flash->mtd.eraseregions[i].numblocks);
	
	//TestSPIDriver(flash, info);
	
	return add_mtd_partitions(&flash->mtd, rtk867x_partitions, ARRAY_SIZE(rtk867x_partitions));
}

static int __init rtk_spi_init(void)
{
	dbg("%s(%d):\n",__FUNCTION__,__LINE__);
	return rtk_spi_probe();
}


static void __exit rtk_spi_exit(void)
{
	
}


module_init(rtk_spi_init);
module_exit(rtk_spi_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mike Lavender");
MODULE_DESCRIPTION("MTD SPI driver for Realtek controller");
