#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "cmbs_platf.h"
#include "cmbs_api.h"
#include "cfr_ie.h"
#include "cfr_mssg.h"
#include "appcmbs.h"
#include "appsrv.h"
#include "appcall.h"
#include "cmbs_str.h"
#include "dect_test_scenario.h"

#ifdef __linux__
#define Sleep(x) usleep(1000*x)
#endif

/* THLin: global num to record if call obj is available or not,
 * 0xFF means call obj is unavailable, else value means call obj is available.
 * The same function with the g_n_CallNumber in keyb_call.c
 */
u16 g_u16_DemoCallId = APPCALL_NO_CALL;

static ST_CALLID_ANSWER CallId_Answer[APPCALL_CALLOBJ_MAX];//Thlin add

extern ST_CALL_OBJ g_call_obj[APPCALL_CALLOBJ_MAX];

void CMBS_Api_RTK_init(void)
{
	int i;
	
	for(i=0; i < APPCALL_CALLOBJ_MAX; i++)
	{
		CallId_Answer[i].u16_LineId = 0;
		CallId_Answer[i].bCallId_NeedAnswer = 0;
		CallId_Answer[i].u16_CallId = 0;
		CallId_Answer[i].u16_HsId = 0;
	}
}

/* Note for outgoing call:(Thlin add)
 * Stage 1:
 * When HS offhook, CMBS RX-thread receive "establish event", 
 * event's call-back function will send "ack" to CMBS,
 * then CMBS send "media update" event to Host.
 * Stage 2:
 * Main thread check HS offhook, then "Answer the call(including change codec)"
 * Deatil see DSPG API docs, Figure- Outgoing WB call, establish as NB(codec switch)
*/

void CMBS_Api_Set_Need_CallId_Answer(u16 lined_id, u16 call_id, u16 hs_id)
{
	CallId_Answer[call_id].u16_LineId = lined_id;
	CallId_Answer[call_id].bCallId_NeedAnswer = 1;
	CallId_Answer[call_id].u16_CallId = call_id;
	CallId_Answer[call_id].u16_HsId = hs_id;
	
	RTK_DBG("%s, line_id = %d, call_id = %d, hs_id = %d\n", __FUNCTION__, lined_id, call_id, hs_id);
}

u16 CMBS_Api_Get_Need_Answer_CallId(u16 line_id)
{
	int i;
	int hs;
	
	RTK_DBG("%s, line_id = %d\n", __FUNCTION__, line_id);
	
	for(i=0; i < APPCALL_CALLOBJ_MAX; i++)
	{
#if 0
		RTK_DBG("------------------------------\n");
		RTK_DBG("CallId_Answer[%d].u16_LineId = %d\n", i, CallId_Answer[i].u16_LineId);
		RTK_DBG("CallId_Answer[%d].bCallId_NeedAnswer = %d\n", i, CallId_Answer[i].bCallId_NeedAnswer);
		RTK_DBG("CallId_Answer[%d].u16_CallId = %d\n", i, CallId_Answer[i].u16_CallId);
		RTK_DBG("CallId_Answer[%d].u16_HsId = %d\n", i, CallId_Answer[i].u16_HsId);
		RTK_DBG("------------------------------\n");
#endif
		if (line_id == CallId_Answer[i].u16_LineId)
		{
			if (CallId_Answer[i].bCallId_NeedAnswer == 1)
			{
				hs = rtk_GetLineOccupyHS(line_id);
				
				//if ((hs != (-1)) && (hs != CallId_Answer[i].hs_id))
				if ( hs != CallId_Answer[i].u16_HsId )
				{
					CMBS_Api_CallRelease_Busy(CallId_Answer[i].u16_CallId);
					CallId_Answer[i].bCallId_NeedAnswer = 0; //clear flag
					dect_event_in(line_id, CallId_Answer[i].u16_HsId, 0); //put on-hook event for released HS to update status RTK maintain.
					return 0xFF;
				}
				else
				{
					CallId_Answer[i].bCallId_NeedAnswer = 0; //clear flag
					return CallId_Answer[i].u16_CallId;
				}
			}			
		}
	}
	
	return 0xFF; //No need answer call id
}

