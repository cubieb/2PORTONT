#ifndef _PRMT_TR181_DSL_DIAG_H_
#define _PRMT_TR181_DSL_DIAG_H_

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_NODE tDSLDiagObject[];

int getDSLLineTest(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setDSLLineTest(char *name, struct CWMP_LEAF *entity, int type, void *data);

#ifdef __cplusplus
}
#endif

#endif /*_PRMT_TR181_DSL_LINE_H_*/



