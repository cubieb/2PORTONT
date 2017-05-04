#ifndef _PRMT_TR181_WIFI_SSID_H_
#define _PRMT_TR181_WIFI_SSID_H_

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_NODE tWiFiSSIDObject[];

int getWiFiSSIDEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setWiFiSSIDEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);
int getWiFiSSIDStats(char *name, struct CWMP_LEAF *entity, int *type, void **data);

#ifdef __cplusplus
}
#endif

#endif /*_PRMT_TR181_WIFI_SSID_H_*/


