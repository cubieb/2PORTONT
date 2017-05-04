#ifndef _PRMT_LANDEVICE_WLAN_H_
#define _PRMT_LANDEVICE_WLAN_H_

#include "prmt_igd.h"

#ifdef WLAN_SUPPORT
#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_LEAF tPreSharedKeyEntityLeaf[];
extern struct CWMP_NODE tPreSharedKeyObject[];
extern struct CWMP_LEAF tWEPKeyEntityLeaf[];
extern struct CWMP_NODE tWEPKeyObject[];
extern struct CWMP_LEAF tAscDeviceEntityLeaf[];
extern struct CWMP_LINKNODE tAscDeviceObject[];
extern struct CWMP_LEAF tWLANConfEntityLeaf[];
extern struct CWMP_NODE tWLANConfEntityObject[];
extern struct CWMP_NODE tWLANConfigObject[];

int getPreSharedKeyEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setPreSharedKeyEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getWEPKeyEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setWEPKeyEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getAscDeviceEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int objAscDevice(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getWLANConf(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setWLANConf(char *name, struct CWMP_LEAF *entity, int type, void *data);

#ifdef CONFIG_WIFI_SIMPLE_CONFIG
int getWLANWPS(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setWLANWPS(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getWPSRegistrarEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setWPSRegistrarEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif

#ifdef CTCOM_WLAN_REQ
int objWLANConfiguration(char *name, struct CWMP_LEAF *e, int type, void *data);
#endif

#ifdef __cplusplus
}
#endif
#endif /*#ifdef WLAN_SUPPORT*/
#endif /*_PRMT_LANDEVICE_WLAN_H_*/
