#include <dram/autok/dram_autok.h>

#define MCR_DRAMTYPE_MASK (0xF0000000)
#define MCR_DRAMTYPE_DDR  (0x00000000)
#define MCR_DRAMTYPE_DDR2 (0x10000000)
#define MCR_DRAMTYPE_DDR3 (0x20000000)
#define DTR2_RFC_FD_S     (20)

#define MEMPLL31_0					(0xb8000234)
#define MEMPLL63_32					(MEMPLL31_0 + 0x4)
#define MEMPLL95_64					(MEMPLL31_0 + 0x8)
#define MEMPLL127_96				(MEMPLL31_0 + 0xc)
#define MEMPLL159_128				(MEMPLL31_0 + 0x10)
#define MEMPLL191_160				(MEMPLL31_0 + 0x14)
#define MEMPLL223_192				(MEMPLL31_0 + 0x18)

#define MEMPLL_CLK_OE	(0x3F)
#define MEMPLL_CLK_EN	(0x03F<<6)

#define DDCR			(0xB8001050)
#define DDR_DELAY_CTRL_REG0			(0xb80015b0)
#define DDR_DELAY_CTRL_REG1			(0xb80015b4)
#define DDR_DELAY_CTRL_REG2			(0xb80015b8)
#define DDR_DELAY_CTRL_REG3			(0xb80015bc)
#define DDR_DELAY_CTRL_REG4			(0xb8001590)

extern void _memctl_update_phy_param(void);
extern void memctlc_dram_phy_reset(void);


static unsigned int tRFC_Spec_DDR2[] SECTION_RO = {
	/* 128Mbit, 256Mbit, 512Mbit,1Gbit, 2Gbit, 4Gbit	*/
	75, 75,105,128,198,328
};

static unsigned int tRFC_Spec_DDR3[] SECTION_RO = {
	/*  512Mbit,1Gbit, 2Gbit, 4Gbit, 8Gbit	*/
	//90, 110, 160, 260, 350
	98, 118, 168, 268, 358
};

unsigned int _dram_DCR_setting[6][5] =
{ 	{	0x10110000/* 8MB_DDR1_08b */, 
		0x10120000/* 16MB_DDR1_08b */, 
		0x10220000/* 32MB_DDR1_08b */, 
		0x10230000/* 64MB_DDR1_08b */, 
		0x10330000/* 128MB_DDR1_08b */	
	},		
	{	0x11100000/* 8MB_DDR1_16b */, 
		0x11110000/* 16MB_DDR1_16b */, 
		0x11210000/* 32MB_DDR1_16b */, 
		0x11220000/* 64MB_DDR1_16b */, 
		0x11320000/* 128MB_DDR1_16b */
	},	
	{	0x10120000/* 16MB_DDR2_08b */, 
		0x10220000/* 32MB_DDR2_08b */, 
		0x10320000/* 64MB_DDR2_08b */, 
		0x20320000/* 128MB_DDR2_08b */, 
		0x20420000/* 256MB_DDR2_08b */	
	},		
	{	0x11110000/* 16MB_DDR2_16b */, 
		0x11210000/* 32MB_DDR2_16b */, 
		0x11220000/* 64MB_DDR2_16b */, 
		0x21220000/* 128MB_DDR2_16b */, 
		0x21320000/* 256MB_DDR2_16b */
	},	 	
	{	0x00000000/* 16MB_DDR3_08b */, 
		0x00000000/* 32MB_DDR3_08b */, 
		0x20220000/* 64MB_DDR3_08b */, 
		0x20320000/* 128MB_DDR3_08b */, 
		0x20420000/* 256MB_DDR3_08b */
	},		
	{	0x00000000/* 16MB_DDR3_16b */, 
		0x00000000/* 32MB_DDR3_16b */, 
		0x21120000/* 64MB_DDR3_16b */, 
		0x21220000/* 128MB_DDR3_16b */, 
		0x21320000/* 256MB_DDR3_16b */}
};

unsigned int _dram_type_setting[6][5] =
{ 	{	0x008004B2/* 8MB_DDR1_08b */, 
		0x010004B2/* 16MB_DDR1_08b */, 
		0x02000692/* 32MB_DDR1_08b */, 
		0x04000802/* 64MB_DDR1_08b */, 
		0x08000C62/* 128MB_DDR1_08b */
	},		
	{	0x008084B2/* 8MB_DDR1_16b */, 
		0x010084B2/* 16MB_DDR1_16b */, 
		0x02008692/* 32MB_DDR1_16b */, 
		0x04008802/* 64MB_DDR1_16b */, 
		0x08008C62/* 128MB_DDR1_16b */
	},	
	{	0x010004B2/* 16MB_DDR2_08b */, 
		0x020004B2/* 32MB_DDR2_08b */, 
		0x04000692/* 64MB_DDR2_08b */, 
		0x08000802/* 128MB_DDR2_08b */, 
		0x10000C62/* 256MB_DDR2_08b */
	},		
	{	0x010084B2/* 16MB_DDR2_16b */, 
		0x020084B2/* 32MB_DDR2_16b */, 
		0x04008692/* 64MB_DDR2_16b */, 
		0x08008802/* 128MB_DDR2_16b */, 
		0x10008C62/* 256MB_DDR2_16b */
	},		
	{	0x010004B3/* 16MB_DDR3_08b */, 
		0x020005A3/* 32MB_DDR3_08b */, 
		0x040005A3/* 64MB_DDR3_08b */, 
		0x080006E3/* 128MB_DDR3_08b */, 
		0x10000A03/* 256MB_DDR3_08b */
	},		
	{	0x010084B3/* 16MB_DDR3_16b */, 
		0x020085A3/* 32MB_DDR3_16b */, 
		0x040085A3/* 64MB_DDR3_16b */, 
		0x080086E3/* 128MB_DDR3_16b */, 
		0x10008A03/* 256MB_DDR3_16b */}	
};

