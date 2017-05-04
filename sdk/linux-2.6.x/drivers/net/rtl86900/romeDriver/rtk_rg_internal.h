#ifndef RTK_RG_INTERNAL_H
#define RTK_RG_INTERNAL_H

#include <rtk_rg_define.h>
#include <rtk_rg_struct.h>
#include <rtk_rg_debug.h>

#ifdef __KERNEL__
//for get link-down indicator register
#if defined(CONFIG_APOLLO)
#include <rtk/intr.h>		
#include <module/intr_bcaster/intr_bcaster.h>
#elif defined(CONFIG_XDSL_NEW_HWNAT_DRIVER) &&  defined(CONFIG_XDSL_ROMEDRIVER)
#endif
#endif

#if 0 //ysleu: I think it's not good to mix model & rome driver codes toghter
#include <hsModel.h>
#endif

// OS-dependent defination
#ifdef CONFIG_APOLLO_MODEL
#define rtlglue_printf printf
#ifndef bzero
#define bzero(p,s) memset(p,0,s)
#endif
#define rg_lock(x) do{}while(0)
#define rg_unlock(y) do{}while(0) 
#else
#ifdef __KERNEL__
#define rtlglue_printf printk
#ifndef bzero
#define bzero(p,s) memset(p,0,s)
#endif
//#define rg_lock(x) down(x)//down_interruptible(x)
#if defined(CONFIG_DEFAULTS_KERNEL_3_18)

//1 FIXME: call local_bh_disable() cause kernel panic 
#define rg_lock(x)			\
	do {						\
		preempt_disable(); \
		spin_lock_irqsave(x, rg_kernel.rg_lock_flags);	\
	} while (0)

#define rg_unlock(y)			\
		do {						\
			spin_unlock_irqrestore(y, rg_kernel.rg_lock_flags);	\
			preempt_enable(); \
		} while (0)

#else
#define rg_lock(x)			\
	do {						\
		preempt_disable(); \
		local_bh_disable(); \
		spin_lock_irqsave(x, rg_kernel.rg_lock_flags);	\
	} while (0)

#define rg_unlock(y)			\
		do {						\
			spin_unlock_irqrestore(y, rg_kernel.rg_lock_flags);	\
			preempt_enable(); \
			local_bh_enable(); \
		} while (0)
#endif


//#define rg_unlock(y) up(y)
#else
#ifndef u8
#define u8 uint8
#endif
#ifndef u16
#define u16 uint16
#endif
#ifndef u32
#define u32 uint32
#endif
#ifndef u64
#define u64 uint64
#endif
#endif
#endif

//Extern external functions
extern void *rtk_rg_malloc(int NBYTES);
extern void rtk_rg_free(void *APTR);
struct sk_buff *rtk_rg_skbCopyToPreAllocSkb(struct sk_buff *skb);
	
void memDump (void *start, unsigned int size, char * strHeader);

//Extern RG Global Variables
extern rtk_rg_globalDatabase_t rg_db;
extern rtk_rg_globalDatabase_cache_t rg_db_cache;
extern rtk_rg_globalKernel_t rg_kernel;
extern struct platform pf;

extern rtk_rg_intfInfo_t RG_GLB_INTF_INFO[8];

extern const char* rg_http_request_cmd[];

#ifndef DISABLE
#define DISABLE 0
#endif

#ifndef ENABLE
#define ENABLE 1
#endif

#define UNKNOW_INTF 0
#define WAN_INTF 1
#define LAN_INTF 2

//NAPT/NAPT-R Traffic
#define REG_L4_TRF0	0x00800300
#define REG_L4_TRF1	0x00800400
#define REG_NAT_CTRL	0x00800010
#define CLR_L4_TRF_DONE	0x0
#define CLR_L4_TRF0	0x1
#define CLR_L4_TRF1	0x2
#define CLR_L4_TRF_BOTH	0x3
#define FIELD_L4_TRF_CLR_OFFSET 19
#define FIELD_L4_TRF_CHG_OFFSET 21
#define FIELD_L4_TRF_SEL_OFFSET 22

//ARP Traffic
#define REG_ARP_TRF0	0x00800500
#define REG_ARP_TRF1	0x00800600
#if defined(CONFIG_RTL9602C_SERIES)
#define REG_ARP_TRF_BASE 0x00800050
#endif
#define CLR_ARP_TRF_DONE	0x0
#define CLR_ARP_TRF0	0x1
#define CLR_ARP_TRF1	0x2
#define CLR_ARP_TRF_BOTH	0x3
#define FIELD_ARP_TRF_CLR_OFFSET 15
#define FIELD_ARP_TRF_CHG_OFFSET 17
#define FIELD_ARP_TRF_SEL_OFFSET 18

//NEIGHBOR Traffic
#define REG_NEIGHBOR_TRF	0x00800018

//PPPoE Traffic
#define REG_PPPOE_TRF 0x00800014



int assert_eq(int func_return,int expect_return,const char* func,int line);

#ifndef ASSERT_EQ
#if 0
#define ASSERT_EQ(value1,value2)\
do {\
		int func_return;\
		int expect_return;\
		func_return=value1;\
		expect_return=value2;\
		if ((func_return) != (expect_return)) {\
				rtlglue_printf("\033[31;43m%s(%d): func_return=0x%x expect_return=0x%x, fail, so abort!\033[m\n", __FUNCTION__, __LINE__,(func_return),(expect_return));\
				return func_return; \
		}\
}while (0)
#else
#define ASSERT_EQ(value1, value2) do { if(assert_eq((value1), (value2), __FUNCTION__,__LINE__)!=0) return value1; }while(0)
#endif

#endif



