/*!
*  \file       tcx_log.c
*  \brief      Logging functionality for the test application
*  \Author     maurer
*
*  @(#)  %filespec: -1 %
*
*******************************************************************************
*  \par  History
*  \n==== History =============================================================\n
*  date        name     version   action                                       \n
*  ----------------------------------------------------------------------------\n
*   10-jul-09  maurer     1.0       Initial version                            \n
*******************************************************************************
*  COPYRIGHT DOSCH & AMAND RESEARCH GMBH & CO.KG
*  DOSCH & AMAND RESEARCH GMBH & CO.KG Confidential
*
*******************************************************************************/
#include <stdio.h>
#include "cmbs_platf.h"

#include "cmbs_api.h"
#include "cfr_ie.h"
#include "cfr_mssg.h"
#include "appcmbs.h"
#include "appsrv.h"
#include "appcall.h"
#include "cmbs_str.h"
#include "tcx_log.h"

FILE * g_fpLogFile = NULL;

u8  g_u8_tcx_LogOutgoingPacket[1024];
u32 g_u32_tcx_LogOutgoingPacket_WriteCounter;


//    ========== tcx_LogOpenLogfile ===========
/*!
      \brief             Open the logfile
      \param[in]         pszLogFile The name of the logfile
      \return            1 if successful, 0 if there was an error

*/
int tcx_LogOpenLogfile(char * psz_LogFile)
{
   char buffer[] = "CMBS-Logfile\n";

   g_fpLogFile = fopen(psz_LogFile, "w");

   if(g_fpLogFile == NULL)
   {
      return 0;
   }

   fwrite (buffer, 1, sizeof(buffer)-1, g_fpLogFile );

   fflush(g_fpLogFile);

   return 1;
}


//    ========== tcx_LogutgoingPacketPrepare ===========
/*!
      \brief             Prepare to write an outgoing packet
      \return            none

*/
void tcx_LogOutgoingPacketPrepare(void)
{
   g_u32_tcx_LogOutgoingPacket_WriteCounter = 0;
}


//    ========== tcx_LogOutgoingPacketPartWrite ===========
/*!
      \brief             Collect part of the packet, to write them in to the logfile later
      \param[in]         Pointer to part of packet
      \param[in]         Length of the part of the packet
      \return            none

*/
void tcx_LogOutgoingPacketPartWrite(u8* pu8_Buffer, u16 u16_Size)
{
   if(g_fpLogFile != NULL)
   {
      memcpy(g_u8_tcx_LogOutgoingPacket + g_u32_tcx_LogOutgoingPacket_WriteCounter, pu8_Buffer, u16_Size);
      g_u32_tcx_LogOutgoingPacket_WriteCounter += u16_Size;
   }
}


//    ========== tcx_LogOutgoingPacketWriteFinish ===========
/*!
      \brief             Write collected outgoing data to logfile
      \return            none

*/
void tcx_LogOutgoingPacketWriteFinish()
{
   u8  u8_tcx_LogOutgoingPacketString[16 + 1024 * 3];
   u32 u32_pos;
   u32 u32_i;

   if(g_fpLogFile != NULL)
   {
      u32_pos = sprintf(u8_tcx_LogOutgoingPacketString, "TX>");

      for(u32_i = 0; u32_i < g_u32_tcx_LogOutgoingPacket_WriteCounter; u32_i++)
      {
         u32_pos += sprintf(u8_tcx_LogOutgoingPacketString + u32_pos, " %02X", g_u8_tcx_LogOutgoingPacket[u32_i]);
      }

      u32_pos += sprintf(u8_tcx_LogOutgoingPacketString + u32_pos, "\n");

      fwrite(u8_tcx_LogOutgoingPacketString, 1, u32_pos, g_fpLogFile);
      fflush(g_fpLogFile);
   }
}


//    ========== tcx_LogIncomingPacketWriteFinish ===========
/*!
      \brief             Write complete incoming data packet to logfile
      \return            none

*/
void tcx_LogIncomingPacketWriteFinish(u8* pu8_Buffer, u16 u16_Size)
{
   u8  u8_tcx_LogIncomingPacketString[16 + 1024 * 3];
   u32 u32_pos;
   u32 u32_i;

   if(g_fpLogFile != NULL)
   {
      u32_pos = sprintf(u8_tcx_LogIncomingPacketString, "RX> DA DA DA DA");

      for(u32_i = 0; u32_i < u16_Size; u32_i++)
      {
         u32_pos += sprintf(u8_tcx_LogIncomingPacketString + u32_pos, " %02X", pu8_Buffer[u32_i]);
      }

      u32_pos += sprintf(u8_tcx_LogIncomingPacketString + u32_pos, "\n");

      fwrite(u8_tcx_LogIncomingPacketString, 1, u32_pos, g_fpLogFile);
      fflush(g_fpLogFile);
   }
}


//    ========== tcx_LogCloseLogfile ===========
/*!
      \brief             Close log file
      \return            none

*/
void tcx_LogCloseLogfile(void)
{
   if( g_fpLogFile != NULL)
   {
      fclose(g_fpLogFile);
      g_fpLogFile = NULL;
   }
}


//*/
