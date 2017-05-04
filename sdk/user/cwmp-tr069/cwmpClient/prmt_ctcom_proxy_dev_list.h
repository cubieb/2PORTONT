#ifndef _PRMT_CTCOM_PROXY_DEV_LIST_H_
#define _PRMT_CTCOM_PROXY_DEV_LIST_H_

#include "prmt_igd.h"

#ifdef __cplusplus
extern "C" {
#endif

/***** Utilities ******/
int get_device_cnt(void);
void upnpdm_check_download(void);

/***** IGD.X_CT-COM_PorxyDevice.DeviceList.ServiceObject.WLANConfiguration *****/
int objCT_AP_WLANConf(char *name, struct CWMP_LEAF *e, int type, void *data);

/***** IGD.X_CT-COM_PorxyDevice.DeviceList.ActionList.BMS.Reboot. ************/
int getCT_BMS_Reboot(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_BMS_Reboot(char *name, struct CWMP_LEAF *entity, int type, void *data);


/***** IGD.X_CT-COM_PorxyDevice.DeviceList.ActionList.BMS.BaselineReset. *****/
int getCT_BMS_BaselineReset(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_BMS_BaselineReset(char *name, struct CWMP_LEAF *entity, int type, void *data);


/***** IGD.X_CT-COM_PorxyDevice.DeviceList.ActionList.BMS.GetDevStatus. ************/
int getCT_BMS_GetDevStatus(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_BMS_GetDevStatus(char *name, struct CWMP_LEAF *entity, int type, void *data);


/***** IGD.X_CT-COM_PorxyDevice.DeviceList.ActionList.BMS.X_CT-COM_Download. *****/
int getCT_BMS_Download(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_BMS_Download(char *name, struct CWMP_LEAF *entity, int type, void *data);


/***** IGD.X_CT-COM_PorxyDevice.DeviceList.ActionList.BMS.GetDownloadStatus. ************/
int getCT_BMS_GetDownloadStatus(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_BMS_GetDownloadStatus(char *name, struct CWMP_LEAF *entity, int type, void *data);


/***** IGD.X_CT-COM_PorxyDevice.DeviceList. **********************************/
extern struct CWMP_OP tCT_DeviceListOP;
extern struct CWMP_LINKNODE tCT_DeviceListObject[];
int objCT_DeviceList(char *name, struct CWMP_LEAF *e, int type, void *data);
int getCT_DeviceListEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);




#ifdef __cplusplus
}
#endif

#endif /*_PRMT_CTCOM_PROXY_DEV_LIST_H_*/

