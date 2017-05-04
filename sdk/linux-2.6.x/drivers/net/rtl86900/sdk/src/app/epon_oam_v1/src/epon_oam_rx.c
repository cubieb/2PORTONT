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
 * $Revision: 44598 $
 * $Date: 2013-11-20 18:17:00 +0800 (?±‰?, 20 ?Å‰???2013) $
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
#include <semaphore.h>
#include <string.h> 
#include <malloc.h> 
#include <sys/socket.h> 
#include <linux/netlink.h> 
#include <errno.h>

#include <rtk/epon.h>

#include "epon_oam_config.h"
#include "epon_oam_rx.h"
#include "epon_oam_db.h"
#include "epon_oam_err.h"
#include "epon_oam_dbg.h"
#include "epon_oam_msgq.h"
#include "pkt_redirect_user.h"

/* 
 * Symbol Definition 
 */ 
#define MAX_PAYLOAD (1600)

/*  
 * Data Declaration  
 */
int pktRedirect_sock;

/* 
 * Macro Definition 
 */

/*  
 * Function Declaration  
 */
static int epon_oam_maxOamPduSize_get(
    unsigned char llidIdx,
    unsigned short *pMaxSize)
{
    unsigned short localMax, remoteMax;
    oam_oamInfo_t localInfo, remoteInfo;

    memset(&localInfo, 0x0, sizeof(oam_oamInfo_t));
    memset(&remoteInfo, 0x0, sizeof(oam_oamInfo_t));

    /* The maximum OAMPDU size is decided by both local and remote info OAMPDU */
    epon_oam_localInfo_get(llidIdx, &localInfo);
    epon_oam_remoteInfo_get(llidIdx, &remoteInfo);

    if(localInfo.valid && remoteInfo.valid)
    {
        localMax = (localInfo.oamPduConfig & 0x7ff);
        remoteMax = (remoteInfo.oamPduConfig & 0x7ff);
       
        *pMaxSize = (localMax > remoteMax) ? remoteMax : localMax;
    }
    else if(localInfo.valid)
    {
        localMax = (localInfo.oamPduConfig & 0x7ff);
        *pMaxSize = localMax;
    }
    else
    {
        return EPON_OAM_ERR_UNKNOWN;
    }

    /* Reserve header length */
    *pMaxSize -= EPON_OAMPDU_HDR_LENGTH;

    return EPON_OAM_ERR_OK;
}

static unsigned short epon_oam_varRespLen_calc(
    unsigned char varType,
    oam_varContainer_t *pVarContainer)
{
    unsigned int retLen;
    oam_varContainerResp_t *pVarResp;

    /* Branch/leaf */
    retLen = EPON_VAR_OAMPDU_DESC_LEN;

    pVarResp = pVarContainer->pVarResp;
    while(NULL != pVarResp)
    {
        /* Length/indication */
        retLen += 1;
        /* Width */
        retLen += EPON_VAR_OAMPDU_WIDTH(pVarResp->varWidth);

        pVarResp = pVarResp->next;
    }

    /* End indication */
    switch(varType)
    {
    case EPON_OAM_VARTYPE_OBJECT:
        retLen += EPON_VAR_OAMPDU_OBJEND_LEN;
        break;
    case EPON_OAM_VARTYPE_PACKAGE:
        retLen += EPON_VAR_OAMPDU_PKGEND_LEN;
        break;
    case EPON_OAM_VARTYPE_ATTRIBUTE:
    case EPON_OAM_VARTYPE_OBJECTID:
    default:
        break;
    }

    /* In case of over flow, reply the maximum value */
    return (retLen < 0xffff) ? retLen : 0xffff;
}

static void epon_oam_varResp_free(
    oam_varContainer_t **ppVarContainer)
{
    oam_varContainerResp_t *pVarResp, *pVarNext;

    if(NULL != *ppVarContainer)
    {

        pVarResp = (*ppVarContainer)->pVarResp;
        while(NULL != pVarResp)
        {
            if(NULL != pVarResp->varData)
            {
                free(pVarResp->varData);
            }
            pVarNext = pVarResp->next;
            free(pVarResp);
            pVarResp = pVarNext;
        }
        free(*ppVarContainer);
        *ppVarContainer = NULL;
    }
}

static int epon_oam_stdHdr_parser(
    oam_oamPdu_t *pOamPdu,
    unsigned char *pFrame,
    unsigned short length,
    unsigned short *pExtractLen)
{
    unsigned char *pParsePtr;
    
    pParsePtr = pFrame + 6; /* Skip dst MAC */
    memcpy(pOamPdu->srcMacAddr, pParsePtr, 6);
    pParsePtr += (6 + 2 + 1); /* Src MAC + ether type + sub-type */
    ((unsigned char *)(&pOamPdu->flag))[0] = *pParsePtr;
    pParsePtr ++; /* Flag */
    ((unsigned char *)(&pOamPdu->flag))[1] = *pParsePtr;
    pParsePtr ++; /* Flag */
    pOamPdu->code = *pParsePtr;
    pParsePtr += 1; /* Code */
    pOamPdu->pData = pParsePtr;
    pOamPdu->dataLen = length - (pParsePtr - pFrame);
    *pExtractLen = (pParsePtr - pFrame);

    return EPON_OAM_ERR_OK;
}

