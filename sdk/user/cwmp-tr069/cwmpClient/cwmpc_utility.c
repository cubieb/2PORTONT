#include <stdlib.h>
#include <string.h>

#ifdef CONFIG_DEV_xDSL
#include <linux/atm.h>
#include <linux/atmdev.h>
#include <rtk/adsl_drv.h>
#endif

#include <sys/ioctl.h>
#include <rtk/utility.h>
#include "cwmpc_utility.h"

/***** Common Utilities ******************************************************/
/*copy from mib.c, because it defines with "static" */
int is_hex(char c)
{
    return (((c >= '0') && (c <= '9')) ||
            ((c >= 'A') && (c <= 'F')) ||
            ((c >= 'a') && (c <= 'f')));
}

int string_to_hex(char *string, unsigned char *key, int len)
{
	char tmpBuf[4];
	int idx, ii=0;
	for (idx=0; idx<len; idx+=2) {
		tmpBuf[0] = string[idx];
		tmpBuf[1] = string[idx+1];
		tmpBuf[2] = 0;
		if ( !is_hex(tmpBuf[0]) || !is_hex(tmpBuf[1]))
			return 0;

		key[ii++] = (unsigned char) strtol(tmpBuf, (char**)NULL, 16);
	}
	return 1;
}

int get_wan_info_tr181(int num, MIB_CE_ATM_VC_T *pEntry, int *id, char *ifname)
{
	int total_wan = mib_chain_total( MIB_ATM_VC_TBL );
	int i;

	if(pEntry == NULL || ifname == NULL)
		return 0;

	for( i = 0 ; i < total_wan ; i++ )
	{
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)pEntry ))
			continue;

		if(pEntry->ConDevInstNum + 1 == num)
		{
			ifGetName(pEntry->ifIndex, ifname, IFNAMSIZ);
			*id = i;
			return 1;
		}
	}
	return 0;
}



#ifdef CONFIG_DEV_xDSL
/***** DSL Utilities ********************************************************/
char	*strAdsl2WanStd[]=
{
	"G.992.1_Annex_A",
	"G.992.1_Annex_B",
	"G.992.1_Annex_C",
	"T1.413",
	"T1.413i2",
	"ETSI_101_388",
	"G.992.2",
	"G.992.3_Annex_A",
	"G.992.3_Annex_B",
	"G.992.3_Annex_C",

	"G.992.3_Annex_I",
	"G.992.3_Annex_J",
	"G.992.3_Annex_L",
	"G.992.3_Annex_M",
	"G.992.4",
	"G.992.5_Annex_A",
	"G.992.5_Annex_B",
	"G.992.5_Annex_C",
	"G.992.5_Annex_I",
	"G.992.5_Annex_J",

	"G.992.5_Annex_M",
	"G.993.1",
	"G.993.1_Annex_A",
	"G.993.2_Annex_A",
	"G.993.2_Annex_B",
	"G.993.1_Annex_C",
	NULL
};


/* copy from boa/src/LINUX/fmmgmt.c */
int get_DSLWANStat( struct net_device_stats *nds )
{
	int skfd, i;
	struct atmif_sioc mysio;
	struct SAR_IOCTL_CFG cfg;

	// pvc statistics
	if((skfd = socket(PF_ATMPVC, SOCK_DGRAM, 0)) < 0){
		perror("socket open error");
		return -1;
	}

	mysio.number = 0;

	for (i=0; i < MAX_VC_NUM; i++)
	{
		cfg.ch_no = i;
		mysio.arg = (void *)&cfg;
		if(ioctl(skfd, ATM_SAR_GETSTAT, &mysio)<0)
		{
			(void)close(skfd);
			return -1;
		}

		if (cfg.created == 0)
			continue;

		nds->tx_bytes += cfg.stat.tx_byte_cnt;
		nds->rx_bytes += cfg.stat.rx_byte_cnt;
		nds->tx_packets += cfg.stat.tx_pkt_ok_cnt;
		nds->rx_packets += cfg.stat.rx_pkt_cnt;
		nds->tx_errors += cfg.stat.tx_pkt_fail_cnt;
		nds->rx_errors += cfg.stat.rx_pkt_fail;
	}

	(void)close(skfd);

	return 0;
}

