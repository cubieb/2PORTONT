/*!
*  \file       cmbs_int.c
*  \brief      Internal api functions
*  \author     stein
*
*  @(#)  %filespec: cmbs_int.c-1 %
*
*******************************************************************************
*  \par  History
*  \n==== History ============================================================\n
*  date        name     version   action                                          \n
*  ----------------------------------------------------------------------------\n
*  06-Feb-09   stein    1     Initialize \n
*  16-Feb-09   kelbch   2     Integration to host/target build\n
*  25-Feb-09   stein    61    Restructuration                             \n
*  09-Apr-09   Kelbch   161   Update of Media Configuration during start-up \n
*  14-Dec-09   sergiym   ?    Add start/stop events \n
*******************************************************************************
*  COPYRIGHT DOSCH & AMAND RESEARCH GMBH & CO.KG
*  DOSCH & AMAND RESEARCH GMBH & CO.KG Confidential
*
*******************************************************************************/

#if defined(__arm)
# include "tclib.h"
# include "embedded.h"
#else
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
#endif

#if defined( __linux__ )
#include <pthread.h>
#endif

#include "cmbs_int.h"
#include "cfr_uart.h"

#include "cfr_debug.h"

#ifdef CONFIG_RTK_VOIP_DECT_SPI_SUPPORT
/* THLin Add for SPI */
#include "bus.h"
#endif

#if defined(CMBS_API_TARGET)
#include "csys0reg.h"
#include "cos00int.h"                // Needed for critical section
#include "tapp_log.h"
#endif // defined(CMBS_API_TARGET)

/* Global variables */

/* Local variables */

//static void       _cmbs_int_MsgCommandPrint( u16 u16_Len, u16 u16_MsgNr, u16 u16_Event );
u8                _cmbs_int_FeatureSupported( E_CMBS_EVENT_ID e_EventID );
char *			  _dbgCMBSCMDPrint( CMBS_CMD id );

#if !defined( CMBS_API_TARGET )
char *            _dbgCMBSEVPrint( E_CMBS_EVENT_ID id );
#endif

/*****************************************************************************
 * API Internal functions
 *****************************************************************************/

u8                _cmbs_int_FeatureSupported( E_CMBS_EVENT_ID e_EventID )
{
#if defined( CMBS_CATIQ )

   if( g_CMBSInstance.u16_TargetVersion < 0x0202 &&
       ( e_EventID == CMBS_EV_DSR_GEN_SEND_VMWI         ||
         e_EventID == CMBS_EV_DSR_GEN_SEND_MISSED_CALLS ||
         e_EventID == CMBS_EV_DSR_GEN_SEND_LIST_CHANGED ||
         e_EventID == CMBS_EV_DSR_GEN_SEND_WEB_CONTENT  ||
         e_EventID == CMBS_EV_DSR_GEN_SEND_PROP_EVENT   ||
         e_EventID == CMBS_EV_DSR_TIME_UPDATE ) )
   {
      return   FALSE;
   }

   return   TRUE;

#else

   return FALSE;

#endif // defined( CMBS_CATIQ )
}


E_CMBS_ENDIAN     cmbs_int_EndiannessGet( void )
{
   int            i = 0x0A0B0C0D;
   char *         p = (char *)&i;

   switch( p[0] )
   {
      case  0x0A:
         CFR_DBG_OUT( "Big endian: 0x%08x = 0x%02x 0x%02x 0x%02x 0x%02x\n",
                        i, p[0], p[1], p[2], p[3] );
         return E_CMBS_ENDIAN_BIG;

      case  0x0B:
         CFR_DBG_OUT( "Mixed endian: 0x%08x = 0x%02x 0x%02x 0x%02x 0x%02x\n",
                        i, p[0], p[1], p[2], p[3] );
         return E_CMBS_ENDIAN_MIXED;

      default:
         CFR_DBG_OUT( "Little endian: 0x%08x = 0x%02x 0x%02x 0x%02x 0x%02x\n",
                        i, p[0], p[1], p[2], p[3] );
         return E_CMBS_ENDIAN_LITTLE;
   }
}

/* Convert endianess for 16 bit value */
u16               cmbs_int_EndianCvt16( u16 u16_Value )
{
   return   ( ((u16_Value & 0x00FF) << 8) |
              ((u16_Value & 0xFF00) >> 8) );
}

/* Convert endianess for 32 bit value */
u32               cmbs_int_EndianCvt32( u32 u32_Value )
{
   return   ( ((u32_Value & 0x000000FF) << 24) |
              ((u32_Value & 0x0000FF00) << 8)  |
              ((u32_Value & 0x00FF0000) >> 8)  |
              ((u32_Value & 0xFF000000) >> 24) );
}


E_CMBS_RC         cmbs_int_ParamValid( E_CMBS_PARAM e_Param, u8 * pu8_Data, u16 u16_DataLen )
{
#if !defined( CMBS_API_TARGET )
   u8             u8_Length = 0;

   if( pu8_Data ){
   }  // No data validation at the moment; eliminate compiler warnings

   switch( e_Param )
   {
      case  CMBS_PARAM_RFPI:
         u8_Length = CMBS_PARAM_RFPI_LENGTH;
         break;

      case  CMBS_PARAM_RVBG:
         u8_Length = CMBS_PARAM_RVBG_LENGTH;
         break;

      case  CMBS_PARAM_RVREF:
         u8_Length = CMBS_PARAM_RVREF_LENGTH;
         break;

      case  CMBS_PARAM_RXTUN:
         u8_Length = CMBS_PARAM_RXTUN_LENGTH;
         break;

      case  CMBS_PARAM_MASTER_PIN:
      case  CMBS_PARAM_AUTH_PIN:
         u8_Length = CMBS_PARAM_PIN_CODE_LENGTH;
         break;

      case  CMBS_PARAM_COUNTRY:
         u8_Length = CMBS_PARAM_COUNTRY_LENGTH;
         break;

      case  CMBS_PARAM_SIGNALTONE_DEFAULT:
         u8_Length = CMBS_PARAM_SIGNALTONE_LENGTH;
         break;

      case  CMBS_PARAM_TEST_MODE:
         u8_Length = CMBS_PARAM_TEST_MODE_LENGTH;
         break;

      case  CMBS_PARAM_AUTO_REGISTER:
         u8_Length = CMBS_PARAM_AUTO_REGISTER_LENGTH;
         break;
      
      case  CMBS_PARAM_NTP:
         u8_Length = CMBS_PARAM_NTP_LENGTH;
         break;
      
      case  CMBS_PARAM_RESET_ALL:
         u8_Length = 1;
         break;
         
	  case CMBS_PARAM_ECO_MODE:
         u8_Length = CMBS_PARAM_ECO_MODE_LENGTH;
         break;
         
      default:
         return CMBS_RC_ERROR_PARAMETER;
   }

   // Check parameter length
   if( u16_DataLen != u8_Length )
   {
      CFR_DBG_ERROR( "Parameter ERROR: Length mismatch. Required:%d <-> Got:%d\n",
                     u8_Length, u16_DataLen );

      return CMBS_RC_ERROR_PARAMETER;
   }
#endif   // !defined( CMBS_API_TARGET )
   return   CMBS_RC_OK;
}


void *cmbs_int_RegisterCb( void * pv_AppRef, PFN_CMBS_API_CB pfn_api_Cb, u16 u16_bcdVersion )
{
   PST_CMBS_API_SLOT p_Slot;
   /*! \todo multiple slot handling - get free slot */
   p_Slot = &g_CMBSInstance.st_ApplSlot;

   if ( u16_bcdVersion ){
   } /*! \todo handle of interoperability, currently not needed */

   if ( p_Slot->pFnAppCb )
   {
      CFR_DBG_ERROR ( "cmbs_int_RegisterCb: !!!! No free application slot available\n" );

      return NULL;
   }
   p_Slot->pFnAppCb = pfn_api_Cb;
   p_Slot->pv_AppRefHandle = pv_AppRef;
   p_Slot->u16_AppAPIVersion = u16_bcdVersion;

   return (void*) p_Slot;
}


