#include "prmt_igd.h"
#include "prmt_deviceinfo.h"
#include "prmt_mngmtserver.h"
#include "prmt_lanconfsec.h"
#ifdef _SUPPORT_L2BRIDGING_PROFILE_
#include "prmt_layer2br.h"
#endif
#include "prmt_layer3fw.h"
#include "prmt_landevice.h"
#include "prmt_ippingdiag.h"
#include "prmt_wandevice.h"
#ifdef TIME_ZONE
#include "prmt_time.h"
#endif
#if defined( _PRMT_SERVICES_)
#include "prmt_services.h"
#endif
#ifdef _PRMT_USERINTERFACE_
#include "prmt_userif.h"
#endif
#ifdef _PRMT_X_CT_COM_PROXY_DEV_
#include "prmt_ctcom_proxy_dev.h"
#endif
#ifdef CTC_DNS_SPEED_LIMIT
#include "prmt_ctcom.h"
#endif
#ifdef _PRMT_X_CT_COM_QOS_
#include "prmt_ctcom_queuemnt.h"
#elif defined(_PRMT_X_STD_QOS_)
#include "prmt_ctcom_queuemnt.h"
#elif defined(IP_QOS)
#include "prmt_queuemnt.h"
#elif defined(CONFIG_USER_IP_QOS_3)
#include "prmt_queuemnt_3.h"
#endif

#ifdef _PRMT_TR143_
#include "prmt_tr143.h"
#endif //_PRMT_TR143_
#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
#include "prmt_captiveportal.h"
#endif //_SUPPORT_CAPTIVEPORTAL_PROFILE_
#ifdef _SUPPORT_TRACEROUTE_PROFILE_
#include "prmt_traceroute.h"
#endif //_SUPPORT_TRACEROUTE_PROFILE_
#ifdef CONFIG_BHS
#include "prmt_telefonica_ipv6.h"
#endif
#ifdef _PRMT_NSLOOKUP_
#include "prmt_nslookup.h"
#endif

#ifdef CONFIG_MIDDLEWARE
#include <rtk/options.h>
#endif

#define LANDEVICE_NUM 1        /* one instance of LANDevice*/

/******DEVICESUMMARY*****************************************************************************/
//baseline profile
#ifdef _PRMT_WT107_
#define DS_PREFIX	"InternetGatewayDevice:1.4[](Baseline:2"
#else
#define DS_PREFIX	"InternetGatewayDevice:1.1[](Baseline:1"
#endif //_PRMT_WT107_

//ethernetlan profile
#ifdef _PRMT_WT107_
#define DS_ETH		",EthernetLAN:2"
#else
#define DS_ETH		",EthernetLAN:1"
#endif //_PRMT_WT107_

//usblan profile
#ifdef _PRMT_USB_ETH_
#ifdef _PRMT_WT107_
#define DS_USB		",USBLAN:2"
#else
#define DS_USB		",USBLAN:1"
#endif //_PRMT_WT107_
#else
#define DS_USB		""
#endif //_PRMT_USB_ETH_

//wifilan profile
#ifdef WLAN_SUPPORT
#ifdef _PRMT_WT107_
#define DS_WIFI		",WiFiLAN:2"
#else
#define DS_WIFI		",WiFiLAN:1"
#endif //_PRMT_WT107_
#else
#define DS_WIFI		""
#endif //WLAN_SUPPORT

//wifiwmm profile
#ifdef _SUPPORT_WIFIWMM_PROFILE_
#define DS_WIFIWMM		",WiFiWMM:1"
#else
#define DS_WIFIWMM		""
#endif //_SUPPORT_WIFIWMM_PROFILE_

//wifiwps profile
#ifdef CONFIG_WIFI_SIMPLE_CONFIG
#define DS_WIFIWPS		",WiFiWPS:1"
#else
#define DS_WIFIWPS		""
#endif //_SUPPORT_WIFIWPS_PROFILE_

//adslwan profile
#define DS_ADSLWAN		",ADSLWAN:1"

