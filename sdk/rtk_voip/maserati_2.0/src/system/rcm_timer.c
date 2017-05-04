#include <time.h>

#include "linphonecore.h"
#include "rcm_timer.h"
//[20110809, SD6, bohungwu, redirect message out Linphone debug modue
#include "uglib.h"
//]

typedef struct {
	char next; //1 byte
	char prev; //1 byte
	void   *function_data; // 4byte
	struct timeval timeout;	//8byte
	FUNC function_p; //4byte
} timerlinked_t;

static timerlinked_t rtk_timerlist[ MAX_TIMER_ARRAY_NUM ];

static char rtk_timer_array_head;
static char rtk_timer_free_list_head;


/*______________________________________________________________________________
**	timer_subsystem_init
**
**	descriptions:init timer function
**	parameters:
**	local:
**	global:
**	return:
**	called by:
**	call:
**	revision:
**____________________________________________________________________________*/


void rcm_timer_subsystem_init(void) {

	int i;

	g_message("timer_subsystem_init\n");
	for(i=0; i<MAX_TIMER_ARRAY_NUM-1; ++i){
		rtk_timerlist[i].next = i+1;
		rtk_timerlist[i].timeout.tv_sec=0;
		rtk_timerlist[i].timeout.tv_usec=0;
		rtk_timerlist[i].function_p=NULL;
		rtk_timerlist[i].prev = i;
	}
	rtk_timerlist[i].next = null;
		
	rtk_timer_free_list_head=0;
	rtk_timer_array_head=null;
	
	return;
}

/*______________________________________________________________________________
**	timercreate
**
**	descriptions: create timer and call-back function
**	parameters: call back function, call back function data
**	local:
**	global:
**	return:
**	called by:
**	call:
**	revision:
**____________________________________________________________________________*/

int rcm_timerCreate(FUNC callback,void *arg){
	
	char curr_index;

//	g_message("timerCreate func\n");
	if (rtk_timer_free_list_head == null) {
		//array full
		g_warning("timer array full !!\n");
			return null;
	}
			
	curr_index = rtk_timer_free_list_head;

	rtk_timer_free_list_head = rtk_timerlist[curr_index].next;	
	rtk_timerlist[curr_index].next = null;
	rtk_timerlist[curr_index].prev = curr_index;
	rtk_timerlist[curr_index].function_p = callback; // call back function.
	rtk_timerlist[curr_index].function_data = arg;
	
	return curr_index;
	
}


/*______________________________________________________________________________
**	timerdestory
**
**	descriptions: free timer and call-back function
**	parameters: timer list array index
**	local:
**	global:
**	return:
**	called by:
**	call:
**	revision:
**____________________________________________________________________________*/

void rcm_timerDestory(int timer_handleID) {
	
//	g_message("timerdestory id %d\n",timer_handleID);
	if(rtk_timerlist[timer_handleID].function_p == NULL){
		g_warning("timer not create\n");
		return;
	}
	
	rcm_timerCancel(timer_handleID);
	
	rtk_timerlist[timer_handleID].function_p=NULL;
	rtk_timerlist[timer_handleID].timeout.tv_sec=0;
	rtk_timerlist[timer_handleID].timeout.tv_usec=0;
	
	/* Insert it in front of the rtk_timer_free_list_head */
	rtk_timerlist[timer_handleID].next = rtk_timer_free_list_head;
	rtk_timer_free_list_head = timer_handleID;
	
//	g_message("timerdestory , free_list_head  is %d\n",rtk_timer_free_list_head);
}


/*______________________________________________________________________________
**	timerLaunch
**
**	descriptions: assign timeout value
**	parameters: timer list array index , the value of timeout (msec)
**	local:
**	global:
**	return:
**	called by:
**	call:
**	revision:
**____________________________________________________________________________*/

int rcm_timerLaunch(int timer_handleID,int timelength)
{
	int temp,tmp;
	int timerid;
	struct timeval temp_tv;


	if(rtk_timerlist[timer_handleID].function_p == NULL){
		g_message("timer_handleID not have callback function\n");
		return -1;
	}
	
	if(rtk_timerlist[timer_handleID].prev!=timer_handleID){
		g_message("already have old launch timer ");	
		rcm_timerCancel(timer_handleID);
		
	}
	
#if 0
	if(list[timer_handleID].timeout.tv_sec!=0){
		g_message("already have old launch timer ");	
		timerCancel(timer_handleID);
	}
#endif


//	g_message("timerstart , after %d msec \n",timelength);
	gettimeofday(&temp_tv, NULL);
	//now time + timelength => timeout
	temp_tv.tv_usec  +=  (timelength%1000)*1000;
	temp_tv.tv_sec  +=  timelength/1000 + temp_tv.tv_usec/1000000;
	temp_tv.tv_usec  =  temp_tv.tv_usec%1000000;

	rtk_timerlist[timer_handleID].timeout.tv_sec=temp_tv.tv_sec;
	rtk_timerlist[timer_handleID].timeout.tv_usec=temp_tv.tv_usec;
	rtk_timerlist[timer_handleID].next = null;	
	rtk_timerlist[timer_handleID].prev = null;

	//Sort timer list
	//add timeout value to array, 
	//g_message("launch: array_head is %d\n",rtk_timer_array_head);
	
	if(rtk_timer_array_head == null)
	{
		rtk_timer_array_head = timer_handleID;   /*if list is empty, strat is assigned the value that is in temp*/
	
	}else if(timercmp(&temp_tv, &(rtk_timerlist[rtk_timer_array_head].timeout),<)){ //insert before
		
		//list[temp].prev = list[array_head].prev; //null
		//list[timer_handleID].prev = null;
		rtk_timerlist[timer_handleID].next = rtk_timer_array_head;
		rtk_timerlist[rtk_timer_array_head].prev=timer_handleID;
		rtk_timer_array_head =timer_handleID;				
					
	
	}else{
	
		tmp = rtk_timer_array_head;
		while((rtk_timerlist[tmp].next != null) && timercmp(&temp_tv,&(rtk_timerlist[rtk_timerlist[tmp].next].timeout),>))
		{
			tmp = rtk_timerlist[tmp].next;
		}
		//new node.next = 前一個node的 next  ,  
		rtk_timerlist[timer_handleID].next = rtk_timerlist[tmp].next;
		//new node.prev = 前一個node
		rtk_timerlist[timer_handleID].prev= tmp;
		
		if(rtk_timerlist[tmp].next!=null)
			rtk_timerlist[rtk_timerlist[tmp].next].prev=timer_handleID;		
			
		rtk_timerlist[tmp].next = timer_handleID;
		
	}
	
		
	return 0;
	
}

