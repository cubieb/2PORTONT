/*
 * Flash mapping for BCM947XX boards
 *
 * Copyright (C) 2001 Broadcom Corporation
 *
 * $Id: rtl8672_flash.c,v 1.6 2012/06/06 09:04:09 kaohj Exp $
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <asm/io.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/root_dev.h>
#include <linux/mtd/partitions.h>
#include <linux/config.h>
#include <linux/delay.h>
#include <bspchip.h>
/*linux-2.6.19*/
#include <linux/version.h>
//ql
//#include "../../../../config/autoconf.h"

#if defined( CONFIG_4M_FLASH) || defined (CONFIG_2M_FLASH)
#define WINDOW_ADDR 0xbfc00000
#else //CONFIG_4M_FLASH
#define WINDOW_ADDR 0xbd000000
#endif //CONFIG_4M_FLASH
#ifdef CONFIG_SPANSION_16M_FLASH	
#define WINDOW_SIZE 0x1000000
#define FLASH_BANK_SIZE 0x400000
#else 
#ifdef CONFIG_4M_FLASH
#define WINDOW_SIZE 0x400000
#endif
#ifdef CONFIG_8M_FLASH 
#define WINDOW_SIZE 0x800000
#endif
#ifdef CONFIG_16M_FLASH
#define WINDOW_SIZE 0x1000000
#endif
#ifdef CONFIG_2M_FLASH
#define WINDOW_SIZE 0x200000
#endif 
#endif //CONFIG_SPANSION_16M_FLASH
#define BUSWIDTH 2

static struct mtd_info *rtl8672_mtd;

__u8 rtl8672_map_read8(struct map_info *map, unsigned long ofs)
{
	//printk("enter %s %d\n",__FILE__,__LINE__);
	return __raw_readb(map->map_priv_1 + ofs);
}

__u16 rtl8672_map_read16(struct map_info *map, unsigned long ofs)
{
	//printk("enter %s %d\n",__FILE__,__LINE__);
	return __raw_readw(map->map_priv_1 + ofs);
}

__u32 rtl8672_map_read32(struct map_info *map, unsigned long ofs)
{
	//printk("enter %s %d\n",__FILE__,__LINE__);
	return __raw_readl(map->map_priv_1 + ofs);
}

void rtl8672_map_copy_from(struct map_info *map, void *to, unsigned long from, ssize_t len)
{
	//printk("enter to %x from  %x len %d\n",to, map->map_priv_1+from , len);
	//11/15/05' hrchen, change the size to fit file systems block size if use different fs
	//4096 for cramfs, 1024 for squashfs
	if (from>0x10000)
	    memcpy(to, map->map_priv_1 + from, (len<=1024)?len:1024);//len);
	else
	    memcpy(to, map->map_priv_1 + from, (len<=4096)?len:4096);//len);
	//printk("enter %s %d\n", __FILE__,__LINE__);

}

void rtl8672_map_write8(struct map_info *map, __u8 d, unsigned long adr)
{
	__raw_writeb(d, map->map_priv_1 + adr);
	mb();
}

void rtl8672_map_write16(struct map_info *map, __u16 d, unsigned long adr)
{
	__raw_writew(d, map->map_priv_1 + adr);
	mb();
}

void rtl8672_map_write32(struct map_info *map, __u32 d, unsigned long adr)
{
	__raw_writel(d, map->map_priv_1 + adr);
	mb();
}

void rtl8672_map_copy_to(struct map_info *map, unsigned long to, const void *from, ssize_t len)
{
	//printk("enter %s %d\n",__FILE__,__LINE__);
	memcpy_toio(map->map_priv_1 + to, from, len);
}

struct map_info rtl8672_map = {
	name: "Physically mapped flash",
	size: WINDOW_SIZE,
	bankwidth: BUSWIDTH,
	phys: WINDOW_ADDR
	/*
	read8: rtl8672_map_read8,
	read16: rtl8672_map_read16,
	read32: rtl8672_map_read32,
	copy_from: rtl8672_map_copy_from,
	write8: rtl8672_map_write8,
	write16: rtl8672_map_write16,
	write32: rtl8672_map_write32,
	copy_to: rtl8672_map_copy_to
	*/
};