unsigned int _dram_detection_addr[6][5] =
{ 	{	0xA63809A4/* 8MB_DDR1_08b */, 
		0xA6380BA4/* 16MB_DDR1_08b */, 
		0xA6780BA4/* 32MB_DDR1_08b */, 
		0xA0000000/* 64MB_DDR1_08b */, 
		0xA0000000/* 128MB_DDR1_08b */
	},		
	{	0xA6701148/* 8MB_DDR1_16b */, 
		0xA6701348/* 16MB_DDR1_16b */, 
		0xA6F01348/* 32MB_DDR1_16b */, 
		0xA6F01748/* 64MB_DDR1_16b */, 
		0xA7F01748/* 128MB_DDR1_16b */
	},		
	{	0xA6380BA4/* 16MB_DDR2_08b */, 
		0xA6780BA4/* 32MB_DDR2_08b */, 
		0xA6F80BA4/* 64MB_DDR2_08b */, 
		0xAEF80BA4/* 128MB_DDR2_08b */, 
		0xAFF80BA4/* 256MB_DDR2_08b */
	},		
	{	0xA6701348/* 16MB_DDR2_16b */, 
		0xA6F01348/* 32MB_DDR2_16b */, 
		0xA6F01748/* 64MB_DDR2_16b */, 
		0xAEF01748/* 128MB_DDR2_16b */, 
		0xAFF01748/* 256MB_DDR2_16b */
	},		
	{	0xA0000000/* 16MB_DDR3_08b */, 
		0xA0000000/* 32MB_DDR3_08b */, 
		0xAE780BA4/* 64MB_DDR3_08b */, 
		0xAEF80BA4/* 128MB_DDR3_08b */, 
		0xAFF80BA4/* 256MB_DDR3_08b */
	},		
	{	0xA0000000/* 16MB_DDR3_16b */, 
		0xA0000000/* 32MB_DDR3_16b */, 
		0xAE701548/* 64MB_DDR3_16b */, 
		0xAEF01548/* 128MB_DDR3_16b */, 
		0xAFF01548/* 256MB_DDR3_16b */}
};
enum DDR1_FREQ_SEL {
	DDR1_250=0,
	DDR1_200,
	DDR1_150
};

enum DDR2_FREQ_SEL {
	DDR2_600 = 0,
	DDR2_500,
	DDR2_450,		
	DDR2_400,
	DDR2_350,
	DDR2_300,
	DDR2_200
};

enum DDR3_FREQ_SEL {
	DDR3_600 = 0,
	DDR3_500,
	DDR3_450,		
	DDR3_400,
	DDR3_350,
	DDR3_300,
	DDR3_200
};
unsigned int DDR1_DTR[3][3] SECTION_PARAMETERS ={
	{	/* RTL8685, DDR1, 250MHz*/
		0x23021720,
		0x0303020B,	
		0x00008000},		//0x01208000
	{	/* RTL8685, DDR1, 200MHz*/
		0x22021520,
		0x0202010B,	
		0x00007000},		//0x00D07000
	{	/* RTL8685, DDR1, 150MHz*/
		0x22021320,
		0x0202010B,	
		0x00006000},		//0x00A06000
};
unsigned int DDR2_DTR[7][3] SECTION_PARAMETERS ={
	{	/* DDR2, 600MHz, CAS=7, WR=7 */
		0x66533e26,
		0x06060312,	
		0x0001B000},
	{	/* DDR2, 500MHz, CAS=7, WR=7 */
		0x66533e26,
		0x06060312,
		0x00016000},
	{	/* DDR2, 450MHz, CAS=6, WR=6*/
		0x55433535,
		0x05050311,	
		0x00014000},		
	{	/* DDR2, 400MHz, CAS=6, WR=6*/
		0x55433535,
		0x05050311,	
		0x00012000},
	{	/* DDR2, 350MHz, CAS=5, WR=5*/
		0x44322824,
		0x0404020b,	
		0x00010000},
	{	/* DDR2, 300MHz, CAS=5, WR=5*/
		0x44322824,
		0x0404020B,	
		0x0000E000},
	{	/* DDR2, 200MHz, CAS=5, WR=4*/
		0x43311224,
		0x04040108,	
		0x00009000},
};
unsigned int DDR3_DTR[7][3] SECTION_PARAMETERS ={
	{	/* DDR3, 600MHz, CAS=7, WR=10, tREFI<1.92u*/
                0x88644838,
                0x0808051F, 
                0x00016000},
	{	/* DDR3, 500MHz, CAS=7, WR=8, tREFI<1.92u*/
                0x78555626,
                0x0707051E,
        0x00014000},
	{	/* DDR3, 450MHz, CAS=6, WR=7, tREFI<1.92u*/
		0x56433625,
		0x05050416,	
		0x00011000},		
	{	/* DDR3, 400MHz, CAS=6, WR=7, tREFI<1.92u*/
		0x56433525,
		0x05050414,	
		0x0000F000},
	{	/* DDR3, 350MHz, CAS=6, WR=6, tREFI<1.92u*/
		0x55433425,
		0x04040311,	
		0x0000D000},
	{	/* DDR3, 300MHz, CAS=5, WR=5, tREFI<1.92u*/
		0x44433324,
		0x0404030F,	
		0x0000B000},
	{	/* DDR3, 200MHz, CAS=5, WR=5, tREFI<1.92u */
		0x44433224,
		0x0404030B,
		0x00008000},
};

SECTION_AUTOK
unsigned int memctlc_DDR_Type(void)
{
	if(MCR_DRAMTYPE_DDR3 == (REG32(MCR) & MCR_DRAMTYPE_MASK))
		return 3;
	else if(MCR_DRAMTYPE_DDR2 == (REG32(MCR) & MCR_DRAMTYPE_MASK))
		return 2;
	else if(MCR_DRAMTYPE_DDR == (REG32(MCR) & MCR_DRAMTYPE_MASK))
		return 1;
	else
		return 0;
}

