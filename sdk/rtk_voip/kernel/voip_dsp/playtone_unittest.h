#ifndef __PLAYTONE_UNITTEST_H__
#define __PLAYTONE_UNITTEST_H__

/*
 *
*
*/



// type define
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
typedef  long  int   Word32;
typedef  short int   Word16;	
typedef  short int   Flag;
#ifndef UINT32_TYPE		// avoid compile redefie with IAD sdk lib\libmd5\libmd5.h
#define UINT32_TYPE
typedef uint32_t	uint32;
#endif


// macro define
#define MAX_DSP_RTK_SS_NUM 16
#define CH_TONE 2
#define TONE_BUFF_SIZE 480
#define SUPPORT_TONE_PROFILE 1
#define SW_DTMF_CID 1

//#define NULL 0

#define timetick_after( a, b )		\
		( ( long )( ( a ) - ( b ) ) > 0 ? 1 : 0 )


#endif /* __PLAYTONE_UNITTEST_H__ */

