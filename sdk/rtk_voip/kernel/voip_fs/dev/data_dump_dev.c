/*
 * In our coding, 'DD' is short for data dump. 
 * This kind of device is useful for dump arbitrary data especially 
 * PCM data, and it only needs very little cost. 
 * In kernel part, add a *SINGLE* function to tell this module the 
 * data you want to dump. 
 * In application, just use 'cat' command to read data from device. 
 *
 * Note: 
 *  1. This module doesn't know data structure and all are seen as 
 *     byte stream. 
 *  2. Data in single writing operation is seens as an element, so
 *     we write nothing if free buffer size is not enough. 
 */

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

#define VOIP_DD_DEV_NAME	"DD"

#define NUM_OF_DD_DEV	( VOIP_DD_DEV_END - VOIP_DD_DEV_START + 1 )
#define DD_BUFF_SIZE	( 80 * 2 * 100 )	// 1 sec in narrow band mode 

// ==================================================================
// Data dump fifo 
// ==================================================================

enum {
	DDMODE_U2K 	= 0x0001,	// bit 0
	DDMODE_K2U	= 0x0002,	// bit 1
};

typedef struct {
	// sync. 
	wait_queue_head_t wq;
	// mode 
	unsigned int mode;
	// fifo
	int ri, wi;
	unsigned char data[ DD_BUFF_SIZE ];
} ddinst_t;

static ddinst_t *ddinst[ NUM_OF_DD_DEV ];

#ifdef CONFIG_RTK_VOIP_IPC_ARCH
static int ddinst_ipc_open( int minor, int mode );
static int ddinst_ipc_close( int minor );
#else
#define ddinst_ipc_open( minor, mode )
#define ddinst_ipc_close( minor )
#endif

static int check_and_get_ddinst_idx( unsigned int minor )
{
	unsigned int n = minor - VOIP_DD_DEV_START;
	
	if( n < NUM_OF_DD_DEV )
		return n;
	
	return -1;
}

static inline int get_ddinst_minor( int idx )
{
	if( idx < NUM_OF_DD_DEV )
		return idx + VOIP_DD_DEV_START;
	
	return -1;
}

static int ddinst_write_internal( ddinst_t *pddinst, const char *buff, size_t count, struct file *filp )
{
	// If filp == NULL, write from kernel 
	size_t space, stage1_len, stage2_len;
	const int is_user = ( int )filp;	// if filp != NULL, it is user's command 
	int old_ri;

	// check mode 
	if( is_user && !( pddinst ->mode & DDMODE_U2K ) )
		return -EPERM;
			
	// wait until enough space for atomic writing 
	while( 1 ) {
		// calculate space 
		if( pddinst ->wi >= ( old_ri = pddinst ->ri ) ) {
			space = DD_BUFF_SIZE - pddinst ->wi + pddinst ->ri;
		} else {
			space = pddinst ->ri - pddinst ->wi;
		}
		
		// check whether space is enough 
		if( is_user ) {
			if( space > 1 ) {		// circular buffer must reserve 1 bytes 
				if( space <= count )
					count = space - 1;	// truncate writing data 
				
				break;	// do writing!! 
			}
		} else {
			if( space > count ) { 	// '=' because (wi==ri) indicates empty 
				break;	// do writing!! 
			} else {
				return -1;	// kernel return immediately 
			}
		}
		
		// here is user's process 
		if( filp ->f_flags & O_NONBLOCK )
			return -EAGAIN;
		
		// wait for ri becomes different, I will check again 
		if( wait_event_interruptible( pddinst ->wq, pddinst ->ri != old_ri ) )
			return -ERESTARTSYS;	/* signal: tell the fs layer to handle it */
	}
	
	// write it! (stage 1, wrap case)
	if( pddinst ->wi + count >= DD_BUFF_SIZE ) {
		stage1_len = DD_BUFF_SIZE - pddinst ->wi;
		if( is_user )
			copy_from_user( pddinst ->data + pddinst ->wi, buff, stage1_len );
		else
			memcpy		  ( pddinst ->data + pddinst ->wi, buff, stage1_len );
		
		pddinst ->wi = 0;
		buff += stage1_len;
	} else
		stage1_len = 0;
	
	// copy it! (stage 2)
	stage2_len = count - stage1_len;
	if( is_user )
		copy_from_user( pddinst ->data + pddinst ->wi, buff, stage2_len );
	else
		memcpy		  ( pddinst ->data + pddinst ->wi, buff, stage2_len );
	
	pddinst ->wi += stage2_len;
	
	// wake up reading process 
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP	// DSP may running at DSR 
	if( !is_user )
		wake_up_interruptible( &pddinst ->wq );
#endif
	
	return count;
}

