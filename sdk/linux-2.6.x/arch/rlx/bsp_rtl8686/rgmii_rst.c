#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <bspchip.h>
#include <rtk/gpio.h>
#include <common/error.h>
#include <rtk/port.h>
#include "gpio.h"

static void RGMII_reset_pin(int *reset){
        *reset = GPIO_57;
}

static int config_rgmii_port(void)
{
        rtk_port_macAbility_t port_mac_ability;
        int ret=0;
        ret = rtk_port_macForceAbility_get(5,&port_mac_ability);
        if(ret){
                printk("rtk_port_macForceAbility_get error ret=%d",__FILE__,__LINE__,ret);
                return ret;
        }
        port_mac_ability.speed = PORT_SPEED_1000M;
        port_mac_ability.duplex = PORT_FULL_DUPLEX;
        port_mac_ability.linkStatus = PORT_LINKUP;
        port_mac_ability.txFc = ENABLED;
        port_mac_ability.rxFc = ENABLED;
        ret = rtk_port_macForceAbility_set(5,port_mac_ability);
        if(ret){
                printk("%s-%d error ret=%d",__FILE__,__LINE__,ret);
                return ret;
        }

        ret = rtk_port_macForceAbilityState_set(5,ENABLED);
        if(ret){
                printk("%s-%d error ret=%d",__FILE__,__LINE__,ret);
                return ret;
        }

        return ret;
}

static unsigned int start_rgmii_rst(void)
{
	int RGMII_gpio_RST;
	int ret;
	// reset RGMII pin
	RGMII_reset_pin(&RGMII_gpio_RST);
	gpioClear(RGMII_gpio_RST);
	gpioConfig(RGMII_gpio_RST, GPIO_FUNC_OUTPUT);
	mdelay(500);
	gpioSet(RGMII_gpio_RST);

	REG32(0xBB00014C) |= 1; // Enable RGMII MAC
	// Config RGMII port to linkup & 1G & full duplex
	if(config_rgmii_port() != 0)
		return -1;
	// reset gpio pin 6/7/8/9/10/11/15/16/17/18/19/20
	if((ret = rtk_gpio_state_set(6,DISABLED))!=RT_ERR_OK)
		return 6;
	if((ret = rtk_gpio_state_set(7,DISABLED))!=RT_ERR_OK)
		return 7;
	if((ret = rtk_gpio_state_set(8,DISABLED))!=RT_ERR_OK)
		return 8;
	if((ret = rtk_gpio_state_set(9,DISABLED))!=RT_ERR_OK)
		return 9;
	if((ret = rtk_gpio_state_set(10,DISABLED))!=RT_ERR_OK)
		return 10;
	if((ret = rtk_gpio_state_set(11,DISABLED))!=RT_ERR_OK)
		return 11;
	if((ret = rtk_gpio_state_set(15,DISABLED))!=RT_ERR_OK)
		return 15;
	if((ret = rtk_gpio_state_set(16,DISABLED))!=RT_ERR_OK)
		return 16;
	if((ret = rtk_gpio_state_set(17,DISABLED))!=RT_ERR_OK)
		return 17;
	if((ret = rtk_gpio_state_set(18,DISABLED))!=RT_ERR_OK)
		return 18;
	if((ret = rtk_gpio_state_set(19,DISABLED))!=RT_ERR_OK)
		return 19;
	if((ret = rtk_gpio_state_set(20,DISABLED))!=RT_ERR_OK)
		return 20;
	
	return 0;
}

static int proc_rgmii_rst_w(struct file *file, const char *buffer,
                      unsigned long count, void *data)
{
	unsigned char flag;
	unsigned int ret;
    if (buffer && !copy_from_user(&flag, buffer, sizeof(flag))) 
	{
        switch(flag) 
		{
            case '1':
				if((ret = start_rgmii_rst()) != 0)
					printk("RGMII Reset Process Failed(%d)\n", ret);
                break;
			default:
                printk("Not support setting value(%d)\n", flag);
            return count;
		}
	}
    return -EFAULT;
}


static int __init rgmii_rst_init(void) 
{
	static struct proc_dir_entry *proc_rgmii_entry ;
	if((proc_rgmii_entry = create_proc_entry("rgmii_rst", 0644, NULL)) == NULL) {
        printk("create proc: rgmii_rst failed!\n");
        return -1;
    }
    proc_rgmii_entry->write_proc  = proc_rgmii_rst_w;
	return 0;
}

static void __exit rgmii_rst_exit(void) 
{
	remove_proc_entry("rgmii_rst", NULL);
}


module_init(rgmii_rst_init);
module_exit(rgmii_rst_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("RGMII Reset Process");