void  CMBS_Api_ReleaseNotify( u16 u16_CallId )
{
   if ( u16_CallId == g_u16_DemoCallId )
   {
      g_u16_DemoCallId = APPCALL_NO_CALL;
   }
}

/*====== Get CMBS Module Version and Build NO. ========*/

void CMBS_Api_FwVersionGet( u16* pVersion, u16* pVersionBuild )
{
	// Note: this API's version is got from HELLO Replay.
	
	if ( (pVersion == NULL) || (pVersionBuild == NULL) )
	{
		printf("CMBS_VersionGet fail!\n");
		return;
	}
	
	*pVersion = cmbs_api_ModuleVersionGet();
	*pVersionBuild = cmbs_int_ModuleVersionBuildGet( );
   
	printf("Get CMBS Version %02x.%02x - Build %x\n", ((*pVersion)>>8), ((*pVersion)&0xFF), *pVersionBuild);
}

void CMBS_Api_FwVersionGet2( void )
{
   ST_APPCMBS_CONTAINER st_Container;
   PST_IE_FW_VERSION  p_IE;

   app_SrvFWVersionGet( TRUE );

   appcmbs_WaitForContainer( CMBS_EV_DSR_FW_VERSION_GET_RES, &st_Container );

   p_IE = (PST_IE_FW_VERSION)st_Container.ch_Info;

   printf( "FIRMWARE Version:\n " );
   switch( p_IE->e_SwModule )
   {
      case CMBS_MODULE_CMBS:        printf( "CMBS Module " );     break;
      case CMBS_MODULE_DECT:        printf( "DECT Module " );     break;
      case CMBS_MODULE_DSP:         printf( "DSP Module " );      break;
      case CMBS_MODULE_EEPROM:      printf( "EEPROM Module " );   break;
      case CMBS_MODULE_USB:         printf( "USB Module " );      break;
      default:                      printf( "UNKNOWN Module " );
   }
   printf( "VER_%04x\n", p_IE->u16_FwVersion );

}

/*====== Open/Close CMBS Module Registration Mode ========*/

E_CMBS_RC CMBS_Api_RegistrationOpen()
{
	return app_SrvSubscriptionOpen();
}

E_CMBS_RC CMBS_Api_RegistrationClose()
{
	return app_SrvSubscriptionClose();
}

/*====== CMBS Module Handset Page ========*/

#if 1
/* hs_mask: string of handset you want to page */
/* For example: 
	"none" - don't page any handset
	"all" - page all handset
	"1" - page handset 1
	"12" - page handset 1 and handset 2
*/ 
E_CMBS_RC CMBS_Api_HandsetPage(char* hs_mask)
{
	return app_SrvHandsetPage(hs_mask);	
}
#else
E_CMBS_RC CMBS_Api_HandsetPage(char all_flag, char hsid)
{
	//"12" can page HS1, HS2, but this API don't support it now.
	
	char buffer[20];
	
   	if ( CMBS_TRUE == all_flag )
   	{
   		sprintf( buffer, "all");
   	}
   	else if (hsid == 0)
   	{
   		sprintf( buffer, "none");
   	}
   	else
   	{
   		sprintf(buffer, "0"+hsid);
   	}
   	return app_SrvHandsetPage(buffer);	
}
#endif

/*====== CMBS Module Handset Delet ========*/
/* hs_mask: string of handset you want to delet */
/* For example: 
	"none" - don't delet any handset
	"all" - delet all handset
	"1" - delet handset 1
	"12" - delet handset 1 and handset 2
*/ 
E_CMBS_RC CMBS_Api_HandsetDelet(char* hs_mask)
{
   return app_SrvHandsetDelete( hs_mask );
}

/*====== CMBS Module Handset List ========*/

