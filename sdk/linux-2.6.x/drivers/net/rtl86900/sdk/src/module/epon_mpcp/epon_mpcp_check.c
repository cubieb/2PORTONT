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
 * $Revision: 51194 $
 * $Date: 2014-09-10 16:36:56 +0800 (Wed, 10 Sep 2014) $
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
#include <linux/skbuff.h>

#include <net/sock.h> 
#include <net/netlink.h> 



/* For RTK APIs */
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <common/rt_type.h>
#include <rtk/epon.h>
#include <rtk/switch.h>
#include <rtk/acl.h>
#include <rtk/irq.h>
#include <rtk/intr.h>
#include <pkt_redirect.h>
#include <hal/common/halctrl.h>
#include <net/rtl/rtl_types.h>
#ifdef CONFIG_SDK_APOLLOMP
#include <dal/apollomp/dal_apollomp_epon.h>
#include <dal/apollomp/dal_apollomp.h>
#endif



#ifdef CONFIG_SDK_RTL9601B
#include <dal/rtl9601b/dal_rtl9601b_epon.h>
#include <dal/rtl9601b/dal_rtl9601b.h>
#include <dal/rtl9601b/dal_rtl9601b_ponmac.h>
#endif

#if defined(CONFIG_RTL9600_SERIES)
#include "re8686.h"
#endif
#if defined(CONFIG_RTL9601B_SERIES)
#include "re8686_rtl9601b.h"
#endif
#if defined(CONFIG_RTL9602C_SERIES)
#include "re8686_rtl9602c.h"
#endif

/*
 * Symbol Definition
 */
#define EPON_MPCP_MAX_GRANT_ANA_DB  128


typedef struct epon_mpcp_gate_info_s
{
    uint32  timeStamp;
    uint32  grantStart;
    uint32  grantLen;
    uint8   grantFlag;
} epon_mpcp_gate_info_t;


/*
 * Data Declaration
 */
__DRAM unsigned int gate_ana_flag = 0;
__DRAM unsigned int laser_patch_flag = 1;


struct proc_dir_entry *epon_mpcp_proc_dir = NULL;

struct proc_dir_entry *mpcp_laser_check_entry = NULL;
struct proc_dir_entry *mpcp_gate_ana_entry = NULL;
struct proc_dir_entry *gata_ana_dbg_entry = NULL;

struct task_struct *pEponLaserChangeCheckTask;
static unsigned int rtl960x_pon_port_mask;

uint32 chipId, rev, subtype;


static unsigned int isFirstError=1;
static uint32 _eponTimeDriftIntCnt=0;
static unsigned int mpcpCauseDriftOccurs=0;
static unsigned int inValidLaserDetectErrorCnt=0;
static unsigned int mpcp_intr_cnt=0,inValidLaserCnt=0;
static unsigned int gateRxExpire=0;
static unsigned int driftTestCnt=0;
static unsigned int mpcpDriftCnt=0;
static unsigned int asicLaserChanedCnt=0;
static unsigned int laserNeedChaned=0;

#define LASER_CHECK_FAIL_BUFFER_CNT 10

static unsigned int laserCheckFailForceStop=0;
static int laserCheckFailMpcpBuffer=LASER_CHECK_FAIL_BUFFER_CNT;



static uint32 preMpcpGateEnd;

#define MAX_GATE_END_MPCP_DELTA 256
static uint16 gateEndMpcpDelta[MAX_GATE_END_MPCP_DELTA];
static uint16 negGateEndMpcpDelta[MAX_GATE_END_MPCP_DELTA];



static uint32 tatalGateCnt;
static uint32 minGateEndMpcpDelta=0xFFFFFF;
static uint32 minNegGateEndMpcpDelta=0xFFFFFF;
static uint32 minGateAndGateDelta=0xFFFFFF;
static uint32 maxGateAndGateDelta=0x0;
static uint32 grantUsed[8];
static uint32 tatalBTBCnt=0;
static uint32 driftTestSafeCnt=0;

static int laser_i=0,laser_j=0;
static unsigned int laser_cnt1,laser_cnt0;


static epon_mpcp_gate_info_t grantDb[EPON_MPCP_MAX_GRANT_ANA_DB];
static uint16 currentGrantIdx=0;


/*
 * Macro Definition
 */
/*mpcp gate analysys*/ 
#define EPON_MPCP_ANA_FLAG_START       0x00000001UL
#define EPON_MPCP_ANA_FLAG_STOP        0x00000002UL  
#define EPON_MPCP_ANA_FLAG_ENABLE      0x80000000UL


/*laser patch flag*/
#define EPON_LASER_PATCH_FLAG_DBGMSG   0x00000001UL
#define EPON_LASER_CHECK_CLR_CNT       0x00000002UL
#define EPON_LASER_FORCE_LASER_CHANGE  0x00000004UL
#define EPON_LASER_MPCP_TRAP_EN        0x08000000UL
#define EPON_LASER_CHECK_FLAG_STOP     0x80000000UL



#define MAX_MPCP_DRIFT_TESTCNT 1000

/*
 * Function Declaration
 */

#define _asm_get_cp0_count() ({ \
    u32 __ret=0;                \
    __asm__ __volatile__ (      \
    "   mfc0    %0, $9  "       \
    : "=r"(__ret)               \
    );                          \
    __ret;                      \
})



void epon_rtl9601b_laser_phase_change(void)
{
#ifdef CONFIG_SDK_RTL9601B

    unsigned int benOn;
    int iCheck=0,iChange=0;
    uint32   data;
    uint16 data16,laserPhase;
    rtl9601b_sds_acc_t sds_acc;
    int ret;
    
    laser_i=0;
    laser_j=0;
    laser_cnt1=0;
    laser_cnt0=0;

    
    
    /*change laser phase*/
    /*get laser phase set SDS_EXT_REG13.SEP_CFG_NEG_GTXC bit[15]*/
    sds_acc.index = RTL9601B_SDS_IDX_PON;
    sds_acc.page = RTL9601B_SDS_PAGE_SDS_EXT;
    sds_acc.regaddr = 13;
    if ((ret = _rtl9601b_serdes_ind_read(sds_acc, &data16)) != RT_ERR_OK)
    {
        printk("\n laser change read fail %s %d\n",__FUNCTION__,__LINE__);
    }    

    laserPhase = data16;
    if(laserPhase & 0x8000)
    {
        laserPhase = laserPhase&0x7FFF;
    }
    else
    {
        laserPhase = laserPhase|0x8000;
    }

    /*polling until BEN on*/
    data = 0x8B;
    if(reg_field_write(RTL9601B_EPON_DEBUG1r, RTL9601B_DBG_SELf, &data)!= RT_ERR_OK)
    {
        return;
    }

    do{
        reg_field_read(RTL9601B_EPON_DEBUG2r, RTL9601B_PRB_EPMCf, &data);
        benOn = data;    

        laser_i++;
        if(laser_i>100000)
            break;
        if((benOn & 0x00040000))
            iCheck++;
        else
        {
            if(iCheck)
                iChange=1;    
            iCheck =0;
        }
    }while(iCheck<2);

    /*polling until BEN off*/
    do{
        laser_cnt0 = _asm_get_cp0_count();
        reg_field_read(RTL9601B_EPON_DEBUG2r, RTL9601B_PRB_EPMCf, &data);
        benOn = data;    
        laser_j++;
        if(laser_j>100000)
            break;        
    }while((benOn & 0x00040000));
                        
        
    /*change laser phase*/
    sds_acc.index = RTL9601B_SDS_IDX_PON;
    sds_acc.page = RTL9601B_SDS_PAGE_SDS_EXT;
    sds_acc.regaddr = 13;
    data16 = laserPhase;
    if ((ret = _rtl9601b_serdes_ind_write(sds_acc, &data16)) != RT_ERR_OK)
    {
        printk("\n laser change read fail %s %d\n",__FUNCTION__,__LINE__);
    } 

    laserNeedChaned=0;
    
    asicLaserChanedCnt++;
    
    laser_cnt1 = _asm_get_cp0_count();    
#endif
} 

