#ifndef __PRMT_VOICE_CAPABILITIES_H__
#define __PRMT_VOICE_CAPABILITIES_H__

#include "prmt_igd.h"

//extern struct sCWMP_ENTITY tCapabilitiesEntity[];
//extern const lstCodecs_t lstCodecs[];

typedef struct lstCodecs_s {
	const char *pszCodec;
	unsigned int nBitRate;
	const char *pszPacketizationPeriod;
	unsigned int bSilenceSupression;
} lstCodecs_t;
extern const lstCodecs_t lstCodecs[];

extern struct CWMP_LEAF tVSCapSIPLeaf[];
extern struct CWMP_LEAF tVSCapCodecsLeaf[];
extern struct CWMP_LINKNODE tVSCapCodecsObject[];
extern struct CWMP_LEAF tVSCapabilitiesLeaf[];
extern struct CWMP_NODE tVSCapabilitiesObject[];

int objCapabilitiesCodecs(char *name, struct CWMP_LEAF *e, int type, void *data);
int getCapabilitiesCodecsEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int getCapabilitiesSipEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int getCapabilitiesEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);

#endif /* __PRMT_VOICE_CAPABILITIES_H__ */

