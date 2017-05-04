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
 * $Date: 2013-11-20 18:17:00 +0800 (週三, 20 十一月 2013) $
 *
 * Purpose : Define the EPON OAM standard database and configuretion
 *           related operation
 *
 * Feature : Provide database and configuration set and get APIs
 *
 */

/*
 * Include Files
 */
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <semaphore.h>

#include "epon_oam_config.h"
#include "epon_oam_err.h"
#include "epon_oam_rx.h"
#include "epon_oam_db.h"
#include "epon_oam_dbg.h"

#include <rtk/epon.h>

/*
 * Symbol Definition
 */
/* Data structure for dummy parser and handler*/
typedef struct oam_dummy_s {
    unsigned short length;
    unsigned char *data;
} oam_dummy_t;

/*
 * Data Declaration
 */
/* OAM protocol stack configuration */
static oam_config_t oamConfig[EPON_OAM_SUPPORT_LLID_NUM];
/* Semaphore for database access */
static sem_t oamDbAccessSem;
/* Allow each LLID has its own local oam info */
static oam_oamInfo_t localOamInfo[EPON_OAM_SUPPORT_LLID_NUM];
static oam_oamInfo_t remoteOamInfo[EPON_OAM_SUPPORT_LLID_NUM];
/* Callback function database */
static oam_infoOrgSpecCb_t defInfoOrgSpecCb;
static oam_infoOrgSpecCbRec_t *pInfoOrgSpecCbDb;
static oam_eventOrgSpecCb_t defEventOrgSpecCb;
static oam_eventOrgSpecCbRec_t *pEventOrgSpecCbDb;
static oam_orgSpecCb_t defOrgSpecCb;
static oam_orgSpecCbRec_t *pOrgSpecCbDb;
static oam_varCbRec_t *pVarCbDb[EPON_OAM_VARIABLEDB_HASH_NUM];
static int (*defVarCb)(
        oam_varTarget_t *pVarTarget,
        oam_varDescriptor_t varDesc,
        oam_varContainer_t **ppVarContainer);
static oam_counter_t oamCounter[EPON_OAM_VARIABLEDB_HASH_NUM];

/*
 * Macro Definition
 */

/*
 * Function Declaration
 */

static int epon_oam_dummyParser(
    unsigned char llidIdx,      /* LLID index from HW table */
    unsigned char *pFrame,      /* Frame payload current pointer */
    unsigned short length,      /* Frame payload length */
    unsigned short *pExtractLen,/* Parser extract length */
    void **orgSpecData)          /* Orgnization specific data */
{
    oam_dummy_t *pOamDummy;

    /* Default parser only allocate memory and pass it to default handler */
    pOamDummy = (oam_dummy_t *) malloc(sizeof(oam_dummy_t));
    if(NULL == pOamDummy)
    {
        return EPON_OAM_ERR_MEM;
    }

    pOamDummy->length = pFrame[1];

    if(pOamDummy->length > length)
    {
        return EPON_OAM_ERR_PARSE;
    }

    pOamDummy->data = (unsigned char *) malloc(sizeof(unsigned char) * pOamDummy->length);

    if(pOamDummy->data == NULL)
    {
        free(pOamDummy);
        return EPON_OAM_ERR_MEM;
    }

    memcpy(pOamDummy->data, pFrame, sizeof(unsigned char) * pOamDummy->length);
    *pExtractLen = pOamDummy->length;
    *((oam_dummy_t **)orgSpecData) = pOamDummy;

    return EPON_OAM_ERR_OK;
}

static int epon_oam_dummyHandler(
    oam_oamPdu_t *pOamPdu,      /* OAMPDU data */
    unsigned char *pReplyBuf,   /* Frame buffer for reply OAM */
    unsigned short bufLen,      /* Frame buffer size */
    unsigned short *pReplyLen,  /* Reply size used by this handler*/
    void **ppOrgSpecData)       /* Orgnization specific data
                                 * Direct comes from parser
                                 */
{
    oam_dummy_t *pOamDummy = *((oam_dummy_t **)ppOrgSpecData);

    /* Default handler only dump payload and free the memory */
    EPON_OAM_PRINT(EPON_OAM_DBGFLAG_DUMP, "Undefined org spec OAM with legnth %u\n", pOamDummy->length);
    DUMP_HEX_VALUE(EPON_OAM_DBGFLAG_DUMP, pOamDummy->data, pOamDummy->length);

    *pReplyLen = 0;

    free(pOamDummy->data);
    free(pOamDummy);

    return EPON_OAM_ERR_OK;
}

static int epon_oam_dummyProcessor(
    oam_oamPdu_t *pOamPdu,      /* OAMPDU data */
    unsigned char *pFrame,      /* Frame payload current pointer */
    unsigned short length,      /* Frame payload length */
    unsigned char *pReplyBuf,   /* Frame buffer for reply OAM */
    unsigned short bufLen,      /* Frame buffer size */
    unsigned short *pReplyLen)  /* Reply size used by this handler*/
{
    DUMP_HEX_VALUE(EPON_OAM_DBGFLAG_DUMP, pFrame, length);
    *pReplyLen = 0;

    return EPON_OAM_ERR_OK;
}

static int epon_oam_dummyVarHandler(
    oam_varTarget_t *pVarTarget,
    oam_varDescriptor_t varDesc,
    oam_varContainer_t **ppVarContainer)
{
    EPON_OAM_PRINT(EPON_OAM_DBGFLAG_DUMP,
        "Unrecognize variable 0x%02x/0x%04x\n", varDesc.varBranch, varDesc.varLeaf);
    if(NULL != pVarTarget)
    {
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_DUMP,
            "Object Idendtifier 0x%02x/0x%04x 0x%02x%02x\n",
            pVarTarget->varDesc.varBranch,
            pVarTarget->varDesc.varLeaf,
            pVarTarget->varData[0],
            pVarTarget->varData[1]);
    }

    *ppVarContainer = NULL;

    return EPON_OAM_ERR_OK;
}

static oam_infoOrgSpecCbRec_t *epon_oam_orgSpecInfo_get(unsigned char *pOui)
{
    oam_infoOrgSpecCbRec_t *pSearchPtr;

    pSearchPtr = pInfoOrgSpecCbDb;
    while(NULL != pSearchPtr)
    {
        if((pSearchPtr->oui[0] == pOui[0]) &&
           (pSearchPtr->oui[1] == pOui[1]) &&
           (pSearchPtr->oui[2] == pOui[2]))
        {
            return pSearchPtr;
        }
        pSearchPtr = pSearchPtr->next;
    }

    return NULL;
}

