#ifndef BASIC_OP_H
#define BASIC_OP_H

#ifndef __ASSEMBLY__
#include "basic_op_proto.h"
#endif

#define extract_h_macro(a)	(Word16)(a>>16)
#define extract_l_macro(a)	(Word16)(a)
#define L_deposit_h_macro(a)	(Word32)(((Word32)a)<<16)
#define L_deposit_l_macro(a)	(Word32)(a)
#define i_mult_macro(a,b)	(Word16)(a*b)
//#define sature_macro(L_var1) (Word16)( (L_var1 > 0X00007fffL) ? MAX_16 : ( (L_var1 < (Word32)0xffff8000L) ? (Word16)MIN_16 : extract_l(L_var1) ) )


#ifdef FEATURE_INLINE_ASM

#include "basop_inline.h"

#define add add_inline
#define sub sub_inline
#define abs_s abs_s_inline
#define shr shr_inline
#define mult mult_inline
#define mult_r mult_r_inline
#define negate negate_inline
#define round round_inline

#define L_add L_add_inline
#define L_sub L_sub_inline
#define L_abs L_abs_inline
#define L_shr L_shr_inline
#define L_mult L_mult_inline
#define L_mac L_mac_inline
#define L_msu L_msu_inline
#define L_negate L_negate_inline
 
#else

#ifndef VC6

#define add adds
#define sub subs
#define abs_s abs_ss
#define shr shrs
#define mult mults
#define mult_r mult_rs
#define negate negates
#define round rounds

#define L_add L_adds
#define L_sub L_subs
#define L_abs L_abss
#define L_shr L_shrs
#define L_mult L_mults
#define L_mac L_macs
#define L_msu L_msus
#define L_negate L_negates

#else

#define add addc
#define sub subc
#define abs_s abs_sc
#define shr shrc
#define mult multc
#define mult_r mult_rc
#define negate negatec
#define round roundc

#define L_add L_addc
#define L_sub L_subc
#define L_abs L_absc
#define L_shr L_shrc
#define L_mult L_multc
#define L_mac L_macc
#define L_msu L_msuc
#define L_negate L_negatec

#endif /* VC6 */
#endif /* FEATURE_INLINE_ASM */ 

#ifndef VC6

#define sature satures
#define shl shls
#define L_shl L_shls
#define L_shr_r L_shr_rs
#define norm_s norm_ss
#define norm_l norm_ls
#define div_s div_ss
#define mac_r mac_rs
#define msu_r msu_rs
#define L_mult0 L_mult0s
#define L_mac0 L_mac0s
#define L_msu0 L_msu0s
#define L_mls L_mlss
#define div_l div_ls

#define extract_h extract_h_macro
#define extract_l extract_l_macro
#define L_deposit_h L_deposit_h_macro
#define L_deposit_l L_deposit_l_macro
#define i_mult	i_mult_macro

#else

#define sature saturec
#define shl shlc
#define L_shl L_shlc
#define L_shr_r L_shr_rc
#define norm_s norm_sc
#define norm_l norm_lc
#define div_s div_sc
#define mac_r mac_rc
#define msu_r msu_rc
#define L_mult0 L_mult0c
#define L_mac0 L_mac0c
#define L_msu0 L_msu0c
#define L_mls L_mlsc
#define div_l div_lc

#define extract_h extract_hc
#define extract_l extract_lc
#define L_deposit_h L_deposit_hc
#define L_deposit_l L_deposit_lc
#define i_mult	i_multc

#endif /* VC6 */

// Only C version
#define shr_r shr_rc
#define L_macNs L_macNsc
#define L_msuNs L_msuNsc
#define L_add_c L_add_cc
#define L_sub_c L_sub_cc
#define L_sat L_satc

/////////// Realtek VoIP Add ////////////
// Only C version
#define shl_nosat shl_nosatc
#define L_shr_r_plus L_shr_r_plusc
#define satureOv satureOvc
#define subOv subOvc
#define addOv addOvc

// Only ASM version (G.729 used)
#define L_macOv L_macOvs
#define L_msuOv L_msuOvs

#endif /* BASIC_OP_H */

