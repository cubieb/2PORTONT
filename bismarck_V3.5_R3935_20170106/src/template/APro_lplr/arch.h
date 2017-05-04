#include <cpu/cpu.h>
#ifndef __ASSEMBLER__
#include <lplr_sections.h>
#include <lplr.h>
#include <inline_util.h>

#define _soc (*(soc_t *)(OTTO_SRAM_START+OTTO_HEADER_OFFSET))

#include <bios_io.h>

#define LPLR_VERSION           (0x03000500)  //Formal release version: otto 3.5
#define LPLR_CID               (0x10300787)  //[31:25]:2016, [24:16]: Week48, [15:8]:RLE0787
#define SECTION_SYS            __attribute__ ((section (".text.sys")))
#define SECTION_SPI_NAND       __attribute__ ((section (".text.spi_nand")))
#define SECTION_SPI_NAND_DATA  SECTION_RO
#define SECTION_ECC_CTRL            __attribute__ ((section (".text.ecc_ctrl")))
#define SECTION_ONFI       __attribute__ ((section (".text.onfi")))
#define SECTION_ONFI_DATA  SECTION_RO
#endif //#ifndef __ASSEMBLER__

/* FUNC - declare global function */
#define GFUNC(symbol)  \
        .text;         \
        .globl symbol; \
        .align 4;      \
        .ent symbol;   \
symbol:

/* END - mark end of function */
#define END(symbol)    \
        .end symbol

#define TIMER_FREQ_MHZ     (200)
#define TIMER_STEP_PER_US  (4)
#define TIMER1_BASE        (0xb8003210)
#define UDELAY_TIMER_BASE  TIMER1_BASE

#define ECC_G1_WITHOUT_DUMMY_READY  1
#define SNAF_G2_WITHOUT_DUMMY_READY 0

