/*!
*  \file       appfacility.c
*	\brief		handles CAT-iq facilities functioality
*	\Author		stein
*
*	@(#)	%filespec: appfacility.c~4 %
*
*******************************************************************************
*	\par	History
*	\n==== History ============================================================\n
*	date			name		version	 action                                          \n
*	----------------------------------------------------------------------------\n
*
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if ! defined ( WIN32 )
    #include <unistd.h>
    #include <fcntl.h>
    #include <termios.h>
    #include <sys/time.h>
    #include <signal.h>
#endif

#include "cmbs_api.h"
#include "cfr_ie.h"
#include "cfr_mssg.h"
#include "cmbs_str.h"

#include "appcmbs.h"

extern u16        app_HandsetMap( char * psz_Handsets );

//		========== app_FacilityVMWI  ===========
/*!
        \brief				 sending Voice Message Waiting Indication

        \param[in,out]		 psz_Handsets     pointer to parameter string,e.g."1234" or "all"

        \return				 <E_CMBS_RC>

*/
E_CMBS_RC         app_FacilityVMWI( u16 u16_RequestId, u8 u8_LineId, u8 u8_Messages, char * psz_Handsets )
{
    u16            u16_Handsets = app_HandsetMap( psz_Handsets );

    return cmbs_dsr_gen_SendVMWI( g_cmbsappl.pv_CMBSRef,
                                  u16_RequestId,
                                  u8_LineId,
                                  u16_Handsets,
                                  u8_Messages );
}


E_CMBS_RC app_FacilityMissedCalls( u16 u16_RequestId, u8 u8_LineId, u8 u8_NewMissedCalls, char * psz_Handsets, bool bNewMissedCall, u8 u8_TotalMissedCalls )
{
    u16 u16_Handsets = app_HandsetMap( psz_Handsets );

    return cmbs_dsr_gen_SendMissedCalls( g_cmbsappl.pv_CMBSRef,
                                         u16_RequestId,
                                         u8_LineId,
                                         u16_Handsets,
                                         u8_NewMissedCalls,
                                         bNewMissedCall,
                                         u8_TotalMissedCalls );
}


E_CMBS_RC app_FacilityListChanged( u16 u16_RequestId, u8 u8_ListId, u8 u8_ListEntries, char * psz_Handsets, u8 u8_LineId, u8 u8_LineSubtype )
{
    u16 u16_Handsets = app_HandsetMap( psz_Handsets );

    return cmbs_dsr_gen_SendListChanged( g_cmbsappl.pv_CMBSRef,
                                         u16_RequestId,
                                         u16_Handsets,
                                         u8_ListId,
                                         u8_ListEntries,
                                         u8_LineId,
                                         u8_LineSubtype );
}


E_CMBS_RC         app_FacilityWebContent( u16 u16_RequestId, u8 u8_NumOfWebCont, char * psz_Handsets )
{
    u16            u16_Handsets = app_HandsetMap( psz_Handsets );

    return cmbs_dsr_gen_SendWebContent( g_cmbsappl.pv_CMBSRef,
                                        u16_RequestId,
                                        u16_Handsets,
                                        u8_NumOfWebCont );
}


E_CMBS_RC         app_FacilityPropEvent( u16 u16_RequestId, u16 u16_PropEvent, u8 * pu8_Data, u8 u8_DataLen, char * psz_Handsets )
{
    u16            u16_Handsets = app_HandsetMap( psz_Handsets );

    return cmbs_dsr_gen_SendPropEvent( g_cmbsappl.pv_CMBSRef,
                                       u16_RequestId,
                                       u16_PropEvent,
                                       pu8_Data,
                                       u8_DataLen,
                                       u16_Handsets );
}


E_CMBS_RC         app_FacilityDateTime( u16 u16_RequestId, ST_DATE_TIME * pst_DateTime, char * psz_Handsets )
{
    u16            u16_Handsets = app_HandsetMap( psz_Handsets );

    return cmbs_dsr_time_Update( g_cmbsappl.pv_CMBSRef,
                                 u16_RequestId,
                                 pst_DateTime,
                                 u16_Handsets );
}


//		========== app_FacilityEntity ===========
/*!
        \brief		 CMBS entity to handle response information from target side
        \param[in]	 pv_AppRef		 application reference
        \param[in]	 e_EventID		 received CMBS event
        \param[in]	 pv_EventData	 pointer to IE list
        \return	 	 <int>

*/

int               app_FacilityEntity( void * pv_AppRef, E_CMBS_EVENT_ID e_EventID, void * pv_EventData )
{
    void *         pv_IE = NULL;
    u16            u16_IE;
    u16            u16_RequestId, u16_Handsets; 

    if ( pv_AppRef )
    {
    }; // eliminate compiler warning

    if ( e_EventID == CMBS_EV_DSR_GEN_SEND_VMWI_RES         ||
         e_EventID == CMBS_EV_DSR_GEN_SEND_MISSED_CALLS_RES ||
         e_EventID == CMBS_EV_DSR_GEN_SEND_LIST_CHANGED_RES ||
         e_EventID == CMBS_EV_DSR_GEN_SEND_WEB_CONTENT_RES  ||
         e_EventID == CMBS_EV_DSR_GEN_SEND_PROP_EVENT_RES   ||
         e_EventID == CMBS_EV_DSR_TIME_UPDATE_RES )
    {
        cmbs_api_ie_GetFirst( pv_EventData, &pv_IE, &u16_IE );
        while ( pv_IE != NULL )
        {
            printf( "%s: ", getstr_E_CMBS_IE( u16_IE ) );

            if ( u16_IE == CMBS_IE_REQUEST_ID )
            {
                cmbs_api_ie_RequestIdGet( pv_IE, &u16_RequestId );
                printf( "%d", u16_RequestId );
            }

            if ( u16_IE == CMBS_IE_HANDSETS )
            {
                cmbs_api_ie_HandsetsGet( pv_IE, &u16_Handsets );
                printf( "0x%04X", u16_Handsets );

                if ( u16_Handsets != 0 )
                {
                    printf( " Warning! Not all handsets responded" );
                }
            }

            if ( u16_IE == CMBS_IE_RESPONSE )
            {
                ST_IE_RESPONSE st_Response;
                cmbs_api_ie_ResponseGet( pv_IE, &st_Response );
                printf( "%s", st_Response.e_Response == CMBS_RESPONSE_OK ? "Ok":"Error" );
            }

            printf( "\n" );

            cmbs_api_ie_GetNext( pv_EventData, &pv_IE, &u16_IE );
        }

        return TRUE;
    }
    else
        return FALSE;

}
//*/