void epon_apollo_laser_phase_change(void)
{
    unsigned int benOn;
    unsigned int laserPhase;
    int iCheck=0,iChange=0;


    laser_i=0;
    laser_j=0;
    laser_cnt1=0;
    laser_cnt0=0;


    /*change laser phase*/
    laserPhase = (*((volatile unsigned int *)(0xBB022a34)));
    if(laserPhase & 0x00008000)
    {
        laserPhase = laserPhase&0xFFFF7FFF;
    }
    else
    {
        laserPhase = laserPhase|0x00008000;
    }

    /*polling until BEN on*/
    (*((volatile unsigned int *)(0xBB036018)))= 0x208B;
    do{
        benOn = (*((volatile unsigned int *)(0xBB03601C)));
        laser_i++;
        if(laser_i>100000)
            break;
        if((benOn & 0x00040000))
            iCheck++;
        else
        {
            if(iCheck)
                iChange=1;    
            iCheck =0;
        }
    }while(iCheck<2);

    /*polling until BEN off*/
    do{
         laser_cnt0 = _asm_get_cp0_count();
         benOn = (*((volatile unsigned int *)(0xBB03601C)));
         laser_j++;
         if(laser_j>100000)
            break;
    }while((benOn & 0x00040000));
                        
        
    /*change laser phase*/
    (*((volatile unsigned int *)(0xBB022a34))) = laserPhase;
    laserNeedChaned=0;
    
    asicLaserChanedCnt++;
    
    laser_cnt1 = _asm_get_cp0_count();    

} 



int epon_laser_change_polling_thread(void *data)
{
    unsigned long eponIrqFlags;

    while(!kthread_should_stop())
    {
        /* No need to wake up earlier */
        set_current_state(TASK_UNINTERRUPTIBLE);
        /*schedule every 0.1 sec*/    
        schedule_timeout(HZ/10);

        /*check if need laser phase change*/
        if(laser_patch_flag & EPON_LASER_CHECK_FLAG_STOP)
        {
            /* Stop polling, just wait until next round */
            continue;
        }
        
        /*check laset change flag*/
        if(1==laserNeedChaned)
        {

            /*check if no llid entry valid*/
            /*receive deregister packet*/    
            rtk_epon_llid_entry_t llidEntry;

            /*disable LLID*/            
            llidEntry.llidIdx = 0;
            rtk_epon_llid_entry_get(&llidEntry);

            if(llidEntry.valid == DISABLED)
            {
               laserNeedChaned = 0;
            }
            else
            {
                local_irq_save(eponIrqFlags);
                if(APOLLOMP_CHIP_ID==chipId)
                {
                    epon_apollo_laser_phase_change();
                }
                if(RTL9601B_CHIP_ID==chipId)
                {
                    epon_rtl9601b_laser_phase_change();
                }
                
                local_irq_restore(eponIrqFlags);
                if(laser_patch_flag&EPON_LASER_PATCH_FLAG_DBGMSG)
                {
                    printk("\n laser change polling:(%d)/(%d) change ticks:%d\n",laser_i,laser_j,(laser_cnt1-laser_cnt0));    
                }
            }            
        }
    }

    return 0;
}

static int mpcp_gateAnaFlag_read(struct seq_file *seq, void *v)
{
	int i;

	seq_printf(seq, "[gate_ana_flag = 0x%08x]\n", gate_ana_flag);
	seq_printf(seq, "EPON_MPCP_ANA_FLAG_START  \t0x%08x\n", (unsigned int)EPON_MPCP_ANA_FLAG_START);
	seq_printf(seq, "EPON_MPCP_ANA_FLAG_STOP \t0x%08x\n", (unsigned int)EPON_MPCP_ANA_FLAG_STOP);
	seq_printf(seq, "EPON_MPCP_ANA_FLAG_ENABLE \t0x%08x\n", (unsigned int)EPON_MPCP_ANA_FLAG_ENABLE);

    for(i=0;i<MAX_GATE_END_MPCP_DELTA;i++)
    {
    	seq_printf(seq, "%d, ",gateEndMpcpDelta[i]);
        gateEndMpcpDelta[i] = 0;
    }

    for(i=0;i<MAX_GATE_END_MPCP_DELTA;i++)
    {
    	seq_printf(seq, "-%d, ",negGateEndMpcpDelta[i]);
        negGateEndMpcpDelta[i] = 0;
    }

	seq_printf(seq, "total[%d] min[%d],[-%d]\n",tatalGateCnt,minGateEndMpcpDelta,minNegGateEndMpcpDelta);
    tatalGateCnt = 0;
    minGateEndMpcpDelta = 0xFFFFFF;
    minNegGateEndMpcpDelta= 0xFFFFFF;

	seq_printf(seq, "Grant used\n");
    for(i=0;i<4;i++)
    {
    	seq_printf(seq, "%d, ",grantUsed[i]);
        grantUsed[i] = 0;
    }

	seq_printf(seq, "\nGrant delta");
  	seq_printf(seq, "BTB cnt:%d,gate delta min[0x%x] max[0x%x]\n",
  	                        tatalBTBCnt,minGateAndGateDelta,maxGateAndGateDelta);
    tatalBTBCnt =0;
    minGateAndGateDelta=0xFFFFFFFF;
    maxGateAndGateDelta=0x0;

	return 0;
}

static int mpcp_gateAnaFlag_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	unsigned char tmpBuf[16] = {0};
	int len = (count > 15) ? 15 : count;

	if (buffer && !copy_from_user(tmpBuf, buffer, len))
	{
		gate_ana_flag = simple_strtoul(tmpBuf, NULL, 16);
		printk("write gate_ana_flag to 0x%08x\n", gate_ana_flag);
		
		
		if(gate_ana_flag&EPON_MPCP_ANA_FLAG_START)
		{
		    /*start cpapture gate message*/
		    currentGrantIdx=0;        
		}


		return count;
	}
	return -EFAULT;
}

