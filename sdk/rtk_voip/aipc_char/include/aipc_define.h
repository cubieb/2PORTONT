#ifndef _AIPC_DEFINE_H_
#define _AIPC_DEFINE_H_

#define AIPC_USE_SECTION
#define AIPC_USE_ALIGN

#ifdef AIPC_USE_SECTION
#define AIPC_SECTION(X) 	__attribute__((unused, section((X))))
#define AIPC_SRAM_SECTION   __attribute__((unused, __section__(".aipc.sram")))
#define AIPC_DRAM_SECTION   __attribute__((unused, __section__(".aipc.dram")))
#else
#define AIPC_SECTION(X)
#define AIPC_SRAM_SECTION
#define AIPC_DRAM_SECTION
#endif

#ifdef AIPC_USE_ALIGN
#define AIPC_ALIGN4			__attribute__((aligned(4)))
#define AIPC_ALIGN32		__attribute__((aligned(32)))

#else
#define AIPC_ALIGN
#endif


// Protection Macro
//#define AIPC_PROTECT_MACRO

#ifdef AIPC_PROTECT_MACRO

 #if defined(__KERNEL__)
  #define AIPC_DISABLE_INTERRUPTS				unsigned long flags; local_irq_save(flags)
  #define AIPC_RESTORE_INTERRUPTS				local_irq_restore(flags)
 #elif defined(__ECOS)
  #define AIPC_DISABLE_INTERRUPTS				cyg_uint32 _old ; HAL_DISABLE_INTERRUPTS(_old)
  #define AIPC_RESTORE_INTERRUPTS				HAL_RESTORE_INTERRUPTS(_old)
 #endif

#else

  #define AIPC_DISABLE_INTERRUPTS
  #define AIPC_RESTORE_INTERRUPTS

#endif

#endif