//adsl2wan profile
#ifdef _SUPPORT_ADSL2WAN_PROFILE_
#define DS_ADSL2WAN		",ADSL2WAN:1"
#else
#define DS_ADSL2WAN		""
#endif //_PRMT_WT107_

//qos profile
#ifdef IP_QOS
#ifdef _PRMT_WT107_
#define DS_QOS		",QoS:2"
#else
#define DS_QOS		",QoS:1"
#endif //_PRMT_WT107_
#else
#define DS_QOS		""
#endif //IP_QOS

//time profile
#ifdef TIME_ZONE
#ifdef _PRMT_WT107_
#define DS_TIME		",Time:2"
#else
#define DS_TIME		",Time:1"
#endif //_PRMT_WT107_
#else
#define DS_TIME		""
#endif

//captiveportal profile
#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
#define DS_CAPTIVEPORTAL	",CaptivePortal:1"
#else
#define DS_CAPTIVEPORTAL	""
#endif //_SUPPORT_CAPTIVEPORTAL_PROFILE_

//ipping profile
#define DS_PING		",IPPing:1"

//traceroute profile
#ifdef _SUPPORT_TRACEROUTE_PROFILE_
#define DS_TRACEROUTE	",TraceRoute:1"
#else
#define DS_TRACEROUTE	""
#endif //_SUPPORT_TRACEROUTE_PROFILE_

//download,downloadtcp,upload,uploadtcp,udpecho,udpechoplus profiles
#ifdef _PRMT_TR143_
#define DS_TR143	",Download:1,DownloadTCP:1,Upload:1,UploadTCP:1,UDPEcho:1,UDPEchoPlus:1"
#else
#define DS_TR143	""
#endif //_PRMT_TR143_

//atmloopback profile
#define DS_ATMLB	",ATMLoopback:1"

//dsldiagnostics profile
#define DS_DSLDIAG	",DSLDiagnostics:1"

//adsl2dsldiagnostics profile
#ifdef _SUPPORT_ADSL2DSLDIAG_PROFILE_
#define DS_DSL2DIAG	",ADSL2DSLDiagnostics:1"
#else
#define DS_DSL2DIAG	""
#endif //_SUPPORT_ADSL2DSLDIAG_PROFILE_

//deviceassociation profile
#ifdef _TR_111_PRMT_
#ifdef _PRMT_WT107_
#define DS_TR111	",DeviceAssociation:2"
#else
#define DS_TR111	",DeviceAssociation:1"
#endif //_PRMT_WT107_
#else
#define DS_TR111	""
#endif //_TR_111_PRMT_

//dhcpcondserving profile
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
#define DS_DHCPCONDSERV	",DHCPCondServing:1"
#else
#define DS_DHCPCONDSERV	""
#endif //_PRMT_X_TELEFONICA_ES_DHCPOPTION_

//dhcpoption profile
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
#define DS_DHCPOPTION	",DHCPOption:1"
#else
#define DS_DHCPOPTION	""
#endif //_PRMT_X_TELEFONICA_ES_DHCPOPTION_

#ifdef CONFIG_VDSL
#define DS_VDSL2WAN		",VDSL2WAN:1"
#define DS_VDSL2DIAG	",VDSL2DSLDiagnostics:1"
#else
#define DS_VDSL2WAN		""
#define DS_VDSL2DIAG	""
#endif /*CONFIG_VDSL*/

#ifdef CONFIG_PTMWAN
#define DS_PTMWAN		"PTMWAN:1"
#else
#define DS_PTMWAN		""
#endif /*CONFIG_PTMWAN*/


#define DS_END		")"
#define DEVICESUMMARY	DS_PREFIX DS_ETH DS_USB DS_WIFI DS_WIFIWMM DS_WIFIWPS \
			DS_ADSLWAN DS_ADSL2WAN DS_QOS DS_TIME DS_CAPTIVEPORTAL \
			DS_PING DS_TRACEROUTE DS_TR143 DS_ATMLB DS_DSLDIAG \
			DS_DSL2DIAG DS_TR111 DS_DHCPCONDSERV DS_DHCPOPTION \
			DS_VDSL2WAN DS_VDSL2DIAG DS_PTMWAN \
			DS_END
