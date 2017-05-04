#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
//#include <net/if.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <qcsapi_output.h>
#include <rpc/rpc.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#include "mib.h"
#include "sysconfig.h"
#include "utility.h"


#ifndef __packed
#define __packed __attribute__((__packed__))
#endif

#include <qcsapi.h>
#include <qcsapi_rpc/client/qcsapi_rpc_client.h>
#include <qcsapi_rpc/generated/qcsapi_rpc.h>
#include <qcsapi_driver.h>
#include <call_qcsapi.h>

#define MAX_RETRY_TIMES 15
#define WIFINAME "wifi0"
#define QTN_STATE "/var/qcsapi_rmt_server_state"
#define RADIUS_PORT 1812

static int s_c_rpc_use_udp = 1;

#ifdef WLAN_CLIENT
int rt_qcsapi_set_STA_config(char *ifname);
#endif
#ifdef WLAN_WDS
int setupWDS_qtn();
#endif

#if 0

static int client_qcsapi_get_udp_retry_timeout(int *argc, char ***argv)
{
	int timeout = -1;

	if (argc && argv && *argc >= 2 && strcmp((*argv)[1], "--udp-retry-timeout") == 0) {
		timeout = atoi((const char *)(*argv)[2]);

		/* move program argv[0] */
		(*argv)[2] = (*argv)[0];

		/* skip over --host <arg> args */
		*argc = *argc - 2;
		*argv = &(*argv)[2];
	}

	return timeout;
}
#endif

static const char * const cfg_file_paths[] = {
	"/"
	"/mnt/jffs2/rmt_ip.conf",
	"/etc/qcsapi_target_ip.conf",
	NULL	/* last entry must be null */
};

#define MAX_HOSTNAME_SIZE	128

static void rt_get_qtn_regulatory_regions(int region_by_index, char *region_by_name);


static void trim_trailing_space(char *buf)
{
	int i;
	for (i = strlen(buf) - 1; isspace(buf[i]); i--) {
		buf[i] = '\0';
	}
}

static const char *first_nonspace(const char *buf)
{
	while (*buf && isspace(*buf)) {
		buf++;
	}
	return buf;
}

static const char * client_qcsapi_find_host_read_file(const char * const filename)
{
	static char hostbuf[MAX_HOSTNAME_SIZE];
	const char* host = NULL;
	char* fret;

	FILE *file = fopen(filename, "r");
	if (file == NULL) {
		/* files may legitimately not exist */
		return NULL;
	}

	/* assume the file contains the target host on the first line */
	fret = fgets(hostbuf, MAX_HOSTNAME_SIZE, file);
	if (fret || feof(file)) {
		trim_trailing_space(hostbuf);
		host = first_nonspace(hostbuf);
	} else {
		fprintf(stderr, "%s: error reading file '%s': %s\n",
				__FUNCTION__, filename, strerror(errno));
	}

	fclose(file);

	return host;
}

static const char* get_host_addr()
{
	int i;
	const char *host;

#if 0
	/* check for command line arguments */
	if (argc && argv && *argc >= 2 && strcmp((*argv)[1], "--host") == 0) {
		host = (*argv)[2];

		/* move program argv[0] */
		(*argv)[2] = (*argv)[0];

		/* skip over --host <arg> args */
		*argc = *argc - 2;
		*argv = &(*argv)[2];

		return host;
	}
#endif

#if 0
	/* check for environment variables */
	host = getenv(QCSAPI_HOST_ENV_VAR);
	if (host) {
		return host;
	}
#endif
	/* check for config files */
	for (i = 0; cfg_file_paths[i]; i++) {
		host = client_qcsapi_find_host_read_file(cfg_file_paths[i]);
		if (host) {
			return host;
		}
	}

	return NULL;
}

char *qtn_wlan_interface[]={"wifi0", "wifi1", "wifi2", "wifi3", "wifi4", "wifi5", "wifi6", "wifi7"};
char* rt_get_qtn_ifname(char *ifname)
{
	int index;
	if(strncmp(ifname, qtn_wlan_interface[0], 4)==0)
		return ifname;
	else if(strlen(ifname)==5)
		return qtn_wlan_interface[0];
	else{
		index = ifname[9]-'0';
		return qtn_wlan_interface[index+1];
	}
}

int rt_qcsapi_get_bssid(char *ifname, unsigned char *bssid)
{
	int ret=0;
	const char *host;
	CLIENT *clnt=NULL;

	int udp_retry_timeout=-1;

	host = get_host_addr();

	clnt = clnt_create(host, QCSAPI_PROG, QCSAPI_VERS, "udp");
	if (clnt == NULL) {
		clnt = clnt_create(host, QCSAPI_PROG, QCSAPI_VERS, "tcp");
	} else {
		if (udp_retry_timeout>0) {
			struct timeval value;
			value.tv_sec = (time_t)udp_retry_timeout;
			value.tv_usec = (suseconds_t)0;
			clnt_control(clnt, CLSET_RETRY_TIMEOUT, (char *)&value);
		}
	}

	if (clnt == NULL) {
		clnt_pcreateerror(host);
		return -1;
	}

	client_qcsapi_set_rpcclient(clnt);

	if((ret = qcsapi_interface_get_BSSID(ifname, bssid))!=0)
		printf("%s %d: failed %d \n", __FUNCTION__,__LINE__, ret);

	clnt_destroy(clnt);
	return ret;
}


int rt_qcsapi_get_ssid(char *ifname, char *ssid)
{
	int ret=0;
	const char *host;
	CLIENT *clnt=NULL;

	int udp_retry_timeout=-1;

	host = get_host_addr();

	clnt = clnt_create(host, QCSAPI_PROG, QCSAPI_VERS, "udp");
	if (clnt == NULL) {
		clnt = clnt_create(host, QCSAPI_PROG, QCSAPI_VERS, "tcp");
	} else {
		if (udp_retry_timeout>0) {
			struct timeval value;
			value.tv_sec = (time_t)udp_retry_timeout;
			value.tv_usec = (suseconds_t)0;
			clnt_control(clnt, CLSET_RETRY_TIMEOUT, (char *)&value);
		}
	}

	if (clnt == NULL) {
		clnt_pcreateerror(host);
		return -1;
	}

	client_qcsapi_set_rpcclient(clnt);

	if((ret = qcsapi_wifi_get_SSID(ifname, ssid))!=0)
		printf("%s %d: failed %d \n", __FUNCTION__,__LINE__, ret);

	clnt_destroy(clnt);
	return ret;
}


int rt_qcsapi_get_channel(unsigned char *chan)
{
	int ret=0;
	const char *host;
	CLIENT *clnt=NULL;
	unsigned int channel;

	int udp_retry_timeout=-1;

	host = get_host_addr();

	clnt = clnt_create(host, QCSAPI_PROG, QCSAPI_VERS, "udp");
	if (clnt == NULL) {
		clnt = clnt_create(host, QCSAPI_PROG, QCSAPI_VERS, "tcp");
	} else {
		if (udp_retry_timeout>0) {
			struct timeval value;
			value.tv_sec = (time_t)udp_retry_timeout;
			value.tv_usec = (suseconds_t)0;
			clnt_control(clnt, CLSET_RETRY_TIMEOUT, (char *)&value);
		}
	}

	if (clnt == NULL) {
		clnt_pcreateerror(host);
		return -1;
	}

	client_qcsapi_set_rpcclient(clnt);

	if((ret = qcsapi_wifi_get_channel(WIFINAME, &channel))!=0)
		printf("%s %d: failed %d \n", __FUNCTION__,__LINE__, ret);

	*chan = channel;
	clnt_destroy(clnt);
	return ret;
}

