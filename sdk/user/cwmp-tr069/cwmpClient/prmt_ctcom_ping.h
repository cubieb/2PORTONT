#ifndef _PRMT_CTCOM_PING_H_
#define _PRMT_CTCOM_PING_H_

#include "prmt_igd.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _PRMT_X_CT_COM_PING_
extern struct CWMP_LEAF tCT_PingLeaf[];
extern struct CWMP_NODE tCT_PingObject[];

int getCT_Ping(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_Ping(char *name, struct CWMP_LEAF *entity, int type, void *data);
int objCT_PingConfig(char *name, struct CWMP_LEAF *e, int type, void *data);
int getCT_PingEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_PingEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

#endif

#ifdef __cplusplus
}
#endif
#endif /*_PRMT_CTCOM_PING_H_*/

