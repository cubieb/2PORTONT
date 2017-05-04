
/*-----------------------------------------------------------------------------
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
 * (C) Copyright SiTel Semiconductor BV, unpublished work.file: 

 * This computer program includes Confidential, Proprietary Information and
 * is a Trade Secret of SiTel Semiconductor BV.
 * All use, disclosure, and/or reproduction is prohibited unless authorized
 * in writing. All Rights Reserved.
 *
 *-----------------------------------------------------------------------------
 * File:		 		 sc1445x_mcu_scheduler.c
 * Purpose:		 		 
 * Created:		 		 Aug 2008
 * By:		 		 	 YP
 * Country:		 		 Greece
 *
 *-----------------------------------------------------------------------------
 */
/*========================== Include files ==================================*/
#include <ctype.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <values.h>    
#include <sys/time.h>
  
#include "sc1445x_media.h"
#include "sc1445x_ortp.h" 
#ifdef SRTP_ENABLED
	#include "sc1445x_srtp.h" 
#endif

#include "sc1445x_mcu_debug.h"
#include "sc1445x_amb_api.h"
#include "sc1445x_mcu_api_defs.h" 
#include "sc1445x_mcu_scheduler.h"
#include <ortp/rtpsession.h>


#ifdef SC1445x_AE_SUPPORT_FAX
	#include "../sc1445x_src/SiATA/si_ata.h"
	#include "sc1445x_t38layer.h"
	unsigned char t38InDataBuffer[1024];
	unsigned char t38OutDataBuffer[1024];
	T38Obj_t T38Object;
	T38Packet_t T38InPacket;
	T38Packet_t T38OutPacket;	
	unsigned char pcm_out[160];
 	int T38PortId=0;  
#endif
/*
	typedef struct 
	{
	  unsigned short  *src_ptr;             // input PCM buffer pointer. From PCM  
	  T38Packet_t     *fax_packetIn_ptr;    // Input fax packet pointer. From IP network  
	  unsigned short  *dst_ptr;             // output PCM buffer pointer. To PCM     
	  T38Packet_t     *fax_packetOut_ptr;   // output fax packet pointer. To IP network 
	} T38Obj_t;
*/

////////////////////////////////////////////////////////////////////////////////////////////////
// MCU SCHEDULER
////////////////////////////////////////////////////////////////////////////////////////////////
 sc1445x_mcu_scheduler_t sc1445x_mcu_scheduler;
int start_thread = 0;
pthread_mutex_t McuProtectMutex;                                               
 
void sc1445x_mcu_scheduler_init(void)
{
	memset(&sc1445x_mcu_scheduler,0,sizeof(sc1445x_mcu_scheduler_t));
	sc1445x_mcu_ipc_init();
}
void sc1445x_mcu_scheduler_destroy(void)
{
 	sc1445x_mcu_ipc_destroy();
}
///////////////////////////////////////////////////////////////////////////////////////////////
/// MCU InterProcess Communication Mechanishm
#define MCU_PIPE_READ_FD  0
#define MCU_PIPE_WRITE_FD 1


static int mcu_ipc_created = 0;
static int mcu_ipc_pipe[2];
int sc1445x_mcu_ipc_init(void)
{
 	if (mcu_ipc_created) return 0;
	if (pipe(mcu_ipc_pipe)<0) return -1;
  
	mcu_ipc_created=1;
	return 0;
}

void sc1445x_mcu_ipc_destroy(void)
{
	if (!mcu_ipc_created) return ;
	 close(mcu_ipc_pipe[MCU_PIPE_READ_FD]); 
	 close(mcu_ipc_pipe[MCU_PIPE_WRITE_FD]); 
	 mcu_ipc_created=0;
}

int sc1445x_mcu_ipc_write_command(int cmdId, int variable)
{
	mcu_ipc_command_t cmd;
	cmd.cmd = cmdId;
	cmd.variable = variable;
	return write(mcu_ipc_pipe[MCU_PIPE_WRITE_FD], (unsigned char*)&cmd,sizeof(mcu_ipc_command_t)); 
}
  
int sc1445x_mcu_ipc_read_command(mcu_ipc_command_t *pCmd )
{
	mcu_ipc_command_t cCmd;
 	int count;
	if (!mcu_ipc_created) { return 0;}
	count = read(mcu_ipc_pipe[MCU_PIPE_READ_FD], (unsigned char*)&cCmd,sizeof(mcu_ipc_command_t));
  	if (count){
 		pCmd->cmd =cCmd.cmd ;
		pCmd->variable = cCmd.variable;
	}
  	return count;
 }
void sc1445x_mcu_set_alsa_fd(void);
void sc1445x_mcu_zero_alsa_fd(void);

#define MCU_ALSA_READ_FD  0
#define MCU_ALSA_WRITE_FD 1
int alsaFD[2];   

 void sc1445x_mcu_set_alsa_fd(void)
 {
	 alsaFD[MCU_ALSA_READ_FD] = sc1445x_amb_get_capture_fd( );
	 alsaFD[MCU_ALSA_WRITE_FD] = sc1445x_amb_get_playback_fd( );
 }
 //add 30 June 2009
 void sc1445x_mcu_zero_alsa_fd(void)
 {
	 alsaFD[MCU_ALSA_READ_FD] = -1;
	 alsaFD[MCU_ALSA_WRITE_FD] = -1;
 }
///////////////////////////////////////////////////////////////////////////////////////////////
// MCU AUDIO BUFFER BRIDGE
int sc1445x_mcu_scheduler_add(sc1445x_mcu_instance_t *pInstance)
{
	int i, ret;
  	for (i=0;i<MCU_MAX_INSTANCES;i++)
	{	
		if (sc1445x_mcu_scheduler.mcuInstances[i] == pInstance)
		{
  			break;
		}
	}
	//fix 30 June 2009
 	sc1445x_mcu_set_alsa_fd(); 
  	ret= sc1445x_mcu_ipc_write_command(MCU_IPC_ACTIVATE_INSTANCE, (int)pInstance);
 	if ( start_thread) return 0;

 	if (sc1445x_mcu_scheduler_start_thread()<0)
	{
		DPRINT("[%s] Fatal Error Unable to start MCU thread \n", __FUNCTION__);//fix April 28, 2009
		return -1;
	}
    PRINTINFO("[%s] thread status=%d\n", __FUNCTION__,start_thread); 
	start_thread=1;
 	return 0;
}

void  sc1445x_mcu_scheduler_protected_add(sc1445x_mcu_instance_t *pInstance)
{
	int i;
	for (i=0;i<MCU_MAX_INSTANCES;i++)
	{	
 		if (sc1445x_mcu_scheduler.mcuInstances[i]==pInstance) 
    			return ;
 	}
  	for (i=0;i<MCU_MAX_INSTANCES;i++)
	{	
 		if (sc1445x_mcu_scheduler.mcuInstances[i]==NULL){
   			sc1445x_mcu_scheduler.mcuInstances[i] = pInstance;
			 
			sc1445x_mcu_scheduler.participants++;
   			break;
  		}
	}
}
int sc1445x_mcu_scheduler_del(sc1445x_mcu_instance_t *pInstance)
{
	int i;
	//fix 18 June, 2009
 	PRINTINFO("[%s] send mcu instance release notification to the scheduller [%x]\n ", __FUNCTION__, (int)pInstance); 
 	for (i=0;i<MCU_MAX_INSTANCES;i++)
	{	
		if (sc1445x_mcu_scheduler.mcuInstances[i]==pInstance){
		    sc1445x_mcu_ipc_write_command(MCU_IPC_DEACTIVATE_INSTANCE, (int)pInstance);
			return 0;
 		}
	}
	PRINTINFO("[%s] mcu instance not found [%x]\n ", __FUNCTION__, (int)pInstance); 
  	return -1;
}

