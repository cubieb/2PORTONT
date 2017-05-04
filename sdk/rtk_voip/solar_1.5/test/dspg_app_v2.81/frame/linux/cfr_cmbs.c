/*!
*  \file       cfr_cmbs.c
*  \brief      Target side
*  \author     stein
*
*  @(#)  %filespec: cfr_cmbs.c~DMZD53#2 %
*
*******************************************************************************
*  \par  History
*  \n==== History ============================================================\n
*  date        name     version   action                                          \n
*  ----------------------------------------------------------------------------\n
*  14-feb-09   R.Stein   1         Initialize \n
*  14-feb-09   D.Kelbch  2         Project integration - VONE \n
*  09-Apr-09   Kelbch    161       Update of Media Configuration during start-up \n

*******************************************************************************/

#include <stdlib.h>
#include <stddef.h>  // for offsetof
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <time.h>
#include <sys/time.h> // we need <sys/select.h>; should be included in <sys/types.h> ???
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <syslog.h>
#include <stdarg.h>

#include "cmbs_platf.h"
#include "cmbs_int.h"   /* internal API structure and defines */
#include "cfr_uart.h"   /* packet handler */
#include "cfr_debug.h"  /* debug handling */

#include "cfr_mssg.h"


void  *           _cmbs_int_CbThread( void * pVoid );

/*
   GLOBALS
*/

/*\brief global CMBS instance object */
ST_CMBS_API_INST  g_CMBSInstance;

ST_CMBS_DECT_MSG  g_st_cmbs_DectMsg;
u8                g_u8_TmpBuffer[CMBS_BUF_SIZE];   // persistent buffer for serialization and cmbs_int_EventSend

//		========== _cmbs_int_StartupBlockSignal ===========
/*!
		\brief			signal to block statement that CMBS is available

		\param[in]		pst_CMBSInst		 pointer to CMBS instance object

		\return			<none>

*/

void              _cmbs_int_StartupBlockSignal( PST_CMBS_API_INST pst_CMBSInst )
{
   int rc;
                                 // signal to CMBS host API, target available
   pthread_mutex_lock( &pst_CMBSInst->cond_Mutex );
   rc = pthread_cond_signal( &pst_CMBSInst->cond_UnLock );
   pthread_mutex_unlock( &pst_CMBSInst->cond_Mutex );

   if( rc != 0 )
      CFR_DBG_OUT( "pthread_cond_signal returned %d\n", rc );

}

//		========== _cmbs_int_MsgQCreate ===========
/*!
		\brief				 Create a message queue

		\param[in,out]		 < none >

		\return				 < int > return identifier of queue. If there was an error, a value of -1 is returned.

*/

int               _cmbs_int_MsgQCreate( void )
{
   int            id = -1;

   id = msgget( IPC_PRIVATE, IPC_CREAT | IPC_EXCL | 0666 );

   if( id == -1 )
   {
      switch( errno )
      {
         case  EEXIST:
            CFR_DBG_ERROR( "[ERROR]msgget, message queue exists (EEXIST)\n" );
            break;

         case  ENOMEM:
            CFR_DBG_ERROR( "[ERROR]msgget, not enough memory (ENOMEM)\n" );
            break;

         case  ENOSPC:
            CFR_DBG_ERROR( "[ERROR]msgget, max. number of queues (MSGMNI) exceeded\n" );
            break;

         default:
            perror( "[ERROR]msgget" );
      }
   }

   return id;
}

//		========== _cmbs_int_MsgQDestroy ===========
/*!
		\brief			   Destroy message queue

		\param[in]  	   nMsgQId		   message queue identifier

		\return			   < none >

*/

void              _cmbs_int_MsgQDestroy( int nMsgQId )
{
   if( msgctl(nMsgQId, IPC_RMID, 0) == -1 )
   {
      perror( "[ERROR]msgctl" );
   }
}

