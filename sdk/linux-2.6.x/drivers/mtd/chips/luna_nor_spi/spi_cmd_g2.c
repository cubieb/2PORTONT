/**
 *  SPI Flash common control code.
 *  (C) 2006 Atmark Techno, Inc.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/config.h>
#include <linux/module.h>
#include <linux/types.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/gen_probe.h>

#include <bspchip.h>
#include <kernel_soc.h>

extern s32_t flash_unit_erase_g2(const u32_t, u32_t);
extern s32_t flash_unit_write_g2(const u32_t, u32_t, const u32_t, const void*);
extern s32_t flash_read_g2(const u32_t, u32_t, const u32_t, void*);

static spinlock_t spi_lock = SPIN_LOCK_UNLOCKED;

int mtd_spi_erase_g2(struct mtd_info *mtd, struct erase_info *instr)
{
	unsigned long adr, len;
	unsigned long cs, spc, cadr;
	int ret = 0;
	unsigned long  flags;
	//if (!chip_info->erase)
	//	return -EOPNOTSUPP;

/* Remove 1st block protect so bootloader partition can be updated correctly */
#if 0
	// skip 1st block erase
	if (instr->addr < (mtd->erasesize)) {
		instr->state = MTD_ERASE_DONE;
		return 0;
	}
#endif
	if (instr->addr & (mtd->erasesize - 1))
		return -EINVAL;

/*
	if (instr->len & (mtd->erasesize -1))
		return -EINVAL;
*/
	if ((instr->len + instr->addr) > mtd->size)
		return -EINVAL;
	adr = instr->addr;
	len = instr->len;
	if (len & (mtd->erasesize - 1)) {
		len = len - (len & (mtd->erasesize - 1)) + mtd->erasesize;
	}
	if (len < mtd->erasesize)
		len = mtd->erasesize;

	spc = mtd->size;

	spin_lock_irqsave(&spi_lock, flags);
	while (len) {
		cs = adr/spc;
		cadr = adr%spc;
		ret = flash_unit_erase_g2(cs, cadr);
		/* printk(KERN_WARNING "DD: NORSPIF G2 Erase: (0x%p).\n", (void *)(cadr));  */
		if (ret) {
			spin_unlock_irqrestore(&spi_lock, flags);
			return ret;
		}
		adr += mtd->erasesize;
		len -= mtd->erasesize;
	}
	spin_unlock_irqrestore(&spi_lock, flags);

	instr->state = MTD_ERASE_DONE;
	if (instr->callback)
		instr->callback(instr);


	return 0;
}

static u32 spi_copy_to_dram(const u32 from, const u32 to, const u32 size)
{
	memcpy((void *)to, (void *)(from + SPI_NOR_FLASH_START_ADDR), size);

	return 0;
}

int mtd_spi_read_g2(struct mtd_info *mtd, loff_t from, size_t len, size_t * retlen, u_char * buf)
{
	int ret = 0;
	unsigned long flags;
	u32 cs, spc, pio_from, pio_len, pio_iter_len;

	spin_lock_irqsave(&spi_lock, flags);
	if((from + len) <= SPI_NOR_FLASH_MMIO_SIZE) {
		/* Use MMIO read */
		ret = spi_copy_to_dram(from, (u32)buf, len);
	} else {
		/* Use PIO read */
		pio_from = from;
		pio_len = len;
		pio_iter_len = 0;
		spc = mtd->size;
		cs = pio_from/spc;
		pio_from = pio_from%spc;
		while(pio_len) {
			if( (pio_from + pio_len) > spc) {
				pio_iter_len = spc - pio_from;	
				ret = flash_read_g2(cs, pio_from, pio_iter_len, buf);
				/* printk(KERN_WARNING "DD: NORSPIF G2 Read: (0x%p).\n", (void *)(pio_from));  */
				cs++;
				pio_from = 0;
				buf += pio_iter_len;
				pio_len -= pio_iter_len; 
			} else {
				ret = flash_read_g2(cs, pio_from, pio_len, buf);
				break;
			}
		}
	}
	spin_unlock_irqrestore(&spi_lock, flags);

	if (ret)
		return ret;
	if (retlen)
		(*retlen) = len;
	return 0;
}

