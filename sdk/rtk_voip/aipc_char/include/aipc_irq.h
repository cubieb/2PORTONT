#ifndef _AIPC_IRQ_H_
#define _AIPC_IRQ_H_

#include "aipc_global.h"
#include "aipc_reg.h"
#include "aipc_mem.h"

#define AIPC_IRQ_CPU_T_DSP	1		// IPC IRQ line number
#define AIPC_IRQ_DSP_T_CPU	0		// IPC IRQ line number

#ifdef __KERNEL__
#define AIPC_INT_PRIORITY	0		// IPC IRQ priority
#elif defined(__ECOS)
#define AIPC_INT_PRIORITY	0		// IPC IRQ priority
#include <stdio.h>
//#define printk 	printf
#else
#error "not support type"
#endif

/*
*	Variable define
*/
#define INT_NAME_SIZE	32

/*
*	Enum
*/
typedef enum {
	T_CPU = 0,		//Trigger CPU
	T_DSP			//Trigger DSP
} T_COP;

typedef enum {
	CPU_LOCK = 0,		//CPU Lock Mutex
	DSP_LOCK			//DSP Lock Mutex
} MUTEX_LOCK;

typedef enum {
	OWN_NONE = 0,		//Nobody own
	OWN_CPU,		    //CPU own
	OWN_DSP				//DSP own
} MUTEX_OWN;

#ifdef IPC_HW_MUTEX_ASM

static void
aipc_cpu_hwmutex_lock(void)
{
	
	unsigned int tmp_mutex;
	unsigned int tmp_own;
	unsigned int res_mutex;
	unsigned int res_own;
	MUTEX_OWN mt_own = OWN_DSP;
	
	unsigned int addr_mutex     = R_AIPC_CPU_MUTEX;
	unsigned int addr_own       = R_AIPC_OWN_MUTEX;
	unsigned int ptr_res_mutex  = (unsigned int)&res_mutex;
	unsigned int ptr_res_own    = (unsigned int)&res_own;
	
    __asm__ __volatile__ (
      
    "1: 			                    \n"
    "   lw      %0, 0(%4)              	\n"
    "   lw      %1, 0(%5)               \n"
	"   sw      %0, %2               	\n"
	"   sw      %1, %3               	\n"
    "   nop                             \n"
    "   beq     %1 , %6, 1b             \n"
    "   nop                             \n"
	:  "=&r" (tmp_mutex) , "=&r" (tmp_own) , "=m" (ptr_res_mutex) , "=m" (ptr_res_own)
	:  "r" (addr_mutex)  , "r" (addr_own)  , "r" (mt_own)
	:  "memory");

}

static void
aipc_cpu_hwmutex_unlock(void)
{
	REG32(R_AIPC_CPU_MUTEX) = 0;
}

static void
aipc_dsp_hwmutex_lock(void)
{
	
	unsigned int tmp_mutex;
	unsigned int tmp_own;
	unsigned int res_mutex;
	unsigned int res_own;
	MUTEX_OWN mt_own = OWN_CPU;
	
	unsigned int addr_mutex     = R_AIPC_DSP_MUTEX;
	unsigned int addr_own       = R_AIPC_OWN_MUTEX;
	unsigned int ptr_res_mutex  = (unsigned int)&res_mutex;
	unsigned int ptr_res_own    = (unsigned int)&res_own;
	
    __asm__ __volatile__ (
      
    "1: 			                    \n"
    "   lw      %0, 0(%4)              	\n"
    "   lw      %1, 0(%5)               \n"
	"   sw      %0, %2               	\n"
	"   sw      %1, %3               	\n"
    "   nop                             \n"
    "   beq     %1 , %6, 1b             \n"
    "   nop                             \n"
	:  "=&r" (tmp_mutex) , "=&r" (tmp_own) , "=m" (ptr_res_mutex) , "=m" (ptr_res_own)
	:  "r" (addr_mutex)  , "r" (addr_own)  , "r" (mt_own)
	:  "memory");

}

static void
aipc_dsp_hwmutex_unlock(void)
{
	REG32(R_AIPC_DSP_MUTEX) = 0;
}
#endif


#ifdef IPC_HW_MUTEX_CCODE

