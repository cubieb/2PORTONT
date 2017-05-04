	.file	1 "asm-offsets.c"
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
 # -D__MIPSEB__ -DKBUILD_STR(s)=#s
 # -DKBUILD_BASENAME=KBUILD_STR(asm_offsets)
 # -DKBUILD_MODNAME=KBUILD_STR(asm_offsets) -isystem
 # /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/bin/../lib/gcc/mips-linux/4.4.6/include
 # -include include/linux/autoconf.h -include
 # /home/gangadhar/2PORTONT/sdk/autoconf.h -include include/soc/luna_cfg.h
 # -MD arch/rlx/kernel/.asm-offsets.s.d arch/rlx/kernel/asm-offsets.c -G 0
 # -meb -march=5281 -mno-check-zero-division -mabi=32 -mno-abicalls
 # -msoft-float -mno-shared -auxbase-strip arch/rlx/kernel/asm-offsets.s
 # -Os -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs
 # -Werror-implicit-function-declaration -Wframe-larger-than=1024
 # -Wdeclaration-after-statement -Wno-pointer-sign -ffix-bdsl -fuse-uls
 # -fno-strict-aliasing -fno-common -fno-delete-null-pointer-checks
 # -fno-inline -ffunction-sections -fno-pic -ffreestanding
 # -fno-stack-protector -fomit-frame-pointer -fno-strict-overflow
 # -fno-dwarf2-cfi-asm -fverbose-asm
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

#APP
	.macro _ssnop; sll $0, $0, 1; .endm
	.macro _ehb; sll $0, $0, 3; .endm
	.macro mtc0_tlbw_hazard; ; .endm
	.macro tlbw_use_hazard; ; .endm
	.macro tlb_probe_hazard; ; .endm
	.macro irq_enable_hazard; ; .endm
	.macro irq_disable_hazard; ; .endm
	.macro back_to_back_c0_hazard; ; .endm
		.macro	raw_local_irq_enable				
	.set	push						
	.set	reorder						
	.set	noat						
	mfc0	$1,$12						
	ori	$1,0x1f						
	xori	$1,0x1e						
	mtc0	$1,$12						
	irq_enable_hazard					
	.set	pop						
	.endm
		.macro	raw_local_irq_disable
	.set	push						
	.set	noat						
	mfc0	$1,$12						
	ori	$1,0x1f						
	xori	$1,0x1f						
	.set	noreorder					
	mtc0	$1,$12						
	irq_disable_hazard					
	.set	pop						
	.endm							

		.macro	raw_local_save_flags flags			
	.set	push						
	.set	reorder						
	mfc0	\flags, $12					
	.set	pop						
	.endm							

		.macro	raw_local_irq_save result			
	.set	push						
	.set	reorder						
	.set	noat						
	mfc0	\result, $12					
	ori	$1, \result, 0x1f				
	xori	$1, 0x1f					
	.set	noreorder					
	mtc0	$1, $12						
	irq_disable_hazard					
	.set	pop						
	.endm							

		.macro	raw_local_irq_restore flags			
	.set	push						
	.set	noreorder					
	.set	noat						
	mfc0	$1, $12						
	andi	\flags, 1					
	ori	$1, 0x1f					
	xori	$1, 0x1f					
	or	\flags, $1					
	mtc0	\flags, $12					
	irq_disable_hazard					
	.set	pop						
	.endm							

#NO_APP
	.section	.text.output_ptreg_defines,"ax",@progbits
	.align	2
	.globl	output_ptreg_defines
	.set	nomips16
	.ent	output_ptreg_defines
	.type	output_ptreg_defines, @function
output_ptreg_defines:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
#APP
 # 22 "arch/rlx/kernel/asm-offsets.c" 1
	
