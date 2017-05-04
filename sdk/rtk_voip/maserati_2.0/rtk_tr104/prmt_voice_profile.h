#ifndef __PRMT_VOICE_VOICE_PROFILE_H__
#define __PRMT_VOICE_VOICE_PROFILE_H__

#include "prmt_igd.h"

extern struct CWMP_LEAF tVPServiceProviderEntityLeaf[];
extern struct CWMP_LEAF tVPSIPEntityLeaf[];
extern struct CWMP_LEAF tVPRTPEntityLeaf[];
extern struct CWMP_LEAF tVPNumberingPlanEntityLeaf[];
extern struct CWMP_LEAF tVPFaxT38EntityLeaf[];
extern struct CWMP_LEAF tVPXCT_G711FAXEntityLeaf[];
extern struct CWMP_LEAF tVPXCT_IADDiagEntityLeaf[];
extern struct CWMP_LEAF tVoiceProfileEntityLeaf[];
extern struct CWMP_NODE tVoiceProfileEntityObject[];
extern struct CWMP_LINKNODE tVoiceProfileObject[];

int objVoiceProfile(char *name, struct CWMP_LEAF *e, int type, void *data);

/* VoiceService.{i}.VoiceProfile.{i}. */
int getVoiceProfileEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setVoiceProfileEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

/* VoiceService.{i}.VoiceProfile.{i}.SIP */
int getVPSipEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setVPSipEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

/* VoiceService.{i}.VoiceProfile.{i}.RTP */
int getVPRTPEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setVPRTPEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

/* VoiceService.{i}.VoiceProfile.{i}.NumberingPlan */
int getVPNumberingPlanEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setVPNumberingPlanEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

/* VoiceService.{i}.VoiceProfile.{i}.FaxT38 */
int getVPFaxT38Entity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setVPFaxT38Entity(char *name, struct CWMP_LEAF *entity, int type, void *data);

/* VoiceService.{i}.VoiceProfile.{i}.X_G711FAX */
int getVPXCT_G711FAXEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setVPXCT_G711FAXEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

/* VoiceService.{i}.VoiceProfile.{i}.X_IADDiagnostics */
int getVPXCT_IADDiagEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setVPXCT_IADDiagEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

/* VoiceService.{i}.VoiceProfile.{i}.ServiceProviderInfo */
int getVPServiceProviderEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setVPServiceProviderEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);


#endif /* __PRMT_VOICE_VOICE_PROFILE_H__ */

