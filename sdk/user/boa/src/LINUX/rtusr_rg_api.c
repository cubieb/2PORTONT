#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <memory.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <linux/config.h>
#include <rtk_rg_struct.h>
#include "rtusr_rg_api.h"
#include "mib.h"
#include "utility.h"

#ifdef CONFIG_TR142_MODULE
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <rtk/rtk_tr142.h>
#endif

#include "chip_deps.h"

const char DHCPC_ROUTERFILE_B[] = "/var/udhcpc/router";
const char RG_LAN_INF_IDX[] = "/var/rg_lan_inf_idx";
const char RG_MAC_RULES_FILE[] = "/var/rg_mac_rules_idx";
const char RG_MAC_ACL_RULES_FILE[] = "/var/rg_mac_acl_rules_idx";
const char RG_ACL_RULES_FILE[] = "/var/rg_acl_rules_idx";
const char RG_ACL_DEFAULT_RULES_FILE[] = "/var/rg_acl_default_rules_idx";
const char RG_ACL_IPv6_RULES_FILE[] = "/var/rg_acl_ipv6_rules_idx";
const char RG_QOS_RULES_FILE[] = "/var/rg_acl_qos_idx";
const char RG_UPNP_CONNECTION_FILE[] = "/var/rg_upnp_idx";
const char RG_UPNP_TMP_FILE[] = "/var/rg_upnp_tmp";
const char RG_VERTUAL_SERVER_FILE[] = "/var/rg_vertual_servers_idx";
const char RG_URL_FILTER_FILE[] = "/var/rg_url_filter_idx";
const char MER_GWINFO_B[] = "/tmp/MERgw";
const char WAN_INTERFACE_TMP[] = "/var/wan_interface_tmp";
const char RG_GATEWAY_SERVICE_FILE[] = "/var/rg_gatewayService_idx";
const char RG_WIFI_INGRESS_RATE_LIMIT_FILE[] = "/proc/rg/wifi_ingress_rate_limit";

#define UntagCPort 1
#define TagCPort 0
#ifdef CONFIG_GPON_FEATURE
#define OMCI_WAN_INFO "/proc/omci/wanInfo"
/*
# cat /proc/omci/wanInfo
wanif[0]: vid=-1, pri=-1, type=-1, service=-1, netIfIdx=-1, isRuleCfg=0
--> omci add cf rule
type:(omci mode)
0 = PPPoE,
1 = IPoE,
2 = BRIDGE

service:
0 = other wan
1 = internet wan

isBinding:
0 = non binding
1 = binding

bAdd:
0 - delete
1 - add

what we write into proc:
netIfIdx=-1, vid=-1, pri=-1, type=-1, service=-1, isBinding=1,  is bAdd=0
*/
#endif
#define MAX_VALUE(val1, val2) (val1>val2?val1:val2)
#define MIN_VALUE(val1, val2) (val1<val2?val1:val2)
unsigned int RG_get_lan_phyPortMask(unsigned int portmask);
unsigned int RG_get_wan_phyPortMask();
void RTK_Setup_Storm_Control(void);

#ifdef CONFIG_IPV6
	unsigned char empty_ipv6[IPV6_ADDR_LEN] = {0};
#endif

int Init_rg_api()
{
	int ret;
	unsigned char mbtd;
	rtk_rg_initParams_t init_param;
	char buf[200]={0};

	bzero(&init_param, sizeof(rtk_rg_initParams_t));
	printf("init mac based tag des\n");

	mib_get(MIB_MAC_BASED_TAG_DECISION, (void *)&mbtd);
	init_param.macBasedTagDecision = mbtd;
#if 1
	//add for storm control
	sprintf(buf,"echo 1 > /proc/rg/layer2LookupMissFlood2CPU\n");
	system(buf);
#endif

#ifdef CONFIG_LUNA
#if defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE) || defined(CONFIG_FIBER_FEATURE)
	unsigned int pon_mode;

	if (mib_get(MIB_PON_MODE, (void *)&pon_mode) != 0)
	{
		if ( pon_mode == GPON_MODE )
		{
			init_param.wanPortGponMode = 1;
			printf("Init RG with GPON mode.\n");
		}
		else {
			init_param.wanPortGponMode = 0;
			printf("Init RG with non-GPON mode.\n");
		}
	}
#endif
#endif

#if defined(CONFIG_RTL_IGMP_SNOOPING)
	char igmp_mode;
	mib_get(MIB_MPMODE, (void *)&igmp_mode);
	if(igmp_mode & MP_IGMP_MASK)
		init_param.igmpSnoopingEnable = 1;
	else
		init_param.igmpSnoopingEnable = 0;

	if((ret = rtk_rg_initParam_set(&init_param)) != SUCCESS)
	{
		DBPRINT(1, "rtk_rg_initParam_set failed! ret=%d\n", ret);
		return -1;
	}
#else
	if((ret = rtk_rg_initParam_set(&init_param)) != SUCCESS)
	{
		DBPRINT(1, "rtk_rg_initParam_set failed! ret=%d\n", ret);
		return -1;
	}
#endif
	printf("=============Init_rg_api SUCESS!!==================\n");
	unlink(RG_LAN_INF_IDX);
	RTK_Setup_Storm_Control();
	return SUCCESS;
}
void dump_lan_info(rtk_rg_lanIntfConf_t *lan_info)
{
	printf("lan_info->ip_version=%d\n", lan_info->ip_version);
	printf("lan_info->gmac=%02X:%02X:%02X:%02X:%02X:%02X\n", lan_info->gmac.octet[0],lan_info->gmac.octet[1],lan_info->gmac.octet[2],lan_info->gmac.octet[3],lan_info->gmac.octet[4],lan_info->gmac.octet[5]);
	printf("lan_info->ip_addr=0x%08x\n", lan_info->ip_addr);
	printf("lan_info->ip_network_mask=%08x\n", lan_info->ip_network_mask);
	printf("lan_info->ipv6_network_mask_length=%08x\n", lan_info->ipv6_network_mask_length);
	printf("lan_info->port_mask.portmask=0x%08x\n", lan_info->port_mask.portmask);
	printf("lan_info->untag_mask.portmask=0x%08x\n", lan_info->untag_mask.portmask);
	printf("lan_info->intf_vlan_id=%d\n", lan_info->intf_vlan_id);
	printf("lan_info->vlan_based_pri=%d\n", lan_info->vlan_based_pri);
	printf("lan_info->vlan_based_pri_enable=%d\n", lan_info->vlan_based_pri_enable);
	printf("lan_info->mtu=%d\n", lan_info->mtu);
	printf("lan_info->isIVL=%d\n", lan_info->isIVL);
	printf("lan_info->replace_subnet=%d\n", lan_info->replace_subnet);
}
/*
    Due to RG maximum support 8 interface
    We must add error control handle to avoid
    unexpected error.
*/
#define MAX_INTF_NUM 7
int Check_RG_Intf_Count(void)
{
	int remained_intf_count=0;
	int cur_intf_count=0;
	rtk_rg_intfInfo_t *intf_info = NULL;
	int i=0,valid_idx=0;
	intf_info = (rtk_rg_intfInfo_t *)malloc(sizeof(rtk_rg_intfInfo_t));
	if(intf_info == NULL){
		printf("%s-%d Can't get enough memory space!\n",__func__,__LINE__);
		goto Error_remain;
	}
	for(i=0; i<MAX_INTF_NUM; i++){
		valid_idx = i;		
		if(rtk_rg_intfInfo_find(intf_info,&i)!=SUCCESS){
			//printf("%s-%d INTF:[%d] free to use!\n",__func__,__LINE__,i);
			remained_intf_count++;
		}else{
			//printf("%s-%d INTF:[%d] has already occupied\n",__func__,__LINE__,i);
			if(valid_idx != i)
				remained_intf_count++;				
			else			
				cur_intf_count++;
		}
	}
	Error_remain:
	fprintf(stderr, "%s-%d remained:%d, used:%d\n",__func__,__LINE__,remained_intf_count,cur_intf_count);
	return remained_intf_count;
}
int RG_reset_LAN(void)
{
	FILE *fp=NULL;
	int lanIdx=-1;
	int lanIntfIdx = -1;
	int ret = 0;
	unsigned char value[6], ip_version=IPVER_V4V6, vchar, ipv6_addr[IPV6_ADDR_LEN], ipv6_prefix_len;
	struct ipv6_ifaddr ip6_addr[6];
	char ipv6addr_str[64], cur_ip6addr_str[64];
	rtk_rg_intfInfo_t *intf_info = NULL;
	rtk_rg_lanIntfConf_t *lan_info = NULL;
	if(!(fp = fopen(RG_LAN_INF_IDX, "r"))){
		ret = -1;
		goto ErrorC;
	}
	intf_info = (rtk_rg_intfInfo_t *)malloc(sizeof(rtk_rg_intfInfo_t));
	if(intf_info == NULL){
		printf("%s-%d Can't get enough memory space!\n",__func__,__LINE__);
		ret = -1;
		goto ErrorC;
	}
	lan_info = (rtk_rg_lanIntfConf_t *)malloc(sizeof(rtk_rg_lanIntfConf_t));
	if(lan_info == NULL){
		printf("%s-%d Can't get enough memory space!\n",__func__,__LINE__);
		ret = -1;
		goto ErrorB;
	}

	memset(lan_info,0,sizeof(rtk_rg_lanIntfConf_t));
	memset(intf_info,0,sizeof(rtk_rg_intfInfo_t));

	while(fscanf(fp, "%d\n", &lanIdx) != EOF)
	{
		if(rtk_rg_intfInfo_find(intf_info,&lanIdx)!=SUCCESS){
			printf("%s-%d Can't find the lan interface idx:%d!",__func__,__LINE__,lanIdx);
			ret = -2;
			goto ErrorA;
		}
		memcpy(lan_info,&(intf_info->lan_intf),sizeof(rtk_rg_lanIntfConf_t));
		//clean lan ip info and force to zero, RG will flush lan routing info.
		//but keep lan intf index.
		lan_info->replace_subnet = 1;
		lan_info->ip_addr=0;
		if((rtk_rg_lanInterface_add(lan_info,&lanIntfIdx))!=SUCCESS)
		{
			DBPRINT(1, "Add LAN interface failed! lanIntfIdx=%d\n",lanIntfIdx);
			ret = -2;
			goto ErrorA;
		}
		if (mib_get(MIB_ADSL_LAN_IP, (void *)value) != 0)
		{
			lan_info->ip_addr=htonl((((struct in_addr *)value)->s_addr)); //192.168.1.1
		}
		if (mib_get(MIB_ADSL_LAN_SUBNET, (void *)value) != 0)
		{
			lan_info->ip_network_mask=htonl((((struct in_addr *)value)->s_addr)); //255.255.255.0
		}
#ifdef CONFIG_IPV6
		mib_get(MIB_LAN_IP_VERSION1, (void *)&ip_version);
		lan_info->ip_version=ip_version;
		if(ip_version == IPVER_V4V6 || ip_version == IPVER_V6ONLY)
		{
			mib_get(MIB_LAN_IPV6_MODE1, (void *)&vchar);
			if(vchar == 0) // IPv6 address mode is auto
			{
				getifip6(LANIF, IPV6_ADDR_UNICAST, ip6_addr, 6);
				memcpy(lan_info->ipv6_addr.ipv6_addr, &ip6_addr[0].addr, IPV6_ADDR_LEN);
				lan_info->ipv6_network_mask_length = ip6_addr[0].prefix_len;
			}
			else
			{
				mib_get(MIB_LAN_IPV6_ADDR1, (void *)ipv6_addr);
				mib_get(MIB_LAN_IPV6_PREFIX_LEN1, (void *)&ipv6_prefix_len);
				memcpy(lan_info->ipv6_addr.ipv6_addr, ipv6_addr, IPV6_ADDR_LEN);
				lan_info->ipv6_network_mask_length = ipv6_prefix_len;
				inet_ntop(PF_INET6, ipv6_addr, ipv6addr_str, sizeof(ipv6addr_str));
				sprintf(ipv6addr_str, "%s/%d", ipv6addr_str, ipv6_prefix_len);
				getifip6(LANIF, IPV6_ADDR_UNICAST, ip6_addr, 6);
				inet_ntop(PF_INET6, &ip6_addr[0].addr, cur_ip6addr_str, sizeof(cur_ip6addr_str));
				sprintf(cur_ip6addr_str, "%s/%d", cur_ip6addr_str, ip6_addr[0].prefix_len);
				va_cmd(IFCONFIG, 3, 1, LANIF, "del", cur_ip6addr_str);
				va_cmd(IFCONFIG, 3, 1, LANIF, "add", ipv6addr_str);
			}
		}
#endif
		lan_info->replace_subnet = 1;
		//dump_lan_info(lan_info);
		if((rtk_rg_lanInterface_add(lan_info,&lanIntfIdx))!=SUCCESS)
		{
			DBPRINT(1, "Add LAN interface failed! lanIntfIdx=%d\n",lanIntfIdx);
			ret = -2;
			goto ErrorA;
		}
#ifdef CONFIG_SECONDARY_IP
		mib_get(MIB_ADSL_LAN_ENABLE_IP2, (void *)value);
		if (value[0] == 1)
		{
			if (mib_get(MIB_ADSL_LAN_IP2, (void *)value) != 0)
			{
				lan_info->ip_addr=htonl((((struct in_addr *)value)->s_addr)); //192.168.1.1
			}
			if (mib_get(MIB_ADSL_LAN_SUBNET2, (void *)value) != 0)
			{
				lan_info->ip_network_mask=htonl((((struct in_addr *)value)->s_addr)); //255.255.255.0
			}
			lan_info->replace_subnet = 0;
			if((rtk_rg_lanInterface_add(lan_info,&lanIntfIdx))!=SUCCESS)
			{
				DBPRINT(1, "Add LAN interface 2 failed! lanIntfIdx=%d\n",lanIntfIdx);
				ret = -2;
				goto ErrorA;
			}
		}
#endif
	}
ErrorA:

	if(lan_info)
		free(lan_info);
ErrorB:
	if(intf_info)
		free(intf_info);

ErrorC:
	if(fp)
	fclose(fp);

	return ret;
}
#ifdef CONFIG_IPV6
int set_LAN_IPv6_IP()
{
	unsigned char value[6], ip_version=IPVER_V4V6, vchar, ipv6_addr[IPV6_ADDR_LEN], ipv6_prefix_len;
	struct ipv6_ifaddr ip6_addr[6];
	char ipv6addr_str[64], cur_ip6addr_str[64];

	mib_get(MIB_LAN_IP_VERSION1, (void *)&ip_version);
	if(ip_version == IPVER_V4V6 || ip_version == IPVER_V6ONLY)
	{
		mib_get(MIB_LAN_IPV6_MODE1, (void *)&vchar);
		if(vchar != 0) // IPv6 address mode is manual
		{
			mib_get(MIB_LAN_IPV6_ADDR1, (void *)ipv6_addr);
			mib_get(MIB_LAN_IPV6_PREFIX_LEN1, (void *)&ipv6_prefix_len);

			inet_ntop(PF_INET6, ipv6_addr, ipv6addr_str, sizeof(ipv6addr_str));
			sprintf(ipv6addr_str, "%s/%d", ipv6addr_str, ipv6_prefix_len);

			getifip6(LANIF, IPV6_ADDR_UNICAST, ip6_addr, 6);
			inet_ntop(PF_INET6, &ip6_addr[0].addr, cur_ip6addr_str, sizeof(cur_ip6addr_str));
			sprintf(cur_ip6addr_str, "%s/%d", cur_ip6addr_str, ip6_addr[0].prefix_len);

			va_cmd(IFCONFIG, 3, 1, LANIF, "del", cur_ip6addr_str);
			va_cmd(IFCONFIG, 3, 1, LANIF, "add", ipv6addr_str);
		}
	}
}
#endif

int Init_RG_ELan(int isUnTagCPort, int isRoutingWan)
{
	rtk_rg_lanIntfConf_t lan_info;
	int lanIntfIdx = -1;
#ifdef CONFIG_SECONDARY_IP
	int lanIntfIdx2 = -1;
#endif
	unsigned char value[6], ip_version=IPVER_V4V6, vchar, ipv6_addr[IPV6_ADDR_LEN], ipv6_prefix_len;
	int i;
	int wanPhyPort=0, vlan_id = 9;
	unsigned int portMask = 0;
	unsigned int phy_portmask;
	struct ipv6_ifaddr ip6_addr[6];
	char ipv6addr_str[64], cur_ip6addr_str[64];
	FILE *fp;
	MIB_CE_ATM_VC_T wan_entry;
	int untag_cpu_port = 0;
	int total_entry;

#if 0
	Init_rg_api();
	DBPRINT(2, "Init_rg_api() on!\n");
#else
	DBPRINT(0, "Init_rg_api() off!!\n");
	RG_Del_All_LAN_Interfaces();
#endif
	memset(&lan_info,0,sizeof(lan_info));

	mib_get(MIB_LAN_IP_VERSION1, (void *)&ip_version);
	lan_info.ip_version=ip_version;

#ifdef CONFIG_IPV6
	if(ip_version == IPVER_V4V6 || ip_version == IPVER_V6ONLY)
	{
		mib_get(MIB_LAN_IPV6_MODE1, (void *)&vchar);
		if(vchar == 0) // IPv6 address mode is auto
		{
			getifip6(LANIF, IPV6_ADDR_UNICAST, ip6_addr, 6);
			memcpy(lan_info.ipv6_addr.ipv6_addr, &ip6_addr[0].addr, IPV6_ADDR_LEN);
			lan_info.ipv6_network_mask_length = ip6_addr[0].prefix_len;
		}
		else
		{
			mib_get(MIB_LAN_IPV6_ADDR1, (void *)ipv6_addr);
			mib_get(MIB_LAN_IPV6_PREFIX_LEN1, (void *)&ipv6_prefix_len);
			memcpy(lan_info.ipv6_addr.ipv6_addr, ipv6_addr, IPV6_ADDR_LEN);
			lan_info.ipv6_network_mask_length = ipv6_prefix_len;

			inet_ntop(PF_INET6, ipv6_addr, ipv6addr_str, sizeof(ipv6addr_str));
			sprintf(ipv6addr_str, "%s/%d", ipv6addr_str, ipv6_prefix_len);

			getifip6(LANIF, IPV6_ADDR_UNICAST, ip6_addr, 6);
			inet_ntop(PF_INET6, &ip6_addr[0].addr, cur_ip6addr_str, sizeof(cur_ip6addr_str));
			sprintf(cur_ip6addr_str, "%s/%d", cur_ip6addr_str, ip6_addr[0].prefix_len);

			va_cmd(IFCONFIG, 3, 1, LANIF, "del", cur_ip6addr_str);
			va_cmd(IFCONFIG, 3, 1, LANIF, "add", ipv6addr_str);
		}
	}
#endif

	if(ip_version == IPVER_V4V6 || ip_version == IPVER_V4ONLY)
	{
		if (mib_get(MIB_ADSL_LAN_IP, (void *)value) != 0)
		{
			lan_info.ip_addr=htonl((((struct in_addr *)value)->s_addr)); //192.168.1.1
		}
		if (mib_get(MIB_ADSL_LAN_SUBNET, (void *)value) != 0)
		{
			lan_info.ip_network_mask=htonl((((struct in_addr *)value)->s_addr));
		}
	}

	if (mib_get(MIB_ELAN_MAC_ADDR, (void *)value) != 0)
	{
		for(i =0;i<6;i++)
			lan_info.gmac.octet[i]=value[i];
	}

#if 0
	if(mib_get(MIB_WAN_PHY_PORT , (void *)&wanPhyPort) == 0){
		printf("get MIB_WAN_PHY_PORT failed!!!\n");
		wanPhyPort=RTK_RG_MAC_PORT3 ; //for 0371 default
	}
#endif
#if 0
/* We only support 1 bridge WAN currently,
   so we do not let user configure LAN VID.
   We need to decide LAN vid here.
   Fix me if you have better idea.
*/
	total_entry = mib_chain_total(MIB_ATM_VC_TBL);

	for(i = 0 ; i < total_entry ; i++)
	{
		if(mib_chain_get(MIB_ATM_VC_TBL, i, &wan_entry) == 0)
			continue;

		if(wan_entry.cmode == CHANNEL_MODE_BRIDGE)
		{
			if(wan_entry.vlan)
			{
				untag_cpu_port = 0;
				vlan_id = wan_entry.vid;
			}
			else
			{
				untag_cpu_port = 1;
				vlan_id = 9;
			}
			break;
		}
	}
#endif
	mib_set(MIB_LAN_VLAN_ID1, (void *)&vlan_id);
// Fix me if you have better idea.
//	if(mib_get(MIB_LAN_VLAN_ID1, (void *)&vlan_id) != 0)
		lan_info.intf_vlan_id = vlan_id;
	lan_info.vlan_based_pri=-1;

	lan_info.mtu=1500;

	mib_get(MIB_LAN_PORT_MASK1, (void *)&portMask);
	//portMask = ((1<<RTK_RG_MAC_PORT0)|(1<<RTK_RG_MAC_PORT1)|(1<<RTK_RG_MAC_PORT2)|(1<<RTK_RG_MAC_PORT3));
	phy_portmask = RG_get_lan_phyPortMask(portMask);
	portMask = phy_portmask;
	if(isRoutingWan){
		portMask &= (~(RG_get_wan_phyPortMask()));
	}
	#if 0
	lan_info.port_mask.portmask=((1<<RTK_RG_PORT0)|(1<<RTK_RG_PORT1)|(1<<RTK_RG_PORT2)|(1<<RTK_RG_PORT3));
	lan_info.untag_mask.portmask=((1<<RTK_RG_MAC_PORT0)|(1<<RTK_RG_MAC_PORT1)|(1<<RTK_RG_MAC_PORT2));
	#endif
	#ifdef CONFIG_RTL9602C_SERIES
	lan_info.port_mask.portmask=portMask|(1<<RTK_RG_EXT_PORT0)|(1<<RTK_RG_PORT_CPU);
	lan_info.untag_mask.portmask = portMask;
	#else
	lan_info.port_mask.portmask=portMask|(1<<RTK_RG_EXT_PORT0)|(1<<RTK_RG_EXT_PORT1)|(1<<RTK_RG_EXT_PORT2)|(1<<RTK_RG_EXT_PORT3)|(1<<RTK_RG_EXT_PORT4)|(1<<RTK_RG_PORT_CPU);
	lan_info.untag_mask.portmask = portMask;
	#endif
	if(isUnTagCPort)
		lan_info.untag_mask.portmask|=(1<<RTK_RG_MAC_PORT_CPU);

	if((rtk_rg_lanInterface_add(&lan_info,&lanIntfIdx))!=SUCCESS)
	{
		DBPRINT(1, "Add LAN interface 1 failed!\n");
		return -1;
	}
	if(fp = fopen(RG_LAN_INF_IDX, "w"))
	{
		fprintf(fp, "%d\n", lanIntfIdx);
		DBPRINT(0, "LAN interface index=%d\n", lanIntfIdx);
		fclose(fp);
	}
	else
		fprintf(stderr, "Open %s failed! %s\n", RG_LAN_INF_IDX, strerror(errno));

#ifdef CONFIG_SECONDARY_IP

	mib_get(MIB_ADSL_LAN_ENABLE_IP2, (void *)value);

	if (value[0] == 1)
	{
		ip_version = IPVER_V4V6;
		mib_get(MIB_LAN_IP_VERSION2, (void *)&ip_version);
		lan_info.ip_version = ip_version;

		if(ip_version == IPVER_V4V6 || ip_version == IPVER_V4ONLY)
		{
			if (mib_get(MIB_ADSL_LAN_IP2, (void *)value) != 0)
				lan_info.ip_addr=htonl((((struct in_addr *)value)->s_addr));

			if (mib_get(MIB_ADSL_LAN_SUBNET2, (void *)value) != 0)
				lan_info.ip_network_mask=htonl((((struct in_addr *)value)->s_addr));

		}
/*
		if (mib_get(MIB_ELAN_MAC_ADDR, (void *)value) != 0)
		{
			for(i =0;i<6;i++)
				lan_info.gmac.octet[i]=value[i];
		}

		if(mib_get(MIB_LAN_VLAN_ID2, (void *)&vlan_id) != 0)
			lan_info.intf_vlan_id = vlan_id;


		if(mib_get(MIB_LAN_PORT_MASK2, (void *)&portMask)!=0)
		{
			phy_portmask = RG_get_lan_phyPortMask(portMask);
			portMask = phy_portmask;
			if(isRoutingWan)
				portMask &= (~(RG_get_wan_phyPortMask()));

			lan_info.port_mask.portmask=portMask|(1<<RTK_RG_MAC_PORT_CPU)|(1<<RTK_RG_EXT_PORT0)|(1<<RTK_RG_EXT_PORT1)|(1<<RTK_RG_EXT_PORT2)|(1<<RTK_RG_EXT_PORT3)|(1<<RTK_RG_EXT_PORT4);
			lan_info.untag_mask.portmask = portMask;

			if(untag_cpu_port)
				lan_info.untag_mask.portmask|=(1<<RTK_RG_MAC_PORT_CPU);
		}
*/
		lan_info.vlan_based_pri=-1;
		lan_info.mtu=1500;
/*
#ifdef CONFIG_IPV6
		if(ip_version == IPVER_V4V6 || ip_version == IPVER_V6ONLY)
		{
			mib_get(MIB_LAN_IPV6_MODE2, (void *)&vchar);
			if(vchar == 0) // IPv6 address mode is auto
			{
				getifip6(LAN_ALIAS, IPV6_ADDR_UNICAST, ip6_addr, 6);
				memcpy(lan_info.ipv6_addr.ipv6_addr, &ip6_addr[0].addr, IPV6_ADDR_LEN);
				lan_info.ipv6_network_mask_length = ip6_addr[0].prefix_len;
			}
			else
			{
				mib_get(MIB_LAN_IPV6_ADDR2, (void *)ipv6_addr);
				mib_get(MIB_LAN_IPV6_PREFIX_LEN2, (void *)&ipv6_prefix_len);
				memcpy(lan_info.ipv6_addr.ipv6_addr, ipv6_addr, IPV6_ADDR_LEN);
				lan_info.ipv6_network_mask_length = ipv6_prefix_len;

				inet_ntop(PF_INET6, ipv6_addr, ipv6addr_str, sizeof(ipv6addr_str));
				sprintf(ipv6addr_str, "%s/%d", ipv6addr_str, ipv6_prefix_len);

				getifip6(LAN_ALIAS, IPV6_ADDR_UNICAST, ip6_addr, 6);
				inet_ntop(PF_INET6, &ip6_addr[0].addr, cur_ip6addr_str, sizeof(cur_ip6addr_str));
				sprintf(cur_ip6addr_str, "%s/%d", cur_ip6addr_str, ip6_addr[0].prefix_len);

				va_cmd(IFCONFIG, 3, 1, LAN_ALIAS, "del", cur_ip6addr_str);
				va_cmd(IFCONFIG, 3, 1, LAN_ALIAS, "add", ipv6addr_str);
			}
		}
#endif
*/

		if((rtk_rg_lanInterface_add(&lan_info,&lanIntfIdx2))!=SUCCESS)
		{
			DBPRINT(1, "Add LAN interface 2 failed!\n");
			return -1;
		}
//		printf("%s-%d lanIntfIdx2=%d\n",__func__,__LINE__,lanIntfIdx2);
	}
#endif
#if 0
	if(fp = fopen(RG_LAN_INF_IDX, "w"))
	{
		fprintf(fp, "%d\n", lanIntfIdx);
		DBPRINT(0, "LAN interface index=%d\n", lanIntfIdx);
#ifdef CONFIG_SECONDARY_IP
		if(lanIntfIdx2 != -1)
		{
			fprintf(fp, "%d\n", lanIntfIdx2);
			DBPRINT(0, "LAN interface2 index=%d\n", lanIntfIdx2);
		}
#endif
		fclose(fp);
	}
	else
		fprintf(stderr, "Open %s failed! %s\n", RG_LAN_INF_IDX, strerror(errno));
#endif
	return SUCCESS;
}