/*******end DEVICESUMMARY****************************************************************************/

#ifdef _PRMT_CAPABILITIES_
/******Capabilities**********************************************************************************/
struct CWMP_PRMT tCapabilitiesObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
#ifdef _PRMT_TR143_
{"PerformanceDiagnostic",	eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif //_PRMT_TR143_
};
enum eCapabilitiesObject
{
#ifdef _PRMT_TR143_
	ePerformanceDiagnostic,
#endif //_PRMT_TR143_
	eCapabilities_END /*the last one*/
};
struct CWMP_NODE tCapabilitiesObject[] =
{
/*info,  						leaf,				node)*/
#ifdef _PRMT_TR143_
{&tCapabilitiesObjectInfo[ePerformanceDiagnostic],	tPerformanceDiagnosticLeaf,	NULL},
#endif //_PRMT_TR143_
{NULL,							NULL,				NULL}
};
#endif //_PRMT_CAPABILITIES_

/******IGD*****************************************************************************************/
struct CWMP_OP tIGDLeafOP = { getIGD, NULL };
struct CWMP_PRMT tIGDLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"LANDeviceNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,		&tIGDLeafOP},
{"WANDeviceNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,		&tIGDLeafOP},
{"DeviceSummary",		eCWMP_tSTRING,	CWMP_READ,		&tIGDLeafOP},
};
enum eIGDLeaf
{
	eLANDeviceNumberOfEntries,
	eWANDeviceNumberOfEntries,
	eDeviceSummary
};
struct CWMP_LEAF tIGDLeaf[] =
{
{ &tIGDLeafInfo[eLANDeviceNumberOfEntries]  },
{ &tIGDLeafInfo[eWANDeviceNumberOfEntries]  },
{ &tIGDLeafInfo[eDeviceSummary]  },
{ NULL	}
};

struct CWMP_OP tWAN_WANDevice_OP = { NULL, objWANDev };
#ifdef CONFIG_BHS
struct CWMP_OP tIPTunnelOP = { NULL, objIPTunnel };
#endif

