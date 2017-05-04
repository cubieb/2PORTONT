#ifndef _PRMT_TR181_DSL_CHANNEL_H_
#define _PRMT_TR181_DSL_CHANNEL_H_

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_NODE tDSLChannelObject[];
extern struct CWMP_LEAF tDSLChannelEntityLeaf[];
extern struct CWMP_NODE tDSLChannelEntityObject[];

int getDSLChannelStatsTotal(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int getDSLChannelStatsShowtime(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int getDSLChannelStats(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int getDSLChannelEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setDSLChannelEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

#ifdef __cplusplus
}
#endif

#endif /*_PRMT_TR181_DSL_CHANNEL_H_*/