//RTK API sync with rg_db MACRO
int32 RTK_L2_ADDR_ADD(rtk_l2_ucastAddr_t *pL2Addr);
int32 RTK_L2_ADDR_DEL(rtk_l2_ucastAddr_t *pL2Addr);
int32 RTK_L34_NETIFTABLE_SET(uint32 idx, rtk_l34_netif_entry_t *entry);
int32 RTK_L34_ROUTINGTABLE_SET(uint32 idx, rtk_l34_routing_entry_t *entry);
int32 RTK_L34_EXTINTIPTABLE_SET(uint32 idx, rtk_l34_ext_intip_entry_t *entry);
int32 RTK_L34_NEXTHOPTABLE_SET(uint32 idx, rtk_l34_nexthop_entry_t *entry);
int32 RTK_L34_PPPOETABLE_SET(uint32 idx, rtk_l34_pppoe_entry_t *entry);
int32 RTK_L34_ARPTABLE_SET(uint32 idx, rtk_l34_arp_entry_t *entry);
int32 RTK_L34_NAPTINBOUNDTABLE_SET(int8 forced,uint32 idx, rtk_l34_naptInbound_entry_t *entry);
int32 RTK_L34_NAPTOUTBOUNDTABLE_SET(int8 forced,uint32 idx, rtk_l34_naptOutbound_entry_t *entry);
int32 RTK_L34_WANTYPETABLE_SET(uint32 idx,rtk_wanType_entry_t * entry);
int32 RTK_VLAN_CREATE(rtk_vlan_t vid);
int32 RTK_VLAN_PORT_SET(rtk_vlan_t vid,rtk_portmask_t * pMember_portmask,rtk_portmask_t * pUntag_portmask);
int32 RTK_VLAN_EXTPORT_SET(rtk_vlan_t vid,rtk_portmask_t * pExt_portmask);
int32 RTK_VLAN_FID_SET(rtk_vlan_t vid,rtk_fid_t fid);
int32 RTK_VLAN_FIDMODE_SET(rtk_vlan_t vid,rtk_fidMode_t mode);
int32 RTK_VLAN_PRIORITY_SET(rtk_vlan_t vid,rtk_pri_t priority);
int32 RTK_VLAN_PRIORITYENABLE_SET(rtk_vlan_t vid,rtk_enable_t enable);
int32 RTK_VLAN_DESTROY(rtk_vlan_t vid);
int32 RTK_VLAN_PORTPVID_SET(rtk_port_t port,uint32 pvid);
int32 RTK_VLAN_EXTPORTPVID_SET(uint32 extPort,uint32 pvid);
int32 RTK_L34_BINDINGTABLE_SET(uint32 idx,rtk_binding_entry_t * bindEntry);
int32 RTK_L34_IPV6ROUTINGTABLE_SET(uint32 idx,rtk_ipv6Routing_entry_t * ipv6RoutEntry);
int32 RTK_L34_IPV6NEIGHBORTABLE_SET(uint32 idx,rtk_ipv6Neighbor_entry_t * ipv6NeighborEntry);

/*add  Boyce 2014-10-15*/

int32 RTK_VLAN_PORTPROTOVLAN_SET(rtk_port_t port,uint32 protoGroupIdx,rtk_vlan_protoVlanCfg_t * pVlanCfg);
void _rtk_rg_cleanPortAndProtocolSettings(rtk_port_t port);
int _rtk_rg_broadcastWithDscpRemarkMask_get(unsigned int bcMask,unsigned int *bcWithoutDscpRemarMask,unsigned int *bcWithDscpRemarByInternalpriMask,unsigned int *bcWithDscpRemarByDscpkMask);
int _rtk_rg_broadcastWithDot1pRemarkMask_get(unsigned int bcMask,unsigned int *bcWithDot1pRemarkMask,unsigned int *bcWithoutDot1pRemarkMask);
int32 _rtk_rg_sendBroadcastToWan(rtk_rg_pktHdr_t *pPktHdr, struct sk_buff *skb, int wanIdx, unsigned int dpMask);
int _rtk_rg_BroadcastPacketToLanWithEgressACLModification(int direct, int naptIdx, rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb,int l3Modify,int l4Modify, unsigned int bcTxMask, unsigned int allDestPortMask,  rtk_rg_port_idx_t egressPort);
int rtk_rg_debug_level_show(struct seq_file *s, void *v);
int rtk_rg_debug_level_change(struct file *file, const char *buffer, unsigned long count, void *data);
int rtk_rg_traceFilterShow(struct seq_file *s, void *v);
int rtk_rg_traceFilterChange(struct file *file, const char *buffer, unsigned long count, void *data);
#ifdef __KERNEL__
#ifdef CONFIG_RG_CALLBACK
int rtk_rg_callback_show(struct seq_file *s, void *v);
#endif
#endif
int rtk_rg_qosInternalAndRemark_show(struct seq_file *s, void *v);
rtk_rg_successFailReturn_t _rtk_rg_skipARPLearningOrNot(int l3Idx, ipaddr_t sip, int srcPortIdx);
int _rtk_rg_softwareArpTableDel(rtk_rg_arp_linkList_t *pDelArpEntry);
int _rtk_rg_freeRecentlyNotUsedArpList(void);
int _rtk_rg_softwareArpTableAdd(unsigned short routingIdx, ipaddr_t ipv4Addr, int l2Idx, int staticEntry);
int _rtk_rg_internal_GWMACSetup(ipaddr_t ipAddr, int l2Idx);
//int32 _rtk_rg_bindingRuleCheck(rtk_rg_pktHdr_t *pPktHdr, int *wanGroupIdx);
rtk_rg_successFailReturn_t _rtk_rg_createGatewayMacEntry(uint8 *gatewayMac, int vlanID, uint32 untagSet, int intfIdx);
int32 _rtk_rg_updateWANPortBasedVID(rtk_rg_port_idx_t wan_port);
void _rtk_rg_refreshPPPoEPassThroughLanOrWanPortMask(void);
int _rtk_rg_globalVariableReset(void);
void rtk_rg_fwdEngineHouseKeepingTimerFunc(unsigned long task_priv);
void _rtk_rg_set_initState(rtk_rg_initState_t newState);
int _rtk_rg_updatePortBasedVIDByLanOrder(rtk_portmask_t mac_pmask, rtk_portmask_t etp_pmask);


int _rtk_rg_egressPacketSend_for_gponDsBcFilterAndRemarking(struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr, int toMasterWifi);

#if 0
int32 _rtk_rg_cf_reserved_pon_intfSSIDRemap_add(int intfIdx,int ssid, int vid);
int32 _rtk_rg_cf_reserved_pon_intfSSIDRemap_del(int intfIdx);
#endif

//setup aclSWEntry
int _rtk_rg_acl_asic_init(void);
int _rtk_rg_classify_asic_init(void);
int _rtk_rg_portmask_translator(rtk_rg_portmask_t in_pmask, rtk_portmask_t* out_mac_pmask, rtk_portmask_t* out_ext_pmask);
int _rtk_rg_is_aclSWEntry_init(void);
int _rtk_rg_aclSWEntry_init(void);
int _rtk_rg_aclSWEntry_get(int index, rtk_rg_aclFilterEntry_t* aclSWEntry);
int _rtk_rg_aclSWEntry_set(int index, rtk_rg_aclFilterEntry_t aclSWEntry);
int _rtk_rg_aclSWEntry_empty_find(int* index);
int _rtk_rg_aclSWEntry_dump(void);
int _rtk_rg_classifySWEntry_init(void);
int _rtk_rg_classifySWEntry_get(int index, rtk_rg_classifyEntry_t* cfSWEntry);
int _rtk_rg_classifySWEntry_set(int index, rtk_rg_classifyEntry_t cfSWEntry);
int _rtk_rg_is_stormControlEntry_init(void);
int _rtk_rg_stormControlEntry_init(void);

