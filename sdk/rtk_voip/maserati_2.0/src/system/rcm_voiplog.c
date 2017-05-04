#include <time.h>
#include <sys/syslog.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/time.h>
#include <time.h>

#include "rcm_voiplog.h"
#include "rcm_sim_defs.h"	

//[SD6, bohungwu, rcm_log_print
//[WBH
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
//]
#define RCM_LOG_FILE_MAX_SIZE (128) //unit=KB, 128KB
#define RCM_LOG_FILE_MAX_NUM (4)
#define KB (1024)
int rcm_log_file_rotate(void);
int rcm_log_file_init(void);

static int rcm_log_fd = -1;

unsigned long category_mask=LOG_SLIC_EVENT|LOG_OSIP_EVENT|LOG_CALL_MGNT|LOG_VOIP_MISC;

#define RCM_LOG_FILE_DISABLE 0
#define RCM_LOG_FILE_ENABLE  1
static unsigned int rcm_log_file_max_size = RCM_LOG_FILE_MAX_SIZE;
static unsigned int rcm_log_file_max_num  = RCM_LOG_FILE_MAX_NUM;


static unsigned int rcm_log_onoff  = RCM_LOG_FILE_DISABLE;

static char rcm_log_file_name[FILENAME_MAX] = "/var/log/maserati.log";

static struct osip_mutex *rcm_log_mutex = NULL;

/*______________________________________________________________________________
**	rcm_voiplog_write
**
**	descriptions: write log text to syslog
**	parameters: category, static message 
**	local:
**	global:
**	return:
**	called by:
**	call:
**	revision:
**____________________________________________________________________________*/

