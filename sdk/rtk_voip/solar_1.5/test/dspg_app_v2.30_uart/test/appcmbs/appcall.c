/*!
*	\file		appcall.c
*	\brief	    handle the call control and linked it to media
*	\Author		kelbch
*
*  appcall Automat
*  Incoming call: 1. start as normal incoming call
*                 2. start automatically media, if target sent CMBS answer message
*
*  Outgoing call: 1. Receive establish
*                 2. Reply with set-up ack
*                 3. Start dial tone
*                 4. After receiving '#' send ringing
*                 5. User interact for active call, with Media on
*
*	@(#)	%filespec: -1 %
*
*******************************************************************************
*	\par	History
*	\n==== History ============================================================\n
*	date			name		version	 action                                          \n
*	----------------------------------------------------------------------------\n
* 19-sep-09		Kelbch		pj1029-478	add demonstration line handling \n
*******************************************************************************
*	COPYRIGHT DOSCH & AMAND RESEARCH GMBH & CO.KG
*	DOSCH & AMAND RESEARCH GMBH & CO.KG Confidential
*
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if ! defined ( WIN32 )
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/time.h> //we need <sys/select.h>; should be included in <sys/types.h> ???
#include <signal.h>
#else
#include "windows.h"
#endif

#include "cmbs_api.h"
#include "cmbs_str.h"
#include "cfr_ie.h"
#include "cfr_mssg.h"
#include "appcmbs.h"
#include "appcall.h"

extern void  keyb_ReleaseNotify( void );

                                 /*! \brief global call object */
E_APPCALL_AUTOMAT_MODE  g_call_automat = E_APPCALL_AUTOMAT_MODE_ON;//E_APPCALL_AUTOMAT_MODE_OFF;

ST_CALL_OBJ    g_call_obj[APPCALL_CALLOBJ_MAX];

//		========== _appcall_CallObjStateString ===========
/*!
		\brief				return the string of enumeration
		\param[in,out]		e_Call		 enumeration value
		\return				<char *>     string value
*/
char *               _appcall_CallObjStateString( E_APPCMBS_CALL e_Call )
{
   switch ( e_Call )
   {
      caseretstr(E_APPCMBS_CALL_CLOSE);

      caseretstr(E_APPCMBS_CALL_INC_PEND);
      caseretstr(E_APPCMBS_CALL_INC_RING);

      caseretstr(E_APPCMBS_CALL_OUT_PEND);
      caseretstr(E_APPCMBS_CALL_OUT_PEND_DIAL);
      caseretstr(E_APPCMBS_CALL_OUT_INBAND);
      caseretstr(E_APPCMBS_CALL_OUT_PROC);
      caseretstr(E_APPCMBS_CALL_OUT_RING);

      caseretstr(E_APPCMBS_CALL_ACTIVE);
      caseretstr(E_APPCMBS_CALL_RELEASE);

      default:
         return (char*)"State undefined";
  }

  return NULL;
}

//		========== _appcall_CallOBJMediaCodecString ===========
/*!
		\brief				return the string of enumeration
		\param[in,out]		E_CMBS_AUDIO_CODEC
                              		 enumeration value
		\return				<char *>     string value
*/
char *               _appcall_CallOBJMediaCodecString( E_CMBS_AUDIO_CODEC e_Codec )
{
   switch ( e_Codec )
   {
      caseretstr(CMBS_AUDIO_CODEC_UNDEF);
      caseretstr(CMBS_AUDIO_CODEC_PCMU);
      caseretstr(CMBS_AUDIO_CODEC_PCMA);
      caseretstr(CMBS_AUDIO_CODEC_PCMU_WB);
      caseretstr(CMBS_AUDIO_CODEC_PCMA_WB);
      caseretstr(CMBS_AUDIO_CODEC_PCM_LINEAR_WB);
      caseretstr(CMBS_AUDIO_CODEC_PCM_LINEAR_NB);

      default:
         return (char*) "Codec undefined";
   }

  return NULL;
}

//		========== _appcall_CallOBJMediaString ===========
/*!
		\brief				return the string of enumeration
		\param[in,out]		E_APPCMBS_MEDIA
                              		 enumeration value
		\return				<char *>     string value
*/
char *               _appcall_CallOBJMediaString( E_APPCMBS_MEDIA e_Media )
{
   switch ( e_Media )
   {
      caseretstr(E_APPCMBS_MEDIA_CLOSE);
      caseretstr(E_APPCMBS_MEDIA_PEND);
      caseretstr(E_APPCMBS_MEDIA_ACTIVE);

      default:
         return (char*)"Media State undefined";
   }

   return NULL;
}

//		========== 	appcall_InfoCall ===========
/*!
		\brief	 		 print the current call information 

		\param[in]		 n_Call		Identifier of call

		\return	 		 <none>

*/

void			appcall_InfoCall( int n_Call )
{
	if ( g_call_obj[n_Call].u32_CallInstance )
	{
	   APPCMBS_INFO((  "APP_Dongle-CALL: INFO CALL %d\n",n_Call ));

	   if ( g_call_obj[n_Call].ch_CallerID )
	      APPCMBS_INFO((  "APP_Dongle-CALL: INFO CLI %s\n", g_call_obj[n_Call].ch_CallerID ));
	   if ( g_call_obj[n_Call].ch_CalledID )
	      APPCMBS_INFO((  "APP_Dongle-CALL: INFO CLD %s\n", g_call_obj[n_Call].ch_CalledID ));

	   APPCMBS_INFO((  "APP_Dongle-CALL: INFO Call State  %s\n",_appcall_CallObjStateString(g_call_obj[n_Call].e_Call) ));
	   APPCMBS_INFO((  "APP_Dongle-CALL: INFO Media State %s\n",_appcall_CallOBJMediaString(g_call_obj[n_Call].e_Media) ));
	   APPCMBS_INFO((  "APP_Dongle-CALL: INFO\n" ));
	   APPCMBS_INFO((  "APP_Dongle-CALL: INFO CallInstance %u =%lx\n",g_call_obj[n_Call].u32_CallInstance, (long unsigned int )g_call_obj[n_Call].u32_CallInstance));
	   APPCMBS_INFO((  "APP_Dongle-CALL: INFO Codec %s\n", _appcall_CallOBJMediaCodecString( g_call_obj[n_Call].e_Codec )));
	}
	else
	{
	   APPCMBS_INFO((  "APPCMBS-CALL: INFO Line %d free\n",n_Call ));
	}
}

//		========== appcall_InfoPrint ===========
/*
		\brief				print the current call objects information
		\param[in,out]		<none>
		\return				<none>

*/
void          appcall_InfoPrint(void)
{
   int   i;

   for(i=0; i < APPCALL_CALLOBJ_MAX; i++ )
   {

      APPCMBS_INFO(( "APP_Dongle-CALL: INFO ----------------------\n" ));
      if (g_call_automat)
      {
         APPCMBS_INFO(( "APP_Dongle-CALL: --AUTOMAT ON ---------------\n" ));
      }
      else
      {
         APPCMBS_INFO(( "APP_Dongle-CALL: --AUTOMAT OFF ---------------\n" ));
      }

      if ( g_call_obj[i].u32_CallInstance )
      {
         APPCMBS_INFO((  "APP_Dongle-CALL: INFO Line %d\n",i ));

         if ( g_call_obj[i].ch_CallerID )
            APPCMBS_INFO((  "APP_Dongle-CALL: INFO CLI %s\n", g_call_obj[i].ch_CallerID ));
         if ( g_call_obj[i].ch_CalledID )
            APPCMBS_INFO((  "APP_Dongle-CALL: INFO CLD %s\n", g_call_obj[i].ch_CalledID ));

         APPCMBS_INFO((  "APP_Dongle-CALL: INFO Call State  %s\n",_appcall_CallObjStateString(g_call_obj[i].e_Call) ));
         APPCMBS_INFO((  "APP_Dongle-CALL: INFO Media State %s\n",_appcall_CallOBJMediaString(g_call_obj[i].e_Media) ));
         APPCMBS_INFO((  "APP_Dongle-CALL: INFO\n" ));
         APPCMBS_INFO(( "APP_Dongle-CALL: INFO CallInstance %u =%lx\n",g_call_obj[i].u32_CallInstance, (long unsigned int )g_call_obj[i].u32_CallInstance));
         APPCMBS_INFO(( "APP_Dongle-CALL: INFO Codec %s\n", _appcall_CallOBJMediaCodecString( g_call_obj[i].e_Codec )));
      }
      else
      {
         APPCMBS_INFO((  "APP_Dongle-CALL: INFO Line %d free\n",i ));
      }
   }
}
//		========== appcall_Initialize  ===========
/*!
		\brief				Initialize the call/line management
		\param[in,out]		<none>
		\return				<none>
*/
void          appcall_Initialize (void)
{
   int i;

   memset( &g_call_obj, 0, sizeof(g_call_obj));

   for (i=0; i< APPCALL_CALLOBJ_MAX; i++ )
   {
    g_call_obj[i].st_CalledParty.pu8_Address=(u8*)g_call_obj[i].ch_CalledID;
    g_call_obj[i].st_CallerParty.pu8_Address=(u8*)g_call_obj[i].ch_CallerID;
   }
}
//		========== _appcall_CallOBJNew ===========
/*!
		\brief				get a free call object
		\param[in,out]		<none>
		\return				<PST_CALL_OBJ>    if no free call object available return NULL
*/

PST_CALL_OBJ _appcall_CallOBJNew( void )
{
   int i;
   
   //printf("[THLin]: _appcall_CallOBJNew, g_call_obj = %p\n", g_call_obj);
   
   for (i=0; i< APPCALL_CALLOBJ_MAX; i++ )
   {
      if ( !g_call_obj[i].u32_CallInstance )
      {
      	 //printf("[THLin]: _appcall_CallOBJNew, g_call_obj+%d = %p\n", i, g_call_obj+i);
      	 //printf("[THLin]: _appcall_CallOBJNew, g_call_obj+1 = %p\n", g_call_obj+1);
      	 
         return g_call_obj + i;
      }
   }

   return NULL;
}

//		========== _appcall_CallOBJGet ===========
/*!
		\brief				return a call object identfied by call instance or caller party
		\param[in,out]		u32_CallInstance
		\param[in,out]		psz_CLI
		\return				<PST_CALL_OBJ>    if no free call object available return NULL
*/
PST_CALL_OBJ   _appcall_CallOBJGet( u32 u32_CallInstance, char * psz_CLI )
{
   int i;

   for (i=0; i< APPCALL_CALLOBJ_MAX; i++ )
   {
      if ( u32_CallInstance && (g_call_obj[i].u32_CallInstance == u32_CallInstance) )
         return g_call_obj +i;

      if ( psz_CLI && !strcmp(g_call_obj[i].ch_CallerID, psz_CLI) )
         return g_call_obj +i;
   }

   return NULL;
}

//THLin Add
PST_CALL_OBJ   _appcall_CallOBJGetByLine( int LineID )
{
	if (LineID > APPCALL_CALLOBJ_MAX)
		return NULL;
	else
		return g_call_obj +LineID;
}

