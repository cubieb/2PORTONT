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
#include <linux/mtd/mtd.h>
#include <bspchip.h>

#include "nor_spif_core.h"

extern int mtd_spi_erase_g2(struct mtd_info *mtd, struct erase_info *instr);
extern int mtd_spi_read_g2(struct mtd_info *mtd, loff_t from, size_t len, size_t * retlen, u_char * buf);
extern int mtd_spi_write_g2(struct mtd_info *mtd, loff_t to, size_t len, size_t * retlen, const u_char * buf);
extern void mtd_spi_sync_g2(struct mtd_info *mtd);
extern int mtd_spi_lock_g2(struct mtd_info *mtd, loff_t ofs, uint64_t len);
extern int mtd_spi_unlock_g2(struct mtd_info *mtd, loff_t ofs, uint64_t len);
extern int mtd_spi_suspend_g2(struct mtd_info *mtd);
extern void mtd_spi_resume_g2(struct mtd_info *mtd);

void nor_spif_detect(void);

extern plr_nor_spi_info_g2_t nor_spif_info;

extern nor_spi_probe_t nor_spif_common_probe;
u32_t wr_boundary_g2;

struct mtd_info *spi_probe_g2(struct map_info *map) {
	struct mtd_info *mtd;

	nor_spif_detect();
	map->size = nor_spif_info.size_per_chip_b;
	wr_boundary_g2 = nor_spif_info.wr_boundary_b;

	mtd = kmalloc(sizeof(*mtd), GFP_KERNEL);
	if (!mtd) {
		printk(KERN_WARNING "Failed to allocate memory for MTD device\n");
		return NULL;
	}
	memset(mtd, 0, sizeof(struct mtd_info));
	mtd->type  = MTD_OTHER;
	mtd->flags = MTD_CAP_NORFLASH;
	mtd->name  = map->name;
	mtd->size  = map->size;
	mtd->erasesize = nor_spif_info.erase_unit_b;
	mtd->erase   = mtd_spi_erase_g2;
	mtd->read    = mtd_spi_read_g2;
	mtd->write   = mtd_spi_write_g2;
	mtd->sync    = mtd_spi_sync_g2;
	mtd->lock    = mtd_spi_lock_g2;
	mtd->unlock  = mtd_spi_unlock_g2;
	mtd->suspend = mtd_spi_suspend_g2;
	mtd->resume  = mtd_spi_resume_g2;
	mtd->priv    = (void *)map;

	/* printk(KERN_WARNING "DD: mtd->size: %lld\n", mtd->size); */
	/* printk(KERN_WARNING "DD: mtd->erase_size: %d\n", mtd->erasesize); */

	mtd->writesize = 1;
	
	return mtd;
}
