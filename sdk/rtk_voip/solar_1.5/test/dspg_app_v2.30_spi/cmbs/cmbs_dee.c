/*!
*	\file			cmbs_dee.c
*	\brief			
*	\Author		stein 
*
*	@(#)	%filespec: cmbs_dee.c-1 %
*
*******************************************************************************
*	\par	History
*	\n==== History =============================================================\n
*	date			name		version	 action                                       \n
*	----------------------------------------------------------------------------\n
*  25-Feb-09   stein    69        Initial revision(just skeleton)              \n

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

//*/
