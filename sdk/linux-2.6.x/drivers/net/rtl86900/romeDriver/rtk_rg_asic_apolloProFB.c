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
 */


/*
 * Include Files
 */
#include <rtk_rg_asic_apolloProFB.h>
#include <ioal/mem32.h>

/*
 * Symbol Definition
 */



/*
 * Data Declaration
 */
static uint32 fb_init = {INIT_NOT_COMPLETED};


/*
 * Macro Definition
 */
int32 rtk_rg_asic_p1_p2_add(rtk_rg_asic_p1_p2_entry_t *pP1P2Data)
{
	return SUCCESS;
}

int32 rtk_rg_asic_p1_p2_del(rtk_rg_asic_p1_p2_entry_t *pP1P2Data)
{
	return SUCCESS;
}

int32 rtk_rg_asic_p1_p2_get(rtk_rg_asic_p1_p2_entry_t *pP1P2Data)
{
	return SUCCESS;
}



int32 rtk_rg_asic_p3_p4_add(rtk_rg_asic_p3_p4_entry_t *pP3P4Data)
{
	return SUCCESS;
}

int32 rtk_rg_asic_p3_p4_del(rtk_rg_asic_p3_p4_entry_t *pP3P4Data)
{
	return SUCCESS;
}

int32 rtk_rg_asic_p3_p4_get(rtk_rg_asic_p3_p4_entry_t *pP3P4Data)
{
	return SUCCESS;
}



int32 rtk_rg_asic_p5_add(rtk_rg_asic_p5_entry_t *pP5Data)
{
	return SUCCESS;
}

int32 rtk_rg_asic_p5_del(rtk_rg_asic_p5_entry_t *pP5Data)
{
	return SUCCESS;
}

int32 rtk_rg_asic_p5_get(rtk_rg_asic_p5_entry_t *pP5Data)
{
	return SUCCESS;
}



int32 rtk_rg_asic_p6_add(rtk_rg_asic_p6_entry_t *pP6Data)
{
	return SUCCESS;
}

int32 rtk_rg_asic_p6_del(rtk_rg_asic_p6_entry_t *pP6Data)
{
	return SUCCESS;
}

int32 rtk_rg_asic_p6_get(rtk_rg_asic_p6_entry_t *pP6Data)
{
	return SUCCESS;
}



int32 rtk_rg_asic_netifTable_add(uint32 idx, rtk_rg_asic_netif_entry_t *pNetifEntry)
{
	return SUCCESS;
}

int32 rtk_rg_asic_netifTable_del(uint32 idx, rtk_rg_asic_netif_entry_t *pNetifEntry)
{
	return SUCCESS;
}

int32 rtk_rg_asic_netifTable_get(uint32 idx, rtk_rg_asic_netif_entry_t *pNetifEntry)
{
	return SUCCESS;
}


int32 rtk_rg_asic_netifMib_get(rtk_rg_asic_netifMib_entry_t *pNetifMibEntry)
{
	return SUCCESS;
}

int32 rtk_rg_asic_netifMib_reset(uint32 netifIdx)
{
	return SUCCESS;
}



int32 rtk_rg_asic_indirectMacTable_add(uint32 idx, rtk_rg_asic_indirectMac_entry_t *pIndirectMacEntry)
{
	return SUCCESS;
}

int32 rtk_rg_asic_indirectMacTable_del(uint32 idx, rtk_rg_asic_indirectMac_entry_t *pIndirectMacEntry)
{
	return SUCCESS;
}

int32 rtk_rg_asic_indirectMacTable_get(uint32 idx, rtk_rg_asic_indirectMac_entry_t *pIndirectMacEntry)
{
	return SUCCESS;
}



int32 rtk_rg_asic_etherTypeTable_add(uint32 idx, rtk_rg_asic_etherType_entry_t *pEtherTypeEntry)
{
	return SUCCESS;
}

int32 rtk_rg_asic_etherTypeTable_del(uint32 idx, rtk_rg_asic_etherType_entry_t *pEtherTypeEntry)
{
	return SUCCESS;
}

int32 rtk_rg_asic_etherTypeTable_get(uint32 idx, rtk_rg_asic_etherType_entry_t *pEtherTypeEntry)
{
	return SUCCESS;
}



int32 rtk_rg_asic_extraTagAction_add(uint32 entryIdx, uint32 actionIdx, rtk_rg_asic_extraTagAction_t *pExtraTagAction)
{
	return SUCCESS;
}

int32 rtk_rg_asic_extraTagAction_del(uint32 entryIdx, uint32 actionIdx, rtk_rg_asic_extraTagAction_t *pExtraTagAction)
{
	return SUCCESS;
}

int32 rtk_rg_asic_extraTagAction_get(uint32 entryIdx, uint32 actionIdx, rtk_rg_asic_extraTagAction_t *pExtraTagAction)
{
	return SUCCESS;
}


int32 rtk_rg_asic_flowTrfIndicator_get(uint32 idx, rtk_enable_t *pFlowIndicator)
{
	return SUCCESS;
}

int32 rtk_rg_asic_flowTraffic_get(uint32 setNum, uint32 *flowTrafficSet)
{
	return SUCCESS;
}

int32 rtk_rg_asic_netifTrfIndicator_get(uint32 idx, rtk_enable_t *pNetifIndicator)
{
	return SUCCESS;
}

int32 rtk_rg_asic_netifTraffic_get(uint32 setNum, uint32 *netifTrafficSet)
{
	return SUCCESS;
}



int32 rtk_rg_asic_trapCpuPriority_set(rtk_enable_t state, uint32 priority)
{
	return SUCCESS;
}

int32 rtk_rg_asic_trapCpuPriority_get(rtk_enable_t *pState, uint32 *pPriority)
{
	return SUCCESS;
}

int32 rtk_rg_asic_spaUnmatchAction_set(rtk_rg_asic_forwardAction_t action)
{
	return SUCCESS;
}

int32 rtk_rg_asic_spaUnmatchAction_get(rtk_rg_asic_forwardAction_t *pAction)
{
	return SUCCESS;
}

int32 rtk_rg_asic_globalState_set(rtk_rg_asic_globalStateType_t stateType, rtk_enable_t state)
{
	return SUCCESS;
}

int32 rtk_rg_asic_globalState_get(rtk_rg_asic_globalStateType_t stateType, rtk_enable_t *pState)
{
	return SUCCESS;
}

int32 rtk_rg_asic_table_reset(rtk_rg_asic_resetTableType_t type)
{
	return SUCCESS;
}

int32 rtk_rg_asic_fb_init(void)
{
	fb_init = INIT_COMPLETED;
	fb_init = INIT_NOT_COMPLETED;
	
	return SUCCESS;
}


