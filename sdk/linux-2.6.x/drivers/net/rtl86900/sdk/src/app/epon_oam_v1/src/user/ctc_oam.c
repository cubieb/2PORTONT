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
 * $Revision: 46475 $
 * $Date: 2014-02-14 11:03:12 +0800 (?±ä?, 14 äºŒæ? 2014) $
 *
 * Purpose : Define the CTC related extended OAM
 *
 * Feature : Provide CTC related extended OAM parsing and handling
 *
 */

/*
 * Include Files
 */
/* Standard include */
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <pthread.h>
#include <errno.h>
#include <semaphore.h>
#include <sys/wait.h>
/* EPON OAM include */
#include "epon_oam_config.h"
#include "epon_oam_err.h"
#include "epon_oam_db.h"
#include "epon_oam_dbg.h"
#include "epon_oam_rx.h"
/* User specific include */
#include "ctc_oam.h"
#include "ctc_oam_var.h"
/* Inlcude the variable table */
#include "ctc_oam_vartbl.c"

/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */
const ctc_infoOamVer_t supportedVerList[] = 
{
/*  {
 *      { OUI - 3 octets },
 *      version - 1 octets
 *  },
 */
    /* Pretend to support these versions */
#if CTC_OAM_SUPPORT_VERSION_01
    {
        { 0x11, 0x11, 0x11 },   /* CTC */
        0x01,                   /* Ver 0.1 */
    },
#endif
#if CTC_OAM_SUPPORT_VERSION_13
    {
        { 0x11, 0x11, 0x11 },   /* CTC */
        0x13                    /* Ver 1.3 */
    },
#endif
#if CTC_OAM_SUPPORT_VERSION_20
    {
        { 0x11, 0x11, 0x11 },   /* CTC */
        0x20                    /* Ver 2.0 */
    },
#endif
#if CTC_OAM_SUPPORT_VERSION_21
    {
        { 0x11, 0x11, 0x11 },   /* CTC */
        0x21                    /* Ver 2.1 */
    },
#endif
#if CTC_OAM_SUPPORT_VERSION_30
    {
        { 0x11, 0x11, 0x11 },   /* CTC */
        0x30                    /* Ver 3.0 */
    }
#endif
};
static ctc_infoOamVer_t currCtcVer[EPON_OAM_SUPPORT_LLID_NUM];
static ctc_onuAuthLoid_t loidDb[EPON_OAM_SUPPORT_LLID_NUM];
static unsigned char ctc_onuAuth_state[EPON_OAM_SUPPORT_LLID_NUM];
static unsigned char ctc_onuAuth_failType[EPON_OAM_SUPPORT_LLID_NUM];
static unsigned char ctc_discovery_state[EPON_OAM_SUPPORT_LLID_NUM];
static ctc_swDlFile_t ctc_swdl_file; /* Single buffer for all LLID */
static pthread_t swDownload = 0; /* Download thread control */
static pthread_t swReboot = 0; /* Reboot thread control */
static ctc_churning_t ctc_last_churningKey[EPON_OAM_SUPPORT_LLID_NUM];
static sem_t ctcDataSem;
ctc_swDlBootInfo_t bootInfo;

/*
 * Macro Definition
 */
#define CTC_SWDL_STATE_SET(state)   \
{                                   \
    sem_wait(&ctcDataSem);          \
    ctc_swdl_file.flag = state;     \
    sem_post(&ctcDataSem);          \
}

#define CTC_SWDL_STATE_GET()        (ctc_swdl_file.flag)

/*
 * Function Declaration
 */

static void dump_ctc_infoOam(
    ctc_infoOam_t *pInfoOam)
{
    ctc_infoOamVerRec_t *pItem;

    if(NULL != pInfoOam)
    {
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_DUMP,
            "CTC Info OAM:\n");
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_DUMP,
            "Type      : 0x%02x\n", pInfoOam->type);
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_DUMP,
            "Length    : %u\n", pInfoOam->length);
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_DUMP,
            "OUI       : %02x:%02x:%02x\n", pInfoOam->oui[0], pInfoOam->oui[1], pInfoOam->oui[2]);
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_DUMP,
            "ExtSupport: 0x%02x\n", pInfoOam->extSupport);
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_DUMP,
            "Version   : 0x%02x\n", pInfoOam->version);

        pItem = pInfoOam->vertionList;
        while(NULL != pItem)
        {
            EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_DUMP,
                "   OUI    : %02x:%02x:%02x\n", pItem->oui[0], pItem->oui[1], pItem->oui[2]);
            EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_DUMP,
                "   Version: 0x%02x\n", pItem->version);
            pItem = pItem->next;
        }
    }
}

int ctc_oam_onuAuthLoid_set(unsigned char llidIdx, ctc_onuAuthLoid_t *pLoidAuth)
{
    /* Parameter check */
    if(llidIdx >= EPON_OAM_SUPPORT_LLID_NUM)
    {
        return EPON_OAM_ERR_PARAM;
    }
    if(NULL == pLoidAuth)
    {
        return EPON_OAM_ERR_PARAM;
    }

    loidDb[llidIdx] = *pLoidAuth;

    return EPON_OAM_ERR_OK;
}

int ctc_oam_onuAuthLoid_get(unsigned char llidIdx, ctc_onuAuthLoid_t *pLoidAuth)
{
    /* Parameter check */
    if(llidIdx >= EPON_OAM_SUPPORT_LLID_NUM)
    {
        return EPON_OAM_ERR_PARAM;
    }
    if(NULL == pLoidAuth)
    {
        return EPON_OAM_ERR_PARAM;
    }

    *pLoidAuth = loidDb[llidIdx];

    return EPON_OAM_ERR_OK;
}

int ctc_oam_onuAuthState_set(
    unsigned char llidIdx,
    unsigned char state,
    unsigned char failType)
{
    /* Parameter check */
    if(llidIdx >= EPON_OAM_SUPPORT_LLID_NUM)
    {
        return EPON_OAM_ERR_PARAM;
    }
    if(CTC_OAM_ONUAUTH_STATE_END <= state)
    {
        return EPON_OAM_ERR_PARAM;
    }

    ctc_onuAuth_state[llidIdx] = state;
    ctc_onuAuth_failType[llidIdx] = failType;

    return EPON_OAM_ERR_OK;
}

int ctc_oam_onuAuthState_get(
    unsigned char llidIdx,
    unsigned char *pState,
    unsigned char *pFailType)
{
    /* Parameter check */
    if(llidIdx >= EPON_OAM_SUPPORT_LLID_NUM)
    {
        return EPON_OAM_ERR_PARAM;
    }
    if(NULL == pState)
    {
        return EPON_OAM_ERR_PARAM;
    }

    *pState = ctc_onuAuth_state[llidIdx];
    if(NULL != pFailType)
    {
        *pFailType = ctc_onuAuth_failType[llidIdx];
    }

    return EPON_OAM_ERR_OK;
}

static void ctc_oam_verListItem_alloc(
    unsigned char oui[3],
    unsigned char ver,
    ctc_infoOamVerRec_t **ppVerListItem)
{
    ctc_infoOamVerRec_t *pItem;
    pItem = (ctc_infoOamVerRec_t *) malloc(sizeof(ctc_infoOamVerRec_t));

    if(NULL != pItem)
    {
        memcpy(pItem->oui, oui, 3);
        pItem->version = ver;
        pItem->next = NULL;
        *ppVerListItem = pItem;
    }
    else
    {
        *ppVerListItem = NULL;
    }
}

static void ctc_oam_verListItem_free(
    ctc_infoOamVerRec_t **ppVerListHead)
{
    ctc_infoOamVerRec_t *pVerList, *pVerListNext;

    pVerList = *ppVerListHead;
    while(NULL != pVerList)
    {
        pVerListNext = pVerList->next;
        free(pVerList);
        pVerList = pVerListNext;
    }
    *ppVerListHead = NULL;
}

static void ctc_oam_swDownloadBuf_clear(void)
{
    sem_wait(&ctcDataSem);
    if(ctc_swdl_file.fileName != NULL)
    {
        free(ctc_swdl_file.fileName);
        ctc_swdl_file.fileName = NULL;
    }

    if(ctc_swdl_file.mode != NULL)
    {
        free(ctc_swdl_file.mode);
        ctc_swdl_file.mode = NULL;
    }

    remove(CTC_ORGSPEC_SWDL_STORAGE);

    ctc_swdl_file.fileSize = 0;
    ctc_swdl_file.checkSize = 0;
    ctc_swdl_file.block = 0;
    /* Inside the semphore protection, don't use macro */
    ctc_swdl_file.flag = CTC_OAM_SWDOWNLOAD_BUF_CLEAR;
    sem_post(&ctcDataSem);

}

static void ctc_oam_swDownloadError_alloc(
    const ctc_swDownload_t inSwDownload, 
    ctc_swDownload_t *pOutSwDownload, 
    unsigned short errCode,
    char *pErrMsg)
{
    unsigned int strLen;

    pOutSwDownload->dataType = CTC_ORGSPEC_SWDL_TYPE_FILE;
    pOutSwDownload->opCode = CTC_ORGSPEC_SWDL_OPCODE_ERROR;
    pOutSwDownload->length = CTC_ORGSPEC_SWDOWNLOAD_HDR_LEN + CTC_ORGSPEC_SWDOWNLOAD_ERROR_LEN;
    pOutSwDownload->tid = inSwDownload.tid;
    pOutSwDownload->parse.error.errCode = errCode;
    if(pErrMsg != NULL)
    {
        strLen = strlen(pErrMsg);
        pOutSwDownload->parse.error.errMsg = (char *)malloc(strLen);
        if(pOutSwDownload->parse.error.errMsg != NULL)
        {
            memcpy(pOutSwDownload->parse.error.errMsg, pErrMsg, strLen);
            pOutSwDownload->length += strLen;
        }
        else
        {
            /* Ignore the error message if allocate failed */
        }
    }
    else
    {
        pOutSwDownload->parse.error.errMsg = NULL;
    }
}

static void ctc_oam_swDownloadError_free(
    ctc_swDownload_t *pSwDownload) 
{
    if(pSwDownload->parse.error.errMsg != NULL)
    {
        free(pSwDownload->parse.error.errMsg);
    }
}

static int ctc_oam_swDownloadNv_set(
    char *varName,
    char *varValue)
{
    char execCommand[CTC_EXECMD_UV_CMD_LEN];
    int execCmdStatus;

    if ((varName == NULL) || (varValue == NULL))
    {
        return EPON_OAM_ERR_PARAM;
    }

    memset(execCommand, 0, CTC_EXECMD_UV_CMD_LEN);

    sprintf(execCommand, "/bin/nv setenv %s %s", varName, varValue);

    execCmdStatus = system(execCommand);
    if (-1 == execCmdStatus || 0 == WIFEXITED(execCmdStatus) || 0 != WEXITSTATUS(execCmdStatus))
    {
        return -1;
    }

    return 0;
}

static int ctc_oam_swDownloadNv_get(
    char *varName,
    char *varValue)
{
    char execCommand[CTC_EXECMD_UV_CMD_LEN];
    char stdoutBuffer[CTC_EXECMD_STDOUT_LEN];
    char *pStr, *pReturnStr = NULL;
    FILE *pFD;

    if ((varName == NULL) || (varValue == NULL))
    {
        return EPON_OAM_ERR_PARAM;
    }

    memset(execCommand, 0, CTC_EXECMD_UV_CMD_LEN);
    memset(stdoutBuffer, 0, CTC_EXECMD_STDOUT_LEN);

    sprintf(execCommand, "/bin/nv getenv %s", varName);

    pFD = popen(execCommand, "r");
    if(pFD != NULL)
    {
        if (fgets(stdoutBuffer, CTC_EXECMD_STDOUT_LEN, pFD))
        {
            pStr = strtok(stdoutBuffer, "=");

            if (NULL == pStr)
            {
                pclose(pFD);
                return EPON_OAM_ERR_UNKNOWN;
            }
            else
            {
                strncpy(varValue, stdoutBuffer + strlen(stdoutBuffer) + 1, CTC_EXECMD_UV_VALUE_LEN);
            }
        }
        pclose(pFD);
    }

    return EPON_OAM_ERR_OK;
}

static void *ctc_oam_swDlThread(void *argu)
{
    int ret;
    unsigned char writeTarget;
    char execCommand[CTC_EXECMD_UV_CMD_LEN];

    /* Once started, chekc MD5 and program the flash 
     * It won't stop event if the EPON link down
     */
    /* Don't allow others to stop the download process */
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    /* Lock the data semaphore to prevent any change */
    sem_wait(&ctcDataSem);
    /* Execute the firmware upgrade script */
    writeTarget = (bootInfo.commit == 0) ? 1 : 0;
    sprintf(execCommand,
    	"/bin/sh -x /etc/scripts/fwu_starter.sh %d " CTC_ORGSPEC_SWDL_STORAGE, writeTarget);
    ret = system(execCommand);

    if(0 == ret)
    {
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_INFO,
            "[OAM:%s:%d] Program to img #%d compelet\n", __FILE__, __LINE__, writeTarget);

        /* Inside the semphore protection, don't use macro */
        ctc_swdl_file.flag = CTC_OAM_SWDOWNLOAD_BUF_WRITEOK;
    }
    else
    {
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_INFO,
            "[OAM:%s:%d] Program to img #%d failed\n", __FILE__, __LINE__, writeTarget);

        /* Inside the semphore protection, don't use macro */
        ctc_swdl_file.flag = CTC_OAM_SWDOWNLOAD_BUF_WRITEFAIL;
    }
    swDownload = 0;
    sem_post(&ctcDataSem);
    pthread_exit(NULL);

    return NULL;
}

static void *ctc_oam_rebootThread(void *argu)
{
    int ret;
    unsigned char writeTarget;
    char execCommand[CTC_EXECMD_UV_CMD_LEN];

    /* Wait 5 seconds to allow all oam to be replied */
    sleep(5);
    /* Reboot the system */
    sprintf(execCommand, "reboot -f");
    ret = system(execCommand);

    if(0 == ret)
    {
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_INFO,
            "[OAM:%s:%d] reboot command success\n", __FILE__, __LINE__);
    }
    else
    {
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_INFO,
            "[OAM:%s:%d] reboot command fail\n", __FILE__, __LINE__);
    }
    pthread_exit(NULL);

    return NULL;
}

static void ctc_oam_Info_gen(
    unsigned short expectLen,
    unsigned char extSupport,
    unsigned char version,
    ctc_infoOam_t *pRemoteInfoOam,
    unsigned char genInfoList,
    unsigned char *pBuf)
{
    /* Buffer size check should be done before calling gen function */
    int i;
    unsigned char *pBufPtr = pBuf;

    pBufPtr[0] = EPON_INFO_OAMPDU_TYPE_ORG_SPEC;
    pBufPtr[1] = expectLen;
    pBufPtr[2] = pRemoteInfoOam->oui[0];
    pBufPtr[3] = pRemoteInfoOam->oui[1];
    pBufPtr[4] = pRemoteInfoOam->oui[2];
    pBufPtr[5] = extSupport;
    pBufPtr[6] = version;
    pBufPtr += CTC_INFO_OAM_MIN;
    if(0 != genInfoList)
    {
        for(i = 0;i < sizeof(supportedVerList)/sizeof(ctc_infoOamVer_t);i++)
        {
            pBufPtr[0] = supportedVerList[i].oui[0];
            pBufPtr[1] = supportedVerList[i].oui[1];
            pBufPtr[2] = supportedVerList[i].oui[2];
            pBufPtr[3] = supportedVerList[i].version;
            pBufPtr += CTC_INFO_OAM_VERITEM_LEN;
        }
    }
}

