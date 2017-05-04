#ifndef __RTK_RG_APOLLOFE_LITEROMEDRIVER_H__
#define __RTK_RG_APOLLOFE_LITEROMEDRIVER_H__

#include <rtk_rg_liteRomeDriver.h>
#include <rtk_rg_fwdEngine.h>
#include <rtk_rg_apollo_liteRomeDriver.h>
#include <rtk_rg_mappingAPI.h>

rtk_rg_err_code_t rtk_rg_apolloFE_dsliteMcTable_set(rtk_l34_dsliteMc_entry_t *pDsliteMcEntry);
rtk_rg_err_code_t rtk_rg_apolloFE_dsliteMcTable_get(rtk_l34_dsliteMc_entry_t *pDsliteMcEntry);
rtk_rg_err_code_t rtk_rg_apolloFE_dsliteControl_set(rtk_l34_dsliteCtrlType_t ctrlType, uint32 act);
rtk_rg_err_code_t rtk_rg_apolloFE_dsliteControl_get(rtk_l34_dsliteCtrlType_t ctrlType, uint32 *pAct);
rtk_rg_err_code_t rtk_rg_apolloFE_interfaceMibCounter_del(int intf_idx);
rtk_rg_err_code_t rtk_rg_apolloFE_interfaceMibCounter_get(rtk_l34_mib_t *pMibCnt);

rtk_rg_err_code_t rtk_rg_apollo_svlanTpid2_enable_set(rtk_rg_enable_t enable);
rtk_rg_err_code_t rtk_rg_apollo_svlanTpid2_enable_get(rtk_rg_enable_t *pEnable);
rtk_rg_err_code_t rtk_rg_apollo_svlanTpid2_set(uint32 svlan_tag_id);
rtk_rg_err_code_t rtk_rg_apollo_svlanTpid2_get(uint32 *pSvlanTagId);

//internal function prototype
int _rtk_rg_hardwareArpTableLookUp(unsigned short routingIdx, ipaddr_t ipAddr, rtk_rg_arp_linkList_t **phardwareArpEntry, int resetIdleTime);
int _rtk_rg_hardwareArpTableAdd(unsigned short routingIdx, ipaddr_t ipv4Addr, int l2Idx, int staticEntry, uint16 *hwArpIdx);
int _rtk_rg_hardwareArpTableDel(rtk_rg_arp_linkList_t *pDelArpEntry);
int32 _rtk_rg_deleteHwARP(int intfIdx, int routingIdx);
int _rtk_rg_lutCamListAdd(int l2Idx, uint32 addLutCamIdx);
int _rtk_rg_lutCamListDel(uint32 delLutCamIdx);
rtk_rg_entryGetReturn_t _rtk_rg_findAndReclamIPv6mcEntry( rtk_ipv6_addr_t *gipv6,int32 ivlsvl);
unsigned int _hash_dip_vidfid_sipidx_sipfilter(int is_ivl,unsigned int dip,unsigned int vid_fid, unsigned int sip_idx, unsigned int sip_filter_en);
unsigned int _hash_ipm_dipv6(uint8 *dip);
int32 _rtk_rg_apollo_ipv6MultiCastFlow_add(rtk_rg_ipv6MulticastFlow_t *ipv6McFlow,int *flow_idx);
void _rtk_rg_IntfIdxDecisionForCF(rtk_rg_pktHdr_t *pPktHdr);

#endif
