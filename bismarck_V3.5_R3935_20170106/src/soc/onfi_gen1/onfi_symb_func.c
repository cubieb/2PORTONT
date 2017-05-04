#include <soc.h>
#include <onfi/onfi_symb_func.h>
#include <symb_define.h>


#ifndef ALWAYS_RETURN_ZERO
    #define _onfi_dummy_func ALWAYS_RETURN_ZERO
#else
    s32_t _onfi_dummy_func(void) {return 0;}
#endif

ecc_encode_t *_ecc_encode_ptr     SECTION_RECYCLE_DATA = (ecc_encode_t *)_onfi_dummy_func;
ecc_decode_t *_ecc_decode_ptr     SECTION_RECYCLE_DATA = (ecc_decode_t *)_onfi_dummy_func;
ecc_engine_t *_ecc_engine_act_ptr SECTION_RECYCLE_DATA = (ecc_engine_t *)_onfi_dummy_func;
fpv_t        *_opu_reset_ptr      SECTION_RECYCLE_DATA = (fpv_t *)_onfi_dummy_func;

// Should globally exists
onfi_get_sts_reg_t *_opu_get_sts_reg_ptr SECTION_SDATA = (onfi_get_sts_reg_t *)_onfi_dummy_func;
fps32_t            *_opu_chk_program_erase_sts_ptr SECTION_SDATA = (fps32_t *)_onfi_dummy_func;
fpu32_t            *_opu_read_onfi_id SECTION_SDATA = (fpu32_t *)_onfi_dummy_func;
onfi_page_read_write_ecc_t  *_opu_page_read_with_ode_ptr = (onfi_page_read_write_ecc_t *)_onfi_dummy_func;
onfi_page_read_write_ecc_t  *_opu_page_write_with_ode_ptr = (onfi_page_read_write_ecc_t *)_onfi_dummy_func;


symb_retrive_entry_t onfi_func_retrive_list[] SECTION_RECYCLE_DATA = {
    {ONAF_PIO_READ_FUNC,           &(onfi_plr_model_info._pio_read)},
    {ONAF_PIO_WRITE_FUNC,          &(onfi_plr_model_info._pio_write)},
    {ONAF_PAGE_READ_FUNC,          &(onfi_plr_model_info._page_read)},
    {ONAF_PAGE_WRITE_FUNC,         &(onfi_plr_model_info._page_write)},
    {ONAF_PAGE_READ_ECC_FUNC,      &(onfi_plr_model_info._page_read_ecc)},
    {ONAF_PAGE_WRITE_ECC_FUNC,     &(onfi_plr_model_info._page_write_ecc)},
    {ONFI_BLOCK_ERASE_FUNC,        &(onfi_plr_model_info._block_erase)},
    {ONFI_WAIT_NAND_CHIP_FUNC,     &(onfi_plr_model_info._wait_onfi_rdy)},
    {ONFI_RESET_NAND_CHIP_FUNC,    &_opu_reset_ptr},
    {ONFI_GET_STATUS_FUNC,         &_opu_get_sts_reg_ptr},
    {ONFI_CHK_PROG_ERASE_STS_FUNC, &_opu_chk_program_erase_sts_ptr},
    {ONFI_READ_ID_FUNC,            &_opu_read_onfi_id},
    {ECC_BCH_ENCODE_FUNC,          &_ecc_encode_ptr},
    {ECC_BCH_DECODE_FUNC,          &_ecc_decode_ptr},
    {ECC_ENGINE_ACTION_FUNC,       &_ecc_engine_act_ptr},
    {ENDING_SYMB_ID, VZERO},
};

SECTION_RECYCLE void 
onfi_func_symbol_retrive(void)
{
    symb_retrive_list(onfi_func_retrive_list, lplr_symb_list_range);
}

REG_INIT_FUNC(onfi_func_symbol_retrive, 2);

