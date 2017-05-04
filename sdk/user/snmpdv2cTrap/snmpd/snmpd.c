#include <rtk/utility.h>
#include "autoconf.h"


#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
//#include	<signal.h>
#include	<unistd.h>

#include	<stdio.h>
#include	<stdlib.h>
#include	<netdb.h>
#include	<strings.h>

#include	"host.h"
#include	"ctypes.h"
#include	"debug.h"
#include	"rdx.h"
#include	"smp.h"
#include	"mis.h"
#include	"miv.h"
#include	"aps.h"
#include	"ap0.h"
#include	"asn.h"
#include	"asl.h"
#include	"avl.h"
#include	"udp.h"
#include	"systm.h"
#include	"tcp_vars.h"
#include	"ip_vars.h"
#include	"iface_vars.h"
#include	"icmp_vars.h"
#include	"udp_vars.h"

#include	"adsl_vars.h"
#include	"nscrt_vars.h"

#ifdef CONFIG_USER_SNMPD_MODULE_FRAMEWORK
#include	"framework_vars.h"
#endif

// Added by Mason Yu for PVC
#include	<linux/atm.h>   	// located on ~/linux-2.4.x/include directory
//#include 	<linux/atmdev.h>
//#include 	<linux/atmbr2684.h>
#include 	<errno.h>
#include 	<syslog.h>

#include 	"../modules/mib_tool.h"
#include	<rtk/adslif.h>
#include 	<sys/time.h>



#include 	<sys/signal.h>
#include 	<netinet/ip.h>
#include 	<netinet/ip_icmp.h>
#include 	<arpa/inet.h>
#include	<netdb.h>

#define		cmdBufferSize		(2048)
#define		RUNFILE "/var/run/snmpd.pid"

// Kaohj -- Remote Management
// cpeFtpOperStatus
#define	OPER_NORMAL		1
#define	OPER_CONNECT_SUCCESS	2
#define	OPER_CONNECT_FAILURE	3
#define	OPER_DOWNLOADING	4
#define	OPER_DOWNLOAD_SUCCESS	5
#define	OPER_DOWNLOAD_FAILURE	6
#define	OPER_SAVING		7
#define	OPER_SAVE_FAILURE	8
#define	OPER_UPGRADE_SUCCESS	9
#define	OPER_UPGRADE_FAILURE	10

btrap_t 	g_snmpBootTrapResponse = BTRAP_NO_RESPONSE;
// Added by Mason Yu for return the correct comm string
char		readComm[16], writeComm[16], pvcReadComm[16], pvcWriteComm[16];
int 		g_reboot_flag=0;
int 		g_save_flag=0;
int 		g_reboot2def_flag=0;
int 		g_ping_flag=0;
int 		g_ftp_upgrade_flag=0;
int 		g_ftp_test_flag=0;
int		g_pppTestFlag=0;
// Kaohj
int		g_ftp_operStatus=OPER_NORMAL;
int		g_ftp_totalSize=0;
int		g_ftp_doneSize=0;
int		g_ftp_elapseTime=0;
int		pvcsd, eocsd;
int		pvcFlag;
extern int g_pingOperStatus;


static 		struct sockaddr_in pingaddr;
static 		int pingsock = -1;
static 		long ntransmitted = 0, nreceived = 0, nrepeats = 0;
static		int myid = 0;
static 		int finished = 0;
static 		int btCnt;
static		int trapCnt;
static 		int		sTrap_m;
static 		u_long          lhostTrap_m;
static 		u_short         lportTrap_m;
static 		ApsIdType	communityId_m;
static 		CCharPtrType    lEOIDString_m;
static		long 		btrap_last_scan=0;

int 		g_PINGCOUNT=3;
int 		g_DEFDATALEN=56;   // 56
char 		g_pingStr[64];
char		g_spingCnt[8];
char		g_spingSize[8];
char 		g_xPingString[500];
char 		g_ftpIp[20];
char 		g_ftpUser[128];
char 		g_ftpPasswd[128];
char 		g_ftpFile[256];
char			lEOIDString_m_back[100];
long 		ftpAdmin=1;
//long 		ntransmitted = 0, nreceived = 0, nrepeats = 0;
static long	tmin = LONG_MAX;	/* minimum round trip time */
static long	tmax = 0;		/* maximum round trip time */
static long	taverage = 0;
//static u_long	tsum;			/* sum of all times, for doing average */

unsigned long 	g_cpePppIfIndex = 0;
int 		g_pppTestAdmin = 0;


const char TOTALSIZE[] ="TotalSize:";
const char OPERSTATUS[] ="OperStatus:";
const char ELAPSETIME[] ="ElapseTime:";

extern unsigned long g_pppTestRseult;

#define		PINGINTERVAL	1	/* second */
#define 	MAXWAIT		5

#if 0
#define 	BUFFER_SIZE_MSG		256
struct data_to_pass_st {
	int	id;
	char data[BUFFER_SIZE_MSG];
};
#endif

// Added by Mason Yu for modify snmp agent
#ifdef CONFIG_USER_SNMPD_MODULE_RM 
extern SmpStatusType startRMTrap(int pvcsd, u_long lhostTrap_m, u_short lportTrap_m, ApsIdType communityId_m, CCharPtrType lEOIDString_m, int channel);
extern SmpStatusType startFTPTrap(int pvcsd, u_long lhostTrap_m, u_short lportTrap_m, ApsIdType communityId_m, CCharPtrType lEOIDString_m, int channel);
extern SmpStatusType startPingTrap(int pvcsd, u_long lhostTrap_m, u_short lportTrap_m, ApsIdType communityId_m, CCharPtrType lEOIDString_m, int channel);
extern SmpStatusType startPPPTrap(int pvcsd, u_long lhostTrap_m, u_short lportTrap_m, ApsIdType communityId_m, CCharPtrType lEOIDString_m, int channel);
#endif

#ifdef CONFIG_USER_SNMPD_MODULE_RM

