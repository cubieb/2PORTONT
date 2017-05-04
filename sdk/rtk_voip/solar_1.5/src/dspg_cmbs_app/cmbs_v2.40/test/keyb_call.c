/*!
*	\file			keyb_call.c
*	\brief			
*	\Author		kelbch 
*
*	@(#)	%filespec: keyb_call.c-1 %
*
*******************************************************************************
*	\par	History
*	\n==== History ============================================================\n
*	date			name		rev	          action                           \n
*	---------------------------------------------------------------------------\n
* 18-sep-09		Kelbch		pj1029-479    add quick call demonstration component\n
*******************************************************************************
*	COPYRIGHT DOSCH & AMAND RESEARCH GMBH & CO.KG
*	DOSCH & AMAND RESEARCH GMBH & CO.KG Confidential
*
*******************************************************************************/

#if	!defined( KEYB_CALL_H )
#define	KEYB_CALL_H


#if defined( __cplusplus )
extern "C"
{
#endif

#if defined( __cplusplus )
}
#endif

#endif	//	KEYB_CALL_H
//*/

#include <stdio.h>
#include <stdlib.h>
#include "cmbs_platf.h"

#include "cmbs_api.h"
#include "cfr_ie.h"
#include "cfr_mssg.h"
#include "appcmbs.h"
#include "appsrv.h"
#include "appcall.h"
#include "tcx_keyb.h"
#include "cmbs_str.h"


/***************************************************************
*	
* external defined function for a quick demonstration
*
****************************************************************/

extern void appcall_InfoCall( int n_Call );

/***************************************************************
*	
* global variable
*
****************************************************************/

int		g_n_CallNumber = 0xFF;

void  keyb_ReleaseNotify( void )
{
  g_n_CallNumber = 0xFF;
}


//		========== 	keyb_CallInfo ===========
/*!
		\brief				 print the call information for demonstration line

		\param[in,out]		 <none>

		\return				 <none>

*/

void	keyb_CallInfo( void )
{
	if ( g_n_CallNumber != 0xFF )
  {
  	appcall_InfoCall( g_n_CallNumber );
  }
}

//		========== 	keyb_IncCallWB ===========
/*!
		\brief				 starts an incoming WB call for demonstration line\n
                           in a active call it launch codec change to WB

		\param[in,out]		 <none>

		\return				 <none>
      
      The demonstration line uses the automat of appcall component\n
		CLI            : 1234
      Ringing Pattern: standard
      Handsets       : 12345
      CNAME		   : CMBS WB
*/

void	keyb_IncCallWB( void )
{
  ST_APPCALL_PROPERTIES st_Properties[5];
  int  n_Prop = 3;

  if (g_n_CallNumber == 0xFF )
  {
  
  	appcall_AutomatMode ( E_APPCALL_AUTOMAT_MODE_ON );
  
  	st_Properties[0].e_IE      = CMBS_IE_CALLERPARTY;
  	st_Properties[0].psz_Value = "p01234\0";
  	st_Properties[1].e_IE      = CMBS_IE_CALLEDPARTY;
  	st_Properties[1].psz_Value = "h12345\0";
  	st_Properties[2].e_IE      = CMBS_IE_MEDIADESCRIPTOR;
  	st_Properties[2].psz_Value = "w\0";
  	st_Properties[3].e_IE      = CMBS_IE_CALLERNAME ;
  	st_Properties[3].psz_Value = "Call WB\0",
  	st_Properties[4].e_IE      = CMBS_IE_LINE_ID ;
  	st_Properties[4].psz_Value = "l1\0",
  	n_Prop=5;
  
  	g_n_CallNumber = appcall_EstablishCall ( st_Properties, n_Prop );
      if ( g_n_CallNumber == 0xFF )
      {
      	printf ( "Call can not be set-up!\n" );
      }
  }
  else
  {
  	printf( "Switch to narrow band audio\n" );
	   appmedia_CallObjMediaOffer( g_n_CallNumber, 'w' );
  }

}

//		========== 	keyb_IncCallNB ===========
/*!
		\brief				 starts an incoming NB call for demonstration line\n
                           in a active call it launch codec change to NB

		\param[in,out]		 <none>

		\return				 <none>
      
      The demonstration line uses the automat of appcall component\n
		CLI            : 5678
      Ringing Pattern: standard
      Handsets       : 12345
      CNAME		   : CMBS NB
*/

