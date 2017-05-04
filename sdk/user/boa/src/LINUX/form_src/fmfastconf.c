/*
Web server handler routines for fast configure
*/
#include<string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../webs.h"
#include "webform.h"
#include "../defs.h"

#include "utility.h"
#include "mib.h"


extern int wlan_idx;

void initPageWizard(int eid, request * wp, int argc, char **argv)
{
	MIB_CE_ATM_VC_T vcEntry;
	MIB_CE_MBSSIB_T wlanEntry;
	unsigned char disable;
	char ssid[MAX_SSID_LEN];
	int tempWlanIdx;

	if (!mib_chain_get(MIB_ATM_VC_TBL, 0, (void *)&vcEntry)) {
		printf("connection_1 is not existed.\n");
		return;
	}

	if (vcEntry.cmode != CHANNEL_MODE_PPPOE) {
		printf("connection_1 is not pppoe");
		
		boaWrite(wp, "\tdocument.formfastconf.username.value=\"\";\n");
		boaWrite(wp, "\tdocument.formfastconf.password.value=\"\";\n");
		boaWrite(wp, "\tdocument.formfastconf.usestaticip.checked=false;\n");
		boaWrite(wp, "\tdocument.formfastconf.username.disabled=true;\n");
		boaWrite(wp, "\tdocument.formfastconf.password.disabled=true;\n");
		boaWrite(wp, "\tdocument.formfastconf.usestaticip.disabled=true;\n");
	}
	else {
		boaWrite(wp, "\tdocument.formfastconf.username.value=\"%s\";\n", vcEntry.pppUsername);
		boaWrite(wp, "\tdocument.formfastconf.password.value=\"%s\";\n", vcEntry.pppPassword);
		boaWrite(wp, "\tdocument.formfastconf.username.disabled=false;\n");
		boaWrite(wp, "\tdocument.formfastconf.password.disabled=false;\n");
		boaWrite(wp, "\tdocument.formfastconf.usestaticip.disabled=false;\n");

		if (vcEntry.pppIp) {
			boaWrite(wp, "\tdocument.formfastconf.usestaticip.checked=true;\n");
			boaWrite(wp, "\tdocument.formfastconf.ipaddress.value=\"%s\";\n",
					inet_ntoa(*(struct in_addr *)vcEntry.ipAddr));
			boaWrite(wp, "\tdocument.formfastconf.mask.value=\"%s\";\n",
					inet_ntoa(*(struct in_addr *)vcEntry.netMask));
			boaWrite(wp, "\tdocument.formfastconf.startaddress.value=\"%s\";\n",
					inet_ntoa(*(struct in_addr *)vcEntry.startAddr));
			boaWrite(wp, "\tdocument.formfastconf.endaddress.value=\"%s\";\n",
					inet_ntoa(*(struct in_addr *)vcEntry.endAddr));
		}
		else
			boaWrite(wp, "\tdocument.formfastconf.usestaticip.checked=false;\n");
	}


	tempWlanIdx=wlan_idx;
	wlan_idx=0;
	
	mib_get( MIB_WLAN_DISABLED, (void *)&disable);
	if (disable)
		boaWrite(wp, "\tdocument.formfastconf.enablewireless_0.checked=false;\n");
	else
		boaWrite(wp, "\tdocument.formfastconf.enablewireless_0.checked=true;\n");
	
	if (mib_get( MIB_WLAN_SSID, (void *)ssid))
		boaWrite(wp, "\tdocument.formfastconf.ssid_0.value=\"%s\";\n", ssid);
	else
		boaWrite(wp, "\tdocument.formfastconf.ssid_0.value=\"\";\n");
	
	if (!wlan_getEntry(&wlanEntry, 0)){
		printf("get wlan0 2.4GHz root entry fail\n");
		wlan_idx=tempWlanIdx;
		return;
	}
	
	boaWrite(wp, "\tdocument.formfastconf.encryption_0.value=%d;\n", wlanEntry.encrypt);

	if (wlanEntry.encrypt == WIFI_SEC_WEP){
		wlanEntry.wep128Key1[WEP128_KEY_LEN]='\0';
		boaWrite(wp, "\tdocument.formfastconf.key_0.value=\"%s\";\n", wlanEntry.wep128Key1);
	}
	else if (wlanEntry.encrypt != WIFI_SEC_NONE)
		boaWrite(wp, "\tdocument.formfastconf.key_0.value=\"%s\";\n", wlanEntry.wpaPSK);
	else
		boaWrite(wp, "\tdocument.formfastconf.key_0.value=\"\";\n");
	

	wlan_idx=1;
	mib_get( MIB_WLAN_DISABLED, (void *)&disable);
	if (disable)
		boaWrite(wp, "\tdocument.formfastconf.enablewireless_1.checked=false;\n");
	else
		boaWrite(wp, "\tdocument.formfastconf.enablewireless_1.checked=true;\n");
	
	if (mib_get( MIB_WLAN_SSID, (void *)ssid))
		boaWrite(wp, "\tdocument.formfastconf.ssid_1.value=\"%s\";\n", ssid);
	else
		boaWrite(wp, "\tdocument.formfastconf.ssid_1.value=\"\";\n");

	if (!wlan_getEntry(&wlanEntry, 0)){
		printf("get wlan0 5GHz root entry fail\n");
		wlan_idx=tempWlanIdx;
		return;
	}
	boaWrite(wp, "\tdocument.formfastconf.encryption_1.value=%d;\n", wlanEntry.encrypt);

	if (wlanEntry.encrypt == WIFI_SEC_WEP){
		wlanEntry.wep128Key1[WEP128_KEY_LEN]='\0';
		boaWrite(wp, "\tdocument.formfastconf.key_1.value=\"%s\";\n", wlanEntry.wep128Key1);
	}
	else if (wlanEntry.encrypt != WIFI_SEC_NONE)
		boaWrite(wp, "\tdocument.formfastconf.key_1.value=\"%s\";\n", wlanEntry.wpaPSK);
	else
		boaWrite(wp, "\tdocument.formfastconf.key_1.value=\"\";\n");

	wlan_idx=tempWlanIdx;
	
}