int rt_qcsapi_get_status(char *ifname, char *status)
{
	int ret=0;
	const char *host;
	CLIENT *clnt=NULL;

	int udp_retry_timeout=-1;

	host = get_host_addr();

	clnt = clnt_create(host, QCSAPI_PROG, QCSAPI_VERS, "udp");
	if (clnt == NULL) {
		clnt = clnt_create(host, QCSAPI_PROG, QCSAPI_VERS, "tcp");
	} else {
		if (udp_retry_timeout>0) {
			struct timeval value;
			value.tv_sec = (time_t)udp_retry_timeout;
			value.tv_usec = (suseconds_t)0;
			clnt_control(clnt, CLSET_RETRY_TIMEOUT, (char *)&value);
		}
	}

	if (clnt == NULL) {
		clnt_pcreateerror(host);
		return -1;
	}

	client_qcsapi_set_rpcclient(clnt);

	ret = qcsapi_interface_get_status(ifname, status);
	
	clnt_destroy(clnt);
	return ret;
}



int rt_qcsapi_get_bss_info(char *interface, bss_info *pInfo)
{
	int ret=0;
	char *ifname = rt_get_qtn_ifname(interface);
	char status[16];
	if(rt_qcsapi_get_status(ifname, status)<0)
		return -1;
	else if(!strcmp(status, "Disabled"))
		return -1;
		
	ret |= rt_qcsapi_get_bssid(ifname, pInfo->bssid);
#ifdef WLAN_CLIENT
	if(pInfo->bssid[0] || pInfo->bssid[1] || pInfo->bssid[2] || pInfo->bssid[3] || pInfo->bssid[4] || pInfo->bssid[5])
		pInfo->state = STATE_CONNECTED;
	else
		pInfo->state = STATE_SCANNING;
#endif
	ret |= rt_qcsapi_get_ssid(ifname, (char *)pInfo->ssid);
	ret |= rt_qcsapi_get_channel(&pInfo->channel);
	return ret;
}

int rt_qcsapi_get_sta_num(char *interface, int *number)
{
	int ret=0;
	const char *host;
	CLIENT *clnt=NULL;

	char *ifname = rt_get_qtn_ifname(interface);

	int udp_retry_timeout=-1;

	host = get_host_addr();

	clnt = clnt_create(host, QCSAPI_PROG, QCSAPI_VERS, "udp");
	if (clnt == NULL) {
		clnt = clnt_create(host, QCSAPI_PROG, QCSAPI_VERS, "tcp");
	} else {
		if (udp_retry_timeout>0) {
			struct timeval value;
			value.tv_sec = (time_t)udp_retry_timeout;
			value.tv_usec = (suseconds_t)0;
			clnt_control(clnt, CLSET_RETRY_TIMEOUT, (char *)&value);
		}
	}

	if (clnt == NULL) {
		clnt_pcreateerror(host);
		return -1;
	}

	client_qcsapi_set_rpcclient(clnt);

	if((ret = qcsapi_wifi_get_count_associations(ifname, (unsigned int *)number))!=0)
		printf("%s %d: failed %d \n", __FUNCTION__,__LINE__, ret);

	clnt_destroy(clnt);
	return ret;
}

int rt_qcsapi_get_sta_rx_pkt(char *ifname, unsigned int index, unsigned int *number)
{
	int ret=0;
	const char *host;
	CLIENT *clnt=NULL;

	int udp_retry_timeout=-1;

	host = get_host_addr();

	clnt = clnt_create(host, QCSAPI_PROG, QCSAPI_VERS, "udp");
	if (clnt == NULL) {
		clnt = clnt_create(host, QCSAPI_PROG, QCSAPI_VERS, "tcp");
	} else {
		if (udp_retry_timeout>0) {
			struct timeval value;
			value.tv_sec = (time_t)udp_retry_timeout;
			value.tv_usec = (suseconds_t)0;
			clnt_control(clnt, CLSET_RETRY_TIMEOUT, (char *)&value);
		}
	}

	if (clnt == NULL) {
		clnt_pcreateerror(host);
		return -1;
	}

	client_qcsapi_set_rpcclient(clnt);

	if((ret = qcsapi_wifi_get_rx_packets_per_association(ifname, index, number))!=0)
		printf("%s %d: failed %d \n", __FUNCTION__,__LINE__, ret);

	clnt_destroy(clnt);
	return ret;
}

int rt_qcsapi_get_sta_tx_pkt(char* ifname, unsigned int index, unsigned int *number)
{
	int ret=0;
	const char *host;
	CLIENT *clnt=NULL;

	int udp_retry_timeout=-1;

	host = get_host_addr();

	clnt = clnt_create(host, QCSAPI_PROG, QCSAPI_VERS, "udp");
	if (clnt == NULL) {
		clnt = clnt_create(host, QCSAPI_PROG, QCSAPI_VERS, "tcp");
	} else {
		if (udp_retry_timeout>0) {
			struct timeval value;
			value.tv_sec = (time_t)udp_retry_timeout;
			value.tv_usec = (suseconds_t)0;
			clnt_control(clnt, CLSET_RETRY_TIMEOUT, (char *)&value);
		}
	}

	if (clnt == NULL) {
		clnt_pcreateerror(host);
		return -1;
	}

	client_qcsapi_set_rpcclient(clnt);

	if((ret = qcsapi_wifi_get_tx_packets_per_association(ifname, index, number))!=0)
		printf("%s %d: failed %d \n", __FUNCTION__,__LINE__, ret);

	clnt_destroy(clnt);
	return ret;
}

int rt_qcsapi_get_sta_tx_err_pkt(char* ifname, unsigned int index, unsigned int *number)
{
	int ret=0;
	const char *host;
	CLIENT *clnt=NULL;

	int udp_retry_timeout=-1;

	host = get_host_addr();

	clnt = clnt_create(host, QCSAPI_PROG, QCSAPI_VERS, "udp");
	if (clnt == NULL) {
		clnt = clnt_create(host, QCSAPI_PROG, QCSAPI_VERS, "tcp");
	} else {
		if (udp_retry_timeout>0) {
			struct timeval value;
			value.tv_sec = (time_t)udp_retry_timeout;
			value.tv_usec = (suseconds_t)0;
			clnt_control(clnt, CLSET_RETRY_TIMEOUT, (char *)&value);
		}
	}

	if (clnt == NULL) {
		clnt_pcreateerror(host);
		return -1;
	}

	client_qcsapi_set_rpcclient(clnt);

	if((ret = qcsapi_wifi_get_tx_err_packets_per_association(ifname, index, number))!=0)
		printf("%s %d: failed %d \n", __FUNCTION__,__LINE__, ret);

	clnt_destroy(clnt);
	return ret;
}


int rt_qcsapi_get_sta_tx_phy_rate(char *ifname, unsigned int index, unsigned int *tx_phy_rate)
{
	int ret=0;
	const char *host;
	CLIENT *clnt=NULL;

	int udp_retry_timeout=-1;

	host = get_host_addr();

	clnt = clnt_create(host, QCSAPI_PROG, QCSAPI_VERS, "udp");
	if (clnt == NULL) {
		clnt = clnt_create(host, QCSAPI_PROG, QCSAPI_VERS, "tcp");
	} else {
		if (udp_retry_timeout>0) {
			struct timeval value;
			value.tv_sec = (time_t)udp_retry_timeout;
			value.tv_usec = (suseconds_t)0;
			clnt_control(clnt, CLSET_RETRY_TIMEOUT, (char *)&value);
		}
	}

	if (clnt == NULL) {
		clnt_pcreateerror(host);
		return -1;
	}

	client_qcsapi_set_rpcclient(clnt);

	if((ret = qcsapi_wifi_get_tx_phy_rate_per_association(ifname, index, tx_phy_rate))!=0)
		printf("%s %d: failed %d \n", __FUNCTION__,__LINE__, ret);

	clnt_destroy(clnt);
	return ret;
}

