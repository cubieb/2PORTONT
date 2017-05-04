#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>

#include<rtk/mib.h>
#include<rtk/utility.h>

#include<feature_export_api.h>
#include<omci_dm_sd.h>

#define DEBUG
#ifdef DEBUG
#define DBG_LOG(...) fprintf(stderr, "[libtr142] "__VA_ARGS__)
#else
#define DBG_LOG while(0){}
#endif

/* Utilities */
static void get_wan_by_if_id(unsigned int if_id, MIB_CE_ATM_VC_Tp pEntry, int *idx)
{
	int total, i;

	if(pEntry == NULL)
		return;

	*idx = -1;
	total = mib_chain_total(MIB_ATM_VC_TBL);

	for(i = 0 ; i < total ; i++)
	{
		if(mib_chain_get(MIB_ATM_VC_TBL, i, pEntry)== 0)
			continue;

		if(pEntry->omci_configured == 0)
			continue;

		if(pEntry->omci_if_id == if_id)
		{
			*idx = i;
			return;
		}
	}
}

static int get_new_ifIndex()
{
	int ifMap = 0;
	int total = mib_chain_total(MIB_ATM_VC_TBL);
	MIB_CE_ATM_VC_T entry;
	int i = 0;

	for (i=0; i<total; i++)
	{
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&entry))
			continue;

		if(MEDIA_ETH == MEDIA_INDEX(entry.ifIndex))
			ifMap |= 1 << PTM_INDEX(entry.ifIndex);	// vc map
	}

	return if_find_index(CHANNEL_MODE_IPOE, ifMap);
}

static int init_wan_entry(MIB_CE_ATM_VC_Tp pEntry)
{
#if defined(CONFIG_LUNA) && defined(GEN_WAN_MAC)
	char macaddr[MAC_ADDR_LEN] = {0};
#endif

	memset(pEntry, 0, sizeof(MIB_CE_ATM_VC_T));

	pEntry->brmode = BRIDGE_DISABLE;
	pEntry->cmode = CHANNEL_MODE_IPOE;

	pEntry->ifIndex = get_new_ifIndex(pEntry);

	if(pEntry->ifIndex == NA_VC)
	{
		fprintf(stderr, "Error: No ifIndex available!\n");
		return -1;
	}
	pEntry->ifIndex = TO_IFINDEX(MEDIA_ETH, PPP_INDEX(pEntry->ifIndex), PTM_INDEX(pEntry->ifIndex));
	pEntry->mtu = 1500;

#if defined(CONFIG_LUNA) && defined(GEN_WAN_MAC)
	mib_get(MIB_ELAN_MAC_ADDR, (void *)macaddr);
	macaddr[MAC_ADDR_LEN-1] += WAN_HW_ETHER_START_BASE + ETH_INDEX(pEntry->ifIndex);
	memcpy(pEntry->MacAddr, macaddr, MAC_ADDR_LEN);
#endif

	pEntry->omci_configured = 1;

	return 0;
}

/* OMCI Operations */
static int omci_op_reset_if()
{
	int cnt = 0, i;
	MIB_CE_ATM_VC_T entry;
	int total;

	DBG_LOG("%s is called.\n", __FUNCTION__);

	total = mib_chain_total(MIB_ATM_VC_TBL);

	for(i = total - 1 ; i >= 0 ; i--)
	{
		if(mib_chain_get(MIB_ATM_VC_TBL, i, &entry)== 0)
			continue;

		if(entry.omci_configured == 0)
			continue;

		deleteConnection(CONFIGONE, &entry);
		mib_chain_delete(MIB_ATM_VC_TBL, i);
		cnt++;
	}

	if(cnt > 0)
		restartWAN(CONFIGALL, NULL);

	return cnt;
}

