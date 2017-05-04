/*
 * This module is used to emulate networking impacts, such as 
 * packet loss, delay, and jitter.  
 *
 * How to use: 
 *   1. include this file in rtk_trap.c 
 *   2. rename rtk_trap to __rtk_trap
 *   3. rename rtk_trap_xx to rtk_trap
 */

#include "voip_types.h"
#include "voip_init.h"
#include "voip_timer.h"

extern int __rtk_trap(struct sk_buff *skb);

/*
 * Standard Normal Distribution
 * normal_dis_inv[ 0 ~ 511 ] = 0 ~ 3.09 
 */

#define Qx( x )		( ( uint16 )( ( x ) * ( 1 << 7 ) ) )	// Q7 

#define NORMAL_INV_TAB_MASK		0x7F	// 512 entries 

static const uint16 normal_dis_inv[] = {
/*  0*/ Qx(0.000), Qx(0.000), Qx(0.000), Qx(0.000), Qx(0.010), Qx(0.010), Qx(0.010), Qx(0.010),
/*  8*/ Qx(0.020), Qx(0.020), Qx(0.020), Qx(0.020), Qx(0.030), Qx(0.030), Qx(0.030), Qx(0.030),
/* 16*/ Qx(0.040), Qx(0.040), Qx(0.040), Qx(0.040), Qx(0.050), Qx(0.050), Qx(0.050), Qx(0.050),
/* 24*/ Qx(0.060), Qx(0.060), Qx(0.060), Qx(0.060), Qx(0.070), Qx(0.070), Qx(0.070), Qx(0.070),
/* 32*/ Qx(0.080), Qx(0.080), Qx(0.080), Qx(0.080), Qx(0.090), Qx(0.090), Qx(0.090), Qx(0.090),
/* 40*/ Qx(0.100), Qx(0.100), Qx(0.100), Qx(0.100), Qx(0.110), Qx(0.110), Qx(0.110), Qx(0.110),
/* 48*/ Qx(0.120), Qx(0.120), Qx(0.120), Qx(0.120), Qx(0.130), Qx(0.130), Qx(0.130), Qx(0.130),
/* 56*/ Qx(0.140), Qx(0.140), Qx(0.140), Qx(0.140), Qx(0.150), Qx(0.150), Qx(0.150), Qx(0.150),
/* 64*/ Qx(0.160), Qx(0.160), Qx(0.160), Qx(0.160), Qx(0.170), Qx(0.170), Qx(0.170), Qx(0.180),
/* 72*/ Qx(0.180), Qx(0.180), Qx(0.180), Qx(0.190), Qx(0.190), Qx(0.190), Qx(0.190), Qx(0.200),
/* 80*/ Qx(0.200), Qx(0.200), Qx(0.200), Qx(0.210), Qx(0.210), Qx(0.210), Qx(0.210), Qx(0.220),
/* 88*/ Qx(0.220), Qx(0.220), Qx(0.220), Qx(0.230), Qx(0.230), Qx(0.230), Qx(0.230), Qx(0.240),
/* 96*/ Qx(0.240), Qx(0.240), Qx(0.240), Qx(0.250), Qx(0.250), Qx(0.250), Qx(0.250), Qx(0.260),
/*104*/ Qx(0.260), Qx(0.260), Qx(0.270), Qx(0.270), Qx(0.270), Qx(0.270), Qx(0.280), Qx(0.280),
/*112*/ Qx(0.280), Qx(0.280), Qx(0.290), Qx(0.290), Qx(0.290), Qx(0.290), Qx(0.300), Qx(0.300),
/*120*/ Qx(0.300), Qx(0.300), Qx(0.310), Qx(0.310), Qx(0.310), Qx(0.310), Qx(0.320), Qx(0.320),
/*128*/ Qx(0.320), Qx(0.330), Qx(0.330), Qx(0.330), Qx(0.330), Qx(0.340), Qx(0.340), Qx(0.340),
/*136*/ Qx(0.340), Qx(0.350), Qx(0.350), Qx(0.350), Qx(0.350), Qx(0.360), Qx(0.360), Qx(0.360),
/*144*/ Qx(0.370), Qx(0.370), Qx(0.370), Qx(0.370), Qx(0.380), Qx(0.380), Qx(0.380), Qx(0.380),
/*152*/ Qx(0.390), Qx(0.390), Qx(0.390), Qx(0.400), Qx(0.400), Qx(0.400), Qx(0.400), Qx(0.410),
/*160*/ Qx(0.410), Qx(0.410), Qx(0.410), Qx(0.420), Qx(0.420), Qx(0.420), Qx(0.430), Qx(0.430),
/*168*/ Qx(0.430), Qx(0.430), Qx(0.440), Qx(0.440), Qx(0.440), Qx(0.440), Qx(0.450), Qx(0.450),
/*176*/ Qx(0.450), Qx(0.460), Qx(0.460), Qx(0.460), Qx(0.460), Qx(0.470), Qx(0.470), Qx(0.470),
/*184*/ Qx(0.480), Qx(0.480), Qx(0.480), Qx(0.480), Qx(0.490), Qx(0.490), Qx(0.490), Qx(0.490),
/*192*/ Qx(0.500), Qx(0.500), Qx(0.500), Qx(0.510), Qx(0.510), Qx(0.510), Qx(0.510), Qx(0.520),
/*200*/ Qx(0.520), Qx(0.520), Qx(0.530), Qx(0.530), Qx(0.530), Qx(0.540), Qx(0.540), Qx(0.540),
/*208*/ Qx(0.540), Qx(0.550), Qx(0.550), Qx(0.550), Qx(0.560), Qx(0.560), Qx(0.560), Qx(0.560),
/*216*/ Qx(0.570), Qx(0.570), Qx(0.570), Qx(0.580), Qx(0.580), Qx(0.580), Qx(0.590), Qx(0.590),
/*224*/ Qx(0.590), Qx(0.590), Qx(0.600), Qx(0.600), Qx(0.600), Qx(0.610), Qx(0.610), Qx(0.610),
/*232*/ Qx(0.620), Qx(0.620), Qx(0.620), Qx(0.620), Qx(0.630), Qx(0.630), Qx(0.630), Qx(0.640),
/*240*/ Qx(0.640), Qx(0.640), Qx(0.650), Qx(0.650), Qx(0.650), Qx(0.660), Qx(0.660), Qx(0.660),
/*248*/ Qx(0.660), Qx(0.670), Qx(0.670), Qx(0.670), Qx(0.680), Qx(0.680), Qx(0.680), Qx(0.690),
/*256*/ Qx(0.690), Qx(0.690), Qx(0.700), Qx(0.700), Qx(0.700), Qx(0.710), Qx(0.710), Qx(0.710),
/*264*/ Qx(0.720), Qx(0.720), Qx(0.720), Qx(0.730), Qx(0.730), Qx(0.730), Qx(0.740), Qx(0.740),
/*272*/ Qx(0.740), Qx(0.750), Qx(0.750), Qx(0.750), Qx(0.760), Qx(0.760), Qx(0.760), Qx(0.770),
/*280*/ Qx(0.770), Qx(0.770), Qx(0.780), Qx(0.780), Qx(0.780), Qx(0.790), Qx(0.790), Qx(0.790),
/*288*/ Qx(0.800), Qx(0.800), Qx(0.800), Qx(0.810), Qx(0.810), Qx(0.810), Qx(0.820), Qx(0.820),
/*296*/ Qx(0.820), Qx(0.830), Qx(0.830), Qx(0.840), Qx(0.840), Qx(0.840), Qx(0.850), Qx(0.850),
/*304*/ Qx(0.850), Qx(0.860), Qx(0.860), Qx(0.860), Qx(0.870), Qx(0.870), Qx(0.870), Qx(0.880),
/*312*/ Qx(0.880), Qx(0.890), Qx(0.890), Qx(0.890), Qx(0.900), Qx(0.900), Qx(0.900), Qx(0.910),
/*320*/ Qx(0.910), Qx(0.920), Qx(0.920), Qx(0.920), Qx(0.930), Qx(0.930), Qx(0.940), Qx(0.940),
/*328*/ Qx(0.940), Qx(0.950), Qx(0.950), Qx(0.950), Qx(0.960), Qx(0.960), Qx(0.970), Qx(0.970),
/*336*/ Qx(0.970), Qx(0.980), Qx(0.980), Qx(0.990), Qx(0.990), Qx(1.000), Qx(1.000), Qx(1.000),
/*344*/ Qx(1.010), Qx(1.010), Qx(1.020), Qx(1.020), Qx(1.030), Qx(1.030), Qx(1.030), Qx(1.040),
/*352*/ Qx(1.040), Qx(1.050), Qx(1.050), Qx(1.060), Qx(1.060), Qx(1.060), Qx(1.070), Qx(1.070),
/*360*/ Qx(1.080), Qx(1.080), Qx(1.090), Qx(1.090), Qx(1.100), Qx(1.100), Qx(1.110), Qx(1.110),
/*368*/ Qx(1.110), Qx(1.120), Qx(1.120), Qx(1.130), Qx(1.130), Qx(1.140), Qx(1.140), Qx(1.150),
/*376*/ Qx(1.150), Qx(1.160), Qx(1.160), Qx(1.170), Qx(1.170), Qx(1.180), Qx(1.180), Qx(1.190),
/*384*/ Qx(1.190), Qx(1.200), Qx(1.200), Qx(1.210), Qx(1.210), Qx(1.220), Qx(1.220), Qx(1.230),
/*392*/ Qx(1.230), Qx(1.240), Qx(1.250), Qx(1.250), Qx(1.260), Qx(1.260), Qx(1.270), Qx(1.270),
/*400*/ Qx(1.280), Qx(1.280), Qx(1.290), Qx(1.300), Qx(1.300), Qx(1.310), Qx(1.310), Qx(1.320),
/*408*/ Qx(1.330), Qx(1.330), Qx(1.340), Qx(1.350), Qx(1.350), Qx(1.360), Qx(1.360), Qx(1.370),
/*416*/ Qx(1.380), Qx(1.380), Qx(1.390), Qx(1.400), Qx(1.400), Qx(1.410), Qx(1.420), Qx(1.420),
/*424*/ Qx(1.430), Qx(1.440), Qx(1.450), Qx(1.450), Qx(1.460), Qx(1.470), Qx(1.470), Qx(1.480),
/*432*/ Qx(1.490), Qx(1.500), Qx(1.510), Qx(1.510), Qx(1.520), Qx(1.530), Qx(1.540), Qx(1.550),
/*440*/ Qx(1.550), Qx(1.560), Qx(1.570), Qx(1.580), Qx(1.590), Qx(1.600), Qx(1.610), Qx(1.620),
/*448*/ Qx(1.630), Qx(1.630), Qx(1.640), Qx(1.650), Qx(1.660), Qx(1.670), Qx(1.680), Qx(1.700),
/*456*/ Qx(1.710), Qx(1.720), Qx(1.730), Qx(1.740), Qx(1.750), Qx(1.760), Qx(1.770), Qx(1.790),
/*464*/ Qx(1.800), Qx(1.810), Qx(1.830), Qx(1.840), Qx(1.850), Qx(1.870), Qx(1.880), Qx(1.900),
/*472*/ Qx(1.910), Qx(1.930), Qx(1.950), Qx(1.960), Qx(1.980), Qx(2.000), Qx(2.020), Qx(2.040),
/*480*/ Qx(2.060), Qx(2.080), Qx(2.100), Qx(2.130), Qx(2.150), Qx(2.180), Qx(2.210), Qx(2.240),
/*488*/ Qx(2.270), Qx(2.300), Qx(2.340), Qx(2.380), Qx(2.430), Qx(2.480), Qx(2.540), Qx(2.610),
/*496*/ Qx(2.700), Qx(2.810), Qx(2.970), Qx(3.090), Qx(3.090), Qx(3.090), Qx(3.090), Qx(3.090),
/*504*/ Qx(3.090), Qx(3.090), Qx(3.090), Qx(3.090), Qx(3.090), Qx(3.090), Qx(3.090), Qx(3.090),
};

