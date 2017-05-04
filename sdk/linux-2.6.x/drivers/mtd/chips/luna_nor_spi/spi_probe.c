/**
 *  SPI Flash probe code.
 *  (C) 2006 Atmark Techno, Inc.
 */  
    
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/config.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/delay.h>
    
#include <linux/mtd/map.h>
//#include <linux/mtd/gen_probe.h>
    
#include <linux/mtd/mtd.h>
#include <bspchip.h>
#include "spi_flash.h"

extern int mtd_spi_erase(struct mtd_info *mtd, struct erase_info *instr);
extern int mtd_spi_read(struct mtd_info *mtd, loff_t from, size_t len, size_t * retlen, u_char * buf);
extern int mtd_spi_write(struct mtd_info *mtd, loff_t to, size_t len, size_t * retlen, const u_char * buf);
extern void mtd_spi_sync(struct mtd_info *mtd);
extern int mtd_spi_lock(struct mtd_info *mtd, loff_t ofs, uint64_t len);
extern int mtd_spi_unlock(struct mtd_info *mtd, loff_t ofs, uint64_t len);
extern int mtd_spi_suspend(struct mtd_info *mtd);
extern void mtd_spi_resume(struct mtd_info *mtd);

static struct mtd_info *spi_chip_setup(struct map_info *map);
static struct mtd_info *spi_probe(struct map_info *map);
static void spi_destroy(struct mtd_info *mtd);


static struct mtd_chip_driver spi_chipdrv = { 
	probe: spi_probe, 
	destroy: spi_destroy, 
	name: "spi_probe", 
	module:THIS_MODULE, 
};

static struct mtd_info *spi_chip_setup(struct map_info *map) 
{
	struct mtd_info *mtd;

	mtd = kmalloc(sizeof(*mtd), GFP_KERNEL);
	if (!mtd) {
		printk(KERN_WARNING "Failed to allocate memory for MTD device\n");
		return NULL;
	}
	memset(mtd, 0, sizeof(struct mtd_info));
	mtd->type  = MTD_OTHER;
	mtd->flags = MTD_CAP_NORFLASH;
	mtd->name  = map->name;
	
	mtd->size = map->size;
	mtd->erasesize = (0x1 << para_flash_info.erase_unit);
	KDEBUG("%s: mtd->size=0x%08llx, mtd->erasesize=0x%08x\n", __FUNCTION__, mtd->size, (unsigned int)mtd->erasesize);

	mtd->erase   = mtd_spi_erase;
	mtd->read    = mtd_spi_read;
	mtd->write   = mtd_spi_write;
	mtd->sync    = mtd_spi_sync;
	mtd->lock    = mtd_spi_lock;
	mtd->unlock  = mtd_spi_unlock;
	mtd->suspend = mtd_spi_suspend;
	mtd->resume  = mtd_spi_resume;
	mtd->priv    = (void *)map;

	map->fldrv = &spi_chipdrv;
	
	mtd->writesize = 1;
	
	//MOD_INC_USE_COUNT;
	
	return mtd;
}

#ifndef CONFIG_USE_PRELOADER_PARAMETERS
plr_flash_info_t flash_param_db[] __initdata = {
#ifdef CONFIG_LUNA_NOR_SPI_SIO_READ
	{
	/* !!!ATTENTION!!!
         * spi_flash_MXIC_MX25L25635F_SIO.h and spi_flash_MXIC_MX25L25735E_SIO.h can be included EXCLUSIVELY
         * as having the same chip ID. */
	#include <param_db/spi_flash_MXIC_MX25L25635F_SIO.h>
	//#include <param_db/spi_flash_MXIC_MX25L25735E_SIO.h>
	}, {
	#include <param_db/spi_flash_GENERIC.h>
	}
#else
	{
	/* !!!ATTENTION!!!
         * spi_flash_MXIC_MX25L25635F.h and spi_flash_MXIC_MX25L25735E.h can be included EXCLUSIVELY
         * as having the same chip ID. */
	#include <param_db/spi_flash_MXIC_MX25L25635F.h>
	//#include <param_db/spi_flash_MXIC_MX25L25735E.h>
	}, {
	#include <param_db/spi_flash_MXIC_MX25L12845E.h>
	}, {
	#include <param_db/spi_flash_EON_EN25Q64.h>
	}, {
	#include <param_db/spi_flash_WINBOND_W25Q32BV.h>
	}, {
	#include <param_db/spi_flash_WINBOND_W25Q128FV.h>
	}, {
	#include <param_db/spi_flash_GENERIC.h>
	}
#endif //#ifdef CONFIG_LUNA_NOR_SPI_SIO_READ
};

plr_flash_info_t __init *get_flash_spare(u32_t *flash_db_num)
{
	*flash_db_num = ARRAY_SIZE(flash_param_db);
	return flash_param_db;
}
#endif /*CONFIG_USER_PRELOADER_PARAMETERS*/

extern void flash_init(void);  //@plr_spi_nor_flash.c
extern struct mtd_info *spi_probe_g2(struct map_info *map);

struct mtd_info *spi_probe(struct map_info *map) 
{
	struct mtd_info *mtd = NULL;
	u32_t SOC_ID;

	WRITE_MEM32(CHIP_ID_REG, 0xa0000000);
	SOC_ID=(REG32(CHIP_ID_REG)& 0xffff);

	if ((SOC_ID == 0x6422) ||
	    (SOC_ID == 0x0639)) {
		printk("luna SPI FLASH G2 driver version 1.00-");
		mtd = spi_probe_g2(map);
		if (mtd) {
			map->fldrv = &spi_chipdrv;
		}
	} else {
#ifdef CONFIG_USE_PRELOADER_PARAMETERS
		printk("SPI FLASH parameters obtained from preloader!\n");
#endif /*CONFIG_USER_PRELOADER_PARAMETERS*/
		
		printk("luna SPI FLASH driver version 1.00-");
		parameters.flash_init_result = INI_RES_UNINIT;
		flash_init();
		map->size = otto_get_flash_size();
		mtd = spi_chip_setup(map);
	}

	return mtd;
}

static void spi_destroy(struct mtd_info *mtd) 
{
	printk("%s\n", __FUNCTION__);
}

int __init spi_probe_init(void) 
{
	register_mtd_chip_driver(&spi_chipdrv);
	return 0;
}

void __exit spi_probe_exit(void) 
{
	unregister_mtd_chip_driver(&spi_chipdrv);
} 

module_init(spi_probe_init);
module_exit(spi_probe_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Masahiro Nakai <nakai@atmark-techno.com> et al.");
MODULE_DESCRIPTION("Probe code for SPI flash chips");

