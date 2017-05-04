/*!
    \brief Initialize the CMBS application

*/


#include <stdio.h>
#if ! defined ( WIN32 )
    #include <unistd.h>
    #include <termios.h>
    #include <string.h>
    #include <sys/time.h> //we need <sys/select.h>; should be included in <sys/types.h> ???
    #include <signal.h>
    #include <sys/msg.h>
    #include <errno.h>
#endif
#include "cmbs_platf.h"
#include "cmbs_api.h"
#include "cmbs_int.h"
#include "cfr_ie.h"
#include "cfr_mssg.h"
#include "appcmbs.h"
#include "appsrv.h"
#include "appcall.h"
#include "appcall.h"

// re-used from the CMBS API Module.
extern void              _cmbs_int_MsgQDestroy( int nMsgQId );
extern int               _cmbs_int_MsgQCreate( void );
extern u8                u8_APP_LA_Initialized;
extern void              app_LaInitialize(void);

/*!***************************************************************************
*
*	\brief CMBS application feedback entity
*
*****************************************************************************/

// different CMBS parts use different entities
// which will be called by callback function
// if the entity consumes the event, it shall return TRUE
typedef  int  (* PFN_APP_ENTITY_CB)( void * pv_AppRef,
                                     E_CMBS_EVENT_ID e_EventID,
                                     void * pv_EventIEListRef );

// global callback function table of received CMBS Events
PFN_APP_ENTITY_CB     g_pfn_app_Entity[] =
{
    (PFN_APP_ENTITY_CB)app_ServiceEntity,
    (PFN_APP_ENTITY_CB)app_CallEntity,
    (PFN_APP_ENTITY_CB)app_SwupEntity,
    NULL,//(PFN_APP_ENTITY_CB)app_MediaEntity - currently inside call entity
    (PFN_APP_ENTITY_CB)app_FacilityEntity,
    (PFN_APP_ENTITY_CB)app_DataEntity,
    (PFN_APP_ENTITY_CB)app_LaEntity
};
// global object of application
ST_CMBS_APPL    g_cmbsappl;

u8                g_HoldCfm = 1;       /* 0 - decline hold, 1 - accept hold */
u8                g_HoldResumeCfm = 1; /* 0 - decline hold resume, 1 - accept hold resume */
u8 g_TransferAutoCfm = 1;
u8 g_ConfAutoCfm = 1;
u8              g_EarlyMediaAutoCfm = 1;

//		========== appcmbs_CallBack ===========
/*!
        \brief		  registered callback function to CMBS API for received events
        \param[in]	  pv_AppRef		 application reference pointer
        \param[in]	  e_EventID		 received event ID
        \param[in]	  pv_EventData	 IE list pointer
        \return		  <none>
*/
void     appcmbs_CallBack( void * pv_AppRef, E_CMBS_EVENT_ID e_EventID, void * pv_EventData )
{
    unsigned int i;

    if ( pv_AppRef )
    {
    } // Just one callback function registered, so we don't have to evaluate pv_AppRef

    for ( i=0; i < (sizeof(g_pfn_app_Entity)/sizeof(PFN_APP_ENTITY_CB)); i++ )
    {
        if ( g_pfn_app_Entity[i] )
        {
            if ( g_pfn_app_Entity[i]( pv_AppRef, e_EventID, pv_EventData ) )
            {
                // no handler is needed anymore, event is consumed
                return;
            }
        }
    }

    APPCMBS_WARN (("APP_Dongle: WARN =============== callback event default handler ================\n"));
    APPCMBS_WARN((" Event: %d is not handled, yet\n", e_EventID ));
}

void		appcmbs_CordlessStart( u8 * pu8_EEprom, u16 u16_Size )
{
    UNUSED_PARAMETER(pu8_EEprom);
    UNUSED_PARAMETER(u16_Size);
/*
  u32 u32_offset = 0;
  u32 u32_buf_size;
  ST_APPCMBS_CONTAINER st_Container;

  APPCMBS_INFO(( "APP: INFO Push eeprom settings to target!\n"));

//  Sleep(1000);

  while (u16_Size)
  {
    if (u16_Size > CMBS_PARAM_AREA_MAX_SIZE)
    {
      u32_buf_size = CMBS_PARAM_AREA_MAX_SIZE;
    }
    else
  {
      u32_buf_size = u16_Size;
  }

    app_SrvParamAreaSet( CMBS_PARAM_AREA_TYPE_EEPROM, u32_offset, u32_buf_size, pu8_EEprom, 1 );
    appcmbs_WaitForContainer( CMBS_EV_DSR_PARAM_AREA_SET_RES, &st_Container );

    pu8_EEprom += u32_buf_size;
    u32_offset += u32_buf_size;
    u16_Size -= u32_buf_size;
  }
*/
   cmbs_dsr_sys_Start( g_cmbsappl.pv_CMBSRef );
}