void              cmbs_int_RegisterLogBufferCb( void * pv_AppRef, PST_CB_LOG_BUFFER pfn_log_buffer_Cb )
{
   PST_CMBS_API_SLOT p_Slot;

   if( pv_AppRef ){
   }  // unused. suppress compiler warnings

   /*! \todo multiple slot handling - get free slot */
   p_Slot = &g_CMBSInstance.st_ApplSlot;

   memcpy( &p_Slot->pFnCbLogBuffer, pfn_log_buffer_Cb, sizeof(ST_CB_LOG_BUFFER) );
}


void              cmbs_int_UnregisterCb( void * pv_AppRefHandle )
{
   PST_CMBS_API_SLOT p_Slot;
   /*! \todo multiple slot handling - get free slot */
   p_Slot = &g_CMBSInstance.st_ApplSlot;

   if ( p_Slot->pv_AppRefHandle == pv_AppRefHandle )
   {
      // clean-up entry
      memset ( p_Slot, 0, sizeof(ST_CMBS_API_SLOT));
   }
}

/*!
   \brief   returns CMBS API target version
#endif
}


*/
u16               cmbs_int_ModuleVersionGet( void )
{
   /*! \todo target version is received by HELLO_RPLY command */
   return g_CMBSInstance.u16_TargetVersion;
}

u16               cmbs_int_ModuleVersionBuildGet( void )
{
   /*! \todo target version is received by HELLO_RPLY command */
   return g_CMBSInstance.u16_TargetBuild;
}


/*!
   \brief   CMBS event is received, prepare for callback function
  \param[in]      pu8_Mssg    pointer to message without sync dword
  \param[in]      u16_Size size of message without sync dword

  \return         <none>

*/
void              cmbs_int_EventReceive ( u8 * pu8_Mssg, u16 u16_Size )
{
   PST_CMBS_SER_MSG p_Mssg = (PST_CMBS_SER_MSG)pu8_Mssg;
   u16 u16_SizeMinusMsgHdr;

#if !defined( CMBS_API_TARGET )
   CFR_DBG_OUT( "CMBS-API: cmbs_int_EventReceive evcmd:0x%x = %s,",
                 p_Mssg->st_MsgHdr.u16_EventID, _dbgCMBSEVPrint(p_Mssg->st_MsgHdr.u16_EventID) );
   if( p_Mssg->st_MsgHdr.u16_ParamLength )
   {
      int i;
      CFR_DBG_OUT( " bytes:%d data: ", p_Mssg->st_MsgHdr.u16_ParamLength );
      for( i=0; i < p_Mssg->st_MsgHdr.u16_ParamLength; i++ )
      {
         CFR_DBG_OUT( "%02x ", p_Mssg->u8_Param[i] );
         if( i == 31 )
         {
            CFR_DBG_OUT( "more..." );
            break;
         }
      }
   }
   CFR_DBG_OUT( "\n" );
#endif

   if ( (p_Mssg->st_MsgHdr.u16_EventID & CMBS_CMD_MASK) == CMBS_CMD_MASK)
   {
      cmbs_int_CmdTrace ( pu8_Mssg, u16_Size );

      u16_SizeMinusMsgHdr = u16_Size - sizeof(ST_CMBS_SER_MSGHDR);

      cmbs_int_cmd_Dispatcher( (u8)p_Mssg->st_MsgHdr.u16_EventID, (u8*)p_Mssg->u8_Param, u16_SizeMinusMsgHdr );
   }
   else
   {
//      cmbs_int_EventTrace ( pu8_Mssg, u16_Size );

      if( g_CMBSInstance.st_ApplSlot.pFnAppCb)
      {
                                 // special case fw update: take whole message
         if( p_Mssg->st_MsgHdr.u16_EventID == CMBS_EV_DSR_FW_UPD_START ||
             p_Mssg->st_MsgHdr.u16_EventID == CMBS_EV_DSR_FW_UPD_PACKETNEXT ||
             p_Mssg->st_MsgHdr.u16_EventID == CMBS_EV_DSR_FW_UPD_END )
         {
            g_CMBSInstance.st_ApplSlot.pFnAppCb( g_CMBSInstance.st_ApplSlot.pv_AppRefHandle,
                                                 p_Mssg->st_MsgHdr.u16_EventID, (void*) pu8_Mssg );
         }
         else                    // standard message with IE list
         {
            ST_CFR_IE_LIST List;
            /*! \todo check if structure fits into calculation work! */
            List.pu8_Buffer = (u8*)p_Mssg->u8_Param;
            List.u16_CurIE  = 0;
            List.u16_CurSize= u16_Size - sizeof(ST_CMBS_SER_MSGHDR);
            List.u16_MaxSize= u16_Size - sizeof(ST_CMBS_SER_MSGHDR);

            g_CMBSInstance.st_ApplSlot.pFnAppCb( g_CMBSInstance.st_ApplSlot.pv_AppRefHandle,
                                                 p_Mssg->st_MsgHdr.u16_EventID, (void*) &List );
         }
      }
      else
      {
         CFR_DBG_ERROR ( "CMBS-EventReceive ERROR: callback application is not available\n" );
      }
   }
}

void              cmbs_int_HdrEndianCvt( ST_CMBS_SER_MSGHDR *pst_Hdr )
{
   pst_Hdr->u16_TotalLength = cmbs_int_EndianCvt16(pst_Hdr->u16_TotalLength);
   pst_Hdr->u16_PacketNr    = cmbs_int_EndianCvt16(pst_Hdr->u16_PacketNr);
   pst_Hdr->u16_EventID     = cmbs_int_EndianCvt16(pst_Hdr->u16_EventID);
   pst_Hdr->u16_ParamLength = cmbs_int_EndianCvt16(pst_Hdr->u16_ParamLength);
}

//		========== _cmbs_int_Send  ===========
/*!
		\brief				 send packet via communication module

		\param[in,out]		 u16_Event	 event identifier with command indicator, if needed

		\param[in,out]		 pu8_Buf		 pointer to parameter buffer

		\param[in,out]		 u16_Length	 parameter buffer size

		\return				 <E_CMBS_RC>

*/

E_CMBS_RC        _cmbs_int_Send ( u16 u16_Event, u8 * pBuf, u16 u16_Length )
{
   ST_CMBS_SER_MSGHDR
      msgHdr;
   u32            u32_Sync = CMBS_SYNC;
   u16            u16_Size;

   memset( &msgHdr, 0, sizeof(msgHdr) );

   msgHdr.u16_TotalLength = sizeof( ST_CMBS_SER_MSGHDR ) + u16_Length;
   msgHdr.u16_PacketNr    = 0;
   msgHdr.u16_EventID     = u16_Event;
   msgHdr.u16_ParamLength = u16_Length;

   u16_Size = sizeof(u32_Sync) + sizeof( msgHdr) + u16_Length;

   // From now on we need exclusive access to transmission of serial port
   CFR_CMBS_ENTER_CRITICALSECTION( g_CMBSInstance.h_CriticalSectionTransmission );

   if( CFR_E_RETVAL_OK == cfr_uartPacketPrepare( u16_Size ) )
   {
   	cfr_uartPacketPartWrite( (u8 *)&u32_Sync, sizeof(u32_Sync) );

      if( g_CMBSInstance.e_Endian != E_CMBS_ENDIAN_LITTLE )
      {
         cmbs_int_HdrEndianCvt( &msgHdr );
      }

      cfr_uartPacketPartWrite( (u8 *)&msgHdr, sizeof( msgHdr ) );

#if !defined( CMBS_API_TARGET )
      CFR_DBG_OUT( "Sending %s",
         (u16_Event & CMBS_CMD_MASK) ? _dbgCMBSCMDPrint(u16_Event & ~CMBS_CMD_MASK) : _dbgCMBSEVPrint(u16_Event) );
#endif
      if( pBuf && u16_Length )
      {
#if !defined( CMBS_API_TARGET )
         int i;
         CFR_DBG_OUT( ": " );
         for (i=0; i< u16_Length; i++ )
         {
            CFR_DBG_OUT( "%02x ",pBuf[i] );
         }
         CFR_DBG_OUT( "\n" );
#endif // CMB_API_TARGET
      	cfr_uartPacketPartWrite( pBuf, u16_Length );
      }

      cfr_uartPacketWriteFinish(CFR_BUFFER_UART_TRANS );
      cfr_uartDataTransmitKick ();

      // We have sent the complete packet, leave critical section
      CFR_CMBS_LEAVE_CRITICALSECTION( g_CMBSInstance.h_CriticalSectionTransmission );

      return   CMBS_RC_OK;
   }
   else
   {
//      CFR_DBG_OUT( "In _cmbs_int_Send\n" );
   }

   // To avoid deadlock in case of error, leave critical section
   CFR_CMBS_LEAVE_CRITICALSECTION( g_CMBSInstance.h_CriticalSectionTransmission );

#if !defined( CMBS_API_TARGET )
   CFR_DBG_ERROR( "cmbs_int_Send: !!!! transmit buffer full\n" );
#endif // CMB_API_TARGET

   return CMBS_RC_ERROR_OUT_OF_MEM;
}

