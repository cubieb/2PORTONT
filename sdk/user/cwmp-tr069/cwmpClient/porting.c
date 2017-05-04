#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <regex.h>
#include "prmt_igd.h"
#include "prmt_ippingdiag.h"
#include "prmt_wancondevice.h"
#include "prmt_wandsldiagnostics.h"
#include "prmt_wanatmf5loopback.h"
#include "prmt_deviceinfo.h"
#ifdef _PRMT_TR143_
#include "prmt_tr143.h"
#endif //_PRMT_TR143_
#ifdef _SUPPORT_TRACEROUTE_PROFILE_
#include "prmt_traceroute.h"
#endif //_SUPPORT_TRACEROUTE_PROFILE_
#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
#include "prmt_captiveportal.h"
#endif //_SUPPORT_CAPTIVEPORTAL_PROFILE_
#ifdef SUPPORT_DHCP_RESERVED_IPADDR
#include "prmt_landevice.h"
#endif //SUPPORT_DHCP_RESERVED_IPADDR
#ifdef _PRMT_NSLOOKUP_
#include "prmt_nslookup.h"
#endif
#include <config/autoconf.h>
#ifdef CONFIG_USER_TR104
#include "cwmp_main_tr104.h"
#endif
#ifdef _PRMT_X_CT_COM_PING_
#include "prmt_ctcom_ping.h"
#endif
#ifdef CONFIG_USER_CWMP_UPNP_DM
#include "prmt_ctcom_proxy_dev_list.h"
#endif

#include <cwmp_notify.h>
#include <cwmp_rpc.h>

#define CWMP_HTTP_REALM		"realtek.com.tw"
#define CONFIG_DIR		"/var/config"
/*notification*/
#define	NOTIFY_FILENAME		CONFIG_DIR"/CWMPNotify.txt"
#ifdef CONFIG_MIDDLEWARE
#define	MW_NOTIFY_FILENAME		CONFIG_DIR"/MWNotify.txt"
#endif
/*certificates*/
#define CA_FNAME		CONFIG_DIR"/cacert.pem"
#define CERT_FNAME		CONFIG_DIR"/client.pem"
#define DEF_CA_FN		"/etc/cacert.pem"
#define DEF_CERT_FN		"/etc/client.pem"

#include <rtk/options.h>

int do_upgrade=0;

#ifdef CONFIG_MIDDLEWARE
#include <rtk/midwaredefs.h>

int sendOpertionDoneMsg2MidIntf(char opertion)
{
	int spid;
	FILE * spidfile;
	int msgid;
	int ret;
 	struct mwMsg sendMsg;
	char * sendBuf = sendMsg.msg_data;

	*sendBuf = OP_OperationDone;	/*Opcode*/
	*(sendBuf+1) = 1;				/*N*/
	*(sendBuf+2) = TYPE_Operation;	/*type*/
	W_WORD2CHAR((sendBuf+3), 1);	/*length*/
	*(sendBuf+5) = opertion;		/*value:'1'- PING;'2'-ATMF5Loopback;'3'-DSL*/

	msgid = msgget((key_t)1357,  0666);
	if(msgid <= 0){
		//fprintf(stdout,"get cwmp msgqueue error!\n");
		return -1;
	}

	/* get midware interface pid*/
	if ((spidfile = fopen(MW_INTF_RUNFILE, "r"))) {
		fscanf(spidfile, "%d\n", &spid);
		fclose(spidfile);
	}else{
		//fprintf(stdout,"midware interface pidfile not exists\n");
		return -1;
	}

	sendMsg.msg_type = spid;
	sendMsg.msg_datatype = PACKET_OK;
	if(msgsnd(msgid, (void *)&sendMsg, MW_MSG_SIZE, 0) < 0){
		fprintf(stdout,"send message to midwareintf error!\n");
		return -1;
	}

 	return 0;
}
#endif

