#ifndef _PRMT_CTCOM_PROXY_DEV_H_
#define _PRMT_CTCOM_PROXY_DEV_H_

#include "prmt_igd.h"

#ifdef __cplusplus
extern "C" {
#endif

/***** IGD.X_CT-COM_ProxyDevice.ServiceProfile.ConfigTemplate ****************/
int getCT_ConfigTemplate(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int getCT_Template_APEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_Template_APEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);


/* InternetGatewayDevice.X_CT-COM_ProxyDevice.ServiceProfile.ConfigProfile. **/
int objCT_ConfigProfile(char *name, struct CWMP_LEAF *e, int type, void *data);
int getCT_ConfigProfileEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_ConfigProfileEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

/**** InternetGatewayDevice.X_CT-COM_ProxyDevice.ServiceProfile **************/
int getCT_ServiceProfile(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_ServiceProfile(char *name, struct CWMP_LEAF *entity, int type, void *data);


/*** IGD.X_CT-COM_PorxyDevice.SoftwareProfile.FileProfile.{i}.TimeWindowList */
int objCT_TimeWin(char *name, struct CWMP_LEAF *e, int type, void *data);
int getCT_FP_TimeWinEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_FP_TimeWinEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);


/***** IGD.X_CT-COM_PorxyDevice.SoftwareProfile.FileProfile ******************/
int getCT_FileProfile_entry(unsigned int num, MIB_CE_UPNPDM_FILE_PROFILE_T *pEntry, int *chainid);
int objCT_FileProfile(char *name, struct CWMP_LEAF *e, int type, void *data);
int getCT_FileProfileEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_FileProfileEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

/***** IGD.X_CT-COM_PorxyDevice.SoftwareProfile. *****************************/
int getCT_SoftwareProfile(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_SoftwareProfile(char *name, struct CWMP_LEAF *entity, int type, void *data);

/***** InternetGatewayDevice.X_CT-COM_ProxyDevice. ***************************/
extern struct CWMP_LEAF tCT_ProxyDeviceLeaf[];
extern struct CWMP_NODE tCT_ProxyDeviceObject[];

int getCT_ProxyDevice(char *name, struct CWMP_LEAF *entity, int *type, void **data);

#ifdef __cplusplus
}
#endif

#endif /*_PRMT_CTCOM_PROXY_DEV_H_*/