->#MIPS pt_regs offsets.
 # 0 "" 2
 # 23 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_R0 24 offsetof(struct pt_regs, regs[0])	 #
 # 0 "" 2
 # 24 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_R1 28 offsetof(struct pt_regs, regs[1])	 #
 # 0 "" 2
 # 25 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_R2 32 offsetof(struct pt_regs, regs[2])	 #
 # 0 "" 2
 # 26 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_R3 36 offsetof(struct pt_regs, regs[3])	 #
 # 0 "" 2
 # 27 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_R4 40 offsetof(struct pt_regs, regs[4])	 #
 # 0 "" 2
 # 28 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_R5 44 offsetof(struct pt_regs, regs[5])	 #
 # 0 "" 2
 # 29 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_R6 48 offsetof(struct pt_regs, regs[6])	 #
 # 0 "" 2
 # 30 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_R7 52 offsetof(struct pt_regs, regs[7])	 #
 # 0 "" 2
 # 31 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_R8 56 offsetof(struct pt_regs, regs[8])	 #
 # 0 "" 2
 # 32 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_R9 60 offsetof(struct pt_regs, regs[9])	 #
 # 0 "" 2
 # 33 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_R10 64 offsetof(struct pt_regs, regs[10])	 #
 # 0 "" 2
 # 34 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_R11 68 offsetof(struct pt_regs, regs[11])	 #
 # 0 "" 2
 # 35 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_R12 72 offsetof(struct pt_regs, regs[12])	 #
 # 0 "" 2
 # 36 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_R13 76 offsetof(struct pt_regs, regs[13])	 #
 # 0 "" 2
 # 37 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_R14 80 offsetof(struct pt_regs, regs[14])	 #
 # 0 "" 2
 # 38 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_R15 84 offsetof(struct pt_regs, regs[15])	 #
 # 0 "" 2
 # 39 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_R16 88 offsetof(struct pt_regs, regs[16])	 #
 # 0 "" 2
 # 40 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_R17 92 offsetof(struct pt_regs, regs[17])	 #
 # 0 "" 2
 # 41 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_R18 96 offsetof(struct pt_regs, regs[18])	 #
 # 0 "" 2
 # 42 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_R19 100 offsetof(struct pt_regs, regs[19])	 #
 # 0 "" 2
 # 43 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_R20 104 offsetof(struct pt_regs, regs[20])	 #
 # 0 "" 2
 # 44 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_R21 108 offsetof(struct pt_regs, regs[21])	 #
 # 0 "" 2
 # 45 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_R22 112 offsetof(struct pt_regs, regs[22])	 #
 # 0 "" 2
 # 46 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_R23 116 offsetof(struct pt_regs, regs[23])	 #
 # 0 "" 2
 # 47 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_R24 120 offsetof(struct pt_regs, regs[24])	 #
 # 0 "" 2
 # 48 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_R25 124 offsetof(struct pt_regs, regs[25])	 #
 # 0 "" 2
 # 49 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_R26 128 offsetof(struct pt_regs, regs[26])	 #
 # 0 "" 2
 # 50 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_R27 132 offsetof(struct pt_regs, regs[27])	 #
 # 0 "" 2
 # 51 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_R28 136 offsetof(struct pt_regs, regs[28])	 #
 # 0 "" 2
 # 52 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_R29 140 offsetof(struct pt_regs, regs[29])	 #
 # 0 "" 2
 # 53 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_R30 144 offsetof(struct pt_regs, regs[30])	 #
 # 0 "" 2
 # 54 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_R31 148 offsetof(struct pt_regs, regs[31])	 #
 # 0 "" 2
 # 55 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_HI 152 offsetof(struct pt_regs, hi)	 #
 # 0 "" 2
 # 56 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_LO 156 offsetof(struct pt_regs, lo)	 #
 # 0 "" 2
 # 57 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_EPC 160 offsetof(struct pt_regs, cp0_epc)	 #
 # 0 "" 2
 # 58 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_BVADDR 164 offsetof(struct pt_regs, cp0_badvaddr)	 #
 # 0 "" 2
 # 59 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_STATUS 168 offsetof(struct pt_regs, cp0_status)	 #
 # 0 "" 2
 # 60 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_CAUSE 172 offsetof(struct pt_regs, cp0_cause)	 #
 # 0 "" 2
 # 61 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_ESTATUS 176 offsetof(struct pt_regs, estatus)	 #
 # 0 "" 2
 # 62 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_ECAUSE 180 offsetof(struct pt_regs, ecause)	 #
 # 0 "" 2
 # 63 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_INTVEC 184 offsetof(struct pt_regs, intvec)	 #
 # 0 "" 2
 # 93 "arch/rlx/kernel/asm-offsets.c" 1
	
->PT_SIZE 192 sizeof(struct pt_regs)	 #
 # 0 "" 2
 # 94 "arch/rlx/kernel/asm-offsets.c" 1
	