struct CWMP_PRMT tIGDObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"DeviceInfo",			eCWMP_tOBJECT,	CWMP_READ,	NULL},
#ifdef _PRMT_DEVICECONFIG_
{"DeviceConfig",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif //_PRMT_DEVICECONFIG_
{"ManagementServer",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#ifdef TIME_ZONE
{"Time",			eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif /*TIME_ZONE*/
#ifdef _PRMT_USERINTERFACE_
{"UserInterface",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif /*_PRMT_USERINTERFACE_*/
#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
{"CaptivePortal",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif //_SUPPORT_CAPTIVEPORTAL_PROFILE_
#ifdef _SUPPORT_L2BRIDGING_PROFILE_
{"Layer2Bridging",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif
{"Layer3Forwarding",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#if (defined(CONFIG_E8B) && defined(_PRMT_X_STD_QOS_)) \
 	|| defined(IP_QOS) \
 	|| (!defined(CONFIG_E8B) && defined(CONFIG_USER_IP_QOS_3))
{"QueueManagement",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif
{"LANConfigSecurity",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"IPPingDiagnostics",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#ifdef _SUPPORT_TRACEROUTE_PROFILE_
{"TraceRouteDiagnostics",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif //_SUPPORT_TRACEROUTE_PROFILE_
{"LANDevice",			eCWMP_tOBJECT,	CWMP_READ,	NULL},
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{"LANInterfaces",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif
/*ping_zhang:20081217 END*/
{"WANDevice",			eCWMP_tOBJECT, CWMP_READ/*|CWMP_WRITE*/, &tWAN_WANDevice_OP},
#ifdef _PRMT_SERVICES_
{"Services",			eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif //_PRMT_SERVICES_
#ifdef _PRMT_CAPABILITIES_
{"Capabilities",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif //_PRMT_CAPABILITIES_
#ifdef _PRMT_TR143_
{"DownloadDiagnostics",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"UploadDiagnostics",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"UDPEchoConfig",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif //_PRMT_TR143_
#ifdef _PRMT_X_CT_COM_QOS_
{"X_CT-COM_UplinkQoS",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif
#ifdef _PRMT_X_CT_COM_USERINFO_
{"X_CT-COM_UserInfo",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif
#ifdef _PRMT_X_CT_COM_PROXY_DEV_
{"X_CT-COM_ProxyDevice",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif
#ifdef CTC_DNS_SPEED_LIMIT
{"X_CT-COM_DNSSpeedLimit",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif
#ifdef CONFIG_BHS
{"X_TELEFONICA-ES_IPTunnel",	eCWMP_tOBJECT,	CWMP_READ | CWMP_WRITE,	&tIPTunnelOP},
{"X_TELEFONICA-ES_IPv6Layer3Forwarding",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif
#ifdef _PRMT_NSLOOKUP_
{"NSLookupDiagnostics", eCWMP_tOBJECT, CWMP_READ, NULL},
#endif
};
enum eIGDObject
{
	eDeviceInfo,
#ifdef _PRMT_DEVICECONFIG_
	eDeviceConfig,
#endif //_PRMT_DEVICECONFIG_
	eManagementServer,
#ifdef TIME_ZONE
	eTime,
#endif //TIME_ZONE
#ifdef _PRMT_USERINTERFACE_
	eUserInterface,
#endif /*_PRMT_USERINTERFACE_*/
#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
	eCaptivePortal,
#endif //_SUPPORT_CAPTIVEPORTAL_PROFILE_
#ifdef _SUPPORT_L2BRIDGING_PROFILE_
	eLayer2Bridging,
#endif
	eLayer3Forwarding,
#if (defined(CONFIG_E8B) && defined(_PRMT_X_STD_QOS_)) \
		|| defined(IP_QOS) \
		|| (!defined(CONFIG_E8B) && defined(CONFIG_USER_IP_QOS_3))
	eQueueManagement,
#endif //defined(_PRMT_X_STD_QOS_) || defined(IP_QOS)
	eLANConfigSecurity,
	eIPPingDiagnostics,
#ifdef _SUPPORT_TRACEROUTE_PROFILE_
	eTraceRouteDiagnostics,
#endif //_SUPPORT_TRACEROUTE_PROFILE_
	eLANDevice,
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	eLANInterfaces,
#endif
/*ping_zhang:20081217 END*/
	eWANDevice,
#ifdef _PRMT_SERVICES_
	eServices,
#endif //_PRMT_SERVICES_
#ifdef _PRMT_CAPABILITIES_
	eCapabilities,
#endif //_PRMT_CAPABILITIES_
#ifdef _PRMT_TR143_
	eDownloadDiagnostics,
	eUploadDiagnostics,
	eUDPEchoConfig,
#endif //_PRMT_TR143_
#ifdef _PRMT_X_CT_COM_QOS_
	eX_CT_COM_UplinkQoS,
#endif
#ifdef _PRMT_X_CT_COM_USERINFO_
	eX_CT_COM_UserInfo,
#endif
#ifdef _PRMT_X_CT_COM_PROXY_DEV_
	eX_CT_COM_ProxyDevice,
#endif
#ifdef CTC_DNS_SPEED_LIMIT
	eX_CT_COM_DnsSpeedLimit,
#endif
#ifdef CONFIG_BHS
	eX_TELEFONICA_ES_IPTunnel,
	eX_TELEFONICA_ES_IPv6Layer3Forwarding,
#endif
#ifdef _PRMT_NSLOOKUP_
	eNSLookupDiagnostics,
#endif
};
struct CWMP_NODE tIGDObject[] =
{
/*info,  				leaf,			next)*/
{&tIGDObjectInfo[eDeviceInfo],		tDeviceInfoLeaf,	tDeviceInfoObject},
#ifdef _PRMT_DEVICECONFIG_
{&tIGDObjectInfo[eDeviceConfig],	tDeviceConfigLeaf,	NULL},
#endif //_PRMT_DEVICECONFIG_
{&tIGDObjectInfo[eManagementServer],	tManagementServerLeaf,	tManagementServerObject},
#ifdef TIME_ZONE
{&tIGDObjectInfo[eTime],		tTimeLeaf,		NULL},
#endif //TIME_ZONE
#ifdef _PRMT_USERINTERFACE_
{&tIGDObjectInfo[eUserInterface],	tUserIFLeaf,		NULL},
#endif /*_PRMT_USERINTERFACE_*/
#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
{&tIGDObjectInfo[eCaptivePortal],	tCaptivePortalLeaf,	NULL},
#endif //_SUPPORT_CAPTIVEPORTAL_PROFILE_
#ifdef _SUPPORT_L2BRIDGING_PROFILE_
{&tIGDObjectInfo[eLayer2Bridging],	tLayer2BridgingLeaf,	tLayer2BridgingObject},
#endif
#ifdef ROUTING
{&tIGDObjectInfo[eLayer3Forwarding],	tLayer3ForwardingLeaf,	tLayer3ForwardingObject},
#endif
#if (defined(CONFIG_E8B) && defined(_PRMT_X_STD_QOS_)) \
 	|| defined(IP_QOS) \
 	|| (!defined(CONFIG_E8B) && defined(CONFIG_USER_IP_QOS_3))
{&tIGDObjectInfo[eQueueManagement],	tQueueMntLeaf,		tQueueMntObject},
#endif
{&tIGDObjectInfo[eLANConfigSecurity],	tLANConfigSecurityLeaf,	NULL},
{&tIGDObjectInfo[eIPPingDiagnostics],	tIPPingDiagnosticsLeaf,	NULL},
#ifdef _SUPPORT_TRACEROUTE_PROFILE_
{&tIGDObjectInfo[eTraceRouteDiagnostics],tTraceRouteDiagLeaf,	tTraceRouteDiagObject},
#endif //_SUPPORT_TRACEROUTE_PROFILE_
{&tIGDObjectInfo[eLANDevice],		NULL,			tLANDeviceObject},
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{&tIGDObjectInfo[eLANInterfaces],	tLANInterfacesLeaf,			NULL},
#endif
/*ping_zhang:20081217 END*/
{&tIGDObjectInfo[eWANDevice], NULL, NULL}, // tWANDeviceObject},
#ifdef _PRMT_SERVICES_
{&tIGDObjectInfo[eServices],		NULL,			tServicesObject},
#endif //_PRMT_SERVICES_
#ifdef _PRMT_CAPABILITIES_
{&tIGDObjectInfo[eCapabilities],	NULL,			tCapabilitiesObject},
#endif //_PRMT_CAPABILITIES_
#ifdef _PRMT_TR143_
{&tIGDObjectInfo[eDownloadDiagnostics],	tDownloadDiagnosticsLeaf,NULL},
{&tIGDObjectInfo[eUploadDiagnostics],	tUploadDiagnosticsLeaf,	NULL},
{&tIGDObjectInfo[eUDPEchoConfig],	tUDPEchoConfigLeaf,	NULL},
#endif //_PRMT_TR143_
#ifdef _PRMT_X_CT_COM_QOS_
{&tIGDObjectInfo[eX_CT_COM_UplinkQoS],	tCT_UplinkQoSLeaf,		tCT_UplinkQoSObject},
#endif
#ifdef _PRMT_X_CT_COM_USERINFO_
{&tIGDObjectInfo[eX_CT_COM_UserInfo],	tCT_UserInfoLeaf,	NULL},
#endif
#ifdef _PRMT_X_CT_COM_PROXY_DEV_
{&tIGDObjectInfo[eX_CT_COM_ProxyDevice],   tCT_ProxyDeviceLeaf,    tCT_ProxyDeviceObject},
#endif
#ifdef CTC_DNS_SPEED_LIMIT
{&tIGDObjectInfo[eX_CT_COM_DnsSpeedLimit],	tCT_DnsSpeedLimitLeaf, NULL},
#endif
#ifdef CONFIG_BHS
{&tIGDObjectInfo[eX_TELEFONICA_ES_IPTunnel],	NULL,		NULL},
{&tIGDObjectInfo[eX_TELEFONICA_ES_IPv6Layer3Forwarding],	tIPv6Layer3ForwardingLeaf,	tIPv6Layer3ForwardingObject},
#endif
#ifdef _PRMT_NSLOOKUP_
{&tIGDObjectInfo[eNSLookupDiagnostics],	tNSLookupDiagLeaf, tNSLookupDiagObject},
#endif
{NULL,					NULL,			NULL}
};

/******Root*****************************************************************************************/
struct CWMP_PRMT tROOTObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"InternetGatewayDevice",	eCWMP_tOBJECT,	CWMP_READ,	NULL}
};
enum eROOTObject
{
	eInternetGatewayDevice
};
struct CWMP_NODE tROOT[] =
{
/*info, 	 				leaf,			next*/
{&tROOTObjectInfo[eInternetGatewayDevice],	tIGDLeaf,		tIGDObject	},
{NULL,						NULL,			NULL		}
};

#ifdef CONFIG_MIDDLEWARE
int getInternetPvc(void ** data)
{
	unsigned int devnum=0;
	unsigned int pppnum=0;
	MIB_CE_ATM_VC_T Entry;
	int ret=-1;
	unsigned int i,num;
	char buf[200];
	char tmp[16];

	memset(buf,0,200);
	memset(tmp,0,16);
	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)&Entry ) )
			continue;

		if(Entry.cmode != CHANNEL_MODE_PPPOE)
			continue;

		if(Entry.ServiceList & X_CT_SRV_INTERNET)
		{
			devnum = Entry.ConDevInstNum;
			pppnum = Entry.ConPPPInstNum;
			sprintf(tmp,"WAN%d.PPP%d",devnum,pppnum);
			if(ret == 0){//has found a internet pppoe pvc again
				strcat(buf,"$");
			}
			strcat(buf,tmp);
			ret = 0;
		}
	}

	if(ret == 0){
		*data = strdup(buf);
	}else{
		*data = strdup("NULL");
	}

	return 0;
}

int getMgtDNS(void ** data)
{
	char buf[100];
	FILE * fp;
#ifdef DNS_BIND_PVC_SUPPORT
	unsigned char DnsBindPvcEnable;
	unsigned int DnsBindPvc;
	struct in_addr dnsIP;
	int num,i=0;
	MIB_CE_ATM_VC_T Entry;
	unsigned int mgtIfindex=0;
#endif
	memset(buf,0,sizeof(buf));

	/*search mgtdns is file*/
	fp = fopen(MGT_DNS_FILE,"r");
	if(fp){
		if (fgets(buf,sizeof(buf),fp)){
			*data = strdup(buf);
			fclose(fp);
			return 0;
		}
	}

	/*search DNS Banding*/
#ifdef DNS_BIND_PVC_SUPPORT
	mib_get(MIB_DNS_BIND_PVC_ENABLE,(void *)&DnsBindPvcEnable);
	if(DnsBindPvcEnable)
	{
		num = mib_chain_total( MIB_ATM_VC_TBL );
		for( i=0; i<num;i++ )
		{
			if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)&Entry ) )
				continue;

			if(Entry.ServiceList & X_CT_SRV_TR069)
			{
				mgtIfindex = Entry.ifIndex;
				break;
			}
		}
		if(mgtIfindex == 0){
			goto notFind;
		}

		for(i=0;i<3;i++)
		{
			mib_get(MIB_DNS_BIND_PVC1+i,&DnsBindPvc);
			if(DnsBindPvc != mgtIfindex)
				continue;

			mib_get(MIB_ADSL_WAN_DNS1+i, (void *)&dnsIP);
			if(dnsIP.s_addr){
				strcpy(buf,inet_ntoa(dnsIP));
				*data = strdup(buf);
				return 0;
			}
		}
	}
#endif

notFind:
	*data = strdup("0.0.0.0");
	return 0;
}

int getCTMgtIPAddress(void ** data)
{
	unsigned int devnum=0;
	unsigned int pppnum=0;
	MIB_CE_ATM_VC_T Entry,*pEntry;
	int ret=-1;
	unsigned int i,num;

	pEntry = &Entry;

	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)pEntry ) )
			continue;

		if(pEntry->cmode == CHANNEL_MODE_BRIDGE){
			continue;
		}

		if(pEntry->ServiceList & X_CT_SRV_TR069)
		{
			ret = 0;
			break;
		}
	}
	if(ret == 0){
		char ifname[16];
		char *temp=NULL;
		struct in_addr inAddr;

		if(ifGetName( pEntry->ifIndex, ifname, 16 )==0)
			return ERR_9002;
		if (getInAddr( ifname, IP_ADDR, (void *)&inAddr) == 1)
			temp = inet_ntoa(inAddr);
		if(temp)
			*data=strdup(temp);
		else
			*data=strdup("0.0.0.0");
	}else{
		*data = strdup("0.0.0.0");
	}

	return 0;
}

int getCTUserIPAddress(void ** data,int ipIndex)
{
	unsigned int devnum=0;
	unsigned int pppnum=0;
	MIB_CE_ATM_VC_T Entry,*pEntry;
	int ret=-1;
	unsigned int i,num,InvalidNum=0;

	pEntry = &Entry;

	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)pEntry ) )
			continue;

		if(pEntry->cmode == CHANNEL_MODE_BRIDGE){
			continue;
		}

		if(pEntry->ServiceList & X_CT_SRV_INTERNET)
		{
			InvalidNum++;	//find the InvalidNumth Internet wandevice
			if(ipIndex == InvalidNum){
				char ifname[16];
				char *temp=NULL;
				struct in_addr inAddr;

				if(ifGetName( pEntry->ifIndex, ifname, 16 )==0)
					return ERR_9002;
				if (getInAddr( ifname, IP_ADDR, (void *)&inAddr) == 1)
					temp = inet_ntoa(inAddr);
				if(temp)
					*data=strdup(temp);
				else
					*data=strdup("0.0.0.0");

				return 0;
			}
		}
	}

	*data = strdup("0.0.0.0");
	return 0;
}

int getAppendInfo(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "InternetPvc" )==0 )
	{
		return(getInternetPvc(data));
	}else if( strcmp( lastname, "MgtDNS" )==0 )
	{
		return(getMgtDNS(data));
	}else if( strcmp( lastname, "CTMgtIPAddress" )==0 )
	{
		return(getCTMgtIPAddress(data));
	}else if( strcmp( lastname, "CTUserIPAddress1" )==0 )
	{
		return(getCTUserIPAddress(data,1));
	}else if( strcmp( lastname, "CTUserIPAddress2" )==0 )
	{
		return(getCTUserIPAddress(data,2));
	}else if( strcmp( lastname, "CTUserIPAddress3" )==0 )
	{
		return(getCTUserIPAddress(data,3));
	}else if( strcmp( lastname, "CTUserIPAddress4" )==0 )
	{
		return(getCTUserIPAddress(data,4));
	}else if( strcmp( lastname, "CTUserIPAddress5" )==0 )
	{
		return(getCTUserIPAddress(data,5));
	}else if( strcmp( lastname, "CTUserIPAddress6" )==0 )
	{
		return(getCTUserIPAddress(data,6));
	}else if( strcmp( lastname, "CTUserIPAddress7" )==0 )
	{
		return(getCTUserIPAddress(data,7));
	}else if( strcmp( lastname, "CTUserIPAddress8" )==0 )
	{
		return(getCTUserIPAddress(data,8));
	}else{
		return ERR_9005;
	}

	return 0;
}

