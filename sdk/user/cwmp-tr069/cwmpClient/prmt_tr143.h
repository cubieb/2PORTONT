#ifndef _PRMT_TR143_H_
#define _PRMT_TR143_H_

#include "prmt_igd.h"
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif
#ifdef _PRMT_TR143_

extern struct CWMP_LEAF tPerformanceDiagnosticLeaf[];
int getPerformanceDiagnostic(char *name, struct CWMP_LEAF *entity, int *type, void **data);

/***** Download Diagnostics **************************************************/
extern struct TR143_Diagnostics gDownloadDiagnostics;
#define StopDownloadAndResetState() \
	do{ \
		StopTR143DownloadDiag(); \
		if(gStartTR143DownloadDiag==0) gDownloadDiagnostics.DiagnosticsState=eTR143_None; \
	}while(0)

enum eDownloadDiagnosticsLeaf
{
	eDD_DiagnosticsState,
	eDD_Interface,
	eDD_DownloadURL,
	eDD_DSCP,
	eDD_EthernetPriority,
	eDD_ROMTime,
	eDD_BOMTime,
	eDD_EOMTime,
	eDD_TestBytesReceived,
	eDD_TotalBytesReceived,
	eDD_TCPOpenRequestTime,
	eDD_TCPOpenResponseTime
};
		
extern struct CWMP_PRMT tDownloadDiagnosticsLeafInfo[];
extern struct CWMP_LEAF tDownloadDiagnosticsLeaf[];
int getDownloadDiagnostics(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setDownloadDiagnostics(char *name, struct CWMP_LEAF *entity, int type, void *data);

/***** Upload Diagnostics ****************************************************/
extern struct TR143_Diagnostics gUploadDiagnostics;
#define StopUploadAndResetState() \
	do{ \
		StopTR143UploadDiag(); \
		if(gStartTR143UploadDiag==0) gUploadDiagnostics.DiagnosticsState=eTR143_None; \
	}while(0)

enum eUploadDiagnosticsLeaf
{
	eUD_DiagnosticsState,
	eUD_Interface,
	eUD_UploadURL,
	eUD_DSCP,
	eUD_EthernetPriority,
	eUD_TestFileLength,
	eUD_ROMTime,
	eUD_BOMTime,
	eUD_EOMTime,
	eUD_TotalBytesSent,
	eUD_TCPOpenRequestTime,
	eUD_TCPOpenResponseTime
};
extern struct CWMP_PRMT tUploadDiagnosticsLeafInfo[];
extern struct CWMP_LEAF tUploadDiagnosticsLeaf[];
int getUploadDiagnostics(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setUploadDiagnostics(char *name, struct CWMP_LEAF *entity, int type, void *data);

/***** UDPEcho ***************************************************************/
extern struct CWMP_PRMT tUDPEchoConfigLeafInfo[];
enum eUDPEchoConfigLeaf
{
	eUEC_Enable,
	eUEC_Interface,
	eUEC_SourceIPAddress,
	eUEC_UDPPort,
	eUEC_EchoPlusEnabled,
	eUEC_EchoPlusSupported,
	eUEC_PacketsReceived,
	eUEC_PacketsResponded,
	eUEC_BytesReceived,
	eUEC_BytesResponded,
	eUEC_TimeFirstPacketReceived,
	eUEC_TimeLastPacketReceived
};

extern struct CWMP_LEAF tUDPEchoConfigLeaf[];
int getUDPEchoConfig(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setUDPEchoConfig(char *name, struct CWMP_LEAF *entity, int type, void *data);


#ifdef CONFIG_USER_FTP_FTP_FTP
void checkPidforFTPDiag( pid_t  pid );
#endif //CONFIG_USER_FTP_FTP_FTP

extern int gStartTR143DownloadDiag;
extern int gStartTR143UploadDiag;
void StopTR143DownloadDiag(void);
void StopTR143UploadDiag(void);
void StartTR143DownloadDiag(void);
void StartTR143UploadDiag(void);

#endif /*_PRMT_TR143_*/	
#ifdef __cplusplus
}
#endif
#endif /*_PRMT_TR143_H_*/
