#include <cpu/cpu.h>

#define SRAM_CTRL_INIT         \
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
	lui t1, 0x0003;            \
	sw  t1, 8(t3);

// TODO: following macro should be implemented
#define GET_CPU_HZ()  (600000000)


#ifndef __ASSEMBLER__
#define SECTION_CMD_NODE __attribute__ ((section (".cli_node")))
#define SECTION_CMD_MP_NODE __attribute__ ((section (".cli_mp_node")))
#define SECTION_CMD_TAIL __attribute__ ((section (".cli_tail")))

#include <inline_util.h>

#define _soc (*(soc_t *)(OTTO_SRAM_START+OTTO_HEADER_OFFSET))
#include <bios_io.h>

#define LPLR_VERSION 0x00000001

#include <plr_sections.h>
#include <util.h>
#include <cg/cg.h>
#define SECTION_CG SECTION_SRAM_TEXT
extern u32_t lx_bus_freq_mhz SECTION_SDATA;
extern u32_t uart_baud_rate SECTION_SDATA;

#endif
