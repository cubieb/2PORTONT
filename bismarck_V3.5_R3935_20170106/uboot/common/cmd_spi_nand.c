/*
 * Driver for NAND support, Rick Bronson
 * borrowed heavily from:
 * (c) 1999 Machine Vision Holdings, Inc.
 * (c) 1999, 2000 David Woodhouse <dwmw2@infradead.org>
 *
 * Ported 'dynenv' to 'nand env.oob' command
 * (C) 2010 Nanometrics, Inc.
 * 'dynenv' -- Dynamic environment offset in NAND OOB
 * (C) Copyright 2006-2007 OpenMoko, Inc.
 * Added 16-bit nand support
 * (C) 2004 Texas Instruments
 *
 * Copyright 2010 Freescale Semiconductor
 * The portions of this file whose copyright is held by Freescale and which
 * are not considered a derived work of GPL v2-only code may be distributed
 * and/or modified under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 */

#include <common.h>
//#include <linux/mtd/mtd.h>
#include <command.h>
//#include <watchdog.h>
//#include <malloc.h>
//#include <asm/byteorder.h>
//#include <jffs2/jffs2.h>
//#include <nand.h>

extern void spi_nand_info(void) ;
extern int spi_nand_read_write(uint32_t offset, uint32_t length, u_char *buffer, char opt);
extern int spi_nand_write_ecc(uint32_t offset, uint32_t length, u_char *buffer);
extern int spi_nand_read_ecc(uint32_t offset, uint32_t length, u_char *buffer);
extern int spi_nand_erase(uint32_t offset, uint32_t length);

int do_spi_nand(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	int ret = 0;
    //int dev = 0;    // current
	uint32_t addr;
	uint32_t off, size;
	char *cmd, *s;

	/* at least two arguments please */
	if (argc<2)
		goto usage;

	cmd = argv[1];
	if (0==strcmp(cmd, "info")) {
        spi_nand_info();
        return 0;
	} else if ((0==strncmp(cmd, "read", 4)) || 0==strncmp(cmd, "write", 5)) {
        char opt;   // 0: read, 1: write
        if (argc<5)
            goto usage;
        
        opt=strncmp(cmd, "read", 4); 
        //int rwsize;
     	addr = (ulong)simple_strtoul(argv[2], NULL, 16);
        if (addr<0x80000000 || addr>0xC0000000) { // FIXME
            puts("EE: incorrect addr.\n");
            return 0;
        }
        off = (ulong)simple_strtoul(argv[3], NULL, 16);
        size = (ulong)simple_strtoul(argv[4], NULL, 16);
        //printf("DD: addr 0x%x, off 0x%x, size 0x%x\n", addr, off, size);
        s = strchr(cmd, '.');
		if (s && !strcmp(s, ".raw")) {            
            ret = spi_nand_read_write(off, size, (u_char *)addr, opt);
        } else {
            ret = (opt)?spi_nand_write_ecc(off, size, (u_char *)addr)
                        :spi_nand_read_ecc(off, size, (u_char *)addr);
        }
        if(0==ret) puts("done\n");
        return ret;
	} else if (0==strncmp(cmd, "erase", 5)) {
	    off = (ulong)simple_strtoul(argv[2], NULL, 16);
        size = (ulong)simple_strtoul(argv[3], NULL, 16);
        ret = spi_nand_erase(off, size);
        return ret;
	}
usage:
	return cmd_usage(cmdtp);
}

U_BOOT_CMD(
	spi_nand, CONFIG_SYS_MAXARGS, 1, do_spi_nand,
	"SPI-NAND sub-system",
	"info - show available NAND devices\n"
	"spi_nand read - addr off size\n"
	"spi_nand read.raw - addr off size\n"	
	"spi_nand write - addr off size\n"
	"spi_nand write.raw - addr off size\n"	
	"    read/write 'size' bytes starting at offset 'off'\n"
	"               offset ignores 'spare' "
	"    to/from memory address 'addr'.\n"
	"    read.raw/write.raw skip ECC\n"
	"spi_nand erase off size - erase 'size' bytes "
	"from offset 'off'\n"
	"'size' and 'off' are block size alignment\n"
);

// FIXME
// do_spi_nand_boot
//U_BOOT_CMD(snboot, 4, 1, do_nandboot,
//	"boot from NAND device",
//	"[partition] | [[[loadAddr] dev] offset]"
//);