static int epon_oam_stdInfo_parser(
    oam_oamInfo_t *pOamInfo,
    unsigned char *pFrame,
    unsigned short length,
    unsigned short *pExtractLen)
{
    unsigned char *pParsePtr;

    /* Minimum length requirement check */
    if(EPON_INFO_OAMPDU_INFO_LEN > length)
    {
        /* Remain length is not enough for parsing
         * Skip the TLV
         */
        *pExtractLen = length;
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
            "[OAM:%s:%d] could not parse info OAMPDU\n", __FILE__, __LINE__);
        return EPON_OAM_ERR_PARSE;
    }

    pParsePtr = pFrame + 1; /* Skip information type */
    if(EPON_INFO_OAMPDU_INFO_LEN != *pParsePtr)
    {
        /* Wrong length for local/remote info OAMPDU
         * Skip the TLV
         */
        *pExtractLen = length;
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
            "[OAM:%s:%d] info OAMPDU length incorrect\n", __FILE__, __LINE__);
        return EPON_OAM_ERR_PARSE;
    }

    pParsePtr += 1;
    pOamInfo->oamVer = *pParsePtr;
    pParsePtr += 1;
    ((unsigned char *)(&pOamInfo->revision))[0] = *pParsePtr;
    pParsePtr += 1;
    ((unsigned char *)(&pOamInfo->revision))[1] = *pParsePtr;
    pParsePtr += 1;
    pOamInfo->state = *pParsePtr;
    pParsePtr += 1;
    pOamInfo->oamConfig = *pParsePtr;
    pParsePtr += 1;
    ((unsigned char *)(&pOamInfo->oamPduConfig))[0] = *pParsePtr;
    pParsePtr += 1;
    ((unsigned char *)(&pOamInfo->oamPduConfig))[1] = *pParsePtr;
    pParsePtr += 1;
    memcpy(pOamInfo->oui, pParsePtr, EPON_OAM_OUI_LENGTH);
    pParsePtr += EPON_OAM_OUI_LENGTH;
    memcpy(pOamInfo->venderSpecInfo, pParsePtr, EPON_INFO_OAMPDU_VENDER_SPEC_LEN);
    pParsePtr += EPON_INFO_OAMPDU_VENDER_SPEC_LEN;
    *pExtractLen = (pParsePtr - pFrame);

    return EPON_OAM_ERR_OK;
}

static int epon_oam_orgSpecInfo_parser(
    unsigned char llidIdx,
    unsigned char *pFrame,
    unsigned short length,
    unsigned short *pExtractLen,
    unsigned char oui[],
    void **ppOrgSpecData)
{
    int ret;
    unsigned char *pParsePtr;
    unsigned short cbExtractLen, orgSpecLen;
    oam_infoOrgSpecCb_t orgSpecCb;

    /* 1. Parsing the length and OUI first
     * 2. Search for callback function via OUI
     * 3. Parse the OAMPDU via user registed or default callback function
     */

    /* Minimum length requirement check */
    if( EPON_INFO_OAMPDU_ORGSPEC_MIN_LEN > length)
    {
        /* Remain length is not enough for parsing
         * Skip the TLV
         */
        *pExtractLen = length;
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
            "[OAM:%s:%d] could not parse info OAMPDU\n", __FILE__, __LINE__);
        return EPON_OAM_ERR_PARSE;
    }

    pParsePtr = pFrame + 1; /* Skip information type */
    orgSpecLen = *pParsePtr;
    if(length < orgSpecLen)
    {
        /* Remain length is not enough for parsing
         * Skip the TLV
         */
        *pExtractLen = length;
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
            "[OAM:%s:%d] OAMPDU length incorrect\n", __FILE__, __LINE__);
        return EPON_OAM_ERR_PARSE;
    }

    pParsePtr += 1; /* length */
    memcpy(oui, pParsePtr, EPON_OAM_OUI_LENGTH);
    pParsePtr += 3; /* OUI */
    orgSpecCb.parser = NULL;
    orgSpecCb.handler = NULL;
    ret = epon_oam_orgSpecCb_get(EPON_OAM_CBTYPE_INFO_ORGSPEC, oui, (void *) &orgSpecCb);
    if(EPON_OAM_ERR_OK != ret)
    {
        /* No such OUI found
         * Get the default callback function
         */
       epon_oam_defOrgSpecCb_get(EPON_OAM_CBTYPE_INFO_ORGSPEC, (void *) &orgSpecCb);
    }

    cbExtractLen = 0;
    if(NULL != orgSpecCb.parser)
    {
        ret = orgSpecCb.parser(
            llidIdx,
            pFrame,
            length,
            &cbExtractLen,
            ppOrgSpecData);
    }

    if((EPON_OAM_ERR_OK != ret) || (NULL == orgSpecCb.parser) || (orgSpecLen != cbExtractLen))
    {
        /* Either 
         * 1. can't find the praser callback or
         * 2. parsring failed
         * Nothing can be done in both cases
         * Just ignore all the following payload
         */
        *pExtractLen = length;
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
            "[OAM:%s:%d] can't parse info organization specific OAM\n", __FILE__, __LINE__);
        return EPON_OAM_ERR_PARSE;
    }
    *pExtractLen = cbExtractLen;

    return EPON_OAM_ERR_OK;
}

static int epon_oam_orgSpecInfo_handler(
    oam_oamPdu_t *pOamPdu,
    unsigned char *pReplyBuf,
    unsigned short bufLen,
    unsigned short *pReplyLen,
    unsigned char oui[],
    void **ppOrgSpecData)
{
    int ret;
    oam_infoOrgSpecCb_t orgSpecCb;

    /* 1. Search for callback function via OUI
     * 2. Handle the OAMPDU via user registed or default callback function
     */

    orgSpecCb.parser = NULL;
    orgSpecCb.handler = NULL;
    ret = epon_oam_orgSpecCb_get(EPON_OAM_CBTYPE_INFO_ORGSPEC, oui, (void *) &orgSpecCb);
    if(EPON_OAM_ERR_OK != ret)
    {
        /* No such OUI found
         * Get the default callback function
         */
       epon_oam_defOrgSpecCb_get(EPON_OAM_CBTYPE_INFO_ORGSPEC, (void *) &orgSpecCb);
    }

    if(NULL != orgSpecCb.handler)
    {
        ret = orgSpecCb.handler(
            pOamPdu,
            pReplyBuf,
            bufLen,
            pReplyLen,
            ppOrgSpecData);
    }

    if((EPON_OAM_ERR_OK != ret) || (NULL == orgSpecCb.handler) || (bufLen < *pReplyLen))
    {
        /* Either 
         * 1. can't find the handler callback or
         * 2. handling failed
         * 3. wrong reply length
         * Nothing can be done in both cases
         * Just ignore all the TLV
         */
        *pReplyLen = 0;;
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
            "[OAM:%s:%d] can't handle info organization specific OAM\n", __FILE__, __LINE__);
        return EPON_OAM_ERR_HANDLE;
    }

    return EPON_OAM_ERR_OK;
}

