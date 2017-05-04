#include <linux/string.h>
#include <linux/version.h>
#include <linux/interrupt.h>
#include "rtk_voip.h"
#include "voip_types.h"
#include "voip_init.h"
#include "voip_ipc.h"
#include "voip_debug.h"

#include "aipc_osal.h"
#include "aipc_api.h"
#include "aipc_ctrl.h"
#include "aipc_ioctl.h"
#include "aipc_cbk.h"
#include "aipc_debug.h"
#include "aipc_swp.h"
#include "aipc_irq.h"

struct aipc_int_priv{
	char name[INT_NAME_SIZE];
 	struct tasklet_struct	rx_tasklet;
	struct tasklet_struct	event_tasklet;
};

static struct aipc_int_priv aipc_int_priv_dsc;


void aipc_hook_call_rx_tasklet(void)
{
	struct aipc_int_priv* priv = &aipc_int_priv_dsc;

	tasklet_hi_schedule(&priv->rx_tasklet);
}

void aipc_hook_call_event_tasklet(void)
{
	struct aipc_int_priv* priv = &aipc_int_priv_dsc;

	tasklet_hi_schedule(&priv->event_tasklet);
}

void aipc_hook_irq_init(void)
{
	struct aipc_int_priv* priv = &aipc_int_priv_dsc;
	char strbuf[INT_NAME_SIZE]={0};
	int result=0;

	extern int probe_dsp_int_notify_cpu2dsp;
	if( probe_dsp_int_notify_cpu2dsp == 0 ){
		printk("aipc: probe_dsp_int_notify_cpu2dsp is %d don't register irq.\n" ,
			probe_dsp_int_notify_cpu2dsp);
		return;
	}

	printk("aipc: start aipc irq init\n");

	memset(priv , 0 , sizeof(struct aipc_int_priv));

	sprintf(strbuf, "aipc_irq");	
	strncpy(priv->name , strbuf , strlen(strbuf));
	
	extern AIPC_OSAL_ISR_RET_TYPE aipc_dsp_interrupt(int irq , void *dev_id);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
	#if 0
		result = request_irq(AIPC_IRQ_CPU_T_DSP , aipc_dsp_interrupt , IRQF_DISABLED , 	
				priv->name, priv);// NULL OK
	#else
		result = request_irq(AIPC_IRQ_CPU_T_DSP , aipc_dsp_interrupt , IRQF_DISABLED | IRQF_PERCPU , 	
				priv->name, priv);// NULL OK
	#endif
#else
		result = request_irq(AIPC_IRQ_CPU_T_DSP , aipc_dsp_interrupt , SA_INTERRUPT , 
				priv->name, priv);// NULL OK
#endif

	if (result){
		printk("aipc: Can't request IRQ for IPC\n");
	}
	else{
		printk("aipc: Request IRQ for IPC OK\n");
	}
	
	extern void aipc_2dsp_rx_tasklet(unsigned long task_priv);
	extern void aipc_2dsp_event_tasklet(unsigned long task_priv);

	tasklet_init(&priv->rx_tasklet   , aipc_2dsp_rx_tasklet    , (unsigned long)priv);
	tasklet_init(&priv->event_tasklet, aipc_2dsp_event_tasklet , (unsigned long)priv);
}

module_init( aipc_hook_irq_init );
//voip_initcall_bus( aipc_hook_irq_init );
//voip_initcall_entry( aipc_hook_irq_init );

