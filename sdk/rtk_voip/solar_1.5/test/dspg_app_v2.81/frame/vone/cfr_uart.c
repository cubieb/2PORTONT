/*!
*  \file       cfr_uart.c
*  \brief      framework uart implementation for VegaONE software suite
*  \Author     kelbch
*
* \todo DnA - DSPG UART connectivity description
*******************************************************************************
*  \par  History
*  \n==== History ============================================================\n
*  date        name     version   action                                          \n
*  ----------------------------------------------------------------------------\n
* 14-feb-09    kelbch      1      Initialize\n
* 31-may-09    kelbch      2      Receive character also in block mode\n
*******************************************************************************/
#include "cg0type.h"
#include "tclib.h"
#include "embedded.h"
//#ifdef 0
#include "bsd09cnf.h"      /* component-globalal, system configuration */
#include "bsd09ddl.h"      /* messages and processes */
//#endif
#include "cmbs_int.h"      /* internal API structure and defines */
#include "cfr_uart.h"         /* interface of uart packagetizer */
#include "cfr_debug.h"  /* debug information */
#include "cdr52spi.h"
#include "cos00int.h"
#include "tapp_log.h"


#define 		MAX_DATA 255
u8  			data_Receive_from_spi[MAX_DATA] ;

//GSN
extern ST_CFR_UARTHDL g_UARTHandler;

#define           CFR_UART_DBG_OUT     printf

extern void       p_dr18_cmbsTransmit( void );

void              cfr_uartTrace( void );

#ifdef USE_SPI_TRANSPORT
//================================================================================
// Function name: "spi_Transport_callback"
// Purpose: it is high layer application callback function and called if the transport layer has some new data
// input values:
//output value: void

void spi_Transport_callback(   t_spi_Trs_Event en_Event , u16 u16_nBytes  )
{	

	switch ( en_Event)
	{
		case SPI_EV_END_RCV:
			cfr_uartDataReceiveGet ( data_Receive_from_spi, p_dr52_RecvData ( MAX_DATA, data_Receive_from_spi ) );

			// loop back test
			//p_dr52_RecvData ( MAX_DATA, data_Receive_from_spi );
			//p_dr52_SendData ( 1,  data_Receive_from_spi); // create a loopback just for the testing

			break; 
			
		case SPI_EV_END_SND:
			p_os10_PutMsgInfo(CMBSTASK_ID, 0, CMBS_TASK_UART_CFM, 0 );
			break;

		default:
			//error();
			break;
	}

}
#endif

//================================================================================
void              cfr_BufferPacketInitialize( PST_CFR_BUFFER pst_Buffer, u8 * pu8_Buffer, u16 u16_Size )
{
   memset ( pst_Buffer, 0, sizeof(ST_CFR_BUFFER) );
   pst_Buffer->pu8_Buffer = pu8_Buffer;
   pst_Buffer->u16_Size   = u16_Size;
}