//		========== 	appcmbs_Initialize ===========
/*!
        \brief				 short description
        \param[in,out]		 pv_AppReference		application pointer reference
        \return				<int>       return zero if successful
*/
int     appcmbs_Initialize( void * pv_AppReference, PST_CMBS_DEV pst_DevCtl, PST_CMBS_DEV pst_DevMedia )
{
    // reset application
    memset ( &g_cmbsappl, 0, sizeof(g_cmbsappl));
    g_cmbsappl.pv_ApplRef = pv_AppReference;

    // initialize application
    appcall_Initialize ();
    // intercommunication between engine and application
#ifdef __linux__
    g_cmbsappl.n_MssgAppID = _cmbs_int_MsgQCreate();
#elif WIN32
    // initialize without any timeout
    g_cmbsappl.pst_AppSyncEntry  = cfr_MQueueCreate ( (u32)GetCurrentThreadId(), 0 );
#endif
    // initialize CMBS API and register callback function
    // for test application
    if ( cmbs_api_Init( CMBS_MODE_MLB, pst_DevCtl, pst_DevMedia ) == CMBS_RESPONSE_OK )
    {
        APPCMBS_INFO(( "APP: INFO CMBS-API started with Module version %04x\n", cmbs_api_ModuleVersionGet()));

        if ( (g_cmbsappl.pv_CMBSRef = cmbs_api_RegisterCb( &g_cmbsappl, (PFN_CMBS_API_CB)appcmbs_CallBack, 0x0100 )) == NULL )
        {
            APPCMBS_WARN(( "APP: WARN Can't register API callback function\n" ));
        }
    }
    else
    {
        APPCMBS_ERROR(( "APP: ERROR !!! Initialization failed\n" ));

        return -1;
    }

    if ( !u8_APP_LA_Initialized )
    {
        app_LaInitialize();
        u8_APP_LA_Initialized = 1;
    }

    return 0;
}
//		========== appcmbs_Cleanup ===========
/*!
        \brief		 clean up CMBS API layer and CMBS API

        \param[in]	 <none>
        \return		 <none>

*/

void     appcmbs_Cleanup(void)
{
    APPCMBS_INFO( ("APP: INFO Cleanup CMBS connectivity\n" ));
    cmbs_api_UnInit();
#ifdef __linux
    _cmbs_int_MsgQDestroy(g_cmbsappl.n_MssgAppID);
#elif WIN32
    cfr_MQueueDestroy( g_cmbsappl.pst_AppSyncEntry );
#endif
}
//		========== app_ResponseCheck ===========
/*!
        \brief		check the IE response item and return response code
        \param[in]  pv_List	IE list pointer
        \return		<s8>  return response code TRUE/FALSE or -1 in case not finding IE
*/
s8       app_ResponseCheck( void * pv_List )
{
   void *   pv_IE;
   u16      u16_IE;

   cmbs_api_ie_GetFirst( pv_List, &pv_IE, &u16_IE );

   while( pv_IE != NULL )
   {
   if( u16_IE == CMBS_IE_RESPONSE )
   {
      ST_IE_RESPONSE st_Response;
                                 // check response code:
      cmbs_api_ie_ResponseGet( pv_IE, &st_Response );
      return st_Response.e_Response;
   }
      cmbs_api_ie_GetNext( pv_List, &pv_IE, &u16_IE );
   }

   return -1;
}
//		========== app_IEToString ===========
/*!
        \brief	   	 IE string print out
        \param[in]		 pv_IE		 IE buffer
        \param[in]		 u16_IE		 IE
        \return			 <none>

*/