static int mpcp_laserChk_read(struct seq_file *seq, void *v)
{
	seq_printf(seq, "[laser_patch_flag = 0x%08x]\n", laser_patch_flag);
	seq_printf(seq, "EPON_LASER_PATCH_FLAG_DBGMSG \t0x%08x\n", (unsigned int)EPON_LASER_PATCH_FLAG_DBGMSG);
	seq_printf(seq, "EPON_LASER_CHECK_CLR_CNT     \t0x%08x\n", (unsigned int)EPON_LASER_CHECK_CLR_CNT);
	seq_printf(seq, "EPON_LASER_FORCE_LASER_CHANGE\t0x%08x\n", (unsigned int)EPON_LASER_FORCE_LASER_CHANGE);
	seq_printf(seq, "EPON_LASER_MPCP_TRAP_EN      \t0x%08x\n", (unsigned int)EPON_LASER_MPCP_TRAP_EN);
	seq_printf(seq, "EPON_LASER_CHECK_FLAG_STOP   \t0x%08x\n\n", (unsigned int)EPON_LASER_CHECK_FLAG_STOP);
	seq_printf(seq, "\ninValidLaserDetectErrorCnt:\t0x%08x\n", (unsigned int)inValidLaserDetectErrorCnt);
	seq_printf(seq, "inValidLaserCnt:           \t0x%08x\n", (unsigned int)inValidLaserCnt);
	seq_printf(seq, "asicLaserChanedCnt:        \t0x%08x\n", (unsigned int)asicLaserChanedCnt);
	seq_printf(seq, "mpcp_intr_cnt:             \t0x%08x\n", (unsigned int)mpcp_intr_cnt);
	seq_printf(seq, "TimeDriftIntCnt:           \t0x%08x\n", (unsigned int)_eponTimeDriftIntCnt);
	return 0;
}



static void epon_mpcp_acl_set(uint32 mode,uint8 state)
{
#ifndef CONFIG_RTK_L34_ENABLE
    rtk_acl_ingress_entry_t aclRule;
    rtk_acl_field_t fieldHead1,fieldHead2;
    int ret;
    if(EPON_LASER_MPCP_TRAP_EN==mode)
    {
        /*add acl rule drop all gate message and trigger interrupt*/
        osal_memset(&aclRule, 0, sizeof(rtk_acl_ingress_entry_t));
        aclRule.index = 63;
        aclRule.templateIdx = 3;
        /*enable ACL on PON port*/
        RTK_PORTMASK_PORT_SET(aclRule.activePorts,HAL_GET_PON_PORT());
        aclRule.valid = ENABLED;
        aclRule.act.enableAct[ACL_IGR_INTR_ACT] = ENABLED;
        aclRule.act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLED;
        aclRule.act.aclInterrupt = ENABLED;
        
    
        if(1==state)
        {
            aclRule.act.forwardAct.act = ACL_IGR_FORWARD_TRAP_ACT;
        }
        else
        {
            aclRule.act.forwardAct.act = ACL_IGR_FORWARD_DROP_ACT;
        }
        
        fieldHead1.fieldType = ACL_FIELD_ETHERTYPE;
        fieldHead1.fieldUnion.data.value = 0x8808;
        fieldHead1.fieldUnion.data.mask = 0xFFFF;
        fieldHead1.next = NULL;
        if ((ret = rtk_acl_igrRuleField_add(&aclRule,&fieldHead1)) != RT_ERR_OK)
        {
            return;
        }
        
        fieldHead2.fieldType = ACL_FIELD_USER_DEFINED03;
        fieldHead2.fieldUnion.data.value = 0x0002;
        fieldHead2.fieldUnion.data.mask = 0xFFFF;                      
        fieldHead2.next = NULL;
        if ((ret = rtk_acl_igrRuleField_add(&aclRule,&fieldHead2)) != RT_ERR_OK)
        {
            return;
        }
    
    
        if ((ret = rtk_acl_igrRuleEntry_add(&aclRule)) != RT_ERR_OK)
        {
            return;
        }

    }


#else

    rtk_acl_ingress_entry_t aclRule;
    rtk_acl_field_t aclField;
    if(EPON_LASER_MPCP_TRAP_EN==mode)
    {    

        osal_memset(&aclRule, 0, sizeof(rtk_acl_ingress_entry_t));
        osal_memset(&aclField, 0, sizeof(rtk_acl_field_t));

        aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
        aclField.fieldUnion.pattern.fieldIdx = 3; //FS[3] in template[2]:field[3]
        aclField.fieldUnion.data.value = 0x0002; //two byte 0x0002 after ethertype for EPON
        aclField.fieldUnion.data.mask = 0xffff;
        rtk_acl_igrRuleField_add(&aclRule, &aclField);
        aclRule.valid = ENABLED;
        aclRule.index = 0;
        aclRule.templateIdx = 2;//FS[3] in template[2]:field[3]
        RTK_PORTMASK_PORT_SET(aclRule.activePorts,HAL_GET_PON_PORT());
        aclRule.act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLED;
        aclRule.act.forwardAct.act= ACL_IGR_FORWARD_REDIRECT_ACT;

        if(1==state)
        {
            aclRule.act.forwardAct.act = ACL_IGR_FORWARD_TRAP_ACT;
        }
        else
        {
            aclRule.act.forwardAct.act = ACL_IGR_FORWARD_DROP_ACT;
        }

        aclRule.act.enableAct[ACL_IGR_INTR_ACT] = ENABLED;
        aclRule.act.aclInterrupt = ENABLED;
        rtk_acl_igrRuleEntry_add(&aclRule);
        
        
    }
#endif


}



static int mpcp_laserChk_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	unsigned char tmpBuf[16] = {0};
	int len = (count > 15) ? 15 : count;

	if (buffer && !copy_from_user(tmpBuf, buffer, len))
	{
		laser_patch_flag = simple_strtoul(tmpBuf, NULL, 16);
		printk("write gate_ana_flag to 0x%08x\n", laser_patch_flag);

    	if(EPON_LASER_CHECK_CLR_CNT&laser_patch_flag)
    	{
            printk("\n counter clear!\n");
            inValidLaserDetectErrorCnt = 0;
            inValidLaserCnt = 0;
            mpcp_intr_cnt = 0 ;
            _eponTimeDriftIntCnt = 0;
            asicLaserChanedCnt=0;
    	}
    	if(EPON_LASER_FORCE_LASER_CHANGE&laser_patch_flag)
    	{
            printk("\n force laser change!\n");
    	    laserNeedChaned = 1;    
    	}


    	if(EPON_LASER_MPCP_TRAP_EN&laser_patch_flag)
    	{
            epon_mpcp_acl_set(EPON_LASER_MPCP_TRAP_EN,1);
            printk("\n enable Trap!\n");
    	}
        else
        {
            epon_mpcp_acl_set(EPON_LASER_MPCP_TRAP_EN,0);
            printk("\n disable Trap!\n");
    
        }        


		return count;

	}
	
	return -EFAULT;
}