int sc1445x_mcu_scheduler_protected_del(sc1445x_mcu_instance_t *pInstance)
{
	int i;
	int found =0;
  	PRINTINFO("[%s]Delete mcu instance  [%x]\n ", __FUNCTION__, (int)pInstance); 

	for (i=0;i<MCU_MAX_INSTANCES;i++)
	{	
		if (sc1445x_mcu_scheduler.mcuInstances[i]==pInstance){
			// MUTEX LOCK
			found=1;
    		sc1445x_mcu_scheduler.mcuInstances[i] =0;
			sc1445x_mcu_scheduler.participants--;
			if (sc1445x_mcu_scheduler.participants<0) 
				sc1445x_mcu_scheduler.participants=0;
 	 		// MUTEX UNLOCK
 			break;
		}
	}
  	

	if (!found) { 
		DPRINT("[%s] Invalid Intance to be deleted \n ", __FUNCTION__);
		return -1;
	}
  	// MUTEX UNLOCK
 	// deactivate audio channel
	if (pInstance->mediaStream.mediaFormat ==MCU_MFMT_T38_ID)
	{
		sc1445x_amb_switch_to_voice(pInstance->alsaChannel);
	}
  	if (pInstance->state & (MCU_CALL_STATE_CONNECTED_RX | MCU_CALL_STATE_CONNECTED_TX) )
	{
		if (!sc1445x_amb_deactivate_channel(pInstance->alsaChannel, pInstance->mediaStream.mediaParams.fxsLine))
			pInstance->alsaChannel=-1; 
	}

   	// close rtp connection
	if (pInstance->mediaStream.mediaFormat ==MCU_MFMT_RTPAVP_ID)
	{
#ifdef SRTP_ENABLED
		if (pInstance->pSrtpSession)
			srtp_destroy_session(pInstance->pSrtpSession );
		pInstance->pSrtpSession= NULL;
#endif
		//Fix May 14/05/2009
		if (pInstance->pRtpSession)
			sc1445x_ortp_stream_close(pInstance->pRtpSession);

	}

#ifdef SC1445x_AE_SUPPORT_FAX 
 	if (pInstance->mediaStream.mediaFormat ==MCU_MFMT_T38_ID)
		sc1445x_t38_stream_close();
#endif
 	//changed
	pInstance->state = MCU_CALL_STATE_IDLE;
  	if (sc1445x_mcu_scheduler.participants==0)
	{
 		sc1445x_amb_sound_close();
  	}
  	PRINTINFO("[%s]Mcu instance  has been deleted [%x][%d]\n ", __FUNCTION__, (int)pInstance,sc1445x_mcu_scheduler.participants); 
    return sc1445x_mcu_scheduler.participants;
}
int sc1445x_mcu_scheduler_instance_close(sc1445x_mcu_instance_t *pInstance)
{
  PRINTINFO("[%s]Flash mcu instance  [%x]\n ", __FUNCTION__, (int)pInstance); 
  	// deactivate audio channel
	if (pInstance->mediaStream.mediaFormat ==MCU_MFMT_T38_ID)
	{
		return -1;
	}
  	if (pInstance->state & (MCU_CALL_STATE_CONNECTED_RX | MCU_CALL_STATE_CONNECTED_TX) )
	{
		if (!sc1445x_amb_deactivate_channel(pInstance->alsaChannel, pInstance->mediaStream.mediaParams.fxsLine))
		  pInstance->alsaChannel=-1; 
	}

   	// close rtp connection
	if (pInstance->mediaStream.mediaFormat ==MCU_MFMT_RTPAVP_ID)
	{
#ifdef SRTP_ENABLED
		if (pInstance->pSrtpSession)
			srtp_destroy_session(pInstance->pSrtpSession);
		pInstance->pSrtpSession= NULL;
#endif
		//Fix May 14/05/2009
		if (pInstance->pRtpSession)
			sc1445x_ortp_stream_close(pInstance->pRtpSession);
 	}
  return 0;
}

void sc1445x_mcu_scheduler_instance_free(sc1445x_mcu_instance_t *pInstance)
{
	int i;
 	for (i=0;i<MCU_MAX_INSTANCES;i++)
	{	
		if (sc1445x_mcu_scheduler.mcuInstances[i]==pInstance){
  			pInstance->state =(int) MCU_CALL_STATE_IDLE;  
   			break ;
		}
	}
 }

 int sc1445x_mcu_scheduler_suspend(sc1445x_mcu_instance_t *pInstance)
{
	return  sc1445x_mcu_ipc_write_command(MCU_IPC_SUSPEND_INSTANCE, (int)pInstance);

}

int sc1445x_mcu_scheduler_protected_suspend(sc1445x_mcu_instance_t *pInstance)
{
	int i;

  for (i=0;i<MCU_MAX_INSTANCES;i++)
	{	
		if (sc1445x_mcu_scheduler.mcuInstances[i]==pInstance){
   	 		if (!sc1445x_amb_deactivate_channel(pInstance->alsaChannel, pInstance->mediaStream.mediaParams.fxsLine))
			{
 				pInstance->alsaChannel=-1; 
			}
			pInstance->state =(int) MCU_CALL_STATE_CONNECTED; //TBS to support rx only, tx only
 		}
		#ifdef CLOSE_ALSA_ON_CHANNELS_DEACTIVATION
			else{//add 30 June 2009
				sc1445x_mcu_instance_t *pInst= sc1445x_mcu_scheduler.mcuInstances[i];	
				if (pInst &&  (pInst->alsaChannel>=0  && pInst->alsaChannel<=2))
				 found =1;
  			}
		#endif
	}
#ifdef CLOSE_ALSA_ON_CHANNELS_DEACTIVATION
	//add 30 June 2009
	if (!found)
	{
	 	sc1445x_amb_sound_close();
		sc1445x_mcu_zero_alsa_fd();
	}
#endif
	return 0;
}
int sc1445x_mcu_scheduler_resume(sc1445x_mcu_instance_t *pInstance)
{
	return  sc1445x_mcu_ipc_write_command(MCU_IPC_RESUME_INSTANCE, (int)pInstance);	

}
int sc1445x_mcu_scheduler_protected_resume(sc1445x_mcu_instance_t *pInstance)
{
	int i;
  
	for (i=0;i<MCU_MAX_INSTANCES;i++)
	{	
		if (sc1445x_mcu_scheduler.mcuInstances[i]==pInstance){
#ifdef CLOSE_ALSA_ON_CHANNELS_DEACTIVATION
			// add 30 June 2009
 	 		if (sc1445x_amb_sound_open(8,0,ALSA_MAX_BYTES*100))
 				return -1;
 			sc1445x_mcu_set_alsa_fd();
#endif
	 
  		    sc1445x_mcu_audio_channel_activation(pInstance);

 			//	pInstance->state =(int) MCU_CALL_STATE_CONNECTED_RX_TX; 
 			if (pInstance->mediaStream.mediaAttr == MCU_MATTR_SND_ID)
				pInstance->state =(int) MCU_CALL_STATE_CONNECTED_TX;  
 			else if (pInstance->mediaStream.mediaAttr == MCU_MATTR_RCV_ID)
				pInstance->state =(int) MCU_CALL_STATE_CONNECTED_RX;  
			else pInstance->state =(int) MCU_CALL_STATE_CONNECTED_RX_TX;  

			if (pInstance->pRtpSession){
				//Fix 30 June 2009
				rtp_session_flush_sockets((RtpSession*)pInstance->pRtpSession );
   				rtp_session_resync((RtpSession*)pInstance->pRtpSession );
				 // pInstance->audioFramesBuffer.rxTimestamp =0; 		
			}
  			return 0;
		}
	}
 	return -1;
}

