/*
 * Copyright 2006, Realtek Semiconductor Corp.
 *
 * arch/rlx/rlxocp/prom.c
 *   Early initialization code for the RLX OCP Platform
 *
 * Tony Wu (tonywu@realtek.com.tw)
 * Nov. 7, 2006
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <asm/bootinfo.h>
#include <asm/addrspace.h>
#include <asm/page.h>
#include <asm/cpu.h>
#include <asm/rlxbsp.h>
#include "bspcpu.h"
#include "bspchip.h"
#include "prom.h"

extern char arcs_cmdline[];

#ifdef CONFIG_EARLY_PRINTK
static int promcons_output __initdata = 0;
                                                                                                    
void unregister_prom_console(void)
{
    if (promcons_output) {
        promcons_output = 0;
    }
}
                                                                                                    
void disable_early_printk(void)
    __attribute__ ((alias("unregister_prom_console")));
#endif
                                                                                                    

const char *get_system_type(void)
{
    return "RTL8672";
}

void __init bsp_prom_meminit(void)
{
    char *ptr;
    u_long mem_size;

   /* Check the command line first for a memsize directive */
   ptr = strstr(arcs_cmdline, "mem=");

   if (ptr)
      mem_size = memparse(ptr + 4, &ptr);
   else
      mem_size = cpu_mem_size;  /* Default to 32MB */
   if (mem_size > CONFIG_RTL8686_DSP_MEM_SIZE)
      mem_size = CONFIG_RTL8686_DSP_MEM_SIZE;
   /*
    * call <add_memory_region> to register boot_mem_map
    * add_memory_region(base, size, type);
    * type: BOOT_MEM_RAM, BOOT_MEM_ROM_DATA or BOOT_MEM_RESERVED
    */
    add_memory_region(0, mem_size, BOOT_MEM_RAM);
}

void __init bsp_free_prom_memory(void)
{
  return;
}


//set system clock and spi control registers according chip type
unsigned int BSP_MHZ, BSP_SYSCLK;
unsigned int SOC_ID, SOC_BOND_ID;
unsigned int GPIO_CTRL_0, GPIO_CTRL_1, GPIO_CTRL_2, GPIO_CTRL_4;

unsigned int _is_CKSEL_25MHz(void)
{
	if(REG32(BSP_SYSREG_PIN_STATUS_REG) & BSP_SYSREG_PIN_STATUS_CLSEL_MASK)
		return 0;
	else
		return 1;
}

unsigned int sys_LX_freq_mhz(void)
{
	volatile unsigned int *cmu_ctlr_reg;
	unsigned int lx_clk_div;
	unsigned int lx_clk_div_offset;
	unsigned int lx_freq_mhz;
	unsigned int src_oc_mhz;
	unsigned int cmu_div;

	cmu_ctlr_reg = (volatile unsigned int *)BSP_SYSREG_CMUCTLR_REG;
	cmu_div = 1;
	if(((*cmu_ctlr_reg & BSP_SYSREG_CMUCTLR_CMU_MD_MASK)>> BSP_SYSREG_CMUCTLR_CMU_MD_FD_S)\
			== BSP_SYSREG_CMUCTLR_CMU_MD_MANUALLY_SEL){
		cmu_div = 1 << ((*cmu_ctlr_reg & BSP_SYSREG_CMUCTLR_CMU_LX0_FREQ_DIV_MASK) >>\
				BSP_SYSREG_CMUCTLR_CMU_LX0_FREQ_DIV_FD_S);
	}

	if(REG32(BSP_SYSREG_LX_PLL_SEL_REG) == 0){
		return 200;
	}else{
		if(_is_CKSEL_25MHz()){
			src_oc_mhz = 25;
			lx_clk_div_offset = 6;
		}else{
			src_oc_mhz = 40 / 2;
			lx_clk_div_offset = 8;
		}

		lx_clk_div = REG32(BSP_SYSREG_LX_PLL_SEL_REG) + lx_clk_div_offset;
		lx_freq_mhz = (src_oc_mhz * (lx_clk_div + 2))/2;
	}

	return  lx_freq_mhz/cmu_div;
}

#define CHIPVERMASK	(0xFFF00000)
#define IC8672		(0xFFF00000)
#define IC8671B		(0xFFE00000)
#define IC0315		(0xFFD00000)
#define IC6166		(0xFFC00000)
#define IC0412		0x0412
#define IC6239		0x6239
static void chip_detect(void){
	//BSP_MHZ=200;
	BSP_MHZ=sys_LX_freq_mhz();
	BSP_SYSCLK = BSP_MHZ*1000*1000;
	//read soc id
	WRITE_MEM32(CHIP_ID_REG, 0xa0000000);
	SOC_ID=(REG32(CHIP_ID_REG)& 0xffff);
	if(SOC_ID==0x0371){
		GPIO_CTRL_0 = 0xbb000100;
		GPIO_CTRL_1 = 0xbb00010c;
		GPIO_CTRL_2 = 0xbb000118;
		GPIO_CTRL_4 = 0xbb0001d4;
	} else {
		GPIO_CTRL_0 = 0xBB0000D8;
		GPIO_CTRL_1 = 0xBB0000E4;
		GPIO_CTRL_2 = 0xBB0000F0;
		GPIO_CTRL_4 = 0xBB0000FC;
	}
	WRITE_MEM32(BOND_CHIP_MODE,0xb0000000); //enable bound id display.
	SOC_BOND_ID=(REG32(BOND_CHIP_MODE)& 0xff);
}


/* Do basic initialization */
void __init bsp_init(void)
{
    chip_detect();
    prom_console_init();
    bsp_prom_meminit();
}