void     app_IEToString( void * pv_IE, u16 u16_IE )
{
    switch ( u16_IE )
    {
    case  CMBS_IE_CALLINSTANCE:
        {
            u32            u32_CallInstance = 0;

            cmbs_api_ie_CallInstanceGet( pv_IE, &u32_CallInstance );
            APPCMBS_IE(( "APP: IETRACK CallInstance: %X\n", u32_CallInstance ));

            break;
        }

    case  CMBS_IE_LINE_ID:
        {
            u8             u8_LineId;

            cmbs_api_ie_LineIdGet( pv_IE, &u8_LineId );
            APPCMBS_IE(( "APP: IETRACK LineId: %d\n", u8_LineId ));

            break;
        }

    case  CMBS_IE_CALLEDPARTY:
        {
            ST_IE_CALLEDPARTY  u8_CalledParty;

            cmbs_api_ie_CalledPartyGet( pv_IE, &u8_CalledParty );
            APPCMBS_IE(( "APP: IETRACK CalledParty: %c\n", u8_CalledParty.pu8_Address[0]));

            break;
        }

    case  CMBS_IE_CALLPROGRESS:
        {
            ST_IE_CALLPROGRESS st_CallProgress;

            cmbs_api_ie_CallProgressGet( pv_IE, &st_CallProgress );

            app_PrintCallProgrInfo( st_CallProgress.e_Progress );

            break;
        }

    case  CMBS_IE_CALLINFO:
        {
            ST_IE_CALLINFO st_CallInfo;

            cmbs_api_ie_CallInfoGet( pv_IE, &st_CallInfo );

            APPCMBS_IE(( "APP: IETRACK CallInfo Type: "));
            app_PrintCallInfoType( st_CallInfo.e_Type );

            if ( st_CallInfo.e_Type == CMBS_CALL_INFO_TYPE_DIGIT )
            {
                u8 i;

                APPCMBS_IE(( ": \"" ));
                for ( i=0; i<st_CallInfo.u8_DataLen; i++ )
                {
                    APPCMBS_IE(( "%c", st_CallInfo.pu8_Info[i] ));
                }
                APPCMBS_IE(( "\" Lenght:%2d\n", st_CallInfo.u8_DataLen ));
            }

            break;
        }

    case  CMBS_IE_MEDIACHANNEL:
        {
            ST_IE_MEDIA_CHANNEL  st_MediaChannel;

            cmbs_api_ie_MediaChannelGet( pv_IE, &st_MediaChannel );

            APPCMBS_IE(( "APP: IETRACK Media Type: " ));
            switch ( st_MediaChannel.e_Type )
            {
            case  CMBS_MEDIA_TYPE_AUDIO_IOM: APPCMBS_IE(( "AUDIO_IOM" ));  break;
            case  CMBS_MEDIA_TYPE_AUDIO_USB: APPCMBS_IE(( "AUDIO_USB" ));  break;
            case  CMBS_MEDIA_TYPE_DATA:      APPCMBS_IE(( "DATA" ));       break;
            default:                         APPCMBS_IE(( "UNKNOWN" ));
            }

            APPCMBS_IE(( " Channel:%d ChannelParam:%x\n",
                         st_MediaChannel.u32_ChannelID, st_MediaChannel.u32_ChannelParameter ));

            break;
        }

    case  CMBS_IE_MEDIADESCRIPTOR:
        {
            ST_IE_MEDIA_DESCRIPTOR  st_MediaDesc;

            cmbs_api_ie_MediaDescGet( pv_IE, &st_MediaDesc );

            APPCMBS_IE(( "APP: IETRACK Media Descriptor: Codec: " ));
            switch ( st_MediaDesc.e_Codec )
            {
            case CMBS_AUDIO_CODEC_PCMU:            APPCMBS_IE(( "PCMU"));     break;
            case CMBS_AUDIO_CODEC_PCMA:            APPCMBS_IE(( "PCMA"));     break;
            case CMBS_AUDIO_CODEC_PCMU_WB:         APPCMBS_IE(( "PCMU_WB"));  break;
            case CMBS_AUDIO_CODEC_PCMA_WB:         APPCMBS_IE(( "PCMA_WB"));  break;
            case CMBS_AUDIO_CODEC_PCM_LINEAR_WB:   APPCMBS_IE(( "PCM_LINEAR_WB"));  break;
            case CMBS_AUDIO_CODEC_PCM_LINEAR_NB:   APPCMBS_IE(( "PCM_LINEAR_NB"));  break;
            case CMBS_AUDIO_CODEC_PCM8:            APPCMBS_IE(( "PCM8"));     break;
            default:                               APPCMBS_IE(( "UNKNOWN" ));
            }
            APPCMBS_IE(( "\n" ));
            break;
        }

    case  CMBS_IE_CALLRELEASE_REASON:
        {
            ST_IE_RELEASE_REASON st_Reason;

            cmbs_api_ie_CallReleaseReasonGet( pv_IE, &st_Reason );

            APPCMBS_IE(( "APP: IETRACK Reason: " ));
            switch ( st_Reason.e_Reason )
            {
            case CMBS_REL_REASON_NORMAL:           APPCMBS_IE(( "NORMAL"));          break;
            case CMBS_REL_REASON_ABNORMAL:         APPCMBS_IE(( "ABNORMAL"));        break;
            case CMBS_REL_REASON_BUSY:             APPCMBS_IE(( "BUSY"));            break;
            case CMBS_REL_REASON_UNKNOWN_NUMBER:   APPCMBS_IE(( "UNKNOWN_NUMBER"));  break;
            case CMBS_REL_REASON_FORBIDDEN:        APPCMBS_IE(( "FORBIDDEN"));       break;
            case CMBS_REL_REASON_UNSUPPORTED_MEDIA:APPCMBS_IE(( "UNSUPPORTED_MEDIA"));break;
            case CMBS_REL_REASON_NO_RESOURCE:      APPCMBS_IE(( "NO_RESOURCE"));     break;
            default:                               APPCMBS_IE(( "UNKNOWN" ));
            }
            APPCMBS_IE(( "\n" ));
            break;
        }

    case  CMBS_IE_PARAMETER:
        {
            ST_IE_PARAMETER st_Param;
            int i;

            cmbs_api_ie_ParameterGet( pv_IE, &st_Param );

            switch ( st_Param.e_Param )
            {
            case CMBS_PARAM_RFPI:         APPCMBS_IE(( "RFPI"));         break;
            case CMBS_PARAM_RVBG:         APPCMBS_IE(( "RVBG"));         break;
            case CMBS_PARAM_RVREF:        APPCMBS_IE(( "RVREF"));        break;
            case CMBS_PARAM_RXTUN:        APPCMBS_IE(( "RXTUN"));        break;
            case CMBS_PARAM_MASTER_PIN:   APPCMBS_IE(( "MASTER_PIN"));   break;
            case CMBS_PARAM_AUTH_PIN:     APPCMBS_IE(( "AUTH_PIN"));     break;
            case CMBS_PARAM_COUNTRY:      APPCMBS_IE(( "COUNTY"));       break;
            case CMBS_PARAM_TEST_MODE:    APPCMBS_IE(( "TEST_MODE"));    break;
            case CMBS_PARAM_ECO_MODE:     APPCMBS_IE(( "ECO_MODE"));     break;
            case CMBS_PARAM_AUTO_REGISTER:   APPCMBS_IE(( "AUTO_REGISTER"));  break;
            case CMBS_PARAM_NTP:             APPCMBS_IE(( "NTP"));            break;
            case CMBS_PARAM_RESET_ALL:    APPCMBS_IE(( "RESET_ALL"));    break;

            default:                      APPCMBS_IE(( "UNKNOWN PARAM:%d\n", st_Param.e_Param ));
         }

         if( st_Param.u16_DataLen > 0 )
         {
            APPCMBS_IE((": "));
            for( i=0; i< st_Param.u16_DataLen; i++ )
               APPCMBS_IE(( "%02X ", st_Param.pu8_Data[i]));
            APPCMBS_IE(("\n"));
         }
      }
      break;

      case  CMBS_IE_PARAMETER_AREA:
      {
         ST_IE_PARAMETER_AREA st_ParamArea;
         int i;

         cmbs_api_ie_ParameterAreaGet( pv_IE, &st_ParamArea );
         
         APPCMBS_IE(("Param_Area=%d, Offset=%d, Length=%d", st_ParamArea.e_AreaType, st_ParamArea.u32_Offset, st_ParamArea.u16_DataLen ));

         if( st_ParamArea.u16_DataLen > 0 )
         {
            APPCMBS_IE((", Data: "));
            for( i=0; i < st_ParamArea.u16_DataLen; i++ )
               APPCMBS_IE(( "%02X ", st_ParamArea.pu8_Data[i]));
         }
            APPCMBS_IE(("\n"));
         }
      break;
      
      case  CMBS_IE_FW_VERSION:
      {
         ST_IE_FW_VERSION st_FwVersion;

         cmbs_api_ie_FwVersionGet( pv_IE, &st_FwVersion );

         switch ( st_FwVersion.e_SwModule )
         {
            case CMBS_MODULE_CMBS:        APPCMBS_IE(( "CMBS Module " ));     break;
            case CMBS_MODULE_DECT:        APPCMBS_IE(( "DECT Module " ));     break;
            case CMBS_MODULE_DSP:         APPCMBS_IE(( "DSP Module " ));      break;
            case CMBS_MODULE_EEPROM:      APPCMBS_IE(( "EEPROM Module " ));   break;
            case CMBS_MODULE_USB:         APPCMBS_IE(( "USB Module " ));      break;
            default:                      APPCMBS_IE(( "UNKNOWN Module " ));
            }
            APPCMBS_IE(( "VER_%04x\n", st_FwVersion.u16_FwVersion ));
        }
        break;

    case  CMBS_IE_SYS_LOG:
        {
            ST_IE_SYS_LOG st_SysLog;
            u8  u8_DataLen;
            u8 *pu8_Data;
            u8  i;
            u8  c;

            cmbs_api_ie_SysLogGet( pv_IE, &st_SysLog );

            u8_DataLen = st_SysLog.u8_DataLen;
            pu8_Data   = st_SysLog.u8_Data;

            if ( !u8_DataLen )
            {
                char psz_Text[] = "LogBuffer is empty\n";
                printf("%s", psz_Text);

                if ( g_CMBSInstance.st_ApplSlot.pFnCbLogBuffer.pfn_cmbs_api_log_write_log_buffer_cb != NULL )
                {
                    g_CMBSInstance.st_ApplSlot.pFnCbLogBuffer.pfn_cmbs_api_log_write_log_buffer_cb((u8 *)psz_Text, sizeof(psz_Text) - 1);
                }
            }
            else
            {
                printf("LogBuffer:\n\n");
                for ( i = 0; i < u8_DataLen; i++ )
                {
                    c = pu8_Data[i];
                    if ( c >= 0x20 && c <= 0x7e ) printf("%c", c);
                    else if ( c == 0x0a )         printf("\n");
                    else                         printf("(%02X)", c);
                }
                printf("\n");

                if ( g_CMBSInstance.st_ApplSlot.pFnCbLogBuffer.pfn_cmbs_api_log_write_log_buffer_cb != NULL )
                {
                    g_CMBSInstance.st_ApplSlot.pFnCbLogBuffer.pfn_cmbs_api_log_write_log_buffer_cb(pu8_Data, (u16)u8_DataLen);
                }
            }

            break;
        }

    case  CMBS_IE_CALLSTATE:
        APPCMBS_IE(( "APP: IETRACK Call State received\n")) ;
        break;
    case  CMBS_IE_RESPONSE:
        {
            ST_IE_RESPONSE st_Resp;

            cmbs_api_ie_ResponseGet( pv_IE, &st_Resp );

            APPCMBS_IE(( "APP: IETRACK Response: %s\n", st_Resp.e_Response == CMBS_RESPONSE_OK ? "OK":"ERROR")) ;
        }
        break;

    case CMBS_IE_SUBSCRIBED_HS_LIST:
        break;

    case CMBS_IE_LINE_SETTINGS_LIST:
        break;

    default:
        if ( CMBS_IE_USER_DEFINED_START <= u16_IE &&  u16_IE <= CMBS_IE_USER_DEFINED_END )
        {
            APPCMBS_IE(( "CMBS_IE_USER_DEFINED\n" )) ;
            break;
        }
        APPCMBS_IE(( "IE_ToString: IE:%d not implemented\n", u16_IE ));
    }
}
//		========== appcmbs_PrepareRecvAdd  ===========
/*!
        \brief		set token to pass received infromation to upper layer
        \param[in] 	u32_Token		 TRUE for passing
        \return	  	<none>

*/

