#include <rtk/mib.h>
#include <rtk/ipv6_info.h>
#include <rtk/utility.h>

#include <cwmp_utility.h>

#include "prmt_apply.h"
#include "prmt_ctcom_lanext.h"
#include "cwmpc_utility.h"

#ifdef _PRMT_X_CT_COM_IPv6_
/*****************************************************************************
 * InternetGatewayDevice.LANDevice.{i}.X_CT-COM_RouterAdvertisement.
 *****************************************************************************/
struct CWMP_OP tCTRALeafOP = { getCTRA, setCTRA };

struct CWMP_PRMT tCTRALeafInfo[] = {
/*(name,		type,		flag,			op)*/
{"Enable",				eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCTRALeafOP},
{"MaxRtrAdvInterval",		eCWMP_tUINT,		CWMP_WRITE|CWMP_READ,	&tCTRALeafOP},
{"MinRtrAdvInterval",		eCWMP_tUINT,		CWMP_WRITE|CWMP_READ,	&tCTRALeafOP},
{"AdvManagedFlag",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCTRALeafOP},
{"AdvOtherConfigFlag",	eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCTRALeafOP},
};

enum eCTRALeaf
{
	eCTRA_Enable,
	eCTRA_MaxRtrAdvInterval,
	eCTRA_MinRtrAdvInterval,
	eCTRA_AdvManagedFlag,
	eCTRA_AdvOtherConfigFlag,
};

struct CWMP_LEAF tCTRALeaf[] = {
{ &tCTRALeafInfo[eCTRA_Enable] },
{ &tCTRALeafInfo[eCTRA_MaxRtrAdvInterval] },
{ &tCTRALeafInfo[eCTRA_MinRtrAdvInterval] },
{ &tCTRALeafInfo[eCTRA_AdvManagedFlag] },
{ &tCTRALeafInfo[eCTRA_AdvOtherConfigFlag] },
{ NULL },
};

int getCTRA(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;

	if (name == NULL || entity == NULL || type == NULL || data == NULL)
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if (strcmp(lastname, "Enable") == 0)
	{
		unsigned char enable;

		mib_get(MIB_V6_RADVD_ENABLE, (void *)&enable);

		*data = booldup(enable);
	}
	else if (strcmp(lastname, "MaxRtrAdvInterval") == 0)
	{
		unsigned int interval = 0;
		char buf[MAX_RADVD_CONF_LEN];

		mib_get(MIB_V6_MAXRTRADVINTERVAL, buf);
		sscanf(buf, "%u", &interval);

		*data = uintdup(interval);
	}
	else if (strcmp(lastname, "MinRtrAdvInterval") == 0)
	{
		unsigned int interval = 0;
		char buf[MAX_RADVD_CONF_LEN];

		mib_get(MIB_V6_MINRTRADVINTERVAL, buf);
		sscanf(buf, "%u", &interval);

		*data = uintdup(interval);
	}
	else if (strcmp(lastname, "AdvManagedFlag") == 0)
	{
		unsigned char flag;

		mib_get(MIB_V6_MANAGEDFLAG, &flag);
		*data = booldup(flag);
	}
	else if (strcmp(lastname, "AdvOtherConfigFlag") == 0)
	{
		unsigned char flag;

		mib_get(MIB_V6_OTHERCONFIGFLAG, &flag);
		*data = booldup(flag);
	}
	else {
		return ERR_9005;
	}

	return 0;
}

