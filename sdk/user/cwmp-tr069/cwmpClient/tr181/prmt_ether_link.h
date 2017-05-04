#ifndef _PRMT_TR181_ETHER_LINK_H_
#define _PRMT_TR181_ETHER_LINK_H_

#ifdef __cplusplus
extern "C" {
#endif

//extern struct node *gDummyEthLinkObjList;
extern struct CWMP_LEAF tEtherLinkLeaf[];
extern struct CWMP_LINKNODE tEtherLinkObject[];

/* Utility Functions*/
int get_eth_link_cnt();

/* Operations */
int objEtherLink(char *name, struct CWMP_LEAF *e, int type, void *data);
int getEtherLinkEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setEtherLinkEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);
int getEtherLinkStats(char *name, struct CWMP_LEAF *entity, int *type, void **data);

#ifdef __cplusplus
}
#endif

#endif /*_PRMT_TR181_ETHER_LINK_H_*/



