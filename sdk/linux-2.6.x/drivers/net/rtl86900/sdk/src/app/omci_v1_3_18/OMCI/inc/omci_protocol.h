/*
 * Copyright (C) 2014 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * Purpose : Definition of OMCI message related info
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) OMCI message definition
 */

#ifndef __OMCI_MESSAGE_H__
#define __OMCI_MESSAGE_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "../../GOS/inc/gos_type.h"


/* G.988 OMCI message format */
#define OMCI_MSG_BASELINE_PRI_MASK      (0x8000)
#define OMCI_MSG_BASELINE_PRI_BIT       (15)
#define OMCI_MSG_BASELINE_TCID_MASK     (0x7FFF)
#define OMCI_MSG_EXTENDED_TCID_MASK     (0xFFFF)

#define OMCI_MSG_TYPE_DB_MASK           (0x80)
#define OMCI_MSG_TYPE_DB_BIT            (7)
#define OMCI_MSG_TYPE_AR_MASK           (0x40)
#define OMCI_MSG_TYPE_AR_BIT            (6)
#define OMCI_MSG_TYPE_AK_MASK           (0x20)
#define OMCI_MSG_TYPE_AK_BIT            (5)
#define OMCI_MSG_TYPE_MT_MASK           (0x1F)

#define OMCI_MSG_BASELINE_DEVICE_ID     (0x0A)
#define OMCI_MSG_EXTENDED_DEVICE_ID     (0x0B)

/* G.988 OMCI message len/limit */
#define OMCI_MSG_BASELINE_CONTENTS_LEN                  (32)

#define OMCI_MSG_BASELINE_GET_RSP_LIMIT                 (25)
#define OMCI_MSG_BASELINE_GET_NEXT_RSP_LIMIT            (29)
#define OMCI_MSG_BASELINE_MIB_UPLOAD_NEXT_RSP_LIMIT     (26)

#define OMCI_MSG_BASELINE_PRI_NUM       (2)

typedef enum
{
    OMCI_MSG_BASELINE_PRI_LOW,
    OMCI_MSG_BASELINE_PRI_HIGH
} omci_msg_baseline_pri_t;

typedef UINT16 omci_me_class_t;
typedef UINT16 omci_me_instance_t;

typedef struct
{
    omci_me_class_t     meClass;
    omci_me_instance_t  meInstance;
} omci_msg_meid_t;

typedef struct
{
    UINT16 cpcsUuCpi;   /* always set to 0 */
    UINT16 cpcsLen;     /* always set to 0x0028 */
    UINT32 crc;
} omci_msg_trailer_t;

/* G.988 Table 11.2.1 - Baseline OMCI message format */
typedef struct
{
    UINT16              tcId;
    UINT8               type;
    UINT8               devId;
    omci_msg_meid_t     meId;
    UINT8               content[OMCI_MSG_BASELINE_CONTENTS_LEN];
    omci_msg_trailer_t  trailer;
} omci_msg_baseline_fmt_t;

/* G.988 Table 11.2.2 - Extended OMCI message format */
typedef struct
{
    UINT16              tcId;
    UINT8               type;
    UINT8               devId;
    omci_msg_meid_t     meId;
    UINT16              contentLen;
    UINT8               *pContent;
    omci_msg_trailer_t  trailer;
} omci_msg_extended_fmt_t;

typedef UINT16 omci_msg_attr_mask_t;

/* normalized baseline fmt */
typedef struct
{
    UINT16              priority;
    UINT16              tcId;
    UINT8               db;
    UINT8               ar;
    UINT8               ak;
    UINT8               type;
    UINT32              devId;
    omci_msg_meid_t     meId;
    UINT8               content[OMCI_MSG_BASELINE_CONTENTS_LEN];
    omci_msg_trailer_t  trailer;
} omci_msg_norm_baseline_t;

