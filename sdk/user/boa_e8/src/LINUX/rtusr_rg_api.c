#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <memory.h>
#include <netinet/in.h>
#include <sys/socket.h>
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
const char RG_ACL_PPPoE_PASS_RULES_FILE[] = "/var/rg_acl_PPPoE_pass_rules_idx";
const char RG_ACL_IPv6_RULES_FILE[] = "/var/rg_acl_ipv6_rules_idx";
const char RG_ACL_BRIDGE_IPv4IPv6_FILTER_RULES_FILE[] = "/var/rg_acl_bridge_ipv4ipv6_filter_rules_idx";
const char RG_QOS_RULES_FILE[] = "/var/rg_acl_qos_idx";
const char RG_QOS_RTP_RULES_FILE[] = "/var/rg_acl_qos_rtp_idx";
const char RG_PPPOEPROXY_RULES_FILE[] = "/var/rg_acl_pppoeproxy_idx";
const char RG_ACL_MVLAN_RULES_FILE[] = "/var/rg_acl_mvlan_rules_idx";

const char RG_UPNP_CONNECTION_FILE[] = "/var/rg_upnp_idx";
const char RG_UPNP_TMP_FILE[] = "/var/rg_upnp_tmp";
const char RG_VERTUAL_SERVER_FILE[] = "/var/rg_vertual_servers_idx";
const char RG_VIRTUAL_SERVER_FILE[] = "/var/rg_virtual_servers_idx";
const char RG_VIRTUAL_SERVER_IP_FILE[] = "/var/rg_virtual_servers_ip";
const char RG_DMZ_FILE[] = "/var/rg_dmz_info";
const char RG_URL_FILTER_FILE[] = "/var/rg_url_filter_idx";
const char MER_GWINFO_B[] = "/tmp/MERgw";
const char WAN_INTERFACE_TMP[] = "/var/wan_interface_tmp";
const char RG_GATEWAY_SERVICE_FILE[] = "/var/rg_gatewayService_idx";
const char RG_IPV6_Bridge_From_Wan_ACL_RILES[] = "/var/rg_ipv6_bridge_from_wan";
const char RG_IPV4_Bridge_From_Wan_ACL_RILES[] = "/var/rg_ipv4_bridge_from_wan";
const char RG_IPV6_PPPoE_From_Wan_KeepOVID_ACL_RILES[] = "/var/rg_ipv6_pppoe_from_wan";
const char RG_IPV4_PPPoE_From_Wan_KeepOVID_ACL_RILES[] = "/var/rg_ipv4_pppoe_from_wan";

const char RG_PATCH_FOR_AVALANCHE[] = "/var/rg_patch_for_avalanche";
const char RG_ALG_FILTER_FILE[] = "/var/rg_alg_filter_idx";
const char RG_DOS_FILTER_FILE[] = "/var/rg_dos_filter_idx";

const char RG_BRIDGE_INET_DHCP_RA_FILTER_FILE[] = "/var/rg_bridge_inet_dhcp_ra_filter_idx";

const char RG_INTERNET_ACCESS_RIGHT_RULES_FILE[] = "/var/rg_internet_access_right_rules_idx";
const char RG_MAX_US_BANDWIDTH_FILE[] = "/var/rg_max_us_bandwidth_idx";
const char RG_MAX_DS_BANDWIDTH_FILE[] = "/var/rg_max_ds_bandwidth_idx";

#define UntagCPort 1
#define TagCPort 0
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
#define MAX_VALUE(val1, val2) (val1>val2?val1:val2)
#define MIN_VALUE(val1, val2) (val1<val2?val1:val2)
int RG_WAN_Interface_Del(unsigned int rg_wan_idx);
unsigned int RG_get_lan_phyPortMask(unsigned int portmask);
unsigned int RG_get_wan_phyPortMask();

int patch_for_avalanche=0;

int Init_rg_api()
{
	int ret;
	unsigned char mbtd;
	rtk_rg_initParams_t init_param;
	unsigned int vid;

	bzero(&init_param, sizeof(rtk_rg_initParams_t));
	printf("init mac based tag des\n");

	mib_get(MIB_MAC_BASED_TAG_DECISION, (void *)&mbtd);
	init_param.macBasedTagDecision = mbtd;

#ifdef CONFIG_LUNA
#if defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)
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
	if(patch_for_avalanche)
	{
		//#turn_off_congestion_ctrl
		//#turn it on after rg init~
		// write proc interface(/proc/rg/bridgeWan_drop_by_protocal)
		FILE *fp;
		fp = fopen("/proc/rg/tcp_hw_learning_at_syn", "w");
		if(fp)
		{
			fprintf(fp, "0\n");
			fclose(fp);
		}else
			fprintf(stderr, "open /proc/rg/tcp_hw_learning_at_syn fail!\n");

		usleep(300000);
		fp = fopen("/proc/rg/tcp_in_shortcut_learning_at_syn", "w");
		if(fp)
		{
			fprintf(fp, "0\n");
			fclose(fp);
		}else
			fprintf(stderr, "open /proc/rg/tcp_in_shortcut_learning_at_syn fail!\n");

		usleep(300000);
		fp = fopen("/proc/rg/trap_syn_and_disable_svlan", "w");
		if(fp)
		{
			fprintf(fp, "0\n");
			fclose(fp);
		}else
			fprintf(stderr, "open /proc/rg/trap_syn_and_disable_svlan fail!\n");

		usleep(300000);
		fp = fopen("/proc/rg/congestion_ctrl_port_mask", "w");
		if(fp)
		{
			fprintf(fp, "0x0\n");
			fclose(fp);
		}else
			fprintf(stderr, "open /proc/rg/congestion_ctrl_port_mask fail!\n");

		usleep(300000);
		fp = fopen("/proc/rg/congestion_ctrl_send_remainder_in_next_gap", "w");
		if(fp)
		{
			fprintf(fp, "0\n");
			fclose(fp);
		}else
			fprintf(stderr, "open /proc/rg/congestion_ctrl_send_remainder_in_next_gap fail!\n");

		usleep(300000);
		fp = fopen("/proc/rg/congestion_ctrl_inbound_ack_to_high_queue", "w");
		if(fp)
		{
			fprintf(fp, "0\n");
			fclose(fp);
		}else
			fprintf(stderr, "open /proc/rg/congestion_ctrl_inbound_ack_to_high_queue fail!\n");

/*
		system("echo 0 > /proc/rg/tcp_hw_learning_at_syn");
		system("echo 0 > /proc/rg/tcp_in_shortcut_learning_at_syn");
		system("echo 0 > /proc/rg/trap_syn_and_disable_svlan");
		system("echo 0x0 > /proc/rg/congestion_ctrl_port_mask");
		system("echo 0 > /proc/rg/congestion_ctrl_send_remainder_in_next_gap");
		system("echo 0 > /proc/rg/congestion_ctrl_inbound_ack_to_high_queue");
*/
	}

/*To configure user's define vlan id range*/
	mib_get(MIB_FWD_CPU_VLAN_ID, (void *)&vid);
	init_param.fwdVLAN_CPU = vid;
	//AUG_PRT("%s-%d fwdVLAN_CPU=%d\n",__func__,__LINE__,init_param.fwdVLAN_CPU);

	mib_get(MIB_FWD_PROTO_BLOCK_VLAN_ID, (void *)&vid);
	init_param.fwdVLAN_Proto_Block = vid;
	//AUG_PRT("%s-%d fwdVLAN_Proto_Block=%d\n",__func__,__LINE__,init_param.fwdVLAN_Proto_Block);

	mib_get(MIB_FWD_BIND_INTERNET_VLAN_ID, (void *)&vid);
	init_param.fwdVLAN_BIND_INTERNET = vid;
	//AUG_PRT("%s-%d fwdVLAN_BIND_INTERNET=%d\n",__func__,__LINE__,init_param.fwdVLAN_BIND_INTERNET);

	mib_get(MIB_FWD_BIND_OTHER_VLAN_ID, (void *)&vid);
	init_param.fwdVLAN_BIND_OTHER = vid;
	//AUG_PRT("%s-%d fwdVLAN_BIND_OTHER=%d\n",__func__,__LINE__,init_param.fwdVLAN_BIND_OTHER);


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
	if(patch_for_avalanche)
	{
		//(1) FTP algmask (0xc0)
		rtk_rg_alg_type_t alg_app = 0;
		rtk_rg_aclFilterAndQos_t aclRule;
		int aclIdx;
		FILE *fp;
		FILE *fp_proc;
		rtk_rg_algApps_get(&alg_app);
		alg_app |= RTK_RG_ALG_FTP_TCP_BIT | RTK_RG_ALG_FTP_UDP_BIT;
		rtk_rg_algApps_set(alg_app);
		//add to trap unicast packet
/*
		rg clear acl-filter
		rg set acl-filter fwding_type_and_direction 0
		rg set acl-filter action action_type 2
		rg set acl-filter pattern ingress_dmac 0:0:0:0:0:0
		rg set acl-filter pattern ingress_dmac_mask 1:0:0:0:0:0
		rg set acl-filter pattern ingress_port_mask 0x10
		rg add acl-filter entry

*/
		if(!(fp = fopen(RG_PATCH_FOR_AVALANCHE, "a")))
		{
			fprintf(stderr, "ERROR! %s\n", strerror(errno));
			return -2;
		}

		memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
		aclRule.fwding_type_and_direction =  ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET;
		aclRule.action_type = ACL_ACTION_TYPE_TRAP;
		aclRule.filter_fields |= INGRESS_DMAC_BIT;
		aclRule.ingress_dmac.octet[0] = 0;
		aclRule.ingress_dmac.octet[1] = 0;
		aclRule.ingress_dmac.octet[2] = 0;
		aclRule.ingress_dmac.octet[3] = 0;
		aclRule.ingress_dmac.octet[4] = 0;
		aclRule.ingress_dmac.octet[5] = 0;
		aclRule.ingress_dmac_mask.octet[0] = 0x1;
		aclRule.ingress_dmac_mask.octet[1] = 0;
		aclRule.ingress_dmac_mask.octet[2] = 0;
		aclRule.ingress_dmac_mask.octet[3] = 0;
		aclRule.ingress_dmac_mask.octet[4] = 0;
		aclRule.ingress_dmac_mask.octet[5] = 0;
		aclRule.filter_fields |= INGRESS_PORT_BIT;
		aclRule.ingress_port_mask.portmask = RG_get_wan_phyPortMask();//wan port
		//rg add acl-filter entry
		if((ret = rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx)) == 0){
			fprintf(fp,"%d\n",aclIdx);
			//fprintf(stderr, "add %s-%d Vlan:%d, index=%d success\n",__func__,__LINE__,entry->vid, aclIdx);
		}else{
			fprintf(stderr,"%s-%d add rule failed! (ret = %d)\n",__func__,__LINE__, ret);
			fclose(fp);
			return -1;
		}
		/*
		rg clear acl-filter
		rg set acl-filter fwding_type_and_direction 0
		rg set acl-filter action action_type 2
		rg set acl-filter pattern ingress_port_mask 0xf
		rg set acl-filter pattern ingress_dest_l4_port_start 80 ingress_dest_l4_port_end 80
		rg set acl-filter pattern ingress_src_ipv4_addr_start 0.0.0.0 ingress_src_ipv4_addr_end 255.255.255.255
		rg add acl-filter entry
		*/
		memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
		aclRule.fwding_type_and_direction =  ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET;
		aclRule.action_type = ACL_ACTION_TYPE_TRAP;
		aclRule.filter_fields |= INGRESS_PORT_BIT;
		#ifdef CONFIG_RTL9602C_SERIES
		aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0x3);
		#else
		aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0xf);
		#endif		
		aclRule.filter_fields |= INGRESS_L4_DPORT_RANGE_BIT;
		aclRule.ingress_dest_l4_port_start = 80;
		aclRule.ingress_dest_l4_port_end = 80;
		aclRule.filter_fields |= INGRESS_IPV4_SIP_RANGE_BIT;
		aclRule.filter_fields |= INGRESS_IPV4_TAGIF_BIT;
		aclRule.ingress_ipv4_tagif = 1;
		aclRule.ingress_src_ipv4_addr_start = 0x0;
		aclRule.ingress_src_ipv4_addr_end = 0xffffffff;
		//rg add acl-filter entry
		if((ret = rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx)) == 0){
			fprintf(fp,"%d\n",aclIdx);
			//fprintf(stderr, "add %s-%d Vlan:%d, index=%d success\n",__func__,__LINE__,entry->vid, aclIdx);
		}else{
			fprintf(stderr,"%s-%d add rule failed! (ret = %d)\n",__func__,__LINE__, ret);
			fclose(fp);
			return -1;
		}

		usleep(300000);
		//system("echo 1 > /proc/rg/trap_syn_and_disable_svlan");
		fp_proc = fopen("/proc/rg/trap_syn_and_disable_svlan", "w");
		if(fp_proc)
		{
			fprintf(fp_proc, "1\n");
			fclose(fp_proc);
		}else
			fprintf(stderr, "open /proc/rg/trap_syn_and_disable_svlan fail!\n");

		usleep(300000);
		//system("echo 1 > /proc/rg/tcp_hw_learning_at_syn");
		fp_proc = fopen("/proc/rg/tcp_hw_learning_at_syn", "w");
		if(fp_proc)
		{
			fprintf(fp_proc, "1\n");
			fclose(fp_proc);
		}else
			fprintf(stderr, "open /proc/rg/tcp_hw_learning_at_syn fail!\n");

		usleep(300000);
		//system("echo 1 > /proc/rg/tcp_in_shortcut_learning_at_syn");
		fp_proc = fopen("/proc/rg/tcp_in_shortcut_learning_at_syn", "w");
		if(fp_proc)
		{
			fprintf(fp_proc, "1\n");
			fclose(fp_proc);
		}else
			fprintf(stderr, "open /proc/rg/tcp_in_shortcut_learning_at_syn fail!\n");



#if defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9607)
//		system("echo 0xd > /proc/rg/congestion_ctrl_port_mask");
		usleep(300000);
		fp_proc = fopen("/proc/rg/congestion_ctrl_port_mask", "w");
		if(fp_proc)
		{
			//we use 9607 to simulate 9603 case filled at 0x5!!
			//In the future, if we use 9607 model to
			//do e8c avalanche test, we must disable congestion ctrl.
			fprintf(fp_proc, "0x5\n");
			fclose(fp_proc);
		}else
			fprintf(stderr, "open /proc/rg/congestion_ctrl_port_mask fail!\n");

#elif defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9602B)
//		system("echo 0x4 > /proc/rg/congestion_ctrl_port_mask");
		usleep(300000);
		fp_proc = fopen("/proc/rg/congestion_ctrl_port_mask", "w");
		if(fp_proc)
		{
			fprintf(fp_proc, "0x4\n");
			fclose(fp_proc);
		}else
			fprintf(stderr, "open /proc/rg/congestion_ctrl_port_mask fail!\n");

#endif
//		system("echo 1000 > /proc/rg/congestion_ctrl_interval_usec");
		usleep(300000);
		fp_proc = fopen("/proc/rg/congestion_ctrl_interval_usec", "w");
		if(fp_proc)
		{
			fprintf(fp_proc, "1000\n");
			fclose(fp_proc);
		}else
			fprintf(stderr, "open /proc/rg/congestion_ctrl_interval_usec fail!\n");

//		system("echo 12600000 > /proc/rg/congestion_ctrl_send_byte_per_sec");
		usleep(300000);
		fp_proc = fopen("/proc/rg/congestion_ctrl_send_byte_per_sec", "w");
		if(fp_proc)
		{
			fprintf(fp_proc, "12600000\n");
			fclose(fp_proc);
		}else
			fprintf(stderr, "open /proc/rg/congestion_ctrl_send_byte_per_sec fail!\n");

//		system("echo 1 > /proc/rg/congestion_ctrl_send_remainder_in_next_gap");
		usleep(300000);
		fp_proc = fopen("/proc/rg/congestion_ctrl_send_remainder_in_next_gap", "w");
		if(fp_proc)
		{
			fprintf(fp_proc, "1\n");
			fclose(fp_proc);
		}else
			fprintf(stderr, "open /proc/rg/congestion_ctrl_send_remainder_in_next_gap fail!\n");

//		system("echo 1 > /proc/rg/congestion_ctrl_inbound_ack_to_high_queue");
		usleep(300000);
		fp_proc = fopen("/proc/rg/congestion_ctrl_inbound_ack_to_high_queue", "w");
		if(fp_proc)
		{
			fprintf(fp_proc, "1\n");
			fclose(fp_proc);
		}else
			fprintf(stderr, "open /proc/rg/congestion_ctrl_inbound_ack_to_high_queue fail!\n");

	}
	printf("=============Init_rg_api SUCESS!!==================\n");
	unlink(RG_LAN_INF_IDX);
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