static int epon_oam_stdEvent_parser(
    unsigned char eventType,
    unsigned char *pFrame,
    unsigned short length,
    unsigned short *pExtractLen)
{
    unsigned char minLen;
    unsigned char *pParsePtr;

    /* Minimum length requirement check */
    switch(eventType)
    {
    case EPON_EVENT_OAMPDU_TYPE_ESPE:
        minLen = EPON_EVENT_OAMPDU_ESPE_LEN;
        break;
    case EPON_EVENT_OAMPDU_TYPE_EFE:
        minLen = EPON_EVENT_OAMPDU_EFE_LEN;
        break;
    case EPON_EVENT_OAMPDU_TYPE_EFPE:
        minLen = EPON_EVENT_OAMPDU_EFPE_LEN;
        break;
    case EPON_EVENT_OAMPDU_TYPE_EFSSE:
        minLen = EPON_EVENT_OAMPDU_EFPE_LEN;
        break;
    default:
        return EPON_OAM_ERR_PARAM;
    }

    if(minLen > length)
    {
        /* Remain length is not enough for parsing
         * Skip the TLV
         */
        *pExtractLen = length;
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
            "[OAM:%s:%d] could not parse event OAMPDU\n", __FILE__, __LINE__);
        return EPON_OAM_ERR_PARSE;
    }

    pParsePtr = pFrame + 1; /* Skip event type */
    if(minLen != *pParsePtr)
    {
        /* Wrong length for event OAMPDU
         * Skip the TLV
         */
        *pExtractLen = length;
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
            "[OAM:%s:%d] event OAMPDU length incorrect\n", __FILE__, __LINE__);
        return EPON_OAM_ERR_PARSE;
    }

    /* MODIFY ME - add event OAMPDU parse */
    *pExtractLen = minLen;

    return EPON_OAM_ERR_OK;
}

static int epon_oam_orgSpecEvent_parser(
    unsigned char llidIdx,
    unsigned char *pFrame,
    unsigned short length,
    unsigned short *pExtractLen,
    unsigned char oui[],
    void **ppOrgSpecData)
{
    int ret;
    unsigned char *pParsePtr;
    unsigned short cbExtractLen, orgSpecLen;
    oam_eventOrgSpecCb_t orgSpecCb;

    /* 1. Parsing the length and OUI first
     * 2. Search for callback function via OUI
     * 3. Parse the OAMPDU via user registed or default callback function
     */

    /* Minimum length requirement check */
    if( EPON_EVENT_OAMPDU_ORGSPEC_MIN_LEN > length)
    {
        /* Remain length is not enough for parsing
         * Skip the TLV
         */
        *pExtractLen = length;
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
            "[OAM:%s:%d] could not parse event OAMPDU\n", __FILE__, __LINE__);
        return EPON_OAM_ERR_PARSE;
    }

    pParsePtr = pFrame + 1; /* Skip event type */
    orgSpecLen = *pParsePtr;
    if(length < orgSpecLen)
    {
        /* Remain length is not enough for parsing
         * Skip the TLV
         */
        *pExtractLen = length;
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
            "[OAM:%s:%d] OAMPDU length incorrect\n", __FILE__, __LINE__);
        return EPON_OAM_ERR_PARSE;
    }

    pParsePtr += 1; /* length */
    memcpy(oui, pParsePtr, EPON_OAM_OUI_LENGTH);
    pParsePtr += 3; /* OUI */
    orgSpecCb.parser = NULL;
    orgSpecCb.handler = NULL;
    ret = epon_oam_orgSpecCb_get(EPON_OAM_CBTYPE_EVENT_ORGSPEC, oui, (void *) &orgSpecCb);
    if(EPON_OAM_ERR_OK != ret)
    {
        /* No such OUI found
         * Get the default callback function
         */
       epon_oam_defOrgSpecCb_get(EPON_OAM_CBTYPE_EVENT_ORGSPEC, (void *) &orgSpecCb);
    }

    cbExtractLen = 0;
    if(NULL != orgSpecCb.parser)
    {
        ret = orgSpecCb.parser(
            llidIdx,
            pFrame,
            length,
            &cbExtractLen,
            ppOrgSpecData);
    }

    if((EPON_OAM_ERR_OK != ret) || (NULL == orgSpecCb.parser) || (orgSpecLen != cbExtractLen))
    {
        /* Either 
         * 1. can't find the praser callback or
         * 2. parsring failed
         * Nothing can be done in both cases
         * Just ignore all the following payload
         */
        *pExtractLen = length;
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
            "[OAM:%s:%d] can't parse event organization specific OAM\n", __FILE__, __LINE__);
        return EPON_OAM_ERR_PARSE;
    }
    *pExtractLen = cbExtractLen;

    return EPON_OAM_ERR_OK;
}

static int epon_oam_orgSpecEvent_handler(
    oam_oamPdu_t *pOamPdu,
    unsigned char *pReplyBuf,
    unsigned short bufLen,
    unsigned short *pReplyLen,
    unsigned char oui[],
    void **ppOrgSpecData)
{
    int ret;
    oam_eventOrgSpecCb_t orgSpecCb;

    /* 1. Search for callback function via OUI
     * 2. Handle the OAMPDU via user registed or default callback function
     */

    orgSpecCb.parser = NULL;
    orgSpecCb.handler = NULL;
    ret = epon_oam_orgSpecCb_get(EPON_OAM_CBTYPE_EVENT_ORGSPEC, oui, (void *) &orgSpecCb);
    if(EPON_OAM_ERR_OK != ret)
    {
        /* No such OUI found
         * Get the default callback function
         */
       epon_oam_defOrgSpecCb_get(EPON_OAM_CBTYPE_EVENT_ORGSPEC, (void *) &orgSpecCb);
    }

    if(NULL != orgSpecCb.handler)
    {
        ret = orgSpecCb.handler(
            pOamPdu,
            pReplyBuf,
            bufLen,
            pReplyLen,
            ppOrgSpecData);
    }

    if((EPON_OAM_ERR_OK != ret) || (NULL == orgSpecCb.handler) || (bufLen < *pReplyLen))
    {
        /* Either 
         * 1. can't find the handler callback or
         * 2. handling failed
         * 3. wrong reply length
         * Nothing can be done in both cases
         * Just ignore all the TLV
         */
        *pReplyLen = 0;;
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
            "[OAM:%s:%d] can't handle event organization specific OAM\n", __FILE__, __LINE__);
        return EPON_OAM_ERR_HANDLE;
    }

    return EPON_OAM_ERR_OK;
}