/*Vlan and port binding will effect the OMCI CF rules
    so, we must check rules after setting vlan port mapping
*/
#if defined(CONFIG_RTK_RG_INIT) && defined(CONFIG_GPON_FEATURE)

int RTK_RG_Sync_OMCI_WAN_INFO(void)
{
	MIB_CE_ATM_VC_T entryVC;
	int totalVC_entry,i,wan_idx=-1,wanIntfIdx=-1;
	int omci_service=-1;
	int omci_mode=-1;
	int omci_bind=-1;
	char cmdStr[64];
	rtk_rg_intfInfo_t *intf_info = NULL;
	rtk_rg_wanIntfConf_t *wan_info_p = NULL;
	char vlan_based_pri=-1;
	int pon_mode;

	mib_get(MIB_PON_MODE, (void *)&pon_mode);
	if(pon_mode != GPON_MODE)
		return -2;

	intf_info = (rtk_rg_intfInfo_t *)malloc(sizeof(rtk_rg_intfInfo_t));
	if(intf_info == NULL){
		printf("%s-%d Can't get enough memory space!\n",__func__,__LINE__);
		return -1;
	}
	memset(intf_info,0,sizeof(rtk_rg_intfInfo_t));
	totalVC_entry = mib_chain_total(MIB_ATM_VC_TBL);
	for(i=0;i<totalVC_entry;i++){
		if(mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&entryVC) == 0)
			continue;
		if(rtk_rg_intfInfo_find(intf_info,&entryVC.rg_wan_idx)!=SUCCESS){
			printf("%s-%d Can't find the wan interface idx:%d!",__func__,__LINE__,entryVC.rg_wan_idx);
			free(intf_info);;
			return -1;
		}
		wan_info_p = &(intf_info->wan_intf.wan_intf_conf);
		if((wan_info_p->port_binding_mask.portmask == 0) && (wan_info_p->vlan_binding_mask.portmask == 0) && (wan_info_p->wlan0_dev_binding_mask == 0))
		{
			if(entryVC.itfGroup > 0)
				continue;
			//none binding wan, reset omci wan info...
			//omci wan info can't write duplicate, must delete it before adding.
			snprintf(cmdStr, sizeof(cmdStr),"echo %d %d %d %d %d %d %d > %s",entryVC.rg_wan_idx,0,0,0,0,0,0,OMCI_WAN_INFO);
			system(cmdStr);
			if(wan_info_p->none_internet)
				omci_service = 0;
			else
				omci_service = 1;

			switch(entryVC.cmode){
				case CHANNEL_MODE_IPOE:
					if( (entryVC.IpProtocol == IPVER_IPV4_IPV6) && entryVC.napt ==1)	
						omci_mode = OMCI_MODE_IPOE_V4NAPT_V6;
					else
						omci_mode = OMCI_MODE_IPOE;
					break;
				case CHANNEL_MODE_PPPOE:
					if( (entryVC.IpProtocol == IPVER_IPV4_IPV6) && entryVC.napt ==1)	
						omci_mode = OMCI_MODE_PPPOE_V4NAPT_V6;
					else
						omci_mode = OMCI_MODE_PPPOE;
					break;
				case CHANNEL_MODE_BRIDGE:
					omci_mode = OMCI_MODE_BRIDGE;
					break;
				default:
					printf("unknow mode %d\n",omci_mode);
					break;
			}
			omci_bind = 0;
			//sync omci cf rules.
			/*untag wan, omci egress vlan id = -1*/
			if(entryVC.vlan == 2)
				wan_info_p->egress_vlan_id = 4095;
			else{
				if(!wan_info_p->egress_vlan_tag_on)
					wan_info_p->egress_vlan_id = -1;
			}
			if(entryVC.vprio)
			{
				vlan_based_pri=(entryVC.vprio)-1;
			}
			else
			{
				vlan_based_pri=-1;
			}			
			snprintf(cmdStr, sizeof(cmdStr),"echo %d %d %d %d %d %d %d > %s",entryVC.rg_wan_idx,wan_info_p->egress_vlan_id,vlan_based_pri,omci_mode,omci_service,omci_bind,1,OMCI_WAN_INFO);
			system(cmdStr);
		}
	}
	free(intf_info);
	return 0;
}
#endif
static inline int RG_get_wan_type(MIB_CE_ATM_VC_Tp entry)
{
	if(entry == NULL)
		return -1;

	switch(entry->cmode)
	{
	case CHANNEL_MODE_BRIDGE:
		return RTK_RG_BRIDGE;
	case CHANNEL_MODE_IPOE:
		if(entry->ipDhcp == DHCP_CLIENT)
			return RTK_RG_DHCP;
		else
			return RTK_RG_STATIC;
	case CHANNEL_MODE_PPPOE:
		return RTK_RG_PPPoE;
	default:
		return -1;
	}
}
void dump_wan_info(rtk_rg_wanIntfConf_t *wan_info)
{
	printf("wan_info->wan_type=%d\n", wan_info->wan_type);
	printf("wan_info->gmac=%02X:%02X:%02X:%02X:%02X:%02X\n", wan_info->gmac.octet[0],wan_info->gmac.octet[1],wan_info->gmac.octet[2],wan_info->gmac.octet[3],wan_info->gmac.octet[4],wan_info->gmac.octet[5]);
	printf("wan_info->wan_port_idx=%d\n", wan_info->wan_port_idx);
	printf("wan_info->port_binding_mask=%08x\n", wan_info->port_binding_mask);
	printf("wan_info->vlan_binding_mask=%08x\n", wan_info->vlan_binding_mask);
	printf("wan_info->egress_vlan_tag_on=%d\n", wan_info->egress_vlan_tag_on);
	printf("wan_info->egress_vlan_id=%d\n", wan_info->egress_vlan_id);
	printf("wan_info->vlan_based_pri_enable=%d\n", wan_info->vlan_based_pri_enable);
	printf("wan_info->vlan_based_pri=%d\n", wan_info->vlan_based_pri);
	printf("wan_info->isIVL=%d\n", wan_info->isIVL);
	printf("wan_info->none_internet=%d\n", wan_info->none_internet);
	printf("wan_info->forcedAddNewIntf=%d\n", wan_info->forcedAddNewIntf);
	printf("wan_info->wlan0_dev_binding_mask=%08x\n", wan_info->wlan0_dev_binding_mask);
}
void dump_ipStaticInfo(rtk_rg_ipStaticInfo_t *staticInfo)
{
	printf("staticInfo->ipv4_default_gateway_on=%d\n", staticInfo->ipv4_default_gateway_on);
	printf("staticInfo->gw_mac_auto_learn_for_ipv4=%d\n", staticInfo->gw_mac_auto_learn_for_ipv4);
	printf("staticInfo->ip_addr=%08x\n", staticInfo->ip_addr);
	printf("staticInfo->ip_network_mask=%08x\n", staticInfo->ip_network_mask);
	printf("staticInfo->gateway_ipv4_addr=%08x\n", staticInfo->gateway_ipv4_addr);
	printf("staticInfo->mtu=%08x\n", staticInfo->mtu);
	printf("staticInfo->napt_enable=%d\n", staticInfo->napt_enable);
}
void dump_ipPppoeClientInfoA(rtk_rg_pppoeClientInfoAfterDial_t *pppoeInfoA)
{
	unsigned char gw_mac[6];
	printf("pppoeInfoA->hw_info.napt_enable=%d\n", pppoeInfoA->hw_info.napt_enable);
	printf("pppoeInfoA->hw_info.ip_addr=0x%08x\n", pppoeInfoA->hw_info.ip_addr);
	printf("pppoeInfoA->hw_info.ip_network_mask=0x%08x\n", pppoeInfoA->hw_info.ip_network_mask);
	printf("pppoeInfoA->hw_info.ipv4_default_gateway_on=0x%08x\n", pppoeInfoA->hw_info.ipv4_default_gateway_on);
	printf("pppoeInfoA->hw_info.gateway_ipv4_addr=0x%08x\n", pppoeInfoA->hw_info.gateway_ipv4_addr);
	printf("pppoeInfoA->hw_info.mtu=0x%08x\n", pppoeInfoA->hw_info.mtu);
	printf("pppoeInfoA->hw_info.gw_mac_auto_learn_for_ipv4=%d\n", pppoeInfoA->hw_info.gw_mac_auto_learn_for_ipv4);
	memcpy(gw_mac, pppoeInfoA->hw_info.gateway_mac_addr_for_ipv4.octet, 6);
	printf("pppoeInfoA->hw_info.gw_mac=>%02x:%02x:%02x:%02x:%02x:%02x\n", gw_mac[0],gw_mac[1],gw_mac[2],gw_mac[3],gw_mac[4],gw_mac[5]);
}
void dump_ipDhcpClientInfo(rtk_rg_ipDhcpClientInfo_t *dhcpClient_info)
{
	printf("dhcpClient_info->stauts=%d\n", dhcpClient_info->stauts);
	printf("dhcpClient_info->hw_info.ipv4_default_gateway_on=%d\n", dhcpClient_info->hw_info.ipv4_default_gateway_on);
	printf("dhcpClient_info->hw_info.gw_mac_auto_learn_for_ipv4=%d\n", dhcpClient_info->hw_info.gw_mac_auto_learn_for_ipv4);
	printf("dhcpClient_info->hw_info.ip_addr=0x%08x\n", dhcpClient_info->hw_info.ip_addr);
	printf("dhcpClient_info->hw_info.ip_network_mask=0x%08x\n",dhcpClient_info->hw_info.ip_network_mask);
	printf("dhcpClient_info->hw_info.mtu=0x%08x\n", dhcpClient_info->hw_info.mtu);
	printf("dhcpClient_info->hw_info.napt_enable=%d\n", dhcpClient_info->hw_info.napt_enable);
	printf("dhcpClient_info->hw_info.gateway_ipv4_addr=0x%08x\n", dhcpClient_info->hw_info.gateway_ipv4_addr);
}
int RG_del_static_route(MIB_CE_IP_ROUTE_T *entry)
{
	int ret=0;

	if(entry->rg_wan_idx > 0){
		ret = RG_WAN_Interface_Del(entry->rg_wan_idx);
		entry->rg_wan_idx = -1;
	}
	return ret;
}
#ifdef ROUTING
int RG_add_static_route_PPP(MIB_CE_IP_ROUTE_T *entry,MIB_CE_ATM_VC_T *vc_entry,int entryID)
{
	rtk_rg_wanIntfConf_t *wan_info = NULL;
	rtk_rg_intfInfo_t *intf_info = NULL;
	rtk_rg_pppoeClientInfoAfterDial_t *pppoeClientInfoA=NULL;
	int wan_idx=-1, wanIntfIdx=-1, ret=-1;
	int omci_service=-1;
	int omci_bind=-1;
	char cmdStr[64];
#if defined(CONFIG_RTK_RG_INIT) && defined(CONFIG_GPON_FEATURE)
	int pon_mode=0;
	mib_get(MIB_PON_MODE, (void *)&pon_mode);
#endif
	intf_info = (rtk_rg_intfInfo_t *)malloc(sizeof(rtk_rg_intfInfo_t));
	if(intf_info == NULL){
		printf("%s-%d Can't get enough memory space!\n",__func__,__LINE__);
		ret = -1;
		goto E_PPPOE_4;
	}
	memset(intf_info,0,sizeof(rtk_rg_intfInfo_t));
	wan_info = (rtk_rg_wanIntfConf_t *)malloc(sizeof(rtk_rg_wanIntfConf_t));
	if(wan_info == NULL){
		printf("%s-%d Can't get enough memory space!\n",__func__,__LINE__);
		ret = -1;
		goto E_PPPOE_3;
	}
	memset(wan_info,0,sizeof(rtk_rg_wanIntfConf_t));
	wan_idx = vc_entry->rg_wan_idx;
	if(rtk_rg_intfInfo_find(intf_info,&wan_idx)!=SUCCESS){
		printf("%s-%d Can't find the wan interface idx:%d!",__func__,__LINE__,wan_idx);
		ret = -1;
		goto E_PPPOE_2;
	}
	memcpy(wan_info,&(intf_info->wan_intf.wan_intf_conf),sizeof(rtk_rg_wanIntfConf_t));
	wan_info->forcedAddNewIntf = 1;
	if((ret = rtk_rg_wanInterface_add(wan_info, &wanIntfIdx))!=SUCCESS){
		printf("%s-%d rtk_rg_wanInterface_add fail! ret=%d\n",__func__,__LINE__,ret);
		ret = -1;
		goto E_PPPOE_2;
	}
#if defined(CONFIG_RTK_RG_INIT) && defined(CONFIG_GPON_FEATURE)
	if(pon_mode == GPON_MODE){
		char vlan_based_pri=-1;
		/*untag wan, omci egress vlan id = -1*/
		if(vc_entry->vlan == 2)
			wan_info->egress_vlan_id = 4095;
		else{
			if(!wan_info->egress_vlan_tag_on)
				wan_info->egress_vlan_id = -1;
		}
		if(wan_info->none_internet)
			omci_service = 0;
		else
			omci_service = 1;
		if((wan_info->port_binding_mask.portmask > 0) || (wan_info->wlan0_dev_binding_mask > 0))
			omci_bind = 1;
		else
			omci_bind = 0;
		if(vc_entry->vprio)
		{
			vlan_based_pri=(vc_entry->vprio)-1;
		}
		else
		{
			vlan_based_pri=-1;
		}		
		snprintf(cmdStr, sizeof(cmdStr),"echo %d %d %d %d %d %d %d > %s",wanIntfIdx,wan_info->egress_vlan_id,vlan_based_pri,0,omci_service,omci_bind,1,OMCI_WAN_INFO);
		system(cmdStr);
	}
#endif
	entry->rg_wan_idx = wanIntfIdx;
	pppoeClientInfoA = (rtk_rg_pppoeClientInfoAfterDial_t *)malloc(sizeof(rtk_rg_pppoeClientInfoAfterDial_t));
	if(pppoeClientInfoA == NULL){
		printf("%s-%d Can't get enough memory space!\n",__func__,__LINE__);
		ret = -1;
		goto E_PPPOE_2;
	}
	memset(pppoeClientInfoA,0,sizeof(rtk_rg_pppoeClientInfoAfterDial_t));
	memcpy(pppoeClientInfoA,&(intf_info->wan_intf.pppoe_info.after_dial),sizeof(rtk_rg_pppoeClientInfoAfterDial_t));
	pppoeClientInfoA->hw_info.gateway_ipv4_addr = pppoeClientInfoA->hw_info.ip_addr;
	pppoeClientInfoA->hw_info.ip_addr = (((struct in_addr *)entry->destID)->s_addr);
	pppoeClientInfoA->hw_info.ip_network_mask =(((struct in_addr *)entry->netMask)->s_addr);
	pppoeClientInfoA->hw_info.ipv4_default_gateway_on =0;
	dump_ipPppoeClientInfoA(pppoeClientInfoA);
	if((ret = rtk_rg_pppoeClientInfoAfterDial_set(entry->rg_wan_idx, pppoeClientInfoA))!=SUCCESS){
		printf("%s-%d add rtk_rg_pppoeClientInfoAfterDial_set fail! ret=%d\n",__func__,__LINE__,ret);
		ret = -1;
	}
	mib_chain_update(MIB_IP_ROUTE_TBL, entry, entryID);
E_PPPOE_1:
	if(pppoeClientInfoA)
		free(pppoeClientInfoA);
E_PPPOE_2:
	if(wan_info)
		free(wan_info);
E_PPPOE_3:
	if(intf_info)
		free(intf_info);
E_PPPOE_4:
	return ret;
}
int RG_add_static_route(MIB_CE_IP_ROUTE_T *entry, char *mac_str, int entryID)
{
	rtk_rg_wanIntfConf_t *wan_info = NULL;
	rtk_rg_intfInfo_t *intf_info = NULL;
	rtk_rg_ipStaticInfo_t *staticInfo = NULL;
	rtk_rg_ipDhcpClientInfo_t *dhcpClient_info=NULL;
	int ret=0;
	char cmdStr[64];
	MIB_CE_ATM_VC_T entryVC;
	int totalVC_entry,i,wan_idx=-1,wanIntfIdx=-1;
#if defined(CONFIG_RTK_RG_INIT) && defined(CONFIG_GPON_FEATURE)
	int omci_service=-1;
	int omci_bind=-1;
	int pon_mode=0;
	mib_get(MIB_PON_MODE, (void *)&pon_mode);
#endif
	totalVC_entry = mib_chain_total(MIB_ATM_VC_TBL);
	for(i=0;i<totalVC_entry;i++){
		if(mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&entryVC) == 0)
			continue;
		if(memcmp(entryVC.MacAddr, mac_str, MAC_ADDR_LEN)==0){
			wan_idx = entryVC.rg_wan_idx;
			break;
		}
	}
	if(wan_idx == -1){
		printf("can't find the respected RG WAN IDX!");
		ret = -1;
		goto Error4;
	}
	intf_info = (rtk_rg_intfInfo_t *)malloc(sizeof(rtk_rg_intfInfo_t));
	if(intf_info == NULL){
		printf("%s-%d Can't get enough memory space!\n",__func__,__LINE__);
		ret = -1;
		goto Error4;
	}
	memset(intf_info,0,sizeof(rtk_rg_intfInfo_t));
	wan_info = (rtk_rg_wanIntfConf_t *)malloc(sizeof(rtk_rg_wanIntfConf_t));
	if(wan_info == NULL){
		printf("%s-%d Can't get enough memory space!\n",__func__,__LINE__);
		ret = -1;
		goto Error3;
	}
	memset(wan_info,0,sizeof(rtk_rg_wanIntfConf_t));
	if(rtk_rg_intfInfo_find(intf_info,&wan_idx)!=SUCCESS){
		printf("%s-%d Can't find the wan interface idx:%d!",__func__,__LINE__,wan_idx);
		ret = -1;
		goto Error2;
	}
	memcpy(wan_info,&(intf_info->wan_intf.wan_intf_conf),sizeof(rtk_rg_wanIntfConf_t));
	/*force to add new WAN interface*/
	wan_info->forcedAddNewIntf = 1;

   	if((ret = rtk_rg_wanInterface_add(wan_info, &wanIntfIdx))!=SUCCESS){
		printf("%s-%d rtk_rg_wanInterface_add fail! ret=%d\n",__func__,__LINE__,ret);
		ret = -1;
		goto Error2;
	}
	//printf("%s-%d static wan wanIntfIdx:%d\n",__func__,__LINE__,wanIntfIdx);
	entry->rg_wan_idx = wanIntfIdx;