//		========== _appcall_CallOBJDelete ===========
/*!
		\brief				delete a call object identfied by call instance or caller party
		\param[in,out]		u32_CallInstance
		\param[in,out]		psz_CLI
		\return				<none>
*/
void                 _appcall_CallOBJDelete( u32 u32_CallInstance, char * psz_CLI )
{
   PST_CALL_OBJ pst_This;

   pst_This = _appcall_CallOBJGet( u32_CallInstance, psz_CLI );

   if( pst_This )
   {
      memset ( pst_This, 0, sizeof(ST_CALL_OBJ)) ;
      pst_This->st_CalledParty.pu8_Address=(u8*)pst_This->ch_CalledID;
      pst_This->st_CallerParty.pu8_Address=(u8*)pst_This->ch_CallerID;
   }
}
//		========== _appcall_CallObjLineGet ===========
/*!
		\brief			return the line ID of call object

		\param[in]		pst_Call		pointer to call object

		\return			<int>          line ID

*/

int _appcall_CallObjLineGet(PST_CALL_OBJ   pst_Call)
{
   //printf("[THLin]: _appcall_CallObjLineGet, %p, %p, pst_Call -g_call_obj = %d\n", pst_Call, g_call_obj, pst_Call -g_call_obj);
   //printf("[THLin]: _appcall_CallObjLineGet, %p, %p, pst_Call -g_call_obj = %d\n", pst_Call, g_call_obj, (unsigned long)pst_Call -(unsigned long)g_call_obj);
   return ( pst_Call -g_call_obj);
}
//		========== _appcall_CallOBJStateSet  ===========
/*!
		\brief			set call object line/call state
		\param[in]		pst_Call		pointer to call object
		\param[in]		e_State		line/call state
		\return			<none>
*/
void _appcall_CallOBJStateSet ( PST_CALL_OBJ pst_Call, E_APPCMBS_CALL e_State )
{
   pst_Call->e_Call = e_State;
   printf("[THLin]: _appcall_CallOBJStateSet, state: %s\n", _appcall_CallObjStateString(e_State));
}

//		========== _appcall_CallOBJStateGet  ===========
/*!
		\brief			set call object line/call state
		\param[in]		pst_Call		 pointer to call object
		\return			<E_APPCMBS_CALL>
                                  line/call state of this call object
*/
E_APPCMBS_CALL       _appcall_CallOBJStateGet( PST_CALL_OBJ pst_Call )
{
   return pst_Call->e_Call;
}

//		========== _appcall_CallOBJMediaSet  ===========
/*!
		\brief			set call object line/call state
		\param[in]		pst_Call		pointer to call object
		\param[in]		e_State		media state
		\return			<none>
*/
void                 _appcall_CallOBJMediaSet ( PST_CALL_OBJ pst_Call, E_APPCMBS_MEDIA e_State )
{
   pst_Call->e_Media = e_State;
}

//		========== _appcall_CallOBJMediaGet  ===========
/*!
		\brief			set call object line/call state
		\param[in]		pst_Call		 pointer to call object
		\return			<E_APPCMBS_MEDIA>
                                  line/call state of this call object
*/
E_APPCMBS_MEDIA       _appcall_CallOBJMediaGet( PST_CALL_OBJ pst_Call )
{
   return pst_Call->e_Media;
}


int _appcall_CallObjDigitCollectorEndSymbolCheck( PST_CALL_OBJ pst_Call )
{
   int   i;

   for ( i=0; i < pst_Call->st_CalledParty.u8_AddressLen; i ++ )
   {
      if ( pst_Call->st_CalledParty.pu8_Address[i] == '#' )
      {
         return TRUE;
      }
   }

   return FALSE;

}
//		========== _appcall_PropertiesIDXGet ===========
/*!
		\brief			 find index of IE in exchange object
		\param[in]		 pst_Properties	 pointer to exchange object
		\param[in]		 n_Properties		 number of contained IEs
		\param[in]		 e_IE		          to be find IE

		\return	 		 <int>              index of the IE

*/

int   _appcall_PropertiesIDXGet( PST_APPCALL_PROPERTIES pst_Properties, int n_Properties, E_CMBS_IE_TYPE e_IE )
{
   int   i;

   for ( i=0; i < n_Properties; i++ )
   {
      if ( pst_Properties[i].e_IE == e_IE )
         break;
   }

   return i;
}
//		========== _appmedia_CallOBJMediaPropertySet  ===========
/*!
		\brief			short description
		\param[in]		pst_Call		 pointer to call object
		\param[in]		u16_IE		 current IE
		\param[in,out]	pst_IEInfo	 pointer to IE info object
		\return			<int>        return TRUE, if IEs were consumed
*/

int         _appmedia_CallOBJMediaPropertySet ( PST_CALL_OBJ pst_Call, u16 u16_IE, PST_APPCMBS_IEINFO pst_IEInfo )
{
   switch ( u16_IE )
   {
      case CMBS_IE_MEDIADESCRIPTOR:
         pst_Call->e_Codec = pst_IEInfo->Info.st_MediaDesc.e_Codec;
        return TRUE;

      case CMBS_IE_MEDIACHANNEL:
         pst_Call->u32_ChannelID = pst_IEInfo->Info.st_MediaChannel.u32_ChannelID;
         _appcall_CallOBJMediaSet( pst_Call, E_APPCMBS_MEDIA_PEND );
         if(pst_Call->u32_ChannelID != pst_Call->u8_LineId)
		printf("[THLin]: _appmedia_CallOBJMediaPropertySet, pst_Call->u32_ChannelID != pst_Call->u8_LineId (%d != %d)\n", pst_Call->u32_ChannelID, pst_Call->u8_LineId);
        return TRUE;
   }

   return FALSE;
}
//		========== appmedia_CallOBJMediaStop ===========
/*!
		\brief				stop the media channel identified by call instance, line or caller ID
		\param[in,out]		u32_CallInstance	if not used zero
		\param[in,out]		n_Line		      Line ID used, if psz_CLI is NULL
		\param[in,out]		psz_CLI		      pointer to caller ID,if not needed NULL
		\return				<none>
*/
void        appmedia_CallOBJMediaStop( u32 u32_CallInstance, int n_Line, char * psz_CLI )
{
   PST_CALL_OBJ   pst_This;
   void *         pv_RefIEList = NULL;

   if ( u32_CallInstance )
   {
      pst_This = _appcall_CallOBJGet( u32_CallInstance, NULL );
   }
   else
   {
      if ( ! psz_CLI )
      {
         // line ID is available
         pst_This = g_call_obj + n_Line;
      }
      else
      {
         pst_This = _appcall_CallOBJGet( 0, psz_CLI );
      }
   }

   if ( pst_This && (E_APPCMBS_MEDIA_ACTIVE) == _appcall_CallOBJMediaGet(pst_This))
   {
         pv_RefIEList = cmbs_api_ie_GetList();

         if( pv_RefIEList )
         {
            ST_IE_MEDIA_CHANNEL  st_MediaChannel;

            memset( &st_MediaChannel, 0, sizeof(ST_IE_MEDIA_CHANNEL) );
            st_MediaChannel.e_Type        = CMBS_MEDIA_TYPE_AUDIO_IOM;
            st_MediaChannel.u32_ChannelID = pst_This->u32_ChannelID;

            if(pst_This->u32_ChannelID != pst_This->u8_LineId)
		printf("[THLin]: appmedia_CallOBJMediaStop, pst_This->u32_ChannelID != pst_This->u8_LineId (%d != %d)\n", pst_This->u32_ChannelID, pst_This->u8_LineId);

            cmbs_api_ie_MediaChannelAdd( pv_RefIEList, &st_MediaChannel );
            cmbs_dem_ChannelStop( g_cmbsappl.pv_CMBSRef, pv_RefIEList );
            _appcall_CallOBJMediaSet( pst_This, E_APPCMBS_MEDIA_PEND );
         }
   }
}

//		========== appmedia_CallOBJMediaStart ===========
/*!
		\brief				start the media channel identified by call instance, line or caller ID
		\param[in,out]		u32_CallInstance	if not used zero
		\param[in,out]		n_Line		      Line ID used, if psz_CLI is NULL
		\param[in,out]		psz_CLI		      pointer to caller ID,if not needed NULL
		\return				<none>
*/
void        appmedia_CallOBJMediaStart( u32 u32_CallInstance, int n_Line, char * psz_CLI )
{
   PST_CALL_OBJ   pst_This;
   void *         pv_RefIEList = NULL;

   if ( u32_CallInstance )
   {
      pst_This = _appcall_CallOBJGet( u32_CallInstance, NULL );
   }
   else
   {
      if ( ! psz_CLI )
      {
         // line ID is available
         pst_This = g_call_obj + n_Line;
      }
      else
      {
         pst_This = _appcall_CallOBJGet( 0, psz_CLI );
      }
   }

   if ( pst_This && (E_APPCMBS_MEDIA_PEND) == _appcall_CallOBJMediaGet(pst_This))
   {
         pv_RefIEList = cmbs_api_ie_GetList();

         if( pv_RefIEList )
         {
            ST_IE_MEDIA_CHANNEL  st_MediaChannel;

            memset( &st_MediaChannel, 0, sizeof(ST_IE_MEDIA_CHANNEL) );
            st_MediaChannel.e_Type               = CMBS_MEDIA_TYPE_AUDIO_IOM;
            st_MediaChannel.u32_ChannelID        = pst_This->u32_ChannelID;
		
	if(pst_This->u32_ChannelID != pst_This->u8_LineId)
		printf("[THLin]: appmedia_CallOBJMediaStart, pst_This->u32_ChannelID != pst_This->u8_LineId (%d != %d)\n", pst_This->u32_ChannelID, pst_This->u8_LineId);

            // keep IOM scheme 0-3 Line 0 4567 LINE 2 891011 LINE 3
            if ( pst_This->e_Codec == CMBS_AUDIO_CODEC_PCM_LINEAR_WB )
            {
               st_MediaChannel.u32_ChannelParameter = 0xF << (pst_This->u32_ChannelID *4 );
               // [DK] see comment below, only opposite! ;))
            }
            else if( pst_This->e_Codec == CMBS_AUDIO_CODEC_PCM_LINEAR_NB )
            {
               st_MediaChannel.u32_ChannelParameter = 0x3 << (pst_This->u32_ChannelID * 4 );
               // [DK] the IOM channel is independent of the Air interface.
               // IOM could be WB and Air interface could be NB!
            }
            else if( pst_This->e_Codec == CMBS_AUDIO_CODEC_PCMA ||
                     pst_This->e_Codec == CMBS_AUDIO_CODEC_PCMU )
            {
               st_MediaChannel.u32_ChannelParameter = 0x1 << (pst_This->u32_ChannelID * 4 );
            }
            else  // currently no CMBS_AUDIO_CODEC_PCMA_WB/CMBS_AUDIO_CODEC_PCMU_WB
            {
                 st_MediaChannel.u32_ChannelParameter = 0x3 << (pst_This->u32_ChannelID *4 );
            }

            cmbs_api_ie_MediaChannelAdd( pv_RefIEList, &st_MediaChannel );
            cmbs_dem_ChannelStart(g_cmbsappl.pv_CMBSRef, pv_RefIEList );
            _appcall_CallOBJMediaSet( pst_This, E_APPCMBS_MEDIA_ACTIVE );
         }

    }
}


