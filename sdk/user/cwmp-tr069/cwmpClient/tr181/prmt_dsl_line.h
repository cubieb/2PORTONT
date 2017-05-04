#ifndef _PRMT_TR181_DSL_LINE_H_
#define _PRMT_TR181_DSL_LINE_H_

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_NODE tDSLLineObject[];
extern struct CWMP_LEAF tDSLLineEntityLeaf[];
extern struct CWMP_NODE tDSLLineEntityObject[];

int getDSLLineStatsTotal(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int getDSLLineStatsShowtime(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int getDSLLineStats(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int getDSLLineEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setDSLLineEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

#ifdef __cplusplus
}
#endif

#endif /*_PRMT_TR181_DSL_LINE_H_*/


