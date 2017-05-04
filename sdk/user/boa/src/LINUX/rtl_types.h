/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.                                                    
* 
* Program : The header file of realtek type definition
* Abstract :                                                           
* Author :              
* $Id: rtl_types.h,v 1.6 2012/09/11 08:46:54 tsaitc Exp $
*/


#ifndef _RTL_TYPES_H
#define _RTL_TYPES_H

/*
 * Internal names for basic integral types.  Omit the typedef if
 * not possible for a machine/compiler combination.
 */

#ifndef __VOIP_TYPES_H__
typedef unsigned long long	uint64;
typedef long long		int64;
typedef unsigned int	uint32;
typedef int			int32;
typedef unsigned short	uint16;
typedef short			int16;
typedef unsigned char	uint8;
typedef char			int8;
#endif

typedef uint32		memaddr;	
typedef uint32          ipaddr_t;
typedef struct {
    uint16      mac47_32;
    uint16      mac31_16;
    uint16      mac15_0;
} macaddr_t;
typedef int8*			calladdr_t;

#ifndef _PPPOE_H_
typedef struct ether_addr_s {
	uint8 octet[6];
} ether_addr_t;
#endif


#ifndef NULL
#define NULL 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef SUCCESS
#define SUCCESS 	0
#endif
#ifndef FAILED
#define FAILED -1
#endif

#ifndef OK
#define OK		0
#endif
#ifndef NOT_OK
#define NOT_OK  1
#endif

typedef enum RESULT_E
{
   RESULT_FAIL = 0, /* for post diagnosis return value, orlando 4/27/199 */
   RESULT_PASS      /* for post diagnosis return value, orlando 4/27/199 */
}RESULT_T;

#define ASSERT_CSP(x) if (!(x)) {printf("\nAssert Fail: %s %d", __FILE__, __LINE__); while(1);}
#define ASSERT_ISR(x) if(!(x)) {while(1);}

#define ABORT	1
#define DEBUG_P(fmt,args...) printf(fmt,## args) 
#define PASS 	0
#define INIT(expr, module, abort) do {\
	if(expr){\
		printf("Error >>> %s initialize Failed!!!\n", module);\
		if(abort)\
			return;\
	}else{\
		printf("%s initialized\n", module);\
	}\
}while(0)
 
#define RTL_STATIC_INLINE   static __inline__
#endif