E_CMBS_RC  CMBS_Api_GetRegHandset( void )
{
    E_CMBS_RC		 ret;
#if 0 //V2.30 OK
    ST_APPCMBS_CONTAINER st_Container;
    ST_HS_CONTAINER      st_HandsetCont[5];
    int                  i;

    memset( &st_Container,0,sizeof(st_Container) );
    memset( &st_HandsetCont,0,sizeof(st_HandsetCont));

    ret = app_SrvRegisteredHandsets( CMBS_PARAM_HS_LST, 1 );
    if (ret != CMBS_RC_OK)
    	return ret;

    // wait for CMBS target message
    appcmbs_WaitForContainer( CMBS_EV_DSR_PARAM_GET_RES, &st_Container );
    
    memcpy(&st_HandsetCont, &st_Container.ch_Info, st_Container.n_InfoLen);

#if 1
    printf("\n");
    printf( "List of registered handsets:\n" );
    printf( "----------------------------\n");
    for (i=0; i<5; i++)
    {
        printf( "Number: %d    Name: %s\n", st_HandsetCont[i].u16_HsNo, st_HandsetCont[i].u8_HsName);
    }
    printf("\n");
#endif

#endif
    return ret;
}

/*========== InBound Call with 2 HS Ring ===========

Establish a call from Host to Target {INCOMING Call from view of CMBS Target}

Property: CMBS_IE_CALLERPARTY, CMBS_CALLEDPARTY, CMBS_IE_MEDIADESCRIPTOR

CMBS API document: C3
      
===================================================*/
void CMBS_Api_InBound_2HS_Ring( void )
{
   ST_APPCALL_PROPERTIES st_Properties[4];
   ST_APPCMBS_CONTAINER st_Container;

   int  n_Prop = 4;
   char ch_cli[] = "p05678";	// caller ID
   char ch_cni[] = "Thlin";	// Caller Name
   char ch_cld[] = "h1h2";	// HS n
   char ch_Audio[2] = "n";	// Media descriptor
   char ch_line[] = "l0";	// Line ID
   
   printf( "Properties:\n" );
   printf( "Line ID: %s\n", ch_line);
   printf( "Caller ID: %s\n", ch_cli);
   printf( "Caller Name: %s\n", ch_cni);
   printf( "Handset mask: %s\n", ch_cld);
   printf( "Audio wideband/narrow band: %s\n", ch_Audio);
   

   st_Properties[0].e_IE      = CMBS_IE_CALLERPARTY;
   st_Properties[0].psz_Value = ch_cli;
   st_Properties[1].e_IE      = CMBS_IE_CALLEDPARTY;
   st_Properties[1].psz_Value = ch_cld;
   st_Properties[2].e_IE      = CMBS_IE_MEDIADESCRIPTOR;
   st_Properties[2].psz_Value = ch_Audio;
   st_Properties[3].e_IE      = CMBS_IE_CALLERNAME ;
   st_Properties[3].psz_Value = ch_cni;
   
#if 1 // line ID is necessary for call establish of incoming call ?
   st_Properties[4].e_IE      = CMBS_IE_LINE_ID;
   st_Properties[4].psz_Value = ch_line;
   n_Prop = 5;
#endif

#if 1	
   // (enable this, FW v1.13 will seq fault)
   // FW v2.30, enable AutoMode, then Rx thread receive CMBS_EV_DEE_CALL_ANSWER event,
   // Rx threadwill send "channel start" to CMBS (then, HS can hear DSP tone from Host)
   appcall_AutomatMode ( E_APPCALL_AUTOMAT_MODE_ON );
#endif

   // Step 1: Host app send "CallEstablist" to target ([TH-Todo]: No line ID now)
#if 0
   if (g_n_CallNumber == 0xFF )
   {
   	appcmbs_PrepareRecvAdd ( 1 );
   	g_n_CallNumber = appcall_EstablishCall ( st_Properties, n_Prop );
   	if ( g_n_CallNumber == 0xFF )
      	{
      		printf ( "Call can not be set-up!\n" );
      		return;
      	}
   }
#endif
}


