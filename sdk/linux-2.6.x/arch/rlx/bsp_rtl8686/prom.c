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
#include "bsp_automem.h"
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

static void inline prom_dram_unmap_clear(void){
  int i;
  for(i=0; i<4; i++){
      REG32(R_C0UMSAR0_BASE + i*0x10) = 0x0;
  }
}
#if defined(CONFIG_LUNA_MEMORY_AUTO_DETECTION)

#ifdef DEBUG
#define DEBUG_MEM_AUTO(fmt, args...) prom_printf( "DEBUG_MEM_AUTO: " fmt, ## args)
#else
#define DEBUG_MEM_AUTO(fmt, args...) 
#endif

#define PBO_BARRIER   PAGE_SIZE
#ifdef CONFIG_LUNA_PBO_DL_DRAM_SIZE
#define   LUNA_PBO_DL_DRAM_SIZE_BARRIER     (CONFIG_LUNA_PBO_DL_DRAM_SIZE + PBO_BARRIER) 
#endif
#ifdef CONFIG_LUNA_PBO_UL_DRAM_SIZE
#define   LUNA_PBO_UL_DRAM_SIZE_BARRIER     (CONFIG_LUNA_PBO_UL_DRAM_SIZE + PBO_BARRIER)
#endif



#if defined(CONFIG_RTL8686_IPC_DEV)
struct bsp_mem_map_s bsp_mem_map;
#endif

static int __initdata zone1_size = 0;
static int __initdata zone2_size = 0;
static unsigned int __initdata mem_size = 0;
static unsigned int __initdata reserved_slave_mem = 0;



static void inline remove_mem_para(void)
{
     char *ptr;
     char *ptr_mem;
     char *endptr;	/* local pointer to end of parsed string */
     unsigned long long ret = 0;
	
     ptr = strstr(arcs_cmdline, "mem=");
     ptr_mem= ptr+4;
     DEBUG_MEM_AUTO("arcs_cmdline=%s, %p, %p\n", arcs_cmdline, arcs_cmdline, ptr);
     if(ptr){
        ret = simple_strtoull(ptr_mem, &endptr, 0);
        memmove(ptr, (endptr+1), strlen((endptr+1)) + 1);
     }
     DEBUG_MEM_AUTO("arcs_cmdline=%s\n", arcs_cmdline);
}


static unsigned int __init memctlc_dram_size(void)
{
    unsigned int dcr;
    int total_bit = 0;

    dcr = *((unsigned int *)(C0DCR));
     DEBUG_MEM_AUTO("DCR=0x%08x\n", dcr );
    total_bit = 0;
    total_bit += ((dcr>>24)&0x3); //bus width
    total_bit += ((dcr>>20)&0x7)+11; //row count
    total_bit += ((dcr>>16)&0x7)+8 ; //col count
    total_bit += ((dcr>>28)&0x3)+1;  //bank count
    total_bit += (dcr>>15)&1;        //Dram Chip Select

    return ((1<<total_bit));
}


#ifdef CONFIG_LUNA_RESERVE_DRAM_FOR_PBO
unsigned int LUNA_PBO_DL_DRAM_OFFSET = 0;
unsigned int LUNA_PBO_UL_DRAM_OFFSET = 0;


