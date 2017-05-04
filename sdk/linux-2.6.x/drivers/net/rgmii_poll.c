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
 * Purpose : RGMII polling kernel thread
 *
 * Feature : Use kernel thread to perodically polling the external phy state
 *
 */


 /*
  * Include Files
  */
#include <linux/module.h> 
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/init.h> 
#include <linux/proc_fs.h>


/* For RTK APIs */
#include <rtk/port.h>


/*
 * Data Declaration
 */
__DRAM unsigned int rgmii_polling_flag = 1;
struct proc_dir_entry *rgmii_proc_dir = NULL;
struct proc_dir_entry *rgmii_polling_flag_entry = NULL;
struct task_struct *pRgmiiPollTask;

#define DEBUG_ALWAYS   0xffffffff

//#define DEBUG_RGMII
#ifdef DEBUG_RGMII
static unsigned int _debug_ = (DEBUG_ALWAYS);//|DEBUG_READ|DEBUG_WRITE|DEBUG_ERASE);
#define DEBUG_RGMII_PRINT(mask, string) \
			if ((_debug_ & mask) || (mask == DEBUG_ALWAYS)) \
			printk string
#else
#define DEBUG_RGMII_PRINT(mask, string)
#endif


#define RGMII_EXT_PORT 	5
//internal soc register
#define MODE_EXT 		0xBB00014C
	#define EXT_MAC_RGMII 	(1 << 0)
#define IOPAD_CFG 		0xBB023010  //0x2560000
#define EXT_RGMXF		0xBB000004	//0xC
#define EXT_TXC_DLY		0xBB000008  //0x1
//REG32(MODE_EXT) |= EXT_MAC_RGMII;
#define RGMII_EN() (*((volatile unsigned int *)(MODE_EXT)))|=EXT_MAC_RGMII
#define RGMII_DIS() (*((volatile unsigned int *)(MODE_EXT)))&=(~EXT_MAC_RGMII)
//external phy register
#define EXT_LINKED 		(1 << 2)
#define GIGA_FUL   		(1 << 11)
#define GIGA_HAL   		(1 << 10)
#define FUL_100			(1 << 8)
#define HAL_100			(1 << 7)
#define FUL_10			(1 << 6)
#define HAL_10			(1 << 5)
#define ABILITY_100_10_MASK (0xf << 5)


/*
 * Function Declaration
 */
int config_rgmii_speed(rtk_port_speed_t speed, rtk_port_duplex_t  duplex)
{
	rtk_port_macAbility_t port_mac_ability;
	int ret=0;
	ret = rtk_port_macForceAbility_get(RGMII_EXT_PORT,&port_mac_ability);
	if(ret){
		printk("%s-%d error ret=%d",__FILE__,__LINE__,ret);
		return ret;
	}
	port_mac_ability.speed = speed; 
	port_mac_ability.duplex = duplex;
	port_mac_ability.linkStatus = PORT_LINKUP; 
	port_mac_ability.txFc = ENABLED;
	port_mac_ability.rxFc = ENABLED;
	ret = rtk_port_macForceAbility_set(RGMII_EXT_PORT,port_mac_ability);
	if(ret){
		printk("%s-%d error ret=%d",__FILE__,__LINE__,ret);
		return ret;
	}

	ret = rtk_port_macForceAbilityState_set(RGMII_EXT_PORT,ENABLED);
	if(ret){
		printk("%s-%d error ret=%d",__FILE__,__LINE__,ret);
		return ret;
	}

	return ret;
}

static int rgmii_en=0;
static int state = 0xff,current_state = 0;

