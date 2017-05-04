/*!
*	\file		cmbs_str.c
*	\brief		CMBS enumeration string converter
*	\Author		kelbch
*
*	@(#)	%filespec: cmbs_str.c %
*
*******************************************************************************
*	\par	History
*	\n==== History ============================================================\n
*	date			name		version	 action                                \n
*	---------------------------------------------------------------------------\n
*******************************************************************************
*	COPYRIGHT DOSCH & AMAND RESEARCH GMBH & CO.KG
*	DOSCH & AMAND RESEARCH GMBH & CO.KG Confidential
*
*******************************************************************************/
#include "cmbs_api.h"
#include <stdlib.h>
#include <string.h>

#ifndef caseretstr
#define caseretstr(x) case x: return #x
#endif

//		========== getstr_E_CMBS_EVENT ===========
/*!
		\brief          return string of events enumeration
		\param[in]		 e_Event		value of enumeration
		\return			 <char *>   return string of enumeration
*/
char *            getstr_E_CMBS_EVENT( E_CMBS_EVENT_ID e_Event )
{
   switch( e_Event )
   {
      caseretstr(CMBS_EV_UNDEF);
      caseretstr(CMBS_EV_DSR_HS_PAGE);
      caseretstr(CMBS_EV_DSR_HS_PAGE_RES);
      caseretstr(CMBS_EV_DSR_HS_DELETE);
      caseretstr(CMBS_EV_DSR_HS_DELETE_RES);
      caseretstr(CMBS_EV_DSR_HS_REGISTERED);
/* ALTDV */
      caseretstr(CMBS_EV_DSR_HS_SUBSCRIBED);
      caseretstr(CMBS_EV_DSR_CORD_OPENREG);
      caseretstr(CMBS_EV_DSR_CORD_OPENREG_RES);
      caseretstr(CMBS_EV_DSR_CORD_CLOSEREG);
      caseretstr(CMBS_EV_DSR_CORD_CLOSEREG_RES);
      caseretstr(CMBS_EV_DSR_PARAM_GET);
      caseretstr(CMBS_EV_DSR_PARAM_GET_RES);
      caseretstr(CMBS_EV_DSR_PARAM_SET);
      caseretstr(CMBS_EV_DSR_PARAM_SET_RES);
      caseretstr(CMBS_EV_DSR_FW_UPD_START);
      caseretstr(CMBS_EV_DSR_FW_UPD_START_RES);
      caseretstr(CMBS_EV_DSR_FW_UPD_PACKETNEXT);
      caseretstr(CMBS_EV_DSR_FW_UPD_PACKETNEXT_RES);
      caseretstr(CMBS_EV_DSR_FW_UPD_END);
      caseretstr(CMBS_EV_DSR_FW_UPD_END_RES);
      caseretstr(CMBS_EV_DSR_FW_VERSION_GET);
      caseretstr(CMBS_EV_DSR_FW_VERSION_GET_RES);
      caseretstr(CMBS_EV_DSR_SYS_START);
      caseretstr(CMBS_EV_DSR_SYS_START_RES);
      caseretstr(CMBS_EV_DSR_SYS_SEND_RAWMSG);
      caseretstr(CMBS_EV_DSR_SYS_SEND_RAWMSG_RES);
      caseretstr(CMBS_EV_DSR_SYS_STATUS);
      caseretstr(CMBS_EV_DSR_SYS_LOG);
      caseretstr(CMBS_EV_DSR_SYS_RESET);
/* ALTDV */
      caseretstr(CMBS_EV_DSR_SYS_POWER_OFF);
      caseretstr(CMBS_EV_DSR_RF_SUSPEND);
      caseretstr(CMBS_EV_DSR_RF_RESUME);
      caseretstr(CMBS_EV_DSR_TURN_ON_NEMO);
      caseretstr(CMBS_EV_DSR_TURN_OFF_NEMO);
      caseretstr(CMBS_EV_DEE_CALL_ESTABLISH);
      caseretstr(CMBS_EV_DEE_CALL_PROGRESS);
      caseretstr(CMBS_EV_DEE_CALL_ANSWER);
      caseretstr(CMBS_EV_DEE_CALL_RELEASE);
      caseretstr(CMBS_EV_DEE_CALL_RELEASECOMPLETE);
      caseretstr(CMBS_EV_DEE_CALL_INBANDINFO);
      caseretstr(CMBS_EV_DEE_CALL_MEDIA_OFFER);
      caseretstr(CMBS_EV_DEE_CALL_MEDIA_OFFER_RES);
      caseretstr(CMBS_EV_DEE_CALL_MEDIA_UPDATE);
      caseretstr(CMBS_EV_DEE_CALL_HOLD);
      caseretstr(CMBS_EV_DEE_CALL_RESUME);
      caseretstr(CMBS_EV_DEM_CHANNEL_START);
      caseretstr(CMBS_EV_DEM_CHANNEL_START_RES);
      caseretstr(CMBS_EV_DEM_CHANNEL_STOP);
      caseretstr(CMBS_EV_DEM_CHANNEL_STOP_RES);
      caseretstr(CMBS_EV_DEM_TONE_START);
      caseretstr(CMBS_EV_DEM_TONE_START_RES);
      caseretstr(CMBS_EV_DEM_TONE_STOP);
      caseretstr(CMBS_EV_DEM_TONE_STOP_RES);
      caseretstr(CMBS_EV_DSR_SYS_LOG_START);
      caseretstr(CMBS_EV_DSR_SYS_LOG_STOP);
      caseretstr(CMBS_EV_DSR_SYS_LOG_REQ);
      caseretstr(CMBS_EV_DSR_PARAM_UPDATED);
      caseretstr(CMBS_EV_DSR_PARAM_AREA_UPDATED);
      caseretstr(CMBS_EV_DSR_PARAM_AREA_GET);
      caseretstr(CMBS_EV_DSR_PARAM_AREA_GET_RES);
      caseretstr(CMBS_EV_DSR_PARAM_AREA_SET);
      caseretstr(CMBS_EV_DSR_PARAM_AREA_SET_RES);

      caseretstr(CMBS_EV_DSR_GEN_SEND_VMWI);
      caseretstr(CMBS_EV_DSR_GEN_SEND_VMWI_RES);
      caseretstr(CMBS_EV_DSR_GEN_SEND_MISSED_CALLS);
      caseretstr(CMBS_EV_DSR_GEN_SEND_MISSED_CALLS_RES);
      caseretstr(CMBS_EV_DSR_GEN_SEND_LIST_CHANGED);
      caseretstr(CMBS_EV_DSR_GEN_SEND_LIST_CHANGED_RES);
      caseretstr(CMBS_EV_DSR_GEN_SEND_WEB_CONTENT);
      caseretstr(CMBS_EV_DSR_GEN_SEND_WEB_CONTENT_RES);
      caseretstr(CMBS_EV_DSR_GEN_SEND_PROP_EVENT);
      caseretstr(CMBS_EV_DSR_GEN_SEND_PROP_EVENT_RES);
      caseretstr(CMBS_EV_DSR_TIME_UPDATE);
      caseretstr(CMBS_EV_DSR_TIME_UPDATE_RES);
      caseretstr(CMBS_EV_DSR_TIME_INDICATION);
      caseretstr(CMBS_EV_DSR_HS_DATA_SESSION_OPEN);
      caseretstr(CMBS_EV_DSR_HS_DATA_SESSION_OPEN_RES);
      caseretstr(CMBS_EV_DSR_HS_DATA_SESSION_CLOSE);
      caseretstr(CMBS_EV_DSR_HS_DATA_SESSION_CLOSE_RES);
      caseretstr(CMBS_EV_DSR_HS_DATA_RECEIVE);
      caseretstr(CMBS_EV_DSR_HS_DATA_RECEIVE_RES);
      caseretstr(CMBS_EV_DSR_LA_SESSION_START);
      caseretstr(CMBS_EV_DSR_LA_SESSION_START_RES);
      caseretstr(CMBS_EV_DSR_LA_SESSION_END);
      caseretstr(CMBS_EV_DSR_LA_SESSION_END_RES);
      caseretstr(CMBS_EV_DSR_LA_QUERY_SUPP_ENTRY_FIELDS);
      caseretstr(CMBS_EV_DSR_LA_QUERY_SUPP_ENTRY_FIELDS_RES);
      caseretstr(CMBS_EV_DSR_LA_READ_ENTRIES);
      caseretstr(CMBS_EV_DSR_LA_READ_ENTRIES_RES);
      caseretstr(CMBS_EV_DSR_LA_SEARCH_ENTRIES);
      caseretstr(CMBS_EV_DSR_LA_SEARCH_ENTRIES_RES);
      caseretstr(CMBS_EV_DSR_LA_EDIT_ENTRY);
      caseretstr(CMBS_EV_DSR_LA_EDIT_ENTRY_RES);
      caseretstr(CMBS_EV_DSR_LA_SAVE_ENTRY);
      caseretstr(CMBS_EV_DSR_LA_SAVE_ENTRY_RES);
      caseretstr(CMBS_EV_DSR_LA_DELETE_ENTRY);
      caseretstr(CMBS_EV_DSR_LA_DELETE_ENTRY_RES);
      caseretstr(CMBS_EV_DSR_LA_DELETE_LIST);
      caseretstr(CMBS_EV_DSR_LA_DELETE_LIST_RES);
      caseretstr(CMBS_EV_DSR_LA_DATA_PACKET_RECEIVE);
      caseretstr(CMBS_EV_DSR_LA_DATA_PACKET_RECEIVE_RES);
      caseretstr(CMBS_EV_DSR_LA_DATA_PACKET_SEND);
      caseretstr(CMBS_EV_DSR_LA_DATA_PACKET_SEND_RES);

      caseretstr(CMBS_EV_DSR_USER_DEFINED_START);
      caseretstr(CMBS_EV_DSR_USER_DEFINED_END);

      caseretstr(CMBS_EV_MAX);
   }

   if(CMBS_EV_DSR_USER_DEFINED_START < e_Event && e_Event < CMBS_EV_DSR_USER_DEFINED_END)
   {
      return (char *)"CMBS_EV_DSR_USER_DEFINED";
   }

   return (char*)"Unknown Event";
}

