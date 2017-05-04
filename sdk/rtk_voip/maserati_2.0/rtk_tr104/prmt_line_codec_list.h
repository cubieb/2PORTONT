#ifndef __PRMT_VOICE_VOICE_PROFILE_LINE_CODEC_LIST_H__
#define __PRMT_VOICE_VOICE_PROFILE_LINE_CODEC_LIST_H__

#include "prmt_igd.h"

extern struct CWMP_LEAF tLineCodecListEntityLeaf[];
extern struct CWMP_LINKNODE tLineCodecListObject[];

int objLineCodecList(char *name, struct CWMP_LEAF *e, int type, void *data);
int getLineCodecListEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setLineCodecListEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

#endif /* __PRMT_VOICE_VOICE_PROFILE_LINE_CODEC_LIST_H__ */