void static inline bsp_pbo_mem_auto(void){
    unsigned int bankcnt;
    unsigned int banksize;

    /* BANKCNT: bit 29~28 */
    bankcnt = (*((unsigned int *)(C0DCR)) >> 28) & 0x3;
    /*
     * 00: 2 banks (used for SDR)
     * 01: 4 banks (used for SDR, DDR, DDR2) 
     * 10: 8 banks (used for DDR2, DDR3)
     */
     bankcnt = 2 << bankcnt;
     banksize = (mem_size/bankcnt);
    DEBUG_MEM_AUTO("dcr=0x%08x, bankcnt=0x%08x, mem_size=0x%08x, banksize=0x%08x\n",*((unsigned int *)(C0DCR)), bankcnt ,mem_size, banksize);     
    
     if(mem_size > 0x10000000){ /* (Total mem) > 256MB  */
    //MEM > 256MB
    /* ---512MB MEM -------
    * - Bank 0
    * 
    * 
    * - Bank x (If bank 8, x=2)
    * LUNA_PBO_DL_DRAM_SIZE_BARRIER
    * 
    * ---Bank x+1
    * LUNA_PBO_UL_DRAM_SIZE_BARRIER
    *---- Zone1 end: 0x10000000, 256MB
    * ...................
    *---- MEM End: 0x20000000, 512MB
    */
       
       LUNA_PBO_UL_DRAM_OFFSET = 0x10000000 - LUNA_PBO_UL_DRAM_SIZE_BARRIER;
       LUNA_PBO_DL_DRAM_OFFSET = 0x10000000 - banksize - LUNA_PBO_DL_DRAM_SIZE_BARRIER;
     }else{
    /* MEM= 256MB, 128MB, 64MB */     
    /* ---256MB MEM: 0x0 Start-------
    *  
    * 
    * LUNA_PBO_DL_DRAM_SIZE_BARRIER
    * ------ the last Bank
    * ....
    * .....
    * LUNA_PBO_UL_DRAM_SIZE_BARRIER
    * CONFIG_RTL8686_IPC_MEM_SIZE
    * CONFIG_RTL8686_DSP_MEM_SIZE
    * ---MEM END: 256MB -------
    */
      
       LUNA_PBO_UL_DRAM_OFFSET = mem_size - LUNA_PBO_UL_DRAM_SIZE_BARRIER;
       
       LUNA_PBO_DL_DRAM_OFFSET = mem_size - banksize - LUNA_PBO_DL_DRAM_SIZE_BARRIER;
#if defined(CONFIG_RTL8686_IPC_MEM_SIZE) && defined(CONFIG_RTL8686_DSP_MEM_SIZE)
       LUNA_PBO_UL_DRAM_OFFSET = LUNA_PBO_UL_DRAM_OFFSET - (CONFIG_RTL8686_IPC_MEM_SIZE + CONFIG_RTL8686_DSP_MEM_SIZE);
       /*Overlap "DSP or IPC" */
       while (LUNA_PBO_UL_DRAM_OFFSET <= LUNA_PBO_DL_DRAM_OFFSET ){
	 /* LUNA_PBO_DL_DRAM_OFFSET overlap LUNA_PBO_UL_DRAM_OFFSET
	  * LUNA_PBO_DL_DRAM_OFFSET must to move to x-1 bank
	  */
	 LUNA_PBO_DL_DRAM_OFFSET = LUNA_PBO_DL_DRAM_OFFSET- banksize ;
	 if (LUNA_PBO_DL_DRAM_OFFSET < banksize){
	   printk("[bsp]LUNA_PBO_DL_DRAM_OFFSET(0x%08x) is bad!", LUNA_PBO_DL_DRAM_OFFSET);
	   break;
	 }
       }
#endif       
       
     }
     DEBUG_MEM_AUTO("LUNA_PBO_UL_DRAM_OFFSET=0x%08x,LUNA_PBO_DL_DRAM_OFFSET=0x%08x\n", LUNA_PBO_UL_DRAM_OFFSET, LUNA_PBO_DL_DRAM_OFFSET );
}
#else
#define bsp_pbo_mem_auto()
#endif

static void inline prom_DSP_IPC_mem(void){

   /* ---256MB MEM: 0x0 Start-------
    *  
    * 
    * LUNA_PBO_DL_DRAM_SIZE_BARRIER
    *   --- Last Bank
    * 
    * LUNA_PBO_UL_DRAM_SIZE_BARRIER
    * CONFIG_RTL8686_IPC_MEM_SIZE
    * CONFIG_RTL8686_DSP_MEM_SIZE
    * ---MEM END: 256MB -------
    */

    /* ---512MB MEM -------
    *  ---- Start: 0x0
    * 
    * LUNA_PBO_DL_DRAM_SIZE_BARRIER
    * 
    * LUNA_PBO_UL_DRAM_SIZE_BARRIER
    *---- Zone1 end: 0x10000000, 256MB
    * 
    * 
    * 
    * CONFIG_RTL8686_IPC_MEM_SIZE
    * CONFIG_RTL8686_DSP_MEM_SIZE
    * ---MEM END: 512-------
    */

#if defined(CONFIG_RTL8686_IPC_DEV) && defined(CONFIG_RTL8686_DSP_MEM_SIZE) && defined(CONFIG_RTL8686_IPC_MEM_SIZE)
   bsp_mem_map.BSP_IPC_MEM_BASE = mem_size - (CONFIG_RTL8686_DSP_MEM_SIZE + CONFIG_RTL8686_IPC_MEM_SIZE);
   bsp_mem_map.BSP_DSP_MEM_BASE =  bsp_mem_map.BSP_IPC_MEM_BASE  + CONFIG_RTL8686_IPC_MEM_SIZE;
   DEBUG_MEM_AUTO("BSP_DSP_MEM_BASE=0x%x, BSP_IPC_MEM_BASE=0x%x \n", bsp_mem_map.BSP_DSP_MEM_BASE, bsp_mem_map.BSP_IPC_MEM_BASE);
#endif
}