//		========== getstr_E_CMBS_IE ===========
/*!
		\brief          returns string of IE enumeration
		\param[in]		 e_IE		   value of enumeration
		\return			 <char *>   return string of enumeration
*/

char *            getstr_E_CMBS_IE( E_CMBS_IE_TYPE e_IE )
{
   switch( e_IE )
   {
      caseretstr( CMBS_IE_UNDEF              );
      caseretstr( CMBS_IE_CALLINSTANCE       );
      caseretstr( CMBS_IE_CALLERPARTY        );
      caseretstr( CMBS_IE_CALLERNAME         );
      caseretstr( CMBS_IE_CALLEDPARTY        );
      caseretstr( CMBS_IE_CALLPROGRESS       );
      caseretstr( CMBS_IE_CALLINFO           );
      caseretstr( CMBS_IE_DISPLAY_STRING     );
      caseretstr( CMBS_IE_CALLRELEASE_REASON );
      caseretstr( CMBS_IE_MEDIACHANNEL       );
      caseretstr( CMBS_IE_MEDIADESCRIPTOR    );
      caseretstr( CMBS_IE_TONE               );
      caseretstr( CMBS_IE_TIMEOFDAY          );
      caseretstr( CMBS_IE_HANDSETINFO        );
      caseretstr( CMBS_IE_PARAMETER          );
      caseretstr( CMBS_IE_PARAMETER_AREA     );
      caseretstr( CMBS_IE_FW_VERSION         );
      caseretstr( CMBS_IE_SYS_LOG            );
      caseretstr( CMBS_IE_RESPONSE           );
      caseretstr( CMBS_IE_STATUS             );
      caseretstr( CMBS_IE_INTEGER_VALUE      );
      caseretstr( CMBS_IE_LINE_ID            );
      caseretstr( CMBS_IE_REQUEST_ID         );
      caseretstr( CMBS_IE_HANDSETS           );
      caseretstr( CMBS_IE_GEN_EVENT          );
      caseretstr( CMBS_IE_PROP_EVENT         );
      caseretstr( CMBS_IE_DATETIME           );
      caseretstr( CMBS_IE_DATA               );
      caseretstr( CMBS_IE_DATA_SESSION_ID    );
      caseretstr( CMBS_IE_DATA_SESSION_TYPE  );
      caseretstr( CMBS_IE_LA_SESSION_ID      );
      caseretstr( CMBS_IE_LA_LIST_ID         );
      caseretstr( CMBS_IE_LA_FIELDS          );
      caseretstr( CMBS_IE_LA_SORT_FIELDS     );
      caseretstr( CMBS_IE_LA_EDIT_FIELDS     );
      caseretstr( CMBS_IE_LA_CONST_FIELDS    );
      caseretstr( CMBS_IE_LA_SEARCH_CRITERIA );
      caseretstr( CMBS_IE_LA_ENTRY_ID        );
      caseretstr( CMBS_IE_LA_ENTRY_INDEX     );
      caseretstr( CMBS_IE_LA_ENTRY_COUNT     );
      caseretstr( CMBS_IE_LA_IS_LAST         );
      caseretstr( CMBS_IE_LA_REJECT_REASON   );
      caseretstr( CMBS_IE_LA_NR_OF_ENTRIES   );

      caseretstr( CMBS_IE_MAX                );
      default:
         return (char*) "Unknown Information Element";
   }
}
//		========== getstr_E_CMBS_PARAM ===========
/*!
		\brief          return string of enumeration
		\param[in]		 e_Param		value of enumeration
		\return			 <char *>   return string of enumeration
*/
char *            getstr_E_CMBS_PARAM( E_CMBS_PARAM e_Param )
{
   switch ( e_Param )
   {
      caseretstr(CMBS_PARAM_UNKNOWN);
      caseretstr(CMBS_PARAM_RFPI);                 /*!< Base identity */
      caseretstr(CMBS_PARAM_RVBG);                 /*!< VBG register */
      caseretstr(CMBS_PARAM_RVREF);                /*!< VREF register */
      caseretstr(CMBS_PARAM_RXTUN);                /*!< RTUN register */
      caseretstr(CMBS_PARAM_MASTER_PIN);           /*!< Base master PIN code */
      caseretstr(CMBS_PARAM_AUTH_PIN);             /*!< Authentication PIN code */
      caseretstr(CMBS_PARAM_COUNTRY);              /*!< Configure cordless module to specific country settings */
      caseretstr(CMBS_PARAM_SIGNALTONE_DEFAULT);   /*!< Define the default behavior for outgoing calls */
      caseretstr(CMBS_PARAM_TEST_MODE);            /*!< Test mode. 0x00: Normal Operation; 0x01: TBR6; 0x02: TBR10 */
      caseretstr(CMBS_PARAM_AUTO_REGISTER);
      caseretstr(CMBS_PARAM_NTP);

      default:
         return (char * )"Param not defined";
   }

   return (char * )"Param not defined,yet\n";
}