static void ctc_oam_orgSpecHdr_gen(
    unsigned char oui[],        /* OUI to be generate */
    unsigned char extOpcode,    /* Extended Opcode to be generate */
    unsigned char *pReplyBuf,   /* Frame buffer for reply OAM */
    unsigned short *pReplyLen)  /* Reply size used by this handler*/
{
    /* Buffer size check should be done before calling gen function */

    pReplyBuf[0] = oui[0];
    pReplyBuf[1] = oui[1];
    pReplyBuf[2] = oui[2];
    pReplyBuf[3] = extOpcode;

    *pReplyLen = CTC_ORGSPEC_HDR_LEN;
}

static void ctc_oam_orgSpecVarInstant_gen(
    ctc_varInstant_t *pInstant, /* Instant data to be generate */
    unsigned char *pReplyBuf,   /* Frame buffer for reply OAM */
    unsigned short *pReplyLen)  /* Reply size used by this handler*/
{
    /* Buffer size check should be done before calling gen function */
    unsigned char *pPtr;

    pPtr = pReplyBuf;
    *pPtr = pInstant->varDesc.varBranch;
    pPtr += 1; /* Branch */
    pPtr[0] = ((unsigned char *)(&pInstant->varDesc.varLeaf))[0];
    pPtr[1] = ((unsigned char *)(&pInstant->varDesc.varLeaf))[1];
    pPtr += 2; /* Leaf */
    *pPtr = pInstant->varWidth;
    pPtr += 1; /* Width */

    if(pInstant->varWidth == CTC_ORGSPEC_VARINSTANT_WIDTH20)
    {
        /*
         * CTC2.0 Index TLV:
         * ----------------
         * 1 	Branch
         * ----------------
         * 2 	Leaf
         * ----------------
         * 1 	Width=0x01
         * ----------------
         * 1	Value
         * ----------------			 
         * if request value is 0xff(all eth ports), so we must set the response value is 0x01(the first port)
         */
        if(pInstant->varData[0] == 0xFF)
        {
            pPtr[0] = 0x01;
        }
        else
        {
            memcpy(pPtr, pInstant->varData, sizeof(unsigned char) * pInstant->varWidth);
        }
    }
    else if(pInstant->varWidth == CTC_ORGSPEC_VARINSTANT_WIDTH21)
    {
        /*
		 * CTC2.1 Index TLV:
		 * ----------------
		 * 1 	Branch
		 * ----------------
		 * 2 	Leaf
		 * ----------------
		 * 1 	Width=0x04
		 * ----------------
		 * 4	Value	-------\
		 * ----------------     \
         *                      ----------
         *                      1 portType
         *                      ----------
         *                      1 chassisNo
         *                      ----------
         *                      1 slotNo
         *                      ----------
         *                      1 portNo
         *                      ----------
         * if request value is 0x0100ffff(all eth ports), so we must set the response value is 0x01000001(the first port)
         */
        if(pInstant->parse.uniPort.portNo == 0xFF)		
        {
            pPtr[0] = pInstant->parse.uniPort.portType;/*the value must be reserved */
            pPtr[1] = pInstant->parse.uniPort.chassisNo;
            pPtr[2] = 0x00;
            pPtr[3] = 0x01;
        }
        else
        {
            memcpy(pPtr, pInstant->varData, sizeof(unsigned char) * pInstant->varWidth);
        }
    }

    pPtr += sizeof(unsigned char) * pInstant->varWidth; /* Data */

    *pReplyLen = pPtr - pReplyBuf;
}

static void ctc_oam_orgSpecVarContainer_gen(
    ctc_varContainer_t *pContainer, /* Instant data to be generate */
    unsigned char *pReplyBuf,   /* Frame buffer for reply OAM */
    unsigned short *pReplyLen)  /* Reply size used by this handler*/
{
    /* Buffer size check should be done before calling gen function */
    unsigned char *pPtr;
    unsigned short remainLen;

    pPtr = pReplyBuf;
    *pPtr = pContainer->varDesc.varBranch;
    pPtr += 1; /* Branch */
    pPtr[0] = ((unsigned char *)(&pContainer->varDesc.varLeaf))[0];
    pPtr[1] = ((unsigned char *)(&pContainer->varDesc.varLeaf))[1];
    pPtr += 2; /* Leaf */
    if(CTC_VAR_CONTAINER_INDICATOR & pContainer->varWidth)
    {
        *pPtr = pContainer->varWidth & 0x00ff;
        pPtr += 1; /* Width */
        /* No data field */
    }
    else
    {
        remainLen = pContainer->varWidth;
        while(0 != remainLen)
        {
            if(remainLen >= 0x80)
            {
                *pPtr = 0x00;
                pPtr += 1; /* Width */
                memcpy(pPtr, pContainer->pVarData, sizeof(unsigned char) * pContainer->varWidth);
                pPtr += sizeof(unsigned char) * pContainer->varWidth; /* Data */

                if(remainLen > 0x80)
                {
                    /* Create consecutive variable container with the same branch/leaf */
                    *pPtr = pContainer->varDesc.varBranch;
                    pPtr += 1; /* Branch */
                    *pPtr = ((unsigned char *)(&pContainer->varDesc.varLeaf))[0];
                    *pPtr = ((unsigned char *)(&pContainer->varDesc.varLeaf))[1];
                    pPtr += 2; /* Leaf */
                }
                remainLen -= 0x80;
            }
            else
            {
                *pPtr = pContainer->varWidth;
                pPtr += 1; /* Width */
                memcpy(pPtr, pContainer->pVarData, sizeof(unsigned char) * pContainer->varWidth);
                pPtr += sizeof(unsigned char) * pContainer->varWidth; /* Data */
                remainLen -= pContainer->varWidth;
            }
        }
    }

    *pReplyLen = pPtr - pReplyBuf;
}

static void ctc_oam_orgSpecOnuAuthLoid_gen(
    ctc_onuAuthLoid_t *pAuthLoid,  /* Auth LOID structure */
    unsigned char *pReplyBuf,   /* Frame buffer for reply OAM */
    unsigned short *pReplyLen)  /* Reply size used by this handler*/
{
    /* Buffer size check should be done before calling gen function */
    unsigned char *pPtr;
    unsigned short dataLen;
    unsigned short length;

    pPtr = pReplyBuf;
    length = CTC_ORGSPEC_ONUAUTH_RESP_LOID_LEN;
    *pPtr = CTC_ORGSPEC_ONUAUTH_CODE_RESP;
    pPtr += 1; /* Auth code */
    pPtr[0] = ((unsigned char *) &length)[0];
    pPtr[1] = ((unsigned char *) &length)[1];
    pPtr += 2; /* Auth data length */
    *pPtr = CTC_ORGSPEC_ONUAUTH_LOID;
    pPtr += 1; /* Auth type */
    /* According to the CTC standard, NULL (0x00) should be padded before
     * the LOID/password
     */
    /* LOID */
    dataLen = strlen(pAuthLoid->loid);
    dataLen = (dataLen <= CTC_ORGSPEC_ONUAUTH_LOID_LEN) ? dataLen : CTC_ORGSPEC_ONUAUTH_LOID_LEN;
    memset(pPtr, 0, CTC_ORGSPEC_ONUAUTH_LOID_LEN);
    memcpy(pPtr + (CTC_ORGSPEC_ONUAUTH_LOID_LEN - dataLen), pAuthLoid->loid, dataLen);
    pPtr += CTC_ORGSPEC_ONUAUTH_LOID_LEN; /* LOID */
    /* Password */
    dataLen = strlen(pAuthLoid->password);
    dataLen = (dataLen <= CTC_ORGSPEC_ONUAUTH_PASSWORD_LEN) ? dataLen : CTC_ORGSPEC_ONUAUTH_PASSWORD_LEN;
    memset(pPtr, 0, CTC_ORGSPEC_ONUAUTH_PASSWORD_LEN);
    memcpy(pPtr + (CTC_ORGSPEC_ONUAUTH_PASSWORD_LEN - dataLen), pAuthLoid->password, dataLen);
    pPtr += CTC_ORGSPEC_ONUAUTH_PASSWORD_LEN; /* Password */

    *pReplyLen = pPtr - pReplyBuf;
}

static void ctc_oam_orgSpecOnuAuthNak_gen(
    unsigned char desireType,   /* Desire auth type */
    unsigned char *pReplyBuf,   /* Frame buffer for reply OAM */
    unsigned short *pReplyLen)  /* Reply size used by this handler*/
{
    /* Buffer size check should be done before calling gen function */
    unsigned char *pPtr;
    unsigned short length;

    pPtr = pReplyBuf;
    length = CTC_ORGSPEC_ONUAUTH_RESP_NAK_LEN;
    *pPtr = CTC_ORGSPEC_ONUAUTH_CODE_RESP;
    pPtr += 1; /* Auth code */
    pPtr[0] = ((unsigned char *) &length)[0];
    pPtr[1] = ((unsigned char *) &length)[1];
    pPtr += 2; /* Auth data length */
    *pPtr = CTC_ORGSPEC_ONUAUTH_NAK;
    pPtr += 1; /* Auth type */
    *pPtr = desireType;
    pPtr += 1; /* Desire Auth type */

    *pReplyLen = pPtr - pReplyBuf;
}

static void ctc_oam_orgSpecSwDownload_gen(
    ctc_swDownload_t *pSwDl,    /* Software download data */
    unsigned char *pReplyBuf,   /* Frame buffer for reply OAM */
    unsigned short *pReplyLen)  /* Reply size used by this handler*/
{
    /* Buffer size check should be done before calling gen function */
    unsigned char *pPtr;

    pPtr = pReplyBuf;
    *pPtr = pSwDl->dataType;
    pPtr += 1; /* dataType */
    pPtr[0] = ((unsigned char *) &pSwDl->length)[0];
    pPtr[1] = ((unsigned char *) &pSwDl->length)[1];
    pPtr += 2; /* software download data length */
    pPtr[0] = ((unsigned char *) &pSwDl->tid)[0];
    pPtr[1] = ((unsigned char *) &pSwDl->tid)[1];
    pPtr += 2; /* TID */
    pPtr[0] = ((unsigned char *) &pSwDl->opCode)[0];
    pPtr[1] = ((unsigned char *) &pSwDl->opCode)[1];
    pPtr += 2; /* opCode */

    switch(pSwDl->opCode)
    {
    case CTC_ORGSPEC_SWDL_OPCODE_FILEREQ:
        /* TODO: not possible for ONU */
        break;
    case CTC_ORGSPEC_SWDL_OPCODE_FILEDATA:
        /* TODO: not possible for ONU */
        break;
    case CTC_ORGSPEC_SWDL_OPCODE_FILEACK:
        pPtr[0] = ((unsigned char *) &pSwDl->parse.fileAck.block)[0];
        pPtr[1] = ((unsigned char *) &pSwDl->parse.fileAck.block)[1];
        pPtr += 2; /* block */
        break;
    case CTC_ORGSPEC_SWDL_OPCODE_ERROR:
        pPtr[0] = ((unsigned char *) &pSwDl->parse.error.errCode)[0];
        pPtr[1] = ((unsigned char *) &pSwDl->parse.error.errCode)[1];
        pPtr += 2; /* errCode */
        if(pSwDl->parse.error.errMsg != NULL)
        {
            memcpy(pPtr, pSwDl->parse.error.errMsg, strlen(pSwDl->parse.error.errMsg));
            pPtr += strlen(pSwDl->parse.error.errMsg);
        }
        *pPtr = 0;
        pPtr += 1; /* ASCII NULL */
        break;
    case CTC_ORGSPEC_SWDL_OPCODE_ENDREQ:
        /* TODO: not possible for ONU */
        break;
    case CTC_ORGSPEC_SWDL_OPCODE_ENDRESP:
        *pPtr = pSwDl->parse.endResp.respCode;
        pPtr += 1; /* respCode */
        break;
    case CTC_ORGSPEC_SWDL_OPCODE_ACTREQ:
        /* TODO: not possible for ONU */
        break;
    case CTC_ORGSPEC_SWDL_OPCODE_ACTRESP:
        *pPtr = pSwDl->parse.activateResp.ack;
        pPtr += 1; /* ack */
        break;
    case CTC_ORGSPEC_SWDL_OPCODE_COMMITREQ:
        /* TODO: not possible for ONU */
        break;
    case CTC_ORGSPEC_SWDL_OPCODE_COMMITRESP:
        *pPtr = pSwDl->parse.commitResp.ack;
        pPtr += 1; /* ack */
        break;
    default:
        break;
    }

    *pReplyLen = pPtr - pReplyBuf;
}

static void ctc_oam_orgSpecChurning_gen(
    ctc_churning_t *pChurning,  /* Churning key structure */
    unsigned char *pReplyBuf,   /* Frame buffer for reply OAM */
    unsigned short *pReplyLen)  /* Reply size used by this handler*/
{
    /* Buffer size check should be done before calling gen function */
    unsigned char *pPtr;

    pPtr = pReplyBuf;
    *pPtr = pChurning->churningCode;
    pPtr += 1; /* churningCode */
    *pPtr = pChurning->keyIdx;
    pPtr += 1; /* keyIdx */
    if(CTC_ORGSPEC_CHURNING_NEWKEY == pChurning->churningCode)
    {
        memcpy(pPtr, pChurning->churningKey, CTC_ORGSPEC_CHURNING_KEY_LEN);
        pPtr += CTC_ORGSPEC_CHURNING_KEY_LEN; /* churningKey */
    }

    *pReplyLen = pPtr - pReplyBuf;
}

static int ctc_oam_orgSpecDba_calc(
    unsigned char dbaCode, /* DBA code to be encoded */
    ctc_dbaThreshold_t *pDbaThreshold)  /* DBA threshold structure */
{
    int i, j;
    unsigned short expectLen = 0;

    expectLen += CTC_ORGSPEC_DBA_HDR_LEN;
    switch(dbaCode)
    {
    case CTC_ORGSPEC_DBA_GET_REQ:
        /* Nothing to be encoded except DBA code */
        break;
    case CTC_ORGSPEC_DBA_SET_RESP:
        /* set_DBA_response has extra field Set ACK */
        expectLen += CTC_ORGSPEC_DBA_SETACK_LEN;
    case CTC_ORGSPEC_DBA_GET_RESP:
    case CTC_ORGSPEC_DBA_SET_REQ:
        expectLen += CTC_ORGSPEC_DBA_REPORTHDR_LEN;
        for(i = 0;i < pDbaThreshold->numQSet;i++)
        {
            expectLen += CTC_ORGSPEC_DBA_REPORTMAP_LEN;
            for(j = 0;j < CTC_ORGSPEC_DBA_QUEUE_MAX;j++)
            {
                if(pDbaThreshold->reportMap[i] & (1 << j))
                {
                    expectLen += CTC_ORGSPEC_DBA_QUEUETHRESHOLD_LEN;
                }
            }
        }
        break;
    }

    return expectLen;
}

