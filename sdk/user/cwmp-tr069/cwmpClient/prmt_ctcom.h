#ifndef _PRMT_CTCOM_H_
#define _PRMT_CTCOM_H_

#include "prmt_igd.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _PRMT_X_CT_COM_ACCOUNT_
extern struct CWMP_LEAF tCTAccountLeaf[];
int getCTAccount(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCTAccount(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif /*_PRMT_X_CT_COM_ACCOUNT_*/

#ifdef _PRMT_X_CT_COM_ALG_
extern struct CWMP_LEAF tXCTCOMALGLeaf[];
int getXCTCOMALG(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setXCTCOMALG(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif //_PRMT_X_CT_COM_ALG_

#ifdef _PRMT_X_CT_COM_RECON_
extern struct CWMP_LEAF tCT_ReConLeaf[];
int getCT_ReCon(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_ReCon(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif //_PRMT_X_CT_COM_RECON_

#ifdef _PRMT_X_CT_COM_PORTALMNT_
extern struct CWMP_LEAF tCT_PortalMNTLeaf[];
int getCT_PortalMNT(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_PortalMNT(char *name, struct CWMP_LEAF *entity, int type, void *data);
int apply_PortalMNT( int action_type, int id, void *olddata );
#endif //_PRMT_X_CT_COM_PORTALMNT_

#ifdef _PRMT_X_CT_COM_SRVMNG_
extern struct CWMP_LEAF tCTServiceLeaf[];
int getCTService(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCTService(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif /*_PRMT_X_CT_COM_SRVMNG_*/


#ifdef _PRMT_X_CT_COM_SYSLOG_
extern struct CWMP_LEAF tCT_SyslogLeaf[];
int getCT_Syslog(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_Syslog(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif //_PRMT_X_CT_COM_SYSLOG_

#ifdef CONFIG_MIDDLEWARE
extern struct CWMP_LEAF tCT_MiddlewareMgtLeaf[];
int getCT_MiddlewareMgt(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_MiddlewareMgt(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif //CONFIG_MIDDLEWARE
	
#ifdef _PRMT_X_CT_COM_UPNP_
extern struct CWMP_LEAF tCT_UPnPLeaf[];
int getCT_UPnP(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_UPnP(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif

#ifdef _PRMT_X_CT_COM_DLNA_
extern struct CWMP_LEAF tCT_DLNALeaf[];
int getCT_DLNA(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_DLNA(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif

#ifdef _PRMT_USBRESTORE
extern struct CWMP_LEAF tCT_RestoreLeaf[];
int getCT_Restore(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_Restore(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif

#ifdef _PRMT_X_CT_COM_DEVINFO_
extern struct CWMP_PRMT tCTDeviceInfoLeafInfo[];
enum eCTDeviceInfoLeaf
{
	eDIX_CTCOM_InterfaceVersion,
	eDIX_CTCOM_CardInterVersion,
	eDIX_CTCOM_DeviceType,
	eDIX_CTCOM_Card,
	eDIX_CTCOM_UPLink,
	eDIX_CTCOM_Capability,
	eDIX_CTCOM_IPForwardModeEnabled,
};
int getCTDeviceInfo(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCTDeviceInfo(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif	//_PRMT_X_CT_COM_DEVINFO_


#ifdef _PRMT_X_CT_COM_ALARM_MONITOR_
extern struct CWMP_NODE tCT_AlarmObject[];
extern struct CWMP_LEAF tCT_AlarmLeaf[];
extern struct CWMP_NODE tCT_MonitorObject[];
extern struct CWMP_LEAF tCT_MonitorLeaf[];
int getCT_Alarm(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_Alarm(char *name, struct CWMP_LEAF *entity, int type, void *data);
int getCT_AlarmEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_AlarmEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);
int objCT_AlarmEntity(char *name, struct CWMP_LEAF *e, int type, void *data);
int getCT_Monitor(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_Monitor(char *name, struct CWMP_LEAF *entity, int type, void *data);
int getCT_MonitorEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_MonitorEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);
int objCT_MonitorEntity(char *name, struct CWMP_LEAF *e, int type, void *data);
#endif	//_PRMT_X_CT_COM_ALARM_MONITOR_


#ifdef _PRMT_X_CT_COM_IPv6_
extern struct CWMP_LEAF tCT_IPProtocolVersionLeaf[];
int getCT_IPProtocolVersion(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_IPProtocolVersion(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif

#ifdef _PRMT_X_CT_COM_TIME_
enum eCTTimeLeaf
{
	eCT_NTPInterval,
	eCT_NTPServerType,
};
extern struct CWMP_PRMT tCTTimeLeafInfo[];
int getCTTime(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCTTime(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif

#ifdef CTC_DNS_SPEED_LIMIT
extern struct CWMP_LEAF tCT_DnsSpeedLimitLeaf[];
int getCTDnsSpeedLimit(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCTDnsSpeedLimit(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif

#ifdef __cplusplus
}
#endif
#endif /*_PRMT_TIME_H_*/
