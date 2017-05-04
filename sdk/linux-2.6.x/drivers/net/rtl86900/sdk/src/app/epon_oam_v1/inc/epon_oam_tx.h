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
 * $Revision: 39101 $
 * $Date: 2013-05-03 17:35:27 +0800 (週五, 03 五月 2013) $
 *
 * Purpose : Define basic Tx OAMPDU constants
 *
 * Feature : 
 *
 */

#ifndef __EPON_OAM_TX_H__
#define __EPON_OAM_TX_H__

/*
 * Include Files
 */

/* 
 * Symbol Definition 
 */
typedef struct oam_eventOrgSpec_s {
    unsigned char oui[EPON_OAM_OUI_LENGTH];
    unsigned char eventLen; /* This length include the eventData only */
    unsigned char eventData[EPON_EVENT_OAMPDU_ORGSPEC_MAX_LEN];
    struct oam_eventOrgSpec_s *pNext;
} oam_eventOrgSpec_t;

/*
 * Macro Definition
 */

/*  
 * Function Declaration  
 */ 
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
extern int epon_oam_orgSpecEvtNotification_send(
    unsigned short llidIdx,
    oam_eventOrgSpec_t *pEvent);

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
extern int epon_oam_reply_send(
    unsigned short llidIdx,
    unsigned char code,
    unsigned char *pReplyPayload,
    unsigned short replyLen);

#endif /* __EPON_OAM_TX_H__ */

