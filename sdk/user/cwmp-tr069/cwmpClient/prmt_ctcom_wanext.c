#include "cwmpc_utility.h"
#include "prmt_wancondevice.h"
#include "prmt_ctcom_wanext.h"

/***** Utilities ***************************************************************/
int convertFlag2ServiceList( unsigned short flag, char *buf )
{
	int buflen=0;

	if(buf==NULL) return -1;

	if( flag&X_CT_SRV_TR069 )	strcat( buf, "TR069," );


	if( flag&X_CT_SRV_INTERNET ) strcat( buf, "INTERNET," );

	if( flag&X_CT_SRV_VOICE) strcat( buf, "VOIP," );

	if( flag&X_CT_SRV_OTHER )	strcat( buf, "OTHER," );

	buflen = strlen(buf);
	if(buflen>0) buf[buflen-1]=0;
	return 0;
}

int convertServiceList2Flag( char *buf, unsigned short *flag )
{
	char *tok;
	if( (buf==NULL) || (flag==NULL) ) return -1;

	*flag = 0;
	tok = strtok( buf, ", \t\r\n" );
	while(tok)
	{
		if( strcmp( tok, "TR069" )==0 )
			*flag |= X_CT_SRV_TR069;
		else if( strcmp( tok, "INTERNET" )==0 )
			*flag |= X_CT_SRV_INTERNET;
		else if( strcmp( tok, "VOIP" )==0 )
			*flag |= X_CT_SRV_VOICE;
		else if( strcmp( tok, "OTHER" )==0 )
			*flag |= X_CT_SRV_OTHER;
		else{	//skip unknown toks or return error?
			*flag = 0;
			return -1;
		}
		tok = strtok( NULL, ", \t\r\n" );
	}

	return 0;
}

#ifdef NEW_PORTMAPPING
#ifdef CONFIG_E8B
char *sLANIF[4]={	"InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.1", //eth0_sw0
			"InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.2", //eth0_sw1
			"InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.3", //eth0_sw2
			"InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.4"  //eth0_sw3
		 };
#else
char *sLANIF[4]={	"InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.4", //eth0_sw0
			"InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.3", //eth0_sw1
			"InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.2", //eth0_sw2
			"InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.1"  //eth0_sw3
		 };
#endif
#ifdef WLAN_SUPPORT
char *sWLANIF[WLAN_IF_NUM]={
			"InternetGatewayDevice.LANDevice.1.WLANConfiguration.1" //wlan0
#ifdef WLAN_MBSSID
			, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.2" //wlan0-vap0
			, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.3" //wlan0-vap1
			, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.4" //wlan0-vap2
			, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.5" //wlan0-vap3
#endif //WLAN_MBSSID
		};

#endif //WLAN_SUPPORT
int getLanInterface(unsigned short itfGroup, char *infstr)
{
	int i, len;
	char *sLANIfAll[16];
	char sw_portnum = SW_PORT_NUM % 2? SW_PORT_NUM - 1: SW_PORT_NUM;

	if(infstr == NULL)
		return -1;

	for( i = 0; i < sw_portnum; i++ )
		sLANIfAll[i] = sLANIF[i];

	i = 0;
#ifdef WLAN_SUPPORT
	for( i = 0; i < WLAN_IF_NUM; i++ )
		sLANIfAll[sw_portnum+i] = sWLANIF[i];
#endif

	sLANIfAll[sw_portnum+i] = NULL;

	infstr[0] = 0;

	for( i = 0; sLANIfAll[i]; i++ )
	{
		if( itfGroup & (1<<i) )
			sprintf(infstr, "%s%s,", infstr, sLANIfAll[i]);
	}

	len = strlen(infstr);

	if(len)
		infstr[len-1] = 0; // Remove trailling comma.

	return len;
}

int setLanInterface(char *infstr, MIB_CE_ATM_VC_T *pEntry)
{
	int i, vcnum;
	char *sLANIfAll[16];
	char sw_portnum = SW_PORT_NUM % 2? SW_PORT_NUM - 1: SW_PORT_NUM;
	char *tok;
	unsigned short itfgroup = 0;
	MIB_CE_ATM_VC_T avc;

	if(infstr == NULL)
		return -1;

	for( i = 0; i < sw_portnum; i++ )
		sLANIfAll[i] = sLANIF[i];

	i = 0;
#ifdef WLAN_SUPPORT
	for( i = 0; i < WLAN_IF_NUM; i++ )
		sLANIfAll[sw_portnum+i] = sWLANIF[i];
#endif

	sLANIfAll[sw_portnum+i] = NULL;

	for( tok = strtok(infstr, ","); tok; tok = strtok(NULL, ",") )
	{
		for( i = 0; sLANIfAll[i]; i++ )
		{
			if(!strcmp(sLANIfAll[i], tok))
				itfgroup |= 1 << i;
		}
	}

	// Let 2 WAN can bind the same LAN ports to satisfy the test case 2.2.3
#if 0
	// Remove the same binding from other vc.
	vcnum = mib_chain_total(MIB_ATM_VC_TBL);
	for( i = 0; i < vcnum; i++ )
	{
		if(mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&avc) <= 0)
			return ERR_9002;

		avc.itfGroup &= ~itfgroup;
		mib_chain_update(MIB_ATM_VC_TBL, (void *)&avc, i);
	}
#endif

	pEntry->itfGroup  = itfgroup;
	return 1;
}
#endif //NEW_PORTMAPPING





/*****************************************************************************
 * InternetGatewayDevice.WANDevice.{i}.
 *****************************************************************************/
#ifdef _PRMT_X_CT_COM_WANEXT_
struct CWMP_OP tCTWANDevEntityLeafOP = { getCTWANDevEntity, NULL };
struct CWMP_PRMT tCTWANDevEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"X_CT-COM_WANIndex",	eCWMP_tSTRING,	CWMP_READ,		&tCTWANDevEntityLeafOP},
};

int getCTWANDevEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	MIB_CE_ATM_VC_T vc_entity;
	int numofentries, i, totalwancon = 0;
	unsigned int wandevnum;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;

	wandevnum = getWANDevInstNum(name);

	if(strcmp( lastname, "X_CT-COM_WANIndex" )==0)
	{
		numofentries = mib_chain_total(MIB_ATM_VC_TBL);
		char buf[1024] = {0};

		for( i = 0; i < numofentries; i++ )
		{
			if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void*)&vc_entity))
			{
				CWMPDBP(1, "Get mib chain table MIB_ATM_VC_TBL failed! (At index i=%d)", i);
				return ERR_9002;
			}

			if(isWANDevModeMatch(wandevnum, vc_entity.ifIndex))
			{
				MIB_CE_ATM_VC_T pre_entity;
				int j,isadded;
				isadded=0;
				for(j=0;j<i;j++)
				{
					if(!mib_chain_get(MIB_ATM_VC_TBL, j, (void*)&pre_entity))
					{
						CWMPDBP(1, "Get mib chain table MIB_ATM_VC_TBL failed! (At index j=%d)", j);
						return ERR_9002;
					}

					if(isWANDevModeMatch(wandevnum, pre_entity.ifIndex))
					{
						if(vc_entity.ConDevInstNum==pre_entity.ConDevInstNum)
						{
							isadded=1;
							break;
						}
					}
				}
				if(isadded==0)
				{
					unsigned int instnum1 = vc_entity.ConDevInstNum;
					unsigned int instnum2;
					char *type = "";
					char tmp[128] = {0};

					// do not append ',' if this is first wan
					if(buf[0] != '\0')
						strcat(buf, ",\"");
					else
						strcat(buf, "\"");

					if(vc_entity.cmode == CHANNEL_MODE_PPPOA || vc_entity.cmode == CHANNEL_MODE_PPPOE)
						instnum2 = vc_entity.ConPPPInstNum;
					else
						instnum2 = vc_entity.ConIPInstNum;

					sprintf(tmp, "%u.%u;", instnum1, instnum2);
					strcat(buf, tmp);

					switch(vc_entity.cmode)
					{
					case CHANNEL_MODE_BRIDGE:
						strcat(buf, "Bridged;");
						break;
					case CHANNEL_MODE_PPPOE:
						strcat(buf, "PPPoE_Routed;");
						break;
					case CHANNEL_MODE_RT1483:
					case CHANNEL_MODE_RT1577:
						strcat(buf, "STATIC_Routed");
						break;
					case CHANNEL_MODE_IPOE:
						if(vc_entity.ipDhcp)
							strcat(buf, "DHCP_Routed;");
						else
							strcat(buf, "STATIC_Routed;");
						break;
					default:
						break;
					}

					// VLAN or PVC number
					if(MEDIA_INDEX(vc_entity.ifIndex) == MEDIA_ATM)
						sprintf(tmp, "%u/%u;", vc_entity.vpi, vc_entity.vci);
					else if(vc_entity.vlan)
						sprintf(tmp, "%u;", vc_entity.vid);
					else
						sprintf(tmp, ";");	//How about no vlan & not ATM?

					strcat(buf, tmp);

					//service list
					strcpy(tmp, "");
					convertFlag2ServiceList(vc_entity.applicationtype, tmp);
					strcat(buf, tmp);

					if(buf[strlen(buf)] == ',')
						buf[strlen(buf) - 1] = '\"'; // let last ',' be '"'
					else
						buf[strlen(buf)] = '\"';	// no service list
				}
			}
		}

		*data = strdup(buf);
	}
	else{
		return ERR_9005;
	}

	return 0;
}
#endif	//_PRMT_X_CT_COM_WANEXT_




/*****************************************************************************
 * InternetGatewayDevice.WANDevice.{i}.WANConnectionDevice.{i}.WANPPPConnection.
 *****************************************************************************/
#ifdef _PRMT_X_CT_EXT_ENABLE_
struct CWMP_OP tCTWANPPPCONENTITYLeafOP = { getCTWANPPPCONENTITY, setCTWANPPPCONENTITY };
struct CWMP_PRMT tCTWANPPPCONENTITYLeafInfo[] =
{
#ifdef _PRMT_X_CT_COM_PPPOE_PROXY_
{"X_CT-COM_ProxyEnable",	eCWMP_tBOOLEAN, CWMP_WRITE|CWMP_READ,	&tCTWANPPPCONENTITYLeafOP},
{"X_CT-COM_MAXUser",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tCTWANPPPCONENTITYLeafOP},
#endif //_PRMT_X_CT_COM_PPPOE_PROXY_
#ifdef _PRMT_X_CT_COM_WANEXT_
{"X_CT-COM_LanInterface",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTWANPPPCONENTITYLeafOP},
{"X_CT-COM_ServiceList",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTWANPPPCONENTITYLeafOP},
{"X_CT-COM_LanInterface-DHCPEnable",	eCWMP_tBOOLEAN, CWMP_WRITE|CWMP_READ,	&tCTWANPPPCONENTITYLeafOP},
{"X_CT-COM_IPForwardList",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTWANPPPCONENTITYLeafOP},
#ifdef CONFIG_MCAST_VLAN
{"X_CT-COM_MulticastVlan", eCWMP_tINT, 		CWMP_WRITE|CWMP_READ,	&tCTWANPPPCONENTITYLeafOP},
#endif
#endif	//_PRMT_X_CT_COM_WANEXT_
#ifdef _PRMT_X_CT_COM_IPv6_
{"X_CT-COM_IPMode",			eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tCTWANPPPCONENTITYLeafOP},
{"X_CT-COM_IPv6ConnStatus",	eCWMP_tSTRING,	CWMP_READ,	&tCTWANPPPCONENTITYLeafOP},
{"X_CT-COM_IPv6IPAddress",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tCTWANPPPCONENTITYLeafOP},
{"X_CT-COM_IPv6IPAddressAlias", 	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tCTWANPPPCONENTITYLeafOP},
{"X_CT-COM_IPv6IPAddressOrigin",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tCTWANPPPCONENTITYLeafOP},
{"X_CT-COM_IPv6DNSServers", 		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tCTWANPPPCONENTITYLeafOP},
{"X_CT-COM_IPv6PrefixDelegationEnabled",	eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tCTWANPPPCONENTITYLeafOP},
{"X_CT-COM_IPv6PrefixAlias", 		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tCTWANPPPCONENTITYLeafOP},
{"X_CT-COM_IPv6PrefixOrigin", 		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tCTWANPPPCONENTITYLeafOP},
{"X_CT-COM_IPv6Prefix",	 		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tCTWANPPPCONENTITYLeafOP},
{"X_CT-COM_IPv6PrefixPltime",		eCWMP_tUINT,		CWMP_READ|CWMP_WRITE,	&tCTWANPPPCONENTITYLeafOP},
{"X_CT-COM_IPv6PrefixVltime",		eCWMP_tUINT,		CWMP_READ|CWMP_WRITE,	&tCTWANPPPCONENTITYLeafOP},
{"X_CT-COM_DefaultIPv6Gateway",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tCTWANPPPCONENTITYLeafOP},
{"X_CT-COM_IPv6DomainName",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tCTWANPPPCONENTITYLeafOP},
{"X_CT-COM_Dslite_Enable",			eCWMP_tUINT,		CWMP_READ|CWMP_WRITE,	&tCTWANPPPCONENTITYLeafOP},
{"X_CT-COM_AftrMode",				eCWMP_tINT, 		CWMP_READ|CWMP_WRITE,	&tCTWANPPPCONENTITYLeafOP},
{"X_CT-COM_Aftr",					eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tCTWANPPPCONENTITYLeafOP},
#endif
};

static get_dns6_by_wan(char *buf, int len, MIB_CE_ATM_VC_T *pEntry)
{
	if ( (pEntry->Ipv6Dhcp == 1) || ((pEntry->Ipv6DhcpRequest & 0x2) == 0x2)
			|| pEntry->AddrMode == IPV6_WAN_STATIC)
	{
		FILE* infdns;
		char file[64] = {0};
		char line[128] = {0};
		char ifname[IFNAMSIZ] = {0};

		ifGetName(pEntry->ifIndex,ifname,sizeof(ifname));

		snprintf(file, 64, "%s.%s", (char *)DNS6_RESOLV, ifname);

		infdns=fopen(file,"r");
		if(infdns)
		{
			int cnt = 0;

			while(fgets(line,sizeof(line),infdns) != NULL)
			{
				char *new_line = NULL;

				new_line = strrchr(line, '\n');
				if(new_line)
					*new_line = '\0';

				if((strlen(line)==0))
					continue;

				if(cnt != 0)
					strncat(buf, ",", len);

				strncat(buf, line, len);
				cnt++;
			}
			fclose(infdns);
		}
	}
}

#define LEASE_FNAME_FMT "/var/dhcpcV6%s.leases"

