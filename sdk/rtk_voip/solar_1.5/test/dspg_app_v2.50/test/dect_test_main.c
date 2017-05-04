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
#include "tcx_keyb.h"
#include "tcx_log.h"
#include "bus.h"
#include "dect_test_scenario.h"

#ifdef MY_TEST_MAIN

extern ST_TDM_CONFIG  g_st_TdmCfg;
extern ST_CMBS_DEV    g_st_DevMedia;
extern ST_UART_CONFIG g_st_UartCfg;
extern ST_CMBS_DEV    g_st_DevCtl;

extern void tcx_SpiIOMConfig (void);
extern void tcx_appCleanup(void);
extern void tcx_StopSigHandler( int nSignal );

int dspg_cmbs_init( int argc, char **argv )
{
   char * psz_XMLFile = NULL;
   char * psz_LogFile = NULL;
   char * psz_TraceFile = NULL;
   char * psz_EEpromFile = NULL;
   ST_CB_LOG_BUFFER pfn_log_buffer_Cb;
   u8     u8_Port = 0;
   u8     u8_USB = 0;

#ifdef CONFIG_RTK_VOIP_DECT_SPI_SUPPORT
	Bus_Init();
#endif

	printf("####################################################\n");
	printf( "DA1260 Demo Software Version:%04X Build:%d\n", tcx_ApplVersionGet(), tcx_ApplVersionBuildGet() );

#ifdef CONFIG_RTK_VOIP_DECT_UART_SUPPORT // Uart
	printf( "COMPORT: %d\n",u8_Port );
#endif


#ifdef CONFIG_RTK_VOIP_DECT_UART_SUPPORT // Uart
	tcx_UARTIOMConfig(u8_Port);
#else	/* THLin Add for SPI */
	tcx_SpiIOMConfig();
#endif

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
	
	
	/**** put test case here ****/
	
#if 1	// test pass for on CMBS V1.13(spi), V2.30(uart)
	u16 ver, build_ver;
	CMBS_Api_FwVersionGet(&ver, &build_ver);
	//CMBS_Api_FwVersionGet2();
	sleep(1);
#endif
	
   }

	printf("####################################################\n");
	return 0;
}

#endif //MY_TEST_MAIN

