#include "voip_types.h"
#include "voip_timer.h"
#include "voip_init.h"
#include "voip_proc.h"
#include "voip_debug.h"
#include "os_timer_core.h"

timetick_t timetick = TIMETICK_SEED;

#define TIMER_ENTRIES_NUM		8	// up to 32 

typedef struct {
	timetick_t next_timetick;	// next announce timetick 
	uint32 period;			// timer period (ms)
	uint32 f_periodic:1;	// periodic timer ?
	fn_timer_t fn_timer;	// timer function 
	void *priv;				// private data for timer function 
} timer_entry_t;

static struct {
	timer_entry_t entries[ TIMER_ENTRIES_NUM ];
	uint32 mask;
} timer;

CT_ASSERT( ( sizeof( timer.mask ) * 8 ) >= TIMER_ENTRIES_NUM );

static void register_timer_core( fn_timer_t fn_timer, void *priv, 
									long period, int idx )
{
	timer_entry_t * const p_entry = &timer.entries[ idx ];
	
	// negative period means one shoot timer 
	p_entry ->f_periodic = ( period > 0 ? 1 : 0 );
	period = ( period < 0 ? period * ( -1 ) : period );
	
	p_entry ->next_timetick = timetick + period;	// save current + period 
	p_entry ->period = period;
	p_entry ->fn_timer = fn_timer;
	p_entry ->priv = priv;
	
	timer.mask |= ( 1 << idx );
}

static int find_unused_timer_entry_index( fn_timer_t fn_timer )
{
	int i;
	const timer_entry_t * p_entry;
	
	// check whether redundant ? 
	if( fn_timer != NULL ) {
		
		for( i = 0; i < TIMER_ENTRIES_NUM; i ++ ) {
			
			p_entry = &timer.entries[ i ];
			
			if( p_entry ->fn_timer == fn_timer )
				return i;
		}
	} 
	
	// normal process 
	for( i = 0; i < TIMER_ENTRIES_NUM; i ++ ) {
		
		if( ( timer.mask & ( 1 << i ) ) == 0 )
			return i;
	}
	
	return -1;
}

int register_timer( fn_timer_t fn_timer, void *priv, long period )
{
	int idx;
	
	idx = find_unused_timer_entry_index( ( period < 0 ? fn_timer : NULL ) );
	
	if( idx >= 0 ) {
	
		register_timer_core( fn_timer, priv, period, idx );
		
		return idx;
	}
	
	PRINT_R( "No more timer entry!!\n" );
	
	return -1;	// no more entry  
}

static inline void check_and_announce_timer( void )
{
	int i;
	uint32 mask_shift = timer.mask;	// always check LSB 
	timer_entry_t *p_entry;
	
	CT_ASSERT( sizeof( mask_shift ) == sizeof( timer.mask ) );
	
	for( i = 0; i < TIMER_ENTRIES_NUM && mask_shift; i ++, mask_shift >>= 1 ) {
		
		if( ( mask_shift & 1 ) == 0 )
			continue;
		
		// check timeout ?
		p_entry = &timer.entries[ i ];
		
		if( timetick_after_eq( timetick, p_entry ->next_timetick ) ) {
			
			p_entry ->next_timetick += p_entry ->period;
			//p_entry ->next_timetick = timetick + p_entry ->period;	// another solution 
			
			p_entry ->fn_timer( p_entry ->priv );
			
			// unmask 
			if( !p_entry ->f_periodic )
				timer.mask &= ~( 1 << i );
		}		
	}
}

void increase_timetick( uint32 inc_ms )
{
	// increase timetick 
	timetick += inc_ms;
	
	// do timer 
	check_and_announce_timer();
}

// ------------------------------------------------------------------
// proc 
// ------------------------------------------------------------------
#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int voip_timer_read_proc(struct seq_file *f, void *v)
{
	int i;
	const timer_entry_t *p_entry;
	
	if( f->index ) {	/* In our case, we write out all data at once. */
		return 0;
	}
	seq_printf( f, "timetick=%lu\n", timetick );
	seq_printf( f, "         next\tperiod\tfn\t\tpriv\n" );
	
	for( i = 0; i < TIMER_ENTRIES_NUM; i ++ ) {
		
		p_entry = &timer.entries[ i ];
		
		seq_printf( f, "[%2d] %8lu\t%u%s\t%8p\t%p %s\n", i,
				p_entry ->next_timetick,
				p_entry ->period,
				( p_entry ->f_periodic ? "" : "@" ),
				p_entry ->fn_timer,
				p_entry ->priv,
				( timer.mask & ( 1 << i ) ? "(*)" : "" ) );
	}

	return 0;
}
#else
static int voip_timer_read_proc( char *buf, char **start, off_t off, int count, int *eof, void *data )
{
	int n = 0;
	int i;
	const timer_entry_t *p_entry;
	
	if( off ) {	/* In our case, we write out all data at once. */
		*eof = 1;
		return 0;
	}
	
	n += sprintf( buf + n, "timetick=%lu\n", timetick );
	n += sprintf( buf + n, "         next\tperiod\tfn\t\tpriv\n" );
	
	for( i = 0; i < TIMER_ENTRIES_NUM; i ++ ) {
		
		p_entry = &timer.entries[ i ];
		
		n += sprintf( buf + n, "[%2d] %8lu\t%u%s\t%8p\t%p %s\n", i,
				p_entry ->next_timetick,
				p_entry ->period,
				( p_entry ->f_periodic ? "" : "@" ),
				p_entry ->fn_timer,
				p_entry ->priv,
				( timer.mask & ( 1 << i ) ? "(*)" : "" ) );
	}
	
	*eof = 1;
	return n;
}
#endif

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int timer_open(struct inode *inode, struct file *file)
{
	return single_open(file, voip_timer_read_proc, NULL);
}

struct file_operations proc_timer_fops = {
	.owner	= THIS_MODULE,
	.open	= timer_open,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.release = single_release,	
//read:   voip_timer_read_proc
};
#endif

int __init voip_timer_proc_init( void )
{
#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
	create_voip_proc_read_entry( PROC_VOIP_DIR "/timer", 0, NULL, &proc_timer_fops, NULL );
#else
	create_voip_proc_read_entry( PROC_VOIP_DIR "/timer", 0, NULL, voip_timer_read_proc, NULL );
#endif
	return 0;
}

void __exit voip_timer_proc_exit( void )
{
	remove_voip_proc_entry( PROC_VOIP_DIR "/timer", NULL );
}

voip_initcall_proc( voip_timer_proc_init );
voip_exitcall( voip_timer_proc_exit );

