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

#include "cmbs_int.h"
#include "cfr_uart.h"
#include "cfr_debug.h"
#include "bus.h"

/* Global variables */

/* Local variables */

//static void       _cmbs_int_MsgCommandPrint( u16 u16_Len, u16 u16_MsgNr, u16 u16_Event );
char *			  _dbgCMBSEVPrint( E_CMBS_EVENT_ID id );
char *			  _dbgCMBSCMDPrint( CMBS_CMD id );

/*****************************************************************************
 * API Internal functions
 *****************************************************************************/

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

/*!
   \brief   CMBS event is received, prepare for callback function
  \param[in]      pu8_Mssg    pointer to message without sync dword
  \param[in]      u16_Size size of message without sync dword

  \return         <none>

*/
void              cmbs_int_EventReceive ( u8 * pu8_Mssg, u16 u16_Size )
{
   PST_CMBS_SER_MSG p_Mssg = (PST_CMBS_SER_MSG)pu8_Mssg;

#if !defined( CMBS_API_TARGET )
   CFR_DBG_OUT( "CMBS-API: cmbs_int_EventReceive evcmd:0x%x", p_Mssg->st_MsgHdr.u16_EventID );
   if( p_Mssg->st_MsgHdr.u16_ParamLength )
   {
      int i;
      CFR_DBG_OUT( ", data: " );
      for( i=0; i < p_Mssg->st_MsgHdr.u16_ParamLength; i++ )
         CFR_DBG_OUT( "%02x ", p_Mssg->u8_Param[i] );
   }
   CFR_DBG_OUT( "\n" );
#endif

   if ( (p_Mssg->st_MsgHdr.u16_EventID & CMBS_CMD_MASK) == CMBS_CMD_MASK)
   {
      cmbs_int_CmdTrace ( pu8_Mssg, u16_Size );

      cmbs_int_cmd_Dispatcher( (u8)p_Mssg->st_MsgHdr.u16_EventID, (u8*)p_Mssg->u8_Param, u16_Size - sizeof(ST_CMBS_SER_MSGHDR) );
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

void              cms_int_HdrEndianCvt( ST_CMBS_SER_MSGHDR *pst_Hdr )
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

   if( g_CMBSInstance.e_Endian != E_CMBS_ENDIAN_LITTLE )
   {
      cms_int_HdrEndianCvt( &msgHdr );
   }

   memcpy((u8*)pkt_tmp, &u8_pktLen, 1);			//cp pkt len
   memcpy((u8*)pkt_tmp+1, &u32_Sync, 4);			//cp sync word
   memcpy((u8*)pkt_tmp+1+4, &msgHdr, sizeof(msgHdr));		//cp msgHdr
   memcpy((u8*)pkt_tmp+1+4+sizeof(msgHdr), pBuf, u16_Length);	//cp para data   
   
   Bus_Write_byte_blocking(pkt_tmp, u8_pktLen);

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

      return   CMBS_RC_OK;

#if 0

#if !defined( CMBS_API_TARGET )
   CFR_DBG_ERROR( "cmbs_int_EventSend: !!!! transmit buffer full\n" );
#endif // CMB_API_TARGET

   return CMBS_RC_ERROR_OUT_OF_MEM;
   
#endif
}

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
   if ( cmbs_int_cmd_FlowStateGet() == E_CMBS_FLOW_STATE_GO )
   {
      return _cmbs_int_Send ( (u16) e_EventID, pBuf, u16_Length );
   }

   return CMBS_RC_ERROR_MEDIA_BUSY;
}

//		========== cmbs_int_cmd_Send ===========
/*!
		\brief				 Send complete internal command message: header + parameter data to communication device

		\param[in,out]		 u8_Cmd         internal command ID

		\param[in,out]		 pBuf		       pointer parameter buffer

		\param[in,out]		 u16_Length		 size of parameter buffer

		\return				< E_CMBS_RC >

*/

E_CMBS_RC         cmbs_int_cmd_Send( u8 u8_Cmd, u8 * pBuf, u16 u16_Length )
{
   return _cmbs_int_Send ( (u16)(0xFF00 | u8_Cmd), pBuf, u16_Length );
}