static int epon_oam_orgSpecInfo_reg(
    unsigned char *pOui,
    oam_infoOrgSpecCb_t *pCbData)
{
    oam_infoOrgSpecCbRec_t *pInfoCbRec;

    pInfoCbRec = epon_oam_orgSpecInfo_get(pOui);
    if(NULL == pInfoCbRec)
    {
        /* New entry */
        pInfoCbRec = (oam_infoOrgSpecCbRec_t *) malloc(sizeof(oam_infoOrgSpecCbRec_t)); 
        if(NULL == pInfoCbRec)
        {
            return EPON_OAM_ERR_MEM;
        }
        pInfoCbRec->oui[0] = pOui[0];
        pInfoCbRec->oui[1] = pOui[1];
        pInfoCbRec->oui[2] = pOui[2];
        pInfoCbRec->parser = pCbData->parser;
        pInfoCbRec->handler = pCbData->handler;

        if(NULL == pInfoOrgSpecCbDb)
        {
            /* First entry */
            pInfoCbRec->prev = NULL;
            pInfoCbRec->next = NULL;
            pInfoOrgSpecCbDb = pInfoCbRec;
        }
        else
        {
            /* Insert entry */
            pInfoCbRec->prev = NULL;
            pInfoCbRec->next = pInfoOrgSpecCbDb;
            pInfoOrgSpecCbDb->prev = pInfoCbRec;
            pInfoOrgSpecCbDb = pInfoCbRec;
        }
    }
    else
    {
        /* Exist entry, just update data */
        pInfoCbRec->parser = pCbData->parser;
        pInfoCbRec->handler = pCbData->handler;
    }

    return EPON_OAM_ERR_OK;
}

static int epon_oam_orgSpecInfo_dereg(
    unsigned char *pOui)
{
    oam_infoOrgSpecCbRec_t *pInfoCbRec;

    pInfoCbRec = epon_oam_orgSpecInfo_get(pOui);
    if(NULL != pInfoCbRec)
    {
        if((NULL == pInfoCbRec->prev) &&
           (NULL == pInfoCbRec->next))
        {
            /* The only item in list */
            pInfoOrgSpecCbDb = NULL;
        }
        else
        {
            /* Multiple items in list */
            if(NULL != pInfoCbRec->prev)
            {
                pInfoCbRec->prev->next = pInfoCbRec->next;
            }
            if(NULL != pInfoCbRec->next)
            {
                pInfoCbRec->next->prev = pInfoCbRec->prev;
            }
            if(pInfoOrgSpecCbDb == pInfoCbRec)
            {
                pInfoOrgSpecCbDb = pInfoCbRec->next;
            }
        }
        free(pInfoCbRec);
    }

    return EPON_OAM_ERR_OK;
}

static oam_eventOrgSpecCbRec_t *epon_oam_orgSpecEvent_get(unsigned char *pOui)
{
    oam_eventOrgSpecCbRec_t *pSearchPtr;

    pSearchPtr = pEventOrgSpecCbDb;
    while(NULL != pSearchPtr)
    {
        if((pSearchPtr->oui[0] == pOui[0]) &&
           (pSearchPtr->oui[1] == pOui[1]) &&
           (pSearchPtr->oui[2] == pOui[2]))
        {
            return pSearchPtr;
        }
        pSearchPtr = pSearchPtr->next;
    }

    return NULL;
}

static int epon_oam_orgSpecEvent_reg(
    unsigned char *pOui,
    oam_eventOrgSpecCb_t *pCbData)
{
    oam_eventOrgSpecCbRec_t *pEventCbRec;

    pEventCbRec = epon_oam_orgSpecEvent_get(pOui);
    if(NULL == pEventCbRec)
    {
        /* New entry */
        pEventCbRec = (oam_eventOrgSpecCbRec_t *) malloc(sizeof(oam_eventOrgSpecCbRec_t)); 
        if(NULL == pEventCbRec)
        {
            return EPON_OAM_ERR_MEM;
        }
        pEventCbRec->oui[0] = pOui[0];
        pEventCbRec->oui[1] = pOui[1];
        pEventCbRec->oui[2] = pOui[2];
        pEventCbRec->parser = pCbData->parser;
        pEventCbRec->handler = pCbData->handler;

        if(NULL == pEventOrgSpecCbDb)
        {
            /* First entry */
            pEventCbRec->prev = NULL;
            pEventCbRec->next = NULL;
            pEventOrgSpecCbDb = pEventCbRec;
        }
        else
        {
            /* Insert entry */
            pEventCbRec->prev = NULL;
            pEventCbRec->next = pEventOrgSpecCbDb;
            pEventOrgSpecCbDb->prev = pEventCbRec;
            pEventOrgSpecCbDb = pEventCbRec;
        }
    }
    else
    {
        /* Exist entry, just update data */
        pEventCbRec->parser = pCbData->parser;
        pEventCbRec->handler = pCbData->handler;
    }

    return EPON_OAM_ERR_OK;
}

static int epon_oam_orgSpecEvent_dereg(
    unsigned char *pOui)
{
    oam_eventOrgSpecCbRec_t *pEventCbRec;

    pEventCbRec = epon_oam_orgSpecEvent_get(pOui);
    if(NULL != pEventCbRec)
    {
        if((NULL == pEventCbRec->prev) &&
           (NULL == pEventCbRec->next))
        {
            /* The only item in list */
            pEventOrgSpecCbDb = NULL;
        }
        else
        {
            /* Multiple items in list */
            if(NULL != pEventCbRec->prev)
            {
                pEventCbRec->prev->next = pEventCbRec->next;
            }
            if(NULL != pEventCbRec->next)
            {
                pEventCbRec->next->prev = pEventCbRec->prev;
            }
            if(pEventOrgSpecCbDb == pEventCbRec)
            {
                pEventOrgSpecCbDb = pEventCbRec->next;
            }
        }
        free(pEventCbRec);
    }

    return EPON_OAM_ERR_OK;
}

static oam_orgSpecCbRec_t *epon_oam_orgSpec_get(unsigned char *pOui)
{
    oam_orgSpecCbRec_t *pSearchPtr;

    pSearchPtr = pOrgSpecCbDb;
    while(NULL != pSearchPtr)
    {
        if((pSearchPtr->oui[0] == pOui[0]) &&
           (pSearchPtr->oui[1] == pOui[1]) &&
           (pSearchPtr->oui[2] == pOui[2]))
        {
            return pSearchPtr;
        }
        pSearchPtr = pSearchPtr->next;
    }

    return NULL;
}