void rcm_voiplog_write(const int category, char *message){
	
	char log_text[LOG_TEXT_MAXLEN];
	int log_len;
	char timestamp_txt[20];
	struct timeval tv;
	struct tm *tm;
	static int output_size = 0;

	if(rcm_log_onoff == RCM_LOG_FILE_DISABLE)
		return;
	
	osip_mutex_lock(rcm_log_mutex);

	/* sample code:

	printf("TIMESTAMP-START\t  %d:%02d:%02d:%d (~%d ms)\n", tm->tm_hour,
		   tm->tm_min, tm->tm_sec, tv.tv_usec,
		   tm->tm_hour * 3600 * 1000 + tm->tm_min * 60 * 1000 +
		   tm->tm_sec * 1000 + tv.tv_usec / 1000);
	*/

	switch(category & category_mask){
		case LOG_SLIC_EVENT:
			sprintf(log_text, "%s %s", SLIC_EVENT_TXT, message);
			break;
		case LOG_OSIP_EVENT:
			sprintf(log_text, "%s %s", OSIP_EVENT_TXT, message);
			break;
		case LOG_CALL_MGNT:
			sprintf(log_text, "%s %s", CALL_MGNT_TXT, message);
			break;
		case LOG_VOIP_MISC:	
			sprintf(log_text, "%s %s", VOIP_MISC_TXT, message);
			break;

		case LOG_VOIP_DEBUG:	
			sprintf(log_text, "%s %s", VOIP_DEBUG_TEXT, message);
			break;
		case LOG_DSP_EVENT:	
			sprintf(log_text, "%s %s", DSP_EVENT_TXT, message);
			break;				
		default:
			return;
			break;
	}

	gettimeofday(&tv,NULL);
	tm = localtime(&tv.tv_sec);
	sprintf(timestamp_txt,"%d:%02d:%02d.%03d",tm->tm_hour,tm->tm_min, tm->tm_sec, tv.tv_usec/1000);

	//ericchung : for DEBUG usage
	//printf("%s %s",timestamp_txt,log_text);
	if(rcm_log_fd == -1)
	{
		//rcm_log_fd = open("/var/log/rcm.log", O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
		rcm_log_fd = open(rcm_log_file_name, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
		//fd = open("/var/log/rcm.log", O_WRONLY | O_CREAT | O_TRUNC);
		if(rcm_log_fd == -1)
			perror("Could not open file");
	}

	if( (write(rcm_log_fd, timestamp_txt, strlen(timestamp_txt)) != strlen(timestamp_txt)) ||
	    (write(rcm_log_fd, log_text, strlen(log_text)) != strlen(log_text)) )
	{
		perror("Failed to write");
	}
	else
	{
		output_size += strlen(timestamp_txt);
		output_size += strlen(log_text);
		
		if(output_size > (rcm_log_file_max_size*KB))
		{
			char *info_str = "RCM LOG ROTATED!\n";

			write(rcm_log_fd, info_str, strlen(info_str));
			close(rcm_log_fd);
			rcm_log_fd = -1;
			rcm_log_file_rotate();
			output_size = 0;
		}
	}

	

#if 0 //use printf
	printf("%s\n",log_text);
#else
	//eric.chung: need modify LOG level , type
	syslog(LOG_LOCAL1|LOG_INFO, "%s %s",timestamp_txt,log_text);
#endif


	//printf("%s\n",log_text);
	rcm_sim_log( "%s\n" , log_text );

	osip_mutex_unlock(rcm_log_mutex);	
}


/*______________________________________________________________________________
**	rcm_voipSyslog
**
**	descriptions: write log text to syslog
**	parameters: category, dynamic string 
**	local:
**	global:
**	return:
**	called by:
**	call:
**	revision:
**____________________________________________________________________________*/

void rcm_voipSyslog(const int category,char *chfr,
	    ...)
{

	va_list args;
	char msg[LOG_TEXT_MAXLEN];
	
	va_start(args, chfr);
	vsprintf(msg, chfr, args);
	va_end(args);

	rcm_voiplog_write(category,msg);

}



/*______________________________________________________________________________
**	rcm_LinphoneSyslog
**
**	descriptions: write log text to syslog
**	parameters: category, linphonecore , dynamic string 
**	local:
**	global:
**	return:
**	called by:
**	call:
**	revision:
**____________________________________________________________________________*/

void rcm_LinphoneSyslog(const int category,LinphoneCore *lc,char *chfr,
	    ...)
{

	va_list args;
	char msg[LOG_TEXT_MAXLEN];
	char log_text[LOG_TEXT_MAXLEN];
	va_start(args, chfr);
	vsprintf(msg, chfr, args);
	va_end(args);

	sprintf(log_text, "[CH%d] %s", lc->chid, msg);
	

	rcm_voiplog_write(category,msg);

}


int rcm_log_file_rotate(void)
{
	int ret_val=0;
	int i;
	int err;

	for(i=rcm_log_file_max_num; i>=0; i--)
	{
		char old_file[FILENAME_MAX] = {0};
		char new_file[FILENAME_MAX] = {0};

		if(i!=0)
			snprintf(old_file, FILENAME_MAX, "%s.%d", rcm_log_file_name, i-1);
		else
			snprintf(old_file, FILENAME_MAX, "%s", rcm_log_file_name);

		snprintf(new_file, FILENAME_MAX, "%s.%d", rcm_log_file_name, i);
		err = rename(old_file, new_file);
		if(err !=0)
		{
			//perror("%s(%d), failed to rename file\n", __FILE__, __LINE__);
			//perror("failed to rename file\n");
			if(errno != ENOENT)
			{
				fprintf(stderr, "%s(err=%d)\n", strerror(errno), errno);
				ret_val = -1;
				//break;
			}
		}
	}
	

	return ret_val;
}


int rcm_log_file_init(void)
{
	int ret_val = 0;
	char *env;

	/* Load setting from ENV variables if any */
	env = getenv("RCM_LOG_ONOFF");
	if(env == NULL)
	{
		//fprintf(stderr, "RCM_LOG_ONOFF is null\n");
	}
	else
	{
		rcm_log_onoff = atoi(env);
		//fprintf(stderr, "RCM_LOG_ONOFF is %d\n", rcm_log_onoff);
	}
	env = getenv("RCM_LOG_FILE_MAX_NUM");
	if(env == NULL)
	{
	//	fprintf(stderr, "RCM_LOG_FILE_MAX_NUM is null\n");
	}
	else
	{
		rcm_log_file_max_num = atoi(env);
		fprintf(stderr, "RCM_LOG_FILE_MAX_NUM is %s\n", env);
	}
	env = getenv("RCM_LOG_FILE_MAX_SIZE");
	if(env == NULL)
	{
		//fprintf(stderr, "RCM_LOG_FILE_MAX_SIZE is null\n");
	}
	else
	{
		rcm_log_file_max_size = atoi(env);
		fprintf(stderr, "RCM_LOG_FILE_MAX_SIZE is %s\n", env);
	}
	env = getenv("RCM_NAME");
	if(env == NULL)
	{
	//	fprintf(stderr, "RCM_NAME is null\n");
	}
	else
	{
		snprintf(rcm_log_file_name, FILENAME_MAX, "/var/log/%s.log", env);
		fprintf(stderr, "RCM_NAME is %s, log file is stored at %s\n", env, rcm_log_file_name);
	}

	/* Configure RCM log file module based upon settting */
//	fprintf(stderr, "\n\n=======================\n");
	if(rcm_log_onoff == RCM_LOG_FILE_ENABLE)
	{
		if(rcm_log_mutex != NULL)
		{
			osip_mutex_destroy(rcm_log_mutex);	//Alex, 20111122, Valgrind, invalid free
		}
		rcm_log_mutex = osip_mutex_init();

		if(rcm_log_fd != -1)
		{
			close(rcm_log_fd);
			rcm_log_fd = -1;
		}
		rcm_log_file_rotate();
		
		//Linphone
		g_SystemDebug = DEBUG_B_ERROR | DEBUG_B_WARNING | DEBUG_B_MESSAGE;
		//g_SystemDebug = DEBUG_B_ERROR | DEBUG_B_WARNING | DEBUG_B_MESSAGE |DEBUG_B_CONSOLE;

		//eXosip
		osip_trace_initialize (OSIP_INFO4, NULL);
		fprintf(stderr, "VoIP RCM LOG is ENABLED!!\n");
	}
	else
	{

//		fprintf(stderr, "RCM LOG is DISABLED!!\n");

	}
	//fprintf(stderr, "=======================\n\n");
		
	return ret_val;	
}

