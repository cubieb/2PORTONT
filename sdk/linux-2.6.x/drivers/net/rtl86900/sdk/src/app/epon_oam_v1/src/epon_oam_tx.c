/*
 * Copyright (C) 2012 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: 44884 $
 * $Date: 2013-12-02 19:31:05 +0800 (?±‰?, 02 ?Å‰???2013) $
 *
 * Purpose : 
 *
 * Feature : 
 *
 */

/*
 * Include Files
 */
#include <stdio.h>
#include <string.h> 
#include <malloc.h> 

#include "epon_oam_config.h"
#include "epon_oam_db.h"
#include "epon_oam_err.h"
#include "epon_oam_rx.h"
#include "epon_oam_tx.h"
#include "epon_oam_dbg.h"
#include "pkt_redirect_user.h"

/* 
 * Symbol Definition 
 */ 

/*  
 * Data Declaration  
 */
static unsigned short eventSeq[EPON_OAM_SUPPORT_LLID_NUM];

/* 
 * Macro Definition 
 */

/*  
 * Function Declaration  
 */
int epon_oam_stdInfo_gen(
    unsigned char llidIdx,
    unsigned char *pBuf,
    unsigned short bufLen,
    unsigned short *pPushedLen)
{
    oam_oamInfo_t localInfo, remoteInfo;

    memset(&localInfo, 0x0, sizeof(oam_oamInfo_t));
    memset(&remoteInfo, 0x0, sizeof(oam_oamInfo_t));
    *pPushedLen = 0;
    epon_oam_localInfo_get(llidIdx, &localInfo);
    if(localInfo.valid && (bufLen > EPON_INFO_OAMPDU_INFO_LEN))
    {
        /* Add local info oam */
        pBuf[0] = EPON_INFO_OAMPDU_TYPE_LOCAL;
        pBuf[1] = EPON_INFO_OAMPDU_INFO_LEN;
        pBuf[2] = localInfo.oamVer;
        pBuf[3] = ((unsigned char *)(&localInfo.revision))[0];
        pBuf[4] = ((unsigned char *)(&localInfo.revision))[1];
        pBuf[5] = localInfo.state;
        pBuf[6] = localInfo.oamConfig;
        pBuf[7] = ((unsigned char *)(&localInfo.oamPduConfig))[0];
        pBuf[8] = ((unsigned char *)(&localInfo.oamPduConfig))[1];
        pBuf[9] = localInfo.oui[0];
        pBuf[10] = localInfo.oui[1];
        pBuf[11] = localInfo.oui[2];
        pBuf[12] = localInfo.venderSpecInfo[0];
        pBuf[13] = localInfo.venderSpecInfo[1];
        pBuf[14] = localInfo.venderSpecInfo[2];
        pBuf[15] = localInfo.venderSpecInfo[3];

        pBuf += EPON_INFO_OAMPDU_INFO_LEN;
        *pPushedLen += EPON_INFO_OAMPDU_INFO_LEN;

        epon_oam_remoteInfo_get(llidIdx, &remoteInfo);
        if(remoteInfo.valid && (bufLen > (EPON_INFO_OAMPDU_INFO_LEN * 2)))
        {
            /* Add remote info oam */
            pBuf[0] = EPON_INFO_OAMPDU_TYPE_REMOTE;
            pBuf[1] = EPON_INFO_OAMPDU_INFO_LEN;
            pBuf[2] = remoteInfo.oamVer;
            pBuf[3] = ((unsigned char *)(&remoteInfo.revision))[0];
            pBuf[4] = ((unsigned char *)(&remoteInfo.revision))[1];
            pBuf[5] = remoteInfo.state;
            pBuf[6] = remoteInfo.oamConfig;
            pBuf[7] = ((unsigned char *)(&remoteInfo.oamPduConfig))[0];
            pBuf[8] = ((unsigned char *)(&remoteInfo.oamPduConfig))[1];
            pBuf[9] = remoteInfo.oui[0];
            pBuf[10] = remoteInfo.oui[1];
            pBuf[11] = remoteInfo.oui[2];
            pBuf[12] = remoteInfo.venderSpecInfo[0];
            pBuf[13] = remoteInfo.venderSpecInfo[1];
            pBuf[14] = remoteInfo.venderSpecInfo[2];
            pBuf[15] = remoteInfo.venderSpecInfo[3];
            pBuf += EPON_INFO_OAMPDU_INFO_LEN;
            *pPushedLen += EPON_INFO_OAMPDU_INFO_LEN;
        }
    }

    return EPON_OAM_ERR_OK;
}

