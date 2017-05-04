#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>     /* kmalloc() */
#include <linux/fs.h>       /* everything... */
#include <linux/errno.h>    /* error codes */
#include <linux/types.h>    /* size_t */
#include <linux/fcntl.h>    /* O_ACCMODE */
#include <linux/cdev.h>
#include <linux/major.h>
#include <linux/device.h>
#include <linux/version.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/poll.h>
#include <linux/semaphore.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>

//#include <asm/system.h>     /* cli(), *_flags */
#include <asm/uaccess.h>    /* copy_*_user */


#elif defined(__ECOS)

#include <cyg/hal/hal_intr.h>
#include <cyg/infra/cyg_type.h>

#endif

#include "./include/aipc_osal.h"
#include "./include/aipc_define.h"
#include "./include/aipc_shm.h"
#include "./include/aipc_debug.h"
#include "./include/aipc_swp.h"

#define DSP_READ_BUF_NOT_EMPTY  (!(((ADSP_CONSOLE_RD.ins + DSP_CONSOLE_READ_BUF_SIZE - ADSP_CONSOLE_RD.del)  % DSP_CONSOLE_READ_BUF_SIZE)  == 0))
#define DSP_WRITE_BUF_NOT_FULL  (!(((ADSP_CONSOLE_WR.del + DSP_CONSOLE_WRITE_BUF_SIZE - ADSP_CONSOLE_WR.ins) % DSP_CONSOLE_WRITE_BUF_SIZE) == 1))

/*
*	Common code for both CPU and DSP
*/
#ifdef CONFIG_RTL8686_IPC_DSP_CONSOLE

unsigned int aipc_dsp_console_current_enable(void)
{
	return ADSP_CONSOLE.enable;
}

/* Read Buffer */
int aipc_dsp_console_read_buf_full(void)
{
	if (((ADSP_CONSOLE_RD.del + DSP_CONSOLE_READ_BUF_SIZE - ADSP_CONSOLE_RD.ins) % DSP_CONSOLE_READ_BUF_SIZE) == 1){
		return TRUE;	
	}
	else{
		return FALSE;
	}
}

int aipc_dsp_console_read_buf_empty(void)
{
	if (((ADSP_CONSOLE_RD.ins + DSP_CONSOLE_READ_BUF_SIZE - ADSP_CONSOLE_RD.del) % DSP_CONSOLE_READ_BUF_SIZE) == 0){
		return TRUE;	
	}
	else{
		return FALSE;
	}
}

unsigned int aipc_dsp_console_read_buf_use(void)
{
	return ((ADSP_CONSOLE_RD.ins + DSP_CONSOLE_READ_BUF_SIZE - ADSP_CONSOLE_RD.del) % DSP_CONSOLE_READ_BUF_SIZE);
}

#ifdef _AIPC_DSP_
int aipc_dsp_console_read_buf_enqueue(char ch)
{
#ifdef AIPC_BARRIER
	volatile unsigned int tmp_wr=0;

	AIPC_DISABLE_INTERRUPTS;
	tmp_wr = ADSP_CONSOLE_RD.eq_try ;
	tmp_wr++;
	AMB( ADSP_CONSOLE_RD.eq_try , tmp_wr );
	
	if ((ADSP_CONSOLE.enable) && (ADSP_CONSOLE_RD.enable) && (!aipc_dsp_console_read_buf_full())) {
		tmp_wr = ADSP_CONSOLE_RD.ins;
		AMB( ADSP_CONSOLE_RD.read_buf[ tmp_wr ] , ch );
		
		tmp_wr = (tmp_wr + 1) % DSP_CONSOLE_READ_BUF_SIZE;
		AMB( ADSP_CONSOLE_RD.ins , tmp_wr );
		
		tmp_wr = ADSP_CONSOLE_RD.eq_ok;
		tmp_wr++;
		AMB( ADSP_CONSOLE_RD.eq_ok , tmp_wr);
		
		AIPC_RESTORE_INTERRUPTS;
		return TRUE;
	}
	else{
		AIPC_RESTORE_INTERRUPTS;
		return FALSE;	
	}
	
#else	
	AIPC_DISABLE_INTERRUPTS;
	ADSP_CONSOLE_RD.eq_try ++;
	
	if ((ADSP_CONSOLE.enable) && (ADSP_CONSOLE_RD.enable) && (!aipc_dsp_console_read_buf_full())) {
		ADSP_CONSOLE_RD.read_buf[ ADSP_CONSOLE_RD.ins ] = ch;
		ADSP_CONSOLE_RD.ins = (ADSP_CONSOLE_RD.ins + 1) % DSP_CONSOLE_READ_BUF_SIZE;
		
		ADSP_CONSOLE_RD.eq_ok ++;
		
		AIPC_RESTORE_INTERRUPTS;
		return TRUE;
	}
	else{
		AIPC_RESTORE_INTERRUPTS;
		return FALSE;	
	}
#endif
}
#endif

