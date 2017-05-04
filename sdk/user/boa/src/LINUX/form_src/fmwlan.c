/*
 *      Web server handler routines for wlan stuffs
 *
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *
 *      $Id: fmwlan.c,v 1.97 2012/11/21 13:00:11 kaohj Exp $
 *
 */

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "../webs.h"
#include "mib.h"
#include "webform.h"
#include "utility.h"
//xl_yue add
#include "../defs.h"
#include "multilang.h"


WLAN_RATE_T rate_11n_table_20M_LONG[]={
	{MCS0, 	"6.5"},
	{MCS1, 	"13"},
	{MCS2, 	"19.5"},
	{MCS3, 	"26"},
	{MCS4, 	"39"},
	{MCS5, 	"52"},
	{MCS6, 	"58.5"},
	{MCS7, 	"65"},
	{MCS8, 	"13"},
	{MCS9, 	"26"},
	{MCS10, 	"39"},
	{MCS11, 	"52"},
	{MCS12, 	"78"},
	{MCS13, 	"104"},
	{MCS14, 	"117"},
	{MCS15, 	"130"},
	{0}
};
WLAN_RATE_T rate_11n_table_20M_SHORT[]={
	{MCS0, 	"7.2"},
	{MCS1, 	"14.4"},
	{MCS2, 	"21.7"},
	{MCS3, 	"28.9"},
	{MCS4, 	"43.3"},
	{MCS5, 	"57.8"},
	{MCS6, 	"65"},
	{MCS7, 	"72.2"},
	{MCS8, 	"14.444"},
	{MCS9, 	"28.889"},
	{MCS10, 	"43.333"},
	{MCS11, 	"57.778"},
	{MCS12, 	"86.667"},
	{MCS13, 	"115.556"},
	{MCS14, 	"130"},
	{MCS15, 	"144.444"},
	{0}
};
WLAN_RATE_T rate_11n_table_40M_LONG[]={
	{MCS0, 	"13.5"},
	{MCS1, 	"27"},
	{MCS2, 	"40.5"},
	{MCS3, 	"54"},
	{MCS4, 	"81"},
	{MCS5, 	"108"},
	{MCS6, 	"121.5"},
	{MCS7, 	"135"},
	{MCS8, 	"27"},
	{MCS9, 	"54"},
	{MCS10, 	"81"},
	{MCS11, 	"108"},
	{MCS12, 	"162"},
	{MCS13, 	"216"},
	{MCS14, 	"243"},
	{MCS15, 	"270"},
	{0}
};
WLAN_RATE_T rate_11n_table_40M_SHORT[]={
	{MCS0, 	"15"},
	{MCS1, 	"30"},
	{MCS2, 	"45"},
	{MCS3, 	"60"},
	{MCS4, 	"90"},
	{MCS5, 	"120"},
	{MCS6, 	"135"},
	{MCS7, 	"150"},
	{MCS8, 	"30"},
	{MCS9, 	"60"},
	{MCS10, 	"90"},
	{MCS11, 	"120"},
	{MCS12, 	"180"},
	{MCS13, 	"240"},
	{MCS14, 	"270"},
	{MCS15, 	"300"},
	{0}
};
WLAN_RATE_T tx_fixed_rate[]={
	{1, "1"},
	{(1<<1), 	"2"},
	{(1<<2), 	"5.5"},
	{(1<<3), 	"11"},
	{(1<<4), 	"6"},
	{(1<<5), 	"9"},
	{(1<<6), 	"12"},
	{(1<<7), 	"18"},
	{(1<<8), 	"24"},
	{(1<<9), 	"36"},
	{(1<<10), 	"48"},
	{(1<<11), 	"54"},
	{(1<<12), 	"MCS0"},
	{(1<<13), 	"MCS1"},
	{(1<<14), 	"MCS2"},
	{(1<<15), 	"MCS3"},
	{(1<<16), 	"MCS4"},
	{(1<<17), 	"MCS5"},
	{(1<<18), 	"MCS6"},
	{(1<<19), 	"MCS7"},
	{(1<<20), 	"MCS8"},
	{(1<<21), 	"MCS9"},
	{(1<<22), 	"MCS10"},
	{(1<<23), 	"MCS11"},
	{(1<<24), 	"MCS12"},
	{(1<<25), 	"MCS13"},
	{(1<<26), 	"MCS14"},
	{(1<<27), 	"MCS15"},
	{0}
};

//changes in following table should be synced to VHT_MCS_DATA_RATE[] in 8812_vht_gen.c
const unsigned short VHT_MCS_DATA_RATE[3][2][20] =
        {       {       {13, 26, 39, 52, 78, 104, 117, 130, 156, 156,
                         26, 52, 78, 104, 156, 208, 234, 260, 312, 312},                        // Long GI, 20MHz
                        {14, 29, 43, 58, 87, 116, 130, 144, 173, 173,
                        29, 58, 87, 116, 173, 231, 260, 289, 347, 347}  },              // Short GI, 20MHz
                {       {27, 54, 81, 108, 162, 216, 243, 270, 324, 360,
                        54, 108, 162, 216, 324, 432, 486, 540, 648, 720},               // Long GI, 40MHz
                        {30, 60, 90, 120, 180, 240, 270, 300,360, 400,
                        60, 120, 180, 240, 360, 480, 540, 600, 720, 800}},              // Short GI, 40MHz
                {       {59, 117,  176, 234, 351, 468, 527, 585, 702, 780,
                        117, 234, 351, 468, 702, 936, 1053, 1170, 1404, 1560},  // Long GI, 80MHz
                        {65, 130, 195, 260, 390, 520, 585, 650, 780, 867,
                        130, 260, 390, 520, 780, 1040, 1170, 1300, 1560,1733}   }       // Short GI, 80MHz
        };


/////////////////////////////////////////////////////////////////////////////
#ifndef NO_ACTION
static void run_script(int mode)
{
	int pid;
	char tmpBuf[100];
	pid = fork();
	if (pid)
		waitpid(pid, NULL, 0);
	else if (pid == 0) {
		snprintf(tmpBuf, 100, "%s/%s", _CONFIG_SCRIPT_PATH, _CONFIG_SCRIPT_PROG);
#ifdef HOME_GATEWAY
		execl( tmpBuf, _CONFIG_SCRIPT_PROG, "gw", "bridge", NULL);
#else
		execl( tmpBuf, _CONFIG_SCRIPT_PROG, "ap", "bridge", NULL);
#endif
		exit(1);
	}
}
#endif

static inline int isAllStar(char *data)
{
	int i;
	for (i=0; i<strlen(data); i++) {
		if (data[i] != '*')
			return 0;
	}
	return 1;
}

#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
void formWlanRedirect(request * wp, char *path, char *query)
{
	char *redirectUrl;
	char *strWlanId;

	redirectUrl= boaGetVar(wp, "redirect-url", "");   // hidden page
	strWlanId= boaGetVar(wp, "wlan_idx", "");   // hidden page
	if(strWlanId[0]){
		wlan_idx = atoi(strWlanId);
		//sprintf(WLAN_IF, "wlan%d", wlan_idx);
	}

	if(redirectUrl[0])
		boaRedirectTemp(wp,redirectUrl); //avoid caching
}
#endif
#if defined(CONFIG_RTL_92D_SUPPORT)
static int swapWlanMibSetting(unsigned char wlanifNumA, unsigned char wlanifNumB, char *tmpBuf)
{
	int i = 0;
#ifdef WLAN_MBSSID
	MIB_CE_MBSSIB_T entryA, entryB;
	unsigned char idx;
#endif	//WLAN_MBSSID
#ifdef WLAN_ACL
	MIB_CE_WLAN_AC_T entryACL;
	int entryNumACL;
#endif

	if((wlanifNumA >= NUM_WLAN_INTERFACE) || (wlanifNumB >= NUM_WLAN_INTERFACE)) {
		printf("%s: wrong wlan interface number!\n", __func__);
		goto setErr_wlan;
	}

	for(i = DUAL_WLAN_START_ID + 1; i < DUAL_WLAN_END_ID; i += NUM_WLAN_INTERFACE){
		if(mib_swap( i, i + 1) == 0){
			strcpy(tmpBuf, multilang(LANG_SWAP_WLAN_MIB_FAILED));
			goto setErr_wlan;
		}
	}

#ifdef WLAN_MBSSID
	for(i=0; i<=WLAN_REPEATER_ITF_INDEX; i++)
		mib_chain_swap(MIB_MBSSIB_TBL, wlanifNumA * (WLAN_REPEATER_ITF_INDEX + 1) + i, wlanifNumB * (WLAN_REPEATER_ITF_INDEX + 1) + i);
#endif	//WLAN_MBSSID

#ifdef WLAN_ACL
	entryNumACL = mib_chain_total(MIB_WLAN_AC_TBL);
	for(i=0; i<entryNumACL; i++) {
		if(!mib_chain_get(MIB_WLAN_AC_TBL, i, (void *)&entryACL)) {
			strcpy(tmpBuf, strGetChainerror);
			goto setErr_wlan;
		}
		if(entryACL.wlanIdx == wlanifNumA)
			entryACL.wlanIdx = wlanifNumB;
		else if(entryACL.wlanIdx == wlanifNumB)
			entryACL.wlanIdx = wlanifNumA;
		else
			continue;

		mib_chain_update(MIB_WLAN_AC_TBL, (void *)&entryACL, i);
	}
#endif

	return 0;

setErr_wlan:
	return -1;
}

void formWlanBand2G5G(request * wp, char *path, char *query)
{
	char *submitUrl;
	char tmpBuf[100];
	char *tmpStr;
	char vChar, wlanBand2G5GSelect, origBand2G5GSelect, phyBand[NUM_WLAN_INTERFACE];
	char lan_ip[30];
	int i;

	mib_get(MIB_WLAN_BAND2G5G_SELECT, (void *)&origBand2G5GSelect);

	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page
	tmpStr = boaGetVar(wp, "wlBandMode", "");

	if(tmpStr[0]) {
		wlanBand2G5GSelect = tmpStr[0]-'0';
	}

	if(wlanBand2G5GSelect<BANDMODEBOTH || wlanBand2G5GSelect>BANDMODESINGLE) {
		strcpy(tmpBuf, multilang(LANG_GET_WRONG_BAND_MODE_VALUE));
		goto setErr_wlan;
	}
	else if(origBand2G5GSelect == wlanBand2G5GSelect) {
		goto sameBand;
	}
	else if(mib_set(MIB_WLAN_BAND2G5G_SELECT, (void *)&wlanBand2G5GSelect) == 0) {
		sprintf(tmpBuf, "%s:%s", Tset_mib_error, MIB_WLAN_BAND2G5G_SELECT);
		goto setErr_wlan;
	}

	for(i=0; i<NUM_WLAN_INTERFACE; i++) {
		wlan_idx = i;
		mib_get(MIB_WLAN_PHY_BAND_SELECT, (void *)&phyBand[i]);

		/* init all wireless interface is set radio off and DMACDPHY */
		vChar = 1;
		if(mib_set(MIB_WLAN_DISABLED, (void *)&vChar) == 0) {
			strcpy(tmpBuf, strDisbWlanErr);
			goto setErr_wlan;
		}
		vChar = DMACDPHY;
		if(mib_set(MIB_WLAN_MAC_PHY_MODE, (void *)&vChar) == 0) {
			sprintf(tmpBuf, "%s:%s", Tset_mib_error, MIB_WLAN_MAC_PHY_MODE);
			goto setErr_wlan;
		}
	}
	if(wlanBand2G5GSelect == BANDMODEBOTH) {
		for(i=0; i<NUM_WLAN_INTERFACE; i++) {
			wlan_idx = i;
			vChar = 0;
			if(mib_set(MIB_WLAN_DISABLED, (void *)&vChar) == 0) {
				strcpy(tmpBuf, strDisbWlanErr);
				goto setErr_wlan;
			}
			vChar = DMACDPHY;
			if(mib_set(MIB_WLAN_MAC_PHY_MODE, (void *)&vChar) == 0) {
				sprintf(tmpBuf, "%s:%s", Tset_mib_error, MIB_WLAN_MAC_PHY_MODE);
				goto setErr_wlan;
			}
		}

		/* 92d rule, 5g must up in wlan0 */
		/*
		for(i=0; i<NUM_WLAN_INTERFACE; i++) {
			if(phyBand[i] == PHYBAND_5G) {
				if((i != 0) && (swapWlanMibSetting(0, i, tmpBuf) < 0))
					goto setErr_wlan;
				break;
			}
		}
		*/
	}
	else if(wlanBand2G5GSelect == BANDMODESINGLE) {
		for(i=0; i<NUM_WLAN_INTERFACE; i++) {
			wlan_idx = i;
			if(i == 0) {	//enable wlan0
				vChar = 0;
				if(mib_set(MIB_WLAN_DISABLED, (void *)&vChar) == 0) {
					strcpy(tmpBuf, strDisbWlanErr);
					goto setErr_wlan;
				}
			}

			vChar = SMACSPHY;
			if(mib_set(MIB_WLAN_MAC_PHY_MODE, (void *)&vChar) == 0) {
				sprintf(tmpBuf, "%s:%s", Tset_mib_error, MIB_WLAN_MAC_PHY_MODE);
				goto setErr_wlan;
			}
		}
		vChar = 0;
		if(mib_set(MIB_WLAN_BAND2G5G_SINGLE_SELECT, (void *)&vChar) == 0) {
			strcpy(tmpBuf, strDisbWlanErr);
			goto setErr_wlan;
		}
	}
	else {
		strcpy(tmpBuf, multilang(LANG_ONLY_SUPPORT_BOTH_OR_SINGLE_MODE_SWITCH));
		goto setErr_wlan;
	}

	getMIB2Str(MIB_ADSL_LAN_IP, lan_ip);
	sprintf(tmpBuf,"%s",multilang(LANG_CHANGE_SETTING_SUCCESSFULLY_DO_NOT_REBOOT));
	OK_MSG_FW(tmpBuf, submitUrl,APPLY_COUNTDOWN_TIME,lan_ip);

#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif
	config_WLAN(ACT_RESTART);
	//OK_MSG(submitUrl);
	return;

sameBand:
	if(submitUrl[0])
		boaRedirect(wp,submitUrl);
setErr_wlan:
	ERR_MSG(tmpBuf);
}
#endif //CONFIG_RTL_92D_SUPPORT

/*
 *	Whenever band changed, it must be called to check some dependency items.
 */
static void update_on_band_changed(MIB_CE_MBSSIB_T *pEntry, int idx, int cur_band)
{
	if(wl_isNband(cur_band)) {	//n mode is enabled
		/*
		 * andrew: new test plan require N mode to
		 * avoid using TKIP.
		 */
		if (pEntry->encrypt != WIFI_SEC_WPA2_MIXED) {
			pEntry->unicastCipher = WPA_CIPHER_AES;
			pEntry->wpa2UnicastCipher= WPA_CIPHER_AES;
		}
	}
}

static void update_vap_band(unsigned char root_band)
{
	MIB_CE_MBSSIB_T Entry;
	int i;
	
	for(i=1; i<=NUM_VWLAN_INTERFACE; i++) {
		wlan_getEntry(&Entry, i);
		if (!Entry.wlanDisabled) {
			Entry.wlanBand &= root_band;
			if (Entry.wlanBand == 0) {
				Entry.wlanBand = root_band;
				update_on_band_changed(&Entry, i, Entry.wlanBand);
			}
			mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, i);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void formWlanSetup(request * wp, char *path, char *query)
{
	char *submitUrl, *strSSID, *strChan, *strDisabled, *strVal;
	char vChar, chan, disabled, mode=-1;
	NETWORK_TYPE_T net;
	char tmpBuf[100];
	int flags;
	MIB_CE_MBSSIB_T Entry;
	int warn = 0;
#ifdef WLAN_WISP
	char need_reboot = 0;
#endif

#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
	char phyBandSelect, wlanBand2G5GSelect, phyBandOrig, wlanBand2G5GSelect_single;
	int i;
	int phyBandSelectChange = 0;
	char lan_ip[30];
#endif //CONFIG_RTL_92D_SUPPORT || WLAN_DUALBAND_CONCURRENT

#if 0 //def CONFIG_RTL_92D_SUPPORT
	mib_get(MIB_WLAN_BAND2G5G_SELECT, (void *)&wlanBand2G5GSelect);

	if(wlanBand2G5GSelect == BANDMODESINGLE) {
		strVal = boaGetVar(wp, "Band2G5GSupport", "");
		if ( strVal[0] ) {
			printf("Band2G5GSupport=%d\n", strVal[0]-'0');
			phyBandSelect = strVal[0]-'0';

			for(i=0; i<NUM_WLAN_INTERFACE; i++) {
				wlan_idx = i;
				mib_get(MIB_WLAN_PHY_BAND_SELECT, (void *)&phyBandOrig);
				if(phyBandOrig == phyBandSelect) {
					if(i != 0) {
						wlan_idx = 0;
						vChar = 1;
						if(mib_set(MIB_WLAN_DISABLED, (void *)&vChar) == 0) {	//close original interface
							strcpy(tmpBuf, strDisbWlanErr);
							goto setErr_wlan;
						}
						if(swapWlanMibSetting(0, i, tmpBuf) < 0)
							goto setErr_wlan;

						vChar = 0;
						if(mib_set(MIB_WLAN_DISABLED, (void *)&vChar) == 0) {	//enable new interface
							strcpy(tmpBuf, strDisbWlanErr);
							goto setErr_wlan;
						}
					}
					break;
				}
			}
		}
	}

	strVal = boaGetVar(wp, "wlan_idx", "");
	if ( strVal[0] ) {
		printf("wlan_idx=%d\n", strVal[0]-'0');
		wlan_idx = strVal[0]-'0';
	}
#endif //CONFIG_RTL_92D_SUPPORT

#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
	strVal = boaGetVar(wp, "wlan_idx", "");
	if ( strVal[0] ) {
		printf("wlan_idx=%d\n", strVal[0]-'0');
		wlan_idx = strVal[0]-'0';
	}
#if defined(CONFIG_RTL_92D_SUPPORT)
	mib_get(MIB_WLAN_BAND2G5G_SELECT, (void *)&wlanBand2G5GSelect);
	
	if(wlanBand2G5GSelect == BANDMODESINGLE) {
		strVal = boaGetVar(wp, "Band2G5GSupport", "");
		printf("Band2G5GSupport=%d\n", strVal[0]-'0');
		phyBandSelect = strVal[0]-'0';
		mib_get(MIB_WLAN_BAND2G5G_SINGLE_SELECT, (void *)&wlanBand2G5GSelect_single);
		phyBandOrig = (wlanBand2G5GSelect_single == BANDMODE5G) ? PHYBAND_5G : PHYBAND_2G;
		if(phyBandSelect != phyBandOrig){
			phyBandSelectChange = 1;
			
			//close original interface
			if(!mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry)){
				strcpy(tmpBuf, strDisbWlanErr);
				goto setErr_wlan;
			}
			Entry.wlanDisabled = 1;
			if(!mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, 0)){
				strcpy(tmpBuf, strDisbWlanErr);
				goto setErr_wlan;
			}
			if(wlan_idx == 1) wlan_idx = 0;
			else wlan_idx = 1;
			
			//enable new interface
			if(!mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry)){
				strcpy(tmpBuf, strDisbWlanErr);
				goto setErr_wlan;
			}
			Entry.wlanDisabled = 0;
			if(!mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, 0)){
				strcpy(tmpBuf, strDisbWlanErr);
				goto setErr_wlan;
			}
			vChar = wlan_idx;
			mib_set(MIB_WLAN_BAND2G5G_SINGLE_SELECT, (void *)&vChar);
		}
		
	}
#endif
#endif

	mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry);

	strDisabled = boaGetVar(wp, "wlanDisabled", "");

	if ( !gstrcmp(strDisabled, "ON") )
		disabled = 1;
	else
		disabled = 0;

#if 0	
	if (getInFlags(getWlanIfName(), &flags) == 1) {
		if (disabled)
			flags &= ~IFF_UP;
		else
			flags |= IFF_UP;

		setInFlags(getWlanIfName(), flags);
	}
