#ifndef OPER_32B_PROTO_H
#define OPER_32B_PROTO_H

void   L_Extracts(Word32 L_32, Word16 *hi, Word16 *lo);
Word32 L_Comps(Word16 hi, Word16 lo);
Word32 Mpy_32s(Word16 hi1, Word16 lo1, Word16 hi2, Word16 lo2);
Word32 Mpy_32_16s(Word16 hi, Word16 lo, Word16 n);
Word32 Div_32s(Word32 L_num, Word16 denom_hi, Word16 denom_lo);

void   L_Extract_c(Word32 L_32, Word16 *hi, Word16 *lo);
Word32 L_Comp_c(Word16 hi, Word16 lo);
Word32 Mpy_32_c(Word16 hi1, Word16 lo1, Word16 hi2, Word16 lo2);
Word32 Mpy_32_16_c(Word16 hi, Word16 lo, Word16 n);
Word32 Div_32_c(Word32 L_num, Word16 denom_hi, Word16 denom_lo);

#endif	// OPER_32B_PROTO_H