static int epon_oam_orgSpec_reg(
    unsigned char *pOui,
    oam_orgSpecCb_t *pCbData)
{
    oam_orgSpecCbRec_t *pOrgSpecCbRec;

    pOrgSpecCbRec = epon_oam_orgSpec_get(pOui);
    if(NULL == pOrgSpecCbRec)
    {
        /* New entry */
        pOrgSpecCbRec = (oam_orgSpecCbRec_t *) malloc(sizeof(oam_orgSpecCbRec_t)); 
        if(NULL == pOrgSpecCbRec)
        {
            return EPON_OAM_ERR_MEM;
        }
        pOrgSpecCbRec->oui[0] = pOui[0];
        pOrgSpecCbRec->oui[1] = pOui[1];
        pOrgSpecCbRec->oui[2] = pOui[2];
        pOrgSpecCbRec->processor = pCbData->processor;

        if(NULL == pOrgSpecCbDb)
        {
            /* First entry */
            pOrgSpecCbRec->prev = NULL;
            pOrgSpecCbRec->next = NULL;
            pOrgSpecCbDb = pOrgSpecCbRec;
        }
        else
        {
            /* Insert entry */
            pOrgSpecCbRec->prev = NULL;
            pOrgSpecCbRec->next = pOrgSpecCbDb;
            pOrgSpecCbDb->prev = pOrgSpecCbRec;
            pOrgSpecCbDb = pOrgSpecCbRec;
        }
    }
    else
    {
        /* Exist entry, just update data */
        pOrgSpecCbRec->processor = pCbData->processor;
    }

    return EPON_OAM_ERR_OK;
}

static int epon_oam_orgSpec_dereg(
    unsigned char *pOui)
{
    oam_orgSpecCbRec_t *pOrgSpecCbRec;

    pOrgSpecCbRec = epon_oam_orgSpec_get(pOui);
    if(NULL != pOrgSpecCbRec)
    {
        if((NULL == pOrgSpecCbRec->prev) &&
           (NULL == pOrgSpecCbRec->next))
        {
            /* The only item in list */
            pOrgSpecCbDb = NULL;
        }
        else
        {
            /* Multiple items in list */
            if(NULL != pOrgSpecCbRec->prev)
            {
                pOrgSpecCbRec->prev->next = pOrgSpecCbRec->next;
            }
            if(NULL != pOrgSpecCbRec->next)
            {
                pOrgSpecCbRec->next->prev = pOrgSpecCbRec->prev;
            }
            if(pOrgSpecCbDb == pOrgSpecCbRec)
            {
                pOrgSpecCbDb = pOrgSpecCbRec->next;
            }
        }
        free(pOrgSpecCbRec);
    }

    return EPON_OAM_ERR_OK;
}

static unsigned char epon_oam_variableDb_hash(
    oam_varDescriptor_t *pVarDesc)
{
    unsigned char hashResult;

    hashResult = pVarDesc->varBranch;
    hashResult ^= ((unsigned char *)&pVarDesc->varLeaf)[0];
    hashResult ^= ((unsigned char *)&pVarDesc->varLeaf)[1];
    hashResult = (hashResult & 0x0f) ^ ((hashResult & 0xf0) >> 4);

    if(hashResult >= EPON_OAM_VARIABLEDB_HASH_NUM)
    {
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
            "[OAM:%s:%d] hash result error!\n", __FILE__, __LINE__);
        hashResult = 0;
    }

    return hashResult;
}

static oam_varCbRec_t *epon_oam_variable_get(
    oam_varDescriptor_t *pVarDesc)
{
    oam_varCbRec_t *pSearchPtr;

    /* 1. Calculate hash result and get the search start pointer
     * 2. Sequencial search for the variable
     */

    pSearchPtr = pVarCbDb[epon_oam_variableDb_hash(pVarDesc)];
    while(NULL != pSearchPtr)
    {
        if((pSearchPtr->varDesc.varBranch == pVarDesc->varBranch) &&
           (pSearchPtr->varDesc.varLeaf == pVarDesc->varLeaf))
        {
            return pSearchPtr;
        }
        pSearchPtr = pSearchPtr->next;
    }

    return NULL;
}

static int epon_oam_variable_reg(
    oam_varCb_t *pVarCb)
{
    oam_varDescriptor_t varDesc;
    oam_varCbRec_t *pVarCbRec;
    oam_varCbRec_t **ppVarDb;

    varDesc = pVarCb->varDesc;
    pVarCbRec = epon_oam_variable_get(&varDesc);
    if(NULL == pVarCbRec)
    {
        /* New entry */
        pVarCbRec = (oam_varCbRec_t *) malloc(sizeof(oam_varCbRec_t)); 
        if(NULL == pVarCbRec)
        {
            return EPON_OAM_ERR_MEM;
        }
        pVarCbRec->varType = pVarCb->varType;
        pVarCbRec->varDesc = pVarCb->varDesc;
        pVarCbRec->handler = pVarCb->handler;

        ppVarDb = &(pVarCbDb[epon_oam_variableDb_hash(&varDesc)]);
        if(NULL == *ppVarDb)
        {
            /* First entry */
            pVarCbRec->prev = NULL;
            pVarCbRec->next = NULL;
            *ppVarDb = pVarCbRec;
        }
        else
        {
            /* Insert entry */
            pVarCbRec->prev = NULL;
            pVarCbRec->next = *ppVarDb;
            (*ppVarDb)->prev = pVarCbRec;
            (*ppVarDb) = pVarCbRec;
        }
    }
    else
    {
        /* Exist entry, just update data */
        pVarCbRec->varType = pVarCb->varType;
        pVarCbRec->varDesc = pVarCb->varDesc;
        pVarCbRec->handler = pVarCb->handler;
    }

    return EPON_OAM_ERR_OK;
}

