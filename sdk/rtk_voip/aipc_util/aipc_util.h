#ifndef _AIPC_OP_H_
#define _AIPC_OP_H_

#include "aipc_ioctl.h"

typedef enum _BITOP{
	BOP_NONE = 0x00,
	BOP_AND  ,
	BOP_OR   ,
	BOP_XOR  ,
	BOP_NOT  ,
	BOP_MAX  
} BITOP;

#endif /* _AIPC_OP_H_ */
