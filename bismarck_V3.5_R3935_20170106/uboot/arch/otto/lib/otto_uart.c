#include <common.h>
#include <soc.h>
#include <asm/otto_pll.h>

DECLARE_GLOBAL_DATA_PTR;

#define OTTO21_TEMP_PARAMETERS (((soc_t *)(0x9f000020))->bios)
#define OTTO21_TEMP_PUTC uart_putc
#define OTTO21_TEMP_GETC uart_getc
#define OTTO21_TEMP_TSTC uart_tstc

#define parameters OTTO21_TEMP_PARAMETERS
#define _uart_putc OTTO21_TEMP_PUTC
#define _uart_getc OTTO21_TEMP_GETC
#define _uart_tstc OTTO21_TEMP_TSTC

int serial_init (void) {
	if (gd->baudrate == 0x0) {
	} else {
		serial_setbrg();
	}
	return (0);
}

void serial_putc(const char c) {
	if (c == '\n')
		parameters._uart_putc('\r');

	parameters._uart_putc(c);
	return;
}

void serial_putc_raw(const char c) {
	parameters._uart_putc(c);
	return;
}

void serial_puts(const char *s) {
	while (*s) {
		serial_putc(*s++);
	}
	return;
}

int serial_getc() {
	return parameters._uart_getc();
}

int serial_tstc() {
	return parameters._uart_tstc();
}

/* void serial_setbrg() { */
/* 	parameters._uart_init(calc_divisor()); */
/* 	return; */
/* } */
void serial_setbrg() {
	/* OTTO21_TEMP dummy function for test. */
	return;
}
