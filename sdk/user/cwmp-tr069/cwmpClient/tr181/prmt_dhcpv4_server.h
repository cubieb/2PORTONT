#ifndef _PRMT_TR181_DHCPV4_SERVER_H_
#define _PRMT_TR181_DHCPV4_SERVER_H_

#include <parameter_api.h>

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_LEAF tDHCPv4ServerLeaf[];
extern struct CWMP_NODE tDHCPv4ServerObject[];

int getDHCPv4Server(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setDHCPv4Server(char *name, struct CWMP_LEAF *entity, int type, void *data);

int objDHCPv4ServerPool(char *name, struct CWMP_LEAF *e, int type, void *data);
int getDHCPv4ServerPoolEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setDHCPv4ServerPoolEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

int objDHCPv4PoolOpt(char *name, struct CWMP_LEAF *e, int type, void *data);
int getDHCPv4PoolOptEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setDHCPv4PoolOptEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

#ifdef __cplusplus
}
#endif

#endif /*_PRMT_TR181_DHCPV4_SERVER_H_ */