/* G.988 Table 11.2.2-1 - OMCI message types */
enum
{
    OMCI_MSG_TYPE_CREATE                = 4,
    OMCI_MSG_TYPE_DELETE                = 6,
    OMCI_MSG_TYPE_SET                   = 8,
    OMCI_MSG_TYPE_GET                   = 9,
    OMCI_MSG_TYPE_GET_ALL_ALARMS        = 11,
    OMCI_MSG_TYPE_GET_ALL_ALARMS_NEXT   = 12,
    OMCI_MSG_TYPE_MIB_UPLOAD            = 13,
    OMCI_MSG_TYPE_MIB_UPLOAD_NEXT       = 14,
    OMCI_MSG_TYPE_MIB_RESET             = 15,
    OMCI_MSG_TYPE_ALARM                 = 16,
    OMCI_MSG_TYPE_ATTR_VALUE_CHANGE     = 17,
    OMCI_MSG_TYPE_TEST                  = 18,
    OMCI_MSG_TYPE_START_SW_DOWNLOAD     = 19,
    OMCI_MSG_TYPE_DOWNLOAD_SECTION      = 20,
    OMCI_MSG_TYPE_END_SW_DOWNLOAD       = 21,
    OMCI_MSG_TYPE_ACTIVATE_SW           = 22,
    OMCI_MSG_TYPE_COMMIT_SW             = 23,
    OMCI_MSG_TYPE_SYNCHRONIZE_TIME      = 24,
    OMCI_MSG_TYPE_REBOOT                = 25,
    OMCI_MSG_TYPE_GET_NEXT              = 26,
    OMCI_MSG_TYPE_TEST_RESULT           = 27,
    OMCI_MSG_TYPE_GET_CURRENT_DATA      = 28,
    OMCI_MSG_TYPE_SET_TABLE             = 29
};

/* G.988 Table 11.2.4-1 - Managed entity identifiers */
enum
{
    /* G.988 9.1 - Equipment management */
    OMCI_ME_CLASS_ONU_G                                 = 256,  // 9.1.1
    OMCI_ME_CLASS_ONU2_G                                = 257,  // 9.1.2
    OMCI_ME_CLASS_ONU_DATA                              = 2,    // 9.1.3
    OMCI_ME_CLASS_SOFTWARE_IMAGE                        = 7,    // 9.1.4
    OMCI_ME_CLASS_CARDHOLDER                            = 5,    // 9.1.5
    OMCI_ME_CLASS_CIRCUIT_PACK                          = 6,    // 9.1.6
    OMCI_ME_CLASS_ONU_POWER_SHEDDING                    = 133,  // 9.1.7
    OMCI_ME_CLASS_ONU_REMOTE_DEBUG                      = 158,  // 9.1.12

    /* G.988 9.2 - ANI management, traffic management */
    OMCI_ME_CLASS_ANI_G                                 = 263,  // 9.2.1
    OMCI_ME_CLASS_T_CONT                                = 262,  // 9.2.2
    OMCI_ME_CLASS_GEM_PORT_NETWORK_CTP                  = 268,  // 9.2.3
    OMCI_ME_CLASS_GEM_IWTP                              = 266,  // 9.2.4
    OMCI_ME_CLASS_MCAST_GEM_IWTP                        = 281,  // 9.2.5
    OMCI_ME_CLASS_GEM_PORT_PMHD                         = 267,  // 9.2.6
    OMCI_ME_CLASS_GAL_ETHERNET_PROFILE                  = 272,  // 9.2.7
    OMCI_ME_CLASS_GAL_ETHERNET_PMHD                     = 276,  // 9.2.8
    OMCI_ME_CLASS_FEC_PMHD                              = 312,  // 9.2.9
    OMCI_ME_CLASS_PRIORITY_QUEUE                        = 277,  // 9.2.10
    OMCI_ME_CLASS_TRAFFIC_SCHEDULER                     = 278,  // 9.2.11
    OMCI_ME_CLASS_TRAFFIC_DESCRIPTOR                    = 280,  // 9.2.12
    OMCI_ME_CLASS_GEM_PORT_NETWORK_CTP_PMHD             = 341,  // 9.2.13