/*********************************************************************/
/* utility */
/*********************************************************************/
/* copy from boa/src/LINUX/parser.c */
int upgradeFirmware( char *fwFilename )
{
	FILE *fp;
	long filelen;
	struct stat st;

	CWMPDBG( 1, ( stderr, "<%s:%d>Ready to upgrade the new firmware\n", __FUNCTION__, __LINE__ ) );
	if (!(fp = fopen(fwFilename, "rb")))
	{
		CWMPDBG( 0, ( stderr, "<%s:%d>Image file open fail(%s)\n", __FUNCTION__, __LINE__, fwFilename ) );
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	filelen = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	if (filelen <= 0)
	{
		CWMPDBG( 0, ( stderr, "<%s:%d>Image file not exist\n", __FUNCTION__, __LINE__ ) );
		fclose(fp);
		return -1;
	}
	fclose(fp);

#ifndef CONFIG_LUNA_FIRMWARE_UPGRADE_SUPPORT
	//check the image header
	if (!cmd_check_image(fwFilename,0)) //return 0:error, others:success
	{
		CWMPDBG( 0, ( stderr, "<%s:%d>Image Checksum Failed!\n", __FUNCTION__, __LINE__ ) );
	   	return -1;
	}
#endif

	if ((fp = fopen(fwFilename, "rb")) == NULL) {
		printf("File %s open fail\n", fwFilename);
		return;
	}

	if (fstat(fileno(fp), &st) < 0) {
		printf("File %s get status fail\n", fwFilename);
		fclose(fp);
		return;
	}

	if (st.st_size <= 0) {
		printf("File %s size error\n", fwFilename);
		fclose(fp);
		return -1;
	}
	fclose(fp);

#ifdef CONFIG_LUNA_FIRMWARE_UPGRADE_SUPPORT
	{
		// Set fail first, the real upgrade function will clear this value if success.
		unsigned int fault = 9010;
		mib_set( CWMP_DL_FAULTCODE, &fault );
	}
#endif

	if( cmd_upload( fwFilename,0, st.st_size )==0 ) return -1;

	return 0;
}


/*********************************************************************/
/* porting functions */
/*********************************************************************/
#ifdef CONFIG_RTL8672_SAR
void port_setuppid(void)
{
	int tr069_pid;
	unsigned char value[32];

	tr069_pid = (int)getpid();
	sprintf(value,"%d",tr069_pid);
	if (va_cmd("/bin/sarctl",2,1,"tr069_pid",value))
		printf("sarctl tr069_pid %s failed\n", value);
}
#endif

#ifdef CONFIG_CWMP_TRANSFER_QUEUE
static int load_transfer_queue(struct node **root)
{
	MIB_CE_CWMP_TRANSFER_T entry;
	struct cwmp_transfer_t *transfer;
	int total, i, j;

	*root = NULL;
	total = mib_chain_total(CWMP_TRANSFER_QUEUE_TBL);

	for(i = 0 ; i < total ; i++)
	{
		if(!mib_chain_get(CWMP_TRANSFER_QUEUE_TBL, i, &entry))
			continue;

		transfer = malloc(sizeof(struct cwmp_transfer_t));
		memset(transfer, 0, sizeof(struct cwmp_transfer_t));
		transfer->State = entry.State;
		transfer->IsDownload = entry.IsDownload;
		transfer->CommandKey = strdup(entry.CommandKey);
		transfer->FileType = strdup(entry.FileType);
		transfer->URL = strdup(entry.URL);
		transfer->Username = strdup(entry.Username);
		transfer->Password = strdup(entry.Password);
		transfer->FileSize = entry.FileSize;
		transfer->TargetFileName = strdup(entry.TargetFileName);
		transfer->DelaySeconds = entry.DelaySeconds;
		transfer->SuccessURL = strdup(entry.SuccessURL);
		transfer->FailureURL = strdup(entry.FailureURL);

		for(j = 0 ; j < 2 ; j++)
		{
			struct cwmp_sdl_time_win_t *time_win = NULL;
			// empty time window
			if(entry.time_win_start[j] == 0 && entry.time_win_start[j] == 0)
				break;

			time_win = malloc(sizeof(struct cwmp_sdl_time_win_t));
			time_win->WindowStart = entry.time_win_start[j];
			time_win->WindowEnd = entry.time_win_end[j];
			time_win->WindowMode = entry.time_win_mode[j];
			time_win->UserMessage = strdup(entry.time_win_user_msg[j]);
			time_win->MaxRetries = entry.time_win_retires[j];

			transfer->time_win[j] = time_win;
		}

		push_node_data(root, transfer);
	}

	return 0;
}

void port_update_transfer_queue(struct node *root)
{
	MIB_CE_CWMP_TRANSFER_T entry;
	struct cwmp_transfer_t *transfer;
	int i, total, j;

	mib_chain_clear(CWMP_TRANSFER_QUEUE_TBL);

	total = get_node_count(root);
	for(i = 0 ; i < total ; i++)
	{
		transfer = get_node_data(root, i);
		if(transfer == NULL)
			continue;

		memset(&entry, 0, sizeof(MIB_CE_CWMP_TRANSFER_T));

		entry.State = transfer->State;
		entry.IsDownload = transfer->IsDownload;

		if(transfer->CommandKey)
			strncpy(entry.CommandKey,transfer->CommandKey, sizeof(entry.CommandKey));
		if(transfer->FileType)
			strncpy(entry.FileType,transfer->FileType, sizeof(entry.FileType));
		if(transfer->URL)
			strncpy(entry.URL,transfer->URL, sizeof(entry.URL));
		if(transfer->Username)
			strncpy(entry.Username,transfer->Username, sizeof(entry.Username));
		if(transfer->Password)
			strncpy(entry.Password,transfer->Password, sizeof(entry.Password));
		entry.FileSize = transfer->FileSize;
		if(transfer->TargetFileName)
			strncpy(entry.TargetFileName,transfer->TargetFileName, sizeof(entry.TargetFileName));
		if(transfer->DelaySeconds)
			entry.DelaySeconds = transfer->DelaySeconds;
		else
			entry.DelaySeconds = 0;
		if(transfer->SuccessURL)
			strncpy(entry.SuccessURL,transfer->SuccessURL, sizeof(entry.SuccessURL));
		if(transfer->FailureURL)
			strncpy(entry.FailureURL,transfer->FailureURL, sizeof(entry.FailureURL));

		for(j = 0 ; j < 2 ; j++)
		{
			struct cwmp_sdl_time_win_t *time_win = transfer->time_win[j];

			if(time_win == NULL)
				break;

			entry.time_win_start[j] = time_win->WindowStart;
			entry.time_win_end[j] = time_win->WindowEnd;
			entry.time_win_mode[j] = time_win->WindowMode;
			if(time_win->UserMessage)
				strncpy(entry.time_win_user_msg[j], time_win->UserMessage, 256);
			entry.time_win_retires[j] = time_win->MaxRetries;
		}

		mib_chain_add(CWMP_TRANSFER_QUEUE_TBL, (void *)&entry);
	}
}
#else
void port_update_transfer_queue(struct node *root)
{
	return;
}
#endif

int port_init_userdata(struct cwmp_userdata *data)
{
	char buf[256 + 1];
	unsigned char ch, changed;
	unsigned int uVal;
	int Val;

	if (data) {
		memset(data, 0, sizeof(struct cwmp_userdata));

		//relative to SOAP header
		data->ID = 1;
		data->HoldRequests = 0;
		data->NoMoreRequests = 0;
		data->CPE_MaxEnvelopes = 1;
		data->ACS_MaxEnvelopes = 1;

		//cwmp:fault
		data->FaultCode = 0;

		//download/upload
		data->DownloadState = DOWNLD_NONE;
		data->DownloadWay = DLWAY_NONE;
		data->TransferQueue = NULL;
#ifdef CONFIG_CWMP_TRANSFER_QUEUE
		data->SupportTransferQueue = 1;
		data->DLQueueIdx = mib_get(CWMP_DL_QUEUE_IDX, &Val) ? Val : 0;
		load_transfer_queue(&data->TransferQueue);
#else
		data->SupportTransferQueue = 0;
		data->DLQueueIdx = -1;
#endif
		data->DLCommandKey = mib_get(CWMP_DL_COMMANDKEY, buf) ? strdup(buf) : NULL;
		data->DLStartTime = mib_get(CWMP_DL_STARTTIME, &uVal) ? uVal : 0;
		data->DLCompleteTime = mib_get(CWMP_DL_COMPLETETIME, &uVal) ? uVal : 0;
		data->DLFaultCode = mib_get(CWMP_DL_FAULTCODE, &uVal) ? uVal : 0;

		data->fw_hdr_checked = 0;
		data->fw_hdr_len = 300;

		//inform
		data->InformInterval = mib_get(CWMP_INFORM_INTERVAL, &uVal) ? uVal : 60;
		data->InformTime = mib_get(CWMP_INFORM_TIME, &uVal) ? uVal : 0;
		data->PeriodicInform = mib_get(CWMP_INFORM_ENABLE, &ch) ? ch : 1;
		data->EventCode = mib_get(CWMP_INFORM_EVENTCODE, &uVal) ? uVal : 0;
		if (mib_get(CWMP_FLAG2, (void *)&ch))
		{
			if ((ch & CWMP_FLAG2_HAD_SENT_BOOTSTRAP) == 0)
				data->EventCode |= EC_BOOTSTRAP;

#ifdef CONFIG_E8B
			if ((ch & CWMP_FLAG2_HAD_SENT_LONGRESET) == 0)
			{
				data->EventCode |= EC_X_CT_COM_LONGRESET;
				ch |= CWMP_FLAG2_HAD_SENT_LONGRESET;
				mib_set(CWMP_FLAG2, &ch);
				mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
			}
#endif
		}
		data->NotifyParameter = NULL;
		data->InformIntervalCnt = 0;

		//ScheduleInform
		data->ScheduleInformCnt = 0;
		data->SI_CommandKey = mib_get(CWMP_SI_COMMANDKEY, buf) ? strdup(buf) : NULL;

		//Reboot
		data->RB_CommandKey = mib_get(CWMP_RB_COMMANDKEY, buf) ? strdup(buf) : NULL;
		data->Reboot = 0;

		//RestartWAN
		data->RestartWAN = 0;

		//FactoryReset
		data->FactoryReset = 0;

#if defined(CONFIG_TR142_MODULE) && !defined(CONFIG_E8B)
		mib_get(RS_OMCI_ACS_CONFIGURED, &ch);
		if(ch)
		{
			data->url = mib_get(RS_OMCI_ACS_URL, buf) ? strdup(buf) : strdup("");
			data->username = mib_get(RS_OMCI_ACS_USERNAME, buf) ? strdup(buf) : NULL;
			data->password = mib_get(RS_OMCI_ACS_PASSWD, buf) ? strdup(buf) : NULL;
		}
		else
		{
			data->url = mib_get(CWMP_ACS_URL, buf) ? strdup(buf) : strdup("");
			data->username = mib_get(CWMP_ACS_USERNAME, buf) ? strdup(buf) : NULL;
			data->password = mib_get(CWMP_ACS_PASSWORD, buf) ? strdup(buf) : NULL;
		}
#else
		// andrew
		data->url = mib_get(CWMP_ACS_URL, buf) ? strdup(buf) : strdup("");
		data->username = mib_get(CWMP_ACS_USERNAME, buf) ? strdup(buf) : NULL;
		data->password = mib_get(CWMP_ACS_PASSWORD, buf) ? strdup(buf) : NULL;
#endif
		//use the wan ip address as realm??
		data->conreq_username = mib_get(CWMP_CONREQ_USERNAME, buf) ? strdup(buf) : NULL;
		data->conreq_password = mib_get(CWMP_CONREQ_PASSWORD, buf) ? strdup(buf) : NULL;
		data->realm = CWMP_HTTP_REALM;
		data->server_port = mib_get(CWMP_CONREQ_PORT, &uVal) ? uVal : 7547;
		if (mib_get(CWMP_CONREQ_PATH, &buf[1])) {
			if (buf[1] != '/') {
				buf[0] = '/';
				data->server_path = strdup(buf);
			} else
				data->server_path = strdup(&buf[1]);
		} else {
			data->server_path = strdup("/");
		}
		/*data->machine = &cpe_client;*/
		data->redirect_url = NULL;
		data->redirect_count = 0;

		//certificate
		data->cert_passwd = mib_get(CWMP_CERT_PASSWORD, buf) ? strdup(buf) : NULL;
		{
			struct stat file_stat;

			if (stat(CERT_FNAME, &file_stat) < 0)
				data->cert_path = strdup(DEF_CERT_FN);
			else
				data->cert_path = strdup(CERT_FNAME);

			if (stat(CA_FNAME, &file_stat) < 0)
				data->ca_cert = strdup(DEF_CA_FN);
			else
				data->ca_cert = strdup(CA_FNAME);
		}

#ifdef CONFIG_MIDDLEWARE
		mib_get(CWMP_TR069_ENABLE, &ch);
		if (ch == 0) {	/*midware enabled*/
			data->notify_filename = strdup(MW_NOTIFY_FILENAME);
		} else
#endif
		data->notify_filename = strdup(NOTIFY_FILENAME);
		data->url_changed = 0;

		/*china-telecom has a extension for inform message, X_OUI_AssocDevice*/
		/*0: diable this field, 1:enable this filed*/
		data->inform_ct_ext = 0;
#ifdef _INFORM_EXT_FOR_X_CT_
		{
			if (mib_get(CWMP_FLAG, &ch)) {
				if (ch & CWMP_FLAG_CTINFORMEXT)
					data->inform_ct_ext = 1;
			}
		}
#endif

#ifdef CONFIG_CWMP_TR181_SUPPORT
		//DT instance document path
		//For SupportedDataModel.{i}
		data->dt_doc_path = strdup(DT_DOC_PATH);
		data->dt_doc_location = strdup(DT_DOC_LOCATION);
#else
		data->dt_doc_path = NULL;
		data->dt_doc_location = NULL;
#endif

		// parameter ManagementServer.EnableCWMP
		data->enable_cwmp = 1;
		if (mib_get(CWMP_FLAG2, (void *)&ch))
		{
			if(ch & CWMP_FLAG2_CWMP_DISABLE)
				data->enable_cwmp = 0;
		}
	}

#ifdef _PRMT_X_CT_COM_USERINFO_
	init_resetuserinfo_timer();
#endif
	return 0;
}


int port_update_userdata(struct cwmp_userdata *data, int is_discon, int mib_id)
{
	char buf[256 + 1];
	unsigned char ch, *pnew, *pold;
	unsigned int vUint=0;
	unsigned char vchar;

	switch(mib_id)
	{
		case CWMP_CONREQ_USERNAME:
			CWMPDBP(1, "CWMP_CONREQ_USERNAME!\n");
			if(mib_get(CWMP_CONREQ_USERNAME, buf) != 0)
			{
				if( (data->conreq_username==NULL) || (strcmp(data->conreq_username,buf)!=0) )
				{
					pnew = strdup(buf);
					pold = data->conreq_username;
					data->conreq_username = pnew;
					if(pold) free(pold);
				}
			}
			break;
		case CWMP_CONREQ_PASSWORD:
			CWMPDBP(1, "CWMP_CONREQ_PASSWORD!\n");
			if( mib_get(CWMP_CONREQ_PASSWORD, buf)!=0 )
			{
				if( (data->conreq_password==NULL) || (strcmp(data->conreq_password,buf)!=0) )
				{
					pnew = strdup(buf);
					pold = data->conreq_password;
					data->conreq_password = pnew;
					if(pold) free(pold);
				}
			}
			break;
#ifdef CONFIG_E8B
		case CWMP_INFORM_EVENTCODE:
			CWMPDBP(1, "CWMP_INFORM_EVENTCODE!\n");
			vUint=0;
			if( mib_get(CWMP_INFORM_EVENTCODE, &vUint)!=0 )
			{
				if( vUint & EC_PERIODIC)
				{
					fprintf( stderr, "get EC_PERIODIC\n" );
					cwmpSendEvent( EC_PERIODIC );
					vUint &= ~EC_PERIODIC;
					mib_set_cwmp(CWMP_INFORM_EVENTCODE, &vUint);
				}

				if( vUint & EC_X_CT_COM_ACCOUNT )
				{
					fprintf( stderr, "get EC_X_CT_COM_ACCOUNT\n" );
					cwmpSendEvent( EC_X_CT_COM_ACCOUNT );
					vUint &= ~EC_X_CT_COM_ACCOUNT;
					mib_set_cwmp(CWMP_INFORM_EVENTCODE, &vUint);
				}

#ifdef  _PRMT_X_CT_COM_USERINFO_
				if( vUint & EC_X_CT_COM_BIND )
				{
					fprintf( stderr, "get EC_X_CT_COM_BIND\n" );
					cwmpSendEvent( EC_X_CT_COM_BIND );
					vUint &= ~EC_X_CT_COM_BIND;
					mib_set_cwmp(CWMP_INFORM_EVENTCODE, &vUint);
				}
#endif
			}

			break;
#endif	//CONFIG_E8B
		case CWMP_ACS_URL:
			//if(is_discon && (mib_get(CWMP_ACS_URL, buf) != 0))
			if(mib_get(CWMP_ACS_URL, buf) != 0)
			{
				if( (data->url==NULL) || (strcmp(data->url,buf)!=0) )
				{
					pnew = strdup(buf);
					pold = data->url;
					data->url = pnew;
					if(pold) free(pold);
					//reset something??
					data->url_changed = 1;
				}
			}
			break;
		case CWMP_ACS_USERNAME:
			//if(is_discon && (mib_get(CWMP_ACS_USERNAME, buf) != 0))
			if(mib_get(CWMP_ACS_USERNAME, buf) != 0)
			{
				if( (data->username==NULL) || (strcmp(data->username,buf)!=0) )
				{
					pnew = strdup(buf);
					pold = data->username;
					data->username = pnew;
					if(pold) free(pold);
				}
			}
			break;
		case CWMP_ACS_PASSWORD:
			//if(is_discon && (mib_get(CWMP_ACS_PASSWORD, buf) != 0))
			if(mib_get(CWMP_ACS_PASSWORD, buf) != 0)
			{
				if((data->password==NULL) || (strcmp(data->password,buf)!=0))
				{
					pnew = strdup(buf);
					pold = data->password;
					data->password = pnew;
					if(pold) free(pold);
				}
			}
			break;
#if defined(CONFIG_TR142_MODULE) && !defined(CONFIG_E8B)
		case RS_OMCI_ACS_CONFIGURED:
			mib_get(RS_OMCI_ACS_CONFIGURED, &vchar);

			if(vchar)
			{
				mib_get(RS_OMCI_ACS_URL, buf);
				if(strcmp(buf, data->url) != 0)
				{
					if(data->url)
						free(data->url);
					data->url = strdup(buf);
					data->url_changed = 1;
				}

				mib_get(RS_OMCI_ACS_USERNAME, buf);
				if(data->username)
					free(data->username);
				data->username = strdup(buf);

				mib_get(RS_OMCI_ACS_PASSWD, buf);
				if(data->password)
					free(data->password);
				data->password = strdup(buf);
			}
			else
			{
				mib_get(CWMP_ACS_URL, buf);
				if(strcmp(buf, data->url) != 0)
				{
					if(data->url)
						free(data->url);
					data->url = strdup(buf);
					data->url_changed = 1;
				}

				mib_get(CWMP_ACS_USERNAME, buf);
				if(data->username)
					free(data->username);
				data->username = strdup(buf);

				mib_get(CWMP_ACS_PASSWORD, buf);
				if(data->password)
					free(data->password);
				data->password = strdup(buf);
			}
#endif
		case CWMP_INFORM_ENABLE:
		case CWMP_INFORM_INTERVAL:
		case CWMP_INFORM_TIME:
			CWMPDBP(0, "CWMP_INFORM_ENABLE/CWMP_INFORM_INTERVAL/CWMP_INFORM_TIME!\n");
			if( mib_id ==	CWMP_INFORM_ENABLE )
			{
				if(mib_get(CWMP_INFORM_ENABLE, &vchar) != 0 )
				{
					if( data->PeriodicInform != vchar )
					{
						data->PeriodicInform = vchar;
						cwmpMgmtSrvInformInterval();
					}
				}
			}
			else if( mib_id ==	CWMP_INFORM_INTERVAL )
			{
				if(mib_get(CWMP_INFORM_INTERVAL, &vUint) != 0 )
				{
					if( data->InformInterval != vUint )
					{
						data->InformInterval = vUint;
						cwmpMgmtSrvInformInterval();
					}
				}
			}
			else if( mib_id ==	CWMP_INFORM_TIME )
			{
				if(mib_get(CWMP_INFORM_TIME, &vUint) != 0 )
				{
					if( data->InformTime != vUint )
					{
						data->InformTime = vUint;
						cwmpMgmtSrvInformInterval();
					}
				}
			}
			break;
		case CWMP_FLAG2:
			if(mib_get(CWMP_FLAG2, &vchar))
			{
				if(vchar & CWMP_FLAG2_CWMP_DISABLE)
					data->enable_cwmp = 0;
				else
					data->enable_cwmp = 1;
			}
			break;
#ifdef CONFIG_E8B
		case CWMP_USERINFO_STATUS:
			{
				int pid = -1;

				pid = read_pid("/var/run/systemd.pid");
				if(pid > 0)
					kill(pid, SIGUSR1);
			}
#endif
#ifdef CTC_DNS_SPEED_LIMIT
		case DNS_LIMIT_DEV_INFO_TBL:
			{
				unsigned char action;

				mib_get(MIB_DNS_LIMIT_ACTION, &action);

				if(action == DNS_LIMIT_ACTION_ALERT)
					cwmpSendEvent( EC_X_CT_COM_DNSLIMITALERT );
			}
#endif
			break;
	}

	return 0;
}

void port_save_reboot( struct cwmp_userdata *user, int reboot_flag )
{
	if( user )
	{
		//reboot commandkey
		if(user->RB_CommandKey)
			mib_set( CWMP_RB_COMMANDKEY, user->RB_CommandKey );
		else
			mib_set( CWMP_RB_COMMANDKEY, "" );

		//scheduleinform commandkey
		if(user->SI_CommandKey)
			mib_set( CWMP_SI_COMMANDKEY, user->SI_CommandKey );
		else
			mib_set( CWMP_SI_COMMANDKEY, "" );

		//related to download
		if(user->DLCommandKey)
			mib_set( CWMP_DL_COMMANDKEY, user->DLCommandKey );
		else
			mib_set( CWMP_DL_COMMANDKEY, "" );
#ifdef CONFIG_CWMP_TRANSFER_QUEUE
		mib_set( CWMP_DL_QUEUE_IDX, &user->DLQueueIdx);
#endif
		mib_set( CWMP_DL_STARTTIME, &user->DLStartTime );
		mib_set( CWMP_DL_COMPLETETIME, &user->DLCompleteTime );
		mib_set( CWMP_DL_FAULTCODE, &user->DLFaultCode );

		//inform
		mib_set_cwmp( CWMP_INFORM_EVENTCODE, &user->EventCode );
	}

#ifdef CONFIG_USER_TR104
	update_VoIP_FLASH();
#endif

#ifdef CONFIG_E8B
	if(reboot_flag)
	{
		// sleep 5 seconds before reboot
		// to let GUI have chance to get register result status
		int delay = 5;
		unsigned char needReboot;

		mib_get(CWMP_USERINFO_NEED_REBOOT, &needReboot);

		if(needReboot)
		{
			while(delay > 0)
				delay = sleep(delay);

			needReboot = 0;
			mib_set(CWMP_USERINFO_NEED_REBOOT, &needReboot);
		}
	}
#endif

	//fprintf( stderr, "<%s:%d>\n", __FUNCTION__, __LINE__ );fflush(stderr);
	mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
	//fprintf( stderr, "<%s:%d>\n", __FUNCTION__, __LINE__ );fflush(stderr);
#ifdef CONFIG_USER_XMLCONFIG
	//wait for mib_update() finish, may got config file corrupted without this line
	sleep(1);
#endif

	if(reboot_flag)
	{
		CWMPDBG( 0, ( stderr, "<%s:%d>The system is restarting ...\n", __FUNCTION__, __LINE__ ) );
		cmd_reboot();
		exit(0);
	}
}

void port_factoryreset_reboot(void)
{
	CWMPDBG( 3, ( stderr, "<%s:%d>\n", __FUNCTION__, __LINE__ ) );
	// Commented by Mason Yu. for not use default setting
	//mib_load(DEFAULT_SETTING, CONFIG_MIB_ALL);
	//va_cmd("/bin/flash", 2, 1, "default", "ds");
#ifdef CONFIG_E8B
	reset_cs_to_default(2); // 2 for CWMP reset
#else
	reset_cs_to_default(1);
#endif


	CWMPDBG( 0, ( stderr, "<%s:%d>The system is restarting ...\n", __FUNCTION__, __LINE__ ) );
#ifdef CONFIG_E8B
	sleep(5); //wait finishing setdefault configuration for 16M flash
#endif
	cmd_reboot();
	exit(0);
}

int port_before_download( int file_type, char *target )
{
	fprintf( stderr, "<%s:%d> file type:%d, target:%s\n", __FUNCTION__, __LINE__, file_type, target?target:"" );

	if(target==NULL) return -1;

	switch( file_type )
	{
	case DLTYPE_IMAGE:
#ifdef CONFIG_LUNA_FIRMWARE_UPGRADE_SUPPORT
		strcpy( target, "/tmp/img.tar" );
#else
		strcpy( target, "/tmp/vm.img" );
#endif
		do_upgrade = 1;
		break;
	case DLTYPE_WEB: //not support right now
		do_upgrade = 0;
		break;
	case DLTYPE_CONFIG:
#if defined(CONFIG_USE_XML) || defined(CONFIG_USER_XMLCONFIG)
		strcpy( target, "/tmp/config.xml" );
#else
		strcpy( target, "/tmp/adsl-config.bin" );
#endif
		do_upgrade = 0;
		break;
	}

	fprintf( stderr, "<%s:%d> file type:%d, target:%s\n", __FUNCTION__, __LINE__, file_type, target?target:"" );
	return 0;
}

int port_after_download( int file_type, char *target )
{
	do_upgrade = 0;

	fprintf( stderr, "<%s:%d> file type:%d, target:%s\n", __FUNCTION__, __LINE__, file_type, target?target:"" );

	switch(file_type)
	{
	case DLTYPE_IMAGE:
		//update firmware
		if( upgradeFirmware( target ) ) //return 0: success
		{
			CWMPDBG( 0, ( stderr, "<%s:%d>Image Checksum Failed!\n", __FUNCTION__, __LINE__ ) );
#ifdef _PRMT_X_CT_COM_ALARM_MONITOR_
			set_ctcom_alarm(CTCOM_ALARM_SW_UPGRADE_FAIL);
			syslog(LOG_ERR, "System software upgrade failed.");
#endif
			return ERR_9010;
		}
#ifdef _PRMT_X_CT_COM_ALARM_MONITOR_
		clear_ctcom_alarm(CTCOM_ALARM_SW_UPGRADE_FAIL);
#endif
		break;
	case DLTYPE_CONFIG:
#ifdef CONFIG_E8B
		{
			struct file_pipe pipe;
			unsigned char cpbuf[256];

			pipe.buffer = cpbuf;
			pipe.bufsize = sizeof(cpbuf);
			pipe.func = decode;

			file_copy_pipe(CONFIG_XMLFILE, CONFIG_XMLENC, &pipe);
			if(rename(CONFIG_XMLENC, CONFIG_XMLFILE) == -1)
			{
				fprintf(stderr, "Rename %s to %s failed: %s\n", CONFIG_XMLENC, CONFIG_XMLFILE, strerror(errno));
			}
		}
#endif
#ifdef CONFIG_USER_XMLCONFIG
		if (call_cmd("/bin/xmlconfig", 3, 1, "-if", CONFIG_XMLFILE, "-nodef"))
		{
			fprintf(stderr, "Load config error\n");
#ifdef _PRMT_X_CT_COM_ALARM_MONITOR_
			/* This event will be cleared if loadconfig success. */
			set_ctcom_alarm(CTCOM_ALARM_CONF_INVALID);
			syslog(LOG_ERR, "Invalid configuration file.");
#endif
			return ERR_9002;
		}
#elif defined(CONFIG_USE_XML)
		//if( va_cmd("/bin/LoadxmlConfig",0,1) )
		if( va_cmd("/bin/loadconfig",0,1) )
		{
			fprintf( stderr, "exec /bin/loadconfig error!\n" );
			return ERR_9002;
		}
#else
		{
			unsigned int startPos,endPos,nLen,nRead;
			FILE	 *fp=NULL;
			unsigned char *buf;

			fp= fopen(target, "r");
			if(fp == NULL)
			{
				fprintf(stderr, "ERROR: Open %s failed\n", target);
				return ERR_9002;
			}

			fseek(fp, 0, SEEK_END);
			nLen = ftell(fp);

			fprintf(stderr, "filesize is %d\n", nLen);
			if(nLen == 0 )
			{
				fprintf(stderr, "ERROR: malloc error\n");
				fclose(fp);
				return ERR_9002;
			}

			fseek(fp, 0, SEEK_SET);
			buf = malloc(nLen);
			if (!buf)
			{
				fprintf(stderr, "ERROR: malloc error\n");
				fclose(fp);
				return ERR_9002;
			}

			nRead = fread((void *)buf, 1, nLen, fp);
			if (nRead != nLen)
			{
				fprintf(stderr, "ERROR: Read %d bytes, expect %d bytes\n", nRead, nLen);
				fclose(fp);
				free(buf);
				return ERR_9002;
			}

			fprintf(stderr, "write to %d bytes from %08x\n", nLen, buf);
			DECODE_DATA(buf + sizeof(PARAM_HEADER_T), nLen - sizeof(PARAM_HEADER_T));
			if(mib_update_from_raw(buf, nLen) != 1)
			{
				fprintf(stderr, "ERROR: Write config error\n");
				fclose(fp);
				free(buf);
				return ERR_9002;
			}

			if(mib_load(CURRENT_SETTING, CONFIG_MIB_ALL) != 1)
			{
				fprintf(stderr, "ERROR: Replace memory error\n");
				free(buf);
				fclose(fp);
				return ERR_9002;
			}
			free(buf);
			fclose(fp);
		}
#endif	//CONFIG_USE_XML
#ifdef CONFIG_E8B
		FILE *fpin;
		int filelen;
		fpin=fopen(target,"r");
		if(fpin>0){
			fseek(fpin, 0, SEEK_END);
			filelen = ftell(fpin);
			fseek(fpin, 0, SEEK_SET);
			fclose(fpin);
			if(filelen<=0)
				return ERR_9002;
		}else
			return ERR_9002;
#endif
		break;
	}
	return 0;
}


int port_before_upload( int file_type, char *target )
{
	if( target==NULL ) return -1;

	switch(file_type)
	{
	case DLTYPE_CONFIG:
#ifdef CONFIG_USER_XMLCONFIG
		strcpy( target, CONFIG_XMLFILE );
		if (va_cmd("/bin/sh",3,1, "/etc/scripts/config_xmlconfig.sh", "-s", target) != 0)
		{
			fprintf( stderr, "Save config error\n" );
		}
#ifdef CONFIG_E8B
		{
			struct file_pipe pipe;
			unsigned char cpbuf[256];

			pipe.buffer = cpbuf;
			pipe.bufsize = sizeof(cpbuf);
			pipe.func = encode;

			file_copy_pipe(CONFIG_XMLFILE, CONFIG_XMLENC, &pipe);
			if(rename(CONFIG_XMLENC, CONFIG_XMLFILE) == -1)
			{
				fprintf(stderr, "Rename %s to %s failed: %s\n", CONFIG_XMLENC, CONFIG_XMLFILE, strerror(errno));
			}
		}
#endif
#elif defined(CONFIG_USE_XML)
		strcpy( target, "/tmp/config.xml" );
		//if( va_cmd("/bin/CreatexmlConfig",0,1) )
		if( va_cmd("/bin/saveconfig",0,1) )
		{
			fprintf( stderr, "exec /bin/saveconfig error!\n" );
		}
#else
		{
			PARAM_HEADER_T header;
			unsigned char *buf;
			int fileSize = 0;
			FILE *fp = NULL;

			if(mib_read_header(CURRENT_SETTING, &header) != 1)
			{
				fprintf(stderr, "ERROR: Flash read fail.");
				return -1;
			}

			fileSize = sizeof(PARAM_HEADER_T) + header.len;
			buf = malloc(fileSize);
			if ( buf == NULL ) {
				fprintf(stderr, "Allocate buffer failed!");
				return -1;
			}

			fprintf(stderr, "fileSize=%d\n",fileSize);
			if(mib_read_to_raw(CURRENT_SETTING, buf, fileSize) != 1)
			{
				free(buf);
				fprintf(stderr, "ERROR: Flash read fail.");
				return -1;
			}
			ENCODE_DATA(buf + sizeof(PARAM_HEADER_T), header.len);

			strcpy(target, "/tmp/adsl-config.bin");
			fp = fopen(target, "w");
			if(!fp)
			{
				free(buf);
				fprintf(stderr, "ERROR: Open %s failed.", target);
				return -1;
			}

			if(fwrite(buf, 1, fileSize, fp) != fileSize)
			{
				free(buf);
				fclose(fp);
				fprintf(stderr, "ERROR: Write config file to %s failed.", target);
				return -1;
			}
			fclose(fp);
			free(buf);
		}
#endif //CONFIG_USE_XML
		break;
	case DLTYPE_LOG:
		//ifndef CONFIG_USER_BUSYBOX_SYSLOGD, send empty http put(content-length=0)
#ifdef CONFIG_E8B
		strcpy( target, "/var/config/syslogd.txt" );
#else
		strcpy( target, "/var/log/messages" );
#endif
		break;
	}

	fprintf( stderr, "<%s:%d> file type:%d, target:%s\n", __FUNCTION__, __LINE__, file_type, target?target:"" );

	return 0;
}


int port_after_upload( int file_type, char *target )
{
	if( target==NULL ) return -1;

	//remove the target file
	switch(file_type)
	{
	case DLTYPE_CONFIG:
		if( strlen(target) )	remove( target );
		break;
	case DLTYPE_LOG:
		//not have to remove the log file
		break;
	}

	return 0;
}

#ifdef CONFIG_LUNA_FIRMWARE_UPGRADE_SUPPORT
int port_check_fw_header(char *buf, struct cwmp_userdata *data)
{
	fprintf( stderr, "<%s:%d>\n", __FUNCTION__, __LINE__);

	// no need to do fw upgrade
	data->fw_hdr_checked = 1;

	if(!do_upgrade)
		return 0;

	// check tar magic number "ustar" at offset 257
	if(memcmp(&buf[257], (void *)"ustar", 5) == 0)
	{
		fprintf( stderr, "<%s:%d> firmware header check ok!\n", __FUNCTION__, __LINE__);
		cmd_killproc(ALL_PID & ~(1<<PID_CWMP));
		return 0;
	}

	fprintf( stderr, "<%s:%d> firmware header check error!\n", __FUNCTION__, __LINE__);
	return -1;
}
#else
int port_check_fw_header(char *buf, struct cwmp_userdata *data)
{
#ifdef ENABLE_SIGNATURE_ADV
	IMGHDR imgHdr;
	SIGHDR sigHdr;
	unsigned int hdrChksum;
	unsigned int i=0;
#endif
#ifdef CONFIG_RTL8686
	int err=-1;
#endif

	if(!do_upgrade)
	{
		// no need to do fw upgrade
		data->fw_hdr_checked = 1;
		return 0;
	}

#ifdef ENABLE_SIGNATURE_ADV
#ifdef ENABLE_SIGNATURE
	memcpy(&sigHdr, buf, sizeof(sigHdr));
	memcpy(&imgHdr, buf+sizeof(sigHdr), sizeof(imgHdr));

	if (sigHdr.sigLen > SIG_LEN) {
		printf("signature Header too long!\n");
	}
	for (i=0; i<sigHdr.sigLen; i++)
		sigHdr.sigStr[i] = sigHdr.sigStr[i] - 10;
	if (strcmp(sigHdr.sigStr, SIGNATURE)) {
		printf("signature error!\n");
		goto FAIL;
	}

	hdrChksum = sigHdr.chksum;
	hdrChksum = ipchksum(&imgHdr, sizeof(imgHdr), hdrChksum);
	if (hdrChksum) {
		printf("signature checksum error!\n");
		goto FAIL;
	}
#else
	memcpy(&imgHdr, buf, sizeof(imgHdr));
	/*ql:20080729 START: because image key is matched to IC type, so we had better
	* check if it is config file or boot file, if not, it must be application image.
	*/
#ifdef MULTI_IC_SUPPORT
	key = getImgKey();
	if ((key != (imgHdr.key & key)) || (((imgHdr.key>>28)&0xf) != ((key>>28)&0xf))) {
		printf("[Multi IC]image header key error!\n");
		goto FAIL;
	}
#else
#ifdef CONFIG_RTL8686
	switch(imgHdr.key){
		case APPLICATION_UBOOT:
		case APPLICATION_UIMAGE:
		case APPLICATION_ROOTFS:
			printf("%s-%d, got header::%x\n", __func__,__LINE__,imgHdr.key);
			err = 0;
			break;
		default:
			printf("%s-%d, Unknown header::%x\n", __func__,__LINE__,imgHdr.key);
			err = 1;
			break;
	}
	if(err)
		goto FAIL;
#else
	if (imgHdr.key != APPLICATION_IMAGE) {
		CWMPDBP(1, "Image header key error!\n");
		goto FAIL;
	}
#endif
#endif // #else of #ifdef MULTI_IC_SUPPORT
			/*ql:20080729 END*/
#endif
	cmd_killproc(ALL_PID & ~(1<<PID_CWMP));
	data->fw_hdr_checked = 1;
	return 0;

FAIL:
	data->fw_hdr_checked = 1;
	return -1;
#else	//ENABLE_SIGNATURE_ADV
	data->fw_hdr_checked = 1;
	return 0;
#endif
}
#endif	//CONFIG_LUNA_FIRMWARE_UPGRADE_SUPPORT

#ifdef CONFIG_CWMP_TR181_SUPPORT
static int notify_load_tr181_default()
{
	notify_set_attributes( "Device.DeviceInfo.HardwareVersion", CWMP_NTF_FORCED|CWMP_NTF_PAS, CWMP_ACS_MASK );
	notify_set_attributes( "Device.DeviceInfo.SoftwareVersion", CWMP_NTF_FORCED|CWMP_NTF_ACT, CWMP_ACS_MASK );
	notify_set_attributes( "Device.DeviceInfo.ProvisioningCode", CWMP_NTF_FORCED|CWMP_NTF_ACT, CWMP_ACS_MASK );
	notify_set_attributes( "Device.ManagementServer.ConnectionRequestURL", CWMP_NTF_FORCED|CWMP_NTF_ACT, CWMP_ACS_MASK  );
	notify_set_attributes( "Device.ManagementServer.ParameterKey", CWMP_NTF_FORCED|CWMP_NTF_PAS, CWMP_ACS_MASK  );

	return 0;
}
#endif

static int notify_load_tr098_default()
{
#ifdef CONFIG_MIDDLEWARE
	unsigned char vChar;
	mib_get(CWMP_TR069_ENABLE,(void *)&vChar);
	if( (vChar == 1)|| (vChar == 2) ){//if enable tr069
#endif
		//forced inform parameters
		notify_set_attributes( "InternetGatewayDevice.DeviceSummary", CWMP_NTF_FORCED|CWMP_NTF_PAS, CWMP_ACS_MASK );
		notify_set_attributes( "InternetGatewayDevice.DeviceInfo.SpecVersion", CWMP_NTF_FORCED|CWMP_NTF_PAS, CWMP_ACS_MASK );
		notify_set_attributes( "InternetGatewayDevice.DeviceInfo.HardwareVersion", CWMP_NTF_FORCED|CWMP_NTF_PAS, CWMP_ACS_MASK );
		notify_set_attributes( "InternetGatewayDevice.DeviceInfo.SoftwareVersion", CWMP_NTF_FORCED|CWMP_NTF_ACT, CWMP_ACS_MASK );
		notify_set_attributes( "InternetGatewayDevice.DeviceInfo.ProvisioningCode", CWMP_NTF_FORCED|CWMP_NTF_ACT, CWMP_ACS_MASK );
		notify_set_attributes( "InternetGatewayDevice.ManagementServer.ConnectionRequestURL", CWMP_NTF_FORCED|CWMP_NTF_ACT, CWMP_ACS_MASK  );
		notify_set_attributes( "InternetGatewayDevice.ManagementServer.ParameterKey", CWMP_NTF_FORCED|CWMP_NTF_PAS, CWMP_ACS_MASK  );

#ifdef _PRMT_X_CT_COM_USERINFO_
		notify_set_attributes( "InternetGatewayDevice.X_CT-COM_UserInfo.UserName", CWMP_NTF_FORCED|CWMP_NTF_PAS, CWMP_ACS_MASK );
#endif

#ifdef CONFIG_MIDDLEWARE
		}
#endif

	return 0;
}

int port_notify_load_default()
{
#ifdef CONFIG_CWMP_TR181_SUPPORT
	if(gUseTR181)
		notify_load_tr181_default();
	else
#endif
		notify_load_tr098_default();
	return 0;
}

int port_notify_save( char *name )
{
#ifdef CONFIG_USER_FLATFSD_XXX
	if( va_cmd( "/bin/flatfsd",1,1,"-s" ) )
		CWMPDBG( 0, ( stderr, "<%s:%d>exec 'flatfsd -s' error!\n", __FUNCTION__, __LINE__ ) );
#endif

	return 0;
}

/* List of old WAN configuration, end with NULL */
MIB_CE_ATM_VC_T *old_wan_config = NULL;
int old_wan_total = 0;

void free_old_wan_config()
{
	if(old_wan_config)
	{
		free(old_wan_config);
		old_wan_config = NULL;
		old_wan_total = 0;
	}
}

void wan_config_backup(MIB_CE_ATM_VC_T *wan_config, int size)
{
	int i;

	for(i = 0 ; i < size ; i++)
		mib_chain_get(MIB_ATM_VC_TBL, i, (void *) &wan_config[i]);
}

void wan_config_restore(MIB_CE_ATM_VC_T *wan_config, int size)
{
	int i;

	mib_chain_clear(MIB_ATM_VC_TBL);

	for(i = 0 ; i < size ; i++)
		mib_chain_add(MIB_ATM_VC_TBL, (void *) &wan_config[i]);
}

int port_before_session_start(struct cwmp_userdata *data)
{
#ifdef E8B_NEW_DIAGNOSE
	char tmpbuf[256 + 1];
	FILE *fp;
	int cwmp_found = 0;
	int wan_found = 0;
	int acs_setting = 1;
	int num, i;
	MIB_CE_ATM_VC_T entry;
	int flags;
	struct in_addr inAddr;

	/* whether there is a WAN connection with an IP address */
	num = mib_chain_total(MIB_ATM_VC_TBL);
	for (i = 0; i < num; i ++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, &entry))
			continue;

		if (!(entry.applicationtype & X_CT_SRV_TR069))
			continue;

		cwmp_found = 1;

		if (!entry.enable)
			continue;

		ifGetName(entry.ifIndex, tmpbuf, sizeof(tmpbuf));

		if (getInFlags(tmpbuf, &flags)) {
			if ((flags & IFF_UP) && getInAddr(tmpbuf, IP_ADDR, &inAddr)) {
				wan_found = 1;
				break;
			}
		}
	}

	/* whether ACS-related MIB setting is all right */
	if (mib_get(CWMP_ACS_URL, tmpbuf) == 0 || strlen(tmpbuf) == 0)
		acs_setting = 0;
	if (mib_get(CWMP_ACS_USERNAME, tmpbuf) == 0 || strlen(tmpbuf) == 0)
		acs_setting = 0;
	if (mib_get(CWMP_ACS_PASSWORD, tmpbuf) == 0 || strlen(tmpbuf) == 0)
		acs_setting = 0;

	if (cwmp_found == 0) {
		sprintf(tmpbuf, "%d:%s", NO_INFORM, NO_CWMP_CONNECTION);
	} else if (wan_found == 0) {
		sprintf(tmpbuf, "%d:%s", NO_INFORM, CWMP_CONNECTION_DISABLE);
	} else if (acs_setting == 0) {
		sprintf(tmpbuf, "%d:%s", NO_INFORM, CWMP_NO_ACSSETTING);
	} else {
		tmpbuf[0] = '\0';
	}

	if (strlen(tmpbuf) != 0) {
		fp = fopen(INFORM_STATUS_FILE, "w");
		if (fp) {
			fprintf(fp, "%s", tmpbuf);
			fclose(fp);
		}
		return -1;
	}