/*
*	Inline function
*/
static inline void
aipc_mutex_lock(MUTEX_LOCK mut)
{
	volatile unsigned int status=0;
	volatile unsigned int own=OWN_NONE;
	#ifdef HW_MUTEX_RD_CNT
	int i = 0;
	#endif

	if (mut==CPU_LOCK){
		do{
	
			#ifdef HW_MUTEX_RD_CNT
			for (i=0;i<HW_MUTEX_RD_CNT;i++){
				status = REG32(R_AIPC_CPU_MUTEX);
				own    = REG32(R_AIPC_OWN_MUTEX);		
			}
			#else
			status = REG32(R_AIPC_CPU_MUTEX);
			own    = REG32(R_AIPC_OWN_MUTEX);
			#endif

			#ifdef HW_MUTEX_DBG
			if(own==OWN_DSP){
				printk("CPU lock failed.  own=%u\n" , own);
			}		
			else if(own==OWN_CPU){
				printk("CPU lock success. own=%u\n" , own);
			}
			else{
				printk("own=%u\n" , own);
			}
			#endif
		}while( own!=OWN_CPU );
	}
	else {
		do{
			#ifdef HW_MUTEX_RD_CNT
			for (i=0;i<HW_MUTEX_RD_CNT;i++){
				status = REG32(R_AIPC_DSP_MUTEX);
				own    = REG32(R_AIPC_OWN_MUTEX);
			}
			#else
			status = REG32(R_AIPC_DSP_MUTEX);
			own    = REG32(R_AIPC_OWN_MUTEX);	
			#endif
			
			#ifdef HW_MUTEX_DBG
			if(own==OWN_CPU){
				printk("DSP lock failed.  own=%u\n" , own);
			}		
			else if(own==OWN_DSP){
				printk("DSP lock success. own=%u\n" , own);
			}
			else{
				printk("own=%u\n" , own);
			}
			#endif
		}while( own!=OWN_DSP );
	}
}

static inline unsigned int 
aipc_mutex_trylock(MUTEX_LOCK mut)		//need to check lock is success or not
{
	volatile unsigned int status=0;
	volatile unsigned int own=OWN_NONE;

	if (mut==CPU_LOCK){
		status = REG32(R_AIPC_CPU_MUTEX);
		own    = REG32(R_AIPC_OWN_MUTEX);
		
		#ifdef HW_MUTEX_DBG
		if(own==OWN_DSP){
			printk("CPU lock failed.  own=%u\n" , own);
		}		
		else if(own==OWN_CPU){
			printk("CPU lock success. own=%u\n" , own);
		}
		else{
			printk("own=%u\n" , own);
		}
		#endif
	}
	else {
		status = REG32(R_AIPC_DSP_MUTEX);
		own    = REG32(R_AIPC_OWN_MUTEX);
		
		#ifdef HW_MUTEX_DBG
		if(own==OWN_CPU){
			printk("DSP lock failed.  own=%u\n" , own);
		}		
		else if(own==OWN_DSP){
			printk("DSP lock success. own=%u\n" , own);
		}
		else{
			printk("own=%u\n" , own);
		}
		#endif
	}

	return own;
}

static inline void
aipc_mutex_unlock(MUTEX_LOCK mut)
{
	volatile unsigned int own=0;

	own = REG32(R_AIPC_OWN_MUTEX);

	if (mut==CPU_LOCK){
		if (own==OWN_CPU){	// locked by CPU
			REG32(R_AIPC_CPU_MUTEX) = 0;
			#ifdef HW_MUTEX_DBG
			printk("CPU unlock mutex\n");
			#endif
		}
	}
	else {
		if (own==OWN_DSP){	// locked by DSP
			REG32(R_AIPC_DSP_MUTEX) = 0;
			#ifdef HW_MUTEX_DBG
			printk("DSP unlock mutex\n");
			#endif
		}
	}
}

static inline unsigned int 
aipc_mutex_own(void)
{
	volatile unsigned int own=0;
	own = REG32(R_AIPC_OWN_MUTEX);

	#ifdef HW_MUTEX_DBG
	if (own==OWN_NONE)
		printk("Mutex own=%s REG32(R_AIPC_OWN_MUTEX)=0x%x\n" , "NONE" , own);
	else if (own==OWN_CPU)
		printk("Mutex own=%s REG32(R_AIPC_OWN_MUTEX)=0x%x\n" , "CPU" , own);
	else if (own==OWN_DSP)
		printk("Mutex own=%s REG32(R_AIPC_OWN_MUTEX)=0x%x\n" , "DSP" , own);
	else
		printk("Strange value!!!! REG32(R_AIPC_OWN_MUTEX)=0x%x\n" , own);
	#endif
	
	return own;
}
#endif


static inline int 
aipc_int_assert(T_COP cop)
{
#if defined(REFINE_SHM_NOTIFY) && defined(CONFIG_RTL8686_SHM_NOTIFY)
	extern const unsigned int *aipc_shm_notify_cpu;
	extern const unsigned int *aipc_shm_notify_dsp;

	if (cop==T_CPU){
		REG32( aipc_shm_notify_cpu ) = 1;
	}
	else if (cop==T_DSP){
		REG32( aipc_shm_notify_dsp ) = 1;
	}
	else{
		printk("not supported type\n");
	}
	return 0;
	
#elif !defined(REFINE_SHM_NOTIFY) && defined(CONFIG_RTL8686_SHM_NOTIFY)
	if (cop==T_CPU){
		REG32(AIPC_SHM_NOTIFY_CPU)	= 1;
	}
	else if (cop==T_DSP){
		REG32(AIPC_SHM_NOTIFY_DSP)	= 1;
	}
	else{
		printk("not supported type\n");
	}
	return 0;
#else
	if (cop==T_CPU){
		REG32(R_AIPC_EVT_T_CPU) |= ASRT_CPU_IPC;
	}
	else if (cop==T_DSP){
		REG32(R_AIPC_EVT_T_DSP) |= ASRT_DSP_IPC;
	}
	else{
		printk("not supported type\n");
	}
	return 0;
#endif
}

