#ifndef _PRMT_TR181_ROUTING_H_
#define _PRMT_TR181_ROUTING_H_

#include <parameter_api.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ROUTING
extern struct CWMP_LEAF tRoutingLeaf[];
#endif
extern struct CWMP_NODE tRoutingObject[];

#ifdef ROUTING
int getRouting(char *name, struct CWMP_LEAF *entity, int *type, void **data);

int getRouterEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);

int objIPv4Fw(char *name, struct CWMP_LEAF *e, int type, void *data);
int getIPv4FwEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setIPv4FwEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getRoutingRIP(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setRoutingRIP(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif

int objRIPIfSetting(char *name, struct CWMP_LEAF *e, int type, void *data);
int getRIPIfSettingEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setRIPIfSettingEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);





#ifdef __cplusplus
}
#endif

#endif /*_PRMT_TR181_ROUTING_H_ */



