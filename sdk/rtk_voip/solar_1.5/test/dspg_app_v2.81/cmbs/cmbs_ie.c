/*!
*	\file			cmbs_ie.c
*	\brief
*	\Author		stein
*
*	@(#)	%filespec: cmbs_ie.c~DMZD53#9 %
*
*******************************************************************************
*	\par	History
*	\n==== History =============================================================\n
*	date			name		version	 action                                       \n
*	----------------------------------------------------------------------------\n
*  25-Feb-09   stein    61        Initial revision                             \n
*  25-Feb-09   stein    62        CallInstance, CallerID, CalledID - Add/Get   \n
*  25-Feb-09   stein    64        CallerName, CallInfo, ReleaseReason, MediaChannel \n
*  25-Feb-09   stein    65        MediaDesc, Time, FwVersion, SysLog \n

*******************************************************************************/

#if defined(__arm)
    #include "tclib.h"
    #include "embedded.h"
#else
    #include <stdlib.h>
    #include <stdio.h>
    #include <string.h>
#endif

#include "cmbs_int.h"
#include "cfr_ie.h"
#include "cfr_debug.h"

/*****************************************************************************
 * Information Elements List functions
 *****************************************************************************/

void *            cmbs_api_ie_GetList( void )
{
    /*! on linux side a dynamic memory behavior is possible */
    return cfr_ie_ListNew( NULL, 0);
}


