/*!
*  \file       cfr_uart.c
*  \brief      UART implementation of linux host side
*  \author     stein
*
*  @(#)  %filespec: cfr_uart.c~DMZD53#2 %
*
*******************************************************************************
*  \par  History
*  \n==== History ============================================================\n
*  date        name     version   action                                          \n
*  ----------------------------------------------------------------------------\n
*  14-feb-09   R.Stein    1         Initialize \n
*******************************************************************************/

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
#include "cfr_uart.h"   /* packet handler */
#include "cfr_debug.h"  /* debug handling */


//    ========== cfr_uartThread ===========
/*!
      \brief            UART data receive pump. if data is available a message is send
                        to cfr_cmbs task.

      \param[in]        pVoid          pointer to CMBS instance object

      \return           <void *>       return always NULL

*/

void  *           cfr_uartThread( void * pVoid )
{
   PST_CMBS_API_INST
                  pInstance = (PST_CMBS_API_INST)pVoid;
   int            fdDevCtl  = pInstance->fdDevCtl;
   int            msgQId    = pInstance->msgQId;
   fd_set         input_fdset;
   size_t         nMsgSize;

   ST_CMBS_LIN_MSG
                  LinMsg;

//   CFR_DBG_OUT( "UART Thread: ID:%lu running\n", (unsigned long)pthread_self() );

   nMsgSize = sizeof( LinMsg.msgData );

   /* Never ending loop.
      Thread will be exited automatically when parent thread finishes.
   */
   while( 1 )
   {
      FD_ZERO( &input_fdset );
      FD_SET( fdDevCtl, &input_fdset);

      if( select(fdDevCtl+1, &input_fdset, NULL, NULL, NULL) == -1 )
      {
         CFR_DBG_ERROR( "UartThread Error: select() failed\n" );
       /*!\todo exception handling is needed !*/
      }
      else
      {
         if( FD_ISSET(fdDevCtl, &input_fdset) )
         {
            memset( &LinMsg.msgData, 0, sizeof(LinMsg.msgData) );

            /* Reading available data from serial interface */
            if( (LinMsg.msgData.nLength = read(fdDevCtl, LinMsg.msgData.u8_Data, sizeof(LinMsg.msgData.u8_Data))) == -1 )
            {
               CFR_DBG_ERROR( "UartThread Error: read() failed\n" );
            }
            else
            {
//               CFR_DBG_OUT( "UartThread: received %d bytes\n", LinMsg.msgData.nLength );

               /* Send what we received to callback thread */
               if( msgQId >= 0 )
               {
                  LinMsg.msgType = 1;

                  if( msgsnd( msgQId, &LinMsg, nMsgSize, 0 ) == -1 )
                  {
                     CFR_DBG_ERROR( "UartThread: msgsnd ERROR:%d\n", errno );
                  }
               }
               else
               {
                  CFR_DBG_ERROR( "UartThread: invalid msgQId:%d\n", msgQId );
               }
            }
         }
      }
   }

   return NULL;
}


//    ==========    cfr_wait_till_characters_transmitted ===========
/*!
      \brief             Wait till characters are transmitted

      \param[in,out]     fd    pointer to packet part

      \return            < none >

*/

void cfr_uartWaitPacketPartTransmitFinished(int fd)
{
   int rv;
   fd_set writefds;

   while(1)
   {
      FD_ZERO(&writefds);
      FD_SET(fd, &writefds);

      rv=select(fd+1, NULL, &writefds, NULL, NULL);

      if (rv >= 0) break;
      if (errno != EINTR) break;
   }
}

//    ==========    cfr_uartPacketPartWrite ===========
/*!
      \brief             write partly the packet into communication device

      \param[in,out]     pu8_Buffer    pointer to packet part

      \param[in,out]     u16_Size      size of packet part

      \return            < int >       currently, alway 0

*/

