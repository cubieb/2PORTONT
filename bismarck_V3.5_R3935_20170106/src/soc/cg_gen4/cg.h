#ifndef __CG_HEADER__
#define __CG_HEADER__

#include <register_map.h>
#include <nor_spi/nor_spif_register.h>
#include "cg_dev_freq.h"


#define CPU_CLK_IN_DEFAULT (400)

typedef struct {
	SYSPLLCTR_T  syspllctr;  /* for CPU & DRAM factor */
	PLL2_T       pll2;       /* for 6422 PLL CPU divisor */
	MCKG_DIV_T   mckg_div;   /* for DRAM divisor */
	LX_CLK_PLL_T lx_clk_pll; /* for SPI-F (PLL) & LX */
	SFCR_T       sfcr;       /* for SPI-F (IP) side */
	CMUGCR_T     cmugcr;     /* for CMU static division */
	CMUO0CR_T    cmuo0cr;    /* for OCP0 slow bits */
} cg_register_set_t;

typedef struct {
	cg_dev_freq_t     dev_freq;
	cg_register_set_t register_set;
} cg_info_t;

extern u32_t cg_query_freq(u32_t dev_type);
void cg_xlat_n_assign(void);
void cg_init(void);
void cg_result_decode(void);

extern cg_info_t cg_info_query;


#endif