static int epon_oam_stdInfo_process(
    oam_oamPdu_t *pOamPdu,
    unsigned char **ppReplyOamPdu,
    unsigned short *pReplyLen)
{
    int ret;
    void *pOrgSpecData;
    unsigned char oui[3];
    unsigned char isEnd;
    unsigned char *pProcPtr, *pReplyPtr;
    unsigned short remainLen, extractLen;
    unsigned short bufLen, replyLen;
    oam_oamInfo_t oamInfo, localOamInfo, lastRInfo, lastLInfo, currLInfo;

    /* 1. Check if the info OAMPDU TLV should be processed (see local info TLV)?
     * 2. Parse standard/organization specific/unknown info OAM fields
     * 3. Handle standard/organization specific/unknown info OAM fields
     * 4. Transmit the reply OAMPDU
     */
    isEnd = 0;
    pProcPtr = pOamPdu->pData;
    remainLen = pOamPdu->dataLen;

    ret = epon_oam_maxOamPduSize_get(pOamPdu->llidIdx, &bufLen);
    if(EPON_OAM_ERR_OK != ret)
    {
        /* Could not decide the OAMPDU size */
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
            "[OAM:%s:%d] can't get OAMPDU max size\n", __FILE__, __LINE__);
        return EPON_OAM_ERR_UNKNOWN;
    }

    pReplyPtr = (unsigned char *)malloc(sizeof(char) * bufLen);
    if(NULL == pReplyPtr)
    {
        /* Could not decide the OAMPDU size */
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
            "[OAM:%s:%d] can't allocate memory for reply OAMPDU\n", __FILE__, __LINE__);
        return EPON_OAM_ERR_MEM;
    }
    /* Reserve end symbol length */
    bufLen -= 1;
    *ppReplyOamPdu = pReplyPtr;

    /* Get the local info before all processing begin */
    epon_oam_localInfo_get(pOamPdu->llidIdx, &lastLInfo);
    while((remainLen >= 1) && (1 != isEnd) && (bufLen >= 1))
    {
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_INFO,
            "[OAM:%s:%d] info type 0x%02x\n", __FILE__, __LINE__, *pProcPtr);
        replyLen = 0;
        switch(*pProcPtr)
        {
        case EPON_INFO_OAMPDU_TYPE_END:
            /* OAM PDU ended */
            isEnd = 1;
            remainLen --;
            pProcPtr ++;
            break;
        case EPON_INFO_OAMPDU_TYPE_LOCAL:
            /* Receive Remote device's "LOCAL" TLV */
            ret = epon_oam_stdInfo_parser(&oamInfo, pProcPtr, remainLen, &extractLen);
            if((EPON_OAM_ERR_OK != ret) || (remainLen < extractLen))
            {
                remainLen = 0;
                EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
                    "[OAM:%s:%d] epon_oam_stdInfo_parser Failed %d\n", __FILE__, __LINE__, ret);
                break;
            }
            remainLen -= extractLen;
            pProcPtr += extractLen;

#if 0
            /* If the revision changed? */
            epon_oam_remoteInfo_get(pOamPdu->llidIdx, &lastRInfo);
            if(lastRInfo.valid && (oamInfo.revision == lastRInfo.revision))
            {
                /* Ignore the info OAM PDU */
                *pReplyLen = 0;
                return EPON_OAM_ERR_OK;
            }
#endif

            ret = epon_oam_discovery_proc(pOamPdu->llidIdx, pOamPdu, &oamInfo);
            if(EPON_OAM_ERR_OK == ret)
            {
                epon_oam_stdInfo_gen(
                    pOamPdu->llidIdx,
                    pReplyPtr,
                    bufLen,
                    &replyLen);
                pReplyPtr += replyLen;
                bufLen -= replyLen;
            }
            break;
        case EPON_INFO_OAMPDU_TYPE_REMOTE:
            /* Receive Remote device's "REMOTE" TLV */
            ret = epon_oam_stdInfo_parser(&oamInfo, pProcPtr, remainLen, &extractLen);
            if((EPON_OAM_ERR_OK != ret) || (remainLen < extractLen))
            {
                remainLen = 0;
                EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
                    "[OAM:%s:%d] epon_oam_stdInfo_parser Failed %d\n", __FILE__, __LINE__, ret);
                break;
            }
            remainLen -= extractLen;
            pProcPtr += extractLen;

            /* The info should be exact the same as the one sent to remote */
            epon_oam_localInfo_get(pOamPdu->llidIdx, &localOamInfo);
            if(memcmp(&oamInfo, &localOamInfo, sizeof(oam_oamInfo_t)) != 0)
            {
                EPON_OAM_PRINT(EPON_OAM_DBGFLAG_DUMP,
                    "[OAM:%s:%d] Remote's remote OAMPDU inconsist as local one\n", __FILE__, __LINE__);
            }
            if(memcmp(&oamInfo.oui[0], &localOamInfo.oui[0], 4) == 0)
            {
                if(memcmp(&oamInfo.venderSpecInfo[0], &localOamInfo.venderSpecInfo[0], 4) != 0)
                {
                    rtk_epon_llid_entry_t llidEntry;
                    
                    /* disable LLID entry */
                    llidEntry.llidIdx = pOamPdu->llidIdx;
                    ret = rtk_epon_llid_entry_get(&llidEntry);
                    if(ret)
                    {
                        return ret;
                    }
                    llidEntry.valid = DISABLED;
                    llidEntry.llid = 0x7fff;
                    ret = rtk_epon_llid_entry_set(&llidEntry);
                    if(ret)
                    {
                        return ret;
                    }
                    printf("\nOUI mismatch %2.2x %2.2x %2.2x %2.2x\n",oamInfo.venderSpecInfo[0],oamInfo.venderSpecInfo[1],oamInfo.venderSpecInfo[2],oamInfo.venderSpecInfo[3]);
                }
            }
            break;
        case EPON_INFO_OAMPDU_TYPE_ORG_SPEC:
            pOrgSpecData = NULL;
            /* Search for registered organization specific via OUI */
            ret = epon_oam_orgSpecInfo_parser(
                pOamPdu->llidIdx,
                pProcPtr,
                remainLen,
                &extractLen,
                oui,
                &pOrgSpecData);
            if((EPON_OAM_ERR_OK != ret) || (remainLen < extractLen))
            {
                remainLen = 0;
                EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
                    "[OAM:%s:%d] epon_oam_orgSpecInfo_parser Failed %d\n", __FILE__, __LINE__, ret);
                break;
            }
            remainLen -= extractLen;
            pProcPtr += extractLen;

            ret = epon_oam_orgSpecInfo_handler(
                pOamPdu,
                pReplyPtr,
                bufLen,
                &replyLen,
                oui,
                &pOrgSpecData);

            if(EPON_OAM_ERR_OK != ret)
            {
                EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
                    "[OAM:%s:%d] epon_oam_orgSpecInfo_handler Failed %d\n", __FILE__, __LINE__, ret);
                break;
            }
            pReplyPtr += replyLen;
            bufLen -= replyLen;
            break;
        case EPON_INFO_OAMPDU_TYPE_RESERVED:
        default:
            EPON_OAM_PRINT(EPON_OAM_DBGFLAG_WARN,
                "[OAM:%s:%d] Reserved info OAM type 0x%02x\n", __FILE__, __LINE__, *pProcPtr);
            /* Reserved types, skip it */
            remainLen --;
            pProcPtr ++;
            break;
        }
    }

    /* Check if local info revision change required */
    epon_oam_localInfo_get(pOamPdu->llidIdx, &currLInfo);
    if(memcmp(&currLInfo, &lastLInfo, sizeof(currLInfo)) != 0)
    {
        /* Increase the revision */
        currLInfo.revision ++;
        epon_oam_localInfo_set(pOamPdu->llidIdx, &currLInfo);
    }

    /* Add end type for reply OAMPDU */
    *pReplyPtr = EPON_INFO_OAMPDU_TYPE_END;
    pReplyPtr ++;
    *pReplyLen = (pReplyPtr - *ppReplyOamPdu);

    return EPON_OAM_ERR_OK;
}