#if defined(CONFIG_RTK_RG_INIT) && defined(CONFIG_GPON_FEATURE)
	if(pon_mode == GPON_MODE){
		char vlan_based_pri=-1;
		/*untag wan, omci egress vlan id = -1*/
		if(entryVC.vlan == 2)
			wan_info->egress_vlan_id = 4095;
		else{
			if(!wan_info->egress_vlan_tag_on)
				wan_info->egress_vlan_id = -1;
		}
		if(wan_info->none_internet)
			omci_service = 0;
		else
			omci_service = 1;
		if((wan_info->port_binding_mask.portmask > 0) || (wan_info->wlan0_dev_binding_mask > 0))
			omci_bind = 1;
		else
			omci_bind = 0;
		if(entryVC.vprio)
		{
			vlan_based_pri=(entryVC.vprio)-1;
		}
		else
		{
			vlan_based_pri=-1;
		}			
		snprintf(cmdStr, sizeof(cmdStr),"echo %d %d %d %d %d %d %d > %s",wanIntfIdx,wan_info->egress_vlan_id,vlan_based_pri,1,omci_service,omci_bind,1,OMCI_WAN_INFO);
		system(cmdStr);

	}
#endif
	switch(entryVC.cmode)
	{
		case CHANNEL_MODE_IPOE:
			if(entryVC.ipDhcp==DHCP_CLIENT){
				dhcpClient_info = (rtk_rg_ipDhcpClientInfo_t *)malloc(sizeof(rtk_rg_ipDhcpClientInfo_t));
				if(dhcpClient_info == NULL){
					printf("%s-%d Can't get enough memory space!\n",__func__,__LINE__);
					ret = -1;
					goto Error2;
				}
				memset(dhcpClient_info,0,sizeof(rtk_rg_ipDhcpClientInfo_t));
				memcpy(dhcpClient_info,&(intf_info->wan_intf.dhcp_client_info),sizeof(rtk_rg_ipDhcpClientInfo_t));
				dhcpClient_info->hw_info.ipv4_default_gateway_on=0;
				dhcpClient_info->hw_info.gw_mac_auto_learn_for_ipv4=1;
				dhcpClient_info->hw_info.ip_addr=(((struct in_addr *)entry->destID)->s_addr);;
				dhcpClient_info->hw_info.ip_network_mask=(((struct in_addr *)entry->netMask)->s_addr);
				dhcpClient_info->hw_info.gateway_ipv4_addr = (((struct in_addr *)entry->nextHop)->s_addr);
				if(rtk_rg_dhcpClientInfo_set(entry->rg_wan_idx, dhcpClient_info)!=SUCCESS)
				{
					printf("rtk_rg_dhcpClientInfo_set error!!!\n");
					return -1;
				}
			}else{
	staticInfo = (rtk_rg_ipStaticInfo_t *)malloc(sizeof(rtk_rg_ipStaticInfo_t));
	if(staticInfo == NULL){
		printf("%s-%d Can't get enough memory space!\n",__func__,__LINE__);
		ret = -1;
		goto Error2;
	}
	memset(staticInfo,0,sizeof(rtk_rg_ipStaticInfo_t));
	staticInfo->ipv4_default_gateway_on=0;
	staticInfo->gw_mac_auto_learn_for_ipv4=1;
	staticInfo->ip_addr=(((struct in_addr *)entry->destID)->s_addr);
	staticInfo->ip_network_mask=(((struct in_addr *)entry->netMask)->s_addr);
	staticInfo->gateway_ipv4_addr = (((struct in_addr *)entry->nextHop)->s_addr);
	staticInfo->mtu=entryVC.mtu;
	if(entryVC.napt==1){
		staticInfo->napt_enable=1;
	}
	else{
		staticInfo->napt_enable=0;
	}
	if((ret = rtk_rg_staticInfo_set(entry->rg_wan_idx, staticInfo))!=SUCCESS){
		printf("%s-%d add rtk_rg_staticInfo_set fail! ret=%d\n",__func__,__LINE__,ret);
		ret = -1;
	}
			}
			break;
		default:
			printf("%s-%d entryVC.cmode=%d set static route error\n",__func__,__LINE__,entryVC.cmode);
			ret = -1;
			goto Error2;
	}
	mib_chain_update(MIB_IP_ROUTE_TBL, entry, entryID);
Error1:
	if(staticInfo)
		free(staticInfo);
	if(dhcpClient_info)
		free(dhcpClient_info);
Error2:
	if(wan_info)
		free(wan_info);
Error3:
	if(intf_info)
		free(intf_info);
Error4:
	return ret;
}
#endif

int RG_add_wan(MIB_CE_ATM_VC_Tp entry, int mib_vc_idx)
{
	int wanIntfIdx;
	int vcTotal, i, vlan_id;
	rtk_rg_wanIntfConf_t wan_info;
	unsigned char value[6];
	int ret=-1;
	int wanPhyPort=0;
	struct in_addr gw_addr;
    char cmdStr[64];
	int omci_mode=-1;
	//Init_RG_ELan(UntagCPort, RoutingWan);
	int rtk_rg_wan_type = RG_get_wan_type(entry);
	unsigned char mbtd;
#ifdef CONFIG_RTL_CLIENT_MODE_SUPPORT
	struct sockaddr hwaddr;
	char intf_name[IFNAMSIZ];
#endif
	int omci_service=-1;
	int omci_bind=-1;
	int pon_mode=0;
	if(rtk_rg_wan_type == -1)
		return -1;

	memset(&wan_info,0,sizeof(wan_info));
	memcpy(wan_info.gmac.octet, entry->MacAddr, MAC_ADDR_LEN);

#if 0
	if(mib_get(MIB_WAN_PHY_PORT , (void *)&wanPhyPort) == 0){
		printf("get MIB_WAN_PHY_PORT failed!!!\n");
		wanPhyPort=RTK_RG_MAC_PORT3 ; //for 0371 default
	}
#endif
#ifdef CONFIG_RTL_CLIENT_MODE_SUPPORT
	if(MEDIA_INDEX(entry->ifIndex)==MEDIA_WLAN)
	{
		int tmp_wlan_idx;
		tmp_wlan_idx=ETH_INDEX(entry->ifIndex);
		if(tmp_wlan_idx)
			wanPhyPort=RTK_RG_EXT_PORT3;	//wlan1-vxd
		else
			wanPhyPort=RTK_RG_EXT_PORT2;	//wlan0-vxd

		snprintf(intf_name, IFNAMSIZ, "wlan%d-vxd", tmp_wlan_idx);
		getInAddr(intf_name, HW_ADDR, (void *)&hwaddr);
		memcpy(wan_info.gmac.octet, hwaddr.sa_data, MAC_ADDR_LEN);
	}
	else
#endif
		wanPhyPort=RG_get_wan_phyPortId();

	//wan_info.egress_vlan_id=8;
	//wan_info.vlan_based_pri=0;
	//wan_info.egress_vlan_tag_on=0;
	if (entry->vlan) {
		wan_info.egress_vlan_tag_on=1;
		wan_info.egress_vlan_id=entry->vid;
	}
	else{
		wan_info.egress_vlan_tag_on=0;
		wan_info.egress_vlan_id=8;

		if(rtk_rg_wan_type == RTK_RG_BRIDGE)
		{
			mib_get(MIB_LAN_VLAN_ID1, (void *)&vlan_id);
			wan_info.egress_vlan_id = vlan_id;
		}
	}

	//Only mac based decision supports port binding
	mib_get(MIB_MAC_BASED_TAG_DECISION, (void *)&mbtd);
	if(mbtd)
		#ifdef CONFIG_RTL9602C_SERIES
		wan_info.port_binding_mask.portmask = RG_get_lan_phyPortMask(entry->itfGroup & 0x3);
		#else
		wan_info.port_binding_mask.portmask = RG_get_lan_phyPortMask(entry->itfGroup & 0xf);
		#endif
	wan_info.wlan0_dev_binding_mask = ((entry->itfGroup & 0x1f0) >> 4);
#if defined(CONFIG_RG_WLAN_HWNAT_ACCELERATION) && !defined(CONFIG_ARCH_LUNA_SLAVE)
#ifdef CONFIG_RTL_CLIENT_MODE_SUPPORT
	wan_info.wlan0_dev_binding_mask |= ((entry->itfGroup & 0x3e00) << 5);
#else
	wan_info.wlan0_dev_binding_mask |= ((entry->itfGroup & 0x3e00) << 4);
#endif
#endif

	if(entry->itfGroup > 0)
		omci_bind = 1;
	else
		omci_bind = 0;
/*
	wan_info.port_binding_mask.portmask = (entry->itfGroup & (1 << PMAP_ETH0_SW0)? 1 << RTK_RG_PORT0: 0) |
		(entry->itfGroup & (1 << PMAP_ETH0_SW1)? 1 << RTK_RG_PORT1: 0) |
		(entry->itfGroup & (1 << PMAP_ETH0_SW2)? 1 << RTK_RG_PORT2: 0) |
		(entry->itfGroup & (1 << PMAP_ETH0_SW3)? 1 << RTK_RG_PORT3: 0) |
		(entry->itfGroup & (1 << PMAP_WLAN0)? 1 << RTK_RG_EXT_PORT0: 0) |
		(entry->itfGroup & (1 << PMAP_WLAN0+1)? 1 << RTK_RG_EXT_PORT1: 0);
*/

	wan_info.wan_port_idx=wanPhyPort;
	wan_info.wan_type=rtk_rg_wan_type;

	/*RG: Internet = 0, other=1*/
	if(entry->applicationtype & X_CT_SRV_INTERNET){
		omci_service = 1;
		wan_info.none_internet = 0;
	}
	else{
		wan_info.none_internet = 1;
		omci_service = 0;
	}
	if((rtk_rg_wanInterface_add(&wan_info, &wanIntfIdx))!=SUCCESS)
		return -1;
#if defined(CONFIG_RTK_RG_INIT) && defined(CONFIG_GPON_FEATURE)
//0 = PPPoE, 1 = IPoE, 2 = BRIDGE --> omci add cf rule

	switch(entry->cmode){
		case CHANNEL_MODE_IPOE:
			if( (entry->IpProtocol == IPVER_IPV4_IPV6) && entry->napt ==1)	
				omci_mode = OMCI_MODE_IPOE_V4NAPT_V6;
			else
				omci_mode = OMCI_MODE_IPOE;
			break;
		case CHANNEL_MODE_PPPOE:
			if( (entry->IpProtocol == IPVER_IPV4_IPV6) && entry->napt ==1)	
				omci_mode = OMCI_MODE_PPPOE_V4NAPT_V6;
			else
				omci_mode = OMCI_MODE_PPPOE;
			break;
		case CHANNEL_MODE_BRIDGE:
			omci_mode = OMCI_MODE_BRIDGE;
			break;
		default:
			printf("unknow mode %d\n",omci_mode);
			break;
	}

	//sync omci cf rules.
	mib_get(MIB_PON_MODE, (void *)&pon_mode);
	if(pon_mode == GPON_MODE){
		char vlan_based_pri;
		/*untag wan, omci egress vlan id = -1*/
		if(entry->vlan == 2)
			wan_info.egress_vlan_id = 4095;
		else{
			if(!wan_info.egress_vlan_tag_on)
				wan_info.egress_vlan_id = -1;
		}
		if(entry->vprio)
		{
			vlan_based_pri=(entry->vprio)-1;
		}
		else
		{
			vlan_based_pri=-1;
		}
		snprintf(cmdStr, sizeof(cmdStr),"echo %d %d %d %d %d %d %d > %s",wanIntfIdx,wan_info.egress_vlan_id,vlan_based_pri,omci_mode,omci_service,omci_bind,1,OMCI_WAN_INFO);
		//AUG_PRT("%s\n",cmdStr);
		system(cmdStr);

	}
#endif
	entry->rg_wan_idx = wanIntfIdx;
	mib_chain_update(MIB_ATM_VC_TBL, entry, mib_vc_idx);
	return SUCCESS;
}
int RG_del_All_Acl_Rules(void)
{
	RG_del_All_default_Acl();
}
int RG_del_All_default_Acl(void)
{
	FILE *fp;
	int aclIdx=-1;
	if(!(fp = fopen(RG_ACL_DEFAULT_RULES_FILE, "r"))){
		return -2;
	}
	while(fscanf(fp, "%d\n", &aclIdx) != EOF)
	{
		if(rtk_rg_aclFilterAndQos_del(aclIdx))
			DBPRINT(1, "rtk_rg_aclFilterAndQos_del failed! idx = %d\n", aclIdx);
	}
	fclose(fp);
	unlink(RG_ACL_DEFAULT_RULES_FILE);
	return 0;
}
/*Due to SOC bug, ACL 1p priority is greater than OMCI CF 1p priority
    per Wan 1p priority would have problem.
    So, if configure GPON we disable per wan 1p ACL rules.
*/
int RG_add_default_Acl_Qos(void)
{
		MIB_CE_ATM_VC_T entry;
		int totalEntry = mib_chain_total(MIB_ATM_VC_TBL);
		int i,aclIdx=0, ret;
		rtk_rg_aclFilterAndQos_t aclRule;
		FILE *fp;
		if(!(fp = fopen(RG_ACL_DEFAULT_RULES_FILE, "a")))
		{
			fprintf(stderr, "ERROR! %s\n", strerror(errno));
			return -2;
		}
		//search all mib entry to add default ACL rules.
		for (i = 0; i < totalEntry; i++)
		{
			if (mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&entry) == 0)
				continue;
			if(entry.vprio){
				memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
				aclRule.acl_weight = RG_QOS_LOW_ACL_WEIGHT;
				aclRule.action_type = ACL_ACTION_TYPE_QOS;
				aclRule.qos_actions |= ACL_ACTION_1P_REMARKING_BIT;
				aclRule.fwding_type_and_direction = ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET;
/*
				aclRule.action_acl_cvlan.cvlanTagIfDecision = 1;
				aclRule.action_acl_cvlan.cvlanCvidDecision = 0;
				aclRule.action_acl_cvlan.cvlanCpriDecision = 0;
				aclRule.action_acl_cvlan.assignedCvid = entry.vid;
				aclRule.action_acl_cvlan.assignedCpri = (entry.vprio - 1);
*/
				aclRule.action_dot1p_remarking_pri = (entry.vprio - 1);

				if(entry.rg_wan_idx <= 0)
				{
					printf("Invalid rg_wan_idx value ! rg_wan_idx=%d\n", entry.rg_wan_idx);
					return -1;
				}
				aclRule.egress_intf_idx = entry.rg_wan_idx;

				aclRule.filter_fields |= EGRESS_INTF_BIT;
//				aclRule.ingress_port_mask.portmask = 0xf;
				if((ret = rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx)) == 0)
					fprintf(fp,"%d\n",aclIdx);
				else
					printf("rtk_rg_aclFilterAndQos_add QoS rule failed! (ret = %d)\n", ret);
			}
		}
		fclose(fp);
		return 0;
}

int RG_set_static(MIB_CE_ATM_VC_Tp entry)
{
	rtk_rg_ipStaticInfo_t staticInfo;

	memset(&staticInfo, 0, sizeof(staticInfo));
	staticInfo.ipv4_default_gateway_on=entry->dgw;
	staticInfo.gw_mac_auto_learn_for_ipv4=1;
	if(entry->dgw)
		staticInfo.gateway_ipv4_addr=(((struct in_addr *)entry->remoteIpAddr)->s_addr);
	staticInfo.ip_addr=(((struct in_addr *)entry->ipAddr)->s_addr);
	staticInfo.ip_network_mask=(((struct in_addr *)entry->netMask)->s_addr);

	if(entry->dgw)
		staticInfo.ipv4_default_gateway_on = 1;

	staticInfo.ip_version = IPVER_V4ONLY;
	staticInfo.mtu=entry->mtu;
	if(entry->napt==1){
		staticInfo.napt_enable=1;
	}
	else{
		staticInfo.napt_enable=0;
	}

	if((rtk_rg_staticInfo_set(entry->rg_wan_idx, &staticInfo))!=SUCCESS)
		return -1;
	return SUCCESS;
}

int RG_release_static(int wanIntfIdx)
{
		rtk_rg_ipStaticInfo_t staticInfo;

		memset(&staticInfo, 0, sizeof(staticInfo));
		staticInfo.ipv4_default_gateway_on=0;
		staticInfo.gw_mac_auto_learn_for_ipv4=1;
		staticInfo.gateway_ipv4_addr=0;
		staticInfo.ip_addr=0;
		staticInfo.ip_network_mask=0;
		staticInfo.mtu=1500;
		staticInfo.napt_enable=0;

#ifdef CONFIG_IPV6
		staticInfo.ip_version = IPVER_V4V6;
		memcpy(staticInfo.ipv6_addr.ipv6_addr, empty_ipv6, IPV6_ADDR_LEN);
		staticInfo.ipv6_mask_length = 0;
		staticInfo.ipv6_default_gateway_on = 0;
		memcpy(staticInfo.gateway_ipv6_addr.ipv6_addr, empty_ipv6, IPV6_ADDR_LEN);
		staticInfo.gw_mac_auto_learn_for_ipv6 = 0;
#endif

		if((rtk_rg_staticInfo_set(wanIntfIdx, &staticInfo))!=SUCCESS)
			return -1;

		return SUCCESS;
}


#ifdef DBG_DHCP
#define DBG_DHCP_PRF(format, args...) printf(format, ##args)
#else
#define DBG_DHCP_PRF(format, args...)
#endif

int RG_set_dhcp(unsigned int ipaddr, unsigned int submsk, MIB_CE_ATM_VC_Tp entry)
{
	rtk_rg_ipDhcpClientInfo_t *dhcpClient_info=NULL;
	FILE *fp;
	struct in_addr gw_addr;
	char intf_name[10];
	int ret;
	rtk_rg_intfInfo_t intf_info;
	rtk_ipv6_addr_t zeroIPv6={{0}};

	ret = rtk_rg_intfInfo_find(&intf_info, &entry->rg_wan_idx);
	if(ret!=0){
		printf("Find RG interface for wan index %d Fail! Return -1!\n",entry->rg_wan_idx);
		return -1;
	}

	dhcpClient_info = &(intf_info.wan_intf.dhcp_client_info);
	ifGetName(entry->ifIndex, intf_name, sizeof(intf_name));
	DBG_DHCP_PRF("%s-%d : entry.ifIndex:%d intf_name:%s\n", __func__,__LINE__,entry->ifIndex, intf_name);
	DBG_DHCP_PRF("%s-%d : entry.ipDhcp:%d \n", __func__,__LINE__,entry->ipDhcp);
	DBG_DHCP_PRF("%s-%d : entry.ipAddr:%x.%x.%x.%x \n", __func__,__LINE__,entry->ipAddr[0],entry->ipAddr[1]
	,entry->ipAddr[2], entry->ipAddr[3]);
	DBG_DHCP_PRF("%s-%d : entry.netMask:%x.%x.%x.%x \n", __func__,__LINE__,entry->netMask[0],entry->netMask[1]
	,entry->netMask[2], entry->netMask[3]);
	if(entry->ipDhcp == 1)
	{
		char gwip[20];
		char buffer[50];
		sprintf(buffer, "%s.%s", MER_GWINFO_B, intf_name);
		if(!(fp = fopen(buffer, "r")))
			return -1;
			fscanf(fp, "%s", gwip);
			DBG_DHCP_PRF("%s-%d : gwip:%s \n", __func__,__LINE__,gwip);
			if(!inet_aton(gwip,&gw_addr)){
				printf("get gw_addr fail!\n");
			}
			DBG_DHCP_PRF("%s-%d: gw_addr:(%u.%u.%u.%u)\n",__func__,__LINE__,NIP_QUAD(gw_addr.s_addr));
			fclose(fp);
		}

	dhcpClient_info->hw_info.ipv4_default_gateway_on=entry->dgw;
	dhcpClient_info->hw_info.gw_mac_auto_learn_for_ipv4=1;
	dhcpClient_info->hw_info.ip_addr=ipaddr;
	DBG_DHCP_PRF("%s-%d: ip_addr:(%u.%u.%u.%u)\n",__func__,__LINE__,NIP_QUAD(dhcpClient_info->hw_info.ip_addr));
	dhcpClient_info->hw_info.ip_network_mask=submsk;
	dhcpClient_info->hw_info.mtu=1500;

	if(entry->napt==1){
		dhcpClient_info->hw_info.napt_enable=1;
	}else{
		dhcpClient_info->hw_info.napt_enable=0;
	}

	if(entry->dgw)
		dhcpClient_info->hw_info.gateway_ipv4_addr = gw_addr.s_addr;

	DBG_DHCP_PRF("%s-%d: gateway_ip_addr:(%u.%u.%u.%u)\n",__func__,__LINE__,NIP_QUAD(dhcpClient_info->hw_info.gateway_ipv4_addr));
	DBG_DHCP_PRF("%s-%d: ip_network_mask:(%u.%u.%u.%u)\n",__func__,__LINE__,NIP_QUAD(dhcpClient_info->hw_info.ip_network_mask));
	dhcpClient_info->stauts=0;

#ifdef CONFIG_IPV6
	if(entry->IpProtocol==IPVER_IPV4_IPV6){
		//If IPv6 is not ready, set IPv4 only
		if(memcmp(dhcpClient_info->hw_info.ipv6_addr.ipv6_addr,zeroIPv6.ipv6_addr,IPV6_ADDR_LEN)==0){
			dhcpClient_info->hw_info.ip_version = IPVER_V4ONLY;
		}
		else{
			dhcpClient_info->hw_info.ip_version = IPVER_V4V6;
		}
	}
#else
	dhcpClient_info->hw_info.ip_version = IPVER_V4ONLY;
#endif

	if(rtk_rg_dhcpClientInfo_set(entry->rg_wan_idx, dhcpClient_info)!=SUCCESS)
	{
		printf("rtk_rg_dhcpClientInfo_set error!!!\n");
		return -1;
	}
	DBG_DHCP_PRF("%s-%d:\n",__func__,__LINE__);

	return SUCCESS;
}

