#ifndef _PRMT_TR181_IP_DIAG_H_
#define _PRMT_TR181_IP_DIAG_H_
	
#ifdef __cplusplus
	extern "C" {
#endif
	
extern struct CWMP_NODE tIPDiagObject[];
int getTR181DownloadDiagnostics(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setTR181DownloadDiagnostics(char *name, struct CWMP_LEAF *entity, int type, void *data);
int getTR181UploadDiagnostics(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setTR181UploadDiagnostics(char *name, struct CWMP_LEAF *entity, int type, void *data);
int getTR181UDPEchoConfig(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setTR181UDPEchoConfig(char *name, struct CWMP_LEAF *entity, int type, void *data);

#ifdef __cplusplus
	}
#endif
	
#endif /*_PRMT_TR181_IP_DIAG_H_*/
	
	