int getCTWANPPPCONENTITY(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned int	chainidx;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;
	char	ifname[16];
	char	buf[512]="";

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	pEntry = &vc_entity;
	if( getATMVCEntry(name, pEntry, &chainidx) < 0 )
		return ERR_9002;

	if(ifGetName( pEntry->ifIndex, ifname, 16 )==0) return ERR_9002;

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "X_CT-COM_ProxyEnable" )==0 )
	{
#ifdef CONFIG_USER_PPPOE_PROXY
		if(pEntry->PPPoEProxyEnable)
			*data = booldup(1);
		else
#endif
			*data = booldup(0);
	}
	else if( strcmp( lastname, "X_CT-COM_MAXUser" )==0 )
	{
#ifdef CONFIG_USER_PPPOE_PROXY
		*data = uintdup( pEntry->PPPoEProxyMaxUser );
#else
		*data = uintdup(0);
#endif
	}
	else if( strcmp( lastname, "X_CT-COM_LanInterface" )==0 )
	{
#ifdef NEW_PORTMAPPING
		char tmp[1024];
		getLanInterface(pEntry->itfGroup, tmp);
		*data = strdup(tmp);
#else
		*data = strdup( "" );
#endif
	}
	else if( strcmp( lastname, "X_CT-COM_ServiceList" )==0 )
	{
		convertFlag2ServiceList( pEntry->ServiceList, buf );
		*data = strdup( buf );
	}
	else if( strcmp( lastname, "X_CT-COM_LanInterface-DHCPEnable" )==0 )
	{
		if(pEntry->disableLanDhcp)
			*data = booldup(0);
		else
			*data = booldup(1);
	}
	else if( strcmp( lastname, "X_CT-COM_IPForwardList" )==0 )
	{
		//We don't support IP Forward mode
		*data = strdup("");
	}
	else if( strcmp( lastname, "X_CT-COM_IPMode" ) == 0 )
	{
#ifdef CONFIG_IPV6
		switch(pEntry->IpProtocol)
		{
			case IPVER_IPV4:
			case IPVER_IPV6:
			case IPVER_IPV4_IPV6:
				*data = uintdup(pEntry->IpProtocol);
				break;
			default:
				return ERR_9007;
				break;
		}
#else
		*data = uintdup(IPVER_IPV4); //IPv4
#endif
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6ConnStatus" ) == 0 )
	{
		int flags;

		if(pEntry->IpProtocol == IPVER_IPV4)	//IPv4 only
			*data = strdup("Unconfigured");
		else if (pEntry->enable == 0 || getInFlags(ifname, &flags) == 0
				|| (flags & IFF_RUNNING) == 0) {
			*data = strdup("Disconnected");
		} else {
			*data = strdup("Connected");
		}
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6IPAddress" )==0 )
	{
		if(pEntry->IpProtocol & IPVER_IPV6)
		{
			struct ipv6_ifaddr addr;
			if(pEntry->AddrMode & IPV6_WAN_STATIC)
				inet_ntop(AF_INET6, pEntry->Ipv6Addr, buf, sizeof(buf));
			else if(getifip6(ifname, IPV6_ADDR_UNICAST, &addr, 1) == 1)
				inet_ntop(AF_INET6, &addr.addr, buf, sizeof(buf));
		}

		*data = strdup(buf);
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6IPAddressAlias" )==0 )
	{
		*data = strdup(pEntry->Ipv6AddrAlias);
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6IPAddressOrigin" )==0 )
	{
		if(pEntry->IpProtocol & IPVER_IPV6)
		{
			if(pEntry->AddrMode & IPV6_WAN_AUTO)
				*data = strdup("AutoConfigured");
			else if(pEntry->AddrMode & IPV6_WAN_STATIC)
				*data = strdup("Static");
			else if(pEntry->AddrMode & IPV6_WAN_DHCP)
				*data = strdup("DHCPv6");
			else
				*data = strdup("None");
		}
		else
			*data = strdup("None");
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6DNSServers" )==0 )
	{
		unsigned char dnsMode = 0;
		char str_server[40] = {0};

		get_dns6_by_wan(buf, sizeof(buf), pEntry);
		*data = strdup(buf);
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6PrefixDelegationEnabled" )==0 )
	{
		*data = booldup((pEntry->Ipv6DhcpRequest & 0x2) ? 1 : 0);
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6PrefixAlias" )==0 )
	{
		*data = strdup(pEntry->Ipv6PrefixAlias);
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6PrefixOrigin" )==0 )
	{
		switch(pEntry->IPv6PrefixOrigin)
		{
		case 1:
			*data = strdup("PrefixDelegation");
			break;
		case 2:
			*data = strdup("Static");
			break;
		case 3:
			*data = strdup("PPPoE");
			break;
		default:
			*data = strdup("None");
			break;
		}
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6Prefix" )==0 )
	{
		const char *dst = NULL;

		if(pEntry->AddrMode & IPV6_WAN_STATIC)
		{
			struct in6_addr prefix = {0};

			ip6toPrefix(pEntry->Ipv6Addr, pEntry->Ipv6AddrPrefixLen, &prefix);
			dst = inet_ntop(AF_INET6, &prefix, buf, sizeof(buf));

			if(dst)
			{
				sprintf(buf, "%s/%d", dst, pEntry->Ipv6AddrPrefixLen);
				*data = strdup(buf);
			}
			else
				*data = strdup("");
		}
		else
		{
			DLG_INFO_T dlg_info;
			char fname[256] = {0};
			int ret;

			snprintf(fname, 256, LEASE_FNAME_FMT, ifname);
			ret = getLeasesInfo(fname, &dlg_info);
			dst = inet_ntop(AF_INET6, dlg_info.prefixIP, buf, sizeof(buf));

			if(ret && dst)
			{
				sprintf(buf, "%s/%d", dst, dlg_info.prefixLen);
				*data = strdup(buf);
			}
			else
				*data = strdup("");
		}
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6PrefixPltime" )==0 )
	{
		if(pEntry->AddrMode & IPV6_WAN_STATIC)
		{
			*data = uintdup(pEntry->IPv6PrefixPltime);
		}
		else
		{
			DLG_INFO_T dlg_info;
			char fname[256] = {0};
			int ret;

			snprintf(fname, 256, LEASE_FNAME_FMT, ifname);
			ret = getLeasesInfo(fname, &dlg_info);
			*data = intdup(ret ? dlg_info.PLTime : 0);
		}
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6PrefixVltime" )==0 )
	{
		if(pEntry->AddrMode & IPV6_WAN_STATIC)
		{
			*data = uintdup(pEntry->IPv6PrefixVltime);
		}
		else
		{
			DLG_INFO_T dlg_info;
			char fname[256] = {0};
			int ret;

			snprintf(fname, 256, LEASE_FNAME_FMT, ifname);
			ret = getLeasesInfo(fname, &dlg_info);
			*data = intdup(ret ? dlg_info.MLTime : 0);
		}
	}
	else if( strcmp( lastname, "X_CT-COM_DefaultIPv6Gateway" )==0 )
	{
		if(pEntry->IpProtocol & IPVER_IPV6)
		{
			if(pEntry->AddrMode & IPV6_WAN_STATIC)
				inet_ntop(AF_INET6, pEntry->RemoteIpv6Addr, buf, sizeof(buf));

			//TODO: get IPv6 gateway
		}

		*data = strdup(buf);
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6DomainName" )==0 )
	{
		*data = strdup( pEntry->IPv6DomainName);
	}
	else if( strcmp( lastname, "X_CT-COM_Dslite_Enable" )==0 )
	{
		int enable = pEntry->dslite_enable;

		*data = booldup(enable);
	}
	else if( strcmp( lastname, "X_CT-COM_AftrMode" )==0 )
	{
		*data = intdup(pEntry->dslite_aftr_mode);
	}
	else if( strcmp( lastname, "X_CT-COM_Aftr" )==0 )
	{
		*data = strdup(pEntry->dslite_aftr_hostname);
	}
#ifdef CONFIG_MCAST_VLAN
	else if( strcmp( lastname, "X_CT-COM_MulticastVlan" )==0 )
	{
		int vid = pEntry->mVid;

		if(vid == 0)
			*data = intdup(-1);
		else
			*data = intdup(vid);
	}
#endif
	else{
		return ERR_9005;
	}

	return 0;
}

int setCTWANPPPCONENTITY(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned int	chainidx;
	MIB_CE_ATM_VC_T *pEntry, vc_entity;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	pEntry = &vc_entity;
	if( getATMVCEntry(name, pEntry, &chainidx) < 0 )
		return ERR_9002;

	gWanBitMap |= (1U << chainidx);

	if( strcmp( lastname, "X_CT-COM_ProxyEnable" )==0 )
	{
#ifdef CONFIG_USER_PPPOE_PROXY
		int *i = data;

		if(i==NULL) return ERR_9007;
		pEntry->PPPoEProxyEnable = (*i==0) ? 0:1;
		if(pEntry->PPPoEProxyEnable==1)
			pEntry->dgw=0;
#ifndef CONFIG_RTK_RG_INIT
		else{
			if(pEntry->cmode!=CHANNEL_MODE_BRIDGE && (pEntry->applicationtype&X_CT_SRV_INTERNET))
				pEntry->dgw=1;
		}
#endif
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
		return CWMP_NEED_RESTART_WAN;
#else
		return ERR_9005;
#endif
	}
	else if( strcmp( lastname, "X_CT-COM_MAXUser" )==0 )
	{
#ifdef CONFIG_USER_PPPOE_PROXY
		unsigned int *maxuser = data;

		if(maxuser==NULL) return ERR_9007;
		/* Tsai: the limitations from pppnumleft of
		 * user/boa/src-e8b-new/LINUX/fmnet.c:initPageAdsl() */
		if (*maxuser > 5) {
			return ERR_9007;
		}
		pEntry->PPPoEProxyMaxUser =  *maxuser;
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
		return CWMP_NEED_RESTART_WAN;
#else
		return ERR_9005;
#endif
	}
	else if( strcmp( lastname, "X_CT-COM_LanInterface" )==0 )
	{
#ifdef NEW_PORTMAPPING
		int ret_err;
		if( buf==NULL ) return ERR_9007;
		ret_err = setLanInterface( buf, pEntry );
		if( ret_err < 0 ) return ret_err;
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );

		if (pEntry->applicationtype&X_CT_SRV_OTHER)
			return CWMP_NEED_RESTART_WAN;
		else{
			apply_add( CWMP_PRI_H, apply_PortMapping, CWMP_RESTART, 0, NULL, 0 );
			return CWMP_APPLIED;
		}
#else
		return ERR_9001;
#endif
	}
	else if( strcmp( lastname, "X_CT-COM_ServiceList" )==0 )
	{
		unsigned short flag=0;
		if( buf==NULL ) return ERR_9007;
		if( convertServiceList2Flag( buf, &flag )<0 ) return ERR_9007;
		pEntry->ServiceList=flag;
		pEntry->applicationtype=flag;
#if defined(CTC_WAN_NAME)&&defined(CONFIG_E8B)
		if(((flag&X_CT_SRV_TR069)!=0)&&((flag&X_CT_SRV_INTERNET)!=0)){
			if(pEntry->cmode!=CHANNEL_MODE_BRIDGE){
				pEntry->napt=1;
#ifndef CONFIG_RTK_RG_INIT
				pEntry->dgw=1;
#endif
			}
		}
		else if((flag&X_CT_SRV_INTERNET)!=0){
			if(pEntry->cmode!=CHANNEL_MODE_BRIDGE){
				pEntry->napt=1;
#ifndef CONFIG_RTK_RG_INIT
				pEntry->dgw=1;
#endif
			}
		}
		else if((flag&X_CT_SRV_TR069)!=0){
			pEntry->napt=0;
			pEntry->dgw=0;
		}
		else if((flag&X_CT_SRV_OTHER)!=0){
			pEntry->napt=0;
			pEntry->dgw=0;
		}
		else{
			pEntry->napt=0;
			pEntry->dgw=0;
		}

		printf("\nflag=%d,applicationtype=%d\n",flag,pEntry->applicationtype);
#endif
#ifdef E8B_NEW_DIAGNOSE
		FILE * fp1;
		int fileid;
		char tmpbuf[20];
		pvc_status_entry entry;
		memset(&entry, 0, sizeof(pvc_status_entry));
		entry.vpi = pEntry->vpi;
		entry.vci = pEntry->vci;
		fileid = findmatchpvcfileid(&entry);
		if (fileid > 0) {
			sprintf(tmpbuf, "%s%d", PVC_FILE, fileid);
			fp1 = fopen(tmpbuf, "wb+");
			if (fp1) {
				setWanName(entry.servertype, pEntry->applicationtype);
				fwrite(&entry, sizeof(pvc_status_entry), 1, fp1);
				fclose(fp1);
			}
		}
#endif
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "X_CT-COM_LanInterface-DHCPEnable" )==0 )
	{
		int *i = data;

		if(i==NULL) return ERR_9007;

		pEntry->disableLanDhcp = (*i==0) ? 1:0;

		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "X_CT-COM_IPForwardList" )==0 )
	{
		//We don't support IP Forward mode
		return ERR_9007;
	}
	else if( strcmp( lastname, "X_CT-COM_IPMode" ) == 0 )
	{
		int *i = data;

		if(i==NULL) return ERR_9007;
#ifdef CONFIG_IPV6
		switch(*i)
		{
			case 1:
			case 2:
			case 3:
				pEntry->IpProtocol = *i;
				break;
			default:
				return ERR_9007;
				break;
		}
		if(pEntry->IpProtocol & IPVER_IPV6)
			pEntry->Ipv6DhcpRequest |= 2;

		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
		return CWMP_NEED_RESTART_WAN;
#else
		return ERR_9003;
#endif
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6IPAddress" )==0 )
	{
		char *buf = data;

		if(pEntry->IpProtocol & IPVER_IPV6)
		{
			if(pEntry->AddrMode & IPV6_WAN_STATIC)
			{
				if(inet_pton(AF_INET6, buf, pEntry->Ipv6Addr) != 1)
					return ERR_9007;

				mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
				return CWMP_NEED_RESTART_WAN;
			}
			else
				return ERR_9001;
		}
		else
			return ERR_9001;
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6IPAddressAlias" )==0 )
	{
		char *buf = data;

		strncpy(pEntry->Ipv6AddrAlias, buf, 41);
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
		return CWMP_APPLIED;
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6IPAddressOrigin" )==0 )
	{
		char *buf = data;

		if(pEntry->IpProtocol & IPVER_IPV6)
		{
			if(strcmp(buf, "AutoConfigured") == 0)
				pEntry->AddrMode = IPV6_WAN_AUTO;
			else if(strcmp(buf, "DHCPv6") == 0)
			{
				pEntry->AddrMode = IPV6_WAN_DHCP;
				pEntry->Ipv6DhcpRequest |= 1;	//get address
			}
			else if(strcmp(buf, "Static") == 0)
				pEntry->AddrMode = IPV6_WAN_STATIC;
			else if(strcmp(buf, "None") == 0)
				pEntry->AddrMode = IPV6_WAN_NONE;
			else
				return ERR_9007;

			mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
			return CWMP_NEED_RESTART_WAN;
		}
		else
			return ERR_9001;
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6DNSServers" )==0 )
	{
		char *buf = data;
		unsigned char dnsMode;
		struct addrinfo hint = {0};
		struct addrinfo *tmpaddr = NULL;
		char *str_server = NULL;
		int cnt = 0;
		const unsigned char zeroip[IP6_ADDR_LEN] = {0};

		mib_get( MIB_ADSL_WAN_DNS_MODE, &dnsMode);

		hint.ai_family = PF_UNSPEC;
		hint.ai_flags = AI_NUMERICHOST;
		hint.ai_family = AF_INET6;

		CWMPDBG(1, (stderr, "<%s/%d> Setting DNS Server: %s\n", __FUNCTION__, __LINE__, buf));

		if(buf == NULL)
			return ERR_9007;

		if(pEntry->AddrMode != IPV6_WAN_STATIC)
			return ERR_9001;

		str_server = strtok(buf, ", ");
		while(str_server)
		{
			struct sockaddr_in6 *new_addr = NULL;

			if(cnt >= 2)
				return ERR_9007;

			if (getaddrinfo(str_server, NULL, &hint, &tmpaddr) != 0)
				return ERR_9007;

			new_addr = (struct sockaddr_in6 *)tmpaddr->ai_addr;

			switch(cnt)
			{
			case 0:
				memcpy(pEntry->Ipv6Dns1, &new_addr->sin6_addr, sizeof(pEntry->Ipv6Dns1));
				break;
			case 1:
				memcpy(pEntry->Ipv6Dns2, &new_addr->sin6_addr, sizeof(pEntry->Ipv6Dns1));
				break;
			}

			cnt++;
			freeaddrinfo(tmpaddr);
			tmpaddr = NULL;
			str_server = strtok(NULL, ", ");
		}

		switch(cnt)
		{
		case 0:
			memset(pEntry->Ipv6Dns1, 0, sizeof(pEntry->Ipv6Dns1));
		case 1:
			memset(pEntry->Ipv6Dns2, 0, sizeof(pEntry->Ipv6Dns2));
			break;
		}

		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6PrefixDelegationEnabled" )==0 )
	{
		int *i = data;

		if(*i == 0)
			pEntry->Ipv6DhcpRequest &= ~0x2;
		else
			pEntry->Ipv6DhcpRequest |= 0x2;

		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6PrefixAlias" )==0 )
	{
		char *buf = data;

		strncpy(pEntry->Ipv6PrefixAlias, buf, 41);
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
		return CWMP_APPLIED;
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6PrefixOrigin" )==0 )
	{
		char *buf = data;

		if(pEntry->IpProtocol & 0x2)
		{
			if(strcmp(buf, "PrefixDelegation") == 0)
				pEntry->IPv6PrefixOrigin = IPV6_PREFIX_DELEGATION;
			else if(strcmp(buf, "Static") == 0)
				pEntry->IPv6PrefixOrigin = IPV6_PREFIX_STATIC;
			else if(strcmp(buf, "PPPoE") == 0)
				pEntry->IPv6PrefixOrigin = IPV6_PREFIX_PPPOE;
			else if(strcmp(buf, "None") == 0)
				pEntry->IPv6PrefixOrigin = IPV6_PREFIX_NONE;
			else
				return ERR_9007;

			mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
			return CWMP_NEED_RESTART_WAN;
		}
		else
			return ERR_9001;
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6Prefix" )==0 )
	{
		char *buf = data;

		if(pEntry->IpProtocol & IPVER_IPV6)
		{
			struct in6_addr prefix;
			int len;
			char addr[40] = "";

			if(pEntry->AddrMode & IPV6_WAN_STATIC)
			{
				if(sscanf(buf, "%[^/]/%d", addr, &len) != 2)
					return ERR_9007;
				if(inet_pton(AF_INET6, addr, &prefix) != 1)
					return ERR_9007;

				pEntry->Ipv6AddrPrefixLen = len;
				mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
				return CWMP_NEED_RESTART_WAN;
			}
			else
				return ERR_9001;
		}
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6PrefixPltime" )==0 )
	{
		unsigned int *i = data;

		if(i == NULL)
			return ERR_9007;

		if((pEntry->IpProtocol & 0x2) && (pEntry->AddrMode & IPV6_WAN_STATIC))
		{
			pEntry->IPv6PrefixPltime = *i;
			mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
			return CWMP_NEED_RESTART_WAN;
		}

		return ERR_9001;
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6PrefixVltime" )==0 )
	{
		unsigned int *i = data;

		if(i == NULL)
			return ERR_9007;

		if((pEntry->IpProtocol & 0x2) && (pEntry->AddrMode & IPV6_WAN_STATIC))
		{
			pEntry->IPv6PrefixVltime = *i;
			mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
			return CWMP_NEED_RESTART_WAN;
		}

		return ERR_9001;
	}
	else if( strcmp( lastname, "X_CT-COM_DefaultIPv6Gateway" )==0 )
	{
		char *buf = data;

		if(pEntry->IpProtocol & IPVER_IPV6)
		{
			struct in6_addr prefix;
			int len;
			char addr[40] = "";

			if(pEntry->AddrMode & IPV6_WAN_STATIC)
			{
				if(inet_pton(AF_INET6, buf, pEntry->RemoteIpv6Addr) != 1)
					return ERR_9007;

				mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
				return CWMP_NEED_RESTART_WAN;
			}
			else
				return ERR_9001;
		}
		else
			return ERR_9001;
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6DomainName" )==0 )
	{
		char *buf = data;

		strncpy(pEntry->IPv6DomainName, buf, sizeof(pEntry->IPv6DomainName));
		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "X_CT-COM_Dslite_Enable" )==0 )
	{
		int *i = data;

		pEntry->dslite_enable = *i;

		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx);
		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "X_CT-COM_AftrMode" )==0 )
	{
		int *i = data;

		if(*i != 1 && *i != 0)
			return ERR_9007;

		pEntry->dslite_aftr_mode = *i;

		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx);
		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "X_CT-COM_Aftr" )==0 )
	{
		char *buf = data;
		struct in6_addr ip6Addr= {0};

		strncpy(pEntry->dslite_aftr_hostname, buf, sizeof(pEntry->dslite_aftr_hostname));
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx);

		return CWMP_NEED_RESTART_WAN;
	}
#ifdef CONFIG_MCAST_VLAN
	else if( strcmp( lastname, "X_CT-COM_MulticastVlan" )==0 )
	{
		int *i = data;
		unsigned short vid = 0;

		if(i == NULL) return ERR_9007;

		if(*i == -1)
			vid = 0;
		else if(*i < 1 || *i > 4094)
			return ERR_9007;
		else
			vid = *i;

		pEntry->mVid = vid;
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx);

		apply_add(CWMP_PRI_N, apply_mcast_vlan, CWMP_RESTART, 0, NULL, 0);
		return CWMP_APPLIED;
	}
#endif
	else{
		return ERR_9005;
	}

	return CWMP_APPLIED;
}
#endif	//_PRMT_X_CT_EXT_ENABLE_



/*****************************************************************************
 * InternetGatewayDevice.WANDevice.{i}.WANConnectionDevice.{i}.WANIPConnection.
 *****************************************************************************/
#ifdef _PRMT_X_CT_COM_WANEXT_
struct CWMP_OP tCTWANIPCONENTITYLeafOP = { getCTWANIPCONENTITY, setCTWANIPCONENTITY };
struct CWMP_PRMT tCTWANIPCONENTITYLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"X_CT-COM_LanInterface",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTWANIPCONENTITYLeafOP},
{"X_CT-COM_ServiceList",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTWANIPCONENTITYLeafOP},
{"X_CT-COM_IPMode",			eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tCTWANIPCONENTITYLeafOP},
{"X_CT-COM_IPv6ConnStatus",	eCWMP_tSTRING,	CWMP_READ,	&tCTWANIPCONENTITYLeafOP},
{"X_CT-COM_LanInterface-DHCPEnable",	eCWMP_tBOOLEAN, CWMP_WRITE|CWMP_READ,	&tCTWANIPCONENTITYLeafOP},
{"X_CT-COM_IPForwardList",	   eCWMP_tSTRING,  CWMP_WRITE|CWMP_READ,   &tCTWANIPCONENTITYLeafOP},
{"X_CT-COM_IPv6IPAddress",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tCTWANIPCONENTITYLeafOP},
{"X_CT-COM_IPv6IPAddressAlias", 	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tCTWANIPCONENTITYLeafOP},
{"X_CT-COM_IPv6IPAddressOrigin",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tCTWANIPCONENTITYLeafOP},
{"X_CT-COM_IPv6DNSServers", 		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tCTWANIPCONENTITYLeafOP},
{"X_CT-COM_IPv6PrefixDelegationEnabled",	eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tCTWANIPCONENTITYLeafOP},
{"X_CT-COM_IPv6PrefixAlias", 		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tCTWANIPCONENTITYLeafOP},
{"X_CT-COM_IPv6PrefixOrigin", 		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tCTWANIPCONENTITYLeafOP},
{"X_CT-COM_IPv6Prefix",	 		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tCTWANIPCONENTITYLeafOP},
{"X_CT-COM_IPv6PrefixPltime",		eCWMP_tUINT,		CWMP_READ|CWMP_WRITE,	&tCTWANIPCONENTITYLeafOP},
{"X_CT-COM_IPv6PrefixVltime",		eCWMP_tUINT,		CWMP_READ|CWMP_WRITE,	&tCTWANIPCONENTITYLeafOP},
{"X_CT-COM_DefaultIPv6Gateway",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tCTWANIPCONENTITYLeafOP},
{"X_CT-COM_IPv6DomainName",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tCTWANIPCONENTITYLeafOP},
{"X_CT-COM_Dslite_Enable",			eCWMP_tUINT,		CWMP_READ|CWMP_WRITE,	&tCTWANIPCONENTITYLeafOP},
{"X_CT-COM_AftrMode",				eCWMP_tINT,		CWMP_READ|CWMP_WRITE,	&tCTWANIPCONENTITYLeafOP},
{"X_CT-COM_Aftr",					eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tCTWANIPCONENTITYLeafOP},
#ifdef CONFIG_MCAST_VLAN
{"X_CT-COM_MulticastVlan",			eCWMP_tINT, 		CWMP_WRITE|CWMP_READ,	&tCTWANIPCONENTITYLeafOP},
#endif
};

int getCTWANIPCONENTITY(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned int	chainidx;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;
	char	ifname[16];
	char	buf[512]="";
	unsigned char vChar=0;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	pEntry = &vc_entity;
	if( getATMVCEntry(name, pEntry, &chainidx) < 0 )
		return ERR_9002;

	if(ifGetName( pEntry->ifIndex, ifname, 16 )==0) return ERR_9002;

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "X_CT-COM_LanInterface" )==0 )
	{
#ifdef NEW_PORTMAPPING
		char tmp[1024];
		getLanInterface(pEntry->itfGroup, tmp);
		*data = strdup( tmp );
#else
		*data = strdup( "" );
#endif
	}
	else if( strcmp( lastname, "X_CT-COM_ServiceList" )==0 )
	{
		convertFlag2ServiceList( pEntry->ServiceList, buf );
		*data = strdup( buf );
	}
	else if( strcmp( lastname, "X_CT-COM_IPForwardList" )==0 )
	{
		//We don't support IP Forward mode
		*data = strdup("");
	}
#ifdef CONFIG_MCAST_VLAN
	else if( strcmp( lastname, "X_CT-COM_MulticastVlan" )==0 )
	{
		int vid = pEntry->mVid;

		if(vid == 0)
			*data = intdup(-1);
		else
			*data = intdup(vid);
	}
#endif
	else if( strcmp( lastname, "X_CT-COM_IPMode" ) == 0 )
	{
#ifdef CONFIG_IPV6
		switch(pEntry->IpProtocol)
		{
			case IPVER_IPV4:
			case IPVER_IPV6:
			case IPVER_IPV4_IPV6:
				*data = uintdup(pEntry->IpProtocol);
				break;
			default:
				return ERR_9003;
				break;
		}
#else
		*data = uintdup(1);	//IPv4
#endif
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6ConnStatus" ) == 0 )
	{
		int flags;

		if(pEntry->IpProtocol == IPVER_IPV4)	//IPv4 only
			*data = strdup("Unconfigured");
		else if (pEntry->enable == 0 || getInFlags(ifname, &flags) == 0
				|| (flags & IFF_RUNNING) == 0) {
			*data = strdup("Disconnected");
		} else {
			*data = strdup("Connected");
		}
	}
	else if( strcmp( lastname, "X_CT-COM_LanInterface-DHCPEnable" )==0 )
	{
		if(pEntry->disableLanDhcp)
			*data = booldup(0);
		else
			*data = booldup(1);
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6IPAddress" )==0 )
	{
		if(pEntry->IpProtocol & IPVER_IPV6)
		{
			struct ipv6_ifaddr addr;
			if(pEntry->AddrMode & IPV6_WAN_STATIC)
				inet_ntop(AF_INET6, pEntry->Ipv6Addr, buf, sizeof(buf));
			else if(getifip6(ifname, IPV6_ADDR_UNICAST, &addr, 1) == 1)
				inet_ntop(AF_INET6, &addr.addr, buf, sizeof(buf));
		}

		*data = strdup(buf);
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6IPAddressAlias" )==0 )
	{
		*data = strdup(pEntry->Ipv6AddrAlias);
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6IPAddressOrigin" )==0 )
	{
		if(pEntry->IpProtocol & IPVER_IPV6)
		{
			if(pEntry->AddrMode & IPV6_WAN_AUTO)
				*data = strdup("AutoConfigured");
			else if(pEntry->AddrMode & IPV6_WAN_STATIC)
				*data = strdup("Static");
			else if(pEntry->AddrMode & IPV6_WAN_DHCP)
				*data = strdup("DHCPv6");
			else
				*data = strdup("None");
		}
		else
			*data = strdup("None");
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6DNSServers" )==0 )
	{
		unsigned char dnsMode = 0;
		char str_server[40] = {0};

		get_dns6_by_wan(buf, sizeof(buf), pEntry);
		*data = strdup(buf);
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6PrefixDelegationEnabled" )==0 )
	{
		*data = booldup((pEntry->Ipv6DhcpRequest & 0x2) ? 1 : 0);
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6PrefixAlias" )==0 )
	{
		*data = strdup(pEntry->Ipv6PrefixAlias);
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6PrefixOrigin" )==0 )
	{
		switch(pEntry->IPv6PrefixOrigin)
		{
		case 1:
			*data = strdup("PrefixDelegation");
			break;
		case 2:
			*data = strdup("Static");
			break;
		case 3:
			*data = strdup("PPPoE");
			break;
		default:
			*data = strdup("None");
			break;
		}
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6Prefix" )==0 )
	{
		const char *dst = NULL;

		if(pEntry->AddrMode & IPV6_WAN_STATIC)
		{
			struct in6_addr prefix = {0};

			ip6toPrefix(pEntry->Ipv6Addr, pEntry->Ipv6AddrPrefixLen, &prefix);
			dst = inet_ntop(AF_INET6, &prefix, buf, sizeof(buf));

			if(dst)
			{
				sprintf(buf, "%s/%d", dst, pEntry->Ipv6AddrPrefixLen);
				*data = strdup(buf);
			}
			else
				*data = strdup("");
		}
		else
		{
			DLG_INFO_T dlg_info;
			char fname[256] = {0};
			int ret;

			snprintf(fname, 256, LEASE_FNAME_FMT, ifname);
			ret = getLeasesInfo(fname, &dlg_info);
			dst = inet_ntop(AF_INET6, dlg_info.prefixIP, buf, sizeof(buf));

			if(ret && dst)
			{
				sprintf(buf, "%s/%d", dst, dlg_info.prefixLen);
				*data = strdup(buf);
			}
			else
				*data = strdup("");
		}
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6PrefixPltime" )==0 )
	{
		if(pEntry->AddrMode & IPV6_WAN_STATIC)
		{
			*data = uintdup(pEntry->IPv6PrefixPltime);
		}
		else
		{
			DLG_INFO_T dlg_info;
			char fname[256] = {0};
			int ret;

			snprintf(fname, 256, LEASE_FNAME_FMT, ifname);
			ret = getLeasesInfo(fname, &dlg_info);
			*data = intdup(ret ? dlg_info.PLTime : 0);
		}
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6PrefixVltime" )==0 )
	{
		if(pEntry->AddrMode & IPV6_WAN_STATIC)
		{
			*data = uintdup(pEntry->IPv6PrefixVltime);
		}
		else
		{
			DLG_INFO_T dlg_info;
			char fname[256] = {0};
			int ret;
		
			snprintf(fname, 256, LEASE_FNAME_FMT, ifname);
			ret = getLeasesInfo(fname, &dlg_info);
			*data = intdup(ret ? dlg_info.MLTime : 0);
		}
	}
	else if( strcmp( lastname, "X_CT-COM_DefaultIPv6Gateway" )==0 )
	{
		if(pEntry->IpProtocol & IPVER_IPV6)
		{
			if(pEntry->AddrMode & IPV6_WAN_STATIC)
				inet_ntop(AF_INET6, pEntry->RemoteIpv6Addr, buf, sizeof(buf));

			//TODO: get IPv6 gateway
		}

		*data = strdup(buf);
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6DomainName" )==0 )
	{
		*data = strdup( pEntry->IPv6DomainName);
	}
	else if( strcmp( lastname, "X_CT-COM_Dslite_Enable" )==0 )
	{
		int enable = pEntry->dslite_enable;

		*data = booldup(enable);
	}
	else if( strcmp( lastname, "X_CT-COM_AftrMode" )==0 )
	{
		*data = intdup(pEntry->dslite_aftr_mode);
	}
	else if( strcmp( lastname, "X_CT-COM_Aftr" )==0 )
	{
		*data = strdup(pEntry->dslite_aftr_hostname);
	}
	else{
		return ERR_9005;
	}

	return 0;
}

int setCTWANIPCONENTITY(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	unsigned int	chainidx;
	MIB_CE_ATM_VC_T *pEntry, vc_entity;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	pEntry = &vc_entity;
	if( getATMVCEntry(name, pEntry, &chainidx) < 0 )
		return ERR_9002;

	gWanBitMap |= (1U << chainidx);

	if( strcmp( lastname, "X_CT-COM_LanInterface" )==0 )
	{
#ifdef NEW_PORTMAPPING
		char *buf = data;
		int ret_err;
		if( buf==NULL ) return ERR_9007;
		ret_err = setLanInterface( buf, pEntry );
		if( ret_err < 0 ) return ret_err;
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );

		apply_add( CWMP_PRI_H, apply_PortMapping, CWMP_RESTART, 0, NULL, 0 );
		return CWMP_APPLIED;
#else
		return ERR_9001;
#endif
	}else if( strcmp( lastname, "X_CT-COM_ServiceList" )==0 )
	{
		char *buf = data;
		unsigned short flag=0;
		if( buf==NULL ) return ERR_9007;
		if( convertServiceList2Flag( buf, &flag )<0 ) return ERR_9007;
		pEntry->ServiceList=flag;
		pEntry->applicationtype=flag;
#if defined(CTC_WAN_NAME)&&defined(CONFIG_E8B)
		if(((flag&X_CT_SRV_TR069)!=0)&&((flag&X_CT_SRV_INTERNET)!=0)){
			if(pEntry->cmode!=CHANNEL_MODE_BRIDGE){
				pEntry->napt=1;
#ifndef CONFIG_RTK_RG_INIT
				pEntry->dgw=1;
#endif
			}
		}
		else if((flag&X_CT_SRV_INTERNET)!=0){
			if(pEntry->cmode!=CHANNEL_MODE_BRIDGE){
				pEntry->napt=1;
#ifndef CONFIG_RTK_RG_INIT
				pEntry->dgw=1;
#endif
			}
		}
		else if((flag&X_CT_SRV_TR069)!=0){
			pEntry->napt=0;
			pEntry->dgw=0;
		}
		else if((flag&X_CT_SRV_OTHER)!=0){
			pEntry->napt=0;
			pEntry->dgw=0;
		}
		else{
			pEntry->napt=0;
			pEntry->dgw=0;
		}
#endif
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
#ifdef E8B_NEW_DIAGNOSE
		FILE * fp1;
		int fileid;
		char tmpbuf[20];
		pvc_status_entry entry;
		memset(&entry, 0, sizeof(pvc_status_entry));
		entry.vpi = pEntry->vpi;
		entry.vci = pEntry->vci;
		fileid = findmatchpvcfileid(&entry);
		if (fileid > 0) {
			sprintf(tmpbuf, "%s%d", PVC_FILE, fileid);
			fp1 = fopen(tmpbuf, "wb+");
			if (fp1) {
				setWanName(entry.servertype, pEntry->applicationtype);
				fwrite(&entry, sizeof(pvc_status_entry), 1, fp1);
				fclose(fp1);
			}
		}
#endif
		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "X_CT-COM_IPMode" ) == 0 )
	{
		int *i = data;

		if(i==NULL) return ERR_9007;

#ifdef CONFIG_IPV6
		switch(*i)
		{
			case 1:
			case 2:
			case 3:
				pEntry->IpProtocol = *i;
				break;
			default:
				return ERR_9003;
				break;
		}
		if(pEntry->IpProtocol & IPVER_IPV6)
			pEntry->Ipv6DhcpRequest |= 2;
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
		return CWMP_NEED_RESTART_WAN;
#else
		return ERR_9003;
#endif
	}
	else if( strcmp( lastname, "X_CT-COM_LanInterface-DHCPEnable" )==0 )
	{
		int *i = data;

		if(i==NULL) return ERR_9007;
		pEntry->disableLanDhcp = (*i==0) ? 1:0;

		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );

		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "X_CT-COM_IPForwardList" )==0 )
	{
		//We don't support IP Forward mode
		return ERR_9001;
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6IPAddress" )==0 )
	{
		char *buf = data;

		if(pEntry->IpProtocol & IPVER_IPV6)
		{
			if(pEntry->AddrMode & IPV6_WAN_STATIC)
			{
				if(inet_pton(AF_INET6, buf, pEntry->Ipv6Addr) != 1)
					return ERR_9007;

				mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
				return CWMP_NEED_RESTART_WAN;
			}
			else
				return ERR_9001;
		}
		else
			return ERR_9001;
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6IPAddressAlias" )==0 )
	{
		char *buf = data;

		strncpy(pEntry->Ipv6AddrAlias, buf, 41);
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
		return CWMP_APPLIED;
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6IPAddressOrigin" )==0 )
	{
		char *buf = data;

		if(pEntry->IpProtocol & IPVER_IPV6)
		{
			if(strcmp(buf, "AutoConfigured") == 0)
				pEntry->AddrMode = IPV6_WAN_AUTO;
			else if(strcmp(buf, "DHCPv6") == 0)
			{
				pEntry->AddrMode = IPV6_WAN_DHCP;
				pEntry->Ipv6DhcpRequest |= 1;	//get address
			}
			else if(strcmp(buf, "Static") == 0)
				pEntry->AddrMode = IPV6_WAN_STATIC;
			else if(strcmp(buf, "None") == 0)
				pEntry->AddrMode = IPV6_WAN_NONE;
			else
				return ERR_9007;

			mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
			return CWMP_NEED_RESTART_WAN;
		}
		else
			return ERR_9001;
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6DNSServers" )==0 )
	{
		char *buf = data;
		unsigned char dnsMode;
		struct addrinfo hint = {0};
		struct addrinfo *tmpaddr = NULL;
		char *str_server = NULL;
		int cnt = 0;
		const unsigned char zeroip[IP6_ADDR_LEN] = {0};

		mib_get( MIB_ADSL_WAN_DNS_MODE, &dnsMode);

		hint.ai_family = PF_UNSPEC;
		hint.ai_flags = AI_NUMERICHOST;
		hint.ai_family = AF_INET6;

		CWMPDBG(1, (stderr, "<%s/%d> Setting DNS Server: %s\n", __FUNCTION__, __LINE__, buf));

		if(buf == NULL)
			return ERR_9007;

		if(pEntry->AddrMode != IPV6_WAN_STATIC)
			return ERR_9001;

		str_server = strtok(buf, ", ");
		while(str_server)
		{
			struct sockaddr_in6 *new_addr = NULL;

			if(cnt >= 2)
				return ERR_9007;

			if (getaddrinfo(str_server, NULL, &hint, &tmpaddr) != 0)
				return ERR_9007;

			new_addr = (struct sockaddr_in6 *)tmpaddr->ai_addr;

			switch(cnt)
			{
			case 0:
				memcpy(pEntry->Ipv6Dns1, &new_addr->sin6_addr, sizeof(pEntry->Ipv6Dns1));
				break;
			case 1:
				memcpy(pEntry->Ipv6Dns2, &new_addr->sin6_addr, sizeof(pEntry->Ipv6Dns1));
				break;
			}
			
			cnt++;
			freeaddrinfo(tmpaddr);
			tmpaddr = NULL;
			str_server = strtok(NULL, ", ");
		}

		switch(cnt)
		{
		case 0:
			memset(pEntry->Ipv6Dns1, 0, sizeof(pEntry->Ipv6Dns1));
		case 1:
			memset(pEntry->Ipv6Dns2, 0, sizeof(pEntry->Ipv6Dns2));
			break;
		}

		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6PrefixDelegationEnabled" )==0 )
	{
		int *i = data;

		if(*i == 0)
			pEntry->Ipv6DhcpRequest &= ~0x2;
		else
			pEntry->Ipv6DhcpRequest |= 0x2;

		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6PrefixAlias" )==0 )
	{
		char *buf = data;

		strncpy(pEntry->Ipv6PrefixAlias, buf, 41);
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
		return CWMP_APPLIED;
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6PrefixOrigin" )==0 )
	{
		char *buf = data;

		if(pEntry->IpProtocol & 0x2)
		{
			if(strcmp(buf, "PrefixDelegation") == 0)
				pEntry->IPv6PrefixOrigin = IPV6_PREFIX_DELEGATION;
			else if(strcmp(buf, "Static") == 0)
				pEntry->IPv6PrefixOrigin = IPV6_PREFIX_STATIC;
			else if(strcmp(buf, "PPPoE") == 0)
				pEntry->IPv6PrefixOrigin = IPV6_PREFIX_PPPOE;
			else if(strcmp(buf, "None") == 0)
				pEntry->IPv6PrefixOrigin = IPV6_PREFIX_NONE;
			else
				return ERR_9007;

			mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
			return CWMP_NEED_RESTART_WAN;
		}
		else
			return ERR_9001;
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6Prefix" )==0 )
	{
		char *buf = data;

		if(pEntry->IpProtocol & IPVER_IPV6)
		{
			struct in6_addr prefix;
			int len;
			char addr[40] = "";

			if(pEntry->AddrMode & IPV6_WAN_STATIC)
			{
				if(sscanf(buf, "%[^/]/%d", addr, &len) != 2)
					return ERR_9007;
				if(inet_pton(AF_INET6, addr, &prefix) != 1)
					return ERR_9007;

				pEntry->Ipv6AddrPrefixLen = len;
				mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
				return CWMP_NEED_RESTART_WAN;
			}
			else
				return ERR_9001;
		}
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6PrefixPltime" )==0 )
	{
		unsigned int *i = data;

		if(i == NULL)
			return ERR_9007;

		if((pEntry->IpProtocol & 0x2) && (pEntry->AddrMode & IPV6_WAN_STATIC))
		{
			pEntry->IPv6PrefixPltime = *i;
			mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
			return CWMP_NEED_RESTART_WAN;
		}

		return ERR_9001;
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6PrefixVltime" )==0 )
	{
		unsigned int *i = data;

		if(i == NULL)
			return ERR_9007;

		if((pEntry->IpProtocol & 0x2) && (pEntry->AddrMode & IPV6_WAN_STATIC))
		{
			pEntry->IPv6PrefixVltime = *i;
			mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
			return CWMP_NEED_RESTART_WAN;
		}

		return ERR_9001;
	}
	
	else if( strcmp( lastname, "X_CT-COM_DefaultIPv6Gateway" )==0 )
	{
		char *buf = data;

		if(pEntry->IpProtocol & IPVER_IPV6)
		{
			struct in6_addr prefix;
			int len;
			char addr[40] = "";

			if(pEntry->AddrMode & IPV6_WAN_STATIC)
			{
				if(inet_pton(AF_INET6, buf, pEntry->RemoteIpv6Addr) != 1)
					return ERR_9007;

				mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
				return CWMP_NEED_RESTART_WAN;
			}
			else
				return ERR_9001;
		}
		else
			return ERR_9001;
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6DomainName" )==0 )
	{
		char *buf = data;

		strncpy(pEntry->IPv6DomainName, buf, sizeof(pEntry->IPv6DomainName));
		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "X_CT-COM_Dslite_Enable" )==0 )
	{
		int *i = data;

		pEntry->dslite_enable = *i;

		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx);
		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "X_CT-COM_AftrMode" )==0 )
	{
		int *i = data;

		if(*i != 1 && *i != 0)
			return ERR_9007;

		pEntry->dslite_aftr_mode = *i;

		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx);
		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "X_CT-COM_Aftr" )==0 )
	{
		char *buf = data;
		struct in6_addr ip6Addr= {0};

		strncpy(pEntry->dslite_aftr_hostname, buf, sizeof(pEntry->dslite_aftr_hostname));
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx);

		return CWMP_NEED_RESTART_WAN;
	}
#ifdef CONFIG_MCAST_VLAN
	else if( strcmp( lastname, "X_CT-COM_MulticastVlan" )==0 )
	{
		int *i = data;
		unsigned short vid = 0;

		if(i == NULL) 	return ERR_9007;

		if(*i == -1)
			vid = 0;
		else if(*i < 1 || *i > 4094)
			return ERR_9007;
		else
			vid = *i;

		pEntry->mVid = vid;
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx);

		apply_add(CWMP_PRI_N, apply_mcast_vlan, CWMP_RESTART, 0, NULL, 0);
		return CWMP_APPLIED;
	}
#endif
	else
		return ERR_9005;

	return 0;
}

#endif	//_PRMT_X_CT_COM_WANEXT_




/*****************************************************************************
 * InternetGatewayDevice.WANDevice.{i}.WANConnectionDevice.{i}.WANEthernetLinkConfig.
 *****************************************************************************/
#if defined(CONFIG_ETHWAN) && defined(_PRMT_X_CT_COM_ETHLINK_)
struct CWMP_OP tCTWANETHLINKFLeafOP = { getCTWANETHLINKCONF, setCTWANETHLINKCONF };
struct CWMP_PRMT tCTWANETHLINKLeafInfo[] =
{
	/*(name,			type,		flag,			op)*/
	{"X_CT-COM_Enable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ, &tCTWANETHLINKFLeafOP},
	{"X_CT-COM_Mode",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ, &tCTWANETHLINKFLeafOP},
	{"X_CT-COM_VLANIDMark",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ, &tCTWANETHLINKFLeafOP},
#if 0
/* Removed becuase this parameter is not available in e8 spec anymore.*/
	{"X_CT-COM_802-1pMark",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ, &tCTWANETHLINKFLeafOP},
#endif
};

int getCTWANETHLINKCONF(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned int wandevnum, devnum,num,i;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	wandevnum = getWANDevInstNum(name);
	devnum = getWANConDevInstNum(name);
	if(devnum==0 || wandevnum ==0) return ERR_9005;

	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		pEntry = &vc_entity;
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)pEntry ))
			continue;

		if(!isWANDevModeMatch(wandevnum, pEntry->ifIndex))
			continue;

		if( pEntry->ConDevInstNum==devnum )
			break;
	}

	if(i==num) return ERR_9005;

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "X_CT-COM_Enable" )==0 )
	{
		*data = booldup(1);
	}
	else if( strcmp( lastname, "X_CT-COM_Mode" )==0 )
	{
		unsigned int mode;

		switch(pEntry->vlan)
		{
		case 0:	//untag
			mode = 0;
			break;
		case 1:	//tag
			mode = 2;
			break;
		case 2:	//transparent
			mode = 1;
			break;
		}

		*data = uintdup(mode);
	}
	else if( strcmp( lastname, "X_CT-COM_VLANIDMark" )==0 )
	{
		*data = uintdup(pEntry->vid);
	}
#if 0
	else if( strcmp( lastname, "X_CT-COM_802-1pMark" )==0 )
	{
		*data = uintdup(pEntry->vprio);
	}
#endif
	else
		return ERR_9005;

	return 0;
}

int setCTWANETHLINKCONF(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;
	unsigned int wandevnum, devnum,num,chainid;
	int ret=0;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;
	if( data==NULL ) return ERR_9007;

	wandevnum = getWANDevInstNum(name);
	devnum = getWANConDevInstNum(name);
	if(devnum==0 || wandevnum==0) return ERR_9005;

	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( chainid=0; chainid<num;chainid++ )
	{
		pEntry = &vc_entity;
		if( !mib_chain_get( MIB_ATM_VC_TBL, chainid, (void*)pEntry ))
			continue;

		if(!isWANDevModeMatch(wandevnum, pEntry->ifIndex))
			continue;

		if( pEntry->ConDevInstNum==devnum )
		{
			gWanBitMap |= (1U << chainid);
			if( strcmp( lastname, "X_CT-COM_VLANIDMark" )==0 )
			{
				unsigned int *i = data;

				if( i==NULL ) return ERR_9007;

				pEntry->vid=*i;
				mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainid );
				ret=CWMP_NEED_RESTART_WAN;//return CWMP_NEED_RESTART_WAN;
			}
			else if( strcmp( lastname, "X_CT-COM_Enable" )==0 )
			{
				return ERR_9001;
			}
			else if( strcmp( lastname, "X_CT-COM_Mode" )==0 )
			{
				unsigned int *mode = data;

				switch(*mode)
				{
				case 1:
					pEntry->vlan = 2;	//transparent
				case 0:
					pEntry->vlan = 0;	//untag
					break;
				case 2:
					pEntry->vlan = 1;	//tag
					break;
				default:
					return ERR_9007;
				}

				mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainid);
				return CWMP_NEED_RESTART_WAN;
			}
#if 0
			else if( strcmp( lastname, "X_CT-COM_802-1pMark" )==0 )
			{
				unsigned int *i = data;

				if( i==NULL ) return ERR_9007;
				if( *i<0 || *i>7) return ERR_9007;
				//pEntry->pmark=*i;
				pEntry->vprio = *i;
				mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainid );
				ret=CWMP_NEED_RESTART_WAN;//return CWMP_NEED_RESTART_WAN;
			}
#endif
			else{
				return ERR_9005;
			}
		}//if
	}//for

	return ret;
}

#endif //defined(CONFIG_ETHWAN) && defined(_PRMT_X_CT_COM_ETHLINK_)




/*****************************************************************************
 * InternetGatewayDevice.WANDevice.{i}.WANConnectionDevice.{i}.WANDSLLinkConfig.
 *****************************************************************************/
#if defined(CONFIG_DEV_xDSL) && defined(_PRMT_X_CT_COM_WANEXT_)
struct CWMP_OP tCTDSLLNKCONFLeafOP = { getCTDSLLNKCONF, setCTDSLLNKCONF };
struct CWMP_PRMT tCTDSLLNKCONFLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"X_CT-COM_VLAN",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tCTDSLLNKCONFLeafOP},
};

int getCTDSLLNKCONF(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned int wandevnum, devnum,num,i;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	wandevnum = getWANDevInstNum(name);
	devnum = getWANConDevInstNum(name);
	if(devnum==0 || wandevnum == 0) return ERR_9005;

	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		pEntry = &vc_entity;
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)pEntry ))
			continue;

		if(!isWANDevModeMatch(wandevnum, pEntry->ifIndex))
			continue;

		if( pEntry->ConDevInstNum==devnum )
			break;
	}
	if(i==num) return ERR_9005;

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "X_CT-COM_VLAN" )==0 )
	{
		if(pEntry->vlan)
			*data = uintdup(pEntry->vid);
		else
			*data = uintdup(0);
	}
	else{
		return ERR_9005;
	}

	return 0;
}

int setCTDSLLNKCONF(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;
	unsigned int wandevnum, devnum,num,chainid;
	int ret=0;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;
	if( data==NULL ) return ERR_9007;

	wandevnum = getWANDevInstNum(name);
	devnum = getWANConDevInstNum(name);
	if(devnum==0 || wandevnum==0) return ERR_9005;

	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( chainid=0; chainid<num;chainid++ )
	{
		pEntry = &vc_entity;
		if( !mib_chain_get( MIB_ATM_VC_TBL, chainid, (void*)pEntry ))
			continue;

		if(!isWANDevModeMatch(wandevnum, pEntry->ifIndex))
			continue;

		if( pEntry->ConDevInstNum==devnum )
		{
			gWanBitMap |= (1U << chainid);
			if( strcmp( lastname, "X_CT-COM_VLAN" )==0 )
			{
				unsigned int *i = data;

				if( i==NULL ) return ERR_9007;
				pEntry->vlan = 1;
				pEntry->vid=*i;
				mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainid );
				ret=CWMP_NEED_RESTART_WAN;//return CWMP_NEED_RESTART_WAN;
			}
			else{
				return ERR_9005;
			}
		}//if
	}//for
	//if(chainid==num) return ERR_9005;

	return ret;
}

#ifdef CONFIG_VDSL
struct CWMP_OP tCT_VDSLLinkConfLeafOP = { getCT_VDSLLinkConf, setCT_VDSLLinkConf };

struct CWMP_PRMT tCT_VDSLLinkConfLeafInfo[] = {
/*(name,		type,		flag,			op)*/
{"Enable",		eCWMP_tBOOLEAN,	CWMP_WRITE | CWMP_READ,	&tCT_VDSLLinkConfLeafOP},
{"Mode",			eCWMP_tUINT,		CWMP_WRITE | CWMP_READ,	&tCT_VDSLLinkConfLeafOP},
{"VLANIDMark",	eCWMP_tUINT,		CWMP_WRITE | CWMP_READ,	&tCT_VDSLLinkConfLeafOP},
};

enum eCT_VDSLLinkConfLeaf
{
	eCTVDSL_Enable,
	eCTVDSL_Mode,
	eCTVDSL_VLANIDMark,
};

struct CWMP_LEAF tCT_VDSLLinkLeaf[] =
{
{ &tCT_VDSLLinkConfLeafInfo[eCTVDSL_Enable] },
{ &tCT_VDSLLinkConfLeafInfo[eCTVDSL_Mode] },
{ &tCT_VDSLLinkConfLeafInfo[eCTVDSL_VLANIDMark] },
{ NULL },
};

int getCT_VDSLLinkConf(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;
	unsigned int wandevnum, devnum,num,i;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;

	if (name == NULL || entity == NULL || type == NULL || data == NULL)
		return -1;

	wandevnum = getWANDevInstNum(name);
	devnum = getWANConDevInstNum(name);
	if(devnum==0 || wandevnum ==0) return ERR_9005;

	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		pEntry = &vc_entity;
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)pEntry ))
			continue;

		if(!isWANDevModeMatch(wandevnum, pEntry->ifIndex))
			continue;

		if( pEntry->ConDevInstNum==devnum )
			break;
	}

	if(i==num) return ERR_9005;

	*type = entity->info->type;
	*data = NULL;
	if(strcmp(lastname, "Enable") == 0)
	{
		XDSL_OP *d = xdsl_get_op(0);
		int mode;

		d->xdsl_msg_get(GetPmdMode, &mode);

		if(mode & MODE_VDSL2)
			*data = booldup(1);
		else
			*data = booldup(0);
	}
	else if(strcmp(lastname, "Mode") == 0)
	{
		unsigned int mode;
		if(pEntry->vlan)
			mode= 2;
		else
		{
			if(pEntry->brmode == BRIDGE_ETHERNET)
				mode = 1;
			else
				mode = 0;
		}

		*data = uintdup(mode);
	}
	else if(strcmp(lastname, "VLANIDMark") == 0)
	{
		*data = uintdup(pEntry->vid);
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int setCT_VDSLLinkConf(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;
	unsigned int wandevnum, devnum,num,i;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;

	if (name == NULL || entity == NULL || data == NULL)
		return -1;

	if (entity->info->type != type)
		return ERR_9006;

	wandevnum = getWANDevInstNum(name);
	devnum = getWANConDevInstNum(name);
	if(devnum==0 || wandevnum ==0) return ERR_9005;

	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		pEntry = &vc_entity;
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)pEntry ))
			continue;

		if(!isWANDevModeMatch(wandevnum, pEntry->ifIndex))
			continue;

		if( pEntry->ConDevInstNum==devnum )
			break;
	}

	if(i==num) return ERR_9005;
	gWanBitMap |= (1U << i);

	if (strcmp(lastname, "Enable") == 0)
	{
		return ERR_9001;
	}
	else if(strcmp(lastname, "Mode") == 0)
	{
		unsigned int *mode = data;

		switch(*mode)
		{
		case 1:
			pEntry->brmode = BRIDGE_ETHERNET;
		case 0:
			pEntry->vlan = 0;
			break;
		case 2:
			pEntry->vlan = 1;
			break;
		default:
			return ERR_9007;
		}

		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, i );
		return CWMP_NEED_RESTART_WAN;
	}
	else if(strcmp(lastname, "VLANIDMark") == 0)
	{
		unsigned int *vlan = data;

		if(*vlan > 4095) return ERR_9007;

		pEntry->vid = *vlan;
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, i );

		return CWMP_NEED_RESTART_WAN;
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