static void inline prom_reserved_last(void){
  if( mem_size <= 0x10000000){
#if  defined(CONFIG_RTL8686_DSP_MEM_SIZE) && defined(CONFIG_RTL8686_IPC_MEM_SIZE)
    reserved_slave_mem = CONFIG_RTL8686_DSP_MEM_SIZE + CONFIG_RTL8686_IPC_MEM_SIZE;
#endif
#if defined(LUNA_PBO_UL_DRAM_SIZE_BARRIER)
    reserved_slave_mem = reserved_slave_mem + LUNA_PBO_UL_DRAM_SIZE_BARRIER;
#endif
  }else{
#if  defined(CONFIG_RTL8686_DSP_MEM_SIZE) && defined(CONFIG_RTL8686_IPC_MEM_SIZE)
    reserved_slave_mem = CONFIG_RTL8686_DSP_MEM_SIZE + CONFIG_RTL8686_IPC_MEM_SIZE;
#endif    
  }
}

void __init prom_mem_zone_detect(void){
   mem_size = memctlc_dram_size();
   bsp_pbo_mem_auto();

   prom_DSP_IPC_mem();
   prom_reserved_last();
   if((mem_size - reserved_slave_mem) > ZONE1_SIZE){  //>256MB
     
     prom_dram_unmap_clear();
     zone2_size = (mem_size - reserved_slave_mem) - ZONE1_SIZE;

     REG32(BSP_CDOR2) = ZONE2_OFF;
     REG32(BSP_CDMAR2) = zone2_size - 1;
     DEBUG_MEM_AUTO("BSP_CDOR2=0x%x, BSP_CDMAR2=0x%x\n",  REG32(BSP_CDOR2),REG32(BSP_CDMAR2) );
     zone1_size = ZONE1_SIZE;          //256MB 
   }else{
     zone1_size = mem_size - reserved_slave_mem; //<=256MB
  }
}



static void inline __init bsp_prom_memmap(void){
    unsigned int mem_start = 0;
    unsigned int region_size = 0;
    
    
    remove_mem_para();
    
    if(mem_size <= 0x10000000) {
    #if defined(LUNA_PBO_DL_DRAM_SIZE_BARRIER)
      region_size = LUNA_PBO_DL_DRAM_OFFSET;
      DEBUG_MEM_AUTO("[%s]add_memory_region(%d, %d, BOOT_MEM_RAM)\n", __func__, mem_start, region_size);
      add_memory_region(mem_start, region_size, BOOT_MEM_RAM);
      mem_start = LUNA_PBO_DL_DRAM_OFFSET + LUNA_PBO_DL_DRAM_SIZE_BARRIER;
    #endif
      region_size = (mem_size - reserved_slave_mem)- mem_start;
       DEBUG_MEM_AUTO("[%s]add_memory_region(%d, %d, BOOT_MEM_RAM)\n", __func__, mem_start, region_size);
      add_memory_region(mem_start, region_size, BOOT_MEM_RAM);
    }else{  /* DRAM > 256MB */

	  /* 
	   * CPU0: ZONE_NORMAL 
	   *  Only consider PBO
	   */
	  region_size = 0x10000000;
	#if defined(LUNA_PBO_DL_DRAM_SIZE_BARRIER)
	  region_size = LUNA_PBO_DL_DRAM_OFFSET;
	  DEBUG_MEM_AUTO("[%s]add_memory_region(%d, %d, BOOT_MEM_RAM)\n", __func__, mem_start, region_size);
	  add_memory_region(mem_start, region_size, BOOT_MEM_RAM);
	  mem_start = LUNA_PBO_DL_DRAM_OFFSET + LUNA_PBO_DL_DRAM_SIZE_BARRIER;
	  region_size = (0x10000000 - LUNA_PBO_UL_DRAM_SIZE_BARRIER)- mem_start;
	#endif
	  DEBUG_MEM_AUTO("[%s]add_memory_region(%d, %d, BOOT_MEM_RAM)\n", __func__, mem_start, region_size);
	  add_memory_region(mem_start, region_size, BOOT_MEM_RAM);

	/* CPU0: ZONE HIGHMEM          */
	/*       Handle IPC/DSM memory */
	  DEBUG_MEM_AUTO("[%s]add_memory_region(%d, %d, BOOT_MEM_RAM)\n", __func__, ZONE2_BASE, zone2_size);
	  add_memory_region(ZONE2_BASE, zone2_size, BOOT_MEM_RAM);
    }
}
#endif

