#ifndef ONFI_UTIL_H
#define ONFI_UTIL_H


#include <soc.h>

#ifdef ONFI_USING_SYMBOL_TABLE_FUNCTION
    #ifdef NSU_DRIVER_IN_ROM
        #error 'lplr should not use symbol_table function'
    #endif
    #include "onfi_symb_func.h"
#else
    #include "onfi_common.h"
    #define onu_reset_spi_nand_chip onfi_reset_nand_chip
    #define onu_read_spi_nand_id onfi_read_id
    #define onu_get_sts_reg onfi_get_status_register
#endif

extern u32_t otto_plr_page_buf_addr;
#define page_buffer ((void*)(otto_plr_page_buf_addr))
#define oob_buffer   ((oob_t*)(OTTO_PLR_OOB_BUFFER))
#define ecc_buffer   ((void*)(OTTO_PLR_ECC_BUFFER))

typedef plr_oob_t oob_t;


int onfi_util_init(void);
int onfi_util_logical_pio_read(void *data, u32_t page_num);
void onfi_probe(onfi_info_t *info, const onfi_probe_t **probe_func_start, const onfi_probe_t **probe_func_end);



#endif //ONFI_UTIL_H