CFR_E_RETVAL      cfr_BufferPacketAlign ( PST_CFR_BUFFER pst_Buffer, u16 u16_Size )
{
   if( pst_Buffer->u16_WRIDX & 0x01)
   {
      // align word
      pst_Buffer->u16_WRIDX ++;
   }

   // Bugfix for ID19: There was a problem when a big packet (e.g. 150 bytes) was sent, but the remaining space was too small
   // The WRIDX was reset to 0, but RDIDX remains set. Because the same packet was sent, no kick transmission was done.
   // Answer on request (get flex 125 byte) was never sent out...
   if ( pst_Buffer->u16_WRIDX == pst_Buffer->u16_RDIDX )
   {
      pst_Buffer->u16_WRIDX = 0;
      pst_Buffer->u16_RDIDX = 0;
   }

   if( (u16_Size +CFR_BUFFER_PACKET_HEADER_SIZE ) >= (pst_Buffer->u16_Size - pst_Buffer->u16_WRIDX))
   {
      //    CFR_DBG_OUT( "%d >= %d\n", (u16_Size +CFR_BUFFER_PACKET_HEADER_SIZE ),(pst_Buffer->u16_Size - pst_Buffer->u16_WRIDX));
      // check wrap is successful
      if( ((u16_Size +CFR_BUFFER_PACKET_HEADER_SIZE) >= pst_Buffer->u16_RDIDX ) &&
           pst_Buffer->u16_RDIDX != pst_Buffer->u16_WRIDX )
      {
         CFR_DBG_ERROR( "cfr_BufferPacketAlign ERROR u16_Size=%d CFR_BUFFER_PACKET_HEADER_SIZE=%d pst_Buffer->u16_RDIDX=%d pst_Buffer->u16_WRIDX=%d pst_Buffer->u16_Size=%d\n",
                        u16_Size, CFR_BUFFER_PACKET_HEADER_SIZE, pst_Buffer->u16_RDIDX, pst_Buffer->u16_WRIDX, pst_Buffer->u16_Size );
         return CFR_E_RETVAL_ERR;
      }
      // handle wrap sync
      if( (pst_Buffer->u16_WRIDX + CFR_BUFFER_PACKET_HEADER_SIZE) < pst_Buffer->u16_Size )
      {
         *(u16*)(pst_Buffer->pu8_Buffer + pst_Buffer->u16_WRIDX) = CFR_BUFFER_PACKET_WRAPSYNC;
      }

      pst_Buffer->u16_WRIDX = 0;
   }

   // next packet is inside, store packed IDX
   pst_Buffer->u16_LastPackIDX = pst_Buffer->u16_WRIDX;

   *(u16*)(pst_Buffer->pu8_Buffer + pst_Buffer->u16_WRIDX) = u16_Size;
   pst_Buffer->u16_WRIDX += sizeof(u16);

   return CFR_E_RETVAL_OK;
}

CFR_E_RETVAL      cfr_BufferPacketWrite( PST_CFR_BUFFER pst_Buffer, u8 * pu8_Buffer, u16 u16_Size )
{
   memcpy( pst_Buffer->pu8_Buffer + pst_Buffer->u16_WRIDX, pu8_Buffer, u16_Size );
   pst_Buffer->u16_WRIDX += u16_Size;

   return CFR_E_RETVAL_OK;
}

u8 *              cfr_BufferPacketRead( PST_CFR_BUFFER pst_Buffer, u16* pu16_Size )
{
   u16   u16_Size = 0;

   if( pst_Buffer->u16_RDIDX == pst_Buffer->u16_WRIDX )
   {
      * pu16_Size = 0;
      return NULL;
   }
   // get next lengh of packet
   if( pst_Buffer->u16_RDIDX & 0x01 )
   {
      // word align
      pst_Buffer->u16_RDIDX ++;
   }

   if( pst_Buffer->u16_RDIDX + CFR_BUFFER_PACKET_HEADER_SIZE >= pst_Buffer->u16_Size )
   {
      pst_Buffer->u16_RDIDX = 0;
   }

   u16_Size = *(u16*)(pst_Buffer->pu8_Buffer + pst_Buffer->u16_RDIDX);

   if( u16_Size == CFR_BUFFER_PACKET_WRAPSYNC )
   {
      pst_Buffer->u16_RDIDX = 0;
      u16_Size = *(u16*)(pst_Buffer->pu8_Buffer + pst_Buffer->u16_RDIDX);
   }

   *pu16_Size = u16_Size;
   return pst_Buffer->pu8_Buffer + pst_Buffer->u16_RDIDX + CFR_BUFFER_PACKET_HEADER_SIZE;
}

//    ========== cfr_BufferPacketFree ===========
/*!
      \brief          free a packet of buffer

      \param[in]      pst_Buffer     pointer to packet buffer

      \return         < none >

*/

void              cfr_BufferPacketFree( PST_CFR_BUFFER pst_Buffer )
{
   u16 u16_Size;

   u16_Size = *(u16*)(pst_Buffer->pu8_Buffer + pst_Buffer->u16_RDIDX);

   if( u16_Size )
   {
      pst_Buffer->u16_RDIDX += (u16_Size + CFR_BUFFER_PACKET_HEADER_SIZE);

      if( pst_Buffer->u16_RDIDX & 0x01 )
      {
         pst_Buffer->u16_RDIDX ++;
      }
      // packet consumed
      pst_Buffer->u8_OUTPack ++;

   }
}
//    ========== cfr_BufferPacketUsed ===========
/*!
      \brief             check if the buffer is free or any outstanding packets are available

      \param[in,out]     pst_Buffer    pointer to buffer handler

      \return            < u8 >        return TRUE if packets are outstanding

*/

