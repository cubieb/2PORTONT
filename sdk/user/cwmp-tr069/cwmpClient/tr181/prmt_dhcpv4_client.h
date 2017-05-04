#ifndef _PRMT_TR181_DHCPV4_CLIENT_H_
#define _PRMT_TR181_DHCPV4_CLIENT_H_

#include <parameter_api.h>

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_OP tDHCPv4Client_OP;

int objDHCPv4Client(char *name, struct CWMP_LEAF *e, int type, void *data);
int getDHCPv4ClientEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setDHCPv4ClientEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

int objDHCPv4ClientSentOpt(char *name, struct CWMP_LEAF *e, int type, void *data);
int objDHCPv4ClientReqOpt(char *name, struct CWMP_LEAF *e, int type, void *data);
int getDHCPv4ClientOptEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setDHCPv4ClientOptENTITY(char *name, struct CWMP_LEAF *entity, int type, void *data);


#ifdef __cplusplus
}
#endif

#endif /*_PRMT_TR181_DHCPV4_CLIENT_H_ */