int  sc1445x_mcu_scheduler_resetInstance(sc1445x_mcu_instance_t *pMcuInstance)  
{
	return  sc1445x_mcu_ipc_write_command(MCU_IPC_RESET_INSTANCE, (int)pMcuInstance);
}

int sc1445x_mcu_scheduler_protected_resetInstance(sc1445x_mcu_instance_t *pMcuInstance)  
{
	int ret;
	PRINTINFO("[%s] reseting mcu instance [%x]\n ", __FUNCTION__, (int) pMcuInstance);
 	
 	//fix 19 June, 2009
	pthread_mutex_lock (&McuProtectMutex);
    ret = sc1445x_mcu_scheduler_instance_close(pMcuInstance);
 	//fix 23 June, 2009
	if (ret==-1) {
		sc1445x_mcu_scheduler_instance_free(pMcuInstance);
		sc1445x_mcu_terminate(pMcuInstance); 
 		pthread_mutex_unlock (&McuProtectMutex);
		return -1;
	}
  	pMcuInstance->mediaStream.mediaAttr   = MCU_MATTR_BOTH_ID;
	//fix 23 June, 2009
 	ret = sc1445x_mcu_scheduler_instance_reopen(pMcuInstance,&pMcuInstance->mediaStream);

	//fix 23 June, 2009
	if (ret==-1) {
		sc1445x_mcu_scheduler_instance_free(pMcuInstance);
		sc1445x_mcu_terminate(pMcuInstance); 
		pthread_mutex_unlock (&McuProtectMutex);return -1;
	}
  	pthread_mutex_unlock (&McuProtectMutex);
	PRINTINFO("[%s] mcu instance has been activated again [%x]\n ", __FUNCTION__, (int) pMcuInstance);
 	return 0;
}

int  sc1445x_mcu_scheduler_changeCodec(sc1445x_mcu_instance_t *pMcuInstance)  
{
 
 		return  sc1445x_mcu_ipc_write_command(MCU_IPC_CHANGE_CODEC, (int)pMcuInstance);
}

int  sc1445x_mcu_scheduler_protected_changeCodec(sc1445x_mcu_instance_t *pMcuInstance)  
{
	int ret;
	 
	ret = rtp_session_set_payload_type(pMcuInstance->pRtpSession,pMcuInstance->mediaStream.mediaParams.rtp_ptype);
	if (ret<0) 
		DPRINT("[%s] FAILED TO SET PAYLOAD TYPE \n ", __FUNCTION__);

 	if (!sc1445x_amb_deactivate_channel(pMcuInstance->alsaChannel, pMcuInstance->mediaStream.mediaParams.fxsLine))
		pMcuInstance->alsaChannel=-1; 

	  
	sc1445x_mcu_audio_channel_activation(pMcuInstance);
 	return 0; 
}

// New Function: Added 15 April 2009
int  sc1445x_mcu_scheduler_resync(void *rtpSession)  
{
  		return  sc1445x_mcu_ipc_write_command(MCU_IPC_RESYNC, (int)rtpSession);
}
// New Function: Added 15 April 2009
int  sc1445x_mcu_scheduler_protected_resync(void *rtpSession)  
{
		if (rtpSession)
	 	   rtp_session_resync((RtpSession *)rtpSession);
	 return 1;
}

int sc1445x_mcu_scheduler_add_loop(sc1445x_mcu_instance_t *pInstance)
{
	int i, ret;
 
	for (i=0;i<MCU_MAX_INSTANCES;i++)
	{	
		if (sc1445x_mcu_scheduler.mcuInstances[i] == pInstance)
			break;
	}
 
 	ret= sc1445x_mcu_ipc_write_command(MCU_IPC_START_LOOPBACK, (int)pInstance);
  
	if ( start_thread) return 0;
	if (sc1445x_mcu_scheduler_start_thread()<0)
		return -1;

	 start_thread = 1;
	return 0;
}

void  sc1445x_mcu_scheduler_protected_add_loop(sc1445x_mcu_instance_t *pInstance)
{
	int i;
  	for (i=0;i<MCU_MAX_INSTANCES;i++)
	{	
 		if (sc1445x_mcu_scheduler.mcuInstances[i]==NULL){
  			sc1445x_mcu_scheduler.mcuInstances[i] = pInstance;
 
			sc1445x_mcu_scheduler.participants++;
  			break;
  		}
	}
	 
}
int sc1445x_mcu_scheduler_del_loop(sc1445x_mcu_instance_t *pInstance)
{
	int i;
	//fix 18 June, 2009
	PRINTINFO("[%s] send mcu instance release notification [%x]\n ", __FUNCTION__, (int)pInstance); 
 	for (i=0;i<MCU_MAX_INSTANCES;i++)
	{	
		if (sc1445x_mcu_scheduler.mcuInstances[i]==pInstance){
		    sc1445x_mcu_ipc_write_command(MCU_IPC_DEACTIVATE_INSTANCE, (int)pInstance);
			return 0;
 		}
	}
	PRINTINFO("[%s] mcu instance not found [%x]\n ", __FUNCTION__, (int)pInstance); 
  	return -1;
}

int sc1445x_mcu_scheduler_protected_del_loop(sc1445x_mcu_instance_t *pInstance)
{
	int i;
	int found =0;
 
 	for (i=0;i<MCU_MAX_INSTANCES;i++)
	{	
		if (sc1445x_mcu_scheduler.mcuInstances[i]==pInstance){
			// MUTEX LOCK
			found=1;
 			sc1445x_mcu_scheduler.mcuInstances[i] = NULL;
			sc1445x_mcu_scheduler.participants--;
	 		// MUTEX UNLOCK
 			break;
		}
	}
	if (!found) 
		DPRINT("[%s] Invalid Instance to be deleted \n ");
  	// MUTEX UNLOCK
 	// deactivate audio channel
    if (!sc1445x_amb_deactivate_channel(pInstance->alsaChannel, pInstance->mediaStream.mediaParams.fxsLine))
		pInstance->alsaChannel=-1; 

  	// close rtp connection
	sc1445x_ortp_stream_close(pInstance->pRtpSession);
 	if (sc1445x_mcu_scheduler.participants==0)
	{
  		sc1445x_amb_sound_close();
  	}
 
  	return sc1445x_mcu_scheduler.participants;
}

static pthread_t Scheduler_Thread;

int sc1445x_mcu_scheduler_start_thread(void)
{
	int ret;
	sc1445x_mcu_set_alsa_fd();
	ret = pthread_create (&Scheduler_Thread, NULL, sc1445x_mcu_thread, NULL);
	pthread_detach( Scheduler_Thread ) ;
 
	return ret;
}
int sc1445x_mcu_scheduler_stop_thread(void)
{
	return sc1445x_mcu_ipc_write_command(MCU_IPC_EXIT, 0);
}

void sc1445x_mcu_scheduler_thread_stopped(void)
{
	 alsaFD[MCU_ALSA_READ_FD] = -1;
	 alsaFD[MCU_ALSA_WRITE_FD] = -1;
	 start_thread = 0;
}

