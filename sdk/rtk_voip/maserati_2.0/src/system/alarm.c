#include <time.h>

#include "linphonecore.h"
#include "alarm.h"

#define ALARM_RINGING_PERIOD		10	// 10 seconds 

typedef struct {
	unsigned char ring_last_day;
	unsigned char ring_defer;
	unsigned char ringing;
	time_t start_time;
} alarm_t;

static alarm_t varAlarm[ MAX_VOIP_PORTS ];

static void StartAlarmRinging( LinphoneCore *lc );

void InitializeAlarmVariable( void )
{
	int i;
	
	for( i = 0; i < MAX_VOIP_PORTS; i ++ ) {
		varAlarm[ i ].ring_last_day = 0;
		varAlarm[ i ].ring_defer = 0;
		varAlarm[ i ].ringing = 0;
	}
}

static int CheckIfStartToRing( LinphoneCore *lc )
{
	time_t now;
	struct tm *tmNow;
	const voipCfgPortParam_t * const pCfg = &g_pVoIPCfg->ports[lc->chid];
	alarm_t * const pAlarm = &varAlarm[ lc ->chid ];
	
	// eanble ?
	if( !pCfg ->alarm_enable )
		return 0;
	
	if( pAlarm ->ring_defer )
		goto label_do_ringing_due_to_defer;
	
	// get current time 
	time( &now );
	
	tmNow = localtime( &now );
	
	// check last ring 
	if( pAlarm ->ring_last_day == tmNow ->tm_mday )
		return 0;
	
	// check hh:mm 
	if( pCfg ->alarm_time_hh == tmNow ->tm_hour &&
		pCfg ->alarm_time_mm == tmNow ->tm_min )
	{
		goto label_do_ringing_right_now;
	}
	
	return 0;
	
label_do_ringing_right_now:

	pAlarm ->ring_last_day = tmNow ->tm_mday;
	pAlarm ->ring_defer = 1;

label_do_ringing_due_to_defer:

	return 1;
}

static void StartAlarmRinging( LinphoneCore *lc )
{
	alarm_t * const pAlarm = &varAlarm[ lc ->chid ];

	// turn on the ring 
	rcm_tapi_SetRingFXS( lc ->chid, 1 );

	pAlarm ->ring_defer = 0;
	pAlarm ->ringing = 1;
	time( &pAlarm ->start_time );
}

void StopAlarmRinging( LinphoneCore *lc )
{
	const voipCfgPortParam_t * const pCfg = &g_pVoIPCfg->ports[lc->chid];
	alarm_t * const pAlarm = &varAlarm[ lc ->chid ];

	// eanble ?
	if( !pCfg ->alarm_enable )
		return;

	// FXS only 
	//if( lc ->chid >= RTK_VOIP_SLIC_NUM( g_VoIP_Feature ) )
	//	return;
	if( !RTK_VOIP_IS_SLIC_CH( lc ->chid, g_VoIP_Feature ) )
		return;
		
	// is it ringing? 
	if( !pAlarm ->ringing )
		return;

	pAlarm ->ringing = 0;

	rtk_SetRingFXS( lc ->chid, 0 );
}

static int CheckIfStopRinging( LinphoneCore *lc )
{
	time_t now;
	const voipCfgPortParam_t * const pCfg = &g_pVoIPCfg->ports[lc->chid];
	alarm_t * const pAlarm = &varAlarm[ lc ->chid ];
	
	// eanble ?
	if( !pCfg ->alarm_enable )
		return 0;
	
	// pAlarm ->ringing
	time( &now );
	
	if( now - pAlarm ->start_time >= ALARM_RINGING_PERIOD )
		return 1;
	
	return 0;	
}

void ProcessAlarmEvent( LinphoneCore *lc )
{
	const voipCfgPortParam_t * const pCfg = &g_pVoIPCfg->ports[lc->chid];
	alarm_t * const pAlarm = &varAlarm[ lc ->chid ];
	SYSTEMSTSTE sysstate;

	// eanble ?
	if( !pCfg ->alarm_enable )
		return;
	
	// FXS only 
	//if( lc ->chid >= RTK_VOIP_SLIC_NUM( g_VoIP_Feature ) )
	//	return;
	if( !RTK_VOIP_IS_SLIC_CH( lc ->chid, g_VoIP_Feature ) )
		return;
	
	if( pAlarm ->ringing ) {
		if( CheckIfStopRinging( lc ) ) {
			StopAlarmRinging( lc );
		}
	} else {
		if( CheckIfStartToRing( lc ) ) {

			// check state 	
			sysstate = GetSysState( lc ->chid );
		
			if( sysstate != SYS_STATE_IDLE )
				return;
				
			StartAlarmRinging( lc );
		}
	}
}

