#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#include "debug.h"
#include "utility.h"
#include "wireless.h"
//#include "subr_wlan.h"
#ifdef WLAN_FAST_INIT
#include "../../../linux-2.6.x/drivers/net/wireless/rtl8192cd/ieee802_mib.h"
#endif

#if defined(CONFIG_SLAVE_WLAN1_ENABLE) && !defined(CONFIG_MASTER_WLAN0_ENABLE)
const char* WLANIF[] = {"wlan1"};
#else
const char* WLANIF[] = {"wlan0", "wlan1"};
#endif

const char IWPRIV[] = "/bin/iwpriv";
const char AUTH[] = "/bin/auth";
const char IWCONTROL[] = "/bin/iwcontrol";
const char AUTH_PID[] = "/var/run/auth-wlan0.pid";

#ifdef WLAN_11R
const char FT_DAEMON_PROG[]	= "/bin/ftd";
const char FT_WLAN_IF[]		= "wlan0";
const char FT_CONF[]		= "/tmp/ft.conf";
const char FT_PID[]			= "/var/run/ft.pid";
#endif

#define IWCONTROLPID  "/var/run/iwcontrol.pid"
#if defined(CONFIG_RTL_92D_DMDP) || defined(WLAN_DUALBAND_CONCURRENT)
#define WLAN0_WLAN1_WSCDPID  "/var/run/wscd-wlan0-wlan1.pid"
#endif //CONFIG_RTL_92D_DMDP || WLAN_DUALBAND_CONCURRENT
#define WSCDPID "/var/run/wscd-%s.pid"
const char WSCD_FIFO[] = "/var/wscd-%s.fifo";
const char WSCD_CONF[] = "/var/wscd.conf";
#ifdef WLAN_WDS
const char WDSIF[]="%s-wds0";
#endif
#ifdef CONFIG_USER_FON
const char FONIF[] = "tun0";
#endif
#ifdef CONFIG_LUNA_DUAL_LINUX
const char WLAN1_MAC_FILE[]= "/var/wlan1_mac";
#endif


int wlan_idx=0;	// interface index

static unsigned int useAuth_RootIf=0;
static int wlan_num=0;
/* 2010-10-27 krammer :  change to 16 for dual band*/
static char para_iwctrl[16][20];
static int is8021xEnabled(int vwlan_idx);

const char *wlan_band[] = {
	0, "2.4 GHz (B)", "2.4 GHz (G)", "2.4 GHz (B+G)", 0
	, 0, 0, 0, "2.4 GHz (N)", 0, "2.4 GHz (G+N)", "2.4 GHz (B+G+N)", 0
};

const char *wlan_mode[] = {
	//"AP", "Client", "AP+WDS"
	"AP", "Client", "WDS", "AP+WDS"
};

const char *wlan_rate[] = {
	"1M", "2M", "5.5M", "11M", "6M", "9M", "12M", "18M", "24M", "36M", "48M", "54M"
	, "MCS0", "MCS1", "MCS2", "MCS3", "MCS4", "MCS5", "MCS6", "MCS7", "MCS8", "MCS9", "MCS10", "MCS11", "MCS12", "MCS13", "MCS14", "MCS15"
};

const char *wlan_auth[] = {
	"Open", "Shared", "Auto"
};

const char *wlan_preamble[] = {
	"Long", "Short"
};

const char *wlan_encrypt[] = {
	"None",
	"WEP",
	"WPA",
	"WPA2",
	"WPA2 Mixed",
#ifdef CONFIG_RTL_WAPI_SUPPORT
	"WAPI",
#else
	"",
#endif
};

const char *wlan_pskfmt[] = {
	"Passphrase", "Hex"
};

const char *wlan_wepkeylen[] = {
	"Disable", "64-bit", "128-bit"
};

const char *wlan_wepkeyfmt[] = {
	"ASCII", "Hex"
};

// Mason Yu. 201009_new_security
const char *wlan_Cipher[] = {
	//"TKIP", "AES", "Both"
	"TKIP", "AES", "TKIP+AES"
};

int getTxPowerScale(int mode)
{
	switch (mode)
	{
		case 0: //100%
			return 0;
		case 1: //70%
			return 3;
		case 2: //50%
			return 6;
		case 3: //35%
			return 9;
		case 4: //15%
			return 17;
	}
}
#ifdef CONFIG_LUNA_DUAL_LINUX
static void get_wlan_mac(char *fname, char *strbuf)
{
	FILE *fp;
	int i=0;
	int temp;
	unsigned char mac[MAC_ADDR_LEN];
	fp = fopen(fname, "r");
	if(fp) {
		for(i=0;i<MAC_ADDR_LEN;i++){
			fscanf(fp, "%x", &temp);
			mac[i] = temp;
		}
		printf("%s %02x%02x%02x%02x%02x%02x\n", __FUNCTION__, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		sprintf(strbuf, "%02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		fclose(fp);
	}
	return;
}
#endif

#if ((defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_RTL_8812_SUPPORT)) && !defined(CONFIG_ENABLE_EFUSE)) || defined(WLAN_DUALBAND_CONCURRENT)

#define B1_G1	40
#define B1_G2	48

#define B2_G1	56
#define B2_G2	64

#define B3_G1	104
#define B3_G2	112
#define B3_G3	120
#define B3_G4	128
#define B3_G5	136
#define B3_G6	144

#define B4_G1	153
#define B4_G2	161
#define B4_G3	169
#define B4_G4	177

void assign_diff_AC(unsigned char* pMib, unsigned char* pVal)
{
/*
	int i;
	printf("%s ", __FUNCTION__);
	for(i=0; i<14;i++)
		printf("%02x ", pVal[i]);
	printf("\n");
*/
	memset(pMib, 0, 35);
	memset((pMib+35), pVal[0], (B1_G1-35));
	memset((pMib+B1_G1), pVal[1], (B1_G2-B1_G1));
	memset((pMib+B1_G2), pVal[2], (B2_G1-B1_G2));
	memset((pMib+B2_G1), pVal[3], (B2_G2-B2_G1));
	memset((pMib+B2_G2), pVal[4], (B3_G1-B2_G2));
	memset((pMib+B3_G1), pVal[5], (B3_G2-B3_G1));
	memset((pMib+B3_G2), pVal[6], (B3_G3-B3_G2));
	memset((pMib+B3_G3), pVal[7], (B3_G4-B3_G3));
	memset((pMib+B3_G4), pVal[8], (B3_G5-B3_G4));
	memset((pMib+B3_G5), pVal[9], (B3_G6-B3_G5));
	memset((pMib+B3_G6), pVal[10], (B4_G1-B3_G6));
	memset((pMib+B4_G1), pVal[11], (B4_G2-B4_G1));
	memset((pMib+B4_G2), pVal[12], (B4_G3-B4_G2));
	memset((pMib+B4_G3), pVal[13], (B4_G4-B4_G3));
/*
	for(i=0; i<178;i++)
		printf("%02x", pMib[i]);
	printf("\n");
*/
}
#endif
enum {IWPRIV_GETMIB=1, IWPRIV_HS=2, IWPRIV_INT=4, IWPRIV_HW2G=8, IWPRIV_TXPOWER=16};
int iwpriv_cmd(int type, ...)
{
	va_list ap;
	int k=0, i;
	char *s, *s2;
	char *argv[24];
	int status;
	unsigned char value[196];
	char parm[1024];
	unsigned char pMib[178];
	int mib_id, mode, intVal;

	TRACE(STA_SCRIPT, "%s ", IWPRIV);
	va_start(ap, type);
	
	s = va_arg(ap, char *); //wlan interface name
	argv[++k] = s;
	TRACE(STA_SCRIPT|STA_NOTAG, "%s ", s); 
	s = va_arg(ap, char *); //cmd, ie set_mib
	argv[++k] = s;
	TRACE(STA_SCRIPT|STA_NOTAG, "%s ", s);
	s = va_arg(ap, char *); //cmd detail

	if(type & IWPRIV_GETMIB){
		mib_id = va_arg(ap, int);
		mib_get(mib_id, (void *)value);
	}
	else{
		if(type & IWPRIV_HS){
			if(type & IWPRIV_HW2G)
				memcpy(value, va_arg(ap, char *), MAX_CHAN_NUM);
#if ((defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_RTL_8812_SUPPORT)) && !defined(CONFIG_ENABLE_EFUSE)) || defined(WLAN_DUALBAND_CONCURRENT)
			else
				memcpy(value, va_arg(ap, char *), MAX_5G_CHANNEL_NUM);
#endif
		}
	}

	if(!(type & IWPRIV_HS)){
		if(type & IWPRIV_GETMIB){
			if(type & IWPRIV_INT) //int
				snprintf(parm, sizeof(parm), "%s=%u", s, value[0]);
			else //string
				snprintf(parm, sizeof(parm), "%s=%s", s, value);
		}
		else{
			if(type & IWPRIV_INT){ //int
				intVal = va_arg(ap, int);
				snprintf(parm, sizeof(parm), "%s=%u", s, intVal);
			}
			else{ //string
				s2 = va_arg(ap, char *);
				snprintf(parm, sizeof(parm), "%s=%s", s, s2);
			}
		}
	}
	else{
		snprintf(parm, sizeof(parm), "%s=", s);
		if(type & IWPRIV_HW2G){ //2G
			if(type & IWPRIV_TXPOWER){
				mode = va_arg(ap, int);
				intVal = getTxPowerScale(mode);
				for(i=0; i<MAX_CHAN_NUM; i++) {
					if(value[i]!=0){
						if((value[i] - intVal)>=1)
							value[i] -= intVal;
						else
							value[i] = 1;
					}
					snprintf(parm, sizeof(parm), "%s%02x", parm, value[i]);
				}
			}
			else{
				for(i=0; i<MAX_CHAN_NUM; i++) {
					snprintf(parm, sizeof(parm), "%s%02x", parm, value[i]);
				}
			}
			
		}
#if ((defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_RTL_8812_SUPPORT)) && !defined(CONFIG_ENABLE_EFUSE)) || defined(WLAN_DUALBAND_CONCURRENT)
		else{ //5G

			if(type & IWPRIV_TXPOWER){
				mode = va_arg(ap, int);
				intVal = getTxPowerScale(mode);
				for(i=0; i<MAX_5G_CHANNEL_NUM; i++) {
					if(value[i]!=0){
						if((value[i] - intVal)>=1)
							value[i] -= intVal;
						else
							value[i] = 1;
					}
					snprintf(parm, sizeof(parm), "%s%02x", parm, value[i]);
				}	
			}
			else{
				assign_diff_AC(pMib, value);
				for(i=0; i<=177; i++) {
					snprintf(parm, sizeof(parm), "%s%02x", parm, pMib[i]);
				}
			}
				
			
		}
#endif
	}
	argv[++k] = parm;
	TRACE(STA_SCRIPT|STA_NOTAG, "%s ", argv[k]);
	TRACE(STA_SCRIPT|STA_NOTAG, "\n");
	argv[k+1] = NULL;
	status = do_cmd(IWPRIV, argv, 1);
	va_end(ap);

	return status;
}

#if 0
// Mason Yu. 201009_new_security
void MBSSID_GetRootEntry(MIB_CE_MBSSIB_T *Entry) {
	//Entry->idx = 0;

	mib_get(MIB_WLAN_ENCRYPT, &Entry->encrypt);
	mib_get(MIB_WLAN_ENABLE_1X, &Entry->enable1X);
	mib_get(MIB_WLAN_WEP, &Entry->wep);
	mib_get(MIB_WLAN_WPA_AUTH, &Entry->wpaAuth);
	mib_get(MIB_WLAN_WPA_PSK_FORMAT, &Entry->wpaPSKFormat);
	mib_get(MIB_WLAN_WPA_PSK, Entry->wpaPSK);
	mib_get(MIB_WLAN_RS_PORT, &Entry->rsPort);
	mib_get(MIB_WLAN_RS_IP, Entry->rsIpAddr);

	mib_get(MIB_WLAN_RS_PASSWORD, Entry->rsPassword);
	mib_get(MIB_WLAN_DISABLED, &Entry->wlanDisabled);
	mib_get(MIB_WLAN_SSID, Entry->ssid);
	mib_get(MIB_WLAN_MODE, &Entry->wlanMode);
	mib_get(MIB_WLAN_AUTH_TYPE, &Entry->authType);
	//added by xl_yue
	// Mason Yu. 201009_new_security
	mib_get( MIB_WLAN_WPA_CIPHER_SUITE, &Entry->unicastCipher);
	mib_get( MIB_WLAN_WPA2_CIPHER_SUITE, &Entry->wpa2UnicastCipher);
	mib_get( MIB_WLAN_WPA_GROUP_REKEY_TIME, &Entry->wpaGroupRekeyTime);

#ifdef CONFIG_RTL_WAPI_SUPPORT
	mib_get( MIB_WLAN_WAPI_PSK, Entry->wapiPsk);
	mib_get( MIB_WLAN_WAPI_PSKLEN, &Entry->wapiPskLen);
	mib_get( MIB_WLAN_WAPI_PSK_FORMAT, &Entry->wapiPskFormat);
	mib_get( MIB_WLAN_WAPI_AUTH, &Entry->wapiAuth);
	mib_get( MIB_WLAN_WAPI_ASIPADDR, Entry->wapiAsIpAddr);
	//mib_get( MIB_WLAN_WAPI_SEARCH_CERTINFO, Entry->wapiSearchCertInfo);
	//mib_get( MIB_WLAN_WAPI_SEARCH_CERTINDEX, &Entry->wapiSearchIndex);
	//mib_get( MIB_WLAN_WAPI_MCAST_REKEYTYPE, &Entry->wapiMcastkey);
	//mib_get( MIB_WLAN_WAPI_MCAST_TIME, &Entry->wapiMcastRekeyTime);
	//mib_get( MIB_WLAN_WAPI_MCAST_PACKETS, &Entry->wapiMcastRekeyPackets);
	//mib_get( MIB_WLAN_WAPI_UCAST_REKETTYPE, &Entry->wapiUcastkey);
	//mib_get( MIB_WLAN_WAPI_UCAST_TIME, &Entry->wapiUcastRekeyTime);
	//mib_get( MIB_WLAN_WAPI_UCAST_PACKETS, &Entry->wapiUcastRekeyPackets);
	//mib_get( MIB_WLAN_WAPI_CA_INIT, &Entry->wapiCAInit);

#endif

	// Mason Yu. 201009_new_security
	mib_get(MIB_WLAN_WEP_KEY_TYPE, &Entry->wepKeyType);      // wep Key Format
	mib_get(MIB_WLAN_WEP64_KEY1, Entry->wep64Key1);
	mib_get(MIB_WLAN_WEP128_KEY1, Entry->wep128Key1);
	mib_get(MIB_WLAN_BAND, &Entry->wlanBand);
#ifdef WLAN_11W
	mib_get(MIB_WLAN_DOTIEEE80211W, &Entry->dotIEEE80211W);
	mib_get(MIB_WLAN_SHA256, &Entry->sha256);
#endif
}
#endif
int wlan_getEntry(MIB_CE_MBSSIB_T *pEntry, int index)
{
	int ret;
	unsigned char vChar;
	WLAN_MODE_T root_mode;

	ret=1;
	ret = mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)pEntry);
	root_mode = (WLAN_MODE_T)pEntry->wlanMode;
	if (index!=0) {
		ret=mib_chain_get(MIB_MBSSIB_TBL, index, (void *)pEntry);
		pEntry->wlanMode=root_mode;
#ifdef WLAN_UNIVERSAL_REPEATER
		if (index == WLAN_REPEATER_ITF_INDEX) {
			mib_get( MIB_REPEATER_ENABLED1, (void *)&vChar);
			pEntry->wlanDisabled= (vChar==0?1:0);
			if (root_mode==CLIENT_MODE)
				pEntry->wlanMode=AP_MODE;
			else
				pEntry->wlanMode=CLIENT_MODE;
		}
#endif
	}
	return ret;
}
#if 0
void MBSSID_SetRootEntry(MIB_CE_MBSSIB_T *Entry) {
	//Entry->idx = 0;

	mib_set(MIB_WLAN_ENCRYPT, &Entry->encrypt);
	mib_set(MIB_WLAN_ENABLE_1X, &Entry->enable1X);
	mib_set(MIB_WLAN_WEP, &Entry->wep);
	mib_set(MIB_WLAN_WPA_AUTH, &Entry->wpaAuth);
	mib_set(MIB_WLAN_WPA_PSK_FORMAT, &Entry->wpaPSKFormat);
	mib_set(MIB_WLAN_WPA_PSK, Entry->wpaPSK);
	mib_set(MIB_WLAN_RS_PORT, &Entry->rsPort);
	mib_set(MIB_WLAN_RS_IP, Entry->rsIpAddr);

	mib_set(MIB_WLAN_RS_PASSWORD, Entry->rsPassword);
	mib_set(MIB_WLAN_DISABLED, &Entry->wlanDisabled);
	mib_set(MIB_WLAN_SSID, Entry->ssid);
	mib_set(MIB_WLAN_MODE, &Entry->wlanMode);
	mib_set(MIB_WLAN_AUTH_TYPE, &Entry->authType);
	//added by xl_yue
	// Mason Yu. 201009_new_security
	mib_set( MIB_WLAN_WPA_CIPHER_SUITE, &Entry->unicastCipher);
	mib_set( MIB_WLAN_WPA2_CIPHER_SUITE, &Entry->wpa2UnicastCipher);
	mib_set( MIB_WLAN_WPA_GROUP_REKEY_TIME, &Entry->wpaGroupRekeyTime);

#ifdef CONFIG_RTL_WAPI_SUPPORT
	mib_set( MIB_WLAN_WAPI_PSK, Entry->wapiPsk);
	mib_set( MIB_WLAN_WAPI_PSKLEN, &Entry->wapiPskLen);
	mib_set( MIB_WLAN_WAPI_PSK_FORMAT, &Entry->wapiPskFormat);
	mib_set( MIB_WLAN_WAPI_AUTH, &Entry->wapiAuth);
	mib_set( MIB_WLAN_WAPI_ASIPADDR, Entry->wapiAsIpAddr);
	//mib_get( MIB_WLAN_WAPI_SEARCH_CERTINFO, Entry->wapiSearchCertInfo);
	//mib_get( MIB_WLAN_WAPI_SEARCH_CERTINDEX, &Entry->wapiSearchIndex);
	//mib_get( MIB_WLAN_WAPI_MCAST_REKEYTYPE, &Entry->wapiMcastkey);
	//mib_get( MIB_WLAN_WAPI_MCAST_TIME, &Entry->wapiMcastRekeyTime);
	//mib_get( MIB_WLAN_WAPI_MCAST_PACKETS, &Entry->wapiMcastRekeyPackets);
	//mib_get( MIB_WLAN_WAPI_UCAST_REKETTYPE, &Entry->wapiUcastkey);
	//mib_get( MIB_WLAN_WAPI_UCAST_TIME, &Entry->wapiUcastRekeyTime);
	//mib_get( MIB_WLAN_WAPI_UCAST_PACKETS, &Entry->wapiUcastRekeyPackets);
	//mib_get( MIB_WLAN_WAPI_CA_INIT, &Entry->wapiCAInit);

#endif

	// Mason Yu. 201009_new_security
	mib_set(MIB_WLAN_WEP_KEY_TYPE, &Entry->wepKeyType);      // wep Key Format
	mib_set(MIB_WLAN_WEP64_KEY1, Entry->wep64Key1);
	mib_set(MIB_WLAN_WEP128_KEY1, Entry->wep128Key1);
	mib_set(MIB_WLAN_BAND, &Entry->wlanBand);
#ifdef WLAN_11W
	mib_set(MIB_WLAN_DOTIEEE80211W, &Entry->dotIEEE80211W);
	mib_set(MIB_WLAN_SHA256, &Entry->sha256);
#endif
}
#endif
int wlan_setEntry(MIB_CE_MBSSIB_T *pEntry, int index)
{
	int ret=1;
	
	ret=mib_chain_update(MIB_MBSSIB_TBL, (void *)pEntry, index);
	
	return ret;
}


/////////////////////////////////////////////////////////////////////////////
static inline int
iw_get_ext(int                  skfd,           /* Socket to the kernel */
           char *               ifname,         /* Device name */
           int                  request,        /* WE ID */
           struct iwreq *       pwrq)           /* Fixed part of the request */
{
  /* Set device name */
  strncpy(pwrq->ifr_name, ifname, IFNAMSIZ);
  /* Do the request */
  return(ioctl(skfd, request, pwrq));
}

int getWlStaInfo( char *interface,  WLAN_STA_INFO_Tp pInfo )
{
    int skfd;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);

    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0)
    {
    	close( skfd );
      /* If no wireless name : no wireless extensions */
        return -1;
    }

    wrq.u.data.pointer = (caddr_t)pInfo;
    wrq.u.data.length = sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1);

    if (iw_get_ext(skfd, interface, SIOCGIWRTLSTAINFO, &wrq) < 0)
    {
    	close( skfd );
	return -1;
    }

    close( skfd );

    return 0;
}
int getWlEnc( char *interface , char *buffer, char *num)
{
    int skfd;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);

    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0)
    {
    	close( skfd );
      /* If no wireless name : no wireless extensions */
        return -1;
    }

    wrq.u.data.pointer = (caddr_t)buffer;
    wrq.u.data.length = strlen(buffer);

    if (iw_get_ext(skfd, interface, RTL8192CD_IOCTL_GET_MIB, &wrq) < 0)
    {
    	close( skfd );
		return -1;
    }
	*num  = buffer[0];
    close( skfd );

    return 0;
}

int getWlWpaChiper( char *interface , char *buffer, int *num)
{
    int skfd;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);

    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0)
    {
    	close( skfd );
      /* If no wireless name : no wireless extensions */
        return -1;
    }

    wrq.u.data.pointer = (caddr_t)buffer;
    wrq.u.data.length = strlen(buffer);

    if (iw_get_ext(skfd, interface, RTL8192CD_IOCTL_GET_MIB, &wrq) < 0)
    {
    	close( skfd );
		return -1;
    }
	memcpy(num, buffer, sizeof(int));
    close( skfd );

    return 0;
}

#ifdef WLAN_FAST_INIT
#ifdef WLAN_ACL
void set_wlan_acl(struct wifi_mib *pmib)
{
	MIB_CE_WLAN_AC_T Entry;
	int num=0, i;
	char vChar;

	// aclnum=0
	(&pmib->dot11StationConfigEntry)->dot11AclNum = 0;

	// aclmode
	mib_get(MIB_WLAN_AC_ENABLED, (void *)&vChar);
	(&pmib->dot11StationConfigEntry)->dot11AclMode = vChar;

	if (vChar){ // ACL enable
		if((num = mib_chain_total(MIB_WLAN_AC_TBL))==0)
			return;
		for (i=0; i<num; i++) {
			if (!mib_chain_get(MIB_WLAN_AC_TBL, i, (void *)&Entry))
				return;
			if(Entry.wlanIdx != wlan_idx)
				continue;
			
			// acladdr
			memcpy(&((&pmib->dot11StationConfigEntry)->dot11AclAddr[i][0]), &(Entry.macAddr[0]), 6);
			(&pmib->dot11StationConfigEntry)->dot11AclNum++;
		}
	}
}
#endif
#else
#ifdef WLAN_ACL
// return value:
// 0  : successful
// -1 : failed
int set_wlan_acl(char *ifname)
{
	unsigned char value[32];
	char parm[64];
	int num, i;
	MIB_CE_WLAN_AC_T Entry;
	int status=0;

	// aclnum=0
	status|=va_cmd(IWPRIV, 3, 1, ifname, "set_mib", "aclnum=0");

	// aclmode
	mib_get(MIB_WLAN_AC_ENABLED, (void *)value);
	snprintf(parm, sizeof(parm), "aclmode=%u", value[0]);
	status|=va_cmd(IWPRIV, 3, 1, ifname, "set_mib", parm);

	if (value[0] == 0) // ACL disabled
		return status;

	if ((num = mib_chain_total(MIB_WLAN_AC_TBL)) == 0)
		return status;

	for (i=0; i<num; i++) {
		if (!mib_chain_get(MIB_WLAN_AC_TBL, i, (void *)&Entry))
			return;
		if(Entry.wlanIdx != wlan_idx)
			continue;

		// acladdr
		snprintf(parm, sizeof(parm), "acladdr=%.2x%.2x%.2x%.2x%.2x%.2x",
			Entry.macAddr[0], Entry.macAddr[1], Entry.macAddr[2],
			Entry.macAddr[3], Entry.macAddr[4], Entry.macAddr[5]);
		status|=va_cmd(IWPRIV, 3, 1, ifname, "set_mib", parm);
	}
	return status;
}
#endif
#endif

#define RTL8185_IOCTL_READ_EEPROM	0x89f9
static int check_wlan_eeprom()
{
	int skfd,i;
	struct iwreq wrq;
	unsigned char tmp[162];
	char mode;
	char parm[64];
	char *argv[6];
	argv[1] = (char*)getWlanIfName();
	argv[2] = "set_mib";

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	/* Set device name */
	strncpy(wrq.ifr_name, (char *)getWlanIfName(), IFNAMSIZ);
	strcpy(tmp,"RFChipID");
	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = 10;
	ioctl(skfd, RTL8185_IOCTL_READ_EEPROM, &wrq);
	if(wrq.u.data.length>0){
		printf("read eeprom success!\n");
		//return 1;
	}
	else{
		printf("read eeprom fail!\n");
		if(skfd!=-1) close(skfd);
		return 0;
	}
	//set TxPowerCCK from eeprom
	strcpy(tmp,"TxPowerCCK");
	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = 20;
	ioctl(skfd, RTL8185_IOCTL_READ_EEPROM, &wrq);
	snprintf(parm, sizeof(parm), "TxPowerCCK=");
	if ( mib_get( MIB_TX_POWER, (void *)&mode) == 0) {
   		printf("Get MIB_TX_POWER error!\n");
	}

//added by xl_yue:
#ifdef WLAN_TX_POWER_DISPLAY
	if ( mode==0 ) {          // 100%
		for(i=0; i<=13; i++)
		{
	//		value[i] = 8;
			snprintf(parm, sizeof(parm), "%s%02x", parm, wrq.u.data.pointer[i]);
		}
	}else if ( mode==1 ) {    // 80%
		for(i=0; i<=13; i++)
		{
		    	wrq.u.data.pointer[i] -= 1;
			snprintf(parm, sizeof(parm), "%s%02x", parm, wrq.u.data.pointer[i]);
		}

	}else if ( mode==2 ) {    // 50%
		for(i=0; i<=13; i++)
		{
		    	wrq.u.data.pointer[i] -= 3;
			snprintf(parm, sizeof(parm), "%s%02x", parm, wrq.u.data.pointer[i]);
		}
	}else if ( mode==3 ) {    // 25%
		for(i=0; i<=13; i++)
		{
		   	wrq.u.data.pointer[i] -= 6;
			snprintf(parm, sizeof(parm), "%s%02x", parm, wrq.u.data.pointer[i]);
		}
	}else if ( mode==4 ) {    // 10%
		for(i=0; i<=13; i++)
		{
		   	wrq.u.data.pointer[i] -= 10;
			snprintf(parm, sizeof(parm), "%s%02x", parm, wrq.u.data.pointer[i]);
		}
	}
#else
	if ( mode==2 ) {          // 60mW
		for(i=0; i<=13; i++)
		{
	//		value[i] = 8;
			snprintf(parm, sizeof(parm), "%s%02x", parm, wrq.u.data.pointer[i]);
		}
	}else if ( mode==1 ) {    // 30mW
		for(i=0; i<=13; i++)
		{
		    	wrq.u.data.pointer[i] -= 3;
			snprintf(parm, sizeof(parm), "%s%02x", parm, wrq.u.data.pointer[i]);
		}
	}else if ( mode==0 ) {    // 15mW
		for(i=0; i<=13; i++)
		{
		   	wrq.u.data.pointer[i] -= 6;
			snprintf(parm, sizeof(parm), "%s%02x", parm, wrq.u.data.pointer[i]);
		}
	}
#endif
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	do_cmd(IWPRIV, argv, 1);

	strcpy(tmp,"TxPowerOFDM");
	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = 20;
	ioctl(skfd, RTL8185_IOCTL_READ_EEPROM, &wrq);
	snprintf(parm, sizeof(parm), "TxPowerOFDM=");
	if ( mib_get( MIB_TX_POWER, (void *)&mode) == 0) {
   		printf("Get MIB_TX_POWER error!\n");
	}

//added by xl_yue:
#ifdef WLAN_TX_POWER_DISPLAY
	if ( mode==0 ) {          // 100%
		for(i=0; i<=13; i++)
		{
	//		value[i] = 8;
			snprintf(parm, sizeof(parm), "%s%02x", parm, wrq.u.data.pointer[i]);
		}
	}else if ( mode==1 ) {    // 80%
		for(i=0; i<=13; i++)
		{
		    	wrq.u.data.pointer[i] -= 1;
			snprintf(parm, sizeof(parm), "%s%02x", parm, wrq.u.data.pointer[i]);
		}
	}else if ( mode==2 ) {    // 50%
		for(i=0; i<=13; i++)
		{
		    	wrq.u.data.pointer[i] -= 3;
			snprintf(parm, sizeof(parm), "%s%02x", parm, wrq.u.data.pointer[i]);
		}
	}else if ( mode==3 ) {    // 25%
		for(i=0; i<=13; i++)
		{
		   	wrq.u.data.pointer[i] -= 6;
			snprintf(parm, sizeof(parm), "%s%02x", parm, wrq.u.data.pointer[i]);
		}
	}else if ( mode==4 ) {    // 10%
		for(i=0; i<=13; i++)
		{
		   	wrq.u.data.pointer[i] -= 10;
			snprintf(parm, sizeof(parm), "%s%02x", parm, wrq.u.data.pointer[i]);
		}
	}
#else
	if ( mode==2 ) {          // 60mW
		for(i=0; i<=13; i++)
		{
	//		value[i] = 8;
			snprintf(parm, sizeof(parm), "%s%02x", parm, wrq.u.data.pointer[i]);
		}
	}else if ( mode==1 ) {    // 30mW
		for(i=0; i<=13; i++)
		{
		    	wrq.u.data.pointer[i] -= 3;
			snprintf(parm, sizeof(parm), "%s%02x", parm, wrq.u.data.pointer[i]);
		}
	}else if ( mode==0 ) {    // 15mW
		for(i=0; i<=13; i++)
		{
		   	wrq.u.data.pointer[i] -= 6;
			snprintf(parm, sizeof(parm), "%s%02x", parm, wrq.u.data.pointer[i]);
		}
	}
#endif
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	do_cmd(IWPRIV, argv, 1);

	close( skfd );
	return 1;
}

