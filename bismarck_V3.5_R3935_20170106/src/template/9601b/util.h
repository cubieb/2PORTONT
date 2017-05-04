#ifndef UTIL_H
#define UTIL_H
#include <soc.h>
#include <proto_printf.h>
#include <inline_util.h>
#include <symb_define.h>
#include <init_define.h>
#include <lib/misc/string.h>

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

#define printf(...) ({proto_printf((proto_putc_t*)_bios.uart_putc, VZERO, __VA_ARGS__);})
extern void puts(const char *s) UTIL_FAR;

#include <timer.h>
#define udelay(us)      otto_lx_timer_udelay(us)
#define mdelay(ms)      otto_lx_timer_mdelay(ms)
#define get_timer(base) otto_lx_timer_get_timer(base, &util_ms_accumulator)

#define SWBREAK() do {               \
		__asm__ __volatile__ ("sdbbp;"); \
	} while(0)

//For LZMA lib
#include <tlzma.h>

#define symb_retrive_plr(key) symb_retrive(key, _soc_header.export_symb_list, _soc_header.end_of_export_symb_list);

/* return decoded sz in efuse#6 */
u8_t efuse_6_rd(void);

#endif // UTIL_H