void setWlan(request * wp,char disable,char* ssid,WIFI_SECURITY_T encryption,char* key)
{
	char curssid[32],tmpBuf[100];
	MIB_CE_MBSSIB_T Entry;

	mib_set( MIB_WLAN_DISABLED, (void *)&disable);
	if(disable==1)
		goto do_nothing;
	
	if(mib_get( MIB_WLAN_SSID, (void *)curssid) == 0)
	{
		strcpy(tmpBuf, strSetSSIDErr);
		goto setErr_wlan;
	}else
	{
	
		if(strcmp(curssid, ssid))
		{
			int ssid_changed = 1;
			
			if( mib_set(MIB_WLAN_SSID_MOD_BY_USER, (void *)&ssid_changed) == 0)
			{
   	 			strcpy(tmpBuf, strSetSSIDErr);
				goto setErr_wlan;
			}
			
			if( mib_set(MIB_WLAN_SSID, (void *)ssid) == 0)
			{
   	 			strcpy(tmpBuf, strSetSSIDErr);
				goto setErr_wlan;
			}

		}else
		{
			//ssid does not changed!
		}
	}

	if (!wlan_getEntry(&Entry, 0)){
		strcpy(tmpBuf, strGetMBSSIBTBLErr);
		goto setErr_wlan;
	}
	Entry.encrypt=encryption;
	
	if (encryption == WIFI_SEC_NONE || encryption == WIFI_SEC_WEP) {
		Entry.enable1X=0;
		if (encryption == WIFI_SEC_WEP)
		{
			Entry.authType=AUTH_SHARED;
			Entry.wep=WEP128;
			Entry.wepKeyType=0;//ascii
			memcpy(Entry.wep128Key1,key,WEP128_KEY_LEN);
		}
	}
	else{
		Entry.wpaAuth = WPA_AUTH_PSK;
		if ((encryption == WIFI_SEC_WPA) || (encryption == WIFI_SEC_WPA2_MIXED)) {
			Entry.unicastCipher=WPA_CIPHER_TKIP;
		}
		if ((encryption== WIFI_SEC_WPA2) || (encryption == WIFI_SEC_WPA2_MIXED)) {
			Entry.wpa2UnicastCipher=WPA_CIPHER_AES;
		}
		Entry.wpaPSKFormat=0;//passphrase
		strcpy(Entry.wpaPSK, key);
	}
	wlan_setEntry(&Entry,0);
#ifdef CONFIG_WIFI_SIMPLE_CONFIG //WPS
	update_wps_configured(0);
#endif
	sleep(5);
do_nothing:
	return;
setErr_wlan:

	ERR_MSG(tmpBuf);
	return;
}

