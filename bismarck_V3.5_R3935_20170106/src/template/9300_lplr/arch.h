
#include <cpu/cpu.h>
#ifndef __ASSEMBLER__
#include <lplr_sections.h>
#include <lplr.h>
#include <inline_util.h>

#define _soc (*(soc_t *)(OTTO_SRAM_START+OTTO_HEADER_OFFSET))
#include <bios_io.h>

#define LPLR_VERSION		(0x00395713)  //RTLxxxx @201x/x/xx	
#define LPLR_CID		(0x0F1A0735)  //[31:25]:2015, [24:16]: Week5, [15:8]: CHIP ID
#define SECTION_SYS		__attribute__ ((section (".text.sys")))
#define SECTION_SPI_NAND	__attribute__ ((section (".text.spi_nand")))
#define SECTION_SPI_NAND_DATA	SECTION_RO
#endif

/* LFUNC - declare local function */
#define LFUNC(symbol)     \
        .text;       \
        .align 4;    \
        .ent symbol; \
symbol:

/* FUNC - declare global function */
#define GFUNC(symbol)     \
        .text;         \
        .globl symbol; \
        .align 4;      \
        .ent symbol;   \
symbol:

/* FUNC - declare global function in ROM */
#define GSFUNC(symbol)     \
        .text;         \
        .globl symbol; \
        .align 4;      \
        .ent symbol;   \
symbol:

/* END - mark end of function */
#define END(symbol)       \
        .end symbol

