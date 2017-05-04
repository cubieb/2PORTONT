#ifndef ONFI_UBOOT_UTIL_H
#define ONFI_UBOOT_UTIL_H

#include "conf.h"
#include <onfi/onfi_struct.h>
#define _plr_soc_t                  (*(soc_t *)(OTTO_SRAM_START+OTTO_HEADER_OFFSET))
#define _plr_onfi_info               _plr_soc_t.flash_info.onfi_info


// Preloader Symbol table retrieve definition
#define symb_start(p) p.bios.header.export_symb_list
#define symb_end(p)   p.bios.header.end_of_export_symb_list

#define RETRIEVE_SYMB_TABLE(ptr, symb_key, list, list_end) ({\
    const symbol_table_entry_t *p=list;\
    ptr=VZERO;\
    while(p!=list_end) {\
        if(p->key.id==(u32_t)symb_key){ptr=p->v.pvalue; break;}\
        if(p->key.id==ENDING_SYMB_ID) break;\
        ++p;\
    }\
})


// ONFI flash APIs (Using "user define")
#define ub_pio_read(info, addr, len, blk_pge_addr, col_addr)     info->_model_info->_pio_read(info, addr, len, blk_pge_addr, col_addr)
#define ub_pio_write(info, addr, len, blk_pge_addr, col_addr)    info->_model_info->_pio_write(info, addr, len, blk_pge_addr, col_addr)
#define ub_page_read(info, dma_addr, blk_pge_addr)               info->_model_info->_page_read(info, dma_addr, blk_pge_addr)
#define ub_page_write(info, dma_addr, blk_pge_addr)              info->_model_info->_page_write(info, dma_addr, blk_pge_addr)
#define ub_page_read_ecc(info, dma_addr, blk_pge_addr, eccbuf)   info->_model_info->_page_read_ecc(info, dma_addr, blk_pge_addr, eccbuf)
#define ub_page_write_ecc(info, dma_addr, blk_pge_addr, eccbuf)  info->_model_info->_page_write_ecc(info, dma_addr, blk_pge_addr, eccbuf)
#define ub_block_erase(info, blk_pge_addr)                       info->_model_info->_block_erase(info, blk_pge_addr)


// ONFI flash APIs (Using "_plr_onfi_info")
#define ofu_pio_read(addr, len, blk_pge_addr, col_addr)     ub_pio_read(_plr_onfi_info, addr, len, blk_pge_addr, col_addr)
#define ofu_pio_write(addr, len, blk_pge_addr, col_addr)    ub_pio_write(_plr_onfi_info, addr, len, blk_pge_addr, col_addr)
#define ofu_page_read(dma_addr, blk_pge_addr)               ub_page_read(_plr_onfi_info, dma_addr, blk_pge_addr)
#define ofu_page_write(dma_addr, blk_pge_addr)              ub_page_write(_plr_onfi_info, dma_addr, blk_pge_addr)
#define ofu_page_read_ecc(dma_addr, blk_pge_addr, eccbuf)   ub_page_read_ecc(_plr_onfi_info, dma_addr, blk_pge_addr, eccbuf)
#define ofu_page_write_ecc(dma_addr, blk_pge_addr, eccbuf)  ub_page_write_ecc(_plr_onfi_info, dma_addr, blk_pge_addr, eccbuf)
#define ofu_block_erase(blk_pge_addr)                       ub_block_erase(_plr_onfi_info, blk_pge_addr)

#define ofu_ecc_encode(dma_addr, eccbuf)                    _plr_onfi_info->_ecc_encode(6, dma_addr, eccbuf)
#define ofu_ecc_decode(dma_addr, eccbuf)                    _plr_onfi_info->_ecc_decode(6, dma_addr, eccbuf)
  
// ONFI flash block & page address definition
#define BLOCK_ADDR(block_0_1023) (block_0_1023<<6)
#define BLOCK_PAGE_ADDR(block_0_1023, page_0_63) ((0x00<<16)|(block_0_1023<<6)|(page_0_63))

#endif //ONFI_UBOOT_UTIL_H

