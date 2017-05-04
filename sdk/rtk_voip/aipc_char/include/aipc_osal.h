#ifndef __AIPC_OSAL_H__
#define __AIPC_OSAL_H__

#ifdef __KERNEL__

// interrupt context lock
#define AIPC_OSAL_INTERRUPT_FLAGS     ul32_t
#define AIPC_OSAL_INTERRUPT_DISABLE   local_irq_save
#define AIPC_OSAL_INTERRUPT_ENABLE    local_irq_restore

// thread context lock
#define AIPC_OSAL_MUTEX_LOCK          down
#define AIPC_OSAL_MUTEX_UNLOCK        up

// schedule
#define AIPC_OSAL_SCHEDULE            schedule
#define AIPC_OSAL_SCHEDULE_TIMEOUT_INTERRUPTIBLE    schedule_timeout_interruptible
#define AIPC_OSAL_MDELAY              mdelay

// isr 
#define AIPC_OSAL_ISR_RET_TYPE        irqreturn_t
#define AIPC_OSAL_ISR_HANDLE          IRQ_HANDLED
#define AIPC_OSAL_ISR_CALLDSR         

#elif defined(__ECOS)
// interrupt context lock
#define AIPC_OSAL_INTERRUPT_FLAGS     cyg_uint32 
#define AIPC_OSAL_INTERRUPT_DISABLE   HAL_DISABLE_INTERRUPTS
#define AIPC_OSAL_INTERRUPT_ENABLE    HAL_RESTORE_INTERRUPTS

// thread context lock
#define AIPC_OSAL_MUTEX_LOCK          cyg_mutex_lock
#define AIPC_OSAL_MUTEX_UNLOCK        cyg_mutex_unlock

// schedule
#define AIPC_OSAL_SCHEDULE            cyg_thread_yield
#define AIPC_OSAL_SCHEDULE_TIMEOUT_INTERRUPTIBLE    error     //no this in eCos
#define AIPC_OSAL_MDELAY              error                   //no this in eCos

// isr 
#define AIPC_OSAL_ISR_RET_TYPE        cyg_uint32
#define AIPC_OSAL_ISR_HANDLE          CYG_ISR_HANDLED
#define AIPC_OSAL_ISR_CALLDSR         CYG_ISR_CALL_DSR

#endif

#endif