//		========== getstr_E_CMBS_HSTYPE ===========
/*!
		\brief          return string of enumeration
		\param[in]		 e_Param		value of enumeration
		\return			 <char *>   return string of enumeration
*/
char *            getstr_E_CMBS_HSTYPE( E_CMBS_HS_TYPE e_HsType )
{
   switch ( e_HsType )
   {
      caseretstr(CMBS_HS_TYPE_GAP);                /*!< GAP handset */
      caseretstr(CMBS_HS_TYPE_CATIQ_1);            /*!< CATiq 1.0 compliant handset */
      caseretstr(CMBS_HS_TYPE_CATIQ_2);            /*!< CATiq 2.0 compliant handset */
      caseretstr(CMBS_HS_TYPE_DSPG);               /*!< DSPG handset */

      default:
         return (char * )"Handset type not defined";
   }
}

//		========== getidx_E_CMBS_PARAM ===========
/*!
		\brief         return enumeration value of string
		\param[in]		psz_Value         pointer enumeration string
		\return			<E_CMBS_PARAM>    enumeration value

*/
E_CMBS_PARAM      getidx_E_CMBS_PARAM( char * psz_Value )
{
   int i;

   for ( i=0; i< CMBS_PARAM_MAX; i++ )
      if ( !strcmp( psz_Value, getstr_E_CMBS_PARAM(i) ) )
         break;

  return i;
}

