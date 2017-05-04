#include "voip_proc.h"

/*
 * Create entries for voip (indirect to be cross platform)
 */
struct proc_dir_entry *create_voip_proc_read_entry(const char *name,
	mode_t mode, struct proc_dir_entry *base,
#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
	struct file_operations *fops,
#else
	read_proc_t *read_proc, 
#endif
	void * data)
{
#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
	return proc_create_data( name, mode, base, fops, data );
#else
	return create_proc_read_entry( name, mode, base, read_proc, data );
#endif
}

struct proc_dir_entry *create_voip_proc_rw_entry(const char *name, mode_t mode,
						struct proc_dir_entry *parent,
#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
						struct file_operations *fops)
#else						
						read_proc_t * read_proc, write_proc_t * write_proc)
#endif						
{
	struct proc_dir_entry *entry;

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
	entry = proc_create(name, mode, parent, fops);
#else
	entry = create_proc_entry( name, mode, parent );
	
	if( entry == NULL )  {
		printk("voip_proc NULL!! \n");
		return NULL;
	}
	
	entry ->read_proc = read_proc;
	entry ->write_proc = write_proc;
#endif	
	return entry;
}

/*
 * Create entries for channel or session 
 */
void create_voip_channel_proc_read_entry(
	const char * name,
#if defined(CONFIG_DEFAULTS_KERNEL_3_10)
	const struct file_operations *fops)
#else
	read_proc_t * read_proc )
#endif	
{
	int i;
	char buf[ 256 ];
		
	for( i = 0; i < PROC_VOIP_CH_NUM; i ++ ) {	// 'voip/ch%u/'
		sprintf( buf, PROC_VOIP_DIR "/" PROC_VOIP_CH_FORMAT "/%s", i, name );
#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)		
		proc_create_data(buf, 0, NULL, fops, (void *)PROC_DATA_FROM_CH(i));
#else
		create_proc_read_entry( buf, 0, NULL, read_proc, ( void * )PROC_DATA_FROM_CH( i ) );
#endif
	}
}

void create_voip_channel_proc_rw_entry(
	const char * name,
#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
	const struct file_operations *fops)
#else
	read_proc_t * read_proc,
	write_proc_t * write_proc )
#endif
{
	struct proc_dir_entry *voip_proc;

	int i;
	char buf[ 256 ];
		
	for( i = 0; i < PROC_VOIP_CH_NUM; i ++ ) {	// 'voip/ch%u/'
		sprintf( buf, PROC_VOIP_DIR "/" PROC_VOIP_CH_FORMAT "/%s", i, name );
		//create_proc_read_entry( buf, 0, NULL, read_proc, ( void * )PROC_DATA_FROM_CH( i ) );
#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)	
		voip_proc = proc_create_data(buf, 0644, NULL, fops, ( void * )PROC_DATA_FROM_CH( i ));
		if ( voip_proc == NULL ) {
			printk("voip_proc NULL!! \n");
			return;
		}
#else
		voip_proc = create_proc_entry( buf, 0644, NULL);
		
		if (voip_proc == NULL) {
			printk("voip_proc NULL!! \n");
			return;
		}
		voip_proc->data = ( void * )PROC_DATA_FROM_CH( i );
		voip_proc->read_proc = read_proc;
		voip_proc->write_proc = write_proc;
#endif
	}
}


void create_voip_session_proc_read_entry(
	const char * name,
#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
	const struct file_operations *fops)
#else
	read_proc_t * read_proc )
#endif	
{
	int i;
	char buf[ 256 ];
		
	for( i = 0; i < PROC_VOIP_SS_NUM; i ++ ) {	// 'voip/ss%u/'
		sprintf( buf, PROC_VOIP_DIR "/" PROC_VOIP_SS_FORMAT "/%s", i, name );
#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
		proc_create_data(buf, 0, NULL, fops, ( void * )PROC_DATA_FROM_SS( i ));
#else
		create_proc_read_entry( buf, 0, NULL, read_proc, ( void * )PROC_DATA_FROM_SS( i ) );
#endif

	}
}

void create_voip_session_proc_rw_entry(
	const char * name,
#if defined(CONFIG_DEFAULTS_KERNEL_3_10)
	const struct file_operations *fops)
#else
	read_proc_t * read_proc,
	write_proc_t * write_proc )
#endif
{
	extern int sess_num;
	struct proc_dir_entry *voip_proc;

	int i;
	char buf[ 256 ];
		
	for( i = 0; i < PROC_VOIP_SS_NUM; i ++ ) {	// 'voip/ss%u/'
		sprintf( buf, PROC_VOIP_DIR "/" PROC_VOIP_SS_FORMAT "/%s", i, name );
#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
		voip_proc = proc_create_data(buf, 0644, NULL, fops, ( void * )PROC_DATA_FROM_SS( i ));
		if ( voip_proc == NULL ) {
			printk("voip_proc NULL!! \n");
			return;
		}		
#else
		voip_proc = create_proc_entry( buf, 0644, NULL);
		
		if (voip_proc == NULL) {
			printk("voip_proc NULL!! \n");
			return;
		}
		voip_proc->data = ( void * )PROC_DATA_FROM_SS( i );
		voip_proc->read_proc = read_proc;
		voip_proc->write_proc = write_proc;
#endif		
	}
}

/*
 * Remove entries from channel or session 
 */
void remove_voip_proc_entry(
	const char * name , struct proc_dir_entry *parent)
{
	remove_proc_entry( name, NULL );
}

void remove_voip_channel_proc_entry(
	const char * name )
{
	int i;
	char buf[ 256 ];
	
	for( i = 0; i < PROC_VOIP_CH_NUM; i ++ ) {	// 'voip/ch%u/'
		sprintf( buf, PROC_VOIP_DIR "/" PROC_VOIP_CH_FORMAT "/%s", i, name );
		remove_voip_proc_entry( buf, NULL );
	}
}

void remove_voip_session_proc_entry(
	const char * name )
{
	int i;
	char buf[ 256 ];
	
	for( i = 0; i < PROC_VOIP_SS_NUM; i ++ ) {	// 'voip/ss%u/'
		sprintf( buf, PROC_VOIP_DIR "/" PROC_VOIP_SS_FORMAT "/%s", i, name );
		remove_voip_proc_entry( buf, NULL );
	}
}

