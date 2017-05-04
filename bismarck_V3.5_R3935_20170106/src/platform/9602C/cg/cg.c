#include <soc.h>
#include <uart/uart.h>
#include <cg/cg.h>
#include <dram/memcntlr.h>

#ifndef SECTION_CG_CORE_INIT
    #define SECTION_CG_CORE_INIT
#endif

#ifndef SECTION_CG_MISC
    #define SECTION_CG_MISC
#endif

#ifndef SECTION_CG_MISC_DATA
    #define SECTION_CG_MISC_DATA
#endif

/*  Declaration of global parameter and functions */
cg_dev_freq_t cg_dev_freq_info;
void reg_to_mhz(void);




SECTION_CG_MISC 
u32_t cg_query_freq(u32_t dev_type)
{
	if(0 == cg_dev_freq_info.ocp_mhz){
		reg_to_mhz();
	}
    return CG_QUERY_FREQUENCY(dev_type,(&cg_dev_freq_info));
}




#ifdef OTTO_PROJECT_FPGA
__attribute__((weak)) void copy_proj_info_to_sram(void) {	return;}


SECTION_CG_MISC 
void reg_to_mhz(void)
{
    memcpy(&cg_dev_freq_info, &cg_proj_freq, sizeof(cg_dev_freq_t));
}

SECTION_CG_MISC 
void cg_result_decode(void)
{
    u32_t val = REG32(0xB8000308);
    if(cg_dev_freq_info.ocp_mhz > cg_dev_freq_info.mem_mhz){
        val = val & 0xFFFFFFEF;
    }else{
        val = val | 0x10;
    }
    if(cg_dev_freq_info.lx_mhz > cg_dev_freq_info.mem_mhz){
        val = val & 0xFFFFFFF8;
    }else{
        val = val | 0x7; 
    }
    REG32(0xB8000308) = val;

    u32_t dmcr = DMCRrv;
    DMCRrv = dmcr;

	printf("II: OCP %dMHz, MEM %dMHz, LX %dMHz, SPIF %dMHz\n",
		cg_dev_freq_info.ocp_mhz,
		cg_dev_freq_info.mem_mhz,
		cg_dev_freq_info.lx_mhz,
		cg_dev_freq_info.spif_mhz);
	return;
}

SECTION_CG_MISC 
void cg_init(void)
{
    reg_to_mhz();    
    uart_init(uart_baud_rate, cg_dev_freq_info.lx_mhz);
    cg_result_decode();
}


#else //#ifdef OTTO_PROJECT_FPGA
SECTION_CG_MISC 
void copy_proj_info_to_sram(void)
{
    if(3 == (RFLD_MCR(dram_type)+1)){
        memcpy((char *)&cg_dev_freq_info, (const char *)&cg_ddr3_proj_freq, sizeof(cg_dev_freq_t));
    }else{
        memcpy((char *)&cg_dev_freq_info, (const char *)&cg_ddr2_proj_freq, sizeof(cg_dev_freq_t));
    }
}

UTIL_FAR SECTION_CG_CORE_INIT 
static u32_t cg_udelay(u32_t us, u32_t mhz)
{
    u32_t loop_cnt = us*mhz/2;
    while(loop_cnt--);
    return loop_cnt;
}
#if 0
UTIL_FAR SECTION_CG_MISC 
void reg_set_info_to_mhz(cg_register_set_t *rs)
{
    /* OCP */
    cg_dev_freq_info.ocp_mhz = ((rs->sys_ocp_pll_ctrl0.f.cpu1_n_l+2) * 25)/(1<<rs->sys_ocp_pll_ctrl0.f.cpu1_clk_sel_l);   
    if(rs->oc0_cmugcr.f.cmu_mode == ENABLE_CMU_FIX_MODE){
	    cg_dev_freq_info.ocp_mhz /= (1 << rs->oc0_cmugcr.f.freq_div);
    }

    /* MEM */
    cg_dev_freq_info.mem_mhz = (25*((rs->sys_mem_pll_ctrl3.f.n_code)+2)/(2*(1<<rs->sys_mem_pll_ctrl2.f.pdiv)));

    /* LX */
    cg_dev_freq_info.lx_mhz  = 1000 / (rs->phy_rg3x_pll.f.cfg_div_lx + 5);

    /* Flash: SPI NAND or SPI NOR*/
    #if defined (OTTO_FLASH_SPI_NAND)
        cg_dev_freq_info.spif_mhz = cg_dev_freq_info.lx_mhz / ((rs->sclk_div2ctrl + 1)*2);
    #elif defined (OTTO_FLASH_NOR)
        cg_dev_freq_info.nor_pll_mhz = 1000 / (rs->phy_rg3x_pll.f.cfg_div_spif + 4);
        cg_dev_freq_info.spif_mhz = cg_dev_freq_info.nor_pll_mhz / ((rs->sclk_div2ctrl + 1)*2);
    #endif
}
#endif

