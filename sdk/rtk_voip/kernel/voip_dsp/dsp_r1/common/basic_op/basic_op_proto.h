#ifndef BASIC_OP_PROTO_H
#define BASIC_OP_PROTO_H

/*________________________________________________________
___________________
 |
                  |
 |   Constants and Globals
                  |

|_________________________________________________________
__________________|
*/
extern Flag Overflow;
extern Flag Carry;

#define MAX_32 (Word32)0x7fffffffL
#define MIN_32 (Word32)0x80000000L

#define MAX_16 (Word16)0x7fff
#define MIN_16 (Word16)0x8000

void set_mmd(char a);

/* AMS function prototype */

Word16 satures(Word32 L_var1);
Word16 adds(Word16 var1, Word16 var2);     /* Short add,           1 */
Word16 subs(Word16 var1, Word16 var2);     /* Short sub,           1 */
Word16 abs_ss(Word16 var1);                /* Short abs,           1 */
Word16 shrs(Word16 var1, Word16 var2);     /* Short shift right,   1 */
Word16 mults(Word16 var1, Word16 var2);    /* Short mult,          1 */
Word16 mult_rs(Word16 var1, Word16 var2);	  /* Short mult_r,        1 */
Word16 negates(Word16 var1);				  /* Short negate,		  1 */
Word16 rounds(Word32 L_var1);              /* Round,               1 */

Word32 L_adds(Word32 L_var1, Word32 L_var2);   /* Long add,        2 */
Word32 L_subs(Word32 L_var1, Word32 L_var2);   /* Long sub,        2 */	
Word32 L_abss(Word32 L_var1);            /* Long abs,              3 */
Word32 L_shrs(Word32 L_var1, Word16 var2); /* Long shift right,    2 */
Word32 L_mults(Word16 var1, Word16 var2);  /* Long mult,           1 */
Word32 L_macs(Word32 L_var3, Word16 var1, Word16 var2); /* Mac,    1 */
Word32 L_msus(Word32 L_var3, Word16 var1, Word16 var2); /* Msu,    1 */
Word32 L_negates(Word32 L_var1);               /* Long negate,     2 */

//Word16 satures(Word32 L_var1);
Word16 shls(Word16 var1, Word16 var2);     /* Short shift left,    1 */
Word32 L_shls(Word32 L_var1, Word16 var2); /* Long shift left,     2 */
Word32 L_shr_rs(Word32 L_var1, Word16 var2);/* Long shift right with round,  3*/
Word16 norm_ss(Word16 var1);             /* Short norm,           15 */
Word16 norm_ls(Word32 L_var1);           /* Long norm,            30 */
Word16 div_ss(Word16 var1, Word16 var2); /* Short division,       18 */
Word16 mac_rs(Word32 L_var3, Word16 var1, Word16 var2);
Word16 msu_rs(Word32 L_var3, Word16 var1, Word16 var2);
Word32 L_mult0s (Word16 var1,Word16 var2);
Word32 L_mac0s (Word32 L_var3, Word16 var1, Word16 var2);
Word32 L_msu0s (Word32 L_var3, Word16 var1, Word16 var2);
/* Additional operators */
Word32 L_mlss( Word32 Lv, Word16 v );	    /* Wght ?? */
Word16 div_ls( Word32, Word16 ) ;

// Only ASM
Word32 L_macOvs(Word32 L_var3, Word16 var1, Word16 var2);
Word32 L_msuOvs(Word32 L_var3, Word16 var1, Word16 var2);

/* C function prototype */
Word16 saturec(Word32 L_var1);
Word16 addc(Word16 var1, Word16 var2);     /* Short add,           1 */
Word16 subc(Word16 var1, Word16 var2);     /* Short sub,           1 */
Word16 abs_sc(Word16 var1);                /* Short abs,           1 */
Word16 shrc(Word16 var1, Word16 var2);     /* Short shift right,   1 */
Word16 multc(Word16 var1, Word16 var2);    /* Short mult,          1 */
Word16 mult_rc(Word16 var1, Word16 var2);	  /* Short mult_r,        1 */
Word16 negatec(Word16 var1);				  /* Short negate,		  1 */
Word16 extract_hc(Word32 L_var1);
Word16 extract_lc(Word32 L_var1);
/* Additional operators */
Word16 roundc(Word32 L_var1);              /* Round,               1 */

Word32 L_macNsc(Word32 L_var3, Word16 var1, Word16 var2);
Word32 L_msuNsc(Word32 L_var3, Word16 var1, Word16 var2);
Word32 L_add_cc(Word32 L_var1, Word32 L_var2);
Word32 L_sub_cc(Word32 L_var1, Word32 L_var2);
Word32 L_addc(Word32 L_var1, Word32 L_var2);   /* Long add,        2 */
Word32 L_subc(Word32 L_var1, Word32 L_var2);   /* Long sub,        2 */	
Word32 L_absc(Word32 L_var1);            /* Long abs,              3 */
Word32 L_satc (Word32 L_var1);
Word32 L_shrc(Word32 L_var1, Word16 var2); /* Long shift right,    2 */
Word32 L_multc(Word16 var1, Word16 var2);  /* Long mult,           1 */
Word32 L_macc(Word32 L_var3, Word16 var1, Word16 var2); /* Mac,    1 */
Word32 L_msuc(Word32 L_var3, Word16 var1, Word16 var2); /* Msu,    1 */
Word32 L_negatec(Word32 L_var1);               /* Long negate,     2 */

Word16 shlc(Word16 var1, Word16 var2);     /* Short shift left,    1 */
Word32 L_shlc(Word32 L_var1, Word16 var2); /* Long shift left,     2 */
Word32 L_shr_rc(Word32 L_var1, Word16 var2);/* Long shift right with round,  3*/
Word16 norm_sc(Word16 var1);             /* Short norm,           15 */
Word16 norm_lc(Word32 L_var1);           /* Long norm,            30 */
Word16 div_sc(Word16 var1, Word16 var2); /* Short division,       18 */
Word16 mac_rc(Word32 L_var3, Word16 var1, Word16 var2);
Word16 msu_rc(Word32 L_var3, Word16 var1, Word16 var2);
Word32 L_deposit_hc(Word16 var1);
Word32 L_deposit_lc(Word16 var1);
Word32 L_mult0c (Word16 var1,Word16 var2);
Word32 L_mac0c (Word32 L_var3, Word16 var1, Word16 var2);
Word32 L_msu0c (Word32 L_var3, Word16 var1, Word16 var2);
Word32 L_mlsc( Word32 Lv, Word16 v );	    /* Wght ?? */
Word16 div_lc( Word32, Word16 ) ;
Word16 i_multc(Word16 a, Word16 b);

Word16 shl_nosatc(Word16 var1,Word16 var2);		/* vv Short shift left,    1 */
Word16 shr_rc(Word16 var1, Word16 var2);

Word32 L_shr_r_plusc(Word32 L_var1,Word16 var2);
Word16 satureOvc(Word32 L_var1);
Word16 subOvc(Word16 var1,Word16 var2);
Word16 addOvc(Word16 var1,Word16 var2);

#endif // BASIC_OP_PROTO_H