/* andrew: new test plan require N mode to avoid using TKIP. This function check the new band
   and unmask TKIP security if it's N mode.
*/
int wl_isNband(unsigned char band) {
	return (band >= 8);
}

void wl_updateSecurity(unsigned char band) {
	if (wl_isNband(band)) {
		MIB_CE_MBSSIB_T Entry;
		if(!mib_chain_get(MIB_MBSSIB_TBL, 0, &Entry))
			return;
		Entry.unicastCipher &= ~(WPA_CIPHER_TKIP);
		Entry.wpa2UnicastCipher &= ~(WPA_CIPHER_TKIP);
		mib_chain_update(MIB_MBSSIB_TBL, &Entry, 0);
	}
}

unsigned char wl_cipher2mib(unsigned char cipher) {
	unsigned char mib = 0;
	if (cipher & WPA_CIPHER_TKIP)
		mib |= 2;
	if (cipher & WPA_CIPHER_AES)
		mib |= 8;
	return mib;
}

static int _is_hex(char c)
{
    return (((c >= '0') && (c <= '9')) ||
            ((c >= 'A') && (c <= 'F')) ||
            ((c >= 'a') && (c <= 'f')));
}

static int string_to_hex(char *string, unsigned char *key, int len)
{
	char tmpBuf[4];
	int idx, ii=0;
	for (idx=0; idx<len; idx+=2) {
		tmpBuf[0] = string[idx];
		tmpBuf[1] = string[idx+1];
		tmpBuf[2] = 0;
		if ( !_is_hex(tmpBuf[0]) || !_is_hex(tmpBuf[1]))
			return 0;

		key[ii++] = (unsigned char) strtol(tmpBuf, (char**)NULL, 16);
	}
	return 1;
}

#ifdef WLAN_FAST_INIT
int setupWDS(struct wifi_mib *pmib)
{
#ifdef WLAN_WDS
	unsigned char value[128];
	char macaddr[16];
	char vChar, wds_enabled;
	char parm[128];
	char wds_num;
	char wdsPrivacy;
	WDS_T Entry;
	char wdsif[11];
	int i;
	int status = 0;

	mib_get(MIB_WLAN_MODE, (void *)&vChar);
	mib_get(MIB_WLAN_WDS_ENABLED, (void *)&wds_enabled);
	if (vChar != AP_WDS_MODE || wds_enabled == 0) {
		(&pmib->dot11WdsInfo)->wdsNum = 0;
		(&pmib->dot11WdsInfo)->wdsEnabled = 0;

		for (i=0; i<MAX_WDS_NUM; i++) {
			snprintf(wdsif, 10, (char *)WDSIF, (char*)getWlanIfName());
			wdsif[9] = '0' + i;
			wdsif[10] = '\0';
			//ifconfig wlanX-wdsX down
			status|=va_cmd(IFCONFIG, 2, 1, wdsif, "down");
			//brctl delif br0 wlanX-wdsX
			status|=va_cmd(BRCTL, 3, 1, "delif", (char *)BRIF, wdsif);
		}
		return 0;
	}

	// wds_pure
	(&pmib->dot11WdsInfo)->wdsPure = 0;

	// wds_enable
	mib_get(MIB_WLAN_WDS_ENABLED, (void *)&vChar);
	(&pmib->dot11WdsInfo)->wdsEnabled = vChar;
	(&pmib->dot11WdsInfo)->wdsNum = 0;

	//mib_get(MIB_WLAN_WDS_ENABLED, (void *)&vChar);
	//if(vChar==1){
		for (i=0; i<MAX_WDS_NUM; i++) {
			snprintf(wdsif, 10, (char *)WDSIF, (char*)getWlanIfName());

			wdsif[9] = '0' + i;
			wdsif[10] = '\0';
			//ifconfig wlanX-wdsX down
			status|=va_cmd(IFCONFIG, 2, 1, wdsif, "down");
			//brctl delif br0 wlanX-wdsX
			status|=va_cmd(BRCTL, 3, 1, "delif", (char *)BRIF, wdsif);
		}
	//}

	mib_get(MIB_WLAN_WDS_NUM, &wds_num);
	snprintf(wdsif, 10, (char *)WDSIF, (char*)getWlanIfName());
	(&pmib->dot11WdsInfo)->wdsNum = 0;

	for(i=0;i<wds_num;i++){
		if (!mib_chain_get(MIB_WDS_TBL, i, (void *)&Entry))
			continue;

		memcpy((&(&pmib->dot11WdsInfo)->entry[i])->macAddr, Entry.macAddr, 6);
		(&(&pmib->dot11WdsInfo)->entry[i])->txRate = Entry.fixedTxRate;
		(&pmib->dot11WdsInfo)->wdsNum++;
		//ifconfig wlanX-wdsX hw ether 00e04c867001
		getMIB2Str(MIB_ELAN_MAC_ADDR, macaddr);
#ifdef CONFIG_LUNA_DUAL_LINUX
		if(!strcmp(getWlanIfName(), "wlan1"))
			get_wlan_mac(WLAN1_MAC_FILE, macaddr);
#elif defined(CONFIG_RTL_92D_DMDP)
		if(wlan_idx == 1 && !useWlanIfVirtIdx())
			macaddr[5] += 5;
#endif
		wdsif[9] = '0'+i;
		wdsif[10] = '\0';
		status|=va_cmd(IFCONFIG, 4, 1, wdsif, "hw", "ether", macaddr);

		//brctl delif br0 wlanX-wdsX
		//va_cmd(BRCTL, 3, 1, "delif", (char *)BRIF, wdsif);
		//brctl addif br0 wlanX-wdsX
		status|=va_cmd(BRCTL, 3, 1, "addif", (char *)BRIF, wdsif);
#ifdef CONFIG_IPV6
		// Disable ipv6 in bridge
		setup_disable_ipv6(wdsif, 1);
#endif

		//ifconfig wlanX-wdsX up
		status|=va_cmd(IFCONFIG, 2, 1, wdsif, "up");
	}

	// wds_encrypt
	mib_get(MIB_WLAN_WDS_ENCRYPT, &vChar);
	if (vChar == WDS_ENCRYPT_DISABLED)//open
		wdsPrivacy = 0;
	else if (vChar == WDS_ENCRYPT_WEP64) {//wep 40
		wdsPrivacy = 1;
	}
	else if (vChar == WDS_ENCRYPT_WEP128) {//wep 104
		wdsPrivacy = 5;
	}
	else if (vChar == WDS_ENCRYPT_TKIP){//tkip
		wdsPrivacy = 2;
	}
	else if(vChar == WDS_ENCRYPT_AES){//ccmp
		wdsPrivacy = 4;
	}
	if(wdsPrivacy == 1 || wdsPrivacy == 5){
		mib_get(MIB_WLAN_WDS_WEP_KEY, (void *)value);
		if(wdsPrivacy == 1)
			string_to_hex((char *)value, (&pmib->dot11WdsInfo)->wdsWepKey, 10);
		else
			string_to_hex((char *)value, (&pmib->dot11WdsInfo)->wdsWepKey, 26);
		
	}
	else if(wdsPrivacy == 2|| wdsPrivacy == 4){
		mib_get(MIB_WLAN_WDS_PSK, (void *)value);
		strcpy((&pmib->dot11WdsInfo)->wdsPskPassPhrase, value);
	}

	(&pmib->dot11WdsInfo)->wdsPrivacy = wdsPrivacy;

	(&pmib->dot11WdsInfo)->wdsPriority = 1;

	//for MOD multicast-filter
	mib_get(MIB_WLAN_WDS_ENABLED, (void *)&vChar);
	if (vChar) {
		//brctl clrfltrport br0
		va_cmd(BRCTL,2,1,"clrfltrport",(char *)BRIF);
		//brctl setfltrport br0 11111
		va_cmd(BRCTL,3,1,"setfltrport",(char *)BRIF,"11111");
		//brctl setfltrport br0 55555
		va_cmd(BRCTL,3,1,"setfltrport",(char *)BRIF,"55555");
		//brctl setfltrport br0 2105
		va_cmd(BRCTL,3,1,"setfltrport",(char *)BRIF,"2105");
		//brctl setfltrport br0 2105
		va_cmd(BRCTL,3,1,"setfltrport",(char *)BRIF,"2107");
	}

	return status;
#else
	return 0;
#endif
}
#else
int setupWDS()
{
#ifdef WLAN_WDS
	unsigned char value[128];
	char macaddr[16];
	char vChar, wds_enabled;
	char parm[128];
	char wds_num;
	char wdsPrivacy;
	WDS_T Entry;
	char wdsif[11];
	int i;
	int status = 0;

	mib_get(MIB_WLAN_MODE, (void *)&vChar);
	mib_get(MIB_WLAN_WDS_ENABLED, (void *)&wds_enabled);
	if (vChar != AP_WDS_MODE || wds_enabled == 0) {
		status|=va_cmd(IWPRIV, 3, 1, (char*)getWlanIfName(), "set_mib", "wds_num=0");
		status|=va_cmd(IWPRIV, 3, 1, (char*)getWlanIfName(), "set_mib", "wds_enable=0");

		for (i=0; i<MAX_WDS_NUM; i++) {
			snprintf(wdsif, 10, (char *)WDSIF, (char*)getWlanIfName());
			wdsif[9] = '0' + i;
			wdsif[10] = '\0';
			//ifconfig wlanX-wdsX down
			status|=va_cmd(IFCONFIG, 2, 1, wdsif, "down");
			//brctl delif br0 wlanX-wdsX
			status|=va_cmd(BRCTL, 3, 1, "delif", (char *)BRIF, wdsif);
		}
		return 0;
	}

	// wds_pure
	status|=va_cmd(IWPRIV, 3, 1, (char*)getWlanIfName(), "set_mib", "wds_pure=0");

	// wds_enable
	mib_get(MIB_WLAN_WDS_ENABLED, (void *)&vChar);
	snprintf(parm,  sizeof(parm), "wds_enable=%u", vChar);
	status|=va_cmd(IWPRIV, 3, 1, (char*)getWlanIfName(), "set_mib", parm);
	
	status|=va_cmd(IWPRIV, 3, 1, (char*)getWlanIfName(), "set_mib", "wds_num=0");

	//mib_get(MIB_WLAN_WDS_ENABLED, (void *)&vChar);
	//if(vChar==1){
		for (i=0; i<MAX_WDS_NUM; i++) {
			snprintf(wdsif, 10, (char *)WDSIF, (char*)getWlanIfName());

			wdsif[9] = '0' + i;
			wdsif[10] = '\0';
			//ifconfig wlanX-wdsX down
			status|=va_cmd(IFCONFIG, 2, 1, wdsif, "down");
			//brctl delif br0 wlanX-wdsX
			status|=va_cmd(BRCTL, 3, 1, "delif", (char *)BRIF, wdsif);
		}
	//}

	mib_get(MIB_WLAN_WDS_NUM, &wds_num);
	snprintf(wdsif, 10, (char *)WDSIF, (char*)getWlanIfName());

	for(i=0;i<wds_num;i++){
		if (!mib_chain_get(MIB_WDS_TBL, i, (void *)&Entry))
			continue;
		snprintf(parm, sizeof(parm), "wds_add=%02x%02x%02x%02x%02x%02x,%u",
			Entry.macAddr[0], Entry.macAddr[1], Entry.macAddr[2],
			Entry.macAddr[3], Entry.macAddr[4], Entry.macAddr[5], Entry.fixedTxRate);
		status|=va_cmd(IWPRIV, 3, 1, (char*)getWlanIfName(), "set_mib", parm);
		//ifconfig wlanX-wdsX hw ether 00e04c867001
		getMIB2Str(MIB_ELAN_MAC_ADDR, macaddr);
#ifdef CONFIG_LUNA_DUAL_LINUX
		if(!strcmp(getWlanIfName(), "wlan1"))
			get_wlan_mac(WLAN1_MAC_FILE, macaddr);
#elif defined(CONFIG_RTL_92D_DMDP)
		if(wlan_idx == 1 && !useWlanIfVirtIdx())
			macaddr[5] += 5;
#endif
		wdsif[9] = '0'+i;
		wdsif[10] = '\0';
		status|=va_cmd(IFCONFIG, 4, 1, wdsif, "hw", "ether", macaddr);

		//brctl delif br0 wlanX-wdsX
		//va_cmd(BRCTL, 3, 1, "delif", (char *)BRIF, wdsif);
		//brctl addif br0 wlanX-wdsX
		status|=va_cmd(BRCTL, 3, 1, "addif", (char *)BRIF, wdsif);
#ifdef CONFIG_IPV6
		// Disable ipv6 in bridge
		setup_disable_ipv6(wdsif, 1);
#endif

		//ifconfig wlanX-wdsX up
		status|=va_cmd(IFCONFIG, 2, 1, wdsif, "up");
	}

	// wds_encrypt
	mib_get(MIB_WLAN_WDS_ENCRYPT, &vChar);
	if (vChar == WDS_ENCRYPT_DISABLED)//open
		wdsPrivacy = 0;
	else if (vChar == WDS_ENCRYPT_WEP64) {//wep 40
		wdsPrivacy = 1;
	}
	else if (vChar == WDS_ENCRYPT_WEP128) {//wep 104
		wdsPrivacy = 5;
	}
	else if (vChar == WDS_ENCRYPT_TKIP){//tkip
		wdsPrivacy = 2;
	}
	else if(vChar == WDS_ENCRYPT_AES){//ccmp
		wdsPrivacy = 4;
	}
	if(wdsPrivacy == 1 || wdsPrivacy == 5){
		mib_get(MIB_WLAN_WDS_WEP_KEY, (void *)value);
		snprintf(parm, sizeof(parm), "wds_wepkey=%s", value);
		status|=va_cmd(IWPRIV, 3, 1, (char*)getWlanIfName(), "set_mib", parm);
		
	}
	else if(wdsPrivacy == 2|| wdsPrivacy == 4){
		mib_get(MIB_WLAN_WDS_PSK, (void *)value);
		snprintf(parm, sizeof(parm), "wds_passphrase=%s", value);
		status|=va_cmd(IWPRIV, 3, 1, (char*)getWlanIfName(), "set_mib", parm);
	}
	snprintf(parm, sizeof(parm), "wds_encrypt=%u", wdsPrivacy);
	status|=va_cmd(IWPRIV, 3, 1, (char*)getWlanIfName(), "set_mib", parm);

	status|=va_cmd(IWPRIV, 3, 1, (char*)getWlanIfName(), "set_mib", "wds_priority=1");

	//for MOD multicast-filter
	mib_get(MIB_WLAN_WDS_ENABLED, (void *)&vChar);
	if (vChar) {
		//brctl clrfltrport br0
		va_cmd(BRCTL,2,1,"clrfltrport",(char *)BRIF);
		//brctl setfltrport br0 11111
		va_cmd(BRCTL,3,1,"setfltrport",(char *)BRIF,"11111");
		//brctl setfltrport br0 55555
		va_cmd(BRCTL,3,1,"setfltrport",(char *)BRIF,"55555");
		//brctl setfltrport br0 2105
		va_cmd(BRCTL,3,1,"setfltrport",(char *)BRIF,"2105");
		//brctl setfltrport br0 2105
		va_cmd(BRCTL,3,1,"setfltrport",(char *)BRIF,"2107");
	}

	return status;
#else
	return 0;
#endif
}
#endif
//krammer
#ifdef WLAN_MBSSID
static void _set_vap_para(const char* arg1, const char* arg2){
	int i, start, end;
	char ifname[16];
	
	for(i=0; i<WLAN_MBSSID_NUM; i++){
		snprintf(ifname, 16, "%s-vap%d", (char *)getWlanIfName(), i);
		va_cmd(IWPRIV, 3, 1, ifname, arg1, arg2);
	}
}

#define set_vap_para(a, b) _set_vap_para(a, b)
#else
#define set_vap_para(a, b)
#endif

#ifdef WLAN_FAST_INIT
static void setupWLan_WPA(struct wifi_mib *pmib, int vwlan_idx)
{
	char parm[64];
	int vInt, status;
	MIB_CE_MBSSIB_T Entry;

	mib_chain_get(MIB_MBSSIB_TBL, vwlan_idx, (void *)&Entry);

	// encmode
	vInt = (int)Entry.encrypt;

	if (Entry.encrypt == WIFI_SEC_WEP)	// WEP mode
	{
		// psk_enable: 0
		(&pmib->dot1180211AuthEntry)->dot11EnablePSK = 0;
		if (Entry.wep == WEP64) {
			// 64 bits
			vInt = 1; // encmode = 1
			// wepkey1
			// Mason Yu. 201009_new_security.
			memcpy((&pmib->dot11DefaultKeysTable)->keytype,Entry.wep64Key1,5);
		}
		else {
			// 128 bits
			vInt = 5; // encmode = 5
			// wepkey1
			// Mason Yu. 201009_new_security.
			memcpy((&pmib->dot11DefaultKeysTable)->keytype,Entry.wep128Key1,13);
		}
	}
	// Kaohj --- this is for driver level security.
	// ignoring it if using userland 'auth' security
	#ifdef CONFIG_RTL_WAPI_SUPPORT
	else if (Entry.encrypt == WIFI_SEC_WAPI) {
		//char wapiAuth;
		vInt = 0; // encmode = 0 for WAPI
		(&pmib->wapiInfo)->wapiType = Entry.wapiAuth;
		// psk_enable: 0
		(&pmib->dot1180211AuthEntry)->dot11EnablePSK = 0;
		
		if (Entry.wapiAuth==2) { //PSK
			//char pskFormat, pskLen, pskValue[MAX_PSK_LEN];
			//mib_get(MIB_WLAN_WAPI_PSK_FORMAT, (void *)&pskFormat);
			//mib_get(MIB_WLAN_WAPI_PSK, (void *)pskValue);
			//mib_get(MIB_WLAN_WAPI_PSKLEN, (void *)&pskLen);


			if (Entry.wapiPskFormat) { // HEX
				int i;
				char hexstr[4];
				for (i=0;i<Entry.wapiPskLen;i++) {
					snprintf(hexstr, sizeof(hexstr), "%02x", Entry.wapiPsk[i]);
					strcat(parm, hexstr);
				}
				(&(&pmib->wapiInfo)->wapiPsk)->len = Entry.wapiPskLen;
				memcpy((&(&pmib->wapiInfo)->wapiPsk)->octet, parm, Entry.wapiPskLen*2);
				

			} else { // passphrase
				(&(&pmib->wapiInfo)->wapiPsk)->len = Entry.wapiPskLen;
				memcpy((&(&pmib->wapiInfo)->wapiPsk)->octet, Entry.wapiPsk, Entry.wapiPskLen);
			}
		} else { //AS

		}


	}
	#endif
	else if (Entry.encrypt >= WIFI_SEC_WPA) {	// WPA setup
		// Mason Yu. 201009_new_security. Start
		if (Entry.encrypt == WIFI_SEC_WPA
			|| Entry.encrypt == WIFI_SEC_WPA2_MIXED) {
			(&pmib->dot1180211AuthEntry)->dot11WPACipher = wl_cipher2mib(Entry.unicastCipher);
		} else {
			(&pmib->dot1180211AuthEntry)->dot11WPACipher = 0;
		}
		
		if (Entry.encrypt == WIFI_SEC_WPA2
			|| Entry.encrypt == WIFI_SEC_WPA2_MIXED) {
			(&pmib->dot1180211AuthEntry)->dot11WPA2Cipher = wl_cipher2mib(Entry.wpa2UnicastCipher);
		} else {
			(&pmib->dot1180211AuthEntry)->dot11WPA2Cipher = 0;
		}
		// Mason Yu. 201009_new_security. End

		if (!is8021xEnabled(vwlan_idx)) {
			// psk_enable: 1: WPA, 2: WPA2, 3: WPA+WPA2
			(&pmib->dot1180211AuthEntry)->dot11EnablePSK = Entry.encrypt/2;
			
			// passphrase
			strcpy((&pmib->dot1180211AuthEntry)->dot11PassPhrase, Entry.wpaPSK);
			
			(&pmib->dot1180211AuthEntry)->dot11GKRekeyTime = Entry.wpaGroupRekeyTime;
		}
		else {
			// psk_enable: 0
			(&pmib->dot1180211AuthEntry)->dot11EnablePSK = 0;
		}
		vInt = 2;
	}
	else {
		// psk_enable: 0
		(&pmib->dot1180211AuthEntry)->dot11EnablePSK = 0;
	}
	(&pmib->dot1180211AuthEntry)->dot11PrivacyAlgrthm = vInt;
}

#else
/*
 *	vwlan_idx:
 *	0:	Root
 *	1 ~ :	VAP, Repeater
 */
static int setupWLan_WPA(int vwlan_idx)
{
	char ifname[16];
	char parm[128];
	int vInt, status;
	MIB_CE_MBSSIB_T Entry;

	mib_chain_get(MIB_MBSSIB_TBL, vwlan_idx, (void *)&Entry);
	
	if (vwlan_idx==0) 
		strncpy(ifname, (char*)getWlanIfName(), 16);
	else {
		#ifdef WLAN_MBSSID
		if (vwlan_idx>=WLAN_VAP_ITF_INDEX && vwlan_idx<(WLAN_VAP_ITF_INDEX+WLAN_MBSSID_NUM))
			snprintf(ifname, 16, "%s-vap%d", (char *)getWlanIfName(), vwlan_idx-1);
		#endif
		#ifdef WLAN_UNIVERSAL_REPEATER
		if (vwlan_idx == WLAN_REPEATER_ITF_INDEX)
			snprintf(ifname, 16, "%s-vxd", (char *)getWlanIfName());
		#endif
	}

	// encmode
	vInt = (int)Entry.encrypt;

	if (Entry.encrypt == WIFI_SEC_WEP)	// WEP mode
	{
		// psk_enable: 0
		status|=va_cmd(IWPRIV, 3, 1, ifname, "set_mib", "psk_enable=0");

		if (Entry.wep == WEP64) {
			// 64 bits
			vInt = 1; // encmode = 1
			// wepkey1
			// Mason Yu. 201009_new_security.
			snprintf(parm, sizeof(parm), "wepkey1=%02x%02x%02x%02x%02x", Entry.wep64Key1[0],
				Entry.wep64Key1[1], Entry.wep64Key1[2], Entry.wep64Key1[3], Entry.wep64Key1[4]);
			status|=va_cmd(IWPRIV, 3, 1, ifname, "set_mib", parm);
		}
		else {
			// 128 bits
			vInt = 5; // encmode = 5
			// wepkey1
			// Mason Yu. 201009_new_security.
			snprintf(parm, sizeof(parm), "wepkey1=%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
				Entry.wep128Key1[0], Entry.wep128Key1[1], Entry.wep128Key1[2], Entry.wep128Key1[3], Entry.wep128Key1[4],
				Entry.wep128Key1[5], Entry.wep128Key1[6], Entry.wep128Key1[7], Entry.wep128Key1[8], Entry.wep128Key1[9],
				Entry.wep128Key1[10], Entry.wep128Key1[11], Entry.wep128Key1[12]);
			status|=va_cmd(IWPRIV, 3, 1, ifname, "set_mib", parm);
		}
	}
	// Kaohj --- this is for driver level security.
	// ignoring it if using userland 'auth' security
	#ifdef CONFIG_RTL_WAPI_SUPPORT
	else if (Entry.encrypt == WIFI_SEC_WAPI) {
		//char wapiAuth;
		vInt = 0; // encmode = 0 for WAPI
		snprintf(parm, sizeof(parm), "wapiType=%d", Entry.wapiAuth);
		status|=va_cmd(IWPRIV, 3, 1, ifname, "set_mib", parm);

		// psk_enable: 0
		status|=va_cmd(IWPRIV, 3, 1, ifname, "set_mib", "psk_enable=0");

		if (Entry.wapiAuth==2) { //PSK
			//char pskFormat, pskLen, pskValue[MAX_PSK_LEN];
			//mib_get(MIB_WLAN_WAPI_PSK_FORMAT, (void *)&pskFormat);
			//mib_get(MIB_WLAN_WAPI_PSK, (void *)pskValue);
			//mib_get(MIB_WLAN_WAPI_PSKLEN, (void *)&pskLen);


			if (Entry.wapiPskFormat) { // HEX
				int i;
				char hexstr[4];
				snprintf(parm, sizeof(parm), "wapiPsk=");
				for (i=0;i<Entry.wapiPskLen;i++) {
					snprintf(hexstr, sizeof(hexstr), "%02x", Entry.wapiPsk[i]);
					strcat(parm, hexstr);
				}
				strcat(parm, ",");
				snprintf(hexstr, sizeof(hexstr), "%x", Entry.wapiPskLen);
				strcat(parm, hexstr);
				status|=va_cmd(IWPRIV, 3, 1, ifname, "set_mib", parm);

			} else { // passphrase
				snprintf(parm, sizeof(parm), "wapiPsk=%s,%x", Entry.wapiPsk, Entry.wapiPskLen);
				status|=va_cmd(IWPRIV, 3, 1, ifname, "set_mib", parm);
			}
		} else { //AS

		}


	}
	#endif
	else if (Entry.encrypt >= WIFI_SEC_WPA) {	// WPA setup
		// Mason Yu. 201009_new_security. Start
		if (Entry.encrypt == WIFI_SEC_WPA
#ifndef NEW_WIFI_SEC
			|| Entry.encrypt == WIFI_SEC_WPA2_MIXED
#endif
			) {
			snprintf(parm, sizeof(parm), "wpa_cipher=%d", wl_cipher2mib(Entry.unicastCipher));
			status|=va_cmd(IWPRIV, 3, 1, ifname, "set_mib", parm);
		}
#ifdef NEW_WIFI_SEC
		else if (Entry.encrypt == WIFI_SEC_WPA2_MIXED){
			snprintf(parm, sizeof(parm), "wpa_cipher=%d", wl_cipher2mib(WPA_CIPHER_MIXED));
			status|=va_cmd(IWPRIV, 3, 1, ifname, "set_mib", parm);
		}
#endif
		else {
			snprintf(parm, sizeof(parm), "wpa_cipher=0");
			status|=va_cmd(IWPRIV, 3, 1, ifname, "set_mib", parm);
		}
		
		if (Entry.encrypt == WIFI_SEC_WPA2
#ifndef NEW_WIFI_SEC
			|| Entry.encrypt == WIFI_SEC_WPA2_MIXED
#endif
			) {
			snprintf(parm, sizeof(parm), "wpa2_cipher=%d", wl_cipher2mib(Entry.wpa2UnicastCipher));
			status|=va_cmd(IWPRIV, 3, 1, ifname, "set_mib", parm);
		}
#ifdef NEW_WIFI_SEC
		else if (Entry.encrypt == WIFI_SEC_WPA2_MIXED){
			snprintf(parm, sizeof(parm), "wpa2_cipher=%d", wl_cipher2mib(WPA_CIPHER_MIXED));
			status|=va_cmd(IWPRIV, 3, 1, ifname, "set_mib", parm);
		}
#endif
		else {
			snprintf(parm, sizeof(parm), "wpa2_cipher=0");
			status|=va_cmd(IWPRIV, 3, 1, ifname, "set_mib", parm);
		}
		// Mason Yu. 201009_new_security. End
		if (!is8021xEnabled(vwlan_idx)) {
			// psk_enable: 1: WPA, 2: WPA2, 3: WPA+WPA2
			snprintf(parm, sizeof(parm), "psk_enable=%d", Entry.encrypt/2);
			status|=va_cmd(IWPRIV, 3, 1, ifname, "set_mib", parm);

			// passphrase
			snprintf(parm, sizeof(parm), "passphrase=%s", Entry.wpaPSK);
			status|=va_cmd(IWPRIV, 3, 1, ifname, "set_mib", parm);

			snprintf(parm, sizeof(parm), "gk_rekey=%d", Entry.wpaGroupRekeyTime);
			status|=va_cmd(IWPRIV, 3, 1, ifname, "set_mib", parm);
		}
		else {
			// psk_enable: 0
			status|=va_cmd(IWPRIV, 3, 1, ifname, "set_mib", "psk_enable=0");
		}
		vInt = 2;
	}
	else {
		// psk_enable: 0
		status|=va_cmd(IWPRIV, 3, 1, ifname, "set_mib", "psk_enable=0");
	}
#ifdef WLAN_11W		
	if (Entry.encrypt == WIFI_SEC_WPA2){
		snprintf(parm, sizeof(parm), "dot11IEEE80211W=%d", Entry.dotIEEE80211W);
		status|=va_cmd(IWPRIV, 3, 1, ifname, "set_mib", parm);
		snprintf(parm, sizeof(parm), "enableSHA256=%d", Entry.sha256);
		status|=va_cmd(IWPRIV, 3, 1, ifname, "set_mib", parm);
	}
	else{
		snprintf(parm, sizeof(parm), "dot11IEEE80211W=0");
		status|=va_cmd(IWPRIV, 3, 1, ifname, "set_mib", parm);
		snprintf(parm, sizeof(parm), "enableSHA256=0");
		status|=va_cmd(IWPRIV, 3, 1, ifname, "set_mib", parm);
	}
#endif
	snprintf(parm, sizeof(parm), "encmode=%u", vInt);
	status|=va_cmd(IWPRIV, 3, 1, ifname, "set_mib", parm);
	return status;
}
#endif

#ifdef WLAN_11R
static int setupWLan_FT(int vwlan_idx)
{
	char ifname[16];
	char parm[64];
	int vInt, status;
	MIB_CE_MBSSIB_T Entry;

	mib_chain_get(MIB_MBSSIB_TBL, vwlan_idx, (void *)&Entry);
	
	if (vwlan_idx==0) 
		strncpy(ifname, (char*)getWlanIfName(), 16);
	else {
		#ifdef WLAN_MBSSID
		if (vwlan_idx>=WLAN_VAP_ITF_INDEX && vwlan_idx<(WLAN_VAP_ITF_INDEX+WLAN_MBSSID_NUM))
			snprintf(ifname, 16, "%s-vap%d", (char *)getWlanIfName(), vwlan_idx-1);
		#endif
		#ifdef WLAN_UNIVERSAL_REPEATER
		if (vwlan_idx == WLAN_REPEATER_ITF_INDEX)
			snprintf(ifname, 16, "%s-vxd", (char *)getWlanIfName());
		#endif
	}

	snprintf(parm, sizeof(parm), "ft_enable=%d", Entry.ft_enable);
	status|=va_cmd(IWPRIV, 3, 1, ifname, "set_mib", parm);

	snprintf(parm, sizeof(parm), "ft_mdid=%s", Entry.ft_mdid);
	status|=va_cmd(IWPRIV, 3, 1, ifname, "set_mib", parm);

	snprintf(parm, sizeof(parm), "ft_over_ds=%d", Entry.ft_over_ds);
	status|=va_cmd(IWPRIV, 3, 1, ifname, "set_mib", parm);

	snprintf(parm, sizeof(parm), "ft_res_request=%d", Entry.ft_res_request);
	status|=va_cmd(IWPRIV, 3, 1, ifname, "set_mib", parm);

	snprintf(parm, sizeof(parm), "ft_r0key_timeout=%d", Entry.ft_r0key_timeout);
	status|=va_cmd(IWPRIV, 3, 1, ifname, "set_mib", parm);

	snprintf(parm, sizeof(parm), "ft_reasoc_timeout=%d", Entry.ft_reasoc_timeout);
	status|=va_cmd(IWPRIV, 3, 1, ifname, "set_mib", parm);

	snprintf(parm, sizeof(parm), "ft_r0kh_id=%s", Entry.ft_r0kh_id);
	status|=va_cmd(IWPRIV, 3, 1, ifname, "set_mib", parm);

	snprintf(parm, sizeof(parm), "ft_push=%d", Entry.ft_push);
	status|=va_cmd(IWPRIV, 3, 1, ifname, "set_mib", parm);

	return status;
}