static void ctc_oam_orgSpecDba_gen(
    unsigned char dbaCode, /* DBA code to be encoded */
    ctc_dbaThreshold_t *pDbaThreshold,  /* DBA threshold structure */
    unsigned char dbaSetAck,  /* DBA threshold set ACK,
                               * only valid if dbaCode = CTC_ORGSPEC_DBA_SET_RESP */
    unsigned char *pReplyBuf,   /* Frame buffer for reply OAM */
    unsigned short *pReplyLen)  /* Reply size used by this handler*/
{
    /* Buffer size check should be done before calling gen function */
    int i, j;
    unsigned char *pPtr;

    pPtr = pReplyBuf;
    *pPtr = dbaCode;
    pPtr += 1; /* DBA code */

    switch(dbaCode)
    {
    case CTC_ORGSPEC_DBA_GET_REQ:
        /* Nothing to be encoded except DBA code */
        break;
    case CTC_ORGSPEC_DBA_SET_RESP:
        /* set_DBA_response has extra field Set ACK */
        *pPtr = dbaSetAck & 0x01;
        pPtr += 1; /* Set ACK */
    case CTC_ORGSPEC_DBA_GET_RESP:
    case CTC_ORGSPEC_DBA_SET_REQ:
        /* According to CTC spec, the number of queue set is actual queue set + 1*/
        *pPtr = pDbaThreshold->numQSet + 1;
        pPtr += 1; /* Number of Queue sets */
        for(i = 0;i < pDbaThreshold->numQSet;i++)
        {
            *pPtr = pDbaThreshold->reportMap[i];
            pPtr += 1; /* Report bitmap */
            for(j = 0;j < CTC_ORGSPEC_DBA_QUEUE_MAX;j++)
            {
                if(pDbaThreshold->reportMap[i] & (1 << j))
                {
                    pPtr[0] = ((unsigned char *)(&pDbaThreshold->queueSet[i].queueThreshold[j]))[0];
                    pPtr[1] = ((unsigned char *)(&pDbaThreshold->queueSet[i].queueThreshold[j]))[1];
                    pPtr += CTC_ORGSPEC_DBA_QUEUETHRESHOLD_LEN; /* Queue threshold */
                }
            }
        }
        break;
    }
    *pReplyLen = pPtr - pReplyBuf;
}

static unsigned char ctc_oam_version_support(
    unsigned char oui[3],
    unsigned char version)
{
    int i;
    for(i = 0;i < sizeof(supportedVerList)/sizeof(ctc_infoOamVer_t);i++)
    {
        if((supportedVerList[i].oui[0] == oui[0]) &&
           (supportedVerList[i].oui[1] == oui[1]) &&
           (supportedVerList[i].oui[2] == oui[2]) &&
           (supportedVerList[i].version == version))
        {
            return 1;
        }           
    }
    return 0;
}

static int ctc_oam_orgSepcVarCb_get(
    ctc_varDescriptor_t varDesc,
    ctc_varCb_t *pVarCb)
{
    ctc_varCb_t *pVarCbDb;

    if(NULL == pVarCb)
    {
        return EPON_OAM_ERR_PARAM;
    }

    /* Search callback function according ot its branch */
    switch(varDesc.varBranch)
    {
    case CTC_VAR_REQBRANCH_STDATTR:
        pVarCbDb = ctc_stdAttrCb;
        break;
    case CTC_VAR_REQBRANCH_STDACT:
        pVarCbDb = ctc_stdActCb;
        break;
    case CTC_VAR_REQBRANCH_EXTATTR:
        pVarCbDb = ctc_extAttrCb;
        break;
    case CTC_VAR_REQBRANCH_EXTACT:
        pVarCbDb = ctc_extActCb;
        break;
    default:
        return EPON_OAM_ERR_NOT_FOUND;
    }

    while(0 != pVarCbDb->varDesc.varBranch)
    {
        if(pVarCbDb->varDesc.varLeaf == varDesc.varLeaf)
        {
            *pVarCb = *pVarCbDb;
            return EPON_OAM_ERR_OK;
        }
        /* Move to next array index */
        pVarCbDb = pVarCbDb + 1;
    }

    return EPON_OAM_ERR_NOT_FOUND;
}

static void ctc_oam_churningKey_rand(
    unsigned char key[])
{
    key[0] = rand() % 0x100;
    key[1] = rand() % 0x100;
    key[2] = rand() % 0x100;
}

static int ctc_oam_orgSpecDbaTheshold_parse(
    unsigned char *pFrame,      /* Frame payload current pointer */
    unsigned short length,      /* Frame payload length */
    unsigned short *pParseLen,
    ctc_dbaThreshold_t *pDbaThreshold)
{
    int i, j;
    unsigned char *pPtr;
    unsigned short expectLen;

    *pParseLen = 0;
    if(length < CTC_ORGSPEC_DBA_REPORTHDR_LEN)
    {
        /* Insufficient length for parsing */
        return EPON_OAM_ERR_OK;
    }

    pPtr = pFrame;
    pDbaThreshold->numQSet = *pPtr;
    /* Due to CTC specified that the report's last queue set contains
     * the total amount of all queues, the number of queue set value
     * in the frame will always include that last queue set but no actual
     * queueu set in the frame
     * pDbaThreshold->numQSet = actual queue set number + 1
     */
    if((pDbaThreshold->numQSet < CTC_ORGSPEC_DBA_QUEUESET_MIN + 1) ||
       (pDbaThreshold->numQSet > CTC_ORGSPEC_DBA_QUEUESET_MAX + 1))
    {
        /* Incorrect value */
        return EPON_OAM_ERR_OK;
    }
    pDbaThreshold->numQSet -= 1; /* Change to actual queue set number */
    CTC_BUF_ADD(pPtr, length, CTC_ORGSPEC_DBA_REPORTHDR_LEN);

    for(i = 0;i < pDbaThreshold->numQSet;i++)
    {
        if(length < CTC_ORGSPEC_DBA_REPORTMAP_LEN)
        {
            /* Insufficient length for parsing */
            return EPON_OAM_ERR_OK;
        }
        pDbaThreshold->reportMap[i] = *pPtr;
        CTC_BUF_ADD(pPtr, length, CTC_ORGSPEC_DBA_REPORTMAP_LEN);

        /* Calculate the expect length from bitmap */
        expectLen = 0;
        for(j = 0;j < CTC_ORGSPEC_DBA_QUEUE_MAX;j++)
        {
            if(pDbaThreshold->reportMap[i] & (1 << j))
            {
                expectLen += CTC_ORGSPEC_DBA_QUEUETHRESHOLD_LEN;
            }
        }
        if((length < expectLen) || (expectLen == 0))
        {
            /* Insufficient length for parsing
             * or No any queue threshold in the map
             */
            return EPON_OAM_ERR_OK;
        }

        for(j = 0;j < CTC_ORGSPEC_DBA_QUEUE_MAX;j++)
        {
            if(pDbaThreshold->reportMap[i] & (1 << j))
            {
                ((unsigned char *)(&pDbaThreshold->queueSet[i].queueThreshold[j]))[0] = pPtr[0];
                ((unsigned char *)(&pDbaThreshold->queueSet[i].queueThreshold[j]))[1] = pPtr[1];
                CTC_BUF_ADD(pPtr, length, CTC_ORGSPEC_DBA_QUEUETHRESHOLD_LEN);
            }
        }
    }
    *pParseLen = pPtr - pFrame;

    return EPON_OAM_ERR_OK;
}

#if CTC_OAM_SUPPORT_BENEATH_20
/* For CTC version smaller or equal to 2.0 */
static int ctc_oam_orgSpecVar_req20(
    unsigned char llidIdx,      /* LLID index of the incoming operation */
    unsigned char *pFrame,      /* Frame payload current pointer */
    unsigned short length,      /* Frame payload length */
    unsigned char *pReplyBuf,   /* Frame buffer for reply OAM */
    unsigned short bufLen,      /* Frame buffer size */
    unsigned short *pReplyLen)  /* Reply size used by this handler*/
{
    int ret;
    unsigned char *pProcPtr, *pReplyPtr;
    unsigned short genLen;
    ctc_varCb_t varCb;
    ctc_varDescriptor_t varDesc;
    ctc_varInstant_t *pVarInstant = NULL, varInstant;
    ctc_varContainer_t varContainer;
    unsigned short remainLen;

    /* 1. Parse branch/leaf and search database for process function
     * 2. Call process function to preocess the branch/leaf
     */
    pProcPtr = pFrame;
    pReplyPtr = pReplyBuf;
    remainLen = length;
    *pReplyLen = 0;

    while(remainLen >= CTC_ORGSPEC_VARDESC_LEN)
    {
        varDesc.varBranch = *pProcPtr;
        memcpy(&varDesc.varLeaf, &pProcPtr[1], sizeof(unsigned short));
        CTC_BUF_ADD(pProcPtr, remainLen, CTC_ORGSPEC_VARDESC_LEN); /* Variable Descriptor */

        switch(varDesc.varBranch)
        {
        case CTC_VAR_REQBRANCH_INSTANT20:
            /* Parse for variable instant */
            if((remainLen < CTC_ORGSPEC_VARINSTANT_LEN20) &&
               (bufLen > (CTC_ORGSPEC_VARDESC_LEN + CTC_ORGSPEC_VARINSTANT_LEN20)))
            {
                /* Ignore the following variables */
                *pReplyLen = (pReplyPtr - pReplyBuf);
                return EPON_OAM_ERR_OK;
            }
            varInstant.varDesc = varDesc;
            varInstant.varWidth = CTC_ORGSPEC_VARINSTANT_WIDTH20;
            CTC_BUF_ADD(pProcPtr, remainLen, 1); /* Width */
            memcpy(varInstant.varData, pProcPtr, sizeof(unsigned char) * CTC_ORGSPEC_VARINSTANT_WIDTH20);
            CTC_BUF_ADD(pProcPtr, remainLen, sizeof(unsigned char) * CTC_ORGSPEC_VARINSTANT_WIDTH20); /* Data */
            /* Set instant pointer for all remain variable request */
            pVarInstant = &varInstant;
            /* Encode the variable instant into reply packet */
            ctc_oam_orgSpecVarInstant_gen(pVarInstant, pReplyPtr, &genLen);
            CTC_BUF_ADD(pReplyPtr, bufLen, genLen);
            break;
        case CTC_VAR_REQBRANCH_STDATTR:
        case CTC_VAR_REQBRANCH_EXTATTR:
        case CTC_VAR_REQBRANCH_STDACT:
        case CTC_VAR_REQBRANCH_EXTACT:
            ret = ctc_oam_orgSepcVarCb_get(varDesc, &varCb);
            if((EPON_OAM_ERR_OK != ret) ||
               (NULL == varCb.handler_get) ||
               (0 == (varCb.op & CTC_VAR_OP_GET)))
            {
                /* Ignore unsupport branch/leaf */
                break;
            }

            /* Use branch/leaf specific callback function to process */
            varContainer.pVarData = NULL;
            ret = varCb.handler_get(llidIdx, CTC_VAR_OP_GET, pVarInstant, varDesc, &varContainer);
            if(EPON_OAM_ERR_OK == ret)
            {
                if(bufLen >= (CTC_ORGSPEC_VARDESC_LEN + varContainer.varWidth + 1 /* Width */))
                {
                    genLen = 0;
                    ctc_oam_orgSpecVarContainer_gen(
                        &varContainer,
                        pReplyPtr,
                        &genLen);
                    CTC_BUF_ADD(pReplyPtr, bufLen, genLen);
                }
            }

            if(NULL != varContainer.pVarData)
            {
                free(varContainer.pVarData);
            }
            break;
        case CTC_VAR_REQBRANCH_END:
            *pReplyLen = (pReplyPtr - pReplyBuf);
            return EPON_OAM_ERR_OK;
        default:
            break;
        }
    }
    *pReplyLen = (pReplyPtr - pReplyBuf);

    return EPON_OAM_ERR_OK;
}

/* For CTC version smaller or equal to 2.0 */
static int ctc_oam_orgSpecSet_req20(
    unsigned char llidIdx,      /* LLID index of the incoming operation */
    unsigned char *pFrame,      /* Frame payload current pointer */
    unsigned short length,      /* Frame payload length */
    unsigned char *pReplyBuf,   /* Frame buffer for reply OAM */
    unsigned short bufLen,      /* Frame buffer size */
    unsigned short *pReplyLen)  /* Reply size used by this handler*/
{
    int ret;
    unsigned int extFlag;
    unsigned char *pProcPtr, *pReplyPtr;
    unsigned short genLen;
    ctc_varCb_t varCb;
    ctc_varInstant_t *pVarInstant = NULL, varInstant;
    ctc_varContainer_t varContainer;
    unsigned short remainLen;

    /* 1. Parse branch/leaf and search database for process function
     * 2. Call process function to preocess the branch/leaf
     */
    pProcPtr = pFrame;
    pReplyPtr = pReplyBuf;
    remainLen = length;
    *pReplyLen = 0;

    while(remainLen >= CTC_ORGSPEC_VARCONTAINER_MIN)
    {
        varContainer.varDesc.varBranch = *pProcPtr;
        memcpy(&varContainer.varDesc.varLeaf, &pProcPtr[1], sizeof(unsigned short));
        varContainer.varWidth = pProcPtr[3];
        /* Mark the next line, when olt reboot onu, the varContainer have no the member or the varWidth = 0 */
        /* varContainer.varWidth = (0 == varContainer.varWidth) ? 0x80 : varContainer.varWidth; */
        CTC_BUF_ADD(pProcPtr, remainLen, CTC_ORGSPEC_VARCONTAINER_MIN); /* Variable Container */

        switch(varContainer.varDesc.varBranch)
        {
        case CTC_VAR_REQBRANCH_INSTANT20:
            /* Parse for variable instant */
            if((remainLen < varContainer.varWidth) &&
               (bufLen > (CTC_ORGSPEC_VARDESC_LEN + CTC_ORGSPEC_VARINSTANT_LEN20)))
            {
                /* Ignore the following variables */
                *pReplyLen = (pReplyPtr - pReplyBuf);
                return EPON_OAM_ERR_OK;
            }
            varInstant.varDesc = varContainer.varDesc;
            varInstant.varWidth = CTC_ORGSPEC_VARINSTANT_WIDTH20;
            memcpy(varInstant.varData, pProcPtr, sizeof(unsigned char) * CTC_ORGSPEC_VARINSTANT_WIDTH20);
            CTC_BUF_ADD(pProcPtr, remainLen, sizeof(unsigned char) * CTC_ORGSPEC_VARINSTANT_WIDTH20); /* Data */
            /* Set instant pointer for all remain variable request */
            pVarInstant = &varInstant;
            /* Encode the variable instant into reply packet */
            ctc_oam_orgSpecVarInstant_gen(pVarInstant, pReplyPtr, &genLen);
            CTC_BUF_ADD(pReplyPtr, bufLen, genLen);
            break;
        case CTC_VAR_REQBRANCH_STDATTR:
        case CTC_VAR_REQBRANCH_EXTATTR:
        case CTC_VAR_REQBRANCH_STDACT:
        case CTC_VAR_REQBRANCH_EXTACT:
            if(remainLen < varContainer.varWidth)
            {
                /* Insufficient length for parsing
                 * Skip all remain frame
                 */
                *pReplyLen = (pReplyPtr - pReplyBuf);
                return EPON_OAM_ERR_OK;
            }

            ret = ctc_oam_orgSepcVarCb_get(varContainer.varDesc, &varCb);
            if((EPON_OAM_ERR_OK != ret) ||
               (NULL == varCb.handler_set) ||
               (0 == (varCb.op & CTC_VAR_OP_SET)))
            {
                /* Ignore unsupport branch/leaf 
                 * Skip the data field of the frame
                 */
                CTC_BUF_ADD(pProcPtr, remainLen, varContainer.varWidth); /* Data */
                break;
            }

            /* Extract data field from frame */
            if(0 != varContainer.varWidth)
            {
                varContainer.pVarData = (unsigned char *) malloc(varContainer.varWidth);
                if(NULL == varContainer.pVarData)
                {
                    return EPON_OAM_ERR_MEM;
                }
                memcpy(varContainer.pVarData, pProcPtr, varContainer.varWidth);
                CTC_BUF_ADD(pProcPtr, remainLen, varContainer.varWidth); /* Data */
            }
            else
            {
                varContainer.pVarData = NULL;
            }

            epon_oam_dbgExt_get(&extFlag);
            if(extFlag & EPON_OAM_DBGEXT_CTC_VARSET)
            {
                /* Use dummy set to ignore all actual set functions */
                ret = ctc_oam_varCb_dummy_set(llidIdx, CTC_VAR_OP_SET, pVarInstant, varContainer.varDesc, &varContainer);
            }
            else
            {
                /* Use branch/leaf specific callback function to process */
                ret = varCb.handler_set(llidIdx, CTC_VAR_OP_SET, pVarInstant, varContainer.varDesc, &varContainer);
            }
            if(EPON_OAM_ERR_OK == ret)
            {
                if(bufLen >= CTC_ORGSPEC_VARCONTAINER_MIN)
                {
                    /* For set operation, only set result should be encoded */
                    varContainer.varWidth |= CTC_VAR_CONTAINER_INDICATOR;

                    genLen = 0;
                    ctc_oam_orgSpecVarContainer_gen(
                        &varContainer,
                        pReplyPtr,
                        &genLen);
                    CTC_BUF_ADD(pReplyPtr, bufLen, genLen);
                }
            }

            if(NULL != varContainer.pVarData)
            {
                free(varContainer.pVarData);
            }
            break;
        case CTC_VAR_REQBRANCH_END:
            *pReplyLen = (pReplyPtr - pReplyBuf);
            return EPON_OAM_ERR_OK;
        default:
            break;
        }
    }
    *pReplyLen = (pReplyPtr - pReplyBuf);

    return EPON_OAM_ERR_OK;
}
#endif /* CTC_OAM_SUPPORT_BENEATH_20 */

