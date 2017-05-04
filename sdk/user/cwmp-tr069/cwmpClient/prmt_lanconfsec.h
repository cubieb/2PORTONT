#ifndef _PRMT_LANCONFSEC_H_
#define _PRMT_LANCONFSEC_H_

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_LEAF tLANConfigSecurityLeaf[];

int getLANConfSec(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setLANConfSec(char *name, struct CWMP_LEAF *entity, int type, void *data);

#ifdef __cplusplus
}
#endif

#endif /*_PRMT_LANCONFSEC_H_*/