typedef struct {
	uint32 loss_rate;			// n / 128 
	uint32 delay_ms;			// in unit of ms 
	uint32 jitter_ms;			// in unit of ms 
	struct {
		uint16 port_min;
		uint16 port_max;
	} udp;
} impact_cfg_t;

typedef struct impact_q_s {
	struct sk_buff *skb;
	uint32 play_time;			// in unit of 10ms 
	struct impact_q_s *next;
} impact_q_t;

static const impact_cfg_t impact_cfg = {
	.loss_rate = 3,
	.delay_ms = 0,//50,
	.jitter_ms = 0,//100,
	.udp = {
		.port_min = 9000,
		.port_max = 9050,
	},
};

#define IMPACT_QUEUE_SIZE		4096

static impact_q_t impact_q[ IMPACT_QUEUE_SIZE ];
static impact_q_t *impact_sq_head = NULL;	// it is a sorted queue 
static impact_q_t *impact_empty_head = NULL;

static int impact_do_packet_loss( struct sk_buff *skb )
{
	static struct {
		uint32 total;
		uint32 loss;
	} packet;
	static uint32 seed = 0x754F378C;
	
	if( impact_cfg.loss_rate == 0 )
		return 0;
	
	seed = ( ( seed >> 7 ) + ( seed << 8 ) ) ^ 0x78908765;
	
	// reset 
	if( packet.total ++ > 128 ) {
		packet.total = 0;
		packet.loss = 0;
	}
	
	// loss this packet? 
	if( packet.loss < impact_cfg.loss_rate )
	{
		if( ( seed & 0x7F ) > impact_cfg.loss_rate ) {	// ratio is x/128 
			return 0;
		}
		
		// loss it!! 
		//printk( "Im:L " );
		packet.loss ++;
		return 1;
	}
	
	return 0;
}