int rt_qcsapi_get_sta_mac_addr(char *ifname, unsigned int index, unsigned char *mac_addr)
{
	int ret=0;
	const char *host;
	CLIENT *clnt=NULL;

	int udp_retry_timeout=-1;

	host = get_host_addr();

	clnt = clnt_create(host, QCSAPI_PROG, QCSAPI_VERS, "udp");
	if (clnt == NULL) {
		clnt = clnt_create(host, QCSAPI_PROG, QCSAPI_VERS, "tcp");
	} else {
		if (udp_retry_timeout>0) {
			struct timeval value;
			value.tv_sec = (time_t)udp_retry_timeout;
			value.tv_usec = (suseconds_t)0;
			clnt_control(clnt, CLSET_RETRY_TIMEOUT, (char *)&value);
		}
	}

	if (clnt == NULL) {
		clnt_pcreateerror(host);
		return -1;
	}

	client_qcsapi_set_rpcclient(clnt);

	if((ret = qcsapi_wifi_get_associated_device_mac_addr(ifname, index, mac_addr))!=0)
		printf("%s %d: failed %d \n", __FUNCTION__,__LINE__, ret);

	clnt_destroy(clnt);
	return ret;
}
#ifdef WLAN_WDS
int rt_qcsapi_get_wds_peer_mac_addr(char *ifname, unsigned int index, unsigned char *mac_addr)
{
	int ret=0;
	const char *host;
	CLIENT *clnt=NULL;

	int udp_retry_timeout=-1;

	host = get_host_addr();

	clnt = clnt_create(host, QCSAPI_PROG, QCSAPI_VERS, "udp");
	if (clnt == NULL) {
		clnt = clnt_create(host, QCSAPI_PROG, QCSAPI_VERS, "tcp");
	} else {
		if (udp_retry_timeout>0) {
			struct timeval value;
			value.tv_sec = (time_t)udp_retry_timeout;
			value.tv_usec = (suseconds_t)0;
			clnt_control(clnt, CLSET_RETRY_TIMEOUT, (char *)&value);
		}
	}

	if (clnt == NULL) {
		clnt_pcreateerror(host);
		return -1;
	}

	client_qcsapi_set_rpcclient(clnt);

	if((ret = qcsapi_wds_get_peer_address(ifname, index, mac_addr))!=0)
		printf("%s %d: failed %d \n", __FUNCTION__,__LINE__, ret);

	clnt_destroy(clnt);
	return ret;
}
#endif

int rt_qcsapi_get_channel_list(char *ifname, int region_by_index, unsigned int bw, char *channel_list)
{
	int ret=0;
	const char *host;
	CLIENT *clnt=NULL;
	char region_by_name[5];

	int udp_retry_timeout=-1;

	host = get_host_addr();

	clnt = clnt_create(host, QCSAPI_PROG, QCSAPI_VERS, "udp");
	if (clnt == NULL) {
		clnt = clnt_create(host, QCSAPI_PROG, QCSAPI_VERS, "tcp");
	} else {
		if (udp_retry_timeout>0) {
			struct timeval value;
			value.tv_sec = (time_t)udp_retry_timeout;
			value.tv_usec = (suseconds_t)0;
			clnt_control(clnt, CLSET_RETRY_TIMEOUT, (char *)&value);
		}
	}

	if (clnt == NULL) {
		clnt_pcreateerror(host);
		return -1;
	}

	client_qcsapi_set_rpcclient(clnt);

	rt_get_qtn_regulatory_regions(region_by_index, region_by_name);

	if((ret = qcsapi_regulatory_get_list_regulatory_channels(region_by_name, bw, channel_list))!=0)
		printf("%s %d: failed %d \n", __FUNCTION__,__LINE__, ret);
	
	if(ret==-qcsapi_region_database_not_found){
		if((ret = qcsapi_wifi_get_list_regulatory_channels(region_by_name, bw, channel_list))!=0)
			printf("%s %d: failed %d \n", __FUNCTION__,__LINE__, ret);
	}
		

	clnt_destroy(clnt);
	return ret;

}

int rt_qcsqpi_get_net_device_stats(char *ifname, struct net_device_stats *nds)
{
	int ret=0;
	const char *host;
	CLIENT *clnt=NULL;
	qcsapi_interface_stats q_stats;

	int udp_retry_timeout=-1;

	host = get_host_addr();

	clnt = clnt_create(host, QCSAPI_PROG, QCSAPI_VERS, "udp");
	if (clnt == NULL) {
		clnt = clnt_create(host, QCSAPI_PROG, QCSAPI_VERS, "tcp");
	} else {
		if (udp_retry_timeout>0) {
			struct timeval value;
			value.tv_sec = (time_t)udp_retry_timeout;
			value.tv_usec = (suseconds_t)0;
			clnt_control(clnt, CLSET_RETRY_TIMEOUT, (char *)&value);
		}
	}

	if (clnt == NULL) {
		clnt_pcreateerror(host);
		return -1;
	}

	client_qcsapi_set_rpcclient(clnt);

	if((ret = qcsapi_get_interface_stats(ifname, &q_stats))!=0)
		printf("%s %d: failed %d \n", __FUNCTION__,__LINE__, ret);

	nds->tx_bytes = q_stats.tx_bytes;
	nds->tx_packets= q_stats.tx_pkts;
	nds->tx_errors= q_stats.tx_err;
	nds->tx_dropped= q_stats.tx_discard;
	
	nds->rx_bytes = q_stats.rx_bytes;
	nds->rx_packets= q_stats.rx_pkts;
	nds->rx_errors= q_stats.rx_err;
	nds->rx_dropped= q_stats.rx_discard;

	clnt_destroy(clnt);
	return ret;

}


int rt_init_qcsapi_rpc(CLIENT **ptr)
{
	const char *host;
	CLIENT *clnt=NULL;

	int udp_retry_timeout=-1;

	host = get_host_addr();

	clnt = clnt_create(host, QCSAPI_PROG, QCSAPI_VERS, "udp");
	if (clnt == NULL) {
		clnt = clnt_create(host, QCSAPI_PROG, QCSAPI_VERS, "tcp");
	} else {
		if (udp_retry_timeout>0) {
			struct timeval value;
			value.tv_sec = (time_t)udp_retry_timeout;
			value.tv_usec = (suseconds_t)0;
			clnt_control(clnt, CLSET_RETRY_TIMEOUT, (char *)&value);
		}
	}

	if (clnt == NULL) {
		clnt_pcreateerror(host);
		return -1;
	}

	client_qcsapi_set_rpcclient(clnt);

	*ptr = clnt;

	return 0;

}

void rt_exit_qcsapi_rpc(CLIENT **clnt)
{
	clnt_destroy(*clnt);
}

static void qtn_band(int band, char *mode)
{
	switch(band){
		case BAND_11B:
			snprintf(mode, sizeof(mode), "11b");
			break;
		case BAND_11G:
			snprintf(mode, sizeof(mode), "11g");
			break;
		case BAND_11A:
			snprintf(mode, sizeof(mode), "11a");
			break;
		//case BAND_11N:
		//	snprintf(mode, sizeof(mode), "11n");
		//	break;
		case (BAND_11G|BAND_11N):
			snprintf(mode, sizeof(mode), "11ng");
			break;
		case BAND_5G_11AN:
			snprintf(mode, sizeof(mode), "11na");
			break;
		case BAND_5G_11AC:
			snprintf(mode, sizeof(mode), "11ac");
			break;
	}
}

unsigned int qtn_chanwidth_value(int chanwidth)
{
	switch(chanwidth){
		case 0:
			return 20;
		case 1:
			return 40;
		case 2:
			return 80;
	}
	return 0;
}

int setup_WPS_qtn(char *ifname, int index)
{
	int ret=0;
	MIB_CE_MBSSIB_T Entry;
	char tmpbuf[128];
	mib_chain_get(MIB_MBSSIB_TBL, index, &Entry);
	if(Entry.enable1X || (Entry.encrypt >= WIFI_SEC_WPA && Entry.wpaAuth == WPA_AUTH_AUTO))
		return ret;
	
	if(Entry.wsc_disabled)
		qcsapi_wps_set_configured_state(ifname, 0);
	else if(!Entry.wsc_configured)
		qcsapi_wps_set_configured_state(ifname, 1);
	else
		qcsapi_wps_set_configured_state(ifname, 2);

	mib_get(MIB_WSC_PIN, (void *)tmpbuf);
	qcsapi_wps_set_ap_pin(ifname, tmpbuf);
	qcsapi_wps_save_ap_pin(ifname);
	
	return ret;
}

static void rt_get_qtn_regulatory_regions(int region_by_index, char *region_by_name)
{
	char regions_list[257];
	if(region_by_index==0)
		snprintf(region_by_name, 5, "none");
	else{
		qcsapi_regulatory_get_list_regulatory_regions(regions_list);
		snprintf(region_by_name, 3, "%s", (regions_list+((region_by_index-1)*3)));
	}
	
}