E_CMBS_RC         cmbs_api_ie_FreeList( void * pv_RefIEList )
{
    if ( pv_RefIEList )
    {
        CFR_DBG_OUT( "cmbs_api_ie_FreeList: Dynamic is not implemented\n" );
    }

    return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_api_ie_GetFirst( void * pv_RefIEList, void ** ppv_RefIE, u16 * pu16_IEType )
{
    u8 * pu8_Tmp;

    pu8_Tmp = cfr_ie_ItemFirstGet( (PST_CFR_IE_LIST)pv_RefIEList );
    *ppv_RefIE = (void *)pu8_Tmp;

    *pu16_IEType = (u16)*pu8_Tmp;

    return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_api_ie_GetNext( void * pv_RefIEList, void ** ppv_RefIE, u16 * pu16_IEType )
{
    u8 * pu8_Tmp = NULL;

    pu8_Tmp = cfr_ie_ItemNextGet( (PST_CFR_IE_LIST)pv_RefIEList );

    if ( pu8_Tmp )
    {
        *ppv_RefIE = (void *)pu8_Tmp;

        *pu16_IEType = (u16)*pu8_Tmp;
    }
    else
    {
        *ppv_RefIE = NULL;

        *pu16_IEType = 0;
    }

    return CMBS_RC_OK;
}


/*****************************************************************************
 * Information Elements ADD / GET functions
 *****************************************************************************/

E_CMBS_RC         cmbs_api_ie_LineIdAdd( void * pv_RefIEList, u8 u8_LineId )
{
    u8  u8_Buffer[3];

    u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_LINE_ID;
    u8_Buffer[CFR_IE_SIZE_POS] = 1;  // sizeof(u8)

    u8_Buffer[CFR_IE_HEADER_SIZE] = u8_LineId;

    cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, 3 );

    return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_api_ie_LineIdGet( void * pv_RefIE, u8 * pu8_LineId )
{
    u8 * pu8_Buffer = (u8*)pv_RefIE;

    if ( pu8_Buffer[CFR_IE_TYPE_POS] == CMBS_IE_LINE_ID )
    {
        *pu8_LineId = pu8_Buffer[CFR_IE_HEADER_SIZE];

        return CMBS_RC_OK;
    }

    return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_api_ie_CallInstanceAdd( void * pv_RefIEList, u32 u32_CallInstance )
{
    u8  u8_Buffer[8];
    u16 u16_Size;

    memset( u8_Buffer, 0, sizeof(u8_Buffer) );

    u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_CALLINSTANCE;
    u8_Buffer[CFR_IE_SIZE_POS] = sizeof(u32);

    cfr_ie_ser_u32( u8_Buffer + CFR_IE_HEADER_SIZE, u32_CallInstance );

    u16_Size = u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE;

    cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u16_Size );

    return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_api_ie_CallInstanceGet( void * pv_RefIE, u32 * pu32_CallInstance )
{
    u8 * pu8_Buffer = (u8*)pv_RefIE;

    if ( pu8_Buffer[CFR_IE_TYPE_POS] == CMBS_IE_CALLINSTANCE )
    {
        cfr_ie_dser_u32( pu8_Buffer + CFR_IE_HEADER_SIZE, pu32_CallInstance );

        return CMBS_RC_OK;
    }

    return CMBS_RC_ERROR_PARAMETER;
}

E_CMBS_RC         cmbs_api_ie_CallTransferReqAdd( void * pv_RefIEList, PST_IE_CALLTRANSFERREQ pst_CallTrf )
{
    u8  u8_Buffer[16];
    u16 u16_Size;

    memset( u8_Buffer, 0, sizeof(u8_Buffer) );

    u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_CALLTRANSFERREQ;
    u8_Buffer[CFR_IE_SIZE_POS] = 2 * sizeof(u32) + sizeof(u8);

    cfr_ie_ser_u32( u8_Buffer + CFR_IE_HEADER_SIZE, pst_CallTrf->u32_CallInstanceFrom );
    cfr_ie_ser_u32( u8_Buffer + CFR_IE_HEADER_SIZE + sizeof(u32), pst_CallTrf->u32_CallInstanceTo );

    u8_Buffer[CFR_IE_HEADER_SIZE + 2*sizeof(u32)] = pst_CallTrf->u8_TermInstance;

    u16_Size = u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE;

    cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u16_Size );

    return CMBS_RC_OK;

}

E_CMBS_RC         cmbs_api_ie_CallTransferReqGet( void * pv_RefIE, PST_IE_CALLTRANSFERREQ pst_CallTrf )
{
    u8 * pu8_Buffer = (u8*)pv_RefIE;

    if ( *pu8_Buffer == CMBS_IE_CALLTRANSFERREQ )
    {
        cfr_ie_dser_u32( pu8_Buffer + CFR_IE_HEADER_SIZE, &pst_CallTrf->u32_CallInstanceFrom );
        cfr_ie_dser_u32( pu8_Buffer + CFR_IE_HEADER_SIZE + sizeof(u32), &pst_CallTrf->u32_CallInstanceTo);
        pst_CallTrf->u8_TermInstance = pu8_Buffer[CFR_IE_HEADER_SIZE + 2*sizeof(u32)];

        return CMBS_RC_OK;
    }

    return CMBS_RC_ERROR_PARAMETER;

}

E_CMBS_RC         cmbs_api_ie_CallerPartyAdd( void * pv_RefIEList, ST_IE_CALLERPARTY * pst_CallerParty )
{
    u8  u8_Buffer[32];
    u16 u16_Size;

    memset( u8_Buffer, 0, sizeof(u8_Buffer) );

    u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_CALLERPARTY;
    u8_Buffer[CFR_IE_SIZE_POS] = 3 * sizeof(u8) + pst_CallerParty->u8_AddressLen;

    u8_Buffer[2] = pst_CallerParty->u8_AddressProperties;
    u8_Buffer[3] = pst_CallerParty->u8_AddressPresentation;
    u8_Buffer[4] = pst_CallerParty->u8_AddressLen;

    cfr_ie_ser_pu8( u8_Buffer + CFR_IE_HEADER_SIZE + 3 * sizeof(u8),
                    pst_CallerParty->pu8_Address,
                    pst_CallerParty->u8_AddressLen );

    u16_Size = u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE;

    cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u16_Size );

    return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_api_ie_CallerPartyGet( void * pv_RefIE, ST_IE_CALLERPARTY * pst_CallerParty )
{
    u8 * pu8_Buffer = (u8*)pv_RefIE;

    if ( *pu8_Buffer == CMBS_IE_CALLERPARTY )
    {
        pst_CallerParty->u8_AddressProperties   = pu8_Buffer[CFR_IE_HEADER_SIZE];
        pst_CallerParty->u8_AddressPresentation = pu8_Buffer[CFR_IE_HEADER_SIZE + sizeof(u8)];
        pst_CallerParty->u8_AddressLen          = pu8_Buffer[CFR_IE_HEADER_SIZE + 2 * sizeof(u8)];
        pst_CallerParty->pu8_Address            = pu8_Buffer + CFR_IE_HEADER_SIZE + 3 * sizeof(u8);

        return CMBS_RC_OK;
    }

    return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_api_ie_CalledPartyAdd( void * pv_RefIEList, ST_IE_CALLEDPARTY * pst_CalledParty )
{
    u8  u8_Buffer[32];
    u16 u16_Size;

    memset( u8_Buffer, 0, sizeof(u8_Buffer) );

    u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_CALLEDPARTY;
    u8_Buffer[CFR_IE_SIZE_POS] = 3 * sizeof(u8) + pst_CalledParty->u8_AddressLen;

    u8_Buffer[2] = pst_CalledParty->u8_AddressProperties;
    u8_Buffer[3] = pst_CalledParty->u8_AddressPresentation;
    u8_Buffer[4] = pst_CalledParty->u8_AddressLen;

    cfr_ie_ser_pu8( u8_Buffer + CFR_IE_HEADER_SIZE + 3 * sizeof(u8),
                    pst_CalledParty->pu8_Address,
                    pst_CalledParty->u8_AddressLen );

    u16_Size = u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE;

    cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u16_Size );

    return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_api_ie_CalledPartyGet( void * pv_RefIE, ST_IE_CALLEDPARTY * pst_CalledParty )
{
    u8 * pu8_Buffer = (u8*)pv_RefIE;

    if ( *pu8_Buffer == CMBS_IE_CALLEDPARTY )
    {
        pst_CalledParty->u8_AddressProperties   = pu8_Buffer[CFR_IE_HEADER_SIZE];
        pst_CalledParty->u8_AddressPresentation = pu8_Buffer[CFR_IE_HEADER_SIZE + sizeof(u8)];
        pst_CalledParty->u8_AddressLen          = pu8_Buffer[CFR_IE_HEADER_SIZE + 2 * sizeof(u8)];
        pst_CalledParty->pu8_Address            = pu8_Buffer + CFR_IE_HEADER_SIZE + 3 * sizeof(u8);

        return CMBS_RC_OK;
    }

    return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_api_ie_CallerNameAdd( void * pv_RefIEList, ST_IE_CALLERNAME * pst_CallerName )
{
    u8  u8_Buffer[32];
    u16 u16_Size;

    memset( u8_Buffer, 0, sizeof(u8_Buffer) );

    u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_CALLERNAME;
    u8_Buffer[CFR_IE_SIZE_POS] = sizeof(u8) + pst_CallerName->u8_DataLen;

    u8_Buffer[CFR_IE_HEADER_SIZE] = pst_CallerName->u8_DataLen;
    cfr_ie_ser_pu8( u8_Buffer + CFR_IE_HEADER_SIZE + sizeof(u8), pst_CallerName->pu8_Name, pst_CallerName->u8_DataLen );

    u16_Size = u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE;

    cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u16_Size );

    return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_api_ie_CallerNameGet( void * pv_RefIE, ST_IE_CALLERNAME * pst_CallerName )
{
    u8 * pu8_Buffer = (u8*)pv_RefIE;

    if ( *pu8_Buffer == CMBS_IE_CALLERNAME )
    {
        pst_CallerName->u8_DataLen = pu8_Buffer[CFR_IE_HEADER_SIZE];
        pst_CallerName->pu8_Name   = pu8_Buffer + CFR_IE_HEADER_SIZE + sizeof(u8);

        return CMBS_RC_OK;
    }

    return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_api_ie_CallProgressAdd( void * pv_RefIEList, ST_IE_CALLPROGRESS * pst_CallProgress )
{
    u8  u8_Buffer[4];
    u16 u16_Size;

    memset( u8_Buffer, 0, sizeof(u8_Buffer) );

    u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_CALLPROGRESS;
    u8_Buffer[CFR_IE_SIZE_POS] = sizeof(u8);

    u8_Buffer[CFR_IE_HEADER_SIZE] = (u8)pst_CallProgress->e_Progress;

    u16_Size = u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE;

    cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u16_Size );

    return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_api_ie_CallProgressGet( void * pv_RefIE, ST_IE_CALLPROGRESS * pst_CallProgress )
{
    u8 * pu8_Buffer = (u8*)pv_RefIE;

    if ( pu8_Buffer[CFR_IE_TYPE_POS] == CMBS_IE_CALLPROGRESS )
    {
        pst_CallProgress->e_Progress = pu8_Buffer[CFR_IE_HEADER_SIZE];

        return CMBS_RC_OK;
    }

    return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_api_ie_CallInfoAdd( void * pv_RefIEList, ST_IE_CALLINFO * pst_CallInfo )
{
    u8  u8_Buffer[32];
    u16 u16_Size;

    memset( u8_Buffer, 0, sizeof(u8_Buffer) );

    u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_CALLINFO;
    u8_Buffer[CFR_IE_SIZE_POS] = 2 * sizeof(u8) + pst_CallInfo->u8_DataLen;

    u8_Buffer[CFR_IE_HEADER_SIZE] = (u8)pst_CallInfo->e_Type;
    u8_Buffer[CFR_IE_HEADER_SIZE + sizeof(u8)] = pst_CallInfo->u8_DataLen;

    cfr_ie_ser_pu8( u8_Buffer + CFR_IE_HEADER_SIZE + 2 * sizeof(u8),
                    pst_CallInfo->pu8_Info,
                    pst_CallInfo->u8_DataLen );

    u16_Size = u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE;

    cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u16_Size );

    return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_api_ie_CallInfoGet( void * pv_RefIE, ST_IE_CALLINFO * pst_CallInfo )
{
    u8 * pu8_Buffer = (u8*)pv_RefIE;

    if ( pu8_Buffer[CFR_IE_TYPE_POS] == CMBS_IE_CALLINFO )
    {
        pst_CallInfo->e_Type = pu8_Buffer[CFR_IE_HEADER_SIZE];

        pst_CallInfo->u8_DataLen = pu8_Buffer[CFR_IE_HEADER_SIZE + sizeof(u8)];
        pst_CallInfo->pu8_Info = pu8_Buffer + CFR_IE_HEADER_SIZE + 2 * sizeof(u8);

        return CMBS_RC_OK;
    }

    return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_api_ie_DisplayStringAdd( void * pv_RefIEList, ST_IE_DISPLAY_STRING * pst_DisplayString )
{
    u8  u8_Buffer[36];
    u16 u16_Size;

    memset( u8_Buffer, 0, sizeof(u8_Buffer) );

    u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_CALLERNAME;
    u8_Buffer[CFR_IE_SIZE_POS] = sizeof(u8) + pst_DisplayString->u8_DataLen;

    u8_Buffer[CFR_IE_HEADER_SIZE] = pst_DisplayString->u8_DataLen;
    cfr_ie_ser_pu8( u8_Buffer + CFR_IE_HEADER_SIZE + sizeof(u8), pst_DisplayString->pu8_Info,
                    (u16)MIN(pst_DisplayString->u8_DataLen, CMBS_DISPLAY_STRING_LENGTH) );

    u16_Size = u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE;

    cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u16_Size );

    return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_api_ie_CallReleaseReasonAdd( void * pv_RefIEList, ST_IE_RELEASE_REASON * pst_RelReason )
{
    u8  u8_Buffer[8];
    u16 u16_Size;

    memset( u8_Buffer, 0, sizeof(u8_Buffer) );

    u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_CALLRELEASE_REASON;
    u8_Buffer[CFR_IE_SIZE_POS] = sizeof(u8) + sizeof(u32);

    u8_Buffer[CFR_IE_HEADER_SIZE] = pst_RelReason->e_Reason;
    cfr_ie_ser_u32( u8_Buffer + CFR_IE_HEADER_SIZE + sizeof(u8), pst_RelReason->u32_ExtReason );

    u16_Size = u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE;

    cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u16_Size );

    return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_api_ie_CallReleaseReasonGet( void * pv_RefIE, ST_IE_RELEASE_REASON * pst_RelReason )
{
    u8 * pu8_Buffer = (u8*)pv_RefIE;

    if ( pu8_Buffer[CFR_IE_TYPE_POS] == CMBS_IE_CALLRELEASE_REASON )
    {
        pst_RelReason->e_Reason = pu8_Buffer[CFR_IE_HEADER_SIZE];
        cfr_ie_dser_u32( pu8_Buffer + CFR_IE_HEADER_SIZE + sizeof(u8), &pst_RelReason->u32_ExtReason );

        return CMBS_RC_OK;
    }

    return CMBS_RC_ERROR_PARAMETER;
}

E_CMBS_RC         cmbs_api_ie_CallStateGet( void * pv_RefIE, ST_IE_CALL_STATE * pst_CallState )
{
    u8 * pu8_Buffer = (u8*)pv_RefIE;

    if ( pu8_Buffer[CFR_IE_TYPE_POS] == CMBS_IE_CALLSTATE )
    {
        pst_CallState->u8_ActCallID = pu8_Buffer[CFR_IE_HEADER_SIZE];
        pst_CallState->e_CallType = pu8_Buffer[CFR_IE_HEADER_SIZE + 1];
        pst_CallState->e_CallStatus = pu8_Buffer[CFR_IE_HEADER_SIZE + 2];
        cfr_ie_dser_u16( &pu8_Buffer[CFR_IE_HEADER_SIZE + 3], &pst_CallState->u16_HandsetsMask );
        pst_CallState->u8_LinesMask = pu8_Buffer[CFR_IE_HEADER_SIZE + 5];

        return CMBS_RC_OK;
    }

    return CMBS_RC_ERROR_PARAMETER;
}

E_CMBS_RC         cmbs_api_ie_MediaChannelAdd( void * pv_RefIEList, ST_IE_MEDIA_CHANNEL * pst_MediaChannel )
{
    u8  u8_Buffer[12];
    u16 u16_Size;

    memset( u8_Buffer, 0, sizeof(u8_Buffer) );

    u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_MEDIACHANNEL;
    u8_Buffer[CFR_IE_SIZE_POS] = 2* sizeof(u32) + sizeof(u8);

    cfr_ie_ser_u32( u8_Buffer + CFR_IE_HEADER_SIZE, pst_MediaChannel->u32_ChannelID );
    cfr_ie_ser_u32( u8_Buffer + CFR_IE_HEADER_SIZE +sizeof(u32), pst_MediaChannel->u32_ChannelParameter );
    u8_Buffer[CFR_IE_HEADER_SIZE + (2*sizeof(u32))] = pst_MediaChannel->e_Type;

    u16_Size = u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE;

    cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u16_Size );

    return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_api_ie_MediaChannelGet( void * pv_RefIE, ST_IE_MEDIA_CHANNEL * pst_MediaChannel )
{
    u8 * pu8_Buffer = (u8*)pv_RefIE;

    if ( pu8_Buffer[CFR_IE_TYPE_POS] == CMBS_IE_MEDIACHANNEL )
    {
        cfr_ie_dser_u32( pu8_Buffer + CFR_IE_HEADER_SIZE, &pst_MediaChannel->u32_ChannelID );
        cfr_ie_dser_u32( pu8_Buffer + CFR_IE_HEADER_SIZE + sizeof(u32), &pst_MediaChannel->u32_ChannelParameter );
        pst_MediaChannel->e_Type = pu8_Buffer[CFR_IE_HEADER_SIZE + (2*sizeof(u32))];

        return CMBS_RC_OK;
    }

    return CMBS_RC_ERROR_PARAMETER;
}

E_CMBS_RC         cmbs_api_ie_MediaICAdd( void * pv_RefIEList, ST_IE_MEDIA_INTERNAL_CONNECT * pst_MediaIC )
{
    u8  u8_Buffer[12];
    u16 u16_Size;

    memset( u8_Buffer, 0, sizeof(u8_Buffer) );

    u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_MEDIA_INTERNAL_CONNECT;
    u8_Buffer[CFR_IE_SIZE_POS] = 2* sizeof(u32) + sizeof(u8);

    cfr_ie_ser_u32( u8_Buffer + CFR_IE_HEADER_SIZE, pst_MediaIC->u32_ChannelID );
    cfr_ie_ser_u32( u8_Buffer + CFR_IE_HEADER_SIZE +sizeof(u32), pst_MediaIC->u32_NodeId );
    u8_Buffer[CFR_IE_HEADER_SIZE + (2*sizeof(u32))] = pst_MediaIC->e_Type;

    u16_Size = u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE;

    cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u16_Size );

    return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_api_ie_MediaICGet( void * pv_RefIE, ST_IE_MEDIA_INTERNAL_CONNECT * pst_MediaIC )
{
    u8 * pu8_Buffer = (u8*)pv_RefIE;

    if ( pu8_Buffer[CFR_IE_TYPE_POS] == CMBS_IE_MEDIA_INTERNAL_CONNECT )
    {
        cfr_ie_dser_u32( pu8_Buffer + CFR_IE_HEADER_SIZE, &pst_MediaIC->u32_ChannelID );
        cfr_ie_dser_u32( pu8_Buffer + CFR_IE_HEADER_SIZE + sizeof(u32), &pst_MediaIC->u32_NodeId );
        pst_MediaIC->e_Type = pu8_Buffer[CFR_IE_HEADER_SIZE + (2*sizeof(u32))];

        return CMBS_RC_OK;
    }

    return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_api_ie_MediaDescAdd( void * pv_RefIEList, ST_IE_MEDIA_DESCRIPTOR * pst_MediaDesc )
{
    u8  u8_Buffer[4+CMBS_AUDIO_CODEC_MAX];
    u16 u16_Size;

    memset( u8_Buffer, 0, sizeof(u8_Buffer) );

    u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_MEDIADESCRIPTOR;
    u8_Buffer[CFR_IE_SIZE_POS] = sizeof(u8) * 2 + pst_MediaDesc->u8_CodecsLength;

    u8_Buffer[CFR_IE_HEADER_SIZE] = pst_MediaDesc->e_Codec;
    u8_Buffer[CFR_IE_HEADER_SIZE + sizeof(u8)] = pst_MediaDesc->u8_CodecsLength;

    cfr_ie_ser_pu8( u8_Buffer + CFR_IE_HEADER_SIZE + sizeof(u8) * 2, pst_MediaDesc->pu8_CodecsList, pst_MediaDesc->u8_CodecsLength );

    u16_Size = u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE;

    cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u16_Size );

    return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_api_ie_MediaDescGet( void * pv_RefIE, ST_IE_MEDIA_DESCRIPTOR * pst_MediaDesc )
{
    u8 * pu8_Buffer = (u8*)pv_RefIE;

    if ( pu8_Buffer[CFR_IE_TYPE_POS] == CMBS_IE_MEDIADESCRIPTOR )
    {
        pst_MediaDesc->e_Codec = pu8_Buffer[CFR_IE_HEADER_SIZE];
        pst_MediaDesc->u8_CodecsLength = pu8_Buffer[CFR_IE_HEADER_SIZE + sizeof(u8)];
        cfr_ie_dser_pu8( pu8_Buffer + CFR_IE_HEADER_SIZE + sizeof(u8) * 2, pst_MediaDesc->pu8_CodecsList, pst_MediaDesc->u8_CodecsLength );

        return CMBS_RC_OK;
    }

    return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_api_ie_ToneAdd( void * pv_RefIEList, ST_IE_TONE * pst_Tone )
{
    u8  u8_Buffer[4];
    u16 u16_Size;

    memset( u8_Buffer, 0, sizeof(u8_Buffer) );

    u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_TONE;
    u8_Buffer[CFR_IE_SIZE_POS] = sizeof(u8);

    u8_Buffer[CFR_IE_HEADER_SIZE] = (u8)pst_Tone->e_Tone;

    u16_Size = u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE;

    cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u16_Size );

    return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_api_ie_TimeAdd( void * pv_RefIEList, ST_IE_TIMEOFDAY * pst_TimeOfDay )
{
    u8  u8_Buffer[8];
    u16 u16_Size;

    memset( u8_Buffer, 0, sizeof(u8_Buffer) );

    u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_TIMEOFDAY;
    u8_Buffer[CFR_IE_SIZE_POS] = sizeof(u32);

    cfr_ie_ser_u32( u8_Buffer + CFR_IE_HEADER_SIZE, pst_TimeOfDay->u32_Timestamp );

    u16_Size = u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE;

    cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u16_Size );

    return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_api_ie_TimeGet( void * pv_RefIE, ST_IE_TIMEOFDAY * pst_TimeOfDay )
{
    u8 * pu8_Buffer = (u8*)pv_RefIE;

    if ( pu8_Buffer[CFR_IE_TYPE_POS] == CMBS_IE_TIMEOFDAY )
    {
        cfr_ie_dser_u32( pu8_Buffer + CFR_IE_HEADER_SIZE, &pst_TimeOfDay->u32_Timestamp );

        return CMBS_RC_OK;
    }

    return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_api_ie_HandsetInfoGet( void * pv_RefIE, ST_IE_HANDSETINFO * pst_HandsetInfo )
{
    u8 * pu8_Buffer = (u8*)pv_RefIE;

    if ( *pu8_Buffer == CMBS_IE_HANDSETINFO )
    {
        u8 u8_Idx = CFR_IE_HEADER_SIZE;

        pst_HandsetInfo->u8_Hs    = pu8_Buffer[u8_Idx++];
        pst_HandsetInfo->u8_State = pu8_Buffer[u8_Idx++];
        pst_HandsetInfo->e_Type   = pu8_Buffer[u8_Idx++];

        u8_Idx += cfr_ie_dser_u32( pu8_Buffer + u8_Idx, &pst_HandsetInfo->u32_ModelNr );

        memcpy( pst_HandsetInfo->u8_IPEI, pu8_Buffer + u8_Idx, sizeof(pst_HandsetInfo->u8_IPEI) );

        return CMBS_RC_OK;
    }

    return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_api_ie_ParameterGet( void * pv_RefIE, ST_IE_PARAMETER * pst_Parameter )
{
    u8 * pu8_Buffer = (u8*)pv_RefIE;

    if ( *pu8_Buffer == CMBS_IE_PARAMETER )
    {
        u8 u8_Idx = 3;                // offset of IE data after: 8 bit IE type + 16 bit IE size

        pst_Parameter->e_Param     = pu8_Buffer[u8_Idx++];      
        pst_Parameter->e_ParamType = pu8_Buffer[u8_Idx++];

        u8_Idx += cfr_ie_dser_u16( pu8_Buffer + u8_Idx, &pst_Parameter->u16_DataLen );

        if ( pst_Parameter->u16_DataLen )
        {
            pst_Parameter->pu8_Data = pu8_Buffer + u8_Idx;
        }
        else
        {
            pst_Parameter->pu8_Data = NULL;
        }

        return CMBS_RC_OK;
    }

    return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_api_ie_SubscribedHSListAdd( void * pv_RefIE, ST_IE_SUBSCRIBED_HS_LIST * pst_SubscribedHsList )
{
    u8  u8_Buffer[128];
    u8  u8_BufPos = 0;
    u16 u16_Size = 0;
    u8  u8_Index = 0;

    if ( !pst_SubscribedHsList )
    {
        return CMBS_RC_ERROR_PARAMETER;
    }

    memset( u8_Buffer, 0, sizeof(u8_Buffer) );

    u16_Size = sizeof(ST_IE_SUBSCRIBED_HS_LIST);
    u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_SUBSCRIBED_HS_LIST;
    u8_Buffer[CFR_IE_SIZE_POS] = (u8)u16_Size;

    u8_BufPos = CFR_IE_HEADER_SIZE;
    cfr_ie_ser_u16( &u8_Buffer[u8_BufPos], pst_SubscribedHsList->u16_HsID);
    u8_BufPos += 2;
    cfr_ie_ser_u16( &u8_Buffer[u8_BufPos], pst_SubscribedHsList->u16_NameLength);
    u8_BufPos += 2;

    for ( u8_Index = 0; u8_Index < CMBS_HS_NAME_MAX_LENGTH; u8_Index++ )
    {
        cfr_ie_ser_u16( &u8_Buffer[u8_BufPos], pst_SubscribedHsList->u16_HsName[u8_Index]);
        u8_BufPos += 2;
    }

    u16_Size = u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE;

    cfr_ie_ItemAdd( pv_RefIE, u8_Buffer, u16_Size );

    return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_api_ie_SubscribedHSListGet( void * pv_RefIE, ST_IE_SUBSCRIBED_HS_LIST * pst_SubscribedHsList )
{
    u8 * pu8_Buffer = (u8*)pv_RefIE;
    u8   u8_Length = 0;
    u8   u8_BufPos = 0;
    u8   u8_Index = 0;

    if ( pst_SubscribedHsList )
    {
        if ( *pu8_Buffer == CMBS_IE_SUBSCRIBED_HS_LIST )
        {
            u8_Length = pu8_Buffer[CFR_IE_SIZE_POS];
            memset ( pst_SubscribedHsList, 0, sizeof (ST_IE_SUBSCRIBED_HS_LIST) );

            if ( u8_Length == (u8)sizeof (ST_IE_SUBSCRIBED_HS_LIST) )
            {
                u8_BufPos = CFR_IE_HEADER_SIZE;
                cfr_ie_dser_u16( &pu8_Buffer[u8_BufPos], &pst_SubscribedHsList->u16_HsID);
                u8_BufPos += 2;
                cfr_ie_dser_u16( &pu8_Buffer[u8_BufPos], &pst_SubscribedHsList->u16_NameLength);
                u8_BufPos += 2;

                for ( u8_Index = 0; u8_Index < CMBS_HS_NAME_MAX_LENGTH; u8_Index++ )
                {
                    cfr_ie_dser_u16( &pu8_Buffer[u8_BufPos], &pst_SubscribedHsList->u16_HsName[u8_Index]);
                    u8_BufPos += 2;
                }

                return CMBS_RC_OK;
            }
        }
    }

    return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_api_ie_LineSettingsListAdd( void * pv_RefIE, ST_IE_LINE_SETTINGS_LIST * pst_LineSettingsList )
{
    u8   u8_Buffer[128];
    u16  u16_Size;
    u8   u8_BufPos = 0;
    u8   u8_Index = 0;

    if ( !pst_LineSettingsList )
    {
        return CMBS_RC_ERROR_PARAMETER;
    }

    memset( u8_Buffer, 0, sizeof(u8_Buffer) );

    u16_Size = sizeof(ST_IE_LINE_SETTINGS_LIST);
    u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_LINE_SETTINGS_LIST;
    u8_Buffer[CFR_IE_SIZE_POS] = (u8)u16_Size;

    u8_BufPos = CFR_IE_HEADER_SIZE;
    u8_Buffer[u8_BufPos] = pst_LineSettingsList->u8_Line_Id;
    u8_Buffer[++u8_BufPos] = pst_LineSettingsList->u8_Line_NameLen;
    u8_Buffer[++u8_BufPos] = pst_LineSettingsList->u8_Call_Intrusion;
    u8_Buffer[++u8_BufPos] = pst_LineSettingsList->u8_Multiple_Calls;

    cfr_ie_ser_u16( &u8_Buffer[++u8_BufPos], pst_LineSettingsList->u16_Attached_HS);
    u8_BufPos += 2;
    for ( u8_Index = 0; u8_Index < 6; u8_Index++ )
    {
        cfr_ie_ser_u16( &u8_Buffer[u8_BufPos], pst_LineSettingsList->u16_LineName[u8_Index]);
        u8_BufPos += 2;
    }

    u16_Size = u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE;

    cfr_ie_ItemAdd( pv_RefIE, u8_Buffer, u16_Size );

    return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_api_ie_LineSettingsListGet( void * pv_RefIE, ST_IE_LINE_SETTINGS_LIST * pst_LineSettingsList )
{
    u8 * pu8_Buffer = (u8*)pv_RefIE;
    u8   u8_Length = 0;
    u8   u8_BufPos = 0;
    u8   u8_Index = 0;

    if ( pst_LineSettingsList )
    {
        if ( *pu8_Buffer == CMBS_IE_LINE_SETTINGS_LIST )
        {
            u8_Length = pu8_Buffer[CFR_IE_SIZE_POS];

            if ( u8_Length == (u8)sizeof (ST_IE_LINE_SETTINGS_LIST) )
            {
                u8_BufPos = CFR_IE_HEADER_SIZE;
                pst_LineSettingsList->u8_Line_Id = pu8_Buffer[u8_BufPos];
                pst_LineSettingsList->u8_Line_NameLen = pu8_Buffer[++u8_BufPos];
                pst_LineSettingsList->u8_Call_Intrusion = pu8_Buffer[++u8_BufPos];
                pst_LineSettingsList->u8_Multiple_Calls = pu8_Buffer[++u8_BufPos];

                cfr_ie_dser_u16( &pu8_Buffer[++u8_BufPos], &pst_LineSettingsList->u16_Attached_HS);
                u8_BufPos += 2;
                for ( u8_Index = 0; u8_Index < 6; u8_Index++ )
                {
                    cfr_ie_dser_u16( &pu8_Buffer[u8_BufPos], &pst_LineSettingsList->u16_LineName[u8_Index]);
                    u8_BufPos += 2;
                }

                return CMBS_RC_OK;
            }
        }
    }

    return CMBS_RC_ERROR_PARAMETER;
}

E_CMBS_RC         cmbs_api_ie_LineSettingsTypeAdd( void * pv_RefIE, ST_IE_LINE_SETTINGS_TYPE * pst_LineSettingsType )
{
    u8  u8_Buffer[12];
    u16 u16_Size;

    if ( !pst_LineSettingsType )
    {
        return CMBS_RC_ERROR_PARAMETER;
    }

    memset( u8_Buffer, 0, sizeof(u8_Buffer) );

    u16_Size = sizeof(u8);
    u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_LINE_SETTINGS_TYPE;
    u8_Buffer[CFR_IE_SIZE_POS] = (u8)u16_Size;

    u8_Buffer[CFR_IE_HEADER_SIZE] = (u8)pst_LineSettingsType->e_LineSettingsType;

    u16_Size = u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE;

    cfr_ie_ItemAdd( pv_RefIE, u8_Buffer, u16_Size );

    return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_api_ie_LineSettingsTypeGet( void * pv_RefIE, ST_IE_LINE_SETTINGS_TYPE * pst_LineSettingsType )
{
    u8 * pu8_Buffer = (u8*)pv_RefIE;
    u8   u8_Length = 0;

    if ( pst_LineSettingsType )
    {
        if ( *pu8_Buffer == CMBS_IE_LINE_SETTINGS_TYPE )
        {
            u8_Length = pu8_Buffer[CFR_IE_SIZE_POS];

            if ( u8_Length == (u8)sizeof (u8) )
            {
                pst_LineSettingsType->e_LineSettingsType = pu8_Buffer[CFR_IE_HEADER_SIZE];
                return CMBS_RC_OK;
            }
        }
    }

    return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_api_ie_ParameterAreaGet( void * pv_RefIE, ST_IE_PARAMETER_AREA * pst_ParameterArea )
{
    u8 * pu8_Buffer = (u8*)pv_RefIE;

    if ( *pu8_Buffer == CMBS_IE_PARAMETER_AREA )
    {
        u8 u8_Idx = 3;                // offset of IE data after: 8 bit IE type + 16 bit IE size

        pst_ParameterArea->e_AreaType = pu8_Buffer[u8_Idx++];      

        u8_Idx += cfr_ie_dser_u32( pu8_Buffer + u8_Idx, &pst_ParameterArea->u32_Offset );
        u8_Idx += cfr_ie_dser_u16( pu8_Buffer + u8_Idx, &pst_ParameterArea->u16_DataLen );

        if ( pst_ParameterArea->u16_DataLen )
        {
            pst_ParameterArea->pu8_Data = pu8_Buffer + u8_Idx;
        }
        else
        {
            pst_ParameterArea->pu8_Data = NULL;
        }

        return CMBS_RC_OK;
    }

    return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_api_ie_FwVersionGet( void * pv_RefIE, ST_IE_FW_VERSION * pst_FwVersion )
{
    u8 * pu8_Buffer = (u8*)pv_RefIE;

    if ( *pu8_Buffer == CMBS_IE_FW_VERSION )
    {
        pst_FwVersion->e_SwModule = pu8_Buffer[CFR_IE_HEADER_SIZE];
        cfr_ie_dser_u16( pu8_Buffer + CFR_IE_HEADER_SIZE + sizeof(u8), &pst_FwVersion->u16_FwVersion );

        return CMBS_RC_OK;
    }

    return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_api_ie_SysLogGet( void * pv_RefIE, ST_IE_SYS_LOG * pst_SysLog )
{
    u8 * pu8_Buffer = (u8*)pv_RefIE;

    if ( *pu8_Buffer == CMBS_IE_SYS_LOG )
    {
        pst_SysLog->u8_DataLen = pu8_Buffer[CFR_IE_SIZE_POS];

        if ( pst_SysLog->u8_DataLen > sizeof(pst_SysLog->u8_Data) )
        {
            return CMBS_RC_ERROR_PARAMETER;
        }

        cfr_ie_dser_pu8( pu8_Buffer + CFR_IE_HEADER_SIZE, (u8 *)pst_SysLog->u8_Data, pst_SysLog->u8_DataLen );

        return CMBS_RC_OK;
    }

    return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_api_ie_SysStatusGet( void * pv_RefIE, ST_IE_SYS_STATUS * pst_SysStatus )
{
    u8 * pu8_Buffer = (u8*)pv_RefIE;

    if ( pu8_Buffer[CFR_IE_TYPE_POS] == CMBS_IE_STATUS )
    {
        pst_SysStatus->e_ModuleStatus = pu8_Buffer[CFR_IE_HEADER_SIZE];

        return CMBS_RC_OK;
    }

    return CMBS_RC_ERROR_PARAMETER;
}

E_CMBS_RC        cmbs_api_ie_ResponseAdd( void * pv_RefIE, ST_IE_RESPONSE * pst_Response )
{
    u8  u8_Buffer[3];
    u16 u16_Size;

    if ( !pst_Response )
    {
        return CMBS_RC_ERROR_PARAMETER;
    }

    memset( u8_Buffer, 0, sizeof(u8_Buffer) );

    u16_Size = sizeof(u8);
    u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_RESPONSE;
    u8_Buffer[CFR_IE_SIZE_POS] = (u8)u16_Size;

    u8_Buffer[CFR_IE_HEADER_SIZE] = (u8)pst_Response->e_Response;

    u16_Size = u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE;

    cfr_ie_ItemAdd( pv_RefIE, u8_Buffer, u16_Size );
    return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_api_ie_ResponseGet( void * pv_RefIE, ST_IE_RESPONSE * pst_Response )
{
    u8 * pu8_Buffer = (u8*)pv_RefIE;

    if ( *pu8_Buffer == CMBS_IE_RESPONSE )
    {
        pst_Response->e_Response  = pu8_Buffer[CFR_IE_HEADER_SIZE];

        return CMBS_RC_OK;
    }

    return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_api_ie_RequestIdGet( void * pv_RefIE, u16 * pu16_RequestId )
{
    u8 * pu8_Buffer = (u8*)pv_RefIE;

    if ( pu8_Buffer[CFR_IE_TYPE_POS] == CMBS_IE_REQUEST_ID )
    {
        cfr_ie_dser_u16( pu8_Buffer + CFR_IE_HEADER_SIZE, pu16_RequestId );

        return CMBS_RC_OK;
    }

    return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_api_ie_HandsetsGet( void * pv_RefIE, u16 * pu16_Handsets )
{
    u8 * pu8_Buffer = (u8*)pv_RefIE;

    if ( pu8_Buffer[CFR_IE_TYPE_POS] == CMBS_IE_HANDSETS )
    {
        cfr_ie_dser_u16( pu8_Buffer + CFR_IE_HEADER_SIZE, pu16_Handsets );

        return CMBS_RC_OK;
    }

    return CMBS_RC_ERROR_PARAMETER;
}

E_CMBS_RC         cmbs_api_ie_HandsetsAdd( void * pv_RefIE, u16  u16_Handsets )
{
    return  cmbs_int_ie_ShortValueAdd( pv_RefIE, u16_Handsets, CMBS_IE_HANDSETS );
}


E_CMBS_RC         cmbs_api_ie_DateTimeGet( void * pv_RefIE, ST_IE_DATETIME * pst_DateTime )
{
    u8 * pu8_Buffer = (u8*)pv_RefIE;

    if ( *pu8_Buffer == CMBS_IE_DATETIME )
    {
        pst_DateTime->e_Coding         = pu8_Buffer[CFR_IE_SIZE_POS + 1];
        pst_DateTime->e_Interpretation = pu8_Buffer[CFR_IE_SIZE_POS + 2];
        pst_DateTime->u8_Year          = pu8_Buffer[CFR_IE_SIZE_POS + 3];
        pst_DateTime->u8_Month         = pu8_Buffer[CFR_IE_SIZE_POS + 4];
        pst_DateTime->u8_Day           = pu8_Buffer[CFR_IE_SIZE_POS + 5];
        pst_DateTime->u8_Hours         = pu8_Buffer[CFR_IE_SIZE_POS + 6];
        pst_DateTime->u8_Mins          = pu8_Buffer[CFR_IE_SIZE_POS + 7];
        pst_DateTime->u8_Secs          = pu8_Buffer[CFR_IE_SIZE_POS + 8];
        pst_DateTime->u8_Zone          = pu8_Buffer[CFR_IE_SIZE_POS + 9];

        return CMBS_RC_OK;
    }

    return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_api_ie_DataGet( void * pv_RefIE, ST_IE_DATA * pst_Data )
{
    u8 * pu8_Buffer = (u8*)pv_RefIE;

    if ( *pu8_Buffer == CMBS_IE_DATA )
    {
        cfr_ie_dser_u16( pu8_Buffer + 3, &pst_Data->u16_DataLen );
        pst_Data->pu8_Data = pu8_Buffer + 5;   // IE_TYPE + u16_IELength + u16_DataLen

        return CMBS_RC_OK;
    }

    return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_api_ie_DataSessionIdGet( void * pv_RefIE, u16 * pu16_DataSessionId )
{
    return cmbs_int_ie_ShortValueGet( pv_RefIE, pu16_DataSessionId );
}


E_CMBS_RC         cmbs_api_ie_DataSessionTypeGet( void * pv_RefIE,
                                                  ST_IE_DATA_SESSION_TYPE * pst_DataSessionType )
{
    u8 * pu8_Buffer = (u8*)pv_RefIE;

    if ( *pu8_Buffer == CMBS_IE_DATA_SESSION_TYPE )
    {
        pst_DataSessionType->e_ChannelType = pu8_Buffer[CFR_IE_SIZE_POS + 1];
        pst_DataSessionType->e_ServiceType = pu8_Buffer[CFR_IE_SIZE_POS + 2];

        return CMBS_RC_OK;
    }

    return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_api_ie_LASessionIdGet( void * pv_RefIE, u16 * pu16_LASessionId )
{
    return cmbs_int_ie_ShortValueGet( pv_RefIE, pu16_LASessionId );
}


E_CMBS_RC         cmbs_api_ie_LAListIdGet( void * pv_RefIE, u16 * pu16_LAListId )
{
    return cmbs_int_ie_ShortValueGet( pv_RefIE, pu16_LAListId );
}


E_CMBS_RC         cmbs_api_ie_LAFieldsGet( void * pv_RefIE, ST_IE_LA_FIELDS * pst_LAFields )
{
    u8 *           pu8_Buffer = (u8*)pv_RefIE;
    int            i, nBuffOffs = CFR_IE_HEADER_SIZE;

    cfr_ie_dser_u16( pu8_Buffer + nBuffOffs, &pst_LAFields->u16_Length );
    nBuffOffs += 2;

    for ( i = 0; i < CMBS_LA_MAX_FLD_COUNT; i++ )
    {
        cfr_ie_dser_u16( pu8_Buffer + nBuffOffs, &pst_LAFields->pu16_FieldId[i] );
        nBuffOffs += 2;
    }

    return CMBS_RC_OK;
}

E_CMBS_RC         cmbs_api_ie_LASearchCriteriaGet( void * pv_RefIE, ST_IE_LA_SEARCH_CRITERIA * pst_LASearchCriteria )
{
    u8 * pu8_Buffer = (u8*)pv_RefIE;

    if ( *pu8_Buffer == CMBS_IE_LA_SEARCH_CRITERIA )
    {
        pst_LASearchCriteria->e_MatchingType    = pu8_Buffer[CFR_IE_HEADER_SIZE];
        pst_LASearchCriteria->u8_CaseSensitive  = pu8_Buffer[CFR_IE_HEADER_SIZE + 1];
        pst_LASearchCriteria->u8_Direction      = pu8_Buffer[CFR_IE_HEADER_SIZE + 2];
        pst_LASearchCriteria->u8_MarkEntriesReq = pu8_Buffer[CFR_IE_HEADER_SIZE + 3];
        pst_LASearchCriteria->u8_PatternLength  = pu8_Buffer[CFR_IE_HEADER_SIZE + 4];

        cfr_ie_dser_pu8( pu8_Buffer + (CFR_IE_HEADER_SIZE + 5), pst_LASearchCriteria->pu8_Pattern, (u16)pst_LASearchCriteria->u8_PatternLength );

        pst_LASearchCriteria->pu8_Pattern[pst_LASearchCriteria->u8_PatternLength] = 0;

        return CMBS_RC_OK;
    }

    return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_api_ie_LAEntryIdGet( void * pv_RefIE, u16 * pu16_LAEntryId )
{
    return cmbs_int_ie_ShortValueGet( pv_RefIE, pu16_LAEntryId );
}


E_CMBS_RC         cmbs_api_ie_LAEntryIndexGet( void * pv_RefIE, u16 * pu16_LAEntryIndex )
{
    return cmbs_int_ie_ShortValueGet( pv_RefIE, pu16_LAEntryIndex );
}


E_CMBS_RC         cmbs_api_ie_LAEntryCountGet( void * pv_RefIE, u16 * pu16_LAEntryCount )
{
    return cmbs_int_ie_ShortValueGet( pv_RefIE, pu16_LAEntryCount );
}


E_CMBS_RC         cmbs_api_ie_LAIsLastGet( void * pv_RefIE, u8 * pu8_LAIsLast )
{
    return cmbs_int_ie_ByteValueGet( pv_RefIE, pu8_LAIsLast );
}

E_CMBS_RC         cmbs_api_ie_HsNumberAdd( void * pv_RefIEList, u8 u8_HsNumber )
{
    u8  u8_Buffer[3];

    u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_HS_NUMBER;
    u8_Buffer[CFR_IE_SIZE_POS] = 1;  // sizeof(u8)

    u8_Buffer[CFR_IE_HEADER_SIZE] = u8_HsNumber;

    cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, 3 );

    return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_api_ie_HsNumberGet( void * pv_RefIE, u8 * pu8_HsNumber )
{
    u8 * pu8_Buffer = (u8*)pv_RefIE;

    if ( pu8_Buffer[CFR_IE_TYPE_POS] == CMBS_IE_HS_NUMBER )
    {
        *pu8_HsNumber = pu8_Buffer[CFR_IE_HEADER_SIZE];

        return CMBS_RC_OK;
    }

    return CMBS_RC_ERROR_PARAMETER;
}

E_CMBS_RC         cmbs_int_ie_GpioGet( void * pv_RefIE, u16 * pu16_Gpio )
{
    u8 *           pu8_Buffer = (u8*)pv_RefIE;

    if ( *pu8_Buffer == CMBS_IE_SHORT_VALUE )
    {
        cfr_ie_dser_u16( pu8_Buffer + CFR_IE_HEADER_SIZE, pu16_Gpio );

        return CMBS_RC_OK;
    }

    return CMBS_RC_ERROR_PARAMETER;
}

E_CMBS_RC         cmbs_int_ie_GpioAdd( void * pv_RefIEList, u16 u16_Gpio )
{
    u8             u8_Buffer[8];
    u16            u16_Size;

    u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_SHORT_VALUE;
    u8_Buffer[CFR_IE_SIZE_POS] = sizeof(u16);

    cfr_ie_ser_u16( u8_Buffer + CFR_IE_HEADER_SIZE, u16_Gpio );

    u16_Size = u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE;

    cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u16_Size );

    return CMBS_RC_OK;

}

E_CMBS_RC         cmbs_int_ie_ATESettingsGet( void * pv_RefIE, ST_IE_ATE_SETTINGS * pst_AteSettings )
{
    u8 *           pu8_Buffer = (u8*)pv_RefIE;

    if ( *pu8_Buffer == CMBS_IE_ATE_SETTINGS )
    {
        pst_AteSettings->e_ATESlotType      = pu8_Buffer[CFR_IE_HEADER_SIZE];
        pst_AteSettings->e_ATEType          = pu8_Buffer[CFR_IE_HEADER_SIZE + 1];
        pst_AteSettings->u8_Instance        = pu8_Buffer[CFR_IE_HEADER_SIZE + 2];
        pst_AteSettings->u8_Slot            = pu8_Buffer[CFR_IE_HEADER_SIZE + 3];
        pst_AteSettings->u8_Carrier         = pu8_Buffer[CFR_IE_HEADER_SIZE + 4];
        pst_AteSettings->u8_Ant             = pu8_Buffer[CFR_IE_HEADER_SIZE + 5];
        pst_AteSettings->u8_Pattern         = pu8_Buffer[CFR_IE_HEADER_SIZE + 6];
        pst_AteSettings->u8_NormalPreamble  = pu8_Buffer[CFR_IE_HEADER_SIZE + 7];
        pst_AteSettings->u8_PowerLevel      = pu8_Buffer[CFR_IE_HEADER_SIZE + 8];

        return CMBS_RC_OK;
    }

    return CMBS_RC_ERROR_PARAMETER;

}


E_CMBS_RC         cmbs_int_ie_ATESettingsAdd( void * pv_RefIEList, ST_IE_ATE_SETTINGS * pst_AteSettings )
{
    u8             u8_Buffer[16];
    u16            u16_Size;

    u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_ATE_SETTINGS;
    u8_Buffer[CFR_IE_SIZE_POS] = sizeof(u8)*9;

    u8_Buffer[CFR_IE_HEADER_SIZE]     = (u8)pst_AteSettings->e_ATESlotType;
    u8_Buffer[CFR_IE_HEADER_SIZE + 1] = (u8)pst_AteSettings->e_ATEType;
    u8_Buffer[CFR_IE_HEADER_SIZE + 2] = pst_AteSettings->u8_Instance;
    u8_Buffer[CFR_IE_HEADER_SIZE + 3] = pst_AteSettings->u8_Slot;
    u8_Buffer[CFR_IE_HEADER_SIZE + 4] = pst_AteSettings->u8_Carrier;
    u8_Buffer[CFR_IE_HEADER_SIZE + 5] = pst_AteSettings->u8_Ant;
    u8_Buffer[CFR_IE_HEADER_SIZE + 6] = pst_AteSettings->u8_Pattern;
    u8_Buffer[CFR_IE_HEADER_SIZE + 7] = pst_AteSettings->u8_NormalPreamble;
    u8_Buffer[CFR_IE_HEADER_SIZE + 8] = pst_AteSettings->u8_PowerLevel;      /* 0,1 or 2 */

    u16_Size = u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE;

    cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u16_Size );

    return CMBS_RC_OK;
}


E_CMBS_RC cmbs_api_ie_ReadDirectionAdd( void * pv_RefIEList, ST_IE_READ_DIRECTION * pst_ReadDirection )
{
    u8  u8_Buffer[4];
    u16 u16_Size;

    memset( u8_Buffer, 0, sizeof(u8_Buffer) );

    u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_LA_READ_DIRECTION;
    u8_Buffer[CFR_IE_SIZE_POS] = sizeof(u8);

    u8_Buffer[CFR_IE_HEADER_SIZE] = (u8)pst_ReadDirection->e_ReadDirection;

    u16_Size = u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE;

    cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u16_Size );

    return CMBS_RC_OK;
}


E_CMBS_RC cmbs_api_ie_ReadDirectionGet( void * pv_RefIE, ST_IE_READ_DIRECTION * pst_ReadDirection )
{
    u8 * pu8_Buffer = (u8*)pv_RefIE;

    if ( pu8_Buffer[CFR_IE_TYPE_POS] == CMBS_IE_LA_READ_DIRECTION )
    {
        pst_ReadDirection->e_ReadDirection = pu8_Buffer[CFR_IE_HEADER_SIZE];

        return CMBS_RC_OK;
    }

    return CMBS_RC_ERROR_PARAMETER;
}

E_CMBS_RC cmbs_api_ie_MarkRequestAdd( void * pv_RefIEList, ST_IE_MARK_REQUEST * pst_MarkRequest )
{
    u8  u8_Buffer[4];
    u16 u16_Size;

    memset( u8_Buffer, 0, sizeof(u8_Buffer) );

    u8_Buffer[CFR_IE_TYPE_POS] = (u8)CMBS_IE_LA_MARK_REQUEST;
    u8_Buffer[CFR_IE_SIZE_POS] = sizeof(u8);

    u8_Buffer[CFR_IE_HEADER_SIZE] = (u8)pst_MarkRequest->e_MarkRequest;

    u16_Size = u8_Buffer[CFR_IE_SIZE_POS] + CFR_IE_HEADER_SIZE;

    cfr_ie_ItemAdd( pv_RefIEList, u8_Buffer, u16_Size );

    return CMBS_RC_OK;
}


E_CMBS_RC cmbs_api_ie_MarkRequestGet( void * pv_RefIE, ST_IE_MARK_REQUEST * pst_MarkRequest )
{
    u8 * pu8_Buffer = (u8*)pv_RefIE;

    if ( pu8_Buffer[CFR_IE_TYPE_POS] == CMBS_IE_LA_MARK_REQUEST )
    {
        pst_MarkRequest->e_MarkRequest = pu8_Buffer[CFR_IE_HEADER_SIZE];

        return CMBS_RC_OK;
    }

    return CMBS_RC_ERROR_PARAMETER;
}


//*/