#ifdef _AIPC_CPU_
int aipc_dsp_console_read_buf_dequeue(char *ch)
{
#ifdef AIPC_BARRIER
	volatile unsigned int tmp_wr=0;
	
	AIPC_DISABLE_INTERRUPTS;
	
	tmp_wr = ADSP_CONSOLE_RD.dq_try;
	tmp_wr++;
	AMB( ADSP_CONSOLE_RD.dq_try , tmp_wr );
	
	if ((ADSP_CONSOLE.enable) && (ADSP_CONSOLE_RD.enable) && (!aipc_dsp_console_read_buf_empty()) && (ch)){
		tmp_wr = ADSP_CONSOLE_RD.del;
		*ch = ADSP_CONSOLE_RD.read_buf[ tmp_wr ];
		tmp_wr = (tmp_wr + 1) % DSP_CONSOLE_READ_BUF_SIZE;
		AMB( ADSP_CONSOLE_RD.del , tmp_wr );
		
		tmp_wr = ADSP_CONSOLE_RD.dq_ok;
		tmp_wr++;
		AMB( ADSP_CONSOLE_RD.dq_try , tmp_wr );
		
		AIPC_RESTORE_INTERRUPTS;
		return TRUE;
	}
	else{
		AIPC_RESTORE_INTERRUPTS;
		return FALSE;	
	}

#else

	AIPC_DISABLE_INTERRUPTS;
	ADSP_CONSOLE_RD.dq_try ++;
	
	if ((ADSP_CONSOLE.enable) && (ADSP_CONSOLE_RD.enable) && (!aipc_dsp_console_read_buf_empty()) && (ch)){
		*ch = ADSP_CONSOLE_RD.read_buf[ ADSP_CONSOLE_RD.del ];
		ADSP_CONSOLE_RD.del = (ADSP_CONSOLE_RD.del + 1) % DSP_CONSOLE_READ_BUF_SIZE;
		ADSP_CONSOLE_RD.dq_ok ++;
		
		AIPC_RESTORE_INTERRUPTS;
		return TRUE;
	}
	else{
		AIPC_RESTORE_INTERRUPTS;
		return FALSE;	
	}

#endif
}
#endif

/* Write Buffer */
int aipc_dsp_console_write_buf_full(void)
{
	if (((ADSP_CONSOLE_WR.del + DSP_CONSOLE_WRITE_BUF_SIZE - ADSP_CONSOLE_WR.ins) % DSP_CONSOLE_WRITE_BUF_SIZE) == 1){
		return TRUE;	
	}
	else{
		return FALSE;
	}
}

int aipc_dsp_console_write_buf_empty(void)
{
	if (((ADSP_CONSOLE_WR.ins + DSP_CONSOLE_WRITE_BUF_SIZE - ADSP_CONSOLE_WR.del) % DSP_CONSOLE_WRITE_BUF_SIZE) == 0){
		return TRUE;
	}
	else{
		return FALSE;
	}
}

unsigned int aipc_dsp_console_write_buf_use(void)
{
	return ((ADSP_CONSOLE_WR.ins + DSP_CONSOLE_WRITE_BUF_SIZE - ADSP_CONSOLE_WR.del) % DSP_CONSOLE_WRITE_BUF_SIZE);
}