int RG_release_dhcp(int wanIntfIdx)
{
	rtk_rg_ipDhcpClientInfo_t dhcpClient_info;
	FILE *fp;
	struct in_addr gw_addr;
	char intf_name[10];

	DBG_DHCP_PRF("%s-%d: Release IP got from DHCP\n",__func__,__LINE__);

	memset(&dhcpClient_info,0,sizeof(dhcpClient_info));

	dhcpClient_info.stauts = 1;
	dhcpClient_info.hw_info.ipv4_default_gateway_on = 0;
	dhcpClient_info.hw_info.gw_mac_auto_learn_for_ipv4=1;
	dhcpClient_info.hw_info.ip_addr = 0;
	dhcpClient_info.hw_info.ip_network_mask = 0;
	dhcpClient_info.hw_info.mtu=1500;
	dhcpClient_info.hw_info.napt_enable=0;
	dhcpClient_info.hw_info.gateway_ipv4_addr = 0;

	if(rtk_rg_dhcpClientInfo_set(wanIntfIdx, &dhcpClient_info) != SUCCESS)
	{
		printf("rtk_rg_dhcpClientInfo_set error!!!\n");
		return -1;
	}
	DBG_DHCP_PRF("%s-%d:\n",__func__,__LINE__);

	return SUCCESS;
}
static unsigned int hextol(unsigned char *hex)
{
	return ( (hex[0] << 24) | (hex[1] << 16) | (hex[2] << 8) | (hex[3]));
}
#ifdef CONFIG_USER_L2TPD_L2TPD
int RG_add_l2tp_wan(MIB_L2TP_T *pentry, int mib_l2tp_idx)
{
	rtk_rg_wanIntfConf_t *wan_info = NULL;
	rtk_rg_intfInfo_t *intf_info = NULL;
	MIB_CE_ATM_VC_T entryVC;
	uint32_t ipAddr, netMask;
	uint32_t server_ip;
	int totalVC_entry, i, vc_wan_index=-1, ret=0, wanIntfIdx;
	server_ip = inet_addr(pentry->server);
	//printf("server_ip=0x%08X mib_l2tp_idx=%d\n",server_ip,mib_l2tp_idx);
	totalVC_entry = mib_chain_total(MIB_ATM_VC_TBL);
	for(i=0;i<totalVC_entry;i++){
		if(mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&entryVC) == 0)
			continue;
		ipAddr = hextol(entryVC.ipAddr);
		netMask = hextol(entryVC.netMask);
		//printf("ipAddr=0x%08X netMask=0x%08X\n",ipAddr, netMask);
		if(netMask == 0 || ipAddr == 0)
			continue;
		if((ipAddr & netMask) == (server_ip & netMask)){
			vc_wan_index = entryVC.rg_wan_idx;
			printf("[%s-%d]vc_wan_index = %d\n",__func__,__LINE__,vc_wan_index);
			break;
		}
	}
	if(vc_wan_index < 0){
		printf("[%s-%d]Can't find output WAN!\n",__func__,__LINE__);
		ret = -1;
		goto Error_l2tp3;
	}
	intf_info = (rtk_rg_intfInfo_t *)malloc(sizeof(rtk_rg_intfInfo_t));
	if(intf_info == NULL){
		fprintf(stderr, "ERROR! intf Can't get enough memory space %s\n", strerror(errno));
		ret = -1;
		goto Error_l2tp3;
	}
	memset(intf_info,0,sizeof(rtk_rg_intfInfo_t));

	wan_info = (rtk_rg_wanIntfConf_t *)malloc(sizeof(rtk_rg_wanIntfConf_t));
	if(wan_info == NULL){
		fprintf(stderr, "ERROR! wanIntf Can't get enough memory space %s\n", strerror(errno));
		ret = -1;
		goto Error_l2tp2;
	}
	memset(wan_info,0,sizeof(rtk_rg_wanIntfConf_t));

	ret = rtk_rg_intfInfo_find(intf_info, &vc_wan_index);
	if(ret!=0){
		fprintf(stderr, "ERROR! rtk_rg_intfInfo_find %s\n", strerror(errno));
		ret = -1;
		goto Error_l2tp1;
	}
	memcpy(wan_info,&(intf_info->wan_intf.wan_intf_conf),sizeof(rtk_rg_wanIntfConf_t));
	wan_info->wan_type = RTK_RG_L2TP;
	wan_info->forcedAddNewIntf = 1;
	//dump_wan_info(wan_info);
	if((rtk_rg_wanInterface_add(wan_info, &wanIntfIdx))!=SUCCESS){
		ret = -1;
		fprintf(stderr, "ERROR! rtk_rg_wanInterface_add %s\n", strerror(errno));
		goto Error_l2tp1;
	}
	//printf("[%s-%d] wanIntfIdx=%d\n",__func__,__LINE__,wanIntfIdx);
	pentry->rg_wan_idx = wanIntfIdx;
	mib_chain_update(MIB_L2TP_TBL, pentry, mib_l2tp_idx);
	Error_l2tp1:
		if(wan_info)
			free(wan_info);
	Error_l2tp2:
		if(intf_info)
			free(intf_info);
	Error_l2tp3:
	return ret;
}
int RG_add_l2tp(unsigned long gw_ip, unsigned long my_ip, MIB_L2TP_T *pentry)
{
	rtk_rg_l2tpClientInfoBeforeDial_t l2tpClientInfoB;
	rtk_rg_l2tpClientInfoAfterDial_t *l2tpClientInfoA = NULL;
	rtk_rg_intfInfo_t intf_info;
	rtk_rg_wanIntfConf_t *wan_info = NULL;
	int ret=-1, i, totalVC_entry, vc_wan_index;
	MIB_CE_ATM_VC_T entryVC;
	uint32_t ipAddr, netMask;
	uint32_t server_ip;
	totalVC_entry = mib_chain_total(MIB_ATM_VC_TBL);
	for(i=0;i<totalVC_entry;i++){
		if(mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&entryVC) == 0)
			continue;
		ipAddr = hextol(entryVC.ipAddr);
		netMask = hextol(entryVC.netMask);
		//printf("ipAddr=0x%08X netMask=0x%08X\n",ipAddr, netMask);
		if(netMask == 0 || ipAddr == 0)
			continue;
		if((ipAddr & netMask) == (server_ip & netMask)){
			vc_wan_index = entryVC.rg_wan_idx;
			//printf("[%s-%d]vc_wan_index = %d\n",__func__,__LINE__,vc_wan_index);
			break;
		}
	}
/*
	printf("\n[%s:%d] gw_ip=0x%x, my_ip=0x%x\n",__func__,__LINE__,gw_ip,my_ip);
	printf("\n[%s:%d] outer_port=%d, gateway_outer_port=%d\n",__func__,__LINE__,pentry->outer_port,pentry->gateway_outer_port);
	printf("\n[%s:%d] session_id=%d, gateway_session_id=%d\n",__func__,__LINE__,pentry->session_id,pentry->gateway_session_id);
	printf("\n[%s:%d] tunnel_id=%d, gateway_tunnel_id=%d\n",__func__,__LINE__,pentry->tunnel_id, pentry->gateway_tunnel_id);
*/
	memset(&l2tpClientInfoB, 0, sizeof(l2tpClientInfoB));
	memcpy(l2tpClientInfoB.username, pentry->username, MAX_NAME_LEN);
	memcpy(l2tpClientInfoB.password, pentry->password, MAX_NAME_LEN);
//	printf("\n[%s:%d] username=%s, password=%s\n",__func__,__LINE__,l2tpClientInfoB.username,l2tpClientInfoB.password);
	l2tpClientInfoB.l2tp_ipv4_addr=inet_addr(pentry->server);
	if((rtk_rg_l2tpClientInfoBeforeDial_set(pentry->rg_wan_idx, &l2tpClientInfoB)) != SUCCESS){
		return -1;
	}
	ret = rtk_rg_intfInfo_find(&intf_info, &pentry->rg_wan_idx);
	if(ret!=0){
		printf("Find RG interface for wan index %d Fail! Return -1!\n",pentry->rg_wan_idx);
		return -1;
	}
	l2tpClientInfoA = &(intf_info.wan_intf.l2tp_info.after_dial);
	l2tpClientInfoA->outer_port=pentry->outer_port;
	l2tpClientInfoA->gateway_outer_port=pentry->gateway_outer_port;
	l2tpClientInfoA->tunnelId=pentry->tunnel_id;
	l2tpClientInfoA->sessionId=pentry->session_id;
	l2tpClientInfoA->gateway_tunnelId=pentry->gateway_tunnel_id;
	l2tpClientInfoA->gateway_sessionId=pentry->gateway_session_id;
	l2tpClientInfoA->hw_info.ip_version= IPVER_V4ONLY;
	l2tpClientInfoA->hw_info.napt_enable=entryVC.napt;
	l2tpClientInfoA->hw_info.ipv4_default_gateway_on=pentry->dgw;
	l2tpClientInfoA->hw_info.ip_addr=my_ip; //wan ip:192.168.150.116
	l2tpClientInfoA->hw_info.ip_network_mask=0xffffff00; //255.255.255.0
	l2tpClientInfoA->hw_info.gateway_ipv4_addr =gw_ip; //wan gateway ip:192.168.150.117
	l2tpClientInfoA->hw_info.mtu=1440;
	l2tpClientInfoA->hw_info.gw_mac_auto_learn_for_ipv4=1;
	if((rtk_rg_l2tpClientInfoAfterDial_set(pentry->rg_wan_idx, l2tpClientInfoA))!= SUCCESS)
	return -1;
	return ret;
}
#endif /*CONFIG_USER_L2TPD_L2TPD*/
#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
int RG_add_pptp_wan(MIB_PPTP_T *pentry, int mib_pptp_idx)
{
	rtk_rg_wanIntfConf_t *wan_info = NULL;
	rtk_rg_intfInfo_t *intf_info = NULL;
	MIB_CE_ATM_VC_T entryVC;
	uint32_t ipAddr, netMask;
	uint32_t server_ip;
	int totalVC_entry, i, vc_wan_index=-1, ret=0, wanIntfIdx;
	server_ip = inet_addr(pentry->server);
//	printf("server_ip=0x%08X mib_pptp_idx=%d\n",server_ip,mib_pptp_idx);
	totalVC_entry = mib_chain_total(MIB_ATM_VC_TBL);
	for(i=0;i<totalVC_entry;i++){
		if(mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&entryVC) == 0)
			continue;
		ipAddr = hextol(entryVC.ipAddr);
		netMask = hextol(entryVC.netMask);
//		printf("ipAddr=0x%08X netMask=0x%08X\n",ipAddr, netMask);
		if(netMask == 0 || ipAddr == 0)
			continue;
		if((ipAddr & netMask) == (server_ip & netMask)){
			vc_wan_index = entryVC.rg_wan_idx;
			printf("[%s-%d] vc_wan_index = %d\n",__func__,__LINE__,vc_wan_index);
			break;
		}
	}
	if(vc_wan_index < 0){
		printf("[%s-%d]Can't find output WAN!\n",__func__,__LINE__);
		ret = -1;
		goto Error_Pptp3;
	}
	intf_info = (rtk_rg_intfInfo_t *)malloc(sizeof(rtk_rg_intfInfo_t));
	if(intf_info == NULL){
		fprintf(stderr, "ERROR! intf Can't get enough memory space %s\n", strerror(errno));
		ret = -1;
		goto Error_Pptp3;
	}
	memset(intf_info,0,sizeof(rtk_rg_intfInfo_t));
	wan_info = (rtk_rg_wanIntfConf_t *)malloc(sizeof(rtk_rg_wanIntfConf_t));
	if(wan_info == NULL){
		fprintf(stderr, "ERROR! wanIntf Can't get enough memory space %s\n", strerror(errno));
		ret = -1;
		goto Error_Pptp2;
	}
	memset(wan_info,0,sizeof(rtk_rg_wanIntfConf_t));
	ret = rtk_rg_intfInfo_find(intf_info, &vc_wan_index);
	if(ret!=0){
		fprintf(stderr, "ERROR! rtk_rg_intfInfo_find %s\n", strerror(errno));
		ret = -1;
		goto Error_Pptp1;
	}
	memcpy(wan_info,&(intf_info->wan_intf.wan_intf_conf),sizeof(rtk_rg_wanIntfConf_t));
	wan_info->wan_type = RTK_RG_PPTP;
	wan_info->forcedAddNewIntf = 1;
	//dump_wan_info(wan_info);
	if((rtk_rg_wanInterface_add(wan_info, &wanIntfIdx))!=SUCCESS){
		ret = -1;
		printf("%s-%d rtk_rg_wanInterface_add error\n",__func__,__LINE__);
		goto Error_Pptp1;
	}
	//printf("%s-%d wanIntfIdx=%d\n",__func__,__LINE__,wanIntfIdx);
	pentry->rg_wan_idx = wanIntfIdx;
	mib_chain_update(MIB_PPTP_TBL, pentry, mib_pptp_idx);
	Error_Pptp1:
		if(wan_info)
			free(wan_info);
	Error_Pptp2:
		if(intf_info)
			free(intf_info);
	Error_Pptp3:
	return ret;
}
int RG_add_pptp( unsigned long gw_ip, unsigned long my_ip, MIB_PPTP_T *pentry)
{
	rtk_rg_pptpClientInfoBeforeDial_t pptpClientInfoB;
	rtk_rg_pptpClientInfoAfterDial_t *pptpClientInfoA = NULL;
	rtk_rg_intfInfo_t intf_info;
	rtk_rg_wanIntfConf_t *wan_info = NULL;
	int ret=-1, i, totalVC_entry, vc_wan_index;
	MIB_CE_ATM_VC_T entryVC;
	uint32_t ipAddr, netMask;
	uint32_t server_ip;
	totalVC_entry = mib_chain_total(MIB_ATM_VC_TBL);
	for(i=0;i<totalVC_entry;i++){
		if(mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&entryVC) == 0)
			continue;
		ipAddr = hextol(entryVC.ipAddr);
		netMask = hextol(entryVC.netMask);
//		printf("ipAddr=0x%08X netMask=0x%08X\n",ipAddr, netMask);
		if(netMask == 0 || ipAddr == 0)
			continue;
		if((ipAddr & netMask) == (server_ip & netMask)){
			vc_wan_index = entryVC.rg_wan_idx;
			printf("vc_wan_index = %d\n",vc_wan_index);
			break;
		}
	}
//	printf("\n[%s:%d] callId=%d, gateway_callId=%d, gw_ip=0x%x, my_ip=0x%x\n",__func__,__LINE__,pentry->callid,pentry->peer_callid,gw_ip,my_ip);
	memset(&pptpClientInfoB, 0, sizeof(pptpClientInfoB));
	memcpy(pptpClientInfoB.username, pentry->username, MAX_NAME_LEN);
	memcpy(pptpClientInfoB.password, pentry->password, MAX_NAME_LEN);
//	printf("\n[%s:%d] username=%s, password=%s\n",__func__,__LINE__,pptpClientInfoB.username,pptpClientInfoB.password);
	pptpClientInfoB.pptp_ipv4_addr=inet_addr(pentry->server);
	if((rtk_rg_pptpClientInfoBeforeDial_set(pentry->rg_wan_idx, &pptpClientInfoB)) != SUCCESS){
		return -1;
	}
	ret = rtk_rg_intfInfo_find(&intf_info, &pentry->rg_wan_idx);
	if(ret!=0){
		printf("Find RG interface for wan index %d Fail! Return -1!\n",pentry->rg_wan_idx);
		return -1;
	}
	pptpClientInfoA = &(intf_info.wan_intf.pptp_info.after_dial);
	pptpClientInfoA->callId=pentry->callid;
	pptpClientInfoA->gateway_callId=pentry->peer_callid;
	pptpClientInfoA->hw_info.ip_version= IPVER_V4ONLY;
	pptpClientInfoA->hw_info.napt_enable=entryVC.napt;
	pptpClientInfoA->hw_info.ipv4_default_gateway_on=pentry->dgw;;
	pptpClientInfoA->hw_info.ip_addr=my_ip; //wan ip:192.168.150.116
	pptpClientInfoA->hw_info.ip_network_mask=0xffffff00; //255.255.255.0
	pptpClientInfoA->hw_info.gateway_ipv4_addr=gw_ip; //wan gateway ip:192.168.150.117
	pptpClientInfoA->hw_info.mtu=1440;
	pptpClientInfoA->hw_info.gw_mac_auto_learn_for_ipv4=1;
	if((rtk_rg_pptpClientInfoAfterDial_set(pentry->rg_wan_idx, pptpClientInfoA))!= SUCCESS)
	return -1;
	return 0;
}
int RG_release_pptp(int wanIntfIdx)
{
	rtk_rg_pptpClientInfoBeforeDial_t pptpClientInfoB;
	rtk_rg_pptpClientInfoAfterDial_t pptpClientInfoA;
//	printf("\n[%s:%d] wanIntfIdx=%d, Release IP got from pptp\n",__func__,__LINE__,wanIntfIdx);
	memset(&pptpClientInfoB, 0, sizeof(pptpClientInfoB));
	memset(&pptpClientInfoA, 0, sizeof(pptpClientInfoA));
	if((rtk_rg_pptpClientInfoBeforeDial_set(wanIntfIdx, &pptpClientInfoB)) != SUCCESS){
		return -1;
	}
	pptpClientInfoA.hw_info.mtu=1492;
	pptpClientInfoA.hw_info.gw_mac_auto_learn_for_ipv4=1;
	if((rtk_rg_pptpClientInfoAfterDial_set(wanIntfIdx, &pptpClientInfoA))!= SUCCESS)
		return -1;
	return 0;
}
#endif /*CONFIG_USER_PPTP_CLIENT_PPTP*/
int RG_add_pppoe(unsigned short session_id, unsigned long gw_ip, unsigned long my_ip, unsigned char* gw_mac, MIB_CE_ATM_VC_T *vcEntry){
	rtk_rg_wanIntfConf_t wan_info;
	rtk_rg_pppoeClientInfoBeforeDial_t pppoeClientInfoB;
	rtk_rg_pppoeClientInfoAfterDial_t *pppoeClientInfoA=NULL;
	unsigned char value[6];
	int i,ret;
	int wanPhyPort=0;
	rtk_rg_intfInfo_t intf_info;
	rtk_ipv6_addr_t zeroIPv6={{0}};

	//This function is to set up PPPoE IPv4 IP/Gateway info into RG

#ifdef CONFIG_IPV6
	if(vcEntry->IpProtocol == IPVER_IPV6)
		return -1;
#endif

	printf("\n[%s:%d] session_id=%d, gw_ip=0x%x, my_ip=0x%x\n",__func__,__LINE__,session_id,gw_ip,my_ip);
	memset(&pppoeClientInfoB, 0, sizeof(pppoeClientInfoB));
	if((rtk_rg_pppoeClientInfoBeforeDial_set(vcEntry->rg_wan_idx, &pppoeClientInfoB)) != SUCCESS){
		return -1;
	}

	ret = rtk_rg_intfInfo_find(&intf_info, &vcEntry->rg_wan_idx);
	if(ret!=0){
		printf("Find RG interface for wan index %d Fail! Return -1!\n",vcEntry->rg_wan_idx);
		return -1;
	}
	pppoeClientInfoA = &(intf_info.wan_intf.pppoe_info.after_dial);
	pppoeClientInfoA->hw_info.napt_enable = vcEntry->napt;
	pppoeClientInfoA->hw_info.ip_addr = my_ip;
	pppoeClientInfoA->hw_info.ip_network_mask = 0xffffffff;
	pppoeClientInfoA->hw_info.ipv4_default_gateway_on = vcEntry->dgw;
	pppoeClientInfoA->hw_info.gateway_ipv4_addr = gw_ip;
	pppoeClientInfoA->hw_info.mtu = vcEntry->mtu;
	pppoeClientInfoA->sessionId = session_id;
	pppoeClientInfoA->hw_info.gw_mac_auto_learn_for_ipv4 = 0;
	memcpy(pppoeClientInfoA->hw_info.gateway_mac_addr_for_ipv4.octet, gw_mac, 6);

#ifdef CONFIG_IPV6
	if(vcEntry->IpProtocol==IPVER_IPV4_IPV6){
		//If IPv6 is not ready, set IPv4 only
		if(memcmp( pppoeClientInfoA->hw_info.ipv6_addr.ipv6_addr,zeroIPv6.ipv6_addr,IPV6_ADDR_LEN)==0){
			pppoeClientInfoA->hw_info.ip_version = IPVER_V4ONLY;
		}
		else{
			pppoeClientInfoA->hw_info.ip_version = IPVER_V4V6;
		}
	}
#else
	pppoeClientInfoA->hw_info.ip_version = IPVER_V4ONLY;
#endif


	if((rtk_rg_pppoeClientInfoAfterDial_set(vcEntry->rg_wan_idx, pppoeClientInfoA)) != SUCCESS){
		return -1;
	}
}

//siyuan release pppoe setting in romedriver when pppoe connection is down
int RG_release_pppoe(int wanIntfIdx)
{
	rtk_rg_wanIntfConf_t wan_info;
	rtk_rg_pppoeClientInfoAfterDial_t pppoeClientInfoA;
	rtk_rg_pppoeClientInfoBeforeDial_t pppoeClientInfoB;

	printf("\n[%s:%d] wanIntfIdx=%d, Release IP got from PPPOE\n",__func__,__LINE__,wanIntfIdx);

	memset(&pppoeClientInfoB, 0, sizeof(pppoeClientInfoB));
	memset(&pppoeClientInfoA, 0, sizeof(pppoeClientInfoA));
	if((rtk_rg_pppoeClientInfoBeforeDial_set(wanIntfIdx, &pppoeClientInfoB)) != SUCCESS){
		return -1;
	}

	pppoeClientInfoA.hw_info.napt_enable = 0;
	pppoeClientInfoA.hw_info.ip_addr = 0;
	pppoeClientInfoA.hw_info.ip_network_mask = 0;
	pppoeClientInfoA.hw_info.ipv4_default_gateway_on = 0;
	pppoeClientInfoA.hw_info.gateway_ipv4_addr = 0;
	pppoeClientInfoA.hw_info.mtu = 1492;
	pppoeClientInfoA.sessionId = 0;
	pppoeClientInfoA.hw_info.gw_mac_auto_learn_for_ipv4 = 1;

	if((rtk_rg_pppoeClientInfoAfterDial_set(wanIntfIdx, &pppoeClientInfoA)) != SUCCESS){
		return -1;
	}

}

int RG_Del_All_LAN_Interfaces()
{
	FILE *fp;
	int lanIdx;

	if(!(fp = fopen(RG_LAN_INF_IDX, "r")))
		return -2;
	//va_cmd("/bin/cat", 1, 1, RG_LAN_INF_IDX);

	while(fscanf(fp, "%d\n", &lanIdx) != EOF)
	{
//printf("%s-%d id=%d\n",__func__,__LINE__,lanIdx);
		if(rtk_rg_interface_del(lanIdx))
			DBPRINT(1, "RG_Del_All_LAN_Interfaces failed! (idx = %d)\n", lanIdx);
	}

	fclose(fp);
	unlink(RG_LAN_INF_IDX);
	return 0;
}