void        appmedia_CallObjMediaInternalConnect( int channel, int context, int connect )
{
   ST_IE_MEDIA_INTERNAL_CONNECT st_MediaIC;
   void *         pv_RefIEList = NULL;

   pv_RefIEList = cmbs_api_ie_GetList();

   if( pv_RefIEList )
   {
      st_MediaIC.e_Type = connect;
      st_MediaIC.u32_ChannelID = channel;
      st_MediaIC.u32_NodeId = context;
      
      // Add call Instance IE
      cmbs_api_ie_MediaICAdd( pv_RefIEList, &st_MediaIC );

      cmbs_dem_ChannelInternalConnect( g_cmbsappl.pv_CMBSRef, pv_RefIEList );
   }
}
void        appmedia_CallObjMediaOffer( int n_Line, char ch_Audio )
{
   PST_CALL_OBJ   pst_This = NULL;
   ST_IE_MEDIA_DESCRIPTOR
                  st_MediaDesc;
   void *         pv_RefIEList = NULL;

   pst_This = g_call_obj + n_Line;

   if( pst_This )
   {
      pv_RefIEList = cmbs_api_ie_GetList();

      if( pv_RefIEList )
      {
         // Add call Instance IE
         cmbs_api_ie_CallInstanceAdd( pv_RefIEList, pst_This->u32_CallInstance );

         // add media descriptor IE
         memset( &st_MediaDesc, 0, sizeof(ST_IE_MEDIA_DESCRIPTOR) );

         if( ch_Audio == 'w' )
         {
            printf( "appmedia_CallObjMediaOffer WIDE\n" );
            st_MediaDesc.e_Codec = CMBS_AUDIO_CODEC_PCM_LINEAR_WB;
         }
         else if( ch_Audio == 'a' )
         {
            printf( "appmedia_CallObjMediaOffer ALAW\n" );
            st_MediaDesc.e_Codec = CMBS_AUDIO_CODEC_PCMA;
         }
         else if( ch_Audio == 'u' )
         {
            printf( "appmedia_CallObjMediaOffer ULAW\n" );
            st_MediaDesc.e_Codec = CMBS_AUDIO_CODEC_PCMU;
         }
         else
         {
            printf( "appmedia_CallObjMediaOffer NARROW\n" );
            st_MediaDesc.e_Codec = CMBS_AUDIO_CODEC_PCM_LINEAR_NB;
         }

         cmbs_api_ie_MediaDescAdd( pv_RefIEList, &st_MediaDesc );

         cmbs_dee_CallMediaOffer( g_cmbsappl.pv_CMBSRef, pv_RefIEList );
      }
   }
}

//		========== appmedia_CallOBJMediaStart ===========
/*!
		\brief				start the media channel identified by call instance, line or caller ID
      \param[in]        psz_Value         pointer to CMBS tone enumeration string
      \param[in]        bo_On             TRUE to play, FALSE to stop
		\param[in,out]		n_Line		      Line ID used, if psz_CLI is NULL
		\param[in,out]		psz_CLI		      pointer to caller ID,if not needed NULL
		\return				<none>
*/
void        appmedia_CallOBJTonePlay( char * psz_Value, int bo_On, int n_Line, char * psz_CLI )
{
   PST_CALL_OBJ   pst_This;

   if ( ! psz_CLI )
   {
      // line ID is available
      pst_This = g_call_obj + n_Line;
   }
   else
   {
      pst_This = _appcall_CallOBJGet( 0, psz_CLI );
   }

   if ( pst_This )
   {
      void *         pv_RefIEList = NULL;

      if ( _appcall_CallOBJMediaGet(pst_This) == E_APPCMBS_MEDIA_PEND ||
           _appcall_CallOBJMediaGet(pst_This) == E_APPCMBS_MEDIA_ACTIVE )
      {

         pv_RefIEList = cmbs_api_ie_GetList();

         if( pv_RefIEList )
         {
            ST_IE_MEDIA_CHANNEL  st_MediaChannel;

            memset( &st_MediaChannel, 0, sizeof(ST_IE_MEDIA_CHANNEL) );
            st_MediaChannel.e_Type        = CMBS_MEDIA_TYPE_AUDIO_IOM;
            st_MediaChannel.u32_ChannelID = pst_This->u32_ChannelID;

            if(pst_This->u32_ChannelID != pst_This->u8_LineId)
		printf("[THLin]: appmedia_CallOBJTonePlay, pst_This->u32_ChannelID != pst_This->u8_LineId (%d != %d)\n", pst_This->u32_ChannelID, pst_This->u8_LineId);


            cmbs_api_ie_MediaChannelAdd( pv_RefIEList, &st_MediaChannel );

            if ( !bo_On )
            {
               cmbs_dem_ToneStop( g_cmbsappl.pv_CMBSRef, pv_RefIEList );
            }
            else
            {
               ST_IE_TONE  st_Tone;

               memset( &st_Tone, 0, sizeof(ST_IE_TONE) );
               st_Tone.e_Tone = getidx_E_CMBS_TONE(psz_Value);
               cmbs_api_ie_ToneAdd( pv_RefIEList, &st_Tone );

               cmbs_dem_ToneStart( g_cmbsappl.pv_CMBSRef, pv_RefIEList );
            }
         }
      }
   }
}

void        _appDTMFPlay ( PST_CALL_OBJ pst_Call, u8* pu8_Tone, u8 u8_Len )
{
   int i, e;
   char  ch_Tone[30];
                                          // Tone info send
   for (i=0; i < u8_Len; i++ )
   {
      if ( pu8_Tone[i] == '*' )
      {
          e= CMBS_TONE_DTMF_STAR;
      }
      else if ( pu8_Tone[i] == '#' )
      {
          e= CMBS_TONE_DTMF_HASH;

      }
      else if ( pu8_Tone[i] >= 0x30 && pu8_Tone[i] <= 0x39 )
      {
         e = (pu8_Tone[i] -0x30 ) + CMBS_TONE_DTMF_0;
      }
      else
      {
         e = (pu8_Tone[i] -'a' ) + CMBS_TONE_DTMF_A;
      }
      strcpy( ch_Tone, getstr_E_CMBS_TONE( e ) );

      appmedia_CallOBJTonePlay( ch_Tone, TRUE, _appcall_CallObjLineGet(pst_Call) , NULL );

#if defined( WIN32 )
      Sleep( 200 );        // wait 0.2 seconds
#else
      usleep( 1000 * 200 );
#endif
   }

}
/***************************************************************************
*
*     Old function for tracking
*
****************************************************************************/
void        app_PrintCallProgrInfo( E_CMBS_CALL_PROGRESS e_Progress )
{
   switch( e_Progress )
   {
      case  CMBS_CALL_PROGR_PROCEEDING:  APPCMBS_INFO(( "PROCEEDING"));     break;
      case  CMBS_CALL_PROGR_RINGING:     APPCMBS_INFO(( "RINGING"));        break;
      case  CMBS_CALL_PROGR_BUSY:        APPCMBS_INFO(( "BUSY"));           break;
      case  CMBS_CALL_PROGR_INBAND:      APPCMBS_INFO(( "INBAND"));         break;
      default:                           APPCMBS_INFO(( "UNKNOWN"));
   }
}

void        app_PrintCallInfoType( E_CMBS_CALL_INFO_TYPE e_Type )
{
   switch( e_Type )
   {
      case CMBS_CALL_INFO_TYPE_DISPLAY:   APPCMBS_INFO(( "DISPLAY"));break;
      case CMBS_CALL_INFO_TYPE_DIGIT:     APPCMBS_INFO(( "DIGITS"));break;
      default:                            APPCMBS_INFO(( "UNKNOWN"));
   }
}

//		========== app_OnCallEstablish ===========
/*!
		\brief	 		CMBS Target wants to make an outgoing call
		\param[in]		pvAppRefHandle   application reference pointer
		\param[in]		pv_List		      IE list pointer
		\return	 		<none>
*/
void        app_OnCallEstablish( void * pvAppRefHandle, void * pv_List )
{
   PST_CALL_OBJ      pst_Call = NULL;	//THLin add to init to NULL
   ST_APPCMBS_IEINFO st_IEInfo;

   void *         pv_IE = NULL;
   u16            u16_IE;
                                 // ensure not to have compiler warning
   if (pvAppRefHandle){};

   if( pv_List )
   {
      // collect information elements. we expect:
      // CMBS_IE_CALLINSTANCE, CMBS_IE_CALLERPARTY, CMBS_IE_CALLEDPARTY and CMBS_IE_MEDIADESCRIPTOR

      cmbs_api_ie_GetFirst( pv_List, &pv_IE, &u16_IE );
      while( pv_IE != NULL )
      {
         app_IEToString( pv_IE, u16_IE );

         appcmbs_IEInfoGet ( pv_IE, u16_IE, &st_IEInfo );

         switch ( u16_IE )
         {
            case CMBS_IE_CALLINSTANCE:
            printf("[THLin]: app_OnCallEstablish, get CMBS_IE_CALLINSTANCE.\n");
               pst_Call = _appcall_CallOBJNew();
               if ( pst_Call )
               {
                  pst_Call->u32_CallInstance = st_IEInfo.Info.u32_CallInstance;

               }
               else
               {
                  // normally not needed, need to be checked for call collision
               }
            break;

            case CMBS_IE_LINE_ID:
            printf("[THLin]: app_OnCallEstablish, get CMBS_IE_LINE_ID, LineId = %d\n", st_IEInfo.Info.u8_LineId);
               if( pst_Call )
               {
                  pst_Call->u8_LineId = st_IEInfo.Info.u8_LineId;
               }
            break;
            
            case CMBS_IE_CALLERPARTY:
            printf("[THLin]: app_OnCallEstablish, get CMBS_IE_CALLERPARTY.\n");
            printf("CALLERID address properties:0x%02x\n", st_IEInfo.Info.st_CallerParty.u8_AddressProperties);
            if ( pst_Call )
            {
               int i;

               printf ( "CallerID length:%d:", st_IEInfo.Info.st_CallerParty.u8_AddressLen );

               for (i=0; i < st_IEInfo.Info.st_CallerParty.u8_AddressLen; i ++ )
                  printf (" 0x%02x", st_IEInfo.Info.st_CallerParty.pu8_Address[i] );

               printf( "\n" );

               memcpy ( pst_Call->ch_CallerID, st_IEInfo.Info.st_CallerParty.pu8_Address, st_IEInfo.Info.st_CallerParty.u8_AddressLen );
               memcpy ( &pst_Call->st_CallerParty, &st_IEInfo.Info.st_CallerParty, sizeof(pst_Call->st_CallerParty) );
               pst_Call->st_CallerParty.pu8_Address = (u8*)pst_Call->ch_CallerID;
            }
            break;

            case CMBS_IE_CALLEDPARTY:
            printf("[THLin]: app_OnCallEstablish, get CMBS_IE_CALLEDPARTY.\n");
            if ( pst_Call )
            {
               int i;

               printf ( "CalledID length:%d:", st_IEInfo.Info.st_CalledParty.u8_AddressLen );

               for (i=0; i < st_IEInfo.Info.st_CalledParty.u8_AddressLen; i ++ )
                  printf (" 0x%02x", st_IEInfo.Info.st_CalledParty.pu8_Address[i] );
               
               printf( "\n" );
               
               memcpy ( &pst_Call->ch_CalledID, &st_IEInfo.Info.st_CalledParty.pu8_Address, st_IEInfo.Info.st_CalledParty.u8_AddressLen );
               memcpy ( &pst_Call->st_CalledParty, &st_IEInfo.Info.st_CalledParty, sizeof(pst_Call->st_CalledParty) );
               pst_Call->st_CalledParty.pu8_Address = (u8*)pst_Call->ch_CalledID;
            }
            break;
         }

         _appmedia_CallOBJMediaPropertySet ( pst_Call, u16_IE, &st_IEInfo );

         cmbs_api_ie_GetNext( pv_List, &pv_IE, &u16_IE );
      }

      if ( pst_Call )
      {
         _appcall_CallOBJStateSet( pst_Call, E_APPCMBS_CALL_OUT_PEND );
      }

	/*************************************************
      	 [THLin]:add off-hook event of HS to event FIFO (Outgoing call , HS off-hook)
		  	pst_Call can extract line ID info */
	//dect_event_in(pst_Call->u8_LineId, 1/*off-hook*/);
	/*************************************************/

      if( g_cmbsappl.n_Token )
      {
         appcmbs_ObjectSignal( NULL , 0, _appcall_CallObjLineGet(pst_Call), CMBS_EV_DEE_CALL_ESTABLISH );
      }

      if( g_call_automat )
      {
         // reply early media connect and start dial tone.

#ifndef MY_TEST_MAIN
         ST_APPCALL_PROPERTIES   st_Properties;
         int   n_Line;

         n_Line = _appcall_CallObjLineGet(pst_Call);

         if ( !pst_Call->st_CalledParty.u8_AddressLen )
         {
            st_Properties.e_IE      = CMBS_IE_CALLPROGRESS;
            st_Properties.psz_Value = "CMBS_CALL_PROGR_SETUP_ACK\0";


            appcall_ProgressCall ( &st_Properties, 1, n_Line, NULL );
         }
         else
         {
            st_Properties.e_IE      = CMBS_IE_CALLPROGRESS;
            st_Properties.psz_Value = "CMBS_CALL_PROGR_INBAND\0";


            appcall_ProgressCall ( &st_Properties, 1, n_Line, NULL );
         }
#else
	//CMBS_Api_OutboundCallProgress(pst_Call->u8_LineId, (char*)"CMBS_CALL_PROGR_INBAND\0");
	CMBS_Api_OutboundCall(pst_Call->u8_LineId);
#endif
      }
   }
}

