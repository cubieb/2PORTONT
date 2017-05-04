/*
 * Copyright (c) 2006, Realtek Semiconductor Corp.
 *
 * rlxbsp.h:
 *   board module interface
 *
 * Tony Wu (tonywu@realtek.com)
 * Oct. 30, 2006
 */

#ifndef _RLXBSP_H_
#define _RLXBSP_H_

/*
 * Function prototypes
 */
#ifndef _LANGUAGE_ASSEMBLY

#include <linux/linkage.h>

/* bsp/serial.c */
extern void bsp_serial_init(void);

/* bsp/time.c */
extern void bsp_timer_init(void);
//extern void bsp_timer_ack(void);

/* kernel/time.c */
extern int rlx_clockevent_init(int irq);
#ifdef CONFIG_RTL_TIMER2
extern void bsp_timer_ack_2(void);
extern int rlx_clockevent_init_2(int irq);
#endif

/*
 * IMEM/DMEM attributes
 */
#define TO_SECT(X) __attribute__((section((X))))
#define TO_IMEM    __attribute__((section(".rlx_imem")))
#define TO_DMEM    __attribute__((section(".rlx_dmem")))

#define __SRAM_TEXT	__attribute__  ((section(".sram_text")))
#define __SRAM_DATA	__attribute__  ((section(".sram_data")))

#endif

#endif
