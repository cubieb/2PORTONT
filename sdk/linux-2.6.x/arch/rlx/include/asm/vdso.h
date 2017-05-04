/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2009 Cavium Networks
 */

#ifndef __ASM_VDSO_H
#define __ASM_VDSO_H

#include <linux/types.h>

struct mips_vdso {
	u32 signal_trampoline[2];
	u32 rt_signal_trampoline[2];
};

#endif /* __ASM_VDSO_H */
