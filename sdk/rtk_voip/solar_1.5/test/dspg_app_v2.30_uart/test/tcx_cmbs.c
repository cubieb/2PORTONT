/*!
	\brief main programm to run XML test cases.

*/

#include <stdio.h>
#include <stdlib.h>
#include "cmbs_platf.h"

#include "cmbs_api.h"
#include "cmbs_int.h"
#include "cfr_ie.h"
#include "cfr_mssg.h"
#include "appcmbs.h"
#include "appsrv.h"
#include "appcall.h"
#ifdef TC_XML
#include "tcx.h"
#endif
#include "tcx_keyb.h"
#include "tcx_log.h"
//#include "bus.h"


#define APPL_VERSION    0x0240
#define APPL_BUILD  0x0042

	                              // setup TDM interface
ST_TDM_CONFIG  g_st_TdmCfg;
ST_CMBS_DEV    g_st_DevMedia;
                                // setup communication path to CMBS
ST_UART_CONFIG g_st_UartCfg;
ST_CMBS_DEV    g_st_DevCtl;


void tcx_UARTIOMConfig ( u8 u8_Port )
{
   // setup port and uart for control plane
   g_st_UartCfg.u8_Port = u8_Port;
   g_st_DevCtl.e_DevType = CMBS_DEVTYPE_UART;
   g_st_DevCtl.u_Config.pUartCfg = &g_st_UartCfg;
   // setup TDM interface for payload plane
   g_st_TdmCfg.e_Type         = CMBS_TDM_TYPE_SLAVE;//CMBS_TDM_TYPE_MASTER;
   g_st_TdmCfg.e_Speed        = CMBS_TDM_PCM_2048;
   g_st_TdmCfg.e_Sync         = CMBS_TDM_SYNC_SHORT_LF;//CMBS_TDM_SYNC_LONG;
   g_st_TdmCfg.u16_SlotEnable = 0xFFFF;

   g_st_DevMedia.e_DevType         = CMBS_DEVTYPE_TDM;
   g_st_DevMedia.u_Config.pTdmCfg  = &g_st_TdmCfg;
}

void tcx_SpiIOMConfig(void)
{

	// setup dev type
	g_st_DevCtl.e_DevType = CMBS_DEVTYPE_SPI;
	//g_st_DevCtl.u_Config.pUartCfg = &g_st_UartCfg;
   
	// setup TDM interface for payload plane
	g_st_TdmCfg.e_Type         = CMBS_TDM_TYPE_SLAVE;
	g_st_TdmCfg.e_Speed        = CMBS_TDM_PCM_2048;
	g_st_TdmCfg.e_Sync         = CMBS_TDM_SYNC_SHORT_LF;
	g_st_TdmCfg.u16_SlotEnable = 0xFFFF;

	g_st_DevMedia.e_DevType         = CMBS_DEVTYPE_TDM;
	g_st_DevMedia.u_Config.pTdmCfg  = &g_st_TdmCfg;
}

void tcx_USBConfig( u8 u8_Port )
{
                                 // setup port and uart for control plane
   g_st_UartCfg.u8_Port = u8_Port;
   g_st_DevCtl.e_DevType = CMBS_DEVTYPE_USB;
   g_st_DevCtl.u_Config.pUartCfg = &g_st_UartCfg;

   g_st_DevMedia.e_DevType         = CMBS_DEVTYPE_USB;
   g_st_DevMedia.u_Config.pTdmCfg  = NULL;
}


//		========== tcx_appCleanup ===========
/*!
		\brief				 clean up API and resources

		\param[in,out]		< none >

		\return				< none >

*/

void  tcx_appCleanup(void)
{
   appcmbs_Cleanup();
}
//		========== tcx_StopSigHandler ===========
/*!
		\brief				Signal handler for linux application (CTRL-C,etc.)

		\param[in,out]		nSignal		 signal value

		\return				< none >

*/

void           tcx_StopSigHandler( int nSignal )
{
   printf( "TCX - Stop SignalHandler SIGNAL:%d\n", nSignal );
}

#ifdef TC_XML
void           tc_loop ( char * psz_File )
{
   if ( psz_File )
   {
      tc_Run( psz_File, NULL );
   }

}
#endif

u16   tcx_ApplVersionGet(void)
{
  return APPL_VERSION;
}

int   tcx_ApplVersionBuildGet(void)
{
  return APPL_BUILD;
}

#ifndef MY_TEST_MAIN