#if CTC_OAM_SUPPORT_ABOVE_21
static int ctc_oam_orgSpecValidInstantLeaf_decode21(
    ctc_varInstant_t *pVarInstant)
{
    unsigned int value;

    switch(pVarInstant->varDesc.varLeaf)
    {
        case CTC_VAR_REQBRANCH_INSTANT_PORT:
            CTC_BUF_PARSE32(pVarInstant->varData, &value);
            pVarInstant->parse.uniPort.portType = (value >> 24) & 0xffUL;
            pVarInstant->parse.uniPort.chassisNo = (value >> 22) & 0x03UL;
            pVarInstant->parse.uniPort.slotNo = (value >> 16) & 0x3fUL;
            pVarInstant->parse.uniPort.portNo = value & 0xffUL;
            /* Valid instant leaf value for CTC 2.1 */
            return 1;
        case CTC_VAR_REQBRANCH_INSTANT_LLID:
            CTC_BUF_PARSE32(pVarInstant->varData, &pVarInstant->parse.llid);
            /* Valid instant leaf value for CTC 2.1 */
            return 1;
        case CTC_VAR_REQBRANCH_INSTANT_PONIF:
            CTC_BUF_PARSE32(pVarInstant->varData, &pVarInstant->parse.ponIf);
            /* Valid instant leaf value for CTC 2.1 */
            return 1;
        case CTC_VAR_REQBRANCH_INSTANT_ONU:
        default:
            return 0;
    }
}

/* For CTC version larger or equal to 2.1 */
static int ctc_oam_orgSpecVar_req21(
    unsigned char llidIdx,      /* LLID index of the incoming operation */
    unsigned char *pFrame,      /* Frame payload current pointer */
    unsigned short length,      /* Frame payload length */
    unsigned char *pReplyBuf,   /* Frame buffer for reply OAM */
    unsigned short bufLen,      /* Frame buffer size */
    unsigned short *pReplyLen)  /* Reply size used by this handler*/
{
    int ret;
    unsigned char *pProcPtr, *pReplyPtr;
    unsigned short genLen;
    ctc_varCb_t varCb;
    ctc_varDescriptor_t varDesc;
    ctc_varInstant_t *pVarInstant = NULL, varInstant;
    ctc_varContainer_t varContainer;
    unsigned short remainLen;

    /* 1. Parse branch/leaf and search database for process function
     * 2. Call process function to preocess the branch/leaf
     */
    pProcPtr = pFrame;
    pReplyPtr = pReplyBuf;
    remainLen = length;
    *pReplyLen = 0;

    while(remainLen >= CTC_ORGSPEC_VARDESC_LEN)
    {
        varDesc.varBranch = *pProcPtr;
        memcpy(&varDesc.varLeaf, &pProcPtr[1], sizeof(unsigned short));
        CTC_BUF_ADD(pProcPtr, remainLen, CTC_ORGSPEC_VARDESC_LEN); /* Variable Descriptor */

        switch(varDesc.varBranch)
        {
        case CTC_VAR_REQBRANCH_INSTANT21:
            /* Parse for variable instant */
            if((remainLen < CTC_ORGSPEC_VARINSTANT_LEN21) &&
               (bufLen > (CTC_ORGSPEC_VARDESC_LEN + CTC_ORGSPEC_VARINSTANT_LEN21)))
            {
                /* Ignore the following variables */
                *pReplyLen = (pReplyPtr - pReplyBuf);
                return EPON_OAM_ERR_OK;
            }
            varInstant.varDesc = varDesc;
            varInstant.varWidth = CTC_ORGSPEC_VARINSTANT_WIDTH21;
            CTC_BUF_ADD(pProcPtr, remainLen, 1); /* Width */
            memcpy(varInstant.varData, pProcPtr, sizeof(unsigned char) * CTC_ORGSPEC_VARINSTANT_WIDTH21);
            CTC_BUF_ADD(pProcPtr, remainLen, sizeof(unsigned char) * CTC_ORGSPEC_VARINSTANT_WIDTH21); /* Data */
            if(ctc_oam_orgSpecValidInstantLeaf_decode21(&varInstant))
            {
                /* Set instant pointer for all remain variable request */
                pVarInstant = &varInstant;
                /* Encode the variable instant into reply packet */
                ctc_oam_orgSpecVarInstant_gen(pVarInstant, pReplyPtr, &genLen);
                CTC_BUF_ADD(pReplyPtr, bufLen, genLen);
            }
            else
            {
                /* Ignore the invalid variable instant */
                pVarInstant = NULL;
            }
            break;
        case CTC_VAR_REQBRANCH_STDATTR:
        case CTC_VAR_REQBRANCH_EXTATTR:
        case CTC_VAR_REQBRANCH_STDACT:
        case CTC_VAR_REQBRANCH_EXTACT:
            EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_INFO,
                "[OAM:%s:%d] var get 0x%02x/0x%04x\n", __FILE__, __LINE__, varDesc.varBranch, varDesc.varLeaf);
            
            ret = ctc_oam_orgSepcVarCb_get(varDesc, &varCb);
            if((EPON_OAM_ERR_OK != ret) ||
               (NULL == varCb.handler_get) ||
               (0 == (varCb.op & CTC_VAR_OP_GET)))
            {
                /* Ignore unsupport branch/leaf */
                break;
            }

            /* Use branch/leaf specific callback function to process */
            varContainer.pVarData = NULL;
            ret = varCb.handler_get(llidIdx, CTC_VAR_OP_GET, pVarInstant, varDesc, &varContainer);
            if(EPON_OAM_ERR_OK == ret)
            {
                if(bufLen >= (CTC_ORGSPEC_VARDESC_LEN + varContainer.varWidth + 1 /* Width */))
                {
                    genLen = 0;
                    ctc_oam_orgSpecVarContainer_gen(
                        &varContainer,
                        pReplyPtr,
                        &genLen);
                    CTC_BUF_ADD(pReplyPtr, bufLen, genLen);
                }
            }

            if(NULL != varContainer.pVarData)
            {
                free(varContainer.pVarData);
            }
            break;
        case CTC_VAR_REQBRANCH_END:
            *pReplyLen = (pReplyPtr - pReplyBuf);
            return EPON_OAM_ERR_OK;
        default:
            break;
        }
    }
    *pReplyLen = (pReplyPtr - pReplyBuf);

    return EPON_OAM_ERR_OK;
}

/* For CTC version larger or equal to 2.1 */
static int ctc_oam_orgSpecSet_req21(
    unsigned char llidIdx,      /* LLID index of the incoming operation */
    unsigned char *pFrame,      /* Frame payload current pointer */
    unsigned short length,      /* Frame payload length */
    unsigned char *pReplyBuf,   /* Frame buffer for reply OAM */
    unsigned short bufLen,      /* Frame buffer size */
    unsigned short *pReplyLen)  /* Reply size used by this handler*/
{
    int ret;
    unsigned int extFlag;
    unsigned char *pProcPtr, *pReplyPtr;
    unsigned short genLen;
    ctc_varCb_t varCb;
    ctc_varInstant_t *pVarInstant = NULL, varInstant;
    ctc_varContainer_t varContainer;
    unsigned short remainLen;

    /* 1. Parse branch/leaf and search database for process function
     * 2. Call process function to preocess the branch/leaf
     */
    pProcPtr = pFrame;
    pReplyPtr = pReplyBuf;
    remainLen = length;
    *pReplyLen = 0;

    while(remainLen >= CTC_ORGSPEC_VARCONTAINER_MIN)
    {
        varContainer.varDesc.varBranch = *pProcPtr;
        memcpy(&varContainer.varDesc.varLeaf, &pProcPtr[1], sizeof(unsigned short));
        varContainer.varWidth = pProcPtr[3];
        /* Mark the next line, when olt reboot onu, the varContainer have no the member or the varWidth = 0 */
        /* varContainer.varWidth = (0 == varContainer.varWidth) ? 0x80 : varContainer.varWidth; */
        CTC_BUF_ADD(pProcPtr, remainLen, CTC_ORGSPEC_VARCONTAINER_MIN); /* Variable Container */

        switch(varContainer.varDesc.varBranch)
        {
        case CTC_VAR_REQBRANCH_INSTANT21:
            /* Parse for variable instant */
            if((remainLen < varContainer.varWidth) &&
               (bufLen > (CTC_ORGSPEC_VARDESC_LEN + CTC_ORGSPEC_VARINSTANT_LEN21)))
            {
                /* Ignore the following variables */
                *pReplyLen = (pReplyPtr - pReplyBuf);
                return EPON_OAM_ERR_OK;
            }
            varInstant.varDesc = varContainer.varDesc;
            varInstant.varWidth = CTC_ORGSPEC_VARINSTANT_WIDTH21;
            memcpy(varInstant.varData, pProcPtr, sizeof(unsigned char) * CTC_ORGSPEC_VARINSTANT_WIDTH21);
            CTC_BUF_ADD(pProcPtr, remainLen, sizeof(unsigned char) * CTC_ORGSPEC_VARINSTANT_WIDTH21); /* Data */
            if(ctc_oam_orgSpecValidInstantLeaf_decode21(&varInstant))
            {
                /* Set instant pointer for all remain variable request */
                pVarInstant = &varInstant;
                /* Encode the variable instant into reply packet */
                ctc_oam_orgSpecVarInstant_gen(pVarInstant, pReplyPtr, &genLen);
                CTC_BUF_ADD(pReplyPtr, bufLen, genLen);
            }
            else
            {
                /* Ignore the invalid variable instant */
                pVarInstant = NULL;
            }
            break;
        case CTC_VAR_REQBRANCH_STDATTR:
        case CTC_VAR_REQBRANCH_EXTATTR:
        case CTC_VAR_REQBRANCH_STDACT:
        case CTC_VAR_REQBRANCH_EXTACT:
            if(remainLen < varContainer.varWidth)
            {
                /* Insufficient length for parsing
                 * Skip all remain frame
                 */
                *pReplyLen = (pReplyPtr - pReplyBuf);
                return EPON_OAM_ERR_OK;
            }

            EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_INFO,
                "[OAM:%s:%d] var set 0x%02x/0x%04x\n", __FILE__, __LINE__, varContainer.varDesc.varBranch, varContainer.varDesc.varLeaf);

            ret = ctc_oam_orgSepcVarCb_get(varContainer.varDesc, &varCb);
            if((EPON_OAM_ERR_OK != ret) ||
               (NULL == varCb.handler_set) ||
               (0 == (varCb.op & CTC_VAR_OP_SET)))
            {
                /* Ignore unsupport branch/leaf 
                 * Skip the data field of the frame
                 */
                CTC_BUF_ADD(pProcPtr, remainLen, varContainer.varWidth); /* Data */
                break;
            }

            /* Extract data field from frame */
            if(0 != varContainer.varWidth)
            {
                varContainer.pVarData = (unsigned char *) malloc(varContainer.varWidth);
                if(NULL == varContainer.pVarData)
                {
                    return EPON_OAM_ERR_MEM;
                }
                memcpy(varContainer.pVarData, pProcPtr, varContainer.varWidth);
                CTC_BUF_ADD(pProcPtr, remainLen, varContainer.varWidth); /* Data */
            }
            else
            {
                varContainer.pVarData = NULL;
            }

            epon_oam_dbgExt_get(&extFlag);
            if(extFlag & EPON_OAM_DBGEXT_CTC_VARSET)
            {
                /* Use dummy set to ignore all actual set functions */
                ret = ctc_oam_varCb_dummy_set(llidIdx, CTC_VAR_OP_SET, pVarInstant, varContainer.varDesc, &varContainer);
            }
            else
            {
                /* Use branch/leaf specific callback function to process */
                ret = varCb.handler_set(llidIdx, CTC_VAR_OP_SET, pVarInstant, varContainer.varDesc, &varContainer);
            }
            if(EPON_OAM_ERR_OK == ret)
            {
                if(bufLen >= CTC_ORGSPEC_VARCONTAINER_MIN)
                {
                    /* For set operation, only set result should be encoded */
                    varContainer.varWidth |= CTC_VAR_CONTAINER_INDICATOR;

                    genLen = 0;
                    ctc_oam_orgSpecVarContainer_gen(
                        &varContainer,
                        pReplyPtr,
                        &genLen);
                    CTC_BUF_ADD(pReplyPtr, bufLen, genLen);
                }
            }

            if(NULL != varContainer.pVarData)
            {
                free(varContainer.pVarData);
            }
            break;
        case CTC_VAR_REQBRANCH_END:
            *pReplyLen = (pReplyPtr - pReplyBuf);
            return EPON_OAM_ERR_OK;
        default:
            break;
        }
    }
    *pReplyLen = (pReplyPtr - pReplyBuf);

    return EPON_OAM_ERR_OK;
}
#endif /* CTC_OAM_SUPPORT_ABOVE_21 */