#endif
	Entry.wlanDisabled = disabled;

	if ( disabled ){
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, 0);
		goto setwlan_ret;
	}

	// Added by Mason Yu for TxPower
	strVal = boaGetVar(wp, "txpower", "");
	if ( strVal[0] ) {

		if (strVal[0] < '0' || strVal[0] > '4') {
			strcpy(tmpBuf, strInvdTxPower);
			goto setErr_wlan;
		}

		mode = strVal[0] - '0';

		if ( mib_set( MIB_TX_POWER, (void *)&mode) == 0) {
   			strcpy(tmpBuf, strSetMIBTXPOWErr);
			goto setErr_wlan;
		}

	}

	strVal = boaGetVar(wp, "mode", "");
	if ( strVal[0] ) {
		if (strVal[0]!= '0' && strVal[0]!= '1' && strVal[0]!= '2' && strVal[0]!= '3') {
			strcpy(tmpBuf, strInvdMode);
			goto setErr_wlan;
		}
		mode = strVal[0] - '0';

#ifdef WLAN_CLIENT
		if (mode == CLIENT_MODE) {
			WIFI_SECURITY_T encrypt;

			vChar = Entry.encrypt;
			encrypt = (WIFI_SECURITY_T)vChar;
			if (encrypt == WIFI_SEC_WPA || encrypt == WIFI_SEC_WPA2) {
				vChar = Entry.wpaAuth;
				if (vChar & 1) { // radius
					strcpy(tmpBuf, strSetWPAWarn);
					goto setErr_wlan;
				}
			}
			else if (encrypt == WIFI_SEC_WEP) {
				vChar = Entry.enable1X;
				if (vChar & 1) {
					strcpy(tmpBuf, strSetWEPWarn);
					goto setErr_wlan;
				}
			}
#ifndef CONFIG_SUPPORT_CLIENT_MIXED_SECURITY
			else if (encrypt == WIFI_SEC_WPA2_MIXED) {
				vChar = WIFI_SEC_WPA2;
				Entry.encrypt = vChar;
				strcpy(tmpBuf, multilang(LANG_WARNING_WPA2_MIXED_ENCRYPTION_IS_NOT_SUPPORTED_IN_CLIENT_MODE_CHANGE_TO_WPA2_ENCRYPTION));
				warn = 1;
			}
#endif
		}
#endif

		Entry.wlanMode = mode;
	}

	strSSID = boaGetVar(wp, "ssid", "");
	if ( strSSID[0] ) 
		strcpy(Entry.ssid, strSSID);
	else if ( mode == 1 && !strSSID[0] )  // client and NULL SSID
		strcpy(Entry.ssid, strSSID);
	

	strChan = boaGetVar(wp, "chan", "");
	if ( strChan[0] ) {
		errno=0;
		chan = strtol( strChan, (char **)NULL, 10);
		if (errno) {
   			strcpy(tmpBuf, strInvdChanNum);
			goto setErr_wlan;
		}
		if(chan != 0)
		{
			vChar = 0;	//disable auto channel
			if ( mib_set( MIB_WLAN_CHAN_NUM, (void *)&chan) == 0) {
				strcpy(tmpBuf, strSetChanErr);
				goto setErr_wlan;
			}
		}
		else
			vChar = 1;	//enable auto channel

		if ( mib_set( MIB_WLAN_AUTO_CHAN_ENABLED, (void *)&vChar) == 0) {
			strcpy(tmpBuf, strSetChanErr);
			goto setErr_wlan;
		}
	}
#if defined(WLAN0_5G_SUPPORT) || defined(WLAN1_5G_SUPPORT)
	{
		char band2G5GSelect = 0;
		char band_no, band_val;
		strVal = boaGetVar(wp, "Band2G5GSupport", "");
		if(strVal[0])
		{
			band2G5GSelect = atoi(strVal);
			printf("band2G5GSelect = %d\n", band2G5GSelect);
		}
		strVal = boaGetVar(wp, "band", "");
		if(strVal[0])
		{
			band_no = atoi(strVal);
			//printf("band_no = %d\n", band_no);
		}
		if(band_no==3 || band_no==11 || band_no==63 || band_no==71 || band_no==75)
			band_val = 2;
		else if(band_no==7)
		{
			band_val = band2G5GSelect;
		}
		else
			band_val = 1;

		if ( mib_set( MIB_WLAN_PHY_BAND_SELECT, (void *)&band_val) == 0) {
			strcpy(tmpBuf, multilang(LANG_SET_BAND_ERROR));
			goto setErr_wlan;
		}
	}
#endif

	char *strRate;
	unsigned short val;

	strVal = boaGetVar(wp, "band", "");
	if ( strVal[0] ) {
		mode = atoi(strVal);
		mode++;

		update_on_band_changed(&Entry, 0, mode);
		update_vap_band(mode); // Update vap band based on root band
		
		Entry.wlanBand = mode;
	}

	strRate = boaGetVar(wp, "basicrates", "");
	if ( strRate[0] ) {
		val = atoi(strRate);
		if ( mib_set(MIB_WLAN_BASIC_RATE, (void *)&val) == 0) {
			strcpy(tmpBuf, strSetBaseRateErr);
			goto setErr_wlan;
		}
	}

	strRate = boaGetVar(wp, "operrates", "");
	if ( strRate[0] ) {
		val = atoi(strRate);
		if ( mib_set(MIB_WLAN_SUPPORTED_RATE, (void *)&val) == 0) {
			strcpy(tmpBuf, strSetOperRateErr);
			goto setErr_wlan;
		}
	}

	strVal = boaGetVar(wp, "chanwid", "");            //add by yq_zhou 2.10
	if ( strVal[0] ) {
		mode = strVal[0] - '0';
#ifdef WLAN_11N_COEXIST
		if(mode == 3){ //20/40MHz
			mode = 1;
			vChar = 1;
		}
		else
			vChar = 0;
		if ( mib_set( MIB_WLAN_11N_COEXIST, (void *)&vChar) == 0) {
			//strcpy(tmpBuf, strSet11NCoexistErr);
			goto setErr_wlan;
		}
#endif
		if ( mib_set( MIB_WLAN_CHANNEL_WIDTH, (void *)&mode) == 0) {
			strcpy(tmpBuf, strSetChanWidthErr);
			goto setErr_wlan;
		}
	}

	strVal = boaGetVar(wp, "ctlband", "");            //add by yq_zhou 2.10
	if ( strVal[0] ) {
		mode = strVal[0] - '0';
		if ( mib_set( MIB_WLAN_CONTROL_BAND, (void *)&mode) == 0) {
			strcpy(tmpBuf, strSetCtlBandErr);
			goto setErr_wlan;
		}
	}
#ifdef WLAN_LIMITED_STA_NUM
	strVal = boaGetVar(wp, "wl_limitstanum", "");
	if ( strVal[0] ) {
		mode = strVal[0] - '0';
		if(mode == 0)
			Entry.stanum = 0;
		else{
			strVal = boaGetVar(wp, "wl_stanum", "");
			Entry.stanum = atoi(strVal);
		}
	}
#endif
	mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, 0);
#ifdef WLAN_UNIVERSAL_REPEATER
	strVal = boaGetVar(wp, "repeaterEnabled", "");
	mib_chain_get(MIB_MBSSIB_TBL, WLAN_REPEATER_ITF_INDEX, (void *)&Entry);
	if ( !gstrcmp(strVal, "ON") ) {
		vChar=1;
		Entry.wlanDisabled = 0;
	}
	else {
		vChar=0;
		Entry.wlanDisabled = 1;
#ifdef WLAN_WISP
		setWirelessWanEntryMode(0, wlan_idx);
#endif
	}
	mib_set( MIB_REPEATER_ENABLED1, (void *)&vChar);

	strVal = boaGetVar(wp, "repeaterSSID", "");
	if ( strVal[0] ) {
		mib_set( MIB_REPEATER_SSID1, (void *)strVal);
		strncpy(Entry.ssid, strVal, MAX_SSID_LEN);
	}
	mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, WLAN_REPEATER_ITF_INDEX);
#endif

	if(mib_get(MIB_WIFI_REGDOMAIN_DEMO, (void *)&vChar)!=0 && vChar!=0)
	{
	    strVal = boaGetVar(wp, "regdomain_demo", "");
	    if ( strVal[0] ) {
			unsigned char reg_num = (unsigned char)atoi(strVal);

			if ( mib_set( MIB_HW_REG_DOMAIN, (void *)&reg_num) == 0) {
				strcpy(tmpBuf, multilang(LANG_SET_REGDOMAIN_ERROR));
				goto setErr_wlan;
			}
		}
	}


setwlan_ret:
#ifdef CONFIG_WIFI_SIMPLE_CONFIG//WPS def WIFI_SIMPLE_CONFIG
	{
		int ret;
		char *wepKey;
		wepKey = boaGetVar(wp, "wps_clear_configure_by_reg0", "");
		ret = 0;
		if (wepKey && wepKey[0])
			ret = atoi(wepKey);
		update_wps_configured(ret);
	}
#endif

submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page

#if defined(CONFIG_RTL_92D_SUPPORT)
if(phyBandSelectChange){
	getMIB2Str(MIB_ADSL_LAN_IP, lan_ip);
	sprintf(tmpBuf,"%s",multilang(LANG_CHANGE_SETTING_SUCCESSFULLY_DO_NOT_REBOOT));
	OK_MSG_FW(tmpBuf, submitUrl,APPLY_COUNTDOWN_TIME,lan_ip);
}
#endif

#ifdef WLAN_WISP
	if(!checkWispAvailability()){
		unsigned int wan_mode;
		mib_get(MIB_WAN_MODE, (void *)&wan_mode);
		if(wan_mode & MODE_Wlan){
			wan_mode &= (~MODE_Wlan);
			mib_set(MIB_WAN_MODE, (void *)&wan_mode);
			need_reboot = 1;
			//setWirelessWanEntryMode(wan_mode, wlan_idx);
		}
	}
#endif

// Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif // of #if COMMIT_IMMEDIATELY

#ifdef WLAN_WISP
	if(need_reboot){
		SaveAndReboot(wp);
		return;
	}
#endif

#ifndef NO_ACTION
	run_script(mode);
#endif

	config_WLAN(ACT_RESTART);

//	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page
#if defined(CONFIG_RTL_92D_SUPPORT)
if(!phyBandSelectChange){
#endif
	if (warn) {
		OK_MSG1(tmpBuf, submitUrl);
	}
	else {
		OK_MSG(submitUrl);
	}
#if defined(CONFIG_RTL_92D_SUPPORT)
}
#endif
	return;

setErr_wlan:
	ERR_MSG(tmpBuf);
}

#ifdef CONFIG_RTL_WAPI_SUPPORT

#define CERT_START "-----BEGIN CERTIFICATE-----"
#define CERT_END "-----END CERTIFICATE-----"

static void formUploadWapiCert(request * wp, char * path, char * query,
	const char *name, const char *submitUrl)
{
	/*save asu and user cert*/
	char *strVal;
	char tmpBuf[128];
	char cmd[128];
	FILE *fp, *fp_input;
	int startPos,endPos,nLen,nRead,nToRead;

	if ((fp_input = _uploadGet(wp, &startPos, &endPos)) == NULL) {
		strcpy(tmpBuf, multilang(LANG_UPLOAD_FAILED));
		goto upload_ERR;
	}

	//fprintf(stderr, "%s(%d): %s,%s (%d,%d)\n", __FUNCTION__,__LINE__,
	//	 submitUrl, strVal, startPos, endPos);

	nLen = endPos - startPos;
	fseek(fp_input, startPos, SEEK_SET); // seek to the data star

	fp=fopen(WAPI_TMP_CERT,"w");
	if(NULL == fp)
	{
		strcpy(tmpBuf, multilang(LANG_CAN_NOT_OPEN_TMP_CERT));
		goto upload_ERR;
	}

	/* copy startPos - endPost to another file */
	nToRead = nLen;
	do {
		nRead = nToRead > sizeof(tmpBuf) ? sizeof(tmpBuf) : nToRead;

		nRead = fread(tmpBuf, 1, nRead, fp_input);
		fwrite(tmpBuf, 1, nRead, fp);
		nToRead -= nRead;
	} while (nRead > 0);

	fclose(fp);
	fclose(fp_input);

	strcpy(cmd,"mv ");
	strcat(cmd,WAPI_TMP_CERT);
	strcat(cmd," ");
	strcat(cmd,name);
	system(cmd);
//ccwei_flatfsd
#ifdef CONFIG_USER_FLATFSD_XXX
	strcpy(cmd, "flatfsd -s");
	system(cmd);
#endif
	/*strcpy(cmd, "ln -s ");
	strcat(cmd, name);
	strcat(cmd," ");
	strcat(cmd, lnname);
	system(cmd); */


	config_WLAN(ACT_RESTART);

	//fprintf(stderr, "%s(%d):cmd \"%s\"\n", __FUNCTION__,__LINE__,cmd);
	/*check if user or asu cerification*/
	strcpy(tmpBuf,multilang(LANG_CERIFICATION_INSTALL_SUCCESS));
	//OK_MSG1(tmpBuf, submitUrl);
	OK_MSG(submitUrl);
	return;
upload_ERR:
	ERR_MSG(tmpBuf);
}



void formUploadWapiCert1(request * wp, char * path, char * query)
{
	formUploadWapiCert(wp, path, query, WAPI_CA4AP_CERT_SAVE, "/wlwapiinstallcert.asp");
	wapi_cert_link_one(WAPI_CA4AP_CERT_SAVE, WAPI_CA4AP_CERT);
}

void formUploadWapiCert2(request * wp, char * path, char * query)
{
	formUploadWapiCert(wp, path, query, WAPI_AP_CERT_SAVE, "/wlwapiinstallcert.asp");
	wapi_cert_link_one(WAPI_AP_CERT_SAVE, WAPI_AP_CERT);
}

void formWapiReKey(request * wp, char * path, char * query)
{
	char *submitUrl, *strVal;
	char vChar;
	int vLong, ret;
	char tmpBuf[128];

	submitUrl = boaGetVar(wp, "submit-url", "");

	strVal = boaGetVar(wp, "REKEY_POLICY", "");
	//fprintf(stderr, "%s(%d): %s\n",__FUNCTION__,__LINE__, strVal);
	if (strVal[0]) {
		vChar=strVal[0]-'0';
		ret = mib_set(MIB_WLAN_WAPI_UCAST_REKETTYPE,(void *)&vChar);
		//fprintf(stderr, "%s(%d): %d, %d\n",__FUNCTION__,__LINE__, vChar, ret);
		if (vChar!=1) {
			strVal = boaGetVar(wp, "REKEY_TIME", "");
			if (strVal[0]) {
				vLong = atoi(strVal);
				ret = mib_set(MIB_WLAN_WAPI_UCAST_TIME,(void *)&vLong);
				//fprintf(stderr, "%s(%d): %s,%d\n",__FUNCTION__,__LINE__, strVal,ret);
			}
			strVal = boaGetVar(wp, "REKEY_PACKET", "");
			if (strVal[0]) {
				vLong = atoi(strVal);
				ret = mib_set(MIB_WLAN_WAPI_UCAST_PACKETS,(void *)&vLong);
				//fprintf(stderr, "%s(%d): %s,%d\n",__FUNCTION__,__LINE__, strVal,ret);
			}
		}
	}

	strVal = boaGetVar(wp, "REKEY_M_POLICY", "");
	if (strVal[0]) {
		vChar=strVal[0]-'0';
		mib_set(MIB_WLAN_WAPI_MCAST_REKEYTYPE,(void *)&vChar);

		if (vChar!=1) {
			strVal = boaGetVar(wp, "REKEY_M_TIME", "");
			if (strVal[0]) {
				vLong = atoi(strVal);
				mib_set(MIB_WLAN_WAPI_MCAST_TIME,(void *)&vLong);
			}

			strVal = boaGetVar(wp, "REKEY_M_PACKET", "");
			if (strVal[0]) {
				vLong = atoi(strVal);
				mib_set(MIB_WLAN_WAPI_MCAST_PACKETS,(void *)&vLong);
			}
		}
	}
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

	config_WLAN(ACT_RESTART);

	OK_MSG(submitUrl);
	return;

upload_ERR:
	ERR_MSG(tmpBuf);
}



#endif //CONFIG_RTL_WAPI_SUPPORT