int RG_WAN_Interface_Del(unsigned int rg_wan_idx)
{
	int ret=0;
	char cmdStr[64];
	int pon_mode=0;
	rtk_rg_intfInfo_t intf_info;
	//printf("%s-%d del RG WAN[%d]\n",__func__,__LINE__,rg_wan_idx);
#if defined(CONFIG_RTK_RG_INIT) && defined(CONFIG_GPON_FEATURE)
	mib_get(MIB_PON_MODE, (void *)&pon_mode);
	if(pon_mode == GPON_MODE){
	snprintf(cmdStr, sizeof(cmdStr),"echo %d %d %d %d %d %d %d > %s",rg_wan_idx,0,0,0,0,0,0,OMCI_WAN_INFO);
		system(cmdStr);
	}
#endif
	ret = rtk_rg_intfInfo_find(&intf_info, &rg_wan_idx);
	if(ret){
	    //printf("[%s:%d]Find RG interface for wan index %d Fail!\n",__func__,__LINE__,rg_wan_idx);
		return -1;
	}
	if(rtk_rg_interface_del(rg_wan_idx)){
		DBPRINT(1, "%s failed! (idx = %d)\n", __func__, rg_wan_idx);
		ret =-1;
	}
	return ret;
}
/*RG just only can set one default route, so we should block it in user space*/
/*The first Internet connection is set as default route, let others dgw=0*/
//check ATM_VC_TBL.x.dgw (default route is already exist!)
//check modify ATM_VC_TBL index if equal to default route
/*
check RG default route exist or not
3: means you modify the D route --> not D route; or del D route entry
2: D route existed already
1: the entry you choose is D route, must enable ATM_VC_TBL mib entry dgw = 1
0: D route is not exist, but don't need to be setted (bridge mode, or routing none_internet)
-1: something error!
*/
int RG_check_Droute(int configAll, MIB_CE_ATM_VC_Tp pEntry, int *EntryID)
{
	int vcTotal=-1;
	int i,key,idx=-1;
	MIB_CE_ATM_VC_T Entry;
	vcTotal = mib_chain_total(MIB_ATM_VC_TBL);
	if(vcTotal<0)
		return -1;
//	if(configAll == CONFIGALL)
//		return 0;
	key=0;
	for (i = 0; i < vcTotal; i++)
	{
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
				return -1;
		if(Entry.dgw == 1){
	    /*If any entry match the condition, would return directly*/
	    /*Below conditons (case 0, 1, 3) are terminated*/
			return 2;
		}
		//VCentry existed an internet and routing WAN
		if((Entry.applicationtype & X_CT_SRV_INTERNET) && (Entry.cmode > 0) && (key==0)){
			key++;
			idx = i;
		}
	}
	if(key > 0){
		//get D route entry!
		mib_chain_get(MIB_ATM_VC_TBL, idx, (void *)&Entry);
		Entry.dgw = 1;
		if( pEntry==NULL && EntryID == NULL){
			//it means we are at starting up process
			fprintf(stderr, "%s-%d key=%d, idx=%d\n",__func__,__LINE__,key,idx);
			mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry, idx);
			return 4;
		}

		fprintf(stderr, "%s-%d key=%d, Entry.dgw=%d\n",__func__,__LINE__,key,Entry.dgw);

		if(pEntry && pEntry->ifIndex == Entry.ifIndex){
		/*the entry which you modified is setted as D route!*/
			mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry, idx);
			return 1;
		}
		/*Two conditions will go here*/
		/*1. The original D route is deleted, choose another one as D route*/
		/*2. The original D route is modified, choose another one as D route*/
		/*the new one must restart again.*/
		*EntryID = idx;
		mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry, idx);
		return 3;
	}
	else
		return 0;
}

#ifdef MAC_FILTER
int AddRTK_RG_MAC_Filter(MIB_CE_MAC_FILTER_T *MacEntry, int mode)
{
	if(mode == 1){ //black list
		int macfilterIdx, ret;
		rtk_rg_macFilterEntry_t macFilterEntry;
		FILE *fp;

		memset(&macFilterEntry, 0, sizeof(rtk_rg_macFilterEntry_t));
		memcpy(&macFilterEntry.mac, MacEntry->srcMac, MAC_ADDR_LEN);
		macFilterEntry.direct = MacEntry->dir;

		if(!(fp = fopen(RG_MAC_RULES_FILE, "a")))
		{
			fprintf(stderr, "ERROR! %s\n", strerror(errno));
			return -1;
		}

		if((ret = rtk_rg_macFilter_add(&macFilterEntry, &macfilterIdx)) == 0)
			fprintf(fp, "%d\n", macfilterIdx);
		else
			printf("Set rtk_rg_macFilter_add failed! dir = %s error = 0x%x\n", MacEntry->dir? MacEntry->dir == 1? "Source": "Destination": "Both", ret);

		fclose(fp);
		return 0;

	}
	else //white list
	{
		rtk_rg_aclFilterAndQos_t aclRule;
		rtk_rg_intfInfo_t infinfo;
		int aclIdx, i;
		FILE *fp;

		memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));

		aclRule.acl_weight = RG_FIREWALL_ACL_WEIGHT;
		aclRule.filter_fields |= INGRESS_PORT_BIT;
		#ifdef CONFIG_RTL9602C_SERIES
		aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0x3);
		#else		
		aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0xf);
		#endif
		//if (MacEntry->action == 0)
		//	aclRule.action_type = ACL_ACTION_TYPE_DROP;
		//else if (MacEntry->action == 1)
			aclRule.action_type = ACL_ACTION_TYPE_PERMIT;
		//else
		//{
		//	fprintf(stderr, "Wrong mac filter action!\n");
		//	return -1;
		//}

		if (memcmp(MacEntry->srcMac, "\x00\x00\x00\x00\x00\x00", MAC_ADDR_LEN))  // src mac is not empty.
		{
			aclRule.filter_fields |= INGRESS_SMAC_BIT;
			memcpy(&aclRule.ingress_smac, MacEntry->srcMac, MAC_ADDR_LEN);
		}

		//if(memcmp(MacEntry->srcMac, "\x00\x00\x00\x00\x00\x00", MAC_ADDR_LEN))  // dst mac is not empty.
		//{
		//	aclRule.filter_fields |= INGRESS_DMAC_BIT;
		//	memcpy(&aclRule.ingress_dmac, MacEntry->srcMac, MAC_ADDR_LEN);
		//}

		if(!(fp = fopen(RG_MAC_ACL_RULES_FILE, "a")))
		{
			fprintf(stderr, "ERROR! %s\n", strerror(errno));
			return -1;
		}

#if 0
		for( i = 0; i < MAX_NETIF_SW_TABLE_SIZE; i++ )
		{
			if(rtk_rg_intfInfo_find(&infinfo, &i))
				break;

			if(infinfo.is_wan && infinfo.wan_intf.wan_intf_conf.wan_type == RTK_RG_BRIDGE)
			{
				if(MacEntry->dir == DIR_OUT)
				{
					aclRule.filter_fields |= EGRESS_INTF_BIT;
					aclRule.egress_intf_idx = i;  // Set egress interface to WAN.
				}
				else if(MacEntry->dir == DIR_IN)
				{
					aclRule.filter_fields |= INGRESS_INTF_BIT;
					aclRule.ingress_intf_idx = i;  // Set ingress interface to WAN.
				}
				else
				{
					DBPRINT(1, "Invalid MAC filtering direction!\n");
					return -1;
				}

				if(rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx) == 0)
					fprintf(fp, "%d\n", aclIdx);
				else
					printf("Set rtk_rg_aclFilterAndQos_add failed! dir = %s\n", MacEntry->dir? "Incoming": "Outgoing");
			}
		}
#endif
		if(rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx) == 0)
			fprintf(fp, "%d\n", aclIdx);
		else
			printf("Set rtk_rg_aclFilterAndQos_add failed!\n");


		fclose(fp);
		return 0;
	}
}

int RTK_RG_MAC_Filter_Default_Policy(int out_policy)
{
	rtk_rg_aclFilterAndQos_t aclRule;
	int aclIdx;
	FILE *fp;

	memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));

	if(!(fp = fopen(RG_MAC_ACL_RULES_FILE, "a")))
	{
		fprintf(stderr, "ERROR! %s\n", strerror(errno));
		return -2;
	}

	if( out_policy == 0 )
	{
		aclRule.action_type = ACL_ACTION_TYPE_DROP;
		aclRule.filter_fields = INGRESS_DMAC_BIT|INGRESS_PORT_BIT;
		#ifdef CONFIG_RTL9602C_SERIES
		aclRule.ingress_port_mask.portmask = 0x7; // All physical ports.
		#else
		aclRule.ingress_port_mask.portmask = 0x3f; // All physical ports.
		#endif
		aclRule.acl_weight = RG_FIREWALL_ACL_WEIGHT;

		mib_get(MIB_ELAN_MAC_ADDR, (void *)&aclRule.ingress_dmac);

		if(rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx) == 0)
			fprintf(fp, "%d\n", aclIdx);
		else
			printf("rtk_rg_aclFilterAndQos_add mac default policy failed!\n");
	}

	fclose(fp);
	return 0;
}

int FlushRTK_RG_MAC_Filters_in_ACL()
{
	FILE *fp;
	int aclIdx;

	if(!(fp = fopen(RG_MAC_ACL_RULES_FILE, "r")))
		return -2;

	while(fscanf(fp, "%d\n", &aclIdx) != EOF)
	{
		if(rtk_rg_aclFilterAndQos_del(aclIdx))
			DBPRINT(1, "rtk_rg_aclFilterAndQos_del failed! idx = %d\n", aclIdx);
	}

	fclose(fp);
	unlink(RG_MAC_ACL_RULES_FILE);
	return 0;
}

int FlushRTK_RG_MAC_Filters()
{
	FILE *fp;
	int mac_idx;

	if(!(fp = fopen(RG_MAC_RULES_FILE, "r")))
		return -2;

	while(fscanf(fp, "%d\n", &mac_idx) != EOF)
	{
		if(rtk_rg_macFilter_del(mac_idx))
			DBPRINT(1, "rtk_rg_macFilter_del failed! idx = %d\n", mac_idx);
	}

	fclose(fp);
	unlink(RG_MAC_RULES_FILE);
	return 0;
}
#endif // MAC_FILTER

#ifdef IP_PORT_FILTER
int AddRTK_RG_ACL_IPPort_Filter(MIB_CE_IP_PORT_FILTER_T *ipEntry)
{
	rtk_rg_aclFilterAndQos_t aclRule;
	rtk_rg_intfInfo_t infinfo;
	int aclIdx, i;
	ipaddr_t mask;
	FILE *fp;

	memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
	aclRule.filter_fields |= INGRESS_PORT_BIT;
	#ifdef CONFIG_RTL9602C_SERIES
	aclRule.ingress_port_mask.portmask = 0x7; // All physical ports.
	#else
	aclRule.ingress_port_mask.portmask = 0x3f; // All physical ports.
	#endif
	aclRule.acl_weight = RG_FIREWALL_ACL_WEIGHT;

	if (ipEntry->action == 0)
		aclRule.action_type = ACL_ACTION_TYPE_DROP;
	else if (ipEntry->action == 1)
		aclRule.action_type = ACL_ACTION_TYPE_PERMIT;
	else
	{
		fprintf(stderr, "Wrong IP/Port filter action!\n");
		return -1;
	}

	// Source port
	if (ipEntry->srcPortFrom != 0)
	{
		aclRule.filter_fields |= INGRESS_L4_SPORT_RANGE_BIT;
		if (ipEntry->srcPortFrom == ipEntry->srcPortTo)
		{
			aclRule.ingress_src_l4_port_start = aclRule.ingress_src_l4_port_end = ipEntry->srcPortFrom;
		}
		else
		{
			aclRule.ingress_src_l4_port_start = ipEntry->srcPortFrom;
			aclRule.ingress_src_l4_port_end = ipEntry->srcPortTo;
		}
	}

	// Destination port
	if(ipEntry->dstPortFrom != 0)
	{
		aclRule.filter_fields |= INGRESS_L4_DPORT_RANGE_BIT;
		if(ipEntry->dstPortFrom == ipEntry->dstPortTo)
		{
			aclRule.ingress_dest_l4_port_start = aclRule.ingress_dest_l4_port_end = ipEntry->dstPortFrom;
		}
		else
		{
			aclRule.ingress_dest_l4_port_start = ipEntry->dstPortFrom;
			aclRule.ingress_dest_l4_port_end = ipEntry->dstPortTo;
		}
	}

	// Source ip, mask
	if(memcmp(ipEntry->srcIp, "\x00\x00\x00\x00", IP_ADDR_LEN) != 0)
	{
		aclRule.filter_fields |= INGRESS_IPV4_SIP_RANGE_BIT;

		if(ipEntry->smaskbit == 0)
			aclRule.ingress_src_ipv4_addr_start = aclRule.ingress_src_ipv4_addr_end = *((ipaddr_t *)ipEntry->srcIp);
		else
		{
			mask = ~0 << (sizeof(ipaddr_t)*8 - ipEntry->smaskbit);
			mask = htonl(mask);
			aclRule.ingress_src_ipv4_addr_start = *((in_addr_t *)ipEntry->srcIp) & mask;
			aclRule.ingress_src_ipv4_addr_end = *((in_addr_t *)ipEntry->srcIp) | ~mask;
		}
	}

	// Destination ip, mask
	if(memcmp(ipEntry->dstIp, "\x00\x00\x00\x00", IP_ADDR_LEN) != 0)
	{
		aclRule.filter_fields |= INGRESS_IPV4_DIP_RANGE_BIT;
		aclRule.filter_fields |= INGRESS_IPV4_TAGIF_BIT;
		aclRule.ingress_ipv4_tagif = 1;

		if(ipEntry->dmaskbit == 0)
			aclRule.ingress_dest_ipv4_addr_start = aclRule.ingress_dest_ipv4_addr_end = *((ipaddr_t *)ipEntry->dstIp);
		else
		{
			mask = ~0 << (sizeof(ipaddr_t)*8 - ipEntry->dmaskbit);
			mask = htonl(mask);
			aclRule.ingress_dest_ipv4_addr_start = *((in_addr_t *)ipEntry->dstIp) & mask;
			aclRule.ingress_dest_ipv4_addr_end = *((in_addr_t *)ipEntry->dstIp) | ~mask;
		}
	}

	// Protocol
	if( ipEntry->protoType != PROTO_NONE )
	{
		if( ipEntry->protoType == PROTO_TCP )
			aclRule.filter_fields |= INGRESS_L4_TCP_BIT;
		else if( ipEntry->protoType == PROTO_UDP )
			aclRule.filter_fields |= INGRESS_L4_UDP_BIT;
		else if( ipEntry->protoType == PROTO_ICMP)
			aclRule.filter_fields |= INGRESS_L4_ICMP_BIT;
		else
			return -1;
	}

	if(!(fp = fopen(RG_ACL_RULES_FILE, "a")))
	{
		fprintf(stderr, "ERROR! %s\n", strerror(errno));
		return -2;
	}

	if(ipEntry->dir == DIR_OUT)
	{
		aclRule.filter_fields |= INGRESS_DMAC_BIT;

		mib_get(MIB_ELAN_MAC_ADDR, (void *)&aclRule.ingress_dmac);

		if(rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx) == 0)
			fprintf(fp, "%d\n", aclIdx);
		else
			printf("rtk_rg_aclFilterAndQos_add ingress dmac failed!\n");
	}
	else if(ipEntry->dir == DIR_IN)
	{
		int wanPhyPort;
		aclRule.filter_fields |= INGRESS_PORT_BIT;

		#if 0
		if(mib_get(MIB_WAN_PHY_PORT , (void *)&wanPhyPort) == 0)
		{
			printf("Get MIB_WAN_PHY_PORT failed!!!\n");
			wanPhyPort = RTK_RG_MAC_PORT3; //for 0371 default
		}

		aclRule.ingress_port_mask.portmask = 1 << wanPhyPort;
		#endif

		aclRule.ingress_port_mask.portmask = RG_get_wan_phyPortMask();

		if(rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx) == 0)
			fprintf(fp, "%d\n", aclIdx);
		else
			printf("rtk_rg_aclFilterAndQos_add physic port failed!\n");
	}
	else
	{
		DBPRINT(1, "Invalid filtering direction!\n");
		fclose(fp);
		return -1;
	}

	fclose(fp);
	return 0;
}
#endif

int RTK_RG_ACL_IPPort_Filter_Default_Policy(int out_policy)
{
	rtk_rg_aclFilterAndQos_t aclRule;
	int aclIdx;
	FILE *fp;

	memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));

	if(!(fp = fopen(RG_ACL_RULES_FILE, "a")))
	{
		fprintf(stderr, "ERROR! %s\n", strerror(errno));
		return -2;
	}

	if( out_policy == 0 )
	{
		aclRule.action_type = ACL_ACTION_TYPE_DROP;
		aclRule.filter_fields = INGRESS_DMAC_BIT|INGRESS_PORT_BIT|INGRESS_IPV6_TAGIF_BIT;
		#ifdef CONFIG_RTL9602C_SERIES
		aclRule.ingress_port_mask.portmask = 0x7; // All physical ports.
		#else
		aclRule.ingress_port_mask.portmask = 0x3f; // All physical ports.
		#endif
		aclRule.acl_weight = RG_FIREWALL_ACL_WEIGHT;

		mib_get(MIB_ELAN_MAC_ADDR, (void *)&aclRule.ingress_dmac);

		if(rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx) == 0)
			fprintf(fp, "%d\n", aclIdx);
		else
			printf("rtk_rg_aclFilterAndQos_add ip port filter default policy failed!\n");
	}

	fclose(fp);
	return 0;
}

int RTK_RG_ACL_IPPort_Filter_Allow_LAN_to_GW()
{
	rtk_rg_aclFilterAndQos_t aclRule;
	int aclIdx;
	FILE *fp;
	struct in_addr lan_ip;
	char ip2_enabled = 0;

	memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));

	if(!(fp = fopen(RG_ACL_RULES_FILE, "a")))
	{
		fprintf(stderr, "ERROR! %s\n", strerror(errno));
		return -1;
	}

	aclRule.action_type = ACL_ACTION_TYPE_PERMIT;
	aclRule.filter_fields = INGRESS_DMAC_BIT | INGRESS_IPV4_DIP_RANGE_BIT | INGRESS_IPV4_TAGIF_BIT| INGRESS_PORT_BIT;
	aclRule.ingress_ipv4_tagif = 1;

#ifdef CONFIG_RTL9602C_SERIES
	aclRule.ingress_port_mask.portmask = 0x7; // All physical ports.
#else
	aclRule.ingress_port_mask.portmask = 0x3f; // All physical ports.
#endif
	aclRule.acl_weight = RG_FIREWALL_ACL_WEIGHT;

	mib_get(MIB_ELAN_MAC_ADDR, (void *)&aclRule.ingress_dmac);
	mib_get(MIB_ADSL_LAN_IP, (void *)&lan_ip);
	aclRule.ingress_dest_ipv4_addr_start = aclRule.ingress_dest_ipv4_addr_end = *((ipaddr_t *)&lan_ip.s_addr);

	if(rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx) == 0)
		fprintf(fp, "%d\n", aclIdx);
	else
		printf("rtk_rg_aclFilterAndQos_add allow gw ip failed!\n");

	mib_get(MIB_ADSL_LAN_ENABLE_IP2, (void *)&ip2_enabled);

	if(ip2_enabled)
	{
		mib_get(MIB_ADSL_LAN_IP2, (void *)&lan_ip);
		aclRule.ingress_dest_ipv4_addr_start = aclRule.ingress_dest_ipv4_addr_end = *((ipaddr_t *)&lan_ip.s_addr);

		if(rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx) == 0)
			fprintf(fp, "%d\n", aclIdx);
		else
			printf("rtk_rg_aclFilterAndQos_add allow gw ip2 failed!\n");
	}

	aclRule.filter_fields = INGRESS_DMAC_BIT | INGRESS_ETHERTYPE_BIT | INGRESS_PORT_BIT;
	#ifdef CONFIG_RTL9602C_SERIES
	aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0x3);
	#else
	aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0xf);
	#endif
	aclRule.ingress_ethertype = 0x0806;

	if(rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx) == 0)
		fprintf(fp, "%d\n", aclIdx);
	else
		printf("rtk_rg_aclFilterAndQos_add allow gw arp failed!\n");

	fclose(fp);
	return 0;
}

int FlushRTK_RG_ACL_Filters()
{
	FILE *fp;
	int aclIdx;

	if(!(fp = fopen(RG_ACL_RULES_FILE, "r")))
		return -2;

	while(fscanf(fp, "%d\n", &aclIdx) != EOF)
	{
		if(rtk_rg_aclFilterAndQos_del(aclIdx))
			DBPRINT(1, "rtk_rg_aclFilterAndQos_del failed! idx = %d\n", aclIdx);
	}

	fclose(fp);
	unlink(RG_ACL_RULES_FILE);
	return 0;
}

#ifdef CONFIG_IPV6
int IPv6PrefixToStartEnd(char ipv6_addr[IPV6_ADDR_LEN], int prefix, char ipv6_start[IPV6_ADDR_LEN], char ipv6_end[IPV6_ADDR_LEN])
{
	int num_byte = prefix / 8;
	int rem_bits = prefix % 8;

	memcpy(ipv6_start, ipv6_addr, IPV6_ADDR_LEN);
	memcpy(ipv6_end, ipv6_addr, IPV6_ADDR_LEN);

	if(num_byte < IPV6_ADDR_LEN)
	{
		ipv6_start[num_byte] &= ((char)0xFF) << (8-rem_bits);
		ipv6_end[num_byte] |= ~(((char)0xFF) << (8-rem_bits));

		if(num_byte+1 < IPV6_ADDR_LEN)
		{
			memset(ipv6_start+num_byte+1, 0, IPV6_ADDR_LEN-num_byte-1);
			memset(ipv6_end+num_byte+1, 0xff, IPV6_ADDR_LEN-num_byte-1);
		}
	}
}

int AddRTK_RG_ACL_IPv6Port_Filter(MIB_CE_V6_IP_PORT_FILTER_T *ipv6_filter_entry, char *prefixIP)
{
	rtk_rg_aclFilterAndQos_t aclRule;
	rtk_rg_intfInfo_t infinfo;
	int aclIdx, i;
	FILE *fp;
	unsigned char empty_ipv6[IPV6_ADDR_LEN] = {0};

	memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));

	if (ipv6_filter_entry->action == 0)
		aclRule.action_type = ACL_ACTION_TYPE_DROP;
	else if (ipv6_filter_entry->action == 1)
		aclRule.action_type = ACL_ACTION_TYPE_PERMIT;
	else
	{
		fprintf(stderr, "Wrong IP/Port filter action!\n");
		return -1;
	}

	// Source port
	if (ipv6_filter_entry->srcPortFrom != 0)
	{
		aclRule.filter_fields |= INGRESS_L4_SPORT_RANGE_BIT;
		if (ipv6_filter_entry->srcPortFrom == ipv6_filter_entry->srcPortTo)
		{
			aclRule.ingress_src_l4_port_start = aclRule.ingress_src_l4_port_end = ipv6_filter_entry->srcPortFrom;
		}
		else
		{
			aclRule.ingress_src_l4_port_start = ipv6_filter_entry->srcPortFrom;
			aclRule.ingress_src_l4_port_end = ipv6_filter_entry->srcPortTo;
		}
	}

	// Destination port
	if(ipv6_filter_entry->dstPortFrom != 0)
	{
		aclRule.filter_fields |= INGRESS_L4_DPORT_RANGE_BIT;
		if(ipv6_filter_entry->dstPortFrom == ipv6_filter_entry->dstPortTo)
		{
			aclRule.ingress_dest_l4_port_start = aclRule.ingress_dest_l4_port_end = ipv6_filter_entry->dstPortFrom;
		}
		else
		{
			aclRule.ingress_dest_l4_port_start = ipv6_filter_entry->dstPortFrom;
			aclRule.ingress_dest_l4_port_end = ipv6_filter_entry->dstPortTo;
		}
	}