#ifdef _AIPC_CPU_
int aipc_dsp_console_write_buf_enqueue(char ch)
{
#ifdef AIPC_BARRIER
	volatile unsigned int tmp_wr=0;

	AIPC_DISABLE_INTERRUPTS;
	tmp_wr = ADSP_CONSOLE_WR.eq_try;
	tmp_wr++;
	AMB( ADSP_CONSOLE_WR.eq_try , tmp_wr );

	if ((ADSP_CONSOLE.enable) && (ADSP_CONSOLE_WR.enable) && (!aipc_dsp_console_write_buf_full())) {
		tmp_wr = ADSP_CONSOLE_WR.ins;		
		AMB( ADSP_CONSOLE_WR.write_buf[ tmp_wr ] , ch );
		
		tmp_wr = ADSP_CONSOLE_WR.ins;
		tmp_wr = (tmp_wr+1) % DSP_CONSOLE_WRITE_BUF_SIZE;
		AMB( ADSP_CONSOLE_WR.ins , tmp_wr );
		
		tmp_wr = ADSP_CONSOLE_WR.eq_ok;
		tmp_wr++;
		AMB( ADSP_CONSOLE_WR.eq_ok , tmp_wr );
		
		AIPC_RESTORE_INTERRUPTS;
		return TRUE;
	}
	else{
		AIPC_RESTORE_INTERRUPTS;
		return FALSE;	
	}
#else
	AIPC_DISABLE_INTERRUPTS;
	ADSP_CONSOLE_WR.eq_try ++;

	if ((ADSP_CONSOLE.enable) && (ADSP_CONSOLE_WR.enable) && (!aipc_dsp_console_write_buf_full())) {
		ADSP_CONSOLE_WR.write_buf[ ADSP_CONSOLE_WR.ins ] = ch;
		ADSP_CONSOLE_WR.ins = (ADSP_CONSOLE_WR.ins + 1) % DSP_CONSOLE_WRITE_BUF_SIZE;
		ADSP_CONSOLE_WR.eq_ok ++;
		
		AIPC_RESTORE_INTERRUPTS;
		return TRUE;
	}
	else{
		AIPC_RESTORE_INTERRUPTS;
		return FALSE;	
	}
#endif
}
#endif

#ifdef _AIPC_DSP_
int aipc_dsp_console_write_buf_dequeue(char *ch)
{
#ifdef AIPC_BARRIER
	volatile unsigned int tmp_wr=0;

	AIPC_DISABLE_INTERRUPTS;
	tmp_wr = ADSP_CONSOLE_WR.dq_try;
	tmp_wr++;
	AMB( ADSP_CONSOLE_WR.dq_try , tmp_wr );
	
	if ((ADSP_CONSOLE.enable) && (ADSP_CONSOLE_WR.enable) && (!aipc_dsp_console_write_buf_empty()) && (ch)) {
		tmp_wr = ADSP_CONSOLE_WR.del;
		*ch    = ADSP_CONSOLE_WR.write_buf[ tmp_wr ];
		tmp_wr = (tmp_wr + 1) % DSP_CONSOLE_WRITE_BUF_SIZE;
		AMB( ADSP_CONSOLE_WR.del , tmp_wr );
		
		tmp_wr = ADSP_CONSOLE_WR.dq_ok;
		tmp_wr++;
		AMB( ADSP_CONSOLE_WR.dq_ok , tmp_wr );
		
		AIPC_RESTORE_INTERRUPTS;
		return TRUE;
	}
	else{
		AIPC_RESTORE_INTERRUPTS;
		return FALSE;	
	}

#else

	AIPC_DISABLE_INTERRUPTS;
	ADSP_CONSOLE_WR.dq_try ++;
	
	if ((ADSP_CONSOLE.enable) && (ADSP_CONSOLE_WR.enable) && (!aipc_dsp_console_write_buf_empty()) && (ch)) {
		*ch = ADSP_CONSOLE_WR.write_buf[ ADSP_CONSOLE_WR.del ];
		ADSP_CONSOLE_WR.del = (ADSP_CONSOLE_WR.del + 1) % DSP_CONSOLE_WRITE_BUF_SIZE;
		ADSP_CONSOLE_WR.dq_ok ++;
		
		AIPC_RESTORE_INTERRUPTS;
		return TRUE;
	}
	else{
		AIPC_RESTORE_INTERRUPTS;
		return FALSE;	
	}

#endif

}
#endif