u8                cfr_BufferPacketUsed( PST_CFR_BUFFER pst_Buffer )
{
   if( pst_Buffer->u16_RDIDX == pst_Buffer->u16_WRIDX )
   {
      return FALSE;
   }
   // check also the word adjustment
   return TRUE;
}

u8 *              cfr_BufferLastPacketGet( PST_CFR_BUFFER pst_Buffer )
{
   return pst_Buffer->pu8_Buffer + pst_Buffer->u16_LastPackIDX + sizeof(u16);
}

void              cfr_BufferSkip( PST_CFR_BUFFER pst_Buffer )
{
   pst_Buffer->u16_WRIDX = pst_Buffer->u16_LastPackIDX;
   memset( pst_Buffer->pu8_Buffer + pst_Buffer->u16_WRIDX, 0, sizeof(u16) );
}

//    ==========  cfr_uartInitalize   ===========
/*!
      \brief            Initialize the  UART handler

      \param[in,out]    < none >

      \return           < none >

      The UART handler bases on DSPG implementation of cdr18UAT.c
*/

void              cfr_uartInitalize( void )
{
   memset( &g_UARTHandler, 0, sizeof(g_UARTHandler));

   cfr_BufferPacketInitialize( &g_UARTHandler.st_Buffer[CFR_BUFFER_UART_REC], g_UARTHandler.u8_BufferRead, CFR_BUFFERREAD_SIZE);
   cfr_BufferPacketInitialize( &g_UARTHandler.st_Buffer[CFR_BUFFER_UART_TRANS], g_UARTHandler.u8_BufferWrite, CFR_BUFFERWRITE_SIZE);
}


void              _cfr_uartReceiverReset( void )
{
   g_UARTHandler.u8_ReceiveSyncCnt  = 0;
   g_UARTHandler.u16_ReceivedLength = 0;
   g_UARTHandler.e_ReceiveState     = CFR_E_UART_IDLE;
}


void              _cfr_uartTransmitterReset( void )
{
   g_UARTHandler.u16_TransmitCnt = 0;
   g_UARTHandler.st_Buffer[CFR_BUFFER_UART_TRANS].u16_WRIDX = 0;
   g_UARTHandler.st_Buffer[CFR_BUFFER_UART_TRANS].u16_RDIDX = 0;
}


extern void       p_dr18_UARTReceived( void );


void              _cfr_uartPacketReceived( void )
{
   // send os message to CMBS task and reset receiver FSM.
   // check windows size of packet flow control
   // if reached, check if packet is command packet, otherwise skip it
   // if not reached signal to CMBS task, that a new packet is available
   if ( cfr_uartPacketWindowReached( CFR_BUFFER_UART_REC ) )
   {
      u8    * pu8_Buffer;
      // check if packet is command packet, otherwise skip it
      cmbs_int_cmd_FlowNOKHandle (g_UARTHandler.st_Buffer[CFR_BUFFER_UART_REC].u16_LastPackIDX   );

      pu8_Buffer = cfr_BufferLastPacketGet( g_UARTHandler.st_Buffer + CFR_BUFFER_UART_REC );
      if ( pu8_Buffer )
      {
         cmbs_int_cmd_ReceiveEarly ( pu8_Buffer );
      }

      cfr_BufferSkip ( g_UARTHandler.st_Buffer + CFR_BUFFER_UART_REC );
   }
   else
   {
      // close packet and indicate it to CMBS task
      cmbs_int_cmd_FlowRestartHandle ( g_UARTHandler.st_Buffer[CFR_BUFFER_UART_REC].u16_LastPackIDX );

      cfr_uartPacketWriteFinish( CFR_BUFFER_UART_REC );

      p_dr18_UARTReceived();
   }

   _cfr_uartReceiverReset();
}