#endif

	free_old_wan_config();
	gWanBitMap = 0;

	// Backup current WAN configuration
	old_wan_total = mib_chain_total(MIB_ATM_VC_TBL);
	old_wan_config = (MIB_CE_ATM_VC_Tp) malloc(sizeof(MIB_CE_ATM_VC_T) * old_wan_total);
	wan_config_backup(old_wan_config, old_wan_total);
#ifdef CONFIG_USER_TR104
	cwmpDoRefresh();
#endif
	return 0;
}


int port_session_closed(struct cwmp_userdata *data)
{
	unsigned int delay=3;
	unsigned char byte= 0;

	if (gStartPing)
	{
#ifdef __CLOSE_INTERFACE_BEFORE_WRITE_
		/*delay some seconds to ready*/
		{while(delay!=0) delay=sleep(delay);}
#endif /*__CLOSE_INTERFACE_BEFORE_WRITE_*/
		cwmpStartPingDiag();
		gStartPing = 0;
	}
#ifdef CONFIG_DEV_xDSL
	else if(gStartDSLDiag)
	{
		cwmpSetCpeHold( 1 );
		cwmpStartDSLDiag();
		gStartDSLDiag=0;
	}
	else if(gStartATMF5LB)
	{
		cwmpStartATMF5LB();
		gStartATMF5LB=0;
	}
#endif
#ifdef _PRMT_TR143_
	else if(gStartTR143DownloadDiag)
	{
#ifdef __CLOSE_INTERFACE_BEFORE_WRITE_
		/*delay some seconds to ready*/
		{while(delay!=0) delay=sleep(delay);}
#endif /*__CLOSE_INTERFACE_BEFORE_WRITE_*/
		StartTR143DownloadDiag();
		gStartTR143DownloadDiag=0;
	}
	else if(gStartTR143UploadDiag)
	{
#ifdef __CLOSE_INTERFACE_BEFORE_WRITE_
		/*delay some seconds to ready*/
		{while(delay!=0) delay=sleep(delay);}
#endif /*__CLOSE_INTERFACE_BEFORE_WRITE_*/
		StartTR143UploadDiag();
		gStartTR143UploadDiag=0;
	}
#endif //_PRMT_TR143_
#ifdef _SUPPORT_TRACEROUTE_PROFILE_
	else if(gStartTraceRouteDiag)
	{
	#ifdef __CLOSE_INTERFACE_BEFORE_WRITE_
		/*delay some seconds to ready*/
		{while(delay!=0) delay=sleep(delay);}
	#endif /*__CLOSE_INTERFACE_BEFORE_WRITE_*/
		StartTraceRouteDiag();
		gStartTraceRouteDiag=0;
	}
#endif //_SUPPORT_TRACEROUTE_PROFILE_
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	else if(gStartReset)
	{
	#ifdef __CLOSE_INTERFACE_BEFORE_WRITE_
		/*delay some seconds to ready*/
		{while(delay!=0) delay=sleep(delay);}
	#endif /*__CLOSE_INTERFACE_BEFORE_WRITE_*/
		cwmpStartReset();
		gStartReset=0;
	}
#endif
/*ping_zhang:20081217 END*/
/*alex_hung:20110923 support TR-104, CONFIG_USER_TR104*/
#ifdef CONFIG_USER_TR104
	else if(gVoipReq)
        {
	#ifdef __CLOSE_INTERFACE_BEFORE_WRITE_
            /*delay some seconds to ready*/
            {while(delay!=0) delay=sleep(delay);}
	#endif /*__CLOSE_INTERFACE_BEFORE_WRITE_*/
            cwmpStartVoipDiag();
	}
#endif /*CONFIG_USER_TR104 END*/
#ifdef _PRMT_NSLOOKUP_
	else if(gStartNSLookupDiag)
	{
	#ifdef __CLOSE_INTERFACE_BEFORE_WRITE_
		/*delay some seconds to ready*/
		{while(delay!=0) delay=sleep(delay);}
	#endif /*__CLOSE_INTERFACE_BEFORE_WRITE_*/
		StartNSLookupDiag();
		gStartNSLookupDiag = 0;
	}
#endif //_PRMT_NSLOOKUP_

#ifdef _PRMT_X_CT_COM_PING_
	ct_check_ping();
#endif

#ifdef CONFIG_USER_CWMP_UPNP_DM
	upnpdm_check_download();
#endif

	unlink(CONFIG_FILE_NAME);
#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
	unlink(FILE4CaptivePortal);
#endif //_SUPPORT_CAPTIVEPORTAL_PROFILE_
#ifdef SUPPORT_DHCP_RESERVED_IPADDR
	unlink(FILE4DHCPReservedIPAddr);
#endif //SUPPORT_DHCP_RESERVED_IPADDR

	return 0;
}