static int epon_oam_stdEvent_process(
    oam_oamPdu_t *pOamPdu,
    unsigned char **ppReplyOamPdu,
    unsigned short *pReplyLen)
{
    int ret;
    void *pOrgSpecData;
    unsigned char oui[3];
    unsigned char isEnd;
    unsigned char *pProcPtr, *pReplyPtr;
    unsigned short remainLen, extractLen;
    unsigned short bufLen, replyLen, seqNumber;

    /* 1. Parse standard/organization specific/unknown event OAM fields
     * 2. Handle standard/organization specific/unknown event OAM fields
     * 3. Transmit the reply OAMPDU
     */
    isEnd = 0;
    pProcPtr = pOamPdu->pData;
    remainLen = pOamPdu->dataLen;

    ret = epon_oam_maxOamPduSize_get(pOamPdu->llidIdx, &bufLen);
    if(EPON_OAM_ERR_OK != ret)
    {
        /* Could not decide the OAMPDU size */
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
            "[OAM:%s:%d] can't get OAMPDU max size\n", __FILE__, __LINE__);
        return EPON_OAM_ERR_UNKNOWN;
    }

    pReplyPtr = (unsigned char *)malloc(sizeof(char) * bufLen);
    if(NULL == pReplyPtr)
    {
        /* Could not decide the OAMPDU size */
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
            "[OAM:%s:%d] can't allocate memory for reply OAMPDU\n", __FILE__, __LINE__);
        return EPON_OAM_ERR_MEM;
    }
    /* Reserve end symbol length */
    bufLen -= 1;
    *ppReplyOamPdu = pReplyPtr;

    if(remainLen >= 2)
    {
        /* Retrive sequence number */
        ((unsigned char *)(&seqNumber))[0] = *pProcPtr;
        pProcPtr++;
        ((unsigned char *)(&seqNumber))[1] = *pProcPtr;
        pProcPtr++;
        remainLen -= 2;
    }
    
    while((remainLen >= 1) && (1 != isEnd) && (bufLen >= 1))
    {
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_INFO,
            "[OAM:%s:%d] event type 0x%02x\n", __FILE__, __LINE__, *pProcPtr);
        replyLen = 0;
        switch(*pProcPtr)
        {
        case EPON_EVENT_OAMPDU_TYPE_END:
            /* OAM PDU ended */
            isEnd = 1;
            remainLen --;
            pProcPtr ++;
            break;
        case EPON_EVENT_OAMPDU_TYPE_ESPE:
        case EPON_EVENT_OAMPDU_TYPE_EFE:
        case EPON_EVENT_OAMPDU_TYPE_EFPE:
        case EPON_EVENT_OAMPDU_TYPE_EFSSE:
            ret = epon_oam_stdEvent_parser(*pProcPtr, pProcPtr, remainLen, &extractLen);
            if((EPON_OAM_ERR_OK != ret) || (remainLen < extractLen))
            {
                remainLen = 0;
                EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
                    "[OAM:%s:%d] epon_oam_stdEvent_parser Failed %d\n", __FILE__, __LINE__, ret);
                break;
            }
            remainLen -= extractLen;
            pProcPtr += extractLen;

            /* MODIFY ME - Process the event */
            /* MODIFY ME - Add reply mechanism here */
            break;
        case EPON_EVENT_OAMPDU_TYPE_ORG_SPEC:
            pOrgSpecData = NULL;
            /* Search for registered organization specific via OUI */
            ret = epon_oam_orgSpecEvent_parser(
                pOamPdu->llidIdx,
                pProcPtr,
                remainLen,
                &extractLen,
                oui,
                &pOrgSpecData);
            if((EPON_OAM_ERR_OK != ret) || (remainLen < extractLen))
            {
                remainLen = 0;
                EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
                    "[OAM:%s:%d] epon_oam_orgSpecEvent_parser Failed %d\n", __FILE__, __LINE__, ret);
                break;
            }
            remainLen -= extractLen;
            pProcPtr += extractLen;

            ret = epon_oam_orgSpecEvent_handler(
                pOamPdu,
                pReplyPtr,
                bufLen,
                &replyLen,
                oui,
                &pOrgSpecData);

            if(EPON_OAM_ERR_OK != ret)
            {
                EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
                    "[OAM:%s:%d] epon_oam_orgSpecEvent_handler Failed %d\n", __FILE__, __LINE__, ret);
                break;
            }
            pReplyPtr += replyLen;
            bufLen -= replyLen;
            break;
        case EPON_EVENT_OAMPDU_TYPE_RESERVED:
        default:
            EPON_OAM_PRINT(EPON_OAM_DBGFLAG_WARN,
                "[OAM:%s:%d] Reserved event OAM type 0x%02x\n", __FILE__, __LINE__, *pProcPtr);
            /* Reserved types, skip it */
            remainLen --;
            pProcPtr ++;
            break;
        }
    }

    /* Add end type for reply OAMPDU */
    *pReplyPtr = EPON_EVENT_OAMPDU_TYPE_END;
    pReplyPtr ++;
    *pReplyLen = (pReplyPtr - *ppReplyOamPdu);

    return EPON_OAM_ERR_OK;
}

