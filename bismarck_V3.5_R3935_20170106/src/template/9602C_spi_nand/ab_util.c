#include <util.h>
#include <uart/uart.h>
#include <cg/cg.h>
#include <lib/lzma/tlzma.h>

#define STACK_GUIDE     0xcafecafe

proto_printf_t *_proto_printf SECTION_SDATA =(void*)ALWAYS_RETURN_ZERO;
lx_timer_init_t *_lx_timer_init SECTION_SDATA =(void *)ALWAYS_RETURN_ZERO;
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


u32_t util_cpu_mhz SECTION_SDATA = 0;
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
char tkinfo[] = TKINFO;
char proj_name[] = name_of_project;
const char _banner_msg[] SECTION_RECYCLE_DATA = {"\n\n%s\nPRELOADER Bismarck %u.%u\n"};
const char _ver_msg[] SECTION_RECYCLE_DATA = {"II: LPLR:%x, SVN:%x, Build_date:%x, Toolkit:%s\n"};

#ifndef KEEP_TIMEOUT_MONITOR
const char __msg_dis_ocp_to_monitor[] SECTION_RECYCLE_DATA = {"II: Disable OCP Timeout Monitor\n"};
const char __msg_dis_lx_to_monitor[] SECTION_RECYCLE_DATA = {"II: Disable LX Timeout Monitor\n"};
SECTION_RECYCLE static void 
disable_timeout_monitor(void)
{
    //Disable OCP timeout monitor   
    RMOD_OCP_TO_CTRL(to_ctrl_en, 0);
    puts(__msg_dis_ocp_to_monitor);

    //Disable LX timeout monitor
    RMOD_LXP_TO_CTRL(to_ctrl_en, 0);

    RMOD_LX0_M_TO_CTRL(to_ctrl_en, 0);
    RMOD_LX0_S_TO_CTRL(to_ctrl_en, 0);

    RMOD_LX1_M_TO_CTRL(to_ctrl_en, 0);
    RMOD_LX1_S_TO_CTRL(to_ctrl_en, 0);

    RMOD_LX2_M_TO_CTRL(to_ctrl_en, 0);
    RMOD_LX2_S_TO_CTRL(to_ctrl_en, 0);

    RMOD_LX_PBO_USW_TO_CTRL(to_ctrl_en, 0);
    RMOD_LX_PBO_USR_TO_CTRL(to_ctrl_en, 0);

    RMOD_LX_PBO_DSW_TO_CTRL(to_ctrl_en, 0);
    RMOD_LX_PBO_DSR_TO_CTRL(to_ctrl_en, 0);

    puts(__msg_dis_lx_to_monitor);
}
#endif

const symb_retrive_entry_t plr_init_utility_retrive_list[] SECTION_RECYCLE_DATA = {
    {SF_PROTO_PRINTF, &_proto_printf},
    {SF_SYS_LX_TIMER_INIT, &_lx_timer_init},
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
plr_init_utility(void)
{
    // 1. Read SOC ID
    _soc.cid = (*((volatile u32_t *)0xB80010FC)>>16) & 0xffff;

    // 2. put stack guide words
    extern u32_t farthest_stack_position;
    u32_t *cur_sp;
    __asm__ __volatile__  ("addiu %0, $29, -4": "=r"(cur_sp));
    u32_t *sp_end=&farthest_stack_position;
    while (cur_sp != sp_end) 
        *(cur_sp--)=STACK_GUIDE;

    // 3. binding
    symb_retrive_list(plr_init_utility_retrive_list, lplr_symb_list_range);
    
    #ifdef HAS_LIB_LZMA
        symb_retrive_and_set(lplr, SF_LIB_LZMA_DECODE, _lzma_decode);
    #endif

    // 4. init uart
    // 4.1 Set UART0 IO_Enable
    UART_IO_EN();

    // 4.2 Set uart baud rate
    uart_init(uart_baud_rate, TIMER_FREQ_MHZ);
    _bios.uart_putc=uart_putc;
    _bios.uart_getc=uart_getc;
    _bios.uart_tstc=uart_tstc;
    
    // 5. using printf showing the banner
    printf(_banner_msg, proj_name, (bismarck_ver >> 24), (bismarck_ver >> 8) & 0xff);
    printf(_ver_msg, (_lplr_soc_t.cid>>16), VCS_VER, MAKE_DATE, tkinfo);
    
    // 6. disable timeout monitor
    #ifndef KEEP_TIMEOUT_MONITOR
        disable_timeout_monitor();
    #endif
    
    // 5. initial utility run-time value
    util_cpu_mhz = CPU_CLK_DEFAULT;
}

SECTION_CG_MISC
void init_util_mhz_after_cg(void) {
    util_cpu_mhz = cg_query_freq(CG_DEV_OCP);
}
REG_INIT_FUNC(plr_init_utility, 1);
REG_INIT_FUNC(init_util_mhz_after_cg, 15);
