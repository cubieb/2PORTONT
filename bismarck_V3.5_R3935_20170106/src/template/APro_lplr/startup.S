#include <soc.h>

#include <cpu/cpu.s>

.macro SRAM_CNTLR_INIT
_sram_cntlr_init:
	lui  t0, 0xb800
	lui  t1, %hi(OTTO_SRAM_START & 0x1fffffff)
	ori  t1, 0x0001
	li   t2, 0x0008
	sw   t1, 0x1300(t0)
	sw   t1, 0x4000(t0)
	sw   t2, 0x1304(t0)
	sw   t2, 0x4004(t0)
.endm

.macro SRAM_FILL_ZERO
_sram_fill_zero:
	li   t0, OTTO_SRAM_START
	addi t1, t0, (OTTO_SRAM_SIZE - 4)
_fill_sram_with_0:
	sw   zero, 0(t0)
	bne  t1, t0, _fill_sram_with_0
	addi t0, t0, 4
.endm

	.section .entry, "ax", @progbits
	.align 2
	.set    noreorder
	.global lplr_S_entry
	.ent    lplr_S_entry
lplr_S_entry:
	j       lplr_basic_init
	nop

	.global always_return_zerox
always_return_zerox:
	jr   ra
	move v0, zero
.end lplr_S_entry

	.section .short_init, "ax", @progbits
	.align 2
	.set    noreorder
	.ent    lplr_basic_init
lplr_basic_init:
	CPU_INIT

	CACHE_INIT

	j    lplr_c_env_prep
	nop
	.end lplr_basic_init

	.section .short_init_2, "ax", @progbits
	.align 2
	.set    noreorder
	.ent    lplr_c_env_prep
lplr_c_env_prep:
	SRAM_CNTLR_INIT

	SRAM_FILL_ZERO

	lui  sp, %hi(OTTO_LPLR_SPI_NAND_STACK_DEF)
	jal  lplr_c_entry
	ori  sp, %lo(OTTO_LPLR_SPI_NAND_STACK_DEF)

	// Jump to preloader on SRAM. Preloader may not return and change SP.
	li   a0, OTTO_SRAM_START
	jalr a0
	nop

	// Traversal the init table and no return
	la   a0, traversal_init_table
	jr   a0
	nop
	.end lplr_c_env_prep

	// for exception from offset 0x200
	.section .isr_entry_tlb_text, "ax", @progbits
	.align 2
	.set    noreorder
	.ent    lplr_isr_entry_tlb
lplr_isr_entry_tlb:
	lui  k0, %hi(OTTO_SRAM_START)
	lw   k1, (OTTO_HEADER_OFFSET + 24)(k0)
	jr   k1
	li   k1, 0
	.end lplr_isr_entry_tlb

	// for exception from offset 0x380
	.section .isr_entry_text, "ax", @progbits
	.align 2
	.set    noreorder
	.ent    lplr_isr_entry
lplr_isr_entry:
	lui  k0, %hi(OTTO_SRAM_START)
	lw   k1, (OTTO_HEADER_OFFSET + 24)(k0)
	jr   k1
	li   k1, 1
	.end lplr_isr_entry
