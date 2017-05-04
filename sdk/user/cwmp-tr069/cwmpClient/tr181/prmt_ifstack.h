#ifndef _PRMT_TR181_IFSTACK_H_
#define _PRMT_TR181_IFSTACK_H_

#include <linux/autoconf.h>
#include <parameter_api.h>


#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_OP tIfStackEntityOP;
extern struct CWMP_LINKNODE *gIfStackObjList;

// For appling new network settings
struct IfStackObjData
{
	char *lowerLayer;
	char *higherLayer;
};

int objIfStack(char *name, struct CWMP_LEAF *e, int type, void *data);
int getIfStackEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);

#ifdef __cplusplus
}
#endif

#endif /*_PRMT_TR181_IFSTACK_H_*/




