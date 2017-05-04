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
 * File:			app_presence.h
 * Purpose:		
 * Created:		Oct 2009
 * By:			YP
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef APP_PRESENCE_H
#define APP_PRESENCE_H
 
#include "../exosip2/src/eXosip.h"
#include <mxml.h>
typedef struct _appl_presence 
{
	char *note;
	char *basicstatus;
	char *username;
	char *host;
}appl_presence;

appl_presence *sc1445x_app_presence_notify_parser(char*info);
char *sc1445x_app_presence_getelement(mxml_node_t *tree, char *element); 
#endif //APP_PRESENCE_H
