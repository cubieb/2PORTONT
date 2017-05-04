#ifndef _PRMT_TR181_DHCPV4_RELAY_H_
#define _PRMT_TR181_DHCPV4_RELAY_H_

#include <parameter_api.h>

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_LEAF tDHCPv4RelayLeaf[];
extern struct CWMP_NODE tDHCPv4RelayObject[];

int getDHCPv4Relay(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setDHCPv4Relay(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getDHCPv4RelayFWEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setDHCPv4RelayFWEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

#ifdef __cplusplus
}
#endif

#endif /*_PRMT_TR181_DHCPV4_RELAY_H_ */