#ifdef CONFIG_IPV6_OLD_FILTER
	// Source ip, mask
	if(memcmp(ipv6_filter_entry->sip6Start, empty_ipv6, IPV6_ADDR_LEN) != 0)
	{
		aclRule.filter_fields |= INGRESS_IPV6_SIP_RANGE_BIT;

		if(ipv6_filter_entry->sip6End[0] == 0)
		{
			if(ipv6_filter_entry->sip6PrefixLen == 0)
			{
				memcpy(aclRule.ingress_src_ipv6_addr_start, ipv6_filter_entry->sip6Start, IPV6_ADDR_LEN);
				memcpy(aclRule.ingress_src_ipv6_addr_end, ipv6_filter_entry->sip6Start, IPV6_ADDR_LEN);
			}
			else
				IPv6PrefixToStartEnd(ipv6_filter_entry->sip6Start, ipv6_filter_entry->sip6PrefixLen, aclRule.ingress_src_ipv6_addr_start, aclRule.ingress_src_ipv6_addr_end);
		}
		else
		{
			memcpy(aclRule.ingress_src_ipv6_addr_start, ipv6_filter_entry->sip6Start, IPV6_ADDR_LEN);
			memcpy(aclRule.ingress_src_ipv6_addr_end, ipv6_filter_entry->sip6End, IPV6_ADDR_LEN);
		}
	}

	// Destination ip, mask
	if(memcmp(ipv6_filter_entry->dip6Start, empty_ipv6, IPV6_ADDR_LEN) != 0)
	{
		aclRule.filter_fields |= INGRESS_IPV6_DIP_RANGE_BIT;

		if(ipv6_filter_entry->dip6End[0] == 0)
		{
			if(ipv6_filter_entry->dip6PrefixLen == 0)
			{
				memcpy(aclRule.ingress_dest_ipv6_addr_start, ipv6_filter_entry->dip6Start, IPV6_ADDR_LEN);
				memcpy(aclRule.ingress_dest_ipv6_addr_end, ipv6_filter_entry->dip6Start, IPV6_ADDR_LEN);
			}
			else
				IPv6PrefixToStartEnd(ipv6_filter_entry->dip6Start, ipv6_filter_entry->dip6PrefixLen, aclRule.ingress_dest_ipv6_addr_start, aclRule.ingress_dest_ipv6_addr_end);
		}
		else
		{
			memcpy(aclRule.ingress_dest_ipv6_addr_start, ipv6_filter_entry->dip6Start, IPV6_ADDR_LEN);
			memcpy(aclRule.ingress_dest_ipv6_addr_end, ipv6_filter_entry->dip6End, IPV6_ADDR_LEN);
		}
	}

#else
	if(!prefixIP){
		printf("Error! PrefixIP should not be NULL!\n");
		return -1;
	}

	// Make Source ip from prefix+sIfId6Start
	if(memcmp(ipv6_filter_entry->sIfId6Start, empty_ipv6, IPV6_ADDR_LEN) != 0)
	{
		int index;

		aclRule.filter_fields |= INGRESS_IPV6_SIP_RANGE_BIT;

		memcpy(aclRule.ingress_src_ipv6_addr_start, (void *) prefixIP, IP6_ADDR_LEN);
		for (index=0; index<8; index++){
			aclRule.ingress_src_ipv6_addr_start[index+8] = ipv6_filter_entry->sIfId6Start[index+8];
		}

		memcpy(aclRule.ingress_src_ipv6_addr_end, aclRule.ingress_src_ipv6_addr_start, IPV6_ADDR_LEN);
	}

	// Make Destination  ip from prefix+dIfId6Start
	if(memcmp(ipv6_filter_entry->dIfId6Start, empty_ipv6, IPV6_ADDR_LEN) != 0)
	{
		int index;
		aclRule.filter_fields |= INGRESS_IPV6_DIP_RANGE_BIT;

		memcpy(aclRule.ingress_dest_ipv6_addr_start, (void *) prefixIP, IP6_ADDR_LEN);
		for (index=0; index<8; index++){
			aclRule.ingress_dest_ipv6_addr_start[index+8] = ipv6_filter_entry->dIfId6Start[index+8];
		}
		memcpy(aclRule.ingress_dest_ipv6_addr_end, aclRule.ingress_dest_ipv6_addr_start, IPV6_ADDR_LEN);
	}
#endif

	// Protocol
	if( ipv6_filter_entry->protoType != PROTO_NONE )
	{
		if( ipv6_filter_entry->protoType == PROTO_TCP )
			aclRule.filter_fields |= INGRESS_L4_TCP_BIT;
		else if( ipv6_filter_entry->protoType == PROTO_UDP )
			aclRule.filter_fields |= INGRESS_L4_UDP_BIT;
		else if( ipv6_filter_entry->protoType == PROTO_ICMP)
			aclRule.filter_fields |= INGRESS_L4_ICMPV6_BIT;
		else
			return -1;
	}
	aclRule.acl_weight = RG_FIREWALL_ACL_WEIGHT;
	aclRule.filter_fields |= INGRESS_IPV4_TAGIF_BIT;//Must not be IPv4
	aclRule.ingress_ipv4_tagif = 1;
	if(!(fp = fopen(RG_ACL_IPv6_RULES_FILE, "a")))
	{
		fprintf(stderr, "ERROR! %s\n", strerror(errno));
		return -2;
	}

	if(ipv6_filter_entry->dir == DIR_OUT)
	{
#if 0
		for( i = 0; i < MAX_NETIF_SW_TABLE_SIZE; i++ )
		{
			memset(&infinfo, 0, sizeof(rtk_rg_intfInfo_t));
			if(rtk_rg_intfInfo_find(&infinfo, &i))
				break;

			if(infinfo.is_wan)
			{
				aclRule.filter_fields |= EGRESS_INTF_BIT;
				aclRule.egress_intf_idx = i;

				if(rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx) == 0)
					fprintf(fp, "%d\n", aclIdx);
				else
					DBPRINT(1, "rtk_rg_aclFilterAndQos_add failed!\n");
			}
		}
#endif
		aclRule.filter_fields |= INGRESS_PORT_BIT;
		#ifdef CONFIG_RTL9602C_SERIES
		aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0x3)|RG_get_wan_phyPortMask();
		#else
		aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0xf)|RG_get_wan_phyPortMask();
		#endif
		if(rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx) == 0)
			fprintf(fp, "%d\n", aclIdx);
		else
			DBPRINT(1, "rtk_rg_aclFilterAndQos_add failed!\n");
	}
	else if(ipv6_filter_entry->dir == DIR_IN)
	{
		int wanPhyPort;
		aclRule.filter_fields |= INGRESS_PORT_BIT;

		#if 0
		if(mib_get(MIB_WAN_PHY_PORT , (void *)&wanPhyPort) == 0)
		{
			printf("Get MIB_WAN_PHY_PORT failed!!!\n");
			wanPhyPort = RTK_RG_MAC_PORT3; //for 0371 default
		}

		aclRule.ingress_port_mask.portmask = 1 << wanPhyPort;
		#endif

		aclRule.ingress_port_mask.portmask = RG_get_wan_phyPortMask();

		if(rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx) == 0)
			fprintf(fp, "%d\n", aclIdx);
		else
			DBPRINT(1, "rtk_rg_aclFilterAndQos_add failed!\n");
	}
	else
	{
		DBPRINT(1, "Invalid filtering direction!\n");
		fclose(fp);
		return -1;
	}

	fclose(fp);
	return 0;
}

int RTK_RG_ACL_IPv6Port_Filter_Default_Policy(int out_policy)
{
	rtk_rg_aclFilterAndQos_t aclRule;
	int aclIdx;
	FILE *fp;
	struct sockaddr hwaddr;

	memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));

	if(!(fp = fopen(RG_ACL_IPv6_RULES_FILE, "a")))
	{
		fprintf(stderr, "ERROR! %s\n", strerror(errno));
		return -2;
	}

	if( out_policy == 0 )
	{
		aclRule.action_type = ACL_ACTION_TYPE_DROP;
		aclRule.filter_fields = INGRESS_DMAC_BIT|INGRESS_PORT_BIT|INGRESS_IPV4_TAGIF_BIT;
		aclRule.ingress_ipv4_tagif = 1;

#ifdef CONFIG_RTL9602C_SERIES
		aclRule.ingress_port_mask.portmask = 0x7; // All physical ports.
#else
		aclRule.ingress_port_mask.portmask = 0x3f; // All physical ports.
#endif
		aclRule.acl_weight = RG_FIREWALL_ACL_WEIGHT;

		//mib_get(MIB_ELAN_MAC_ADDR, (void *)&aclRule.ingress_dmac);
		getInAddr((char *)LANIF, HW_ADDR, &hwaddr);
		memcpy((void *)&aclRule.ingress_dmac, hwaddr.sa_data, 6);

		if(rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx) == 0)
			fprintf(fp, "%d\n", aclIdx);
		else
			printf("rtk_rg_aclFilterAndQos_add ipv6 port filter default policy failed!\n");
	}

	fclose(fp);
	return 0;
}

int FlushRTK_RG_ACL_IPv6Port_Filters()
{
	FILE *fp;
	int aclIdx;

	if(!(fp = fopen(RG_ACL_IPv6_RULES_FILE, "r")))
		return -2;

	while(fscanf(fp, "%d\n", &aclIdx) != EOF)
	{
		if(rtk_rg_aclFilterAndQos_del(aclIdx))
			DBPRINT(1, "rtk_rg_aclFilterAndQos_del failed! idx = %d\n", aclIdx);
	}

	fclose(fp);
	unlink(RG_ACL_IPv6_RULES_FILE);
	return 0;
}
#endif

int RTK_RG_DMZ_Set(int enabled, in_addr_t ip_addr)
{
	int i;
	rtk_rg_intfInfo_t infinfo;
	rtk_rg_dmzInfo_t dmz_info;

	for( i = 0; i < MAX_NETIF_SW_TABLE_SIZE; i++ )
	{
		memset(&infinfo, 0, sizeof(rtk_rg_intfInfo_t));
		if(rtk_rg_intfInfo_find(&infinfo, &i))
			break;

		if(infinfo.is_wan && infinfo.wan_intf.wan_intf_conf.wan_type != RTK_RG_BRIDGE)
		{
			/* dmz setting */
			memset(&dmz_info, 0, sizeof(rtk_rg_dmzInfo_t));
			dmz_info.enabled = enabled;
			dmz_info.mac_mapping_enabled = 0; //use ip
			dmz_info.private_ip = ip_addr;
			rtk_rg_dmzHost_set(i, &dmz_info);
		}
	}
}

#ifdef PORT_FORWARD_GENERAL
int RTK_RG_Vertual_Server_Set(MIB_CE_PORT_FW_T *pf)
{
	rtk_rg_virtualServer_t vs;
	rtk_rg_intfInfo_t inf_info;
	int vs_idx, i, ret;
	FILE *fp;

	memset(&vs, 0, sizeof(rtk_rg_virtualServer_t));

	vs.local_ip = *((ipaddr_t *)pf->ipAddr);
	vs.valid = pf->enable;

	if(pf->fromPort)
	{
		vs.local_port_start = pf->fromPort;
		vs.gateway_port_start = pf->fromPort;
		vs.mappingPortRangeCnt = pf->toPort - pf->fromPort + 1;
	}

	if(pf->externalfromport)
	{
		if(!pf->fromPort)
			vs.local_port_start = pf->externalfromport;

		vs.gateway_port_start = pf->externalfromport;

		if(pf->externaltoport)
			vs.mappingPortRangeCnt = pf->externaltoport - pf->externalfromport + 1;
		else if(pf->fromPort && pf->toPort)
			vs.mappingPortRangeCnt = pf->toPort - pf->fromPort + 1;
		else
			vs.mappingPortRangeCnt = 1;
	}

	// Mapping all, if all fileds of ports are empty.
	if(!pf->fromPort && !pf->externalfromport)
	{
		vs.local_port_start = vs.gateway_port_start = 1;
		vs.mappingPortRangeCnt = 0xffff;
	}

	if(pf->remotehost[0])
	{
	}

	if(!(fp = fopen(RG_VERTUAL_SERVER_FILE, "a")))
		return -2;

	if(pf->ifIndex == DUMMY_IFINDEX) // Work on any interface.
	{
		for( i = 0; i < MAX_NETIF_SW_TABLE_SIZE; i++ )
		{
			memset(&inf_info, 0, sizeof(rtk_rg_intfInfo_t));
			if(rtk_rg_intfInfo_find(&inf_info, &i))
				break;

			if(inf_info.is_wan)
			{
				vs.wan_intf_idx = i;

				if(pf->protoType == PROTO_TCP || pf->protoType == PROTO_UDPTCP)
				{
					vs.is_tcp = 1;
					ret = rtk_rg_virtualServer_add(&vs, &vs_idx);

					DBPRINT(0, "Add vertual server. RG WAN Index=%d, protocol=%s\n", vs.wan_intf_idx, vs.is_tcp? "TCP": "UDP");
					if(ret == 0)
						fprintf(fp, "%d\n", vs_idx);
					else
					{
						DBPRINT(1, "Error %d: rtk_rg_virtualServer_add failed! protoType=TCP\n", ret);
						continue;
					}
				}

				if(pf->protoType == PROTO_UDP || pf->protoType == PROTO_UDPTCP)
				{
					vs.is_tcp = 0;
					ret = rtk_rg_virtualServer_add(&vs, &vs_idx);

					DBPRINT(0, "Add vertual server. RG WAN Index=%d, protocol=%s\n", vs.wan_intf_idx, vs.is_tcp? "TCP": "UDP");

					if(ret == 0)
						fprintf(fp, "%d\n", vs_idx);
					else
					{
						DBPRINT(1, "Error %d: rtk_rg_virtualServer_add failed! protoType=UDP\n", ret);
						continue;
					}
				}
			}
		}
	}
	else
	{
		int totalEntry = mib_chain_total(MIB_ATM_VC_TBL);
		MIB_CE_ATM_VC_T entry;

		for (i = 0; i < totalEntry; i++)
		{
			if (mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&entry) == 0)
				continue;

			if( pf->ifIndex == entry.ifIndex )
			{
				vs.wan_intf_idx = entry.rg_wan_idx;

				if(pf->protoType == PROTO_TCP || pf->protoType == PROTO_UDPTCP)
				{
					vs.is_tcp = 1;
					ret = rtk_rg_virtualServer_add(&vs, &vs_idx);

					DBPRINT(2, "Add vertual server. Interface index=%X, RG WAN Index=%d, protocol=%s\n", entry.ifIndex, entry.rg_wan_idx, vs.is_tcp? "TCP": "UDP");
					if(ret == 0)
						fprintf(fp, "%d\n", vs_idx);
					else
					{
						DBPRINT(1, "Error %d: rtk_rg_virtualServer_add failed! protoType=TCP\n", ret);
						continue;
					}
				}

				if(pf->protoType == PROTO_UDP || pf->protoType == PROTO_UDPTCP)
				{
					vs.is_tcp = 0;
					ret = rtk_rg_virtualServer_add(&vs, &vs_idx);

					DBPRINT(2, "Add vertual server. Interface index=%X, RG WAN Index=%d, protocol=%s\n", entry.ifIndex, entry.rg_wan_idx, vs.is_tcp? "TCP": "UDP");
					if(ret == 0)
						fprintf(fp, "%d\n", vs_idx);
					else
					{
						DBPRINT(1, "Error %d: rtk_rg_virtualServer_add failed! protoType=UDP\n", ret);
						continue;
					}
				}
				break;
			}
		}
	}

	fclose(fp);
	return 0;
}

int FlushRTK_RG_Vertual_Server()
{
	FILE *fp;
	int vsIdx;

	if(!(fp = fopen(RG_VERTUAL_SERVER_FILE, "r")))
		return -1;

	while(fscanf(fp, "%d\n", &vsIdx) != EOF)
	{
		if(rtk_rg_virtualServer_del(vsIdx))
			printf("rtk_rg_virtualServer_del failed! idx = %d\n", vsIdx);
		else
			printf("Deleted Vertual Server %d.\n", vsIdx);
	}

	unlink(RG_VERTUAL_SERVER_FILE);

	return 0;
}
#endif

#ifdef CONFIG_USER_IP_QOS_3

int RTK_RG_QoS_TotalBandwidth_Set(int TotalBandwidthKbps)
{
	int wanPhyPort=0;

	// Note: if TotalBandwidthKbps =0, means unlimit
#if 0
	if(!mib_get(MIB_WAN_PHY_PORT , (void *)&wanPhyPort)){
		printf("get MIB_WAN_PHY_PORT failed!!!\n");
		return -1;
	}
#endif
	if((wanPhyPort = RG_get_wan_phyPortId()) == -1){
		printf("get wan phy port id failed!!!\n");
		return -1;
	}

	if(rtk_rg_portEgrBandwidthCtrlRate_set(wanPhyPort,TotalBandwidthKbps)){
		printf("set EgrBandwidthCtrlRate on port %d failed!!!\n",wanPhyPort);
		return -1;
	}

	return 0;
}


#ifdef QOS_TRAFFIC_SHAPING_BY_SSID
void reset_ssid_traffic_shwping_rule()
{
	char cmdBuf[100]={0};
	int i,vwlan_idx, mapped_vwlan_idx;

	printf("reset_ssid_traffic_shwping_rule");
	for(i = 0; i<NUM_WLAN_INTERFACE; i++) {

#ifdef WLAN_MBSSID
		for (vwlan_idx=0; vwlan_idx<=NUM_VWLAN_INTERFACE; vwlan_idx++) {
			if(i==1){
#ifdef CONFIG_RTL_CLIENT_MODE_SUPPORT
				mapped_vwlan_idx = vwlan_idx+14;
#else
				mapped_vwlan_idx = vwlan_idx+13;
#endif
			}else
				mapped_vwlan_idx = vwlan_idx;

			sprintf(cmdBuf,"echo %d 0 > %s",mapped_vwlan_idx, RG_WIFI_INGRESS_RATE_LIMIT_FILE);
			printf("%s\n",cmdBuf);
			system(cmdBuf);
		}
#endif
	}
}

int set_ssid_traffic_shaping_rule(MIB_CE_IP_TC_Tp qos_entry)
{
	MIB_CE_MBSSIB_T Entry;
	int i,vwlan_idx;
	int found=0;
	int ori_wlan_idx;
	int found_wlan_idx=-1;

	ori_wlan_idx = wlan_idx;
	for(i = 0; i<NUM_WLAN_INTERFACE; i++) {

		wlan_idx = i;
		if (!getInFlags((char *)getWlanIfName(), 0)) {
			printf("Wireless Interface Not Found !\n");
			continue;
	    }

#ifdef WLAN_MBSSID
		for (vwlan_idx=0; vwlan_idx<=NUM_VWLAN_INTERFACE; vwlan_idx++) {
			wlan_getEntry(&Entry, vwlan_idx);
			if(Entry.wlanDisabled)
				continue;

			if(strcmp(Entry.ssid,qos_entry->ssid)==0){
				found=1;
				found_wlan_idx = wlan_idx;
				goto found;
			}
		}
#endif
	}

found:
	wlan_idx = ori_wlan_idx;
	if(found){
		char cmdBuf[100]={0};
		//echo Wlan_idx Rate > /proc/rg/wifi_ingress_rate_limit
		if(found_wlan_idx==0){  //wlan0
			sprintf(cmdBuf,"echo %d %d > /proc/rg/wifi_ingress_rate_limit",vwlan_idx,qos_entry->limitSpeed);
			printf("cmdBuf=%s\n",cmdBuf);
			system(cmdBuf);
			return 0;

		}else if(found_wlan_idx==1){ //wlan1
#ifdef CONFIG_RTL_CLIENT_MODE_SUPPORT
			vwlan_idx+=14;
#else
			vwlan_idx+=13;
#endif
			sprintf(cmdBuf,"echo %d %d > /proc/rg/wifi_ingress_rate_limit",vwlan_idx,qos_entry->limitSpeed);
			printf("cmdBuf=%s\n",cmdBuf);
			system(cmdBuf);
			return 0;

		}

	}
	else
		printf("Error! Not found this ssid in active SSID list\n");

	return -1;

}
#endif

int RTK_RG_QoS_Car_Rule_Set(MIB_CE_IP_TC_Tp qos_entry)
{
	rtk_rg_aclFilterAndQos_t aclRule;
	MIB_CE_ATM_VC_T vc_entry;
	int aclIdx, ret, i, total_vc;
	FILE *fp;
	ipaddr_t mask;
	unsigned char empty_ipv6[IPV6_ADDR_LEN] = {0};

#ifdef QOS_TRAFFIC_SHAPING_BY_SSID
	if(qos_entry->ssid[0]){
		return set_ssid_traffic_shaping_rule(qos_entry);
	}
#endif

	memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
	aclRule.fwding_type_and_direction = ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET;
	aclRule.action_type = ACL_ACTION_TYPE_QOS;
	aclRule.acl_weight = RG_QOS_ACL_WEIGHT;

	//By default is filter packets from ALL LAN port.
	aclRule.filter_fields |= INGRESS_PORT_BIT;

	if(qos_entry->direction == QOS_DIRECTION_UPSTREAM)
	#ifdef CONFIG_RTL9602C_SERIES
		aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0x3);
	#else		
		aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0xf);
	#endif
	else if(qos_entry->direction == QOS_DIRECTION_DOWNSTREAM)
		aclRule.ingress_port_mask.portmask = RG_get_wan_phyPortMask();

	// Filter rule of Protocol: UDP, TCP, ICMP, UDP and TCP
	if(qos_entry->protoType != PROTO_NONE)
	{
		if(qos_entry->protoType == PROTO_TCP)
			aclRule.filter_fields |= INGRESS_L4_TCP_BIT;
		else if(qos_entry->protoType == PROTO_UDP)
			aclRule.filter_fields |= INGRESS_L4_UDP_BIT;
		else if(qos_entry->protoType == PROTO_ICMP)
			aclRule.filter_fields |= INGRESS_L4_ICMP_BIT;
		else if(qos_entry->protoType == PROTO_UDPTCP)
			aclRule.filter_fields |= INGRESS_L4_TCP_BIT | INGRESS_L4_UDP_BIT;
		else
		{
			DBPRINT(1, "Add acl rule failed! No support of this protocol type!\n");
			return -1;
		}
	}

	if(qos_entry->vlanID != 0)
	{
		aclRule.filter_fields |= INGRESS_CTAG_VID_BIT;
		aclRule.ingress_ctag_vid = qos_entry->vlanID;
	}

	if(qos_entry->sport != 0)
	{
		aclRule.filter_fields |= INGRESS_L4_SPORT_RANGE_BIT;
		aclRule.ingress_src_l4_port_start = aclRule.ingress_src_l4_port_end = qos_entry->sport;
	}

	if(qos_entry->dport != 0)
	{
		aclRule.filter_fields |= INGRESS_L4_DPORT_RANGE_BIT;
		aclRule.ingress_dest_l4_port_start = aclRule.ingress_dest_l4_port_end = qos_entry->dport;
	}