int ddinst_write( unsigned int minor, const void *buff, size_t count )
{
	int idx;
	int ret;
	ddinst_t *pddinst;
	
	// get instance  
	idx = check_and_get_ddinst_idx( minor );
	
	if( idx < 0 || ddinst[ idx ] == NULL )
		return -1;

	pddinst = ddinst[ idx ];
	
	// check mode 
	if( !( pddinst ->mode & DDMODE_K2U ) )
		return -1;
	
	if( ( ret = ddinst_write_internal( pddinst, buff, count, NULL ) ) < 0 )
		printk( "DWF%d ", minor );
	
	return ret;
}

static ssize_t ddinst_read_internal( ddinst_t *pddinst, char *buff, size_t count, struct file *filp )
{
	// If filp == NULL, read from kernel 
	size_t data_len, stage1_len, stage2_len;
	const int is_user = ( int )filp;	// if filp != NULL, it is user's command  
	
	// check mode 
	if( is_user && !( pddinst ->mode & DDMODE_K2U ) )
		return -EPERM;
	
	// kernel don't wait for data 
	if( !is_user && ( pddinst ->ri == pddinst ->wi ) )
		return -EAGAIN;	// no data, try again 
	
	// wait for some data (kernel must not enter this loop)
	while( pddinst ->ri == pddinst ->wi ) {
		
		if( filp ->f_flags & O_NONBLOCK )
			return -EAGAIN;
		
		if( wait_event_interruptible( pddinst ->wq, pddinst ->ri != pddinst ->wi ) )
			return -ERESTARTSYS;	/* signal: tell the fs layer to handle it */
	}
	
	// calculate data length 
	if( pddinst ->ri > pddinst ->wi )
		data_len = DD_BUFF_SIZE - pddinst ->ri + pddinst ->wi;
	else
		data_len = pddinst ->wi - pddinst ->ri;
	
	// kernel should atmoic reading 
	if( !is_user && data_len < count )
		return -EAGAIN;	// not enough data, try again 
	
	// truncate copy size  
	if( data_len > count )
		data_len = count;
	
	// copy it! (stage 1, wrap case)
	if( pddinst ->ri + data_len >= DD_BUFF_SIZE ) {
		stage1_len = DD_BUFF_SIZE - pddinst ->ri;
		if( is_user )
			copy_to_user( buff, pddinst ->data + pddinst ->ri, stage1_len );
		else
			memcpy		( buff, pddinst ->data + pddinst ->ri, stage1_len );
		
		pddinst ->ri = 0;
		buff += stage1_len;
	} else
		stage1_len = 0;
	
	// copy it! (stage 2)
	stage2_len = data_len - stage1_len;
	if( is_user )
		copy_to_user( buff, pddinst ->data + pddinst ->ri, stage2_len );
	else
		memcpy		( buff, pddinst ->data + pddinst ->ri, stage2_len );
	
	pddinst ->ri += stage2_len;

	// wake up writing process 
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP	// DSP may running at DSR 
	if( !is_user )
		wake_up_interruptible( &pddinst ->wq );
#endif
	
	return data_len;
}

int ddinst_read( unsigned int minor, void *buff, size_t count )
{
	int idx;
	int ret;
	ddinst_t *pddinst;
	
	// get instance  
	idx = check_and_get_ddinst_idx( minor );
	
	if( idx < 0 || ddinst[ idx ] == NULL )
		return -1;
	
	pddinst = ddinst[ idx ];
	
	// check mode 
	if( !( pddinst ->mode & DDMODE_U2K ) )
		return -1;
	
	if( ( ret = ddinst_read_internal( pddinst, buff, count, NULL ) ) < 0 )
		;//printk( "DRE%d ", minor );
	
	return ret;
}

