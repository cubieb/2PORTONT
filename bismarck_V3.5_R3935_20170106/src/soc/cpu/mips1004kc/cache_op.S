#define __ASM__
#include <soc.h>
//#include "mips/m32c0.h"

GFUNC(cache_init_and_reset)
	CACHE_INIT
	CACHE_RESET
	ICACHE_INIT
	DCACHE_INIT
	jr	ra
END(cache_init_and_reset)


#define is_cps		1	   /* Core is part of a Coherent Processing System. */
#define gcr_addr	0xbfbf8000 /* KSEG0 address of the GCR registers */ 

GFUNC(init_icache)

	// Can be skipped if using magic simulation cache flush

	// Determine how big the I$ is
	mfc0	v0, CP0_CONFIG1		// read CP0_Config1

	// Isolate I$ Line Size
	ext	    v1, v0, CFG1_ILSHIFT, 3		// extract IL

	// Skip ahead if No I$
	beq	    v1, zero, done_icache
	nop

	li	    a2, 2
	sllv    v1, a2, v1			// Now have true I$ line size in bytes

	ext	    a0, v0, CFG1_ISSHIFT, 3		// extract IS
	li	    a2, 64
	sllv    a0, a2, a0			// I$ Sets per way

	// Config1IA == I$ Assoc - 1
	ext	    a1, v0, CFG1_IASHIFT, 3		// extract IA
	add	    a1, 1

	mul	    a0, a0, a1			// Total number of sets

	lui	    a2, 0x8000			// Get a KSeg0 address for cacheops

	// Clear TagLo/TagHi registers
	mtc0    zero, CP0_TAGLO		// write CP0_ITagLo
	mtc0    zero, CP0_TAGHI		// write CP0_ITagHi

	move    a3, a0

next_icache_tag:
	// Index Store Tag Cache Op
	// Will invalidate the tag entry, clear the lock bit, and clear the LRF bit
	cache   0x8, 0(a2)			// ICIndexStTag
	add	    a3, -1				// Decrement set counter
	bne     a3, zero, next_icache_tag
	add     a2, v1				// Get next line address

done_icache:

    jr      ra
    nop
END(init_icache)

/**************************************************************************************
* init_dcache invalidates all data cache entries
**************************************************************************************/

GFUNC(init_dcache)

	mfc0	v0, CP0_CONFIG1		// read CP0_Config1

	// Isolate D$ Line Size
	ext	    v1, v0, CFG1_DLSHIFT, 3		// extract DL

	// Skip ahead if No D$
	beq	    v1, zero, done_dcache
	nop

	li	    a2, 2
	sllv	v1, a2, v1			// Now have true D$ line size in bytes

	ext	    a0, v0, CFG1_DSSHIFT, 3		// extract DS
	li	    a2, 64
	sllv	a0, a2, a0	// D$ Sets per way

	// Config1DA == D$ Assoc - 1
	ext	    a1, v0, CFG1_DASHIFT, 3		// extract DA
	add	    a1, 1

	mul	    a0, a0, a1			// Get total number of sets

	lui	    a2, 0x8000		    // Get a KSeg0 address for cacheops

	// Clear TagLo/TagHi registers
	mtc0	zero, CP0_TAGLO		// write CP0_TagLo
	mtc0	zero, CP0_TAGHI		// write CP0_TagHi
	mtc0	zero, CP0_TAGLO, 2	// write CP0_DTagLo
	mtc0	zero, CP0_TAGHI, 2	// write CP0_DTagHi

	move	a3, a0

next_dcache_tag:
	// Index Store Tag Cache Op
	// Will invalidate the tag entry, clear the lock bit, and clear the LRF bit

    cache	0x9, 0(a2)			// DCIndexStTag
	add	    a3, -1			    // Decrement set counter

	bne	    a3, zero, next_dcache_tag
	add	    a2, v1		    	// Get next line address

done_dcache:
    jr      ra
    nop
END(init_dcache)

#if 0
GFUNC(change_k0_cca)
	// NOTE! This code must be executed in KSEG1 (not KSGE0 uncached)
    // Set CCA for kseg0 to cacheable
	mfc0	v0, CP0_CONFIG	// read CP0_Config
	beqz    is_cps, set_kseg0_cca
	li	    v1, 3			// CCA for all others
	li	    v1, 5			// CCA for coherent cores

set_kseg0_cca:
    ins	    v0, v1, 0, 3	// instert K0
	mtc0	v0, CP0_CONFIG	// write CP0_Config
	jr.hb   ra
    nop

END(change_k0_cca)
#endif

GFUNC(disable_L23)
	// GCR Base: 0x50, CCA override bit and CCA = UNCACHED
        li	a1, GCR_ADDR
	lw	a0, 0x0008(a1)  // Read GCR_BASE
	li	a3, 0x50				  // Enable CCA and set to uncached
	ins 	a0, a3, 0, 8	// Insert bits
	sw	a0, 0x0008(a1)  // Write GCR_BASE
	jr		ra
	nop
END(disable_L23)

GFUNC(bypass_L2)
	mfc0	a0, CP0_CONFIG2
	li	a1, 1
	ins	a0, a1, 12, 1
	mtc0	a0, CP0_CONFIG2
	jr	ra
	nop
END(bypass_L2)

GFUNC(invalidate_icache_all)
	mtc0	zero, CP0_TAGLO
	li	t0, CKSEG0
	li	t2, ICACHE_SIZE
	addu	t1, t0, t2
1:
	cacheop(Index_Invalidate_I, t0)
	addiu	t0, t0, CACHELINE_SIZE
	bne	t0, t1, 1b
	jr	ra
END(invalidate_icache_all)

GFUNC(writeback_invalidate_dcache_all)
	mtc0	zero, CP0_TAGLO, 2
	li	t0, CKSEG0
	li	t2, DCACHE_SIZE
	addu	t1, t0, t2
1:
	cacheop(Index_Writeback_Inv_D, t0)
	addiu	t0, t0, CACHELINE_SIZE
	bne	t0, t1, 1b
	jr	ra
END(writeback_invalidate_dcache_all)

GFUNC(writeback_invalidate_dcache_range)
	move	t0, a0
	li	t1, CACHELINE_SIZE
	move	t2, a1
wi_dc_r_loop:
	cacheop(Hit_Writeback_Inv_D, t0)
	addu	t0, t1
	bge	t2, t0, wi_dc_r_loop
	j	ra
END(writeback_invalidate_dcache_range)

GFUNC(invalidate_icache_range)
	move	t0, a0
	li	t1, CACHELINE_SIZE
	move	t2, a1
i_ic_r_loop:
	cacheop(Index_Invalidate_I, t0)
	addu	t0, t1
	bge	t2, t0, i_ic_r_loop
	j	ra
END(invalidate_icache_range)
