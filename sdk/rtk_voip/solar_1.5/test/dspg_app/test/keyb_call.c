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
   ST_APPCALL_PROPERTIES st_Properties[4];
   int  n_Prop = 3;
   char ch_cli[30];
   char ch_cni[30];
   char ch_cld[20];
   char ch_Audio[2];
   
   printf( "Enter Properties\n" );
   printf( "CLI [{p/r}{RingType 0..7}{number}]:");
   tcx_gets(ch_cli, sizeof(ch_cli));
   
   printf( "CNI [{name}]:");
   tcx_gets(ch_cni, sizeof(ch_cni));
   
   printf( "\nGroupID or Handset mask [{g/h}{number/hshshs}]:");
   tcx_gets(ch_cld, sizeof(ch_cld));
   
   printf( "\nAudio wideband/narrow band [{w/n}]:" );
   ch_Audio[0] = tcx_getch();
   ch_Audio[1] = 0;
   
   st_Properties[0].e_IE      = CMBS_IE_CALLERPARTY;
   st_Properties[0].psz_Value = ch_cli;
   st_Properties[1].e_IE      = CMBS_IE_CALLEDPARTY;
   st_Properties[1].psz_Value = ch_cld;
   st_Properties[2].e_IE      = CMBS_IE_MEDIADESCRIPTOR;
   st_Properties[2].psz_Value = ch_Audio;
   if ( strlen(ch_cni) )
   {
      st_Properties[3].e_IE      = CMBS_IE_CALLERNAME ;
      st_Properties[3].psz_Value = ch_cni,
      n_Prop=4;
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
      printf ("g => setup-ack\n");
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

