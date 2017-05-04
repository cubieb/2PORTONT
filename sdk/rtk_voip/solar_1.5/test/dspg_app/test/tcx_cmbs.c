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
#include "bus.h"


	                              // setup TDM interface
ST_TDM_CONFIG  g_st_TdmCfg;
ST_CMBS_DEV    g_st_DevMedia;
                                // setup communication path to CMBS
ST_UART_CONFIG g_st_UartCfg;
ST_CMBS_DEV    g_st_DevCtl;


void     tcx_UARTIOMConfig ( u8 u8_Port )
{

   // setup port and uart for control plane
   g_st_UartCfg.u8_Port = u8_Port;
   g_st_DevCtl.e_DevType = CMBS_DEVTYPE_SPI; //CMBS_DEVTYPE_UART;
   g_st_DevCtl.u_Config.pUartCfg = &g_st_UartCfg;
   
   // setup TDM interface for payload plane
   g_st_TdmCfg.e_Type         = CMBS_TDM_TYPE_SLAVE;//CMBS_TDM_TYPE_MASTER;
   g_st_TdmCfg.e_Speed        = CMBS_TDM_PCM_2048;
   g_st_TdmCfg.e_Sync         = CMBS_TDM_SYNC_SHORT_LF;//CMBS_TDM_SYNC_LONG;
   g_st_TdmCfg.u16_SlotEnable = 0xFFFF;

   g_st_DevMedia.e_DevType         = CMBS_DEVTYPE_TDM;
   g_st_DevMedia.u_Config.pTdmCfg  = &g_st_TdmCfg;
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

int   main( int argc, char **argv )
{
   char * psz_XMLFile = NULL;
   char * psz_LogFile = NULL;

   u8     u8_Port = 1;
   
   Bus_Init();

   printf( "TCX - CMBS Software Version:%04X Build:%d\n", 0x0111, 368 );
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
          else
          {
            printf( "usage: tcx_cmbs -log $LOGFILE$ -com $COMPORT$ -xml $XML_CTRLFILE$" );
            exit (-1);
          }
      }
   }
   //printf( "COMPORT: %d\n",u8_Port );

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

#if defined ( __linux__ )
    // catch terminal interrupt signals for graceful exit
   signal( SIGINT,  tcx_StopSigHandler ); // catch CTL-C
   signal( SIGTSTP, tcx_StopSigHandler ); // catch CTL-Z
#endif
   // initialize CMBS
   tcx_UARTIOMConfig(u8_Port);

   if( appcmbs_Initialize ( NULL, &g_st_DevCtl, &g_st_DevMedia ) != 0 )
   {
      printf( "TCX ERROR: !!! CMBS could not be started up!\n");
   }
   else
   {
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

   return 0;
}
//*/
