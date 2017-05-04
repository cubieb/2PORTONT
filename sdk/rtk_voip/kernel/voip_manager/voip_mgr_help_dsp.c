#include <linux/kernel.h>
#include <linux/timer.h>

#include "rtk_voip.h"
#include "voip_types.h"
#include "voip_errno.h"
#include "voip_control.h"
#include "voip_ipc.h"
#include "voip_params.h"
#include "voip_init.h"
#include "voip_proc.h"

#include "snd_define.h"

#include "ipc_arch_help_dsp.h"
#include "ipc_arch_tx.h"

#include "voip_mgr_events.h"

//#define EVENT_TX_DETAIL_DEBUG

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_ISSUE_TRIVIAL_ACK
#define EVENT_ISSUE_TRIVIAL_ACK	1
#endif

// --------------------------------------------------------
// DSP side send event to Host side (by timer)
// --------------------------------------------------------

#ifdef EVENT_ISSUE_TRIVIAL_ACK

#define EVENT_TX_WINDOW_SIZE	( CON_CH_NUM * 10 )
#define MAX_RESEND_EVENT		3
#define RESEND_DELAY_COUNT		10	// 100 ms  

typedef struct {
	TstVoipEvent event;
	int ack;			// wait for ack 
	uint16 sent_seq;	// sent squence number 
	int resend_count;	// resent count 
} EventTxElement_t;

static struct {
	EventTxElement_t element[ EVENT_TX_WINDOW_SIZE ];
	uint32 ri, wi;
	uint32 resend_time_delay;
} EventTx;

#endif // EVENT_ISSUE_TRIVIAL_ACK

int last_process_seqno = -1;

extern int con_ch_num;

void clean_event_cheak_ack(unsigned short category, unsigned short rev_seq)
{
#ifdef EVENT_ISSUE_TRIVIAL_ACK
	int i;
	uint32 ri = EventTx.ri;
	EventTxElement_t *pelm;
	
	for( i = 0; i < EVENT_TX_WINDOW_SIZE; i ++ ) {
		
		// no more sent event 
		if( ri == EventTx.wi ) {
			PRINT_R( "Unhandled event seq=%u, category=%u\n", rev_seq, category );
			break;
		}
		
		// do check 
		pelm = &EventTx.element[ ri ];
		
		if( pelm ->sent_seq == rev_seq ) {
			
			pelm ->ack = 1;
			goto label_reclaim_space;
		}
		
		// next ri 
		ri = ( ri + 1 >= EVENT_TX_WINDOW_SIZE ? 0 : ri + 1 );
	}
	
	return;
	
	// reclaim process 
label_reclaim_space:
	
	ri = EventTx.ri;
	
	for( i = 0; i < EVENT_TX_WINDOW_SIZE; i ++ ) {
		
		// no more sent event 
		if( ri == EventTx.wi ) {
			break;
		}
		
		// do check 
		pelm = &EventTx.element[ ri ];
		
		if( pelm ->ack == 0 )
			break;
		
		// next ri 
		ri = ( ri + 1 >= EVENT_TX_WINDOW_SIZE ? 0 : ri + 1 );
	}
	
	EventTx.ri = ri;
	
	return;
#endif // EVENT_ISSUE_TRIVIAL_ACK
}