#if ( !defined(__linux__) && !defined(WIN32) )
#include "bsd02eep.h"
extern u8 g_u8_HsSubData[SD02_SUB_LEN];

E_CMBS_RC         cmbs_int_OnHsRegistration( void )
{
   ST_IE_HANDSETINFO st_HandsetInfo;
   PST_CFR_IE_LIST   p_List;

   memset( &st_HandsetInfo, 0, sizeof(ST_IE_HANDSETINFO) );

   st_HandsetInfo.u8_Hs    = g_u8_HsSubData[38] & 0x0F;   // take only the least significant nibble
   st_HandsetInfo.u8_State = g_u8_HsSubData[37];
   memcpy( st_HandsetInfo.u8_IPEI, g_u8_HsSubData, sizeof(st_HandsetInfo.u8_IPEI) );

   p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();

   cmbs_int_ie_HandsetInfoAdd( (void*)p_List, &st_HandsetInfo );

   return cmbs_int_EventSend( CMBS_EV_DSR_HS_REGISTERED, p_List->pu8_Buffer, p_List->u16_CurSize );
}
#endif


void              cmbs_int_ie_HandsetInfoAdd( void * pv_RefIEList, ST_IE_HANDSETINFO * pst_HandsetInfo )
{
   u8 u8_Buffer[16];

   u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_HANDSETINFO;
   u8_Buffer[CFR_IE_SIZE_POS] = 12;

   u8_Buffer[2] = pst_HandsetInfo->u8_Hs;
   u8_Buffer[3] = pst_HandsetInfo->u8_State;
   u8_Buffer[4] = (u8)pst_HandsetInfo->e_Type;

   cfr_ie_ser_u32( u8_Buffer + 5, pst_HandsetInfo->u32_ModelNr );

   memcpy( u8_Buffer + 9, pst_HandsetInfo->u8_IPEI, sizeof(pst_HandsetInfo->u8_IPEI) );

   cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE );
}

// byte order [IE][size][16bit pos][16bit len][data...]
void              cmbs_int_ie_ParameterAdd( void * pv_RefIEList, ST_IE_PARAMETER * pst_Param )
{
//   int i;
                                 // max var. param size 128
   u8 u8_Buffer[128 + CFR_IE_HEADER_SIZE + 3];

//   CFR_DBG_OUT("IE Param %d pos %d len %d ", pst_Param->e_Param, pst_Param->u16_Position, pst_Param->u16_DataLen);

   memset( u8_Buffer, 0, sizeof(u8_Buffer) );

   u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_PARAMETER;
   u8_Buffer[CFR_IE_SIZE_POS] = sizeof(u8) + sizeof(u16) + sizeof(u16);

   u8_Buffer[CFR_IE_HEADER_SIZE] = (u8)pst_Param->e_Param;

   cfr_ie_ser_u16( u8_Buffer + CFR_IE_HEADER_SIZE + sizeof(u8), pst_Param->u16_Position );
   cfr_ie_ser_u16( u8_Buffer + CFR_IE_HEADER_SIZE + sizeof(u8)+ sizeof(u16), pst_Param->u16_DataLen );

   if( pst_Param->pu8_Data )
   {
      cfr_ie_ser_pu8( u8_Buffer + CFR_IE_HEADER_SIZE + u8_Buffer[CFR_IE_SIZE_POS],
                      pst_Param->pu8_Data, pst_Param->u16_DataLen );

      u8_Buffer[CFR_IE_SIZE_POS] += pst_Param->u16_DataLen;
   }
/*
   for (i=0;i< u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE;i++)
      CFR_DBG_OUT( "%02x ", u8_Buffer[i]);
*/
   cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE );

}


void              cmbs_int_ie_ResponseAdd( void * pv_RefIEList, ST_IE_RESPONSE * pst_Response )
{
   u8 u8_Buffer[8];

   u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_RESPONSE;
   u8_Buffer[CFR_IE_SIZE_POS] = sizeof(u8);

   u8_Buffer[CFR_IE_HEADER_SIZE] = pst_Response->e_Response;

   cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE );
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

   u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_INTEGER_VALUE;
   u8_Buffer[CFR_IE_SIZE_POS] = sizeof(u32);

   cfr_ie_ser_u32( u8_Buffer + CFR_IE_HEADER_SIZE, u32_Value );

   cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE );
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