#ifdef CONFIG_RTK_VOIP_DECT_SPI_SUPPORT /* THLin Add for SPI */
E_CMBS_RC _cmbs_int_Spi_Send ( u16 u16_Event, u8 * pBuf, u16 u16_Length )
{
   ST_CMBS_SER_MSGHDR msgHdr;
   u32 u32_Sync = CMBS_SYNC;
   u8 u8_pktLen = 0;
   u32 u32_pktLen = 0;
   u8 pkt_tmp[255];

   memset( &msgHdr, 0, sizeof(msgHdr) );
   memset( pkt_tmp, 0, 255 );

   msgHdr.u16_TotalLength = sizeof( ST_CMBS_SER_MSGHDR ) + u16_Length;
   msgHdr.u16_PacketNr    = 0;
   msgHdr.u16_EventID     = u16_Event;
   msgHdr.u16_ParamLength = u16_Length;
   
   u8_pktLen = sizeof(u32_Sync) + msgHdr.u16_TotalLength;
   u32_pktLen = sizeof(u32_Sync) + msgHdr.u16_TotalLength;
   
   if ( u32_pktLen > 255)
		printf(" >>>>>> _cmbs_int_Send: u32_pktLen > 255, too large!\n");

   // From now on we need exclusive access to transmission of serial port
   CFR_CMBS_ENTER_CRITICALSECTION( g_CMBSInstance.h_CriticalSectionTransmission );


   if( g_CMBSInstance.e_Endian != E_CMBS_ENDIAN_LITTLE )
   {
      cmbs_int_HdrEndianCvt( &msgHdr );
   }

   memcpy((u8*)pkt_tmp, &u8_pktLen, 1);			//cp pkt len
   memcpy((u8*)pkt_tmp+1, &u32_Sync, 4);			//cp sync word
   memcpy((u8*)pkt_tmp+1+4, &msgHdr, sizeof(msgHdr));		//cp msgHdr
   memcpy((u8*)pkt_tmp+1+4+sizeof(msgHdr), pBuf, u16_Length);	//cp para data   
   
   Bus_Write_byte_blocking(pkt_tmp, u8_pktLen+1);

#if 1
   int i;
   CFR_DBG_OUT("TX: ");
   for (i=0; i < (u8_pktLen+1); i++)
   {
   	CFR_DBG_OUT("%02x ", pkt_tmp[i]);
   }
   CFR_DBG_OUT(", len = %d\n", u8_pktLen);
#endif
   
   /***** DEBUG Start *****/

#if !defined( CMBS_API_TARGET )
      CFR_DBG_OUT( "Sending %s",
         (u16_Event & CMBS_CMD_MASK) ? _dbgCMBSCMDPrint(u16_Event & ~CMBS_CMD_MASK) : _dbgCMBSEVPrint(u16_Event) );
#endif
      if( pBuf && u16_Length )
      {
#if !defined( CMBS_API_TARGET )
         int i;
         CFR_DBG_OUT( ": " );
         for (i=0; i< u16_Length; i++ )
         {
            CFR_DBG_OUT( "%02x ",pBuf[i] );
         }
         CFR_DBG_OUT( "\n" );
#endif // CMB_API_TARGET
      }
      else
         CFR_DBG_OUT( "\n" );
      
   /***** DEBUG End *****/

      // We have sent the complete packet, leave critical section
      CFR_CMBS_LEAVE_CRITICALSECTION( g_CMBSInstance.h_CriticalSectionTransmission );

      return   CMBS_RC_OK;

   // To avoid deadlock in case of error, leave critical section
   CFR_CMBS_LEAVE_CRITICALSECTION( g_CMBSInstance.h_CriticalSectionTransmission );

#if 0
#if !defined( CMBS_API_TARGET )
   CFR_DBG_ERROR( "cmbs_int_Send: !!!! transmit buffer full\n" );
#endif // CMB_API_TARGET

   return CMBS_RC_ERROR_OUT_OF_MEM;
#endif
}
#endif /* THLin Add for SPI */

//		========== cmbs_int_EventSend ===========
/*!
		\brief				 Send complete event message: header + parameter data to communication device

		\param[in,out]		 e_EventID		 CMBS Event ID

		\param[in,out]		 pBuf		       pointer parameter buffer

		\param[in,out]		 u16_Length		 size of parameter buffer

		\return				< E_CMBS_RC >

*/

E_CMBS_RC         cmbs_int_EventSend( E_CMBS_EVENT_ID e_EventID, u8 * pBuf, u16 u16_Length )
{
   if( !_cmbs_int_FeatureSupported(e_EventID) )
   {
#if !defined( CMBS_API_TARGET )
      CFR_DBG_ERROR( "Feature Not supported: %s\n",  _dbgCMBSEVPrint(e_EventID) );
#else
      CFR_DBG_ERROR( "Feature Not supported: %d\n",  e_EventID );
#endif

      return CMBS_RC_ERROR_NOT_SUPPORTED;
   }

   if ( cmbs_int_cmd_FlowStateGet() == E_CMBS_FLOW_STATE_GO )
   {
#ifdef CONFIG_RTK_VOIP_DECT_UART_SUPPORT	//Uart
      return _cmbs_int_Send ( (u16) e_EventID, pBuf, u16_Length );
#else	//SPI /* THLin Add for SPI */
      return _cmbs_int_Spi_Send ( (u16) e_EventID, pBuf, u16_Length );
#endif
   }

   return CMBS_RC_ERROR_MEDIA_BUSY;
}

//		========== cmbs_int_cmd_Send ===========
/*!
		\brief				 Send complete internal command message: header + parameter data to communication device

		\param[in]        u8_Cmd         internal command ID

		\param[in]        pBuf		       pointer parameter buffer

		\param[in]        u16_Length		 size of parameter buffer

		\return				< E_CMBS_RC >

*/

E_CMBS_RC cmbs_int_cmd_Send( u8 u8_Cmd, u8 * pBuf, u16 u16_Length )
{
#ifdef CONFIG_RTK_VOIP_DECT_UART_SUPPORT	//Uart
	return _cmbs_int_Send ( (u16)(0xFF00 | u8_Cmd), pBuf, u16_Length );
#else	//SPI /* THLin Add for SPI */
	return _cmbs_int_Spi_Send ( (u16)(0xFF00 | u8_Cmd), pBuf, u16_Length );
#endif
}


#if ( !defined(__linux__) && !defined(WIN32) && !defined(DNA_BOOTING))

#include "bsd02eep.h"

#if defined( CMBS_CATIQ )
#include "bsd09cnf.h"
#include "bhl00int.h"
#include "bcs00int.h"
#endif

extern u8 g_u8_HsSubData[SD02_SUB_LEN];

