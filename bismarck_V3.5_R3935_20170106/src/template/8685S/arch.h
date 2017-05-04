//arch.h for template/8685S (NOR SPI)

#include <cpu/cpu.h>

#ifndef __ASSEMBLER__
#define SECTION_CMD_NODE __attribute__ ((section (".cli_node")))
#define SECTION_CMD_MP_NODE __attribute__ ((section (".cli_mp_node")))
#define SECTION_CMD_TAIL __attribute__ ((section (".cli_tail")))

#define _soc (*(soc_t *)(OTTO_SRAM_START+OTTO_HEADER_OFFSET))
#include <bios_io.h>
#include <plr_sections.h>
#include <util.h>


#include <cg/cg.h>
#define SECTION_CG SECTION_SRAM_TEXT
extern const cg_info_t cg_info_proj SECTION_PARAMETERS;
extern const u32_t uart_baud_rate SECTION_SDATA;
#define GET_CPU_MHZ()    (cg_query_freq(0))
#define GET_CPU_HZ()     (GET_CPU_MHZ()*1000000)


/* NOR SPI-F driver uses udelay. Putting it on flash causes MMIO/PIO conflicts. */
#define SECTION_SYS  SECTION_SRAM_TEXT

/* For SPI-F driver. */
#define SECTION_NOR_SPIF_PROBE_FUNC    __attribute__ ((section (".nor_spif_probe_func")))
#define SECTION_NOR_SPIF_GEN2_CORE     SECTION_SRAM_TEXT
#define SECTION_NOR_SPIF_GEN2_COREDATA SECTION_DATA
#define SECTION_NOR_SPIF_GEN2_MISC     SECTION_TEXT
#define SECTION_AUTOK __attribute__ ((section (".flash_text")))

#define MC_ZQC_RETRY_LIM   8

#endif //!__ASSEMBLER__

#define OTTO_PLR_STACK_DEF     (OTTO_SRAM_START + OTTO_SRAM_SIZE - 8)

#define SYSTEM_RESET() do { \
		*((volatile u32_t *)0xb8003268) = 0x80000000; \
	} while(0)
