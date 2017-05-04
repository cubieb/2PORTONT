#include <util.h>
#include <uart/uart.h>
#include <cg/cg.h>
#include <lib/lzma/tlzma.h>

#define STACK_GUIDE     0xcafebeef

proto_printf_t *_proto_printf SECTION_SDATA =(void*)ALWAYS_RETURN_ZERO;
udelay_t *_udelay SECTION_SDATA =(void *)ALWAYS_RETURN_ZERO;
mdelay_t *_mdelay SECTION_SDATA =(void *)ALWAYS_RETURN_ZERO;
get_timer_t *_get_timer SECTION_SDATA =(get_timer_t *)ALWAYS_RETURN_ZERO;
unsigned int (*_atoi)(const char *v) SECTION_SDATA=(void*)ALWAYS_RETURN_ZERO;
char (*_strcpy)(char *dst, const char *src) SECTION_SDATA=(void*)ALWAYS_RETURN_ZERO;
u32_t (*_strlen)(const char *s) SECTION_SDATA=(void*)ALWAYS_RETURN_ZERO;
int (*_strcmp)(const char *s1, const char *s2) SECTION_SDATA=(void*)ALWAYS_RETURN_ZERO;
char (*_memcpy)(void *dst, const void *src, unsigned int len) SECTION_SDATA=(void*)ALWAYS_RETURN_ZERO;
char (*_mass_copy)(void *dst, const void *src, unsigned int len) SECTION_SDATA=(void*)ALWAYS_RETURN_ZERO;
char (*_memset)(void *dst, char value, unsigned int len) SECTION_SDATA=(void*)ALWAYS_RETURN_ZERO;


u32_t util_ms_accumulator SECTION_SDATA =0;

UTIL_FAR SECTION_UNS_TEXT void 
puts(const char *s) {
    inline_puts(s);
}

UTIL_FAR SECTION_UNS_TEXT void
__sprintf_putc(const char c, void *user) {
    //if (c=='\r') return;
    char **p=(char**)user;
    **p=c;
    ++(*p);
}


// message
const char _banner_msg[] SECTION_RECYCLE_DATA = {"PRELOADER.%08x.LPLR.%08x.\n"};
const char __msg_dis_ocp_to_monitor[] SECTION_RECYCLE_DATA = {"Disable OCP Timeout Monitor\n"};
const char __msg_dis_lx_to_monitor[] SECTION_RECYCLE_DATA = {"Disable LX Timeout Monitor\n"};

#if 0  // FIXME, remove timeout monitor while release
#ifndef KEEP_TIMEOUT_MONITOR
SECTION_RECYCLE static void 
disable_timeout_monitor(void)
{
    //Disable OCP timeout monitor
    TO_CTRL_T ocp_to_mon;
    ocp_to_mon.v = TO_CTRLrv;
    ocp_to_mon.f.to_ctrl_en = 0;
    TO_CTRLrv = ocp_to_mon.v;
    puts(__msg_dis_ocp_to_monitor);

    //Disable LX timeout monitor
    BUS_TO_CTRL_T lx_to_mon;
    lx_to_mon.v = BUS_TO_CTRLrv;
    lx_to_mon.f.to_en = 0;
    BUS_TO_CTRLrv = lx_to_mon.v;
    puts(__msg_dis_lx_to_monitor);
}
#endif
#endif

const symb_retrive_entry_t plr_init_utility_retrive_list[] SECTION_RECYCLE_DATA = {
    {SF_PROTO_PRINTF, &_proto_printf},
    {SF_SYS_UDELAY, &_udelay},   
    {SF_SYS_MDELAY, &_mdelay},
    {SF_SYS_GET_TIMER, &_get_timer},
    {SCID_STR_ATOI, &_atoi},
    {SCID_STR_STRCPY, &_strcpy},
    {SCID_STR_STRLEN, &_strlen},
    {SCID_STR_STRCMP, &_strcmp},
    {SCID_STR_MEMCPY, &_memcpy},
    {SCID_STR_MEMSET, &_memset},
    {SCID_STR_MASSCPY, &_mass_copy},
    {ENDING_SYMB_ID, VZERO}
};


SECTION_RECYCLE void 
plr_init_utility(void) {
    // 0. put stack guide words
    extern u32_t farthest_stack_position;
    u32_t *cur_sp;
    __asm__ __volatile__  ("addiu %0, $29, -4": "=r"(cur_sp));
    u32_t *sp_end=&farthest_stack_position;
    while (cur_sp != sp_end) 
        *(cur_sp--)=STACK_GUIDE;

    u32_t count_down=1000000;
    while(count_down>0) {
        count_down--;
    }
    // 1. binding
    /*
    symb_retrive_and_set(lplr, SF_PROTO_PRINTF, _proto_printf);
    symb_retrive_and_set(lplr, SF_SYS_UDELAY, _udelay);
    symb_retrive_and_set(lplr, SF_SYS_MDELAY, _mdelay);
    symb_retrive_and_set(lplr, SF_SYS_GET_TIMER, _get_timer);
    symb_retrive_and_set(lplr, SCID_STR_ATOI, _atoi);
    symb_retrive_and_set(lplr, SCID_STR_STRCPY, _strcpy);
    symb_retrive_and_set(lplr, SCID_STR_STRLEN, _strlen);
    symb_retrive_and_set(lplr, SCID_STR_STRCMP, _strcmp);
    symb_retrive_and_set(lplr, SCID_STR_MEMCPY, _memcpy);
    symb_retrive_and_set(lplr, SCID_STR_MEMSET, _memset);
    symb_retrive_and_set(lplr, SCID_STR_MASSCPY, _mass_copy);
    */
    symb_retrive_list(plr_init_utility_retrive_list, lplr_symb_list_range);

#ifdef HAS_LIB_LZMA
    symb_retrive_and_set(lplr, SF_LIB_LZMA_DECODE, _lzma_decode);
#endif

    // 2. init uart
    uart_init(uart_baud_rate, cg_info_proj.dev_freq.lx_mhz);
    _bios.uart_putc=uart_putc;
    _bios.uart_getc=uart_getc;
    _bios.uart_tstc=uart_tstc;
    
    // 3. using printf showing the banner 
    printf(_banner_msg, _soc_header.version, _lplr_header.version);
    
    // 4. disable timeout monitor
#ifndef KEEP_TIMEOUT_MONITOR
//    disable_timeout_monitor();
#endif
        
    // 5. initial utility run-time value
    //util_cpu_mhz = GET_CPU_HZ();
    
}

REG_INIT_FUNC(plr_init_utility, 1);