int setup_acl_qtn(char *ifname)
{
	int i, num, ret=0;
	unsigned char vChar;
	MIB_CE_WLAN_AC_T aclEntry;
	
	mib_get(MIB_WLAN_AC_ENABLED, (void *)&vChar);

	qcsapi_init();
	
	if(vChar == 0)
		qcsapi_wifi_set_mac_address_filtering(ifname, qcsapi_disable_mac_address_filtering);
	else if(vChar == 2)
		qcsapi_wifi_set_mac_address_filtering(ifname, qcsapi_accept_mac_address_unless_denied); //black list mode
	else if(vChar == 1)
		qcsapi_wifi_set_mac_address_filtering(ifname, qcsapi_deny_mac_address_unless_authorized); //white list mode


	if(vChar !=0){
		
		qcsapi_wifi_clear_mac_address_filters(ifname);
		
		num = mib_chain_total(MIB_WLAN_AC_TBL);
		
		for (i=0; i<num; i++) {
			if (!mib_chain_get(MIB_WLAN_AC_TBL, i, (void *)&aclEntry))
				continue;

			// acladdr
			if(vChar == 1) //white
				qcsapi_wifi_authorize_mac_address(ifname, aclEntry.macAddr);
			else //black
				qcsapi_wifi_deny_mac_address(ifname, aclEntry.macAddr);
		}
	}

	return ret;

}

int setupWLan_qtn(char *ifname, int index)
{
	CLIENT *clnt;
	unsigned int vInt;
	MIB_CE_MBSSIB_T Entry;
	unsigned short sInt;
	unsigned char vChar, regdomain, auto_chan;
	char *uPtr, *token;
	char parm[64], parm2[128], parm3[1025], parm4[8];
	int mib_id;
	struct in_addr ip_addr;
	char ip_str[32];
	//int ret;
	if(!mib_chain_get(MIB_MBSSIB_TBL, index, &Entry)){
		printf("Error! Get MIB_MBSSIB_TBL Entry failed\n");
		return -1;
	}
	
	if(rt_init_qcsapi_rpc(&clnt)<0)
		return -1;
	
	if(index==0){

		qcsapi_interface_get_status(ifname, parm);
		if((!strcmp(parm, "Up") && Entry.wlanDisabled) ||
			(!strcmp(parm, "Disabled") && !Entry.wlanDisabled))
				qcsapi_interface_enable(ifname, Entry.wlanDisabled? 0:1);
		if(Entry.wlanDisabled)
			goto exit_setup_wlan_qtn;
		
		mib_get(MIB_WLAN_BEACON_INTERVAL, &sInt);
		vInt = (unsigned int) sInt;
		qcsapi_wifi_set_beacon_interval(ifname, vInt);

		if(Entry.wlanMode == AP_MODE ||Entry.wlanMode == AP_WDS_MODE){
			qcsapi_config_update_parameter(ifname, "mode", "ap");
		}
		else if(Entry.wlanMode == CLIENT_MODE){
			qcsapi_config_update_parameter(ifname, "mode", "sta");
		}

#ifdef WLAN1_QTN
		mib_id = MIB_HW_WLAN1_REG_DOMAIN;
#else
		mib_id = MIB_HW_REG_DOMAIN;
#endif
		mib_get(mib_id, &regdomain);
		rt_get_qtn_regulatory_regions(regdomain,parm);
		//printf("region %s\n", parm);
		//qcsapi_config_update_parameter(ifname, "region", parm);
		//qcsapi_regulatory_set_regulatory_region(ifname, parm);

		mib_get(MIB_WLAN_CHAN_NUM, &vChar);
		mib_get(MIB_WLAN_AUTO_CHAN_ENABLED, &auto_chan);
		if(auto_chan) vChar = 0;
		//if(regdomain==0) //none
			qcsapi_wifi_set_channel(ifname, vChar);
		//else
		//	qcsapi_wifi_set_regulatory_channel(ifname, vChar, parm, 0);
		snprintf(parm, sizeof(parm), "%d", vChar);
		qcsapi_config_update_parameter(ifname, "channel", parm);

		mib_get(MIB_WLAN_BAND, &vChar);
		qtn_band(vChar, parm);
		//qcsapi_config_update_parameter(ifname, "band", parm);
		qcsapi_wifi_set_phy_mode(ifname, parm);
		
		if(vChar==BAND_5G_11AC)
			qcsapi_wifi_set_vht(ifname, 1);
		else
			qcsapi_wifi_set_vht(ifname, 0);
		
		qcsapi_wifi_set_bw(ifname, 20);	//prevent for invalid operation from 80->40MHz
		mib_get(MIB_WLAN_CHANNEL_WIDTH, &vChar);
		qcsapi_wifi_set_bw(ifname, qtn_chanwidth_value(vChar));

		mib_get(MIB_WLAN_DTIM_PERIOD, &vChar);
		vInt = (unsigned int)vChar;
		qcsapi_wifi_set_dtim(ifname, vInt);

		mib_get(MIB_WLAN_SHORTGI_ENABLED, &vChar);
		qcsapi_wifi_set_option(ifname, qcsapi_short_GI, (int)vChar);

		mib_get(MIB_WLAN_RTS_THRESHOLD, &sInt); 
		vInt = (unsigned int) sInt;
		qcsapi_wifi_set_rts_threshold(ifname, vInt);

		qcsapi_wifi_set_option(ifname, qcsapi_wmm, Entry.wmmEnabled? 1:0);


		if(Entry.rateAdaptiveEnabled)
			qcsapi_wifi_set_option(ifname, qcsapi_autorate_fallback, 1);
		else{
			//todo: mcs0~76, nssx-y: mcsx0y
			qcsapi_wifi_set_option(ifname, qcsapi_autorate_fallback, 0);
			snprintf(parm, sizeof(parm), "MCS%d", Entry.fixedTxRate);
			//qtn_mcs_rate(Entry.fixedTxRate, parm);
			qcsapi_wifi_set_mcs_rate(ifname, parm);
			//va_cmd(QCSAPI_SRPC, 3, 1, "set_mcs_rate", ifname, parm);
		}

	//	if(Entry.encrypt > WIFI_SEC_WPA && Entry.encrypt <= WIFI_SEC_WPA2_MIXED){
	//		sprintf(parm, sizeof(parm), "%lu", Entry.wpaGroupRekeyTime);
	//		va_cmd(QCSAPI_SRPC, 3, 1, "set_group_key_interval", ifname, parm);
	//	}

		setup_WPS_qtn(ifname, index);

	}

	if(index!=0){
		if(Entry.wlanDisabled){
			qcsapi_wifi_remove_bss(ifname);
			goto exit_setup_wlan_qtn;
		}
		else
			qcsapi_wifi_create_bss(ifname, NULL);
	}

	//snprintf(value, sizeof(value), "%s", (unsigned char *)Entry.ssid);
	uPtr = (char *)Entry.ssid;
	qcsapi_wifi_set_SSID(ifname, uPtr);
	qcsapi_wifi_set_option(ifname, qcsapi_SSID_broadcast, Entry.hidessid? 0:1);
	qcsapi_wifi_set_ap_isolate(ifname, Entry.userisolation? 1:0);

	//disable pmf
	qcsapi_wifi_set_pmf(ifname, 0);

	if(!qcsapi_wifi_get_radius_auth_server_cfg(ifname, parm3)){
		sscanf(parm3, "%s %s %*s", parm2, parm);
		qcsapi_wifi_del_radius_auth_server_cfg(ifname, parm2, parm);
	}

	if(Entry.encrypt == WIFI_SEC_NONE){ 
		qcsapi_wifi_set_beacon_type(ifname, "Basic");
		#if 0 //disable 8021x for none encryption
		if(Entry.enable1X){
			qcsapi_wifi_set_WPA_authentication_mode(ifname, "EAPAuthentication");
			//set_own_ip_addr //get eth1_0 or br0 ip call_qcsapi set_own_ip_addr wifi0 
			qcsapi_interface_get_ip4(BRIF, "ipaddr", parm2);
			qcsapi_wifi_set_own_ip_addr(ifname, parm2);
			//br0:0 ip , radius port + vwlan_idx <--> radius server ip, radius port
			snprintf(parm, sizeof(parm), "%u", Entry.rsPort);
			mib_get(MIB_ADSL_LAN_IP2, &ip_addr);
			snprintf(ip_str, sizeof(ip_str), "%s", inet_ntoa(ip_addr));
			snprintf(parm2, sizeof(parm2), "%d.%d.%d.%d", Entry.rsIpAddr[0],Entry.rsIpAddr[1],Entry.rsIpAddr[2],Entry.rsIpAddr[3]);
			uPtr = (char *)Entry.rsPassword;
			snprintf(parm4, sizeof(parm4), "%d", RADIUS_PORT+index);
			qcsapi_wifi_add_radius_auth_server_cfg(ifname, ip_str, parm4, uPtr);
			snprintf(parm3, sizeof(parm3), "ip_relay %s %s %s %d &", parm4, parm2, parm, index);
			system(parm3);
		}
		else
		#endif
			qcsapi_wifi_set_WPA_authentication_mode(ifname, "PSKAuthentication");
		qcsapi_wifi_set_WPA_encryption_modes(ifname, "AESEncryption");
	}
	else if(Entry.encrypt == WIFI_SEC_WPA2){
		qcsapi_wifi_set_beacon_type(ifname, "11i");
		qcsapi_wifi_set_WPA_encryption_modes(ifname, "AESEncryption");
		if(Entry.wpaAuth == WPA_AUTH_PSK){
			qcsapi_wifi_set_WPA_authentication_mode(ifname, "PSKAuthentication");
			uPtr = (char *)Entry.wpaPSK;
			if(Entry.wpaPSKFormat == KEY_ASCII)
				qcsapi_wifi_set_key_passphrase(ifname, 0, uPtr);
			else
				qcsapi_wifi_set_pre_shared_key(ifname, 0, uPtr);
		}
		else{
			qcsapi_wifi_set_WPA_authentication_mode(ifname, "EAPAuthentication");
			//set_own_ip_addr //get eth1_0 or br0 ip call_qcsapi set_own_ip_addr wifi0
			qcsapi_interface_get_ip4(BRIF, "ipaddr", parm2);
			qcsapi_wifi_set_own_ip_addr(ifname, parm2);
			//br0:0 ip , radius port + vwlan_idx <--> radius server ip, radius port
			snprintf(parm, sizeof(parm), "%u", Entry.rsPort);
			mib_get(MIB_ADSL_LAN_IP2, &ip_addr);
			snprintf(ip_str, sizeof(ip_str), "%s", inet_ntoa(ip_addr));
			snprintf(parm2, sizeof(parm2), "%d.%d.%d.%d", Entry.rsIpAddr[0],Entry.rsIpAddr[1],Entry.rsIpAddr[2],Entry.rsIpAddr[3]);
			uPtr = (char *)Entry.rsPassword;
			snprintf(parm4, sizeof(parm4), "%d", RADIUS_PORT+index);
			qcsapi_wifi_add_radius_auth_server_cfg(ifname, ip_str, parm4, uPtr);
			snprintf(parm3, sizeof(parm3), "ip_relay %s %s %s %d &", parm4, parm2, parm, index);
			system(parm3);
		}
	}
	else if(Entry.encrypt == WIFI_SEC_WPA2_MIXED){
		qcsapi_wifi_set_beacon_type(ifname, "WPAand11i");	
		qcsapi_wifi_set_WPA_encryption_modes(ifname, "TKIPandAESEncryption");
		if(Entry.wpaAuth == WPA_AUTH_PSK){
			qcsapi_wifi_set_WPA_authentication_mode(ifname, "PSKAuthentication");
			uPtr = (char *)Entry.wpaPSK;
			if(Entry.wpaPSKFormat == KEY_ASCII)
				qcsapi_wifi_set_key_passphrase(ifname, 0, uPtr);
			else
				qcsapi_wifi_set_pre_shared_key(ifname, 0, uPtr);
		}
		else{
			qcsapi_wifi_set_WPA_authentication_mode(ifname, "EAPAuthentication");
			//set_own_ip_addr //get eth1_0 or br0 ip call_qcsapi set_own_ip_addr wifi0 
			qcsapi_interface_get_ip4(BRIF, "ipaddr", parm2);
			qcsapi_wifi_set_own_ip_addr(ifname, parm2);
			//br0:0 ip , radius port + vwlan_idx <--> radius server ip, radius port
			snprintf(parm, sizeof(parm), "%u", Entry.rsPort);
			mib_get(MIB_ADSL_LAN_IP2, &ip_addr);
			snprintf(ip_str, sizeof(ip_str), "%s", inet_ntoa(ip_addr));
			snprintf(parm2, sizeof(parm2), "%d.%d.%d.%d", Entry.rsIpAddr[0],Entry.rsIpAddr[1],Entry.rsIpAddr[2],Entry.rsIpAddr[3]);
			uPtr = (char *)Entry.rsPassword;
			snprintf(parm4, sizeof(parm4), "%d", RADIUS_PORT+index);
			qcsapi_wifi_add_radius_auth_server_cfg(ifname, ip_str, parm4, uPtr);
			snprintf(parm3, sizeof(parm3), "ip_relay %s %s %s %d &", parm4, parm2, parm, index);
			system(parm3);
		}

	}

	setup_acl_qtn(ifname);
#ifdef WLAN_WDS
	setupWDS_qtn();
#endif

exit_setup_wlan_qtn:
	rt_exit_qcsapi_rpc(&clnt);

	return 0;
}
#if 0
#define PINGCOUNT	3
#define DEFDATALEN	56
#define	PINGINTERVAL	1	/* second */
#define MAXWAIT		5