#define MCU_ALSA_READ_FD_ISACTIVATED 0x01
#define MCU_PIPE_READ_FD_ISACTIVATED 0x02
#define MCU_RTP_READ_FD_ISACTIVATED  0x04
#define MCU_TIMEOUT_ISACTIVATED		 0x08
 
int mcu_select(int maxWaitTime, int fd1,int fd2,int fd3) 
{
 	struct timeval tv;
	fd_set readFD;
 	int maxFD =0;
	int ret,i, fdi;
	int fd_instances[5];
//_start:
	maxFD =0;
	for (i=0;i<5;i++) 	fd_instances[i]=(int)0;
	 fdi=0;
	FD_ZERO(&readFD);

	if (fd1>0){
		FD_SET(fd1, &readFD);
		fd_instances[fdi]=fd1;
 
		fdi++;
	}
	if (fd2>0){
			FD_SET(fd2, &readFD);
			fd_instances[fdi]=fd2;
 			fdi++;
 	}
	if (fd3>0){
			FD_SET(fd3, &readFD);
			fd_instances[fdi]=fd3;
 			fdi++;
 	}
   	FD_SET(mcu_ipc_pipe[MCU_PIPE_READ_FD], &readFD);
	fd_instances[fdi]=mcu_ipc_pipe[MCU_PIPE_READ_FD];
 	fdi++;  

	if (alsaFD[MCU_ALSA_READ_FD] != -1){
 		FD_SET(alsaFD[MCU_ALSA_READ_FD], &readFD);
		fd_instances[fdi]=alsaFD[MCU_ALSA_READ_FD];
 		fdi++;  
 	}
 
	maxFD=fd_instances[0];
	for (i=1;i<5;i++) 	{
 		if (maxFD<fd_instances[i]) maxFD=fd_instances[i];
	}
  
  	tv.tv_sec = 0;  
	tv.tv_usec = maxWaitTime;

	ret = select(maxFD+1, &readFD ,NULL, NULL, &tv);
 	 
   	if (ret < 0 && ret!=EINTR)  
	{
  		ret=0;  
		return ret;
 	}
   	ret = 0; 
 	if (FD_ISSET( fd1, &readFD))  
		ret|=MCU_RTP_READ_FD_ISACTIVATED;  
 	if (FD_ISSET( fd2, &readFD))  
		ret|=MCU_RTP_READ_FD_ISACTIVATED;  
 	if (FD_ISSET( fd3, &readFD))  
		ret|=MCU_RTP_READ_FD_ISACTIVATED;  
   	 if (FD_ISSET( alsaFD[MCU_ALSA_READ_FD], &readFD))  
		ret|=MCU_ALSA_READ_FD_ISACTIVATED;  
  	if (FD_ISSET( mcu_ipc_pipe[MCU_PIPE_READ_FD], &readFD))  
 		ret |= MCU_PIPE_READ_FD_ISACTIVATED;
   
 	   return ret;
}
 
// Main Session Thread
#define MCU_THREAD_MAX_SLEEP_INTERVAL 10000
int socketfd[MCU_MAX_INSTANCES];
 
int bridgetimes=0;
unsigned char AlsaSamples[1024];
unsigned char WriteToAlsaSamples[1024];

int sc1445x_mcu_thread(void * arg)
{
	int i;
	mcu_ipc_command_t m_SoundCommand; 
	struct _RtpSession *pSess;
	int soundloopback = 0;
	sc1445x_mcu_instance_t *pInstance;
  	int ret;

	memset(WriteToAlsaSamples, 0,ALSA_MAX_BYTES);
	memset(AlsaSamples, 0,ALSA_MAX_BYTES);
	for (;;)  
	{
  		// get an active input or sleep for max interval
   		 for (i=0;i<MCU_MAX_INSTANCES;i++)
		{
			pInstance = sc1445x_mcu_scheduler.mcuInstances[i];
  			if (pInstance)
			{
				pSess =(struct _RtpSession*)pInstance->pRtpSession;
			 	socketfd[i]=pSess->rtp.socket;
  			}else socketfd[i]=0;
			socketfd[i]=0;
   		}  
     	ret = mcu_select((int)MCU_THREAD_MAX_SLEEP_INTERVAL,(int) socketfd[0], (int)socketfd[1], (int)socketfd[2]); 
		if (!ret) continue;
     		//	sound session API - checks for new commands from the User Agent or MMI
		//	e.g call progress tones
    	if (ret & MCU_PIPE_READ_FD_ISACTIVATED)	
		{
   				if (sc1445x_mcu_ipc_read_command(&m_SoundCommand))
				{
					 
					switch (m_SoundCommand.cmd)
					{
						case MCU_IPC_EXIT:
 							sc1445x_mcu_scheduler_thread_stopped();
   							return (EXIT_SUCCESS);	
 						case MCU_IPC_SUSPEND_INSTANCE:
  							  sc1445x_mcu_scheduler_protected_suspend((sc1445x_mcu_instance_t *)m_SoundCommand.variable);
							break;
 						case MCU_IPC_RESUME_INSTANCE:
  							   sc1445x_mcu_scheduler_protected_resume((sc1445x_mcu_instance_t *)m_SoundCommand.variable);
								//rtp_session_resync(((sc1445x_mcu_instance_t *)m_SoundCommand.variable)->pRtpSession);
							break;
 						case MCU_IPC_ACTIVATE_INSTANCE:
   							sc1445x_mcu_scheduler_protected_add((sc1445x_mcu_instance_t *)m_SoundCommand.variable);
							pthread_mutex_unlock (&McuProtectMutex);
  			 				break;
 						case MCU_IPC_DEACTIVATE_INSTANCE:
 							//pthread_mutex_lock (&McuProtectMutex);
  							ret = sc1445x_mcu_scheduler_protected_del((sc1445x_mcu_instance_t *)m_SoundCommand.variable);
							if (ret==0)
							{
 								sc1445x_mcu_scheduler_thread_stopped();
		 						pthread_mutex_unlock (&McuProtectMutex);
  								return (EXIT_SUCCESS);	
 							}
		 					pthread_mutex_unlock (&McuProtectMutex);
							break;
						case	MCU_IPC_RESET_INSTANCE:
							sc1445x_mcu_scheduler_protected_resetInstance((sc1445x_mcu_instance_t *)m_SoundCommand.variable);
							break;
						case	MCU_IPC_CHANGE_CODEC:
							sc1445x_mcu_scheduler_protected_changeCodec((sc1445x_mcu_instance_t *)m_SoundCommand.variable);
							break;
							// New option: Added 15 April 2009
						case	MCU_IPC_RESYNC:
							sc1445x_mcu_scheduler_protected_resync((void *)m_SoundCommand.variable)  ;
							break;
 				 		case MCU_IPC_START_LOOPBACK:
							soundloopback = (int)1;  
 							sc1445x_mcu_scheduler_protected_add_loop((sc1445x_mcu_instance_t *)m_SoundCommand.variable);
 							break;
 				 		case MCU_IPC_STOP_LOOPBACK:
							soundloopback = (int)0;  
 							ret = sc1445x_mcu_scheduler_protected_del_loop((sc1445x_mcu_instance_t *)m_SoundCommand.variable);
							if (ret==0)
							{
								sc1445x_mcu_scheduler_thread_stopped();
								return 0;
 							}
  							break;
						case MCU_IPC_SETCONFERENCE:
 							break;
  					}
					m_SoundCommand.cmd=0;m_SoundCommand.variable =0;
				}
 		 } 
		
   		//	Read Audio Samples from the RTP Layer and pass them to ALSA driver  
 		//	-Read Audio Packets from the ALSA driver and Send them to the network
		//	 build an audio packet for the current session (one or multiple calls)
		//	 based on the current value of ptime
		//	-check the audio packet and state of the current call/session
		//	 eg mark bit
		//	-send audio packets to rtp layer

 
      	if (!(ret & MCU_ALSA_READ_FD_ISACTIVATED)  /*&& !(ret & MCU_TIMEOUT_ISACTIVATED) */) 
		{
  			continue;
		} 
 
 		if ( soundloopback)
		{
   	 	 	ret = sc1445x_mcu_soundloopback_read(ALSA_MAX_BYTES);
   	 	 	sc1445x_mcu_soundloopback_write(ALSA_MAX_BYTES);
  			continue;
  		} 
	
 		mcu_bridge(ret);  
		#ifdef REMOTE_LOOPBACK_TEST  
 			sc1445x_mcu_remote_loopback();			
		#endif
  	}		 
}

