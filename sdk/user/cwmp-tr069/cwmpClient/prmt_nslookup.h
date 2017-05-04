#ifndef _PRMT_NSLOOKUP_H_
#define _PRMT_NSLOOKUP_H_

#include "prmt_igd.h"
#ifdef __cplusplus
extern "C" {
#endif

#ifdef _PRMT_NSLOOKUP_
extern struct CWMP_NODE tNSLookupDiagObject[];
extern struct CWMP_LEAF tNSLookupDiagLeaf[];
int getNSLookupDiag(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setNSLookupDiag(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getResultEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);

extern int gStartNSLookupDiag;
void StartNSLookupDiag();
#endif //_SUPPORT_TRACEROUTE_PROFILE_

#ifdef __cplusplus
}
#endif
#endif /*_PRMT_TRACEROUTE_H_*/

