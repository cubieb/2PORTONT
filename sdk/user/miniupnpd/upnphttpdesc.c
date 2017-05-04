#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <syslog.h>

#include "config.h"
#include "upnpglobalvars.h"
#include "upnpdescgen.h"
#include "miniupnpdpath.h"
#include "upnpdescstrings.h"

#ifdef CONFIG_USE_SHARED_LIB
#include "upnphttp.h"
#include "upnpsoap.h"
#else
#include "localupnphttp.h"
#include "localupnpsoap.h"
#endif

//#ifdef HAS_DUMMY_SERVICE
static void sendDummyDesc(struct upnphttp * h)
{
	static const char xml_desc[] = "<?xml version=\"1.0\"?>\r\n"
		"<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">"
		" <specVersion>"
		"    <major>1</major>"
		"    <minor>0</minor>"
		"  </specVersion>"
		"  <actionList />"
		"  <serviceStateTable />"
		"</scpd>\r\n";
	BuildResp_upnphttp(h, xml_desc, sizeof(xml_desc)-1);
	SendResp_upnphttp(h);
	CloseSocket_upnphttp(h);
}
//#endif

#ifdef CONFIG_TR_064
char *IntToStr(char *str_int, int dit)  // [2008.05.05]Magician: Convert integer value to string.
{
	sprintf(str_int, "%d", dit);
	return str_int;
}

char *strcatEx(char *dest, ...)  // [2008.05.05]Magician
{
	va_list ap;
	char *str;

	va_start(ap, dest);

  while (str = va_arg(ap, char *))
		strcat(dest, str);

	va_end(ap);

	return dest;
}