int genFtKhConfig()
{
	MIB_CE_WLAN_FTKH_T ent;
	int ii, i, entryNum;
	int j, wlanIdxBackup=wlan_idx;
	char ifname[17]={0};
	char tmpbuf[200], macStr[18]={0};
	int status=0, ishex;
	FILE *fp;
 	char enckey[35]={0};

	if((fp=fopen(FT_CONF, "w")) == NULL)
	{
		printf("%s: Cannot create %s!\n", __func__, FT_CONF);
		return -1;
	}

	for (j=0; j<NUM_WLAN_INTERFACE; j++) {
		wlan_idx = j;
		entryNum = mib_chain_total(MIB_WLAN_FTKH_TBL);
		for (i=0; i<entryNum; i++) {
			if (!mib_chain_get(MIB_WLAN_FTKH_TBL, i, (void *)&ent))
			{
				printf("%s: Get chain record error!\n", __func__);
				return -1;
			}
			if (ent.intfIdx==0)
				strncpy(ifname, (char*)getWlanIfName(), 16);
			else {
				#ifdef WLAN_MBSSID
				if (ent.intfIdx>=WLAN_VAP_ITF_INDEX && ent.intfIdx<(WLAN_VAP_ITF_INDEX+WLAN_MBSSID_NUM))
					snprintf(ifname, 16, "%s-vap%d", (char *)getWlanIfName(), ent.intfIdx-1);
				#endif
				#ifdef WLAN_UNIVERSAL_REPEATER
				if (ent.intfIdx == WLAN_REPEATER_ITF_INDEX)
					snprintf(ifname, 16, "%s-vxd", (char *)getWlanIfName());
				#endif
			}
			snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
				ent.addr[0], ent.addr[1], ent.addr[2], ent.addr[3], ent.addr[4], ent.addr[5]);

			// check key format
			ishex = 1;
			if (strlen(ent.key) != 32) {
				ishex=0;
			}
			else
				for (ii=0; ii<32; ii++)
					if (!_is_hex(ent.key[ii])) {
						ishex = 0;
						break;
					}
			if (!ishex)
				snprintf(enckey, sizeof(enckey), "\"%s\"", ent.key);
			
			fprintf(fp, "r0kh=%s %s %s %s\n", macStr, ent.r0kh_id, ishex?ent.key:enckey, ifname);
			fprintf(fp, "r1kh=%s %s %s %s\n", macStr, macStr, ishex?ent.key:enckey, ifname);
		}
	}

	wlan_idx = wlanIdxBackup;
	fclose(fp);
	return 0;
}

int start_FT()
{
	MIB_CE_MBSSIB_T Entry;
	int vwlan_idx=0, i;
	int status=0;
	char wlanDisabled;
	char *cmd_opt[16];
	int cmd_cnt = 0;
	int idx;
	//char intfList[200]="";
	char intfList[12][32]={0};
	int intfNum=0;
	int enablePush=0;
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
	int orig_wlan_idx=wlan_idx;
#endif

	genFtKhConfig();
	for(i = 0; i<NUM_WLAN_INTERFACE; i++) {
		wlan_idx = i;
#ifdef WLAN_MBSSID
		for (vwlan_idx=0; vwlan_idx<=NUM_VWLAN_INTERFACE; vwlan_idx++) {
#endif
			wlan_getEntry(&Entry, vwlan_idx);
			if (vwlan_idx==0) // root
				wlanDisabled = Entry.wlanDisabled;
			if (wlanDisabled || Entry.wlanDisabled)
				continue;
			if (Entry.wlanMode == CLIENT_MODE)
				continue;

			if ((Entry.encrypt==4||Entry.encrypt==6) && Entry.ft_enable) {
				if (vwlan_idx==0)
					snprintf(intfList[intfNum], sizeof(intfList[intfNum]), "wlan%d", i);
				else
					snprintf(intfList[intfNum], sizeof(intfList[intfNum]), "wlan%d-vap%d", i, vwlan_idx-WLAN_VAP_ITF_INDEX);
				intfNum++;
				if (Entry.ft_push)
					enablePush = 1;
			}
#ifdef WLAN_MBSSID
		}
#endif
	}

	fprintf(stderr, "START 802.11r SETUP!\n");
	cmd_opt[cmd_cnt++] = "";

	if (intfNum) {
		cmd_opt[cmd_cnt++] = "-w";
		for (i=0; i<intfNum; i++)
			cmd_opt[cmd_cnt++] = (char *)intfList[i];

		cmd_opt[cmd_cnt++] = "-c";
		cmd_opt[cmd_cnt++] = (char *)FT_CONF;

		cmd_opt[cmd_cnt++] = "-pid";
		cmd_opt[cmd_cnt++] = (char *)FT_PID;

		cmd_opt[cmd_cnt] = 0;
		printf("CMD ARGS: ");
		for (idx=0; idx<cmd_cnt;idx++)
			printf("%s ", cmd_opt[idx]);
		printf("\n");

		status |= do_cmd(FT_DAEMON_PROG, cmd_opt, 0);
	}

#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
	wlan_idx = orig_wlan_idx;
#endif

	return status;
}
#endif

#ifdef WLAN_FAST_INIT
static void setupWLan_802_1x(struct wifi_mib * pmib, int vwlan_idx)
{
	char parm[64];
	int vInt, status;
	MIB_CE_MBSSIB_T Entry;

	mib_chain_get(MIB_MBSSIB_TBL, vwlan_idx, (void *)&Entry);

	// Set 802.1x flag
	vInt = (int)Entry.encrypt;

	if (vInt <= WIFI_SEC_WEP)
	{
		// 802_1x
		(&pmib->dot118021xAuthEntry)->dot118021xAlgrthm = Entry.enable1X;
	}
#ifdef CONFIG_RTL_WAPI_SUPPORT
	else if (vInt == WIFI_SEC_WAPI) 
		(&pmib->dot118021xAuthEntry)->dot118021xAlgrthm = 0;
#endif
	else
		(&pmib->dot118021xAuthEntry)->dot118021xAlgrthm = 1;
}
#else
/*
 *	vwlan_idx:
 *	0:	Root
 *	1 ~ :	VAP, Repeater
 */
static int setupWLan_802_1x(int vwlan_idx)
{
	char ifname[16];
	char parm[64];
	int vInt, status;
	MIB_CE_MBSSIB_T Entry;

	mib_chain_get(MIB_MBSSIB_TBL, vwlan_idx, (void *)&Entry);
	if (vwlan_idx==0) 
		strncpy(ifname, (char*)getWlanIfName(), 16);
	else {
		#ifdef WLAN_MBSSID
		if (vwlan_idx>=WLAN_VAP_ITF_INDEX && vwlan_idx<(WLAN_VAP_ITF_INDEX+WLAN_MBSSID_NUM))
			snprintf(ifname, 16, "%s-vap%d", (char *)getWlanIfName(), vwlan_idx-1);
		#endif
		#ifdef WLAN_UNIVERSAL_REPEATER
		if (vwlan_idx == WLAN_REPEATER_ITF_INDEX)
			snprintf(ifname, 16, "%s-vxd", (char *)getWlanIfName());
		#endif
	}

	// Set 802.1x flag
	vInt = (int)Entry.encrypt;

	if (vInt <= WIFI_SEC_WEP)
	{
		// 802_1x
		snprintf(parm, sizeof(parm), "802_1x=%u", Entry.enable1X);
		status|=va_cmd(IWPRIV, 3, 1, ifname, "set_mib", parm);
	}
#ifdef CONFIG_RTL_WAPI_SUPPORT
	else if (vInt == WIFI_SEC_WAPI) 
		status|=va_cmd(IWPRIV, 3, 1, ifname, "set_mib", "802_1x=0");
#endif
	else
		status|=va_cmd(IWPRIV, 3, 1, ifname, "set_mib", "802_1x=1");
	
	return status;
}
#endif
#ifdef WLAN_FAST_INIT
static void setupWLan_dot11_auth(struct wifi_mib* pmib, int vwlan_idx)
{
	char parm[64];
	unsigned char auth;
	#ifdef CONFIG_WIFI_SIMPLE_CONFIG
	unsigned char wsc_disabled;
	#endif
	int vInt, status;
	MIB_CE_MBSSIB_T Entry;

	mib_chain_get(MIB_MBSSIB_TBL, vwlan_idx, (void *)&Entry);

	auth = Entry.authType;

	if (Entry.authType == AUTH_SHARED && Entry.encrypt != WIFI_SEC_WEP)
		// shared-key and not WEP enabled, force to open-system
		auth = (AUTH_TYPE_T)AUTH_OPEN;
	#ifdef CONFIG_RTL_WAPI_SUPPORT
	if (Entry.encrypt==WIFI_SEC_WAPI)
		auth = (AUTH_TYPE_T)AUTH_OPEN;
	else 
		(&pmib->wapiInfo)->wapiType = 0;
	#endif

	#ifdef CONFIG_WIFI_SIMPLE_CONFIG //cathy
	if (vwlan_idx == 0) // root
		//clear wsc_enable, this parameter will be set in wsc daemon
		(&pmib->wscEntry)->wsc_enable = 0;

	wsc_disabled = Entry.wsc_disabled;
	if(vwlan_idx == 0 && Entry.authType == AUTH_SHARED && Entry.encrypt == WIFI_SEC_WEP && wsc_disabled!=1)
		auth = (AUTH_TYPE_T)AUTH_BOTH;	//if root shared-key and wep/wps enable, force to open+shared system for wps
	#endif
	(&pmib->dot1180211AuthEntry)->dot11AuthAlgrthm = auth;

}
#else
/*
 *	vwlan_idx:
 *	0:	Root
 *	1 ~ :	VAP, Repeater
 */
static int setupWLan_dot11_auth(int vwlan_idx)
{
	char ifname[16];
	char parm[64];
	unsigned char auth;
	#ifdef CONFIG_WIFI_SIMPLE_CONFIG
	unsigned char wsc_disabled;
	#endif
	int vInt, status;
	MIB_CE_MBSSIB_T Entry;
	mib_chain_get(MIB_MBSSIB_TBL, vwlan_idx, (void *)&Entry);
	
	if (vwlan_idx==0)
		strncpy(ifname, (char*)getWlanIfName(), 16);
	else {
		#ifdef WLAN_MBSSID
		if (vwlan_idx>=WLAN_VAP_ITF_INDEX && vwlan_idx<(WLAN_VAP_ITF_INDEX+WLAN_MBSSID_NUM))
			snprintf(ifname, 16, "%s-vap%d", (char *)getWlanIfName(), vwlan_idx-1);
		#endif
		#ifdef WLAN_UNIVERSAL_REPEATER
		if (vwlan_idx == WLAN_REPEATER_ITF_INDEX)
			snprintf(ifname, 16, "%s-vxd", (char *)getWlanIfName());
		#endif
	}

	auth = Entry.authType;

	if (Entry.authType == AUTH_SHARED && Entry.encrypt != WIFI_SEC_WEP)
		// shared-key and not WEP enabled, force to open-system
		auth = (AUTH_TYPE_T)AUTH_OPEN;
	#ifdef CONFIG_RTL_WAPI_SUPPORT
	if (Entry.encrypt==WIFI_SEC_WAPI)
		auth = (AUTH_TYPE_T)AUTH_OPEN;
	else 
		status|=va_cmd(IWPRIV, 3, 1, ifname, "set_mib", "wapiType=0");
	#endif

	#ifdef CONFIG_WIFI_SIMPLE_CONFIG //cathy
	if (vwlan_idx == 0) // root
		//clear wsc_enable, this parameter will be set in wsc daemon
		status|=va_cmd(IWPRIV, 3, 1, ifname, "set_mib", "wsc_enable=0");

	wsc_disabled = Entry.wsc_disabled;
	if(vwlan_idx == 0 && Entry.authType == AUTH_SHARED && Entry.encrypt == WIFI_SEC_WEP && wsc_disabled!=1)
		auth = (AUTH_TYPE_T)AUTH_BOTH;	//if root shared-key and wep/wps enable, force to open+shared system for wps
	#endif
	snprintf(parm, sizeof(parm), "authtype=%u", auth);
	status|=va_cmd(IWPRIV, 3, 1, ifname, "set_mib", parm);
	return status;
}
#endif
#ifdef WLAN_FAST_INIT
void setupWlanHWSetting(char *ifname, struct wifi_mib *pmib)
{
		unsigned char value[34];
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN0_5G_SUPPORT) || defined(WLAN1_5G_SUPPORT)
#if defined(CONFIG_RTL_92D_SUPPORT)
		mib_get(MIB_WLAN_MAC_PHY_MODE, (void *)value);
		(&pmib->dot11RFEntry)->macPhyMode = value[0];
#endif
		mib_get(MIB_WLAN_PHY_BAND_SELECT, (void *)value);
		(&pmib->dot11RFEntry)->phyBandSelect = value[0];
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
		mib_get(MIB_HW_11N_TRSWITCH, (void *)value);
		if(value[0] != 0) 
			(&pmib->dot11RFEntry)->trswitch = value[0];	
#endif

#ifdef WLAN_DUALBAND_CONCURRENT
		if(strcmp(ifname ,WLANIF[1]) == 0) // wlan1
			mib_get(MIB_HW_WLAN1_TX_POWER_CCK_A, (void *)value);
		else // wlan0
#endif
		mib_get(MIB_HW_TX_POWER_CCK_A, (void *)value);
	if(value[0]!=0){
		(&pmib->efuseEntry)->enable_efuse = 0;
		memcpy((&pmib->dot11RFEntry)->pwrlevelCCK_A, value, MAX_CHAN_NUM);
	}
	else
		(&pmib->efuseEntry)->enable_efuse = 1;

#ifdef WLAN_DUALBAND_CONCURRENT
	if(strcmp(ifname ,WLANIF[1]) == 0) // wlan1
		mib_get(MIB_HW_WLAN1_TX_POWER_CCK_B, (void *)value);
	else // wlan0
#endif
		mib_get(MIB_HW_TX_POWER_CCK_B, (void *)value);
	if(value[0]!=0)
		memcpy((&pmib->dot11RFEntry)->pwrlevelCCK_B, value, MAX_CHAN_NUM);
		
#ifdef WLAN_DUALBAND_CONCURRENT
	if(strcmp(ifname ,WLANIF[1]) == 0) // wlan1
		mib_get(MIB_HW_WLAN1_TX_POWER_HT40_1S_A, (void *)value);
	else // wlan0
#endif
		mib_get(MIB_HW_TX_POWER_HT40_1S_A, (void *)value);
	if(value[0]!=0)
		memcpy((&pmib->dot11RFEntry)->pwrlevelHT40_1S_A, value, MAX_CHAN_NUM);
		
#ifdef WLAN_DUALBAND_CONCURRENT
	if(strcmp(ifname ,WLANIF[1]) == 0) // wlan1
		mib_get(MIB_HW_WLAN1_TX_POWER_HT40_1S_B, (void *)value);
	else // wlan0
#endif
		mib_get(MIB_HW_TX_POWER_HT40_1S_B, (void *)value);
	if(value[0]!=0)
		memcpy((&pmib->dot11RFEntry)->pwrlevelHT40_1S_B, value, MAX_CHAN_NUM);	

#ifdef WLAN_DUALBAND_CONCURRENT
	if(strcmp(ifname ,WLANIF[1]) == 0) // wlan1
		mib_get(MIB_HW_WLAN1_TX_POWER_HT40_2S, (void *)value);
	else
#endif
		mib_get(MIB_HW_TX_POWER_HT40_2S, (void *)value);
		memcpy((&pmib->dot11RFEntry)->pwrdiffHT40_2S, value, MAX_CHAN_NUM);

#ifdef WLAN_DUALBAND_CONCURRENT
		if(strcmp(ifname ,WLANIF[1]) == 0) // wlan1
			mib_get(MIB_HW_WLAN1_TX_POWER_HT20, (void *)value);
		else
#endif
		mib_get(MIB_HW_TX_POWER_HT20, (void *)value);
		memcpy((&pmib->dot11RFEntry)->pwrdiffHT20, value, MAX_CHAN_NUM);

#ifdef WLAN_DUALBAND_CONCURRENT
	if(strcmp(ifname ,WLANIF[1]) == 0) // wlan1
		mib_get(MIB_HW_WLAN1_TX_POWER_DIFF_OFDM, (void *)value);
	else
#endif
		mib_get(MIB_HW_TX_POWER_DIFF_OFDM, (void *)value);
		memcpy((&pmib->dot11RFEntry)->pwrdiffOFDM, value, MAX_CHAN_NUM);

#ifdef WLAN_DUALBAND_CONCURRENT
	if(strcmp(ifname ,WLANIF[1]) == 0) // wlan1
		mib_get(MIB_HW_WLAN1_11N_TSSI1 , (void *) value);
	else // wlan0
#endif
		mib_get(MIB_HW_11N_TSSI1, (void *)value);

	if(value[0] != 0)
		(&pmib->dot11RFEntry)->tssi1 = value[0];
	
#ifdef WLAN_DUALBAND_CONCURRENT
	if(strcmp(ifname ,WLANIF[1]) == 0) // wlan1
		mib_get(MIB_HW_WLAN1_11N_TSSI2, (void *)value);
	else
#endif
		mib_get(MIB_HW_11N_TSSI2, (void *)value);	

	if(value[0] != 0)
		(&pmib->dot11RFEntry)->tssi2 = value[0];

#ifdef WLAN_DUALBAND_CONCURRENT
	if(strcmp(ifname ,WLANIF[1]) == 0) // wlan1
		mib_get(MIB_HW_WLAN1_11N_THER, (void *)value);
	else
#endif
		mib_get(MIB_HW_11N_THER, (void *)&value);

	if(value[0] != 0)
		(&pmib->dot11RFEntry)->ther = value[0];

#if defined (WLAN_DUALBAND_CONCURRENT)
	if(strcmp(ifname ,WLANIF[1]) == 0) // wlan1
		mib_get(MIB_HW_WLAN1_RF_XCAP, (void *)value);
	else