UTIL_FAR SECTION_CG_MISC 
void reg_to_mhz(void)
{
    /* OCP */
    cg_dev_freq_info.ocp_mhz = (((RFLD_SYS_OCP_PLL_CTRL0(cpu1_n_l) + 2) * 25)/(1<<RFLD_SYS_OCP_PLL_CTRL0(cpu1_clk_sel_l)));
    if(0 != RFLD_OC0_CMUGCR(cmu_mode)){
	    cg_dev_freq_info.ocp_mhz /= (1 << RFLD_OC0_CMUGCR(freq_div));
    }
    
    /* MEM */
    cg_dev_freq_info.mem_mhz = (25*((RFLD_SYS_MEM_PLL_CTRL3(n_code))+2)/(2*(1<<RFLD_SYS_MEM_PLL_CTRL2(pdiv))));

    /* LX */
    cg_dev_freq_info.lx_mhz  = 1000 / (RFLD_PHY_RG3X_PLL(cfg_div_lx) + 5);

    /* Flash: SPI NAND or SPI NOR*/
    #if defined OTTO_FLASH_SPI_NAND
        cg_dev_freq_info.spif_mhz = cg_dev_freq_info.lx_mhz / ((get_spi_ctrl_divisor() + 1)*2);
    #elif defined (OTTO_FLASH_NOR)
        cg_dev_freq_info.nor_pll_mhz = 1000 / (RFLD_PHY_RG3X_PLL(cfg_div_spif) + 4);
        cg_dev_freq_info.spif_mhz = cg_dev_freq_info.nor_pll_mhz / ((get_spi_ctrl_divisor() + 1)*2);
    #endif
}