#ifdef EVENT_ISSUE_TRIVIAL_ACK
static void event_tx_issue_trivial_ack( unsigned long data )
{
	uint32 cch;
	TstTxPktCtrl pktCtrl;
	int full = 0;
	
	TstVoipEvent event;
	uint32 wi_next;
	EventTxElement_t *pelm;
	
	int i;
	uint32 ri, wi;
	
	pktCtrl.dsp_cpuid = Get_DSP_CPUID();
	
	// -----------------------------------------------------------------------
	// resend old events 
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_RESEND_CTRL
	if( EventTx.resend_time_delay ++ >= RESEND_DELAY_COUNT )
		EventTx.resend_time_delay = 0;	// reset and resend 
	else
#endif
	{
		goto label_resend_old_events_end;
	}
	
	ri = EventTx.ri;
	wi = EventTx.wi;
	
	for( i = 0; i < EVENT_TX_WINDOW_SIZE; i ++ ) {
		
		// no more to resend 
		if( ri == wi )
			break;
		
		// check this element 
		pelm = &EventTx.element[ ri ];
		
		if( pelm ->resend_count >= MAX_RESEND_EVENT ) {
			pelm ->ack = 2;	// too many resend 
		} else if( pelm ->ack == 0 ) {
			
			// resend it 
			pelm ->resend_count ++;
			pktCtrl.dsp_cch = pelm ->event.ch_id;
			pktCtrl.seq_no = pelm ->sent_seq;
			pktCtrl.resend_flag = 1;

#ifdef EVENT_TX_DETAIL_DEBUG
		printk( "resend event ch_id=%X, type=%x, mask=%x, id=%x, p0=%x, p1=%x, time=%x\n",
				 pelm ->event.ch_id, pelm ->event.type, pelm ->event.mask, 
				 pelm ->event.id, pelm ->event.p0, pelm ->event.p1, pelm ->event.time);
#endif
			
			ipc_pkt_tx_final( IPC_EVT_VOIP_EVENT, IPC_PROT_EVENT, 
								( uint8* )&pelm ->event, sizeof( pelm ->event ), 
								&pktCtrl, NULL, NULL );
		}
		
		// next ri
		ri = ( ri + 1 >= EVENT_TX_WINDOW_SIZE ? 0 : ri + 1 );
	}

label_resend_old_events_end:
	
	// -----------------------------------------------------------------------
	// send events  
	wi_next = ( EventTx.wi + 1 >= EVENT_TX_WINDOW_SIZE ? 0 : EventTx.wi + 1 );
	
	for( cch = 0; cch < con_ch_num; cch ++ )
	{
		// tx window full ?
		if( wi_next == EventTx.ri ) {
			full = 1;
			break;	// no more buffer 
		}
		
		// fill event fields to retrieve event 
		event.ch_id = cch;
		event.type = VET_ALL;
		event.mask = VEM_ALL;
		
		// event empty ? 
		if( voip_mgr_event_out( &event ) < 0 || event.id == VEID_NONE )
			continue;

#ifdef EVENT_TX_DETAIL_DEBUG
		printk( "event ch_id=%X, type=%x, mask=%x, id=%x, p0=%x, p1=%x, time=%x\n",
				 event.ch_id, event.type, event.mask, event.id, event.p0, event.p1, event.time);
#endif
		
		// fix event field
		event.type = event.id & VET_MASK;

#ifdef EVENT_TX_DETAIL_DEBUG
		printk( "event ch_id=%X, type=%x, mask=%x, id=%x, p0=%x, p1=%x, time=%x\n",
				 event.ch_id, event.type, event.mask, event.id, event.p0, event.p1, event.time);
#endif

#ifdef EVENT_TX_DETAIL_DEBUG
		int i;
		
		printk( "event tx:\n" );
		
		for( i = 0; i < sizeof( event ); i ++ ) {
			printk( "%02X, ", *( ( unsigned char * )&event + i ) );
		}
		printk( "\n" );
#endif
		
		// do it!! 
		pelm = &EventTx.element[ EventTx.wi ];
		
		pktCtrl.dsp_cch = cch;
		pktCtrl.seq_no = 0;	// ignore 
		pktCtrl.resend_flag = 0;	// not resend event 
		
		pelm ->event = event;
		pelm ->ack = 0;
		pelm ->resend_count = 0;
		EventTx.wi = wi_next;	// to avoid race condition 
		ipc_pkt_tx_final( IPC_EVT_VOIP_EVENT, IPC_PROT_EVENT, 
							( uint8* )&pelm ->event, sizeof( pelm ->event ), 
							&pktCtrl, &pelm ->sent_seq, NULL );
		
		// reset resnd delay 
		EventTx.resend_time_delay = 0;
		
		// next wi 
		wi_next = ( EventTx.wi + 1 >= EVENT_TX_WINDOW_SIZE ? 0 : EventTx.wi + 1 );
		
	}	// cch for loop
	
}
#else
static void event_tx_not_issue_trivial_ack( unsigned long data )
{
	uint32 cch;
	TstTxPktCtrl pktCtrl;
	TstVoipEvent event;
	
	pktCtrl.dsp_cpuid = Get_DSP_CPUID();
	
	for( cch = 0; cch < con_ch_num; cch ++ )
	{
		// fill event fields to retrieve event 
		event.ch_id = cch;
		event.type = VET_ALL;
		event.mask = VEM_ALL;
		
		// event empty ? 
		if( voip_mgr_event_out( &event ) < 0 || event.id == VEID_NONE )
			continue;
		
		// fix event field
		event.type = event.id & VET_MASK;
		
		// do it!! 
		pktCtrl.dsp_cch = cch;
		pktCtrl.seq_no = 0;	// ignore 
		pktCtrl.resend_flag = 0;	// not resend event 
		
		ipc_pkt_tx_final( IPC_EVT_VOIP_EVENT, IPC_PROT_EVENT, 
							( uint8* )&event, sizeof( event ), 
							&pktCtrl, NULL, NULL );
	}
}
#endif // EVENT_ISSUE_TRIVIAL_ACK