static struct sockaddr_in pingaddr;
static int pingsock = -1;
static long ntransmitted = 0, nreceived = 0, nrepeats = 0;
static int myid = 0;
static int finished = 0;

static void pingfinal()
{
	finished = 1;
}

static void sendping()
{
	struct icmp *pkt;
	int c;
	char packet[DEFDATALEN + 8];

	pkt = (struct icmp *) packet;
	pkt->icmp_type = ICMP_ECHO;
	pkt->icmp_code = 0;
	pkt->icmp_cksum = 0;
	pkt->icmp_seq = ntransmitted++;
	pkt->icmp_id = myid;
	pkt->icmp_cksum = in_cksum((unsigned short *) pkt, sizeof(packet));

	c = sendto(pingsock, packet, sizeof(packet), 0,
			   (struct sockaddr *) &pingaddr, sizeof(struct sockaddr_in));

	if (c < 0 || c != sizeof(packet)) {
		ntransmitted--;
		finished = 1;
		//printf("sock: sendto fail !");
		return;
	}

	signal(SIGALRM, sendping);
	if (ntransmitted < PINGCOUNT) {	/* schedule next in 1s */
		alarm(PINGINTERVAL);
	} else {	/* done, wait for the last ping to come back */
		signal(SIGALRM, pingfinal);
		alarm(MAXWAIT);
	}
}