->
 # 0 "" 2
#NO_APP
	j	$31
	.end	output_ptreg_defines
	.size	output_ptreg_defines, .-output_ptreg_defines
	.section	.text.output_task_defines,"ax",@progbits
	.align	2
	.globl	output_task_defines
	.set	nomips16
	.ent	output_task_defines
	.type	output_task_defines, @function
output_task_defines:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
#APP
 # 99 "arch/rlx/kernel/asm-offsets.c" 1
	
->#MIPS task_struct offsets.
 # 0 "" 2
 # 100 "arch/rlx/kernel/asm-offsets.c" 1
	
->TASK_STATE 0 offsetof(struct task_struct, state)	 #
 # 0 "" 2
 # 101 "arch/rlx/kernel/asm-offsets.c" 1
	
->TASK_THREAD_INFO 4 offsetof(struct task_struct, stack)	 #
 # 0 "" 2
 # 102 "arch/rlx/kernel/asm-offsets.c" 1
	
->TASK_FLAGS 12 offsetof(struct task_struct, flags)	 #
 # 0 "" 2
 # 103 "arch/rlx/kernel/asm-offsets.c" 1
	
->TASK_MM 216 offsetof(struct task_struct, mm)	 #
 # 0 "" 2
 # 104 "arch/rlx/kernel/asm-offsets.c" 1
	
->TASK_PID 252 offsetof(struct task_struct, pid)	 #
 # 0 "" 2
 # 105 "arch/rlx/kernel/asm-offsets.c" 1
	
->TASK_STRUCT_SIZE 776 sizeof(struct task_struct)	 #
 # 0 "" 2
 # 106 "arch/rlx/kernel/asm-offsets.c" 1
	
->
 # 0 "" 2
#NO_APP
	j	$31
	.end	output_task_defines
	.size	output_task_defines, .-output_task_defines
	.section	.text.output_thread_info_defines,"ax",@progbits
	.align	2
	.globl	output_thread_info_defines
	.set	nomips16
	.ent	output_thread_info_defines
	.type	output_thread_info_defines, @function
output_thread_info_defines:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
#APP
 # 111 "arch/rlx/kernel/asm-offsets.c" 1
	
->#MIPS thread_info offsets.
 # 0 "" 2
 # 112 "arch/rlx/kernel/asm-offsets.c" 1
	
->TI_TASK 0 offsetof(struct thread_info, task)	 #
 # 0 "" 2
 # 113 "arch/rlx/kernel/asm-offsets.c" 1
	
->TI_EXEC_DOMAIN 4 offsetof(struct thread_info, exec_domain)	 #
 # 0 "" 2
 # 114 "arch/rlx/kernel/asm-offsets.c" 1
	
->TI_FLAGS 8 offsetof(struct thread_info, flags)	 #
 # 0 "" 2
 # 115 "arch/rlx/kernel/asm-offsets.c" 1
	
->TI_TP_VALUE 12 offsetof(struct thread_info, tp_value)	 #
 # 0 "" 2
 # 116 "arch/rlx/kernel/asm-offsets.c" 1
	
->TI_CPU 16 offsetof(struct thread_info, cpu)	 #
 # 0 "" 2
 # 117 "arch/rlx/kernel/asm-offsets.c" 1
	
->TI_PRE_COUNT 20 offsetof(struct thread_info, preempt_count)	 #
 # 0 "" 2
 # 118 "arch/rlx/kernel/asm-offsets.c" 1
	
->TI_ADDR_LIMIT 24 offsetof(struct thread_info, addr_limit)	 #
 # 0 "" 2
 # 119 "arch/rlx/kernel/asm-offsets.c" 1
	
->TI_RESTART_BLOCK 32 offsetof(struct thread_info, restart_block)	 #
 # 0 "" 2
 # 120 "arch/rlx/kernel/asm-offsets.c" 1
	
->TI_REGS 64 offsetof(struct thread_info, regs)	 #
 # 0 "" 2
 # 121 "arch/rlx/kernel/asm-offsets.c" 1
	
->_THREAD_SIZE 32768 THREAD_SIZE	 #
 # 0 "" 2
 # 122 "arch/rlx/kernel/asm-offsets.c" 1
	
