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
 * Purpose : 
 *
 * Feature : 
 *
 */

#ifndef __EPON_OAM_DB_H__
#define __EPON_OAM_DB_H__

/*
 * Include Files
 */

/* 
 * Symbol Definition 
 */
/* Change EPON_OAM_VARIABLEDB_HASH_POWER will need to revise epon_oam_variableDb_hash too */
#define EPON_OAM_VARIABLEDB_HASH_POWER  4  /* 2 ^ 4 = 16 */
#define EPON_OAM_VARIABLEDB_HASH_NUM    (1 << EPON_OAM_VARIABLEDB_HASH_POWER)

typedef struct oam_oamPdu_s {
    unsigned char  llidIdx;
    unsigned char  srcMacAddr[6];
    unsigned short flag;
    unsigned char  code;
    unsigned short dataLen;
    unsigned char  *pData;
} oam_oamPdu_t;

typedef enum oam_cbType_e {
    EPON_OAM_CBTYPE_INFO_ORGSPEC = 1,
    EPON_OAM_CBTYPE_EVENT_ORGSPEC,
    EPON_OAM_CBTYPE_ORGSPEC,
    EPON_OAM_CBTYPE_END
} oam_cbType_t;

typedef enum oam_varType_e {
    EPON_OAM_VARTYPE_ATTRIBUTE = 1,
    EPON_OAM_VARTYPE_PACKAGE   = 2,
    EPON_OAM_VARTYPE_OBJECT    = 3,
    EPON_OAM_VARTYPE_OBJECTID  = 4,
} oam_varType_t;

typedef enum oam_counterType_e {
    EPON_OAM_COUNTERTYPE_TX         = 1,
    EPON_OAM_COUNTERTYPE_RX         = 2,
    EPON_OAM_COUNTERTYPE_TXINFO     = 3,
    EPON_OAM_COUNTERTYPE_RXINFO     = 4,
    EPON_OAM_COUNTERTYPE_TXORGSPEC  = 5,
    EPON_OAM_COUNTERTYPE_RXORGSPEC  = 6,
    EPON_OAM_COUNTERTYPE_DROP       = 7,
    EPON_OAM_COUNTERTYPE_LOSTLINK   = 8,
    EPON_OAM_COUNTERTYPE_END,
} oam_counterType_t;

typedef struct oam_config_s {
   unsigned char oamEnabled;        /* Enable OAM discovery process */
   unsigned char macAddr[6];        /* MAC address through out all OAM protocol */
   unsigned short autoRegEnable;    /* Auto register */
   unsigned short autoRegTime;      /* Auto register time (ms)*/
   unsigned short holdoverEnable;   /* For CTC holdover feature */
   unsigned short holdoverTime;     /* For CTC holdover time (ms) */
   unsigned short eventRepCnt;      /* Maximum number of event resend */
   unsigned short eventRepIntvl;    /* Interval between event resend */
} oam_config_t;

typedef struct oam_oamInfo_s {
    unsigned char valid;            /* If the OAM info valid */
    unsigned char oamVer;           /* OAM version defined in 802.3 section 57 */
    unsigned short revision;        /* Revision defined in 802.3 section 57 */
    unsigned char state;            /* State defined in 802.3 section 57 */
    unsigned char oamConfig;        /* OAM config defined in 802.3 section 57 */
    unsigned short oamPduConfig;    /* OAMPDU config defined in 802.3 section 57 */
    unsigned char oui[3];           /* OUI version defined in 802.3 section 57 */
    unsigned char venderSpecInfo[4];/* Vender Specific Info defined in 802.3 section 57 */
} oam_oamInfo_t;

typedef struct oam_infoOrgSpecCb_s {
    int (*parser)(
        unsigned char llidIdx,      /* LLID index from HW table */
        unsigned char *pFrame,      /* Frame payload current pointer */
        unsigned short length,      /* Frame payload length */
        unsigned short *pExtractLen,/* Parser extract length */
        void **ppOrgSpecData);      /* Orgnization specific data */
    int (*handler)(
        oam_oamPdu_t *pOamPdu,      /* OAMPDU data */
        unsigned char *pReplyBuf,   /* Frame buffer for reply OAM */
        unsigned short bufLen,      /* Frame buffer size */
        unsigned short *pReplyLen,  /* Reply size used by this handler*/
        void **ppOrgSpecData);      /* Orgnization specific data
                                     * Direct comes from parser
                                     */
} oam_infoOrgSpecCb_t;

