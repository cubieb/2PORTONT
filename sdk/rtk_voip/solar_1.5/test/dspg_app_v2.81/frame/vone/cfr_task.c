/*!
*	\file			cfr_task.c
*	\brief
*	\Author		kelbch
*
*	@(#)	%filespec: cfr_task.c~DMZD53#3 %
*
*******************************************************************************
*	\par	History
*	\n==== History ============================================================\n
*	date			name		version	 action                                          \n
*	----------------------------------------------------------------------------\n
* 14-feb-09		kelbch		1		 Initialize\n
*******************************************************************************/

#include "tclib.h"
#include "embedded.h"
#include "cg0type.h"
#include "bsd09cnf.h"            /* component-globalal, system configuration */
#include "bsd09ddl.h"            /* messages and processes */
#include "csys0reg.h"
#include "cos00int.h"
#include "cmbs_int.h"            /* internal API structure and defines */
#include "cfr_uart.h"            /* interface of uart packagetizer */
#include "tapp.h"

#if defined( CMBS_CATIQ )
#include "tapp_facility.h"
#endif

#include "tapp_log.h"
#include "cfr_debug.h"

#ifdef DNA_CFR_TASK_DEBUG
#	define	DNA_CFR_TASK_DBG(x)     printf( (x) );
#else
#	define	DNA_CFR_TASK_DBG(x)		;
#endif

#if defined (CMBS_AUDIO_TEST_SLAVE)
extern void    cmbsAudioSlaveChange ( u8 u8_key );
#endif

//GSN
extern u8 		u8Test0[];// = "\xda\xda\xda\xda\x0a\x00\x11\x22\x33\x44\x55\x66\x77\x88";
extern u8 		u8Test1[];// = "\xda\xda\xda\xda\x0b\x00\x99\xaa\x11\x22\x44\x66\x44\x99\x31";

extern ST_CMBS_APP       g_CMBSApp;

void	cfr_taskPacketTest ( u8 u8_IDX )
{
   if ( u8_IDX )
      cfr_uartPacketWrite ( u8Test1, 14 );
   else
      cfr_uartPacketWrite ( u8Test0, 15 );
}

//		========== 	cfr_taskPacketReceived ===========
/*!
		\brief				 A serialized packet is received, check format and
      					 pass it to De-serializer

		\param[in,out]		 < none >

		\return				< none >

*/

void	cfr_taskPacketReceived( void )
{
   u16  u16_Length;
   u8* pu8_Buffer;

   pu8_Buffer = cfr_uartPacketRead( CFR_BUFFER_UART_REC, &u16_Length);

   if (pu8_Buffer)
   {
      /*
      CFR_DBG_OUT ( "==================================\n" );
      CFR_DBG_OUT ( "CMBS-Task: Packet len %d received \n",u16_Length );

      cfr_uartPacketTrace ( CFR_BUFFER_UART_REC );
      */

      cmbs_int_EventReceive ( pu8_Buffer, u16_Length );

      cfr_uartPacketFree( CFR_BUFFER_UART_REC );
      // re-enable transmission if target is in windows
      if ( cfr_uartPacketWindowReached( CFR_BUFFER_UART_REC ) )
      {
         /*!\todo calculate correct last window */
         cmbs_int_cmd_FlowNOKHandle ( 0 );
      }
      else
      {
         /*!\todo calculate correct last window */
         cmbs_int_cmd_FlowRestartHandle ( 0);
      }
   }
}

extern void   p_dr18_cmbsTransmit(void);
//extern
//ST_CMBS_APP       g_CMBSApp;

#if defined( CMBS_CATIQ )
extern
ST_CMBS_FIFO      g_FacilityReqFifo;
#endif