#endif

#ifdef _AIPC_CPU_

#ifdef CONFIG_RTL8686_IPC_DSP_CONSOLE
extern unsigned short CheckDspIfAllSoftwareReady(void);

#define AIPC_MAJOR_DSP_DEV_CONSOLE      253
#define MAX_DEV_NAME_LEN                16
#define DSP_DEV_CONSOLE_NAME            "dsp_console"

typedef struct aipc_dsp_dev_s {
	unsigned int use_count;
	unsigned int major;
    char name[ MAX_DEV_NAME_LEN + 1 ];
    wait_queue_head_t wq;
    struct file_operations *fop;
	unsigned int f_flags;
#ifdef AIPC_DSP_CONSOLE_USE_TASKLET
	struct tasklet_struct    dsp_console_tasklet;
#endif
#ifdef AIPC_DSP_CONSOLE_USE_WORKQUEUE
	struct workqueue_struct  *dsp_console_workqueue;
	struct work_struct       dsp_console_work;
#endif
    //struct semaphore sem;
	//struct cdev cdev;
} aipc_dsp_dev_t;

static aipc_dsp_dev_t dsp_dev;

/*	File operations */
static ssize_t 
aipc_dsp_console_read (struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	//aipc_dsp_dev_t *dev = filp->private_data;
	int i = 0;
	char ch = 0;
	unsigned int rc = 0;
	char __user *bp = buf;
	
	while ( aipc_dsp_console_read_buf_empty() ) { /* nothing to read */
        if (filp->f_flags & O_NONBLOCK){
            return -EAGAIN;
        }
        schedule();
    }

	for (i=0 ,rc=0 ; i<count ; i++ , rc++) {
		if( !aipc_dsp_console_read_buf_empty() ) {
			aipc_dsp_console_read_buf_dequeue( &ch );
			*bp = ch;
			bp++;
		} else {
			break;	
		}	
	}

	//SDEBUG("count = %u , rc = %u\n" , count , rc);
	
	return rc;
}

ssize_t
aipc_dsp_console_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	//aipc_dsp_dev_t *dev = filp->private_data;
	ssize_t retval = -ENOMEM;
	char * alloc_buf = NULL;
	char * bp = NULL;
	int i = 0;
	
	if( count > DSP_CONSOLE_READ_BUF_SIZE ) {
		SDEBUG("Write length too long\n");
		return  -ERESTARTSYS;
	}
		
	alloc_buf = (char *)kzalloc(DSP_CONSOLE_WRITE_BUF_SIZE , GFP_KERNEL);
	if( !alloc_buf ) {
		SDEBUG("malloc failed\n");
		return -ENOMEM;	
	}

	while ( aipc_dsp_console_read_buf_full() ) { /* write buffer full */
        if (filp->f_flags & O_NONBLOCK) {
            return -EAGAIN;
        }
        schedule();
    }

	copy_from_user( alloc_buf , buf , count );

	bp = alloc_buf;
	
	for (i=0 ; i<count ; i++ , bp++){
		if( !aipc_dsp_console_write_buf_full() ) {
			 aipc_dsp_console_write_buf_enqueue( *bp );
		}
		else {
			break;
		}
	}
	
	retval = count;

	if( !alloc_buf ) {
		kfree( alloc_buf );
	}
	
	//SDEBUG("retval = %d \n" , retval);
	
	return retval;
}

static void
aipc_dsp_console_chk_select(unsigned int *mask)
{
	if( !mask ) {
		SDEBUG("mask address is NULL = %p \n" , mask);
	}

	if( (dsp_dev.f_flags & O_RDWR) == O_RDWR ) {
		if( !aipc_dsp_console_read_buf_empty() ) {
	        *mask |= (POLLIN | POLLRDNORM);   /* readable */
    	}
		if( !aipc_dsp_console_write_buf_full() ) {
			*mask |= (POLLOUT | POLLWRNORM);   /* writable */
		}
	} 
	else if( (dsp_dev.f_flags & O_WRONLY) == O_WRONLY ) {
		if( !aipc_dsp_console_write_buf_full() ) {
			*mask |= (POLLOUT | POLLWRNORM);   /* writable */
		}
	}
	else if( (dsp_dev.f_flags & O_RDONLY) == O_RDONLY ) {
		if( !aipc_dsp_console_read_buf_empty() ) {
	        *mask |= (POLLIN | POLLRDNORM);   /* readable */
    	}
	}else {
		printk("%s(%d) wrong f_flags = 0x%x \n" , __FUNCTION__ , __LINE__ , dsp_dev.f_flags);
	}
}

