#ifndef __AIPC_DEBUG_H__
#define __AIPC_DEBUG_H__

#undef  ADEBUG

#ifdef CONFIG_RTL8686_IPC_DEBUG_MESSAGE
#define AIPC_DEBUG
#define AIPC_DEBUG_SWITCH
#endif

#ifdef AIPC_DEBUG
#  ifdef __KERNEL__
#    ifdef AIPC_DEBUG_SWITCH
#        define ADEBUG(flg , fmt, args...)  if(flg & ACTSW.dbg_mask) \
				printk("aipc: (%04d)%s  " fmt, __LINE__ , __FUNCTION__ , ## args)
#    else
#        define ADEBUG(fmt, args...)  printk("aipc: (%04d)%s  " fmt , __LINE__ , __FUNCTION__ , ## args)
#    endif
#  else 
#  ifdef __ECOS
#    ifdef AIPC_DEBUG_SWITCH
#        define ADEBUG(flg , fmt, args...)  if(flg & ACTSW.dbg_mask) \
				printk("aipc: (%04d)%s  " fmt , __LINE__ , __FUNCTION__ , ## args)
#    else
#        define ADEBUG(fmt, args...)  printk("aipc: (%04d)%s  " fmt , __LINE__ , __FUNCTION__ , ## args)
#    endif
#  endif
#  endif

#  define SDEBUG(fmt, args...)  printk("aipc: (%04d)%s  " fmt , __LINE__ , __FUNCTION__ , ## args)

#else
#  define ADEBUG(fmt, args...) /* not debugging: nothing */
#  define SDEBUG(fmt, args...) /* not debugging: nothing */
#endif

#endif