void     appcmbs_PrepareRecvAdd ( u32 u32_Token )
{
    g_cmbsappl.n_Token = (int)u32_Token;
}

//		========== appcmbs_WaitForContainer ===========
/*!
        \brief			 Synchronization function enables the application to wait until
                      requested CMBS event was received.
        \param[in]		 n_Event          wait for this CMBS event
        \param[in,out]	 pst_Container		pointer to CMBS information contatiner
        \return			 <int>            return TRUE, if received, otherwise FALSE on error
      \todo           timeout handling has to be integrated
*/

int      appcmbs_WaitForContainer ( int n_Event, PST_APPCMBS_CONTAINER pst_Container )
{
#ifdef __linux__
    int   nRetVal;
    int   bo_Run = TRUE;
    ST_APPCMBS_LINUX_CONTAINER LinuxContainer;

    while ( bo_Run )
    {
        nRetVal = msgrcv( g_cmbsappl.n_MssgAppID, &LinuxContainer, sizeof(ST_APPCMBS_CONTAINER), 0, 0);
        if ( nRetVal == -1 )
            bo_Run = FALSE;

        if ( n_Event == LinuxContainer.Content.n_Event )
        {
            memcpy( pst_Container, &LinuxContainer.Content, sizeof(ST_APPCMBS_CONTAINER));
            g_cmbsappl.n_Token = 0;
            return TRUE;
        }

        // timeout handler
    }

    g_cmbsappl.n_Token = 0;
#elif WIN32
    u32  u32_MssgID;
    u16  u16_ParamSize;
    void * pv_Param;
    int   bo_Run = TRUE;

    while ( bo_Run )
    {
        if ( E_CFR_INTERPROCESS_MSSG  == cfr_MQueueGet( g_cmbsappl.pst_AppSyncEntry , &u32_MssgID, &pv_Param, &u16_ParamSize ) )
        {
            if ( u32_MssgID == CFR_CMBS_MSSG_APPSYNC )
            {
                PST_APPCMBS_LINUX_CONTAINER pstLinuxContainer = (PST_APPCMBS_LINUX_CONTAINER) pv_Param ;

                if ( n_Event == pstLinuxContainer->Content.n_Event )
                {
                    memcpy ( pst_Container, &pstLinuxContainer->Content, sizeof(ST_APPCMBS_CONTAINER) );
                    g_cmbsappl.n_Token = 0;

                    cfr_MQueueMssgFree( g_cmbsappl.pst_AppSyncEntry , pv_Param );
                    return TRUE;
                }
                else
                {
                    cfr_MQueueMssgFree( g_cmbsappl.pst_AppSyncEntry , pv_Param );
                }
            }
            else
            {
                // later check also for destroy message
                cfr_MQueueMssgFree( g_cmbsappl.pst_AppSyncEntry , pv_Param );
                return TRUE;
            }
        }
    }
#endif // __linux__
    return FALSE;
}

