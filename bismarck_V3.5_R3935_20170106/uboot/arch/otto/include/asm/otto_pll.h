#ifndef _OTTO_PLL_H_
#define _OTTO_PLL_H_
#include <soc.h>

#define board_CPU_freq_mhz()  cg_query_freq(CG_DEV_OCP)
#define board_LX_freq_mhz()   cg_query_freq(CG_DEV_LX)
#define board_DRAM_freq_mhz() cg_query_freq(CG_DEV_MEM)

#endif
