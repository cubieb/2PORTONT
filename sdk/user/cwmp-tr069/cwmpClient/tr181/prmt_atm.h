#ifndef _PRMT_TR181_ATM_H_
#define _PRMT_TR181_ATM_H_

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_LEAF tATMLeaf[];
extern struct CWMP_NODE tATMObject[];

int getATM(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int objATMLink(char *name, struct CWMP_LEAF *e, int type, void *data);
int getATMLinkEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setATMLinkEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);
int getATMLinkStats(char *name, struct CWMP_LEAF *entity, int *type, void **data);

int getATMF5LB(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setATMF5LB(char *name, struct CWMP_LEAF *entity, int type, void *data);



#ifdef __cplusplus
}
#endif

#endif /*_PRMT_TR181_ATM_H_*/