//#define testWEP 1
#ifdef WLAN_WPA
/////////////////////////////////////////////////////////////////////////////
void formWlEncrypt(request * wp, char *path, char *query)
{
	char *submitUrl, *strEncrypt, *strVal;
	char vChar, *strKeyLen, *strFormat, *wepKey, *strAuth;
	char tmpBuf[100];
	WIFI_SECURITY_T encrypt;
	int enableRS=0, intVal, getPSK=0, len, keyLen;
	SUPP_NONWAP_T suppNonWPA;
	struct in_addr inIp;
	WEP_T wep;
	char key[30];
	AUTH_TYPE_T authType;
	MIB_CE_MBSSIB_T Entry;
	int i;
#ifdef WPS20
	unsigned char wpsUseVersion;
#endif
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
	strVal = boaGetVar(wp, "wlan_idx", "");
	if ( strVal[0] ) {
		printf("wlan_idx=%d\n", strVal[0]-'0');
		wlan_idx = strVal[0]-'0';
	}
#endif //CONFIG_RTL_92D_SUPPORT || WLAN_DUALBAND_CONCURRENT
#ifdef WPS20
	mib_get(MIB_WSC_VERSION, (void *) &wpsUseVersion);
#endif

	strVal = boaGetVar(wp, "wpaSSID", "");

	if (strVal[0]) {
		i = strVal[0]-'0';
		if (i<0 || i > NUM_VWLAN_INTERFACE) {
			strcpy(tmpBuf, strNotSuptSSIDType);
			goto setErr_encrypt;
		}

	} else {
		strcpy(tmpBuf, strNoSSIDTypeErr);
		goto setErr_encrypt;
	}

	if (!wlan_getEntry(&Entry, i)){
		strcpy(tmpBuf, strGetMBSSIBTBLErr);
		goto setErr_encrypt;
	}
	// Added by Mason Yu. End
	/*
	printf("Entry.idx=%d\n", Entry.idx);
	printf("Entry.encrypt=%d\n", Entry.encrypt);
	printf("Entry.enable1X=%d\n", Entry.enable1X);
	printf("Entry.wep=%d\n", Entry.wep);
	printf("Entry.wpaAuth=%d\n", Entry.wpaAuth);
	printf("Entry.wpaPSKFormat=%d\n", Entry.wpaPSKFormat);
	printf("Entry.wpaPSK=%s\n", Entry.wpaPSK);
	printf("Entry.rsPort=%d\n", Entry.rsPort);
	printf("Entry.rsIpAddr=0x%x\n", *((unsigned long *)Entry.rsIpAddr));
	printf("Entry.rsPassword=%s\n", Entry.rsPassword);
	*/

	strEncrypt = boaGetVar(wp, "security_method", "");
	if (!strEncrypt[0]) {
 		strcpy(tmpBuf, strNoEncryptionErr);
		goto setErr_encrypt;
	}

	encrypt = (WIFI_SECURITY_T) strEncrypt[0] - '0';
	vChar = (char)encrypt;
	Entry.encrypt = vChar;

	if (encrypt == WIFI_SEC_NONE || encrypt == WIFI_SEC_WEP) {

#ifdef WLAN_1x
		strVal = boaGetVar(wp, "use1x", "");
		if ( !gstrcmp(strVal, "ON")) {
			vChar = Entry.wlanMode;
			if (vChar) { // not AP mode
				strcpy(tmpBuf, strSet8021xWarning);
				goto setErr_encrypt;
			}
			vChar = 1;
			enableRS = 1;
		}
		else
			vChar = 0;		
		Entry.enable1X = vChar;
#endif

		if (encrypt == WIFI_SEC_WEP) {
	 		WEP_T wep;
			// Mason Yu. 201009_new_security. If wireless do not use 802.1x for wep mode. We should set wep key and Authentication type.
			if ( enableRS != 1 ) {
				// (1) Authentication Type
				strAuth = boaGetVar(wp, "auth_type", "");
				if (strAuth[0]) {
					if ( !gstrcmp(strAuth, "open"))
						authType = AUTH_OPEN;
					else if ( !gstrcmp(strAuth, "shared"))
						authType = AUTH_SHARED;
					else if ( !gstrcmp(strAuth, "both"))
						authType = AUTH_BOTH;
					else {
						strcpy(tmpBuf, strInvdAuthType);
						goto setErr_encrypt;
					}
					vChar = (char)authType;
					Entry.authType = vChar;
				}

				// (2) Key Length
				strKeyLen = boaGetVar(wp, "length0", "");
				if (!strKeyLen[0]) {
 					strcpy(tmpBuf, strKeyLenMustExist);
					goto setErr_encrypt;
				}
				if (strKeyLen[0]!='1' && strKeyLen[0]!='2') {
 					strcpy(tmpBuf, strInvdKeyLen);
					goto setErr_encrypt;
				}
				if (strKeyLen[0] == '1')
					wep = WEP64;
				else
					wep = WEP128;

				vChar = (char)wep;
				Entry.wep = vChar;

				// (3) Key Format
				strFormat = boaGetVar(wp, "format0", "");
				if (!strFormat[0]) {
 					strcpy(tmpBuf, strKeyTypeMustExist);
					goto setErr_encrypt;
				}

				if (strFormat[0]!='1' && strFormat[0]!='2') {
					strcpy(tmpBuf, strInvdKeyType);
					goto setErr_encrypt;
				}

				vChar = (char)(strFormat[0] - '0' - 1);
				Entry.wepKeyType = vChar;

				if (wep == WEP64) {
					if (strFormat[0]=='1')
						keyLen = WEP64_KEY_LEN;
					else
						keyLen = WEP64_KEY_LEN*2;
				}
				else {
					if (strFormat[0]=='1')
						keyLen = WEP128_KEY_LEN;
					else
						keyLen = WEP128_KEY_LEN*2;
				}

				// (4) Encryption Key
				wepKey = boaGetVar(wp, "key0", "");
				if  (wepKey[0]) {
					if (strlen(wepKey) != keyLen) {
						strcpy(tmpBuf, strInvdKey1Len);
						goto setErr_encrypt;
					}
					if ( !isAllStar(wepKey) ) {
						if (strFormat[0] == '1') // ascii
							strcpy(key, wepKey);
						else { // hex
							if ( !string_to_hex(wepKey, key, keyLen)) {
				   				strcpy(tmpBuf, strInvdWEPKey1);
								goto setErr_encrypt;
							}
						}
						if (wep == WEP64)
							memcpy(Entry.wep64Key1,key,WEP64_KEY_LEN);
						else
							memcpy(Entry.wep128Key1,key,WEP128_KEY_LEN);
					}
				}// (4) Encryption Key


			}
			#ifdef CONFIG_WIFI_SIMPLE_CONFIG
			#ifdef WPS20
			if (0 == i && wpsUseVersion != 0)
				Entry.wsc_disabled = 1;
			#endif
			#endif
		}

		Entry.unicastCipher = 0;
		Entry.wpa2UnicastCipher = 0;
		
	}
#ifdef CONFIG_RTL_WAPI_SUPPORT
	/* assume MBSSID for now. */
	else if (encrypt == WIFI_SEC_WAPI) {
		char *wpaAuth=0, *pskFormat=0, *pskValue=0;
		char *asIP=0, wapiType=0;
		int len;
		//fprintf(stderr, "%s(%d):\n", __FUNCTION__,__LINE__);
		wpaAuth = boaGetVar(wp, "wpaAuth", "");
		if (wpaAuth && !gstrcmp(wpaAuth, "eap")) {
			wapiType = 1;
			asIP = boaGetVar(wp, "radiusIP", "");
			//fprintf(stderr, "%s(%d): %p\n", __FUNCTION__,__LINE__, asIP);
		}
		else if (wpaAuth && !gstrcmp(wpaAuth, "psk")) {
			wapiType = 2;
			pskFormat = boaGetVar(wp, "pskFormat", "");
			pskValue = boaGetVar(wp, "pskValue", "");
			len = strlen(pskValue);
		}
		else {
			strcpy(tmpBuf, strInvdWPAAuthValue);
			goto setErr_encrypt;
		}

		//fprintf(stderr, "%s(%d):\n", __FUNCTION__,__LINE__);
			Entry.wapiAuth = wapiType;
			if (wapiType == 2) { // PSK
				vChar = pskFormat[0] - '0';
				if (vChar != 0 && vChar != 1) {
					strcpy(tmpBuf, strInvdPSKFormat);
					goto setErr_encrypt;
				}
				Entry.wapiPskFormat = vChar;//mib_set(MIB_WLAN_WAPI_PSK_FORMAT, (void *)&vChar);
				if (vChar == 1) {// hex
					if (!string_to_hex(pskValue, tmpBuf, MAX_PSK_LEN)) {
						strcpy(tmpBuf, strInvdPSKValue);
						goto setErr_encrypt;
					}
					if ((len & 1) || (len/2 >= MAX_PSK_LEN)) {
						strcpy(tmpBuf, strInvdPSKValue);
						goto setErr_encrypt;
					}
					len = len / 2;
					vChar = len;
					Entry.wapiPskLen = vChar;//mib_set(MIB_WLAN_WAPI_PSKLEN, &vChar);
					strcpy(Entry.wapiPsk, tmpBuf);//mib_set(MIB_WLAN_WAPI_PSK, (void *)tmpBuf);

				} else { // passphrase
					if (len==0 || len > (MAX_PSK_LEN - 1)) {
						strcpy(tmpBuf, strInvdPSKValue);
						goto setErr_encrypt;
					}
					vChar = len;
					Entry.wapiPskLen = vChar;//mib_set(MIB_WLAN_WAPI_PSKLEN, &vChar);
					strcpy(Entry.wapiPsk, pskValue);//mib_set(MIB_WLAN_WAPI_PSK, (void *)pskValue);

				}

			} else { // AS
				if ( !inet_aton(asIP, &inIp) ) {
					strcpy(tmpBuf, strInvdRSIPValue);
					goto setErr_encrypt;
				}
				*((unsigned long *)Entry.wapiAsIpAddr) = inIp.s_addr;
			}

	}
#endif 	// CONFIG_RTL_WAPI_SUPPORT
	else {	// WPA
#ifdef WPS20
		unsigned char disableWps = 0;
#endif //WPS20
#ifdef WLAN_1x
		// WPA authentication
		vChar = 0;
		Entry.enable1X = vChar;
		
		strVal = boaGetVar(wp, "wpaAuth", "");
		if (strVal[0]) {
			if ( !gstrcmp(strVal, "eap")) {
				vChar = Entry.wlanMode;
				if (vChar == CLIENT_MODE) { //client mode
					strcpy(tmpBuf, strSetWPARADIUSWarn);
					goto setErr_encrypt;
				}
				vChar = WPA_AUTH_AUTO;
				enableRS = 1;
			}
			else if ( !gstrcmp(strVal, "psk")) {
				vChar = WPA_AUTH_PSK;
				getPSK = 1;
			}
			else {
				strcpy(tmpBuf, strInvdWPAAuthValue);
				goto setErr_encrypt;
			}
			Entry.wpaAuth = vChar;
		}
#endif

		// Mason Yu. 201009_new_security. Set ciphersuite(wpa_cipher) for wpa/wpa mixed
		if ((encrypt == WIFI_SEC_WPA) || (encrypt == WIFI_SEC_WPA2_MIXED)) {
			unsigned char intVal = 0;
			unsigned char val2;
			strVal = boaGetVar(wp, "ciphersuite_t", "");
			if(strVal[0]){
				if (strVal[0]=='1')
					intVal |= WPA_CIPHER_TKIP;
				strVal = boaGetVar(wp, "ciphersuite_a", "");
				if (strVal[0]=='1')
					intVal |= WPA_CIPHER_AES;

				if ( intVal == 0 )
					intVal = WPA_CIPHER_TKIP;
			}
			else if(encrypt == WIFI_SEC_WPA2_MIXED){
				intVal = WPA_CIPHER_MIXED;
			}
			Entry.unicastCipher = intVal;

			if(i==0){
				#ifdef WPS20
				if ((wpsUseVersion != 0) && ((encrypt == WIFI_SEC_WPA) ||
					(encrypt == WIFI_SEC_WPA2_MIXED && intVal == WPA_CIPHER_TKIP))) {	//disable wps if wpa only or tkip only
					disableWps = 1;
				}
				#endif
			}
		}
		else
			Entry.unicastCipher = 0;

		// Mason Yu. 201009_new_security. Set wpa2ciphersuite(wpa2_cipher) for wpa2/wpa mixed
		if ((encrypt == WIFI_SEC_WPA2) || (encrypt == WIFI_SEC_WPA2_MIXED)) {
			unsigned char intVal = 0;
			strVal = boaGetVar(wp, "wpa2ciphersuite_t", "");
			if (strVal[0]){
				if (strVal[0]=='1')
					intVal |= WPA_CIPHER_TKIP;
				strVal = boaGetVar(wp, "wpa2ciphersuite_a", "");
				if (strVal[0]=='1')
					intVal |= WPA_CIPHER_AES;

				if ( intVal == 0 )
					intVal = WPA_CIPHER_AES;
		
			}
			else if(encrypt == WIFI_SEC_WPA2_MIXED){
				intVal = WPA_CIPHER_MIXED;
			}
			else if(encrypt == WIFI_SEC_WPA2){
				intVal = WPA_CIPHER_AES;
			}
			
			Entry.wpa2UnicastCipher = intVal;
			
			if(i==0){
				#ifdef WPS20
				if (wpsUseVersion != 0) {
					if (encrypt == WIFI_SEC_WPA2) {
						if (intVal == WPA_CIPHER_TKIP)
							disableWps = 1;
					}
					else { // mixed
						if (intVal == WPA_CIPHER_TKIP && disableWps)	//disable wps if wpa2 mixed + tkip only
							disableWps = 1;
						else
							disableWps = 0;
					}
				}
				#endif //WPS20
			}
		}
		else
			Entry.wpa2UnicastCipher = 0;
		
		#ifdef CONFIG_WIFI_SIMPLE_CONFIG
		#ifdef WPS20
		if (disableWps)
			Entry.wsc_disabled = 1;
		#endif //WPS20
		#endif
#ifdef WLAN_11W
		if (encrypt == WIFI_SEC_WPA2){
			strVal = boaGetVar(wp, "dotIEEE80211W", "");
			if (!strVal[0])
				goto setErr_encrypt;
			Entry.dotIEEE80211W = strVal[0] - '0';
			
			if(Entry.dotIEEE80211W == 1){ // 1: MGMT_FRAME_PROTECTION_OPTIONAL
				strVal = boaGetVar(wp, "sha256", "");
				if (!strVal[0])
					goto setErr_encrypt;
				Entry.sha256 = strVal[0] - '0';
			}
			else if(Entry.dotIEEE80211W == 0) // 0: NO_MGMT_FRAME_PROTECTION
				Entry.sha256 = 0;
			else if (Entry.dotIEEE80211W == 2)  // 2: MGMT_FRAME_PROTECTION_REQUIRED 
				Entry.sha256 = 1;
			else
				goto setErr_encrypt;
				
		}
#endif
		// pre-shared key
		if ( getPSK ) {
			strVal = boaGetVar(wp, "pskFormat", "");
			if (!strVal[0]) {
	 			strcpy(tmpBuf, strNoPSKFormat);
				goto setErr_encrypt;
			}
			vChar = strVal[0] - '0';
			if (vChar != 0 && vChar != 1) {
	 			strcpy(tmpBuf, strInvdPSKFormat);
				goto setErr_encrypt;
			}
			strVal = boaGetVar(wp, "pskValue", "");
			len = strlen(strVal);
			Entry.wpaPSKFormat = vChar;
			
			if (vChar==1) { // hex
				if (len!=MAX_PSK_LEN || !string_to_hex(strVal, tmpBuf, MAX_PSK_LEN)) {
	 				strcpy(tmpBuf, strInvdPSKValue);
					goto setErr_encrypt;
				}
			}
			else { // passphras
				if (len==0 || len > (MAX_PSK_LEN - 1) ) {
	 				strcpy(tmpBuf, strInvdPSKValue);
					goto setErr_encrypt;
				}
			}
			strcpy(Entry.wpaPSK, strVal);
		}

		strVal = boaGetVar(wp, "gk_rekey", "");
		if (strVal[0]) {
			int val;
			if ( !string_to_dec(strVal, &val)) {
				strcpy(tmpBuf, strSetREKEYTIMEErr);
				goto setErr_encrypt;
			}
			Entry.wpaGroupRekeyTime = (unsigned long)val;
		}
	}
#ifdef WLAN_1x
	if (enableRS == 1) { // if 1x enabled, get RADIUS server info
		unsigned short uShort;

		strVal = boaGetVar(wp, "radiusPort", "");
		if (!strVal[0]) {
			strcpy(tmpBuf, multilang(LANG_NO_RS_PORT_NUMBER));
			goto setErr_encrypt;
		}
		if (!string_to_dec(strVal, &intVal) || intVal<=0 || intVal>65535) {
			strcpy(tmpBuf, strInvdRSPortNum);
			goto setErr_encrypt;
		}
		uShort = (unsigned short)intVal;
		Entry.rsPort = uShort;

		strVal = boaGetVar(wp, "radiusIP", "");
		if (!strVal[0]) {
			strcpy(tmpBuf, strNoIPAddr);
			goto setErr_encrypt;
		}
		if ( !inet_aton(strVal, &inIp) ) {
			strcpy(tmpBuf, strInvdRSIPValue);
			goto setErr_encrypt;
		}
		*((unsigned long *)Entry.rsIpAddr) = inIp.s_addr;

		strVal = boaGetVar(wp, "radiusPass", "");
		if (strlen(strVal) > (MAX_PSK_LEN) ) {
			strcpy(tmpBuf, strRSPwdTooLong);
			goto setErr_encrypt;
		}
		strcpy(Entry.rsPassword, strVal);

		strVal = boaGetVar(wp, "radiusRetry", "");
		if (strVal[0]) {
			if ( !string_to_dec(strVal, &intVal) ) {
				strcpy(tmpBuf, strInvdRSRetry);
				goto setErr_encrypt;
			}
			vChar = (char)intVal;
			if ( !mib_set(MIB_WLAN_RS_RETRY, (void *)&vChar)) {
				strcpy(tmpBuf, strSetRSRETRYErr);
				goto setErr_encrypt;
			}
		}
		
		strVal = boaGetVar(wp, "radiusTime", "");
		if (strVal[0]) {
			if ( !string_to_dec(strVal, &intVal) ) {
				strcpy(tmpBuf, strInvdRSTime);
				goto setErr_encrypt;
			}
			uShort = (unsigned short)intVal;
			if ( !mib_set(MIB_WLAN_RS_INTERVAL_TIME, (void *)&uShort)) {
				strcpy(tmpBuf, strSetRSINTVLTIMEErr);
				goto setErr_encrypt;
			}
		}

get_wepkey:
		// get 802.1x WEP key length
		strVal = boaGetVar(wp, "wepKeyLen", "");
		if (strVal[0]) {
			if ( !gstrcmp(strVal, "wep64"))
				vChar = WEP64;
			else if ( !gstrcmp(strVal, "wep128"))
				vChar = WEP128;
			else {
				strcpy(tmpBuf, strInvdWepKeyLen);
				goto setErr_encrypt;
			}
			Entry.wep = vChar;
		}
	}
#endif

	wlan_setEntry(&Entry,i);

	sleep(5);
	/*
	if (!wlan_getEntry(&Entry,i)) {
 		strcpy(tmpBuf, strGetMBSSIBTBLUpdtErr);
		goto setErr_encrypt;
	}
	
	printf("MIB_MBSSIB_TBL updated\n");
	printf("Entry.idx=%d\n", Entry.idx);
	printf("Entry.encrypt=%d\n", Entry.encrypt);
	printf("Entry.enable1X=%d\n", Entry.enable1X);
	printf("Entry.wep=%d\n", Entry.wep);
	printf("Entry.wpaAuth=%d\n", Entry.wpaAuth);
	printf("Entry.wpaPSKFormat=%d\n", Entry.wpaPSKFormat);
	printf("Entry.wpaPSK=%s\n", Entry.wpaPSK);
	printf("Entry.rsPort=%d\n", Entry.rsPort);
	printf("Entry.rsIpAddr=0x%x\n", *((unsigned long *)Entry.rsIpAddr));
	printf("Entry.rsPassword=%s\n", Entry.rsPassword);
	*/

set_OK:
#ifndef NO_ACTION
	run_script(-1);
#endif
#ifdef CONFIG_WIFI_SIMPLE_CONFIG //WPS
	//fprintf(stderr, "WPA WPS Configure\n");
	strVal = boaGetVar(wp, "wps_clear_configure_by_reg0", "");
	intVal = 0;
	if (strVal && strVal[0])
		intVal = atoi(strVal);
	update_wps_configured(intVal);
#endif

// Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif // of #if COMMIT_IMMEDIATELY
	config_WLAN(ACT_RESTART);

	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page
	OK_MSG(submitUrl);

	return;

setErr_encrypt:
	ERR_MSG(tmpBuf);
}
#endif // WLAN_WPA

#ifdef WLAN_ACL
/////////////////////////////////////////////////////////////////////////////
int wlAcList(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0, entryNum, i;
	MIB_CE_WLAN_AC_T Entry;
	char tmpBuf[100];
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
	char *strVal;
	strVal = boaGetVar(wp, "wlan_idx", "");
	if ( strVal[0] ) {
		printf("wlan_idx=%d\n", strVal[0]-'0');
		wlan_idx = strVal[0]-'0';
	}
#endif //CONFIG_RTL_92D_SUPPORT || WLAN_DUALBAND_CONCURRENT

	entryNum = mib_chain_total(MIB_WLAN_AC_TBL);

	nBytesSent += boaWrite(wp, "<tr>"
      	"<td align=center width=\"45%%\" bgcolor=\"#808080\"><font size=\"2\"><b>%s</b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>%s</b></font></td></tr>\n", multilang(LANG_MAC_ADDRESS), multilang(LANG_SELECT));
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_WLAN_AC_TBL, i, (void *)&Entry))
		{
  			boaError(wp, 400, "Get chain record error!\n");
			return -1;
		}
		if(Entry.wlanIdx != wlan_idx)
			continue;
		snprintf(tmpBuf, 100, "%02x:%02x:%02x:%02x:%02x:%02x",
			Entry.macAddr[0], Entry.macAddr[1], Entry.macAddr[2],
			Entry.macAddr[3], Entry.macAddr[4], Entry.macAddr[5]);

		nBytesSent += boaWrite(wp, "<tr>"
			"<td align=center width=\"45%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
       			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td></tr>\n",
				tmpBuf, i);
	}
	return nBytesSent;
}

/////////////////////////////////////////////////////////////////////////////
void formWlAc(request * wp, char *path, char *query)
{
	char *strAddMac, *strDelMac, *strDelAllMac, *strVal, *submitUrl, *strEnabled;
	char tmpBuf[100];
	char vChar;
	int entryNum, i, enabled;
	MIB_CE_WLAN_AC_T macEntry;
	MIB_CE_WLAN_AC_T Entry;
//xl_yue
	char * strSetMode;
	strSetMode = boaGetVar(wp, "setFilterMode", "");
	strAddMac = boaGetVar(wp, "addFilterMac", "");
	strDelMac = boaGetVar(wp, "deleteSelFilterMac", "");
	strDelAllMac = boaGetVar(wp, "deleteAllFilterMac", "");
	strEnabled = boaGetVar(wp, "wlanAcEnabled", "");
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
	strVal = boaGetVar(wp, "wlan_idx", "");
	if ( strVal[0] ) {
		printf("wlan_idx=%d\n", strVal[0]-'0');
		wlan_idx = strVal[0]-'0';
	}
#endif //CONFIG_RTL_92D_SUPPORT || WLAN_DUALBAND_CONCURRENT


//xl_yue: access control mode is set independently from adding MAC for 531b
	if (strSetMode[0]) {
		vChar = strEnabled[0] - '0';

		if ( mib_set( MIB_WLAN_AC_ENABLED, (void *)&vChar) == 0) {
  			strcpy(tmpBuf, strEnabAccCtlErr);
			goto setErr_ac;
		}
		goto setac_ret;
	}

	if (strAddMac[0]) {
		int intVal;
		/*
		if ( !gstrcmp(strEnabled, "ON"))
			vChar = 1;
		else
			vChar = 0;
		*/
		strVal = boaGetVar(wp, "mac", "");
		if ( !strVal[0] ) {
//			strcpy(tmpBuf, "Error! No mac address to set.");
			goto setac_ret;
		}

		if (strlen(strVal)!=12 || !string_to_hex(strVal, macEntry.macAddr, 12)) {
			strcpy(tmpBuf, strInvdMACAddr);
			goto setErr_ac;
		}
		if (!isValidMacAddr(macEntry.macAddr)) {
			strcpy(tmpBuf, strInvdMACAddr);
			goto setErr_ac;
		}
		macEntry.wlanIdx = wlan_idx;
/*
		strVal = boaGetVar(wp, "comment", "");
		if ( strVal[0] ) {
			if (strlen(strVal) > COMMENT_LEN-1) {
				strcpy(tmpBuf, "Error! Comment length too long.");
				goto setErr_ac;
			}
			strcpy(macEntry.comment, strVal);
		}
		else
			macEntry.comment[0] = '\0';
*/

		entryNum = mib_chain_total(MIB_WLAN_AC_TBL);
		if ( entryNum >= MAX_WLAN_AC_NUM ) {
			strcpy(tmpBuf, strAddAcErrForFull);
			goto setErr_ac;
		}

		// set to MIB. Check if entry exists
		for (i=0; i<entryNum; i++) {
			if (!mib_chain_get(MIB_WLAN_AC_TBL, i, (void *)&Entry))
			{
	  			strcpy(tmpBuf, strGetChainerror);
				goto setErr_ac;
			}
			if(Entry.wlanIdx != macEntry.wlanIdx)
				continue;
			if (!memcmp(macEntry.macAddr, Entry.macAddr, 6))
			{
				strcpy(tmpBuf, strMACInList);
				goto setErr_ac;
			}
		}

		intVal = mib_chain_add(MIB_WLAN_AC_TBL, (unsigned char*)&macEntry);
		if (intVal == 0) {
			strcpy(tmpBuf, strAddListErr);
			goto setErr_ac;
		}
		else if (intVal == -1) {
			strcpy(tmpBuf, strTableFull);
			goto setErr_ac;
		}
	}

	/* Delete entry */
	if (strDelMac[0]) {
		unsigned int deleted = 0;
		entryNum = mib_chain_total(MIB_WLAN_AC_TBL);
		for (i=entryNum; i>0; i--) {
			if (!mib_chain_get(MIB_WLAN_AC_TBL, i-1, (void *)&Entry))
				break;
			if(Entry.wlanIdx != wlan_idx)
				continue;
			snprintf(tmpBuf, 20, "select%d", i-1);
			strVal = boaGetVar(wp, tmpBuf, "");

			if ( !gstrcmp(strVal, "ON") ) {

				deleted ++;
				if(mib_chain_delete(MIB_WLAN_AC_TBL, i-1) != 1) {
					strcpy(tmpBuf, strDelListErr);
					goto setErr_ac;
				}
			}
		}
		if (deleted <= 0) {
			strcpy(tmpBuf, multilang(LANG_THERE_IS_NO_ITEM_SELECTED_TO_DELETE));
			goto setErr_ac;
		}
	}

	/* Delete all entry */
	if ( strDelAllMac[0]) {
		//mib_chain_clear(MIB_WLAN_AC_TBL); /* clear chain record */
		entryNum = mib_chain_total(MIB_WLAN_AC_TBL);
		for (i=entryNum; i>0; i--) {
			if (!mib_chain_get(MIB_WLAN_AC_TBL, i-1, (void *)&Entry)) {
	  			strcpy(tmpBuf, strGetChainerror);
				goto setErr_ac;
			}
			if(Entry.wlanIdx == wlan_idx) {
				if(mib_chain_delete(MIB_WLAN_AC_TBL, i-1) != 1) {
					strcpy(tmpBuf, strDelListErr);
					goto setErr_ac;
				}
			}
		}
	}

setac_ret:
	config_WLAN(ACT_RESTART);
// Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif // of #if COMMIT_IMMEDIATELY

#ifndef NO_ACTION
	run_script(-1);
#endif

	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page
	OK_MSG( submitUrl );
  	return;

setErr_ac:
	ERR_MSG(tmpBuf);
}
#endif

