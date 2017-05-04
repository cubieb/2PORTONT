/*
 *-----------------------------------------------------------------------------
 *                                                                             
 *               @@@@@@@      *   @@@@@@@@@           *                                     
 *              @       @             @               *                            
 *              @             *       @      ****     *                                 
 *               @@@@@@@      *       @     *    *    *                              
 *        ___---        @     *       @     ******    *                                 
 *  ___---      @       @     *       @     *         *                             
 *   -_          @@@@@@@  _   *       @      ****     *                               
 *     -_                 _ -                                                     
 *       -_          _ -                                                       
 *         -_   _ -        s   e   m   i   c   o   n   d   u   c   t   o   r 
 *           -                                                                    
 *                                                                              
 * (C) Copyright SiTel Semiconductor BV, unpublished work.
 * This computer program includes Confidential, Proprietary Information and
 * is a Trade Secret of SiTel Semiconductor BV.
 * All use, disclosure, and/or reproduction is prohibited unless authorized
 * in writing. All Rights Reserved.
 *
 *-----------------------------------------------------------------------------
 * File:		sc1445x_mcu_scheduler.h
 * Purpose:		
 * Created:		Aug 2008
 * By:			YP
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef sc1445x_mcu__SCHEDULER
#define sc1445x_mcu__SCHEDULER
#include "sc1445x_amb_api.h"
#include "sc1445x_mcu_api.h"
#include "sc1445x_mcu_block.h"


#define MCU_MAX_INSTANCES			15//sos

typedef struct _sc1445x_mcu_sceduler_t
{
	int participants;
	sc1445x_mcu_instance_t *mcuInstances[MCU_MAX_INSTANCES];
}sc1445x_mcu_scheduler_t; 

void sc1445x_mcu_scheduler_init(void);
void sc1445x_mcu_scheduler_destroy(void);
 
/// MCU InterProcess Communication Mechanishm
#define MCU_IPC_COMMAND_SIZE							16
typedef enum _sc1445x_mcu_ipc_commands
{
	 MCU_IPC_EXIT ,
	 MCU_IPC_ACTIVATE_INSTANCE ,
	 MCU_IPC_DEACTIVATE_INSTANCE, 
	 MCU_IPC_SUSPEND_INSTANCE ,
	 MCU_IPC_RESUME_INSTANCE,
	 MCU_IPC_CHANGE_CODEC,
	 MCU_IPC_SETCONFERENCE,
	 MCU_IPC_START_LOOPBACK,
	 MCU_IPC_STOP_LOOPBACK,
	 MCU_IPC_RESYNC, 
	 MCU_IPC_RESET_INSTANCE
}sc1445x_mcu_ipc_commands;
/*#define MCU_IPC_EXIT									101
#define MCU_IPC_ACTIVATE_INSTANCE						102
#define MCU_IPC_DEACTIVATE_INSTANCE						103
#define MCU_IPC_SUSPEND_INSTANCE						104
#define MCU_IPC_RESUME_INSTANCE							105

#define MCU_IPC_SETCONFERENCE							106
#define MCU_IPC_SETLOOPBACK								110
*/
typedef struct __mcu_ipc_command_t
{
	int cmd;
	int variable;
}mcu_ipc_command_t;

int sc1445x_mcu_ipc_init(void);
void sc1445x_mcu_ipc_destroy(void);
int sc1445x_mcu_ipc_write_command(int cmdId, int variable);
int sc1445x_mcu_ipc_read_command(mcu_ipc_command_t *pCmd );
// MCU AUDIO BUFFER BRIDGE

int sc1445x_mcu_scheduler_add(sc1445x_mcu_instance_t *pInstance);
void sc1445x_mcu_scheduler_protected_add(sc1445x_mcu_instance_t *pInstance);

int sc1445x_mcu_scheduler_del(sc1445x_mcu_instance_t *pInstance);//
int sc1445x_mcu_scheduler_suspend(sc1445x_mcu_instance_t *pInstance);
int sc1445x_mcu_scheduler_resume(sc1445x_mcu_instance_t *pInstance);
int sc1445x_mcu_scheduler_protected_del(sc1445x_mcu_instance_t *pInstance);//
int sc1445x_mcu_scheduler_protected_suspend(sc1445x_mcu_instance_t *pInstance);
int sc1445x_mcu_scheduler_protected_resume(sc1445x_mcu_instance_t *pInstance);
int  sc1445x_mcu_scheduler_resetInstance(sc1445x_mcu_instance_t *pMcuInstance); 
int sc1445x_mcu_scheduler_protected_resetInstance(sc1445x_mcu_instance_t *pMcuInstance)  ;
int sc1445x_mcu_scheduler_changeCodec(sc1445x_mcu_instance_t *pMcuInstance) ;
int sc1445x_mcu_scheduler_protected_changeCodec(sc1445x_mcu_instance_t *pMcuInstance);  
int  sc1445x_mcu_scheduler_resync(void *rtpSession);  
int  sc1445x_mcu_scheduler_protected_resync(void *rtpSession);  
int sc1445x_mcu_scheduler_instance_close(sc1445x_mcu_instance_t *pInstance);
void sc1445x_mcu_scheduler_instance_free(sc1445x_mcu_instance_t *pMcuInstance);

int sc1445x_mcu_scheduler_add_loop(sc1445x_mcu_instance_t *pInstance);
void sc1445x_mcu_scheduler_protected_add_loop(sc1445x_mcu_instance_t *pInstance);
int sc1445x_mcu_scheduler_del_loop(sc1445x_mcu_instance_t *pInstance);//
int sc1445x_mcu_scheduler_protected_del_loop(sc1445x_mcu_instance_t *pInstance);//

int sc1445x_mcu_scheduler_start_thread(void);
int sc1445x_mcu_scheduler_stop_thread(void);
void sc1445x_mcu_scheduler_thread_stopped(void);
int mcu_select(int maxWaitTime,int fd1,int fd2,int fd3); 

int sc1445x_mcu_thread(void * arg);
 
void mcu_bridge(int);
int sc1445x_mcu_soundloopback_read(int size);
int sc1445x_mcu_soundloopback_write(int size);
int sc1445x_mcu_read_packet_from_alsa( void );
int sc1445x_mcu_send_packet_to_rtp(void);
int sc1445x_mcu_read_packet_from_rtp(void);
int sc1445x_mcu_write_packet_to_alsa(void);
#ifdef REMOTE_LOOPBACK_TEST
 	void sc1445x_mcu_remote_loopback(void); 			
#endif
#endif //sc1445x_mcu__SCHEDULER
