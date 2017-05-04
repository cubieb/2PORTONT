/*
 *  linux/arch/rlx/rlxocp/time.c
 *
 *  Copyright (C) 1999 Harald Koerfgen
 *  Copyright (C) 2000 Pavel Machek (pavel@suse.cz)
 *  Copyright (C) 2001 Steven J. Hill (sjhill@realitydiluted.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Time handling functinos for Philips Nino.
 */
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/param.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/timex.h>
#include <linux/delay.h>
#include <asm/time.h>
#include <asm/rlxbsp.h>
#include  "bspchip.h"

#ifdef CONFIG_RTL_TIMER_ADJUSTMENT
#include <rtl_types.h>
#include <rtl865xc_asicregs.h>

void rtl865x_setupTimer1(void)
{
	WRITE_MEM32( TCCNR, READ_MEM32(TCCNR) & ~TC1EN );/* Disable timer1 */
	WRITE_MEM32( TC1DATA, 0xffffff00);	
	WRITE_MEM32( TCCNR, ( READ_MEM32(TCCNR) | TC1EN ) | TC1MODE_TIMER );/* Enable timer1 - timer mode */
	WRITE_MEM32( TCIR, READ_MEM32(TCIR) & ~TC1IE ); /* Disable timer1 interrupt */
}
#endif

extern unsigned int SOC_ID;
extern void (*bsp_timer_ack)(void);

void old_bsp_timer_ack(void)
{
	REG32(BSP_TCIR) = (REG32(BSP_TCIR)|BSP_TC0IE|BSP_TC0IP) & (~(BSP_TC1IP|BSP_TC2IP|BSP_TC3IP|BSP_TC4IP|BSP_TC5IP));
}

void new_bsp_timer_ack(void)
{
	REG32(BSP_TC0INT) |= (BSP_TCIP);
}

void __init bsp_timer_init(void)
{

	if(SOC_ID == 0x0371){
		/* Clear Timer IP status */
		if (REG32(BSP_TCIR) & BSP_TC0IP)
			REG32(BSP_TCIR) |= BSP_TC0IP;
	
		/* disable timer */
		REG32(BSP_TCCNR) = 0; /* disable timer before setting CDBR */
	
		/* initialize timer registers */
		REG32(BSP_CDBR)=(BSP_DIVISOR) << 16;
		REG32(BSP_TC0DATA) = ( (BSP_MHZ)*((BSP_DIVISOR)/HZ)) << 4;
	}
	else{
		/* Clear Timer IP status */
		if (REG32(BSP_TC0INT) & BSP_TCIP) {
			REG32(BSP_TC0INT) |= (BSP_TCIP);
		}
	
		/* disable timer */
		REG32(BSP_TC0CTL) = 0x0; /* disable timer before setting CDBR */
	
		/* initialize timer registers */
		REG32(BSP_TC0CTL) |= (BSP_DIVISOR) << 0;
		REG32(BSP_TC0DATA) = ( (BSP_MHZ)*((BSP_DIVISOR)/HZ)) << 0;
	}
		
	/* hook up timer interrupt handler */
	rlx_clockevent_init(BSP_TC0_IRQ);
	
	if(SOC_ID == 0x0371){
		bsp_timer_ack = old_bsp_timer_ack;
		/* enable timer */
		REG32(BSP_TCCNR) = BSP_TC0EN | BSP_TC0MODE_TIMER;
		REG32(BSP_TCIR) = BSP_TC0IE;
	}
	else{
		bsp_timer_ack = new_bsp_timer_ack;
		/* enable timer */
		REG32(BSP_TC0CTL) |= BSP_TCEN | BSP_TCMODE_TIMER;
		REG32(BSP_TC0INT) |= BSP_TCIE;
	}
}

#ifdef CONFIG_RTK_VOIP
void timer1_enable(void)
{
	printk( "timer1_enable not implement!!\n" );
}

void timer1_disable(void)
{
	printk( "timer1_disable not implement!!\n" );
}

#ifdef CONFIG_RTL_WTDOG
int bBspWatchdog = 0;

void bsp_enable_watchdog( void )
{
	bBspWatchdog = 1;
	*(volatile unsigned long *)(0xb800311C)=0x00600000;
}

void bsp_disable_watchdog( void )
{
	*(volatile unsigned long *)(0xb800311C)=0xA5600000;
	bBspWatchdog = 0;
}
#endif // CONFIG_RTL_WTDOG

#endif  // CONFIG_RTK_VOIP