static int ping_function(char *str)
{
	char tmpBuf[100];
	int c;
	struct hostent *h;
	struct icmp *pkt;
	struct iphdr *iphdr;
	char packet[DEFDATALEN + 8];
	int rcvdseq;
	fd_set rset;
	struct timeval tv;
	int ret;

	if ((pingsock = create_icmp_socket()) < 0) {
		//perror("socket");
		//snprintf(tmpBuf, 100, "ping: socket create error");
		return -1;
	}

	memset(&pingaddr, 0, sizeof(struct sockaddr_in));

	pingaddr.sin_family = AF_INET;

	if ((h = gethostbyname(str)) == NULL) {
		//herror("ping: ");
		//snprintf(tmpBuf, 100, "ping: %s: %s", str, hstrerror(h_errno));
		return -1;
	}

	if (h->h_addrtype != AF_INET) {
		//strcpy(tmpBuf, "unknown address type; only AF_INET is currently supported.");
		return -1;
	}

	memcpy(&pingaddr.sin_addr, h->h_addr, sizeof(pingaddr.sin_addr));

	myid = getpid() & 0xFFFF;

	ntransmitted = nreceived = nrepeats = 0;
	finished = 0;
	rcvdseq=ntransmitted-1;
	FD_ZERO(&rset);
	FD_SET(pingsock, &rset);
	/* start the ping's going ... */
	sendping();

	/* listen for replies */
	while (1) {
		struct sockaddr_in from;
		socklen_t fromlen = (socklen_t) sizeof(from);
		int c, hlen, dupflag;

		if (finished){
			ret = -1;
			goto finished_sock;
		}

		tv.tv_sec = 1;
		tv.tv_usec = 0;

		if (select(pingsock+1, &rset, NULL, NULL, &tv) > 0) {
			if ((c = recvfrom(pingsock, packet, sizeof(packet), 0,
							  (struct sockaddr *) &from, &fromlen)) < 0) {
				if (errno == EINTR)
					continue;

				//printf("sock: recvfrom fail !");
				continue;
			}
		}
		else // timeout or error
			continue;

		if (c < DEFDATALEN+ICMP_MINLEN)
			continue;

		iphdr = (struct iphdr *) packet;
		hlen = iphdr->ihl << 2;
		pkt = (struct icmp *) (packet + hlen);	/* skip ip hdr */
		if (pkt->icmp_id != myid) {
//				printf("not myid\n");
			continue;
		}
		if (pkt->icmp_type == ICMP_ECHOREPLY) {
			++nreceived;
			if (pkt->icmp_seq == rcvdseq) {
				// duplicate
				++nrepeats;
				--nreceived;
				dupflag = 1;
			} else {
				rcvdseq = pkt->icmp_seq;
				dupflag = 0;
				if (nreceived < PINGCOUNT)
				// reply received, send another immediately
					sendping();
			}

		}
		if (nreceived >= PINGCOUNT){
			ret = 0;
			goto finished_sock;
		}
	}
finished_sock:
	close(pingsock);
	return ret;
}
#endif
void createQTN_rmt_svr_state(int state);

int ping_qtn_check()
{
	struct stat st;
	struct in_addr ip_addr;
	char ip_str[32];
	int ping_cnt=0, ping_ret;
	char tmpbuf[100];
	
	if (stat(QTN_STATE, &st) == 0 ) {// file exist
		unlink(QTN_STATE);
	}
	
	//check ip is reachable before setup
	mib_get(MIB_ADSL_LAN_IP2, &ip_addr);
	ip_addr.s_addr++;
	snprintf(ip_str, sizeof(ip_str), "%s", inet_ntoa(ip_addr));
	snprintf(tmpbuf, sizeof(tmpbuf), "ping -c 1 %s > /dev/null\n", ip_str);
	ping_ret = system(tmpbuf);
	
	while((ping_ret == -1 || (!WIFEXITED(ping_ret))|| WEXITSTATUS(ping_ret)!=0 ) && ping_cnt++ < 3){
		ping_ret = system(tmpbuf);
		printf("ping %s\n", ip_str);
		sleep(2);	
	}

	if(ping_ret != 0){
		printf("failed to ping %s\n", ip_str);
		return -1;
	}

	//check remote qcsapi server is reachable before setup
	ping_cnt=0;
	do
	{
		ping_ret = va_cmd("/bin/qcsapi_sockrpc", 1, 1, "get_firmware_version");
		sleep(2);
	} while(ping_ret != 0 && ping_cnt++ < 15);
	
	if(ping_ret != 0){
		printf("failed to connect to qtn wifi\n");
		return -1;
	}

	createQTN_rmt_svr_state(ping_ret);
	
	return 0;

}

void startWLan_qtn()
{
	int orig_idx=wlan_idx;
	int j=0;
	MIB_CE_MBSSIB_T Entry;
	char ifname[16];
	unsigned char wlan_mode;	
	struct stat st;
	#define QTNIMG_NANE "/tmp/topaz-linux.lzma.img"

#ifdef WLAN1_QTN
	wlan_idx = 1;
#else
	wlan_idx = 0;
#endif
	
	mib_chain_get(MIB_MBSSIB_TBL, 0, &Entry);

	snprintf(ifname, sizeof(ifname), "wifi%d", j);
	setupWLan_qtn(ifname, j);
	wlan_mode = Entry.wlanMode;
#ifdef WLAN_CLIENT
	if(wlan_mode == CLIENT_MODE)
		rt_qcsapi_set_STA_config(ifname);
#endif

#ifdef WLAN_MBSSID
	if (wlan_mode == AP_MODE || wlan_mode == AP_WDS_MODE) {
		for (j=1; j<=WLAN_MBSSID_NUM; j++){
			mib_chain_get(MIB_MBSSIB_TBL, j, &Entry);
			snprintf(ifname, sizeof(ifname), "wifi%d", j);
			setupWLan_qtn(ifname, j);
		}
	}
#endif

	if (stat(QTNIMG_NANE, &st) == 0 )// file exist
		unlink(QTNIMG_NANE);

	wlan_idx=orig_idx;
}
int stopwlan_qtn()
{
	int ip_relay_pid, i;
	char pidfile[64];
	for(i=0; i<NUM_VWLAN_INTERFACE;i++){
		snprintf(pidfile, sizeof(pidfile), "/var/run/ip_relay-%d.pid", i);
		ip_relay_pid = read_pid(pidfile);
		if(ip_relay_pid > 0) {
			kill(ip_relay_pid, 9);
			unlink(pidfile);
		}
	}

	return 0;
}

void createQTN_targetIPconf()
{
	FILE *fp;
	struct in_addr ip_addr;

	fp = fopen("/var/qcsapi_target_ip.conf", "w");
	mib_get(MIB_ADSL_LAN_IP2, &ip_addr);
	ip_addr.s_addr++;
	
	if(fp) {
		fprintf(fp, "%s\n", inet_ntoa(ip_addr));
		fclose(fp);
	}

	return;
}
void createQTN_rmt_svr_state(int state)
{
	FILE *fp;

	fp = fopen(QTN_STATE, "w");
	
	if(fp) {
		fprintf(fp, "%d\n", state);
		fclose(fp);
	}
}


void rt_report_qtn_button_state()
{
	char tmpbuf;
	FILE *fp;
	char line[20];
	int pState=0;
	int ret;
	MIB_CE_MBSSIB_T mEntry;
	int orig_wlan_idx=wlan_idx;
#ifdef WLAN1_QTN
	wlan_idx=1;
#else
	wlan_idx=0;
#endif
	mib_chain_get(MIB_MBSSIB_TBL, 0, &mEntry);
	if(mEntry.wlanDisabled || mEntry.wsc_disabled || mEntry.enable1X || (mEntry.encrypt >= WIFI_SEC_WPA && mEntry.wpaAuth == WPA_AUTH_AUTO))
		goto end;

	if ((fp = fopen("/proc/gpio", "r")) != NULL) {
			fgets(line, sizeof(line), fp);
			if (sscanf(line, "%c", &tmpbuf)) {
					if (tmpbuf == '0')
							pState = 0;
					else if(tmpbuf == '1')
							pState = 1;
			}
			else
					pState = 0;
			fclose(fp);
	}
	else
			pState = 0;

	if(pState==1){
		ret = va_cmd("/bin/qcsapi_sockrpc", 2, 1, "registrar_report_pbc", WIFINAME);
		if(!ret)
			printf("Start QTN WPS\n");
	}
end:
	wlan_idx = orig_wlan_idx;

}