void epon_oam_varResp_gen(
    unsigned char varType,
    oam_varContainer_t *pVarContainer,
    unsigned char *pFrameBuf)
{
    unsigned short varWidth; 
    oam_varContainerResp_t *pVarResp;

    /* Already check the size at caller side */

    /* Variable container's branch/leaf */
    *pFrameBuf = pVarContainer->varDesc.varBranch;
    pFrameBuf++;
    *pFrameBuf = ((unsigned char *)(&pVarContainer->varDesc.varLeaf))[0];
    *pFrameBuf = ((unsigned char *)(&pVarContainer->varDesc.varLeaf))[1];
    pFrameBuf += 2;

    pVarResp = pVarContainer->pVarResp;
    while(NULL != pVarResp)
    {
        *pFrameBuf = pVarResp->varWidth;
        pFrameBuf ++;
        varWidth = EPON_VAR_OAMPDU_WIDTH(pVarResp->varWidth);
        if(0 != varWidth)
        {
            memcpy(pFrameBuf, pVarResp->varData, varWidth);
            pFrameBuf += varWidth;
        }
        pVarResp = pVarResp->next;
    }

    /* End indication */
    switch(varType)
    {
    case EPON_OAM_VARTYPE_OBJECT:
        *pFrameBuf = EPON_VAR_OAMPDU_INDICATOR(EPON_VAR_OAMPDU_IND_OBJEND);
        pFrameBuf ++;
        break;
    case EPON_OAM_VARTYPE_PACKAGE:
        *pFrameBuf = EPON_VAR_OAMPDU_INDICATOR(EPON_VAR_OAMPDU_IND_PKGEND);
        pFrameBuf ++;
        break;
    case EPON_OAM_VARTYPE_ATTRIBUTE:
    case EPON_OAM_VARTYPE_OBJECTID:
    default:
        break;
    }
}

void epon_oam_varRespErrorLen_gen(
    oam_varContainer_t *pVarContainer,
    unsigned char *pFrameBuf)
{
    unsigned short varWidth; 
    oam_varContainerResp_t *pVarResp;

    /* Already check the size at caller side */

    /* Variable container's branch/leaf */
    *pFrameBuf = pVarContainer->varDesc.varBranch;
    pFrameBuf++;
    *pFrameBuf = ((unsigned char *)(&pVarContainer->varDesc.varLeaf))[0];
    *pFrameBuf = ((unsigned char *)(&pVarContainer->varDesc.varLeaf))[1];
    pFrameBuf += 2;

    *pFrameBuf = EPON_VAR_OAMPDU_INDICATOR(EPON_VAR_OAMPDU_IND_LENGTH);
    pFrameBuf++;
}

void epon_oam_stdHdr_gen(
    unsigned short llidIdx,
    unsigned char code,
    unsigned char *pFrameBuf)
{
    const unsigned char oamDstMac[6] = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x02};
    unsigned short oamPduFlag;
    oam_config_t oamConfig;

    /* Already check the size at caller side */

    memcpy(pFrameBuf, oamDstMac, sizeof(oamDstMac));
    pFrameBuf += sizeof(oamDstMac);
    epon_oam_config_get(llidIdx, &oamConfig);
    memcpy(pFrameBuf, oamConfig.macAddr, sizeof(oamConfig.macAddr));
    pFrameBuf += sizeof(oamConfig.macAddr);
    /* EtherType */
    *pFrameBuf = 0x88;
    pFrameBuf ++;
    *pFrameBuf = 0x09;
    pFrameBuf ++;
    /* SubType */
    *pFrameBuf = 0x03;
    pFrameBuf ++;
    /* Flag */
    oamPduFlag = epon_oam_oamPduFlag_get(llidIdx);
    *pFrameBuf = ((unsigned char *)(&oamPduFlag))[0];
    pFrameBuf ++;
    *pFrameBuf = ((unsigned char *)(&oamPduFlag))[1];
    pFrameBuf ++;
    /* Code */
    *pFrameBuf = code;
}