void mcu_bridge(int flags)
{
	// Read an audio chunk and prepare an audio packet with ptime size
	// if ready, send it to RTP layer

	// T38
#ifdef SC1445x_AE_SUPPORT_FAX
	memset(&T38Object,0, sizeof(T38Obj_t));
#endif

	if ( flags & MCU_ALSA_READ_FD_ISACTIVATED)  
	{
		sc1445x_mcu_read_packet_from_alsa();
	}
// For FAX support 
#ifdef SC1445x_AE_SUPPORT_FAX
	t38Polling();
#endif	

	//-Receive Audio Packets from the RTP and Send them to the ALSA driver	
	sc1445x_mcu_read_packet_from_rtp();
	sc1445x_mcu_write_packet_to_alsa();

	if ( flags & MCU_ALSA_READ_FD_ISACTIVATED)  
	{
		sc1445x_mcu_send_packet_to_rtp( );
	}
}

int sc1445x_mcu_soundloopback_read(int size)
{
  return sc1445x_amb_sound_read((unsigned char*)AlsaSamples, size);
}

int sc1445x_mcu_soundloopback_write(int size)
{
  return sc1445x_amb_sound_write((unsigned char*)AlsaSamples, size);
}
 
int sc1445x_mcu_read_packet_from_alsa( void )
{
	sc1445x_mcu_instance_t *pInstance;
	int sampleSize;
	int activity;
	int ret ;
 	int pos;
	int found = 0 ;
	int i;
	int ch;
  	// read an audio chunk [NumOfChannel*(80 audio samples + 2 header) = n*82= ALSA_MAX_BYTES] from the alsa driver 
 	ret = sc1445x_amb_sound_read((unsigned char*)AlsaSamples, ALSA_MAX_BYTES);

    // check all active instances 
	// check if SID or IDLE or Sample
	for (i=0;i<MCU_MAX_INSTANCES;i++)
	{
		pInstance = sc1445x_mcu_scheduler.mcuInstances[i];
		if (!pInstance)  continue;
		
	 	// check only active mcu instances (active calls)
 		if (pInstance && (pInstance->mediaStream.mediaFormat ==MCU_MFMT_T38_ID))
		{
#ifdef SC1445x_AE_SUPPORT_FAX
      ch =(int) pInstance->alsaChannel;
 			// copy ALSA stream to T.38 PCM input buffer
      //T38Object.src_ptr=(unsigned short*)&AlsaSamples[2];
      T38Object.src_ptr = (unsigned short*)&AlsaSamples[(ch*82) + 2]; 
#endif
			continue;
		} 
 
		if (pInstance  && (pInstance->state & MCU_CALL_STATE_CONNECTED_TX)) 
		{
			ch =(int) pInstance->alsaChannel ;
 			if (ch<0  || ch>=MAX_MCU_ALLOWED_CHANNELS ) continue;
    		pos = pInstance->audioFramesBuffer.txAudioSamplesSize ;
  			sampleSize=(int)AlsaSamples[ch*82];
			activity = AlsaSamples[(ch*82)+1];
		//	if (activity && (pInstance->audioFramesBuffer.txTicks & 0x80000000))
		//		pInstance->audioFramesBuffer.txTicks = pInstance->audioFramesBuffer.txPacketization;
		//	else
	 		pInstance->audioFramesBuffer.txTicks++;
 			pInstance->audioFramesBuffer.txTimestamp+=80	;	
		 
  			switch (activity)
			{ 
				case SC1445x_CHANNEL_STATE_SILENCE: 
  					//pInstance->audioFramesBuffer.state= SC1445x_TALK_STATE_SILENCE;
   					if (pInstance->audioFramesBuffer.txTicks==pInstance->audioFramesBuffer.txPacketization)
					{
							// check if there are saved samples within the current ptime interval
							//  if yes then at the end of ptime interval, transmit it
 							if (pInstance->audioFramesBuffer.SidSize)
							{
 								//rtp_session_set_markbit(pInstance->pRtpSession,0) ;				  
								 ret = sc1445x_ortp_send(pInstance->pRtpSession ,pInstance->audioFramesBuffer.SID,(int)pInstance->audioFramesBuffer.SidSize, 
					 				pInstance->audioFramesBuffer.txTimestamp);
								 pInstance->audioFramesBuffer.SidSize=(unsigned char)0;
 							}
  							pInstance->audioFramesBuffer.txAudioPacketPos  = 0 ;
 							pInstance->audioFramesBuffer.txTicks=(unsigned char)0;
					}
 					pInstance->audioFramesBuffer.txStoredSamples =0;
					pInstance->audioFramesBuffer.txAudioSamplesSize =0;
					break;
				case SC1445x_CHANNEL_STATE_SID:
						//if (pInstance->audioFramesBuffer.state==SC1445x_TALK_STATE_SID) continue ;
						if (sampleSize)
						{
							memcpy(pInstance->audioFramesBuffer.SID,&AlsaSamples[(ch*80)+(2*(ch+1))], sampleSize);
							pInstance->audioFramesBuffer.SidSize = (unsigned char)sampleSize;
						} 

 						if (pInstance->mediaStream.mediaParams.ptype!=PH_MEDIA_G729_PAYLOAD)
  							ret = rtp_session_set_payload_type( pInstance->pRtpSession, PH_MEDIA_CN_PAYLOAD);

						if ((pInstance->audioFramesBuffer.txTicks==pInstance->audioFramesBuffer.txPacketization) /*|| (pInstance->audioFramesBuffer.txStoredSamples ==pInstance->audioFramesBuffer.txPacketization )*/ )
						{
							pInstance->audioFramesBuffer.txAudioPacketPos= 0;
							pInstance->audioFramesBuffer.txTicks=(unsigned char)0;

							if ( pInstance->audioFramesBuffer.txStoredSamples==1 && (pInstance->mediaStream.mediaParams.ptype==PH_MEDIA_G729_PAYLOAD))
							{

								 memcpy(&pInstance->audioFramesBuffer.audioPacket[pInstance->codecSampleSize], &AlsaSamples[(ch*80)+(2*(ch+1))],  sampleSize);
								 //rtp_session_set_markbit(pInstance->pRtpSession,0) ;				  
								 ret = sc1445x_ortp_send(pInstance->pRtpSession ,&pInstance->audioFramesBuffer.audioPacket[0], pInstance->codecSampleSize+sampleSize, 
									pInstance->audioFramesBuffer.txTimestamp );
							}else{
								if (pInstance->audioFramesBuffer.SidSize)
								{
									// pInstance->audioFramesBuffer.state= SC1445x_TALK_STATE_SID;
									// rtp_session_set_markbit(pInstance->pRtpSession,0) ;				  
									 ret = sc1445x_ortp_send(pInstance->pRtpSession ,pInstance->audioFramesBuffer.SID,(int)pInstance->audioFramesBuffer.SidSize, 
						 				pInstance->audioFramesBuffer.txTimestamp);
									 pInstance->audioFramesBuffer.SidSize=(unsigned char)0;
 								}
							}
   						} 
							 
						pInstance->audioFramesBuffer.txStoredSamples=0;
						pInstance->audioFramesBuffer.txAudioSamplesSize =0;
						pInstance->audioFramesBuffer.txAudioPacketPos= 0;
						//pInstance->audioFramesBuffer.state = SC1445x_CHANNEL_STATE_SID;
  					   break;
 				case SC1445x_CHANNEL_STATE_ACTIVE:	
  					//pInstance->audioFramesBuffer.state = SC1445x_CHANNEL_STATE_ACTIVE;
					pInstance->audioFramesBuffer.SidSize=(unsigned char)0;
   					if (sampleSize==pInstance->codecSampleSize) 
 					{
  	 					if ((pos+sampleSize)>(SC1445x_MAX_AUDIO_PACKET * pInstance->audioFramesBuffer.txPacketization)) 
						{
				 			pos=0;
 							pInstance->audioFramesBuffer.txAudioSamplesSize =0;
						}else
						{
 							memcpy (&pInstance->audioFramesBuffer.audioPacket[pos], &AlsaSamples[(ch*80)+(2*(ch+1))], sampleSize);
							pInstance->audioFramesBuffer.txAudioSamplesSize  +=  sampleSize;
						}
  						pInstance->audioFramesBuffer.txStoredSamples ++;
  						if ((pInstance->audioFramesBuffer.txTicks==pInstance->audioFramesBuffer.txPacketization)||(pInstance->audioFramesBuffer.txStoredSamples == pInstance->audioFramesBuffer.txPacketization ))
						{
 							pInstance->audioFramesBuffer.txAudioPacketPos  = pInstance->audioFramesBuffer.txAudioSamplesSize ;
 							//if (pInstance->audioFramesBuffer.txTicks==pInstance->audioFramesBuffer.txPacketization)
  							pInstance->audioFramesBuffer.txTicks=(unsigned char)0;
  							pInstance->audioFramesBuffer.txStoredSamples =0;
 							pInstance->audioFramesBuffer.txAudioSamplesSize =0;
							found = (int)1;
 						}else {pInstance->audioFramesBuffer.txAudioPacketPos  =  0; }
					} 
					break;
 			}	
   		 	AlsaSamples[ch*82]=0;
		} else{
			if (pInstance)
		 		 pInstance->audioFramesBuffer.txTimestamp+=(80);//125usecs
	  	
		}
 	}
  	return found;
}
 