static unsigned int 
aipc_dsp_console_poll(struct file *filp, poll_table *wait)
{
    aipc_dsp_dev_t *dev = filp->private_data;
    unsigned int mask = 0;
    
	// check before poll_wait(). 
	// thread doesn't have to wait, 
	// if read buf not empty or write queue not full.
#if 0
    aipc_dsp_console_chk_select( &mask );
	
	if( mask ) {
		return mask;
	}
#endif
	
    poll_wait( filp, &dev->wq, wait );
    
    aipc_dsp_console_chk_select( &mask );

    return mask;
}

int
aipc_dsp_console_open(struct inode *inode, struct file *filp)
{
	if( !dsp_dev.use_count ) 
		return -EBUSY;
	
	dsp_dev.use_count--;
	dsp_dev.f_flags = filp->f_flags;
   
	filp->private_data = &dsp_dev;
    
	SDEBUG("use_count = %d , f_flags = %x \n" , dsp_dev.use_count , dsp_dev.f_flags);
    
    return 0;          /* success */
}

int
aipc_dsp_console_release(struct inode *inode, struct file *filp)
{
	dsp_dev.use_count++;

    //SDEBUG("use_count = %d \n" , dsp_dev.use_count);

    return 0;
}

/*  /dev/dsp_console  */
static struct file_operations dsp_console_fops = {
    .read    =   aipc_dsp_console_read,
    .write   =   aipc_dsp_console_write,
    .poll    =   aipc_dsp_console_poll,
    .open    =   aipc_dsp_console_open,
    .release =   aipc_dsp_console_release,
};

static void
_aipc_dsp_console_wakeup( void )
{
	if( (dsp_dev.f_flags & O_RDWR) == O_RDWR ) {
		if( (!aipc_dsp_console_read_buf_empty()) || 
			(!aipc_dsp_console_write_buf_full()) ) {
			wake_up( &dsp_dev.wq );
		}
	}
	else if( (dsp_dev.f_flags & O_WRONLY) == O_WRONLY ) {
		if( (!aipc_dsp_console_write_buf_full()) ) {
			wake_up( &dsp_dev.wq );
		}
	}
	else if( (dsp_dev.f_flags & O_RDONLY) == O_RDONLY ) {
		if( (!aipc_dsp_console_read_buf_empty()) ) {
			wake_up( &dsp_dev.wq );
		}
	}
	else {
		/* other cases. don't care */
	}
}

static void
_aipc_dsp_console_wakeup_interruptible( void )
{
	if( (dsp_dev.f_flags & O_RDWR) == O_RDWR ) {
		if( (!aipc_dsp_console_read_buf_empty()) || 
			(!aipc_dsp_console_write_buf_full()) ) {
			wake_up_interruptible( &dsp_dev.wq );
		}
	}
	else if( (dsp_dev.f_flags & O_WRONLY) == O_WRONLY ) {
		if( (!aipc_dsp_console_write_buf_full()) ) {
			wake_up_interruptible( &dsp_dev.wq );
		}
	}
	else if( (dsp_dev.f_flags & O_RDONLY) == O_RDONLY ) {
		if( (!aipc_dsp_console_read_buf_empty()) ) {
			wake_up_interruptible( &dsp_dev.wq );
		}
	}
	else {
		/* other cases. don't care */
	}
}

#ifdef AIPC_DSP_CONSOLE_USE_TASKLET
void
aipc_dsp_console_wakeup_atomic(unsigned long task_priv)
{
	unsigned short status = 0;
	status = CheckDspIfAllSoftwareReady();
		
	if( status == INIT_OK ) {
		_aipc_dsp_console_wakeup();
	}
}