#define MAX_INTF_NUM 7
int Check_RG_Intf_Count(void)
{
	int remained_intf_count=0;
	int cur_intf_count=0;
	rtk_rg_intfInfo_t *intf_info = NULL;
	int i=0, valid_idx=0;
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
	printf("%s-%d remained:%d, used:%d\n",__func__,__LINE__,remained_intf_count,cur_intf_count);
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
int Init_RG_ELan(int isUnTagCPort, int isRoutingWan)
{
	rtk_rg_lanIntfConf_t lan_info;
	int lanIntfIdx = -1;
#ifdef CONFIG_SECONDARY_IP
	int lanIntfIdx2 = -1;
#endif
	unsigned char value[6], ip_version=IPVER_V4V6, vchar, ipv6_addr[IPV6_ADDR_LEN], ipv6_prefix_len;
	int i;
	int wanPhyPort=0, vlan_id;
	unsigned int portMask = 0;
	unsigned int phy_portmask;
	struct ipv6_ifaddr ip6_addr[6];
	char ipv6addr_str[64], cur_ip6addr_str[64];
	FILE *fp;

#if 0
	Init_rg_api();
	DBPRINT(2, "Init_rg_api() on!\n");
#else
	DBPRINT(1, "Init_rg_api() off!!\n");
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

	if(mib_get(MIB_LAN_VLAN_ID1, (void *)&vlan_id) != 0)
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
	#ifdef 	CONFIG_RTL9602C_SERIES
	lan_info.port_mask.portmask=portMask|(1<<RTK_RG_EXT_PORT0)|(1<<RTK_RG_PORT_CPU);
	#else
	lan_info.port_mask.portmask=portMask|(1<<RTK_RG_EXT_PORT0)|(1<<RTK_RG_EXT_PORT1)|(1<<RTK_RG_EXT_PORT2)|(1<<RTK_RG_EXT_PORT3)|(1<<RTK_RG_EXT_PORT4)|(1<<RTK_RG_PORT_CPU);
	#endif
	lan_info.untag_mask.portmask = portMask;

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

		if(mib_get(MIB_LAN_VLAN_ID2, (void *)&vlan_id) != 0)
			lan_info.intf_vlan_id = vlan_id;

		if(mib_get(MIB_LAN_PORT_MASK2, (void *)&portMask)!=0)
		{
			phy_portmask = RG_get_lan_phyPortMask(portMask);
			portMask = phy_portmask;
			if(isRoutingWan)
				portMask &= (~(RG_get_wan_phyPortMask()));

			lan_info.port_mask.portmask=portMask|(1<<RTK_RG_EXT_PORT0)|(1<<RTK_RG_EXT_PORT1)|(1<<RTK_RG_EXT_PORT2)|(1<<RTK_RG_EXT_PORT3)|(1<<RTK_RG_EXT_PORT4);
			lan_info.untag_mask.portmask = portMask;

			if(isUnTagCPort)
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
/*find wan or lan interface index*/
int RG_intfInfo_find(MIB_CE_ATM_VC_Tp entry)
{
	rtk_rg_intfInfo_t *intf_info;
	int ret=0;
	int IntfIdx=-1;
	if(rtk_rg_intfInfo_find(intf_info,IntfIdx)!=0){
		printf("%s-%d Can't find the interface!",__func__,__LINE__);
		return -1;
	}
	return IntfIdx;
}

extern struct pmap_s pmap_list[MAX_VC_NUM];
extern int get_pmap_fgroup(struct pmap_s *pmap_p, int num);
unsigned short RG_get_port_binding(MIB_CE_ATM_VC_Tp entry)
{
        int i;
        unsigned int Index = entry->ifIndex;
		unsigned int wanPhyPort=0;
        get_pmap_fgroup(pmap_list,MAX_VC_NUM);
        for(i=0; i<MAX_VC_NUM; i++)
        {
                if(pmap_list[i].ifIndex == Index){
			return pmap_list[i].fgroup;
                }
        }
        printf("%s-%d Can't find the respected index\n");
        return -1;
}

const char VCONFIG[] = "/bin/vconfig";
#define ALIASNAME_ELAN_RG_WLAN "eth0"
/*setup vconfig of LAN for vlan binding*/
int setup_vconfig(unsigned short LanVid, int LanPortIdx)
{
	char v_eth_name[32];
	char sLanVid[16];
	unsigned char value[6];
	sprintf(sLanVid,"%d",LanVid);
	switch(LanPortIdx)
	{
		case 0:
			va_cmd(VCONFIG, 3, 1, "add", ALIASNAME_ELAN0, sLanVid);
			sprintf(v_eth_name,"%s.%d",ALIASNAME_ELAN0,LanVid);
			break;
		case 1:
			va_cmd(VCONFIG, 3, 1, "add", ALIASNAME_ELAN1, sLanVid);
			sprintf(v_eth_name,"%s.%d",ALIASNAME_ELAN1,LanVid);
			break;
		case 2:
			va_cmd(VCONFIG, 3, 1, "add", ALIASNAME_ELAN2, sLanVid);
			sprintf(v_eth_name,"%s.%d",ALIASNAME_ELAN2,LanVid);
			break;
		case 3:
			va_cmd(VCONFIG, 3, 1, "add", ALIASNAME_ELAN3, sLanVid);
			sprintf(v_eth_name,"%s.%d",ALIASNAME_ELAN3,LanVid);
			break;
#ifdef WLAN_SUPPORT
		case 4:
			//add this for normal path. (To protocol stack.)
			va_cmd(VCONFIG, 3, 1, "add", ALIASNAME_WLAN0, sLanVid);
			sprintf(v_eth_name,"%s.%d",ALIASNAME_WLAN0,LanVid);
			va_cmd(BRCTL, 3, 1, "addif", ALIASNAME_BR0, v_eth_name);
			va_cmd(IFCONFIG, 2, 1, v_eth_name, "up");

			//add this for forwarding engine
			va_cmd(VCONFIG, 3, 1, "add", ALIASNAME_ELAN_RG_WLAN, sLanVid);
			sprintf(v_eth_name,"%s.%d",ALIASNAME_ELAN_RG_WLAN,LanVid);
			break;
#endif
	}
	va_cmd(BRCTL, 3, 1, "addif", ALIASNAME_BR0, v_eth_name);
	va_cmd(IFCONFIG, 2, 1, v_eth_name, "up");
	return 0;
}
int flush_vconfig(unsigned short LanVid, int LanPortIdx)
{
	char v_eth_name[32];
	char sLanVid[16];
	unsigned char value[6];
	sprintf(sLanVid,"%d",LanVid);
	switch(LanPortIdx)
	{
		case 0:
			sprintf(v_eth_name,"%s.%d",ALIASNAME_ELAN0,LanVid);
			va_cmd(VCONFIG, 2, 1, "rem", v_eth_name);
			break;
		case 1:
			sprintf(v_eth_name,"%s.%d",ALIASNAME_ELAN1,LanVid);
			va_cmd(VCONFIG, 2, 1, "rem", v_eth_name);
			break;
		case 2:
			sprintf(v_eth_name,"%s.%d",ALIASNAME_ELAN2,LanVid);
			va_cmd(VCONFIG, 2, 1, "rem", v_eth_name);
			break;
		case 3:
			sprintf(v_eth_name,"%s.%d",ALIASNAME_ELAN3,LanVid);
			va_cmd(VCONFIG, 2, 1, "rem", v_eth_name);
#ifdef WLAN_SUPPORT
		case 4:
			//add this for normal path. (To protocol stack.)
			sprintf(v_eth_name,"%s.%d",ALIASNAME_WLAN0,LanVid);
			va_cmd(VCONFIG, 2, 1, "rem", v_eth_name);
			//add this for forwarding engine
			sprintf(v_eth_name,"%s.%d",ALIASNAME_ELAN_RG_WLAN,LanVid);
			va_cmd(VCONFIG, 2, 1, "rem", v_eth_name);
			break;
#endif
			break;
	}
	//va_cmd(BRCTL, 3, 1, "delif", ALIASNAME_BR0, v_eth_name);
	//va_cmd(IFCONFIG, 2, 1, v_eth_name, "down");
	return 0;

}
struct v_pair {
	unsigned short vid_a;
	unsigned short vid_b;
#ifdef CONFIG_RTK_RG_INIT
	unsigned short rg_vbind_entryID;
#endif
};
/*Vlan and port binding will effect the OMCI CF rules
    so, we must check rules after setting vlan port mapping
*/
#if defined(CONFIG_RTK_RG_INIT) || defined(CONFIG_GPON_FEATURE)

int RTK_RG_Sync_OMCI_WAN_INFO(void)
{
	MIB_CE_ATM_VC_T entryVC;
	int totalVC_entry,i,wan_idx=-1,wanIntfIdx=-1;
	int omci_service=-1;
	int omci_mode=-1;
	int omci_bind=-1;
	int ret=0;
	char cmdStr[64];
	char vlan_based_pri=-1;
	rtk_rg_intfInfo_t *intf_info = NULL;
	rtk_rg_wanIntfConf_t *wan_info_p = NULL;
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
			free(intf_info);
			return -1;
		}
		//for vlan binding, if we change binding relationship, we must sync rg WAN info.
		wan_info_p = &(intf_info->wan_intf.wan_intf_conf);
		#ifdef CONFIG_RTL9602C_SERIES
		wan_info_p->port_binding_mask.portmask = RG_get_lan_phyPortMask(entryVC.itfGroup & 0x3);
		wan_info_p->wlan0_dev_binding_mask = ((entryVC.itfGroup & 0xf8) >> 3);
		#else
		wan_info_p->port_binding_mask.portmask = RG_get_lan_phyPortMask(entryVC.itfGroup & 0xf);
		wan_info_p->wlan0_dev_binding_mask = ((entryVC.itfGroup & 0x1f0) >> 4);
		#endif		
		//wan_info_p->port_binding_mask.portmask = RG_get_lan_phyPortMask(entryVC.itfGroup & 0xf);
		//wan_info_p->wlan0_dev_binding_mask = ((entryVC.itfGroup & 0x1f0) >> 4);		
		wan_info_p->forcedAddNewIntf = 0;
		if((ret = rtk_rg_wanInterface_add(wan_info_p, &entryVC.rg_wan_idx))!=SUCCESS){
			printf("%s-%d rtk_rg_wanInterface_add fail! ret=%d\n",__func__,__LINE__,ret);
			free(intf_info);
			return -1;
		}		
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
int RG_add_vlanBinding(MIB_CE_ATM_VC_Tp pEntry,int pairID, unsigned short LanVid, int LanPortIdx)
{
	rtk_rg_vlanBinding_t vlanBind;
	MIB_CE_PORT_BINDING_T pbEntry;
	int rg_bind_idx=-1;
	int omci_service=-1;
	int omci_bind=-1;
	int omci_mode=-1;
	char cmdStr[64];
	char vlan_based_pri=-1;
	rtk_rg_intfInfo_t *intf_info = NULL;
	rtk_rg_wanIntfConf_t *wan_info_p = NULL;
	intf_info = (rtk_rg_intfInfo_t *)malloc(sizeof(rtk_rg_intfInfo_t));
	if(intf_info == NULL){
		printf("%s-%d Can't get enough memory space!\n",__func__,__LINE__);
		return -1;
	}
	memset(intf_info,0,sizeof(rtk_rg_intfInfo_t));
	if(rtk_rg_intfInfo_find(intf_info,&pEntry->rg_wan_idx)!=SUCCESS){
		printf("%s-%d Can't find the wan interface idx:%d!",__func__,__LINE__,pEntry->rg_wan_idx);
		free(intf_info);;
		return -1;
	}
	wan_info_p = &(intf_info->wan_intf.wan_intf_conf);
	//wan_info_p->port_binding_mask.portmask = (entryVC.itfGroup & 0xf);
	//wan_info_p->wlan0_dev_binding_mask = ((entryVC.itfGroup & 0x1f0) >> 4);
	//wan_info_p->forcedAddNewIntf = 0;
	memset(&vlanBind,0,sizeof(rtk_rg_vlanBinding_t));
	if(LanPortIdx <= PMAP_ETH0_SW3)
		vlanBind.port_idx = RG_get_lan_phyPortId(LanPortIdx);
	else
		vlanBind.port_idx = RG_get_wlan_phyPortId(LanPortIdx);
	vlanBind.ingress_vid=LanVid;
	vlanBind.wan_intf_idx=pEntry->rg_wan_idx;
	setup_vconfig(LanVid, LanPortIdx);
	mib_chain_get(MIB_PORT_BINDING_TBL, LanPortIdx, (void*)&pbEntry);
	//DBPRINT(2, "%s-%d \n",__func__,__LINE__,LanPortIdx,LanVid,RG_WAN_idx);
	if(rtk_rg_vlanBinding_add(&vlanBind,&rg_bind_idx)!= SUCCESS){
		DBPRINT(1, "%s-%d rtk_rg_vlanBinding_add fail\n",__func__,__LINE__);
		free(intf_info);;
		return -1;
	}
	//sync omci wan info......
	if(wan_info_p->none_internet)
		omci_service = 0;
	else
		omci_service = 1;
	//if((wan_info_p->port_binding_mask.portmask > 0) || (wan_info_p->wlan0_dev_binding_mask > 0))
		omci_bind = 1;
	//else
	//	omci_bind = 0;

	//omci wan info can't write duplicate, must delete it before adding.
	snprintf(cmdStr, sizeof(cmdStr),"echo %d %d %d %d %d %d %d > %s",pEntry->rg_wan_idx,0,0,0,0,0,0,OMCI_WAN_INFO);
	system(cmdStr);
	switch(wan_info_p->wan_type)
	{
		case RTK_RG_STATIC:
			omci_mode = 1;
			break;
		case RTK_RG_PPPoE:
			omci_mode = 0;
			break;
		case RTK_RG_BRIDGE:
			omci_mode = 2;
			break;
		default:
			printf("omci unsupport wan type!!!\n");
	}
	if(pEntry->vprio)
	{
		vlan_based_pri = pEntry->vprio-1;

	}
	else
	{
		vlan_based_pri = -1;
	}
	snprintf(cmdStr, sizeof(cmdStr),"echo %d %d %d %d %d %d %d > %s",pEntry->rg_wan_idx,wan_info_p->egress_vlan_id,wan_info_p->vlan_based_pri,omci_mode,omci_service,omci_bind,1,OMCI_WAN_INFO);
	system(cmdStr);
	//fprintf(stderr, "%s-%d %s\n",__func__,__LINE__,cmdStr);
	switch(pairID)
	{
		case 0:
			pbEntry.rg_vlan0_entryID = rg_bind_idx;
			break;
		case 1:
			pbEntry.rg_vlan1_entryID = rg_bind_idx;
			break;
		case 2:
			pbEntry.rg_vlan2_entryID = rg_bind_idx;
			break;
		case 3:
			pbEntry.rg_vlan3_entryID = rg_bind_idx;
			break;
		default:
			printf("%s-%d wrong pair id=%d\n",__func__,__LINE__,pairID);
	}
	mib_chain_update(MIB_PORT_BINDING_TBL,(void*)&pbEntry, LanPortIdx);
	free(intf_info);;
	return SUCCESS;
}
int RG_flush_vlanBinding(int LanPortIdx)
{
	rtk_rg_vlanBinding_t vlanBind;
	int totalPortbd,port;
	int valid_idx;
	MIB_CE_PORT_BINDING_T pbEntry;
	memset(&vlanBind,0,sizeof(rtk_rg_vlanBinding_t));
		//get the number 'LanPortIdx' pbentry!
		mib_chain_get(MIB_PORT_BINDING_TBL, LanPortIdx, (void*)&pbEntry);
		//is it vlan-mapping lan-port?
		if((unsigned char)VLAN_BASED_MODE == pbEntry.pb_mode){
			struct v_pair *vid_pair;
			int k;
			vid_pair = (struct v_pair *)&pbEntry.pb_vlan0_a;
			// because there are only 4 pairs~
			for (k=0; k<4; k++)
			{
				//Be sure the content of vlan-mapping exsit!
				if (vid_pair[k].vid_a)
				{
					flush_vconfig(vid_pair[k].vid_a,LanPortIdx);
					if(rtk_rg_vlanBinding_del(vid_pair[k].rg_vbind_entryID)!= SUCCESS){
						DBPRINT(1, "%s-%d rtk_rg_vlanBinding_del fail\n",__func__,__LINE__);
						return -1;
					}
					vid_pair[k].rg_vbind_entryID = 0;
				}
			}
			mib_chain_update(MIB_PORT_BINDING_TBL,(void*)&pbEntry, LanPortIdx);
		}
	return 0;
}

static inline int RG_get_wan_type(MIB_CE_ATM_VC_Tp entry)
{
	if(entry == NULL)
		return -1;

#if defined(CONFIG_IPV6) && defined(DUAL_STACK_LITE)
	//if ds-lite enable, special care first
	if(entry->dslite_enable){
		if(entry->cmode == CHANNEL_MODE_IPOE)
			return RTK_RG_DSLITE;

		else if(entry->cmode == CHANNEL_MODE_PPPOE)
			return RTK_RG_PPPoE_DSLITE;
	}
#endif

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
		printf("%s-%d del RG WAN[%d]\n",__func__,__LINE__,entry->rg_wan_idx);
		ret = RG_WAN_Interface_Del(entry->rg_wan_idx);
		entry->rg_wan_idx = -1;
	}
	return ret;
}
int RG_add_static_route_PPP(MIB_CE_IP_ROUTE_T *entry,MIB_CE_ATM_VC_T *vc_entry,int entryID)
{
	rtk_rg_wanIntfConf_t *wan_info = NULL;
	rtk_rg_intfInfo_t *intf_info = NULL;
	rtk_rg_pppoeClientInfoAfterDial_t *pppoeClientInfoA=NULL;
	int wan_idx=-1, wanIntfIdx=-1, ret=-1;
	int omci_service=-1;
	int omci_bind=-1;
	char cmdStr[64];
	char vlan_based_pri=-1;
	intf_info = (rtk_rg_intfInfo_t *)malloc(sizeof(rtk_rg_intfInfo_t));
	if(intf_info == NULL){
		printf("%s-%d Can't get enough memory space!\n",__func__,__LINE__);
		ret = -1;
		goto E_PPPOE_4;
	}
	memset(intf_info,0,sizeof(rtk_rg_intfInfo_t));
	//printf("%s-%d \n",__func__,__LINE__);
	wan_info = (rtk_rg_wanIntfConf_t *)malloc(sizeof(rtk_rg_wanIntfConf_t));
	if(wan_info == NULL){
		printf("%s-%d Can't get enough memory space!\n",__func__,__LINE__);
		ret = -1;
		goto E_PPPOE_3;
	}
	memset(wan_info,0,sizeof(rtk_rg_wanIntfConf_t));
	wan_idx = vc_entry->rg_wan_idx;
	//printf("%s-%d wan_idx=%d\n",__func__,__LINE__,wan_idx);
	if(rtk_rg_intfInfo_find(intf_info,&wan_idx)!=SUCCESS){
		printf("%s-%d Can't find the wan interface idx:%d!",__func__,__LINE__,wan_idx);
		ret = -1;
		goto E_PPPOE_2;
	}
	memcpy(wan_info,&(intf_info->wan_intf.wan_intf_conf),sizeof(rtk_rg_wanIntfConf_t));
	//printf("%s-%d \n",__func__,__LINE__);
	wan_info->forcedAddNewIntf = 1;
	if((ret = rtk_rg_wanInterface_add(wan_info, &wanIntfIdx))!=SUCCESS){
		printf("%s-%d rtk_rg_wanInterface_add fail! ret=%d\n",__func__,__LINE__,ret);
		ret = -1;
		goto E_PPPOE_2;
	}
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
	int omci_service=-1;
	int omci_bind=-1;
	char vlan_based_pri=-1;
	totalVC_entry = mib_chain_total(MIB_ATM_VC_TBL);
//printf("%s-%d totalVC_entry:%d\n",__func__,__LINE__,totalVC_entry);
//printf("%s-%d mac %02x:%02x:%02x:%02x:%02x:%02x\n",__func__,__LINE__,mac_str[0],mac_str[1],mac_str[2],mac_str[3],mac_str[4],mac_str[5]);
	for(i=0;i<totalVC_entry;i++){
		if(mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&entryVC) == 0)
			continue;
		if(memcmp(entryVC.MacAddr, mac_str, MAC_ADDR_LEN)==0){
			wan_idx = entryVC.rg_wan_idx;
//fprintf(stderr, "%s-%d wan_idx:%d\n",__func__,__LINE__,wan_idx);
			break;
		}
	}
//printf("%s-%d wan_idx:%d\n",__func__,__LINE__,wan_idx);
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

	//dump_wan_info(wan_info);
   	if((ret = rtk_rg_wanInterface_add(wan_info, &wanIntfIdx))!=SUCCESS){
		printf("%s-%d rtk_rg_wanInterface_add fail! ret=%d\n",__func__,__LINE__,ret);
		ret = -1;
		goto Error2;
	}
	//printf("%s-%d static wan wanIntfIdx:%d\n",__func__,__LINE__,wanIntfIdx);
	entry->rg_wan_idx = wanIntfIdx;
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
				//dump_ipDhcpClientInfo(dhcpClient_info);
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
	//dump_ipStaticInfo(staticInfo);
	if((ret = rtk_rg_staticInfo_set(entry->rg_wan_idx, staticInfo))!=SUCCESS){
		printf("%s-%d add rtk_rg_staticInfo_set fail! ret=%d\n",__func__,__LINE__,ret);
		ret = -1;
				}
			}
			break;
		default:
			//this function doesn't support other WAN.
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
/*For E8C 2014 Test Plan 2.2.4/2.2.3*/
/*Two wan with the same VID + PortBinding MASK*/
/*
case 1: (2.2.3)
1)Routing Wan ipv4
2)Bridge Wan ipv6
case 2: (2.2.4)
1)Bridge Wan ipv4
2)Routing Wan ipv6
*/
int RTK_RG_Check_VID_PortBind(void)
{

	int totalVC_entry,i,j,k,wan_idx=-1,wanIntfIdx=-1;
	MIB_CE_ATM_VC_T entryVC, entryA;
	int ret;
	totalVC_entry = mib_chain_total(MIB_ATM_VC_TBL);
	for(i=totalVC_entry-1;i>=0;i--){
		if(mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&entryVC) == 0)
			continue;
		if(entryVC.IpProtocol == IPVER_IPV4_IPV6)
			continue;
		k=i-1;
		for(j=k;j>=0;j--){
			if(mib_chain_get(MIB_ATM_VC_TBL, j, (void *)&entryA) == 0)
				continue;
			if(entryA.IpProtocol == IPVER_IPV4_IPV6)
				continue;
			if((entryVC.vid == entryA.vid) && (entryVC.itfGroup == entryA.itfGroup)){
				rtk_rg_intfInfo_t *intf_info = NULL;
				rtk_rg_wanIntfConf_t *wan_info_p;
				intf_info = (rtk_rg_intfInfo_t *)malloc(sizeof(rtk_rg_intfInfo_t));
				if(intf_info == NULL){
					printf("%s-%d Can't get enough memory space!\n",__func__,__LINE__);
					ret = -1;
					goto ERR_VP;
				}
				memset(intf_info,0,sizeof(rtk_rg_intfInfo_t));
				if(entryVC.cmode == CHANNEL_MODE_BRIDGE && (entryA.cmode != CHANNEL_MODE_BRIDGE)){
					if(rtk_rg_intfInfo_find(intf_info,&entryVC.rg_wan_idx)!=SUCCESS){
						printf("%s-%d Can't find the wan interface idx:%d!",__func__,__LINE__,entryVC.rg_wan_idx);
						ret = -1;
						goto ERR_VP;
					}
					wan_info_p = &(intf_info->wan_intf.wan_intf_conf);
					wan_info_p->port_binding_mask.portmask = 0;
					wan_info_p->wlan0_dev_binding_mask = 0;
					wan_info_p->forcedAddNewIntf = 0;
					if((ret = rtk_rg_wanInterface_add(wan_info_p, &entryVC.rg_wan_idx))!=SUCCESS){
						printf("%s-%d rtk_rg_wanInterface_add fail! ret=%d\n",__func__,__LINE__,ret);
						ret = -1;
						goto ERR_VP;
					}
					memset(intf_info,0,sizeof(rtk_rg_intfInfo_t));
					//first routing wan's itfGroup will be replaced by second bridge~
					if(rtk_rg_intfInfo_find(intf_info,&entryA.rg_wan_idx)!=SUCCESS){
						printf("%s-%d Can't find the wan interface idx:%d!",__func__,__LINE__,entryA.rg_wan_idx);
						ret = -1;
						goto ERR_VP;
					}
					wan_info_p = &(intf_info->wan_intf.wan_intf_conf);
					#ifdef CONFIG_RTL9602C_SERIES
					wan_info_p->port_binding_mask.portmask = (entryA.itfGroup & 0x3);
					wan_info_p->wlan0_dev_binding_mask = ((entryA.itfGroup & 0xf8) >> 3);
					#else
					wan_info_p->port_binding_mask.portmask = (entryA.itfGroup & 0xf);
					wan_info_p->wlan0_dev_binding_mask = ((entryA.itfGroup & 0x1f0) >> 4);
					#endif
					wan_info_p->forcedAddNewIntf = 0;
					if((ret = rtk_rg_wanInterface_add(wan_info_p, &entryA.rg_wan_idx))!=SUCCESS){
						printf("%s-%d rtk_rg_wanInterface_add fail! ret=%d\n",__func__,__LINE__,ret);
						ret = -1;
						goto ERR_VP;
					}

					/*case1: first wan is routed, second wan is bridged*/
					switch(entryVC.IpProtocol)
					{
					case IPVER_IPV4: // 2.IPv4 Bridge, 1.IPv6 Routing
						RTK_RG_Set_ACL_IPV6_PPPoE_from_Wan_KeepOVID(&entryA);
						RTK_RG_Set_ACL_IPV4_Bridge_from_Wan(&entryVC);
						system("echo 2 > /proc/rg/portBindingByProtocal");
						break;
					case IPVER_IPV6: // 2.IPv6 Bridge, 1.IPv4 Routing
						RTK_RG_Set_ACL_IPV4_PPPoE_from_Wan_KeepOVID(&entryA);
						RTK_RG_Set_ACL_IPV6_Bridge_from_Wan(&entryVC);
						system("echo 1 > /proc/rg/portBindingByProtocal");
						break;
					case IPVER_IPV4_IPV6:
						default:
						/*not implement*/
						break;
					}
					entryVC.check_br_pm = 1;
					mib_chain_update(MIB_ATM_VC_TBL, (void*)&entryVC, i);

				}
				else if(entryVC.cmode != CHANNEL_MODE_BRIDGE && entryA.cmode == CHANNEL_MODE_BRIDGE){
					/*routing*/
					/*case2: first wan is bridged, second wan is routed*/
					/*we must remove first bridge wan's port binding mask.*/
/*
					if(rtk_rg_intfInfo_find(intf_info,&entryA.rg_wan_idx)!=SUCCESS){
						printf("%s-%d Can't find the wan interface idx:%d!",__func__,__LINE__,entryA.rg_wan_idx);
						ret = -1;
						goto ERR_VP;
					}
					wan_info_p = &(intf_info->wan_intf.wan_intf_conf);
					wan_info_p->port_binding_mask.portmask = 0;
					wan_info_p->wlan0_dev_binding_mask = 0;
					wan_info_p->forcedAddNewIntf = 0;

					//dump_wan_info(wan_info);
					if((ret = rtk_rg_wanInterface_add(wan_info_p, &entryA.rg_wan_idx))!=SUCCESS){
						printf("%s-%d rtk_rg_wanInterface_add fail! ret=%d\n",__func__,__LINE__,ret);
						ret = -1;
						goto ERR_VP;
					}
*/
					switch(entryVC.IpProtocol)
					{
						case IPVER_IPV4:  //2.IPv4 Routing, 1.IPv6 Bridge
						RTK_RG_Set_ACL_IPV4_PPPoE_from_Wan_KeepOVID(&entryVC);
						RTK_RG_Set_ACL_IPV6_Bridge_from_Wan(&entryA);
						system("echo 1 > /proc/rg/portBindingByProtocal");
							break;
						case IPVER_IPV6: //2.IPv6 Routing, 1.IPv4 Bridge
						RTK_RG_Set_ACL_IPV6_PPPoE_from_Wan_KeepOVID(&entryVC);
						RTK_RG_Set_ACL_IPV4_Bridge_from_Wan(&entryA);
						system("echo 2 > /proc/rg/portBindingByProtocal");
							break;
						case IPVER_IPV4_IPV6:
						default:
						/*not implement*/
							break;
					}
					entryA.check_br_pm = 1;
					mib_chain_update(MIB_ATM_VC_TBL, (void*)&entryA, j);

				}
				ERR_VP:
					if(intf_info)
						free(intf_info);
				return 1;
			}
		}

	}
	system("echo 0 > /proc/rg/portBindingByProtocal");

	return 0;

}

int RTK_RG_Set_IPv4_IPv6_Vid_Binding_ACL(void)
{
	int ret, totalVC_entry,i;
	MIB_CE_ATM_VC_T entryVC;
	totalVC_entry = mib_chain_total(MIB_ATM_VC_TBL);
	ret = RTK_RG_Check_VID_PortBind();
//fprintf(stderr, "%s-%d ret=%d\n",__func__,__LINE__,ret);
	if(ret == 0)
	{
		for(i=0;i<totalVC_entry;i++){
			if(mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&entryVC) == 0)
				continue;
			//fprintf(stderr, "%s-%d i=%d, entryVC.check_br_pm=%d\n",__func__,__LINE__,i,entryVC.check_br_pm);

			if(entryVC.check_br_pm)
			{
				//fprintf(stderr, "%s-%d entryVC.cmode=%d\n",__func__,__LINE__,entryVC.cmode);

				//reset BridgeWan's port mask
				if(entryVC.cmode == CHANNEL_MODE_BRIDGE)
				{
					rtk_rg_intfInfo_t *intf_info = NULL;
					rtk_rg_wanIntfConf_t *wan_info_p = NULL;
					intf_info = (rtk_rg_intfInfo_t *)malloc(sizeof(rtk_rg_intfInfo_t));
					if(intf_info == NULL){
						printf("%s-%d Can't get enough memory space!\n",__func__,__LINE__);
						ret = -1;
						goto ERR_CHECK_BR;
					}
					memset(intf_info,0,sizeof(rtk_rg_intfInfo_t));

					if(rtk_rg_intfInfo_find(intf_info,&entryVC.rg_wan_idx)!=SUCCESS){
						printf("%s-%d Can't find the wan interface idx:%d!",__func__,__LINE__,entryVC.rg_wan_idx);
						ret = -1;
						free(intf_info);;
						goto ERR_CHECK_BR;
					}
					wan_info_p = &(intf_info->wan_intf.wan_intf_conf);
					#ifdef CONFIG_RTL9602C_SERIES
					wan_info_p->port_binding_mask.portmask = (entryVC.itfGroup & 0x3);
					wan_info_p->wlan0_dev_binding_mask = ((entryVC.itfGroup & 0xf8) >> 3);
					#else
					wan_info_p->port_binding_mask.portmask = (entryVC.itfGroup & 0xf);
					wan_info_p->wlan0_dev_binding_mask = ((entryVC.itfGroup & 0x1f0) >> 4);
					#endif

					wan_info_p->forcedAddNewIntf = 0;
					if((ret = rtk_rg_wanInterface_add(wan_info_p, &entryVC.rg_wan_idx))!=SUCCESS){
						printf("%s-%d rtk_rg_wanInterface_add fail! ret=%d\n",__func__,__LINE__,ret);
						ret = -1;
						free(intf_info);;
						goto ERR_CHECK_BR;
					}
					entryVC.check_br_pm = 0;
					//fprintf(stderr, "%s-%d entryVC.check_br_pm=%d\n",__func__,__LINE__,entryVC.check_br_pm);
					mib_chain_update(MIB_ATM_VC_TBL, (void*)&entryVC, i);
				}
			}
		}
	}
	ERR_CHECK_BR:
	return ret;
}
int Flush_RTK_RG_IPv4_Bridge_From_Wan_ACL(void)
{
	FILE *fp;
	int acl_idx;

	if(!(fp = fopen(RG_IPV4_Bridge_From_Wan_ACL_RILES, "r")))
		return -2;

	while(fscanf(fp, "%d\n", &acl_idx) != EOF)
	{
		if(rtk_rg_aclFilterAndQos_del(acl_idx))
			DBPRINT(1, "rtk_rg_aclFilterAndQos_del failed! idx = %d\n", acl_idx);
	}

	fclose(fp);
	unlink(RG_IPV4_Bridge_From_Wan_ACL_RILES);
	return 0;
}

int Flush_RTK_RG_IPv4_PPPoE_From_Wan_ACL(void)
{
	FILE *fp;
	int acl_idx;

	if(!(fp = fopen(RG_IPV4_PPPoE_From_Wan_KeepOVID_ACL_RILES, "r")))
		return -2;

	while(fscanf(fp, "%d\n", &acl_idx) != EOF)
	{
		if(rtk_rg_aclFilterAndQos_del(acl_idx))
			DBPRINT(1, "rtk_rg_aclFilterAndQos_del failed! idx = %d\n", acl_idx);
	}

	fclose(fp);
	unlink(RG_IPV4_PPPoE_From_Wan_KeepOVID_ACL_RILES);
	return 0;
}


int Flush_RTK_RG_IPv6_Bridge_From_Wan_ACL(void)
{
	FILE *fp;
	int acl_idx;

	if(!(fp = fopen(RG_IPV6_Bridge_From_Wan_ACL_RILES, "r")))
		return -2;

	while(fscanf(fp, "%d\n", &acl_idx) != EOF)
	{
		if(rtk_rg_aclFilterAndQos_del(acl_idx))
			DBPRINT(1, "rtk_rg_aclFilterAndQos_del failed! idx = %d\n", acl_idx);
	}

	fclose(fp);
	unlink(RG_IPV6_Bridge_From_Wan_ACL_RILES);
	return 0;
}

int Flush_RTK_RG_IPv6_PPPoE_From_Wan_ACL(void)
{
	FILE *fp;
	int acl_idx;

	if(!(fp = fopen(RG_IPV6_PPPoE_From_Wan_KeepOVID_ACL_RILES, "r")))
		return -2;

	while(fscanf(fp, "%d\n", &acl_idx) != EOF)
	{
		if(rtk_rg_aclFilterAndQos_del(acl_idx))
			DBPRINT(1, "rtk_rg_aclFilterAndQos_del failed! idx = %d\n", acl_idx);
	}

	fclose(fp);
	unlink(RG_IPV6_PPPoE_From_Wan_KeepOVID_ACL_RILES);


	return 0;
}


int Flush_RTK_RG_IPv4_IPv6_Vid_Binding_ACL(void)
{
	Flush_RTK_RG_IPv4_Bridge_From_Wan_ACL();
	Flush_RTK_RG_IPv4_PPPoE_From_Wan_ACL();
	Flush_RTK_RG_IPv6_Bridge_From_Wan_ACL();
	Flush_RTK_RG_IPv6_PPPoE_From_Wan_ACL();

	return 0;
}


