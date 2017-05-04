/*
 *-----------------------------------------------------------------------------
 *                                                                             
 *               @@@@@@@      *   @@@@@@@@@           *                                     
 *              @       @             @               *                            
 *              @             *       @      ****     *                                 
 *               @@@@@@@      *       @     *    *    *                              
 *        ___---        @     *       @     ******    *                                 
 *  ___---      @       @     *       @     *         *                             
 *   -_          @@@@@@@  _   *       @      ****     *                               
 *     -_                 _ -                                                     
 *       -_          _ -                                                       
 *         -_   _ -        s   e   m   i   c   o   n   d   u   c   t   o   r 
 *           -                                                                    
 *                                                                              
 * (C) Copyright SiTel Semiconductor BV, unpublished work.file: 

 * This computer program includes Confidential, Proprietary Information and
 * is a Trade Secret of SiTel Semiconductor BV.
 * All use, disclosure, and/or reproduction is prohibited unless authorized
 * in writing. All Rights Reserved.
 *
 *-----------------------------------------------------------------------------
 * File:		 		 si_user_agent.c
 * Purpose:		 		 
 * Created:		 		 Dec 2007
 * By:		 		 	 YP
 * Country:		 		 Greece
 *
 *-----------------------------------------------------------------------------
 */

/*========================== Include files ==================================*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h> 

#include <si_print_api.h>

#include "../../mcu/sc1445x_amb_api.h"
#include "../common/si_timers_api.h"
#include "../common/si_configfiles_api.h"
#include "../common/operation_mode_defs.h"

#include "siUAFunctions.h"
#include "si_ua_init.h"

#include "../CCFSM/ccfsm_api.h"
#include "../CCFSM/ccfsm_init.h"
#include "../CCFSM/sc1445x_phone_api.h"

#ifdef GUI_ENABLED
#include "../radio/player.h"
#include "../RSSFeed/RSSFeed.h"
	#ifdef TEXT_USER_INTERFACE
		#include  "../TUI/sc1445x_gui_main.h"
	#elif defined TFT_USER_INTERFACE
		#include  "../TFT_GUI/sc1445x_gui_main.h"
	#else
		#include  "../GUI/sc1445x_gui_main.h"
	#endif
#endif

ConfigSettings m_Config_Settings;
SICORE m_SystemCore;
ConfigSettings m_LocalConfigFile;
extern int RSSFeed(void * arg);
unsigned char SystemMACAddress[6];

int	main(void) 
{
  unsigned char SystemIPAddress[64];
 	int ret;

  //start networking functions and return the IP address
	si_init_timer();
	si_net_init(SystemIPAddress);

	//read/verify configuration files 
	ret = LoadAllConfigFiles(&m_Config_Settings);    
	si_print_init(&m_Config_Settings);

	

	si_ua_readMACAddress(SystemMACAddress);
	dect_change_rfpi(SystemMACAddress);
	run_dect_main;

	//build internal runtime structures
	libUASetCore(SystemIPAddress, &m_Config_Settings, &m_SystemCore);
	sc_fileapi_LoadNetApplications(&m_LocalConfigFile.m_NetApplications );

	#ifdef GUI_ENABLED
 	 	internet_radio_init();
 
		if (m_Config_Settings.m_NetApplications.RSSFeedEnable )
		{
		 RSSFeed_Init();
  	}
	#endif
	apiUAInitRequest(&m_SystemCore);
	ccfsm_init(getpid());
	sc1445x_phoneapi_init();
 	start_console_thread;
 	si_ua_start_ntpclient();
		 while (1){
			#ifdef GUI_ENABLED  
				GUI_polling();
			#endif

			 usleep (50000);
		 }

  return 0;
}

 