int               cfr_uartDataReceiveGet( u8 * pu8_Data, u16 u16_DataLength )
{
   u16 u16_Adjust = 0;
   u16 u16_ConsumedBytes = 0;
   u16 u16_BytesToPacketEnd;
   u16 u16_BytesToCopy;

   if( u16_DataLength == 0)
   {
      return 0;
   }

   // packet information shall be entered into packet first.
   if ( g_UARTHandler.e_ReceiveState == CFR_E_UART_PENDING_PACKET )
   {
      u16_BytesToPacketEnd = MIN(u16_DataLength, g_UARTHandler.u16_ReceivedLength);

      cfr_BufferPacketWrite ( g_UARTHandler.st_Buffer + CFR_BUFFER_UART_REC, pu8_Data, u16_BytesToPacketEnd );

      u16_ConsumedBytes                += u16_BytesToPacketEnd;
      g_UARTHandler.u16_ReceivedLength -= u16_BytesToPacketEnd;

      if ( !g_UARTHandler.u16_ReceivedLength )
      {
         _cfr_uartPacketReceived ();
      }

      return u16_ConsumedBytes;
   }
   // packet header inspection if in other state
   // first copy received data to sync buffer;
   if ( g_UARTHandler.u8_ReceiveSyncCnt + u16_DataLength >= sizeof(g_UARTHandler.u8_ReceiveSync))
   {
      // copy data to header inspection.
      memcpy ( g_UARTHandler.u8_ReceiveSync + g_UARTHandler.u8_ReceiveSyncCnt,
         pu8_Data,
         sizeof(g_UARTHandler.u8_ReceiveSync) - g_UARTHandler.u8_ReceiveSyncCnt );

      u16_ConsumedBytes += (sizeof(g_UARTHandler.u8_ReceiveSync) - g_UARTHandler.u8_ReceiveSyncCnt);

      if ( (g_UARTHandler.u8_ReceiveSyncCnt + u16_DataLength) > sizeof( g_UARTHandler.u8_ReceiveSync) )
      {
         u16_Adjust  = * (u16*)(g_UARTHandler.u8_ReceiveSync + CMBS_SYNC_LENGTH);
         u16_Adjust -= (sizeof(g_UARTHandler.u8_ReceiveSync) - CMBS_SYNC_LENGTH);
      }
      // calculate the new data length and sync length
      g_UARTHandler.u8_ReceiveSyncCnt = sizeof(g_UARTHandler.u8_ReceiveSync);
   }
   else
   {
      memcpy ( g_UARTHandler.u8_ReceiveSync + g_UARTHandler.u8_ReceiveSyncCnt,
         pu8_Data, u16_DataLength );

      g_UARTHandler.u8_ReceiveSyncCnt += u16_DataLength;
      u16_ConsumedBytes += u16_DataLength;
   }
   // FSM for data receiving.
   // IDLE: move to PENDING_SIZE state
   // PENDING_SIZE: check if sync and total length is received
   // PENDING_PACKET: packet can be entered into packet buffer, if we have an overlap data
   switch ( g_UARTHandler.e_ReceiveState )
   {
   case  CFR_E_UART_IDLE:
      if ( g_UARTHandler.u8_ReceiveSync[0] != 0xDA )
      {
         _cfr_uartReceiverReset ( );

         return 1;
      }

      g_UARTHandler.e_ReceiveState = CFR_E_UART_PENDING_SIZE;
      // break if sync length is no reached
      if( g_UARTHandler.u8_ReceiveSyncCnt < CMBS_SYNC_LENGTH )
      {
         break;
      }
   case CFR_E_UART_PENDING_SIZE:
      // \todo sync word check
      if ( g_UARTHandler.u8_ReceiveSyncCnt >= CMBS_SYNC_LENGTH )
      {
         if ( *(u32*)g_UARTHandler.u8_ReceiveSync != CMBS_SYNC  )
         {
            _cfr_uartReceiverReset ( );

            return 1;
         }
      }

      if ( g_UARTHandler.u8_ReceiveSyncCnt >= CMBS_SYNC_LENGTH + 2 )
      {
         // according specification of 16 bit values serialization.
         u16 u16_Size = * (u16*)(g_UARTHandler.u8_ReceiveSync + CMBS_SYNC_LENGTH);

         if ( CFR_E_RETVAL_OK == cfr_BufferPacketAlign( g_UARTHandler.st_Buffer + CFR_BUFFER_UART_REC, u16_Size ) )
         {
            // change state and enter total packet length without sync.
            g_UARTHandler.e_ReceiveState = CFR_E_UART_PENDING_PACKET;
            g_UARTHandler.u16_ReceivedLength = u16_Size;
            // write packet header to packet buffer
            cfr_BufferPacketWrite ( g_UARTHandler.st_Buffer + CFR_BUFFER_UART_REC,
               g_UARTHandler.u8_ReceiveSync + CMBS_SYNC_LENGTH,
               g_UARTHandler.u8_ReceiveSyncCnt - CMBS_SYNC_LENGTH
               );

            g_UARTHandler.u16_ReceivedLength -= (g_UARTHandler.u8_ReceiveSyncCnt - CMBS_SYNC_LENGTH);

            if ( u16_Adjust )
            {
               u16_BytesToCopy = MIN(u16_Adjust, u16_DataLength - u16_ConsumedBytes);

               cfr_BufferPacketWrite ( g_UARTHandler.st_Buffer + CFR_BUFFER_UART_REC,
                  pu8_Data + u16_ConsumedBytes,
                  u16_BytesToCopy
                  );
               g_UARTHandler.u16_ReceivedLength -= u16_BytesToCopy;
               u16_ConsumedBytes                += u16_BytesToCopy;

                                 // check if packet is already complete received
               if ( !g_UARTHandler.u16_ReceivedLength )
               {
                  _cfr_uartPacketReceived ();
               }
            }
         }
         else
         {
            // flow control is needed - today reset communication
            _cfr_uartReceiverReset ( );
            // Send to host communication reseted.
            return 1;
         }
      }
   }

   return u16_ConsumedBytes;
}