static int ctc_oam_orgSpecOnuAuth(
    unsigned char llidIdx,      /* LLID index of the incoming operation */
    unsigned char *pFrame,      /* Frame payload current pointer */
    unsigned short length,      /* Frame payload length */
    unsigned char *pReplyBuf,   /* Frame buffer for reply OAM */
    unsigned short bufLen,      /* Frame buffer size */
    unsigned short *pReplyLen)  /* Reply size used by this handler*/
{
    int ret;
    unsigned char *pProcPtr, *pReplyPtr;
    unsigned short genLen;
    unsigned short remainLen;
    unsigned char authCode;
    unsigned char authType;
    unsigned char authFailType;
    unsigned short authDataLen;
    ctc_onuAuthLoid_t loidAuth;

    /* 1. Parse churning header
     * 2. Process CTC extended OAMPDU according to churning op code
     */
    pProcPtr = pFrame;
    pReplyPtr = pReplyBuf;
    remainLen = length;
    *pReplyLen = 0;

    if(remainLen < CTC_ORGSPEC_ONUAUTH_HDR_LEN)
    {
        /* Insufficient length for parsing */
        return EPON_OAM_ERR_OK;
    }

    /* Parse common header for ONU auth extended opcode */
    authCode = pProcPtr[0];
    ((unsigned char *)&authDataLen)[0] = pProcPtr[1];
    ((unsigned char *)&authDataLen)[1] = pProcPtr[2];
    CTC_BUF_ADD(pProcPtr, remainLen, CTC_ORGSPEC_ONUAUTH_HDR_LEN);

    if(remainLen < authDataLen)
    {
        /* Insufficient length for parsing */
        return EPON_OAM_ERR_OK;
    }

    switch(authCode)
    {
    case CTC_ORGSPEC_ONUAUTH_CODE_REQ:
        authType = *pProcPtr;
        switch(authType)
        {
        case CTC_ORGSPEC_ONUAUTH_LOID:
            /* Reply the LOID + password */
            ret = ctc_oam_onuAuthLoid_get(llidIdx, &loidAuth);
            if(EPON_OAM_ERR_OK != ret ||
              (bufLen < CTC_ORGSPEC_ONUAUTH_HDR_LEN + CTC_ORGSPEC_ONUAUTH_RESP_LOID_LEN))
            {
                /* Get failed/insufficient length, don't reply to OLT */
                return EPON_OAM_ERR_ORG;
            }
            ctc_oam_orgSpecOnuAuthLoid_gen(&loidAuth, pReplyPtr, &genLen);
            CTC_BUF_ADD(pReplyPtr, bufLen, genLen);
            break;
        default:
            /* Unsupported AUTH type */
            /* Reply the NAK */
            if(bufLen < CTC_ORGSPEC_ONUAUTH_HDR_LEN + CTC_ORGSPEC_ONUAUTH_RESP_NAK_LEN)
            {
                /* Insufficient length, don't reply to OLT */
                return EPON_OAM_ERR_ORG;
            }
            /* TODO - Only support LOID + password currently, add new auth type */
            ctc_oam_orgSpecOnuAuthNak_gen(CTC_ORGSPEC_ONUAUTH_LOID, pReplyPtr, &genLen);
            CTC_BUF_ADD(pReplyPtr, bufLen, genLen);
            break;
        }
        break;
    case CTC_ORGSPEC_ONUAUTH_CODE_SUCC:
        ret = ctc_oam_onuAuthState_set(llidIdx, CTC_OAM_ONUAUTH_STATE_SUCC, 0);
        if(EPON_OAM_ERR_OK != ret)
        {
            /* Set failed */
            EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_ERROR,
                "[OAM:%s:%d] Fail to set auth state %u\n", __FILE__, __LINE__, ret);
            return EPON_OAM_ERR_OK;
        }
        break;
    case CTC_ORGSPEC_ONUAUTH_CODE_FAIL:
        authFailType = *pProcPtr;
        ret = ctc_oam_onuAuthState_set(llidIdx, CTC_OAM_ONUAUTH_STATE_FAIL, authFailType);
        if(EPON_OAM_ERR_OK != ret)
        {
            /* Set failed */
            EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_ERROR,
                "[OAM:%s:%d] Fail to set auth state %u\n", __FILE__, __LINE__, ret);
            return EPON_OAM_ERR_OK;
        }
        break;
    case CTC_ORGSPEC_ONUAUTH_CODE_RESP:
        /* Only ONU can send this, so there might be some error to receive this */
    default:
        return EPON_OAM_ERR_ORG;
    }
    *pReplyLen = (pReplyPtr - pReplyBuf);

    return EPON_OAM_ERR_OK;
}

