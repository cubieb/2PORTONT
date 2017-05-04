#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/sched.h>

#include "rtk_voip.h"
#include "voip_init.h"
#include "voip_dev.h"
#include "voip_ipc.h"
#include "voip_timer.h"
#include "voip_debug.h"

#define VOIP_CL_DEV_NAME	"CL"

#define NUM_OF_CL_DEV	( VOIP_CL_DEV_END - VOIP_CL_DEV_START + 1 )
#define CL_BUFF_SIZE	CONFIG_RTK_VOIP_LOG_IOCTL_SIZE	// TODO: CL use same size 

// ==================================================================

typedef struct {
	// busy
	int busy;	// is opened? 
	// lock
	int lock;	// read / write lock 
	// sync. 
	wait_queue_head_t wq_w, wq_r;
	// mode 
	//unsigned int mode;
	// fifo
	int ri, wi;
	int size;
	unsigned char data[ CL_BUFF_SIZE ];
} clinst_t;

static clinst_t clinst[ NUM_OF_CL_DEV ];

static int check_and_get_clinst_idx( unsigned int minor )
{
	unsigned int n = minor - VOIP_CL_DEV_START;
	
	if( n < NUM_OF_CL_DEV )
		return n;
	
	return -1;
}

static inline int get_clinst_minor( int idx )
{
	if( idx < NUM_OF_CL_DEV )
		return idx + VOIP_CL_DEV_START;
	
	return -1;
}

static inline void cl_EnterCriticalSection( clinst_t *pclinst )
{
	unsigned long flags;
	
	int lock;
	int count = 3000;	// set a threshold to show a message 
	
	do {
		
		save_flags(flags); cli();
		
		if( !( lock = pclinst ->lock ) )
			pclinst ->lock = 1;
		
		restore_flags(flags);
		
		if( !lock )
			break;
		
		// schedule out to wait for lock 
		schedule();
		
	} while( -- count );
	
	if( count == 0 )
		printk( "circular log wait long time!\n" );
	
}

static inline void cl_LeaveCriticalSection( clinst_t *pclinst )
{
	unsigned long flags;
	
	save_flags(flags); cli();
		
	if( !pclinst ->lock )
		printk( "circular log should be locked!\n" );
	
	pclinst ->lock = 0;
	
	restore_flags(flags);
}

static ssize_t cl_read_inst( clinst_t /*volatile*/ *pclinst, 
							char *buff, size_t count, struct file *filp )
{
	// caller must be user process!! from fop.read() 
	
	// If filp == NULL, read from kernel (but user process)
	size_t data_len, stage1_len, stage2_len;
	//const int is_user = ( int )filp;	// if filp != NULL, it is user's command  
	
	// wait for some data (kernel must not enter this loop)
	while( pclinst ->ri == pclinst ->wi ) {
		
		if( filp ->f_flags & O_NONBLOCK )
			return -EAGAIN;
		
		if( wait_event_interruptible( pclinst ->wq_r, pclinst ->ri != pclinst ->wi ) )
			return -ERESTARTSYS;	/* signal: tell the fs layer to handle it */
	}
	
	///////////////////////////////////////////////////////////////////////////
	// enter critical section 
	cl_EnterCriticalSection( pclinst );
	
	// calculate data length 
	if( pclinst ->ri > pclinst ->wi )
		data_len = pclinst ->size - pclinst ->ri + pclinst ->wi;
	else
		data_len = pclinst ->wi - pclinst ->ri;
	
	// truncate copy size  
	if( data_len > count )
		data_len = count;
	
	// copy it! (stage 1, wrap case)
	if( pclinst ->ri + data_len >= pclinst ->size ) {
		stage1_len = pclinst ->size - pclinst ->ri;
		copy_to_user( buff, pclinst ->data + pclinst ->ri, stage1_len );
		
		pclinst ->ri = 0;
		buff += stage1_len;
	} else
		stage1_len = 0;
	
	// copy it! (stage 2)
	stage2_len = data_len - stage1_len;
	copy_to_user( buff, pclinst ->data + pclinst ->ri, stage2_len );
	
	pclinst ->ri += stage2_len;
	
	// done 
	
	cl_LeaveCriticalSection( pclinst );
	// leave critical section 
	///////////////////////////////////////////////////////////////////////////
	
	return data_len;
}

