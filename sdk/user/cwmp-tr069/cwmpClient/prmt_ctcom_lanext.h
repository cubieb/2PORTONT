#ifndef _PRMT_CTCOM_LANEXT_H_
#define _PRMT_CTCOM_LANEXT_H_

#include "prmt_igd.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _PRMT_X_CT_COM_IPv6_
/*****************************************************************************
 * InternetGatewayDevice.LANDevice.{i}.X_CT-COM_RouterAdvertisement.
 *****************************************************************************/
extern struct CWMP_LEAF tCTRALeaf[];
int getCTRA(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCTRA(char *name, struct CWMP_LEAF *entity, int type, void *data);


/*****************************************************************************
 * InternetGatewayDevice.LANDevice.{i}.X_CT-COM_DHCPv6Server.
 *****************************************************************************/
extern struct CWMP_LEAF tCTDHCPv6ServerLeaf[];
int getCTDHCPv6Server(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCTDHCPv6Server(char *name, struct CWMP_LEAF *entity, int type, void *data);


/*****************************************************************************
 * InternetGatewayDevice.LANDevice.{i}.X_CT-COM_IPv6Config.
 *****************************************************************************/
extern struct CWMP_NODE tCTIPv6ConfigObject[];
extern struct CWMP_LEAF tCTIPv6ConfigLeaf[];
int getCTIPv6Config(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCTIPv6Config(char *name, struct CWMP_LEAF *entity, int type, void *data);
int getCTIPv6PrefixInfoEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCTIPv6PrefixInfoEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);


/*****************************************************************************
 * InternetGatewayDevice.LANDevice.{i}.LANHostConfigManagement.IPInterface.{i}.
 *****************************************************************************/
extern struct CWMP_NODE tCTIPInterfaceEntityObject[];
int getCTIPv6LocalAddress(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCTIPv6LocalAddress(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif


/*****************************************************************************
 * InternetGatewayDevice.LANDevice.{i}.LANHostConfigManagement.
 *****************************************************************************/
#ifdef _PRMT_X_CT_COM_DHCP_
extern struct CWMP_PRMT tCTLANHostConfLeafInfo[];
enum eCTLANHostConfLeaf
{
	eOPTION60Enable,
	eOPTION125Enable,
	eOPTION16Enable,
	eOPTION17Enable,
	eSTB_MinAddress,
	eSTB_MaxAddress,
	ePhone_MinAddress,
	ePhone_MaxAddress,
	eCamera_MinAddress,
	eCamera_MaxAddress,
	eComputer_MinAddress,
	eComputer_MaxAddress,
};

int getCTLANHostConf(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCTLANHostConf(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif	//_PRMT_X_CT_COM_DHCP_



/*****************************************************************************
 * InternetGatewayDevice.LANDevice.{i}.WLANConfiguration.{i}.
 *****************************************************************************/
#ifdef _PRMT_X_CT_COM_WLAN_
extern struct CWMP_PRMT tCTWLANConfEntityLeafInfo[];
enum eCTWLANConfEntityLeafInfo
{
	eWL_CTStandard,
	eWL_CTWEPEncryptionLevel,
	eWL_X_CTCOM_SSIDHide,
	eWL_X_CTCOM_RFBand,
	eWL_X_CTCOM_ChannelWidth,
	eWL_X_CTCOM_GuardInterval,
	eWL_X_CTCOM_RetryTimeout,
	eWL_X_CTCOM_Powerlevel,
	eWL_X_CTCOM_PowerValue,
	eWL_X_CTCOM_APModuleEnable,
	eWL_X_CTCOM_WPSKeyWord,
};
int getCTWLANConf(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCTWLANConf(char *name, struct CWMP_LEAF *entity, int type, void *data);

#endif	//_PRMT_X_CT_COM_WLAN_


#ifdef _PRMT_X_CT_COM_VLAN_BOUND_
enum eCTVlanBoundingLeaf
{
	eCTBOUNDING_Mode,
	eCTBOUNDING_VLAN,
};
extern struct CWMP_PRMT tCTVlanBoundingLeafInfo[];
int getCTVlanBounding(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCTVlanBounding(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif	//_PRMT_X_CT_COM_VLAN_BOUND_

#ifdef _PRMT_X_CT_COM_LBD_
extern struct CWMP_LEAF tCT_LBDLeaf[];
int getCT_LBD(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_LBD(char *name, struct CWMP_LEAF *entity, int type, void *data);

extern struct CWMP_PRMT tCT_EthLBDStatusLeafInfo[];
enum eCT_EthLBDStatusLeaf
{
	eCT_EthLBDStatus,
};
int getCT_EthLBDStatus(char *name, struct CWMP_LEAF *entity, int *type, void **data);
#endif


#ifdef __cplusplus
}
#endif

#endif /*_PRMT_CTCOM_LANEXT_H_*/

