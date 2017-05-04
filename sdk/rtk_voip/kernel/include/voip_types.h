#ifndef __VOIP_TYPES_H__
#define __VOIP_TYPES_H__

#ifdef __KERNEL__
#include <linux/types.h>		/* intXX_t, uintXX_t */
#elif defined( __ECOS )
#include <sys/types.h>
#else
#include <stdint.h>
#endif

/* VoIP also support these types, and we RECOMMEND to use. */
#ifndef _RTL_TYPES_H	// include/net/rtl/rtl_types.h
#ifndef uint64	/*for compile warning in networking driver */
#define uint64 uint64_t
#define int64 int64_t
#ifndef UINT32_TYPE		// avoid compile redefie with IAD sdk lib\libmd5\libmd5.h
#define UINT32_TYPE
#define uint32 uint32_t
#endif
#ifndef int32
#define int32  int32_t
#endif
#ifndef uint16
#define uint16 uint16_t
#endif
#ifndef int16
#define int16 int16_t
#endif
#ifndef uint8
#define uint8 uint8_t
#endif
#ifndef int8
#define int8 int8_t
#endif
#endif
#endif

/* We list exceptions in below. (kerenl only) */
#ifdef __KERNEL__

#ifndef __cplusplus
#ifndef __ECOS
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0)
typedef int				bool;	/* 8672 kernel-2.6 support */
#else
#define bool int
#endif
#endif
#endif

#ifndef true
#define true		(0 == 0)
#define false		(0 != 0)
#endif

#ifndef BOOL
#define BOOL		unsigned char
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL ((void *) 0)
#endif

#ifndef SUCCESS
#define SUCCESS 	0
#endif
#ifndef FAILED
#define FAILED -1
#endif

typedef  long  int   Word32;
typedef  short int   Word16;	
typedef  short int   Flag;	

#endif	/* __KERNEL__ */

/* for 865xB only */
#ifdef __KERNEL__
//#include <linux/config.h>

#ifdef CONFIG_RTL865XB
#include "rtl865xb.h"
#endif

#endif 

#endif /* __VOIP_TYPES_H__ */