struct CWMP_OP tAppendInfoOP = { getAppendInfo, NULL };

struct CWMP_PRMT tAppendLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"InternetPvc",	eCWMP_tSTRING,	CWMP_READ,		&tAppendInfoOP},
{"MgtDNS",	eCWMP_tSTRING,	CWMP_READ,		&tAppendInfoOP},
{"CTMgtIPAddress",		eCWMP_tSTRING,	CWMP_READ,		&tAppendInfoOP},
{"CTUserIPAddress1",		eCWMP_tSTRING,	CWMP_READ,		&tAppendInfoOP},
{"CTUserIPAddress2",		eCWMP_tSTRING,	CWMP_READ,		&tAppendInfoOP},
{"CTUserIPAddress3",		eCWMP_tSTRING,	CWMP_READ,		&tAppendInfoOP},
{"CTUserIPAddress4",		eCWMP_tSTRING,	CWMP_READ,		&tAppendInfoOP},
{"CTUserIPAddress5",		eCWMP_tSTRING,	CWMP_READ,		&tAppendInfoOP},
{"CTUserIPAddress6",		eCWMP_tSTRING,	CWMP_READ,		&tAppendInfoOP},
{"CTUserIPAddress7",		eCWMP_tSTRING,	CWMP_READ,		&tAppendInfoOP},
{"CTUserIPAddress8",		eCWMP_tSTRING,	CWMP_READ,		&tAppendInfoOP},
};

