// http://publib.boulder.ibm.com/infocenter/iseries/v5r3/index.jsp?topic=/apis/setitime.htm

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
 * File:		 		 si_ua_timer.c
 * Purpose:		 		 
 * Created:		 		 Jul 2008
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
#include <unistd.h>
#include <sys/time.h>

#include <si_print_api.h>
  
#include "si_timers_api.h" 

/*
#include "si_ua_timer.h" 
#define TIMER_SOURCE_MMI	1
#define TIMER_SOURCE_ATA	2
#define TIMER_SOURCE_UA		3

void myTimerFunction(void);
void myTimerFunction(void)
{
 	.......
}
main()
{
	int mSec = 500000; // 500 mseconds
	int sourceId = TIMER_SOURCE_MMI;
	....
	si_init_timer();
	.....
	pthread_mutex_lock(&mTimerMutex);
 	si_addtimerList((void*)&myTimerFunction ,  mSec, sourceId);
	pthread_mutex_unlock(&mTimerMutex);
	....

	....
	//in case we need to stop timer
	si_canceltimerBySourceId(sourceId);
	or 
	si_canceltimerByFunction((void*)&myTimerFunction);
}

*/
static int si_timer_start(void);
static int si_timer_stop(void);
static void si_timer_action(void *data);
static int  si_checktimerList(void);
static void si_cancel_timer(int index) ; 
static int si_update_item(void* addfunct, int interval, int SourceId, void* userParameter);

#define MIN_TIMER_TICK 100000
pthread_mutex_t mTimerMutex;
static struct sigaction old_action_handler;
static struct itimerval old_timer;
static int timerTicks = MIN_TIMER_TICK;
static int timerStatus =0;
static si_timer_t si_timer_list[MAX_TIMER_ITEMS];

void (*userTimerFunction) (void)=NULL;
void (*userTimerFunction1) (void*)=NULL;
// TIMER API Funtions
//initiate timer structure, global variables , etc.
void si_init_timer(void)
{
	int i;
	pthread_mutex_init(&mTimerMutex, NULL);
	timerTicks = MIN_TIMER_TICK;
	timerStatus = 0;
 
	for (i=0;i<MAX_TIMER_ITEMS;i++)
	{
		 si_timer_list[i].timerFunction = 0; 
		 si_timer_list[i].timerInterval = 0;  
		 si_timer_list[i].SourceId = 0;
		 si_timer_list[i].userParameter = NULL;
		 si_timer_list[i].nonstop = 0;

 	}
}
static int protectionFlag = 0;

static int si_update_item(void* addfunct, int interval, int SourceId, void* userParameter)
{
	int i;
	for (i=0; i<MAX_TIMER_ITEMS; i++)
	{
		if ((si_timer_list[i].timerFunction == addfunct)&&(si_timer_list[i].SourceId  == SourceId))
		{
			si_timer_list[i].timerInterval = interval;
			si_timer_list[i].timerFunction = addfunct;
			if (interval<timerTicks) interval = timerTicks;
			si_timer_list[i].SourceId = SourceId;
			 
			return 1;
		}
	}
	return 0;
}

int si_addNonStopTimer(void* addfunct, int addinterval, int SourceId, void* userParameter)
{
	int index = si_addtimerList(addfunct, addinterval, SourceId, userParameter);

	if (index > 0)
		si_timer_list[index-1].nonstop = addinterval;
 
	return index;
}

