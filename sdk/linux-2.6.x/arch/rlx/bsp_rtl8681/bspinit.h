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

#if 0
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

	
#ifdef CONFIG_USB_RTL8192SU_SOFTAP	
	#--- load dram base and top
	la		$8,__dram
	la		$9,0x0ffffc00
	and		$8,$8,$9
	mtc3	$8,$4	# DW bas
	nop
	nop
	addiu	$8,$8,0x3fff
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
#endif		
#endif

	.set	pop
.endm

.macro  smp_slave_setup
.endm

#endif