int pppTesting(unsigned long ifIndex, int admin)
{
	struct data_to_pass_st msg;
	int i, selected;
	unsigned int ifMap;	// high half for PPP bitmap, low half for vc bitmap
	unsigned int totalEntry;
	//MIB_CE_ATM_VC_Tp pEntry, tmp_pEntry;
	MIB_CE_ATM_VC_T Entry, tmp_Entry;
	int drflag=0;	// Jenny, check if default route exists
	char tmp_vpi[6], tmp_vci[6];
	char vpi[6], vci[6];
	char ifname[6];
	int flags, strStatus;
	CONN_T conn_status;
	unsigned long pppAuth;
	char pppifname[6];
	
	
	selected = -1;
	ifMap = 0;
	
	totalEntry = mib_chain_total(MIB_ATM_VC_TBL); /* get chain record size */
	
	for (i=0; i<totalEntry; i++) {
		//pEntry = (MIB_CE_ATM_VC_Tp) mib_chain_get(MIB_ATM_VC_TBL, i); /* get the specified chain record */
		mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry); /* get the specified chain record */
		
		//if(pEntry == NULL)
		//{			
		//	return 0;
		//}		
		
		
		snprintf(tmp_vpi, 6, "%u", Entry.vpi);
		snprintf(tmp_vci, 6, "%u", Entry.vci);
		//printf("atoi(tmp_vpi)=%d\n", atoi(tmp_vpi) );
		//printf("atoi(tmp_vci)=%d\n", atoi(tmp_vci) );	
		
		//if ( (selected == -1) && ((atoi(tmp_vpi) == para->inputvpi) && (atoi(tmp_vci) == para->inputvci))  )
		if ( (selected == -1) && (ifIndex == Entry.cpePppIfIndex)  )
			selected = i;
		else
		{
			ifMap |= 1 << VC_INDEX(Entry.ifIndex);	// vc map
			ifMap |= (1 << 16) << PPP_INDEX(Entry.ifIndex);	// PPP map
		}

		if (Entry.cmode != CHANNEL_MODE_BRIDGE)
			if (Entry.dgw)
				drflag = 1;
	}
	
	
	if (selected == -1)
	{
		printf("pppTesting: Please select a VC channel!\n");
		return 0;
	}
	
	//pEntry = (MIB_CE_ATM_VC_Tp) mib_chain_get(MIB_ATM_VC_TBL, selected); /* get the specified chain record */
	mib_chain_get(MIB_ATM_VC_TBL, selected, (void *)&Entry); /* get the specified chain record */
	
	//snprintf(vpi, 6, "%u", Entry.vpi);
	//snprintf(vci, 6, "%u", Entry.vci);
	//printf("atoi(vpi)=%d\n", atoi(vpi) );
	//printf("atoi(vci)=%d\n", atoi(vci) );	
	
	/*
	if ( admin == 3 ) {
		//printf("Do PPP Test: down\n");		
		//snprintf(msg.data, BUFFER_SIZE_MSG, "spppctl down %u", PPP_INDEX(Entry.ifIndex));
		//write_to_pppd(&msg);
		
		//printf("Do PPP Test: up\n");
		snprintf(msg.data, BUFFER_SIZE_MSG, "spppctl pppoetest %u username %s password %s", PPP_INDEX(Entry.ifIndex), Entry.pppUsername, Entry.pppPassword);	
		write_to_pppd(&msg);	
		
	}else {
		//snprintf(msg.data, BUFFER_SIZE, "spppctl down %u", PPP_INDEX(Entry.ifIndex));
		printf("Stop PPPoE Test for Remote Management\n");	
	}
	*/		
	
	// Get the correct Entry for Retrieve AdminStatus and OperStatus
	sleep(10);  // sleep 10 secs to get the correct AdminStatus and OperStatus
	for (i=0; i<totalEntry; i++){
		//tmp_pEntry = (MIB_CE_ATM_VC_Tp) mib_chain_get(MIB_ATM_VC_TBL, i);
		mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&tmp_Entry);			
				
		if ( tmp_Entry.cpePppIfIndex != ifIndex  )
			continue;
		else if ( tmp_Entry.cpePppIfIndex == ifIndex  ){
			//pEntry = tmp_pEntry;	
			memcpy(&Entry, &tmp_Entry, sizeof(MIB_CE_ATM_VC_T));		
			break;
		}	
	
	}


	// Retrieve AdminStatus and OperStatus
	if (Entry.cmode == CHANNEL_MODE_PPPOE || Entry.cmode == CHANNEL_MODE_PPPOA){
		snprintf(ifname, 6, "ppp%u", PPP_INDEX(Entry.ifIndex));
		
		// set status flag
		if (Entry.enable == 0)
		{	
			strStatus = 2;		
			conn_status = CONN_DOWN;
		}
		else
		if (getInFlags( ifname, &flags) == 1)
		{
			if (flags & IFF_UP)			
			{
				strStatus = 1;
				conn_status = CONN_UP;
			}
			else
			{
#ifdef CONFIG_PPP
				if (find_ppp_from_conf(ifname))
				{
					strStatus = 1;
					conn_status = CONN_DOWN;
				}
				else
#endif
				{
					strStatus = 1;
					//conn_status = CONN_NOT_EXIST;
					conn_status = CONN_DOWN;
				}
			}
		}
		else
		{
			strStatus = 1;
			//conn_status = CONN_NOT_EXIST;
			conn_status = CONN_DOWN;
		}	
		
	}
	
	
	if ( conn_status == CONN_UP ) {
		g_pppTestRseult = 1;       // Testing Success		             
	}	            
	else {
		snprintf(pppifname, 6, "%u:", PPP_INDEX(Entry.ifIndex));
		pppAuth=RMPppRetrieveValueFromFile(pppifname, strlen(pppifname));
		
		if ( pppAuth == 3 ){
			g_pppTestRseult = 3;     // Testing AuthFail
		}else{
			g_pppTestRseult = 4;     // Testing Failure
		}	
	}
	
	return 1;
}	



int initCpePppIfIndex()
// Init cpePppIfIndex
{
	int i, entryNumPPP;
        unsigned long cnt_pppIfIndex;
        //MIB_CE_ATM_VC_Tp pEntry;
        MIB_CE_ATM_VC_T Entry;
        struct channel_conf_para para;
        char vpi[6], vci[6];
        
        
        cnt_pppIfIndex = 210000;
        
        entryNumPPP = mib_chain_total(MIB_ATM_VC_TBL);
        
        for (i=0; i<entryNumPPP; i++){
        	//pEntry = (MIB_CE_ATM_VC_Tp) mib_chain_get(MIB_ATM_VC_TBL, i);
        	mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry);
        	
        	if ( Entry.cmode == CHANNEL_MODE_PPPOE || Entry.cmode == CHANNEL_MODE_PPPOA ){	        		
        		snprintf(vpi, 6, "%u", Entry.vpi);
        		snprintf(vci, 6, "%u", Entry.vci);
        		
        		para.inputvpi=atoi(vpi);
        		para.inputvci=atoi(vci);
        		para.cmode=0;
        		para.admin=0;
        		para.natmode=0;
        		para.dhcpmode=0;        		
        		para.pppIfIndex=cnt_pppIfIndex;	
        		para.IpIndex=0;		
        		para.encap=100;
        		para.brmode=100;
        		
        		modifyChannelConf(&para);			
        		cnt_pppIfIndex++;			
        		//printf("***** pppInit: Find the PPPoE/PPPoA and set his cpePppIfIndex=0x%x!\n", Entry.cpePppIfIndex);			
        	}else {
        		snprintf(vpi, 6, "%u", Entry.vpi);
        		snprintf(vci, 6, "%u", Entry.vci);
        		
        		para.inputvpi=atoi(vpi);
        		para.inputvci=atoi(vci);
        		para.cmode=0;
        		para.admin=0;
        		para.natmode=0;
        		para.dhcpmode=0;
        		para.pppIfIndex=0xff;	
        		para.IpIndex=0;	
        		para.encap=100;	
        		para.brmode=100;
        		
        		modifyChannelConf(&para);	        		
        		//printf("***** pppInit: Not Find the PPPoE/PPPoA and set his cpePppIfIndex=0x%x!\n", Entry.cpePppIfIndex);
        		continue;
        	}	
        				
        }	
	
	
}