//		========== getstr_E_CMBS_TONE ===========
/*!
		\brief          return string of enumeration
		\param[in]		 e_Param		value of enumeration
		\return			 <char *>   return string of enumeration
*/
char *            getstr_E_CMBS_TONE( E_CMBS_TONE e_Tone )
{
   switch ( e_Tone )
   {
      caseretstr(CMBS_TONE_DIAL);                  /*!< Dial tone according country spec. */
      caseretstr(CMBS_TONE_STUTTER_DIAL);          /*!< Stutter dial tone according country spec. */
      caseretstr(CMBS_TONE_RING_BACK);             /*!< Ring-back tone according country spec. */
      caseretstr(CMBS_TONE_BUSY);                  /*!< Busy tone according country spec. */
      caseretstr(CMBS_TONE_CALL_WAITING);          /*!< Call waiting tone according country spec. */
      caseretstr(CMBS_TONE_HINT);                  /*!< Hint tone */
      caseretstr(CMBS_TONE_OK);                    /*!< OK tone */
      caseretstr(CMBS_TONE_NOK);                   /*!< Not OK tone */
      caseretstr(CMBS_TONE_DTMF_0);
      caseretstr(CMBS_TONE_DTMF_1);
      caseretstr(CMBS_TONE_DTMF_2);
      caseretstr(CMBS_TONE_DTMF_3);
      caseretstr(CMBS_TONE_DTMF_4);
      caseretstr(CMBS_TONE_DTMF_5);
      caseretstr(CMBS_TONE_DTMF_6);
      caseretstr(CMBS_TONE_DTMF_7);
      caseretstr(CMBS_TONE_DTMF_8);
      caseretstr(CMBS_TONE_DTMF_9);
      caseretstr(CMBS_TONE_DTMF_STAR);
      caseretstr(CMBS_TONE_DTMF_HASH);
      caseretstr(CMBS_TONE_DTMF_A);
      caseretstr(CMBS_TONE_DTMF_B);
      caseretstr(CMBS_TONE_DTMF_C);
      caseretstr(CMBS_TONE_DTMF_D);
   //CMBS_TONE_USER_DEF               /*!< User defined tone, not supported, yet. */
   default:
      return (char * )"Tone not defined";
   }

   return (char * )"Tone not defined,yet\n";
}

