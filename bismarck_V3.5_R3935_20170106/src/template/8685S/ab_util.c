#include <util.h>
#include <uart/uart.h>
#include <cg/cg.h>
#include <lib/lzma/LzmaDec.h>

#define STACK_GUIDE     0xcafecafe

unsigned int _util_dummy_func(void) {return 0;}

proto_printf_t *_proto_printf SECTION_SDATA =(proto_printf_t*)_util_dummy_func;
udelay_t *_udelay SECTION_SDATA =(udelay_t *)_util_dummy_func;
mdelay_t *_mdelay SECTION_SDATA =(mdelay_t *)_util_dummy_func;
get_timer_t *_get_timer SECTION_SDATA =(get_timer_t *)_util_dummy_func;
u32_t util_ms_accumulator SECTION_SDATA =0;


// message
static char _banner_msg[] SECTION_RECYCLE_DATA = {"PRELOADER.%x.%x\n"};

UTIL_FAR SECTION_UNS_TEXT void
puts(const char *s) {
	inline_puts(s);
}

SECTION_RECYCLE 
void plr_init_utility(void)
{
	u32_t chip_ver = 0;
    const symbol_table_entry_t *s;

    // 1. binding from PLR symbol table
    // 1.1 bind proto_printf
    s=symb_retrive_plr(SF_PROTO_PRINTF);
    if (s) _proto_printf=s->v.pvalue;

    // 1.2 bind delays
    s=symb_retrive_plr(SF_SYS_UDELAY);
    if (s) _udelay=s->v.pvalue;
    s=symb_retrive_plr(SF_SYS_MDELAY);
    if (s) _mdelay=s->v.pvalue;
    s=symb_retrive_plr(SF_SYS_GET_TIMER);
    if (s) _get_timer=s->v.pvalue;

#ifdef HAS_LIB_LZMA
    // 1.3 bind lzma
    s=symb_retrive_lplr(SF_LIB_LZMA_DECODE);
    if (s) _lzma_decode=s->v.pvalue;
    else _lzma_decode = LzmaDecode;
#endif

	// 2. init uart
	uart_init(uart_baud_rate, cg_info_proj.dev_freq.lx_mhz);
	_bios.uart_putc=uart_putc;
	_bios.uart_getc=uart_getc;
	_bios.uart_tstc=uart_tstc;

	// 3. using printf showing the banner
	printf(_banner_msg, _soc_header.version, chip_ver);

	// 4. initial utility run-time value
    //util_cpu_mhz = GET_CPU_HZ();

	// 5. put stack guide words
	/* extern u32_t farthest_stack_position; */
	/* u32_t *cur_sp; */
	/* __asm__ __volatile__  ("addiu %0, $29, -4": "=r"(cur_sp)); */
	/* u32_t *sp_end=(u32_t *)(0x9f000000 + 16*1024); */
	/* while (cur_sp != sp_end)  */
	/*     *(cur_sp--)=STACK_GUIDE; */
}

REG_INIT_FUNC(plr_init_utility, 1);