int getATMStats( unsigned char vpi, unsigned short vci, ch_stat *stat)
{
	int skfd, i;
	struct atmif_sioc mysio;
	struct SAR_IOCTL_CFG cfg;

	if( stat == NULL ) return -1;

	// pvc statistics
	if((skfd = socket(PF_ATMPVC, SOCK_DGRAM, 0)) < 0){
		perror("socket open error");
		return -1;
	}

	mysio.number = 0;

	for (i=0; i < MAX_VC_NUM; i++)
	{
		cfg.ch_no = i;
		mysio.arg = (void *)&cfg;
		if(ioctl(skfd, ATM_SAR_GETSTAT, &mysio)<0)
		{
			(void)close(skfd);
			return -1;
		}

		if (cfg.created == 0)
			continue;

		if( cfg.vpi==vpi && cfg.vci==vci )
		{
			memcpy(stat, &(cfg.stat), sizeof(ch_stat));
			break;
		}

	}
	(void)close(skfd);

	return 0;
}

int getAAL5CRCErrors( unsigned char vpi, unsigned short vci, unsigned int *count )
{
	int skfd, i;
	struct atmif_sioc mysio;
	struct SAR_IOCTL_CFG cfg;
	//struct ch_stat stat;

	if( count==NULL ) return -1;
	*count = 0;

	// pvc statistics
	if((skfd = socket(PF_ATMPVC, SOCK_DGRAM, 0)) < 0){
		perror("socket open error");
		return -1;
	}

	mysio.number = 0;

	for (i=0; i < MAX_VC_NUM; i++)
	{
		cfg.ch_no = i;
		mysio.arg = (void *)&cfg;
		if(ioctl(skfd, ATM_SAR_GETSTAT, &mysio)<0)
		{
			(void)close(skfd);
			return -1;
		}

		if (cfg.created == 0)
			continue;

		if( cfg.vpi==vpi && cfg.vci==vci )
		{
			*count = cfg.stat.rx_crc_error;
			break;
		}

	}
	(void)close(skfd);


	return 0;
}
#endif

#ifdef WLAN_SUPPORT
/***** WLAN Utilities ********************************************************/
char *wlan_name[ WLAN_IF_NUM ]=
{
	"wlan0"
#ifdef WLAN_MBSSID
	, "wlan0-vap0", "wlan0-vap1", "wlan0-vap2", "wlan0-vap3"
#endif
#ifdef WLAN_DUALBAND_CONCURRENT
	,"wlan1"
#ifdef WLAN_MBSSID
	, "wlan1-vap1", "wlan1-vap1", "wlan1-vap2", "wlan1-vap3"
#endif
#endif
};
#endif


#define	WLANUPDATETIME	90


#ifdef WLAN_SUPPORT
char	gWLANAssociations[ sizeof(WLAN_STA_INFO_T)*(MAX_STA_NUM+1) ];
time_t		gWLANAssUpdateTime=0;
unsigned int	gWLANTotalClients=0;
int		gWLANIDForAssInfo=-1; /*-1:no info, 0:wlan0, 1:wlan0-vap0, 2:wlan-vap1.....*/

char WLANASSFILE[] = "/tmp/stainfo";
int updateWLANAssociations( void )
{
	int i;
	time_t c_time=0;

	c_time = time(NULL);
	if( c_time >= gWLANAssUpdateTime+WLANUPDATETIME )
	{
//		if(gWLANAssociations==NULL)
//		{
//			gWLANAssociations = malloc( sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1));
//			if(gWLANAssociations==NULL) return -1;
//		}

		for(i=0;i<WLAN_IF_NUM;i++)
		{
			char filename[32];
			FILE *fp=NULL;
			int  has_info;

			has_info=1;
			memset( gWLANAssociations, 0, sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1) );
			if ( getWlStaInfo( (char*)wlan_name[i],  (WLAN_STA_INFO_T *)gWLANAssociations ) < 0 )
			{
				memset( gWLANAssociations, 0, sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1) );
				has_info=0;
			}

#if 0
{
			int j;
			int found=0;
			for (j=1; j<=MAX_STA_NUM; j++)
			{
				WLAN_STA_INFO_T *pInfo;
				pInfo = (WLAN_STA_INFO_T*)&gWLANAssociations[j*sizeof(WLAN_STA_INFO_T)];
				if (pInfo->aid && (pInfo->flag & STA_INFO_FLAG_ASOC))
					found++;
			}
			fprintf( stderr, "wlan_name:%s, found stations:%d\n ", wlan_name[i], found );
}
#endif

			sprintf( filename, "%s.%s", WLANASSFILE, wlan_name[i] );
			fp=fopen( filename, "wb" );
			if(fp)
			{
				if(has_info)
				{
					fwrite( gWLANAssociations, 1, sizeof(WLAN_STA_INFO_T)*(MAX_STA_NUM+1), fp );
				}
				fclose(fp);
			}
		}