static int impact_do_delay_jitter( struct sk_buff *skb )
{
	static uint32 seed = 0xD04BD377;
	uint16 ratio_q7;
	int32 target_delay_ms, target_jitter_ms;
	uint32 target_delay_10ms;	// assume system HZ = 100 
	impact_q_t *new_qentry;
	unsigned long flags;
	
	if( impact_empty_head == NULL ) {	// no more size 
		printk( "QF " );
		return 0;
	}
	
	if( impact_cfg.delay_ms == 0 &&
		impact_cfg.jitter_ms == 0 )
	{
		return 0;
	}
	
	seed = ( ( seed >> 7 ) + ( seed << 8 ) ) ^ 0x78908765;
	
	ratio_q7 = normal_dis_inv[ seed & NORMAL_INV_TAB_MASK ];
	
	// target_jitter_ms
	target_jitter_ms = ( impact_cfg.jitter_ms * ratio_q7 ) >> 7;	// Q7 -> Q0 
	
	// target_delay_ms
	if( ( seed & 0x80 ) == 0x80 )
		target_delay_ms = impact_cfg.delay_ms - target_jitter_ms;
	else
		target_delay_ms = impact_cfg.delay_ms + target_jitter_ms;
	
	// target_delay_10ms
	if( target_delay_ms < 0 )
		target_delay_10ms = 0;
	else
		target_delay_10ms = timetick / 10 + target_delay_ms / 10;
	
	if( target_delay_10ms == 0 )
		return 0;	// no need delay 
	
	// dequeue from empty queue 
	local_irq_save( flags );
	
	new_qentry = impact_empty_head;
	impact_empty_head = impact_empty_head ->next;
	
	local_irq_restore( flags );
	
	// fill data to new entry 
	new_qentry ->skb = skb;
	new_qentry ->play_time = target_delay_10ms;
	
	// insert sort to sq head 
	local_irq_save( flags );
	
	if( impact_sq_head == NULL ) {
		// first one 
		impact_sq_head = new_qentry;
		impact_sq_head ->next = NULL;
	} else if( timetick_after_eq( ( unsigned long )impact_sq_head ->play_time, ( unsigned long )target_delay_10ms ) ) {
		// insert in head 
		new_qentry ->next = impact_sq_head;
		impact_sq_head = new_qentry;
	} else {
		// insert in middle 
		impact_q_t *sq_pre = impact_sq_head;
		impact_q_t *sq = sq_pre ->next;
		
		while( sq && timetick_after( ( unsigned long )target_delay_10ms, ( unsigned long )sq ->play_time ) ) {
			sq_pre = sq;
			sq = sq ->next;
		}
		
		sq_pre ->next = new_qentry;
		new_qentry ->next = sq;
	}
	
	local_irq_restore( flags );
	
	return 1;
}

