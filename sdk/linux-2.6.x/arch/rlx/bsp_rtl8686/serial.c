/*
 * Copyright 2006, Realtek Semiconductor Corp.
 *
 * arch/rlx/rlxocp/serial.c
 *     RLXOCP serial port initialization
 *
 * Tony Wu (tonywu@realtek.com.tw)
 * Nov. 07, 2006
 */

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/serial.h>
#include <linux/serial_core.h>
#include <linux/serial_8250.h>
#include <linux/string.h>
#include <asm/serial.h>
#include "bspchip.h"
#include "netlog.h"
#include "prom.h"

int prom_putchar(char c)
{
	unsigned int busy_cnt = 0;

#ifdef CONFIG_RTL867X_NETLOG
	netlog_emit_char(c);
#endif

	do
	{
		/* Prevent Hanging */
		if (busy_cnt++ >= 30000)
		{
			/* Reset Tx FIFO */
			REG8(_UART_FCR) = BSP_TXRST | BSP_CHAR_TRIGGER_14;
			return 0;
		}
	} while ((REG8(_UART_LSR) & BSP_LSR_THRE) == BSP_TxCHAR_AVAIL);

	/* Send Character */
	REG8(_UART_THR) = c;

	return 1;
}


void  __init prom_console_init(void)
{	
#ifdef CONFIG_CPU0_DIS_UART	
	REG32(SOC_6266_UART1_PINMUX) &= ~((1<<3) | (1<<5));
#else
	/* pin mux contrl register n.21& n.20 for enabling GPIO_SELUART1&GPIO_SELUART1_CTRL*/
#ifdef CONFIG_USE_UART1
	//UART1 needs to be configured first for the full boot-up message to be displayed correctly.
	/* For UART 1 pin-mux settings */
	#define SOC_0371_UART1_PINMUX     (0xbb02301c)
	#define SOC_6266_IO_LED_EN_REG_A  (0xbb023014)
	#define LED8_EN (0x1 << 8)
	#define LED6_EN (0x1 << 6)
	#define SOC_6266_UART1_PINMUX     (0xbb023018)
	#define GPIO_CRTL_2_REG_A         (0xBB0000F0)
	#define GPIO13_EN (0x1 << 13)
	#define GPIO24_EN (0x1 << 24)

	/* Turn off GPIO 13/24 */
	REG32(GPIO_CRTL_2_REG_A) &= ~(GPIO13_EN | GPIO24_EN);
	/* Turn off LED 6/8 */
	REG32(SOC_6266_IO_LED_EN_REG_A) &= ~(LED6_EN | LED8_EN);
	/* Turn on UART1 */
	REG32(SOC_6266_UART1_PINMUX) |= (1<<4);
#endif 

	/* 8 bits, 1 stop bit, no parity. */
	REG8(_UART_LCR) = BSP_CHAR_LEN_8 | BSP_ONE_STOP | BSP_PARITY_DISABLE;

	/* Reset/Enable the FIFO */
	REG8(_UART_FCR) = BSP_FCR_EN | BSP_RXRST | BSP_TXRST | BSP_CHAR_TRIGGER_14;

	/* Disable All Interrupts */
	REG8(_UART_IER) = 0x00000000;

	/* Enable Divisor Latch */
	REG8(_UART_LCR) |= BSP_DLAB;

	/* Set Divisor */
	REG8(_UART_DLL) = (BSP_SYSCLK / (BSP_BAUDRATE * 16) - 1) & 0x00FF;
	REG8(_UART_DLM) = ((BSP_SYSCLK / (BSP_BAUDRATE * 16) - 1) & 0xFF00) >> 8;

	/* Disable Divisor Latch */
	REG8(_UART_LCR) &= (~BSP_DLAB);


#if (CONFIG_SERIAL_8250_RUNTIME_UARTS > 1)
	/*UART 1*/

	/* 8 bits, 1 stop bit, no parity. */
	REG8(_UART1_LCR) = BSP_CHAR_LEN_8 | BSP_ONE_STOP | BSP_PARITY_DISABLE;

	/* Reset/Enable the FIFO */
	REG8(_UART1_FCR) = BSP_FCR_EN | BSP_RXRST | BSP_TXRST | BSP_CHAR_TRIGGER_14;

	/* Disable All Interrupts */
	REG8(_UART1_IER) = 0x00000000;

	/* Enable Divisor Latch */
	REG8(_UART1_LCR) |= BSP_DLAB;

	/* Set Divisor */
	REG8(_UART1_DLL) = (BSP_SYSCLK / (BSP_BAUDRATE * 16) - 1) & 0x00FF;
	REG8(_UART1_DLM) = ((BSP_SYSCLK / (BSP_BAUDRATE * 16) - 1) & 0xFF00) >> 8;

	/* Disable Divisor Latch */
	REG8(_UART1_LCR) &= (~BSP_DLAB);

#endif /* #if (CONFIG_SERIAL_8250_RUNTIME_UARTS > 1) */

#endif /* CONFIG_CPU0_DIS_UART */
}

void __init bsp_serial_init(void)
{
	struct uart_port s;

	/* clear memory */
	memset(&s, 0, sizeof(s));

	s.line = 0;
	s.type = PORT_16550A;
	s.irq = _UART_IRQ;
	s.iotype = UPIO_MEM;
	s.regshift = 2;

	s.uartclk = BSP_SYSCLK - BSP_BAUDRATE * 24; 
	s.fifosize = 1;
	s.flags = UPF_SKIP_TEST /*| UPF_LOW_LATENCY*/ | UPF_SPD_CUST;
	s.membase = (unsigned char *)_UART_BASE;
	s.custom_divisor = BSP_SYSCLK / (BSP_BAUDRATE * 16) - 1;

	if (early_serial_setup(&s) != 0) {
		panic("RTL8672: bsp_serial_init failed!");
	}

#if (CONFIG_SERIAL_8250_RUNTIME_UARTS > 1)
	/* /dev/ttyS1 */
	s.line = 1;
	s.type = PORT_16550A;
	s.irq = _UART1_IRQ;
	s.iotype = UPIO_MEM;
	s.regshift = 2;
	
	s.uartclk = BSP_SYSCLK - BSP_BAUDRATE * 24; 
	s.fifosize = 1;
	s.flags = UPF_SKIP_TEST /*| UPF_LOW_LATENCY*/ | UPF_SPD_CUST;
	s.membase = (unsigned char *) _UART1_BASE;
	s.custom_divisor = BSP_SYSCLK / (BSP_BAUDRATE * 16) - 1;
	
	if (early_serial_setup(&s) != 0) {
		panic("luna: bsp_serial_init ttyS1 failed!");
	}
#endif /* #if (CONFIG_SERIAL_8250_RUNTIME_UARTS > 1) */

}