SECTION_AUTOK
void memctlc_config_DTR(unsigned int default_instruction, unsigned int dram_size)
{
	unsigned int dram_freq_mhz,dram_size_t;
	unsigned int dram_base_size=0, dtr2_temp = 0;
	unsigned int DRAM_capacity_index=0;
	unsigned int *tRFC = VZERO;
	unsigned char tRFC_extend=0;

    DCR_T dcr = {.v = DCRrv};
	volatile unsigned int *dtr0 = (volatile unsigned int *)DTR0;
	volatile unsigned int *dtr1 = (volatile unsigned int *)DTR1;
	volatile unsigned int *dtr2 = (volatile unsigned int *)DTR2;

#ifdef AUTOK_DEBUG
	printf("AK: %s, option: %d\n", __func__, default_instruction);
	printf("    DTR0:0x%x, DTR1:0x%x, DTR2:0x%x\n",*dtr0,*dtr1,*dtr2);
#endif

	dram_freq_mhz = GET_MEM_MHZ();
    
	if(default_instruction == 1){
        dcr.v = 0x21320000;
        dcr.f.prl_bank_act_en = PROJ_PARA_PARALLEL_BANK_EN;
        DCRrv = dcr.v; // 2Gbits, both DDR2 and DDR3
		if(memctlc_DDR_Type() == IS_DDR2_SDRAM)
			DRAM_capacity_index = 5;
		else if (memctlc_DDR_Type() == IS_DDR3_SDRAM)
			DRAM_capacity_index = 4;
		else
			DRAM_capacity_index = 4;
	}else{
		/* Provide the minmun dram size as base */
		if(memctlc_DDR_Type()==IS_DDR2_SDRAM){
			dram_base_size = 0x1000000;		//min capacite 16Mbytes
			tRFC_extend = 3;
		}else if (memctlc_DDR_Type()==IS_DDR3_SDRAM){
			dram_base_size = 0x4000000;		//min capacite 64Mbytes
			tRFC_extend = 10;
		}else{		//DDR1
			dram_base_size = 0x800000;		//8Mbytes
			tRFC_extend = 1;
			}
		dram_size_t=dram_size&0xFFFF0000;
#ifdef AUTOK_DEBUG
		printf("AK: %s-%d dram_size=0x%x\n",__func__,__LINE__,dram_size_t);
#endif
		for(DRAM_capacity_index=0; DRAM_capacity_index<6; DRAM_capacity_index++){
			if(dram_size_t == ((dram_base_size) << DRAM_capacity_index)){
				break;
			}
		}
	}

    u32_t DRAM_freq_index;
	if(memctlc_DDR_Type()==IS_DDR2_SDRAM){
		tRFC = &tRFC_Spec_DDR2[0];
        if(dram_freq_mhz >= 550){
			DRAM_freq_index = DDR2_600;
        }else if(dram_freq_mhz >= 450){
			DRAM_freq_index = DDR2_500;
        }else if(dram_freq_mhz >= 350){
			DRAM_freq_index = DDR2_400;
        }else if(dram_freq_mhz >= 250){
			DRAM_freq_index = DDR2_300;
        }else{
			DRAM_freq_index = DDR2_200;
		}

		*dtr0 = DDR2_DTR[DRAM_freq_index][0];
		*dtr1 = DDR2_DTR[DRAM_freq_index][1];
		dtr2_temp = DDR2_DTR[DRAM_freq_index][2];
	}else if(memctlc_DDR_Type()==IS_DDR3_SDRAM){
		tRFC = &tRFC_Spec_DDR3[0];	
        if(dram_freq_mhz >= 550){
			DRAM_freq_index = DDR3_600;
        }else if(dram_freq_mhz >= 450){
			DRAM_freq_index = DDR3_500;
        }else if(dram_freq_mhz >= 350){
			DRAM_freq_index = DDR3_400;
        }else if(dram_freq_mhz >= 250){
			DRAM_freq_index = DDR3_300;
        }else{
			DRAM_freq_index = DDR3_200;
		}

		*dtr0 = DDR3_DTR[DRAM_freq_index][0];
		*dtr1 = DDR3_DTR[DRAM_freq_index][1];
		dtr2_temp = DDR3_DTR[DRAM_freq_index][2];
	}
#ifdef AUTOK_DEBUG
	printf("AK: tRFC[%d]=%d\n",DRAM_capacity_index,tRFC[DRAM_capacity_index]);
	printf("AK: DTR2_RFC_FD_S=%d\n",DTR2_RFC_FD_S);
	printf("AK: dram_freq_mhz=%d\n",dram_freq_mhz);
	printf("    ====>%d %d\n",(((tRFC[DRAM_capacity_index]*dram_freq_mhz)/1000)+3),(tRFC[DRAM_capacity_index]*dram_freq_mhz));
	printf("    0x%x\n",((((tRFC[DRAM_capacity_index]*dram_freq_mhz)/1000)+3)<<DTR2_RFC_FD_S));
#endif
	*dtr2=dtr2_temp | 
		((((tRFC[DRAM_capacity_index]*dram_freq_mhz)/1000)+tRFC_extend)<<DTR2_RFC_FD_S);

#ifdef AUTOK_DEBUG
	printf("AK: Setting DTR dtr0=0x%x dtr1=0x%x dtr2=0x%x\n",*dtr0,*dtr1,*dtr2);
#endif

	return;
}

unsigned int memctlc_config_DRAM_size(void)
{
    DCR_T dcr;
	unsigned int dcr_value=0, dram_size=0x2000000;
	volatile unsigned int *dram_addr;
	unsigned int i;
	unsigned int DDR_para_index=0, DDR_width=16, loc=0;

	if(memctlc_DDR_Type()==IS_DDR3_SDRAM)
		DDR_para_index = 4;
	else if(memctlc_DDR_Type()==IS_DDR2_SDRAM)
		DDR_para_index = 2;
	else	//DDR1
		DDR_para_index = 0;
	
	DDR_width =  8 << ((REG32(DCR) & DCR_DBUSWID_MASK) >> DCR_DBUSWID_FD_S) ;

	loc=(DDR_width/8-1) + DDR_para_index;

	_memctl_update_phy_param();

	dram_addr = (volatile unsigned int *)_dram_detection_addr[loc][4];
	*dram_addr = 0x5A0FF0A5;

	/* DCache flush is necessary ? */
	//_memctl_DCache_flush_invalidate();

	/* Assign 64MBytes DRAM parameters as default value */
	dcr_value = _dram_DCR_setting[loc][2];
	dram_size = _dram_type_setting[loc][2];

	for(i=(sizeof(_dram_detection_addr[loc])/sizeof(unsigned int)); i>0; i--){
		if( REG32(_dram_detection_addr[loc][i-1]) != 0x5A0FF0A5 ){
			dcr_value = _dram_DCR_setting[loc][i];
			dram_size = ((_dram_type_setting[loc][i]) & 0xFFFF0000);
			break;
		}
	}

    //NOTE: Need to check the setting of "Parallel Bank"
    dcr.v = dcr_value;
    dcr.f.prl_bank_act_en = PROJ_PARA_PARALLEL_BANK_EN;
    DCRrv = dcr.v;
	_memctl_update_phy_param();

	return dram_size;
}

