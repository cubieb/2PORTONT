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
/*linux-2.6.19*/
#include <linux/version.h>
//ql
//#include "../../../../config/autoconf.h"

#include <bspchip.h>
#include <kernel_soc.h>

#define BUSWIDTH 2

static struct mtd_info *luna_nor_spi_mtd = NULL;

struct map_info luna_nor_spi_map = {
	name:	RTK_MTD_DEV_NAME,
	// size:	WINDOW_SIZE,//Filled after probing spi nor flash
	bankwidth:BUSWIDTH,
	phys:	SPI_NOR_FLASH_START_ADDR
};

#define BOOT_OFFSET   (0)
#define BOOT_SIZE     (208*1024)

#ifdef CONFIG_DEFAULTS_REALTEK_RTL8670
#define HS_OFFSET     (BOOT_OFFSET + BOOT_SIZE)
#define HS_SIZE       (4*1024)
#define CS_OFFSET     (HS_OFFSET + HS_SIZE)
#define CS_SIZE       (128*1024)
#define CONFIG_OFFSET (CS_OFFSET + CS_SIZE)
#define CONFIG_SIZE   (204*1024)
#else //CONFIG_DEFAULTS_REALTEK_LUNA
#define CONFIG_OFFSET (BOOT_OFFSET + BOOT_SIZE)
#define CONFIG_SIZE   (336*1024)
#endif //#ifdef CONFIG_DEFAULTS_REALTEK_RTL8670

#define LINUX_OFFSET  (CONFIG_OFFSET + CONFIG_SIZE)
#define LINUX_SIZE    (2*1024*1024)
#define ROOTFS_OFFSET (LINUX_OFFSET + LINUX_SIZE)
#define ROOTFS_SIZE   (5600*1024)
static struct mtd_partition luna_nor_spi_parts[] = {
	{name: "boot",   offset: BOOT_OFFSET,   size: BOOT_SIZE,   mask_flags:0}, 
#ifdef CONFIG_DEFAULTS_REALTEK_RTL8670
	{name: "HS",     offset: HS_OFFSET,     size: HS_SIZE,     mask_flags:0}, 
	{name: "CS",     offset: CS_OFFSET,     size: CS_SIZE,     mask_flags:0}, 
#endif //#ifdef CONFIG_DEFAULTS_REALTEK_RTL8670
	{name: "config", offset: CONFIG_OFFSET, size: CONFIG_SIZE, mask_flags:0}, 
	{name: "linux",  offset: LINUX_OFFSET,  size: LINUX_SIZE,  mask_flags:0}, 
	{name: "rootfs", offset: ROOTFS_OFFSET, size: ROOTFS_SIZE, mask_flags:0},
};


typedef enum {
	SPI_T        = 0x0,
	I2C_EEPROM_T = 0x1,
	NAND_T       = 0x2
} FLASH_TYPE_T;

#define DELIMITER_LINE "===============================================================================\n" 
__init static int init_luna_nor_spi_map(void)
{
	unsigned int mcr = REG32(BSP_MC_MCR);

	printk(KERN_NOTICE DELIMITER_LINE);
	printk(KERN_NOTICE "%s: flash map at 0x%x\n", __FUNCTION__, (u32)luna_nor_spi_map.phys);

	luna_nor_spi_map.virt = (void *)SPI_NOR_FLASH_START_ADDR;
	if (!luna_nor_spi_map.virt) {
		printk(KERN_ERR "Failed to ioremap_nocache\n");
		return -EIO;
		//rc = -EIO;
		//goto err2;
	}

	simple_map_init(&luna_nor_spi_map);

	/* We only support SPI NOR FLASH */
	if (!luna_nor_spi_mtd && ((mcr & BSP_BOOT_FLASH_STS) == SPI_T)) {
		printk("luna spi probe...\n");
		luna_nor_spi_mtd = do_map_probe("spi_probe", &luna_nor_spi_map);

		//printk("probe ends\n");
		if (luna_nor_spi_mtd) {
			struct  mtd_partition *parts;
			int nr_parts = 0;
			const char*part_probes[] = {"cmdlinepart", NULL,};

			nr_parts = parse_mtd_partitions(luna_nor_spi_mtd, part_probes, &parts, 0);

			printk("add luna nor spi partition\n");
			luna_nor_spi_mtd->owner = THIS_MODULE;
			if(nr_parts <= 0) {
				printk("MTD partitions obtained from built-in array\n");
				add_mtd_partitions(luna_nor_spi_mtd, luna_nor_spi_parts,
						   ARRAY_SIZE(luna_nor_spi_parts));
			} else {
				printk("MTD partitions obtained from kernel command line\n");
				add_mtd_partitions(luna_nor_spi_mtd, parts, nr_parts);
			}
			ROOT_DEV = MKDEV(MTD_BLOCK_MAJOR, 0);

			printk(KERN_NOTICE DELIMITER_LINE);
			return 0;
		}
		printk("ERROR: luna nor spi partition invalid\n");
	} else {
		printk("%s: probe failed! mcr=0x%08x\n", __func__, mcr);
	}

	iounmap((void *)luna_nor_spi_map.virt);
	printk(KERN_NOTICE DELIMITER_LINE);
	return -ENXIO;
}

__exit static void cleanup_luna_nor_spi_map(void)
{
	if (luna_nor_spi_mtd) {
		del_mtd_partitions(luna_nor_spi_mtd);
		map_destroy(luna_nor_spi_mtd);
	}
	if (luna_nor_spi_map.virt) {
		iounmap((void *)luna_nor_spi_map.virt);
		luna_nor_spi_map.map_priv_1 = 0;
	}
}

MODULE_LICENSE("GPL");
late_initcall(init_luna_nor_spi_map);//Postpone init_luna_nor_spi_map() as it required spi_prboe() to be ready which is prepared at module_init()
//module_init(init_luna_nor_spi_map);
module_exit(cleanup_luna_nor_spi_map);
