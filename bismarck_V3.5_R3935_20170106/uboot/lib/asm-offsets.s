	.section .mdebug.abi32
	.previous
	.gnu_attribute 4, 3
	.section	.debug_abbrev,"",@progbits
$Ldebug_abbrev0:
	.section	.debug_info,"",@progbits
$Ldebug_info0:
	.section	.debug_line,"",@progbits
$Ldebug_line0:
	.text
$Ltext0:
	.section	.text.main,"ax",@progbits
	.align	2
	.globl	main
$LFB88 = .
	.file 1 "lib/asm-offsets.c"
	.loc 1 23 0
	.set	nomips16
	.ent	main
	.type	main, @function
main:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.loc 1 25 0
#APP
 # 25 "lib/asm-offsets.c" 1
	
->GENERATED_GBL_DATA_SIZE 80 (sizeof(struct global_data) + 15) & ~15
 # 0 "" 2
	.loc 1 28 0
 # 28 "lib/asm-offsets.c" 1
	
->GENERATED_BD_INFO_SIZE 48 (sizeof(struct bd_info) + 15) & ~15
 # 0 "" 2
	.loc 1 32 0
#NO_APP
	.set	noreorder
	.set	nomacro
	j	$31
	move	$2,$0
	.set	macro
	.set	reorder

	.end	main
$LFE88:
	.size	main, .-main
	.section	.debug_frame,"",@progbits
$Lframe0:
	.4byte	$LECIE0-$LSCIE0
$LSCIE0:
	.4byte	0xffffffff
	.byte	0x1
	.ascii	"\000"
	.uleb128 0x1
	.sleb128 -4
	.byte	0x1f
	.byte	0xc
	.uleb128 0x1d
	.uleb128 0x0
	.align	2
$LECIE0:
$LSFDE0:
	.4byte	$LEFDE0-$LASFDE0
$LASFDE0:
	.4byte	$Lframe0
	.4byte	$LFB88
	.4byte	$LFE88-$LFB88
	.align	2
$LEFDE0:
	.text
$Letext0:
	.section	.debug_info
	.4byte	0x8d
	.2byte	0x2
	.4byte	$Ldebug_abbrev0
	.byte	0x4
	.uleb128 0x1
	.4byte	$LASF10
	.byte	0x1
	.4byte	$LASF11
	.4byte	$LASF12
	.4byte	0x0
	.4byte	0x0
	.4byte	$Ldebug_ranges0+0x0
	.4byte	$Ldebug_line0
	.uleb128 0x2
	.byte	0x1
	.byte	0x8
	.4byte	$LASF0
	.uleb128 0x2
	.byte	0x4
	.byte	0x7
	.4byte	$LASF1
	.uleb128 0x2
	.byte	0x2
	.byte	0x7
	.4byte	$LASF2
	.uleb128 0x2
	.byte	0x1
	.byte	0x6
	.4byte	$LASF3
	.uleb128 0x2
	.byte	0x2
	.byte	0x5
	.4byte	$LASF4
	.uleb128 0x3
	.byte	0x4
	.byte	0x5
	.ascii	"int\000"
	.uleb128 0x2
	.byte	0x4
	.byte	0x7
	.4byte	$LASF5
	.uleb128 0x2
	.byte	0x8
	.byte	0x5
	.4byte	$LASF6
	.uleb128 0x2
	.byte	0x8
	.byte	0x7
	.4byte	$LASF7
	.uleb128 0x4
	.byte	0x4
	.byte	0x7
	.uleb128 0x2
	.byte	0x4
	.byte	0x5
	.4byte	$LASF8
	.uleb128 0x2
	.byte	0x1
	.byte	0x6
	.4byte	$LASF9
	.uleb128 0x5
	.byte	0x1
	.4byte	$LASF13
	.byte	0x1
	.byte	0x16
	.byte	0x1
	.4byte	0x4c
	.4byte	$LFB88
	.4byte	$LFE88
	.byte	0x1
	.byte	0x6d
	.byte	0x0
	.section	.debug_abbrev
	.uleb128 0x1
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x25
	.uleb128 0xe
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1b
	.uleb128 0xe
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x52
	.uleb128 0x1
	.uleb128 0x55
	.uleb128 0x6
	.uleb128 0x10
	.uleb128 0x6
	.byte	0x0
	.byte	0x0
	.uleb128 0x2
	.uleb128 0x24
	.byte	0x0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.byte	0x0
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x24
	.byte	0x0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x8
	.byte	0x0
	.byte	0x0
	.uleb128 0x4
	.uleb128 0x24
	.byte	0x0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.byte	0x0
	.byte	0x0
	.uleb128 0x5
	.uleb128 0x2e
	.byte	0x0
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.byte	0x0
	.section	.debug_pubnames,"",@progbits
	.4byte	0x17
	.2byte	0x2
	.4byte	$Ldebug_info0
	.4byte	0x91
	.4byte	0x79
	.ascii	"main\000"
	.4byte	0x0
	.section	.debug_aranges,"",@progbits
	.4byte	0x1c
	.2byte	0x2
	.4byte	$Ldebug_info0
	.byte	0x4
	.byte	0x0
	.2byte	0x0
	.2byte	0x0
	.4byte	$LFB88
	.4byte	$LFE88-$LFB88
	.4byte	0x0
	.4byte	0x0
	.section	.debug_ranges,"",@progbits
$Ldebug_ranges0:
	.4byte	$Ltext0
	.4byte	$Letext0
	.4byte	$LFB88
	.4byte	$LFE88
	.4byte	0x0
	.4byte	0x0
	.section	.debug_str,"MS",@progbits,1
$LASF10:
	.ascii	"GNU C 4.4.6\000"
$LASF5:
	.ascii	"unsigned int\000"
$LASF1:
	.ascii	"long unsigned int\000"
$LASF11:
	.ascii	"lib/asm-offsets.c\000"
$LASF12:
	.ascii	"/home/gangadhar/2PORTONT/bismarck_V3.5_R3935_20170106/ub"
	.ascii	"oot\000"
$LASF3:
	.ascii	"signed char\000"
$LASF2:
	.ascii	"short unsigned int\000"
$LASF4:
	.ascii	"short int\000"
$LASF7:
	.ascii	"long long unsigned int\000"
$LASF0:
	.ascii	"unsigned char\000"
$LASF8:
	.ascii	"long int\000"
$LASF13:
	.ascii	"main\000"
$LASF9:
	.ascii	"char\000"
$LASF6:
	.ascii	"long long int\000"
	.ident	"GCC: (Realtek RSDK-1.5.6p2) 4.4.6"