int32 _rtk_rg_acl_reserved_stag_ingressCVidFromPVID(uint32 in_pvid, uint32 in_port);
#if 1
int32 _rtk_rg_acl_reserved_pppoePassthrough_IntfisPppoewan_add(int intf_idx, rtk_mac_t gmac);
int32 _rtk_rg_acl_reserved_pppoePassthrough_IntfisPppoewan_del(int intf_idx);
void _rtk_rg_acl_reserved_pppoePassthroughDefaultRule_add(unsigned int lan_pmask, unsigned int wan_pmsk, int remark_vid);
void _rtk_rg_acl_reserved_pppoePassthroughDefaultRule_del(void);
#endif



#if 0
int32 _rtk_rg_acl_trap_broadcast_add(void);
int32 _rtk_rg_acl_trap_broadcast_del(void);
#endif

int32 _rtk_rg_acl_reserved_wifi_extPMaskTranslate_add(int patchFor,unsigned int igr_extPmsk,unsigned int egr_extPmsk);
int _rtk_rg_ingressACLPatternCheck(rtk_rg_pktHdr_t *pPktHdr, int ingressCvidRuleIdxArray[]);
rtk_rg_fwdEngineReturn_t _rtk_rg_ingressACLAction(rtk_rg_pktHdr_t *pPktHdr);
int _rtk_rg_egressACLPatternCheck(int direct, int naptIdx, rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb,int l3Modify,int l4Modify,rtk_rg_port_idx_t egressPort);
int _rtk_rg_egressACLAction(int direct, rtk_rg_pktHdr_t *pPktHdr);
int _rtk_rg_modifyPacketByACLAction(struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr,rtk_rg_port_idx_t egressPort);
int _rtk_rg_TranslateVlanSvlan2Packet(struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr, uint32 dataPathToWifi);
int _rtk_rg_egressPacketSend(struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr);
int _rtk_rg_dscpRemarkToSkb(rtk_rg_qosDscpRemarkSrcSelect_t dscpSrc,rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb, int dscp_off);
int _rtk_rg_qosDscpRemarking(rtk_rg_mac_port_idx_t egressPort,rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb);
void _rtk_rg_hwnatACLManipulate(rtk_enable_t natSwitch);
int _rtk_rg_getPortLinkupStatus(void);
int _rtk_rg_egressPortMaskCheck(rtk_rg_pktHdr_t *pPktHdr, unsigned int bcTxMask, unsigned int allDestPortMask);
int _rtk_rg_egressPacketDoQosRemarkingDecision(rtk_rg_pktHdr_t *pPktHdr, struct sk_buff *skb, struct sk_buff *bcSkb, unsigned int dpMask, unsigned int internalVlanID);


#if defined(CONFIG_RG_IGMP_SNOOPING) || defined(CONFIG_RG_MLD_SNOOPING)
/*IGMP*/
int32 _rtk_rg_igmpReport_portmask_check_and_limit(rtk_rg_pktHdr_t *pPktHdr, uint32* egress_port_mask);
#endif

#if defined(CONFIG_RG_IGMP_SNOOPING) || defined(CONFIG_RG_MLD_SNOOPING)
int32 _rtk_rg_igmpMldQuery_portmask_check_and_limit(rtk_rg_pktHdr_t *pPktHdr, uint32* egress_port_mask);
#endif


/* MAC/URL Filter */
int _rtk_rg_is_macFilter_table_init(void);
int _rtk_rg_macFilter_table_init(void);
int _rtk_rg_is_urlFilter_table_init(void);
int _rtk_rg_urlFilter_table_init(void);


/* NIC Module */
#define RG_FWDENGINE_PKT_LEN	2048
#define RX_OFFSET	2

/* LUT */
rtk_rg_lookupIdxReturn_t _rtk_rg_macLookup(u8 *pMac, int vlanId);
void _rtk_rg_layer2CleanL34ReferenceTable(int l2Idx);
rtk_rg_successFailReturn_t _rtk_rg_softwareLut_add(rtk_rg_macEntry_t *macEntry, int hashIdx, char category, char wlan_dev_index);
rtk_rg_successFailReturn_t _rtk_rg_softwareLut_addFromHw(int l2Idx, char category);
void _rtk_rg_softwareLut_allDelete(void);
int _rtk_rg_wlanDeviceCount_dec(int wlan_idx, unsigned char *macAddr, int *dev_idx);

void _rtk_rg_lutReachLimit_init(rtk_rg_accessWanLimitType_t type, void (*function)(unsigned long), unsigned long data);
void _rtk_rg_lutReachLimit_category(unsigned long category);
void _rtk_rg_lutReachLimit_portmask(unsigned long portmsk);
rtk_rg_err_code_t _rtK_rg_checkCategoryPortmask(rtk_l2_ucastAddr_t *lut);
int32 _rtK_rg_checkCategoryPortmask_spa(rtk_rg_port_idx_t spa);
int32 _rtk_rg_checkPortNotExistByPhy(rtk_rg_port_idx_t port);
rtk_rg_entryGetReturn_t _rtk_rg_findAndReclamIpmcEntry(ipaddr_t sip,ipaddr_t dip,int32 ivlsvl,int32 vid_fid,int32 ipfilterIdx,int32 ipfilterEn);


/* NAPT */
int _rtk_rg_l3lookup(ipaddr_t ip);
unsigned int _rtk_rg_NAPTIndex_get(unsigned char isTCP, unsigned int srcAddr, unsigned short srcPort, unsigned int destAddr, unsigned short destPort);
#if defined(CONFIG_APOLLO)
uint32 _rtk_rg_NAPTRIndex_get(uint16 isTCP, uint32 dip, uint16 dport);
unsigned int _rtk_rg_NAPTRemoteHash_get( unsigned int ip, unsigned int port);
rtk_rg_extPortGetReturn_t _rtk_rg_naptExtPortGetAndUse(int force,int isTcp,uint16 wishPort,uint32 *pIp,int addRefCnt);
#elif defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
uint32 _rtk_rg_NAPTRIndex_get(uint16 isTCP,uint32 sip,uint32 sport, uint32 dip, uint16 dport);
unsigned int _rtk_rg_NAPTRemoteHash_get( uint16 isTCP,unsigned int ip, unsigned int port);
int _rtk_rg_naptExtPortGetAndUse(int force,int isTcp,uint32 sip,uint16 sport,uint16 wishPort,uint32 *pIp,int addRefCnt);
#endif
int _rtk_rg_naptExtPortFree(int fromPS,int isTcp,uint16 port);
int32 _rtk_rg_shortCut_clear(void);
#ifdef CONFIG_ROME_NAPT_SHORTCUT
int32 _rtk_rg_v4ShortCut_delete(int deleteIdx);
#endif
#ifdef CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT
int32 _rtk_rg_v6ShortCut_delete(int deleteIdx);
#endif
rtk_rg_fwdEngineReturn_t _rtk_rg_fwdEngine_connType_lookup(rtk_rg_pktHdr_t *pPktHdr, ipaddr_t *transIP, int16 *transPort);
int32 _rtk_rg_delNaptShortCutEntrybyOutboundIdx(int outIdx);
uint32 _rtk_rg_shortcutHashIndex(uint32 srcIP, uint32 destIP, uint16 srcPort, uint16 destPort);
uint32 _rtk_rg_ipv6ShortcutHashIndex(uint32 srcIP, uint16 srcPort, uint16 destPort);
rtk_rg_fwdEngineReturn_t _rtk_rg_naptPriority_assign(int direct, rtk_rg_pktHdr_t *pPktHdr, rtk_rg_table_naptOut_t *naptout_entry, rtk_rg_table_naptIn_t *naptin_entry);
rtk_rg_entryGetReturn_t _rtk_rg_naptPriority_pattern_check(int rule_direction, int pkt_direction, rtk_rg_pktHdr_t *pPktHdr, rtk_rg_sw_naptFilterAndQos_t *pNaptPriorityRuleStart);
void _rtk_rg_fwdEngine_updateFlowStatus(struct sk_buff *skb,rtk_rg_pktHdr_t *pPktHdr);
void _rtk_rg_naptInfoCollectForCallback(int naptOutIdx, rtk_rg_naptInfo_t *naptInfo);