// Added by Mason Yu
#ifdef WLAN_MBSSID
#ifdef CONFIG_RTL_WAPI_SUPPORT
void wapi_mod_entry(MIB_CE_MBSSIB_T *Entry, char *strbuf, char *strbuf2) {
	int len;

	Entry->wpaPSKFormat = Entry->wapiPskFormat;
	Entry->wep = 0;
	Entry->enable1X = 0;
	Entry->rsPort = 0;
	Entry->rsPassword[0] = 0;

	if (Entry->wapiAuth==1) {// AS
		Entry->wpaAuth = 1;

		if ( ((struct in_addr *)Entry->wapiAsIpAddr)->s_addr == INADDR_NONE ) {
			sprintf(strbuf2, "%s", "");
		} else {
			sprintf(strbuf2, "%s", inet_ntoa(*((struct in_addr *)Entry->wapiAsIpAddr)));
		}


	} else { //PSK
		Entry->wpaAuth = 2;

		for (len=0; len<Entry->wapiPskLen; len++)
			strbuf[len]='*';
		strbuf[len]='\0';
	}
}
#endif

void formWlanMultipleAP(request * wp, char *path, char *query)
{
	char *str, *submitUrl;
	MIB_CE_MBSSIB_T Entry;
	char tmpBuf[100], en_vap[256];
	int i;
	unsigned int val;
#ifndef NO_ACTION
	int pid;
#endif
	unsigned char vChar;

#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
	str = boaGetVar(wp, "wlan_idx", "");
	if ( str[0] ) {
		printf("wlan_idx=%d\n", str[0]-'0');
		wlan_idx = str[0]-'0';
	}
#endif //CONFIG_RTL_92D_SUPPORT || WLAN_DUALBAND_CONCURRENT

	//for blocking between MBSSID
	sprintf(en_vap, "mbssid_block");
	str = boaGetVar(wp, en_vap, "");
	if (str[0]) {
		if ( !gstrcmp(str, "disable"))
			vChar = 0;
		else
			vChar = 1;

		if ( mib_set(MIB_WLAN_BLOCK_MBSSID, (void *)&vChar) == 0) {
			sprintf(tmpBuf, "%s:%s", Tset_mib_error, MIB_WLAN_BLOCK_MBSSID);
			goto setErr_mbssid;
		}
	}

	for (i = 1; i <= NUM_VWLAN_INTERFACE; i ++) {
		if (!mib_chain_get(MIB_MBSSIB_TBL, i, (void *)&Entry)) {
			strcpy(tmpBuf, strGetMULTIAPTBLErr);
			goto setErr_mbssid;
		}

		sprintf(en_vap, "wl_disable%d", i);
		str = boaGetVar(wp, en_vap, "");
		if (str && !strcmp(str, "ON")) {	// enable
			Entry.wlanDisabled = 0;
		}
		else {	// disable
			Entry.wlanDisabled = 1;
			mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, i);
			continue;
		}

		sprintf(en_vap, "wl_band%d", i);
		str = boaGetVar(wp, en_vap, "");
		if (str) {
			vChar = atoi(str);
			vChar ++;

			Entry.wlanBand= vChar;
			update_on_band_changed(&Entry, i, Entry.wlanBand);
		}

		sprintf(en_vap, "wl_ssid%d", i);
		str = boaGetVar(wp, en_vap, "");
		if (str) {
#ifdef	CONFIG_USER_FON
		if(i == WLAN_MBSSID_NUM){
			strcpy(Entry.ssid, "FON_");
			strcat(Entry.ssid, str);
		}
		else
#endif
			strcpy(Entry.ssid, str);
		}

		sprintf(en_vap, "TxRate%d", i);
		str = boaGetVar(wp, en_vap, "");
		if (str) {
			if (str[0] == '0') { // auto
				vChar = 1;
				Entry.rateAdaptiveEnabled = vChar;
			}
			else {
				vChar = 0;
				Entry.rateAdaptiveEnabled = vChar;
				val = atoi(str);
				if (val < 30)
					val = 1 << (val - 1);
				else
					val = ((1 << 31) + (val - 30));
				Entry.fixedTxRate = val;
			}
		}

		sprintf(en_vap, "wl_hide_ssid%d", i);
		str = boaGetVar(wp, en_vap, "");
		if (str) {
			if (str[0] == '0')
				vChar = 0;
			else		// '1'
				vChar = 1;
			Entry.hidessid = vChar;
		}

		sprintf(en_vap, "wl_wmm_capable%d", i);
		str = boaGetVar(wp, en_vap, "");
		if (str) {
			if (str[0] == '0')
				vChar = 0;
			else		// '1'
				vChar = 1;
			Entry.wmmEnabled = vChar;
		}

		sprintf(en_vap, "wl_access%d", i);
		str = boaGetVar(wp, en_vap, "");
		if (str) {
			if (str[0] == '0')
				vChar = 0;
			else		// '1'
				vChar = 1;
			Entry.userisolation = vChar;
		}
#ifdef WLAN_LIMITED_STA_NUM
		sprintf(en_vap, "wl_limitstanum%d", i);
		str = boaGetVar(wp, en_vap, "");
		if(str){
			if(str[0] == '0')
				Entry.stanum = 0;
			else{
				sprintf(en_vap, "wl_stanum%d", i);
				str = boaGetVar(wp, en_vap, "");
				Entry.stanum = atoi(str);
			}
		}
#endif
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, i);
	}

	config_WLAN(ACT_RESTART);

#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif // of #if COMMIT_IMMEDIATELY

#ifndef NO_ACTION
	pid = fork();
	if (pid)
		waitpid(pid, NULL, 0);
	else if (pid == 0) {
		snprintf(tmpBuf, 100, "%s/%s", _CONFIG_SCRIPT_PATH, _CONFIG_SCRIPT_PROG);
#ifdef HOME_GATEWAY
		execl( tmpBuf, _CONFIG_SCRIPT_PROG, "gw", "bridge", NULL);
#else
		execl( tmpBuf, _CONFIG_SCRIPT_PROG, "ap", "bridge", NULL);
#endif
		exit(1);
	}
#endif

	submitUrl = boaGetVar(wp, "submit-url", "");
	OK_MSG(submitUrl);
  	return;

setErr_mbssid:
	ERR_MSG(tmpBuf);
}

int SSIDStr(int eid, request * wp, int argc, char **argv)
{
	char *name;
	MIB_CE_MBSSIB_T Entry;
	//char strbuf[20], strbuf2[20];
	//int len;
	int i;
	char ssid[200];
#ifdef CONFIG_USER_FON
	unsigned char FON_SSID[200];
	char *pch;
	int j = 0;
#endif

	if (boaArgs(argc, argv, "%s", &name) < 1) {
		boaError(wp, 400, "Insufficient args\n");
		return -1;
	}

	if ( !strcmp(name, "vap0") ){
		i = 1;

	}
	else if ( !strcmp(name, "vap1") ) {
		i = 2;

	}
	else if ( !strcmp(name, "vap2") ) {
		i = 3;
	}
	else if ( !strcmp(name, "vap3") ) {
		i = 4;
	}
	else {
		printf("SSIDStr: Not support this VAP!\n");
		return 1;
	}
	{
		if (!mib_chain_get(MIB_MBSSIB_TBL, i, (void *)&Entry)) {
	  		printf("Error! Get MIB_MBSSIB_TBL(SSIDStr) error.\n");
  			return 1;
		}
	}
#ifdef CONFIG_USER_FON
	if(i == WLAN_MBSSID_NUM){
		strncpy(FON_SSID, Entry.ssid+4, strlen(Entry.ssid)); 
		translate_control_code(FON_SSID);
		boaWrite(wp, "%s", FON_SSID);
	}
	else
#endif
	{
		strncpy(ssid, Entry.ssid, MAX_SSID_LEN);
		translate_control_code(ssid);
		boaWrite(wp, "%s", ssid);
	}

}
#endif

/////////////////////////////////////////////////////////////////////////////
//check wlan status and set checkbox
int wlanStatus(int eid, request * wp, int argc, char **argv)
{
#ifdef WLAN_SUPPORT
	if (wlan_is_up())
	    //boaWrite(wp, "\"OFF\" enabled");
	    boaWrite(wp, "\"OFF\"");
	else
	    //boaWrite(wp, "\"ON\" checked disabled");
	    boaWrite(wp, "\"ON\" checked");
	return 0;
#endif
}

#ifdef WLAN_SUPPORT
int wlan_ssid_select(int eid, request * wp, int argc, char **argv)
{
	MIB_CE_MBSSIB_T Entry;
	WLAN_MODE_T root_mode;
	int len, i, k;
	char repeater_AP[]="Repeater AP";
	char repeater_Client[]="Repeater Client";
	char *pstr;
	char ssid[200];

	wlan_getEntry(&Entry, 0);
	strncpy(ssid, Entry.ssid, MAX_SSID_LEN);
	translate_control_code(ssid);
	root_mode = (WLAN_MODE_T)Entry.wlanMode;
	k=0;
	if (root_mode!=CLIENT_MODE) {
		/*--------------------- Root AP ----------------------------*/
		if (!Entry.wlanDisabled)
			boaWrite(wp, "<option value=0>Root AP - %s</option>\n", ssid);
		#ifdef WLAN_MBSSID
		/*----------------------- VAP ------------------------------*/
		for (i=0; i<WLAN_MBSSID_NUM; i++) {
		#ifdef CONFIG_USER_FON
			if(i == WLAN_MBSSID_NUM-1) continue;
		#endif
			wlan_getEntry(&Entry, WLAN_VAP_ITF_INDEX+i);
			strncpy(ssid, Entry.ssid, MAX_SSID_LEN);
			translate_control_code(ssid);
			if (!Entry.wlanDisabled) {
				boaWrite(wp, "\t\t<option value=%d>AP%d - %s\n", WLAN_VAP_ITF_INDEX + i, i + 1, ssid);
			}
		}
		#endif
	}
	//else { // client mode
	//	/*--------------------- Root Client ----------------------------*/
	//	boaWrite(wp, "<option value=0>Root Client - %s</option>\n", Entry.ssid);
	//}
	#ifdef WLAN_UNIVERSAL_REPEATER
	wlan_getEntry(&Entry, WLAN_REPEATER_ITF_INDEX);
	strncpy(ssid, Entry.ssid, MAX_SSID_LEN);
	translate_control_code(ssid);
	if (!Entry.wlanDisabled && (root_mode != WDS_MODE)) {
		if (root_mode == CLIENT_MODE){
			pstr = repeater_AP;
		//else
		//	pstr = repeater_Client;

		boaWrite(wp, "\t\t<option value=%d>%s - %s</option>\n", WLAN_REPEATER_ITF_INDEX, pstr, ssid);
		}
	}
	#endif
}
#endif

/////////////////////////////////////////////////////////////////////////////
void formAdvanceSetup(request * wp, char *path, char *query)
{
	char *submitUrl, *strAuth, *strFragTh, *strRtsTh, *strBeacon, *strPreamble;
	char *strRate, *strHiddenSSID, *strDtim, *strIapp, *strBlock;
	char *strProtection, *strAggregation, *strShortGIO, *strVal;
	char vChar;
	unsigned short uShort;
	AUTH_TYPE_T authType;
	PREAMBLE_T preamble;
	int val;
	char tmpBuf[100];
#ifdef WPS20
	unsigned char wpsUseVersion;
#endif
	MIB_CE_MBSSIB_T Entry;

#ifdef WPS20
	mib_get(MIB_WSC_VERSION, (void *) &wpsUseVersion);
#endif
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
	strVal = boaGetVar(wp, "wlan_idx", "");
	if ( strVal[0] ) {
		printf("wlan_idx=%d\n", strVal[0]-'0');
		wlan_idx = strVal[0]-'0';
	}
#endif //CONFIG_RTL_92D_SUPPORT || WLAN_DUALBAND_CONCURRENT
//xl_yue: translocate to basic_setting   for ZTE531B BRIDGE
	mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry);

	strFragTh = boaGetVar(wp, "fragThreshold", "");
	if (strFragTh[0]) {
		if ( !string_to_dec(strFragTh, &val) || val<256 || val>2346) {
			strcpy(tmpBuf, strFragThreshold);
			goto setErr_advance;
		}
		uShort = (unsigned short)val;
		if ( mib_set(MIB_WLAN_FRAG_THRESHOLD, (void *)&uShort) == 0) {
			strcpy(tmpBuf, strSetFragThreErr);
			goto setErr_advance;
		}
	}
	strRtsTh = boaGetVar(wp, "rtsThreshold", "");
	if (strRtsTh[0]) {
		if ( !string_to_dec(strRtsTh, &val) || val<0 || val>2347) {
			strcpy(tmpBuf, strRTSThreshold);
			goto setErr_advance;
		}
		uShort = (unsigned short)val;
		if ( mib_set(MIB_WLAN_RTS_THRESHOLD, (void *)&uShort) == 0) {
			strcpy(tmpBuf, strSetRTSThreErr);
			goto setErr_advance;
		}
	}

	strBeacon = boaGetVar(wp, "beaconInterval", "");
	if (strBeacon[0]) {
		if ( !string_to_dec(strBeacon, &val) || val<20 || val>1024) {
			strcpy(tmpBuf, strInvdBeaconIntv);
			goto setErr_advance;
		}
		uShort = (unsigned short)val;
		if ( mib_set(MIB_WLAN_BEACON_INTERVAL, (void *)&uShort) == 0) {
			strcpy(tmpBuf, strSetBeaconIntvErr);
			goto setErr_advance;
		}
	}

//xl_yue: translocate to basic_setting  for ZTE531B BRIDGE
	// set tx rate
	strRate = boaGetVar(wp, "txRate", "");
	if ( strRate[0] ) {
		if ( strRate[0] == '0' )  // auto
			Entry.rateAdaptiveEnabled = 1;
		else  {
			Entry.rateAdaptiveEnabled = 0;

#ifdef WLAN_QTN
#ifdef WLAN1_QTN
		if(wlan_idx==1)
#elif defined(WLAN0_QTN)
		if(wlan_idx==0)
#endif
		{
			unsigned int uInt;
			uInt = atoi(strRate);
			if(uInt<78)
				uInt--;
			Entry.fixedTxRate = uInt;
		}
		else
#endif
			{
				{
					unsigned int uInt;
					uInt = atoi(strRate);
					if(uInt<30)
						uInt = 1 << (uInt-1);
					else
						uInt = ((1 << 31) + (uInt-30));
					Entry.fixedTxRate = uInt;
				}
				strRate = boaGetVar(wp, "basicrates", "");
				if ( strRate[0] ) {
					uShort = atoi(strRate);
					if ( mib_set(MIB_WLAN_BASIC_RATE, (void *)&uShort) == 0) {
						strcpy(tmpBuf, strSetBaseRateErr);
						goto setErr_advance;
					}
				}

				strRate = boaGetVar(wp, "operrates", "");
				if ( strRate[0] ) {
					uShort = atoi(strRate);
					if ( mib_set(MIB_WLAN_SUPPORTED_RATE, (void *)&uShort) == 0) {
						strcpy(tmpBuf, strSetOperRateErr);
						goto setErr_advance;
					}
				}
			}
		}
	}
	else { // set rate in operate, basic sperately
#ifdef WIFI_TEST
		// disable rate adaptive
		Entry.rateAdaptiveEnabled = 0;

#endif // of WIFI_TEST
	}

	// set preamble
	strPreamble = boaGetVar(wp, "preamble", "");
	if (strPreamble[0]) {
		if (!gstrcmp(strPreamble, "long"))
			preamble = LONG_PREAMBLE;
		else if (!gstrcmp(strPreamble, "short"))
			preamble = SHORT_PREAMBLE;
		else {
			strcpy(tmpBuf, strInvdPreamble);
			goto setErr_advance;
		}
		vChar = (char)preamble;
		if ( mib_set(MIB_WLAN_PREAMBLE_TYPE, (void *)&vChar) == 0) {
			strcpy(tmpBuf, strSetPreambleErr);
			goto setErr_advance;
		}
	}

	// set hidden SSID
	strHiddenSSID = boaGetVar(wp, "hiddenSSID", "");
	if (strHiddenSSID[0]) {
		if (!gstrcmp(strHiddenSSID, "no"))
			vChar = 0;
		else if (!gstrcmp(strHiddenSSID, "yes"))
			vChar = 1;
		else {
			strcpy(tmpBuf, strInvdBrodSSID);
			goto setErr_advance;
		}
		Entry.hidessid = vChar;
		#ifdef CONFIG_WIFI_SIMPLE_CONFIG
		#ifdef WPS20
		if(vChar && wpsUseVersion != 0)//if hidden, wsc should disable
			Entry.wsc_disabled = vChar;
		#endif
		#endif
	}

	strProtection = boaGetVar(wp, "protection", "");
	if (strProtection[0]){
		if (!gstrcmp(strProtection,"yes"))
			vChar = 0;
		else if (!gstrcmp(strProtection,"no"))
			vChar = 1;
		else{
			strcpy(tmpBuf, strInvdProtection);
			goto setErr_advance;
		}
		if (mib_set(MIB_WLAN_PROTECTION_DISABLED,(void *)&vChar) ==0){
			strcpy(tmpBuf, strSetProtectionErr);
			goto setErr_advance;
		}
	}

	strAggregation = boaGetVar(wp, "aggregation", "");
	if (strAggregation[0]){
		if (!gstrcmp(strAggregation,"enable"))
			vChar = 1;
		else if (!gstrcmp(strAggregation,"disable"))
			vChar = 0;
		else{
			strcpy(tmpBuf, strInvdAggregation);
			goto setErr_advance;
		}
		if (mib_set(MIB_WLAN_AGGREGATION,(void *)&vChar) ==0){
			strcpy(tmpBuf, strSetAggregationErr);
			goto setErr_advance;
		}
	}

	strShortGIO = boaGetVar(wp, "shortGI0", "");
	if (strShortGIO[0]){
		if (!gstrcmp(strShortGIO,"on"))
			vChar = 1;
		else if (!gstrcmp(strShortGIO,"off"))
			vChar = 0;
		else{
			strcpy(tmpBuf, strInvdShortGI0);
			goto setErr_advance;
		}
		if (mib_set(MIB_WLAN_SHORTGI_ENABLED,(void *)&vChar) ==0){
			strcpy(tmpBuf, strSetShortGI0Err);
			goto setErr_advance;
		}
	}

	strDtim = boaGetVar(wp, "dtimPeriod", "");
	if (strDtim[0]) {
		if ( !string_to_dec(strDtim, &val) || val<1 || val>255) {
			strcpy(tmpBuf, strInvdDTIMPerd);
			goto setErr_advance;
		}
		vChar = (char)val;
		if ( mib_set(MIB_WLAN_DTIM_PERIOD, (void *)&vChar) == 0) {
			strcpy(tmpBuf, strSetDTIMErr);
			goto setErr_advance;
		}
	}

	// set block-relay
	strBlock = boaGetVar(wp, "block", "");
	if (strBlock[0]) {
		if (strBlock[0] == '0')
			vChar = 0;
		else // '1'
			vChar = 1;
		Entry.userisolation = vChar;
	}

#ifdef WLAN_QoS
	strBlock = boaGetVar(wp, "WmmEnabled", "");
	if (strBlock[0]) {
		if (strBlock[0] == '0')
			vChar = 0;
		else // '1'
			vChar = 1;
		Entry.wmmEnabled = vChar;
	}
#endif
	mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, 0);
	config_WLAN(ACT_RESTART);

// Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif // of #if COMMIT_IMMEDIATELY

#ifndef NO_ACTION
	run_script(-1);
#endif

	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page

//	boaRedirect(wp, submitUrl);
	OK_MSG(submitUrl);
  	return;

setErr_advance:
	ERR_MSG(tmpBuf);
}

