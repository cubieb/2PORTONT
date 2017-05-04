#ifndef _PRMT_DDNS_H_
#define _PRMT_DDNS_H_
#include "prmt_igd.h"

#if defined(_PRMT_X_CT_COM_DDNS_) && defined(CONFIG_USER_DDNS)
#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_LEAF tDDNSEntityLeaf[];
extern struct CWMP_LINKNODE tDDNSObject[];

int getDDNSEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setDDNSEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);
int objDDNS(char *name, struct CWMP_LEAF *entity, int type, void *data);

#ifdef __cplusplus
}
#endif
#endif /*_PRMT_X_CT_COM_DDNS_*/


#endif /*_PRMT_DDNS_H_*/
