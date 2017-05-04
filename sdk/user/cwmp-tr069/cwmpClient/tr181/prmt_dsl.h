#ifndef _PRMT_TR181_DSL_H_
#define _PRMT_TR181_DSL_H_

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_LEAF tDSLLeaf[];
extern struct CWMP_NODE tDSLObject[];

int getDSL(char *name, struct CWMP_LEAF *entity, int *type, void **data);

#ifdef __cplusplus
}
#endif

#endif /*_PRMT_TR181_DSL_H_*/