SECTION_AUTOK
static unsigned int get_memory_ddr2_dram_odt_parameters(unsigned int *para_array)
{
	para_array[0] = (unsigned int)PROJ_PARA_DDR2_DRAM_ODT_VALUE;
	return 1; /*fail */
}

SECTION_AUTOK
static unsigned int get_memory_ddr3_dram_rtt_nom_parameters(unsigned int *para_array)

{
	para_array[0] = (unsigned int)PROJ_PARA_DDR3_DRAM_RTT_NOM_VALUE;
	return 1; /*fail */
}

SECTION_AUTOK
static unsigned int get_memory_ddr3_dram_rtt_wr_parameters(unsigned int *para_array)
{
	para_array[0] = (unsigned int)PROJ_PARA_DDR3_DRAM_RTT_WR_VALUE;
	return 1; /*fail */
}

SECTION_AUTOK
static unsigned int get_memory_dram_reduce_drv_parameters(unsigned int *para_array)
{
	if(PROJ_PARA_PREFERED_DRAM_DRIV_STRENGTH){
		para_array[0] = 0; /*full*/
	}else{
		para_array[0] = 1;/*reduce*/
	}
	return 1;
}

SECTION_AUTOK
static void _DTR_DDR1_MRS_setting(unsigned int *mr)
{
	unsigned int cas, buswidth;
	/* Default value of Mode registers */
	mr[0] = DMCR_MRS_MODE_MR | DDR1_MR_BURST_SEQ | DDR1_MR_OP_NOR |\
		DMCR_MR_MODE_EN ;

	mr[1] = DMCR_MRS_MODE_EMR1 | DDR1_EMR1_DLL_EN | DDR1_EMR1_DRV_NOR |\
		DMCR_MR_MODE_EN;

	/* Extract CAS and WR in DTR0 */
	cas = (REG32(DTR0) & DTR0_CAS_MASK) >> DTR0_CAS_FD_S;
	buswidth = (REG32(DCR) & DCR_DBUSWID_MASK) >> DCR_DBUSWID_FD_S;
	switch (cas){
		case 0:
			mr[0] = mr[0] | DDR1_MR_CAS_25;
			break;
		case 1:
			mr[0] = mr[0] | DDR1_MR_CAS_2;
			break;
		case 2:
			mr[0] = mr[0] | DDR1_MR_CAS_3;
			break;
		default:
			mr[0] = mr[0] | DDR1_MR_CAS_3;
			break;

	}

	switch (buswidth){
		case 0:
			mr[0] = mr[0] | DDR1_MR_BURST_4;
			break;
		case 1:
			mr[0] = mr[0] | DDR1_MR_BURST_2;
			break;
		default:
			mr[0] = mr[0] | DDR1_MR_BURST_2;
			break;
	}

	return;
}

SECTION_AUTOK
static void _DTR_DDR2_MRS_setting(unsigned int *mr)
{
	unsigned int cas, wr, odt_value, drv_str;
	/* Default value of Mode registers */
	mr[0] = DMCR_MRS_MODE_MR | DDR2_MR_BURST_4 | DDR2_MR_BURST_SEQ | \
		DDR2_MR_TM_NOR | DDR2_MR_DLL_RESET_NO | DDR2_MR_PD_FAST |\
		DMCR_MR_MODE_EN ;

	if(get_memory_ddr2_dram_odt_parameters(&odt_value)){
		switch (odt_value){
			case 0:
				odt_value = DDR2_EMR1_RTT_DIS;
				break;
			case 75:
				odt_value = DDR2_EMR1_RTT_75;
				break;
			case 150:
				odt_value = DDR2_EMR1_RTT_150;
				break;
			default: /* 50 */
				odt_value = DDR2_EMR1_RTT_50;
				break;
		}
	}else{
		odt_value = DDR2_EMR1_RTT_75;
	}

	if(get_memory_dram_reduce_drv_parameters(&drv_str)){
		if(drv_str){/* reduce */
			drv_str = DDR2_EMR1_DIC_REDUCE;
		}else{
			drv_str = DDR2_EMR1_DIC_FULL;
		}
	}else{ /* full mode */
		drv_str = DDR2_EMR1_DIC_FULL;
	}

	drv_str = drv_str << 1;
	
	mr[1] = DDR2_EMR1_DLL_EN | drv_str |\
		odt_value | DDR2_EMR1_ADD_0 | DDR2_EMR1_OCD_EX | \
		DDR2_EMR1_QOFF_EN | DDR2_EMR1_NDQS_EN | DDR2_EMR1_RDQS_DIS |\
		DMCR_MR_MODE_EN | DMCR_MRS_MODE_EMR1;

	mr[2] = DDR2_EMR2_HTREF_DIS | DDR2_EMR2_DCC_DIS | DDR2_EMR2_PASELF_FULL |\
		DMCR_MR_MODE_EN | DMCR_MRS_MODE_EMR2;

	mr[3] = DMCR_MR_MODE_EN | DMCR_MRS_MODE_EMR3;

	/* Extract CAS and WR in DTR0 */
	cas = (REG32(DTR0) & DTR0_T_CAS_PHY_MASK);
	wr = (REG32(DTR0) & DTR0_WR_MASK) >> DTR0_WR_FD_S;

	if( cas == 1)
		mr[0] = mr[0] | DDR2_MR_CAS_2;
	else if(cas == 2)
		mr[0] = mr[0] | DDR2_MR_CAS_3;
	else if(cas == 3)
		mr[0] = mr[0] | DDR2_MR_CAS_4;
	else if(cas == 4)
		mr[0] = mr[0] | DDR2_MR_CAS_5;
	else if(cas == 5)
		mr[0] = mr[0] | DDR2_MR_CAS_6;
	else if(cas ==6)
		mr[0] = mr[0] | DDR2_MR_CAS_7;
	else
		mr[0] = mr[0] | DDR2_MR_CAS_6;

	if( wr == 1)
		mr[0] = mr[0] | DDR2_MR_WR_2;
	else if(wr == 2)
		mr[0] = mr[0] | DDR2_MR_WR_3;
	else if(wr == 3)
		mr[0] = mr[0] | DDR2_MR_WR_4;
	else if(wr == 4)
		mr[0] = mr[0] | DDR2_MR_WR_5;
	else if(wr == 5)
		mr[0] = mr[0] | DDR2_MR_WR_6;
	else if(wr == 6)
		mr[0] = mr[0] | DDR2_MR_WR_7;
	else
		mr[0] = mr[0] | DDR2_MR_WR_7;

	return;
}

