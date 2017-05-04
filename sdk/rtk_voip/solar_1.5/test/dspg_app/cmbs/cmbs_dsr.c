/*!
*	\file			cmbs_dsr.c
*	\brief
*	\Author		stein
*
*	@(#)	%filespec: cmbs_dsr.c-1 %
*
*******************************************************************************
*	\par	History
*	\n==== History =============================================================\n
*	date			name		version	 action                                       \n
*	----------------------------------------------------------------------------\n
*  25-Feb-09   stein    61        Initial revision                             \n

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

E_CMBS_RC         cmbs_dsr_param_Get( void * pv_AppRefHandle, E_CMBS_PARAM e_Param )
{
	PST_CFR_IE_LIST   p_List;
	ST_IE_PARAMETER   st_Param;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   memset( &st_Param,0, sizeof(st_Param));

   st_Param.e_Param     = e_Param;
   st_Param.pu8_Data    = NULL;
   st_Param.u16_DataLen = 0;

   p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   cmbs_int_ie_ParameterAdd( (void*)p_List, &st_Param );

   return cmbs_int_EventSend( CMBS_EV_DSR_PARAM_GET, p_List->pu8_Buffer, p_List->u16_CurSize );
}

E_CMBS_RC         cmbs_dsr_param_area_Get( void * pv_AppRefHandle , E_CMBS_MEM_TYPE e_MemType, u32 u32_Location, u16 u16_Size )
{
	PST_CFR_IE_LIST   p_List;
	ST_IE_PARAMETER   st_Param;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   if ( e_MemType == CMBS_MEM_EEPROM )
   {
      memset( &st_Param,0, sizeof(st_Param));

      st_Param.e_Param     = CMBS_PARAM_FLEX;
      st_Param.u16_Position= (u16)u32_Location;
      st_Param.pu8_Data    = NULL;
      st_Param.u16_DataLen = u16_Size;

      p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

      cmbs_int_ie_ParameterAdd( (void*)p_List, &st_Param );

      return cmbs_int_EventSend( CMBS_EV_DSR_PARAM_GET, p_List->pu8_Buffer, p_List->u16_CurSize );
   }

   return CMBS_RC_ERROR_PARAMETER;
}

E_CMBS_RC         cmbs_dsr_param_area_Set( void * pv_AppRefHandle , E_CMBS_MEM_TYPE e_MemType, u32 u32_Location, u8* pu8_Data, u16 u16_Size )
{
	PST_CFR_IE_LIST
                  p_List;
	ST_IE_PARAMETER
                  st_Param;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   if ( e_MemType == CMBS_MEM_EEPROM && u16_Size <= 128)
   {
      // Add parameter to IE list
      st_Param.e_Param     = CMBS_PARAM_FLEX;
      st_Param.u16_Position=(u16)u32_Location;
      st_Param.pu8_Data    = pu8_Data;
      st_Param.u16_DataLen = u16_Size;

      p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

      cmbs_int_ie_ParameterAdd( (void*)p_List, &st_Param );

      // Send Parameter Set request
      return cmbs_int_EventSend( CMBS_EV_DSR_PARAM_SET, p_List->pu8_Buffer, p_List->u16_CurSize );
   }

   return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_dsr_param_Set( void * pv_AppRefHandle, E_CMBS_PARAM e_Param, u8 * pu8_Data, u16 u16_DataLen )
{
	PST_CFR_IE_LIST
                  p_List;
	ST_IE_PARAMETER
                  st_Param;
   u8             u8_Ret;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   // Validate parameter settings
   if( (u8_Ret = cmbs_int_ParamValid(e_Param, pu8_Data, u16_DataLen)) != CMBS_RC_OK )
   {
      return u8_Ret;
   }

   // Add parameter to IE list
   st_Param.e_Param     = e_Param;
   st_Param.pu8_Data    = pu8_Data;
   st_Param.u16_DataLen = u16_DataLen;

   p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   cmbs_int_ie_ParameterAdd( (void*)p_List, &st_Param );

   // Send Parameter Set request
   return cmbs_int_EventSend( CMBS_EV_DSR_PARAM_SET, p_List->pu8_Buffer, p_List->u16_CurSize );
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

   return cmbs_int_EventSend( CMBS_EV_DSR_HS_PAGE, p_List->pu8_Buffer, p_List->u16_CurSize );
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


E_CMBS_RC         cmbs_dsr_LogBufferRead( void * pv_AppRefHandle, E_CMBS_MODULE e_FwModule )
{
	PST_CFR_IE_LIST
                  p_List;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   return cmbs_int_EventSend( CMBS_EV_DSR_SYS_LOG_REQ, p_List->pu8_Buffer, p_List->u16_CurSize );
}


//*/
