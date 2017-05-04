/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2003, 04, 05 Ralf Baechle (ralf@linux-mips.org)
 * Copyright (C) 2007  Maciej W. Rozycki
 * Copyright (C) 2008  Thiemo Seufer
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/proc_fs.h>

#include <asm/bugs.h>
#include <asm/cacheops.h>
#include <asm/inst.h>
#include <asm/io.h>
#include <asm/page.h>
#include <asm/pgtable.h>
#include <asm/system.h>
#include <asm/bootinfo.h>
#include <asm/rlxregs.h>
#include <asm/mmu_context.h>
#include <asm/cpu.h>

#include <asm/uasm.h>

/* Registers used in the assembled routines. */
#define ZERO 0
#define AT 2
#define A0 4
#define A1 5
#define A2 6
#define T0 8
#define T1 9
#define T2 10
#define T3 11
#define T9 25
#define RA 31

/* Handle labels (which must be positive integers). */
enum label_id {
	label_clear = 1,
	label_copy_both,
	label_copy_store,
};

UASM_L_LA(_clear)
UASM_L_LA(_copy_both)
UASM_L_LA(_copy_store)

/* We need one branch and therefore one relocation per target label. */
static struct uasm_label __cpuinitdata labels[3];
static struct uasm_reloc __cpuinitdata relocs[3];

/*
 * Maximum sizes:
 *
 * R4000 128 bytes S-cache:		0x058 bytes
 * With prefetching, 16 word strides	0x120 bytes
 */

static u32 clear_page_array[0x120 / 4];
void clear_page(void * page) __attribute__((alias("clear_page_array")));
EXPORT_SYMBOL(clear_page);

/*
 * Maximum sizes:
 *
 * R4000 128 bytes S-cache:		0x11c bytes
 * With prefetching, 16 word strides	0x540 bytes
 */
static u32 copy_page_array[0x120 / 4];
void copy_page(void *to, void *from) __attribute__((alias("copy_page_array")));
EXPORT_SYMBOL(copy_page);

static inline void __cpuinit
pg_addiu(u32 **buf, unsigned int reg1, unsigned int reg2, unsigned int off)
{
	UASM_i_ADDIU(buf, reg1, reg2, off);
}

static void __cpuinit build_clear_store(u32 **buf, int off)
{
	uasm_i_sw(buf, ZERO, off, A0);
}

void __cpuinit build_clear_page(void)
{
	u32 *buf = (u32 *)&clear_page_array;
	struct uasm_label *l = labels;
	struct uasm_reloc *r = relocs;
	int i;

	memset(labels, 0, sizeof(labels));
	memset(relocs, 0, sizeof(relocs));

	/*
	 * This algorithm makes the following assumptions:
	 *   - The prefetch bias is a multiple of 2 words.
	 *   - The prefetch bias is less than one page.
	 */
	pg_addiu(&buf, A2, A0, PAGE_SIZE);

	uasm_l_clear(&l, buf);
	build_clear_store(&buf, 0);
	build_clear_store(&buf, 4);
	build_clear_store(&buf, 8);
	build_clear_store(&buf, 12);

	pg_addiu(&buf, A0, A0, 32);

	build_clear_store(&buf, -16);
	build_clear_store(&buf, -12);
	build_clear_store(&buf, -8);
    uasm_il_bne(&buf, &r, A0, A2, label_clear);
	build_clear_store(&buf, -4);

	uasm_i_jr(&buf, RA);
	uasm_i_nop(&buf);

	BUG_ON(buf > clear_page_array + ARRAY_SIZE(clear_page_array));

	uasm_resolve_relocs(relocs, labels);

	pr_debug("Synthesized clear page handler (%u instructions).\n",
		 (u32)(buf - clear_page_array));

	pr_debug("\t.set push\n");
	pr_debug("\t.set noreorder\n");
	for (i = 0; i < (buf - clear_page_array); i++)
		pr_debug("\t.word 0x%08x\n", clear_page_array[i]);
	pr_debug("\t.set pop\n");
}

static void __cpuinit build_copy_load(u32 **buf, int reg, int off)
{
	uasm_i_lw(buf, reg, off, A1);
}

static void __cpuinit build_copy_store(u32 **buf, int reg, int off)
{
	uasm_i_sw(buf, reg, off, A0);
}

void __cpuinit build_copy_page(void)
{
	u32 *buf = (u32 *)&copy_page_array;
	struct uasm_label *l = labels;
	struct uasm_reloc *r = relocs;
	int i;

	memset(labels, 0, sizeof(labels));
	memset(relocs, 0, sizeof(relocs));

	/*
	 * This algorithm makes the following assumptions:
	 *   - All prefetch biases are multiples of 8 words.
	 *   - The prefetch biases are less than one page.
	 *   - The store prefetch bias isn't greater than the load
	 *     prefetch bias.
	 */
	pg_addiu(&buf, A2, A0, PAGE_SIZE);

	uasm_l_copy_both(&l, buf);

	build_copy_load(&buf, T0,   0);
	build_copy_load(&buf, T1,   4);
	build_copy_load(&buf, T2,   8);
	build_copy_load(&buf, T3,  12);
	build_copy_store(&buf, T0,  0);
	build_copy_store(&buf, T1,  4);
	build_copy_store(&buf, T2,  8);
	build_copy_store(&buf, T3, 12);

	pg_addiu(&buf, A1, A1, 32);
	pg_addiu(&buf, A0, A0, 32);

	build_copy_load(&buf, T0, -16);
	build_copy_load(&buf, T1, -12);
	build_copy_load(&buf, T2, -8);
	build_copy_load(&buf, T3, -4);
	build_copy_store(&buf, T0, -16);
	build_copy_store(&buf, T1, -12);
	build_copy_store(&buf, T2, -8);
	uasm_il_bne(&buf, &r, A2, A0, label_copy_both);
	build_copy_store(&buf, T3, -4);

	uasm_i_jr(&buf, RA);
	uasm_i_nop(&buf);

	BUG_ON(buf > copy_page_array + ARRAY_SIZE(copy_page_array));

	uasm_resolve_relocs(relocs, labels);

	pr_debug("Synthesized copy page handler (%u instructions).\n",
		 (u32)(buf - copy_page_array));

	pr_debug("\t.set push\n");
	pr_debug("\t.set noreorder\n");
	for (i = 0; i < (buf - copy_page_array); i++)
		pr_debug("\t.word 0x%08x\n", copy_page_array[i]);
	pr_debug("\t.set pop\n");
}