u8                cfr_uartDataTransmitOngoingCheck( void )
{
   return cfr_BufferPacketUsed ( g_UARTHandler.st_Buffer + CFR_BUFFER_UART_TRANS );
}

void              cfr_uartDataTransmitKick( void )
{
   // CFR_UART_DBG_OUT ( "KICK-MISSION\n" );
   // cfr_uartTrace ();
   if ( cfr_BufferPacketUsed ( g_UARTHandler.st_Buffer + CFR_BUFFER_UART_TRANS ) )
   {
      if ( !g_UARTHandler.u16_TransmitCnt )
      {
         p_dr18_cmbsTransmit();
      }
   }
   else
   {
#ifdef OS08_TRACE
      extern u8  g_u8_tapp_log_already_flushing;

      if(g_u8_tapp_log_already_flushing)
      {
         if(_tapp_log_GetFilledBytes())
         {
            tapp_dsr_LogBufferRead( 0, 0 );
         }
         else
         {
            g_u8_tapp_log_already_flushing = 0;
         }
      }
#endif      
   }
}


u8*               cfr_uartDataTransmitGet( u16 * pu16_DataLength, u16 u16_Size )
{
   u8 * pu8_Buffer;

   pu8_Buffer = cfr_BufferPacketRead( g_UARTHandler.st_Buffer + CFR_BUFFER_UART_TRANS , pu16_DataLength );

   if ( pu8_Buffer )
   {
      u16 u16_Temp;

      u16_Temp = *pu16_DataLength - g_UARTHandler.u16_TransmitCnt;
      //      CFR_UART_DBG_OUT ( "tmp %d len %d cnt %d\n",u16_Temp,*pu16_DataLength,g_UARTHandler.u16_TransmitCnt);

      if ( !u16_Temp )
      {
         //       // packet transmitted free packet.
         cfr_BufferPacketFree ( g_UARTHandler.st_Buffer+CFR_BUFFER_UART_TRANS );
         *pu16_DataLength = 0;
         pu8_Buffer   = NULL;
         g_UARTHandler.u16_TransmitCnt = 0;
///         CFR_UART_DBG_OUT( "COMPLETE_PACKAGE\n" );
         //         cfr_uartTrace ();
      }
      else if ( u16_Temp >= u16_Size )
      {
         pu8_Buffer += g_UARTHandler.u16_TransmitCnt;
         // adjust counter
         g_UARTHandler.u16_TransmitCnt += u16_Size;
         *pu16_DataLength = u16_Size;
         //          CFR_UART_DBG_OUT ( "next byte cnt %d\n",g_UARTHandler.u16_TransmitCnt);
      }
      else
      {
         //          CFR_UART_DBG_OUT ( "rest\n");
         pu8_Buffer += g_UARTHandler.u16_TransmitCnt;
         g_UARTHandler.u16_TransmitCnt += u16_Temp;
         *pu16_DataLength = u16_Temp;
      }
   }
   return pu8_Buffer;
}