#endif	//CONFIG_VDSL
#endif	//defined(CONFIG_DEV_xDSL) && defined(_PRMT_X_CT_COM_WANEXT_)


#if defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)
#if defined(CONFIG_RTK_L34_ENABLE)
#include <rtk_rg_liteRomeDriver.h>
#else
#include <rtk/ponmac.h>
#include <rtk/gpon.h>
#include <rtk/epon.h>
#endif
#include <math.h>

/*****************************************************************************
 * IGD.WANDevice.{i}.X_CT-COM_GponInterfaceConfig.
 * IGD.WANDevice.{i}.X_CT-COM_EponInterfaceConfig.
 *****************************************************************************/
struct CWMP_OP tCT_XPONInterfaceConfLeafOP = { getCT_XPONInterfaceConf, NULL };

struct CWMP_PRMT tCT_XPONInterfaceConfLeafInfo[] = {
/*(name,		type,		flag,			op)*/
{"Status",				eCWMP_tSTRING,	CWMP_READ, &tCT_XPONInterfaceConfLeafOP},
{"TXPower",				eCWMP_tINT,		CWMP_READ, &tCT_XPONInterfaceConfLeafOP},
{"RXPower",				eCWMP_tINT,		CWMP_READ, &tCT_XPONInterfaceConfLeafOP},
{"TransceiverTemperature",	eCWMP_tINT,		CWMP_READ, &tCT_XPONInterfaceConfLeafOP},
{"SupplyVottage",			eCWMP_tINT,		CWMP_READ, &tCT_XPONInterfaceConfLeafOP},
{"BiasCurrent",			eCWMP_tINT,		CWMP_READ, &tCT_XPONInterfaceConfLeafOP},
};

