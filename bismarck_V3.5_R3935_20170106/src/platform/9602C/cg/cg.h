#ifndef __CG_HEADER__
#define __CG_HEADER__

//#include <register_map.h>
//#include "/home/gangadhar/Bismarck/bismarck_V3.5_R3935_20170106/src/template/9602C_spi_nand/register_map.h"
#include "../../../template/9602C_spi_nand/register_map.h"
//#include <dram/memcntlr_reg.h>
//#include "/home/gangadhar/Bismarck/bismarck_V3.5_R3935_20170106/src/platform/9602C/dram/memcntlr_reg.h"
#include "../dram/memcntlr_reg.h"
#include "cg_dev_freq.h"
#include "ocp_pll.h"
#include "mem_pll.h"
#include "gphy_pll.h"
//#include "/home/gangadhar/Bismarck/bismarck_V3.5_R3935_20170106/src/platform/9602C/cmu/cmu.h"
#include "../cmu/cmu.h"


/****** Default PLL Frequency Information******/
#define OCP_PLL_MIN        (450)
#define OCP_PLL_MAX        (1000)
#define CPU_CLK_DEFAULT    (500)
#define CPU_CLK_MAX        (725)
#define CPU_CLK_MIN        (OCP_PLL_MIN/4)

#define LX_PLL_DEFAULT     (200)
#define LX_PLL_DEFAULT_DIV (0)
#define NOR_PLL_DEFAULT     (200)
#define NOR_PLL_DEFAULT_DIV (1)

#define MEM_PLL_MIN        (250)
#define MEM_PLL_MAX        (600)
#define MEM_PLL_STEP       (25)
#define MEM_PLL_ELEMENTS   (((MEM_PLL_MAX-MEM_PLL_MIN)/MEM_PLL_STEP)+1)

typedef struct {
    SYS_OCP_PLL_CTRL0_T sys_ocp_pll_ctrl0;

    SYS_MEM_PLL_CTRL0_T sys_mem_pll_ctrl0;
    SYS_MEM_PLL_CTRL1_T sys_mem_pll_ctrl1;
    SYS_MEM_PLL_CTRL2_T sys_mem_pll_ctrl2;
    SYS_MEM_PLL_CTRL3_T sys_mem_pll_ctrl3;
    SYS_MEM_PLL_CTRL5_T sys_mem_pll_ctrl5;
    SYS_MEM_PLL_CTRL6_T sys_mem_pll_ctrl6;

    PHY_RG3X_PLL_T      phy_rg3x_pll;

    u32_t               sclk_div2ctrl;

    OC0_CMUGCR_T    oc0_cmugcr;
    OC0_CMUCR_T     oc0_cmucr;       /* for CMU static division */
	LX0_CMUCR_T     lx0_cmucr;		 /* for CMU static division */
	LX1_CMUCR_T     lx1_cmucr;		 /* for CMU static division */
	LX2_CMUCR_T     lx2_cmucr;		 /* for CMU static division */
	LXP_CMUCR_T     lxp_cmucr;		 /* for CMU static division */
} cg_register_set_t;


#define END_OF_INFO (0xFFFF)

typedef struct {
    u16_t pll_freq_mhz;
    u16_t pll_cfg_div;
    u16_t cal_sclk_mhz;
    u16_t sclk_div2ctrl;
} cg_sclk_info_t;

typedef struct pll_freq_sel_info_s{
    u16_t freq_mhz;
    u16_t cfg_div;
}pll_freq_sel_info_t;

typedef struct clk_div_sel_info_s{
    u16_t divisor;
    u16_t div_to_ctrl;
}clk_div_sel_info_t;


u32_t cg_query_freq(u32_t dev_type);
void cg_xlat_n_assign(cg_register_set_t *rs);
__attribute__ ((far)) void cg_init(void);
void cg_result_decode(void);

extern cg_dev_freq_t cg_dev_freq_info;

#ifndef GET_MEM_MHZ
#define GET_MEM_MHZ() (cg_query_freq(CG_DEV_MEM))
#endif

#ifndef GET_CPU_MHZ
#define GET_CPU_MHZ() (cg_query_freq(CG_DEV_OCP))
#define GET_CPU_HZ()  (GET_CPU_MHZ()*1000000)
#endif


extern void set_spi_ctrl_divisor(u16_t clk_div, u16_t spif_mhz);
extern u32_t get_spi_ctrl_divisor(void);
extern u32_t get_default_spi_ctrl_divisor(void);
#endif