/////////////////////////////////////////////////////////////////////////////
#ifdef CONFIG_E8B
static struct mtd_partition rtl8672_parts[] = {
	{ name: "boot",	  offset: 0,       size: 0x20000, mask_flags: 0 },
	{ name: "HW",     offset: 0x20000, size: 0x20000, mask_flags: 0 },
	{ name: "CS",     offset: 0x40000, size: 0x20000, mask_flags: 0 },
	{ name: "tr069",  offset: 0x60000, size: 0x20000, mask_flags: 0 },
//#ifdef CONFIG_BACKUP_IMG
#if 1
	{ name: "cs-bak", offset: 0x80000, size: 0x80000, mask_flags: 0 },
	//offset:0xA0000, size:0x60000 reserved.
#ifdef CONFIG_DOUBLE_IMAGE
	{ name: "rootfs", offset: 0x100000,size: 0x500000,mask_flags: 0 },
	{ name: "fs-bak", offset: 0x600000,size: 0x500000,mask_flags: 0 },
#endif
#ifdef CONFIG_SINGLE_IMAGE
	{ name: "fs-bak", offset: 0x100000,size: 0x200000,mask_flags: 0 },
	{ name: "rootfs", offset: 0x300000,size: 0x500000,mask_flags: 0 },
#endif
	{ name: "midware",offset: 0xB00000,size: 0x500000,mask_flags: 0 }
#else
	{ name: "rootfs", offset: 0x80000, size: 0x500000,mask_flags: 0 },
	{ name: "midware",offset: 0x800000,size: 0x800000,mask_flags: 0 }
#endif
};
#else // of CONFIG_E8B
//10/17/05' hrchen, kernel is removed
static struct mtd_partition rtl8672_parts[] = {
	/*patch from linux 2.4*/
	{ name: "boot",	offset:  0 ,size:  CONFIG_BOOT_SIZE, mask_flags: 0 },
#ifndef CONFIG_2M_FLASH
#ifdef CONFIG_SINGLE_IMAGE
	{ name: "CS",     offset: CONFIG_BOOT_SIZE, size: 0x10000, mask_flags: 0 },
	{ name: "config", offset: CONFIG_BOOT_SIZE+0x10000, size: 0x10000,mask_flags:0},
	{ name: "rootfs", offset: CONFIG_BOOT_SIZE+0x20000, size: CONFIG_FLASH_SIZE-(CONFIG_BOOT_SIZE+0x20000),mask_flags:0},
#else	// CONFIG_DOUBLE_IMAGE
	{ name: "HW",     offset: CONFIG_BOOT_SIZE, size: 0x20000, mask_flags: 0 },
	{ name: "CS",     offset: CONFIG_BOOT_SIZE+0x20000, size: 0x20000, mask_flags: 0 },
	{ name: "tr069",  offset: CONFIG_BOOT_SIZE+0x40000, size: 0x20000, mask_flags: 0 },
	{ name: "cs-bak", offset: CONFIG_BOOT_SIZE+0x60000, size: 0x100000-CONFIG_BOOT_SIZE-0x60000, mask_flags: 0 },
	{ name: "rootfs", offset: 0x100000,size: 0x500000,mask_flags: 0 },
	{ name: "fs-bak", offset: 0x600000,size: 0x500000,mask_flags: 0 },
#endif
#else // CONFIG_2M_FLASH
	{ name: "rootfs",offset:  CONFIG_BOOT_SIZE, size:0x1F0000-CONFIG_BOOT_SIZE,mask_flags:0},
	{ name: "config",offset: 0x1F0000, size: 0x10000,mask_flags:0},
#endif
};
#endif // of CONFIG_E8B

#ifdef CONFIG_SPANSION_16M_FLASH
enum GPIO_DEF {
	GPIO_A_0 = 0, GPIO_A_1, GPIO_A_2, GPIO_A_3, GPIO_A_4, GPIO_A_5, GPIO_A_6, GPIO_A_7, 
	GPIO_B_0 = 8, GPIO_B_1, GPIO_B_2, GPIO_B_3, GPIO_B_4, GPIO_B_5, GPIO_B_6, GPIO_B_7, 
	GPIO_C_0 = 16, GPIO_C_1, GPIO_C_2, GPIO_C_3, GPIO_C_4, GPIO_C_5, GPIO_C_6, GPIO_C_7, 
	GPIO_D_0 = 24, GPIO_D_1, GPIO_D_2, GPIO_D_3, GPIO_D_4, GPIO_D_5, GPIO_D_6, GPIO_D_7
};
/*linux-2.6.19*/
extern void gpioConfig (int gpio_num, int gpio_func);
extern void gpioSet(int gpio_num);
extern void gpioClear(int gpio_num);