int ddinst_rw_auto( unsigned int minor, void *buff, size_t count )
{
	int idx;
	int ret = -1;
	ddinst_t *pddinst;
	
	// get instance  
	idx = check_and_get_ddinst_idx( minor );
	
	if( idx < 0 || ddinst[ idx ] == NULL )
		return -1;
	
	pddinst = ddinst[ idx ];
	
	// check mode 
	if( pddinst ->mode & DDMODE_U2K ) {
		// kernel read 
		if( ( ret = ddinst_read_internal( pddinst, buff, count, NULL ) ) < 0 )
			;
	} else if( pddinst ->mode & DDMODE_K2U ) {
		// kernel write 
		if( ( ret = ddinst_write_internal( pddinst, buff, count, NULL ) ) < 0 )
			printk( "DWf%d ", minor );
	} 
	
	return ret;
}

static int ddinst_open( unsigned int minor, void **pp_pd, unsigned int mode )
{
	int idx;
	ddinst_t *pddinst;
	int ret;
	
	idx = check_and_get_ddinst_idx( minor );
	
	if( idx < 0 )
		return -ENODEV;
	
	// check inst exist? 
	if( ( pddinst = ddinst[ idx ] ) )
		return -EBUSY;
	
	ddinst[ idx ] = ( ddinst_t * )1;		// make it non-zero to avoid race condition 
	
	// allocate space 
	pddinst = kmalloc( sizeof( ddinst_t ), GFP_KERNEL );
	
	if( pddinst == NULL ) {
		ret = -ENOMEM;
		goto label_err;
	}
	
	// init wait queue
	init_waitqueue_head( &pddinst ->wq );
	
	// init fifo 
	pddinst ->ri = pddinst ->wi = 0;
	
	// set mode 
	pddinst ->mode = mode;
	
	// save pointer for reading and writing function 
	ddinst[ idx ] = *pp_pd = pddinst;
	
	// open for IPC 
	ddinst_ipc_open( minor, mode );
	
	return 0;

label_err:
	ddinst[ idx ] = 0;

	return ret;
}

static int ddinst_close( unsigned int minor )
{
	int idx;
	ddinst_t *pddinst;
		
	idx = check_and_get_ddinst_idx( minor );
	
	if( idx < 0 )
		return -ENODEV;
	
	// close for IPC 
	ddinst_ipc_close( minor );
	
	// backup ddinst ptr 
	pddinst = ddinst[ idx ];
		
	// clean pointer 
	ddinst[ idx ] = NULL;
	
	// free memory 
	kfree( pddinst );
	
	return 0;
}

// ==================================================================
// dev f_op 
// ==================================================================

static ssize_t dd_read( struct file *filp, char *buff, size_t count, loff_t *offp )
{
	return ddinst_read_internal( ( ddinst_t * )filp ->private_data, buff, count, filp );
}

static ssize_t dd_write( struct file *filp, const char *buff, size_t count, loff_t *offp )
{
	return ddinst_write_internal( ( ddinst_t * )filp ->private_data, buff, count, filp );
}

static int dd_open( struct inode *node, struct file *filp )
{
	int ret;
	void *p_pd;	// private data 
	unsigned int minor;
	unsigned int mode;
	
	if( ( filp ->f_flags & O_ACCMODE ) == O_WRONLY )
		mode = DDMODE_U2K;		// support write only
	else if( ( filp ->f_flags & O_ACCMODE ) == O_RDONLY )
		mode = DDMODE_K2U;		// support read only
	else
		return -EINVAL;
	
	minor = MINOR( node ->i_rdev );
	
	ret = ddinst_open( minor, &p_pd, mode );
	
	if( ret < 0 )
		return ret;
	
	filp ->private_data = p_pd;
	
	return 0;
}

