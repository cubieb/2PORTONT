#include <asm/uaccess.h>    /* copy_*_user */
#include "rtk_voip.h"
#include "con_register.h"
#include "voip_init.h"
#include "voip_proc.h"

#ifndef CONFIG_RTK_VOIP_QOS
#error "Build this file only if CONFIG_RTK_VOIP_QOS defined!!"
#endif

extern voip_con_t *get_voip_cons_ptr( void );

int voip_tx_shortcut_enable = 0;  // disable tx by default. use netfilter for default setting.
int voip_rx_shortcut_enable = 1;  // enable rx by default. 

int chk_pkt_lmt = 100;         // default value 100.
int ( *check_voip_channel_loading )( void ) = NULL;
unsigned int voip_cch_enable[ CON_CH_NUM ] = {0};

static int Check_Any_ChanEnabled(void)
{
	// call by networking driver 
	int i;
	voip_con_t *p_con = get_voip_cons_ptr();

	for( i = 0; i < CON_CH_NUM; i ++, p_con ++ ){
		if( voip_cch_enable[i] )    
			return 1;
	}
	
	return 0;
}

static __init int init_check_voip_channel_loading( void )
{
	check_voip_channel_loading = Check_Any_ChanEnabled;
	
	return 0;
}

voip_initcall( init_check_voip_channel_loading );

// --------------------------------------------------------
// proc 
// --------------------------------------------------------
#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int 
voip_check_channel_loading_read_proc(struct seq_file *f, void *v)
{
	int n = 0 , i = 0;
	
	voip_con_t *p_con = get_voip_cons_ptr();

	if( check_voip_channel_loading ){
		seq_printf( f, "voip enable = %d\n" , check_voip_channel_loading() );
	}
	
	for( i = 0; i < CON_CH_NUM; i ++, p_con ++ ){
		#if 0	
			n += sprintf( buf + n , "channel_enable[%d] = %u\n" , i , p_con ->enabled );
		#else
			seq_printf( f, "voip_cch_enable[%d] = %u\n" , i , voip_cch_enable[i] );
		#endif
	}	
	
	seq_printf( f, "callback=%p\n", check_voip_channel_loading );

	return n;
}

#else
static int 
voip_check_channel_loading_read_proc( char *buf, char **start, off_t off, int count, int *eof, void *data )
{
	int n = 0 , i = 0;
	
	voip_con_t *p_con = get_voip_cons_ptr();

	if( off ) {	/* In our case, we write out all data at once. */
		*eof = 1;
		return 0;
	}
	
	if( check_voip_channel_loading ){
		n += sprintf( buf + n, "voip enable = %d\n" , check_voip_channel_loading() );
	}
	
	for( i = 0; i < CON_CH_NUM; i ++, p_con ++ ){
		#if 0	
			n += sprintf( buf + n , "channel_enable[%d] = %u\n" , i , p_con ->enabled );
		#else
			n += sprintf( buf + n , "voip_cch_enable[%d] = %u\n" , i , voip_cch_enable[i] );
		#endif
	}	
	
	n += sprintf( buf + n, "callback=%p\n", check_voip_channel_loading );
	
	*eof = 1;
	return n;
}
#endif

static int 
voip_check_packet_limit_write_proc( struct file *file, const char *buffer, unsigned long count, void *data )
{
	char tmpbuf[100] = {0};
	int flag = 0;

	if (buffer && !copy_from_user(tmpbuf, buffer, count)) {

		sscanf(tmpbuf, "%d", &flag);

		chk_pkt_lmt = flag;
			
		printk( "Chekc packet limit chk_pkt_lmt = %d\n" , chk_pkt_lmt );
	}

	return count;
}

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int 
voip_check_packet_limit_read_proc(struct seq_file *f, void *v)
{
	int n = 0;

	seq_printf( f, "chk_pkt_lmt = %d\n", chk_pkt_lmt );

	return n;
}
#else
static int 
voip_check_packet_limit_read_proc( char *buf, char **start, off_t off, int count, int *eof, void *data )
{
	int n = 0;

	if( off ) {	/* In our case, we write out all data at once. */
		*eof = 1;
		return 0;
	}
	
	n += sprintf( buf + n, "chk_pkt_lmt = %d\n", chk_pkt_lmt );
	
	*eof = 1;
	return n;
}
#endif

