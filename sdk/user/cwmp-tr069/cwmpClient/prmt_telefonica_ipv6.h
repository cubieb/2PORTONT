#ifndef _PRMT_TELEFONICA_IPV6_H
#define _PRMT_TELEFONICA_IPV6_H

#include "prmt_igd.h"

#ifdef __cplusplus
extern "C" {
#endif


int getIPTunnelEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setIPTunnelEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);
int get4in6TunnelEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int set4in6TunnelEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);
int get6in4TunnelEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int set6in4TunnelEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);
int obj4in6Tunnel(char *name, struct CWMP_LEAF *e, int type, void *data);
int obj6in4Tunnel(char *name, struct CWMP_LEAF *e, int type, void *data);
int objIPTunnel(char *name, struct CWMP_LEAF *e, int type, void *data);
int getIPv6Layer3Forwarding(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setIPv6Layer3Forwarding(char *name, struct CWMP_LEAF *entity, int type, void *data);
int getIPv6ForwardingEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setIPv6ForwardingEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);
int objIPv6Forwarding(char *name, struct CWMP_LEAF *e, int type, void *data);

extern struct CWMP_LEAF tIPv6Layer3ForwardingLeaf[];
extern struct CWMP_NODE tIPv6Layer3ForwardingObject[];


/***** IPv6 LAN Parameters ***************************/
int getIPv6LanIntfAddress(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setIPv6LanIntfAddress(char *name, struct CWMP_LEAF *entity, int type, void *data);
int getIPv6InterfaceAddressingType(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setIPv6InterfaceAddressingType(char *name, struct CWMP_LEAF *entity, int type, void *data);
int getDelegatedAddressEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setDelegatedAddressEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);
int objDelegatedAddress(char *name, struct CWMP_LEAF *e, int type, void *data);
int getIPv6LANHostConfigManagement(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setIPv6LANHostConfigManagement(char *name, struct CWMP_LEAF *entity, int type, void *data);
int getRADVDLeaf(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setRADVDLeaf(char *name, struct CWMP_LEAF *entity, int type, void *data);
int getIPv6SitePrefixConfigType(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setIPv6SitePrefixConfigType(char *name, struct CWMP_LEAF *entity, int type, void *data);
int getIPv6PDWanConnection(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setIPv6PDWanConnection(char *name, struct CWMP_LEAF *entity, int type, void *data);
int getIPv6InterfaceNumberOfEntries(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setIPv6InterfaceNumberOfEntries(char *name, struct CWMP_LEAF *entity, int type, void *data);
int getULAPrefixInfo(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setULAPrefixInfo(char *name, struct CWMP_LEAF *entity, int type, void *data);
int getPrefixInformationEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setPrefixInformationEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);
int objPrefixInformation(char *name, struct CWMP_LEAF *e, int type, void *data);
int getServerType(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setServerType(char *name, struct CWMP_LEAF *entity, int type, void *data);
int getServerTypeEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setServerTypeEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

extern struct CWMP_NODE tIPInterfaceEntityObject[];
extern struct CWMP_LEAF tIPv6LANHostConfigManagementLeaf[];
extern struct CWMP_NODE tIPv6LANHostConfigManagementObject[];


/***** IPv6 WAN Parameters ***************************/
/**** InternetGatewayDevice.WANDevice.{i}.WANConnectionDevice.{i}.WANPPPConnection.*/
extern struct CWMP_PRMT tBHSWANPPPCONENTITYLeafInfo[];
enum eBHSWANPPPCONENTITYLeaf
{
	 ePPP_X_TELEFONICA_ES_IPv6Enabled,
	 ePPP_X_TELEFONICA_ES_IPv4Enabled,
	 ePPP_X_TELEFONICA_ES_UnnumberedModel,
	 ePPP_X_TELEFONICA_ES_IPv6ConnStatus,
	 ePPP_X_TELEFONICA_ES_IPv6PppUp,
	 ePPP_X_TELEFONICA_ES_IPv6AddressingType,
	 ePPP_X_TELEFONICA_ES_DHCP6cForAddress,
	 ePPP_X_TELEFONICA_ES_ExternalIPv6Address,
	 ePPP_X_TELEFONICA_ES_DefaultIPv6Gateway,
	 ePPP_X_TELEFONICA_ES_IPv6DNSServers,
	 ePPP_X_TELEFONICA_ES_IPv6PrefixDelegationEnabled,
	 ePPP_X_TELEFONICA_ES_IPv6SitePrefix,
	 ePPP_X_TELEFONICA_ES_IPv6SitePrefixPltime,
	 ePPP_X_TELEFONICA_ES_IPv6SitePrefixVLtime,
	 ePPP_X_TELEFONICA_ES_ExternalIPv6AddressPrefixLength,
};

int getBHSWANPPPCONENTITY(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setBHSWANPPPCONENTITY(char *name, struct CWMP_LEAF *entity, int type, void *data);


/**** InternetGatewayDevice.WANDevice.{i}.WANConnectionDevice.{i}.WANIPConnection.*/
extern struct CWMP_PRMT tBHSWANIPCONENTITYLeafInfo[];
enum eBHSWANIPCONENTITYLeaf
{
	eIP_X_TELEFONICA_ES_IPv6Enabled,
	eIP_X_TELEFONICA_ES_IPv4Enabled,
	eIP_X_TELEFONICA_ES_UnnumberedModel,
	eIP_X_TELEFONICA_ES_IPv6ConnStatus,
	eIP_X_TELEFONICA_ES_IPv6AddressingType,
	eIP_X_TELEFONICA_ES_DHCP6cForAddress,
	eIP_X_TELEFONICA_ES_ExternalIPv6Address,
	eIP_X_TELEFONICA_ES_DefaultIPv6Gateway,
	eIP_X_TELEFONICA_ES_IPv6DNSServers,
	eIP_X_TELEFONICA_ES_IPv6PrefixDelegationEnabled,
	eIP_X_TELEFONICA_ES_IPv6SitePrefix,
	eIP_X_TELEFONICA_ES_IPv6SitePrefixPltime,
	eIP_X_TELEFONICA_ES_IPv6SitePrefixVLtime,
	eIP_X_TELEFONICA_ES_ExternalIPv6AddressPrefixLength,
};

int getBHSWANIPCONENTITY(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setBHSWANIPCONENTITY(char *name, struct CWMP_LEAF *entity, int type, void *data);


#ifdef __cplusplus
}
#endif
#endif /* _PRMT_TELEFONICA_IPV6_H */