static int mpcp_gate_ana_read(struct seq_file *seq, void *v)
{
    int i,j;
    uint32 totalTq=0;
    uint32 totalTime;
	
	gate_ana_flag = gate_ana_flag|EPON_MPCP_ANA_FLAG_STOP;
	
	seq_printf(seq, "\n\n index,TimeStmp,GntStart,GrantEnd,GrantLen,  Flag\n");

    for(j=0;j<EPON_MPCP_MAX_GRANT_ANA_DB;j++)
    {
    	i = currentGrantIdx+j;
    	
    	i= i%EPON_MPCP_MAX_GRANT_ANA_DB;
    	
    	seq_printf(seq, " %-5d,%-8.8x,%-8.8x,%-8.8x,%-8d,0x%4x\n"
    	      ,i
    	      ,grantDb[i].timeStamp
    	      ,grantDb[i].grantStart
    	      ,(grantDb[i].grantStart+grantDb[i].grantLen)
    	      ,grantDb[i].grantLen
    	      ,grantDb[i].grantFlag);
    	      
    	if(grantDb[i].grantFlag != 0x09)/*skip mpcp discovery gate*/
    	    totalTq=grantDb[i].grantLen+totalTq;      
    }
    
    totalTime =  (grantDb[EPON_MPCP_MAX_GRANT_ANA_DB-1].grantStart+grantDb[EPON_MPCP_MAX_GRANT_ANA_DB-1].grantLen) - grantDb[0].grantStart;
    if(totalTime)
    {
    	seq_printf(seq, "\n\n bandWidth:%d.%d\n",(totalTq*100)/totalTime,(totalTq*100)%totalTime);
    }
    
	gate_ana_flag = gate_ana_flag&(uint32)(~EPON_MPCP_ANA_FLAG_STOP);

    laserCheckFailForceStop=0;

	return 0;
}

static int mpcp_gate_ana_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	unsigned char tmpBuf[16] = {0};
	int len = (count > 15) ? 15 : count;

	if (buffer && !copy_from_user(tmpBuf, buffer, len))
	{
		simple_strtoul(tmpBuf, NULL, 16);
		
		return count;
	}
	return -EFAULT;
}

static int mpcp_laser_check_open(struct inode *inode, struct file *file)
{
        return single_open(file, mpcp_laserChk_read, inode->i_private);
}

static const struct file_operations mpcp_laser_check_fops = {
        .owner          = THIS_MODULE,
        .open           = mpcp_laser_check_open,
        .read           = seq_read,
        .write          = mpcp_laserChk_write,
        .llseek         = seq_lseek,
        .release        = single_release,
};

static int mpcp_gate_ana_open(struct inode *inode, struct file *file)
{
        return single_open(file, mpcp_gate_ana_read, inode->i_private);
}

static const struct file_operations mpcp_gate_ana_fops = {
        .owner          = THIS_MODULE,
        .open           = mpcp_gate_ana_open,
        .read           = seq_read,
        .write          = mpcp_gate_ana_write,
        .llseek         = seq_lseek,
        .release        = single_release,
};

static int gata_ana_dbg_open(struct inode *inode, struct file *file)
{
        return single_open(file, mpcp_gateAnaFlag_read, inode->i_private);
}

static const struct file_operations gata_ana_dbg_fops = {
        .owner          = THIS_MODULE,
        .open           = gata_ana_dbg_open,
        .read           = seq_read,
        .write          = mpcp_gateAnaFlag_write,
        .llseek         = seq_lseek,
        .release        = single_release,
};


static void epon_mpcp_packet_dbg_init(void)
{
    /* Create proc debug commands */
    if(NULL == epon_mpcp_proc_dir)
    {
        epon_mpcp_proc_dir = proc_mkdir("epon_mpcp", NULL);
    }
    if(epon_mpcp_proc_dir)
    {
    	/*mpcp debug cnt*/
		proc_create("laser_chk", 0644, epon_mpcp_proc_dir, &mpcp_laser_check_fops);

    	/*mpcp gate analysis*/
		proc_create("gate_ana", 0644, epon_mpcp_proc_dir, &mpcp_gate_ana_fops);

    	/*debug flag*/
		proc_create("gate_ana_dbg_flag", 0644, epon_mpcp_proc_dir, &gata_ana_dbg_fops);
    }
}

static void epon_mpcp_dbg_exit(void)
{
    /* Remove proc debug commands */
    if(mpcp_laser_check_entry)
    {
    	remove_proc_entry("laser_chk", epon_mpcp_proc_dir);
        mpcp_laser_check_entry = NULL;
    }

    if(mpcp_gate_ana_entry)
    {
    	remove_proc_entry("gate_ana_dump", epon_mpcp_proc_dir);
        mpcp_gate_ana_entry = NULL;
    }


    if(gata_ana_dbg_entry)
    {
    	remove_proc_entry("gate_ana_dbg_flag", epon_mpcp_proc_dir);
        gata_ana_dbg_entry = NULL;
    }
    
    
    if(epon_mpcp_proc_dir)
    {
        remove_proc_entry("epon_mpcp", NULL);
        epon_mpcp_proc_dir = NULL;
    }
}



static int epon_register_pkt_accept_check(struct sk_buff *skb)
{
    rtk_epon_llid_entry_t regEntry;    
    
    printk("\n receive register packet mac [%2.2x %2.2x %2.2x %2.2x %2.2x %2.2x] flag:%d\n",
                    skb->data[0],
                    skb->data[1],
                    skb->data[2],
                    skb->data[3],
                    skb->data[4],
                    skb->data[5],
                    skb->data[22]);
    regEntry.llidIdx = 0;
    rtk_epon_llid_entry_get(&regEntry);

    printk("\n -- local mac addr [%2.2x %2.2x %2.2x %2.2x %2.2x %2.2x]\n",
            regEntry.mac.octet[0],
            regEntry.mac.octet[1],
            regEntry.mac.octet[2],
            regEntry.mac.octet[3],
            regEntry.mac.octet[4],
            regEntry.mac.octet[5]);
    /*mac control packet*/
    if(skb->data[0]==0x01 && 
       skb->data[1]==0x80 &&
       skb->data[2]==0xC2 &&
       skb->data[3]==0x00 &&
       skb->data[4]==0x00 &&
       skb->data[5]==0x01)    
    {
        return 1;
    }
    
    /*local mac address*/
    if(skb->data[0]==regEntry.mac.octet[0] && 
       skb->data[1]==regEntry.mac.octet[1] &&
       skb->data[2]==regEntry.mac.octet[2] &&
       skb->data[3]==regEntry.mac.octet[3] &&
       skb->data[4]==regEntry.mac.octet[4] &&
       skb->data[5]==regEntry.mac.octet[5])    
    {
        return 1;
    }
    return 0;
}