static int dd_close( struct inode *node, struct file *filp )
{
	int ret;
	unsigned int minor;
	
	minor = MINOR( node ->i_rdev );
	
	ret = ddinst_close( minor );
	
	if( ret < 0 )
		return ret;
	
	return 0;
}

static struct file_operations dd_fops = {
	read:		dd_read,
	write:		dd_write,
	open:		dd_open,
	release:	dd_close,
};

static int __init voip_dd_dev_init( void )
{
	int ret;
	
	ret = register_voip_chrdev( VOIP_DD_DEV_START, NUM_OF_DD_DEV, VOIP_DD_DEV_NAME, &dd_fops );
	
	if( ret < 0 )
		printk( "register data dump dev fail\n" );
	
	return 0;
}

static void __exit voip_dd_dev_exit( void )
{
	unregister_voip_chrdev( VOIP_DD_DEV_START, NUM_OF_DD_DEV );
}

voip_initcall( voip_dd_dev_init );
voip_exitcall( voip_dd_dev_exit );

// ==================================================================
// IPC 
// ==================================================================

#ifdef CONFIG_RTK_VOIP_IPC_ARCH

#include "ipc_arch_tx.h"

static struct {
	uint32 open;	// open(1) or close(0) 
	uint32 mode;	// read(1) or write(0) 
	uint32 pause;	// pause(1) or resume(0)
	uint16 offset[ NUM_OF_DD_DEV ];	// offset
} ddd_ipc;

CT_ASSERT( sizeof( ddd_ipc.open ) * 8 >= NUM_OF_DD_DEV );
CT_ASSERT( sizeof( ddd_ipc.mode ) * 8 >= NUM_OF_DD_DEV );
CT_ASSERT( sizeof( ddd_ipc.pause ) * 8 >= NUM_OF_DD_DEV );

static inline void printk_off( const char *, ... );
#define dddipc_debug		printk	//_off

#define IPC_DDD_EXCHANGE_DATA_SIZE	640
#define AGENT_BURST_READ_COUNT		1		// burst read quantity is size * count 

static int ddinst_ipc_open( int minor, int mode )
{
	int idx;
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	ddd_content_t ddd_content;
#endif
	uint16 category;
	
	// exclude 'IPC' 
	if( minor == VOIP_DEV_IPC )
		return -1;
	
	if( ( idx = check_and_get_ddinst_idx( minor ) ) < 0 )
		return -1;
	
	if( ddd_ipc.open & ( 1 << idx ) )	// already opened 
		return -1;
	
	// process 'open' 
	if( mode == DDMODE_U2K ) {			// write 
		ddd_ipc.mode &= ~( 1 << idx );
		category = IPC_DDD_H2D_OPEN_WRMODE;
	} else if( mode == DDMODE_K2U ) {	// read 
		ddd_ipc.mode |= ( 1 << idx );
		category = IPC_DDD_H2D_OPEN_RDMODE;
	} else
		return -1;
	
	ddd_ipc.open |= ( 1 << idx );
	ddd_ipc.pause &= ~( 1 << idx );
	ddd_ipc.offset[ idx ] = 0;
	
	// host fw to dsp 
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	ddd_content.minor = minor;
	ddd_content.offset = 0;
	
	ipcSendDddH2dPacket( category, &ddd_content, SIZEOF_DDD_BASIC_CONT );
#endif
	
	return 0;
}

static int ddinst_ipc_close( int minor )
{
	int idx;
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	ddd_content_t ddd_content;
#endif
	
	// Exclude 'IPC' 
	if( minor == VOIP_DEV_IPC )
		return -1;
	
	if( ( idx = check_and_get_ddinst_idx( minor ) ) < 0 )
		return -1;
	
	if( !( ddd_ipc.open & ( 1 << idx ) ) )	// not opened 
		return -1;
	
	// process 'close' 
	ddd_ipc.open &= ~( 1 << idx );
	
	// host fw to dsp 
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	ddd_content.minor = minor;
	ddd_content.offset = ddd_ipc.offset[ idx ];
	
	ipcSendDddH2dPacket( IPC_DDD_H2D_CLOSE, &ddd_content, SIZEOF_DDD_BASIC_CONT );
#endif
	
	return 0;
}

