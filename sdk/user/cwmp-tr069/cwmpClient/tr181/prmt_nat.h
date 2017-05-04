#ifndef _PRMT_TR181_NAT_H_
#define _PRMT_TR181_NAT_H_

#include <parameter_api.h>

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_LEAF tNATLeaf[];
extern struct CWMP_NODE tNATObject[];

int getNAT(char *name, struct CWMP_LEAF *entity, int *type, void **data);

int objNATIfSetting(char *name, struct CWMP_LEAF *e, int type, void *data);
int getNATIfSettingEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setNATIfSettingEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

int objNATPortMapping(char *name, struct CWMP_LEAF *e, int type, void *data);
int getNATPortMappingEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setNATPortMappingEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);



#ifdef __cplusplus
}
#endif

#endif /*_PRMT_TR181_NAT_H_ */



