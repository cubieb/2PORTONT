#ifndef _PRMT_DEVICEINFO_H_
#define _PRMT_DEVICEINFO_H_

#include "prmt_igd.h"

/*Device Type isntance, only for TR-181 currently*/
#define DT_DOC_PATH		"/dt_doc/"
#define DT_DOC_LOCATION	"/etc/dt_doc"

#ifdef __cplusplus
extern "C" {
#endif

enum eDeviceInfoLeaf
{
	eDIManufacturer,
	eDIManufacturerOUI,
	eDIModelName,
	eDIDescription,
	eDIProductClass,
	eDISerialNumber,
	eDIHardwareVersion,
	eDISoftwareVersion,
#ifdef CONFIG_DEV_xDSL
	eDIModemFirmwareVersion,
#endif
	eDIEnabledOptions,
	eDISpecVersion,
	eDIProvisioningCode,
	eDIUpTime,
	eDIFirstUseDate,
	eDIDeviceLog,
	eDIVendorConfigFileNumberOfEntries,
};

extern struct CWMP_LEAF tDeviceInfoLeaf[];
extern struct CWMP_NODE tDeviceInfoObject[];
extern struct CWMP_NODE tVendorConfigObject[];
extern struct CWMP_PRMT tVendorCfgEntityLeafInfo[];

int getDeviceInfo(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setDeviceInfo(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getVendorCfgEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);

#ifdef _PRMT_DEVICECONFIG_
extern struct CWMP_LEAF tDeviceConfigLeaf[];
int getDeviceConfig(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setDeviceConfig(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif //_PRMT_DEVICECONFIG_

#define CONFIG_FILE_NAME "/tmp/tr69cfg.xml" 

#ifdef __cplusplus
}
#endif

#endif /*_PRMT_DEVICEINFO_H_*/