static inline void _ipc_ddd_parser( const ipc_ctrl_pkt_t *ipc_ctrl, uint32 mode_lsb )
{
	const ddd_content_t * const p_ddd_content =
				( const ddd_content_t * )ipc_ctrl ->content;
	const int minor = p_ddd_content ->minor;
	const uint16 offset = p_ddd_content ->offset;
	const int datalen = ipc_ctrl ->cont_len - SIZEOF_DDD_BASIC_CONT;
	const uint16 category = ipc_ctrl ->category;
	
	int idx;
	int ret;
	
	void *p_pd;
	unsigned int mode;
	
	// get 'idx'
	if( ( idx = check_and_get_ddinst_idx( minor ) ) < 0 )
		return;
	
	// do action 
	switch( category ) {
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	case IPC_DDD_D2H_READ:
#endif
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
	case IPC_DDD_H2D_WRITE:
#endif
		// check opened? 
		if( !( ddd_ipc.open & ( 1 << idx ) ) )
			return;
		
		// read/write mode check 
		if( ( ( ddd_ipc.mode >> idx ) & 0x01 ) != mode_lsb )
			return;
		
		// check 'offset' 
		if( offset != ddd_ipc.offset[ idx ] ) {
			PRINT_R( "%s %d: invalid offset %04X (exp: %04X) @%d\n", __FUNCTION__, __LINE__, 
							offset, ddd_ipc.offset[ idx ], minor );
		}
		
		ddd_ipc.offset[ idx ] = offset + datalen;
		
		// write it 
		ret = ddinst_write_internal( ddinst[ idx ], p_ddd_content ->data, datalen, NULL );
		
		if( ret < 0 ) {
			PRINT_R( "%s %d: write DD offset %04X ret=%d\n", __FUNCTION__, __LINE__, 
							offset, ret );
		}
		
		break;
	
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
	case IPC_DDD_H2D_OPEN_RDMODE:
	case IPC_DDD_H2D_OPEN_WRMODE:
		
		if( category == IPC_DDD_H2D_OPEN_RDMODE )
			mode = DDMODE_K2U;		// support read only
		else
			mode = DDMODE_U2K;		// support write only

		ret = ddinst_open( minor, &p_pd, mode );
		break;
		
	case IPC_DDD_H2D_CLOSE:
		ret = ddinst_close( minor );
		break;
#endif

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	case IPC_DDD_D2H_WRITE_PAUSE:
		ddd_ipc.pause |= ( 1 << idx );
		break;
		
	case IPC_DDD_D2H_WRITE_RESUME:
		ddd_ipc.pause &= ~( 1 << idx );
		break;
#endif
		
	default:
		PRINT_R( "%s %d: Unhandled category=%d\n", __FUNCTION__, __LINE__, category );
		break;
	}
}

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
void ipc_ddd_d2h_parser( const ipc_ctrl_pkt_t *ipc_ctrl )
{
	_ipc_ddd_parser( ipc_ctrl, 1 /* read */ );
}
#endif

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
void ipc_ddd_h2d_parser( const ipc_ctrl_pkt_t *ipc_ctrl )
{
	_ipc_ddd_parser( ipc_ctrl, 0 /* write */ );
}
#endif

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
static inline void check_if_announce_pause_resume( 
						const ddinst_t *pddinst, int idx, int minor, 
						uint32 mode_lsb ) 
{
	size_t data_len;
	ddd_content_t ddd_content;
	uint16 category;
	
	// not write --> ignore 
	if( ( mode_lsb & 0x01 ) != 0 )	
		return;
	
	// calculate data length 
	if( pddinst ->ri > pddinst ->wi )
		data_len = DD_BUFF_SIZE - pddinst ->ri + pddinst ->wi;
	else
		data_len = pddinst ->wi - pddinst ->ri;
	
	// check if announce 
	if( ddd_ipc.pause & ( 1 << idx ) ) {	// now is pause, resume? 
		
		if( data_len <= DD_BUFF_SIZE / 3 )
			;
		else
			return;
		
		// do resume 
		ddd_ipc.pause &= ~( 1 << idx );
		
		category = IPC_DDD_D2H_WRITE_RESUME;
	
	} else {								// now is resume, pause? 
		
		if( data_len > DD_BUFF_SIZE - IPC_DDD_EXCHANGE_DATA_SIZE * 
										AGENT_BURST_READ_COUNT * 4 - 1 )
		{
			// product of 2 and burst read count reserved for sending data on the way. 
			// -1 for safe circular buffer. 
		} else
			return;
			
		// do pause 
		ddd_ipc.pause |= ( 1 << idx );
		
		category = IPC_DDD_D2H_WRITE_PAUSE;
	}
	
	// send to Host 
	ddd_content.minor = minor;
	ddd_content.offset = ddd_ipc.offset[ idx ];
	
	ipcSendDddD2hPacket( category, &ddd_content, SIZEOF_DDD_BASIC_CONT );
}
#endif