static inline int 
aipc_int_deassert(T_COP cop)
{
#if defined(REFINE_SHM_NOTIFY) && defined(CONFIG_RTL8686_SHM_NOTIFY)
	extern const unsigned int *aipc_shm_notify_cpu;
	extern const unsigned int *aipc_shm_notify_dsp;

	if (cop==T_CPU){
		REG32( aipc_shm_notify_cpu ) = 0;
	}
	else if (cop==T_DSP){
		REG32( aipc_shm_notify_dsp ) = 0;
	}
	else{
		printk("not supported type\n");
	}
	return 0;
#elif !defined(REFINE_SHM_NOTIFY) && defined(CONFIG_RTL8686_SHM_NOTIFY)
	if (cop==T_CPU){
		REG32(AIPC_SHM_NOTIFY_CPU)	= 0;
	}
	else if (cop==T_DSP){
		REG32(AIPC_SHM_NOTIFY_DSP)	= 0;
	}
	else{
		printk("not supported type\n");
	}
	return 0;
#else
	if (cop==T_CPU){
		REG32(R_AIPC_EVT_T_CPU) &= ~ASRT_CPU_IPC;
	}
	else if (cop==T_DSP){
		REG32(R_AIPC_EVT_T_DSP) &= ~ASRT_DSP_IPC;
	}
	else{
		printk("not supported type\n");
	}
	return 0;
#endif
}

static inline int 
aipc_int_mask(T_COP cop)
{
#ifndef CONFIG_RTL8686_SHM_NOTIFY
	if (cop==T_CPU){
#if 0
		REG32(R_GIMR0_0) &= ~DSP_T_CPU_IE;
#else
// R_GIMR0_0
		volatile unsigned int status0 = 0;
		volatile unsigned int now0 = 0;
		status0           =  REG32(R_GIMR0_0);
		status0           &= ~DSP_T_CPU_IE;
		REG32(R_GIMR0_0)  =  status0;
		now0              =  REG32(R_GIMR0_0);
		if( now0 != status0 ){
			printk("%s(%d) status=%d now=%d\n" , __FUNCTION__ , __LINE__
				, status0 , now0);
			}
#endif
	}
	else if (cop==T_DSP){
#if 0
		REG32(R_GIMR0_0) &= ~CPU_T_DSP_IE;
#else
// R_GIMR0_1
		volatile unsigned int status1 = 0;
		volatile unsigned int now1 = 0;
		status1           =  REG32(R_GIMR0_1);
		status1           &= ~CPU_T_DSP_IE;
		REG32(R_GIMR0_1)  =  status1;
		now1              = REG32(R_GIMR0_1);
		if( now1 != status1 ){
			printk("%s(%d) status=%d now=%d\n" , __FUNCTION__ , __LINE__
				, status1 , now1);
			}
#endif
	}
	else{
		printk("not supported type\n");
	}
#endif
	return 0;
}

static inline int 
aipc_int_unmask(T_COP cop)
{
#ifndef CONFIG_RTL8686_SHM_NOTIFY
	if (cop==T_CPU){
#if 0
		REG32(R_GIMR0_0) |= DSP_T_CPU_IE;
#else  
// R_GIMR0_0
        volatile unsigned int status0 = 0;
        volatile unsigned int now0 = 0;
        status0           =  REG32(R_GIMR0_0);
        status0           |= DSP_T_CPU_IE;
        REG32(R_GIMR0_0)  =  status0;
        now0              = REG32(R_GIMR0_0);
        if( now0 != status0 ){
            printk("%s(%d) status=%d now=%d\n" , __FUNCTION__ , __LINE__
                , status0 , now0);
            }
#endif
	}
	else if (cop==T_DSP){
#if 0
		REG32(R_GIMR0_0) |= CPU_T_DSP_IE;
#else
// R_GIMR0_1
		volatile unsigned int status1 = 0;
		volatile unsigned int now1 = 0;
		status1           =  REG32(R_GIMR0_1);
		status1           |= CPU_T_DSP_IE;
		REG32(R_GIMR0_1)  =  status1;
		now1              = REG32(R_GIMR0_1);
		if( now1 != status1 ){
			printk("%s(%d) status=%d now=%d\n" , __FUNCTION__ , __LINE__
				, status1 , now1);
			}

#endif
	}
	else{
		printk("not supported type\n");
	}
#endif
	return 0;
}

#endif

