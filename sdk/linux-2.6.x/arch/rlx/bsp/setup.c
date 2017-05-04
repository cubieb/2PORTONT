/*
 * Copyright 2006, Realtek Semiconductor Corp.
 *
 * arch/rlx/rlxocp/setup.c
 *   Interrupt and exception initialization for RLX OCP Platform
 *
 * Tony Wu (tonywu@realtek.com.tw)
 * Nov. 7, 2006
 */
#include <linux/console.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/sched.h>

#include <asm/addrspace.h>
#include <asm/irq.h>
#include <asm/io.h>

#include <asm/bootinfo.h>
#include <asm/time.h>
#include <asm/reboot.h>
#include <asm/rlxbsp.h>
#include <linux/netdevice.h>

#include "bspchip.h"

#ifdef CONFIG_WIRELESS_LAN_MODULE
static void (*wirelessnet_shutdown_hook)(void) = NULL;

void wirelessnet_shutdown_set(void (*hook) (void))
{
	wirelessnet_shutdown_hook = hook;
}

int wirelessnet_shutdown_loaded(void)
{
	if (wirelessnet_shutdown_hook)
		return 1;
	else
		return 0;
}
EXPORT_SYMBOL(wirelessnet_shutdown_set);
EXPORT_SYMBOL(wirelessnet_shutdown_loaded);
#endif

static void shutdown_netdev(void)
{
	//struct net_device *dev;

	printk("Shutdown network interface\n");
	read_lock(&dev_base_lock);

#if 0
	for (dev = dev_base; dev != NULL; dev = dev->next) {
		if((dev->flags & IFF_UP) && dev->stop) {
			printk("%s:===>\n",dev->name);
			rtnl_lock();
			dev_close(dev);
			rtnl_unlock();
		}
	}
#endif

#if defined(CONFIG_RTL8192CD)
	{
		extern void force_stop_wlan_hw(void);
		force_stop_wlan_hw();
	}
#elif defined (CONFIG_WIRELESS_LAN_MODULE)
	if(wirelessnet_shutdown_loaded())
	{
		wirelessnet_shutdown_hook();
	}
#endif
	read_unlock(&dev_base_lock);
}

static void bsp_machine_restart(char *command)
{
    local_irq_disable();
#ifdef CONFIG_NET
	shutdown_netdev();
#endif
    REG32(BSP_WDTCNR) = 0; //enable watch dog
    while (1) ;
}
                                                                                                    
static void bsp_machine_halt(void)
{
    printk("RTL8672 halted.\n");
    while(1);
}
                                                                                                    
static void bsp_machine_power_off(void)
{
    printk("RTL8672 halted. Please turn off the power.\n");
    while(1);
}

/*
 * callback function
 */
extern void _imem_dmem_init(void);
void __init bsp_setup(void)
{
    /* define io/mem region */
	#if 0
    ioport_resource.start = 0x18000000; 
    ioport_resource.end = 0x1fffffff;

    iomem_resource.start = 0x18000000;
    iomem_resource.end = 0x1fffffff;
	#endif

    /* set reset vectors */
    _machine_restart = bsp_machine_restart;
    _machine_halt = bsp_machine_halt;
    pm_power_off = bsp_machine_power_off;

    /* initialize uart */
    bsp_serial_init();
#ifdef CONFIG_RTL8672
    //init at head.S:kernel_entry() => kernel_entry_setup()
#else
    _imem_dmem_init();
#endif //CONFIG_RTL8672
}