SECTION_CG_MISC 
void _cg_calc_ocp(cg_register_set_t *rs)
{
#define MAX_CMU_DIV_SEL (1)
#define MAX_CPU_DIV_SEL (2)

    u32_t cur_clk = 0;
    u32_t calc_clk;
    u32_t cpu_div=0xFFFFFFFF;
    u32_t input_pll;
    
    u32_t target_cpu_clk = (cg_dev_freq_info.ocp_mhz>CPU_CLK_MAX)?CPU_CLK_MAX:cg_dev_freq_info.ocp_mhz;
    u32_t use_cmu_mode = (cg_dev_freq_info.ocp_mhz<CPU_CLK_MIN)?ENABLE_CMU_FIX_MODE:0;
    
    rs->oc0_cmugcr.f.cmu_mode = use_cmu_mode;

    if(target_cpu_clk>=OCP_PLL_MIN){
        rs->sys_ocp_pll_ctrl0.f.cpu1_n_l = (target_cpu_clk/25) - 2;
        rs->sys_ocp_pll_ctrl0.f.cpu1_clk_sel_l = 0;
    }else if(DISABLE_CMU_FIX_MODE == use_cmu_mode){
        for(cpu_div=1 ; cpu_div<=2 ; cpu_div++){
            for(input_pll=OCP_PLL_MIN ; input_pll<=OCP_PLL_MAX ; input_pll+=25){
                calc_clk = input_pll/(1<<cpu_div);
                if((target_cpu_clk >= calc_clk) && (calc_clk > cur_clk)){
                    cur_clk = calc_clk;
                    rs->sys_ocp_pll_ctrl0.f.cpu1_n_l = (input_pll/25) - 2;
                    rs->sys_ocp_pll_ctrl0.f.cpu1_clk_sel_l = cpu_div;
                    if(target_cpu_clk == calc_clk) return;
                }
            }
        }
    }else if(ENABLE_CMU_FIX_MODE == use_cmu_mode){
        u32_t cmu_div;
        for(cmu_div=0 ; cmu_div<=MAX_CMU_DIV_SEL ; cmu_div++){
            for(cpu_div=0 ; cpu_div<=MAX_CPU_DIV_SEL ; cpu_div++){
                for(input_pll=OCP_PLL_MIN ; input_pll<=OCP_PLL_MAX ; input_pll+=25){
                    calc_clk = (input_pll/(1<<cpu_div))/(1<<cmu_div);
//                    printf("cpu_div=%d, cmu_div=%d, calc_clk=%d, cur_clk=%d, input_pll=%d\n",cpu_div,cmu_div,calc_clk,cur_clk,input_pll);
                    if((target_cpu_clk >= calc_clk) && (calc_clk > cur_clk)){
                        printf("II: cpu_div=%d, cmu_div=%d, calc_clk=%d, cur_clk=%d, input_pll=%d\n",cpu_div,cmu_div,calc_clk,cur_clk,input_pll);
                        cur_clk = calc_clk;
                        rs->sys_ocp_pll_ctrl0.f.cpu1_n_l = (input_pll/25) - 2;
                        rs->sys_ocp_pll_ctrl0.f.cpu1_clk_sel_l = cpu_div;
                        rs->oc0_cmugcr.f.freq_div = cmu_div;
                        if(target_cpu_clk == calc_clk) return;
                    }
                }
            }       
        }
    }
}



extern u32_t ddr2_mem_ctrl0[];
extern u32_t ddr3_mem_ctrl0[];
extern u32_t ddr2_mem_ctrl1[];
extern u32_t ddr3_mem_ctrl1[];
extern u32_t mem_ctrl2[];
extern u32_t ddr2_mem_ctrl3[];
extern u32_t ddr3_mem_ctrl3[];


SECTION_CG_MISC 
void _cg_calc_mem(cg_register_set_t *rs)
{   
    u16_t target_freq = cg_dev_freq_info.mem_mhz;
    u32_t *ptr0, *ptr1, *ptr3;
    
    if (100 == target_freq) {
        rs->sys_mem_pll_ctrl0.v = 0x0000007F;
        rs->sys_mem_pll_ctrl1.v = 0x000001EF;
        rs->sys_mem_pll_ctrl2.v = 0x26226400;
        rs->sys_mem_pll_ctrl3.v = 0x0E0F0000;
    }else{
        if(target_freq > MEM_PLL_MAX){
            target_freq = MEM_PLL_MAX;
        }else if(target_freq < MEM_PLL_MIN){
            target_freq = MEM_PLL_MIN;
        }
    
        if(2 == (RFLD_MCR(dram_type)+1)){
            ptr0 = ddr2_mem_ctrl0;
            ptr1 = ddr2_mem_ctrl1;
            ptr3 = ddr2_mem_ctrl3;
        }else{
            ptr0 = ddr3_mem_ctrl0;
            ptr1 = ddr3_mem_ctrl1;
            ptr3 = ddr3_mem_ctrl3;
        }

        u32_t idx = (target_freq - MEM_PLL_MIN)/25;
        rs->sys_mem_pll_ctrl0.v = ptr0[idx];
        rs->sys_mem_pll_ctrl1.v = ptr1[idx];
        rs->sys_mem_pll_ctrl2.v = mem_ctrl2[idx];
        rs->sys_mem_pll_ctrl3.v = ptr3[idx];
    }
    
    rs->sys_mem_pll_ctrl5.v = SYS_MEM_PLL_CTRL5dv;
    rs->sys_mem_pll_ctrl6.v = SYS_MEM_PLL_CTRL6dv;
}