CFR_E_RETVAL      cfr_uartPacketWrite( u8 * pu8_Buffer, u16 u16_Size )
{

   if ( CFR_E_RETVAL_OK == cfr_BufferPacketAlign( g_UARTHandler.st_Buffer + CFR_BUFFER_UART_TRANS, u16_Size ) )
   {
      // write into buffer for transmission
      cfr_BufferPacketWrite ( g_UARTHandler.st_Buffer + CFR_BUFFER_UART_TRANS, pu8_Buffer, u16_Size );
      // kick transmission if needed
//      CFR_UART_DBG_OUT ( "Kicked by packet write\n" ),
         cfr_uartDataTransmitKick();

      return CFR_E_RETVAL_OK;
   }
   // not enough memory;
   return CFR_E_RETVAL_ERR_MEM;
}


CFR_E_RETVAL      cfr_uartPacketPrepare( u16 u16_Size )
{
   return cfr_BufferPacketAlign ( g_UARTHandler.st_Buffer + CFR_BUFFER_UART_TRANS, u16_Size );
}

//    ==========     cfr_uartPacketPartWrite ===========
/*!
      \brief             write partly a packet to buffer

      \param[in,out]     pu8_Buffer     pointer to be written packet part

      \param[in,out]     u16_Size       size of packet part

      \return            < none >

*/

void              cfr_uartPacketPartWrite( u8 * pu8_Buffer, u16 u16_Size )
{
   cfr_BufferPacketWrite ( g_UARTHandler.st_Buffer + CFR_BUFFER_UART_TRANS, pu8_Buffer, u16_Size );
}

//    ========== cfr_uartPacketWriteFinish ===========
/*!
      \brief             adjust packet to platform alignment

      \param[in,out]     u8_BufferIDX     buffer index

      \return           < none >

*/

void              cfr_uartPacketWriteFinish( u8 u8_BufferIDX )
{
   g_UARTHandler.st_Buffer[u8_BufferIDX].u8_INPack ++;
   // adjust write index to platform alignment
   if ( g_UARTHandler.st_Buffer[u8_BufferIDX].u16_WRIDX & 0x01 )
   {
      /*!\todo check wrap if needed to put to 0 manually */
      g_UARTHandler.st_Buffer[u8_BufferIDX].u16_WRIDX ++;
   }
}

//    ==========  cfr_uartPacketRead   ===========
/*!
      \brief            returns the next packet data pointer and the size of packet

      \param[in]        u8_BufferIDX       buffer index

      \param[in,out]    pu16_DataLength    contains the packet size

      \return           < u8 * >           pointer to next packet, if no next packet exist, NULL is returned.

*/

u8*               cfr_uartPacketRead( u8 u8_BufferIDX, u16 * pu16_DataLength )
{
   u8 * pu8_Buffer;

   pu8_Buffer = cfr_BufferPacketRead( g_UARTHandler.st_Buffer +u8_BufferIDX , pu16_DataLength );

   return  pu8_Buffer;
}



void              cfr_uartPacketFree( u8 u8_BufferIDX )
{
   cfr_BufferPacketFree ( g_UARTHandler.st_Buffer +u8_BufferIDX  );
}

//    ========== cfr_uartPacketWindowReached ===========
/*!
      \brief             check if the window size of outstanding packets is reached

      \param[in,out]     u8_BufferIDX      buffer index

      \return           < u8 >   TRUE, if reached otherwise FALSE.

   in  out               in  out
   1   0      GO         0   254    GO    ( ~out + in < WINDOWS_SIZE )
   2   0      Go         1   254    GO
   3   0      Go         2   254    STOP
   4   0      STOP
*/

