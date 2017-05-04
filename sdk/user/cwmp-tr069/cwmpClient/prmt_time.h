#ifndef _PRMT_TIME_H_
#define _PRMT_TIME_H_

#include "prmt_igd.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef TIME_ZONE

extern struct CWMP_LEAF tTimeLeaf[];

int getTime(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setTime(char *name, struct CWMP_LEAF *entity, int type, void *data);

#endif /*TIME_ZONE*/
	
#ifdef __cplusplus
}
#endif
#endif /*_PRMT_TIME_H_*/