//		========== app_OnCallProgress ===========
/*!
		\brief	 		CMBS Target signal call progres
		\param[in]		pvAppRefHandle   application reference pointer
		\param[in]		pv_List		      IE list pointer
		\return	 		<none>
*/
void        app_OnCallProgress( void * pvAppRefHandle, void * pv_List )
{
   PST_CALL_OBJ      pst_Call = NULL;
   ST_APPCMBS_IEINFO st_IEInfo;

   void *         pv_IE;
   u16            u16_IE;
   u8             u8_i;

   u32                   u32_CallInstance;
   E_CMBS_CALL_PROGRESS  e_CallProgress;
   ST_IE_CALLEDPARTY     st_CalledParty;
   
   memset(&st_CalledParty, 0, sizeof(st_CalledParty));
                                 
   if (pvAppRefHandle){};   // ensure not to have compiler warning

   if( pv_List )
   {
      // collect information elements. we expect:
      // CMBS_IE_CALLINSTANCE and CMBS_IE_CALLPROGRESS

      cmbs_api_ie_GetFirst( pv_List, &pv_IE, &u16_IE );
      while( pv_IE != NULL )
      {
         app_IEToString( pv_IE, u16_IE );

         appcmbs_IEInfoGet ( pv_IE, u16_IE, &st_IEInfo );

         switch (u16_IE)
         {
            case CMBS_IE_CALLINSTANCE:
               u32_CallInstance = st_IEInfo.Info.u32_CallInstance;
               
            // Todo: get call resources for this CallInstance...
               pst_Call = _appcall_CallOBJGet(st_IEInfo.Info.u32_CallInstance, NULL);
               break;

            case CMBS_IE_CALLPROGRESS:
               e_CallProgress = st_IEInfo.Info.st_CallProgress.e_Progress;
               
               if ( pst_Call )
               {
                  switch ( st_IEInfo.Info.st_CallProgress.e_Progress )
                  {
                     case  CMBS_CALL_PROGR_PROCEEDING:
 //                       _appcall_CallOBJStateSet (pst_Call, E_APPCMBS_CALL_OUT_PROC );
                       break;
                     case  CMBS_CALL_PROGR_RINGING:
                        _appcall_CallOBJStateSet (pst_Call, E_APPCMBS_CALL_INC_RING );
                        break;
                     case  CMBS_CALL_PROGR_BUSY:
                        break;
                     case  CMBS_CALL_PROGR_INBAND:
                        break;
                     default:
                        break;
                  }
               }
               break;

            case CMBS_IE_CALLEDPARTY:
               st_CalledParty = st_IEInfo.Info.st_CalledParty;
               
               break;
          }
                  // check for media information
          _appmedia_CallOBJMediaPropertySet ( pst_Call, u16_IE, &st_IEInfo );

         cmbs_api_ie_GetNext( pv_List, &pv_IE, &u16_IE );
      }

      //TODO: 
      printf("\napp_OnCallProgress: CallInstance=0x%08X, CallProgress=%s, CalledParty[ prop=%d, pres=%d, len=%d, addr=",
             u32_CallInstance, getstr_CMBS_CALL_PROGR(e_CallProgress), st_CalledParty.u8_AddressProperties,
             st_CalledParty.u8_AddressPresentation, st_CalledParty.u8_AddressLen);
      for (u8_i = 0; u8_i < st_CalledParty.u8_AddressLen; ++u8_i)
      {
         printf("%c", st_CalledParty.pu8_Address[u8_i]);
      }
      printf(" ]\n");


      if ( g_cmbsappl.n_Token )
      {
          appcmbs_ObjectSignal(  (void*)&st_IEInfo.Info.st_CallProgress ,
                                 sizeof(st_IEInfo.Info.st_CallProgress),
                                 _appcall_CallObjLineGet(pst_Call),
                                 CMBS_EV_DEE_CALL_PROGRESS );
      }


   }
   else
      printf( "app_OnCallEstablish ERROR: invalid IE list\n" );
}

//		========== app_OnCallInbandInfo ===========
/*!
		\brief	 		CMBS Target signal inband info, e.g. digits
		\param[in]		pvAppRefHandle   application reference pointer
		\param[in]		pv_List		      IE list pointer
		\return	 		<none>
*/
void        app_OnCallInbandInfo( void * pvAppRefHandle,void * pv_List )
{
   PST_CALL_OBJ      pst_Call = NULL;
   ST_APPCMBS_IEINFO st_IEInfo;

   void *         pv_IE;
   u16            u16_IE;
   int i;
                                 // ensure not to have compiler warning
   if (pvAppRefHandle){};

   if( pv_List )
   {
      // collect information elements. we expect:
      // CMBS_IE_CALLINSTANCE and CMBS_IE_CALLINFO

      cmbs_api_ie_GetFirst( pv_List, &pv_IE, &u16_IE );
      while( pv_IE != NULL )
      {
         app_IEToString( pv_IE, u16_IE );

         appcmbs_IEInfoGet ( pv_IE, u16_IE, &st_IEInfo );

         switch ( u16_IE )
         {
            case CMBS_IE_CALLINSTANCE:
            // Todo: get call resources for this CallInstance...
               pst_Call = _appcall_CallOBJGet(st_IEInfo.Info.u32_CallInstance, NULL);
               break;
           case CMBS_IE_CALLINFO :
                if ( pst_Call )
                {
                  // enter to called party entry digits;

                  if ( _appcall_CallOBJStateGet(pst_Call) == E_APPCMBS_CALL_OUT_PEND )
                  {
                     if ( _appcall_CallOBJMediaGet(pst_Call) == E_APPCMBS_MEDIA_PEND )
                     {
                        appmedia_CallOBJTonePlay( NULL, FALSE, _appcall_CallObjLineGet(pst_Call), NULL );
                     }

                     _appcall_CallOBJStateSet(pst_Call, E_APPCMBS_CALL_OUT_PEND_DIAL );
                  }

                  if ( _appcall_CallOBJStateGet(pst_Call) == E_APPCMBS_CALL_OUT_PEND_DIAL )
                  {
                     if ( (strlen((char*)pst_Call->st_CalledParty.pu8_Address)+ st_IEInfo.Info.st_CallInfo.u8_DataLen ) < sizeof(pst_Call->ch_CalledID))
                     {
                        memcpy( pst_Call->st_CalledParty.pu8_Address + strlen((char*)pst_Call->st_CalledParty.pu8_Address),
                                st_IEInfo.Info.st_CallInfo.pu8_Info,
                                st_IEInfo.Info.st_CallInfo.u8_DataLen );
                        pst_Call->st_CalledParty.u8_AddressLen = strlen((char*)pst_Call->st_CalledParty.pu8_Address);

                        _appDTMFPlay ( pst_Call,
                                       st_IEInfo.Info.st_CallInfo.pu8_Info,
                                       st_IEInfo.Info.st_CallInfo.u8_DataLen );

                        if ( g_call_automat )
                        {
                           if ( _appcall_CallObjDigitCollectorEndSymbolCheck( pst_Call ) )
                           {
                              appmedia_CallOBJTonePlay("CMBS_TONE_RING_BACK\0", TRUE, _appcall_CallObjLineGet(pst_Call), NULL );
                           }
                       }
                     }
                     else
                     {
                        printf ("Digits full\n" );
                     }
                  }

		  /*************************************************
		  /* [THLin]:add DTMF event of HS to event FIFO 
		  	pst_Call can extract line ID info */
                  //for (i=0; i < st_IEInfo.Info.st_CallInfo.u8_DataLen; i++)
                  	//dect_event_in(pst_Call->u8_LineId, st_IEInfo.Info.st_CallInfo.pu8_Info[i]);
                  /*************************************************/

                  if (_appcall_CallOBJStateGet(pst_Call) == E_APPCMBS_CALL_ACTIVE)
                  {
                        _appDTMFPlay ( pst_Call,
                                       st_IEInfo.Info.st_CallInfo.pu8_Info,
                                       st_IEInfo.Info.st_CallInfo.u8_DataLen );
                  }
                }
           break;

         }

          _appmedia_CallOBJMediaPropertySet ( pst_Call, u16_IE, &st_IEInfo );

         cmbs_api_ie_GetNext( pv_List, &pv_IE, &u16_IE );
      }
//!! todo
   }
}