    /* G.988 9.3 - Layer 2 data services */
    OMCI_ME_CLASS_MAC_BRG_SRV_PROFILE                   = 45,   // 9.3.1
    OMCI_ME_CLASS_MAC_BRG_CFG_DATA                      = 46,   // 9.3.2
    OMCI_ME_CLASS_MAC_BRG_PMHD                          = 51,   // 9.3.3
    OMCI_ME_CLASS_MAC_BRG_PORT_CFG_DATA                 = 47,   // 9.3.4
    OMCI_ME_CLASS_MAC_BRG_PORT_DESIGNATION_DATA         = 48,   // 9.3.5
    OMCI_ME_CLASS_MAC_BRG_PORT_FILTER_TBL_DATA          = 49,   // 9.3.6
    OMCI_ME_CLASS_MAC_BRG_PORT_FILTER_PRE_ASSIGN_TBL    = 79,   // 9.3.7
    OMCI_ME_CLASS_MAC_BRG_PORT_BRIDGE_TBL_DATA          = 50,   // 9.3.8
    OMCI_ME_CLASS_MAC_BRG_PORT_PMHD                     = 52,   // 9.3.9
    OMCI_ME_CLASS_IEEE_802DOT1P_MAPPER_SRV_PROFILE      = 130,  // 9.3.10
    OMCI_ME_CLASS_VLAN_TAGGING_FILTER_DATA              = 84,   // 9.3.11
    OMCI_ME_CLASS_VLAN_TAGGING_OP_CFG_DATA              = 78,   // 9.3.12
    OMCI_ME_CLASS_EXTENDED_VLAN_TAGGING_OP_CFG_DATA     = 171,  // 9.3.13
    OMCI_ME_CLASS_DOT1_RATE_LIMITER                     = 298,  // 9.3.18
    OMCI_ME_CLASS_MCAST_OPERATIONS_PROFILE              = 309,  // 9.3.27
    OMCI_ME_CLASS_MCAST_SUBSCRIBER_CFG_INFO             = 310,  // 9.3.28
    OMCI_ME_CLASS_MCAST_SUBSCRIBER_MONITOR              = 311,  // 9.3.29
    OMCI_ME_CLASS_ETHERNET_FRAME_PMHD_UPSTREAM          = 322,  // 9.3.30
    OMCI_ME_CLASS_ETHERNET_FRAME_PMHD_DOWNSTREAM        = 321,  // 9.3.31
    OMCI_ME_CLASS_ETHERNET_FRAME_EXTENDED_PM            = 334,  // 9.3.32

    /* G.988 9.4 - Layer 3 data services */
    OMCI_ME_CLASS_IP_HOST_CFG_DATA                      = 134,  // 9.4.1
    OMCI_ME_CLASS_IP_HOST_PMHD                          = 135,  // 9.4.2
    OMCI_ME_CLASS_TCP_UDP_CFG_DATA                      = 136,  // 9.4.3
    OMCI_ME_CLASS_TCP_UDP_PMHD                          = 342,  // 9.4.4

    /* G.988 9.5 - Ethernet services */
    OMCI_ME_CLASS_PPTP_ETHERNET_UNI                     = 11,   // 9.5.1
    OMCI_ME_CLASS_ETHERNET_PMHD                         = 24,   // 9.5.2
    OMCI_ME_CLASS_ETHERNET_PMHD_2                       = 89,   // 9.5.3
    OMCI_ME_CLASS_ETHERNET_PMHD_3                       = 296,  // 9.5.4
    OMCI_ME_CLASS_VIRTUAL_ETHERNET_INTF_POINT           = 329,  // 9.5.5

    /* G.988 9.7 - xDSL services */
    /* G.988 9.8 - TDM services */