int RTK_RG_Set_ACL_IPV6_Bridge_from_Wan(MIB_CE_ATM_VC_Tp entry)
{
	FILE *fp;
	rtk_rg_aclFilterAndQos_t aclRule;
	int aclIdx, ret;
	rtk_rg_initParams_t init_param;
	bzero(&init_param, sizeof(rtk_rg_initParams_t));
	if((ret = rtk_rg_initParam_get(&init_param)) != SUCCESS)
	{
		fprintf(stderr, "rtk_rg_initParam_set failed! ret=%d\n", ret);
		return -1;
	}
	if(!(fp = fopen(RG_IPV6_Bridge_From_Wan_ACL_RILES, "a")))
	{
		fprintf(stderr, "ERROR! %s\n", strerror(errno));
		return -2;
	}
	//For IPv6 IPoE Bridge packet from Wan
	//rg clear acl-filter
	memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
	//rg set acl-filter fwding_type_and_direction 0
	aclRule.fwding_type_and_direction =  ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET;
	//rg set acl-filter pattern ingress_port_mask 0x10
	aclRule.filter_fields |= INGRESS_PORT_BIT;
	aclRule.ingress_port_mask.portmask = RG_get_wan_phyPortMask();//wan port
	aclRule.acl_weight = RG_DEFAULT_ACL_WEIGHT;
	if(entry->vid > 0){
		//rg set acl-filter pattern ingress_ctagIf 1
   		aclRule.filter_fields |= INGRESS_CTAGIF_BIT;
		aclRule.ingress_ctagIf = 1;
		//rg set acl-filter pattern ingress_ctag_vid 45
		aclRule.filter_fields |= INGRESS_CTAG_VID_BIT;
		aclRule.ingress_ctag_vid = entry->vid;
	}
	//rg set acl-filter pattern ingress_ethertype 0x86dd (ipv6)
	aclRule.filter_fields |= INGRESS_ETHERTYPE_BIT;
	aclRule.ingress_ethertype = 0x86dd;
	//rg set acl-filter action action_type 3
	aclRule.action_type = ACL_ACTION_TYPE_QOS;
	//rg set acl-filter action qos action_ingress_vid 4005
	aclRule.qos_actions |= ACL_ACTION_ACL_INGRESS_VID_BIT;
	aclRule.action_acl_ingress_vid = init_param.fwdVLAN_BIND_INTERNET; //lan interface's vlan
	//rg add acl-filter entry
	if((ret = rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx)) == 0){
		fprintf(fp,"%d\n",aclIdx);
		//fprintf(stderr, "add %s-%d Vlan:%d, index=%d success\n",__func__,__LINE__,entry->vid, aclIdx);
	}else{
		fprintf(stderr,"%s-%d add rule failed! (ret = %d)\n",__func__,__LINE__, ret);
		fclose(fp);
		return -1;
	}
	//For IPv6 PPPoE(0x8863) Bridge packet from Wan
	memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
	//rg set acl-filter fwding_type_and_direction 0
	aclRule.fwding_type_and_direction =  ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET;
	//rg set acl-filter pattern ingress_port_mask 0x10
	aclRule.filter_fields |= INGRESS_PORT_BIT;
	aclRule.ingress_port_mask.portmask = RG_get_wan_phyPortMask();//wan port
	aclRule.acl_weight = RG_DEFAULT_ACL_WEIGHT;
	if(entry->vid > 0){
		//rg set acl-filter pattern ingress_ctagIf 1
   		aclRule.filter_fields |= INGRESS_CTAGIF_BIT;
		aclRule.ingress_ctagIf = 1;
		//rg set acl-filter pattern ingress_ctag_vid 45
		aclRule.filter_fields |= INGRESS_CTAG_VID_BIT;
		aclRule.ingress_ctag_vid = entry->vid;
	}
	//rg set acl-filter pattern ingress_ethertype 0x8863
	aclRule.filter_fields |= INGRESS_ETHERTYPE_BIT;
	aclRule.ingress_ethertype = 0x8863;
	//rg set acl-filter action action_type 3
	aclRule.action_type = ACL_ACTION_TYPE_QOS;
	//rg set acl-filter action qos action_ingress_vid 4005
	aclRule.qos_actions |= ACL_ACTION_ACL_INGRESS_VID_BIT;
	aclRule.action_acl_ingress_vid = init_param.fwdVLAN_BIND_INTERNET; //lan interface's vlan
	//rg add acl-filter entry
	if((ret = rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx)) == 0){
		fprintf(fp,"%d\n",aclIdx);
		//fprintf(stderr, "add %s-%d Vlan:%d, index=%d success\n",__func__,__LINE__,entry->vid, aclIdx);
	}else{
		fprintf(stderr,"%s-%d add rule failed! (ret = %d)\n",__func__,__LINE__, ret);
		fclose(fp);
		return -1;
	}
	//For IPv6 PPPoE(0x8864) Bridge packet from Wan
	memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
	//rg set acl-filter fwding_type_and_direction 0
	aclRule.fwding_type_and_direction =  ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET;
	//rg set acl-filter pattern ingress_port_mask 0x10
	aclRule.filter_fields |= INGRESS_PORT_BIT;
	aclRule.ingress_port_mask.portmask = RG_get_wan_phyPortMask();//wan port
	aclRule.acl_weight = RG_DEFAULT_ACL_WEIGHT;
	if(entry->vid > 0){
		//rg set acl-filter pattern ingress_ctagIf 1
   		aclRule.filter_fields |= INGRESS_CTAGIF_BIT;
		aclRule.ingress_ctagIf = 1;
		//rg set acl-filter pattern ingress_ctag_vid 45
		aclRule.filter_fields |= INGRESS_CTAG_VID_BIT;
		aclRule.ingress_ctag_vid = entry->vid;
	}
	//rg set acl-filter pattern ingress_ethertype 0x8864
	aclRule.filter_fields |= INGRESS_ETHERTYPE_BIT;
	aclRule.ingress_ethertype = 0x8864;
	//rg set acl-filter action action_type 3
	aclRule.action_type = ACL_ACTION_TYPE_QOS;
	//rg set acl-filter action qos action_ingress_vid 4005
	aclRule.qos_actions |= ACL_ACTION_ACL_INGRESS_VID_BIT;
	aclRule.action_acl_ingress_vid = init_param.fwdVLAN_BIND_INTERNET; //lan interface's vlan
	//rg add acl-filter entry
	if((ret = rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx)) == 0){
		fprintf(fp,"%d\n",aclIdx);
		//fprintf(stderr, "add %s-%d Vlan:%d, index=%d success\n",__func__,__LINE__,entry->vid, aclIdx);
	}else{
		fprintf(stderr,"%s-%d add rule failed! (ret = %d)\n",__func__,__LINE__, ret);
		fclose(fp);
		return -1;
	}
	//trap upstream ipv6 pppoe bridge
	//rg clear acl-filter
	memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
	//rg set acl-filter pattern ingress_port_mask 0xf
	aclRule.filter_fields |= INGRESS_PORT_BIT;
	#ifdef CONFIG_RTL9602C_SERIES
	aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0x3);
	#else
	aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0xf);
	#endif		
	//rg set acl-filter pattern ingress_ethertype 0x8864
	aclRule.filter_fields |= INGRESS_ETHERTYPE_BIT;
	aclRule.ingress_ethertype = 0x8864;
	//rg set acl-filter pattern ingress_ipv6_tagif 1
	//aclRule.filter_fields |= INGRESS_IPV6_TAGIF_BIT;
	//aclRule.ingress_ipv6_tagif = 1;
	//rg set acl-filter action action_type 2
	aclRule.action_type = ACL_ACTION_TYPE_TRAP;
	//rg add acl-filter entry
	if((ret = rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx)) == 0){
		fprintf(fp,"%d\n",aclIdx);
		//fprintf(stderr, "add %s-%d Vlan:%d, index=%d success\n",__func__,__LINE__,entry->vid, aclIdx);
	}else{
		fprintf(stderr,"%s-%d add rule failed! (ret = %d)\n",__func__,__LINE__, ret);
		fclose(fp);
		return -1;
	}
	fclose(fp);
	return 0;
}
int RTK_RG_Set_ACL_IPV4_Bridge_from_Wan(MIB_CE_ATM_VC_Tp entry)
{
	FILE *fp;
	rtk_rg_aclFilterAndQos_t aclRule;
	int aclIdx, ret;
	rtk_rg_initParams_t init_param;
	bzero(&init_param, sizeof(rtk_rg_initParams_t));
	if((ret = rtk_rg_initParam_get(&init_param)) != SUCCESS)
	{
		fprintf(stderr, "rtk_rg_initParam_set failed! ret=%d\n", ret);
		return -1;
	}
	if(!(fp = fopen(RG_IPV4_Bridge_From_Wan_ACL_RILES, "a")))
	{
		fprintf(stderr, "ERROR! %s\n", strerror(errno));
		return -2;
	}
	//For IPv4 IPoE Bridge packet from Wan
	//rg clear acl-filter
	memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
	//rg set acl-filter fwding_type_and_direction 0
	aclRule.fwding_type_and_direction =  ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET;
	//rg set acl-filter pattern ingress_port_mask 0x10
	aclRule.filter_fields |= INGRESS_PORT_BIT;
	aclRule.ingress_port_mask.portmask = RG_get_wan_phyPortMask();//wan port
	aclRule.acl_weight = RG_DEFAULT_ACL_WEIGHT;
	if(entry->vid > 0){
		//rg set acl-filter pattern ingress_ctagIf 1
   		aclRule.filter_fields |= INGRESS_CTAGIF_BIT;
		aclRule.ingress_ctagIf = 1;
		//rg set acl-filter pattern ingress_ctag_vid 45
		aclRule.filter_fields |= INGRESS_CTAG_VID_BIT;
		aclRule.ingress_ctag_vid = entry->vid;
	}
//	fprintf(stderr, "%s-%d entry->vid=%d\n",__func__,__LINE__,entry->vid);

	//rg set acl-filter pattern ingress_ethertype 0x0800 (ipv4)
	aclRule.filter_fields |= INGRESS_ETHERTYPE_BIT;
	aclRule.ingress_ethertype = 0x0800;
	//rg set acl-filter action action_type 3
	aclRule.action_type = ACL_ACTION_TYPE_QOS;
	//rg set acl-filter action qos action_ingress_vid 4005
	aclRule.qos_actions |= ACL_ACTION_ACL_INGRESS_VID_BIT;
	aclRule.action_acl_ingress_vid = init_param.fwdVLAN_BIND_INTERNET; //lan interface's vlan
	//rg add acl-filter entry
	if((ret = rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx)) == 0){
		fprintf(fp,"%d\n",aclIdx);
		//fprintf(stderr, "add %s-%d Vlan:%d, index=%d success\n",__func__,__LINE__,entry->vid, aclIdx);
	}else{
		fprintf(stderr,"%s-%d add rule failed! (ret = %d)\n",__func__,__LINE__, ret);
		fclose(fp);
		return -1;
	}
	//For IPv4 IPoE Bridge packet from Wan
	//rg clear acl-filter
	memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
	//rg set acl-filter fwding_type_and_direction 0
	aclRule.fwding_type_and_direction =  ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET;
	//rg set acl-filter pattern ingress_port_mask 0x10
	aclRule.filter_fields |= INGRESS_PORT_BIT;
	aclRule.ingress_port_mask.portmask = RG_get_wan_phyPortMask();//wan port
	aclRule.acl_weight = RG_DEFAULT_ACL_WEIGHT;
	if(entry->vid > 0){
		//rg set acl-filter pattern ingress_ctagIf 1
   		aclRule.filter_fields |= INGRESS_CTAGIF_BIT;
		aclRule.ingress_ctagIf = 1;
		//rg set acl-filter pattern ingress_ctag_vid 45
		aclRule.filter_fields |= INGRESS_CTAG_VID_BIT;
		aclRule.ingress_ctag_vid = entry->vid;
	}
	//rg set acl-filter pattern ingress_ethertype 0x0806 (ipv4)
	aclRule.filter_fields |= INGRESS_ETHERTYPE_BIT;
	aclRule.ingress_ethertype = 0x0806;
	//rg set acl-filter action action_type 3
	aclRule.action_type = ACL_ACTION_TYPE_QOS;
	//rg set acl-filter action qos action_ingress_vid 4005
	aclRule.qos_actions |= ACL_ACTION_ACL_INGRESS_VID_BIT;
	aclRule.action_acl_ingress_vid = init_param.fwdVLAN_BIND_INTERNET; //lan interface's vlan
	//rg add acl-filter entry
	if((ret = rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx)) == 0){
		fprintf(fp,"%d\n",aclIdx);
		//fprintf(stderr, "add %s-%d Vlan:%d, index=%d success\n",__func__,__LINE__,entry->vid, aclIdx);
	}else{
		fprintf(stderr,"%s-%d add rule failed! (ret = %d)\n",__func__,__LINE__, ret);
		fclose(fp);
		return -1;
	}

	//For IPv4 PPPoE(0x8863) Bridge packet from Wan
	memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
	//rg set acl-filter fwding_type_and_direction 0
	aclRule.fwding_type_and_direction =  ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET;
	//rg set acl-filter pattern ingress_port_mask 0x10
	aclRule.filter_fields |= INGRESS_PORT_BIT;
	aclRule.ingress_port_mask.portmask = RG_get_wan_phyPortMask();//wan port
	aclRule.acl_weight = RG_DEFAULT_ACL_WEIGHT;
	if(entry->vid > 0){
		//rg set acl-filter pattern ingress_ctagIf 1
   		aclRule.filter_fields |= INGRESS_CTAGIF_BIT;
		aclRule.ingress_ctagIf = 1;
		//rg set acl-filter pattern ingress_ctag_vid 45
		aclRule.filter_fields |= INGRESS_CTAG_VID_BIT;
		aclRule.ingress_ctag_vid = entry->vid;
	}
	//rg set acl-filter pattern ingress_ethertype 0x8863
	aclRule.filter_fields |= INGRESS_ETHERTYPE_BIT;
	aclRule.ingress_ethertype = 0x8863;
	//rg set acl-filter action action_type 3
	aclRule.action_type = ACL_ACTION_TYPE_QOS;
	//rg set acl-filter action qos action_ingress_vid 4005
	aclRule.qos_actions |= ACL_ACTION_ACL_INGRESS_VID_BIT;
	aclRule.action_acl_ingress_vid = init_param.fwdVLAN_BIND_INTERNET; //lan interface's vlan
	//rg add acl-filter entry
	if((ret = rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx)) == 0){
		fprintf(fp,"%d\n",aclIdx);
		//fprintf(stderr, "add %s-%d Vlan:%d, index=%d success\n",__func__,__LINE__,entry->vid, aclIdx);
	}else{
		fprintf(stderr,"%s-%d add rule failed! (ret = %d)\n",__func__,__LINE__, ret);
		fclose(fp);
		return -1;
	}
	//For IPv4 PPPoE(0x8864) Bridge packet from Wan
	memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
	//rg set acl-filter fwding_type_and_direction 0
	aclRule.fwding_type_and_direction =  ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET;
	//rg set acl-filter pattern ingress_port_mask 0x10
	aclRule.filter_fields |= INGRESS_PORT_BIT;
	aclRule.ingress_port_mask.portmask = RG_get_wan_phyPortMask();//wan port
	aclRule.acl_weight = RG_DEFAULT_ACL_WEIGHT;
	if(entry->vid > 0){
		//rg set acl-filter pattern ingress_ctagIf 1
   		aclRule.filter_fields |= INGRESS_CTAGIF_BIT;
		aclRule.ingress_ctagIf = 1;
		//rg set acl-filter pattern ingress_ctag_vid 45
		aclRule.filter_fields |= INGRESS_CTAG_VID_BIT;
		aclRule.ingress_ctag_vid = entry->vid;
	}
	//rg set acl-filter pattern ingress_ethertype 0x8864
	aclRule.filter_fields |= INGRESS_ETHERTYPE_BIT;
	aclRule.ingress_ethertype = 0x8864;
	//rg set acl-filter action action_type 3
	aclRule.action_type = ACL_ACTION_TYPE_QOS;
	//rg set acl-filter action qos action_ingress_vid 4005
	aclRule.qos_actions |= ACL_ACTION_ACL_INGRESS_VID_BIT;
	aclRule.action_acl_ingress_vid = init_param.fwdVLAN_BIND_INTERNET; //lan interface's vlan
	//rg add acl-filter entry
	if((ret = rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx)) == 0){
		fprintf(fp,"%d\n",aclIdx);
		//fprintf(stderr, "add %s-%d Vlan:%d, index=%d success\n",__func__,__LINE__,entry->vid, aclIdx);
	}else{
		fprintf(stderr,"%s-%d add rule failed! (ret = %d)\n",__func__,__LINE__, ret);
		fclose(fp);
		return -1;
	}
	//trap upstream ipv4 pppoe bridge
	//rg clear acl-filter
	memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
	//rg set acl-filter pattern ingress_port_mask 0xf
	aclRule.filter_fields |= INGRESS_PORT_BIT;
	#ifdef CONFIG_RTL9602C_SERIES
	aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0x3);
	#else
	aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0xf);
	#endif		
	//rg set acl-filter pattern ingress_ethertype 0x8864
	aclRule.filter_fields |= INGRESS_ETHERTYPE_BIT;
	aclRule.ingress_ethertype = 0x8864;
	//rg set acl-filter pattern ingress_ipv4_tagif 1
	//aclRule.filter_fields |= INGRESS_IPV4_TAGIF_BIT;
	//aclRule.ingress_ipv4_tagif = 1;
	//rg set acl-filter action action_type 2
	aclRule.action_type = ACL_ACTION_TYPE_TRAP;
	//rg add acl-filter entry
	if((ret = rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx)) == 0){
		fprintf(fp,"%d\n",aclIdx);
		//fprintf(stderr, "add %s-%d Vlan:%d, index=%d success\n",__func__,__LINE__,entry->vid, aclIdx);
	}else{
		fprintf(stderr,"%s-%d add rule failed! (ret = %d)\n",__func__,__LINE__, ret);
		fclose(fp);
		return -1;
	}
	fclose(fp);
	return 0;

}
int RTK_RG_Set_ACL_IPV6_PPPoE_from_Wan_KeepOVID(MIB_CE_ATM_VC_Tp entry)
{
	FILE *fp;
	rtk_rg_aclFilterAndQos_t aclRule;
	int aclIdx, ret;
	rtk_rg_initParams_t init_param;
	bzero(&init_param, sizeof(rtk_rg_initParams_t));
	if((ret = rtk_rg_initParam_get(&init_param)) != SUCCESS)
	{
		fprintf(stderr, "rtk_rg_initParam_set failed! ret=%d\n", ret);
		return -1;
	}
	if(!(fp = fopen(RG_IPV6_PPPoE_From_Wan_KeepOVID_ACL_RILES, "a")))
	{
		fprintf(stderr, "ERROR! %s\n", strerror(errno));
		return -2;
	}
	//For IPv6 PPPoE(0x8863) Routing packet from Wan, keep original VID
	memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
	//rg set acl-filter fwding_type_and_direction 0
	aclRule.fwding_type_and_direction =  ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET;
	//rg set acl-filter pattern ingress_port_mask 0x10
	aclRule.filter_fields |= INGRESS_PORT_BIT;
	aclRule.ingress_port_mask.portmask = RG_get_wan_phyPortMask();//wan port
	aclRule.acl_weight = RG_DEFAULT_ACL_WEIGHT;
	if(entry->vid > 0){
		//rg set acl-filter pattern ingress_ctagIf 1
   		aclRule.filter_fields |= INGRESS_CTAGIF_BIT;
		aclRule.ingress_ctagIf = 1;
		//rg set acl-filter pattern ingress_ctag_vid 45
		aclRule.filter_fields |= INGRESS_CTAG_VID_BIT;
		aclRule.ingress_ctag_vid = entry->vid;
	}
	//rg set acl-filter pattern ingress_ethertype 0x8863
	aclRule.filter_fields |= INGRESS_ETHERTYPE_BIT;
	aclRule.ingress_ethertype = 0x8863;
	//rg set acl-filter pattern ingress_dmac 00:00:39:00:74:00
   	aclRule.filter_fields |= INGRESS_DMAC_BIT;
	memcpy(&aclRule.ingress_dmac, entry->MacAddr, MAC_ADDR_LEN);
	//rg set acl-filter action action_type 3
	aclRule.action_type = ACL_ACTION_TYPE_QOS;
	//rg set acl-filter action qos action_ingress_vid 45
	aclRule.qos_actions |= ACL_ACTION_ACL_INGRESS_VID_BIT;
	aclRule.action_acl_ingress_vid = entry->vid;
	//rg add acl-filter entry
	if((ret = rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx)) == 0){
		fprintf(fp,"%d\n",aclIdx);
		//fprintf(stderr, "add %s-%d Vlan:%d, index=%d success\n",__func__,__LINE__,entry->vid, aclIdx);
	}else{
		fprintf(stderr,"%s-%d add rule failed! (ret = %d)\n",__func__,__LINE__, ret);
		fclose(fp);
		return -1;
	}
	//For IPv4 PPPoE(0x8864) Routing packet from Wan,  keep original VID
	memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
	//rg set acl-filter fwding_type_and_direction 0
	aclRule.fwding_type_and_direction =  ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET;
	//rg set acl-filter pattern ingress_port_mask 0x10
	aclRule.filter_fields |= INGRESS_PORT_BIT;
	aclRule.ingress_port_mask.portmask = RG_get_wan_phyPortMask();//wan port
	aclRule.acl_weight = RG_DEFAULT_ACL_WEIGHT;
	if(entry->vid > 0){
		//rg set acl-filter pattern ingress_ctagIf 1
   		aclRule.filter_fields |= INGRESS_CTAGIF_BIT;
		aclRule.ingress_ctagIf = 1;
		//rg set acl-filter pattern ingress_ctag_vid 45
		aclRule.filter_fields |= INGRESS_CTAG_VID_BIT;
		aclRule.ingress_ctag_vid = entry->vid;
	}
	//rg set acl-filter pattern ingress_ethertype 0x8864
	aclRule.filter_fields |= INGRESS_ETHERTYPE_BIT;
	aclRule.ingress_ethertype = 0x8864;
	//rg set acl-filter pattern ingress_dmac 00:00:39:00:74:00
   	aclRule.filter_fields |= INGRESS_DMAC_BIT;
	memcpy(&aclRule.ingress_dmac, entry->MacAddr, MAC_ADDR_LEN);
	//rg set acl-filter action action_type 3
	aclRule.action_type = ACL_ACTION_TYPE_QOS;
	//rg set acl-filter action qos action_ingress_vid 45
	aclRule.qos_actions |= ACL_ACTION_ACL_INGRESS_VID_BIT;
	aclRule.action_acl_ingress_vid = entry->vid;
	//rg add acl-filter entry
	if((ret = rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx)) == 0){
		fprintf(fp,"%d\n",aclIdx);
		//fprintf(stderr, "add %s-%d Vlan:%d, index=%d success\n",__func__,__LINE__,entry->vid, aclIdx);
	}else{
		fprintf(stderr,"%s-%d add rule failed! (ret = %d)\n",__func__,__LINE__, ret);
		fclose(fp);
		return -1;
	}
	//below permit rules will make ipv6 dest addr fe80::0 trap to cpu acl rules failure.
	//we add acls before below rules. (i don't know it have another side effect or not?)
/*
	rg clear acl-filter
	rg set acl-filter acl_weight 2
	rg set acl-filter fwding_type_and_direction 0
	rg set acl-filter action action_type 2
	rg set acl-filter pattern ingress_dest_ipv6_addr fe80:0000:0000:0000:0000:0000:0000:0000
	rg set acl-filter pattern ingress_dest_ipv6_addr_mask ffff:0000:0000:0000:0000:0000:0000:0000
	rg set acl-filter pattern ingress_port_mask 0x10
	rg add acl-filter entry
*/
	memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
	aclRule.fwding_type_and_direction =  ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET;
	aclRule.filter_fields |= INGRESS_PORT_BIT;
	aclRule.ingress_port_mask.portmask = RG_get_wan_phyPortMask();//wan port
	aclRule.acl_weight = RG_FIREWALL_ACL_WEIGHT;
	if(entry->vid > 0){
   		aclRule.filter_fields |= INGRESS_CTAGIF_BIT;
		aclRule.ingress_ctagIf = 1;
		//rg set acl-filter pattern ingress_ctag_vid 45
		aclRule.filter_fields |= INGRESS_CTAG_VID_BIT;
		aclRule.ingress_ctag_vid = entry->vid;
	}
	aclRule.filter_fields |= INGRESS_IPV6_TAGIF_BIT;
	aclRule.ingress_ipv6_tagif = 1;
	aclRule.filter_fields |= INGRESS_IPV6_DIP_BIT;
	aclRule.ingress_dest_ipv6_addr[0]=0xfe;
	aclRule.ingress_dest_ipv6_addr[1]=0x80;
	aclRule.ingress_dest_ipv6_addr_mask[0]=0xff;
	aclRule.ingress_dest_ipv6_addr_mask[1]=0xff;
	aclRule.action_type = ACL_ACTION_TYPE_TRAP;
	//rg add acl-filter entry
	if((ret = rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx)) == 0){
		fprintf(fp,"%d\n",aclIdx);
		//fprintf(stderr, "add %s-%d Vlan:%d, index=%d success\n",__func__,__LINE__,entry->vid, aclIdx);
	}else{
		fprintf(stderr,"%s-%d add rule failed! (ret = %d)\n",__func__,__LINE__, ret);
		fclose(fp);
		return -1;
	}

	//memcpy((void *)aclRule.ingress_dest_ipv6_addr, (void *)ip6addr ,sizeof(struct in6_addr));
	//memset((void *)aclRule.ingress_dest_ipv6_addr_mask, 0xff ,sizeof(struct in6_addr));

	//ipv4 only bridge will add acl to block ipv6 routing packet if vlan is the same
	//we add acl rules to permit ipv6 routing packet by dmac!
	memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
	//rg set acl-filter fwding_type_and_direction 0
	aclRule.fwding_type_and_direction =  ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET;
	//rg set acl-filter pattern ingress_port_mask 0x10
	aclRule.filter_fields |= INGRESS_PORT_BIT;
	aclRule.ingress_port_mask.portmask = RG_get_wan_phyPortMask();//wan port
	aclRule.acl_weight = RG_QOS_ACL_WEIGHT;
	if(entry->vid > 0){
		//rg set acl-filter pattern ingress_ctagIf 1
   		aclRule.filter_fields |= INGRESS_CTAGIF_BIT;
		aclRule.ingress_ctagIf = 1;
		//rg set acl-filter pattern ingress_ctag_vid 45
		aclRule.filter_fields |= INGRESS_CTAG_VID_BIT;
		aclRule.ingress_ctag_vid = entry->vid;
	}
	aclRule.filter_fields |= INGRESS_IPV6_TAGIF_BIT;
	aclRule.ingress_ipv6_tagif = 1;
	//rg set acl-filter pattern ingress_dmac 00:00:39:00:74:00
	aclRule.action_type = ACL_ACTION_TYPE_PERMIT;
   	aclRule.filter_fields |= INGRESS_DMAC_BIT;
	memcpy(&aclRule.ingress_dmac, entry->MacAddr, MAC_ADDR_LEN);
	//rg add acl-filter entry
	if((ret = rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx)) == 0){
		fprintf(fp,"%d\n",aclIdx);
		//fprintf(stderr, "add %s-%d Vlan:%d, index=%d success\n",__func__,__LINE__,entry->vid, aclIdx);
	}else{
		fprintf(stderr,"%s-%d add rule failed! (ret = %d)\n",__func__,__LINE__, ret);
		fclose(fp);
		return -1;
	}
	fclose(fp);

	return 0;

}