static int epon_oam_variable_dereg(
    oam_varDescriptor_t *pVarDesc)
{
    oam_varCbRec_t *pVarCbRec;
    oam_varCbRec_t **ppVarDb;

    pVarCbRec = epon_oam_variable_get(pVarDesc);
    if(NULL != pVarCbRec)
    {
        ppVarDb = &(pVarCbDb[epon_oam_variableDb_hash(pVarDesc)]);
        if((NULL == pVarCbRec->prev) &&
           (NULL == pVarCbRec->next))
        {
            /* The only item in list */
            *ppVarDb = NULL;
        }
        else
        {
            /* Multiple items in list */
            if(NULL != pVarCbRec->prev)
            {
                pVarCbRec->prev->next = pVarCbRec->next;
            }
            if(NULL != pVarCbRec->next)
            {
                pVarCbRec->next->prev = pVarCbRec->prev;
            }
            if(*ppVarDb == pVarCbRec)
            {
                *ppVarDb = pVarCbRec->next;
            }
        }
        free(pVarCbRec);
    }

    return EPON_OAM_ERR_OK;
}

static void epon_oam_initObjIdNum(void)
{
    oam_varCb_t varCb;

    varCb.varType = EPON_OAM_VARTYPE_OBJECTID;
    varCb.handler = NULL;

    /* PME ID */
    varCb.varDesc.varBranch = 7;
    varCb.varDesc.varLeaf = 330;
    epon_oam_variableCb_reg(&varCb);

    /* EPON LLID */
    varCb.varDesc.varBranch = 7;
    varCb.varDesc.varLeaf = 282;
    epon_oam_variableCb_reg(&varCb);
}

/*
 * User APIs
 */
/* Function Name:
 *      epon_oam_database_init
 * Description:
 *      Init database and related items
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      EPON_OAM_ERR_OK
 * Note:
 *      None
 */
int epon_oam_database_init(void)
{
    int i, ret;
    oam_oamInfo_t infoOam;

    /* Initial semaphore,
     * The semaphore might be used by multple source, including
     *   - packet rx thread
     *   - State keep thread
     *   - CLI configuration process
     * So set the semaphore can be access by multiple processes
     */
    ret = sem_init(&oamDbAccessSem, 0, 1);
    if(0 != ret)
    {
        printf("sem_init %d (%d)\n", ret, errno);
    }

    /* Init configuration */
    memset((unsigned char *) &oamConfig, 0x0, sizeof(oam_config_t) * EPON_OAM_SUPPORT_LLID_NUM);

    /* Init local/remote info database */
    memset((unsigned char *) &localOamInfo, 0x0, sizeof(oam_oamInfo_t) * EPON_OAM_SUPPORT_LLID_NUM);
    memset((unsigned char *) &remoteOamInfo, 0x0, sizeof(oam_oamInfo_t) * EPON_OAM_SUPPORT_LLID_NUM);
    /* Set default local info OAM */
    memset(&infoOam, 0x0, sizeof(oam_oamInfo_t));
    infoOam.valid = 1;
    infoOam.oamVer = 1;
    infoOam.revision = 1;
    infoOam.state = 0;
    infoOam.oamConfig = 0x14;
    infoOam.oamPduConfig = 0x5ee;   /* 1518 */
    /* Realtek OUI */
    infoOam.oui[0] = 0x00;
    infoOam.oui[1] = 0xE0;
    infoOam.oui[2] = 0x4C;
    for(i = 0;i < EPON_OAM_SUPPORT_LLID_NUM;i++)
    {
        epon_oam_localInfo_set(i, &infoOam);

        /* Set default OAM enable and time */
        oamConfig[i].oamEnabled = 1;

        /* Set default auto register enable and time */
        oamConfig[i].autoRegEnable = 1;
        oamConfig[i].autoRegTime = 1000; /* ms */

        /* Set default holdover enable and time */
        oamConfig[i].holdoverEnable = 1;
        oamConfig[i].holdoverTime = 200; /* ms */

        /* Reset OAM related counters */
        epon_oam_counter_init(i, EPON_OAM_COUNTERTYPE_TX);
        epon_oam_counter_init(i, EPON_OAM_COUNTERTYPE_RX);
        epon_oam_counter_init(i, EPON_OAM_COUNTERTYPE_DROP);
        epon_oam_counter_init(i, EPON_OAM_COUNTERTYPE_LOSTLINK);
    }

    /* Init callback function database */
    pInfoOrgSpecCbDb = NULL;
    defInfoOrgSpecCb.parser = epon_oam_dummyParser;
    defInfoOrgSpecCb.handler = epon_oam_dummyHandler;
    pEventOrgSpecCbDb = NULL;
    defEventOrgSpecCb.parser = epon_oam_dummyParser;
    defEventOrgSpecCb.handler = epon_oam_dummyHandler;
    pOrgSpecCbDb = NULL;
    defOrgSpecCb.processor = epon_oam_dummyProcessor;
    for(i = 0;i < EPON_OAM_VARIABLEDB_HASH_NUM;i++)
    {
        pVarCbDb[i] = NULL;
    }
    defVarCb = epon_oam_dummyVarHandler;

    /* Register object identifier number */
    epon_oam_initObjIdNum();

    return EPON_OAM_ERR_OK;
}

/* Function Name:
 *      epon_oam_config_get
 * Description:
 *      Get the OAM protocol configuration for each LLID index
 * Input:
 *      llidIdx - LLID index of EPON module
 * Output:
 *      pOamConfig - pointer buffer of returned OAM configuration
 * Return:
 *      EPON_OAM_ERR_OK
 *      EPON_OAM_ERR_PARAM - input parameter invalid
 * Note:
 *      None
 */
int epon_oam_config_get(
    unsigned char llidIdx,
    oam_config_t *pOamConfig)
{
    /* Parameter check */
    if(llidIdx >= EPON_OAM_SUPPORT_LLID_NUM)
    {
        return EPON_OAM_ERR_PARAM;
    }

    if(NULL == pOamConfig)
    {
        return EPON_OAM_ERR_PARAM;
    }

    sem_wait(&oamDbAccessSem);
    *pOamConfig = oamConfig[llidIdx];
    sem_post(&oamDbAccessSem);

    return EPON_OAM_ERR_OK;
}

/* Function Name:
 *      epon_oam_config_set
 * Description:
 *      Set the OAM protocol  configuration for each LLID index
 * Input:
 *      llidIdx - LLID index of EPON module
 *      pOamConfig - pointer buffer of input OAM configuration
 * Output:
 *      None
 * Return:
 *      EPON_OAM_ERR_OK
 *      EPON_OAM_ERR_PARAM - input parameter invalid
 * Note:
 *      None
 */