void formFastConf(request * wp, char *path, char *query)
{
	char *userName,*pwd,*strValue,*ssid;
	char *strSubmit;
	char tmpBuf[100],disable,tempssid[MAX_SSID_LEN];
	struct in_addr ipAddr, Mask, startAddr, endAddr, subNet;
	unsigned int totalEntry,i;
	MIB_CE_ATM_VC_T Entry,entry,tentry;
	unsigned char encryption;
	int tempWlanIdx, ssid_changed = 0;
	char enableIP2;
	unsigned int ip2, mask2,unnumberChanged;
	unsigned char  currState=0;

	strSubmit = boaGetVar(wp, "omit", "");
	if (strSubmit[0])
		goto setOK;
	
	totalEntry = mib_chain_total(MIB_ATM_VC_TBL);
	
	if (!mib_chain_get(MIB_ATM_VC_TBL, 0, (void *)&Entry)) {
		strcpy(tmpBuf, Tget_mib_error);
		goto setErr;
	}

	memcpy(&entry,&Entry,sizeof(Entry));

	Entry.enable = 1;
	
	strValue=boaGetVar(wp,"username","");
	strncpy(Entry.pppUsername, strValue, MAX_PPP_NAME_LEN);
	Entry.pppUsername[MAX_PPP_NAME_LEN-1]='\0';
	
	strValue=boaGetVar(wp,"password","");
	strncpy(Entry.pppPassword, strValue, MAX_NAME_LEN);
	Entry.pppPassword[MAX_NAME_LEN-1]='\0';
	
	strValue=boaGetVar(wp,"usestaticip","");
	if(!strcmp(strValue,"ON"))
	{
		Entry.pppIp = 1;
		
		strValue=boaGetVar(wp,"ipaddress","");
		if(inet_aton(strValue,(struct in_addr *)&ipAddr)==0)
		{
			strcpy(tmpBuf,strInvalIP);
			goto setErr;
		}
		
		strValue=boaGetVar(wp,"mask","");
		if(inet_aton(strValue,(struct in_addr *)&Mask)==0)
		{
			strcpy(tmpBuf,strInvalMask);
			goto setErr;
		}
		
		strValue=boaGetVar(wp,"startaddress","");
		if(inet_aton(strValue,(struct in_addr *)&startAddr)==0)
		{
			strcpy(tmpBuf,strInvalIP);
			goto setErr;
		}
		
		strValue=boaGetVar(wp,"endaddress","");
		if(inet_aton(strValue,(struct in_addr *)&endAddr)==0)
		{
			strcpy(tmpBuf,strInvalIP);
			goto setErr;
		}
		
		subNet.s_addr = ipAddr.s_addr & Mask.s_addr;
		if ((subNet.s_addr != (startAddr.s_addr & Mask.s_addr)) || 
			(subNet.s_addr != (endAddr.s_addr & Mask.s_addr))) {
			strcpy(tmpBuf," start,  end and static IP address shoud be in the same subnet!");	
			goto setErr;
		}

		memcpy(Entry.ipAddr, &ipAddr, IP_ADDR_LEN);
		memcpy(Entry.netMask, &Mask, IP_ADDR_LEN);
		memcpy(Entry.startAddr, &startAddr, IP_ADDR_LEN);
		memcpy(Entry.endAddr, &endAddr, IP_ADDR_LEN);

		for (i=1; i<totalEntry; i++)
		{
			if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&tentry))
				continue;

			if ((tentry.cmode == CHANNEL_MODE_PPPOE) && tentry.pppIp) {
				deleteConnection(CONFIGONE, &tentry);
				tentry.pppIp = 0;
				mib_chain_update(MIB_ATM_VC_TBL, (void *)&tentry, i);
				restartWAN(CONFIGONE, &tentry);
				break;
			}
		}
	}
	else
		Entry.pppIp = 0;

	deleteConnection(CONFIGONE, &entry);
	mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry, 0);
	unnumberChanged = resolveUnnumberDependency(&Entry, &entry);
	restartWAN(CONFIGONE, &Entry);
	
	if (1 == unnumberChanged)
		restart_lanip();
	
