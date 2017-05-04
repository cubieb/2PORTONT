#ifndef __PRMT_VOICE_SERVICE_H__
#define __PRMT_VOICE_SERVICE_H__

#include "prmt_igd.h"

//extern struct sCWMP_ENTITY tVoiceService[];

//int objVoiceService(char *name, struct sCWMP_ENTITY *entity, int type, void *data);

extern struct CWMP_LEAF tVoiceServiceEntityLeaf[];
extern struct CWMP_NODE tVoiceServiceEntityObject[];
extern struct CWMP_LINKNODE tVoiceServiceObject[];
extern struct CWMP_NODE tVSRoot[];

int objVoiceService(char *name, struct CWMP_LEAF *e, int type, void *data);
int getVSEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);


#endif /* __PRMT_VOICE_SERVICE_H__ */

