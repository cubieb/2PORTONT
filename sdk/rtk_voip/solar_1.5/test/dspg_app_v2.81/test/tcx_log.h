/*!
*  \file       tcx_log.h
*  \brief
*  \Author     maurer
*
*  @(#)  %filespec: tcx_log.h~DMZD53#2 %
*
*******************************************************************************
*  \par  History
*  \n==== History ============================================================\n
*  date        name     version   action                                          \n
*  ----------------------------------------------------------------------------\n
*******************************************************************************/

#if   !defined( TCX_LOG_H )
#define  TCX_LOG_H


#if defined( __cplusplus )
extern "C"
{
#endif

int tcx_LogOpenLogfile(char * psz_LogFile);

void tcx_LogOutgoingPacketPrepare(void);

void tcx_LogOutgoingPacketPartWrite(u8* pu8_Buffer, u16 u16_Size);

void tcx_LogOutgoingPacketWriteFinish(void);

void tcx_LogIncomingPacketWriteFinish(u8* pu8_Buffer, u16 u16_Size);

void tcx_LogWriteLogBuffer(u8* pu8_Buffer, u16 u16_Size);

void tcx_LogCloseLogfile(void);

int tcx_LogOpenTracefile(char * psz_TraceFile);

void tcx_LogCloseTracefile(void);

#if defined( __cplusplus )
}
#endif

#endif   // TCX_LOG_H
//*/