E_CMBS_RC         cmbs_int_OnHsRegistration( void )
{
   ST_IE_HANDSETINFO st_HandsetInfo;
   PST_CFR_IE_LIST   p_List;

   memset( &st_HandsetInfo, 0, sizeof(ST_IE_HANDSETINFO) );

   st_HandsetInfo.u8_Hs    = g_u8_HsSubData[38] & 0x0F;   // take only the least significant nibble
   st_HandsetInfo.u8_State = g_u8_HsSubData[37];
   memcpy( st_HandsetInfo.u8_IPEI, g_u8_HsSubData, sizeof(st_HandsetInfo.u8_IPEI) );

#if defined( CMBS_CATIQ )
   if( ( G_st_hl00_TermCapability[st_HandsetInfo.u8_Hs].u8_ProfileIndicator7 &
                                 CS00_CATIQ_2_0_MASK ) == CS00_CATIQ_2_0_MASK )
   {
      st_HandsetInfo.e_Type = CMBS_HS_TYPE_CATIQ_2;
   }
   else if( ( G_st_hl00_TermCapability[st_HandsetInfo.u8_Hs].u8_ProfileIndicator7 &
                                 CS00_CATIQ_1_0_MASK ) == CS00_CATIQ_1_0_MASK )
   {
      st_HandsetInfo.e_Type = CMBS_HS_TYPE_CATIQ_1;
   }
   else
#endif
   {
      st_HandsetInfo.e_Type = CMBS_HS_TYPE_GAP;
   }

   p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   cmbs_int_ie_HandsetInfoAdd( (void*)p_List, &st_HandsetInfo );

   return cmbs_int_EventSend( CMBS_EV_DSR_HS_REGISTERED, p_List->pu8_Buffer, p_List->u16_CurSize );
}
#endif


void              cmbs_int_ie_HandsetInfoAdd( void * pv_RefIEList, ST_IE_HANDSETINFO * pst_HandsetInfo )
{
   u8 u8_Buffer[16];
   u16 u16_Size;

   u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_HANDSETINFO;
   u8_Buffer[CFR_IE_SIZE_POS] = 12;

   u8_Buffer[2] = pst_HandsetInfo->u8_Hs;
   u8_Buffer[3] = pst_HandsetInfo->u8_State;
   u8_Buffer[4] = (u8)pst_HandsetInfo->e_Type;

   cfr_ie_ser_u32( u8_Buffer + 5, pst_HandsetInfo->u32_ModelNr );

   memcpy( u8_Buffer + 9, pst_HandsetInfo->u8_IPEI, sizeof(pst_HandsetInfo->u8_IPEI) );

   u16_Size = u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE;

   cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u16_Size );
}


void              cmbs_int_ie_ParameterAdd( void * pv_RefIEList, ST_IE_PARAMETER * pst_Param )
{
   u16            u16_Size = 4;     // minimum = sizeof(e_Param + e_ParamType +  u16_DataLen)
   u8             u8_Idx   = 3;     // offset of IE data after: 8bit IE type + 16bit IE size                                    
   u8             u8_Buffer[519];   // u16_Size + u8_Idx + max var. param size 512

   memset( u8_Buffer, 0, sizeof(u8_Buffer) );

                                    // byte order: [IE][16bit IE size][IE structure data...]
   u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_PARAMETER;

                                    // IE size will be updated later
   u8_Buffer[u8_Idx++] = (u8)pst_Param->e_Param;
   u8_Buffer[u8_Idx++] = (u8)pst_Param->e_ParamType;

   u8_Idx += cfr_ie_ser_u16( u8_Buffer + u8_Idx, pst_Param->u16_DataLen );

   if( pst_Param->pu8_Data )
   {
                                    // also add data length to IE size
      u16_Size += cfr_ie_ser_pu8( u8_Buffer + u8_Idx, pst_Param->pu8_Data, pst_Param->u16_DataLen );
   }
                                    // update IE size
   cfr_ie_ser_u16( u8_Buffer + CFR_IE_SIZE_POS, u16_Size );

                                    // total length = 8bit IE type + 16bit IE size + IE size
   cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u16_Size + 3 );
   }


void              cmbs_int_ie_ParameterAreaAdd( void * pv_RefIEList, ST_IE_PARAMETER_AREA * pst_ParamArea )
{
   u16            u16_Size = 5;     // minimum = sizeof(e_AreaType + u16_Offset + u16_DataLen)
   u8             u8_Idx   = 3;     // offset of IE data after: 8bit IE type + 16bit IE size                                    
   u8             u8_Buffer[520];   // u16_Size + u8_Idx + max var. param size 512

   memset( u8_Buffer, 0, sizeof(u8_Buffer) );

                                    // byte order: [IE][16bit IE size][IE structure data...]
   u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_PARAMETER_AREA;

                                    // IE size will be updated later
   u8_Buffer[u8_Idx++] = (u8)pst_ParamArea->e_AreaType;

   u8_Idx += cfr_ie_ser_u16( u8_Buffer + u8_Idx, pst_ParamArea->u16_Offset );
   u8_Idx += cfr_ie_ser_u16( u8_Buffer + u8_Idx, pst_ParamArea->u16_DataLen );

   if( pst_ParamArea->pu8_Data )
   {
                                    // also add data length to IE size
      u16_Size += cfr_ie_ser_pu8( u8_Buffer + u8_Idx, pst_ParamArea->pu8_Data, pst_ParamArea->u16_DataLen );
   }
                                    // update IE size
   cfr_ie_ser_u16( u8_Buffer + CFR_IE_SIZE_POS, u16_Size );

                                    // total length = 8bit IE type + 16bit IE size + IE size
   cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u16_Size + 3 );
}


void              cmbs_int_ie_ResponseAdd( void * pv_RefIEList, ST_IE_RESPONSE * pst_Response )
{
   u8 u8_Buffer[8];
   u16 u16_Size;

   u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_RESPONSE;
   u8_Buffer[CFR_IE_SIZE_POS] = sizeof(u8);

   u8_Buffer[CFR_IE_HEADER_SIZE] = pst_Response->e_Response;

   u16_Size = u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE;

   cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u16_Size );
}


E_CMBS_RC         cmbs_int_ResponseSend( E_CMBS_EVENT_ID e_ID, E_CMBS_RESPONSE e_RSPCode )
{
   PST_CFR_IE_LIST p_List;
   ST_IE_RESPONSE  st_Response;

   st_Response.e_Response = e_RSPCode;

   p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   cmbs_int_ie_ResponseAdd( (void*)p_List, &st_Response );

   return cmbs_int_EventSend( e_ID, p_List->pu8_Buffer, p_List->u16_CurSize );
}


E_CMBS_RC         cmbs_int_ie_DisplayStringGet( void * pv_RefIE, ST_IE_DISPLAY_STRING * pst_DisplayString )
{
   u8 * pu8_Buffer = (u8*)pv_RefIE;

   if( *pu8_Buffer == CMBS_IE_DISPLAY_STRING )
   {
      pst_DisplayString->u8_DataLen = pu8_Buffer[CFR_IE_HEADER_SIZE];
      pst_DisplayString->pu8_Info   = pu8_Buffer + CFR_IE_HEADER_SIZE + sizeof(u8);

   	return CMBS_RC_OK;
   }

   return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_int_ie_ToneGet( void * pv_RefIE, ST_IE_TONE * pst_Tone )
{
   u8 * pu8_Buffer = (u8*)pv_RefIE;

   if( pu8_Buffer[CFR_IE_TYPE_POS] == CMBS_IE_TONE )
   {
      pst_Tone->e_Tone = pu8_Buffer[CFR_IE_HEADER_SIZE];

   	return CMBS_RC_OK;
   }

   return CMBS_RC_ERROR_PARAMETER;
}


void              cmbs_int_ie_IntValueAdd( void * pv_RefIEList, u32 u32_Value )
{
   u8  u8_Buffer[8];
   u16            u16_Size;

   u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_INTEGER_VALUE;
   u8_Buffer[CFR_IE_SIZE_POS] = sizeof(u32);

   cfr_ie_ser_u32( u8_Buffer + CFR_IE_HEADER_SIZE, u32_Value );

   u16_Size = u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE;

   cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u16_Size );
}


E_CMBS_RC         cmbs_int_ie_IntValueGet( void * pv_RefIE, u32 * pu32_Value )
{
   u8 * pu8_Buffer = (u8*)pv_RefIE;

   if ( *pu8_Buffer == CMBS_IE_INTEGER_VALUE )
   {
      cfr_ie_dser_u32( pu8_Buffer + CFR_IE_HEADER_SIZE, pu32_Value );

      return CMBS_RC_OK;
   }

   return CMBS_RC_ERROR_PARAMETER;

}


