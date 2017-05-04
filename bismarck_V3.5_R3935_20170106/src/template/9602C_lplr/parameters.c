#include <soc.h>
#include <lplr_sections.h>

#ifndef SECTION_LPLR_BASIC_IO
    #define SECTION_LPLR_BASIC_IO SECTION_SOC_STRU 
#endif

extern void rlx5281_write_back_invalidate_dcache_cctl(void);
extern void rlx5281_write_back_inv_dcache_range(unsigned int base_addr, unsigned int end_addr);
extern void invalidate_icache_all(void);
extern void rlx5281_invalidate_icache_range(unsigned int base_addr, unsigned int end_addr);


extern init_table_entry_t start_of_init_func_table, end_of_init_func_table;
extern symbol_table_entry_t start_of_symble_table, end_of_symble_table;
soc_t _lplr_soc_structure SECTION_LPLR_BASIC_IO = {
    .bios = {
        .header= {
            .signature=SIGNATURE_LPLR,
            .version=LPLR_VERSION,
            .export_symb_list=&start_of_symble_table,
            .end_of_export_symb_list=&end_of_symble_table,
            .init_func_list=&start_of_init_func_table,
            .end_of_init_func_list=&end_of_init_func_table,
        },
        .dcache_writeback_invalidate_all= &rlx5281_write_back_invalidate_dcache_cctl,
        .dcache_writeback_invalidate_range=&rlx5281_write_back_inv_dcache_range,
        .icache_invalidate_all=&invalidate_icache_all,
        .icache_invalidate_range=&rlx5281_invalidate_icache_range
    },
    .cid = LPLR_CID,
};


