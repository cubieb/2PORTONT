/*
 * Realtek Semiconductor Corp.
 *
 * cache-rlx.c: RLX specific mmu/cache code.
 *
 * Tony Wu (tonywu@realtek.com)
 * Dec. 07, 2008
 */
#include <linux/hardirq.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/mm.h>

#include <asm/page.h>
#include <asm/pgtable.h>
#include <asm/mmu_context.h>
#include <asm/system.h>
#include <asm/io.h>
#include <asm/cpu.h>
#include <asm/cpu-features.h>

#include <asm/rlxbsp.h>

/*
 * Determine whether CPU has CACHE OP
 */
#if defined(CONFIG_CPU_RLX4181) || defined(CONFIG_CPU_RLX5181) || \
    defined(CONFIG_CPU_RLX4281) || defined(CONFIG_CPU_RLX5281)
  #define CONFIG_CPU_HAS_DCACHE_OP
#else
  #undef CONFIG_CPU_HAS_DCACHE_OP
#endif

#if defined(CONFIG_CPU_RLX4281) || defined(CONFIG_CPU_RLX5281)
  #define CONFIG_CPU_HAS_ICACHE_OP
#else
  #undef CONFIG_CPU_HAS_ICACHE_OP
#endif

/*
 *  CACHE OP
 *   0x10 = IInval
 *   0x11 = DInval
 *   0x15 = DWBInval
 *   0x19 = DWB
 *   0x1b = DWB_IInval
 */
#if defined(CONFIG_CPU_HAS_WBC) || defined(CONFIG_CPU_HAS_L2C)
  #define CACHE_DCACHE_FLUSH	0x15
  #define CACHE_DCACHE_WBACK	0x19
#else
  #define CACHE_DCACHE_FLUSH	0x11
  #define CACHE_DCACHE_WBACK	0x11
#endif

#ifdef CONFIG_CPU_HAS_WBIC
  #define CACHE_ICACHE_FLUSH	0x1b
#else
  #define CACHE_ICACHE_FLUSH	0x10
#endif

#define CACHE_OP(op, p)          \
    __asm__ __volatile__ (       \
         ".set  push\n"          \
         ".set  noreorder\n"     \
         "cache %0, 0x000(%1)\n" \
         ".set  pop\n"           \
         : : "i" (op), "r" (p)   \
    )

#define CACHE32_UNROLL4(op, p)   \
    __asm__ __volatile__ (       \
         ".set  push\n"          \
         ".set  noreorder\n"     \
         "cache %0, 0x000(%1)\n" \
         "cache %0, 0x020(%1)\n" \
         "cache %0, 0x040(%1)\n" \
         "cache %0, 0x060(%1)\n" \
         ".set  pop\n"           \
         : : "i" (op), "r" (p)   \
    )

#define CACHE16_UNROLL8(op, p)   \
    __asm__ __volatile__ (       \
         ".set  push\n"          \
         ".set  noreorder\n"     \
         "cache %0, 0x000(%1)\n" \
         "cache %0, 0x010(%1)\n" \
         "cache %0, 0x020(%1)\n" \
         "cache %0, 0x030(%1)\n" \
         "cache %0, 0x040(%1)\n" \
         "cache %0, 0x050(%1)\n" \
         "cache %0, 0x060(%1)\n" \
         "cache %0, 0x070(%1)\n" \
         ".set  pop\n"           \
         : : "i" (op), "r" (p)   \
    )

#if (cpu_dcache_line == 32)
  #define DCACHE_OP(op,p)  CACHE32_UNROLL4(op,(p))
#else
  #define DCACHE_OP(op,p)  CACHE16_UNROLL8(op,(p))
#endif

#if (cpu_icache_line == 32)
  #define ICACHE_OP(op,p)  CACHE32_UNROLL4(op,(p))
#else
  #define ICACHE_OP(op,p)  CACHE16_UNROLL8(op,(p))
#endif

/*
 *  CCTL OP
 *   0x1   = DInval
 *   0x2   = IInval
 *   0x100 = DWB
 *   0x200 = DWB_Inval
 */
#define CCTL_ICACHE_FLUSH		0x2
#if defined(CONFIG_CPU_HAS_WBC) || defined(CONFIG_CPU_HAS_L2C)
  #define CCTL_DCACHE_WBACK		0x100
  #define CCTL_DCACHE_FLUSH		0x200
#else
  #define CCTL_DCACHE_WBACK		0x1
  #define CCTL_DCACHE_FLUSH		0x1
#endif

#if defined(CONFIG_CPU_RLX4281) || defined(CONFIG_CPU_RLX5281)
#define CCTL_OP(op)		\
    __asm__ __volatile__(	\
       ".set  push\n"		\
       ".set  noreorder\n"	\
       "mtc0	$0, $20\n"	\
       "li	$8, %0\n"	\
       "mtc0	$8, $20\n"	\
       ".set  pop\n"		\
       : : "i" (op)		\
   )