static int ctc_oam_orgSpecSwDownload(
    unsigned char llidIdx,      /* LLID index of the incoming operation */
    unsigned char *pFrame,      /* Frame payload current pointer */
    unsigned short length,      /* Frame payload length */
    unsigned char *pReplyBuf,   /* Frame buffer for reply OAM */
    unsigned short bufLen,      /* Frame buffer size */
    unsigned short *pReplyLen)  /* Reply size used by this handler*/
{
    int ret;
    unsigned int checkSize;
    unsigned char *pProcPtr, *pReplyPtr;
    unsigned char activeFlag, commitFlag;
    unsigned short genLen;
    unsigned short remainLen;
    ctc_swDownload_t inSwDownload, outSwDownload;
    unsigned int strLen, dataLen;
    FILE *pFp;

    /* 1. Parse software download header
     * 2. Process CTC extended OAMPDU according to software download data type
     */
    pProcPtr = pFrame;
    pReplyPtr = pReplyBuf;
    remainLen = length;
    *pReplyLen = 0;

    if(remainLen < CTC_ORGSPEC_SWDOWNLOAD_HDR_LEN)
    {
        /* Insufficient length for parsing */
        return EPON_OAM_ERR_OK;
    }

    /* Parse common header for churning extended opcode */
    inSwDownload.dataType = pProcPtr[0];
    ((unsigned char *)&inSwDownload.length)[0] = pProcPtr[1];
    ((unsigned char *)&inSwDownload.length)[1] = pProcPtr[2];
    ((unsigned char *)&inSwDownload.tid)[0] = pProcPtr[3];
    ((unsigned char *)&inSwDownload.tid)[1] = pProcPtr[4];
    ((unsigned char *)&inSwDownload.opCode)[0] = pProcPtr[5];
    ((unsigned char *)&inSwDownload.opCode)[1] = pProcPtr[6];
    CTC_BUF_ADD(pProcPtr, remainLen, CTC_ORGSPEC_SWDOWNLOAD_HDR_LEN);

    EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_INFO,
        "[OAM:%s:%d] software download, opcode %d\n", __FILE__, __LINE__, inSwDownload.opCode);

    switch(inSwDownload.dataType)
    {
    case CTC_ORGSPEC_SWDL_TYPE_FILE:
        switch(inSwDownload.opCode)
        {
        case CTC_ORGSPEC_SWDL_OPCODE_FILEREQ:
            /* Retrive file name */
            strLen = strlen(pProcPtr);
            if(strLen >= remainLen)
            {
                EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_INFO, "[OAM:%s:%d]\n", __FILE__, __LINE__);
                return EPON_OAM_ERR_PARSE;
            }

            if(CTC_SWDL_STATE_GET() != CTC_OAM_SWDOWNLOAD_BUF_CLEAR)
            {
                /* Woops? Buffer is not empty? */
                ctc_oam_swDownloadBuf_clear();
                EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_WARN,
                    "[OAM:%s:%d] Clear software download buffer\n", __FILE__, __LINE__);
            }

            sem_wait(&ctcDataSem);
            ctc_swdl_file.fileName = (char *)malloc((strLen + 1) * sizeof(char));
            sem_post(&ctcDataSem);
            if(ctc_swdl_file.fileName == NULL)
            {
                ctc_oam_swDownloadError_alloc(inSwDownload, &outSwDownload, CTC_ORGSPEC_SWDL_ERROR_MEMORY, NULL);
                EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_ERROR, "[OAM:%s:%d]\n", __FILE__, __LINE__);
                break;
            }

            memcpy(ctc_swdl_file.fileName, pProcPtr, (strLen + 1) * sizeof(char));
            CTC_BUF_ADD(pProcPtr, remainLen, (strLen + 1));

            /* Retrive mode */
            strLen = strlen(pProcPtr);
            if(strLen > remainLen)
            {
                EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_INFO, "[OAM:%s:%d]\n", __FILE__, __LINE__);
                return EPON_OAM_ERR_PARSE;
            }
            
            sem_wait(&ctcDataSem);
            ctc_swdl_file.mode = (char *)malloc((strLen + 1) * sizeof(char));
            sem_post(&ctcDataSem);
            if(ctc_swdl_file.mode == NULL)
            {
                free(ctc_swdl_file.fileName);
                sem_wait(&ctcDataSem);
                ctc_swdl_file.fileName = NULL;
                sem_post(&ctcDataSem);
                ctc_oam_swDownloadError_alloc(inSwDownload, &outSwDownload, CTC_ORGSPEC_SWDL_ERROR_MEMORY, NULL);
                EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_ERROR, "[OAM:%s:%d]\n", __FILE__, __LINE__);
                break;
            }

            memcpy(ctc_swdl_file.mode, pProcPtr, (strLen + 1) * sizeof(char));
            CTC_BUF_ADD(pProcPtr, remainLen, (strLen + 1));

            if(CTC_ORGSPEC_SWDOWNLOAD_HDR_LEN + CTC_ORGSPEC_SWDOWNLOAD_FILEACK_LEN > bufLen)
            {
                /* Insufficient length, don't reply to OLT */
                EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_ERROR, "[OAM:%s:%d]\n", __FILE__, __LINE__);
                return EPON_OAM_ERR_ORG;
            }

            EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_INFO,
                "[OAM:%s:%d] File write request received, file name %s, file mode %s\n",
                __FILE__, __LINE__, ctc_swdl_file.fileName, ctc_swdl_file.mode);

            CTC_SWDL_STATE_SET(CTC_OAM_SWDOWNLOAD_BUF_FILEREQ);

            /* File write request complete, generate the file transfer ACK */
            outSwDownload.dataType = CTC_ORGSPEC_SWDL_TYPE_FILE;
            outSwDownload.opCode = CTC_ORGSPEC_SWDL_OPCODE_FILEACK;
            outSwDownload.length = CTC_ORGSPEC_SWDOWNLOAD_HDR_LEN + CTC_ORGSPEC_SWDOWNLOAD_FILEACK_LEN;
            outSwDownload.tid = inSwDownload.tid;
            outSwDownload.parse.fileAck.block = 0;
            break;
        case CTC_ORGSPEC_SWDL_OPCODE_FILEDATA:
            /* Retrive block # */
            if(remainLen < CTC_ORGSPEC_SWDOWNLOAD_FILEDATA_LEN)
            {
                return EPON_OAM_ERR_PARSE;
            }
            ((unsigned char *)&inSwDownload.parse.fileData.block)[0] = pProcPtr[0];
            ((unsigned char *)&inSwDownload.parse.fileData.block)[1] = pProcPtr[1];
            CTC_BUF_ADD(pProcPtr, remainLen, CTC_ORGSPEC_SWDOWNLOAD_FILEDATA_LEN);
            
            EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_INFO,
                "[OAM:%s:%d] block # %d, cur # %d\n",
                __FILE__, __LINE__, inSwDownload.parse.fileData.block, ctc_swdl_file.block);
            if(inSwDownload.parse.fileData.block <= ctc_swdl_file.block)
            {
                /* OLT resend? Ack the block again */
                outSwDownload.dataType = CTC_ORGSPEC_SWDL_TYPE_FILE;
                outSwDownload.opCode = CTC_ORGSPEC_SWDL_OPCODE_FILEACK;
                outSwDownload.length = CTC_ORGSPEC_SWDOWNLOAD_HDR_LEN + CTC_ORGSPEC_SWDOWNLOAD_FILEACK_LEN;
                outSwDownload.tid = inSwDownload.tid;
                outSwDownload.parse.fileAck.block = inSwDownload.parse.fileData.block;
                EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_INFO, "[OAM:%s:%d] resend ack for block # %d\n", __FILE__, __LINE__, inSwDownload.parse.fileData.block);
                break;
            }

            if(inSwDownload.parse.fileData.block != (ctc_swdl_file.block + 1))
            {
                /* block number not continue */
                ctc_oam_swDownloadError_alloc(inSwDownload, &outSwDownload, CTC_ORGSPEC_SWDL_ERROR_ILLEGALOAM, NULL);
                EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_ERROR, "[OAM:%s:%d] block # %d\n", __FILE__, __LINE__, inSwDownload.parse.fileData.block);
                break;
            }
            if((CTC_SWDL_STATE_GET() != CTC_OAM_SWDOWNLOAD_BUF_FILEREQ) &&
               (CTC_SWDL_STATE_GET() != CTC_OAM_SWDOWNLOAD_BUF_FILEDATA))
            {
                /* Incorrect buffer state */
                ctc_oam_swDownloadError_alloc(inSwDownload, &outSwDownload, CTC_ORGSPEC_SWDL_ERROR_ILLEGALOAM, NULL);
                EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_ERROR, "[OAM:%s:%d]\n", __FILE__, __LINE__);
                break;
            }
            CTC_SWDL_STATE_SET(CTC_OAM_SWDOWNLOAD_BUF_FILEDATA);

            dataLen = inSwDownload.length - 
                CTC_ORGSPEC_SWDOWNLOAD_HDR_LEN - 
                CTC_ORGSPEC_SWDOWNLOAD_FILEDATA_LEN;

            if(dataLen > remainLen)
            {
                /* Insufficient length for parsing */
                EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_INFO, "[OAM:%s:%d]\n", __FILE__, __LINE__);
                return EPON_OAM_ERR_OK;
            }

            if(dataLen > CTC_ORGSPEC_SWDOWNLOAD_BLOCK_LEN)
            {
                CTC_SWDL_STATE_SET(CTC_OAM_SWDOWNLOAD_BUF_DATAEND);
                ctc_oam_swDownloadError_alloc(inSwDownload, &outSwDownload, CTC_ORGSPEC_SWDL_ERROR_ILLEGALOAM, NULL);
                EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_ERROR, "[OAM:%s:%d]\n", __FILE__, __LINE__);
                break;
            }

            pFp = fopen(CTC_ORGSPEC_SWDL_STORAGE, "ab");
            if(pFp == NULL)
            {
                ctc_oam_swDownloadError_alloc(inSwDownload, &outSwDownload, CTC_ORGSPEC_SWDL_ERROR_MEMORY, NULL);
                EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_ERROR, "[OAM:%s:%d]\n", __FILE__, __LINE__);
                break;
            }
            
            ret = fwrite(pProcPtr, sizeof(char), dataLen, pFp);
            if(ret != dataLen)
            {
                ctc_oam_swDownloadError_alloc(inSwDownload, &outSwDownload, CTC_ORGSPEC_SWDL_ERROR_MEMORY, NULL);
                EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_ERROR, "[OAM:%s:%d]\n", __FILE__, __LINE__);
                fclose(pFp);
                break;
            }
            fclose(pFp);
            CTC_BUF_ADD(pProcPtr, remainLen, dataLen);
            sem_wait(&ctcDataSem);
            ctc_swdl_file.fileSize += dataLen;
            ctc_swdl_file.block ++;
            sem_post(&ctcDataSem);
            if(dataLen != 1400)
            {
                CTC_SWDL_STATE_SET(CTC_OAM_SWDOWNLOAD_BUF_DATAEND);
                EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_INFO,
                    "[OAM:%s:%d] File end receive, total size %d, block count %d\n",
                    __FILE__, __LINE__, ctc_swdl_file.fileSize, ctc_swdl_file.block);
            }
            else
            {
                EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_INFO,
                    "[OAM:%s:%d] File partial receive, total size %d, block count %d\n",
                    __FILE__, __LINE__, ctc_swdl_file.fileSize, ctc_swdl_file.block);
            }

            /* Ack the block */
            outSwDownload.dataType = CTC_ORGSPEC_SWDL_TYPE_FILE;
            outSwDownload.opCode = CTC_ORGSPEC_SWDL_OPCODE_FILEACK;
            outSwDownload.length = CTC_ORGSPEC_SWDOWNLOAD_HDR_LEN + CTC_ORGSPEC_SWDOWNLOAD_FILEACK_LEN;
            outSwDownload.tid = inSwDownload.tid;
            outSwDownload.parse.fileAck.block = ctc_swdl_file.block;
            break;
        case CTC_ORGSPEC_SWDL_OPCODE_FILEACK:
        case CTC_ORGSPEC_SWDL_OPCODE_ERROR:
            /* Only ONU can send this, so there might be some error to receive this */
        default:
            return EPON_OAM_ERR_ORG;
        }
        break;
    case CTC_ORGSPEC_SWDL_TYPE_END:
        switch(inSwDownload.opCode)
        {
        case CTC_ORGSPEC_SWDL_OPCODE_ENDREQ:
            /* Retrive block # */
            if(remainLen < CTC_ORGSPEC_SWDOWNLOAD_ENDREQ_LEN)
            {
                return EPON_OAM_ERR_PARSE;
            }
            ((unsigned char *)&checkSize)[0] = pProcPtr[0];
            ((unsigned char *)&checkSize)[1] = pProcPtr[1];
            ((unsigned char *)&checkSize)[2] = pProcPtr[2];
            ((unsigned char *)&checkSize)[3] = pProcPtr[3];
            CTC_BUF_ADD(pProcPtr, remainLen, CTC_ORGSPEC_SWDOWNLOAD_ENDREQ_LEN);

            switch(CTC_SWDL_STATE_GET())
            {
            case CTC_OAM_SWDOWNLOAD_BUF_DATAEND:
                sem_wait(&ctcDataSem);
                ctc_swdl_file.checkSize = checkSize;
                sem_post(&ctcDataSem);

                if(ctc_swdl_file.checkSize != ctc_swdl_file.fileSize)
                {
                    outSwDownload.dataType = CTC_ORGSPEC_SWDL_TYPE_END;
                    outSwDownload.opCode = CTC_ORGSPEC_SWDL_OPCODE_ENDRESP;
                    outSwDownload.length = CTC_ORGSPEC_SWDOWNLOAD_HDR_LEN + CTC_ORGSPEC_SWDOWNLOAD_ENDRESP_LEN;
                    outSwDownload.tid = inSwDownload.tid;
                    outSwDownload.parse.endResp.respCode = CTC_ORGSPEC_SWDL_RPSCODE_CHECKFAIL;
                    EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_ERROR,
                        "[OAM:%s:%d] File check failed, %d != %d \n", __FILE__, __LINE__, ctc_swdl_file.fileSize, ctc_swdl_file.checkSize);
                    break;
                }
                else
                {
                    EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_INFO,
                        "[OAM:%s:%d] File check passed, check size %d\n", __FILE__, __LINE__, ctc_swdl_file.checkSize);
                }
                if(bootInfo.commit != -1)
                {
                    CTC_SWDL_STATE_SET(CTC_OAM_SWDOWNLOAD_BUF_WRITEING);
                    /* Create thread for software download */
                    ret = pthread_create(&swDownload, NULL, &ctc_oam_swDlThread, NULL);
                    if(ret != 0)
                    {
                        outSwDownload.dataType = CTC_ORGSPEC_SWDL_TYPE_END;
                        outSwDownload.opCode = CTC_ORGSPEC_SWDL_OPCODE_ENDRESP;
                        outSwDownload.length = CTC_ORGSPEC_SWDOWNLOAD_HDR_LEN + CTC_ORGSPEC_SWDOWNLOAD_ENDRESP_LEN;
                        outSwDownload.tid = inSwDownload.tid;
                        outSwDownload.parse.endResp.respCode = CTC_ORGSPEC_SWDL_RPSCODE_CHECKFAIL;
                        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_ERROR,
                            "[OAM:%s:%d] download thread start fail %d %d\n", __FILE__, __LINE__, ret, errno);
                        break;
                    }
                }
                else
                {
                    outSwDownload.dataType = CTC_ORGSPEC_SWDL_TYPE_END;
                    outSwDownload.opCode = CTC_ORGSPEC_SWDL_OPCODE_ENDRESP;
                    outSwDownload.length = CTC_ORGSPEC_SWDOWNLOAD_HDR_LEN + CTC_ORGSPEC_SWDOWNLOAD_ENDRESP_LEN;
                    outSwDownload.tid = inSwDownload.tid;
                    outSwDownload.parse.endResp.respCode = CTC_ORGSPEC_SWDL_RPSCODE_NOTSUPPORT;
                    EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_ERROR,
                        "[OAM:%s:%d] loader not support dual image\n", __FILE__, __LINE__);
                    break;
                }
                /* Fall through */
            case CTC_OAM_SWDOWNLOAD_BUF_WRITEING:
                /* Programming the flash by the other thread 
                 * Reply and wait for complete
                 */
                outSwDownload.dataType = CTC_ORGSPEC_SWDL_TYPE_END;
                outSwDownload.opCode = CTC_ORGSPEC_SWDL_OPCODE_ENDRESP;
                outSwDownload.length = CTC_ORGSPEC_SWDOWNLOAD_HDR_LEN + CTC_ORGSPEC_SWDOWNLOAD_ENDRESP_LEN;
                outSwDownload.tid = inSwDownload.tid;
                outSwDownload.parse.endResp.respCode = CTC_ORGSPEC_SWDL_RPSCODE_PROGRAMING;
                break;
            case CTC_OAM_SWDOWNLOAD_BUF_WRITEOK:
                outSwDownload.dataType = CTC_ORGSPEC_SWDL_TYPE_END;
                outSwDownload.opCode = CTC_ORGSPEC_SWDL_OPCODE_ENDRESP;
                outSwDownload.length = CTC_ORGSPEC_SWDOWNLOAD_HDR_LEN + CTC_ORGSPEC_SWDOWNLOAD_ENDRESP_LEN;
                outSwDownload.tid = inSwDownload.tid;
                outSwDownload.parse.endResp.respCode = CTC_ORGSPEC_SWDL_RPSCODE_COMPLETE;
                break;
            default:
                return EPON_OAM_ERR_ORG;
            }
            break;
        case CTC_ORGSPEC_SWDL_OPCODE_ENDRESP:
            /* Only ONU can send this, so there might be some error to receive this */
        default:
            return EPON_OAM_ERR_ORG;
        }
        break;
    case CTC_ORGSPEC_SWDL_TYPE_ACTIVATE:
        switch(inSwDownload.opCode)
        {
        case CTC_ORGSPEC_SWDL_OPCODE_ACTREQ:
            /* Retrive activate flag */
            if(remainLen < CTC_ORGSPEC_SWDOWNLOAD_ACTREQ_LEN)
            {
                return EPON_OAM_ERR_PARSE;
            }
            activeFlag = pProcPtr[0];
            CTC_BUF_ADD(pProcPtr, remainLen, CTC_ORGSPEC_SWDOWNLOAD_ACTREQ_LEN);

            if(0 != activeFlag)
            {
                outSwDownload.dataType = CTC_ORGSPEC_SWDL_TYPE_ACTIVATE;
                outSwDownload.opCode = CTC_ORGSPEC_SWDL_OPCODE_ACTRESP;
                outSwDownload.length = CTC_ORGSPEC_SWDOWNLOAD_HDR_LEN + CTC_ORGSPEC_SWDOWNLOAD_ACTRESP_LEN;
                outSwDownload.tid = inSwDownload.tid;
                outSwDownload.parse.activateResp.ack = CTC_ORGSPEC_SWDL_ACTACK_PARAMETER;
                EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_ERROR,
                    "[OAM:%s:%d] activate oam parameter incorrect\n", __FILE__, __LINE__);
                break;
            }

            if(CTC_OAM_SWDOWNLOAD_BUF_WRITEOK == CTC_SWDL_STATE_GET())
            {
                if(bootInfo.active != -1)
                {
                    ret = ctc_oam_swDownloadNv_set("sw_tryactive", (bootInfo.active == 0) ? "1" : "0");
                    if(ret != 0)
                    {
                        outSwDownload.dataType = CTC_ORGSPEC_SWDL_TYPE_ACTIVATE;
                        outSwDownload.opCode = CTC_ORGSPEC_SWDL_OPCODE_ACTRESP;
                        outSwDownload.length = CTC_ORGSPEC_SWDOWNLOAD_HDR_LEN + CTC_ORGSPEC_SWDOWNLOAD_ACTRESP_LEN;
                        outSwDownload.tid = inSwDownload.tid;
                        outSwDownload.parse.activateResp.ack = CTC_ORGSPEC_SWDL_ACTACK_FAIL;
                        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_ERROR,
                            "[OAM:%s:%d] active img #%d fail\n", __FILE__, __LINE__, (bootInfo.commit == 0) ? 1 : 0);
                        break;
                    }
                }
                else
                {
                    outSwDownload.dataType = CTC_ORGSPEC_SWDL_TYPE_ACTIVATE;
                    outSwDownload.opCode = CTC_ORGSPEC_SWDL_OPCODE_ACTRESP;
                    outSwDownload.length = CTC_ORGSPEC_SWDOWNLOAD_HDR_LEN + CTC_ORGSPEC_SWDOWNLOAD_ACTRESP_LEN;
                    outSwDownload.tid = inSwDownload.tid;
                    outSwDownload.parse.activateResp.ack = CTC_ORGSPEC_SWDL_ACTACK_NOTSUPPORT;
                    EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_ERROR,
                        "[OAM:%s:%d] loader not support dual image\n", __FILE__, __LINE__);
                    break;
                }
            }
            outSwDownload.dataType = CTC_ORGSPEC_SWDL_TYPE_ACTIVATE;
            outSwDownload.opCode = CTC_ORGSPEC_SWDL_OPCODE_ACTRESP;
            outSwDownload.length = CTC_ORGSPEC_SWDOWNLOAD_HDR_LEN + CTC_ORGSPEC_SWDOWNLOAD_ACTRESP_LEN;
            outSwDownload.tid = inSwDownload.tid;
            outSwDownload.parse.activateResp.ack = CTC_ORGSPEC_SWDL_ACTACK_SUCCESS;
            EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_INFO,
                "[OAM:%s:%d] activate img #%d\n", __FILE__, __LINE__, (bootInfo.commit == 0) ? 1 : 0);

            /* Trigger reboot process */
            ret = pthread_create(&swReboot, NULL, &ctc_oam_rebootThread, NULL);
            if(ret != 0)
            {
                swReboot = 0;
                EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_ERROR,
                    "[OAM:%s:%d] trigger reboot process fail\n", __FILE__, __LINE__);
                break;
            }
            break;
        case CTC_ORGSPEC_SWDL_OPCODE_ACTRESP:
            /* Only ONU can send this, so there might be some error to receive this */
        default:
            return EPON_OAM_ERR_ORG;
        }
        break;
    case CTC_ORGSPEC_SWDL_TYPE_COMMIT:
        switch(inSwDownload.opCode)
        {
        case CTC_ORGSPEC_SWDL_OPCODE_COMMITREQ:
            /* Retrive commit flag */
            if(remainLen < CTC_ORGSPEC_SWDOWNLOAD_CMTREQ_LEN)
            {
                return EPON_OAM_ERR_PARSE;
            }
            commitFlag = pProcPtr[0];
            CTC_BUF_ADD(pProcPtr, remainLen, CTC_ORGSPEC_SWDOWNLOAD_CMTREQ_LEN);

            if(0 != commitFlag)
            {
                outSwDownload.dataType = CTC_ORGSPEC_SWDL_TYPE_COMMIT;
                outSwDownload.opCode = CTC_ORGSPEC_SWDL_OPCODE_COMMITRESP;
                outSwDownload.length = CTC_ORGSPEC_SWDOWNLOAD_HDR_LEN + CTC_ORGSPEC_SWDOWNLOAD_CMTRESP_LEN;
                outSwDownload.tid = inSwDownload.tid;
                outSwDownload.parse.commitResp.ack = CTC_ORGSPEC_SWDL_CMTACK_PARAMETER;
                EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_ERROR,
                    "[OAM:%s:%d] commit oam parameter incorrect\n", __FILE__, __LINE__);
                break;
            }

            if(bootInfo.commit != -1)
            {
                ret = ctc_oam_swDownloadNv_set("sw_commit", (bootInfo.commit == 0) ? "1" : "0");
                if(ret != 0)
                {
                    outSwDownload.dataType = CTC_ORGSPEC_SWDL_TYPE_COMMIT;
                    outSwDownload.opCode = CTC_ORGSPEC_SWDL_OPCODE_COMMITRESP;
                    outSwDownload.length = CTC_ORGSPEC_SWDOWNLOAD_HDR_LEN + CTC_ORGSPEC_SWDOWNLOAD_CMTRESP_LEN;
                    outSwDownload.tid = inSwDownload.tid;
                    outSwDownload.parse.commitResp.ack = CTC_ORGSPEC_SWDL_CMTACK_FAIL;
                    EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_ERROR,
                        "[OAM:%s:%d] commit img #%d fail\n", __FILE__, __LINE__, (bootInfo.commit == 0) ? 1 : 0);
                    break;
                }
            }
            else
            {
                outSwDownload.dataType = CTC_ORGSPEC_SWDL_TYPE_COMMIT;
                outSwDownload.opCode = CTC_ORGSPEC_SWDL_OPCODE_COMMITRESP;
                outSwDownload.length = CTC_ORGSPEC_SWDOWNLOAD_HDR_LEN + CTC_ORGSPEC_SWDOWNLOAD_CMTRESP_LEN;
                outSwDownload.tid = inSwDownload.tid;
                outSwDownload.parse.commitResp.ack = CTC_ORGSPEC_SWDL_CMTACK_NOTSUPPORT;
                EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_ERROR,
                    "[OAM:%s:%d] loader not support dual image\n", __FILE__, __LINE__);
                break;
            }
            outSwDownload.dataType = CTC_ORGSPEC_SWDL_TYPE_COMMIT;
            outSwDownload.opCode = CTC_ORGSPEC_SWDL_OPCODE_COMMITRESP;
            outSwDownload.length = CTC_ORGSPEC_SWDOWNLOAD_HDR_LEN + CTC_ORGSPEC_SWDOWNLOAD_CMTRESP_LEN;
            outSwDownload.tid = inSwDownload.tid;
            outSwDownload.parse.commitResp.ack = CTC_ORGSPEC_SWDL_CMTACK_SUCCESS;
            EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_INFO,
                "[OAM:%s:%d] commit img #%d\n", __FILE__, __LINE__, (bootInfo.commit == 0) ? 1 : 0);
            break;
        case CTC_ORGSPEC_SWDL_OPCODE_COMMITRESP:
            /* Only ONU can send this, so there might be some error to receive this */
        default:
            return EPON_OAM_ERR_ORG;
        }
        break;
    default:
        return EPON_OAM_ERR_ORG;
    }
    ctc_oam_orgSpecSwDownload_gen(&outSwDownload, pReplyPtr, &genLen);
    CTC_BUF_ADD(pReplyPtr, bufLen, genLen);
    if(CTC_ORGSPEC_SWDL_OPCODE_ERROR == outSwDownload.opCode)
    {
        ctc_oam_swDownloadError_free(&outSwDownload);
    }
    *pReplyLen = (pReplyPtr - pReplyBuf);

    return EPON_OAM_ERR_OK;
}

