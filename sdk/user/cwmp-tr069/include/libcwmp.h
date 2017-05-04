#ifndef _LIBCWMP_H_
#define _LIBCWMP_H_

#include <rtk/options.h>
#include "parameter_api.h"
#include "cwmp_utility.h"
#include "cwmp_download.h"

#define _USE_FILE_FOR_OUTPUT_
#ifdef _USE_FILE_FOR_OUTPUT_
#define OUTXMLFILENAME 		"/tmp/cwmp.xml"
#define OUTXMLFILENAME_BK	"/tmp/cwmp_bk.xml"
#endif

extern int gUseTR181;

struct cwmp_userdata
{
	//relative to SOAP header
	unsigned int		ID;
	unsigned int		HoldRequests;
	unsigned int		NoMoreRequests;
	unsigned int		CPE_MaxEnvelopes;
	unsigned int		ACS_MaxEnvelopes;
	
	//cwmp:fault
	int			FaultCode;
	
	//download/upload
	int			DownloadState;
	int			DownloadWay;
	unsigned char		SupportTransferQueue;
	int			DLQueueIdx;
	struct node *TransferQueue;
	char			*DLCommandKey;
	time_t			DLStartTime;
	time_t			DLCompleteTime;
	unsigned int		DLFaultCode;
	DownloadInfo_T	DownloadInfo;
        	
	//inform
	unsigned int		InformInterval; //PeriodicInformInterval
	time_t			InformTime; //PeriodicInformTime
	unsigned char		PeriodicInform;
	unsigned int		EventCode;
	struct node		*NotifyParameter;
	unsigned int		InformIntervalCnt; 
	
	//ScheduleInform
	unsigned int		ScheduleInformCnt; //for scheduleInform RPC Method, save the DelaySeconds
	char			*SI_CommandKey;

	//Reboot
	char			*RB_CommandKey;	//reboot's commandkey
	int			Reboot; // reboot flag

	//Need to restart WAN
	unsigned char RestartWAN;
	
	//FactoryReset
	int			FactoryReset;

	// andrew. 
	char 			*url;	// ACS URL
	char 			*username; // username used to auth us to ACS.
	char 			*password; // passwrd used to auth us to ACS.
	char 			*conreq_username;
	char 			*conreq_password;
	char 			*realm;
	int			server_port;
	char			*server_path;
	void *			machine;
	
	char			*redirect_url;
	int			redirect_count;
	
	//certificate
	char			*cert_passwd;
	char			*cert_path;
	char			*ca_cert;

	char			*notify_filename;
	
	int			url_changed;
	
	int			inform_ct_ext;

	//For SupportedDataModel.{i}
	char		*dt_doc_path;
	char 		*dt_doc_location;

	unsigned int		fw_hdr_len;	// 0 means no need to check header
	unsigned char		fw_hdr_checked;

	unsigned char		enable_cwmp;	//parameter ManagementServer.EnableCWMP
};


int cwmp_main( struct CWMP_NODE troot[] );

#ifdef _PRMT_X_CT_COM_ALARM_MONITOR_
enum { STATE_OFF, STATE_NOTIFY, STATE_ONALARM, STATE_MONITOR } alarm_timer_state;

typedef struct alarm_timer
{
	unsigned int period;	//seconds
	unsigned int instnum;
	struct alarm_timer *next;
	unsigned char state;
	int value;		// for ALARM_MODE_ADD & ALARM_MODE_AVER
	int divisor;	// for ALARM_MODE_ADD
} ALARM_TIMER;
#endif	//_PRMT_X_CT_COM_ALARM_MONITOR_


#endif /*#ifndef _LIBCWMP_H_*/
