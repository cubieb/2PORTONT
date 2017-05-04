/*
** Copyright (c) 2007-2011 by Silicon Laboratories
**
** $Id: proslic_mlt_math.c 4550 2014-10-27 20:57:00Z nizajerk $
**
*/
/*! \file proslic_mlt_math.c
**  \brief ProSLIC MLT math functions implementation file
**  
**	This is the implementation file for the ProSLIC MLT math functions.
**
**  \author Silicon Laboratories, Inc (laj, cdp)
**
**  \attention 
**	This file contains proprietary information.	 
**	No dissemination allowed without prior written permission from
**	Silicon Laboratories, Inc. 
*/

#include "si_voice_datatypes.h"


#define BIN_PLACES       16         /*!< Resolution of log estimate */
#define MLT_CONST_LN     45426      /*!< Constant used in computing natural log */

/*
** fp_abs -  Return absolute value of 32-bit integer
*/
int32 fp_abs(int32 a)
{
    if (a < 0)
	return -1 * a;
    return a;
}


/*
** Isqrt - estimate square root of 32bit integer
*/
uInt32 Isqrt(uInt32 number)
{
    uInt32 op, res, one;

    op = number;
    res = 0;

    /* "one" starts at the highest power of four <= than the argument. */
    one = 1L << 30;		/* second-to-top bit set */
    while (one > op)
	one >>= 2;

    while (one != 0)
      {
	  if (op >= res + one)
	    {
		op = op - (res + one);
		res = res + 2 * one;
	    }
	  res >>= 1;
	  one >>= 2;
      }


    return (res);
}
/*
**  fp_log2 - Fixed point log base-2 estimate
*/
uInt32 fp_log2(int32 input)
{
    int i;
    uInt32 tmp;
    int divs = 0;
    uInt32 prod;
    uInt32 mantissa = 0;

    /* binary point is between bits 15 & 16 */
    tmp = input << 16;

    while (tmp >= (2 << 16)) {
        tmp = tmp >> 1;
        divs++;
    }

    for (i = 0; i < BIN_PLACES; i++) {

        prod = ((tmp >> 2) * (tmp >> 2)) >> 12;

        mantissa = mantissa << 1;
        if (prod >= (2 << 16)) {
            prod = prod >> 1;
            mantissa = mantissa | 1;
        } else {
        }
        tmp = prod;
    }

    return ((divs << 16) | (mantissa << (16 - BIN_PLACES)));
}


/*
** fp_ln - fixed point log base-e estimate
*/
uInt32 fp_ln(int32 input)
{
    return (fp_log2(input) >> 2) * (MLT_CONST_LN >> 2) >> 12;
}

/*@}*/


/*
** computeResTC - convert ratio to dB
*/
int32 computeResTC(int32 v1, int32 v2, int32 delta_t, int32 cEst , int32 *tau)
{
    int32 lnDiff;
    int32 rEst;
            
    lnDiff = fp_ln(v1) - fp_ln(v2);
    if(lnDiff > 0){
        *tau = (65536*delta_t) / lnDiff;
    }
    else {
        *tau = 65536*delta_t; /* Arbitrarily large number */
    }

    rEst = 1000000L/cEst;  /* Should never underflow */

    rEst *= (*tau);
    return rEst;
}


/*
** dB lookup table
*/
const uInt32 dBTable10_n60 [] = {
    31623,
    29854,
    28184,
    26607,
    25119,
    23714,
    22387,
    21135,
    19953,
    18836,
    17783,
    16788,
    15849,
    14962,
    14125,
    13335,
    12589,
    11885,
    11220,
    10593,
    10000,
    9441,
    8913,
    8414,
    7943,
    7499,
    7079,
    6683,
    6310,
    5957,
    5623,
    5309,
    5012,
    4732,
    4467,
    4217,
    3981,
    3758,
    3548,
    3350,
    3162,
    2985,
    2818,
    2661,
    2512,
    2371,
    2239,
    2113,
    1995,
    1884,
    1778,
    1679,
    1585,
    1496,
    1413,
    1334,
    1259,
    1189,
    1122,
    1059,
    1000,
    944,
    891,
    841,
    794,
    750,
    708,
    668,
    631,
    596,
    562,
    531,
    501,
    473,
    447,
    422,
    398,
    376,
    355,
    335,
    316,
    299,
    282,
    266,
    251,
    237,
    224,
    211,
    200,
    188,
    178,
    168,
    158,
    150,
    141,
    133,
    126,
    119,
    112,
    106,
    100,
    94,
    89,
    84,
    79,
    75,
    71,
    67,
    63,
    60,
    56,
    53,
    50,
    47,
    45,
    42,
    40,
    38,
    35,
    33,
    32,
    30,
    28,
    27,
    25,
    24,
    22,
    21,
    20,
    19,
    18,
    17,
    16,
    15,
    14,
    13,
    13,
    12,
    11,
    11,
    10
};


/*
** dbLookup - looks up dB value for passed ratio
*/
int32 dBLookup(uInt32 number)
{
    int i;
    uInt32 err;

    if(number >= dBTable10_n60[0])
    {
        return 10000;  /* out of range - clamp at 10dB */
    }

    for(i=0;i<139;i++)
    {
        if((number < dBTable10_n60[i])&&(number >= dBTable10_n60[i+1]))
        {
        /* See which level it is closest to */
            err = dBTable10_n60[i] - number;
            if(err < (number - dBTable10_n60[i+1]))
            {
               return (10000 - i*500);
            }
            else
            {
               return (10000 - (i+1)*500);
            }
        }
    }
    /* No solution found?  Odd..but return -40.5dB */
    return -60000;
}


