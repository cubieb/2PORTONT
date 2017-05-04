#include <util.h>
#include <ecc/ecc_ctrl.h>
#include <spi_nand/spi_nand_ctrl.h>
#include <spi_nand/spi_nand_common.h>
#include <onfi/onfi_ctrl.h>
#include <lib/misc/string.h>
#include "register_map.h"

#if MAX_PLR_FIRST_LOAD_PAGE > 32
    #error MAX PLR_FIRST_LOAD_PAGE should not be over 32
#endif

extern const init_table_entry_t start_of_init_func_table, end_of_init_func_table;
extern const spi_nand_probe_t *start_of_spi_nand_probe_func;
extern const spi_nand_probe_t *end_of_spi_nand_probe_func;
extern const onfi_probe_t *start_of_onfi_probe_func;
extern const onfi_probe_t *end_of_onfi_probe_func;

#define lplr_page_buffer    ((char*)OTTO_LPLR_SPI_NAND_PAGE_BUFFER)


static inline UTIL_MIPS16 int
counting_one(char byte1, char byte2) {
    int c=0, b1=byte1, b2=byte2;
    int i=0x80;
    while (i!=0) {
        c += ((i&b1)!=0) + ((i&b2)!=0);
        i>>=1;
    }
    return c;
}
static UTIL_MIPS16 int
is_bch12_encoded(plr_first_load_layout_t *buf) {
    char *p=buf->syndrome0 + BCH6_SYNDROME_SIZE;
    char *q=buf->syndrome1 + BCH6_SYNDROME_SIZE;
    int ones=0;
    int size_diff=(BCH12_SYNDROME_SIZE - BCH6_SYNDROME_SIZE);
    int c=size_diff;

    for (;c>0;--c) {
        ones+=counting_one(*(p++), *(q++));
    }
    return ones<(size_diff*2*8-12);
}

// the snaf_info is used only for loading first-load
SECTION_RO
spi_nand_flash_info_t first_load_snaf_info = {
    .man_id                 = 0,            // won't be used
    .dev_id                 = 0,            // won't be used
    ._num_block             = SNAF_MODEL_NUM_BLK_1024,
    ._num_page_per_block    = SNAF_MODEL_NUM_PAGE_64,
    ._page_size             = SNAF_MODEL_PAGE_SIZE_2048B,
    ._spare_size            = SNAF_MODEL_SPARE_SIZE_64B,
    ._oob_size              = SNAF_MODEL_OOB_SIZE(24),
    ._ecc_ability           = ECC_MODEL_6T,
    ._ecc_encode            = VZERO,        // won't be used
    ._ecc_decode            = VZERO,        // won't be used
    ._reset                 = VZERO,        // won't be used
    ._cmd_info              = &nsc_sio_cmd_info,
    ._model_info            = &snaf_rom_general_model,
};

static UTIL_MIPS16 int
plr_first_load_to_sram(u32_t is_spi_nand, void *fl_info_ptr) {
    u32_t blk_page_idx=0;
    s32_t num_page_to_copy=-1;
    u32_t page_log=0;
    u32_t bch_ability;
    plr_first_load_layout_t *plr_first_load_buf=(plr_first_load_layout_t *)lplr_page_buffer;
    typedef void (func_nand_page_rd_t)(void *, void *, u32_t);
    func_nand_page_rd_t * nand_page_read;

    if(is_spi_nand){
        nand_page_read = (func_nand_page_rd_t *)snaf_page_read;
    }else{
        nand_page_read = (func_nand_page_rd_t *)onaf_page_read;
    }

    while (blk_page_idx<MAX_PLR_FIRST_LOAD_PAGE_NUM) {
        nand_page_read(fl_info_ptr, plr_first_load_buf, blk_page_idx++);

        bch_ability = (1==is_bch12_encoded((plr_first_load_layout_t *)UADDR(plr_first_load_buf)))?12:6;
        if (ecc_engine_action(bch_ability, plr_first_load_buf->data0, plr_first_load_buf->oob0, 0)==ECC_CTRL_ERR) continue;
        if (ecc_engine_action(bch_ability, plr_first_load_buf->data1, plr_first_load_buf->oob1, 0)==ECC_CTRL_ERR) continue;

        // check signature
        if (PLR_FL_GET_SIGNATURE(plr_first_load_buf)!=SIGNATURE_PLR_FL) continue;

        // set num_page_to_copy
        if (num_page_to_copy<0) {
            num_page_to_copy=PLR_FL_GET_TOTAL_NUM_PAGE(plr_first_load_buf);
        }

        // copy and log page
        u32_t index=PLR_FL_GET_PAGE_INDEX(plr_first_load_buf);
        if (index>=MAX_PLR_FIRST_LOAD_PAGE) continue;   // unreasonable page index

        u32_t log_index=1<<index;
        if ((page_log&log_index)==0) {
            page_log |= log_index;
            char *plr_buf=(char*)(OTTO_SRAM_START+(index*PLR_FL_PAGE_USAGE));
            inline_memcpy(plr_buf, plr_first_load_buf->data0, BCH_SECTOR_SIZE);
            inline_memcpy(plr_buf+BCH_SECTOR_SIZE, plr_first_load_buf->data1, BCH_SECTOR_SIZE);
            if(0 ==(--num_page_to_copy)) break;
        }
    }
    _lplr_basic_io.icache_invalidate_all();
    _lplr_basic_io.dcache_writeback_invalidate_all();

    return (num_page_to_copy==0)?0:(-1);
}