typedef struct oam_eventOrgSpecCb_s {
    int (*parser)(
        unsigned char llidIdx,      /* LLID index from HW table */
        unsigned char *pFrame,      /* Frame payload current pointer */
        unsigned short length,      /* Frame payload length */
        unsigned short *pExtractLen,/* Parser extract length */
        void **ppOrgSpecData);      /* Orgnization specific data */
    int (*handler)(
        oam_oamPdu_t *pOamPdu,      /* OAMPDU data */
        unsigned char *pReplyBuf,   /* Frame buffer for reply OAM */
        unsigned short bufLen,      /* Frame buffer size */
        unsigned short *pReplyLen,  /* Reply size used by this handler*/
        void **ppOrgSpecData);      /* Orgnization specific data
                                     * Direct comes from parser
                                     */
} oam_eventOrgSpecCb_t;

typedef struct oam_varDescriptor_s {
    unsigned char varBranch;
    unsigned short varLeaf;
} oam_varDescriptor_t;

typedef struct oam_varContainerResp_s {
    unsigned char varWidth;
    unsigned char *varData;
    struct oam_varContainerResp_s *next;
} oam_varContainerResp_t;

typedef struct oam_varContainer_s {
    oam_varDescriptor_t varDesc;
    oam_varContainerResp_t *pVarResp;
} oam_varContainer_t;

/* Special variable container for object identifier number */
typedef struct oam_varTarget_s {
    oam_varDescriptor_t varDesc;
    unsigned char varWidth; /* Width is fixed at 2 for object idendifier number */
    unsigned char varData[2]; /* 2 octect data for object identifier number */
} oam_varTarget_t;

typedef struct oam_varCb_s {
    unsigned char varType;
    oam_varDescriptor_t varDesc;
    int (*handler)(
        oam_varTarget_t *pVarTarget, /* It might be NULL if no target specified */
        oam_varDescriptor_t varDesc,
        oam_varContainer_t **ppVarContainer); /* handler should allocate resource, 
                                              * caller will release it
                                              */
} oam_varCb_t;

typedef struct oam_orgSpecCb_s {
    int (*processor)(
        oam_oamPdu_t *pOamPdu,      /* OAMPDU data */
        unsigned char *pFrame,      /* Frame payload current pointer */
        unsigned short length,      /* Frame payload length */
        unsigned char *pReplyBuf,   /* Frame buffer for reply OAM */
        unsigned short bufLen,      /* Frame buffer size */
        unsigned short *pReplyLen); /* Reply size used by this handler*/
} oam_orgSpecCb_t;

typedef struct oam_counter_s {
    unsigned int oamTx;         /* Number of OAM transmited */
    unsigned int oamRx;         /* Number of OAM received */
    unsigned int oamTxInfo;     /* Information OAM transmited */
    unsigned int oamRxInfo;     /* Information OAM received */
    unsigned int oamTxOrgSpec;  /* Organization specifiec OAM transmited */
    unsigned int oamRxOrgSpec;  /* Organization specifiec OAM received */
    unsigned int oamDrop;       /* Number of OAM being dropped (Not replied) */
    unsigned int oamLostLink;   /* Number of OAM lost link */
} oam_counter_t;

/* ------------------------------------------------------------------------------
 * Protocol stack internal data structure (INTERNAL USEAGE ONLY)
 */
typedef struct oam_infoOrgSpecCbRec_s {
    unsigned char oui[3];
    int (*parser)(
        unsigned char llidIdx,      /* LLID index from HW table */
        unsigned char *pFrame,      /* Frame payload current pointer */
        unsigned short length,      /* Frame payload length */
        unsigned short *pExtractLen,/* Parser extract length */
        void **ppOrgSpecData);      /* Orgnization specific data */
    int (*handler)(
        oam_oamPdu_t *pOamPdu,      /* OAMPDU data */
        unsigned char *pReplyBuf,   /* Frame buffer for reply OAM */
        unsigned short bufLen,      /* Frame buffer size */
        unsigned short *pReplyLen,  /* Reply size used by this handler*/
        void **ppOrgSpecData);      /* Orgnization specific data
                                     * Direct comes from parser
                                     */
    struct oam_infoOrgSpecCbRec_s *prev;
    struct oam_infoOrgSpecCbRec_s *next;
} oam_infoOrgSpecCbRec_t;