#else
#define CCTL_OP(op)		\
    __asm__ __volatile__(	\
       ".set  push\n"		\
       ".set  noreorder\n"	\
       "mfc0	$8, $20\n"	\
       "ori	$8, %0\n"	\
       "xori	$9, $8, %0\n"	\
       "mtc0	$9, $20\n"	\
       "mtc0	$8, $20\n"	\
       ".set pop\n"		\
       : : "i" (op)		\
   )
#endif

/*
 * Dummy cache handling routines for machines without boardcaches
 */
static void cache_noop(void) {}

static inline void rlx_flush_dcache_fast(unsigned long start, unsigned long end)
{
	unsigned long p;

	for (p = start; p <= end-0x080; p += 0x080) {
        DCACHE_OP(CACHE_DCACHE_FLUSH, p);
      }

	p = p & ~(cpu_dcache_line -1);
	while (p < end){
		CACHE_OP(CACHE_DCACHE_FLUSH, p);
                p += cpu_dcache_line;
        }
}

static inline void rlx_wback_dcache_fast(unsigned long start, unsigned long end)
{
	unsigned long p;

	for (p = start; p <= end-0x080; p += 0x080) {
		DCACHE_OP(CACHE_DCACHE_WBACK, p);
	}

	p = p & ~(cpu_dcache_line -1);
	while (p < end){
		CACHE_OP(CACHE_DCACHE_WBACK, p);
                p += cpu_dcache_line;
        }
    }

static inline void rlx_flush_icache_fast(unsigned long start, unsigned long end)
{
	unsigned long p;

	for (p = start; p <= end-0x080; p += 0x080) {
        ICACHE_OP(CACHE_ICACHE_FLUSH, p);
    }

	p = p & ~(cpu_icache_line -1);
	while (p < end){
		CACHE_OP(CACHE_ICACHE_FLUSH, p);
                p += cpu_icache_line;
       } 
}

/*
 * DCACHE part
 */
__IRAM_SYS_HIGH static /*inline*/ void rlx_flush_dcache_range(unsigned long start, unsigned long end)
{
#ifdef CONFIG_CPU_HAS_DCACHE_OP
	if (end - start > cpu_dcache_size) {
		CCTL_OP(CCTL_DCACHE_FLUSH);
        return;
      }
	rlx_flush_dcache_fast(start, end);
#else
	CCTL_OP(CCTL_DCACHE_FLUSH);
#endif
}

static inline void rlx_wback_dcache_range(unsigned long start, unsigned long end)
{
#ifdef CONFIG_CPU_HAS_DCACHE_OP
	if (end - start > cpu_dcache_size) {
		CCTL_OP(CCTL_DCACHE_WBACK);
		return;
	}
	rlx_wback_dcache_fast(start, end);
#else
	CCTL_OP(CCTL_DCACHE_WBACK);
#endif
}

static inline void local_rlx_flush_data_cache_page(void *addr)
{
#if defined(CONFIG_CPU_HAS_WBC) || defined(CONFIG_CPU_HAS_L2C)
# ifdef CONFIG_CPU_HAS_DCACHE_OP
	rlx_flush_dcache_fast((unsigned long)addr, (unsigned long)addr + PAGE_SIZE - 1);
# else
	CCTL_OP(CCTL_DCACHE_FLUSH);
# endif
#endif
}

static void rlx_flush_data_cache_page(unsigned long addr)
{
#if defined(CONFIG_CPU_HAS_WBC) || defined(CONFIG_CPU_HAS_L2C)
	if (in_atomic())
		local_rlx_flush_data_cache_page((void *)addr);
	else {
		preempt_disable();
		local_rlx_flush_data_cache_page((void *)addr);
		preempt_enable();
	}
#endif
}

/*
 * ICACHE part
 */
static inline void local_rlx_flush_icache_range(unsigned long start, unsigned long end)
{
#ifdef CONFIG_CPU_HAS_ICACHE_OP
    unsigned long size;

  #if !defined(CONFIG_CPU_HAS_WBIC) \
	&& (defined(CONFIG_CPU_HAS_WBC) \
		|| defined(CONFIG_CPU_HAS_L2C))
	rlx_flush_dcache_range(start, end);
  #endif

    size = end - start;
	if (size > cpu_icache_size) {
  #ifdef CONFIG_CPU_HAS_WBIC
		if (size > cpu_dcache_size) {
			CCTL_OP(CCTL_ICACHE_FLUSH | CCTL_DCACHE_FLUSH);
        return;
		} else
		  rlx_flush_dcache_fast(start, end);
#endif
		CCTL_OP(CCTL_ICACHE_FLUSH);
		return;
    }

	rlx_flush_icache_fast(start, end);
#else
	rlx_flush_dcache_range(start, end);
	CCTL_OP(CCTL_ICACHE_FLUSH);
#endif
}

static void rlx_flush_icache_range(unsigned long start, unsigned long end)
{
    preempt_disable();
    local_rlx_flush_icache_range(start, end);
    preempt_enable();
}