//		========== cmbs_int_EnvCreate ===========
/*!
		\brief				build up the environment of CMBS-API. Open the relevant devices and starts the pumps.

		\param[in,out]		e_Mode         to be used CMBS mode, currently only CMBS Multiline is supported

		\param[in,out]		pst_DevCtl		pointer to device call control properties

		\param[in,out]		pst_DevMedia	pointer to device media control properties

		\return				< E_CMBS_RC >

*/
E_CMBS_RC         cmbs_int_EnvCreate( E_CMBS_API_MODE e_Mode, ST_CMBS_DEV * pst_DevCtl, ST_CMBS_DEV * pst_DevMedia )
{
   int               rc;
   struct timespec   ts;
   struct timeval    tp;
   u8                u8_Buffer[12];
   u8                u8_BufferIDX = 0;

   openlog("cmbs_api.c", LOG_CONS | LOG_PID, LOG_NEWS);

   memset( &g_CMBSInstance, 0, sizeof(g_CMBSInstance) );

   g_CMBSInstance.u32_CallInstanceCount = 0x80000000;

   pthread_cond_init( &g_CMBSInstance.cond_UnLock, NULL );
   pthread_mutex_init( &g_CMBSInstance.cond_Mutex, NULL );

   CFR_CMBS_INIT_CRITICALSECTION ( g_CMBSInstance.h_CriticalSectionTransmission );

   /*! \todo check later the mode, if supported from target */
   g_CMBSInstance.e_Mode = e_Mode;  // useful later, if the API is connected to target side.

   g_CMBSInstance.e_Endian = cmbs_int_EndiannessGet();

                                    // initialize the device control
   if( pst_DevCtl )
   {
      g_CMBSInstance.eDevCtlType = pst_DevCtl->e_DevType;

      if( (g_CMBSInstance.msgQId = _cmbs_int_MsgQCreate() ) == -1 )
      {
         CFR_DBG_ERROR( "cmbs_int_EnvCreate: ERROR creating message queue\n" );
         return CMBS_RC_ERROR_GENERAL;
      }
      else
      {
         CFR_DBG_INFO( "cmbs_int_EnvCreate: MsgQId:%d\n", g_CMBSInstance.msgQId );
      }

      if(  pst_DevCtl->e_DevType == CMBS_DEVTYPE_UART )
      {
         g_CMBSInstance.fdDevCtl = cfr_uartInitialize( pst_DevCtl->u_Config.pUartCfg);
      }
      else if(  pst_DevCtl->e_DevType == CMBS_DEVTYPE_USB )
      {
         g_CMBSInstance.fdDevCtl = cfr_usbInitialize( pst_DevCtl->u_Config.pUartCfg);
      }
      else
      {
         CFR_DBG_ERROR ( "cmbs_api_Init: !!!! device is not supported, yet!\n");
         return CMBS_RC_ERROR_GENERAL;
      }

      if( g_CMBSInstance.fdDevCtl != -1 )
      {
         rc = pthread_create( &g_CMBSInstance.serialThreadId, NULL, & cfr_uartThread, &g_CMBSInstance );
         if( rc != 0 )
         {
            CFR_DBG_ERROR( "[ERROR]cmbs_api_Init: !!!! Couldn't create Serial Thread. ErrorCode:%d\n", rc );
            /*! \todo exception handling is needed */
            return CMBS_RC_ERROR_GENERAL;
         }
      }
      else
      {
         CFR_DBG_ERROR( "[ERROR]cmbs_api_Init: !!!! Couldn't open Serial device\n" );
         return CMBS_RC_ERROR_GENERAL;
      }

      CFR_DBG_OUT( "cmbs_api_Init: data pump is started. ThreadId:%lu\n", (unsigned long)g_CMBSInstance.serialThreadId  );

                                    // control device pipe is established
                                    // start control thread
      rc = pthread_create( &g_CMBSInstance.callbThreadId, NULL, &_cmbs_int_CbThread, &g_CMBSInstance );
      if( rc != 0 )
      {
         CFR_DBG_ERROR( "[ERROR]cmbs_api_Init: !!!! Couldn't create CB Thread. ErrorCode:%d\n", rc );
         /*! \todo exception handling is needed */
         return CMBS_RC_ERROR_GENERAL;
      }

      CFR_DBG_OUT( "cmbs_api_Init: call back thread started. ThreadId:%lu\n", (unsigned long)g_CMBSInstance.callbThreadId );
   }
   else
   {
      CFR_DBG_WARN( "[WARN]cmbs_api_Init: !!!! device type is not supported or identified!\n");

      return CMBS_RC_ERROR_PARAMETER;
   }
   if( pst_DevMedia )
   {
      g_CMBSInstance.eDevMediaType = pst_DevMedia->e_DevType;
      // support of PCM IOM
      if ( pst_DevMedia->e_DevType == CMBS_DEVTYPE_TDM )
      {
         // prepare buffer for TDM configuration in Hello command
         CFR_DBG_OUT( "cmbs_api_Init: Media is TDM interface \n");
         u8_Buffer[0] =(u8)pst_DevMedia->u_Config.pTdmCfg->e_Type;
         u8_Buffer[1] =(u8)pst_DevMedia->u_Config.pTdmCfg->e_Speed;
         u8_Buffer[2] =(u8)pst_DevMedia->u_Config.pTdmCfg->e_Sync;

         u8_BufferIDX = cfr_ie_ser_u16(u8_Buffer +3, pst_DevMedia->u_Config.pTdmCfg->u16_SlotEnable);
         // adjust IDX
         u8_BufferIDX += 3;
         ;
      }
   }
   else
   {
      CFR_DBG_OUT( "cmbs_api_Init: Media is not set-up, yet!\n");
   }

   cmbs_int_cmd_Send( CMBS_CMD_HELLO, u8_Buffer, u8_BufferIDX );
                                    // block init until timeout, or reply is received

   rc = gettimeofday( &tp, NULL );
                                    // typically gettimofday does not return with error
   ts.tv_sec  = tp.tv_sec;
   ts.tv_nsec = tp.tv_usec * 1000;
   ts.tv_sec += 10;                 // wait 10 sec

   CFR_DBG_OUT( "cmbs_api_Init: Connecting target...\n" );

   pthread_mutex_lock( &g_CMBSInstance.cond_Mutex );

   rc = pthread_cond_timedwait( &g_CMBSInstance.cond_UnLock, &g_CMBSInstance.cond_Mutex, &ts );

   pthread_mutex_unlock( &g_CMBSInstance.cond_Mutex );

   if( rc != 0 )
   {
      if( rc == ETIMEDOUT )
      {
         CFR_DBG_ERROR( "[ERROR]cmbs_api_Init: !!!! Timeout: Target did not respond on HELLO\n" );
      }
      else
      {
         CFR_DBG_ERROR( "[ERROR]cmbs_api_Init: !!!! pthread_cond_timedwait returned %d\n", rc );
      }

      return   CMBS_RC_ERROR_GENERAL;
   }
                                    // reset communication module on target side
                                    // skip any history issues
   cmbs_int_cmd_Send( CMBS_CMD_RESET, NULL, 0 );

   return   CMBS_RC_OK;
}
//		========== cmbs_int_EnvDestroy ===========
/*!
		\brief				 clean up the CMBS environment

		\param[in,out]		 < none >

		\return				< E_CMBS_RC >

      \todo de-register on module side
*/

