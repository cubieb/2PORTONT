#ifndef _RTL8367B_TYPES_H_
#define _RTL8367B_TYPES_H_

#include "rtk_types_common.h"


#ifdef EMBEDDED_SUPPORT

#include <common_types.h>

#else

//#include <stdio.h>



#define CONST_T     const

#endif

#ifndef _RTL_TYPES_H

#if 0
typedef unsigned long long		uint64;
typedef long long				int64;
typedef unsigned int			uint32;
typedef int						int32;
typedef unsigned short			uint16;
typedef short					int16;
typedef unsigned char			uint8;
typedef char					int8;
#endif

typedef rtk_uint32          		ipaddr_t;
typedef rtk_uint32					memaddr;

#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN		6
#endif

typedef struct ether_addr_s {
	rtk_uint8 octet[ETHER_ADDR_LEN];
} ether_addr_t;

#define swapl32(x)\
        ((((x) & 0xff000000U) >> 24) | \
         (((x) & 0x00ff0000U) >>  8) | \
         (((x) & 0x0000ff00U) <<  8) | \
         (((x) & 0x000000ffU) << 24))
#define swaps16(x)        \
        ((((x) & 0xff00) >> 8) | \
         (((x) & 0x00ff) << 8))


#ifdef _LITTLE_ENDIAN
	#define ntohs(x)   (swaps16(x))
	#define ntohl(x)   (swapl32(x))
	#define htons(x)   (swaps16(x))
	#define htonl(x)   (swapl32(x))
#else
	#define ntohs(x)	(x)
	#define ntohl(x)	(x)
	#define htons(x)	(x)
	#define htonl(x)	(x)
#endif


#ifdef _LITTLE_ENDIAN
	#define MEM16(x)		(x)
#else
	#define MEM16(x)		(swaps16(x))
#endif

#ifdef __KERNEL__
#define rtlglue_printf printk
#else
#define rtlglue_printf printf
#endif
#define PRINT			rtlglue_printf
#endif /*_RTL_TYPES_H*/



#ifndef NULL
#define NULL 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define CONST			const
#endif /* _RTL8367B_TYPES_H_ */