static int omci_op_set_if(if_info_t *info)
{
	int idx;
	MIB_CE_ATM_VC_T entry = {0};

	DBG_LOG("[libtr142] %s is called.\n", __FUNCTION__);

	get_wan_by_if_id(info->if_id, &entry, &idx);

	if(idx < 0)
	{
		if(init_wan_entry(&entry) < 0)
			return -1;
	}
	else if(entry.enable)
		deleteConnection(CONFIGONE, &entry);

	//TODO: If IPv6/IPv4 stack is disabled
	DBG_LOG("if_is_ip_stack_B = %d\n", info->if_is_ip_stack_B);
	//entry.enable = (info->if_is_ip_stack_B == 0) ? 0 : 1;
	entry.enable = 1;

	entry.omci_if_id = info->if_id;
	DBG_LOG("if_tci = %d\n", info->if_tci);
	if(info->if_tci == 0xffff)
	{
		entry.vlan = 0;
	}
	else
	{
		entry.vlan = 1;
		entry.vid = info->if_tci & 0xfff;
		entry.vprio = info->if_tci >> 13;
	}

	/* Ignore data */
//	if(info->if_service_type & IF_SERVICE_DATA)
//		entry.applicationtype |= X_CT_SRV_INTERNET;
	if(info->if_service_type & IF_SERVICE_TR69)
		entry.applicationtype |= X_CT_SRV_TR069;
	if(info->if_service_type & IF_SERVICE_SIP)
		entry.applicationtype |= X_CT_SRV_VOICE;

	if(!info->if_is_ipv6_B)
	{
		entry.IpProtocol |= IPVER_IPV4;

		if(info->if_is_DHCP_B)
		{
			//DHCP
			entry.ipDhcp = DHCP_CLIENT;
			entry.dnsMode = 1;	//get dns automatically
		}
		else
		{
			//Static
			entry.ipDhcp = DHCP_DISABLED;
			entry.dnsMode = 0;

			memcpy(entry.ipAddr, &info->ip_addr, sizeof(entry.ipAddr));
			memcpy(entry.netMask, &info->mask_addr, sizeof(entry.netMask));
			memcpy(entry.remoteIpAddr, &info->gateway_addr.ipv4_gateway_addr, sizeof(entry.remoteIpAddr));

			memcpy(entry.v4dns1, &info->primary_dns_addr.ipv4_primary_dns_addr, sizeof(entry.v4dns1));
			memcpy(entry.v4dns2, &info->second_dns_addr.ipv4_second_dns_addr, sizeof(entry.v4dns2));
		}
	}
	//TODO: IPv6

	if(idx >= 0)
		mib_chain_update(MIB_ATM_VC_TBL, &entry, idx);
	else
		mib_chain_add(MIB_ATM_VC_TBL, &entry);
	restartWAN(CONFIGONE, &entry);
}

static int omci_op_reset_acs()
{
	unsigned char configured = 0;

	mib_set(RS_OMCI_ACS_CONFIGURED, &configured);
}

static int omci_op_set_acs(acs_info_t *info)
{
	unsigned char configured = 1;

	DBG_LOG("%s is called.\n", __FUNCTION__);

	if(info == NULL)
		return -1;

	DBG_LOG("acs_url = %s\n", info->acs_url);
	mib_set(RS_OMCI_ACS_URL, info->acs_url);
	DBG_LOG("if_id = %u\n", info->related_if_id);
	mib_set(RS_OMCI_TR069_IF_ID, &info->related_if_id);
	mib_set(RS_OMCI_ACS_USERNAME, &info->username);
	mib_set(RS_OMCI_ACS_PASSWD, &info->password);
	mib_set(RS_OMCI_ACS_CONFIGURED, &configured);

	return 0;
}

static pthread_mutex_t set_lock = PTHREAD_MUTEX_INITIALIZER;
void *omci_cfg_set_thread(void *arg)
{
	mgmt_cfg_msg_t *pParam = arg;
	pthread_t tid = pthread_self();

	if(pParam == NULL)
		return NULL;

	DBG_LOG("tid %d starts, op = %d\n", tid, pParam->op_id);

	pthread_mutex_lock(&set_lock);

	switch(pParam->op_id)
	{
	case OP_RESET_ALL:
		omci_op_reset_if();
		omci_op_reset_acs();
		break;
	case OP_RESET_ACS:
		omci_op_reset_acs();
		break;
	case OP_SET_ACS:
		omci_op_set_acs(&pParam->cfg.acs);
		break;
	case OP_SET_IF:
		omci_op_set_if(&pParam->cfg.if_entry);
		break;
	default:
		fprintf(stderr, "[libtr142] Unknown command %d!\n", pParam->op_id);
		break;
	}

	DBG_LOG("tid %d unlock\n", tid);
	pthread_mutex_unlock(&set_lock);

	free(pParam);

	return NULL;
}

unsigned int omci_cfg_set(mgmt_cfg_msg_t *pParam, unsigned int len)
{
	pthread_t tid = 0, tid2 = 0;
	mgmt_cfg_msg_t *param = malloc(sizeof(mgmt_cfg_msg_t));

	if(pParam == NULL)
		return FAL_FAIL;

	DBG_LOG("%s is called\n", __FUNCTION__);

	memcpy(param, pParam, sizeof(mgmt_cfg_msg_t));

	// Use thread to do configurations to avoid OLT timeout
	pthread_create(&tid, NULL, omci_cfg_set_thread, param);
	pthread_detach(tid);

#if 0 // for mutex_lock test
	param = malloc(sizeof(mgmt_cfg_msg_t));
	memcpy(param, pParam, sizeof(mgmt_cfg_msg_t));
	pthread_create(&tid2, NULL, omci_cfg_set_thread, param);
	pthread_detach(tid2);
#endif

	return FAL_OK;
}

unsigned int omci_cfg_parser(va_list argp)
{
	mgmt_cfg_msg_t *pParam = va_arg(argp, mgmt_cfg_msg_t *);
	unsigned int len = va_arg(argp, unsigned int);
	va_end(argp);

	return omci_cfg_set(pParam, len);
}

unsigned int feature_module_init(unsigned int moduleID)
{
	if(feature_api_register(FEATURE_API_L3SVC_MGMT_CFG_SET, moduleID, &omci_cfg_parser) != FAL_OK)
	{
		fprintf(stderr, "[libtr142] Error: Register OMCI feature API failed!\n");
	}

    return FAL_OK;
}

