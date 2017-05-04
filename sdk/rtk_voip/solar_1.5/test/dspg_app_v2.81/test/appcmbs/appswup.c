/*!
*	\file		appswup.c
*	\brief		handles firmware update requests
*	\Author		stein
*
*	@(#)	%filespec: appswup.c~DMZD53#6 %
*
*******************************************************************************
*	\par	History
*	\n==== History ============================================================\n
*	date			name		version	 action                                          \n
*	----------------------------------------------------------------------------\n
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if ! defined ( WIN32 )
#include <unistd.h>
#include <termios.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/msg.h>
#else
#include <conio.h>
#include <io.h>
#endif

#include <fcntl.h>
#include <errno.h>

#include "cmbs_api.h"
#include "cfr_ie.h"
#include "cfr_mssg.h"

#include "appcmbs.h"
#include "appswup.h"

#pragma pack(push) //Store defaults
#pragma pack(1) // Byte packed

typedef struct {
	unsigned char	md5[16]; // MD5 chksum
} t_swup_ChkSum;

typedef struct
{
   unsigned int     MagicNumber;
   unsigned char    u8_ImgHeaderSize;   /* Image Header size in bytes */
   unsigned char    u8_Version[3];      /* Version in BCD coded format i.e 03.11.10 */
   unsigned int   u32_Build;            /* Version build */
   unsigned int     u32_BuildDate;      /* Build Date ddmmyyyy */
   char             u8_ImgType;                /* 1 - Handset image 0 - Base image */
   char             u8_Reserved;
   unsigned short   u16_EepromLayoutVer;       /*  Version of EEPROM Layout */
   unsigned int     u32_ImageCodeSize;  /* Code Image size in bytes */
   t_swup_ChkSum    ImageCodeChecksum;  /* 16 bytes checksum for Code Image */
   unsigned int     u32_ImageDataSize;  /* Data Image size in bytes */
   t_swup_ChkSum    ImageDataChecksum;  /* 16 bytes checksum for Data Image */
}  ST_IMAGE_HEADER, * PST_IMAGE_HEADER;
#pragma pack(pop) // Restore defaults


#if defined ( WIN32 )
HANDLE         swupFd;
#else
int            swupFd;
#endif


#define PACKETSIZE 128	// spi restrictions 
unsigned char  swupBuffer[PACKETSIZE];
int            nChunk;  // just for debugging

#if defined ( WIN32 )
void           app_FwUpdStart( HANDLE fd )
{
   int         nBytes = 0;

   swupFd = fd;

   SetFilePointer( swupFd, 0, NULL, FILE_BEGIN );
   ReadFile( swupFd, swupBuffer, sizeof(ST_IMAGE_HEADER), &nBytes, NULL );

   if( nBytes > 0 )
   {
      nChunk = 1;
      APPCMBS_INFO(( "FwUpdStart chunk%4d %d bytes\n", nChunk, nBytes ));
      cmbs_dsr_fw_UpdateStart( g_cmbsappl.pv_CMBSRef, swupBuffer, nBytes );
   }
   else
   {
      APPCMBS_ERROR(( "APPSWUP-ERROR: !!! CMBS_EV_DSR_FW_UPD_START failed. Couldn't read file\n"));
      CloseHandle( swupFd );
   }
}
#else
void           app_FwUpdStart( int fd )
{
   int         nBytes = 0;

   swupFd = fd;

   nBytes = read( swupFd, swupBuffer, sizeof(ST_IMAGE_HEADER) );

   if( nBytes > 0 )
   {
      nChunk = 1;
      APPCMBS_INFO(( "FwUpdStart chunk%4d %d bytes\n", nChunk, nBytes ));
      cmbs_dsr_fw_UpdateStart( g_cmbsappl.pv_CMBSRef, swupBuffer, nBytes );
   }
   else
   {
      APPCMBS_ERROR(( "APPSWUP-ERROR: !!! CMBS_EV_DSR_FW_UPD_START failed. Couldn't read file\n"));
      close( swupFd );
   }
}
#endif