//		========== app_OnCallAnswer ===========
/*!
		\brief	 		CMBS Target answer call
		\param[in]		pvAppRefHandle   application reference pointer
		\param[in]		pv_List		      IE list pointer
		\return	 		<none>
*/
void        app_OnCallAnswer( void * pvAppRefHandle,void * pv_List )
{
   PST_CALL_OBJ      pst_Call = NULL;
   ST_APPCMBS_IEINFO st_IEInfo;

   void *         pv_IE;
   u16            u16_IE;
                                 // ensure not to have compiler warning
   if (pvAppRefHandle){};

   if( pv_List )
   {
      // collect information elements. we expect:
      // CMBS_IE_CALLINSTANCE, CMBS_IE_MEDIACHANNEL and CMBS_IE_MEDIADESCRIPTOR
      // Todo: Search for call instance first

      cmbs_api_ie_GetFirst( pv_List, &pv_IE, &u16_IE );
      while( pv_IE != NULL )
      {
         app_IEToString( pv_IE, u16_IE );

         appcmbs_IEInfoGet( pv_IE, u16_IE, &st_IEInfo );

         switch( u16_IE )
         {
            case CMBS_IE_CALLINSTANCE:
            // Todo: get call resources for this CallInstance...
               pst_Call = _appcall_CallOBJGet(st_IEInfo.Info.u32_CallInstance, NULL);
               
               if( pst_Call == NULL )
               {
                  printf( "app_OnCallAnswer ERROR: CallObject = NULL!\n" );
                  return;
               }
               break;
         }

         if( pst_Call )
         {
            _appmedia_CallOBJMediaPropertySet( pst_Call, u16_IE, &st_IEInfo );
         }

         cmbs_api_ie_GetNext( pv_List, &pv_IE, &u16_IE );
      }

      if( pst_Call )
      {
         _appcall_CallOBJStateSet(pst_Call, E_APPCMBS_CALL_ACTIVE );

         if ( g_call_automat )
         {
            appmedia_CallOBJMediaStart( pst_Call->u32_CallInstance, 0, NULL );
         }
      }
      
      	/************************************************
         [THLin]:add off-hook event of HS to event FIFO (Incoming call , HS off-hook)
		  	pst_Call can extract line ID info */
	//dect_event_in(pst_Call->u8_LineId, 1/*off-hook*/);
	/*************************************************/
   }
}


//		========== app_OnCallRelease ===========
/*!
		\brief	 		CMBS Target release call
		\param[in]		pvAppRefHandle   application reference pointer
		\param[in]		pv_List		      IE list pointer
		\return	 		<none>
*/
void        app_OnCallRelease( void * pvAppRefHandle,void * pv_List )
{
   PST_CALL_OBJ      pst_Call = NULL;	//THLin add to init to NULL
   ST_APPCMBS_IEINFO st_IEInfo;
   u32 					u32_InstanceID = 0;
   void *   		pv_IE;
   u16      		u16_IE;
   ST_IE_RELEASE_REASON
                  st_Reason;
                                 // ensure not to have compiler warning
   if (pvAppRefHandle){};

   if( pv_List )
   {
      // collect information elements. we expect:
      // CMBS_IE_CALLINSTANCE and CMBS_IE_CALLRELEASE_REASON

      cmbs_api_ie_GetFirst( pv_List, &pv_IE, &u16_IE );

      while( pv_IE != NULL )
      {
         app_IEToString( pv_IE, u16_IE );
         appcmbs_IEInfoGet ( pv_IE, u16_IE, &st_IEInfo );

         if( u16_IE == CMBS_IE_CALLINSTANCE )
         {
            pst_Call = _appcall_CallOBJGet( st_IEInfo.Info.u32_CallInstance, NULL );
            u32_InstanceID = st_IEInfo.Info.u32_CallInstance;
         }

         if ( u16_IE == CMBS_IE_CALLRELEASE_REASON )
         {
            // we have to keep the structure for later messaging
            cmbs_api_ie_CallReleaseReasonGet( pv_IE, &st_Reason );
         }

         cmbs_api_ie_GetNext( pv_List, &pv_IE, &u16_IE );
      }

      if ( pst_Call )
      {
         if ( _appcall_CallOBJMediaGet( pst_Call ) == E_APPCMBS_MEDIA_ACTIVE )
         {
            // release Media stream
            /*!\todo release media stream */
            appmedia_CallOBJMediaStop( pst_Call->u32_CallInstance,0,NULL );
         }

         keyb_ReleaseNotify();
         
         // THLin add: to set call state to release 
         _appcall_CallOBJStateSet (pst_Call, E_APPCMBS_CALL_RELEASE );
         
         
         /*************************************************
           [THLin]:add on-hook event of HS to event FIFO.
		pst_Call can extract line ID info 	  */
	 //dect_event_in(pst_Call->u8_LineId, 0/*on-hook*/);
	 /*************************************************/
         
         if ( g_cmbsappl.n_Token )
         {
            appcmbs_ObjectSignal( (void*)&st_Reason , sizeof(st_Reason), _appcall_CallObjLineGet(pst_Call), CMBS_EV_DEE_CALL_RELEASE );
         }

         _appcall_CallOBJDelete( pst_Call->u32_CallInstance, NULL );
      }
      // send Release_Complete
      cmbs_dee_CallReleaseComplete( pvAppRefHandle, u32_InstanceID );
   }
}

void        app_OnCallHold( void * pvAppRefHandle,void * pv_List )
{
   u32            u32_CallInstance;
   void *   		pv_IE;
   u16      		u16_IE;

   if (pvAppRefHandle){};

   if( pv_List )
   {
      // collect information elements. we expect the CMBS_IE_CALLINSTANCE

      cmbs_api_ie_GetFirst( pv_List, &pv_IE, &u16_IE );
      if( u16_IE == CMBS_IE_CALLINSTANCE )
      {
         cmbs_api_ie_CallInstanceGet( pv_IE, &u32_CallInstance );
         printf( "CMBS_IE_CALLINSTANCE: %u\n", u32_CallInstance );
      }
   }
}

void        app_OnCallResume( void * pvAppRefHandle,void * pv_List )
{
   u32            u32_CallInstance;
   void *   		pv_IE;
   u16      		u16_IE;

   if (pvAppRefHandle){};

   if( pv_List )
   {
      // collect information elements. we expect the CMBS_IE_CALLINSTANCE

      cmbs_api_ie_GetFirst( pv_List, &pv_IE, &u16_IE );
      if( u16_IE == CMBS_IE_CALLINSTANCE )
      {
         cmbs_api_ie_CallInstanceGet( pv_IE, &u32_CallInstance );
         printf( "CMBS_IE_CALLINSTANCE: %u\n", u32_CallInstance );
      }
   }
}

void        app_OnCallState( void * pvAppRefHandle, void * pv_List )
{
   u32              u32_CallInstance = 0;
   void *           pv_IE;
   u16              u16_IE;
   ST_IE_CALL_STATE st_CallState;

   if (pvAppRefHandle){};

   if( pv_List )
   {
       cmbs_api_ie_GetFirst( pv_List, &pv_IE, &u16_IE );
       while( pv_IE )
       {
           switch ( u16_IE )
           {
               case CMBS_IE_CALLINSTANCE:
                   cmbs_api_ie_CallInstanceGet(pv_IE, &u32_CallInstance);
                   break;

               case CMBS_IE_CALLSTATE:
                   cmbs_api_ie_CallStateGet( pv_IE, &st_CallState );
                   break;
           }

       cmbs_api_ie_GetNext( pv_List, &pv_IE, &u16_IE );
       }
   }

   printf("\n- - - - - - - - - - -\nCall state:\n CallInstance = 0x%08X\n CallID = %d\n LineMask = 0x%02X\n HSMask = 0x%04X",
	   u32_CallInstance, st_CallState.u8_ActCallID, st_CallState.u8_LinesMask, st_CallState.u16_HandsetsMask);

   printf("\n CallType = ");
   switch (st_CallState.e_CallType)
   {
	case CMBS_CALL_STATE_TYPE_INTERNAL:
		printf("CMBS_CALL_STATE_TYPE_INTERNAL");
		break;
	case CMBS_CALL_STATE_TYPE_EXT_INCOMING:
		printf("CMBS_CALL_STATE_TYPE_EXT_INCOMING");
		break;
	case CMBS_CALL_STATE_TYPE_EXT_OUTGOING:
		printf("CMBS_CALL_STATE_TYPE_EXT_OUTGOING");
		break;
	case CMBS_CALL_STATE_TYPE_TRANSFER:
		printf("CMBS_CALL_STATE_TYPE_TRANSFER");
		break;
	case CMBS_CALL_STATE_TYPE_CONFERENCE:
		printf("CMBS_CALL_STATE_TYPE_CONFERENCE");
		break;
	case CMBS_CALL_STATE_TYPE_SERVICE:
		printf("CMBS_CALL_STATE_TYPE_SERVICE");
		break;
	default:
		printf("UNDEFINED");
		break;
   }

   printf("\n CallStatus = ");
   switch (st_CallState.e_CallStatus)
   {
   case CMBS_CALL_STATE_STATUS_IDLE:
	   printf("CMBS_CALL_STATE_STATUS_IDLE");
	   break;
   case CMBS_CALL_STATE_STATUS_CALL_SETUP:
	   printf("CMBS_CALL_STATE_STATUS_CALL_SETUP");
	   break;
   case CMBS_CALL_STATE_STATUS_CALL_SETUP_ACK:
	   printf("CMBS_CALL_STATE_STATUS_CALL_SETUP");
	   break;
   case CMBS_CALL_STATE_STATUS_CALL_PROCEEDING:
	   printf("CMBS_CALL_STATE_STATUS_CALL_PROCEEDING");
	   break;
   case CMBS_CALL_STATE_STATUS_CALL_ALERTING:
	   printf("CMBS_CALL_STATE_STATUS_CALL_ALERTING");
	   break;
   case CMBS_CALL_STATE_STATUS_CALL_CONNECTED:
	   printf("CMBS_CALL_STATE_STATUS_CALL_CONNECTED");
	   break;
   case CMBS_CALL_STATE_STATUS_CALL_DISCONNECTING:
	   printf("CMBS_CALL_STATE_STATUS_CALL_DISCONNECTING");
	   break;
   case CMBS_CALL_STATE_STATUS_CALL_HOLD:
	   printf("CMBS_CALL_STATE_STATUS_CALL_HOLD");
	   break;
   case CMBS_CALL_STATE_STATUS_CALL_UNDER_TRANSFER:
	   printf("CMBS_CALL_STATE_STATUS_CALL_UNDER_TRANSFER");
	   break;
   case CMBS_CALL_STATE_STATUS_CONF_CONNECTED:
	   printf("CMBS_CALL_STATE_STATUS_CONF_CONNECTED");
	   break;
   case CMBS_CALL_STATE_STATUS_CALL_INTERCEPTED:
	   printf("CMBS_CALL_STATE_STATUS_CALL_INTERCEPTED");
	   break;
   case CMBS_CALL_STATE_STATUS_CALL_WAITING:
	   printf("CMBS_CALL_STATE_STATUS_CALL_WAITING");
	   break;
   case CMBS_CALL_STATE_STATUS_CALL_REINJECTED:
	   printf("CMBS_CALL_STATE_STATUS_CALL_REINJECTED");
	   break;
   case CMBS_CALL_STATE_STATUS_IDLE_PENDING:
	   printf("CMBS_CALL_STATE_STATUS_IDLE_PENDING");
	   break;
   case CMBS_CALL_STATE_STATUS_CONF_SECONDARY:
	   printf("CMBS_CALL_STATE_STATUS_CONF_SECONDARY");
	   break;
   default:
	   printf("UNDEFINED");
	   break;
   }
   printf("\n");
}

