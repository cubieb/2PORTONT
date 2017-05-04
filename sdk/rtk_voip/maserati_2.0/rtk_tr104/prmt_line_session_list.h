#ifndef __PRMT_VOICE_VOICE_PROFILE_LINE_SESSION_LIST_H__
#define __PRMT_VOICE_VOICE_PROFILE_LINE_SESSION_LIST_H__

#include "prmt_igd.h"

extern struct CWMP_LEAF tLineSessionListEntityLeaf[];
extern struct CWMP_LINKNODE tLineSessionListObject[];

int objLineSessionList(char *name, struct CWMP_LEAF *e, int type, void *data);
int getLineSessionListEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setLineSessionListEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

#endif /* __PRMT_VOICE_VOICE_PROFILE_LINE_SESSION_LIST_H__ */