void keyb_IncCallNB( void )
{
  ST_APPCALL_PROPERTIES st_Properties[5];
  int  n_Prop = 3;
  
  if ( g_n_CallNumber == 0xFF )
  {
  	appcall_AutomatMode ( E_APPCALL_AUTOMAT_MODE_ON );
  
  	st_Properties[0].e_IE      = CMBS_IE_CALLERPARTY;
  	st_Properties[0].psz_Value = "p05678\0";
  	st_Properties[1].e_IE      = CMBS_IE_CALLEDPARTY;
  	st_Properties[1].psz_Value = "h12345\0";
  	st_Properties[2].e_IE      = CMBS_IE_MEDIADESCRIPTOR;
  	st_Properties[2].psz_Value = "n\0";
  	st_Properties[3].e_IE      = CMBS_IE_CALLERNAME ;
  	st_Properties[3].psz_Value = "Call NB\0",
  	st_Properties[4].e_IE      = CMBS_IE_LINE_ID;
  	st_Properties[4].psz_Value = "l1\0",
  	n_Prop=5;
  
  	g_n_CallNumber = appcall_EstablishCall ( st_Properties, n_Prop );
      if ( g_n_CallNumber == 0xFF )
      {
      	printf ( "Call can not be set-up!\n" );
      }
  }
  else
  {
  	printf( "Switch to wideband audio\n" );
	   appmedia_CallObjMediaOffer( g_n_CallNumber, 'n' );
  }

}
//		========== 	keyb_IncCallRelease ===========
/*!
		\brief				 release call on demonstration line

		\param[in,out]		 <none>

		\return				 <none>

*/

void 	keyb_IncCallRelease( void )
{
   ST_APPCALL_PROPERTIES st_Properties;
   
   if ( g_n_CallNumber != 0xFF )
   {
   	st_Properties.e_IE      = CMBS_IE_CALLRELEASE_REASON;
   	st_Properties.psz_Value = "0\0";
   
	    appcall_ReleaseCall(  &st_Properties, 1, g_n_CallNumber, NULL );
      g_n_CallNumber = 0xFF;
   }
}

//		========== _keyb_LineReturn  ===========
/*!
		\brief	enter line ID 

		\param[in]		 <none> 

		\return			<int> return Line ID in binary form

*/

int   _keyb_LineReturn ( void )
{
   int n_Line;
   
   printf( "Enter Line ID " );
   n_Line = tcx_getch();
   n_Line -= '0';
   
   return n_Line;
}
//		========== keyb_CallEstablish ===========
/*!
		\brief				 establish a call from Host to Target {INCOMING Call from view of CMBS Target}
		\param[in,out]		 <none>
		\return				 <none>

      CMBS_IE_CALLERPARTY, CMBS_CALLEDPARTY, CMBS_IE_MEDIADESCRIPTOR
      
*/
void     keyb_CallEstablish ( void )
{
   ST_APPCALL_PROPERTIES st_Properties[5];
   int  n_Prop = 5;
   static char ch_cli[30];
   static char ch_cni[30];
   static char ch_cld[20];
   static char ch_clineid[20];
   static char ch_chm[20];
   static char ch_Audio[2];
   
   printf( "Enter Properties\n" );
   printf( "CLI [{p/r}{RingType 0..7}{number}]:");
   tcx_gets(ch_cli, sizeof(ch_cli));
   
   printf( "CNI [{name}]:");
   tcx_gets(ch_cni, sizeof(ch_cni));
   
   printf( "\nLineID[{number}]:");
   tcx_gets(ch_clineid, sizeof(ch_clineid));

   printf( "\nHandset mask [{h}{number/hshshs}]:");
   tcx_gets(ch_cld, sizeof(ch_cld));
   

   
   printf( "\nAudio wide band/narrow band/a-law/u-law [{w/n/a/u}]:" );
   ch_Audio[0] = tcx_getch();
   ch_Audio[1] = 0;
   
   st_Properties[0].e_IE      = CMBS_IE_CALLERPARTY;
   st_Properties[0].psz_Value = ch_cli;
   st_Properties[1].e_IE      = CMBS_IE_CALLEDPARTY;
   st_Properties[1].psz_Value = ch_cld;
   st_Properties[2].e_IE      = CMBS_IE_MEDIADESCRIPTOR;
   st_Properties[2].psz_Value = ch_Audio;
	st_Properties[3].e_IE      = CMBS_IE_LINE_ID;
	st_Properties[3].psz_Value = ch_cld;
   if ( strlen(ch_cni) )
   {
      st_Properties[4].e_IE      = CMBS_IE_CALLERNAME ;
      st_Properties[4].psz_Value = ch_cni,
      n_Prop=5;
   }
   appcall_EstablishCall ( st_Properties, n_Prop );
}

