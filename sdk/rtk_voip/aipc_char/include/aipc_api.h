#ifndef __AIPC_API_H__
#define __AIPC_API_H__

/*****************************************************************************
*   Data Plane
*****************************************************************************/
/*
*	CPU
*/
/*	
*	Function name:
*		aipc_data_2dsp_alloc
*	Description:
*		Get available buffer from mailbox (2dsp direction)
*		This function is process context
*	Parameters:
*		None
*	Return:
*		None NULL	: start address of available mail (available buffer address)
*		NULL	 	: no available buffer
*/
extern void *   aipc_data_2dsp_alloc( void );

/*	
*	Function name:
*		aipc_data_2dsp_send
*	Description:
*		Send data from CPU to DSP (2dsp direction)
*		This function is process context
*	Parameters:
*		void *dp	: buffer which will be sent to DSP
*	Return:
*		OK	: success
*		NOK	: fail
*/
extern int      aipc_data_2dsp_send( void *dp );

/*	
*	Function name:
*		aipc_data_2cpu_ret
*	Description:
*		Return buffer to mailbox (2cpu direction)
*		This function is process context
*	Parameters:
*		void *dp	: buffer which will be returned to buffer circulation
*	Return:
*		OK	: success
*		NOK	: fail
*/
extern int      aipc_data_2cpu_ret( void *dp );


/*	
*	Function name:
*		aipc_data_2cpu_recv
*	Description:
*		Receive buffer from mailbox (2cpu direction)
*		This function is interrupt context (to be invoked in ISR or DSR)
*	Parameters:
*		None
*	Return:
*		None NULL	: buffer which will be received from mailbox
*		NULL		: No buffer
*/
extern void *	aipc_data_2cpu_recv( void );

/*
*	DSP
*/
/*	
*	Function name:
*		aipc_data_2cpu_alloc
*	Description:
*		Get available buffer from mailbox (2cpu direction)
*		This function is process context
*	Parameters:
*		None
*	Return:
*		None NULL	: start address of available mail (available buffer address)
*		NULL	 	: no available buffer
*/

extern void *	aipc_data_2cpu_alloc( void );

/*	
*	Function name:
*		aipc_data_2cpu_send
*	Description:
*		Send data from DSP to CPU (2cpu direction)
*		This function is process context
*	Parameters:
*		void *dp	: buffer which will be sent to DSP
*	Return:
*		OK	: success
*		NOK	: fail
*/
extern int		aipc_data_2cpu_send( void *dp );

/*	
*	Function name:
*		aipc_data_2dsp_ret
*	Description:
*		Return buffer to mailbox (2dsp direction)
*		This function is process context
*	Parameters:
*		void *dp	: buffer which will be returned to buffer circulation
*	Return:
*		OK	: success
*		NOK	: fail
*/
extern int		aipc_data_2dsp_ret( void *dp );

/*	
*	Function name:
*		aipc_data_2dsp_recv
*	Description:
*		Receive buffer from mailbox (2dsp direction)
*		This function is process context
*	Parameters:
*		None
*	Return:
*		None NULL	: buffer which will be received from mailbox
*		NULL		: No buffer
*/
extern void *	aipc_data_2dsp_recv( void);

/*****************************************************************************
*   Control Plane
*****************************************************************************/
typedef enum {
	AIPC_NON_SCHEDULABLE = 0,
	AIPC_SCHEDULABLE
} aipc_scheduable;

typedef enum {
	CTRL_NOFBK,
	CTRL_FBK,
	CTRL_OTHER
} ctrl_type;

/** Control **/
#define IS_NOFBK(_p)   (((unsigned long)&(CBUF_CMD_2DSP) <= (unsigned long)_p) && 								\
						((unsigned long)_p <= (unsigned long)&(CBUF_CMD_2DSP[CMD_BUF_TOTAL-1])) && 				\
						(((unsigned long)_p - (unsigned long)&(CBUF_CMD_2DSP))%sizeof(cmd_element_t)==0)		\
						? 1 : 0) 

#define IS_FBK(_p)     (((unsigned long)&(CBUF_CMD_LOCAL_2DSP) <= (unsigned long)_p) && 								\
						((unsigned long)_p <= (unsigned long)&(CBUF_CMD_LOCAL_2DSP[CMD_LOCAL_BUF_TOTAL-1])) && 			\
						(((unsigned long)_p - (unsigned long)&(CBUF_CMD_LOCAL_2DSP))%sizeof(cmd_local_element_t)==0)	\
						? 1 : 0)



#define CMD_LOCBUF_IDX(_p)																\
						((((unsigned long)&(CBUF_CMD_LOCAL_2DSP) <= (unsigned long)_p) && 								\
						 ((unsigned long)_p <= (unsigned long)&(CBUF_CMD_LOCAL_2DSP[CMD_LOCAL_BUF_TOTAL-1]))) 			\
						 ? ((unsigned long)_p - (unsigned long)&(CBUF_CMD_LOCAL_2DSP))/sizeof(cmd_local_element_t) : NOK )


/** Event **/
#define IS_EVENT(_p)   (((unsigned long)&(CBUF_EVENT_2CPU) <= (unsigned long)_p) && 									\
						((unsigned long)_p <= (unsigned long)&(CBUF_EVENT_2CPU[EVENT_QUEUE_TOTAL-1])) && 				\
						(((unsigned long)_p - (unsigned long)&(CBUF_EVENT_2CPU))%sizeof(event_element_t)==0)			\
						? 1 : 0)

