#ifdef _AIPC_CPU_
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>	/* printk() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/cdev.h>
#include <linux/major.h>
#include <linux/device.h> 
#include <linux/version.h>
#include <linux/kthread.h>
#include <linux/delay.h>


//#include <asm/system.h>		/* cli(), *_flags */
#include <asm/uaccess.h>	/* copy_*_user */

#if defined(CONFIG_DEFAULTS_KERNEL_3_18) && !defined(CONFIG_LUNA)
#include "bspchip.h"
#else
#include "gpio.h"
#endif
#include "bspchip.h"

#include "./include/dram_share.h"
#include "./include/aipc_dev.h"		/* local definitions */
#include "./include/aipc_reg.h"
#include "./include/soc_type.h"
#include "./include/aipc_mem.h"
#include "./include/aipc_irq.h"
#include "./include/aipc_mbox.h"
#include "./include/aipc_debug.h"


#define AIPC_BUF_MALLOC

/*
*	Need to be modified as apollo setting
*/
static ul32_t *dst_addr = NULL;		//setup by ioctl
static ul32_t *src_addr = NULL;		//setup by ioctl
static ul32_t length = 0;			//setup by ioctl

#ifdef AIPC_BUF_MALLOC
static ul32_t *aipc_buf = NULL;
#else
static ul32_t  aipc_buf[(AIPC_BUF_SIZE)/(sizeof(ul32_t))] = {0};
#endif

static const size_t ul_size = sizeof(ul32_t);
static struct class *charmodule_class;

static int aipc_dev_major =   AIPC_DEV_MAJOR;
static int aipc_dev_minor =   0;
static int aipc_dev_nr_devs = AIPC_DEV_NR_DEVS;	/* number of bare aipc_dev devices */

module_param(aipc_dev_major, int, S_IRUGO);
module_param(aipc_dev_minor, int, S_IRUGO);
module_param(aipc_dev_nr_devs, int, S_IRUGO);

static aipc_dev_t aipc_dev;
static aipc_ioc_t aipc_ioc;

static void aipc_cpu_ipc_init(void);
extern int  aipc_cpu_irq_init(void);

#ifndef CONFIG_RTL8686_IPC_DUAL_LINUX
extern int  aipc_dsp_boot(void);
#else
extern int  aipc_dual_linux_boot(void);
#endif

#ifdef CONFIG_RTL8686_IPC_TEST_THREAD
static struct task_struct *aipc_startup_task;
static int aipc_cpu_test_thread_entry(void);
#endif
#ifdef CONFIG_RTL8686_READ_DRAM_THREAD
extern int cpu_read_task_start(void);
#endif

#ifdef CONFIG_RTL8686_IPC_DSP_CONSOLE
extern void  aipc_dsp_console_init_var(void);
extern int   aipc_dsp_console_init_module(void);
extern void  aipc_dsp_console_cleanup_module(void);
#endif

extern void aipc_dev_create_proc(void);
extern void aipc_module_voip_set_pcm_fs(void);
extern void aipc_module_voip_slic_reset(void);
extern void aipc_module_disable_wdog(void);
extern void aipc_module_voip_ip_enable(void);

extern unsigned int SOC_ID, SOC_BOND_ID;

MODULE_AUTHOR("Darren Shiue <darren_shiue@realtek.com>");
MODULE_LICENSE("GPL");

/*
 * Open and close
 */
int 
aipc_dev_open(struct inode *inode, struct file *filp)
{
	aipc_dev_t *dev; /* device information */

	dev = container_of(inode->i_cdev, aipc_dev_t, cdev);
	filp->private_data = dev; /* for other methods */
	return 0;          /* success */
}

int 
aipc_dev_release(struct inode *inode, struct file *filp)
{
	return 0;
}


ssize_t 
aipc_dev_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	aipc_dev_t *dev = filp->private_data; 
	ssize_t retval = 0;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	copy_to_user(buf , dst_addr , count);

    *f_pos += count;
    retval = count;

//	SDEBUG("buf=%p count=%u f_pos=%p *f_pos=%lu\n", 
//			buf , count , f_pos , (ul32_t)*f_pos);

	up(&dev->sem);
	return retval;
}


//add padding byte in the end
ul32_t *
aipc_word_copy(ul32_t *dst , ul32_t *src , size_t count)
{
    ul32_t * plDst = dst;
    ul32_t const * plSrc = src;
	size_t len = count;

	if (count%ul_size != 0){
		SDEBUG("wrong count indication. count=%u sizeof(ul32_t)=%u\n", count, ul_size);
		return NULL;
	}

    while (len>=ul_size){
    	*plDst++ = *plSrc++;
		len -= ul_size;
	}
		
    return (dst);
}