enum eCT_XPONInterfaceConfLeaf
{
	eCTXPON_Status,
	eCTXPON_TXPower,
	eCTXPON_RXPower,
	eCTXPON_TransceiverTemperature,
	eCTXPON_SupplyVottage,
	eCTXPON_BiasCurrent,
};

struct CWMP_LEAF tCT_XPONInterfaceConfLeaf[] =
{
{ &tCT_XPONInterfaceConfLeafInfo[eCTXPON_Status] },
{ &tCT_XPONInterfaceConfLeafInfo[eCTXPON_TXPower] },
{ &tCT_XPONInterfaceConfLeafInfo[eCTXPON_RXPower] },
{ &tCT_XPONInterfaceConfLeafInfo[eCTXPON_TransceiverTemperature] },
{ &tCT_XPONInterfaceConfLeafInfo[eCTXPON_SupplyVottage] },
{ &tCT_XPONInterfaceConfLeafInfo[eCTXPON_BiasCurrent] },
{ NULL },
};

int getCT_XPONInterfaceConf(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;
	unsigned int wandevnum;

	if (name == NULL || entity == NULL || type == NULL || data == NULL)
		return -1;

	wandevnum = getWANDevInstNum(name);
	if(wandevnum != WANDEVNUM_ETH) return ERR_9005;

	*type = entity->info->type;
	*data = NULL;

	if(strcmp(lastname, "Status") == 0)
	{
		unsigned int mode;

		mib_get(MIB_PON_MODE, (void *)&mode);

		if(mode == GPON_MODE)
		{
			rtk_gpon_fsm_status_t onu;

#if defined(CONFIG_RTK_L34_ENABLE)
	rtk_rg_gpon_ponStatus_get(&onu);
#else
	rtk_gpon_ponStatus_get(&onu);
#endif
			switch(onu)
			{
			case 1:
				*data = strdup("NoSignal");
				break;
			case 2:
				*data = strdup("Initializing");
				break;
			case 3:
			case 4:
				*data = strdup("EstablishingLink");
				break;
			case 5:
				*data = strdup("Up");
				break;
			case 6:
			case 7:
				*data = strdup("Error");
				break;
			}
		}
		else if(mode == EPON_MODE)
		{
			rtk_epon_llid_entry_t llid_entry = {0};

			llid_entry.llidIdx = 0;

#if defined(CONFIG_RTK_L34_ENABLE)
			rtk_rg_epon_llid_entry_get(&llid_entry);
#else
			rtk_epon_llid_entry_get(&llid_entry);
#endif
			if(llid_entry.valid)
				*data = strdup("Up");
			else
				*data = strdup("NoSignal");
		}
		else
		{
			*data = strdup("Disabled");
		}
	}
	else if(strcmp(lastname, "TXPower") == 0)
	{
		rtk_transceiver_data_t transceiver;
		double power;
		int output = 0;

#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_ponmac_transceiver_get(RTK_TRANSCEIVER_PARA_TYPE_TX_POWER,&transceiver);
#else
		rtk_ponmac_transceiver_get(RTK_TRANSCEIVER_PARA_TYPE_TX_POWER,&transceiver);
#endif

		sscanf(transceiver.buf, "%lf", &power);

		output = (10000.0 * pow(10.0, (power/10.0)));

		CWMPDBP2("Got Tx Power: %s = %lf dBm = %d 0.1uW\n", transceiver.buf, power, output);
		*data = intdup(output);
	}
	else if(strcmp(lastname, "RXPower") == 0)
	{
		rtk_transceiver_data_t transceiver;
		double power;
		int output = 0;

#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_ponmac_transceiver_get(RTK_TRANSCEIVER_PARA_TYPE_RX_POWER,&transceiver);
#else
		rtk_ponmac_transceiver_get(RTK_TRANSCEIVER_PARA_TYPE_RX_POWER,&transceiver);
#endif

		sscanf(transceiver.buf, "%lf", &power);

		output = (10000.0 * pow(10.0, (power/10.0)));

		CWMPDBP2("Got Rx Power: %s = %lf dBm = %d 0.1uW\n", transceiver.buf, power, output);
		*data = intdup(output);
	}
	else if(strcmp(lastname, "TransceiverTemperature") == 0)
	{
		rtk_transceiver_data_t transceiver;
		double temp;
		int output = 0;

#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_ponmac_transceiver_get(RTK_TRANSCEIVER_PARA_TYPE_TEMPERATURE,&transceiver);
#else
		rtk_ponmac_transceiver_get(RTK_TRANSCEIVER_PARA_TYPE_TEMPERATURE,&transceiver);
#endif

		sscanf(transceiver.buf, "%lf", &temp);

		output = (temp * 256.0);

		CWMPDBP2("Got Temperature: %s = %lf C = %d 1/256 C\n", transceiver.buf, temp, output);
		*data = intdup(output);
	}
	else if(strcmp(lastname, "SupplyVottage") == 0)
	{
		rtk_transceiver_data_t transceiver;
		double volt;
		int output = 0;

#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_ponmac_transceiver_get(RTK_TRANSCEIVER_PARA_TYPE_VOLTAGE, &transceiver);
#else
		rtk_ponmac_transceiver_get(RTK_TRANSCEIVER_PARA_TYPE_VOLTAGE, &transceiver);
#endif

		sscanf(transceiver.buf, "%lf", &volt);

		output = (volt * 10000.0);

		CWMPDBP2("Got Vottage: %s = %lf V = %d  100uV\n", transceiver.buf, volt, output);
		*data = intdup(output);

	}
	else if(strcmp(lastname, "BiasCurrent") == 0)
	{
		rtk_transceiver_data_t transceiver;
		double bias;
		int output = 0;

#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_ponmac_transceiver_get(RTK_TRANSCEIVER_PARA_TYPE_BIAS_CURRENT, &transceiver);
#else
		rtk_ponmac_transceiver_get(RTK_TRANSCEIVER_PARA_TYPE_BIAS_CURRENT, &transceiver);
#endif

		sscanf(transceiver.buf, "%lf", &bias);

		output = (bias * 500.0);

		CWMPDBP2("Got Bias: %s = %lf mA = %d  2uA\n", transceiver.buf, bias, output);
		*data = intdup(output);

	}
	else
	{
		return ERR_9005;
	}

	return 0;
}



