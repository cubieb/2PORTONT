#include <stdlib.h>
#include <stddef.h>  // for offsetof
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <sys/time.h> // we need <sys/select.h>; should be included in <sys/types.h> ???
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

#include "cmbs_int.h"   /* internal API structure and defines */
#include "cfr_spi.h"   /* packet handler */
#include "cfr_debug.h"  /* debug handling */
#include "tcx_log.h"    /* logging functionality */
#include "bus.h"



int cfr_SpiInitialze( void )
{
   char           szDevName[128];
   int            fd = 0;

   sprintf( szDevName, MODEMDEVICE);
   
   // open device
   if ((fd = Bus_Open()) < 0)
   {	
      CFR_DBG_ERROR( "Error: Can't open device %s\n", szDevName );
      return -1;
   }
   else
   {
      CFR_DBG_OUT( "Opened %s\n", szDevName );
   }

   return fd;
}

//    ========== cfr_SpiRxThread ===========
/*!
      \brief            SPI data receive pump. if data is available a message is send
                        to cfr_cmbs task.

      \param[in]        pVoid          pointer to CMBS instance object

      \return           <void *>       return always NULL

*/

void  *cfr_SpiRxThread( void * pVoid )
{
   PST_CMBS_API_INST pInstance = (PST_CMBS_API_INST)pVoid;
   int            msgQId    = pInstance->msgQId;
   size_t         nMsgSize;

   ST_CMBS_LIN_MSG LinMsg;
      
//   CFR_DBG_OUT( "UART Thread: ID:%lu running\n", (unsigned long)pthread_self() );

   nMsgSize = sizeof( LinMsg.msgData );
   //CFR_DBG_OUT( "nMsgSize = %d, LinMsg.msgData.u8_Data size = %d\n", nMsgSize, sizeof(LinMsg.msgData.u8_Data));

   /* Never ending loop.   
      Thread will be exited automatically when parent thread finishes.
   */
   while( 1 )
   {
     	//memset( &LinMsg.msgData, 0, sizeof(LinMsg.msgData) );
     	
	if( (LinMsg.msgData.nLength = Bus_Read_blocking(&(LinMsg.msgData.u8_Data), sizeof(LinMsg.msgData.u8_Data))) == -1)
	{
		CFR_DBG_ERROR( "SpiRxThread Error: read() failed\n" );
	}
	else
	{
#if 0
		CFR_DBG_OUT( "SpiRxThread: received %d bytes\n", LinMsg.msgData.nLength );

		int i;
		CFR_DBG_OUT("RX: ");
		for (i=0; i < LinMsg.msgData.nLength; i++)
		{
			CFR_DBG_OUT("%02x ", LinMsg.msgData.u8_Data[i]);
		}
		CFR_DBG_OUT(", len = %d\n", LinMsg.msgData.nLength);
#endif
            
		/* Send what we received to callback thread */
		if( msgQId >= 0 )
		{
			LinMsg.msgType = 1;
                           
			if( msgsnd( msgQId, &LinMsg, nMsgSize, 0 ) == -1 )
			{
				CFR_DBG_ERROR( "SpiRxThread: msgsnd ERROR:%d\n", errno );
			}
		}
		else
		{
			CFR_DBG_ERROR( "SpiRxThread: invalid msgQId:%d\n", msgQId );
		} 	
	}
   }

   return NULL;
}



