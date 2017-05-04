#ifndef _PRMT_TR181_WIFI_AP_H_
#define _PRMT_TR181_WIFI_AP_H_

#include <parameter_api.h>

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_NODE tWiFiAPObject[];

int getWiFiAPEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setWiFiAPEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);
#ifdef CONFIG_WIFI_SIMPLE_CONFIG
int getWiFiAPWPS(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setWiFiAPWPS(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif
int getWiFiAPSecurity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setWiFiAPSecurity(char *name, struct CWMP_LEAF *entity, int type, void *data);

int objWiFiAPAssocDev(char *name, struct CWMP_LEAF *e, int type, void *data);
int getWiFiAPAssocats(char *name, struct CWMP_LEAF *entity, int *type, void **data);


#ifdef __cplusplus
}
#endif

#endif /*_PRMT_TR181_WIFI_AP_H_*/