/*****************************************************************************
 * IGD.WANDevice.{i}.X_CT-COM_GponInterfaceConfig.Stat.
 * IGD.WANDevice.{i}.X_CT-COM_EponInterfaceConfig.Stat.
 *****************************************************************************/
struct CWMP_OP tCT_CT_XPONInterfaceConfStatEntityLeafOP = { getCT_XPONInterfaceEntityLeaf, NULL };

struct CWMP_PRMT tCT_XPONInterfaceConfStatEntityLeafInfo[] = {
/*(name,		type,		flag,			op)*/
{"BytesSent",			eCWMP_tUINT,	CWMP_READ, &tCT_CT_XPONInterfaceConfStatEntityLeafOP},
{"BytesReceived", 	eCWMP_tUINT, 	CWMP_READ, &tCT_CT_XPONInterfaceConfStatEntityLeafOP},
{"PacketsSent", 		eCWMP_tUINT, 	CWMP_READ, &tCT_CT_XPONInterfaceConfStatEntityLeafOP},
{"PacketsReceived",	eCWMP_tUINT, 	CWMP_READ, &tCT_CT_XPONInterfaceConfStatEntityLeafOP},
{"SUnicastPackets",	eCWMP_tUINT, 	CWMP_READ, &tCT_CT_XPONInterfaceConfStatEntityLeafOP},
{"RUnicastPackets", 	eCWMP_tUINT, 	CWMP_READ, &tCT_CT_XPONInterfaceConfStatEntityLeafOP},
{"SMulticastPackets", eCWMP_tUINT,	CWMP_READ, &tCT_CT_XPONInterfaceConfStatEntityLeafOP},
{"RMulticastPackets", eCWMP_tUINT, 	CWMP_READ, &tCT_CT_XPONInterfaceConfStatEntityLeafOP},
{"SBroadcastPackets", eCWMP_tUINT, 	CWMP_READ, &tCT_CT_XPONInterfaceConfStatEntityLeafOP},
{"RBroadcastPackets", eCWMP_tUINT, 	CWMP_READ, &tCT_CT_XPONInterfaceConfStatEntityLeafOP},
{"FECError",			eCWMP_tUINT,	CWMP_READ, &tCT_CT_XPONInterfaceConfStatEntityLeafOP},
{"HECError",			eCWMP_tUINT,	CWMP_READ, &tCT_CT_XPONInterfaceConfStatEntityLeafOP},
{"DropPackets",		eCWMP_tUINT,	CWMP_READ, &tCT_CT_XPONInterfaceConfStatEntityLeafOP},
{"SpausePackets",		eCWMP_tUINT,	CWMP_READ, &tCT_CT_XPONInterfaceConfStatEntityLeafOP},
{"RpausePackets",		eCWMP_tUINT,	CWMP_READ, &tCT_CT_XPONInterfaceConfStatEntityLeafOP},
};