static int ctc_oam_orgSpecChurning(
    unsigned char llidIdx,      /* LLID index of the incoming operation */
    unsigned char *pFrame,      /* Frame payload current pointer */
    unsigned short length,      /* Frame payload length */
    unsigned char *pReplyBuf,   /* Frame buffer for reply OAM */
    unsigned short bufLen,      /* Frame buffer size */
    unsigned short *pReplyLen)  /* Reply size used by this handler*/
{
    int ret;
    unsigned char *pProcPtr, *pReplyPtr;
    unsigned short genLen;
    unsigned short remainLen;
    ctc_churning_t inChurning, outChurning;

    /* 1. Parse churning header
     * 2. Process CTC extended OAMPDU according to churning op code
     */
    pProcPtr = pFrame;
    pReplyPtr = pReplyBuf;
    remainLen = length;
    *pReplyLen = 0;

    if(remainLen < CTC_ORGSPEC_CHURNING_HDR_LEN)
    {
        /* Insufficient length for parsing */
        return EPON_OAM_ERR_OK;
    }

    /* Parse common header for churning extended opcode */
    inChurning.churningCode = pProcPtr[0] & CTC_ORGSPEC_CHURNING_CODE_MASK;
    inChurning.keyIdx = pProcPtr[1];
    CTC_BUF_ADD(pProcPtr, remainLen, CTC_ORGSPEC_CHURNING_HDR_LEN);

    switch(inChurning.churningCode)
    {
    case CTC_ORGSPEC_CHURNING_NEWKEYREQ:
        /* TODO - 10G uses different format of new key reply */
        /* Reply the new key by random select one */
        outChurning.churningCode = CTC_ORGSPEC_CHURNING_NEWKEY;
        if(ctc_last_churningKey[llidIdx].keyIdx!=inChurning.keyIdx)
        {
            ctc_oam_churningKey_rand(outChurning.churningKey);
            ctc_last_churningKey[llidIdx].churningKey[0] = outChurning.churningKey[0];
            ctc_last_churningKey[llidIdx].churningKey[1] = outChurning.churningKey[1];
            ctc_last_churningKey[llidIdx].churningKey[2] = outChurning.churningKey[2];
        }
        else
        {
            outChurning.churningKey[0]=ctc_last_churningKey[llidIdx].churningKey[0];
            outChurning.churningKey[1]=ctc_last_churningKey[llidIdx].churningKey[1];
            outChurning.churningKey[2]=ctc_last_churningKey[llidIdx].churningKey[2];            
        }
        ctc_last_churningKey[llidIdx].keyIdx = inChurning.keyIdx;    

        outChurning.keyIdx = inChurning.keyIdx ^ 0x01; /* Toggle the key index */
        ret = ctc_oam_churningKey_set(llidIdx, outChurning.keyIdx, outChurning.churningKey);
        if(EPON_OAM_ERR_OK != ret ||
          (bufLen < CTC_ORGSPEC_CHURNING_HDR_LEN + CTC_ORGSPEC_CHURNING_KEY_LEN))
        {
            /* Set failed/insufficient length, don't reply to OLT */
            return EPON_OAM_ERR_ORG;
        }

        ctc_oam_orgSpecChurning_gen(&outChurning, pReplyPtr, &genLen);
        CTC_BUF_ADD(pReplyPtr, bufLen, genLen);        
        break;
    case CTC_ORGSPEC_CHURNING_NEWKEY:
        /* Only ONU can send this, so there might be some error to receive this */
    default:
        return EPON_OAM_ERR_ORG;
    }
    *pReplyLen = (pReplyPtr - pReplyBuf);

    return EPON_OAM_ERR_OK;
}

static int ctc_oam_orgSpecDba(
    unsigned char llidIdx,      /* LLID index of the incoming operation */
    unsigned char *pFrame,      /* Frame payload current pointer */
    unsigned short length,      /* Frame payload length */
    unsigned char *pReplyBuf,   /* Frame buffer for reply OAM */
    unsigned short bufLen,      /* Frame buffer size */
    unsigned short *pReplyLen)  /* Reply size used by this handler*/
{
    int ret;
    unsigned char dbaCode;
    unsigned char *pProcPtr, *pReplyPtr;
    unsigned short genLen, parseLen, expectLen;
    unsigned short remainLen;
    ctc_dbaThreshold_t dbaThreshold;

    /* 1. Parse common DBA header
     * 2. Parse DBA header according to different DBA code
     * 3. Process according to different DBA code
     */
    pProcPtr = pFrame;
    pReplyPtr = pReplyBuf;
    remainLen = length;
    *pReplyLen = 0;
    expectLen = 0;

    if(remainLen < CTC_ORGSPEC_DBA_HDR_LEN)
    {
        /* Insufficient length for parsing */
        return EPON_OAM_ERR_OK;
    }

    /* Parse DBA common header */
    dbaCode = pProcPtr[0] & CTC_ORGSPEC_DBA_CODE_MASK;
    CTC_BUF_ADD(pProcPtr, remainLen, CTC_ORGSPEC_DBA_HDR_LEN);
    switch(dbaCode)
    {
    case CTC_ORGSPEC_DBA_GET_REQ:
        /* Get DBA threshold from database */
        ctc_oam_dbaConfig_get(&dbaThreshold);

        /* Check reply buffer length */
        expectLen = ctc_oam_orgSpecDba_calc(
            CTC_ORGSPEC_DBA_GET_RESP, &dbaThreshold);
        if(bufLen < expectLen)
        {
            /* Insufficient length for reply */
            break;
        }

        /* Generate DBA reply */
        ctc_oam_orgSpecDba_gen(
            CTC_ORGSPEC_DBA_GET_RESP,
            &dbaThreshold,
            0,
            pReplyPtr,
            &genLen);
        CTC_BUF_ADD(pReplyPtr, bufLen, genLen);
        break;
    case CTC_ORGSPEC_DBA_SET_REQ:
        /* Length calculation in parse function */
        ctc_oam_orgSpecDbaTheshold_parse(pProcPtr, remainLen, &parseLen, &dbaThreshold);
        if(0 == parseLen)
        {
            /* Incorrect frame format */
            break;
        }
        CTC_BUF_ADD(pProcPtr, remainLen, parseLen);

        /* Check reply buffer length */
        expectLen = ctc_oam_orgSpecDba_calc(
            CTC_ORGSPEC_DBA_SET_RESP, &dbaThreshold);
        if(bufLen < expectLen)
        {
            /* Insufficient length for reply */
            break;
        }

        ret = ctc_oam_dbaConfig_set(&dbaThreshold);
        /* Use set result to indicate the set ACK */
        ctc_oam_orgSpecDba_gen(
            CTC_ORGSPEC_DBA_SET_RESP,
            &dbaThreshold,
            (ret == EPON_OAM_ERR_OK) ? 0x01 : 0x00,
            pReplyPtr,
            &genLen);
        CTC_BUF_ADD(pReplyPtr, bufLen, genLen);
        break;
    case CTC_ORGSPEC_DBA_GET_RESP:
    case CTC_ORGSPEC_DBA_SET_RESP:
        /* According to CTC standard, only ONU can send this to OLT
         * ONU should not receive these two, just ignore it
         */
    default:
        /* Impossible case */
        return EPON_OAM_ERR_OK;
    }
    *pReplyLen = (pReplyPtr - pReplyBuf);

    return EPON_OAM_ERR_OK;
}

static int ctc_oam_orgSpecEvent(
    unsigned char llidIdx,      /* LLID index of the incoming operation */
    unsigned char *pFrame,      /* Frame payload current pointer */
    unsigned short length,      /* Frame payload length */
    unsigned char *pReplyBuf,   /* Frame buffer for reply OAM */
    unsigned short bufLen,      /* Frame buffer size */
    unsigned short *pReplyLen)  /* Reply size used by this handler*/
{
    int ret;
    unsigned char eventCode;
    unsigned char *pProcPtr, *pReplyPtr;
    unsigned short genLen, parseLen, expectLen;
    unsigned short remainLen;
    ctc_dbaThreshold_t dbaThreshold;

    /* 1. Parse common event header
     * 2. Parse event header according to different subtype
     * 3. Process according to different subtype
     */
    pProcPtr = pFrame;
    pReplyPtr = pReplyBuf;
    remainLen = length;
    *pReplyLen = 0;
    expectLen = 0;

    if(remainLen < CTC_ORGSPEC_EVENT_HDR_LEN)
    {
        /* Insufficient length for parsing */
        return EPON_OAM_ERR_OK;
    }

    /* Parse DBA common header */
    eventCode = pProcPtr[0];
    CTC_BUF_ADD(pProcPtr, remainLen, CTC_ORGSPEC_EVENT_HDR_LEN);
    switch(eventCode)
    {
    case CTC_ORGSPEC_EVENT_STATUS_REQ:
        break;
    case CTC_ORGSPEC_EVENT_STATUS_SET:
        break;
    case CTC_ORGSPEC_EVENT_THRESHOLD_REQ:
        break;
    case CTC_ORGSPEC_EVENT_THRESHOLD_SET:
        break;
    case CTC_ORGSPEC_EVENT_STATUS_RESP:
    case CTC_ORGSPEC_EVENT_THRESHOLD_RESP:
        /* According to CTC standard, only ONU can send this to OLT
         * ONU should not receive these two, just ignore it
         */
    default:
        /* Impossible case */
        return EPON_OAM_ERR_OK;
    }
    *pReplyLen = (pReplyPtr - pReplyBuf);

    return EPON_OAM_ERR_OK;
}

static int ctc_oam_extInfoStd_parser(
    unsigned char *pFrame,      /* Frame payload current pointer */
    unsigned short length,      /* Frame payload length */
    ctc_infoOam_t *pInfoOam
)
{
    unsigned char verListLen;
    unsigned char *pParsePtr;
    ctc_infoOamVerRec_t *pVerListNew, *pVerList;
    
    pInfoOam->vertionList = NULL;
    pParsePtr = pFrame;
    pInfoOam->type = *pParsePtr;
    pParsePtr += 1; /* Type */
    pInfoOam->length = *pParsePtr;
    pParsePtr += 1; /* Length */

    /* Length check */
    if(length < pInfoOam->length)
    {
        /* Incorrect length field */
        return 0;
    }

    /* Version list length correction */
    verListLen = (pInfoOam->length - CTC_INFO_OAM_MIN);
    if((verListLen % CTC_INFO_OAM_VERITEM_LEN) != 0) {
        /* version list should be integer times of CTC_INFO_OAM_VERITEM_LEN */
        verListLen -= (verListLen % CTC_INFO_OAM_VERITEM_LEN);
    }

    memcpy(pInfoOam->oui, pParsePtr, 3);
    pParsePtr += 3; /* OUI */
    pInfoOam->extSupport = *pParsePtr;
    pParsePtr += 1; /* ExtSupport */
    pInfoOam->version = *pParsePtr;
    pParsePtr += 1; /* Version */

    /* Parse version list */
    while(0 != verListLen)
    {
        ctc_oam_verListItem_alloc(pParsePtr, *(pParsePtr + 3), &pVerListNew);
        if(NULL == pVerListNew)
        {
            /* Allocate failed */
            /* Just ignore the remain length */
            break;
        }

        if(NULL == pInfoOam->vertionList)
        {
            pInfoOam->vertionList = pVerListNew;
        }
        if(NULL != pVerList)
        {
            pVerList->next = pVerListNew;
        }
        pVerList = pVerListNew;

        verListLen -= CTC_INFO_OAM_VERITEM_LEN;
        pParsePtr += CTC_INFO_OAM_VERITEM_LEN;
    }

    return pInfoOam->length;
}

static int ctc_oam_extInfo_parser(
    unsigned char llidIdx,      /* LLID index from HW table */
    unsigned char *pFrame,      /* Frame payload current pointer */
    unsigned short length,      /* Frame payload length */
    unsigned short *pExtractLen,/* Parser extract length */
    void **ppOrgSpecData)       /* Orgnization specific data */
{
    ctc_infoOam_t *pInfoOam;

    /* Check for minimum length for parsing */
    if(length < CTC_INFO_OAM_MIN)
    {
        return EPON_OAM_ERR_ORG;
    }

    pInfoOam = (ctc_infoOam_t *) malloc(sizeof(ctc_infoOam_t));
    if(NULL == pInfoOam)
    {
        return EPON_OAM_ERR_ORG;
    }

    /* Confirm length before parsing */
    *pExtractLen = ctc_oam_extInfoStd_parser(pFrame, length, pInfoOam);
    if(0 == *pExtractLen)
    {
        if(NULL != pInfoOam->vertionList)
        {
            ctc_oam_verListItem_free(&pInfoOam->vertionList);
        }
        free(pInfoOam);
        *ppOrgSpecData = NULL;
        return EPON_OAM_ERR_ORG;
    }
    *ppOrgSpecData = pInfoOam;

    return EPON_OAM_ERR_OK;
}