void set_11ac_txrate(WLAN_STA_INFO_Tp pInfo,char* txrate)
{
	char channelWidth=0;//20M 0,40M 1,80M 2
	char shortGi=0;
	char rate_idx=pInfo->txOperaRates-0x90;
	if(!txrate)return;
/*
	TX_USE_40M_MODE         = BIT(0),
	TX_USE_SHORT_GI         = BIT(1),
	TX_USE_80M_MODE         = BIT(2)
*/
	if(pInfo->ht_info & 0x4)
		channelWidth=2;
	else if(pInfo->ht_info & 0x1)
		channelWidth=1;
	else
		channelWidth=0;
	if(pInfo->ht_info & 0x2)
		shortGi=1;
	
	sprintf(txrate, "%d", VHT_MCS_DATA_RATE[channelWidth][shortGi][rate_idx]>>1);
}

/////////////////////////////////////////////////////////////////////////////
int wirelessVAPClientList(int eid, request *wp, int argc, char **argv)
{
	int nBytesSent=0, i, found=0;
	WLAN_STA_INFO_Tp pInfo;
	char *buff;
	char s_ifname[16];
	char mode_buf[20];
	char txrate[20];
	int rateid=0;
	

#ifdef WLAN_QTN
#ifdef WLAN1_QTN
	if(wlan_idx==1){
#else
	if(wlan_idx==0){
#endif

		int virtual_index;
		if (argc == 2) 
			virtual_index = atoi(argv[argc-1]);
		else
			return 0;
		unsigned int sta_num=0;
		unsigned int tx_pkt, rx_pkt, tx_phy_rate;
		unsigned char mac_addr[6];
		unsigned char ifname[16];
		char status[16];
		snprintf(ifname, 16, "wifi%d", virtual_index);

		if(!rt_qcsapi_get_status(ifname, status)){
			rt_qcsapi_get_sta_num(ifname, &sta_num);

			for (i=0; i<sta_num; i++) {
				rt_qcsapi_get_sta_tx_pkt(ifname, i, &tx_pkt);
				rt_qcsapi_get_sta_rx_pkt(ifname, i, &rx_pkt);
				rt_qcsapi_get_sta_tx_phy_rate(ifname, i, &tx_phy_rate);
				rt_qcsapi_get_sta_mac_addr(ifname, i, mac_addr);
				nBytesSent += boaWrite(wp,
				"<tr bgcolor=#b7b7b7><td><font size=2>%02x:%02x:%02x:%02x:%02x:%02x</td>"
				"<td><font size=2>---</td>"
				"<td><font size=2>%d</td>"
				"<td><font size=2>%d</td>"
				"<td><font size=2>%d</td>"
				"<td><font size=2>---</td>"
				"<td><font size=2>---</td></tr>",
				mac_addr[0],mac_addr[1],mac_addr[2],mac_addr[3],mac_addr[4],mac_addr[5],
				tx_pkt, rx_pkt,
				tx_phy_rate);
				found++;
			}
		}
		if (found == 0) {
			nBytesSent += boaWrite(wp,
				"<tr bgcolor=#b7b7b7><td><font size=2>None</td>"
				"<td><font size=2>---</td>"
				"<td><font size=2>---</td>"
				"<td><font size=2>---</td>"
				"<td><font size=2>---</td>"
				"<td><font size=2>---</td>"
				"<td><font size=2>---</td></tr>");
		}

		return nBytesSent;
	}
#endif


	buff = calloc(1, sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1));
	if ( buff == 0 ) {
		printf("Allocate buffer failed!\n");
		return 0;
	}

#ifdef WLAN_MBSSID
	char Root_WLAN_IF[20];

	snprintf(s_ifname, 16, "%s", (char *)getWlanIfName());
	strcpy(Root_WLAN_IF, s_ifname);
	if (argc == 2) {
		int virtual_index;
		char virtual_name[20];
		virtual_index = atoi(argv[argc-1]) - 1;

		snprintf(s_ifname, 16, "%s-vap%d", (char *)getWlanIfName(), virtual_index);
	}
#endif

	if (getWlStaInfo(s_ifname,  (WLAN_STA_INFO_Tp)buff) < 0) {
		printf("Read wlan sta info failed!\n");

#ifdef WLAN_MBSSID
		if (argc == 2)
			strcpy(s_ifname, Root_WLAN_IF);
#endif
		return 0;
	}

#ifdef WLAN_MBSSID
	if (argc == 2)
		strcpy(s_ifname, Root_WLAN_IF);
#endif

	for (i=1; i<=MAX_STA_NUM; i++) {
		pInfo = (WLAN_STA_INFO_Tp)&buff[i*sizeof(WLAN_STA_INFO_T)];
		if (pInfo->aid && (pInfo->flag & STA_INFO_FLAG_ASOC)) {
			if(pInfo->network& BAND_5G_11AC)
				sprintf(mode_buf, "%s", (" 11ac"));
			else if (pInfo->network & BAND_11N)
				sprintf(mode_buf, "%s", (" 11n"));
			else if (pInfo->network & BAND_11G)
				sprintf(mode_buf,"%s",  (" 11g"));
			else if (pInfo->network & BAND_11B)
				sprintf(mode_buf, "%s", (" 11b"));
			else if (pInfo->network& BAND_11A)
				sprintf(mode_buf, "%s", (" 11a"));
			else
				sprintf(mode_buf, "%s", (" ---"));

			//printf("\n\nthe sta txrate=%d\n\n\n", pInfo->txOperaRates);
			
			if(pInfo->txOperaRates >= 0x90) {
				//sprintf(txrate, "%d", pInfo->acTxOperaRate);
				set_11ac_txrate(pInfo, txrate);
			} else if ((pInfo->txOperaRates & 0x80) != 0x80) {
				if (pInfo->txOperaRates%2) {
					sprintf(txrate, "%d%s",pInfo->txOperaRates/2, ".5");
				} else {
					sprintf(txrate, "%d",pInfo->txOperaRates/2);
				}
			} else {
				if ((pInfo->ht_info & 0x1)==0) { //20M
					if ((pInfo->ht_info & 0x2)==0){//long
						for (rateid=0; rateid<16;rateid++) {
							if (rate_11n_table_20M_LONG[rateid].id == pInfo->txOperaRates) {
								sprintf(txrate, "%s", rate_11n_table_20M_LONG[rateid].rate);
								break;
							}
						}
					} else if ((pInfo->ht_info & 0x2)==0x2) {//short
						for (rateid=0; rateid<16;rateid++) {
							if (rate_11n_table_20M_SHORT[rateid].id == pInfo->txOperaRates) {
								sprintf(txrate, "%s", rate_11n_table_20M_SHORT[rateid].rate);
								break;
							}
						}
					}
				} else if ((pInfo->ht_info & 0x1)==0x1) {//40M
					if ((pInfo->ht_info & 0x2)==0) {//long
						for (rateid=0; rateid<16;rateid++) {
							if (rate_11n_table_40M_LONG[rateid].id == pInfo->txOperaRates) {
								sprintf(txrate, "%s", rate_11n_table_40M_LONG[rateid].rate);
								break;
							}
						}
					} else if ((pInfo->ht_info & 0x2)==0x2) {//short
						for (rateid=0; rateid<16;rateid++) {
							if (rate_11n_table_40M_SHORT[rateid].id == pInfo->txOperaRates) {
								sprintf(txrate, "%s", rate_11n_table_40M_SHORT[rateid].rate);
								break;
							}
						}
					}
				}
			}
			nBytesSent += boaWrite(wp,
		   		"<tr bgcolor=#b7b7b7><td><font size=2>%02x:%02x:%02x:%02x:%02x:%02x</td>"
				"<td><font size=2>%s</td>"
				"<td><font size=2>%d</td>"
	     			"<td><font size=2>%d</td>"
				"<td><font size=2>%s</td>"
				"<td><font size=2>%s</td>"
				"<td><font size=2>%d</td>"
				"</tr>",
				pInfo->addr[0],pInfo->addr[1],pInfo->addr[2],pInfo->addr[3],pInfo->addr[4],pInfo->addr[5],
				mode_buf,
				pInfo->tx_packets, pInfo->rx_packets,
				txrate,
				((pInfo->flag & STA_INFO_FLAG_ASLEEP) ? "yes" : "no"),
				pInfo->expired_time / 100
			);
			found ++;
		}
	}
	if (found == 0) {
		nBytesSent += boaWrite(wp,
	   		"<tr bgcolor=#b7b7b7><td><font size=2>None</td>"
			"<td><font size=2>---</td>"
	     		"<td><font size=2>---</td>"
			"<td><font size=2>---</td>"
			"<td><font size=2>---</td>"
			"<td><font size=2>---</td>"
			"<td><font size=2>---</td>"
			"</tr>");
	}

	free(buff);

	return nBytesSent;
}

/////////////////////////////////////////////////////////////////////////////
void formWirelessVAPTbl(request * wp, char *path, char *query)
{
	char *submitUrl;

	submitUrl = boaGetVar(wp, "submit-url", "");
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
}

/////////////////////////////////////////////////////////////////////////////
int wirelessClientList(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0, i,j,found=0;
	WLAN_STA_INFO_Tp pInfo;
	char *buff;
	char s_ifname[16];
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
	char *strVal;

	strVal = boaGetVar(wp, "wlan_idx", "");
	if ( strVal[0] ) {
		//printf("wlan_idx=%d\n", strVal[0]-'0');
		wlan_idx = strVal[0]-'0';
	}
#endif //CONFIG_RTL_92D_SUPPORT || WLAN_DUALBAND_CONCURRENT

#ifdef WLAN_QTN
#ifdef WLAN1_QTN
	if(wlan_idx==1){
#else
	if(wlan_idx==0){
#endif
		unsigned int sta_num;
		unsigned int tx_pkt, rx_pkt, tx_phy_rate;
		unsigned char mac_addr[6];
		unsigned char ifname[16];
		snprintf(ifname, 16, "wifi0");
		rt_qcsapi_get_sta_num(ifname, &sta_num);

		for (i=0; i<sta_num; i++) {
			rt_qcsapi_get_sta_tx_pkt(ifname, i, &tx_pkt);
			rt_qcsapi_get_sta_rx_pkt(ifname, i, &rx_pkt);
			rt_qcsapi_get_sta_tx_phy_rate(ifname, i, &tx_phy_rate);
			rt_qcsapi_get_sta_mac_addr(ifname, i, mac_addr);
			nBytesSent += boaWrite(wp,
	   		"<tr bgcolor=#b7b7b7><td><font size=2>%02x:%02x:%02x:%02x:%02x:%02x</td>"
			"<td><font size=2>%d</td>"
	     		"<td><font size=2>%d</td>"
			"<td><font size=2>%d</td>"
			"<td><font size=2>---</td>"
			"<td><font size=2>---</td></tr>",
			mac_addr[0],mac_addr[1],mac_addr[2],mac_addr[3],mac_addr[4],mac_addr[5],
			tx_pkt, rx_pkt,
			tx_phy_rate);
			found++;
		}

		if (found == 0) {
			nBytesSent += boaWrite(wp,
		   		"<tr bgcolor=#b7b7b7><td><font size=2>None</td>"
				"<td><font size=2>---</td>"
		     		"<td><font size=2>---</td>"
				"<td><font size=2>---</td>"
				"<td><font size=2>---</td>"
				"<td><font size=2>---</td></tr>");
		}

		return nBytesSent;
	}
#endif

//xl_yue
//#ifndef WLAN_MBSSID
	buff = calloc(1, sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1));
	if ( buff == 0 ) {
		printf("Allocate buffer failed!\n");
		return 0;
	}
	if ( getWlStaInfo(getWlanIfName(),  (WLAN_STA_INFO_Tp)buff ) < 0 ) {
		printf("Read wlan sta info failed!\n");
		free(buff);
		return 0;
	}
#if 0
//#else
	for(j=0;j<=WLAN_MBSSID_NUM;j++)
	{
		buff = calloc(1, sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1));
		if ( buff == 0 ) {
			printf("Allocate buffer failed!\n");
			return 0;
		}

		if (j==0)
			snprintf(s_ifname, 16, "%s", (char *)getWlanIfName());
		if (j >= WLAN_VAP_ITF_INDEX && j < WLAN_VAP_ITF_INDEX+WLAN_MBSSID_NUM)
			snprintf(s_ifname, 16, "%s-vap%d", (char *)getWlanIfName(), j-WLAN_VAP_ITF_INDEX);
		if ( getWlStaInfo(s_ifname,  (WLAN_STA_INFO_Tp)buff ) < 0 ) {
			printf("Read wlan sta info failed!\n");
			free(buff);
			return 0;
		}
#endif

	for (i=1; i<=MAX_STA_NUM; i++) {
		pInfo = (WLAN_STA_INFO_Tp)&buff[i*sizeof(WLAN_STA_INFO_T)];
		if (pInfo->aid && (pInfo->flag & STA_INFO_FLAG_ASOC)) {
			char txrate[20];
			int rateid=0;

			if(pInfo->txOperaRates >= 0x90) {
				//sprintf(txrate, "%d", pInfo->acTxOperaRate);
				set_11ac_txrate(pInfo, txrate);
			}else if((pInfo->txOperaRates & 0x80) != 0x80){
				if(pInfo->txOperaRates%2){
					sprintf(txrate, "%d%s",pInfo->txOperaRates/2, ".5");
				}else{
					sprintf(txrate, "%d",pInfo->txOperaRates/2);
				}
			}else{
				if((pInfo->ht_info & 0x1)==0){ //20M
					if((pInfo->ht_info & 0x2)==0){//long
						for(rateid=0; rateid<16;rateid++){
							if(rate_11n_table_20M_LONG[rateid].id == pInfo->txOperaRates){
								sprintf(txrate, "%s", rate_11n_table_20M_LONG[rateid].rate);
								break;
							}
						}
					}else if((pInfo->ht_info & 0x2)==0x2){//short
						for(rateid=0; rateid<16;rateid++){
							if(rate_11n_table_20M_SHORT[rateid].id == pInfo->txOperaRates){
								sprintf(txrate, "%s", rate_11n_table_20M_SHORT[rateid].rate);
								break;
							}
						}
					}
				}else if((pInfo->ht_info & 0x1)==0x1){//40M
					if((pInfo->ht_info & 0x2)==0){//long

						for(rateid=0; rateid<16;rateid++){
							if(rate_11n_table_40M_LONG[rateid].id == pInfo->txOperaRates){
								sprintf(txrate, "%s", rate_11n_table_40M_LONG[rateid].rate);
								break;
							}
						}
					}else if((pInfo->ht_info & 0x2)==0x2){//short
						for(rateid=0; rateid<16;rateid++){
							if(rate_11n_table_40M_SHORT[rateid].id == pInfo->txOperaRates){
								sprintf(txrate, "%s", rate_11n_table_40M_SHORT[rateid].rate);
								break;
							}
						}
					}
				}

			}

/*
printf("***%02x:%02x:%02x:%02x:%02x:%02x,%d,%d,%d%s,%s,%d, j=%d\n",pInfo->addr[0],pInfo->addr[1],pInfo->addr[2],pInfo->addr[3],pInfo->addr[4],pInfo->addr[5],
			pInfo->tx_packets, pInfo->rx_packets,
			pInfo->txOperaRates/2, ((pInfo->txOperaRates%2) ? ".5" : "" ),
			( (pInfo->flag & STA_INFO_FLAG_ASLEEP) ? "yes" : "no"),
			pInfo->expired_time/100,j);
*/
			nBytesSent += boaWrite(wp,
	   		"<tr bgcolor=#b7b7b7><td><font size=2>%02x:%02x:%02x:%02x:%02x:%02x</td>"
			"<td><font size=2>%d</td>"
	     		"<td><font size=2>%d</td>"
			"<td><font size=2>%s</td>"
			"<td><font size=2>%s</td>"
			"<td><font size=2>%d</td></tr>",
			pInfo->addr[0],pInfo->addr[1],pInfo->addr[2],pInfo->addr[3],pInfo->addr[4],pInfo->addr[5],
			pInfo->tx_packets, pInfo->rx_packets,
			txrate,
			( (pInfo->flag & STA_INFO_FLAG_ASLEEP) ? "yes" : "no"),
			pInfo->expired_time/100 );
			found++;
		}
	}
//xl_yue
#if 0
//#ifdef WLAN_MBSSID
	free(buff);
	}
#endif

	if (found == 0) {
		nBytesSent += boaWrite(wp,
	   		"<tr bgcolor=#b7b7b7><td><font size=2>None</td>"
			"<td><font size=2>---</td>"
	     		"<td><font size=2>---</td>"
			"<td><font size=2>---</td>"
			"<td><font size=2>---</td>"
			"<td><font size=2>---</td></tr>");
	}
//xl_yue
#ifndef WLAN_MBSSID
	free(buff);
#endif
	return nBytesSent;
}

/////////////////////////////////////////////////////////////////////////////
void formWirelessTbl(request * wp, char *path, char *query)
{
	char *submitUrl;
	char *strWlanId;

	strWlanId= boaGetVar(wp, "wlan_idx", "");
	if(strWlanId[0]){
		wlan_idx = atoi(strWlanId);
		//printf("%s: wlan_idx=%d\n", __func__, wlan_idx);
	}
	submitUrl = boaGetVar(wp, "submit-url", "");
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
}