#endif
		mib_get(MIB_HW_RF_XCAP, (void *)value);
		(&pmib->dot11RFEntry)->xcap = value[0];
}
#else
int setupWlanHWSetting(char *ifname)
{
	int status=0;
	unsigned char value[34];
	char parm[64];
	char mode=0;
	
	if ( mib_get( MIB_TX_POWER, (void *)&mode) == 0) {
   			printf("Get MIB_TX_POWER error!\n");
   			status=-1;
	}

	#ifdef WLAN_DUALBAND_CONCURRENT
	if(strcmp(ifname ,WLANIF[1]) == 0) // wlan1
		mib_get(MIB_HW_WLAN1_TX_POWER_CCK_A, (void *)value);
	else // wlan0
	#endif
		mib_get(MIB_HW_TX_POWER_CCK_A, (void *)value);


	if(value[0] != 0) {
#ifdef CONFIG_ENABLE_EFUSE
		//disable efuse
		status|=va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", "use_efuse=0");
#endif
		status|=iwpriv_cmd(IWPRIV_HS|IWPRIV_HW2G|IWPRIV_TXPOWER, getWlanIfName(), "set_mib", "pwrlevelCCK_A", value, mode);
	}
#ifdef CONFIG_ENABLE_EFUSE
	else {
		//enable efuse
		status|=va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", "use_efuse=1");
	}
#endif

	#ifdef WLAN_DUALBAND_CONCURRENT
	if(strcmp(ifname ,WLANIF[1]) == 0) // wlan1
		mib_get(MIB_HW_WLAN1_TX_POWER_CCK_B, (void *)value);
	else // wlan0
	#endif
		mib_get(MIB_HW_TX_POWER_CCK_B, (void *)value);

	if(value[0] != 0) 
		status|=iwpriv_cmd(IWPRIV_HS|IWPRIV_HW2G|IWPRIV_TXPOWER, getWlanIfName(), "set_mib", "pwrlevelCCK_B", value, mode);

#ifdef CONFIG_WLAN_HAL_8814AE
#ifdef WLAN_DUALBAND_CONCURRENT
	if(strcmp(ifname ,WLANIF[1]) == 0) // wlan1
		mib_get(MIB_HW_WLAN1_TX_POWER_CCK_C, (void *)value);
	else // wlan0
#endif
		mib_get(MIB_HW_TX_POWER_CCK_C, (void *)value);

	if(value[0] != 0) 
		status|=iwpriv_cmd(IWPRIV_HS|IWPRIV_HW2G|IWPRIV_TXPOWER, getWlanIfName(), "set_mib", "pwrlevelCCK_C", value, mode);
#ifdef WLAN_DUALBAND_CONCURRENT
	if(strcmp(ifname ,WLANIF[1]) == 0) // wlan1
		mib_get(MIB_HW_WLAN1_TX_POWER_CCK_D, (void *)value);
	else // wlan0
#endif
		mib_get(MIB_HW_TX_POWER_CCK_D, (void *)value);

	if(value[0] != 0) 
		status|=iwpriv_cmd(IWPRIV_HS|IWPRIV_HW2G|IWPRIV_TXPOWER, getWlanIfName(), "set_mib", "pwrlevelCCK_D", value, mode);
#endif //CONFIG_WLAN_HAL_8814AE


	#ifdef WLAN_DUALBAND_CONCURRENT
	if(strcmp(ifname ,WLANIF[1]) == 0) // wlan1
		mib_get(MIB_HW_WLAN1_TX_POWER_HT40_1S_A, (void *)value);
	else // wlan0
	#endif
		mib_get(MIB_HW_TX_POWER_HT40_1S_A, (void *)value);

	if(value[0] != 0) 
		status|=iwpriv_cmd(IWPRIV_HS|IWPRIV_HW2G|IWPRIV_TXPOWER, getWlanIfName(), "set_mib", "pwrlevelHT40_1S_A", value, mode);

	#ifdef WLAN_DUALBAND_CONCURRENT
	if(strcmp(ifname ,WLANIF[1]) == 0) // wlan1
		mib_get(MIB_HW_WLAN1_TX_POWER_HT40_1S_B, (void *)value);
	else // wlan0
	#endif
		mib_get(MIB_HW_TX_POWER_HT40_1S_B, (void *)value);

	if(value[0] != 0) 
		status|=iwpriv_cmd(IWPRIV_HS|IWPRIV_HW2G|IWPRIV_TXPOWER, getWlanIfName(), "set_mib", "pwrlevelHT40_1S_B", value, mode);

#ifdef CONFIG_WLAN_HAL_8814AE
#ifdef WLAN_DUALBAND_CONCURRENT
	if(strcmp(ifname ,WLANIF[1]) == 0) // wlan1
		mib_get(MIB_HW_WLAN1_TX_POWER_HT40_1S_C, (void *)value);
	else // wlan0
#endif
		mib_get(MIB_HW_TX_POWER_HT40_1S_C, (void *)value);

	if(value[0] != 0) 
		status|=iwpriv_cmd(IWPRIV_HS|IWPRIV_HW2G|IWPRIV_TXPOWER, getWlanIfName(), "set_mib", "pwrlevelHT40_1S_C", value, mode);

#ifdef WLAN_DUALBAND_CONCURRENT
	if(strcmp(ifname ,WLANIF[1]) == 0) // wlan1
		mib_get(MIB_HW_WLAN1_TX_POWER_HT40_1S_D, (void *)value);
	else // wlan0
#endif
		mib_get(MIB_HW_TX_POWER_HT40_1S_D, (void *)value);

	if(value[0] != 0) 
		status|=iwpriv_cmd(IWPRIV_HS|IWPRIV_HW2G|IWPRIV_TXPOWER, getWlanIfName(), "set_mib", "pwrlevelHT40_1S_D", value, mode);
#endif //CONFIG_WLAN_HAL_8814AE



	#ifdef WLAN_DUALBAND_CONCURRENT
	if(strcmp(ifname ,WLANIF[1]) == 0) // wlan1
		mib_get(MIB_HW_WLAN1_TX_POWER_HT40_2S, (void *)value);
	else
	#endif
		mib_get(MIB_HW_TX_POWER_HT40_2S, (void *)value);

	// may be 0 for power difference
	//if(value[0] != 0) {
		status|=iwpriv_cmd(IWPRIV_HS|IWPRIV_HW2G, getWlanIfName(), "set_mib", "pwrdiffHT40_2S", value);
	//}

	#ifdef WLAN_DUALBAND_CONCURRENT
	if(strcmp(ifname ,WLANIF[1]) == 0) // wlan1
		mib_get(MIB_HW_WLAN1_TX_POWER_HT20, (void *)value);
	else
	#endif
		mib_get(MIB_HW_TX_POWER_HT20, (void *)value);

	// may be 0 for power difference
	//if(value[0] != 0) {
		status|=iwpriv_cmd(IWPRIV_HS|IWPRIV_HW2G, getWlanIfName(), "set_mib", "pwrdiffHT20", value);
	//}

	#ifdef WLAN_DUALBAND_CONCURRENT
	if(strcmp(ifname ,WLANIF[1]) == 0) // wlan1
		mib_get(MIB_HW_WLAN1_TX_POWER_DIFF_OFDM, (void *)value);
	else
	#endif
		mib_get(MIB_HW_TX_POWER_DIFF_OFDM, (void *)value);
		
	// may be 0 for power difference
	//if(value[0] != 0) {
		status|=iwpriv_cmd(IWPRIV_HS|IWPRIV_HW2G, getWlanIfName(), "set_mib", "pwrdiffOFDM", value);
	//}

	#ifdef WLAN_DUALBAND_CONCURRENT
	if(strcmp(ifname ,WLANIF[1]) == 0) // wlan1
		mib_get(MIB_HW_WLAN1_11N_TSSI1 , (void *) value);
	else // wlan0
	#endif
		mib_get(MIB_HW_11N_TSSI1, (void *)value);

	if(value[0] != 0)
		status|=iwpriv_cmd(IWPRIV_INT, getWlanIfName(), "set_mib", "tssi1", value[0]);

	#ifdef WLAN_DUALBAND_CONCURRENT
	if(strcmp(ifname ,WLANIF[1]) == 0) // wlan1
		mib_get(MIB_HW_WLAN1_11N_TSSI2, (void *)value);
	else
	#endif
		mib_get(MIB_HW_11N_TSSI2, (void *)value);

	if(value[0] != 0) 
		status|=iwpriv_cmd(IWPRIV_INT, getWlanIfName(), "set_mib", "tssi2", value[0]);

	#ifdef WLAN_DUALBAND_CONCURRENT
	if(strcmp(ifname ,WLANIF[1]) == 0) // wlan1
		mib_get(MIB_HW_WLAN1_11N_THER, (void *)value);
	else
	#endif
		mib_get(MIB_HW_11N_THER, (void *)value);

	if(value[0] != 0) {
		snprintf(parm, sizeof(parm), "ther=%d", value[0]);
		status|=va_cmd(IWPRIV, 3, 1, (char *) getWlanIfName(), "set_mib", parm);
	}


#if defined (WLAN_DUALBAND_CONCURRENT)
	if(strcmp(ifname ,WLANIF[1]) == 0) // wlan1
		mib_get(MIB_HW_WLAN1_RF_XCAP, (void *)value);
	else
#endif
		mib_get(MIB_HW_RF_XCAP, (void *)value);

	status|=iwpriv_cmd(IWPRIV_INT, getWlanIfName(), "set_mib", "xcap", value[0]);
}
#endif
#ifdef WLAN_FAST_INIT
void setup8812Wlan(char *ifname, struct wifi_mib *pmib)
{
	unsigned char buf1[1024];
	struct Dot11RFEntry *rf_entry;
#if defined(CONFIG_WLAN0_5G_WLAN1_2G)
	if(!strcmp(ifname , WLANIF[0])) //wlan0:5G, wlan1:2G
#elif defined(CONFIG_WLAN0_2G_WLAN1_5G)
	if(!strcmp(ifname , WLANIF[1])) //wlan0:2G, wlan1:5G			
#endif
	{
#if ((defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_RTL_8812_SUPPORT)) && !defined(CONFIG_ENABLE_EFUSE)) || defined(WLAN_DUALBAND_CONCURRENT)

		rf_entry = &pmib->dot11RFEntry;

		mib_get(MIB_HW_TX_POWER_5G_HT40_1S_A, (void*) buf1);
		memcpy(rf_entry->pwrlevel5GHT40_1S_A, (unsigned char*) buf1, MAX_5G_CHANNEL_NUM);
		mib_get(MIB_HW_TX_POWER_5G_HT40_1S_B, (void*) buf1);
		memcpy(rf_entry->pwrlevel5GHT40_1S_B, (unsigned char*) buf1, MAX_5G_CHANNEL_NUM);
		// 5G
		mib_get(MIB_HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_A, (void *)buf1);
		assign_diff_AC(rf_entry->pwrdiff_5G_20BW1S_OFDM1T_A, (unsigned char*) buf1);	
		mib_get(MIB_HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_A, (void *)buf1);
		assign_diff_AC(rf_entry->pwrdiff_5G_40BW2S_20BW2S_A, (unsigned char*)buf1);
		mib_get(MIB_HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_A, (void *)buf1);
		assign_diff_AC(rf_entry->pwrdiff_5G_80BW1S_160BW1S_A, (unsigned char*)buf1);
		mib_get(MIB_HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_A, (void *)buf1);
		assign_diff_AC(rf_entry->pwrdiff_5G_80BW2S_160BW2S_A, (unsigned char*)buf1);
	
		mib_get(MIB_HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_B, (void *)buf1);
		assign_diff_AC(rf_entry->pwrdiff_5G_20BW1S_OFDM1T_B, (unsigned char*)buf1);	
		mib_get(MIB_HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_B, (void *)buf1);
		assign_diff_AC(rf_entry->pwrdiff_5G_40BW2S_20BW2S_B, (unsigned char*)buf1);
		mib_get(MIB_HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_B, (void *)buf1);
		assign_diff_AC(rf_entry->pwrdiff_5G_80BW1S_160BW1S_B, (unsigned char*)buf1);
		mib_get(MIB_HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_B, (void *)buf1);
		assign_diff_AC(rf_entry->pwrdiff_5G_80BW2S_160BW2S_B, (unsigned char*)buf1);
	
		// 2G
		mib_get(MIB_HW_TX_POWER_DIFF_20BW1S_OFDM1T_A, (void *)buf1);
		memcpy(rf_entry->pwrdiff_20BW1S_OFDM1T_A, buf1, MAX_CHAN_NUM); 
		mib_get(MIB_HW_TX_POWER_DIFF_40BW2S_20BW2S_A, (void *)buf1);
		memcpy(rf_entry->pwrdiff_40BW2S_20BW2S_A, buf1, MAX_CHAN_NUM);
	
		mib_get(MIB_HW_TX_POWER_DIFF_20BW1S_OFDM1T_B, (void *)buf1);
		memcpy(rf_entry->pwrdiff_20BW1S_OFDM1T_B, buf1, MAX_CHAN_NUM);
		mib_get(MIB_HW_TX_POWER_DIFF_40BW2S_20BW2S_B, (void *)buf1);
		memcpy(rf_entry->pwrdiff_40BW2S_20BW2S_B, buf1, MAX_CHAN_NUM);
#endif
				
	}
}
#else
int setup8812Wlan(	char *ifname)
{
	int i = 0 ;
	int status = 0 ;
	unsigned char value[196];
	char parm[1024];
	int intVal;
	char mode=0;
#if defined(CONFIG_WLAN0_5G_WLAN1_2G)
	if(!strcmp(ifname , WLANIF[1])) //wlan0:5G, wlan1:2G
		return status;
#elif defined(CONFIG_WLAN0_2G_WLAN1_5G)
	if(!strcmp(ifname , WLANIF[0])) //wlan0:2G, wlan1:5G
		return status;
#endif
#if ((defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_RTL_8812_SUPPORT)) && !defined(CONFIG_ENABLE_EFUSE)) || defined(WLAN_DUALBAND_CONCURRENT)
		unsigned char pMib[178];

		if ( mib_get( MIB_TX_POWER, (void *)&mode) == 0) {
   			printf("Get MIB_TX_POWER error!\n");
   			status=-1;
		}
		
		mib_get(MIB_HW_TX_POWER_5G_HT40_1S_A, (void *)value);
		//if(value[0] != 0)
		{
			iwpriv_cmd(IWPRIV_HS | IWPRIV_TXPOWER, ifname, "set_mib", "pwrlevel5GHT40_1S_A", value, mode);
		}

		mib_get(MIB_HW_TX_POWER_5G_HT40_1S_B, (void *)value);
		//if(value[0] != 0)
		{		
			iwpriv_cmd(IWPRIV_HS | IWPRIV_TXPOWER, ifname, "set_mib", "pwrlevel5GHT40_1S_B", value , mode);
		}

		mib_get(MIB_HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_A, (void *)value);
		if(value[0] != 0)
			iwpriv_cmd(IWPRIV_HS, ifname, "set_mib", "pwrdiff_5G_20BW1S_OFDM1T_A", value);

		mib_get(MIB_HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_A, (void *)value);
		if(value[0] != 0)
			iwpriv_cmd(IWPRIV_HS, ifname, "set_mib", "pwrdiff_5G_40BW2S_20BW2S_A", value);

		mib_get(MIB_HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_A, (void *)value);		
		if(value[0] != 0)			
			iwpriv_cmd(IWPRIV_HS, ifname, "set_mib", "pwrdiff_5G_80BW1S_160BW1S_A", value);
		
		mib_get(MIB_HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_A, (void *)value);
		if(value[0] != 0)			
			iwpriv_cmd(IWPRIV_HS, ifname, "set_mib", "pwrdiff_5G_80BW2S_160BW2S_A", value);
		
		mib_get(MIB_HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_B, (void *)value);
		if(value[0] != 0)
			iwpriv_cmd(IWPRIV_HS, ifname, "set_mib", "pwrdiff_5G_20BW1S_OFDM1T_B", value);

		mib_get(MIB_HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_B, (void *)value);
		if(value[0] != 0)
			iwpriv_cmd(IWPRIV_HS, ifname, "set_mib", "pwrdiff_5G_40BW2S_20BW2S_B", value);
		
		mib_get(MIB_HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_B, (void *)value);
		if(value[0] != 0)
			iwpriv_cmd(IWPRIV_HS, ifname, "set_mib", "pwrdiff_5G_80BW1S_160BW1S_B", value);
		
		mib_get(MIB_HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_B, (void *)value);
		if(value[0] != 0)
			iwpriv_cmd(IWPRIV_HS, ifname, "set_mib", "pwrdiff_5G_80BW2S_160BW2S_B", value);
		
		// 2G
		mib_get(MIB_HW_TX_POWER_DIFF_20BW1S_OFDM1T_A, (void *)value);
		if(value[0] != 0)
			iwpriv_cmd(IWPRIV_HS|IWPRIV_HW2G, ifname, "set_mib", "pwrdiff_20BW1S_OFDM1T_A", value);

		mib_get(MIB_HW_TX_POWER_DIFF_40BW2S_20BW2S_A, (void *)value);
		if(value[0] != 0)
			iwpriv_cmd(IWPRIV_HS|IWPRIV_HW2G, ifname, "set_mib", "pwrdiff_40BW2S_20BW2S_A", value);
		
		mib_get(MIB_HW_TX_POWER_DIFF_20BW1S_OFDM1T_B, (void *)value);
		if(value[0] != 0)
			iwpriv_cmd(IWPRIV_HS|IWPRIV_HW2G, ifname, "set_mib", "pwrdiff_20BW1S_OFDM1T_B", value);
		
		mib_get(MIB_HW_TX_POWER_DIFF_40BW2S_20BW2S_B, (void *)value);
		if(value[0] != 0)
			iwpriv_cmd(IWPRIV_HS|IWPRIV_HW2G, ifname, "set_mib", "pwrdiff_40BW2S_20BW2S_B", value);
		
#endif
#if defined(CONFIG_WLAN_HAL_8814AE)

		mib_get(MIB_HW_TX_POWER_5G_HT40_1S_D, (void *)value);
		//if(value[0] != 0)
		{		
			iwpriv_cmd(IWPRIV_HS | IWPRIV_TXPOWER, ifname, "set_mib", "pwrlevel5GHT40_1S_D", value , mode);
		}

		mib_get(MIB_HW_TX_POWER_5G_HT40_1S_C, (void *)value);
		//if(value[0] != 0)
		{
			iwpriv_cmd(IWPRIV_HS | IWPRIV_TXPOWER, ifname, "set_mib", "pwrlevel5GHT40_1S_C", value, mode);
		}

		mib_get(MIB_HW_TX_POWER_5G_HT40_1S_D, (void *)value);
		//if(value[0] != 0)
		{		
			iwpriv_cmd(IWPRIV_HS | IWPRIV_TXPOWER, ifname, "set_mib", "pwrlevel5GHT40_1S_D", value , mode);
		}

		mib_get(MIB_HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_C, (void *)value);
		if(value[0] != 0)
			iwpriv_cmd(IWPRIV_HS, ifname, "set_mib", "pwrdiff_5G_20BW1S_OFDM1T_C", value);

		mib_get(MIB_HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_C, (void *)value);
		if(value[0] != 0)
			iwpriv_cmd(IWPRIV_HS, ifname, "set_mib", "pwrdiff_5G_40BW2S_20BW2S_C", value);

		mib_get(MIB_HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_C, (void *)value);		
		if(value[0] != 0)			
			iwpriv_cmd(IWPRIV_HS, ifname, "set_mib", "pwrdiff_5G_80BW1S_160BW1S_C", value);
		
		mib_get(MIB_HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_C, (void *)value);
		if(value[0] != 0)			
			iwpriv_cmd(IWPRIV_HS, ifname, "set_mib", "pwrdiff_5G_80BW2S_160BW2S_C", value);
		
		mib_get(MIB_HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_D, (void *)value);
		if(value[0] != 0)
			iwpriv_cmd(IWPRIV_HS, ifname, "set_mib", "pwrdiff_5G_20BW1S_OFDM1T_D", value);

		mib_get(MIB_HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_D, (void *)value);
		if(value[0] != 0)
			iwpriv_cmd(IWPRIV_HS, ifname, "set_mib", "pwrdiff_5G_40BW2S_20BW2S_D", value);
		
		mib_get(MIB_HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_D, (void *)value);
		if(value[0] != 0)
			iwpriv_cmd(IWPRIV_HS, ifname, "set_mib", "pwrdiff_5G_80BW1S_160BW1S_D", value);
		
		mib_get(MIB_HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_D, (void *)value);
		if(value[0] != 0)
			iwpriv_cmd(IWPRIV_HS, ifname, "set_mib", "pwrdiff_5G_80BW2S_160BW2S_D", value);
		
		// 2G
		mib_get(MIB_HW_TX_POWER_DIFF_20BW1S_OFDM1T_C, (void *)value);
		if(value[0] != 0)
			iwpriv_cmd(IWPRIV_HS|IWPRIV_HW2G, ifname, "set_mib", "pwrdiff_20BW1S_OFDM1T_C", value);

		mib_get(MIB_HW_TX_POWER_DIFF_40BW2S_20BW2S_C, (void *)value);
		if(value[0] != 0)
			iwpriv_cmd(IWPRIV_HS|IWPRIV_HW2G, ifname, "set_mib", "pwrdiff_40BW2S_20BW2S_C", value);
		
		mib_get(MIB_HW_TX_POWER_DIFF_20BW1S_OFDM1T_D, (void *)value);
		if(value[0] != 0)
			iwpriv_cmd(IWPRIV_HS|IWPRIV_HW2G, ifname, "set_mib", "pwrdiff_20BW1S_OFDM1T_D", value);
		
		mib_get(MIB_HW_TX_POWER_DIFF_40BW2S_20BW2S_D, (void *)value);
		if(value[0] != 0)
			iwpriv_cmd(IWPRIV_HS|IWPRIV_HW2G, ifname, "set_mib", "pwrdiff_40BW2S_20BW2S_D", value);
			
#endif

	return status;
}
#endif
#if defined WLAN_QoS && !defined CONFIG_RTL8192CD
int setupWLanQos(char *argv[6])
{
	int i, status;
	unsigned char value[34];
	char parm[64];
	MIB_WLAN_QOS_T QOSEntry;

	for (i=0; i<4; i++) {
		if (!mib_chain_get(MIB_WLAN_QOS_AP_TBL, i, (void *)&QOSEntry)) {
  			printf("Error! Get MIB_WLAN_AP_QOS_TBL error.\n");
  			continue;
		}
		switch(i){
			case 0://VO
				value[0] = QOSEntry.txop;
				snprintf(parm,sizeof(parm),"vo_txop=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
				value[0] = QOSEntry.ecwmax;
				snprintf(parm,sizeof(parm),"vo_ecwmax=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
				value[0] = QOSEntry.ecwmin;
				snprintf(parm,sizeof(parm),"vo_ecwmin=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
				value[0] = QOSEntry.aifsn;
				snprintf(parm,sizeof(parm),"vo_aifsn=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
				break;
			case 1://VI
				value[0] = QOSEntry.txop;
				snprintf(parm,sizeof(parm),"vi_txop=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
				value[0] = QOSEntry.ecwmax;
				snprintf(parm,sizeof(parm),"vi_ecwmax=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
				value[0] = QOSEntry.ecwmin;
				snprintf(parm,sizeof(parm),"vi_ecwmin=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
				value[0] = QOSEntry.aifsn;
				snprintf(parm,sizeof(parm),"vi_aifsn=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
				break;
			case 2://BE
				value[0] = QOSEntry.txop;
				snprintf(parm,sizeof(parm),"be_txop=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
				value[0] = QOSEntry.ecwmax;
				snprintf(parm,sizeof(parm),"be_ecwmax=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
				value[0] = QOSEntry.ecwmin;
				snprintf(parm,sizeof(parm),"be_ecwmin=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
				value[0] = QOSEntry.aifsn;
				snprintf(parm,sizeof(parm),"be_aifsn=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
				break;
			case 3://BK
				value[0] = QOSEntry.txop;
				snprintf(parm,sizeof(parm),"bk_txop=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
				value[0] = QOSEntry.ecwmax;
				snprintf(parm,sizeof(parm),"bk_ecwmax=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
				value[0] = QOSEntry.ecwmin;
				snprintf(parm,sizeof(parm),"bk_ecwmin=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
				value[0] = QOSEntry.aifsn;
				snprintf(parm,sizeof(parm),"bk_aifsn=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
				break;
			default:
				break;
		}
	}

	//sta
	for (i=0; i<4; i++) {
		if (!mib_chain_get(MIB_WLAN_QOS_STA_TBL, i, (void *)&QOSEntry)) {
  			printf("Error! Get MIB_WLAN_STA_QOS_TBL error.\n");
  			continue;
		}
		switch(i){
			case 0://VO
				value[0] = QOSEntry.txop;
				snprintf(parm,sizeof(parm),"sta_vo_txop=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
				value[0] = QOSEntry.ecwmax;
				snprintf(parm,sizeof(parm),"sta_vo_ecwmax=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
				value[0] = QOSEntry.ecwmin;
				snprintf(parm,sizeof(parm),"sta_vo_ecwmin=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
				value[0] = QOSEntry.aifsn;
				snprintf(parm,sizeof(parm),"sta_vo_aifsn=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
				break;
			case 1://VI
				value[0] = QOSEntry.txop;
				snprintf(parm,sizeof(parm),"sta_vi_txop=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
				value[0] = QOSEntry.ecwmax;
				snprintf(parm,sizeof(parm),"sta_vi_ecwmax=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
				value[0] = QOSEntry.ecwmin;
				snprintf(parm,sizeof(parm),"sta_vi_ecwmin=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
				value[0] = QOSEntry.aifsn;
				snprintf(parm,sizeof(parm),"sta_vi_aifsn=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
				break;
			case 2://BE
				value[0] = QOSEntry.txop;
				snprintf(parm,sizeof(parm),"sta_be_txop=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
				value[0] = QOSEntry.ecwmax;
				snprintf(parm,sizeof(parm),"sta_be_ecwmax=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
				value[0] = QOSEntry.ecwmin;
				snprintf(parm,sizeof(parm),"sta_be_ecwmin=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
				value[0] = QOSEntry.aifsn;
				snprintf(parm,sizeof(parm),"sta_be_aifsn=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
				break;
			case 3://BK
				value[0] = QOSEntry.txop;
				snprintf(parm,sizeof(parm),"sta_bk_txop=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
				value[0] = QOSEntry.ecwmax;
				snprintf(parm,sizeof(parm),"sta_bk_ecwmax=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
				value[0] = QOSEntry.ecwmin;
				snprintf(parm,sizeof(parm),"sta_bk_ecwmin=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
				value[0] = QOSEntry.aifsn;
				snprintf(parm,sizeof(parm),"sta_bk_aifsn=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
				break;
			default:
				break;
		}
	}
	return status;
}
#endif

#ifdef WLAN_FAST_INIT
static int setupWLan(char *ifname, int vwlan_idx)
{
	struct wifi_mib *pmib;
	int i, skfd, intVal;
	unsigned int vInt;
	unsigned short int sInt;
	unsigned char buf1[1024];
	unsigned char value[196];
	struct iwreq wrq, wrq_root;
	MIB_CE_MBSSIB_T Entry;
	unsigned char wlan_mode, vChar, band, phyband;
	int vap_enable=0;
#ifdef WLAN_UNIVERSAL_REPEATER
	char rpt_enabled;
#endif

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd < 0)
		return -1;

    // Get wireless name
    if ( iw_get_ext(skfd, ifname, SIOCGIWNAME, &wrq) < 0) {
      close( skfd );
      // If no wireless name : no wireless extensions
      return -1;
    }

	if ((pmib = (struct wifi_mib *)malloc(sizeof(struct wifi_mib))) == NULL) {
		printf("MIB buffer allocation failed!\n");
		close(skfd);
		return -1;
	}

	// Disable WLAN MAC driver and shutdown interface first
	//sprintf((char *)buf1, "ifconfig %s down", ifname);
	//system((char *)buf1);
	
	//va_cmd(IFCONFIG, 2, 1, ifname, "down");

	if (vwlan_idx == 0) {
		// shutdown all WDS interface
		#if 0 //def WLAN_WDS
		for (i=0; i<8; i++) {
			//sprintf((char *)buf1, "ifconfig %s-wds%d down", ifname, i);
			//system((char *)buf1);
			sprintf((char *)buf1, "%s-wds%d", ifname, i);
			va_cmd(IFCONFIG, 2, 1, buf1, "down");
		}
		#endif
		// kill wlan application daemon
		//sprintf((char *)buf1, "wlanapp.sh kill %s", ifname);
		//system((char *)buf1);
	}
	else { // virtual interface
		snprintf((char *)buf1, sizeof(buf1), "wlan%d", wlan_idx);
		strncpy(wrq_root.ifr_name, (char *)buf1, IFNAMSIZ);
		if (ioctl(skfd, SIOCGIWNAME, &wrq_root) < 0) {
			printf("Root Interface %s open failed!\n", buf1);
			free(pmib);
			close(skfd);
			return -1;
		}
	}

#if 0
	if (vwlan_idx == 0) {
		mib_get(MIB_HW_RF_TYPE, (void *)&vChar);
		if (vChar == 0) {
			printf("RF type is NULL!\n");
			free(pmib);
			close(skfd);
			return 0;
		}
	}
#endif
	if(!mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
		printf("Error! Get MIB_MBSSIB_TBL for root SSID error.\n");

	vChar = Entry.wlanDisabled;

	if (vChar == 1) {
		free(pmib);
		close(skfd);
		return 0;
	}

	// get mib from driver
	wrq.u.data.pointer = (caddr_t)pmib;
	wrq.u.data.length = sizeof(struct wifi_mib);

	if (vwlan_idx == 0) {
		if (ioctl(skfd, SIOCMIBINIT, &wrq) < 0) {
			printf("Get WLAN MIB failed!\n");
			free(pmib);
			close(skfd);
			return -1;
		}
	}
	else {
		wrq_root.u.data.pointer = (caddr_t)pmib;
		wrq_root.u.data.length = sizeof(struct wifi_mib);				
		if (ioctl(skfd, SIOCMIBINIT, &wrq_root) < 0) {
			printf("Get WLAN MIB failed!\n");
			free(pmib);
			close(skfd);
			return -1;
		}		
	}

	// check mib version
	if (pmib->mib_version != MIB_VERSION) {
		printf("WLAN MIB version mismatch!\n");
		free(pmib);
		close(skfd);
		return -1;
	}

	if (vwlan_idx > 0) {	
		//if not root interface, clone root mib to virtual interface
		wrq.u.data.pointer = (caddr_t)pmib;
		wrq.u.data.length = sizeof(struct wifi_mib);
		if (ioctl(skfd, SIOCMIBSYNC, &wrq) < 0) {
			printf("Set WLAN MIB failed!\n");
			free(pmib);
			close(skfd);
			return -1;
		}
	}	
	//pmib->miscEntry.func_off = 0;	
	if (!mib_chain_get(MIB_MBSSIB_TBL, vwlan_idx, (void *)&Entry)) {
			printf("Error! Get MIB_MBSSIB_TBL for VAP SSID error.\n");
		free(pmib);
		close(skfd);
		return -1;
	}
	


#ifdef WLAN_MBSSID
	if(vwlan_idx > 0 && vwlan_idx < WLAN_REPEATER_ITF_INDEX)
#endif
	{
		struct sockaddr hwaddr;
		getInAddr(ifname, HW_ADDR, &hwaddr);
		memcpy(&(pmib->dot11OperationEntry.hwaddr[0]), hwaddr.sa_data, 6);
	}
	if (vwlan_idx == 0) {	//root
		mib_get(MIB_WIFI_TEST, (void *)value);
		if(value[0])
			va_cmd(IWPRIV, 3, 1, ifname, "set_mib", "mp_specific=1");
		else
			va_cmd(IWPRIV, 3, 1, ifname, "set_mib", "mp_specific=0");
		
		mib_get(MIB_HW_REG_DOMAIN, (void *)&vChar);
		pmib->dot11StationConfigEntry.dot11RegDomain = vChar;

		//mib_get(MIB_HW_LED_TYPE, (void *)&vChar);
#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
		pmib->dot11OperationEntry.ledtype = 11;	
#else
		pmib->dot11OperationEntry.ledtype = 3;	
#endif
	}

	wlan_mode = Entry.wlanMode;

	//SSID setting
	memset(pmib->dot11StationConfigEntry.dot11DesiredSSID, 0, 32);
	memset(pmib->dot11StationConfigEntry.dot11SSIDtoScan, 0, 32);
	
	if(vwlan_idx == 0 || (vwlan_idx > 0 && vwlan_idx < WLAN_REPEATER_ITF_INDEX && (wlan_mode == AP_MODE || wlan_mode == AP_WDS_MODE))){
		pmib->dot11StationConfigEntry.dot11DesiredSSIDLen = strlen((char *)Entry.ssid);
		memcpy(pmib->dot11StationConfigEntry.dot11DesiredSSID, Entry.ssid, strlen((char *)Entry.ssid));
		pmib->dot11StationConfigEntry.dot11SSIDtoScanLen = strlen((char *)Entry.ssid);
		memcpy(pmib->dot11StationConfigEntry.dot11SSIDtoScan, Entry.ssid, strlen((char *)Entry.ssid));
	}
#ifdef WLAN_UNIVERSAL_REPEATER
	else if(vwlan_idx == WLAN_REPEATER_ITF_INDEX){
		mib_get( MIB_REPEATER_ENABLED1, (void *)&rpt_enabled);
		if (rpt_enabled) {
			pmib->dot11StationConfigEntry.dot11DesiredSSIDLen = strlen((char *)Entry.ssid);
			memcpy(pmib->dot11StationConfigEntry.dot11DesiredSSID, Entry.ssid, strlen((char *)Entry.ssid));
			pmib->dot11StationConfigEntry.dot11SSIDtoScanLen = strlen((char *)Entry.ssid);
			memcpy(pmib->dot11StationConfigEntry.dot11SSIDtoScan, Entry.ssid, strlen((char *)Entry.ssid));
		}
	}
#endif

#ifdef WLAN_MBSSID
	if(vwlan_idx==0){		
		// opmode
		if (wlan_mode == CLIENT_MODE)
			vInt = 8;	// client
		else	// 0(AP_MODE) or 3(AP_WDS_MODE)
			vInt = 16;	// AP
		pmib->dot11OperationEntry.opmode = vInt;

		if(pmib->dot11OperationEntry.opmode & 0x00000010){// AP mode
			for (vwlan_idx = 1; vwlan_idx < (WLAN_MBSSID_NUM+1); vwlan_idx++) {
				mib_chain_get(MIB_MBSSIB_TBL, vwlan_idx, (void *)&Entry);
				if (Entry.wlanDisabled == 0)
					vap_enable++;
			}
			vwlan_idx = 0;
		}
		if (vap_enable && (wlan_mode ==  AP_MODE || wlan_mode ==  AP_WDS_MODE))	
			pmib->miscEntry.vap_enable=1;
		else
			pmib->miscEntry.vap_enable=0;
	}
#endif
#ifdef WLAN_UNIVERSAL_REPEATER
	// Repeater opmode
	if(vwlan_idx == WLAN_REPEATER_ITF_INDEX){
		if (rpt_enabled) {
			if (wlan_mode == CLIENT_MODE)
				vInt = 16;	// Repeater is AP
			else
				vInt = 8;	// Repeater is Client
				
			pmib->dot11OperationEntry.opmode = vInt;
		}
	}
#endif

	if (vwlan_idx == 0) //root
	{
		//hw setting
		setupWlanHWSetting(ifname, pmib);
		setup8812Wlan(ifname, pmib);

		// tw power scale
		mib_get(MIB_TX_POWER, (void *)&vChar);
		intVal = getTxPowerScale(vChar);

		if (intVal) {
			for (i=0; i<MAX_CHAN_NUM; i++) {
				if(pmib->dot11RFEntry.pwrlevelCCK_A[i] != 0){ 
					if ((pmib->dot11RFEntry.pwrlevelCCK_A[i] - intVal) >= 1)
						pmib->dot11RFEntry.pwrlevelCCK_A[i] -= intVal;
					else
						pmib->dot11RFEntry.pwrlevelCCK_A[i] = 1;
				}
				if(pmib->dot11RFEntry.pwrlevelCCK_B[i] != 0){ 
					if ((pmib->dot11RFEntry.pwrlevelCCK_B[i] - intVal) >= 1)
						pmib->dot11RFEntry.pwrlevelCCK_B[i] -= intVal;
					else
						pmib->dot11RFEntry.pwrlevelCCK_B[i] = 1;
				}
				if(pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] != 0){ 
					if ((pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] - intVal) >= 1)
						pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] -= intVal;
					else
						pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] = 1;
				}
				if(pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] != 0){ 
					if ((pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] - intVal) >= 1)
						pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] -= intVal;
					else
						pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] = 1;
				}
			}	
			
#if ((defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_RTL_8812_SUPPORT)) && !defined(CONFIG_ENABLE_EFUSE)) || defined(WLAN_DUALBAND_CONCURRENT)
			for (i=0; i<MAX_5G_CHANNEL_NUM; i++) {
				if(pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] != 0){ 
					if ((pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] - intVal) >= 1)
						pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] -= intVal;
					else
						pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] = 1;					
				}
				if(pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] != 0){ 
					if ((pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] - intVal) >= 1)
						pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] -= intVal;
					else
						pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] = 1;
				}
			}
#endif					
		}	
	
		mib_get(MIB_WLAN_BEACON_INTERVAL, (void *)&sInt);
		pmib->dot11StationConfigEntry.dot11BeaconPeriod = sInt;

		mib_get(MIB_WLAN_AUTO_CHAN_ENABLED, (void *)value);
		if(value[0])
			pmib->dot11RFEntry.dot11channel = 0;
		else{
			mib_get(MIB_WLAN_CHAN_NUM, (void *)value);
			pmib->dot11RFEntry.dot11channel = value[0];
		}
		band = Entry.wlanBand;
#ifdef WIFI_TEST	
		if(band == 4) {// WiFi-G
			pmib->dot11StationConfigEntry.dot11BasicRates = 0x15f;
			pmib->dot11StationConfigEntry.dot11SupportedRates = 0xfff;
		}
		else if (band == 5){ // WiFi-BG
			pmib->dot11StationConfigEntry.dot11BasicRates = 0x00f;
			pmib->dot11StationConfigEntry.dot11SupportedRates = 0xfff;
		}
		else
#endif
		{
			mib_get(MIB_WIFI_SUPPORT, (void*)value);
			if(value[0] == 1){
				if(band == 2) {// WiFi-G
					pmib->dot11StationConfigEntry.dot11BasicRates = 0x15f;
					pmib->dot11StationConfigEntry.dot11SupportedRates = 0xfff;
				}
				else if(band == 3){ // WiFi-BG
					pmib->dot11StationConfigEntry.dot11BasicRates = 0x00f;
					pmib->dot11StationConfigEntry.dot11SupportedRates = 0xfff;
				}
				else{
					mib_get(MIB_WLAN_BASIC_RATE, (void *)&sInt);
					pmib->dot11StationConfigEntry.dot11BasicRates = sInt;
					mib_get(MIB_WLAN_SUPPORTED_RATE, (void *)&sInt);
					pmib->dot11StationConfigEntry.dot11SupportedRates = sInt;
				}
			}
			else{
				mib_get(MIB_WLAN_BASIC_RATE, (void *)&sInt);
				pmib->dot11StationConfigEntry.dot11BasicRates = sInt;
				mib_get(MIB_WLAN_SUPPORTED_RATE, (void *)&sInt);
				pmib->dot11StationConfigEntry.dot11SupportedRates = sInt;
				
			}
		}

		mib_get(MIB_WLAN_INACTIVITY_TIME, (void *)&vInt);
		pmib->dot11OperationEntry.expiretime = vInt;
		mib_get(MIB_WLAN_PREAMBLE_TYPE, (void *)&vChar);
		pmib->dot11RFEntry.shortpreamble = vChar;
		vChar = Entry.hidessid;
		pmib->dot11OperationEntry.hiddenAP = vChar;
		mib_get(MIB_WLAN_DTIM_PERIOD, (void *)&vChar);
		pmib->dot11StationConfigEntry.dot11DTIMPeriod = vChar;

#ifdef WLAN_ACL
		set_wlan_acl(pmib);
#endif

#ifdef WLAN_WDS
		setupWDS(pmib);
#endif
	} // root
#ifdef WLAN_UNIVERSAL_REPEATER
	if(vwlan_idx == WLAN_REPEATER_ITF_INDEX && !rpt_enabled){}
#endif
	{
		//auth
		setupWLan_dot11_auth(pmib, vwlan_idx);
#ifdef WLAN_WPA
		setupWLan_WPA(pmib, vwlan_idx);
#endif
		setupWLan_802_1x(pmib, vwlan_idx);
	}	
#ifdef WLAN_UNIVERSAL_REPEATER
	if(vwlan_idx != WLAN_REPEATER_ITF_INDEX)
#endif
	{
		mib_get(MIB_WLAN_RTS_THRESHOLD, (void *)&sInt);
		pmib->dot11OperationEntry.dot11RTSThreshold = sInt;
		mib_get(MIB_WLAN_FRAG_THRESHOLD, (void *)&sInt);
		pmib->dot11OperationEntry.dot11FragmentationThreshold = sInt;
		
		// band
		if (!mib_chain_get(MIB_MBSSIB_TBL, vwlan_idx, (void *)&Entry)) {
				printf("Error! Get MIB_MBSSIB_TBL for VAP SSID error.\n");
		}
		band = Entry.wlanBand;
#ifdef WIFI_TEST
		if (band == 4) // WiFi-G
			band = 3; // 2.4 GHz (B+G)
		else if (band == 5) // WiFi-BG
			band = 3; // 2.4 GHz (B+G)
#endif
		mib_get(MIB_WIFI_SUPPORT, (void*)&vChar);
		if(vChar==1) {
			if(band == 2)
				band = 3;
		}
		if (band == 8) { //pure 11n
			mib_get(MIB_WLAN_PHY_BAND_SELECT, (void *)&phyband);
			if(phyband == PHYBAND_5G) {//5G
				band += 4; // a
				vChar = 4;
			}
			else{
				band += 3;	//b+g+n
				vChar = 3;
			}
		}
		else if (band == 2) {	//pure 11g
			band += 1;	//b+g
			vChar = 1;
		}
		else if (band == 10) {	//g+n
			band += 1;	//b+g+n
			vChar = 1;
		}
		else if(band == 64) {	//pure 11ac
			band += 12; 	//a+n
			vChar = 12;
		}
		else if(band == 72) {	//n+ac
			band += 4; 	//a
			vChar = 4;
		}
		else vChar = 0;

		pmib->dot11BssType.net_work_type = band;
		pmib->dot11StationConfigEntry.legacySTADeny = vChar;
		pmib->dot11nConfigEntry.dot11nLgyEncRstrct = 15;
		pmib->dot11OperationEntry.wifi_specific = 2;
		pmib->dot11ErpInfo.ctsToSelf = 1;
		
		if(vwlan_idx==0){
			value[0] = Entry.rateAdaptiveEnabled;
			if (value[0] == 0) {
				pmib->dot11StationConfigEntry.autoRate = 0;
				vInt = Entry.fixedTxRate;
				pmib->dot11StationConfigEntry.fixedTxRate = vInt;
			}
			else
				pmib->dot11StationConfigEntry.autoRate = 1;
			pmib->dot11OperationEntry.block_relay = Entry.userisolation;
			setup_wlan_block();

#ifdef WIFI_TEST
			band = Entry.wlanBand;
			if (band == 4 || band == 5) {// WiFi-G or WiFi-BG
				pmib->dot11OperationEntry.block_relay = 0;
				pmib->dot11OperationEntry.wifi_specific = 1;
			}
#endif
			//jim do wifi test tricky,
			//    1 for wifi logo test,
			//    0 for normal case...
			mib_get(MIB_WIFI_SUPPORT, (void*)&vChar);
			if(vChar==1){
				band = Entry.wlanBand;
				if (band == 2 || band == 3) {// WiFi-G or WiFi-BG
					pmib->dot11OperationEntry.block_relay = 0;
					pmib->dot11OperationEntry.wifi_specific = 1;
				}
				else {// WiFi-11N
				    printf("In MIB_WLAN_BAND = 2 or 3\n");
					pmib->dot11OperationEntry.block_relay = 0;
					pmib->dot11OperationEntry.wifi_specific = 2;
				}
			}
#ifdef WLAN_QoS			
			pmib->dot11QosEntry.dot11QosEnable = Entry.wmmEnabled;
#endif
		}
		else{
			pmib->dot11StationConfigEntry.autoRate = Entry.rateAdaptiveEnabled;
			if(Entry.rateAdaptiveEnabled == 0)
				pmib->dot11StationConfigEntry.fixedTxRate = Entry.fixedTxRate;
			pmib->dot11OperationEntry.hiddenAP = Entry.hidessid;
			pmib->dot11QosEntry.dot11QosEnable = Entry.wmmEnabled;
			pmib->dot11OperationEntry.block_relay = Entry.userisolation;	
		}
		
		mib_get(MIB_WLAN_PROTECTION_DISABLED, (void *)value);
		pmib->dot11StationConfigEntry.protectionDisabled = value[0];

		//Channel Width
		mib_get(MIB_WLAN_CHANNEL_WIDTH, (void *)value);
		pmib->dot11nConfigEntry.dot11nUse40M = value[0];

		//Conntrol Sideband
		if(value[0]==0) {	//20M
			pmib->dot11nConfigEntry.dot11n2ndChOffset = 0;
		}
		else {	//40M
			mib_get(MIB_WLAN_CONTROL_BAND, (void *)value);
			if(value[0]==0)	//upper
				pmib->dot11nConfigEntry.dot11n2ndChOffset = 1;
			else	//lower
				pmib->dot11nConfigEntry.dot11n2ndChOffset = 2;
#if defined(CONFIG_WLAN_HAL_8814AE) || defined (CONFIG_RTL_8812_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
			mib_get(MIB_WLAN_AUTO_CHAN_ENABLED, &vChar);
			mib_get(MIB_WLAN_CHAN_NUM, (void *)value);
			if(vChar == 0 && value[0] > 14){
				printf("!!! adjust 5G 2ndoffset for 8812 !!!\n");
				if(value[0]==36 || value[0]==44 || value[0]==52 || value[0]==60)
					pmib->dot11nConfigEntry.dot11n2ndChOffset = 2;
				else
					pmib->dot11nConfigEntry.dot11n2ndChOffset = 1;
				
			}
#endif
		}
		//11N Co-Existence
		mib_get(MIB_WLAN_11N_COEXIST, (void *)value);
		pmib->dot11nConfigEntry.dot11nCoexist = value[0];
		
		//short GI
		mib_get(MIB_WLAN_SHORTGI_ENABLED, (void *)value);
		pmib->dot11nConfigEntry.dot11nShortGIfor20M = value[0];
		pmib->dot11nConfigEntry.dot11nShortGIfor40M = value[0];

		//aggregation
		mib_get(MIB_WLAN_AGGREGATION, (void *)value);
		pmib->dot11nConfigEntry.dot11nAMPDU = (value[0])? 1:0;
		pmib->dot11nConfigEntry.dot11nAMSDU = 0;

	}
#ifdef CONFIG_RTL_WAPI_SUPPORT
	if(vwlan_idx==0){
		mib_get(MIB_WLAN_WAPI_UCAST_REKETTYPE, (void *)&vChar);
		pmib->wapiInfo.wapiUpdateUCastKeyType = vChar;
		if (vChar!=1) {
			mib_get(MIB_WLAN_WAPI_UCAST_TIME, (void *)&vInt);
			pmib->wapiInfo.wapiUpdateUCastKeyTimeout = vInt;
			mib_get(MIB_WLAN_WAPI_UCAST_PACKETS, (void *)&vInt);
			pmib->wapiInfo.wapiUpdateUCastKeyPktNum = vInt;
		}
		
		mib_get(MIB_WLAN_WAPI_MCAST_REKEYTYPE, (void *)&vChar);
		pmib->wapiInfo.wapiUpdateMCastKeyType = vChar;
		if (vChar!=1) {
			mib_get(MIB_WLAN_WAPI_MCAST_TIME, (void *)&vInt);
			pmib->wapiInfo.wapiUpdateMCastKeyTimeout = vInt;
			mib_get(MIB_WLAN_WAPI_MCAST_PACKETS, (void *)&vInt);
			pmib->wapiInfo.wapiUpdateMCastKeyPktNum = vInt;
		}
	}
#endif

	//sync mib
	wrq.u.data.pointer = (caddr_t)pmib;
	wrq.u.data.length = sizeof(struct wifi_mib);
	if (ioctl(skfd, SIOCMIBSYNC, &wrq) < 0) {
		printf("Set WLAN MIB failed!\n");
		free(pmib);
		close(skfd);
		return -1;
	}
	close(skfd);

	free(pmib);
	return 0;
}
#else
// return value:
// 0  : success
// -1 : failed
int setupWLan()
{
	char *argv[6];
	unsigned char value[34], phyband;
	char parm[64], para2[15];
	int i, vInt, autoRate, autoRateRoot;
	unsigned char intf_map=1;
	unsigned char wlan_mode;
	// Added by Mason Yu for Set TxPower
	char mode=0;
	int status=0;
	unsigned char stringbuf[MAX_SSID_LEN + 1];
	unsigned char rootSSID[MAX_SSID_LEN + 1];
	int j=0;
	MIB_CE_MBSSIB_T Entry;
	int intVal;
#ifdef WLAN_UNIVERSAL_REPEATER
	char rpt_enabled;
#endif

	// ifconfig wlan0 hw ether 00e04c867002
	/*if (mib_get(MIB_WLAN_MAC_ADDR, (void *)value) != 0)
	{
		snprintf(macaddr, 13, "%02x%02x%02x%02x%02x%02x",
			value[0], value[1], value[2], value[3], value[4], value[5]);
		argv[1]=(char *)WLANIF;
		argv[2]="hw";
		argv[3]="ether";
		argv[4]=macaddr;
		argv[5]=NULL;
		TRACE(STA_SCRIPT, "%s %s %s %s %s\n", IFCONFIG, argv[1], argv[2], argv[3], argv[4]);
		do_cmd(IFCONFIG, argv, 1);
	}*/

	if(!mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
		printf("Error! Get MIB_MBSSIB_TBL for root SSID error.\n");

	wlan_mode = Entry.wlanMode;
#ifdef WLAN_UNIVERSAL_REPEATER
	mib_get( MIB_REPEATER_ENABLED1, (void *)&rpt_enabled);
#endif

	argv[1] = (char*)getWlanIfName();
	argv[2] = "set_mib";

	status|=iwpriv_cmd(IWPRIV_GETMIB | IWPRIV_INT, getWlanIfName(), "set_mib", "mp_specific", MIB_WIFI_TEST);

	status|=iwpriv_cmd(IWPRIV_GETMIB | IWPRIV_INT, getWlanIfName(), "set_mib", "regdomain", MIB_HW_REG_DOMAIN);


#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
	status|=va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", "led_type=11");
#else
	// e8 spec need led_type=7
	status|=va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", "led_type=7");
#endif

	// ssid
	// Modified by Mason Yu
	// Root AP's SSID
	snprintf(parm, sizeof(parm), "ssid=%s", Entry.ssid);
	status|=va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", parm);

#ifdef WLAN_MBSSID
	// VAP's SSID
	if (wlan_mode == AP_MODE || wlan_mode == AP_WDS_MODE) {
		for (j=1; j<=4; j++) {
			if (!mib_chain_get(MIB_MBSSIB_TBL, j, (void *)&Entry)) {
  				printf("Error! Get MIB_MBSSIB_TBL for VAP SSID error.\n");
			}

 			//if ( Entry.wlanDisabled == 1 ) {
				snprintf(para2, sizeof(para2), "%s-vap%d", getWlanIfName(), j-1);
				snprintf(parm, sizeof(parm), "ssid=%s", Entry.ssid);
				status|=va_cmd(IWPRIV, 3, 1, para2, "set_mib", parm);
			//}

			if (!Entry.wlanDisabled) {
				intf_map |= (1 << j);
			}
		}
		argv[1] = (char*)getWlanIfName();
	}
#endif
#ifdef WLAN_UNIVERSAL_REPEATER
	if (rpt_enabled) {
		if (!mib_chain_get(MIB_MBSSIB_TBL, WLAN_REPEATER_ITF_INDEX, (void *)&Entry)) {
  			printf("Error! Get MIB_MBSSIB_TBL for VXD SSID error.\n");
		}

		snprintf(para2, sizeof(para2), "%s-vxd", getWlanIfName());
		snprintf(parm, sizeof(parm), "ssid=%s", Entry.ssid);
		status|=va_cmd(IWPRIV, 3, 1, para2, "set_mib", parm);
		argv[1] = (char*)getWlanIfName();
	}
#endif

	// opmode
	if (wlan_mode == CLIENT_MODE)
		vInt = 8;	// client
	else	// 0(AP_MODE) or 3(AP_WDS_MODE)
		vInt = 16;	// AP

	status|=iwpriv_cmd(IWPRIV_INT, getWlanIfName(), "set_mib", "opmode", vInt);

#ifdef WLAN_UNIVERSAL_REPEATER
	// Repeater opmode
	if (rpt_enabled) {
		if (wlan_mode == CLIENT_MODE)
			vInt = 16;	// Repeater is AP
		else
			vInt = 8;	// Repeater is Client
		snprintf(para2, sizeof(para2), "%s-vxd", getWlanIfName());
		status|=iwpriv_cmd(IWPRIV_INT, para2, "set_mib", "opmode", vInt);
		argv[1] = (char*)getWlanIfName();
	}
#endif

	//12/16/04' hrchen, only need to set once
	// Added by Mason Yu for Set TxPower
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN0_5G_SUPPORT) || defined(WLAN1_5G_SUPPORT)
#if defined(CONFIG_RTL_92D_SUPPORT)
	status|=iwpriv_cmd(IWPRIV_GETMIB|IWPRIV_INT, getWlanIfName(), "set_mib", "macPhyMode", MIB_WLAN_MAC_PHY_MODE);
#endif
	status|=iwpriv_cmd(IWPRIV_GETMIB|IWPRIV_INT, getWlanIfName(), "set_mib", "phyBandSelect", MIB_WLAN_PHY_BAND_SELECT);
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
	mib_get(MIB_HW_11N_TRSWITCH, (void *)value);
	if(value[0] != 0) 
		status|=iwpriv_cmd(IWPRIV_INT, getWlanIfName(), "set_mib", "trswitch", value[0]);
	
#endif //CONFIG_RTL_92D_SUPPORT

	status |= setupWlanHWSetting(getWlanIfName());

#if defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_RTL_8812_SUPPORT ) || defined(WLAN_DUALBAND_CONCURRENT)
	status |= setup8812Wlan(getWlanIfName());
#endif

	// bcnint
	mib_get(MIB_WLAN_BEACON_INTERVAL, (void *)value);
	vInt = (int)(*(unsigned short *)value);
	snprintf(parm, sizeof(parm), "bcnint=%u", vInt);
	status|=va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", parm);

	// channel
	mib_get(MIB_WLAN_AUTO_CHAN_ENABLED, (void *)value);
	if(value[0])
		status|=va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", "channel=0");
	else
		status|=iwpriv_cmd(IWPRIV_GETMIB|IWPRIV_INT, getWlanIfName(), "set_mib", "channel", MIB_WLAN_CHAN_NUM);

#ifdef WIFI_TEST
	if(!mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
		printf("Error! Get MIB_MBSSIB_TBL for root SSID error.\n");
	value[0] = Entry.wlanBand;
	if (value[0] == 4){ // WiFi-G
		snprintf(parm, sizeof(parm), "basicrates=%u", 0x15f);
		status|=va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", parm);
	}
	else if (value[0] == 5){ // WiFi-BG
		snprintf(parm, sizeof(parm), "basicrates=%u", 0x00f);
		status|=va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", parm);
	}
	else {
#endif
	//jim do wifi test tricky,
	//    1 for wifi logo test,
	//    0 for normal case...
	mib_get(MIB_WIFI_SUPPORT, (void*)value);
	if(value[0]==1){
		if(!mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
			printf("Error! Get MIB_MBSSIB_TBL for root SSID error.\n");
		value[0] = Entry.wlanBand;
		if (value[0] == 2) {// WiFi-G
			snprintf(parm, sizeof(parm), "basicrates=%u", 0x15f);
			status|=va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", parm);
		}
		else if (value[0] == 3){// WiFi-BG
			snprintf(parm, sizeof(parm), "basicrates=%u", 0x00f);
			status|=va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", parm);
		}
		else {
			mib_get(MIB_WLAN_BASIC_RATE, (void *)value);
			vInt = (int)(*(unsigned short *)value);
			status|=iwpriv_cmd(IWPRIV_INT, getWlanIfName(), "set_mib", "basicrates", vInt);
		}
	}
	else{
		// basicrates
		mib_get(MIB_WLAN_BASIC_RATE, (void *)value);
		vInt = (int)(*(unsigned short *)value);
		status|=iwpriv_cmd(IWPRIV_INT, getWlanIfName(), "set_mib", "basicrates", vInt);
	}
#ifdef WIFI_TEST
	}
#endif

#ifdef WIFI_TEST
	if(!mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
		printf("Error! Get MIB_MBSSIB_TBL for root SSID error.\n");
	value[0] = Entry.wlanBand;
	if (value[0] == 4) {// WiFi-G
		snprintf(parm, sizeof(parm), "oprates=%u", 0xfff);
		status|=va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", parm);
	}
	else if (value[0] == 5) {// WiFi-BG
		snprintf(parm, sizeof(parm), "oprates=%u", 0xfff);
		status|=va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", parm);
	}
	else {
#endif
	//jim do wifi test tricky,
	//    1 for wifi logo test,
	//    0 for normal case...
	mib_get(MIB_WIFI_SUPPORT, (void*)value);
	if(value[0]==1){
		if(!mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
			printf("Error! Get MIB_MBSSIB_TBL for root SSID error.\n");
		value[0] = Entry.wlanBand;
		if (value[0] == 2) {// WiFi-G
			snprintf(parm, sizeof(parm), "oprates=%u", 0xfff);
			status|=va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", parm);
		}
		else if (value[0] == 3) {// WiFi-BG
			snprintf(parm, sizeof(parm), "oprates=%u", 0xfff);
			status|=va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", parm);
		}
		else {
			mib_get(MIB_WLAN_SUPPORTED_RATE, (void *)value);
			vInt = (int)(*(unsigned short *)value);
			status|=iwpriv_cmd(IWPRIV_INT, getWlanIfName(), "set_mib", "oprates", vInt);
		}
	}
	else{
		// oprates
		mib_get(MIB_WLAN_SUPPORTED_RATE, (void *)value);
		vInt = (int)(*(unsigned short *)value);
		status|=iwpriv_cmd(IWPRIV_INT, getWlanIfName(), "set_mib", "oprates", vInt);
	}
#ifdef WIFI_TEST
	}
#endif

	if(!mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
		printf("Error! Get MIB_MBSSIB_TBL for root SSID error.\n");
	// autorate
	autoRateRoot = Entry.rateAdaptiveEnabled;
	snprintf(parm, sizeof(parm), "autorate=%u", autoRateRoot);
	status|=va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", parm);

	// rtsthres
	mib_get(MIB_WLAN_RTS_THRESHOLD, (void *)value);
	vInt = (int)(*(unsigned short *)value);
	snprintf(parm, sizeof(parm), "rtsthres=%u", vInt);
	status|=va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", parm);
	set_vap_para("set_mib", parm);

	// fragthres
	mib_get(MIB_WLAN_FRAG_THRESHOLD, (void *)value);
	vInt = (int)(*(unsigned short *)value);
	snprintf(parm, sizeof(parm), "fragthres=%u", vInt);
	status|=va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", parm);
	set_vap_para("set_mib", parm);

	// expired_time
	mib_get(MIB_WLAN_INACTIVITY_TIME, (void *)value);
	vInt = (int)(*(unsigned long *)value);
	snprintf(parm, sizeof(parm), "expired_time=%u", vInt);
	status|=va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", parm);

	// preamble
	status|=iwpriv_cmd(IWPRIV_GETMIB|IWPRIV_INT, getWlanIfName(), "set_mib", "preamble", MIB_WLAN_PREAMBLE_TYPE);

	// hiddenAP
	snprintf(parm, sizeof(parm), "hiddenAP=%u", Entry.hidessid);
	status|=va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", parm);

	// dtimperiod
	status|=iwpriv_cmd(IWPRIV_GETMIB|IWPRIV_INT, getWlanIfName(), "set_mib", "dtimperiod", MIB_WLAN_DTIM_PERIOD);

        //txbf must disable if enable antenna diversity
        #if defined(CONFIG_SLOT_0_ANT_SWITCH) || defined(CONFIG_SLOT_1_ANT_SWITCH)
        status|=va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", "txbf=0");
        #endif

#ifdef WLAN_ACL
	status|=set_wlan_acl(getWlanIfName());
#ifdef WLAN_MBSSID
		if (wlan_mode == AP_MODE || wlan_mode == AP_WDS_MODE) {
			for (j=1; j<=WLAN_MBSSID_NUM; j++){
				snprintf(para2, sizeof(para2), "%s-vap%d", getWlanIfName(), j-1);
				status|=set_wlan_acl(para2);
			}
		}
#endif
#ifdef WLAN_UNIVERSAL_REPEATER
		if (wlan_mode != WDS_MODE) {
			if (rpt_enabled){
				snprintf(para2, sizeof(para2), "%s-vxd", getWlanIfName());
				status|=set_wlan_acl(para2);
			}
		}
#endif
#endif

	// authtype
	// Modified by Mason Yu
	// Root AP's authtype
	setupWLan_dot11_auth(0);

#ifdef WLAN_MBSSID
	// VAP
	for (j=1; j<=4; j++) {
		setupWLan_dot11_auth(j);
		if (!mib_chain_get(MIB_MBSSIB_TBL, j, (void *)&Entry)) {
  			printf("Error! Get MIB_MBSSIB_TBL for VAP SSID error.\n");
		}

		snprintf(para2, sizeof(para2), "%s-vap%d", getWlanIfName(), j-1);
		argv[1] = para2;

		//for wifi-vap band
		value[0] = Entry.wlanBand;
#ifdef WIFI_TEST
		if (value[0] == 4) // WiFi-G
			value[0] = 3; // 2.4 GHz (B+G)
		else if (value[0] == 5) // WiFi-BG
			value[0] = 3; // 2.4 GHz (B+G)
#endif
		unsigned char vChar;
		mib_get(MIB_WIFI_SUPPORT, (void*)&vChar);
		if(vChar==1) {
			if(value[0] == 2)
				value[0] = 3;
		}
		if (value[0] == 8) { //pure 11n
			mib_get(MIB_WLAN_PHY_BAND_SELECT, (void *)&phyband);
			if(phyband == PHYBAND_5G) {//5G
				value[0] += 4; // a
				vChar = 4;
			}
			else{
				value[0] += 3;	//b+g+n
				vChar = 3;
			}
		}
		else if (value[0] == 2) {	//pure 11g
			value[0] += 1;	//b+g
			vChar = 1;
		}
		else if (value[0] == 10) {	//g+n
			value[0] += 1;	//b+g+n
			vChar = 1;
		}
		else if(value[0] == 64) {	//pure 11ac
			value[0] += 12; 	//a+n
			vChar = 12;
		}
		else if(value[0] == 72) {	//n+ac
			value[0] += 4; 	//a
			vChar = 4;
		}		
		else vChar = 0;
		
		status |= iwpriv_cmd(IWPRIV_INT, para2, "set_mib", "band", value[0]); //802.11b:1, 802.11g:2, 802.11n:8

		// for wifi-vap autorate
		value[0] = Entry.rateAdaptiveEnabled;
		autoRate = (int)value[0];
		status |=iwpriv_cmd(IWPRIV_INT, para2, "set_mib", "autorate", autoRate);

		if (autoRate == 0) 
			// for wifi-vap fixrate
			status |=iwpriv_cmd(IWPRIV_INT, para2, "set_mib", "fixrate", Entry.fixedTxRate);

		//for wifi-vap hidden AP
		status|=iwpriv_cmd(IWPRIV_INT, para2, "set_mib", "hiddenAP", Entry.hidessid);

		// for wifi-vap WMM
		status|=iwpriv_cmd(IWPRIV_INT, para2, "set_mib", "qos_enable", Entry.wmmEnabled);

		//for wifi-vap max block_relay
		status|=iwpriv_cmd(IWPRIV_INT, para2, "set_mib", "block_relay", Entry.userisolation);

		//deny legacy
		status|=iwpriv_cmd(IWPRIV_INT, para2, "set_mib", "deny_legacy", vChar);
	        //txbf must disable if enable antenna diversity
        	#if defined(CONFIG_SLOT_0_ANT_SWITCH) || defined(CONFIG_SLOT_1_ANT_SWITCH)
		status|=iwpriv_cmd(IWPRIV_INT, para2, "set_mib", "txbf", 0);
        	#endif

	}
	argv[1] = (char*)getWlanIfName();
#endif
#ifdef WLAN_UNIVERSAL_REPEATER
	if (rpt_enabled) {
		setupWLan_dot11_auth(WLAN_REPEATER_ITF_INDEX);
	}
#endif // of WLAN_UNIVERSAL_REPEATER

#if defined WLAN_QoS && !defined CONFIG_RTL8192CD
	status|=setupWLanQos(argv);
#endif

#ifdef WLAN_WPA

	// Modified by Mason Yu
	// encmode
	// Root AP
	setupWLan_WPA(0); // Root
#ifdef WLAN_11R
	setupWLan_FT(0);
#endif

#ifdef WLAN_MBSSID
	// encmode
	for (j=1; j<=4; j++) {
		setupWLan_WPA(j);
#ifdef WLAN_11R
		setupWLan_FT(j);
#endif
	}
#endif
#ifdef WLAN_UNIVERSAL_REPEATER
	if (rpt_enabled)
		setupWLan_WPA(WLAN_REPEATER_ITF_INDEX);
#endif

	// Modified by Mason Yu
	// Set 802.1x flag
	setupWLan_802_1x(0); // Root

#ifdef WLAN_MBSSID
	// Set 802.1x flag
	for (j=1; j<=4; j++)
		setupWLan_802_1x(j); // VAP
#endif
#ifdef WLAN_UNIVERSAL_REPEATER
	if (rpt_enabled)
		setupWLan_802_1x(WLAN_REPEATER_ITF_INDEX); // Repeater
#endif

#endif // of WLAN_WPA

	if(!mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
		printf("Error! Get MIB_MBSSIB_TBL for root SSID error.\n");
	// band
	value [0] = Entry.wlanBand;
#ifdef WIFI_TEST
	if (value[0] == 4) // WiFi-G
		value[0] = 3; // 2.4 GHz (B+G)
	else if (value[0] == 5) // WiFi-BG
		value[0] = 3; // 2.4 GHz (B+G)
#endif
	//jim do wifi test tricky,
	//    1 for wifi logo test,
	//    0 for normal case...
	unsigned char vChar;
	mib_get(MIB_WIFI_SUPPORT, (void*)&vChar);
	if(vChar==1){
		if(value[0] == 2)
			value[0] = 3;
	}

	if(value[0] == 8) {	//pure 11n
		mib_get(MIB_WLAN_PHY_BAND_SELECT, (void *)&phyband);
		if(phyband == PHYBAND_5G) {//5G
			value[0] += 4; // a
			vChar = 4;
		}
		else{
			value[0] += 3;	//b+g+n
			vChar = 3;
		}
	}
	else if(value[0] == 2) {	//pure 11g
		value[0] += 1;	//b+g
		vChar = 1;
	}
	else if(value[0] == 10) {	//g+n
		value[0] += 1; 	//b+g+n
		vChar = 1;
	}
	else if(value[0] == 64) {	//pure 11ac
		value[0] += 12; 	//a+n
		vChar = 12;
	}
	else if(value[0] == 72) {	//n+ac
		value[0] += 4; 	//a
		vChar = 4;
	}
	else vChar = 0;

	status|=iwpriv_cmd(IWPRIV_INT, getWlanIfName(), "set_mib", "band", value[0]); //802.11b:1, 802.11g:2, 802.11n:8

	//deny legacy
	status|=iwpriv_cmd(IWPRIV_INT, getWlanIfName(), "set_mib", "deny_legacy", vChar);

	// For TKIP g mode issue (WiFi Cert 4.2.44: Disallow TKIP with HT Rates Test). Added by Annie, 2010-06-29.
	va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", "lgyEncRstrct=15");
	set_vap_para("set_mib","lgyEncRstrct=15");

	// For WiFi Test Plan. Added by Annie, 2010-06-29.
	va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", "wifi_specific=2");
	set_vap_para("set_mib","wifi_specific=2");

	if (autoRateRoot == 0)
	{
		// fixrate
		snprintf(parm, sizeof(parm), "fixrate=%u", Entry.fixedTxRate);
		status|=va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", parm);
	}
//cathy, for multicast rate
#ifdef CONFIG_USB_RTL8187SU_SOFTAP
	mib_get(MIB_WLAN_MLCSTRATE, (void *)value);
	vInt = (int)(*(unsigned short *)value);
	snprintf(parm, sizeof(parm), "lowestMlcstRate=%u", vInt);
	status|=va_cmd(IWPRIV, (char *)getWlanIfName(), "set_mib", parm);
	set_vap_para(argv[2], argv[3]);
#endif

#ifdef WLAN_WDS
	setupWDS();
#endif

//12/23/04' hrchen, these MIBs are for CLIENT mode, disable them
#if 0
	//12/16/04' hrchen, disable nat25_disable
	// nat25_disable
	value[0] = 0;
	sprintf(parm, "nat25_disable=%u", value[0]);
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);

	//12/16/04' hrchen, disable macclone_enable
	// macclone_enable
	value[0] = 0;
	sprintf(parm, "macclone_enable=%u", value[0]);
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
#endif

#if 0
	//12/16/04' hrchen, debug flag
	// debug_err
	sprintf(parm, "debug_err=ffffffff");
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
	// debug_info
	sprintf(parm, "debug_info=0");
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
	// debug_warn
	sprintf(parm, "debug_warn=0");
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
	// debug_trace
	sprintf(parm, "debug_trace=0");
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
#endif

	//12/16/04' hrchen, for protection mode
	// cts2self
	status|=va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", "cts2self=1");

	//12/16/04' hrchen, set 11g protection mode
	// disable_protection
	mib_get(MIB_WLAN_PROTECTION_DISABLED, (void *)value);
	snprintf(parm, sizeof(parm), "disable_protection=%u", value[0]);
	status|=va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", parm);
	set_vap_para("set_mib", parm);

	//12/16/04' hrchen, chipVersion
	// chipVersion
	/*
	sprintf(parm, "chipVersion=0");
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
	*/

#if 0	// not necessary for AP
	//12/16/04' hrchen, defssid
	// defssid
	sprintf(parm, "defssid=\"defaultSSID\"");
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
#endif

	//12/16/04' hrchen, set block relay
	// block_relay
	snprintf(parm, sizeof(parm), "block_relay=%u", Entry.userisolation);
	status|=va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", parm);
	setup_wlan_block();
	
#ifdef WIFI_TEST
	value[0] = Entry.wlanBand;
	if (value[0] == 4 || value[0] == 5) {// WiFi-G or WiFi-BG
		// block_relay=0
		status|=va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", "block_relay=0");
		// wifi_specific=1
		status|=va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", "wifi_specific=1");
	}
#endif
	//jim do wifi test tricky,
	//    1 for wifi logo test,
	//    0 for normal case...
	mib_get(MIB_WIFI_SUPPORT, (void*)value);
	if(value[0]==1){
		value[0] = Entry.wlanBand;
		if (value[0] == 2 || value[0] == 3) {// WiFi-G or WiFi-BG
			// block_relay=0
			status|=va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", "block_relay=0");
			// wifi_specific=1
			status|=va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", "wifi_specific=1");
		}
		else {// WiFi-11N
		    printf("In MIB_WLAN_BAND = 2 or 3\n");
			status|=va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", "block_relay=0");
			// wifi_specific=2
			status|=va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", "wifi_specific=2");
		}
	}

#ifdef WLAN_QoS
	value[0] = Entry.wmmEnabled;
	if(value[0]==0){
		va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", "qos_enable=0");
		//set_vap_para("set_mib","qos_enable=0");
	}
	else if(value[0]==1){
		va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", "qos_enable=1");
		//set_vap_para("set_mib","qos_enable=1");
	}

	// Kaohj -- not support in this moment
	#if 0
	//for wmm power saving
	mib_get(MIB_WLAN_APSD_ENABLE, (void *)value);
	if(value[0]==0)
		va_cmd(IWPRIV, 3, 1, (char *)WLANIF, "set_mib", "apsd_enable=0");
	else if(value[0]==1)
		va_cmd(IWPRIV, 3, 1, (char *)WLANIF, "set_mib", "apsd_enable=1");
	#endif
#endif

	//Channel Width
	mib_get(MIB_WLAN_CHANNEL_WIDTH, (void *)value);
	if(value[0]==0)	// 20MHZ
	{
		va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", "use40M=0");
		set_vap_para("set_mib", "use40M=0");
	}
	else if(value[0]==1)	// 40MHZ
	{
		va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", "use40M=1");
		set_vap_para("set_mib", "use40M=1");
	}
	else	// 80MHZ
	{
		va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", "use40M=2");
		set_vap_para("set_mib", "use40M=2");
	}
	//Conntrol Sideband
	if(value[0]==0) {	//20M
		va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", "2ndchoffset=0");
		set_vap_para("set_mib", "2ndchoffset=0");
	}
	else {	//40M
		mib_get(MIB_WLAN_CONTROL_BAND, (void *)value);
		if(value[0]==0){	//upper
			va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", "2ndchoffset=1");
			set_vap_para("set_mib", "2ndchoffset=1");
		}
		else{		//lower
			va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", "2ndchoffset=2");
			set_vap_para("set_mib", "2ndchoffset=2");
		}
#if defined(CONFIG_WLAN_HAL_8814AE) || defined (CONFIG_RTL_8812_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
		mib_get(MIB_WLAN_AUTO_CHAN_ENABLED, &vChar);
		mib_get(MIB_WLAN_CHAN_NUM, (void *)value);
		if(vChar == 0 && value[0] > 14)
		{
			printf("!!! adjust 5G 2ndoffset for 8812 !!!\n");
			if(value[0]==36 || value[0]==44 || value[0]==52 || value[0]==60){
				va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", "2ndchoffset=2");
				set_vap_para("set_mib", "2ndchoffset=2");
			}
			else{
				va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", "2ndchoffset=1");
				set_vap_para("set_mib", "2ndchoffset=1");
			}
		}
#endif
	}
	//11N Co-Existence
	mib_get(MIB_WLAN_11N_COEXIST, (void *)value);
	if(value[0]==0)
	{
		va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", "coexist=0");
		set_vap_para("set_mib", "coexist=0");
	}
	else
	{
		va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", "coexist=1");
		set_vap_para("set_mib", "coexist=1");

	}

	//short GI
	mib_get(MIB_WLAN_SHORTGI_ENABLED, (void *)value);
	if(value[0]==0) {
		va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", "shortGI20M=0");
		set_vap_para("set_mib", "shortGI20M=0");
		va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", "shortGI40M=0");
		set_vap_para("set_mib", "shortGI40M=0");
		va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", "shortGI80M=0");
		set_vap_para("set_mib", "shortGI80M=0");
	}
	else {
		va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", "shortGI20M=1");
		set_vap_para("set_mib", "shortGI20M=1");
		va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", "shortGI40M=1");
		set_vap_para("set_mib", "shortGI40M=1");
		va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", "shortGI80M=1");
		set_vap_para("set_mib", "shortGI80M=1");
	}

	//aggregation
	mib_get(MIB_WLAN_AGGREGATION, (void *)value);
	if(value[0]==0) {
		va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", "ampdu=0");
		set_vap_para("set_mib", "ampdu=0");
		va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", "amsdu=0");
	}
	else {
		va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", "ampdu=1");
		set_vap_para("set_mib", "ampdu=1");
		va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", "amsdu=0");
	}
	set_vap_para("set_mib", "amsdu=0");

	if (wlan_mode == AP_MODE || wlan_mode == AP_WDS_MODE) {
		if (intf_map!=1) {
			va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", "vap_enable=1");
		}
		else {
			va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", "vap_enable=0");
		}
	}

#ifdef CONFIG_RTL_WAPI_SUPPORT
	mib_get(MIB_WLAN_WAPI_UCAST_REKETTYPE, (void *)&vChar);
	snprintf(parm, sizeof(parm), "wapiUCastKeyType=%d", vChar);
	status|=va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", parm);
	if (vChar!=1) {
		mib_get(MIB_WLAN_WAPI_UCAST_TIME, (void *)&vInt);
		snprintf(parm, sizeof(parm), "wapiUCastKeyTimeout=%d", vInt);
		status|=va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", parm);
		mib_get(MIB_WLAN_WAPI_UCAST_PACKETS, (void *)&vInt);
		snprintf(parm, sizeof(parm), "wapiUCastKeyPktNum=%d", vInt);
		status|=va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", parm);
	}
	

	mib_get(MIB_WLAN_WAPI_MCAST_REKEYTYPE, (void *)&vChar);
	snprintf(parm, sizeof(parm), "wapiMCastKeyType=%d", vChar);
	status|=va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", parm);
	if (vChar!=1) {
		mib_get(MIB_WLAN_WAPI_MCAST_TIME, (void *)&vInt);
		snprintf(parm, sizeof(parm), "wapiMCastKeyTimeout=%d", vInt);
		status|=va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", parm);
		mib_get(MIB_WLAN_WAPI_MCAST_PACKETS, (void *)&vInt);
		snprintf(parm, sizeof(parm), "wapiMCastKeyPktNum=%d", vInt);
		status|=va_cmd(IWPRIV, 3, 1, (char *)getWlanIfName(), "set_mib", parm);
	}
#endif
	return status;
}
#endif
// Added by Mason Yu
int stopwlan()
{
	int status = 0;
	int wirelessauthpid=0,iwcontrolpid=0, wscdpid=0, upnppid=0, run_mini_upnpd = 0;
#ifdef WLAN_11R
	int ftpid=0;
#endif
	int AuthPid;
	int i,j, flags;
	unsigned char no_wlan;
	char s_auth_pid[32], s_auth_conf[32], s_auth_fifo[32];
	char s_ifname[16];
	char *argv[7];
#ifdef	WLAN_MBSSID
	MIB_CE_MBSSIB_T Entry;
#endif
#ifdef CONFIG_WIFI_SIMPLE_CONFIG
	char wscd_pid_name[32];
	char wscd_fifo_name[32];
#endif

	// Kill iwcontrol
	iwcontrolpid = read_pid((char*)IWCONTROLPID);
	if(iwcontrolpid > 0){
		kill(iwcontrolpid, 9);
		unlink(IWCONTROLPID);
	}

	// Kill Auth
	for(j=0;j<NUM_WLAN_INTERFACE;j++){
        	for ( i=0; i<=NUM_VWLAN_INTERFACE; i++) {
        		if (i==0) {
                             snprintf(s_ifname, sizeof(s_ifname), WLANIF[j]);
        			snprintf(s_auth_pid, 32, "/var/run/auth-%s.pid", (char *)s_ifname);
        			snprintf(s_auth_conf, 32, "/var/config/%s.conf", (char *)s_ifname);
        			snprintf(s_auth_fifo, 32, "/var/auth-%s.fifo", (char *)s_ifname);
        		}
        		#ifdef	WLAN_MBSSID
        		if (i >= WLAN_VAP_ITF_INDEX && i < WLAN_VAP_ITF_INDEX+WLAN_MBSSID_NUM) {
                            snprintf(s_ifname, sizeof(s_ifname), "%s-vap", WLANIF[j]);
        			snprintf(s_auth_pid, 32, "/var/run/auth-%s%d.pid", (char *)s_ifname, i-WLAN_VAP_ITF_INDEX);
        			snprintf(s_auth_conf, 32, "/var/config/%s%d.conf", (char *)s_ifname, i-WLAN_VAP_ITF_INDEX);
        			snprintf(s_auth_fifo, 32, "/var/auth-%s%d.fifo", (char *)s_ifname, i-WLAN_VAP_ITF_INDEX);
        		}
        		#endif
        		#ifdef WLAN_UNIVERSAL_REPEATER
        		if (i == WLAN_REPEATER_ITF_INDEX) {
                            snprintf(s_ifname, sizeof(s_ifname), "%s-vxd", WLANIF[j]);
        			snprintf(s_auth_pid, 32, "/var/run/auth-%s.pid", (char *)s_ifname);
        			snprintf(s_auth_conf, 32, "/var/config/%s.conf", (char *)s_ifname);
        			snprintf(s_auth_fifo, 32, "/var/auth-%s.fifo", (char *)s_ifname);
        		}
        		#endif
        		AuthPid = read_pid(s_auth_pid);
        		if(AuthPid > 0) {
        			kill(AuthPid, 9);
        			unlink(s_auth_conf);
        			unlink(s_auth_pid);
        			unlink(s_auth_fifo);
        		}
        	}
	}

#ifdef WLAN_SUPPORT
#ifdef WLAN_11R
	ftpid = read_pid(FT_PID);
	if(ftpid > 0) {
		kill(ftpid, 9);
		unlink(FT_PID);
		unlink(FT_CONF);
	}
#endif
#ifdef CONFIG_WIFI_SIMPLE_CONFIG // WPS
	// Kill wscd-wlan0.pid
	getWscPidName(wscd_pid_name);
	wscdpid = read_pid(wscd_pid_name);
	if(wscdpid > 0){
		system("/bin/echo 0 > /proc/gpio");
		kill(wscdpid, 9);
		unlink(wscd_pid_name);
		unlink(wscd_fifo_name);
		unlink(WSCD_CONF);
	}
	startSSDP();

#endif
#endif


#ifdef CONFIG_RTL_WAPI_SUPPORT
		system("killall aeUdpClient");
#endif // WAPI


        for(j=0;j<NUM_WLAN_INTERFACE;j++){
        	for ( i=0; i<=NUM_VWLAN_INTERFACE; i++) {
        		if (i==0){
                            snprintf(s_ifname, sizeof(s_ifname), "%s", WLANIF[j]);
        		}
        		#ifdef	WLAN_MBSSID
        		if (i >= WLAN_VAP_ITF_INDEX && i < WLAN_VAP_ITF_INDEX+WLAN_MBSSID_NUM)
                            snprintf(s_ifname, sizeof(s_ifname), "%s-vap%d", WLANIF[j], i-WLAN_VAP_ITF_INDEX);
        		#endif
        		#ifdef WLAN_UNIVERSAL_REPEATER
        		if (i == WLAN_REPEATER_ITF_INDEX)
                            snprintf(s_ifname, sizeof(s_ifname), "%s-vxd", WLANIF[j]);
        		#endif
        		if (getInFlags( s_ifname, &flags) == 1){
        			if (flags & IFF_UP){
        				status |= va_cmd(IFCONFIG, 2, 1, s_ifname, "down");
				#ifdef CONFIG_USER_FON //not under br0
						if(j == 0 && i == WLAN_MBSSID_NUM) continue;
				#endif
				#if defined(CONFIG_MASTER_WLAN0_ENABLE) && defined(CONFIG_SLAVE_WLAN1_ENABLE)
						if( j == 0 || !strcmp(s_ifname, WLANIF[1]))
				#elif defined(CONFIG_SLAVE_WLAN1_ENABLE)
						if(!strcmp(s_ifname, WLANIF[0]))
				#endif
        				status|=va_cmd(BRCTL, 3, 1, "delif", (char *)BRIF, s_ifname);
        			}
        		}
        	}
        }
#ifdef WLAN_WISP
	int dhcp_pid;
	unsigned char value[32];
	char itf_name[IFNAMSIZ];
	getWispWanName(itf_name);
	//snprintf(value, 32, "%s.%s", (char*)DHCPC_PID, "wlan0");
	snprintf(value, 32, "%s.%s", (char*)DHCPC_PID, itf_name);
	dhcp_pid = read_pid((char*)value);
	if(dhcp_pid > 0){
		kill(dhcp_pid, SIGUSR1); //dhcp new
	}
#endif
}

#define ConfigWlanLock "/var/run/configWlanLock"
#define LOCK_WLAN()	\
do {	\
	if ((lockfd = open(ConfigWlanLock, O_RDWR)) == -1) {	\
		perror("open wlan lockfile");	\
		return 0;	\
	}	\
	while (flock(lockfd, LOCK_EX)) { \
		if (errno != EINTR) \
			break; \
	}	\
} while (0)

#define UNLOCK_WLAN()	\
do {	\
	flock(lockfd, LOCK_UN);	\
	close(lockfd);	\
} while (0)

#ifdef CONFIG_USER_FON
#define FONCOOVACHILLI "/var/run/chilli.pid"

void enableFonSpot()
{
	int pid = 0;
	char str[20], str2[100], str3[100];
	unsigned char buffer[32];
	unsigned char devAddr[6];
	//char *argv[18];
	pid = read_pid((char *)FONCOOVACHILLI);
	if(pid > 0){
		printf( "%s: already start.\n", __FUNCTION__ );
		return;
	}
	snprintf(str, sizeof(str), "wlan0-vap%d", WLAN_MBSSID_NUM - 1);
	mib_get(MIB_ADSL_LAN_IP, (void *) buffer);
	snprintf(str2, sizeof(str2), "--dns1=%s --dns2=%s", inet_ntoa(*((struct in_addr *)buffer)), inet_ntoa(*((struct in_addr *)buffer)));
	mib_get(MIB_ELAN_MAC_ADDR, (void *)devAddr);
	snprintf(str3, sizeof(str3), "--radiusnasid=%02x-%02x-%02x-%02x-%02x-%02x",
		devAddr[0], devAddr[1], devAddr[2],
#ifdef CONFIG_USER_IPV6READYLOGO_ROUTER
		//Set vc and nas mac with br0's mac plus 1
		devAddr[3], devAddr[4], devAddr[5]+1);
#else
		devAddr[3], devAddr[4], devAddr[5]);
#endif
	va_cmd_no_echo("/bin/chilli", 17, 0, "-c", "/var/chilli.conf", "--statip=192.168.182.0/24", str3,
		"--dhcpif", str, "--papalwaysok", "--localusers=/etc/fon/localusers", "--wwwbin=/bin/true", 
		"--ipup=/bin/true", "--ipdown=/bin/true", "--conup=/bin/true", "--condown=/bin/true",
		"--pidfile", "/var/run/chilli.pid", str2, "--ipwhitelist=/tmp/whitelist.cache"); 
/*	va_cmd("/bin/chilli", 19, 0, "-c", "/var/chilli.conf", "--statip=192.168.182.0/24", str3,
		"--dhcpif", str, "--papalwaysok", "--localusers=/etc/fon/localusers", "--wwwbin=/bin/true", 
		"--ipup=/bin/true", "--ipdown=/bin/true", "--conup=/bin/true", "--condown=/bin/true",
		"--pidfile", "/var/run/chilli.pid", str2, "--ipwhitelist=/tmp/whitelist.cache", "--debug", "--debugfacility=31");*/
	printf("fon spot service start.\n");
}

void disableFonSpot()
{
	int pid = 0;
	pid = read_pid((char *)FONCOOVACHILLI);
	if(pid <= 0){
		printf( "%s: already stop.\n", __FUNCTION__ );
	}
	else{
		kill(pid, 9);
		unlink(FONCOOVACHILLI);
	}
}
void startFonSpot()
{
	MIB_CE_MBSSIB_T Entry;
	unsigned char fon_onoffline;
	mib_get(MIB_FON_ONOFF, (void *)&fon_onoffline);
	if (!mib_chain_get(MIB_MBSSIB_TBL, WLAN_MBSSID_NUM, (void *)&Entry)) {
		printf("mib get failed!\n");
		return;
	}
	if( !fon_onoffline || (fon_onoffline && Entry.wlanDisabled) )
		disableFonSpot();
	else if( fon_onoffline && !Entry.wlanDisabled)
		enableFonSpot();
	
}
#endif // of CONFIG_USER_FON

int config_WLAN( int action_type )
{
	int lockfd, orig_wlan_idx;

	LOCK_WLAN();
	orig_wlan_idx = wlan_idx;
	switch( action_type )
	{
	case ACT_START:
		startWLan();
#ifdef CONFIG_USER_FON
		startFonSpot();
#endif
		break;

	case ACT_RESTART:
		stopwlan();
		startWLan();
#ifdef CONFIG_USER_FON
		startFonSpot();
#endif
		break;

	case ACT_STOP:
		stopwlan();
#ifdef CONFIG_USER_FON
		disableFonSpot();
#endif
		break;

	default:
		wlan_idx = orig_wlan_idx;
		return -1;
	}
	wlan_idx = orig_wlan_idx;
	UNLOCK_WLAN();
	return 0;
}

// Wlan configuration
#ifdef WLAN_1x
static void WRITE_WPA_FILE(int fh, unsigned char *buf)
{
	if ( write(fh, buf, strlen(buf)) != strlen(buf) ) {
		printf("Write WPA config file error!\n");
		close(fh);
		//exit(1);
	}
}

// return value:
// 0  : success
// -1 : failed
static int generateWpaConf(char *outputFile, int isWds, MIB_CE_MBSSIB_T *Entry)
{
	int fh, intVal;
	unsigned char chVal, wep, encrypt, enable1x;
	unsigned char buf1[1024], buf2[1024];
	unsigned short sintVal;

	fh = open(outputFile, O_RDWR|O_CREAT|O_TRUNC);
	if (fh == -1) {
		printf("Create WPA config file error!\n");
		return -1;
	}
	if (!isWds) {

	encrypt = Entry->encrypt;
	snprintf(buf2, sizeof(buf2), "encryption = %d\n", encrypt);
	WRITE_WPA_FILE(fh, buf2);

	strcpy(buf1, Entry->ssid);
	snprintf(buf2, sizeof(buf2), "ssid = \"%s\"\n", buf1);
	WRITE_WPA_FILE(fh, buf2);

	enable1x = Entry->enable1X;
	snprintf(buf2, sizeof(buf2), "enable1x = %d\n", enable1x);
	WRITE_WPA_FILE(fh, buf2);

	//mib_get( MIB_WLAN_ENABLE_MAC_AUTH, (void *)&intVal);
	snprintf(buf2, sizeof(buf2), "enableMacAuth = %d\n", 0);
	WRITE_WPA_FILE(fh, buf2);

/*
	mib_get( MIB_WLAN_ENABLE_SUPP_NONWPA, (void *)&intVal);
	if (intVal)
		mib_get( MIB_WLAN_SUPP_NONWPA, (void *)&intVal);
*/

	snprintf(buf2, sizeof(buf2), "supportNonWpaClient = %d\n", 0);
	WRITE_WPA_FILE(fh, buf2);

	wep = Entry->wep;
	snprintf(buf2, sizeof(buf2), "wepKey = %d\n", wep);
	WRITE_WPA_FILE(fh, buf2);

/*
	if ( encrypt==1 && enable1x ) {
		if (wep == 1) {
			mib_get( MIB_WLAN_WEP64_KEY1, (void *)buf1);
			sprintf(buf2, "wepGroupKey = \"%02x%02x%02x%02x%02x\"\n", buf1[0],buf1[1],buf1[2],buf1[3],buf1[4]);
		}
		else {
			mib_get( MIB_WLAN_WEP128_KEY1, (void *)buf1);
			sprintf(buf2, "wepGroupKey = \"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\"\n",
				buf1[0],buf1[1],buf1[2],buf1[3],buf1[4],
				buf1[5],buf1[6],buf1[7],buf1[8],buf1[9],
				buf1[10],buf1[11],buf1[12]);
		}
	}
	else
*/
	strcpy(buf2, "wepGroupKey = \"\"\n");
	WRITE_WPA_FILE(fh, buf2);


#ifdef WLAN_11R
	if (Entry->wpaAuth == 1 && Entry->ft_enable)
		chVal = 3;
	else
#endif
	chVal = Entry->wpaAuth;
	snprintf(buf2, sizeof(buf2), "authentication = %d\n", chVal); //1: radius 2:PSK 3:FT
	WRITE_WPA_FILE(fh, buf2);

	chVal = Entry->unicastCipher;
	snprintf(buf2, sizeof(buf2), "unicastCipher = %d\n", chVal);
	WRITE_WPA_FILE(fh, buf2);

	chVal = Entry->wpa2UnicastCipher;
	snprintf(buf2, sizeof(buf2), "wpa2UnicastCipher = %d\n", chVal);
	WRITE_WPA_FILE(fh, buf2);

/*
	mib_get( MIB_WLAN_WPA2_PRE_AUTH, (void *)&intVal);
	sprintf(buf2, "enablePreAuth = %d\n", intVal);
	WRITE_WPA_FILE(fh, buf2);
*/

	chVal = Entry->wpaPSKFormat;
	if (chVal==0)
		snprintf(buf2, sizeof(buf2), "usePassphrase = 1\n");
	else
		snprintf(buf2, sizeof(buf2), "usePassphrase = 0\n");
	WRITE_WPA_FILE(fh, buf2);


	strcpy(buf1, Entry->wpaPSK);
	snprintf(buf2, sizeof(buf2), "psk = \"%s\"\n", buf1);
	WRITE_WPA_FILE(fh, buf2);

	mib_get( MIB_WLAN_WPA_GROUP_REKEY_TIME, (void *)&intVal);
	snprintf(buf2, sizeof(buf2), "groupRekeyTime = %d\n", intVal);
	WRITE_WPA_FILE(fh, buf2);

#if 1 // not support RADIUS

	sintVal = Entry->rsPort;
	snprintf(buf2, sizeof(buf2), "rsPort = %d\n", sintVal);
	WRITE_WPA_FILE(fh, buf2);

	*((unsigned long *)buf1) = *((unsigned long *)Entry->rsIpAddr);
	snprintf(buf2, sizeof(buf2), "rsIP = %s\n", inet_ntoa(*((struct in_addr *)buf1)));
	WRITE_WPA_FILE(fh, buf2);

	strcpy(buf1, Entry->rsPassword);
	snprintf(buf2, sizeof(buf2), "rsPassword = \"%s\"\n", buf1);
	WRITE_WPA_FILE(fh, buf2);

	mib_get( MIB_WLAN_RS_RETRY, (void *)&chVal);
	snprintf(buf2, sizeof(buf2), "rsMaxReq = %d\n", chVal);
	WRITE_WPA_FILE(fh, buf2);

	mib_get( MIB_WLAN_RS_INTERVAL_TIME, (void *)&sintVal);
	snprintf(buf2, sizeof(buf2), "rsAWhile = %d\n", sintVal);
	WRITE_WPA_FILE(fh, buf2);

	mib_get( MIB_WLAN_ACCOUNT_RS_ENABLED, (void *)&chVal);
	snprintf(buf2, sizeof(buf2), "accountRsEnabled = %d\n", chVal);
	WRITE_WPA_FILE(fh, buf2);

	mib_get( MIB_WLAN_ACCOUNT_RS_PORT, (void *)&sintVal);
	snprintf(buf2, sizeof(buf2), "accountRsPort = %d\n", sintVal);
	WRITE_WPA_FILE(fh, buf2);

	mib_get( MIB_WLAN_ACCOUNT_RS_IP, (void *)buf1);
	snprintf(buf2, sizeof(buf2), "accountRsIP = %s\n", inet_ntoa(*((struct in_addr *)buf1)));
	WRITE_WPA_FILE(fh, buf2);

	mib_get( MIB_WLAN_ACCOUNT_RS_PASSWORD, (void *)buf1);
	snprintf(buf2, sizeof(buf2), "accountRsPassword = \"%s\"\n", buf1);
	WRITE_WPA_FILE(fh, buf2);

	mib_get( MIB_WLAN_ACCOUNT_UPDATE_ENABLED, (void *)&chVal);
	snprintf(buf2, sizeof(buf2), "accountRsUpdateEnabled = %d\n", chVal);
	WRITE_WPA_FILE(fh, buf2);

	mib_get( MIB_WLAN_ACCOUNT_UPDATE_DELAY, (void *)&sintVal);
	snprintf(buf2, sizeof(buf2), "accountRsUpdateTime = %d\n", sintVal);
	WRITE_WPA_FILE(fh, buf2);

	mib_get( MIB_WLAN_ACCOUNT_RS_RETRY, (void *)&chVal);
	snprintf(buf2, sizeof(buf2), "accountRsMaxReq = %d\n", chVal);
	WRITE_WPA_FILE(fh, buf2);

	mib_get( MIB_WLAN_ACCOUNT_RS_INTERVAL_TIME, (void *)&sintVal);
	snprintf(buf2, sizeof(buf2), "accountRsAWhile = %d\n", sintVal);
	WRITE_WPA_FILE(fh, buf2);
#endif

#ifdef WLAN_11W  
	if (encrypt == WIFI_SEC_WPA2)
		snprintf(buf2, sizeof(buf2), "ieee80211w = %d\n", Entry->dotIEEE80211W);
	else
		snprintf(buf2, sizeof(buf2), "ieee80211w = %d\n", 0);
	WRITE_WPA_FILE(fh, buf2);

	snprintf(buf2, sizeof(buf2), "sha256 = %d\n", Entry->sha256);
	WRITE_WPA_FILE(fh, buf2);
#endif
	}

	else {
#if 0 // not support WDS
#endif
	}

	close(fh);

	return 0;
}

static int is8021xEnabled(int vwlan_idx) {
#ifdef WLAN_1x
	MIB_CE_MBSSIB_T Entry;

	mib_chain_get(MIB_MBSSIB_TBL, vwlan_idx, (void *)&Entry);

	if (Entry.enable1X) {
		return 1;
	} else {
		if (Entry.encrypt >= WIFI_SEC_WPA) {
			///fprintf(stderr, "wpaAuth=%d\n", wpaAuth);
			if (WPA_AUTH_AUTO == Entry.wpaAuth)
				return 1;

		}
	}
#endif
	return 0;
}

#ifdef CONFIG_WIFI_SIMPLE_CONFIG // WPS
#define MIB_GET(id, val) do { \
		if (0==mib_get(id, (void *)val)) { \
		} \
	} while (0)

void sync_wps_config_mib()
{
	MIB_CE_MBSSIB_T Entry;
	int i, orig_idx;

	orig_idx = wlan_idx;
	for(i=0; i<NUM_WLAN_INTERFACE; i++){
		wlan_idx = i;
		mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry);
		mib_get(MIB_WSC_CONFIGURED, &Entry.wsc_configured);
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, 0);
	}
	wlan_idx = orig_idx;
}

void update_wps_from_mibtable()
{
	MIB_CE_MBSSIB_T Entry;
	int i, orig_idx;

	mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry);

	mib_get(MIB_WLAN_SSID, Entry.ssid);
	mib_get(MIB_WLAN_ENCRYPT, &Entry.encrypt);
	mib_get(MIB_WLAN_AUTH_TYPE, &Entry.authType);
	mib_get(MIB_WSC_AUTH, &Entry.wsc_auth);
	mib_get(MIB_WLAN_WPA_AUTH, &Entry.wpaAuth);
	mib_get(MIB_WLAN_WPA_PSK_FORMAT, &Entry.wpaPSKFormat);
	mib_get(MIB_WLAN_WPA_PSK, Entry.wpaPSK);
	mib_get(MIB_WSC_PSK, Entry.wscPsk);
	mib_get(MIB_WLAN_WPA_CIPHER_SUITE, &Entry.unicastCipher);
	mib_get(MIB_WLAN_WPA2_CIPHER_SUITE, &Entry.wpa2UnicastCipher);
	mib_get(MIB_WLAN_WEP, &Entry.wep);
	mib_get(MIB_WLAN_WEP_DEFAULT_KEY, &Entry.wepDefaultKey);
	mib_get(MIB_WLAN_WEP64_KEY1, Entry.wep64Key1);
	mib_get(MIB_WLAN_WEP64_KEY2, Entry.wep64Key2);
	mib_get(MIB_WLAN_WEP64_KEY3, Entry.wep64Key3);
	mib_get(MIB_WLAN_WEP64_KEY4, Entry.wep64Key4);
	mib_get(MIB_WLAN_WEP128_KEY1, Entry.wep128Key1);
	mib_get(MIB_WLAN_WEP128_KEY2, Entry.wep128Key2);
	mib_get(MIB_WLAN_WEP128_KEY3, Entry.wep128Key3);
	mib_get(MIB_WLAN_WEP128_KEY4, Entry.wep128Key4);
	mib_get(MIB_WLAN_WEP_KEY_TYPE, &Entry.wepKeyType);		// wep Key Format
	mib_get(MIB_WSC_ENC, &Entry.wsc_enc);

	mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, 0);
	sync_wps_config_mib();
	mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);

}

void update_wps_mib()
{
	MIB_CE_MBSSIB_T Entry;
	mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry);
	
	mib_set(MIB_WLAN_ENCRYPT, &Entry.encrypt);
	mib_set(MIB_WLAN_ENABLE_1X, &Entry.enable1X);
	mib_set(MIB_WLAN_WEP, &Entry.wep);
	mib_set(MIB_WLAN_WPA_AUTH, &Entry.wpaAuth);
	mib_set(MIB_WLAN_WPA_PSK_FORMAT, &Entry.wpaPSKFormat);
	mib_set(MIB_WLAN_WPA_PSK, Entry.wpaPSK);
	mib_set(MIB_WLAN_RS_PORT, &Entry.rsPort);
	mib_set(MIB_WLAN_RS_IP, Entry.rsIpAddr);

	mib_set(MIB_WLAN_RS_PASSWORD, Entry.rsPassword);
	mib_set(MIB_WLAN_DISABLED, &Entry.wlanDisabled);
	mib_set(MIB_WLAN_SSID, Entry.ssid);
	mib_set(MIB_WLAN_MODE, &Entry.wlanMode);
	mib_set(MIB_WLAN_AUTH_TYPE, &Entry.authType);

	mib_set(MIB_WLAN_WPA_CIPHER_SUITE, &Entry.unicastCipher);
	mib_set(MIB_WLAN_WPA2_CIPHER_SUITE, &Entry.wpa2UnicastCipher);
	mib_set(MIB_WLAN_WPA_GROUP_REKEY_TIME, &Entry.wpaGroupRekeyTime);

	mib_set(MIB_WLAN_WEP_KEY_TYPE, &Entry.wepKeyType);      // wep Key Format
	mib_set(MIB_WLAN_WEP_DEFAULT_KEY, &Entry.wepDefaultKey);
	mib_set(MIB_WLAN_WEP64_KEY1, Entry.wep64Key1);
	mib_set(MIB_WLAN_WEP64_KEY2, Entry.wep64Key2);
	mib_set(MIB_WLAN_WEP64_KEY3, Entry.wep64Key3);
	mib_set(MIB_WLAN_WEP64_KEY4, Entry.wep64Key4);
	mib_set(MIB_WLAN_WEP128_KEY1, Entry.wep128Key1);
	mib_set(MIB_WLAN_WEP128_KEY2, Entry.wep128Key2);
	mib_set(MIB_WLAN_WEP128_KEY3, Entry.wep128Key3);
	mib_set(MIB_WLAN_WEP128_KEY4, Entry.wep128Key4);
	mib_set(MIB_WLAN_BAND, &Entry.wlanBand);
	mib_set(MIB_WSC_DISABLE, &Entry.wsc_disabled);
	//mib_set(MIB_WSC_CONFIGURED, &Entry.wsc_configured);
	mib_set(MIB_WSC_UPNP_ENABLED, &Entry.wsc_upnp_enabled);
	mib_set(MIB_WSC_AUTH, &Entry.wsc_auth);
	mib_set(MIB_WSC_ENC, &Entry.wsc_enc);
	mib_set(MIB_WSC_PSK, Entry.wscPsk);

#ifdef WLAN_11W
	mib_set(MIB_WLAN_DOTIEEE80211W, &Entry.dotIEEE80211W);
	mib_set(MIB_WLAN_SHA256, &Entry.sha256);
#endif

}
void update_wps_configured(int reset_flag)
{
	char is_configured, encrypt1, encrypt2, auth, disabled, iVal, mode, format, encryptwps;
	char ssid1[100], ssid2[100];
	unsigned char tmpbuf[100];
#ifdef WPS20
	unsigned char wpsUseVersion;
#endif
	MIB_CE_MBSSIB_T Entry;
	if(!mib_chain_get(MIB_MBSSIB_TBL, 0, &Entry))
		return;
	
	update_wps_mib();

	//fprintf(stderr, "update_wps_configured(%d)\n", reset_flag);

	MIB_GET(MIB_WSC_CONFIGURED, (void *)&is_configured);
	MIB_GET(MIB_WLAN_MODE, (void *)&mode);

	if (!is_configured && mode == AP_MODE) {
		MIB_GET(MIB_WLAN_SSID, (void *)ssid1);
		mib_getDef(MIB_WLAN_SSID, (void *)ssid2);
		if (strcmp(ssid1, ssid2)) { // Magician: Fixed parsing error by Source Insight
			is_configured = 1;
			mib_set(MIB_WSC_CONFIGURED, (void *)&is_configured);
			Entry.wsc_configured = is_configured;

			MIB_GET(MIB_WSC_CONFIG_BY_EXT_REG, (void *)&iVal);
			if (is_configured && iVal == 0) {
				iVal = 1;
				mib_set(MIB_WSC_MANUAL_ENABLED, (void *)&iVal);
			}
			//return;
		}

		MIB_GET(MIB_WLAN_ENCRYPT, (void *)&encrypt1);
		mib_getDef(MIB_WLAN_ENCRYPT, (void *)&encrypt2);

		if (encrypt1 != encrypt2) {
			is_configured = 1;
			mib_set(MIB_WSC_CONFIGURED, (void *)&is_configured);
			Entry.wsc_configured = is_configured;
		}
	}
	mib_chain_update(MIB_MBSSIB_TBL, &Entry, 0);
	
	MIB_GET(MIB_WSC_DISABLE, (void *)&disabled);
#ifdef WPS20
	MIB_GET(MIB_WSC_VERSION, (void *)&wpsUseVersion);
	if (wpsUseVersion == 0 && disabled)
		return;
#else
	if (disabled)
		return;
#endif

	MIB_GET(MIB_WLAN_ENCRYPT, (void *)&encrypt1);
	if (encrypt1 == WIFI_SEC_NONE) {
		auth = WSC_AUTH_OPEN;
		encrypt2 = WSC_ENCRYPT_NONE;
	}
	else if (encrypt1 == WIFI_SEC_WEP) {
		auth = WSC_AUTH_OPEN;
		encrypt2 = WSC_ENCRYPT_WEP;
	}
	else if (encrypt1 == WIFI_SEC_WPA) {
		MIB_GET(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&encryptwps);
		auth = WSC_AUTH_WPAPSK;
		encrypt2 = WSC_ENCRYPT_TKIPAES;
		if (encryptwps == WPA_CIPHER_AES)
			encrypt2 = WSC_ENCRYPT_AES;
		if (encryptwps == WPA_CIPHER_TKIP)
			encrypt2 = WSC_ENCRYPT_TKIP;
		if (encryptwps == WPA_CIPHER_MIXED)
			encrypt2 = WSC_ENCRYPT_TKIPAES;
	}
	else if (encrypt1 == WIFI_SEC_WPA2) {
		MIB_GET(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&encryptwps);
		auth = WSC_AUTH_WPA2PSK;
		encrypt2 = WSC_ENCRYPT_TKIPAES;
		if (encryptwps == WPA_CIPHER_AES)
			encrypt2 = WSC_ENCRYPT_AES;
		if (encryptwps == WPA_CIPHER_TKIP)
			encrypt2 = WSC_ENCRYPT_TKIP;
		if (encryptwps == WPA_CIPHER_MIXED)
			encrypt2 = WSC_ENCRYPT_TKIPAES;
	}
	else {
		auth = WSC_AUTH_WPA2PSKMIXED;
		encrypt2 = WSC_ENCRYPT_TKIPAES;
// sync with ap team at 2011-04-25, When mixed mode, if no WPA2-AES, try to use WPA-AES or WPA2-TKIP
        	MIB_GET(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&encrypt1);
        	MIB_GET(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&iVal);
		if (encrypt1 == iVal) {	//cathy, fix wps web bug when encryption is: WPA-AES+WPA2-AES / WPA-TKIP+WPA2-AES
			if (encrypt1 == WPA_CIPHER_TKIP)
				encrypt2 = WSC_ENCRYPT_TKIP;
			else if (encrypt1 == WPA_CIPHER_AES)
				encrypt2 = WSC_ENCRYPT_AES;
		}
		else if (!(iVal & WPA_CIPHER_AES)) {
			if (encrypt1 & WPA_CIPHER_AES) {
				encrypt2 = WSC_ENCRYPT_AES;
			}
		}
//-------------------------------------------- david+2008-01-03
	}
	mib_set(MIB_WSC_AUTH, (void *)&auth);
	Entry.wsc_auth = auth;
	mib_set(MIB_WSC_ENC, (void *)&encrypt2);
	Entry.wsc_enc = encrypt2;

	MIB_GET(MIB_WLAN_ENCRYPT, (void *)&encrypt1);
	if (encrypt1 == WIFI_SEC_WPA || encrypt1 == WIFI_SEC_WPA2
		|| encrypt1 == WIFI_SEC_WPA2_MIXED) {
		MIB_GET(MIB_WLAN_WPA_AUTH, (void *)&format);
		if (format & 2) { // PSK
			MIB_GET(MIB_WLAN_WPA_PSK, (void *)tmpbuf);
			mib_set(MIB_WSC_PSK, (void *)tmpbuf);
			strcpy(Entry.wscPsk, tmpbuf);
		}
	}
	if (reset_flag) {
		reset_flag = 0;
		mib_set(MIB_WSC_CONFIG_BY_EXT_REG, (void *)&reset_flag);
	}

	MIB_GET(MIB_WSC_CONFIG_BY_EXT_REG, (void *)&iVal);
	if (is_configured && iVal == 0) {
		iVal = 1;
		mib_set(MIB_WSC_MANUAL_ENABLED, (void *)&iVal);
	}
	iVal = 0;
	if (mode == AP_MODE || mode == AP_WDS_MODE) {
		if (encrypt1 == WIFI_SEC_WEP || encrypt1 == WIFI_SEC_NONE) {
			MIB_GET(MIB_WLAN_ENABLE_1X, (void *)&encrypt2);
			if (encrypt2)
				iVal = 1;
		}
		else {
			MIB_GET(MIB_WLAN_WPA_AUTH, (void *)&encrypt2);
			if (encrypt2 == WPA_AUTH_AUTO)
				iVal = 1;
		}
	}
	else if (mode == CLIENT_MODE || mode == AP_WDS_MODE)
		iVal = 1;
	if (iVal) {
		iVal = 0;
		mib_set(MIB_WSC_MANUAL_ENABLED, (void *)&iVal);
		mib_set(MIB_WSC_CONFIGURED, (void *)&iVal);
		Entry.wsc_configured = iVal;
		mib_set(MIB_WSC_CONFIG_BY_EXT_REG, (void *)&iVal);
	}
	mib_chain_update(MIB_MBSSIB_TBL, &Entry, 0);
	sync_wps_config_mib();
}

int start_WPS()
{
	int status=0;
	unsigned char encrypt;
	int retry;
	unsigned char wsc_disable;
	unsigned char wlan_mode;
	unsigned char wlan_nettype;
	unsigned char wpa_auth;
	char *cmd_opt[16];
	int cmd_cnt = 0; int idx;
	int wscd_pid_fd = -1;
	int i;
	unsigned int enableWscIf = 0;
	int orig_wlan_idx;
	unsigned char no_wlan;
	char fifo_buf[32], fifo_buf2[32];
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
	char wlanBand2G5GSelect;
	orig_wlan_idx = wlan_idx;
#endif

#if 0 //def CONFIG_RTL_92D_DMDP
	unsigned char wlan0_mode, wlan1_mode, both_band_ap;
	unsigned char wlan_disabled_root, wlan_wsc_disabled;
	unsigned char wlan_wsc1_disabled;
	unsigned char wlan1_disabled_root;
	mib_get(MIB_WLAN_MODE, (void *)&wlan0_mode);
	mib_get(MIB_WLAN_DISABLED, (void *)&wlan_disabled_root);
	mib_get(MIB_WSC_DISABLE, (void *)&wlan_wsc_disabled);
	mib_get(MIB_WLAN1_MODE, (void *)&wlan1_mode);
	mib_get(MIB_WLAN1_DISABLED, (void *)&wlan1_disabled_root);
	mib_get(MIB_WLAN1_WSC_DISABLE, (void *)&wlan_wsc1_disabled);
	printf("wlan0_mode=%d wlan_disabled_root=%d wlan_wsc_disabled=%d\n", wlan0_mode, wlan_disabled_root, wlan_wsc_disabled);
	printf("wlan1_mode=%d wlan1_disabled_root=%d wlan_wsc1_disabled=%d\n", wlan1_mode, wlan1_disabled_root, wlan_wsc1_disabled);
	if (((wlan0_mode == AP_MODE) || (wlan0_mode == AP_WDS_MODE)) && ((wlan1_mode == 0) || (wlan1_mode == AP_WDS_MODE))
		&& (wlan_disabled_root == 0) && (wlan1_disabled_root == 0) && (wlan_wsc_disabled == 0) && (wlan_wsc1_disabled == 0))
		both_band_ap = 1;
#endif

	for(i = 0; i<NUM_WLAN_INTERFACE; i++){
		wlan_idx = i;

		mib_get(MIB_WSC_DISABLE, (void *)&wsc_disable);
		mib_get(MIB_WLAN_DISABLED, (void *)&no_wlan);
		mib_get(MIB_WLAN_MODE, (void *)&wlan_mode);
		mib_get(MIB_WLAN_NETWORK_TYPE, (void *)&wlan_nettype);
		mib_get(MIB_WLAN_WPA_AUTH, (void *)&wpa_auth);
		mib_get(MIB_WLAN_ENCRYPT, (void *)&encrypt);

		if(no_wlan || wsc_disable || is8021xEnabled(0))
			continue;
		else if(wlan_mode == CLIENT_MODE) {
			if(wlan_nettype != INFRASTRUCTURE)
				continue;
		}
		else if(wlan_mode == AP_MODE) {
			if((encrypt >= WIFI_SEC_WPA) && (wpa_auth == WPA_AUTH_AUTO))
				continue;
		}
		if(useWlanIfVirtIdx())
			enableWscIf |= 1;
		else
			enableWscIf |= (1<<i);
	}

	if(!enableWscIf)
		goto WSC_DISABLE;
	else
		useAuth_RootIf |= enableWscIf;

	fprintf(stderr, "START WPS SETUP!\n\n\n");
	cmd_opt[cmd_cnt++] = "";

	if (wlan_mode == CLIENT_MODE) {
		cmd_opt[cmd_cnt++] = "-mode";
		cmd_opt[cmd_cnt++] = "2";
	} else {
		cmd_opt[cmd_cnt++] = "-start";
	}

#if defined(CONFIG_RTL_92D_DMDP) || defined(WLAN_DUALBAND_CONCURRENT)
//	if (both_band_ap == 1)
	if(enableWscIf == 3){
		cmd_opt[cmd_cnt++] = "-both_band_ap";
		cmd_opt[cmd_cnt++] = "-w";
		cmd_opt[cmd_cnt++] = (char *)WLANIF[0];
	}
	else{
		if(enableWscIf & 1){
			cmd_opt[cmd_cnt++] = "-w";
			cmd_opt[cmd_cnt++] = (char *)WLANIF[0];
		}
		else{
			cmd_opt[cmd_cnt++] = "-w";
			cmd_opt[cmd_cnt++] = (char *)WLANIF[1];
		}
	}
#endif

	cmd_opt[cmd_cnt++] = "-c";
	cmd_opt[cmd_cnt++] = (char *)WSCD_CONF;
#if !defined(CONFIG_RTL_92D_DMDP) && !defined(WLAN_DUALBAND_CONCURRENT)
	cmd_opt[cmd_cnt++] = "-w";
	cmd_opt[cmd_cnt++] = (char *)WLANIF[0];
#endif
	//strcat(cmd, " -c /var/wscd.conf -w wlan0");

	if (enableWscIf & 1) { // use iwcontrol
		cmd_opt[cmd_cnt++] = "-fi";
		snprintf(fifo_buf, 32, (char *)WSCD_FIFO, WLANIF[0]);
		cmd_opt[cmd_cnt++] = (char *)fifo_buf;
		//strcat(cmd, " -fi /var/wscd-wlan0.fifo");
	}
#if defined(CONFIG_RTL_92D_DMDP) || defined(WLAN_DUALBAND_CONCURRENT)
	if (enableWscIf & 2){
		cmd_opt[cmd_cnt++] = "-fi2";
		snprintf(fifo_buf2, 32, (char *)WSCD_FIFO, WLANIF[1]);
		cmd_opt[cmd_cnt++] = (char *)fifo_buf2;
	}
#endif

	//cmd_opt[cmd_cnt++] = "-debug";
	//strcat(cmd, " -debug");
	//strcat(cmd, " &");
	#define TARGDIR "/var/wps/"
	#define SIMPLECFG "simplecfgservice.xml"
	//status |= va_cmd("/bin/flash", 3, 1, "upd-wsc-conf", "/etc/wscd.conf", "/var/wscd.conf");
	status |= va_cmd("/bin/mkdir", 2, 1, "-p", TARGDIR);
	status |= va_cmd("/bin/cp", 2, 1, "/etc/" SIMPLECFG, TARGDIR);

	cmd_opt[cmd_cnt] = 0;
	printf("CMD ARGS: ");
	for (idx=0; idx<cmd_cnt;idx++)
		printf("%s ", cmd_opt[idx]);
	printf("\n");

	status |= do_cmd("/bin/wscd", cmd_opt, 0);

	if (enableWscIf & 1) {
		retry = 0;
		snprintf(fifo_buf, sizeof(fifo_buf), WSCD_FIFO, WLANIF[0]);
		while ((wscd_pid_fd = open((char *)fifo_buf, O_WRONLY|O_NONBLOCK)) == -1)
		{
			usleep(100000);
			retry ++;

			if (retry > 10) {
				printf("wscd fifo timeout, abort\n");
				break;
			}
		}

		if(wscd_pid_fd!=-1) close(wscd_pid_fd);/*jiunming, close the opened fd*/
	}
	
#if defined(CONFIG_RTL_92D_DMDP) || defined(WLAN_DUALBAND_CONCURRENT)
	if (enableWscIf & 2) {
		retry = 0;
		snprintf(fifo_buf, sizeof(fifo_buf), WSCD_FIFO, WLANIF[1]);
		while ((wscd_pid_fd = open((char *)fifo_buf, O_WRONLY|O_NONBLOCK)) == -1)
		{
			usleep(100000);
			retry ++;

			if (retry > 10) {
				printf("wscd fifo timeout, abort\n");
				break;
			}
		}

		if(wscd_pid_fd!=-1) close(wscd_pid_fd);
	}
#endif
	
WSC_DISABLE:
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
	wlan_idx = orig_wlan_idx;
#endif
	return status;
}
#endif

/*
 *	vwlan_idx:
 *	0:	Root
 *	1 ~ 4:	VAP
 *	5:	Repeater
 */
static int wlanItfUpAndStartAuth(int vwlan_idx)
{
	int auth_pid_fd=-1;
	int status=0;
	char ifname[16];
	char para_auth_conf[30];
	char para_auth_fifo[30];
	MIB_CE_MBSSIB_T Entry;
#ifdef WLAN_UNIVERSAL_REPEATER
	char rpt_enabled;
#endif
#ifdef WLAN_WISP
	unsigned int wan_mode;
	char wlanmode;
	int wisp_wan_id;
#endif
	mib_chain_get(MIB_MBSSIB_TBL, vwlan_idx, (void *)&Entry);

	if (vwlan_idx==0) {
		strncpy(ifname, (char*)getWlanIfName(), 16);
	}
	else {
		#ifdef WLAN_MBSSID
		if (vwlan_idx>=WLAN_VAP_ITF_INDEX && vwlan_idx<(WLAN_VAP_ITF_INDEX+WLAN_MBSSID_NUM))
			snprintf(ifname, 16, "%s-vap%d", (char *)getWlanIfName(), vwlan_idx-1);
		#endif
		#ifdef WLAN_UNIVERSAL_REPEATER
		if (vwlan_idx == WLAN_REPEATER_ITF_INDEX) {
			snprintf(ifname, 16, "%s-vxd", (char *)getWlanIfName());
			mib_get( MIB_REPEATER_ENABLED1, (void *)&rpt_enabled);
			if (rpt_enabled)
				Entry.wlanDisabled=0;
			else
				Entry.wlanDisabled=1;
		}
		#endif
	}

	if (Entry.wlanDisabled == 0)	// WLAN enabled
	{
#ifdef WLAN_WISP
	mib_get( MIB_WAN_MODE, (void *)&wan_mode);
	wlanmode = Entry.wlanMode;
#ifdef WLAN_UNIVERSAL_REPEATER
	mib_get( MIB_REPEATER_ENABLED1, (void *)&rpt_enabled);
#endif
	getWispWanID(&wisp_wan_id);
	if(!(wan_mode & MODE_Wlan) || wisp_wan_id!=wlan_idx
#ifdef WLAN_UNIVERSAL_REPEATER
		|| (vwlan_idx != WLAN_REPEATER_ITF_INDEX && (wlanmode==AP_MODE || wlanmode==AP_WDS_MODE) && rpt_enabled) //vxd is WISP interface, add root & vap to bridge
		//|| (vwlan_idx == WLAN_REPEATER_ITF_INDEX && wlanmode==CLIENT_MODE) //root is WISP interface, add vxd to bridge
#endif
	){
#endif
		// brctl addif br0 wlan0
#if defined(CONFIG_MASTER_WLAN0_ENABLE) && defined(CONFIG_SLAVE_WLAN1_ENABLE)
		if( wlan_idx==0 || !strcmp(ifname, WLANIF[1]))
			status|=va_cmd(BRCTL, 3, 1, "addif", (char *)BRIF, ifname);
#elif defined(CONFIG_SLAVE_WLAN1_ENABLE)
		if( !strcmp(ifname, WLANIF[0]))
			status|=va_cmd(BRCTL, 3, 1, "addif", (char *)BRIF, ifname);
#else
#ifdef CONFIG_USER_FON
		if(vwlan_idx != WLAN_MBSSID_NUM) //last wlan vap is used for fon service
#endif
		status|=va_cmd(BRCTL, 3, 1, "addif", (char *)BRIF, ifname);
#ifdef WLAN_WISP
		if(vwlan_idx == WLAN_REPEATER_ITF_INDEX)
			setWlanDevFlag(ifname, 0);
#endif
#endif
#ifdef WLAN_WISP
	}else{
		if(vwlan_idx == WLAN_REPEATER_ITF_INDEX)
			setWlanDevFlag(ifname, 1);
	}
#endif
#ifdef CONFIG_IPV6
		// Disable ipv6 in bridge
		setup_disable_ipv6(ifname, 1);
#endif
		// ifconfig wlan0 up
		status|=va_cmd(IFCONFIG, 2, 1, ifname, "up");

#ifdef WLAN_1x
		snprintf(para_auth_conf, sizeof(para_auth_conf), "/var/config/%s.conf", ifname);
		snprintf(para_auth_fifo, sizeof(para_auth_fifo), "/var/auth-%s.fifo", ifname);
		if (is8021xEnabled(vwlan_idx)) // 802.1x enabled, auth is only used when 802.1x is enable since encryption is driver based in 11n driver
		{ // Magician: Fixed parsing error by Source Insight
			status|=generateWpaConf(para_auth_conf, 0, &Entry);
			status|=va_cmd(AUTH, 4, 0, ifname, (char *)LANIF, "auth", para_auth_conf);
			// fix the depency problem
			// check fifo
			while ((auth_pid_fd = open(para_auth_fifo, O_WRONLY)) == -1)
			{
				usleep(30000);
			}
			if(auth_pid_fd!=-1) close(auth_pid_fd);/*jiunming, close the opened fd*/
			if (vwlan_idx == 0){ // Root
			        /* 2010-10-27 krammer :  use bit map to record what wlan root interface is use for auth*/
			if(useWlanIfVirtIdx())
				useAuth_RootIf |= 1;
			else
				useAuth_RootIf |= (1<<wlan_idx);//bit 0 ==> wlan0, bit 1 ==>wlan1
			}
			else {
				strcpy(para_iwctrl[wlan_num], ifname);
				wlan_num++;
			}
		}
#endif
		status = (status==-1)?-1:1;
	}
	else
		return 0;

}

#ifdef WLAN_WEB_REDIRECT  //jiunming,web_redirect
int start_wlan_web_redirect(){

	int status=0;
	char tmpbuf[MAX_URL_LEN];
	char ipaddr[16], ip_port[32], redir_server[33];

	ipaddr[0]='\0'; ip_port[0]='\0';redir_server[0]='\0';
	if (mib_get(MIB_ADSL_LAN_IP, (void *)tmpbuf) != 0)
	{
		strncpy(ipaddr, inet_ntoa(*((struct in_addr *)tmpbuf)), 16);
		ipaddr[15] = '\0';
		snprintf(ip_port, sizeof(ip_port), "%s:%d",ipaddr,8080);
	}//else ??

	if( mib_get(MIB_WLAN_WEB_REDIR_URL, (void*)tmpbuf) )
	{
		char *p=NULL, *end=NULL;

		p = strstr( tmpbuf, "http://" );
		if(p)
			p = p + 7;
		else
			p = tmpbuf;

		end = strstr( p, "/" );
		if(end)
			*end = '\0';

		snprintf( redir_server,32,"%s",p );
		redir_server[32]='\0';
	}//else ??

	// Enable Bridge Netfiltering
	//status|=va_cmd("/bin/brctl", 2, 0, "brnf", "on");

	//iptables -t nat -N Web_Redirect
	status|=va_cmd(IPTABLES, 4, 1, "-t", "nat","-N","Web_Redirect");

	//iptables -t nat -A Web_Redirect -d 192.168.1.1 -j RETURN
	status|=va_cmd(IPTABLES, 8, 1, "-t", "nat","-A","Web_Redirect",
		"-d", ipaddr, "-j", (char *)FW_RETURN);

	//iptables -t nat -A Web_Redirect -d 192.168.2.11 -j RETURN
	status|=va_cmd(IPTABLES, 8, 1, "-t", "nat","-A","Web_Redirect",
		"-d", redir_server, "-j", (char *)FW_RETURN);

	//iptables -t nat -A Web_Redirect -p tcp --dport 80 -j DNAT --to-destination 192.168.1.1:8080
	status|=va_cmd(IPTABLES, 12, 1, "-t", "nat","-A","Web_Redirect",
		"-p", "tcp", "--dport", "80", "-j", "DNAT",
		"--to-destination", ip_port);

	//iptables -t nat -A PREROUTING -p tcp --dport 80 -j Web_Redirect
	status|=va_cmd(IPTABLES, 12, 1, "-t", "nat","-A","PREROUTING",
		"-i", (char *)getWlanIfName(),
		"-p", "tcp", "--dport", "80", "-j", "Web_Redirect");

	return status;
}
#endif

#ifdef CONFIG_RTL_WAPI_SUPPORT

void wapi_cert_link_one(const char *name, const char *lnname) {
	char cmd[128];

	strcpy(cmd, "mkdir -p /var/myca/");
	system(cmd);


	strcpy(cmd, "ln -s ");
	strcat(cmd, name);
	strcat(cmd," ");
	strcat(cmd, lnname);
	system(cmd);
}

int start_aeUdpClient(void)
{
	int status=0;
	unsigned char tmpbuf[128], encrypt, wapiAuth;
	char ipaddr[16];
	extern void wapi_cert_link(void);

	mib_get(MIB_WLAN_ENCRYPT, (void *)&encrypt);
	mib_get(MIB_WLAN_WAPI_AUTH, (void *)&wapiAuth);
	if (encrypt != WIFI_SEC_WAPI || wapiAuth != 1) {
		goto OUT;
	}

	wapi_cert_link_one(WAPI_CA4AP_CERT_SAVE, WAPI_CA4AP_CERT);
	wapi_cert_link_one(WAPI_AP_CERT_SAVE, WAPI_AP_CERT);

	if (mib_get(MIB_WLAN_WAPI_ASIPADDR, (void *)tmpbuf) == 0)
	{
		status = -1;
		goto OUT;
	}

	strncpy(ipaddr, inet_ntoa(*((struct in_addr *)tmpbuf)), 16);

	strncpy(tmpbuf, "aeUdpClient ", sizeof(tmpbuf));
	strncat(tmpbuf, "-d ", sizeof(tmpbuf));
	strncat(tmpbuf, ipaddr, sizeof(tmpbuf));
	strncat(tmpbuf, " -i wlan0 &", sizeof(tmpbuf));
	fprintf(stderr, "%s(%d): %s\n", __FUNCTION__,__LINE__, tmpbuf);
	system(tmpbuf);


OUT:
	return status;
}
#endif


int start_iwcontrol()
{
	int tmp, found = 0;
	int status = 0;
	char *argv[12];

	// When (1) WPS enabled or (2) Root AP's encryption is WPA/WPA2 without MBSSID,
	// we should start iwcontrol with wlan0 interface.
	int i = 0;
    char buf[8] = {0};
	for(i = 0; useAuth_RootIf; i++){
        	if ( useAuth_RootIf & 0x00000001) {
                      //snprintf(buf, sizeof(buf), "wlan%d", i);
                      snprintf(buf, sizeof(buf), WLANIF[i]);
        		for (tmp=0; tmp < wlan_num; tmp++) {
        			if (strcmp(para_iwctrl[tmp], buf)==0) {
        				found = 1;
        				break;
        			}
        		}
        		if (!found) {
        			strcpy(para_iwctrl[wlan_num], buf);
        			wlan_num++;
        		}
        	}
               useAuth_RootIf >>= 1;
	}
	printf("Total WPA/WPA2 number is %d\n", wlan_num);
	if(wlan_num>0){
		//printf("CMD ARGS: ");
		for(i=0; i<wlan_num; i++){
			argv[i+1] = para_iwctrl[i];
		//	printf("%s", argv[i+1]);
		}
		argv[i+1]=NULL;
		//printf("\n");
		status|=do_cmd(IWCONTROL, argv, 1);
	}

}

// return value:
// 0  : success
// -1 : failed
int startWLanOneTimeDaemon()
{
	int status=0;
#ifdef WLAN_11R
	status |= start_FT();
#endif
#ifdef CONFIG_WIFI_SIMPLE_CONFIG
	status |= start_WPS();
#endif
#ifdef WLAN_WEB_REDIRECT
	status |= start_wlan_web_redirect();
#endif
#ifdef CONFIG_RTL_WAPI_SUPPORT
	status |= start_aeUdpClient();
#endif

	return status;
}
#endif


//--------------------------------------------------------
// Wireless LAN startup
// return value:
// 0  : not start by configuration
// 1  : successful
// -1 : failed
int startWLan()
{
	unsigned char no_wlan, wsc_disable, wlan_mode;
	int status=0, upnppid=0;
	char *argv[9];
#if defined(CONFIG_LUNA_DUAL_LINUX)
	int ping_ret = 0, ping_cnt = 0;
#endif

#if 0
	// Check wireless interface
	if (!getInFlags((char *)WLANIF, 0)) {
		printf("Wireless Interface Not Found !\n");
		return -1;	// interface not found
	}

	mib_get(MIB_WLAN_DISABLED, (void *)&no_wlan);
	if (no_wlan)
		return 0;

	//1/17/06' hrchen, always start WLAN MIB, for MP test will use
	// "ifconfig wlan0 up" to start WLAN
	// config WLAN
	status|=setupWLan();
#endif
	int i = 0, active_wlan = 0, j = 0, orig_wlan_idx;
	char ifname[16];
	MIB_CE_MBSSIB_T Entry;
	// Modified by Mason Yu
	wlan_num = 0; /*reset to 0,jiunming*/
	useAuth_RootIf = 0;  /*reset to 0 */
	orig_wlan_idx = wlan_idx;

	//process each wlan interface
	for(i = 0; i<NUM_WLAN_INTERFACE; i++){
		wlan_idx = i;
		if (!getInFlags((char *)getWlanIfName(), 0)) {
			printf("Wireless Interface Not Found !\n");
			status = -1;	// interface not found
			continue;
	    }
		mib_chain_get(MIB_MBSSIB_TBL, 0, &Entry);
		no_wlan = Entry.wlanDisabled;
		if (no_wlan){
			continue;
		}

		#if defined(CONFIG_LUNA_DUAL_LINUX)
		if(wlan_idx == 1)
		{
			ping_ret = system("ping -w 2 10.253.253.2 > /dev/null");
			while(ping_ret != -1 && ping_ret != 0 && ping_cnt++ < 3)
			{
				ping_ret = system("ping -w 2 10.253.253.2 > /dev/null");
				sleep(2);
			}
		}

		if(ping_ret != -1 && ping_ret != 0 && wlan_idx == 1)
		{
			printf("Slave can't connect \n");
			continue;
		}
		#endif

		//interface and root is enable, now we start
		active_wlan++;
#ifdef WLAN_FAST_INIT
		setupWLan(getWlanIfName(), 0);
#else
		status|=setupWLan();
#endif		

		//ifconfig wlan up ,add into bridge and start [auth<-depend on interface, so run many times]
		status |= wlanItfUpAndStartAuth(0);
#if !defined(CONFIG_RTL_92D_SUPPORT) && !defined(CONFIG_LUNA_DUAL_LINUX) && !defined(WLAN_DUALBAND_CONCURRENT)
		{
			#define RTL8185_IOCTL_GET_MIB   0x89f2
			int skfd;
			struct iwreq wrq;
			char value[16];

			//CurrentChannel
			mib_get(MIB_WLAN_AUTO_CHAN_ENABLED, (void *)value);
			if( value[0] ){ //root ap is auto channel
				do {		//wait for selecting channel
					sleep(1);
					skfd = socket(AF_INET, SOCK_DGRAM, 0);
					strcpy(wrq.ifr_name, getWlanIfName());
					strcpy(value,"opmode");
					wrq.u.data.pointer = (caddr_t)&value;
					wrq.u.data.length = 10;
					ioctl(skfd, RTL8185_IOCTL_GET_MIB, &wrq);
					close( skfd );
				}while(value[0] == 0x04);	//WIFI_WAIT_FOR_CHANNEL_SELECT
			}
		}
#endif //CONFIG_RTL_92D_SUPPORT
		wlan_mode = Entry.wlanMode;
#ifdef WLAN_MBSSID
		if (wlan_mode == AP_MODE || wlan_mode == AP_WDS_MODE) {
			for (j=1; j<=WLAN_MBSSID_NUM; j++){
#ifdef WLAN_FAST_INIT
				snprintf(ifname, sizeof(ifname), "%s-vap%d", getWlanIfName(), j-1);
				setupWLan(ifname, j);
#endif
			    status |= wlanItfUpAndStartAuth(j);
			}
		}
#endif
#ifdef WLAN_UNIVERSAL_REPEATER
		if (wlan_mode != WDS_MODE) {
			char rpt_enabled;
			mib_get(MIB_REPEATER_ENABLED1, (void *)&rpt_enabled);
			if (rpt_enabled){
#ifdef WLAN_FAST_INIT
				snprintf(ifname, sizeof(ifname), "%s-vxd", getWlanIfName());
				setupWLan(ifname, WLAN_REPEATER_ITF_INDEX);
#endif
			    status |= wlanItfUpAndStartAuth(WLAN_REPEATER_ITF_INDEX);
			}
		}
#endif

	}
	wlan_idx = orig_wlan_idx;
	if(!active_wlan){
		return status;
	}

	//start wlan daemon at last due to these daemons only need one
	status|=startWLanOneTimeDaemon();
	status|=start_iwcontrol();
#ifdef WLAN_SUPPORT
#ifdef CONFIG_WIFI_SIMPLE_CONFIG
	orig_wlan_idx = wlan_idx;
	char wscd_pid_name[32];
	for(i = 0; i<NUM_WLAN_INTERFACE; i++){
		wlan_idx = i;
		mib_chain_get(MIB_MBSSIB_TBL, 0, &Entry);
		no_wlan = Entry.wlanDisabled;
		wsc_disable = Entry.wsc_disabled;
		//upnppid = read_pid((char*)MINI_UPNPDPID);	//cathy
		if( !no_wlan && !wsc_disable && !is8021xEnabled(0)) {
			int retry = 10;
			getWscPidName(wscd_pid_name);
			while(--retry && (read_pid(wscd_pid_name) < 0))
			{
				//printf("WSCD is not running. Please wait!\n");
				usleep(300000);
			}
			startSSDP();
			break;
		}
	}
	wlan_idx = orig_wlan_idx;
#endif
#endif


	// enable samba service to mount slave's /proc/wlan0 with master /proc/wlan1
#ifdef CONFIG_SLAVE_WLAN1_ENABLE
	// check wlan1 enable or disable ?
#ifdef WLAN_DUALBAND_CONCURRENT
	orig_wlan_idx = wlan_idx ;
    wlan_idx = 1;
#endif
	mib_chain_get(MIB_MBSSIB_TBL, 0, &Entry);
	no_wlan = Entry.wlanDisabled;
	if(no_wlan != 1) // enable status , need mount proc/wlan1
	{
		//check mount already ?
		FILE *mount_fp;
		int need_mount = 0;
		char mount_buf[256];

		mount_fp = fopen("/proc/mounts", "r");
		if(mount_fp)
		{
			while (fgets(mount_buf, sizeof(mount_buf), mount_fp) != NULL)
			{
				if(strstr(mount_buf,  "10.253.253.2") > 0)
				{
					need_mount++;
				}
			}
		}
		// get slave ip
		if(need_mount < 5)
		{
			va_cmd("/bin/mount", 6 , 1 , "-t" ,"cifs", "//10.253.253.2/wlan" , "/proc/wlan1" , "-o", "username=admin");
			va_cmd("/bin/mount", 6 , 1 , "-t" ,"cifs", "//10.253.253.2/wlan-vap0" , "/proc/wlan1-vap0" , "-o", "username=admin");
			va_cmd("/bin/mount", 6 , 1 , "-t" ,"cifs", "//10.253.253.2/wlan-vap1" , "/proc/wlan1-vap1" , "-o", "username=admin");
			va_cmd("/bin/mount", 6 , 1 , "-t" ,"cifs", "//10.253.253.2/wlan-vap2" , "/proc/wlan1-vap2" , "-o", "username=admin");
			va_cmd("/bin/mount", 6 , 1 , "-t" ,"cifs", "//10.253.253.2/wlan-vap3" , "/proc/wlan1-vap3" , "-o", "username=admin");
			#ifdef CONFIG_RTL_REPEATER_MODE_SUPPORT
			va_cmd("/bin/mount", 6 , 1 , "-t" ,"cifs", "//10.253.253.2/wlan-vxd" , "/proc/wlan1-vxd" , "-o", "username=admin");
			#endif		
			va_cmd("/bin/mount", 6 , 1 , "-t" ,"cifs", "//10.253.253.2/slave-tmp" , "/tmp/slave" , "-o", "username=admin");
		}
	}
#ifdef WLAN_DUALBAND_CONCURRENT
	wlan_idx = orig_wlan_idx;
#endif

#endif


	return status;
}

int getMiscData(char *interface, struct _misc_data_ *pData)
{

    int skfd;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);

    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0){
      /* If no wireless name : no wireless extensions */
	  	close(skfd);
        return -1;
    }

    wrq.u.data.pointer = (caddr_t)pData;
    wrq.u.data.length = sizeof(struct _misc_data_);

    if (iw_get_ext(skfd, interface, SIOCGMISCDATA, &wrq) < 0){
		close(skfd);
		return -1;
    }
    close(skfd);

    return 0;
}

int getWlStaNum( char *interface, int *num )
{
    int skfd;
    unsigned short staNum;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);

    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0) {
      close( skfd );
      /* If no wireless name : no wireless extensions */
      return -1;
    }

    wrq.u.data.pointer = (caddr_t)&staNum;
    wrq.u.data.length = sizeof(staNum);

    if (iw_get_ext(skfd, interface, SIOCGIWRTLSTANUM, &wrq) < 0) {
      close( skfd );
      return -1;
    }

    *num  = (int)staNum;

    close( skfd );

    return 0;
}

#ifdef WLAN_CLIENT
/////////////////////////////////////////////////////////////////////////////
int getWlSiteSurveyResult(char *interface, SS_STATUS_Tp pStatus )
{
    int skfd;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;

    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0) {
       close( skfd );
      /* If no wireless name : no wireless extensions */
       return -1;
    }

    wrq.u.data.pointer = (caddr_t)pStatus;

    if ( pStatus->number == 0 )
    	wrq.u.data.length = sizeof(SS_STATUS_T);
    else
        wrq.u.data.length = sizeof(pStatus->number);

    if (iw_get_ext(skfd, interface, SIOCGIWRTLGETBSSDB, &wrq) < 0) {
      close( skfd );
      return -1;
    }

    close( skfd );

    return 0;
}

/////////////////////////////////////////////////////////////////////////////
int getWlJoinRequest(char *interface, pBssDscr pBss, unsigned char *res)
{
    int skfd;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;
    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0) {
      close( skfd );
      /* If no wireless name : no wireless extensions */
      return -1;
    }

    wrq.u.data.pointer = (caddr_t)pBss;
    wrq.u.data.length = sizeof(BssDscr);

    if (iw_get_ext(skfd, interface, SIOCGIWRTLJOINREQ, &wrq) < 0) {
      close( skfd );
      return -1;
    }

    close( skfd );

    *res = wrq.u.data.pointer[0];

    return 0;
}

/////////////////////////////////////////////////////////////////////////////
int getWlJoinResult(char *interface, unsigned char *res)
{
    int skfd;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;

    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0) {
      close( skfd );
      /* If no wireless name : no wireless extensions */
      return -1;
    }

    wrq.u.data.pointer = (caddr_t)res;
    wrq.u.data.length = 1;

    if (iw_get_ext(skfd, interface, SIOCGIWRTLJOINREQSTATUS, &wrq) < 0) {
      close( skfd );
      return -1;
    }

    close( skfd );

    return 0;
}



/////////////////////////////////////////////////////////////////////////////
int getWlSiteSurveyRequest(char *interface, int *pStatus)
{
    int skfd;
    struct iwreq wrq;
    unsigned char result;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;

    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0) {
      close( skfd );
      /* If no wireless name : no wireless extensions */
      return -1;
    }

    wrq.u.data.pointer = (caddr_t)&result;
    wrq.u.data.length = sizeof(result);

    if (iw_get_ext(skfd, interface, SIOCGIWRTLSCANREQ, &wrq) < 0) {
      close( skfd );
      return -1;
    }

    close( skfd );

    if ( result == 0xff )
    	*pStatus = -1;
    else
	*pStatus = (int) result;

    return 0;
}
#endif	// of WLAN_CLIENT

/////////////////////////////////////////////////////////////////////////////
int getWlBssInfo(char *interface, bss_info *pInfo)
{
    int skfd;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;

    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0) {
      close( skfd );
      /* If no wireless name : no wireless extensions */
      return -1;
    }

    wrq.u.data.pointer = (caddr_t)pInfo;
    wrq.u.data.length = sizeof(bss_info);

    if (iw_get_ext(skfd, interface, SIOCGIWRTLGETBSSINFO, &wrq) < 0) {
      close( skfd );
      return -1;
    }

    close( skfd );

    return 0;
}

#ifdef WLAN_WDS
/////////////////////////////////////////////////////////////////////////////
int getWdsInfo(char *interface, char *pInfo)
{

    int skfd;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);

    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0) {
      close( skfd );
      /* If no wireless name : no wireless extensions */
      return -1;
    }

    wrq.u.data.pointer = (caddr_t)pInfo;
    wrq.u.data.length = MAX_WDS_NUM*sizeof(WDS_INFO_T);

    if (iw_get_ext(skfd, interface, SIOCGIWRTLGETWDSINFO, &wrq) < 0) {
      close( skfd );
      return -1;
    }

    close( skfd );

    return 0;
}

#endif

int getWlVersion( char *interface, char *verstr )
{
    int skfd;
    unsigned char vernum[4];
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);

    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0) {
      close( skfd );
      /* If no wireless name : no wireless extensions */
      return -1;
    }

    memset(vernum, 0, 4);
    wrq.u.data.pointer = (caddr_t)&vernum[0];
    wrq.u.data.length = sizeof(vernum);

    if (iw_get_ext(skfd, interface, SIOCGIWRTLDRVVERSION, &wrq) < 0) {
      close( skfd );
      return -1;
    }

    close( skfd );

    sprintf(verstr, "%d.%d.%d", vernum[0], vernum[1], vernum[2]);

    return 0;
}

