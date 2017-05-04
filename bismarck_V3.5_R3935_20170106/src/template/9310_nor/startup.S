#include <soc.h>
//#include <arch.h>

#define REBASE_CALL_TO_UCFLASH(f) \
	la	t0, f;      \
	lui	t1, 0xbfc0; \
	or	t0, t0, t1; \
	jalr	t0;         \
	nop;

/* this loop is used to create a time window
	so ICE can break to debug and recover.*/
#define DEBUG_LOOP(n)	\
	li	t0, n;      \
	bnez	t0, -4;     \
	addi	t0, t0, -1;

/*get uc-sram code/data from flash*/
#define CSRAM_INIT	\
csram_init:	\
	la	t0, OTTO_SRAM_START;	\
	la	t1, OTTO_ENTRY;	\
	li	t2, %lo(end_of_sram - OTTO_SRAM_START);	\
1:	lw	t3, 0(t1);	\
	lw	t4, 4(t1);	\
	sw	t3, 0(t0);	\
	nop;	\
	sw	t4, 4(t0);	\
	nop;	\
	addi	t2, t2, -8;	\
	addi	t0, t0, 8;	\
	bgtz	t2, 1b;		\
	addi	t1, t1, 8;	\
	REBASE_CALL_TO_UCFLASH(writeback_invalidate_dcache_all);

/* Init BSS */
#define BSS_INIT	\
init_bss:	\
	la	t0, start_of_bss;	\
	la	t1, end_of_bss - 4;	\
1:	\
	sw	zero, 0(t0);	\
	bne	t0, t1, 1b;	\
	addi	t0, t0, 4

/* Init Stack Guard */
#define STACK_GUARD_INIT	\
init_stack_guard:
	la	t0, sp_bottom;	\
	addi	t1, t0, 32 - 4;	\
	li	t2, 0xcafecafe;	\
1:	\
	sw	t2, 0(t0);	\
	bne	t0, t1, 1b;	\
	addi	t0, t0, 4;

/* Init Stack Pointer */
#define STACK_INIT	\
	li      sp, OTTO_PLR_STACK_DEF;


/* Preloader Assembly Entry Function */
	.section .exc_reset, "ax", @progbits
	.align 4
	.global plr_S_entry
	.ent    plr_S_entry
plr_S_entry:
	j	plr_basic_init
	nop
	.end    plr_S_entry


	.section .flash_text, "ax", @progbits
	.align 4
	.set    noreorder
	.ent    plr_basic_init
plr_basic_init:
	// 1. initial cpu
	CPU_BASIC_INIT

	// 2. initial cache
	jal disable_L23;
	nop;

	jal	cache_init_and_reset;
	nop;

#if 0	// 1004K for 9310
	la a2,	   init_icache	// Initialize the L1 instruction cache. (Executing using I$ on return.)
	jalr a2
	nop

	// The changing of Kernel mode cacheability must be done from KSEG1
	// Since the code is executing from KSEG0 It needs to do a jump to KSEG1 change K0 and jump back to KSEG0
	la a2,		change_k0_cca
	li			a1, 0xf
	ins 		a2, a1, 29, 1 // changed to KSEG1 address by setting bit 29
	jalr a2
	nop

	la a2,	   init_dcache	// Initialize the L1 data cache
	jalr a2
	nop
#endif


	// 3. initial sram
9:	SRAM_CTRL_INIT
	CSRAM_INIT

	// 4. setup stack
	BSS_INIT
	STACK_GUARD_INIT
	STACK_INIT

	// 5. traversal the init table
	la	a0, traversal_init_table
	j	a0
	nop
	.end    plr_basic_init


    .section .isr_entry_text, "ax", @progbits
    .align 2
    .ent    lplr_isr_entry
lplr_isr_entry:
    lui     k0, %hi(OTTO_SRAM_START)
    lw      k1, (OTTO_HEADER_OFFSET + 24)(k0)
    jr      k1
    nop
    .end    lplr_isr_entry