#ifdef CONFIG_IPV6
	if(qos_entry->IpProtocol == IPVER_IPV6)  // IPv6
	{
		aclRule.filter_fields |= INGRESS_IPV4_TAGIF_BIT;
		aclRule.ingress_ipv4_tagif = 1;
		// Source ip, prefix
		if(memcmp(qos_entry->sip6, empty_ipv6, IPV6_ADDR_LEN) != 0)
		{
			aclRule.filter_fields |= INGRESS_IPV6_SIP_RANGE_BIT;

			IPv6PrefixToStartEnd(qos_entry->sip6, qos_entry->sip6PrefixLen, aclRule.ingress_src_ipv6_addr_start, aclRule.ingress_src_ipv6_addr_end);
		}

		// Destination ip, prefix
		if(memcmp(qos_entry->dip6, empty_ipv6, IPV6_ADDR_LEN) != 0)
		{
			aclRule.filter_fields |= INGRESS_IPV6_DIP_RANGE_BIT;

			IPv6PrefixToStartEnd(qos_entry->dip6, qos_entry->dip6PrefixLen, aclRule.ingress_dest_ipv6_addr_start, aclRule.ingress_dest_ipv6_addr_end);
		}
	}
	else if(qos_entry->IpProtocol == IPVER_IPV4) // IPv4
	{
#endif
		aclRule.filter_fields |= INGRESS_IPV6_TAGIF_BIT;
		// Source ip, mask
		if(memcmp(qos_entry->srcip, "\x00\x00\x00\x00", IP_ADDR_LEN) != 0)
		{
			aclRule.filter_fields |= INGRESS_IPV4_SIP_RANGE_BIT;

			if(qos_entry->smaskbits == 0)
				aclRule.ingress_src_ipv4_addr_start = aclRule.ingress_src_ipv4_addr_end = *((ipaddr_t *)qos_entry->srcip);
			else
			{
				mask = ~0 << (sizeof(ipaddr_t)*8 - qos_entry->smaskbits);
				mask = htonl(mask);
				aclRule.ingress_src_ipv4_addr_start = *((in_addr_t *)qos_entry->srcip) & mask;
				aclRule.ingress_src_ipv4_addr_end = *((in_addr_t *)qos_entry->srcip) | ~mask;
			}
		}

		// Destination ip, mask
		if(memcmp(qos_entry->dstip, "\x00\x00\x00\x00", IP_ADDR_LEN) != 0)
		{
			aclRule.filter_fields |= INGRESS_IPV4_DIP_RANGE_BIT;
			aclRule.filter_fields |= INGRESS_IPV4_TAGIF_BIT;
			aclRule.ingress_ipv4_tagif = 1;

			if(qos_entry->dmaskbits == 0)
				aclRule.ingress_dest_ipv4_addr_start = aclRule.ingress_dest_ipv4_addr_end = *((ipaddr_t *)qos_entry->dstip);
			else
			{
				mask = ~0 << (sizeof(ipaddr_t)*8 - qos_entry->dmaskbits);
				mask = htonl(mask);
				aclRule.ingress_dest_ipv4_addr_start = *((in_addr_t *)qos_entry->dstip) & mask;
				aclRule.ingress_dest_ipv4_addr_end = *((in_addr_t *)qos_entry->dstip) | ~mask;
			}
		}
#ifdef CONFIG_IPV6
	}
#endif

/* ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET support ingress pattern only
 *
	// Filter rule of WAN interface
	if(qos_entry->ifIndex != DUMMY_IFINDEX)
	{
		total_vc = mib_chain_total(MIB_ATM_VC_TBL);

		for( i = 0; i < total_vc; i++ )
		{
			if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&vc_entry))
				continue;

			if(vc_entry.ifIndex == qos_entry->ifIndex)
			{
				aclRule.filter_fields |= EGRESS_INTF_BIT;
				aclRule.egress_intf_idx = vc_entry.rg_wan_idx;  // Set egress interface.
			}
		}
	}
*/
	rtk_rg_shareMeter_set (qos_entry->entryid,qos_entry->limitSpeed,ENABLED);
	aclRule.qos_actions=ACL_ACTION_SHARE_METER_BIT;
	aclRule.action_share_meter=qos_entry->entryid;


	if(!(fp = fopen(RG_QOS_RULES_FILE, "a")))
	{
		fprintf(stderr, "ERROR! %s\n", strerror(errno));
		return -2;
	}

	if((ret = rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx)) == 0)
		fprintf(fp, "%d\n", aclIdx);
	else
		printf("rtk_rg_aclFilterAndQos_add QoS rule failed! (ret = %d)\n", ret);

	fclose(fp);
	return 0;
}

#ifdef CONFIG_TR142_MODULE
#define TR142_DEV_FILE "/dev/rtk_tr142"
static void setup_pon_queues(unsigned char policy)
{
	MIB_CE_IP_QOS_QUEUE_T qEntry;
	int qEntryNum, i, fd;
	rtk_tr142_qos_queues_t queues = {0};

	fd = open(TR142_DEV_FILE, O_WRONLY);
	if(fd < 0)
	{
		DBPRINT(1, "ERROR: failed to open %s\n", TR142_DEV_FILE);
		return;
	}

	if((qEntryNum = mib_chain_total(MIB_IP_QOS_QUEUE_TBL)) <=0)
	{
		DBPRINT(1, "ERROR: set PON QoS queues failed\n");
		return;
	}

	for(i = 0; i < qEntryNum; i++)
	{
		if(!mib_chain_get(MIB_IP_QOS_QUEUE_TBL, i, (void*)&qEntry))
			continue;

		if(i >= WAN_PONMAC_QUEUE_MAX)
			break;

		queues.queue[i].enable = qEntry.enable;
		queues.queue[i].type = (policy== 0) ? STRICT_PRIORITY : WFQ_WRR_PRIORITY;
		queues.queue[i].weight = (policy== 1) ? qEntry.weight : 0;
	}
	if(ioctl(fd, RTK_TR142_IOCTL_SET_QOS_QUEUES, &queues) != 0)
	{
		DBPRINT(1, "ERROR: set PON QoS queues failed\n");
	}
}

static void clear_pon_queues(void)
{
	MIB_CE_IP_QOS_QUEUE_T qEntry;
	int qEntryNum, i, fd;
	rtk_tr142_qos_queues_t queues = {0};

	fd = open(TR142_DEV_FILE, O_WRONLY);
	if(fd < 0)
	{
		DBPRINT(1, "ERROR: failed to open %s\n", TR142_DEV_FILE);
		return;
	}

	if(ioctl(fd, RTK_TR142_IOCTL_SET_QOS_QUEUES, &queues) != 0)
	{
		DBPRINT(1, "ERROR: set PON QoS queues failed\n");
	}
}
#endif


int RTK_RG_QoS_Queue_Set()
{
	unsigned char policy;
	int aclIdx, i, ret, lanPhyPort;
	rtk_rg_qos_queue_weights_t q_weight;
#if defined(CONFIG_GPON_FEATURE)
	unsigned int pon_mode;
	int wanPhyPort;
#endif


	memset(&q_weight, 0, sizeof(q_weight));

	if(!mib_get(MIB_QOS_POLICY, (void *)&policy))
	{
		DBPRINT(1, "MIB get MIB_QOS_POLICY failed!\n");
		return -2;
	}

	if(policy == 0) // PRIO
	{
		q_weight.weights[6] = 0; // Queue4~7: Strict Priority
		q_weight.weights[5] = 0;
		q_weight.weights[4] = 0;
#if defined(CONFIG_EPON_FEATURE)
		q_weight.weights[3] = 0;
#else
		q_weight.weights[7] = 0;
#endif
	}
	else if(policy ==1) // WRR
	{
		MIB_CE_IP_QOS_QUEUE_T qEntry;
		int qEntryNum, i;

		if((qEntryNum = mib_chain_total(MIB_IP_QOS_QUEUE_TBL)) <=0)
			return -1;

		for(i = 0; i < qEntryNum; i++)
		{
			if(!mib_chain_get(MIB_IP_QOS_QUEUE_TBL, i, (void*)&qEntry))
				continue;
#if defined(CONFIG_EPON_FEATURE)
			if( i <= 4 )
				q_weight.weights[6-i] = qEntry.weight;
#else
			if( i <= 7 )
				q_weight.weights[7-i] = qEntry.weight;
#endif
		}
	}
	else
	{
		DBPRINT(1, "policy=%d: Unexpected policy value! (0=PRIO, 1=WRR)\n", policy);
		return -1;
	}

#if defined(CONFIG_RTK_RG_INIT) && defined(CONFIG_GPON_FEATURE)
	if (mib_get(MIB_PON_MODE, (void *)&pon_mode) == 0)
		printf("get MIB_PON_MODE failed!!!\n");

	if((wanPhyPort = RG_get_wan_phyPortId()) == -1)
		printf("get wan phy port id failed!!!\n");
#endif

	for( i = 0; i < 7; i++ )
	{
#if defined(CONFIG_GPON_FEATURE)
		//In GPON, queue in PON port should be set by OMCI, so ignore it.
		if ((pon_mode==GPON_MODE) && (i==wanPhyPort))
		{
#ifdef CONFIG_TR142_MODULE
			setup_pon_queues(policy);
#endif
			continue;
		}
#endif

        lanPhyPort= RG_get_lan_phyPortId(i);
        if (lanPhyPort < 0 ) continue; //Iulian , port mapping fail in 9602 series
        if((ret = rtk_rg_qosStrictPriorityOrWeightFairQueue_set(lanPhyPort, q_weight)) != 0)
			DBPRINT(1, "rtk_qos_schedulingQueue_set failed! (ret=%d, i=%d)\n", ret, i);
	}

	return 0;
}

int RTK_RG_QoS_Queue_Remove()
{
#if defined(CONFIG_EPON_FEATURE)
	int  i, ret;
	rtk_rg_qos_queue_weights_t q_weight;
#endif
#ifdef CONFIG_TR142_MODULE
	unsigned int pon_mode;

	if (mib_get(MIB_PON_MODE, (void *)&pon_mode) == 0)
		printf("get MIB_PON_MODE failed!!!\n");

	if (pon_mode==GPON_MODE)
		clear_pon_queues();
#endif
#if defined(CONFIG_EPON_FEATURE)
	memset(&q_weight, 0, sizeof(q_weight));
	
	for( i = 0; i < 6; i++ )
	{
		/*reset to default*/
		if((ret = rtk_rg_qosStrictPriorityOrWeightFairQueue_set(i, q_weight)) != 0)
			DBPRINT(1, "rtk_qos_schedulingQueue_set failed! (ret=%d, i=%d)\n", ret, i);
	}

#endif

	return 0;
}


int RTK_RG_QoS_Rule_Set(MIB_CE_IP_QOS_Tp qos_entry)
{
	rtk_rg_aclFilterAndQos_t aclRule;
	MIB_CE_ATM_VC_T vc_entry;
	int aclIdx, ret, i, total_vc, udp_tcp_rule=0;
	FILE *fp;
	ipaddr_t mask;
	unsigned char empty_ipv6[IPV6_ADDR_LEN] = {0};

	memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));

	aclRule.fwding_type_and_direction =  ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET;
	aclRule.action_type = ACL_ACTION_TYPE_QOS;
	aclRule.acl_weight = RG_QOS_ACL_WEIGHT;

	// Source MAC
	if(memcmp(qos_entry->smac, EMPTY_MAC, MAC_ADDR_LEN))
	{
		aclRule.filter_fields |= INGRESS_SMAC_BIT;
		memcpy(&aclRule.ingress_smac, qos_entry->smac, MAC_ADDR_LEN);
	}

	// Destination MAC
	if(memcmp(qos_entry->dmac, EMPTY_MAC, MAC_ADDR_LEN))
	{
		aclRule.filter_fields |= INGRESS_DMAC_BIT;
		memcpy(&aclRule.ingress_dmac, qos_entry->dmac, MAC_ADDR_LEN);
	}

	//By default is filter packets from ALL LAN port.
	aclRule.filter_fields |= INGRESS_PORT_BIT;
	#ifdef CONFIG_RTL9602C_SERIES
	aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0x3);
	#else
	aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0xf);
	#endif

	// Filter rule of physic ports.
	if(qos_entry->phyPort >= 1 && qos_entry->phyPort <= SW_LAN_PORT_NUM)
	{
		aclRule.filter_fields |= INGRESS_PORT_BIT;
		//aclRule.ingress_port_mask.portmask = (1 << RTK_RG_PORT0)  | (1 << RTK_RG_PORT1) | (1 << RTK_RG_PORT2) | (1 << RTK_RG_PORT3);
		aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(1 << (qos_entry->phyPort-1));
	}

	// Filter rule of DSCP
	if(qos_entry->qosDscp != 0)
	{
		aclRule.filter_fields |= INGRESS_DSCP_BIT;
		aclRule.ingress_dscp = qos_entry->qosDscp >> 2;
	}

	// Filter rule of Ether Type
	if(qos_entry->ethType != 0)
	{
		aclRule.filter_fields |= INGRESS_ETHERTYPE_BIT;
		aclRule.ingress_ethertype = qos_entry->ethType;
	}

	// Filter rule of 802.1p mark
	if(qos_entry->vlan1p != 0)
	{
		aclRule.filter_fields |= INGRESS_CTAG_PRI_BIT;
		aclRule.ingress_ctag_pri = qos_entry->vlan1p - 1;
	}

	// Filter rule of Protocol: UDP, TCP, ICMP, UDP and TCP
	if(qos_entry->protoType != PROTO_NONE)
	{
		if(qos_entry->protoType == PROTO_TCP)
			aclRule.filter_fields |= INGRESS_L4_TCP_BIT;
		else if(qos_entry->protoType == PROTO_UDP)
			aclRule.filter_fields |= INGRESS_L4_UDP_BIT;
		else if(qos_entry->protoType == PROTO_ICMP)
			aclRule.filter_fields |= INGRESS_L4_ICMP_BIT;
		else if(qos_entry->protoType == PROTO_UDPTCP){
			aclRule.filter_fields |= INGRESS_L4_UDP_BIT;
			udp_tcp_rule = 1;
		}
		else
		{
			DBPRINT(1, "Add acl rule failed! No support of this protocol type!\n");
			return -1;
		}
	}

add_udp_tcp:
    if(udp_tcp_rule==2){
        aclRule.filter_fields &= ~(INGRESS_L4_UDP_BIT);
        aclRule.filter_fields |= INGRESS_L4_TCP_BIT; //add tcp for udp/tcp protocol
    }

	if(qos_entry->sPort != 0)
	{
		aclRule.filter_fields |= INGRESS_L4_SPORT_RANGE_BIT;
		if (qos_entry->sPort == qos_entry->sPortRangeMax)
		{
			aclRule.ingress_src_l4_port_start = aclRule.ingress_src_l4_port_end = qos_entry->sPort;
		}
		else
		{
			aclRule.ingress_src_l4_port_start = MIN_VALUE(qos_entry->sPort, qos_entry->sPortRangeMax);
			aclRule.ingress_src_l4_port_end = MAX_VALUE(qos_entry->sPort, qos_entry->sPortRangeMax);
		}
	}

	if(qos_entry->dPort != 0)
	{
		aclRule.filter_fields |= INGRESS_L4_DPORT_RANGE_BIT;
		if (qos_entry->dPort == qos_entry->dPortRangeMax)
		{
			aclRule.ingress_dest_l4_port_start = aclRule.ingress_dest_l4_port_end = qos_entry->dPort;
		}
		else
		{
			aclRule.ingress_dest_l4_port_start = MIN_VALUE(qos_entry->dPort, qos_entry->dPortRangeMax);
			aclRule.ingress_dest_l4_port_end = MAX_VALUE(qos_entry->dPort, qos_entry->dPortRangeMax);
		}
	}

#ifdef CONFIG_IPV6
	if(qos_entry->IpProtocol == IPVER_IPV6)  // IPv6
	{
		aclRule.filter_fields |= INGRESS_IPV6_TAGIF_BIT;
		aclRule.ingress_ipv6_tagif = 1;
		// Source ip, prefix
		if(memcmp(qos_entry->sip6, empty_ipv6, IPV6_ADDR_LEN) != 0)
		{
			aclRule.filter_fields |= INGRESS_IPV6_SIP_RANGE_BIT;

			IPv6PrefixToStartEnd(qos_entry->sip6, qos_entry->sip6PrefixLen, aclRule.ingress_src_ipv6_addr_start, aclRule.ingress_src_ipv6_addr_end);
		}

		// Destination ip, prefix
		if(memcmp(qos_entry->dip6, empty_ipv6, IPV6_ADDR_LEN) != 0)
		{
			aclRule.filter_fields |= INGRESS_IPV6_DIP_RANGE_BIT;

			IPv6PrefixToStartEnd(qos_entry->dip6, qos_entry->dip6PrefixLen, aclRule.ingress_dest_ipv6_addr_start, aclRule.ingress_dest_ipv6_addr_end);
		}
	}
	else if(qos_entry->IpProtocol == IPVER_IPV4) // IPv4
	{
#endif
		aclRule.filter_fields |= INGRESS_IPV4_TAGIF_BIT;
		aclRule.ingress_ipv4_tagif = 1;
		// Source ip, mask
		if(memcmp(qos_entry->sip, "\x00\x00\x00\x00", IP_ADDR_LEN) != 0)
		{
			aclRule.filter_fields |= INGRESS_IPV4_SIP_RANGE_BIT;

			if(qos_entry->smaskbit == 0)
				aclRule.ingress_src_ipv4_addr_start = aclRule.ingress_src_ipv4_addr_end = *((ipaddr_t *)qos_entry->sip);
			else
			{
				mask = ~0 << (sizeof(ipaddr_t)*8 - qos_entry->smaskbit);
				mask = htonl(mask);
				aclRule.ingress_src_ipv4_addr_start = *((in_addr_t *)qos_entry->sip) & mask;
				aclRule.ingress_src_ipv4_addr_end = *((in_addr_t *)qos_entry->sip) | ~mask;
			}
		}

		// Destination ip, mask
		if(memcmp(qos_entry->dip, "\x00\x00\x00\x00", IP_ADDR_LEN) != 0)
		{
			aclRule.filter_fields |= INGRESS_IPV4_DIP_RANGE_BIT;
			aclRule.filter_fields |= INGRESS_IPV4_TAGIF_BIT;
			aclRule.ingress_ipv4_tagif = 1;

			if(qos_entry->dmaskbit == 0)
				aclRule.ingress_dest_ipv4_addr_start = aclRule.ingress_dest_ipv4_addr_end = *((ipaddr_t *)qos_entry->dip);
			else
			{
				mask = ~0 << (sizeof(ipaddr_t)*8 - qos_entry->dmaskbit);
				mask = htonl(mask);
				aclRule.ingress_dest_ipv4_addr_start = *((in_addr_t *)qos_entry->dip) & mask;
				aclRule.ingress_dest_ipv4_addr_end = *((in_addr_t *)qos_entry->dip) | ~mask;
			}
		}
#ifdef CONFIG_IPV6
	}
#endif

// NOT support WANInterface now!
/*
	// Filter rule of WAN interface
	if(qos_entry->outif != DUMMY_IFINDEX)
	{
		total_vc = mib_chain_total(MIB_ATM_VC_TBL);

		for( i = 0; i < total_vc; i++ )
		{
			if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&vc_entry))
				continue;

			if(vc_entry.ifIndex == qos_entry->outif)
			{
				aclRule.filter_fields |= EGRESS_INTF_BIT;
				aclRule.egress_intf_idx = vc_entry.rg_wan_idx;  // Set egress interface.
			}
		}
	}
*/

	// Action rule of DSCP
	if(qos_entry->m_dscp != 0)
	{
		aclRule.qos_actions |= ACL_ACTION_DSCP_REMARKING_BIT;
		aclRule.action_dscp_remarking_pri = qos_entry->m_dscp >> 2;
	}

	// Action rule of IP precedence.
	if(qos_entry->prior != 0)
	{
		MIB_CE_IP_QOS_QUEUE_T qEntry;
		int qEntryNum, i;

		if(!mib_chain_get(MIB_IP_QOS_QUEUE_TBL, qos_entry->prior-1, (void*)&qEntry))
			return -1;

		if(qEntry.enable)
		{
			aclRule.qos_actions |= ACL_ACTION_ACL_PRIORITY_BIT;
#if defined(CONFIG_EPON_FEATURE)
			aclRule.action_acl_priority = 7 - qos_entry->prior;
#else
			aclRule.action_acl_priority = 8 - qos_entry->prior;
#endif
		}
	}

	// Action rule of CVLAN priority change.
	if(qos_entry->m_1p != 0)
	{
		aclRule.qos_actions |= ACL_ACTION_1P_REMARKING_BIT;
		aclRule.action_dot1p_remarking_pri = qos_entry->m_1p - 1;
	}

	if(!(fp = fopen(RG_QOS_RULES_FILE, "a")))
	{
		fprintf(stderr, "ERROR! %s\n", strerror(errno));
		return -2;
	}

	if((ret = rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx)) == 0)
		fprintf(fp, "%d\n", aclIdx);
	else
		printf("rtk_rg_aclFilterAndQos_add QoS rule failed! (ret = %d)\n", ret);

    if(udp_tcp_rule==1){
        udp_tcp_rule = 2;
        goto add_udp_tcp;
    }

	fclose(fp);
	return 0;
}

int FlushRTK_RG_QoS_Rules()
{
	FILE *fp;
	int qos_idx;

	if(!(fp = fopen(RG_QOS_RULES_FILE, "r")))
		return -2;

	while(fscanf(fp, "%d\n", &qos_idx) != EOF)
	{
		if(rtk_rg_aclFilterAndQos_del(qos_idx))
			DBPRINT(1, "rtk_rg_aclFilterAndQos_del failed! idx = %d\n", qos_idx);
	}

	fclose(fp);
	unlink(RG_QOS_RULES_FILE);
	return 0;
}
#endif

#ifdef CONFIG_USER_MINIUPNPD
int AddRTK_RG_UPnP_Connection(unsigned short eport, const char *iaddr, unsigned short iport, int protocol)
{
	int upnpIdx, ret, total_vc, i;
	rtk_rg_upnpConnection_t upnp;
	MIB_CE_ATM_VC_T vc_entry;
	FILE *fp;
	unsigned int ext_if;
	char lan_ip[IP_ADDR_LEN];

	mib_get(MIB_UPNP_EXT_ITF, (void *)&ext_if);
	inet_pton(AF_INET, iaddr, (void *)lan_ip);
	memset(&upnp, 0, sizeof(rtk_rg_upnpConnection_t));

	upnp.valid = ENABLED;
	upnp.is_tcp = (protocol == IPPROTO_TCP? 1: 0);
	upnp.gateway_port = eport;
	upnp.local_ip = *((ipaddr_t *)lan_ip);
	upnp.local_port = iport;
	upnp.limit_remote_ip = DISABLED;
	upnp.limit_remote_port = DISABLED;
	upnp.type = UPNP_TYPE_PERSIST;
	upnp.timeout = 0; // 0: disable auto-delete

	total_vc = mib_chain_total(MIB_ATM_VC_TBL);

	for( i = 0; i < total_vc; i++ )
	{
		if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&vc_entry))
			continue;

		if(vc_entry.ifIndex == ext_if)
		{
			upnp.wan_intf_idx = vc_entry.rg_wan_idx;
			break;
		}
	}

	if(!(fp = fopen(RG_UPNP_CONNECTION_FILE, "a")))
	{
		fprintf(stderr, "ERROR! %s\n", strerror(errno));
		return -2;
	}

	if((ret = rtk_rg_upnpConnection_add(&upnp, &upnpIdx)) == 0)
		fprintf(fp, "%d %u %d\n", upnpIdx, eport, protocol);
	else
		printf("rtk_rg_upnpConnection_add rule failed!\n");

	fclose(fp);
	return 0;
}