int epon_oam_dyingGasp_gen(
    unsigned char llidIdx,
    unsigned char *pBuf,
    unsigned short bufLen,
    unsigned short *pPushedLen)
{
    const unsigned char oamDstMac[6] = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x02};
    unsigned short oamPduFlag;
    oam_config_t oamConfig;
    unsigned char *pPtr;
    unsigned short remainLen, pusdhedLen;

    pPtr = pBuf;
    pusdhedLen = 0;
    remainLen = bufLen - 1; /* Reserved for end token */

    if(bufLen >= EPON_OAMPDU_HDR_LENGTH) 
    {
        /* Add standard header */
        memcpy(pPtr, oamDstMac, sizeof(oamDstMac));
        pPtr += sizeof(oamDstMac);
        epon_oam_config_get(llidIdx, &oamConfig);
        memcpy(pPtr, oamConfig.macAddr, sizeof(oamConfig.macAddr));
        pPtr += sizeof(oamConfig.macAddr);
        /* EtherType */
        *pPtr = 0x88;
        pPtr ++;
        *pPtr = 0x09;
        pPtr ++;
        /* SubType */
        *pPtr = 0x03;
        pPtr ++;
        /* Flag */
        oamPduFlag = epon_oam_oamPduFlag_get(llidIdx);
        /* Add dying gasp bit */
        oamPduFlag |= EPON_OAM_FLAG_DYING_GASP;
        *pPtr = ((unsigned char *)(&oamPduFlag))[0];
        pPtr ++;
        *pPtr = ((unsigned char *)(&oamPduFlag))[1];
        pPtr ++;
        /* Code */
        *pPtr = EPON_OAMPDU_CODE_INFO;
        pPtr ++;
        remainLen -= EPON_OAMPDU_HDR_LENGTH;

        epon_oam_stdInfo_gen(llidIdx, pPtr, remainLen, &pusdhedLen);
        pPtr += pusdhedLen;
        remainLen -= pusdhedLen;
    }
    *pPtr = 0x00;
    pPtr ++;
    remainLen --;

    *pPushedLen = pPtr - pBuf;

    return EPON_OAM_ERR_OK;
}

/* Function Name:
 *      epon_oam_orgSpecEvtNotification_send
 * Description:
 *      Send an organization specific event
 * Input:
 *      llidIdx - LLID index to be sent
 *      pEvent  - organization specific event to be sent
 * Output:
 *      None
 * Return:
 *      EPON_OAM_ERR_OK
 *      EPON_OAM_ERR_MEM
 *      EPON_OAM_ERR_PARAM
 * Note:
 *      None
 */