int port_restart_wan(void)
{
	MIB_CE_ATM_VC_T Entry;
	int i, nrWan;

	for (i = 0, nrWan = 0; i < MAX_VC_NUM; i++) {
		if (gWanBitMap & (1U << i)) {
			nrWan++;
		}
	}

	if(old_wan_total && old_wan_config)
	{
		int wan_total = mib_chain_total(MIB_ATM_VC_TBL);
		MIB_CE_ATM_VC_T *wan_config = (MIB_CE_ATM_VC_T *) malloc(sizeof(MIB_CE_ATM_VC_T) * wan_total);

		// restore old config & delete WAN interfaces
		if(wan_config == NULL)
		{
			fprintf(stderr, "<%s:%d> malloc failed\n", __FUNCTION__, __LINE__);
			gWanBitMap = 0;
			return -1;
		}
		wan_config_backup(wan_config, wan_total);
		wan_config_restore(old_wan_config, old_wan_total);

		if (nrWan == MAX_VC_NUM) {
			deleteConnection(CONFIGALL, NULL);
		}
		else {
			for (i = 0; i < MAX_VC_NUM; i++) {
				if (gWanBitMap & (1U << i)) {
					if (mib_chain_get(MIB_ATM_VC_TBL, i, &Entry) != 0) {
						deleteConnection(CONFIGONE, &Entry);
					}
				}
			}
		}

		// restore new config
		wan_config_restore(wan_config, wan_total);

		free(wan_config);
		free_old_wan_config();
	}

	if (nrWan == MAX_VC_NUM) {
		restartWAN(CONFIGALL, NULL);
	}
	else {
		for (i = 0; i < MAX_VC_NUM; i++) {
			if (gWanBitMap & (1U << i)) {
				if (mib_chain_get(MIB_ATM_VC_TBL, i, &Entry) != 0) {
					restartWAN(CONFIGONE, &Entry);
				}
			}
		}
	}

	gWanBitMap = 0;
	return 0;
}