//		========== app_OnCallRelease ===========
/*!
		\brief	 		CMBS Target announce media connectivity information
		\param[in]		pv_List		      IE list pointer
		\return	 		<none>
*/
void        app_OnCallMediaUpdate( void * pv_List )
{
   PST_CALL_OBJ      pst_Call = NULL;	//THLin add to init to NULL
   ST_APPCMBS_IEINFO st_IEInfo;

   void *   		pv_IE;
   u16      		u16_IE;

   printf( "Get Media Update:\n" );

   if( pv_List )
   {
      // collect information elements. we expect:
      // CMBS_IE_CALLINSTANCE and CMBS_IE_MEDIACHANNEL

      cmbs_api_ie_GetFirst( pv_List, &pv_IE, &u16_IE );
      while( pv_IE != NULL )
      {
         app_IEToString( pv_IE, u16_IE );

         appcmbs_IEInfoGet ( pv_IE, u16_IE, &st_IEInfo );

         if( u16_IE == CMBS_IE_CALLINSTANCE )
         {
            pst_Call = _appcall_CallOBJGet( st_IEInfo.Info.u32_CallInstance, NULL );
         }

          _appmedia_CallOBJMediaPropertySet ( pst_Call, u16_IE, &st_IEInfo );

         cmbs_api_ie_GetNext( pv_List, &pv_IE, &u16_IE );
      }

      if ( pst_Call )
      {
         switch (_appcall_CallOBJStateGet(pst_Call))
         {
            case E_APPCMBS_CALL_OUT_INBAND:
            case E_APPCMBS_CALL_ACTIVE:
               appmedia_CallOBJMediaStart( pst_Call->u32_CallInstance,0,NULL);
               _appcall_CallOBJMediaSet(pst_Call, E_APPCMBS_MEDIA_ACTIVE );
              break;
            case E_APPCMBS_CALL_OUT_PEND:
               appmedia_CallOBJTonePlay("CMBS_TONE_DIAL\0", TRUE, _appcall_CallObjLineGet(pst_Call), NULL );
               break;
            case E_APPCMBS_CALL_OUT_PEND_DIAL:
               if(  _appcall_CallObjDigitCollectorEndSymbolCheck( pst_Call ) )
               {
                  appmedia_CallOBJTonePlay("CMBS_TONE_RING_BACK\0", TRUE, _appcall_CallObjLineGet(pst_Call), NULL );
               }
               break;
            default:
            	printf("[THLin]: do NA\n");
               break;
         }
      }
      
      appcall_InfoPrint(); //THLin add
  }
}

//		========== app_OnChannelStartRsp ===========
/*!
		\brief	 		CMBS Target ackknoledge channel start
		\param[in]		pv_List		      IE list pointer
		\return	 		<none>
*/
void        app_OnChannelStartRsp( void * pv_List )
{
   u32      		u32_CallInstance;
   void *   		pv_IE;
   u16      		u16_IE;

   if( pv_List )
   {
      cmbs_api_ie_GetFirst( pv_List, &pv_IE, &u16_IE );
      while( pv_IE != NULL )
      {
         app_IEToString( pv_IE, u16_IE );

         // for your convenience, Call Instance IE will allways be the first
         if( u16_IE == CMBS_IE_CALLINSTANCE )
         {
            cmbs_api_ie_CallInstanceGet( pv_IE, &u32_CallInstance );
//            app_CallAnswer( &CmbsApp.u32_AppRef, u32_CallInstance );
         }
         cmbs_api_ie_GetNext( pv_List, &pv_IE, &u16_IE );
      }
   }
}

//		========== _appcall_CallerIDSet ===========
/*!
		\brief				 convert caller ID from string to IE structure

		\param[in,out]		 pst_This		   pointer to call object
		\param[in,out]		 pst_Properties   pointer to exchange object
		\param[in,out]		 n_properties     number of containing IEs in exchange object
		\param[in,out]		 pst_CallerParty	pointer to IE structure
		\return				 <none>

*/
void _appcall_CallerIDSet( PST_CALL_OBJ pst_This, PST_APPCALL_PROPERTIES pst_Properties, int n_Properties , PST_IE_CALLERPARTY pst_CallerParty )
{
   int   n_Pos;

   n_Pos = _appcall_PropertiesIDXGet( pst_Properties, n_Properties, CMBS_IE_CALLERPARTY );

   if ( n_Pos < n_Properties)
   {
      strcpy( pst_This->ch_CallerID, pst_Properties[n_Pos].psz_Value );
      pst_CallerParty->u8_AddressProperties     = CMBS_ADDR_PROPTYPE_INTERNATIONAL | CMBS_ADDR_PROPPLAN_E164;
      pst_CallerParty->u8_AddressPresentation   = pst_This->ch_CallerID[0] == 'p'?
                                                  CMBS_ADDR_PRESENT_ALLOW:CMBS_ADDR_PRESENT_DENIED;
      if ( pst_This->ch_CallerID[1] >= '0' && pst_This->ch_CallerID[1] <= '7' )
      {
         pst_CallerParty->u8_AddressPresentation   |= (pst_This->ch_CallerID[1]-0x30)<<2;
      }

      pst_CallerParty->u8_AddressLen            = strlen(pst_This->ch_CallerID)-2;
      pst_CallerParty->pu8_Address              = (u8 *)(pst_This->ch_CallerID +2);
   }
}

//		========== _appcall_CalledIDSet ===========
/*!
		\brief				 convert caller ID from string to IE structure

		\param[in,out]		 pst_This		   pointer to call object
		\param[in,out]		 pst_Properties   pointer to exchange object
		\param[in,out]		 n_properties     number of containing IEs in exchange object
		\param[in,out]		 pu8_LineId     	pointer to Line Id
		\return				 <none>

*/
void _appcall_LineIDSet(PST_CALL_OBJ pst_This, PST_APPCALL_PROPERTIES pst_Properties, int n_Properties, u8 * pu8_LineId )
{
   int   n_Pos;
   char* psz_Value;

   if ( pst_This){};

   //[THLin]: seems a bug: shold be CMBS_IE_LINE_ID
#if 0  //org
   n_Pos = _appcall_PropertiesIDXGet( pst_Properties, n_Properties, CMBS_IE_CALLEDPARTY );
#else
   n_Pos = _appcall_PropertiesIDXGet( pst_Properties, n_Properties, CMBS_IE_LINE_ID );
#endif

   if ( n_Pos < n_Properties)
   {
      psz_Value = pst_Properties[n_Pos].psz_Value;

      if ( psz_Value[0] == 'l' )
      {
         //printf("[THLin]: _appcall_LineIDSet, psz_Value[1] = %c\n", psz_Value[1]);
         *pu8_LineId = psz_Value[1] - '0';
      }
#if 1 //THLin Add
      else
      {
      	*pu8_LineId = 0;
      	printf("[THLin]: _appcall_LineIDSet, set line to default value 0\n");
      }
   }
   else
   {
    	*pu8_LineId = 0;
      	printf("[THLin]: _appcall_LineIDSet 2, set line to default value 0\n");
#endif
   }
}


//		========== _appcall_HsMaskSet ===========
/*!
		\brief				 convert Handsets mask from string to IE structure

		\param[in,out]		 pst_This		   pointer to call object
		\param[in,out]		 pst_Properties   pointer to exchange object
		\param[in,out]		 n_properties     number of containing IEs in exchange object
		\param[in,out]		 pu8_HsMask     	pointer to Line Id
		\return				 <none>

*/
void _appcall_HsMaskSet(PST_CALL_OBJ pst_This, PST_APPCALL_PROPERTIES pst_Properties, int n_Properties, u8 * pu8_HsMask )
{
    int i;
   u16 hsm=0;
   int   n_Pos;
   char* psz_Value;

   if ( pst_This){};

   n_Pos = _appcall_PropertiesIDXGet( pst_Properties, n_Properties, CMBS_IE_HANDSETS);

   if ( n_Pos < n_Properties)
   {
      psz_Value = pst_Properties[n_Pos].psz_Value;
      *pu8_HsMask = app_HandsetMap(psz_Value);
   }
}


//		========== _appcall_CalledIDSet ===========
/*!
		\brief				 convert caller ID from string to IE structure

		\param[in,out]		 pst_This		   pointer to call object
		\param[in,out]		 pst_Properties   pointer to exchange object
		\param[in,out]		 n_properties     number of containing IEs in exchange object
		\param[in,out]		 pst_CalledParty	pointer to IE structure
		\return				 <none>

*/
void _appcall_CalledIDSet(PST_CALL_OBJ pst_This, PST_APPCALL_PROPERTIES pst_Properties, int n_Properties , PST_IE_CALLEDPARTY pst_CalledParty )
{
   int   n_Pos;
   char* psz_Value;

   if ( pst_This){};

   n_Pos = _appcall_PropertiesIDXGet( pst_Properties, n_Properties, CMBS_IE_CALLEDPARTY );

   if ( n_Pos < n_Properties)
   {
      psz_Value = pst_Properties[n_Pos].psz_Value;

      if ( psz_Value[0] == 'h' )
      {
         pst_CalledParty->u8_AddressProperties  = CMBS_ADDR_PROPTYPE_UNKNOWN | CMBS_ADDR_PROPPLAN_INTHS;

         pst_CalledParty->u8_AddressPresentation = CMBS_ADDR_PRESENT_ALLOW;
         pst_CalledParty->u8_AddressLen          = strlen(psz_Value)-1;
         pst_CalledParty->pu8_Address            = (u8 *)(psz_Value + 1);
      }
      else
      if ( psz_Value[0] == 'l' )
      {
         pst_CalledParty->u8_AddressProperties   = CMBS_ADDR_PROPTYPE_UNKNOWN | CMBS_ADDR_PROPPLAN_INTLINE;

      pst_CalledParty->u8_AddressPresentation   = CMBS_ADDR_PRESENT_ALLOW;
         pst_CalledParty->u8_AddressLen          = 0;
         pst_CalledParty->pu8_Address            = 0;
      }
  }
}

//		========== _appcall_MediaDescriptorSet ===========
/*!
		\brief				 convert caller ID from string to IE structure

		\param[in,out]		 pst_This		   pointer to call object
		\param[in,out]		 pst_Properties   pointer to exchange object
		\param[in,out]		 n_properties     number of containing IEs in exchange object
		\param[in,out]		 pst_MediaDescr	pointer to IE structure
		\return				 <none>

*/
int  _appcall_MediaDescriptorSet(PST_CALL_OBJ pst_This, PST_APPCALL_PROPERTIES pst_Properties, int n_Properties , PST_IE_MEDIA_DESCRIPTOR pst_MediaDescr )
{
   int   n_Pos;

   n_Pos = _appcall_PropertiesIDXGet( pst_Properties, n_Properties, CMBS_IE_MEDIADESCRIPTOR );

   if ( n_Pos < n_Properties)
   {
      if ( pst_Properties[n_Pos].psz_Value[0] == 'w' )
      {
         pst_This->e_Codec = CMBS_AUDIO_CODEC_PCM_LINEAR_WB;
      }
      else if ( pst_Properties[n_Pos].psz_Value[0] == 'a' )
      {
         pst_This->e_Codec = CMBS_AUDIO_CODEC_PCMA;
      }
      else if ( pst_Properties[n_Pos].psz_Value[0] == 'u' )
      {
         pst_This->e_Codec = CMBS_AUDIO_CODEC_PCMU;
      }
      else
      {
         pst_This->e_Codec = CMBS_AUDIO_CODEC_PCM_LINEAR_NB;
      }

      pst_MediaDescr->e_Codec = pst_This->e_Codec;
   }
   else
   {
      return FALSE;
   }

   return TRUE;

}

