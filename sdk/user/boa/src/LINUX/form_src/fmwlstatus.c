/*
 *      Web server handler routines for wireless status
 *
 */

#include <string.h>
#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"
#include "debug.h"

static void getEncryption(MIB_CE_MBSSIB_T *Entry, char *buffer)
{
	switch (Entry->encrypt) {
		case WIFI_SEC_WEP:
			if (Entry->wep == WEP_DISABLED)
				strcpy(buffer, "Disabled");
			else if (Entry->wep == WEP64 )
				strcpy(buffer, "WEP 64bits");
			else if (Entry->wep == WEP128)
				strcpy(buffer, "WEP 128bits");
			else
				buffer[0] = '\0';
			break;
		case WIFI_SEC_NONE:
		case WIFI_SEC_WPA:
			strcpy(buffer, wlan_encrypt[Entry->encrypt]);
			break;
		case WIFI_SEC_WPA2:
			strcpy(buffer, wlan_encrypt[3]);
			break;
		case WIFI_SEC_WPA2_MIXED:
			strcpy(buffer, wlan_encrypt[4]);
			break;
#ifdef CONFIG_RTL_WAPI_SUPPORT
		case WIFI_SEC_WAPI:
			strcpy(buffer, wlan_encrypt[5]);
			break;
#endif
		default:
			strcpy(buffer, wlan_encrypt[0]);
	}
}

static void getTranSSID(char *buff, char *ssid)
{
	memset(buff, '\0', 200);
	memcpy(buff, ssid, MAX_SSID_LEN);
	translate_control_code(buff);
}

static void getEncryptionFromDriver(char *interface_name, MIB_CE_MBSSIB_T *Entry, char *buffer)
{
	char encmode;
	int wpa_chiper, wpa2_chiper;
	char mib_name[30];
	sprintf(mib_name, "encmode");
	getWlEnc(interface_name, mib_name, &encmode);
	if(encmode == WIFI_SEC_NONE)
		strcpy(buffer, wlan_encrypt[0]);
	else if(encmode == WIFI_SEC_WPA || encmode == WIFI_SEC_WPA2){
		sprintf(mib_name, "wpa_cipher");
		getWlWpaChiper(interface_name, mib_name, &wpa_chiper);
		sprintf(mib_name, "wpa2_cipher");
		getWlWpaChiper(interface_name, mib_name, &wpa2_chiper);
		if(wpa2_chiper==0)
			strcpy(buffer, wlan_encrypt[2]);
		else if(wpa_chiper==0)
			strcpy(buffer, wlan_encrypt[3]);
		else
			strcpy(buffer, wlan_encrypt[4]);
	}
	else if(encmode == 1 || encmode == 5) 
		strcpy(buffer, "WEP");
	else
		buffer[0] = '\0';

}

