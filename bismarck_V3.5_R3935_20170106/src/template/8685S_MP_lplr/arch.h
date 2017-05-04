
#include <cpu/cpu.h>
#ifndef __ASSEMBLER__
#include <lplr_sections.h>
#include <lplr.h>
#include <inline_util.h>

#define _soc (*(soc_t *)(OTTO_SRAM_START+OTTO_HEADER_OFFSET))

#include <bios_io.h>

#define LPLR_VERSION           (0x03000100)  //Formal release version: otto 3.1
#define LPLR_CID               (0x0F216405)  //[31:25]:2015, [24:16]: Week33, [15:8]:RL6405
#define SECTION_SYS            __attribute__ ((section (".text.sys")))
#define SECTION_SPI_NAND       __attribute__ ((section (".text.spi_nand")))
#define SECTION_SPI_NAND_DATA  SECTION_RO
#define SECTION_ECC_CTRL            __attribute__ ((section (".text.ecc_ctrl")))
#endif

