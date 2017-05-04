#include <common.h>
#include <soc.h>
#include <asm/otto_symb_define.h>
#include <asm/otto_cg_dev_freq.h>

#define OTTO21_TEMP_SYM_HEADER ((const symbol_table_entry_t *)(((soc_t *)(0x9f000020))->bios.header.export_symb_list))
#define OTTO21_TEMP_SYM_END    ((const symbol_table_entry_t *)(((soc_t *)(0x9f000020))->bios.header.end_of_export_symb_list))


u32_t cg_query_freq(u32_t dev_type) {
    static cg_dev_freq_t *_cg_dev_freq_ptr = VZERO;
    const symbol_table_entry_t *ste;
    
	if (_cg_dev_freq_ptr == VZERO) {
		ste = UBOOT_SYMB_RETRIEVE(SF_SYS_CG_DEV_FREQ, OTTO21_TEMP_SYM_HEADER, OTTO21_TEMP_SYM_END);
		if (ste) {
			_cg_dev_freq_ptr = (cg_dev_freq_t *)ste->v.pvalue;
		} else {
			return 0;
		}
	}

    return CG_QUERY_FREQUENCY(dev_type, _cg_dev_freq_ptr);
};

