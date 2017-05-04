#include <config.h>
#include <asm/asm.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include <asm/arch/bspchip.h>

        .text
        .set noreorder
	.globl  get_cpu_cycle_per_ms
	.ent  get_cpu_cycle_per_ms
get_cpu_cycle_per_ms:

        /*
         * Enable CP3 and clear performance counter
         */
                /* stop counters */
                ctc3 zero, $0
                /* clear counters */
                mtc3 zero, $8
                mtc3 zero, $9
                mtc3 zero, $10
                mtc3 zero, $11
                mtc3 zero, $12
                mtc3 zero, $13
                mtc3 zero, $14
                mtc3 zero, $15
                nop
                nop
	
        /* Enable performance counter and start to test. */
        /* set counter controls:  0      1      2       3
         ** counter0 cycles        (0x08) (0x08) (0x08) (0x08)
         ** counter1 ifetches      (0x01) (0x01) (0x01)
         ** counter1 dmiss busy                         (0x0b)
         ** counter2 ld+stores     (0x06)               (0x06)
         ** counter2 icache misses        (0x02) (0x02)
         ** counter3 dcache misses (0x0a) (0x0a)        (0x0a)
         ** counter3 imiss busy                  (0x03)
        */

       li t9, 0x0a060108 /* mode 0 */
       //li t9, 0x0a020108 /* mode 1 */
       //li t9, 0x03020108 /* mode 2 */
       //li t9, 0x0a060b08 /* mode 3 */


	/* 
	 * Init timer0 
	 */
	li	t5, BSP_TC0CNT
	li	t6, 1000
	sll	t6, BSP_TCD_OFFSET

	/* 1. Disable timer 0 */
	li	t0, BSP_TCCNR
	lw	t1, 0(t0)
	li	t2, ~BSP_TC0EN
	and 	t3, t1, t2
	sw	t3, 0(t0)

	/* 2. Setting BASE div */
	li	t0, BSP_CDBR
	li	t1, ((200) << BSP_DIVF_OFFSET)
	sw	t1, 0(t0)

	/* 3. Setting Data 0 */
	li	t0, BSP_TC0DATA
	li	t1, 0xFFFFFFF0
	sw	t1, 0(t0)
	
	/* 4. Enable timer 0 */
	li	t0, BSP_TCCNR
	lw	t1, 0(t0)
	li	t2, (BSP_TC0EN | BSP_TC0MODE_TIMER)
	or	t3, t2, t1
	sw	t3, 0(t0)

        ctc3 t9, $0

1:
	lw	t7, 0(t5)	
	nop
	slt	t8, t7, t6
	bne	zero, t8, 1b
	nop
	
        /* Stop the CP3 performance counter */
        ctc3 zero, $0
        nop


        /*
         * Save the counter
         */
        mfc3 t0, $9 #counter0[48:32]
        nop
        sw t0, 0(a0)
        mfc3 t0, $8 #counter0[31:0]
        nop
        nop
        sw t0, 4(a0)
        mfc3 t0, $11 #counter1[48:32]
        nop
        sw t0, 8(a0)
        mfc3 t0, $10 #counter1[31:0]
        nop
        nop
        sw t0, 12(a0)
        mfc3 t0, $13 #counter2[48:32]
        nop
        sw t0, 16(a0)
        mfc3 t0, $12 #counter2[31:0]
        nop
        nop
        sw t0, 20(a0)
        mfc3 t0, $15 #counter3[48:32]
        nop
        sw t0, 24(a0)
        mfc3 t0, $14 #counter3[31:0]
        nop
        nop
        sw t0, 28(a0)
        nop


        jr ra
        nop

	.end get_cpu_cycle_per_ms