int               cfr_uartPacketPartWrite( u8* pu8_Buffer, u16 u16_Size )
{
   // int    i;

   // CFR_DBG_OUT( "PacketPartWrite: " );
   // for (i=0; i < u16_Size; i++ )
   // {
   //   CFR_DBG_OUT( "%02x ",pu8_Buffer[i] );
   //   write( g_CMBSInstance.fdDevCtl, pu8_Buffer + i, 1 );
   // }
   // CFR_DBG_OUT( "\n" );

   write( g_CMBSInstance.fdDevCtl, pu8_Buffer, u16_Size);

   cfr_uartWaitPacketPartTransmitFinished(g_CMBSInstance.fdDevCtl);

   // For logging sent data packets
   if( g_CMBSInstance.st_ApplSlot.pFnCbLogBuffer.pfn_cmbs_api_log_outgoing_packet_part_write_cb != NULL )
   {
      g_CMBSInstance.st_ApplSlot.pFnCbLogBuffer.pfn_cmbs_api_log_outgoing_packet_part_write_cb( pu8_Buffer, u16_Size );
   }

   return 0;
}
//    ========== cfr_uartPacketWriteFinish ===========
/*!
      \brief             Currently dummy function is not needed on host side

      \param[in,out]     u8_BufferIDX  buffer index

      \return            < none >

*/

void              cfr_uartPacketWriteFinish( u8 u8_BufferIDX )
{
   // For logging sent data packets
   if(u8_BufferIDX == CFR_BUFFER_UART_TRANS)
   {
      if( g_CMBSInstance.st_ApplSlot.pFnCbLogBuffer.pfn_cmbs_api_log_outgoing_packet_write_finish_cb != NULL )
      {
         g_CMBSInstance.st_ApplSlot.pFnCbLogBuffer.pfn_cmbs_api_log_outgoing_packet_write_finish_cb();
      }
   }
}

//    ========== cfr_uartPacketPrepare  ===========
/*!
      \brief             Currently dummy function is not needed on host side

      \param[in,out]     u16_size      size of to be submitted packet for transmission

      \return           < CFR_E_RETVAL >

*/

CFR_E_RETVAL      cfr_uartPacketPrepare( u16 u16_size )
{
   // dummy function
   if( u16_size ){}  // eliminate compiler warnings

   // For logging sent data packets
   if( g_CMBSInstance.st_ApplSlot.pFnCbLogBuffer.pfn_cmbs_api_log_outgoing_packet_prepare_cb != NULL )
   {
      g_CMBSInstance.st_ApplSlot.pFnCbLogBuffer.pfn_cmbs_api_log_outgoing_packet_prepare_cb();
   }

   return CFR_E_RETVAL_OK;
}

//    ========== cfr_uartDataTransmitKick  ===========
/*!
      \brief             Currently dummy function is not needed on host side

      \param[in,out]     < none >

      \return           < CFR_E_RETVAL >

*/

void              cfr_uartDataTransmitKick( void )
{
   // dummy function
}

//    ========== cfr_uartInitialize ===========
/*!
      \brief             open the serial communication interface with relevant parameter sets

      \param[in,out]     pst_Config    pointer to UART configuration

      \return           < int >        if failed returns -1, otherwise 0

*/