static int epon_oam_stdVarReq_process(
    oam_oamPdu_t *pOamPdu,
    unsigned char **ppReplyOamPdu,
    unsigned short *pReplyLen)
{
    int ret;
    unsigned char *pProcPtr, *pReplyPtr;
    unsigned short remainLen, allRespLen;
    unsigned short bufLen, maxBufLen;
    oam_varCb_t varCb;
    oam_varTarget_t *pVarTarget = NULL, varTarget;
    oam_varContainer_t *pVarContainer;
    int (*handler)(
        oam_varTarget_t *pVarTarget,
        oam_varDescriptor_t varDesc,
        oam_varContainer_t **ppVarContainer);

    /* 1. Allocate buffer for reply
     * 2. Parse branch/leaf and search database for process function
     * 3. Check if immediately reply is required
     */
    pProcPtr = pOamPdu->pData;
    remainLen = pOamPdu->dataLen;

    ret = epon_oam_maxOamPduSize_get(pOamPdu->llidIdx, &maxBufLen);
    if(EPON_OAM_ERR_OK != ret)
    {
        /* Could not decide the OAMPDU size */
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
            "[OAM:%s:%d] can't get OAMPDU max size\n", __FILE__, __LINE__);
        return EPON_OAM_ERR_UNKNOWN;
    }

    pReplyPtr = (unsigned char *)malloc(sizeof(char) * maxBufLen);
    if(NULL == pReplyPtr)
    {
        /* Could not decide the OAMPDU size */
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
            "[OAM:%s:%d] can't allocate memory for reply OAMPDU\n", __FILE__, __LINE__);
        return EPON_OAM_ERR_MEM;
    }
    /* Reserve end symbol length */
    bufLen = maxBufLen - 1;
    *ppReplyOamPdu = pReplyPtr;

    /* No any variable descriptor in request? */
    if((remainLen >= 1) && (EPON_VAR_OAMPDU_TYPE_END == *pProcPtr))
    {
        *pReplyLen = 0;
        return EPON_OAM_ERR_OK;
    }

    while(remainLen >= EPON_VAR_OAMPDU_DESC_LEN)
    {
        varCb.varDesc.varBranch = *pProcPtr;
        pProcPtr ++;
        memcpy(&varCb.varDesc.varLeaf, pProcPtr, sizeof(unsigned short));
        pProcPtr += 2;
        remainLen -= EPON_VAR_OAMPDU_DESC_LEN;

        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_INFO,
            "[OAM:%s:%d] var branch/leaf 0x%02x/0x%04x\n",
            __FILE__, __LINE__, varCb.varDesc.varBranch, varCb.varDesc.varLeaf);

        pVarContainer = NULL;
        /* Searh for callback function for specified variable (branch/leaf) */
        ret = epon_oam_variableCb_get(&varCb);
        if(EPON_OAM_ERR_OK != ret)
        {
            handler = NULL;
            /* Get default callback function for specified variable (branch/leaf) */
            ret = epon_oam_defVarCb_get(&handler);
            if(NULL != handler)
            {
                ret = handler(pVarTarget, varCb.varDesc, &pVarContainer);
            }
        }
        else
        {
            /* Check if it is an object identifier number */
            if(varCb.varType == EPON_OAM_VARTYPE_OBJECTID)
            {
                if(remainLen >= EPON_VAR_OAMPDU_TARGET_LEN)
                {
                    if(EPON_VAR_OAMPDU_OBJID_LEN == *pProcPtr)
                    {
                        /* This is a variable container - object identifier number */
                        varTarget.varDesc = varCb.varDesc;
                        varTarget.varWidth = EPON_VAR_OAMPDU_OBJID_LEN;
                        pProcPtr++;     /* Width */
                        memcpy(&varTarget.varData, pProcPtr, EPON_VAR_OAMPDU_OBJID_LEN);
                        pProcPtr += EPON_VAR_OAMPDU_OBJID_LEN;  /* Data */
                        pVarTarget = &varTarget;
                    }
                    else
                    {
                        /* Bad format */
                        pProcPtr += EPON_VAR_OAMPDU_TARGET_LEN;
                        remainLen -= EPON_VAR_OAMPDU_TARGET_LEN;
                    }
                }
                else
                {
                    /* Bad format */
                    pProcPtr += remainLen;
                    remainLen = 0;
                }    
            }
            else
            {
                if(NULL != varCb.handler)
                {
                    /* Handler the variable request */
                    ret = varCb.handler(pVarTarget, varCb.varDesc, &pVarContainer);
                }
                else
                {
                    EPON_OAM_PRINT(EPON_OAM_DBGFLAG_INFO,
                        "[OAM:%s:%d] unknown error\n", __FILE__, __LINE__);
                }                    
            }
        }

        if((EPON_OAM_ERR_OK == ret) && (NULL != pVarContainer))
        {
            /* Calculate all response length */
            allRespLen = epon_oam_varRespLen_calc(varCb.varType, pVarContainer);
            /* Check for oversize variable response */
            if(allRespLen > maxBufLen)
            {
                /* Response size is larger than the maximum response size
                 * Reply with error indicator
                 */
                epon_oam_varRespErrorLen_gen(pVarContainer, pReplyPtr);
                break;
            }
            else
            {
                if(allRespLen > bufLen)
                {
                    /* The reply is larger then the remain buffer
                     * Send reply immediately
                     * END symbol size is guarantee at buffer allocate
                     */
                    *pReplyPtr = EPON_VAR_OAMPDU_TYPE_END;
                    pReplyPtr++;
                    bufLen--;

                    epon_oam_reply_send(
                        pOamPdu->llidIdx,
                        pOamPdu->code,
                        *ppReplyOamPdu,
                        (pReplyPtr - *ppReplyOamPdu));

                    /* Allocate new buffer for the remain response */
                    pReplyPtr = (unsigned char *)malloc(sizeof(char) * maxBufLen);
                    if(NULL == pReplyPtr)
                    {
                        /* Could not decide the OAMPDU size */
                        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
                            "[OAM:%s:%d] can't allocate memory for reply OAMPDU\n", __FILE__, __LINE__);
                        return EPON_OAM_ERR_MEM;
                    }
                    bufLen = maxBufLen;
                    *ppReplyOamPdu = pReplyPtr;
                }

                /* Put the resp into frame buffer */
                epon_oam_varResp_gen(varCb.varType, pVarContainer, pReplyPtr);
                pReplyPtr += allRespLen;
                bufLen -= allRespLen;
            }
            epon_oam_varResp_free(&pVarContainer);
        }

        if((remainLen >= 1) && (EPON_VAR_OAMPDU_TYPE_END == *pProcPtr))
        {
            /* END symbol size is guarantee at buffer allocate */
            *pReplyPtr = EPON_VAR_OAMPDU_TYPE_END;
            pReplyPtr++;
            bufLen--;
            break;
        }
    }
    *pReplyLen = (pReplyPtr - *ppReplyOamPdu);

    return EPON_OAM_ERR_OK;
}