// export the list of vendor probe functions to preloader
symb_pdefine(spi_nand_device_probe_list_start, SPI_NAND_DEVICE_PROB_LIST_START, &start_of_spi_nand_probe_func);
symb_pdefine(spi_nand_device_probe_list_end, SPI_NAND_DEVICE_PROB_LIST_END, &end_of_spi_nand_probe_func);
symb_pdefine(onfi_device_probe_list_start, ONFI_DEVICE_PROB_LIST_START, &start_of_onfi_probe_func);
symb_pdefine(onfi_device_probe_list_end, ONFI_DEVICE_PROB_LIST_END, &end_of_onfi_probe_func);


void UTIL_MIPS16
lplr_c_entry(void) {
    onfi_info_t first_load_onaf_info;
    void *fl_info_ptr = VZERO;
    u32_t is_spi_nand = (BOOT_ROM_SPI_NAND ==RFLD_MCR(boot_sel))?1:0;

    if(is_spi_nand){
        //Step0: Assign flash_first_load_info
        fl_info_ptr = (void *)(&first_load_snaf_info);
    }else{
        //Step0: Assign flash_first_load_info
        fl_info_ptr = (void *)(&first_load_onaf_info);

        //Step1: Read address_cycles & page_size from strap pin
        u32_t pin_nafc_rc = RFLD_NACFR(nafc_rc);
        u32_t pin_nafc_ac = RFLD_NACFR(nafc_ac);
        first_load_onaf_info.bs_page_size  = ONFI_BS_PAGE_SIZE(pin_nafc_rc);
        first_load_onaf_info.bs_addr_cycle = ONFI_BS_ADDR_CYCLE(pin_nafc_ac);
        first_load_onaf_info.bs_cmd_cycle  = ONFI_BS_CMD_CYCLE(pin_nafc_rc);

        if(2048 == first_load_onaf_info.bs_page_size){
            first_load_onaf_info._page_size   = ONFI_MODEL_PAGE_SIZE_2048B;
            first_load_onaf_info._spare_size  = ONFI_MODEL_SPARE_SIZE_64B;
        }else if(4096 == first_load_onaf_info.bs_page_size){
            first_load_onaf_info._page_size   = ONFI_MODEL_PAGE_SIZE_4096B;
            first_load_onaf_info._spare_size  = ONFI_MODEL_SPARE_SIZE_128B;
        }else if(8192 == first_load_onaf_info.bs_page_size){
            while(1); //Do not support this page size
        }

        //Step2: Doing ONFI controller Init
        onaf_ctrl_init();
    }

    //Init. timer for udelay
    otto_lx_timer_init(TIMER_FREQ_MHZ);

    //Set Pin-Mux for SPI NAND or ONFI IO Interface
    RMOD_IO_MODE_EN(spi_en, is_spi_nand);

    if(plr_first_load_to_sram(is_spi_nand, fl_info_ptr) == -1) while(1);

    _soc.bios.dcache_writeback_invalidate_range=_lplr_basic_io.dcache_writeback_invalidate_range;
    _soc.bios.dcache_writeback_invalidate_all=_lplr_basic_io.dcache_writeback_invalidate_all;
    _soc.bios.icache_invalidate_range=_lplr_basic_io.icache_invalidate_range;
    _soc.bios.icache_invalidate_all=_lplr_basic_io.icache_invalidate_all;
}

void UTIL_MIPS16
run_init_level(const init_table_entry_t *lplr_init_table_start, const init_table_entry_t *lplr_init_table_end,
    const init_table_entry_t *plr_init_table_start, const init_table_entry_t *plr_init_table_end) {
    const init_table_entry_t *lp=lplr_init_table_start, *le=lplr_init_table_end;
    const init_table_entry_t *pp=plr_init_table_start, *pe=plr_init_table_end;

    while ((lp!=le)||(pp!=pe)) {
        if ((lp!=le) && (pp!=pe)) {
            if (lp->level<=pp->level) {
                (*lp->fn)();
                lp++;
            } else {
                (*pp->fn)();
                pp++;
            }
        } else if (lp!=le) {
            (*lp->fn)();
            lp++;
        } else {
            //(pp!=pe)
            (*pp->fn)();
            pp++;
        }
    }
}

void __attribute__((noreturn)) UTIL_MIPS16
traversal_init_table()  {
    // run init functions by merge lplr's and plr's init functions
    const init_table_entry_t *ifunc_end=_soc_header.end_of_init_func_list;
    run_init_level(&start_of_init_func_table, &end_of_init_func_table, _soc_header.init_func_list, ifunc_end);
    while(1) ; // should never return
}

// 1. export functions
// 2. export parameters
symb_fdefine(SF_RUN_INIT_LEVEL, run_init_level);
symb_fdefine(SF_SYS_LX_TIMER_INIT, otto_lx_timer_init);
symb_fdefine(SF_SYS_UDELAY, otto_lx_timer_udelay);
symb_fdefine(SF_SYS_MDELAY, otto_lx_timer_mdelay);
symb_fdefine(SF_SYS_GET_TIMER, otto_lx_timer_get_timer);
symb_fdefine(SF_PROTO_PRINTF, proto_printf);
symb_fdefine(SCID_STR_ATOI, atoi);
symb_fdefine(SCID_STR_STRCPY, strcpy);
symb_fdefine(SCID_STR_STRLEN, strlen);
symb_fdefine(SCID_STR_STRCMP, strcmp);
symb_fdefine(SCID_STR_MEMCPY, memcpy);
symb_fdefine(SCID_STR_MEMSET, memset);
symb_fdefine(SCID_STR_MASSCPY, mass_copy);

// end symbols