ul32_t *
aipc_align_copy(ul32_t *dst , ul32_t *src , size_t count)
{
	ul32_t remainder = count%ul_size;
	u8_t   padding = 0;
	
	switch (remainder)
	{
		case 0:
			padding=0;
			break;
		case 1:
			padding=3;
			break;
		case 2:
			padding=2;
			break;
		case 3:
			padding=1;
			break;
		default:
			break;
	}
	return aipc_word_copy(dst , src , count+padding);
}

void
aipc_memcpy_padding(void *dst_addr , void *src_addr , ul32_t length)

{
	ul32_t *dp = dst_addr;			//dst pointer. should be change to final destination
	char   *bp = src_addr;	
	ul32_t wc = 0;
	ul32_t rest = length;
	
	#ifdef AIPC_BUF_MALLOC
	aipc_buf = (ul32_t *)kmalloc(AIPC_BUF_SIZE , GFP_KERNEL);
	if (aipc_buf==NULL){
		SDEBUG("malloc failed\n");
		return;
		}
	#endif
	
	for (wc=0 ; rest>=AIPC_BUF_SIZE ; wc+=AIPC_BUF_SIZE, rest-=AIPC_BUF_SIZE){
		#ifdef AIPC_BUF_MALLOC
		memcpy(aipc_buf  , bp , AIPC_BUF_SIZE);
		aipc_word_copy(dp , aipc_buf  , AIPC_BUF_SIZE);
		#else
		memcpy(&aipc_buf , bp , AIPC_BUF_SIZE);
		aipc_word_copy(dp , (ul32_t*)&aipc_buf , AIPC_BUF_SIZE);
		#endif
		
		bp += AIPC_BUF_SIZE;			//char * pointer
		dp += AIPC_BUF_SIZE/ul_size;	//ul32_t * pointer
	}

	if (rest>0 && rest<AIPC_BUF_SIZE){
		#ifdef AIPC_BUF_MALLOC
		memset(aipc_buf, 0 , AIPC_BUF_SIZE);	//make sure all 0 in buffer before copy
		memcpy(aipc_buf , bp , rest);
		aipc_align_copy(dp , aipc_buf , rest);	//for the 4 byte padding
		#else
		memset(&aipc_buf, 0 , AIPC_BUF_SIZE);	//make sure all 0 in buffer before copy
		memcpy(&aipc_buf , bp , rest);
		aipc_align_copy(dp , (ul32_t*)&aipc_buf , rest);	//for the 4 byte padding
		#endif
		bp += rest;
	}else{
		SDEBUG("rest=%lu\n", rest);
	}

	#ifdef AIPC_BUF_MALLOC
	if (aipc_buf){
		kfree(aipc_buf);
		}

	aipc_buf=NULL;
	#endif
		
	//SDEBUG("dst_addr=%p src_addr=%p length=%lu rest=%lu\n", 
	//		dst_addr , src_addr ,   length , rest);
}


ssize_t 
aipc_dev_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	aipc_dev_t *dev = filp->private_data;
	ssize_t retval = -ENOMEM;

	ul32_t *dp = dst_addr;			//dst pointer. should be change to final destination
	const char __user *bp = buf;	//user space pointer
	ul32_t wc = 0;
	ul32_t rest = count;
	ul32_t tmp = 0;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

#ifdef AIPC_BUF_MALLOC
	aipc_buf = (ul32_t *)kmalloc(AIPC_BUF_SIZE , GFP_KERNEL);
	if (aipc_buf==NULL){
		SDEBUG("malloc failed\n");
		return -ERESTARTSYS;
		}
