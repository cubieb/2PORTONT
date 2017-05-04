#include <soc.h>
#include <memcntlr.h>
#include <cg/cg.h>
#include <plr_sections.h>
#include <util.h>

#define SECTION_PROBEINFO __attribute__ ((section (".parameters")))

#undef _soc

#ifndef SECTION_SOC
#define SECTION_SOC      __attribute__ ((section (".sdata.soc_stru")))
#endif

extern void rlx5281_write_back_invalidate_dcache_cctl(void);
extern void rlx5281_write_back_inv_dcache_range(unsigned int base_addr, unsigned int end_addr);
extern void invalidate_icache_all(void);
extern void rlx5281_invalidate_icache_range(unsigned int base_addr, unsigned int end_addr);

#ifndef PLR_VERSION
#define PLR_VERSION 0x03000500
#endif

extern init_table_entry_t start_of_init_func_table, end_of_init_func_table;
extern symbol_table_entry_t start_of_symble_table, end_of_symble_table;

soc_t _soc SECTION_SOC = {
	.bios={
		.header= {
			.signature=SIGNATURE_PLR,
			.version=PLR_VERSION,
			.export_symb_list=&start_of_symble_table,
			.end_of_export_symb_list=&end_of_symble_table,
			.init_func_list=&start_of_init_func_table,
			.end_of_init_func_list=&end_of_init_func_table,
		},
		.isr=VZERO,
		.size_of_plr_load_firstly=0,
		.uart_putc=VZERO,
		.uart_getc=VZERO,
		.uart_tstc=VZERO,
		.dcache_writeback_invalidate_all= &rlx5281_write_back_invalidate_dcache_cctl,
		.dcache_writeback_invalidate_range=&rlx5281_write_back_inv_dcache_range,
		.icache_invalidate_all=&invalidate_icache_all,
		.icache_invalidate_range=&rlx5281_invalidate_icache_range,
	},
};

u32_t uart_baud_rate  = 115200;

const mc_dram_param_t ddr2_winbond_W975116KG SECTION_PROBEINFO = {
#include <dram_models/winbond_W975116KG-18.h>
};

const mc_dram_param_t ddr2_etron_EM68B16CGDH SECTION_PROBEINFO = {
#include <dram_models/etron_EM68B16CGDH-18J.h>
};

const mc_cntlr_opt_t ddr2_cntlr_opt SECTION_PROBEINFO = {
	.dqs0_en_hclk = 0,
	.dqs1_en_hclk = 0,

	.fast_exit = 1,

	.normal_drv = 1,
	.rtt_nom_ohm = 150,
	.write_lvl_en = 0,
	.dll_dis = 0,

	.cntlr_odt = 75,
	.cntlr_ocd = 50,

	.tx_delay       = 0,
	.clkm_delay     = 0,
	.clkm90_delay   = 0,
	.dqs0_en_tap    = 0x1f,
	.dqs1_en_tap    = 0x1f,
	.dqs0_group_tap = 0xc,
	.dqs1_group_tap = 0xc,
	.dqm0_tap       = 0,
	.dqm1_tap       = 0,
	.wr_dq_delay    = {0},
	.rdr_dq_delay   = {0},
	.rdf_dq_delay   = {0},
	.cal_method     = 1,

	.bin_phs_sel = 9, /* this is IC default value */
	.rx_win_sel_mul = 1,
	.rx_win_sel_div = 2,

	.clk_dll_type = 0,
	.dq_dll_type = 0,
};

