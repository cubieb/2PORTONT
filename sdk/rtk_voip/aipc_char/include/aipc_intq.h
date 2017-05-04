#ifndef __AIPC_INTQ_H__
#define __AIPC_INTQ_H__

#include "aipc_global.h"
#include "soc_type.h"
#include "aipc_osal.h"
#include "aipc_shm.h"

/*****************************************************************************
*   Export Function
*****************************************************************************/
/*
*	CPU
*/
extern int	aipc_int_send_2dsp( u32_t int_id );
extern int	aipc_int_2dsp_hiq_enqueue( u32_t int_id );
extern int	aipc_int_2dsp_lowq_enqueue( u32_t int_id );
extern int	aipc_int_2cpu_hiq_dequeue( u32_t *fid );
extern int	aipc_int_2cpu_lowq_dequeue( u32_t *fid );
extern int	aipc_int_2cpu_hiq_empty( void );
extern int	aipc_int_2cpu_lowq_empty( void );
extern int	aipc_int_2dsp_hiq_full( void );
extern int	aipc_int_2dsp_lowq_full( void );

/*
*	DSP
*/
extern int 	aipc_int_send_2cpu( u32_t int_id );
extern int	aipc_int_2cpu_hiq_enqueue( u32_t int_id );
extern int	aipc_int_2cpu_lowq_enqueue( u32_t int_id );
extern int	aipc_int_2dsp_hiq_dequeue( u32_t *fid );
extern int	aipc_int_2dsp_lowq_dequeue( u32_t *fid );
extern int	aipc_int_2dsp_hiq_empty( void );
extern int	aipc_int_2dsp_lowq_empty( void );
extern int	aipc_int_2cpu_hiq_full( void );
extern int	aipc_int_2cpu_lowq_full( void );

#endif