int epon_oam_config_set(
    unsigned char llidIdx,
    oam_config_t *pOamConfig)
{
    /* Parameter check */
    if(llidIdx >= EPON_OAM_SUPPORT_LLID_NUM)
    {
        return EPON_OAM_ERR_PARAM;
    }

    if(NULL == pOamConfig)
    {
        return EPON_OAM_ERR_PARAM;
    }

    sem_wait(&oamDbAccessSem);
    oamConfig[llidIdx] = *pOamConfig;
    sem_post(&oamDbAccessSem);

    return EPON_OAM_ERR_OK;
}

/* Function Name:
 *      epon_oam_localInfo_get
 * Description:
 *      Get the OAM local information settings
 * Input:
 *      llidIdx - LLID index of EPON module
 * Output:
 *      pOamInfo- pointer buffer of returned OAM information
 * Return:
 *      EPON_OAM_ERR_OK
 *      EPON_OAM_ERR_PARAM - input parameter invalid
 * Note:
 *      None
 */
int epon_oam_localInfo_get(
    unsigned char llidIdx,
    oam_oamInfo_t *pOamInfo)
{
    /* Parameter check */
    if(llidIdx >= EPON_OAM_SUPPORT_LLID_NUM)
    {
        return EPON_OAM_ERR_PARAM;
    }

    if(NULL == pOamInfo)
    {
        return EPON_OAM_ERR_PARAM;
    }

    sem_wait(&oamDbAccessSem);
    *pOamInfo = localOamInfo[llidIdx];
    sem_post(&oamDbAccessSem);

    return EPON_OAM_ERR_OK;
}

/* Function Name:
 *      epon_oam_localInfo_set
 * Description:
 *      Set the OAM local information settings
 * Input:
 *      llidIdx - LLID index of EPON module
 *      pOamInfo- pointer buffer of input OAM information
 * Output:
 *      None
 * Return:
 *      EPON_OAM_ERR_OK
 *      EPON_OAM_ERR_PARAM - input parameter invalid
 * Note:
 *      None
 */
int epon_oam_localInfo_set(
    unsigned char llidIdx,
    oam_oamInfo_t *pOamInfo)
{
    /* Parameter check */
    if(llidIdx >= EPON_OAM_SUPPORT_LLID_NUM)
    {
        return EPON_OAM_ERR_PARAM;
    }

    if(NULL == pOamInfo)
    {
        return EPON_OAM_ERR_PARAM;
    }

    sem_wait(&oamDbAccessSem);
    localOamInfo[llidIdx] = *pOamInfo;
    sem_post(&oamDbAccessSem);
    DUMP_INFO_OAMPDU(llidIdx, &localOamInfo[llidIdx]);

    return EPON_OAM_ERR_OK;
}

/* Function Name:
 *      epon_oam_remoteInfo_get
 * Description:
 *      Get the OAM remote information settings
 * Input:
 *      llidIdx - LLID index of EPON module
 * Output:
 *      pOamInfo- pointer buffer of returned OAM information
 * Return:
 *      EPON_OAM_ERR_OK
 *      EPON_OAM_ERR_PARAM - input parameter invalid
 * Note:
 *      None
 */
int epon_oam_remoteInfo_get(
    unsigned char llidIdx,
    oam_oamInfo_t *pOamInfo)
{
    /* Parameter check */
    if(llidIdx >= EPON_OAM_SUPPORT_LLID_NUM)
    {
        return EPON_OAM_ERR_PARAM;
    }

    if(NULL == pOamInfo)
    {
        return EPON_OAM_ERR_PARAM;
    }

    sem_wait(&oamDbAccessSem);
    *pOamInfo = remoteOamInfo[llidIdx];
    sem_post(&oamDbAccessSem);

    return EPON_OAM_ERR_OK;
}

/* Function Name:
 *      epon_oam_remoteInfo_set
 * Description:
 *      Set the OAM remote information settings
 * Input:
 *      llidIdx - LLID index of EPON module
 *      pOamInfo- pointer buffer of input OAM information
 * Output:
 *      None
 * Return:
 *      EPON_OAM_ERR_OK
 *      EPON_OAM_ERR_PARAM - input parameter invalid
 * Note:
 *      None
 */
int epon_oam_remoteInfo_set(
    unsigned char llidIdx,
    oam_oamInfo_t *pOamInfo)
{
    /* Parameter check */
    if(llidIdx >= EPON_OAM_SUPPORT_LLID_NUM)
    {
        return EPON_OAM_ERR_PARAM;
    }

    if(NULL == pOamInfo)
    {
        return EPON_OAM_ERR_PARAM;
    }

    sem_wait(&oamDbAccessSem);
    remoteOamInfo[llidIdx] = *pOamInfo;
    sem_post(&oamDbAccessSem);
    DUMP_INFO_OAMPDU(llidIdx, &remoteOamInfo[llidIdx]);

    return EPON_OAM_ERR_OK;
}

/* Function Name:
 *      epon_oam_remoteInfo_init
 * Description:
 *      Clear the OAM remote information settings
 * Input:
 *      llidIdx - LLID index of EPON module
 * Output:
 *      None
 * Return:
 *      EPON_OAM_ERR_OK
 *      EPON_OAM_ERR_PARAM - input parameter invalid
 * Note:
 *      None
 */
int epon_oam_remoteInfo_init(
    unsigned char llidIdx)
{
    /* Parameter check */
    if(llidIdx >= EPON_OAM_SUPPORT_LLID_NUM)
    {
        return EPON_OAM_ERR_PARAM;
    }

    sem_wait(&oamDbAccessSem);
    memset(&remoteOamInfo[llidIdx], 0x0, sizeof(oam_oamInfo_t));
    sem_post(&oamDbAccessSem);

    return EPON_OAM_ERR_OK;
}

/* Function Name:
 *      epon_oam_orgSpecCb_reg
 * Description:
 *      Register callback function for organization specific OAM fields
 * Input:
 *      regType - state the type of callback function to be registed
 *      pOui    - OUI of the callback function, this is the search key
 *      pCbData - virtual function pointer of the callback function
 * Output:
 *      None
 * Return:
 *      EPON_OAM_ERR_OK
 *      EPON_OAM_ERR_PARAM - input parameter invalid
 *      EPON_OAM_ERR_MEM - memory allocation failed
 * Note:
 *      Register to the exist regType/out tuple will replace the exist callback
 */
