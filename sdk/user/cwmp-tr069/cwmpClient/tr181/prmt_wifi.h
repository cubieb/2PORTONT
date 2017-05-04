#ifndef _PRMT_TR181_WIFI_H_
#define _PRMT_TR181_WIFI_H_

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_LEAF tWiFiLeaf[];
extern struct CWMP_NODE tWiFiObject[];

int getWiFi(char *name, struct CWMP_LEAF *entity, int *type, void **data);


#ifdef __cplusplus
}
#endif

#endif /*_PRMT_TR181_WIFI_H_*/