int port_backup_config( void )
{
	int ret=0;
	fprintf( stderr, "<%s:%d>\n", __FUNCTION__, __LINE__ );fflush(stderr);

	ret = mib_backup(CONFIG_MIB_ALL);
	//fprintf( stderr, "<%s:%d>mib_backup return: %d\n", __FUNCTION__, __LINE__, ret );fflush(stderr);
	return 0;
}

int port_restore_config( void )
{
	int ret=0;
	fprintf( stderr, "<%s:%d>\n", __FUNCTION__, __LINE__ );fflush(stderr);

	ret=mib_restore(CONFIG_MIB_ALL);
	//fprintf( stderr, "<%s:%d>mib_restore return: %d\n", __FUNCTION__, __LINE__, ret );fflush(stderr);
	return 0;
}

#ifdef CONFIG_E8B
void init_resetuserinfo_timer();

int port_is_dsl_mode_changed()
{
	int changed = 0, initial = 0;
#ifdef CONFIG_VDSL
	int mode;	//0: initial, 1: ADSL, 2: VDSL
	XDSL_OP *op = xdsl_get_op(0);
	unsigned char old_mode;


	op->xdsl_msg_get(GetPmdMode, &mode);

	mib_get(MIB_OLD_DSL_MODE, &old_mode);

	if(old_mode == 0)
		initial = 1;

	if(mode & (MODE_VDSL2 | MODE_VDSL1))
	{
		if(old_mode == 1)
			changed = 1;
		old_mode = 2;
	}
	else
	{
		if(old_mode == 2)
			changed = 1;
		old_mode = 1;
	}

	if(changed || initial)
		mib_set(MIB_OLD_DSL_MODE, &old_mode);
#endif

	return changed;
}

