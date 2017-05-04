#include <init_define.h>
#include <uart/uart.h>
#include <util.h>
#include <lib/lzma/LzmaDec.h>

#define STACK_GUIDE     0xcafecafe

u32_t util_ms_accumulator SECTION_SDATA = 0;
s8_t chip_ver[4] SECTION_SDATA = {0};

// message
static char _banner_msg[] SECTION_RECYCLE_DATA = {"\n\nBismarck.%u.%u.PRELOADER.%u.%u.%u.%x.%s\n"};

UTIL_FAR SECTION_UNS_TEXT void
puts(const char *s) {
	inline_puts(s);
}

#define IO_MODE_EN (*((volatile u32_t *)0xbb023004))

SECTION_RECYCLE void
plr_init_utility(void) {
	u32_t ver, orig;
	s8_t chip_ver[4] = {0};

	orig = *((volatile u32_t *)0xbb00007c);
	*((volatile u32_t *)0xbb00007c) = *((volatile u32_t *)0xbb00007c) | (0xb << 28);
	ver = ((*((volatile u32_t *)0xbb00007c)) >> 7) & 0x7;
	*((volatile u32_t *)0xbb00007c) = orig;
	switch (ver >> 1) {
	case 0:
		chip_ver[0] = 'x';
		break;
	case 1:
		chip_ver[0] = 'w';
		break;
	case 2:
		chip_ver[0] = 't';
		break;
	case 3:
		chip_ver[0] = 's';
		break;
	}
	chip_ver[1] = (ver & 0x1) + '0';

	*((volatile u32_t *)0xbb010004) = 0xa0000000;
	_soc.cid = *((volatile u32_t *)0xbb010004) & 0xffff;
	*((volatile u32_t *)0xbb010004) = 0x0;
	switch (_soc.cid) {
	case 25634:
		chip_ver[2] = '1';
		break;
	case 1593:
		chip_ver[2] = '0';
		break;
	default:
		chip_ver[2] = '?';
		break;
	}

	/* This is to enable UART IP. */
	IO_MODE_EN = (IO_MODE_EN | (1 << 4));

	/* Modify MVREF */
	RMOD_ANA_DLL0(cf_mvref0_por_sel, 1,
	              cf_mvref1_por_sel, 1);

#ifdef HAS_LIB_LZMA
	// 1.3 bind lzma
	_lzma_decode = LzmaDecode;
#endif

	// 2. init uart
	uart_init(uart_baud_rate, cg_query_freq(CG_DEV_LX));
	_bios.uart_putc=uart_putc;
	_bios.uart_getc=uart_getc;
	_bios.uart_tstc=uart_tstc;

	// 3. using printf showing the banner
	printf(_banner_msg,
	       (bismarck_ver >> 24),
	       (bismarck_ver >> 8) & 0xff,
	       (_soc_header.version >> 24),
	       (_soc_header.version >> 8) & 0xffff,
	       _soc_header.version & 0xff,
	       VCS_VER,
	       chip_ver);

	// 4. init timer
	otto_lx_timer_init(TIMER_FREQ_MHZ);

	// 5. put stack guide words
	/* extern u32_t farthest_stack_position; */
	/* u32_t *cur_sp; */
	/* __asm__ __volatile__  ("addiu %0, $29, -4": "=r"(cur_sp)); */
	/* u32_t *sp_end=(u32_t *)(0x9f000000 + 16*1024); */
	/* while (cur_sp != sp_end)  */
	/*     *(cur_sp--)=STACK_GUIDE; */
}

REG_INIT_FUNC(plr_init_utility, 1);