int initCpeIpIndex()
// Init cpeIpIndex
{
	int i, entryNum;
        unsigned long cnt_IpIndex;
        //MIB_CE_ATM_VC_Tp pEntry;
        MIB_CE_ATM_VC_T Entry;
        struct channel_conf_para para;
        char vpi[6], vci[6];
        
        
        cnt_IpIndex = 10001;
        
        entryNum = mib_chain_total(MIB_ATM_VC_TBL);
        
        for (i=0; i<entryNum; i++){
        	//pEntry = (MIB_CE_ATM_VC_Tp) mib_chain_get(MIB_ATM_VC_TBL, i);
        	mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry);
        	
        	if ( Entry.cmode != CHANNEL_MODE_BRIDGE ){	        		
        		snprintf(vpi, 6, "%u", Entry.vpi);
        		snprintf(vci, 6, "%u", Entry.vci);
        		
        		para.inputvpi=atoi(vpi);
        		para.inputvci=atoi(vci);
        		para.cmode=0;
        		para.admin=0;
        		para.natmode=0;
        		para.dhcpmode=0;
        		para.pppIfIndex=0;			
        		para.IpIndex=cnt_IpIndex;	
        		para.encap=100;	
        		para.brmode=100;	
        		
        		modifyChannelConf(&para);			
        		cnt_IpIndex++;			
        		//printf("***** pppInit: Find the MER/RT1483 and set his cpePppIfIndex=0x%x!\n", Entry.cpeIpIndex);			
        	}else {
        		snprintf(vpi, 6, "%u", Entry.vpi);
        		snprintf(vci, 6, "%u", Entry.vci);
        		
        		para.inputvpi=atoi(vpi);
        		para.inputvci=atoi(vci);
        		para.cmode=0;
        		para.admin=0;
        		para.natmode=0;
        		para.dhcpmode=0;
        		para.pppIfIndex=0;
        		para.IpIndex=0xff;	
        		para.encap=100;	
        		para.brmode=100;	
        		
        		modifyChannelConf(&para);	        		
        		//printf("***** pppInit: Not Find the MER/RT1483 and set his cpePppIfIndex=0x%x!\n", Entry.cpeIpIndex);
        		continue;
        	}	
        				
        }	
	
	
}


// Kaohj
// On doing ftpTest or ftp_upgrading, and pingTest, periodically signal to check status
static void check_rmStatus()
{
	FILE *fp;
	char temps[512];
	char *str;
	
	if (g_ftp_test_flag || g_ftp_upgrade_flag) {
		if ((fp = fopen("/var/ftpStatus.txt", "r")) == NULL)
		{
		}
		else {
			while (fgets(temps, 512, fp)) {
				if (str=strstr(temps, OPERSTATUS))
					g_ftp_operStatus = strtoul(str+strlen(OPERSTATUS)+1, 0, 10);
				else if (str=strstr(temps, TOTALSIZE))
					g_ftp_totalSize = g_ftp_doneSize = strtoul(str+strlen(TOTALSIZE)+1, 0, 10);
				else if (str=strstr(temps, ELAPSETIME))
					g_ftp_elapseTime = strtoul(str+strlen(ELAPSETIME)+1, 0, 10);
			}
			fclose(fp);
		}
		//printf("FtpOperStatus = %d\n", g_ftp_operStatus);
		signal(SIGALRM, check_rmStatus);
		if (g_ftp_operStatus == OPER_NORMAL || g_ftp_operStatus == OPER_DOWNLOADING ||
			g_ftp_operStatus == OPER_SAVING) { // in transient state
			alarm(2);
			return;
		}
		
		g_ftp_test_flag = g_ftp_upgrade_flag = 0;
		// send snmp trap
		//printf("send ftp Trap\n");
		if ( pvcFlag == ILMI_Channel )
			startFTPTrap(pvcsd, lhostTrap_m, lportTrap_m, communityId_m, lEOIDString_m_back, ILMI_Channel);  
		else if ( pvcFlag == EOC_Channel )
			startFTPTrap(eocsd, lhostTrap_m, lportTrap_m, communityId_m, lEOIDString_m_back, EOC_Channel);
		
		if (g_ftp_operStatus == OPER_UPGRADE_FAILURE) {
			sprintf(temps, "/tmp/%s", g_ftpFile);  			   					
			//printf("unlink %s\n", temps);
			unlink(temps);
		}
	}
	
	if (g_ping_flag) {
		if ((fp = fopen("/var/pingStatus.txt", "r")) != NULL) {
			fscanf(fp, "%d\n%d\n%d\n%d\n%d\n%d\n%d",
				&g_pingOperStatus, &ntransmitted,
				&nreceived, &nrepeats, &tmin, &tmax, &taverage);
			fclose(fp);
		}
		
		//printf("PingOperStatus = %d\n", g_pingOperStatus);
		sprintf(g_xPingString, "Aim ping Address is: %s ; "
			"sent = %d ; Received = %d ; Lost = %d ; "
			"Min = %lu.%lu ms ; Max = %lu.%lu ms ; "
			"Average = %lu.%lu ms",
                	g_pingStr, ntransmitted, nreceived,
                	ntransmitted-nreceived, tmin / 10, tmin % 10,
                	tmax / 10, tmax % 10, taverage / 10, taverage % 10);
                
		if (g_pingOperStatus == 1 || g_pingOperStatus == 2) { // normal or pinging
			signal(SIGALRM, check_rmStatus);
			alarm(2);
			return;
		}
		
		g_ping_flag = 0;
                if ( pvcFlag == ILMI_Channel ){
                	//printf("Send Ping Trap via ILMI\n");
                	startPingTrap(pvcsd, lhostTrap_m, lportTrap_m, communityId_m, lEOIDString_m_back, ILMI_Channel);
                }else if ( pvcFlag == EOC_Channel ){
                	//printf("Send Ping Trap via EOC\n");
                	startPingTrap(eocsd, lhostTrap_m, lportTrap_m, communityId_m, lEOIDString_m_back, EOC_Channel);
                }
	}
}