#if 0
{
		//test wireless association clients
		srand( (unsigned int)time(NULL) );
		for (i=1; i<=MAX_STA_NUM; i++)
		{
			int rnum;

			pInfo = (WLAN_STA_INFO_T*)&gWLANAssociations[i*sizeof(WLAN_STA_INFO_T)];
			if( (rand() % 4 )==2 )
			{
				pInfo->flag =  pInfo->flag | STA_INFO_FLAG_ASOC;
				pInfo->aid=i;
				pInfo->addr[0]= (unsigned int)i;
				fprintf( stderr, "Add one wlanassociation client: mac=%02x\n", pInfo->addr[0] );
			}

		}
}
#endif

		gWLANAssUpdateTime = c_time;
	}
	return 0;
}

int loadWLANAssInfoByInstNum( unsigned int instnum )
{
	char filename[32];
	FILE *fp=NULL;
	int  wlanid,found=0;
	if( instnum==0 || instnum>WLAN_IF_NUM ) return -1;

	if( updateWLANAssociations()< 0 )
	{
		gWLANIDForAssInfo = -1;
		memset( gWLANAssociations, 0, sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1) );
		gWLANTotalClients=0;
		return -1;
	}

	wlanid = instnum-1;
	if( gWLANIDForAssInfo==wlanid ) return 0;

	gWLANIDForAssInfo = -1;
	memset( gWLANAssociations, 0, sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1) );
	gWLANTotalClients=0;

	sprintf( filename, "%s.%s", WLANASSFILE, wlan_name[wlanid] );
	fp=fopen( filename, "rb" );
	if(fp)
	{
		int i;
		WLAN_STA_INFO_T *pInfo;

		fread( gWLANAssociations,  1, sizeof(WLAN_STA_INFO_T)*(MAX_STA_NUM+1), fp );
		fclose(fp);

		for (i=1; i<=MAX_STA_NUM; i++)
		{
			pInfo = (WLAN_STA_INFO_T*)&gWLANAssociations[i*sizeof(WLAN_STA_INFO_T)];
			if (pInfo->aid && (pInfo->flag & STA_INFO_FLAG_ASOC))
				found++;
		}
		gWLANTotalClients = found;
	}

	return 0;
}
#endif

#ifdef CONFIG_PPP
/***** PPP Utilities *********************************************************/
/*refer to fmstatus.c & utility.c*/
int getPPPConStatus( char *pppname, char *status )
{
	char buff[256];
	FILE *fp = NULL;
	char strif[6],tmpst[32];
	int  ret=-1;

	if( (pppname==NULL) || (status==NULL) ) return -1;

	status[0]=0;
	if (!(fp=fopen(PPP_CONF, "r")))
	{
		fprintf( stderr, "%s not exists.\n", PPP_CONF);
		return -1;
	}else{
		fgets(buff, sizeof(buff), fp);
		while( fgets(buff, sizeof(buff), fp) != NULL )
		{
			//if   dev   dev_v   gw phase          username                password
			if(sscanf(buff, "%s %*s %*s %*s %s", strif, tmpst)!=2)
			{
				fprintf( stderr, "Unsuported ppp configuration format\n");
				break;
			}
			if ( !strcmp(pppname, strif) )
			{
				strcpy( status, tmpst );
				ret=0;
				break;
			}
		}
		fclose(fp);

		/*status is defined in if_spppsubr.c*/
		if( strcmp( status, "Dead" ) == 0 )
			strcpy(status, "Disconnected");
		else if( strcmp( status, "Establish" ) == 0 )
			strcpy(status, "Connecting");
		else if( strcmp( status, "Terminate" ) == 0 )
			strcpy(status, "Disconnecting");
		else if( strcmp( status, "Authenticate" ) == 0 )
			strcpy(status, "Authenticating");
		else if( strcmp( status, "Network" ) == 0 )
			strcpy(status, "Connected");
		else
			strcpy(status, "Disconnected");
	}
	return ret;
}

