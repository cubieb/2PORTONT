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
 * Purpose : Definition of OMCI MIB tree related define
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) OMCI MIB tree related define
 */

#ifndef __MIB_TREE_H__
#define __MIB_TREE_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "gos_linux.h"
#include "mib_table_defs.h"
#include "omci_vlan_rule.h"


typedef enum {

	AVL_KEY_PPTPUNI,
	AVL_KEY_VEIP,
	AVL_KEY_IPHOST,
	AVL_KEY_EXTVLAN_UNI,
	AVL_KEY_VLANTAGFILTER_UNI,
	AVL_KEY_VLANTAGOPCFG_UNI,
	AVL_KEY_MACBRIPORT_UNI,
	AVL_KEY_MACBRISERVPROF,
	AVL_KEY_MACBRIPORT_ANI,
	AVL_KEY_EXTVLAN_ANI,
	AVL_KEY_VLANTAGOPCFG_ANI,
	AVL_KEY_VLANTAGFILTER_ANI,
	AVL_KEY_MAP8021PSERVPROF,
	AVL_KEY_GEMIWTP,
	AVL_KEY_MULTIGEMIWTP,
	AVL_KEY_GEMPORTCTP,
	AVL_KEY_PRIQ,
	AVL_KEY_TCONT

}MIB_AVL_KEY_T;

typedef struct mib_tree_node_entry_s
{
	struct mib_entry_s *mibEntry;
    LIST_ENTRY(mib_tree_node_entry_s) treeNodeEntry;
}MIB_TREE_NODE_ENTRY_T;

/*for maintain ME relationship*/
typedef struct mib_tree_data_s
{
	MIB_TABLE_INDEX	  tableIndex;
	MIB_AVL_KEY_T key;
	LIST_HEAD(treeNodeEntryHead,mib_tree_node_entry_s) treeNodeEntryHead;
}MIB_TREE_DATA_T;

typedef struct mib_node_s
{
	MIB_TREE_DATA_T data;
	struct mib_node_s *rChild;
	struct mib_node_s *lChild;
}MIB_NODE_T;


typedef enum {
	OMCI_TRAF_MODE_FLOW_BASE,
	OMCI_TRAF_MODE_8021P_BASE,
} OMCI_TRAF_MODE_T;


typedef enum {
	OMCI_CONN_STATE_COMMON,
	OMCI_CONN_STATE_NEW,
	OMCI_CONN_STATE_UPDATE,
	OMCI_CONN_STATE_DEL,

}OMCI_CONN_STATE_T;

typedef struct mib_tree_conn_s
{
	OMCI_TRAF_MODE_T						traffMode; /*flow base or 1p base*/
	MIB_TABLE_MACBRIPORTCFGDATA_T			*pAniPort;
	MIB_TABLE_MACBRIPORTCFGDATA_T			*pUniPort;
	MIB_TABLE_ETHUNI_T						*pEthUni;
	MIB_TABLE_VEIP_T						*pVeip;
	MIB_TABLE_IP_HOST_CFG_DATA_T 			*pIpHost;
	MIB_TABLE_MAP8021PSERVPROF_T			*p8021Map;
	MIB_TABLE_VLANTAGFILTERDATA_T			*pAniVlanTagFilter;
	MIB_TABLE_VLANTAGFILTERDATA_T			*pUniVlanTagFilter;	
	MIB_TABLE_VLANTAGOPCFGDATA_T			*pAniVlanTagOpCfg;
	MIB_TABLE_VLANTAGOPCFGDATA_T			*pUniVlanTagOpCfg;
	MIB_TABLE_EXTVLANTAGOPERCFGDATA_T		*pAniExtVlanCfg;
	MIB_TABLE_EXTVLANTAGOPERCFGDATA_T		*pUniExtVlanCfg;
	MIB_TABLE_MULTIGEMIWTP_T				*pMcastGemIwTp;
	MIB_TABLE_GEMIWTP_T						*pGemIwTp[8];
	MIB_TABLE_GEMPORTCTP_T					*pGemPortCtp[8];
	OMCI_CONN_STATE_T						state;
	LIST_HEAD(ruleHead,omci_vlan_rule_s)	ruleHead[8];
	LIST_ENTRY(mib_tree_conn_s) 			entries;

}MIB_TREE_CONN_T;

typedef struct mib_tree_s
{

	MIB_NODE_T *root;
	LIST_HEAD(connHead,mib_tree_conn_s) conns;
	LIST_ENTRY(mib_tree_s) entries;

}MIB_TREE_T;

typedef struct mib_forest_s{
	int treeCount;
    	LIST_HEAD(avlhead,mib_tree_s) treeHead;
}MIB_FOREST_T;


#define avlTreeGetMax(a,b) (((a) > (b)) ? (a) : (b))


#ifdef __cplusplus
}
#endif

#endif
