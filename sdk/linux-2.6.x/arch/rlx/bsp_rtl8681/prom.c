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
#ifdef CONFIG_RTL8672_SPI_FLASH
extern unsigned int SFCR, SFCSR, SFDR;
#endif //CONFIG_RTL8672_SPI_FLASH
#define CHIPVERMASK	(0xFFF00000)
#define IC8672		(0xFFF00000)
#define IC8671B		(0xFFE00000)
#define IC0315		(0xFFD00000)
#define IC6166		(0xFFC00000)
#define IC0412		0x0412
#define IC0437		0x0437
#define IC0513		0x0513

static void chip_detect(void){

	unsigned short chip_id, chip_ver;
	unsigned int LX_value;
	
	chip_id = (*(volatile unsigned int*)CHIP_ID_REG>>CHIP_ID_offset);
	chip_ver = (*(volatile unsigned int*)CHIP_ID_REG&0xffff);
	LX_value =  ((*(volatile unsigned int*)0xb8000008)>>1)&0xff;
	
	switch(chip_id){
		case IC0437:
		case IC0513:
			if( REG32(BSP_MISC_PINSTSR) & BSP_SYS_CLK_SEL){
				//BSP_MHZ = 150;
				BSP_MHZ = (LX_value+2)*10;
			}else{
				//BSP_MHZ = 200;
				BSP_MHZ = ((LX_value+2)*25/2);
			}
			//BSP_MHZ=25; //set 25M for FPGA testing
#ifdef CONFIG_RTL867X_NFBI_MASTER
			//BSP_MHZ=50;
			//BSP_MHZ=100;
			//BSP_MHZ=150;
			//BSP_MHZ=175;
			printk("--------set BSP_MHZ=%d---------\n", BSP_MHZ);
#endif /*CONFIG_RTL867X_NFBI_MASTER*/
			#ifdef CONFIG_RTL8672_SPI_FLASH
			SFCR = 0xB8001200;
			SFCSR = 0xB8001208;
			SFDR = 0xB800120C;
			#endif //CONFIG_RTL8672_SPI_FLASH
			break;
		default:
			printk("unknown chip!!!");
			break;
	}
	BSP_SYSCLK = BSP_MHZ*1000*1000;

#ifndef CONFIG_RTL8681_PTM
	*((volatile unsigned int*)0xb8a8c038)=0x001ffbfe;
	*((volatile unsigned int*)BSP_IP_SEL) = 0x0000000c;
#endif

}


/* Do basic initialization */
void __init bsp_init(void)
{
    chip_detect();
    prom_console_init();
    bsp_prom_meminit();
}


#ifdef CONFIG_RTL867X_NFBI_SLAVE
extern void mdio_set_dsllink_bit(int val);
#endif /*CONFIG_RTL867X_NFBI_SLAVE*/
/*xDSL status*/
unsigned char adslup=0;
unsigned char getADSLLinkStatus(void){
	return adslup;
}

void AdslLinkUp(void){
	adslup=1;
#ifdef CONFIG_RTL867X_NFBI_SLAVE
	mdio_set_dsllink_bit(1);
#endif /*CONFIG_RTL867X_NFBI_SLAVE*/
	printk("!!!!!!!!!! ADSL link up !!!!!!!!!!\n");
}

void AdslLinkDown(void){
	adslup=0;
#ifdef CONFIG_RTL867X_NFBI_SLAVE
	mdio_set_dsllink_bit(0);
#endif /*CONFIG_RTL867X_NFBI_SLAVE*/
	printk("!!!!!!!!!! ADSL link down !!!!!!!!!!\n");
}