/* For ctc spec, we need to set onu to silent mode when receive a NACK mpcp REGISTER frame */
static void epon_ctc_set_oam_silent_mode(void)
{
	unsigned char silentMsg[] = {
        0x15, 0x68, /* Magic key */
        0x66, 0x66  /* Message body */
    };

	pkt_redirect_kernelApp_sendPkt(PR_USER_UID_EPONOAM, 1, sizeof(silentMsg), silentMsg);
}

int epon_mpcp_pkt_rx(
    struct re_private *cp,
    struct sk_buff *skb,
    struct rx_info *pRxInfo)
{
    if((skb->data[12] == 0x88) && (skb->data[13] == 0x08))
    {
        /*ony mac control packet(MPCP) need handle by this API*/ 
    }
    else
    {
         return RE8670_RX_CONTINUE;    
    }

    /*filter mpcp  register packet*/
    if((skb->data[14] == 0x00) &&  (skb->data[15] == 0x05))
    {
        if(epon_register_pkt_accept_check(skb)==0)
        {
            printk("\n this register is not for this onu!!\n");
        }
        else
        {
            if(skb->data[22] == 2)/*flag == deregister */
            {
                /*receive deregister packet*/    
                rtk_epon_llid_entry_t llidEntry;
    
                /*disable LLID*/            
                llidEntry.llidIdx = 0;
                rtk_epon_llid_entry_get(&llidEntry);
    
                llidEntry.valid = DISABLED;
                llidEntry.llid = 0x7fff;
                rtk_epon_llid_entry_set(&llidEntry);
            }
			else if(skb->data[22] == 4)/* flag == NACK */
			{
				epon_ctc_set_oam_silent_mode();
			}
        }
        return RE8670_RX_STOP;    
    }
    
    /*filter mpcp gate message*/
    if((skb->data[14] == 0x00) &&  (skb->data[15] == 0x02))
    {
        uint32 mpcpGateEnd;
        uint16 gateLen;
        uint32 mpcpGateStar,mpcpTimeStamp,grantStart;
        uint32 negCurrGateEndMpcpDelta=0xFFFFFFFF,currGateEndMpcpDelta=0xFFFFFFFF; 

        memcpy(&mpcpTimeStamp,&(skb->data[16]),4); 
        memcpy(&mpcpGateStar,&(skb->data[21]),4); 
        memcpy(&gateLen,&(skb->data[25]),2); 

        if(!(gate_ana_flag&EPON_MPCP_ANA_FLAG_STOP))
        {
            if(laserCheckFailForceStop==0 || laserCheckFailMpcpBuffer>0)
            {
                if(currentGrantIdx >= EPON_MPCP_MAX_GRANT_ANA_DB)
                    currentGrantIdx=0;
                grantDb[currentGrantIdx].timeStamp = mpcpTimeStamp;
                grantDb[currentGrantIdx].grantStart = mpcpGateStar;
                grantDb[currentGrantIdx].grantLen = gateLen;
                grantDb[currentGrantIdx].grantFlag = skb->data[20];
                currentGrantIdx++;
                if(laserCheckFailForceStop==1)
                    laserCheckFailMpcpBuffer--;
            }
        }

        if(skb->data[20] == 0x09)
        {/*do not handle discovery gate*/
            return RE8670_RX_STOP;
        }

        if(preMpcpGateEnd >= mpcpTimeStamp)
        {
            currGateEndMpcpDelta = preMpcpGateEnd-mpcpTimeStamp;    
            if(currGateEndMpcpDelta < MAX_GATE_END_MPCP_DELTA)
            {
                if(gateEndMpcpDelta[currGateEndMpcpDelta]<0xFFFF)
                    gateEndMpcpDelta[currGateEndMpcpDelta]++;
            }
            if(currGateEndMpcpDelta < minGateEndMpcpDelta)
            {
                minGateEndMpcpDelta = currGateEndMpcpDelta;    
            }
        }
        else
        {
            negCurrGateEndMpcpDelta = mpcpTimeStamp-preMpcpGateEnd;    
            if(negCurrGateEndMpcpDelta < MAX_GATE_END_MPCP_DELTA)
            {
                if(negGateEndMpcpDelta[negCurrGateEndMpcpDelta]<0xFFFF)
                    negGateEndMpcpDelta[negCurrGateEndMpcpDelta]++;
            }
            if(negCurrGateEndMpcpDelta < minNegGateEndMpcpDelta)
            {
                minNegGateEndMpcpDelta = negCurrGateEndMpcpDelta;    
            }
        }
        
        #if 0
        if((currGateEndMpcpDelta)<=2)
            printk("\n[gate will drift]:%d\n",(currGateEndMpcpDelta));   
        #endif    
        /*check gate behavior*/    
        if((preMpcpGateEnd>0xf0000000) && (mpcpGateStar<0xf0000000))
        {/*overflow*/
            //printk("\noverflow: %8.8x %8.8x   delta:%8.8x\n",preMpcpGateEnd,mpcpGateStar,(mpcpGateStar - preMpcpGateEnd));
            if(((0xFFFFFFFF-preMpcpGateEnd)+mpcpGateStar) < minGateAndGateDelta)
            {
                minGateAndGateDelta = ((0xFFFFFFFF-preMpcpGateEnd)+mpcpGateStar);    
            }
            if(((0xFFFFFFFF-preMpcpGateEnd)+mpcpGateStar) > maxGateAndGateDelta)
            {
                maxGateAndGateDelta = ((0xFFFFFFFF-preMpcpGateEnd)+mpcpGateStar);    
            }
        }
        else
        {/*not overflow*/
            if(mpcpGateStar < preMpcpGateEnd)
            {/*BTB occurs*/
                tatalBTBCnt++;
                printk("\nBTB: %8.8x %8.8x  delta:%8.8x\n",preMpcpGateEnd,mpcpGateStar,(mpcpGateStar - preMpcpGateEnd));
            }
            if((mpcpGateStar-preMpcpGateEnd) < minGateAndGateDelta)
            {
                minGateAndGateDelta = (mpcpGateStar-preMpcpGateEnd);    
            }
            if((mpcpGateStar-preMpcpGateEnd) > maxGateAndGateDelta)
            {
                maxGateAndGateDelta = (mpcpGateStar-preMpcpGateEnd);    
            }
        }
        
        mpcpGateEnd = mpcpGateStar + gateLen;
        preMpcpGateEnd = mpcpGateEnd;
        
        tatalGateCnt++;     


        if(APOLLOMP_CHIP_ID==chipId)
        {
            grantStart = (*((volatile unsigned int *)(0xBB036064)));
            if(grantStart!=0)
                grantUsed[0]++;
            
            grantStart = (*((volatile unsigned int *)(0xBB036068)));
            if(grantStart!=0)
                grantUsed[1]++;
    
            grantStart = (*((volatile unsigned int *)(0xBB03606C)));
            if(grantStart!=0)
                grantUsed[2]++;
    
            grantStart = (*((volatile unsigned int *)(0xBB036070)));
            if(grantStart!=0)
                grantUsed[3]++;
        }
        if(RTL9601B_CHIP_ID==chipId)
        {
            grantStart = (*((volatile unsigned int *)(0xBB036058)));
            if(grantStart!=0)
                grantUsed[0]++;
            
            grantStart = (*((volatile unsigned int *)(0xBB03605c)));
            if(grantStart!=0)
                grantUsed[1]++;
    
            grantStart = (*((volatile unsigned int *)(0xBB036060)));
            if(grantStart!=0)
                grantUsed[2]++;
    
            grantStart = (*((volatile unsigned int *)(0xBB036064)));
            if(grantStart!=0)
                grantUsed[3]++;
        }
        
        return RE8670_RX_STOP;
        
    }    
    return RE8670_RX_CONTINUE;
}