/* platform */
int32 _rtk_rg_platform_function_register_check(struct platform *pf);


/* IPv6 */
uint8 _rtk_rg_CompareIFID(uint8* dip, uint64 interfaceid);
int _rtk_rg_v6L3lookup(unsigned char *ipv6);


/* CP3 PROFILE */
int profile(const char *func);
void profile_end(int idx);

/* debug */
void _rtk_rg_psRxMirrorToPort0(struct sk_buff *skb, struct net_device *dev);



/*Other Module*/

//extern rtk_rg_arp_request_t	RG_GLB_ARP_REQUEST[MAX_NETIF_HW_TABLE_SIZE];

//called back from Layer 2, return the IPaddr and mac for gateway 
//if the ARP request is not found or timeout, return L2Idx = -1
int32 _rtk_rg_internal_wanSet(int wan_intf_idx, rtk_rg_ipStaticInfo_t *hw_static_info);
int _rtk_rg_internal_GWMACSetup(ipaddr_t IPaddr, int L2Idx);	
int _rtk_rg_internal_GWMACSetup_stage2(int matchIdx, int l2Idx);
int _rtk_rg_internal_PPTPMACSetup(ipaddr_t ipAddr, int l2Idx);
void _rtk_rg_PPTPLearningTimerInitialize(int wan_intf_idx);
int _rtk_rg_internal_L2TPMACSetup(ipaddr_t ipAddr, int l2Idx);
void _rtk_rg_L2TPLearningTimerInitialize(int wan_intf_idx);
int _rtk_rg_internal_IPV6GWMACSetup(unsigned char *ipv6Addr, int L2Idx);	
int _rtk_rg_internal_IPV6GWMACSetup_stage2(int matchIdx, int l2Idx);
int _rtk_rg_globalVariableReset(void);
void _rtk_rg_fwdEngineGlobalVariableReset(void);
int _rtk_rg_layer2GarbageCollection(int l2Idx);
rtk_rg_entryGetReturn_t _rtk_rg_layer2LeastRecentlyUsedReplace(int l2Idx);
int _rtk_rg_decreaseNexthopReference(int nexthopIdx);
rtk_rg_fwdEngineReturn_t _rtk_rg_bindingRuleCheck(rtk_rg_pktHdr_t *pPktHdr, int *wanGroupIdx);
int32 rtk_rg_ipv6_externalIp_set(int index, rtk_rg_table_v6ExtIp_t v6ExtIp_entry);
int32 _rtk_rg_ipv6_externalIp_get(int index, rtk_rg_table_v6ExtIp_t *v6ExtIp_entry);
rtk_rg_fwdEngineReturn_t _rtk_rg_checkGwIp(rtk_rg_pktHdr_t *pPktHdr);
int32 _rtk_rg_updateBindWanIntf(rtk_rg_wanIntfConf_t *wanintf);
int32 _rtk_rg_updateBindOtherWanPortBasedVID(rtk_rg_wanIntfConf_t *otherWanIntf);
int32 _rtk_rg_updateNoneBindingPortmask(uint32 wanPmsk);


#ifdef __KERNEL__
//void _rtk_rg_switchLinkChangeHandler(void);

void _rtk_rg_switchLinkChangeHandler(intrBcasterMsg_t *pMsgData);
int _rtk_rg_broadcastForwardWithPkthdr(rtk_rg_pktHdr_t *pPktHdr, struct sk_buff *skb, unsigned int internalVlanID, unsigned int srcPort,unsigned int extSpa);
#endif

#define RG_ONE_COUNT(x)\
do {\
	x = (x & 0x55555555) + ((x & 0xaaaaaaaa) >> 1);\
	x = (x & 0x33333333) + ((x & 0xcccccccc) >> 2);\
	x = (x & 0x0f0f0f0f) + ((x & 0xf0f0f0f0) >> 4);\
    x = (x & 0x00ff00ff) + ((x & 0xff00ff00) >> 8);\
    x = (x & 0x0000ffff) + ((x & 0xffff0000) >> 16);\
} while (0)

typedef struct _rg_DHCPStaticEntry_s
{
	ipaddr_t ip;
	rtk_mac_t mac;
	int valid;
} _rg_DHCPStaticEntry_t;

unsigned int _rtk_rg_hash_mac_fid_efid(unsigned char *mac,unsigned int fid,unsigned int efid);
unsigned int _rtk_rg_hash_mac_vid_efid(unsigned char *mac,unsigned int vid,unsigned int efid);
#if defined(CONFIG_RTL9602C_SERIES)
unsigned int _hash_dip_vidfid_sipidx_sipfilter(int is_ivl,unsigned int dip,unsigned int vid_fid,unsigned int sip_idx,unsigned int sip_filter_en);
unsigned int _hash_ipm_dipv6(uint8 *dip);
#elif defined(CONFIG_RTL9600_SERIES)
unsigned int _rtk_rg_hash_sip_gip(unsigned int sip,unsigned int gip,int isIvl,int vid_fid);
#endif