int sc1445x_mcu_send_packet_to_rtp(void)
{
 	int i, ret=0 ;
	int ch;

 	sc1445x_mcu_instance_t *pInstance;
  	for (i=0;i<MCU_MAX_INSTANCES;i++)
	{
		pInstance = sc1445x_mcu_scheduler.mcuInstances[i];
		if (!pInstance) continue;

	 	// check only active mcu instances (active calls)
		if ( pInstance->mediaStream.mediaFormat != MCU_MFMT_RTPAVP_ID) 
			continue;
 		if ( (pInstance->state & MCU_CALL_STATE_CONNECTED_TX) && pInstance->pRtpSession)
		{
			ch = pInstance->alsaChannel ;
	 

			if (ch<0  || ch>=MAX_MCU_ALLOWED_CHANNELS ) continue;
 			// check if the required samples (according to ptime of the call) has beed read 
			// if yes then send the packet to rtp layer
 
 			if (pInstance->audioFramesBuffer.txAudioPacketPos == 2)
			{
 	//			pInstance->audioFramesBuffer.txTimestamp+=(pInstance->audioFramesBuffer.txPacketization*80);//125usecs
				pInstance->audioFramesBuffer.txAudioPacketPos= 0;
			}else if (pInstance->audioFramesBuffer.txAudioPacketPos)
			{
 				//pInstance->audioFramesBuffer.txTimestamp+=(pInstance->audioFramesBuffer.txPacketization*80);//125usecs
  
				if ((int)(pInstance->pRtpSession) >0)
				{
  
// 					if (pInstance->audioFramesBuffer.state == SC1445x_TALK_STATE_ACTIVE)
//						rtp_session_set_markbit(pInstance->pRtpSession,0) ;
//					else 
//						rtp_session_set_markbit(pInstance->pRtpSession,1) ;
  			 		ret = rtp_session_set_payload_type( pInstance->pRtpSession, pInstance->mediaStream.mediaParams.rtp_ptype );
   					ret = sc1445x_ortp_send(pInstance->pRtpSession ,
						pInstance->audioFramesBuffer.audioPacket,
						(int)pInstance->audioFramesBuffer.txAudioPacketPos, 
						pInstance->audioFramesBuffer.txTimestamp);
				}

				if (ret<0) 
				{	
 					DPRINT(" sc1445x_mcu_send_packet_to_rtp %d socketerror=[%s]\n",ret,  getSocketError());
					DPRINT("  packetsize=%d  timestamp=%d \n",pInstance->audioFramesBuffer.txAudioPacketPos, pInstance->audioFramesBuffer.txTimestamp );
					DPRINT("------ MCU START: %d , RTP serssion = %d\n ",(int) pInstance, (int) (pInstance->pRtpSession));
   				}
 				pInstance->audioFramesBuffer.txAudioPacketPos  =  0;
				pInstance->audioFramesBuffer.txAudioSamplesSize  = 0;
				pInstance->audioFramesBuffer.txStoredSamples  = 0;
   			}  
		
		}else{
			if (pInstance)
			{
				;//pInstance->audioFramesBuffer.txTimestamp+=(80);//125usecs
			}
 		}
	}
	return 0;
 }
 
 int sc1445x_mcu_read_packet_from_rtp(void)
{
 	int found =1;
	int i,ret; 
	int have_more = 0;
 	int ch;
	int validSize = 0;
	int try_again=0;

	sc1445x_mcu_instance_t *pInstance;
  
	for (i=0;i<MCU_MAX_INSTANCES;i++)
	{
		pInstance = sc1445x_mcu_scheduler.mcuInstances[i];
		if (!pInstance) continue;
  		if (pInstance->mediaStream.mediaFormat != MCU_MFMT_RTPAVP_ID) 
			continue;
 		// check only active mcu instances (active calls)
		if ((pInstance->state & MCU_CALL_STATE_CONNECTED_RX) &&  pInstance->pRtpSession)
		{
			ch = pInstance->alsaChannel ;
			if (ch<0  || ch>=MAX_MCU_ALLOWED_CHANNELS ) continue;
  			if (pInstance->audioFramesBuffer.rxMorePackets==0)
			{
				try_again=0;
//_start1:
				if (pInstance->pRtpSession>0) {
					have_more = 0;
  				 	ret= sc1445x_ortp_receive((RtpSession *)(pInstance->pRtpSession),(unsigned int) (pInstance->audioFramesBuffer.rxTimestamp),
 				 	&pInstance->audioFramesBuffer.rtpPacket[0], /*pInstance->audioFramesBuffer.rxPacketization*/ SC1445x_MAX_PTIME*pInstance->codecSampleSize, &have_more);
 					//FIX 11082009
   				}else 
				{
					DPRINT("[%s] Invalid RTP SESSION \n", __FUNCTION__);
				}
		 		if (ret<0) 
				{
					DPRINT("Error [%s] \n",getSocketError());
					continue;//fix April 
				} 
				if (ret>0) {
					
   					validSize = ret%pInstance->codecSampleSize;
  	 				if (validSize !=0 ) 
					{
						int pt=(int)rtp_session_get_recv_payload_type(pInstance->pRtpSession );
   						if (pt==PH_MEDIA_CN_PAYLOAD)
						{
						 	pInstance->audioFramesBuffer.rxPacketSize =  ret;
						 	pInstance->audioFramesBuffer.rxMorePackets= 1;
							 
 						}else if (pt==pInstance->mediaStream.mediaParams.rtp_ptype) 
						{	
  							if (ret<pInstance->codecSampleSize) //Only SID Frame
							{
    							pInstance->audioFramesBuffer.rxPacketSize =   ret;
							 	pInstance->audioFramesBuffer.rxMorePackets=  1;
   							}
							else {
								// Audio + SID frames
   								pInstance->audioFramesBuffer.rxPacketSize =  pInstance->codecSampleSize;
								pInstance->audioFramesBuffer.rxMorePackets=  1;//(ret/pInstance->codecSampleSize); 
 							}
 						}else{
							printf("Codec Changed !!!!\n");

						}
  					 	pInstance->audioFramesBuffer.rxTimestamp+= (pInstance->audioFramesBuffer.rxMorePackets*80); 
						continue;//fix April 
  			        }else if (validSize==0) {
							
  					 	if (ret != pInstance->audioFramesBuffer.rxPacketization*pInstance->codecSampleSize)
							 pInstance->audioFramesBuffer.rxPacketization = (ret/pInstance->codecSampleSize);//FIX 11082009
					  	{
 							pInstance->audioFramesBuffer.rxMorePackets = (ret/pInstance->codecSampleSize); //pInstance->audioFramesBuffer.rxPacketization;//normal audio packet
							pInstance->audioFramesBuffer.rxPacketSize = ret;
							pInstance->audioFramesBuffer.rxTimestamp+= ( pInstance->audioFramesBuffer.rxPacketization*80); 
 							continue;//fix April 
						 }
 					}
 				}
				pInstance->audioFramesBuffer.rxTimestamp+= ( 80); 
			}  
		}else  {
			if (0){//pInstance){
				{
					if (pInstance->audioFramesBuffer.rxMorePackets==0)
					{
						ret= sc1445x_ortp_receive(pInstance->pRtpSession,(unsigned int) (pInstance->audioFramesBuffer.rxTimestamp),
						&pInstance->audioFramesBuffer.rtpPacket[0], pInstance->audioFramesBuffer.rxPacketization*pInstance->codecSampleSize, &have_more); //SC1445x_MAX_AUDIO_HEADER
				
						pInstance->audioFramesBuffer.rxTimestamp+=( pInstance->audioFramesBuffer.rxPacketization*80);//SIPIT23 pInstance->codecSampleSize);//125usecs
						pInstance->audioFramesBuffer.rxMorePackets  =2;
						memset(&pInstance->audioFramesBuffer.rtpPacket[0], 0, pInstance->audioFramesBuffer.rxPacketization*pInstance->codecSampleSize);
					}
				}
			}
 		} 
 	}  
  	return found; 
} 
 
