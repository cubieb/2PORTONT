#include <stdio.h>
#include <stdlib.h>
#include "cmbs_platf.h"
#include "cmbs_api.h"
#include "cfr_ie.h"
#include "cfr_mssg.h"
#include "appcmbs.h"
#include "appsrv.h"
#include "appcall.h"
#include "cmbs_str.h"

/* THLin: global num to record if call obj is available or not,
 * 0xFF means call obj is unavailable, else value means call obj is available.
 * The same function with the g_n_CallNumber in keyb_call.c
 */
//int g_CallObjNum = 0xFF;	//THLin: can be use to multiple-HS ? @@
extern int g_n_CallNumber;

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
    ST_APPCMBS_CONTAINER st_Container;
    ST_HS_CONTAINER      st_HandsetCont[5];
    E_CMBS_RC		 ret;
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
}


int CMBS_Api_InBound_Ring_CallerID( char lineId, char* clid_num, char* clid_name )
{
   ST_APPCALL_PROPERTIES st_Properties[4];
   ST_APPCMBS_CONTAINER st_Container;

   int  i, n_Prop = 4;
   char ch_cli[23] = "p0";	// caller ID
   //char ch_cni[] = "Thlin";	// Caller Name
   char ch_cld[3] = "h1";	// HS n
   char ch_Audio[2] = "n";	// Media descriptor
   char ch_line[3] = "l0";	// Line ID
   
   // HS, line mapping: assume line0 map to HS1, line1 map to HS2, ...
   ch_line[2] += lineId;			//update Line ID string
   ch_cld[2] += lineId;				//update HS mask string
   if (strlen(clid_num) <= 20)
   	strcat(ch_cli, clid_num);		//update Caller ID
   else
   	printf("Caller ID buffer is full.\n");
   
   printf( "Properties:\n" );
   printf( "Line ID: %s\n", ch_line);
   printf( "Caller ID: %s\n", ch_cli);
   printf( "Caller Name: %s\n", clid_name);
   printf( "Handset mask: %s\n", ch_cld);
   printf( "Audio wideband/narrow band: %s\n", ch_Audio);
   

   st_Properties[0].e_IE      = CMBS_IE_CALLERPARTY;
   st_Properties[0].psz_Value = ch_cli;
   st_Properties[1].e_IE      = CMBS_IE_CALLEDPARTY;
   st_Properties[1].psz_Value = ch_cld;
   st_Properties[2].e_IE      = CMBS_IE_MEDIADESCRIPTOR;
   st_Properties[2].psz_Value = ch_Audio;
   st_Properties[3].e_IE      = CMBS_IE_CALLERNAME ;
   st_Properties[3].psz_Value = clid_name;
   
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
   if (g_n_CallNumber == 0xFF )
   {
   	appcmbs_PrepareRecvAdd ( 1 );
   	g_n_CallNumber = appcall_EstablishCall ( st_Properties, n_Prop );
   	if ( g_n_CallNumber == 0xFF )
      	{
      		printf ( "Call can not be set-up!\n" );
      	}
   }
   
   return g_n_CallNumber;
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

/*====== Call Release ========*/
// Ex. Incoming call, HS Ringing, call this API to stop Ringing.

void CMBS_Api_CallRelease (int n_Line, char reason)
{
   char ch_Reason[2];
   ST_APPCALL_PROPERTIES st_Properties;
   
   // Release Reason:
   // 0 => Normal, 1 => Abnormal
   // 2 => Busy,   3 => unknown Number

   ch_Reason[0] = reason;
   ch_Reason[1] = 0;
   
   st_Properties.e_IE      = CMBS_IE_CALLRELEASE_REASON;
   st_Properties.psz_Value = ch_Reason;
   
   appcall_ReleaseCall(  &st_Properties, 1, n_Line, NULL );
   
   g_n_CallNumber = 0xFF;
}


/*========= Line/Call State Check ========= */
E_APPCMBS_CALL CMBS_Api_LineStateGet(int line)
{
	PST_CALL_OBJ pst_this;
	E_APPCMBS_CALL line_st;

	pst_this = _appcall_CallOBJGet2( line );

	line_st = _appcall_CallOBJStateGet( pst_this );
	
	printf("CMBS_Api_LineStateGet: %s\n", _appcall_CallObjStateString(pst_this->e_Call) );

	return line_st;
}