#if defined(CONFIG_RTL8686NIC)||!(defined(__KERNEL__)) || defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
void _rtk_rg_interfaceVlanIDPriority(rtk_rg_pktHdr_t *pPktHdr,rtk_rg_intfInfo_t *pStoredInfo,struct tx_info *ptxInfo,struct tx_info *ptxInfoMask);
void _rtk_rg_interfaceVlanIDPriority_directTX(rtk_rg_pktHdr_t *pPktHdr,rtk_rg_intfInfo_t *pStoredInfo,struct tx_info *ptxInfo,struct tx_info *ptxInfoMask);
#else
void _rtk_rg_interfaceVlanIDPriority(rtk_rg_pktHdr_t *pPktHdr,rtk_rg_intfInfo_t *pStoredInfo,rtk_rg_txdesc_t *ptxInfo,rtk_rg_txdesc_t *ptxInfoMask);
void _rtk_rg_interfaceVlanIDPriority_directTX(rtk_rg_pktHdr_t *pPktHdr,rtk_rg_intfInfo_t *pStoredInfo,rtk_rg_txdesc_t *ptxInfo,rtk_rg_txdesc_t *ptxInfoMask);
#endif
void _rtk_rg_wanVlanTagged(rtk_rg_pktHdr_t *pPktHdr,int vlan_tag_on);
void _rtk_rg_addPPPoETag(rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb,int l3Modify,int l4Modify);
void _rtk_rg_removeTunnelTag(struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr);
int _rtk_rg_arpGeneration(uint8 netIfIdx,ipaddr_t gwIpAddr,rtk_rg_arp_request_t *arpReq);
int _rtk_rg_NDGeneration(uint8 netIfIdx,rtk_ipv6_addr_t gwIpAddr,rtk_rg_neighbor_discovery_t *neighborDisc);
rtk_rg_successFailReturn_t _rtk_rg_arpAndMacEntryAdd(ipaddr_t sip,int sipL3Idx,uint8 *pSmac,int srcPortIdx, int srcWlanDevIdx, int *pL2Idx,int cvid,int cvidForceAdd,int arpEntryForceAdd);
int _rtk_rg_naptConnection_add(int naptIdx, rtk_l34_naptOutbound_entry_t *asic_napt, rtk_l34_naptInbound_entry_t *asic_naptr);
uint32 _rtk_rg_naptTcpUdpOutHashIndex(int8 isTcp, ipaddr_t srcAddr, uint16 srcPort, ipaddr_t destAddr, uint16 destPort);
#if defined(CONFIG_APOLLO)
uint32 _rtk_rg_naptTcpUdpInHashIndex(uint16 isTcp, uint32 dip, uint16 dport);
#elif defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
uint32 _rtk_rg_naptTcpUdpInHashIndex(uint16 isTcp,uint32 sip,uint16 sport, uint32 dip, uint16 dport);
#endif
rtk_rg_lookupIdxReturn_t _rtk_rg_naptTcpUdpOutHashIndexLookup(int8 isTcp, ipaddr_t srcAddr, uint16 srcPort, ipaddr_t destAddr, uint16 destPort);
rtk_rg_lookupIdxReturn_t _rtk_rg_naptTcpUdpOutHashIndexLookupByPktHdr(int8 isTcp, rtk_rg_pktHdr_t *pPktHdr);
rtk_rg_lookupIdxReturn_t _rtk_rg_naptTcpUdpInHashIndexLookup(int8 isTcp, ipaddr_t remoteAddr, uint16 remotePort, ipaddr_t extAddr, uint16 extPort);
unsigned char _rtk_rg_IPv6NeighborHash(unsigned char *dip, unsigned char rtidx);
rtk_rg_successFailReturn_t _rtk_rg_neighborAndMacEntryAdd(unsigned char *sip,int sipOrDipL3Idx,uint8 *pSmac,int srcPortIdx,int srcWlanDevIdx,int *pNeighborOrMacIdx);
rtk_rg_fwdEngineReturn_t _rtk_rg_fwdEngine_naptPacketModify(int direct, int naptIdx, rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb,int l3Modify,int l4Modify);


int32 _rtk_rg_addArpRoutingArray(rtk_rg_routing_arpInfo_t *newAddingEntry, ipaddr_t newIpAddr, int intfVlanId);
int32 _rtk_rg_delArpRoutingArray(rtk_rg_routing_arpInfo_t *deletingEntry);
int32 _rtk_rg_arpRearrange(rtk_rg_routing_arpInfo_t *newAddingEntry, ipaddr_t newIpAddr, int routingARPNum);

rtk_rg_fwdEngineReturn_t _rtk_rg_fwdEngineDirectTx(struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr);
int _rtk_rg_fwdengine_handleArpMiss(rtk_rg_pktHdr_t *pPktHdr);
int _rtk_rg_fwdengine_handleArpMissInRoutingLookUp(rtk_rg_pktHdr_t *pPktHdr);
rtk_rg_successFailReturn_t _rtk_rg_fwdEngine_upnpCheck(void*,ipaddr_t*,uint16*);
rtk_rg_successFailReturn_t _rtk_rg_fwdEngine_virtualServerCheck(void*,ipaddr_t*,uint16*);
rtk_rg_successFailReturn_t _rtk_rg_fwdEngine_dmzCheck(void*,ipaddr_t*,uint16*);
int _rtk_rg_eiplookup(ipaddr_t ip);
int _rtk_rg_softwareArpTableLookUp(unsigned short routingIdx, ipaddr_t ipAddr, rtk_rg_arp_linkList_t **pSoftwareArpEntry, int resetIdleTime);
#if defined(CONFIG_RTL9602C_SERIES)
int _rtk_rg_hardwareArpTableLookUp(unsigned short routingIdx, ipaddr_t ipAddr, rtk_rg_arp_linkList_t **pSoftwareArpEntry, int resetIdleTime);
int _rtk_rg_hardwareArpTableAdd(unsigned short routingIdx, ipaddr_t ipv4Addr, int l2Idx, int staticEntry, uint16 *hwArpIdx);
int _rtk_rg_hardwareArpTableDel(rtk_rg_arp_linkList_t *pDelArpEntry);
#endif
#if defined(CONFIG_RTL9600_SERIES) || defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
#else	//support lut traffic bit
int _rtk_rg_lutCamListAdd(int l2Idx, uint32 addLutCamIdx);
int _rtk_rg_lutCamListDel(uint32 delLutCamIdx);
#endif

int _rtk_rg_naptExtPortInUsedCheck(int force, int isTcp,uint16 wishPort,int addRefCount);
int32 _rtk_rg_apollo_naptFilterAndQos_init(void);
#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
void _rtk_rg_fwdEngine_ipv6ConnList_del(rtk_rg_ipv6_layer4_linkList_t *pConnList);
#endif

int _rtk_rg_dot1pPriRemarking2InternalPri_search(int target_rmk_dot1p, int *duplicateSize,int* intPri);
int32 _rtk_rg_cvidCpri2Sidmapping_get(uint16 cvid, uint8 cpri, uint8 *sid);

//sw rate limit
rtk_rg_fwdEngineReturn_t _rtk_rg_unknownDARateLimit_check(struct sk_buff *skb,rtk_rg_pktHdr_t *pPktHdr);