E_CMBS_RC         cmbs_int_ie_ByteValueAdd( void * pv_RefIEList, u8 u8_Value, E_CMBS_IE_TYPE e_IETYPE )
{
   u8             u8_Buffer[3];

   u8_Buffer[CFR_IE_TYPE_POS]    = (u8)e_IETYPE;
   u8_Buffer[CFR_IE_SIZE_POS]    = 1;
   u8_Buffer[CFR_IE_HEADER_SIZE] = u8_Value;

   cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, 3 );

   return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_int_ie_ByteValueGet( void * pv_RefIE, u8 * pu8_Value )
{
   u8 *           pu8_Buffer = (u8*)pv_RefIE;
   *pu8_Value = pu8_Buffer[CFR_IE_HEADER_SIZE];

   return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_int_ie_ShortValueAdd( void * pv_RefIEList, u16 u16_Value, E_CMBS_IE_TYPE e_IETYPE )
{
   u8             u8_Buffer[8];
   u16            u16_Size;

   if( e_IETYPE != CMBS_IE_REQUEST_ID      &&
       e_IETYPE != CMBS_IE_HANDSETS        &&
       e_IETYPE != CMBS_IE_LA_LIST_ID      &&
       e_IETYPE != CMBS_IE_LA_ENTRY_ID     &&
       e_IETYPE != CMBS_IE_LA_ENTRY_COUNT  &&
       e_IETYPE != CMBS_IE_LA_ENTRY_INDEX  &&
       e_IETYPE != CMBS_IE_LA_SESSION_ID   &&
       e_IETYPE != CMBS_IE_DATA_SESSION_ID &&
       e_IETYPE != CMBS_IE_LA_NR_OF_ENTRIES )
   {
      CFR_DBG_ERROR( "cmbs_int_ie_ShortValueAdd ERROR: IE_TYPE %d no 16 bit value\n", e_IETYPE );
      return CMBS_RC_ERROR_PARAMETER;
   }

   u8_Buffer[CFR_IE_TYPE_POS] = (u8)e_IETYPE;
   u8_Buffer[CFR_IE_SIZE_POS] = sizeof(u16);

   cfr_ie_ser_u16( u8_Buffer + CFR_IE_HEADER_SIZE, u16_Value );

   u16_Size = u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE;

   cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u16_Size );

   return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_int_ie_ShortValueGet( void * pv_RefIE, u16 * pu16_Value )
{
   u8 *           pu8_Buffer = (u8*)pv_RefIE;

   if( *pu8_Buffer == CMBS_IE_REQUEST_ID      ||
       *pu8_Buffer == CMBS_IE_HANDSETS        ||
       *pu8_Buffer == CMBS_IE_LA_LIST_ID      ||
       *pu8_Buffer == CMBS_IE_LA_ENTRY_ID     ||
       *pu8_Buffer == CMBS_IE_LA_ENTRY_COUNT  ||
       *pu8_Buffer == CMBS_IE_LA_ENTRY_INDEX  ||
       *pu8_Buffer == CMBS_IE_LA_SESSION_ID   ||
       *pu8_Buffer == CMBS_IE_DATA_SESSION_ID ||
       *pu8_Buffer == CMBS_IE_LA_NR_OF_ENTRIES )
   {
      cfr_ie_dser_u16( pu8_Buffer + CFR_IE_HEADER_SIZE, pu16_Value );

      return CMBS_RC_OK;
   }

   return CMBS_RC_ERROR_PARAMETER;

}


void              cmbs_int_ie_FwVersionAdd( void * pv_RefIEList, ST_IE_FW_VERSION * pst_FwVersion )
{
   u8  u8_Buffer[8];
   u16 u16_Size;

   u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_FW_VERSION;
   u8_Buffer[CFR_IE_SIZE_POS] = sizeof(u8) + sizeof(u16);

   u8_Buffer[CFR_IE_HEADER_SIZE] = pst_FwVersion->e_SwModule;

   cfr_ie_ser_u16( u8_Buffer + CFR_IE_HEADER_SIZE + sizeof(u8), pst_FwVersion->u16_FwVersion );

   u16_Size = u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE;

   cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u16_Size );
}


void              cmbs_int_ie_SysLogAdd( void * pv_RefIEList, ST_IE_SYS_LOG * pst_SysLog )
{
   u8  u8_Buffer[1 + 1 + 128];
   u16 u16_Size;

   u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_SYS_LOG;

   u8_Buffer[CFR_IE_SIZE_POS] = pst_SysLog->u8_DataLen;

   cfr_ie_ser_pu8( u8_Buffer + CFR_IE_HEADER_SIZE, pst_SysLog->u8_Data, pst_SysLog->u8_DataLen );

   u16_Size = u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE;

   cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u16_Size );
}


E_CMBS_RC cmbs_int_ie_SysStatusSet( void * pv_RefIEList, ST_IE_SYS_STATUS * pst_SysStatus )
{
   u8 u8_Buffer[4];
   u16 u16_Size;

   memset( u8_Buffer, 0, sizeof(u8_Buffer) );

   u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_STATUS;
   u8_Buffer[CFR_IE_SIZE_POS] = sizeof(u8);

   u8_Buffer[CFR_IE_HEADER_SIZE] = (u8)pst_SysStatus->e_ModuleStatus;

   u16_Size = u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE;

   cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u16_Size );

   return CMBS_RC_OK;
}


void              cmbs_int_ie_GenEventAdd( void * pv_RefIEList, ST_IE_GEN_EVENT * pst_GenEvent )
{
   u8  u8_Buffer[16];
   u16 u16_Size;

   u8_Buffer[CFR_IE_TYPE_POS] = CMBS_IE_GEN_EVENT;
   u8_Buffer[CFR_IE_SIZE_POS] = 3;

   u8_Buffer[CFR_IE_SIZE_POS + 1] = pst_GenEvent->u8_SubType;
   u8_Buffer[CFR_IE_SIZE_POS + 2] = pst_GenEvent->u8_MultiPlicity;
   u8_Buffer[CFR_IE_SIZE_POS + 3] = pst_GenEvent->u8_LineId;

   u16_Size = u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE;

   cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u16_Size );
}


E_CMBS_RC         cmbs_int_ie_GenEventGet( void * pv_RefIE, ST_IE_GEN_EVENT * pst_GenEvent )
{
   u8 * pu8_Buffer = (u8*)pv_RefIE;

   if( *pu8_Buffer == CMBS_IE_GEN_EVENT )
   {
      pst_GenEvent->u8_SubType      = pu8_Buffer[CFR_IE_HEADER_SIZE];
      pst_GenEvent->u8_MultiPlicity = pu8_Buffer[CFR_IE_HEADER_SIZE + sizeof(u8)];
      pst_GenEvent->u8_LineId       = pu8_Buffer[CFR_IE_HEADER_SIZE + 2*sizeof(u8)];

   	return CMBS_RC_OK;
   }

   return CMBS_RC_ERROR_PARAMETER;
}


void              cmbs_int_ie_PropEventAdd( void * pv_RefIEList, u16 u16_PropEvent, u8 * pu8_Data, u8 u8_DataLen )
{
   u8  u8_Buffer[CFR_IE_HEADER_SIZE + 3 + CMBS_PROP_EVENT_LENGTH];
   u16 u16_Size;

   memset( u8_Buffer, 0, sizeof(u8_Buffer) );

   u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_PROP_EVENT;
   u8_Buffer[CFR_IE_SIZE_POS] = sizeof(u16) + sizeof(u8);

   cfr_ie_ser_u16( u8_Buffer + CFR_IE_HEADER_SIZE, u16_PropEvent );
   u8_Buffer[CFR_IE_HEADER_SIZE + sizeof(u16)] = u8_DataLen;

   if( u8_DataLen )
   {
      u16_Size = MIN( (u16)u8_DataLen, CMBS_PROP_EVENT_LENGTH );
      cfr_ie_ser_pu8( u8_Buffer + CFR_IE_HEADER_SIZE + u8_Buffer[CFR_IE_SIZE_POS], pu8_Data, u16_Size );

      u8_Buffer[CFR_IE_SIZE_POS] += u8_DataLen;
   }

   u16_Size = u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE;

   cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u16_Size );
}


