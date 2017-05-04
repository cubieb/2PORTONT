#include <cpu/cpu.h>
#ifndef __ASSEMBLER__
#include <lplr_sections.h>
#include <lplr.h>
#include <inline_util.h>

#define _soc (*(soc_t *)(OTTO_SRAM_START+OTTO_HEADER_OFFSET))

#include <bios_io.h>

#define LPLR_VERSION           (0x03000500)  //Formal release version: Bismarch 3.5
#define LPLR_CID               (0x10126485)  //[31:25]:2016, [24:16]: Week18, [15:8]:RL6485
#define SECTION_SYS            __attribute__ ((section (".text.sys")))
#define SECTION_SPI_NAND       __attribute__ ((section (".text.spi_nand")))
#define SECTION_SPI_NAND_DATA  SECTION_RO
#define SECTION_ECC_CTRL            __attribute__ ((section (".text.ecc_ctrl")))
#define SECTION_ONFI       __attribute__ ((section (".text.onfi")))
#define SECTION_ONFI_DATA  SECTION_RO

#define TIMER_FREQ_MHZ     (200)
#define TIMER_STEP_PER_US  (4)
#define TIMER1_BASE        (0xb8003210)
#define UDELAY_TIMER_BASE  TIMER1_BASE
#endif