int epon_oam_orgSpecCb_reg(
    oam_cbType_t regType,
    unsigned char *pOui,
    void *pCbData)
{
    int ret = EPON_OAM_ERR_OK;

    /* Parameter check */
    if(regType >= EPON_OAM_CBTYPE_END)
    {
        return EPON_OAM_ERR_PARAM;
    }

    if((NULL == pOui) || (NULL == pCbData))
    {
        return EPON_OAM_ERR_PARAM;
    }

    sem_wait(&oamDbAccessSem);
    switch(regType)
    {
    case EPON_OAM_CBTYPE_INFO_ORGSPEC:
        ret = epon_oam_orgSpecInfo_reg(pOui, (oam_infoOrgSpecCb_t *)pCbData);
        break;
    case EPON_OAM_CBTYPE_EVENT_ORGSPEC:
        ret = epon_oam_orgSpecEvent_reg(pOui, (oam_eventOrgSpecCb_t *)pCbData);
        break;
    case EPON_OAM_CBTYPE_ORGSPEC:
        ret = epon_oam_orgSpec_reg(pOui, (oam_orgSpecCb_t *)pCbData);
        break;
    default:
        ret = EPON_OAM_ERR_PARAM;
        break;
    }
    sem_post(&oamDbAccessSem);

    return ret;
}

/* Function Name:
 *      epon_oam_orgSpecCb_dereg
 * Description:
 *      Deregister callback function for organization specific OAM fields
 * Input:
 *      regType - state the type of callback function to be deregisted
 *      pOui    - OUI of the callback function, this is the search key
 * Output:
 *      None
 * Return:
 *      EPON_OAM_ERR_OK
 *      EPON_OAM_ERR_PARAM - input parameter invalid
 * Note:
 *      Register to the exist regType/out tuple will replace the exist callback
 */
int epon_oam_orgSpecCb_dereg(
    oam_cbType_t regType,
    unsigned char *pOui)
{
    int ret = EPON_OAM_ERR_OK;

    /* Parameter check */
    if(regType >= EPON_OAM_CBTYPE_END)
    {
        return EPON_OAM_ERR_PARAM;
    }

    if(NULL == pOui)
    {
        return EPON_OAM_ERR_PARAM;
    }

    sem_wait(&oamDbAccessSem);
    switch(regType)
    {
    case EPON_OAM_CBTYPE_INFO_ORGSPEC:
        ret = epon_oam_orgSpecInfo_dereg(pOui);
        break;
    case EPON_OAM_CBTYPE_EVENT_ORGSPEC:
        ret = epon_oam_orgSpecEvent_dereg(pOui);
        break;
    case EPON_OAM_CBTYPE_ORGSPEC:
        ret = epon_oam_orgSpec_dereg(pOui);
        break;
    default:
        ret = EPON_OAM_ERR_PARAM;
        break;
    }
    sem_post(&oamDbAccessSem);

    return ret;
}

/* Function Name:
 *      epon_oam_orgSpecCb_get
 * Description:
 *      Get callback function for organization specific OAM fields
 * Input:
 *      regType - state the type of callback function to be get
 *      pOui    - OUI of the callback function, this is the search key
 * Output:
 *      pCbData - virtual function pointer of the callback function
 * Return:
 *      EPON_OAM_ERR_OK
 *      EPON_OAM_ERR_PARAM - input parameter invalid
 *      EPON_OAM_ERR_NOT_FOUND - No such callback function
 * Note:
 *      None
 */
int epon_oam_orgSpecCb_get(
    oam_cbType_t regType,
    unsigned char *pOui,
    void *pCbData)
{
    int ret = EPON_OAM_ERR_OK;
    oam_infoOrgSpecCbRec_t *pInfoCbRec;
    oam_eventOrgSpecCbRec_t *pEventCbRec;
    oam_orgSpecCbRec_t *pOrgSpecCbRec;

    /* Parameter check */
    if(regType >= EPON_OAM_CBTYPE_END)
    {
        return EPON_OAM_ERR_PARAM;
    }

    if((NULL == pOui) || (NULL == pCbData))
    {
        return EPON_OAM_ERR_PARAM;
    }

    sem_wait(&oamDbAccessSem);
    switch(regType)
    {
    case EPON_OAM_CBTYPE_INFO_ORGSPEC:
        pInfoCbRec = epon_oam_orgSpecInfo_get(pOui);
        if(NULL == pInfoCbRec)
        {
            ret = EPON_OAM_ERR_NOT_FOUND;
        }
        else
        {
            ((oam_infoOrgSpecCb_t *) pCbData)->parser = pInfoCbRec->parser;
            ((oam_infoOrgSpecCb_t *) pCbData)->handler = pInfoCbRec->handler;
        }
        break;
    case EPON_OAM_CBTYPE_EVENT_ORGSPEC:
        pEventCbRec = epon_oam_orgSpecEvent_get(pOui);
        if(NULL == pEventCbRec)
        {
            ret = EPON_OAM_ERR_NOT_FOUND;
        }
        else
        {
            ((oam_eventOrgSpecCb_t *) pCbData)->parser = pEventCbRec->parser;
            ((oam_eventOrgSpecCb_t *) pCbData)->handler = pEventCbRec->handler;
        }
        break;
    case EPON_OAM_CBTYPE_ORGSPEC:
        pOrgSpecCbRec = epon_oam_orgSpec_get(pOui);
        if(NULL == pOrgSpecCbRec)
        {
            ret = EPON_OAM_ERR_NOT_FOUND;
        }
        else
        {
            ((oam_orgSpecCb_t *) pCbData)->processor = pOrgSpecCbRec->processor;
        }
        break;
    default:
        ret = EPON_OAM_ERR_PARAM;
        break;
    }
    sem_post(&oamDbAccessSem);
    
    return ret;
}

/* Function Name:
 *      epon_oam_orgSpecCb_reg
 * Description:
 *      Register callback function for variable with branch/leaf
 * Input:
 *      pVarCb  - branch/leaf of variable to be registered
 *                virtual function pointer of the callback function and
 *                variable type to be registered
 * Output:
 *      None
 * Return:
 *      EPON_OAM_ERR_OK
 *      EPON_OAM_ERR_PARAM - input parameter invalid
 *      EPON_OAM_ERR_MEM - memory allocation failed
 * Note:
 *      Register to the exist regType/out tuple will replace the exist callback
 */
int epon_oam_variableCb_reg(
    oam_varCb_t *pVarCb)
{
    if(NULL == pVarCb)
    {
        return EPON_OAM_ERR_PARAM;
    }

    return epon_oam_variable_reg(pVarCb);
}