/* Jenny, refer to if_sppp.h */
enum ppp_last_connection_error {
	ERROR_NONE, ERROR_ISP_TIME_OUT, ERROR_COMMAND_ABORTED,
	ERROR_NOT_ENABLED_FOR_INTERNET, ERROR_BAD_PHONE_NUMBER,
	ERROR_USER_DISCONNECT, ERROR_ISP_DISCONNECT, ERROR_IDLE_DISCONNECT,
	ERROR_FORCED_DISCONNECT, ERROR_SERVER_OUT_OF_RESOURCES,
	ERROR_RESTRICTED_LOGON_HOURS, ERROR_ACCOUNT_DISABLED,
	ERROR_ACCOUNT_EXPIRED, ERROR_PASSWORD_EXPIRED,
	ERROR_AUTHENTICATION_FAILURE, ERROR_NO_DIALTONE, ERROR_NO_CARRIER,
	ERROR_NO_ANSWER, ERROR_LINE_BUSY, ERROR_UNSUPPORTED_BITSPERSECOND,
	ERROR_TOO_MANY_LINE_ERRORS, ERROR_IP_CONFIGURATION, ERROR_UNKNOWN
};

const char * getLastConnectionError(unsigned int ifindex)
{
	FILE *fp;
	char buff[10];
	int pppif;
	enum ppp_last_connection_error error = ERROR_UNKNOWN;

	fp = fopen("/tmp/ppp_error_log", "r");
	if (fp) {
		while (fgets(buff, sizeof(buff), fp) != NULL) {
			sscanf(buff, "%d:%d", &pppif, (int*)&error);
			if (pppif == PPP_INDEX(ifindex))
					break;
		}
		fclose(fp);
	}

	switch (error) {
		case ERROR_NONE:						return "ERROR_NONE";
		case ERROR_ISP_TIME_OUT:				return "ERROR_ISP_TIME_OUT";
		case ERROR_COMMAND_ABORTED:			return "ERROR_COMMAND_ABORTED";
		case ERROR_NOT_ENABLED_FOR_INTERNET: 	return "ERROR_NOT_ENABLED_FOR_INTERNET";
		case ERROR_BAD_PHONE_NUMBER:			return "ERROR_BAD_PHONE_NUMBER";
		case ERROR_USER_DISCONNECT:			return "ERROR_USER_DISCONNECT";
		case ERROR_ISP_DISCONNECT:				return "ERROR_ISP_DISCONNECT";
		case ERROR_IDLE_DISCONNECT:			return "ERROR_IDLE_DISCONNECT";
		case ERROR_FORCED_DISCONNECT: 		return "ERROR_FORCED_DISCONNECT";
		case ERROR_SERVER_OUT_OF_RESOURCES:	return "ERROR_SERVER_OUT_OF_RESOURCES";
		case ERROR_RESTRICTED_LOGON_HOURS:	return "ERROR_RESTRICTED_LOGON_HOURS";
		case ERROR_ACCOUNT_DISABLED:			return "ERROR_ACCOUNT_DISABLED";
		case ERROR_ACCOUNT_EXPIRED:			return "ERROR_ACCOUNT_EXPIRED";
		case ERROR_PASSWORD_EXPIRED: 			return "ERROR_PASSWORD_EXPIRED";
		case ERROR_AUTHENTICATION_FAILURE:		return "ERROR_AUTHENTICATION_FAILURE";
		case ERROR_NO_DIALTONE:				return "ERROR_NO_DIALTONE";
		case ERROR_NO_CARRIER:					return "ERROR_NO_CARRIER";
		case ERROR_NO_ANSWER:					return "ERROR_NO_ANSWER";
		case ERROR_LINE_BUSY: 					return "ERROR_LINE_BUSY";
		case ERROR_UNSUPPORTED_BITSPERSECOND:	return "ERROR_UNSUPPORTED_BITSPERSECOND";
		case ERROR_TOO_MANY_LINE_ERRORS:		return "ERROR_TOO_MANY_LINE_ERRORS";
		case ERROR_IP_CONFIGURATION:			return "ERROR_IP_CONFIGURATION";
		case ERROR_UNKNOWN: 					return "ERROR_UNKNOWN";
	}
	return "ERROR_UNKNOWN";
}