void probe_rgmii(void)
{
	unsigned int pData=0;
	unsigned int page = 0;
	unsigned int temp=0;
	rtk_port_macAbility_t port_mac_ability;
	rtk_port_speed_t speed;
	rtk_port_duplex_t duplex;	
	//printk("[%s-%d] start!!!!!\n",__func__,__LINE__);				
	//rtk_port_speed_t speed_current;
	//rtk_port_duplex_t duplex_current;
	if (!rtk_port_phyReg_get(RGMII_EXT_PORT,page,PHY_REG_STATUS,&pData) && pData!=0xffff)
	{
		//read success, means you got a external phy.
		//enable external mac with rgmii
/*
		if(!rgmii_en){
			RGMII_EN();
			rgmii_en=1;
		}
*/
		if(pData & EXT_LINKED){
			/*linkup*/
			rtk_port_phyReg_get(RGMII_EXT_PORT,page,PHY_REG_AN_LINKPARTNER,&pData);
			DEBUG_RGMII_PRINT(DEBUG_ALWAYS,("[%s-%d] pData=0x%x\n",__func__,__LINE__,pData));
			temp = (pData & ABILITY_100_10_MASK) >> 5;
			DEBUG_RGMII_PRINT(DEBUG_ALWAYS,("[%s-%d] temp=0x%x\n",__func__,__LINE__,temp));

			DEBUG_RGMII_PRINT(DEBUG_ALWAYS,("[%s-%d] rgmii link up pData=0x%x\n",__func__,__LINE__,pData));				
			rtk_port_phyReg_get(RGMII_EXT_PORT,page,PHY_REG_1000_BASET_STATUS,&pData);
			DEBUG_RGMII_PRINT(DEBUG_ALWAYS,("[%s-%d] pData=0x%x\n",__func__,__LINE__,pData));
			if(pData & GIGA_FUL){
				current_state=1;
			}else if(pData & GIGA_HAL){
				current_state=2;
			}else{
				switch(temp)
				{
					case 1:
						/*10h*/
						current_state=3;
						break;
					case 2:
					case 3:
						current_state=4;				
						/*10f*/
						break;
					case 4:
					case 5:
					case 6:
					case 7:
						current_state=5;						
						/*100h*/
						break;
					case 8:
					case 9:
					case 10:
					case 11:
					case 12:
					case 13:
					case 14:
					case 15:
						current_state=6;						
						/*100f*/
						break;
					default:
						printk("[%s-%d] configure external phy error!\n",__func__,__LINE__);
						break;
				}
			}
			if(state==current_state)
				return;
			if(pData & GIGA_FUL){
				/*link partner is 1000 Full duplex*/
				state = 1;
				if(rgmii_polling_flag & 0x2)
					printk("1000F state=%d\n",state);
				speed = PORT_SPEED_1000M;
				duplex = PORT_FULL_DUPLEX;
				config_rgmii_speed(speed,duplex);
			}else if(pData & GIGA_HAL){
				/*link partner is 1000 Half duplex*/
				state = 2;
				if(rgmii_polling_flag & 0x2)				
					printk("1000H state=%d\n",state);
				speed = PORT_SPEED_1000M;
				duplex = PORT_HALF_DUPLEX;
				config_rgmii_speed(speed,duplex);
			}else{
				//rtk_port_phyReg_get(RGMII_EXT_PORT,page,PHY_REG_AN_LINKPARTNER,&pData);
				//DEBUG_RGMII_PRINT(DEBUG_ALWAYS,("[%s-%d] pData=0x%x\n",__func__,__LINE__,pData));
				//temp = (pData & ABILITY_100_10_MASK) >> 5;
				//DEBUG_RGMII_PRINT(DEBUG_ALWAYS,("[%s-%d] temp=0x%x\n",__func__,__LINE__,temp));				
				switch(temp)
				{
					case 1:
						/*10h*/
						state = 3;
						if(rgmii_polling_flag & 0x2)						
							printk("10H state=%d\n",state);						
						speed = PORT_SPEED_10M;
						duplex = PORT_HALF_DUPLEX;
						config_rgmii_speed(speed,duplex);					
						break;
					case 2:
					case 3:
						/*10f*/						
						state = 4;
						if(rgmii_polling_flag & 0x2)						
							printk("10F state=%d\n",state);						
						speed = PORT_SPEED_10M;
						duplex = PORT_FULL_DUPLEX;
						config_rgmii_speed(speed,duplex);						
						break;
					case 4:
					case 5:
					case 6:
					case 7:
						/*100h*/						
						state = 5;
						if(rgmii_polling_flag & 0x2)						
							printk("100H state=%d\n",state);						
						speed = PORT_SPEED_100M;
					    duplex = PORT_HALF_DUPLEX;
						config_rgmii_speed(speed,duplex);
					break;
					case 8:
					case 9:
					case 10:
					case 11:
					case 12:
					case 13:
					case 14:
					case 15:
						/*100f*/						
						state = 6;
						if(rgmii_polling_flag & 0x2)						
							printk("100F state=%d\n",state);						
						speed = PORT_SPEED_100M;
						duplex = PORT_FULL_DUPLEX;
						config_rgmii_speed(speed,duplex);					
					break;
					default:
						printk("[%s-%d] configure external phy error!\n",__func__,__LINE__);
						break;
				}
			}
		}
		else{
			/*link down*/
			current_state=7;
			if(state==current_state)
				return;
			state = 7;
			if(rgmii_polling_flag & 0x2)			
				printk("external phy link down!");
			DEBUG_RGMII_PRINT(DEBUG_ALWAYS,("[%s-%d] rgmii link down pData=0x%x\n",__func__,__LINE__,pData)); 			
			rtk_port_macForceAbility_get(RGMII_EXT_PORT,&port_mac_ability);
			port_mac_ability.linkStatus = PORT_LINKDOWN; 
			rtk_port_macForceAbility_set(RGMII_EXT_PORT,port_mac_ability);
		}
	}
}


