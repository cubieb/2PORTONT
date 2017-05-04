/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, <wd@denx.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <asm/mipsregs.h>
#include <soc.h>

int do_reset_sw(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]) {
	void (*f)(void) = (void *)0xbfc00000;
	f();
	fprintf(stderr, "*** system reset failed ***\n");
	return 0;
}

U_BOOT_CMD(
        reset_sw, 1, 0,    do_reset_sw,
        "Perform software RESET",
        ""
);

int do_reset_all(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]) {
	return do_reset_all(cmdtp, flag, argc, argv);
}

extern void rlx5281_cache_flush_dcache(void);
extern void rlx5281_cache_invali_icache(void);
void flush_cache(ulong start_addr, ulong size)
{
	rlx5281_cache_flush_dcache();
	rlx5281_cache_invali_icache();
}

/* luna needs no interrupt in UBoot, so just left INTVEC
   alone. This function is left here for the sake of
   control flow. */
void set_exception_base(u32_t ebase) {
#if 0
	/* Change INTVEC (LXCP0 $2, 1). */
	__asm__ __volatile__ ("mtlxc0 %0, $2;\n"
	                      ::"r" (ebase));
#endif

	return;
}