int isTR069(char *name)
{
	unsigned int devnum,ipnum,pppnum;
	MIB_CE_ATM_VC_T Entry;
	unsigned int total;
	int i;

	devnum = getWANConDevInstNum( name );
	ipnum  = getWANIPConInstNum( name );
	pppnum = getWANPPPConInstNum( name );

	total = mib_chain_total(MIB_ATM_VC_TBL);
	for(i=0;i<total;i++){
		if(mib_chain_get(MIB_ATM_VC_TBL,i,&Entry)!=1)
			continue;
		if(Entry.ConDevInstNum==devnum
			&& Entry.ConIPInstNum==ipnum
			&& Entry.ConPPPInstNum==pppnum
			&& (Entry.applicationtype&X_CT_SRV_TR069)) {
			return 1;
		}
	}

	return 0;

}
int isINTERNET(char *name)
{
	unsigned int devnum,ipnum,pppnum;
	MIB_CE_ATM_VC_T Entry;
	unsigned int total;
	int i;

	devnum = getWANConDevInstNum( name );
	ipnum  = getWANIPConInstNum( name );
	pppnum = getWANPPPConInstNum( name );

	total = mib_chain_total(MIB_ATM_VC_TBL);
	for(i=0;i<total;i++){
		if(mib_chain_get(MIB_ATM_VC_TBL,i,&Entry)!=1)
			continue;
		if(Entry.ConDevInstNum==devnum
			&& Entry.ConIPInstNum==ipnum
			&& Entry.ConPPPInstNum==pppnum
			&& (Entry.applicationtype&X_CT_SRV_INTERNET)) {
			return 1;
		}
	}

	return 0;


}

