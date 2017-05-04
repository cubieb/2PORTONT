#include <sys/msg.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "ivripc.h"
#include "ivrhandler.h"
#include "voip_flash.h"

static int qidServer, qidClient;
static char *pidfile = "/var/run/ivrserver.pid";

voip_flash_share_t *g_pVoIPShare;
int g_mapSupportedCodec[_CODEC_MAX];
int g_nMaxCodec;

static void InitIvrServer( void )
{
	key_t key;

	/* message queue */
	key = ftok( IVR_SERVER_PATHNAME, IVR_SERVER_VERSION );

	if( ( qidServer = msgget( key, 0666 | IPC_CREAT /* | IPC_EXCL */ ) ) == -1 ) {
		printf( "msgget(S) fail: %X, %d\n", key, errno );
		return;
	}
	
	key += IVR_CLIENT_KEY_OFFSET;
	
	if( ( qidClient = msgget( key, 0666 | IPC_CREAT /* | IPC_EXCL */ ) ) == -1 ) {
		printf( "msgget(C) fail: %X, %d\n", key, errno );
		return;
	}
	
	/* share memory */
	if (voip_flash_client_init(&g_pVoIPShare, VOIP_FLASH_WRITE_CLIENT_IVRSERVER) == -1)
	{
		fprintf(stderr, "voip_flash_client_init failed.\n");
		return;
	}

	/* initialize g_VoIP_Feature */
	rtk_Get_VoIP_Feature();
}

static void TerminateIvrServer( void )
{
	/* message queue */
	if( msgctl( qidServer, IPC_RMID, 0 ) == -1 ) {
		printf( "msgctl(S) fail: %d\n", errno );
	}
	
	if( msgctl( qidClient, IPC_RMID, 0 ) == -1 ) {
		printf( "msgctl(C) fail: %d\n", errno );
	}	

	/* share memory */
	voip_flash_client_close();
}

static void IvrServerAbort( int sig )
{
	TerminateIvrServer();
	
	exit( 0 );
}

static int SetGlobalConstant( const send_global_t *global )
{
	int i;

	g_nMaxCodec = global ->nMaxCodec;
	
	for( i = 0; i < _CODEC_MAX; i ++ )
		g_mapSupportedCodec[ i ] = global ->mapSupportedCodec[ i ];

	return 0;
}

static int pidfile_acquire(char *pidfile) 
{ 
	int pid_fd; 
	
	if(pidfile == NULL) 
	       return -1; 
	
	pid_fd = open(pidfile, O_CREAT | O_WRONLY, 0644); 
	if (pid_fd < 0) 
	       printf("Unable to open pidfile %s\n", pidfile); 
	else 
	       lockf(pid_fd, F_LOCK, 0); 
	
	return pid_fd; 
} 

static void pidfile_write_release(int pid_fd) 
{ 
	FILE *out; 
	
	if(pid_fd < 0) 
	       return; 
	
	if((out = fdopen(pid_fd, "w")) != NULL) { 
	       fprintf(out, "%d\n", getpid()); 
	       fclose(out); 
	} 
	lockf(pid_fd, F_UNLCK, 0); 
	close(pid_fd); 
} 

int main()
{
	ivr_ipc_msg_t msgdata;
	ivr_ipc_ret_t retdata;
		
	// destroy old process and create a PID file 
	{ 
		int pid_fd; 
		FILE *fp; 
		char line[20]; 
		pid_t pid; 
		
		if ((fp = fopen(pidfile, "r")) != NULL) { 
			fgets(line, sizeof(line), fp); 
			if (sscanf(line, "%d", &pid)) { 
			       if (pid > 1) 
			               kill(pid, SIGTERM); 
			} 
			fclose(fp); 
		} 
		
		pid_fd = pidfile_acquire(pidfile); 
		if (pid_fd < 0) 
			return 0; 
		pidfile_write_release(pid_fd); 
	} 

	/* initialize IVR server */
	InitIvrServer();
	
	/* initialize IVR handler */
	InitializeIVR();

	signal( SIGINT, IvrServerAbort );
	
	retdata.message_type = 1;	/* non-zero value */
	
	printf( "Startup IVR server...[ok]\n" );

	while( 1 ) {
		
		if( msgrcv( qidServer, &msgdata, sizeof( do_ivr_ins_t ), 0, 0 ) == -1 ) {
			printf( "Recv fail: %d\n", errno );
			continue;
		}
		
		/* ok. show this message */
		switch( msgdata.message_type ) {
		case IVR_IPC_MSG_DO_IVR_INS:
			retdata.ret = IsInstructionForIVR( &msgdata.do_ivr_ins );
			break;
			
		case IVR_IPC_MSG_SEND_GLOBAL:
			retdata.ret = SetGlobalConstant( &msgdata.send_global );
			break;
		
		case IVR_IPC_MSG_UPDATE_FLASH:
			voip_flash_client_update();
			retdata.ret = 0;
			break;
		
		default:
			retdata.ret = 0;
			printf( "Unknown message type(%d)?\n", msgdata.message_type );
			break;
		}
		
		/* give return value */
		if( msgsnd( qidClient, &retdata, IVR_RET_DATA_SIZE, 0 ) == -1 ) {
			printf( "msgsnd fail: %d\n", errno );
		}
	}
	
	/* Terminate IVR server */
	TerminateIvrServer();
		
	return 1;
}