typedef struct oam_eventOrgSpecCbRec_s {
    unsigned char oui[3];
    int (*parser)(
        unsigned char llidIdx,      /* LLID index from HW table */
        unsigned char *pFrame,      /* Frame payload current pointer */
        unsigned short length,      /* Frame payload length */
        unsigned short *pExtractLen,/* Parser extract length */
        void **ppOrgSpecData);      /* Orgnization specific data */
    int (*handler)(
        oam_oamPdu_t *pOamPdu,      /* OAMPDU data */
        unsigned char *pReplyBuf,   /* Frame buffer for reply OAM */
        unsigned short bufLen,      /* Frame buffer size */
        unsigned short *pReplyLen,  /* Reply size used by this handler*/
        void **ppOrgSpecData);      /* Orgnization specific data
                                     * Direct comes from parser
                                     */
    struct oam_eventOrgSpecCbRec_s *prev;
    struct oam_eventOrgSpecCbRec_s *next;
} oam_eventOrgSpecCbRec_t;

typedef struct oam_varCbRec_s {
    unsigned char varType;
    oam_varDescriptor_t varDesc;
    int (*handler)(
        oam_varTarget_t *pVarTarget, /* It might be NULL if no target specified */
        oam_varDescriptor_t varDesc,
        oam_varContainer_t **ppVarContainer); /* handler should allocate resource, 
                                              * caller will release it
                                              */
    struct oam_varCbRec_s *prev;
    struct oam_varCbRec_s *next;
} oam_varCbRec_t;

typedef struct oam_orgSpecCbRec_s {
    unsigned char oui[3];
    int (*processor)(
        oam_oamPdu_t *pOamPdu,      /* OAMPDU data */
        unsigned char *pFrame,      /* Frame payload current pointer */
        unsigned short frameLen,    /* Frame payload length */
        unsigned char *pReplyBuf,   /* Frame buffer for reply OAM */
        unsigned short bufLen,      /* Frame buffer size */
        unsigned short *pReplyLen); /* Reply size used by this handler*/
    struct oam_orgSpecCbRec_s *prev;
    struct oam_orgSpecCbRec_s *next;
} oam_orgSpecCbRec_t;

/*  
 * Function Declaration  
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
extern int epon_oam_database_init(void);

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
extern int epon_oam_config_set(
    unsigned char llidIdx,
    oam_config_t *pOamConfig);

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
extern int epon_oam_config_get(
    unsigned char llidIdx,
    oam_config_t *pOamConfig);

/* Function Name:
 *      epon_oam_localInfo_get
 * Description:
 *      Get the OAM local information settings
 * Input:
 *      llid    - LLID of EPON module
 * Output:
 *      pOamInfo- pointer buffer of returned OAM information
 * Return:
 *      EPON_OAM_ERR_OK
 *      EPON_OAM_ERR_PARAM - input parameter invalid
 * Note:
 *      None
 */
extern int epon_oam_localInfo_get(
    unsigned char llid,
    oam_oamInfo_t *pOamInfo);

/* Function Name:
 *      epon_oam_localInfo_set
 * Description:
 *      Set the OAM local information settings
 * Input:
 *      llid    - LLID of EPON module
 *      pOamInfo- pointer buffer of input OAM information
 * Output:
 *      None
 * Return:
 *      EPON_OAM_ERR_OK
 *      EPON_OAM_ERR_PARAM - input parameter invalid
 * Note:
 *      None
 */
extern int epon_oam_localInfo_set(
    unsigned char llid,
    oam_oamInfo_t *pOamInfo);

/* Function Name:
 *      epon_oam_remoteInfo_get
 * Description:
 *      Get the OAM remote information settings
 * Input:
 *      llid    - LLID of EPON module
 * Output:
 *      pOamInfo- pointer buffer of returned OAM information
 * Return:
 *      EPON_OAM_ERR_OK
 *      EPON_OAM_ERR_PARAM - input parameter invalid
 * Note:
 *      None
 */
