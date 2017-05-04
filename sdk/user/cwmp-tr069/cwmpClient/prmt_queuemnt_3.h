#ifndef _PRMT_QUEUEMNT_3_H_
#define _PRMT_QUEUEMNT_3_H_

#include "prmt_igd.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct IPQosQueueStats
{
	struct IPQosQueueStats *next;
	unsigned char enable;
	unsigned char queueInstNum;
	unsigned int instanceNum;
} QUEUE_STATS;

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

#ifdef __cplusplus
}
#endif
#endif /*_PRMT_QUEUEMNT_3_H_*/
