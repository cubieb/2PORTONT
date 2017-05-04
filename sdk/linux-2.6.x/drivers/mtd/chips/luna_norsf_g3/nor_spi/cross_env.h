#ifndef _CROSS_ENV_H_
#define _CROSS_ENV_H_

#if defined(__LUNA_KERNEL__)
#include <linux/kernel.h>
#include <linux/module.h>
#include <luna_cfg.h>
#include <asm/regdef.h>
#include <asm/cacheflush.h>
#define puts(...)          printk(__VA_ARGS__)
#define printf(...)        printk(__VA_ARGS__)
#define inline_memcpy(...) memcpy(__VA_ARGS__)
#ifndef GET_CPU_MHZ
	#define GET_CPU_MHZ()    (1000)
#endif

#ifdef noinline
#undef noinline
#endif
#define IDCACHE_FLUSH() do { \
		__flush_cache_all(); \
	} while (0)
#define DCACHE_LINE_SZ_B CACHELINE_SIZE

#elif defined(CONFIG_UNDER_UBOOT)
#include <common.h>
#include <asm/mipsregs.h>
#define inline_memcpy(...) memcpy(__VA_ARGS__)
#define IDCACHE_FLUSH() do {	  \
		flush_cache(0, 0); \
	} while (0)
#ifndef GET_CPU_MHZ
	#define GET_CPU_MHZ()    cg_query_freq(CG_DEV_OCP)
#endif
#define DCACHE_LINE_SZ_B CONFIG_SYS_CACHELINE_SIZE

#else
#include <cpu/cpu.h>
#define IDCACHE_FLUSH() do {	  \
		_bios.dcache_writeback_invalidate_all(); \
		_bios.icache_invalidate_all(); \
	} while (0)
#ifndef GET_CPU_MHZ
	#define GET_CPU_MHZ()    cg_query_freq(CG_DEV_OCP)
#endif
#define DCACHE_LINE_SZ_B DCACHE_LINE_SIZE
#endif

#endif
