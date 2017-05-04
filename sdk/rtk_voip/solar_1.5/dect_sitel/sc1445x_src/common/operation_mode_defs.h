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
 * (C) Copyright SiTel Semiconductor BV, unpublished work.
 * This computer program includes Confidential, Proprietary Information and
 * is a Trade Secret of SiTel Semiconductor BV.
 * All use, disclosure, and/or reproduction is prohibited unless authorized
 * in writing. All Rights Reserved.
 *
 *-----------------------------------------------------------------------------
 * File:			operation_mode_defs.h
 * Purpose:		
 * Created:		18/12/2008
 * By:			YP
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */
#ifndef OPERATION_MODE_DEFS_H
#define OPERATION_MODE_DEFS_H

//#include "../../../../../../linux-2.6.x/include/linux/autoconf.h" 
#include "si_linux_autoconf.h"

#if (defined CONFIG_SC14452)
#define SUPPORT_4_CHANNELS
#else
#define SUPPORT_3_CHANNELS
#endif

//#define PC_PLATFORM_LINUX
#define GUI_ENABLED
#define CONSOLE_ENABLED 

#if (defined CONFIG_SC1445x_TFT_LCD_SUPPORT)
	#define TFT_USER_INTERFACE
#endif 

#if ((defined CONFIG_SC1445x_TEXT_LCD_SUPPORT))// && (!(defined CONFIG_SC1445x_LEGERITY_890_SUPPORT)) && (!(defined CONFIG_SC1445x_DECT_SUPPORT)))
	#define TEXT_USER_INTERFACE
#endif 

#ifdef CONFIG_SC1445x_LEGERITY_890_SUPPORT
	#include "../SiATA/si_ata_int.h"
  #define ATA_ENABLED
  //#define USE_PCM
	#undef GUI_ENABLED
	#undef CONSOLE_ENABLED
	#undef DECT_ENABLED
 //#define SC1445x_AE_SUPPORT_FAX //selected at sc1445x_mcu_api_defs.h
#endif

#ifdef CONFIG_SC1445x_DECT_HEADSET_SUPPORT
#define DECT_HEADSET_ENABLED
#endif

#if ((defined CONFIG_SC1445x_DECT_SUPPORT) && (!(defined CONFIG_SC1445x_DECT_HEADSET_SUPPORT)))
	#include  "../SiDECT/cvmcon.h"
	#define DECT_ENABLED
  //#define USE_PCM
	#define run_dect_main dect_main()
	#define dect_change_rfpi(X) sc1445x_dect_change_rfpi(X)
	#undef GUI_ENABLED
	#undef CONSOLE_ENABLED
#elif defined CONFIG_SC1445x_DECT_HEADSET_SUPPORT
	#include  "../SiDECT/cvmcon.h"
	#define run_dect_main dect_main()
	#define dect_change_rfpi(X) sc1445x_dect_change_rfpi(X)
#else
	#define run_dect_main
  #define dect_change_rfpi(X)
#endif


#ifdef CONFIG_LMX4180_DECT_SUPPORT
#define NATALIE
#endif

#ifdef CONSOLE_ENABLED
	#include "../ConsoleIFACE/si_cc_server_api.h"
	#define start_console_thread  si_cc_api_process()
#else 
	#define start_console_thread 
#endif


#endif //OPERATION_MODE_DEFS_H