#if defined ( WIN32 )
void           app_FwUpdContinue( void )
{
   int         nBytes = 0;

   ReadFile( swupFd, swupBuffer, sizeof( swupBuffer ), &nBytes, NULL );

   if( nBytes > 0 )
   {
      nChunk++;
      APPCMBS_INFO(( "FwUpdNext  chunk%4d %d bytes\n", nChunk, nBytes ));
      cmbs_dsr_fw_UpdatePacketNext( g_cmbsappl.pv_CMBSRef, swupBuffer, (u16)nBytes );
   }
   else
   {
      if( nBytes == 0 )
      {
         cmbs_dsr_fw_UpdateEnd( g_cmbsappl.pv_CMBSRef, NULL, 0 );
      }
      else
      {
         APPCMBS_ERROR(( "APPSWUP-ERROR: !!! CMBS_EV_DSR_FW_UPD_PACKETNEXT failed. Couldn't read file\n")) ;
      }

      CloseHandle( swupFd );
   }
}
#else
void           app_FwUpdContinue( void )
{
   int         nBytes = 0;

   nBytes = read( swupFd, swupBuffer, PACKETSIZE );

   APPCMBS_INFO(( "app_FwUpdContinue bytes = %d\n", nBytes));

   if( nBytes > 0 )
   {
      nChunk++;
      APPCMBS_INFO(( "FwUpdNext  chunk%4d %d bytes\n", nChunk, nBytes ));
      cmbs_dsr_fw_UpdatePacketNext( g_cmbsappl.pv_CMBSRef, swupBuffer, nBytes );
   }
   else
   {
      if( nBytes == 0 )
      {
         cmbs_dsr_fw_UpdateEnd( g_cmbsappl.pv_CMBSRef, NULL, 0 );
      }
      else
      {
         APPCMBS_ERROR(( "APPSWUP-ERROR: !!! CMBS_EV_DSR_FW_UPD_PACKETNEXT failed. Couldn't read file\n")) ;
      }

      close( swupFd );
   }
}
#endif

void           app_FwUpdPrepare( void )
{
   cmbs_dsr_fw_UpdateStart( g_cmbsappl.pv_CMBSRef, NULL, 0 );
}

void           app_OnFwUpdStartRsp( void * pv_List )
{
   if( !app_ResponseCheck( pv_List ) )
   {
      APPCMBS_INFO(( "APPSWUP-INFO: CMBS_EV_DSR_FW_UPD_START successful\n"));
      app_FwUpdContinue();
   }
   else
   {
      APPCMBS_ERROR(( "APPSWUP-ERROR: !!! CMBS_EV_DSR_FW_UPD_START failed on target\n"));
   }
}



void           app_OnFwUpdNextRsp( void * pv_List )
{
   if( !app_ResponseCheck( pv_List ) )
   {
      APPCMBS_INFO(( "APPSWUP-INFO: CMBS_EV_DSR_FW_UPD_PACKETNEXT successful\n"));
      app_FwUpdContinue();
   }
   else
   {
      APPCMBS_ERROR(( "APPSWUP-ERROR: !!! CMBS_EV_DSR_FW_UPD_PACKETNEXT failed on target\n"));
   }
}


void           app_OnFwUpdEndRsp( void * pv_List )
{
   if( !app_ResponseCheck( pv_List ) )
   {
      APPCMBS_INFO(( "APPSWUP-INFO: CMBS_EV_DSR_FW_UPD_END successful. Please reset target\n"));
   }
   else
   {
      APPCMBS_ERROR(( "APPSWUP-ERROR: !!! CMBS_EV_DSR_FW_UPD_END failed!\nPlease reset target and try update again\n"));
   }
}

//		========== app_SwupEntity ===========
/*!
		\brief		 CMBS entity to handle response information from target side
		\param[in]	 pv_AppRef		 application reference
		\param[in]	 e_EventID		 received CMBS event
		\param[in]	 pv_EventData	 pointer to IE list
		\return	 	 <int>

*/

int            app_SwupEntity( void * pv_AppRef, E_CMBS_EVENT_ID e_EventID, void * pv_EventData )
{
                                 // ensure that the compiler does not print out a warning
   if (pv_AppRef){};

   switch( e_EventID )
   {
      case  CMBS_EV_DSR_FW_UPD_START_RES:
         app_OnFwUpdStartRsp(pv_EventData);
         break;

      case CMBS_EV_DSR_FW_UPD_PACKETNEXT_RES:
         app_OnFwUpdNextRsp(pv_EventData);
         break;

      case CMBS_EV_DSR_FW_UPD_END_RES:
         app_OnFwUpdEndRsp(pv_EventData);
         break;

      default:
         return FALSE;
   }

  return TRUE;
}


//*/
