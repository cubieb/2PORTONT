#include <common.h>
#include <asm/arch/bspchip.h>
#include <asm/arch/memctl.h>
#include <asm/otto_pll.h>

/*Board DQS Range (mode, MHz, min tap, max tap)*/
/* Mode: DLL/Digital */
tap_info_t DQS_RANGE[10] =	{
					/*100MHz*/{DLL_MODE, 100, 3, 16} , {DIGITAL_MODE, 100, 1, 23}, 
					/*125MHz*/{DLL_MODE, 125, 3, 13} , {DIGITAL_MODE, 125, 1, 18},
					/*150MHz*/{DLL_MODE, 150, 3, 12} , {DIGITAL_MODE, 150, 1, 13},
					/*175MHz*/{DLL_MODE, 175, 3, 11} , {DIGITAL_MODE, 175, 1, 10},
					/*200MHz*/{DLL_MODE, 200, 4, 10} , {DIGITAL_MODE, 200, 1, 8}
				};

/*28M Demoboard: Write 90 phase range (mode, MHz, min tap, max tap) */
tap_info_t W90_RANGE[10] =	{
					/*100MHz*/{DLL_MODE, 100, 0, 19} , {DIGITAL_MODE, 100, 0, 19},
					/*125MHz*/{DLL_MODE, 125, 0, 16} , {DIGITAL_MODE, 125, 0, 16},
					/*150MHz*/{DLL_MODE, 150, 0, 14} , {DIGITAL_MODE, 150, 0, 14},
					/*175MHz*/{DLL_MODE, 175, 1, 13} , {DIGITAL_MODE, 175, 1, 13},
					/*200MHz*/{DLL_MODE, 200, 2, 12} , {DIGITAL_MODE, 200, 2, 12},
				};


//unsigned int board_DRAM_freq_mhz(void);
extern unsigned int sys_DSP_freq_mhz(void);
extern void memctlc_check_DCR(void);

unsigned int get_input(void)
{
	char input_buffer[10];
	char c;
	unsigned int input_num;

	c = '\0';
	input_num = 0;

	while( (c != '\r') && (input_num < sizeof(input_buffer)) ){
		c = getc();
		putc(c);
		input_buffer[input_num] = c;
		input_num++;
	}

	input_buffer[input_num-1] = '\0';

	return (unsigned int)simple_strtoull(input_buffer, NULL, 10);

}


int luna_sys_adj_mem_clk(void)
{
#ifdef CONFIG_SOFTWARE_OVERWRITE_FREQ
	unsigned int cpu_clk, cpu_field;
	unsigned int dsp_clk, dsp_field;
	unsigned int mem_clk, mem_field;
	unsigned int sys_pll_ctl_value;
	char *s;

	cpu_clk = 500;
	dsp_clk = 500;

#ifdef CONFIG_SYS_BOOTING_DIALOG
	printf("\n\nPlease input memory clock(MHz):");
	mem_clk = get_input();
	printf("\nGet input mem_clk:(%d)\n", mem_clk);
#else
	#ifdef CONFIG_SOFTWARE_FREQUENCY_ENV_DECIDE
	mem_clk = 0;
	#else
	mem_clk = CONFIG_MEMCLK_MHZ;
	#endif
#endif

	if(mem_clk ==0){
	        s = getenv("mem_clk");
       		if (s) {
			mem_clk = (unsigned int)simple_strtoull(s, NULL, 10);
		
			if(mem_clk < 50)
				mem_clk = 50;

			if(mem_clk > 400)
				mem_clk = 400;
	        }else{
			mem_clk = 100;
		}
	}else{

		if(mem_clk < 50)
			mem_clk = 50;

		if(mem_clk > 400)
			mem_clk = 400;
	}

	return sys_adj_clk(cpu_clk, dsp_clk, mem_clk);
#endif
	return 0;
}

void _set_dram_buswidth(unsigned int dram_bus_width)
{
	volatile unsigned int *dcr;

	
	dcr = (volatile unsigned int *)DCR;

	switch (dram_bus_width){
		case 8:
			*dcr = *dcr & (0xF0FFFFFF);		
			break;
		case 16:
			*dcr = *dcr | (0x01000000);		
			break;

		default:
			return;
	}

	return;
}