void epon_rtl9601b_mpcp_gate_checking(void)
{
#ifdef CONFIG_SDK_RTL9601B

    unsigned int localtime,firstLocalTime;
    unsigned int benOn;
    unsigned int startTime=0;
    uint32 data;
    /*gate grant list start time*/ 
    /*set EPON_DEBUG1.DBG_SEL = 0x81*/
    data = 0x81;
    if(reg_field_write(RTL9601B_EPON_DEBUG1r, RTL9601B_DBG_SELf, &data)!= RT_ERR_OK)
    {
        return;
    }

    if(reg_field_read(RTL9601B_EPON_DEBUG2r, RTL9601B_PRB_EPMCf, &data)!= RT_ERR_OK)
    {
        return;
    }
    startTime = data & 0x00FFFFFF;
    
    /*set EPON_DEBUG1.DBG_SEL = 0x82*/
    data = 0x82;
    if(reg_field_write(RTL9601B_EPON_DEBUG1r, RTL9601B_DBG_SELf, &data)!= RT_ERR_OK)
    {
        return;
    }

    if(reg_field_read(RTL9601B_EPON_DEBUG2r, RTL9601B_PRB_EPMCf, &data)!= RT_ERR_OK)
    {
        return;
    }
    startTime = startTime+((data&0x000000FF)<<24);

    if(reg_field_read(RTL9601B_EPON_LOCAL_TIMEr, RTL9601B_LOCAL_TIMEf, &data)!= RT_ERR_OK)
    {
        return;
    }
    firstLocalTime = data;

    mpcp_intr_cnt ++;
    

    /*set EPON_DEBUG1.DBG_SEL = 0x82*/
    data = 0x8B;
    if(reg_field_write(RTL9601B_EPON_DEBUG1r, RTL9601B_DBG_SELf, &data)!= RT_ERR_OK)
    {
        return;
    }

    if(reg_field_read(RTL9601B_EPON_DEBUG2r, RTL9601B_PRB_EPMCf, &data)!= RT_ERR_OK)
    {
        return;
    }
    benOn = data;
    if(benOn & 0x00040000)
    {
        if(reg_field_read(RTL9601B_EPON_LOCAL_TIMEr, RTL9601B_LOCAL_TIMEf, &data)!= RT_ERR_OK)
        {
            return;
        }
        localtime = data;
        if(((startTime - localtime)< 0x0fffffff) && ((startTime - localtime) > 1))
        {
            if(localtime<firstLocalTime)
            {
                inValidLaserDetectErrorCnt++;

                if(laser_patch_flag&EPON_LASER_PATCH_FLAG_DBGMSG)
                    printk("(Detect error!!)\n");
            }
            else
            {
                inValidLaserCnt++;
                if(laserCheckFailForceStop==0)
                {
                    laserCheckFailForceStop=1;
                    laserCheckFailMpcpBuffer=LASER_CHECK_FAIL_BUFFER_CNT;
                }

                laserNeedChaned=1;
                
                if(laser_patch_flag&EPON_LASER_PATCH_FLAG_DBGMSG)
                {
                    //printk("\n [Total Ivalid Laser CNT]:%d\n",inValidLaserCnt);
                    //printk("\n========================================\n");
                    printk("\n LT:(2)0x%8.8x/(1)0x%8.8x GS:0x%8.8x",localtime,firstLocalTime,startTime);
                    //printk("\n Ben:0x%8.8x\n",benOn);
                    //printk("\n========================================\n");
                    //printk("timeDrift_intr:%d\n",mpcpCauseDriftOccurs);
                }
            }
        }
    }
    
    if((startTime - localtime) >= 0x0fffffff)
    {
        gateRxExpire++;
    }
#endif
}


void epon_mpcp_gate_checking(void)
{
    unsigned int localtime,firstLocalTime;
    unsigned int benOn;
    unsigned int startTime=0,tmpData1,tmpData2;

    /*gate grant list start time*/        
    (*((volatile unsigned int *)(0xBB036018)))= 0x2081;
    tmpData1=(*((volatile unsigned int *)(0xBB03601C)));
    startTime = tmpData1 & 0x00FFFFFF;
    (*((volatile unsigned int *)(0xBB036018)))= 0x2082;
    tmpData2=(*((volatile unsigned int *)(0xBB03601C)));
    startTime = startTime+((tmpData2&0x000000FF)<<24);

    localtime = (*((volatile unsigned int *)(0xBB036230)));
    firstLocalTime = localtime;
    mpcp_intr_cnt ++;
    

    (*((volatile unsigned int *)(0xBB036018)))= 0x208B;
    benOn = (*((volatile unsigned int *)(0xBB03601C)));
    if(benOn & 0x00040000)
    {
        localtime = (*((volatile unsigned int *)(0xBB036230)));
        if(((startTime - localtime)< 0x0fffffff) && ((startTime - localtime) > 1))
        {
            if(localtime<firstLocalTime)
            {
                inValidLaserDetectErrorCnt++;

                if(laser_patch_flag&EPON_LASER_PATCH_FLAG_DBGMSG)
                    printk("(Detect error!!)\n");
            }
            else
            {
                inValidLaserCnt++;
                if(laserCheckFailForceStop==0)
                {
                    laserCheckFailForceStop=1;
                    laserCheckFailMpcpBuffer=LASER_CHECK_FAIL_BUFFER_CNT;
                }
                laserNeedChaned=1;

                if(laser_patch_flag&EPON_LASER_PATCH_FLAG_DBGMSG)
                {
                    //printk("\n [Total Ivalid Laser CNT]:%d\n",inValidLaserCnt);
                    //printk("\n========================================\n");
                    printk("\n LT:(2)0x%8.8x/(1)0x%8.8x GS:0x%8.8x",localtime,firstLocalTime,startTime);
                    //printk("\n Ben:0x%8.8x\n",benOn);
                    //printk("\n========================================\n");
                    //printk("timeDrift_intr:%d\n",mpcpCauseDriftOccurs);
                }
                
            }    


        }
    }
    
    if((startTime - localtime) >= 0x0fffffff)
    {
        gateRxExpire++;
    }
 
}