static void event_tx( unsigned long data )
{
#ifdef EVENT_ISSUE_TRIVIAL_ACK
	event_tx_issue_trivial_ack( data );
#else
	event_tx_not_issue_trivial_ack( data );
#endif
}

static int Add_event_tx_timer( void )
{
	register_timer_10ms( ( fn_timer_t )event_tx, NULL );
	
	PRINT_G("Add Timer for Ack Check\n");
	PRINT_G("This is DSP-ID %d\n", Get_DSP_CPUID());
	
	return 0;
}

voip_initcall_sync( Add_event_tx_timer );	// should after event manager 

// -----------------------------------------------------------------
// Proc 
// -----------------------------------------------------------------

#ifdef EVENT_ISSUE_TRIVIAL_ACK

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int voip_event_tx_read_proc(struct seq_file *f, void *v)
{
	int n = 0;
	int i;
	uint32 ri, wi;
		
	seq_printf( f, "Evnet TX: ri=%u, wi=%u\n", EventTx.ri, EventTx.wi );
	
	// detail elements 
	seq_printf( f, "\tri\tseq\tack\n" );
	
	ri = EventTx.ri;
	wi = EventTx.wi;
	
	for( i = 0; i < EVENT_TX_WINDOW_SIZE; i ++ ) {
		
		if( ri == wi )
			break;
		
		seq_printf( f, "\t%u\t%u\t%u\n", ri,
								EventTx.element[ ri ].sent_seq,
								EventTx.element[ ri ].ack );
		
		// next ri
		ri = ( ri + 1 >= EVENT_TX_WINDOW_SIZE ? 0 : ri + 1 );
	}
	
	return n;
}
#else
static int voip_event_tx_read_proc( char *buf, char **start, off_t off, int count, int *eof, void *data )
{
	int n = 0;
	int i;
	uint32 ri, wi;
	
	if( off ) {	/* In our case, we write out all data at once. */
		*eof = 1;
		return 0;
	}
	
	n += sprintf( buf + n, "Evnet TX: ri=%u, wi=%u\n", EventTx.ri, EventTx.wi );
	
	// detail elements 
	n += sprintf( buf + n, "\tri\tseq\tack\n" );
	
	ri = EventTx.ri;
	wi = EventTx.wi;
	
	for( i = 0; i < EVENT_TX_WINDOW_SIZE; i ++ ) {
		
		if( ri == wi )
			break;
		
		n += sprintf( buf + n, "\t%u\t%u\t%u\n", ri,
								EventTx.element[ ri ].sent_seq,
								EventTx.element[ ri ].ack );
		
		// next ri
		ri = ( ri + 1 >= EVENT_TX_WINDOW_SIZE ? 0 : ri + 1 );
	}
	
	*eof = 1;
	return n;
}
#endif

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int event_tx_read_open(struct inode *inode, struct file *file)
{
	return single_open(file, voip_event_tx_read_proc, NULL);
}

struct file_operations proc_event_tx_read_fops = {
	.owner	= THIS_MODULE,
	.open	= event_tx_read_open,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.release = single_release,
//read:   voip_event_tx_read_proc
};
#endif


static int __init voip_event_tx_proc_init( void )
{
#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
	proc_create_data( PROC_VOIP_DIR "/event_tx", 0, NULL, &proc_event_tx_read_fops, NULL );
#else
	create_proc_read_entry( PROC_VOIP_DIR "/event_tx", 0, NULL, voip_event_tx_read_proc, NULL );
#endif
	
	return 0;
}

static void __exit voip_event_tx_proc_exit( void )
{
	remove_voip_proc_entry( PROC_VOIP_DIR "/event_tx", NULL );
}

voip_initcall_proc( voip_event_tx_proc_init );
voip_exitcall( voip_event_tx_proc_exit );

#endif // EVENT_ISSUE_TRIVIAL_ACK

int do_voip_mgr_ctl_in_dsp( unsigned short cmd, unsigned char* para, unsigned short length, unsigned short seq_no, const voip_mgr_entry_t *p_entry )
{
	int ret = 0;

	if( p_entry ->do_mgr == NULL )
	{
		PRINT_R("IOCTL command %d has no do_mgr\n", cmd);
		ret = -EVOIP_IOCTL_NO_MGR_ERR;
	}
	else
		ret = p_entry ->do_mgr( cmd, para, length, seq_no );

	last_process_seqno = seq_no;
	
	return ret;
}