u8                cfr_uartPacketWindowReached( u8 u8_BufferIDX )
{
   u8    u8_In, u8_Out;

   u8_In  = g_UARTHandler.st_Buffer[u8_BufferIDX].u8_INPack;
   u8_Out = g_UARTHandler.st_Buffer[u8_BufferIDX].u8_OUTPack;

   //   CFR_UART_DBG_OUT ( "In %d out %d\n", u8_In, u8_Out );
   if ( u8_In > u8_Out )
   {
      //     CFR_UART_DBG_OUT ( "Out Windows %d\n", u8_Out + CFR_BUFFER_WINDOW_SIZE );

      if ( u8_In > (u8_Out + CFR_BUFFER_WINDOW_SIZE) )
      {
         // flow control stop transmission
         //         CFR_UART_DBG_OUT( "STOP TRANSMISSION\n" );
         return TRUE;
      }
   }
   else if ( u8_In < u8_Out )
   {
      // cnt wrap
      u8_Out = ~u8_Out;

      if ( (u8_In + u8_Out) > CFR_BUFFER_WINDOW_SIZE )
      {
         // flow control stop transmission
         return TRUE;
      }
   }
   //   CFR_UART_DBG_OUT ( "GO TRANSMISSION\n" );

   return FALSE;
}

void              cfr_uartPacketTrace( u8 u8_BufferIDX )
{
   CFR_UART_DBG_OUT( "PACKET Buf: REC => in %d out%d, flow %d\n",
                     g_UARTHandler.st_Buffer[u8_BufferIDX].u8_INPack,
                     g_UARTHandler.st_Buffer[u8_BufferIDX].u8_OUTPack,
                     cfr_uartPacketWindowReached(u8_BufferIDX) );
}

void              cfr_uartTrace( void )
{
   CFR_UART_DBG_OUT( "##########################\n" );
   CFR_UART_DBG_OUT( "R-Buf: RD %d WR %d \n",
                     g_UARTHandler.st_Buffer[CFR_BUFFER_UART_REC].u16_RDIDX,
                     g_UARTHandler.st_Buffer[CFR_BUFFER_UART_REC].u16_WRIDX );

   CFR_UART_DBG_OUT( "T-Buf: RD %d WR %d \n",
                     g_UARTHandler.st_Buffer[CFR_BUFFER_UART_TRANS].u16_RDIDX,
                     g_UARTHandler.st_Buffer[CFR_BUFFER_UART_TRANS].u16_WRIDX );

   CFR_UART_DBG_OUT( "REC: ST %d cnt %d len %d\n",
                     g_UARTHandler.e_ReceiveState,
                     g_UARTHandler.u8_ReceiveSyncCnt,
                     g_UARTHandler.u16_ReceivedLength );

   CFR_UART_DBG_OUT( "TRA: cnt %d \n", g_UARTHandler.u16_TransmitCnt );
   CFR_UART_DBG_OUT( "##########################\n" );
}

void              cfr_uartTrace_1( void )
{
   CFR_DBG_OUT( "##########################\n" );
   CFR_DBG_OUT( "R-Buf: RD %d WR %d \n",
               g_UARTHandler.st_Buffer[CFR_BUFFER_UART_REC].u16_RDIDX,
               g_UARTHandler.st_Buffer[CFR_BUFFER_UART_REC].u16_WRIDX );

   CFR_DBG_OUT( "T-Buf: RD %d WR %d \n",
               g_UARTHandler.st_Buffer[CFR_BUFFER_UART_TRANS].u16_RDIDX,
               g_UARTHandler.st_Buffer[CFR_BUFFER_UART_TRANS].u16_WRIDX );

   CFR_DBG_OUT( "REC: ST %d cnt %d len %d\n",
               g_UARTHandler.e_ReceiveState,
               g_UARTHandler.u8_ReceiveSyncCnt,
               g_UARTHandler.u16_ReceivedLength );

   CFR_DBG_OUT( "TRA: cnt %d \n", g_UARTHandler.u16_TransmitCnt );
   CFR_DBG_OUT( "##########################\n" );
}
//*/
