/*!
*	\file			cmbs_dsr.c
*	\brief
*	\Author		stein
*
*	@(#)	%filespec: cmbs_dsr.c-4 %
*
*******************************************************************************
*	\par	History
*	\n==== History =============================================================\n
*	date			name		version	 action                                       \n
*	----------------------------------------------------------------------------\n
*  25-Feb-09   stein    61        Initial revision                             \n
*  14-Dec-09   sergiym   ?        Add start/stop log commands                             \n

*******************************************************************************
*	COPYRIGHT DOSCH & AMAND RESEARCH GMBH & CO.KG
*	DOSCH & AMAND RESEARCH GMBH & CO.KG Confidential
*
*******************************************************************************/

#if defined(__arm)
# include "tclib.h"
# include "embedded.h"
#else
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
#endif

#include "cmbs_int.h"
#include "cfr_ie.h"
#include "cfr_debug.h"

/*****************************************************************************
 * DSR (service) functions
 *****************************************************************************/

E_CMBS_RC         cmbs_dsr_sys_Start( void * pv_AppRefHandle )
{
   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   return cmbs_int_EventSend( CMBS_EV_DSR_SYS_START, NULL, 0 );
}


E_CMBS_RC         cmbs_dsr_sys_Reset( void * pv_AppRefHandle )
{
   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   return cmbs_int_EventSend( CMBS_EV_DSR_SYS_RESET, NULL, 0 );
}