SECTION_AUTOK
static void _DTR_DDR3_MRS_setting(unsigned int *sug_dtr, unsigned int *mr)
{
	unsigned int cas, wr, cwl, MRS_tmp;
	unsigned int rtt_nom_value, rtt_wr_value, dram_reduce_drv;

	/* Default value of Mode registers */
	mr[0] = DMCR_MRS_MODE_MR | DDR3_MR_BURST_8 | DDR3_MR_READ_BURST_NIBBLE | \
		DDR3_MR_TM_NOR | DDR3_MR_DLL_RESET_NO | DDR3_MR_PD_FAST |\
		DMCR_MR_MODE_EN ;

	mr[1] = DDR3_EMR1_DLL_EN | DDR3_EMR1_DIC_RZQ_DIV_6 |\
		DDR3_EMR1_RTT_NOM_DIS | DDR3_EMR1_ADD_0 | DDR3_EMR1_WRITE_LEVEL_DIS | \
		DDR3_EMR1_TDQS_DIS | DDR3_EMR1_QOFF_EN |\
		DMCR_MR_MODE_EN | DMCR_MRS_MODE_EMR1;
	mr[2] = DDR3_EMR2_PASR_FULL | DDR3_EMR2_ASR_DIS | DDR3_EMR2_SRT_NOR |\
		DDR3_EMR2_RTT_WR_DIS | DMCR_MR_MODE_EN | DMCR_MRS_MODE_EMR2;

	mr[3] = DDR3_EMR3_MPR_OP_NOR | DDR3_EMR3_MPR_LOC_PRE_PAT |\
		DMCR_MR_MODE_EN | DMCR_MRS_MODE_EMR3;


	if(get_memory_ddr3_dram_rtt_nom_parameters(&rtt_nom_value)){
		if(rtt_nom_value != 0){
			MRS_tmp=(240/rtt_nom_value)/2;
			if(MRS_tmp==1)			/* div 2 */
				rtt_nom_value = DDR3_EMR1_RTT_NOM_RZQ_DIV2;
			else if(MRS_tmp==2)		/* div 4*/
				rtt_nom_value = DDR3_EMR1_RTT_NOM_RZQ_DIV4;
			else if(MRS_tmp==3)		/* div 6 */
				rtt_nom_value = DDR3_EMR1_RTT_NOM_RZQ_DIV6;
			else if(MRS_tmp==4)		/* div 8 */
				rtt_nom_value = DDR3_EMR1_RTT_NOM_RZQ_DIV8;
			else if(MRS_tmp==6)		/* div 12 */
				rtt_nom_value = DDR3_EMR1_RTT_NOM_RZQ_DIV12;
			else 						/* 40 */
				rtt_nom_value = DDR3_EMR1_RTT_NOM_RZQ_DIV2;

		}else{
			rtt_nom_value = DDR3_EMR1_RTT_NOM_DIS;
		}
	}else{
		rtt_nom_value = DDR3_EMR1_RTT_NOM_DIS;
	}

	if(get_memory_ddr3_dram_rtt_wr_parameters(&rtt_wr_value)){
		if(rtt_wr_value != 0){
			MRS_tmp=(240/rtt_wr_value)/2;
			if(MRS_tmp==2)		/* div 4 */
				rtt_wr_value = DDR3_EMR2_RTT_WR_RZQ_DIV_4;
			else					/* div 2 */
				rtt_wr_value = DDR3_EMR2_RTT_WR_RZQ_DIV_2;
		}else{
			rtt_wr_value = DDR3_EMR2_RTT_WR_DIS;
		}
	}else{
		rtt_wr_value = DDR3_EMR2_RTT_WR_RZQ_DIV_2;
	}


	if(get_memory_dram_reduce_drv_parameters(&dram_reduce_drv)){
		if(dram_reduce_drv){
			dram_reduce_drv = DDR3_EMR1_DIC_RZQ_DIV_6;
		}else{
			dram_reduce_drv = DDR3_EMR1_DIC_RZQ_DIV_7;
		}
	}else{
		dram_reduce_drv = DDR3_EMR1_DIC_RZQ_DIV_6;
	}

	mr[1] = mr[1] | rtt_nom_value | dram_reduce_drv;
	mr[2] = mr[2] | rtt_wr_value ;



	/* Extract CAS and WR in DTR0 */
	cas = (sug_dtr[0] & DTR0_T_CAS_PHY_MASK);	
	wr = (sug_dtr[0] & DTR0_WR_MASK) >> DTR0_WR_FD_S;
	cwl = (sug_dtr[0] & DTR0_CWL_MASK) >> DTR0_CWL_FD_S;

	if(cas==4)
		mr[0] = mr[0] | DDR3_MR_CAS_5;
	else if(cas==5)
		mr[0] = mr[0] | DDR3_MR_CAS_6;
	else if(cas==6)
		mr[0] = mr[0] | DDR3_MR_CAS_7;
	else if(cas==7)
		mr[0] = mr[0] | DDR3_MR_CAS_8;
	else if(cas==8)
		mr[0] = mr[0] | DDR3_MR_CAS_9;
	else if(cas==9)
		mr[0] = mr[0] | DDR3_MR_CAS_10;
	else if(cas==10)
		mr[0] = mr[0] | DDR3_MR_CAS_11;
	else
		mr[0] = mr[0] | DDR3_MR_CAS_6;

	if(wr==4)
		mr[0] = mr[0] | DDR3_MR_WR_5;
	else if(wr==5)
		mr[0] = mr[0] | DDR3_MR_WR_6;
	else if(wr==6)
		mr[0] = mr[0] | DDR3_MR_WR_7;
	else if(wr==7)
		mr[0] = mr[0] | DDR3_MR_WR_8;
	else if(wr==8)
		mr[0] = mr[0] | DDR3_MR_WR_9;
	else if(wr==9)
		mr[0] = mr[0] | DDR3_MR_WR_10;
	else if(wr==11)
		mr[0] = mr[0] | DDR3_MR_WR_12;
	else
		mr[0] = mr[0] | DDR3_MR_WR_6;

	if(cwl==4)
		mr[2] = mr[2] | DDR3_EMR2_CWL_5;
	else if(cwl==5)
		mr[2] = mr[2] | DDR3_EMR2_CWL_6;
	else if(cwl==6)
		mr[2] = mr[2] | DDR3_EMR2_CWL_7;
	else if(cwl==7)
		mr[2] = mr[2] | DDR3_EMR2_CWL_8;
	else
		mr[2] = mr[2] | DDR3_EMR2_CWL_6;

#ifdef AUTOK_DEBUG
    printf("AK: DDR3 MRS[0] = 0x%08x\n",mr[0]);
    printf("AK: DDR3 MRS[1] = 0x%08x\n",mr[1]);
    printf("AK: DDR3 MRS[2] = 0x%08x\n",mr[2]);
    printf("AK: DDR3 MRS[3] = 0x%08x\n",mr[3]);
#endif
	return;
}