int CMBS_Api_InBound_Ring_CallerID( char lineId, char* clid_num, char* clid_name )
{
   ST_APPCALL_PROPERTIES st_Properties[6];
   ST_APPCMBS_CONTAINER st_Container;
   char s_codecs[CMBS_AUDIO_CODEC_MAX*2] = {0};
   // put codecs priority
#if 0 // WB
   sprintf(s_codecs, "%d,%d", (CMBS_AUDIO_CODEC_PCM_LINEAR_NB), (CMBS_AUDIO_CODEC_PCM_LINEAR_WB)); 
#else // NB
   sprintf(s_codecs, "%d", (CMBS_AUDIO_CODEC_PCM_LINEAR_NB)); 
#endif

   int  i = 0, n_Prop = 0;
   char ch_cli[23] = "p0";	// Caller ID
   //char ch_cni[] = "Thlin";	// Caller Name
#if 0
   char ch_cld[3] = "h1";	// HS n
#else
   char ch_cld[7] = "h12345";	// HS all
#endif
   char ch_line[3] = "l0";	// Line ID
   
   
  if ( g_u16_DemoCallId == APPCALL_NO_CALL )
  {
     	appcall_AutomatMode ( E_APPCALL_AUTOMAT_MODE_ON );
   	// HS, line mapping: assume line0 map to HS1, line1 map to HS2, ...
   	ch_line[2] += lineId;			//update Line ID string
   #if 0
   	ch_cld[2] += lineId;			//update HS mask string
   #endif
   
	if (clid_num != NULL)
   	{
   		if (strlen(clid_num) <= 20)
   			strcat(ch_cli, clid_num);		//update Caller ID
   		else
   			printf("Caller ID buffer is full.\n");
   	}
   
   	printf( "Properties:\n" );
   
   	//if (clid_num != NULL)	// Must! Or segamentation fault. ch_cli[]="p0", HS display "UNKNOW".
   	{
   		st_Properties[i].e_IE      = CMBS_IE_CALLERPARTY;
   		st_Properties[i].psz_Value = ch_cli;
   		n_Prop++;
   		i++;
    		printf( "Caller ID: %s\n", ch_cli);
   	}

	if (ch_cld != NULL)
   	{
		st_Properties[i].e_IE      = CMBS_IE_CALLEDPARTY;
  		st_Properties[i].psz_Value = ch_cld;
  		n_Prop++;
		i++;
    		printf( "Handset mask: %s\n", ch_cld);
   	}

   	if (s_codecs != NULL)
   	{
   		st_Properties[i].e_IE      = CMBS_IE_MEDIADESCRIPTOR;
   		st_Properties[i].psz_Value = s_codecs;
   		n_Prop++;
		i++;
    		printf( "Audio wideband/narrow band: %s\n", s_codecs);
   	}
   
   	if (clid_name != NULL)
   	{
   		st_Properties[i].e_IE      = CMBS_IE_CALLERNAME ;
   		st_Properties[i].psz_Value = clid_name;
   		n_Prop++;
		i++;
    		printf( "Caller Name: %s\n", clid_name);
   	}
   
#if 1 // line ID is necessary for call establish of incoming call ?
	if (ch_line != NULL)
	{
		st_Properties[i].e_IE      = CMBS_IE_LINE_ID;
		st_Properties[i].psz_Value = ch_line;
   		n_Prop++;
		i++;
    		printf( "Line ID: %s\n", ch_line);
   	}
#endif

	printf("[THLin]: CMBS_Api_InBound_Ring_CallerID, n_Prop = %d\n", n_Prop);

	if (n_Prop > sizeof(st_Properties)/sizeof(ST_APPCALL_PROPERTIES))
	{
		printf("[THLin]: Error at CMBS_Api_InBound_Ring_CallerID: n_Prop size exceed limitiation %d\n", sizeof(st_Properties)/sizeof(ST_APPCALL_PROPERTIES));
		return 0xFF;
	}

#if 0	
   	// (enable this, FW v1.13 will seq fault)
   	// FW v2.30, enable AutoMode, then Rx thread receive CMBS_EV_DEE_CALL_ANSWER event,
   	// Rx threadwill send "channel start" to CMBS (then, HS can hear DSP tone from Host)
   	appcall_AutomatMode ( E_APPCALL_AUTOMAT_MODE_ON );
#endif



   	g_u16_DemoCallId = appcall_EstablishCall ( st_Properties, n_Prop );
   	if ( g_u16_DemoCallId == APPCALL_NO_CALL )
      	{
      		printf ( "Call can not be set-up!\n" );
      	}
   }
   else
   {
		printf( "Switch to narrowband audio\n" );
		appmedia_CallObjMediaOffer( g_u16_DemoCallId, 'n' );
   }
   
   return g_u16_DemoCallId;

}