/*______________________________________________________________________________
**	timerCancel
**
**	descriptions: stop timer
**	parameters: timer list array index ,
**	local:
**	global:
**	return:
**	called by:
**	call:
**	revision:
**____________________________________________________________________________*/


void rcm_timerCancel(int timer_handleID)
{
	
//	int curr,prev;
	
//	g_message("cancel timer_handleID %d\n",timer_handleID); 
	if (rtk_timer_array_head == null){ //list is empty
		g_warning("timerCancel ,timer is empty\n");
		return;
	}
		
				
	if(rtk_timerlist[timer_handleID].prev==timer_handleID ){ // timer allready timeout or cancel
//				g_message("timerCancel is already cacnel\n");
		return;
	}
		
			
	if(rtk_timerlist[timer_handleID].prev == null) //head
	{
		//g_message("\r\naa  list[array_head.next is %d",list[timer_handleID].next);
		rtk_timer_array_head = rtk_timerlist[timer_handleID].next;
		//g_message("\r\n bb after array_head %d",array_head);	
		rtk_timerlist[timer_handleID].next = null;
	//	list[timer_handleID].prev = timer_handleID;
		if(rtk_timer_array_head!=null)
			rtk_timerlist[rtk_timer_array_head].prev=null;

	}else{
		rtk_timerlist[rtk_timerlist[timer_handleID].prev].next = rtk_timerlist[timer_handleID].next;
		if(rtk_timerlist[timer_handleID].next!=null)
			rtk_timerlist[rtk_timerlist[timer_handleID].next].prev = rtk_timerlist[timer_handleID].prev;
		
		rtk_timerlist[timer_handleID].next = null;
		//list[timer_handleID].prev = timer_handleID;
	
	}
	
		rtk_timerlist[timer_handleID].prev = timer_handleID;
	  rtk_timerlist[timer_handleID].timeout.tv_sec=0;
	
}
	
#if 0

int getHeadTimer(struct timeval *HeadTimer){

	
g_message("\r\n getHeadTimer rtk_timer_array_head is %d\n",rtk_timer_array_head);
	if(rtk_timer_array_head==null){
		g_message("\r\ngetHeadTimer  no timer in list");
		HeadTimer=NULL;
		return -1;
		
	}
	
g_message("\r\n get head timer time is %u %u \n",rtk_timerlist[rtk_timer_array_head].timeout.tv_sec,rtk_timerlist[rtk_timer_array_head].timeout.tv_usec);
	*HeadTimer=rtk_timerlist[rtk_timer_array_head].timeout;

	return 0;

}
#endif
#if 1
Rtk_Time *rcm_getHeadTimer(void){
struct timeval *head_p=NULL;

//	g_message("\r\n getHeadTimer rtk_timer_array_head is %d\n",rtk_timer_array_head);
	if(rtk_timer_array_head==null){
	//	g_message("\r\ngetHeadTimer  no timer in list");
//		HeadTimer=NULL;
		return NULL;
		
	}
	
//	g_message("\r\n get head timer time is %u %u \n",rtk_timerlist[rtk_timer_array_head].timeout.tv_sec,rtk_timerlist[rtk_timer_array_head].timeout.tv_usec);
	//*HeadTimer=list[array_head].timeout;
	//temptv=&(list[array_head].timeout);
	head_p=&(rtk_timerlist[rtk_timer_array_head].timeout);
//	g_message("\r\nHeadTimer->.tv_sec is %u   \n",head_p->tv_sec);
	return head_p;
//HeadTimer.tv_sec = list[array_head].timeout.tv_sec;
//

	//return 0;

}
#endif

	


void rcm_TimerProc(){

	int curr;
	struct timeval tv,*temptv; 
	
//	if(rtk_timer_array_head==null){
		//g_message("\r\n no timer in list");
	//	return ;
//	}
	

	while(1){
		if(rtk_timer_array_head == null)
			break;
			
		//check timer 	
		gettimeofday(&tv, NULL);
		temptv=&(rtk_timerlist[rtk_timer_array_head].timeout);

		if(!(timercmp(&tv, temptv,<))){
	//		g_message("--------------\n");
		//	g_message(" index %d timeout\n",rtk_timer_array_head);

			curr=rtk_timer_array_head;
			rcm_timerCancel(curr);
			
			/* Call call back function */
			(rtk_timerlist[curr].function_p)(rtk_timerlist[curr].function_data);

//			g_message("next array_head is %d\n",rtk_timer_array_head);
	
		} else {
			break;
		}
	}

	return;
}