//		========== appcmbs_ObjectSignal ===========
/*!
        \brief          signal to application of received event
        \param[in]		 psz_Info      pointer to information buffer
        \param[in]		 n_InfoLen		length of information
        \param[in]		 n_Info		   additional information, e.g. IE element
        \param[in]		 n_Event		   received CMBS event
        \return			<none>
*/
void     appcmbs_ObjectSignal( char * psz_Info, int n_InfoLen, int n_Info, int n_Event )
{
#ifdef __linux__
    int   nRetVal;

    if ( g_cmbsappl.n_MssgAppID >= 0 )
    {
        ST_APPCMBS_LINUX_CONTAINER LinuxContainer;

        LinuxContainer.mType = 1;
        memcpy( LinuxContainer.Content.ch_Info, psz_Info,n_InfoLen );
        LinuxContainer.Content.n_InfoLen = n_InfoLen;
        LinuxContainer.Content.n_Event= n_Event;
        LinuxContainer.Content.n_Info = n_Info;

        nRetVal = msgsnd( g_cmbsappl.n_MssgAppID , &LinuxContainer, sizeof(ST_APPCMBS_CONTAINER), 0 );

        if ( nRetVal == -1 )
        {
            APPCMBS_ERROR(( "App:ERROR !!! Container Object was not sent! %d\n",errno ));
        }
    }
#elif WIN32
    if ( g_cmbsappl.pst_AppSyncEntry )
    {
        PST_APPCMBS_LINUX_CONTAINER pst_Container;

        pst_Container = (PST_APPCMBS_LINUX_CONTAINER)malloc( sizeof(ST_APPCMBS_LINUX_CONTAINER) );

        if ( pst_Container )
        {
            memset( pst_Container, 0, sizeof(ST_APPCMBS_LINUX_CONTAINER) );

            if ( psz_Info )
            {
                memcpy( pst_Container->Content.ch_Info, psz_Info, n_InfoLen );
            }

            pst_Container->mType = 1;
            pst_Container->Content.n_InfoLen = n_InfoLen;
            pst_Container->Content.n_Event= n_Event;
            pst_Container->Content.n_Info = n_Info;

            cfr_MQueueSend( g_cmbsappl.pst_AppSyncEntry, CFR_CMBS_MSSG_APPSYNC, (void *) pst_Container, sizeof(ST_APPCMBS_LINUX_CONTAINER) );
        }
    }

#endif // __linux__
}
//		========== appcmbs_IEInfoGet  ===========
/*!
        \brief				handles general to IE function
        \param[in]		   pv_IE		 IE  buffer
        \param[in]		   u16_IE	 enumeration of IE
        \param[in,out]		p_Info	 pointer to IE object
        \return				<none>
*/
void     appcmbs_IEInfoGet ( void * pv_IE, u16 u16_IE, PST_APPCMBS_IEINFO p_Info )
{
    switch ( u16_IE )
    {
    case  CMBS_IE_CALLERNAME:
        cmbs_api_ie_CallerNameGet( pv_IE, &p_Info->Info.st_CallerName );
        break;

    case  CMBS_IE_CALLERPARTY:
        cmbs_api_ie_CallerPartyGet( pv_IE, &p_Info->Info.st_CallerParty );
        break;
    case  CMBS_IE_CALLEDPARTY:
        cmbs_api_ie_CalledPartyGet( pv_IE, &p_Info->Info.st_CalledParty );
        break;

    case  CMBS_IE_CALLINSTANCE:
        cmbs_api_ie_CallInstanceGet( pv_IE, &p_Info->Info.u32_CallInstance );
        break;

    case  CMBS_IE_CALLPROGRESS:
        cmbs_api_ie_CallProgressGet( pv_IE, &p_Info->Info.st_CallProgress );
        break;

    case  CMBS_IE_CALLINFO:
        cmbs_api_ie_CallInfoGet( pv_IE, &p_Info->Info.st_CallInfo );
        break;

    case  CMBS_IE_MEDIACHANNEL:
        cmbs_api_ie_MediaChannelGet( pv_IE, &p_Info->Info.st_MediaChannel );
        break;

    case  CMBS_IE_MEDIADESCRIPTOR:
        cmbs_api_ie_MediaDescGet( pv_IE, &p_Info->Info.st_MediaDesc );
        break;

    case  CMBS_IE_CALLRELEASE_REASON:
        cmbs_api_ie_CallReleaseReasonGet( pv_IE, &p_Info->Info.st_Reason );
        break;

    case  CMBS_IE_PARAMETER:
        cmbs_api_ie_ParameterGet( pv_IE, &p_Info->Info.st_Param );
        break;

    case  CMBS_IE_FW_VERSION:
        cmbs_api_ie_FwVersionGet( pv_IE, &p_Info->Info.st_FwVersion );
        break;

    case  CMBS_IE_SYS_LOG:
        cmbs_api_ie_SysLogGet( pv_IE, &p_Info->Info.st_SysLog );
        break;

    case  CMBS_IE_SUBSCRIBED_HS_LIST:
        cmbs_api_ie_SubscribedHSListGet( pv_IE, &p_Info->Info.st_SubscribedHsList );
        break;

    case CMBS_IE_LINE_SETTINGS_LIST:
        cmbs_api_ie_LineSettingsListGet( pv_IE, &p_Info->Info.st_LineSettingsList );
        break;

    case  CMBS_IE_RESPONSE:
        cmbs_api_ie_ResponseGet( pv_IE, &p_Info->Info.st_Resp);
        break;

    case  CMBS_IE_LINE_ID:
        cmbs_api_ie_LineIdGet( pv_IE, &p_Info->Info.u8_LineId );
        break;

    default:
        if ( CMBS_IE_USER_DEFINED_START <= u16_IE &&  u16_IE <= CMBS_IE_USER_DEFINED_END )
        {
            break;
        }
        APPCMBS_WARN(( "APP: WARN IE_ToString: IE:%d not implemented\n", u16_IE ));
    }

}

extern u16 cmbs_int_ModuleVersionBuildGet(void );

void     appcmbs_VersionGet( char * pc_Version )
{
    u16 u16_Version = cmbs_api_ModuleVersionGet();
    u16 u16_Build   = cmbs_int_ModuleVersionBuildGet( );

    sprintf( pc_Version, "Version %02x.%02x - Build %x", (u16_Version>>8),(u16_Version &0xFF)
             , u16_Build );
}
//*/