/***********************************************************************/

#ifdef CONFIG_LUNA_MEMORY_AUTO_DETECTION
void __init bsp_prom_meminit(void)
{
        prom_mem_zone_detect();
        bsp_prom_memmap();
}
#else 
/***********************************************
 * Manually set up the mem map.
 * When DRAM >= 512MB
 * 1. With DSP/IPC arch
 *   a) Not add "mem=MEMEORY_SIZE" in the kernel cmdline
 *   b) Select CONFIG_HIGHMEM in kernel option
 *   c) Set up the mem offset for DSP, IPC
 *   d) Set up the mem offset for PBO
 * 
 * 
 * 2. Single image( Without DSP arch)
 *   a) Not add "mem=MEMEORY_SIZE" in the kernel cmdline
 *   b) Add "highmem=xxxM" in the kernel cmdline
 *      e,q: highmem=256M
 *   c) Select CONFIG_HIGHMEM in kernel option
 *   
 * Note
 *   1) The PBO offset must between 0~256MB
 *   2) The CONFIG_LUNA_PBO_DL_DRAM_OFFSET or 
 *      CONFIG_LUNA_PBO_UL_DRAM_OFFSET
 *      must be the end of CPU0 MEM Size
 *   3) The DSP/IPC should be put at the end of DRAM!
 ***********************************************/