int          cfr_uartInitialize( PST_UART_CONFIG pst_Config )
{
   char           szDevName[128];
   int            fd;
   struct termios term_attr;
   // generate device name
   if( pst_Config->u8_Port < 1 )
   {
//      CFR_DBG_WARN( "Warning: Invalid port COM%d. Using default COM1\n", pst_Config->u8_Port );
      sprintf( szDevName, "/dev/ttyS0" );
   }
   else
   {
     sprintf( szDevName, "/dev/ttyS%d", pst_Config->u8_Port -1 );
   }
   // open device
   if( (fd = open(szDevName, O_RDWR | O_NOCTTY | O_NDELAY)) == -1 )
   {
      CFR_DBG_ERROR( "Error: Can't open device %s\n", szDevName );
      return -1;
   }
   else
   {
      CFR_DBG_OUT( "Opened %s\n", szDevName );

      /* Configure terminal attributes */
      if( tcgetattr(fd, &term_attr) != 0 )
      {
         CFR_DBG_ERROR( "Error: tcgetattr() for TERM_DEVICE failed\n" );
       close ( fd );
         return -1;
      }
     /* setup the serial device communication properties */
#ifdef UART_DECT_CMBS_HW_FLOW_CTRL
      CFR_DBG_OUT( "Support HW Flow Control.\n");
      term_attr.c_cflag = CS8|CREAD|CLOCAL|CRTSCTS;
#else
      term_attr.c_cflag = CS8|CREAD|CLOCAL;
#endif
                                                               /* Control flag
                                                               CS8     : 8n1 (8bit,no parity,1 stopbit)
                                                               CREAD   : enable receiving characters
                                                               CLOCAL  : local connection, no modem control
                                                               CRTSCTS : output hardware flow control
                                                               */
      /* Raw data transmission; No pre- or post-processing */
      term_attr.c_iflag = 0;                                   // Input flag
      term_attr.c_oflag = 0;                                   // Output flag
      term_attr.c_lflag = 0;                                   // Local flag

      if( cfsetispeed(&term_attr, CMBS_UART_BAUD) == -1 )
      {
         CFR_DBG_ERROR( "Error: failed to set input baud rate\n" );
         close (fd);
       return -1;
      }
      else
      {
      	CFR_DBG_OUT( "Set in baud rate to %d\n", CMBS_UART_BAUD );
      }

      if( cfsetospeed(&term_attr, CMBS_UART_BAUD) == -1 )
      {
         CFR_DBG_ERROR( "Error: failed to set output baud rate\n" );
         close (fd);
       return -1;
      }
      {
      	CFR_DBG_OUT( "Set out baud rate to %d\n", CMBS_UART_BAUD );
      }

      if( tcsetattr(fd, TCSAFLUSH, &term_attr) != 0 )
      {
         CFR_DBG_ERROR( "Error: tcsetattr() for TERM_DEVICE failed\n" );
         close (fd);
       return -1;
      }
   }

   return   fd;
}


//todo: rename ST_UART_CONFIG to ST_SERIAL_CONFIG
int          cfr_usbInitialize( PST_UART_CONFIG pst_Config )
{
   char           szDevName[128];
   int            fd;
   struct termios term_attr;

   // generate device name
   sprintf( szDevName, "/dev/ttyACM%d", pst_Config->u8_Port );

   // open device
   if( (fd = open(szDevName, O_RDWR | O_NOCTTY | O_NDELAY)) == -1 )
   {
      CFR_DBG_ERROR( "Error: Can't open device %s\n", szDevName );
      return -1;
   }
   else
   {
      CFR_DBG_OUT( "Opened %s\n", szDevName );

      /* Configure terminal attributes */
      if( tcgetattr(fd, &term_attr) != 0 )
      {
         CFR_DBG_ERROR( "Error: tcgetattr() for TERM_DEVICE failed\n" );
       close ( fd );
         return -1;
      }
     /* setup the serial device communication properties */
      term_attr.c_cflag = CS8|CREAD|CLOCAL;//|CRTSCTS;
                                                               /* Control flag
                                                               CS8     : 8n1 (8bit,no parity,1 stopbit)
                                                               CREAD   : enable receiving characters
                                                               CLOCAL  : local connection, no modem control
                                                               CRTSCTS : output hardware flow control
                                                               */
      /* Raw data transmission; No pre- or post-processing */
      term_attr.c_iflag = 0;                                   // Input flag
      term_attr.c_oflag = 0;                                   // Output flag
      term_attr.c_lflag = 0;                                   // Local flag

      if( cfsetispeed(&term_attr, CMBS_UART_BAUD) == -1 )
      {
         CFR_DBG_ERROR( "Error: failed to set input baud rate\n" );
         close (fd);
       return -1;
      }

      if( cfsetospeed(&term_attr, CMBS_UART_BAUD) == -1 )
      {
         CFR_DBG_ERROR( "Error: failed to set output baud rate\n" );
         close (fd);
       return -1;
      }

      if( tcsetattr(fd, TCSAFLUSH, &term_attr) != 0 )
      {
         CFR_DBG_ERROR( "Error: tcsetattr() for TERM_DEVICE failed\n" );
         close (fd);
       return -1;
      }
   }

   return   fd;
}
