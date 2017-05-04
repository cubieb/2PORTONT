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
 * File:		si_ua_timer.h
 * Purpose:		
 * Created:		Jul 2008
 * By:			YP
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef SI_UA_TIMER_H
#define SI_UA_TIMER_H
#include "si_config_file.h"
#include "si_phone_api.h"
 
#define MAX_TIMER_ITEMS						20
#define TFT_SCROLL_TIMER_ITEMS		10
#define TIMER_SOURCE_MMI					11
#define TIMER_SOURCE_ATA_LINE_0		12
#define TIMER_SOURCE_ATA_LINE_1		13
#define TIMER_SOURCE_ATA_LINE_2		14
#define TIMER_SOURCE_ATA_LINE_3		15
#define TIMER_SOURCE_UA						16
#define TIMER_SOURCE_MMI_CLK			17
#define TIMER_SOURCE_MMI_IMS			18
 
typedef struct _si_timer_t
{
	void* timerFunction; 
	void* timerFunction1; 
	int timerInterval; 
	int SourceId;
	int nonstop;
	void *userParameter;

}si_timer_t;

void si_init_timer(void);
int si_addtimerList(void* addfunct, int addinterval,int SourceId, void* userParameter);
int si_addNonStopTimer(void* addfunct, int addinterval, int SourceId, void* userParameter);
int si_canceltimerByFunction(void* userfunct);
int si_canceltimerBySourceId(int SourceId);
int si_canceltimerByIndex(int index);
#endif //SI_UA_TIMER_H
