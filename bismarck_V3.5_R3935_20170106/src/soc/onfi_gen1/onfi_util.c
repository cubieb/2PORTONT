#include <soc.h>
#include <onfi/onfi_util.h>
#include <ecc/ecc_ctrl.h>

#ifndef SECTION_RECYCLE
    #define SECTION_RECYCLE
#endif

// linker script defines following symbol
extern const char lma_mapping_start SECTION_RO;

static u16_t plr_vmpage_table[PLR_MAP_TABLE_SIZE] SECTION_SDATA;
u32_t tlb_start_idx SECTION_SDATA;
u32_t chunk_per_tlb_page SECTION_SDATA;
u32_t otto_plr_page_buf_addr SECTION_SDATA;


SECTION_RECYCLE void 
onfi_probe(onfi_info_t *info,
    const onfi_probe_t **probe_func_start,
    const onfi_probe_t **probe_func_end) {
    // result will be store in *info and its following structure
    onfi_info_t *onaf_info=VZERO;

    const onfi_probe_t **probe_onfi = probe_func_start;
    while (probe_onfi != probe_func_end) {
        onaf_info = (*probe_onfi)(info);
        if(onaf_info != VZERO) {
            return;
        }
        ++probe_onfi;	
    }
    return;
}


SECTION_RECYCLE static int 
is_plr_first_load_part(u32_t blk_page_idx)
{
    plr_first_load_layout_t *fl_buf=(plr_first_load_layout_t *)page_buffer;
    //u32_t page_size = (ONFI_PAGE_SIZE((_onfi_info))+ONFI_SPARE_SIZE((_onfi_info)));
    opu_page_read(_onfi_info, page_buffer, blk_page_idx);

    if(opu_ecc_engine_act(_onfi_info->_ecc_ability, fl_buf->data0, fl_buf->oob0, 0) == ECC_CTRL_ERR) return ECC_CTRL_ERR; 
    if(opu_ecc_engine_act(_onfi_info->_ecc_ability, fl_buf->data1, fl_buf->oob1, 0) == ECC_CTRL_ERR) return ECC_CTRL_ERR; 
    if(PLR_FL_GET_SIGNATURE(fl_buf) == SIGNATURE_PLR_FL) return 1;

    return 0;
}


SECTION_RECYCLE int 
onfi_util_init(void)
{
    u32_t page_size = ONFI_PAGE_SIZE(_onfi_info);
    if(page_size == MAX_PAGE_BUF_SIZE) otto_plr_page_buf_addr = OTTO_PLR_PAGE_BUFFER;
    else otto_plr_page_buf_addr = (OTTO_PLR_OOB_BUFFER-page_size);

    chunk_per_tlb_page = TLB_PAGE_SIZE / page_size;
    
    int   ret;
    u32_t blk_page_idx;
    u32_t max_blk_pge_num = ONFI_NUM_OF_BLOCK(_onfi_info) * ONFI_NUM_OF_PAGE_PER_BLK(_onfi_info);
    u8_t fill_table;
    u32_t total_num_of_chunk = 0;   //OOB
    u32_t total_num_of_plr_copy = 0;//OOB
    u32_t cnt_chunk = 0;
    u32_t cnt_plr = 0;
    u32_t cnt_valid_chunk = 0;
    oob_t *oob = oob_buffer;
    tlb_start_idx=((u32_t)&lma_mapping_start)/PLR_FL_PAGE_USAGE;

    memset((void*)plr_vmpage_table, 0xFF, sizeof(plr_vmpage_table));
    for(blk_page_idx=0; blk_page_idx<max_blk_pge_num; blk_page_idx++) {
        fill_table = 0;
        if(0 == total_num_of_chunk){
            ret = opu_page_read_ecc(_onfi_info, page_buffer, blk_page_idx, ecc_buffer);

            if(1 == is_plr_first_load_part(blk_page_idx)){
                fill_table = 1;
            }else if(!IS_ECC_DECODE_FAIL(ret)){
                //Read OOB for each block
                total_num_of_chunk = oob->total_chunk;
                total_num_of_plr_copy = oob->num_copy;
                if(SIGNATURE_PLR != oob->signature) return -1; 

                if(cnt_chunk >= total_num_of_chunk){
                    cnt_plr = cnt_chunk/total_num_of_chunk;
                    cnt_chunk = cnt_chunk%total_num_of_chunk;
                    if(cnt_plr >= total_num_of_plr_copy) return -1;
                }                 
                fill_table = 1;
            }
        } else if(0xFFFF == plr_vmpage_table[cnt_chunk]){
            ret = opu_page_read_ecc(_onfi_info, page_buffer, blk_page_idx, ecc_buffer);
            if(!IS_ECC_DECODE_FAIL(ret)){
                if(SIGNATURE_PLR != oob->signature) return -1;
                fill_table = 1;
            }else{
                if(1 == is_plr_first_load_part(blk_page_idx)) fill_table = 1;
            }
        }

        
        if(1 == fill_table){
            plr_vmpage_table[cnt_chunk] = (u16_t)blk_page_idx;
            cnt_valid_chunk++;
            if(cnt_valid_chunk == total_num_of_chunk) return 0;
        }
        cnt_chunk++;
        if(cnt_chunk == total_num_of_chunk){
            cnt_chunk = 0;
            cnt_plr++;
            if(cnt_plr == total_num_of_plr_copy) return -1;
        }
    }

    return -1;
}

SECTION_UNS_TEXT int 
onfi_util_logical_pio_read(void *data, u32_t page_num)
{
    int ret;
    u32_t i;
    u32_t prl_nand_chunk_size = ONFI_PAGE_SIZE(_onfi_info);
    u32_t index = page_num * chunk_per_tlb_page + tlb_start_idx;
    u16_t *mapping=plr_vmpage_table + index;   

    for (i=0 ; i<chunk_per_tlb_page; ++i){
        if((*mapping) == 0xFFFF) break;
        ret = opu_page_read_ecc(_onfi_info, page_buffer, (u32_t)(*mapping),  ecc_buffer);
            
        if(IS_ECC_DECODE_FAIL(ret)) return -1;

        mass_copy(data, page_buffer, prl_nand_chunk_size);
        ++mapping;
        data += prl_nand_chunk_size;
    }
    dcache_wr_inv_all();
    icache_inv_all();
    return 0;
}


