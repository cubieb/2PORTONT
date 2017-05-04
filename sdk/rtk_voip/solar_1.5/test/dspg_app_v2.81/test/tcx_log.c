/*!
*  \file       tcx_log.c
*  \brief      Logging functionality for the test application
*  \Author     maurer
*
*  @(#)  %filespec: tcx_log.c~DMZD53#3 %
*
*******************************************************************************
*  \par  History
*  \n==== History =============================================================\n
*  date        name     version   action                                       \n
*  ----------------------------------------------------------------------------\n
*   10-jul-09  maurer     1.0       Initial version                            \n
*   14-dec-09  sergiym     ?        Add OsTrace to CMBS log                            \n
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

/* Trace features */
#define OS08_TRACE
#include "cos08trc.h"   // ..\core\ext\dspg\vegaone\scorpion4\vdsw-ftvone
#define OS_DEBUG 5
#define VDSW_SMS 1
#define VDSW_FEATURES 1
#define cos19_c
/* State names, event names */
#include "bsd09ddl.h"   // ..\core\ext\dspg\vegaone\scorpion4\vdsw-ftvone
#undef cos19_c
#undef VDSW_FEATURES
#undef VDSW_SMS
#undef OS_DEBUG
/* OS Interface */
#include "cos00int.h"   // ..\core\ext\dspg\vegaone\scorpion4\vdsw-ftvone


/* Dummy functions as workaround for result of ddl2c tool */
void FTCSM(void) {}
void FTDSR(void) {}
void FTSS(void) {}
void FTMM(void) {}
void FTMLP(void) {}
void p_hl17_LUXProcess(void) {}
void FTHE(void) {}
void FTCH(void) {}
void FTCC(void) {}
void FTRMC(void) {}
void TRACE_PROCESS(void) {}
void FTMI(void) {}
void CSA(void) {}
void FTMMS(void) {}
void FTCLI(void) {}
void FTLA(void) {}
void TERMINAL(void) {}
void FTTTS(void) {}
void CMBSTASK(void) {}
void FWUP_TASK (void) {}
void FTFWUP (void) {}


FILE * g_fpLogFile = NULL;
FILE * g_fpTraceFile = NULL;

u8  g_u8_tcx_LogOutgoingPacket[1024];
u32 g_u32_tcx_LogOutgoingPacket_WriteCounter;

u8 G_u8_tcx_TraceBuffer_InUse = 0;
u8 G_u8_tcx_TraceBuffer_Pos = 0;
u8 G_u8_tcx_TraceBuffer_Size = 0;
u8 G_u8_tcx_TraceBuffer[255];

u32 G_u32_tcx_TraceTime = 0;
u16 G_u16_tcx_TraceTickPrev = 0;


//    ========== tcx_LogOpenLogfile ===========
/*!
      \brief             Open the logfile
      \param[in]         pszLogFile The name of the logfile
      \return            1 if successful, 0 if there was an error

*/
int tcx_LogOpenLogfile(char * psz_LogFile)
{
   char buffer[] = "DA1260-Logfile\n";

   g_fpLogFile = fopen(psz_LogFile, "w");

   if(g_fpLogFile == NULL)
   {
      return 0;
   }

   fwrite (buffer, 1, sizeof(buffer)-1, g_fpLogFile );

   fflush(g_fpLogFile);

   return 1;
}


