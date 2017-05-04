#ifndef _PRMT_TR181_PPP_H_
#define _PRMT_TR181_PPP_H_

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_LINKNODE *gPPPIfEntityObjList;
extern struct CWMP_LEAF tPPPLeaf[];
extern struct CWMP_NODE tPPPObject[];

// For appling new network settings
struct PPPIfObjData
{
	char *LowerLayers;
};

int getPPP(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int objPPPIf(char *name, struct CWMP_LEAF *entity, int type, void *data);
int getPPPIfEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int getPPPoE(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int getPPPIfStats(char *nae, struct CWMP_LEAF *entity, int *type, void **data);
int setPPPIfEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);


#ifdef __cplusplus
}
#endif

#endif /*_PRMT_TR181_PPP_H_*/