const mc_register_set_t regset SECTION_PROBEINFO = {
	.mcr.v    = MCRdv,
	.dcr.v    = DCRdv,
	.dtr0.v   = DTR0dv,
	.dtr1.v   = DTR1dv,
	.dtr2.v   = DTR2dv,
	.dider.v  = DIDERdv,
	.drcko.v  = DRCKOdv,
	.daccr.v  = DACCRdv,
	.ddzqpr.v = DDZQPRdv,
	.dcdqmr.v = DCDQMRdv,
	.ddzqpcr.v  = DDZQPCRdv,
	.dcsdcr0.v  = DCSDCR0dv,
	.dcsdcr1.v  = DCSDCR1dv,
	.dcsdcr2.v  = DCSDCR2dv,
	.dcsdcr3.v  = DCSDCR3dv,
	.ana_dll0.v = ANA_DLL0dv,

	.mr[0].f.mr_mode_en = 1,
	.mr[1].f.mr_mode_en = 1,
	.mr[2].f.mr_mode_en = 1,
	.mr[3].f.mr_mode_en = 1,
	.mr[0].f.mr_mode    = 0,
	.mr[1].f.mr_mode    = 1,
	.mr[2].f.mr_mode    = 2,
	.mr[3].f.mr_mode    = 3,

	.dacdq_rr  = {
		{.v = DACDQ_IDX_RRdv}, {.v = DACDQ_IDX_RRdv},
		{.v = DACDQ_IDX_RRdv}, {.v = DACDQ_IDX_RRdv},
		{.v = DACDQ_IDX_RRdv}, {.v = DACDQ_IDX_RRdv},
		{.v = DACDQ_IDX_RRdv}, {.v = DACDQ_IDX_RRdv},
		{.v = DACDQ_IDX_RRdv}, {.v = DACDQ_IDX_RRdv},
		{.v = DACDQ_IDX_RRdv}, {.v = DACDQ_IDX_RRdv},
		{.v = DACDQ_IDX_RRdv}, {.v = DACDQ_IDX_RRdv},
		{.v = DACDQ_IDX_RRdv}, {.v = DACDQ_IDX_RRdv}
	},
	.dacdq_fr  = {
		{.v = DACDQ_IDX_FRdv}, {.v = DACDQ_IDX_FRdv},
		{.v = DACDQ_IDX_FRdv}, {.v = DACDQ_IDX_FRdv},
		{.v = DACDQ_IDX_FRdv}, {.v = DACDQ_IDX_FRdv},
		{.v = DACDQ_IDX_FRdv}, {.v = DACDQ_IDX_FRdv},
		{.v = DACDQ_IDX_FRdv}, {.v = DACDQ_IDX_FRdv},
		{.v = DACDQ_IDX_FRdv}, {.v = DACDQ_IDX_FRdv},
		{.v = DACDQ_IDX_FRdv}, {.v = DACDQ_IDX_FRdv},
		{.v = DACDQ_IDX_FRdv}, {.v = DACDQ_IDX_FRdv}
	},
};

const mc_info_t mi[] SECTION_PROBEINFO = {
	{(mc_dram_param_t *)&ddr2_winbond_W975116KG,
	 (mc_cntlr_opt_t *)&ddr2_cntlr_opt,
	 (mc_register_set_t *)&regset},
	{(mc_dram_param_t *)&ddr2_etron_EM68B16CGDH,
	 (mc_cntlr_opt_t *)&ddr2_cntlr_opt,
	 (mc_register_set_t *)&regset},
};

MEMCNTLR_SECTION
const mc_info_t *
proj_mc_info_select(void) {
	u32_t ver, orig;

	if (RFLD_MCR(dram_type) == 1) {
		orig = *((volatile u32_t *)0xbb00007c);
		*((volatile u32_t *)0xbb00007c) = *((volatile u32_t *)0xbb00007c) | (0xb << 28);
		ver = ((*((volatile u32_t *)0xbb00007c)) >> 8) & 0x3;
		*((volatile u32_t *)0xbb00007c) = orig;
		if ((ver < 1) || (ver > 2)) {
			ver = 0;
		} else {
			ver--;
		}
		return &mi[ver];
	} else {
		return VZERO;
	}
}

symb_fdefine(SF_SYS_UDELAY,    otto_lx_timer_udelay);
symb_fdefine(SF_SYS_GET_TIMER, otto_lx_timer_get_timer);

const cg_info_t cg_info_proj SECTION_PROBEINFO = {
	.dev_freq ={
		.ocp_mhz  = 400,
		.mem_mhz  = 325,
		.lx_mhz   = 200,
		.spif_mhz = 25,
	},

	.register_set = {
		.syspllctr.v  = SYSPLLCTRdv,
		.lx_clk_pll.v = LX_CLK_PLLdv,
		.cmugcr.v = CMUGCRdv,
	},
};
symb_pdefine(cg_info_dev_freq, SF_SYS_CG_DEV_FREQ, &(cg_info_query.dev_freq));