/* common routines */
static int create_icmp_socket(void)
{
	struct protoent *proto;
	int sock;

	proto = getprotobyname("icmp");
	/* if getprotobyname failed, just silently force
	 * proto->p_proto to have the correct value for "icmp" */
	if ((sock = socket(AF_INET, SOCK_RAW,
			(proto ? proto->p_proto : 1))) < 0) {        /* 1 == ICMP */
		printf("cannot create raw socket\n");
	}

	/* drop root privs if running setuid */
//	setuid(getuid());

	return sock;
}


// Kaohj
void pingTest()
{
	FILE *fp;

	// init pingStatus
	if ((fp = fopen("/var/pingStatus.txt", "w")) == NULL)
	{
		printf("***** Open file /var/pingStatus.txt failed !\n");			
	}
	else {
		fprintf(fp, "2\n0\n0\n0\n0\n0\n0\n");
		fclose(fp);
	}
	
	g_pingOperStatus = 2; // set to pinging
	snprintf(g_spingCnt, 8, "%u", g_PINGCOUNT);
	snprintf(g_spingSize, 8, "%u", g_DEFDATALEN);
	va_cmd("/bin/rmping", 5, 0, "-c", g_spingCnt, "-s", g_spingSize, g_pingStr);
}

int ftpGet()
{
	FILE *fp;

	// Kaohj
	// init ftpStatus
	if ((fp = fopen("/var/ftpStatus.txt", "w")) == NULL)
	{
		printf("***** Open file /var/ftpStatus.txt failed !\n");			
	}
	else {
		fprintf(fp, "OperStatus: %d\n", OPER_NORMAL);    // normal
		fprintf(fp, "TotalSize: 0\n" );
		fprintf(fp, "ElapseTime: 0\n");	
		fclose(fp);
	}
	
	if ((fp = fopen("/var/ftpget.txt", "w")) == NULL)
	{
		printf("***** Open file /var/ftpget.txt failed !\n");
		return 0;
	}
	
	fprintf(fp, "open %s\n", g_ftpIp);
	fprintf(fp, "user %s %s\n", g_ftpUser, g_ftpPasswd);
	fprintf(fp, "lcd /tmp\n");
	fprintf(fp, "bin\n");
	if (g_ftp_test_flag)
		fprintf(fp, "get %s %s\n", g_ftpFile, "/dev/null");
	else
		fprintf(fp, "get %s\n", g_ftpFile);
	fprintf(fp, "bye\n");
	fprintf(fp, "quit\n");
	fclose(fp);
	
	//system("/bin/ftp -inv < /var/ftpget.txt");
	va_cmd("/bin/ftp", 3, 0, "-inv", "-f", "/var/ftpget.txt");
	
	return 1;
}	

#if 0
#include "msgq.h"
#define CONF_SERVER_PIDFILE	"/var/run/boa.pid"
#define UPLOAD_PREFIX		"uc"
void sendMessage(const char *buf)
{
	key_t key;
	int   qid, cpid, spid;
   	struct mymsgbuf qbuf;
	FILE *spidfile;
   	//char buf[64];

	/* Create unique key via call to ftok() */
	key = ftok("/bin/init", 'k');
	if ((qid = open_queue(key, MQ_GET)) == -1) {
		perror("open_queue");
  	//fprintf(myfp, "sendMessage: open_queue qid=%d\n", qid);
 	 //fflush(myfp);
		return;
	}
	
	// get client pid
	cpid = (int)getpid();
	
	// get server pid
	if ((spidfile = fopen(CONF_SERVER_PIDFILE, "r"))) {
		fscanf(spidfile, "%d\n", &spid);
		fclose(spidfile);
	}
	else
		printf("server pidfile not exists\n");

  	 //fprintf(myfp, "sendMessage: buf=%s\n", buf);
  	 //fflush(myfp);
  	 send_message(qid, spid, cpid, buf);
  	 kill( spid, SIGUSR2);
		read_message(qid, &qbuf, cpid);
   	//fprintf(myfp, "sendMessage: qbuf.request=%d MSG_FAIL=%d\n", qbuf.request, MSG_FAIL);
   	//fflush(myfp);
	if (qbuf.request == MSG_FAIL)
		printf("reload request failed\n");
}



int upgradeImage()
{
	
	char buf[64];
	unsigned long value;
	
	value=RMFtpRetrieveValueFromFile(TOTALSIZE, strlen(TOTALSIZE)+1);
   	
   	sprintf(buf, "upload /tmp/%s %d", g_ftpFile, value);
   	//fprintf(myfp, "ftpUpdateImage: name=%s len=%d\n", name, len);
   	//fprintf(myfp, "ftpUpdateImage: buf=%s\n", buf);
   	//fflush(myfp);
   	sendMessage( buf);
   		
}
#endif	
#endif


// Added by Mason Yu for WLAN SNMP MIB
int do_cmd(const char *filename, char *argv [], int dowait)
{
	pid_t pid, wpid;
	int stat, st;
		
	
	if((pid = vfork()) == 0) {
		/* the child */
		char *env[3];
		
		signal(SIGINT, SIG_IGN);
		argv[0] = (char *)filename;
		env[0] = "PATH=/bin:/usr/bin:/etc:/sbin:/usr/sbin";
		env[1] = NULL;

		execve(filename, argv, env);

		printf("exec %s failed\n", filename);
		_exit(2);
	} else if(pid > 0) {
		if (!dowait)
			stat = 0;
		else {
			/* parent, wait till rc process dies before spawning */
			while ((wpid = wait(&stat)) != pid)
				if (wpid == -1 && errno == ECHILD) { /* see wait(2) manpage */
					stat = 0;
					break;
				}
		}
	} else if(pid < 0) {
		printf("fork of %s failed\n", filename);
		stat = -1;
	}
	
	//st = WEXITSTATUS(stat);
	//return st;
	return stat;
}




static void done(int sig)
{
  exit(0);
}

// Kaohj
/* suspend execution of the current process until a child has exited.
	Any system resources used by the child are freed.
*/
static void clear_child(int i)
{
	int status;
	wait( &status );
	return;
}

