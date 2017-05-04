#ifndef _PRMT_TR181_PTM_H_
#define _PRMT_TR181_PTM_H_

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_LEAF tPTMLeaf[];
extern struct CWMP_NODE tPTMObject[];

int getPTM(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int objPTMLink(char *name, struct CWMP_LEAF *e, int type, void *data);
int getPTMLinkEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setPTMLinkEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);
int getPTMLinkStats(char *name, struct CWMP_LEAF *entity, int *type, void **data);

#ifdef __cplusplus
}
#endif

#endif /*_PRMT_TR181_PTM_H_*/