int DelRTK_RG_UPnP_Connection(unsigned short eport, int protocol)
{
	FILE *fp, *fp_tmp;
	int upnp_idx, upnp_eport, upnp_proto;
	char line[24];

	if(!(fp = fopen(RG_UPNP_CONNECTION_FILE, "r")))
		return -2;

	if(!(fp_tmp = fopen(RG_UPNP_TMP_FILE, "w")))
		return -2;

	while(fgets(line, 23, fp) != NULL)
	{
		sscanf(line, "%d %d %d\n", &upnp_idx, &upnp_eport, &upnp_proto);

		if( upnp_eport == eport && upnp_proto == protocol )
		{
			if(rtk_rg_upnpConnection_del(upnp_idx))
				DBPRINT(1, "rtk_rg_upnpConnection_del failed! idx = %d\n", upnp_idx);
		}
		else
			fprintf(fp_tmp, "%d %d %d\n", upnp_idx, upnp_eport, upnp_proto);
	}

	fclose(fp);
	fclose(fp_tmp);
	unlink(RG_UPNP_CONNECTION_FILE);
	rename(RG_UPNP_TMP_FILE, RG_UPNP_CONNECTION_FILE);
	return 0;
}
#endif

#ifdef CONFIG_IP_NF_ALG_ONOFF
int RTK_RG_ALG_Set()
{
	rtk_rg_alg_type_t alg_app = 0;

	unsigned char value;
#ifdef CONFIG_NF_CONNTRACK_FTP
	if(mib_get(MIB_IP_ALG_FTP, &value) && value == 1)
		alg_app |= RTK_RG_ALG_FTP_TCP_BIT | RTK_RG_ALG_FTP_UDP_BIT;
#endif
#ifdef CONFIG_NF_CONNTRACK_H323
	if(mib_get(MIB_IP_ALG_H323, &value) && value == 1)
		alg_app |= RTK_RG_ALG_H323_TCP_BIT | RTK_RG_ALG_H323_UDP_BIT;
#endif
#ifdef CONFIG_NF_CONNTRACK_RTSP
	if(mib_get(MIB_IP_ALG_RTSP, &value) && value == 1)
		alg_app |= RTK_RG_ALG_RTSP_TCP_BIT | RTK_RG_ALG_RTSP_UDP_BIT;
#endif
#ifdef CONFIG_NF_CONNTRACK_L2TP
	if(mib_get(MIB_IP_ALG_L2TP, &value) && value == 1)
		alg_app |= RTK_RG_ALG_L2TP_TCP_PASSTHROUGH_BIT | RTK_RG_ALG_L2TP_UDP_PASSTHROUGH_BIT;
#endif
#ifdef CONFIG_NF_CONNTRACK_IPSEC
	if(mib_get(MIB_IP_ALG_IPSEC, &value) && value == 1)
		alg_app |= RTK_RG_ALG_IPSEC_TCP_PASSTHROUGH_BIT | RTK_RG_ALG_IPSEC_UDP_PASSTHROUGH_BIT;
#endif
#ifdef CONFIG_NF_CONNTRACK_SIP
	if(mib_get(MIB_IP_ALG_SIP, &value) && value == 1)
		alg_app |= RTK_RG_ALG_SIP_TCP_BIT | RTK_RG_ALG_SIP_UDP_BIT;
#endif
#ifdef CONFIG_NF_CONNTRACK_PPTP
	if(mib_get(MIB_IP_ALG_PPTP, &value) && value == 1)
		alg_app |= RTK_RG_ALG_PPTP_TCP_PASSTHROUGH_BIT | RTK_RG_ALG_PPTP_UDP_PASSTHROUGH_BIT;
#endif

	if(rtk_rg_algApps_set(alg_app))
	{
		DBPRINT(1, "rtk_rg_aclFilterAndQos_del failed! alg_app = %X\n", alg_app);
		return -1;
	}

	return 0;
}
#endif

#ifdef URL_BLOCKING_SUPPORT
int RTK_RG_URL_Filter_Set()
{
	int url_idx, ret, total_url, total_keyd, i;
	rtk_rg_urlFilterString_t url_f_s;
	MIB_CE_URL_FQDN_T fqdn;
	MIB_CE_KEYWD_FILTER_T keyword;
	FILE *fp;

	total_url = mib_chain_total(MIB_URL_FQDN_TBL);

	if(!(fp = fopen(RG_URL_FILTER_FILE, "a")))
	{
		fprintf(stderr, "ERROR! %s\n", strerror(errno));
		return -2;
	}

	url_f_s.path_exactly_match = 0;

	for (i = 0; i < total_url; i++)
	{
		if (!mib_chain_get(MIB_URL_FQDN_TBL, i, (void *)&fqdn))
			continue;

		strncpy(url_f_s.url_filter_string, fqdn.fqdn, MAX_URL_LENGTH);

		if((ret = rtk_rg_urlFilterString_add(&url_f_s, &url_idx)) == 0)
			fprintf(fp, "%d\n", url_idx);
		else
			DBPRINT(1, "rtk_rg_urlFilterString_add QoS rule failed!\n");
	}

	total_keyd = mib_chain_total(MIB_KEYWD_FILTER_TBL);

	for(i = 0; i < total_keyd; i++)
	{
		if(!mib_chain_get(MIB_KEYWD_FILTER_TBL, i, (void *)&keyword))
		 continue;

		strncpy(url_f_s.url_filter_string, keyword.keyword, MAX_KEYWD_LENGTH);

		if((ret = rtk_rg_urlFilterString_add(&url_f_s, &url_idx)) == 0)
			fprintf(fp, "%d\n", url_idx);
		else
			DBPRINT(1, "rtk_rg_urlFilterString_add QoS rule failed!\n");
	}

	fclose(fp);
	return 0;
}

int Flush_RTK_RG_URL_Filter()
{
	FILE *fp;
	int url_filter_idx;

	if(!(fp = fopen(RG_URL_FILTER_FILE, "r")))
		return -2;

	while(fscanf(fp, "%d\n", &url_filter_idx) != EOF)
	{
		if(rtk_rg_urlFilterString_del(url_filter_idx))
			DBPRINT(1, "rtk_rg_urlFilterString_del failed! idx = %d\n", url_filter_idx);
	}

	fclose(fp);
	unlink(RG_URL_FILTER_FILE);
	return 0;
}
#endif

#if 0
#define CALLBACKREGIST  "/proc/rg/callbackRegist"
int callbackRegistCheck(void)
{
	FILE *fp;
	int proc_read;
	int enabled=0;
	char buffer[8];
	fp=fopen(CALLBACKREGIST, "r");
	if(fp==NULL)
	{
		printf("file %s open failed @%s line:%d\n", CALLBACKREGIST,__func__,__LINE__);
		return FALSE;
	}
	memset(buffer, 0, sizeof(buffer));
	proc_read=fread(buffer, 1, 8, fp);
	if(proc_read!=0){
		printf("read size=%d, buffer=%s\n", proc_read, buffer);
	}
	if(proc_read < 0){
		printf("proc_read failed @%s line:%d\n",__func__,__LINE__);
		goto err;
	}

	if(strncmp(buffer, "1",1) == 0){
		enabled = TRUE;
	}else if(strncmp(buffer, "0",1) == 0){
		enabled = FALSE;
	}
err:
	fclose(fp);

	return enabled;
}
#endif

int rg_eth2wire_block(int enable)
{
	rtk_rg_port_isolation_t isoset;
	int idx,phyPortId;

	// block from lan port
	for(idx = 0 ; idx < SW_LAN_PORT_NUM; idx++)
	{
		if(rtk_rg_switch_phyPortId_get(idx, &phyPortId) == 0)
			isoset.port = phyPortId;
		else
		{
			DBPRINT(1, "Get LAN port(%d) to phy port failed\n", idx);
			isoset.port = idx;
		}
		if(enable) isoset.portmask.portmask = 0x5f; // clear bit 7/8
		else isoset.portmask.portmask = 0xfff;
		if(rtk_rg_portIsolation_set(isoset) != 0)
			DBPRINT(1, "set LAN port(%d) to wireless blocking failed\n", idx);
	}
	// block from extport (port 7/8)
	isoset.port = 7;
	if(enable) isoset.portmask.portmask = 0xff0;
	else isoset.portmask.portmask = 0xfff;
	if(rtk_rg_portIsolation_set(isoset) != 0)
		DBPRINT(1, "set EXT port(7) to LAN blocking failed\n");
	#if defined(WLAN_DUALBAND_CONCURRENT)
	isoset.port = 8;
	if(enable) isoset.portmask.portmask = 0xff0;
	else isoset.portmask.portmask = 0xfff;
	if(rtk_rg_portIsolation_set(isoset) != 0)
		DBPRINT(1, "set EXT port(8) to LAN blocking failed\n");
	#endif

	return 0;
}

#ifdef DOS_SUPPORT
int RTK_RG_DoS_Set(int enable)
{
	rtk_rg_mac_portmask_t dos_port_mask;
	int wanPhyPort;
	int floodCount;
	int floodTh;

	if(!(enable & DOS_ENABLE)){
		printf("rg DoS: disable\n");
		dos_port_mask.portmask = 0x0;
		rtk_rg_dosPortMaskEnable_set(dos_port_mask);
	}
	else{
		printf("rg DoS: enable\n");
		#if 0
		if(mib_get(MIB_WAN_PHY_PORT , (void *)&wanPhyPort) == 0){
			printf("get MIB_WAN_PHY_PORT failed!!!\n");
			wanPhyPort=RTK_RG_MAC_PORT3 ; //for 0371 default
		}
		dos_port_mask.portmask = 1 << wanPhyPort;
		#endif
		dos_port_mask.portmask = RG_get_wan_phyPortMask();
		rtk_rg_dosPortMaskEnable_set(dos_port_mask);
	}

	if(enable & SYSFLOODSYN){
		if(mib_get(MIB_DOS_SYSSYN_FLOOD, (void *)&floodCount)!=0){
			floodTh = 1000/floodCount;
			if(rtk_rg_dosFloodType_set(RTK_RG_DOS_SYNFLOOD_DENY, 1,RTK_RG_DOS_ACTION_DROP,floodTh)) //time unit 1ms
				DBPRINT(1, "rtk_rg_dosFloodType_set failed! type = RTK_RG_DOS_SYNFLOOD_DENY\n");
		}
		else
			DBPRINT(1, "MIB_DOS_SYSSYN_FLOOD get failed! \n");
	}

	if(enable & SYSFLOODFIN){
		if(mib_get(MIB_DOS_SYSFIN_FLOOD, (void *)&floodCount)!=0){
			floodTh = 1000/floodCount;
			if(rtk_rg_dosFloodType_set(RTK_RG_DOS_FINFLOOD_DENY, 1,RTK_RG_DOS_ACTION_DROP,floodTh)) //time unit 1ms
				DBPRINT(1, "rtk_rg_dosFloodType_set failed! type = RTK_RG_DOS_FINFLOOD_DENY\n");
		}
		else
			DBPRINT(1, "MIB_DOS_SYSFIN_FLOOD get failed! \n");
	}

	if(enable & SYSFLOODICMP){
		if(mib_get(MIB_DOS_SYSICMP_FLOOD, (void *)&floodCount)!=0){
			floodTh = 1000/floodCount;
			if(rtk_rg_dosFloodType_set(RTK_RG_DOS_ICMPFLOOD_DENY,1,RTK_RG_DOS_ACTION_DROP,floodTh)) //time unit 1ms
				DBPRINT(1, "rtk_rg_dosFloodType_set failed! type = RTK_RG_DOS_ICMPFLOOD_DENY\n");
		}
		else
			DBPRINT(1, "MIB_DOS_SYSICMP_FLOOD get failed! \n");
	}

	if(rtk_rg_dosType_set(RTK_RG_DOS_LAND_DENY,(enable & IPLANDENABLED)? 1:0,RTK_RG_DOS_ACTION_DROP))
		DBPRINT(1, "rtk_rg_dosType_set failed! type = RTK_RG_DOS_LAND_DENY\n");

	if(rtk_rg_dosType_set(RTK_RG_DOS_POD_DENY,(enable & PINGOFDEATHENABLED)? 1:0,RTK_RG_DOS_ACTION_DROP))
		DBPRINT(1, "rtk_rg_dosType_set failed! type = RTK_RG_DOS_POD_DENY\n");


	if(enable & TCPSCANENABLED){
		if(rtk_rg_dosType_set(RTK_RG_DOS_SYNFIN_DENY,ENABLED,RTK_RG_DOS_ACTION_DROP))
			DBPRINT(1, "rtk_rg_dosType_set failed! type = RTK_RG_DOS_SYNFIN_DENY\n");

		if(rtk_rg_dosType_set(RTK_RG_DOS_XMA_DENY,ENABLED,RTK_RG_DOS_ACTION_DROP))
			DBPRINT(1, "rtk_rg_dosType_set failed! type = RTK_RG_DOS_XMA_DENY\n");

		if(rtk_rg_dosType_set(RTK_RG_DOS_NULLSCAN_DENY,ENABLED,RTK_RG_DOS_ACTION_DROP))
			DBPRINT(1, "rtk_rg_dosType_set failed! type = RTK_RG_DOS_NULLSCAN_DENY\n");
	}
	else{
		if(rtk_rg_dosType_set(RTK_RG_DOS_SYNFIN_DENY,DISABLED,RTK_RG_DOS_ACTION_DROP))
			DBPRINT(1, "rtk_rg_dosType_set failed! type = RTK_RG_DOS_SYNFIN_DENY\n");

		if(rtk_rg_dosType_set(RTK_RG_DOS_XMA_DENY,DISABLED,RTK_RG_DOS_ACTION_DROP))
			DBPRINT(1, "rtk_rg_dosType_set failed! type = RTK_RG_DOS_XMA_DENY\n");

		if(rtk_rg_dosType_set(RTK_RG_DOS_NULLSCAN_DENY,DISABLED,RTK_RG_DOS_ACTION_DROP))
			DBPRINT(1, "rtk_rg_dosType_set failed! type = RTK_RG_DOS_NULLSCAN_DENY\n");
	}

	if(rtk_rg_dosType_set(RTK_RG_DOS_SYNWITHDATA_DENY,(enable & TCPSynWithDataEnabled)? 1:0,RTK_RG_DOS_ACTION_DROP))
		DBPRINT(1, "rtk_rg_dosType_set failed! type = RTK_RG_DOS_SYNWITHDATA_DENY\n");

	return 0;
}
#endif

unsigned int RG_get_lan_phyPortMask(unsigned int portmask)
{
	int i=0, phyPortId, ret;
	unsigned int phyportmask=0;

	for(i=0;i<SW_LAN_PORT_NUM;i++)
	{
		if((portmask>>i) & 1){
			ret = rtk_rg_switch_phyPortId_get(i, &phyPortId);
			if(ret == 0)
				phyportmask |= (1 << phyPortId);
			else
				DBPRINT(1, "%s rtk_rg_switch_phyPortId_get id %d failed!\n", __FUNCTION__, i);
		}
	}

#ifdef CONFIG_RGMII_RESET_PROCESS
	phyportmask |= (1 << RTK_RG_MAC_PORT_RGMII);
#endif

	return phyportmask;
}

unsigned int RG_get_wan_phyPortMask()
{
	int phyPortId, ret, logPortId;
	unsigned int phyportmask=0;

	logPortId = RTK_PORT_PON;

	ret = rtk_rg_switch_phyPortId_get(logPortId, &phyPortId);

	if(ret == 0)
		phyportmask |= (1 << phyPortId);
	else
		DBPRINT(1, "%s rtk_rg_switch_phyPortId_get failed!\n", __FUNCTION__);

	return phyportmask;
}

int RG_get_lan_phyPortId(int logPortId)
{
	int phyPortId, ret;

	ret = rtk_rg_switch_phyPortId_get(logPortId, &phyPortId);

	if(ret == 0)
		return phyPortId;
	else{
		DBPRINT(1, "%s rtk_rg_switch_phyPortId_get failed!\n", __FUNCTION__);
		return -1;
	}
}

int RG_get_wlan_phyPortId(int logPortId)
{
	int phyPortId, ret;
	phyPortId = 7; /*wlan phy port is fixed to 7*/
	return phyPortId;
}
int RG_get_wan_phyPortId()
{
	int phyPortId, ret, logPortId;

	logPortId = RTK_PORT_PON;
	ret = rtk_rg_switch_phyPortId_get(logPortId, &phyPortId);

	if(ret == 0)               
		return phyPortId;
	else{
		DBPRINT(1, "%s rtk_rg_switch_phyPortId_get failed!\n", __FUNCTION__);
		return -1;
	}
}

unsigned int RG_get_portCounter(unsigned int portIndex,unsigned long *tx_pkts,unsigned long *tx_drops,unsigned long *tx_errs,
										unsigned long *rx_pkts,unsigned long *rx_drops,unsigned long *rx_errs)
{
	rtk_rg_port_mib_info_t portmib;
	int ret;

	ret = rtk_rg_portMibInfo_get(RG_get_lan_phyPortId(portIndex),&portmib);
	if(ret != 0)
	{
		DBPRINT(1, "%s get port %d mib info failed!\n", __FUNCTION__, portIndex);
		return 0;
	}

	*rx_pkts = (portmib.ifInUcastPkts + portmib.ifInMulticastPkts + portmib.ifInBroadcastPkts);
	*rx_drops = portmib.dot1dTpPortInDiscards;
	*rx_errs = (portmib.dot3StatsSymbolErrors + portmib.dot3ControlInUnknownOpcodes);
	*tx_pkts = (portmib.ifOutUcastPkts + portmib.ifOutMulticastPkts + portmib.ifOutBrocastPkts);
	*tx_drops = portmib.ifOutDiscards ;
	*tx_errs = 0;
	return 1;
}

unsigned int RG_clear_portCounter(unsigned int portIndex)
{
	int ret;
	
	ret = rtk_rg_portMibInfo_clear(RG_get_lan_phyPortId(portIndex));
	if(ret != 0)
	{
		DBPRINT(1, "%s get port %d mib info failed!\n", __FUNCTION__, portIndex);
		return 0;
	}
	return 1;
}

int RG_wan_phy_force_power_down(int enabled)
{
	uint32 reg;
	rtk_rg_mac_port_idx_t port = 0;

	for(port = 0 ; port < RTK_RG_MAC_PORT_MAX ; port++)
	{
		rtk_rg_port_phyReg_get(port, 0xbc0, 19, &reg);
		if(enabled)
			reg = reg | 0x10;
		else
			reg = reg & 0xFFEF;
		rtk_rg_port_phyReg_set(port, 0xbc0, 19, reg);
	}
}

void RTK_RG_gatewayService_add()
{
	rtk_rg_gatewayServicePortEntry_t serviceEntry;
	int ret, i=0, index;
	int port_service_num[] = {53, 123, 5060, 67}; //DNS, SNTP, SIP, DHCP server port
	FILE *fp;
	int port;


	if(!(fp = fopen(RG_GATEWAY_SERVICE_FILE, "a")))
	{
		fprintf(stderr, "Open %s failed! %s\n", RG_GATEWAY_SERVICE_FILE, strerror(errno));
	}

	for(i=0; i<(sizeof(port_service_num)/sizeof(port_service_num[0])); i++){
		serviceEntry.valid = 1;
		serviceEntry.port_num = port_service_num[i];
		serviceEntry.type = GATEWAY_SERVER_SERVICE;

		if((ret = rtk_rg_gatewayServicePortRegister_add(&serviceEntry, &index)) == 0)
			fprintf(fp, "%d\n", index);
		else
			DBPRINT(1, "%s idx %d ret = %d rtk_rg_gatewayServicePortRegister_add failed!\n", __FUNCTION__, i, ret);
	}

#ifdef CONFIG_USER_CWMP_TR069
	//Add TR-069 http server
	mib_get(CWMP_CONREQ_PORT, &port);
	serviceEntry.valid = 1;
	serviceEntry.port_num = port;
	serviceEntry.type = GATEWAY_SERVER_SERVICE;
	if((ret = rtk_rg_gatewayServicePortRegister_add(&serviceEntry, &index)) == 0)
			fprintf(fp, "%d\n", index);
	else
		DBPRINT(1, "%s: add cwmp port via rtk_rg_gatewayServicePortRegister_add failed! ret = %d!\n", __FUNCTION__, ret);
#endif
}

void Flush_RTK_RG_gatewayService()
{
	int i=0;
	FILE *fp;

	if(!(fp = fopen(RG_GATEWAY_SERVICE_FILE, "r")))
	{
		fprintf(stderr, "Open %s failed! %s\n", RG_GATEWAY_SERVICE_FILE, strerror(errno));
	}


	while(fscanf(fp, "%d\n", &i) != EOF)
	{
		if(rtk_rg_gatewayServicePortRegister_del(i))
			DBPRINT(1, "rtk_rg_gatewayServicePortRegister_del failed! idx = %d\n", i);
	}

	fclose(fp);
	unlink(RG_GATEWAY_SERVICE_FILE);

}
#if 1
void RTK_Setup_Storm_Control(void)
{
	int portId;
	int portNum;
	unsigned int meterId;
	rtk_switch_devInfo_t	tDevInfo;
	rtk_rate_storm_group_ctrl_t	stormCtrl;
	rtk_rg_switch_deviceInfo_get (&tDevInfo);
	meterId  = tDevInfo.capacityInfo.max_num_of_metering - 1;
AUG_PRT("%s-%d meterId=%d\n",__func__,__LINE__,meterId);
	#ifdef CONFIG_RTL9600_SERIES
	//6266
		rtk_rg_rate_shareMeterMode_set (meterId, METER_MODE_BIT_RATE);
		rtk_rg_rate_shareMeter_set (meterId, 7000, DISABLED);
	#else
		rtk_rg_rate_shareMeterMode_set (meterId, METER_MODE_PACKET_RATE);
		rtk_rg_rate_shareMeter_set (meterId, 10000, DISABLED);
	#endif
	
	rtk_rg_rate_stormControlEnable_get(&stormCtrl);
	stormCtrl.unknown_unicast_enable = ENABLED;
	stormCtrl.broadcast_enable = ENABLED;
	stormCtrl.unknown_multicast_enable  = ENABLED;
	rtk_rg_rate_stormControlEnable_set (&stormCtrl);
#if defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9607) || defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9607_IAD_V00) || defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9607_IAD_V01)
	portNum = 4; //0,1,2,3  4(wan) 
#elif defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9602B) || defined(CONFIG_RTL9602C_SERIES)
	portNum = 2; //0,1  2(wan) 
#endif
	AUG_PRT("%s-%d portNum=%d\n",__func__,__LINE__,portNum);

	/*portId for all LAN + WAN port*/
	for(portId = 0 ; portId <= portNum; portId ++)
	{
		rtk_rg_rate_stormControlMeterIdx_set(portId, STORM_GROUP_UNKNOWN_UNICAST, meterId);
		rtk_rg_rate_stormControlMeterIdx_set (portId, STORM_GROUP_BROADCAST, meterId);
		rtk_rg_rate_stormControlMeterIdx_set (portId, STORM_GROUP_UNKNOWN_MULTICAST, meterId);
		rtk_rg_rate_stormControlPortEnable_set (portId, STORM_GROUP_UNKNOWN_UNICAST, ENABLED);
		rtk_rg_rate_stormControlPortEnable_set (portId, STORM_GROUP_BROADCAST, ENABLED);
		rtk_rg_rate_stormControlPortEnable_set (portId, STORM_GROUP_UNKNOWN_MULTICAST, ENABLED);
	}



}
#endif