#ifdef _PRMT_X_CT_COM_USERINFO_
void resetuserinfo(void)
{
	unsigned int regstatus = 99;
	unsigned int regtimes = 0;
	unsigned char informStatus = CWMP_REG_IDLE;

	mib_set(CWMP_USERINFO_STATUS, &regstatus);
	mib_set(CWMP_USERINFO_TIMES, &regtimes);
	mib_set(CWMP_REG_INFORM_STATUS, &informStatus);
}

extern int reset_reg_times_timer;
void init_resetuserinfo_timer()
{
	unsigned int regLimit;
	unsigned int regTimes;

	mib_get(CWMP_USERINFO_LIMIT, &regLimit);
	mib_get(CWMP_USERINFO_TIMES, &regTimes);

	if(regTimes >= regLimit)
	{
		CWMPDBG(1, (stderr, "<%s:%d> Reach register retry limit, setting reset timer....\n", __FUNCTION__, __LINE__));
		reset_reg_times_timer = 180;	//user can register after 3 mins.
	}
	else
		reset_reg_times_timer = -1;
}
#endif
#endif

int port_get_tr069_ifname(char *ifname)
{
#ifdef CONFIG_USER_RTK_WAN_CTYPE
	int total = mib_chain_total(MIB_ATM_VC_TBL);
	int i;
	MIB_CE_ATM_VC_T entry;
	struct in_addr inAddr;
	int flags = 0;

	for(i = 0 ; i < total ; i++)
	{
		if(mib_chain_get(MIB_ATM_VC_TBL, i, &entry)!=1)
			continue;

		if(entry.applicationtype & X_CT_SRV_TR069)
		{
			if(ifGetName( entry.ifIndex, ifname, IFNAMSIZ) == 0)
				continue;

			if (getInFlags( ifname, &flags) == 1 && flags & IFF_UP)
			{
				if (getInAddr(ifname, IP_ADDR, (void *)&inAddr) == 1)
					return 0;
			}
		}
	}
	return -1;
#else
	int cwmp_ifidx = DUMMY_IFINDEX;

	mib_get(CWMP_WAN_INTERFACE, (void *)&cwmp_ifidx);

	if(cwmp_ifidx == DUMMY_IFINDEX)
		return 0;	// No TR-069 interface specified

	if(ifGetName(cwmp_ifidx, ifname, IFNAMSIZ) == NULL)
		return -1;	//error
	return 0;
#endif
}

