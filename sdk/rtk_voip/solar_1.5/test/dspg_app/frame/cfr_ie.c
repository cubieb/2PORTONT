/*!
*  \file       cfr_ie.c
*  \brief         
*  \Author     kelbch 
*
*  @(#)  %filespec: cfr_ie.c-1 %                   
*
*******************************************************************************
*  \par  History
*  \n==== History ============================================================\n
*  date        name     version   action                                          \n
*  ----------------------------------------------------------------------------\n
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

#include "cmbs_int.h"      /* internal API structure and defines */
#include "cfr_uart.h"      /* packet handler */
#include "cfr_debug.h"     /* debug handling */


void *      cfr_ie_ListNew( u8* pu8_Buffer, u16 u16_Size )
{
   PST_CFR_IE_LIST p_List = &g_CMBSInstance.st_ApplSlot.st_TransmitterIEList;
   
   if ( !u16_Size || !pu8_Buffer)
   {
      memset(p_List, 0, sizeof(ST_CFR_IE_LIST));
      p_List->pu8_Buffer = g_CMBSInstance.st_ApplSlot.u8_IEBuffers;
      p_List->u16_MaxSize= sizeof(g_CMBSInstance.st_ApplSlot.u8_IEBuffers);
   
      return p_List;
   }
   else
   {
      CFR_DBG_OUT( "cfr_ie_ListNew: Dynamic is not implemented, yet\n" );
   }

   return NULL;
}

int         cfr_ie_ItemAdd( PST_CFR_IE_LIST p_List, u8 * pu8_Buffer, u16 u16_Size)
{
// CFR_DBG_OUT( "ItemAdd: total %d cur %d IE %d\n", p_List->u16_MaxSize, p_List->u16_CurSize, p_List->u16_CurIE );
  
   if ( p_List->u16_MaxSize > ( u16_Size + p_List->u16_CurSize ))
   {
      memcpy( p_List->pu8_Buffer + p_List->u16_CurSize, pu8_Buffer, u16_Size );
      p_List->u16_CurSize += u16_Size;
      
//    CFR_DBG_OUT( "ItemAdd: total %d cur %d IE %d\n", p_List->u16_MaxSize, p_List->u16_CurSize, p_List->u16_CurIE);
      return 0;
   }
   
   return -1;
}

u8 *        cfr_ie_ItemGet( PST_CFR_IE_LIST p_List )
{
// CFR_DBG_OUT( "ItemGet: total %d cur %d IE %d\n", p_List->u16_MaxSize, p_List->u16_CurSize, p_List->u16_CurIE);
   return p_List->pu8_Buffer + p_List->u16_CurIE;
}

u8 *        cfr_ie_ItemFirstGet ( PST_CFR_IE_LIST p_List )
{
   p_List->u16_CurIE = 0;
   return cfr_ie_ItemGet( p_List );
}

u8 *        cfr_ie_ItemNextGet( PST_CFR_IE_LIST p_List )
{
   u16 u16_NextIE = p_List->pu8_Buffer[ p_List->u16_CurIE + CFR_IE_SIZE_POS] + p_List->u16_CurIE + CFR_IE_HEADER_SIZE;

   if ( p_List->u16_CurSize > u16_NextIE )
   {
      p_List->u16_CurIE = u16_NextIE;
//    CFR_DBG_OUT( "ItemNextGet: total %d cur %d IE %d\n", p_List->u16_MaxSize, p_List->u16_CurSize, p_List->u16_CurIE);
      
      return cfr_ie_ItemGet( p_List );
   }

//   CFR_DBG_OUT( "ItemNextGet: No next item\n");

   return NULL;
}

u16            cfr_ie_ser_u16( u8 * pu8_Buffer, u16 u16_Value )
{
#if defined( CMBS_API_TARGET )

   memcpy( pu8_Buffer, &u16_Value, sizeof(u16_Value));

#else                               // Host
   u8 * pu8_16 = (u8*)&u16_Value;

   //CFR_DBG_OUT( "Ser_u16 u16_Value:0x%04x = ", u16_Value );

   if( g_CMBSInstance.e_Endian == E_CMBS_ENDIAN_LITTLE )
   {
      //CFR_DBG_OUT( "(LE)0x%02x 0x%02x -> ", pu8_16[0], pu8_16[1] );
      
      memcpy( pu8_Buffer, &u16_Value, sizeof(u16_Value));
   }
   else
   {
      //CFR_DBG_OUT( "(BE)0x%02x 0x%02x -> ", pu8_16[0], pu8_16[1] );
      
      pu8_Buffer[0] = pu8_16[1];
      pu8_Buffer[1] = pu8_16[0];
   }

   //CFR_DBG_OUT( "0x%02x 0x%02x\n", pu8_Buffer[0], pu8_Buffer[1] );
#endif
   
   return sizeof(u16_Value);
}

