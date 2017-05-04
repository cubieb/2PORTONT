#ifndef _RTL8367B_TYPES_COMMON_H_
#define _RTL8367B_TYPES_COMMON_H_

typedef unsigned long long		rtk_uint64;
typedef long long				rtk_int64;
typedef unsigned int			rtk_uint32;
typedef int						rtk_int32;
typedef unsigned short			rtk_uint16;
typedef short					rtk_int16;
typedef unsigned char			rtk_uint8;
typedef char					rtk_int8;

/* type abstraction */
#ifdef EMBEDDED_SUPPORT

typedef rtk_int16                   rtk_api_ret_t;
typedef rtk_int16                   ret_t;
typedef rtk_uint32                  rtk_u_long;

#else

typedef rtk_int32                   rtk_api_ret_t;
typedef rtk_int32                   ret_t;
typedef rtk_uint64                  rtk_u_long_t;

#endif



#endif
