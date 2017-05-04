#ifndef _PRMT_TR181_DHCPV4_H_
#define _PRMT_TR181_DHCPV4_H_

#include <parameter_api.h>

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_LEAF tDHCPv4Leaf[];
extern struct CWMP_NODE tDHCPv4Object[];

int getDHCPv4(char *name, struct CWMP_LEAF *entity, int *type, void **data);

#ifdef __cplusplus
}
#endif

#endif /*_PRMT_TR181_DHCPV4_H_ */


