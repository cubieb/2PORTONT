#ifndef OTTO_SYMBOL_DEFINE_H
#define OTTO_SYMBOL_DEFINE_H

#include "symb_define.h"

#define UBOOT_SYMB_RETRIEVE(key, symb_list, list_end) ({\
    const symbol_table_entry_t *__symb_ptr = symb_retrive(key, symb_list, list_end);\
    if((((u32_t)(__symb_ptr->v.pvalue))>>28) >= 0xC) \
        printf("WW: The retrieved symbol is in swapable area 0x%x\n",((u32_t)(__symb_ptr->v.pvalue)));\
    __symb_ptr;\
})


#endif //OTTO_SYMBOL_DEFINE_H
