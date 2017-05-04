#include <dram/memcntlr.h>
#include <dram/autok/dram_autok.h>
#include <cg/cg.h>


extern void memctlc_config_DTR(unsigned int default_instruction, unsigned int dram_size);
extern u32_t DDR_Calibration(unsigned char full_scan);
extern unsigned int memctlc_config_DRAM_size(void);
extern int memctlc_ZQ_calibration(unsigned int auto_cali_value);
extern void memctlc_mempll_setup(void);
extern void memctlc_dll_setup(void);
extern void memctlc_config_delay_line(unsigned int dram_freq_mhz);

__attribute__((weak)) u8_t efuse_6_rd(void) {
	return 0;
}

SECTION_AUTOK
void memctlc_dram_phy_reset(void)
{
    REG32(DACCR) = REG32(DACCR) & ((u32_t) 0xFFFFFFEF);
    REG32(DACCR) = REG32(DACCR) | ((u32_t) 0x10);

    return;
}

SECTION_AUTOK
void memctlc_AddrCtrl_output_driving(void)
{
	volatile unsigned int *DDR_MA_PAD_DRV_ADDR;
	unsigned int delay=0;
	DDR_MA_PAD_DRV_ADDR = (volatile unsigned int *) 0xb8000140;
	//*DDR_MA_PAD_DRV_ADDR = (1<<25) |(1<<16) |(1<<9) | (1<<0);					//n25: Output driving control. Active high, n16: MISC-Force_Mode, n9: Output driving control. Active high, n0: MBA-Force_Mode
	*DDR_MA_PAD_DRV_ADDR = (1<<16) | (1<<0);					//n25: Output driving control. Active high, n16: MISC-Force_Mode, n9: Output driving control. Active high, n0: MBA-Force_Mode
	for(delay=0;delay<0x1ffff;delay++) ;
	
}

void dram_ZQCS_ZQCL_enable(void)
{
    volatile unsigned int *ZQ_ctrl, *Sil_pat, *dmcr;

    ZQ_ctrl = (volatile unsigned int *)D3ZQCCR;
    Sil_pat = (volatile unsigned int *)DACSPCR;
    dmcr = (volatile unsigned int *)DMCR;

    /* ZQCL Trigger */
    *ZQ_ctrl = *ZQ_ctrl | ZQ_LONG_TRI;
    while(*ZQ_ctrl & ZQ_LONG_TRI_BUSY);

    /* ZQCS Enable */
    *Sil_pat = *Sil_pat | AC_SILEN_PERIOD_EN | AC_SILEN_PERIOD_UNIT | AC_SILEN_PERIOD;        //AC_SILEN_PERIOD_UNIT x AC_SILEN_PERIOD = ZQCS period
    *ZQ_ctrl = *ZQ_ctrl | ZQ_SHORT_EN | T_ZQCS;  //T_ZQCS means the ZQCS requires time, which dram can't access data

    /* DMCR update */
    *dmcr = *dmcr;
    puts("\nAK: ZQCL done & ZQCS Enable\n\r");
    return;
}