SECTION_AUTOK
static void memctlc_ddr1_dll_reset(void)
{
	volatile unsigned int *dmcr, *dtr0 __attribute__((unused));
	volatile unsigned int delay_time;
	unsigned int mr[4];

	dmcr = (volatile unsigned int *)DMCR;
	dtr0 = (volatile unsigned int *)DTR0;

	_DTR_DDR1_MRS_setting(mr);

	/* 1. Disable DLL */
	*dmcr = mr[1] | DDR1_EMR1_DLL_DIS;
	while(*dmcr & DMCR_MRS_BUSY);

	/* 2. Enable DLL */
	*dmcr = mr[1] & (~DDR1_EMR1_DLL_DIS);
	while(*dmcr & DMCR_MRS_BUSY);

	/* 3. Reset DLL */
	*dmcr = mr[0] | DDR1_MR_OP_RST_DLL ;
	while(*dmcr & DMCR_MRS_BUSY);

	/* 4. Waiting 200 clock cycles */
	delay_time = 0x800;
	while(delay_time--);

	/* 5. Normal mode, avoid to reset DLL when updating phy params */
	*dmcr = mr[0];
	while(*dmcr & DMCR_MRS_BUSY);

	/* 6. reset phy fifo */
	memctlc_dram_phy_reset();

	return;
}

#define CHK_DMCR_RDY() do { \
		loop_lim = 0; \
		udelay(1); \
		while (*dmcr & DMCR_MRS_BUSY) { \
			if (loop_lim++ > 100) { \
				puts("EE: DMCR does not respond during MRS. Reset...\n"); \
				SYSTEM_RESET(); \
			} \
		} \
	} while(0)
SECTION_AUTOK
static void memctlc_ddr2_dll_reset(void)
{
	volatile unsigned int *dmcr, *dtr0 __attribute__((unused));
	volatile unsigned int delay_time;
	unsigned int mr[4];
	int i;
	volatile unsigned int loop_lim = 0;

	dmcr = (volatile unsigned int *)DMCR;
	dtr0 = (volatile unsigned int *)DTR0;

	_DTR_DDR2_MRS_setting(mr);

	/* 1. Disable DLL */
	*dmcr = mr[1] | DDR2_EMR1_DLL_DIS;
	/* while(*dmcr & DMCR_MRS_BUSY); */
	CHK_DMCR_RDY();

	/* 2. Enable DLL */
	*dmcr = mr[1] & (~DDR2_EMR1_DLL_DIS);
	/* while(*dmcr & DMCR_MRS_BUSY); */
	CHK_DMCR_RDY();

	/* 3. Reset DLL */
	*dmcr = mr[0] | DDR2_MR_DLL_RESET_YES ;
	/* while(*dmcr & DMCR_MRS_BUSY); */
	CHK_DMCR_RDY();

	/* 4. Waiting 200 clock cycles */
	delay_time = 0x800;
	while(delay_time--);

	/* 5. Set EMR2 */
	*dmcr = mr[2];
	/* while(*dmcr & DMCR_MRS_BUSY); */
	CHK_DMCR_RDY();

	/* 6. reset phy fifo */
	memctlc_dram_phy_reset();

	/*modify mr  values accroding to the opeartions that we've done previously*/
	mr[1] |= DDR2_EMR1_DLL_DIS;
	mr[1] &= (~DDR2_EMR1_DLL_DIS);
	mr[0] |= DDR2_MR_DLL_RESET_YES;

	for(i = 0;i < 4; i++){
		printf("AK: MR%d: 0x%08x\n", i, mr[i]);
	}
	return;
}