enum eAppendLeaf
{
	eInternetPvc,
	eMgtDNS,
	eCTMgtIPAddress,
	eCTUserIPAddress1,
	eCTUserIPAddress2,
	eCTUserIPAddress3,
	eCTUserIPAddress4,
	eCTUserIPAddress5,
	eCTUserIPAddress6,
	eCTUserIPAddress7,
	eCTUserIPAddress8
};

struct CWMP_LEAF tAppendLeaf[] =
{
{ &tAppendLeafInfo[eInternetPvc]  },
{ &tAppendLeafInfo[eMgtDNS]  },
{ &tAppendLeafInfo[eCTMgtIPAddress]  },
{ &tAppendLeafInfo[eCTUserIPAddress1]  },
{ &tAppendLeafInfo[eCTUserIPAddress2]  },
{ &tAppendLeafInfo[eCTUserIPAddress3]  },
{ &tAppendLeafInfo[eCTUserIPAddress4]  },
{ &tAppendLeafInfo[eCTUserIPAddress5]  },
{ &tAppendLeafInfo[eCTUserIPAddress6]  },
{ &tAppendLeafInfo[eCTUserIPAddress7]  },
{ &tAppendLeafInfo[eCTUserIPAddress8]  },
{ NULL	}
};

struct CWMP_PRMT tROOTAppendInfo[] =
{
/*(name,			type,		flag,		op)*/
{"AppendInfo",	eCWMP_tOBJECT,	CWMP_READ,	NULL}
};