SECTION_CG_MISC_DATA 
pll_freq_sel_info_t lx_pll_src[] = {
    {
        .freq_mhz = 200, 
        .cfg_div  = 0,
    },
    {
        .freq_mhz = 166, 
        .cfg_div  = 1,
    },
    {
        .freq_mhz = 142, 
        .cfg_div  = 2,
    },
    {
        .freq_mhz = 125, 
        .cfg_div  = 3,
    },
    { /* The end of structure */
        .freq_mhz = END_OF_INFO,
    }
};

SECTION_CG_MISC 
void _cg_calc_lx(cg_register_set_t *rs)
{
    u32_t target_freq = cg_dev_freq_info.lx_mhz;
    int i = -1;
    u32_t cur_lx;
    pll_freq_sel_info_t final = {0 ,0};

    while(1){
        cur_lx = lx_pll_src[++i].freq_mhz;
        if(END_OF_INFO == cur_lx) break;
        if((target_freq >= cur_lx) && (cur_lx > final.freq_mhz)){
            final.freq_mhz = cur_lx;
            final.cfg_div = lx_pll_src[i].cfg_div;
        }
    }

    if(0 == final.freq_mhz){
        final.freq_mhz = LX_PLL_DEFAULT;
        final.cfg_div = LX_PLL_DEFAULT_DIV;
        puts("WW: Unreasonable LX PLL. Set to Default.\n");
    }
    
    rs->phy_rg3x_pll.f.cfg_div_lx = final.cfg_div;
}



#if defined (OTTO_FLASH_SPI_NAND) || defined (OTTO_FLASH_NOR)

extern clk_div_sel_info_t sclk_divisor[];

SECTION_CG_MISC 
void __cal_spi_ctrl_divisor(cg_sclk_info_t *info)
{
    u32_t target_sclk = cg_dev_freq_info.spif_mhz;
    u32_t input_pll   = info->pll_freq_mhz;
    u32_t cur_sclk    = info->cal_sclk_mhz;
    int i = -1;
    u32_t cur_divisor;
    u32_t calc_sclk;

    while(1){
        cur_divisor = sclk_divisor[++i].divisor;
        
        if(END_OF_INFO == cur_divisor) break;

        calc_sclk = input_pll/cur_divisor;
        if((target_sclk >= calc_sclk) && (calc_sclk > cur_sclk)){
            cur_sclk = calc_sclk;
            info->cal_sclk_mhz  = calc_sclk;
            info->sclk_div2ctrl = sclk_divisor[i].div_to_ctrl;
        }
    }
}
#endif //#if defined (OTTO_FLASH_SPI_NAND) || defined (OTTO_FLASH_ONFI)


#if defined (OTTO_FLASH_SPI_NAND)
SECTION_CG_MISC 
void _cg_calc_spif(cg_register_set_t *rs)
{
    cg_sclk_info_t final = {0, 0, 0, 0};
    final.pll_freq_mhz    = cg_dev_freq_info.lx_mhz;    

    __cal_spi_ctrl_divisor(&final);

    if(0 == final.cal_sclk_mhz){
        final.sclk_div2ctrl = get_default_spi_ctrl_divisor();
    }
    
    rs->sclk_div2ctrl = final.sclk_div2ctrl;
}

SECTION_CG_CORE_INIT 
void cg_spif_clk_init(cg_register_set_t *rs)
{
    set_spi_ctrl_divisor(rs->sclk_div2ctrl, cg_dev_freq_info.spif_mhz);
}