int getPPPCurrentMRU( char *pppname, int ppptype, unsigned int *cmru )
{
	char buff[256];
	FILE *fp;
	char strif[6];
	int  ret=-1;

	if( (pppname==NULL) || (cmru==NULL) ||
	    ((ppptype!=CHANNEL_MODE_PPPOE) && (ppptype!=CHANNEL_MODE_PPPOA)) )
	      return -1;

	*cmru=0;
	if (!(fp=fopen(PPP_CONF, "r")))
	{
		fclose(fp);
		fprintf( stderr, "%s not exists.\n", PPP_CONF);
		return -1;
	}else{
		fgets(buff, sizeof(buff), fp);
		while( fgets(buff, sizeof(buff), fp) != NULL )
		{
			//if   dev   dev_v   gw phase          username                password	mru
			if(sscanf(buff, "%s %*s %*s %*s %*s %*s %*s %u", strif, cmru)!=2)
			{
				fprintf( stderr, "Unsuported ppp configuration format\n");
				break;
			}
			if ( !strcmp(pppname, strif) )
			{
				ret=0;
				break;
			}
		}
		fclose(fp);
	}
	return ret;
}

int getPPPLCPEcho( char *pppname, int ppptype, unsigned int *echo )
{
	FILE *fp;

	if( (pppname==NULL) || (echo==NULL) ||
	    ((ppptype!=CHANNEL_MODE_PPPOE) && (ppptype!=CHANNEL_MODE_PPPOA)) )
	      return -1;

	*echo=0;
	fp = fopen("/tmp/ppp_lcp_echo", "r");
	if (fp) {
		fscanf(fp, "%d", echo);
		fclose(fp);
		return 0;
	}
	else
		return -1;
}

int getPPPEchoRetry( char *pppname, int ppptype, unsigned int *retry )
{
	FILE *fp;

	if( (pppname==NULL) || (retry==NULL) ||
	    ((ppptype!=CHANNEL_MODE_PPPOE) && (ppptype!=CHANNEL_MODE_PPPOA)) )
	      return -1;

	*retry=0;
	fp = fopen("/tmp/ppp_echo_retry", "r");
	if (fp) {
		fscanf(fp, "%d", retry);
		fclose(fp);
		return 0;
	}
	else
		return -1;
}
#endif

/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
int gStartReset=0;
int resetChainID=-1;
static int resetThread=0;

// calculate the 15-0(bits) Cell Rate register value (PCR or SCR)
// return its corresponding register value
static int cr2reg(int pcr)
{
	int k, e, m, pow2, reg;

	k = pcr;
	e=0;

	while (k>1) {
		k = k/2;
		e++;
	}

	//printf("pcr=%d, e=%d\n", pcr,e);
	pow2 = 1;
	for (k = 1; k <= e; k++)
		pow2*=2;

	//printf("pow2=%d\n", pow2);
	//m = ((pcr/pow2)-1)*512;
	k = 0;
	while (pcr >= pow2) {
		pcr -= pow2;
		k++;
	}
	m = (k-1)*512 + pcr*512/pow2;
	//printf("m=%d\n", m);
	reg = (e<<9 | m );
	//printf("reg=%d\n", reg);
	return reg;
}

