/*!
*	\file			cmbs_dem.c
*	\brief			
*	\Author		stein 
*
*	@(#)	%filespec: cmbs_dem.c-1 %
*
*******************************************************************************
*	\par	History
*	\n==== History =============================================================\n
*	date			name		version	 action                                       \n
*	----------------------------------------------------------------------------\n
*  25-Feb-09   stein    70        Initial revision(just skeleton)              \n

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
 * DEM (external media) functions
 *****************************************************************************/

E_CMBS_RC         cmbs_dem_ChannelStart( void * pv_AppRefHandle, void * pv_RefIEList )
{
   PST_CFR_IE_LIST
                  p_List = (PST_CFR_IE_LIST)pv_RefIEList;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   return cmbs_int_EventSend( CMBS_EV_DEM_CHANNEL_START, p_List->pu8_Buffer, p_List->u16_CurSize );  
}


E_CMBS_RC         cmbs_dem_ChannelStop( void * pv_AppRefHandle, void * pv_RefIEList )
{
   PST_CFR_IE_LIST
                  p_List = (PST_CFR_IE_LIST)pv_RefIEList;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   return cmbs_int_EventSend( CMBS_EV_DEM_CHANNEL_STOP, p_List->pu8_Buffer, p_List->u16_CurSize );  
}


E_CMBS_RC         cmbs_dem_ToneStart( void *pv_AppRefHandle, void * pv_RefIEList )
{
   PST_CFR_IE_LIST
                  p_List = (PST_CFR_IE_LIST)pv_RefIEList;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   return cmbs_int_EventSend( CMBS_EV_DEM_TONE_START, p_List->pu8_Buffer, p_List->u16_CurSize );  
}


E_CMBS_RC         cmbs_dem_ToneStop( void *pv_AppRefHandle, void * pv_RefIEList )
{
   PST_CFR_IE_LIST
                  p_List = (PST_CFR_IE_LIST)pv_RefIEList;

   if( pv_AppRefHandle ){
   }  // unused. suppress compiler warnings

   return cmbs_int_EventSend( CMBS_EV_DEM_TONE_STOP, p_List->pu8_Buffer, p_List->u16_CurSize );  
}

//*/