#ifdef WLAN_QTN
int qtn_wlstatus(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0, i, k;
	MIB_CE_MBSSIB_T Entry, Entry2;
	char vChar, vChar2, vChar3, wlan_disabled;
	int vInt, vInt2;
	bss_info bss;
	unsigned char buffer[64], buffer2[64];
	unsigned char translate_ssid[200];
	int wlan_num=1;

	for (i=0; i<wlan_num; i++) {
		mib_chain_get(MIB_MBSSIB_TBL, 0, &Entry);
		vChar = Entry.wlanBand;
		if ( rt_qcsapi_get_bss_info(getWlanIfName(), &bss) < 0){
			//wlan_idx = orig_wlan_idx;
			return -1;
		}	
		getTranSSID(translate_ssid, bss.ssid);
		nBytesSent += boaWrite(wp,
			"wlanMode[%d]=%d;\n\tnetworkType[%d]=%d;\n"
			"\tband[%d]=%d;\n\tssid_drv[%d]='%s';\n",
			i, Entry.wlanMode, i, 0, i, vChar, i, translate_ssid);
		/* Encryption */
		if((Entry.wlanMode == AP_MODE) || (Entry.wlanMode == AP_WDS_MODE))
			getEncryption(&Entry, buffer);
		#ifdef WLAN_CLIENT
		else
		//	getEncryptionFromDriver(getWlanIfName(), &Entry, buffer);
			rt_get_SSID_encryption(getWlanIfName(), Entry.ssid, buffer);
		#endif
			
		/* WDS encryption */
		#ifdef WLAN_WDS
		mib_get(MIB_WLAN_WDS_ENCRYPT, (void *)&vChar);
		#else
		vChar = WEP_DISABLED;
		#endif
		if (vChar == WEP_DISABLED)
			strcpy( buffer2, "Disabled" );
		else if (vChar == WEP64)
			strcpy( buffer2, "WEP 64bits" );
		else if (vChar == WEP128)
			strcpy( buffer2, "WEP 128bits" );
		else
			buffer2[0]='\0';
		nBytesSent += boaWrite(wp,
			"\tchannel_drv[%d]='%d';\n\twep[%d]='%s';\n"
			"\twdsEncrypt[%d]='%s';\n\tmeshEncrypt[%d]='';\n",
			i, bss.channel, i, buffer, i, buffer2, i, "Disabled");
		nBytesSent += boaWrite(wp,
			"\tbssid_drv[%d]='%02x:%02x:%02x:%02x:%02x:%02x';\n",
			i, bss.bssid[0], bss.bssid[1], bss.bssid[2],
			bss.bssid[3], bss.bssid[4], bss.bssid[5]);
		/* client number */
		wlan_disabled = Entry.wlanDisabled;
		if (wlan_disabled == 1) // disabled
			vInt = 0;
		else {
			if ( rt_qcsapi_get_sta_num(getWlanIfName(), &vInt) < 0)
				vInt = 0;
		}
		/* state */
		switch (bss.state) {
		case STATE_DISABLED:
			strcpy( buffer, "Disabled");
			break;
		case STATE_IDLE:
			strcpy( buffer, "Idle");
			break;
		case STATE_STARTED:
			strcpy( buffer, "Started");
			break;
		case STATE_CONNECTED:
			strcpy( buffer, "Connected");
			break;
		case STATE_WAITFORKEY:
			strcpy( buffer, "Waiting for keys");
			break;
		case STATE_SCANNING:
			strcpy( buffer, "Scanning");
			break;
		default:
			buffer[0]='\0';;
		}
		#ifdef WLAN_UNIVERSAL_REPEATER
		/* Is Repeater enabled ? */
		mib_get(MIB_REPEATER_ENABLED1, (void *)&vChar2);
		mib_get(MIB_WLAN_NETWORK_TYPE, (void *)&vChar3);
		if (vChar2 != 0 && Entry.wlanMode != WDS_MODE &&
			!(Entry.wlanMode==CLIENT_MODE && vChar3==ADHOC))
			vInt2 = 1;
		else
		#endif
			vInt2 = 0;
		nBytesSent += boaWrite(wp,
			"\tclientnum[%d]='%d';\n\tstate_drv[%d]='%s';\n"
			"\twlanDisabled[%d]=%d;\n\trp_enabled[%d]=%d;\n",
			i, vInt, i, buffer, i, wlan_disabled, i, vInt2);
		#ifdef WLAN_UNIVERSAL_REPEATER
		/*------- Repeater Interface -------*/
		mib_chain_get(MIB_MBSSIB_TBL, WLAN_REPEATER_ITF_INDEX, (void *)&Entry2);
		/* Repeater mode */
		if (Entry.wlanMode == AP_MODE || Entry.wlanMode == AP_WDS_MODE)
			vInt = CLIENT_MODE;
		else
			vInt = AP_MODE;
		/* Repeater encryption */
		sprintf(buffer2, "wlan%d-vxd", wlan_idx);
		if(Entry.wlanMode == CLIENT_MODE)
			getEncryption(&Entry2, buffer);
		else
			getEncryptionFromDriver(buffer2, &Entry2, buffer);
		if (rt_qcsapi_get_bss_info(buffer2, &bss)<0)
			printf("getWlBssInfo failed\n");
		getTranSSID(translate_ssid, bss.ssid);
		nBytesSent += boaWrite(wp,
			"\trp_mode[%d]=%d;\n\trp_encrypt[%d]='%s';\n"
			"\trp_ssid[%d]='%s';\n",
			i, vInt, i, buffer, i, translate_ssid);
		/* Repeater bssid */
		nBytesSent += boaWrite(wp,
			"\trp_bssid[%d]='%02x:%02x:%02x:%02x:%02x:%02x';\n",
			i, bss.bssid[0], bss.bssid[1], bss.bssid[2],
			bss.bssid[3], bss.bssid[4], bss.bssid[5]);
		/* Repeater state */
		switch (bss.state) {
		case STATE_DISABLED:
			strcpy( buffer, "Disabled");
			break;
		case STATE_IDLE:
			strcpy( buffer, "Idle");
			break;
		case STATE_STARTED:
			strcpy( buffer, "Started");
			break;
		case STATE_CONNECTED:
			strcpy( buffer, "Connected");
			break;
		case STATE_WAITFORKEY:
			strcpy( buffer, "Waiting for keys");
			break;
		case STATE_SCANNING:
			strcpy( buffer, "Scanning");
			break;
		default:
			buffer[0]='\0';;
		}
		/* Repeater client number */
		rt_qcsapi_get_sta_num(buffer2, &vInt);
		nBytesSent += boaWrite(wp,
			"\trp_state[%d]='%s';\n\trp_clientnum[%d]='%d';\n",
			i, buffer, i, vInt);
		#endif // of WLAN_UNIVERSAL_REPEATER
		#ifdef WLAN_MBSSID
		nBytesSent += boaWrite(wp,
			"\tmssid_num=%d;\n", WLAN_MBSSID_NUM);
		/*-------------- VAP Interface ------------*/
		for (k=0; k<WLAN_MBSSID_NUM; k++) {
			//wlan_idx = orig_wlan_idx;
			mib_chain_get(MIB_MBSSIB_TBL, WLAN_VAP_ITF_INDEX+k, (void *)&Entry2);
			if(Entry2.wlanDisabled){
				nBytesSent += boaWrite(wp,
					"\tmssid_disable[%d]=%d;\n",
					k, Entry2.wlanDisabled);
			}
			else{
				sprintf(buffer, "%s-vap%d", getWlanIfName(), k);
				if (rt_qcsapi_get_bss_info(buffer, &bss)<0)
					printf("getWlBssInfo failed\n");
				getTranSSID(translate_ssid, bss.ssid);
				nBytesSent += boaWrite(wp,
					"\tmssid_ssid_drv[%d]='%s';\n\tmssid_band[%d]=%d;\n"
					"\tmssid_disable[%d]=%d;\n",
					k, translate_ssid, k, Entry2.wlanBand, k, Entry2.wlanDisabled);
				nBytesSent += boaWrite(wp,
					"\tmssid_bssid_drv[%d]='%02x:%02x:%02x:%02x:%02x:%02x';\n",
					k, bss.bssid[0], bss.bssid[1], bss.bssid[2],
					bss.bssid[3], bss.bssid[4], bss.bssid[5]);
				/* VAP client number */
				rt_qcsapi_get_sta_num(buffer, &vInt);
				/* VAP encryption */
				getEncryption(&Entry2, buffer2);
				nBytesSent += boaWrite(wp,
					"\tmssid_clientnum[%d]='%d';\n\tmssid_wep[%d]='%s';\n",
					k, vInt, k, buffer2);
			}
		}
		#else // of WLAN_MBSSID
		nBytesSent += boaWrite(wp,
			"\tmssid_num=0;\n");
		#endif
		//wlan_idx = orig_wlan_idx;
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN0_5G_SUPPORT) || defined(WLAN1_5G_SUPPORT)
		mib_get( MIB_WLAN_PHY_BAND_SELECT, (void *)&vChar);
#else //CONFIG_RTL_92D_SUPPORT
		vChar = PHYBAND_2G;
#endif //CONFIG_RTL_92D_SUPPORT
		nBytesSent += boaWrite(wp, "\tBand2G5GSupport=%d;\n", vChar);
	}
