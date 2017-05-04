#ifndef _PRMT_TR181_BRIDGE_PORT_H_
#define _PRMT_TR181_BRIDGE_PORT_H_

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_LEAF tBRPortLeaf[];
extern struct CWMP_NODE tBRPortObject[];

//Utitlity Functions
int get_br_port_cnt();

//Operations
int objBRPort(char *name, struct CWMP_LEAF *e, int type, void *data);
int getBRPortEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int getBRPortStats(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setBRPortEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);


#ifdef __cplusplus
}
#endif

#endif /*_PRMT_TR181_BRIDGE_PORT_H_*/