void              cmbs_int_ie_FwVersionAdd( void * pv_RefIEList, ST_IE_FW_VERSION * pst_FwVersion )
{
   u8  u8_Buffer[8];

   u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_FW_VERSION;
   u8_Buffer[CFR_IE_SIZE_POS] = sizeof(u8) + sizeof(u16);

   u8_Buffer[CFR_IE_HEADER_SIZE] = pst_FwVersion->e_SwModule;

   cfr_ie_ser_u16( u8_Buffer + CFR_IE_HEADER_SIZE + sizeof(u8), pst_FwVersion->u16_FwVersion );

   cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE );
}

E_CMBS_RC cmbs_int_ie_SysStatusSet( void * pv_RefIEList, ST_IE_SYS_STATUS * pst_SysStatus )
{
   u8 u8_Buffer[4];

   memset( u8_Buffer, 0, sizeof(u8_Buffer) );

   u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_STATUS;
   u8_Buffer[CFR_IE_SIZE_POS] = sizeof(u8);

   u8_Buffer[CFR_IE_HEADER_SIZE] = (u8)pst_SysStatus->e_ModuleStatus;

   cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE );
   
   return CMBS_RC_OK;//th add
}


//		========== 	_dbgCMBSEVPrint ===========
/*!
		\brief				 return CMBS event string
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
      caseretstr(CMBS_EV_DEE_CALL_ESTABLISH);      /*!< Event generated on start of a new call( incoming or outgoing ) */
      caseretstr(CMBS_EV_DEE_CALL_PROGRESS);       /*!< Events for various call progress states */
      caseretstr(CMBS_EV_DEE_CALL_ANSWER);         /*!< Generated when a call is answered */
      caseretstr(CMBS_EV_DEE_CALL_RELEASE);        /*!< Generated when a call is released */
      caseretstr(CMBS_EV_DEE_CALL_RELEASECOMPLETE);/*!< Generated when call instance deleted */
      caseretstr(CMBS_EV_DEE_CALL_INBANDINFO);     /*!< Events created for inband keys */
      caseretstr(CMBS_EV_DEE_CALL_MEDIA_OFFER);    /*!< Offer media */
      caseretstr(CMBS_EV_DEE_CALL_MEDIA_OFFER_RES);/*!< Response to CMBS_EV_DEE_CALL_MEDIA_OFFER */
      caseretstr(CMBS_EV_DEE_CALL_MEDIA_UPDATE);   /*!< Received when cordless module updated the media */
      caseretstr(CMBS_EV_DEE_CALL_HOLD);           /*!< Generated on call HOLD */
      caseretstr(CMBS_EV_DEE_CALL_RESUME);         /*!< Generated on call RESUME */
      caseretstr(CMBS_EV_DEM_CHANNEL_START);       /*!< Start sending (voice) data on a particular channel */
      caseretstr(CMBS_EV_DEM_CHANNEL_START_RES);   /*!< Response to CMBS_EV_DEM_CHANNEL_START */
      caseretstr(CMBS_EV_DEM_CHANNEL_STOP);        /*!< Stop sending data on a particular channel */
      caseretstr(CMBS_EV_DEM_CHANNEL_STOP_RES);    /*!< Response to CMBS_EV_DEM_CHANNEL_STOP */
      caseretstr(CMBS_EV_DEM_TONE_START);          /*!< Start the tone generation on a particular media channel */
      caseretstr(CMBS_EV_DEM_TONE_START_RES);      /*!< Response to CMBS_EV_DEM_TONE_START */
      caseretstr(CMBS_EV_DEM_TONE_STOP);           /*!< Stop tone generation on a particular media channel */
      caseretstr(CMBS_EV_DEM_TONE_STOP_RES);       /*!< Response to CMBS_EV_DEM_TONE_STOP */
      caseretstr(CMBS_EV_DSR_SYS_LOG_REQ);         /*!< Request to get content of the log buffer */
      caseretstr(CMBS_EV_MAX);
   }

   return (char*)"Unknown Event";
}

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

   if( u16_Size ){
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
      caseretstr(CMBS_PARAM_FLEX);

      default:
         return (char*) "Unknown CMBS parameter!\n";
   }
}


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
