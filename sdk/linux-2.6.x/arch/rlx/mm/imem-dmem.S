#include <asm/asmmacro.h>

	.text
        LEAF(_imem_dmem_init)
        .set noreorder

	#--- initialize and start COP3
        mfc0    $8,$12
        nop
        nop
        or              $8,0x80000000
        mtc0    $8,$12
        nop
        nop

        #--- invalidate the IRAM with a 0->1 transition
        mtc0    $0, $20 # CCTL
        nop
        nop
        li              $8,0x00000020 # IRAM Off
        mtc0    $8, $20
        nop
        nop

        #--- invalidate the icache and dcache with a 0->1 transition
        mtc0    $0, $20 # CCTL
        nop
        nop
        li              $8,0x00000202 # Invalid ICACHE and DCACHE
        mtc0    $8, $20
        nop
        nop

        #--- load iram base and top
        la              $8,__iram
        la              $9,0x0fffc000
        and             $8,$8,$9
        mtc3    $8,$0                                                           # IW bas
        nop
        nop
#ifdef CONFIG_ARCH_CPU_RLX5281
        addiu   $8,$8,0xfff
        mtc3    $8,$1                                                           # IW top
        nop
        nop

        #--- Refill the IRAM with a 0->1 transition
        mtc0    $0, $20 # CCTL
        nop
        nop
        li              $8,0x00000010 # IRAM Fill
        mtc0    $8, $20
        nop
        nop
	#--- load iram base1 and top1
        la              $8,__iram
	add             $8,$8,0x1000
        la              $9,0x0fffc000
        and             $8,$8,$9
        mtc3    $8,$2                                                           # IW bas 1
        nop
        nop
        addiu   $8,$8,0xfff
        mtc3    $8,$3                                                           # IW top 1
        nop
        nop

        #--- Refill the IRAM with a 0->1 transition
        mtc0    $0, $20,1 # CCTL
        nop
        nop
        li              $8,0x00000010 # IRAM Fill
        mtc0    $8, $20,1
        nop
        nop

        #--- load dram base and top
        la              $8,__dram_start
        la              $9,__dram_end
        beq             $8,$9,skip_dramInit
	nop
        la              $9,0x0fffe000
        and             $8,$8,$9
        mtc3    $8,$4                                                           # DW bas
        nop
        nop
        addiu   $8,$8,0xfff
        mtc3    $8,$5                                                           # DW top
        nop
        nop
	#la              $8,__dram_start
        #la              $9,__dram_end
        #beq             $8,$9,skip_dramInit
	#nop
        #la              $9,0x0fffe000
        add		$8,$8,1
	#and             $8,$8,$9
        mtc3    $8,$6                                                           # DW bas 1
        nop
        nop
        addiu   $8,$8,0xfff
        mtc3    $8,$7                                                           # DW top 1
        nop
        nop

#else
        addiu   $8,$8,0x3fff
        mtc3    $8,$1                                                           # IW top
        nop
        nop

        #--- Refill the IRAM with a 0->1 transition
        mtc0    $0, $20 # CCTL
        nop
        nop
        li              $8,0x00000010 # IRAM Fill
        mtc0    $8, $20
        nop
        nop

        #--- load dram base and top
        la              $8,__dram_start
        la              $9,__dram_end
        beq             $8,$9,skip_dramInit
	nop
        la              $9,0x0fffe000
        and             $8,$8,$9
        mtc3    $8,$4                                                           # DW bas
        nop
        nop
        addiu   $8,$8,0x1fff
        mtc3    $8,$5                                                           # DW top
        nop
        nop
#endif
skip_dramInit:
        #--- enable icache and dcache
        mtc0    $0, $20 # CCTL
        nop
        nop

        .set    reorder
        j               $31
	END(_imem_dmem_init)