int RTK_RG_Set_ACL_IPV4_PPPoE_from_Wan_KeepOVID(MIB_CE_ATM_VC_Tp entry)
{

	FILE *fp;
	rtk_rg_aclFilterAndQos_t aclRule;
	int aclIdx, ret;
	rtk_rg_initParams_t init_param;
	bzero(&init_param, sizeof(rtk_rg_initParams_t));
	if((ret = rtk_rg_initParam_get(&init_param)) != SUCCESS)
	{
		fprintf(stderr, "rtk_rg_initParam_set failed! ret=%d\n", ret);
		return -1;
	}
	if(!(fp = fopen(RG_IPV4_PPPoE_From_Wan_KeepOVID_ACL_RILES, "a")))
	{
		fprintf(stderr, "ERROR! %s\n", strerror(errno));
		return -2;
	}
	//For IPv4 PPPoE(0x8863) Routing packet from Wan, keep original VID
	memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
	//rg set acl-filter fwding_type_and_direction 0
	aclRule.fwding_type_and_direction =  ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET;
	//rg set acl-filter pattern ingress_port_mask 0x10
	aclRule.filter_fields |= INGRESS_PORT_BIT;
	aclRule.ingress_port_mask.portmask = RG_get_wan_phyPortMask();//wan port
	aclRule.acl_weight = RG_DEFAULT_ACL_WEIGHT;
	if(entry->vid > 0){
		//rg set acl-filter pattern ingress_ctagIf 1
   		aclRule.filter_fields |= INGRESS_CTAGIF_BIT;
		aclRule.ingress_ctagIf = 1;
		//rg set acl-filter pattern ingress_ctag_vid 45
		aclRule.filter_fields |= INGRESS_CTAG_VID_BIT;
		aclRule.ingress_ctag_vid = entry->vid;
	}
	//rg set acl-filter pattern ingress_ethertype 0x8863
	aclRule.filter_fields |= INGRESS_ETHERTYPE_BIT;
	aclRule.ingress_ethertype = 0x8863;
	//rg set acl-filter pattern ingress_dmac 00:00:39:00:74:00
   	aclRule.filter_fields |= INGRESS_DMAC_BIT;
	memcpy(&aclRule.ingress_dmac, entry->MacAddr, MAC_ADDR_LEN);
	//rg set acl-filter action action_type 3
	aclRule.action_type = ACL_ACTION_TYPE_QOS;
	//rg set acl-filter action qos action_ingress_vid 45
	aclRule.qos_actions |= ACL_ACTION_ACL_INGRESS_VID_BIT;
	aclRule.action_acl_ingress_vid = entry->vid;
	//rg add acl-filter entry
	if((ret = rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx)) == 0){
		fprintf(fp,"%d\n",aclIdx);
		//fprintf(stderr, "add %s-%d Vlan:%d, index=%d success\n",__func__,__LINE__,entry->vid, aclIdx);
	}else{
		fprintf(stderr,"%s-%d add rule failed! (ret = %d)\n",__func__,__LINE__, ret);
		fclose(fp);
		return -1;
	}
	//For IPv4 PPPoE(0x8864) Routing packet from Wan,  keep original VID
	memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
	//rg set acl-filter fwding_type_and_direction 0
	aclRule.fwding_type_and_direction =  ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET;
	//rg set acl-filter pattern ingress_port_mask 0x10
	aclRule.filter_fields |= INGRESS_PORT_BIT;
	aclRule.ingress_port_mask.portmask = RG_get_wan_phyPortMask();//wan port
	aclRule.acl_weight = RG_DEFAULT_ACL_WEIGHT;
	if(entry->vid > 0){
		//rg set acl-filter pattern ingress_ctagIf 1
   		aclRule.filter_fields |= INGRESS_CTAGIF_BIT;
		aclRule.ingress_ctagIf = 1;
		//rg set acl-filter pattern ingress_ctag_vid 45
		aclRule.filter_fields |= INGRESS_CTAG_VID_BIT;
		aclRule.ingress_ctag_vid = entry->vid;
	}
	//rg set acl-filter pattern ingress_ethertype 0x8864
	aclRule.filter_fields |= INGRESS_ETHERTYPE_BIT;
	aclRule.ingress_ethertype = 0x8864;
	//rg set acl-filter pattern ingress_dmac 00:00:39:00:74:00
   	aclRule.filter_fields |= INGRESS_DMAC_BIT;
	memcpy(&aclRule.ingress_dmac, entry->MacAddr, MAC_ADDR_LEN);
	//rg set acl-filter action action_type 3
	aclRule.action_type = ACL_ACTION_TYPE_QOS;
	//rg set acl-filter action qos action_ingress_vid 45
	aclRule.qos_actions |= ACL_ACTION_ACL_INGRESS_VID_BIT;
	aclRule.action_acl_ingress_vid = entry->vid;
	//rg add acl-filter entry
	if((ret = rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx)) == 0){
		fprintf(fp,"%d\n",aclIdx);
		//fprintf(stderr, "add %s-%d Vlan:%d, index=%d success\n",__func__,__LINE__,entry->vid, aclIdx);
	}else{
		fprintf(stderr,"%s-%d add rule failed! (ret = %d)\n",__func__,__LINE__, ret);
		fclose(fp);
		return -1;
	}

	//ipv6 only bridge will add acl to block ipv4 routing packet if vlan is the same
	//we add acl rules to permit ipv4 routing packet by dmac!
	memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
	//rg set acl-filter fwding_type_and_direction 0
	aclRule.fwding_type_and_direction =  ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET;
	//rg set acl-filter pattern ingress_port_mask 0x10
	aclRule.filter_fields |= INGRESS_PORT_BIT;
	aclRule.ingress_port_mask.portmask = RG_get_wan_phyPortMask();//wan port
	aclRule.acl_weight = RG_QOS_ACL_WEIGHT;
	if(entry->vid > 0){
		//rg set acl-filter pattern ingress_ctagIf 1
   		aclRule.filter_fields |= INGRESS_CTAGIF_BIT;
		aclRule.ingress_ctagIf = 1;
		//rg set acl-filter pattern ingress_ctag_vid 45
		aclRule.filter_fields |= INGRESS_CTAG_VID_BIT;
		aclRule.ingress_ctag_vid = entry->vid;
	}
	aclRule.filter_fields |= INGRESS_IPV4_TAGIF_BIT;
	aclRule.ingress_ipv4_tagif = 1;
	//rg set acl-filter pattern ingress_dmac 00:00:39:00:74:00
	aclRule.action_type = ACL_ACTION_TYPE_PERMIT;
   	aclRule.filter_fields |= INGRESS_DMAC_BIT;
	memcpy(&aclRule.ingress_dmac, entry->MacAddr, MAC_ADDR_LEN);
	//rg add acl-filter entry
	if((ret = rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx)) == 0){
		fprintf(fp,"%d\n",aclIdx);
		//fprintf(stderr, "add %s-%d Vlan:%d, index=%d success\n",__func__,__LINE__,entry->vid, aclIdx);
	}else{
		fprintf(stderr,"%s-%d add rule failed! (ret = %d)\n",__func__,__LINE__, ret);
		fclose(fp);
		return -1;
	}
	fclose(fp);
	return 0;
}

int RTK_RG_FLUSH_Bridge_DHCP_ACL_FILE(void)
{
	FILE *fp;
	int aclIdx=-1;
	if(!(fp = fopen(RG_BRIDGE_INET_DHCP_RA_FILTER_FILE, "r"))){
		fprintf(stderr, "ERROR! %s\n", strerror(errno));
		return -2;
	}
	while(fscanf(fp, "%d\n", &aclIdx) != EOF)
	{
		//fprintf(stderr, "del mvlan index %d\n",aclIdx);
		if(rtk_rg_aclFilterAndQos_del(aclIdx))
			fprintf(stderr, "%s failed! idx = %d\n",__func__, aclIdx);
	}
	fclose(fp);
	unlink(RG_BRIDGE_INET_DHCP_RA_FILTER_FILE);
	return 0;


}
int RTK_RG_Set_ACL_Bridge_DHCP_Filter(void)
{
	FILE *fp;
	int acl_index=0, ret=-1;
	MIB_CE_ATM_VC_T entryVC;
	rtk_rg_aclFilterAndQos_t aclRule;
	int i, totalVC_entry = mib_chain_total(MIB_ATM_VC_TBL);
	for(i=0;i<totalVC_entry;i++){
		if(mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&entryVC) == 0)
			continue;

		if(entryVC.cmode == CHANNEL_MODE_BRIDGE && (entryVC.applicationtype & X_CT_SRV_INTERNET))
		{
			if(!(fp = fopen(RG_BRIDGE_INET_DHCP_RA_FILTER_FILE, "a"))){
				fprintf(stderr, "open %s fail!", RG_BRIDGE_INET_DHCP_RA_FILTER_FILE);
				return -2;
			}
			if(entryVC.IpProtocol & IPVER_IPV6){
				/*bridge internet wan, we don't wan to receive dhcp/RA offer from outside.*/
				/*we want to let lan get IP from gateway, so we set acl to filter*/
		 		//#drop from wan 3001 ipv6 ra
				//rg clear acl-filter
				//rg set acl-filter fwding_type_and_direction 0
				//rg set acl-filter action action_type 0
				//rg set acl-filter pattern ingress_ctag_vid 3001
				//rg set acl-filter pattern ingress_l4_protocal_value 0x3a
				//rg set acl-filter pattern ingress_port_mask 0x10
				//rg add acl-filter entry
				memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
				aclRule.fwding_type_and_direction = ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET;
				aclRule.acl_weight = RG_DEFAULT_ACL_WEIGHT;

				aclRule.action_type = ACL_ACTION_TYPE_TRAP_TO_PS;
				aclRule.filter_fields |= INGRESS_PORT_BIT;
		       	aclRule.filter_fields |= INGRESS_CTAGIF_BIT;
				aclRule.ingress_ctagIf = 1;
		       	aclRule.filter_fields |= INGRESS_CTAG_VID_BIT;
				aclRule.ingress_ctag_vid = entryVC.vid;	
				aclRule.filter_fields |= INGRESS_L4_POROTCAL_VALUE_BIT;
				aclRule.ingress_l4_protocal = 0x3a;
				aclRule.filter_fields |= INGRESS_PORT_BIT;
				aclRule.ingress_port_mask.portmask = RG_get_wan_phyPortMask();
				aclRule.filter_fields |= INGRESS_IPV6_TAGIF_BIT;
				aclRule.ingress_ipv6_tagif = 1;
				aclRule.filter_fields |= INGRESS_IPV6_DIP_BIT;
				aclRule.ingress_dest_ipv6_addr[0]=0xff;
				aclRule.ingress_dest_ipv6_addr[1]=0x02;
				aclRule.ingress_dest_ipv6_addr[15]=0x1;
				memset((void *)aclRule.ingress_dest_ipv6_addr_mask, 0xff ,sizeof(struct in6_addr));
				//aclRule.ingress_dest_ipv6_addr_mask[0]=0xff;
				//aclRule.ingress_dest_ipv6_addr_mask[1]=0xff;				
				if((ret = rtk_rg_aclFilterAndQos_add(&aclRule,&acl_index)) == 0)
					fprintf(fp, "%d\n", acl_index);
				else
					printf("rtk_rg_aclFilterAndQos_add QoS rule failed! (ret = %d)\n", ret);
				//downstream DHCPv6 
				memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
				aclRule.fwding_type_and_direction = ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET;
				aclRule.action_type = ACL_ACTION_TYPE_DROP;
				aclRule.filter_fields |= INGRESS_PORT_BIT;
		       	aclRule.filter_fields |= INGRESS_CTAGIF_BIT;
				aclRule.ingress_ctagIf = 1;
		       	aclRule.filter_fields |= INGRESS_CTAG_VID_BIT;
				aclRule.ingress_ctag_vid = entryVC.vid;	
				aclRule.filter_fields |= INGRESS_PORT_BIT;
				aclRule.ingress_port_mask.portmask = RG_get_wan_phyPortMask();
				aclRule.filter_fields |= INGRESS_ETHERTYPE_BIT;
				aclRule.ingress_ethertype = 0x86dd;//ipv6
				aclRule.filter_fields |= INGRESS_IPV6_TAGIF_BIT;
				aclRule.ingress_ipv6_tagif = 1;
				aclRule.filter_fields |= INGRESS_L4_SPORT_RANGE_BIT;
				aclRule.ingress_src_l4_port_start = 546;
				aclRule.ingress_src_l4_port_end = 546;
				aclRule.filter_fields |= INGRESS_L4_DPORT_RANGE_BIT;
				aclRule.ingress_dest_l4_port_start = 547;
				aclRule.ingress_dest_l4_port_end = 547;	
				if((ret = rtk_rg_aclFilterAndQos_add(&aclRule,&acl_index)) == 0)
					fprintf(fp, "%d\n", acl_index);
				else
					printf("rtk_rg_aclFilterAndQos_add QoS rule failed! (ret = %d)\n", ret);				
			}
			
			if(entryVC.IpProtocol & IPVER_IPV4){
				//rg clear acl-filter
				//rg set acl-filter fwding_type_and_direction 0
				//rg set acl-filter action action_type 0
				//rg set acl-filter pattern ingress_l4_protocal 0
				//rg set acl-filter pattern ingress_port_mask 0x10
				//rg set acl-filter pattern ingress_ctag_vid 3001
				//rg set acl-filter pattern ingress_src_l4_port_start 67 ingress_src_l4_port_end 67
				//rg set acl-filter pattern ingress_dest_l4_port_start 68 ingress_dest_l4_port_end 68
				//rg add acl-filter entry
				memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
				aclRule.fwding_type_and_direction = ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET;
				aclRule.action_type = ACL_ACTION_TYPE_DROP;
				aclRule.filter_fields |= INGRESS_PORT_BIT;
				aclRule.filter_fields |= INGRESS_CTAGIF_BIT;
				aclRule.ingress_ctagIf = 1;
		       	aclRule.filter_fields |= INGRESS_CTAG_VID_BIT;
				aclRule.ingress_ctag_vid = entryVC.vid;
				aclRule.filter_fields |= INGRESS_L4_UDP_BIT;
				aclRule.filter_fields |= INGRESS_PORT_BIT;
				aclRule.ingress_port_mask.portmask = RG_get_wan_phyPortMask();
				aclRule.filter_fields |= INGRESS_L4_SPORT_RANGE_BIT;
				aclRule.ingress_src_l4_port_start = 67;
				aclRule.ingress_src_l4_port_end = 67;
				aclRule.filter_fields |= INGRESS_L4_DPORT_RANGE_BIT;
				aclRule.ingress_dest_l4_port_start = 68;
				aclRule.ingress_dest_l4_port_end = 68;		
				if((ret = rtk_rg_aclFilterAndQos_add(&aclRule,&acl_index)) == 0)
					fprintf(fp, "%d\n", acl_index);
				else
					printf("rtk_rg_aclFilterAndQos_add QoS rule failed! (ret = %d)\n", ret);
			}
			fclose(fp);
		}
	}
	return ret;
}

int RG_add_wan(MIB_CE_ATM_VC_Tp entry, int mib_vc_idx)
{
	int wanIntfIdx;
	int vcTotal, i, vlan_id;
	char intf_name[10], mbtd;
	rtk_rg_wanIntfConf_t wan_info;
	unsigned char value[6];
	int ret=-1;
	int wanPhyPort=0;
	struct in_addr gw_addr;
    char cmdStr[64];
	int omci_mode=-1;
	int omci_service=-1;
	int omci_bind=-1;

	int pb_group=-1;
	unsigned int pon_mode=0;
	//Init_RG_ELan(UntagCPort, RoutingWan);
	int rtk_rg_wan_type = RG_get_wan_type(entry);

	if(rtk_rg_wan_type == -1)
		return -1;

	memset(&wan_info,0,sizeof(wan_info));
	memcpy(wan_info.gmac.octet, entry->MacAddr, MAC_ADDR_LEN);
	RG_WAN_CVLAN_DEL(entry->vid);

#if 0
	if(mib_get(MIB_WAN_PHY_PORT , (void *)&wanPhyPort) == 0){
		printf("get MIB_WAN_PHY_PORT failed!!!\n");
		wanPhyPort=RTK_RG_MAC_PORT3 ; //for 0371 default
	}
#else

	wanPhyPort=RG_get_wan_phyPortId();
#endif
	//wan_info.egress_vlan_id=8;
	//wan_info.vlan_based_pri=0;
	//wan_info.egress_vlan_tag_on=0;
	if (entry->vlan == 1) {
		wan_info.egress_vlan_tag_on=1;
		wan_info.egress_vlan_id=entry->vid;
	}
	else{
		wan_info.egress_vlan_tag_on=0;
		wan_info.egress_vlan_id=8;

		if(rtk_rg_wan_type == RTK_RG_BRIDGE)
		{
			mib_get(MIB_MAC_BASED_TAG_DECISION, (void *)&mbtd);

			if(mbtd == RTK_RG_DISABLED)
			{
				mib_get(MIB_LAN_VLAN_ID1, (void *)&vlan_id);
				wan_info.egress_vlan_id = vlan_id;
			}
		}
	}
#ifdef CONFIG_RTL9602C_SERIES
	wan_info.port_binding_mask.portmask = RG_get_lan_phyPortMask(entry->itfGroup & 0x3);
#else
	wan_info.port_binding_mask.portmask = RG_get_lan_phyPortMask(entry->itfGroup & 0xf);
#endif
	wan_info.wlan0_dev_binding_mask = ((entry->itfGroup & 0x1f0) >> 4);
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

#if 0//defined(CONFIG_GPON_FEATURE)
	unsigned int pon_mode;

	if (mib_get(MIB_PON_MODE, (void *)&pon_mode) != 0)
	{
		if ( pon_mode == GPON_MODE )
		{
			wan_info.gponStreamID = entry->sid;
			printf("GPON StreamID : %d.\n",wan_info.gponStreamID);
		}
	}
#endif
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
#ifdef CONFIG_GPON_FEATURE
	mib_get(MIB_PON_MODE, (void *)&pon_mode);
	if(pon_mode == GPON_MODE){
		char vlan_based_pri;
		if(entry->vprio)
		{
			vlan_based_pri=(entry->vprio)-1;
		}
		else
		{
			vlan_based_pri=-1;
		}
		//sync omci cf rules.
		/*untag wan, omci egress vlan id = -1*/
		if(entry->vlan == 2)
			wan_info.egress_vlan_id = 4095;
		else{
			if(!wan_info.egress_vlan_tag_on)
				wan_info.egress_vlan_id = -1;
		}
		snprintf(cmdStr, sizeof(cmdStr),"echo %d %d %d %d %d %d %d > %s",wanIntfIdx,wan_info.egress_vlan_id,vlan_based_pri,omci_mode,omci_service,omci_bind,1,OMCI_WAN_INFO);
		//fprintf(stderr, "%s\n",cmdStr);
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
/*When user enable pppoe routing/bridge mixed mode
    must add the acl rules for downstream path. or LAN
    pppoe passthrough will be dropped.
*/
int RG_del_PPPoE_Acl(void)
{
	FILE *fp;
	int aclIdx=-1;
    char cmdStr[64];
	if(!(fp = fopen(RG_ACL_PPPoE_PASS_RULES_FILE, "r"))){
		return -2;
	}
	while(fscanf(fp, "%d\n", &aclIdx) != EOF)
	{
		if(rtk_rg_aclFilterAndQos_del(aclIdx))
			fprintf(stderr, "rtk_rg_aclFilterAndQos_del failed! idx = %d\n", aclIdx);
	}
	fclose(fp);
	unlink(RG_ACL_PPPoE_PASS_RULES_FILE);
	snprintf(cmdStr, sizeof(cmdStr),"echo 0 > /proc/rg/pppoe_bc_passthrought_to_bindingWan");
	system(cmdStr);
	return 0;

}

int RG_add_PPPoE_RB_passthrough_Acl(void)
{
	MIB_CE_ATM_VC_T entry;
	int totalEntry = mib_chain_total(MIB_ATM_VC_TBL);
	int i,aclIdx=0, ret;
	rtk_rg_aclFilterAndQos_t aclRule;
	FILE *fp;
	int key=0;
    char cmdStr[64];
	rtk_rg_initParams_t init_param;
	bzero(&init_param, sizeof(rtk_rg_initParams_t));

	if((ret = rtk_rg_initParam_get(&init_param)) != SUCCESS)
	{
		fprintf(stderr, "rtk_rg_initParam_set failed! ret=%d\n", ret);
		return -1;
	}
//	fprintf(stderr,"[%s-%d] fwdVLAN_BIND_INTERNET=%d\n",__func__,__LINE__,init_param.fwdVLAN_BIND_INTERNET);
	if(!(fp = fopen(RG_ACL_PPPoE_PASS_RULES_FILE, "a")))
	{
		fprintf(stderr, "ERROR! %s\n", strerror(errno));
		return -2;
	}
	for (i = 0; i < totalEntry; i++)
	{
		if (mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&entry) == 0)
			continue;
		if(entry.cmode == CHANNEL_MODE_PPPOE && entry.brmode == BRIDGE_PPPOE){
			//ACL for pppoe passthrought support us by SW (session ID keep original)
			memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
			aclRule.fwding_type_and_direction =  ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET;
       		aclRule.acl_weight = RG_DEFAULT_ACL_WEIGHT;
        	aclRule.filter_fields |= INGRESS_PORT_BIT;
			#ifdef CONFIG_RTL9602C_SERIES
			aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0x3);
			#else
			aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0xf);
			#endif	
        	aclRule.filter_fields |= INGRESS_ETHERTYPE_BIT;
			aclRule.ingress_ethertype = 0x8863;
			aclRule.action_type = ACL_ACTION_TYPE_TRAP;
			if((ret = rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx)) == 0)
				fprintf(fp,"%d\n",aclIdx);
			else
				fprintf(stderr,"rtk_rg_aclFilterAndQos_add QoS rule failed! (ret = %d)\n", ret);

			memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
       		aclRule.acl_weight = RG_DEFAULT_ACL_WEIGHT;
        	aclRule.filter_fields |= INGRESS_PORT_BIT;
			#ifdef CONFIG_RTL9602C_SERIES
			aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0x3);
			#else
			aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0xf);
			#endif			
        	aclRule.filter_fields |= INGRESS_ETHERTYPE_BIT;
			aclRule.ingress_ethertype = 0x8864;
			aclRule.action_type = ACL_ACTION_TYPE_TRAP;
			if((ret = rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx)) == 0)
				fprintf(fp,"%d\n",aclIdx);
			else
				fprintf(stderr,"rtk_rg_aclFilterAndQos_add QoS rule failed! (ret = %d)\n", ret);

			//ACL for pppoe routing ds vlan keep original (avoid vlan translate for routing wan)
			memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
       		aclRule.acl_weight = RG_DEFAULT_ACL_WEIGHT;
        	aclRule.filter_fields |= INGRESS_PORT_BIT;
			aclRule.ingress_port_mask.portmask = RG_get_wan_phyPortMask();
        	aclRule.filter_fields |= INGRESS_ETHERTYPE_BIT;
			aclRule.ingress_ethertype = 0x8863;
        	aclRule.filter_fields |= INGRESS_DMAC_BIT;
			memcpy(&aclRule.ingress_dmac, entry.MacAddr, MAC_ADDR_LEN);
        	aclRule.filter_fields |= INGRESS_CTAGIF_BIT;
			aclRule.ingress_ctagIf = 1;
			aclRule.ingress_ctag_vid = entry.vid;
			aclRule.action_type = ACL_ACTION_TYPE_QOS;
			aclRule.qos_actions |= ACL_ACTION_ACL_INGRESS_VID_BIT;
			aclRule.action_acl_ingress_vid = entry.vid;
			if((ret = rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx)) == 0)
				fprintf(fp,"%d\n",aclIdx);
			else
				fprintf(stderr,"rtk_rg_aclFilterAndQos_add QoS rule failed! (ret = %d)\n", ret);

			memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
       		aclRule.acl_weight = RG_DEFAULT_ACL_WEIGHT;
        	aclRule.filter_fields |= INGRESS_PORT_BIT;
			aclRule.ingress_port_mask.portmask = RG_get_wan_phyPortMask();
        	aclRule.filter_fields |= INGRESS_ETHERTYPE_BIT;
			aclRule.ingress_ethertype = 0x8864;
        	aclRule.filter_fields |= INGRESS_DMAC_BIT;
			memcpy(&aclRule.ingress_dmac, entry.MacAddr, MAC_ADDR_LEN);
        	aclRule.filter_fields |= INGRESS_CTAGIF_BIT;
			aclRule.ingress_ctagIf = 1;
			aclRule.ingress_ctag_vid = entry.vid;
			aclRule.action_type = ACL_ACTION_TYPE_QOS;
			aclRule.qos_actions |= ACL_ACTION_ACL_INGRESS_VID_BIT;
			aclRule.action_acl_ingress_vid = entry.vid;
			if((ret = rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx)) == 0)
				fprintf(fp,"%d\n",aclIdx);
			else
				fprintf(stderr,"rtk_rg_aclFilterAndQos_add QoS rule failed! (ret = %d)\n", ret);

			//ACL for pppoe ds passthrough (vlan translate to lan vid to avoid vlan filter)
			memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
       		aclRule.acl_weight = RG_DEFAULT_ACL_WEIGHT;
        	aclRule.filter_fields |= INGRESS_PORT_BIT;
			aclRule.ingress_port_mask.portmask = RG_get_wan_phyPortMask();
        	aclRule.filter_fields |= INGRESS_ETHERTYPE_BIT;
			aclRule.ingress_ethertype = 0x8863;
        	aclRule.filter_fields |= INGRESS_CTAGIF_BIT;
			aclRule.ingress_ctagIf = 1;
			aclRule.ingress_ctag_vid = entry.vid;
			aclRule.action_type = ACL_ACTION_TYPE_QOS;
			aclRule.qos_actions |= ACL_ACTION_ACL_INGRESS_VID_BIT;
			aclRule.action_acl_ingress_vid = init_param.fwdVLAN_BIND_INTERNET; //lan interface's vlan
			if((ret = rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx)) == 0)
				fprintf(fp,"%d\n",aclIdx);
			else
				fprintf(stderr,"rtk_rg_aclFilterAndQos_add QoS rule failed! (ret = %d)\n", ret);

			memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
       		aclRule.acl_weight = RG_DEFAULT_ACL_WEIGHT;
        	aclRule.filter_fields |= INGRESS_PORT_BIT;
			aclRule.ingress_port_mask.portmask = RG_get_wan_phyPortMask();
        	aclRule.filter_fields |= INGRESS_ETHERTYPE_BIT;
			aclRule.ingress_ethertype = 0x8864;
        	aclRule.filter_fields |= INGRESS_CTAGIF_BIT;
			aclRule.ingress_ctagIf = 1;
			aclRule.ingress_ctag_vid = entry.vid;
			aclRule.action_type = ACL_ACTION_TYPE_QOS;
			aclRule.qos_actions |= ACL_ACTION_ACL_INGRESS_VID_BIT;
			aclRule.action_acl_ingress_vid = init_param.fwdVLAN_BIND_INTERNET; //lan interface's vlan
			if((ret = rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx)) == 0)
				fprintf(fp,"%d\n",aclIdx);
			else
				fprintf(stderr,"rtk_rg_aclFilterAndQos_add QoS rule failed! (ret = %d)\n", ret);

			key = 1;
		}
	}
	if(key){
		snprintf(cmdStr, sizeof(cmdStr),"echo 1 > /proc/rg/pppoe_bc_passthrought_to_bindingWan");
		system(cmdStr);
	}

	fclose(fp);
	return 0;

}

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

	memset(&staticInfo,0,sizeof(staticInfo));
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

	if((rtk_rg_staticInfo_set(wanIntfIdx, &staticInfo))!=SUCCESS)
		return -1;

	return SUCCESS;
}

#ifdef SUPPORT_ACCESS_RIGHT
int FlushRTK_RG_RT_INTERNET_ACCESS_RIGHT()
{
	FILE *fp;
	int mac_idx;

	if(!(fp = fopen(RG_INTERNET_ACCESS_RIGHT_RULES_FILE, "r")))
		return -2;

	while(fscanf(fp, "%d\n", &mac_idx) != EOF)
	{
		if(rtk_rg_macFilter_del(mac_idx))
			DBPRINT(1, "rtk_rg_macFilter_del failed! idx = %d\n", mac_idx);
	}

	fclose(fp);
	unlink(RG_INTERNET_ACCESS_RIGHT_RULES_FILE);
	return 0;
}


int AddRTK_RG_RT_INTERNET_ACCESS_RIGHT(unsigned char *smac)
{
	int macfilterIdx;
	rtk_rg_macFilterEntry_t macFilterEntry;
	FILE *fp;

	memset(&macFilterEntry, 0, sizeof(rtk_rg_macFilterEntry_t));
	memcpy(&macFilterEntry.mac, smac, MAC_ADDR_LEN);
	macFilterEntry.direct = RTK_RG_MACFILTER_FILTER_SRC_MAC_ONLY;
	
	if(!(fp = fopen(RG_INTERNET_ACCESS_RIGHT_RULES_FILE, "a")))
	{
		fprintf(stderr, "ERROR! %s\n", strerror(errno));
		return -1;
	}

	if(rtk_rg_macFilter_add(&macFilterEntry, &macfilterIdx) == 0)
		fprintf(fp, "%d\n", macfilterIdx);
	else
		printf("Set rtk_rg_macFilter_add failed! dir = Source\n");

	fclose(fp);
	return 0;
}
#endif

#ifdef CONFIG_USER_LAN_BANDWIDTH_CONTROL
int FlushRTK_RG_RT_MAX_US_BANDWIDTH(void)
{
#if 0//because 9607 hardware not support set bandwidth by MAC
	FILE *fp;
	int acl_idx;

	if(!(fp = fopen(RG_MAX_US_BANDWIDTH_FILE, "r")))
		return -2;

	while(fscanf(fp, "%d\n", &acl_idx) != EOF)
	{
		if(rtk_rg_aclFilterAndQos_del(acl_idx))
			DBPRINT(1, "rtk_rg_aclFilterAndQos_del failed! idx = %d\n", acl_idx);
	}

	fclose(fp);
	unlink(RG_MAX_US_BANDWIDTH_FILE);
	return 0;
#endif
}

