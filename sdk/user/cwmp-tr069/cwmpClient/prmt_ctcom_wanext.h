#ifndef _PRMT_CTCOM_WANEXT_H_
#define _PRMT_CTCOM_WANEXT_H_

#include "prmt_igd.h"

#ifdef __cplusplus
extern "C" {
#endif

/***** InternetGatewayDevice.WANDevice.{i}. */
#ifdef _PRMT_X_CT_COM_WANEXT_
extern struct CWMP_PRMT tCTWANDevEntityLeafInfo[];
enum eCTWANDevEntityLeaf
{
	eX_CTCOM_WANIndex,
};

int getCTWANDevEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
#endif

/***** InternetGatewayDevice.WANDevice.{i}.WANConnectionDevice.{i}.WANPPPConnection.*/
#ifdef _PRMT_X_CT_EXT_ENABLE_
extern struct CWMP_PRMT tCTWANPPPCONENTITYLeafInfo[];
enum eCTWANPPPCONENTITYLeaf
{
#ifdef _PRMT_X_CT_COM_PPPOE_PROXY_
	ePPP_X_CTCOM_ProxyEnable,
	ePPP_X_CTCOM_MAXUser,
#endif //_PRMT_X_CT_COM_PPPOE_PROXY_
#ifdef _PRMT_X_CT_COM_WANEXT_
	ePPP_X_CTCOM_LanInterface,
	ePPP_X_CTCOM_ServiceList,
	ePPP_X_CTCOM_LanInterface_DHCPEnable,
	ePPP_X_CTCOM_IPForwardList,
#ifdef CONFIG_MCAST_VLAN
	ePPP_X_CTCOM_MulticastVlan,
#endif
#endif
#ifdef _PRMT_X_CT_COM_IPv6_
	ePPP_X_CTCOM_IPMode,
	ePPP_X_CTCOM_IPv6ConnectionStatus,
	ePPP_X_CTCOM_IPv6IPAddress,
	ePPP_X_CTCOM_IPv6IPAddressAlias,
	ePPP_X_CTCOM_IPv6IPAddressOrigin,
	ePPP_X_CTCOM_IPv6DNSServers,
	ePPP_X_CTCOM_IPv6PrefixDelegationEnabled,
	ePPP_X_CTCOM_IPv6PrefixAlias,
	ePPP_X_CTCOM_IPv6PrefixOrigin,
	ePPP_X_CTCOM_IPv6Prefix,
	ePPP_X_CTCOM_IPv6PrefixPltime,
	ePPP_X_CTCOM_IPv6PrefixVltime,
	ePPP_X_CTCOM_DefaultIPv6Gateway,
	ePPP_X_CTCOM_IPv6DomainName,
	ePPP_X_CTCOM_Dslite_Enable,
	ePPP_X_CTCOM_AftrMode,
	ePPP_X_CTCOM_Aftr,
#endif
};
int getCTWANPPPCONENTITY(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCTWANPPPCONENTITY(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif

/***** InternetGatewayDevice.WANDevice.{i}.WANConnectionDevice.{i}.WANIPConnection.*/
#ifdef _PRMT_X_CT_COM_WANEXT_
extern struct CWMP_PRMT tCTWANIPCONENTITYLeafInfo[];
enum eCTWANIPCONENTITYLeaf
{
	eIP_X_CTCOM_LanInterface,
	eIP_X_CTCOM_ServiceList,
	eIP_X_CTCOM_IPMode,
	eIP_X_CTCOM_IPv6ConnectionStatus,
	eIP_X_CTCOM_LanInterface_DHCPEnable,
	eIP_X_CTCOM_IPForwardList,
	eIP_X_CTCOM_IPv6IPAddress,
	eIP_X_CTCOM_IPv6IPAddressAlias,
	eIP_X_CTCOM_IPv6IPAddressOrigin,
	eIP_X_CTCOM_IPv6DNSServers,
	eIP_X_CTCOM_IPv6PrefixDelegationEnabled,
	eIP_X_CTCOM_IPv6PrefixAlias,
	eIP_X_CTCOM_IPv6PrefixOrigin,
	eIP_X_CTCOM_IPv6Prefix,
	eIP_X_CTCOM_IPv6PrefixPltime,
	eIP_X_CTCOM_IPv6PrefixVltime,
	eIP_X_CTCOM_DefaultIPv6Gateway,
	eIP_X_CTCOM_IPv6DomainName,
	eIP_X_CTCOM_Dslite_Enable,
	eIP_X_CTCOM_AftrMode,
	eIP_X_CTCOM_Aftr,
#ifdef CONFIG_MCAST_VLAN
	eIP_X_CTCOM_MulticastVlan,
#endif
};

int getCTWANIPCONENTITY(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCTWANIPCONENTITY(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif


/***** InternetGatewayDevice.WANDevice.{i}.WANConnectionDevice.{i}.WANEthernetLinkConfig. */
#if defined(CONFIG_ETHWAN) && defined(_PRMT_X_CT_COM_ETHLINK_)
extern struct CWMP_PRMT tCTWANETHLINKLeafInfo[];
enum eCTWANETHLINKLeaf
{
	eX_CTCOM_Enable,
	eX_CTCOM_Mode,
	eX_CTCOM_VLANIDMark,
	eX_CTCOM_8021pMark,
};

int getCTWANETHLINKCONF(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCTWANETHLINKCONF(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif


/***** InternetGatewayDevice.WANDevice.{i}.WANConnectionDevice.{i}.WANDSLLinkConfig. */
#if defined(CONFIG_DEV_xDSL) && defined(_PRMT_X_CT_COM_WANEXT_)
extern struct CWMP_PRMT tCTDSLLNKCONFLeafInfo[];
enum eCTDSLLNKCONFLeaf
{
	eDL_X_CTCOM_VLAN,
};

int getCTDSLLNKCONF(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCTDSLLNKCONF(char *name, struct CWMP_LEAF *entity, int type, void *data);

#ifdef CONFIG_VDSL
extern struct CWMP_LEAF tCT_VDSLLinkLeaf[];
int getCT_VDSLLinkConf(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_VDSLLinkConf(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif	//CONFIG_VDSL
#endif


/*****************************************************************************
 * IGD.WANDevice.{i}.WANConnectionDevice.{i}.WANIPConnection.{i}.X_CT-COM_DHCPOPTIONXXX.
 *****************************************************************************/
extern struct CWMP_NODE tCTDhcpOpt60Object[];
int getCTDhcpOpt60Entity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCTDhcpOpt60Entity(char *name, struct CWMP_LEAF *entity, int type, void *data);

extern struct CWMP_NODE tCTDhcpOpt125Object[];
int getCTDhcpOpt125Entity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCTDhcpOpt125Entity(char *name, struct CWMP_LEAF *entity, int type, void *data);

extern struct CWMP_NODE tCTDhcpv6Opt16Object[];
int getCTDhcpv6Opt16Entity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCTDhcpv6Opt16Entity(char *name, struct CWMP_LEAF *entity, int type, void *data);

extern struct CWMP_NODE tCTDhcpv6Opt17Object[];
int getCTDhcpv6Opt17Entity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCTDhcpv6Opt17Entity(char *name, struct CWMP_LEAF *entity, int type, void *data);

#if defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)
extern struct CWMP_LEAF tCT_XPONLinkLeaf[];
int getCT_XPONLinkConf(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_XPONLinkConf(char *name, struct CWMP_LEAF *entity, int type, void *data);

extern struct CWMP_LEAF tCT_XPONInterfaceConfLeaf[];
extern struct CWMP_PRMT tCT_XPONInterfaceConfLeafInfo[];
extern struct CWMP_NODE tCT_XPONInterfaceConfObject[];
extern struct CWMP_PRMT tCT_XPONInterfaceConfObjectInfo[];

int getCT_XPONInterfaceConf(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int getCT_XPONInterfaceEntityLeaf(char *name, struct CWMP_LEAF *entity, int *type, void **data);
#endif

#ifdef __cplusplus
}
#endif

#endif /*_PRMT_CTCOM_WANEXT_H_*/

