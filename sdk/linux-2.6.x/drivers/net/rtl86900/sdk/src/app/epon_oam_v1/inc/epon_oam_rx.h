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
 * Purpose : Define basic Rx OAMPDU constants
 *
 * Feature : 
 *
 */

#ifndef __EPON_OAM_RX_H__
#define __EPON_OAM_RX_H__

/*
 * Include Files
 */

/*  
 * Data Declaration  
 */
extern int pktRedirect_sock;

/* 
 * Symbol Definition 
 */
/* OAMPDU flag */
#define EPON_OAM_FLAG_LINK_FAULT            0x0001
#define EPON_OAM_FLAG_DYING_GASP            0x0002
#define EPON_OAM_FLAG_CRITICAL_EVENT        0x0004
#define EPON_OAM_FLAG_LOCAL_EVAL            0x0008
#define EPON_OAM_FLAG_LOCAL_STABLE          0x0010
#define EPON_OAM_FLAG_REMOTE_EVAL           0x0020
#define EPON_OAM_FLAG_REMOTE_STABLE         0x0040
/* OAMPDU Code */
#define EPON_OAMPDU_CODE_INFO               0x00
#define EPON_OAMPDU_CODE_EVENT              0x01
#define EPON_OAMPDU_CODE_VAR_REQ            0x02
#define EPON_OAMPDU_CODE_VAR_RESP           0x03
#define EPON_OAMPDU_CODE_LOOPBACK           0x04
#define EPON_OAMPDU_CODE_ORG_SPEC           0xFE
#define EPON_OAMPDU_CODE_RESERVED           0xFF
/* Info OAMPDU TLV types */
#define EPON_INFO_OAMPDU_TYPE_END           0x00
#define EPON_INFO_OAMPDU_TYPE_LOCAL         0x01
#define EPON_INFO_OAMPDU_TYPE_REMOTE        0x02
#define EPON_INFO_OAMPDU_TYPE_ORG_SPEC      0xFE
#define EPON_INFO_OAMPDU_TYPE_RESERVED      0xFF
/* Event OAMPDU TLV types */
#define EPON_EVENT_OAMPDU_TYPE_END          0x00
#define EPON_EVENT_OAMPDU_TYPE_ESPE         0x01 /* Errored Symbol Period Event */
#define EPON_EVENT_OAMPDU_TYPE_EFE          0x02 /* Errored Frame Event */
#define EPON_EVENT_OAMPDU_TYPE_EFPE         0x03 /* Errored Frame Period Evnet */
#define EPON_EVENT_OAMPDU_TYPE_EFSSE        0x04 /* Errored Frame Seconds Summary Event */
#define EPON_EVENT_OAMPDU_TYPE_ORG_SPEC     0xFE
#define EPON_EVENT_OAMPDU_TYPE_RESERVED     0xFF
/* Variable OAMPDU TLV types */
#define EPON_VAR_OAMPDU_TYPE_END            0x00
#define EPON_VAR_OAMPDU_TYPE_ATTR           0x07 /* Attribute */
#define EPON_VAR_OAMPDU_TYPE_PKG            0x04 /* Package */
#define EPON_VAR_OAMPDU_TYPE_OBJ            0x03 /* Object */
#define EPON_VAR_OAMPDU_IND                 0x80
#define EPON_VAR_OAMPDU_IND_LENGTH          0x01 /* Length of requested variable
                                                  * container(s) exceed OAMPDU data field
                                                  */
#define EPON_VAR_OAMPDU_IND_OBJEND          0x40 /* End of object indication */
#define EPON_VAR_OAMPDU_IND_PKGEND          0x60 /* End of package indication */

/* Info OAMPDU related */
#define EPON_INFO_OAMPDU_INFO_LEN           0x10
#define EPON_INFO_OAMPDU_VENDER_SPEC_LEN    0x04
#define EPON_INFO_OAMPDU_ORGSPEC_MIN_LEN    0x05 /* Type(1) + Length(1) + OUI(3) */
/* Event OAMPDU related */
#define EPON_EVENT_OAMPDU_ESPE_LEN          0x28 /* Size defined in 802.3 57.5.3.1 */
#define EPON_EVENT_OAMPDU_EFE_LEN           0x1A /* Size defined in 802.3 57.5.3.2 */
#define EPON_EVENT_OAMPDU_EFPE_LEN          0x1C /* Size defined in 802.3 57.5.3.3 */
#define EPON_EVENT_OAMPDU_EFSSE_LEN         0x12 /* Size defined in 802.3 57.5.3.4 */
#define EPON_EVENT_OAMPDU_SEQUENCE_LEN      0x02 /* Sequence(2) */
#define EPON_EVENT_OAMPDU_ORGSPEC_MIN_LEN   0x05 /* Type(1) + Length(1) + OUI(3) */
#define EPON_EVENT_OAMPDU_ORGSPEC_MAX_LEN   0xFF /* Maximum size of one octet */
/* Variable OAMPDU related */
#define EPON_VAR_OAMPDU_DESC_LEN            0x03 /* Branch(1) + Leaf(2) */
#define EPON_VAR_OAMPDU_TARGET_LEN          0x03 /* Width(1) + Data(2) */
#define EPON_VAR_OAMPDU_PKGEND_LEN          0x01 /* Package end indication length */
#define EPON_VAR_OAMPDU_OBJEND_LEN          0x01 /* Object end indication length */
#define EPON_VAR_OAMPDU_OBJID_LEN           0x02 /* Size defined in 802.3 57.6.2.1 */
/* Others */
#define EPON_OAMPDU_HDR_LENGTH              0x12 /* DstMac(6) + SrcMac(6) + EtherType(2) + SubType(1) + Flag(2) + Code(1)*/
#define EPON_OAM_OUI_LENGTH                 0x03
#define EPON_OAM_DYINGGASP_BUF_LEN          0x40 /* at least > (EPON_OAMPDU_HDR_LENGTH + (EPON_INFO_OAMPDU_INFO_LEN * 2) + 1) */
#define EPON_TX_OAM_LLID_LEN                (sizeof(unsigned char)) /* This length is for OAM tx module to notify the Tx driver */

/*
 * Macro Definition
 */
#define EPON_VAR_OAMPDU_WIDTH(width)        ((width & EPON_VAR_OAMPDU_IND) ? 0 : ((0 == width) ? 128 : width))
#define EPON_VAR_OAMPDU_INDICATOR(ind)      (ind | EPON_VAR_OAMPDU_IND)

/*  
 * Function Declaration  
 */ 
extern void *epon_oam_rxThread(void *argu);

#endif /* __EPON_OAM_RX_H__ */