int FlushRTK_RG_RT_MAX_DS_BANDWIDTH(void)
{
#if 0
	FILE *fp;
	int acl_idx;

	if(!(fp = fopen(RG_MAX_DS_BANDWIDTH_FILE, "r")))
		return -2;

	while(fscanf(fp, "%d\n", &acl_idx) != EOF)
	{
		if(rtk_rg_aclFilterAndQos_del(acl_idx))
			DBPRINT(1, "rtk_rg_aclFilterAndQos_del failed! idx = %d\n", acl_idx);
	}

	fclose(fp);
	unlink(RG_MAX_DS_BANDWIDTH_FILE);
	return 0;
#endif	
}

int AddRTK_RG_RT_MAX_US_BANDWIDTH(void)
{
#if 0
	int ret, i, totalNum, acl_index=0;
	unsigned int maxUsRate = 0;
	unsigned char shareMeterIdx = 0;
	FILE *fp;
	rtk_rg_aclFilterAndQos_t aclRule;

	MIB_LAN_HOST_BANDWIDTH_T entry;
	memset(&entry, 0 , sizeof(MIB_LAN_HOST_BANDWIDTH_T));
	for(i=0; i<totalNum; i++)
	{
		if(!mib_chain_get(MIB_LAN_HOST_BANDWIDTH_TBL, i, (void*)&entry))
			continue;
		
		if(entry.maxUsBandwidth == 0)
			continue;
		/* add ACL rule */
		if(!(fp = fopen(RG_MAX_US_BANDWIDTH_FILE, "a")))
		{
			fprintf(stderr, "ERROR! %s\n", strerror(errno));
			return -2;
		}

		maxUsRate = entry.maxUsBandwidth/8;
		/**************************************/
		shareMeterIdx = 0;/* how to set */
		/**************************************/
		
		ret = rtk_rg_shareMeter_set(shareMeterIdx, maxUsRate, RTK_RG_ENABLED);
		memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
		aclRule.fwding_type_and_direction = ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET;
		aclRule.action_type = ACL_ACTION_TYPE_QOS;
		aclRule.acl_weight = RG_DEFAULT_ACL_WEIGHT;
		aclRule.filter_fields |= INGRESS_SMAC_BIT;
		memcpy(&aclRule.ingress_smac.octet, entry.mac, MAC_ADDR_LEN);
		
		aclRule.qos_actions = ACL_ACTION_SHARE_METER_BIT;
		aclRule.action_share_meter = shareMeterIdx;
		if((ret = rtk_rg_aclFilterAndQos_add(&aclRule,&acl_index)) == 0)
			fprintf(fp, "%d\n", acl_index);
		else
			printf("rtk_rg_aclFilterAndQos_add QoS rule failed! (ret = %d)\n", ret);

		fclose(fp);
	}
#endif	
}

/* setup maxUsBandwidth */
int AddRTK_RG_RT_MAX_DS_BANDWIDTH(void)
{

}
#endif
//#define DBG_DHCP
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
	dhcpClient_info->hw_info.mtu=entry->mtu;

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

	if(entry->IpProtocol==IPVER_IPV4_IPV6){
		//If IPv6 is not ready, set IPv4 only
		if(memcmp(dhcpClient_info->hw_info.ipv6_addr.ipv6_addr,zeroIPv6.ipv6_addr,IPV6_ADDR_LEN)==0){
			dhcpClient_info->hw_info.ip_version = IPVER_V4ONLY;
		}
		else{
			dhcpClient_info->hw_info.ip_version = IPVER_V4V6;
		}
	}

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
	//printf("server_ip=0x%08X mib_pptp_idx=%d\n",server_ip,mib_pptp_idx);
	totalVC_entry = mib_chain_total(MIB_ATM_VC_TBL);
	for(i=0;i<totalVC_entry;i++){
		if(mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&entryVC) == 0)
			continue;
		ipAddr = hextol(entryVC.ipAddr);
		netMask = hextol(entryVC.netMask);
		printf("ipAddr=0x%08X netMask=0x%08X\n",ipAddr, netMask);
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

	if(vcEntry->IpProtocol == IPVER_IPV6)
		return -1;

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

	if(vcEntry->IpProtocol==IPVER_IPV4_IPV6){
		//If IPv6 is not ready, set IPv4 only
		if(memcmp( pppoeClientInfoA->hw_info.ipv6_addr.ipv6_addr,zeroIPv6.ipv6_addr,IPV6_ADDR_LEN)==0){
			pppoeClientInfoA->hw_info.ip_version = IPVER_V4ONLY;
		}
		else{
			pppoeClientInfoA->hw_info.ip_version = IPVER_V4V6;
		}
	}

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

	while(fscanf(fp, "%d\n", &lanIdx) != EOF)
	{
		if(rtk_rg_interface_del(lanIdx))
			DBPRINT(1, "RG_Del_All_LAN_Interfaces failed! (idx = %d)\n", lanIdx);
	}

	fclose(fp);
	unlink(RG_LAN_INF_IDX);
	return 0;
}

int RG_Interface_Del(int rg_intf_idx)
{
	rtk_rg_interface_del(rg_intf_idx);
}
int RG_WAN_Interface_Del(unsigned int rg_wan_idx)
{
	int ret=0;
	char cmdStr[64];
	printf("%s-%d del RG WAN[%d]\n",__func__,__LINE__,rg_wan_idx);
	snprintf(cmdStr, sizeof(cmdStr),"echo %d %d %d %d %d %d %d > %s",rg_wan_idx,0,0,0,0,0,0,OMCI_WAN_INFO);
	system(cmdStr);
	if(rtk_rg_interface_del(rg_wan_idx)){
		DBPRINT(1, "%s failed! (idx = %d)\n", __func__, rg_wan_idx);
		ret =-1;
	}
	return ret;
}
int RG_WAN_CVLAN_DEL(int vlanID)
{
	rtk_rg_cvlan_del(vlanID);
	return 0;
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
			//fprintf(stderr, "%s-%d key=%d, idx=%d\n",__func__,__LINE__,key,idx);
			mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry, idx);
			return 4;
		}

		//fprintf(stderr, "%s-%d key=%d, Entry.dgw=%d\n",__func__,__LINE__,key,Entry.dgw);

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

#ifdef CONFIG_MCAST_VLAN
char MCAST_ADDR[MAC_ADDR_LEN]={0x01,0x00,0x5E,0x00,0x00,0x00};
char MCAST_MASK[MAC_ADDR_LEN]={0xff,0xff,0xff,0x00,0x00,0x00};
char MCAST_ADDR_V6[MAC_ADDR_LEN]={0x33,0x33,0x00,0x00,0x00,0x00};
char MCAST_MASK_V6[MAC_ADDR_LEN]={0xff,0xff,0x00,0x00,0x00,0x00};

/*one mVlan can only be setted to one WAN*/
int RTK_RG_ACL_Handle_IGMP(FILE *fp, MIB_CE_ATM_VC_T *pentry)
{
	unsigned char mode,igmp_snoop_flag=0;
	rtk_rg_aclFilterAndQos_t aclRule;
	int i,aclIdx=0, ret;
	rtk_rg_initParams_t init_param;
	int port_idx=0;
	unsigned short itfGroup;
	unsigned int fwdcpu_vid;
	int dev_idx=0; /*only support master WLAN*/

	bzero(&init_param, sizeof(rtk_rg_initParams_t));
	if((ret = rtk_rg_initParam_get(&init_param)) != SUCCESS)
	{
		fprintf(stderr, "rtk_rg_initParam_set failed! ret=%d\n", ret);
		return -1;
	}

	//check igmp snooping is on/off
	mib_get(MIB_MPMODE, (void *)&mode);
	igmp_snoop_flag = (((mode&MP_IGMP_MASK)==MP_IGMP_MASK)?1:0);
//fprintf(stderr, "igmp_snoop_flag:%d\n",igmp_snoop_flag);
	if(!igmp_snoop_flag){
		memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
		//tranfser mVid to internal vid 1, to flood to all member!
		aclRule.filter_fields |= INGRESS_PORT_BIT;
		aclRule.ingress_port_mask.portmask = RG_get_wan_phyPortMask();//wan port
		aclRule.acl_weight = RG_DEFAULT_ACL_WEIGHT;
		aclRule.filter_fields |= INGRESS_CTAG_VID_BIT;
		if(pentry->mVid > 0)
			aclRule.ingress_ctag_vid = pentry->mVid;
		else
			aclRule.ingress_ctag_vid = pentry->vid;
		aclRule.filter_fields |= INGRESS_DMAC_BIT;
		memcpy(&aclRule.ingress_dmac,MCAST_ADDR,MAC_ADDR_LEN);
		memcpy(&aclRule.ingress_dmac_mask,MCAST_MASK,MAC_ADDR_LEN);
		aclRule.action_type = ACL_ACTION_TYPE_QOS;
		aclRule.qos_actions |= ACL_ACTION_ACL_INGRESS_VID_BIT;
		mib_get(MIB_FWD_CPU_VLAN_ID, (void *)&fwdcpu_vid);
		aclRule.action_acl_ingress_vid = fwdcpu_vid; //lan interface's vlan
		if((ret = rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx)) == 0){
			fprintf(fp,"%d\n",aclIdx);
			fprintf(stderr, "add mCast ACL Vlan:%d, index=%d success\n",pentry->mVid, aclIdx);
		}else{
			fprintf(stderr,"rtk_rg_aclFilterAndQos_add QoS rule failed! (ret = %d)\n", ret);
			return -1;
		}
		goto CHECK_V4_SNOOPING;
	}
	if(pentry->mVid > 0){
		//transfer multicast vlan to wan's vlan to avoid ingress vlan filter.
		memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
		aclRule.filter_fields |= INGRESS_PORT_BIT;
		aclRule.ingress_port_mask.portmask = RG_get_wan_phyPortMask();//wan port
		aclRule.acl_weight = RG_DEFAULT_ACL_WEIGHT;
		aclRule.filter_fields |= INGRESS_CTAG_VID_BIT;
		aclRule.ingress_ctag_vid = pentry->mVid; //multicast vlan 				
		aclRule.filter_fields |= INGRESS_DMAC_BIT;
		memcpy(&aclRule.ingress_dmac,MCAST_ADDR,MAC_ADDR_LEN);
		memcpy(&aclRule.ingress_dmac_mask,MCAST_MASK,MAC_ADDR_LEN);
		aclRule.action_type = ACL_ACTION_TYPE_QOS;
		aclRule.qos_actions |= ACL_ACTION_ACL_INGRESS_VID_BIT;
		//fprintf(stderr, "port_idx:%d\n",port_idx);
		aclRule.action_acl_ingress_vid = pentry->vid; //wan's vlan
		if((ret = rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx)) == 0){
			fprintf(fp,"%d\n",aclIdx);
			fprintf(stderr, "add mCast ACL Vlan:%d, index=%d success\n",pentry->mVid, aclIdx);
		}else{
			fprintf(stderr,"rtk_rg_aclFilterAndQos_add QoS rule failed! (ret = %d)\n", ret);
			return -1;
		}
	}
	#if 0
	port_idx=0;
	itfGroup = pentry->itfGroup;
	//printf("itfGroup:%x\n",itfGroup);
	//memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
	if(itfGroup > 0){
		while(itfGroup > 0){
			if(itfGroup & 1){
				memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
				aclRule.filter_fields |= INGRESS_PORT_BIT;
				aclRule.ingress_port_mask.portmask = RG_get_wan_phyPortMask();//wan port
				aclRule.acl_weight = RG_DEFAULT_ACL_WEIGHT;
				aclRule.filter_fields |= INGRESS_CTAG_VID_BIT;
				if(pentry->mVid)
					aclRule.ingress_ctag_vid = pentry->mVid;
				else
					aclRule.ingress_ctag_vid = pentry->vid;					
				aclRule.filter_fields |= INGRESS_DMAC_BIT;
				memcpy(&aclRule.ingress_dmac,MCAST_ADDR,MAC_ADDR_LEN);
				memcpy(&aclRule.ingress_dmac_mask,MCAST_MASK,MAC_ADDR_LEN);
				aclRule.action_type = ACL_ACTION_TYPE_QOS;
				aclRule.qos_actions |= ACL_ACTION_ACL_INGRESS_VID_BIT;
				//fprintf(stderr, "port_idx:%d\n",port_idx);
				RG_get_lan_phyPortId(port_idx);
				if(port_idx < 4){ //lan
					int phyID;
					phyID = RG_get_lan_phyPortId(port_idx);
					//fprintf(stderr, "%s-%d phyID:%d, logID=%d\n",__func__,__LINE__,phyID,port_idx);
					rtk_rg_portBasedCVlanId_get(phyID,&pPvid);
				}
				else{ //wlan
					int phyID;
					phyID = RG_get_wlan_phyPortId(port_idx);
					//fprintf(stderr, "%s-%d phyID:%d, logID=%d\n",__func__,__LINE__,phyID,port_idx);
					rtk_rg_wlanDevBasedCVlanId_get(phyID,dev_idx,&pPvid);
				}
				aclRule.action_acl_ingress_vid = pPvid; //lan interface's vlan
				if((ret = rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx)) == 0){
					fprintf(fp,"%d\n",aclIdx);
					fprintf(stderr, "add mCast ACL Vlan:%d, index=%d success\n",pentry->mVid, aclIdx);
				}else{
					fprintf(stderr,"rtk_rg_aclFilterAndQos_add QoS rule failed! (ret = %d)\n", ret);
					return -1;
				}
				break;
			}
			port_idx++;
			itfGroup=itfGroup>>1;
		}
	}else{
		memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
		//we have mVid, but not binding to any port.
		aclRule.filter_fields |= INGRESS_PORT_BIT;
		aclRule.ingress_port_mask.portmask = RG_get_wan_phyPortMask();//wan port
		aclRule.acl_weight = RG_DEFAULT_ACL_WEIGHT;
		aclRule.filter_fields |= INGRESS_CTAG_VID_BIT;
		if(pentry->mVid)
			aclRule.ingress_ctag_vid = pentry->mVid;
		else
			aclRule.ingress_ctag_vid = pentry->vid;
		aclRule.filter_fields |= INGRESS_DMAC_BIT;
		memcpy(&aclRule.ingress_dmac,MCAST_ADDR,MAC_ADDR_LEN);
		memcpy(&aclRule.ingress_dmac_mask,MCAST_MASK,MAC_ADDR_LEN);
		aclRule.action_type = ACL_ACTION_TYPE_QOS;
		aclRule.qos_actions |= ACL_ACTION_ACL_INGRESS_VID_BIT;
		//rtk_rg_portBasedCVlanId_get(port_idx,&pPvid);
		if(pentry->applicationtype & X_CT_SRV_INTERNET)
			aclRule.action_acl_ingress_vid = init_param.fwdVLAN_BIND_INTERNET; //lan interface's vlan
		else
			aclRule.action_acl_ingress_vid = pentry->vid;//init_param.fwdVLAN_BIND_OTHER; //lan interface's vlan
		if((ret = rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx)) == 0){
			fprintf(fp,"%d\n",aclIdx);
		//fprintf(stderr, "add mCast ACL Vlan:%d, index=%d success\n",entry.mVid, aclIdx);
		}else{
			fprintf(stderr,"rtk_rg_aclFilterAndQos_add QoS rule failed! (ret = %d)\n", ret);
			return -1;
		}
	}
	#endif
	CHECK_V4_SNOOPING:
	return 0;

}

int RTK_RG_ACL_Handle_MLD(FILE *fp,MIB_CE_ATM_VC_T *pentry)
{
	unsigned char mode;
	unsigned char mld_snoop_flag=0;
	rtk_rg_aclFilterAndQos_t aclRule;
	int i,aclIdx=0, ret;
	rtk_rg_initParams_t init_param;
	int port_idx=0;
	unsigned short itfGroup;
	unsigned int fwdcpu_vid;	
	int dev_idx=0; /*only support master WLAN*/
	bzero(&init_param, sizeof(rtk_rg_initParams_t));
	if((ret = rtk_rg_initParam_get(&init_param)) != SUCCESS)
	{
		fprintf(stderr, "rtk_rg_initParam_set failed! ret=%d\n", ret);
		return -1;
	}
	//check mld snooping is on/off
	mib_get(MIB_MPMODE, (void *)&mode);
	mld_snoop_flag = (((mode&MP_MLD_MASK)==MP_MLD_MASK)?1:0);
//fprintf(stderr, "%s-%d mld_snoop_flag=%d\n",__func__,__LINE__,mld_snoop_flag);

	//handle multicast v6
	if(!mld_snoop_flag){
		//multicast ipv6
		memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
		//tranfser mVid to internal vid 1, to flood to all member!
		aclRule.filter_fields |= INGRESS_PORT_BIT;
		aclRule.ingress_port_mask.portmask = RG_get_wan_phyPortMask();//wan port
		aclRule.acl_weight = RG_DEFAULT_ACL_WEIGHT;
		aclRule.filter_fields |= INGRESS_CTAG_VID_BIT;
		if(pentry->mVid > 0)
			aclRule.ingress_ctag_vid = pentry->mVid;
		else
			aclRule.ingress_ctag_vid = pentry->vid;
		aclRule.filter_fields |= INGRESS_DMAC_BIT;
		memcpy(&aclRule.ingress_dmac,MCAST_ADDR_V6,MAC_ADDR_LEN);
		memcpy(&aclRule.ingress_dmac_mask,MCAST_MASK_V6,MAC_ADDR_LEN);
		aclRule.action_type = ACL_ACTION_TYPE_QOS;
		aclRule.qos_actions |= ACL_ACTION_ACL_INGRESS_VID_BIT;
		mib_get(MIB_FWD_CPU_VLAN_ID, (void *)&fwdcpu_vid);
		aclRule.action_acl_ingress_vid = fwdcpu_vid; //lan interface's vlan
		if((ret = rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx)) == 0){
			fprintf(fp,"%d\n",aclIdx);
			fprintf(stderr, "%s-%d add mCast ACL Vlan:%d, index=%d success\n",__func__,__LINE__,pentry->mVid, aclIdx);
		}else{
			fprintf(stderr,"rtk_rg_aclFilterAndQos_add QoS rule failed! (ret = %d)\n", ret);
			return -1;
		}
		goto CHECK_V6_SNOOPING;
	}
	if(pentry->mVid > 0)
	{
		//multicast ipv6, mld
		memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
		aclRule.filter_fields |= INGRESS_PORT_BIT;
		aclRule.ingress_port_mask.portmask = RG_get_wan_phyPortMask();//wan port
		aclRule.acl_weight = RG_DEFAULT_ACL_WEIGHT;
		aclRule.filter_fields |= INGRESS_CTAG_VID_BIT;
		aclRule.ingress_ctag_vid = pentry->mVid;
		aclRule.filter_fields |= INGRESS_DMAC_BIT;
		memcpy(&aclRule.ingress_dmac,MCAST_ADDR_V6,MAC_ADDR_LEN);
		memcpy(&aclRule.ingress_dmac_mask,MCAST_MASK_V6,MAC_ADDR_LEN);
		aclRule.action_type = ACL_ACTION_TYPE_QOS;
		aclRule.qos_actions |= ACL_ACTION_ACL_INGRESS_VID_BIT;
		aclRule.action_acl_ingress_vid = pentry->vid; //wan interface's vlan
		if((ret = rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx)) == 0){
			fprintf(fp,"%d\n",aclIdx);
			fprintf(stderr, "%s-%d add mCast ACL Vlan:%d, index=%d success\n",__func__,__LINE__,pentry->mVid, aclIdx);
		}else{
			fprintf(stderr,"rtk_rg_aclFilterAndQos_add QoS rule failed! (ret = %d)\n", ret);
			return -1;
		}
	}
	#if 0
	port_idx=0;
	itfGroup = pentry->itfGroup;
	//printf("itfGroup:%x\n",itfGroup);
	//memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
	if(itfGroup > 0){
		while(itfGroup > 0){
			if(itfGroup & 1){
				//multicast ipv6, mld
				memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
				aclRule.filter_fields |= INGRESS_PORT_BIT;
				aclRule.ingress_port_mask.portmask = RG_get_wan_phyPortMask();//wan port
				aclRule.acl_weight = RG_DEFAULT_ACL_WEIGHT;
				aclRule.filter_fields |= INGRESS_CTAG_VID_BIT;
				aclRule.ingress_ctag_vid = pentry->mVid;
				aclRule.filter_fields |= INGRESS_DMAC_BIT;
				memcpy(&aclRule.ingress_dmac,MCAST_ADDR_V6,MAC_ADDR_LEN);
				memcpy(&aclRule.ingress_dmac_mask,MCAST_MASK_V6,MAC_ADDR_LEN);
				aclRule.action_type = ACL_ACTION_TYPE_QOS;
				aclRule.qos_actions |= ACL_ACTION_ACL_INGRESS_VID_BIT;
				//fprintf(stderr, "port_idx:%d\n",port_idx);
				if(port_idx < 4){ //lan
					int phyID;
					phyID = RG_get_lan_phyPortId(port_idx);
					//fprintf(stderr, "%s-%d phyID:%d, logID=%d\n",__func__,__LINE__,phyID,port_idx);
					rtk_rg_portBasedCVlanId_get(phyID,&pPvid);
				}
				else{ //wlan
					int phyID;
					phyID = RG_get_wlan_phyPortId(port_idx);
					//fprintf(stderr, "%s-%d phyID:%d, logID=%d\n",__func__,__LINE__,phyID,port_idx);
					rtk_rg_wlanDevBasedCVlanId_get(port_idx-4,dev_idx,&pPvid);
				}

				aclRule.action_acl_ingress_vid = pPvid; //lan interface's vlan
				if((ret = rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx)) == 0){
					fprintf(fp,"%d\n",aclIdx);
					fprintf(stderr, "%s-%d add mCast ACL Vlan:%d, index=%d success\n",__func__,__LINE__,pentry->mVid, aclIdx);
				}else{
					fprintf(stderr,"rtk_rg_aclFilterAndQos_add QoS rule failed! (ret = %d)\n", ret);
					return -1;
				}
				break;
			}
			port_idx++;
			itfGroup=itfGroup>>1;
		}
	}else{
			//multicast ipv6, mld
			memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
			//we have mVid, but not binding to any port.
			aclRule.filter_fields |= INGRESS_PORT_BIT;
			aclRule.ingress_port_mask.portmask = RG_get_wan_phyPortMask();//wan port
			aclRule.acl_weight = RG_DEFAULT_ACL_WEIGHT;
			aclRule.filter_fields |= INGRESS_CTAG_VID_BIT;
			aclRule.ingress_ctag_vid = pentry->mVid;
			aclRule.filter_fields |= INGRESS_DMAC_BIT;
			memcpy(&aclRule.ingress_dmac,MCAST_ADDR_V6,MAC_ADDR_LEN);
			memcpy(&aclRule.ingress_dmac_mask,MCAST_MASK_V6,MAC_ADDR_LEN);
			aclRule.action_type = ACL_ACTION_TYPE_QOS;
			aclRule.qos_actions |= ACL_ACTION_ACL_INGRESS_VID_BIT;
			//rtk_rg_portBasedCVlanId_get(port_idx,&pPvid);
			if(pentry->applicationtype & X_CT_SRV_INTERNET)
				aclRule.action_acl_ingress_vid = init_param.fwdVLAN_BIND_INTERNET; //lan interface's vlan
			else
				aclRule.action_acl_ingress_vid = init_param.fwdVLAN_BIND_OTHER; //lan interface's vlan
			if((ret = rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx)) == 0){
				fprintf(fp,"%d\n",aclIdx);
				fprintf(stderr, "%s-%d add mCast ACL Vlan:%d, index=%d success\n",__func__,__LINE__,pentry->mVid, aclIdx);
			}else{
				fprintf(stderr,"rtk_rg_aclFilterAndQos_add QoS rule failed! (ret = %d)\n", ret);
				return -1;
			}

	}
	#endif
	CHECK_V6_SNOOPING:
	return 0;

}