void epon_mpcp_timeDriftCheck(void)
{
    driftTestCnt++;
    
    if(driftTestCnt >= MAX_MPCP_DRIFT_TESTCNT)
    {/*start check*/
        
        if((mpcpDriftCnt) < (MAX_MPCP_DRIFT_TESTCNT-100))
        {/*need change laser*/
            laserNeedChaned = 1;
            if(laser_patch_flag&EPON_LASER_PATCH_FLAG_DBGMSG)
                printk("\n laser phase changed by timeDrift(%8.8x/%8.8x)\n",driftTestCnt,mpcpDriftCnt);
        }
        else
        {
           if(driftTestSafeCnt > 5)
           {
#ifdef CONFIG_SDK_APOLLOMP
                uint32 data;
                /*stop trap mpcp packet to cpu*/
                data = 0x0; /*gate handle asic only*/
                if (reg_field_write(APOLLOMP_EPON_MPCP_CTRr,APOLLOMP_GATE_HANDLEf,&data)!= RT_ERR_OK)
                {
                    return;
                } 
#endif
                driftTestSafeCnt=0;
           }
           else
           {
               driftTestSafeCnt++;
           } 
        }
        driftTestCnt = 0;
        mpcpDriftCnt = 0;
    }
}


void epon_mpcp_isr_entry(void)
{

    /*check if no llid entry valid*/
    rtk_epon_llid_entry_t llidEntry;

    if(rtk_intr_imr_set(INTR_TYPE_ACL_ACTION,DISABLED)!= RT_ERR_OK)
    {
		return;
    }

    
    /*get LLID*/            
    llidEntry.llidIdx = 0;
    rtk_epon_llid_entry_get(&llidEntry);
 
 
    if(llidEntry.valid != DISABLED)
    { 
#ifdef CONFIG_SDK_APOLLOMP
        uint32  data;
        if(APOLLOMP_CHIP_ID==chipId)
        {/*only apollo need polling*/
            /*check if timeDrift occurs*/
            if(reg_field_read(APOLLOMP_EPON_INTRr, APOLLOMP_TIME_DRIFT_IMSf, &data)!= RT_ERR_OK)
            {
                return;
            } 
            if(data == 0)
            {
                mpcpCauseDriftOccurs=1;
                mpcpDriftCnt++;
                data = 1;
                _eponTimeDriftIntCnt++;
                if(reg_field_write(APOLLOMP_EPON_INTRr, APOLLOMP_TIME_DRIFT_IMSf, &data)!= RT_ERR_OK)
                {
                    return;
                } 
            }
            else
            {
                mpcpCauseDriftOccurs=0;
            }
            epon_mpcp_gate_checking();
            
            switch(rev)        
            {        
                case CHIP_REV_ID_A:
                case CHIP_REV_ID_B:
                case CHIP_REV_ID_C:
                case CHIP_REV_ID_D:
                case CHIP_REV_ID_E:
                    break;
                case CHIP_REV_ID_F:     
                    epon_mpcp_timeDriftCheck();
                    break;
                default:
                    break;    
            }
        }

#endif
        if(RTL9601B_CHIP_ID==chipId)
        {
            epon_rtl9601b_mpcp_gate_checking();    
        }
    }

    if(rtk_intr_ims_clear(INTR_TYPE_ACL_ACTION) != RT_ERR_OK)
	{
        return;
	}

    if(rtk_intr_imr_set(INTR_TYPE_ACL_ACTION,ENABLED) != RT_ERR_OK)
    {
		return;
    }
    return;
}


/*acl trap drop init*/
static int epon_mpcp_pkt_acl_dropIntr_register(void)
{
    int32  ret;

    /*ACL init*/
#ifndef CONFIG_RTK_L34_ENABLE
    rtk_acl_field_entry_t aclField;
    rtk_acl_template_t aclTemplete;
    rtk_acl_ingress_entry_t aclRule;
    rtk_acl_field_t fieldHead1,fieldHead2;
    
    /*drop all mpcp gate message*/
    /*field selector*/
    aclField.index = 3;
    aclField.format = ACL_FORMAT_RAW;
    aclField.offset = 14;
    if ((ret = rtk_acl_fieldSelect_set(&aclField)) != RT_ERR_OK)
    {
        return ret;
    }
    
    /*set templete*/
    aclTemplete.index = 3;
    if ((ret = rtk_acl_template_get(&aclTemplete)) != RT_ERR_OK)
    {
        return ret;
    }
    aclTemplete.fieldType[0]=ACL_FIELD_ETHERTYPE;
    aclTemplete.fieldType[1]=ACL_FIELD_USER_DEFINED03;
    if ((ret = rtk_acl_template_set(&aclTemplete)) != RT_ERR_OK)
    {
        return ret;
    }
    
    /*add acl rule drop all gate message and trigger interrupt*/
    osal_memset(&aclRule, 0, sizeof(rtk_acl_ingress_entry_t));
    aclRule.index = 63;
    aclRule.templateIdx = 3;
    /*enable ACL on PON port*/
    RTK_PORTMASK_PORT_SET(aclRule.activePorts,HAL_GET_PON_PORT());
    aclRule.valid = ENABLED;
    aclRule.act.enableAct[ACL_IGR_INTR_ACT] = ENABLED;
    aclRule.act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLED;
    aclRule.act.aclInterrupt = ENABLED;
    aclRule.act.forwardAct.act = ACL_IGR_FORWARD_DROP_ACT;
    
    fieldHead1.fieldType = ACL_FIELD_ETHERTYPE;
    fieldHead1.fieldUnion.data.value = 0x8808;
    fieldHead1.fieldUnion.data.mask = 0xFFFF;
    fieldHead1.next = NULL;
    if ((ret = rtk_acl_igrRuleField_add(&aclRule,&fieldHead1)) != RT_ERR_OK)
    {
        return ret;
    }

    fieldHead2.fieldType = ACL_FIELD_USER_DEFINED03;
    fieldHead2.fieldUnion.data.value = 0x0002;
    fieldHead2.fieldUnion.data.mask = 0xFFFF;                      
    fieldHead2.next = NULL;
    if ((ret = rtk_acl_igrRuleField_add(&aclRule,&fieldHead2)) != RT_ERR_OK)
    {
        return ret;
    }
    
    if ((ret = rtk_acl_igrRuleEntry_add(&aclRule)) != RT_ERR_OK)
    {
        return ret;
    }

    if ((ret = rtk_acl_igrState_set(HAL_GET_PON_PORT(), ENABLED)) != RT_ERR_OK)
    {
        return ret;
    }
#endif

    /*register ACL interrupt*/
       /*register interrupt handle*/
    if((ret = rtk_irq_isr_register(INTR_TYPE_ACL_ACTION,epon_mpcp_isr_entry)) != RT_ERR_OK)
    {
    	return ret;
    }
    
    if((ret = rtk_intr_imr_set(INTR_TYPE_ACL_ACTION,ENABLED)) != RT_ERR_OK)
    {
        return ret;
    }
    
    return 0;
}



