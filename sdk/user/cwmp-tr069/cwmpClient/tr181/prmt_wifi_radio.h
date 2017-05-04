#ifndef _PRMT_TR181_WIFI_RADIO_H_
#define _PRMT_TR181_WIFI_RADIO_H_

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_NODE tWiFiRadioObject[];

int getWiFiRadioEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setWiFiRadioEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);
int getWiFiRadioStats(char *name, struct CWMP_LEAF *entity, int *type, void **data);


#ifdef __cplusplus
}
#endif

#endif /*_PRMT_TR181_WIFI_RADIO_H_*/


