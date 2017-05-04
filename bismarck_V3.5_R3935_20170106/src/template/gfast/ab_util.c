#include <init_define.h>
#include <uart/uart.h>
#include <util.h>

#define STACK_GUIDE     0xcafecafe

u32_t util_ms_accumulator SECTION_SDATA =0;

// message
static char _banner_msg[] SECTION_RECYCLE_DATA = {"PRELOADER.%x.%x\n"};

UTIL_FAR SECTION_UNS_TEXT void
puts(const char *s) {
	inline_puts(s);
}

SECTION_RECYCLE void
plr_init_utility(void) {
	const u32_t chip_ver = 0;

	/* This is to enable UART IP. */
	//*((volatile u32_t *)0xbb023004) = 0x00000070;

	// 2. init uart
	uart_init(uart_baud_rate, lx_bus_freq_mhz);
	_bios.uart_putc=uart_putc;
	_bios.uart_getc=uart_getc;
	_bios.uart_tstc=uart_tstc;

	// 3. using printf showing the banner
	printf(_banner_msg, _soc_header.version, chip_ver);

	// 5. put stack guide words
	/* extern u32_t farthest_stack_position; */
	/* u32_t *cur_sp; */
	/* __asm__ __volatile__  ("addiu %0, $29, -4": "=r"(cur_sp)); */
	/* u32_t *sp_end=(u32_t *)(0x9f000000 + 16*1024); */
	/* while (cur_sp != sp_end)  */
	/*     *(cur_sp--)=STACK_GUIDE; */
}

REG_INIT_FUNC(plr_init_utility, 1);
