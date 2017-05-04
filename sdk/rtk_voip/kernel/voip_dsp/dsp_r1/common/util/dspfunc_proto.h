#ifndef _DSP_FUNC_PROTO_H_
#define _DSP_FUNC_PROTO_H_

#include "typedef.h"


/*-------------------------------*
 * Mathematic functions.         *
 *-------------------------------*/
 
/* AMS function prototype */
Word32 Inv_sqrts(   /* (o) Q30 : output value   (range: 0<=val<1)           */
  Word32 L_x       /* (i) Q0  : input value    (range: 0<=val<=7fffffff)   */
);

void Log2s(
  Word32 L_x,       /* (i) Q0 : input value                                 */
  Word16 *exponent, /* (o) Q0 : Integer part of Log2.   (range: 0<=val<=30) */
  Word16 *fraction  /* (o) Q15: Fractionnal part of Log2. (range: 0<=val<1) */
);

Word32 Pow2s(        /* (o) Q0  : result       (range: 0<=val<=0x7fffffff) */
  Word16 exponent,  /* (i) Q0  : Integer part.      (range: 0<=val<=30)   */
  Word16 fraction   /* (i) Q15 : Fractionnal part.  (range: 0.0<=val<1.0) */
);


/* C function prototype */
Word32 Inv_sqrt_c(   /* (o) Q30 : output value   (range: 0<=val<1)           */
  Word32 L_x       /* (i) Q0  : input value    (range: 0<=val<=7fffffff)   */
);

void Log2_c(
  Word32 L_x,       /* (i) Q0 : input value                                 */
  Word16 *exponent, /* (o) Q0 : Integer part of Log2.   (range: 0<=val<=30) */
  Word16 *fraction  /* (o) Q15: Fractionnal part of Log2. (range: 0<=val<1) */
);

Word32 Pow2_c(        /* (o) Q0  : result       (range: 0<=val<=0x7fffffff) */
  Word16 exponent,  /* (i) Q0  : Integer part.      (range: 0<=val<=30)   */
  Word16 fraction   /* (i) Q15 : Fractionnal part.  (range: 0.0<=val<1.0) */
);

long FFracSqrt_c(long x, int nfbits);

#endif /* _DSP_FUNC_PROTO_H_ */