#endif

	if (aipc_ioc.write_file){
		for (wc=0 ; rest>=AIPC_BUF_SIZE ; wc+=AIPC_BUF_SIZE, rest-=AIPC_BUF_SIZE){
			#ifdef AIPC_BUF_MALLOC
			copy_from_user(aipc_buf  , bp , AIPC_BUF_SIZE);
			aipc_word_copy(dp , aipc_buf  , AIPC_BUF_SIZE);
			#else
			copy_from_user(&aipc_buf , bp , AIPC_BUF_SIZE);
			aipc_word_copy(dp , (ul32_t*)&aipc_buf , AIPC_BUF_SIZE);
			#endif
			
			bp += AIPC_BUF_SIZE;			//char * pointer
			dp += AIPC_BUF_SIZE/ul_size;	//ul32_t * pointer
		}

		if (rest>0 && rest<AIPC_BUF_SIZE){
			#ifdef AIPC_BUF_MALLOC
			memset(aipc_buf, 0 , AIPC_BUF_SIZE);	//make sure all 0 in buffer before copy
			copy_from_user(aipc_buf , bp , rest);
			aipc_align_copy(dp , aipc_buf , rest);	//for the 4 byte padding
			#else
			memset(&aipc_buf, 0 , AIPC_BUF_SIZE);	//make sure all 0 in buffer before copy
			copy_from_user(&aipc_buf , bp , rest);
			aipc_align_copy(dp , (ul32_t*)&aipc_buf , rest);	//for the 4 byte padding
			#endif
			bp += rest;
		} else if (rest==0) {			
			//SDEBUG("rest=%lu\n", rest);
		} else {
			SDEBUG("rest=%lu\n", rest);
		}
		
	}
	else if (aipc_ioc.write_word){
		copy_from_user(&tmp , bp , ul_size);
		if(dst_addr){
			*dst_addr = tmp; 
			SDEBUG("write word=0x%lx tmp=%lx\n", *dst_addr , tmp);
		}
		else{
			SDEBUG("wrong dst_address\n");
		}
	}
	else if (aipc_ioc.bitop_and){
		copy_from_user(&tmp , bp , ul_size);
		if(dst_addr){
			*dst_addr &= tmp;
			SDEBUG("and result 0x%lx tmp=%lx\n", *dst_addr , tmp);
		}
		else{
			SDEBUG("wrong dst_address\n");
		}
	}
	else if (aipc_ioc.bitop_or){
		copy_from_user(&tmp , bp , ul_size);
		if(dst_addr){
			*dst_addr |= tmp;
			SDEBUG("or result 0x%lx tmp=%lx\n", *dst_addr , tmp);
		}
		else{
			SDEBUG("wrong dst_address\n");
		}
	}
	else if (aipc_ioc.bitop_xor){
		copy_from_user(&tmp , bp , ul_size);
		if(dst_addr){
			*dst_addr ^= tmp;
			SDEBUG("xor result 0x%lx tmp=%lx\n", *dst_addr , tmp);
		}
		else{
			SDEBUG("wrong dst_address\n");
		}
	}
	else if (aipc_ioc.bitop_not){
		copy_from_user(&tmp , bp , ul_size);
		if(dst_addr){
			*dst_addr &= ~tmp;
			SDEBUG("not result 0x%lx tmp=%lx\n", *dst_addr , tmp);
		}
		else{
			SDEBUG("wrong dst_address\n");
		}
	}else{
		SDEBUG("undefined write action\n");
		}
	

	#ifdef AIPC_BUF_MALLOC
	if (aipc_buf){
		kfree(aipc_buf);
		}

	aipc_buf=NULL;
	#endif

    *f_pos += count;
    retval = count;

    /* update the size */
    if (dev->size < *f_pos)
        dev->size = *f_pos;

//	SDEBUG("buf=%p bp=%p count=%u rest=%lu f_pos=%p *f_pos=%lu\n", 
//			buf , bp , count , rest , f_pos , (ul32_t)*f_pos);

	up(&dev->sem);

	return retval;
}


void
aipc_dev_memcpy( void *dst_addr , void *src_addr , ul32_t length , int add_padding )
{
	if (!add_padding)
		memcpy(dst_addr , src_addr , length);
	else{
		aipc_memcpy_padding(dst_addr , src_addr , length);
	}
}

