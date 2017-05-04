#ifndef _PRMT_SERVICES_H_
#define _PRMT_SERVICES_H_

#include "prmt_igd.h"

#ifdef __cplusplus
extern "C" {
#endif
#ifdef _PRMT_SERVICES_

extern struct CWMP_NODE tServicesObject[];

#ifdef _PRMT_X_CT_COM_IPTV_
extern struct CWMP_LEAF tCT_IPTVLeaf[];
int getCT_IPTV(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_IPTV(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif

#ifdef _PRMT_X_CT_COM_MWBAND_
extern struct CWMP_LEAF tCT_MWBANDLeaf[];
int getCT_MWBAND(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_MWBAND(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif#ifdef _PRMT_X_CT_COM_USERINFO_
extern struct CWMP_LEAF tCT_UserInfoLeaf[];
int getCT_UserInfo(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_UserInfo(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif

#endif /*_PRMT_SERVICES_*/	
#ifdef __cplusplus
}
#endif
#endif /*_PRMT_SERVICES_H_*/
