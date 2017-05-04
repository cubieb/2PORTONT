#ifndef UTIL_H
#define UTIL_H
#include <soc.h>
#include <proto_printf.h>
#include <inline_util.h>
#include <symb_define.h>
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

#define printf(...) ({proto_printf((proto_putc_t*)_bios.uart_putc, VZERO, __VA_ARGS__);})
extern void plr_puts(const char *s) UTIL_FAR;
#define puts	plr_puts

#include <sys.h>
extern u32_t util_ms_accumulator SECTION_SDATA;

#define udelay(us) otto_sys_udelay(us, GET_CPU_HZ()/1000000);
#define mdelay(ms) otto_sys_mdelay(ms, GET_CPU_HZ()/1000000)
#define get_timer(base) (otto_sys_get_timer)(base, &util_ms_accumulator,GET_CPU_HZ()/1000000)

#define SWBREAK() do {               \
		__asm__ __volatile__ ("sdbbp;"); \
	} while(0)

#endif // UTIL_H