static void epon_mpcp_pkt_register(void)
{
    extern int drv_nic_register_rxhook(int portmask,int priority,p2rfunc_t rx);
    
    rtk_portmask_t portMask;
    rtk_switch_portMask_Clear(&portMask);
    rtk_switch_port2PortMask_set(&portMask,RTK_PORT_PON);
    rtl960x_pon_port_mask = portMask.bits[0];

    #if defined(FPGA_DEFINED)
    drv_nic_register_rxhook(0xFF, RE8686_RXPRI_MPCP, epon_mpcp_pkt_rx);
    #else
    drv_nic_register_rxhook(rtl960x_pon_port_mask, RE8686_RXPRI_MPCP, epon_mpcp_pkt_rx);
    #endif
    
}       



int __init epon_mpcp_init(void)
{
    int32  ret;
    uint32  data;
    uint32  needChangeLaser=0;
            
    rtk_switch_version_get(&chipId, &rev, &subtype);

#ifdef CONFIG_SDK_APOLLOMP
    if(APOLLOMP_CHIP_ID==chipId)
    {
        switch(rev)        
        {        
            case CHIP_REV_ID_A:
            case CHIP_REV_ID_B:
            case CHIP_REV_ID_C:
            case CHIP_REV_ID_D:
            case CHIP_REV_ID_E:
                /*accept special packet on pon port, for mpcp register packet*/
                data = 1;
                if ((ret = reg_array_field_write(APOLLOMP_P_MISCr, HAL_GET_PON_PORT(), REG_ARRAY_INDEX_NONE, APOLLOMP_RX_SPCf, &data)) != RT_ERR_OK)
                {
                    return ret;
                }
                data = 1;
                if ((ret = reg_array_field_write(APOLLOMP_P_MISCr, HAL_GET_PON_PORT(), REG_ARRAY_INDEX_NONE, APOLLOMP_ACCEPT_RX_ERRORf, &data)) != RT_ERR_OK)
                {
                    return ret;
                }
                /*MPCP Gate frame handle set to asic handle*/
                data = 0x0; /*trap all gate to CPU*/
                if ((ret = reg_field_write(APOLLOMP_EPON_MPCP_CTRr,APOLLOMP_GATE_TRAP_TYPEf,&data)) != RT_ERR_OK)
                {
                    return ret;
                }
                data = 0x1; /*gate handle asic and trap to CPU*/
                if ((ret = reg_field_write(APOLLOMP_EPON_MPCP_CTRr,APOLLOMP_GATE_HANDLEf,&data)) != RT_ERR_OK)
                {
                    return ret;
                }            
                /*ACL init*/
                epon_mpcp_pkt_acl_dropIntr_register();
                /* for mpcp timedrift check
                reg set 0x36038 0x1
                reg set 0x2281c 0x059
                reg set 0x22808 0x5080
                */
    
                data=0x1;
                reg_field_write(APOLLOMP_EPON_TIME_CTRLr,APOLLOMP_RTT_ADJf,&data);
                data=0x059;
                reg_write(APOLLOMP_SDS_REG7r, &data);
                data=0x5080;
                reg_write(APOLLOMP_SDS_REG2r, &data);
                needChangeLaser=1;
            break;
            /*do not patch CHIP_REV_ID_F*/    
            case CHIP_REV_ID_F:
            default:
            data = 0x0; /*trap all gate to CPU*/
            if ((ret = reg_field_write(APOLLOMP_EPON_MPCP_CTRr,APOLLOMP_GATE_TRAP_TYPEf,&data)) != RT_ERR_OK)
            {
                return ret;
            }
        
            data = 0x0; /*gate handle asic only*/
            if ((ret = reg_field_write(APOLLOMP_EPON_MPCP_CTRr,APOLLOMP_GATE_HANDLEf,&data)) != RT_ERR_OK)
            {
                return ret;
            }            
            /*accept special packet on pon port*/
            data = 0;
            if ((ret = reg_array_field_write(APOLLOMP_P_MISCr, HAL_GET_PON_PORT(), REG_ARRAY_INDEX_NONE, APOLLOMP_RX_SPCf, &data)) != RT_ERR_OK)
            {
                return ret;
            }
            break;                
        }

    }
#endif

#ifdef CONFIG_SDK_RTL9601B
    if(RTL9601B_CHIP_ID==chipId)
    {
        switch(rev)        
        {  
            case CHIP_REV_ID_0:
                /*MPCP Gate frame handle set to asic handle*/
                data = 0x0; /*trap all gate to CPU*/
                if ((ret = reg_field_write(RTL9601B_EPON_MPCP_CTRr,RTL9601B_GATE_TRAP_TYPEf,&data)) != RT_ERR_OK)
                {
                    return ret;
                }
                data = 0x1; /*gate handle asic and trap to CPU*/
                if ((ret = reg_field_write(RTL9601B_EPON_MPCP_CTRr,RTL9601B_GATE_HANDLEf,&data)) != RT_ERR_OK)
                {
                    return ret;
                }             
                epon_mpcp_pkt_acl_dropIntr_register();
                needChangeLaser=1;
            break;            
            default:
                data = 0x0; /*trap all gate to CPU*/
                if ((ret = reg_field_write(RTL9601B_EPON_MPCP_CTRr,RTL9601B_GATE_TRAP_TYPEf,&data)) != RT_ERR_OK)
                {
                    return ret;
                }
            
                data = 0x0; /*gate handle asic only*/
                if ((ret = reg_field_write(RTL9601B_EPON_MPCP_CTRr,RTL9601B_GATE_HANDLEf,&data)) != RT_ERR_OK)
                {
                    return ret;
                }
                break;            
            
        }    
    }
#endif


    if(needChangeLaser==1)
    {
        
        pEponLaserChangeCheckTask = kthread_create(epon_laser_change_polling_thread, NULL, "epon_polling");
        if(IS_ERR(pEponLaserChangeCheckTask))
        {
            printk("%s:%d epon_mpcp_init failed %ld!\n", __FILE__, __LINE__, PTR_ERR(pEponLaserChangeCheckTask));
        }
        else
        {
            wake_up_process(pEponLaserChangeCheckTask);
        }
    }


    
    epon_mpcp_pkt_register();
    epon_mpcp_packet_dbg_init();

    printk("\nepon_mpcp_init ok!!\n");

    return 0;
}

void __exit epon_mpcp_exit(void)
{
    kthread_stop(pEponLaserChangeCheckTask);
    epon_mpcp_dbg_exit();
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("RealTek EPON mpcp check module");
MODULE_AUTHOR("Realtek");
module_init(epon_mpcp_init);
module_exit(epon_mpcp_exit);
#endif
