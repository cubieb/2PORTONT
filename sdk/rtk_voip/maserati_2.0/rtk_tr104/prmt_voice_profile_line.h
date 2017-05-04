#ifndef __PRMT_VOICE_VOICE_PROFILE_LINE_H__
#define __PRMT_VOICE_VOICE_PROFILE_LINE_H__

#include "prmt_igd.h"

extern struct CWMP_LEAF tVPLineStatsEntityLeaf[];
extern struct CWMP_NODE tVPLineCodecEntityObject[];
extern struct CWMP_LEAF tVPLineVoiceProcessingEntityLeaf[];
extern struct CWMP_LEAF tVPLineXCT_IMSEntityLeaf[];
extern struct CWMP_LEAF tVPLineEntityLeaf[];
extern struct CWMP_NODE tVPLineEntityObject[];
extern struct CWMP_LINKNODE tVPLineObject[];

//extern int objVoiceProfileLine(char *name, struct sCWMP_ENTITY *entity, int type, void *data);
int objVoiceProfileLine(char *name, struct CWMP_LEAF *e, int type, void *data);

/* VoiceService.{i}.VoiceProfile.{i}.Line.{i}. */
int getVoiceProfileLineEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setVoiceProfileLineEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getLineStatsEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setLineStatsEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

/* VoiceService.{i}.VoiceProfile.{i}.Line.{i}.Stats. */
int getLineStatsEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setLineStatsEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

/* VoiceService.{i}.VoiceProfile.{i}.Line.{i}.codecc. */

int getLineCodecEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setLineCodecEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);


/* VoiceService.{i}.VoiceProfile.{i}.SIP.Line.{i}.SIP. */
int getLineSipEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setLineSipEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

/* VoiceService.{i}.VoiceProfile.{i}.SIP.Line.{i}.VoiceProcessing. */
int getLineVoiceProcessingEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setLineVoiceProcessingEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

/* VoiceService.{i}.VoiceProfile.{i}.SIP.Line.{i}.CallingFeatures. */
int getLineCallingFeaturesEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setLineCallingFeaturesEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);


/* VoiceService.{i}.VoiceProfile.{i}.SIP.Line.{i}.X_CT-COM_IMS. */
int getLineXCT_IMSEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setLineXCT_IMSEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif /* __PRMT_VOICE_VOICE_PROFILE_LINE_H__ */

