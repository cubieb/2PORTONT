#include <soc.h>
#include <dram/memcntlr.h>
#include <cg/cg.h>
#include <plr_sections.h>
#include <util.h>

#undef _soc

extern void rlx5281_write_back_invalidate_dcache_cctl(void);
extern void rlx5281_write_back_inv_dcache_range(unsigned int base_addr, unsigned int end_addr);
extern void invalidate_icache_all(void);
extern void rlx5281_invalidate_icache_range(unsigned int base_addr, unsigned int end_addr);
extern init_table_entry_t start_of_init_func_table, end_of_init_func_table;
extern symbol_table_entry_t start_of_symble_table, end_of_symble_table;


soc_t _soc SECTION_SOC_STRU = {
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

//*************************************
// MEM PLL Relative Setting 
//*************************************
//   250,                275,               300,               325,                350,               375,                400,               425,
//   450,                475,               500,               525,                550,               575,                600,
SECTION_CG_MISC_DATA 
u32_t mem_ctrl2[] = {
    0x26222400, 0x26222400, 0x26222520, 0x26222520, 0x26222520, 0x26622520, 0x26622520, 0x26a23720,
    0x26a23720, 0x26a23740, 0x26a23740, 0x26a33840, 0x26a33840, 0x26a33860, 0x26a33860};

SECTION_CG_MISC_DATA 
u32_t ddr2_mem_ctrl0[] = {
    0x0008407f, 0x0008407f, 0x0008407f, 0x0008407f, 0x0008407f, 0x0008407f, 0x0008407f, 0x0008407f,
    0x0008407f, 0x000C607f, 0x000C607f, 0x000C607f, 0x000C607f, 0x000C807f, 0x000C807f};

SECTION_CG_MISC_DATA 
u32_t ddr3_mem_ctrl0[] = {
    0x0008407f, 0x0008407f, 0x0008407f, 0x0008407f, 0x0008407f, 0x0008407f, 0x0008407f, 0x0008407f,
    0x0008407f, 0x000C607f, 0x000C607f, 0x000C607f, 0x000C607f, 0x000C807f, 0x000C807f};

SECTION_CG_MISC_DATA 
u32_t ddr2_mem_ctrl1[] = {
    0x0000018F, 0x0000018F, 0x4000018F, 0x4000018f, 0x4000018f, 0x4000018f, 0x4000018F, 0x4000018F,
    0x4000018F, 0x4000018F, 0x4000018F, 0x4000018f, 0x4000018f, 0x4000018f, 0x4000018F};

SECTION_CG_MISC_DATA 
u32_t ddr2_mem_ctrl3[] = {
    0x120F0000, 0x140F0000, 0x160F0000, 0x180F0000, 0x1A0F0000, 0x1C0F0000, 0x1E0F0000, 0x200F0000,
    0x220F0000, 0x240F0000, 0x260f0000, 0x280F0000, 0x2A0F0000, 0x2C0F0000, 0x2E0F0000};

SECTION_CG_MISC_DATA 
u32_t ddr3_mem_ctrl1[] = {
    0x4000018f, 0x4000018f, 0x4000018f, 0x4000018f, 0x4000018f, 0x4000018f, 0x4000018f, 0x4000018f,
    0x4000018f, 0x4000018f, 0x4000018f, 0x4000018f, 0x4000018f, 0x4000018f, 0x4000018f};

SECTION_CG_MISC_DATA 
u32_t ddr3_mem_ctrl3[] = {
    0x120F0000, 0x140F0000, 0x160f0000, 0x180f0000, 0x1a0f0000, 0x1c0f0000, 0x1e0f0000, 0x200f0000,
    0x220f0000, 0x240f0000, 0x260f0000, 0x280f0000, 0x2a0f0000, 0x2c0f0000, 0x2e0f0000};

//************************************
// DRAM: Controller Setting & Data Base
//************************************
const mc_dram_param_t ddr2_winbond_W972516KG SECTION_PARAMETERS = {
    #include <dram_models/winbond_W972516KG-18.h>
};

const mc_dram_param_t ddr3_db_300mhz_para SECTION_PARAMETERS = {
    #include "dram_models/winbond_W634GG6LB-11_300mhz.h"
};

const mc_dram_param_t ddr3_db_400mhz_para SECTION_PARAMETERS = {
    #include "dram_models/winbond_W634GG6LB-11_400mhz.h"
};

const mc_dram_param_t ddr3_db_533mhz_para SECTION_PARAMETERS = {
    #include "dram_models/winbond_W634GG6LB-11_533mhz.h"
};

const mc_cntlr_opt_t ddr2_cntlr_opt SECTION_PARAMETERS = {
//    .fast_exit      = 1,
    .normal_drv     = 1,
    .ddr2_odt_val   = 150,
//    .dll_dis        = 0,

    .bus_sync       = 1,
    .dqs0_en_hclk   = 1,
    .dqs1_en_hclk   = 1,
    .cntlr_odt      = 120,
    .cntlr_ocd      = 40,
    .cntlr_odt_clk  = 120,
    .cntlr_ocd_clk  = 40,
    .cntlr_odt_adr  = 120,
    .cntlr_ocd_adr  = 40,
    .cntlr_odt_dq   = 120,
    .cntlr_ocd_dq   = 40,
    .dqs0_en_tap    = 0x1f,
    .dqs1_en_tap    = 0x1f,
    .dqs0_group_tap = 0,
    .dqs1_group_tap = 0,
//    .dqm0_tap       = 0,
//    .dqm1_tap       = 0,
//    .wr_dq_delay    = {0},
//    .rdr_dq_delay   = {0},
//    .rdf_dq_delay   = {0},
    .cal_method     = 1,   // 1: autoK, 0: others
    .dq_dll_type    = 0,   // 1:Analog dll,  0: digital
    .zq_cal_method  = 1,   // 1: autoK, 0: others
    .parallel_bank_en = 1,
    .dbg_en         = 0,
};

#define WR_DELAY_V  (0x8)
const mc_cntlr_opt_t ddr3_cntlr_opt SECTION_PARAMETERS = {
//    .fast_exit      = 0,   // fast exit (dll on), slow exit (dll off)
    .rtt_nom_ohm    = 120,
    .rtt_wr_ohm     = 0,
    .normal_drv     = 1,
//    .dll_dis        = 0,
//    .write_lvl_en   = 1,
    
    .bus_sync       = 1,
    .dqs0_en_hclk   = 1,
    .dqs1_en_hclk   = 1,
    .cntlr_odt      = 120,
    .cntlr_ocd      = 48, 
    .cntlr_odt_clk  = 120,
    .cntlr_ocd_clk  = 34,
    .cntlr_odt_adr  = 120,
    .cntlr_ocd_adr  = 60,
    .cntlr_odt_dq   = 120,
    .cntlr_ocd_dq   = 40,
    .dqs0_en_tap    = 0x1f,
    .dqs1_en_tap    = 0x1f,
    .dqs0_group_tap = 0,
    .dqs1_group_tap = 0,
//    .dqm0_tap       = 0,
//    .dqm1_tap       = 0,
//    .wr_dq_delay    = {0},
//    .rdr_dq_delay   = {0},
//    .rdf_dq_delay   = {0},
    .cal_method     = 1,   // 1: autoK, 0: others
    .dq_dll_type    = 0,   // 1: Analog dll,  0: digital
    .zq_cal_method  = 1,   // 1: autoK, 0: others
    .parallel_bank_en = 1,
    .dbg_en         = 0,
};

const mc_register_set_t regset SECTION_PARAMETERS = {
    .mcr.v     = MCRdv,
    .dcr.v     = DCRdv,
    .dtr0.v    = DTR0dv,
    .dtr1.v    = DTR1dv,
    .dtr2.v    = DTR2dv,
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

const mc_info_t mi_ddr2[] SECTION_PARAMETERS = {
    {(mc_dram_param_t *)&ddr2_winbond_W972516KG,
     (mc_cntlr_opt_t *)&ddr2_cntlr_opt,
     (mc_register_set_t *)&regset},
};

const mc_info_t mi_ddr3[] SECTION_PARAMETERS = {
    {(mc_dram_param_t *)&ddr3_db_300mhz_para,
     (mc_cntlr_opt_t *)&ddr3_cntlr_opt,
     (mc_register_set_t *)&regset},
    {(mc_dram_param_t *)&ddr3_db_400mhz_para,
     (mc_cntlr_opt_t *)&ddr3_cntlr_opt,
     (mc_register_set_t *)&regset},
    {(mc_dram_param_t *)&ddr3_db_533mhz_para,
     (mc_cntlr_opt_t *)&ddr3_cntlr_opt,
     (mc_register_set_t *)&regset}
};


MEMCNTLR_SECTION
const mc_info_t *
proj_mc_info_select(void) {
    if (RFLD_MCR(dram_type) == 1) {
		return &mi_ddr2[0];
	}else if (RFLD_MCR(dram_type) == 2) {
		u32_t mem_ps, size, i;
        mem_ps = 1000*1000/cg_dev_freq_info.mem_mhz;
        size = sizeof(mi_ddr3)/sizeof(mc_info_t);
        i=0;
        while (i<size) {
            if ((mem_ps >= mi_ddr3[i].dram_param->min_tck_ps) &&
                (mem_ps < mi_ddr3[i].dram_param->max_tck_ps)){
                return &mi_ddr3[i];
            }
            i++;
        }
        return &mi_ddr3[0];
	}else{
        return VZERO;
    }
}

//*************************************
// CG: OCP, MEM, LX, UART Clock Setting
//*************************************
const cg_dev_freq_t cg_ddr2_proj_freq SECTION_PARAMETERS = {
    .ocp_mhz  = 625,
    .mem_mhz  = 525,
    .lx_mhz   = 200,
    .spif_mhz = 50
};

const cg_dev_freq_t cg_ddr3_proj_freq SECTION_PARAMETERS = {
    .ocp_mhz  = 725,
    .mem_mhz  = 525,
    .lx_mhz   = 200,
    .spif_mhz = 50
};

u32_t uart_baud_rate =115200;
symb_fdefine(SF_SYS_UDELAY, otto_lx_timer_udelay);
symb_fdefine(SF_SYS_MDELAY, otto_lx_timer_mdelay);
symb_fdefine(SF_SYS_GET_TIMER, otto_lx_timer_get_timer);
symb_fdefine(SF_PROTO_PRINTF, proto_printf);