int RTK_RG_ACL_Add_mVlan(void)
{
	MIB_CE_ATM_VC_T entry;
	unsigned char mode,igmp_snoop_flag=0;
	int port_idx=0;
	int pPvid;
	int totalEntry = mib_chain_total(MIB_ATM_VC_TBL);
	int i,aclIdx=0, ret;
	unsigned short itfGroup;
	rtk_rg_aclFilterAndQos_t aclRule;
	FILE *fp;
	int key=0;
    char cmdStr[64];
	rtk_rg_initParams_t init_param;
	int wlan_idx=0;
	int dev_idx=0; /*only support master WLAN*/
	rtk_rg_gpon_ds_bc_vlanfilterAndRemarking_t filterRule;
	int ds_bc_index=-1;
	unsigned char mldproxyEnable=0;
	unsigned int mldproxyItf=0;
	int setup_ds_bc_flag=0;
	if(!(fp = fopen(RG_ACL_MVLAN_RULES_FILE, "a")))
	{
		fprintf(stderr, "ERROR! %s\n", strerror(errno));
		return -2;
	}
	for (i = 0; i < totalEntry; i++)
	{
		ds_bc_index=-1;
		if (mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&entry) == 0)
			continue;
		//if(entry.mVid > 0){
//fprintf(stderr, "%s-%d entry.mVid=%d IpProtocol=%d\n",__func__,__LINE__, entry.mVid, entry.IpProtocol);
			if(entry.IpProtocol & IPVER_IPV4){
//fprintf(stderr, "%s-%d enableIGMP=%d\n",__func__,__LINE__, entry.enableIGMP);
				RTK_RG_ACL_Handle_IGMP(fp,&entry);
#if 0
				if(entry.enableIGMP){
					if(entry.mVid > 0){
					//for igmp query or mld query~
					memset(&filterRule, 0, sizeof(rtk_rg_gpon_ds_bc_vlanfilterAndRemarking_t));
					//rg set gponDsBcFilter pattern ingress_ctagIf 1
					filterRule.filter_fields |= GPON_DS_BC_FILTER_INGRESS_CTAGIf_BIT;
					filterRule.ingress_ctagIf = 1;
					//rg set gponDsBcFilter pattern ingress_ctag_cvid 600
					filterRule.filter_fields |= GPON_DS_BC_FILTER_INGRESS_CVID_BIT;
					filterRule.ingress_ctag_cvid = entry.mVid;
					//rg set gponDsBcFilter pattern egress_portmask 0x40
					//mvid 600 to cpu --> tag vid44
					filterRule.filter_fields |= GPON_DS_BC_FILTER_EGRESS_PORT_BIT;
					filterRule.egress_portmask.portmask |= RTK_RG_MAC_PORT_CPU;//cpu port
					//rg set gponDsBcFilter action tag_decision 1 tag_cvid 44 tag_cpri 0
					filterRule.ctag_action.ctag_decision = RTK_RG_GPON_BC_FORCE_TAGGIN_WITH_CVID;
					filterRule.ctag_action.assigned_ctag_cvid = entry.vid;
					filterRule.ctag_action.assigned_ctag_cpri = 0;
					rtk_rg_gponDsBcFilterAndRemarking_add(&filterRule, &ds_bc_index);
//fprintf(stderr, "%s-%d ds_bc_index=%d\n",__func__,__LINE__,ds_bc_index);
					setup_ds_bc_flag = 1;
					}
				}
#endif				
			}
			if(entry.IpProtocol & IPVER_IPV6){
				RTK_RG_ACL_Handle_MLD(fp,&entry);
#if 0
				mib_get(MIB_MLD_PROXY_DAEMON, (void *)&mldproxyEnable);
//fprintf(stderr, "%s-%d mldproxyEnable=%d\n",__func__,__LINE__, mldproxyEnable);
				if(mldproxyEnable){
					if(entry.mVid > 0){
					mib_get(MIB_MLD_PROXY_EXT_ITF, (void *)&mldproxyItf);
//fprintf(stderr, "%s-%d mldproxyItf=0x%x setup_ds_bc_flag=%d\n",__func__,__LINE__,mldproxyItf,setup_ds_bc_flag);
					//setup_ds_bc_flag --> if wan has protocol IPv4_IPv6
					//, don't set the same rules twice!
						if((entry.ifIndex == mldproxyItf) && !setup_ds_bc_flag)
						{
							//for igmp query or mld query~
							memset(&filterRule, 0, sizeof(rtk_rg_gpon_ds_bc_vlanfilterAndRemarking_t));
							//rg set gponDsBcFilter pattern ingress_ctagIf 1
							filterRule.filter_fields |= GPON_DS_BC_FILTER_INGRESS_CTAGIf_BIT;
							filterRule.ingress_ctagIf = 1;
							//rg set gponDsBcFilter pattern ingress_ctag_cvid 600
							filterRule.filter_fields |= GPON_DS_BC_FILTER_INGRESS_CVID_BIT;
							filterRule.ingress_ctag_cvid = entry.mVid;
							//rg set gponDsBcFilter pattern egress_portmask 0x40
							//mvid 600 to cpu --> tag vid44
							filterRule.filter_fields |= GPON_DS_BC_FILTER_EGRESS_PORT_BIT;
							filterRule.egress_portmask.portmask |= RTK_RG_MAC_PORT_CPU;//cpu port
							//rg set gponDsBcFilter action tag_decision 1 tag_cvid 44 tag_cpri 0
							filterRule.ctag_action.ctag_decision = RTK_RG_GPON_BC_FORCE_TAGGIN_WITH_CVID;
							filterRule.ctag_action.assigned_ctag_cvid = entry.vid;
							filterRule.ctag_action.assigned_ctag_cpri = 0;
							rtk_rg_gponDsBcFilterAndRemarking_add(&filterRule, &ds_bc_index);
							//fprintf(stderr, "%s-%d ds_bc_index=%d\n",__func__,__LINE__,ds_bc_index);
						}
					}
				}
#endif				
			}
//			setup_ds_bc_flag = 0;
#if 0
			memset(&filterRule, 0, sizeof(rtk_rg_gpon_ds_bc_vlanfilterAndRemarking_t));
			//rg set gponDsBcFilter pattern ingress_ctagIf 1
			filterRule.filter_fields |= GPON_DS_BC_FILTER_INGRESS_CTAGIf_BIT;
			filterRule.ingress_ctagIf = 1;
			//rg set gponDsBcFilter pattern ingress_ctag_cvid 600
			filterRule.ingress_ctag_cvid = entry->mVid;
			//rg set gponDsBcFilter pattern egress_portmask 0x40
			//mvid 600 to internal  --> untag
			filterRule.egress_portmask.portmask = 0x40;
			//rg set gponDsBcFilter action tag_decision 1 tag_cvid 0 tag_cpri 0
			filterRule.ctag_action.ctag_decision = RTK_RG_GPON_BC_FORCE_UNATG;
			filterRule.ctag_action.assigned_ctag_cvid = 0;
			filterRule.ctag_action.assigned_ctag_cpri = 0;
			rtk_rg_apollo_gponDsBcFilterAndRemarking_add(&filterRule, &ds_bc_index);
#endif
//fprintf(stderr, "%s-%d setup_ds_bc_flag=%d\n",__func__,__LINE__, setup_ds_bc_flag);
		//}
	}
	fclose(fp);
	return 0;
}

int RTK_RG_FlushDsBcFilter_Rules(void)
{
	int i;
	for(i=0;i<128;i++)
		rtk_rg_gponDsBcFilterAndRemarking_del(i);
	return 0;
}
int RTK_RG_ACL_Flush_mVlan(void)
{

	FILE *fp;
	int aclIdx=-1;
	if(!(fp = fopen(RG_ACL_MVLAN_RULES_FILE, "r"))){
		fprintf(stderr, "ERROR! %s\n", strerror(errno));
		return -2;
	}
	while(fscanf(fp, "%d\n", &aclIdx) != EOF)
	{
		//fprintf(stderr, "del mvlan index %d\n",aclIdx);
		if(rtk_rg_aclFilterAndQos_del(aclIdx))
			fprintf(stderr, "%s failed! idx = %d\n",__func__, aclIdx);
	}
	fclose(fp);
	unlink(RG_ACL_MVLAN_RULES_FILE);
	RTK_RG_FlushDsBcFilter_Rules();
	return 0;
}
#endif /*CONFIG_MCAST_VLAN*/

#ifdef MAC_FILTER
int AddRTK_RG_MAC_Filter(MIB_CE_MAC_FILTER_T *MacEntry)
{
	int macfilterIdx;
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

	if(rtk_rg_macFilter_add(&macFilterEntry, &macfilterIdx) == 0)
		fprintf(fp, "%d\n", macfilterIdx);
	else
		printf("Set rtk_rg_macFilter_add failed! dir = %d\n", MacEntry->dir? MacEntry->dir == 1? "Source": "Destination": "Both");

	fclose(fp);
	return 0;
#if 0
	rtk_rg_aclFilterAndQos_t aclRule;
	rtk_rg_intfInfo_t infinfo;
	int aclIdx, i;
	FILE *fp;

	memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));

	if (MacEntry->action == 0)
		aclRule.action_type = ACL_ACTION_TYPE_DROP;
	else if (MacEntry->action == 1)
		aclRule.action_type = ACL_ACTION_TYPE_PERMIT;
	else
	{
		fprintf(stderr, "Wrong mac filter action!\n");
		return -1;
	}

	if (memcmp(MacEntry->srcMac, "\x00\x00\x00\x00\x00\x00", MAC_ADDR_LEN))  // src mac is not empty.
	{
		aclRule.filter_fields |= INGRESS_SMAC_BIT;
		memcpy(&aclRule.ingress_smac, MacEntry->srcMac, MAC_ADDR_LEN);
	}

	if(memcmp(MacEntry->dstMac, "\x00\x00\x00\x00\x00\x00", MAC_ADDR_LEN))  // dst mac is not empty.
	{
		aclRule.filter_fields |= INGRESS_DMAC_BIT;
		memcpy(&aclRule.ingress_dmac, MacEntry->dstMac, MAC_ADDR_LEN);
	}

	if(!(fp = fopen(ACL_MAC_RULES_FILE, "a")))
	{
		fprintf(stderr, "ERROR! %s\n", strerror(errno));
		return -1;
	}

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

	fclose(fp);
#endif
}

int AddRTK_RG_BG_MAC_Filter(MIB_CE_BRGMAC_T *MacEntry, unsigned char *smac, unsigned char *dmac, unsigned char macFilterMode, int ethertype)
{
#if 0
	int macfilterIdx;
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

	if(rtk_rg_macFilter_add(&macFilterEntry, &macfilterIdx) == 0)
		fprintf(fp, "%d\n", macfilterIdx);
	else
		printf("Set rtk_rg_macFilter_add failed! dir = %d\n", MacEntry->dir? MacEntry->dir == 1? "Source": "Destination": "Both");

	fclose(fp);
	return 0;
#endif
#if 1
	rtk_rg_aclFilterAndQos_t aclRule;
	rtk_rg_intfInfo_t infinfo;
	int aclIdx, i;
	FILE *fp;
	unsigned int wanPhyPort;
	MIB_CE_ATM_VC_T vc_entry;

	memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));

	if (macFilterMode == 0)
		aclRule.action_type = ACL_ACTION_TYPE_DROP;
	else if (macFilterMode == 1)
		aclRule.action_type = ACL_ACTION_TYPE_PERMIT;
	else
	{
		fprintf(stderr, "Wrong mac filter action!\n");
		return -1;
	}

	//if (memcmp(MacEntry->smac, "\x00\x00\x00\x00\x00\x00", MAC_ADDR_LEN))  // src mac is not empty.
	if(strlen(MacEntry->smac))
	{
		aclRule.filter_fields |= INGRESS_SMAC_BIT;
		memcpy(&aclRule.ingress_smac, smac, MAC_ADDR_LEN);
	}

	//if(memcmp(MacEntry->dmac, "\x00\x00\x00\x00\x00\x00", MAC_ADDR_LEN))  // dst mac is not empty.
	if(strlen(MacEntry->dmac))
	{
		aclRule.filter_fields |= INGRESS_DMAC_BIT;
		memcpy(&aclRule.ingress_dmac, dmac, MAC_ADDR_LEN);
	}

	if(ethertype!=0)
	{
		aclRule.filter_fields |= INGRESS_ETHERTYPE_BIT;
		aclRule.ingress_ethertype = ethertype;
	}

	if(!(fp = fopen(RG_MAC_ACL_RULES_FILE, "a")))
	{
		fprintf(stderr, "ERROR! %s\n", strerror(errno));
		return -1;
	}

	int itfindex=0;
	for(itfindex=0;itfindex<MacEntry->portNum;itfindex++)
	{

		if(getATMVCEntryByIfIndex(MacEntry->ifIndex[itfindex], &vc_entry) == NULL)
			return -1;
		i = vc_entry.rg_wan_idx;

		if(rtk_rg_intfInfo_find(&infinfo, &i))
			break;

		if(infinfo.is_wan && infinfo.wan_intf.wan_intf_conf.wan_type == RTK_RG_BRIDGE)
		{
			if(MacEntry->direction == DIR_OUT || MacEntry->direction == 2)
			{
				if(macFilterMode == 0){
					aclRule.fwding_type_and_direction = ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP;
					aclRule.filter_fields |= (EGRESS_INTF_BIT|INGRESS_PORT_BIT);
					aclRule.egress_intf_idx = i;  // Set egress interface to WAN.
#ifdef CONFIG_RTL9602C_SERIES
					aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0x3);
#else
					aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0xf); //lan port
#endif
					if(rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx) == 0)
						fprintf(fp, "%d\n", aclIdx);
					else
						printf("Set rtk_rg_aclFilterAndQos_add failed! dir = Outgoing\n");
				}
				else
					printf("%s not support permit rule in RG for dir = Outgoing\n", __FUNCTION__);
			}

			if(MacEntry->direction == DIR_IN || MacEntry->direction == 2)
			{
				aclRule.fwding_type_and_direction = ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET;
				aclRule.filter_fields &= ~(EGRESS_INTF_BIT);
				aclRule.filter_fields |= (INGRESS_INTF_BIT|INGRESS_PORT_BIT);
				aclRule.ingress_intf_idx = i;  // Set ingress interface to WAN.

				#if 0
				if(mib_get(MIB_WAN_PHY_PORT , (void *)&wanPhyPort) == 0){
					printf("get MIB_WAN_PHY_PORT failed!!!\n");
					wanPhyPort = RTK_RG_MAC_PORT_PON;
				}
				aclRule.ingress_port_mask.portmask = 1 << wanPhyPort; //wan port
				#endif

				aclRule.ingress_port_mask.portmask = RG_get_wan_phyPortMask();

				if(rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx) == 0)
					fprintf(fp, "%d\n", aclIdx);
				else
					printf("Set rtk_rg_aclFilterAndQos_add failed! dir = Incoming\n");
			}

			if(MacEntry->direction > 2)
			{
				DBPRINT(1, "Invalid MAC filtering direction!\n");
				return -1;
			}

		}
	}

	fclose(fp);
	return 0;
#endif
}

int AddRTK_RG_RT_MAC_Filter(unsigned char *smac)
{
	int macfilterIdx;
	rtk_rg_macFilterEntry_t macFilterEntry;
	FILE *fp;

	memset(&macFilterEntry, 0, sizeof(rtk_rg_macFilterEntry_t));
	memcpy(&macFilterEntry.mac, smac, MAC_ADDR_LEN);
	macFilterEntry.direct = RTK_RG_MACFILTER_FILTER_SRC_MAC_ONLY;
	//printf("%x %x %x %x %x %x\n", macFilterEntry.mac.octet[0], macFilterEntry.mac.octet[1], macFilterEntry.mac.octet[2]
	//	, macFilterEntry.mac.octet[3], macFilterEntry.mac.octet[4], macFilterEntry.mac.octet[5]);

	if(!(fp = fopen(RG_MAC_RULES_FILE, "a")))
	{
		fprintf(stderr, "ERROR! %s\n", strerror(errno));
		return -1;
	}

	if(rtk_rg_macFilter_add(&macFilterEntry, &macfilterIdx) == 0)
		fprintf(fp, "%d\n", macfilterIdx);
	else
		printf("Set rtk_rg_macFilter_add failed! dir = Source\n");

	fclose(fp);
	return 0;
}

int RTK_RG_MAC_Filter_Default_Policy(int out_policy, int in_policy)
{
	rtk_rg_aclFilterAndQos_t aclRule;
	rtk_rg_intfInfo_t infinfo;
	int aclIdx, i;
	FILE *fp;
	unsigned int wanPhyPort;
	int ret;

	memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));

	if(!(fp = fopen(RG_MAC_ACL_RULES_FILE, "a")))
	{
		fprintf(stderr, "ERROR! %s\n", strerror(errno));
		return -1;
	}

	if( out_policy == 0 || in_policy == 0 )
	{
		aclRule.action_type = ACL_ACTION_TYPE_DROP;

		for( i = 0; i < MAX_NETIF_SW_TABLE_SIZE; i++ )
		{
			memset(&infinfo, 0, sizeof(rtk_rg_intfInfo_t));
			if(rtk_rg_intfInfo_find(&infinfo, &i))
				break;

			DBPRINT(0, "i=%d; is_wan=%d; intf_name=%s\n", i, infinfo.is_wan, infinfo.intf_name);
			if(infinfo.is_wan && infinfo.wan_intf.wan_intf_conf.wan_type == RTK_RG_BRIDGE)
			{

				if(out_policy == 0)
				{
					aclRule.fwding_type_and_direction = ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP;
					aclRule.filter_fields = (EGRESS_INTF_BIT|INGRESS_PORT_BIT);
					aclRule.egress_intf_idx = i;  // Set egress interface to WAN.
#ifdef CONFIG_RTL9602C_SERIES
					aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0x3);
#else
					aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0xf); //lan port
#endif

					ret = rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx);

					if(ret == 0)
						fprintf(fp, "%d\n", aclIdx);
					else
						printf("<%s>: Set rtk_rg_aclFilterAndQos_add failed! fault code = %d (out policy=%d)\n", __func__, ret, out_policy);
				}

				if(in_policy == 0)
				{
					aclRule.fwding_type_and_direction = ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET;
					aclRule.filter_fields = (INGRESS_INTF_BIT|INGRESS_PORT_BIT);
					aclRule.ingress_intf_idx = i;  // Set ingress interface to WAN.

					#if 0
					if(mib_get(MIB_WAN_PHY_PORT , (void *)&wanPhyPort) == 0){
						printf("get MIB_WAN_PHY_PORT failed!!!\n");
						wanPhyPort = RTK_RG_MAC_PORT_PON;
					}
					aclRule.ingress_port_mask.portmask = 1 << wanPhyPort; //wan port
					#endif

					aclRule.ingress_port_mask.portmask = RG_get_wan_phyPortMask();

					ret = rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx);

					if(ret == 0)
						fprintf(fp, "%d\n", aclIdx);
					else
						printf("<%s>: Set rtk_rg_aclFilterAndQos_add failed! fault code = %d (in policy=%d)\n", __func__, ret, in_policy);
				}
			}
		}
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

int AddRTK_RG_ACL_IPPort_Filter(MIB_CE_IP_PORT_FILTER_T *ipEntry)
{
	rtk_rg_aclFilterAndQos_t aclRule;
	rtk_rg_intfInfo_t infinfo;
	int aclIdx, i, udp_tcp_rule=0;
	ipaddr_t mask;
	FILE *fp;

	memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
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
		aclRule.filter_fields |= INGRESS_IPV4_TAGIF_BIT;
		aclRule.ingress_ipv4_tagif = 1;

		if(ipEntry->smaskbit == 0)
		{
			aclRule.ingress_src_ipv4_addr_start = *((in_addr_t *)ipEntry->srcIp);
			aclRule.ingress_src_ipv4_addr_end = *((in_addr_t *)ipEntry->srcIp2);
		}
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
		{
			aclRule.ingress_dest_ipv4_addr_start = *((in_addr_t *)ipEntry->dstIp);
			aclRule.ingress_dest_ipv4_addr_end = *((in_addr_t *)ipEntry->dstIp2);
		}
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
		else if(ipEntry->protoType == PROTO_UDPTCP){
			aclRule.filter_fields |= INGRESS_L4_UDP_BIT; //add udp for udp/tcp protocol
			udp_tcp_rule = 1;
		}
		else
			return -1;
	}

	if(!(fp = fopen(RG_ACL_RULES_FILE, "a")))
	{
		fprintf(stderr, "ERROR! %s\n", strerror(errno));
		return -2;
	}

add_udp_tcp:
	if(udp_tcp_rule==2){
		aclRule.filter_fields &= ~(INGRESS_L4_UDP_BIT);
		aclRule.filter_fields |= INGRESS_L4_TCP_BIT; //add tcp for udp/tcp protocol
	}
	if(ipEntry->dir == DIR_OUT)
	{
		aclRule.filter_fields |= INGRESS_DMAC_BIT;
		mib_get(MIB_ELAN_MAC_ADDR, (void *)&aclRule.ingress_dmac);
		
		aclRule.filter_fields |= INGRESS_PORT_BIT;
#ifdef CONFIG_RTL9602C_SERIES
		aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0x3);
#else		
		aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0xf);
#endif		
#ifdef WLAN_SUPPORT
		aclRule.ingress_port_mask.portmask |= (1 << RG_get_wlan_phyPortId());
#endif

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
	if(udp_tcp_rule==1){
		udp_tcp_rule = 2;
		goto add_udp_tcp;
	}

	fclose(fp);
	return 0;
}

int RTK_RG_ACL_IPPort_Filter_Default_Policy(int out_policy, int in_policy)
{
	rtk_rg_aclFilterAndQos_t aclRule;
	int aclIdx, ret;
	FILE *fp;
	int wanPhyPort;
	int lanIdx;

	memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));

	if(!(fp = fopen(RG_ACL_RULES_FILE, "a")))
	{
		fprintf(stderr, "ERROR! %s\n", strerror(errno));
		return -2;
	}

	if( out_policy == 0 )
	{
		aclRule.action_type = ACL_ACTION_TYPE_DROP;
		aclRule.filter_fields = INGRESS_DMAC_BIT|INGRESS_PORT_BIT;
		#ifdef CONFIG_RTL9602C_SERIES
		aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0x3);
		#else
		aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0xf);
		#endif
#ifdef WLAN_SUPPORT
		aclRule.ingress_port_mask.portmask |= (1 << RG_get_wlan_phyPortId());
#endif
		aclRule.acl_weight = RG_FIREWALL_ACL_WEIGHT;

		mib_get(MIB_ELAN_MAC_ADDR, (void *)&aclRule.ingress_dmac);

		if(rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx) == 0)
			fprintf(fp, "%d\n", aclIdx);
		else
			printf("rtk_rg_aclFilterAndQos_add physic port failed!\n");
	}

	if( in_policy == 0 ) //WAN -> LAN
	{

	#if 0
		rtk_rg_intfInfo_t infinfo;
		int i;

		memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));

		aclRule.action_type = ACL_ACTION_TYPE_DROP;
		aclRule.filter_fields = (INGRESS_INTF_BIT | INGRESS_PORT_BIT);

		if(mib_get(MIB_WAN_PHY_PORT , (void *)&wanPhyPort) == 0)
		{
			printf("Get MIB_WAN_PHY_PORT failed!!!\n");
			wanPhyPort = RTK_RG_MAC_PORT3; //for 0371 default
		}

		aclRule.ingress_port_mask.portmask = 1 << wanPhyPort;

		for( i = 0; i < MAX_NETIF_SW_TABLE_SIZE; i++ )
		{
			memset(&infinfo, 0, sizeof(rtk_rg_intfInfo_t));
			if(rtk_rg_intfInfo_find(&infinfo, &i))
				break;

			DBPRINT(0, "i=%d; is_wan=%d; intf_name=%s\n", i, infinfo.is_wan, infinfo.intf_name);
			if(infinfo.is_wan && infinfo.wan_intf.wan_intf_conf.wan_type != RTK_RG_BRIDGE)
			{
				aclRule.ingress_intf_idx = i;
				if(rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx) == 0)
					fprintf(fp, "%d\n", aclIdx);
				else
					printf("rtk_rg_aclFilterAndQos_add physic port failed!\n");
			}
		}

	#endif
	}

	fclose(fp);
	return 0;
}

int RTK_RG_ACL_IPPort_Filter_Allow_LAN_to_GW()
{
	rtk_rg_aclFilterAndQos_t aclRule;
	int aclIdx, ret;
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
	aclRule.filter_fields = INGRESS_DMAC_BIT | INGRESS_IPV4_DIP_RANGE_BIT | INGRESS_PORT_BIT;
	aclRule.filter_fields |= INGRESS_IPV4_TAGIF_BIT;
	aclRule.ingress_ipv4_tagif = 1;	
	#ifdef CONFIG_RTL9602C_SERIES
	aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0x3);
	#else
	aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0xf);
	#endif
#ifdef WLAN_SUPPORT
	aclRule.ingress_port_mask.portmask |= (1 << RG_get_wlan_phyPortId());
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

#if 0
int AddRTK_RG_ACL_IPv6Port_Filter(MIB_CE_V6_IP_PORT_FILTER_T *ipv6_filter_entry)
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

	// Protocol
	if( ipv6_filter_entry->protoType != PROTO_NONE )
	{
		if( ipv6_filter_entry->protoType == PROTO_TCP )
			aclRule.filter_fields |= INGRESS_L4_TCP_BIT;
		else if( ipv6_filter_entry->protoType == PROTO_UDP )
			aclRule.filter_fields |= INGRESS_L4_UDP_BIT;
		else if( ipv6_filter_entry->protoType == PROTO_ICMP)
			aclRule.filter_fields |= INGRESS_L4_ICMP_BIT;
		else
			return -1;
	}

	if(!(fp = fopen(RG_ACL_IPv6_RULES_FILE, "a")))
	{
		fprintf(stderr, "ERROR! %s\n", strerror(errno));
		return -2;
	}

	if(ipv6_filter_entry->dir == DIR_OUT)
	{
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
	}
	else if(ipv6_filter_entry->dir == DIR_IN)
	{
		int wanPhyPort;
		aclRule.filter_fields |= INGRESS_PORT_BIT;

		if(mib_get(MIB_WAN_PHY_PORT , (void *)&wanPhyPort) == 0)
		{
			printf("Get MIB_WAN_PHY_PORT failed!!!\n");
			wanPhyPort = RTK_RG_MAC_PORT3; //for 0371 default
		}

		aclRule.ingress_port_mask.portmask = 1 << wanPhyPort;

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
#endif

int AddRTK_RG_ACL_IPPort_Filter_IPv6(MIB_CE_IP_PORT_FILTER_T *ipEntry)
{
	rtk_rg_aclFilterAndQos_t aclRule;
	rtk_rg_intfInfo_t infinfo;
	int aclIdx, i, udp_tcp_rule=0;
	FILE *fp;
	unsigned char empty_ipv6[IPV6_ADDR_LEN] = {0};

	memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
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
	if(memcmp(ipEntry->sip6Start, empty_ipv6, IPV6_ADDR_LEN) != 0)
	{
		aclRule.filter_fields |= INGRESS_IPV6_SIP_RANGE_BIT;

		if(ipEntry->sip6End[0] == 0)
		{
			if(ipEntry->sip6PrefixLen == 0)
			{
				memcpy(aclRule.ingress_src_ipv6_addr_start, ipEntry->sip6Start, IPV6_ADDR_LEN);
				memcpy(aclRule.ingress_src_ipv6_addr_end, ipEntry->sip6Start, IPV6_ADDR_LEN);
			}
			else
				IPv6PrefixToStartEnd(ipEntry->sip6Start, ipEntry->sip6PrefixLen, aclRule.ingress_src_ipv6_addr_start, aclRule.ingress_src_ipv6_addr_end);
		}
		else
		{
			memcpy(aclRule.ingress_src_ipv6_addr_start, ipEntry->sip6Start, IPV6_ADDR_LEN);
			memcpy(aclRule.ingress_src_ipv6_addr_end, ipEntry->sip6End, IPV6_ADDR_LEN);
		}
	}

	// Destination ip, mask
	if(memcmp(ipEntry->dip6Start, empty_ipv6, IPV6_ADDR_LEN) != 0)
	{
		aclRule.filter_fields |= INGRESS_IPV6_DIP_RANGE_BIT;

		if(ipEntry->dip6End[0] == 0)
		{
			if(ipEntry->dip6PrefixLen == 0)
			{
				memcpy(aclRule.ingress_dest_ipv6_addr_start, ipEntry->dip6Start, IPV6_ADDR_LEN);
				memcpy(aclRule.ingress_dest_ipv6_addr_end, ipEntry->dip6Start, IPV6_ADDR_LEN);
			}
			else
				IPv6PrefixToStartEnd(ipEntry->dip6Start, ipEntry->dip6PrefixLen, aclRule.ingress_dest_ipv6_addr_start, aclRule.ingress_dest_ipv6_addr_end);
		}
		else
		{
			memcpy(aclRule.ingress_dest_ipv6_addr_start, ipEntry->dip6Start, IPV6_ADDR_LEN);
			memcpy(aclRule.ingress_dest_ipv6_addr_end, ipEntry->dip6End, IPV6_ADDR_LEN);
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
		else if( ipEntry->protoType == PROTO_UDPTCP){
			aclRule.filter_fields |= INGRESS_L4_UDP_BIT; //add udp for udp/tcp protocol
			udp_tcp_rule = 1;
		}
		else
			return -1;
	}

	if(!(fp = fopen(RG_ACL_IPv6_RULES_FILE, "a")))
	{
		fprintf(stderr, "ERROR! %s\n", strerror(errno));
		return -2;
	}

add_udp_tcp:
	if(udp_tcp_rule==2){
		aclRule.filter_fields &= ~(INGRESS_L4_UDP_BIT);
		aclRule.filter_fields |= INGRESS_L4_TCP_BIT; //add tcp for udp/tcp protocol
	}

	if(ipEntry->dir == DIR_OUT)
	{
		int wanPhyPort;
		aclRule.filter_fields |= INGRESS_PORT_BIT;

		#if 0
		if(mib_get(MIB_WAN_PHY_PORT , (void *)&wanPhyPort) == 0)
		{
			printf("Get MIB_WAN_PHY_PORT failed!!!\n");
			wanPhyPort = RTK_RG_MAC_PORT3; //for 0371 default
		}

		aclRule.ingress_port_mask.portmask = (0xf)&~(1 << wanPhyPort);
		#endif
		#ifdef CONFIG_RTL9602C_SERIES
		aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0x3) & ~(RG_get_wan_phyPortMask());
		#else
		aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0xf) & ~(RG_get_wan_phyPortMask());
		#endif
#ifdef WLAN_SUPPORT
		aclRule.ingress_port_mask.portmask |= (1 << RG_get_wlan_phyPortId());
#endif
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

	}
	else
	{
		DBPRINT(1, "Invalid filtering direction!\n");
		fclose(fp);
		return -1;
	}

	if(rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx) == 0)
		fprintf(fp, "%d\n", aclIdx);
	else
		DBPRINT(1, "rtk_rg_aclFilterAndQos_add failed!\n");

	if(udp_tcp_rule==1){
		udp_tcp_rule = 2;
		goto add_udp_tcp;
	}

	fclose(fp);
	return 0;
}