enum eROOTAppendInfo
{
	eAppendInfo
};

struct CWMP_NODE mw_tROOT[] =	/*add some parameters in AppendInfo for midware*/
{
/*info, 	 				leaf,			next*/
{&tROOTObjectInfo[eInternetGatewayDevice],	tIGDLeaf,		tIGDObject	},
{&tROOTAppendInfo[eAppendInfo],	tAppendLeaf,		NULL	},
{NULL,						NULL,			NULL		}
};
#endif
/***********************************************************************************************/

int getIGD(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "LANDeviceNumberOfEntries" )==0 )
	{
		*data = uintdup( LANDEVICE_NUM );
	}else if( strcmp( lastname, "WANDeviceNumberOfEntries" )==0 )
	{
		unsigned int num=0;
#ifdef CONFIG_DEV_xDSL
		if(WAN_MODE&MODE_ATM)
		{
			num++;
		}
#endif /*CONFIG_DEV_xDSL*/
#ifdef CONFIG_PTMWAN
		if(WAN_MODE&MODE_PTM)
		{
			num++;
		}
#endif /*CONFIG_PTMWAN*/
#ifdef CONFIG_ETHWAN
		if(WAN_MODE&MODE_Ethernet)
		{
			num++;
		}
#endif /*CONFIG_ETHWAN*/
		*data = uintdup(num);
	}else if( strcmp( lastname, "DeviceSummary" )==0 )
	{
		*data = strdup( DEVICESUMMARY );
	}else{
		return ERR_9005;
	}

	return 0;
}