->_THREAD_MASK 32767 THREAD_MASK	 #
 # 0 "" 2
 # 123 "arch/rlx/kernel/asm-offsets.c" 1
	
->
 # 0 "" 2
#NO_APP
	j	$31
	.end	output_thread_info_defines
	.size	output_thread_info_defines, .-output_thread_info_defines
	.section	.text.output_thread_defines,"ax",@progbits
	.align	2
	.globl	output_thread_defines
	.set	nomips16
	.ent	output_thread_defines
	.type	output_thread_defines, @function
output_thread_defines:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
#APP
 # 128 "arch/rlx/kernel/asm-offsets.c" 1
	
->#MIPS specific thread_struct offsets.
 # 0 "" 2
 # 129 "arch/rlx/kernel/asm-offsets.c" 1
	
->THREAD_REG16 512 offsetof(struct task_struct, thread.reg16)	 #
 # 0 "" 2
 # 130 "arch/rlx/kernel/asm-offsets.c" 1
	
->THREAD_REG17 516 offsetof(struct task_struct, thread.reg17)	 #
 # 0 "" 2
 # 131 "arch/rlx/kernel/asm-offsets.c" 1
	
->THREAD_REG18 520 offsetof(struct task_struct, thread.reg18)	 #
 # 0 "" 2
 # 132 "arch/rlx/kernel/asm-offsets.c" 1
	
->THREAD_REG19 524 offsetof(struct task_struct, thread.reg19)	 #
 # 0 "" 2
 # 133 "arch/rlx/kernel/asm-offsets.c" 1
	
->THREAD_REG20 528 offsetof(struct task_struct, thread.reg20)	 #
 # 0 "" 2
 # 134 "arch/rlx/kernel/asm-offsets.c" 1
	
->THREAD_REG21 532 offsetof(struct task_struct, thread.reg21)	 #
 # 0 "" 2
 # 135 "arch/rlx/kernel/asm-offsets.c" 1
	
->THREAD_REG22 536 offsetof(struct task_struct, thread.reg22)	 #
 # 0 "" 2
 # 136 "arch/rlx/kernel/asm-offsets.c" 1
	
->THREAD_REG23 540 offsetof(struct task_struct, thread.reg23)	 #
 # 0 "" 2
 # 137 "arch/rlx/kernel/asm-offsets.c" 1
	
->THREAD_REG29 544 offsetof(struct task_struct, thread.reg29)	 #
 # 0 "" 2
 # 138 "arch/rlx/kernel/asm-offsets.c" 1
	
->THREAD_REG30 548 offsetof(struct task_struct, thread.reg30)	 #
 # 0 "" 2
 # 139 "arch/rlx/kernel/asm-offsets.c" 1
	
->THREAD_REG31 552 offsetof(struct task_struct, thread.reg31)	 #
 # 0 "" 2
 # 140 "arch/rlx/kernel/asm-offsets.c" 1
	
->THREAD_STATUS 556 offsetof(struct task_struct, thread.cp0_status)	 #
 # 0 "" 2
 # 142 "arch/rlx/kernel/asm-offsets.c" 1
	
->THREAD_BVADDR 560 offsetof(struct task_struct, thread.cp0_badvaddr)	 #
 # 0 "" 2
 # 143 "arch/rlx/kernel/asm-offsets.c" 1
	
->THREAD_BUADDR 564 offsetof(struct task_struct, thread.cp0_baduaddr)	 #
 # 0 "" 2
 # 144 "arch/rlx/kernel/asm-offsets.c" 1
	
->THREAD_ECODE 568 offsetof(struct task_struct, thread.error_code)	 #
 # 0 "" 2
 # 145 "arch/rlx/kernel/asm-offsets.c" 1
	
->
 # 0 "" 2
#NO_APP
	j	$31
	.end	output_thread_defines
	.size	output_thread_defines, .-output_thread_defines
	.section	.text.output_mm_defines,"ax",@progbits
	.align	2
	.globl	output_mm_defines
	.set	nomips16
	.ent	output_mm_defines
	.type	output_mm_defines, @function
output_mm_defines:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
#APP
 # 150 "arch/rlx/kernel/asm-offsets.c" 1
	
->#Size of struct page
 # 0 "" 2
 # 151 "arch/rlx/kernel/asm-offsets.c" 1
	
