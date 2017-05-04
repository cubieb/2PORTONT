	.file	1 "bounds.c"
	.section .mdebug.abi32
	.previous
	.gnu_attribute 4, 3

 # -G value = 0, Arch = 5281, ISA = 1
 # GNU C (Realtek RSDK-1.5.6p2) version 4.4.6 (mips-linux)
 #	compiled by GNU C version 4.4.4, GMP version 4.2.4, MPFR version 2.3.2.
 # GGC heuristics: --param ggc-min-expand=100 --param ggc-min-heapsize=131072
 # options passed:  -nostdinc -Iinclude
 # -I/home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include
 # -Iinclude/soc -Iinclude/asm-rlx -Iarch/rlx/bsp_rtl8686/
 # -Idrivers/net/rtl86900/sdk/include/
 # -Idrivers/net/rtl86900/sdk/system/include
 # -I/home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/mach-generic
 # -imultilib 5281 -iprefix
 # /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/bin/../lib/gcc/mips-linux/4.4.6/
 # -UCONFIG_CPU_HAS_ULS -DCONFIG_CPU_HAS_ULS -D__KERNEL__
 # -DVMLINUX_LOAD_ADDRESS=0x80000000 -DLOADADDR=0x80000000 -DDATAOFFSET=0
 # -D__LUNA_KERNEL__ -UMIPSEB -U_MIPSEB -U__MIPSEB -U__MIPSEB__ -UMIPSEL
 # -U_MIPSEL -U__MIPSEL -U__MIPSEL__ -DMIPSEB -D_MIPSEB -D__MIPSEB
 # -D__MIPSEB__ -DKBUILD_STR(s)=#s -DKBUILD_BASENAME=KBUILD_STR(bounds)
 # -DKBUILD_MODNAME=KBUILD_STR(bounds) -isystem
 # /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/bin/../lib/gcc/mips-linux/4.4.6/include
 # -include include/linux/autoconf.h -include
 # /home/gangadhar/2PORTONT/sdk/autoconf.h -include include/soc/luna_cfg.h
 # -MD kernel/.bounds.s.d kernel/bounds.c -G 0 -meb -march=5281
 # -mno-check-zero-division -mabi=32 -mno-abicalls -msoft-float -mno-shared
 # -auxbase-strip kernel/bounds.s -Os -Wall -Wundef -Wstrict-prototypes
 # -Wno-trigraphs -Werror-implicit-function-declaration
 # -Wframe-larger-than=1024 -Wdeclaration-after-statement -Wno-pointer-sign
 # -ffix-bdsl -fuse-uls -fno-strict-aliasing -fno-common
 # -fno-delete-null-pointer-checks -fno-inline -ffunction-sections -fno-pic
 # -ffreestanding -fno-stack-protector -fomit-frame-pointer
 # -fno-strict-overflow -fno-dwarf2-cfi-asm -fverbose-asm
 # options enabled:  -falign-loops -fargument-alias -fauto-inc-dec
 # -fbranch-count-reg -fcaller-saves -fcprop-registers -fcrossjumping
 # -fcse-follow-jumps -fdefer-pop -fdelayed-branch -fearly-inlining
 # -feliminate-unused-debug-types -fexpensive-optimizations -ffix-bdsl
 # -fforward-propagate -ffunction-cse -ffunction-sections -fgcse -fgcse-lm
 # -fguess-branch-probability -fident -fif-conversion -fif-conversion2
 # -findirect-inlining -finline-functions -finline-functions-called-once
 # -finline-small-functions -fipa-cp -fipa-pure-const -fipa-reference
 # -fira-share-save-slots -fira-share-spill-slots -fivopts
 # -fkeep-static-consts -fleading-underscore -fmath-errno -fmerge-constants
 # -fmerge-debug-strings -fmove-loop-invariants -fomit-frame-pointer
 # -foptimize-register-move -foptimize-sibling-calls -fpcc-struct-return
 # -fpeephole -fpeephole2 -fregmove -freorder-blocks -freorder-functions
 # -frerun-cse-after-loop -fsched-interblock -fsched-spec
 # -fsched-stalled-insns-dep -fschedule-insns -fschedule-insns2
 # -fsigned-zeros -fsplit-ivs-in-unroller -fsplit-wide-types -fthread-jumps
 # -ftoplevel-reorder -ftrapping-math -ftree-builtin-call-dce -ftree-ccp
 # -ftree-ch -ftree-copy-prop -ftree-copyrename -ftree-cselim -ftree-dce
 # -ftree-dominator-opts -ftree-dse -ftree-fre -ftree-loop-im
 # -ftree-loop-ivcanon -ftree-loop-optimize -ftree-parallelize-loops=
 # -ftree-pre -ftree-reassoc -ftree-scev-cprop -ftree-sink -ftree-sra
 # -ftree-switch-conversion -ftree-ter -ftree-vect-loop-version -ftree-vrp
 # -funit-at-a-time -fuse-uls -fverbose-asm -fzero-initialized-in-bss
 # -mdivide-traps -mdouble-float -meb -mexplicit-relocs -mextern-sdata
 # -mfp-exceptions -mfp32 -mfused-madd -mgp32 -mgpopt -mlocal-sdata
 # -mlong32 -mmemcpy -mno-mips16 -mno-mips3d -msoft-float -msplit-addresses
 # -muclibc

 # Compiler executable checksum: e332c98a801dbf1ab6c552d6631fca7f

	.section	.text.foo,"ax",@progbits
	.align	2
	.globl	foo
	.set	nomips16
	.ent	foo
	.type	foo, @function
foo:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
#APP
 # 16 "kernel/bounds.c" 1
	
->NR_PAGEFLAGS 23 __NR_PAGEFLAGS	 #
 # 0 "" 2
 # 17 "kernel/bounds.c" 1
	
->MAX_NR_ZONES 2 __MAX_NR_ZONES	 #
 # 0 "" 2
#NO_APP
	j	$31
	.end	foo
	.size	foo, .-foo
	.ident	"GCC: (Realtek RSDK-1.5.6p2) 4.4.6"