static int epon_oam_stdVarResp_process(
    oam_oamPdu_t *pOamPdu,
    unsigned char **ppReplyOamPdu,
    unsigned short *pReplyLen)
{
    *pReplyLen = 0;
    return EPON_OAM_ERR_OK;
}

static int epon_oam_stdLoopback_process(
    oam_oamPdu_t *pOamPdu,
    unsigned char **ppReplyOamPdu,
    unsigned short *pReplyLen)
{
    *pReplyLen = 0;
    return EPON_OAM_ERR_OK;
}

int epon_oam_stdOrgSpec_process(
    oam_oamPdu_t *pOamPdu,
    unsigned char **ppReplyOamPdu,
    unsigned short *pReplyLen)
{
    int ret;
    unsigned char oui[3];
    unsigned char *pProcPtr, *pReplyPtr;
    unsigned short remainLen;
    unsigned short bufLen, replyLen;
    oam_orgSpecCb_t orgSpecCb;

    /* 1. Parse organization specific OAM's OUI
     * 2. Process organization specific OAM
     * 3. Transmit the reply OAMPDU
     */
    pProcPtr = pOamPdu->pData;
    remainLen = pOamPdu->dataLen;

    ret = epon_oam_maxOamPduSize_get(pOamPdu->llidIdx, &bufLen);
    if(EPON_OAM_ERR_OK != ret)
    {
        /* Could not decide the OAMPDU size */
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
            "[OAM:%s:%d] can't get OAMPDU max size\n", __FILE__, __LINE__);
        return EPON_OAM_ERR_UNKNOWN;
    }

    pReplyPtr = (unsigned char *)malloc(sizeof(char) * bufLen);
    if(NULL == pReplyPtr)
    {
        /* Could not decide the OAMPDU size */
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
            "[OAM:%s:%d] can't allocate memory for reply OAMPDU\n", __FILE__, __LINE__);
        return EPON_OAM_ERR_MEM;
    }

	memset(pReplyPtr, 0, (sizeof(char) * bufLen));
	
    *ppReplyOamPdu = pReplyPtr;

    oui[0] = pProcPtr[0];
    oui[1] = pProcPtr[1];
    oui[2] = pProcPtr[2];

    orgSpecCb.processor = NULL;
    ret = epon_oam_orgSpecCb_get(EPON_OAM_CBTYPE_ORGSPEC, oui, (void *) &orgSpecCb);
    if(EPON_OAM_ERR_OK != ret)
    {
        /* No such OUI found
         * Get the default callback function
         */
       epon_oam_defOrgSpecCb_get(EPON_OAM_CBTYPE_ORGSPEC, (void *) &orgSpecCb);
    }

    replyLen = 0;
    if(NULL != orgSpecCb.processor)
    {
        ret = orgSpecCb.processor(
            pOamPdu,
            pProcPtr,
            remainLen,
            pReplyPtr,
            bufLen,
            &replyLen);
    }

    if((EPON_OAM_ERR_OK != ret) || (bufLen < replyLen) || (NULL == orgSpecCb.processor))
    {
        replyLen = 0;
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
            "[OAM:%s:%d] orgSpecCb Failed %d\n", __FILE__, __LINE__, ret);
    }
    *pReplyLen = replyLen;

    return EPON_OAM_ERR_OK;
}