#elif defined (OTTO_FLASH_NOR)
SECTION_CG_MISC_DATA 
pll_freq_sel_info_t nor_pll_src[] = {
    {
        .freq_mhz = 250, 
        .cfg_div  = 0,
    },
    {
        .freq_mhz = 200, 
        .cfg_div  = 1,
    },
    {
        .freq_mhz = 166, 
        .cfg_div  = 2,
    },
    {
        .freq_mhz = 142, 
        .cfg_div  = 3,
    },
    { /* The end of structure */
        .freq_mhz = END_OF_INFO,
    }
};

SECTION_CG_MISC 
void _cg_calc_spif(cg_register_set_t *rs)
{
    cg_sclk_info_t current = {0 ,0 ,0 ,0};
    cg_sclk_info_t final = {0 ,0 ,0 ,0};
    
    int i = -1;
    while(1){
        current.pll_freq_mhz = nor_pll_src[++i].freq_mhz;
        if(END_OF_INFO == current.pll_freq_mhz) break;
        __cal_spi_ctrl_divisor(&current);

        if(current.cal_sclk_mhz > final.cal_sclk_mhz){
            final.pll_freq_mhz     = nor_pll_src[i].freq_mhz;
            final.pll_cfg_div      = nor_pll_src[i].cfg_div;
            final.sclk_div2ctrl = current.sclk_div2ctrl;
            final.cal_sclk_mhz     = current.cal_sclk_mhz;
        }
    }

    if(0 == final.cal_sclk_mhz){
        final.pll_cfg_div = NOR_PLL_DEFAULT_DIV;
        final.sclk_div2ctrl = get_default_spi_ctrl_divisor();        
        puts("WW: Unreasonable SCLK. Set to Default divisor.\n");
    }
    
    rs->phy_rg3x_pll.f.cfg_div_spif = final.pll_cfg_div;
    rs->sclk_div2ctrl = final.sclk_div2ctrl;
}

SECTION_CG_CORE_INIT 
void cg_spif_clk_init(cg_register_set_t *rs)
{
    RMOD_PHY_RG3X_PLL(cfg_div_spif, rs->phy_rg3x_pll.f.cfg_div_spif);
    set_spi_ctrl_divisor(rs->sclk_div2ctrl, cg_dev_freq_info.spif_mhz);

    //Set OCP to SPI NOR slow bit
    rs->oc0_cmucr.f.se_spif = (cg_dev_freq_info.nor_pll_mhz > cg_dev_freq_info.ocp_mhz);
    RMOD_OC0_CMUCR(se_spif, rs->oc0_cmucr.f.se_spif);
}
#endif //#elif defined (OTTO_FLASH_NOR)


SECTION_CG_MISC 
void get_cg_reg_init_value(cg_register_set_t *rs)
{
    rs->sys_ocp_pll_ctrl0.v = SYS_OCP_PLL_CTRL0rv;
    rs->sys_mem_pll_ctrl0.v = SYS_MEM_PLL_CTRL0rv;
    rs->sys_mem_pll_ctrl1.v = SYS_MEM_PLL_CTRL1rv;
    rs->sys_mem_pll_ctrl2.v = SYS_MEM_PLL_CTRL2rv;
    rs->sys_mem_pll_ctrl3.v = SYS_MEM_PLL_CTRL3rv;
    rs->sys_mem_pll_ctrl5.v = SYS_MEM_PLL_CTRL5rv;
    rs->sys_mem_pll_ctrl6.v = SYS_MEM_PLL_CTRL6rv;
    rs->phy_rg3x_pll.v = PHY_RG3X_PLLrv;
    rs->oc0_cmugcr.v = OC0_CMUGCRrv;
    rs->oc0_cmucr.v  = OC0_CMUCRrv;
    rs->lx0_cmucr.v  = LX0_CMUCRrv;
    rs->lx1_cmucr.v  = LX1_CMUCRrv;
    rs->lx2_cmucr.v  = LX2_CMUCRrv;
    rs->lxp_cmucr.v  = LXP_CMUCRrv;
}