rtk_rg_fwdEngineReturn_t _rtk_rg_BCMCRateLimit_check(struct sk_buff *skb,rtk_rg_pktHdr_t *pPktHdr);


int _rtk_rg_fwdEngine_ipv6DmzCheck(void *data,rtk_ipv6_addr_t *transIP,uint16 *transPort);
int _rtk_rg_fwdEngine_ipv6VirtualServerCheck(void *data,rtk_ipv6_addr_t *transIP,uint16 *transPort);



//Multicast
//1 FIXME: this declaration should be delete when the liteRomeDriver.h is ready!!
//#ifdef CONFIG_APOLLO
void _rtk_rg_apollo_ipmcMultiCast_transTbl_add(int32 isIpv6,rtk_l2_ipMcastAddr_t *lut);
int32 _rtk_rg_apollo_ipv4MultiCastFlow_add(rtk_rg_ipv4MulticastFlow_t *ipv4McFlow,int *flow_idx);
int32 _rtk_rg_apollo_l2MultiCastFlow_add(rtk_rg_l2MulticastFlow_t *l2McFlow,int *flow_idx);
rtk_rg_entryGetReturn_t _rtk_rg_findAndReclamL2mcEntry(rtk_mac_t *mac,int fid);
rtk_rg_entryGetReturn_t _rtk_rg_findAndReclamL2mcEntryIVL(rtk_mac_t *mac,int vid);
#if defined(CONFIG_RTL9602C_SERIES)
rtk_rg_entryGetReturn_t _rtk_rg_findAndReclamIPv6mcEntry( rtk_ipv6_addr_t *gipv6,int32 ivlsvl);
#endif
rtk_rg_entryGetReturn_t _rtk_rg_findAndReclamIpmcEntry(ipaddr_t sip,ipaddr_t dip,int32 ivlsvl,int32 vid_fid,int32 ipfilterIdx,int32 ipfilterEn);
//#endif

//IGMP
int32 rtl_flushAllIgmpRecord(int forceFlush);
int _rtk_rg_igmpSnoopingOnOff(int isOn, int onlyChangeTimer, int isIVL);



//ALG
int _rtk_rg_algCheckEnable(unsigned char isTCP,unsigned short checkPort);
rtk_rg_successFailReturn_t _rtk_rg_algSrvInLanCheckEnable(unsigned char isTCP, unsigned short checkPort);
rtk_rg_successFailReturn_t _rtk_rg_algDynamicPort_set(p_algRegisterFunction registerFunction, unsigned int serverInLan, ipaddr_t intIP, unsigned short int portNum, int isTCP, int timeout);

//WIFI
#if defined(__KERNEL__)&&defined(CONFIG_APOLLO)
extern int rtl8192cd_start_xmit(struct sk_buff *skb, struct net_device *dev);
#endif
void _rtk_rg_wlanMbssidLearning(u8* smac,rtk_rg_pktHdr_t *pPktHdr);
rtk_rg_lookupIdxReturn_t _rtk_rg_wlanMbssidLookup(u8 *dmac,rtk_rg_mbssidDev_t *wlan_dev_idx);
rtk_rg_mbssidDev_t _rtk_master_wlan_mbssid_tx(rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb);
extern struct net_device *wlan_root_netdev;
extern struct net_device *wlan_vap_netdev[4];
extern struct net_device *wlan_wds_netdev[8];
#ifdef CONFIG_RTL_CLIENT_MODE_SUPPORT
extern struct net_device *wlan_vxd_netdev;
#endif
#if defined(CONFIG_RG_WLAN_HWNAT_ACCELERATION) && !defined(CONFIG_ARCH_LUNA_SLAVE)
// wlan1 in master
extern struct net_device *wlan1_root_netdev;
extern struct net_device *wlan1_vap_netdev[4];
extern struct net_device *wlan1_wds_netdev[8];
#ifdef CONFIG_RTL_CLIENT_MODE_SUPPORT
extern struct net_device *wlan1_vxd_netdev;
#endif
#endif

//TOOL
void _rtk_rg_str2mac(unsigned char *mac_string,rtk_mac_t *pMacEntry);
unsigned short inet_chksum(unsigned char *dataptr, unsigned short len);
u16 inet_chksum_pseudo(u8 *tcphdr, u16 tcplen, u32 srcip, u32 destip,  u8 proto);
int _rtk_rg_pasring_proc_string_to_integer(const char *buff,unsigned long len);
void _rtk_rg_tcpShortTimeoutHouseKeep_set(uint32 jiffies_interval);



//DEBUG
int _rtk_rg_trace_filter_compare(struct sk_buff *skb,rtk_rg_pktHdr_t *pPktHdr);



//NIC
struct sk_buff *re8670_getAlloc(unsigned int size);
struct sk_buff *re8670_getBcAlloc(unsigned int size);
struct sk_buff *re8670_getMcAlloc(unsigned int size);

//#ifdef CONFIG_APOLLO  //xdsl need this function
int _rtk_rg_splitJumboSendToNicWithTxInfoAndMask(rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb, struct tx_info* ptxInfo, int ring_num, struct tx_info* ptxInfoMask);
rtk_rg_fwdEngineReturn_t _rtk_rg_splitJumboSendToMasterWifi(rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb, struct net_device *dev);
//#endif


//SKB
void _rtk_rg_dev_kfree_skb_any(struct sk_buff *skb);
struct sk_buff *_rtk_rg_dev_alloc_skb(unsigned int length);
struct sk_buff *_rtk_rg_getAlloc(unsigned int size);
struct sk_buff *_rtk_rg_skb_copy(const struct sk_buff *skb, unsigned gfp_mask);

struct sk_buff *_rtk_rg_skb_clone(struct sk_buff *skb, unsigned gfp_mask);


//Callback by workqueue
#ifdef __KERNEL__
void wq_do_initByHwCallBack(struct work_struct *p_work);
void wq_do_arpAddByHwCallBack(struct work_struct *p_work);
void wq_do_arpDelByHwCallBack(struct work_struct *p_work);
void wq_do_macAddByHwCallBack(struct work_struct *p_work);
void wq_do_macDelByHwCallBack(struct work_struct *p_work);
void wq_do_routingAddByHwCallBack(struct work_struct *p_work);
void wq_do_routingDelByHwCallBack(struct work_struct *p_work);
void wq_do_naptAddByHwCallBack(struct work_struct *p_work);
void wq_do_naptDelByHwCallBack(struct work_struct *p_work);
void wq_do_bindingAddByHwCallBack(struct work_struct *p_work);
void wq_do_bindingDelByHwCallBack(struct work_struct *p_work);
void wq_do_interfaceAddByHwCallBack(struct work_struct *p_work);
void wq_do_interfaceDelByHwCallBack(struct work_struct *p_work);
void wq_do_neighborAddByHwCallBack(struct work_struct *p_work);
void wq_do_neighborDelByHwCallBack(struct work_struct *p_work);
void wq_do_v6RoutingAddByHwCallBack(struct work_struct *p_work);
void wq_do_v6RoutingDelByHwCallBack(struct work_struct *p_work);
void wq_do_pppoeBeforeDiagByHwCallBack(struct work_struct *p_work);
void wq_do_dhcpRequestByHwCallBack(struct work_struct *p_work);
void wq_do_pptpBeforeDialByHwCallBack(struct work_struct *p_work);
void wq_do_l2tpBeforeDialByHwCallBack(struct work_struct *p_work);
void wq_do_pppoeDsliteBeforeDialByHwCallBack(struct work_struct *p_work);
#if 0
void wq_do_softwareNaptAddCallBack(struct work_struct *p_work);
void wq_do_softwareNaptDelCallBack(struct work_struct *p_work);
#endif
#endif