enum eCT_XPONInterfaceConfStatEntityLeaf
{
	eCTXPON_BytesSent,
	eCTXPON_BytesReceived,
	eCTXPON_PacketsSent,
	eCTXPON_PacketsReceived,
	eCTXPON_SUnicastPackets,
	eCTXPON_RUnicastPackets,
	eCTXPON_SMulticastPackets,
	eCTXPON_RMulticastPackets,
	eCTXPON_SBroadcastPackets,
	eCTXPON_RBroadcastPackets,
	eCTXPON_FECError,
	eCTXPON_HECError,
	eCTXPON_DropPackets,
	eCTXPON_SpausePackets,
	eCTXPON_RpausePackets,
};

struct CWMP_LEAF tCT_XPONInterfaceConfStatEntityLeaf[] =
{
{ &tCT_XPONInterfaceConfStatEntityLeafInfo[eCTXPON_BytesSent] },
{ &tCT_XPONInterfaceConfStatEntityLeafInfo[eCTXPON_BytesReceived] },
{ &tCT_XPONInterfaceConfStatEntityLeafInfo[eCTXPON_PacketsSent] },
{ &tCT_XPONInterfaceConfStatEntityLeafInfo[eCTXPON_PacketsReceived] },
{ &tCT_XPONInterfaceConfStatEntityLeafInfo[eCTXPON_SUnicastPackets] },
{ &tCT_XPONInterfaceConfStatEntityLeafInfo[eCTXPON_RUnicastPackets] },
{ &tCT_XPONInterfaceConfStatEntityLeafInfo[eCTXPON_SMulticastPackets] },
{ &tCT_XPONInterfaceConfStatEntityLeafInfo[eCTXPON_RMulticastPackets] },
{ &tCT_XPONInterfaceConfStatEntityLeafInfo[eCTXPON_SBroadcastPackets] },
{ &tCT_XPONInterfaceConfStatEntityLeafInfo[eCTXPON_RBroadcastPackets] },
{ &tCT_XPONInterfaceConfStatEntityLeafInfo[eCTXPON_FECError] },
{ &tCT_XPONInterfaceConfStatEntityLeafInfo[eCTXPON_HECError] },
{ &tCT_XPONInterfaceConfStatEntityLeafInfo[eCTXPON_DropPackets] },
{ &tCT_XPONInterfaceConfStatEntityLeafInfo[eCTXPON_SpausePackets] },
{ &tCT_XPONInterfaceConfStatEntityLeafInfo[eCTXPON_RpausePackets] },
{ NULL },
};


struct CWMP_PRMT tCT_XPONInterfaceConfObjectInfo[] =
{
/*(name,	type,		flag,			op)*/
{"Stat",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum eCT_XPONInterfaceConfStatOjbect
{
	eCT_XPONStat,
};

struct CWMP_NODE tCT_XPONInterfaceConfObject[] =
{
/*info,  				leaf,			next)*/
{&tCT_XPONInterfaceConfObjectInfo[eCT_XPONStat],	tCT_XPONInterfaceConfStatEntityLeaf,	NULL},
{NULL,						NULL,			NULL}
};


int getCT_XPONInterfaceEntityLeaf(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;
	unsigned int wandevnum;
	uint64 value;


	if (name == NULL || entity == NULL || type == NULL || data == NULL)
		return -1;

	wandevnum = getWANDevInstNum(name);
	if(wandevnum != WANDEVNUM_ETH) return ERR_9005;

	*type = entity->info->type;
	*data = NULL;
	if(strcmp(lastname, "BytesSent") == 0)
	{
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_stat_port_get(RTK_RG_PORT_PON, IF_OUT_OCTETS_INDEX, &value);
#else
		rtk_stat_port_get(RTK_RG_PORT_PON, IF_OUT_OCTETS_INDEX, &value);
#endif
		*data = uintdup(value);
	}
	else if(strcmp(lastname, "BytesReceived") == 0)
	{
#if defined(CONFIG_RTK_L34_ENABLE)
	rtk_rg_stat_port_get(RTK_RG_PORT_PON, IF_IN_OCTETS_INDEX, &value);
#else
	rtk_stat_port_get(RTK_RG_PORT_PON, IF_IN_OCTETS_INDEX, &value);
#endif
		*data = uintdup(value);
	}
	else if(strcmp(lastname, "PacketsSent") == 0)
	{
		rtk_stat_port_cntr_t counters;

#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_stat_port_getAll(RTK_RG_PORT_PON, &counters);
#else
		rtk_stat_port_getAll(RTK_PORT_PON, &counters);
#endif

		*data = uintdup(counters.ifOutUcastPkts + counters.ifOutMulticastPkts
						+ counters.ifOutBrocastPkts);
	}
	else if(strcmp(lastname, "PacketsReceived") == 0)
	{
		rtk_stat_port_cntr_t counters;

#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_stat_port_getAll(RTK_RG_PORT_PON, &counters);
#else
		rtk_stat_port_getAll(RTK_RG_PORT_PON, &counters);
#endif

		*data = uintdup(counters.ifInUcastPkts + counters.ifInMulticastPkts
						+ counters.ifInBroadcastPkts);
	}
	else if(strcmp(lastname, "SUnicastPackets") == 0)
	{
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_stat_port_get(RTK_RG_PORT_PON, IF_OUT_UCAST_PKTS_CNT_INDEX, &value);
#else
		rtk_stat_port_get(RTK_RG_PORT_PON, IF_OUT_UCAST_PKTS_CNT_INDEX, &value);
#endif
		*data = uintdup(value);
	}
	else if(strcmp(lastname, "RUnicastPackets") == 0)
	{
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_stat_port_get(RTK_RG_PORT_PON, IF_IN_UCAST_PKTS_INDEX, &value);
#else
		rtk_stat_port_get(RTK_RG_PORT_PON, IF_IN_UCAST_PKTS_INDEX, &value);
#endif
		*data = uintdup(value);
	}
	else if(strcmp(lastname, "SMulticastPackets") == 0)
	{
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_stat_port_get(RTK_RG_PORT_PON, IF_OUT_MULTICAST_PKTS_CNT_INDEX, &value);
#else
		rtk_stat_port_get(RTK_RG_PORT_PON, IF_OUT_MULTICAST_PKTS_CNT_INDEX, &value);
#endif
		*data = uintdup(value);
	}
	else if(strcmp(lastname, "RMulticastPackets") == 0)
	{
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_stat_port_get(RTK_RG_PORT_PON, IF_IN_MULTICAST_PKTS_INDEX, &value);
#else
		rtk_stat_port_get(RTK_RG_PORT_PON, IF_IN_MULTICAST_PKTS_INDEX, &value);
#endif
		*data = uintdup(value);
	}
	else if(strcmp(lastname, "SBroadcastPackets") == 0)
	{
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_stat_port_get(RTK_RG_PORT_PON, IF_OUT_BROADCAST_PKTS_CNT_INDEX, &value);
#else
		rtk_stat_port_get(RTK_RG_PORT_PON, IF_OUT_BROADCAST_PKTS_CNT_INDEX, &value);
#endif
		*data = uintdup(value);
	}
	else if(strcmp(lastname, "RBroadcastPackets") == 0)
	{
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_stat_port_get(RTK_RG_PORT_PON, IF_IN_BROADCAST_PKTS_INDEX, &value);
#else
		rtk_stat_port_get(RTK_RG_PORT_PON, IF_IN_BROADCAST_PKTS_INDEX, &value);
#endif
		*data = uintdup(value);
	}
	else if(strcmp(lastname, "FECError") == 0)
	{
		unsigned int mode;

		mib_get(MIB_PON_MODE, &mode);

		if(mode == GPON_MODE)
		{
			rtk_gpon_global_performance_type_t type;
			rtk_gpon_global_counter_t counter;

#if defined(CONFIG_RTK_L34_ENABLE)
			rtk_rg_gpon_globalCounter_get(RTK_GPON_PMTYPE_DS_PHY, &counter);
#else
			rtk_gpon_globalCounter_get(RTK_GPON_PMTYPE_DS_PHY, &counter);
#endif
			*data = uintdup(counter.dsphy.rx_fec_uncor_cw);
		}
		else
		{
			rtk_epon_counter_t counter;

#if defined(CONFIG_RTK_L34_ENABLE)
			rtk_rg_epon_mibCounter_get(&counter);
#else
			rtk_epon_mibCounter_get(&counter);
#endif

			*data = uintdup(counter.fecUncorrectedBlocks);
		}
	}
	else if(strcmp(lastname, "HECError") == 0)
	{
		unsigned int mode;

		mib_get(MIB_PON_MODE, &mode);

		if(mode == GPON_MODE)
		{
			rtk_gpon_global_performance_type_t type;
			rtk_gpon_global_counter_t counter;

#if defined(CONFIG_RTK_L34_ENABLE)
			rtk_rg_gpon_globalCounter_get(RTK_GPON_PMTYPE_DS_GEM, &counter);
#else
			rtk_gpon_globalCounter_get(RTK_GPON_PMTYPE_DS_GEM, &counter);
#endif
			*data = uintdup(counter.dsgem.rx_hec_correct);
		}
		else
		{
			// EPON has no HEC error
			*data = uintdup(0);
		}
	}
	else if(strcmp(lastname, "DropPackets") == 0)
	{
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_stat_port_get(RTK_RG_PORT_PON, IF_OUT_DISCARDS_INDEX, &value);
#else
		rtk_stat_port_get(RTK_RG_PORT_PON, IF_OUT_DISCARDS_INDEX, &value);
#endif
		*data = uintdup(value);
	}
	else if(strcmp(lastname, "SpausePackets") == 0)
	{
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_stat_port_get(RTK_RG_PORT_PON, DOT3_OUT_PAUSE_FRAMES_INDEX, &value);
#else
		rtk_stat_port_get(RTK_RG_PORT_PON, DOT3_OUT_PAUSE_FRAMES_INDEX, &value);
#endif
		*data = uintdup(value);
	}
	else if(strcmp(lastname, "RpausePackets") == 0)
	{
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_stat_port_get(RTK_RG_PORT_PON, DOT3_IN_PAUSE_FRAMES_INDEX, &value);
#else
		rtk_stat_port_get(RTK_RG_PORT_PON, DOT3_IN_PAUSE_FRAMES_INDEX, &value);
#endif
		*data = uintdup(value);
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}


/*****************************************************************************
 * IGD.WANDevice.{i}.WANConnectionDevice.{i}.X_CT-COM_WANGponLinkConfig.
 * IGD.WANDevice.{i}.WANConnectionDevice.{i}.X_CT-COM_WANEponLinkConfig.
 *****************************************************************************/
struct CWMP_OP tCT_XPONLinkConfLeafOP = { getCT_XPONLinkConf, setCT_XPONLinkConf };

struct CWMP_PRMT tCT_XPONLinkConfLeafInfo[] = {
/*(name,		type,		flag,			op)*/
{"Enable",		eCWMP_tBOOLEAN, CWMP_WRITE | CWMP_READ, &tCT_XPONLinkConfLeafOP},
{"Mode",			eCWMP_tUINT,		CWMP_WRITE | CWMP_READ, &tCT_XPONLinkConfLeafOP},
{"VLANIDMark",	eCWMP_tUINT,		CWMP_WRITE | CWMP_READ, &tCT_XPONLinkConfLeafOP},
};

enum eCT_XPONLinkConfLeaf
{
	eCTXPON_Enable,
	eCTXPON_Mode,
	eCTXPON_VLANIDMark,
};

struct CWMP_LEAF tCT_XPONLinkLeaf[] =
{
{ &tCT_XPONLinkConfLeafInfo[eCTXPON_Enable] },
{ &tCT_XPONLinkConfLeafInfo[eCTXPON_Mode] },
{ &tCT_XPONLinkConfLeafInfo[eCTXPON_VLANIDMark] },
{ NULL },
};

int getCT_XPONLinkConf(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;
	unsigned int wandevnum, devnum,num,i;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;

	if (name == NULL || entity == NULL || type == NULL || data == NULL)
		return -1;

	wandevnum = getWANDevInstNum(name);
	devnum = getWANConDevInstNum(name);
	if(devnum==0 || wandevnum ==0) return ERR_9005;

	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		pEntry = &vc_entity;
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)pEntry ))
			continue;

