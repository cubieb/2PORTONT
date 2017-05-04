#ifndef _PRMT_LAYER2BR_H_
#define _PRMT_LAYER2BR_H_

#include "prmt_igd.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_LEAF tLayer2BridgingLeaf[];
extern struct CWMP_NODE tLayer2BridgingObject[];

int getLayer2Br(char* name, struct CWMP_LEAF* entity, int* type, void** data);

int getLayer2BrBridge(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setLayer2BrBridge(char *name, struct CWMP_LEAF *entity, int type, void *data);
int objLayer2BrBridgeOP(char *name, struct CWMP_LEAF *e, int type, void *data);

int getLayer2BrFilter(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setLayer2BrFilter(char *name, struct CWMP_LEAF *entity, int type, void *data);
int objLayer2BrFilterOP(char *name, struct CWMP_LEAF *e, int type, void *data);

int getLayer2BrMarking(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setLayer2BrMarking(char *name, struct CWMP_LEAF *entity, int type, void *data);
int objLayer2BrMarkingOP(char *name, struct CWMP_LEAF *e, int type, void *data);

int getLayer2BrAvailableInterface(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int objLayer2BrAvailableInterfaceOP(char *name, struct CWMP_LEAF *e, int type, void *data);

#ifdef __cplusplus
}
#endif

#endif /*end of _PRMT_LAYER2BR_H_*/
