#ifndef _PRMT_MNGMTSERVER_H_
#define _PRMT_MNGMTSERVER_H_

#include "prmt_igd.h"

#ifdef __cplusplus
extern "C" {
#endif

#define _TR_111_PRMT_
#ifdef _TR_111_PRMT_
//#define TR111_DEVICEFILE	"/tmp/tr111device.txt"
#define TR111_DEVICEFILE	"/var/udhcpd/tr111device.txt"
extern struct CWMP_LEAF tManageDevEntityLeaf[];
extern struct CWMP_LINKNODE tManageableDeviceObject[];
int getManageDevEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int objManageDevice(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif /*TR111_DEVICEFILE*/

extern struct CWMP_LEAF tManagementServerLeaf[];
#ifdef _TR_111_PRMT_
extern struct CWMP_NODE tManagementServerObject[];
#else
#define tManagementServerObject NULL
#endif /*_TR_111_PRMT_*/

int getMngmntServer(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setMngmntServer(char *name, struct CWMP_LEAF *entity, int type, void *data);

#ifdef __cplusplus
}
#endif

#endif /*_PRMT_MNGMTSERVER_H_*/
