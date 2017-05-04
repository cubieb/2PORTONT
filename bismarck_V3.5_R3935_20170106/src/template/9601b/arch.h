#include <cpu/cpu.h>

#ifndef __ASSEMBLER__
#include <lplr_sections.h>
#include <lplr.h>
#include <inline_util.h>
#include <bios_io.h>
#include <plr_sections.h>
#include <util.h>
#include <cg/cg.h>
#include <9601b_pad_ctrl.h>
extern u32_t uart_baud_rate SECTION_SDATA;
extern const cg_info_t cg_info_proj __attribute__ ((section (".parameters")));
#endif

#define SRAM_CTRL_INIT \
	li  t0, 0xB8001300; \
	lui t1, %hi(OTTO_SRAM_START); \
	lui t3, 0x1FFF; \
	and t1, t1, t3; \
	ori t1, t1, 0x1; /*Set (Un)mapping enable bit*/ \
	li  t3, 0xB8004000; \
	li  t2, 8; \
	sw  t1, 0(t0); /*unmapping*/ \
	sw  t2, 4(t0); /*unmapping size of default sram controller setting*/ \
	sw  t1, 0(t3); /*mapping*/ \
	sw  t2, 4(t3); /*mapping*/ \
	lui t1, 0x0003; \
	sw  t1, 8(t3);

#define GET_CPU_HZ() (cg_query_freq(0)*1000000)

#define SECTION_CMD_NODE __attribute__ ((section (".cli_node")))
#define SECTION_CMD_MP_NODE __attribute__ ((section (".cli_mp_node")))
#define SECTION_CMD_TAIL __attribute__ ((section (".cli_tail")))
#define SECTION_AUTOK __attribute__ ((section (".flash_text")))

#define _soc (*(soc_t *)(OTTO_SRAM_START+OTTO_HEADER_OFFSET))

#define LPLR_VERSION 0x00000001

/* NOR SPI-F driver uses udelay. Putting it on flash causes MMIO/PIO conflicts. */
#define SECTION_SYS __attribute__ ((section (".sram_text")))

/* For SPI-F driver. */
#define SECTION_NOR_SPIF_PROBE_FUNC    __attribute__ ((section (".nor_spif_probe_func")))
/* prevent SRAM-specific code to be inlined in FLASH-specific code. */
#define SECTION_NOR_SPIF_GEN2_CORE     __attribute__ ((section (".sram_text"), noinline, aligned(32)))
#define SECTION_NOR_SPIF_GEN2_COREDATA __attribute__ ((section (".data")))
#define SECTION_NOR_SPIF_GEN2_MISC     __attribute__ ((section (".text")))
#define SECTION_NOR_SPIF_GEN2_PARAM    __attribute__ ((section (".parameters")))

#define SECTION_CG SECTION_SRAM_TEXT

#define SECTION_UART SECTION_SRAM_TEXT

#define SYSTEM_RESET() do { \
		*((volatile u32_t *)0xbb000044) = 0x00000004; \
	} while(0)

#define NORSF_CHIP_NUM     (1)
#define NORSF_MMIO_4B_EN   (1)
#define NORSF_XREAD_EN     (0)
#define NORSF_WBUF_LIM_B   (128)
#define NORSF_CFLASH_BASE  (0x9D000000)
#define NORSF_UCFLASH_BASE (NORSF_CFLASH_BASE | 0x20000000)

#define UBOOT_TEXT_BASE    (0x80f00000)
#define NEW_STACK_AT_DRAM  (UBOOT_TEXT_BASE-32)

#define MC_ZQC_RETRY_LIM   8

#define TIMER_FREQ_MHZ     (cg_query_freq(CG_DEV_LX))
#define TIMER_STEP_PER_US  (4)
#define TIMER1_BASE        (0xb8003210)
#define UDELAY_TIMER_BASE  TIMER1_BASE