->STRUCT_PAGE_SIZE 32 sizeof(struct page)	 #
 # 0 "" 2
 # 152 "arch/rlx/kernel/asm-offsets.c" 1
	
->
 # 0 "" 2
 # 153 "arch/rlx/kernel/asm-offsets.c" 1
	
->#Linux mm_struct offsets.
 # 0 "" 2
 # 154 "arch/rlx/kernel/asm-offsets.c" 1
	
->MM_USERS 40 offsetof(struct mm_struct, mm_users)	 #
 # 0 "" 2
 # 155 "arch/rlx/kernel/asm-offsets.c" 1
	
->MM_PGD 36 offsetof(struct mm_struct, pgd)	 #
 # 0 "" 2
 # 156 "arch/rlx/kernel/asm-offsets.c" 1
	
->MM_CONTEXT 328 offsetof(struct mm_struct, context)	 #
 # 0 "" 2
 # 157 "arch/rlx/kernel/asm-offsets.c" 1
	
->
 # 0 "" 2
 # 158 "arch/rlx/kernel/asm-offsets.c" 1
	
->_PAGE_SIZE 4096 PAGE_SIZE	 #
 # 0 "" 2
 # 159 "arch/rlx/kernel/asm-offsets.c" 1
	
->_PAGE_SHIFT 12 PAGE_SHIFT	 #
 # 0 "" 2
 # 160 "arch/rlx/kernel/asm-offsets.c" 1
	
->
 # 0 "" 2
 # 161 "arch/rlx/kernel/asm-offsets.c" 1
	
->_PGD_T_SIZE 4 sizeof(pgd_t)	 #
 # 0 "" 2
 # 162 "arch/rlx/kernel/asm-offsets.c" 1
	
->_PMD_T_SIZE 4 sizeof(pmd_t)	 #
 # 0 "" 2
 # 163 "arch/rlx/kernel/asm-offsets.c" 1
	
->_PTE_T_SIZE 4 sizeof(pte_t)	 #
 # 0 "" 2
 # 164 "arch/rlx/kernel/asm-offsets.c" 1
	
->
 # 0 "" 2
 # 165 "arch/rlx/kernel/asm-offsets.c" 1
	
->_PGD_T_LOG2 2 PGD_T_LOG2	 #
 # 0 "" 2
 # 166 "arch/rlx/kernel/asm-offsets.c" 1
	
->_PMD_T_LOG2 2 PMD_T_LOG2	 #
 # 0 "" 2
 # 167 "arch/rlx/kernel/asm-offsets.c" 1
	
->_PTE_T_LOG2 2 PTE_T_LOG2	 #
 # 0 "" 2
 # 168 "arch/rlx/kernel/asm-offsets.c" 1
	
->
 # 0 "" 2
 # 169 "arch/rlx/kernel/asm-offsets.c" 1
	
->_PGD_ORDER 0 PGD_ORDER	 #
 # 0 "" 2
 # 170 "arch/rlx/kernel/asm-offsets.c" 1
	
->_PMD_ORDER 1 PMD_ORDER	 #
 # 0 "" 2
 # 171 "arch/rlx/kernel/asm-offsets.c" 1
	
->_PTE_ORDER 0 PTE_ORDER	 #
 # 0 "" 2
 # 172 "arch/rlx/kernel/asm-offsets.c" 1
	
->
 # 0 "" 2
 # 173 "arch/rlx/kernel/asm-offsets.c" 1
	
->_PMD_SHIFT 22 PMD_SHIFT	 #
 # 0 "" 2
 # 174 "arch/rlx/kernel/asm-offsets.c" 1
	
->_PGDIR_SHIFT 22 PGDIR_SHIFT	 #
 # 0 "" 2
 # 175 "arch/rlx/kernel/asm-offsets.c" 1
	
->
 # 0 "" 2
 # 176 "arch/rlx/kernel/asm-offsets.c" 1
	
->_PTRS_PER_PGD 1024 PTRS_PER_PGD	 #
 # 0 "" 2
 # 177 "arch/rlx/kernel/asm-offsets.c" 1
	
->_PTRS_PER_PMD 1 PTRS_PER_PMD	 #
 # 0 "" 2
 # 178 "arch/rlx/kernel/asm-offsets.c" 1
	