int port_bindAddrOnTR069Interface(struct sockaddr_in *myaddr)
{
	int ret = -1;
	unsigned int i,num;
	char ifname[IFNAMSIZ] = {0};
	struct in_addr inAddr;
	int flags;

	if(port_get_tr069_ifname(ifname) == 0)
	{
#ifdef CONFIG_USER_RTK_WAN_CTYPE
		if(getInAddr(ifname, IP_ADDR, (void *)&inAddr) == 1)
#else
		if(ifname[0] != '\0' && getInAddr(ifname, IP_ADDR, (void *)&inAddr) == 1)
#endif
		{
			printf("Get TR069 ip is %s\n", inet_ntoa(inAddr));
			ret = 0;
		}
	}

	if (ret == 0) {
		myaddr->sin_addr.s_addr = htonl(inAddr.s_addr);
	}
	else {
		fprintf(stderr, "<%s:%d> TR-069 interface is not specified\n", __FUNCTION__, __LINE__);
		myaddr->sin_addr.s_addr = htonl(INADDR_ANY);
	}

	return 0;
}

void port_acs_url_changed( struct cwmp_userdata *data )
{
	return;
}


/* Load parameter list in SET_REJECT_FILE. */

#define SET_REJECT_FILE "/etc/setprmt_reject"
#define MAX_REJ_LIST 20

/* used to store compiled regular expression */
regex_t rej_set_regs[MAX_REJ_LIST] = {0};
size_t rej_set_list_size = -1;		//-1 means list is not loaded.

void load_reject_list()
{
	FILE *file = NULL;
	char path[MAX_PRMT_NAME_LEN] = {0};

	rej_set_list_size = 0;

	file = fopen(SET_REJECT_FILE, "r");
	if(file == NULL)
	{
		CWMPDBP(1, "%s is not found, ignore it.\n", SET_REJECT_FILE);
		return;
	}

	// Parse parameters we want to reject the set operation.
	while(fgets(path, MAX_PRMT_NAME_LEN, file) != NULL)
	{
		char line_got = 0;	// -1 means error
		size_t len = 0;
		int i = 0, j = 1;
		char expr[MAX_PRMT_NAME_LEN + 32] = "^";

		len = strlen(path);

		if(len <= 1 || path[0] == '#')
			continue;

		for(i = 0 ; i < len || !line_got ; i++, j++)
		{
			switch(path[i])
			{
			case '\n':
			case '\0':
				expr[j] = '\0';
				line_got = 1;
				break;
			case '.':
				expr[j++] = '\\';
				expr[j] = '.';
				break;
			case '{':
				//let "{i}" be any string
				if( i+2 >= len || path[++i] != 'i' || path[++i] != '}')
				{
					line_got = -1;
					break;
				}

				expr[j++] = '.';
				expr[j] = '*';
				break;
			case ' ':
				line_got = -1;
				break;
			default:
				expr[j] = path[i];
				break;
			}
		}

		if(line_got == -1)
		{
			CWMPDBP(1, "Invalid parameter name: %s\n", path);
			continue;
		}

		// line_got == 1
		if(regcomp(&rej_set_regs[rej_set_list_size], expr, REG_NOSUB) != 0)
		{
			CWMPDBP(1, "\"%s\" is failed to compile.\n", expr);
			continue;
		}

		if(++rej_set_list_size >= MAX_REJ_LIST)
			break;
	}

	fclose(file);
	CWMPDBP(1, "Total %d rules is compiled.\n", rej_set_list_size);
}

/* If `prmt_name` is in the reject list, return ERR_9001, else return 0. */
int port_before_SetParameterValues(char *prmt_name)
{
	int i = 0;
	int err;

	CWMPDBP(2, "Before SetParameterValues %s\n", prmt_name);

	if(rej_set_list_size == -1)
	{
		CWMPDBP(2, "Initializing reject list....\n");
		load_reject_list();
	}

	for(i = 0 ; i < rej_set_list_size ; i++)
	{
		err = regexec(&rej_set_regs[i], prmt_name, 0, NULL, 0);
		if(err == 0)
		{
			CWMPDBP(1, "\"%s\" is matched, reject set operation\n", prmt_name);
			return ERR_9001;
		}
		else if(err != REG_NOMATCH)
		{
			char msgbuf[100] = {0};
			regerror(err, &rej_set_regs[i], msgbuf, sizeof(msgbuf));
			CWMPDBP(1, "Regex match failed: %s\n", msgbuf);
		}
	}

	return 0;
}