//		========== keyb_CallRelease ===========
/*!
		\brief				Release a call from Host to Target side
		\param[in,out]		<none>
		\return				<none>

      CMBS_IE_CALLRELEASE_REASON
*/
void     keyb_CallRelease ( void )
{
   int                   n_Line;
   char ch_Reason[2];
   ST_APPCALL_PROPERTIES st_Properties;
   
   n_Line = _keyb_LineReturn();

   printf( "\nReason \n" );
   printf( "0 => Normal\t 1 => Abnormal\n" );
   printf( "2 => Busy\t 3 => unknown Number\n" );
   
   ch_Reason[0] = tcx_getch();
   ch_Reason[1] = 0;
   
   st_Properties.e_IE      = CMBS_IE_CALLRELEASE_REASON;
   st_Properties.psz_Value = ch_Reason;
   
   appcall_ReleaseCall(  &st_Properties, 1, n_Line, NULL );
}

//		========== keyb_CallProgress ===========
/*!
		\brief				call progress information from Host to Target side
		\param[in,out]		<none>
		\return				<none>

      CMBS_IE_CALLPROGRESS
*/
void     keyb_CallProgress ( char * psz_Value )
{
   int      n_Line;
   ST_APPCALL_PROPERTIES   st_Properties;
   
   n_Line = _keyb_LineReturn();
   
   st_Properties.e_IE      = CMBS_IE_CALLPROGRESS;
   st_Properties.psz_Value = psz_Value;
   
   appcall_ProgressCall ( &st_Properties, 1, n_Line, NULL );
}
//		========== keyb_CallAnswer ===========
/*!
		\brief				call progress information from Host to Target side
		\param[in,out]		<none>
		\return				<none>

*/
void     keyb_CallAnswer( void )
{
   int      n_Line;
   ST_APPCALL_PROPERTIES   st_Properties;
   
   memset ( &st_Properties,0, sizeof(st_Properties));
   
   n_Line = _keyb_LineReturn();
   
    appcall_AnswerCall ( &st_Properties, 0, n_Line, NULL );
}

//		========== keyb_MediaSwitch ===========
/*!
		\brief				Media channel start or stop
		\param[in,out]		bo_On    TRUE to start, FALSE to stop
		\return				<none>

*/
void     keyb_MediaSwitch ( int bo_On )
{
   int n_Line;
   
   n_Line = _keyb_LineReturn();
   
   if ( bo_On )
   {
      appmedia_CallOBJMediaStart( 0, n_Line, NULL );
   }
   else
   {
      appmedia_CallOBJMediaStop(  0, n_Line, NULL );
   }
}

void keyb_MediaInernalCall()
{
   char  ch_Channel;  
   char  ch_Context;
   char  ch_Operation;
   
   printf("\nChannel ID? [0..3]:");
   ch_Channel = tcx_getch();

   if ( (ch_Channel >= '0') && (ch_Channel <='3' ))
   {
      ch_Channel -= '0';    
   }
   else
   {
      printf("\nUnavailble Channel ID, Must be 0 to 3, Press any key to continue");
      tcx_getch();
      return; 
   }

   printf("\nContext/Node ID? [0..1]:");
   ch_Context = tcx_getch();

   if ( (ch_Context >= '0') && (ch_Context <='1' ))
   {
      ch_Context -= '0';    
   }
   else
   {
      printf("\nUnavailble Context/Node ID, Must be 0 or 1, Press any key to continue");
      tcx_getch();
      return;
   }


   printf("\nConnect or Disconnect? [1, Connect; 0, Disconnect]:");
   ch_Operation = tcx_getch();

   if ( (ch_Operation >= '0') && (ch_Operation <='1' ))
   {
      ch_Operation -= '0';    
   }
   else
   {
      printf("\nError Command, Must be 0 or 1, Press any key to continue");
      tcx_getch();
      return;
   }

   appmedia_CallObjMediaInternalConnect(ch_Channel, ch_Context, ch_Operation);

}

void     keyb_MediaOffer()
{
   int   n_Line;
   char  ch_Audio;   

   n_Line = _keyb_LineReturn();

   printf( "\nAudio wideband/narrow band [{w/n}]:" );
   ch_Audio = tcx_getch();

   
   appmedia_CallObjMediaOffer( n_Line, ch_Audio );
}


//		========== keyb_TonePlay ===========
/*!
		\brief				Play/Stop a tone on Media channel 
		\param[in,out]		psz_Value  enumeration string of CMBS tone
		\param[in,out]		bo_On    TRUE to start, FALSE to stop
		\return				<none>

*/
void     keyb_TonePlay ( char * psz_Value, int no_On )
{
   int      n_Line;

   n_Line = _keyb_LineReturn();

   appmedia_CallOBJTonePlay( psz_Value, no_On,n_Line,NULL );
}