int rt_get_qtn_wps_configured_settings()
{
	CLIENT *clnt;
	char tmpbuf[256];
	MIB_CE_MBSSIB_T Entry;

	if(rt_init_qcsapi_rpc(&clnt)<0)
		return -1;
	
	if(!mib_chain_get(MIB_MBSSIB_TBL, 0, &Entry)){
		printf("Error! Get MIB_MBSSIB_TBL Entry failed\n");
		rt_exit_qcsapi_rpc(&clnt);
		return -1;
	}

	memset(tmpbuf, 0, sizeof(tmpbuf));
	qcsapi_wifi_get_beacon_type(WIFINAME, tmpbuf);
	if(!strcmp(tmpbuf, "WPAand11i")){
		Entry.encrypt = WIFI_SEC_WPA2_MIXED;
		Entry.wsc_auth = WSC_AUTH_WPA2PSKMIXED;
	}
	else if(!strcmp(tmpbuf, "11i")){
		Entry.encrypt = WIFI_SEC_WPA2;
		Entry.wsc_auth = WSC_AUTH_WPA2PSK;
	}

	memset(tmpbuf, 0, sizeof(tmpbuf));
	qcsapi_wifi_get_WPA_encryption_modes(WIFINAME, tmpbuf);
	if(!strcmp(tmpbuf, "TKIPandAESEncryption")){
		Entry.unicastCipher = WPA_CIPHER_MIXED;
		Entry.wpa2UnicastCipher = WPA_CIPHER_MIXED;
		Entry.wsc_enc = WSC_ENCRYPT_TKIPAES;
	}
	else if(!strcmp(tmpbuf, "AESEncryption")){
		Entry.wpa2UnicastCipher = WPA_CIPHER_AES;
		Entry.wsc_enc = WSC_ENCRYPT_AES;
	}
	
	memset(tmpbuf, 0, sizeof(tmpbuf));
	qcsapi_wifi_get_WPA_authentication_mode(WIFINAME, tmpbuf);
	if(!strcmp(tmpbuf, "PSKAuthentication")){
		Entry.wpaAuth= WPA_AUTH_PSK;
	}

	memset(tmpbuf, 0, sizeof(tmpbuf));
	Entry.wpaPSKFormat = KEY_ASCII;
	if(qcsapi_wifi_get_key_passphrase(WIFINAME, 0, tmpbuf)<0){
		Entry.wpaPSKFormat = KEY_HEX;
		qcsapi_wifi_get_pre_shared_key(WIFINAME, 0, tmpbuf);
	}
	
	memset(Entry.wpaPSK, '\0', sizeof(Entry.wpaPSK));
	memcpy(Entry.wpaPSK, tmpbuf, strlen(tmpbuf));
	
	memset(Entry.wscPsk, '\0', sizeof(Entry.wscPsk));
	memcpy(Entry.wscPsk, tmpbuf, strlen(tmpbuf));

	sleep(1); //wait for ssid update
	memset(tmpbuf, 0, sizeof(tmpbuf));
	qcsapi_wifi_get_SSID(WIFINAME, tmpbuf);
	memset(Entry.ssid, '\0', sizeof(Entry.ssid));
	memcpy(Entry.ssid, tmpbuf, strlen(tmpbuf));

	Entry.wsc_configured = 1;
	
	
	mib_chain_update(MIB_MBSSIB_TBL, &Entry, 0);

	extern void update_wps_mib();
	update_wps_mib();
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

	rt_exit_qcsapi_rpc(&clnt);

	return 0;
		
}

int rt_qcsapi_wps_get_configured_state()
{
	
	int ret=0;
	const char *host;
	CLIENT *clnt=NULL;
	char wps_state[64];

	int udp_retry_timeout=-1;

	host = get_host_addr();

	clnt = clnt_create(host, QCSAPI_PROG, QCSAPI_VERS, "udp");
	if (clnt == NULL) {
		clnt = clnt_create(host, QCSAPI_PROG, QCSAPI_VERS, "tcp");
	} else {
		if (udp_retry_timeout>0) {
			struct timeval value;
			value.tv_sec = (time_t)udp_retry_timeout;
			value.tv_usec = (suseconds_t)0;
			clnt_control(clnt, CLSET_RETRY_TIMEOUT, (char *)&value);
		}
	}

	if (clnt == NULL) {
		clnt_pcreateerror(host);
		return -1;
	}

	client_qcsapi_set_rpcclient(clnt);

	if((ret = qcsapi_wps_get_configured_state(WIFINAME, wps_state, 63))!=0)
		printf("%s %d: failed %d \n", __FUNCTION__,__LINE__, ret);

	clnt_destroy(clnt);

	if(!strcmp(wps_state, "configured"))
		return 1;
	return -1;
}

void rt_check_qtn_wps_config()
{
	int orig_wlan_idx= wlan_idx;
	FILE *fp;
	char line[20];
	char tmpbuf;
	MIB_CE_MBSSIB_T mEntry;
#ifdef WLAN1_QTN
	wlan_idx=1;
#else
	wlan_idx=0;
#endif
	mib_chain_get(MIB_MBSSIB_TBL, 0, &mEntry);
	if(mEntry.wlanDisabled || mEntry.wsc_disabled || mEntry.wsc_configured|| mEntry.enable1X || (mEntry.encrypt >= WIFI_SEC_WPA && mEntry.wpaAuth == WPA_AUTH_AUTO))
		goto end;

	if((fp = fopen(QTN_STATE, "r")) != NULL){
		fgets(line, sizeof(line), fp);
		if (sscanf(line, "%c", &tmpbuf)) {
			if (tmpbuf == '0'){
				if(!mEntry.wlanDisabled){
					if(!mEntry.wsc_disabled && !mEntry.wsc_configured){
						//printf("%s %d\n", __FUNCTION__, __LINE__);
						if(rt_qcsapi_wps_get_configured_state()==1){
							rt_get_qtn_wps_configured_settings();
						}
					}
				}
				
			}
		}
		fclose(fp);
	}

end:
	wlan_idx = orig_wlan_idx;
}

#if defined(WLAN_CLIENT) || defined(WLAN_SITESURVEY)
int rt_qcsapi_get_properties_AP(char *ifname, SS_STATUS_Tp pStatus)
{
	unsigned int count, i;
	CLIENT *clnt;
	int ret=0;
	qcsapi_ap_properties ap_properties;
	
	if(rt_init_qcsapi_rpc(&clnt)<0)
		return -1;
	
	if((ret = qcsapi_wifi_get_results_AP_scan(ifname, &count))<0)
		goto end;

	pStatus->number = count;

	for(i=0; i<count; i++){
		qcsapi_wifi_get_properties_AP(ifname, i, &ap_properties);
		memcpy(pStatus->bssdb[i].bdBssId, ap_properties.ap_mac_addr, MAC_ADDR_SIZE);
		memcpy(pStatus->bssdb[i].bdSsIdBuf, ap_properties.ap_name_SSID, QCSAPI_SSID_MAXLEN); 
		pStatus->bssdb[i].bdSsId.Length = strlen(ap_properties.ap_name_SSID);
		pStatus->bssdb[i].network = ((ap_properties.ap_80211_proto&0x10)? ((ap_properties.ap_80211_proto&0xf)|0x40) : ap_properties.ap_80211_proto);
		pStatus->bssdb[i].ChannelNumber = ap_properties.ap_channel;
		pStatus->bssdb[i].rssi = ap_properties.ap_RSSI;
		if(ap_properties.ap_flags & qcsapi_ap_security_enabled){
			pStatus->bssdb[i].bdCap |= cPrivacy;
			if(ap_properties.ap_encryption_modes & qcsapi_ap_TKIP_encryption_mask){//TKIP
				if(ap_properties.ap_authentication_mode == qcsapi_ap_PSK_authentication) //PSK
					pStatus->bssdb[i].bdTstamp[0] |= (0x4 << 12);
				else //EAP
					pStatus->bssdb[i].bdTstamp[0] |= (0x2 << 12);
				
				if(ap_properties.ap_protocol & qcsapi_protocol_WPA_mask) // WPA
					pStatus->bssdb[i].bdTstamp[0] |= (0x1 << 8);
				if(ap_properties.ap_protocol & qcsapi_protocol_11i_mask) // WPA2
					pStatus->bssdb[i].bdTstamp[0] |= (0x1 << 24);
				
			}
			if(ap_properties.ap_encryption_modes & qcsapi_ap_CCMP_encryption_mask){//CCMP
				if(ap_properties.ap_authentication_mode == qcsapi_ap_PSK_authentication) //PSK
					pStatus->bssdb[i].bdTstamp[0] |= (0x4 << 28);
				else //EAP
					pStatus->bssdb[i].bdTstamp[0] |= (0x2 << 28);
					
				if(ap_properties.ap_protocol & qcsapi_protocol_WPA_mask) // WPA
					pStatus->bssdb[i].bdTstamp[0] |= (0x4 << 8);
				if(ap_properties.ap_protocol & qcsapi_protocol_11i_mask) // WPA2
					pStatus->bssdb[i].bdTstamp[0] |= (0x4 << 24);
			}
		}	
	}

end:
	rt_exit_qcsapi_rpc(&clnt);
	return ret;
}

