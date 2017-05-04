#ifndef __AIPC_CACHE_H__
#define __AIPC_CACHE_H__

//#include "aipc_global.h"
//#include "soc_type.h"


/*
*	Apollo IPC cache operations
*	Wrapper API for Apollo IPC
*/
#ifdef AIPC_CACHE_FLUSH
static inline void apic_flush_dcache(unsigned long start, unsigned long end)
{
#if   defined(_AIPC_CPU_) && defined(CONFIG_CPU_RLX5281)
	flush_cache_range(start , end); 
	// Linux: arch/rlx/cache.c arch/rlx/cache-rlx.c
#elif defined(_AIPC_DSP_) && defined(CONFIG_CPU_RLX5181)
	rlx_dcache_flush_range(start, end);
	// eCos:  ecos-3.0/packages/hal/mips/rlx/v3_0/src/var_misc.c
#elif defined(_AIPC_CPU_) && defined(CONFIG_CPU_RLX5181)
	#error "not supported type"
#elif defined(_AIPC_DSP_) && defined(CONFIG_CPU_RLX5281)
	#error "not supported type"
#endif
}
#endif

#endif
