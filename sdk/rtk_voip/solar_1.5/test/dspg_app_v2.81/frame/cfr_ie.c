/*!
*  \file       cfr_ie.c
*  \brief
*  \Author     kelbch
*
*  @(#)  %filespec: cfr_ie.c~DMZD53#3 %
*
*******************************************************************************
*  \par  History
*  \n==== History ============================================================\n
*  date        name     version   action                                          \n
*  ----------------------------------------------------------------------------\n
*******************************************************************************/

#if defined(__arm)
    #include "tclib.h"
    #include "embedded.h"
#else
    #include <stdlib.h>
    #include <stdio.h>
    #include <string.h>
#endif

#if defined( __linux__ )
    #include <pthread.h>
#endif

#include "cmbs_int.h"      /* internal API structure and defines */
#include "cfr_uart.h"      /* packet handler */

#if defined( CMBS_API_TARGET )
    #include "tapp_log.h"
#endif

#include "cfr_debug.h"     /* debug handling */


void *      cfr_ie_ListNew( u8* pu8_Buffer, u16 u16_Size )
{
    PST_CFR_IE_LIST p_List = &g_CMBSInstance.st_ApplSlot.st_TransmitterIEList;
    u8 u8_idx;
#if defined ( CMBS_API_TARGET )
#else
    u8  u8_idx_max;
    u32 u32_CurrentThreadId;
#endif // defined ( CMBS_API_TARGET )

    if ( !u16_Size || !pu8_Buffer )
    {
        memset(p_List, 0, sizeof(ST_CFR_IE_LIST));

#if defined ( CMBS_API_TARGET )
        // When target, always use index 0
        u8_idx = 0;
#else
        // Linux or WIN32 on host

        u8_idx_max = sizeof(g_CMBSInstance.st_ApplSlot.u8_IEBuffers) / sizeof(g_CMBSInstance.st_ApplSlot.u8_IEBuffers[0]);
        // CFR_DBG_OUT("u8_idx_max=%d\n", u8_idx_max);

    #if defined ( WIN32 )
        u32_CurrentThreadId = (u32)GetCurrentThreadId();
    #endif // defined ( WIN32 )

    #if defined ( __linux__ )
        u32_CurrentThreadId = (u32)pthread_self();
    #endif // defined ( __linux__ )

        // CFR_DBG_OUT("u32_CurrentThreadId=%d\n", u32_CurrentThreadId);

        for ( u8_idx = 0; u8_idx < u8_idx_max; u8_idx++ )
        {
            if ( g_CMBSInstance.st_ApplSlot.u32_ThreadIdArray[u8_idx] == u32_CurrentThreadId )
            {
                // Current u8_idx represents the index of u8_IEBuffers
                break;
            }
        }

        if ( u8_idx == u8_idx_max )
        {
            for ( u8_idx = 0; u8_idx < u8_idx_max; u8_idx++ )
            {
                if ( g_CMBSInstance.st_ApplSlot.u32_ThreadIdArray[u8_idx] == 0 )
                {
                    // We found a free entry
                    g_CMBSInstance.st_ApplSlot.u32_ThreadIdArray[u8_idx] = u32_CurrentThreadId;
                    // CFR_DBG_OUT("New entry %d assign to thread id %d\n", u8_idx, u32_CurrentThreadId);
                    break;
                }
            }

            if ( u8_idx == u8_idx_max )
            {
                // CFR_DBG_OUT("Not enough entries to store Thread Id !!!\n");
                return NULL;
            }
        }
#endif // defined ( CMBS_API_TARGET )

        p_List->pu8_Buffer = g_CMBSInstance.st_ApplSlot.u8_IEBuffers[u8_idx];
        p_List->u16_MaxSize= sizeof(g_CMBSInstance.st_ApplSlot.u8_IEBuffers[u8_idx]);

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
    if ( p_List->u16_MaxSize > ( u16_Size + p_List->u16_CurSize ) )
    {
        memcpy( p_List->pu8_Buffer + p_List->u16_CurSize, pu8_Buffer, u16_Size );
        p_List->u16_CurSize += u16_Size;

        return 0;
    }

    CFR_DBG_ERROR("[ERROR] cfr_ie_ItemAdd MaxSize=%d u16_Size=%d u16_CurSize=%d", p_List->u16_MaxSize, u16_Size, p_List->u16_CurSize);

    return -1;
}

u8 *        cfr_ie_ItemGet( PST_CFR_IE_LIST p_List )
{
    return p_List->pu8_Buffer + p_List->u16_CurIE;
}

u8 *        cfr_ie_ItemFirstGet ( PST_CFR_IE_LIST p_List )
{
    p_List->u16_CurIE = 0;
    return cfr_ie_ItemGet( p_List );
}


u8 *        cfr_ie_ItemNextGet( PST_CFR_IE_LIST p_List )
{
    u16      u16_NextIE, u16_IESize;

    if ( p_List->pu8_Buffer[ p_List->u16_CurIE ] == CMBS_IE_DATA       ||
         p_List->pu8_Buffer[ p_List->u16_CurIE ] == CMBS_IE_PARAMETER  ||
         p_List->pu8_Buffer[ p_List->u16_CurIE ] == CMBS_IE_PARAMETER_AREA )
    {
        // long IE with 16 bit size
        cfr_ie_dser_u16( p_List->pu8_Buffer + p_List->u16_CurIE + CFR_IE_SIZE_POS,
                         &u16_IESize );

        u16_NextIE = p_List->u16_CurIE  +  // start of current IE
                     3                  +  // IE type + IE length 16 bit
                     u16_IESize;           // size of current IE 
    }
    else
    {
        // short IE with 8 bit size
        u16_NextIE = p_List->u16_CurIE  +  // start of current IE
                     2                  +  // IE type + IE length 8 bit
                     // size of current IE
                     p_List->pu8_Buffer[ p_List->u16_CurIE + CFR_IE_SIZE_POS ]; 
    }

    if ( p_List->u16_CurSize > u16_NextIE )
    {
        p_List->u16_CurIE = u16_NextIE;

        return cfr_ie_ItemGet( p_List );
    }

    return NULL;
}


u16            cfr_ie_ser_u16( u8 * pu8_Buffer, u16 u16_Value )
{
#if defined( CMBS_API_TARGET )

    memcpy( pu8_Buffer, &u16_Value, sizeof(u16_Value));

#else                               // Host
    u8 * pu8_16 = (u8*)&u16_Value;

    if ( g_CMBSInstance.e_Endian == E_CMBS_ENDIAN_LITTLE )
    {
        memcpy( pu8_Buffer, &u16_Value, sizeof(u16_Value));
    }
    else
    {
        pu8_Buffer[0] = pu8_16[1];
        pu8_Buffer[1] = pu8_16[0];
    }
#endif

    return sizeof(u16_Value);
}

u16            cfr_ie_ser_u32( u8 * pu8_Buffer, u32 u32_Value )
{
#if defined( CMBS_API_TARGET )

    memcpy( pu8_Buffer, &u32_Value, sizeof(u32));

#else                               // Host
    u8 * pu8_32 = (u8*)&u32_Value;

    if ( g_CMBSInstance.e_Endian == E_CMBS_ENDIAN_LITTLE )
    {
        memcpy( pu8_Buffer, &u32_Value, sizeof(u32));
    }
    else if ( g_CMBSInstance.e_Endian == E_CMBS_ENDIAN_BIG )
    {
        pu8_Buffer[0] = pu8_32[3];
        pu8_Buffer[1] = pu8_32[2];
        pu8_Buffer[2] = pu8_32[1];
        pu8_Buffer[3] = pu8_32[0];
    }
    else if ( g_CMBSInstance.e_Endian == E_CMBS_ENDIAN_MIXED )
    {
        pu8_Buffer[0] = pu8_32[1];
        pu8_Buffer[1] = pu8_32[0];
        pu8_Buffer[2] = pu8_32[3];
        pu8_Buffer[3] = pu8_32[2];
    }
    else
    {
        return 0;
    }
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

    if ( g_CMBSInstance.e_Endian == E_CMBS_ENDIAN_LITTLE )
    {
        memcpy( pu8_16, pu8_Buffer, sizeof(u16));
    }
    else
    {
        pu8_16[1] = pu8_Buffer[0];
        pu8_16[0] = pu8_Buffer[1];
    }
#endif

    return sizeof(u16);
}

u16            cfr_ie_dser_u32( u8 * pu8_Buffer, u32 * pu32_Value )
{
#if defined( CMBS_API_TARGET )

    memcpy( pu32_Value, pu8_Buffer, sizeof(u32));

#else                               // Host
    u8 *        pu8_32 = (u8*)pu32_Value;

    if ( g_CMBSInstance.e_Endian == E_CMBS_ENDIAN_LITTLE )
    {
        memcpy( pu8_32, pu8_Buffer, sizeof(u32));
    }
    else if ( g_CMBSInstance.e_Endian == E_CMBS_ENDIAN_BIG )
    {
        pu8_32[3] = pu8_Buffer[0];
        pu8_32[2] = pu8_Buffer[1];
        pu8_32[1] = pu8_Buffer[2];
        pu8_32[0] = pu8_Buffer[3];
    }
    else if ( g_CMBSInstance.e_Endian == E_CMBS_ENDIAN_MIXED )
    {
        pu8_32[1] = pu8_Buffer[0];
        pu8_32[0] = pu8_Buffer[1];
        pu8_32[3] = pu8_Buffer[2];
        pu8_32[2] = pu8_Buffer[3];
    }
    else
    {
        *pu32_Value = 0;
        return 0;
    }
#endif

    return sizeof(u32);
}


u16            cfr_ie_dser_pu8( u8 * pu8_Buffer, u8 * pu8_Data, u16 u16_DataSize )
{
    memcpy ( pu8_Data, pu8_Buffer, u16_DataSize );
    return u16_DataSize;
}


//*/
