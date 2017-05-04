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
#include "wireless.h"


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

int wlStatus_parm(int eid, request * wp, int argc, char **argv)
{
	bss_info bss;
	int nBytesSent = 0, i, k;
	int orig_wlan_idx;
	MIB_CE_MBSSIB_T Entry, Entry2;
	struct _misc_data_ misc_data;
	unsigned char buffer[64], buffer2[64], translate_ssid[200];
	unsigned char band, autort, hiddenSSID, wlan_disabled, phyBandSelect;
	unsigned int fixedTxRate;

	wlan_getEntry(&Entry, 0);
	band = Entry.wlanBand;
	autort = Entry.rateAdaptiveEnabled;
	hiddenSSID = Entry.hidessid;
	wlan_disabled = Entry.wlanDisabled;
	fixedTxRate = Entry.fixedTxRate;
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN0_5G_SUPPORT) || defined(WLAN1_5G_SUPPORT)
	mib_get(MIB_WLAN_PHY_BAND_SELECT, &phyBandSelect);
#else //CONFIG_RTL_92D_SUPPORT
	phyBandSelect = PHYBAND_2G;
#endif //CONFIG_RTL_92D_SUPPORT
	orig_wlan_idx = wlan_idx;

	for (i = 0; i < NUM_WLAN_INTERFACE; i++) {
		wlan_idx = i;
		if (getWlBssInfo(getWlanIfName(), &bss) < 0) {
			wlan_idx = orig_wlan_idx;
			return -1;
		}
		getTranSSID(translate_ssid, bss.ssid);
		nBytesSent += boaWrite(wp,
				       "\tband[%d]=%d;\n\tssid_drv[%d]='%s';\n\twlanSsidAttr[%d]='%s';\n",
				       i, band, i, translate_ssid, i,
				       (hiddenSSID == 0) ? "Visual" : "Hidden");
		/* Encryption */
		getEncryption(&Entry, buffer);
		nBytesSent += boaWrite(wp, "\tencryptState[%d]='%s';\n",
				       i,
				       (Entry.encrypt ==
					0) ? INFO_DISABLED : INFO_ENABLED);

		nBytesSent += boaWrite(wp,
				       "\tchannel_drv[%d]='%d';\n",
				       i, bss.channel);
		nBytesSent += boaWrite(wp,
				       "\tbssid_drv[%d]='%02x:%02x:%02x:%02x:%02x:%02x';\n",
				       i, bss.bssid[0], bss.bssid[1],
				       bss.bssid[2], bss.bssid[3], bss.bssid[4],
				       bss.bssid[5]);

		nBytesSent += boaWrite(wp,
				       "\twlanDisabled[%d]=%d;\n",
				       i, wlan_disabled);

		struct iwreq wrq;
		int ret, idx;
#define RTL8185_IOCTL_GET_MIB	0x89f2
		idx = socket(AF_INET, SOCK_DGRAM, 0);
		strcpy(wrq.ifr_name, getWlanIfName());
		strcpy(buffer, "channel");
		wrq.u.data.pointer = (caddr_t) & buffer;
		wrq.u.data.length = 10;
		ret = ioctl(idx, RTL8185_IOCTL_GET_MIB, &wrq);
		close(idx);
		if (ret != -1)
			nBytesSent +=
			    boaWrite(wp, "\twlDefChannel[%d]=%d;\n", i,
				     buffer[wrq.u.data.length - 1]);
		else
			nBytesSent += boaWrite(wp, "\twlDefChannel[%d]='N/A'", i);

		nBytesSent += boaWrite(wp, "\ttxrate[%d]=%u;\n", i, fixedTxRate);

		nBytesSent += boaWrite(wp, "\tauto[%d]=%d;\n", i, autort);

		memset(&misc_data, 0, sizeof(struct _misc_data_));
		getMiscData(getWlanIfName(), &misc_data);
		nBytesSent += boaWrite(wp, "\trf_used[%d]=%u;\n", i, misc_data.mimo_tr_used);

		nBytesSent += boaWrite(wp,
				       "\tmssid_num=%d;\n", WLAN_MBSSID_NUM);
#ifdef WLAN_MBSSID
		/*-------------- VAP Interface ------------*/
		for (k = 0; k < WLAN_MBSSID_NUM; k++) {
			//wlan_idx = orig_wlan_idx;
			mib_chain_get(MIB_MBSSIB_TBL, WLAN_VAP_ITF_INDEX + k,
				      (void *)&Entry2);
			sprintf(buffer, "%s-vap%d", getWlanIfName(), k);
			if (getWlBssInfo(buffer, &bss) < 0)
				printf("getWlBssInfo failed\n");
			getTranSSID(translate_ssid, bss.ssid);
			nBytesSent += boaWrite(wp,
					       "\tmssid_ssid_drv[%d][%d]='%s';\n\tmssid_wlanSsidAttr[%d][%d]='%s';\n\tmssid_band[%d][%d]=%d;\n"
					       "\tmssid_disable[%d][%d]=%d;\n",
					       i, k, translate_ssid, i, k,
					       (Entry2.hidessid ==
						0) ? "Visual" : "Hidden", i, k,
					       Entry2.wlanBand, i, k,
					       Entry2.wlanDisabled);
			nBytesSent +=
			    boaWrite(wp,
				     "\tmssid_bssid_drv[%d][%d]='%02x:%02x:%02x:%02x:%02x:%02x';\n",
				     i, k, bss.bssid[0], bss.bssid[1],
				     bss.bssid[2], bss.bssid[3], bss.bssid[4],
				     bss.bssid[5]);
			/* VAP encryption */
			getEncryption(&Entry2, buffer2);
			nBytesSent +=
			    boaWrite(wp, "\tmssid_encryptState[%d][%d]='%s';\n",
				     i, k,
				     (Entry2.encrypt ==
				      0) ? INFO_DISABLED : INFO_ENABLED);
		}
#endif
		nBytesSent +=
		    boaWrite(wp, "\tBand2G5GSupport[%d]=%d;\n", i, phyBandSelect);
	}
	wlan_idx = orig_wlan_idx;

	return nBytesSent;
}