static void rlx_flush_cache_range(struct vm_area_struct *vma,
				  unsigned long start, unsigned long end)
{
#if defined(CONFIG_CPU_HAS_WBC) || defined(CONFIG_CPU_HAS_L2C)
	if (cpu_context(smp_processor_id(), vma->vm_mm) == 0)
		return;

	preempt_disable();
# ifdef CONFIG_CPU_HAS_WBIC
	if (vma->vm_flags & VM_EXEC)
		CCTL_OP(CCTL_ICACHE_FLUSH | CCTL_DCACHE_FLUSH);
	else
		CCTL_OP(CCTL_DCACHE_FLUSH);
# else
	CCTL_OP(CCTL_DCACHE_FLUSH);
	if (vma->vm_flags & VM_EXEC)
		CCTL_OP(CCTL_ICACHE_FLUSH);
# endif
	preempt_enable();
#endif
}

/*
 * CACHE part
 */
static inline void rlx___cache_flush_all(void)
{
#ifdef CONFIG_CPU_HAS_WBIC
	CCTL_OP(CCTL_ICACHE_FLUSH | CCTL_DCACHE_FLUSH);
#else
	CCTL_OP(CCTL_DCACHE_FLUSH);
	CCTL_OP(CCTL_ICACHE_FLUSH);
#endif
}

static void rlx_flush_cache_page(struct vm_area_struct *vma,
				 unsigned long addr, unsigned long pfn)
{
	unsigned long kaddr = KSEG0ADDR(pfn << PAGE_SHIFT);
	int exec = vma->vm_flags & VM_EXEC;
	struct mm_struct *mm = vma->vm_mm;
	pgd_t *pgdp;
	pud_t *pudp;
	pmd_t *pmdp;
	pte_t *ptep;

	pr_debug("cpage[%08lx,%08lx]\n",
		 cpu_context(smp_processor_id(), mm), addr);

	/* No ASID => no such page in the cache.  */
	if (cpu_context(smp_processor_id(), mm) == 0)
		return;

	pgdp = pgd_offset(mm, addr);
	pudp = pud_offset(pgdp, addr);
	pmdp = pmd_offset(pudp, addr);
	ptep = pte_offset(pmdp, addr);

	/* Invalid => no such page in the cache.  */
	if (!(pte_val(*ptep) & _PAGE_PRESENT))
		return;

	preempt_disable();
#ifdef CONFIG_CPU_HAS_WBIC
	if (exec)
		rlx_flush_icache_fast(kaddr, kaddr + PAGE_SIZE - 1);
	else
		rlx_flush_dcache_fast(kaddr, kaddr + PAGE_SIZE - 1);
#else
	rlx_flush_dcache_fast(kaddr, kaddr + PAGE_SIZE - 1);
	if (exec)
		rlx_flush_icache_fast(kaddr, kaddr + PAGE_SIZE - 1);
#endif
	preempt_enable();
}

static void rlx_flush_cache_sigtramp(unsigned long addr)
{
    pr_debug("csigtramp[%08lx]\n", addr);

#ifdef CONFIG_CPU_HAS_WBIC
    CACHE_OP(0x1b, addr);
 #else
  #ifdef CONFIG_CPU_HAS_DCACHE_OP
    CACHE_OP(CACHE_DCACHE_FLUSH, addr);
  #else
    CCTL_OP(CCTL_DCACHE_WBACK);
#endif

  #ifdef CONFIG_CPU_HAS_ICACHE_OP
    CACHE_OP(CACHE_ICACHE_FLUSH, addr);
#else
    CCTL_OP(CCTL_ICACHE_FLUSH);
  #endif
#endif
}

__IRAM_SYS_HIGH static void rlx_dma_cache_wback_inv(unsigned long start, unsigned long size)
{
	/* Catch bad driver code */
	BUG_ON(size == 0);

	iob();
    rlx_flush_dcache_range(start, start + size);
}

static void rlx_dma_cache_wback(unsigned long start, unsigned long size)
{
	/* Catch bad driver code */
	BUG_ON(size == 0);

	iob();
	rlx_wback_dcache_range(start, start + size);
}

void __cpuinit rlx_cache_init(void)
{
	extern void build_clear_page(void);
	extern void build_copy_page(void);

          flush_cache_all     =   (void *) cache_noop;
        __flush_cache_all     =   rlx___cache_flush_all;
          flush_cache_mm      =   (void *) cache_noop;
          flush_cache_range   =   rlx_flush_cache_range;
          flush_cache_page    =   rlx_flush_cache_page;
          flush_icache_range  =   rlx_flush_icache_range;
          local_flush_icache_range  =   local_rlx_flush_icache_range;
	  local_flush_data_cache_page = local_rlx_flush_data_cache_page;
	  flush_data_cache_page = rlx_flush_data_cache_page;
	  flush_cache_sigtramp  = rlx_flush_cache_sigtramp;

	_dma_cache_wback_inv = rlx_dma_cache_wback_inv;
	_dma_cache_wback = rlx_dma_cache_wback;
	_dma_cache_inv = rlx_dma_cache_wback_inv;

    printk("icache: %dkB/%dB, dcache: %dkB/%dB, scache: %dkB/%dB\n",
           cpu_icache_size >> 10, cpu_icache_line,
           cpu_dcache_size >> 10, cpu_dcache_line,
           cpu_scache_size >> 10, cpu_scache_line);

	build_clear_page();
	build_copy_page();
        rlx___cache_flush_all();
}
