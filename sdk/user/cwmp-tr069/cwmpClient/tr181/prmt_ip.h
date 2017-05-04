#ifndef _PRMT_TR181_IP_H_
#define _PRMT_TR181_IP_H_

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_LEAF tIPLeaf[];
extern struct CWMP_NODE tIPObject[];

int getIP(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int getIPIfEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int objIPIf(char *name, struct CWMP_LEAF *e, int type, void *data);
int getIPv4AddrEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int getIPIfStats(char *name, struct CWMP_LEAF *entity, int *type, void **data);


#ifdef __cplusplus
}
#endif

#endif /*_PRMT_TR181_IP_H_*/