void
aipc_dsp_console_wakeup_tasklet( void )
{
	if( dsp_dev.dsp_console_tasklet.func != NULL )
		tasklet_schedule( &dsp_dev.dsp_console_tasklet );
}
#endif

#ifdef AIPC_DSP_CONSOLE_USE_WORKQUEUE
void
aipc_dsp_console_wakeup_thread(struct work_struct *work)
{
	_aipc_dsp_console_wakeup();
}

void
aipc_dsp_console_wakeup_workqueue( void )
{
	unsigned short status = 0;
	status = CheckDspIfAllSoftwareReady();
	
	if( status == INIT_OK ) {
		queue_work( dsp_dev.dsp_console_workqueue , &dsp_dev.dsp_console_work );
	}
}
#endif

void
aipc_dsp_console_init_var(void)
{
	// Enable DSP Console Feature
	ADSP_CONSOLE.enable     = 1;
	
	// Init DSP RD QUEUE
	ADSP_CONSOLE_RD.enable  = 1;
	ADSP_CONSOLE_RD.ins     = 0;
	ADSP_CONSOLE_RD.del     = 0;
	ADSP_CONSOLE_RD.eq_try  = 0;
	ADSP_CONSOLE_RD.eq_ok   = 0;
	ADSP_CONSOLE_RD.dq_try  = 0;
	ADSP_CONSOLE_RD.dq_ok   = 0;
	memset( ADSP_CONSOLE_RD.read_buf , 0 , DSP_CONSOLE_READ_BUF_SIZE );
	
	// Init DSP WR QUEUE
	ADSP_CONSOLE_WR.enable  = 1;
	ADSP_CONSOLE_WR.ins     = 0;
	ADSP_CONSOLE_WR.del     = 0;
	ADSP_CONSOLE_WR.eq_try  = 0;
	ADSP_CONSOLE_WR.eq_ok   = 0;
	ADSP_CONSOLE_WR.dq_try  = 0;
	ADSP_CONSOLE_WR.dq_ok   = 0;
	memset( ADSP_CONSOLE_WR.write_buf , 0 , DSP_CONSOLE_WRITE_BUF_SIZE );
	
}

static void
aipc_dsp_console_set_ops(aipc_dsp_dev_t *dev , struct file_operations *fop)
{
	if( dev ) {
		dev->use_count =  DSP_CONSOLE_MAX_USE_COUNT;		/* Max use count is 1 */
		dev->major     =  AIPC_MAJOR_DSP_DEV_CONSOLE;
		dev->fop       =  fop;
		dev->f_flags   =  0;
		strncpy( dev->name , DSP_DEV_CONSOLE_NAME , sizeof(dev->name) );
		init_waitqueue_head( &dev->wq );
#ifdef AIPC_DSP_CONSOLE_USE_TASKLET
		tasklet_init( &dev->dsp_console_tasklet , aipc_dsp_console_wakeup_atomic , (unsigned long)&dev );
#endif
#ifdef AIPC_DSP_CONSOLE_USE_WORKQUEUE
		INIT_WORK( &dev->dsp_console_work , aipc_dsp_console_wakeup_thread );
		dev->dsp_console_workqueue = create_singlethread_workqueue( "dspConsoleD" );
		if( dev->dsp_console_workqueue == NULL ) {
			printk( "%s(%d) create dsp console work queue failed.\n" , __FUNCTION__ , __LINE__ );
		}
#endif
	}
	else {
		printk("dev pointer is NULL\n");
	}
}

int
aipc_dsp_console_init_module(void)
{
	int ret = 0;
	
	aipc_dsp_console_set_ops( &dsp_dev , &dsp_console_fops );
		
	ret = register_chrdev( dsp_dev.major , dsp_dev.name , dsp_dev.fop );
	if (ret<0) {
		printk( "aipc: create dsp console device failed\n" );
		goto fail;
	}
	else{
		printk( "aipc: create dsp console device successed\n" );
	}
		
	return ret;	/* success */
	
fail: 
	return -EFAULT;
}

void
aipc_dsp_console_cleanup_module(void)
{
	unregister_chrdev( dsp_dev.major , dsp_dev.name );
}

#endif
#endif