E_CMBS_RC         cmbs_dsr_sys_SendRawMsg( void * pv_AppRefHandle, u8 u8_Process, u8 u8_Instance,
                                           u8 u8_Event, u16 u16_Info, u8 u16_DataLen, u8 *pu8_Data )
{
   if( pv_AppRefHandle || u8_Process || u8_Instance || u8_Event || u16_Info || u16_DataLen || pu8_Data ){
   }  // unused. suppress compiler warnings

   /// \todo: implement cmbs_dsr_sys_SendRawMsg
   CFR_DBG_OUT( "cmbs_dsr_sys_SendRawMsg NOT implemented\n" );

   return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_dsr_param_Get( void * pv_AppRefHandle, E_CMBS_PARAM_TYPE e_ParamType, E_CMBS_PARAM e_Param )
{
	PST_CFR_IE_LIST   p_List;
	ST_IE_PARAMETER   st_Param;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

                                    // parameter validation
   if( e_ParamType != CMBS_PARAM_TYPE_EEPROM && e_ParamType != CMBS_PARAM_TYPE_PRODUCTION )
   {
      return CMBS_RC_ERROR_PARAMETER;
   }

   memset( &st_Param,0, sizeof(st_Param));

   st_Param.e_ParamType = e_ParamType;
   st_Param.e_Param     = e_Param;
   st_Param.pu8_Data    = NULL;
   st_Param.u16_DataLen = 0;

   p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   cmbs_int_ie_ParameterAdd( (void*)p_List, &st_Param );

   return cmbs_int_EventSend( CMBS_EV_DSR_PARAM_GET, p_List->pu8_Buffer, p_List->u16_CurSize );
}


E_CMBS_RC         cmbs_dsr_param_Set( void * pv_AppRefHandle, E_CMBS_PARAM_TYPE e_ParamType, E_CMBS_PARAM e_Param, u8 * pu8_Data, u16 u16_DataLen )
{
	PST_CFR_IE_LIST   p_List;
	ST_IE_PARAMETER   st_Param;
        u8             u8_Ret;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

                                    // parameter validation
   if( e_ParamType != CMBS_PARAM_TYPE_EEPROM && e_ParamType != CMBS_PARAM_TYPE_PRODUCTION )
   {
      return CMBS_RC_ERROR_PARAMETER;
   }

   if( (u8_Ret = cmbs_int_ParamValid(e_Param, pu8_Data, u16_DataLen)) != CMBS_RC_OK )
   {
      return u8_Ret;
   }

                                    // Add parameter to IE list
   st_Param.e_ParamType = e_ParamType;
   st_Param.e_Param     = e_Param;
   st_Param.pu8_Data    = pu8_Data;
   st_Param.u16_DataLen = u16_DataLen;

      p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

      cmbs_int_ie_ParameterAdd( (void*)p_List, &st_Param );

                                    // Send Parameter Set request
   return cmbs_int_EventSend( CMBS_EV_DSR_PARAM_SET, p_List->pu8_Buffer, p_List->u16_CurSize );
   }


E_CMBS_RC         cmbs_dsr_param_area_Get( void * pv_AppRefHandle, E_CMBS_PARAM_AREA_TYPE e_AreaType,
                                           u16 u16_Offset, u16 u16_DataLen )
{
	PST_CFR_IE_LIST
                  p_List;
	ST_IE_PARAMETER_AREA
                  st_ParamArea;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

                                    // parameter validation
   if( e_AreaType != CMBS_PARAM_AREA_TYPE_EEPROM || u16_DataLen > 512 )
   {
                                    // currently only area type EEPROM supported
                                    // maximum length is 512 bytes
      return CMBS_RC_ERROR_PARAMETER;
   }

   memset( &st_ParamArea,0, sizeof(st_ParamArea));

   st_ParamArea.e_AreaType  = e_AreaType;
   st_ParamArea.u16_Offset  = u16_Offset;
   st_ParamArea.u16_DataLen = u16_DataLen;
   st_ParamArea.pu8_Data    = NULL;

      p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   cmbs_int_ie_ParameterAreaAdd( (void*)p_List, &st_ParamArea );

   return cmbs_int_EventSend( CMBS_EV_DSR_PARAM_AREA_GET, p_List->pu8_Buffer, p_List->u16_CurSize );
}


E_CMBS_RC         cmbs_dsr_param_area_Set( void * pv_AppRefHandle, E_CMBS_PARAM_AREA_TYPE e_AreaType, u16 u16_Offset, u8* pu8_Data, u16 u16_DataLen )
{
	PST_CFR_IE_LIST
                  p_List;
	ST_IE_PARAMETER_AREA
                  st_ParamArea;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

                                    // parameter validation
   if( e_AreaType != CMBS_PARAM_AREA_TYPE_EEPROM || u16_DataLen > 512 )
   {
                                    // currently only area type EEPROM supported
                                    // maximum length is 512 bytes
      return CMBS_RC_ERROR_PARAMETER;
   }

   // Add parameter to IE list
   st_ParamArea.e_AreaType  = e_AreaType;
   st_ParamArea.u16_Offset  = u16_Offset;
   st_ParamArea.u16_DataLen = u16_DataLen;
   st_ParamArea.pu8_Data    = pu8_Data;

   p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   cmbs_int_ie_ParameterAreaAdd( (void*)p_List, &st_ParamArea );

                                    // Send Parameter Area Set request
   return cmbs_int_EventSend( CMBS_EV_DSR_PARAM_AREA_SET, p_List->pu8_Buffer, p_List->u16_CurSize );
}


E_CMBS_RC         cmbs_dsr_cord_OpenRegistration( void * pv_AppRefHandle )
{
   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   return cmbs_int_EventSend( CMBS_EV_DSR_CORD_OPENREG, NULL, 0 );
}


E_CMBS_RC         cmbs_dsr_cord_CloseRegistration( void * pv_AppRefHandle )
{
   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   return cmbs_int_EventSend( CMBS_EV_DSR_CORD_CLOSEREG, NULL, 0 );
}


E_CMBS_RC         cmbs_dsr_hs_Page( void * pv_AppRefHandle, u16 u16_Handsets )
{
	PST_CFR_IE_LIST p_List;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   cmbs_int_ie_IntValueAdd( (void*)p_List, (u32)u16_Handsets );

   return cmbs_int_EventSend( CMBS_EV_DSR_HS_PAGE_START, p_List->pu8_Buffer, p_List->u16_CurSize );
}


E_CMBS_RC         cmbs_dsr_hs_StopPaging( void * pv_AppRefHandle )
{
	PST_CFR_IE_LIST p_List;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   return cmbs_int_EventSend( CMBS_EV_DSR_HS_PAGE_STOP, NULL, 0 );
}


E_CMBS_RC         cmbs_dsr_HsLocProgress( void * pv_AppRefHandle, void * pv_RefIEList )
{
   PST_CFR_IE_LIST
                  p_List = (PST_CFR_IE_LIST)pv_RefIEList;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   return cmbs_int_EventSend( CMBS_EV_DSR_HS_PAGE_PROGRESS, p_List->pu8_Buffer, p_List->u16_CurSize );  
}


E_CMBS_RC         cmbs_dsr_HsLocAnswer( void * pv_AppRefHandle, void * pv_RefIEList )
{
   PST_CFR_IE_LIST
                  p_List = (PST_CFR_IE_LIST)pv_RefIEList;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   return cmbs_int_EventSend( CMBS_EV_DSR_HS_PAGE_ANSWER, p_List->pu8_Buffer, p_List->u16_CurSize );  
}


E_CMBS_RC         cmbs_dsr_HsLocRelease( void * pv_AppRefHandle, void * pv_RefIEList )
{
   PST_CFR_IE_LIST
                  p_List = (PST_CFR_IE_LIST)pv_RefIEList;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   return cmbs_int_EventSend( CMBS_EV_DSR_HS_PAGE_STOP, p_List->pu8_Buffer, p_List->u16_CurSize );  
}


E_CMBS_RC         cmbs_dsr_hs_Range( void * pv_AppRefHandle, u8 u8_Hs, E_CMBS_HS_RANGE_STATUS e_Status )
{
   PST_CFR_IE_LIST p_List;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   cmbs_int_ie_IntValueAdd( (void*)p_List, (u32)u8_Hs );
   //cmbs_int_ie_IntValueAdd( (void*)p_List, (u32)e_Status );

   if (e_Status == CMBS_HS_RANGE_STATUS_IN_RANGE)
   {
      return cmbs_int_EventSend( CMBS_EV_DSR_HS_IN_RANGE, p_List->pu8_Buffer, p_List->u16_CurSize );
   }

   return CMBS_RC_ERROR_NOT_SUPPORTED;
}


E_CMBS_RC         cmbs_dsr_hs_Delete( void * pv_AppRefHandle, u16 u16_Handsets )
{
	PST_CFR_IE_LIST p_List;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   cmbs_int_ie_IntValueAdd( (void*)p_List, (u32)u16_Handsets );

   return cmbs_int_EventSend( CMBS_EV_DSR_HS_DELETE, p_List->pu8_Buffer, p_List->u16_CurSize );
}


E_CMBS_RC         cmbs_dsr_fw_UpdateStart( void * pv_AppRefHandle, u8* pu8_Data, u16 u16_DataLen )
{
//   CFR_DBG_OUT( "cmbs_dsr_fw_UpdateStart %3d bytes\n", u16_DataLen );

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   return cmbs_int_EventSend( CMBS_EV_DSR_FW_UPD_START, pu8_Data, u16_DataLen );
}


E_CMBS_RC         cmbs_dsr_fw_UpdatePacketNext( void * pv_AppRefHandle, u8* pu8_Data, u16 u16_DataLen )
{
//   CFR_DBG_OUT( "cmbs_dsr_fw_UpdatePacketNext %3d bytes\n", u16_DataLen );

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   return cmbs_int_EventSend( CMBS_EV_DSR_FW_UPD_PACKETNEXT, pu8_Data, u16_DataLen );
}


E_CMBS_RC         cmbs_dsr_fw_UpdateEnd( void * pv_AppRefHandle, u8* pu8_Data, u16 u16_DataLen )
{
//   CFR_DBG_OUT( "cmbs_dsr_fw_UpdateEnd %3d bytes\n", u16_DataLen );

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   return cmbs_int_EventSend( CMBS_EV_DSR_FW_UPD_END, pu8_Data, u16_DataLen );
}


E_CMBS_RC         cmbs_dsr_fw_VersionGet( void * pv_AppRefHandle, E_CMBS_MODULE e_FwModule )
{
	PST_CFR_IE_LIST
                  p_List;
   ST_IE_FW_VERSION
                  st_FwVersion;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   memset( &st_FwVersion, 0, sizeof(ST_IE_FW_VERSION) );
   st_FwVersion.e_SwModule = e_FwModule;

   p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   cmbs_int_ie_FwVersionAdd( (void*)p_List, &st_FwVersion );

   return cmbs_int_EventSend( CMBS_EV_DSR_FW_VERSION_GET, p_List->pu8_Buffer, p_List->u16_CurSize );
}


E_CMBS_RC         cmbs_dsr_LogBufferStart( void * pv_AppRefHandle )
{
	PST_CFR_IE_LIST  p_List;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   return cmbs_int_EventSend( CMBS_EV_DSR_SYS_LOG_START, p_List->pu8_Buffer, p_List->u16_CurSize );
}


E_CMBS_RC         cmbs_dsr_LogBufferStop( void * pv_AppRefHandle )
{
	PST_CFR_IE_LIST  p_List;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   return cmbs_int_EventSend( CMBS_EV_DSR_SYS_LOG_STOP, p_List->pu8_Buffer, p_List->u16_CurSize );
}


E_CMBS_RC         cmbs_dsr_LogBufferRead( void * pv_AppRefHandle )
{
	PST_CFR_IE_LIST
                  p_List;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   return cmbs_int_EventSend( CMBS_EV_DSR_SYS_LOG_REQ, p_List->pu8_Buffer, p_List->u16_CurSize );
}


/*****************************************************************************
 * CAT-iq 2.0
 *****************************************************************************/

E_CMBS_RC         cmbs_dsr_gen_SendVMWI( void * pv_AppRefHandle,
                                         u16 u16_RequestId,
                                         u8 u8_Line,
                                         u16 u16_Handsets,
                                         u8 u8_NumOfMsg )
{
	PST_CFR_IE_LIST
                  p_List;
   ST_IE_GEN_EVENT
                  st_GenEvent;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   st_GenEvent.u8_SubType      = 0x01; // Voice message
   st_GenEvent.u8_MultiPlicity = u8_NumOfMsg;
   st_GenEvent.u8_LineId       = u8_Line;

   p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   cmbs_int_ie_ShortValueAdd( p_List, u16_RequestId, CMBS_IE_REQUEST_ID );
   cmbs_int_ie_GenEventAdd( p_List, &st_GenEvent );
   cmbs_int_ie_ShortValueAdd( p_List, u16_Handsets, CMBS_IE_HANDSETS );

   return cmbs_int_EventSend( CMBS_EV_DSR_GEN_SEND_VMWI, p_List->pu8_Buffer, p_List->u16_CurSize );
}


E_CMBS_RC         cmbs_dsr_gen_SendMissedCalls( void * pv_AppRefHandle,
                                                u16 u16_RequestId,
                                                u8 u8_Line,
                                                u16 u16_Handsets,
                                                u8 u8_NumOfCalls )
{
	PST_CFR_IE_LIST
                  p_List;
   ST_IE_GEN_EVENT
                  st_GenEvent;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   st_GenEvent.u8_SubType      = 0x01; // Voice call
   st_GenEvent.u8_MultiPlicity = u8_NumOfCalls;
   st_GenEvent.u8_LineId       = u8_Line;

   p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   cmbs_int_ie_ShortValueAdd( p_List, u16_RequestId, CMBS_IE_REQUEST_ID );
   cmbs_int_ie_GenEventAdd( p_List, &st_GenEvent );
   cmbs_int_ie_ShortValueAdd( p_List, u16_Handsets, CMBS_IE_HANDSETS );

   return cmbs_int_EventSend( CMBS_EV_DSR_GEN_SEND_MISSED_CALLS, p_List->pu8_Buffer, p_List->u16_CurSize );
}


E_CMBS_RC         cmbs_dsr_gen_SendListChanged( void * pv_AppRefHandle,
                                                u16 u16_RequestId,
                                                u16 u16_Handsets,
                                                u8  u8_ListId,
                                                u16 u16_NumOfEntries )
{
	PST_CFR_IE_LIST
                  p_List;
   ST_IE_GEN_EVENT
                  st_GenEvent;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   st_GenEvent.u8_SubType      = u8_ListId;
   st_GenEvent.u8_MultiPlicity = (u8)u16_NumOfEntries;
   st_GenEvent.u8_LineId       = 0xFF; // no specific line

   p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   cmbs_int_ie_ShortValueAdd( p_List, u16_RequestId, CMBS_IE_REQUEST_ID );
   cmbs_int_ie_GenEventAdd( p_List, &st_GenEvent );
   cmbs_int_ie_ShortValueAdd( p_List, u16_Handsets, CMBS_IE_HANDSETS );

   return cmbs_int_EventSend( CMBS_EV_DSR_GEN_SEND_LIST_CHANGED, p_List->pu8_Buffer, p_List->u16_CurSize );
}


E_CMBS_RC         cmbs_dsr_gen_SendWebContent( void * pv_AppRefHandle,
                                               u16 u16_RequestId,
                                               u16 u16_Handsets,
                                               u8  u8_NumOfWebCont )
{
	PST_CFR_IE_LIST
                  p_List;
   ST_IE_GEN_EVENT
                  st_GenEvent;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   st_GenEvent.u8_SubType      = 0x01; // RSS description
   st_GenEvent.u8_MultiPlicity = u8_NumOfWebCont;
   st_GenEvent.u8_LineId       = 0xFF; // no specific line

   p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   cmbs_int_ie_ShortValueAdd( p_List, u16_RequestId, CMBS_IE_REQUEST_ID );
   cmbs_int_ie_GenEventAdd( p_List, &st_GenEvent );
   cmbs_int_ie_ShortValueAdd( p_List, u16_Handsets, CMBS_IE_HANDSETS );

   return cmbs_int_EventSend( CMBS_EV_DSR_GEN_SEND_WEB_CONTENT, p_List->pu8_Buffer, p_List->u16_CurSize );
}


E_CMBS_RC         cmbs_dsr_gen_SendPropEvent( void * pv_AppRefHandle,
                                              u16 u16_RequestId,
                                              u16 u16_PropEvent,
                                              u8 * pu8_Data,
                                              u8  u8_DataLen,
                                              u16 u16_Handsets )
{
	PST_CFR_IE_LIST
                  p_List;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   cmbs_int_ie_ShortValueAdd( p_List, u16_RequestId, CMBS_IE_REQUEST_ID );
   cmbs_int_ie_PropEventAdd( p_List, u16_PropEvent, pu8_Data, u8_DataLen );
   cmbs_int_ie_ShortValueAdd( p_List, u16_Handsets, CMBS_IE_HANDSETS );

   return cmbs_int_EventSend( CMBS_EV_DSR_GEN_SEND_PROP_EVENT, p_List->pu8_Buffer, p_List->u16_CurSize );
}


E_CMBS_RC         cmbs_dsr_hs_DataSessionOpen( void * pv_AppRefHandle,
                                               ST_DATA_SESSION_TYPE * pst_DataSessionType,
                                               u16 u16_Handsets )
{
	PST_CFR_IE_LIST
                  p_List;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   cmbs_int_ie_DataSessionTypeAdd( p_List, pst_DataSessionType );
   cmbs_int_ie_ShortValueAdd( p_List, u16_Handsets, CMBS_IE_HANDSETS );

   return cmbs_int_EventSend( CMBS_EV_DSR_HS_DATA_SESSION_OPEN, p_List->pu8_Buffer, p_List->u16_CurSize );
}


E_CMBS_RC         cmbs_dsr_hs_DataSessionClose( void * pv_AppRefHandle, u16 u16_SessionId )
{
	PST_CFR_IE_LIST
                  p_List;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   cmbs_int_ie_ShortValueAdd( p_List, u16_SessionId, CMBS_IE_DATA_SESSION_ID );

   return cmbs_int_EventSend( CMBS_EV_DSR_HS_DATA_SESSION_CLOSE, p_List->pu8_Buffer, p_List->u16_CurSize );
}


E_CMBS_RC         cmbs_dsr_hs_DataSend( void * pv_AppRefHandle,
                                        u16 u16_SessionId,
                                        u8 * pu8_Data,
                                        u16 u16_DataLen )
{
	PST_CFR_IE_LIST
                  p_List;
   ST_IE_DATA     st_Data;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   st_Data.u16_DataLen = u16_DataLen;
   st_Data.pu8_Data    = pu8_Data;

   cmbs_int_ie_ShortValueAdd( p_List, u16_SessionId, CMBS_IE_DATA_SESSION_ID );
   cmbs_int_ie_DataAdd( p_List, &st_Data );

   return cmbs_int_EventSend( CMBS_EV_DSR_HS_DATA_RECEIVE, p_List->pu8_Buffer, p_List->u16_CurSize );
}


E_CMBS_RC         cmbs_dsr_time_Update( void * pv_AppRefHandle,
                                        u16 u16_RequestId,
                                        ST_DATE_TIME * pst_DateTime,
                                        u16 u16_Handsets )
{
	PST_CFR_IE_LIST
                  p_List;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   cmbs_int_ie_ShortValueAdd( p_List, u16_RequestId, CMBS_IE_REQUEST_ID );
   cmbs_int_ie_DateTimeAdd( p_List, (PST_IE_DATETIME)pst_DateTime );
   cmbs_int_ie_ShortValueAdd( p_List, u16_Handsets, CMBS_IE_HANDSETS );

   return cmbs_int_EventSend( CMBS_EV_DSR_TIME_UPDATE, p_List->pu8_Buffer, p_List->u16_CurSize );
}


E_CMBS_RC         cmbs_dsr_la_SessionStartRes( void * pv_AppRefHandle,
                                               u16 u16_SessionId,
                                               u16 u16_NumOfEntries,
                                               ST_LA_FIELDS * pst_LASortFields,
                                               E_CMBS_RESPONSE e_Response,
                                               E_CMBS_LA_START_SESSION_ERRORS e_RejectReason )
{
   PST_CFR_IE_LIST
                  p_List;
   ST_IE_RESPONSE st_Response;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   // Add LA Session Id IE
   cmbs_int_ie_ShortValueAdd( p_List, u16_SessionId, CMBS_IE_LA_SESSION_ID );

   // Add LA Nr of Entries IE
   cmbs_int_ie_ShortValueAdd( p_List, u16_NumOfEntries, CMBS_IE_LA_NR_OF_ENTRIES );

   // Add Sort Fields IE
   cmbs_int_ie_LAFieldsAdd( p_List, pst_LASortFields, CMBS_IE_LA_SORT_FIELDS );

   // Add reject reason if there was an error
   if( e_Response == CMBS_RESPONSE_ERROR )
   {
      cmbs_int_ie_ByteValueAdd( p_List, (u8)e_RejectReason, CMBS_IE_LA_REJECT_REASON );
   }

   // Add response IE
   st_Response.e_Response = e_Response;
   cmbs_int_ie_ResponseAdd( p_List, &st_Response );

   return cmbs_int_EventSend( CMBS_EV_DSR_LA_SESSION_START_RES, p_List->pu8_Buffer, p_List->u16_CurSize );
}


E_CMBS_RC         cmbs_dsr_la_SessionEndRes( void * pv_AppRefHandle,
                                             u16 u16_SessionId,
                                             E_CMBS_RESPONSE e_Response,
                                             E_CMBS_LA_NACK_ERRORS e_RejectReason )
{
   PST_CFR_IE_LIST
                  p_List;
   ST_IE_RESPONSE st_Response;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   // Add LA Session Id IE
   cmbs_int_ie_ShortValueAdd( p_List, u16_SessionId, CMBS_IE_LA_SESSION_ID );

   // Add reject reason if there was an error
   if( e_Response == CMBS_RESPONSE_ERROR )
   {
      cmbs_int_ie_ByteValueAdd( p_List, (u8)e_RejectReason, CMBS_IE_LA_REJECT_REASON );
   }

   // Add response IE
   st_Response.e_Response = e_Response;
   cmbs_int_ie_ResponseAdd( p_List, &st_Response );


   return cmbs_int_EventSend( CMBS_EV_DSR_LA_SESSION_END_RES, p_List->pu8_Buffer, p_List->u16_CurSize );
}


E_CMBS_RC         cmbs_dsr_la_QuerySupEntryFieldsRes( void * pv_AppRefHandle,
                                                      u16 u16_SessionId,
                                                      ST_LA_FIELDS * pst_LANonEditFields,
                                                      ST_LA_FIELDS * pst_LAEditFields,
                                                      E_CMBS_RESPONSE e_Response,
                                                      E_CMBS_LA_NACK_ERRORS e_RejectReason )
{
   PST_CFR_IE_LIST
                  p_List;
   ST_IE_RESPONSE st_Response;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   // Add LA Session Id IE
   cmbs_int_ie_ShortValueAdd( p_List, u16_SessionId, CMBS_IE_LA_SESSION_ID );

   // Add editable fields
   cmbs_int_ie_LAFieldsAdd( p_List, pst_LAEditFields, CMBS_IE_LA_EDIT_FIELDS );

   // Add non-editable fields
   cmbs_int_ie_LAFieldsAdd( p_List, pst_LANonEditFields, CMBS_IE_LA_CONST_FIELDS );

   // Add reject reason if there was an error
   if( e_Response == CMBS_RESPONSE_ERROR )
   {
      cmbs_int_ie_ByteValueAdd( p_List, (u8)e_RejectReason, CMBS_IE_LA_REJECT_REASON );
   }

   // Add response IE
   st_Response.e_Response = e_Response;
   cmbs_int_ie_ResponseAdd( p_List, &st_Response );

   return cmbs_int_EventSend( CMBS_EV_DSR_LA_QUERY_SUPP_ENTRY_FIELDS_RES, p_List->pu8_Buffer, p_List->u16_CurSize );
}


E_CMBS_RC         cmbs_dsr_la_SearchEntriesRes( void * pv_AppRefHandle,
                                                u16 u16_SessionId,
                                                u16 u16_EntryStartIndex,
                                                u16 u16_NumOfEntries,
                                                E_CMBS_RESPONSE e_Response,
                                                E_CMBS_LA_NACK_ERRORS e_RejectReason )
{
   PST_CFR_IE_LIST
                  p_List;
   ST_IE_RESPONSE st_Response;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   // Add LA Session Id IE
   cmbs_int_ie_ShortValueAdd( p_List, u16_SessionId, CMBS_IE_LA_SESSION_ID );

   // Add start index
   cmbs_int_ie_ShortValueAdd( p_List, u16_EntryStartIndex, CMBS_IE_LA_ENTRY_INDEX );

   // Add field LA Nr of Entries
   cmbs_int_ie_ShortValueAdd( p_List, u16_NumOfEntries, CMBS_IE_LA_NR_OF_ENTRIES );

   // Add reject reason if there was an error
   if( e_Response == CMBS_RESPONSE_ERROR )
   {
      cmbs_int_ie_ByteValueAdd( p_List, (u8)e_RejectReason, CMBS_IE_LA_REJECT_REASON );
   }

   // Add response IE
   st_Response.e_Response = e_Response;
   cmbs_int_ie_ResponseAdd( p_List, &st_Response );

   return cmbs_int_EventSend( CMBS_EV_DSR_LA_SEARCH_ENTRIES_RES, p_List->pu8_Buffer, p_List->u16_CurSize );
}


E_CMBS_RC         cmbs_dsr_la_ReadEntriesRes( void * pv_AppRefHandle,
                                                u16 u16_SessionId,
                                                u16 u16_EntryStartIndex,
                                                u16 u16_NumOfEntries,
                                                E_CMBS_RESPONSE e_Response,
                                                E_CMBS_LA_NACK_ERRORS e_RejectReason )
{
   PST_CFR_IE_LIST
                  p_List;
   ST_IE_RESPONSE st_Response;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   // Add LA Session Id IE
   cmbs_int_ie_ShortValueAdd( p_List, u16_SessionId, CMBS_IE_LA_SESSION_ID );

   // Add start index
   cmbs_int_ie_ShortValueAdd( p_List, u16_EntryStartIndex, CMBS_IE_LA_ENTRY_INDEX );

   // Add counter
   cmbs_int_ie_ShortValueAdd( p_List, u16_NumOfEntries, CMBS_IE_LA_ENTRY_COUNT );

   // Add reject reason if there was an error
   if( e_Response == CMBS_RESPONSE_ERROR )
   {
      cmbs_int_ie_ByteValueAdd( p_List, (u8)e_RejectReason, CMBS_IE_LA_REJECT_REASON );
   }

   // Add response IE
   st_Response.e_Response = e_Response;
   cmbs_int_ie_ResponseAdd( p_List, &st_Response );

   return cmbs_int_EventSend( CMBS_EV_DSR_LA_READ_ENTRIES_RES, p_List->pu8_Buffer, p_List->u16_CurSize );
}


E_CMBS_RC         cmbs_dsr_la_EditEntryRes( void * pv_AppRefHandle,
                                            u16 u16_SessionId,
                                            E_CMBS_RESPONSE e_Response,
                                            E_CMBS_LA_NACK_ERRORS e_RejectReason )
{
   PST_CFR_IE_LIST
                  p_List;
   ST_IE_RESPONSE st_Response;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   // Add LA Session Id IE
   cmbs_int_ie_ShortValueAdd( p_List, u16_SessionId, CMBS_IE_LA_SESSION_ID );

   // Add reject reason if there was an error
   if( e_Response == CMBS_RESPONSE_ERROR )
   {
      cmbs_int_ie_ByteValueAdd( p_List, (u8)e_RejectReason, CMBS_IE_LA_REJECT_REASON );
   }

   // Add response IE
   st_Response.e_Response = e_Response;
   cmbs_int_ie_ResponseAdd( p_List, &st_Response );

   return cmbs_int_EventSend( CMBS_EV_DSR_LA_EDIT_ENTRY_RES, p_List->pu8_Buffer, p_List->u16_CurSize );
}


E_CMBS_RC         cmbs_dsr_la_SaveEntryRes( void * pv_AppRefHandle,
                                            u16 u16_SessionId,
                                            u16 u16_EntryId,
                                            u16 u16_EntryIndex,
                                            u16 u16_NumOfEntries,
                                            E_CMBS_RESPONSE e_Response,
                                            E_CMBS_LA_NACK_ERRORS e_RejectReason )
{
   PST_CFR_IE_LIST
                  p_List;
   ST_IE_RESPONSE st_Response;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   // Add LA Session Id IE
   cmbs_int_ie_ShortValueAdd( p_List, u16_SessionId, CMBS_IE_LA_SESSION_ID );

   // Add entry Id
   cmbs_int_ie_ShortValueAdd( p_List, u16_EntryId, CMBS_IE_LA_ENTRY_ID );

   // Add entry index
   cmbs_int_ie_ShortValueAdd( p_List, u16_EntryIndex, CMBS_IE_LA_ENTRY_INDEX );

   // Add field LA Nr of Entries
   cmbs_int_ie_ShortValueAdd( p_List, u16_NumOfEntries, CMBS_IE_LA_NR_OF_ENTRIES );

   // Add reject reason if there was an error
   if( e_Response == CMBS_RESPONSE_ERROR )
   {
      cmbs_int_ie_ByteValueAdd( p_List, (u8)e_RejectReason, CMBS_IE_LA_REJECT_REASON );
   }

   // Add response IE
   st_Response.e_Response = e_Response;
   cmbs_int_ie_ResponseAdd( p_List, &st_Response );

   return cmbs_int_EventSend( CMBS_EV_DSR_LA_SAVE_ENTRY_RES, p_List->pu8_Buffer, p_List->u16_CurSize );
}


E_CMBS_RC         cmbs_dsr_la_DeleteEntryRes( void * pv_AppRefHandle,
                                              u16 u16_SessionId,
                                              u16 u16_TotalNoOfEntries,
                                              E_CMBS_RESPONSE e_Response,
                                              E_CMBS_LA_NACK_ERRORS e_RejectReason )
{
   PST_CFR_IE_LIST
                  p_List;
   ST_IE_RESPONSE st_Response;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   // Add LA Session Id IE
   cmbs_int_ie_ShortValueAdd( p_List, u16_SessionId, CMBS_IE_LA_SESSION_ID );

   // Add LA Entry Count IE
   cmbs_int_ie_ShortValueAdd( p_List, u16_TotalNoOfEntries, CMBS_IE_LA_ENTRY_COUNT );

   // Add reject reason if there was an error
   if( e_Response == CMBS_RESPONSE_ERROR )
   {
      cmbs_int_ie_ByteValueAdd( p_List, (u8)e_RejectReason, CMBS_IE_LA_REJECT_REASON );
   }

   // Add response IE
   st_Response.e_Response = e_Response;
   cmbs_int_ie_ResponseAdd( p_List, &st_Response );

   return cmbs_int_EventSend( CMBS_EV_DSR_LA_DELETE_ENTRY_RES, p_List->pu8_Buffer, p_List->u16_CurSize );
}


E_CMBS_RC         cmbs_dsr_la_DeleteListRes( void * pv_AppRefHandle,
                                             u16 u16_SessionId,
                                             E_CMBS_RESPONSE e_Response,
                                             E_CMBS_LA_NACK_ERRORS e_RejectReason )
{
   PST_CFR_IE_LIST
                  p_List;
   ST_IE_RESPONSE st_Response;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   // Add LA Session Id IE
   cmbs_int_ie_ShortValueAdd( p_List, u16_SessionId, CMBS_IE_LA_SESSION_ID );

   // Add reject reason if there was an error
   if( e_Response == CMBS_RESPONSE_ERROR )
   {
      cmbs_int_ie_ByteValueAdd( p_List, (u8)e_RejectReason, CMBS_IE_LA_REJECT_REASON );
   }

   // Add response IE
   st_Response.e_Response = e_Response;
   cmbs_int_ie_ResponseAdd( p_List, &st_Response );

   return cmbs_int_EventSend( CMBS_EV_DSR_LA_DELETE_LIST_RES, p_List->pu8_Buffer, p_List->u16_CurSize );
}


E_CMBS_RC         cmbs_dsr_la_SessionEnd( void * pv_AppRefHandle, u16 u16_SessionId )
{
   PST_CFR_IE_LIST
                  p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   // Add LA Session Id IE
   cmbs_int_ie_ShortValueAdd( p_List, u16_SessionId, CMBS_IE_LA_SESSION_ID );

   return cmbs_int_EventSend( CMBS_EV_DSR_LA_SESSION_END_RES, p_List->pu8_Buffer, p_List->u16_CurSize );
}


E_CMBS_RC         cmbs_dsr_la_DataPacketSend( void * pv_AppRefHandle,
                                              u16 u16_SessionId,
                                              u8 u8_IsLast,
                                              u8 * pu8_Data,
                                              u16 u16_DataLen )
{
   ST_IE_DATA     st_Data;
   PST_CFR_IE_LIST
                  p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   // Add LA Session Id IE
   cmbs_int_ie_ShortValueAdd( p_List, u16_SessionId, CMBS_IE_LA_SESSION_ID );
   
   // Add LA IsLast IE
   cmbs_int_ie_ByteValueAdd( p_List, u8_IsLast, CMBS_IE_LA_IS_LAST );

   // Add Data IE
   st_Data.u16_DataLen = u16_DataLen;
   st_Data.pu8_Data    = pu8_Data;

   cmbs_int_ie_DataAdd( p_List, &st_Data );

   return cmbs_int_EventSend( CMBS_EV_DSR_LA_DATA_PACKET_SEND, p_List->pu8_Buffer, p_List->u16_CurSize );
}


E_CMBS_RC         cmbs_dsr_la_DataPacketReceivedRes( void * pv_AppRefHandle,
                                             u16 u16_SessionId,
                                             E_CMBS_RESPONSE e_Response)
{
   PST_CFR_IE_LIST p_List;
   ST_IE_RESPONSE st_Response;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   // Add LA Session Id IE
   cmbs_int_ie_ShortValueAdd( p_List, u16_SessionId, CMBS_IE_LA_SESSION_ID );


   // Add response IE
   st_Response.e_Response = e_Response;
   cmbs_int_ie_ResponseAdd( p_List, &st_Response );

   return cmbs_int_EventSend( CMBS_EV_DSR_LA_DATA_PACKET_RECEIVE_RES, p_List->pu8_Buffer, p_List->u16_CurSize );
}



/* == ALTDV == */
E_CMBS_RC         cmbs_dsr_sys_PowerOff( void * pv_AppRefHandle )
{
   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   return cmbs_int_EventSend( CMBS_EV_DSR_SYS_POWER_OFF, NULL, 0 );
}

E_CMBS_RC         cmbs_dsr_sys_RFSuspend( void * pv_AppRefHandle )
{
   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   return cmbs_int_EventSend( CMBS_EV_DSR_RF_SUSPEND, NULL, 0 );
}

E_CMBS_RC         cmbs_dsr_sys_RFResume( void * pv_AppRefHandle )
{
   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   return cmbs_int_EventSend( CMBS_EV_DSR_RF_RESUME, NULL, 0 );
}


E_CMBS_RC         cmbs_dsr_sys_TurnOnNEMo( void * pv_AppRefHandle )
{
   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   return cmbs_int_EventSend( CMBS_EV_DSR_TURN_ON_NEMO, NULL, 0 );
}

E_CMBS_RC         cmbs_dsr_sys_TurnOffNEMo( void * pv_AppRefHandle )
{
   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   return cmbs_int_EventSend( CMBS_EV_DSR_TURN_OFF_NEMO, NULL, 0 );
}

E_CMBS_RC         cmbs_dsr_GET_InternalnameList( void * pv_AppRefHandle, u16 u16_HsMask  )
{
   PST_CFR_IE_LIST              p_List;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   cmbs_int_ie_ShortValueAdd( (void*)p_List, u16_HsMask, CMBS_IE_HANDSETS );

   return cmbs_int_EventSend( CMBS_EV_DSR_HS_SUBSCRIBED_LIST_GET, p_List->pu8_Buffer, p_List->u16_CurSize );
}

E_CMBS_RC         cmbs_dsr_SET_InternalnameList( void * pv_AppRefHandle, ST_IE_SUBSCRIBED_HS_LIST * pst_SubscribedHsList )
{
   PST_CFR_IE_LIST  p_List;

   if ( !pst_SubscribedHsList  )
   {
      return CMBS_RC_ERROR_PARAMETER;
   }

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   p_List = cmbs_api_ie_GetList();

   cmbs_api_ie_SubscribedHSListAdd( (void*)p_List, pst_SubscribedHsList );

   return cmbs_int_EventSend( CMBS_EV_DSR_HS_SUBSCRIBED_LIST_SET, p_List->pu8_Buffer, p_List->u16_CurSize );
}

E_CMBS_RC         cmbs_dsr_GET_Line_setting_list( void * pv_AppRefHandle, u16 u16_LineMask )
{
   PST_CFR_IE_LIST              p_List;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   cmbs_int_ie_ShortValueAdd( (void*)p_List, u16_LineMask, CMBS_IE_REQUEST_ID );

   return cmbs_int_EventSend( CMBS_EV_DSR_LINE_SETTINGS_LIST_GET, p_List->pu8_Buffer, p_List->u16_CurSize );
}

E_CMBS_RC         cmbs_dsr_SET_Line_setting_list ( void * pv_AppRefHandle, 
                                                   ST_IE_LINE_SETTINGS_TYPE* pst_LineSettingsType,
                                                   ST_IE_LINE_SETTINGS_LIST* pst_LineSettingsList ) 
{
   PST_CFR_IE_LIST              p_List;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   cmbs_api_ie_LineSettingsTypeAdd( (void*)p_List, pst_LineSettingsType );
   cmbs_api_ie_LineSettingsListAdd( (void*)p_List, pst_LineSettingsList );

   return cmbs_int_EventSend( CMBS_EV_DSR_LINE_SETTINGS_LIST_SET, p_List->pu8_Buffer, p_List->u16_CurSize );
}



//*/
