#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <stdio.h>

extern FILE *fp_debug;

int cdh_debug_init(void);

void cdh_debug_cleanup(void);

//#define DEBUG_PRINT	printf
#define DEBUG_PRINT(ARGS...)	if ( fp_debug )	{	fprintf(fp_debug, ARGS); fflush(fp_debug);	}

#endif
