#include <init_define.h>
#include <uart/uart.h>
#include <util.h>
#include <lib/lzma/LzmaDec.h>
#include <lib/lzma/tlzma.h>


#define STACK_GUIDE     0xcafecafe

unsigned int _util_dummy_func(void) {return 0;}

proto_printf_t *_proto_printf SECTION_SDATA =(proto_printf_t*)_util_dummy_func;
lx_timer_init_t *_lx_timer_init SECTION_SDATA =(lx_timer_init_t *)_util_dummy_func;
udelay_t *_udelay SECTION_SDATA =(udelay_t *)_util_dummy_func;
mdelay_t *_mdelay SECTION_SDATA =(mdelay_t *)_util_dummy_func;
get_timer_t *_get_timer SECTION_SDATA =(get_timer_t *)_util_dummy_func;
u32_t util_ms_accumulator SECTION_SDATA = 0;
s8_t chip_ver[4] SECTION_SDATA = {0};

// message
char toolkitinfo[] SECTION_RECYCLE_DATA = TKINFO;
char proj_name[] SECTION_RECYCLE_DATA = name_of_project;
static char _banner_msg[] SECTION_RECYCLE_DATA = {"\n\n%s\nPRELOADER Bismarck %u.%u\n"};
static char _ver_msg[] SECTION_RECYCLE_DATA = {"II: SVN:%x, Build_date:%x, Toolkit:%s\n"};

UTIL_FAR SECTION_UNS_TEXT void
puts(const char *s) {
	inline_puts(s);
}

#ifndef KEEP_TIMEOUT_MONITOR
SECTION_RECYCLE static void 
disable_timeout_monitor(void)
{
    //Disable OCP timeout monitor   
    RMOD_OCP_TO_CTRL(to_ctrl_en, 0);
    puts("II: Disable OCP Timeout Monitor\n");

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

    puts("II: Disable LX Timeout Monitor\n");
}
#endif

SECTION_RECYCLE 
void plr_init_utility(void)
{
    //Step1: Read SOC ID
    _soc.cid = (*((volatile u32_t *)0xB80010FC)>>16) & 0xffff;


    // Step2. Binding from PLR symbol table
#if 0
    const symbol_table_entry_t *s;

    // Step2.1 bind proto_printf
    s=symb_retrive_plr(SF_PROTO_PRINTF);
    if (s) _proto_printf=s->v.pvalue;

    // Step2.2 bind delays
    s=symb_retrive_plr(SF_SYS_UDELAY);
    if (s) _udelay=s->v.pvalue;
    s=symb_retrive_plr(SF_SYS_MDELAY);
    if (s) _mdelay=s->v.pvalue;
    s=symb_retrive_plr(SF_SYS_GET_TIMER);
    if (s) _get_timer=s->v.pvalue;

#ifdef HAS_LIB_LZMA
    // Step2.3 bind lzma
    s=symb_retrive_plr(SF_LIB_LZMA_DECODE);
    if (s) _lzma_decode=s->v.pvalue;
    else _lzma_decode = LzmaDecode;
#endif
#endif //#if 0

    _proto_printf = proto_printf;
    _lx_timer_init = otto_lx_timer_init;
    _udelay = otto_lx_timer_udelay;
    _mdelay = otto_lx_timer_mdelay;
    _get_timer = otto_lx_timer_get_timer;
#ifdef HAS_LIB_LZMA
    _lzma_decode = LzmaDecode;
#endif

    //Step3. Set UART0 IO_Enable, Baud-rate, LX Timer
    UART_IO_EN();
	uart_init(uart_baud_rate, TIMER_FREQ_MHZ);
	_bios.uart_putc=uart_putc;
	_bios.uart_getc=uart_getc;
	_bios.uart_tstc=uart_tstc;
    lx_timer_init(TIMER_FREQ_MHZ);

	//Step4. using printf showing the banner
    printf(_banner_msg, proj_name, (bismarck_ver >> 24), (bismarck_ver >> 8) & 0xff);
    printf(_ver_msg, VCS_VER, MAKE_DATE, toolkitinfo);

#ifndef KEEP_TIMEOUT_MONITOR
	//Step5. disable timeout monitor
	disable_timeout_monitor();
#endif

	// 5. put stack guide words
	/* extern u32_t farthest_stack_position; */
	/* u32_t *cur_sp; */
	/* __asm__ __volatile__  ("addiu %0, $29, -4": "=r"(cur_sp)); */
	/* u32_t *sp_end=(u32_t *)(0x9f000000 + 16*1024); */
	/* while (cur_sp != sp_end)  */
	/*     *(cur_sp--)=STACK_GUIDE; */
}

REG_INIT_FUNC(plr_init_utility, 1);