void	CMBSTASK ( void )
{
   switch (G_st_os00_Act.u8_Event)
   {
   case   CMBS_TASK_UART_IND:
      {
         u8 u8_Buffer = (u8)G_st_os00_Act.u16_XInfo;

         CFR_CMBS_ENTER_CRITICALSECTION( g_CMBSInstance.h_CriticalSectionTransmission );
         cfr_uartDataReceiveGet ( &u8_Buffer, 1 );
         CFR_CMBS_LEAVE_CRITICALSECTION( g_CMBSInstance.h_CriticalSectionTransmission );
      }
      break;

   case   CMBS_TASK_PACK_IND:
      // complete packet received
      cfr_taskPacketReceived();
      break;

   case CMBS_TASK_UART_CFM:

      // From now on we need exclusive access to transmission of serial port
      CFR_CMBS_ENTER_CRITICALSECTION( g_CMBSInstance.h_CriticalSectionTransmission );

      p_dr18_cmbsTransmit();

      // To avoid deadlock in case of error, leave critical section
      CFR_CMBS_LEAVE_CRITICALSECTION( g_CMBSInstance.h_CriticalSectionTransmission );

      break;

   case CMBS_TASK_PACK_REQ:

      break;

   case CMBS_TASK_UART_CMPL:
      // packet is transmitted, kick next packet if needed
      //      CFR_DBG_OUT ( "Kicked by completed package\n" ),

      // From now on we need exclusive access to transmission of serial port
      CFR_CMBS_ENTER_CRITICALSECTION( g_CMBSInstance.h_CriticalSectionTransmission );

      cfr_uartDataTransmitKick ();

      // To avoid deadlock in case of error, leave critical section
      CFR_CMBS_LEAVE_CRITICALSECTION( g_CMBSInstance.h_CriticalSectionTransmission );

      break;
#ifndef DNA_BOOTING
   case CMBS_TASK_HS_REGISTERED:
      //			CFR_DBG_OUT( "CMBS_TASK_HS_REGISTERED\n" );
      cmbs_int_OnHsRegistration();
      break;
#endif

   case CMBS_TASK_USB_LINE_STATUS:
      //			CFR_DBG_OUT( "CMBS_TASK_USB_LINE_STATUS\n" );
      break;

#if defined( CMBS_CATIQ )
   case  CMBS_TASK_FACILITY_REQ:
   {
      PST_CMBS_FACILITY pst_Facility;
      // CFR_DBG_OUT( "CMBS_TASK_FACILITY_REQ\n" );

      pst_Facility = (PST_CMBS_FACILITY)tapp_util_FifoGet( &g_FacilityReqFifo );

      if( pst_Facility )
      {
         if( pst_Facility->u8_State == 0 )   // new, unused facility request
         {
            pst_Facility->u8_State = tapp_util_FacilityReq( pst_Facility );
/*
            CFR_DBG_OUT( "CMBS_TASK_FACILITY_REQ RequestId:%d\n",
                         pst_Facility->u16_RequestId );
*/
            if( pst_Facility->u8_State == 0 )
            {
               // facility request couldn't be processed correctly;
               // we remove it from fifo.
               tapp_util_FifoPop( &g_FacilityReqFifo );
            }
         }
//         else
//            CFR_DBG_OUT( "CMBS_TASK_FACILITY_REQ RequestId:%d in use\n",
//                          pst_Facility->u16_RequestId );
      }
   }
      break;

   case  CMBS_TASK_FACILITY_CNF:
   {
      PST_CMBS_FACILITY pst_Facility;
      // CFR_DBG_OUT( "CMBS_TASK_FACILITY_CNF\n" );

      pst_Facility = (PST_CMBS_FACILITY)tapp_util_FifoPop( &g_FacilityReqFifo );

      if( pst_Facility )
      {
         if( pst_Facility->u8_State == 1 )
         {
/*
            CFR_DBG_OUT( "CMBS_TASK_FACILITY_CNF RequestId:%d Hs:0x%02x\n",
                          pst_Facility->u16_RequestId,
                          G_st_os00_Act.u16_XInfo );
*/
            pst_Facility->u8_State = 2;

            tapp_util_FacilityCnf( pst_Facility, G_st_os00_Act.u16_XInfo );
                                    // meanwhile, there might be another request in the FIFO.
                                    // retrigger a facility request
            p_os10_PutMsg( CMBSTASK_ID, 0, CMBS_TASK_FACILITY_REQ );
         }
         else
         {
            CFR_DBG_OUT( "Facility Request Id:%d not in use !!!!!\n" );
         }
      }
   }
      break;
#endif // defined( CMBS_CATIQ )

   case  CMBS_TASK_TIMED_REL_LINE:
   {
	PST_CMBS_APP_LINE
					  pst_Line;
	
   	CFR_DBG_ERROR( "\nCMBS_TASK_TIMED_REL_LINE: CallRelease timed out !!!!\n" );
	pst_Line = tapp_util_LineGet( &g_CMBSApp, TCMBS_APP_LINE_ILLEGAL, (u8)G_st_os00_Act.u16_XInfo );
	
	if( pst_Line )
	{
		cmbs_dee_CallReleaseComplete( NULL/*pv_AppRefHandle*/, pst_Line->u32_CallInstance );
		tapp_util_LineFree( pst_Line );
	}
	else
	{
		CFR_DBG_ERROR( "\nCMBS_TASK_TIMED_REL_LINE: No line found for CallSessionID %u !!!!\n",  G_st_os00_Act.u16_XInfo);
	}
    break;
   }

/*
   case FTMI_RINGER_ON_EXP:
   {
   	PST_CFR_IE_LIST p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

      cmbs_int_ie_IntValueAdd( (void*)p_List, 0 );

      tapp_dsr_HandsetPage( &g_CMBSApp, p_List );
   }
      break;
*/
      //		========== T ===========
      /*!
      \brief				 short description

      \param[in,out]		 CMBS_TASK_KEYBOARD_HIT:		 description

      \return				     case CMBS_TASK_KEYBOARD_HI			 description

      */

#if defined (CMBS_AUDIO_TEST_SLAVE)
   case CMBS_TASK_KEYBOARD_HIT:
//      printf ( " KEY %02x\n", (u8)G_st_os00_Act.u16_XInfo);
      cmbsAudioSlaveChange ( (u8)G_st_os00_Act.u16_XInfo );
      break;
#endif
   default:
      CFR_DBG_INFO( "CMBS-TASK: default ev handler %d\n", G_st_os00_Act.u8_Event );
   }
}

//*/