//    ========== tcx_LogOpenTracefile ===========
/*!
      \brief             Open the tracefile
      \param[in]         pszTraceFile The name of the tracefile
      \return            1 if successful, 0 if there was an error

*/
int tcx_LogOpenTracefile(char * psz_TraceFile)
{
   g_fpTraceFile = fopen(psz_TraceFile, "wb");

   if(g_fpTraceFile == NULL)
   {
      return 0;
   }
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
   char tcx_LogOutgoingPacketString[16 + 1024 * 3];
   u32  u32_pos;
   u32  u32_i;

   if(g_fpLogFile != NULL)
   {
      u32_pos = sprintf(tcx_LogOutgoingPacketString, "TX>");

      for(u32_i = 0; u32_i < g_u32_tcx_LogOutgoingPacket_WriteCounter; u32_i++)
      {
         u32_pos += sprintf(tcx_LogOutgoingPacketString + u32_pos, " %02X", g_u8_tcx_LogOutgoingPacket[u32_i]);
      }

      u32_pos += sprintf(tcx_LogOutgoingPacketString + u32_pos, "\n");

      fwrite(tcx_LogOutgoingPacketString, 1, u32_pos, g_fpLogFile);
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
   char tcx_LogIncomingPacketString[16 + 1024 * 3];
   u32  u32_pos;
   u32  u32_i;

   if(g_fpLogFile != NULL)
   {
      u32_pos = sprintf(tcx_LogIncomingPacketString, "RX> DA DA DA DA");

      for(u32_i = 0; u32_i < u16_Size; u32_i++)
      {
         u32_pos += sprintf(tcx_LogIncomingPacketString + u32_pos, " %02X", pu8_Buffer[u32_i]);
      }

      u32_pos += sprintf(tcx_LogIncomingPacketString + u32_pos, "\n");

      fwrite(tcx_LogIncomingPacketString, 1, u32_pos, g_fpLogFile);
      fflush(g_fpLogFile);
   }
}


//    ========== tcx_TraceTimeUpdate ===========
/*!
      \brief             Update trace time
      \return            none

*/
void tcx_TraceTimeUpdate( u16 u16_tick )
{
   if (G_u16_tcx_TraceTickPrev <= u16_tick)
   {
      G_u32_tcx_TraceTime += u16_tick - G_u16_tcx_TraceTickPrev;
   }
   else
   {
      G_u32_tcx_TraceTime += 0x00010000 + u16_tick - G_u16_tcx_TraceTickPrev;  
   }
   G_u16_tcx_TraceTickPrev = u16_tick;
}


//    ========== tcx_TraceTimeUpdate_u8 ===========
/*!
      \brief             Update trace time
      \return            none

*/
void tcx_TraceTimeUpdate_u8( u8 u8_tick )
{
   u16 u16_ticks = (G_u16_tcx_TraceTickPrev & ~0x00FF) | u8_tick;
   if (u16_ticks < G_u16_tcx_TraceTickPrev)
   {
      u16_ticks += 0x0100;
   }
   tcx_TraceTimeUpdate(u16_ticks);
}


//    ========== tcx_TraceTimeToStr ===========
/*!
      \brief             Write trace time to string
      \return            none

*/
u32 tcx_TraceTimeToStr( char* pc_str )
{
   u8 u8_ms, u8_sec, u8_min;
   u32 u32_time = G_u32_tcx_TraceTime;
   u8_ms = u32_time % 100;
   u32_time /= 100;
   u8_sec = u32_time % 60;
   u32_time /= 60;
   u8_min = u32_time % 60;
   u32_time /= 60;
   return sprintf( pc_str, "%2d:%02d:%02d.%02d ", u32_time, u8_min, u8_sec, u8_ms );
}


//    ========== tcx_TraceBufferToStr ===========
/*!
      \brief             Write trace buffer entries to string
      \return            none

*/
u32 tcx_TraceBufferToStr( char* pc_str, u8* pu8_Buffer, u8 u8_Size )
{
   t_st_os08_TraceBuffer* p_buf;
   u32  u32_pos = 0;
   u8   u8_i;

   p_buf = (t_st_os08_TraceBuffer*)pu8_Buffer;
   switch (p_buf->u8_TraceType)
   {
      case OS08_IF:
      {
         break;
      }
      case OS08_CASE:
      {
         break;
      }
      case OS08_TASK:
      {
         break;
      }
      case OS08_OUTPUT:
      {
         break;
      }
      case OS08_OUTIPC:
      {
         break;
      }
      case OS08_TIMER:
      {
         break;
      }
      case OS08_SDL_CALL:
      {
         break;
      }
      case OS08_SDL_RET:
      {
         break;
      }
      case OS08_PSI:
      {
         if (OS08_SIZEOF_TRACE(p_buf->un_Payload.st_PSI) == u8_Size)
         {
            tcx_TraceTimeUpdate(p_buf->un_Payload.st_PSI.u16_SystemTime);
            u32_pos += tcx_TraceTimeToStr(pc_str + u32_pos);
            u32_pos += sprintf(pc_str + u32_pos, "RCV-%c   p-%d/%-16s  s-%-21s  ev-%-30s  info-%04X",
                                 (p_buf->u8_Process < OS09_1ST_LOW_PRIO_PROCESS) ? 'H' : 'L',
                                 p_buf->un_Payload.st_PSI.u8_Instance,
                                 G_pu8_os09_ProcessNames[p_buf->u8_Process],
                                 G_pu8_os09_StateNames[p_buf->u8_Process][p_buf->un_Payload.st_PSI.u8_State],
                                 G_pu8_os09_EventNames[p_buf->u8_Process][p_buf->un_Payload.st_PSI.u8_Event],
                                 p_buf->un_Payload.st_PSI.u16_XInfo);
         }
         break;
      }
      case OS08_EXIT:
      {
         if (OS08_SIZEOF_TRACE(p_buf->un_Payload.st_Exit) == u8_Size)
         {
            tcx_TraceTimeUpdate(p_buf->un_Payload.st_Exit.u16_SystemTime);
            u32_pos += tcx_TraceTimeToStr(pc_str + u32_pos);
            if (p_buf->u8_Process == OS08_IDLE_PROCESS_ID)
            {
               u32_pos += sprintf(pc_str + u32_pos, "EXIT    *idle*");            }
            else
            {
               u32_pos += sprintf(pc_str + u32_pos, "EXIT-%c  p-%-18s  s-%s",
                                    (p_buf->u8_Process < OS09_1ST_LOW_PRIO_PROCESS) ? 'H' : 'L',
                                    G_pu8_os09_ProcessNames[p_buf->u8_Process],
                                    G_pu8_os09_StateNames[p_buf->u8_Process][p_buf->un_Payload.st_Exit.u8_State]);
            }                        
         }
         break;
      }
      case OS08_EXCEPTION:
      {
         break;
      }
      case OS08_MSG_CONTENTS:
      {
         break;
      }
      case OS08_DUMP:
      {
         break;
      }
      case OS08_FUNCTION_CALL:
      {
         if (OS08_SIZEOF_TRACE(p_buf->un_Payload.st_FuncCall) <= u8_Size)
         {
            tcx_TraceTimeUpdate_u8(p_buf->un_Payload.st_FuncCall.u8_Time);  // func sequence completes within 256 ticks (because of WatchDog)
            u32_pos += tcx_TraceTimeToStr(pc_str + u32_pos);
            u32_pos += sprintf(pc_str + u32_pos, "func                          s-%-21s  ev-%-30s ",
                                 G_pu8_os09_StateNames[p_buf->u8_Process][p_buf->un_Payload.st_FuncCall.u8_State],
                                 G_pu8_os09_EventNames[p_buf->u8_Process][p_buf->un_Payload.st_FuncCall.u8_Event]);
            if (p_buf->un_Payload.st_FuncCall.u8_NrOfBytes + OS08_SIZEOF_TRACE(p_buf->un_Payload.st_FuncCall) == u8_Size)
            {
               for (u8_i = OS08_SIZEOF_TRACE(p_buf->un_Payload.st_FuncCall); u8_i < u8_Size; ++u8_i)
               {
                  u32_pos += sprintf(pc_str + u32_pos, " %02X", pu8_Buffer[u8_i]);
               }
            }
         }
         break;
      }
      case OS08_POINT:
      {
         if (OS08_SIZEOF_TRACE(p_buf->un_Payload.st_TracePoint) == u8_Size)
         {
            tcx_TraceTimeUpdate(p_buf->un_Payload.st_TracePoint.u16_SystemTime);
            u32_pos += tcx_TraceTimeToStr(pc_str + u32_pos);
            u32_pos += sprintf(pc_str + u32_pos, "point                                                  ev-%-30s  info-%04X",
                                 G_pu8_os09_EventNames[p_buf->u8_Process][p_buf->un_Payload.st_TracePoint.u8_Event],
                                 p_buf->un_Payload.st_TracePoint.u16_XInfo);
         }
         break;
      }
      case OS08_MSG_PUT:
      {
         if (OS08_SIZEOF_TRACE(p_buf->un_Payload.st_MsgPut) == u8_Size)
         {
            tcx_TraceTimeUpdate(p_buf->un_Payload.st_MsgPut.u16_SystemTime);
            u32_pos += tcx_TraceTimeToStr(pc_str + u32_pos);
            if (p_buf->un_Payload.st_MsgPut.u8_SrcProc == OS00_ILLEGAL_PROCESS)
            {
               u32_pos += sprintf(pc_str + u32_pos, "SEND-%c  *ISR*/%-14d --> d-%d/%-16s  ev-%-30s  info-%04X",
                                 (p_buf->un_Payload.st_MsgPut.u8_Instance & OS00_LIFO_MSG) ? 'H' : 'L',
                                 p_buf->un_Payload.st_MsgPut.u8_SrcInst, //todo: isr number 
                                 p_buf->un_Payload.st_MsgPut.u8_Instance & ~OS00_LIFO_MSG,
                                 G_pu8_os09_ProcessNames[p_buf->u8_Process],
                                 G_pu8_os09_EventNames[p_buf->u8_Process][p_buf->un_Payload.st_MsgPut.u8_Event],
                                 p_buf->un_Payload.st_MsgPut.u16_XInfo);
            }
            else
            {
               u32_pos += sprintf(pc_str + u32_pos, "SEND-%c  p-%d/%-16s --> d-%d/%-16s  ev-%-30s  info-%04X",
                                 (p_buf->un_Payload.st_MsgPut.u8_Instance & OS00_LIFO_MSG) ? 'H' : 'L',
                                 p_buf->un_Payload.st_MsgPut.u8_SrcInst,
                                 G_pu8_os09_ProcessNames[p_buf->un_Payload.st_MsgPut.u8_SrcProc],
                                 p_buf->un_Payload.st_MsgPut.u8_Instance & ~OS00_LIFO_MSG,
                                 G_pu8_os09_ProcessNames[p_buf->u8_Process],
                                 G_pu8_os09_EventNames[p_buf->u8_Process][p_buf->un_Payload.st_MsgPut.u8_Event],
                                 p_buf->un_Payload.st_MsgPut.u16_XInfo);
            }
         }
         break;
      }
   }
   return u32_pos;
}


//    ========== tcx_LogWriteLogBuffer ===========
/*!
      \brief             Write log buffer entries as string into logfile
      \return            none

*/
void tcx_LogWriteLogBuffer(u8* pu8_Buffer, u16 u16_Size)
{
   char tcx_LogBufferString[16 + 1024 * 3];
   u32  u32_pos;
   u32  u32_i;
   u8   c;
   // for debug
   //u8   u8_ii;

      u32_pos = sprintf(tcx_LogBufferString, "LOGBUFFER> ");

   for(u32_i = 0; u32_i < u16_Size; ++u32_i)
      {
         c = pu8_Buffer[u32_i];

      if (G_u8_tcx_TraceBuffer_InUse)
      {
         if (G_u8_tcx_TraceBuffer_Size == 0)
         {
            if (c == 0)
            {
               u32_pos += sprintf(tcx_LogBufferString + u32_pos, "\nLOGBUFFER> * * * * * BUFFER OVERFLOW * * * * *");
            }
            else
            {
               G_u8_tcx_TraceBuffer_Size = c;
            }
         }
         else
         {
            G_u8_tcx_TraceBuffer[G_u8_tcx_TraceBuffer_Pos] = c;
            if (++G_u8_tcx_TraceBuffer_Pos == G_u8_tcx_TraceBuffer_Size)
            {
               // for debug
               //u32_pos += sprintf(tcx_LogBufferString + u32_pos, "\nTrcBuf> ");
               //for (u8_ii = 0; u8_ii < G_u8_tcx_TraceBuffer_Size; ++u8_ii)
               //{
               //   c = G_u8_tcx_TraceBuffer[u8_ii];
               //   u32_pos += sprintf(tcx_LogBufferString + u32_pos, "(%02X)", c);
               //}

               u32_pos += sprintf(tcx_LogBufferString + u32_pos, "\nOsTrace> ");
               u32_pos += tcx_TraceBufferToStr(tcx_LogBufferString + u32_pos, G_u8_tcx_TraceBuffer, G_u8_tcx_TraceBuffer_Size);
               
               if(g_fpTraceFile != NULL)
               {
                  fwrite(G_u8_tcx_TraceBuffer, 1, G_u8_tcx_TraceBuffer_Size, g_fpTraceFile);
               }
               
               G_u8_tcx_TraceBuffer_InUse = 0;
               G_u8_tcx_TraceBuffer_Pos = 0;
               G_u8_tcx_TraceBuffer_Size = 0;
            }
         }
      }
      else
      {
         if (c == 0)                       G_u8_tcx_TraceBuffer_InUse = 1;
         else if( c >= 0x20 && c <= 0x7e ) u32_pos += sprintf(tcx_LogBufferString + u32_pos, "%c", c);
         else if( c == 0x0a )         u32_pos += sprintf(tcx_LogBufferString + u32_pos, "\nLOGBUFFER> ");
         else                         u32_pos += sprintf(tcx_LogBufferString + u32_pos, "(%02X)", c);
      }

   }

      u32_pos += sprintf(tcx_LogBufferString + u32_pos, "\n");
   if(g_fpLogFile != NULL)
   {
      fwrite(tcx_LogBufferString, 1, u32_pos, g_fpLogFile);
      fflush(g_fpLogFile);
   }
   if(g_fpTraceFile != NULL)
   {
      fflush(g_fpTraceFile);
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

//    ========== tcx_LogCloseTracefile ===========
/*!
      \brief             Close trace file
      \return            none

*/
void tcx_LogCloseTracefile(void)
{
   if( g_fpTraceFile != NULL)
   {
      fclose(g_fpTraceFile);
      g_fpTraceFile = NULL;
   }
}

//*/
