/*
 * Copyright (C) 2011 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 *
 *
 * $Revision: 65801 $
 * $Date: 2016-02-02 20:29:45 +0800 (Tue, 02 Feb 2016) $
 *
 * Purpose : EPON polling kernel thread
 *
 * Feature : Use kernel thread to perodically polling the LOS state
 *
 */

#ifdef CONFIG_EPON_FEATURE
/*
 * Include Files
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>

/* For RTK APIs */
#include <common/rt_error.h>
#include <rtk/epon.h>
#include <rtk/switch.h>

#include <pkt_redirect.h>
#include <net/rtl/rtl_types.h>
#include <linux/seq_file.h>
#if defined(CONFIG_SDK_APOLLOMP)
#include <dal/apollomp/dal_apollomp.h>
#endif

/*
 * Symbol Definition
 */
#define EPON_POLLING_FLAG_STOPPOLLING       0x00000001UL

/*
 * Data Declaration
 */
__DRAM unsigned int polling_flag = 0;
struct proc_dir_entry *epon_proc_dir = NULL;
struct proc_dir_entry *polling_flag_entry = NULL;
struct task_struct *pEponPollTask;


/*
 * Macro Definition
 */

/*
 * Function Declaration
 */

int epon_polling_thread(void *data)
{
    int ret;
    unsigned char losMsg[] = {
        0x15, 0x68, /* Magic key */
        0xde, 0xad  /* Message body */
    };
#ifdef CONFIG_EPON_LOS_RECOVER
	unsigned char losRecoverMsg[] = {
		0x15, 0x68, /* Magic key */
        0x55, 0x55  /* Message body */
	};
#endif
    rtk_enable_t state, currState = ENABLED;
    rtk_epon_llid_entry_t llidEntry;

    while(!kthread_should_stop())
    {
        /* No need to wake up earlier */
        set_current_state(TASK_UNINTERRUPTIBLE);
        schedule_timeout(2 * HZ / 100);

        if(polling_flag & EPON_POLLING_FLAG_STOPPOLLING)
        {
            /* Stop polling, just wati until next round */
            continue;
        }

        ret = rtk_epon_losState_get(&state);
        do {
            if((RT_ERR_OK == ret) && (currState != state))
            {
                if(DISABLED == state)
                {
                    uint32 chipId, rev, subtype;
                    uint32  data;

                    rtk_switch_version_get(&chipId, &rev, &subtype);

                    switch(rev)
                    {
                        case CHIP_REV_ID_A:
                        case CHIP_REV_ID_B:
                        case CHIP_REV_ID_C:
                        case CHIP_REV_ID_D:
                        case CHIP_REV_ID_E:
                        case CHIP_REV_ID_F:
                            /*re-lock gphy*/
#if defined(CONFIG_SDK_APOLLOMP)
                            data = 0x0;
                            reg_field_write(APOLLOMP_WSDS_ANA_15r, APOLLOMP_REG_CMU_EN_GPHYf, &data);
                            data = 0x1;
                            reg_field_write(APOLLOMP_WSDS_ANA_15r, APOLLOMP_REG_CMU_EN_GPHYf, &data);
#endif
                            break;
                        default:
                            break;
                    }
#ifndef CONFIG_EPON_LOS_RECOVER
                    /* Trigger register */
                    llidEntry.llidIdx = 0;
                    ret = rtk_epon_llid_entry_get(&llidEntry);
                    if(RT_ERR_OK != ret)
                    {
                        break;
                    }

                    llidEntry.valid = DISABLED;
                    llidEntry.llid = 0x7fff;
                    ret = rtk_epon_llid_entry_set(&llidEntry);
                    if(RT_ERR_OK != ret)
                    {
                        break;
                    }
#else
					pkt_redirect_kernelApp_sendPkt(PR_USER_UID_EPONOAM, 1, sizeof(losRecoverMsg), losRecoverMsg);
#endif
                }
                else
                {
#ifndef CONFIG_EPON_LOS_RECOVER
                    /* Trigger lost of link */
                    llidEntry.llidIdx = 0;
                    ret = rtk_epon_llid_entry_get(&llidEntry);
                    if(RT_ERR_OK != ret)
                    {
                        break;
                    }

                    llidEntry.valid = DISABLED;
                    llidEntry.llid = 0x7fff;
                    ret = rtk_epon_llid_entry_set(&llidEntry);
                    if(RT_ERR_OK != ret)
                    {
                        break;
                    }
#endif
                    pkt_redirect_kernelApp_sendPkt(PR_USER_UID_EPONOAM, 1, sizeof(losMsg), losMsg);
                }
                currState = state;
            }
        }while(0);
    }

    return 0;
}

static int polling_flag_read(struct seq_file *seq, void *v)
{
	seq_printf(seq, "[polling_flag = 0x%x]\n", polling_flag);
	seq_printf(seq, "EPON_POLLING_FLAG_STOPPOLLING\t0x%x\n", EPON_POLLING_FLAG_STOPPOLLING);

	return 0;
}

static int polling_flag_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	unsigned char tmpBuf[16] = {0};
	int len = (count > 15) ? 15 : count;

	if (buffer && !copy_from_user(tmpBuf, buffer, len))
	{
		polling_flag = simple_strtoul(tmpBuf, NULL, 16);
		printk("write polling_flag to 0x%08x\n", polling_flag);
		return count;
	}
	return -EFAULT;
}

static int polling_flag_open(struct inode *inode, struct file *file)
{
        return single_open(file, polling_flag_read, inode->i_private);
}

static const struct file_operations polling_flag_fops = {
        .owner          = THIS_MODULE,
        .open           = polling_flag_open,
        .read           = seq_read,
        .write          = polling_flag_write,
        .llseek         = seq_lseek,
        .release        = single_release,
};

static void epon_polling_dbg_init(void)
{
    /* Create proc debug commands */
    if(NULL == epon_proc_dir)
    {
        epon_proc_dir = proc_mkdir("epon", NULL);
    }
    if(epon_proc_dir)
    {
		proc_create("polling_flag", 0644, epon_proc_dir, &polling_flag_fops);
    }
}

static void epon_polling_dbg_exit(void)
{
    /* Remove proc debug commands */
    if(polling_flag_entry)
    {
    	remove_proc_entry("polling_flag", epon_proc_dir);
        polling_flag_entry = NULL;
    }
    if(epon_proc_dir)
    {
        remove_proc_entry("epon", NULL);
        epon_proc_dir = NULL;
    }
}

int __init epon_poling_init(void)
{
    uint32 chipId, rev, subtype;

    rtk_switch_version_get(&chipId, &rev, &subtype);
    if(APOLLOMP_CHIP_ID==chipId)
    {/*only allo need polling*/

        pEponPollTask = kthread_create(epon_polling_thread, NULL, "epon_polling");
        if(IS_ERR(pEponPollTask))
        {
            printk("%s:%d epon_poling_init failed %ld!\n", __FILE__, __LINE__, PTR_ERR(pEponPollTask));
        }
        else
        {
            wake_up_process(pEponPollTask);
            printk("%s:%d epon_poling_init complete!\n", __FILE__, __LINE__);
        }
        epon_polling_dbg_init();
    }

    return 0;
}

void __exit epon_polling_exit(void)
{
    kthread_stop(pEponPollTask);
    epon_polling_dbg_exit();
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("RealTek EPON polling module");
MODULE_AUTHOR("Realtek");
module_init(epon_poling_init);
module_exit(epon_polling_exit);
#endif

