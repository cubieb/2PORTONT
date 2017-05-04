/*!
*  \file       cfr_cmbs.c
*  \brief      Target side
*  \author     kelbch
*
*  @(#)  %filespec: cfr_cmbs.c~DMZD53#2 %
*
*******************************************************************************
*  \par  History
*  \n==== History ============================================================\n
*  date        name     version   action                                          \n
*  ----------------------------------------------------------------------------\n
*  12-may-09   Kelbch   1         Initialize / port to WIN32 console application  \n
*******************************************************************************/

#include "windows.h"
#include <stdlib.h>
#include <stddef.h>  // for offsetof
#include <stdio.h>
#include "cmbs_platf.h"

#include "cmbs_int.h"   /* internal API structure and defines */
#include "cfr_uart.h"   /* packet handler */
#include "cfr_debug.h"  /* debug handling */

#include "cfr_mssg.h"

extern void		cfr_uartClose ( void );
extern void cmbs_int_DataSignal(void);

DWORD WINAPI _cmbs_int_CbThread(LPVOID pvoid );

/*
   GLOBALS
*/

/*\brief global CMBS instance object */
/*\todo global needs to be changed to dynamic memory in order to handle multiple instances */

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
	SetEvent( pst_CMBSInst->h_InitBlock );

}

//		========== _cmbs_int_MsgQCreate ===========
/*!
		\brief				 Create a message queue

		\param[in,out]		 < none >

		\return				 < int > return identifier of queue. If there was an error, a value of -1 is returned.

      \note win32 implementation not needed
*/

int               _cmbs_int_MsgQCreate( void )
{
   return 1;
}

//		========== _cmbs_int_MsgQDestroy ===========
/*!
		\brief			   Destroy message queue

		\param[in]  	   nMsgQId		   message queue identifier

		\return			   < none >

      \note win32 implementation not needed
*/

void              _cmbs_int_MsgQDestroy( int nMsgQId )
{
   if (nMsgQId){};
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
   u8             u8_Buffer[12];
   u8             u8_BufferIDX = 0;

   memset( &g_CMBSInstance, 0, sizeof(g_CMBSInstance) );

   OutputDebugString( "Initialize\n" );

   g_CMBSInstance.u32_CallInstanceCount = 0x80000000;

   CFR_CMBS_INIT_CRITICALSECTION ( g_CMBSInstance.h_CriticalSectionTransmission );

   /*! \todo check later the mode, if supported from target */
   g_CMBSInstance.e_Mode = e_Mode;  // useful later, if the API is connected to target side.

   g_CMBSInstance.e_Endian = cmbs_int_EndiannessGet();

                                    // initialize the device control
   if( pst_DevCtl )
   {
      g_CMBSInstance.eDevCtlType = pst_DevCtl->e_DevType;
	  // initialize message queue to upper application

      switch( pst_DevCtl->e_DevType )
      {
         case CMBS_DEVTYPE_UART:
            if( cfr_uartInitialize( pst_DevCtl->u_Config.pUartCfg) == -1 )
            {
               CFR_CMBS_DELETE_CRITICALSECTION ( g_CMBSInstance.h_CriticalSectionTransmission );
               return CMBS_RC_ERROR_GENERAL;
            }
            cfr_uartReceiveNotifyCBRegister( (void*)cmbs_int_DataSignal );
            break;

         default:
            CFR_DBG_ERROR ( "cmbs_api_Init: !!!! device is not supported, yet!\n");
            CFR_CMBS_DELETE_CRITICALSECTION ( g_CMBSInstance.h_CriticalSectionTransmission );
            return CMBS_RC_ERROR_GENERAL;
      }

	  // start control pipe thread
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

         u8_BufferIDX = (u8)cfr_ie_ser_u16(u8_Buffer +3, pst_DevMedia->u_Config.pTdmCfg->u16_SlotEnable);
         // adjust IDX
         u8_BufferIDX += 3;
         ;
      }
   }
   else
   {
      CFR_DBG_OUT( "cmbs_api_Init: Media is not set-up, yet!\n");
   }
   // start CMBS connection
   g_CMBSInstance.h_RecPath = CreateEvent( NULL, FALSE, FALSE, "CMBS_REC Path" );

   g_CMBSInstance.bo_Run = TRUE;
   g_CMBSInstance.h_RecThread = CreateThread( NULL, 0, _cmbs_int_CbThread, (DWORD*)&g_CMBSInstance,
                                              0, &g_CMBSInstance.dw_ThreadID );

   g_CMBSInstance.h_InitBlock = CreateEvent( NULL, FALSE, FALSE, "CMBS Init Block" );

   cmbs_int_cmd_Send( CMBS_CMD_HELLO, u8_Buffer, u8_BufferIDX );
                                    // block init until timeout, or reply is received
   CFR_DBG_OUT( "cmbs_api_Init: Connecting target...\n" );
   // wait until CMBS is connected, value is in ms
	if ( WaitForSingleObject(g_CMBSInstance.h_InitBlock, 3 * 1000 ) == WAIT_TIMEOUT )
   {
      CFR_DBG_ERROR( "[ERROR]cmbs_api_Init: !!!! Timeout: Target did not respond on HELLO\n" );

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
   PST_CMBS_API_INST pst_CMBSInst = &g_CMBSInstance;

	CFR_DBG_OUT( "Destroy Env\n" );
  cfr_uartClose ();

   CFR_CMBS_DELETE_CRITICALSECTION ( g_CMBSInstance.h_CriticalSectionTransmission );

   if ( pst_CMBSInst->h_InitBlock )
   {
      CloseHandle( pst_CMBSInst->h_InitBlock );
      pst_CMBSInst->h_InitBlock = NULL;
   }

   if ( pst_CMBSInst->h_RecPath )
   {
      CloseHandle( pst_CMBSInst->h_RecPath );
      pst_CMBSInst->h_RecPath = NULL;

   }
   // stop pipe thread
	CFR_DBG_OUT( "Stop Thread\n" );
   g_CMBSInstance.bo_Run = FALSE;

   while ( pst_CMBSInst->dw_ThreadID != 0)
   {
      Sleep(100);
   }
   // destroy message queue and semaphore

	CFR_DBG_OUT( "End Destroy Env\n" );
  cfr_uartReceiveNotifyCBDeRegister();
  
   return   CMBS_RC_OK;
}


