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

int main( int argc, char **argv )
{
	Bus_Init();

	printf( "DA1260 Demo Software Version:%04X Build:%d\n", tcx_ApplVersionGet(), tcx_ApplVersionBuildGet() );

#if defined ( __linux__ )
	// catch terminal interrupt signals for graceful exit
	signal( SIGINT,  tcx_StopSigHandler ); // catch CTL-C
	signal( SIGTSTP, tcx_StopSigHandler ); // catch CTL-Z
#endif

	// initialize CMBS
	tcx_SpiIOMConfig();

	if( appcmbs_Initialize ( NULL, &g_st_DevCtl, &g_st_DevMedia ) != 0 )
	{
		printf( "TCX ERROR: !!! CMBS could not be started up!\n");
	}

	
	sleep(3);
	
	
	/**** put test case here ****/
	
#if 1	// test pass for on CMBS V1.13(spi), V2.30(uart)
	u16 ver, build_ver;
	CMBS_Api_FwVersionGet(&ver, &build_ver);
	//CMBS_Api_FwVersionGet2();
	sleep(1);
#endif
	/****************************/
#if 0	// test pass for on CMBS V1.13(spi), V2.30(uart)
	CMBS_Api_RegistrationOpen();
	sleep(20);
	CMBS_Api_RegistrationClose();

#endif
	/****************************/
#if 0	// test pass for on CMBS V1.13(spi), V2.30(uart)
	CMBS_Api_HandsetPage("1");
	sleep(10);
	CMBS_Api_HandsetPage("2");
	sleep(10);
	CMBS_Api_HandsetPage("all");
	sleep(10);
	CMBS_Api_HandsetPage("none");
#endif
	/****************************/
#if 0	// test pass for on CMBS V1.13(spi), V2.30(uart)
	CMBS_Api_HandsetDelet("1");
	sleep(5);
	CMBS_Api_RegistrationOpen();
	sleep(30);
	CMBS_Api_RegistrationClose();
	sleep(3);
	
	CMBS_Api_HandsetDelet("2");
	sleep(5);
	CMBS_Api_RegistrationOpen();
	sleep(60);
	CMBS_Api_RegistrationClose();
	sleep(3);
	
	CMBS_Api_HandsetDelet("all");
	sleep(5);
	CMBS_Api_RegistrationOpen();
	sleep(30);
	CMBS_Api_RegistrationClose();
	sleep(3);
	
	CMBS_Api_HandsetDelet("none");
#endif
	/****************************/
#if 0	// test FAIL for on CMBS V1.13, org test app also fail.
	// test pass for on CMBS V2.30(uart)
	CMBS_Api_GetRegHandsetNum();
#endif
	/****************************/
#if 0	// test FAIL for on CMBS V1.13, org test app also fail.
	// test pass for on CMBS V2.30(uart)
	CMBS_Api_InBound_2HS_Ring();
	sleep(3); //wait HS off-hook
	CMBS_Api_CallRelease(0, 0);
#endif
	/****************************/
#if 1	// test pass for on CMBS V2.30(uart)
	int call_num;
	call_num = CMBS_Api_InBound_Ring_CallerID(0, "5780211", "THLin");	//HS Ringing, and display CLID
	printf("call_num = %d\n", call_num);
	//sleep(3);
	//CMBS_Api_LineStateGet(call_num);
	sleep(10); //wait HS off-hook
	//CMBS_Api_CallRelease(0, 0);	// call release (stop Ring)
	
	//sleep(5);
	
	CMBS_Api_InBound_Ring_CallerID(0, "5780211", "THLin");	//HS Ringing, and display CLID
	printf("call_num = %d\n", call_num);
	//sleep(3);
	//CMBS_Api_LineStateGet(call_num);
	sleep(10); //wait HS off-hook
	//CMBS_Api_CallRelease(0, 0);	// call release (stop Ring)
#endif
	/****************************/
#if 0	// test pass for on CMBS V2.30(uart)
	/* Description:
	* HS need off-hook first, Host app get Call Establist, 
	* then run below API with arg "CMBS_CALL_PROGR_INBAND",
	* Audio path is setup, HS can hear DSP Tone for Host CPU 
	*/
	sleep(3);
	// C8 Demo: early media
	CMBS_Api_OutboundCallProgress(0, (char*)"CMBS_CALL_PROGR_INBAND\0");
#endif
	/****************************/
#if 0
	//CMBS_Api_OutboundCallProgress(0, (char*)"CMBS_CALL_PROGR_SETUP_ACK\0");	
#endif
	/****************************/
#if 0
	
#endif
	/****************************/
	
	sleep(3);
	
   }

	tcx_appCleanup();

	return 0;
}

#endif //MY_TEST_MAIN