SECTION_CG_MISC 
void xlat_freq_n_assign_reg(cg_register_set_t *rs)
{
    _cg_calc_ocp(rs);	
    _cg_calc_lx(rs);
    _cg_calc_mem(rs);
    #if defined (OTTO_FLASH_SPI_NAND) || defined (OTTO_FLASH_NOR)
    //This function only used in SPI NAND or SPI NOR template
    _cg_calc_spif(rs);	
    #endif
}


UTIL_FAR SECTION_CG_MISC 
void cg_mem_pll_init(cg_register_set_t *rs)
{
    u32_t ocp_freq = cg_dev_freq_info.ocp_mhz; 

    CG_MEM_PLL_OE_DIS();
    cg_udelay(200, ocp_freq);

    SYS_MEM_PLL_CTRL0rv = rs->sys_mem_pll_ctrl0.v;
    SYS_MEM_PLL_CTRL1rv = rs->sys_mem_pll_ctrl1.v;
    SYS_MEM_PLL_CTRL2rv = rs->sys_mem_pll_ctrl2.v;
    SYS_MEM_PLL_CTRL3rv = rs->sys_mem_pll_ctrl3.v;
    SYS_MEM_PLL_CTRL5rv = SYS_MEM_PLL_CTRL5dv;
    SYS_MEM_PLL_CTRL6rv = SYS_MEM_PLL_CTRL6dv;
    cg_udelay(5, ocp_freq);
    CG_MEM_PLL_OE_EN();
    cg_udelay(200, ocp_freq);


    //Check for LX/OCP to DRAM slow bits.
    //Conditions:
    //  Should: OCP  》」 clkm
    //  Could:   OCP  》   clkm *2
    //  Prohibit:OCP  「」 clkm *2
    rs->oc0_cmucr.f.se_dram = (!(cg_dev_freq_info.ocp_mhz >= (cg_dev_freq_info.mem_mhz *2)));
    rs->lx0_cmucr.f.se_dram = (cg_dev_freq_info.mem_mhz > cg_dev_freq_info.lx_mhz);
    rs->lx1_cmucr.f.se_dram = rs->lx0_cmucr.f.se_dram;
    rs->lx2_cmucr.f.se_dram = rs->lx0_cmucr.f.se_dram;
    rs->lxp_cmucr.f.se_dram = rs->lx0_cmucr.f.se_dram;        

    RMOD_OC0_CMUCR(se_dram, rs->oc0_cmucr.f.se_dram);
    RMOD_LX0_CMUCR(se_dram, rs->lx0_cmucr.f.se_dram);
    RMOD_LX1_CMUCR(se_dram, rs->lx1_cmucr.f.se_dram);
    RMOD_LX2_CMUCR(se_dram, rs->lx2_cmucr.f.se_dram);
    RMOD_LXP_CMUCR(se_dram, rs->lxp_cmucr.f.se_dram);
}

SECTION_CG_CORE_INIT 
inline static void cg_cpu_clk_init(cg_register_set_t *rs)
{
    /* Step1: Change OCP PLL to a stable frequency (LX) */
    RMOD_SYS_STATUS(cf_ckse_ocp0, 0);
    u32_t lx_freq = cg_dev_freq_info.lx_mhz;
    cg_udelay(1, lx_freq);
    
    /* Step2: Restore cpu pll to default & Set CPU1_N_L to change CPU PLL*/
    SYS_OCP_PLL_CTRL0rv = rs->sys_ocp_pll_ctrl0.v;

    /* Step3: Delay 40 us for safe*/
    cg_udelay(40, lx_freq);

    /* Step4: Recover OCP PLL */
    RMOD_SYS_STATUS(cf_ckse_ocp0, 1);
    cg_udelay(1, lx_freq);

    /* Step5: Set CMU divisor */
    if(rs->oc0_cmugcr.f.cmu_mode){
        RMOD_OC0_CMUGCR(freq_div, rs->oc0_cmugcr.f.freq_div, cmu_mode, ENABLE_CMU_FIX_MODE);
    }
#if 0
    u32_t i;
    for(i=0; i<=1 ; i++){
        if((rs->oc0_cmugcr.f.cmu_mode)&&(i==1)){
            RMOD_OC0_CMUGCR(freq_div, rs->oc0_cmugcr.f.freq_div, cmu_mode, ENABLE_CMU_FIX_MODE);
        }
    }
#endif
}