static int cl_write_inst( clinst_t *pclinst, const void *buff, size_t count )
{
	// caller must be user process!! from function call 
	
	// If filp == NULL, write from kernel 
	size_t space, stage1_len, stage2_len;
	//const int is_user = ( int )filp;	// if filp != NULL, it is user's command 
	size_t skip_ri;
	
	{
		// calculate space 
		if( pclinst ->wi >= pclinst ->ri ) {
			space = pclinst ->size - pclinst ->wi + pclinst ->ri;
		} else {
			space = pclinst ->ri - pclinst ->wi;
		}
		
		//printk( "space=%d, count=%d wi=%d ri=%d size=%d\n", space, count, 
		//				pclinst ->wi, pclinst ->ri, pclinst ->size );
		
		// check whether space is enough 
		if( space > count ) { 	// '=' because (wi==ri) indicates empty 
			;	// do writing!! 
		} else {
			// space is not enough --> move ri 
			cl_EnterCriticalSection( pclinst );
			
			skip_ri = count - space + 1;	// +1 for circular buffer 
			
			pclinst ->ri += skip_ri;
			
			if( pclinst ->ri >= pclinst ->size )
				pclinst ->ri -= pclinst ->size;
			
			cl_LeaveCriticalSection( pclinst );
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// enter critical section 
	cl_EnterCriticalSection( pclinst );
	
	// write it! (stage 1, wrap case)
	if( pclinst ->wi + count >= pclinst ->size ) {
		stage1_len = pclinst ->size - pclinst ->wi;
		memcpy( pclinst ->data + pclinst ->wi, buff, stage1_len );
		
		pclinst ->wi = 0;
		buff += stage1_len;
	} else
		stage1_len = 0;
	
	// copy it! (stage 2)
	stage2_len = count - stage1_len;
	memcpy( pclinst ->data + pclinst ->wi, buff, stage2_len );
	
	pclinst ->wi += stage2_len;

	// done 
	
	cl_LeaveCriticalSection( pclinst );
	// leave critical section 
	///////////////////////////////////////////////////////////////////////////
	
	// wake up reading process 
	wake_up_interruptible( &pclinst ->wq_r );
	
	return count;
}

static int cl_open_inst( clinst_t *pclinst, void **pp_pd )
{
	// check inst busy? 
	if( pclinst ->busy )
		return -EBUSY;
	
	// set it busy 
	pclinst ->busy = 1;
	
	// save pointer for reading and writing function 
	*pp_pd = pclinst;
	
	return 0;
}

static int cl_close_inst( clinst_t *pclinst )
{
	// check inst busy(open)? 
	if( pclinst ->busy ) {
		pclinst ->busy = 0;
		return 0;
	}
	
	return -EINVAL;
}

static int cl_init_inst( clinst_t *pclinst )
{
	// set it unlock 
	pclinst ->lock = 0;
	
	// init wait queue
	init_waitqueue_head( &pclinst ->wq_r );
	
	// init fifo 
	pclinst ->ri = pclinst ->wi = 0;
	pclinst ->size = CL_BUFF_SIZE;
	
	return 0;
}

// ==================================================================
// dev f_op - minor 
// ==================================================================

static clinst_t *get_clinst_ptr( unsigned int minor )
{
	int idx;
	
	if( ( idx = check_and_get_clinst_idx( minor ) ) < 0 )
		return NULL;
	
	return &clinst[ idx ];
}

static int cl_open_minor( unsigned int minor, void **pp_pd )
{
	clinst_t *pclinst;
	
	if( ( pclinst = get_clinst_ptr( minor ) ) == NULL )
		return -ENODEV;
		
	return cl_open_inst( pclinst, pp_pd );
}

static int cl_close_minor( unsigned int minor )
{
	clinst_t *pclinst;
	
	if( ( pclinst = get_clinst_ptr( minor ) ) == NULL )
		return -ENODEV;
	
	return cl_close_inst( pclinst );
}

int cl_write_minor( unsigned int minor, const void *buff, size_t count )
{
	clinst_t *pclinst;
	
	if( ( pclinst = get_clinst_ptr( minor ) ) == NULL )
		return -ENODEV;
	
	return cl_write_inst( pclinst, buff, count );
}

static int cl_init_minor( unsigned int minor )
{
	clinst_t *pclinst;
	
	if( ( pclinst = get_clinst_ptr( minor ) ) == NULL )
		return -ENODEV;
	
	return cl_init_inst( pclinst );
}

// ==================================================================
// dev f_op 
// ==================================================================

static ssize_t cl_read( struct file *filp, char *buff, size_t count, loff_t *offp )
{
	return cl_read_inst( ( clinst_t * )filp ->private_data, buff, count, filp );
}

static int cl_open( struct inode *node, struct file *filp )
{
	int ret;
	void *p_pd = NULL;	// private data 
	unsigned int minor;
	
	if( ( filp ->f_flags & O_ACCMODE ) == O_RDONLY )
		;		// support read only
	else
		return -EINVAL;
	
	minor = MINOR( node ->i_rdev );
	
	ret = cl_open_minor( minor, &p_pd );
	
	if( ret < 0 )
		return ret;
	
	filp ->private_data = p_pd;
	
	return 0;
}

static int cl_close( struct inode *node, struct file *filp )
{
	int ret;
	unsigned int minor;
	
	minor = MINOR( node ->i_rdev );
	
	ret = cl_close_minor( minor );
	
	if( ret < 0 )
		return ret;
	
	return 0;
}

static struct file_operations cl_fops = {
	read:		cl_read,
	//write:		cl_write,
	open:		cl_open,
	release:	cl_close,
};

static int __init voip_cl_dev_init( void )
{
	int ret;
	int i;
	
	for( i = VOIP_CL_DEV_START; i <= VOIP_CL_DEV_END; i ++ )
		cl_init_minor( i );
	
	ret = register_voip_chrdev( VOIP_CL_DEV_START, NUM_OF_CL_DEV, VOIP_CL_DEV_NAME, &cl_fops );
	
	if( ret < 0 )
		printk( "register circular log dev fail\n" );
	
	return 0;
}

static void __exit voip_cl_dev_exit( void )
{
	unregister_voip_chrdev( VOIP_CL_DEV_START, NUM_OF_CL_DEV );
}

voip_initcall( voip_cl_dev_init );
voip_exitcall( voip_cl_dev_exit );

