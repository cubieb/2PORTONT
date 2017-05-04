#ifndef ACLMW_GOLBAL_TYPEDEF_H
#define ACLMW_GOLBAL_TYPEDEF_H


/**********************************************************************
    Global Constants Definitions :  
************************************************************************/
#define ERROR		-1
#define NULL			0

#define GOOD		1
#define BAD			0

#define OK			0
#define NOT_OK		1

#define TRUE			1
#define FALSE		0

#define DISABLE		0
#define ENABLE		1

#define SUCCESS		0
#define FAILED		-1


/**********************************************************************
    Global Macro Definitions :  
************************************************************************/
#define ALIGNED(x)	__attribute__ ((aligned (x)))
#define SECTION(x)	__attribute__ ((section(x)))


/**********************************************************************
    Global TypeDef Definitions :  
************************************************************************/

typedef signed long		Flag;
typedef signed long		Word32;
typedef signed short		Word16;
typedef signed char		Word8;
typedef signed long		word32;
typedef signed short		word16;
typedef signed char		word8;
typedef unsigned long long	uint64;
typedef long long			int64;
typedef unsigned int		uint32;
typedef int				int32;
typedef unsigned short		uint16;
typedef short				int16;
typedef unsigned char		uint8;
typedef char				int8;


#endif