#if defined(WLAN_CLIENT) || defined(WLAN_SITESURVEY)
static SS_STATUS_Tp pStatus=NULL;
/////////////////////////////////////////////////////////////////////////////
void formWlSiteSurvey(request * wp, char *path, char *query)
{
 	char *submitUrl, *refresh, *connect, *strSel, *strEncrypt;
	int status, idx;
	unsigned char res, *pMsg=NULL;
	int wait_time;
	char tmpBuf[100];
	char *strVal;

#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
	strVal = boaGetVar(wp, "wlan_idx", "");
	if ( strVal[0] ) {
		printf("wlan_idx=%d\n", strVal[0]-'0');
		wlan_idx = strVal[0]-'0';
	}
#endif //CONFIG_RTL_92D_SUPPORT || WLAN_DUALBAND_CONCURRENT

	submitUrl = boaGetVar(wp, "submit-url", "");

	refresh = boaGetVar(wp, "refresh", "");
	if ( refresh[0] ) {

#ifdef WLAN_QTN
#ifdef WLAN1_QTN
		if(wlan_idx == 1)
#else
		if(wlan_idx == 0)
#endif
		{
			va_cmd("/bin/qcsapi_sockrpc", 2, 1, "start_scan", rt_get_qtn_ifname(getWlanIfName()));
			sleep(3);
		}
		else
#endif
		{
			// issue scan request
			wait_time = 0;
			while (1) {
				if ( getWlSiteSurveyRequest(getWlanIfName(),  &status) < 0 ) {
					strcpy(tmpBuf, multilang(LANG_SITE_SURVEY_REQUEST_FAILED));
					goto ss_err;
				}
				if (status != 0) {	// not ready
					if (wait_time++ > 5) {
						strcpy(tmpBuf, multilang(LANG_SCAN_REQUEST_TIMEOUT));
						goto ss_err;
					}
					sleep(1);
				}
				else
					break;
			}

			// wait until scan completely
			wait_time = 0;
			while (1) {
				res = 1;	// only request request status
				if ( getWlSiteSurveyResult(getWlanIfName(), (SS_STATUS_Tp)&res) < 0 ) {
					strcpy(tmpBuf, multilang(LANG_SITE_SURVEY_REQUEST_FAILED));
					free(pStatus);
					pStatus = NULL;
					goto ss_err;
				}
				if (res == 0xff) {   // in progress
#if defined(CONFIG_RTL_DFS_SUPPORT)
					if (wait_time++ > 20)
#else
					if (wait_time++ > 10)
#endif
					{
						strcpy(tmpBuf, multilang(LANG_SCAN_REQUEST_TIMEOUT));
						free(pStatus);
						pStatus = NULL;
						goto ss_err;
					}
					sleep(1);
				}
				else
					break;
			}
		}
		if (submitUrl[0])
			boaRedirect(wp, submitUrl);

		return;
	}

	connect = boaGetVar(wp, "connect", "");
	if ( connect[0] ) {	
		strSel = boaGetVar(wp, "select", "");
			if (strSel[0]) {	
			unsigned char res;
			NETWORK_TYPE_T net;
			int keyLen, len;
			unsigned char chan, encrypt, vChar;
			char *strAuth, *strKeyLen, *strFormat, *wepKey;
			char key[30];

			MIB_CE_MBSSIB_T pEntry;
			int vxd_wisp_wan = 0, interface_idx = 0;
			char opmode, wlanmode, rptEnabled;

			if (pStatus == NULL) {
				strcpy(tmpBuf, multilang(LANG_PLEASE_REFRESH_AGAIN));
				goto ss_err;

			}
			sscanf(strSel, "sel%d", &idx);
			if ( idx >= pStatus->number ) { // invalid index
				strcpy(tmpBuf, multilang(LANG_CONNECT_FAILED_1));
				goto ss_err;
			}

			wlan_getEntry(&pEntry, 0);
			wlanmode = pEntry.wlanMode;
#ifdef WLAN_UNIVERSAL_REPEATER
			mib_get(MIB_REPEATER_ENABLED1, (void *)&rptEnabled);
#else
			rptEnabled = 0;
#endif

			if( (wlanmode == AP_MODE || wlanmode == AP_WDS_MODE) && (rptEnabled == 1))
				vxd_wisp_wan = 1;
			
			if(vxd_wisp_wan)
				interface_idx = WLAN_REPEATER_ITF_INDEX;

			wlan_getEntry(&pEntry, interface_idx);

			
			// Set SSID, network type to MIB
			memcpy(tmpBuf, pStatus->bssdb[idx].bdSsIdBuf, pStatus->bssdb[idx].bdSsId.Length);
			tmpBuf[pStatus->bssdb[idx].bdSsId.Length] = '\0';
			//if ( mib_set(MIB_WLAN_SSID, (void *)tmpBuf) == 0) {
			//	strcpy(tmpBuf, "Set SSID error!");
			//	goto ss_err;
			//}
			strcpy(pEntry.ssid, tmpBuf);
			
			if ( pStatus->bssdb[idx].bdCap & cESS )
				net = INFRASTRUCTURE;
			else
				net = ADHOC;
			
			if(interface_idx == 0){
				if ( mib_set(MIB_WLAN_NETWORK_TYPE, (void *)&net) == 0) {
					sprintf(tmpBuf, "%s:%s", Tset_mib_error, MIB_WLAN_NETWORK_TYPE);
					goto ss_err;
				}
			}
			
			if (net == ADHOC) {
				chan = pStatus->bssdb[idx].ChannelNumber;
				if ( mib_set( MIB_WLAN_CHAN_NUM, (void *)&chan) == 0) {
					sprintf(tmpBuf, "%s:%s", Tset_mib_error, MIB_WLAN_CHAN_NUM);
					goto ss_err;
				}
			}
			else if(vxd_wisp_wan){ //set to root
				chan = pStatus->bssdb[idx].ChannelNumber;
				if ( mib_set( MIB_WLAN_CHAN_NUM, (void *)&chan) == 0) {
					sprintf(tmpBuf, "%s:%s", Tset_mib_error, MIB_WLAN_CHAN_NUM);
					goto ss_err;
				}
				int sideband;
				mib_get( MIB_WLAN_CONTROL_BAND, (void *)&sideband);
				if ((chan<=4 && sideband==0) || (chan>=10 && sideband==1)){
				   sideband = ((sideband + 1) & 0x1);										
				   mib_set( MIB_WLAN_CONTROL_BAND, (void *)&sideband);
				}
			}
			
			//strEncrypt = boaGetVar(wp, "security_method", "");
			strEncrypt = boaGetVar(wp, "wlan_encrypt", "");
			if (!strEncrypt[0]) {
		 		strcpy(tmpBuf, strNoEncryptionErr);
				goto ss_err;
			}

			encrypt = (WIFI_SECURITY_T) strEncrypt[0] - '0';
			vChar = (char)encrypt;
			pEntry.encrypt = vChar;
		
			if (encrypt == WIFI_SEC_WEP) {
				AUTH_TYPE_T authType;
				WEP_T wep;
				// (1) Authentication Type
				strAuth = boaGetVar(wp, "auth_type", "");
				if (strAuth[0]) {
					if ( !gstrcmp(strAuth, "open"))
						authType = AUTH_OPEN;
					else if ( !gstrcmp(strAuth, "shared"))
						authType = AUTH_SHARED;
					else if ( !gstrcmp(strAuth, "both"))
						authType = AUTH_BOTH;
					else {
						strcpy(tmpBuf, strInvdAuthType);
						goto ss_err;
					}
					vChar = (char)authType;
					pEntry.authType = vChar;
				}
				// (2) Key Length
				strKeyLen = boaGetVar(wp, "length0", "");
				if (!strKeyLen[0]) {
					strcpy(tmpBuf, strKeyLenMustExist);
					goto ss_err;
				}
				if (strKeyLen[0]!='1' && strKeyLen[0]!='2') {
					strcpy(tmpBuf, strInvdKeyLen);
					goto ss_err;
				}
				if (strKeyLen[0] == '1')
					wep = WEP64;
				else
					wep = WEP128;

				vChar = (char)wep;

				//if ( mib_set( MIB_WLAN_WEP, (void *)&vChar) == 0) {
				//	strcpy(tmpBuf, strSetWLANWEPErr);
				//	goto ss_err;
				//}
				pEntry.wep = vChar;

				// (3) Key Format
				strFormat = boaGetVar(wp, "format0", "");
				if (!strFormat[0]) {
					strcpy(tmpBuf, strKeyTypeMustExist);
					goto ss_err;
				}

				if (strFormat[0]!='1' && strFormat[0]!='2') {
					strcpy(tmpBuf, strInvdKeyType);
					goto ss_err;
				}

				vChar = (char)(strFormat[0] - '0' - 1);

				//if ( mib_set( MIB_WLAN_WEP_KEY_TYPE, (void *)&vChar) == 0) {
				//	strcpy(tmpBuf, strSetWepKeyTypeErr);
				//	goto ss_err;
				//}
				pEntry.wepKeyType = vChar;

				if (wep == WEP64) {
					if (strFormat[0]=='1')
						keyLen = WEP64_KEY_LEN;
					else
						keyLen = WEP64_KEY_LEN*2;
				}
				else {
					if (strFormat[0]=='1')
						keyLen = WEP128_KEY_LEN;
					else
						keyLen = WEP128_KEY_LEN*2;
				}

				// (4) Encryption Key
				wepKey = boaGetVar(wp, "key0", "");
				if	(wepKey[0]) {
					if (strlen(wepKey) != keyLen) {
						strcpy(tmpBuf, strInvdKey1Len);
						goto ss_err;
					}
					if ( !isAllStar(wepKey) ) {
						if (strFormat[0] == '1') // ascii
							strcpy(key, wepKey);
						else { // hex
							if ( !string_to_hex(wepKey, key, keyLen)) {
								strcpy(tmpBuf, strInvdWEPKey1);
								goto ss_err;
							}
						}
						if (wep == WEP64) {
							//if (mib_set(MIB_WLAN_WEP64_KEY1, (void *)key) == 0 ) {
							//	strcpy(tmpBuf, strSetWEPKey1Err);
							//	goto ss_err;
							//}
							strcpy(pEntry.wep64Key1, key);
						}
						else {
							//if (mib_set(MIB_WLAN_WEP128_KEY1, (void *)key) == 0 ) {
							//	strcpy(tmpBuf, strSetWEPKey1Err);
							//	goto ss_err;
							//}
							strcpy(pEntry.wep128Key1, key);
						}

					}
				}// (4) Encryption Key	
#ifdef CONFIG_WIFI_SIMPLE_CONFIG
#ifdef WPS20
				if(interface_idx == 0)			
					pEntry.wsc_disabled = 1;
#endif
#endif
			}else if(encrypt >= WIFI_SEC_WPA) {	// WPA
#ifdef CONFIG_WIFI_SIMPLE_CONFIG
#ifdef WPS20
				unsigned char disableWps = 0;
#endif //WPS20			
#endif
				if ((encrypt == WIFI_SEC_WPA) || (encrypt == WIFI_SEC_WPA2_MIXED)) {
					
					unsigned char intVal = 0;
					unsigned char val2;
					//strVal = boaGetVar(wp, "ciphersuite_t", "");
					strVal = boaGetVar(wp, "wpa_tkip_aes", "");
					if (strVal[0]=='1')
						intVal |= WPA_CIPHER_TKIP;
					//strVal = boaGetVar(wp, "ciphersuite_a", "");
					if (strVal[0]=='2')
						intVal |= WPA_CIPHER_AES;

					if ( intVal == 0 )
						intVal = WPA_CIPHER_TKIP;
					
					pEntry.unicastCipher = intVal;
			#ifdef CONFIG_WIFI_SIMPLE_CONFIG		
			#ifdef WPS20
					if(interface_idx == 0){
						if ((encrypt == WIFI_SEC_WPA) ||
							(encrypt == WIFI_SEC_WPA2_MIXED && intVal == WPA_CIPHER_TKIP)) {	//disable wps if wpa only or tkip only
							disableWps = 1;
						}
					}
			#endif
			#endif
				}
		
				// Mason Yu. 201009_new_security. Set wpa2ciphersuite(wpa2_cipher) for wpa2/wpa mixed
				if ((encrypt == WIFI_SEC_WPA2) || (encrypt == WIFI_SEC_WPA2_MIXED)) {
					unsigned char intVal = 0;
					//strVal = boaGetVar(wp, "wpa2ciphersuite_t", "");
					strVal = boaGetVar(wp, "wpa2_tkip_aes", "");
					if (strVal[0]=='1')
						intVal |= WPA_CIPHER_TKIP;
					//strVal = boaGetVar(wp, "wpa2ciphersuite_a", "");
					if (strVal[0]=='2')
						intVal |= WPA_CIPHER_AES;

					if ( intVal == 0 )
						intVal = WPA_CIPHER_AES;

					pEntry.wpa2UnicastCipher = intVal;
				#ifdef CONFIG_WIFI_SIMPLE_CONFIG
				#ifdef WPS20
					if(interface_idx == 0){
						if (encrypt == WIFI_SEC_WPA2) {
							if (intVal == WPA_CIPHER_TKIP)
								disableWps = 1;
						}
						else { // mixed
							if (intVal == WPA_CIPHER_TKIP && disableWps)	//disable wps if wpa2 mixed + tkip only
								disableWps = 1;
							else
								disableWps = 0;
						}
					}
				#endif //WPS20
				#endif
				}
#ifdef CONFIG_WIFI_SIMPLE_CONFIG
#ifdef WPS20
				if(interface_idx == 0){
					if (disableWps) 
						pEntry.wsc_disabled = 1;
				}
#endif //WPS20
#endif
				// pre-shared key
					strVal = boaGetVar(wp, "pskFormat", "");
					if (!strVal[0]) {
						strcpy(tmpBuf, strNoPSKFormat);
						goto ss_err;
					}
					vChar = strVal[0] - '0';
					if (vChar != 0 && vChar != 1) {
						strcpy(tmpBuf, strInvdPSKFormat);
						goto ss_err;
					}
		
					strVal = boaGetVar(wp, "pskValue", "");
					len = strlen(strVal);
		
					//if ( mib_set(MIB_WLAN_WPA_PSK_FORMAT, (void *)&vChar) == 0) {
					//	strcpy(tmpBuf, strSetWPAPSKFMATErr);
					//	goto ss_err;
					//}
					pEntry.wpaPSKFormat = vChar;
					
		
					if (vChar==1) { // hex
						if (len!=MAX_PSK_LEN || !string_to_hex(strVal, tmpBuf, MAX_PSK_LEN)) {
							strcpy(tmpBuf, strInvdPSKValue);
							goto ss_err;
						}
					}
					else { // passphras
						if (len==0 || len > (MAX_PSK_LEN - 1) ) {
							strcpy(tmpBuf, strInvdPSKValue);
							goto ss_err;
						}
					}
		


					//if ( !mib_set(MIB_WLAN_WPA_PSK, (void *)strVal)) {
					//	strcpy(tmpBuf, strSetWPAPSKErr);
					//	goto ss_err;
					//}
					strcpy(pEntry.wpaPSK, strVal);

			}

			wlan_setEntry(&pEntry, interface_idx);
			mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);	// update to flash

#ifdef WLAN_QTN
#ifdef WLAN1_QTN
			if(wlan_idx==1)
#else
			if(wlan_idx==0)
#endif
			{
				rt_qcsapi_set_STA_config(rt_get_qtn_ifname(getWlanIfName()));
				pMsg = "Finish Setting";
			}
			else
#endif
{
			config_WLAN(ACT_RESTART);

			res = idx;
			wait_time = 0;
			char interface_name[20];

			if(vxd_wisp_wan)
				sprintf(interface_name, "%s-vxd", getWlanIfName());
			else
				sprintf(interface_name, "%s", getWlanIfName());

#ifndef CONFIG_SUPPORT_CLIENT_MIXED_SECURITY
			while (1) {
				if ( getWlJoinRequest(interface_name, &pStatus->bssdb[idx], &res) < 0 ) {
					strcpy(tmpBuf, multilang(LANG_JOIN_REQUEST_FAILED));
					goto ss_err;
				}
				if ( res == 1 ) { // wait
					if (wait_time++ > 5) {
						strcpy(tmpBuf, multilang(LANG_CONNECT_REQUEST_TIMEOUT));
						goto ss_err;
					}
					sleep(1);
					continue;
				}
				break;
			}

			if ( res == 2 ) // invalid index
				pMsg = "Connect failed 2!";
			else {
				wait_time = 0;
				while (1) {
					if ( getWlJoinResult(interface_name, &res) < 0 ) {
						strcpy(tmpBuf, multilang(LANG_GET_JOIN_RESULT_FAILED));
						goto ss_err;
					}
					if ( res != 0xff ) { // completed
						break;
					}
					if (wait_time++ > 10) {
						strcpy(tmpBuf, multilang(LANG_CONNECT_REQUEST_TIMEOUT));
						goto ss_err;
					}
					sleep(1);
				}

				if ( res!=STATE_Bss && res!=STATE_Ibss_Idle && res!=STATE_Ibss_Active )
					pMsg = "Connect failed 3!";
				else {
					status = 0;
					if (encrypt == WIFI_SEC_WPA
						|| encrypt == WIFI_SEC_WPA2) {
						bss_info bss;
						wait_time = 0;
						while (wait_time++ < 5) {
							getWlBssInfo(interface_name, &bss);
							if (bss.state == STATE_CONNECTED)
								break;
							sleep(1);
						}
						if (wait_time >= 5)
							status = 1;
					}
					if (status)
						pMsg = "Connect failed 4!";
					else
						pMsg = "Connect successfully!";
				}
			}
#else
			status = 0;
			bss_info bss;
			while (wait_time++ < 20) {
				getWlBssInfo(interface_name, &bss);
				if (bss.state == STATE_CONNECTED)
					break;
				sleep(1);
			}
			if (wait_time >= 20)
				status = 1;
			if (status)
				pMsg = "Connect failed 4!";
			else
				pMsg = "Connect successfully!";
#endif	
}
			OK_MSG1(pMsg, submitUrl);
		}
	
	}
	return;

ss_err:
	ERR_MSG(tmpBuf);
}