static	void	cmdInit (void)
{
	aslInit ();
	asnInit ();
	misInit ();
	avlInit ();
	mixInit ();
	apsInit ();
	ap0Init ();
	smpInit ();

#ifdef CONFIG_USER_SNMPD_MODULE_SYSTEM
	systmInit ();
#endif
#ifdef CONFIG_USER_SNMPD_MODULE_IP
	ipInit ();
#endif
#ifdef CONFIG_USER_SNMPD_MODULE_IFACE
	ifaceInit ();
#endif
#ifdef CONFIG_USER_SNMPD_MODULE_TCP
	tcpInit ();
#endif
#ifdef CONFIG_USER_SNMPD_MODULE_ICMP
	icmpInit ();
#endif
#ifdef CONFIG_USER_SNMPD_MODULE_UDP
	udpInit ();
#endif

#ifdef CONFIG_USER_SNMPD_MODULE_ADSL
	adslInit ();
	adsl2Init ();
#endif

#ifdef CONFIG_USER_SNMPD_MODULE_ATM
	//printf("***** Init atm MIB\n");
	//atmInit ();
#endif


#ifdef CONFIG_USER_SNMPD_MODULE_FRAMEWORK
	frameworkInit ();
#endif

// Added by Mason Yu
#ifdef CONFIG_USER_SNMPD_MODULE_WLAN
	//printf("Init WLAN MIB\n");
	wlanInit();
#endif
 
#ifdef CONFIG_RTL8686_SWITCH
	nscrtInit();
#endif
      
// Added by Mason Yu
#ifdef CONFIG_USER_SNMPD_MODULE_RM
	//printf("***** Init RM_sysinfo MIB\n");
	sysinfoInit();
	RMsystemInit();
	atmvclInit();
	pvcInit();
	pppInit();
	pppssInit();
	rmipInit();
	rmdhcpInit();
	rmdnsInit();
	RMpingInit();
	RMftpInit();
	rmadslLineInit();
#endif      
       
}

//ex: snmpd -th 192.168.1.100 -te 1.3.6.1.4.1.3.1.1.0 
static  CIntfType       usage (CCharPtrType s)
{
        fprintf (stderr, "Usage: %s", s);
        fprintf (stderr, " [-p SNMP port]");
        fprintf (stderr, " [-c community]");
        fprintf (stderr, " [-th trap dest host ip]\n");
        fprintf (stderr, " [-tp trap dest host port]\n");
        fprintf (stderr, " [-te trap enterprise-oid]\n");
        return (1);
}

SmpStatusType	myUpCall (SmpIdType smp, SmpRequestPtrType req)
{
	smp = smp;
	req = req;
	printf ("Upcall:\n");
	return (errOk);
}


extern CCharPtrType defaultEOID;
extern SmpStatusType startTrap(int sTrap_m, u_long lhostTrap_m, u_short lportTrap_m, ApsIdType communityId_m, CCharPtrType lEOIDString_m, int cnt);
extern int getIP(char *devName, int *pIp);