int si_addtimerList(void* addfunct, int addinterval, int SourceId, void* userParameter)
{
	int i;
	int selectedtimer = 0;

	if (protectionFlag){	
 		return -1;
	}
 
 	protectionFlag = 1;
 	if (!si_update_item(addfunct, addinterval, SourceId,userParameter)) 
 	{
		for (i=0;i<MAX_TIMER_ITEMS;i++)
		{
			if (si_timer_list[i].timerFunction == NULL)
			{
 				si_timer_list[i].timerInterval = addinterval;
				si_timer_list[i].timerFunction = addfunct;
				if (addinterval<timerTicks) addinterval = timerTicks;
				si_timer_list[i].SourceId = SourceId;
				si_timer_list[i].userParameter = userParameter;
				si_timer_list[i].nonstop = 0;
				selectedtimer=i+1;
  			break;
  		}
		}
	}
	if (!timerStatus) si_timer_start(); 
 	protectionFlag=0;

 	return selectedtimer;
}

int si_canceltimerByFunction(void* userfunct)
{
 	int i;

	for (i=0;i<MAX_TIMER_ITEMS;i++)
	{
		if (si_timer_list[i].timerFunction == userfunct)
		{
			si_cancel_timer(i); 
		}
	}
	return 0;
}

int si_canceltimerByIndex(int index)
{
  	if (index && index<=(MAX_TIMER_ITEMS+1))
	{
		si_cancel_timer(index-1); 
 	}
	return 0;
}

int si_canceltimerBySourceId(int SourceId)
{
 	int i;
	for (i=0;i<MAX_TIMER_ITEMS;i++)
	{
		if (si_timer_list[i].SourceId == SourceId)
		{
			si_cancel_timer(i); 
		}
	}
	return 0;
}



///////////////////////////////////////////////////////////////////////
// Static Functions

static void si_timer_action(void *data)
{
	if (!si_checktimerList()) si_timer_stop();
}

static int si_timer_start(void)
{
	struct itimerval timerv;
	struct sigaction siga;
	timerStatus = 1;
	sigaction (SIGALRM, NULL, &old_action_handler);
	siga = old_action_handler;
	siga.sa_handler = (void(*)(int))si_timer_action;
	siga.sa_flags = 0;
	sigaction (SIGALRM, &siga, NULL);

	timerv.it_interval.tv_sec = 0;
	timerv.it_interval.tv_usec = timerTicks;         
	timerv.it_value = timerv.it_interval;

	if (setitimer (ITIMER_REAL, &timerv, &old_timer) == -1) 
	{
		si_print(PRINT_LEVEL_ERR, "Setitimer failed\n");
		return -1;
	}
	 return 0;
}

static int si_timer_stop(void)
{
	struct itimerval value;
	timerStatus=0;
	getitimer(ITIMER_REAL, &value);
	value.it_value.tv_sec = 0;
	value.it_value.tv_usec = 0;

	setitimer(ITIMER_REAL, &value, NULL);
	return 0;
}

static int  si_checktimerList(void)
{
	int found = 0;
	int i;
	if (protectionFlag) return 1;
  
	//if (!timerStatus) return 0;

	for (i=0; i<MAX_TIMER_ITEMS; i++)
	{
		if (si_timer_list[i].timerFunction)
		{
			found =1;
			si_timer_list[i].timerInterval -= timerTicks;
			if (si_timer_list[i].timerInterval < timerTicks) 
			{
  				si_timer_list[i].timerInterval = si_timer_list[i].nonstop;
				if (!si_timer_list[i].userParameter)
				{
					userTimerFunction = si_timer_list[i].timerFunction;
					if (!si_timer_list[i].nonstop)
						si_timer_list[i].timerFunction = 0;
			        userTimerFunction();
				}else{
					userTimerFunction1 = si_timer_list[i].timerFunction;
					if (!si_timer_list[i].nonstop)
						si_timer_list[i].timerFunction = 0;
			        userTimerFunction1(si_timer_list[i].userParameter);
 				}		
				// initiate current timer record
			}
 		}
	}
	return found ;
}

static void si_cancel_timer(int index)  
{
	si_timer_list[index].timerFunction = 0; 
	si_timer_list[index].timerInterval = 0;  
 	si_timer_list[index].SourceId	= 0;
	si_timer_list[index].nonstop = 0;
}
 