char *getWlanIfName_web(void)
{
	if(wlan_idx == 0)
		return (char *)WLANIF[0];
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
	else if(wlan_idx == 1)
		return (char *)WLANIF[1];
#endif //CONFIG_RTL_92D_SUPPORT

	printf("%s: Wrong wlan_idx!\n", __func__);

	return NULL;
}

char *getWlanIfName(void)
{
#ifdef CONFIG_RTL_92D_SUPPORT
	char wlanBand2G5GSelect;
	if(wlan_idx == 1){
		mib_get(MIB_WLAN_BAND2G5G_SELECT, (void *)&wlanBand2G5GSelect);
		if(wlanBand2G5GSelect == BANDMODESINGLE) {
			return (char *)WLANIF[0];
		}
	}
#endif //CONFIG_RTL_92D_SUPPORT

	return getWlanIfName_web();
}

int useWlanIfVirtIdx(void)
{
#ifdef CONFIG_RTL_92D_SUPPORT
	char wlanBand2G5GSelect, wlanBand2G5GSelect_single;
	mib_get(MIB_WLAN_BAND2G5G_SELECT, (void *)&wlanBand2G5GSelect);
	mib_get(MIB_WLAN_BAND2G5G_SINGLE_SELECT, (void *)&wlanBand2G5GSelect_single);
	if(wlanBand2G5GSelect == BANDMODESINGLE && wlanBand2G5GSelect_single == BANDMODE2G)
		return 1;
	else
#endif
		return 0;
}