#define DEFAULT_SSID_LIST_SIZE	2
#define MAX_SSID_LIST_SIZE	10

int rt_qcsapi_clear_SSID_list(char *ifname)
{
	unsigned int		 iter;
	static qcsapi_SSID	 array_ssids[MAX_SSID_LIST_SIZE];
	qcsapi_unsigned_int	 sizeof_list = DEFAULT_SSID_LIST_SIZE;
	char			*list_ssids[MAX_SSID_LIST_SIZE + 1];
	int ret;

	for (iter = 0; iter < sizeof_list; iter++) {
		list_ssids[iter] = array_ssids[iter];
		*(list_ssids[iter]) = '\0';
	}

	ret = qcsapi_SSID_get_SSID_list(ifname, sizeof_list, &list_ssids[0]);

	if(!ret){
		for (iter = 0; iter < sizeof_list; iter++) {
				if ((list_ssids[iter] == NULL) || strlen(list_ssids[iter]) < 1) {
					break;
				}
				qcsapi_SSID_remove_SSID(ifname, list_ssids[iter]);
			}
	}
	return ret;
}

int rt_qcsapi_set_STA_config(char *ifname)
{
	CLIENT *clnt;
	int ret=0;
	MIB_CE_MBSSIB_T Entry;

	if(!mib_chain_get(MIB_MBSSIB_TBL, 0, &Entry)){
		printf("Error! Get MIB_MBSSIB_TBL Entry failed\n");
		return -1;
	}

	if(rt_init_qcsapi_rpc(&clnt)<0)
		return -1;

	rt_qcsapi_clear_SSID_list(ifname);

	if(qcsapi_SSID_verify_SSID(ifname, (char *)Entry.ssid )!=0){
		qcsapi_SSID_create_SSID(ifname, (char *)Entry.ssid);
	}

	if(Entry.encrypt == WIFI_SEC_NONE)
		qcsapi_SSID_set_authentication_mode(ifname, (char *)Entry.ssid, "NONE");
	else{
		if(Entry.encrypt == WIFI_SEC_WPA2){
			qcsapi_SSID_set_protocol(ifname, (char *)Entry.ssid, "11i");
			if(Entry.unicastCipher==WPA_CIPHER_TKIP)
				qcsapi_SSID_set_encryption_modes(ifname, (char *)Entry.ssid, "TKIPEncryption");
			else if(Entry.unicastCipher==WPA_CIPHER_AES)
				qcsapi_SSID_set_encryption_modes(ifname, (char *)Entry.ssid, "AESEncryption");
			else if(Entry.unicastCipher==WPA_CIPHER_MIXED)
				qcsapi_SSID_set_encryption_modes(ifname, (char *)Entry.ssid, "TKIPandAESEncryption");
		}
		else if(Entry.encrypt == WIFI_SEC_WPA){
			qcsapi_SSID_set_protocol(ifname, (char *)Entry.ssid, "WPA");
			if(Entry.wpa2UnicastCipher==WPA_CIPHER_TKIP)
				qcsapi_SSID_set_encryption_modes(ifname, (char *)Entry.ssid, "TKIPEncryption");
			else if(Entry.wpa2UnicastCipher==WPA_CIPHER_AES)
				qcsapi_SSID_set_encryption_modes(ifname, (char *)Entry.ssid, "AESEncryption");
			else if(Entry.wpa2UnicastCipher==WPA_CIPHER_MIXED)
				qcsapi_SSID_set_encryption_modes(ifname, (char *)Entry.ssid, "TKIPandAESEncryption");
		}
		else if(Entry.encrypt == WIFI_SEC_WPA2_MIXED){
			qcsapi_SSID_set_protocol(ifname, (char *)Entry.ssid, "WPAand11i");
			qcsapi_SSID_set_encryption_modes(ifname, (char *)Entry.ssid, "TKIPandAESEncryption");
		}
		if(Entry.wpaAuth == WPA_AUTH_PSK){

			qcsapi_SSID_set_authentication_mode(ifname, (char *)Entry.ssid, "PSKAuthentication");
			if(Entry.wpaPSKFormat==KEY_ASCII)
				qcsapi_SSID_set_key_passphrase(ifname, (char *)Entry.ssid, 0, (char *)Entry.wpaPSK);
			else
				qcsapi_SSID_set_pre_shared_key(ifname, (char *)Entry.ssid, 0, (char *)Entry.wpaPSK);
		}

		
		
	}
	

	rt_exit_qcsapi_rpc(&clnt);
	return ret;
	
}

int rt_get_SSID_encryption(char *interface, char *ssid, char *buffer)
{
	string_16	 SSID_proto;
	char *ifname = rt_get_qtn_ifname(interface);
	int ret;
	const char *host;
	CLIENT *clnt=NULL;

	int udp_retry_timeout=-1;

	host = get_host_addr();

	clnt = clnt_create(host, QCSAPI_PROG, QCSAPI_VERS, "udp");
	if (clnt == NULL) {
		clnt = clnt_create(host, QCSAPI_PROG, QCSAPI_VERS, "tcp");
	} else {
		if (udp_retry_timeout>0) {
			struct timeval value;
			value.tv_sec = (time_t)udp_retry_timeout;
			value.tv_usec = (suseconds_t)0;
			clnt_control(clnt, CLSET_RETRY_TIMEOUT, (char *)&value);
		}
	}

	if (clnt == NULL) {
		clnt_pcreateerror(host);
		return -1;
	}

	client_qcsapi_set_rpcclient(clnt);
	
	ret = qcsapi_SSID_get_protocol(ifname, ssid, &SSID_proto[0]);
	if(ret < 0)
		strcpy(buffer, wlan_encrypt[0]);
	else if(!strcmp(SSID_proto, "WPA"))
		strcpy(buffer, wlan_encrypt[2]);
	else if(!strcmp(SSID_proto, "11i"))
		strcpy(buffer, wlan_encrypt[3]);
	else if(!strcmp(SSID_proto, "WPAand11i"))
		strcpy(buffer, wlan_encrypt[4]);

	clnt_destroy(clnt);
	
	return ret;
}
#endif
#ifdef WLAN_WDS
int setupWDS_qtn()
{
	int i;
	unsigned char macaddr[6];
	char vChar, wds_enabled, wds_num;
	WDS_T Entry;
	unsigned char value[128];
	
	for(i=0; i<MAX_WDS_NUM; i++){
		if(qcsapi_wds_get_peer_address(WIFINAME, i, macaddr)==0)
			qcsapi_wds_remove_peer(WIFINAME, macaddr);
	}

	mib_get(MIB_WLAN_MODE, (void *)&vChar);
	mib_get(MIB_WLAN_WDS_ENABLED, (void *)&wds_enabled);
	if (vChar != AP_WDS_MODE || wds_enabled == 0)
		return 0;
	
	mib_get(MIB_WLAN_WDS_NUM, &wds_num);
	mib_get(MIB_WLAN_WDS_ENCRYPT, &vChar);
	mib_get(MIB_WLAN_WDS_PSK, (void *)value);
	for(i=0; i<wds_num; i++){
		if (!mib_chain_get(MIB_WDS_TBL, i, (void *)&Entry))
			continue;
		
		qcsapi_wds_add_peer(WIFINAME, Entry.macAddr);
		if(vChar == WDS_ENCRYPT_DISABLED)
			qcsapi_wds_set_psk(WIFINAME, Entry.macAddr, NULL);
		else
			qcsapi_wds_set_psk(WIFINAME, Entry.macAddr, (char *)value);
	}

	return 0;
	
}

#endif

