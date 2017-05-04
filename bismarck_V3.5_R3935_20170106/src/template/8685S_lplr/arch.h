
#include <cpu/cpu.h>
#ifndef __ASSEMBLER__
#include <lplr_sections.h>
#include <lplr.h>
#include <inline_util.h>

#define _soc (*(soc_t *)(OTTO_SRAM_START+OTTO_HEADER_OFFSET))
#include <bios_io.h>

#define LPLR_VERSION 0x64051211  //RL6405 @2014/12/11
#define SECTION_SYS         __attribute__ ((section (".text.sys")))
#define SECTION_NAND_SPI    __attribute__ ((section (".text.nand_spi")))

#endif
