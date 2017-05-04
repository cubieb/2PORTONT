#ifndef __AIPC_MBOX_H__
#define __AIPC_MBOX_H__

#include "aipc_global.h"
#include "soc_type.h"
#include "aipc_osal.h"
#include "aipc_shm.h"
#include "aipc_api.h"
#include "aipc_cbk.h"

/*****************************************************************************
*   Macro Definitions
*****************************************************************************/

/*****************************************************************************
*   Data Structure
*****************************************************************************/
#ifdef CONFIG_RTL8686_IPC_TEST_THREAD
#define CPU_SEND_THREAD_NAME_SIZE		32
#define DSP_SEND_THREAD_NAME_SIZE		32
#define DSP_RECV_THREAD_NAME_SIZE		32


//CPU
#define CPU_SEND_THREAD_NAME		"CpuMboxSendD"
#define CPU_CHECK_START_PERIOD		((1)*(MUL)*10)
#define CPU_SEND_PERIOD				((1)*(MUL))

//DSP
#define DSP_SEND_THREAD_NAME		"DspMboxSendD"
#define DSP_POLL_THREAD_NAME		"DspMboxPollD"
#define DSP_CHECK_START_PERIOD		((1)*(MUL)*10)
#define DSP_SEND_PERIOD				((1)*(MUL))
#define DSP_POLL_PERIOD				((1)*(MUL))
#endif

#ifdef CONFIG_RTL8686_IPC_TEST_THREAD_2
#define CPU_SEND_THREAD_NAME_SIZE_2		32
#define DSP_SEND_THREAD_NAME_SIZE_2		32
#define DSP_RECV_THREAD_NAME_SIZE_2		32


//CPU
#define CPU_SEND_THREAD_NAME_2		"CpuMboxSendD_2"
#define CPU_CHECK_START_PERIOD_2	((1)*(MUL)*10)
#define CPU_SEND_PERIOD_2			((1)*(MUL)*2)

//DSP
#define DSP_SEND_THREAD_NAME_2		"DspMboxSendD_2"
#define DSP_POLL_THREAD_NAME_2		"DspMboxPollD_2"
#define DSP_CHECK_START_PERIOD_2	((1)*(MUL)*10)
#define DSP_SEND_PERIOD_2			((1)*(MUL)*2)
#define DSP_POLL_PERIOD_2			((1)*(MUL)*2)
#endif

#ifdef CONFIG_RTL8686_IPC_TEST_THREAD_3
#define CPU_SEND_THREAD_NAME_SIZE_3		32
#define DSP_SEND_THREAD_NAME_SIZE_3		32
#define DSP_RECV_THREAD_NAME_SIZE_3		32


//CPU
#define CPU_SEND_THREAD_NAME_3		"CpuMboxSendD_3"
#define CPU_CHECK_START_PERIOD_3	((1)*(MUL)*10)
#define CPU_SEND_PERIOD_3			((1)*(MUL)*3)

//DSP
#define DSP_SEND_THREAD_NAME_3		"DspMboxSendD_3"
#define DSP_POLL_THREAD_NAME_3		"DspMboxPollD_3"
#define DSP_CHECK_START_PERIOD_3	((1)*(MUL)*10)
#define DSP_SEND_PERIOD_3			((1)*(MUL)*3)
#define DSP_POLL_PERIOD_3			((1)*(MUL)*3)
#endif

/*****************************************************************************
*   Export Function
*****************************************************************************/
/*
*	CPU		Linux AIPC
*/
/*	
*	Function name:
*		aipc_2cpu_mbox_recv
*	Description:
*		Receive Data from mbox
*		This function is ISR context. Receiving RTP packets in CPU<-DSP direction
*		uses interrupt mechanism. DSP sending RTP packets in constant time and 
*		frequency is small. DSP notifies CPU about packet arriving only when need.
*	Parameters:
*		u32_t int_id:	parameters from ISR. doesn't need to handle
*		void *data	:	parameters from ISR. doesn't need to handle
*	Return:
*		OK	: success
*		NOK	: fail
*/
extern int		aipc_2cpu_mbox_recv( u32_t int_id , void *data );

/*****************************************************************************
*   Function
*****************************************************************************/


/*****************************************************************************
*   External Function
*****************************************************************************/
/*
*	CPU
*/
extern void*	aipc_2cpu_mb_dequeue( void );
extern int		aipc_2cpu_bc_enqueue( void * dp );
extern void*	aipc_2dsp_bc_dequeue( void );
extern int		aipc_2dsp_mb_enqueue( void * dp );

/*
*	DSP
*/
extern int		aipc_2cpu_mb_enqueue( void * dp );
extern void*	aipc_2cpu_bc_dequeue( void );
extern void*	aipc_2dsp_mb_dequeue( void );
extern int		aipc_2dsp_bc_enqueue( void * dp );

#endif