int rgmii_polling_thread(void *data)
{


    while(!kthread_should_stop())
    {
    	/* No need to wake up earlier */
    	set_current_state(TASK_UNINTERRUPTIBLE);
    	schedule_timeout(5 * HZ / 10);    
    	if(rgmii_polling_flag){
			probe_rgmii();
		}
    }
	return 0;
}



static int rgmii_polling_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len;

	len = sprintf(page, "[rgmii_polling_flag = 0x%08x]\n", rgmii_polling_flag);
	len += sprintf(page + len, "0x1: enable rgmii polling\n");
	len += sprintf(page + len, "0x3: enable rgmii debug message\n");

	return len;
}

static int rgmii_polling_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	unsigned char tmpBuf[16] = {0};
	int len = (count > 15) ? 15 : count;

	if (buffer && !copy_from_user(tmpBuf, buffer, len))
	{
		rgmii_polling_flag = simple_strtoul(tmpBuf, NULL, 16);
		printk("write rgmii_polling_flag to 0x%08x\n", rgmii_polling_flag);
		return count;
	}
	return -EFAULT;
}


static void rgmii_polling_dbg_init(void)
{
    /* Create proc debug commands */
    if(NULL == rgmii_proc_dir)
    {
        rgmii_proc_dir = proc_mkdir("rgmii", NULL);
		printk("[%s-%d] \n", __FILE__, __LINE__);
    }
    if(rgmii_proc_dir)
    {
   		printk("[%s-%d] \n", __FILE__, __LINE__);
    	rgmii_polling_flag_entry = create_proc_entry("rgmii_polling_flag", 0, rgmii_proc_dir);
    	if(rgmii_polling_flag_entry){
    		rgmii_polling_flag_entry->read_proc = rgmii_polling_read;
    		rgmii_polling_flag_entry->write_proc = rgmii_polling_write;
			printk("[%s-%d] \n", __FILE__, __LINE__);
    	}
    }
}

static void rgmii_polling_dbg_exit(void)
{
    /* Remove proc debug commands */
    if(rgmii_polling_flag_entry)
    {
    	remove_proc_entry("polling_flag", rgmii_proc_dir);
        rgmii_polling_flag_entry = NULL;
    }
    if(rgmii_proc_dir)
    {
        remove_proc_entry("rgmii", NULL);
        rgmii_proc_dir = NULL;
    }
}



int __init rgmii_poling_init(void)
{
	unsigned int pData=0;
	unsigned int page = 0;
	rtk_port_macAbility_t port_mac_ability;
	int ret=0;
	//if check RT_ERR_OK, mean that it can detect external rgmii phy!!
	if(!rtk_port_phyReg_get(RGMII_EXT_PORT,page,PHY_REG_STATUS,&pData)){
		//enable external rgmii
		printk("%s-%d pData=0x%x\n",__func__,__LINE__,pData);
		RGMII_EN();
		//set tx, rx delay
		(*((volatile unsigned int *)(EXT_RGMXF)))|=0xc;
		(*((volatile unsigned int *)(EXT_TXC_DLY)))|=0x1;
		(*((volatile unsigned int *)(IOPAD_CFG)))|=0x2560000;
		ret = rtk_port_macForceAbility_get(RGMII_EXT_PORT,&port_mac_ability);
		printk("%s-%d ret=%d\n",__func__,__LINE__,ret);
		port_mac_ability.linkStatus = PORT_LINKDOWN; 
		ret = rtk_port_macForceAbility_set(RGMII_EXT_PORT,port_mac_ability);		
		printk("%s-%d ret=%d\n",__func__,__LINE__,ret);
		//first stat is link down!
		state = 7;
		//force rgmii state enable!
		ret = rtk_port_macForceAbilityState_set(RGMII_EXT_PORT,ENABLED);
		if(ret){
			printk("%s-%d error ret=%d",__FILE__,__LINE__,ret);
			return ret;
		}
    pRgmiiPollTask = kthread_create(rgmii_polling_thread, NULL, "rgmii_polling");
    if(IS_ERR(pRgmiiPollTask))
    {
        printk("[%s-%d] init failed %ld!\n", __FILE__, __LINE__, PTR_ERR(pRgmiiPollTask));
    }
    else
    {
        wake_up_process(pRgmiiPollTask);
        printk("[%s-%d] init complete!\n", __FILE__, __LINE__);
    }
    rgmii_polling_dbg_init();
	}else{
		printk("@@external rgmii detect fail!\n");
	}
    return 0;
}

void __exit rgmii_polling_exit(void)
{
	printk("[%s-%d] exit!\n", __FILE__, __LINE__);
	rgmii_en = 0;
	RGMII_DIS();
    kthread_stop(pRgmiiPollTask);
    rgmii_polling_dbg_exit();
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("RealTek RGMII polling module");
MODULE_AUTHOR("David Chen <david_cw@realtek.com>");
module_init(rgmii_poling_init);
module_exit(rgmii_polling_exit);