int			snmpdCommand (int argc, char **argv)
{
	int			s;
	//int			sTrap;
	int			salen;
	int			result;
	struct	sockaddr	salocal;
	struct	sockaddr	saremote;
	struct	sockaddr_in	*sin;
	struct	servent		*svp;

        u_long                  lhost;
        u_short                 lport;
        //u_long                  lhostTrap;
        //u_short                 lportTrap;

	CByteType		buf [ cmdBufferSize ];
	CBytePtrType		bp;
	SmpIdType		smp;
	SmpSocketType		udp;
	// Added by Mason Yu for return the correct comm string
	//ApsIdType		communityId;
	ApsIdType		communityIdCur;
        CCharPtrType            *ap;
        CCharPtrType            cp;
        CBoolType               noerror;
	CUnslType		number;

        CCharPtrType            communityString;
        CCharPtrType            lhostString;
        CCharPtrType            lportString;
        CCharPtrType            lhostTrapString;
        CCharPtrType            lportTrapString;
        CCharPtrType            lversionString;
        //CCharPtrType            lEOIDString;
        int                     lversion;
        int                     i;
        SmpStatusType           status;
        SmpRequestType          req;
        SmpErrorType            error;

	communityString = (CCharPtrType) 0;
	lhostString = (CCharPtrType) 0;
	lportString = (CCharPtrType) 0;
	lhostTrapString = (CCharPtrType) 0;
	lportTrapString = (CCharPtrType) 0;
        lversionString = (CCharPtrType) 0;
        lEOIDString_m = (CCharPtrType) 0;
        
	// Added by Mason Yu for modify snmp agent
	// Kaohj
	//int pvcsd, pvcTrapsd, eocsd;
	int err;
	struct sockaddr_atmpvc pvcaddr;
	// Added by Mason Yu for Limit End user to change configuration state
	// pvcFlag = ETH_Channel, it means the packet come from ethernet
	// pvcFlag = ILMI_Channel, it means the packet come from pvc(ILMI(0/16))
	// pvcFlag = EOC_Channel, it means the packet come from EOC
	// Kaohj
	//int pvcFlag;  
	// Added by Mason Yu for return the correct comm string 
	FILE 			*fp;
	struct itimerval repeat_timer;
	struct timeval tv;
	
	btCnt = 1;
	trapCnt = 1;  
	
        if (argc<3)
                return ((int) usage ((CCharPtrType) argv [ 0 ]));
        
	ap = (CCharPtrType *) argv + 1;
	argc--;
	noerror = TRUE;
	while ((argc != 0) && (**ap == (CCharType) '-') && (noerror)) {
		cp = *ap;
		cp++;
		ap++;
		argc--;
		while ((*cp != (CCharType) 0) && (noerror)) {
			switch (*cp) {

			case 'c':  //comunity
				argc--;
				communityString = *ap++;
				break;

			case 'h':  //SNMP host
				argc--;
				lhostString = *ap++;
				break;
			

			case 'p':  //SNMP port
				argc--;
				lportString = *ap++;
				break;

                        /*case 'v':  //SNMP version
                                argc--;
                                lversionString = *ap++;
                                break;*/

                        case 't':  //SNMP Trap argument
                            cp++;
                            switch (*cp) {

                                case 'h':  //Trap dst host ip
                                        argc--;
                                        lhostTrapString = *ap++;
                                        break;
                                
                                case 'p':  //Trap dst port
                                        argc--;
                                        lportTrapString = *ap++;
                                        break;
                                
                                case 'e':  //Trap enterprise OID
                                        argc--;
                                        lEOIDString_m = *ap++;
                                        break;
                                
                                default:
                                        noerror = FALSE;
                                        break;
                            };
                            break;

			default:
				noerror = FALSE;
				break;
			}
			cp++;
		}
	}

	// Mason Yu
	strcpy(lEOIDString_m_back, lEOIDString_m);
	sprintf(lEOIDString_m_back, "%s.6", lEOIDString_m_back);
	
	if ((! noerror) || (argc > 0)) {
		return ((int) usage ((CCharPtrType) argv [ 0 ]));
	}

        if (lhostString != (CCharPtrType) 0) {
                lhost = (u_long) hostAddress (lhostString);
                if (lhost == (u_long) -1) {
                        fprintf (stderr, "%s: Bad foreign host: %s\n",
                                argv [ 0 ], lhostString);
                        return (2);
                }
        }
        else {
                lhost = (u_long) 0;
        }

	if (lhostTrapString != (CCharPtrType) 0) {
		lhostTrap_m = (u_long) hostAddress (lhostTrapString);
		if (lhostTrap_m == (u_long) -1) {
			fprintf (stderr, "%s: Bad Trap destination host IP address: %s\n",
				argv [ 0 ], lhostTrapString);
			return (2);
		}
	}
	else {
		lhostTrap_m = (u_long) 0;
		fprintf (stderr, "%s: No Trap destination host IP address.\n",
				argv [ 0 ]);
		return (2);
	}

	if (lportString != (CCharPtrType) 0) {
                if (rdxDecodeAny (& number, lportString) < (CIntfType) 0) {
                        fprintf (stderr, "%s: Bad local port: %s\n",
                                argv [ 0 ], lportString);
                        return (2);
                }
                else {
                        lport = number;
                }
        }
        else 
		{
                lport = 161;
        	}

        if (lportTrapString != (CCharPtrType) 0) {
                if (rdxDecodeAny (& number, lportTrapString) < (CIntfType) 0) {
                        fprintf (stderr, "%s: Bad Trap port: %s\n",
                                argv [ 0 ], lportTrapString);
                        return (2);
                }
                else {
                        lportTrap_m = number;
                }
        }
        else 
                {
                lportTrap_m = 162;  //default trap
                }

	if (communityString == (CCharPtrType) 0) {
		communityString = (CCharPtrType) "public";
	}

        if (lEOIDString_m == (CCharPtrType) 0) {
                lEOIDString_m = (CCharPtrType) defaultEOID;
        }
        
	cmdInit ();

	s = socket (AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		(void) perror ("socket");
		return (1);
	}
	
	sTrap_m = socket (AF_INET, SOCK_DGRAM, 0);
	if (sTrap_m < 0) {
		(void) perror ("Trap socket");
		return (1);
	}

	//SNMP port
	sin = (struct sockaddr_in *) & salocal;
        bzero ((char *) sin, sizeof (salocal));
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = lhost;
	sin->sin_port = htons(lport);

	result = bind (s, & salocal, sizeof (*sin));
	if (result < 0) {
		(void) perror ("bind");
		return (1);
	}

#ifdef CONFIG_DEV_xDSL	
	// Added by Mason Yu for modify snmp agent. Start
	// Create ATM socket for snmp agent on PVC channel
	pvcsd = socket(PF_ATMPVC, SOCK_DGRAM, ATM_AAL5);
	if (pvcsd < 0) {
		printf("Create ATM Socket fail for PVC\n");
		return (1);
	}	
	
	pvcaddr.sap_family = AF_ATMPVC;
    	pvcaddr.sap_addr.itf = -1;
    	pvcaddr.sap_addr.vpi = 0;
    	pvcaddr.sap_addr.vci = 16;  	

   
    	// Connect to ATM socket for snmp agent
    	// Mason Yu for multi pvc

    	err = connect(pvcsd, (struct sockaddr*)&pvcaddr, sizeof(pvcaddr));
    	if (err < 0) {
		printf("failed to connect on ATM socket\n");    
		return -1;
	}
	// Added by Mason Yu for modify snmp agent. End
#endif

#ifdef CONFIG_USER_SNMPD_MODULE_RM	
	// Add EOC channle
	eocsd = socket(PF_DSLEOC, SOCK_DGRAM, 0);
 	if (eocsd < 0) {
  		printf("Create ATM Socket fail for EoC\n");
  		return (1);
 	}
#endif


	communityId_m = apsNew ((ApsNameType) communityString,
		(ApsNameType) "trivial", (ApsGoodiesType) 0);


#ifndef CONFIG_USER_SNMPD_MODULE_RM
#if 0
	//start trap monitor
	printf("Send Trap (1)\n");
	if (warmStartTrap(sTrap_m, lhostTrap_m, lportTrap_m, communityId_m, lEOIDString_m_back,  trapCnt )==errBad) {
                (void) perror ("Trap Start");
                return (1);
	};
#endif
#endif

	// Added by Mason Yu for return the correct comm string, Start		
	if ((fp = fopen("/var/snmpComStr.conf", "r")) == NULL) {
		printf("Open file /var/snmpComStr.conf failed !\n");
		return -1;
	}
	
	fscanf(fp, "readStr %s\n", readComm);
	fscanf(fp, "writeStr %s\n", writeComm);
	fscanf(fp, "PvcReadStr %s\n", pvcReadComm);
	fscanf(fp, "PvcWriteStr %s\n", pvcWriteComm);	
	fclose(fp);
	
	if ( strcmp(readComm, writeComm) == 0 )	{
		communityIdCur = apsNew ((ApsNameType) readComm,
			(ApsNameType) "trivial", (ApsGoodiesType) 0);
	}else {
		communityIdCur = apsNew ((ApsNameType) readComm,
			(ApsNameType) "trivial", (ApsGoodiesType) 0);
		
		communityIdCur = apsNew ((ApsNameType) writeComm,
			(ApsNameType) "trivial", (ApsGoodiesType) 0);	
	}			
		
	if ( strcmp(pvcReadComm, pvcWriteComm) != 0 ) {
		communityIdCur = apsNew ((ApsNameType) pvcWriteComm,
			(ApsNameType) "trivial", (ApsGoodiesType) 0);
	}		
	// Added by Mason Yu for return the correct comm string, End
	
#ifdef CONFIG_USER_SNMPD_MODULE_RM	
	// Added by Mason Yu for create pthread for save/reboot
	// Init MIB 
	//if ( mib_init() == 0 ) {
	//	printf("[snmpd]***** Initialize MIB failed!\n");
	//}
	
	
	// Init cpePppIfIndex
	initCpePppIfIndex();	
	
	// Init cpeIpIndex
	initCpeIpIndex();
	
	// Init Ping Address
	memset(g_pingStr, 0, sizeof(g_pingStr));
	strcpy(g_pingStr, "10.1.1.1");
	
	
	// Init Ping Result
	g_xPingString[0]=0;
	
	// Init Ftp Server IP
	memset(g_ftpIp, 0, sizeof(g_ftpIp));
	strcpy(g_ftpIp, "172.21.70.22");
	
	// Init FTP User
	memset(g_ftpUser, 0, sizeof(g_ftpUser));
	strcpy(g_ftpUser, "pti");
	
	// Init FTP Password
	memset(g_ftpPasswd, 0, sizeof(g_ftpPasswd));
	strcpy(g_ftpPasswd, "pti123");
	
	// Init FTP File name
	memset(g_ftpFile, 0, sizeof(g_ftpFile));
	strcpy(g_ftpFile, "vm.img");
	
#endif	
	
	sin = (struct sockaddr_in *) & saremote;
	
	// Test SendTrap
	// sleep(30);	
	//warmStartTrap(sTrap_m, lhostTrap_m, lportTrap_m, communityId_m, lEOIDString_m_back,  trapCnt );
	
	while(1)	
	{	
		fd_set fd;			
		int select_s;
		struct timeval now;
		Modem_LinkSpeed vLs;
		vLs.upstreamRate=0;
		
		
		FD_ZERO(&fd);
		FD_SET(s, &fd);
		
#ifdef CONFIG_DEV_xDSL
		FD_SET(pvcsd, &fd);
#endif

#ifdef CONFIG_USER_SNMPD_MODULE_RM		
		FD_SET(eocsd, &fd);			
#endif


#ifdef CONFIG_USER_SNMPD_MODULE_RM
		select_s = eocsd + 1;
#elif defined(CONFIG_DEV_xDSL)
		select_s = pvcsd + 1;
#else
		select_s = s + 1;
#endif

			
		// Check if we should send BootTrap again for Remote Management. Mason Yu
#ifdef CONFIG_USER_SNMPD_MODULE_RM
		// wait until showtime
		
		if (adsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&vLs,
			RLCM_GET_LINK_SPEED_SIZE) && vLs.upstreamRate != 0) {				
		
			//printf("ADSL is showtime\n");
			if ( btCnt <=3 && g_snmpBootTrapResponse == BTRAP_NO_RESPONSE ) {
				gettimeofday(&now, 0);
				
				if ( btrap_last_scan == 0)
					btrap_last_scan = now.tv_sec;
					
				//printf("now.tv_sec=%d  btrap_last_scan=%d\n", now.tv_sec, btrap_last_scan);			
				if((now.tv_sec - btrap_last_scan) >= 5 || btCnt == 1 ){	//  5 sec
					btrap_last_scan = now.tv_sec;					
					
					sleep(3);	
        				startTrap(sTrap_m, lhostTrap_m, lportTrap_m, communityId_m, lEOIDString_m_back, trapCnt);
        				startRMTrap(pvcsd,lhostTrap_m, lportTrap_m, communityId_m, lEOIDString_m_back, ILMI_Channel);
        				startRMTrap(eocsd,lhostTrap_m, lportTrap_m, communityId_m, lEOIDString_m_back, EOC_Channel);
        				btCnt++;
				}
				
			}
#endif		
	
			// Mason Yu
			// In order to save CPU resource, 
			// if Auto Configuration in not finished for Remote Management, the select() will blocks for 5 sec.
			// if Auto Configuration is finished for Remote Management, the select() will blocks perpetually.
#ifdef CONFIG_USER_SNMPD_MODULE_RM		
			if ( btCnt > 3 || g_snmpBootTrapResponse != BTRAP_NO_RESPONSE) {
				// Kaohj
				//select(select_s, &fd, NULL, NULL, NULL);	
				if (select(select_s, &fd, NULL, NULL, NULL) == -1) {
					if (errno == EINTR || errno == EBADF)
						continue;	/* while(1) */
					else
						exit(0);
				}
			}
			else 	
			{
				tv.tv_sec = 5;       // 5 sec
	  			tv.tv_usec = 0;
	  			//printf("Set timer(20)\n");
	  			// Kaohj
				//select(select_s, &fd, NULL, NULL, &tv);	
				if (select(select_s, &fd, NULL, NULL, &tv) == -1) {
					if (errno == EINTR || errno == EBADF)
						continue;	/* while(1) */
					else
						exit(0);
				}
			}
		
		}else {
			tv.tv_sec = 1;       //  1 sec
			tv.tv_usec = 0;
	  		//printf("Set timer(1)\n");
	  		// Kaohj
			//select(select_s, &fd, NULL, NULL, &tv);	
			if (select(select_s, &fd, NULL, NULL, &tv) == -1) {
				if (errno == EINTR || errno == EBADF)
					continue;	/* while(1) */
				else
					exit(0);
			}
		}		
		
#else
		// Kaohj
		//select(select_s, &fd, NULL, NULL, NULL);
		if (select(select_s, &fd, NULL, NULL, NULL) == -1) {
				if (errno == EINTR || errno == EBADF)
					continue;	/* while(1) */
				else
					exit(0);
		}
#endif

		
		if (FD_ISSET(s, &fd)) {
			salen = sizeof (saremote);
			result = recvfrom (s, (char *) buf, (int) cmdBufferSize,
				(int) 0, & saremote, & salen);			
			
#ifdef TEST_PVC_LOOPBACK
			printf("Send to ILMI/EOC for Testing\n");			
			//write(pvcsd, (char *) buf, result);
			write(eocsd, (char *) buf, result);
#endif


#ifndef TEST_PVC_LOOPBACK				
			DEBUG1 ("Recvfrom: %d\n", result);
			DEBUGBYTES (buf, result);
			DEBUG0 ("\n");			
		
			udp = udpNew (s, sin->sin_addr.s_addr, sin->sin_port);
			smp = smpNew (udp, udpSend, myUpCall);	
			// Added by Mason Yu for Limit End user to change configuration state
			pvcFlag = ETH_Channel;			
			
			for (bp = buf; ((result > 0) &&
				// Modified by Mason Yu for Limit End user to change configuration state
				//(smpInput (smp, *bp++) == errOk));
				(smpInput (smp, *bp++, pvcFlag) == errOk));
				result--);

			smp = smpFree (smp);
			udp = udpFree (udp);			
#endif	

			
		}


#ifdef CONFIG_DEV_xDSL		
		if (FD_ISSET(pvcsd, &fd)) {
			salen = sizeof (saremote);
			result = read (pvcsd, (char *) buf, cmdBufferSize);	
				
			DEBUG1 ("Recvfrom: %d\n", result);
			DEBUGBYTES (buf, result);
			DEBUG0 ("\n");			

#ifndef TEST_PVC_LOOPBACK
			// Mason Yu
			// Although PVC channel not need the IP address and Port No.
			// But we still hold them here. Because the pvcSend() do not use them later. 			
			sin->sin_addr.s_addr = 0x1010101;
			sin->sin_port = 1050;	
#endif					


#ifdef TEST_PVC_LOOPBACK						
			udp = udpNew (s, sin->sin_addr.s_addr, sin->sin_port);
			smp = smpNew (udp, udpSend, myUpCall);
#else
			udp = udpNew (pvcsd, sin->sin_addr.s_addr, sin->sin_port);
			smp = smpNew (udp, pvcSend, myUpCall);
#endif						
			//smp = smpNew (udp, udpSend, myUpCall);				
			// Added by Mason Yu for Limit End user to change configuration state
			pvcFlag = ILMI_Channel;
			
			for (bp = buf; ((result > 0) &&
				// Modified by Mason Yu for Limit End user to change configuration state
				//(smpInput (smp, *bp++) == errOk));
				(smpInput (smp, *bp++, pvcFlag) == errOk));
				result--);

			smp = smpFree (smp);
			udp = udpFree (udp);			
				
		}
#endif

#ifdef CONFIG_USER_SNMPD_MODULE_RM		
		if (FD_ISSET(eocsd, &fd)) {
			salen = sizeof (saremote);
			result = read (eocsd, (char *) buf, cmdBufferSize);	
			//result = recv(eocsd, (char *) buf, cmdBufferSize, MSG_DONTWAIT);			
			
			DEBUG1 ("Recvfrom: %d\n", result);
			DEBUGBYTES (buf, result);
			DEBUG0 ("\n");			

#ifndef TEST_PVC_LOOPBACK
			// Mason Yu
			// Although EOC channel not need the IP address and Port No.
			// But we still hold them here. Because the pvcSend() do not use them later. 			
			sin->sin_addr.s_addr = 0x1010101;
			sin->sin_port = 1050;	
#endif					


#ifdef TEST_PVC_LOOPBACK						
			udp = udpNew (s, sin->sin_addr.s_addr, sin->sin_port);
			smp = smpNew (udp, udpSend, myUpCall);
#else
			udp = udpNew (eocsd, sin->sin_addr.s_addr, sin->sin_port);
			smp = smpNew (udp, eocSend, myUpCall);
#endif						
			//smp = smpNew (udp, udpSend, myUpCall);				
			// Added by Mason Yu for Limit End user to change configuration state
			pvcFlag = EOC_Channel;
			
			for (bp = buf; ((result > 0) &&
				// Modified by Mason Yu for Limit End user to change configuration state
				//(smpInput (smp, *bp++) == errOk));
				(smpInput (smp, *bp++, pvcFlag) == errOk));
				result--);

			smp = smpFree (smp);
			udp = udpFree (udp);			
				
		}
	

	
		// Reboot system for Remote management
		if ( g_reboot_flag == 1) {			
			/* reboot the system */
			sync();
			va_cmd("/bin/ifconfig", 2, 1, "eth0", "down");
			va_cmd("/bin/ifconfig", 2, 1, "wlan0", "down");
			va_cmd("/bin/sarctl",1,1,"disable");
			va_cmd("/bin/reboot", 0, 1);
		}
		
		
		// Save configuration to flash for RM
		if ( g_save_flag == 1) {			
			mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);			
		}		
		
		
		// Restore to default
		if ( g_reboot2def_flag == 1) {
			//printf("snmpd: Reset to default\n");	
			
			// Commented by Mason Yu. for not use default setting		
			// load Default setting from flash to RAM
			//mib_load(DEFAULT_SETTING, CONFIG_MIB_ALL);						
			
			// Update Default setting from RAM to flash
			//mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
			
			//va_cmd("/bin/flash", 2, 1, "default", "cs");
			reset_cs_to_default(1);
			g_reboot2def_flag = 0;
			
			// Mason Yu on true
			// True require need to reboot system
			sync();
			va_cmd("/bin/ifconfig", 2, 1, "eth0", "down");
			va_cmd("/bin/ifconfig", 2, 1, "wlan0", "down");
			va_cmd("/bin/sarctl",1,1,"disable");
			va_cmd("/bin/reboot", 0, 1);
		}
		
		
		// Do ping Test
		// "Aim ping Address is: %s\n sent = %d\n Received = %d\n Lost = %d\n Min = 0ms\n Max = 0ms\n Average = 0ms\n"
		if ( g_ping_flag == 1 ) {
			//printf("snmpd: do Ping Test\n");	
			sleep(3);
			// Kaohj
			pingTest();
			g_ping_flag = 2; // doing ping test
			// keep checking ping status
			check_rmStatus();
		}
		
		// Do FTP upgrade Image
		if ( g_ftp_upgrade_flag == 1 ) {
			char buf[64], pid[10];				
			FILE *fp;
			
			//printf("snmpd: do ftp upgrade\n");
						
#ifdef CONFIG_8M_SDRAM
			// todo: what if ftp download fail
			cmd_killproc(ALL_PID & ~(1<<PID_SNMPD));
#endif
			//va_cmd_no_echo("/bin/watchdog", 1, 1, "off");
   			sprintf(buf, "/tmp/%s", g_ftpFile);  			   					
									
			ftpGet();
			g_ftp_upgrade_flag = 2; // doing upgrade
			// Kaohj
			// keep checking ftp status
			check_rmStatus();
			// upgrading is done by ftp client
		}
		
		// Do FTP Test
		if ( g_ftp_test_flag == 1 ) {
			//printf("snmpd: do FTP Test\n");	
			sleep(3);					
			ftpGet();
			g_ftp_test_flag = 2; // doing ftp test
			// Kaohj
			check_rmStatus();
			#if 0
			
			if ( pvcFlag == ILMI_Channel )
				startFTPTrap(pvcsd, lhostTrap_m, lportTrap_m, communityId_m, lEOIDString_m_back, ILMI_Channel);  
			else if ( pvcFlag == EOC_Channel )
				startFTPTrap(eocsd, lhostTrap_m, lportTrap_m, communityId_m, lEOIDString_m_back, EOC_Channel);
			#endif
		}
		
		// Do PPP Test
		if ( g_pppTestFlag == 1 ) {
			sleep(10);
			//printf("snmpd: do PPP Test\n");	
			pppTesting(g_cpePppIfIndex, g_pppTestAdmin);	
			
			/*
			if ( pvcFlag == ILMI_Channel )		  
				startPPPTrap(pvcsd, lhostTrap_m, lportTrap_m, communityId_m, lEOIDString_m_back, ILMI_Channel); 
			else if ( pvcFlag == EOC_Channel )
				startPPPTrap(eocsd, lhostTrap_m, lportTrap_m, communityId_m, lEOIDString_m_back, EOC_Channel); 
			*/
						            	
			g_pppTestFlag = 0;
		}
			
#endif		


	} 


	(void) perror ("recv");
	communityId_m = apsFree (communityId_m);
	
	// Added by Mason Yu for return comm string dynamic
	communityIdCur = apsFree (communityIdCur);
	
	return (close (s));
}




//extern void trapOccur(int sig);
void trapOccur(int sig)
{
	if (warmStartTrap(sTrap_m, lhostTrap_m, lportTrap_m, communityId_m, lEOIDString_m_back,  trapCnt )==errBad) {
                (void) perror ("Warm Trap Start");
                return (1);
	}
	trapCnt++;
	
	return;
}

int	main (int argc, char **argv)
{
  FILE *pidfile;

  /* signal handler */
  signal(SIGINT, done);
  signal(SIGTERM, done);
  signal(SIGHUP, done);
  
  signal(SIGUSR1, trapOccur);  //for trap 
	// Kaohj
	/*when those processes created by startup are killed,
	  they will be zombie processes,jiunming*/
	signal(  SIGCHLD, clear_child);

	/* write pidfile */
	if ((pidfile = fopen(RUNFILE, "w"))) {
		fprintf(pidfile, "%d\n", (int) getpid());
		fclose(pidfile);
	}
	
	exit (snmpdCommand (argc, argv));
}

