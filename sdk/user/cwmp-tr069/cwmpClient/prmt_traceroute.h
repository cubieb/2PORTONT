#ifndef _PRMT_TRACEROUTE_H_
#define _PRMT_TRACEROUTE_H_

#include "prmt_igd.h"
#ifdef __cplusplus
extern "C" {
#endif


#ifdef _SUPPORT_TRACEROUTE_PROFILE_
extern struct CWMP_NODE tTraceRouteDiagObject[];
extern struct CWMP_LEAF tTraceRouteDiagLeaf[];

int getTraceRouteDiag(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setTraceRouteDiag(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getRouteHopsEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);

extern int gStartTraceRouteDiag;
void StartTraceRouteDiag(void);
#endif //_SUPPORT_TRACEROUTE_PROFILE_



#ifdef __cplusplus
}
#endif
#endif /*_PRMT_TRACEROUTE_H_*/