#ifdef CONFIG_SMP

// Timer to Tasklet in SMP
void rtk_rg_fwdEngineHouseKeepingTimerFuncTasklet(unsigned long task_priv);
#endif



//USERSPACE HELPER
/* used for command which need to pipe to file, ex: >  >>*/
int rtk_rg_callback_pipe_cmd(const char *comment,...);

uint16 _rtk_rg_fwdengine_L4checksumUpdate(uint8 acktag, uint16 ori_checksum, uint32 ori_ip, uint16 ori_port, uint32 ori_seq, uint32 ori_ack, uint32 new_ip, uint16 new_port, uint32 new_seq, uint32 new_ack);
uint16 _rtk_rg_fwdengine_L4checksumUpdateForMss(uint16 ori_checksum, uint16 ori_mss, uint16 new_mss);
uint16 _rtk_rg_fwdengine_L3checksumUpdate(uint16 ori_checksum, uint32 ori_sip, uint8 ori_ttl, uint8 ori_protocol, uint32 new_sip, uint8 new_ttl);
uint16 _rtk_rg_fwdengine_L3checksumUpdateDSCP(uint16 ori_checksum, uint8 header_length, uint8 ori_DSCP, uint8 new_DSCP);



#if 1//ndef CONFIG_APOLLO_TESTING

#ifdef __KERNEL__
#include <linux/if_vlan.h>
#include <linux/skbuff.h>


//VLAN TAG
#define VLAN_HLEN 4
#define VLAN_ETH_ALEN 6

struct vlan_double_ethhdr {
	unsigned char	h_dest[ETH_ALEN];
	unsigned char	h_source[ETH_ALEN];
	__be16		h_outter_vlan_proto;
	__be16		h_outter_vlan_content;
	__be16		h_inner_vlan_proto;
	__be16		h_inner_vlan_content;
	__be16		h_vlan_encapsulated_proto;
};

/**
 * _vlan_insert_tag - regular VLAN/SVLAN tag inserting 
 * @skb: skbuff to tag
 * @outter_tagif: insert outterTag or not
 * @outter_protocal: VLAN protocal, usually 0x8100 or ox88a8
 * @outter_content: VLAN content, usually is ((vlan_pri<<13)|(vlan_tci<<12)|vlan_id)
 * @inner_tagif: insert innerTag or not (only supported while outter_tagif is valid)
 * @inner_protocal: VLAN protocal, usually 0x8100 
 * @inner_content: VLAN content, usually is ((vlan_pri<<13)|(vlan_tci<<12)|vlan_id)
 *
 * Inserts the VLAN tag into @skb as part of the payload(at most add double vlan tags)
 * Returns a VLAN tagged skb. If a new skb is created, @skb is freed.
 *
 * Following the skb_unshare() example, in case of error, the calling function
 * doesn't have to worry about freeing the original skb.
 */
 static inline struct sk_buff *_vlan_insert_tag(rtk_rg_pktHdr_t *pPktHdr, struct sk_buff *skb,u16 outter_tagif,u16 outter_protocal,u16 outter_content,u16 inner_tagif,u16 inner_protocal,u16 inner_content)
{
	struct vlan_double_ethhdr *veth;
	int num_of_tag=0;
	

	if(outter_tagif){ //outter_tag must assign first
		num_of_tag++;
		
		if(inner_tagif)	
			num_of_tag++;
	}
	
	if((skb->tail - skb->data)!=skb->len)
		WARNING("[skb error] skb->head=0x%x, skb->data=0x%x, skb->tail=0x%x, skb->end=0x%x, skb->len=%d\n",
		skb->head, skb->data, skb->tail, skb->end, skb->len);
	if (skb_cow_head(skb, num_of_tag*VLAN_HLEN) < 0) {
		WARNING("skb head room is not enough..return without insert tag");
		return NULL;
	}

	veth = (struct vlan_double_ethhdr *)skb_push(skb, num_of_tag*VLAN_HLEN);

	/* Move the mac addresses to the beginning of the new header. */
	memmove(skb->data, skb->data + num_of_tag*VLAN_HLEN, 2 * VLAN_ETH_ALEN);
	skb->mac_header -= num_of_tag*VLAN_HLEN;

	if(outter_tagif){
		/* first, the outter prtocal type */
		veth->h_outter_vlan_proto = htons(outter_protocal);
		/* now, the outter content */
		veth->h_outter_vlan_content = htons(outter_content);

		skb->protocol = htons(outter_protocal);

		if(inner_tagif){
			/* the inner prtocal type */
			veth->h_inner_vlan_proto = htons(inner_protocal);
			/*the inner  content*/
			veth->h_inner_vlan_content = htons(inner_content);
		}
	}

	if(pPktHdr!=NULL){
		pPktHdr->l3Offset += (num_of_tag*VLAN_HLEN);
		pPktHdr->l4Offset += (num_of_tag*VLAN_HLEN);
	}

	return skb;
}

/**
 * _vlan_remove_tag - regular VLAN/SVLAN tag remove
 * @skb: skbuff to tag
 * @protocal: target VLAN protocal, usually 0x8100 or ox88a8
 *
 * Remove the VLAN tag payload from @skb 
 * Returns a tag-removed skb. 
 */
static inline struct sk_buff *_vlan_remove_tag(rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb, u16 protocal)
{
	if ((skb==NULL) || (skb->data==NULL) ) {
		return NULL;
	}

	//check the protocal is the same
	if((skb->data[2*VLAN_ETH_ALEN]<<0x8 | skb->data[2*VLAN_ETH_ALEN+1] )!= htons(protocal)){
		WARNING("Assigned Tag protocal is not found, removing tag failed.");
		return skb;
	}

	/* Move the mac addresses to the beginning of the new header. */
	//memmove(skb->data, skb->data - VLAN_HLEN, 2 * VLAN_ETH_ALEN);
	memmove(skb->data+VLAN_HLEN, skb->data, 2 * VLAN_ETH_ALEN);
	skb->protocol = pPktHdr->etherType;	//when we remove the vlan tag, we should recover skb->protocol from etherType
	skb->mac_header += VLAN_HLEN;
	skb->data += VLAN_HLEN;
	skb->len  -= VLAN_HLEN;

	if(pPktHdr!=NULL){
		if(pPktHdr->l3Offset!=0) //pPktHdr->l3Offset has been parsed
			pPktHdr->l3Offset -= VLAN_HLEN;
		if(pPktHdr->l4Offset!=0) //pPktHdr->l4Offset has been parsed
			pPktHdr->l4Offset -= VLAN_HLEN;
	}

	return skb;
}

