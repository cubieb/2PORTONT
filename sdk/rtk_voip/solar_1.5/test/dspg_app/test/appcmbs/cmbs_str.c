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

//		========== getstr_E_CMBS_PARAM ===========
/*!
		\brief          return string of enumeration
		\param[in]		 e_Param		value of enumeration
		\return			 <char *>   return string of enumeration
*/
char *               getstr_E_CMBS_PARAM( E_CMBS_PARAM e_Param )
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
      caseretstr(CMBS_PARAM_FLEX);

      default:
         return (char * )"Param not defined";
   }

   return (char * )"Param not defined,yet\n";
}

//		========== getidx_E_CMBS_PARAM ===========
/*!
		\brief         return enumeration value of string
		\param[in]		psz_Value         pointer enumeration string
		\return			<E_CMBS_PARAM>    enumeration value

*/
E_CMBS_PARAM         getidx_E_CMBS_PARAM( char * psz_Value )
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
char *               getstr_E_CMBS_TONE( E_CMBS_TONE e_Tone )
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
E_CMBS_TONE          getidx_E_CMBS_TONE( char * psz_Value )
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
char *               getstr_CMBS_CALL_PROGR( E_CMBS_CALL_PROGRESS e_Prog )
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
