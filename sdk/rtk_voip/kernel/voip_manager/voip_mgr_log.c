//#include <linux/config.h>
#include <linux/poll.h>
#include <linux/string.h>

#include "rtk_voip.h"
#include "voip_types.h"
#include "voip_control.h"
#include "voip_init.h"
#include "voip_proc.h"
#include "voip_dev.h"
#include "voip_mgr_log.h"

typedef struct {
	unsigned short cmd;
	unsigned long count;
} eliminate_element;

#define MAX_ELIMINATE_ELEMENTS		8

static struct {
	struct {
		int enable;
		eliminate_element element[ MAX_ELIMINATE_ELEMENTS ];
	} eliminate;
} log_ioctl_var = {
	.eliminate = {
		1,  // enable 
		{
			{ VOIP_MGR_GET_VOIP_EVENT, 0, }, 
			{ VOIP_MGR_FAX_END_DETECT, 0, },
			{ VOIP_MGR_FAX_DIS_DETECT, 0, },
			{ VOIP_MGR_FAX_DIS_TX_DETECT, 0, },
			{ VOIP_MGR_FAX_DIS_RX_DETECT, 0, },
			{ VOIP_MGR_FAX_DCN_DETECT, 0, },
			{ VOIP_MGR_FAX_DCN_TX_DETECT, 0, },
			{ VOIP_MGR_FAX_DCN_RX_DETECT, 0, },
		},
	},
};

void log_ioctl( unsigned short cmd, void *user, unsigned short len, int is_user )
{
	#define LOG_TMP_BUFFER_SIZE		1024
	#define LOG_TMP_USER_SIZE		100
	
	char buffer[ LOG_TMP_BUFFER_SIZE ];
	unsigned char tmpuser[ LOG_TMP_USER_SIZE ];
	int truncate_user = 0;
	int pos = 0;
	int i;
	eliminate_element *pelem;
	
	// check eliminate list 
	if( !log_ioctl_var.eliminate.enable )
		goto label_check_eliminate_list_done;
	
	for( i = 0; i < MAX_ELIMINATE_ELEMENTS; i ++ ) {
	
		pelem = &log_ioctl_var.eliminate.element[ i ];
		
		if( pelem ->cmd == cmd ) {
			pelem ->count ++;
			
			return;		// eliminate this one! 
		}
	}
	
label_check_eliminate_list_done:
	
	// copy from user ioctl 
	if( len > LOG_TMP_USER_SIZE ) {
		len = LOG_TMP_USER_SIZE;
		truncate_user = 1;
	}
	
	if( is_user )
		copy_from_user( tmpuser, user, len );
	else
		memcpy		  ( tmpuser, user, len );	// TODO: pointer is better 
	
	// header 
	pos += sprintf( buffer, "%08X %d %d ", jiffies, cmd, len );
	
	// data 
	for( i = 0; i < len; i ++ ) {
		
		pos += sprintf( buffer + pos, "%02X ", tmpuser[ i ] );
		
		if( pos >= LOG_TMP_BUFFER_SIZE - 10 ) {
			pos += sprintf( buffer + pos, "..." );
			break;
		}
	}
	
	if( truncate_user )
		pos += sprintf( buffer + pos, "..." );
	
	pos += sprintf( buffer + pos, "\n" );
	
	// write to device 
	cl_write_minor( VOIP_DEV_LOG_IOCTL, buffer, pos );
}

// ==================================================================
// PROC for MGR log 
// ==================================================================

#define _M_NAME( name )		name, sizeof( name ) - 1

typedef struct {
	const char *name;
	int name_len;
	int *pval;
} log_ioctl_option_t;

static const log_ioctl_option_t log_ioctl_options[] = {
	{ _M_NAME( "eliminate=" ), &log_ioctl_var.eliminate.enable }
};

#undef _M_NAME

