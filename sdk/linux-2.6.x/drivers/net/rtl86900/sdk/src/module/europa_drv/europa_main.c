#ifdef CONFIG_EUROPA_MODULE

#include <osal/memory.h>
#include <osal/sem.h>
#include <osal/time.h>
#include <common/debug/rt_log.h>

#include <linux/module.h>  /* Needed by all modules */
#include <linux/kernel.h>  /* Needed for KERN_ALERT */
#include <linux/init.h>

#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>      // Needed by filp
#include <asm/uaccess.h>   // Needed by segment descriptors

#include <module/europa/common.h>
#include <module/europa/europa.h>
#include <module/europa/init.h>
#include <module/europa/laser_drv.h>
#include <module/europa/utility.h>
#include <module/europa/ddmi.h>
#include <module/europa/interrupt.h>

#include <module/europa/rtl8290_api.h>


uint8 loopmode=1;  // default DCL mode
module_param(loopmode, byte, 0644);
MODULE_PARM_DESC(loopmode, "Europa Loop Mode: 0 for DOL, 1 for DCL, 2 for SCL bias, 3 for SCL mod.");

//uint16 loopcnt=100;
//module_param(loopcnt, short, 0644);
//MODULE_PARM_DESC(loopcnt, "Europa Loop Count: SDADC RSSI calculation loop.");


/* board dependent parameters + */
#ifdef CONFIG_EUROPA_I2C_PORT_0
uint8 I2C_PORT=0;
#elif defined(CONFIG_EUROPA_I2C_PORT_1)
uint8 I2C_PORT=1;
#else
uint8 I2C_PORT=0; /* default port 0 */
#endif
module_param(I2C_PORT, byte, 0644);
MODULE_PARM_DESC(I2C_PORT, "Europa i2c port.");

#ifdef CONFIG_EUROPA_INTERRUPT
uint8 INTR_PIN=CONFIG_EUROPA_INTERRUPT_GPIO_PIN;
#else
uint8 INTR_PIN=255;
#endif
module_param(INTR_PIN, byte, 0644);
MODULE_PARM_DESC(INTR_PIN, "Europa Interrupt GPIO pin.");

#ifdef CONFIG_TX_DISABLE_GPIO_PIN
uint8 TXDIS_PIN=CONFIG_TX_DISABLE_GPIO_PIN;
#else
uint8 TXDIS_PIN=255;
#endif
module_param(TXDIS_PIN, byte, 0644);
MODULE_PARM_DESC(TXDIS_PIN, "Europa Tx Dis GPIO pin");

#ifdef CONFIG_TX_POWER_GPO_PIN
uint8 TXPWR_PIN=CONFIG_TX_POWER_GPO_PIN;
#else
uint8 TXPWR_PIN=255;
#endif
module_param(TXPWR_PIN, byte, 0644);
MODULE_PARM_DESC(TXPWR_PIN, "Europa Tx Power GPIO pin.");

uint8 PON_MODE=1;  /* default GPON mode */
module_param(PON_MODE, byte, 0644);
MODULE_PARM_DESC(PON_MODE, "Europa PON Mode.");
/* board dependent parameters - */



uint8 flash_update;

struct task_struct *pEuropaDdmiPollTask;
struct task_struct *pEuropaIntPollTask;

uint8 flash_data[EUROPA_FLASH_DATA_SIZE];

static uint8 calibration_state = FALSE;

static uint8 europa_debug_level = 1;   /* TODO: debug prints APIs */


void europa_debug_level_set(uint8 level)
{
    europa_debug_level = level;
}

uint8 europa_debug_level_get()
{
    return europa_debug_level;
}


void calibration_state_set(uint8 state)
{
    // just assign the state
    calibration_state = state;
}

int europa_ddmi_polling_thread(void *data)
{
    uint16 regData;

    regData = 0;  /* make compiler happy */

    while(!kthread_should_stop())
    {
        /* No need to wake up earlier */
        set_current_state(TASK_UNINTERRUPTIBLE);
        //schedule_timeout(5 * HZ / 10);
        //schedule_timeout(1 * HZ ); 
       schedule_timeout(5 * HZ ); 

        // in calibration mode,
        // we don't need ddmi polling
        if (calibration_state)
            continue;

       update_ddmi();

#if 0
       //regData = getReg(EUROPA_DEBUG_REG);
       //regData = 10;
       //if (regData != 0)
       //{
       //     printk(" DDMI Polling Task(%d) !!!!!!.\n", regData);
      //      regData--;
       //}

       if (regData != 0)
       {
           if(regData&EUROPA_DEBUG_DDMI_MASK)
           {
               update_ddmi();
           }
           if(regData&EUROPA_DEBUG_LUT_MASK)
           {
           //update_lookup_table();
           }
           //printk(KERN_EMERG " Reg = 0x%x, Data = 0x%x \n",EUROPA_DEBUG_REG, regData);
           //regData--;
	    //setReg(EUROPA_DEBUG_REG, regData)	;
    	}
#endif

    }

    return 0;
}

