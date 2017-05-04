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
 * File:		si_print_api.h
 * Purpose:		
 * Created:		27/04/2009
 * By:			EF
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef SI_PRINT_MANAGE_H
#define SI_PRINT_MANAGE_H
/*========================== Include files ==================================*/
#include "../sc1445x_src/common/si_config_file.h"
/*========================== Local macro definitions & typedefs =============*/
/* enumaration for selecting appropriate print output */
typedef enum _PRINT_TYPE
{
	PRINT_TO_STDIO=0,
	PRINT_TO_FILE,
	PRINT_TO_SYSLOG,
	PRINT_TYPE_INVALID
}PRINT_TYPE;

/* enumaration for enabling or disabling print to any output */
typedef enum _PRINT_STATUS
{
	PRINT_DISABLE=0,
	PRINT_ENABLE,
	PRINT_STATUS_INVALID,
}PRINT_STATUS;

/* enumaration for print debug level */
typedef enum _PRINT_DEBUG_LEVEL
{
	PRINT_LEVEL_EMERG=0,
	PRINT_LEVEL_ALERT,
	PRINT_LEVEL_CRIT,
	PRINT_LEVEL_ERR,
	PRINT_LEVEL_WARNING,
	PRINT_LEVEL_NOTICE,
	PRINT_LEVEL_INFO,
	PRINT_LEVEL_DEBUG,
	PRINT_DEBUG_LEVEL_INVALID
}PRINT_DEBUG_LEVEL;

/* enumaration for selecting prints below a specific 
	 level or selecting prints from a single level */
typedef enum _PRINT_SELECT_LEVEL
{
	PRINT_ONE_LEVEL=0,
	PRINT_LEVELS_BELOW,
	PRINT_SELECT_LEVEL_INVALID
}PRINT_SELECT_LEVEL;


/* Above Debug level match the syslog debug levels
#define	LOG_EMERG	0	
#define	LOG_ALERT	1	
#define	LOG_CRIT	2	
#define	LOG_ERR		3	
#define	LOG_WARNING	4	
#define	LOG_NOTICE	5	
#define	LOG_INFO	6	
#define	LOG_DEBUG	7	
*/

/* print structure holding the entire print settings */
typedef struct _Print_Settings
{
	PRINT_TYPE print_to_output;
	PRINT_STATUS print_on_off;
	PRINT_DEBUG_LEVEL print_level;
	PRINT_SELECT_LEVEL print_select;
	char syslog_server_ip[16];
	char print_file_path[256];

}Print_Settings;

/*========================== Global variables ===============================*/


/*========================== Global function prototypes =====================*/
void si_print_init(ConfigSettings *s_ConfigSettings);
void si_print_set_status(PRINT_STATUS print_on_off, PRINT_TYPE print_to, PRINT_DEBUG_LEVEL print_debug_level, PRINT_SELECT_LEVEL print_select, 	char *syslog_server_ip, char *print_file);
void si_print_stdio_or_file(PRINT_DEBUG_LEVEL print_level, char *fmt, ...);
void si_print_syslog(PRINT_DEBUG_LEVEL print_level, char *fmt, ...);
int si_print(PRINT_DEBUG_LEVEL print_level, char *fmt, ...);

#endif /* SI_PRINT_MANAGE_H */