u16            cfr_ie_ser_u32( u8 * pu8_Buffer, u32 u32_Value )
{
#if defined( CMBS_API_TARGET )

   memcpy( pu8_Buffer, &u32_Value, sizeof(u32));

#else                               // Host
   u8 * pu8_32 = (u8*)&u32_Value;

   //CFR_DBG_OUT( "Ser_u32 u32_Value:0x%08x = ", u32_Value );

   if( g_CMBSInstance.e_Endian == E_CMBS_ENDIAN_LITTLE )
   {
      //CFR_DBG_OUT( "(LE)0x%02x 0x%02x 0x%02x 0x%02x -> ", pu8_32[0], pu8_32[1], pu8_32[2], pu8_32[3]);
      
      memcpy( pu8_Buffer, &u32_Value, sizeof(u32));
   }
   else if( g_CMBSInstance.e_Endian == E_CMBS_ENDIAN_BIG )
   {
      //CFR_DBG_OUT( "(BE)0x%02x 0x%02x 0x%02x 0x%02x -> ", pu8_32[0], pu8_32[1], pu8_32[2], pu8_32[3]);
      
      pu8_Buffer[0] = pu8_32[3];
      pu8_Buffer[1] = pu8_32[2];
      pu8_Buffer[2] = pu8_32[1];
      pu8_Buffer[3] = pu8_32[0];
   }
   else if( g_CMBSInstance.e_Endian == E_CMBS_ENDIAN_MIXED )
   {
      //CFR_DBG_OUT( "(ME)0x%02x 0x%02x 0x%02x 0x%02x -> ", pu8_32[0], pu8_32[1], pu8_32[2], pu8_32[3]);
      
      pu8_Buffer[0] = pu8_32[1];
      pu8_Buffer[1] = pu8_32[0];
      pu8_Buffer[2] = pu8_32[3];
      pu8_Buffer[3] = pu8_32[2];
   }
   else
   {
      //CFR_DBG_ERROR( "cfr_ie_ser_u32 ERROR: Unknown endianess\n" );
      return 0;
   }

   //CFR_DBG_OUT( "0x%02x 0x%02x 0x%02x 0x%02x\n", pu8_Buffer[0], pu8_Buffer[1], pu8_Buffer[2], pu8_Buffer[3] );

#endif
   
   return sizeof(u32);
}

u16            cfr_ie_ser_pu8( u8 * pu8_Buffer, u8 * pu8_Data, u16 u16_DataSize )
{
   memcpy ( pu8_Buffer, pu8_Data, u16_DataSize );
   return u16_DataSize;
}

u16            cfr_ie_dser_u16( u8 *pu8_Buffer, u16 * pu16_Value )
{
#if defined( CMBS_API_TARGET )

   memcpy( pu16_Value, pu8_Buffer, sizeof(u16));

#else                               // Host
   u8 *        pu8_16 = (u8*)pu16_Value;

   //CFR_DBG_OUT( "Dser_u16 0x%02x 0x%02x -> ", pu8_Buffer[0], pu8_Buffer[1] );

   if( g_CMBSInstance.e_Endian == E_CMBS_ENDIAN_LITTLE )
   {
      //CFR_DBG_OUT( "(LE): " );
      memcpy( pu8_16, pu8_Buffer, sizeof(u16));
   }
   else
   {
      //CFR_DBG_OUT( "(BE): " );
      pu8_16[1] = pu8_Buffer[0];
      pu8_16[0] = pu8_Buffer[1];
   }

   //CFR_DBG_OUT( "0x%02x 0x%02x = 0x%04x\n", pu8_16[0], pu8_16[1], *pu16_Value );

#endif   

   return sizeof(u16);
}

u16            cfr_ie_dser_u32( u8 * pu8_Buffer, u32 * pu32_Value )
{
#if defined( CMBS_API_TARGET )

   memcpy( pu32_Value, pu8_Buffer, sizeof(u32));

#else                               // Host
   u8 *        pu8_32 = (u8*)pu32_Value;
   
   //CFR_DBG_OUT( "Dser_u32 0x%02x 0x%02x 0x%02x 0x%02x -> ", pu8_Buffer[0], pu8_Buffer[1], pu8_Buffer[2], pu8_Buffer[3] );
   
   if( g_CMBSInstance.e_Endian == E_CMBS_ENDIAN_LITTLE )
   {
      //CFR_DBG_OUT( "(LE): " );
      memcpy( pu8_32, pu8_Buffer, sizeof(u32));
   }
   else if( g_CMBSInstance.e_Endian == E_CMBS_ENDIAN_BIG )
   {
      //CFR_DBG_OUT( "(BE): " );
      pu8_32[3] = pu8_Buffer[0];
      pu8_32[2] = pu8_Buffer[1];
      pu8_32[1] = pu8_Buffer[2];
      pu8_32[0] = pu8_Buffer[3];
   }
   else if( g_CMBSInstance.e_Endian == E_CMBS_ENDIAN_MIXED )
   {
      //CFR_DBG_OUT( "(ME): " );
      pu8_32[1] = pu8_Buffer[0];
      pu8_32[0] = pu8_Buffer[1];
      pu8_32[3] = pu8_Buffer[2];
      pu8_32[2] = pu8_Buffer[3];
   }
   else
   {
      //CFR_DBG_ERROR( "cfr_ie_dser_u32 ERROR: Unknown endianess\n" );
      *pu32_Value = 0;
      return 0;
   }

   //CFR_DBG_OUT( "0x%02x 0x%02x 0x%02x 0x%02x = 0x%08x\n", pu8_32[0], pu8_32[1], pu8_32[2], pu8_32[3], *pu32_Value );

#endif

   return sizeof(u32);
}

//*/