int   main( int argc, char **argv )
{
   char * psz_XMLFile = NULL;
   char * psz_LogFile = NULL;
   char * psz_TraceFile = NULL;
   char * psz_EEpromFile = NULL;
   ST_CB_LOG_BUFFER pfn_log_buffer_Cb;
   u8     u8_Port = 0;
   u8     u8_USB = 0;
   
   //Bus_Init();

   printf( "DA1260 Demo Software Version:%04X Build:%d\n", tcx_ApplVersionGet(), tcx_ApplVersionBuildGet() );
   // configuration handline
   if( argc >= 2 )
   {
      int i;

      for ( i=1; i < argc; i++ )
      {
         if ( !strcmp ( argv[i],"-com" ))
         {
            u8_Port = atoi(argv[i+1]);
            i++;
          }
          else if ( !strcmp( argv[i], "-xml" ) )
          {
            psz_XMLFile = (char*)argv[i+1];
            i++;
          }
          else if ( !strcmp( argv[i], "-log" ) )
          {
            psz_LogFile = (char*)argv[i+1];
            i++;
          }
          else if ( !strcmp( argv[i], "-trc" ) )
          {
            psz_TraceFile = (char*)argv[i+1];
            i++;
          }
          else if ( !strcmp( argv[i], "-eeprom" ) )
          {
            psz_EEpromFile = (char*)argv[i+1];
            i++;
          }

#if defined ( __linux__ )
          else if ( !strcmp( argv[i], "-usb" ) )
          {
            u8_USB = 1;
          }
#endif
          else
          {
            printf( "usage: tcx_cmbs -log $LOGFILE$ -com $COMPORT$" );
#if defined ( __linux__ )
            printf( " -usb" );
#endif
            printf( "\n" );
            exit (-1);
          }
      }
   }
   printf( "COMPORT: %d\n",u8_Port );

   if ( psz_XMLFile )
   {
      printf( "XML: %s\n", psz_XMLFile );
   }

   if ( psz_LogFile )
   {
      printf( "Logfile: %s\n", psz_LogFile );
      if(tcx_LogOpenLogfile( psz_LogFile ) == 0)
      {
         printf("Can't open logfile %s\n", psz_LogFile);
         exit(1);
      }
   }

   if ( psz_TraceFile )
   {
      printf( "Tracefile: %s\n", psz_TraceFile );
      if(tcx_LogOpenTracefile( psz_TraceFile ) == 0)
      {
         printf("Can't open tracefile %s\n", psz_TraceFile);
         exit(1);
      }
   }

#if defined ( __linux__ )
    // catch terminal interrupt signals for graceful exit
//   signal( SIGINT,  tcx_StopSigHandler ); // catch CTL-C
//   signal( SIGTSTP, tcx_StopSigHandler ); // catch CTL-Z

   // initialize CMBS
   if( u8_USB == 1 )
   {
      tcx_USBConfig(u8_Port);
   }
   else
#endif
   {
#if 1	// Uart
      tcx_UARTIOMConfig(u8_Port);
#else	/* THLin Add for SPI */
      tcx_SpiIOMConfig();
#endif
   }

   if( appcmbs_Initialize ( NULL, &g_st_DevCtl, &g_st_DevMedia ) != 0 )
   {
      printf( "TCX ERROR: !!! DA1260 could not be started up!\n");
   }
   else
   {
      // Register callback functions for log buffer handling
      pfn_log_buffer_Cb.pfn_cmbs_api_log_outgoing_packet_prepare_cb      = tcx_LogOutgoingPacketPrepare;
      pfn_log_buffer_Cb.pfn_cmbs_api_log_outgoing_packet_part_write_cb   = tcx_LogOutgoingPacketPartWrite;
      pfn_log_buffer_Cb.pfn_cmbs_api_log_outgoing_packet_write_finish_cb = tcx_LogOutgoingPacketWriteFinish;
      pfn_log_buffer_Cb.pfn_cmbs_api_log_incoming_packet_write_finish_cb = tcx_LogIncomingPacketWriteFinish;
      pfn_log_buffer_Cb.pfn_cmbs_api_log_write_log_buffer_cb             = tcx_LogWriteLogBuffer;

      cmbs_int_RegisterLogBufferCb( &g_cmbsappl, &pfn_log_buffer_Cb );

		if ( !psz_EEpromFile )
      {
      	printf( "Start Cordless Stack\n" );
			appcmbs_CordlessStart( NULL,0 );
         // tcx_getch();
      }

#ifdef TC_XML
      tcx_appClearScreen();

      printf( "######################\n");
      printf( "TCX : Start System now\n\n" );
      printf( "Mode: t = TC or  k = keyboard\n" );
      printf( "\nChoose " );

      switch ( tcx_getch() )
      {
         case 't':
            tc_loop( psz_XMLFile );
         break;
         case 'k':
      keyboard_loop( );
         break;

         default:
            printf( "Mode is not supported! Bye\n" );
      }
#else
      keyboard_loop( );
#endif
   }

   tcx_appCleanup();

   tcx_LogCloseLogfile();
   tcx_LogCloseTracefile();

   return 0;
}
#endif

//*/
