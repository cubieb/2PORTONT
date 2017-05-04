#ifndef __ALARM_H__
#define __ALARM_H__

/* Initialize alarm */
extern void InitializeAlarmVariable( void );

/* Core function for alarm */
extern void ProcessAlarmEvent( LinphoneCore *lc );

/* Stop alarm rining, when off-hook or incoming call */
extern void StopAlarmRinging( LinphoneCore *lc );

#endif /* __ALARM_H__ */