int epon_oam_orgSpecEvtNotification_send(
    unsigned short llidIdx,
    oam_eventOrgSpec_t *pEvent)
{
    int ret;
    unsigned short totalLen, eventLen;
    unsigned char *pReplyPtr, *pCurr;
    oam_eventOrgSpec_t *pCurrEvent;

    /* Parameter check */
    if((llidIdx >= EPON_OAM_SUPPORT_LLID_NUM) || (NULL == pEvent))
    {
        return EPON_OAM_ERR_PARAM;
    }

    /* Calculate the total length
     * The total length will be the sum of following items
     * 1. EPON_OAMPDU_HDR_LENGTH
     * 2. Sequence number(2)
     * 3. Total length of all organization specific events
     */
    totalLen = EPON_OAMPDU_HDR_LENGTH + EPON_EVENT_OAMPDU_SEQUENCE_LEN + EPON_TX_OAM_LLID_LEN;
    pCurrEvent = pEvent;
    while(NULL != pCurrEvent)
    {
        totalLen += (pCurrEvent->eventLen + EPON_EVENT_OAMPDU_ORGSPEC_MIN_LEN);
        pCurrEvent = pCurrEvent->pNext;
    };

    pReplyPtr = (unsigned char *) malloc(sizeof(unsigned char) * totalLen);
    if(NULL == pReplyPtr)
    {
        return EPON_OAM_ERR_MEM;
    }

    /* Fill in standard header */
    epon_oam_stdHdr_gen(llidIdx, EPON_OAMPDU_CODE_EVENT, pReplyPtr);
    pCurr = pReplyPtr + EPON_OAMPDU_HDR_LENGTH;
    /* Fill in event header */
    /* Sequence Number */
    eventSeq[llidIdx]++;
    *pCurr = ((unsigned char *)(&eventSeq[llidIdx]))[0];
    pCurr ++;
    *pCurr = ((unsigned char *)(&eventSeq[llidIdx]))[1];
    pCurr ++;
    /* Event Payload */
    pCurrEvent = pEvent;
    while(NULL != pCurrEvent)
    {
        /* Event Type */
        *pCurr = EPON_EVENT_OAMPDU_TYPE_ORG_SPEC;
        pCurr ++;
        /* Event Length */
        eventLen = pCurrEvent->eventLen + EPON_EVENT_OAMPDU_ORGSPEC_MIN_LEN;
        *pCurr = ((unsigned char *)(&eventLen))[0];
        pCurr ++;
        *pCurr = ((unsigned char *)(&eventLen))[1];
        pCurr ++;
        /* OUI */
        memcpy(pCurr, pCurrEvent->oui, EPON_OAM_OUI_LENGTH);
        pCurr += EPON_OAM_OUI_LENGTH;
        /* Organization specific value */
        memcpy(pCurr, pCurrEvent->eventData, pCurrEvent->eventLen);
        pCurr += pCurrEvent->eventLen;
        pCurrEvent = pCurrEvent->pNext;
    };

    /* Fill in the LLID index at the end of all payload */
    *pCurr = ((unsigned char) llidIdx);
    pCurr += EPON_TX_OAM_LLID_LEN;

    ret= ptk_redirect_userApp_sendPkt(
        pktRedirect_sock,
        PR_KERNEL_UID_GMAC,
        0,
        pCurr - pReplyPtr,
        pReplyPtr);
    EPON_OAM_PRINT(EPON_OAM_DBGFLAG_TXOAM,
        "[OAM:%s:%d] OAMPDU Tx (%d)\n", __FILE__, __LINE__, ret);
    DUMP_HEX_VALUE(EPON_OAM_DBGFLAG_TXOAM, pReplyPtr, pCurr - pReplyPtr - EPON_TX_OAM_LLID_LEN);
    EPON_OAM_PRINT(EPON_OAM_DBGFLAG_TXOAM, "\n");

    epon_oam_counter_inc(llidIdx, EPON_OAM_COUNTERTYPE_TX);

    /* MODIFY ME - implement the event resent mechanism */

    free(pReplyPtr);
    
    return EPON_OAM_ERR_OK;
}


/* Function Name:
 *      epon_oam_reply_send
 * Description:
 *      Send an OAMPDU
 * Input:
 *      llidIdx - LLID index to be sent
 *      code    - OAMPDU code field
 *      pReplyPayload - OAMPDU payload to be sent
 *      replyLen - OAMPDU payload length
 * Output:
 *      None
 * Return:
 *      EPON_OAM_ERR_OK
 *      EPON_OAM_ERR_MEM
 *      EPON_OAM_ERR_PARAM
 * Note:
 *      None
 */