void getWscPidName(char *wscd_pid_name)
{
	int orig_wlan_idx = wlan_idx;
#if defined(CONFIG_RTL_92D_DMDP) || defined(WLAN_DUALBAND_CONCURRENT)
	int pid;
	pid = read_pid((char *) WLAN0_WLAN1_WSCDPID);
	if (pid > 0) {
		sprintf(wscd_pid_name, "%s", WLAN0_WLAN1_WSCDPID);
		return; 
	}
	wlan_idx = 1;
	sprintf(wscd_pid_name, WSCDPID, getWlanIfName());
	pid = read_pid((char *) wscd_pid_name);
	if (pid > 0) {
		wlan_idx = orig_wlan_idx;
		return;
	}
#endif
	wlan_idx = 0;
	sprintf(wscd_pid_name, WSCDPID, getWlanIfName());
	wlan_idx = orig_wlan_idx;
}

//check flash config if wlan0 is up, called by BOA,
//0:down, 1:up
int wlan_is_up(void)
{
  int value=0;
  MIB_CE_MBSSIB_T Entry;
  mib_chain_get(MIB_MBSSIB_TBL,0,&Entry);
	
    if (Entry.wlanDisabled==0) {  // wlan0 is enabled
    	value=1;
    }
	return value;
}
#ifdef WLAN_WISP
void getWispWanID(int *idx)
{
	int i, mibtotal;
	MIB_CE_ATM_VC_T Entry;
	
	mibtotal = mib_chain_total(MIB_ATM_VC_TBL);
	for(i=0; i<mibtotal; i++)
	{
		if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			continue;

		if(MEDIA_INDEX(Entry.ifIndex) == MEDIA_WLAN)
			break;
	}

	*idx = ETH_INDEX(Entry.ifIndex);
}

