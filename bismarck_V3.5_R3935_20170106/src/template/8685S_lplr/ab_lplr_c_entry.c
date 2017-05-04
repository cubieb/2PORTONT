#include <util.h>

extern const init_table_entry_t start_of_init_func_table, end_of_init_func_table;
extern const nand_spi_probe_t *start_of_nand_spi_probe_func;
extern const nand_spi_probe_t *end_of_nand_spi_probe_func;

typedef oob6t_t oob_t;

static UTIL_MIPS16 int 
ecc_plr_to_sram(plr_nand_spi_info_t *snaf_info) {
    #define lplr_chunk_buffer   ((char*)OTTO_LPLR_SPI_NAND_CHUNK_BUFFER)
    #define lplr_oob_buffer     ((char*)OTTO_LPLR_SPI_NAND_OOB_BUFFER)
    #define lplr_ecc_buffer     ((char*)OTTO_LPLR_SPI_NAND_ECC_BUFFER)

    u32_t max_blk_pge_num = SNAF_NUM_OF_BLOCK(snaf_info)*SNAF_NUM_OF_CHUNK_PER_BLK(snaf_info);
    u32_t chunk_size = SNAF_CHUNK_SIZE(snaf_info);
    u32_t max_plr_first_load = (OTTO_SRAM_SIZE/chunk_size);

    u32_t sram_plr_addr;
    int ret;
    u32_t need_cpy;                  //Denote which chunk need to be copied to SRAM
    u32_t signature = 0;             //OOB
    u32_t total_num_of_chunk = 0;    //OOB
    u32_t total_num_of_plr_copy = 0; //OOB
    u32_t num_of_1st_load = 0;       //OOB
    u8_t chunk_valid_table[max_plr_first_load];   //Used to denote which chunk is readed
    u32_t cnt_chunk = 0;            //Used to count the next PLR copy position
    u32_t cnt_valid_chunk = 0;      //Used to count if the program of "num_of_1st_load" been loaded successfully
    u32_t cnt_plr = 0;              //Used to count if there is another PLR can be read
    u32_t blk_pge_addr;
    oob_t *oob = (oob_t *)lplr_oob_buffer;
    

	inline_bzero(chunk_valid_table, sizeof(chunk_valid_table)); 

    for(blk_pge_addr=0 ; blk_pge_addr<max_blk_pge_num ; blk_pge_addr++){
        need_cpy = 0;
        if(0 == signature){
            ret = spi_nand_chunk_read_ecc(snaf_info)(snaf_info, (void *)lplr_chunk_buffer, blk_pge_addr, (void *)lplr_ecc_buffer);

            if((ECC_DECODE_FAIL != ret) && (ECC_DECODE_ALL_ONE != ret)){
                //Read OOB for each block
                signature = oob->signature;
                num_of_1st_load = oob->startup_num;
                total_num_of_chunk = oob->total_chunk;
                total_num_of_plr_copy = oob->num_copy;

                //Check if not PLR
                if(SIGNATURE_PLR != signature) return -1;

                if(cnt_chunk >= total_num_of_chunk){
                    cnt_plr = cnt_chunk/total_num_of_chunk;
                    cnt_chunk = cnt_chunk%total_num_of_chunk;
                    if(cnt_plr >= total_num_of_plr_copy) return -1;
                }
                need_cpy= 1;
             }
        }else if((0 == chunk_valid_table[cnt_chunk]) && (cnt_chunk < num_of_1st_load)){
            ret = spi_nand_chunk_read_ecc(snaf_info)(snaf_info, (void *)lplr_chunk_buffer, blk_pge_addr, (void *)lplr_ecc_buffer);
            if((ECC_DECODE_FAIL != ret) && (ECC_DECODE_ALL_ONE != ret)){
                if(SIGNATURE_PLR != oob->signature) return -1;
                need_cpy= 1;
            }
        }
        if(1 == need_cpy){
            sram_plr_addr = (OTTO_SRAM_START + (chunk_size*cnt_chunk));
            inline_memcpy(sram_plr_addr, lplr_chunk_buffer, chunk_size);
            chunk_valid_table[cnt_chunk] = 1;
            cnt_valid_chunk++;
            //Check if all the number of chunk of 1st_load is OK?
            if(cnt_valid_chunk == num_of_1st_load){
                _lplr_basic_io.dcache_writeback_invalidate_all();
                _lplr_basic_io.icache_invalidate_all();
                return 0;
            } 
        }

        //The number of 1st load is readed, cotinue to finding next start of copy
        cnt_chunk++;
        if(cnt_chunk == total_num_of_chunk){
            cnt_chunk = 0;
            cnt_plr++;
            if(cnt_plr == total_num_of_plr_copy) return -1;
        }
    }
    return -1;
}

void UTIL_MIPS16
lplr_c_entry(void) {
    plr_nand_spi_info_t *snaf_info=VZERO;
    inline_wmemset((void*)OTTO_SRAM_START, 0xcafecafe, OTTO_SRAM_SIZE-TLB_PAGE_SIZE);
    inline_wzero((void*)OTTO_SRAM_START, sizeof(basic_io_t));	

    // probe NAND SPI 
    const nand_spi_probe_t **probe_snf = &start_of_nand_spi_probe_func;
    while (probe_snf!=&end_of_nand_spi_probe_func) {
        snaf_info = (*probe_snf)();
        if(snaf_info != VZERO) break;
        ++probe_snf;	
    }
    
    // If probe fail, stuck at here. 
    if(snaf_info == VZERO) while (1);

    // load preloader first a few kilo bytes
    if(-1 == ecc_plr_to_sram(snaf_info)) while(1);
    _soc.flash_info.nand_spi_info = snaf_info;
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

void UTIL_MIPS16 
traversal_init_table() {
    // run init functions by merge lplr's and plr's init functions
    const init_table_entry_t *ifunc_end=_soc_header.end_of_init_func_list;
    run_init_level(&start_of_init_func_table, &end_of_init_func_table, _soc_header.init_func_list, ifunc_end);
    while(1) ; // should never return
}
// 1. export functions
// 2. export parameters

symb_fdefine(SF_RUN_INIT_LEVEL, run_init_level);
symb_fdefine(SF_SYS_UDELAY, otto_sys_udelay);
symb_fdefine(SF_SYS_MDELAY, otto_sys_mdelay);
symb_fdefine(SF_SYS_GET_TIMER, otto_sys_get_timer);
symb_fdefine(SF_PROTO_PRINTF, proto_printf);


// end symbols
//MARK_INIT_FUNC_ENDING_SYMB;
//MARK_SYMB_TABLE_ENDING_SYMB;