/* Magician: For TR-064 Root device description. */
/* Mason write version 1*/
/* 2008.05.05: Magician modified. Put global constants and variables into the code.*/
static void sendRootDesc(struct upnphttp * h)
{
	char xml_desc[20000], service_str[24], service_path[24], service_ctlurl[24], service_evturl[24], str_int[12];
	int i, totalEntry;
	MIB_CE_ATM_VC_T Entry;
	static const char root_device[] = "root xmlns=\"urn:schemas-upnp-org:device-1-0\"";

	strcpy(xml_desc, "<?xml version=\"1.0\" ?>\n");
	strcatEx(xml_desc, "<", root_device, ">\n", 0);
	strcatEx(xml_desc, "	<specVersion>\n", 0);
	strcatEx(xml_desc, "	<major>1</major>\n", 0);
	strcatEx(xml_desc, "	<minor>0</minor>\n", 0);
	strcatEx(xml_desc, "	</specVersion>\n", 0);
	strcatEx(xml_desc, "	<URLBase>", URL_BASE, "</URLBase>\n", 0);

	// Root Device
	strcatEx(xml_desc, "	<device>\n", 0);
	strcatEx(xml_desc, "		<deviceType>urn:schemas-upnp-org:device:InternetGatewayDevice:1</deviceType>\n", 0);
	strcatEx(xml_desc, "		<friendlyName>", ROOTDEV_FRIENDLYNAME, "</friendlyName>\n", 0);
	strcatEx(xml_desc, "		<manufacturer>", ROOTDEV_MANUFACTURER, "</manufacturer>\n", 0);
	strcatEx(xml_desc, "		<manufacturerURL>", ROOTDEV_MANUFACTURERURL, "</manufacturerURL>\n", 0);
	strcatEx(xml_desc, "		<modelDescription>", ROOTDEV_MODELDESCRIPTION, "</modelDescription>\n", 0);
	strcatEx(xml_desc, "		<modelName>", ROOTDEV_MODELNAME, "</modelName>\n", 0);
	strcatEx(xml_desc, "		<modelNumber>", ROOTDEV_MODELNUMBER, "</modelNumber>\n", 0);
	strcatEx(xml_desc, "		<modelURL>", ROOTDEV_MODELURL, "</modelURL>\n", 0);
	strcatEx(xml_desc, "		<serialNumber>", serialnumber, "</serialNumber>\n", 0);
	strcatEx(xml_desc, "		<UDN>", uuidvalue, "</UDN>\n", 0);
	strcatEx(xml_desc, "		<UPC>", ROOTDEV_UPC, "</UPC>\n", 0);

	strcatEx(xml_desc, "		<serviceList>\n", 0);
	strcatEx(xml_desc, "			<service>\n", 0);
	strcatEx(xml_desc, "				<serviceType>urn:schemas-upnp-org:service:Layer3Forwarding:1</serviceType>\n", 0);
	strcatEx(xml_desc, "				<serviceId>urn:upnp-org:serviceId:L3Forwarding1</serviceId>\n", 0);
	strcatEx(xml_desc, "				<SCPDURL>", L3F_PATH, "</SCPDURL>\n", 0);
	strcatEx(xml_desc, "				<controlURL>", L3F_CONTROLURL, "</controlURL>\n", 0);
	strcatEx(xml_desc, "				<eventSubURL>", L3F_EVENTURL, "</eventSubURL>\n", 0);
	strcatEx(xml_desc, "			</service>\n", 0);

	strcatEx(xml_desc, "			<service>\n", 0);
	strcatEx(xml_desc, "				<serviceType>urn:schemas-upnp-org:service:DeviceInfo:1</serviceType>\n", 0);
	strcatEx(xml_desc, "				<serviceId>urn:upnp-org:serviceId:DeviceInfo1</serviceId>\n", 0);
	strcatEx(xml_desc, "				<SCPDURL>", DEVICEINFO_PATH, "</SCPDURL>\n", 0);
	strcatEx(xml_desc, "				<controlURL>", DEVICEINFO_CONTROLURL, "</controlURL>\n", 0);
	strcatEx(xml_desc, "				<eventSubURL>", DEVICEINFO_EVENTURL, "</eventSubURL>\n", 0);
	strcatEx(xml_desc, "			</service>\n", 0);

	strcatEx(xml_desc, "			<service>\n", 0);
	strcatEx(xml_desc, "				<serviceType>urn:schemas-upnp-org:service:DeviceConfig:1</serviceType>\n", 0);
	strcatEx(xml_desc, "				<serviceId>urn:upnp-org:serviceId:DeviceConfig1</serviceId>\n", 0);
	strcatEx(xml_desc, "				<SCPDURL>", DEVICECONFIG_PATH, "</SCPDURL>\n", 0);
	strcatEx(xml_desc, "				<controlURL>", DEVICECONFIG_CONTROLURL, "</controlURL>\n", 0);
	strcatEx(xml_desc, "				<eventSubURL>", DEVICECONFIG_EVENTURL, "</eventSubURL>\n", 0);
	strcatEx(xml_desc, "			</service>\n", 0);

	strcatEx(xml_desc, "			<service>\n", 0);
	strcatEx(xml_desc, "				<serviceType>urn:schemas-upnp-org:service:LANConfigSecurity:1</serviceType>\n", 0);
	strcatEx(xml_desc, "				<serviceId>urn:upnp-org:serviceId:LANConfigSecurity1</serviceId>\n", 0);
	strcatEx(xml_desc, "				<SCPDURL>", LANCONFIGSECURITY_PATH, "</SCPDURL>\n", 0);
	strcatEx(xml_desc, "				<controlURL>", LANCONFIGSECURITY_CONTROLURL, "</controlURL>\n", 0);
	strcatEx(xml_desc, "				<eventSubURL>", LANCONFIGSECURITY_EVENTURL, "</eventSubURL>\n", 0);
	strcatEx(xml_desc, "			</service>\n", 0);

	strcatEx(xml_desc, "			<service>\n", 0);
	strcatEx(xml_desc, "				<serviceType>urn:schemas-upnp-org:service:ManagementServer:1</serviceType>\n", 0);
	strcatEx(xml_desc, "				<serviceId>urn:upnp-org:serviceId:ManagementServer1</serviceId>\n", 0);
	strcatEx(xml_desc, "				<SCPDURL>", MANAGEMENTSERVER_PATH, "</SCPDURL>\n", 0);
	strcatEx(xml_desc, "				<controlURL>", MANAGEMENTSERVER_CONTROLURL, "</controlURL>\n", 0);
	strcatEx(xml_desc, "				<eventSubURL>", MANAGEMENTSERVER_EVENTURL, "</eventSubURL>\n", 0);
	strcatEx(xml_desc, "			</service>\n", 0);

	strcatEx(xml_desc, "			<service>\n", 0);
	strcatEx(xml_desc, "				<serviceType>urn:schemas-upnp-org:service:Time:1</serviceType>\n", 0);
	strcatEx(xml_desc, "				<serviceId>urn:upnp-org:serviceId:Time1</serviceId>\n", 0);
	strcatEx(xml_desc, "				<SCPDURL>", TIME_PATH, "</SCPDURL>\n", 0);
	strcatEx(xml_desc, "				<controlURL>", TIME_CONTROLURL, "</controlURL>\n", 0);
	strcatEx(xml_desc, "				<eventSubURL>", TIME_EVENTURL, "</eventSubURL>\n", 0);
	strcatEx(xml_desc, "			</service>\n", 0);

	strcatEx(xml_desc, "			<service>\n", 0);
 	strcatEx(xml_desc, "				<serviceType>urn:schemas-upnp-org:service:USERINTERFACE_PATH:1</serviceType>\n", 0);
  strcatEx(xml_desc, "				<serviceId>urn:upnp-org:serviceId:USERINTERFACE_PATH1</serviceId>\n", 0);
  strcatEx(xml_desc, "				<SCPDURL>" USERINTERFACE_PATH "</SCPDURL>\n", 0);
  strcatEx(xml_desc, "				<controlURL>" USERINTERFACE_CONTROLURL "</controlURL>\n", 0);
  strcatEx(xml_desc, "				<eventSubURL>" USERINTERFACE_EVENTURL "</eventSubURL>\n", 0);
 	strcatEx(xml_desc, "			</service>\n", 0);
	strcatEx(xml_desc, "		</serviceList>\n", 0);

	strcatEx(xml_desc, "		<deviceList>\n", 0);
	// WAN device
	strcatEx(xml_desc, "			<device>\n", 0);
	strcatEx(xml_desc, "				<deviceType>urn:schemas-upnp-org:device:WANDevice:1</deviceType>\n", 0);
	strcatEx(xml_desc, "				<friendlyName>", WANDEV_FRIENDLYNAME, "</friendlyName>\n", 0);
	strcatEx(xml_desc, "				<manufacturer>", WANDEV_MANUFACTURER, "</manufacturer>\n", 0);
	strcatEx(xml_desc, "				<manufacturerURL>", WANDEV_MANUFACTURERURL, "</manufacturerURL>\n", 0);
	strcatEx(xml_desc, "				<modelDescription>", WANDEV_MODELDESCRIPTION, "</modelDescription>\n", 0);
	strcatEx(xml_desc, "				<modelName>", WANDEV_MODELNAME, "</modelName>\n", 0);
	strcatEx(xml_desc, "				<modelNumber>", WANDEV_MODELNUMBER, "</modelNumber>\n", 0);
	strcatEx(xml_desc, "				<modelURL>", WANDEV_MODELURL, "</modelURL>\n", 0);
	strcatEx(xml_desc, "				<serialNumber>", serialnumber, "</serialNumber>\n", 0);
	strcatEx(xml_desc, "				<UDN>", WANDEV_UDN, "</UDN>\n", 0);
	strcatEx(xml_desc, "				<UPC>", WANDEV_UPC, "</UPC>\n", 0);

	strcatEx(xml_desc, "				<serviceList>\n", 0);
	strcatEx(xml_desc, "					<service>\n", 0);
	strcatEx(xml_desc, "						<serviceType>urn:schemas-upnp-org:service:WANDSLConnectionManagement:1</serviceType>\n", 0);
	strcatEx(xml_desc, "						<serviceId>urn:upnp-org:serviceId:WANDSLConnectionManagement1</serviceId>\n", 0);
	strcatEx(xml_desc, "						<SCPDURL>", WANDSLCONNECTIONMANAGEMENT_PATH, "</SCPDURL>\n", 0);
	strcatEx(xml_desc, "						<controlURL>", WANDSLCONNECTIONMANAGEMENT_CONTROLURL, "</controlURL>\n", 0);
	strcatEx(xml_desc, "						<eventSubURL>", WANDSLCONNECTIONMANAGEMENT_EVENTURL, "</eventSubURL>\n", 0);
	strcatEx(xml_desc, "					</service>\n", 0);

#if 0
	strcatEx(xml_desc, "					<service>\n", 0);
	strcatEx(xml_desc, "						<serviceType>urn:schemas-upnp-org:service:WANEthernetInterfaceConfig:1</serviceType>\n", 0);
	strcatEx(xml_desc, "						<serviceId>urn:upnp-org:serviceId:WANEthernetInterfaceConfig1</serviceId>\n", 0);
	strcatEx(xml_desc, "						<SCPDURL>", WANETHERNETINTERFACECONFIG_PATH, "</SCPDURL>\n", 0);
	strcatEx(xml_desc, "						<controlURL>", WANETHERNETINTERFACECONFIG_CONTROLURL, "</controlURL>\n", 0);
	strcatEx(xml_desc, "						<eventSubURL>", WANETHERNETINTERFACECONFIG_EVENTURL, "</eventSubURL>\n", 0);
	strcatEx(xml_desc, "					</service>\n", 0);
#endif

	strcatEx(xml_desc, "					<service>\n", 0);
	strcatEx(xml_desc, "						<serviceType>urn:schemas-upnp-org:service:WANDSLInterfaceConfig:1</serviceType>\n", 0);
	strcatEx(xml_desc, "						<serviceId>urn:upnp-org:serviceId:WANDSLInterfaceConfig1</serviceId>\n", 0);
	strcatEx(xml_desc, "						<SCPDURL>", WANDSLINTERFACECONFIG_PATH, "</SCPDURL>\n", 0);
	strcatEx(xml_desc, "						<controlURL>", WANDSLINTERFACECONFIG_CONTROLURL, "</controlURL>\n", 0);
	strcatEx(xml_desc, "						<eventSubURL>", WANDSLINTERFACECONFIG_EVENTURL, "</eventSubURL>\n", 0);
	strcatEx(xml_desc, "					</service>\n", 0);

	strcatEx(xml_desc, "					<service>\n", 0);
	strcatEx(xml_desc, "						<serviceType>urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1</serviceType>\n", 0);
	strcatEx(xml_desc, "						<serviceId>urn:upnp-org:serviceId:WANCommonIFC1</serviceId>\n", 0);
	strcatEx(xml_desc, "						<SCPDURL>", WANCFG_PATH, "</SCPDURL>\n", 0);
	strcatEx(xml_desc, "						<controlURL>", WANCFG_CONTROLURL, "</controlURL>\n", 0);
	strcatEx(xml_desc, "						<eventSubURL>", WANCFG_EVENTURL, "</eventSubURL>\n", 0);
	strcatEx(xml_desc, "					</service>\n", 0);
	strcatEx(xml_desc, "				</serviceList>\n", 0);

	strcatEx(xml_desc, "				<deviceList>\n", 0);

	// To generate all WANConnectionDevice xml
	totalEntry = mib_chain_total(MIB_ATM_VC_TBL); /* get chain record size */
	int cmod[] = {-1, -1}, cidx = 0;

	for ( i=0 ; i<totalEntry; i++)
	{
		mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry);

		if((Entry.cmode == CHANNEL_MODE_PPPOE) || (Entry.cmode == CHANNEL_MODE_PPPOA))
		{
			strcpy(service_str, "WANPPPConnection");
			strcpy(service_path, WANPPPC_PATH);
			strcpy(service_ctlurl, WANPPPC_CONTROLURL);
			strcpy(service_evturl, WANPPPC_EVENTURL);
			cidx = 0;
			cmod[cidx]++;
		}
		else
		{
			strcpy(service_str, "WANIPConnection");
			strcpy(service_path, WANIPC_PATH);
			strcpy(service_ctlurl, WANIPC_CONTROLURL);
			strcpy(service_evturl, WANIPC_EVENTURL);
			cidx = 1;
			cmod[cidx]++;
		}

		strcatEx(xml_desc, "					<device>\n", 0);
		strcatEx(xml_desc, "						<deviceType>urn:schemas-upnp-org:device:WANConnectionDevice:", IntToStr(str_int, i), "</deviceType>\n", 0);
		strcatEx(xml_desc, "						<friendlyName>", WANCDEV_FRIENDLYNAME, ":", IntToStr(str_int, i), "</friendlyName>\n", 0);
		strcatEx(xml_desc, "						<manufacturer>", WANCDEV_MANUFACTURER, "</manufacturer>\n", 0);
		strcatEx(xml_desc, "						<manufacturerURL>", WANCDEV_MANUFACTURERURL, "</manufacturerURL>\n", 0);
		strcatEx(xml_desc, "						<modelDescription>", WANCDEV_MODELDESCRIPTION, "</modelDescription>\n", 0);
		strcatEx(xml_desc, "						<modelName>", WANCDEV_MODELNAME, "</modelName>\n", 0);
		strcatEx(xml_desc, "						<modelNumber>", WANCDEV_MODELNUMBER, "</modelNumber>\n", 0);
		strcatEx(xml_desc, "						<modelURL>", WANCDEV_MODELURL, "</modelURL>\n", 0);
		strcatEx(xml_desc, "						<serialNumber>", serialnumber, "</serialNumber>\n", 0);
		strcatEx(xml_desc, "						<UDN>", WANCDEV_UDN, "</UDN>\n", 0);
		strcatEx(xml_desc, "						<UPC>", WANCDEV_UPC, "</UPC>\n", 0);

		strcatEx(xml_desc, "						<serviceList>\n", 0);
		strcatEx(xml_desc, "							<service>\n", 0);
		strcatEx(xml_desc, "								<serviceType>urn:schemas-upnp-org:service:WANDSLLinkConfig:", IntToStr(str_int, i), "</serviceType>\n", 0);
		strcatEx(xml_desc, "								<serviceId>urn:upnp-org:serviceId:WANDSLLinkC:", IntToStr(str_int, i), "</serviceId>\n", 0);
		strcatEx(xml_desc, "								<SCPDURL>", WANDSLLINKCONFIG_PATH, "</SCPDURL>\n", 0);
		strcatEx(xml_desc, "								<controlURL>", WANDSLLINKCONFIG_CONTROLURL, "</controlURL>\n", 0);
		strcatEx(xml_desc, "								<eventSubURL>", WANDSLLINKCONFIG_EVENTURL, "</eventSubURL>\n", 0);
		strcatEx(xml_desc, "							</service>\n", 0);

		strcatEx(xml_desc, "							<service>\n", 0);
		strcatEx(xml_desc, "								<serviceType>urn:schemas-upnp-org:service:", service_str, ":", IntToStr(str_int, cmod[cidx]), "</serviceType>\n", 0);
		strcatEx(xml_desc, "								<serviceId>urn:upnp-org:serviceId:", service_str, IntToStr(str_int, cmod[cidx]), "</serviceId>\n", 0);
		strcatEx(xml_desc, "								<SCPDURL>", service_path, "</SCPDURL>\n", 0);
		strcatEx(xml_desc, "								<controlURL>", service_ctlurl, "</controlURL>\n", 0);
		strcatEx(xml_desc, "								<eventSubURL>", service_evturl, "</eventSubURL>\n", 0);
		strcatEx(xml_desc, "							</service>\n", 0);
		strcatEx(xml_desc, "						</serviceList>\n", 0);
		strcatEx(xml_desc, "					</device>\n", 0);
	}

		strcatEx(xml_desc, "				</deviceList>\n", 0);
		strcatEx(xml_desc, "			</device>\n", 0);
 		// LAN device
		strcatEx(xml_desc, "			<device>\n", 0);
		strcatEx(xml_desc, "				<deviceType>urn:schemas-upnp-org:device:LANDevice:1</deviceType>\n", 0);
		strcatEx(xml_desc, "				<friendlyName>", LANDEV_FRIENDLYNAME, "</friendlyName>\n", 0);
		strcatEx(xml_desc, "				<manufacturer>", LANDEV_MANUFACTURER, "</manufacturer>\n", 0);
		strcatEx(xml_desc, "				<manufacturerURL>", LANDEV_MANUFACTURERURL, "</manufacturerURL>\n", 0);
		strcatEx(xml_desc, "				<modelDescription>", LANDEV_MODELDESCRIPTION, "</modelDescription>\n", 0);
		strcatEx(xml_desc, "				<modelName>", LANDEV_MODELNAME, "</modelName>\n", 0);
		strcatEx(xml_desc, "				<modelNumber>", LANDEV_MODELNUMBER, "</modelNumber>\n", 0);
		strcatEx(xml_desc, "				<modelURL>", LANDEV_MODELURL, "</modelURL>\n", 0);
		strcatEx(xml_desc, "				<serialNumber>", LANDEV_UPC, "</serialNumber>\n", 0);
		strcatEx(xml_desc, "				<UDN>", LANDEV_UDN, "</UDN>\n", 0);

		strcatEx(xml_desc, "				<serviceList>\n", 0);
		strcatEx(xml_desc, "					<service>\n", 0);
		strcatEx(xml_desc, "						<serviceType>urn:schemas-upnp-org:service:Hosts:1</serviceType>\n", 0);
		strcatEx(xml_desc, "						<serviceId>urn:upnp-org:serviceId:Hosts1</serviceId>\n", 0);
		strcatEx(xml_desc, "						<SCPDURL>", HOSTS_PATH, "</SCPDURL>\n", 0);
		strcatEx(xml_desc, "						<controlURL>", HOSTS_CONTROLURL, "</controlURL>\n", 0);
		strcatEx(xml_desc, "						<eventSubURL>", HOSTS_EVENTURL, "</eventSubURL>\n", 0);
		strcatEx(xml_desc, "					</service>\n", 0);