/**
 * _vlan_remove_doubleTag - regular SVLAN and VLAN tag remove (can be using in fwEngine)
 * @skb: skbuff to tag (must have Stag and Ctag)
 *
 * Remove the VLAN tag payload from @skb 
 * Returns a tag-removed skb. 
 */
static inline struct sk_buff *_vlan_remove_doubleTag(rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb)
{
	if (skb==NULL || skb->data==NULL ) {
		return NULL;
	}

	//check the outter tag protocal 
	if((skb->data[2*VLAN_ETH_ALEN]<<0x8 | skb->data[2*VLAN_ETH_ALEN+1] )!= htons(rg_db.systemGlobal.tpid)){
		WARNING("stag protocal is not found, removing tag failed.");
		return skb;
	}
	//check the inner tag protocal 
	if((skb->data[2*VLAN_ETH_ALEN+VLAN_HLEN]<<0x8 | skb->data[2*VLAN_ETH_ALEN+VLAN_HLEN+1] )!= htons(0x8100)){
		WARNING("ctag protocal is not found, removing tag failed.");
		return skb;
	}

	/* Move the mac addresses to the beginning of the new header. */
	memmove(skb->data+(VLAN_HLEN*2), skb->data, 2 * VLAN_ETH_ALEN);
	skb->protocol = pPktHdr->etherType;	//when we remove the vlan tag, we should recover skb->protocol from etherType
	skb->mac_header += (VLAN_HLEN*2);
	skb->data += (VLAN_HLEN*2);
	skb->len  -= (VLAN_HLEN*2);

	if(pPktHdr!=NULL){
		if(pPktHdr->l3Offset!=0) //pPktHdr->l3Offset has been parsed
			pPktHdr->l3Offset -= (VLAN_HLEN*2);
		if(pPktHdr->l4Offset!=0) //pPktHdr->l4Offset has been parsed
			pPktHdr->l4Offset -= (VLAN_HLEN*2);
	}

	return skb;
}


/**
 * _vlan_modify_tag - outter modify, usually using in VLAN<=>SVLAN exchange
 * @skb: skbuff to tag
 * @ori_protocal: target VLAN protocal for checking is tag exist.
 * @mod_protocal: target VLAN protocal, usually 0x8100 or ox88a8
 * @mod_content: target VLAN content, usually is ((vlan_pri<<13)|(vlan_tci<<12)|vlan_id)
 *
 * Modify the VLAN tag payload from @skb 
 * Returns a modified skb. 
 */
static inline struct sk_buff *_vlan_modify_tag(rtk_rg_pktHdr_t *pPktHdr, struct sk_buff *skb, u16 ori_protocal,u16 mod_protocal,u16 mod_content)
{
	//struct vlan_ethhdr *veth;
	//u16 vlan_pattern;

	if (skb==NULL || skb->data==NULL ) {
		return NULL;
	}

	//check the protocal is the same
	if((skb->data[2*VLAN_ETH_ALEN]<<0x8 | skb->data[2*VLAN_ETH_ALEN+1] )!= htons(ori_protocal)){
		WARNING("Assigned Tag protocal is not found, modify tag failed.");
		return skb;
	}

	
	//modified protocal
	(*(u16*)(skb->data+2*VLAN_ETH_ALEN))=htons(mod_protocal);

	//modified content
	(*(u16*)(skb->data+2*VLAN_ETH_ALEN+2))=htons(mod_content);


	return skb;
}

/**
 * _vlan_modify_doubleTag - outter & inner tag modify
 * @skb: skbuff to tag
 * @mod_outter_protocal: target SVLAN protocal, usually 0x8100 or ox88a8
 * @mod_outter_content: target SVLAN content, usually is ((svlan_pri<<13)|(svlan_dei<<12)|svlan_id)
 * @mod_inner_protocal: target VLAN protocal, usually 0x8100 
 * @mod_inner_content: target VLAN content, usually is ((vlan_pri<<13)|(vlan_tci<<12)|vlan_id)
 *
 * Modify the VLAN tag payload from @skb 
 * Returns a modified skb. 
 */
static inline struct sk_buff *_vlan_modify_doubleTag(rtk_rg_pktHdr_t *pPktHdr, struct sk_buff *skb, u16 mod_outter_protocal,u16 mod_outter_content,u16 mod_inner_protocal,u16 mod_inner_content)
{
	//struct vlan_ethhdr *veth;
	//u16 vlan_pattern;

	if (skb==NULL || skb->data==NULL ) {
		return NULL;
	}

	//check the outter tag protocal 
#if defined(CONFIG_RTL9602C_SERIES)
	if((skb->data[2*VLAN_ETH_ALEN]<<0x8 | skb->data[2*VLAN_ETH_ALEN+1] )!= htons(pPktHdr->stagTpid)){
#else //9600 series
	if((skb->data[2*VLAN_ETH_ALEN]<<0x8 | skb->data[2*VLAN_ETH_ALEN+1] )!= htons(rg_db.systemGlobal.tpid)){
#endif
		WARNING("Assigned stag protocal is not found, removing tag failed.");
		return skb;
	}
	//check the inner tag protocal 
	if((skb->data[2*VLAN_ETH_ALEN+VLAN_HLEN]<<0x8 | skb->data[2*VLAN_ETH_ALEN+VLAN_HLEN+1] )!= htons(0x8100)){
		WARNING("Assigned ctag protocal is not found, removing tag failed.");
		return skb;
	}


	
	//modified outter protocal
	(*(u16*)(skb->data+2*VLAN_ETH_ALEN))=htons(mod_outter_protocal);

	//modified outter content
	(*(u16*)(skb->data+2*VLAN_ETH_ALEN+2))=htons(mod_outter_content);

	//modified inner protocal
	(*(u16*)(skb->data+2*VLAN_ETH_ALEN+VLAN_HLEN))=htons(mod_inner_protocal);

	//modified inner content
	(*(u16*)(skb->data+2*VLAN_ETH_ALEN+VLAN_HLEN+2))=htons(mod_inner_content);


	return skb;
}

rtk_rg_initState_t _rtk_rg_get_initState(void);


#endif

#endif


#endif

