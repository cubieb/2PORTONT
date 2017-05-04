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
 * File:			si_print_manage.c
 * Purpose:		
 * Created:		27/4/2009
 * By:				EF
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */
 
/*========================== Include files ==================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

#include "si_print_api.h"

/* Syslog messages */
#include "../syslog/syslog.h"
/* Check IP */
#include "../sc1445x_src/CCFSM/sc1445x_phone_api.h"


/*========================== Local macro definitions ========================*/


/*========================== Global definitions =============================*/
Print_Settings s_print_settings={
	PRINT_TO_STDIO,
	PRINT_ENABLE,
	PRINT_LEVEL_INFO,
	PRINT_LEVELS_BELOW,
	'\0',
	'\0',
};

/*========================== Local function prototypes ======================*/


/*========================== Local data definitions =========================*/
FILE *fileinfo = NULL;

/*========================== Function definitions ===========================*/

void si_print_init(ConfigSettings *s_ConfigSettings)
{
	time_t now;
	struct tm *ts;
	char buf[40];
	
	time(&now);
	ts = localtime(&now);
	strftime(buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", ts);

	if (strcpy(s_ConfigSettings->m_PrintLogSettings.print_to, "file")==0)
	{
		s_print_settings.print_to_output = PRINT_TO_FILE;
	}
	else if (strcpy(s_ConfigSettings->m_PrintLogSettings.print_to, "syslog")==0)
	{
		s_print_settings.print_to_output = PRINT_TO_SYSLOG;
	}
	else if (strcpy(s_ConfigSettings->m_PrintLogSettings.print_to, "stdio")==0)
	{
		s_print_settings.print_to_output = PRINT_TO_STDIO;

	}else s_print_settings.print_to_output = PRINT_TO_STDIO;

	s_print_settings.print_on_off = s_ConfigSettings->m_PrintLogSettings.print_enable;
	s_print_settings.print_level = s_ConfigSettings->m_PrintLogSettings.print_level;
	s_print_settings.print_select = s_ConfigSettings->m_PrintLogSettings.print_level_option;
	strcpy(s_print_settings.syslog_server_ip, s_ConfigSettings->m_PrintLogSettings.syslog_server_ip);
	strcpy(s_print_settings.print_file_path, s_ConfigSettings->m_PrintLogSettings.print_file_path);

	si_print_set_status(s_print_settings.print_on_off,
											s_print_settings.print_to_output,
											s_print_settings.print_level,
											s_print_settings.print_select,
											s_print_settings.syslog_server_ip,
											s_print_settings.print_file_path);

	if (s_print_settings.print_on_off == PRINT_DISABLE)
	{
		return;
	}
	else if(s_print_settings.print_to_output == PRINT_TO_STDIO)
	{
		fileinfo = stderr;
		si_print(PRINT_LEVEL_INFO,"\nStart logging at stdio at %s \n\n", buf);
	}
	else if (s_print_settings.print_to_output == PRINT_TO_FILE)
	{
		fileinfo = stderr;
		si_print(PRINT_LEVEL_INFO, "\nStart logging at file %s at  %s \n\n", s_print_settings.print_file_path, buf);
		fileinfo = fopen(s_print_settings.print_file_path, "wt");
		if (fileinfo == NULL)
		{
			fileinfo = stderr;
			s_print_settings.print_to_output = PRINT_TO_STDIO;
			si_print(PRINT_LEVEL_INFO, "\nPrint Log File was not found \n");
			si_print(PRINT_LEVEL_INFO, "Redirecting errors to stdio \n\n");
			return;
		} 
		si_print(PRINT_LEVEL_INFO, "\nStart logging at file %s at %s \n\n", s_print_settings.print_file_path, buf);
 	}
	
	else if (s_print_settings.print_to_output == PRINT_TO_SYSLOG)
	{
		if (s_print_settings.syslog_server_ip[0] == '\0')
		{
			fileinfo = stderr;
			s_print_settings.print_to_output = PRINT_TO_STDIO;
			si_print(PRINT_LEVEL_INFO, "\nNone or wrong server IP, for syslog, was given \n\n");
			si_print(PRINT_LEVEL_INFO, "Start logging at stdio at %s \n\n", buf);
		}
		else
		{
			fileinfo = stderr;
			s_print_settings.print_to_output = PRINT_TO_STDIO;
			si_print(PRINT_LEVEL_INFO, "\nStart logging at syslog with server IP %s \n\n",  s_print_settings.syslog_server_ip);
			s_print_settings.print_to_output = PRINT_TO_SYSLOG;
			fileinfo = fopen(s_print_settings.print_file_path, "wt");
			if ((s_print_settings.print_file_path == NULL) || (fileinfo == NULL)) 
			{
				/* print at stdio console only */
				fileinfo = NULL;
				si_print(PRINT_LEVEL_INFO, "\nSyslog File was not found \n");
				si_print(PRINT_LEVEL_INFO, "Start logging to syslog console only at %s\n\n", buf);				

			}	
		}
	}
}


void si_print_set_status(PRINT_STATUS print_on_off, PRINT_TYPE print_to, PRINT_DEBUG_LEVEL print_level, PRINT_SELECT_LEVEL print_select, 	char *syslog_server_ip, char *print_file)
{
	unsigned char tmpip[4];
	char num_ip[64];
  int check_ip=0;

 
	if((print_to<PRINT_TYPE_INVALID)&&(print_to>=0))
	{
		s_print_settings.print_to_output = print_to;

	}else s_print_settings.print_to_output = PRINT_TO_STDIO; //Default set to stdio

	if((print_on_off<PRINT_STATUS_INVALID)&&(print_on_off>=0))
	{
		s_print_settings.print_on_off = print_on_off;

	}else s_print_settings.print_on_off = PRINT_ENABLE; //Default set enable

	if((print_level<PRINT_DEBUG_LEVEL_INVALID)&&(print_level>=0))
	{
		s_print_settings.print_level = print_level;

	}else s_print_settings.print_level = PRINT_LEVEL_INFO; //Default set to enable all levels

	if((print_select<PRINT_SELECT_LEVEL_INVALID)&&(print_select>=0))
	{
		s_print_settings.print_select = print_select;

	}else s_print_settings.print_select = PRINT_LEVELS_BELOW; //Default set to enable all levels below the one selected at print_level

	if(syslog_server_ip)
	{
		strcpy(num_ip, syslog_server_ip);
		check_ip = sc1445x_phoneapi_extract_address(tmpip, num_ip);
		/* Check if the sylog IP is OK */
		if (check_ip == 0)
		{
			strcpy(s_print_settings.syslog_server_ip, syslog_server_ip);
		}	
		else
		{
			s_print_settings.syslog_server_ip[0]='\0'; // Default None
		}
 
	}else {
		s_print_settings.syslog_server_ip[0]='\0'; // Default None
	}

	if(print_file)
	{
		strcpy(s_print_settings.print_file_path, print_file);
	}
	else s_print_settings.print_file_path[0]='\0'; // Default None

}



void si_print_syslog(PRINT_DEBUG_LEVEL print_level, char *fmt, ...)
{
	va_list argp;

	va_start(argp, fmt);
	vsyslog(print_level, fmt, argp);
	va_end(argp);
}


int si_print(PRINT_DEBUG_LEVEL print_level, char *fmt, ...)
{
	va_list argp;

	if (s_print_settings.print_on_off == PRINT_DISABLE)
	{
		return 0;
	}
	else
	{
		// If print level selected (print_select) is at PRINT_LEVELS_BELOW then we check for the print_level,
		// set at  si_print_init, to be smaller or equal in order to be print.
		// If print level selected (print_select) is at PRINT_ONE_LEVEL then only the same level set at 
		// si_print_init should be allowed to be printed 
		if ((s_print_settings.print_select == PRINT_LEVELS_BELOW) && (!(print_level <= s_print_settings.print_level)))
		{
			return 0;

		}else if ((s_print_settings.print_select == PRINT_ONE_LEVEL) && (!(print_level == s_print_settings.print_level)))
		{
			return 0;
		}
	}
	/* stdio or file print */
	if ((!(fileinfo==NULL)) && (!(s_print_settings.print_to_output==PRINT_TO_SYSLOG)))
	{
		va_start(argp, fmt);
		vfprintf(fileinfo, fmt, argp);
		va_end(argp);

	}
	/* syslog and file print */
	else
	{	
		va_start(argp, fmt);
		vsyslog(print_level, fmt, argp);
		va_end(argp);
	}

	return 0;
}



