#if 0
		strcatEx(xml_desc, "					<service>\n", 0);
		strcatEx(xml_desc, "						<serviceType>urn:schemas-upnp-org:service:WLANConfiguration:1</serviceType>\n", 0);
		strcatEx(xml_desc, "						<serviceId>urn:upnp-org:serviceId:WLANConfiguration1</serviceId>\n", 0);
		strcatEx(xml_desc, "						<SCPDURL>", WLANCONFIGURATION_PATH, "</SCPDURL>\n", 0);
		strcatEx(xml_desc, "						<controlURL>", WLANCONFIGURATION_CONTROLURL, "</controlURL>\n", 0);
		strcatEx(xml_desc, "						<eventSubURL>", WLANCONFIGURATION_EVENTURL, "</eventSubURL>\n", 0);
		strcatEx(xml_desc, "					</service>\n", 0);
#endif

		strcatEx(xml_desc, "					<service>\n", 0);
		strcatEx(xml_desc, "						<serviceType>urn:schemas-upnp-org:service:LANEthernetInterfaceConfig:1</serviceType>\n", 0);
		strcatEx(xml_desc, "						<serviceId>urn:upnp-org:serviceId:LANEthernetInterfaceConfig1</serviceId>\n", 0);
		strcatEx(xml_desc, "						<SCPDURL>", LANETHERNETINTERFACECONFIG_PATH, "</SCPDURL>\n", 0);
		strcatEx(xml_desc, "						<controlURL>", LANETHERNETINTERFACECONFIG_CONTROLURL, "</controlURL>\n", 0);
		strcatEx(xml_desc, "						<eventSubURL>", LANETHERNETINTERFACECONFIG_EVENTURL, "</eventSubURL>\n", 0);
		strcatEx(xml_desc, "					</service>\n", 0);

		strcatEx(xml_desc, "					<service>\n", 0);
		strcatEx(xml_desc, "						<serviceType>urn:schemas-upnp-org:service:LANHostConfigManagement:1</serviceType>\n", 0);
		strcatEx(xml_desc, "						<serviceId>urn:upnp-org:serviceId:LANHostConfigManagement1</serviceId>\n", 0);
		strcatEx(xml_desc, "						<SCPDURL>", LANHOSTCONFIGMANAGEMENT_PATH, "</SCPDURL>\n", 0);
		strcatEx(xml_desc, "						<controlURL>", LANHOSTCONFIGMANAGEMENT_CONTROLURL, "</controlURL>\n", 0);
		strcatEx(xml_desc, "						<eventSubURL>", LANHOSTCONFIGMANAGEMENT_EVENTURL, "</eventSubURL>\n", 0);
		strcatEx(xml_desc, "					</service>\n", 0);
		strcatEx(xml_desc, "				</serviceList>\n", 0);
		strcatEx(xml_desc, "			</device>\n", 0);
		strcatEx(xml_desc, "		</deviceList>\n", 0);
		strcatEx(xml_desc, "		<presentationURL>", presentationurl, "</presentationURL>\n", 0);
		strcatEx(xml_desc, "	</device>\n", 0);
		strcatEx(xml_desc, "</root>\n", 0);

	BuildResp_upnphttp(h, xml_desc, strlen(xml_desc));
	SendResp_upnphttp(h);
	CloseSocket_upnphttp(h);
}
#endif

