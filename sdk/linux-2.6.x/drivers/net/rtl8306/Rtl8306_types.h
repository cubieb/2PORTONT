#ifndef _RTL8306SDM_TYPES_H_
#define _RTL8306SDM_TYPES_H_


/* ===============================================================================
		Type definition
    =============================================================================== */
#ifndef _RTL_TYPES_H
typedef unsigned long long	uint64;
typedef long long		int64;
typedef unsigned int	uint32;
typedef int			int32;
typedef unsigned short	uint16;
typedef short			int16;
typedef unsigned char	uint8;
typedef char			int8;
#endif
#ifdef CONFIG_ETHWAN
//shlee, for one armed router ---------
typedef int32                   rtk_api_ret_t;
typedef int32                   ret_t;
typedef uint64                  rtk_u_long_t;
#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN 6
#endif
#define rtlglue_printf printf
//shlee, -------------------------------
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


#endif