E_CMBS_RC         cmbs_int_EnvDestroy( void )
{
                                    // maybe we need to de-register on module side
   if( g_CMBSInstance.serialThreadId )
      pthread_cancel( g_CMBSInstance.serialThreadId );

   if( g_CMBSInstance.callbThreadId )
      pthread_cancel( g_CMBSInstance.callbThreadId );

   CFR_CMBS_DELETE_CRITICALSECTION ( g_CMBSInstance.h_CriticalSectionTransmission );

   if( g_CMBSInstance.msgQId != -1 )
      _cmbs_int_MsgQDestroy( g_CMBSInstance.msgQId );

   close( g_CMBSInstance.fdDevCtl );
   close( g_CMBSInstance.fdDevMedia );
   
	 closelog();
   
   return   CMBS_RC_OK;
}
//		========== _cmbs_int_CbThread ===========
/*!
		\brief				 callback pump to receive and call application call-back

		\param[in,out]		 pVoid		pointer to CMBS instance object

		\return				< void * >  always NULL

*/

void  *           _cmbs_int_CbThread( void * pVoid )
{
   PST_CMBS_API_INST
                  pInstance = (PST_CMBS_API_INST)pVoid;
   int            msgQId    = pInstance->msgQId;
   int            nRetVal;
   size_t         nMsgSize;
   ST_CMBS_LIN_MSG
                  LinMsg;
   U_CMBS_SER_DATA
                  CmbsMsg;
   u32            u32_Sync = CMBS_SYNC;

   static u32     nDataIndex = 0;

   nMsgSize = sizeof( LinMsg.msgData );

                                    // never ending loop
                                    // thread will be exited automatically when parent thread finishes
   while( 1 )
   {
      nRetVal = msgrcv( msgQId, &LinMsg, nMsgSize, 0, 0);

      if( nRetVal == -1 )
      {
         CFR_DBG_ERROR( "[ERROR]CB Thread: !!!! msgrcv ERROR:%d\n", errno );
      }
      else
      {
         u32 i;

/*
         CFR_DBG_OUT( "Received raw data %2d bytes:", LinMsg.msgData.nLength );
         for( i = 0; i < (u32)LinMsg.msgData.nLength; i++ )
         {
            CFR_DBG_OUT( " %02X", LinMsg.msgData.u8_Data[i] );
         }
         CFR_DBG_OUT( "\n" );
*/
         if( (u32)LinMsg.msgData.nLength >= sizeof(u32) &&
             memcmp( (u32*)LinMsg.msgData.u8_Data, &u32_Sync, sizeof(u32)) == 0 )
         {
//            CFR_DBG_OUT( "CB Thread: Sync word detected, reset DataIndex\n" );
            nDataIndex = 0;
         }
                                    // parse received message
         for( i = 0; i < (u32)LinMsg.msgData.nLength; i++ )
         {
            if( nDataIndex == 0 )
            {
               memset( &CmbsMsg, 0, sizeof(CmbsMsg) );
            }

                                    // save message data
            if( i < sizeof(CmbsMsg.serialBuf) )
            {
               CmbsMsg.serialBuf[nDataIndex] = LinMsg.msgData.u8_Data[i];
               nDataIndex++;
            }
            else
            {
               CFR_DBG_ERROR( "[ERROR]CB Thread: !!!! msgrcv ERROR: buffer overflow\n" );
               nDataIndex = 0;
               break;
            }

                                    // validate syc dword
            if( nDataIndex == sizeof(u32) )
            {
               if( memcmp( CmbsMsg.serialBuf, &u32_Sync, sizeof(u32)) != 0 )
               {
                  CFR_DBG_ERROR( "[ERROR]CB Thread: !!!! msgrcv ERROR: NO sync word detected\n" );
                  nDataIndex = 0;
                  break;
               }
            }

                                    // check cmbs message length
            if( nDataIndex >= sizeof(u32) + sizeof(u16) ) // sizeof(u32_Sync) + sizeof(u16_TotalLength)
            {
               u16 u16_Total ;

               if( g_CMBSInstance.e_Endian == E_CMBS_ENDIAN_BIG )
               {
                  u16_Total = cmbs_int_EndianCvt16(CmbsMsg.st_Data.st_Msg.st_MsgHdr.u16_TotalLength);
               }
               else
               {
                  u16_Total = CmbsMsg.st_Data.st_Msg.st_MsgHdr.u16_TotalLength;
               }

               if( nDataIndex == sizeof(CmbsMsg.st_Data.u32_Sync) + u16_Total )
               {
                                    // we assume that cmbs message is complete
                  CFR_DBG_OUT( "===================================\n" );

                  if( g_CMBSInstance.e_Endian == E_CMBS_ENDIAN_BIG )
                  {
                     cmbs_int_HdrEndianCvt( &CmbsMsg.st_Data.st_Msg.st_MsgHdr );
                  }

                  if( g_CMBSInstance.st_ApplSlot.pFnCbLogBuffer.pfn_cmbs_api_log_incoming_packet_write_finish_cb != NULL )
                  {
                     g_CMBSInstance.st_ApplSlot.pFnCbLogBuffer.pfn_cmbs_api_log_incoming_packet_write_finish_cb((u8 *)&CmbsMsg.st_Data.st_Msg, CmbsMsg.st_Data.st_Msg.st_MsgHdr.u16_TotalLength);
                  }

                  cmbs_int_EventReceive( (u8 *)&CmbsMsg.st_Data.st_Msg, CmbsMsg.st_Data.st_Msg.st_MsgHdr.u16_TotalLength );
                                    // reset; we might have received more than one cmbs message
                  nDataIndex = 0;
               }
            }
         }
      }
   }

   return NULL;
}

void     cmbs_intDebugOUT ( const char *pszFormat, ... )
{
   TCHAR buffer[256];
   va_list ap;

   strcpy( buffer, "CMBS_API ");
   va_start(ap, pszFormat);
   vsprintf(buffer +strlen("CMBS_API "), pszFormat, ap);
   va_end(ap);
	 syslog (LOG_DEBUG, "%s.", buffer);

}


//*/
