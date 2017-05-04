/*!
*	\file			cmbs_dee.c
*	\brief			
*	\Author		stein 
*
*	@(#)	%filespec: cmbs_dee.c~DMZD53#5 %
*
*******************************************************************************
*	\par	History
*	\n==== History =============================================================\n
*	date			name		version	 action                                       \n
*	----------------------------------------------------------------------------\n
*  25-Feb-09   stein    69        Initial revision(just skeleton)              \n

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
 * DEE (external endpoint) functions
 *****************************************************************************/

u32               cmbs_dee_CallInstanceNew( void * pv_AppRefHandle )
{
   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   return ++g_CMBSInstance.u32_CallInstanceCount;
}


E_CMBS_RC         cmbs_dee_CallEstablish( void * pv_AppRefHandle, void * pv_RefIEList )
{
   PST_CFR_IE_LIST
                  p_List = (PST_CFR_IE_LIST)pv_RefIEList;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   return cmbs_int_EventSend( CMBS_EV_DEE_CALL_ESTABLISH, p_List->pu8_Buffer, p_List->u16_CurSize );  
}


E_CMBS_RC         cmbs_dee_CallProgress( void * pv_AppRefHandle, void * pv_RefIEList )
{
   PST_CFR_IE_LIST
                  p_List = (PST_CFR_IE_LIST)pv_RefIEList;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   return cmbs_int_EventSend( CMBS_EV_DEE_CALL_PROGRESS, p_List->pu8_Buffer, p_List->u16_CurSize );  
}


E_CMBS_RC         cmbs_dee_CallAnswer( void * pv_AppRefHandle, void * pv_RefIEList )
{
   PST_CFR_IE_LIST
                  p_List = (PST_CFR_IE_LIST)pv_RefIEList;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   return cmbs_int_EventSend( CMBS_EV_DEE_CALL_ANSWER, p_List->pu8_Buffer, p_List->u16_CurSize );  
}


E_CMBS_RC         cmbs_dee_CallRelease( void * pv_AppRefHandle, void * pv_RefIEList )
{
   PST_CFR_IE_LIST
                  p_List = (PST_CFR_IE_LIST)pv_RefIEList;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   return cmbs_int_EventSend( CMBS_EV_DEE_CALL_RELEASE, p_List->pu8_Buffer, p_List->u16_CurSize );  
}


E_CMBS_RC         cmbs_dee_CallReleaseComplete( void * pv_AppRefHandle, u32 u32_CallInstance )
{
   PST_CFR_IE_LIST
                  p_List = cmbs_api_ie_GetList();

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   cmbs_api_ie_CallInstanceAdd( p_List, u32_CallInstance );

   return cmbs_int_EventSend( CMBS_EV_DEE_CALL_RELEASECOMPLETE, p_List->pu8_Buffer, p_List->u16_CurSize );  
}


E_CMBS_RC         cmbs_dee_CallInbandInfo(void * pv_AppRefHandle, void * pv_RefIEList )
{
   PST_CFR_IE_LIST
                  p_List = (PST_CFR_IE_LIST)pv_RefIEList;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   return cmbs_int_EventSend( CMBS_EV_DEE_CALL_INBANDINFO, p_List->pu8_Buffer, p_List->u16_CurSize );  
}


E_CMBS_RC         cmbs_dee_CallMediaOffer( void * pv_AppRefHandle, void * pv_RefIEList )
{
   PST_CFR_IE_LIST
                  p_List = (PST_CFR_IE_LIST)pv_RefIEList;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   return cmbs_int_EventSend( CMBS_EV_DEE_CALL_MEDIA_OFFER, p_List->pu8_Buffer, p_List->u16_CurSize );  
}


E_CMBS_RC         cmbs_dee_CallMediaOfferRes( void * pv_AppRefHandle, void * pv_RefIEList )
{
   PST_CFR_IE_LIST
                  p_List = (PST_CFR_IE_LIST)pv_RefIEList;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   return cmbs_int_EventSend( CMBS_EV_DEE_CALL_MEDIA_OFFER_RES, p_List->pu8_Buffer, p_List->u16_CurSize );  
}


E_CMBS_RC         cmbs_dee_CallHold( void * pv_AppRefHandle, void * pv_RefIEList )
{
   PST_CFR_IE_LIST
                  p_List = (PST_CFR_IE_LIST)pv_RefIEList;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   return cmbs_int_EventSend( CMBS_EV_DEE_CALL_HOLD, p_List->pu8_Buffer, p_List->u16_CurSize );  
}


E_CMBS_RC         cmbs_dee_CallResume( void * pv_AppRefHandle, void * pv_RefIEList )
{
   PST_CFR_IE_LIST
                  p_List = (PST_CFR_IE_LIST)pv_RefIEList;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   return cmbs_int_EventSend( CMBS_EV_DEE_CALL_RESUME, p_List->pu8_Buffer, p_List->u16_CurSize );  
}

E_CMBS_RC         cmbs_dee_CallHoldRes( void * pv_AppRefHandle, u32 u32_CallInstance, u16 u16_HsNum, E_CMBS_RESPONSE e_Response )
{
   ST_IE_RESPONSE   st_Response;
   PST_CFR_IE_LIST  pv_RefIEList;

   pv_RefIEList = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();
   st_Response.e_Response = e_Response;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   if( pv_RefIEList )
   {
       // Add call Instance IE
      cmbs_api_ie_CallInstanceAdd( (void*)pv_RefIEList, u32_CallInstance );
      cmbs_int_ie_ResponseAdd( (void*)pv_RefIEList, &st_Response );
      cmbs_api_ie_HandsetsAdd( (void*)pv_RefIEList, u16_HsNum);
   }

   return cmbs_int_EventSend( CMBS_EV_DEE_CALL_HOLD_RES, pv_RefIEList->pu8_Buffer, pv_RefIEList->u16_CurSize );
}


E_CMBS_RC         cmbs_dee_CallResumeRes( void * pv_AppRefHandle, u32 u32_CallInstance, u16 u16_HsNum, E_CMBS_RESPONSE e_Response )
{
   ST_IE_RESPONSE   st_Response;
   PST_CFR_IE_LIST  pv_RefIEList;

   pv_RefIEList = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();
   st_Response.e_Response = e_Response;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   if( pv_RefIEList )
   {
       // Add call Instance IE
      cmbs_api_ie_CallInstanceAdd( (void*)pv_RefIEList, u32_CallInstance );
      cmbs_int_ie_ResponseAdd( (void*)pv_RefIEList, &st_Response );
      cmbs_api_ie_HandsetsAdd( (void*)pv_RefIEList, u16_HsNum);
   }


   return cmbs_int_EventSend( CMBS_EV_DEE_CALL_RESUME_RES, pv_RefIEList->pu8_Buffer, pv_RefIEList->u16_CurSize );
}

void cmbs_dee_HandsetLinkRelease(u8 Handset)
{
   PST_CFR_IE_LIST  pv_RefIEList;
   pv_RefIEList = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   if( pv_RefIEList )
   {
   	cmbs_api_ie_HsNumberAdd((void*)pv_RefIEList, Handset);
	cmbs_int_EventSend( CMBS_EV_DEE_HANDSET_LINK_RELEASE , pv_RefIEList->pu8_Buffer, pv_RefIEList->u16_CurSize );
   }

}


//*/