static inline void bsp_add_highmem(void){
#if defined(CONFIG_RTL8686_CPU_MEM_SIZE) && (CONFIG_RTL8686_CPU_MEM_SIZE > ZONE1_SIZE)
#if !defined(CONFIG_HIGHMEM)
#warning "*******************************************************************"
#warning "****  CONFIG_RTL8686_CPU_MEM_SIZE > 256MB !                 *******"
#warning "****  You should select CONFIG_HIGHMEM to support HIGHMEM.  *******"
#warning "*******************************************************************"
#endif
if(CONFIG_RTL8686_CPU_MEM_SIZE > ZONE1_SIZE){
    prom_dram_unmap_clear();
    REG32(BSP_CDOR2) = ZONE2_OFF;
    REG32(BSP_CDMAR2) = (CONFIG_RTL8686_CPU_MEM_SIZE - ZONE1_SIZE) - 1;
    add_memory_region(ZONE2_BASE, (CONFIG_RTL8686_CPU_MEM_SIZE - ZONE1_SIZE), BOOT_MEM_RAM);
}
#else
/* No DSP arch                        */
/* We must get mem size from memhigh=??   */
     char *ptr;
     char *ptr_mem;
     char *endptr;	/* local pointer to end of parsed string */
     unsigned long mem_para = 0;
	
     ptr = strstr(arcs_cmdline, "highmem=");
     if(ptr){
        ptr_mem= ptr+8;
        prom_printf("arcs_cmdline=%s, %p, %p\n", arcs_cmdline, arcs_cmdline, ptr);
        mem_para = simple_strtoull(ptr_mem, &endptr, 0);//MB
	if(mem_para > 0){
	   mem_para = mem_para << 20;//MB->Byte
	   prom_printf("mem_para=0x%08x\n", mem_para);    
	   prom_dram_unmap_clear();
           REG32(BSP_CDOR2) = ZONE2_OFF;
           REG32(BSP_CDMAR2) = mem_para - 1;
           add_memory_region(ZONE2_BASE, mem_para , BOOT_MEM_RAM); 
	}
	
     }
#endif
}
void __init bsp_prom_meminit(void)
{
    char *ptr;
    u_long mem_size;
#ifdef CONFIG_LUNA_RESERVE_DRAM_FOR_PBO
    u_long base;
#endif

    /* Check the command line first for a memsize directive */
    ptr = strstr(arcs_cmdline, "mem=");

    if (ptr) {
        mem_size = memparse(ptr+4, &ptr);
    } else {
        /* No memsize in command line, add a default memory region */
#if defined(CONFIG_LUNA_RESERVE_DRAM_FOR_PBO)
	mem_size = min(CONFIG_LUNA_PBO_DL_DRAM_OFFSET, CONFIG_LUNA_PBO_UL_DRAM_OFFSET);
#elif defined(CONFIG_RTL8686_CPU_MEM_SIZE)
	mem_size = CONFIG_RTL8686_CPU_MEM_SIZE;
#else
        mem_size = cpu_mem_size;  /* Default to 32MB */
#endif /* #ifdef CONFIG_LUNA_RESERVE_DRAM_FOR_PBO */
    }

#ifdef CONFIG_RTL8686_CPU_MEM_SIZE
    if (mem_size > CONFIG_RTL8686_CPU_MEM_SIZE)
        mem_size = CONFIG_RTL8686_CPU_MEM_SIZE;
#endif /* #ifdef CONFIG_RTL8686_CPU_MEM_SIZE */

#ifdef CONFIG_LUNA_RESERVE_DRAM_FOR_PBO
    if (mem_size > CONFIG_LUNA_PBO_DL_DRAM_OFFSET)  
        mem_size = CONFIG_LUNA_PBO_DL_DRAM_OFFSET;
    if (mem_size > CONFIG_LUNA_PBO_UL_DRAM_OFFSET)  
        mem_size = CONFIG_LUNA_PBO_UL_DRAM_OFFSET;
#endif /* #ifdef CONFIG_LUNA_RESERVE_DRAM_FOR_PBO */
    /*
     * call <add_memory_region> to register boot_mem_map
     * add_memory_region(base, size, type);
     * type: BOOT_MEM_RAM, BOOT_MEM_ROM_DATA or BOOT_MEM_RESERVED
     */
     if (mem_size <= ZONE1_SIZE){
        add_memory_region(0, mem_size, BOOT_MEM_RAM);
     }else{
        add_memory_region(0, ZONE1_SIZE, BOOT_MEM_RAM);
     }
     

#ifdef CONFIG_LUNA_RESERVE_DRAM_FOR_PBO
    /* It is assumed that UL or DL is put on the end of DRAM and the other is put
       on the end of the previous bank to utilze the strength of para-bank accees
       E.g., For 32MB DDR2, it has 4 banks(8MB for each).
             Assuming that DL and UL use 2MB and 2MB respectively, if DL is put
             on 30MB(end of the DRAM), UL will be put on 22MB(end of the 3rd bank). 
    */ 
    if(CONFIG_LUNA_PBO_DL_DRAM_OFFSET > CONFIG_LUNA_PBO_UL_DRAM_OFFSET) {
        base = CONFIG_LUNA_PBO_UL_DRAM_OFFSET + CONFIG_LUNA_PBO_UL_DRAM_SIZE;
        mem_size = CONFIG_LUNA_PBO_DL_DRAM_OFFSET - base;
    } else {
        base = CONFIG_LUNA_PBO_DL_DRAM_OFFSET + CONFIG_LUNA_PBO_DL_DRAM_SIZE;
        mem_size = CONFIG_LUNA_PBO_UL_DRAM_OFFSET - base;
    }
    //prom_printf("base=0x%08x, mem_size=0x%08x\n", base, mem_size);
    add_memory_region(base, mem_size, BOOT_MEM_RAM);
#endif /* #ifdef CONFIG_LUNA_RESERVE_DRAM_FOR_PBO */

    /* If there is "mem" in UBoot bootargs, arcs_cmdline will be overwritten       . It is processed at 
       "early_para_mem():linux-2.6.x/arch/rlx/kernel/setup.c"
       and add_memory_region() is called again to update memory region 0 */
    
    /* For 512MB or above, 
     * For Apollo Memory Mem Map
     * ZONE_NORMAL :  0~256  (MB)@Physical
     * ZONE_HIGH   :  256~xx (MB)@Physical
     * 
     */
    bsp_add_highmem();   
}
#endif
void __init bsp_free_prom_memory(void)
{
  return;
}


//set system clock and spi control registers according chip type
unsigned int BSP_MHZ, BSP_SYSCLK;
unsigned int SOC_ID , SOC_ID_EXT , SOC_BOND_ID;
unsigned int GPIO_CTRL_0, GPIO_CTRL_1, GPIO_CTRL_2, GPIO_CTRL_4;
/*ccwei*/
#ifndef CONFIG_MTD_NAND
extern unsigned int SFCR, SFCSR, SFDR;
#else
unsigned int SFCR, SFCSR, SFDR;
#endif
/*end-ccwei*/

