#ifndef _PRMT_TR181_IP_IF_H_
#define _PRMT_TR181_IP_IF_H_

#include <parameter_api.h>
#include <rtk/utility.h>

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_LINKNODE *gIPIfEntityObjList;

// For appling new network settings
struct IPIfEntityData
{
	char 			*LowerLayers;
};

/* Operations */
int getIPIfEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setIPIfEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);
int objIPIf(char *name, struct CWMP_LEAF *e, int type, void *data);
int getIPv4AddrEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setIPv4AddrEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);
int getIPv6AddrEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int objIPv6Addr(char *name, struct CWMP_LEAF *e, int type, void *data);
int getIPv6PrefixEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int objIPv6Prefix(char *name, struct CWMP_LEAF *e, int type, void *data);
int getIPIfStats(char *name, struct CWMP_LEAF *entity, int *type, void **data);

/* Public Utilities */
int inline get_ip_if_cnt();
int get_ip_if_ifname(char *name, char *ifname);
int ifIdex_to_ip_if_path( int ifindex, char *path );
int get_ip_if_info(int num, MIB_CE_ATM_VC_T *pEntry, int *idx, char *ifname);
struct IPIfEntityData* get_ip_if_entity_data(int num);

#ifdef __cplusplus
}
#endif

#endif /*_PRMT_TR181_IP_IF_H_*/