//return wisp wan interface name
void getWispWanName(char *name)
{
	int wisp_wan_id;
	getWispWanID(&wisp_wan_id);
	snprintf(name, IFNAMSIZ, "wlan%d-vxd", wisp_wan_id);
}
void setWlanDevFlag(char *ifname, int set_wan)
{
	char cmd_str[100];

	if(set_wan){
		snprintf(cmd_str, sizeof(cmd_str), "echo \"%s write 0x2000\" > /proc/netdev_flag", ifname);
		system(cmd_str);
	}
	else{
		snprintf(cmd_str, sizeof(cmd_str), "echo \"%s write 0x4000\" > /proc/netdev_flag", ifname);
		system(cmd_str);
		snprintf(cmd_str, sizeof(cmd_str), "ifconfig %s 0.0.0.0", ifname);
		system(cmd_str);
	}
}
#endif

#ifdef CONFIG_WIFI_SIMPLE_CONFIG //WPS
// this is a workaround for wscd to get MIB id without including "mib.h" (which causes compile error), andrew

const int gMIB_WSC_DISABLE			  = MIB_WSC_DISABLE;
const int gMIB_WSC_CONFIGURED         = MIB_WSC_CONFIGURED;
const int gMIB_WLAN_SSID              = MIB_WLAN_SSID;
const int gMIB_WSC_SSID               = MIB_WSC_SSID;
const int gMIB_WLAN_AUTH_TYPE         = MIB_WLAN_AUTH_TYPE;
const int gMIB_WLAN_ENCRYPT           = MIB_WLAN_ENCRYPT;
const int gMIB_WSC_AUTH               = MIB_WSC_AUTH;
const int gMIB_WLAN_WPA_AUTH          = MIB_WLAN_WPA_AUTH;
const int gMIB_WLAN_WPA_PSK           = MIB_WLAN_WPA_PSK;
const int gMIB_WLAN_WPA_PSK_FORMAT    = MIB_WLAN_WPA_PSK_FORMAT;
const int gMIB_WSC_PSK                = MIB_WSC_PSK;
const int gMIB_WLAN_WPA_CIPHER_SUITE  = MIB_WLAN_WPA_CIPHER_SUITE;
const int gMIB_WLAN_WPA2_CIPHER_SUITE = MIB_WLAN_WPA2_CIPHER_SUITE;
const int gMIB_WLAN_WEP               = MIB_WLAN_WEP;
const int gMIB_WLAN_WEP64_KEY1        = MIB_WLAN_WEP64_KEY1;
const int gMIB_WLAN_WEP64_KEY2        = MIB_WLAN_WEP64_KEY2;
const int gMIB_WLAN_WEP64_KEY3        = MIB_WLAN_WEP64_KEY3;
const int gMIB_WLAN_WEP64_KEY4        = MIB_WLAN_WEP64_KEY4;
const int gMIB_WLAN_WEP128_KEY1       = MIB_WLAN_WEP128_KEY1;
const int gMIB_WLAN_WEP128_KEY2       = MIB_WLAN_WEP128_KEY2;
const int gMIB_WLAN_WEP128_KEY3       = MIB_WLAN_WEP128_KEY3;
const int gMIB_WLAN_WEP128_KEY4       = MIB_WLAN_WEP128_KEY4;
const int gMIB_WLAN_WEP_DEFAULT_KEY   = MIB_WLAN_WEP_DEFAULT_KEY;
const int gMIB_WLAN_WEP_KEY_TYPE      = MIB_WLAN_WEP_KEY_TYPE;
const int gMIB_WSC_ENC                = MIB_WSC_ENC;
const int gMIB_WSC_CONFIG_BY_EXT_REG  = MIB_WSC_CONFIG_BY_EXT_REG;
const int gMIB_WSC_PIN = MIB_WSC_PIN;
const int gMIB_ELAN_MAC_ADDR = MIB_ELAN_MAC_ADDR;
const int gMIB_WLAN_MODE = MIB_WLAN_MODE;
const int gMIB_WSC_REGISTRAR_ENABLED = MIB_WSC_REGISTRAR_ENABLED;
const int gMIB_WLAN_CHAN_NUM = MIB_WLAN_CHAN_NUM;
const int gMIB_WSC_UPNP_ENABLED = MIB_WSC_UPNP_ENABLED;
const int gMIB_WSC_METHOD = MIB_WSC_METHOD;
const int gMIB_WSC_MANUAL_ENABLED = MIB_WSC_MANUAL_ENABLED;
const int gMIB_SNMP_SYS_NAME = MIB_SNMP_SYS_NAME;
const int gMIB_WLAN_NETWORK_TYPE = MIB_WLAN_NETWORK_TYPE;
const int gMIB_WLAN_WSC_VERSION	= MIB_WSC_VERSION;