/* Function Name:
 *      epon_oam_variableCb_dereg
 * Description:
 *      Deregister callback function for variable with branch/leaf
 * Input:
 *      varDesc - branch/leaf of variable to be deregistered
 * Output:
 *      None
 * Return:
 *      EPON_OAM_ERR_OK
 * Note:
 *      Register to the exist regType/out tuple will replace the exist callback
 */
int epon_oam_variableCb_dereg(
    oam_varDescriptor_t varDesc)
{
    return epon_oam_variable_dereg(&varDesc);
}

/* Function Name:
 *      epon_oam_variableCb_get
 * Description:
 *      Get callback function for variable with branch/leaf
 * Input:
 *      pVarCb  - branch/leaf of variable to be get
 * Output:
 *      pVarCb  - virtual function pointer of the callback function and
 *                variable type
 * Return:
 *      EPON_OAM_ERR_OK
 *      EPON_OAM_ERR_PARAM - input parameter invalid
 *      EPON_OAM_ERR_NOT_FOUND - No such callback function
 * Note:
 *      None
 */
int epon_oam_variableCb_get(
    oam_varCb_t *pVarCb)
{
    oam_varDescriptor_t varDesc;
    oam_varCbRec_t *pVarCbRec;

    if(NULL == pVarCb)
    {
        return EPON_OAM_ERR_PARAM;
    }

    varDesc = pVarCb->varDesc;
    pVarCbRec = epon_oam_variable_get(&varDesc);
    if(NULL != pVarCbRec)
    {
        pVarCb->varType = pVarCbRec->varType;
        pVarCb->handler = pVarCbRec->handler;
        return EPON_OAM_ERR_OK;
    }
    else
    {
        return EPON_OAM_ERR_NOT_FOUND;
    }
}

/* Function Name:
 *      epon_oam_defOrgSpecCb_set
 * Description:
 *      Set default callback function for organization specific OAM fields
 * Input:
 *      regType - state the type of callback function to be get
 *      pCbData - virtual function pointer of the callback function
 * Output:
 *      None
 * Return:
 *      EPON_OAM_ERR_OK
 *      EPON_OAM_ERR_PARAM - input parameter invalid
 * Note:
 *      None
 */
int epon_oam_defOrgSpecCb_set(
    oam_cbType_t regType,
    void *pCbData)
{
    /* Parameter check */
    if(regType >= EPON_OAM_CBTYPE_END)
    {
        return EPON_OAM_ERR_PARAM;
    }

    if(NULL == pCbData)
    {
        return EPON_OAM_ERR_PARAM;
    }

    switch(regType)
    {
    case EPON_OAM_CBTYPE_INFO_ORGSPEC:
        defInfoOrgSpecCb.parser = ((oam_infoOrgSpecCb_t *)pCbData)->parser;
        defInfoOrgSpecCb.handler = ((oam_infoOrgSpecCb_t *)pCbData)->handler;
        break;
    case EPON_OAM_CBTYPE_EVENT_ORGSPEC:
        defEventOrgSpecCb.parser = ((oam_eventOrgSpecCb_t *)pCbData)->parser;
        defEventOrgSpecCb.handler = ((oam_eventOrgSpecCb_t *)pCbData)->handler;
        break;
    case EPON_OAM_CBTYPE_ORGSPEC:
        defOrgSpecCb.processor = ((oam_orgSpecCb_t *)pCbData)->processor;
        break;
    default:
        break;
    }
    
    return EPON_OAM_ERR_OK;
}

/* Function Name:
 *      epon_oam_defOrgSpecCb_get
 * Description:
 *      Get default callback function for organization specific OAM fields
 * Input:
 *      regType - state the type of callback function to be get
 * Output:
 *      pCbData - virtual function pointer of the callback function
 * Return:
 *      EPON_OAM_ERR_OK
 *      EPON_OAM_ERR_PARAM - input parameter invalid
 * Note:
 *      None
 */
int epon_oam_defOrgSpecCb_get(
    oam_cbType_t regType,
    void *pCbData)
{
    /* Parameter check */
    if(regType >= EPON_OAM_CBTYPE_END)
    {
        return EPON_OAM_ERR_PARAM;
    }

    if(NULL == pCbData)
    {
        return EPON_OAM_ERR_PARAM;
    }

    switch(regType)
    {
    case EPON_OAM_CBTYPE_INFO_ORGSPEC:
        ((oam_infoOrgSpecCb_t *)pCbData)->parser = defInfoOrgSpecCb.parser;
        ((oam_infoOrgSpecCb_t *)pCbData)->handler = defInfoOrgSpecCb.handler;
        break;
    case EPON_OAM_CBTYPE_EVENT_ORGSPEC:
        ((oam_eventOrgSpecCb_t *)pCbData)->parser = defEventOrgSpecCb.parser;
        ((oam_eventOrgSpecCb_t *)pCbData)->handler = defEventOrgSpecCb.handler;
        break;
    case EPON_OAM_CBTYPE_ORGSPEC:
        ((oam_orgSpecCb_t *)pCbData)->processor = defOrgSpecCb.processor;
        break;
    default:
        break;
    }
    
    return EPON_OAM_ERR_OK;
}

/* Function Name:
 *      epon_oam_defVarCb_set
 * Description:
 *      Set default callback function for variable handling
 * Input:
 *      handler - virtual function pointer of the callback function
 * Output:
 *      None
 * Return:
 *      EPON_OAM_ERR_OK
 *      EPON_OAM_ERR_PARAM - input parameter invalid
 * Note:
 *      None
 */
int epon_oam_defVarCb_set(
    int (*handler)(
    oam_varTarget_t *pVarTarget,
    oam_varDescriptor_t varDesc,
    oam_varContainer_t **ppVarContainer))
{
    if(NULL == handler)
    {
        return EPON_OAM_ERR_PARAM;
    }

    defVarCb = handler;

    return EPON_OAM_ERR_OK;
}

/* Function Name:
 *      epon_oam_defVarCb_get
 * Description:
 *      Get default callback function for variable handling
 * Input:
 *      None
 * Output:
 *      handler - virtual function pointer of the callback function
 * Return:
 *      EPON_OAM_ERR_OK
 *      EPON_OAM_ERR_PARAM - input parameter invalid
 * Note:
 *      None
 */
int epon_oam_defVarCb_get(
    int (**handler)(
    oam_varTarget_t *pVarTarget,
    oam_varDescriptor_t varDesc,
    oam_varContainer_t **ppVarContainer))
{
    if(NULL == handler)
    {
        return EPON_OAM_ERR_PARAM;
    }

    *handler = defVarCb;

    return EPON_OAM_ERR_OK;
}