// Magician: No support default policy yet.
int RTK_RG_ACL_IPv6Port_Filter_Default_Policy(int out_policy, int in_policy)
{
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

/************************************************************************************
 * Martin zhu add for Bridge IPv4/IPv6 filter support 2015.6.29
 * Target: IPv4 only: Drop IPv6 tagif packet and IPv6 PPPoE dial packet
 *		   IPv6 only: Drop IPv4 tagif packet and IPv4 PPPoE dial packet
 *		   IPv4&IPv6: Do nothing.
 * Implement method:
 * 		   Rule1: Drop IPv4 or IPv6 tag packet by pEntry->protocol
 *		   Rule2: Trap all PPPoE(0x8864) packets which have no IPv4&IPv6 tagif to CPU
 * 	       Exp:	  Because Rule1 match before Rule2, so all PPPoE(0x8864) LCP/IPCP/IPCPv6 will
 * 				  be traped to CPU and Proc interface(/proc/rg/bridgeWan_drop_by_protocal)
 *                will drop or permit those packets.
**************************************************************************************/
int AddRTK_RG_ACL_Bridge_IPv4IPv6_Filters( MIB_CE_ATM_VC_Tp pEntry )
{
	rtk_rg_aclFilterAndQos_t aclRule;
	rtk_rg_intfInfo_t infinfo;
	int aclIdx, i;
	FILE *fp;
	char fileName[256] = {0};

	if (NULL == pEntry)
		return -1;

	if ( (pEntry->IpProtocol != 0x01) && (pEntry->IpProtocol != 0x02) )//IPv4 & IPv6,do nothing
		return 0;

	sprintf(fileName,"%s_for_%d",RG_ACL_BRIDGE_IPv4IPv6_FILTER_RULES_FILE,pEntry->ifIndex);
	if(!(fp = fopen(fileName, "a")))
	{
		fprintf(stderr, "ERROR! %s\n", strerror(errno));
		return -2;
	}

	// Rule1: Drop IPv4 or IPv6 tag packet by pEntry->IpProtocol
	memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
	aclRule.action_type = ACL_ACTION_TYPE_DROP;
	aclRule.fwding_type_and_direction = ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET;
	aclRule.filter_fields |= INGRESS_INTF_BIT;
	aclRule.ingress_intf_idx = pEntry->rg_wan_idx;
	if ( 0x01 == pEntry->IpProtocol ){//only IPv4 Pass, drop IPv6 tagif
		aclRule.filter_fields |= INGRESS_IPV6_TAGIF_BIT;
		aclRule.ingress_ipv6_tagif = 1;
	} else if ( 0x02 == pEntry->IpProtocol ){//only IPv6 Pass, drop IPv4 tagif
		aclRule.filter_fields |= INGRESS_IPV4_TAGIF_BIT;
		aclRule.ingress_ipv4_tagif = 1;
	}

	aclRule.filter_fields |= INGRESS_PORT_BIT;
	aclRule.ingress_port_mask.portmask = RG_get_wan_phyPortMask();
	if(rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx) == 0)
		fprintf(fp, "%d\n", aclIdx);
	else {
		fclose(fp);
		DBPRINT(1, "rtk_rg_aclFilterAndQos_add failed!\n");
		return -1;
	}

	// Rule2: Trap all PPPoE(0x8864) packets which have no IPv4&IPv6 tagif to CPU.
	memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
	aclRule.action_type = ACL_ACTION_TYPE_TRAP;
	aclRule.fwding_type_and_direction = ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET;
	aclRule.filter_fields |= INGRESS_INTF_BIT;
	aclRule.ingress_intf_idx = pEntry->rg_wan_idx;
	aclRule.filter_fields |= INGRESS_ETHERTYPE_BIT;
	aclRule.ingress_ethertype = 0x8864;
	//Trap all PPPoE 0x8864 packet without IPv4&IPv6 tagif to CPU
	aclRule.filter_fields |= INGRESS_IPV6_TAGIF_BIT;
	aclRule.ingress_ipv6_tagif = 0;
	aclRule.filter_fields |= INGRESS_IPV4_TAGIF_BIT;
	aclRule.ingress_ipv4_tagif = 0;

	aclRule.filter_fields |= INGRESS_PORT_BIT;
	aclRule.ingress_port_mask.portmask = RG_get_wan_phyPortMask();
	if(rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx) == 0)
		fprintf(fp, "%d\n", aclIdx);
	else {
		fclose(fp);
		DBPRINT(1, "rtk_rg_aclFilterAndQos_add failed!\n");
		return -1;
	}
	fclose(fp);

	// write proc interface(/proc/rg/bridgeWan_drop_by_protocal)
	fp = fopen("/proc/rg/bridgeWan_drop_by_protocal", "w");
	if(fp)
	{
		fprintf(fp, "%d %d\n",pEntry->rg_wan_idx, pEntry->IpProtocol);
		fclose(fp);
	}

#if 0
	//upstream:LAN->WAN
	memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
	aclRule.action_type = ACL_ACTION_TYPE_DROP;
	aclRule.fwding_type_and_direction = ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP;
#if 0
	aclRule.filter_fields |= EGRESS_INTF_BIT;
	aclRule.egress_intf_idx = pEntry->rg_wan_idx;
#endif
	aclRule.filter_fields |= INGRESS_ETHERTYPE_BIT;
	if ( 0x01 == pEntry->IpProtocol ){//only IPv4
		aclRule.ingress_ethertype = 0x86dd;
	} else if ( 0x02 == pEntry->IpProtocol ){//only IPv6
		aclRule.ingress_ethertype = 0x0800;
	}
#if 1
	aclRule.filter_fields |= INGRESS_PORT_BIT;
	if (pEntry->itfGroup != 0){
		aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(pEntry->itfGroup);
	}else {
		aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0xf);
	}
#endif
	printf("%s %d:egress_intf_idx =%d\n", __func__, __LINE__,pEntry->rg_wan_idx);
	sleep(5);
	if(rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx) == 0)
		fprintf(fp, "%d\n", aclIdx);
	else {
		fclose(fp);
		DBPRINT(1, "rtk_rg_aclFilterAndQos_add failed!\n");
		return -1;
	}

	if ( 0x02 == pEntry->IpProtocol ){//upstream:ARP
		memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
		aclRule.action_type = ACL_ACTION_TYPE_DROP;
		aclRule.fwding_type_and_direction = ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP;
#if 0
		aclRule.filter_fields |= EGRESS_INTF_BIT;
		aclRule.egress_intf_idx = pEntry->rg_wan_idx;
#endif
		aclRule.filter_fields |= INGRESS_ETHERTYPE_BIT;
		aclRule.ingress_ethertype = 0x0806;
#if 1
		aclRule.filter_fields |= INGRESS_PORT_BIT;
		if (pEntry->itfGroup != 0){
			aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(pEntry->itfGroup);
		}else {
			aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0xf);
		}
#endif
		printf("%s %d:egress_intf_idx = %d\n", __func__, __LINE__,pEntry->rg_wan_idx);
		sleep(5);
		if(rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx) == 0)
			fprintf(fp, "%d\n", aclIdx);
		else {
			fclose(fp);
			DBPRINT(1, "rtk_rg_aclFilterAndQos_add failed!\n");
			return -1;
		}
	}
#endif
	return 0;
}

int FlushRTK_RG_ACL_Bridge_IPv4IPv6_Filters( MIB_CE_ATM_VC_Tp pEntry )
{
	FILE *fp;
	int aclIdx;
	char fileName[256];

	if (NULL == pEntry)
		return -1;

	sprintf(fileName,"%s_for_%d",RG_ACL_BRIDGE_IPv4IPv6_FILTER_RULES_FILE, pEntry->ifIndex);
	if(!(fp = fopen(fileName, "r")))
	{
		fprintf(stderr, "ERROR! %s\n", strerror(errno));
		return -2;
	}

	while(fscanf(fp, "%d\n", &aclIdx) != EOF)
	{
		if(rtk_rg_aclFilterAndQos_del(aclIdx)) {
			DBPRINT(1, "rtk_rg_aclFilterAndQos_del failed! idx = %d\n", aclIdx);
			return -1;
		}
	}

	fclose(fp);
	unlink(fileName);

	fp = fopen("/proc/rg/bridgeWan_drop_by_protocal", "w");
	if(fp)
	{
		fprintf(fp, "%d %d\n",pEntry->rg_wan_idx, 0);// IPv4 && IPv6 pass
		fclose(fp);
	}

	return 0;
}
#endif

int RTK_RG_ALG_SRV_in_Lan_Set()
{
	int i, ret, vsr_num=0;
	rtk_rg_alg_type_t alg_app, alg_app_dis;
	rtk_rg_alg_serverIpMapping_t srvIpMapping;
	rtk_rg_alg_type_t server_in_lan_bit[]={RTK_RG_ALG_FTP_UDP_SRV_IN_LAN_BIT, RTK_RG_ALG_FTP_TCP_SRV_IN_LAN_BIT};
	FILE *fp;
	int enabled;
	in_addr_t ip_addr=0;
	int is_tcp;

	for(i=0; i<(sizeof(server_in_lan_bit)/sizeof(server_in_lan_bit[0])); i++)
	{
		if((ret = rtk_rg_algApps_get(&alg_app))){
			DBPRINT(1, "Error %d: rtk_rg_algApps_get failed!\n", ret);
			continue;
		}
		alg_app_dis  = (alg_app & ~server_in_lan_bit[i]);
		if((ret = rtk_rg_algApps_set(alg_app_dis))){
			DBPRINT(1, "Error %d: rtk_rg_algApps_set failed!\n", ret);
			continue;
		}
		if(alg_app & server_in_lan_bit[i]){
			if((ret=rtk_rg_algServerInLanAppsIpAddr_del(server_in_lan_bit[i]))){
				DBPRINT(1, "Error %d: rtk_rg_algServerInLanAppsIpAddr_del failed!\n", ret);
				continue;
			}
		}
		alg_app = alg_app_dis;

		memset(&srvIpMapping, 0, sizeof(rtk_rg_alg_serverIpMapping_t));
		srvIpMapping.algType = server_in_lan_bit[i];

#ifdef VIRTUAL_SERVER_SUPPORT
		//virtual server
		vsr_num = 0;
		fp = fopen(RG_VIRTUAL_SERVER_IP_FILE, "r");
		if(fp){
			while(fscanf(fp, "%lx %d\n", &ip_addr, &is_tcp)!=EOF){
				if(is_tcp == i && ip_addr){
					srvIpMapping.serverAddress=ip_addr;
					vsr_num++;
					break;
				}
			
			}
			fclose(fp);
		}
#endif
		//dmz	
		if(!vsr_num){
			fp = fopen(RG_DMZ_FILE, "r");
			if(fp){
				fscanf(fp, "%d %lx\n", &enabled, &ip_addr);
				if(enabled)
					srvIpMapping.serverAddress=ip_addr;
				fclose(fp);
			}
			else{
				enabled = 0;
				ip_addr = 0;
			}		
		}

		if(enabled || vsr_num > 0){
			if((ret=rtk_rg_algServerInLanAppsIpAddr_add(&srvIpMapping))){
				DBPRINT(1, "Error %d: rtk_rg_algServerInLanAppsIpAddr_add failed!\n", ret);
			}
			else{	
				alg_app |= server_in_lan_bit[i];		
				if((ret = rtk_rg_algApps_set(alg_app))){
					DBPRINT(1, "Error %d: rtk_rg_algApps_set failed!\n", ret);
				}
			}
		}
	}
}

int RTK_RG_DMZ_Set(int enabled, in_addr_t ip_addr)
{
	int i;
	rtk_rg_intfInfo_t infinfo;
	rtk_rg_dmzInfo_t dmz_info;
	FILE *fp;
	fp = fopen(RG_DMZ_FILE, "w");
	if(fp){
		fprintf(fp, "%d %lx\n", enabled, ip_addr);
		fclose(fp);
	}
	else
		return -2;

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
	RTK_RG_ALG_SRV_in_Lan_Set();
}

#ifdef VIRTUAL_SERVER_SUPPORT
int RTK_RG_Virtual_Server_Set(MIB_VIRTUAL_SVR_T *pf)
{
	rtk_rg_virtualServer_t vs;
	rtk_rg_intfInfo_t inf_info;
	int vs_idx, i, ret;
	FILE *fp, *fp2;

	memset(&vs, 0, sizeof(rtk_rg_virtualServer_t));

	vs.local_ip = *((ipaddr_t *)pf->serverIp);
	vs.valid = ENABLED;

	if(pf->lanPort)
	{
		vs.local_port_start = pf->lanPort;
		vs.gateway_port_start = pf->lanPort;
		vs.mappingPortRangeCnt = 1;
	}

	if(pf->wanStartPort)
	{
		if(!pf->lanPort)
			vs.local_port_start = pf->wanStartPort;

		vs.gateway_port_start = pf->wanStartPort;

		if(pf->wanEndPort)
			vs.mappingPortRangeCnt = pf->wanEndPort - pf->wanStartPort + 1;
		else
			vs.mappingPortRangeCnt = 1;
	}

	// Mapping all, if all fileds of ports are empty.
	if(!pf->lanPort && !pf->wanStartPort)
	{
		vs.local_port_start = vs.gateway_port_start = 1;
		vs.mappingPortRangeCnt = 0xffff;
	}

	if(!(fp = fopen(RG_VIRTUAL_SERVER_FILE, "a")))
		return -2;

	if(!(fp2 = fopen(RG_VIRTUAL_SERVER_IP_FILE, "a"))){
		fclose(fp);
		return -2;
	}

	for( i = 0; i < MAX_NETIF_SW_TABLE_SIZE; i++ )
	{
		memset(&inf_info, 0, sizeof(rtk_rg_intfInfo_t));
		if(rtk_rg_intfInfo_find(&inf_info, &i))
			break;

		if(inf_info.is_wan)
		{
			vs.wan_intf_idx = i;
			//vs.mappingType=VS_MAPPING_N_TO_1;

			if(pf->protoType == PROTO_TCP || pf->protoType == 0)
			{
				vs.is_tcp = 1;
				ret = rtk_rg_virtualServer_add(&vs, &vs_idx);

				DBPRINT(0, "Add virtual server. RG WAN Index=%d, protocol=%s\n", vs.wan_intf_idx, vs.is_tcp? "TCP": "UDP");
				if(ret == 0){
					fprintf(fp2, "%lx %d\n", vs.local_ip, vs.is_tcp);
					fprintf(fp, "%d\n", vs_idx);
				}
				else
				{
					DBPRINT(1, "Error %d: rtk_rg_virtualServer_add failed! protoType=TCP\n", ret);
					continue;
				}
			}

			if(pf->protoType == PROTO_UDP || pf->protoType == 0)
			{
				vs.is_tcp = 0;
				ret = rtk_rg_virtualServer_add(&vs, &vs_idx);

				DBPRINT(0, "Add virtual server. RG WAN Index=%d, protocol=%s\n", vs.wan_intf_idx, vs.is_tcp? "TCP": "UDP");

				if(ret == 0){
					fprintf(fp2, "%lx %d\n", vs.local_ip, vs.is_tcp);
					fprintf(fp, "%d\n", vs_idx);
				}
				else
				{
					DBPRINT(1, "Error %d: rtk_rg_virtualServer_add failed! protoType=UDP\n", ret);
					continue;
				}
			}
		}
	}

	fclose(fp2);
	fclose(fp);
	RTK_RG_ALG_SRV_in_Lan_Set();
	return 0;
}

int FlushRTK_RG_Virtual_Server()
{
	FILE *fp;
	int vsIdx;

	if(!(fp = fopen(RG_VIRTUAL_SERVER_FILE, "r")))
		return -1;

	while(fscanf(fp, "%d\n", &vsIdx) != EOF)
	{
		if(rtk_rg_virtualServer_del(vsIdx))
			printf("rtk_rg_virtualServer_del failed! idx = %d\n", vsIdx);
		else
			printf("Deleted Virtual Server %d.\n", vsIdx);
	}
	unlink(RG_VIRTUAL_SERVER_IP_FILE);
	unlink(RG_VIRTUAL_SERVER_FILE);

	RTK_RG_ALG_SRV_in_Lan_Set();

	return 0;
}
#endif


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

int RTK_RG_QoS_Car_Rule_Set(MIB_CE_IP_TC_Tp qos_entry)
{
	rtk_rg_aclFilterAndQos_t aclRule;
	MIB_CE_ATM_VC_T vc_entry;
	int aclIdx, ret, i, total_vc;
	FILE *fp;
	ipaddr_t mask;
	unsigned char empty_ipv6[IPV6_ADDR_LEN] = {0};

	memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));

	aclRule.fwding_type_and_direction = ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET;
	aclRule.action_type = ACL_ACTION_TYPE_QOS;

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
		close(fd);
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
	close(fd);
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
	close(fd);
}
#endif

int RTK_RG_QoS_Queue_Set()
{
	unsigned char policy;
	int aclIdx, i, ret;
	rtk_rg_qos_queue_weights_t q_weight;
#if defined(CONFIG_GPON_FEATURE)
	unsigned int pon_mode;
	int wanPhyPort, lanPhyPort;
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


#if defined(CONFIG_GPON_FEATURE)
	if (mib_get(MIB_PON_MODE, (void *)&pon_mode) == 0)
		printf("get MIB_PON_MODE failed!!!\n");
#if 0
	if(mib_get(MIB_WAN_PHY_PORT , (void *)&wanPhyPort) == 0)
		printf("get MIB_WAN_PHY_PORT failed!!!\n");
#endif
	if((wanPhyPort = RG_get_wan_phyPortId()) == -1)
		printf("get wan phy port id failed!!!\n");
#endif

	for( i = 0; i < 6; i++ )
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
		/*恢复为默认的绝对优先级*/
		if((ret = rtk_rg_qosStrictPriorityOrWeightFairQueue_set(i, q_weight)) != 0)
			DBPRINT(1, "rtk_qos_schedulingQueue_set failed! (ret=%d, i=%d)\n", ret, i);
	}

#endif

	return 0;
}

int do_special_handle_RTP(int prior)
{
	FILE *fp=NULL;
	int aclIdx,ret;
	rtk_rg_alg_type_t alg_app;


	rtk_rg_naptFilterAndQos_t naptFilter;

	//enable SIP alg
	rtk_rg_algApps_get(&alg_app);
	alg_app|= (RTK_RG_ALG_SIP_TCP_BIT|RTK_RG_ALG_SIP_UDP_BIT);
	printf("ALG APP is 0x%x\n",alg_app);
	rtk_rg_algApps_set(alg_app);

	memset(&naptFilter, 0, sizeof(rtk_rg_naptFilterAndQos_t));

	naptFilter.ingress_dest_l4_port = 5060;
	naptFilter.assign_priority = 8-prior;
	naptFilter.action_fields |= ASSIGN_NAPT_PRIORITY_BIT;
	naptFilter.filter_fields |= INGRESS_DPORT;

	if(!(fp = fopen(RG_QOS_RTP_RULES_FILE, "a")))
	{
		fprintf(stderr, "ERROR! %s\n", strerror(errno));
		return -2;
	}

	if((ret = rtk_rg_naptFilterAndQos_add(&aclIdx, &naptFilter)) == 0)
		fprintf(fp, "%d\n", aclIdx);
	else
		printf("rtk_rg_apollo_naptFilterAndQos_add  QoS rule failed! (ret = %d)\n", ret);

	fclose(fp);
	return 0;
}


int clean_special_handle_RTP()
{
	FILE *fp=NULL;
	int qos_idx;

	if(!(fp = fopen(RG_QOS_RTP_RULES_FILE, "r")))
		return -2;

	while(fscanf(fp, "%d\n", &qos_idx) != EOF)
	{
		if(rtk_rg_naptFilterAndQos_del(qos_idx))
			DBPRINT(1, "rtk_rg_apollo_naptFilterAndQos_del failed! idx = %d\n", qos_idx);
	}

	fclose(fp);
	unlink(RG_QOS_RTP_RULES_FILE);
	return 0;
}


int RTK_RG_PPPoEProxy_ACL_Rule_Set()
{
	FILE *fp;
	rtk_rg_aclFilterAndQos_t aclRule;
	int aclIdx, ret;

	memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));

	aclRule.fwding_type_and_direction =  ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET;
	aclRule.action_type = ACL_ACTION_TYPE_TRAP_TO_PS;
	aclRule.acl_weight = RG_DEFAULT_ACL_WEIGHT;
	aclRule.filter_fields |= INGRESS_PORT_BIT;
#ifdef CONFIG_RTL9602C_SERIES
	aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0x3);
#else		
	aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0xf);
#endif

	aclRule.filter_fields |= INGRESS_ETHERTYPE_BIT;
	aclRule.ingress_ethertype =  0x8864;

	if(!(fp = fopen(RG_PPPOEPROXY_RULES_FILE, "w")))
	{
		fprintf(stderr, "ERROR! %s\n", strerror(errno));
		return -2;
	}

	if((ret = rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx)) == 0)
		fprintf(fp, "%d\n", aclIdx);
	else
		printf("rtk_rg_aclFilterAndQos_add PPPoEProxy rule failed! (ret = %d)\n", ret);

	fclose(fp);
	return 0;
}

int RTK_RG_PPPoEProxy_ACL_Rule_Flush()
{
	FILE *fp;
	int acl_idx;

	if(!(fp = fopen(RG_PPPOEPROXY_RULES_FILE, "r")))
		return -2;

	while(fscanf(fp, "%d\n", &acl_idx) != EOF)
	{
		if(rtk_rg_aclFilterAndQos_del(acl_idx))
			DBPRINT(1, "rtk_rg_aclFilterAndQos_del failed! idx = %d\n", acl_idx);
	}

	fclose(fp);
	unlink(RG_PPPOEPROXY_RULES_FILE);
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
	unsigned char enableDscpMark=1;
	unsigned char enableQos1p=2;  // enableQos1p=0: not use, enableQos1p=1: use old value, enableQos1p=2: mark new value

	memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));

	aclRule.fwding_type_and_direction =  ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET;
	aclRule.action_type = ACL_ACTION_TYPE_QOS;
	aclRule.acl_weight = RG_QOS_ACL_WEIGHT;

	mib_get(MIB_QOS_ENABLE_DSCP_MARK, (void *)&enableDscpMark);
	mib_get(MIB_QOS_ENABLE_1P, (void *)&enableQos1p);

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
	aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0x3) | (1<<RTK_RG_PORT_CPU);
	#else		
	aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0xf) | (1<<RTK_RG_PORT_CPU);
	#endif	

	// Filter rule of physic ports.
	if(qos_entry->phyPort >= 1 && qos_entry->phyPort <= SW_LAN_PORT_NUM)
	{
		aclRule.filter_fields |= INGRESS_PORT_BIT;
		//aclRule.ingress_port_mask.portmask = (1 << RTK_RG_PORT0)  | (1 << RTK_RG_PORT1) | (1 << RTK_RG_PORT2) | (1 << RTK_RG_PORT3);
		aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(1 << (qos_entry->phyPort-1));
	}

	// Filter rule of DSCP
#ifdef QOS_DIFFSERV
	if(qos_entry->qosDscp != 0)
	{
		aclRule.filter_fields |= INGRESS_DSCP_BIT;
		aclRule.ingress_dscp = qos_entry->qosDscp >> 2;
	}