/////////////////////////////////////////////////////////////////////////////
int wlSiteSurveyTbl(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0, i;
	BssDscr *pBss;
	char tmpBuf[100], ssidbuf[40],tmp1Buf[10], tmp2Buf[20], wpa_tkip_aes[20],wpa2_tkip_aes[20];
	WLAN_MODE_T mode;
	bss_info bss;
#ifdef WLAN_UNIVERSAL_REPEATER
	char rpt_enabled;
#endif
	MIB_CE_MBSSIB_T Entry;

	if (pStatus==NULL) {
		pStatus = calloc(1, sizeof(SS_STATUS_T));
		if ( pStatus == NULL ) {
			printf("Allocate buffer failed!\n");
			return 0;
		}
	}

	pStatus->number = 0; // request BSS DB
#ifdef WLAN_QTN
#ifdef WLAN1_QTN
	if(wlan_idx == 1){
#else
	if(wlan_idx == 0){
#endif
		memset(pStatus, 0, sizeof(SS_STATUS_T));
		if ( rt_qcsapi_get_properties_AP(rt_get_qtn_ifname(getWlanIfName()), pStatus) < 0 ) {
			ERR_MSG(multilang(LANG_READ_SITE_SURVEY_STATUS_FAILED));
			free(pStatus);
			pStatus = NULL;
			return 0;
		}
		wlan_getEntry((void *)&Entry, 0);
		mode=Entry.wlanMode;
	}
	else
#endif
	{
		if ( getWlSiteSurveyResult(getWlanIfName(), pStatus) < 0 ) {
			ERR_MSG(multilang(LANG_READ_SITE_SURVEY_STATUS_FAILED));
			free(pStatus);
			pStatus = NULL;
			return 0;
		}
		wlan_getEntry((void *)&Entry, 0);
		mode=Entry.wlanMode;
		if ( getWlBssInfo(getWlanIfName(), &bss) < 0) {
			printf("Get bssinfo failed!");
			return 0;
		}
	}
#ifdef WLAN_UNIVERSAL_REPEATER
	mib_get( MIB_REPEATER_ENABLED1, (void *)&rpt_enabled);
#endif

	nBytesSent += boaWrite(wp, "<tr>"
	"<td align=center width=\"30%%\" bgcolor=\"#808080\"><font size=\"2\"><b>SSID</b></font></td>\n"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>BSSID</b></font></td>\n"
	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b>%s</b></font></td>\n"
      	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b>%s</b></font></td>\n"
      	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b>%s</b></font></td>\n"
	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b>%s</b></font></td>\n",
	multilang(LANG_CHANNEL), multilang(LANG_TYPE), multilang(LANG_ENCRYPTION), multilang(LANG_SIGNAL));
	if ( mode == CLIENT_MODE 
#ifdef WLAN_UNIVERSAL_REPEATER
		|| rpt_enabled
#endif
		)
		nBytesSent += boaWrite(wp, "<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Select</b></font></td></tr>\n");
	else
		nBytesSent += boaWrite(wp, "</tr>\n");

	for (i=0; i<pStatus->number && pStatus->number!=0xff; i++) {
		pBss = &pStatus->bssdb[i];
		snprintf(tmpBuf, 200, "%02x:%02x:%02x:%02x:%02x:%02x",
			pBss->bdBssId[0], pBss->bdBssId[1], pBss->bdBssId[2],
			pBss->bdBssId[3], pBss->bdBssId[4], pBss->bdBssId[5]);

		//memcpy(ssidbuf, pBss->bdSsIdBuf, pBss->bdSsId.Length);
		//ssidbuf[pBss->bdSsId.Length] = '\0';
		memcpy(ssidbuf, pBss->bdSsIdBuf, pBss->bdSsId.Length>=SSID_LEN?SSID_LEN-1:pBss->bdSsId.Length);
		ssidbuf[pBss->bdSsId.Length>=SSID_LEN?SSID_LEN-1:pBss->bdSsId.Length] = '\0';

		if (pBss->network==BAND_11B)
				strcpy(tmp1Buf, (" (B)"));
		else if (pBss->network==BAND_11G)
				strcpy(tmp1Buf, (" (G)"));	   
		else if (pBss->network==(BAND_11G|BAND_11B))
				strcpy(tmp1Buf, (" (B+G)"));
		else if (pBss->network==(BAND_11N))
				strcpy(tmp1Buf, (" (N)"));				
		else if (pBss->network==(BAND_11G|BAND_11N))
				strcpy(tmp1Buf, (" (G+N)"));
		else if (pBss->network==(BAND_11G|BAND_11B | BAND_11N))
				strcpy(tmp1Buf, (" (B+G+N)"));
		else if(pBss->network== BAND_11A)
				strcpy(tmp1Buf, (" (A)"));
		else if(pBss->network== (BAND_11A | BAND_11N))
				strcpy(tmp1Buf, (" (A+N)"));
		else if(pBss->network== (BAND_5G_11AC | BAND_11N))
				strcpy(tmp1Buf, (" (AC+N)"));
		else if(pBss->network== (BAND_11A | BAND_5G_11AC))
				strcpy(tmp1Buf, (" (A+AC)"));				 
		else if(pBss->network== (BAND_11A |BAND_11N | BAND_5G_11AC))
				strcpy(tmp1Buf, (" (A+N+AC)")); 		  
		else
				sprintf(tmp1Buf, (" -%d-"),pBss->network);

		
		memset(wpa_tkip_aes,0x00,sizeof(wpa_tkip_aes));
		memset(wpa2_tkip_aes,0x00,sizeof(wpa2_tkip_aes));
						
		if ((pBss->bdCap & cPrivacy) == 0)
			sprintf(tmp2Buf, "no");
		else {
			if (pBss->bdTstamp[0] == 0)
				sprintf(tmp2Buf, "WEP");
#if defined(CONFIG_RTL_WAPI_SUPPORT)
			else if (pBss->bdTstamp[0] == SECURITY_INFO_WAPI)
				sprintf(tmp2Buf, "WAPI");
#endif
			else 
			{
				int wpa_exist = 0, idx = 0;
				if (pBss->bdTstamp[0] & 0x0000ffff) 
				{
					idx = sprintf(tmp2Buf, "WPA");
					if (((pBss->bdTstamp[0] & 0x0000f000) >> 12) == 0x4)
						idx += sprintf(tmp2Buf+idx, "-PSK");
					else if(((pBss->bdTstamp[0] & 0x0000f000) >> 12) == 0x2)
						idx += sprintf(tmp2Buf+idx, "-1X");
					wpa_exist = 1;
				
					if (((pBss->bdTstamp[0] & 0x00000f00) >> 8) == 0x5)
						sprintf(wpa_tkip_aes,"%s","aes/tkip");
					else if (((pBss->bdTstamp[0] & 0x00000f00) >> 8) == 0x4)
						sprintf(wpa_tkip_aes,"%s","aes");
					else if (((pBss->bdTstamp[0] & 0x00000f00) >> 8) == 0x1)
						sprintf(wpa_tkip_aes,"%s","tkip");
				}
				if (pBss->bdTstamp[0] & 0xffff0000) 
				{
					if (wpa_exist)
						idx += sprintf(tmp2Buf+idx, "/");
					idx += sprintf(tmp2Buf+idx, "WPA2");
					if (((pBss->bdTstamp[0] & 0xf0000000) >> 28) == 0x4)
						idx += sprintf(tmp2Buf+idx, "-PSK");
					else if (((pBss->bdTstamp[0] & 0xf0000000) >> 28) == 0x2)
						idx += sprintf(tmp2Buf+idx, "-1X");

					if (((pBss->bdTstamp[0] & 0x0f000000) >> 24) == 0x5)
						sprintf(wpa2_tkip_aes,"%s","aes/tkip");
					else if (((pBss->bdTstamp[0] & 0x0f000000) >> 24) == 0x4)
						sprintf(wpa2_tkip_aes,"%s","aes");
					else if (((pBss->bdTstamp[0] & 0x0f000000) >> 24) == 0x1)
						sprintf(wpa2_tkip_aes,"%s","tkip");
				}
			}
		}



		nBytesSent += boaWrite(wp, "<tr>"
			"<td align=left width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%d%s</td>\n"
      			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%d</td>\n",
			ssidbuf, tmpBuf, pBss->ChannelNumber,tmp1Buf,
			((pBss->bdCap & cIBSS) ? "Ad hoc" : "AP"),
			tmp2Buf, pBss->rssi);

		if ( mode == CLIENT_MODE 
#ifdef WLAN_UNIVERSAL_REPEATER
		|| rpt_enabled
#endif
		)
			nBytesSent += boaWrite(wp,
			//"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><input type=\"radio\" name="
			//"\"select\" value=\"sel%d\" onClick=\"enableConnect()\"></td></tr>\n", i);
			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><input type=\"hidden\" id=\"selSSID_%d\" value=\"%s\" > <input type=\"hidden\" id=\"selChannel_%d\" value=\"%d\" > <input type=\"hidden\" id=\"selEncrypt_%d\" value=\"%s\" > <input type=\"hidden\" id=\"wpa_tkip_aes_%d\" value=\"%s\" > <input type=\"hidden\" id=\"wpa2_tkip_aes_%d\" value=\"%s\" > <input type=\"radio\" name="
			"\"select\" value=\"sel%d\" onClick=\"enableConnect(%d)\"></td></tr>\n", i,tmpBuf,i,pBss->ChannelNumber,i,tmp2Buf,i,wpa_tkip_aes,i,wpa2_tkip_aes ,i,i);
		else
			nBytesSent += boaWrite(wp, "</tr>\n");
	}

	return nBytesSent;
}
#endif	// of WLAN_CLIENT


#ifdef WLAN_WDS
/////////////////////////////////////////////////////////////////////////////
void formWlWds(request * wp, char *path, char *query)
{
	char *strAddMac, *strDelMac, *strDelAllMac, *strVal, *submitUrl, *strEnabled, *strSet, *strRate;
	char tmpBuf[100];
	int  i,idx;
	WDS_T macEntry;
	WDS_T Entry;
	unsigned char entryNum,enabled,delNum=0;

	strSet = boaGetVar(wp, "wdsSet", "");
	strAddMac = boaGetVar(wp, "addWdsMac", "");
	strDelMac = boaGetVar(wp, "deleteSelWdsMac", "");
	strDelAllMac = boaGetVar(wp, "deleteAllWdsMac", "");
	strEnabled = boaGetVar(wp, "wlanWdsEnabled", "");

#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
	strVal = boaGetVar(wp, "wlan_idx", "");
	if ( strVal[0] ) {
		printf("wlan_idx=%d\n", strVal[0]-'0');
		wlan_idx = strVal[0]-'0';
	}
#endif //CONFIG_RTL_92D_SUPPORT || WLAN_DUALBAND_CONCURRENT

	if (strSet[0]) {
		if (!gstrcmp(strEnabled, "ON")){
			enabled = 1;
		}
		else
			enabled = 0;
		if (mib_set( MIB_WLAN_WDS_ENABLED, (void *)&enabled) == 0) {
  			strcpy(tmpBuf, strSetEnableErr);
			goto setErr_wds;
		}
	}

	if (strAddMac[0]) {
		int intVal;
		/*if ( !gstrcmp(strEnabled, "ON")){
			enabled = 1;
		}
		else
			enabled = 0;
		if ( mib_set( MIB_WLAN_WDS_ENABLED, (void *)&enabled) == 0) {
  			strcpy(tmpBuf, strSetEnableErr);
			goto setErr_wds;
		}*/
		strVal = boaGetVar(wp, "mac", "");
		if ( !strVal[0] )
			goto setWds_ret;

		if (strlen(strVal)!=12 || !string_to_hex(strVal, macEntry.macAddr, 12)) {
			strcpy(tmpBuf, strInvdMACAddr);
			goto setErr_wds;
		}
		if (!isValidMacAddr(macEntry.macAddr)) {
			strcpy(tmpBuf, strInvdMACAddr);
			goto setErr_wds;
		}

		strVal = boaGetVar(wp, "comment", "");
		if ( strVal[0] ) {
			if (strlen(strVal) > COMMENT_LEN-1) {
				strcpy(tmpBuf, strCommentTooLong);
				goto setErr_wds;
			}
			strcpy(macEntry.comment, strVal);
		}
		else
			macEntry.comment[0] = '\0';
		strRate = boaGetVar(wp, "txRate", "");
		if ( strRate[0] ) {
			if ( strRate[0] == '0' ) { // auto
				macEntry.fixedTxRate =0;
			}else  {
				intVal = atoi(strRate);
				intVal = 1 << (intVal-1);
				macEntry.fixedTxRate = intVal;
			}
		}

		if ( !mib_get(MIB_WLAN_WDS_NUM, (void *)&entryNum)) {
			strcpy(tmpBuf, strGetEntryNumErr);
			goto setErr_wds;
		}
		if ( (entryNum + 1) > MAX_WDS_NUM) {
			strcpy(tmpBuf, strErrForTablFull);
			goto setErr_wds;
		}

		// Jenny added, set to MIB. Check if entry exists
		for (i=0; i<entryNum; i++) {
			if (!mib_chain_get(MIB_WDS_TBL, i, (void *)&Entry)) {
	  			boaError(wp, 400, "Get chain record error!\n");
				return;
			}
			if (!memcmp(macEntry.macAddr, Entry.macAddr, 6)) {
				strcpy(tmpBuf, strMACInList);
				goto setErr_wds;
			}
		}

		// set to MIB. try to delete it first to avoid duplicate case
		//mib_set(MIB_WLAN_WDS_DEL, (void *)&macEntry);
		intVal = mib_chain_add(MIB_WDS_TBL, (void *)&macEntry);
		if (intVal == 0) {
			strcpy(tmpBuf, strAddEntryErr);
			goto setErr_wds;
		}
		else if (intVal == -1) {
			strcpy(tmpBuf, strTableFull);
			goto setErr_wds;
		}
		entryNum++;
		if ( !mib_set(MIB_WLAN_WDS_NUM, (void *)&entryNum)) {
			strcpy(tmpBuf, strGetEntryNumErr);
			goto setErr_wds;
		}
	}

	/* Delete entry */
	delNum=0;
	if (strDelMac[0]) {
		if ( !mib_get(MIB_WLAN_WDS_NUM, (void *)&entryNum)) {
			strcpy(tmpBuf, strGetEntryNumErr);
			goto setErr_wds;
		}
		for (i=0; i<entryNum; i++) {

			idx = entryNum-i-1;
			snprintf(tmpBuf, 20, "select%d", idx);
			strVal = boaGetVar(wp, tmpBuf, "");

			if ( !gstrcmp(strVal, "ON") ) {
				if(mib_chain_delete(MIB_WDS_TBL, idx) != 1) {
					strcpy(tmpBuf, strDelRecordErr);
				}
				delNum++;
			}
		}
		entryNum-=delNum;
		if ( !mib_set(MIB_WLAN_WDS_NUM, (void *)&entryNum)) {
			strcpy(tmpBuf, strGetEntryNumErr);
			goto setErr_wds;
		}
		if (delNum <= 0) {
			strcpy(tmpBuf, multilang(LANG_THERE_IS_NO_ITEM_SELECTED_TO_DELETE));
			goto setErr_wds;
		}
	}

	/* Delete all entry */
	if ( strDelAllMac[0]) {
		mib_chain_clear(MIB_WDS_TBL); /* clear chain record */

		entryNum=0;
		if ( !mib_set(MIB_WLAN_WDS_NUM, (void *)&entryNum)) {
			strcpy(tmpBuf, strGetEntryNumErr);
			goto setErr_wds;
		}

	}

setWds_ret:
	config_WLAN(ACT_RESTART);
// Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif // of #if COMMIT_IMMEDIATELY

#ifndef NO_ACTION
	run_script(-1);
#endif

	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page
	OK_MSG( submitUrl );
	return;

setErr_wds:
	ERR_MSG(tmpBuf);
}

void formWdsEncrypt(request * wp, char *path, char *query)
{
	char *strVal, *submitUrl;
	unsigned char tmpBuf[100];
	unsigned char encrypt;
	unsigned char intVal, keyLen=0, oldFormat, oldPskLen, len, i;
	char charArray[16]={'0' ,'1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
	char key[100];
	char varName[20];
	sprintf(varName, "encrypt%d", wlan_idx);
	strVal = boaGetVar(wp, varName, "");
	if (strVal[0]) {
		encrypt = strVal[0] - '0';
		if (encrypt != WDS_ENCRYPT_DISABLED && encrypt != WDS_ENCRYPT_WEP64 &&
			encrypt != WDS_ENCRYPT_WEP128 && encrypt != WDS_ENCRYPT_TKIP &&
				encrypt != WDS_ENCRYPT_AES) {
			strcpy(tmpBuf, multilang(LANG_ENCRYPT_VALUE_NOT_VALID));
			goto setErr_wdsEncrypt;
		}
		if ( !mib_set(MIB_WLAN_WDS_ENCRYPT, (void *)&encrypt)) {
			strcpy(tmpBuf, strGetEntryNumErr);
			goto setErr_wdsEncrypt;
		}
	}
	else{
		if ( !mib_get(MIB_WLAN_WDS_ENCRYPT, (void *)&encrypt)) {
			strcpy(tmpBuf, strGetEntryNumErr);
			goto setErr_wdsEncrypt;
		}
	}
	if (encrypt == WDS_ENCRYPT_WEP64 || encrypt == WDS_ENCRYPT_WEP128) {
		sprintf(varName, "format%d", wlan_idx);
		strVal = boaGetVar(wp, varName, "");
		if (strVal[0]) {
			if (strVal[0]!='0' && strVal[0]!='1') {
				strcpy(tmpBuf, multilang(LANG_INVALID_WEP_KEY_FORMAT_VALUE));
				goto setErr_wdsEncrypt;
			}
			intVal = strVal[0] - '0';
			if ( !mib_set(MIB_WLAN_WDS_WEP_FORMAT, (void *)&intVal)) {
				strcpy(tmpBuf, strGetEntryNumErr);
				goto setErr_wdsEncrypt;
			}
		}
		else{
			if ( !mib_get(MIB_WLAN_WDS_WEP_FORMAT, (void *)&intVal)) {
				strcpy(tmpBuf, strGetEntryNumErr);
				goto setErr_wdsEncrypt;
			}
		}
		if (encrypt == WDS_ENCRYPT_WEP64)
			keyLen = WEP64_KEY_LEN;
		else if (encrypt == WDS_ENCRYPT_WEP128)
			keyLen = WEP128_KEY_LEN;
		if (intVal == 1) // hex
			keyLen <<= 1;
		sprintf(varName, "wepKey%d", wlan_idx);
		strVal = boaGetVar(wp, varName, "");
		if(strVal[0]) {
			if (strlen(strVal) != keyLen) {
				strcpy(tmpBuf, multilang(LANG_INVALID_WEP_KEY_LENGTH));
				goto setErr_wdsEncrypt;
			}
			if ( !isAllStar(strVal) ) {
				if (intVal == 0) { // ascii
					for (i=0; i<keyLen; i++) {
						key[i*2] = charArray[(strVal[i]>>4)&0xf];
						key[i*2+1] = charArray[strVal[i]&0xf];
					}
					key[i*2] = '\0';
				}
				else  // hex
					strcpy(key, strVal);
				if ( !mib_set(MIB_WLAN_WDS_WEP_KEY, (void *)key)) {
					strcpy(tmpBuf, strGetEntryNumErr);
					goto setErr_wdsEncrypt;
				}
			}
		}
	}
	if (encrypt == WDS_ENCRYPT_TKIP || encrypt == WDS_ENCRYPT_AES) {
		sprintf(varName, "pskFormat%d", wlan_idx);
		strVal = boaGetVar(wp, varName, "");
		if (strVal[0]) {
			if (strVal[0]!='0' && strVal[0]!='1') {
				strcpy(tmpBuf, multilang(LANG_INVALID_WEP_KEY_FORMAT_VALUE));
				goto setErr_wdsEncrypt;
			}
			intVal = strVal[0] - '0';
		}
		else{
			if ( !mib_get(MIB_WLAN_WDS_PSK_FORMAT, (void *)&intVal)) {
				strcpy(tmpBuf, strGetEntryNumErr);
				goto setErr_wdsEncrypt;
			}
		}
		if ( !mib_get(MIB_WLAN_WDS_PSK_FORMAT, (void *)&oldFormat)) {
			strcpy(tmpBuf, strGetEntryNumErr);
			goto setErr_wdsEncrypt;
		}
		if ( !mib_get(MIB_WLAN_WDS_PSK, (void *)tmpBuf)) {
			strcpy(tmpBuf, strGetEntryNumErr);
			goto setErr_wdsEncrypt;
		}
		oldPskLen = strlen(tmpBuf);
		sprintf(varName, "pskValue%d", wlan_idx);
		strVal = boaGetVar(wp, varName, "");
		len = strlen(strVal);
		if (len > 0 && oldFormat == intVal && len == oldPskLen ) {
			for (i=0; i<len; i++) {
				if ( strVal[i] != '*' )
				break;
			}
			if (i == len)
				goto save_wdsEcrypt;
		}
		if (intVal==1) { // hex
			if (len!=MAX_PSK_LEN || !string_to_hex(strVal, tmpBuf, MAX_PSK_LEN)) {
				strcpy(tmpBuf, multilang(LANG_ERROR_INVALID_PSK_VALUE));
				goto setErr_wdsEncrypt;
			}
		}
		else { // passphras
			if (len==0 || len > (MAX_PSK_LEN-1) ) {
				strcpy(tmpBuf, multilang(LANG_ERROR_INVALID_PSK_VALUE));
				goto setErr_wdsEncrypt;
			}
		}
		if ( !mib_set(MIB_WLAN_WDS_PSK_FORMAT, (void *)&intVal)) {
			strcpy(tmpBuf, strGetEntryNumErr);
			goto setErr_wdsEncrypt;
		}
		if ( !mib_set(MIB_WLAN_WDS_PSK, (void *)strVal)) {
			strcpy(tmpBuf, strGetEntryNumErr);
			goto setErr_wdsEncrypt;
		}
	}
save_wdsEcrypt:
	{
		unsigned char enable = 1;
		if ( !mib_set(MIB_WLAN_WDS_ENABLED, (void *)&enable)) {
			strcpy(tmpBuf, strGetEntryNumErr);
			goto setErr_wdsEncrypt;
		}
	}
	config_WLAN(ACT_RESTART);
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif // of #if COMMIT_IMMEDIATELY
	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page
	OK_MSG(submitUrl);
	return;
setErr_wdsEncrypt:
	ERR_MSG(tmpBuf);
}

/////////////////////////////////////////////////////////////////////////////
int wlWdsList(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0, i;
	WDS_T entry;
	char tmpBuf[100];
	char txrate[20];
	unsigned char entryNum;
	WDS_T Entry;
	int rateid = 0;

	if ( !mib_get(MIB_WLAN_WDS_NUM, (void *)&entryNum)) {
  		boaError(wp, 400, "Get table entry error!\n");
		return -1;
	}

#ifdef WLAN_QTN
#ifdef WLAN1_QTN
	if(wlan_idx==1)
#else
	if(wlan_idx==0)
#endif
	{
	//modified by xl_yue
		nBytesSent += boaWrite(wp, "<tr>"
			"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>%s</b></font></td>\n"
			"<td align=center width=\"80%%\" bgcolor=\"#808080\"><font size=\"2\"><b>%s</b></font></td></tr>\n",
		multilang(LANG_SELECT), multilang(LANG_MAC_ADDRESS));
	
		for (i=0; i<entryNum; i++) {
			*((char *)&entry) = (char)i;
			if (!mib_chain_get(MIB_WDS_TBL, i, (void *)&Entry)) {
				boaError(wp, 400, errGetEntry);
				return -1;
			}
			snprintf(tmpBuf, 100, "%02x:%02x:%02x:%02x:%02x:%02x",
				Entry.macAddr[0], Entry.macAddr[1], Entry.macAddr[2],
				Entry.macAddr[3], Entry.macAddr[4], Entry.macAddr[5]);
	
	
			nBytesSent += boaWrite(wp, "<tr>"
				"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td>\n"
				"<td align=center width=\"80%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td></tr>\n",
				i, tmpBuf);
		}

	}
	else
#endif

{
//modified by xl_yue
	nBytesSent += boaWrite(wp, "<tr>"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>%s</b></font></td>\n"
      	"<td align=center width=\"45%%\" bgcolor=\"#808080\"><font size=\"2\"><b>%s</b></font></td>\n"
      	"<td align=center width=\"25%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Tx Rate (Mbps)</b></font></td>\n"
      	"<td align=center width=\"35%%\" bgcolor=\"#808080\"><font size=\"2\"><b>%s</b></font></td></tr>\n",
	multilang(LANG_SELECT), multilang(LANG_MAC_ADDRESS), multilang(LANG_COMMENT));

	for (i=0; i<entryNum; i++) {
		*((char *)&entry) = (char)i;
		if (!mib_chain_get(MIB_WDS_TBL, i, (void *)&Entry)) {
  			boaError(wp, 400, errGetEntry);
			return -1;
		}
		snprintf(tmpBuf, 100, "%02x:%02x:%02x:%02x:%02x:%02x",
			Entry.macAddr[0], Entry.macAddr[1], Entry.macAddr[2],
			Entry.macAddr[3], Entry.macAddr[4], Entry.macAddr[5]);

		//strcpy(txrate, "N/A");
		if(Entry.fixedTxRate == 0){
			sprintf(txrate, "%s","Auto");
		}
		else{
			for(rateid=0; rateid<28;rateid++){
				if(tx_fixed_rate[rateid].id == Entry.fixedTxRate){
					sprintf(txrate, "%s", tx_fixed_rate[rateid].rate);
					break;
				}
			}
		}

		nBytesSent += boaWrite(wp, "<tr>"
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td>\n"
			"<td align=center width=\"45%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center width=\"25%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center width=\"35%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td></tr>\n",
			i, tmpBuf, txrate, Entry.comment);
	}
}
	return nBytesSent;
}

int wdsList(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0, i;
	WDS_INFO_Tp pInfo;
	char *buff;

#ifdef WLAN_QTN
#ifdef WLAN1_QTN
		if(wlan_idx==1){
#else
		if(wlan_idx==0){
#endif
			unsigned int sta_num;
			unsigned int tx_pkt, rx_pkt, tx_phy_rate, tx_err_pkt;
			unsigned char mac_addr[6];
			unsigned char ifname[16];
			unsigned char wds_num;
			mib_get(MIB_WLAN_WDS_NUM, &wds_num);

			for (i=0; i<wds_num; i++) {
				snprintf(ifname, 16, "wds%d", i);
				rt_qcsapi_get_wds_peer_mac_addr("wifi0", i, mac_addr);
				rt_qcsapi_get_sta_tx_pkt(ifname, 0, &tx_pkt);
				rt_qcsapi_get_sta_tx_err_pkt(ifname, 0, &tx_err_pkt);
				rt_qcsapi_get_sta_rx_pkt(ifname, 0, &rx_pkt);
				rt_qcsapi_get_sta_tx_phy_rate(ifname, 0, &tx_phy_rate);
				nBytesSent += boaWrite(wp,
			   		"<tr bgcolor=#b7b7b7><td><font size=2>%02x:%02x:%02x:%02x:%02x:%02x</td>"
					"<td><font size=2>%d</td>"
			     		"<td><font size=2>%d</td>"
					"<td><font size=2>%d</td>"
					"<td><font size=2>%d%s</td></tr>",
					mac_addr[0],mac_addr[1],mac_addr[2],mac_addr[3],mac_addr[4],mac_addr[5],
					tx_pkt, tx_err_pkt, rx_pkt,	tx_phy_rate);
			}
			return nBytesSent;
		}
#endif


	buff = calloc(1, sizeof(WDS_INFO_T)*MAX_WDS_NUM);
	if ( buff == 0 ) {
		printf("Allocate buffer failed!\n");
		return 0;
	}

	if ( getWdsInfo(getWlanIfName(), buff) < 0 ) {
		printf("Read wlan sta info failed!\n");
		return 0;
	}

	for (i=0; i<MAX_WDS_NUM; i++) {
		pInfo = (WDS_INFO_Tp)&buff[i*sizeof(WDS_INFO_T)];

		if (pInfo->state == STATE_WDS_EMPTY)
			break;

		nBytesSent += boaWrite(wp,
	   		"<tr bgcolor=#b7b7b7><td><font size=2>%02x:%02x:%02x:%02x:%02x:%02x</td>"
			"<td><font size=2>%d</td>"
	     		"<td><font size=2>%d</td>"
			"<td><font size=2>%d</td>"
			"<td><font size=2>%d%s</td></tr>",
			pInfo->addr[0],pInfo->addr[1],pInfo->addr[2],pInfo->addr[3],pInfo->addr[4],pInfo->addr[5],
			pInfo->tx_packets, pInfo->tx_errors, pInfo->rx_packets,
			pInfo->txOperaRate/2, ((pInfo->txOperaRate%2) ? ".5" : "" ));
	}

	free(buff);

	return nBytesSent;
}
#endif // WLAN_WDS

#ifdef CONFIG_WIFI_SIMPLE_CONFIG // WPS
#define _WSC_DAEMON_PROG       "/bin/wscd"
#define WLAN_IF  "wlan0"
#define OK_MSG1(msg, url) { \
        boaHeader(wp); \
        boaWrite(wp, "<body><blockquote><h4>%s</h4>\n", msg); \
        if (url) boaWrite(wp, "<form><input type=button value=\"  OK  \" OnClick=window.location.replace(\"%s\")></form></blockquote></body>", url);\
        else boaWrite(wp, "<form><input type=button value=\"  OK  \" OnClick=window.close()></form></blockquote></body>");\
        boaFooter(wp); \
        boaDone(wp, 200); \
}

#define OK_MSG2(msg, msg1, url) { \
        char tmp[200]; \
        sprintf(tmp, msg, msg1); \
        OK_MSG1(tmp, url); \
}
#define START_PBC_MSG \
        "Start PBC successfully!<br><br>" \
        "You have to run Wi-Fi Protected Setup in %s within 2 minutes."
#define START_PIN_MSG \
        "Start PIN successfully!<br><br>" \
        "You have to run Wi-Fi Protected Setup in %s within 2 minutes."
#define SET_PIN_MSG \
        "Applied client's PIN successfully!<br><br>" \
        "You have to run Wi-Fi Protected Setup in client within 2 minutes."
/*for WPS2DOTX brute force attack , unlock*/
#define UNLOCK_MSG \
	"Applied WPS unlock successfully!<br>"

void formWsc(request * wp, char *path, char *query)
{
	char *strVal, *submitUrl;
	char intVal;
	char tmpbuf[200];
//	int mode;
	unsigned char mode;
	MIB_CE_MBSSIB_T Entry;


#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
	strVal = boaGetVar(wp, "wlan_idx", "");
	if ( strVal[0] ) {
		printf("wlan_idx=%d\n", strVal[0]-'0');
		wlan_idx = strVal[0]-'0';
	}
#endif //CONFIG_RTL_92D_SUPPORT || WLAN_DUALBAND_CONCURRENT
	wlan_getEntry((void *)&Entry, 0);
	mode = Entry.wlanMode;
	submitUrl = boaGetVar(wp, "submit-url", "");

	// for PIN brute force attack
	strVal = boaGetVar(wp, "unlockautolockdown", "");
	if (strVal[0]) {
		va_cmd(_WSC_DAEMON_PROG, 1, 1, "-sig_unlock");
		OK_MSG2(UNLOCK_MSG, ((mode==AP_MODE) ? "client" : "AP"), submitUrl);
		return;
	}

	strVal = boaGetVar(wp, "triggerPBC", "");
	if (strVal[0]) {
		if (Entry.wsc_disabled) {
			Entry.wsc_disabled = 0;
			wlan_setEntry((void *)&Entry, 0);
			mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);	// update to flash
			system("echo 1 > /var/wps_start_pbc");
#ifndef NO_ACTION
			run_init_script("bridge");
#endif
		}
		else {
			//sprintf(tmpbuf, "%s -sig_pbc", _WSC_DAEMON_PROG);
			//system(tmpbuf);
			//va_cmd(_WSC_DAEMON_PROG, 1, 1, "-sig_pbc");
			if(wlan_idx == 0 )
			{
				system("echo 1 > /var/wps_start_interface0");
			}
			else
			{
				system("echo 1 > /var/wps_start_interface1");

			}
#ifdef WLAN_QTN
#ifdef WLAN1_QTN
			if(wlan_idx==1){
#else
			if(wlan_idx==0){
#endif
				va_cmd("/bin/qcsapi_sockrpc", 2, 1, "registrar_report_pbc", rt_get_qtn_ifname(getWlanIfName()));
			}
			else
#endif
			va_cmd(_WSC_DAEMON_PROG, 2 , 1 , "-sig_pbc" , getWlanIfName());
		}
		OK_MSG2(START_PBC_MSG, ((mode==AP_MODE) ? "client" : "AP"), submitUrl);
		return;
	}

	strVal = boaGetVar(wp, "triggerPIN", "");
	if (strVal[0]) {
		int local_pin_changed = 0;
		strVal = boaGetVar(wp, "localPin", "");
		if (strVal[0]) {
			mib_get(MIB_WSC_PIN, (void *)tmpbuf);
			if (strcmp(tmpbuf, strVal)) {
				mib_set(MIB_WSC_PIN, (void *)strVal);
				local_pin_changed = 1;
			}
		}
		if (Entry.wsc_disabled) {
			char localpin[100];
			Entry.wsc_disabled = 0;
			wlan_setEntry((void *)&Entry, 0);
			mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);	// update to flash
			system("echo 1 > /var/wps_start_pin");

#ifndef NO_ACTION
			if (local_pin_changed) {
				mib_get(MIB_WSC_PIN, (void *)localpin);
				sprintf(tmpbuf, "echo %s > /var/wps_local_pin", localpin);
				system(tmpbuf);
			}
			run_init_script("bridge");
#endif
		}
		else {
			if (local_pin_changed) {
				system("echo 1 > /var/wps_start_pin");

				mib_update(CURRENT_SETTING,CONFIG_MIB_ALL);
				//run_init_script("bridge");
			}
			else {
				sprintf(tmpbuf, "%s -sig_start", _WSC_DAEMON_PROG);
				system(tmpbuf);
			}
		}
		OK_MSG2(START_PIN_MSG, ((mode==AP_MODE) ? "client" : "AP"), submitUrl);
		return;
	}

	strVal = boaGetVar(wp, "setPIN", "");
	if (strVal[0]) {
		strVal = boaGetVar(wp, "peerPin", "");
		if (strVal[0]) {
			if (Entry.wsc_disabled) {
				Entry.wsc_disabled = 0;
				wlan_setEntry((void *)&Entry, 0);
				mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);

				sprintf(tmpbuf, "echo %s > /var/wps_peer_pin", strVal);
				system(tmpbuf);

#ifndef NO_ACTION
				run_init_script("bridge");
#endif
			}
			else {
				//sprintf(tmpbuf, "iwpriv %s set_mib pin=%s", WLAN_IF, strVal);
				//system(tmpbuf);
#ifdef WLAN_QTN
#ifdef WLAN1_QTN
			if(wlan_idx==1){
#else
			if(wlan_idx==0){
#endif
				sprintf(tmpbuf, "%s", strVal);
				va_cmd("/bin/qcsapi_sockrpc", 3, 1, "registrar_report_pin", rt_get_qtn_ifname(getWlanIfName()), tmpbuf);
			}
			else
#endif
			{
				if(wlan_idx == 0 )
				{
					system("echo 1 > /var/wps_start_interface0");
				}
				else
				{
					system("echo 1 > /var/wps_start_interface1");
				}
				sprintf(tmpbuf, "pin=%s", strVal);
				va_cmd("/bin/iwpriv", 3, 1, getWlanIfName(), "set_mib", tmpbuf);
			}
			}
			OK_MSG1(SET_PIN_MSG, submitUrl);
			return;
		}
	}

#ifdef WPS20
#ifdef WPS_VERSION_CONFIGURABLE
	strVal = boaGetVar(wp, "wpsUseVersion", "");
	intVal = atoi(strVal);
#else
	intVal = 1;
#endif
#else
	intVal = 0;
#endif
	mib_set(MIB_WSC_VERSION, (void *)&intVal);

	strVal = boaGetVar(wp, "disableWPS", "");
	if ( !gstrcmp(strVal, "ON"))
		intVal = 1;
	else
		intVal = 0;
	Entry.wsc_disabled = intVal;
	//mib_set(MIB_WSC_DISABLE, &Entry.wsc_disabled);
	wlan_setEntry((void *)&Entry, 0);
	update_wps_mib();

	strVal = boaGetVar(wp, "localPin", "");
	if (strVal[0]){
		mib_set(MIB_WSC_PIN, (void *)strVal);
#ifdef WLAN_QTN //only have one wps ap pin now
		//if(wlan_idx==1){
			va_cmd("/bin/qcsapi_sockrpc", 3, 1, "set_wps_ap_pin", rt_get_qtn_ifname(getWlanIfName()), strVal);
			va_cmd("/bin/qcsapi_sockrpc", 2, 1, "save_wps_ap_pin", rt_get_qtn_ifname(getWlanIfName()));
		//}
#endif
	}

//	update_wps_configured(0);
	config_WLAN(ACT_RESTART);

#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif // of #if COMMIT_IMMEDIATELY

#ifndef NO_ACTION
	run_init_script("bridge");
#endif

	OK_MSG(submitUrl);
}
#endif