int luna_sys_adj_cpu_clk(void)
{
#ifdef CONFIG_SOFTWARE_OVERWRITE_FREQ
	unsigned int cpu_clk, cpu_field;
	unsigned int dsp_clk, dsp_field;
	unsigned int mem_clk, mem_field;
	unsigned int sys_pll_ctl_value;
	unsigned int dram_bus_width;
	char *s;

#ifdef CONFIG_SYS_BOOTING_DIALOG
	printf("\n\nPlease input CPU clock(MHz):");
	cpu_clk = get_input();
	printf("\nGet input cpu_clk:(%d)\n", cpu_clk);

	printf("\n\nPlease input DRAM bus width:");
	dram_bus_width = get_input();
	printf("\nGet input bus width:(%d)\n", dram_bus_width);
	_set_dram_buswidth(dram_bus_width);

#else
	#ifdef CONFIG_SOFTWARE_FREQUENCY_ENV_DECIDE
	cpu_clk = 0;
	dsp_clk = 0;
	#else
	cpu_clk = CONFIG_CPUCLK_MHZ;
	dsp_clk = CONFIG_DSPCLK_MHZ;
	#endif
#endif

	if(cpu_clk == 0){
	        s = getenv("cpu_clk");
       	 	if (s) {
			cpu_clk = (unsigned int)simple_strtoull(s, NULL, 10);
			if(cpu_clk < 500)
				cpu_clk = 500;

			if(cpu_clk > 700)
				cpu_clk = 700;
	        }else{
			cpu_clk = 500;
		}
	}else{
		if(cpu_clk < 500)
			cpu_clk = 500;

		if(cpu_clk > 700)
			cpu_clk = 700;
	}
	
	if(dsp_clk == 0){
        	s = getenv("dsp_clk");
	        if (s) {
			dsp_clk = (unsigned int)simple_strtoull(s, NULL, 10);

			if(dsp_clk < 500)
				dsp_clk = 500;

			if(dsp_clk > 550)
				dsp_clk = 550;
	        }else{
			dsp_clk = 500;
		}
	}else{
		if(dsp_clk < 500)
			dsp_clk = 500;

		if(dsp_clk > 550)
			dsp_clk = 550;
	}

	mem_clk = board_DRAM_freq_mhz();

	return sys_adj_clk(cpu_clk, dsp_clk, mem_clk);
#endif
	return 0;
}

/* Function Name: 
 * 	board_DSP_freq_hz
 * Descripton:
 *	Get the current CPU frequency in Hz.
 * Input:
 *	None
 * Output:
 * 	None
 * Return:
 *  	CPU frequncy in Hz.
 */
unsigned int board_DSP_freq_hz(void)
{
	unsigned int ocp_freq_hz;

	ocp_freq_hz = board_DSP_freq_mhz() * 1000000;
	return  ocp_freq_hz;
}

/* Function Name: 
 * 	board_CPU_freq_mhz
 * Descripton:
 *	Get the current CPU frequency in MHz.
 * Input:
 *	None
 * Output:
 * 	None
 * Return:
 *  	CPU frequncy in MHz.
 */
#if 0 /* Use the one in otto_pll.c */
unsigned int board_CPU_freq_mhz(void)
{
	return sys_CPU_freq_mhz();
}
#endif



/* Function Name: 
 * 	board_CPU_freq_hz
 * Descripton:
 *	Get the current CPU frequency in Hz.
 * Input:
 *	None
 * Output:
 * 	None
 * Return:
 *  	CPU frequncy in Hz.
 */
unsigned int board_CPU_freq_hz(void)
{
	unsigned int ocp_freq_hz;

	ocp_freq_hz = board_CPU_freq_mhz() * 1000000;
	return  ocp_freq_hz;
}




/* Function Name: 
 * 	board_LX_freq_mhz
 * Descripton:
 *	Get the current LX bus frequency in MHz.
 * Input:
 *	None
 * Output:
 * 	None
 * Return:
 *  	LX bus frequncy in MHz.
 */
#if 0 /* Use the one in otto_pll.c */
unsigned int board_LX_freq_mhz(void)
{
	return  sys_LX_freq_mhz();
}
#endif

/* Function Name: 
 * 	board_LX_freq_hz
 * Descripton:
 *	Get the current LX bus frequency in Hz.
 * Input:
 *	None
 * Output:
 * 	None
 * Return:
 *  	LX bus frequncy in MHz.
 */
unsigned int board_LX_freq_hz(void)
{
	unsigned int lx_freq_hz;

	lx_freq_hz = board_LX_freq_mhz();
	return (lx_freq_hz * 1000000);

}
/* Function Name: 
 * 	board_DRAM_freq_mhz
 * Descripton:
 *	Get the current DRAM frequency in MHz.
 * Input:
 *	None
 * Output:
 * 	None
 * Return:
 *  	DRAM Frequncy in MHz.
 */
#if 0 /* Use the one in otto_pll.c */
unsigned int board_DRAM_freq_mhz(void)
{
	return sys_DRAM_freq_mhz();
}
#endif

