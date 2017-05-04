#ifndef UTIL_H
#define UTIL_H
#include <soc.h>
#include <timer.h>
#include <proto_printf.h>
#include <inline_util.h>
#include <symb_define.h>
#include <init_define.h>

#define printf(...) ({proto_printf((proto_putc_t*)_bios.uart_putc, VZERO, __VA_ARGS__);})

extern soc_t _lplr_soc_structure;
#define _lplr_basic_io _lplr_soc_structure.bios

#ifndef UTIL_MIPS16 
    #ifdef NO_UTIL_MIPS16 
        #define UTIL_MIPS16  
    #else
        #define UTIL_MIPS16  __attribute__((mips16))
    #endif
#endif


#endif // UTIL_H