SECTION_AUTOK
static void memctlc_ddr3_dll_reset(void)
{
	volatile unsigned int *dmcr, *dtr0;
	volatile unsigned int delay_time;
	unsigned int dtr[3], mr[4];
	volatile unsigned int loop_lim = 0;

	dmcr = (volatile unsigned int *)DMCR;
	dtr0 = (volatile unsigned int *)DTR0;

	dtr[0]= *dtr0;
	dtr[1]= *(dtr0 + 1);
	dtr[2]= *(dtr0 + 2);

	_DTR_DDR3_MRS_setting(dtr, mr);

	/* 1. Disable DLL */
	*dmcr = mr[1] | DDR3_EMR1_DLL_DIS;
	/* while(*dmcr & DMCR_MRS_BUSY); */
	CHK_DMCR_RDY();

	/* 2. Enable DLL */
	*dmcr = mr[1] & (~DDR3_EMR1_DLL_DIS);
	/* while(*dmcr & DMCR_MRS_BUSY); */
	CHK_DMCR_RDY();

	/* 3. Reset DLL */
	*dmcr = mr[0] | DDR3_MR_DLL_RESET_YES ;
	/* while(*dmcr & DMCR_MRS_BUSY); */
	CHK_DMCR_RDY();

	/* 4. Waiting 200 clock cycles */
	delay_time = 0x800;
	while(delay_time--);

	/* 5. Set EMR2 */
	*dmcr = mr[2];
	/* while(*dmcr & DMCR_MRS_BUSY); */
	CHK_DMCR_RDY();

	/* 6. Set EMR3 */
	*dmcr = mr[3];
	/* while(*dmcr & DMCR_MRS_BUSY); */
	CHK_DMCR_RDY();

	/* 7. reset phy fifo */
	memctlc_dram_phy_reset();

	return;
}

SECTION_AUTOK
static void plat_memctl_ZQ_force_config(void)
{
	    unsigned int zq_force_value0, zq_force_value1, zq_force_value2;
	    volatile unsigned int *zq_pad_ctl_reg;
	    zq_force_value0 = 0x0022b49f; /*OCD 60, ODT 50*/
	    zq_force_value1 = 0x00570057;
	    zq_force_value2 = 0x58282809;

	    zq_pad_ctl_reg = (volatile unsigned int *)0xB8000118;

	    *zq_pad_ctl_reg     = zq_force_value0;
	    *(zq_pad_ctl_reg+1) = zq_force_value0;
	    *(zq_pad_ctl_reg+2) = zq_force_value0;
	    *(zq_pad_ctl_reg+3) = zq_force_value0;
	    *(zq_pad_ctl_reg+6) = zq_force_value0;
	    *(zq_pad_ctl_reg+7) = zq_force_value0;
	    *(zq_pad_ctl_reg+8) = zq_force_value0;
	    *(zq_pad_ctl_reg+9) = zq_force_value0;

	    *(zq_pad_ctl_reg+4) = zq_force_value1;
	    *(zq_pad_ctl_reg+5) = zq_force_value1;
	    *(zq_pad_ctl_reg+10)= zq_force_value1;

	    *(zq_pad_ctl_reg+11)= zq_force_value2;

	    return;
}