/*
 * The ioctl() implementation
 */
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35))
long aipc_dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
#else
int aipc_dev_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, ul32_t arg)
#endif
{
	int retval = 0;
	ul32_t rom_addr = 0;
	ul32_t tmp = 0;
	ul32_t sram_addr = 0;

	#ifdef IPC_HW_MUTEX_CCODE
	volatile ul32_t result = 0;
	#endif

	switch(cmd) {
	case IOCTL_DST_ADDR: /* Set: arg points to the value */
		dst_addr	= (ul32_t*)arg;
		break;

	case IOCTL_SRC_ADDR:
		src_addr	= (ul32_t*)arg;
		break;

	case IOCTL_LENGTH:
		length		= (ul32_t)arg;
		break;

	case IOCTL_COPY:
		aipc_dev_memcpy(dst_addr , src_addr , length , 0);
		break;

	case IOCTL_COPY_PADDING:
		aipc_dev_memcpy(dst_addr , src_addr , length , 1);
		break;

	case IOCTL_WF:	/* Set: Write File*/
		memset(&aipc_ioc , 0 , sizeof(aipc_ioc));
		aipc_ioc.write_file = 1;
		break;
	
	case IOCTL_WW:	/* Set: Write Word*/
		memset(&aipc_ioc , 0 , sizeof(aipc_ioc));
		aipc_ioc.write_word = 1;
		break;

	case IOCTL_BITOP_AND:
		memset(&aipc_ioc , 0 , sizeof(aipc_ioc));
		aipc_ioc.bitop_and  = 1;
		break;

	case IOCTL_BITOP_OR:
		memset(&aipc_ioc , 0 , sizeof(aipc_ioc));
		aipc_ioc.bitop_or   = 1;
		break;

	case IOCTL_BITOP_XOR:
		memset(&aipc_ioc , 0 , sizeof(aipc_ioc));
		aipc_ioc.bitop_xor  = 1;
		break;

	case IOCTL_BITOP_NOT:
		memset(&aipc_ioc , 0 , sizeof(aipc_ioc));
		aipc_ioc.bitop_not  = 1;
		break;

	case IOCTL_BOOT_DSP:
#ifdef BOOT_DSP_DELAY
		//msleep(BOOT_DSP_DELAY_TIME);
		mdelay(BOOT_DSP_DELAY_TIME);
#endif
#if !defined(CONFIG_RTL8686_IPC_SINGLE_CPU)
		aipc_zone_set(zp_dsp_boot);

		aipc_cpu_ipc_init();

#ifdef CONFIG_RTL8686_IPC_DSP_CONSOLE
		aipc_dsp_console_init_var();
#endif
#ifndef CONFIG_RTL8686_IPC_DUAL_LINUX
		aipc_dsp_boot();
		SDEBUG("boot dsp\n");
#else
		aipc_dual_linux_boot();
		SDEBUG("boot dual linux\n");
#endif
#endif
		break;

	case IOCTL_CPU_DRAM_UNMAP:
		break;
			
	case IOCTL_CPU_SRAM_MAP:
		break;

	case IOCTL_DSP_ENTRY:
#if !defined(CONFIG_RTL8686_IPC_SINGLE_CPU)
		aipc_dsp_entry();
#endif
		break;
	
	case IOCTL_SOC_SRAM_BACKUP:
#if !defined(CONFIG_RTL8686_IPC_SINGLE_CPU)
		aipc_soc_sram_backup();
#endif
		break;
		
	case IOCTL_ZONE_SET:
#if !defined(CONFIG_RTL8686_IPC_SINGLE_CPU)
		aipc_zone_set(zp_dsp_init);
#endif
		break;

	case IOCTL_TRIGGER:
		tmp = (ul32_t)arg;

		if (tmp==T_DSP){		//trigger DSP
			aipc_int_assert( T_DSP );
			SDEBUG("tirgger DSP\n");
		}
		else{					//trigger CPU
			aipc_int_assert( T_CPU );
			SDEBUG("trigger CPU\n");
		}
		break;

	case IOCTL_ROM_SET:
		rom_addr = (ul32_t)arg;
		
#if !defined(CONFIG_RTL8686_IPC_SINGLE_CPU)
		aipc_rom_set(rom_addr);
		SDEBUG("set ROM. rom_addr=0x%lx\n" , rom_addr);
#endif
		break;

	case IOCTL_SOC_SRAM_SET:
		sram_addr = (ul32_t)arg;

#if !defined(CONFIG_RTL8686_IPC_SINGLE_CPU)
		aipc_soc_sram_set(sram_addr);
#endif
		break;	

	case IOCTL_INIT_IPC:	
#ifdef CONFIG_RTL8686_IPC_TEST_THREAD
		aipc_cpu_test_thread_entry();
#endif
#ifdef CONFIG_RTL8686_READ_DRAM_THREAD
		cpu_read_task_start();
#endif
#if !defined(CONFIG_RTL8686_IPC_SINGLE_CPU)
		ACTSW.init |= INIT_OK;
#endif
		
		break;

	case IOCTL_RESET_SLIC:
		#ifdef AIPC_MODULE_VOIP_SET_PCM_FS
		aipc_module_voip_set_pcm_fs();
		#endif	
		#ifdef CONFIG_RTL8686_SLIC_RESET
		aipc_module_voip_slic_reset();
		#endif
		break;

	case IOCTL_CTRL_2DSP_SEND:
		ATHREAD.ctrl_2dsp_send = (u32_t)arg;
		break;

	case IOCTL_CTRL_2DSP_POLL:
		ATHREAD.ctrl_2dsp_poll = (u32_t)arg;
		break;

	case IOCTL_CTRL_2CPU_SEND:
		ATHREAD.ctrl_2cpu_send = (u32_t)arg;
		break;

	case IOCTL_CTRL_2CPU_POLL:
		ATHREAD.ctrl_2cpu_poll = (u32_t)arg;
		break;

	case IOCTL_MBOX_2DSP_SEND: 
		ATHREAD.mbox_2dsp_send = (u32_t)arg;
		break;

	case IOCTL_MBOX_2DSP_POLL:
		ATHREAD.mbox_2dsp_poll = (u32_t)arg;
		break;

	case IOCTL_MBOX_2CPU_SEND:
		ATHREAD.mbox_2cpu_send = (u32_t)arg;
		break;

	case IOCTL_MBOX_2CPU_RECV:
		ATHREAD.mbox_2cpu_recv = (u32_t)arg;
		break;
	
	case IOCTL_DBG_PRINT:
		tmp = (ul32_t)arg;

		if(!tmp){
			ACTSW.dbg_mask = 0;			//set as 0. turn off debug print
			}
		else{
			ACTSW.dbg_mask |= tmp;		//turn on debug print mask.
			}
		break;

	case IOCTL_DBG_DUMP:
		break;

	default: 
		retval = -ENOTTY;
	}
	return retval;
}

