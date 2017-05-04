#ifndef UTIL_H
#define UTIL_H
#include <soc.h>
#include <init_define.h>

#ifndef UTIL_MIPS16 
    #ifdef NO_UTIL_MIPS16 
        #define UTIL_MIPS16  
    #else
        #define UTIL_MIPS16  __attribute__((mips16))
    #endif
#endif

#ifndef UTIL_FAR 
    #ifndef NO_UTIL_FAR 
        #define UTIL_FAR __attribute__ ((far))
    #else
        #define UTIL_FAR 
    #endif
#endif

#include <symb_define.h>
#define symb_retrive_lplr(key) symb_retrive(key, _lplr_header.export_symb_list, _lplr_header.end_of_export_symb_list);
#define symb_retrive_plr(key) symb_retrive(key, _soc_header.export_symb_list, _soc_header.end_of_export_symb_list);

// functions defined in LPLR
#define NO_PROTO_PRINTF 
#include <proto_printf.h>
extern proto_printf_t *_proto_printf;
#define printf(...) ({proto_printf((proto_putc_t*)_bios.uart_putc, VZERO, __VA_ARGS__);})
extern void puts(const char *s) UTIL_FAR;

#define NO_SYS_FUNC
#include <sys.h>

extern udelay_t *_udelay SECTION_SDATA;
extern mdelay_t *_mdelay SECTION_SDATA;
extern get_timer_t *_get_timer SECTION_SDATA;
extern u32_t util_ms_accumulator SECTION_SDATA;


#define SWBREAK() do {               \
                __asm__ __volatile__ ("sdbbp;"); \
        } while(0)

#define udelay(us) (*_udelay)(us, GET_CPU_MHZ())
#define mdelay(ms) (*_mdelay)(ms, GET_CPU_MHZ())
#define get_timer(base) (*_get_timer)(base, &util_ms_accumulator, GET_CPU_MHZ())

//For LZMA lib
#include <tlzma.h>

#endif //UTIL_H