//	wlan_idx = orig_wlan_idx;
	return nBytesSent;

}
#endif
int wlStatus_parm(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0, i, k;
	MIB_CE_MBSSIB_T Entry, Entry2;
	char vChar, vChar2, vChar3, wlan_disabled;
	int vInt, vInt2;
	bss_info bss;
	unsigned char buffer[64], buffer2[64];
	unsigned char translate_ssid[200];
	int wlan_num=1;
	//int orig_wlan_idx;
	//orig_wlan_idx = wlan_idx;

#ifdef WLAN_QTN
#ifdef WLAN1_QTN
	if(wlan_idx==1)
#else
	if(wlan_idx==0)
#endif
		return qtn_wlstatus(eid, wp, argc, argv);
#endif
	
	for (i=0; i<wlan_num; i++) {
		mib_chain_get(MIB_MBSSIB_TBL, 0, &Entry);
		vChar = Entry.wlanBand;
		if ( getWlBssInfo(getWlanIfName(), &bss) < 0){
			//wlan_idx = orig_wlan_idx;
			return -1;
		}	
		getTranSSID(translate_ssid, bss.ssid);
		nBytesSent += boaWrite(wp,
			"wlanMode[%d]=%d;\n\tnetworkType[%d]=%d;\n"
			"\tband[%d]=%d;\n\tssid_drv[%d]='%s';\n",
			i, Entry.wlanMode, i, 0, i, vChar, i, translate_ssid);
		/* Encryption */
		if((Entry.wlanMode == AP_MODE) || (Entry.wlanMode == AP_WDS_MODE))
			getEncryption(&Entry, buffer);
		else
			getEncryptionFromDriver(getWlanIfName(), &Entry, buffer);
			
		/* WDS encryption */
		#ifdef WLAN_WDS
		mib_get(MIB_WLAN_WDS_ENCRYPT, (void *)&vChar);
		#else
		vChar = WEP_DISABLED;
		#endif
		if (vChar == WEP_DISABLED)
			strcpy( buffer2, "Disabled" );
		else if (vChar == WEP64)
			strcpy( buffer2, "WEP 64bits" );
		else if (vChar == WEP128)
			strcpy( buffer2, "WEP 128bits" );
		else
			buffer2[0]='\0';
		nBytesSent += boaWrite(wp,
			"\tchannel_drv[%d]='%d';\n\twep[%d]='%s';\n"
			"\twdsEncrypt[%d]='%s';\n\tmeshEncrypt[%d]='';\n",
			i, bss.channel, i, buffer, i, buffer2, i, "Disabled");
		nBytesSent += boaWrite(wp,
			"\tbssid_drv[%d]='%02x:%02x:%02x:%02x:%02x:%02x';\n",
			i, bss.bssid[0], bss.bssid[1], bss.bssid[2],
			bss.bssid[3], bss.bssid[4], bss.bssid[5]);
		/* client number */
		wlan_disabled = Entry.wlanDisabled;
		if (wlan_disabled == 1) // disabled
			vInt = 0;
		else {
			if ( getWlStaNum(getWlanIfName(), &vInt) < 0)
				vInt = 0;
		}
		/* state */
		switch (bss.state) {
		case STATE_DISABLED:
			strcpy( buffer, "Disabled");
			break;
		case STATE_IDLE:
			strcpy( buffer, "Idle");
			break;
		case STATE_STARTED:
			strcpy( buffer, "Started");
			break;
		case STATE_CONNECTED:
			strcpy( buffer, "Connected");
			break;
		case STATE_WAITFORKEY:
			strcpy( buffer, "Waiting for keys");
			break;
		case STATE_SCANNING:
			strcpy( buffer, "Scanning");
			break;
		default:
			buffer[0]='\0';;
		}
		#ifdef WLAN_UNIVERSAL_REPEATER
		/* Is Repeater enabled ? */
		mib_get(MIB_REPEATER_ENABLED1, (void *)&vChar2);
		mib_get(MIB_WLAN_NETWORK_TYPE, (void *)&vChar3);
		if (vChar2 != 0 && Entry.wlanMode != WDS_MODE &&
			!(Entry.wlanMode==CLIENT_MODE && vChar3==ADHOC))
			vInt2 = 1;
		else
		#endif
			vInt2 = 0;
		nBytesSent += boaWrite(wp,
			"\tclientnum[%d]='%d';\n\tstate_drv[%d]='%s';\n"
			"\twlanDisabled[%d]=%d;\n\trp_enabled[%d]=%d;\n",
			i, vInt, i, buffer, i, wlan_disabled, i, vInt2);
		#ifdef WLAN_UNIVERSAL_REPEATER
		/*------- Repeater Interface -------*/
		mib_chain_get(MIB_MBSSIB_TBL, WLAN_REPEATER_ITF_INDEX, (void *)&Entry2);
		/* Repeater mode */
		if (Entry.wlanMode == AP_MODE || Entry.wlanMode == AP_WDS_MODE)
			vInt = CLIENT_MODE;
		else
			vInt = AP_MODE;
		/* Repeater encryption */
		sprintf(buffer2, "wlan%d-vxd", wlan_idx);
		if(Entry.wlanMode == CLIENT_MODE)
			getEncryption(&Entry2, buffer);
		else
			getEncryptionFromDriver(buffer2, &Entry2, buffer);
		if (getWlBssInfo(buffer2, &bss)<0)
			printf("getWlBssInfo failed\n");
		getTranSSID(translate_ssid, bss.ssid);
		nBytesSent += boaWrite(wp,
			"\trp_mode[%d]=%d;\n\trp_encrypt[%d]='%s';\n"
			"\trp_ssid[%d]='%s';\n",
			i, vInt, i, buffer, i, translate_ssid);
		/* Repeater bssid */
		nBytesSent += boaWrite(wp,
			"\trp_bssid[%d]='%02x:%02x:%02x:%02x:%02x:%02x';\n",
			i, bss.bssid[0], bss.bssid[1], bss.bssid[2],
			bss.bssid[3], bss.bssid[4], bss.bssid[5]);
		/* Repeater state */
		switch (bss.state) {
		case STATE_DISABLED:
			strcpy( buffer, "Disabled");
			break;
		case STATE_IDLE:
			strcpy( buffer, "Idle");
			break;
		case STATE_STARTED:
			strcpy( buffer, "Started");
			break;
		case STATE_CONNECTED:
			strcpy( buffer, "Connected");
			break;
		case STATE_WAITFORKEY:
			strcpy( buffer, "Waiting for keys");
			break;
		case STATE_SCANNING:
			strcpy( buffer, "Scanning");
			break;
		default:
			buffer[0]='\0';;
		}
		/* Repeater client number */
		getWlStaNum(buffer2, &vInt);
		nBytesSent += boaWrite(wp,
			"\trp_state[%d]='%s';\n\trp_clientnum[%d]='%d';\n",
			i, buffer, i, vInt);
		#endif // of WLAN_UNIVERSAL_REPEATER
		#ifdef WLAN_MBSSID
		nBytesSent += boaWrite(wp,
			"\tmssid_num=%d;\n", WLAN_MBSSID_NUM);
		/*-------------- VAP Interface ------------*/
		for (k=0; k<WLAN_MBSSID_NUM; k++) {
			//wlan_idx = orig_wlan_idx;
			mib_chain_get(MIB_MBSSIB_TBL, WLAN_VAP_ITF_INDEX+k, (void *)&Entry2);
			if(Entry2.wlanDisabled){
				nBytesSent += boaWrite(wp,
					"\tmssid_disable[%d]=%d;\n",
					k, Entry2.wlanDisabled);
			}
			else{
				sprintf(buffer, "%s-vap%d", getWlanIfName(), k);
			if (getWlBssInfo(buffer, &bss)<0)
					printf("getWlBssInfo failed\n");
				getTranSSID(translate_ssid, bss.ssid);
				nBytesSent += boaWrite(wp,
					"\tmssid_ssid_drv[%d]='%s';\n\tmssid_band[%d]=%d;\n"
					"\tmssid_disable[%d]=%d;\n",
					k, translate_ssid, k, Entry2.wlanBand, k, Entry2.wlanDisabled);
				nBytesSent += boaWrite(wp,
					"\tmssid_bssid_drv[%d]='%02x:%02x:%02x:%02x:%02x:%02x';\n",
					k, bss.bssid[0], bss.bssid[1], bss.bssid[2],
					bss.bssid[3], bss.bssid[4], bss.bssid[5]);
				/* VAP client number */
			getWlStaNum(buffer, &vInt);
				/* VAP encryption */
				getEncryption(&Entry2, buffer2);
				nBytesSent += boaWrite(wp,
					"\tmssid_clientnum[%d]='%d';\n\tmssid_wep[%d]='%s';\n",
					k, vInt, k, buffer2);
			}
		}
		#else // of WLAN_MBSSID
		nBytesSent += boaWrite(wp,
			"\tmssid_num=0;\n");
		#endif
		//wlan_idx = orig_wlan_idx;
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN0_5G_SUPPORT) || defined(WLAN1_5G_SUPPORT)
		mib_get( MIB_WLAN_PHY_BAND_SELECT, (void *)&vChar);
#else //CONFIG_RTL_92D_SUPPORT
		vChar = PHYBAND_2G;
#endif //CONFIG_RTL_92D_SUPPORT
		nBytesSent += boaWrite(wp, "\tBand2G5GSupport=%d;\n", vChar);
	}
//	wlan_idx = orig_wlan_idx;
	return nBytesSent;
}

