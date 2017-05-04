#ifndef __OCP_PLL_H__
#define __OCP_PLL_H__


typedef union {
	struct {
		unsigned int cpu1_pllen_l:1;
		unsigned int cpu1_lr_l:3;
		unsigned int cpu1_testen_l:1;
		unsigned int cpu1_cp_1:3;
		unsigned int en_tiehalf_l:1;
		unsigned int cpu1_lc_l:3;
		unsigned int selcpi_l:1;
		unsigned int sel_test_l:3;
		unsigned int sel_test_div_l:2;
		unsigned int reg_pll_l:6;
		unsigned int cpu1_clk_sel_l:2;
		unsigned int cpu1_n_l:6;       //Test: CPU=450~625, N=16~23, Formal: CPU=450~725, N=16~27,
	} f;
	unsigned int v;
} SYS_OCP_PLL_CTRL0_T;
#define SYS_OCP_PLL_CTRL0rv (*((regval)0xb8000208))
#define SYS_OCP_PLL_CTRL0dv (0xBA180010)
#define RMOD_SYS_OCP_PLL_CTRL0(...) rset(SYS_OCP_PLL_CTRL0, SYS_OCP_PLL_CTRL0rv, __VA_ARGS__)
#define RIZS_SYS_OCP_PLL_CTRL0(...) rset(SYS_OCP_PLL_CTRL0, 0, __VA_ARGS__)
#define RFLD_SYS_OCP_PLL_CTRL0(fld) (*((const volatile SYS_OCP_PLL_CTRL0_T *)0xb8000208)).f.fld

typedef union {
	struct {
		unsigned int mbz:29;
		unsigned int cpu1_gd_l:2; 
		unsigned int oen_l:1;
	} f;
	unsigned int v;
} SYS_OCP_PLL_CTRL1_T;
#define SYS_OCP_PLL_CTRL1rv (*((regval)0xb800020C))
#define SYS_OCP_PLL_CTRL1dv (0x400001EF)
#define RMOD_SYS_OCP_PLL_CTRL1(...) rset(SYS_OCP_PLL_CTRL1, SYS_OCP_PLL_CTRL1rv, __VA_ARGS__)
#define RIZS_SYS_OCP_PLL_CTRL1(...) rset(SYS_OCP_PLL_CTRL1, 0, __VA_ARGS__)
#define RFLD_SYS_OCP_PLL_CTRL1(fld) (*((const volatile SYS_OCP_PLL_CTRL1_T *)0xb800020C)).f.fld

#if 0
#define CG_MEM_PLL_OE_DIS()    ({  SYS_MEM_PLL_CTRL4_T __reg= {.v=SYS_MEM_PLL_CTRL4rv};    \
                                __reg.f.clk_oe=0;   \
                                SYS_MEM_PLL_CTRL4rv=__reg.v; })
#define CG_MEM_PLL_OE_EN()     ({  SYS_MEM_PLL_CTRL4_T __reg= {.v=SYS_MEM_PLL_CTRL4rv};    \
                                __reg.f.clk_oe=0x3F;   \
                                SYS_MEM_PLL_CTRL4rv=__reg.v; })
#endif


#endif  //__OCP_PLL_H__