//		========== keyb_CallWaiting ===========
/*!
		\brief				start/stop Call waiting from host to target.
		\param[in,out]		<none>
		\return				<none>

      \note             Host is in responsibility of stop call waiting and
                        provide the old CLI to CMBS target.
*/
void     keyb_CallWaiting ( void )
{
   ST_APPCALL_PROPERTIES st_Properties;
   
   int      n_Line;
   int      n_On = FALSE;
   char     ch_cli[30];
   
   n_Line = _keyb_LineReturn();
   
   printf( "Call Waiting On {y/n}:\n" );
   n_On = tcx_getch();
   
   printf( "Caller Party :" );
   tcx_gets(ch_cli, sizeof(ch_cli));
   if ( n_On == 'y' )
   {
      appmedia_CallOBJTonePlay( "CMBS_TONE_CALL_WAITING\0", TRUE,n_Line,NULL );
   }
   else
   {
      appmedia_CallOBJTonePlay( NULL, FALSE,n_Line,NULL );
   }
   st_Properties.e_IE = CMBS_IE_CALLINFO;
   st_Properties.psz_Value =ch_cli;
   
   appcall_DisplayCall ( &st_Properties, 1, n_Line, NULL );
}
//		========== keyb_CallHold  ===========
/*!
		\brief			signal to CMBS Target, that the call is hold or resumed.
                     the CMBS Target enable/disable the audio stream.
		\param[in]	   <none>
		\return			<none>
*/

void  keyb_CallHold ( void )
{
   int n_Line, n_On;
   
   n_Line = _keyb_LineReturn();

   printf("\nCall Hold? [y/n] : ");
   n_On = tcx_getch();
   if( n_On == 'y' )
   {
      appcall_HoldCall( n_Line, NULL );
   }
   else
   {
      appcall_ResumeCall( n_Line, NULL );
   }
}

//		========== keyb_CallLoop ===========
/*!
		\brief				call management loop to control the call activities
		\param[in,out]		<none>
		\return				<none>

*/

void     keyb_CallLoop(void)
{
   int n_Keep = TRUE;
   
   while ( n_Keep )
   {
      tcx_appClearScreen();
      
      printf ("\n======Call Objects===============================\n\n");
      appcall_InfoPrint();
      printf ("\n======Call Control===============================\n\n");
      printf ("l => automat on     L => automat off\n");
      printf ("e => establish call r => release call\n");
      printf ("c => change codec   h => hold/resume\n");
      printf ("w => call waiting   k => ringing\n" );
      printf ("a => answer call    f => early connect\n");
      printf ("g => setup-ack      p => proceeding\n"); // THLin add
      printf ("\n======Media===============================\n\n");
      printf ("1 => Dial Tone      2 => Ring Tone\n");
      printf ("3 => Busy Tone      0 => Off Tone \n");
      printf ("m => Media on       n => Media off\n");
      printf ("\n------------------------------------------\n\n" );
      printf ("i => call/line infos \n");
      printf ("q => return to interface \n" );

      switch ( tcx_getch() )
      {
         case 'l':
            appcall_AutomatMode ( E_APPCALL_AUTOMAT_MODE_ON );
            break;
         case 'L':
            appcall_AutomatMode ( E_APPCALL_AUTOMAT_MODE_OFF );
            break;
            
         case 'e':
               // establish
               keyb_CallEstablish();
            break;
         case 'r':
               // release call
               keyb_CallRelease();
            break;
         case 'c':
              // change codec
              keyb_MediaOffer();
               break;
         case 'h':
               // Call Hold/Resume
               keyb_CallHold();
            break;
         case 'w':
               // call waiting
               keyb_CallWaiting();
            break;
         case 'k':
              // ringback-tone
              keyb_CallProgress ( (char*)"CMBS_CALL_PROGR_RINGING\0" );
              break;
         case 'a':
               // answer call
               keyb_CallAnswer();
            break;
         case 'f':
               // early media
              keyb_CallProgress ( (char*)"CMBS_CALL_PROGR_INBAND\0" );
             break;
         case 'g':
               keyb_CallProgress ( (char*)"CMBS_CALL_PROGR_SETUP_ACK\0");
               break;
         case 'p': // THLin add
               keyb_CallProgress ( (char*)"CMBS_CALL_PROGR_PROCEEDING\0");
               break;               
         case '1':
              keyb_TonePlay( "CMBS_TONE_DIAL\0", TRUE );
              break;
         case '2':
              keyb_TonePlay( "CMBS_TONE_RING_BACK\0", TRUE );
              break;
         case '3':
              keyb_TonePlay( "CMBS_TONE_BUSY\0", TRUE );
              break;
         case 'm':
              keyb_MediaSwitch( TRUE );
              break;
         case 'n':
              keyb_MediaSwitch( FALSE );
               break;
         case '0':
               keyb_TonePlay( NULL,FALSE );
               break;
         case 'i':
               // call/line info
            tcx_appClearScreen();
            appcall_InfoPrint();
            printf ("Press Any Key\n " );
            tcx_getch();
            break;
            
         case 'q':
              n_Keep = FALSE;
              break;
            
      }      
   }
}

