/*!
*	\file
*	\brief
*	\Author		kelbch
*
*	@(#)	%filespec: appsrv.h~DMZD53#6 %
*
*******************************************************************************
*	\par	History
*	\n==== History ============================================================\n
*	date			name		version	 action                                          \n
*	----------------------------------------------------------------------------\n
*******************************************************************************/

#if	!defined( APPSRV_H )
#define	APPSRV_H


#if defined( __cplusplus )
extern "C"
{
#endif

E_CMBS_RC      app_SrvHandsetDelete ( char * psz_Handsets );
E_CMBS_RC      app_SrvHandsetPage( char * psz_Handsets );
E_CMBS_RC      app_SrvHandsetStopPaging( void );
E_CMBS_RC      app_SrvSubscriptionOpen( void );
E_CMBS_RC      app_SrvSubscriptionClose( void );
//E_CMBS_RC      app_SrvPINCodeGet( u32 u32_Token );
E_CMBS_RC      app_SrvPINCodeSet( char * psz_PIN );
//E_CMBS_RC      app_SrvRFPIGet( u32 u32_Token );
E_CMBS_RC      app_SrvTestModeGet( u32 u32_Token );
E_CMBS_RC      app_SrvTestModeSet( void );
E_CMBS_RC      app_SrvFWVersionGet( u32 u32_Token );
E_CMBS_RC      app_SrvLogBufferStart( void );
E_CMBS_RC      app_SrvLogBufferStop( void );
E_CMBS_RC      app_SrvLogBufferRead( u32 u32_Token );
E_CMBS_RC      app_SrvSystemReboot( void );
E_CMBS_RC      app_SrvSystemPowerOff( void );
E_CMBS_RC      app_SrvRegisteredHandsets( u16 u16_HsMask, u32 u32_Token );
E_CMBS_RC      app_SrvSetNewHandsetName( u16 u16_HsId, u16* pu16_HsName, u16 u16_HsNameSize, u32 u32_Token );
E_CMBS_RC      app_SrvLineSettingsGet( u16 u16_LinesMask, u32 u32_Token );
E_CMBS_RC      app_SrvLineSettingsSet( ST_IE_LINE_SETTINGS_TYPE* pst_LineSettingsType, 
                                       ST_IE_LINE_SETTINGS_LIST* pst_LineSettingsList, u32 u32_Token );
E_CMBS_RC      app_SrvRFSuspend( void );
E_CMBS_RC      app_SrvRFResume( void );
E_CMBS_RC      app_SrvTurnOnNEMo( void );
E_CMBS_RC      app_SrvTurnOffNEMo( void );
E_CMBS_RC      app_SrvParamGet( E_CMBS_PARAM e_Param, u32 u32_Token );
E_CMBS_RC      app_SrvParamSet( E_CMBS_PARAM e_Param, u8* pu8_Data, u16 u16_Length, u32 u32_Token );
E_CMBS_RC      app_ProductionParamGet( E_CMBS_PARAM e_Param, u32 u32_Token );
E_CMBS_RC      app_ProductionParamSet( E_CMBS_PARAM e_Param, u8* pu8_Data, u16 u16_Length, u32 u32_Token );
E_CMBS_RC      app_SrvParamAreaGet( E_CMBS_PARAM_AREA_TYPE e_AreaType, u32 u32_Pos, u16 u16_Length, u32 u32_Token );
E_CMBS_RC      app_SrvParamAreaSet( E_CMBS_PARAM_AREA_TYPE e_AreaType, u32 u32_Pos, u16 u16_Length, u8* pu8_Data, u32 u32_Token );
E_CMBS_RC      app_OnHandsetLinkRelease( void * pv_List );

int            app_ServiceEntity( void * pv_AppRef, E_CMBS_EVENT_ID e_EventID, void * pv_EventData );


#if defined( __cplusplus )
}
#endif

#endif	//APPSRV	_H
//*/