#ifdef CONFIG_TR_064
struct _sendDesc sendDesc_tr064[] =
{
	{ ROOTDESC_PATH_TR064, .Impl.sendDescImpl_h = sendRootDesc, 1 },
	{ WANIPC_PATH, .Impl.sendDescImpl = genWANIPCn, 0 },
	{ WANCFG_PATH, .Impl.sendDescImpl = genWANCfg, 0 },
	{ WANDSLLINKCONFIG_PATH, .Impl.sendDescImpl = genWANDSLLink, 0 },
#ifdef HAS_DUMMY_SERVICE
	{ DUMMY_PATH, .Impl.sendDescImpl_h = sendDummyDesc, 1 },
#endif
	{ L3F_PATH, .Impl.sendDescImpl = genLayer3Forwarding, 0 },
	{ DEVICEINFO_PATH, .Impl.sendDescImpl = genDeviceInfo, 0 },
	{ DEVICECONFIG_PATH, .Impl.sendDescImpl = genDeviceConfig, 0 },
	{ LANCONFIGSECURITY_PATH, .Impl.sendDescImpl = genLANConfigSecurity, 0 },
	{ MANAGEMENTSERVER_PATH, .Impl.sendDescImpl = genManagementServer, 0 },
	{ TIME_PATH, .Impl.sendDescImpl = genTime, 0 },
	{ USERINTERFACE_PATH, .Impl.sendDescImpl_h = sendDummyDesc, 1 },
	{ LANHOSTCONFIGMANAGEMENT_PATH, .Impl.sendDescImpl = genLANHostConfigManagement, 0 },
	{ LANETHERNETINTERFACECONFIG_PATH, .Impl.sendDescImpl = genLANEthernetInterfaceConfig, 0 },
	{ WLANCONFIGURATION_PATH, .Impl.sendDescImpl = genWLANConfiguration, 0 },
	{ HOSTS_PATH, .Impl.sendDescImpl = genHosts, 0 },
	{ WANDSLCONNECTIONMANAGEMENT_PATH, .Impl.sendDescImpl = genWANDSLConnectionManagement, 0 },
	{ WANDSLINTERFACECONFIG_PATH, .Impl.sendDescImpl = genWANDSLInterfaceConfig, 0 },
	{ WANETHERNETINTERFACECONFIG_PATH, .Impl.sendDescImpl = genWANEthernetInterfaceConfig, 0 },
	{ WANPPPC_PATH, .Impl.sendDescImpl = genWANPPPCn, 0 },
	{ 0, .Impl.sendDescImpl_h = sendDummyDesc, 1 }	//cathy, if descName not found, send DummyDesc
};
#endif

struct _sendDesc sendDesc_igd[] =
{
	{ ROOTDESC_PATH_IGD, .Impl.sendDescImpl = genRootDesc, 0 },
	{ WANIPC_PATH, .Impl.sendDescImpl = genWANIPCn, 0 },
	{ WANCFG_PATH, .Impl.sendDescImpl = genWANCfg, 0 },
	{ WANDSLLINKCONFIG_PATH, .Impl.sendDescImpl = genWANDSLLink, 0 },
#ifdef HAS_DUMMY_SERVICE
	{ DUMMY_PATH, .Impl.sendDescImpl_h = sendDummyDesc, 1 },
#endif
#ifdef ENABLE_L3F_SERVICE
	{ L3F_PATH, .Impl.sendDescImpl_h = genL3F, 0 },
#endif
	{ 0, .Impl.sendDescImpl_h = sendDummyDesc, 1 }	//cathy, if descName not found, send DummyDesc
};

