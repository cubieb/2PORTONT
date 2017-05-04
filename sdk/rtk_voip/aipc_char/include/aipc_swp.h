#ifndef __AIPC_SWP_H__
#define __AIPC_SWP_H__

#include "aipc_global.h"
#include "aipc_shm.h"

#ifdef AIPC_BARRIER

#if 1  // uncached memory

volatile static unsigned char _aipc_rd_brr = 0;
volatile static unsigned int  _aipc_tmp_wr = 0;

#define AMB(VAR,VALUE)      do{                                                   \
								_aipc_tmp_wr++;                                   \
								VAR = VALUE;                                      \
								ADUM.wr[0]=_aipc_tmp_wr;                          \
								ADUM.wr[1]=_aipc_tmp_wr;                          \
								ADUM.wr[2]=_aipc_tmp_wr;                          \
								ADUM.wr[3]=_aipc_tmp_wr;                          \
								_aipc_rd_brr = *(volatile unsigned char*)&(VAR);  \
							}while(0)

#if !defined(ADUM)
#error "need to define ADUM"
#endif							
							

#if 0
#ifdef CONFIG_CPU_HAS_SYNC
	#define aipc_sync()                 \
	    __asm__ __volatile__(           \
	        ".set   push\n\t"           \
	        ".set   noreorder\n\t"      \
	        "sync\n\t"                  \
	        ".set   pop"                \
	        : /* no output */           \
	        : /* no input */            \
	        : "memory")
#else
	#define aipc_sync()    __asm__ __volatile__("   \n" : : : "memory")    //cathy, prevent compiler reordering
#endif

#define AWMB(VAR)           do{                                                   \
								aipc_sync();                                      \
								aipc_rd_brr = *(volatile unsigned char*)&(VAR);	  \
							}while(0)
#endif

#else // cached memory	

volatile static unsigned int aipc_wr_brr[4] = {0,0,0,0};
volatile static unsigned int aipc_rd_brr    = 0;

#define AWMB(VAR)			do{												\
								++aipc_wr_brr[0];							\
								++aipc_wr_brr[1];							\
								++aipc_wr_brr[2];							\
								++aipc_wr_brr[3];							\
								aipc_rd_brr = (volatile unsigned int)VAR;	\
							}while(0)
							
#endif

#else // !defined(AIPC_BARRIER)

#define AMB(VAR)			do{												\
							}while(0)

#define AWMB(VAR)			do{												\
							}while(0)

#endif

#endif
