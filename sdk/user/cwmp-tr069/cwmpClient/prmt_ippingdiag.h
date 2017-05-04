#ifndef _PRMT_IPPINGDIAG_H_
#define _PRMT_IPPINGDIAG_H_

#include "prmt_igd.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_LEAF tIPPingDiagnosticsLeaf[];

int getIPPingDiag(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setIPPingDiag(char *name, struct CWMP_LEAF *entity, int type, void *data);


extern int gStartPing;
void cwmpStartPingDiag();

#ifdef __cplusplus
}
#endif

#endif /*_PRMT_IPPINGDIAG_H_*/
