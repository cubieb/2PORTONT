/*!
*	\file			cmbs_ie.c
*	\brief			
*	\Author		stein 
*
*	@(#)	%filespec: cmbs_ie.c-1 %
*
*******************************************************************************
*	\par	History
*	\n==== History =============================================================\n
*	date			name		version	 action                                       \n
*	----------------------------------------------------------------------------\n
*  25-Feb-09   stein    61        Initial revision                             \n
*  25-Feb-09   stein    62        CallInstance, CallerID, CalledID - Add/Get   \n
*  25-Feb-09   stein    64        CallerName, CallInfo, ReleaseReason, MediaChannel \n
*  25-Feb-09   stein    65        MediaDesc, Time, FwVersion, SysLog \n

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
 * Information Elements List functions
 *****************************************************************************/

void *            cmbs_api_ie_GetList( void )
{
   /*! on linux side a dynamic memory behavior is possible */
	return cfr_ie_ListNew( NULL, 0);
}


E_CMBS_RC         cmbs_api_ie_FreeList( void * pv_RefIEList )
{
   if( pv_RefIEList )
   {
      CFR_DBG_OUT( "cmbs_api_ie_FreeList: Dynamic is not implemented\n" );
   }
   
   return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_api_ie_GetFirst( void * pv_RefIEList, void ** ppv_RefIE, u16 * pu16_IEType )
{
	u8 * pu8_Tmp;
  
   pu8_Tmp = cfr_ie_ItemFirstGet( (PST_CFR_IE_LIST)pv_RefIEList );
   *ppv_RefIE = (void *)pu8_Tmp;
   
   *pu16_IEType = (u16)*pu8_Tmp;
   
   return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_api_ie_GetNext( void * pv_RefIEList, void ** ppv_RefIE, u16 * pu16_IEType ) 
{
	u8 * pu8_Tmp = NULL;
  
   pu8_Tmp = cfr_ie_ItemNextGet( (PST_CFR_IE_LIST)pv_RefIEList );

   if( pu8_Tmp )
   {
      *ppv_RefIE = (void *)pu8_Tmp;
   
      *pu16_IEType = (u16)*pu8_Tmp;
   }   
   else
   {
      *ppv_RefIE = NULL;
   
      *pu16_IEType = 0;
   }
      
   return CMBS_RC_OK;
}


/*****************************************************************************
 * Information Elements ADD / GET functions
 *****************************************************************************/

E_CMBS_RC         cmbs_api_ie_CallInstanceAdd( void * pv_RefIEList, u32 u32_CallInstance )
{
   u8  u8_Buffer[8];

   memset( u8_Buffer, 0, sizeof(u8_Buffer) );
   
   u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_CALLINSTANCE;
   u8_Buffer[CFR_IE_SIZE_POS] = sizeof(u32);


   cfr_ie_ser_u32( u8_Buffer + CFR_IE_HEADER_SIZE, u32_CallInstance );

   cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE );

   return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_api_ie_CallInstanceGet( void * pv_RefIE, u32 * pu32_CallInstance )
{
   u8 * pu8_Buffer = (u8*)pv_RefIE;
   
   if( pu8_Buffer[CFR_IE_TYPE_POS] == CMBS_IE_CALLINSTANCE )
   {
      cfr_ie_dser_u32( pu8_Buffer + CFR_IE_HEADER_SIZE, pu32_CallInstance );
   
   	return CMBS_RC_OK;
   }
   printf("[THLin]:cmbs_api_ie_CallInstanceGet get error param\n");
   return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_api_ie_CallerPartyAdd( void * pv_RefIEList, ST_IE_CALLERPARTY * pst_CallerParty )
{
   u8 u8_Buffer[32];

   memset( u8_Buffer, 0, sizeof(u8_Buffer) );
   
   u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_CALLERPARTY;
   u8_Buffer[CFR_IE_SIZE_POS] = 3 * sizeof(u8) + pst_CallerParty->u8_AddressLen;

   u8_Buffer[2] = pst_CallerParty->u8_AddressProperties;
   u8_Buffer[3] = pst_CallerParty->u8_AddressPresentation;
   u8_Buffer[4] = pst_CallerParty->u8_AddressLen;

   cfr_ie_ser_pu8( u8_Buffer + CFR_IE_HEADER_SIZE + 3 * sizeof(u8),
                    pst_CallerParty->pu8_Address,
                    pst_CallerParty->u8_AddressLen );
   
   cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE );   

   return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_api_ie_CallerPartyGet( void * pv_RefIE, ST_IE_CALLERPARTY * pst_CallerParty )
{
   u8 * pu8_Buffer = (u8*)pv_RefIE;
   
   if( *pu8_Buffer == CMBS_IE_CALLERPARTY )
   {
      pst_CallerParty->u8_AddressProperties   = pu8_Buffer[CFR_IE_HEADER_SIZE];     
      pst_CallerParty->u8_AddressPresentation = pu8_Buffer[CFR_IE_HEADER_SIZE + sizeof(u8)];     
      pst_CallerParty->u8_AddressLen          = pu8_Buffer[CFR_IE_HEADER_SIZE + 2 * sizeof(u8)];
      pst_CallerParty->pu8_Address            = pu8_Buffer + CFR_IE_HEADER_SIZE + 3 * sizeof(u8);     

   	return CMBS_RC_OK;
   }
   
   return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_api_ie_CalledPartyAdd( void * pv_RefIEList, ST_IE_CALLEDPARTY * pst_CalledParty )
{
   u8 u8_Buffer[32];

   memset( u8_Buffer, 0, sizeof(u8_Buffer) );
   
   u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_CALLEDPARTY;
   u8_Buffer[CFR_IE_SIZE_POS] = 3 * sizeof(u8) + pst_CalledParty->u8_AddressLen;

   u8_Buffer[2] = pst_CalledParty->u8_AddressProperties;
   u8_Buffer[3] = pst_CalledParty->u8_AddressPresentation;
   u8_Buffer[4] = pst_CalledParty->u8_AddressLen;

   cfr_ie_ser_pu8( u8_Buffer + CFR_IE_HEADER_SIZE + 3 * sizeof(u8),
                    pst_CalledParty->pu8_Address,
                    pst_CalledParty->u8_AddressLen );
   
   cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE );   

   return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_api_ie_CalledPartyGet( void * pv_RefIE, ST_IE_CALLEDPARTY * pst_CalledParty )
{
   u8 * pu8_Buffer = (u8*)pv_RefIE;
   
   if( *pu8_Buffer == CMBS_IE_CALLEDPARTY )
   {
      pst_CalledParty->u8_AddressProperties   = pu8_Buffer[CFR_IE_HEADER_SIZE];     
      pst_CalledParty->u8_AddressPresentation = pu8_Buffer[CFR_IE_HEADER_SIZE + sizeof(u8)];     
      pst_CalledParty->u8_AddressLen          = pu8_Buffer[CFR_IE_HEADER_SIZE + 2 * sizeof(u8)];
      pst_CalledParty->pu8_Address            = pu8_Buffer + CFR_IE_HEADER_SIZE + 3 * sizeof(u8);     

   	return CMBS_RC_OK;
   }
   
   return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_api_ie_CallerNameAdd( void * pv_RefIEList, ST_IE_CALLERNAME * pst_CallerName )
{
   u8 u8_Buffer[32];

   memset( u8_Buffer, 0, sizeof(u8_Buffer) );
   
   u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_CALLERNAME;
   u8_Buffer[CFR_IE_SIZE_POS] = sizeof(u8) + pst_CallerName->u8_DataLen;

   u8_Buffer[CFR_IE_HEADER_SIZE] = pst_CallerName->u8_DataLen;
   cfr_ie_ser_pu8( u8_Buffer + CFR_IE_HEADER_SIZE + sizeof(u8), pst_CallerName->pu8_Name, pst_CallerName->u8_DataLen );
   
   cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE );   

   return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_api_ie_CallerNameGet( void * pv_RefIE, ST_IE_CALLERNAME * pst_CallerName )
{
   u8 * pu8_Buffer = (u8*)pv_RefIE;
   
   if( *pu8_Buffer == CMBS_IE_CALLERNAME )
   {
      pst_CallerName->u8_DataLen = pu8_Buffer[CFR_IE_HEADER_SIZE];     
      pst_CallerName->pu8_Name   = pu8_Buffer + CFR_IE_HEADER_SIZE + sizeof(u8);     

   	return CMBS_RC_OK;
   }
   
   return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_api_ie_CallProgressAdd( void * pv_RefIEList, ST_IE_CALLPROGRESS * pst_CallProgress )
{
   u8 u8_Buffer[4];

   memset( u8_Buffer, 0, sizeof(u8_Buffer) );
   
   u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_CALLPROGRESS;
   u8_Buffer[CFR_IE_SIZE_POS] = sizeof(u8);

   u8_Buffer[CFR_IE_HEADER_SIZE] = (u8)pst_CallProgress->e_Progress;
   
   cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE );   

   return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_api_ie_CallProgressGet( void * pv_RefIE, ST_IE_CALLPROGRESS * pst_CallProgress )
{
   u8 * pu8_Buffer = (u8*)pv_RefIE;
   
   if( pu8_Buffer[CFR_IE_TYPE_POS] == CMBS_IE_CALLPROGRESS )
   {
      pst_CallProgress->e_Progress = pu8_Buffer[CFR_IE_HEADER_SIZE];     

   	return CMBS_RC_OK;
   }
   
   return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_api_ie_CallInfoAdd( void * pv_RefIEList, ST_IE_CALLINFO * pst_CallInfo )
{
   u8 u8_Buffer[32];

   memset( u8_Buffer, 0, sizeof(u8_Buffer) );
   
   u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_CALLINFO;
   u8_Buffer[CFR_IE_SIZE_POS] = 2 * sizeof(u8) + pst_CallInfo->u8_DataLen;

   u8_Buffer[CFR_IE_HEADER_SIZE] = (u8)pst_CallInfo->e_Type;
   u8_Buffer[CFR_IE_HEADER_SIZE + sizeof(u8)] = pst_CallInfo->u8_DataLen;
      
   cfr_ie_ser_pu8( u8_Buffer + CFR_IE_HEADER_SIZE + 2 * sizeof(u8),
                   pst_CallInfo->pu8_Info,
                   pst_CallInfo->u8_DataLen );
   
   cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE );   

   return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_api_ie_CallInfoGet( void * pv_RefIE, ST_IE_CALLINFO * pst_CallInfo )
{
   u8 * pu8_Buffer = (u8*)pv_RefIE;
   
   if( pu8_Buffer[CFR_IE_TYPE_POS] == CMBS_IE_CALLINFO )
   {
      pst_CallInfo->e_Type = pu8_Buffer[CFR_IE_HEADER_SIZE];     

      pst_CallInfo->u8_DataLen = pu8_Buffer[CFR_IE_HEADER_SIZE + sizeof(u8)];
      pst_CallInfo->pu8_Info = pu8_Buffer + CFR_IE_HEADER_SIZE + 2 * sizeof(u8);

   	return CMBS_RC_OK;
   }
   
   return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_api_ie_DisplayStringAdd( void * pv_RefIEList, ST_IE_DISPLAY_STRING * pst_DisplayString )
{
   u8 u8_Buffer[36];

   memset( u8_Buffer, 0, sizeof(u8_Buffer) );
   
   u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_CALLERNAME;
   u8_Buffer[CFR_IE_SIZE_POS] = sizeof(u8) + pst_DisplayString->u8_DataLen;

   u8_Buffer[CFR_IE_HEADER_SIZE] = pst_DisplayString->u8_DataLen;
   cfr_ie_ser_pu8( u8_Buffer + CFR_IE_HEADER_SIZE + sizeof(u8),
   					 pst_DisplayString->pu8_Info, MAX(pst_DisplayString->u8_DataLen, CMBS_DISPLAY_STRING_LENGTH) );
   
   cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE );   

   return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_api_ie_CallReleaseReasonAdd( void * pv_RefIEList, ST_IE_RELEASE_REASON * pst_RelReason )
{
   u8 u8_Buffer[8];

   memset( u8_Buffer, 0, sizeof(u8_Buffer) );
   
   u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_CALLRELEASE_REASON;
   u8_Buffer[CFR_IE_SIZE_POS] = sizeof(u8) + sizeof(u32);

   u8_Buffer[CFR_IE_HEADER_SIZE] = pst_RelReason->e_Reason;
   cfr_ie_ser_u32( u8_Buffer + CFR_IE_HEADER_SIZE + sizeof(u8), pst_RelReason->u32_ExtReason );
   
   cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE );   

   return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_api_ie_CallReleaseReasonGet( void * pv_RefIE, ST_IE_RELEASE_REASON * pst_RelReason )
{
   u8 * pu8_Buffer = (u8*)pv_RefIE;
   
   if( pu8_Buffer[CFR_IE_TYPE_POS] == CMBS_IE_CALLRELEASE_REASON )
   {
      pst_RelReason->e_Reason = pu8_Buffer[CFR_IE_HEADER_SIZE];     
      cfr_ie_dser_u32( pu8_Buffer + CFR_IE_HEADER_SIZE + sizeof(u8), &pst_RelReason->u32_ExtReason );

   	return CMBS_RC_OK;
   }
   
   return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_api_ie_MediaChannelAdd( void * pv_RefIEList, ST_IE_MEDIA_CHANNEL * pst_MediaChannel )
{
   u8 u8_Buffer[12];

   memset( u8_Buffer, 0, sizeof(u8_Buffer) );
   
   u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_MEDIACHANNEL;
   u8_Buffer[CFR_IE_SIZE_POS] = 2* sizeof(u32) + sizeof(u8);

   cfr_ie_ser_u32( u8_Buffer + CFR_IE_HEADER_SIZE, pst_MediaChannel->u32_ChannelID );
   cfr_ie_ser_u32( u8_Buffer + CFR_IE_HEADER_SIZE +sizeof(u32), pst_MediaChannel->u32_ChannelParameter );
   u8_Buffer[CFR_IE_HEADER_SIZE + (2*sizeof(u32))] = pst_MediaChannel->e_Type;
   
   cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE );   

   return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_api_ie_MediaChannelGet( void * pv_RefIE, ST_IE_MEDIA_CHANNEL * pst_MediaChannel )
{
   u8 * pu8_Buffer = (u8*)pv_RefIE;
   
   if( pu8_Buffer[CFR_IE_TYPE_POS] == CMBS_IE_MEDIACHANNEL )
   {
      cfr_ie_dser_u32( pu8_Buffer + CFR_IE_HEADER_SIZE, &pst_MediaChannel->u32_ChannelID );
      cfr_ie_dser_u32( pu8_Buffer + CFR_IE_HEADER_SIZE + sizeof(u32), &pst_MediaChannel->u32_ChannelParameter );
      pst_MediaChannel->e_Type = pu8_Buffer[CFR_IE_HEADER_SIZE + (2*sizeof(u32))];     

   	return CMBS_RC_OK;
   }
   
   return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_api_ie_MediaDescAdd( void * pv_RefIEList, ST_IE_MEDIA_DESCRIPTOR * pst_MediaDesc )
{
   u8 u8_Buffer[4];

   memset( u8_Buffer, 0, sizeof(u8_Buffer) );
   
   u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_MEDIADESCRIPTOR;
   u8_Buffer[CFR_IE_SIZE_POS] = sizeof(u8);

   u8_Buffer[CFR_IE_HEADER_SIZE] = (u8)pst_MediaDesc->e_Codec;
   
   cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE );   

   return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_api_ie_MediaDescGet( void * pv_RefIE, ST_IE_MEDIA_DESCRIPTOR * pst_MediaDesc )
{
   u8 * pu8_Buffer = (u8*)pv_RefIE;
   
   if( pu8_Buffer[CFR_IE_TYPE_POS] == CMBS_IE_MEDIADESCRIPTOR )
   {
      pst_MediaDesc->e_Codec = pu8_Buffer[CFR_IE_HEADER_SIZE];     

   	return CMBS_RC_OK;
   }
   
   return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_api_ie_ToneAdd( void * pv_RefIEList, ST_IE_TONE * pst_Tone )
{
   u8 u8_Buffer[4];

   memset( u8_Buffer, 0, sizeof(u8_Buffer) );
   
   u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_TONE;
   u8_Buffer[CFR_IE_SIZE_POS] = sizeof(u8);

   u8_Buffer[CFR_IE_HEADER_SIZE] = (u8)pst_Tone->e_Tone;
   
   cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE );   

   return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_api_ie_TimeAdd( void * pv_RefIEList, ST_IE_TIMEOFDAY * pst_TimeOfDay )
{
   u8 u8_Buffer[8];

   memset( u8_Buffer, 0, sizeof(u8_Buffer) );
   
   u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_TIMEOFDAY;
   u8_Buffer[CFR_IE_SIZE_POS] = sizeof(u32);

   cfr_ie_ser_u32( u8_Buffer + CFR_IE_HEADER_SIZE, pst_TimeOfDay->u32_Timestamp );
   
   cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE );   

   return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_api_ie_TimeGet( void * pv_RefIE, ST_IE_TIMEOFDAY * pst_TimeOfDay )
{
   u8 * pu8_Buffer = (u8*)pv_RefIE;
   
   if( pu8_Buffer[CFR_IE_TYPE_POS] == CMBS_IE_TIMEOFDAY )
   {
      cfr_ie_dser_u32( pu8_Buffer + CFR_IE_HEADER_SIZE, &pst_TimeOfDay->u32_Timestamp );

   	return CMBS_RC_OK;
   }
   
   return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_api_ie_HandsetInfoGet( void * pv_RefIE, ST_IE_HANDSETINFO * pst_HandsetInfo )
{
   u8 * pu8_Buffer = (u8*)pv_RefIE;
   
   if( *pu8_Buffer == CMBS_IE_HANDSETINFO )
   {
      pst_HandsetInfo->u8_Hs    = pu8_Buffer[CFR_IE_HEADER_SIZE];     
      pst_HandsetInfo->u8_State = pu8_Buffer[CFR_IE_HEADER_SIZE + sizeof(u8)];     

      memcpy( pst_HandsetInfo->u8_IPEI, pu8_Buffer + CFR_IE_HEADER_SIZE + 7, sizeof(pst_HandsetInfo->u8_IPEI) );

   	return CMBS_RC_OK;
   }
   
   return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_api_ie_ParameterGet( void * pv_RefIE, ST_IE_PARAMETER * pst_Parameter )
{
   u8 * pu8_Buffer = (u8*)pv_RefIE;
   
   if ( *pu8_Buffer == CMBS_IE_PARAMETER )
   {
   	u16 i= 3; // start of data length
      
   	pst_Parameter->e_Param    = pu8_Buffer[CFR_IE_HEADER_SIZE];

      i += cfr_ie_dser_u16( pu8_Buffer + i, &pst_Parameter->u16_Position );
      i += cfr_ie_dser_u16( pu8_Buffer + i, &pst_Parameter->u16_DataLen );
      if ( pst_Parameter->u16_DataLen )
      {
      	pst_Parameter->pu8_Data = pu8_Buffer + i; 
      }
      else
      {
      	pst_Parameter->pu8_Data = NULL;
      }
   	return CMBS_RC_OK;
   }
   
   return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_api_ie_FwVersionGet( void * pv_RefIE, ST_IE_FW_VERSION * pst_FwVersion )
{
   u8 * pu8_Buffer = (u8*)pv_RefIE;
   
   if( *pu8_Buffer == CMBS_IE_FW_VERSION )
   {
      pst_FwVersion->e_SwModule = pu8_Buffer[CFR_IE_HEADER_SIZE];
      cfr_ie_dser_u16( pu8_Buffer + CFR_IE_HEADER_SIZE + sizeof(u8), &pst_FwVersion->u16_FwVersion );     

   	return CMBS_RC_OK;
   }
   
   return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_api_ie_SysLogGet( void * pv_RefIE, ST_IE_SYS_LOG * pst_SysLog )
{
   u8 * pu8_Buffer = (u8*)pv_RefIE;
   
   if( *pu8_Buffer == CMBS_IE_SYS_LOG )
   {
      cfr_ie_dser_u16( pu8_Buffer + CFR_IE_HEADER_SIZE, &pst_SysLog->u16_DataLen );     
      pst_SysLog->pu8_Data = pu8_Buffer + CFR_IE_HEADER_SIZE + sizeof(u16);

   	return CMBS_RC_OK;
   }
   
   return CMBS_RC_ERROR_PARAMETER;
}

E_CMBS_RC         cmbs_api_ie_SysStatusGet( void * pv_RefIE, ST_IE_SYS_STATUS * pst_SysStatus )
{
   u8 * pu8_Buffer = (u8*)pv_RefIE;
   
   if( pu8_Buffer[CFR_IE_TYPE_POS] == CMBS_IE_STATUS )
   {
      pst_SysStatus->e_ModuleStatus = pu8_Buffer[CFR_IE_HEADER_SIZE];     

   	return CMBS_RC_OK;
   }
   
   return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_api_ie_ResponseGet( void * pv_RefIE, ST_IE_RESPONSE * pst_Response )
{
	u8 * pu8_Buffer = (u8*)pv_RefIE;
  
  if ( *pu8_Buffer == CMBS_IE_RESPONSE )
  {
	 	pst_Response->e_Response  = pu8_Buffer[CFR_IE_HEADER_SIZE];
    
   	return CMBS_RC_OK;
  }
  
  return CMBS_RC_ERROR_PARAMETER;
}


//*/
