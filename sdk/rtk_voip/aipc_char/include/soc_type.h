/*
*	Description:
*		SD6 SoC C language type definitions
*/

#ifndef __SOC_TYPE_H__
#define __SOC_TYPE_H__

typedef unsigned long long	u64_t;
typedef long long			s64_t;
typedef unsigned int		u32_t;
typedef unsigned long		ul32_t;
typedef int					s32_t;
typedef unsigned short		u16_t;
typedef short				s16_t;
typedef unsigned char		u8_t;
typedef char				s8_t;


#ifndef OK
#define OK 		0
#endif
#ifndef NOK
#define NOK		-1
#endif

#ifndef SUCCESS
#define SUCCESS		0
#endif
#ifndef FAILED
#define FAILED		-1
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef NULL
#define NULL (void *)0
#endif

#endif