SECTION_AUTOK
int memctlc_ZQ_calibration(unsigned int auto_cali_value)
{
    if(PROJ_PARA_ZQ_AUTO_CALI){
    	volatile unsigned int *dmcr, *daccr __attribute__((unused)), *zq_cali_reg;
    	volatile unsigned int *zq_cali_status_reg;
    	unsigned int polling_limit, zqc_cnt;
    	unsigned int reg_v, odtn, odtp, ocdn, ocdp;

    	dmcr = (volatile unsigned int *)DMCR;
    	daccr = (volatile unsigned int *)DACCR;
    	zq_cali_reg = (volatile unsigned int *)0xB8001094;
    	zq_cali_status_reg = (volatile unsigned int *)0xB8001098;

    	/* Disable DRAM refresh operation */
    	*dmcr = ((*dmcr | DMCR_DIS_DRAM_REF_MASK) & (~DMCR_MR_MODE_EN_MASK));

    	zqc_cnt = 0;
     zq_cali_start:
    	/* Trigger the calibration */
    	*zq_cali_reg = auto_cali_value | 0x80000000;

    	/* Polling to ready */
    	polling_limit = 0x10000;
    	while(*zq_cali_reg & 0x80000000){
    		polling_limit--;
    		if(polling_limit == 0){
    			printf("AK: %s, %d: Error, ZQ auto-calibration ready polling timeout!\n", __FUNCTION__, __LINE__);
    			plat_memctl_ZQ_force_config(); /*Use static ZQ setting*/
    			goto static_zq_setting_done;
    		}
    	}

    	/* Patch code for IO PAD */
    	/* plat_memctl_IO_PAD_patch();  */
    	reg_v = *zq_cali_status_reg;

    	if(reg_v & 0x20000000) {
    		if ((zqc_cnt++) < MC_ZQC_RETRY_LIM) {
    			udelay(1000);
    			goto zq_cali_start;
    		}
    		odtp = ((reg_v >> 27) & 0x3);
    		odtn = ((reg_v >> 25) & 0x3);
    		ocdp = ((reg_v >> 23) & 0x3);
    		ocdn = ((reg_v >> 21) & 0x3);
    		printf("AK: Result of ODTP/ODTN/OCDP/OCDN=");
    		printf("0x%08X / 0x%08X / 0x%08X / 0x%08X\n", odtp, odtn, ocdp, ocdn);

    		if((ocdp == 1) || /* OCDP must NOT be overflow (may tolerate code underflow error) */
    		   (ocdn == 1))   /* OCDN must NOT be overflow (may tolerate code underflow error) */  {
    			printf("AK: ZQ Calibration Failed\n\r");
    			return MEMCTL_ZQ_CALI_FAIL; /* Error, calibration fail. */
    		} else {
    			printf("AK: ZQ Calibration Relaxed Pass\n\r");
    			goto static_zq_setting_done;
    		}
    	}

    	if (zqc_cnt == 0) {
    		printf("AK: ZQ Calibration Passed\r");
    	} else {
    		printf("AK: ZQ Calibration Retried Passed\n\r");
    	}
        static_zq_setting_done:
        	/* Enable DRAM refresh operation */
        	*dmcr = *dmcr &  (~DMCR_DIS_DRAM_REF_MASK) ;

    	return MEMCTL_ZQ_CALI_PASS;
    }else{
        printf("AK: Warning: No Static ZQ Calibration\n");
        return MEMCTL_ZQ_CALI_FAIL;
    }
}
void memctlc_config_delay_line(unsigned int dram_freq_mhz){
	unsigned char ac_mode=(!PROJ_PARA_USE_ANALOG_DELAY_LINE);
    unsigned char dqs0_grp_dly=0,dqs1_grp_dly=0, dynamic_FIFO_rst=0;
	unsigned char dqs0_hclk=0,dqs1_hclk=0,dqs0_en_tap=0,dqs1_en_tap=0;
	if(dram_freq_mhz > 300){
		/* Enable dynamic PHY FIFO Reset & DQS half clock cycle */
		dynamic_FIFO_rst = 1;
	}
    
    if(PROJ_PARA_USE_ANALOG_DELAY_LINE){
        // 0xb8000608
    	// POW(enable DLL) = n20 , 
    	// BIN_DL0_PH_SEL<4:0>	n18..n15 = 5'bxxxx1 = 5'b00001, 
    	// BIN_DL1_PH_SEL<4:0> = 5'b10111, 
    	// BIN_DL2_PH_SEL<4:0> 	n19 = 5'bx1111 = 5'b01111
    	// PFD_DIV<5:0>			n14..n9 = 6'b000011
    	// SC<2:0>				n8..n6 = 3'b001
    	// SEL_IPUMP<3:0> 		n5..n2 = 4'b0111
    	// EN_NEWPUMP			n1 = 1'b1
    	// EN_TEST				n0 = 1'b0
    	// DQ ?™Ê?0ÔΩ?3?âÊ?ÔºåÂ??ÜBIN_DL0_PH_SEL<4:0>/ BIN_DL1_PH_SEL<4:0>/ BIN_DL2_PH_SEL<4:0>‰πüÂè™??0000ÔΩ?0111?âÊ???	//REG32(0xb8000608) = 	(3<<9) | (1<<6) | (7<<2) | (1<<1);		//clock 400MHz, 1step = 35ps
    	REG32(0xb8000608) = 	(0x0E<<15) | (3<<9) | (1<<6) | (7<<2) | (1<<1);		//clock 400MHz, 1step = 30ps
    	//REG32(0xb8000608) = 	(1<<19) | (0x0E<<15) | (3<<9) | (1<<6) | (7<<2) | (1<<1);		//clock 400MHz, 1step = 25ps, ?ôÂÄãË®≠ÂÆöÂ?‰∏ä‰???
    	REG32(0xb8000608) |= (1<<20);
    	dqs0_grp_dly=0;
        dqs1_grp_dly=0;
    	REG32(DACCR) &= ~(1<<31);				// n31=0 => analog delay chain	
    }else{
    	REG32(0xb8000608) &= ~(1<<20);		// disalbe DLL block. default enable.
    }
	dqs0_grp_dly = PROJ_PARA_DQS0_GROUP_DELAY;
    dqs1_grp_dly = PROJ_PARA_DQS1_GROUP_DELAY;
    dqs0_hclk    = PROJ_PARA_DQS0_HCLK;
    dqs1_hclk    = PROJ_PARA_DQS1_HCLK;    
	dqs0_en_tap  = PROJ_PARA_DQS0_EN_TAP;
   	dqs1_en_tap  = PROJ_PARA_DQS1_EN_TAP;
    REG32(DDCR) = (dqs0_hclk<<31) | (dqs0_en_tap<<24) | (dqs1_hclk<<23) |(dqs1_en_tap<<16);
    REG32(DACCR) = (ac_mode<<31) | (dqs0_grp_dly<< 16) |(dqs1_grp_dly<< 8)|(dynamic_FIFO_rst<<5);
    
 	if(dram_freq_mhz >= 575){
        REG32(DDR_DELAY_CTRL_REG0)=0x0fffffff;	
        REG32(DDR_DELAY_CTRL_REG1)=0x04040fff;	
        REG32(DDR_DELAY_CTRL_REG2)=0xffffffff;	
        REG32(DDR_DELAY_CTRL_REG3)=0xffffffff;	
        REG32(DDR_DELAY_CTRL_REG4)=0x00000000;
    }else if(dram_freq_mhz >= 475){
        REG32(DDR_DELAY_CTRL_REG0)=0x0fffffff;
        REG32(DDR_DELAY_CTRL_REG1)=0x04040fff;	
        REG32(DDR_DELAY_CTRL_REG2)=0xffffffff;
        REG32(DDR_DELAY_CTRL_REG3)=0xffffffff;
        REG32(DDR_DELAY_CTRL_REG4)=0x00000000;
    }else if(dram_freq_mhz >= 300){
        REG32(DDR_DELAY_CTRL_REG0)=0x0fffffff;
        REG32(DDR_DELAY_CTRL_REG1)=0x00000fff;
        REG32(DDR_DELAY_CTRL_REG2)=0xffffffff;
        REG32(DDR_DELAY_CTRL_REG3)=0xffffffff;
        REG32(DDR_DELAY_CTRL_REG4)=0x00000000;
    }else{
        REG32(DDR_DELAY_CTRL_REG0)=0x0fffffff;
        REG32(DDR_DELAY_CTRL_REG1)=0x00000fff;
        REG32(DDR_DELAY_CTRL_REG2)=0xffffffff;
        REG32(DDR_DELAY_CTRL_REG3)=0xffffffff;
        REG32(DDR_DELAY_CTRL_REG4)=0x00000000;
	}
	REG32(0xb80015c0)=REG32(DDR_DELAY_CTRL_REG4);
	_memctl_update_phy_param();
	return;
}

void memctlc_dll_setup(void){
	if(memctlc_DDR_Type() == IS_DDR2_SDRAM)
	{
		memctlc_ddr2_dll_reset();
	}
	else if(memctlc_DDR_Type() == IS_DDR3_SDRAM)
	{
		memctlc_ddr3_dll_reset();
	}
	else if(memctlc_DDR_Type() == IS_DDR_SDRAM)
	{
		memctlc_ddr1_dll_reset();
	}
	else
	{
		printf("AK: DLL reset failed: UNKNOWN DRAM TYPE.\n");
		while(1);
	}
}


