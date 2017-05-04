#ifndef _PRMT_TR181_BRIDGE_H_
#define _PRMT_TR181_BRIDGE_H_

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_LEAF tBridgingLeaf[];
extern struct CWMP_NODE tBridgingObject[];

int getBridging(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int getBridgingBridgeEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setBridgingBridgeEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);


#ifdef __cplusplus
}
#endif

#endif /*_PRMT_TR181_BRIDGE_H_*/



