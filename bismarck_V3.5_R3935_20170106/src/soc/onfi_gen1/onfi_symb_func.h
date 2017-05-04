#ifndef ONFI_SYMB_H
#define ONFI_SYMB_H

#include <onfi/onfi_struct.h>

/* Extern Function Pointer Prototype */
extern onfi_model_info_t  onfi_plr_model_info;
extern onfi_info_t  onfi_plr_info;

extern ecc_encode_t  *_ecc_encode_ptr;
extern ecc_decode_t  *_ecc_decode_ptr;
extern ecc_engine_t  *_ecc_engine_act_ptr;

extern onfi_get_sts_reg_t  *_opu_get_sts_reg_ptr;
extern fps32_t             *_opu_chk_program_erase_sts_ptr;
extern fpv_t               *_opu_reset_ptr;
extern fpu32_t             *_opu_read_onfi_id;
extern onfi_page_read_write_ecc_t  *_opu_page_read_with_ode_ptr;
extern onfi_page_read_write_ecc_t  *_opu_page_write_with_ode_ptr;

    
#define opu_pio_read(info, wr_buf, len, blk_page_idx, col_addr)     _onfi_info->_model_info->_pio_read(info, wr_buf, len, blk_page_idx, col_addr)
#define opu_pio_write(info, wr_buf, len, blk_page_idx, col_addr)    _onfi_info->_model_info->_pio_write(info, wr_buf, len, blk_page_idx, col_addr)
#define opu_page_read(info, wr_buf, blk_page_idx)                   _onfi_info->_model_info->_page_read(info, wr_buf, blk_page_idx)
#define opu_page_write(info, wr_buf, blk_page_idx)                  _onfi_info->_model_info->_page_write(info, wr_buf, blk_page_idx)
#define opu_page_read_ecc(info, wr_buf, blk_page_idx, eccbuf)       _onfi_info->_model_info->_page_read_ecc(info, wr_buf, blk_page_idx, eccbuf)
#define opu_page_write_ecc(info, wr_buf, blk_page_idx, eccbuf)      _onfi_info->_model_info->_page_write_ecc(info, wr_buf, blk_page_idx, eccbuf)
#define opu_block_erase(info, blk_page_idx)                         _onfi_info->_model_info->_block_erase(info, blk_page_idx)
#define opu_wait_nand_spi_rdy()                                     _onfi_info->_wait_onfi_rdy()
#define opu_ecc_encode(is_bch12, dma_addr, eccbuf)                  _onfi_info->_ecc_encode(is_bch12, dma_addr, eccbuf)
#define opu_ecc_decode(is_bch12, dma_addr, eccbuf)                  _onfi_info->_ecc_decode(is_bch12, dma_addr, eccbuf)
#define opu_ecc_engine_act(is_bch12, dma_addr, eccbuf,is_encode)    (*_ecc_engine_act_ptr)(is_bch12, dma_addr, eccbuf,is_encode)
#define opu_reset_spi_nand_chip()                                   _onfi_info->_reset()
#define opu_get_status_reg(void)                                    (*_opu_get_sts_reg_ptr)()
#define opu_read_onfi_id(void)                                      (*_opu_read_onfi_id)()

#endif //ONFI_SYMB_H