void show_DRAM_phy_parameters(void)
{
	unsigned int i;
	volatile unsigned int *ptr;

	printf("\tSYSREG_DDRCKODL_REG(0x%08x):0x%08x;\n", \
			SYSREG_DDRCKODL_REG, *((unsigned int *)SYSREG_DDRCKODL_REG) );
	ptr = (unsigned int *)DACCR;
	printf("\tDACCR(0x%08x):\n", (unsigned int)ptr);
	for(i = 0; i < 10; i++){
		printf("\t\t(0x%08x):0x%08x", (unsigned int)ptr, *ptr );
		ptr++;
		printf(", 0x%08x ", *ptr );
		ptr++;
		printf(", 0x%08x ", *ptr );
		ptr++;
		printf(", 0x%08x\n", *ptr);
		ptr++;
	}
	return;
}

#if (CONFIG_STANDALONE_UBOOT != 1)
/*
 * Check DRAM Configuration setting. 
 */
int chk_dram_cfg(unsigned int  dram_freq_mhz)
{
	int     	rcode = 0;

	/* Show register informantion. */
	printf("Register settings:\n");
	printf("\tMCR (0x%08x):0x%08x\n", MCR, REG(MCR));
	printf("\tDCR (0x%08x):0x%08x\n", DCR, REG(DCR));
	printf("\tDTR0(0x%08x):0x%08x\n", DTR0,REG(DTR0));
	printf("\tDTR1(0x%08x):0x%08x\n", DTR1,REG(DTR1));
	printf("\tDTR2(0x%08x):0x%08x\n", DTR2,REG(DTR2));
	printf("\tDDCR(0x%08x):0x%08x\n", DDCR, REG(DDCR));
	printf("\tDCDR(0x%08x):0x%08x\n", DCDR, REG(DCDR));

	show_DRAM_phy_parameters();

	/* Get current DRAM freq. */
	
	if(dram_freq_mhz==0){
		dram_freq_mhz = board_DRAM_freq_mhz(); 
		printf("\t Current DRAM freqency: %dMHz\n", dram_freq_mhz);
	}else{
		printf("\t Check for DRAM freqency: %dMHz\n", dram_freq_mhz);
	}

	/* DRAM memory type in MCR (DDR1/2/3). */
	printf("DRAM Type: ");
	if(memctlc_is_DDR())
		printf("DDR SDRAM \n");
	else if(memctlc_is_DDR2())
		printf("DDR2 SDRAM \n");
	else if(memctls_is_DDR3())
		printf("DDR3 SDRAM \n");
	else
		printf("ERROR UNKNOWN! \n");


	/* 
	 * Check DCR
	 */
	memctlc_check_DCR();

	
	/* 
	 * Check DTR
	 */
	memctlc_check_DTR(dram_freq_mhz);


	/*
	 * Chech DDCR and DCDR
	 */
	if(memctlc_is_DDR()){
		/* if DDR SDRAM : Check DDR SDRAM DQS delay in DDCR. */
		memctlc_check_DQS_range();
		/* if DDR SDRAM : Check 90 phase delay in DCDR. */
		memctlc_check_90phase_range();
	}
	if(memctlc_is_DDR2()){
		memctlc_check_ZQ();
		/* Check TX/RX value ?*/
	}
	if(memctlc_is_DDR3()){
		memctlc_check_ZQ();
		/* Check TX/RX value ?*/
	}

	return rcode;
}
#endif

/*
 * DRAM parameters APIs for initializing DDR1/2/3 SDRAM
 * We current define environment variables listed below
 * for DRAM ODT/OCD/ZQ/Delay parameters. These fuctions are called when initializing
 * DDR1/2/3 SDRAM memory.
 * zq_cali_value: ZQ auto-calibration value, we apply zq_cali_value if it exists.
 * dram_odt	: DRAM side ODT value of DDR2 SDRAM(0, 50, 75, 150)
 * dram_ocd	: DRAM side OCD value of DDR3 SDRAM(ZQ/6, ZQ/7)
 * ddrkodl	: DDRKODL register value. (Assign CLKM/TX/PHASE 90 delay.)
 * 
 */
unsigned int get_memory_delay_parameters(unsigned int *para_array)
{
        char *s = getenv("ddrkodl");
        if (s) {
		para_array[0] = (unsigned int)simple_strtoull(s, NULL, 16);
		return 1;//OK 
        }
	return 0; //fail
}


unsigned int get_memory_ZQ_parameters(unsigned int *para_array)
{
        char *s = getenv("zq_cali_value");
        if (s) {
		para_array[0] = (unsigned int)simple_strtoull(s, NULL, 16);
		return 1;//OK 
        }
	return 0; //fail
}

unsigned int get_memory_dram_odt_parameters(unsigned int *para_array)
{
        char *s = getenv("dram_odt");
        if (s) {
		para_array[0] = (unsigned int)simple_strtoull(s, NULL, 16);
		return 1;//OK 
        }
	return 0; //fail
}

unsigned int get_memory_dram_ocd_parameters(unsigned int *para_array)
{
        char *s = getenv("dram_ocd");
        if (s) {
		para_array[0] = (unsigned int)simple_strtoull(s, NULL, 16);
		return 1;//OK 
        }
	return 0; //fail
}