E_CMBS_RC         cmbs_int_ie_PropEventGet( void * pv_RefIE, ST_IE_PROP_EVENT * pst_PropEvent )
{
   u8 * pu8_Buffer = (u8*)pv_RefIE;

   if( *pu8_Buffer == CMBS_IE_PROP_EVENT )
   {
      memset( pst_PropEvent, 0, sizeof(ST_IE_PROP_EVENT) );

      cfr_ie_dser_u16( pu8_Buffer + CFR_IE_HEADER_SIZE, &pst_PropEvent->u16_PropEvent );

      pst_PropEvent->u8_DataLen = pu8_Buffer[CFR_IE_HEADER_SIZE + sizeof(u16)];

      if( pst_PropEvent->u8_DataLen )
      {
      	memcpy( &pst_PropEvent->u8_Data, pu8_Buffer + CFR_IE_HEADER_SIZE + sizeof(u16) + sizeof(u8),
                 MIN( pst_PropEvent->u8_DataLen, CMBS_PROP_EVENT_LENGTH ) );
      }
   	return CMBS_RC_OK;
   }

   return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_int_ie_DateTimeAdd( void * pv_RefIEList, ST_IE_DATETIME * pst_DateTime )
{
   u8  u8_Buffer[16];

   memset( u8_Buffer, 0, sizeof(u8_Buffer) );

   u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_DATETIME;
   u8_Buffer[CFR_IE_SIZE_POS] = 9;

   u8_Buffer[CFR_IE_SIZE_POS + 1] = pst_DateTime->e_Coding;
   u8_Buffer[CFR_IE_SIZE_POS + 2] = pst_DateTime->e_Interpretation;
   u8_Buffer[CFR_IE_SIZE_POS + 3] = pst_DateTime->u8_Year;
   u8_Buffer[CFR_IE_SIZE_POS + 4] = pst_DateTime->u8_Month;
   u8_Buffer[CFR_IE_SIZE_POS + 5] = pst_DateTime->u8_Day;
   u8_Buffer[CFR_IE_SIZE_POS + 6] = pst_DateTime->u8_Hours;
   u8_Buffer[CFR_IE_SIZE_POS + 7] = pst_DateTime->u8_Mins;
   u8_Buffer[CFR_IE_SIZE_POS + 8] = pst_DateTime->u8_Secs;
   u8_Buffer[CFR_IE_SIZE_POS + 9] = pst_DateTime->u8_Zone;

   cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, 11 );

   return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_int_ie_DataSessionTypeAdd( void * pv_RefIEList,
                                                  ST_IE_DATA_SESSION_TYPE * pst_DataSessionType )
{
   u8  u8_Buffer[8];

   memset( u8_Buffer, 0, sizeof(u8_Buffer) );

   u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_DATA_SESSION_TYPE;
   u8_Buffer[CFR_IE_SIZE_POS] = 2;

   u8_Buffer[CFR_IE_SIZE_POS + 1] = pst_DataSessionType->e_ChannelType;
   u8_Buffer[CFR_IE_SIZE_POS + 2] = pst_DataSessionType->e_ServiceType;

   cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, 4 );

   return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_int_ie_DataAdd( void * pv_RefIEList, ST_IE_DATA * pst_Data )
{
   u8  u8_Buffer[768];
   u16 u16_Size;

   memset( u8_Buffer, 0, sizeof(u8_Buffer) );

   u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_DATA;

   u16_Size = sizeof(u16) + pst_Data->u16_DataLen;
   cfr_ie_ser_u16( u8_Buffer + 1, u16_Size );

   cfr_ie_ser_u16( u8_Buffer + 3, pst_Data->u16_DataLen );
   cfr_ie_ser_pu8( u8_Buffer + 5, pst_Data->pu8_Data, pst_Data->u16_DataLen );

   u16_Size = 5 + pst_Data->u16_DataLen;

   cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u16_Size );

   return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_int_ie_CallStateAdd( void * pv_RefIEList, ST_IE_CALL_STATE * pst_CallState )
{
   u8  u8_Buffer[32];
   u16 u16_Size;

   memset( u8_Buffer, 0, sizeof(u8_Buffer) );

   u16_Size = sizeof(*pst_CallState);

   u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_CALLSTATE;
   cfr_ie_ser_u16( &u8_Buffer[CFR_IE_SIZE_POS], u16_Size );

   u8_Buffer[CFR_IE_HEADER_SIZE] = pst_CallState->u8_ActCallID;
   u8_Buffer[CFR_IE_HEADER_SIZE + 1] = pst_CallState->e_CallType;
   u8_Buffer[CFR_IE_HEADER_SIZE + 2] = pst_CallState->e_CallStatus;
   cfr_ie_ser_u16( &u8_Buffer[CFR_IE_HEADER_SIZE + 3], pst_CallState->u16_HandsetsMask );
   u8_Buffer[CFR_IE_HEADER_SIZE + 5] = pst_CallState->u8_LinesMask;

   u16_Size = u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE;

   cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u16_Size );

   return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_int_ie_LAFieldsAdd( void * pv_RefIEList, ST_IE_LA_FIELDS * pst_LAFields,
                                           E_CMBS_IE_TYPE e_IEType )
{
   u8             u8_Buffer[32];
   u16            u16_Size;
   u8             u8_Count;

   u8_Buffer[CFR_IE_TYPE_POS] = (u8)e_IEType;
   u8_Buffer[CFR_IE_SIZE_POS] = sizeof(u16) + sizeof(u16) * CMBS_LA_MAX_FLD_COUNT;

   cfr_ie_ser_u16( u8_Buffer + CFR_IE_HEADER_SIZE, pst_LAFields->u16_Length );
   u16_Size = CFR_IE_HEADER_SIZE + sizeof(u16);

   for( u8_Count = 0; u8_Count < CMBS_LA_MAX_FLD_COUNT; u8_Count++ )
   {
      cfr_ie_ser_u16( u8_Buffer + u16_Size, pst_LAFields->pu16_FieldId[u8_Count] );
      u16_Size += sizeof(u16);
   }

   cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u16_Size );

   return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_int_ie_LASearchCriteriaAdd( void * pv_RefIEList,
                                                   ST_IE_LA_SEARCH_CRITERIA * pst_LASearchCriteria )
{
   u8             u8_Buffer[64];
   u16            u16_Size;

   u8_Buffer[CFR_IE_TYPE_POS] = CMBS_IE_LA_SEARCH_CRITERIA;
   u8_Buffer[CFR_IE_SIZE_POS] = 3 + pst_LASearchCriteria->u8_PatternLength;

   u8_Buffer[CFR_IE_SIZE_POS + 1] = pst_LASearchCriteria->e_MatchingType;
   u8_Buffer[CFR_IE_SIZE_POS + 2] = pst_LASearchCriteria->u8_CaseSensitive;
   u8_Buffer[CFR_IE_SIZE_POS + 3] = pst_LASearchCriteria->u8_PatternLength;

   cfr_ie_ser_pu8( u8_Buffer + CFR_IE_SIZE_POS + 4,
                   pst_LASearchCriteria->pu8_Pattern,
                   pst_LASearchCriteria->u8_PatternLength );

   u16_Size = u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE;
   cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u16_Size );

   return CMBS_RC_OK;
}


/*****************************************************************************
 * List Access fields
 *****************************************************************************/

/*****************************************************************************/