extern int epon_oam_remoteInfo_get(
    unsigned char llid,
    oam_oamInfo_t *pOamInfo);

/* Function Name:
 *      epon_oam_remoteInfo_set
 * Description:
 *      Set the OAM remote information settings
 * Input:
 *      llid    - LLID of EPON module
 *      pOamInfo- pointer buffer of input OAM information
 * Output:
 *      None
 * Return:
 *      EPON_OAM_ERR_OK
 *      EPON_OAM_ERR_PARAM - input parameter invalid
 * Note:
 *      None
 */
extern int epon_oam_remoteInfo_set(
    unsigned char llid,
    oam_oamInfo_t *pOamInfo);

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
extern int epon_oam_remoteInfo_init(
    unsigned char llidIdx);

/* Function Name:
 *      epon_oam_orgSpecCb_reg
 * Description:
 *      Register callback function for organization specific OAM fields
 * Input:
 *      regType - state the type of callback function to be registed
 *      oui     - OUI of the callback function, this is the search key
 *      callback- virtual function pointer of the callback function
 * Output:
 *      None
 * Return:
 *      EPON_OAM_ERR_OK
 *      EPON_OAM_ERR_PARAM - input parameter invalid
 *      EPON_OAM_ERR_MEM - memory allocation failed
 * Note:
 *      Register to the exist regType/out tuple will replace the exist callback
 */
extern int epon_oam_orgSpecCb_reg(
    oam_cbType_t regType,
    unsigned char *oui,
    void *cbData);

/* Function Name:
 *      epon_oam_orgSpecCb_dereg
 * Description:
 *      Deregister callback function for organization specific OAM fields
 * Input:
 *      regType - state the type of callback function to be deregisted
 *      oui     - OUI of the callback function, this is the search key
 * Output:
 *      None
 * Return:
 *      EPON_OAM_ERR_OK
 *      EPON_OAM_ERR_PARAM - input parameter invalid
 * Note:
 *      Register to the exist regType/out tuple will replace the exist callback
 */
extern int epon_oam_orgSpecCb_dereg(
    oam_cbType_t regType,
    unsigned char *oui);

/* Function Name:
 *      epon_oam_orgSpecCb_get
 * Description:
 *      Get callback function for organization specific OAM fields
 * Input:
 *      regType - state the type of callback function to be get
 *      oui     - OUI of the callback function, this is the search key
 * Output:
 *      callback- virtual function pointer of the callback function
 * Return:
 *      EPON_OAM_ERR_OK
 *      EPON_OAM_ERR_PARAM - input parameter invalid
 *      EPON_OAM_ERR_NOT_FOUND - No such callback function
 * Note:
 *      None
 */
extern int epon_oam_orgSpecCb_get(
    oam_cbType_t regType,
    unsigned char *oui,
    void *cbData);

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
extern int epon_oam_variableCb_reg(
    oam_varCb_t *pVarCb);

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
extern int epon_oam_variableCb_dereg(
    oam_varDescriptor_t varDesc);

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
extern int epon_oam_variableCb_get(
    oam_varCb_t *pVarCb);

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
extern int epon_oam_defOrgSpecCb_set(
    oam_cbType_t regType,
    void *pCbData);

/* Function Name:
 *      epon_oam_defOrgSpecCb_get
 * Description:
 *      Get default callback function for organization specific OAM fields
 * Input:
 *      regType - state the type of callback function to be get
 * Output:
 *      callback- virtual function pointer of the callback function
 * Return:
 *      EPON_OAM_ERR_OK
 *      EPON_OAM_ERR_PARAM - input parameter invalid
 * Note:
 *      None
 */
extern int epon_oam_defOrgSpecCb_get(
    oam_cbType_t regType,
    void *cbData);

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
extern int epon_oam_counter_init(
    unsigned short llidIdx,
    oam_counterType_t counterType);

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
extern int epon_oam_counter_inc(
    unsigned short llidIdx,
    oam_counterType_t counterType);

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
extern unsigned int epon_oam_counter_get(
    unsigned short llidIdx,
    oam_counterType_t counterType);

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
extern unsigned int epon_oam_oamDb_init(
    unsigned short llidIdx);

#endif /* __EPON_OAM_DB_H__ */