#endif

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
		else if(qos_entry->protoType == PROTO_RTP){
			do_special_handle_RTP(qos_entry->prior);
			return 0;
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
				aclRule.filter_fields &= ~(INGRESS_PORT_BIT|INGRESS_IPV4_DIP_RANGE_BIT|INGRESS_CTAG_VID_BIT|INGRESS_IPV4_DIP_RANGE_BIT|INGRESS_DMAC_BIT);		// Current RG design, using egress pattern, should not have ingress pattern

				//IF QoS has egress pattern, let QoS be lower then normal QoS.
				//or it will be sorted to higher prioirty then other QoS rules.
				aclRule.acl_weight = RG_QOS_LOW_ACL_WEIGHT;
			}
		}
	}

	// Action rule of DSCP
	if( enableDscpMark && (qos_entry->m_dscp != 0))
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

	// 1p remarking.
	if( (enableQos1p==2) && (qos_entry->m_1p != 0))
	{
		aclRule.qos_actions |= ACL_ACTION_1P_REMARKING_BIT;
		aclRule.action_dot1p_remarking_pri = qos_entry->m_1p - 1;
	}
	else{
		//Do nothing for 1p remarking
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

int FLUSH_RTK_RG_UPnP_Entry(void)
{
	FILE *fp, *fp_tmp;
	int upnp_idx;
	char line[64];
	if(!(fp = fopen(RG_UPNP_CONNECTION_FILE, "r")))
		return -2;

	while(fgets(line, 64, fp) != NULL)
	{
		sscanf(line, "%d\n", &upnp_idx);
			if(rtk_rg_upnpConnection_del(upnp_idx))
				DBPRINT(1, "rtk_rg_upnpConnection_del failed! idx = %d\n", upnp_idx);
	}
	unlink(RG_UPNP_CONNECTION_FILE);
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
int RTK_RG_FLUSH_ALG_FILTER_RULE()
{

	FILE *fp;
	int filter_idx;

	if(!(fp = fopen(RG_ALG_FILTER_FILE, "r")))
		return -2;

	while(fscanf(fp, "%d\n", &filter_idx) != EOF)
	{
		if(rtk_rg_naptFilterAndQos_del(filter_idx))
			DBPRINT(1, "rtk_rg_naptFilterAndQos_del failed! idx = %d\n", filter_idx);
	}

	fclose(fp);
	unlink(RG_ALG_FILTER_FILE);

	return 0;


}
int RTK_RG_ALG_Set()
{
	rtk_rg_alg_type_t alg_app = 0;
	rtk_rg_naptFilterAndQos_t napt_filter;
	int ret=0;
	int filterIdx=0;
	FILE *fp;
	unsigned char value = 0;
	
	if((ret = rtk_rg_algApps_get(&alg_app))){
		DBPRINT(1, "Error %d: rtk_rg_algApps_get failed!\n", ret);
		return -1;
	}

	if(!(fp = fopen(RG_ALG_FILTER_FILE, "w")))
		return -2;


#ifdef CONFIG_NF_CONNTRACK_FTP
	if(mib_get(MIB_IP_ALG_FTP, &value) && value == 1)
		alg_app |= RTK_RG_ALG_FTP_TCP_BIT | RTK_RG_ALG_FTP_UDP_BIT;
	else{
		alg_app &= ~(RTK_RG_ALG_FTP_TCP_BIT | RTK_RG_ALG_FTP_UDP_BIT);

		//disable alg for ftp
			memset(&napt_filter, 0, sizeof(rtk_rg_naptFilterAndQos_t));
			napt_filter.direction = RTK_RG_NAPT_FILTER_OUTBOUND;
			napt_filter.filter_fields = (L4_PROTOCAL | INGRESS_DPORT);
			napt_filter.ingress_dest_l4_port = 21; //pptp wellknown port
			napt_filter.ingress_l4_protocal = 0x6; //tcp protocol
			napt_filter.action_fields = NAPT_DROP_BIT;
			napt_filter.ruleType = RTK_RG_NAPT_FILTER_PERSIST;
			if((ret = rtk_rg_naptFilterAndQos_add( &filterIdx, &napt_filter)) == 0)
				fprintf(fp, "%d\n", filterIdx);
			else
				printf("rtk_rg_naptFilterAndQos_add QoS rule failed! (ret = %d)\n", ret);

	}
#endif
#ifdef CONFIG_NF_CONNTRACK_H323
	if(mib_get(MIB_IP_ALG_H323, &value) && value == 1)
		alg_app |= RTK_RG_ALG_H323_TCP_BIT | RTK_RG_ALG_H323_UDP_BIT;
	else{
		alg_app &= ~(RTK_RG_ALG_H323_TCP_BIT | RTK_RG_ALG_H323_UDP_BIT);
	}
#endif
#ifdef CONFIG_NF_CONNTRACK_RTSP
	if(mib_get(MIB_IP_ALG_RTSP, &value) && value == 1)
		alg_app |= RTK_RG_ALG_RTSP_TCP_BIT | RTK_RG_ALG_RTSP_UDP_BIT;
	else{
		alg_app &= ~(RTK_RG_ALG_RTSP_TCP_BIT | RTK_RG_ALG_RTSP_UDP_BIT);
	}
#endif
#ifdef CONFIG_NF_CONNTRACK_L2TP
	if(mib_get(MIB_IP_ALG_L2TP, &value) && value == 1)
		alg_app |= RTK_RG_ALG_L2TP_TCP_PASSTHROUGH_BIT | RTK_RG_ALG_L2TP_UDP_PASSTHROUGH_BIT;
	else{
		alg_app &= ~(RTK_RG_ALG_L2TP_TCP_PASSTHROUGH_BIT | RTK_RG_ALG_L2TP_UDP_PASSTHROUGH_BIT);
		
		//disable alg for l2tp
		memset(&napt_filter, 0, sizeof(rtk_rg_naptFilterAndQos_t));
		napt_filter.direction = RTK_RG_NAPT_FILTER_OUTBOUND;
		napt_filter.filter_fields = (L4_PROTOCAL | INGRESS_DPORT);
		napt_filter.ingress_dest_l4_port = 1701; //l2tp wellknown port
		napt_filter.ingress_l4_protocal = 0x11; //udp protocol
		napt_filter.action_fields = NAPT_DROP_BIT;
		napt_filter.ruleType = RTK_RG_NAPT_FILTER_PERSIST;
		if((ret = rtk_rg_naptFilterAndQos_add( &filterIdx, &napt_filter)) == 0)
			fprintf(fp, "%d\n", filterIdx);
		else
			printf("rtk_rg_naptFilterAndQos_add QoS rule failed! (ret = %d)\n", ret);

	}
#endif
#ifdef CONFIG_NF_CONNTRACK_IPSEC
	if(mib_get(MIB_IP_ALG_IPSEC, &value) && value == 1)
		alg_app |= RTK_RG_ALG_IPSEC_TCP_PASSTHROUGH_BIT | RTK_RG_ALG_IPSEC_UDP_PASSTHROUGH_BIT;
	else{
		alg_app &= ~(RTK_RG_ALG_IPSEC_TCP_PASSTHROUGH_BIT | RTK_RG_ALG_IPSEC_UDP_PASSTHROUGH_BIT);
		
		//disable alg for ipsec
		memset(&napt_filter, 0, sizeof(rtk_rg_naptFilterAndQos_t));
		napt_filter.direction = RTK_RG_NAPT_FILTER_OUTBOUND;
		napt_filter.filter_fields = (L4_PROTOCAL | INGRESS_DPORT);
		napt_filter.ingress_dest_l4_port = 500; //ipsec wellknown port
		napt_filter.ingress_l4_protocal = 0x11; //udp protocol
		napt_filter.action_fields = NAPT_DROP_BIT;
		napt_filter.ruleType = RTK_RG_NAPT_FILTER_PERSIST;
		if((ret = rtk_rg_naptFilterAndQos_add( &filterIdx, &napt_filter)) == 0)
			fprintf(fp, "%d\n", filterIdx);
		else
			printf("rtk_rg_naptFilterAndQos_add QoS rule failed! (ret = %d)\n", ret);

		//disable alg for ipsec
		memset(&napt_filter, 0, sizeof(rtk_rg_naptFilterAndQos_t));
		napt_filter.direction = RTK_RG_NAPT_FILTER_OUTBOUND;
		napt_filter.filter_fields = (L4_PROTOCAL | INGRESS_DPORT);
		napt_filter.ingress_dest_l4_port = 4500; //ipsec wellknown port
		napt_filter.ingress_l4_protocal = 0x11; //udp protocol
		napt_filter.action_fields = NAPT_DROP_BIT;
		napt_filter.ruleType = RTK_RG_NAPT_FILTER_PERSIST;
		if((ret = rtk_rg_naptFilterAndQos_add( &filterIdx, &napt_filter)) == 0)
			fprintf(fp, "%d\n", filterIdx);
		else
			printf("rtk_rg_naptFilterAndQos_add QoS rule failed! (ret = %d)\n", ret);

	}
#endif
#ifdef CONFIG_NF_CONNTRACK_SIP
	if(mib_get(MIB_IP_ALG_SIP, &value) && value == 1){
		alg_app |= RTK_RG_ALG_SIP_TCP_BIT | RTK_RG_ALG_SIP_UDP_BIT;
		/*Due to napt entry will be added into HW, so traffic won't handle by Romedriver
		    Alg passthrough will fail., so we trap sip packet to forwarding engine, let RG have
		    chance to handle.
		    Acl Trap will have another problem, would hit RG l3 short cut.
		    so......just use napt find entry. which dport == 5060 to delete it.!!
		*/
		//loop MAX_NAPT_OUT_HW_TABLE_SIZE
		int i, ret;
		rtk_rg_naptInfo_t naptInfo;
		for(i=0;i<MAX_NAPT_OUT_HW_TABLE_SIZE;i++){
			ret = rtk_rg_naptConnection_find(&naptInfo,&i);
			if(ret==RT_ERR_RG_OK){
				/*
				Acl Trap will have another problem, would hit RG l3 shortcut.
				so......just use napt find entry. which dport == 5060 to delete it.!!
				*/
				if(naptInfo.naptTuples.remote_port == 5060){
					ret = rtk_rg_naptConnection_del(i);
					if(ret != 0)
						fprintf(stderr,"%s-%d error ret=%d del entry:%d fail!",__func__,__LINE__,ret,i);
				}
			}
		}
	}
	else{
		alg_app &= ~(RTK_RG_ALG_SIP_TCP_BIT | RTK_RG_ALG_SIP_UDP_BIT);

	}
#endif
#ifdef CONFIG_NF_CONNTRACK_PPTP
	if(mib_get(MIB_IP_ALG_PPTP, &value) && value == 1)
		alg_app |= RTK_RG_ALG_PPTP_TCP_PASSTHROUGH_BIT | RTK_RG_ALG_PPTP_UDP_PASSTHROUGH_BIT;
	else{
		alg_app &= ~(RTK_RG_ALG_PPTP_TCP_PASSTHROUGH_BIT | RTK_RG_ALG_PPTP_UDP_PASSTHROUGH_BIT);

		//disable alg for pptp
			memset(&napt_filter, 0, sizeof(rtk_rg_naptFilterAndQos_t));
			napt_filter.direction = RTK_RG_NAPT_FILTER_OUTBOUND;
			napt_filter.filter_fields = (L4_PROTOCAL | INGRESS_DPORT);
			napt_filter.ingress_dest_l4_port = 1723; //pptp wellknown port
			napt_filter.ingress_l4_protocal = 0x6; //tcp protocol
			napt_filter.action_fields = NAPT_DROP_BIT;
			napt_filter.ruleType = RTK_RG_NAPT_FILTER_PERSIST;
			if((ret = rtk_rg_naptFilterAndQos_add( &filterIdx, &napt_filter)) == 0)
				fprintf(fp, "%d\n", filterIdx);
			else
				printf("rtk_rg_naptFilterAndQos_add QoS rule failed! (ret = %d)\n", ret);

	}
#endif
	if(rtk_rg_algApps_set(alg_app))
	{
		fclose(fp);
		DBPRINT(1, "rtk_rg_algApps_set failed! alg_app = %X\n", alg_app);
		return -1;
	}
	
	fclose(fp);
	return 0;
}
#endif

#ifdef CONFIG_CTC_E8_CLIENT_LIMIT
enum DevType
{
	//RG set category 0 as default category, all traffic will learn mac at category 0.
	//then compare with other category. So, we shift all category by 1.
	CTC_RG_Default=0,
	CTC_Computer_,
	CTC_Camera_,
	CTC_HGW_,
	CTC_STB_,
	CTC_PHONE_,
	CTC_UNKNOWN_=100
};
int RTK_RG_AccessWanLimitCategory_Set(unsigned char *mac, int category)
{
	/*
		typedef struct rtk_rg_accessWanLimitCategory_s
		{
			unsigned char category;
			rtk_mac_t mac;
		}rtk_rg_accessWanLimitCategory_t;
	*/
	rtk_rg_accessWanLimitCategory_t macCategory_info;
	int ret=0;
	unsigned int vInt;
	mib_get( CWMP_CT_MWBAND_MODE, (void *)&vInt);
	if(vInt != 2) //mode 2 is equal to RG struct.h's RG_ACCESSWAN_TYPE_CATEGORY
		return 0;
	macCategory_info.category = category;
	memcpy(&macCategory_info.mac,mac,MAC_ADDR_LEN);
	ret = rtk_rg_accessWanLimitCategory_set(macCategory_info);
	if(ret){
			fprintf(stderr, "%s-%d error ret=%d\n",__func__,__LINE__,ret);
			return -1;
	}
	return ret;
}

int RTK_RG_AccessWanLimit_Set(void)
{
	/*
		rtk_rg_accessWanLimitType_t type;
		union
		{
			unsigned char category;
			rtk_rg_portmask_t port_mask;
		}data;
		int learningLimitNumber;
		int learningCount;
		rtk_rg_sa_learning_exceed_action_t action;
#ifdef CONFIG_MASTER_WLAN0_ENABLE
		unsigned int wlan0_dev_mask;	//used for WLAN0 device access limit
#endif
	*/

	rtk_rg_accessWanLimitData_t access_wan_info;
	int ret=0;
	unsigned int vInt;
	unsigned char Value;
	memset(&access_wan_info,0,sizeof(rtk_rg_accessWanLimitData_t));
	mib_get( CWMP_CT_MWBAND_MODE, (void *)&vInt);
	switch(vInt)
	{
		case 0:
			access_wan_info.type = RG_ACCESSWAN_TYPE_UNLIMIT;
			access_wan_info.action = SA_LEARN_EXCEED_ACTION_PERMIT;
			ret = rtk_rg_accessWanLimit_set(access_wan_info);
			if(ret){
				fprintf(stderr, "%s-%d error ret=%d\n",__func__,__LINE__,ret);
				return -1;
			}
			break;
		case 1://all port
			access_wan_info.type = RG_ACCESSWAN_TYPE_PORTMASK;
#ifdef CONFIG_RTL9602C_SERIES
			access_wan_info.data.port_mask.portmask = RG_get_lan_phyPortMask(0x3);
#else			
			access_wan_info.data.port_mask.portmask = RG_get_lan_phyPortMask(0xf);//all lan
#endif			
#ifdef CONFIG_MASTER_WLAN0_ENABLE
			access_wan_info.wlan0_dev_mask = 0x1f;//all mast wifi port
#endif
			mib_get( CWMP_CT_MWBAND_NUMBER, (void *)&vInt);
			access_wan_info.learningLimitNumber = vInt;
			access_wan_info.action = SA_LEARN_EXCEED_ACTION_PERMIT_L2;
			ret = rtk_rg_accessWanLimit_set(access_wan_info);
			if(ret){
				fprintf(stderr, "%s-%d error ret=%d\n",__func__,__LINE__,ret);
				return -1;
			}
			break;
		case 2:
			access_wan_info.type = RG_ACCESSWAN_TYPE_CATEGORY;
			access_wan_info.action = SA_LEARN_EXCEED_ACTION_PERMIT_L2;
			break;
	}

	if(access_wan_info.type == RG_ACCESSWAN_TYPE_CATEGORY)
	{
		mib_get( CWMP_CT_MWBAND_PC_ENABLE, (void *)&Value);
		if(Value){
			access_wan_info.data.category = CTC_Computer_;
			mib_get( CWMP_CT_MWBAND_PC_NUM, (void *)&vInt);
			access_wan_info.learningLimitNumber = vInt;
			ret = rtk_rg_accessWanLimit_set(access_wan_info);
			if(ret){
				fprintf(stderr, "%s-%d error ret=%d\n",__func__,__LINE__,ret);
				return -1;
			}
		}

		mib_get( CWMP_CT_MWBAND_CMR_ENABLE, (void *)&Value);
		if(Value){
			access_wan_info.data.category = CTC_Camera_;
			mib_get( CWMP_CT_MWBAND_CMR_NUM, (void *)&vInt);
			access_wan_info.learningLimitNumber = vInt;
			ret = rtk_rg_accessWanLimit_set(access_wan_info);
			if(ret){
				fprintf(stderr, "%s-%d error ret=%d\n",__func__,__LINE__,ret);
				return -1;
			}
		}

		mib_get( CWMP_CT_MWBAND_STB_ENABLE, (void *)&Value);
		if(Value){
			access_wan_info.data.category = CTC_STB_;
			mib_get( CWMP_CT_MWBAND_STB_NUM, (void *)&vInt);
			access_wan_info.learningLimitNumber = vInt;
			ret = rtk_rg_accessWanLimit_set(access_wan_info);
			if(ret){
				fprintf(stderr, "%s-%d error ret=%d\n",__func__,__LINE__,ret);
				return -1;
			}
		}
		mib_get( CWMP_CT_MWBAND_PHN_ENABLE, (void *)&Value);

		if(Value){
			access_wan_info.data.category = CTC_PHONE_;
			mib_get( CWMP_CT_MWBAND_PHN_NUM, (void *)&vInt);
			access_wan_info.learningLimitNumber = vInt;
			ret = rtk_rg_accessWanLimit_set(access_wan_info);
			if(ret){
				fprintf(stderr, "%s-%d error ret=%d\n",__func__,__LINE__,ret);
				return -1;
			}
		}

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

	for (i = 0; i < total_url; i++)
	{
		if (!mib_chain_get(MIB_URL_FQDN_TBL, i, (void *)&fqdn))
			continue;

		memset(&url_f_s,0,sizeof(rtk_rg_urlFilterString_t));

		url_f_s.path_exactly_match = 0;

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

		memset(&url_f_s,0,sizeof(rtk_rg_urlFilterString_t));

		url_f_s.path_exactly_match = 0;

		strncpy(url_f_s.url_filter_string, keyword.keyword, MAX_KEYWD_LENGTH);

		if((ret = rtk_rg_urlFilterString_add(&url_f_s, &url_idx)) == 0)
			fprintf(fp, "%d\n", url_idx);
		else
			DBPRINT(1, "rtk_rg_urlFilterString_add QoS rule failed!\n");
	}

	fclose(fp);
	return 0;
}

int RTK_RG_URL_Filter_Set_By_Key(int mode)
{
	int url_idx, ret, total_url, i;
	rtk_rg_urlFilterString_t url_f_s;
	MIB_CE_URL_FQDN_T fqdn;
	FILE *fp;

	if(mode==1) //blacklist
		system("echo 0 > /proc/rg/urlFilter_mode");
	else if(mode==2) //whitelist
		system("echo 1 > /proc/rg/urlFilter_mode");
	else { //disable
		system("echo 0 > /proc/rg/urlFilter_mode");
		return 0;
	}

	total_url = mib_chain_total(MIB_URL_FQDN_TBL);

	if(!(fp = fopen(RG_URL_FILTER_FILE, "a")))
	{
		fprintf(stderr, "ERROR! %s\n", strerror(errno));
		return -2;
	}

	for (i = 0; i < total_url; i++)
	{
		if (!mib_chain_get(MIB_URL_FQDN_TBL, i, (void *)&fqdn))
			continue;

		memset(&url_f_s,0,sizeof(rtk_rg_urlFilterString_t));

		url_f_s.path_exactly_match = 0;

		strncpy(url_f_s.url_filter_string, fqdn.key, MAX_URL_LENGTH);

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

	rtk_portmask_t portmask,extportmask;
	if(enable == 1)
		portmask.bits[0] = 0x70;
	else
		portmask.bits[0] = 0x7f; // disalbe

	extportmask.bits[0] = 0x1f;

	//for master
	rtk_rg_port_isolationEntryExt_set(RTK_PORT_ISO_CFG_0,1,&portmask,&extportmask);
	rtk_rg_port_isolationEntryExt_set(RTK_PORT_ISO_CFG_1,1,&portmask,&extportmask);

	//for slave
	rtk_rg_port_isolationEntryExt_set(RTK_PORT_ISO_CFG_0,2,&portmask,&extportmask);
	rtk_rg_port_isolationEntryExt_set(RTK_PORT_ISO_CFG_1,2,&portmask,&extportmask);

	return 0;
}

int RTK_RG_FLUSH_DOS_FILTER_RULE()
{

	FILE *fp;
	int filter_idx;

	if(!(fp = fopen(RG_DOS_FILTER_FILE, "r")))
		return -2;

	while(fscanf(fp, "%d\n", &filter_idx) != EOF)
	{
		if(rtk_rg_aclFilterAndQos_del(filter_idx))
			DBPRINT(1, "rtk_rg_aclFilterAndQos_del failed! idx = %d\n", filter_idx);
	}

	fclose(fp);
	unlink(RG_DOS_FILTER_FILE);
	return 0;


}

int RTK_RG_DoS_Set(int enable)
{
	rtk_rg_mac_portmask_t dos_port_mask;
	int wanPhyPort;

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
	//for blocking udp flood
	//AUG_PRT("%s-%d enable=0x%x\n",__func__,__LINE__,enable);
	if(enable & UDPBombEnabled){
		
		int ret=0, acl_index=0;
		FILE *fp;
		rtk_rg_aclFilterAndQos_t aclRule;
		if(!(fp = fopen(RG_DOS_FILTER_FILE, "w")))
		{
			fprintf(stderr, "ERROR! %s\n", strerror(errno));
			return -2;
		}

		//rg add shareMeter index 1 rate 800 ifgInclude 1

		//rg set acl-filter fwding_type_and_direction 0
		//rg set acl-filter pattern ingress_port_mask 0x10
		//rg set acl-filter pattern ingress_l4_protocal 0
		//rg set acl-filter action action_type 3
		//rg set acl-filter action qos action_share_meter 1
		//rg add acl-filter entry	
		ret = rtk_rg_shareMeter_set(28,800,RTK_RG_ENABLED);
		memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
		aclRule.fwding_type_and_direction = ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET;
		aclRule.action_type = ACL_ACTION_TYPE_QOS;
		aclRule.acl_weight = RG_DEFAULT_ACL_WEIGHT; 
		aclRule.filter_fields |= INGRESS_PORT_BIT;
		aclRule.ingress_port_mask.portmask =  RG_get_wan_phyPortMask();
		aclRule.filter_fields |= INGRESS_L4_UDP_BIT;
		aclRule.qos_actions = ACL_ACTION_SHARE_METER_BIT;
		aclRule.action_share_meter = 28;
		if((ret = rtk_rg_aclFilterAndQos_add(&aclRule,&acl_index)) == 0)
			fprintf(fp, "%d\n", acl_index);
		else
			printf("rtk_rg_aclFilterAndQos_add QoS rule failed! (ret = %d)\n", ret);

		fclose(fp);

	}
	if(rtk_rg_dosFloodType_set(RTK_RG_DOS_SYNFLOOD_DENY, (enable & SYSFLOODSYN)? 1:0,RTK_RG_DOS_ACTION_DROP,1)) //threshold 1K/sec
		DBPRINT(1, "rtk_rg_dosFloodType_set failed! type = RTK_RG_DOS_SYNFLOOD_DENY\n");

	if(rtk_rg_dosFloodType_set(RTK_RG_DOS_FINFLOOD_DENY, (enable & SYSFLOODFIN)? 1:0,RTK_RG_DOS_ACTION_DROP,1)) //threshold 1K/sec
		DBPRINT(1, "rtk_rg_dosFloodType_set failed! type = RTK_RG_DOS_FINFLOOD_DENY\n");

	if(rtk_rg_dosFloodType_set(RTK_RG_DOS_ICMPFLOOD_DENY,(enable & SYSFLOODICMP)? 1:0,RTK_RG_DOS_ACTION_DROP,1)) //threshold 1K/sec
		DBPRINT(1, "rtk_rg_dosFloodType_set failed! type = RTK_RG_DOS_ICMPFLOOD_DENY\n");

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

unsigned int RG_get_portCounter(unsigned int portIndex, unsigned long long *tx_bytes,unsigned long *tx_pkts,unsigned long *tx_drops,unsigned long *tx_errs,
										unsigned long long *rx_bytes, unsigned long *rx_pkts,unsigned long *rx_drops,unsigned long *rx_errs)
{
	rtk_rg_port_mib_info_t portmib;
	int ret;

	ret = rtk_rg_portMibInfo_get(RG_get_lan_phyPortId(portIndex),&portmib);
	if(ret != 0)
	{
		DBPRINT(1, "%s get port %d mib info failed!\n", __FUNCTION__, portIndex);
		return 0;
	}

	*rx_bytes = portmib.ifInOctets;
	*rx_pkts = (portmib.ifInUcastPkts + portmib.ifInMulticastPkts + portmib.ifInBroadcastPkts);
	*rx_drops = portmib.dot1dTpPortInDiscards;
	*rx_errs = (portmib.dot3StatsSymbolErrors + portmib.dot3ControlInUnknownOpcodes);
	*tx_bytes = portmib.ifOutOctets;
	*tx_pkts = (portmib.ifOutUcastPkts + portmib.ifOutMulticastPkts + portmib.ifOutBrocastPkts);
	*tx_drops = portmib.ifOutDiscards ;
	*tx_errs = 0;
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

	//Add TR-069 http server
	mib_get(CWMP_CONREQ_PORT, &port);
	serviceEntry.valid = 1;
	serviceEntry.port_num = port;
	serviceEntry.type = GATEWAY_SERVER_SERVICE;
	if((ret = rtk_rg_gatewayServicePortRegister_add(&serviceEntry, &index)) == 0)
			fprintf(fp, "%d\n", index);
	else
		DBPRINT(1, "%s: add cwmp port via rtk_rg_gatewayServicePortRegister_add failed! ret = %d!\n", __FUNCTION__, ret);
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

void RG_tcp_stateful_tracking(int enable)
{
	if(enable)
		system("echo 0 > /proc/rg/tcp_disable_stateful_tracking");
	else
		system("echo 1 > /proc/rg/tcp_disable_stateful_tracking");
}


#ifdef SUPPORT_WAN_BANDWIDTH_INFO
int RG_get_interface_counter(int rg_wan_idx, unsigned long long * uploadcnt, unsigned long long * downloadcnt)
{
	int ret;
	rtk_l34_mib_t mib;

	mib.ifIndex = rg_wan_idx;
    ret = rtk_rg_interfaceMibCounter_get(&mib);
	if(ret != RT_ERR_OK)
	{
		DBPRINT(1, "%s rtk_rg_interfaceMibCounter_get idx[%d] failed!\n", __FUNCTION__, rg_wan_idx);
		return -1;
	}

	*uploadcnt = mib.ifOutOctets;
	*downloadcnt = mib.ifInOctets;

	return 0;
}
#endif

#ifdef SUPPORT_WEB_REDIRECT
int RG_set_redirect_http_all(int enable, char * httpContent, int size)
{	
	int ret;
	rtk_rg_redirectHttpAll_t redInfo;

	printf("%s http size[%d]\n",__func__,size);
	if(enable)
		strncpy(redInfo.pushweb, httpContent, MAX_REDIRECT_PUSH_WEB_SIZE);
	else
		redInfo.pushweb[0] = '\0';
	
	redInfo.enable= enable;
	ret = rtk_rg_redirectHttpAll_set(&redInfo);
	if(ret != RT_ERR_OK){
		DBPRINT(1, "%s rtk_rg_redirectHttpAll_set failed ret[%x]\n", __func__, ret);
		return -1;
	}

	return 0;
}

static const char RG_REDIRECT_HTTP[] = "/proc/rg/redirect_first_http_req_set_url";
int RG_set_welcome_redirect(int enable, char * url)
{
	FILE *fp;
	
	if (enable)
	{
		fp = fopen(RG_REDIRECT_HTTP, "w");
		fprintf(fp, "a -1 %s", url);
		fclose(fp);
	}else
	{
		fp = fopen(RG_REDIRECT_HTTP, "w");
		fprintf(fp, "d -1");
		fclose(fp);
	}
	return 0;
}

int RG_add_redirectHttpURL(MIB_REDIRECT_URL_LIST_T * redirectUrl)
{
	int ret;
	rtk_rg_redirectHttpURL_t redInfo;
	
	strncpy(redInfo.url_str, redirectUrl->srcUrl, MAX_URL_FILTER_STR_LENGTH);
	strncpy(redInfo.dst_url_str, redirectUrl->dstUrl, MAX_URL_FILTER_STR_LENGTH);
	
	redInfo.count= redirectUrl->number;
	ret = rtk_rg_redirectHttpURL_add(&redInfo);
	if(ret != RT_ERR_OK){
		DBPRINT(1, "%s rtk_rg_redirectHttpURL_add ret[%x]\n", __func__, ret);
		return -1;
	}
	return 0;
}

int RG_del_redirectHttpURL(MIB_REDIRECT_URL_LIST_T * redirectUrl)
{
	int ret;
	rtk_rg_redirectHttpURL_t redInfo;

	strncpy(redInfo.url_str, redirectUrl->srcUrl, MAX_URL_FILTER_STR_LENGTH);
	
	ret = rtk_rg_redirectHttpURL_del(&redInfo);
	if(ret != RT_ERR_OK){
		DBPRINT(1, "%s rtk_rg_redirectHttpURL_del ret[%x]\n", __func__, ret);
		return -1;
	}
	return 0;
}

int RG_add_redirectWhiteUrl(MIB_REDIRECT_WHITE_LIST_T * whiteUrl)
{
	int ret;
	rtk_rg_redirectHttpWhiteList_t redInfo;
	
	strncpy(redInfo.url_str, whiteUrl->url, MAX_URL_FILTER_STR_LENGTH);
	strncpy(redInfo.keyword_str, whiteUrl->keyword, MAX_URL_FILTER_STR_LENGTH);
	ret = rtk_rg_redirectHttpWhiteList_add(&redInfo);
	if(ret != RT_ERR_OK){
		DBPRINT(1, "%s rtk_rg_redirectHttpWhiteList_add ret[%x]\n", __func__, ret);
		return -1;
	}
	return 0;
}

int RG_del_redirectWhiteUrl(MIB_REDIRECT_WHITE_LIST_T * whiteUrl)
{
	int ret;
	rtk_rg_redirectHttpWhiteList_t redInfo;

	strncpy(redInfo.url_str, whiteUrl->url, MAX_URL_FILTER_STR_LENGTH);
	strncpy(redInfo.keyword_str, whiteUrl->keyword, MAX_URL_FILTER_STR_LENGTH);
	ret = rtk_rg_redirectHttpWhiteList_del(&redInfo);
	if(ret != RT_ERR_OK){
		DBPRINT(1, "%s rtk_rg_redirectHttpWhiteList_del ret[%x]\n", __func__, ret);
		return -1;
	}
	return 0;
}

#endif