/*====================== Outbound Call Progress ===================
 * Call progress information from Host to Target side
 * Property: CMBS_IE_CALLPROGRESS
====================================================================*/
void CMBS_Api_OutboundCallProgress(int n_Line, char * psz_Value )
{
   ST_APPCALL_PROPERTIES st_Properties;
   
   st_Properties.e_IE = CMBS_IE_CALLPROGRESS;
   st_Properties.psz_Value = psz_Value;
   
   appcall_ProgressCall ( &st_Properties, 1, n_Line, NULL );
}


void CMBS_Api_OutboundEstablishCall(PST_CALL_OBJ pst_Call)
{
   u16 u16_CallId;
   u16 u16_LineId;
   ST_APPCALL_PROPERTIES st_Properties;
   
   u16_CallId = _appcall_CallObjIdGet(pst_Call);

   if (pst_Call->u8_LineId == APPCALL_NO_LINE)
   {
       u16_LineId = 0;  //choose default line for current HS
       RTK_DBG("Set default line to %d for current HS\n", u16_LineId);
       _appcall_CallObjLineObjSet( pst_Call, _appcall_LineObjGet(u16_LineId) );
       //THLin: u16_LineId = 0, Call state LineMask =0x01
       //THLin: u16_LineId = 1, Call state LineMask =0x02
       //THLin: u16_LineId = 2, Call state LineMask =0x04
       //THLin: u16_LineId = 3, Call state LineMask =0x08
   }
   
   char value1[] = "CMBS_CALL_PROGR_SETUP_ACK\0";
   char value2[] = "CMBS_CALL_PROGR_PROCEEDING\0";
   
   if ( !pst_Call->st_CalledParty.u8_AddressLen )
   {
   	st_Properties.e_IE = CMBS_IE_CALLPROGRESS;
   	st_Properties.psz_Value = value1;   
   	appcall_ProgressCall ( &st_Properties, 1, u16_CallId, NULL );
   
   	//THLin: Let state between CMBS and HS to be CC-Proceeding.
   	st_Properties.e_IE = CMBS_IE_CALLPROGRESS;
   	st_Properties.psz_Value = value2;   
   	appcall_ProgressCall ( &st_Properties, 1, u16_CallId, NULL );
   	
   }
   
   CMBS_Api_Set_Need_CallId_Answer(u16_LineId, u16_CallId, pst_Call->ch_CallerID[0]-0x31);
}

void CMBS_Api_OutboundAnswerCall(char lineId)
{
	int i;
	u8 u8_Answ;
	u8 u8_WBCodec = 0;
	u16 u16_CallId;
	PST_CALL_OBJ pst_Call;
	ST_APPCALL_PROPERTIES st_Properties;
   
	// wait for CMBS_EV_DEE_CALL_MEDIA_UPDATE
   	Sleep(300);

	RTK_DBG("%s, line_id = %d\n", __FUNCTION__, lineId);	
	u16_CallId = CMBS_Api_Get_Need_Answer_CallId(lineId);
	if (u16_CallId == 0xFF)
	{
		printf("%s fail, or no need to answer call. call_id = 0xFF\n", __FUNCTION__);
        return;
	}
	pst_Call = _appcall_CallObjGetById(u16_CallId);
	RTK_DBG("Get call_id = %d\n", u16_CallId);
	
	// check WB support
	for(i=0; i < pst_Call->u8_CodecsLength; i++)
	{
	   if(pst_Call->pu8_CodecsList[i] == CMBS_AUDIO_CODEC_PCM_LINEAR_WB)
	   {
			u8_WBCodec = 1;
			RTK_DBG("Outgoing support WB\n");
			//break;
	   }
	   else if(pst_Call->pu8_CodecsList[i] == CMBS_AUDIO_CODEC_PCM_LINEAR_NB)
	   {
			u8_WBCodec = 0;
			RTK_DBG("Outgoing support NB\n");
			break;
	   }
	}


	u8_Answ = 'N';
	if ( u8_Answ == 'w' || u8_Answ == 'W' )
		u8_WBCodec = 1;
	else
		u8_WBCodec = 0;

	// check do we need codec switch
	if(pst_Call->e_Codec == CMBS_AUDIO_CODEC_PCM_LINEAR_WB && u8_WBCodec == 0)
	{
		appmedia_CallObjMediaStop(0, u16_CallId, NULL);

		// wait for CMBS_EV_DEM_CHANNEL_STOP_RES
		Sleep(200);

		appmedia_CallObjMediaOffer( u16_CallId, 'n' );
	}
	else if(pst_Call->e_Codec == CMBS_AUDIO_CODEC_PCM_LINEAR_NB && u8_WBCodec == 1)
	{
		appmedia_CallObjMediaStop(0, u16_CallId, NULL);

		// wait for CMBS_EV_DEM_CHANNEL_STOP_RES
		Sleep(200);

		appmedia_CallObjMediaOffer( u16_CallId, 'w' );
	}

	// wait for CMBS_EV_DEM_CHANNEL_OFFER_RES
	Sleep(500);

	appmedia_CallObjMediaStart(0, u16_CallId, 0xFF, NULL);

	memset ( &st_Properties,0, sizeof(st_Properties));  

   	appcall_AnswerCall ( &st_Properties, 0, u16_CallId, NULL );

   	g_u16_DemoCallId = u16_CallId;
}

