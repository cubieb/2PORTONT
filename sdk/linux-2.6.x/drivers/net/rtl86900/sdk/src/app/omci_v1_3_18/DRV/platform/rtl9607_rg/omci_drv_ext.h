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
 * $Date: 2013-06-24 04:35:27 -0500 (Fri, 03 May 2013) $
 *
 * Purpose : OMCI driver layer module defination
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) OMCI (G.984.4)
 *
 */

#ifndef __OMCI_DRV_EXT_H__
#define __OMCI_DRV_EXT_H__


#define DIAG_STR_NOP                    "No operation"
#define DIAG_STR_ADD                    "Add"
#define DIAG_STR_DEL                    "Delete"
#define DIAG_STR_ASSIGN                 "Assign"
#define DIAG_STR_VS_TPID                "VS_TPID"
#define DIAG_STR_TPID_8100              "TPID_8100"
#define DIAG_STR_UNTAG                  "Untagging"
#define DIAG_STR_TAG                    "Tagging"
#define DIAG_STR_C2S                    "C2S translate"
#define DIAG_STR_SP2C                   "SP2C translate"
#define DIAG_STR_TRANSPARENT            "Transparent"
#define DIAG_STR_COPY_C                 "Copy from outer tag"
#define DIAG_STR_COPY_S                 "Copy from outer tag"
#define DIAG_STR_COPY_1st               "Copy from 1st tag"
#define DIAG_STR_COPY_2nd               "Copy from 2nd tag"
#define DIAG_STR_INTER_PRI              "Assign from internal priority"
#define DIAG_STR_INTER_VID              "Assign from internal VID"
#define DIAG_STR_DMAC_VID              "Assign from destination MAC"

#define DIAG_STR_ASSIGN_QID             "ASSIGN to QID"
#define DIAG_STR_ASSIGN_SID             "ASSIGN to SID"
#define DIAG_STR_SWITCH_CORE            "Follow switch core"
#define DIAG_STR_LUT_LRN                "LUT learning"
#define DIAG_STR_FS_FORWARD             "Force forward"
#define DIAG_STR_ON_OPERATION    "No operation"


#define DIAG_STR_DROP                   "Drop"
#define DIAG_STR_TRAP2CPU               "Trap to CPU"
#define DIAG_STR_FORWARD                "Forward"
#define DIAG_STR_ASSIGN_SVLAN           "Assign to SVLAN"
#define DIAG_STR_ASSIGN_SVLAN_AND_KEEP  "Assign to SVLAN and Keep format"
#define DIAG_STR_USE_CVID               "Using CVID"
#define DIAG_STR_PB_SVID                "Port based SVID"
#define DIAG_STR_DISCARD                "Discard"

#define DIAG_STR_IPV4_SIP               "Ipv4 Sip"
#define DIAG_STR_IPV6_SIP               "Ipv6 Sip"
#define DIAG_STR_IPV6_DIP               "Ipv6 Dip"
#define DIAG_STR_IPV4_DIP               "IPv4 Dip"

#define DIAG_STR_CVID                   "CVID"
#define DIAG_STR_SVID                   "SVID"


#define DIAG_STR_SPORT                  "Source Port"
#define DIAG_STR_DPORT                  "Destination Port"


const char *diagStr_usCStagAction[] = {
    DIAG_STR_NOP,
    DIAG_STR_VS_TPID,
    DIAG_STR_TPID_8100,
    DIAG_STR_SP2C,
    DIAG_STR_DEL,
    DIAG_STR_TRANSPARENT,
};

const char *diagStr_usSvidAction[] = {
    DIAG_STR_ASSIGN,
    DIAG_STR_COPY_1st,
    DIAG_STR_COPY_2nd
};

const char *diagStr_usSpriAction[] = {
    DIAG_STR_ASSIGN,
    DIAG_STR_COPY_1st,
    DIAG_STR_COPY_2nd,
    DIAG_STR_INTER_PRI
};

const char *diagStr_usCtagAction[] = {
    DIAG_STR_NOP,
    DIAG_STR_TAG,
    DIAG_STR_C2S,
    DIAG_STR_SP2C,
    DIAG_STR_DEL,
    DIAG_STR_TRANSPARENT
};

const char *diagStr_usCvidAction[] = {
    DIAG_STR_ASSIGN,
    DIAG_STR_COPY_1st,
    DIAG_STR_COPY_2nd,
    DIAG_STR_INTER_VID
};

const char *diagStr_usCpriAction[] = {
    DIAG_STR_ASSIGN,
    DIAG_STR_COPY_1st,
    DIAG_STR_COPY_2nd,
    DIAG_STR_INTER_PRI
};

const char *diagStr_usSidAction[] = {
    DIAG_STR_NOP,
    DIAG_STR_ASSIGN_SID,
    DIAG_STR_ASSIGN_QID
};

const char *diagStr_dsCStagAction[] = {
    DIAG_STR_NOP,
    DIAG_STR_VS_TPID,
    DIAG_STR_TPID_8100,
    DIAG_STR_SP2C,
    DIAG_STR_DEL,
    DIAG_STR_TRANSPARENT
};

const char *diagStr_dsSvidAction[] = {
    DIAG_STR_ASSIGN,
    DIAG_STR_COPY_1st,
    DIAG_STR_COPY_2nd
};

const char *diagStr_dsSpriAction[] = {
    DIAG_STR_ASSIGN,
    DIAG_STR_COPY_1st,
    DIAG_STR_COPY_2nd,
    DIAG_STR_INTER_PRI
};

const char *diagStr_dsCtagAction[] = {
    DIAG_STR_NOP,
    DIAG_STR_TAG,
    DIAG_STR_C2S,
    DIAG_STR_SP2C,
    DIAG_STR_DEL,
    DIAG_STR_TRANSPARENT
};

const char *diagStr_dsCvidAction[] = {
    DIAG_STR_ASSIGN,
    DIAG_STR_COPY_1st,
    DIAG_STR_COPY_2nd,
    DIAG_STR_INTER_VID,
    DIAG_STR_DMAC_VID
};

const char *diagStr_dsCpriAction[] = {
    DIAG_STR_ASSIGN,
    DIAG_STR_COPY_1st,
    DIAG_STR_COPY_2nd,
    DIAG_STR_INTER_PRI
};

const char *diagStr_cfpriAction[] = {
    DIAG_STR_SWITCH_CORE,
    DIAG_STR_ASSIGN
};

const char *diagStr_dsUniAction[] = {
    DIAG_STR_FORWARD,
    DIAG_STR_FS_FORWARD,
};

const char *diagStr_cfRangeCheckIpTypeStr[] = {
    DIAG_STR_IPV4_SIP,
    DIAG_STR_IPV4_DIP
};

const char *diagStr_cfRangeCheckPortTypeStr[] = {
    DIAG_STR_SPORT,
    DIAG_STR_DPORT,
};


#endif
