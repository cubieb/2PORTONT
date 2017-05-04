#ifndef _PRMT_TR181_ETHER_H_
#define _PRMT_TR181_ETHER_H_

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_LEAF tEtherLeaf[];
extern struct CWMP_NODE tEtherObject[];

int getEther(char *name, struct CWMP_LEAF *entity, int *type, void **data);

#ifdef __cplusplus
}
#endif

#endif /*_PRMT_TR181_ETHER_H_*/