    /* G.988 9.9 - Voice services */
    OMCI_ME_CLASS_PPTP_POTS_UNI                         = 53,   // 9.9.1
    OMCI_ME_CLASS_SIP_USER_DATA                         = 153,  // 9.9.2
    OMCI_ME_CLASS_SIP_AGENT_CFG_DATA                    = 150,  // 9.9.3
    OMCI_ME_CLASS_VOIP_VOICE_CTP                        = 139,  // 9.9.4
    OMCI_ME_CLASS_VOIP_MEDIA_PROFILE                    = 142,  // 9.9.5
    OMCI_ME_CLASS_VOICE_SRV_PROFILE                     = 58,   // 9.9.6
    OMCI_ME_CLASS_RTP_PROFILE_DATA                      = 143,  // 9.9.7
    OMCI_ME_CLASS_VOIP_APPLICATION_SRV_PROFILE          = 146,  // 9.9.8
    OMCI_ME_CLASS_VOIP_FEATURE_ACCESS_CODES             = 147,  // 9.9.9
    OMCI_ME_CLASS_NETWORK_DIAL_PLAN_TBL                 = 145,  // 9.9.10
    OMCI_ME_CLASS_VOIP_LINE_STATUS                      = 141,  // 9.9.11
    OMCI_ME_CLASS_CALL_CONTROL_PMHD                     = 140,  // 9.9.12
    OMCI_ME_CLASS_RTP_PMHD                              = 144,  // 9.9.13
    OMCI_ME_CLASS_SIP_AGENT_PMHD                        = 151,  // 9.9.14
    OMCI_ME_CLASS_SIP_CALL_INITIATION_PMHD              = 152,  // 9.9.15
    OMCI_ME_CLASS_MGC_CFG_DATA                          = 155,  // 9.9.16
    OMCI_ME_CLASS_MGC_PMHD                              = 156,  // 9.9.17
    OMCI_ME_CLASS_VOIP_CFG_DATA                         = 138,  // 9.9.18
    OMCI_ME_CLASS_SIP_CFG_PORTAL                        = 149,  // 9.9.19
    OMCI_ME_CLASS_MGC_CFG_PORTAL                        = 154,  // 9.9.20

    /* G.988 9.10 - Premises networks */

    /* G.988 9.12 - General purpose MEs */
    OMCI_ME_CLASS_UNI_G                                 = 264,  // 9.12.1
    OMCI_ME_CLASS_OLT_G                                 = 131,  // 9.12.2
    OMCI_ME_CLASS_NETWORK_ADDRESS                       = 137,  // 9.12.3
    OMCI_ME_CLASS_AUTH_SECURITY_METHOD                  = 148,  // 9.12.4
    OMCI_ME_CLASS_LARGE_STRING                          = 157,  // 9.12.5
    OMCI_ME_CLASS_THRESHOLD_DATA_1                      = 273,  // 9.12.6
    OMCI_ME_CLASS_THRESHOLD_DATA_2                      = 274,  // 9.12.7
    OMCI_ME_CLASS_OMCI                                  = 287,  // 9.12.8
    OMCI_ME_CLASS_MANAGED_ENTITY                        = 288,  // 9.12.9
    OMCI_ME_CLASS_ATTRIBUTE                             = 289,  // 9.12.10
    OMCI_ME_CLASS_OCTET_STRING                          = 307,  // 9.12.11
    OMCI_ME_CLASS_GENERAL_PURPOSE_BUFFER                = 308,  // 9.12.12
    OMCI_ME_CLASS_GENERIC_STATUS_PORTAL                 = 330,  // 9.12.14
    OMCI_ME_CLASS_BBF_TR069_MGMT_SERVER                 = 340,  // 9.12.16

    /* G.988 9.13 - Miscellaneous services */
    OMCI_ME_CLASS_PPTP_VIDEO_UNI                        = 82,   // 9.13.1
    OMCI_ME_CLASS_PPTP_VIDEO_ANI                        = 90,   // 9.13.2
    OMCI_ME_CLASS_PPTP_LCT_UNI                          = 83,   // 9.13.3

    /* G.988 9.14 - Mid-span PON reach extender */

    /* RTK Private ME for IOT */
    OMCI_ME_CLASS_PRIVATE_TQCFG                         = 65534, // Private
    OMCI_ME_CLASS_PRIVATE_VLANCFG                       = 65535 // Private
};

typedef UINT8 omci_msg_response_t;

/* G.988 Table A.1.1-1 - Result and reason codes */
enum
{
    OMCI_RSP_PROCESSED_SUCCESSFULLY     = 0x0,
    OMCI_RSP_PROCESSING_ERROR           = 0x1,
    OMCI_RSP_CMD_NOT_SUPPORTED          = 0x2,
    OMCI_RSP_PARAMETER_ERROR            = 0x3,
    OMCI_RSP_UNKNOWN_ME_CLASS           = 0x4,
    OMCI_RSP_UNKNOWN_ME_INSTANCE        = 0x5,
    OMCI_RSP_DEVICE_BUSY                = 0x6,
    OMCI_RSP_INSTANCE_EXISTS            = 0x7,
    OMCI_RSP_ATTR_FAILED_OR_UNKNOWN     = 0x9
};


#ifdef __cplusplus
}
#endif

#endif