unsigned int flash_bank = 0;
u32 rtl8672_flash_bank(u32 address)
{
	u32 val, bank;
	bank = address >> 22;

	//setup GPA driving 4mA
	val = REG32(BSP_MISC_IO_DRIVING);
	REG32(BSP_MISC_IO_DRIVING) =  val|(1 << 12);

	//config gpa5,6
	gpioConfig(GPIO_A_5, 0x0002);	
	gpioConfig(GPIO_A_6, 0x0002);	

	switch (bank){
		case 1:
			gpioSet(GPIO_A_5);
			gpioClear(GPIO_A_6);
			break;
		case 2:
			gpioClear(GPIO_A_5);
			gpioSet(GPIO_A_6);
			break;
		case 3:
			gpioSet(GPIO_A_5);
			gpioSet(GPIO_A_6);
			break;
		case 0:
		default:
			gpioClear(GPIO_A_5);
			gpioClear(GPIO_A_6);
	}	
	

	if (bank != flash_bank)
	{
		//printk("\r\n bank switch :%d --> %d",flash_bank, bank);
		//udelay(5000);//test
		flash_bank = bank ;
	}
	
	return (address & (FLASH_BANK_SIZE-1));
	
}
#endif 



#if LINUX_VERSION_CODE < 0x20212 && defined(MODULE)
#define init_rtl8672_map init_module
#define cleanup_rtl8672_map cleanup_module
#endif

#define mod_init_t  static int __init
#define mod_exit_t  static void __exit

mod_init_t init_rtl8672_map(void)
{
    printk(KERN_NOTICE "flash device: 0x%x at 0x%x\n", WINDOW_SIZE, WINDOW_ADDR);
/*
	rtl8672_map.map_priv_1 =WINDOW_ADDR;

	if (!rtl8672_map.map_priv_1) {
		printk("Failed to ioremap\n");
		return -EIO;
	}
*/
	rtl8672_map.virt = WINDOW_ADDR; //ioremap(rtl8672_map.phys, rtl8672_map.size);
	if (!rtl8672_map.virt) {
		printk(KERN_ERR "Failed to ioremap_nocache\n");
		return -EIO;
		//rc = -EIO;
		//goto err2;
	}

	simple_map_init(&rtl8672_map);

	//printk("probe start %08x\n", rtl8672_map.virt);
	rtl8672_mtd = do_map_probe("cfi_probe", &rtl8672_map);
#ifdef CONFIG_RTL8672_SPI_FLASH
	if (!rtl8672_mtd)
		rtl8672_mtd = do_map_probe("spi_probe", &rtl8672_map);
#endif
	//printk("probe ends\n");
	if (rtl8672_mtd) {
		rtl8672_mtd->owner = THIS_MODULE;
		add_mtd_partitions(rtl8672_mtd, rtl8672_parts, ARRAY_SIZE(rtl8672_parts));
		ROOT_DEV = MKDEV(MTD_BLOCK_MAJOR, 0);
		
		#ifdef CONFIG_HTTP_FILE
		do {
			int idx;
			extern void setup_http_file_flash(u32 flashsize);
			for (idx=0; idx < (sizeof(rtl8672_parts)/sizeof(struct mtd_partition)); idx++) {
				if (strcmp("rootfs",rtl8672_parts[idx].name))
					continue;
				setup_http_file_flash(rtl8672_parts[idx].size);
			}						
		} while (0);
		#endif
		return 0;
	}
/*
	printk("probe start 2\n");
	rtl8672_mtd = do_map_probe("jedec_probe", &rtl8672_map);
	printk("probe ends 2\n");
	if (rtl8672_mtd) {
		rtl8672_mtd->owner = THIS_MODULE;
		add_mtd_partitions(rtl8672_mtd, rtl8672_parts, sizeof(rtl8672_parts)/sizeof(rtl8672_parts[0]));
		return 0;
	}

	printk("probe start 3\n");
	rtl8672_mtd = do_map_probe("map_rom", &rtl8672_map);
	printk("probe ends 3\n");
	if (rtl8672_mtd) {
		rtl8672_mtd->owner = THIS_MODULE;
		add_mtd_partitions(rtl8672_mtd, rtl8672_parts, sizeof(rtl8672_parts)/sizeof(rtl8672_parts[0]));
		return 0;
	}
*/
	iounmap((void *)rtl8672_map.virt);
	return -ENXIO;
}

mod_exit_t cleanup_rtl8672_map(void)
{
	if (rtl8672_mtd) {
		del_mtd_partitions(rtl8672_mtd);
		map_destroy(rtl8672_mtd);
	}
	if (rtl8672_map.virt) {
		iounmap((void *)rtl8672_map.virt);
		rtl8672_map.map_priv_1 = 0;
	}
}

MODULE_LICENSE("GPL");
module_init(init_rtl8672_map);
module_exit(cleanup_rtl8672_map);