static int impact_do_udp_port_filter( struct sk_buff *skb )
{
	const struct udphdr * const uh = udp_hdr( skb );
	
	if( impact_cfg.udp.port_min == 0 ||
		impact_cfg.udp.port_max == 0 )
	{
		return 1;	// don't process it 
	}
	
	if( ( uint16 )uh ->dest >= impact_cfg.udp.port_min &&
		( uint16 )uh ->dest <= impact_cfg.udp.port_max )
	{
		return 0;	// process it!! 
	}
	
	return 1;	// don't process it 
}

int rtk_trap_xx( struct sk_buff *skb )
{
	// filter udp port 
	if( impact_do_udp_port_filter( skb ) )
		goto label_done;
	
	// packet loss ? 
	if( impact_do_packet_loss( skb ) ) {
		kfree_skb( skb );
		return RTK_TRAP_ACCEPT;
	} 
	
	// delay? jitter?
	if( impact_do_delay_jitter( skb ) ) {
		// free skb in impact_timer_handler() or __rtk_trap() 
		// If this packet is not RTP, it will be lost!! 
		return RTK_TRAP_ACCEPT;
	}
	
label_done:	
	return __rtk_trap( skb );
}

static void impact_timer_handler( unsigned long data )
{
	impact_q_t *sq, *sq_prev;
	impact_q_t *wq;
	unsigned long flags;
	
	if( impact_sq_head == NULL )
		goto label_done;
	
	// retrieve queue for work
	local_irq_save( flags );
	
	wq = sq = impact_sq_head;
	sq_prev = NULL;

	while( sq && timetick_after_eq( timetick, ( unsigned long )sq ->play_time ) ) 
	{
		sq_prev = sq;
		sq = sq ->next;
	}
	
	if( sq_prev == NULL )	// nothing need process 
		goto label_done;
	
	impact_sq_head = sq;	// disconnect sorted queue 
	
	sq_prev ->next = NULL;	// the last entry 
	
	local_irq_restore( flags );
	
	// send all skb in wq 
	sq = wq;
	
	while( sq ) 
	{
		//printk( "sq:%x ", sq );
		
		// try to trap this 
		if( __rtk_trap( sq ->skb ) != RTK_TRAP_ACCEPT ) {
			// free skb 
			kfree_skb( sq ->skb );
		}
		
		sq ->skb = NULL;
		
		// try next 
		sq = sq ->next;
	}
	
	// save to empty queue 
	local_irq_save( flags );
	
	sq_prev ->next = impact_empty_head;
	impact_empty_head = wq;
	
	local_irq_restore( flags );
	
label_done:	
	;
}

