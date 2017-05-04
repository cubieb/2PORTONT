#ifndef _CWMP_DOWNLOAD_H_
#define _CWMP_DOWNLOAD_H_

#include "soapH.h"

#ifdef __cplusplus
extern "C" {
#endif


/*download way*/
#define DLWAY_NONE	0
#define DLWAY_DOWN	1
#define DLWAY_UP	2
/*download type*/
#define DLTYPE_NONE	0
#define DLTYPE_IMAGE	1
#define DLTYPE_WEB	2
#define DLTYPE_CONFIG	3
#define DLTYPE_LOG	4
/*download status*/
#define DOWNLD_NONE	0
#define DOWNLD_READY	1
#define DOWNLD_START	2
#define DOWNLD_FINISH	3
#define DOWNLD_ERROR	4
/*download union*/
typedef union
{
struct cwmp__Download	Download;
struct cwmp__Upload	Upload;
}DownloadInfo_T;


extern int gStartDownload;

void cwmpStartDownload(struct soap *soap);


/* Transfer Queue Support */
extern timer_t cwmp_dl_timer_id;
extern int cwmp_dl_msgid;

enum
{
	CWMP_DLCMD_TIMER = 100,
};

enum cwmp_dlq_state
{
	CWMP_DLQ_STATE_NOT_YET_START = 1,
	CWMP_DLQ_STATE_IN_PROGRESS,
	CWMP_DLQ_STATE_COMPLETED,
};

enum cwmp_time_win_mode
{
	CWMP_TIME_WIN_MODE_NONE = 0,
	CWMP_TIME_WIN_MODE_AT_ANY_TIME,
	CWMP_TIME_WIN_MODE_IMMEDIATELY,
	CWMP_TIME_WIN_MODE_WHEN_IDLE,
	CWMP_TIME_WIN_MODE_CONFIRM_NEEDED,	/* Do not support currently*/
};

struct cwmp_sdl_time_win_t
{
	unsigned int WindowStart;
	unsigned int WindowEnd;
	unsigned char WindowMode;
	char *UserMessage;
	int MaxRetries;
};

#define CWMP_MAX_TIME_WINDOW 2
struct cwmp_transfer_t
{
	int State;      /* 1:not yet start, 2: in progress, 3: completed*/
	unsigned char IsDownload; /* 0: Upload, 1: Download, 2: ScheduleDownload */
	char *CommandKey;   /* optional element of type xsd:string */
	char *FileType; /* optional element of type xsd:string */
	char *URL;  /* optional element of type xsd:string */
	char *Username; /* optional element of type xsd:string */
	char *Password; /* optional element of type xsd:string */
	unsigned int FileSize;  /* required element of type xsd:unsignedInt */
	char *TargetFileName;   /* optional element of type xsd:string */
	unsigned int DelaySeconds;  /* required element of type xsd:unsignedInt */
	char *SuccessURL;  /* optional element of type xsd:string */
	char *FailureURL;  /* optional element of type xsd:string */

	/* For ScheduleDownload RPC */
	struct cwmp_sdl_time_win_t *time_win[CWMP_MAX_TIME_WINDOW];
};

void *cwmp_download_thread(void *data);
void cwmp_clear_transfer_node(struct cwmp_transfer_t *transfer);
void cwmp_free_transfer_queue(struct node **queue);


/* TR143 HTTP Download/Upload Diagnostics*/
enum
{
	eTR143_None=0,
	eTR143_Requested,
	eTR143_Completed,
	eTR143_Error_InitConnectionFailed,
	eTR143_Error_NoResponse,
	eTR143_Error_PasswordRequestFailed,
	eTR143_Error_LoginFailed,
	eTR143_Error_NoTransferMode,
	eTR143_Error_NoPASV,
	//download
	eTR143_Error_TransferFailed,
	eTR143_Error_IncorrectSize,
	eTR143_Error_Timeout,
	//upload
	eTR143_Error_NoCWD,
	eTR143_Error_NoSTOR,
	eTR143_Error_NoTransferComplete,

	eTR143_End /*last one*/
};

struct TR143_Diagnostics
{
	int		Way;
	
	int		DiagnosticsState;
	char		*pInterface;
	char		IfName[32];
	char		*pURL;
	unsigned int	DSCP;
	unsigned int	EthernetPriority;
	unsigned int	TestFileLength;
	struct timeval	ROMTime;
	struct timeval	BOMTime;
	struct timeval	EOMTime;
	unsigned int	TestBytesReceived;
	unsigned int	TotalBytesReceived;
	unsigned int	TotalBytesSent;
	struct timeval	TCPOpenRequestTime;
	struct timeval	TCPOpenResponseTime;
	
	unsigned long int	http_pid;
	unsigned long int	ftp_pid;
};

int TR143StartHttpDiag(struct TR143_Diagnostics *p);
int TR143StopHttpDiag(struct TR143_Diagnostics *p);
/* End TR143 HTTP Download/Upload Diagnostics*/

int http_client(char *url, char *name, char *passwd, char *filename, int action, void *data);
#ifdef CONFIG_USER_FTP_FTP_FTP
int ftp_client(char *url, char *name, char *passwd, char *filename, int action, void *data);
#endif
#ifdef CONFIG_USER_TFTP_TFTP
int tftp_client(char *url, char *name, char *passwd, char *filename, int action, void *data);
#endif


#ifdef __cplusplus
}
#endif

#endif /*_CWMP_DOWNLOAD_H_*/
