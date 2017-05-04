#ifndef _PRMT_TR181_DEVICEINFO_H_
#define _PRMT_TR181_DEVICEINFO_H_

#include "parameter_api.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_LEAF tTR181DeviceInfoLeaf[];
extern struct CWMP_NODE tTR181DeviceInfoObject[];
extern struct CWMP_NODE tVendorLogObject[];
extern struct CWMP_PRMT tVendorLogEntityLeafInfo[];

int getTR181DeviceInfo(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int getVendorLogEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int objSupportedDM(char *name, struct CWMP_LEAF *e, int type, void *data);
int getSupportedDMEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);


#ifdef __cplusplus
}
#endif

#endif /*_PRMT_TR181_DEVICEINFO_H_*/