#ifdef WLAN_11R
extern char FT_DAEMON_PROG[];
void formFt(request * wp, char *path, char *query)
{
	char *strVal, *submitUrl;
	char tmpbuf[200];
	MIB_CE_MBSSIB_T Entry;
	MIB_CE_WLAN_FTKH_T khEntry, getEntry;
	int idx, i, entryNum, intVal, deleted=0;

	submitUrl = boaGetVar(wp, "submit-url", "");
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
	strVal = boaGetVar(wp, "wlan_idx", "");
	if ( strVal[0] ) {
		printf("wlan_idx=%d\n", strVal[0]-'0');
		wlan_idx = strVal[0]-'0';
	}
#endif //CONFIG_RTL_92D_SUPPORT || WLAN_DUALBAND_CONCURRENT

	// check which interface is selected
	strVal = boaGetVar(wp, "ftSSID", "");
	if (strVal[0]) {
		idx = strVal[0]-'0';
		if (idx<0 || idx > NUM_VWLAN_INTERFACE) {
			strcpy(tmpbuf, strNotSuptSSIDType);
			goto setErr_ft;
		}
	} else {
		strcpy(tmpbuf, strNoSSIDTypeErr);
		goto setErr_ft;
	}

	if (!wlan_getEntry((void *)&Entry, idx)) {
		strcpy(tmpbuf, strGetMBSSIBTBLErr);
		goto setErr_ft;
	}

	// for driver configurateion
	strVal = boaGetVar(wp, "ftSaveConfig", "");
	if (strVal[0]) {
		// 802.11r related settings
		strVal = boaGetVar(wp, "ft_enable", "");
		if (strVal[0])
			Entry.ft_enable = atoi(strVal);

		strVal = boaGetVar(wp, "ft_mdid", "");
		if (strVal[0])
			strncpy(Entry.ft_mdid, strVal, 4);

		strVal = boaGetVar(wp, "ft_over_ds", "");
		if (strVal[0])
			Entry.ft_over_ds = atoi(strVal);

		strVal = boaGetVar(wp, "ft_res_request", "");
		if (strVal[0])
			Entry.ft_res_request = atoi(strVal);

		strVal = boaGetVar(wp, "ft_r0key_timeout", "");
		if (strVal[0])
			Entry.ft_r0key_timeout = atoi(strVal);

		strVal = boaGetVar(wp, "ft_reasoc_timeout", "");
		if (strVal[0])
			Entry.ft_reasoc_timeout = atoi(strVal);

		strVal = boaGetVar(wp, "ft_r0kh_id", "");
		if (strVal[0])
			strncpy(Entry.ft_r0kh_id, strVal, 48);

		strVal = boaGetVar(wp, "ft_push", "");
		if (strVal[0])
			Entry.ft_push = atoi(strVal);

		// save changes
		wlan_setEntry(&Entry, idx);
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, idx);
		config_WLAN(ACT_RESTART);
		goto setOk_ft;
	}

	// for R0KH/R1KH configuration
	/* Add entry */
	strVal = boaGetVar(wp, "ftAddKH", "");
	if (strVal[0]) {
		if ( Entry.ft_kh_num >= MAX_VWLAN_FTKH_NUM ) {
			strcpy(tmpbuf, strAddAcErrForFull);
			goto setErr_ft;
		}

		memset(&khEntry, 0, sizeof(khEntry));
		strVal = boaGetVar(wp, "kh_mac", "");
		if (!strVal[0]) {
			strcpy(tmpbuf, multilang(LANG_ERROR_NO_MAC_ADDRESS_TO_SET));
			goto setErr_ft;
		}
		if (strlen(strVal)!=12 || !string_to_hex(strVal, khEntry.addr, 12)) {
			strcpy(tmpbuf, strInvdMACAddr);
			goto setErr_ft;
		}
		if (!isValidMacAddr(khEntry.addr)) {
			strcpy(tmpbuf, strInvdMACAddr);
			goto setErr_ft;
		}
		khEntry.wlanIdx = wlan_idx;
		khEntry.intfIdx = idx;

		strVal = boaGetVar(wp, "kh_nas_id", "");
		if (!strVal[0]) {
			strcpy(tmpbuf, multilang(LANG_INVALID_NAS_IDENTIFIER_1_48_CHARACTERS));
			goto setErr_ft;
		}
		strncpy(khEntry.r0kh_id, strVal, 48);

		strVal = boaGetVar(wp, "kh_kek", "");
		if (!strVal[0]) {
			strcpy(tmpbuf, multilang(LANG_ERROR_NO_R0KH_R1KH_KEY_TO_SET));
			goto setErr_ft;
		}
		strncpy(khEntry.key, strVal, 32);

		entryNum = mib_chain_total(MIB_WLAN_FTKH_TBL);

		// set to MIB. Check if entry exists
		for (i=0; i<entryNum; i++) {
			if (!mib_chain_get(MIB_WLAN_FTKH_TBL, i, (void *)&getEntry))
			{
	  			strcpy(tmpbuf, strGetChainerror);
				goto setErr_ft;
			}
			if (!memcmp(khEntry.addr, getEntry.addr, 6) && khEntry.intfIdx==getEntry.intfIdx)
			{
				strcpy(tmpbuf, strMACInList);
				goto setErr_ft;
			}
		}

		// add new KH entry
		intVal = mib_chain_add(MIB_WLAN_FTKH_TBL, (unsigned char *)&khEntry);
		if (intVal == 0) {
			strcpy(tmpbuf, strAddListErr);
			goto setErr_ft;
		}
		else if (intVal == -1) {
			strcpy(tmpbuf, strTableFull);
			goto setErr_ft;
		}

		// save entry count
		Entry.ft_kh_num++;
		wlan_setEntry(&Entry, idx);
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, idx);

		// generate new ft.conf and update to FT daemon
		genFtKhConfig();
		va_cmd(FT_DAEMON_PROG, 1 , 1 , "-update");
		goto setOk_ft;
	}

	/* Delete selected entry */
	strVal = boaGetVar(wp, "ftDelSelKh", "");
	if (strVal[0]) {
		entryNum = mib_chain_total(MIB_WLAN_FTKH_TBL);
		for (i=entryNum-1; i>=0; i--) {
			if (!mib_chain_get(MIB_WLAN_FTKH_TBL, i, (void *)&khEntry))
				break;
			if(khEntry.wlanIdx != wlan_idx)
				continue;

			snprintf(tmpbuf, 20, "kh_entry_%d", i);
			strVal = boaGetVar(wp, tmpbuf, "");

			if (!strcmp(strVal, "ON")) {
				deleted++;
				if(mib_chain_delete(MIB_WLAN_FTKH_TBL, i) != 1) {
					strcpy(tmpbuf, strDelListErr);
					goto setErr_ft;
				}
			}
		}
		if (deleted <= 0) {
			strcpy(tmpbuf, multilang(LANG_THERE_IS_NO_ITEM_SELECTED_TO_DELETE));
			goto setErr_ft;
		}

		// save entry count
		Entry.ft_kh_num--;
		wlan_setEntry(&Entry, idx);
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, idx);

		// generate new ft.conf and update to FT daemon
		genFtKhConfig();
		va_cmd(FT_DAEMON_PROG, 1 , 1 , "-clear");
		va_cmd(FT_DAEMON_PROG, 1 , 1 , "-update");
		goto setOk_ft;
	}

	/* Delete all entry */
	strVal = boaGetVar(wp, "ftDelAllKh", "");
	if (strVal[0]) {
		entryNum = mib_chain_total(MIB_WLAN_FTKH_TBL);
		for (i=entryNum-1; i>=0; i--) {
			if (!mib_chain_get(MIB_WLAN_FTKH_TBL, i, (void *)&khEntry)) {
	  			strcpy(tmpbuf, strGetChainerror);
				goto setErr_ft;
			}
			if(khEntry.wlanIdx == wlan_idx) {
				if(mib_chain_delete(MIB_WLAN_FTKH_TBL, i) != 1) {
					strcpy(tmpbuf, strDelListErr);
					goto setErr_ft;
				}
			}
		}

		// reset entry count
		for (i=0; i<=NUM_VWLAN_INTERFACE; i++) {
			if (!wlan_getEntry((void *)&Entry, i)) {
				strcpy(tmpbuf, strGetMBSSIBTBLErr);
				goto setErr_ft;
			}
			Entry.ft_kh_num = 0;
			wlan_setEntry(&Entry, i);
			mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, i);
		}

		// generate new ft.conf and update to FT daemon
		genFtKhConfig();
		va_cmd(FT_DAEMON_PROG, 1 , 1 , "-clear");
		goto setOk_ft;
	}

setOk_ft:
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif // of #if COMMIT_IMMEDIATELY
	OK_MSG(submitUrl);
	return;

setErr_ft:
	ERR_MSG(tmpbuf);
	return;
}

int wlFtKhList(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0, entryNum, i, j, intfIndex=-1;
	MIB_CE_MBSSIB_T Entry;
	MIB_CE_WLAN_FTKH_T khEntry;
	char strAddr[18], strId[49], strKey[33];

	// show title
	nBytesSent += boaWrite(wp, "<tr>"
      	"<td align=center width=\"14%%\" bgcolor=\"#808080\"><font size=\"2\"><b>%s</b></font></td>\n"
      	"<td align=center width=\"44%%\" bgcolor=\"#808080\"><font size=\"2\"><b>%s</b></font></td>\n"
      	"<td align=center width=\"30%%\" bgcolor=\"#808080\"><font size=\"2\"><b>%s</b></font></td>\n"
      	"<td align=center width=\"7%%\" bgcolor=\"#808080\"><font size=\"2\"><b>%s</b></font></td></tr>\n",
      	multilang(LANG_MAC_ADDRESS), multilang(LANG_NAS_IDENTIFIER),multilang(LANG_128_BIT_KEY_PASSPHRASE), multilang(LANG_SELECT));

	// get total count of KH entry
	entryNum = mib_chain_total(MIB_WLAN_FTKH_TBL);

	// list KH entries, in order of interface index
	for (j=0; j<=NUM_VWLAN_INTERFACE; j++) {
		for (i=0; i<entryNum; i++) {
			// get KH entries
			if (!mib_chain_get(MIB_WLAN_FTKH_TBL, i, (void *)&khEntry)) {
	  			boaError(wp, 400, "Get chain record error!\n");
				return -1;
			}
			if (khEntry.intfIdx != j)
				continue;

			// show SSID if
			if (intfIndex != j) {
				if (!wlan_getEntry((void *)&Entry, j)) {
					boaError(wp, 400, "Get chain record error!\n");
					return -1;
				}
				nBytesSent += boaWrite(wp, "<tr>"
					"<td align=left width=\"100%%\" colspan=\"4\" bgcolor=\"#A0A0A0\"><font size=\"2\"><b>%s</b></td></tr>\n",
					Entry.ssid);
				intfIndex = j;
			}

			// show content of KH entry
			snprintf(strAddr, sizeof(strAddr), "%02x:%02x:%02x:%02x:%02x:%02x",
				khEntry.addr[0], khEntry.addr[1], khEntry.addr[2],
				khEntry.addr[3], khEntry.addr[4], khEntry.addr[5]);
			snprintf(strId, sizeof(strId), "%s", khEntry.r0kh_id);
			snprintf(strKey, sizeof(strKey), "%s", khEntry.key);

			nBytesSent += boaWrite(wp, "<tr>"
				"<td align=center width=\"14%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
				"<td align=center width=\"44%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
				"<td align=center width=\"30%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
				"<td align=center width=\"7%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"kh_entry_%d\" value=\"ON\"></td></tr>\n",
					strAddr, strId, strKey, i);
		}
	}

	return nBytesSent;
}
#endif

