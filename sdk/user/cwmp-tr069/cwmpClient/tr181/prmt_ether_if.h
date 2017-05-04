#ifndef _PRMT_TR181_ETHER_IF_H_
#define _PRMT_TR181_ETHER_IF_H_

#include <linux/autoconf.h>
#include "../prmt_landevice_eth.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CWMP_ETH_LAN_IF_NUM CWMP_LANETHIFNO
#define CWMP_ETH_LAN_BASE 1
#define CWMP_ETH_WAN_BASE (CWMP_ETH_LAN_IF_NUM + 1)

int get_eth_if_cnt();

int objEtherIf(char *name, struct CWMP_LEAF *e, int type, void *data);
int getEtherIfEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int getEtherIfStats(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setEtherIfEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

#ifdef __cplusplus
}
#endif

#endif /*_PRMT_TR181_ETHER_IF_H_*/



