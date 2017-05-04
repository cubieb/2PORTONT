#ifndef _AIPC_GOT_H_
#define _AIPC_GOT_H_

//#include "aipc_global.h"
//#include "soc_type.h"
//#include "aipc_mbox.h"
//#include "aipc_intq.h"

typedef struct {
	/*******************************************************************************
	*	mbox
	*******************************************************************************/
	/* CPU */
	void*	(*aipc_2cpu_mb_dequeue)( void );
	int 	(*aipc_2cpu_bc_enqueue)( void * dp );
	void*	(*aipc_2dsp_bc_dequeue)( void );
	int 	(*aipc_2dsp_mb_enqueue)( void * dp );
	
	/* DSP */
	int 	(*aipc_2cpu_mb_enqueue)( void * dp );
	void*	(*aipc_2cpu_bc_dequeue)( void );
	void*	(*aipc_2dsp_mb_dequeue)( void );
	int 	(*aipc_2dsp_bc_enqueue)( void * dp );
	
	/*******************************************************************************
	*	int queue
	*******************************************************************************/
	/* CPU */
	//int 	(*aipc_int_send_2dsp)( u32_t int_id );
	int		(*aipc_int_2dsp_hiq_enqueue)( u32_t int_id );
	int		(*aipc_int_2dsp_lowq_enqueue)( u32_t int_id );
	int 	(*aipc_int_2cpu_hiq_dequeue)( u32_t *fid );
	int 	(*aipc_int_2cpu_lowq_dequeue)( u32_t *fid );
	int 	(*aipc_int_2cpu_hiq_empty)( void );
	int 	(*aipc_int_2cpu_lowq_empty)( void );
	int 	(*aipc_int_2dsp_hiq_full)( void );
	int 	(*aipc_int_2dsp_lowq_full)( void );
	
	/* DSP */
	//int 	(*aipc_int_send_2cpu)( u32_t int_id );
	int		(*aipc_int_2cpu_hiq_enqueue)( u32_t int_id );
	int		(*aipc_int_2cpu_lowq_enqueue)( u32_t int_id );
	int 	(*aipc_int_2dsp_hiq_dequeue)( u32_t *fid );
	int 	(*aipc_int_2dsp_lowq_dequeue)( u32_t *fid );
	int 	(*aipc_int_2dsp_hiq_empty)( void );
	int 	(*aipc_int_2dsp_lowq_empty)( void );
	int 	(*aipc_int_2cpu_hiq_full)( void );
	int 	(*aipc_int_2cpu_lowq_full)( void );

} aipc_got_t;


#endif