/*====== Call Release ========*/
// Ex. Incoming call, HS Ringing, call this API to stop Ringing.

void CMBS_Api_CallRelease (int n_Line, char reason)
{
   char ch_Reason[2];
   ST_APPCALL_PROPERTIES st_Properties;
   
   // Release Reason:
   // 0 => Normal, 1 => Abnormal
   // 2 => Busy,   3 => unknown Number

   if ( g_u16_DemoCallId != APPCALL_NO_CALL )
   {
	   ch_Reason[0] = reason;
	   ch_Reason[1] = 0;
	   
	   st_Properties.e_IE      = CMBS_IE_CALLRELEASE_REASON;
	   st_Properties.psz_Value = ch_Reason;
	   
	   appcall_ReleaseCall(  &st_Properties, 1, n_Line, NULL );
	   
	   g_u16_DemoCallId = APPCALL_NO_CALL;
   }
}

void CMBS_Api_CallRelease_Busy (u16 u16_CallId)
{
   char ch_Reason[2];
   ST_APPCALL_PROPERTIES st_Properties;
   
   // Release Reason:
   // 0 => Normal, 1 => Abnormal
   // 2 => Busy,   3 => unknown Number
   
   //if ( g_u16_DemoCallId != APPCALL_NO_CALL )
   {
	   ch_Reason[0] = 0; //line ID?
	   ch_Reason[1] = 2;
	   
	   st_Properties.e_IE      = CMBS_IE_CALLRELEASE_REASON;
	   st_Properties.psz_Value = ch_Reason;
	   
	   appcall_ReleaseCall( &st_Properties, 1, u16_CallId, NULL );
	   
	   //g_u16_DemoCallId = APPCALL_NO_CALL;
   }
   RTK_DBG("%s, callId=%d\n", __FUNCTION__, u16_CallId);
}


/*========= Line/Call State Check ========= */
E_APPCMBS_CALL CMBS_Api_LineStateGet(int line)
{
	PST_CALL_OBJ pst_this;
	E_APPCMBS_CALL line_st;

	if (line > APPCALL_CALLOBJ_MAX)
	{
		printf("Error at %s, %d\n", __FUNCTION__, __LINE__);
                return E_APPCMBS_CALL_CLOSE;
	}
        else
        	pst_this = g_call_obj + line;
        	
	line_st = _appcall_CallObjStateGet( pst_this );

	
	RTK_DBG("CMBS_Api_LineStateGet: %s\n", _appcall_CallObjStateString(pst_this->e_Call) );

	return line_st;
}

void CMBS_Api_BusyPlay ( u16 u16_CallId, int no_On )
{
   appmedia_CallObjTonePlay( "CMBS_TONE_BUSY\0", no_On, u16_CallId, NULL );
   RTK_DBG("%s, callId=%d, flag=%d\n", __FUNCTION__, u16_CallId, no_On);
}