SECTION_AUTOK
u32_t dram_autok(void) {
	unsigned int dram_size=0;
	u32_t ret = 0;
    u32_t e2, nt, plsb, pt, ttcn, ttfn, ttcp, ttfp;
    
#define ZQ_E2(val)    ((val>>0) & 0x1)
#define ZQ_NT(val)    ((val>>1) & 0x7)
#define ZQ_PLSB0(val) ((val>>4) & 0x1)
#define ZQ_PT(val)    ((val>>5) & 0x7)
#define ZQ_TTCN(val)  ((val>>8) & 0x7)
#define ZQ_TTFN(val)  ((val>>11) & 0x7)
#define ZQ_PLSB1(val) ((val>>14) & 0x1)
#define ZQ_TTCP(val)  ((val>>15) & 0x7)
#define ZQ_TTFP(val)  ((val>>18) & 0x7)
#define ZQ_PLSB(val)  (ZQ_PLSB0(val)|(ZQ_PLSB1(val)<<1))

	printf("\nAK: DRAM AUTO CALIBRATION\n");

	do {
		/* gerneal DTR config */
		memctlc_config_DTR(1, dram_size);

        /* ZQ Separate Calibration for [CLK, Addr, DQ]*/
        u32_t zprog[3]={mc_xlat_zprog_value(PROJ_PARA_CNTLR_ODT_CLK, PROJ_PARA_CNTLR_OCD_CLK), mc_xlat_zprog_value(PROJ_PARA_CNTLR_ODT_ADR, PROJ_PARA_CNTLR_OCD_ADR), mc_xlat_zprog_value(PROJ_PARA_CNTLR_ODT_DQ, PROJ_PARA_CNTLR_OCD_DQ)};
        u32_t reg_val, final[3]={0,0,0}; //CLK, ADR, DQ
        u32_t i;
        for(i=0; i<3; i++){
            memctlc_ZQ_calibration(zprog[i]);
            
            reg_val = DDZQPSRrv;
            e2 = ZQ_E2(reg_val);
            nt = ZQ_NT(reg_val);
            plsb = ZQ_PLSB(reg_val);
            pt = ZQ_PT(reg_val);
            ttcn = ZQ_TTCN(reg_val);
            ttfn = ZQ_TTFN(reg_val);
            ttcp = ZQ_TTCP(reg_val);
            ttfp = ZQ_TTFP(reg_val);

            switch(i){
                case 0:            
                    final[0] = (1<<0)|(1<<16) | (plsb<<1)|(plsb<<17) | (pt<<3)|(pt<<19) | (nt<<6)|(nt<<22) | (e2<<9)|(e2<<25);                    
                    #ifdef AUTOK_DEBUG
                    printf("ZQ: CLK: zprog(0x%08x), result(0x%08x): e2/nt/plsb/pt=",zprog[i], final[0]);
                    printf("0x%x / 0x%x / 0x%x / 0x%x\n", e2, nt, plsb, pt);
                    #endif
                    break;
                    
                case 1:            
                    final[1] = (1<<0)|(1<<16) | (plsb<<1)|(plsb<<17) | (pt<<3)|(pt<<19) | (nt<<6)|(nt<<22) | (e2<<9)|(e2<<25);
                    #ifdef AUTOK_DEBUG
                    printf("ZQ: ADR: zprog(0x%08x), result(0x%08x): e2/nt/plsb/pt=",zprog[i], final[1]);
                    printf("0x%x / 0x%x / 0x%x / 0x%x\n", e2, nt, plsb, pt);
                    #endif
                    break;                       

                case 2:
                    final[2] = (ttfp<<0)|(ttfn<<3)|(ttcp<<6)|(ttcn<<9)|(plsb<<12)|(pt<<14)|(nt<<17)|(e2<<20)|(1<<21);
                    #ifdef AUTOK_DEBUG
                    printf("ZQ: DQ: zprog(0x%08x), result(0x%08x): ttfp/ttfn/ttcp/ttcn/e2/nt/plsb/pt=",zprog[i], final[2]);
                    printf("0x%x / 0x%x / 0x%x / 0x%x / 0x%x / 0x%x / 0x%x / 0x%x\n", ttfp, ttfn, ttcp, ttcn, e2, nt, plsb, pt);
                    #endif
                    break;
            }
        }

		/*ZQ Configuration*/
        memctlc_ZQ_calibration(PROJ_PARA_CNTLR_ZPROG);
        #ifdef AUTOK_DEBUG
        printf("ZQ: Default REG32(0xB8001098)=0x%08x\n",REG32(0xB8001098));
        #endif
        if(RFLD_MCR(dram_type)==2){
            dram_ZQCS_ZQCL_enable();
        }
        
        /* ZQ Separate Calibration for [CLK, Addr, DQ]*/
        REG32(0xb800012C) = final[0];
        REG32(0xb8000140) = final[1];
        
        REG32(0xb8000118) = final[2];
        REG32(0xb800011c) = final[2];
        REG32(0xb8000120) = final[2];
        REG32(0xb8000124) = final[2];

#ifdef AUTOK_DEBUG
        printf("ZQ: REG32(0xb8000118)=0x%08x\n",REG32(0xb8000118));
        printf("ZQ: REG32(0xb800011c)=0x%08x\n",REG32(0xb800011c));
        printf("ZQ: REG32(0xb8000120)=0x%08x\n",REG32(0xb8000120));
        printf("ZQ: REG32(0xb8000124)=0x%08x\n",REG32(0xb8000124));
        printf("ZQ: REG32(0xb800012C)=0x%08x\n",REG32(0xb800012C));
        printf("ZQ: REG32(0xb8000140)=0x%08x\n",REG32(0xb8000140));
#endif

		/*force address and control signals output driving*/
        if(!PRODUZIONE){
            memctlc_AddrCtrl_output_driving();
        }

		/* Memory delay line config */
		memctlc_config_delay_line(GET_MEM_MHZ());
        
		/*DLL setup*/
		memctlc_dll_setup();

		/* DQ delay tap selection */
		ret = DDR_Calibration(1);
		if (ret != 0) break;

		/* reset dram phy */
		memctlc_dram_phy_reset();

		/* dram size detection */
        dram_size = memctlc_config_DRAM_size();

		/* set DTR accroding to dram size */
		memctlc_config_DTR(0, dram_size);

        /* Disable Read after Write */
        MCERAWCR0rv = 0x11111100;
        MCERAWCR1rv = 0x11;
        DMCRrv = DMCRrv;
        while(RFLD_DMCR(dtr_up_busy_mrs_busy));
        printf("AK: Disable read after write function\n\r");


#ifdef AUTOK_DEBUG
		/* print regs settings after autok*/
		printf("AK: Regs Settings:\n");
		printf("    0xb8001000: 0x%08x\n", REG32(0xb8001000));
		printf("    0xb8001004: 0x%08x\n", REG32(0xb8001004));
		printf("    0xb8001008: 0x%08x\n", REG32(0xb8001008));
		printf("    0xb800100c: 0x%08x\n", REG32(0xb800100c));
		printf("    0xb8001010: 0x%08x\n", REG32(0xb8001010));
		printf("    0xb800021c: 0x%08x\n", REG32(0xb800021c));
		printf("    0xb8001090: 0x%08x\n", REG32(0xb8001090));
		printf("    0xb8001094: 0x%08x\n", REG32(0xb8001094));
		printf("    0xb8001590: 0x%08x\n", REG32(0xb8001590));
		printf("    0xb8001050: 0x%08x\n", REG32(0xb8001050));
		printf("    0xb8001500: 0x%08x\n", REG32(0xb8001500));
		printf("    0xb8000208: 0x%08x\n", REG32(0xb8000208));
		printf("    0xb800020c: 0x%08x\n", REG32(0xb800020c));
		printf("    0xb8000210: 0x%08x\n", REG32(0xb8000210));
		printf("    0xb80015b0: 0x%08x\n", REG32(0xb80015b0));
		printf("    0xb80015b4: 0x%08x\n", REG32(0xb80015b4));
		printf("    0xb80015b8: 0x%08x\n", REG32(0xb80015b8));
		printf("    0xb80015bc: 0x%08x\n", REG32(0xb80015bc));

		volatile unsigned int * ptr = (volatile unsigned int *) 0xb8001510;
        int j;
		for(j=0;j < 32; j++) {
			printf("    0x%08x: 0x%08x\n",(ptr+j), *(ptr+j));
		}
#endif
		printf("AK: dram auto calibrtaion is done\n\n");
		return ret;
	} while(0);

	printf("AK: dram auto calibrtaion failed!!!\n\n");
	return ret;
}
