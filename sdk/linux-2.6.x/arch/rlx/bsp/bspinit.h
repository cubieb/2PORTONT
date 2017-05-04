/*
 * Realtek Semiconductor Corp.
 *
 * bspinit.h:
 *
 * Tony Wu (tonywu@realtek.com.tw)
 * Dec. 7, 2007
 */
#ifndef __BSPINIT_H_
#define __BSPINIT_H_

.macro  kernel_entry_setup
        .set    push
        .set    noreorder
        
        
	#--- initialize and start COP3
        mfc0	$8,$12
        nop
        or		$8,0x80000000
        mtc0	$8,$12
        nop
        nop
        
        
        # IRAM off
	mtc0	$0, $20 # CCTL
	nop
	nop
	li		$8,0x00000020 
	mtc0	$8, $20
	nop
	nop

	
    # DRAM off
	mtc0	$0, $20 # CCTL
	nop
	nop
	li		$8,0x00000800 
	mtc0	$8, $20
	nop
	nop	

#ifdef CONFIG_CPU_RLX5281
        # IRAM1 off
	mtc0	$0, $20, 1 # CCTL
	nop
	nop
	li		$8,0x00000020 
	mtc0	$8, $20, 1
	nop
	nop

	
        # DRAM1 off
	mtc0	$0, $20, 1 # CCTL
	nop
	nop
	li		$8,0x00000800 
	mtc0	$8, $20, 1
	nop
	nop	
#endif
	

	#--- invalidate the icache and dcache with a 0->1 transition
	mtc0	$0, $20 # CCTL
	nop
	nop
	li		$8,0x00000003 # Invalid ICACHE and DCACHE
	mtc0	$8, $20
	nop
	nop

#if 1
	#--- load iram base and top
	la		$8,__iram
	la		$9,0x0ffffc00
	and		$8,$8,$9
	mtc3	$8,$0	# IW bas
	nop
	nop
	add	$8,$8,(CONFIG_IMEM_SIZE-1)
	mtc3	$8,$1	# IW top
	nop
	nop

	#--- Refill the IRAM with a 0->1 transition
	mtc0	$0, $20 # CCTL
	nop
	nop
	li		$8,0x00000010 # IRAM Fill
	mtc0	$8, $20
	nop
	nop

#ifdef CONFIG_CPU_RLX5281
	#--- load iram1 base and top
	la		$8,__iram
	add	$8,$8, CONFIG_IMEM_SIZE
	la		$9,0x0ffffc00
	and		$8,$8,$9
	mtc3	$8,$2	# IW bas
	nop
	nop
	add	$8,$8,(CONFIG_IMEM1_SIZE-1)
	mtc3	$8,$3	# IW top
	nop
	nop

	#--- Refill the IRAM1 with a 0->1 transition
	mtc0	$0, $20, 1 # CCTL
	nop
	nop
	li		$8,0x00000010 # IRAM Fill
	mtc0	$8, $20, 1
	nop
	nop
#endif

//#define DRAM_ENABLE

#ifdef DRAM_ENABLE
#define DRAM_SIZE		0x4000
#define HALF_DRAM_SIZE	(DRAM_SIZE>>1)
#define WORD_SZIE		4
	##############################################
	#--- load dram base and top
	la		$8,(__dram+DRAM_SIZE)
	la		$9,0x0ffffc00
	and		$8,$8,$9
	mtc3	$8,$4	# DW bas
	nop
	nop
	addiu	$8,$8,(HALF_DRAM_SIZE-1)
	mtc3	$8,$5	# DW top
	nop
	nop
	
	# DRAM on
	mtc0	$0, $20 # CCTL
	nop
	nop
	li		$8,0x00000400
	mtc0	$8, $20
	nop
	nop

	#--- load dram1 base and top
	la		$8,(__dram+DRAM_SIZE)
	add	    $8,$8,HALF_DRAM_SIZE
	la		$9,0x0ffffc00
	and		$8,$8,$9
	mtc3	$8,$6	# IW bas
	nop
	nop
	add	    $8,$8,(HALF_DRAM_SIZE-1)
	mtc3	$8,$7	# IW top
	nop
	nop

	# DRAM1 on
	mtc0	$0, $20, 1 # CCTL
	nop
	nop
	li		$8,0x00000400
	mtc0	$8, $20, 1
	nop
	nop

	#--- backup sdram to dram
	la	$10, DRAM_SIZE	#copy size
	la	$8, (__dram +DRAM_SIZE+DRAM_SIZE-WORD_SZIE) #dst
	la	$9, (__dram +DRAM_SIZE-WORD_SZIE)	#src

1:
	lw	$12, 0($9)
	sub $9, $9, WORD_SZIE
	sub $10, $10, WORD_SZIE
	sw	$12, 0($8)
	sub $8, $8, WORD_SZIE
	bnez	$10, 1b
	nop
	nop

    # DRAM off
	mtc0	$0, $20 # CCTL
	nop
	nop
	li		$8,0x00000800 
	mtc0	$8, $20
	nop
	nop	

	# DRAM1 off
	mtc0	$0, $20, 1 # CCTL
	nop
	nop
	li		$8,0x00000800 
	mtc0	$8, $20, 1
	nop
	nop

	##############################################
	#--- load dram base and top
	la		$8,__dram
	la		$9,0x0ffffc00
	and		$8,$8,$9
	mtc3	$8,$4	# DW bas
	nop
	nop
	addiu	$8,$8,(HALF_DRAM_SIZE-1)
	mtc3	$8,$5	# DW top
	nop
	nop
	
	# DRAM on
	mtc0	$0, $20 # CCTL
	nop
	nop
	li		$8,0x00000400
	mtc0	$8, $20
	nop
	nop

	#--- load dram1 base and top
	la		$8,__dram
	add	    $8,$8,HALF_DRAM_SIZE
	la		$9,0x0ffffc00
	and		$8,$8,$9
	mtc3	$8,$6	# IW bas
	nop
	nop
	add	    $8,$8,(HALF_DRAM_SIZE-1)
	mtc3	$8,$7	# IW top
	nop
	nop

	# DRAM1 on
	mtc0	$0, $20, 1 # CCTL
	nop
	nop
	li		$8,0x00000400
	mtc0	$8, $20, 1
	nop
	nop
#endif
#endif

	.set	pop
.endm

.macro  smp_slave_setup
.endm

#endif
