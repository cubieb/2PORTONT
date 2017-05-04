#include <linux/timer.h>
#include <linux/jiffies.h>


#define timetick_before( a, b )         \
                ( ( long )( ( b ) - ( a ) ) > 0 ? 1 : 0 )
#define timetick_before_eq( a, b )              \
                ( ( long )( ( b ) - ( a ) ) >= 0 ? 1 : 0 )
#define timetick_after( a, b )          \
                ( ( long )( ( a ) - ( b ) ) > 0 ? 1 : 0 )
#define timetick_after_eq( a, b )               \
                ( ( long )( ( a ) - ( b ) ) >= 0 ? 1 : 0 )

#define TIMETICK_SEED      0xFFFF0000

#ifdef __KERNEL__
#define PCSCTIMER_PRINT printk
#else
#define PCSCTIMER_PRINT printf
#endif


typedef unsigned long timetick_t;
typedef unsigned int uint32;
typedef void ( *fn_timer_t )( void *priv );
static struct timer_list os_linux_timer;
timetick_t sc_timetick = TIMETICK_SEED;

#define TIMER_ENTRIES_NUM               8       // up to 32

typedef struct {
        timetick_t next_timetick;       // next announce timetick
        uint32 period;                  // timer period (ms)
        uint32 f_periodic:1;    // periodic timer ?
        fn_timer_t fn_timer;    // timer function
        void *priv;                             // private data for timer function
} timer_entry_t;

static struct {
        timer_entry_t entries[ TIMER_ENTRIES_NUM ];
        uint32 mask;
} timer;

static void register_timer_core( fn_timer_t fn_timer, void *priv,
                                                                        long period, int idx )
{
        timer_entry_t * const p_entry = &timer.entries[ idx ];

        // negative period means one shoot timer
        p_entry ->f_periodic = ( period > 0 ? 1 : 0 );
        period = ( period < 0 ? period * ( -1 ) : period );

        p_entry ->next_timetick = sc_timetick + period;    // save current + period
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

int reg_timer( fn_timer_t fn_timer, void *priv, long period )
{
        int idx;

        idx = find_unused_timer_entry_index( ( period < 0 ? fn_timer : NULL ) );

        if( idx >= 0 ) {

                register_timer_core( fn_timer, priv, period, idx );

                return idx;
        }

        PCSCTIMER_PRINT( "No more timer entry!!\n" );

        return -1;      // no more entry
}

static inline void check_and_announce_timer( void )
{
        int i;
        uint32 mask_shift = timer.mask; // always check LSB
        timer_entry_t *p_entry;

        for( i = 0; i < TIMER_ENTRIES_NUM && mask_shift; i ++, mask_shift >>= 1 ) {

                if( ( mask_shift & 1 ) == 0 )
                        continue;

                // check timeout ?
                p_entry = &timer.entries[ i ];

                if( timetick_after_eq( sc_timetick, p_entry ->next_timetick ) ) {

                        p_entry ->next_timetick += p_entry ->period;
                        //p_entry ->next_timetick = timetick + p_entry ->period;        // another solution

                        p_entry ->fn_timer( p_entry ->priv );

                        // unmask
                        if( !p_entry ->f_periodic )
                                timer.mask &= ~( 1 << i );
                }
        }
}

void increase_tick( uint32 inc_ms )
{
        // increase timetick
        sc_timetick += inc_ms;

        // do timer
        check_and_announce_timer();
}

static void os_linux_timer_func( unsigned long data )
{
#if ( 1000 / HZ ) < 10
        // jiffies period < 10ms ==> announce timer in 10 ms
        increase_tick( 10 /* 10ms */ );

        mod_timer( &os_linux_timer, jiffies + 10 / ( 1000 / HZ ) );
#else
        // jiffies period >= 10ms
        increase_tick( 1000 / HZ );

        mod_timer( &os_linux_timer, jiffies + 1 );
#endif
}

void start_timer( void )
{
        init_timer( &os_linux_timer );

        os_linux_timer.expires = jiffies + 1;
        os_linux_timer.function = os_linux_timer_func;

        add_timer( &os_linux_timer );
}

