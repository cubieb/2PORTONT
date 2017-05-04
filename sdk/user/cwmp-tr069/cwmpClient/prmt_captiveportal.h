#ifndef _PRMT_CAPTIVEPORTAL_H_
#define _PRMT_CAPTIVEPORTAL_H_

#include "prmt_igd.h"
#ifdef __cplusplus
extern "C" {
#endif


#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
extern struct CWMP_LEAF tCaptivePortalLeaf[];
int getCaptivePortal(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCaptivePortal(char *name, struct CWMP_LEAF *entity, int type, void *data);
#define FILE4CaptivePortal	"/tmp/CaptivePortalAllowedList.txt"
#endif //_SUPPORT_CAPTIVEPORTAL_PROFILE_



#ifdef __cplusplus
}
#endif
#endif /*_PRMT_CAPTIVEPORTAL_H_*/