static int 
voip_shortcut_enable_write_proc( struct file *file, const char *buffer, unsigned long count, void *data )
{
	char tmpbuf[100] = {0};
	int flag = 0;

	if (buffer && !copy_from_user(tmpbuf, buffer, count)) {

		sscanf(tmpbuf, "%d", &flag);
		
		if( (flag == 0) || (flag == 1) ){
			voip_rx_shortcut_enable = flag;
			voip_tx_shortcut_enable=0;
			printk( "voip_shortcut_enable(RX) = %d\n" , voip_rx_shortcut_enable );
			printk( "voip_shortcut_enable(TX) = %d\n" , voip_tx_shortcut_enable );
		}else if(flag == 2){
			voip_rx_shortcut_enable = 1;
			voip_tx_shortcut_enable =1;
			printk( "voip_shortcut_enable(RX) = %d\n" , voip_rx_shortcut_enable );
			printk( "voip_shortcut_enable(TX) = %d\n" , voip_tx_shortcut_enable );

		}else {
			printk( "Please assign appropriate value , 0,1 or 2\n" );
		}		
	}

	return count;
}

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int 
voip_shortcut_enable_read_proc(struct seq_file *f, void *v)
{
	int n = 0;

	seq_printf( f, "voip_shortcut_RX_enable = %d\n", voip_rx_shortcut_enable );
	seq_printf( f, "voip_shortcut_TX_enable = %d\n", voip_tx_shortcut_enable );

	return n;
}
#else
static int 
voip_shortcut_enable_read_proc( char *buf, char **start, off_t off, int count, int *eof, void *data )
{
	int n = 0;

	if( off ) {	/* In our case, we write out all data at once. */
		*eof = 1;
		return 0;
	}
	
	n += sprintf( buf + n, "voip_shortcut_RX_enable = %d\n", voip_rx_shortcut_enable );
	n += sprintf( buf + n, "voip_shortcut_TX_enable = %d\n", voip_tx_shortcut_enable );
	*eof = 1;
	return n;
}
#endif

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int load_chk_open(struct inode *inode, struct file *file)
{
	return single_open(file, voip_check_channel_loading_read_proc, NULL);
}

struct file_operations proc_load_chk_fops = {
	.owner	= THIS_MODULE,
	.open	= load_chk_open,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.release = single_release,
};

static int chk_pkt_lmt_open(struct inode *inode, struct file *file)
{
	return single_open(file, voip_check_packet_limit_read_proc, NULL);
}

struct file_operations proc_chk_pkt_lmt_fops = {
	.owner	= THIS_MODULE,
	.open	= chk_pkt_lmt_open,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.release = single_release,
	.write  = voip_check_packet_limit_write_proc,	
};

static int shortcut_enable_open(struct inode *inode, struct file *file)
{
	return single_open(file, voip_shortcut_enable_read_proc, NULL);
}

struct file_operations proc_shortcut_enable_fops = {
	.owner	= THIS_MODULE,
	.open	= shortcut_enable_open,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.release = single_release,
	.write  = voip_shortcut_enable_write_proc,	
};


#endif

int __init voip_check_channel_loading_proc_init( void )
{
	struct proc_dir_entry *proc_aipc_dev_dir ;
	struct proc_dir_entry *entry;

	proc_aipc_dev_dir = proc_mkdir( PROC_VOIP_DIR "/" "load" , NULL );

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
	entry = proc_create(PROC_VOIP_DIR "/" "load" "/" "load_chk", 0644, NULL, &proc_load_chk_fops);
	if (entry == NULL) {
		printk("create proc voip/load/load_chk failed!\n");
		return 0;
	}	

	entry = proc_create(PROC_VOIP_DIR "/" "load" "/" "chk_pkt_lmt", 0644, NULL, &proc_chk_pkt_lmt_fops);
	if (entry == NULL) {
		printk("create proc voip/load/chk_pkt_lmt failed!\n");
		return 0;
	}			
#else

	if((entry = create_proc_entry( "load_chk" , 0644 , proc_aipc_dev_dir)) == NULL) {
		printk("create proc voip/load/load_chk failed!\n");
		return 0;
	}
	entry->write_proc = NULL;
	entry->read_proc  = voip_check_channel_loading_read_proc;

	if((entry = create_proc_entry( "chk_pkt_lmt" , 0644 , proc_aipc_dev_dir)) == NULL) {
		printk("create proc voip/load/chk_pkt_lmt failed!\n");
		return 0;
	}
	entry->write_proc = voip_check_packet_limit_write_proc;
	entry->read_proc  = voip_check_packet_limit_read_proc;
#endif

	proc_aipc_dev_dir = proc_mkdir( PROC_VOIP_DIR "/" "shortcut" , NULL );

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
	entry = proc_create(PROC_VOIP_DIR "/" "shortcut" "/" "enable", 0644, NULL, &proc_shortcut_enable_fops);
	if (entry == NULL) {
		printk("create proc voip/shortcut/enable failed!\n");
		return 0;
	}
#else
	if((entry = create_proc_entry( "enable" , 0644 , proc_aipc_dev_dir)) == NULL) {
		printk("create proc voip/shortcut/enable failed!\n");
		return 0;
	}
	entry->write_proc = voip_shortcut_enable_write_proc;
	entry->read_proc  = voip_shortcut_enable_read_proc;
#endif	

	return 0;
}

void __exit voip_check_channel_loading_proc_exit( void )
{
	remove_proc_entry( PROC_VOIP_DIR "/load_chk", NULL );
}

voip_initcall_proc( voip_check_channel_loading_proc_init );
voip_exitcall( voip_check_channel_loading_proc_exit );

