/*
 * Copyright 2001 MontaVista Software Inc.
 * Author: Jun Sun, jsun@mvista.com or jsun@junsun.net
 *
 * Copyright (C) 2001 Ralf Baechle
 * Copyright (C) 2005  MIPS Technologies, Inc.  All rights reserved.
 *      Author: Maciej W. Rozycki <macro@mips.com>
 *
 * This file define the irq handler for MIPS CPU interrupts.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

/*
 * Almost all MIPS CPUs define 8 interrupt sources.  They are typically
 * level triggered (i.e., cannot be cleared from CPU; must be cleared from
 * device).  The first two are software interrupts which we don't really
 * use or support.  The last one is usually the CPU timer interrupt if
 * counter register is present or, for CPUs with an external FPU, by
 * convention it's the FPU exception interrupt.
 *
 * Don't even think about using this on SMP.  You have been warned.
 *
 * This file exports one global function:
 *	void rlx_cpu_irq_init(int irq_base);
 */
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>

#include <asm/irq_cpu.h>
#include <asm/rlxregs.h>
#include <asm/system.h>

static int rlx_cpu_irq_base;

static inline void unmask_rlx_irq(unsigned int irq)
{
  set_c0_status(0x100 << (irq - rlx_cpu_irq_base));
  irq_enable_hazard();
}

static inline void mask_rlx_irq(unsigned int irq)
{
  clear_c0_status(0x100 << (irq - rlx_cpu_irq_base));
  irq_disable_hazard();
}

static struct irq_chip rlx_cpu_irq_controller = {
	.name		= "RLX",
	.ack		= mask_rlx_irq,
	.mask		= mask_rlx_irq,
	.mask_ack	= mask_rlx_irq,
	.unmask		= unmask_rlx_irq,
	.eoi		= unmask_rlx_irq,
};

void __init rlx_cpu_irq_init(int irq_base)
{
  int i;

  /* Mask interrupts. */
  clear_c0_status(ST0_IM);
  clear_c0_cause(CAUSEF_IP);

  rlx_cpu_irq_base = irq_base;

  for (i = irq_base + 2; i < irq_base + 8; i++)
    set_irq_chip_and_handler(i, &rlx_cpu_irq_controller, handle_percpu_irq);
}
