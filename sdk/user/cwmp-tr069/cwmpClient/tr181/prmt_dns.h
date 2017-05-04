#ifndef _PRMT_TR181_DNS_H_
#define _PRMT_TR181_DNS_H_

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_NODE tDNSObject[];

int getDNSClient(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int getDNSCServerEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setDNSCServerEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);
int objDNSCServer(char *name, struct CWMP_LEAF *e, int type, void *data);	

#ifdef __cplusplus
}
#endif

#endif /*_PRMT_TR181_DNS_H_*/
