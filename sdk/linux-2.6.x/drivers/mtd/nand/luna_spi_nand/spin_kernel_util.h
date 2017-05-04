

#ifndef SPIN_KERNEL_UTIL_H__
#define SPIN_KERNEL_UTIL_H__

#ifndef inline_memcpy
#define inline_memcpy(d, s, l) ({char *__d=(char*)d;const char *__s=(const char *)s;unsigned __l=l;while (__l-- >0) *(__d++)=*(__s++);})  
#endif

#ifndef inline_memset
#define inline_memset(d, v, l) ({char *__d=(char*)d; char __v=(char)v; unsigned __l=l; while (__l-- >0) *(__d++)=__v;})
#endif


extern struct spi_nand_flash_info_s *_spi_nand_info;        


#define DCACHE_AAA
#ifdef DCACHE_AAA
#include <asm/io.h>
#define DMA_CACHE_WBACK_INV(start_addr, end_addr) dma_cache_wback_inv((start_addr), (end_addr)-(start_addr))
//#define _lplr_basic_io.dcache_writeback_invalidate_range(start_addr, end_addr) dma_cache_wback_inv((start_addr), (end_addr)-(start_addr))

#endif

#ifndef  dcache_wr_inv
#define dcache_wr_inv(start_addr, end_addr) dma_cache_wback_inv((start_addr), (end_addr)-(start_addr))
#endif

#define nsu_reset_spi_nand_chip nsc_reset_spi_nand_chip
#define nsu_get_feature_reg nsc_get_feature_register
#define nsu_set_feature_reg nsc_set_feature_register
#define nsu_read_spi_nand_id nsc_read_spi_nand_id
#define nsu_page_read(info, dma_addr, blk_pge_addr)               _spi_nand_info->_model_info->_page_read(info, dma_addr, blk_pge_addr)
#define nsu_page_read_ecc(info, dma_addr, blk_pge_addr, eccbuf)   _spi_nand_info->_model_info->_page_read_ecc(info, dma_addr, blk_pge_addr, eccbuf)
#define nsu_page_write(info, dma_addr, blk_pge_addr)              _spi_nand_info->_model_info->_page_write(info, dma_addr, blk_pge_addr)
#define nsu_page_write_ecc(info, dma_addr, blk_pge_addr, eccbuf)  _spi_nand_info->_model_info->_page_write_ecc(info, dma_addr, blk_pge_addr, eccbuf)


#define symb_pdefine(...)
#define symb_fdefine(...)

#ifndef SECTION_NAND_SPIF_PROBE_FUNC
#define SECTION_NAND_SPIF_PROBE_FUNC __attribute__ ((section (".nand_spif_probe_func")))
#endif

#define REG_SPI_NAND_PROBE_FUNC(fn) spi_nand_probe_t* __nspf_ ## ## fn ## _ \
        SECTION_NAND_SPIF_PROBE_FUNC = (spi_nand_probe_t*) fn


#define NOTALIGNED(mtd, x) ((x & (mtd->writesize-1)) != 0)   //czyao, 2010/0917


#define BLOCK_ADDR(block_0_1023) (block_0_1023<<6)
#define BLOCK_PAGE_ADDR(block_0_1023, page_0_63) ((0x00<<16)|(block_0_1023<<6)|(page_0_63))

#ifndef bzero
#define bzero(p,n) memset(p, '\0', n)
#endif


#define SECTION_SDATA 
#define SECTION_UNS_TEXT
#define SECTION_RECYCLE

#define REG_INIT_FUNC(a, b)

#define NSU_DRIVER_IN_ROM 1

#endif // end SPIN_KERNEL_UTIL_H__