/* Function Name:
 *      epon_oam_counter_init
 * Description:
 *      Get default callback function for variable handling
 * Input:
 *      llidIdx - LLID index of the counter
 *      counterType - type of counter to be init
 * Output:
 *      None
 * Return:
 *      EPON_OAM_ERR_OK
 * Note:
 *      Incorrect counterType will be ignored
 */
int epon_oam_counter_init(
    unsigned short llidIdx,
    oam_counterType_t counterType)
{
    if(counterType >= EPON_OAM_COUNTERTYPE_END)
    {
        return EPON_OAM_ERR_OK;
    }

    sem_wait(&oamDbAccessSem);
    switch(counterType)
    {
    case EPON_OAM_COUNTERTYPE_TX:
        oamCounter[llidIdx].oamTx = 0;
        break;
    case EPON_OAM_COUNTERTYPE_RX:
        oamCounter[llidIdx].oamRx = 0;
        break;
    case EPON_OAM_COUNTERTYPE_TXINFO:
        oamCounter[llidIdx].oamTxInfo = 0;
        break;
    case EPON_OAM_COUNTERTYPE_RXINFO:
        oamCounter[llidIdx].oamRxInfo = 0;
        break;
    case EPON_OAM_COUNTERTYPE_TXORGSPEC:
        oamCounter[llidIdx].oamTxOrgSpec = 0;
        break;
    case EPON_OAM_COUNTERTYPE_RXORGSPEC:
        oamCounter[llidIdx].oamRxOrgSpec = 0;
        break;
    case EPON_OAM_COUNTERTYPE_DROP:
        oamCounter[llidIdx].oamDrop = 0;
        break;
    case EPON_OAM_COUNTERTYPE_LOSTLINK:
        oamCounter[llidIdx].oamLostLink = 0;
        break;
    }
    sem_post(&oamDbAccessSem);

    return EPON_OAM_ERR_OK;
}

/* Function Name:
 *      epon_oam_counter_inc
 * Description:
 *      Get default callback function for variable handling
 * Input:
 *      llidIdx - LLID index of the counter
 *      counterType - type of counter to be increased
 * Output:
 *      None
 * Return:
 *      EPON_OAM_ERR_OK
 * Note:
 *      Incorrect counterType will be ignored
 */
int epon_oam_counter_inc(
    unsigned short llidIdx,
    oam_counterType_t counterType)
{
    if(counterType >= EPON_OAM_COUNTERTYPE_END)
    {
        return EPON_OAM_ERR_OK;
    }

    sem_wait(&oamDbAccessSem);
    switch(counterType)
    {
    case EPON_OAM_COUNTERTYPE_TX:
        oamCounter[llidIdx].oamTx++;
        break;
    case EPON_OAM_COUNTERTYPE_RX:
        oamCounter[llidIdx].oamRx++;
        break;
    case EPON_OAM_COUNTERTYPE_TXINFO:
        oamCounter[llidIdx].oamTxInfo++;
        break;
    case EPON_OAM_COUNTERTYPE_RXINFO:
        oamCounter[llidIdx].oamRxInfo++;
        break;
    case EPON_OAM_COUNTERTYPE_TXORGSPEC:
        oamCounter[llidIdx].oamTxOrgSpec++;
        break;
    case EPON_OAM_COUNTERTYPE_RXORGSPEC:
        oamCounter[llidIdx].oamRxOrgSpec++;
        break;
    case EPON_OAM_COUNTERTYPE_DROP:
        oamCounter[llidIdx].oamDrop++;
        break;
    case EPON_OAM_COUNTERTYPE_LOSTLINK:
        oamCounter[llidIdx].oamLostLink++;
        break;
    }
    sem_post(&oamDbAccessSem);

    return EPON_OAM_ERR_OK;
}

/* Function Name:
 *      epon_oam_counter_get
 * Description:
 *      Get default callback function for variable handling
 * Input:
 *      llidIdx - LLID index of the counter
 *      counterType - type of counter to be get
 * Output:
 *      None
 * Return:
 *      The counter value specified by counterType
 * Note:
 *      Incorrect counterType will be ignored and always return 0
 */
unsigned int epon_oam_counter_get(
    unsigned short llidIdx,
    oam_counterType_t counterType)
{
    unsigned int value;

    if(counterType >= EPON_OAM_COUNTERTYPE_END)
    {
        return 0;
    }

    sem_wait(&oamDbAccessSem);
    switch(counterType)
    {
    case EPON_OAM_COUNTERTYPE_TX:
        value = oamCounter[llidIdx].oamTx;
        break;
    case EPON_OAM_COUNTERTYPE_RX:
        value = oamCounter[llidIdx].oamRx;
        break;
    case EPON_OAM_COUNTERTYPE_TXINFO:
        value = oamCounter[llidIdx].oamTxInfo;
        break;
    case EPON_OAM_COUNTERTYPE_RXINFO:
        value = oamCounter[llidIdx].oamRxInfo;
        break;
    case EPON_OAM_COUNTERTYPE_TXORGSPEC:
        value = oamCounter[llidIdx].oamTxOrgSpec;
        break;
    case EPON_OAM_COUNTERTYPE_RXORGSPEC:
        value = oamCounter[llidIdx].oamRxOrgSpec;
        break;
    case EPON_OAM_COUNTERTYPE_DROP:
        value = oamCounter[llidIdx].oamDrop;
        break;
    case EPON_OAM_COUNTERTYPE_LOSTLINK:
        value = oamCounter[llidIdx].oamLostLink;
        break;
    }
    sem_post(&oamDbAccessSem);

    return value;
}

/* Function Name:
 *      epon_oam_oamDb_init
 * Description:
 *      This function is called when the OAM disconnected with OLT
 * Input:
 *      llidIdx - LLID index
 * Output:
 *      None
 * Return:
 *      EPON_OAM_ERR_PARAM
 * Note:
 *      None
 */
unsigned int epon_oam_oamDb_init(
    unsigned short llidIdx)
{
    /* Parameter check */
    if(llidIdx >= EPON_OAM_SUPPORT_LLID_NUM)
    {
        return EPON_OAM_ERR_PARAM;
    }

    sem_wait(&oamDbAccessSem);
    memset(&oamConfig[llidIdx].macAddr[0], 0, sizeof(oamConfig[llidIdx].macAddr));
    sem_post(&oamDbAccessSem);
}

