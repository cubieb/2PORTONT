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
 * Purpose : Define the error code for EPON OAM protocol stack
 *
 * Feature : 
 *
 */

#ifndef __EPON_OAM_ERR_H__
#define __EPON_OAM_ERR_H__

/* 
 * Symbol Definition 
 */
enum {
    EPON_OAM_ERR_OK         = 0,  /* Function complete without error */
    EPON_OAM_ERR_PARAM      = -1, /* Input parameter invalid */
    EPON_OAM_ERR_MEM        = -2, /* Memory allocation failed */
    EPON_OAM_ERR_NOT_FOUND  = -3, /* Can't find specified item */
    EPON_OAM_ERR_PARSE      = -4, /* OAMPDU parsing failed */
    EPON_OAM_ERR_HANDLE     = -5, /* OAMPDU handle failed */
    EPON_OAM_ERR_LENGTH     = -6, /* Reply length overflow */
    EPON_OAM_ERR_MSGQ       = -7, /* Message queue operation error */
    EPON_OAM_ERR_ORG        = -8, /* Organization specific callback error */
    EPON_OAM_ERR_UNKNOWN    = -9, /* Unknonw error */
};

#endif /* __EPON_OAM_ERR_H__ */