->_PTRS_PER_PTE 1024 PTRS_PER_PTE	 #
 # 0 "" 2
 # 179 "arch/rlx/kernel/asm-offsets.c" 1
	
->
 # 0 "" 2
#NO_APP
	j	$31
	.end	output_mm_defines
	.size	output_mm_defines, .-output_mm_defines
	.section	.text.output_sc_defines,"ax",@progbits
	.align	2
	.globl	output_sc_defines
	.set	nomips16
	.ent	output_sc_defines
	.type	output_sc_defines, @function
output_sc_defines:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
#APP
 # 184 "arch/rlx/kernel/asm-offsets.c" 1
	
->#Linux sigcontext offsets.
 # 0 "" 2
 # 185 "arch/rlx/kernel/asm-offsets.c" 1
	
->SC_REGS 16 offsetof(struct sigcontext, sc_regs)	 #
 # 0 "" 2
 # 186 "arch/rlx/kernel/asm-offsets.c" 1
	
->SC_MDHI 272 offsetof(struct sigcontext, sc_mdhi)	 #
 # 0 "" 2
 # 187 "arch/rlx/kernel/asm-offsets.c" 1
	
->SC_MDLO 280 offsetof(struct sigcontext, sc_mdlo)	 #
 # 0 "" 2
 # 188 "arch/rlx/kernel/asm-offsets.c" 1
	
->SC_PC 8 offsetof(struct sigcontext, sc_pc)	 #
 # 0 "" 2
 # 189 "arch/rlx/kernel/asm-offsets.c" 1
	
->
 # 0 "" 2
#NO_APP
	j	$31
	.end	output_sc_defines
	.size	output_sc_defines, .-output_sc_defines
	.section	.text.output_signal_defined,"ax",@progbits
	.align	2
	.globl	output_signal_defined
	.set	nomips16
	.ent	output_signal_defined
	.type	output_signal_defined, @function
output_signal_defined:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
#APP
 # 194 "arch/rlx/kernel/asm-offsets.c" 1
	
->#Linux signal numbers.
 # 0 "" 2
 # 195 "arch/rlx/kernel/asm-offsets.c" 1
	
->_SIGHUP 1 SIGHUP	 #
 # 0 "" 2
 # 196 "arch/rlx/kernel/asm-offsets.c" 1
	
->_SIGINT 2 SIGINT	 #
 # 0 "" 2
 # 197 "arch/rlx/kernel/asm-offsets.c" 1
	
->_SIGQUIT 3 SIGQUIT	 #
 # 0 "" 2
 # 198 "arch/rlx/kernel/asm-offsets.c" 1
	
->_SIGILL 4 SIGILL	 #
 # 0 "" 2
 # 199 "arch/rlx/kernel/asm-offsets.c" 1
	
->_SIGTRAP 5 SIGTRAP	 #
 # 0 "" 2
 # 200 "arch/rlx/kernel/asm-offsets.c" 1
	
->_SIGIOT 6 SIGIOT	 #
 # 0 "" 2
 # 201 "arch/rlx/kernel/asm-offsets.c" 1
	
->_SIGABRT 6 SIGABRT	 #
 # 0 "" 2
 # 202 "arch/rlx/kernel/asm-offsets.c" 1
	
->_SIGEMT 7 SIGEMT	 #
 # 0 "" 2
 # 203 "arch/rlx/kernel/asm-offsets.c" 1
	
->_SIGFPE 8 SIGFPE	 #
 # 0 "" 2
 # 204 "arch/rlx/kernel/asm-offsets.c" 1
	
->_SIGKILL 9 SIGKILL	 #
 # 0 "" 2
 # 205 "arch/rlx/kernel/asm-offsets.c" 1
	
->_SIGBUS 10 SIGBUS	 #
 # 0 "" 2
 # 206 "arch/rlx/kernel/asm-offsets.c" 1
	
->_SIGSEGV 11 SIGSEGV	 #
 # 0 "" 2
 # 207 "arch/rlx/kernel/asm-offsets.c" 1
	
->_SIGSYS 12 SIGSYS	 #
 # 0 "" 2
 # 208 "arch/rlx/kernel/asm-offsets.c" 1
	
->_SIGPIPE 13 SIGPIPE	 #
 # 0 "" 2
 # 209 "arch/rlx/kernel/asm-offsets.c" 1
	