int sc1445x_mcu_write_packet_to_alsa(void)
{
	int channel = 0;
	int pos = 0;
 	int i;
	int found = 0;
 
   	sc1445x_mcu_instance_t *pInstance;
	//memset(WriteToAlsaSamples, 0,ALSA_MAX_BYTES); 	
//	return  sc1445x_amb_sound_write( WriteToAlsaSamples, ALSA_MAX_BYTES); 
 
	for (i=0;i<MCU_MAX_INSTANCES;i++)
	{
		pInstance = sc1445x_mcu_scheduler.mcuInstances[i];
		if (!pInstance)  continue;

		if (pInstance && (pInstance->mediaStream.mediaFormat ==MCU_MFMT_T38_ID))
		{
			found =1;
            channel = pInstance->alsaChannel;
            WriteToAlsaSamples[channel*82]=80;
			WriteToAlsaSamples[(channel*82)+1]=1;
			continue;
		}

		// check only active mcu instances (active calls)
		if (pInstance &&  (pInstance->state & MCU_CALL_STATE_CONNECTED_RX)) //fix 4 channels 
		{
			found =1;
    		channel = pInstance->alsaChannel;
			if (channel<0  || channel>=MAX_MCU_ALLOWED_CHANNELS ) continue;
 		
  			 if (!pInstance->audioFramesBuffer.rxPacketSize || !pInstance->audioFramesBuffer.rxMorePackets) { //fix 4 channels
				WriteToAlsaSamples[channel*82]= 0; 
				WriteToAlsaSamples[channel*82+1]=0; 
  				pInstance->audioFramesBuffer.rxMorePackets=0;
 				continue;
			}
     			 
			if (!(pInstance->audioFramesBuffer.rxPacketSize % pInstance->codecSampleSize))
			{
 					pos = (pInstance->audioFramesBuffer.rxPacketization-pInstance->audioFramesBuffer.rxMorePackets)*pInstance->codecSampleSize ;
					memcpy(&WriteToAlsaSamples[channel*82+2], &pInstance->audioFramesBuffer.rtpPacket[pos], pInstance->codecSampleSize); 
					WriteToAlsaSamples[channel*82]=(unsigned char)pInstance->codecSampleSize;
					WriteToAlsaSamples[channel*82+1]=1;
 
					pInstance->audioFramesBuffer.rxPacketSize -= pInstance->codecSampleSize ;
					if (pInstance->audioFramesBuffer.rxPacketSize<0) pInstance->audioFramesBuffer.rxPacketSize=0;
				}else{ //VAD/CN Frame 
					if (pInstance->audioFramesBuffer.rxPacketSize && pInstance->audioFramesBuffer.rxPacketSize<pInstance->codecSampleSize) //fix 4 channels
					{
   						memcpy(&WriteToAlsaSamples[channel*82+2], &pInstance->audioFramesBuffer.rtpPacket[pos], pInstance->audioFramesBuffer.rxPacketSize); 
      					WriteToAlsaSamples[channel*82]=(unsigned char)pInstance->audioFramesBuffer.rxPacketSize;
	 					WriteToAlsaSamples[channel*82+1]=2;
  
				}else{
					WriteToAlsaSamples[channel*82]= 0; 
					WriteToAlsaSamples[channel*82+1]=0; 
   					}  
					pInstance->audioFramesBuffer.rxPacketSize = 0;
			}
			if (pInstance->audioFramesBuffer.rxMorePackets>0)
				pInstance->audioFramesBuffer.rxMorePackets--;
  
 
 	}else{
		if (pInstance){ //fix 4 channels
	//		WriteToAlsaSamples[channel*82]= 0; 
	//		WriteToAlsaSamples[channel*82+1]=0; 
 
			pInstance->audioFramesBuffer.rxMorePackets=0;
			continue;
    }
  }   
 }  
 
 return  sc1445x_amb_sound_write( WriteToAlsaSamples, ALSA_MAX_BYTES);
}
 
  
#ifdef REMOTE_LOOPBACK_TEST