static void voip_dd_ipc_tunnel_agent( void *priv )
{
	uint32 open, mode, pause;
	int idx;
	int minor;
	ddinst_t *pddinst;
	int burst;
	
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	const uint32 mode_lsb = 0;	// process write only 
#else
	const uint32 mode_lsb = 1;	// process read only 
#endif
	
	union {
		ddd_content_t t;
		uint8 raw[ IPC_DDD_EXCHANGE_DATA_SIZE + SIZEOF_DDD_BASIC_CONT ];
	} ddd_content;
	ssize_t cont_size;

	for( open = ddd_ipc.open, mode = ddd_ipc.mode, pause = ddd_ipc.pause, idx = 0; 
		 open; 
		 open >>= 1, mode >>= 1, pause >>= 1, idx ++ )
	{
		// LSB indicate 'idx'-st DD 
		
		if( ( open & 0x01 ) == 0 )	// this one is not opened 
			continue;
		
		pddinst = ddinst[ idx ];
		
		// get minor from idx 
		if( ( minor = get_ddinst_minor( idx ) ) < 0 ) {
			PRINT_R( "%s: %d - minor < 0 (idx=%d)!!??\n", __FUNCTION__, __LINE__, idx );
			continue;
		}
		
		// DSP side check if announce pause/resume to Host 
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
		check_if_announce_pause_resume( pddinst, idx, minor, mode );
#endif
		
		// check read / write 
		if( ( mode & 0x01 ) != mode_lsb )
			continue;

		// Host side ignore due to pause 
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
		if( ( pause & 0x01 ) )
			continue;
#endif
		
		// try to read specified minor 
		for( burst = AGENT_BURST_READ_COUNT; burst; burst -- ) 
		{
			if( ( cont_size = ddinst_read_internal( pddinst, ddd_content.t.data, 
								IPC_DDD_EXCHANGE_DATA_SIZE, NULL ) ) <= 0 )
			{
				break;
			}
								
			// fill content & send to other side  
			ddd_content.t.minor = minor;
			ddd_content.t.offset = ddd_ipc.offset[ idx ];	ddd_ipc.offset[ idx ] += cont_size;
		
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
			ipcSendDddH2dPacket( IPC_DDD_H2D_WRITE, &ddd_content.t, 
									SIZEOF_DDD_BASIC_CONT + cont_size );
#else
			ipcSendDddD2hPacket( IPC_DDD_D2H_READ, &ddd_content.t, 
									SIZEOF_DDD_BASIC_CONT + cont_size );
#endif
		} // read all data 
		
	} // for each minor  
}

static int __init voip_dd_ipc_init( void )
{
	ddd_ipc.open = 0;
	
	register_timer( voip_dd_ipc_tunnel_agent, NULL, 10 );
	
	return 0;
}

static void __exit voip_dd_ipc_exit( void )
{
	
}

voip_initcall( voip_dd_ipc_init );
voip_exitcall( voip_dd_ipc_exit );

#endif // CONFIG_RTK_VOIP_IPC_ARCH