#define LOG_IOCTL_OPTIONS_SIZE		( sizeof( log_ioctl_options ) / sizeof( log_ioctl_options[ 0 ] ) )

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int voip_log_ioctl_read(struct seq_file *f, void *v)
{
	int n = 0;
	int i;
	const log_ioctl_option_t *popt;
	const eliminate_element *pelem;
	
	// options 
	for( i = 0; i < LOG_IOCTL_OPTIONS_SIZE; i ++ ) {
		
		popt = &log_ioctl_options[ i ];
		
		seq_printf( f, "%s%d\n", popt ->name, *( popt ->pval ) );
	}
	
	// eliminate list 
	if( !log_ioctl_var.eliminate.enable )
		goto label_eliminate_list_done;
	
	seq_printf( f, "\neliminate list:\n" );
	
	for( i = 0; i < MAX_ELIMINATE_ELEMENTS; i ++ ) {
		pelem = &log_ioctl_var.eliminate.element[ i ];
		
		if( pelem ->cmd )
			seq_printf( f, "\t%u\t%lu\n", pelem ->cmd, pelem ->count );
	}
	
label_eliminate_list_done:

	return n;

}
#else
static int voip_log_ioctl_read( char *buf, char **start, off_t off, int count, int *eof, void *data )
{
	int n = 0;
	int i;
	const log_ioctl_option_t *popt;
	const eliminate_element *pelem;
	
	if( off ) {	/* In our case, we write out all data at once. */
		*eof = 1;
		return 0;
	}
	
	// options 
	for( i = 0; i < LOG_IOCTL_OPTIONS_SIZE; i ++ ) {
		
		popt = &log_ioctl_options[ i ];
		
		n += sprintf( buf + n, "%s%d\n", popt ->name, *( popt ->pval ) );
	}
	
	// eliminate list 
	if( !log_ioctl_var.eliminate.enable )
		goto label_eliminate_list_done;
	
	n += sprintf( buf + n, "\neliminate list:\n" );
	
	for( i = 0; i < MAX_ELIMINATE_ELEMENTS; i ++ ) {
		pelem = &log_ioctl_var.eliminate.element[ i ];
		
		if( pelem ->cmd )
			n += sprintf( buf + n, "\t%u\t%lu\n", pelem ->cmd, pelem ->count );
	}
	
label_eliminate_list_done:
	
	*eof = 1;
	return n;

}
#endif

static int voip_log_ioctl_write(struct file *file, const char *buffer, 
                               unsigned long count, void *data)
{
	char tmp[ 128 ];
	int i;
	const log_ioctl_option_t *popt;
	
	if( !buffer )
		return count;
	
	if( copy_from_user( tmp, buffer, 128 ) )
		return count;
	
	for( i = 0; i < LOG_IOCTL_OPTIONS_SIZE; i ++ ) {
		
		popt = &log_ioctl_options[ i ];
		
		if( memcmp( tmp, popt ->name, popt ->name_len ) == 0 ) {
			
			*( popt ->pval ) = simple_strtol( tmp + popt ->name_len, NULL, 10 );
			break;
		}
	}
	
	return count;
}

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int log_ioctl_open(struct inode *inode, struct file *file)
{
	return single_open(file, voip_log_ioctl_read, NULL);
}

static ssize_t log_ioctl_write(struct file *file, const char __user * userbuf, size_t count, loff_t * off) {
        return voip_log_ioctl_write(file, userbuf, count, NULL);
}

struct file_operations proc_log_ioctl_fops = {
	.owner	= THIS_MODULE,
	.open	= log_ioctl_open,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.release = single_release,
	.write  = log_ioctl_write,
//read:   voip_log_ioctl_read,
//write:	voip_log_ioctl_write
};
#endif


static int __init voip_proc_log_ioctl_init(void)
{
	struct proc_dir_entry *voip_log_ioctl_proc;

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
	voip_log_ioctl_proc = create_voip_proc_rw_entry( PROC_VOIP_DIR "/log_ioctl", 0, NULL,
						&proc_log_ioctl_fops );
#else
	voip_log_ioctl_proc = create_voip_proc_rw_entry( PROC_VOIP_DIR "/log_ioctl", 0, NULL,
						voip_log_ioctl_read, voip_log_ioctl_write );
#endif
	
	if( voip_log_ioctl_proc == NULL )
	{
		printk( "voip_log_ioctl_proc NULL!! \n" );
		return -1;
	}
	
	return 0;
}

static void __exit voip_proc_log_ioctl_exit( void )
{
	remove_voip_proc_entry( PROC_VOIP_DIR "/log_ioctl", NULL );
}

voip_initcall_proc( voip_proc_log_ioctl_init );
voip_exitcall( voip_proc_log_ioctl_exit );