extern int			cfr_uartReceivedDataGet( PBYTE b_Buffer, int n_MaxLength );

int               _cmbs_int_PackageCollector (U_CMBS_SER_DATA * pst_Package, int nDataIndex )
{
   u16 u16_Total;

   do
   {
                 // validate syc dword
     if( nDataIndex >= sizeof(u32) )
      {
         u32 u32_Sync = CMBS_SYNC;

         if( memcmp( pst_Package->serialBuf , &u32_Sync, sizeof(u32)) != 0 )
         {
            CFR_DBG_ERROR( "[ERROR]CB Thread: !!!! msgrcv ERROR: NO sync word detected\n" );

            // Skip first byte and do perhaps a new loop
            memcpy(pst_Package->serialBuf, pst_Package->serialBuf + 1, nDataIndex - 1);

            nDataIndex--;

            continue;
         }
      }

                         // check cmbs message length
      if( nDataIndex < sizeof(u32) + sizeof(u16) ) // sizeof(u32_Sync) + sizeof(u16_TotalLength)
      {
         // Too less data to calculate message length, leave loop and collect more data
         break;
      }

      if( g_CMBSInstance.e_Endian == E_CMBS_ENDIAN_BIG )
      {
         u16_Total = cmbs_int_EndianCvt16(pst_Package->st_Data.st_Msg.st_MsgHdr.u16_TotalLength);
      }
      else
      {
         u16_Total = pst_Package->st_Data.st_Msg.st_MsgHdr.u16_TotalLength;
      }

      if( nDataIndex < (int)sizeof(pst_Package->st_Data.u32_Sync) + u16_Total )
      {
         // not enough data for complete cmbs message, leave loop and collect more data
         break;
      }

      // now: at least one complete message in receive buffer

                                    // we assume that cmbs message is complete
      CFR_DBG_OUT( "===================================\n" );

      if( g_CMBSInstance.e_Endian == E_CMBS_ENDIAN_BIG )
      {
         cmbs_int_HdrEndianCvt( &pst_Package->st_Data.st_Msg.st_MsgHdr );
      }

      if( g_CMBSInstance.st_ApplSlot.pFnCbLogBuffer.pfn_cmbs_api_log_incoming_packet_write_finish_cb != NULL )
      {
         g_CMBSInstance.st_ApplSlot.pFnCbLogBuffer.pfn_cmbs_api_log_incoming_packet_write_finish_cb((u8 *)&pst_Package->st_Data.st_Msg, pst_Package->st_Data.st_Msg.st_MsgHdr.u16_TotalLength);
      }

      cmbs_int_EventReceive( (u8 *)&pst_Package->st_Data.st_Msg, pst_Package->st_Data.st_Msg.st_MsgHdr.u16_TotalLength );

                                   // check, if we might have received more than one cmbs message
      if(nDataIndex == (int)sizeof(pst_Package->st_Data.u32_Sync) + u16_Total)
      {
         // we received no further data
         nDataIndex = 0;
         break;
      }

      memcpy(pst_Package->serialBuf, pst_Package->serialBuf + (int)sizeof(pst_Package->st_Data.u32_Sync) + u16_Total,
                                     nDataIndex - ((int)sizeof(pst_Package->st_Data.u32_Sync) + u16_Total));

      nDataIndex -= (int)sizeof(pst_Package->st_Data.u32_Sync) + u16_Total;
   } while(nDataIndex > 0);

   return nDataIndex;
}

//		========== _cmbs_int_CbThread ===========
/*!
		\brief				 callback pump to receive and call application call-back

		\param[in,out]		 pVoid		pointer to CMBS instance object

		\return				< void * >  always NULL

*/

DWORD WINAPI      _cmbs_int_CbThread(LPVOID pvoid )
{
   PST_CMBS_API_INST	pst_CMBS = &g_CMBSInstance;

   U_CMBS_SER_DATA   st_CMBSPackage;
   u32               n_DataIndex = 0;

   CFR_DBG_OUT( "CbThread Start \n");
   while ( pst_CMBS->bo_Run )
   {
      WaitForSingleObject(pst_CMBS->h_RecPath,500);

      // get the serial data and pass to collector function
      n_DataIndex += cfr_uartReceivedDataGet( (PBYTE )st_CMBSPackage.serialBuf + n_DataIndex ,
                                              sizeof(st_CMBSPackage.serialBuf) - n_DataIndex );

      n_DataIndex = _cmbs_int_PackageCollector ( &st_CMBSPackage, n_DataIndex );
   }

	 pst_CMBS->dw_ThreadID = 0;
   CFR_DBG_OUT( "CbThread End\n");
   return 0;
}

void               cmbs_int_DataSignal( void )
{
   SetEvent( g_CMBSInstance.h_RecPath );
}

void     cmbs_intDebugOUT ( const char *pszFormat, ... )
{
   TCHAR buffer[256];
   va_list ap;

   strcpy( buffer, "CMBS_API ");
   va_start(ap, pszFormat);
   vsprintf(buffer +strlen("CMBS_API "), pszFormat, ap);
   va_end(ap);

   OutputDebugString(buffer);

}

//*/