		if(!isWANDevModeMatch(wandevnum, pEntry->ifIndex))
			continue;

		if( pEntry->ConDevInstNum==devnum )
			break;
	}

	if(i==num) return ERR_9005;

	*type = entity->info->type;
	*data = NULL;
	if(strcmp(lastname, "Enable") == 0)
	{
		//TODO
		*data = booldup(1);
	}
	else if(strcmp(lastname, "Mode") == 0)
	{
		unsigned int mode;
		if(pEntry->vlan)
			mode= 2;
		else
		{
			if(pEntry->brmode == BRIDGE_ETHERNET)
				mode = 1;
			else
				mode = 0;
		}

		*data = uintdup(mode);
	}
	else if(strcmp(lastname, "VLANIDMark") == 0)
	{
			*data = uintdup(pEntry->vid);
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int setCT_XPONLinkConf(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;
	unsigned int wandevnum, devnum,num,i;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;
	int ret = CWMP_APPLIED;
	int found = 0;

	if (name == NULL || entity == NULL || data == NULL)
		return -1;

	if (entity->info->type != type)
		return ERR_9006;

	wandevnum = getWANDevInstNum(name);
	devnum = getWANConDevInstNum(name);
	if(devnum==0 || wandevnum ==0) return ERR_9005;

	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		pEntry = &vc_entity;
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)pEntry ))
			continue;

		if(!isWANDevModeMatch(wandevnum, pEntry->ifIndex))
			continue;

		if( pEntry->ConDevInstNum==devnum )
		{
			found = 1;
			gWanBitMap |= (1U << i);

			if (strcmp(lastname, "Enable") == 0)
			{
				int *i = data;

				if(*i == 1)
					return 0;

				return ERR_9007;
			}
			else if(strcmp(lastname, "Mode") == 0)
			{
				unsigned int *mode = data;

				switch(*mode)
				{
				case 1:
					pEntry->brmode = BRIDGE_ETHERNET;
				case 0:
					pEntry->vlan = 0;
					break;
				case 2:
					pEntry->vlan = 1;
					break;
				default:
					return ERR_9007;
				}

				mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, i );
				ret = CWMP_NEED_RESTART_WAN;
			}
			else if(strcmp(lastname, "VLANIDMark") == 0)
			{
				unsigned int *vlan = data;

				if(*vlan > 4095) return ERR_9007;

				pEntry->vid = *vlan;
				mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, i );

				ret = CWMP_NEED_RESTART_WAN;
			}
			else
			{
				return ERR_9005;
			}
		}
	}

	if(!found)
		return ERR_9005;

	return ret;
}
#endif



#ifdef _PRMT_X_CT_COM_DHCP_
/*****************************************************************************
 * IGD.WANDevice.{i}.WANConnectionDevice.{i}.WANIPConnection.{i}.X_CT-COM_DHCPOPTION60.
 *****************************************************************************/
struct CWMP_OP tCTDhcpOpt60EntityLeafOP = { getCTDhcpOpt60Entity, setCTDhcpOpt60Entity };
struct CWMP_PRMT tCTDhcpOpt60EntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCTDhcpOpt60EntityLeafOP},
{"Type",			eCWMP_tINT,		CWMP_WRITE|CWMP_READ,	&tCTDhcpOpt60EntityLeafOP},
{"ValueMode",		eCWMP_tINT,		CWMP_WRITE|CWMP_READ,	&tCTDhcpOpt60EntityLeafOP},
{"Value",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTDhcpOpt60EntityLeafOP},
};

enum eCTDhcpOpt60EntityLeaf
{
	eCTOpt60_Enable,
	eCTOpt60_Type,
	eCTOpt60_ValueMode,
	eCTOpt60_Value,
};

struct CWMP_LEAF tCTDhcpOpt60EntityLeaf[] =
{
{ &tCTDhcpOpt60EntityLeafInfo[eCTOpt60_Enable] },
{ &tCTDhcpOpt60EntityLeafInfo[eCTOpt60_Type] },
{ &tCTDhcpOpt60EntityLeafInfo[eCTOpt60_ValueMode] },
{ &tCTDhcpOpt60EntityLeafInfo[eCTOpt60_Value] },
{ NULL }
};

int getCTDhcpOpt60Entity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned int wandevnum, devnum,num,i, opt_num;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	wandevnum = getWANDevInstNum(name);
	devnum = getWANConDevInstNum(name);
	if(devnum==0 || wandevnum == 0) return ERR_9005;

	opt_num = getInstNum(name, "X_CT-COM_DHCPOPTION60");
	if(opt_num < 1 || opt_num > 4)
		return ERR_9005;

	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		pEntry = &vc_entity;
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)pEntry ))
			continue;

		if(!isWANDevModeMatch(wandevnum, pEntry->ifIndex))
			continue;

		if( pEntry->ConDevInstNum==devnum )
			break;
	}
	if(i==num) return ERR_9005;

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "Enable" )==0 )
	{
		*data = booldup(pEntry->dhcp_opt60_enable[opt_num - 1]);
	}
	else if( strcmp( lastname, "Type" )==0 )
	{
		*data = intdup(pEntry->dhcp_opt60_type[opt_num - 1]);
	}
	else if( strcmp( lastname, "ValueMode" )==0 )
	{
		*data = intdup(pEntry->dhcp_opt60_value_mode[opt_num - 1]);
	}
	else if( strcmp( lastname, "Value" )==0 )
	{
		*data = strdup(pEntry->dhcp_opt60_value[opt_num - 1]);
	}
	else{
		return ERR_9005;
	}

	return 0;
}


int setCTDhcpOpt60Entity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;
	unsigned int wandevnum, devnum,num,chainid, opt_num;
	int ret=0;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;
	if( data==NULL ) return ERR_9007;

	wandevnum = getWANDevInstNum(name);
	devnum = getWANConDevInstNum(name);
	if(devnum==0 || wandevnum==0) return ERR_9005;

	opt_num = getInstNum(name, "X_CT-COM_DHCPOPTION60");
	if(opt_num < 1 || opt_num > 4)
		return ERR_9005;

	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( chainid=0; chainid<num;chainid++ )
	{
		pEntry = &vc_entity;
		if( !mib_chain_get( MIB_ATM_VC_TBL, chainid, (void*)pEntry ))
			continue;

		if(!isWANDevModeMatch(wandevnum, pEntry->ifIndex))
			continue;

		if( pEntry->ConDevInstNum==devnum )
			break;
	}//for
	if(chainid==num) return ERR_9005;
	gWanBitMap |= (1U << chainid);

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;

		pEntry->dhcp_opt60_enable[opt_num - 1] = (*i == 0) ? 0 : 1;
		mib_chain_update(MIB_ATM_VC_TBL, pEntry, chainid);

		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "Type" )==0 )
	{
		int *i = data;

		if(*i < 0 || * i > 255)
			return ERR_9007;

		pEntry->dhcp_opt60_type[opt_num - 1] = *i;
		mib_chain_update(MIB_ATM_VC_TBL, pEntry, chainid);

		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "ValueMode" )==0 )
	{
		int *i = data;

		if(*i < 0 || *i > 2)
			return ERR_9007;

		pEntry->dhcp_opt60_value_mode[opt_num - 1] = *i;
		mib_chain_update(MIB_ATM_VC_TBL, pEntry, chainid);

		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "Value" )==0 )
	{
		char *buf = data;
		char len = strlen(buf);

		if(len == 0 || len >= 80) return ERR_9007;

		strcpy(pEntry->dhcp_opt60_value[opt_num - 1], buf);
		mib_chain_update(MIB_ATM_VC_TBL, pEntry, chainid);

		return CWMP_NEED_RESTART_WAN;
	}
	else
	{
		return ERR_9005;
	}

	return ret;
}



struct CWMP_PRMT tCTDhcpOpt60OjbectInfo[] =
{
/*(name,	type,		flag,			op)*/
{"1",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"2",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"3",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"4",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum eCTDhcpOpt60Ojbect
{
	eCTDhcpOpt60_1,
	eCTDhcpOpt60_2,
	eCTDhcpOpt60_3,
	eCTDhcpOpt60_4,
};

struct CWMP_NODE tCTDhcpOpt60Object[] =
{
/*info,  				leaf,			next)*/
{&tCTDhcpOpt60OjbectInfo[eCTDhcpOpt60_1],	tCTDhcpOpt60EntityLeaf,	NULL},
{&tCTDhcpOpt60OjbectInfo[eCTDhcpOpt60_2],	tCTDhcpOpt60EntityLeaf,	NULL},
{&tCTDhcpOpt60OjbectInfo[eCTDhcpOpt60_3],	tCTDhcpOpt60EntityLeaf,	NULL},
{&tCTDhcpOpt60OjbectInfo[eCTDhcpOpt60_4],	tCTDhcpOpt60EntityLeaf,	NULL},
{NULL,						NULL,			NULL}
};



/*****************************************************************************
 * IGD.WANDevice.{i}.WANConnectionDevice.{i}.WANIPConnection.{i}.X_CT-COM_DHCPOPTION125.
 *****************************************************************************/
struct CWMP_OP tCTDhcpOpt125EntityLeafOP = { getCTDhcpOpt125Entity, setCTDhcpOpt125Entity };
struct CWMP_PRMT tCTDhcpOpt125EntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCTDhcpOpt125EntityLeafOP},
{"Type",				eCWMP_tINT,		CWMP_WRITE|CWMP_READ,	&tCTDhcpOpt125EntityLeafOP},
{"subopt-code",		eCWMP_tINT,		CWMP_WRITE|CWMP_READ,	&tCTDhcpOpt125EntityLeafOP},
{"sub-option-data", 	eCWMP_tSTRING, 	CWMP_WRITE|CWMP_READ,	&tCTDhcpOpt125EntityLeafOP},
{"Value",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTDhcpOpt125EntityLeafOP},
};

enum eCTDhcpOpt125EntityLeaf
{
	eCTOpt125_Enable,
	eCTOpt125_Type,
	eCTOpt125_subopt_code,
	eCTOpt125_sub_option_data,
	eCTOpt125_Value,
};

struct CWMP_LEAF tCTDhcpOpt125EntityLeaf[] =
{
{ &tCTDhcpOpt125EntityLeafInfo[eCTOpt125_Enable] },
{ &tCTDhcpOpt125EntityLeafInfo[eCTOpt125_Type] },
{ &tCTDhcpOpt125EntityLeafInfo[eCTOpt125_subopt_code] },
{ &tCTDhcpOpt125EntityLeafInfo[eCTOpt125_sub_option_data] },
{ &tCTDhcpOpt125EntityLeafInfo[eCTOpt125_Value] },
{ NULL }
};

int getCTDhcpOpt125Entity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned int wandevnum, devnum,num,i, opt_num;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	wandevnum = getWANDevInstNum(name);
	devnum = getWANConDevInstNum(name);
	if(devnum==0 || wandevnum == 0) return ERR_9005;

	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		pEntry = &vc_entity;
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)pEntry ))
			continue;

		if(!isWANDevModeMatch(wandevnum, pEntry->ifIndex))
			continue;

		if( pEntry->ConDevInstNum==devnum )
			break;
	}
	if(i==num) return ERR_9005;

	opt_num = getInstNum(name, "X_CT-COM_DHCPOPTION125");
	if(opt_num < 1 || opt_num > 4)
		return ERR_9005;

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "Enable" )==0 )
	{
		*data = booldup(pEntry->dhcp_opt125_enable[opt_num-1]);
	}
	else if( strcmp( lastname, "Type" )==0 )
	{
		*data = intdup(pEntry->dhcp_opt125_type[opt_num-1]);
	}
	else if( strcmp( lastname, "subopt-code" )==0 )
	{
		*data = intdup(pEntry->dhcp_opt125_sub_code[opt_num-1]);
	}
	else if( strcmp( lastname, "sub-option-data" )==0 )
	{
		*data = strdup(pEntry->dhcp_opt125_sub_data[opt_num-1]);
	}
	else if( strcmp( lastname, "Value" )==0 )
	{
		*data = strdup(pEntry->dhcp_opt125_value[opt_num-1]);
	}
	else{
		return ERR_9005;
	}

	return 0;
}

int setCTDhcpOpt125Entity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;
	unsigned int wandevnum, devnum,num,chainid, opt_num;
	int ret=0;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;
	if( data==NULL ) return ERR_9007;

	wandevnum = getWANDevInstNum(name);
	devnum = getWANConDevInstNum(name);
	if(devnum==0 || wandevnum==0) return ERR_9005;

	opt_num = getInstNum(name, "X_CT-COM_DHCPOPTION125");
	if(opt_num < 1 || opt_num > 4)
		return ERR_9005;

	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( chainid=0; chainid<num;chainid++ )
	{
		pEntry = &vc_entity;
		if( !mib_chain_get( MIB_ATM_VC_TBL, chainid, (void*)pEntry ))
			continue;

		if(!isWANDevModeMatch(wandevnum, pEntry->ifIndex))
			continue;

		if( pEntry->ConDevInstNum==devnum )
			break;
	}//for
	if(chainid==num) return ERR_9005;
	gWanBitMap |= (1U << chainid);

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;

		pEntry->dhcp_opt125_enable[opt_num - 1] = (*i == 0) ? 0 : 1;
		mib_chain_update(MIB_ATM_VC_TBL, pEntry, chainid);

		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "Type" )==0 )
	{
		int *i = data;

		if(*i != 1 && *i != 2)
			return ERR_9007;

		pEntry->dhcp_opt125_type[opt_num - 1] = *i;
		mib_chain_update(MIB_ATM_VC_TBL, pEntry, chainid);

		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "subopt-code" )==0 )
	{
		int *i = data;

		if(*i < 0 && *i > 255)
			return ERR_9007;

		pEntry->dhcp_opt125_sub_code[opt_num - 1] = *i;
		mib_chain_update(MIB_ATM_VC_TBL, pEntry, chainid);

		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "sub-option-data" )==0 )
	{
		char *buf = data;
		int len = strlen(buf);

		if(len == 0 || len >= 36) return ERR_9007;

		strcpy(pEntry->dhcp_opt125_sub_data[opt_num -1], buf);
		mib_chain_update(MIB_ATM_VC_TBL, pEntry, chainid);

		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "Value" )==0 )
	{
		char *buf = data;
		int len = strlen(buf);

		if(len == 0 || len >= 36) return ERR_9007;

		strcpy(pEntry->dhcp_opt125_value[opt_num -1], buf);
		mib_chain_update(MIB_ATM_VC_TBL, pEntry, chainid);

		return CWMP_NEED_RESTART_WAN;
	}
	else
	{
		return ERR_9005;
	}

	return ret;
}