int setCTRA(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;

	if (name == NULL || entity == NULL) return -1;
	if (entity->info->type != type) return ERR_9006;
	if(data == NULL) return ERR_9007;

	if (strcmp(lastname, "Enable") == 0)
	{
		int *i = data;
		unsigned char enable = *i;

		mib_set(MIB_V6_RADVD_ENABLE, (void *)&enable);

		apply_add(CWMP_PRI_N, apply_radvd, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}
	else if (strcmp(lastname, "MaxRtrAdvInterval") == 0)
	{
		unsigned int *interval = data;
		char str[MAX_RADVD_CONF_LEN] = {0};

		if(*interval < 4 || *interval > 1800)
			return ERR_9007;

		sprintf(str, "%u", *interval);
		mib_set(MIB_V6_MAXRTRADVINTERVAL, str);

		apply_add(CWMP_PRI_N, apply_radvd, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}
	else if (strcmp(lastname, "MinRtrAdvInterval") == 0)
	{
		unsigned int *interval = data;
		char str[MAX_RADVD_CONF_LEN] = {0};

		if(*interval < 4 || *interval > 1800)
			return ERR_9007;

		sprintf(str, "%u", *interval);
		mib_set(MIB_V6_MINRTRADVINTERVAL, str);

		apply_add(CWMP_PRI_N, apply_radvd, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}
	else if (strcmp(lastname, "AdvManagedFlag") == 0)
	{
		int *i = data;
		unsigned char flag = *i;

		mib_set(MIB_V6_MANAGEDFLAG, &flag);

		apply_add(CWMP_PRI_N, apply_radvd, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}
	else if (strcmp(lastname, "AdvOtherConfigFlag") == 0)
	{
		int *i = data;
		unsigned char flag = *i;

		mib_set(MIB_V6_OTHERCONFIGFLAG, &flag);

		apply_add(CWMP_PRI_N, apply_radvd, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}
	else {
		return ERR_9005;
	}

	return 0;
}




/*****************************************************************************
 * InternetGatewayDevice.LANDevice.{i}.X_CT-COM_DHCPv6Server.
 *****************************************************************************/
struct CWMP_OP tCTDHCPv6ServerLeafOP = { getCTDHCPv6Server, setCTDHCPv6Server };

struct CWMP_PRMT tCTDHCPv6ServerLeafInfo[] = {
/*(name,		type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCTDHCPv6ServerLeafOP},
{"MinAddress",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTDHCPv6ServerLeafOP},
{"MaxAddress",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTDHCPv6ServerLeafOP},
{"PreferredLifeTime",	eCWMP_tUINT,		CWMP_WRITE|CWMP_READ,	&tCTDHCPv6ServerLeafOP},
{"ValidLifeTime",		eCWMP_tUINT,		CWMP_WRITE|CWMP_READ,	&tCTDHCPv6ServerLeafOP},
};

enum eCTDHCPv6ServerLeaf
{
	eCTDHCPv6_Enable,
	eCTDHCPv6_MinAddress,
	eCTDHCPv6_MaxAddress,
	eCTDHCPv6_PreferredLifeTime,
	eCTDHCPv6_ValidLifeTime,
};

struct CWMP_LEAF tCTDHCPv6ServerLeaf[] = {
{ &tCTDHCPv6ServerLeafInfo[eCTDHCPv6_Enable] },
{ &tCTDHCPv6ServerLeafInfo[eCTDHCPv6_MinAddress] },
{ &tCTDHCPv6ServerLeafInfo[eCTDHCPv6_MaxAddress] },
{ &tCTDHCPv6ServerLeafInfo[eCTDHCPv6_PreferredLifeTime] },
{ &tCTDHCPv6ServerLeafInfo[eCTDHCPv6_ValidLifeTime] },
{ NULL },
};

int getCTDHCPv6Server(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;
	unsigned int vUint;
	char buf[1024];
	struct in6_addr ip6Addr;

	if (name == NULL || entity == NULL || type == NULL || data == NULL)
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if (strcmp(lastname, "Enable") == 0)
	{
		unsigned char enable;
		mib_get(MIB_DHCPV6_MODE, &enable);

		if(enable == DHCP_LAN_SERVER_AUTO || enable == DHCP_LAN_SERVER)
			*data = booldup(1);
		else
			*data = booldup(0);
	}
	else if (strcmp(lastname, "MinAddress") == 0)
	{
		int i;
		char min_addr[20] = {0};

		mib_get(MIB_DHCPV6S_MIN_ADDRESS, min_addr);
		*data = strdup(min_addr);
	}
	else if (strcmp(lastname, "MaxAddress") == 0)
	{
		int i;
		char max_addr[64] = {0};

		mib_get(MIB_DHCPV6S_MAX_ADDRESS, max_addr);
		*data = strdup(max_addr);
	}
	else if (strcmp(lastname, "PreferredLifeTime") == 0)
	{
		mib_get(MIB_DHCPV6S_PREFERRED_LIFETIME, &vUint);
		*data = uintdup(vUint);
	}
	else if (strcmp(lastname, "ValidLifeTime") == 0)
	{
		mib_get(MIB_DHCPV6S_DEFAULT_LEASE, &vUint);
		*data = uintdup(vUint);
	}
	else {
		return ERR_9005;
	}

	return 0;
}

int setCTDHCPv6Server(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;
	int vInt;
	unsigned int mode;
	FILE *proc;

	if (name == NULL || entity == NULL) return -1;
	if (entity->info->type != type) return ERR_9006;
	if (data == NULL) return ERR_9007;

	if (strcmp(lastname, "Enable") == 0)
	{
		int *i = data;
		unsigned char enable = (*i == 1) ? DHCP_LAN_SERVER_AUTO : DHCP_LAN_NONE;

		mib_set(MIB_DHCPV6_MODE, &enable);

		apply_add(CWMP_PRI_N, apply_DHCPv6S, CWMP_RESTART, 0, NULL, 0);
	}
	else if (strcmp(lastname, "MinAddress") == 0)
	{
		char *buf = data;

		mib_set(MIB_DHCPV6S_MIN_ADDRESS, buf);
		apply_add(CWMP_PRI_N, apply_DHCPv6S, CWMP_RESTART, 0, NULL, 0);
	}
	else if (strcmp(lastname, "MaxAddress") == 0)
	{
		char *buf = data;

		mib_set(MIB_DHCPV6S_MAX_ADDRESS, buf);
		apply_add(CWMP_PRI_N, apply_DHCPv6S, CWMP_RESTART, 0, NULL, 0);
	}
	else if (strcmp(lastname, "PreferredLifeTime") == 0)
	{
		unsigned int *i = data;

		mib_set(MIB_DHCPV6S_DEFAULT_LEASE, i);
		apply_add(CWMP_PRI_N, apply_DHCPv6S, CWMP_RESTART, 0, NULL, 0);
	}
	else if (strcmp(lastname, "ValidLifeTime") == 0)
	{
		unsigned int *i = data;

		mib_set(MIB_DHCPV6S_PREFERRED_LIFETIME, i);
		apply_add(CWMP_PRI_N, apply_DHCPv6S, CWMP_RESTART, 0, NULL, 0);
	}
	else {
		return ERR_9005;
	}

	return 0;
}




/*****************************************************************************
 * InternetGatewayDevice.LANDevice.{i}.X_CT-COM_IPv6Config.PrefixInformation.1.
 *****************************************************************************/
struct CWMP_OP tCTIPv6PrefixInfoEntityLeafOP = { getCTIPv6PrefixInfoEntity, setCTIPv6PrefixInfoEntity };

struct CWMP_PRMT tCTIPv6PrefixInfoEntityLeafInfo[] = {
/*(name,		type,		flag,			op)*/
{"Mode",				eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTIPv6PrefixInfoEntityLeafOP},
{"DelegatedWanConnection",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTIPv6PrefixInfoEntityLeafOP},
{"Prefix",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTIPv6PrefixInfoEntityLeafOP},
{"PreferredLifeTime",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tCTIPv6PrefixInfoEntityLeafOP},
{"ValidLifeTime",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tCTIPv6PrefixInfoEntityLeafOP},
};

enum eCTIPv6PrefixInfoEntityLeaf
{
	eCTPI_Mode,
	eCTPI_DelegatedWanConnection,
	eCTPI_Prefix,
	eCTPI_PreferredLifeTime,
	eCTPI_ValidLifeTime,
};

struct CWMP_LEAF tCTIPv6PrefixInfoEntityLeaf[] = {
{ &tCTIPv6PrefixInfoEntityLeafInfo[eCTPI_Mode] },
{ &tCTIPv6PrefixInfoEntityLeafInfo[eCTPI_DelegatedWanConnection] },
{ &tCTIPv6PrefixInfoEntityLeafInfo[eCTPI_Prefix] },
{ &tCTIPv6PrefixInfoEntityLeafInfo[eCTPI_PreferredLifeTime] },
{ &tCTIPv6PrefixInfoEntityLeafInfo[eCTPI_ValidLifeTime] },
{ NULL },
};

int get_delegated_wan()
{
	int total = mib_chain_total(MIB_ATM_VC_TBL);
	int i = 0;
	MIB_CE_ATM_VC_T entry = {0};

	for(i = 0 ; i < total ; i++)
	{
		mib_chain_get(MIB_ATM_VC_TBL, i, &entry);
		if(entry.applicationtype & X_CT_SRV_INTERNET
			&& (entry.Ipv6DhcpRequest & 0x2))
		{
			return entry.ifIndex;
		}
	}

	return DUMMY_IFINDEX;	//failed
}

int getCTIPv6PrefixInfoEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;
	FILE *proc;

	if (name == NULL || entity == NULL || type == NULL || data == NULL)
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if (strcmp(lastname, "Mode") == 0)
	{
		unsigned char mode;
		mib_get(MIB_PREFIXINFO_PREFIX_MODE, (void *)&mode);

		switch(mode)
		{
		case IPV6_PREFIX_DELEGATION:
			*data = strdup("WANDelegated");
			break;
		case IPV6_PREFIX_STATIC:
			*data = strdup("Static");
			break;
		default:
			*data = strdup("None");
			break;
		}
	}
	else if (strcmp(lastname, "DelegatedWanConnection") == 0)
	{
		int i = 0;
		char path[256] = {0};
		unsigned int ifIndex = DUMMY_IFINDEX;

		mib_get(MIB_PREFIXINFO_DELEGATED_WANCONN, &ifIndex);

		if(ifIndex != DUMMY_IFINDEX)
		{
			transfer2PathName(ifIndex, path);
			*data = strdup(path);
		}
		else
			*data = strdup("");
	}
	else if (strcmp(lastname, "Prefix") == 0)
	{
		char buf[50] = {0};
		int ret;
		PREFIX_V6_INFO_T prefixInfo = {0};
		char prefix[40] = {0};

		if(get_prefixv6_info(&prefixInfo) == 0 )
		{

			if (inet_ntop(AF_INET6, (void *)prefixInfo.prefixIP, prefix, 40) == NULL)
				return ERR_9002;

			snprintf(buf, 50, "%s/%d", prefix, prefixInfo.prefixLen);
		}
		*data = strdup(buf);
	}
	else if (strcmp(lastname, "PreferredLifeTime") == 0)
	{
		char buf[50] = {0};
		int ret;
		PREFIX_V6_INFO_T prefixInfo = {0};
		char prefix[40] = {0};

		if(get_prefixv6_info(&prefixInfo) == 0)
		{
			*data = uintdup(prefixInfo.PLTime);
		}
		else
			*data = uintdup(0);
	}
	else if (strcmp(lastname, "ValidLifeTime") == 0)
	{
		char buf[50] = {0};
		int ret;
		PREFIX_V6_INFO_T prefixInfo = {0};
		char prefix[40] = {0};

		if(get_prefixv6_info(&prefixInfo) == 0)
		{
			*data = uintdup(prefixInfo.MLTime);
		}
		else
			*data = uintdup(0);
	}
	else {
		return ERR_9005;
	}

	return 0;
}


int setCTIPv6PrefixInfoEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;
	int vInt;
	unsigned int mode;
	FILE *proc;

	if (name == NULL || entity == NULL || data == NULL)
		return -1;

	if (entity->info->type != type)
		return ERR_9006;

	if (strcmp(lastname, "Mode") == 0)
	{
		char *buf = data;
		unsigned char mode;

		if(strcmp(buf, "WANDelegated") == 0)
			mode = IPV6_PREFIX_DELEGATION;
		else if(strcmp(buf, "Static") == 0)
			mode = IPV6_PREFIX_STATIC;
		else
			return ERR_9007;

		mib_set(MIB_PREFIXINFO_PREFIX_MODE, &mode);
		apply_add(CWMP_PRI_N, apply_lanv6_server, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}
	else if (strcmp(lastname, "DelegatedWanConnection") == 0)
	{
		char *buf = data;
		unsigned int ifIndex = DUMMY_IFINDEX;

		ifIndex = transfer2IfIndex(buf);

		if(ifIndex == DUMMY_IFINDEX)
			return ERR_9007;

		mib_set(MIB_PREFIXINFO_DELEGATED_WANCONN, &ifIndex);
		apply_add(CWMP_PRI_N, apply_lanv6_server, CWMP_RESTART, 0, NULL, 0);
		return CWMP_APPLIED;
	}
	else if (strcmp(lastname, "Prefix") == 0)
	{
		char *buf = data;
		unsigned char mode;
		unsigned char len;
		char addr[40];
		struct in6_addr prefix = {0};

#if 0	//ITMS will set this parameter even if mode is not static
		mib_get(MIB_PREFIXINFO_PREFIX_MODE, &mode);
		if(mode != IPV6_PREFIX_STATIC)
			return ERR_9001;
#endif
		if(sscanf(buf, "%[^/]/%hhu", addr, &len) != 2)
			return ERR_9007;

		if(inet_pton(AF_INET6, addr, &prefix) != 1)
			return ERR_9007;

		mib_set(MIB_IPV6_LAN_PREFIX, &addr);
		mib_set(MIB_IPV6_LAN_PREFIX_LEN, &len);
		apply_add(CWMP_PRI_N, apply_lanv6_server, CWMP_RESTART, 0, NULL, 0);
	}
	else if (strcmp(lastname, "PreferredLifeTime") == 0)
	{
		unsigned int *i = data;
		unsigned char mode;

#if 0	//ITMS will set this parameter even if mode is not static
		mib_get(MIB_PREFIXINFO_PREFIX_MODE, &mode);
		if(mode != IPV6_PREFIX_STATIC)
			return ERR_9001;
#endif
		mib_set(MIB_V6_PREFERREDLIFETIME, i);
		apply_add(CWMP_PRI_N, apply_lanv6_server, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}
	else if (strcmp(lastname, "ValidLifeTime") == 0)
	{
		unsigned int *i = data;
		unsigned char mode;

#if 0	//ITMS will set this parameter even if mode is not static
		mib_get(MIB_PREFIXINFO_PREFIX_MODE, &mode);
		if(mode != IPV6_PREFIX_STATIC)
			return ERR_9001;
#endif

		mib_set(MIB_V6_VALIDLIFETIME, i);
		apply_add(CWMP_PRI_N, apply_lanv6_server, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}
	else {
		return ERR_9005;
	}

	return 0;
}



/*****************************************************************************
 * InternetGatewayDevice.LANDevice.{i}.X_CT-COM_IPv6Config.PrefixInformation.
 *****************************************************************************/
struct CWMP_PRMT tCTIPv6PrefixObjectInfo[] =
{
/* name,					 type,		 flag,		 op */
{"1", 	 eCWMP_tOBJECT,  CWMP_READ,  NULL},
};

enum eCTIPv6PrefixObject
{
	ePrefixInfo1,
};

struct CWMP_NODE tCTIPv6PrefixObject[] =
{
/* info,									 leaf,					 next */
{ &tCTIPv6PrefixObjectInfo[ePrefixInfo1],		 tCTIPv6PrefixInfoEntityLeaf, NULL},
{ NULL, NULL, NULL },
};

/*****************************************************************************
 * InternetGatewayDevice.LANDevice.{i}.X_CT-COM_IPv6Config.
 *****************************************************************************/
struct CWMP_OP tCTIPv6ConfigLeafOP = { getCTIPv6Config, setCTIPv6Config };

struct CWMP_PRMT tCTIPv6ConfigLeafInfo[] = {
/*(name,		type,		flag,			op)*/
{"DomainName",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTIPv6ConfigLeafOP},
{"IPv6DNSConfigType",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTIPv6ConfigLeafOP},
{"IPv6DNSWANConnection",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTIPv6ConfigLeafOP},
{"IPv6DNSServers",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTIPv6ConfigLeafOP},

};

enum eCTIPv6ConfigLeaf
{
	eCT_DomainName,
	eCT_IPv6DNSConfigType,
	eCT_IPv6DNSWANConnection,
	eCT_IPv6DNSServers,
};

struct CWMP_LEAF tCTIPv6ConfigLeaf[] = {
{ &tCTIPv6ConfigLeafInfo[eCT_DomainName] },
{ &tCTIPv6ConfigLeafInfo[eCT_IPv6DNSConfigType] },
{ &tCTIPv6ConfigLeafInfo[eCT_IPv6DNSWANConnection] },
{ &tCTIPv6ConfigLeafInfo[eCT_IPv6DNSServers] },
{ NULL },
};

struct CWMP_PRMT tCTIPv6ConfigObjectInfo[] =
{
/* name,					 type,		 flag,		 op */
{"PrefixInformation", 	 eCWMP_tOBJECT,  CWMP_READ,  NULL},
};

enum eCTIPv6ConfigObject
{
	ePrefixInformation,
};

struct CWMP_NODE tCTIPv6ConfigObject[] =
{
/* info,									 leaf,					 next */
{ &tCTIPv6ConfigObjectInfo[ePrefixInformation],	NULL,	tCTIPv6PrefixObject},
{ NULL, NULL, NULL },
};


int getCTIPv6Config(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;
	unsigned int mode;
	FILE *proc;

	if (name == NULL || entity == NULL || type == NULL || data == NULL)
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if (strcmp(lastname, "DomainName") == 0)
	{
		int total = mib_chain_total(MIB_DHCPV6S_DOMAIN_SEARCH_TBL);
		MIB_DHCPV6S_DOMAIN_SEARCH_T entry = {0};

		if(total > 0)
		{
			mib_chain_get(MIB_DHCPV6S_DOMAIN_SEARCH_TBL, 0, &entry);
			*data = strdup(entry.domain);
		}
		else
			*data = strdup("");
	}
	else if (strcmp(lastname, "IPv6DNSConfigType") == 0)
	{
		unsigned char mode;
		mib_get(MIB_LAN_DNSV6_MODE, &mode);

		switch(mode)
		{
		case IPV6_DNS_HGWPROXY:
			*data = strdup("HGWProxy");
			break;
		case IPV6_DNS_WANCONN:
			*data = strdup("WANConnection");
			break;
		case IPV6_DNS_STATIC:
			*data = strdup("Static");
			break;
		}
	}
	else if (strcmp(lastname, "IPv6DNSWANConnection") == 0)
	{
		int i = 0;
		char path[256] = {0};
		unsigned int ifIndex = DUMMY_IFINDEX;

		mib_get(MIB_DNSINFO_WANCONN, (void *)&ifIndex);

		if(ifIndex != DUMMY_IFINDEX)
			transfer2PathName(ifIndex, path);

		*data = strdup(path);
	}
	else if (strcmp(lastname, "IPv6DNSServers") == 0)
	{
		DNS_V6_INFO_T dnsV6Info = {0};

		get_dnsv6_info(&dnsV6Info);
		*data = strdup(dnsV6Info.nameServer);
	}
	else {
		return ERR_9005;
	}

	return 0;
}


int setCTIPv6Config(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;
	int vInt;
	unsigned int mode;
	FILE *proc;

	if (name == NULL || entity == NULL || data == NULL)
		return -1;

	if (entity->info->type != type)
		return ERR_9006;

	if (strcmp(lastname, "DomainName") == 0)
	{
		char *buf = data;
		char *domain = NULL;
		MIB_DHCPV6S_DOMAIN_SEARCH_T entry = {0};

		if(strlen(buf) >= MAX_DOMAIN_LENGTH)
			return ERR_9007;

		strcpy(entry.domain, buf);
		mib_chain_clear(MIB_DHCPV6S_DOMAIN_SEARCH_TBL);
		mib_chain_add(MIB_DHCPV6S_DOMAIN_SEARCH_TBL, &entry);

		apply_add(CWMP_PRI_N, apply_DHCPv6S, CWMP_RESTART, 0, NULL, 0);

		return 0;
	}
	else if (strcmp(lastname, "IPv6DNSConfigType") == 0)
	{
		char *buf = data;
		unsigned char mode;

		if(strcmp(buf, "HGWProxy") == 0)
			mode = IPV6_DNS_HGWPROXY;
		else if(strcmp(buf, "WANConnection") == 0)
			mode = IPV6_DNS_WANCONN;
		else if(strcmp(buf, "Static") == 0)
			mode = IPV6_DNS_STATIC;
		else
			return ERR_9007;

		mib_set(MIB_LAN_DNSV6_MODE, &mode);
		apply_add(CWMP_PRI_N, apply_lanv6_server, CWMP_RESTART, 0, NULL, 0);

		return 0;
	}
	else if (strcmp(lastname, "IPv6DNSWANConnection") == 0)
	{
		char *buf = data;
		int ifIndex = DUMMY_IFINDEX;
		int total = mib_chain_total(MIB_ATM_VC_TBL);
		int i;
		int old_ifIndex = DUMMY_IFINDEX;
		MIB_CE_ATM_VC_T vc_entity;
		int changed = 0;

		ifIndex = transfer2IfIndex(buf);

		if(ifIndex == DUMMY_IFINDEX)
			return ERR_9007;

		mib_get(MIB_DNSINFO_WANCONN, (void *)&old_ifIndex);
		if(ifIndex == old_ifIndex)
			return 0;

		mib_set(MIB_DNSINFO_WANCONN, &ifIndex);

		apply_add(CWMP_PRI_N, apply_lanv6_server, CWMP_RESTART, 0, NULL, 0);
		return CWMP_APPLIED;
	}
	else if (strcmp(lastname, "IPv6DNSServers") == 0)
	{
		char *buf = data;
		struct in6_addr ip6Addr;
		char *dns = NULL;
		unsigned char mode;
		int mib_id = MIB_ADSL_WAN_DNSV61;

#if 0	//ITMS will set this parameter even if mode is not static
		mib_get(MIB_LAN_DNSV6_MODE, &mode);
		if(mode != IPV6_DNS_STATIC)
			return ERR_9001;
#endif
		dns = strtok(buf, ",");
		while(dns)
		{
			if(mib_id > MIB_ADSL_WAN_DNSV62)
				return ERR_9007;

			if (inet_pton(AF_INET6, dns, &ip6Addr) != 1)
				return ERR_9007;

			mib_set(mib_id, &ip6Addr);
			dns = strtok(NULL, ",");
			mib_id++;
		}
		apply_add(CWMP_PRI_N, apply_lanv6_server, CWMP_RESTART, 0, NULL, 0);

		return 0;
	}
	else {
		return ERR_9005;
	}

	return 0;
}




/*****************************************************************************
 * InternetGatewayDevice.LANDevice.{i}.LANHostConfigManagement.IPInterface.{i}.X_CT-COM_IPv6Address.
 *****************************************************************************/
struct CWMP_OP tCTIPv6LocalAddressLeafOP = { getCTIPv6LocalAddress, setCTIPv6LocalAddress };

struct CWMP_PRMT tCTIPv6LocalAddressLeafInfo[] = {
/*(name,		type,		flag,			op)*/
{"LocalAddress",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTIPv6LocalAddressLeafOP},
};

enum eCTIPv6LocalAddressLeaf
{
	eCT_LocalAddress,
};

struct CWMP_LEAF tCTIPv6LocalAddressLeaf[] = {
{ &tCTIPv6LocalAddressLeafInfo[eCT_LocalAddress] },
{ NULL },
};

int getCTIPv6LocalAddress(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;
	unsigned int mode;
	FILE *proc;

	if (name == NULL || entity == NULL || type == NULL || data == NULL)
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if (strcmp(lastname, "LocalAddress") == 0)
	{
		char buf[MAX_V6_IP_LEN]= {0};

		if(getInstNum(name, "IPInterface") == 1)
		{
			mib_get(MIB_IPV6_LAN_IP_ADDR, buf);
			*data = strdup(buf);
		}
		else
			*data = strdup("");
	}
	else {
		return ERR_9005;
	}

	return 0;
}

int setCTIPv6LocalAddress(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;
	int vInt;
	unsigned int mode;
	FILE *proc;

	if (name == NULL || entity == NULL || data == NULL)
		return -1;

	if (entity->info->type != type)
		return ERR_9006;

	if (strcmp(lastname, "LocalAddress") == 0)
	{
		char *buf = data;
		struct in6_addr ip6Addr;
		char ip6_str[MAX_V6_IP_LEN]= {0};
		char old_ip6_str[MAX_V6_IP_LEN]= {0};

		if(getInstNum(name, "IPInterface") == 1)
		{
			if (inet_pton(AF_INET6, buf, &ip6Addr) != 1)
				return ERR_9007;

			strncpy(ip6_str, buf, MAX_V6_IP_LEN);

			mib_get(MIB_IPV6_LAN_IP_ADDR, old_ip6_str);
			mib_set(MIB_IPV6_LAN_IP_ADDR, ip6_str);

			apply_add(CWMP_PRI_N, apply_ip6_lan_addr, CWMP_RESTART, 0, old_ip6_str, MAX_V6_IP_LEN);
		}
		else
			return ERR_9001;
	}
	else {
		return ERR_9005;
	}

	return 0;
}


/*****************************************************************************
 * InternetGatewayDevice.LANDevice.{i}.LANHostConfigManagement.IPInterface.{i}.
 *****************************************************************************/
struct CWMP_PRMT tCTIPInterfaceEntityObjectInfo[] =
{
/* name,					 type,		 flag,		 op */
{"X_CT-COM_IPv6Address", 	 eCWMP_tOBJECT,  CWMP_READ,  NULL},
};

enum eCTIPInterfaceEntityObject
{
 eX_CTCOM_IPv6Address,
};

struct CWMP_NODE tCTIPInterfaceEntityObject[] =
{
/* info,									 leaf,					 next */
{ &tCTIPInterfaceEntityObjectInfo[eX_CTCOM_IPv6Address],		 tCTIPv6LocalAddressLeaf, NULL},
{ NULL, NULL, NULL },
};
#endif	//_PRMT_X_CT_COM_IPv6_


/*****************************************************************************
 * InternetGatewayDevice.LANDevice.{i}.LANHostConfigManagement.
 *****************************************************************************/
#ifdef _PRMT_X_CT_COM_DHCP_
struct CWMP_OP tCTLANHostConfLeafOP = { getCTLANHostConf, setCTLANHostConf };
struct CWMP_PRMT tCTLANHostConfLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"X_CT-COM_OPTION60Enable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCTLANHostConfLeafOP},
{"X_CT-COM_OPTION125Enable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCTLANHostConfLeafOP},
{"X_CT-COM_OPTION16Enable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCTLANHostConfLeafOP},
{"X_CT-COM_OPTION17Enable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCTLANHostConfLeafOP},
{"X_CT-COM_STB-MinAddress",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTLANHostConfLeafOP},
{"X_CT-COM_STB-MaxAddress",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTLANHostConfLeafOP},
{"X_CT-COM_Phone-MinAddress",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTLANHostConfLeafOP},
{"X_CT-COM_Phone-MaxAddress",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTLANHostConfLeafOP},
{"X_CT-COM_Camera-MinAddress",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTLANHostConfLeafOP},
{"X_CT-COM_Camera-MaxAddress",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTLANHostConfLeafOP},
{"X_CT-COM_Computer-MinAddress",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTLANHostConfLeafOP},
{"X_CT-COM_Computer-MaxAddress",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTLANHostConfLeafOP},
};

int getCTLANHostConf(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char buf[256]="";
	unsigned char vChar=0;
	int  vInt=0;
	int i, entryNum;
	DHCPS_SERVING_POOL_T dhcppoolentry;

	entryNum = mib_chain_total(MIB_DHCPS_SERVING_POOL_TBL);

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "X_CT-COM_OPTION60Enable" )==0 )
	{
		mib_get(CWMP_CT_DHCPS_CHECK_OPT_60, &vChar);
		*data = booldup(vChar ? 1 : 0);
	}
	else if( strcmp( lastname, "X_CT-COM_OPTION125Enable" )==0 )
	{
		mib_get(CWMP_CT_DHCPS_SEND_OPT_125, &vChar);
		*data = booldup(vChar ? 1 : 0);
	}
	else if( strcmp( lastname, "X_CT-COM_OPTION16Enable" )==0 )
	{
		mib_get(CWMP_CT_DHCP6S_CHECK_OPT_16, &vChar);
		*data = booldup(vChar ? 1 : 0);
	}
	else if( strcmp( lastname, "X_CT-COM_OPTION17Enable" )==0 )
	{
		mib_get(CWMP_CT_DHCP6S_SEND_OPT_17, &vChar);
		*data = booldup(vChar ? 1 : 0);
	}
	else if( strcmp( lastname, "X_CT-COM_STB-MinAddress" )==0 )
	{
		unsigned int stb_min_ip = 0;

		for( i = 0; i < entryNum; i++)
		{
			if(!mib_chain_get(MIB_DHCPS_SERVING_POOL_TBL, i, (void *)&dhcppoolentry))
				continue;

			if(dhcppoolentry.poolname)
				if(!strcmp(dhcppoolentry.poolname, "STB"))
					memcpy(&stb_min_ip, dhcppoolentry.startaddr, IP_ADDR_LEN);
		}

		if(stb_min_ip)
			*data = strdup(inet_ntoa(*(struct in_addr*)&stb_min_ip));
		else
			*data = strdup("");
	}
	else if( strcmp( lastname, "X_CT-COM_STB-MaxAddress" )==0 )
	{
		unsigned int stb_max_ip = 0;

		for( i = 0; i < entryNum; i++)
		{
			if(!mib_chain_get(MIB_DHCPS_SERVING_POOL_TBL, i, (void *)&dhcppoolentry))
				continue;

			if(dhcppoolentry.poolname)
				if(!strcmp(dhcppoolentry.poolname, "STB"))
					memcpy(&stb_max_ip, dhcppoolentry.endaddr, IP_ADDR_LEN);
		}

		if(stb_max_ip)
			*data = strdup(inet_ntoa(*(struct in_addr*)&stb_max_ip));
		else
			*data = strdup("");
	}
	else if( strcmp( lastname, "X_CT-COM_Phone-MinAddress" )==0 )
	{
		unsigned int phone_min_ip = 0;

		for( i = 0; i < entryNum; i++)
		{
			if(!mib_chain_get(MIB_DHCPS_SERVING_POOL_TBL, i, (void *)&dhcppoolentry))
				continue;

			if(dhcppoolentry.poolname)
				if(!strcmp(dhcppoolentry.poolname, "Phone"))
					memcpy(&phone_min_ip, dhcppoolentry.startaddr, IP_ADDR_LEN);
		}

		if(phone_min_ip)
			*data = strdup(inet_ntoa(*(struct in_addr*)&phone_min_ip));
		else
			*data = strdup("");
	}
	else if( strcmp( lastname, "X_CT-COM_Phone-MaxAddress" )==0 )
	{
		unsigned int phone_max_ip = 0;

		for( i = 0; i < entryNum; i++)
		{
			if(!mib_chain_get(MIB_DHCPS_SERVING_POOL_TBL, i, (void *)&dhcppoolentry))
				continue;

			if(dhcppoolentry.poolname)
				if(!strcmp(dhcppoolentry.poolname, "Phone"))
					memcpy(&phone_max_ip, dhcppoolentry.endaddr, IP_ADDR_LEN);
		}

		if(phone_max_ip)
			*data = strdup(inet_ntoa(*(struct in_addr*)&phone_max_ip));
		else
			*data = strdup("");
	}
	else if( strcmp( lastname, "X_CT-COM_Camera-MinAddress" )==0 )
	{
		unsigned int camera_min_ip = 0;

		for( i = 0; i < entryNum; i++)
		{
			if(!mib_chain_get(MIB_DHCPS_SERVING_POOL_TBL, i, (void *)&dhcppoolentry))
				continue;

			if(dhcppoolentry.poolname)
				if(!strcmp(dhcppoolentry.poolname, "Camera"))
					memcpy(&camera_min_ip, dhcppoolentry.startaddr, IP_ADDR_LEN);
		}

		if(camera_min_ip)
			*data = strdup(inet_ntoa(*(struct in_addr*)&camera_min_ip));
		else
			*data = strdup("");
	}
	else if( strcmp( lastname, "X_CT-COM_Camera-MaxAddress" )==0 )
	{
		unsigned int camera_max_ip = 0;

		for( i = 0; i < entryNum; i++)
		{
			if(!mib_chain_get(MIB_DHCPS_SERVING_POOL_TBL, i, (void *)&dhcppoolentry))
				continue;

			if(dhcppoolentry.poolname)
				if(!strcmp(dhcppoolentry.poolname, "Camera"))
					memcpy(&camera_max_ip, dhcppoolentry.endaddr, IP_ADDR_LEN);
		}

		if(camera_max_ip)
			*data = strdup(inet_ntoa(*(struct in_addr*)&camera_max_ip));
		else
			*data = strdup("");
	}
	else if( strcmp( lastname, "X_CT-COM_Computer-MinAddress" )==0 )
	{
		unsigned int computer_min_ip = 0;

		for( i = 0; i < entryNum; i++)
		{
			if(!mib_chain_get(MIB_DHCPS_SERVING_POOL_TBL, i, (void *)&dhcppoolentry))
				continue;

			if(dhcppoolentry.poolname)
				if(!strcmp(dhcppoolentry.poolname, "Computer"))
					memcpy(&computer_min_ip, dhcppoolentry.startaddr, IP_ADDR_LEN);
		}

		if(computer_min_ip)
			*data = strdup(inet_ntoa(*(struct in_addr*)&computer_min_ip));
		else
			*data = strdup("");
	}
	else if( strcmp( lastname, "X_CT-COM_Computer-MaxAddress" )==0 )
	{
		unsigned int computer_max_ip = 0;

		for( i = 0; i < entryNum; i++)
		{
			if(!mib_chain_get(MIB_DHCPS_SERVING_POOL_TBL, i, (void *)&dhcppoolentry))
				continue;

			if(dhcppoolentry.poolname)
				if(!strcmp(dhcppoolentry.poolname, "Computer"))
					memcpy(&computer_max_ip, dhcppoolentry.endaddr, IP_ADDR_LEN);
		}

		if(computer_max_ip)
			*data = strdup(inet_ntoa(*(struct in_addr*)&computer_max_ip));
		else
			*data = strdup("");
	}
	else{
		return ERR_9005;
	}

	return 0;
}

int setCTLANHostConf(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned char	vChar=0;
	unsigned char	SerEn=0;
	struct in_addr addr;
	int i, entryNum;
	DHCPS_SERVING_POOL_T dhcppoolentry;

	entryNum = mib_chain_total(MIB_DHCPS_SERVING_POOL_TBL);

	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;
	if( data==NULL ) return ERR_9007;

	mib_get( CWMP_DHCP_SERVERCONF, (void *)&SerEn);

	if( strcmp( lastname, "X_CT-COM_OPTION60Enable" )==0 )
	{
		int *tmp = data;
		unsigned char enable = (*tmp == 1) ? 1 : 0;

		mib_set(CWMP_CT_DHCPS_CHECK_OPT_60, &enable);
		apply_add( CWMP_PRI_L, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "X_CT-COM_OPTION125Enable" )==0 )
	{
		int *tmp = data;
		unsigned char enable = (*tmp == 1) ? 1 : 0;

		mib_set(CWMP_CT_DHCPS_SEND_OPT_125, &enable);
		apply_add( CWMP_PRI_L, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "X_CT-COM_OPTION16Enable" )==0 )
	{
		int *tmp = data;
		unsigned char enable = (*tmp == 1) ? 1 : 0;

		mib_set(CWMP_CT_DHCP6S_CHECK_OPT_16, &enable);
		apply_add( CWMP_PRI_L, apply_DHCPv6S, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "X_CT-COM_OPTION17Enable" )==0 )
	{
		int *tmp = data;
		unsigned char enable = (*tmp == 1) ? 1 : 0;

		mib_set(CWMP_CT_DHCP6S_CHECK_OPT_16, &enable);
		apply_add( CWMP_PRI_L, apply_DHCPv6S, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "X_CT-COM_STB-MinAddress" )==0 )
	{
		if( strlen(buf)==0 ) return ERR_9007;

		if( SerEn==0 ) return ERR_9001;

		if( inet_aton( buf, &addr )==0 ) return ERR_9007; //the ip address is error.

		for( i = 0; i < entryNum; i++)
		{
			if(!mib_chain_get(MIB_DHCPS_SERVING_POOL_TBL, i, (void *)&dhcppoolentry))
				continue;

			if(dhcppoolentry.poolname)
			{
				if(!strcmp(dhcppoolentry.poolname, "STB"))
				{
					memcpy(dhcppoolentry.startaddr, &addr, IP_ADDR_LEN);
					break;
				}
			}
		}

		mib_chain_update(MIB_DHCPS_SERVING_POOL_TBL, (void *)&dhcppoolentry, i);

		apply_add(CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}
	else if( strcmp( lastname, "X_CT-COM_STB-MaxAddress" )==0 )
	{
		if( strlen(buf)==0 ) return ERR_9007;

		if( SerEn==0 ) return ERR_9001;

		if( inet_aton( buf, &addr )==0 ) return ERR_9007; //the ip address is error.

		for(i = 0; i < entryNum; i++)
		{
			if(!mib_chain_get(MIB_DHCPS_SERVING_POOL_TBL, i, (void *)&dhcppoolentry))
				continue;

			if(dhcppoolentry.poolname)
			{
				if(!strcmp(dhcppoolentry.poolname, "STB"))
				{
					memcpy(dhcppoolentry.endaddr, &addr, IP_ADDR_LEN);
					break;
				}
			}
		}

		mib_chain_update(MIB_DHCPS_SERVING_POOL_TBL, (void *)&dhcppoolentry, i);

		apply_add(CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}
	else if( strcmp( lastname, "X_CT-COM_Phone-MinAddress" )==0 )
	{
		if( strlen(buf)==0 ) return ERR_9007;

		if( SerEn==0 ) return ERR_9001;

		if( inet_aton( buf, &addr )==0 ) return ERR_9007; //the ip address is error.

		for(i = 0; i < entryNum; i++)
		{
			if(!mib_chain_get(MIB_DHCPS_SERVING_POOL_TBL, i, (void *)&dhcppoolentry))
				continue;

			if(dhcppoolentry.poolname)
			{
				if(!strcmp(dhcppoolentry.poolname, "Phone"))
				{
					memcpy(dhcppoolentry.startaddr, &addr, IP_ADDR_LEN);
					break;
				}
			}
		}

		mib_chain_update(MIB_DHCPS_SERVING_POOL_TBL, (void *)&dhcppoolentry, i);

		apply_add(CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}
	else if( strcmp( lastname, "X_CT-COM_Phone-MaxAddress" )==0 )
	{
		if( strlen(buf)==0 ) return ERR_9007;

		if( SerEn==0 ) return ERR_9001;

		if( inet_aton( buf, &addr )==0 ) return ERR_9007; //the ip address is error.

		for( i = 0; i < entryNum; i++)
		{
			if(!mib_chain_get(MIB_DHCPS_SERVING_POOL_TBL, i, (void *)&dhcppoolentry))
				continue;

			if(dhcppoolentry.poolname)
			{
				if(!strcmp(dhcppoolentry.poolname, "Phone"))
				{
					memcpy(dhcppoolentry.endaddr, &addr, IP_ADDR_LEN);
					break;
				}
			}
		}

		mib_chain_update(MIB_DHCPS_SERVING_POOL_TBL, (void *)&dhcppoolentry, i);

		apply_add(CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}
	else if( strcmp( lastname, "X_CT-COM_Camera-MinAddress" )==0 )
	{
		if( strlen(buf)==0 ) return ERR_9007;

		if( SerEn==0 ) return ERR_9001;

		if( inet_aton( buf, &addr )==0 ) return ERR_9007; //the ip address is error.

		for(i = 0; i < entryNum; i++)
		{
			if(!mib_chain_get(MIB_DHCPS_SERVING_POOL_TBL, i, (void *)&dhcppoolentry))
				continue;

			if(dhcppoolentry.poolname)
			{
				if(!strcmp(dhcppoolentry.poolname, "Camera"))
				{
					memcpy(dhcppoolentry.startaddr, &addr, IP_ADDR_LEN);
					break;
				}
			}
		}

		mib_chain_update(MIB_DHCPS_SERVING_POOL_TBL, (void *)&dhcppoolentry, i);

		apply_add(CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}
	else if( strcmp( lastname, "X_CT-COM_Camera-MaxAddress" )==0 )
	{
		if( strlen(buf)==0 ) return ERR_9007;

		if( SerEn==0 ) return ERR_9001;

		if( inet_aton( buf, &addr )==0 ) return ERR_9007;

		for( i = 0; i < entryNum; i++)
		{
			if(!mib_chain_get(MIB_DHCPS_SERVING_POOL_TBL, i, (void *)&dhcppoolentry))
				continue;

			if(dhcppoolentry.poolname)
			{
				if(!strcmp(dhcppoolentry.poolname, "Camera"))
				{
					memcpy(dhcppoolentry.endaddr, &addr, IP_ADDR_LEN);
					break;
				}
			}
		}

		mib_chain_update(MIB_DHCPS_SERVING_POOL_TBL, (void *)&dhcppoolentry, i);

		apply_add(CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}
	else if( strcmp( lastname, "X_CT-COM_Computer-MinAddress" )==0 )
	{
		if( strlen(buf)==0 ) return ERR_9007;

		if( SerEn==0 ) return ERR_9001;

		if( inet_aton( buf, &addr )==0 ) return ERR_9007; //the ip address is error.

		for(i = 0; i < entryNum; i++)
		{
			if(!mib_chain_get(MIB_DHCPS_SERVING_POOL_TBL, i, (void *)&dhcppoolentry))
				continue;

			if(dhcppoolentry.poolname)
			{
				if(!strcmp(dhcppoolentry.poolname, "Computer"))
				{
					memcpy(dhcppoolentry.startaddr, &addr, IP_ADDR_LEN);
					break;
				}
			}
		}

		mib_chain_update(MIB_DHCPS_SERVING_POOL_TBL, (void *)&dhcppoolentry, i);

		apply_add(CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}
	else if( strcmp( lastname, "X_CT-COM_Computer-MaxAddress" )==0 )
	{
		if( strlen(buf)==0 ) return ERR_9007;

		if( SerEn==0 ) return ERR_9001;

		if( inet_aton( buf, &addr )==0 ) return ERR_9007;

		for(i = 0; i < entryNum; i++)
		{
			if(!mib_chain_get(MIB_DHCPS_SERVING_POOL_TBL, i, (void *)&dhcppoolentry))
				continue;

			if(dhcppoolentry.poolname)
			{
				if(!strcmp(dhcppoolentry.poolname, "Computer"))
				{
					memcpy(dhcppoolentry.endaddr, &addr, IP_ADDR_LEN);
					break;
				}
			}
		}

		mib_chain_update(MIB_DHCPS_SERVING_POOL_TBL, (void *)&dhcppoolentry, i);

		apply_add(CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}
	else{
		return ERR_9005;
	}

	return 0;
}

#endif	//_PRMT_X_CT_COM_DHCP_



/*****************************************************************************
 * InternetGatewayDevice.LANDevice.{i}.WLANConfiguration.{i}.
 *****************************************************************************/
#ifdef _PRMT_X_CT_COM_WLAN_
struct CWMP_OP tCTWLANConfEntityLeafOP = { getCTWLANConf, setCTWLANConf };
struct CWMP_PRMT tCTWLANConfEntityLeafInfo[] =
{
/*(name,				type,		flag,			op)*/
{"Standard",				eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTWLANConfEntityLeafOP},
{"WEPEncryptionLevel",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTWLANConfEntityLeafOP},
{"X_CT-COM_SSIDHide", 		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ, 	&tCTWLANConfEntityLeafOP},
{"X_CT-COM_RFBand", 		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ, 	&tCTWLANConfEntityLeafOP},
{"X_CT-COM_ChannelWidth",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ, 	&tCTWLANConfEntityLeafOP},
{"X_CT-COM_GuardInterval",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ, 	&tCTWLANConfEntityLeafOP},
{"X_CT-COM_RetryTimeout",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ, 	&tCTWLANConfEntityLeafOP},
{"X_CT-COM_Powerlevel",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tCTWLANConfEntityLeafOP},
{"X_CT-COM_PowerValue",		eCWMP_tUINT,	CWMP_READ, 				&tCTWLANConfEntityLeafOP},
{"X_CT-COM_APModuleEnable",	eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCTWLANConfEntityLeafOP},
{"X_CT-COM_WPSKeyWord",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tCTWLANConfEntityLeafOP},
};


int getCTWLANConf(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar=0;
	unsigned int vUint=0;
	unsigned int wlaninst=0;
#ifdef WLAN_MBSSID
	MIB_CE_MBSSIB_T Entry;
#endif
	unsigned char dot11n = 0;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	wlaninst = getWLANConfInstNum( name );
#ifdef WLAN_MBSSID
	if( wlaninst<1 || wlaninst>5 )	return ERR_9007;
	if (!mib_chain_get(MIB_MBSSIB_TBL, wlaninst-1, (void *)&Entry)) return ERR_9002;
#else
	if( wlaninst!=1 ) return ERR_9007;
#endif

	mib_get( MIB_WLAN_BAND, (void *)&vChar);
	if(vChar & BAND_11N)
		dot11n = 1;


	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Standard" )==0 )
	{
		mib_get( MIB_WLAN_BAND, (void *)&vChar);

		if( vChar==BAND_11B) //2.4 GHz (B)
			*data = strdup( "b" );
		else if( vChar==BAND_11G )//2.4 GHz (G)
			*data = strdup( "g" );
		else if( vChar==BAND_11BG)//2.4 GHz (B+G)
			*data = strdup( "g" );
		else if( vChar == (BAND_11BG | BAND_11N))
			*data = strdup( "b,g,n" );
		else if( vChar >= BAND_11N ) // N mode
			*data = strdup( "n" );
		else /*0, wifi_g==4, or wifi_bg==5?????*/
			*data = strdup( "" );
	}
	else if( strcmp( lastname, "WEPEncryptionLevel" )==0 )
	{
		//0:disable, 1:64, 2:128
#ifdef WLAN_MBSSID
		if(wlaninst != 1)
			vChar = Entry.wep;
		else
#endif
			mib_get( MIB_WLAN_WEP, (void *)&vChar);

		if(vChar == 0)
			*data = strdup( "Disabled" );
		else if (vChar == 1)
			*data = strdup( "40-bit" );
		else
			*data = strdup( "104-bit" );
	}
	else if( strcmp( lastname, "X_CT-COM_SSIDHide" )==0 )
	{
#ifdef WLAN_MBSSID
	    if(wlaninst!=1)
			vChar = Entry.hidessid;
	    else
#endif
			mib_get( MIB_WLAN_HIDDEN_SSID, (void *)&vChar);

		*data = booldup( (vChar!=0) );
	}
	else if( strcmp( lastname, "X_CT-COM_RFBand" )==0 )
	{
#ifdef CONFIG_RTL_92D_SUPPORT
		if ( mib_get(MIB_WLAN_PHY_BAND_SELECT, (void *)&vChar) == 0)
			return ERR_9002;

		// 0: 2.4G, 1: 5G
		if(vChar == PHYBAND_5G)
			*data = uintdup(1);
		else
#endif
			*data = uintdup(0);
	}
	else if( strcmp( lastname, "X_CT-COM_ChannelWidth" )==0 )
	{
		unsigned char width;

		mib_get(MIB_WLAN_CHANNEL_WIDTH, (void *)&width);
		mib_get(MIB_WLAN_11N_COEXIST, (void *)&vChar);

		// 0: 20 MHz, 1: 40 MHz, 2: 20/40 MHz
		if(width == 1 && vChar == 1)
		{
			width = 2;
		}

		*data = uintdup(width);
	}
	else if( strcmp( lastname, "X_CT-COM_GuardInterval" )==0 )
	{
		if(dot11n)
		{
			mib_get(MIB_WLAN_SHORTGI_ENABLED, &vChar);
			if(vChar)
				*data = uintdup(0);	// 400ns
			else
				*data = uintdup(1);	// 800ns
		}
		else
			*data = uintdup(1); // 800ns

	}
	else if( strcmp( lastname, "X_CT-COM_RetryTimeout" )==0 )
	{
		*data = uintdup(0);
	}
	else if( strcmp( lastname, "X_CT-COM_Powerlevel" )==0 )
	{	//MIB_TX_POWER=> 0: 100%, 1: 80%, 2: 50%
		//				 3: 25%,  4: 10%
		mib_get( MIB_TX_POWER, (void *)&vChar);

		if(vChar < 5)
			vUint = vChar + 1;
		else
			return ERR_9002;

		*data = uintdup( vUint );
	}
	else if( strcmp( lastname, "X_CT-COM_PowerValue" )==0 )
	{	//mW-to-dBm Power Conversion =>  dBm = 10*log(mW)
		mib_get( MIB_TX_POWER, (void *)&vChar);

		switch(vChar)
		{
		case 0:
			vUint=18;/*100%*/
			break;
		case 1:
			vUint=17;/*80%*/
			break;
		case 2:
			vUint=15;/*50%*/
			break;
		case 3:
			vUint=12;/*25%*/
			break;
		case 4:
			vUint=8;/*10%*/
			break;
		default:
			return ERR_9002;
		}

		*data = uintdup( vUint );
	}
	else if(strcmp(lastname,"X_CT-COM_APModuleEnable")==0){
		mib_get( MIB_WLAN_DISABLED, (void *)&vChar);
		*data = booldup( (vChar==0) );
	}
	else if( strcmp( lastname, "X_CT-COM_WPSKeyWord" )==0 )
	{
		*data = uintdup(128);
	}
	else{
		return ERR_9005;
	}

	return 0;
}

int setCTWLANConf(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned char	vChar=0;
	unsigned int wlaninst=0;
#ifdef WLAN_MBSSID
	MIB_CE_MBSSIB_T Entry;
#endif

	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;
	if( data==NULL ) return ERR_9007;


	wlaninst = getWLANConfInstNum( name );
#ifdef WLAN_MBSSID
	if( wlaninst<1 || wlaninst>5 )	return ERR_9007;
	if (!mib_chain_get(MIB_MBSSIB_TBL, wlaninst-1, (void *)&Entry)) return ERR_9002;
#else
	if( wlaninst!=1 ) return ERR_9007;
#endif

#ifdef E8B_NEW_DIAGNOSE
	writeSSIDFile("ÐÞ¸Ä", wlaninst);
#endif

	if( strcmp( lastname, "Standard" )==0 )
	{
		char *std = data;
		vChar = 0;

		if(strcmp(std,"a")== 0)
			return ERR_9001;
		else if(strcmp(std,"b")== 0)
			vChar = BAND_11B;
		else if(strcmp(std,"g")== 0)
			vChar = BAND_11G;
		else if(strcmp(std,"n")== 0)
			vChar = BAND_11N;
		else if(strcmp(std,"b,g,n")== 0)
			vChar = (BAND_11BG | BAND_11N);
		else
			return ERR_9007;

		mib_set( MIB_WLAN_BAND, (void *)&vChar);

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "WEPEncryptionLevel" )==0 )
	{
		if( strlen(buf)==0 ) return ERR_9007;
		if( strcmp( buf, "Disabled" )==0 )
			vChar = WEP_DISABLED;
		else if( strcmp( buf, "40-bit" )==0 )
			vChar = WEP64;
		else if( strcmp( buf, "104-bit" )==0 )
			vChar = WEP128;
		else return ERR_9007;
#ifdef WLAN_MBSSID
		if(wlaninst != 1){
			Entry.wep = vChar;
			mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
		}
		else
#endif
		mib_set( MIB_WLAN_WEP, (void *)&vChar);

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "X_CT-COM_SSIDHide" )==0 )
	{
		int *i = data;

		vChar = (*i==0)?0:1;
#ifdef WLAN_MBSSID
		if( wlaninst!=1 )
		{
			Entry.hidessid = vChar;
			mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
		}else
#endif
			mib_set(MIB_WLAN_HIDDEN_SSID, (void *)&vChar);

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "X_CT-COM_RFBand" )==0 )
	{
#ifdef CONFIG_RTL_92D_SUPPORT
		int *i = data;

		vChar = *i;

		if(vChar != 0 && vChar != 1)
			return ERR_9007;

		mib_set(MIB_WLAN_PHY_BAND_SELECT, (void *)&vChar);

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		// only 2.4 GHz
		return ERR_9001;
#endif
	}
	else if( strcmp( lastname, "X_CT-COM_ChannelWidth" )==0 )
	{
		int *i = data;
		unsigned char width;

		if( *i < 0 && *i > 2) return ERR_9007;

		// 0: 20 MHz, 1: 40 MHz, 2: 20/40 MHz
		if(*i == 2)
		{
			vChar = 1;
			width = 1;
		}
		else
		{
			vChar = 0;
			width = *i;
		}

		mib_set(MIB_WLAN_CHANNEL_WIDTH, (void *)&width);
		mib_set(MIB_WLAN_11N_COEXIST, (void *)&vChar);

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "X_CT-COM_GuardInterval" )==0 )
	{
		unsigned int *i = data;
		unsigned char vChar;

		if( *i == 0)	//400ns
			vChar = 1;
		else if(*i == 1)
			vChar = 0;
		else
			return ERR_9007;

		mib_set(MIB_WLAN_SHORTGI_ENABLED, &vChar);

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "X_CT-COM_RetryTimeout" )==0 )
	{
		return ERR_9001;
	}
	else if( strcmp( lastname, "X_CT-COM_Powerlevel" )==0 )
	{
		unsigned int *i = data;

		if( *i<0 || *i>5 ) return ERR_9007;

		if((*i) >= 1 && (*i) <= 5)
			vChar = (*i) - 1;
		else
			return ERR_9001;

		mib_set(MIB_TX_POWER, (void *)&vChar);

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if(strcmp(lastname,"X_CT-COM_APModuleEnable")==0){
		int *i = data;

		vChar = (*i==0)?1:0;
		mib_set(MIB_WLAN_DISABLED, (void *)&vChar);

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "X_CT-COM_WPSKeyWord" )==0 )
	{
		return ERR_9001;
	}
	else{
		return ERR_9005;
	}

	return 0;
}

#endif //_PRMT_X_CT_COM_WLAN_







/*************************************************************************************************************************/
/***********_PRMT_X_CT_COM_VLAN_BOUND_****************************************************************************/
/*************************************************************************************************************************/
#ifdef _PRMT_X_CT_COM_VLAN_BOUND_
/***** InternetGatewayDevice.LANDevice.{i}.LANEthernetInterfaceConfig.{i}. */
/***** InternetGatewayDevice.LANDevice.{i}.WLANConfiguration.{i}. */
/***** InternetGatewayDevice.LANDevice.{i}.LANUSBInterfaceConfig.{i}. */

struct CWMP_OP tCTVlanBoundingOP = { getCTVlanBounding, setCTVlanBounding };
struct CWMP_PRMT tCTVlanBoundingLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"X_CT-COM_Mode",	eCWMP_tUINT,		CWMP_READ|CWMP_WRITE,	&tCTVlanBoundingOP},
{"X_CT-COM_VLAN",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tCTVlanBoundingOP},
};

#define LANIF_NUM	9
struct vlan_pair {
	unsigned short vid_a;
	unsigned short vid_b;
};
int getCTVlanBounding(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	int if_num = 0, num = -1;
	int total, i;
	MIB_CE_PORT_BINDING_T pbEntry;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	if_num = getInstNum(name, "LANEthernetInterfaceConfig");
	if(if_num >= 1 && if_num <= SW_LAN_PORT_NUM)
		num = if_num - 1;

	if_num = getInstNum(name, "WLANConfiguration");
	if(if_num >= 1 && if_num <= WLAN_IF_NUM)
		num = if_num + 3;

	if_num = getInstNum(name, "LANUSBInterfaceConfig");
	if(if_num == 1)
		num = LANIF_NUM - 1;

	if(num == -1)
		return ERR_9005;

	total = mib_chain_total(MIB_PORT_BINDING_TBL);
	if (total == 0) {
		memset(&pbEntry, 0, sizeof(MIB_CE_PORT_BINDING_T));
		for (i=0; i<LANIF_NUM; i++) {
			mib_chain_add(MIB_PORT_BINDING_TBL, (void *)&pbEntry);
		}
	}

	mib_chain_get(MIB_PORT_BINDING_TBL, num, (void *)&pbEntry);

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "X_CT-COM_Mode" )==0 )
	{
		*data = uintdup(pbEntry.pb_mode);
	}
	else if( strcmp( lastname, "X_CT-COM_VLAN" )==0 )
	{
		char vlan_str[40] = {0};
		struct vlan_pair *vid_pair = (struct vlan_pair *)&pbEntry.pb_vlan0_a;

		for (i = 0; i<4; i++) {
			if (vid_pair[i].vid_a) {
				if (i == 0)
					sprintf(vlan_str, "%d/%d", vid_pair[i].vid_a, vid_pair[i].vid_b);
				else
					sprintf(vlan_str, "%s,%d/%d", vlan_str, vid_pair[i].vid_a, vid_pair[i].vid_b);
			}
		}

		*data = strdup(vlan_str);
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int setCTVlanBounding(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;
	int if_num = 0, num = -1;
	int total, i;
	MIB_CE_PORT_BINDING_T pbEntry;

	if (name == NULL || entity == NULL)
		return -1;

	if (data == NULL)
		return ERR_9007;

	if_num = getInstNum(name, "LANEthernetInterfaceConfig");
	if(if_num >= 1 && if_num <= SW_LAN_PORT_NUM)
		num = if_num - 1;

	if_num = getInstNum(name, "WLANConfiguration");
	if(if_num >= 1 && if_num <= WLAN_IF_NUM)
		num = if_num + 3;

	if_num = getInstNum(name, "LANUSBInterfaceConfig");
	if(if_num == 1)
		num = LANIF_NUM - 1;

	if(num == -1)
		return ERR_9005;

	total = mib_chain_total(MIB_PORT_BINDING_TBL);
	if (total == 0) {
		memset(&pbEntry, 0, sizeof(MIB_CE_PORT_BINDING_T));
		for (i=0; i<LANIF_NUM; i++) {
			mib_chain_add(MIB_PORT_BINDING_TBL, (void *)&pbEntry);
		}
	}

	mib_chain_get(MIB_PORT_BINDING_TBL, num, (void *)&pbEntry);

	if (entity->info->type != type)
		return ERR_9006;

	if( strcmp( lastname, "X_CT-COM_Mode" )==0 )
	{
		unsigned int *uint = data;
		unsigned char mode = *uint;
		unsigned char org_mode;

		if(mode != 0 && mode != 1)
			return ERR_9007;

		org_mode = pbEntry.pb_mode;
		pbEntry.pb_mode = mode;

		mib_chain_update(MIB_PORT_BINDING_TBL, (void *)&pbEntry, num);

		// sync with port-based mapping
		if (pbEntry.pb_mode!=0 && org_mode != pbEntry.pb_mode)
			sync_itfGroup(num);

		apply_add( CWMP_PRI_N, apply_PortMapping, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "X_CT-COM_VLAN" )==0 )
	{
		char *buf = data;
		int ret, cnt=0;
		struct vlan_pair *vid_pair = (struct vlan_pair *)&pbEntry.pb_vlan0_a;

		// reset all vlan pairs to 0.
		memset(vid_pair, 0, sizeof(struct vlan_pair) * 4);

		ret = sscanf(buf, "%hu/%hu,%hu/%hu,%hu/%hu,%hu/%hu", &vid_pair[0].vid_a, &vid_pair[0].vid_b
				, &vid_pair[1].vid_a, &vid_pair[1].vid_b
				, &vid_pair[2].vid_a, &vid_pair[2].vid_b
				, &vid_pair[3].vid_a, &vid_pair[3].vid_b);

		if(ret % 2)
			return ERR_9007;

		mib_chain_update(MIB_PORT_BINDING_TBL, (void *)&pbEntry, num);
		apply_add( CWMP_PRI_N, apply_PortMapping, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else {
		return ERR_9005;
	}

	return 0;
}
#endif

/*************************************************************************************************************************/
/***********END _PRMT_X_CT_COM_VLAN_BOUND_****************************************************************************/
/*************************************************************************************************************************/



/*************************************************************************************************************************/
/*********** _PRMT_X_CT_COM_LBD_****************************************************************************/
/*************************************************************************************************************************/
#ifdef _PRMT_X_CT_COM_LBD_
static int parse_vlans(char *str, MIB_CE_LBD_VLAN_Tp vlans)
{
	int got = 0;
	int vlan;
	char *start = str, *saveptr, *tok= NULL, *end;
	int i;

	tok = strtok_r(str, " ,", &saveptr);
	while(tok && got < MAX_LBD_VLANS)
	{
		if(strcmp(tok, "untagged") == 0)
			vlan = 0;
		else
		{
			vlan = strtol(tok, &end, 10);
			if(vlan < 0 || vlan > 4094 || end[0] != '\0')
			{
				CWMPDBP0("%s is invalid vlan value\n", tok);
				return ERR_9007;
			}
		}

		for(i = 0 ; i < got ; i++)
		{
			if(vlans[i].vid == vlan)
				break;
		}
		if(i == got)
			vlans[got++].vid = vlan;

		tok = strtok_r(NULL, " ,", &saveptr);
	}
	if(tok)
	{
		CWMPDBP0("Number of VLAN values is limited to 100.\n");
		return ERR_9007;
	}

	return got;
}


struct CWMP_OP tCT_LBDLeafOP = { getCT_LBD, setCT_LBD };

struct CWMP_PRMT tCT_LBDLeafInfo[] = {
/*(name,		type,		flag,			op)*/
{"LoopbackEnable",	eCWMP_tBOOLEAN,	CWMP_WRITE | CWMP_READ,	&tCT_LBDLeafOP},
{"LoopExistPeriod",	eCWMP_tUINT,		CWMP_WRITE | CWMP_READ,	&tCT_LBDLeafOP},
{"LoopCancelPeriod",	eCWMP_tUINT,		CWMP_WRITE | CWMP_READ,	&tCT_LBDLeafOP},
{"VlanTag",			eCWMP_tSTRING,	CWMP_WRITE | CWMP_READ,	&tCT_LBDLeafOP},
{"EthernetType",		eCWMP_tSTRING,	CWMP_WRITE | CWMP_READ,	&tCT_LBDLeafOP},
};

enum eCT_LBDLeaf
{
	eCT_LoopbackEnable,
	eCT_LoopExistPeriod,
	eCT_LoopCancelPeriod,
	eCT_VlanTag,
	eCT_EthernetType,
};

struct CWMP_LEAF tCT_LBDLeaf[] = {
{ &tCT_LBDLeafInfo[eCT_LoopbackEnable] },
{ &tCT_LBDLeafInfo[eCT_LoopExistPeriod] },
{ &tCT_LBDLeafInfo[eCT_LoopCancelPeriod] },
{ &tCT_LBDLeafInfo[eCT_VlanTag] },
{ &tCT_LBDLeafInfo[eCT_EthernetType] },
{ NULL },
};

int getCT_LBD(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;

	if (name == NULL || entity == NULL || type == NULL || data == NULL)
		return -1;

	*type = entity->info->type;
	*data = NULL;

	if (strcmp(lastname, "LoopbackEnable") == 0)
	{
		unsigned char enable;
		int i;

		mib_get(MIB_LBD_ENABLE, &enable);

		i = enable;
		*data = booldup(i);
	}
	else if (strcmp(lastname, "LoopExistPeriod") == 0)
	{
		unsigned int period;

		mib_get(MIB_LBD_EXIST_PERIOD, &period);
		*data = uintdup(period);
	}
	else if (strcmp(lastname, "LoopCancelPeriod") == 0)
	{
		unsigned int period;

		mib_get(MIB_LBD_CANCEL_PERIOD, &period);
		*data = uintdup(period);
	}
	else if (strcmp(lastname, "VlanTag") == 0)
	{
		int total = mib_chain_total(MIB_LBD_VLAN_TBL);
		MIB_CE_LBD_VLAN_T entry;
		char buf[512] = {0};
		char *target = buf;
		int i;

		for(i = 0 ; i < total ; i++)
		{
			if(mib_chain_get(MIB_LBD_VLAN_TBL, i, &entry) < 0)
				continue;

			if(i > 0)
				target += sprintf(buf, ",");

			if(entry.vid == 0)
				target += sprintf(buf, "untagged");
			else
				target += sprintf(buf, "%hu", entry.vid);
		}

		*data = strdup(buf);
	}
	else if (strcmp(lastname, "EthernetType") == 0)
	{
		char buf[256];
		unsigned short type = 0;

		mib_get(MIB_LBD_ETHER_TYPE, &type);
		sprintf(buf, "0x%04X", type);

		*data = strdup(buf);
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int setCT_LBD(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;

	if (name == NULL || entity == NULL || data == NULL)
		return -1;

	if (entity->info->type != type)
		return ERR_9006;

	if (strcmp(lastname, "LoopbackEnable") == 0)
	{
		int *i = data;
		unsigned char enable = (*i == 0) ? 0 : 1;

		mib_set(MIB_LBD_ENABLE, &enable);

		apply_add(CWMP_PRI_N, apply_lbd, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}
	else if (strcmp(lastname, "LoopExistPeriod") == 0)
	{
		unsigned int *period = data;

		if(*period < 1 || *period > 60)
			return ERR_9007;

		mib_set(MIB_LBD_EXIST_PERIOD, period);

		apply_add(CWMP_PRI_N, apply_lbd, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}
	else if (strcmp(lastname, "LoopCancelPeriod") == 0)
	{
		unsigned int *period = data;

		if(*period < 10 || *period > 1800)
			return ERR_9007;

		mib_set(MIB_LBD_CANCEL_PERIOD, period);

		apply_add(CWMP_PRI_N, apply_lbd, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}
	else if (strcmp(lastname, "VlanTag") == 0)
	{
		char *buf = data;
		MIB_CE_LBD_VLAN_T vlans[MAX_LBD_VLANS] = {0};
		int size, i;

		if(strlen(buf) == 0)
			return ERR_9007;

		size = parse_vlans(buf, vlans);
		if(size < 0)
			return size;	// return error number

		mib_chain_clear(MIB_LBD_VLAN_TBL);
		for(i = 0 ; i < size ; i++)
			mib_chain_add(MIB_LBD_VLAN_TBL, &vlans[i]);

		apply_add(CWMP_PRI_N, apply_lbd, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}
	else if (strcmp(lastname, "EthernetType") == 0)
	{
		char *buf = data, *end;
		unsigned short type;

		if(strlen(buf) == 0)
			return ERR_9007;

		type = strtol(buf, &end, 16);
		if(type == 0 || end[0] != '\0')
		{
			CWMPDBP0("%s is invalid Ethernet type\n", buf);
			return ERR_9007;
		}

		mib_set(MIB_LBD_ETHER_TYPE, &type);
		apply_add(CWMP_PRI_N, apply_lbd, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}
	else {
		return ERR_9005;
	}

	return 0;
}


struct CWMP_OP tCT_EthLBDStatusLeafOP = { getCT_EthLBDStatus, NULL };

struct CWMP_PRMT tCT_EthLBDStatusLeafInfo[] = {
/*(name,		type,		flag,			op)*/
{"X_CT-COM_DetectionStatus",	eCWMP_tUINT,		CWMP_READ,	&tCT_EthLBDStatusLeafOP},
};

int getCT_EthLBDStatus(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;
	int ifnum = 0;

	if (name == NULL || entity == NULL || type == NULL || data == NULL)
		return -1;

	*type = entity->info->type;
	*data = NULL;

	ifnum = getInstNum(name, "LANEthernetInterfaceConfig");
	if(ifnum < 1 || ifnum > ELANVIF_NUM)
		return ERR_9005;

	if (strcmp(lastname, "X_CT-COM_DetectionStatus") == 0)
	{
		unsigned char status[4];

		mib_get(MIB_RS_LBD_PORT_STATUS, status);
		*data = uintdup(status[ifnum-1]);
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}


#endif
/*************************************************************************************************************************/
/***********END _PRMT_X_CT_COM_LBD_****************************************************************************/
/*************************************************************************************************************************/


