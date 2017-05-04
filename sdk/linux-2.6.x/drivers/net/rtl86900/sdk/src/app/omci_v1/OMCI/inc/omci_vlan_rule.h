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
 * Purpose : Definition of OMCI VLAN rule related info
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) OMCI VLAN rule definition
 */

#ifndef __OMCI_VLAN_RULE_H__
#define __OMCI_VLAN_RULE_H__

#ifdef  __cplusplus
extern "C" {
#endif


typedef enum
{
    PON_GEMPORT_DIRECTION_US = 0x1,
    PON_GEMPORT_DIRECTION_DS = 0x2,
    PON_GEMPORT_DIRECTION_BI = 0x3,
} PON_GEMPORT_DIRECTION;

/*classify difference VLAN flilter mode*/
typedef enum {
	OMCI_VLANFILTER_MODE_FORWARDALL,
	OMCI_VLANFILTER_MODE_DROPTAG_FOWARDUNTAG,
	OMCI_VLANFILTER_MODE_FORWARDTAG_DROPUNTAG,
	OMCI_VLANFILTER_MODE_FORWARDLIST_VID_FORWARDUNTAG,
	OMCI_VLANFILTER_MODE_FORWARDLIST_PRI_FORWARDUNTAG,
	OMCI_VLANFILTER_MODE_FORWARDLIST_TCI_FORWARDUNTAG,
	OMCI_VLANFILTER_MODE_FORWARDLIST_VID_DROPUNTAG,
	OMCI_VLANFILTER_MODE_FORWARDLIST_PRI_DROPUNTAG,
	OMCI_VLANFILTER_MODE_FORWARDLIST_TCI_DROPUNTAG,
	OMCI_VLANFILTER_MODE_NOTSUPPORT
}OMCI_VLANFILTER_MODE_T;

typedef enum
{
	VLAN_OPER_MODE_FORWARD_ALL,
	VLAN_OPER_MODE_FORWARD_UNTAG,
	VLAN_OPER_MODE_FORWARD_SINGLETAG,
	VLAN_OPER_MODE_FILTER_INNER_PRI,
	VLAN_OPER_MODE_FILTER_SINGLETAG,
	VLAN_OPER_MODE_EXTVLAN,
	VLAN_OPER_MODE_VLANTAG_OPER,
} OMCI_VLAN_OPER_MODE_t;

typedef struct
{
	unsigned int pri;
	unsigned int vid;
	unsigned int tpid;
} OMCI_VLAN_ts;

enum {
	OMCI_VID_FILTER_IGNORE				= 4096,
    OMCI_PRI_FILTER_IGNORE				= 8,
    OMCI_EXTVLAN_REMOVE_TAG_DISCARD		= 3
};

typedef enum
{
    VLAN_FILTER_NO_CARE_TAG =(1 << 0),
	VLAN_FILTER_CARE_TAG	=(1 << 1),
	VLAN_FILTER_NO_TAG		=(1 << 2),
	VLAN_FILTER_VID			=(1 << 3),
	VLAN_FILTER_PRI			=(1 << 4),
	VLAN_FILTER_TCI			=(1 << 5),
	VLAN_FILTER_ETHTYPE		=(1 << 6),
	VLAN_FILTER_DSCP_PRI	=(1 << 7)
} OMCI_VLAN_FILTER_MODE_e;

typedef enum
{
	ETHTYPE_FILTER_NO_CARE	=0,
	ETHTYPE_FILTER_IP		=1,
	ETHTYPE_FILTER_PPPOE	=2,
	ETHTYPE_FILTER_ARP		=3,
	ETHTYPE_FILTER_IPV6		=4,
	ETHTYPE_FILTER_PPPOE_S	=5
} OMCI_ETHTYPE_FILTER_MODE_e;

typedef struct
{
	OMCI_VLAN_ts filterSTag;
	OMCI_VLAN_ts filterCTag;
	OMCI_VLAN_FILTER_MODE_e filterStagMode;
	OMCI_VLAN_FILTER_MODE_e filterCtagMode;
	OMCI_ETHTYPE_FILTER_MODE_e etherType;
} OMCI_VLAN_FILTER_ts;

typedef enum
{
	VLAN_ACT_NON,
	VLAN_ACT_ADD,
	VLAN_ACT_REMOVE,
	VLAN_ACT_MODIFY,
	VLAN_ACT_TRANSPARENT
} OMCI_VLAN_ACT_MODE_e;

typedef enum
{
	VID_ACT_ASSIGN,
	VID_ACT_COPY_INNER,
	VID_ACT_COPY_OUTER,
	VID_ACT_TRANSPARENT,
} OMCI_VID_ACT_MODE_e;

typedef enum
{
	PRI_ACT_ASSIGN,
	PRI_ACT_COPY_INNER,
	PRI_ACT_COPY_OUTER,
	PRI_ACT_FROM_DSCP,
	PRI_ACT_TRANSPARENT,
} OMCI_PRI_ACT_MODE_e;

typedef struct
{
	OMCI_VLAN_ACT_MODE_e vlanAct;
	OMCI_VID_ACT_MODE_e  vidAct;
	OMCI_PRI_ACT_MODE_e  priAct;
	OMCI_VLAN_ts assignVlan;
} OMCI_VLAN_ACT_ts;

typedef struct
{
	unsigned int isDefaultRule;
	unsigned int outTagNum;
	unsigned int tpid;
	unsigned int dsMode;
	OMCI_VLAN_ACT_MODE_e dsTagOperMode;
	OMCI_VLAN_ts outVlan;
} OMCI_VLAN_OUT_ts;

typedef struct
{
	OMCI_VLAN_OPER_MODE_t 	filterMode;
	OMCI_VLAN_FILTER_ts 	filterRule;
	OMCI_VLAN_ACT_ts 		sTagAct;
	OMCI_VLAN_ACT_ts 		cTagAct;
	OMCI_VLAN_OUT_ts		outStyle;
} OMCI_VLAN_OPER_ts;

#ifndef __KERNEL__
typedef struct omci_vlan_rule_s
{
	int			servId;
	int 		isLatchB;
	int			ingress;
	int			outgress;
	PON_GEMPORT_DIRECTION	dir;
	OMCI_VLAN_OPER_ts	vlanRule;
	LIST_ENTRY(omci_vlan_rule_s) entries;
} omci_vlan_rule_t;
#endif

typedef enum
{
    OMCI_UNI_RATE_DIRECTION_INGRESS = 0x1,
    OMCI_UNI_RATE_DIRECTION_EGRESS  = 0x2,
} OMCI_UNI_RATE_DIRECTION;

enum {
	OMCI_VLAN_RULE_LOW_PRI,
    OMCI_VLAN_RULE_HIGH_PRI
};


#ifdef  __cplusplus
}
#endif

#endif