static int
epon_oam_rx_handler(
    unsigned char llidIdx,
    unsigned char *frame,
    unsigned short length)
{
    int ret;
    unsigned char *pReplyOamPdu;
    unsigned short replyLen, extractLen;
    oam_oamPdu_t oamPdu;
    oam_config_t oamConfig;
    unsigned int  oam_dbg_flag;

    /* Check for OAM enable */
    epon_oam_config_get(llidIdx, &oamConfig);
    if(!oamConfig.oamEnabled)
    {
        return EPON_OAM_ERR_OK;
    }

    epon_oam_dbgFlag_get(&oam_dbg_flag);

    if ((oam_dbg_flag & EPON_OAM_DBGFLAG_ORGRX) && (frame[17] == 0xfe))
    {
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ORGRX,
            "[OAM:%s:%d] OAMPDU Rx\n", __FILE__, __LINE__);
        DUMP_HEX_VALUE(EPON_OAM_DBGFLAG_ORGRX, frame, length);
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ORGRX, "\n");
    }
    else
    {
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_RXOAM,
            "[OAM:%s:%d] OAMPDU Rx\n", __FILE__, __LINE__);
        DUMP_HEX_VALUE(EPON_OAM_DBGFLAG_RXOAM, frame, length);
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_RXOAM, "\n");
    }
    
    epon_oam_counter_inc(llidIdx, EPON_OAM_COUNTERTYPE_RX);

    /* 1. Parsing the frame
     * 2. Process frame according to the OAMPDU code
     * 3. Reply OAMPDU
     */
    if(length < 60)
    {
        /* Ignore under size frame */
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_WARN, "[OAM:%s:%d] frame under size\n", __FILE__, __LINE__);
        return EPON_OAM_ERR_PARAM;
    }

    memset((unsigned char *)&oamPdu, 0x0, sizeof(oam_oamPdu_t));
    oamPdu.llidIdx = llidIdx;
    ret = epon_oam_stdHdr_parser(&oamPdu, frame, length, &extractLen);
    if(0 != ret)
    {
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR, "[OAM:%s:%d]\n", __FILE__, __LINE__);
    }

    pReplyOamPdu = NULL;
    replyLen = 0;
    EPON_OAM_PRINT(EPON_OAM_DBGFLAG_INFO, 
        "[OAM:%s:%d] pOamPdu->code %d\n", __FILE__, __LINE__, oamPdu.code);
    switch(oamPdu.code)
    {
    case EPON_OAMPDU_CODE_INFO:
        ret = epon_oam_stdInfo_process(&oamPdu, &pReplyOamPdu, &replyLen);
        epon_oam_counter_inc(oamPdu.llidIdx, EPON_OAM_COUNTERTYPE_RXINFO);
        if(replyLen > 0)
        {
            epon_oam_counter_inc(oamPdu.llidIdx, EPON_OAM_COUNTERTYPE_TXINFO);
        }
        break;
    case EPON_OAMPDU_CODE_EVENT:
        ret = epon_oam_stdEvent_process(&oamPdu, &pReplyOamPdu, &replyLen);
        break;
    case EPON_OAMPDU_CODE_VAR_REQ:
        ret = epon_oam_stdVarReq_process(&oamPdu, &pReplyOamPdu, &replyLen);
        /* Reply code for request is reqponse */
        oamPdu.code = EPON_OAMPDU_CODE_VAR_RESP;
        break;
    case EPON_OAMPDU_CODE_VAR_RESP:
        ret = epon_oam_stdVarResp_process(&oamPdu, &pReplyOamPdu, &replyLen);
        break;
    case EPON_OAMPDU_CODE_LOOPBACK:
        ret = epon_oam_stdLoopback_process(&oamPdu, &pReplyOamPdu, &replyLen);
        break;
    case EPON_OAMPDU_CODE_ORG_SPEC:
        ret = epon_oam_stdOrgSpec_process(&oamPdu, &pReplyOamPdu, &replyLen);
        epon_oam_counter_inc(oamPdu.llidIdx, EPON_OAM_COUNTERTYPE_RXORGSPEC);
        if(replyLen > 0)
        {
            epon_oam_counter_inc(oamPdu.llidIdx, EPON_OAM_COUNTERTYPE_TXORGSPEC);
        }
        break;
    case EPON_OAMPDU_CODE_RESERVED:
    default:
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_WARN,
            "[OAM:%s:%d] illegal OAMPDU code %u\n", __FILE__, __LINE__, oamPdu.code);
        break;
    }

    if(replyLen > 0)
    {
        ret = epon_oam_reply_send(oamPdu.llidIdx, oamPdu.code, pReplyOamPdu, replyLen);
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_INFO, 
            "[OAM:%s:%d] epon_oam_reply_send %d\n", __FILE__, __LINE__, oamPdu.code);

        /* If there anything to be sent, the incoming OAMPDU must have at least
         * one parsable TLV
         */
        epon_oam_event_send(oamPdu.llidIdx, EPON_OAM_EVENT_KEEPALIVE_RESET);
    }
    else
    {
        epon_oam_counter_inc(oamPdu.llidIdx, EPON_OAM_COUNTERTYPE_DROP);
    }

    if(NULL != pReplyOamPdu)
    {
        free(pReplyOamPdu);
    }

    return ret;
}


void *epon_oam_rxThread(void *argu)
{
    int ret;
    pr_identifier_t *prId;
    unsigned short dataLen;
    unsigned char *payload;
    unsigned char llidIdx;

    pktRedirect_sock = socket(PF_NETLINK, SOCK_RAW,NETLINK_USERSOCK);
    ptk_redirect_userApp_reg(pktRedirect_sock, 0, MAX_PAYLOAD);

    payload = (unsigned char *)malloc(MAX_PAYLOAD * sizeof(char));
    /* Read message from kernel */ 
    if(NULL == payload)
    {
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
            "[OAM:%s:%d] No packet buffer for Rx!\n", __FILE__, __LINE__);
        return NULL;
    }

    while(1)
    {
        ret = ptk_redirect_userApp_recvPkt(pktRedirect_sock, MAX_PAYLOAD, &dataLen, payload);
        if(ret > 60)
        {
            /* Message from network(PON) side */
            llidIdx = *((unsigned char *)(payload + (dataLen - sizeof(llidIdx))));
            EPON_OAM_PRINT(EPON_OAM_DBGFLAG_INFO,
                "[OAM:%s:%d] Received message from LLIDIdx %u size: %d (0x%08x)\n", __FILE__, __LINE__, llidIdx, dataLen - sizeof(llidIdx), payload);
            
            epon_oam_rx_handler(llidIdx, payload, dataLen - sizeof(llidIdx));
        }
        else if(ret > 0)
        {
            /* Message from kernel space */
            /* Only LOS message use this channel currently */
            if(4 == dataLen)
            {
                /* Check Magic */
                if((payload[0] == 0x15) && (payload[1] == 0x68))
                {
                    if((payload[2] == 0xde) && (payload[3] == 0xad))
                    {
                        /* LOS message */
                        /* MODIFY ME - add multiple LLID support */
                        epon_oam_event_send(0, EPON_OAM_EVENT_LOS);
                    }
                }
            }
        }
        else if(ret <= 0)
        {
            EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
                "[OAM:%s:%d] receive fail %d\n", __FILE__, __LINE__, ret);
        }
    }
    free(payload);
    close(pktRedirect_sock);

	return NULL;
}