#define IS_OVERLAP(a, b, c, d) ((b>c) && (a<d))
extern u32_t wr_boundary_g2;
int mtd_spi_write_g2(struct mtd_info *mtd, loff_t to, size_t len,
		  size_t * retlen, const u_char * buf)
{
	int ret = 0;
	unsigned long flags;
	unsigned int remaining_size;
	unsigned int cur_addr;
	void *dp;/* Current data pointer */

	remaining_size = len;
	cur_addr = to;
	dp = (void *)buf;
	if(IS_OVERLAP(CADDR(buf), CADDR(buf)+len, SPI_NOR_FLASH_MMIO_BASE0, SPI_NOR_FLASH_MMIO_BASE0+SPI_NOR_FLASH_MMIO_SIZE0) ||
	   IS_OVERLAP(CADDR(buf), CADDR(buf)+len, SPI_NOR_FLASH_MMIO_BASE1, SPI_NOR_FLASH_MMIO_BASE1+SPI_NOR_FLASH_MMIO_SIZE1) ) { 
		printk(KERN_ERR "ERROR: SPI Write from MMIO range(0x%p).\n", (void *)(CADDR(buf) + len)); 
		return -EINVAL;
	}
	spin_lock_irqsave(&spi_lock, flags);
	while(remaining_size>0) {
		unsigned int iter_size;
		unsigned int cs, spc;
		
		iter_size = (wr_boundary_g2-1);
		iter_size &= cur_addr;
		iter_size = wr_boundary_g2 - iter_size;
		if(iter_size > remaining_size) {
			iter_size = remaining_size;
		}
		spc = mtd->size;
		cs = cur_addr/spc;
		ret = flash_unit_write_g2(cs, (cur_addr%spc), iter_size, dp); 
		/* printk(KERN_WARNING "DD: NORSPIF G2 Write: 0x%p, %dB.\n", (void *)(cur_addr), iter_size);  */
		if(ret) {
			printk("Warning: failed to write flash at 0x%08x with error code=%d\n", cur_addr, ret);
			spin_unlock_irqrestore(&spi_lock, flags);
			return ret;
		} else {
			cur_addr += iter_size;
			remaining_size -= iter_size;
			dp += iter_size;
		}
	}
	spin_unlock_irqrestore(&spi_lock, flags);
	if (ret)
		return ret;
	if (retlen)
		(*retlen) = len;
	return 0;
}

void mtd_spi_sync_g2(struct mtd_info *mtd)
{
	/* Operation not supported on transport endpoint */
} 
int mtd_spi_lock_g2(struct mtd_info *mtd, loff_t ofs, uint64_t len)
{
	/* Operation not supported on transport endpoint */
	return -EOPNOTSUPP;
}

int mtd_spi_unlock_g2(struct mtd_info *mtd, loff_t ofs, uint64_t len)
{
	/* Operation not supported on transport endpoint */
	return -EOPNOTSUPP;
}

int mtd_spi_suspend_g2(struct mtd_info *mtd)
{
	/* Operation not supported on transport endpoint */
	return -EOPNOTSUPP;
}

void mtd_spi_resume_g2(struct mtd_info *mtd)
{
	/* Operation not supported on transport endpoint */
} 

EXPORT_SYMBOL(mtd_spi_erase_g2);
EXPORT_SYMBOL(mtd_spi_read_g2);
EXPORT_SYMBOL(mtd_spi_write_g2);
EXPORT_SYMBOL(mtd_spi_sync_g2);
EXPORT_SYMBOL(mtd_spi_lock_g2);
EXPORT_SYMBOL(mtd_spi_unlock_g2);
EXPORT_SYMBOL(mtd_spi_suspend_g2);
EXPORT_SYMBOL(mtd_spi_resume_g2);