#ifdef CONFIG_RTL_92D_SUPPORT
const int gMIB_WLAN_BAND2G5G_SELECT				= MIB_WLAN_BAND2G5G_SELECT;
#endif //CONFIG_RTL_92D_DMDP

int mib_update_all()
{
	return mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
}
#endif

/*
 *	Setup firewall for fon
 */
#ifdef CONFIG_USER_FON
int setFonFirewall()
{
	// iptables -N fongw
	va_cmd(IPTABLES, 2, 1, "-N", "fongw");

	// iptables -A INPUT -i tun0 -j fongw
	va_cmd(IPTABLES, 6, 1, (char *)FW_ADD, (char *)FW_INPUT, "-i", (char *)FONIF, "-j", "fongw");

	// iptables -A fongw -m state --state RELATED,ESTABLISHED -j ACCEPT
	va_cmd(IPTABLES, 8, 1, (char *)FW_ADD, "fongw",
		"-m", "state", "--state", "RELATED,ESTABLISHED", "-j", (char *)FW_ACCEPT);

	// iptables -A fongw -p tcp -m tcp --dport 3990 --syn -j ACCEPT
	va_cmd(IPTABLES, 11, 1, (char *)FW_ADD, "fongw",
		"-p", "tcp", "-m", "tcp", "--dport", "3990", "--syn", "-j", (char *)FW_ACCEPT);
	// iptables -A fongw -p udp -m udp --dport 53 -j ACCEPT
	va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, "fongw",
		"-p", "udp", "-m", "udp", "--dport", "53", "-j", (char *)FW_ACCEPT);

	// iptables -A fongw -j DROP
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, "fongw", "-j", (char *)FW_DROP);


	// iptables -N fongw_fwd
	va_cmd(IPTABLES, 2, 1, "-N", "fongw_fwd");

	//iptables -A FORWARD -j fongw_fwd
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", "fongw_fwd");

	// iptables -A fongw_fwd -i tun0 -j ACCEPT
	va_cmd(IPTABLES, 6, 1, (char *)FW_ADD, "fongw_fwd", "-i", (char *)FONIF, "-j", (char *)FW_ACCEPT);

	// iptables -A fongw_fwd -o tun0 -j ACCEPT
	va_cmd(IPTABLES, 6, 1, (char *)FW_ADD, "fongw_fwd", "-o", (char *)FONIF, "-j", (char *)FW_ACCEPT);

}
#endif


#if	defined(CONFIG_LUNA_DUAL_LINUX)
static void createVWLAN_mac(char *name, unsigned char *mac)
{
	FILE *fp;

	fp = fopen(name, "w");
	if(fp) {
		fprintf(fp, "%x %x %x %x %x %x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		fclose(fp);
	}
	return;
}

static int set_vwlan_hwaddr()
{
	unsigned char value[6];
	unsigned char ethaddr[6];
	char macaddr[13];
	int i;
#ifdef WLAN_MBSSID
	char para2[20];
#endif
	int hwaddr_ind = 1;

	if (mib_get(MIB_ELAN_MAC_ADDR, (void *)value) != 0)
	{

#ifdef WLAN_MBSSID
		value[5] += WLAN_MBSSID_NUM;
#endif
		value[5] = value[5] + 1;
		snprintf(macaddr, 13, "%02x%02x%02x%02x%02x%02x",
		value[0], value[1], value[2], value[3], value[4], value[5]);
		va_cmd(IFCONFIG, 4, 1, "wlan1", "hw", "ether", macaddr);

		memcpy(ethaddr, value, 6);
#ifdef	WLAN_MBSSID
		hwaddr_ind += WLAN_MBSSID_NUM;
#endif
#if 0 //def WLAN_UNIVERSAL_REPEATER //	CONFIG_RTL_REPEATER_MODE_SUPPORT
		hwaddr_ind += 1;
#endif

#ifdef WLAN_WDS
		hwaddr_ind += (MAX_WDS_NUM);
#endif
		ethaddr[5] += hwaddr_ind;
	 	createVWLAN_mac((char *)WLAN1_MAC_FILE, ethaddr);

#ifdef WLAN_MBSSID
		// Set macaddr for VAP
		for (i=1; i<=WLAN_MBSSID_NUM; i++) {
			value[5] = value[5] + 1;

			snprintf(macaddr, 13, "%02x%02x%02x%02x%02x%02x",
				value[0], value[1], value[2], value[3], value[4], value[5]);

			snprintf(para2, sizeof(para2), "wlan1-vap%d", i-1);

			va_cmd(IFCONFIG, 4, 1, para2, "hw", "ether", macaddr);
		}
#endif

	}
}
static int ping(char *ipaddr)
{
	char cmd[100];
	int status, ping_rtn;
	snprintf(cmd, sizeof(cmd), "ping -w 2 %s > /dev/null", ipaddr);

	status = system(cmd);
	ping_rtn = status;
	if (status != -1) {
		ping_rtn = WEXITSTATUS(status);
		printf("ping returned: %sConnected\n", ping_rtn ? "Not " : "");
	} else {
		fprintf(stderr, "The system(3) function returned -1\n");
	}
	return ping_rtn;
}
#if 0
static int ping_test(char *ipaddr)
{
	int status = 0;
	int num = 0;
	char disabled;
	int orig_idx = wlan_idx;
	status = ping(ipaddr);

	while(status){
		printf("Machine not reachable, status %d\n", status);
		sleep(2);
		status = ping(ipaddr);
		num++;
		if(num > 5) {
			disabled = 2;
			wlan_idx = 1;
			update_wlan_disable(disabled);
			wlan_idx = orig_idx;
			printf("Disable wlan1 interface\n");
			Commit();
			return status;
		}
	}
	printf("Could ping %s successfully, status %d\n", ipaddr, status);
	disabled = 0;
	wlan_idx = 1;
	update_wlan_disable(disabled);
	wlan_idx = orig_idx;
	Commit();
	return status;
}
#endif
int setup_vwlan()
{
#if defined(CONFIG_VIRTUAL_WLAN_DRIVER)
	unsigned char vCharget;
	int orig_idx = wlan_idx;
#endif

//	va_cmd(BRCTL, 3, 1, "addif", BRIF, "wlan1");
//	va_cmd(IFCONFIG, 2, 1, "wlan1", "up");
	va_cmd(IFCONFIG, 4, 1, "vwlan", "hw", "ether", CONFIG_DEFAULT_MASTER_IPC_MAC_ADDRESS);
	va_cmd(IFCONFIG, 4, 1, "vwlan", "10.253.253.1", "netmask", "255.255.255.252");
	va_cmd(IFCONFIG, 2, 1, "vwlan", "up");
	va_cmd("/bin/arp", 3, 1 , "-s", "10.253.253.2", CONFIG_DEFAULT_SLAVE_IPC_MAC_ADDRESS);
	system("ping -c 1 10.253.253.2 > /dev/null");
#if defined(CONFIG_VIRTUAL_WLAN_DRIVER) && defined(CONFIG_SLAVE_WLAN1_ENABLE)
	set_vwlan_hwaddr();
#endif
}

int update_vwlan_disable( char *interface, int disable )
{
	if (!disable){
		system("echo 2 > /proc/vwlan");
			return 1;
	}
	else{
		system("echo 3 > /proc/vwlan");
			return 1;
	}
	return -1;
}

#endif

int setup_wlan_block(void)
{
	unsigned char enable = 0;

	va_cmd(EBTABLES, 2, 1, "-F", "wlan_block");

	// Between ELAN & WIFI
	mib_get(MIB_WLAN_BLOCK_ETH2WIR, (void *)&enable);
#ifdef CONFIG_RTK_RG_INIT
	rg_eth2wire_block(enable);
#else
	if(enable)
	{
		va_cmd(EBTABLES, 8, 1, "-A", "wlan_block", "-i", "wlan+", "-o", "eth0.+", "-j", "DROP");
		va_cmd(EBTABLES, 8, 1, "-A", "wlan_block", "-i", "eth0.+", "-o", "wlan+", "-j", "DROP");
	}
#endif

#ifdef WLAN_MBSSID
	// Between MBSSIDs
	mib_get(MIB_WLAN_BLOCK_MBSSID, (void *)&enable);
	if(enable)
		va_cmd(EBTABLES, 8, 1, "-A", "wlan_block", "-i", "wlan+", "-o", "wlan+", "-j", "DROP");
#endif
	return 0;
}

#ifdef WIFI_TIMER_SCHEDULE
void updateWifiSchedCrondFile(char *pathname, int startup)
{
	char *strVal;
	int totalnum, i, j, first, crond_pid;
	FILE *fp, *getPIDS;
	char tmpbuf[100], day_string[20], kill_cmd[100], filename[100];
	MIB_CE_WIFI_TIMER_EX_T Entry_ex;
	MIB_CE_WIFI_TIMER_T Entry;
	int wifiSchedFileExist = 0;
	unsigned char startHour, startMinute, endHour, endMinute;

	if(startup){
		snprintf(tmpbuf, 100, "mkdir -p %s", pathname);
		system(tmpbuf);
	}
	if ( !mib_get(MIB_SUSER_NAME, (void *)tmpbuf) ) {
		printf("ERROR: Get superuser name from MIB database failed.\n");
		return;
	}
	snprintf(filename, 100, "%s/%s", pathname, tmpbuf);

	crond_pid = read_pid("/var/run/crond.pid");
	if(crond_pid > 0){
		kill(crond_pid, 9);
		unlink("/var/run/crond.pid");
	}				
	
	fp = fopen(filename, "w");
	

	totalnum = mib_chain_total(MIB_WIFI_TIMER_EX_TBL);
	for(i=0; i<totalnum; i++){
		mib_chain_get(MIB_WIFI_TIMER_EX_TBL, i, &Entry_ex);
		
		first = 1;
		if(Entry_ex.day & (1<<7)){
			if(first){
				snprintf(day_string, 20, "0");
				first = 0;
			}
			else
				snprintf(day_string, 20,"%s,0", day_string);
		}
		for(j=1;j<7;j++){
			if(Entry_ex.day & (1<<j)){
				if(first){
					snprintf(day_string, 20, "%d", j);
					first = 0;
				}
				else
					snprintf(day_string, 20,"%s,%d", day_string, j);
			}
		}
		if(Entry_ex.enable){
			sscanf(Entry_ex.Time, "%hhu:%hhu", &startHour, &startMinute);
			snprintf(tmpbuf, 100, "%hhu %hhu * * %s /bin/config_wlan %d\n", startMinute, startHour, day_string, Entry_ex.onoff);
			fputs(tmpbuf,fp);
			if(wifiSchedFileExist == 0)
					wifiSchedFileExist = 1;
		}
	}

	totalnum = mib_chain_total(MIB_WIFI_TIMER_TBL);
	for(i=0; i<totalnum; i++){
		mib_chain_get(MIB_WIFI_TIMER_TBL, i, &Entry);
		if(Entry.enable){
			sscanf(Entry.startTime, "%hhu:%hhu", &startHour, &startMinute);
			snprintf(tmpbuf, 100, "%hhu %hhu */%hhu * * /bin/config_wlan 1\n", startMinute, startHour, Entry.controlCycle);
			fputs(tmpbuf,fp);
			sscanf(Entry.endTime, "%hhu:%hhu", &endHour, &endMinute);
			snprintf(tmpbuf, 100, "%hhu %hhu */%hhu * * /bin/config_wlan 0\n", endMinute, endHour, Entry.controlCycle);
			fputs(tmpbuf,fp);
			if(wifiSchedFileExist == 0)
					wifiSchedFileExist = 1;
		}
	}

	fclose(fp);

	if(wifiSchedFileExist) {
		// file is not empty
		va_cmd("/bin/crond", 0, 1);
	}
	else
		unlink(filename);
}
#endif
