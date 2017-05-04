// jason++ 2005/04/04
// common routines

#ifndef __UTILITY_H
#define __UTILITY_H

#include <linux/string.h>
//#include <asm/uaccess.h>
//#include "typedef.h"
#include "voip_types.h"

#ifndef __ASSEMBLY__
#include "utility_proto.h"
#endif

// Select C/ASM version utility
#ifndef VC6
#define Copy			Copys
#define Pre_Process2		Pre_Process2s
#define memset32(a, b, c)	memset32s(a, b, c)
#define memset64(a, b, c)	memset64s(a, b, c)
#define memcpy16(a, b, c)	memcpy16s(a, b, c)
#define memcpy32(a, b, c)	memcpy32s(a, b, c)
#define memcpy64(a, b, c)	memcpy64s(a, b, c)
#else
#define Copy	Copyc
#define Pre_Process2		Pre_Process2c
#define memset32(a, b, c)	memset(a, b, 2*(c))
#define memset64(a, b, c)	memset(a, b, 2*(c))
#define memcpy16(a, b, c)	memcpy(a, b, 2*(c))
#define memcpy32(a, b, c)	memcpy(a, b, 2*(c))
#define memcpy64(a, b, c)	memcpy(a, b, 2*(c))
#endif	// VC6

// Only C version utility
void Set_zero( Word16 x[], Word16 L );
void Init_Pre_Process2(void);					// in util/pre_proc.c
void Init_Pre_Process2_by_sid(unsigned int sid);		// in util/pre_proc.c

// Only ASM function
// in util/loop.S
Word32	L_mac_loop1s(Word32 Acc, Word16 *Buff, Word16 Len);
Word32	L_mac_loop2s(Word32 Acc, Word16 *Buff1, Word16 *Buff2, Word16 Len);
Word32	L_msu_loops(Word32 Acc, Word16 *pCoe, Word16 *pDat, Word16 Len);
void	mult_r_loop1s(Word16 *DstTable, Word16 *SrcTable, Word16 Len);
void 	shr_loop1s(Word16 *Dst, Word16 Exp, Word16 Len);
void 	shr_loops(Word16 *Dst, Word16 *Src, Word16 Exp, Word16 Len);
void	i_mult_loops(Word16 *Dst, Word16 *Src1, Word16 Src2, Word16 Len);
void 	add_loop2s(Word16 *Dst, Word16 *Src1, Word16 *Src2, Word16 Len);
void	add_add_loops(Word16 *pExc, Word16 *Dpnt, Word16 *ImpResps, Word16 Len);
void 	add_loops(Word16 *Dst, Word16 *Src, Word16 Len);
void 	sub_loops(Word16 *Dst, Word16 *Src, Word16 Len);
// in util/utilitys.S
Word16	L_shl2_rounds(Word32 Acc);	// round(L_shl(Acc, 2)) 
Word16	L_shl2_L_mac_rounds(Word32 Acc, Word16 var1, Word16 var2);
Word16	L_shl2_extract_hs(Word32 Acc);
Word32	L_deposit_h_L_shrs(Word16 Corr, Word16 Exp);
void	shl_postive_loops(Word16 var1[], Word16 var2[], Word16 var3, Word16 len);
void	shr_postive_loops(Word16 var1[], Word16 var2[], Word16 var3, Word16 len);

//void	mult_r_loop2s(Word16 *DstTable, Word16 *Src1Table, Word16 *Src2Table, Word16 Len);
//Word16	L_shl1_rounds(Word32 Acc);	// round(L_shl(Acc, 1)) 
//Word16	L_deposit_h_L_mac_rounds(Word16 val1, Word16 val2, Word16 val3);
//Word16	L_deposit_h_L_msu_rounds(Word16 val1, Word16 val2, Word16 val3);
//Word32	L_shr_L_adds(Word32 Acc1, Word16 Exp);
//Word32	L_add_imult_loops(Word32 val1, Word16 *Tv, Word16 *Imr, Word16 Len);
//void 	sub_loop2s(Word16 *Dst, Word16 *Src, Word16 Len);

#endif