static void *reset_thread(void *arg) {
	MIB_CE_ATM_VC_T *pEntry,Entry;
	char wanif[16], ifIdx[3], pppif[6],vpivci[6],qosParms[64];//,cmdbuf[256] ;
	int pcreg,screg;

	pEntry = &Entry;
	//printf("%s:%d resetChainID=%d\n",__FUNCTION__,__LINE__,resetChainID);
	if( !mib_chain_get( MIB_ATM_VC_TBL, resetChainID, (void*)pEntry ) )
		return NULL;

	//CONFIG_PTMWAN, CONFIG_ETHWAN
	//snprintf(wanif, 5, "vc%d", VC_INDEX(pEntry->ifIndex));
	ifGetName(PHY_INTF(pEntry->ifIndex),wanif,sizeof(wanif));
	snprintf(ifIdx, 3, "%u", PPP_INDEX(pEntry->ifIndex));
	snprintf(pppif, 6, "ppp%u", PPP_INDEX(pEntry->ifIndex));

#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	if ((pEntry->cmode == CHANNEL_MODE_IPOE) && (pEntry->ipDhcp == DHCP_CLIENT))
		delDhcpcOption(pEntry->ifIndex);
#endif

	if (pEntry->cmode == CHANNEL_MODE_BRIDGE)
	{
		va_cmd(IFCONFIG,2,1,wanif,"down");
		va_cmd(IFCONFIG,2,1,wanif,"up");
	}
	else if ((CHANNEL_MODE_T)pEntry->cmode == CHANNEL_MODE_IPOE
		||(CHANNEL_MODE_T)pEntry->cmode == CHANNEL_MODE_RT1483
		#ifdef CONFIG_ATM_CLIP
		||(CHANNEL_MODE_T)pEntry->cmode == CHANNEL_MODE_RT1577
		#endif
		)
	{
		if(pEntry->ipDhcp != (char)DHCP_DISABLED)
		{
			int dhcpcpid;
			char pidfile[32];

			snprintf(pidfile, 32, "/var/run/udhcpc.pid");
			dhcpcpid = read_pid(pidfile);
			if(dhcpcpid > 0)
				kill(dhcpcpid, 15);

			va_cmd("/bin/udhcpc",6,0,
				"-i",wanif,
				"-p",DHCPC_PID,
				"-s",DHCPC_SCRIPT
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
				,"-w",pEntry->ifIndex
#endif
				);
		}
		else
		{
			va_cmd(IFCONFIG,2,1,wanif,"down");
			va_cmd(IFCONFIG,2,1,wanif,"up");
		}
	}
	else if ((CHANNEL_MODE_T)pEntry->cmode == CHANNEL_MODE_PPPOE)
	{
		if (pEntry->pppCtype != MANUAL)
		{
			// spppctl add 0 pppoe vc0 username USER password PASSWORD gw 1 mru xxxx acname xxx
			va_cmd("/bin/spppctl",14,0,
				"add",ifIdx,
				"pppoe",wanif,
				"username",pEntry->pppUsername,
				"password",pEntry->pppPassword,
				"gw",pEntry->dgw,
				"mru",pEntry->mtu,
				"acname",pEntry->pppACName);
		}
		else
		{
			va_cmd(IFCONFIG,2,1,pppif,"down");
			va_cmd(IFCONFIG,2,1,pppif,"up");
		}
	}
	else if ((CHANNEL_MODE_T)pEntry->cmode == CHANNEL_MODE_PPPOA)
	{
		if (pEntry->pppCtype != MANUAL)
		{
			pcreg = cr2reg(pEntry->pcr);
			sprintf(vpivci,"%u.%u",pEntry->vpi, pEntry->vci);
			if ((ATM_QOS_T)pEntry->qos == ATMQOS_CBR)
			{
				snprintf(qosParms, 64, "cbr:pcr=%u", pcreg);
			}
			else if ((ATM_QOS_T)pEntry->qos == ATMQOS_VBR_NRT)
			{
				screg = cr2reg(pEntry->scr);
				snprintf(qosParms, 64, "nrt-vbr:pcr=%u,scr=%u,mbs=%u",
					pcreg, screg, pEntry->mbs);
			}
			else if ((ATM_QOS_T)pEntry->qos == ATMQOS_VBR_RT)
			{
				screg = cr2reg(pEntry->scr);
				snprintf(qosParms, 64, "rt-vbr:pcr=%u,scr=%u,mbs=%u",
					pcreg, screg, pEntry->mbs);
			}
			else //if ((ATM_QOS_T)pEntry->qos == ATMQOS_UBR)
			{
				snprintf(qosParms, 64, "ubr:pcr=%u", pcreg);
			}

			//spppctl add 0 pppoa vpi.vci encaps ENCAP qos xxx  username USER password PASSWORD gw 1 mru xxxx
			va_cmd("/bin/spppctl",16,0,
				"add",ifIdx,
				"pppoa",vpivci,
				"encaps",pEntry->encap,
				"qos",qosParms,
				"username",pEntry->pppUsername,
				"password",pEntry->pppPassword,
				"gw",pEntry->dgw,
				"mru",pEntry->mtu);
		}
		else
		{
			va_cmd(IFCONFIG,2,1,pppif,"down");
			va_cmd(IFCONFIG,2,1,pppif,"up");
		}
	}

//END:
	resetThread=0;

	return NULL;
}

void cwmpStartReset() {
	pthread_t reset_pid;

	//printf("%s:%d resetThread=%d,resetChainID=%d\n",__FUNCTION__,__LINE__,resetThread,resetChainID);
	if (resetThread) {
		//printf("reset in progress, try again later=\n");
		return;
	}

	resetThread = 1;
	if( pthread_create( &reset_pid, NULL, reset_thread, 0 ) != 0 )
	{
		resetThread = 0;
		return;
	}
	pthread_detach(reset_pid);

}
#endif
/*ping_zhang:20081217 END*/

