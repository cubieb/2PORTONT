#ifndef _PRMT_QUEUEMNT_H_
#define _PRMT_QUEUEMNT_H_

#include "prmt_igd.h"

#ifdef __cplusplus
extern "C" {
#endif
#ifdef _PRMT_X_CT_COM_QOS_
extern struct CWMP_LEAF tCT_UplinkQoSLeaf[];
extern struct CWMP_NODE tCT_UplinkQoSObject[];
int getCT_UplinkQoS(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_UplinkQoS(char *name, struct CWMP_LEAF *entity, int type, void *data);

extern struct CWMP_LEAF tCT_ClassEntityLeaf[];
int getCT_ClassEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_ClassEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

extern struct CWMP_LINKNODE tCT_ClassObject[];
int objCT_Class(char *name, struct CWMP_LEAF *entity, int type, void *data);

extern struct CWMP_LEAF tCT_AppEntityLeaf[];
int getCT_AppEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_AppEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

extern struct CWMP_LINKNODE tCT_AppObject[];
int objCT_App(char *name, struct CWMP_LEAF *entity, int type, void *data);

extern struct CWMP_LEAF tCT_TypeEntityLeaf[];
int getCT_TypeEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_TypeEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);
extern struct CWMP_LINKNODE tCT_TypeObject[];
int objCT_Type(char *name, struct CWMP_LEAF *e, int type, void *data);

#if 0
extern struct sCWMP_ENTITY tPolicerEntity[];
int getPolicerEntity(char *name, struct sCWMP_ENTITY *entity, int *type, void **data);
int setPolicerEntity(char *name, struct sCWMP_ENTITY *entity, int type, void *data);

extern struct sCWMP_ENTITY tPolicer[];
int objPolicer(char *name, struct sCWMP_ENTITY *entity, int type, void *data);
#endif

extern struct CWMP_LEAF tCT_QueueEntityLeaf[];
int getCT_QueueEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_QueueEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

extern struct CWMP_LINKNODE tCT_QueueObject[];
int objCT_Queue(char *name, struct CWMP_LEAF *entity, int type, void *data);

#endif
#ifdef _PRMT_X_STD_QOS_

extern struct CWMP_LEAF tQueueMntLeaf[];
extern struct CWMP_NODE tQueueMntObject[];
int getQueueMnt(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setQueueMnt(char *name, struct CWMP_LEAF *entity, int type, void *data);

extern struct CWMP_LEAF tClassEntityLeaf[];
int getClassEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setClassEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

extern struct CWMP_LINKNODE tClassObject[];
int objClass(char *name, struct CWMP_LEAF *entity, int type, void *data);

#if 0
extern struct sCWMP_ENTITY tPolicerEntity[];
int getPolicerEntity(char *name, struct sCWMP_ENTITY *entity, int *type, void **data);
int setPolicerEntity(char *name, struct sCWMP_ENTITY *entity, int type, void *data);

extern struct sCWMP_ENTITY tPolicer[];
int objPolicer(char *name, struct sCWMP_ENTITY *entity, int type, void *data);
#endif

extern struct CWMP_LEAF tQueueEntityLeaf[];
int getQueueEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setQueueEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

extern struct CWMP_LINKNODE tQueueObject[];
int objQueue(char *name, struct CWMP_LEAF *entity, int type, void *data);

#endif /*IP_QOS*/
#ifdef __cplusplus
}
#endif
#endif /*_PRMT_QUEUEMNT_H_*/