//		========== _appcall_CallerNameSet ===========
/*!
		\brief				 convert caller ID from string to IE structure

		\param[in,out]		 pst_This		   pointer to call object
		\param[in,out]		 pst_Properties   pointer to exchange object
		\param[in,out]		 n_properties     number of containing IEs in exchange object
		\param[in,out]		 pst_CallerName	pointer to IE structure
		\return				 <none>

*/
int  _appcall_CallerNameSet(PST_CALL_OBJ pst_This, PST_APPCALL_PROPERTIES pst_Properties, int n_Properties , PST_IE_CALLERNAME pst_CallerName )
{
   int   n_Pos;

   if (pst_This){};

   n_Pos = _appcall_PropertiesIDXGet( pst_Properties, n_Properties, CMBS_IE_CALLERNAME );

   if ( n_Pos < n_Properties)
   {
      pst_CallerName->pu8_Name   = (u8*)pst_Properties[n_Pos].psz_Value;
      pst_CallerName->u8_DataLen = strlen(pst_Properties[n_Pos].psz_Value);
   }
   else
   {
      return FALSE;
   }

   return TRUE;
}

//		========== _appcall_ReleaseReasonSet ===========
/*!
		\brief				 convert caller ID from string to IE structure

		\param[in,out]		 pst_This		   pointer to call object
		\param[in,out]		 pst_Properties   pointer to exchange object
		\param[in,out]		 n_properties     number of containing IEs in exchange object
		\param[in,out]		 pst_Reason	      pointer to IE structure
		\return				 <none>

*/
int  _appcall_ReleaseReasonSet(PST_CALL_OBJ pst_This, PST_APPCALL_PROPERTIES pst_Properties, int n_Properties , PST_IE_RELEASE_REASON pst_Reason )
{
   int   n_Pos;

   if (pst_This){};

   n_Pos = _appcall_PropertiesIDXGet( pst_Properties, n_Properties, CMBS_IE_CALLRELEASE_REASON );

   if ( n_Pos < n_Properties)
   {
      pst_Reason->e_Reason       = atoi( pst_Properties[n_Pos].psz_Value);
      pst_Reason->u32_ExtReason  = 0;
   }
   else
   {
      return FALSE;
   }

   return TRUE;
}

//		========== _appcall_CallProgressSet ===========
/*!
		\brief				 convert caller ID from string to IE structure

		\param[in,out]		 pst_This		   pointer to call object
		\param[in,out]		 pst_Properties   pointer to exchange object
		\param[in,out]		 n_properties     number of containing IEs in exchange object
		\param[in,out]		 pst_CallProgress pointer to IE structure
		\return				 <none>

*/
int  _appcall_CallProgressSet( PST_CALL_OBJ pst_This, PST_APPCALL_PROPERTIES pst_Properties, int n_Properties , PST_IE_CALLPROGRESS pst_CallProgress )
{
   int   n_Pos;

   if (pst_This){};

   n_Pos = _appcall_PropertiesIDXGet( pst_Properties, n_Properties, CMBS_IE_CALLPROGRESS );

   if ( n_Pos < n_Properties)
   {
      pst_CallProgress->e_Progress = getidx_CMBS_CALL_PROGR( pst_Properties[n_Pos].psz_Value);
   }
   else
   {
      return FALSE;
   }

   return TRUE;
}

//		========== _appcall_CallDisplaySet ===========
/*!
		\brief				 convert caller ID from string to IE structure

		\param[in,out]		 pst_This		   pointer to call object
		\param[in,out]		 pst_Properties   pointer to exchange object
		\param[in,out]		 n_properties     number of containing IEs in exchange object
		\param[in,out]		 pst_CallInfo     pointer to IE structure
		\return				 <none>

*/
int  _appcall_CallDisplaySet( PST_CALL_OBJ pst_This, PST_APPCALL_PROPERTIES pst_Properties, int n_Properties , PST_IE_CALLINFO pst_CallInfo )
{
   int   n_Pos;

   if ( pst_This){;}

   n_Pos = _appcall_PropertiesIDXGet( pst_Properties, n_Properties, CMBS_IE_CALLINFO );

   if ( n_Pos < n_Properties)
   {
      pst_CallInfo->e_Type     = CMBS_CALL_INFO_TYPE_DISPLAY;
      pst_CallInfo->pu8_Info   = (u8*)pst_Properties[n_Pos].psz_Value;
                                 // take care on max string length according DECT specification
      pst_CallInfo->u8_DataLen = strlen (pst_Properties[n_Pos].psz_Value);
   }
   else
   {
      return FALSE;
   }

   return TRUE;
}

//		========== app_CallEntity ===========
/*!
		\brief			 dispatcher fo call CMBS events

		\param[in]		 pv_AppRef		 application reference pointer
		\param[in]		 e_EventID		 received CMBS event
		\param[in]		 pv_EventData	 pointer to IE list
		\return			 <int>   TRUE, if consumed

*/
int  app_CallEntity( void * pv_AppRef, E_CMBS_EVENT_ID e_EventID, void * pv_EventData )
{
   switch( e_EventID )
   {
      case CMBS_EV_DEE_CALL_ESTABLISH:
         app_OnCallEstablish( pv_AppRef,pv_EventData );
         return TRUE;

      case CMBS_EV_DEE_CALL_PROGRESS:
         app_OnCallProgress( pv_AppRef,pv_EventData );
         return TRUE;

      case CMBS_EV_DEE_CALL_INBANDINFO:
         app_OnCallInbandInfo( pv_AppRef,pv_EventData );
         return TRUE;

      case CMBS_EV_DEE_CALL_ANSWER:
         app_OnCallAnswer( pv_AppRef,pv_EventData );
         return TRUE;

      case CMBS_EV_DEE_CALL_RELEASE:
         app_OnCallRelease( pv_AppRef,pv_EventData );
         return TRUE;

      case CMBS_EV_DEE_CALL_MEDIA_UPDATE:
         app_OnCallMediaUpdate( pv_EventData );
         return TRUE;

      case CMBS_EV_DEM_CHANNEL_START_RES:
         app_OnChannelStartRsp( pv_EventData );
         return TRUE;

      case CMBS_EV_DEE_CALL_HOLD:
         app_OnCallHold( pv_AppRef, pv_EventData );
         return TRUE;
         
      case CMBS_EV_DEE_CALL_RESUME:
         app_OnCallResume( pv_AppRef, pv_EventData );
         return TRUE;

      case CMBS_EV_DCM_CALL_STATE:
         app_OnCallState( pv_AppRef, pv_EventData );
         return TRUE;

      default:
         return FALSE;
   }

   return FALSE;
}
//		========== appcall_ReleaseCall ===========
/*!
		\brief			release call identified by line ID or caller party
		\param[in]		pst_Properties	pointer to exchange object
		\param[in]		n_Properties		number of containing IEs
		\param[in]		n_Line		      relevant line ID
		\param[in]		psz_CLI		      Caller Party string
		\return			<int>             TRUE, if successful
*/
int      appcall_ReleaseCall( PST_APPCALL_PROPERTIES pst_Properties, int n_Properties, int n_Line, char * psz_CLI )
{
   PST_CALL_OBJ   pst_This;

   if ( ! psz_CLI )
   {
      // line ID is available
      pst_This = g_call_obj + n_Line;
   }
   else
   {
      pst_This = _appcall_CallOBJGet( 0, psz_CLI );
   }

   if ( pst_This )
   {
      void *         pv_RefIEList = NULL;

      ST_IE_RELEASE_REASON st_Reason;

      printf( "Release Call\n");

      appmedia_CallOBJMediaStop( pst_This->u32_CallInstance,0,NULL );

      pv_RefIEList = cmbs_api_ie_GetList();
      if( pv_RefIEList )
      {
         // Add call Instance IE
         cmbs_api_ie_CallInstanceAdd( pv_RefIEList, pst_This->u32_CallInstance );

         // Add release reason IE
         _appcall_ReleaseReasonSet(pst_This, pst_Properties, n_Properties , &st_Reason );

         cmbs_api_ie_CallReleaseReasonAdd( pv_RefIEList, &st_Reason );

         cmbs_dee_CallRelease( g_cmbsappl.pv_CMBSRef, pv_RefIEList );

         _appcall_CallOBJDelete( pst_This->u32_CallInstance, NULL );

         return TRUE;
      }
   }

   return FALSE;
}

//		========== appcall_EstablishCall ===========
/*!
		\brief			release call identified by line ID or caller party
		\param[in]		pst_Properties	pointer to exchange object
		\param[in]		n_Properties		number of containing IEs
		\return			<int>             call number, or 0xFF if failed!
*/
int      appcall_EstablishCall ( PST_APPCALL_PROPERTIES pst_Properties, int n_Properties )
{
   PST_CALL_OBJ   pst_This;
   void *         pv_RefIEList = NULL;
   ST_IE_CALLEDPARTY
                  st_CalledParty;
   ST_IE_CALLERPARTY
                  st_CallerParty;
   ST_IE_CALLERNAME
                  st_CallerName;
   ST_IE_MEDIA_DESCRIPTOR
                  st_MediaDescr;
   u8             u8_LineId = 0xFF;
   u8             u8_HsMask = 0xFF;

   pst_This = _appcall_CallOBJNew( );
   if ( !pst_This )
   {
      return 0xFF;
   }
   // Create a new Call ID
   pst_This->u32_CallInstance = cmbs_dee_CallInstanceNew( g_cmbsappl.pv_CMBSRef );
   // Initialize IE List
   pv_RefIEList = cmbs_api_ie_GetList();
   if( pv_RefIEList )
   {
      // Add call Instance IE
      cmbs_api_ie_CallInstanceAdd( pv_RefIEList, pst_This->u32_CallInstance );

      // Add Line Id
      _appcall_LineIDSet( pst_This, pst_Properties, n_Properties, &u8_LineId );
      printf("[THLin]: appcall_EstablishCall, add line ID = %d\n", u8_LineId);
      cmbs_api_ie_LineIdAdd( pv_RefIEList, u8_LineId );

      if (u8_LineId != pst_This - g_call_obj)
      {
      	printf("[THLin]: appcall_EstablishCall: Error! LineID Set(%d) != pst_This - g_call_obj (%d)\n", u8_LineId, pst_This - g_call_obj);
      }

      // Add called ID IE
      _appcall_CalledIDSet( pst_This, pst_Properties, n_Properties, &st_CalledParty );
      cmbs_api_ie_CalledPartyAdd( pv_RefIEList, &st_CalledParty );

      // Add caller ID IE
      _appcall_CallerIDSet( pst_This, pst_Properties, n_Properties , &st_CallerParty );
      cmbs_api_ie_CallerPartyAdd( pv_RefIEList, &st_CallerParty );

      // Add caller name IE
      //st_CallerName.pu8_Name = (u8 *)szCallerName;
      //st_CallerName.u8_DataLen = sizeof( szCallerName ) -1;
      if ( _appcall_CallerNameSet(pst_This, pst_Properties, n_Properties , &st_CallerName ) )
      {
         cmbs_api_ie_CallerNameAdd( pv_RefIEList, &st_CallerName );
      }
      // Add media descriptor IE
      _appcall_MediaDescriptorSet( pst_This, pst_Properties, n_Properties , &st_MediaDescr );
      printf("[THLin]: appcall_EstablishCall, add MediaDescr = %d\n", st_MediaDescr);
      cmbs_api_ie_MediaDescAdd( pv_RefIEList, &st_MediaDescr );
      // Establish the call now...
      cmbs_dee_CallEstablish( g_cmbsappl.pv_CMBSRef, pv_RefIEList );

      return (pst_This - g_call_obj);
   }

   return 0xFF;
}
//       =========== _appcall_LineOBJGet ============
/*!
   \brief   get line obj get identified by line ID, caller party
   \param[in]     n_Line         Line ID
   \param[in]     psz_CLI        pointer to caller party string
   \return        <PST_CALL_OBJ> return
*/