/* MODIFY ME - need further enhance for exception cases in all states */
static int ctc_oam_extInfo_handler(
    oam_oamPdu_t *pOamPdu,      /* OAMPDU data */
    unsigned char *pReplyBuf,   /* Frame buffer for reply OAM */
    unsigned short bufLen,      /* Frame buffer size */
    unsigned short *pReplyLen,  /* Reply size used by this handler*/
    void **ppOrgSpecData)       /* Orgnization specific data
                                 * Direct comes from parser
                                 */
{
    int ret = EPON_OAM_ERR_OK;
    unsigned short expectLen;
    unsigned char extSupport;
    ctc_infoOam_t *pInfoOam;

    if(NULL != ppOrgSpecData)
    {
        pInfoOam = *ppOrgSpecData;
    }
    else
    {
        return EPON_OAM_ERR_PARAM;
    }
    if(NULL == pInfoOam)
    {
        return EPON_OAM_ERR_PARAM;
    }

    dump_ctc_infoOam(pInfoOam);


    EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_INFO, 
        "[OAM:%s:%d] ctc_discovery_state[LLIDidx:%d]=%d \n", __FILE__, __LINE__, pOamPdu->llidIdx,ctc_discovery_state[pOamPdu->llidIdx]);


    switch(ctc_discovery_state[pOamPdu->llidIdx])
    {
    case CTC_OAM_FSM_STATE_WAIT_REMOTE:
        ctc_discovery_state[pOamPdu->llidIdx] = CTC_OAM_FSM_STATE_WAIT_REMOTE_OK;
        extSupport = ctc_oam_version_support(pInfoOam->oui, pInfoOam->version);
        expectLen = CTC_INFO_OAM_MIN + sizeof(supportedVerList);
        if(bufLen < expectLen)
        {
            *pReplyLen = 0;
            ret = EPON_OAM_ERR_ORG;
        }
        else
        {
            ctc_oam_Info_gen(expectLen, extSupport, 0x00, pInfoOam, 1, pReplyBuf);
            *pReplyLen = expectLen;
        }
        break;
    case CTC_OAM_FSM_STATE_WAIT_REMOTE_OK:
        /* Check if the OUI/version pair in reply is supported */
        extSupport = ctc_oam_version_support(pInfoOam->oui, pInfoOam->version);
        if((1 == extSupport) && (1 == pInfoOam->extSupport))
        {
            /* Both ONU and OLT accept the OUI/Version pair */
            ctc_discovery_state[pOamPdu->llidIdx] = CTC_OAM_FSM_STATE_COMPLETE;
            memcpy(currCtcVer[pOamPdu->llidIdx].oui, pInfoOam->oui, EPON_OAM_OUI_LENGTH);
            currCtcVer[pOamPdu->llidIdx].version = pInfoOam->version;
        }
        else
        {
            /* At least one side didn't satisfied the OUI/Version */
            ctc_discovery_state[pOamPdu->llidIdx] = CTC_OAM_FSM_STATE_WAIT_REMOTE;
            /* Force send out NACK */
            extSupport = 0;
        }

        expectLen = CTC_INFO_OAM_MIN;
        if(bufLen < expectLen)
        {
            *pReplyLen = 0;
            ret = EPON_OAM_ERR_ORG;
        }
        else
        {
            ctc_oam_Info_gen(expectLen, extSupport, pInfoOam->version, pInfoOam, 0, pReplyBuf);
            *pReplyLen = expectLen;
        }
        break;
    case CTC_OAM_FSM_STATE_COMPLETE:
        /* Ignore any extended info after extended discovery compelete */
        if(pInfoOam->vertionList!=NULL&&pInfoOam->extSupport==1)
        {
         ctc_discovery_state[pOamPdu->llidIdx] = CTC_OAM_FSM_STATE_WAIT_REMOTE;
        }
        ret = EPON_OAM_ERR_OK;
        break;
    default:
        /* Not supported states */
        break;
    }

    ctc_oam_verListItem_free(&pInfoOam->vertionList);
    free(pInfoOam);
    
    return ret;
}

static int
ctc_oam_orgSpec_processor(
    oam_oamPdu_t *pOamPdu,      /* OAMPDU data */
    unsigned char *pFrame,      /* Frame payload current pointer */
    unsigned short length,      /* Frame payload length */
    unsigned char *pReplyBuf,   /* Frame buffer for reply OAM */
    unsigned short bufLen,      /* Frame buffer size */
    unsigned short *pReplyLen)  /* Reply size used by this handler*/
{
    int ret = EPON_OAM_ERR_OK;
    unsigned char oui[3];
    unsigned char currVer;
    unsigned char extOpcode;
    unsigned short genLen;

    /* 0. Check for extended discovery state
     * 1. Parsing the frame
     * 2. Process frame according to the extended opcode
     * 3. Reply extended OAMPDU
     */
    *pReplyLen = 0;
    if((length < CTC_ORGSPEC_HDR_LEN) || (bufLen < CTC_ORGSPEC_HDR_LEN))
    {
        /* Insufficient length for parsing */
        return EPON_OAM_ERR_PARAM;
    }

    /* Parse the extended OAM header */
    oui[0] = pFrame[0];
    oui[1] = pFrame[1];
    oui[2] = pFrame[2];
    extOpcode = pFrame[3];
    CTC_BUF_ADD(pFrame, length, CTC_ORGSPEC_HDR_LEN);
    EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_INFO, 
        "[OAM:%s:%d] ext. opcode %d\n", __FILE__, __LINE__, extOpcode);

    if(CTC_OAM_FSM_STATE_COMPLETE != ctc_discovery_state[pOamPdu->llidIdx])
    {
        /* Extended OAM discovery not complete
         * Don't know what version to be used
         * Try to use CTC 1.0
         */
        currVer = 0x01;
    }
    else
    {
        currVer = currCtcVer[pOamPdu->llidIdx].version;
    }   

    switch(extOpcode)
    {
    case CTC_EXTOAM_OPCODE_VARREQ:
        ctc_oam_orgSpecHdr_gen(oui, CTC_EXTOAM_OPCODE_VARRESP, pReplyBuf, &genLen);
        CTC_BUF_ADD(pReplyBuf, bufLen, genLen);
        *pReplyLen += genLen;
        genLen= 0;
        if(currVer <= CTC_OAM_VERSION_20)
        {
#if CTC_OAM_SUPPORT_BENEATH_20
            ret = ctc_oam_orgSpecVar_req20(
                pOamPdu->llidIdx, pFrame, length, pReplyBuf, bufLen, &genLen);
#endif /* CTC_OAM_SUPPORT_BENEATH_20 */
        }
        else
        {
#if CTC_OAM_SUPPORT_ABOVE_21
            ret = ctc_oam_orgSpecVar_req21(
                pOamPdu->llidIdx, pFrame, length, pReplyBuf, bufLen, &genLen);
#endif /* CTC_OAM_SUPPORT_ABOVE_21 */
        }
        if(0 != genLen)
        {
            *pReplyLen += genLen;
        }
        else
        {
            /* Nothing to be replied, remove encoded header */
            *pReplyLen = 0;
        }
        break;
    case CTC_EXTOAM_OPCODE_VARRESP:
        /* Should not receive this on ONU */
        break;
    case CTC_EXTOAM_OPCODE_SETREQ:
        ctc_oam_orgSpecHdr_gen(oui, CTC_EXTOAM_OPCODE_SETRESP, pReplyBuf, &genLen);
        CTC_BUF_ADD(pReplyBuf, bufLen, genLen);
        *pReplyLen += genLen;
        genLen= 0;
        if(currVer <= CTC_OAM_VERSION_20)
        {
#if CTC_OAM_SUPPORT_BENEATH_20
            ret = ctc_oam_orgSpecSet_req20(
                pOamPdu->llidIdx, pFrame, length, pReplyBuf, bufLen, &genLen);
#endif /* CTC_OAM_SUPPORT_BENEATH_20 */
        }
        else
        {
#if CTC_OAM_SUPPORT_ABOVE_21
            ret = ctc_oam_orgSpecSet_req21(
                pOamPdu->llidIdx, pFrame, length, pReplyBuf, bufLen, &genLen);
#endif /* CTC_OAM_SUPPORT_ABOVE_21 */
        }
        if(0 != genLen)
        {
            *pReplyLen += genLen;
        }
        else
        {
            /* Nothing to be replied, remove encoded header */
            *pReplyLen = 0;
        }
        break;
    case CTC_EXTOAM_OPCODE_SETRESP:
        /* Should not receive this on ONU */
        break;
    case CTC_EXTOAM_OPCODE_ONUAUTH:
        ctc_oam_orgSpecHdr_gen(oui, CTC_EXTOAM_OPCODE_ONUAUTH, pReplyBuf, &genLen);
        CTC_BUF_ADD(pReplyBuf, bufLen, genLen);
        *pReplyLen += genLen;
        genLen= 0;
        ret = ctc_oam_orgSpecOnuAuth(
            pOamPdu->llidIdx, pFrame, length, pReplyBuf, bufLen, &genLen);
        if(0 != genLen)
        {
            *pReplyLen += genLen;
        }
        else
        {
            /* Nothing to be replied, remove encoded header */
            *pReplyLen = 0;
        }
        break;
    case CTC_EXTOAM_OPCODE_SWDOWNLOAD:
        ctc_oam_orgSpecHdr_gen(oui, CTC_EXTOAM_OPCODE_SWDOWNLOAD, pReplyBuf, &genLen);
        CTC_BUF_ADD(pReplyBuf, bufLen, genLen);
        *pReplyLen += genLen;
        genLen= 0;
        ret = ctc_oam_orgSpecSwDownload(
            pOamPdu->llidIdx, pFrame, length, pReplyBuf, bufLen, &genLen);
        if(0 != genLen)
        {
            *pReplyLen += genLen;
        }
        else
        {
            /* Nothing to be replied, remove encoded header */
            *pReplyLen = 0;
        }
        break;
    case CTC_EXTOAM_OPCODE_CHURNING:
        ctc_oam_orgSpecHdr_gen(oui, CTC_EXTOAM_OPCODE_CHURNING, pReplyBuf, &genLen);
        CTC_BUF_ADD(pReplyBuf, bufLen, genLen);
        *pReplyLen += genLen;
        genLen= 0;
        ret = ctc_oam_orgSpecChurning(
            pOamPdu->llidIdx, pFrame, length, pReplyBuf, bufLen, &genLen);
        if(0 != genLen)
        {
            *pReplyLen += genLen;
        }
        else
        {
            /* Nothing to be replied, remove encoded header */
            *pReplyLen = 0;
        }
        break;
    case CTC_EXTOAM_OPCODE_DBA:
        ctc_oam_orgSpecHdr_gen(oui, CTC_EXTOAM_OPCODE_DBA, pReplyBuf, &genLen);
        CTC_BUF_ADD(pReplyBuf, bufLen, genLen);
        *pReplyLen += genLen;
        genLen= 0;
        ret = ctc_oam_orgSpecDba(
            pOamPdu->llidIdx, pFrame, length, pReplyBuf, bufLen, &genLen);
        if(0 != genLen)
        {
            *pReplyLen += genLen;
        }
        else
        {
            /* Nothing to be replied, remove encoded header */
            *pReplyLen = 0;
        }
        break;
    case CTC_EXTOAM_OPCODE_EVENT:
        ctc_oam_orgSpecHdr_gen(oui, CTC_EXTOAM_OPCODE_EVENT, pReplyBuf, &genLen);
        CTC_BUF_ADD(pReplyBuf, bufLen, genLen);
        *pReplyLen += genLen;
        genLen= 0;
        ret = ctc_oam_orgSpecEvent(
            pOamPdu->llidIdx, pFrame, length, pReplyBuf, bufLen, &genLen);
        if(0 != genLen)
        {
            *pReplyLen += genLen;
        }
        else
        {
            /* Nothing to be replied, remove encoded header */
            *pReplyLen = 0;
        }
        break;
    case CTC_EXTOAM_OPCODE_RESERVED:
    default:
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_WARN,
            "[OAM:%s:%d] illegal ext opcode %u\n", __FILE__, __LINE__, extOpcode);
        break;
    }

    if(EPON_OAM_ERR_OK != ret)
    {
        /* Something wrong, remove all reply */
        *pReplyLen = 0;
    }

    return ret;
}

int ctc_oam_evtNotification_send(
    ctc_eventOam_t *pEvent)
{
    
    return EPON_OAM_ERR_OK;
}

int ctc_oam_init(void)
{
    int i, ret;
    char nvVarValue[CTC_EXECMD_UV_VALUE_LEN];
    char nvVar[CTC_EXECMD_UV_VAR_LEN];
    unsigned char oui[] = CTC_OAM_OUI;
    oam_infoOrgSpecCb_t infoCb;
    oam_orgSpecCb_t orgSpecCb;

    /* Register info organization specific callback */
    infoCb.parser = ctc_oam_extInfo_parser;
    infoCb.handler = ctc_oam_extInfo_handler;
    ret = epon_oam_orgSpecCb_reg(
        EPON_OAM_CBTYPE_INFO_ORGSPEC,
        oui,
        (void *) &infoCb);
    if(EPON_OAM_ERR_OK != ret)
    {
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_ERROR,
            "Failed to register CTC info callback functions\n");
        return EPON_OAM_ERR_UNKNOWN;
    }

    /* Register organization specific callback */
    orgSpecCb.processor = ctc_oam_orgSpec_processor;
    ret = epon_oam_orgSpecCb_reg(
        EPON_OAM_CBTYPE_ORGSPEC,
        oui,
        (void *) &orgSpecCb);
    if(EPON_OAM_ERR_OK != ret)
    {
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_ERROR,
            "Failed to register CTC orgSpec callback functions\n");
        return EPON_OAM_ERR_UNKNOWN;
    }

    ret = sem_init(&ctcDataSem, 0, 1);
    if(0 != ret)
    {
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_CTC_ERROR,
            "Failed init data semaphore %d %d\n", ret, errno);
    }

    /* Retrive the current image info */
    ret = ctc_oam_swDownloadNv_get("sw_active", nvVarValue);
    /* Lock the data during the init */
    sem_wait(&ctcDataSem);
    if(0 == ret)
    {
        bootInfo.active = atoi(nvVarValue);
        /* Read version according to the active number */
        sprintf(nvVar, "sw_version%d", bootInfo.active);
        ret = ctc_oam_swDownloadNv_get(nvVar, nvVarValue);
        if(0 == ret)
        {
            strncpy(bootInfo.version, nvVarValue, CTC_EXECMD_UV_VALUE_LEN);
        }
        else
        {
            strcpy(bootInfo.version, EPON_OAM_VERSION);
        }
    }
    else
    {
        bootInfo.active = -1;
        strcpy(bootInfo.version, EPON_OAM_VERSION);
    }

    ret = ctc_oam_swDownloadNv_get("sw_commit", nvVarValue);
    if(0 == ret)
    {
        bootInfo.commit = atoi(nvVarValue);
    }
    else
    {
        bootInfo.commit = -1;
    }
    sem_post(&ctcDataSem);

    return EPON_OAM_ERR_OK;
}

int ctc_oam_db_init(
    unsigned char llidIdx)
{
    ctc_onuAuthLoid_t loidAuth;

    /* MODIFY ME - all those variables need semaphore protection */
    /* Init extended discovery state */
    ctc_discovery_state[llidIdx] = CTC_OAM_FSM_STATE_WAIT_REMOTE;
    memset(&currCtcVer[llidIdx], 0x0, sizeof(ctc_infoOamVer_t));

    memset(&loidAuth, 0x0, sizeof(ctc_onuAuthLoid_t));
    strncpy(loidAuth.loid, "user", sizeof("user"));
    strncpy(loidAuth.password, "password", sizeof("password"));
    ctc_oam_onuAuthLoid_set(llidIdx, &loidAuth);
    ctc_oam_onuAuthState_set(llidIdx, CTC_OAM_ONUAUTH_STATE_NOTCOMPLETE, 0);

    /* Only base LLID can use software download */
    if(llidIdx == 0)
    {
        ctc_oam_swDownloadBuf_clear();
    }

    memset(&ctc_last_churningKey[llidIdx], 0x0, sizeof(ctc_churning_t));

    return EPON_OAM_ERR_OK;
}