int europa_int_polling_thread(void *data)
{

    while(!kthread_should_stop())
    {
        /* No need to wake up earlier */
        set_current_state(TASK_UNINTERRUPTIBLE);
        schedule_timeout(5 * HZ / 10);
        //schedule_timeout(1 * HZ );

       laser_drv_intrHandle();

       //printk(KERN_EMERG " Interrupt Polling Task !!!!!!.\n");

    }

    return 0;
}
void europa_poling_init(void)
{
    pEuropaDdmiPollTask = kthread_create(europa_ddmi_polling_thread, NULL, "europa_ddmi_polling");
    if(IS_ERR(pEuropaDdmiPollTask))
    {
        printk("%s:%d europa_ddmi_polling_thread failed %ld!\n", __FILE__, __LINE__, PTR_ERR(pEuropaDdmiPollTask));
        printk(KERN_EMERG " DDMI Polling Task Add Fail!!!!.\n");
    }
    else
    {
        wake_up_process(pEuropaDdmiPollTask);
        printk("%s:%d europa_ddmi_polling_thread complete!\n", __FILE__, __LINE__);
        printk(KERN_EMERG " DDMI Polling Task Add Success.\n");
    }

#if 0 //Disable Interrupt Polling
    pEuropaIntPollTask = kthread_create(europa_int_polling_thread, NULL, "europa_int_polling");
    if(IS_ERR(pEuropaIntPollTask))
    {
        printk("%s:%d europa_int_polling_thread failed %ld!\n", __FILE__, __LINE__, PTR_ERR(pEuropaIntPollTask));
        printk(KERN_EMERG " Interrupt Polling Task Add Fail!!!!.\n");
    }
    else
    {
        wake_up_process(pEuropaIntPollTask);
        printk("%s:%d europa_int_polling_thread complete!\n", __FILE__, __LINE__);
        printk(KERN_EMERG " Interrupt Polling Task Add Success!!!!.\n");
    }
#endif

}


int __init europa_module_init(void)
{
    struct file *f;
    mm_segment_t fs;

    flash_update = 0;

    printk(KERN_EMERG "\nRTL8290 driver version [%d.%d.%d]\n\n",
		    EUROPA_DRIVER_VERSION_MAJOR,
		    EUROPA_DRIVER_VERSION_MINOR,
		    EUROPA_DRIVER_VERSION_PATCH);

    printk(KERN_EMERG "   Parameter: I2C_PORT=%d INTR_PIN=%d TXDIS_PIN=%d TXPWR_PIN=%d PON_MODE=%d\n\n", I2C_PORT, INTR_PIN, TXDIS_PIN, TXPWR_PIN, PON_MODE);

    printk(KERN_EMERG "Open file: europa.data.\n");


    f = filp_open("/var/config/europa.data", O_RDONLY, 0);

    if( f != NULL)
    {
        flash_update = 1;
        memset(flash_data,0,EUROPA_FLASH_DATA_SIZE*sizeof(uint8));
        // Get current segment descriptor
        fs = get_fs();
        // Set segment descriptor associated to kernel space
        set_fs(get_ds());
        // Read the file
        f->f_op->read(f, flash_data, EUROPA_FLASH_DATA_SIZE, &f->f_pos);
        // Restore segment descriptor
        set_fs(fs);
        filp_close(f,NULL);
        printk(KERN_EMERG "Loaded data from europa.data\n");
    }
    else
    {
        printk(KERN_EMERG "filp_open error!!.\n");
        flash_update = 0;
    }

    printk(KERN_EMERG "Init Europa module.\n");

    apollo_init();

    europa_init();

    // insert mapper functions
    ldd_mapper_init();

    // register ploam state handler
    ldd_ploamState_handler_init();

    printk(KERN_EMERG "Init Europa Finish.\n");

#if 0
    /*For debug, disable ddmi polling*/
    setReg(EUROPA_DEBUG_REG,0);
#endif

    europa_poling_init();

    /* Turn on Interrupt mask */
    europa_intrInit();

    return 0;
}

void __exit europa_exit(void)
{
    // remove mapper functions
    ldd_mapper_exit();

    // unregister ploam state handler
    ldd_ploamState_handler_exit();

    europa_intrDeInit();

    kthread_stop(pEuropaDdmiPollTask);
#if 0
    kthread_stop(pEuropaIntPollTask);
#endif

    rtl8290_reset(LDD_RESET_CHIP);

    printk("%s(): exit\n",__FUNCTION__);
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("RealTek Europa kernel module");
MODULE_AUTHOR("Star Chang <starchang@realtek.com>");

module_init(europa_module_init);
module_exit(europa_exit);

#endif