#define EVENT_IDX(_p)  ((((unsigned long)&(CBUF_EVENT_2CPU) <= (unsigned long)_p) && 									\
						((unsigned long)_p <= (unsigned long)&(CBUF_EVENT_2CPU[EVENT_QUEUE_TOTAL-1]))) 					\
						? ((unsigned long)_p - (unsigned long)&(CBUF_EVENT_2CPU))/sizeof(event_element_t) : NOK)

#define RETURN_CTRL(_p)	({																\
						do{																\
							if (IS_NOFBK(_p)){											\
							    aipc_ctrl_2dsp_nofbk_ret(_p);							\
								}														\
							else if(IS_FBK(_p)){										\
							    aipc_ctrl_2dsp_fbk_fin(_p);								\
								}														\
							else{														\
								ASTATS.aipc_ctrl_error++;								\
							    printk("error case!! wrong buffer pointer\n");			\
								}														\
						}while(0);														\
						})


/*
*	Control CPU->DSP
*/
/**	CPU **/
/*	
*	Function name:
*		aipc_ctrl_2dsp_nofbk_alloc
*	Description:
*		Get buffer no feedback ready to send from CPU to DSP
*		This function is process context
*	Parameters:
*		None
*	Return:
*		Control buffer address
*	Comments:
*		
*/
extern void *	aipc_ctrl_2dsp_nofbk_alloc(void);

/*	
*	Function name:
*		aipc_ctrl_2dsp_fbk_alloc
*	Description:
*		Get buffer need feedback ready to send from CPU to DSP
*		This function is process context
*	Parameters:
*		None
*	Return:
*		Control buffer address
*	Comments:
*		
*/
extern void *	aipc_ctrl_2dsp_fbk_alloc(void);

/*	
*	Function name:
*		aipc_ctrl_2dsp_send
*	Description:
*		Send control message to DSP
*		This function is process context
*	Parameters:
*		void *dp	: buffer address
*	Return:
*		OK	: success
*		NOK	: fail
*	Comments:
*		
*/
extern int		aipc_ctrl_2dsp_send(void *dp, aipc_scheduable schedulable);

/*	
*	Function name:
*		aipc_ctrl_2dsp_fbk_ret
*	Description:
*		Return control buffer to need feedback poll
*		This function is process context
*	Parameters:
*		void *dp	: buffer address
*	Return:
*		OK	: success
*		NOK	: fail
*/
extern int		aipc_ctrl_2dsp_fbk_ret(void *dp);

/**	DSP **/

/*	
*	Function name:
*		aipc_ctrl_2dsp_recv
*	Description:
*		Get buffer sent from CPU to DSP
*		This function is process context
*	Parameters:
*		None
*	Return:
*		Control buffer address
*	Comments:
*		
*/
extern void *	aipc_ctrl_2dsp_recv(void);

/*
*	Function name:
*		aipc_ctrl_2dsp_nofbk_ret
*	Description:
*		Return control buffer to no feedback poll
*		This function is process context
*	Parameters:
*		void *dp	: buffer address
*	Return:
*		OK	: success
*		NOK	: fail
*/
extern int		aipc_ctrl_2dsp_nofbk_ret(void *dp);

/*
*	Function name:
*		aipc_ctrl_2dsp_fbk_fin
*	Description:
*		Indicate this need feedback buffer is done
*		This function is process context
*	Parameters:
*		void *dp	: buffer address
*	Return:
*		OK	: success
*		NOK	: fail
*/
extern int		aipc_ctrl_2dsp_fbk_fin(void *dp);


/*
*	Event CPU<-DSP
*/
/**	CPU **/
/*	
*	Function name:
*		aipc_ctrl_2cpu_recv
*	Description:
*		Get event buffer sent from DSP to CPU
*		This function is process context
*	Parameters:
*		None
*	Return:
*		Control buffer address
*	Comments:
*		
*/
extern void *	aipc_ctrl_2cpu_recv(void);

/*	
*	Function name:
*		aipc_ctrl_2cpu_ret
*	Description:
*		Return control buffer to event poll
*		This function is process context
*	Parameters:
*		void *dp	: buffer address
*	Return:
*		OK	: success
*		NOK	: fail
*/
extern int		aipc_ctrl_2cpu_ret(void *dp);

/**	DSP **/
/*	
*	Function name:
*		aipc_ctrl_2cpu_alloc
*	Description:
*		Get Event buffer to send from DSP to CPU
*		This function is process context
*	Parameters:
*		None
*	Return:
*		Control buffer address
*	Comments:
*		
*/
extern void *	aipc_ctrl_2cpu_alloc(void);

/*	
*	Function name:
*		aipc_ctrl_2cpu_send
*	Description:
*		Send event message to DSP
*		This function is process context
*	Parameters:
*		void *dp	: buffer address
*	Return:
*		OK	: success
*		NOK	: fail
*	Comments:
*		
*/
extern int		aipc_ctrl_2cpu_send(void *dp);

/*****************************************************************************
*   DSL IPC
*****************************************************************************/
//===========================================================================
/** DSL Event **/
#define DSL_EVENT_IDX(_p)  ((((unsigned long)&(DSL_EVENT_SHM_EVENT) <= (unsigned long)_p) && 									\
							 ((unsigned long)_p <= (unsigned long)&(DSL_EVENT_SHM_EVENT[DSL_EVENT_QUEUE_TOTAL-1]))) 			\
							? ((unsigned long)_p - (unsigned long)&(DSL_EVENT_SHM_EVENT))/sizeof(aipc_dsl_event_element_t) : NOK)

//===========================================================================

#endif
