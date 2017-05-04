#ifndef __RCM_TIMER_H__
#define __RCM_TIMER_H__

#define MAX_TIMER_ARRAY_NUM 50

#define null -1

typedef void(*FUNC)(void *);
typedef struct timeval Rtk_Time;


/* Initialize Timer */
void rcm_timer_subsystem_init(void);

/* Core function for Timer */

int rcm_timerCreate(FUNC callback,void *arg);
int rcm_timerLaunch(int timer_handleID,int timelength);
void rcm_timerCancel(int timer_handleID);
void rcm_timerDestory(int timer_handleID);

//int getHeadTimer(struct timeval *HeadTimer);

Rtk_Time *rcm_getHeadTimer(void);


#endif /* __RTK_TIMER_H__ */