struct CWMP_PRMT tCTDhcpOpt125OjbectInfo[] =
{
/*(name,	type,		flag,			op)*/
{"1",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"2",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"3",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"4",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum eCTDhcpOpt125Ojbect
{
	eCTDhcpOpt125_1,
	eCTDhcpOpt125_2,
	eCTDhcpOpt125_3,
	eCTDhcpOpt125_4,
};

struct CWMP_NODE tCTDhcpOpt125Object[] =
{
/*info,  				leaf,			next)*/
{&tCTDhcpOpt125OjbectInfo[eCTDhcpOpt125_1],	tCTDhcpOpt125EntityLeaf,	NULL},
{&tCTDhcpOpt125OjbectInfo[eCTDhcpOpt125_2],	tCTDhcpOpt125EntityLeaf,	NULL},
{&tCTDhcpOpt125OjbectInfo[eCTDhcpOpt125_3],	tCTDhcpOpt125EntityLeaf,	NULL},
{&tCTDhcpOpt125OjbectInfo[eCTDhcpOpt125_4],	tCTDhcpOpt125EntityLeaf,	NULL},
{NULL,						NULL,			NULL}
};




/*****************************************************************************
 * IGD.WANDevice.{i}.WANConnectionDevice.{i}.WANIPConnection.{i}.X_CT-COM_DHCPv6OPTION16.
 *****************************************************************************/
struct CWMP_OP tCTDhcpv6Opt16EntityLeafOP = { getCTDhcpv6Opt16Entity, setCTDhcpv6Opt16Entity };
struct CWMP_PRMT tCTDhcpv6Opt16EntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCTDhcpv6Opt16EntityLeafOP},
{"Type",			eCWMP_tINT,		CWMP_WRITE|CWMP_READ,	&tCTDhcpv6Opt16EntityLeafOP},
{"ValueMode",		eCWMP_tINT,		CWMP_WRITE|CWMP_READ,	&tCTDhcpv6Opt16EntityLeafOP},
{"Value",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTDhcpv6Opt16EntityLeafOP},
};

enum eCTDhcpv6Opt16EntityLeaf
{
	eCTOpt16_Enable,
	eCTOpt16_Type,
	eCTOpt16_ValueMode,
	eCTOpt16_Value,
};

struct CWMP_LEAF tCTDhcpv6Opt16EntityLeaf[] =
{
{ &tCTDhcpv6Opt16EntityLeafInfo[eCTOpt16_Enable] },
{ &tCTDhcpv6Opt16EntityLeafInfo[eCTOpt16_Type] },
{ &tCTDhcpv6Opt16EntityLeafInfo[eCTOpt16_ValueMode] },
{ &tCTDhcpv6Opt16EntityLeafInfo[eCTOpt16_Value] },
{ NULL }
};

int getCTDhcpv6Opt16Entity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned int wandevnum, devnum,num,i, opt_num;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	wandevnum = getWANDevInstNum(name);
	devnum = getWANConDevInstNum(name);
	if(devnum==0 || wandevnum == 0) return ERR_9005;

	opt_num = getInstNum(name, "X_CT-COM_DHCPv6OPTION16");
	if(opt_num < 1 || opt_num > 4)
		return ERR_9005;

	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		pEntry = &vc_entity;
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)pEntry ))
			continue;

		if(!isWANDevModeMatch(wandevnum, pEntry->ifIndex))
			continue;

		if( pEntry->ConDevInstNum==devnum )
			break;
	}
	if(i==num) return ERR_9005;

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "Enable" )==0 )
	{
		*data = booldup(pEntry->dhcpv6_opt16_enable[opt_num - 1]);
	}
	else if( strcmp( lastname, "Type" )==0 )
	{
		*data = intdup(pEntry->dhcpv6_opt16_type[opt_num - 1]);
	}
	else if( strcmp( lastname, "ValueMode" )==0 )
	{
		*data = intdup(pEntry->dhcpv6_opt16_value_mode[opt_num - 1]);
	}
	else if( strcmp( lastname, "Value" )==0 )
	{
		*data = strdup(pEntry->dhcpv6_opt16_value[opt_num - 1]);
	}
	else{
		return ERR_9005;
	}

	return 0;
}

int setCTDhcpv6Opt16Entity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;
	unsigned int wandevnum, devnum,num,chainid, opt_num;
	int ret=0;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;
	if( data==NULL ) return ERR_9007;

	wandevnum = getWANDevInstNum(name);
	devnum = getWANConDevInstNum(name);
	if(devnum==0 || wandevnum==0) return ERR_9005;

	opt_num = getInstNum(name, "X_CT-COM_DHCPv6OPTION16");
	if(opt_num < 1 || opt_num > 4)
		return ERR_9005;

	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( chainid=0; chainid<num;chainid++ )
	{
		pEntry = &vc_entity;
		if( !mib_chain_get( MIB_ATM_VC_TBL, chainid, (void*)pEntry ))
			continue;

		if(!isWANDevModeMatch(wandevnum, pEntry->ifIndex))
			continue;

		if( pEntry->ConDevInstNum==devnum )
			break;
	}//for
	if(chainid==num) return ERR_9005;
	gWanBitMap |= (1U << chainid);

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;

		pEntry->dhcpv6_opt16_enable[opt_num - 1] = (*i == 0) ? 0 : 1;
		mib_chain_update(MIB_ATM_VC_TBL, pEntry, chainid);

		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "Type" )==0 )
	{
		int *i = data;

		if(*i < 0 || *i > 255)
			return ERR_9007;

		pEntry->dhcpv6_opt16_type[opt_num - 1] = *i;
		mib_chain_update(MIB_ATM_VC_TBL, pEntry, chainid);

		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "ValueMode" )==0 )
	{
		int *i = data;

		if(*i < 0 || *i > 2)
			return ERR_9007;

		pEntry->dhcpv6_opt16_value_mode[opt_num - 1] = *i;
		mib_chain_update(MIB_ATM_VC_TBL, pEntry, chainid);

		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "Value" )==0 )
	{
		char *buf = data;
		char len = strlen(buf);

		if(len == 0 || len >= 80) return ERR_9007;

		strcpy(pEntry->dhcpv6_opt16_value[opt_num - 1], buf);
		mib_chain_update(MIB_ATM_VC_TBL, pEntry, chainid);

		return CWMP_NEED_RESTART_WAN;
	}
	else
	{
		return ERR_9005;
	}

	return ret;
}


struct CWMP_PRMT tCTDhcpv6Opt16OjbectInfo[] =
{
/*(name,	type,		flag,			op)*/
{"1",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"2",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"3",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"4",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum eCTDhcpv6Opt16Ojbect
{
	eCTDhcpv6Opt16_1,
	eCTDhcpv6Opt16_2,
	eCTDhcpv6Opt16_3,
	eCTDhcpv6Opt16_4,
};

struct CWMP_NODE tCTDhcpv6Opt16Object[] =
{
/*info,  				leaf,			next)*/
{&tCTDhcpv6Opt16OjbectInfo[eCTDhcpv6Opt16_1],	tCTDhcpv6Opt16EntityLeaf,	NULL},
{&tCTDhcpv6Opt16OjbectInfo[eCTDhcpv6Opt16_2],	tCTDhcpv6Opt16EntityLeaf,	NULL},
{&tCTDhcpv6Opt16OjbectInfo[eCTDhcpv6Opt16_3],	tCTDhcpv6Opt16EntityLeaf,	NULL},
{&tCTDhcpv6Opt16OjbectInfo[eCTDhcpv6Opt16_4],	tCTDhcpv6Opt16EntityLeaf,	NULL},
{NULL,						NULL,			NULL}
};



/*****************************************************************************
 * IGD.WANDevice.{i}.WANConnectionDevice.{i}.WANIPConnection.{i}.X_CT-COM_DHCPv6OPTION17.
 *****************************************************************************/
struct CWMP_OP tCTDhcpv6Opt17EntityLeafOP = { getCTDhcpv6Opt17Entity, setCTDhcpv6Opt17Entity };
struct CWMP_PRMT tCTDhcpv6Opt17EntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCTDhcpv6Opt17EntityLeafOP},
{"Type",				eCWMP_tINT,		CWMP_WRITE|CWMP_READ,	&tCTDhcpv6Opt17EntityLeafOP},
{"subopt-code",		eCWMP_tINT,		CWMP_WRITE|CWMP_READ,	&tCTDhcpv6Opt17EntityLeafOP},
{"sub-option-data", 	eCWMP_tSTRING, 	CWMP_WRITE|CWMP_READ,	&tCTDhcpv6Opt17EntityLeafOP},
{"Value",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTDhcpv6Opt17EntityLeafOP},
};

enum eCTDhcpv6Opt17EntityLeaf
{
	eCTOpt17_Enable,
	eCTOpt17_Type,
	eCTOpt17_subopt_code,
	eCTOpt17_sub_option_data,
	eCTOpt17_Value,
};

struct CWMP_LEAF tCTDhcpv6Opt17EntityLeaf[] =
{
{ &tCTDhcpv6Opt17EntityLeafInfo[eCTOpt17_Enable] },
{ &tCTDhcpv6Opt17EntityLeafInfo[eCTOpt17_Type] },
{ &tCTDhcpv6Opt17EntityLeafInfo[eCTOpt17_subopt_code] },
{ &tCTDhcpv6Opt17EntityLeafInfo[eCTOpt17_sub_option_data] },
{ &tCTDhcpv6Opt17EntityLeafInfo[eCTOpt17_Value] },
{ NULL }
};

int getCTDhcpv6Opt17Entity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned int wandevnum, devnum,num,i, opt_num;;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	wandevnum = getWANDevInstNum(name);
	devnum = getWANConDevInstNum(name);
	if(devnum==0 || wandevnum == 0) return ERR_9005;

	opt_num = getInstNum(name, "X_CT-COM_DHCPv6OPTION17");
	if(opt_num < 1 || opt_num > 4)
		return ERR_9005;

	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		pEntry = &vc_entity;
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)pEntry ))
			continue;

		if(!isWANDevModeMatch(wandevnum, pEntry->ifIndex))
			continue;

		if( pEntry->ConDevInstNum==devnum )
			break;
	}
	if(i==num) return ERR_9005;

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "Enable" )==0 )
	{
		*data = booldup(pEntry->dhcpv6_opt17_enable[opt_num-1]);
	}
	else if( strcmp( lastname, "Type" )==0 )
	{
		*data = intdup(pEntry->dhcpv6_opt17_type[opt_num-1]);
	}
	else if( strcmp( lastname, "subopt-code" )==0 )
	{
		*data = intdup(pEntry->dhcpv6_opt17_sub_code[opt_num-1]);
	}
	else if( strcmp( lastname, "sub-option-data" )==0 )
	{
		*data = strdup(pEntry->dhcpv6_opt17_sub_data[opt_num-1]);
	}
	else if( strcmp( lastname, "Value" )==0 )
	{
		*data = strdup(pEntry->dhcpv6_opt17_value[opt_num-1]);
	}
	else{
		return ERR_9005;
	}

	return 0;
}

int setCTDhcpv6Opt17Entity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;
	unsigned int wandevnum, devnum,num,chainid, opt_num;
	int ret=0;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;
	if( data==NULL ) return ERR_9007;

	wandevnum = getWANDevInstNum(name);
	devnum = getWANConDevInstNum(name);
	if(devnum==0 || wandevnum==0) return ERR_9005;

	opt_num = getInstNum(name, "X_CT-COM_DHCPv6OPTION17");
	if(opt_num < 1 || opt_num > 4)
		return ERR_9005;

	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( chainid=0; chainid<num;chainid++ )
	{
		pEntry = &vc_entity;
		if( !mib_chain_get( MIB_ATM_VC_TBL, chainid, (void*)pEntry ))
			continue;

		if(!isWANDevModeMatch(wandevnum, pEntry->ifIndex))
			continue;

		if( pEntry->ConDevInstNum==devnum )
			break;
	}//for
	if(chainid==num) return ERR_9005;
	gWanBitMap |= (1U << chainid);

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;

		pEntry->dhcpv6_opt17_enable[opt_num - 1] = (*i == 0) ? 0 : 1;
		mib_chain_update(MIB_ATM_VC_TBL, pEntry, chainid);

		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "Type" )==0 )
	{
		int *i = data;

		if(*i != 1 && *i != 2)
			return ERR_9007;

		pEntry->dhcpv6_opt17_type[opt_num - 1] = *i;
		mib_chain_update(MIB_ATM_VC_TBL, pEntry, chainid);

		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "subopt-code" )==0 )
	{
		int *i = data;

		if(*i < 0 || *i > 255)
			return ERR_9007;

		pEntry->dhcpv6_opt17_sub_code[opt_num - 1] = *i;
		mib_chain_update(MIB_ATM_VC_TBL, pEntry, chainid);

		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "sub-option-data" )==0 )
	{
		char *buf = data;
		int len = strlen(buf);

		if(len == 0 || len >= 36) return ERR_9007;

		strcpy(pEntry->dhcpv6_opt17_sub_data[opt_num -1], buf);
		mib_chain_update(MIB_ATM_VC_TBL, pEntry, chainid);

		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "Value" )==0 )
	{
		char *buf = data;
		int len = strlen(buf);

		if(len == 0 || len >= 36) return ERR_9007;

		strcpy(pEntry->dhcpv6_opt17_value[opt_num -1], buf);
		mib_chain_update(MIB_ATM_VC_TBL, pEntry, chainid);

		return CWMP_NEED_RESTART_WAN;
	}
	else
	{
		return ERR_9005;
	}

	return ret;
}


struct CWMP_PRMT tCTDhcpv6Opt17OjbectInfo[] =
{
/*(name,	type,		flag,			op)*/
{"1",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"2",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"3",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"4",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum eCTDhcpv6Opt17Ojbect
{
	eCTDhcpv6Opt17_1,
	eCTDhcpv6Opt17_2,
	eCTDhcpv6Opt17_3,
	eCTDhcpv6Opt17_4,
};

struct CWMP_NODE tCTDhcpv6Opt17Object[] =
{
/*info,  				leaf,			next)*/
{&tCTDhcpv6Opt17OjbectInfo[eCTDhcpv6Opt17_1],	tCTDhcpv6Opt17EntityLeaf,	NULL},
{&tCTDhcpv6Opt17OjbectInfo[eCTDhcpv6Opt17_2],	tCTDhcpv6Opt17EntityLeaf,	NULL},
{&tCTDhcpv6Opt17OjbectInfo[eCTDhcpv6Opt17_3],	tCTDhcpv6Opt17EntityLeaf,	NULL},
{&tCTDhcpv6Opt17OjbectInfo[eCTDhcpv6Opt17_4],	tCTDhcpv6Opt17EntityLeaf,	NULL},
{NULL,						NULL,			NULL}
};

#endif	//_PRMT_X_CT_COM_DHCP_