//		========== getidx_E_CMBS_TONE ===========
/*!
		\brief         return enumeration value of string
		\param[in]		psz_Value         pointer enumeration string
		\return			<E_CMBS_TONE>    enumeration value

*/
E_CMBS_TONE       getidx_E_CMBS_TONE( char * psz_Value )
{
   int i;

   for ( i=0; i< CMBS_TONE_USER_DEF; i++ )
      if ( !strcmp( psz_Value, getstr_E_CMBS_TONE(i) ) )
         break;
  return i;
}

//		========== getstr_CMBS_CALL_PROGR ===========
/*!
		\brief          return string of enumeration
		\param[in]		 e_Param		value of enumeration
		\return			 <char *>   return string of enumeration
*/
char *            getstr_CMBS_CALL_PROGR( E_CMBS_CALL_PROGRESS e_Prog )
{
   switch ( e_Prog )
   {
      caseretstr(CMBS_CALL_PROGR_UNDEF);
      caseretstr(CMBS_CALL_PROGR_SETUP_ACK);
      caseretstr(CMBS_CALL_PROGR_PROCEEDING);
      caseretstr(CMBS_CALL_PROGR_RINGING);
      caseretstr(CMBS_CALL_PROGR_BUSY);
      caseretstr(CMBS_CALL_PROGR_CALLWAITING);
      caseretstr(CMBS_CALL_PROGR_INBAND);
      default:
            return (char*)"Call Progress not defined";
     }

   return NULL;
 }

//		========== getidx_CMBS_CALL_PROGR ===========
/*!
		\brief         return enumeration value of string
		\param[in]		psz_Value         pointer enumeration string
		\return			<E_CMBS_CALL_PROGRESS>
                                       enumeration value

*/
E_CMBS_CALL_PROGRESS getidx_CMBS_CALL_PROGR( char * psz_Value )
{
   int   i;

   for ( i=0; i < CMBS_CALL_PROGR_MAX; i++ )
      if ( !strcmp( psz_Value, getstr_CMBS_CALL_PROGR(i)))
         break;
   return i;
}


//*/
