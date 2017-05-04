#ifndef _PRMT_USERINTERFACE_H_
#define _PRMT_USERINTERFACE_H_

#include "prmt_igd.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _PRMT_USERINTERFACE_

extern struct CWMP_LEAF tUserIFLeaf[];

int getUserIF(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setUserIF(char *name, struct CWMP_LEAF *entity, int type, void *data);

#endif /*_PRMT_USERINTERFACE_*/
	
#ifdef __cplusplus
}
#endif
#endif /*_PRMT_USERINTERFACE_H_*/
