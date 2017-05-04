#include <cpu/cpu.h>

#ifndef __ASSEMBLER__
#define PLR_VERSION           (0x03000500)  //Formal release version: Bismarch 3.5

#define SECTION_CMD_NODE __attribute__ ((section (".cli_node")))
#define SECTION_CMD_MP_NODE __attribute__ ((section (".cli_mp_node")))
#define SECTION_CMD_TAIL __attribute__ ((section (".cli_tail")))
#define SECTION_AUTOK __attribute__ ((section (".flash_text")))
#include <inline_util.h>

#define _soc (*(soc_t *)(OTTO_SRAM_START+OTTO_HEADER_OFFSET))
#include <bios_io.h>
#include <plr_sections.h>
#include <util.h>

// system parameters
#include <cg/cg.h>
#define SECTION_CG_CORE_INIT      UTIL_FAR SECTION_UNS_TEXT
#define SECTION_CG_MISC      SECTION_TEXT
#define SECTION_CG_MISC_DATA SECTION_RO
extern const cg_dev_freq_t cg_ddr2_proj_freq;
extern const cg_dev_freq_t cg_ddr3_proj_freq;
extern u32_t uart_baud_rate;
#define SECTION_UART SECTION_SRAM_TEXT


/* NOR SPI-F driver uses udelay. Putting it on flash causes MMIO/PIO conflicts. */
#define SECTION_SYS  SECTION_SRAM_TEXT

/* For SPI-F driver. */
#define SECTION_NOR_SPIF_PROBE_FUNC    __attribute__ ((section (".nor_spif_probe_func")))
#define SECTION_NOR_SPIF_GEN2_CORE     __attribute__ ((section (".sram_text"), noinline))
#define SECTION_NOR_SPIF_GEN2_COREDATA __attribute__ ((section (".data")))
#define SECTION_NOR_SPIF_GEN2_MISC     __attribute__ ((section (".text")))
#define SECTION_NOR_SPIF_GEN2_PARAM    __attribute__ ((section (".parameters")))



#define NORSF_CHIP_NUM     (1)
#define NORSF_MMIO_4B_EN   (1)
#define NORSF_XREAD_EN     (0)
#define NORSF_WBUF_LIM_B   (128)
#define NORSF_CFLASH_BASE  (0x94000000)
#define NORSF_UCFLASH_BASE (NORSF_CFLASH_BASE | 0x20000000)
#define NORSF_CNTLR_4B_ADDR(enable) do {   \
		RMOD_PIN_STS(spi_flash_4b_en, enable); \
	} while (0)

#define SYSTEM_RESET() do { \
    *((volatile u32_t *)0xb8003268) = 0x0; \
    *((volatile u32_t *)0xb8003268) = 0x80000000;\
	} while(0)
#endif //!__ASSEMBLER

#define UBOOT_TEXT_BASE    (0x80f00000)
#define NEW_STACK_AT_DRAM  (UBOOT_TEXT_BASE-32)

#define MC_ZQC_RETRY_LIM   8

#define SWITCH_4B_ADDR_MODE()

#define TIMER_FREQ_MHZ     (cg_query_freq(CG_DEV_LX))
#define TIMER_STEP_PER_US  (4)
#define TIMER1_BASE        (0xb8003210)
#define UDELAY_TIMER_BASE  TIMER1_BASE