loff_t 
aipc_dev_llseek(struct file *filp, loff_t off, int whence)
{
	aipc_dev_t *dev = filp->private_data;
	loff_t newpos;

	switch(whence) {
	  case 0: /* SEEK_SET */
		newpos = off;
		break;

	  case 1: /* SEEK_CUR */
		newpos = filp->f_pos + off;
		break;

	  case 2: /* SEEK_END */
		newpos = dev->size + off;
		break;

	  default: /* can't happen */
		return -EINVAL;
	}
	if (newpos < 0) return -EINVAL;
	filp->f_pos = newpos;
	return newpos;
}

struct file_operations aipc_dev_fops = {
	.owner =    THIS_MODULE,
	.llseek =   aipc_dev_llseek,
	.read =     aipc_dev_read,
	.write =    aipc_dev_write,
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35))
	.unlocked_ioctl = aipc_dev_ioctl,
#else
	.ioctl =    aipc_dev_ioctl,
#endif
	.open =     aipc_dev_open,
	.release =  aipc_dev_release,
};


/*
 * Set up the char_dev structure for this device.
 */
static void 
aipc_dev_setup_cdev(aipc_dev_t *dev, int index)
{
	int err, devno = MKDEV(aipc_dev_major, aipc_dev_minor + index);
    
	cdev_init(&dev->cdev, &aipc_dev_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &aipc_dev_fops;
	err = cdev_add (&dev->cdev, devno, 1);
	/* Fail gracefully if need be */
	if (err)
		printk(KERN_NOTICE "Error %d adding aipc_dev%d\n", err, index);
}

void 
aipc_dev_cleanup_module(void)
{
	dev_t devno = MKDEV(aipc_dev_major, aipc_dev_minor);
	
	/* cleanup_module is never called if registering failed */
	unregister_chrdev_region(devno, aipc_dev_nr_devs);

	device_destroy(charmodule_class, MKDEV(aipc_dev_major, aipc_dev_minor));
	class_destroy(charmodule_class);

#ifdef CONFIG_RTL8686_IPC_DSP_CONSOLE	
	aipc_dsp_console_cleanup_module();
#endif
}


#if	defined(REFINE_SHM_NOTIFY) && defined(CONFIG_RTL8686_SHM_NOTIFY)
static void
aipc_shm_notify_init(void)
{
	extern const unsigned int *aipc_shm_notify_cpu;
	extern const unsigned int *aipc_shm_notify_dsp;

	REG32(aipc_shm_notify_cpu) = 0;
	REG32(aipc_shm_notify_dsp) = 0;
}
#elif !defined(REFINE_SHM_NOTIFY) && defined(CONFIG_RTL8686_SHM_NOTIFY)
static void
aipc_shm_notify_init(void)
{
	REG32(AIPC_SHM_NOTIFY_CPU) = 0;
	REG32(AIPC_SHM_NOTIFY_DSP) = 0;
}
#endif



#if !defined(CONFIG_RTL8686_IPC_SINGLE_CPU)
static void
aipc_ipc_clear(void)
{
	memset(ADRAM, 0 , sizeof(aipc_dram_t));
	memset(ASRAM, 0 , sizeof(aipc_sram_t));
}

static void 
aipc_cpu_ipc_init(void)
{
	extern void aipc_mb_bc_init(void);
	extern void aipc_intq_init(void);
	extern void aipc_ctrl_event_init(void);
#ifdef AIPC_MSG_PROBE
	extern void aipc_msg_probe_init(void);
#endif
#ifdef CONFIG_RTL8686_IPC_DSL_IPC
	extern int  aipc_cpu_dsl_ctrl_init(void);
	extern int  aipc_cpu_dsl_eoc_init(void);
	extern int  aipc_cpu_dsl_event_queue_init(void);
#endif

	//clear ipc related data sturctures
	aipc_ipc_clear();
	
	//data plane init
	aipc_mb_bc_init();	//init mbox and bc
	aipc_intq_init();	//init int queue

	//control plane init
	aipc_ctrl_event_init();

#ifdef CONFIG_RTL8686_SHM_NOTIFY
	aipc_shm_notify_init();
#endif

#ifdef AIPC_MSG_PROBE
	aipc_msg_probe_init();
#endif

#ifdef CONFIG_RTL8686_IPC_DSL_IPC
	aipc_cpu_dsl_ctrl_init();
	aipc_cpu_dsl_eoc_init();
	aipc_cpu_dsl_event_queue_init();
#endif

	//ACTSW.init |= INIT_OK;
}
#endif


#ifdef CONFIG_RTL8686_IPC_TEST_THREAD
#define CPU_STARTUP_THREAD "IpcStartD"

static int 
aipc_startup_thread(void *p)
{
	extern int	mbox_2dsp_task_init(void);
	extern int	cpu_ctrl_task_init(void);

	static int init_done  = 0;

	SDEBUG("%s start\n" , CPU_STARTUP_THREAD);
	
	set_current_state(TASK_INTERRUPTIBLE);
	do {					
		msleep_interruptible(CPU_CHECK_START_PERIOD);
		//ADEBUG(DBG_SYSTEM, "%s executing ACTSW.init=%x addr=%p\n" , 
		//CPU_STARTUP_THREAD , ACTSW.init , &(ACTSW));

		if(!init_done){
			if((ACTSW.init & INIT_OK)){
				//control & data test thread init
				cpu_ctrl_task_init();
				mbox_2dsp_task_init();
				init_done = 1;
				break;
				}
			}
	} while (!kthread_should_stop());

	ADEBUG(DBG_SYSTEM, "%s exit\n" , CPU_STARTUP_THREAD);
	kthread_stop(aipc_startup_task);
	
	return OK;
}

static int
aipc_cpu_test_thread_entry(void)
{
	aipc_startup_task = kthread_run(
					aipc_startup_thread,
					NULL, 
					CPU_STARTUP_THREAD);

	if (!IS_ERR(aipc_startup_task)){
		printk("%s create successfully!\n" , CPU_STARTUP_THREAD);
		return OK;
	}

	printk("%s create failed!\n" , CPU_STARTUP_THREAD);
	return NOK;
}
#endif


#ifdef REBOOT_THREAD
#define REBOOT_THREAD_NAME "RebootD"
#define REBOOT_WAIT_TIME (20*HZ)

static struct task_struct *aipc_reboot_task;

static int 
aipc_reboot_thread(void *p)
{
	volatile unsigned int *dsp_reg;
	SDEBUG("%s start\n" , REBOOT_THREAD_NAME);
	
	set_current_state(TASK_INTERRUPTIBLE);
	do {
		SDEBUG("Schedule out ...\n");
		schedule_timeout_interruptible(REBOOT_WAIT_TIME);
		SDEBUG("Schedule back ...\n");		
		
		SDEBUG("Reset CPU\n");
		dsp_reg    =  (volatile unsigned int *)(R_AIPC_ASIC_RESET_CPU);
		*dsp_reg  |=  BIT_RESET_CPU;
		/* Use WDT to do full chip reset */
		dsp_reg    =  (volatile unsigned int *)(0xb8003268);
		*dsp_reg  |=  0x80000000;
		
	} while (!kthread_should_stop());
	kthread_stop(aipc_reboot_task);
	
	return OK;
}

static int
aipc_reboot_task_init(void)
{
	aipc_reboot_task = kthread_run(
					aipc_reboot_thread,
					NULL, 
					REBOOT_THREAD_NAME);

	if (!IS_ERR(aipc_reboot_task)){
		printk("%s create successfully!\n" , REBOOT_THREAD_NAME);
		return OK;
	}

	printk("%s create failed!\n" , REBOOT_THREAD_NAME);
	return NOK;
}
#endif

#ifdef AIPC_MODULE_INIT_ZONE_ENTRY
static void
aipc_module_init_zone_entry(void)
{
	aipc_zone_set(zp_dsp_init);
#if 0 // move this to rc
	aipc_dsp_entry();
#endif
	aipc_rom_set(DSP_ROMCODE_ADDR);
}
#endif

static void
aipc_module_dump_soc_bond_info(void)
{
	printk( "SOC_ID 0x%04x\n" , SOC_ID );

	switch(SOC_BOND_ID)	{
	    case CHIP_901   : printk( "CHIP_901   0x%04x\n" , CHIP_901 );   break;
		case CHIP_906_1 : printk( "CHIP_906_1 0x%04x\n" , CHIP_906_1 ); break;
		case CHIP_906_2 : printk( "CHIP_906_2 0x%04x\n" , CHIP_906_2 ); break;
		case CHIP_907   : printk( "CHIP_907   0x%04x\n" , CHIP_907 );   break;
		case CHIP_902   : printk( "CHIP_902   0x%04x\n" , CHIP_902 );   break;
		case CHIP_903   : printk( "CHIP_903   0x%04x\n" , CHIP_903 );   break;
		case CHIP_96    : printk( "CHIP_96    0x%04x\n" , CHIP_96 );    break;
		case CHIP_98B   : printk( "CHIP_98B   0x%04x\n" , CHIP_98B );   break;
		case CHIP_DBG   : printk( "CHIP_DBG   0x%04x\n" , CHIP_DBG );   break;
		default: printk( "unknown bond id\n");
	}
}

//#ifdef CONFIG_RTL8686_IPC_UART1_ENABLE
//#define SOC_0371_UART1_PINMUX     0xbb02301c
//#define SOC_6266_IO_LED_EN_REG_A  0xbb023014
//#define LED8_EN (0x1 << 8)
//#define LED6_EN (0x1 << 6)
//#define SOC_6266_UART1_PINMUX     0xbb023018
//
//void
//aipc_module_uart1_pinmux(void)
//{
//    if (SOC_ID==0x0371) {
//        REG32(SOC_0371_UART1_PINMUX) = 0;
//    //  SDEBUG("SOC_0371_UART1_PINMUX 0x%08x\n" , REG32(SOC_0371_UART1_PINMUX));
//    }
//
//    printk("SOC_BOND_ID=0x%08x\n", SOC_BOND_ID);
//    switch(SOC_BOND_ID) {
//        case CHIP_901   :  break;
//        case CHIP_906_1 :
//                REG32(0xbb0010f0)    = 0;
//                REG32(0xbb0010f4)    = 0;
//                REG32(0xbb0010f8)    = 0;
//                REG32(0xbb0010fc)    = 0;
//                REG32(0xbb000100)    = 0;
//                REG32(0xbb000104)    = 0;
//                REG32(0xbb023014)    = 0;
//                REG32(SOC_6266_UART1_PINMUX) |= (1<<4);
//
//                printk("0xbb0010f0 0x%08x\n" ,  REG32(0xbb0010f0));
//                printk("0xbb0010f4 0x%08x\n" ,  REG32(0xbb0010f4));
//                printk("0xbb0010f8 0x%08x\n" ,  REG32(0xbb0010f8));
//                printk("0xbb0010fc 0x%08x\n" ,  REG32(0xbb0010fc));
//                printk("0xbb000100 0x%08x\n" ,  REG32(0xbb000100));
//                printk("0xbb000104 0x%08x\n" ,  REG32(0xbb000104));
//                printk("0xbb023014 0x%08x\n" ,  REG32(0xbb023014));
//                printk("0xbb023018 0x%08x\n" ,  REG32(SOC_6266_UART1_PINMUX));
//            break;
//        case CHIP_906_2 :  break;
//        case CHIP_902   :  break;
//        case CHIP_903   :  break;
//
//        /* UTX1D1 is pinmuxed with  GPIO13/LED8(P0LED0) */
//        /* URX1D1 is pinmuxed with  GPIO24/LED6(P1LED0) */
//        case CHIP_907   :  
//        case CHIP_96    :  
//        case CHIP_2510  :  
//                /* ToDo: GPIO13/24 are suggested to be turned explicitly.                     The current code works because GPIO13/24 are turned off by default */
//		/* Setting is obtained from the development of Ken@sd5 */
//                REG32(SOC_6266_IO_LED_EN_REG_A) &= ~(LED6_EN | LED8_EN);
//                REG32(SOC_6266_UART1_PINMUX) |= (1<<4);
//                printk("0xbb023014 0x%08x\n" ,  REG32(0xbb023014));
//                printk("0xbb023018 0x%08x\n" ,  REG32(SOC_6266_UART1_PINMUX));
//            break;
//        case CHIP_98B   :  break;
//        case CHIP_DBG   :  break;
//        default: printk( "unknown bond id\n");
//    }
//}
//#endif

static void
aipc_module_pre_init(void)
{
	aipc_module_dump_soc_bond_info();

#if defined(AIPC_MODULE_VOIP_IP_ENABLE) || defined(CONFIG_RTK_VOIP)
	aipc_module_voip_ip_enable();
#endif

//#ifdef AIPC_MODULE_VOIP_SET_PCM_FS   // move to slic driver 
//	aipc_module_voip_set_pcm_fs();
//#endif

//#ifdef CONFIG_RTL8686_SLIC_RESET     // move to slic driver 
//	aipc_module_voip_slic_reset();
//#endif

//#ifdef CONFIG_RTL8686_IPC_UART1_ENABLE
//	aipc_module_uart1_pinmux();
//#endif

#ifdef AIPC_MODULE_DISALBE_WDOG
    aipc_module_disable_wdog();
#endif

#ifdef  AIPC_MODULE_INIT_ZONE_ENTRY
    aipc_module_init_zone_entry();
#endif
}

int 
aipc_dev_init_module(void)
{
	int result;
	dev_t dev = 0;
	struct device *dp;

	aipc_module_pre_init();

	memset(&aipc_dev , 0 , sizeof(aipc_dev));
	memset(&aipc_ioc , 0 , sizeof(aipc_ioc));


	#if 1
	result = alloc_chrdev_region(&dev, aipc_dev_minor, aipc_dev_nr_devs, DEVICE_NAME);
	aipc_dev_major = MAJOR(dev);
	aipc_dev_minor = MINOR(dev);
	#else
	if (aipc_dev_major) {
		dev = MKDEV(aipc_dev_major, aipc_dev_minor);
		result = register_chrdev_region(dev, aipc_dev_nr_devs, DEVICE_NAME);
	} else {
		result = alloc_chrdev_region(&dev, aipc_dev_minor, aipc_dev_nr_devs, DEVICE_NAME);
		aipc_dev_major = MAJOR(dev);
	}
	#endif

	printk("aipc: register chrdev(%d,%d)\n", aipc_dev_major, aipc_dev_minor);	

	if (result) {
		printk("aipc: can't get major %d\n", aipc_dev_major);
		goto fail;
	}

	charmodule_class = class_create(THIS_MODULE, DEVICE_NAME);
	if (IS_ERR(charmodule_class))
		return -EFAULT;

	#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
	dp = device_create(charmodule_class, NULL , MKDEV(aipc_dev_major, aipc_dev_minor) , NULL , DEVICE_NAME);
	#else
	dp = device_create(charmodule_class, NULL , MKDEV(aipc_dev_major, aipc_dev_minor) , DEVICE_NAME);
	#endif

	if (IS_ERR(dp))
		printk( "aipc: create device failed\n" );
	else
		printk( "aipc: create device successed\n" );

	#if !defined(CONFIG_RTL8686_IPC_SINGLE_CPU)
	aipc_dev_create_proc();
	#endif

	#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36))
	sema_init(&aipc_dev.sem , 1);
	#else
	init_MUTEX(&aipc_dev.sem);
	#endif
	
	aipc_dev_setup_cdev(&aipc_dev, 0);
	
	#if !defined(CONFIG_RTL8686_IPC_SINGLE_CPU)
	aipc_cpu_irq_init();
	#endif

	#ifdef CONFIG_RTL8686_IPC_DSP_CONSOLE
	aipc_dsp_console_init_module();
	#endif

	#ifdef REBOOT_THREAD
	aipc_reboot_task_init();
	#endif

	printk( "aipc: init done\n" );

	return 0; /* succeed */

fail:
	aipc_dev_cleanup_module();
	return result;
}

module_init(aipc_dev_init_module);
module_exit(aipc_dev_cleanup_module);

#endif

