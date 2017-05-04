#ifndef _PRMT_LANDEVICE_ETH_H_
#define _PRMT_LANDEVICE_ETH_H_

#include "prmt_igd.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(CONFIG_RTL_MULTI_LAN_DEV)
#define CWMP_LANETHIFNO		SW_LAN_PORT_NUM
#else
#define CWMP_LANETHIFNO		1
#endif //CONFIG_RTL_MULTI_LAN_DEV

extern struct CWMP_LINKNODE tLANEthConfObject[];
int objLANEthConf(char *name, struct CWMP_LEAF *e, int type, void *data);
int getLANEthConf(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setLANEthConf(char *name, struct CWMP_LEAF *entity, int type, void *data);
int getLANEthStats(char *name, struct CWMP_LEAF *entity, int *type, void **data);


#ifdef __cplusplus
}
#endif
#endif /*_PRMT_LANDEVICE_ETH_H_*/