#if !defined( CMBS_API_TARGET )
//		========== 	_dbgCMBSEVPrint ===========
/*!
		\brief				 return CMBS event string; For host side
		\param[in,out]		 id		 CMBSinternal command

		\return		       <char * >

*/
char *			  _dbgCMBSEVPrint( E_CMBS_EVENT_ID id )
{
   switch( id )
   {
      caseretstr(CMBS_EV_UNDEF);
      caseretstr(CMBS_EV_DSR_HS_PAGE);             /*!< Performs paging handsets */
      caseretstr(CMBS_EV_DSR_HS_PAGE_RES);         /*!< Response to CMBS_EV_DSR_HS_PAGE */
      caseretstr(CMBS_EV_DSR_HS_DELETE);           /*!< Delete one or more handsets from the base's database */
      caseretstr(CMBS_EV_DSR_HS_DELETE_RES);       /*!< Response to CMBS_EV_DSR_HS_DELETE */
      caseretstr(CMBS_EV_DSR_HS_REGISTERED);       /*!< Unsolicited event generated on successful register/unregister operation of a handset */
/* ALTDV */
      caseretstr(CMBS_EV_DSR_HS_SUBSCRIBED_LIST_GET);/*!< Get list of registered handsets */
      caseretstr(CMBS_EV_DSR_HS_SUBSCRIBED_LIST_GET_RES);/*!< Get list of registered handsets */
      caseretstr(CMBS_EV_DSR_HS_SUBSCRIBED_LIST_SET);/*!< Set new name for registered handset */
      caseretstr(CMBS_EV_DSR_HS_SUBSCRIBED_LIST_SET_RES);/*!< Set new name for registered handset */
      caseretstr(CMBS_EV_DSR_LINE_SETTINGS_LIST_GET);/*!< Get line settings */
      caseretstr(CMBS_EV_DSR_LINE_SETTINGS_LIST_GET_RES);/*!< Get line settings */
      caseretstr(CMBS_EV_DSR_LINE_SETTINGS_LIST_SET);/*!< Set line settings */
      caseretstr(CMBS_EV_DSR_LINE_SETTINGS_LIST_SET_RES);/*!< Get line settings */

      caseretstr(CMBS_EV_DSR_CORD_OPENREG);        /*!< Starts registration mode on the base station */
      caseretstr(CMBS_EV_DSR_CORD_OPENREG_RES);    /*!< Response to CMBS_EV_DSR_CORD_OPENREG */
      caseretstr(CMBS_EV_DSR_CORD_CLOSEREG);       /*!< Stops registration mode on the base station */
      caseretstr(CMBS_EV_DSR_CORD_CLOSEREG_RES);   /*!< Response to CMBS_EV_DSR_CORD_CLOSEREG */
      caseretstr(CMBS_EV_DSR_PARAM_GET);           /*!< Get a parameter value */
      caseretstr(CMBS_EV_DSR_PARAM_GET_RES);       /*!< Response to CMBS_EV_DSR_PARAM_GET */
      caseretstr(CMBS_EV_DSR_PARAM_SET);           /*!< Sets / updates a parameter value */
      caseretstr(CMBS_EV_DSR_PARAM_SET_RES);       /*!< Response to CMBS_EV_DSR_PARAM_SET */
      caseretstr(CMBS_EV_DSR_FW_UPD_START);        /*!< Starts firmware update on the base station */
      caseretstr(CMBS_EV_DSR_FW_UPD_START_RES);    /*!< Response to CMBS_EV_DSR_FW_UPD_START */
      caseretstr(CMBS_EV_DSR_FW_UPD_PACKETNEXT);   /*!< Sends a chunk of firmware to the base station */
      caseretstr(CMBS_EV_DSR_FW_UPD_PACKETNEXT_RES);/*!< Response to CMBS_EV_DSR_FW_UPD_PACKETNEXT */
      caseretstr(CMBS_EV_DSR_FW_UPD_END);          /*!< Ending firmware update process with last chunk of data */
      caseretstr(CMBS_EV_DSR_FW_UPD_END_RES);      /*!< Response to CMBS_EV_DSR_FW_UPD_END */
      caseretstr(CMBS_EV_DSR_FW_VERSION_GET);      /*!< Gets the base's current firmware version of a particular module */
      caseretstr(CMBS_EV_DSR_FW_VERSION_GET_RES);  /*!< Response to CMBS_EV_DSR_FW_VERSION_GET */
      caseretstr(CMBS_EV_DSR_SYS_START);           /*!< Starts the base station's CMBS after parameters were set */
      caseretstr(CMBS_EV_DSR_SYS_START_RES);       /*!< Response to CMBS_EV_DSR_SYS_START  */
      caseretstr(CMBS_EV_DSR_SYS_SEND_RAWMSG);     /*!< Event containing a raw message to the target */
      caseretstr(CMBS_EV_DSR_SYS_SEND_RAWMSG_RES); /*!< Response to CMBS_EV_DSR_SYS_SEND_RAWMSG */
      caseretstr(CMBS_EV_DSR_SYS_STATUS);          /*!< Announce current target status); e.g. up); down); removed */
      caseretstr(CMBS_EV_DSR_SYS_LOG);             /*!< Event containing target system logs */
      caseretstr(CMBS_EV_DSR_SYS_RESET);           /*!< Performs a base station reboot */
/* ALTDV */
      caseretstr(CMBS_EV_DSR_SYS_POWER_OFF);       /*!< Performs a base station power off */
      caseretstr(CMBS_EV_DSR_RF_SUSPEND);          /*!< RF Suspend on CMBS target */
      caseretstr(CMBS_EV_DSR_RF_RESUME);           /*!< RF Resume on CMBS target */
      caseretstr(CMBS_EV_DSR_TURN_ON_NEMO);        /*!< Turn On NEMo mode for the CMBS base */
      caseretstr(CMBS_EV_DSR_TURN_OFF_NEMO);       /*!< Turn Off NEMo mode for the CMBS base */
      caseretstr(CMBS_EV_DEE_CALL_ESTABLISH);      /*!< Event generated on start of a new call( incoming or outgoing ) */
      caseretstr(CMBS_EV_DEE_CALL_PROGRESS);       /*!< Events for various call progress states */
      caseretstr(CMBS_EV_DEE_CALL_ANSWER);         /*!< Generated when a call is answered */
      caseretstr(CMBS_EV_DEE_CALL_RELEASE);        /*!< Generated when a call is released */
      caseretstr(CMBS_EV_DEE_CALL_RELEASECOMPLETE);/*!< Generated when call instance deleted */
      caseretstr(CMBS_EV_DEE_CALL_INBANDINFO);     /*!< Events created for inband keys */
      caseretstr(CMBS_EV_DCM_CALL_STATE);          /*!< Call state */
      caseretstr(CMBS_EV_DEE_CALL_MEDIA_OFFER);    /*!< Offer media */
      caseretstr(CMBS_EV_DEE_CALL_MEDIA_OFFER_RES);/*!< Response to CMBS_EV_DEE_CALL_MEDIA_OFFER */
      caseretstr(CMBS_EV_DEE_CALL_MEDIA_UPDATE);   /*!< Received when cordless module updated the media */
      caseretstr(CMBS_EV_DEE_CALL_HOLD);           /*!< Generated on call HOLD */
      caseretstr(CMBS_EV_DEE_CALL_RESUME);         /*!< Generated on call RESUME */
      caseretstr(CMBS_EV_DEM_CHANNEL_START);       /*!< Start sending (voice) data on a particular channel */
      caseretstr(CMBS_EV_DEM_CHANNEL_START_RES);   /*!< Response to CMBS_EV_DEM_CHANNEL_START */
      caseretstr(CMBS_EV_DEM_CHANNEL_INTERNAL_CONNECT);  /*!< Modify the IN/Out Connection for Media Channels */
      caseretstr(CMBS_EV_DEM_CHANNEL_INTERNAL_CONNECT_RES);/*!< Response to CMBS_EV_DEM_CHANNEL_INTERNAL_CONNECT */
      caseretstr(CMBS_EV_DEM_CHANNEL_STOP);        /*!< Stop sending data on a particular channel */
      caseretstr(CMBS_EV_DEM_CHANNEL_STOP_RES);    /*!< Response to CMBS_EV_DEM_CHANNEL_STOP */
      caseretstr(CMBS_EV_DEM_TONE_START);          /*!< Start the tone generation on a particular media channel */
      caseretstr(CMBS_EV_DEM_TONE_START_RES);      /*!< Response to CMBS_EV_DEM_TONE_START */
      caseretstr(CMBS_EV_DEM_TONE_STOP);           /*!< Stop tone generation on a particular media channel */
      caseretstr(CMBS_EV_DEM_TONE_STOP_RES);       /*!< Response to CMBS_EV_DEM_TONE_STOP */
      caseretstr(CMBS_EV_DSR_SYS_LOG_START);       /*!< Start system logging */
      caseretstr(CMBS_EV_DSR_SYS_LOG_STOP);        /*!< Stop system logging */
      caseretstr(CMBS_EV_DSR_SYS_LOG_REQ);         /*!< Request to get content of the log buffer */
      caseretstr(CMBS_EV_DSR_PARAM_UPDATED);
      caseretstr(CMBS_EV_DSR_PARAM_AREA_UPDATED);
      caseretstr(CMBS_EV_DSR_PARAM_AREA_GET);
      caseretstr(CMBS_EV_DSR_PARAM_AREA_GET_RES);
      caseretstr(CMBS_EV_DSR_PARAM_AREA_SET);
      caseretstr(CMBS_EV_DSR_PARAM_AREA_SET_RES);

      caseretstr(CMBS_EV_DSR_GEN_SEND_VMWI);             /*!< Send Voice Message Waiting Indication to one or more handsets */
      caseretstr(CMBS_EV_DSR_GEN_SEND_VMWI_RES);         /*!< Response to CMBS_EV_DSR_GEN_SEND_VMWI */
      caseretstr(CMBS_EV_DSR_GEN_SEND_MISSED_CALLS);     /*!< Send Missed Calls Indication to one or more handsets */
      caseretstr(CMBS_EV_DSR_GEN_SEND_MISSED_CALLS_RES); /*!< Response to CMBS_EV_DSR_GEN_SEND_MISSED_CALLS */
      caseretstr(CMBS_EV_DSR_GEN_SEND_LIST_CHANGED);     /*!< Send List Changed event to one or more handsets */
      caseretstr(CMBS_EV_DSR_GEN_SEND_LIST_CHANGED_RES); /*!< Response to CMBS_EV_DSR_GEN_SEND_LIST_CHANGED */
      caseretstr(CMBS_EV_DSR_GEN_SEND_WEB_CONTENT);      /*!< Send Web Content event to one or more handsets */
      caseretstr(CMBS_EV_DSR_GEN_SEND_WEB_CONTENT_RES);  /*!< Response to CMBS_EV_DSR_GEN_SEND_WEB_CONTENT */
      caseretstr(CMBS_EV_DSR_GEN_SEND_PROP_EVENT);       /*!< Send Escape to Proprietary event to one or more handsets */
      caseretstr(CMBS_EV_DSR_GEN_SEND_PROP_EVENT_RES);   /*!< Response to CMBS_EV_DSR_GEN_SEND_PROP_EVENT */
      caseretstr(CMBS_EV_DSR_TIME_UPDATE);               /*!< Send Time-Date update event to one or more handsets */
      caseretstr(CMBS_EV_DSR_TIME_UPDATE_RES);           /*!< Response to CMBS_EV_DSR_TIME_UPDATE */
      caseretstr(CMBS_EV_DSR_TIME_INDICATION);           /*!< Event received when a handset has updated its Time-Date setting */
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

   if(CMBS_EV_DSR_USER_DEFINED_START < id && id < CMBS_EV_DSR_USER_DEFINED_END)
   {
      return (char *)"CMBS_EV_DSR_USER_DEFINED";
   }

   return (char*)"Unknown Event";
}
#endif   // !defined( CMBS_API_TARGET )

//		========== cmbs_int_EventTrace  ===========
/*!
		\brief				 trace CMBS event

		\param[in,out]		 pu8_Buffer		 pointer to message buffer

		\param[in,out]		 u16_Size		 size of message buffer

		\return				< none >

*/

void				   cmbs_int_EventTrace ( u8 * pu8_Buffer, u16 u16_Size )
{
   PST_CMBS_SER_MSG p_UartMsg = (PST_CMBS_SER_MSG)pu8_Buffer;
   //	int				 i;

   if( u16_Size || p_UartMsg ){
   }  // eliminate compiler warnings

   CFR_DBG_OUT( "CMBS-API: Event received => len:%d, nr:%d, ev:%s, plen:%d\n",
      p_UartMsg->st_MsgHdr.u16_TotalLength,
      p_UartMsg->st_MsgHdr.u16_PacketNr,
      _dbgCMBSEVPrint( (E_CMBS_EVENT_ID)p_UartMsg->st_MsgHdr.u16_EventID),
      p_UartMsg->st_MsgHdr.u16_ParamLength );

   /*
   CFR_DBG_OUT( "Parameter\n" );
   for( i=0; i<p_UartMsg->st_MsgHdr.u16_ParamLength; i++ )
   {
   	   CFR_DBG_OUT( "%02x", p_UartMsg->u8_Param[i] );

   if ( i%16 == 0 )
   {
   	CFR_DBG_OUT( "\n" );
   }
   }

   CFR_DBG_OUT( "\n" );
   */
}

//		========== 	_dbgCMBSCMDPrint ===========
/*!
		\brief				 return command string
		\param[in,out]		 id		 CMBSinternal command

		\return		       <char * >

*/

char *			  _dbgCMBSCMDPrint( CMBS_CMD id )
{
   switch( id )
   {
      caseretstr(CMBS_CMD_HELLO);
      caseretstr(CMBS_CMD_HELLO_RPLY);
      caseretstr(CMBS_CMD_FLOW_NOK);
      caseretstr(CMBS_CMD_FLOW_RESTART);
      caseretstr(CMBS_CMD_RESET);
   }

   return (char*) "Unknown CMBS command";
}


//		========== 	_dbgCMBSParamPrint ===========
/*!
		\brief            return cmbs parameter string
		\param[in]        parameter id

		\return		       <char * >

*/

char *			  _dbgCMBSParamPrint( E_CMBS_PARAM e_ParamId )
{
   switch( e_ParamId )
   {
      caseretstr(CMBS_PARAM_RFPI);
      caseretstr(CMBS_PARAM_RVBG);
      caseretstr(CMBS_PARAM_RVREF);
      caseretstr(CMBS_PARAM_RXTUN);
      caseretstr(CMBS_PARAM_MASTER_PIN);
      caseretstr(CMBS_PARAM_AUTH_PIN);
      caseretstr(CMBS_PARAM_COUNTRY);
      caseretstr(CMBS_PARAM_SIGNALTONE_DEFAULT);
      caseretstr(CMBS_PARAM_TEST_MODE);
      caseretstr(CMBS_PARAM_AUTO_REGISTER);
      caseretstr(CMBS_PARAM_NTP);         
      caseretstr(CMBS_PARAM_ECO_MODE);

      default:
         return (char*) "Unknown CMBS parameter!\n";
   }
}


#if !defined( CMBS_API_TARGET )
//		========== cmbs_int_Dbg_IE_Print ===========
/*!
      \brief            returns Information Element enumerator definition string

      \param[in]        id    CMBSinternal command

      \return           <char * >
*/
char *            cmbs_int_Dbg_IE_Print( E_CMBS_IE_TYPE e_IE )
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
#endif

//		========== cmbs_int_CmdTrace  ===========
/*!
		\brief				 trace CMBS internal command

		\param[in,out]		 pu8_Buffer		 pointer to message buffer

		\param[in,out]		 u16_Size		 size of message buffer

		\return				< none >

*/

void              cmbs_int_CmdTrace ( u8 * pu8_Buffer, u16 u16_Size )
{
   PST_CMBS_SER_MSG p_UartMsg = (PST_CMBS_SER_MSG)pu8_Buffer;
   //	int				 i;

   if( u16_Size ){
   }  // eliminate compiler warnings

   CFR_DBG_OUT( "CMBS-API: Command received => len:%d, nr:%d, cmd:%s, plen:%d\n",
      p_UartMsg->st_MsgHdr.u16_TotalLength,
      p_UartMsg->st_MsgHdr.u16_PacketNr,
      _dbgCMBSCMDPrint( (CMBS_CMD)(p_UartMsg->st_MsgHdr.u16_EventID & 0xFF) ),
      p_UartMsg->st_MsgHdr.u16_ParamLength );
   /*
   CFR_DBG_OUT( "Parameter\n" );
   for( i=0; i<p_UartMsg->st_MsgHdr.u16_ParamLength; i++ )
   {
   CFR_DBG_OUT( "%02x", p_UartMsg->u8_Param[i] );

   if ( i%16 == 0 )
   {
   	   CFR_DBG_OUT( "\n" );
   }
      }

   CFR_DBG_OUT( "\n" );
   */
}

/*
static void       _cmbs_int_MsgCommandPrint( u16 u16_Len, u16 u16_MsgNr, u16 u16_Event )
{
   if( u16_Event > 255 )
   {
      CFR_DBG_OUT ( "CMBS-API: message len %d, msg %d EV: %s\n", u16_Len, u16_MsgNr, _dbgCMBSCMDPrint((u16_Event & 0xFF)) );
   }
   else
   {
      CFR_DBG_OUT ( "CMBS-API: message len %d, msg %d EV: %s\n", u16_Len, u16_MsgNr, _dbgCMBSEVPrint( u16_Event ));
   }
}
*/

//EOF