unsigned int _is_CKSEL_25MHz(void)
{
	if(REG32(BSP_SYSREG_PIN_STATUS_REG) & BSP_SYSREG_PIN_STATUS_CLSEL_MASK)
		return 0;
	else
		return 1;
}

static unsigned int pll_gen4_sys_LX_freq_mhz(void)
{
	unsigned int reg_val, LX_freq;

	reg_val = REG32(0xb8000228);
	reg_val &= (0xf);
	LX_freq = 1000/(reg_val + 2);

	return LX_freq;
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
#ifdef CONFIG_RTL8672_SPI_FLASH
	SFCR = 0xB8001200;
	SFCSR = 0xB8001208;
	SFDR = 0xB800120C;
#endif /* CONFIG_RTL8672_SPI_FLASH */

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

	/* BSP MHZ setting */
#ifdef LUNA_RTL9602C
	/* The following should be revised after 9602C PLL is finalized. */
	printk("RTL9602C PLL\n");
	//BSP_MHZ=40; //For FPGA
	BSP_MHZ=200; //For test chip QA/demo board
#else
	if((SOC_ID==0x6422) ||
	          (SOC_ID==0x0639)) {
		BSP_MHZ=pll_gen4_sys_LX_freq_mhz();
	} else {
		BSP_MHZ=sys_LX_freq_mhz();
	}
#endif /* #ifdef LUNA_RTL9602C */
	BSP_SYSCLK = BSP_MHZ*1000*1000;

	WRITE_MEM32(BOND_CHIP_MODE,0xb0000000); //enable bound id display.
	SOC_BOND_ID=(REG32(BOND_CHIP_MODE)& 0xff);
	SOC_ID_EXT=((REG32(BOND_CHIP_MODE)& 0x0000ff00)>>8);
}

#ifdef CONFIG_LUNA_CMU
void __init luna_cmu_init_at_boot(void)
{
    unsigned int reg_val;

    printk("===================================================\n");
    printk("0xbb000064=0x%08x\n", REG32(0xbb000064));
    /* Enable access CMU information register */
    printk("Enable access CMU information register\n");
    printk("0xb8000600=0x%08x\n", REG32(0xb8000600));
    reg_val = REG32(0xb8000600);
    reg_val &= ~(0x1FF << 16);
    reg_val |= (0x132 << 16);
    REG32(0xb8000600) = reg_val;
    printk("0xb8000600=0x%08x\n", REG32(0xb8000600));
    printk("0xbb000064=0x%08x\n", REG32(0xbb000064));

    printk("0xbb0001b0=0x%08x\n", REG32(0xbb0001b0));
    reg_val = REG32(0xbb0001b0);
    reg_val &= ~(0xFF << 8);
    reg_val |= (0x0F << 8);
    REG32(0xbb0001b0) = reg_val;
    printk("0xbb0001b0=0x%08x\n", REG32(0xbb0001b0));
    printk("0xbb000064=0x%08x\n", REG32(0xbb000064));

    printk("Enable CMU\n");

    reg_val = REG32(0xb8000308);

    /* OCP 0 slow bit=1, should be moved to preloader */
    reg_val |= (0x1 << 4);

    /* Set OCP0 freq divider */
    reg_val = (reg_val & ~(0x7 << 23)) | (CONFIG_CPU0_FREQ_DIV_INDEX << 23);

    /* Set LX012P SW BZ bits to 1 */
    reg_val |= ((0x1 << 29) | (0x1 << 28) | (0x1 << 27) | (0x1 << 26));

    /* Enable CMU dynamic mode */
    reg_val = (reg_val & ~(0x3 << 6)) | (0x2 << 6);

    /* Set OCP0 busy */
    reg_val |= (0x1 << 31);
    //printk("reg_val=0x%08x\n", reg_val);

    REG32(0xb8000308)=reg_val; 

    printk("0xb8000308=0x%08x\n", REG32(0xb8000308));
    printk("0xbb000064=0x%08x\n", REG32(0xbb000064));

    printk("===================================================\n");
}
#endif /* #ifdef CONFIG_LUNA_CMU */

/* Do basic initialization */
void __init bsp_init(void)
{
    chip_detect();
    prom_console_init();
    bsp_prom_meminit();
    
#ifdef CONFIG_LUNA_CMU
    luna_cmu_init_at_boot();
#endif /* #ifdef CONFIG_LUNA_CMU */
}