int epon_oam_reply_send(
    unsigned short llidIdx,
    unsigned char code,
    unsigned char *pReplyPayload,
    unsigned short replyLen)
{
    int ret;
    unsigned char *pReplyPtr, *pCurr;
    unsigned int  oam_dbg_flag;

    /* Parameter check */
    if(llidIdx >= EPON_OAM_SUPPORT_LLID_NUM)
    {
        return EPON_OAM_ERR_PARAM;
    }
    if(NULL == pReplyPayload)
    {
        return EPON_OAM_ERR_PARAM;
    }

    pReplyPtr = 
        (unsigned char *) malloc(sizeof(unsigned char) * replyLen + EPON_OAMPDU_HDR_LENGTH + EPON_TX_OAM_LLID_LEN);

    if(NULL == pReplyPtr)
    {
        return EPON_OAM_ERR_MEM;
    }

    /* Fill in standard header */
    epon_oam_stdHdr_gen(llidIdx, code, pReplyPtr);
    pCurr = pReplyPtr + EPON_OAMPDU_HDR_LENGTH;
    /* Payload */
    memcpy(pCurr, pReplyPayload, replyLen);
    pCurr += replyLen;

    /* Fill in the LLID index at the end of all payload */
    *pCurr = ((unsigned char) llidIdx);
    pCurr += EPON_TX_OAM_LLID_LEN;

    ret= ptk_redirect_userApp_sendPkt(
        pktRedirect_sock,
        PR_KERNEL_UID_GMAC,
        0,
        pCurr - pReplyPtr,
        pReplyPtr);

    epon_oam_dbgFlag_get(&oam_dbg_flag);

    if ((oam_dbg_flag & EPON_OAM_DBGFLAG_ORGTX) && (pReplyPtr[17] == 0xfe))
    {
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ORGTX,
            "[OAM:%s:%d] OAMPDU Tx\n", __FILE__, __LINE__);
        DUMP_HEX_VALUE(EPON_OAM_DBGFLAG_ORGTX, pReplyPtr, pCurr - pReplyPtr - EPON_TX_OAM_LLID_LEN);
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ORGTX, "\n");
    }
    else
    {
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_TXOAM,
            "[OAM:%s:%d] OAMPDU Tx\n", __FILE__, __LINE__);
        DUMP_HEX_VALUE(EPON_OAM_DBGFLAG_TXOAM, pReplyPtr, pCurr - pReplyPtr - EPON_TX_OAM_LLID_LEN);
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_TXOAM, "\n");
    }

    epon_oam_counter_inc(llidIdx, EPON_OAM_COUNTERTYPE_TX);

    free(pReplyPtr);
    
    return EPON_OAM_ERR_OK;
}

/* Function Name:
 *      epon_oam_dyingGasp_send
 * Description:
 *      Send an OAMPDU
 * Input:
 *      llidIdx - LLID index to be sent
 *      pPayload - OAMPDU payload to be sent
 *      len - OAMPDU payload length
 * Output:
 *      None
 * Return:
 *      EPON_OAM_ERR_OK
 *      EPON_OAM_ERR_MEM
 *      EPON_OAM_ERR_PARAM
 * Note:
 *      None
 */
int epon_oam_dyingGasp_send(
    unsigned short llidIdx,
    unsigned char *pPayload,
    unsigned short len)
{
    int ret;
    unsigned char *pPtr, *pCurr;

    /* Parameter check */
    if(llidIdx >= EPON_OAM_SUPPORT_LLID_NUM)
    {
        return EPON_OAM_ERR_PARAM;
    }
    if(NULL == pPayload)
    {
        return EPON_OAM_ERR_PARAM;
    }

    len = (len >= 60 ? len : 60);
    pPtr = (unsigned char *) malloc(sizeof(unsigned char) * (len + EPON_TX_OAM_LLID_LEN));

    if(NULL == pPtr)
    {
        return EPON_OAM_ERR_MEM;
    }

    pCurr = pPtr;
	
    /* Payload */
    memcpy(pCurr, pPayload, len);
    pCurr += len;

    /* Fill in the LLID index at the end of all payload */
    *pCurr = ((unsigned char) llidIdx);
    pCurr += EPON_TX_OAM_LLID_LEN;

    ret= ptk_redirect_userApp_sendPkt(
        pktRedirect_sock,
        PR_KERNEL_UID_EPONDYINGGASP,
        0,
        pCurr - pPtr,
        pPtr);
    EPON_OAM_PRINT(EPON_OAM_DBGFLAG_TXOAM,
        "[OAM:%s:%d] OAMPDU Tx Dying Gasp (%d)\n", __FILE__, __LINE__, ret);
    DUMP_HEX_VALUE(EPON_OAM_DBGFLAG_TXOAM, pPtr, pCurr - pPtr - EPON_TX_OAM_LLID_LEN);
    EPON_OAM_PRINT(EPON_OAM_DBGFLAG_TXOAM, "\n");

    epon_oam_counter_inc(llidIdx, EPON_OAM_COUNTERTYPE_TX);

    free(pPtr);
    
    return EPON_OAM_ERR_OK;
}