->_SIGALRM 14 SIGALRM	 #
 # 0 "" 2
 # 210 "arch/rlx/kernel/asm-offsets.c" 1
	
->_SIGTERM 15 SIGTERM	 #
 # 0 "" 2
 # 211 "arch/rlx/kernel/asm-offsets.c" 1
	
->_SIGUSR1 16 SIGUSR1	 #
 # 0 "" 2
 # 212 "arch/rlx/kernel/asm-offsets.c" 1
	
->_SIGUSR2 17 SIGUSR2	 #
 # 0 "" 2
 # 213 "arch/rlx/kernel/asm-offsets.c" 1
	
->_SIGCHLD 18 SIGCHLD	 #
 # 0 "" 2
 # 214 "arch/rlx/kernel/asm-offsets.c" 1
	
->_SIGPWR 19 SIGPWR	 #
 # 0 "" 2
 # 215 "arch/rlx/kernel/asm-offsets.c" 1
	
->_SIGWINCH 20 SIGWINCH	 #
 # 0 "" 2
 # 216 "arch/rlx/kernel/asm-offsets.c" 1
	
->_SIGURG 21 SIGURG	 #
 # 0 "" 2
 # 217 "arch/rlx/kernel/asm-offsets.c" 1
	
->_SIGIO 22 SIGIO	 #
 # 0 "" 2
 # 218 "arch/rlx/kernel/asm-offsets.c" 1
	
->_SIGSTOP 23 SIGSTOP	 #
 # 0 "" 2
 # 219 "arch/rlx/kernel/asm-offsets.c" 1
	
->_SIGTSTP 24 SIGTSTP	 #
 # 0 "" 2
 # 220 "arch/rlx/kernel/asm-offsets.c" 1
	
->_SIGCONT 25 SIGCONT	 #
 # 0 "" 2
 # 221 "arch/rlx/kernel/asm-offsets.c" 1
	
->_SIGTTIN 26 SIGTTIN	 #
 # 0 "" 2
 # 222 "arch/rlx/kernel/asm-offsets.c" 1
	
->_SIGTTOU 27 SIGTTOU	 #
 # 0 "" 2
 # 223 "arch/rlx/kernel/asm-offsets.c" 1
	
->_SIGVTALRM 28 SIGVTALRM	 #
 # 0 "" 2
 # 224 "arch/rlx/kernel/asm-offsets.c" 1
	
->_SIGPROF 29 SIGPROF	 #
 # 0 "" 2
 # 225 "arch/rlx/kernel/asm-offsets.c" 1
	
->_SIGXCPU 30 SIGXCPU	 #
 # 0 "" 2
 # 226 "arch/rlx/kernel/asm-offsets.c" 1
	
->_SIGXFSZ 31 SIGXFSZ	 #
 # 0 "" 2
 # 227 "arch/rlx/kernel/asm-offsets.c" 1
	
->
 # 0 "" 2
#NO_APP
	j	$31
	.end	output_signal_defined
	.size	output_signal_defined, .-output_signal_defined
	.section	.text.output_irq_cpustat_t_defines,"ax",@progbits
	.align	2
	.globl	output_irq_cpustat_t_defines
	.set	nomips16
	.ent	output_irq_cpustat_t_defines
	.type	output_irq_cpustat_t_defines, @function
output_irq_cpustat_t_defines:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
#APP
 # 232 "arch/rlx/kernel/asm-offsets.c" 1
	
->#Linux irq_cpustat_t offsets.
 # 0 "" 2
 # 233 "arch/rlx/kernel/asm-offsets.c" 1
	
->IC_SOFTIRQ_PENDING 0 offsetof(irq_cpustat_t, __softirq_pending)	 #
 # 0 "" 2
 # 234 "arch/rlx/kernel/asm-offsets.c" 1
	
->IC_IRQ_CPUSTAT_T 32 sizeof(irq_cpustat_t)	 #
 # 0 "" 2
 # 235 "arch/rlx/kernel/asm-offsets.c" 1
	
->
 # 0 "" 2
#NO_APP
	j	$31
	.end	output_irq_cpustat_t_defines
	.size	output_irq_cpustat_t_defines, .-output_irq_cpustat_t_defines
	.ident	"GCC: (Realtek RSDK-1.5.6p2) 4.4.6"