static int __init init_rtk_trap_impact( void )
{
	int i;
	
	// init timer 
	register_timer_10ms( ( fn_timer_t )impact_timer_handler, NULL );
	
	// init queue 
	impact_empty_head = &impact_q[ 0 ];
	
	for( i = 0; i < IMPACT_QUEUE_SIZE - 1; i ++ )
		impact_q[ i ].next = &impact_q[ i + 1 ];
	
	impact_q[ IMPACT_QUEUE_SIZE - 1 ].next = NULL;
	
	return 0;	
}

voip_initcall( init_rtk_trap_impact );

// ------------------------------------------------------------
// PROC
// ------------------------------------------------------------

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int voip_rtk_trap_impact_read_proc(struct seq_file *f, void *v)
{	
	int n = 0;
	impact_q_t *sq = impact_sq_head;

	seq_printf( f, "cfg:\n" );
	seq_printf( f, "\tloss=%u/128\n", impact_cfg.loss_rate );
	seq_printf( f, "\tdelay=%ums, jitter=%ums\n", impact_cfg.delay_ms, impact_cfg.jitter_ms );
	seq_printf( f, "\tudp port between (%u, %u)\n", impact_cfg.udp.port_min, impact_cfg.udp.port_max );
	
	seq_printf( f, "timetick = %lu\n", timetick );
	
	while( sq ) {
		
		// prevent overflow
		if( n > 3000 ) {
			seq_printf( f, "(...)\n" );
			break;
		}
		
		seq_printf( f, "%p: skb=%p, pt=%u\n", 
						sq, sq ->skb, sq ->play_time );
		
		sq = sq ->next;
	}
	
	return n;
}
#else
static int voip_rtk_trap_impact_read_proc( char *buf, char **start, off_t off, int count, int *eof, void *data )
{	
	int n = 0;
	impact_q_t *sq = impact_sq_head;

	if( off ) {	/* In our case, we write out all data at once. */
		*eof = 1;
		return 0;
	}
	
	n += sprintf( buf + n, "cfg:\n" );
	n += sprintf( buf + n, "\tloss=%u/128\n", impact_cfg.loss_rate );
	n += sprintf( buf + n, "\tdelay=%ums, jitter=%ums\n", impact_cfg.delay_ms, impact_cfg.jitter_ms );
	n += sprintf( buf + n, "\tudp port between (%u, %u)\n", impact_cfg.udp.port_min, impact_cfg.udp.port_max );
	
	n += sprintf( buf + n, "timetick = %lu\n", timetick );
	
	while( sq ) {
		
		// prevent overflow
		if( n > 3000 ) {
			n += sprintf( buf + n, "(...)\n" );
			break;
		}
		
		n += sprintf( buf + n, "%p: skb=%p, pt=%u\n", 
						sq, sq ->skb, sq ->play_time );
		
		sq = sq ->next;
	}
	
	*eof = 1;
	return n;
}
#endif

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int rtp_trap_impact_open(struct inode *inode, struct file *file)
{
	return single_open(file, voip_rtk_trap_impact_read_proc, NULL);
}

struct file_operations proc_rtp_trap_impact_fops = {
	.owner	= THIS_MODULE,
	.open	= rtp_trap_impact_open,
	.read	= seq_read,
	.llseek = seq_lseek,
	.release = single_release,	
//read:   voip_rtk_trap_impact_read_proc
};
#endif


static int __init voip_rtk_trap_impact_proc_init( void )
{
#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
	proc_create_dataa( PROC_VOIP_DIR "/impact", 0, NULL, &proc_rtp_trap_impact_fops, NULL );
#else
	create_proc_read_entry( PROC_VOIP_DIR "/impact", 0, NULL, voip_rtk_trap_impact_read_proc, NULL );
#endif

	return 0;
}

static void __exit voip_rtk_trap_impact_proc_exit( void )
{
	remove_proc_entry( PROC_VOIP_DIR "/impact", NULL );
}

voip_initcall_proc( voip_rtk_trap_impact_proc_init );
voip_exitcall( voip_rtk_trap_impact_proc_exit );

