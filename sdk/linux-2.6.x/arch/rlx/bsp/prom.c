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
/*ccwei*/
#ifndef CONFIG_MTD_NAND
extern unsigned int SFCR, SFCSR, SFDR;
#else
unsigned int SFCR, SFCSR, SFDR;
#endif
/*end-ccwei*/

#define CHIPVERMASK	(0xFFF00000)
#define IC8672		(0xFFF00000)
#define IC8671B		(0xFFE00000)
#define IC0315		(0xFFD00000)
#define IC6166		(0xFFC00000)
#define IC0412		0x0412
#define IC6239		0x6239
static void chip_detect(void){
#ifdef CONFIG_RTL8676
	unsigned short chip_id, chip_ver;
	
	chip_id = (*(volatile unsigned int*)CHIP_ID_REG>>CHIP_ID_offset);
	chip_ver = (*(volatile unsigned int*)CHIP_ID_REG&0xffff);
	
	switch(chip_id){
		case IC0412:
		case IC6239:
			BSP_MHZ=((((REG32(BSP_SCCR) & 0x01F00)>>8)+2)*10); //lx clock =  20 * (N+2) * 1 / 2
			SFCR = 0xB8001200;
			SFCSR = 0xB8001208;
			SFDR = 0xB800120C;
			break;
		default:
			printk("unknown chip!!!");
			break;
	}
	BSP_SYSCLK = BSP_MHZ*1000*1000;
#else
	unsigned int sccr, ICver;
	
	sccr=*(volatile unsigned int*)BSP_SCCR;
	if ((sccr&CHIPVERMASK)==IC8671B)
	{//ic8671B(6085 with packet processor)
		ICver = IC8671B;
	}
	else if ((sccr&CHIPVERMASK)==IC0315)
	{//ic0315 //adjust clock frequency
		ICver = IC0315;
	}
	else if(( sccr & CHIPVERMASK) == IC8672)
	{//ic8672  Ver A & B
		ICver =IC8672;
	}
	else if(( sccr & CHIPVERMASK) == IC6166)
	{//6166  
		ICver =IC6166;
	}
	else
	{//unknown chip type, regard as IC6166
		ICver =IC6166;		
	}

	switch (ICver)
	{
	case IC8671B://6085
		BSP_MHZ=175;
		SFCR = 0xB8001200;
		SFCSR = 0xB8001208;
		SFDR = 0xB800120C;
		break;
	case IC0315:
	case IC6166:
		BSP_MHZ=180;
		SFCR = 0xB8001200;
		SFCSR = 0xB8001208;
		SFDR = 0xB800120C;
		break;
	case IC8672://6028
	default:
		BSP_MHZ=175;
		SFCR = 0xB8001200;
		SFCSR= 0xB8001204;
		SFDR = 0xB8001208;
	}
	BSP_SYSCLK = BSP_MHZ*1000*1000;
#endif
}


/* Do basic initialization */
void __init bsp_init(void)
{
    chip_detect();
    prom_console_init();
    bsp_prom_meminit();
}