SECTION_CG_CORE_INIT 
static inline void cg_lx_pll_init(cg_register_set_t *rs)
{
    if(cg_dev_freq_info.lx_mhz != LX_PLL_DEFAULT){
        PHY_RG3X_PLLrv = rs->phy_rg3x_pll.v;
    }
}

SECTION_CG_MISC 
void cg_set_bus_aync(void)
{
    if(meminfo.cntlr_opt->bus_sync){
        printf("II: Enable Triple Synchronizer\n");
        RMOD_MCR(sync_ocp0_dram, 1, sync_lx0_dram, 1, sync_lx1_dram, 1, sync_lx2_dram, 1);
        RMOD_PBOLSRR(sync_pbo_uw_dram, 1, sync_pbo_ur_dram, 1, sync_pbo_dw_dram, 1, sync_pbo_dr_dram, 1);
    }
}

SECTION_CG_MISC 
void cg_register_preset(cg_register_set_t *rs)
{
    //Step1: Get all register default values for initialization
    get_cg_reg_init_value(rs);

    //Step2: Translate the dev_freq into register
    xlat_freq_n_assign_reg(rs);

    //Step3: Check the buses synchronously interface setting
    cg_set_bus_aync();
}

SECTION_CG_CORE_INIT 
void cg_init_in_unswap_ram(cg_register_set_t *rs)
{
    //Step1: Init OCP PLL
    cg_cpu_clk_init(rs);
    
    //Step2: Init LX PLL
    cg_lx_pll_init(rs);    

    #if defined (OTTO_FLASH_SPI_NAND) || defined (OTTO_FLASH_NOR)
    //Step3: Only for SPI NAND or SPI NOR to change SCLK
    cg_spif_clk_init(rs);
    #endif

    //Step4: Setting UART baudrate
    uart_init(uart_baud_rate, cg_dev_freq_info.lx_mhz);

    //Step5: Add Branch prediction barrier
    BP_BARRIER();
}

SECTION_CG_MISC 
void cg_init_in_swap_ram(cg_register_set_t *rs)
{
    //Step5: Setting LX Timer
    lx_timer_init(cg_dev_freq_info.lx_mhz);

    //Step6: Init Momory PLL
    cg_mem_pll_init(rs);
}

SECTION_CG_MISC 
void cg_result_decode(void)
{
    printf("II: CPU %dMHz (%d/%d/%d), MEM %dMHz, LX %dMHz",
        cg_dev_freq_info.ocp_mhz,
        ((RFLD_SYS_OCP_PLL_CTRL0(cpu1_n_l) + 2) * 25),
        (1<<RFLD_SYS_OCP_PLL_CTRL0(cpu1_clk_sel_l)),
        (RFLD_OC0_CMUGCR(cmu_mode)!=0?(1 << RFLD_OC0_CMUGCR(freq_div)):0),
        cg_dev_freq_info.mem_mhz,
        cg_dev_freq_info.lx_mhz);
    
    #ifdef OTTO_FLASH_ONFI
        puts("\n");
    #else
        printf(", SPIF %dMHz\n", cg_dev_freq_info.spif_mhz);
    #endif
}

SECTION_CG_MISC 
void cg_init(void)
{
    cg_register_set_t cg_rs;

    cg_register_preset(&cg_rs);

    cg_init_in_unswap_ram(&cg_rs);

    cg_init_in_swap_ram(&cg_rs);

    reg_to_mhz();

    cg_result_decode();
}
#endif //#ifdef OTTO_PROJECT_FPGA

REG_INIT_FUNC(copy_proj_info_to_sram, 12);
REG_INIT_FUNC(cg_init, 13);
symb_pdefine(cg_info_dev_freq, SF_SYS_CG_DEV_FREQ, (void *)(&cg_dev_freq_info));


