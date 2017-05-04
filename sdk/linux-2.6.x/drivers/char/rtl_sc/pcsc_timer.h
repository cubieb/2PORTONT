#ifndef __SMTIMER__
#define __SMTIMER__

typedef unsigned long timetick_t;
typedef unsigned int uint32;

typedef void ( *fn_timer_t )( void *priv );

int reg_timer( void * fn_timer, void *priv, long period );
void start_timer( void );
#endif