#ifdef CONFIG_RTK_RG_INIT
	FlushRTK_RG_ACL_IP_UNNUMBER_RULE();
	AddRTK_RG_ACL_IP_UNNUMBER_RULE();
#endif
	
	//cxy: start wlan setup
	strValue=boaGetVar(wp,"enablewireless_0","");
	if(!strcmp(strValue,"ON"))
		disable=0;
	else
		disable=1;
	
	
	tempWlanIdx=wlan_idx;
	wlan_idx=0;
	
	
	ssid=boaGetVar(wp,"ssid_0","");
	
	strValue=boaGetVar(wp,"encryption_0","");
	encryption=strValue[0]-'0';
	
	strValue=boaGetVar(wp,"key_0","");
	
	setWlan(wp,disable,ssid,(WIFI_SECURITY_T)encryption,strValue);
	mib_get(MIB_WLAN_SSID_MOD_BY_USER, (void *)&ssid_changed);
	if(ssid_changed == 0)
	{
		sprintf(tempssid,"%s_2.4G",(char *)Entry.pppUsername);
		tempssid[MAX_SSID_LEN-1]='\0';
		mib_set(MIB_WLAN_SSID, (void *)tempssid);
	}
	strValue=boaGetVar(wp,"enablewireless_1","");
	if(!strcmp(strValue,"ON"))
		disable=0;
	else
		disable=1;
	
	
	wlan_idx=1;	
	
	ssid=boaGetVar(wp,"ssid_1","");
	
	strValue=boaGetVar(wp,"encryption_1","");
	encryption=strValue[0]-'0';
	
	strValue=boaGetVar(wp,"key_1","");
	
	setWlan(wp,disable,ssid,(WIFI_SECURITY_T)encryption,strValue);
	mib_get(MIB_WLAN_SSID_MOD_BY_USER, (void *)&ssid_changed);
	if(ssid_changed == 0)
	{
		sprintf(tempssid,"%s_5G",(char *)Entry.pppUsername);
		tempssid[MAX_SSID_LEN-1]='\0';
		mib_set(MIB_WLAN_SSID, (void *)tempssid);
	}
	wlan_idx=tempWlanIdx;
	
	config_WLAN(ACT_RESTART);

	
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif	

setOK:
	strValue=boaGetVar(wp,"submit-url","");
	if(strValue[0])
		boaRedirect(wp, strValue);
	else
		boaDone(wp, 200);
	return;
setErr:
	ERR_MSG(tmpBuf);
	//boaRedirect(wp, "/admin/fastconf.asp");
	return;
}