PST_CALL_OBJ   _appcall_LineOBJGet( int n_Line, char * psz_CLI )
{
   PST_CALL_OBJ   pst_This;

   if ( ! psz_CLI )
   {
      // line ID is available
      pst_This = g_call_obj + n_Line;
      printf("[THLin]: _appcall_LineOBJGet case1, pst_This = %p\n", pst_This);
   }
   else
   {
      pst_This = _appcall_CallOBJGet( 0, psz_CLI );
      printf("[THLin]: _appcall_LineOBJGet case2, pst_This = %p\n", pst_This);
   }

   return pst_This;
}

//		========== appcall_ProgressCall ===========
/*!
		\brief			providing progress inforation of call identified by line ID or caller party
		\param[in]		pst_Properties	pointer to exchange object
		\param[in]		n_Properties		number of containing IEs
		\param[in]		n_Line		      relevant line ID
		\param[in]		psz_CLI		      Caller Party string
		\return			<int>             TRUE, if successful
*/
int appcall_ProgressCall ( PST_APPCALL_PROPERTIES pst_Properties, int n_Properties, int n_Line, char * psz_CLI )
{
   PST_CALL_OBJ pst_This;
   ST_IE_CALLPROGRESS st_CallProgress;

   pst_This = _appcall_LineOBJGet( n_Line, psz_CLI );

   if ( pst_This )
   {
      void *         pv_RefIEList = NULL;

      pv_RefIEList = cmbs_api_ie_GetList();

      // Answer the call: 
      // send CMBS_IE_CALLINSTANCE, CMBS_IE_MEDIADESCRIPTOR and CMBS_IE_CALLPROGRESS
      if( pv_RefIEList )
      {
         ST_IE_MEDIA_DESCRIPTOR st_MediaDesc;
         

         // Add Call Instance IE
         cmbs_api_ie_CallInstanceAdd( pv_RefIEList, pst_This->u32_CallInstance );

         _appcall_CallProgressSet( pst_This, pst_Properties, n_Properties , &st_CallProgress );
 
         if ( st_CallProgress.e_Progress != CMBS_CALL_PROGR_RINGING )
         {
            // Add media descriptor IE
            memset( &st_MediaDesc, 0, sizeof(ST_IE_MEDIA_DESCRIPTOR) );
            st_MediaDesc.e_Codec = pst_This->e_Codec;
            printf("[THLin]: appcall_ProgressCall, add Media Desc = %d\n", st_MediaDesc.e_Codec);
            cmbs_api_ie_MediaDescAdd( pv_RefIEList, &st_MediaDesc );
         }

        if(st_CallProgress.e_Progress == CMBS_CALL_PROGR_SETUP_ACK)
        {
            cmbs_api_ie_LineIdAdd(pv_RefIEList, n_Line);
        }
	 // Add Call Progress IE
         cmbs_api_ie_CallProgressAdd( pv_RefIEList, &st_CallProgress );

         cmbs_dee_CallProgress( g_cmbsappl.pv_CMBSRef, pv_RefIEList );

         switch (st_CallProgress.e_Progress)
         {
            case CMBS_CALL_PROGR_SETUP_ACK:
                _appcall_CallOBJStateSet(pst_This, E_APPCMBS_CALL_OUT_PEND );
                break;
            case CMBS_CALL_PROGR_PROCEEDING:
               _appcall_CallOBJStateSet(pst_This, E_APPCMBS_CALL_OUT_PROC );
               break;
            case CMBS_CALL_PROGR_RINGING :
               _appcall_CallOBJStateSet(pst_This, E_APPCMBS_CALL_OUT_RING );
               break;
            case CMBS_CALL_PROGR_INBAND:
               _appcall_CallOBJStateSet(pst_This,   E_APPCMBS_CALL_OUT_INBAND);
            default:
               break;
         }
      }
   }

   return TRUE;
}

//		========== appcall_AnswerCall ===========
/*!
		\brief			answer call identified by line ID or caller party
		\param[in]		pst_Properties	pointer to exchange object
		\param[in]		n_Properties		number of containing IEs
		\param[in]		n_Line		      relevant line ID
		\param[in]		psz_CLI		      Caller Party string
		\return			<int>             TRUE, if successful
*/
int      appcall_AnswerCall ( PST_APPCALL_PROPERTIES pst_Properties, int n_Properties, int n_Line, char * psz_CLI )
{
   PST_CALL_OBJ   pst_This;
   ST_IE_MEDIA_CHANNEL
                  st_MediaChannel;

   if ( pst_Properties && n_Properties){};

   pst_This = _appcall_LineOBJGet( n_Line, psz_CLI );

   if ( pst_This )
   {
      void *         pv_RefIEList = NULL;

      pv_RefIEList = cmbs_api_ie_GetList();

      if( pv_RefIEList )
      {
         ST_IE_MEDIA_DESCRIPTOR st_MediaDesc;
         // Add call Instance IE
         cmbs_api_ie_CallInstanceAdd( pv_RefIEList, pst_This->u32_CallInstance );
         // answer the call: send CMBS_IE_CALLINSTANCE, CMBS_IE_MEDIACHANNEL and CMBS_IE_MEDIADESCRIPTOR

         // add media descriptor IE
         memset( &st_MediaDesc, 0, sizeof(ST_IE_MEDIA_DESCRIPTOR) );
         st_MediaDesc.e_Codec = pst_This->e_Codec;
         cmbs_api_ie_MediaDescAdd( pv_RefIEList, &st_MediaDesc );

         // add media channel IE
         memset( &st_MediaChannel, 0, sizeof(ST_IE_MEDIA_CHANNEL) );
         st_MediaChannel.e_Type        = CMBS_MEDIA_TYPE_AUDIO_IOM;
         st_MediaChannel.u32_ChannelID = pst_This->u32_ChannelID;
         cmbs_api_ie_MediaChannelAdd( pv_RefIEList, &st_MediaChannel );

	if(pst_This->u32_ChannelID != pst_This->u8_LineId)
		printf("[THLin]: appcall_AnswerCall, pst_This->u32_ChannelID != pst_This->u8_LineId (%d != %d)\n", pst_This->u32_ChannelID, pst_This->u8_LineId);


         cmbs_dee_CallAnswer( g_cmbsappl.pv_CMBSRef, pv_RefIEList );

         _appcall_CallOBJStateSet(pst_This, E_APPCMBS_CALL_ACTIVE );

         if ( g_call_automat )
         {
            appmedia_CallOBJMediaStart( pst_This->u32_CallInstance , 0, NULL );
         }
      }
   }

   return TRUE;
}

//		========== appcall_DisplayCall ===========
/*!
		\brief			provide display infomration of a call identified by line ID or caller party
		\param[in]		pst_Properties	pointer to exchange object
		\param[in]		n_Properties		number of containing IEs
		\param[in]		n_Line		      relevant line ID
		\param[in]		psz_CLI		      Caller Party string
		\return			<int>             TRUE, if successful
*/
int      appcall_DisplayCall ( PST_APPCALL_PROPERTIES pst_Properties, int n_Properties, int n_Line, char * psz_CLI )
{
   PST_CALL_OBJ   pst_This;

   pst_This = _appcall_LineOBJGet( n_Line, psz_CLI );

   if ( pst_This )
   {
      void *         pv_RefIEList = NULL;

      printf( "CW Display\n" );
      pv_RefIEList = cmbs_api_ie_GetList();

      if( pv_RefIEList )
      {
         ST_IE_CALLINFO st_CallInfo;
         // Add call Instance IE
         cmbs_api_ie_CallInstanceAdd( pv_RefIEList, pst_This->u32_CallInstance );

         _appcall_CallDisplaySet( pst_This, pst_Properties, n_Properties , &st_CallInfo );
         cmbs_api_ie_CallInfoAdd( pv_RefIEList, &st_CallInfo );

         cmbs_dee_CallInbandInfo(  g_cmbsappl.pv_CMBSRef, pv_RefIEList );
      }
   }

   return TRUE;
}

//		========== appcall_HoldCall ===========
/*!
		\brief			signal CMBS target call hold, mute media stream
		\param[in]		n_Line		      relevant line ID
		\param[in]		psz_CLI		      Caller Party string
		\return			<int>             TRUE, if successful
*/
void     appcall_HoldCall( int n_Line, char * psz_CLI )
{
   PST_CALL_OBJ   pst_This;
   void *         pv_RefIEList = cmbs_api_ie_GetList();

   pst_This = _appcall_LineOBJGet( n_Line, psz_CLI );

   if( pv_RefIEList )
   {
      // Add call Instance IE
      cmbs_api_ie_CallInstanceAdd( pv_RefIEList, pst_This->u32_CallInstance );

      cmbs_dee_CallHold( g_cmbsappl.pv_CMBSRef, pv_RefIEList );
   }
}

//		========== appcall_ResumeCall ===========
/*!
		\brief			signal CMBS target call hold, mute media stream
		\param[in]		n_Line		      relevant line ID
		\param[in]		psz_CLI		      Caller Party string
		\return			<int>             TRUE, if successful
*/
void     appcall_ResumeCall( int n_Line, char * psz_CLI )
{
   PST_CALL_OBJ   pst_This;
   void *         pv_RefIEList = cmbs_api_ie_GetList();

   pst_This = _appcall_LineOBJGet( n_Line, psz_CLI );

   if( pv_RefIEList )
   {
      // Add call Instance IE
      cmbs_api_ie_CallInstanceAdd( pv_RefIEList, pst_This->u32_CallInstance );

      cmbs_dee_CallResume( g_cmbsappl.pv_CMBSRef, pv_RefIEList );
   }
}
void     appcall_AutomatMode ( E_APPCALL_AUTOMAT_MODE e_Mode )
{
   if ( e_Mode == E_APPCALL_AUTOMAT_MODE_ON )
   {
      printf ( "Switch application to automat mode\n" );
   }
   else
   {
      printf ( "Switch application to step mode\n" );
   }

   g_call_automat = e_Mode;
}


//*/