void sc1445x_mcu_remote_loopback(void) 			
{
	int i, ret, ret1;
	int have_more;
  	sc1445x_mcu_instance_t *pInstance;
 
	for (i=0;i<MCU_MAX_INSTANCES;i++)
	{
		pInstance = sc1445x_mcu_scheduler.mcuInstances[i];
	 	// check only active mcu instances (active calls)
		//if (pInstance &&  (pInstance->state & MCU_CALL_STATE_CONNECTED_TX) && pInstance->pRtpSession)
		{
 			if (pInstance && pInstance->mediaStream.mediaAttr== MCU_MATTR_RLOOP_ID)
 			{
					have_more = 0;

 					ret= sc1445x_ortp_receive((RtpSession *)(pInstance->pRtpSession),
						(unsigned int) (pInstance->audioFramesBuffer.rxTimestamp),
 						&pInstance->audioFramesBuffer.rtpPacket[0],      
						pInstance->audioFramesBuffer.rxPacketization*  pInstance->codecSampleSize, 
						&have_more); 
					if (ret>0)
						pInstance->audioFramesBuffer.rxTimestamp+=(pInstance->audioFramesBuffer.txPacketization*80);//125usecs
					else 
						pInstance->audioFramesBuffer.rxTimestamp+=80;

 
					if (0)// ret>0)
					{
						ret1 = sc1445x_ortp_send(pInstance->pRtpSession ,
 						    &pInstance->audioFramesBuffer.rtpPacket[0],
							ret, 
							pInstance->audioFramesBuffer.txTimestamp);
						pInstance->audioFramesBuffer.txTimestamp+=(pInstance->audioFramesBuffer.txPacketization*80);//125usecs
					}
  
			}
 		}
	}
}
int  loop_srtp_sendto (struct _RtpTransport *t, const  void *buf, size_t len, int flags, const struct sockaddr *to, socklen_t tolen); 
int  loop_srtp_recvfrom(struct _RtpTransport *t, void *buf, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen) ;
int  loop_srtp_sendto (struct _RtpTransport *t, const  void *buf, size_t len, int flags, const struct sockaddr *to, socklen_t tolen)
{
	 sc1445x_mcu_instance_t *pMcuInstance=(sc1445x_mcu_instance_t *)t->data;
	RtpSession *session = (RtpSession *)pMcuInstance->pRtpSession ;
	ortp_socket_t sockfd=  session->rtp.socket; 
	if ( !session || !pMcuInstance) return 0;

	return sendto (sockfd,buf, (int)len, 0,to,tolen);
}

int loop_srtp_recvfrom(struct _RtpTransport *t, void *buf, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen) 
{ 
	int error=0;
	int slen=len; 
	int ret;
	int rtpheader =12;

	 sc1445x_mcu_instance_t *pMcuInstance=(sc1445x_mcu_instance_t *)t->data;
	RtpSession *session = (RtpSession *)pMcuInstance->pRtpSession ;
	ortp_socket_t sockfd=  session->rtp.socket; 
	if ( !session || !pMcuInstance) return 0;
	 
	error = recvfrom(sockfd, buf,len, 0, (struct sockaddr *) from,fromlen);
 
	if (error>rtpheader)
	{
 		ret = sc1445x_ortp_send(session ,
						&buf[rtpheader],
						error-rtpheader, 
						pMcuInstance->audioFramesBuffer.txTimestamp);
					    pMcuInstance->audioFramesBuffer.txTimestamp+=
							(pMcuInstance->audioFramesBuffer.txPacketization*80);//125usecs
  
	}
 
	return 0;
}

void loop_add_session_transport(sc1445x_mcu_instance_t *pMcuInstance);
void loop_add_session_transport(sc1445x_mcu_instance_t *pMcuInstance)
{
	RtpTransport *rtpt;  
 
 	rtpt=ortp_new(RtpTransport,1);
	rtpt->data=(void*)pMcuInstance;
	rtpt->t_getsocket=NULL;
	rtpt->t_sendto= loop_srtp_sendto;
	rtpt->t_recvfrom=loop_srtp_recvfrom;
	if (pMcuInstance->pRtpSession)
	 	rtp_session_set_transports((RtpSession *)pMcuInstance->pRtpSession, rtpt, NULL);  
 }
#endif

#ifdef SC1445x_AE_SUPPORT_FAX
void t38Polling(void)
{
	int ret, i;
	int found =0;
  struct timeval       time_stmp;
  sc1445x_mcu_instance_t *pInstance;
  
	for (i=0;i<MCU_MAX_INSTANCES;i++)
	{
		pInstance = sc1445x_mcu_scheduler.mcuInstances[i];
		if (!pInstance)  continue;
 		if (pInstance && (pInstance->mediaStream.mediaFormat ==MCU_MFMT_T38_ID))
		{
			found =1;
      break;
		}
	}
	if (!found) return ;
 	// RX T38/PACKET
	ret = sc1445x_t38RecvData(t38InDataBuffer);
	if (ret>0) 
	{
    gettimeofday(&time_stmp, NULL);
    /*
    printf( "[%d:%06d] RX IP len=%d: 0x%04x 0x%04x 0x%04x\n",  time_stmp.tv_sec, time_stmp.tv_usec,ret,
                *(t38InDataBuffer), *(t38InDataBuffer+1), *(t38InDataBuffer+2) );
    */    
		T38InPacket.data =(unsigned short*) t38InDataBuffer;
		T38InPacket.numBits = ret << 3;
 		ata_put_t38_packet( (unsigned short) pInstance->alsaChannel, &T38InPacket);
    //T38Object.fax_packetIn_ptr = &T38InPacket; 
    T38Object.fax_packetIn_ptr  = NULL;
	}else 
	    T38Object.fax_packetIn_ptr = NULL; 

	//TX T38/PACKET
	T38OutPacket.data =(unsigned short*) t38OutDataBuffer;
	T38OutPacket.numBits = 0;
 	T38Object.fax_packetOut_ptr = &T38OutPacket;

  /* dst_ptr now holds the data to be written to DSP via Alsa */
 	//T38Object.dst_ptr =(unsigned short*) &WriteToAlsaSamples[2];
  T38Object.dst_ptr =(unsigned short*) &WriteToAlsaSamples[((pInstance->alsaChannel)*82) + 2];

  if( T38Object.src_ptr != NULL )
	{  
 		//P2_SET_DATA_REG = 0x400 ; //GPIO 
		ata_run_t38( (unsigned short) pInstance->alsaChannel, &T38Object); 
	}
	if (T38Object.fax_packetOut_ptr->numBits)
	{
 //   ip_pck_cnt++;
    gettimeofday(&time_stmp, NULL);
	  //printf("BITS to send to IP = %d \n", T38Object.fax_packetOut_ptr->numBits);
    /* printf( "[%d:%06d] TX IP len=%d: 0x%04x 0x%04x 0x%04x\n",  time_stmp.tv_sec, time_stmp.tv_usec,(int)(T38Object.fax_packetOut_ptr->numBits >> 3),
                *(T38Object.fax_packetOut_ptr->data), *(T38Object.fax_packetOut_ptr->data+1), *(T38Object.fax_packetOut_ptr->data+2) );
    */
		sc1445x_t38SendData((char*)(T38Object.fax_packetOut_ptr->data), (int)(T38Object.fax_packetOut_ptr->numBits >> 3) );
	}

  /* This code resolves the issue DK=>DK fax call where the DSP hangs after first INIT.
  ** After sending the first 3 IP packets, re-initialise the DSP.
  */
/*  if(ip_pck_cnt == 3 && t38_call_start == 0)
  {
    t38_call_start = 1;
    ata_init_t38_modem( pInstance->alsaChannel, 1024, 1024, 250, 2050);
  }
*/
}
#endif

