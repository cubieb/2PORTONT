/*
 *  Utility module for WiFi Simple-Config
 *
 *	Copyright (C)2006, Realtek Semiconductor Corp. All rights reserved.
 *
 *	$Id: utils.c,v 1.20 2012/02/23 07:19:45 cathy Exp $
 */

/*================================================================*/
/* Include Files */

#include <openssl/crypto.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <netinet/in.h>

// RTL8671, for file IO, andrew
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


#include "wsc.h"
#include <../../../config/autoconf.h>
#include "../../../include/linux/autoconf.h"


/*================================================================*/
/* Constant Definitions */

#define PERSONAL_STRING "Wi-Fi Easy and Secure Key Derivation"
#define PERSONAL_STRING_SIZE 36
#define PRF_DIGEST_SIZE 32


/*================================================================*/
/* Local Variables */

static unsigned char WSC_IE_OUI[4] = {0x00, 0x50, 0xf2, 0x04};


#ifdef WPS2DOTX
unsigned char WSC_VENDOR_OUI[3] = {0x00, 0x37, 0x2a};
unsigned char BroadCastMac[6]=	{"\xFF\xFF\xFF\xFF\xFF\xFF"};
unsigned char WSC_VENDOR_V2[6] = {0x00, 0x37, 0x2a , 0x00 , 0x01 , 0x20};
unsigned char WSC_VENDOR_V57[6] = {0x00, 0x37, 0x2a , 0x00 , 0x01 , 0x57};
unsigned char EXT_ATTRI_TEST[6] = {0x01, 0x02, 0x03 , 0x03 , 0x02 , 0x01};
#endif

static unsigned char Provisioning_Service_IE_OUI[4] = {0x00, 0x50, 0xf2, 0x05};

/*================================================================*/
/* Implementation Routines */

#ifdef DEBUG
void debug_out(unsigned char *label, unsigned char *data, int data_length)
{
	int i,j;
	int num_blocks;
	int block_remainder;

	num_blocks = data_length >> 4;
	block_remainder = data_length & 15;

	if (label)
		printf("%s\n", label);

	if (data==NULL || data_length==0)
		return;

	for (i=0; i<num_blocks; i++) {
		printf("\t");
		for (j=0; j<16; j++)
			printf("%02x ", data[j + (i<<4)]);       
		printf("\n");
	}

	if (block_remainder > 0) {
		printf("\t");
		for (j=0; j<block_remainder; j++)
			printf("%02x ", data[j+(num_blocks<<4)]);        
		printf("\n");
	}
}
#endif // DEBUG

#ifdef WPS2DOTX

void registrar_add_authorized_mac(CTX_Tp pCtx, const  unsigned char *addr)
{

	int i;
	UTIL_DEBUG("\n");
	for (i = 0; i < MAX_AUTHORIZED_MACS; i++){
		if (memcmp(pCtx->authorized_macs[i], addr , ETHER_ADDRLEN)==0)
			return; /* already in list */
	}	
		
	for (i = MAX_AUTHORIZED_MACS - 1; i > 0; i--){
		memcpy(pCtx->authorized_macs[i], pCtx->authorized_macs[i - 1], ETHER_ADDRLEN);
	}
	
	memcpy(pCtx->authorized_macs[0], addr, ETHER_ADDRLEN);
}


void registrar_remove_authorized_mac(CTX_Tp pCtx,const unsigned char *addr)
{
	int i;
	UTIL_DEBUG("\n");
	for (i = 0; i < MAX_AUTHORIZED_MACS; i++) {
		if (memcmp(pCtx->authorized_macs, addr, ETHER_ADDRLEN) == 0)
			break;
	}
	
	if (i == MAX_AUTHORIZED_MACS)
		return; /* not in the list */


	for (; i + 1 < MAX_AUTHORIZED_MACS; i++)
		memcpy(pCtx->authorized_macs[i], pCtx->authorized_macs[i + 1], ETHER_ADDRLEN);
	
	memset(pCtx->authorized_macs[MAX_AUTHORIZED_MACS - 1], 0 , ETHER_ADDRLEN);

	//	memset(pCtx->authorized_macs[i], 0 , ETHER_ADDRLEN);

}

void registrar_remove_all_authorized_mac(CTX_Tp pCtx)
{
	UTIL_DEBUG("\n");
	memset(pCtx->authorized_macs, 0 , MAX_AUTHORIZED_MACS*ETHER_ADDRLEN);
}

int report_authoriedMacCount(CTX_Tp pCtx)
{
	int i2 = 0;
	unsigned char* Ptr = NULL;
	for ( ;i2 < MAX_AUTHORIZED_MACS; i2++){
		if (is_zero_ether_addr(pCtx->authorized_macs[i2]))
			break;
		else{
			Ptr = pCtx->authorized_macs[i2];
			UTIL_DEBUG("auth mac:%02X%02X%02X:%02X%02X%02X\n",Ptr[0],Ptr[1],Ptr[2],Ptr[3],Ptr[4],Ptr[5]);
		}
	}
	if(i2)
		UTIL_DEBUG("count=%d\n",i2);
	
	return i2;
}
	
#endif


void convert_bin_to_str(unsigned char *bin, int len, char *out)
{
	int i;
	char tmpbuf[10];

	out[0] = '\0';

	for (i=0; i<len; i++) {
		sprintf(tmpbuf, "%02x", bin[i]);
		strcat(out, tmpbuf);
	}
}

#ifdef SUPPORT_UPNP
void convert_bin_to_str_UPnP(unsigned char *bin, int len, char *out)
{
	int i;
	char tmpbuf[10];

	out[0] = '\0';

	for (i=0; i<len; i++) {
		if (i == len-1)
			sprintf(tmpbuf, "%02x", bin[i]);
		else
			sprintf(tmpbuf, "%02x:", bin[i]);
		strcat(out, tmpbuf);
	}
}
#endif


#ifdef CONFIG_RTL867x
enum { 
	WPS_AP_MODE=0, 
	WPS_CLIENT_MODE=1, 
	WPS_WDS_MODE=2, 
	WPS_AP_WDS_MODE=3 
};

#define WRITE_WSC_PARAM(dst, tmp, str, val) {	\
	sprintf(tmp, str, val); \
	memcpy(dst, tmp, strlen(tmp)); \
	dst += strlen(tmp); \
}

static void convert_hex_to_ascii(unsigned long code, char *out)
{
	*out++ = '0' + ((code / 10000000) % 10);  
	*out++ = '0' + ((code / 1000000) % 10);
	*out++ = '0' + ((code / 100000) % 10);
	*out++ = '0' + ((code / 10000) % 10);
	*out++ = '0' + ((code / 1000) % 10);
	*out++ = '0' + ((code / 100) % 10);
	*out++ = '0' + ((code / 10) % 10);
	*out++ = '0' + ((code / 1) % 10);
	*out = '\0';
}

static int compute_pin_checksum(unsigned long int PIN)
{
	unsigned long int accum = 0;
	int digit;
	
	PIN *= 10;
	accum += 3 * ((PIN / 10000000) % 10); 	
	accum += 1 * ((PIN / 1000000) % 10);
	accum += 3 * ((PIN / 100000) % 10);
	accum += 1 * ((PIN / 10000) % 10); 
	accum += 3 * ((PIN / 1000) % 10); 
	accum += 1 * ((PIN / 100) % 10); 
	accum += 3 * ((PIN / 10) % 10);

	digit = (accum % 10);
	return (10 - digit) % 10;
}

typedef enum { _ENCRYPT_DISABLED_=0, _ENCRYPT_WEP_=1, _ENCRYPT_WPA_=2, _ENCRYPT_WPA2_=4, _ENCRYPT_WPA2_MIXED_=6 } ENCRYPT_T;

int WPS_updateWscConf(char *in, char *out, int genpin)
{
	int fh = -1;
	struct stat status;
	char *buf = NULL, *ptr;
	unsigned char intVal2, is_client, is_config, is_registrar, is_wep=0, wep_key_type=0, wep_transmit_key=0;
	unsigned char intVal, current_wps_version;
	unsigned char wlan_encrypt=0, wlan_wpa_cipher=0, wlan_wpa2_cipher=0;
	int config_method;
	unsigned char tmpbuf[100], tmp1[100];
	int len;
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(FOR_DUAL_BAND)
	unsigned char wlanBand2G5GSelect;
	int orig_wlan_idx; 
#endif

#ifdef FOR_DUAL_BAND
//	int wlan_idx_orig = wlan_idx;
#endif //FOR_DUAL_BAND
	/*
	if ( !mib_init()) {
		printf("Initialize AP MIB failed!\n");
		return -1;
	}
	*/
	fprintf(stderr, "Writing file %s...\n", out ? out : "");
	mib_get(gMIB_WSC_PIN, (void *)tmpbuf);
	if (genpin || !strcmp(tmpbuf, "\x0")) {
		#include <sys/time.h>			
		struct timeval tod;
		unsigned long num;
		
		mib_get(gMIB_ELAN_MAC_ADDR/*MIB_HW_NIC0_ADDR*/, (void *)&tmp1);			
		gettimeofday(&tod , NULL);
		tod.tv_sec += tmp1[4]+tmp1[5];		
		srand(tod.tv_sec);
		num = rand() % 10000000;
		num = num*10 + compute_pin_checksum(num);
		convert_hex_to_ascii((unsigned long)num, tmpbuf);

		mib_set(gMIB_WSC_PIN, (void *)tmpbuf);
		mib_update_all();//mib_update(CURRENT_SETTING);		

		printf("Generated PIN = %s\n", tmpbuf);

		if (genpin)
			return 0;
	}
#ifdef FOR_DUAL_BAND
//	wlan_idx = 0;
#endif //FOR_DUAL_BAND
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(FOR_DUAL_BAND)
	orig_wlan_idx = wlan_idx;
	wlan_idx = 0;
	if (useWlanIfVirtIdx())
			wlan_idx = 1;
#endif
	if (stat(in, &status) < 0) {
		printf("stat() error [%s]!\n", in);
		//return -1;
		goto ERROR;
	}

	buf = malloc(status.st_size+2048);
	if (buf == NULL) {
		printf("malloc() error [%d]!\n", (int)status.st_size+2048);
		//return -1;
		goto ERROR;
	}

	ptr = buf;
	mib_get(gMIB_WLAN_MODE, (void *)&is_client);
	mib_get(gMIB_WSC_CONFIGURED, (void *)&is_config);
	mib_get(gMIB_WSC_REGISTRAR_ENABLED, (void *)&is_registrar);
	if (is_client == WPS_CLIENT_MODE) {
		if (is_registrar)
			intVal = MODE_CLIENT_CONFIG;			
		else {
			if (!is_config)
				intVal = MODE_CLIENT_UNCONFIG;
			else
				intVal = MODE_CLIENT_CONFIG;
		}
	}
	else {
		is_registrar = 1; // always true in AP		
		if (!is_config)
			intVal = MODE_AP_UNCONFIG;
		else {
			if (is_registrar)
				intVal = MODE_AP_PROXY_REGISTRAR;
			else
				intVal = MODE_AP_PROXY;
		}		
	}
	WRITE_WSC_PARAM(ptr, tmpbuf, "mode = %d\n", intVal);

	if (is_client)
		intVal = 0;
	else
		mib_get(gMIB_WSC_UPNP_ENABLED, (void *)&intVal);
	WRITE_WSC_PARAM(ptr, tmpbuf, "upnp = %d\n", intVal);

#ifdef WPS2DOTX
#ifdef WPS_VERSION_CONFIGURABLE
	if (mib_get(gMIB_WLAN_WSC_VERSION, (void *)&current_wps_version) == 0)
#endif
		current_wps_version = WPS_VERSION_V2;
#else
	current_wps_version = WPS_VERSION_V1;
#endif
	WRITE_WSC_PARAM(ptr, tmpbuf, "current_wps_version = %d\n", current_wps_version);

	intVal = 0;
	mib_get(gMIB_WSC_METHOD, (void *)&intVal);
#ifdef WPS2DOTX
	if (current_wps_version == WPS_VERSION_V2) {
		if (intVal == 1) //Pin
			config_method = CONFIG_METHOD_VIRTUAL_PIN;
		else if (intVal == 2) //PBC
			config_method = (CONFIG_METHOD_PHYSICAL_PBC | CONFIG_METHOD_VIRTUAL_PBC);
		else if (intVal == 3) //Pin+PBC
			config_method = (CONFIG_METHOD_VIRTUAL_PIN |  CONFIG_METHOD_PHYSICAL_PBC | CONFIG_METHOD_VIRTUAL_PBC);
	} else
#endif
	{
		//Ethernet(0x2)+Label(0x4)+PushButton(0x80) Bitwise OR
		if (intVal == 1) //Pin+Ethernet
			config_method = (CONFIG_METHOD_ETH | CONFIG_METHOD_PIN);
		else if (intVal == 2) //PBC+Ethernet
			config_method = (CONFIG_METHOD_ETH | CONFIG_METHOD_PBC);
		else if (intVal == 3) //Pin+PBC+Ethernet
			config_method = (CONFIG_METHOD_ETH | CONFIG_METHOD_PIN | CONFIG_METHOD_PBC);
	}

	WRITE_WSC_PARAM(ptr, tmpbuf, "config_method = %d\n", config_method);

#ifdef FOR_DUAL_BAND
	mib_get(gMIB_WSC_DISABLE, (void *)&intVal2);
	WRITE_WSC_PARAM(ptr, tmpbuf, "wlan0_wsc_disabled = %d\n", intVal2);
#endif

	mib_get(gMIB_WSC_AUTH, (void *)&intVal2);
	WRITE_WSC_PARAM(ptr, tmpbuf, "auth_type = %d\n", intVal2);

	mib_get(gMIB_WSC_ENC, (void *)&intVal);
	WRITE_WSC_PARAM(ptr, tmpbuf, "encrypt_type = %d\n", intVal);
	if (intVal == ENCRYPT_WEP)
		is_wep = 1;
	
	/*for detial mixed mode info*/
	mib_get(gMIB_WLAN_ENCRYPT, (void *)&wlan_encrypt);
	mib_get(gMIB_WLAN_WPA_CIPHER_SUITE, (void *)&wlan_wpa_cipher);
	mib_get(gMIB_WLAN_WPA2_CIPHER_SUITE, (void *)&wlan_wpa2_cipher);
	
	intVal=0;	
	if(wlan_encrypt==6){	// mixed mode
		if(wlan_wpa_cipher==1){
			intVal |= WSC_WPA_TKIP;
		}else if(wlan_wpa_cipher==2){
			intVal |= WSC_WPA_AES;
		}else if(wlan_wpa_cipher==3){
			intVal |= (WSC_WPA_TKIP | WSC_WPA_AES);
		}
		if(wlan_wpa2_cipher==1){
			intVal |= WSC_WPA2_TKIP;
		}else if(wlan_wpa2_cipher==2){
			intVal |= WSC_WPA2_AES;
		}else if(wlan_wpa2_cipher==3){
			intVal |= (WSC_WPA2_TKIP | WSC_WPA2_AES);
		}
	}
	WRITE_WSC_PARAM(ptr, tmpbuf, "mixedmode = %d\n", intVal);
	/*for detial mixed mode info*/

	if (is_client) {
		mib_get(gMIB_WLAN_NETWORK_TYPE, (void *)&intVal);
		if (intVal == 0)
			intVal = 1;
		else
			intVal = 2;
	}
	else
		intVal = 1;
	WRITE_WSC_PARAM(ptr, tmpbuf, "connection_type = %d\n", intVal);

	mib_get(gMIB_WSC_MANUAL_ENABLED, (void *)&intVal);
	WRITE_WSC_PARAM(ptr, tmpbuf, "manual_config = %d\n", intVal);

	if (is_wep) { // only allow WEP in none-MANUAL mode (configured by external registrar)
		mib_get(gMIB_WLAN_ENCRYPT, (void *)&intVal);
		if (intVal != _ENCRYPT_WEP_) {
			printf("WEP mismatched between WPS and host system\n");
			goto ERROR;
		}
		mib_get(gMIB_WLAN_WEP, (void *)&intVal);
		if (intVal <= WEP_DISABLED || intVal > WEP128) {
			printf("WEP encrypt length error\n");
			goto ERROR;
		}
		mib_get(gMIB_WLAN_WEP_KEY_TYPE, (void *)&wep_key_type);
		mib_get(gMIB_WLAN_WEP_DEFAULT_KEY, (void *)&wep_transmit_key);
		wep_transmit_key++;
		WRITE_WSC_PARAM(ptr, tmpbuf, "wep_transmit_key = %d\n", wep_transmit_key);
		if (intVal == WEP64) {
			mib_get(gMIB_WLAN_WEP64_KEY1, (void *)&tmpbuf);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 5);
				tmp1[5] = '\0';
			}
			else {
				convert_bin_to_str(tmpbuf, 5, tmp1);
				tmp1[10] = '\0';
			}			
			WRITE_WSC_PARAM(ptr, tmpbuf, "network_key = %s\n", tmp1);

			mib_get(gMIB_WLAN_WEP64_KEY2, (void *)&tmpbuf);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 5);
				tmp1[5] = '\0';
			}
			else {
				convert_bin_to_str(tmpbuf, 5, tmp1);
				tmp1[10] = '\0';
			}			
			WRITE_WSC_PARAM(ptr, tmpbuf, "wep_key2 = %s\n", tmp1);

			mib_get(gMIB_WLAN_WEP64_KEY3, (void *)&tmpbuf);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 5);
				tmp1[5] = '\0';
			}
			else {
				convert_bin_to_str(tmpbuf, 5, tmp1);
				tmp1[10] = '\0';
			}			
			WRITE_WSC_PARAM(ptr, tmpbuf, "wep_key3 = %s\n", tmp1);


			mib_get(gMIB_WLAN_WEP64_KEY4, (void *)&tmpbuf);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 5);
				tmp1[5] = '\0';
			}
			else {
				convert_bin_to_str(tmpbuf, 5, tmp1);
				tmp1[10] = '\0';
			}			
			WRITE_WSC_PARAM(ptr, tmpbuf, "wep_key4 = %s\n", tmp1);
		}
		else {
			mib_get(gMIB_WLAN_WEP128_KEY1, (void *)&tmpbuf);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 13);
				tmp1[13] = '\0';
			}
			else {
				convert_bin_to_str(tmpbuf, 13, tmp1);
				tmp1[26] = '\0';
			}
			WRITE_WSC_PARAM(ptr, tmpbuf, "network_key = %s\n", tmp1);


			mib_get(gMIB_WLAN_WEP128_KEY2, (void *)&tmpbuf);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 13);
				tmp1[13] = '\0';
			}
			else {
				convert_bin_to_str(tmpbuf, 13, tmp1);
				tmp1[26] = '\0';
			}
			WRITE_WSC_PARAM(ptr, tmpbuf, "wep_key2 = %s\n", tmp1);

			mib_get(gMIB_WLAN_WEP128_KEY3, (void *)&tmpbuf);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 13);
				tmp1[13] = '\0';
			}
			else {
				convert_bin_to_str(tmpbuf, 13, tmp1);
				tmp1[26] = '\0';
			}
			WRITE_WSC_PARAM(ptr, tmpbuf, "wep_key3 = %s\n", tmp1);

			mib_get(gMIB_WLAN_WEP128_KEY4, (void *)&tmpbuf);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 13);
				tmp1[13] = '\0';
			}
			else {
				convert_bin_to_str(tmpbuf, 13, tmp1);
				tmp1[26] = '\0';
			}
			WRITE_WSC_PARAM(ptr, tmpbuf, "wep_key4 = %s\n", tmp1);
		}
	}
	else {
		mib_get(gMIB_WLAN_WPA_PSK, (void *)&tmp1);		
		WRITE_WSC_PARAM(ptr, tmpbuf, "network_key = %s\n", tmp1);
	}
		
	mib_get(gMIB_WLAN_SSID, (void *)&tmp1);	
	WRITE_WSC_PARAM(ptr, tmpbuf, "ssid = %s\n", tmp1);	

#if 0	
//	}
//	else {			
		mib_get(MIB_WSC_PSK, (void *)&tmp1);
		WRITE_WSC_PARAM(ptr, tmpbuf, "network_key = %s\n", tmp1);		
		
		mib_get(MIB_WSC_SSID, (void *)&tmp1);
		WRITE_WSC_PARAM(ptr, tmpbuf, "ssid = %s\n", tmp1);
//	}
#endif

	mib_get(gMIB_WSC_PIN, (void *)&tmp1);
	WRITE_WSC_PARAM(ptr, tmpbuf, "pin_code = %s\n", tmp1);

	mib_get(gMIB_WLAN_CHAN_NUM, (void *)&intVal);
	if (intVal > 14)
		intVal = 2;
	else
		intVal = 1;
	WRITE_WSC_PARAM(ptr, tmpbuf, "rf_band = %d\n", intVal);

#ifdef FOR_DUAL_BAND
#ifdef CONFIG_RTL_92D_DMDP
mib_get(gMIB_WLAN_BAND2G5G_SELECT, (void *)&wlanBand2G5GSelect);
if(wlanBand2G5GSelect == BANDMODEBOTH){
#endif
	wlan_idx = 1;

	WRITE_WSC_PARAM(ptr, tmpbuf, "#=====wlan1 start==========%d\n",intVal);

	mib_get(gMIB_WSC_DISABLE, (void *)&intVal2);
	WRITE_WSC_PARAM(ptr, tmpbuf, "wlan1_wsc_disabled = %d\n", intVal2);

	mib_get(gMIB_WLAN_SSID, (void *)&tmp1); 
	WRITE_WSC_PARAM(ptr, tmpbuf, "ssid2 = %s\n", tmp1);

	mib_get(gMIB_WSC_AUTH, (void *)&intVal2);
	WRITE_WSC_PARAM(ptr, tmpbuf, "auth_type2 = %d\n", intVal2);

	mib_get(gMIB_WSC_ENC, (void *)&intVal);
	WRITE_WSC_PARAM(ptr, tmpbuf, "encrypt_type2 = %d\n", intVal);
	if (intVal == ENCRYPT_WEP)
		is_wep = 1;
	
	/*for detial mixed mode info*/
	mib_get(gMIB_WLAN_ENCRYPT, (void *)&wlan_encrypt);
	mib_get(gMIB_WLAN_WPA_CIPHER_SUITE, (void *)&wlan_wpa_cipher);
	mib_get(gMIB_WLAN_WPA2_CIPHER_SUITE, (void *)&wlan_wpa2_cipher);
	
	intVal=0;	
	if(wlan_encrypt==6){	// mixed mode
		if(wlan_wpa_cipher==1){
			intVal |= WSC_WPA_TKIP;
		}else if(wlan_wpa_cipher==2){
			intVal |= WSC_WPA_AES;		
		}else if(wlan_wpa_cipher==3){
			intVal |= (WSC_WPA_TKIP | WSC_WPA_AES);		
		}
		if(wlan_wpa2_cipher==1){
			intVal |= WSC_WPA2_TKIP;
		}else if(wlan_wpa2_cipher==2){
			intVal |= WSC_WPA2_AES;		
		}else if(wlan_wpa2_cipher==3){
			intVal |= (WSC_WPA2_TKIP | WSC_WPA2_AES);		
		}		
	}
	WRITE_WSC_PARAM(ptr, tmpbuf, "mixedmode2 = %d\n", intVal);
	/*for detial mixed mode info*/
	
/* 
	mib_get(gMIB_WLAN_BAND2G5G_SELECT, (void *)&intVal);	// 0:2.4g  ; 1:5G   ; 2:both(dual band)
	if(intVal != 2) {
		intVal=1;
		WRITE_WSC_PARAM(ptr, tmpbuf, "wlan1_wsc_disabled = %d\n",intVal);
	}
	else {
		mib_get(gMIB_WSC_DISABLE, (void *)&intVal);
		WRITE_WSC_PARAM(ptr, tmpbuf, "wlan1_wsc_disabled = %d\n", intVal);	
	}
*/
	if (is_wep) { // only allow WEP in none-MANUAL mode (configured by external registrar)
		mib_get(gMIB_WLAN_ENCRYPT, (void *)&intVal);
		if (intVal != _ENCRYPT_WEP_) {
			printf("WEP mismatched between WPS and host system\n");
			goto ERROR;
		}
		mib_get(gMIB_WLAN_WEP, (void *)&intVal);
		if (intVal <= WEP_DISABLED || intVal > WEP128) {
			printf("WEP encrypt length error\n");
			goto ERROR;
		}
		mib_get(gMIB_WLAN_WEP_KEY_TYPE, (void *)&wep_key_type);
		mib_get(gMIB_WLAN_WEP_DEFAULT_KEY, (void *)&wep_transmit_key);
		wep_transmit_key++;
		WRITE_WSC_PARAM(ptr, tmpbuf, "wep_transmit_key2 = %d\n", wep_transmit_key);
		if (intVal == WEP64) {
			mib_get(gMIB_WLAN_WEP64_KEY1, (void *)&tmpbuf);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 5);
				tmp1[5] = '\0';
			}
			else {
				convert_bin_to_str(tmpbuf, 5, tmp1);
				tmp1[10] = '\0';
			}			
			WRITE_WSC_PARAM(ptr, tmpbuf, "network_key2 = %s\n", tmp1);

			mib_get(gMIB_WLAN_WEP64_KEY2, (void *)&tmpbuf);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 5);
				tmp1[5] = '\0';
			}
			else {
				convert_bin_to_str(tmpbuf, 5, tmp1);
				tmp1[10] = '\0';
			}			
			WRITE_WSC_PARAM(ptr, tmpbuf, "wep_key22 = %s\n", tmp1);

			mib_get(gMIB_WLAN_WEP64_KEY3, (void *)&tmpbuf);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 5);
				tmp1[5] = '\0';
			}
			else {
				convert_bin_to_str(tmpbuf, 5, tmp1);
				tmp1[10] = '\0';
			}			
			WRITE_WSC_PARAM(ptr, tmpbuf, "wep_key32 = %s\n", tmp1);


			mib_get(gMIB_WLAN_WEP64_KEY4, (void *)&tmpbuf);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 5);
				tmp1[5] = '\0';
			}
			else {
				convert_bin_to_str(tmpbuf, 5, tmp1);
				tmp1[10] = '\0';
			}			
			WRITE_WSC_PARAM(ptr, tmpbuf, "wep_key42 = %s\n", tmp1);
		}
		else {
			mib_get(gMIB_WLAN_WEP128_KEY1, (void *)&tmpbuf);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 13);
				tmp1[13] = '\0';
			}
			else {
				convert_bin_to_str(tmpbuf, 13, tmp1);
				tmp1[26] = '\0';
			}
			WRITE_WSC_PARAM(ptr, tmpbuf, "network_key2 = %s\n", tmp1);

			mib_get(gMIB_WLAN_WEP128_KEY2, (void *)&tmpbuf);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 13);
				tmp1[13] = '\0';
			}
			else {
				convert_bin_to_str(tmpbuf, 13, tmp1);
				tmp1[26] = '\0';
			}
			WRITE_WSC_PARAM(ptr, tmpbuf, "wep_key22 = %s\n", tmp1);

			mib_get(gMIB_WLAN_WEP128_KEY3, (void *)&tmpbuf);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 13);
				tmp1[13] = '\0';
			}
			else {
				convert_bin_to_str(tmpbuf, 13, tmp1);
				tmp1[26] = '\0';
			}
			WRITE_WSC_PARAM(ptr, tmpbuf, "wep_key32 = %s\n", tmp1);

			mib_get(gMIB_WLAN_WEP128_KEY4, (void *)&tmpbuf);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 13);
				tmp1[13] = '\0';
			}
			else {
				convert_bin_to_str(tmpbuf, 13, tmp1);
				tmp1[26] = '\0';
			}
			WRITE_WSC_PARAM(ptr, tmpbuf, "wep_key42 = %s\n", tmp1);
		}
	}
	else {
		mib_get(gMIB_WLAN_WPA_PSK, (void *)&tmp1);
		WRITE_WSC_PARAM(ptr, tmpbuf, "network_key2 = %s\n", tmp1);
		
	}
	intVal =2 ;
	WRITE_WSC_PARAM(ptr, tmpbuf, "#=====wlan1 end==========%d\n",intVal);
//	wlan_idx = 0;
#ifdef CONFIG_RTL_92D_DMDP
}
#endif
#endif //FOR_DUAL_BAND

/*
	mib_get(MIB_HW_MODEL_NUM, (void *)&tmp1);	
	WRITE_WSC_PARAM(ptr, tmpbuf, "model_num = \"%s\"\n", tmp1);	

	mib_get(MIB_HW_SERIAL_NUM, (void *)&tmp1);	
	WRITE_WSC_PARAM(ptr, tmpbuf, "serial_num = \"%s\"\n", tmp1);	
*/
	mib_get(gMIB_SNMP_SYS_NAME, (void *)&tmp1);	
	WRITE_WSC_PARAM(ptr, tmpbuf, "device_name = \"%s\"\n", tmp1);	

	len = (int)(((long)ptr)-((long)buf));
	
	fh = open(in, O_RDONLY);
	if (fh == -1) {
		printf("open() error [%s]!\n", in);
		goto ERROR;
	}

	lseek(fh, 0L, SEEK_SET);
	if (read(fh, ptr, status.st_size) != status.st_size) {		
		printf("read() error [%s]!\n", in);
		//return -1;	
		goto ERROR;
	}
	close(fh);

	// search UUID field, replace last 12 char with hw mac address
	ptr = strstr(ptr, "uuid =");
	if (ptr) {
		char tmp2[100];
		mib_get(gMIB_ELAN_MAC_ADDR/*MIB_HW_NIC0_ADDR*/, (void *)&tmp1);	
		convert_bin_to_str(tmp1, 6, tmp2);
		memcpy(ptr+27, tmp2, 12);		
	}

	fh = open(out, O_RDWR|O_CREAT|O_TRUNC);
	if (fh == -1) {
		printf("open() error [%s]!\n", out);
		//return -1;
		goto ERROR;
	}

	if (write(fh, buf, len+status.st_size) != len+status.st_size ) {
		printf("Write() file error [%s]!\n", out);
		goto ERROR;
	}
	close(fh);
	free(buf);
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(FOR_DUAL_BAND)
	wlan_idx = orig_wlan_idx;
#endif
#ifdef FOR_DUAL_BAND
//	wlan_idx = wlan_idx_orig;
#endif //FOR_DUAL_BAND
	return 0;

ERROR:
	if (buf) free(buf);
	if (-1 != fh) close(fh);
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(FOR_DUAL_BAND)
	wlan_idx = orig_wlan_idx;
#endif
#ifdef FOR_DUAL_BAND
//	wlan_idx = wlan_idx_orig;
#endif //FOR_DUAL_BAND
	return -1;
}

#endif // CONFIG_RTL867x


unsigned char *add_tlv(unsigned char *data, unsigned short id, int len, void *val)
{
	unsigned short size = htons(len);
	unsigned short tag = htons(id);

	memcpy(data, &tag, 2);
	memcpy(data+2, &size, 2);
	memcpy(data+4, val, len);

	return (data+4+len);
}

unsigned char *append(unsigned char *src, unsigned char *data, int data_len)
{
	memcpy(src, data, data_len);
	return (src+data_len);
}


int wlioctl_set_led(char *interface, int flag)
{
	unsigned char enable;
	int wps_led_active;
	char tmpbuf[50];

	if (flag == TURNKEY_LED_WSC_END) { // for turnkey LED_WSC_END
		if (pGlobalCtx->config_state == CONFIG_STATE_UNCONFIGURED)
			enable = (unsigned char)pGlobalCtx->WPS_END_LED_unconfig_GPIO_number;
		else
			enable = (unsigned char)pGlobalCtx->WPS_END_LED_config_GPIO_number;
	}
	else if (flag == TURNKEY_LED_WSC_START) {// for turnkey LED_WSC_START
		enable = (unsigned char)pGlobalCtx->WPS_START_LED_GPIO_number;
		pGlobalCtx->LedTimeout = 0;
	}
	else if (flag == TURNKEY_LED_PBC_OVERLAPPED) // for turnkey LED_PBC_OVERLAPPED
		enable = (unsigned char)pGlobalCtx->WPS_PBC_overlapping_GPIO_number;

	else if (flag == TURNKEY_LED_WSC_ERROR) {
		enable = (unsigned char)pGlobalCtx->WPS_ERROR_LED_GPIO_number;
		pGlobalCtx->LedTimeout = pGlobalCtx->WPS_ERROR_LED_time_out;
	}
	else if (flag == TURNKEY_LED_WSC_SUCCESS) {
		enable = (unsigned char)pGlobalCtx->WPS_SUCCESS_LED_GPIO_number;
		pGlobalCtx->LedTimeout = pGlobalCtx->WPS_SUCCESS_LED_time_out;		
	}
#ifdef AUTO_LOCK_DOWN
	else if (flag == TURNKEY_LED_LOCK_DOWN) {
		enable = (unsigned char)pGlobalCtx->WPS_ERROR_LED_GPIO_number;
	}
#endif
	else if (flag == TURNKEY_LED_WSC_NOP) {
		return 0;
	}
	else{
		enable = (unsigned char)flag;
	}
	sprintf(tmpbuf, "echo %x > /proc/gpio", enable);
//	UTIL_DEBUG("%s\n",tmpbuf);
	system(tmpbuf);

#ifdef CONFIG_PCIE_POWER_SAVING
	// inform Wi-Fi driver to enter/exit power saving mode
	wps_led_active = ((TURNKEY_LED_WSC_END == flag) ? 0 : 1);
	sprintf(tmpbuf, "iwpriv %s set_mib wps_led_active=%d", pGlobalCtx->wlan_interface_name, wps_led_active);
//	printf("%s\n", tmpbuf);
	system(tmpbuf);
#endif

	return 0;
}

int IssueDisconnect(char *interface, unsigned char *pucMacAddr, unsigned short reason)
{
	int skfd;
	int retVal = 0;
	struct iwreq wrq;
	DOT11_DISCONNECT_REQ Disconnect_Req;
#ifdef FOR_DUAL_BAND
	CTX_Tp pCtx = pGlobalCtx;
#endif //FOR_DUAL_BAND

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (skfd < 0) {
		DEBUG_ERR("socket() error!\n");
		return -1;
	}


	Disconnect_Req.EventId = DOT11_EVENT_DISCONNECT_REQ;

	Disconnect_Req.IsMoreEvent = 0;
	Disconnect_Req.Reason = reason;
	memcpy(Disconnect_Req.MACAddr,  pucMacAddr, ETHER_ADDRLEN);

#ifdef FOR_DUAL_BAND
	if(pCtx->is_ap){ // 1020
		if(pCtx->InterFaceComeIn == COME_FROM_WLAN1){
			strcpy(wrq.ifr_name, "wlan1");	
		}
		else if(pCtx->InterFaceComeIn == COME_FROM_WLAN0){
			strcpy(wrq.ifr_name, "wlan0");	
		}
		else{
			strcpy(wrq.ifr_name, "wlan0");	
		}
	}
	else{  // 1020
		strcpy(wrq.ifr_name, "wlan0");	
	}
#else //FOR_DUAL_BAND
	strcpy(wrq.ifr_name, interface);
#endif //FOR_DUAL_BAND
	
	wrq.u.data.pointer = (caddr_t)&Disconnect_Req;
	wrq.u.data.length = sizeof(DOT11_DISCONNECT_REQ);

	if(ioctl(skfd, SIOCGIWIND, &wrq) < 0) {
		DEBUG_ERR("WPS issues disassociation : ioctl error!\n");
          	retVal = -1;
	}

	close(skfd);
	UTIL_DEBUG("	WPS issues disassociation : reason code = %d\n", reason);
      	return retVal;

}

int wlioctl_set_wsc_ie(char *interface, char *data, int len, int id, int flag)
{
	int skfd;
	int retVal=0;
	struct iwreq wrq;
	DOT11_SET_WSCIE set_ie;	
#ifdef FOR_DUAL_BAND
	CTX_Tp pCtx = pGlobalCtx;
#endif
	
	if (len > MAX_WSC_IE_LEN) {
		DEBUG_ERR("WSC IE length too big [%d]!\n", len);
		return -1;
	}

#ifdef FOR_DUAL_BAND		
	if(!strcmp(interface , "wlan0")){
		if(pCtx->wlan0_wsc_disabled)			
		{
			UTIL_DEBUG("\n\n");
			return 1;
		}
	}else if(!strcmp(interface , "wlan1")){
		if(pCtx->wlan1_wsc_disabled){
			UTIL_DEBUG("\n\n");			
			return 1;
		}
	}
#endif

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (skfd < 0) {
		DEBUG_ERR("socket() error!\n");
		return -1;
	}

	set_ie.EventId = id;;
	set_ie.Flag = flag;
	set_ie.RSNIELen = len;
	memcpy(set_ie.RSNIE, data, len);

	strcpy(wrq.ifr_name, interface);	
	wrq.u.data.pointer = (caddr_t)&set_ie;
	wrq.u.data.length = sizeof(DOT11_SET_WSCIE);
	
	if (ioctl(skfd, SIOCGIWIND, &wrq) < 0)
	{
		printf("%s %d failed\n",__FUNCTION__,__LINE__);
		retVal = -1;
	}

	close(skfd);
	return retVal;	
}

int wlioctl_get_button_state(char *interface, int *pState)
{
	char tmpbuf;
	FILE *fp;
	char line[20];

	if ((fp = fopen("/proc/gpio", "r")) != NULL) {
		fgets(line, sizeof(line), fp);
		if (sscanf(line, "%c", &tmpbuf)) {
			if (tmpbuf == '0')
				*pState = 0;
			else if(tmpbuf == '1')
				*pState = 1;
		}
		else
			*pState = 0;
		fclose(fp);
	}
	else
		*pState = 0;

	return 0;
}

#ifdef P2P_SUPPORT
struct report_ssid_psk{
    char ssid_report[33];
    char ssid_len;
    char psk_report[65];
    char psk_len;    
};

 int wlioctl_report_ssid_psk(char *interface, char* SSID_in, char* psk_in)
 {
     int skfd;

     struct iwreq wrq;

     struct report_ssid_psk report_t;
     memset(&report_t,0,sizeof(struct report_ssid_psk));

     strcpy(report_t.ssid_report ,  SSID_in);
     strcpy(report_t.psk_report ,  psk_in);

     report_t.ssid_len= strlen(report_t.ssid_report);
     report_t.psk_len= strlen(report_t.psk_report);
     
     UTIL_DEBUG("ssid[%s],len=[%d]\n",SSID_in,report_t.ssid_len);
     UTIL_DEBUG("psk[%s],len[%d]\n",psk_in,report_t.psk_len);     
     
     skfd = socket(AF_INET, SOCK_DGRAM, 0);
 
     strcpy(wrq.ifr_name, interface);    

     wrq.u.data.pointer = (caddr_t)&report_t;

     wrq.u.data.length = sizeof(struct report_ssid_psk);
     
     if (ioctl(skfd, SIOCP2P_WSC_FAST_CONNECT, &wrq) < 0)
     {
         perror("ioctl[SIOCP2P_WSC_FAST_CONNECT]");
         close(skfd);
         return -1;
     }
     close(skfd); 
     return 0;
 }

#endif


#ifdef CLIENT_MODE
int wlioctl_scan_reqest(char *interface, int *pStatus)
{
    int skfd;
    struct iwreq wrq;
    unsigned char result[2];

    skfd = socket(AF_INET, SOCK_DGRAM, 0);

	strcpy(wrq.ifr_name, interface);	
	wrq.u.data.pointer = (caddr_t)result;
    wrq.u.data.length = sizeof(result);	
	if (ioctl(skfd, SIOCGIWRTLSCANREQ, &wrq) < 0) {
		perror("ioctl[SIOCGIWRTLSCANREQ]");
		close(skfd);
		return -1;
	}
	close(skfd);

    if ( result[0] == 0xff )
    	*pStatus = -1;
    else
		*pStatus = (int) result[0];

    return 0;
}

int wlioctl_scan_result(char *interface, SS_STATUS_Tp pStatus)
{
	int skfd;
	struct iwreq wrq;

	skfd = socket(AF_INET, SOCK_DGRAM, 0);

	strcpy(wrq.ifr_name, interface);	
	wrq.u.data.pointer = (caddr_t)pStatus;

	if (pStatus->number == 0)
		wrq.u.data.length = sizeof(SS_STATUS_T);
	else if (pStatus->number == 1)
		wrq.u.data.length = sizeof(pStatus->number);
	else
   	   	wrq.u.data.length = sizeof(SS_IE_T);

	if (ioctl(skfd, SIOCGIWRTLGETBSSDB, &wrq) < 0)
	{
		perror("ioctl[SIOCGIWRTLGETBSSDB]");
		close(skfd);
		return -1;
	}
	close(skfd);

	return 0;
}
#endif // CLIENT_MODE

/*now just fit small value*/
int wlioctl_get_mib(	char *interfacename , char* mibname ,int *result )
{

    int skfd;	
    struct iwreq wrq;
	unsigned char tmp[10];

    skfd = socket(AF_INET, SOCK_DGRAM, 0);	

	/* Set device name */	
	strcpy(wrq.ifr_name, interfacename);	
	strcpy(tmp,mibname);
	
    wrq.u.data.pointer = tmp;
    wrq.u.data.length = strlen(tmp);

	/* Do the request */
	if(ioctl(skfd, SIOCGIWRTLGETMIB, &wrq) < 0)
	{

		close(skfd);
		return -1;	
	}
  	
	close(skfd);
	*result = *(unsigned int*)tmp;
	return 0;
  
}
void client_set_WlanDriver_WscEnable(const CTX_Tp pCtx, const int wps_enabled)
{
	unsigned char tmpbuf[100];
	
	sprintf(tmpbuf,"iwpriv %s set_mib wsc_enable=%d", pCtx->wlan_interface_name, wps_enabled);
	system(tmpbuf);
}


int derive_key(CTX_Tp pCtx, STA_CTX_Tp pSta)
{
	int size;
	unsigned char tmpbuf[BYTE_LEN_640B], *pmac, *p, tmp1[100];
	DH *dh_our, *dh_peer;

	if (pCtx->role == REGISTRAR) {
		dh_our = pSta->dh_registrar;
		dh_peer = pSta->dh_enrollee;
#if 0 // for Intel SDK
		pmac = pSta->addr;
#else
		pmac = pSta->msg_addr;
#endif
	}
	else {
		dh_peer = pSta->dh_registrar;
		dh_our = pSta->dh_enrollee;
		
#ifdef FOR_DUAL_BAND		
		if(pCtx->is_ap){
			if(pCtx->InterFaceComeIn == COME_FROM_WLAN1){
				pmac = pCtx->our_addr2;
			}
			else if(pCtx->InterFaceComeIn == COME_FROM_WLAN0){
				pmac = pCtx->our_addr;
			}
			else{
				pmac = pCtx->our_addr;
			}
		}
		else {
			pmac = pCtx->our_addr;
		}
#else //FOR_DUAL_BAND
		pmac = pCtx->our_addr;
#endif //FOR_DUAL_BAND
	}

	size = DH_compute_key(pSta->dh_shared_key, dh_peer->p, dh_our);
	if (size < 0) {
		DEBUG_ERR("DH_compute_key error!\n");
		return size;
	}
	SHA256(pSta->dh_shared_key, size, pSta->dh_digest_key);

	memcpy(tmpbuf, pSta->nonce_enrollee, NONCE_LEN);	
	p = append(&tmpbuf[NONCE_LEN], pmac, ETHER_ADDRLEN);
	p = append(p, pSta->nonce_registrar, NONCE_LEN);
	size =(int)(((unsigned long)p) -  ((unsigned long)tmpbuf));

	hmac_sha256(tmpbuf, size, pSta->dh_digest_key, BYTE_LEN_256B, tmp1, &size);

	wsc_kdf(tmp1, BYTE_LEN_256B, KDF_STRING, strlen(KDF_STRING), BYTE_LEN_640B*8, tmpbuf);

	memcpy(pSta->auth_key, tmpbuf, BYTE_LEN_256B);
	memcpy(pSta->key_wrap_key, &tmpbuf[BYTE_LEN_256B], BYTE_LEN_128B);
	memcpy(pSta->EMSK, &tmpbuf[BYTE_LEN_256B+BYTE_LEN_128B], BYTE_LEN_256B);

#ifdef DEBUG
	if (pCtx->debug2) {
		debug_out("DH shared key", pSta->dh_shared_key, size);
		debug_out("DH SHA256 key", pSta->dh_digest_key, BYTE_LEN_256B);
		debug_out("KDK", tmp1, BYTE_LEN_256B);
		debug_out("AuthKey", pSta->auth_key, BYTE_LEN_256B);
		debug_out("KeyWrapKey", pSta->key_wrap_key, BYTE_LEN_128B);
		debug_out("EMSK", pSta->EMSK, BYTE_LEN_256B);
	}
#endif		

	return 0;
}

#ifndef CONFIG_RTL867x // original code, andrew
int write_param_to_flash(CTX_Tp pCtx, int is_local)
{
	char tmpbuf[120];// orig =100
	FILE *fp;	
	int encrypt, cipher, is_psk=0;

	fp = fopen(PARAM_TEMP_FILE, "w");

	if (is_local)
		sprintf(tmpbuf, "SSID=\"%s\"\n", pCtx->SSID);		
	else
		sprintf(tmpbuf, "SSID=\"%s\"\n", pCtx->assigned_ssid);
	fputs(tmpbuf, fp);	

	if (is_local)
		sprintf(tmpbuf, "WSC_SSID=\"%s\"\n", pCtx->SSID);		
	else
		sprintf(tmpbuf, "WSC_SSID=\"%s\"\n", pCtx->assigned_ssid);
	fputs(tmpbuf, fp);	
			
	encrypt = 0;
	if (is_local) {
		if (pCtx->auth_type_flash & AUTH_WPAPSK)			
			encrypt |= 2;
		if (pCtx->auth_type_flash & AUTH_WPA2PSK)			
			encrypt |= 4;		
	}
	else {
		sprintf(tmpbuf, "AUTH_TYPE=%d\n", 2);

		if(pCtx->is_ap){
			if (pCtx->assigned_auth_type & AUTH_WPAPSK)			
				encrypt |= 2;
			
			if (pCtx->assigned_auth_type & AUTH_WPA2PSK)			
				encrypt |= 4;
		}else{
			if (pCtx->assigned_auth_type & AUTH_WPAPSK) 		
				encrypt = 2;
			
			if (pCtx->assigned_auth_type & AUTH_WPA2PSK)			
				encrypt = 4;
		}

		if (pCtx->assigned_auth_type == AUTH_OPEN && pCtx->assigned_encrypt_type == ENCRYPT_WEP)
			encrypt = 1;
		else if (pCtx->assigned_auth_type == AUTH_SHARED && pCtx->assigned_encrypt_type == ENCRYPT_WEP) {
			encrypt = 1;
			sprintf(tmpbuf, "AUTH_TYPE=%d\n", 1);
		}
		fputs(tmpbuf, fp);
	}
	sprintf(tmpbuf, "ENCRYPT=%d\n", encrypt);
	fputs(tmpbuf, fp);			

	encrypt = 0;
	if (is_local){
		encrypt = pCtx->auth_type_flash;
	}else{ 
		if(pCtx->is_ap){			
			encrypt = pCtx->assigned_auth_type;
		}else{
			/*under client mode just choose highest security set*/
			encrypt = pCtx->assigned_auth_type;
			
			if(pCtx->assigned_auth_type == AUTH_WPA2PSKMIXED)
				encrypt = AUTH_WPA2PSK ;			
		}
	}
	
	sprintf(tmpbuf, "WSC_AUTH=%d\n", encrypt);
	fputs(tmpbuf, fp);

	if (is_local) {
		if (pCtx->auth_type_flash & AUTH_WPAPSK || pCtx->auth_type_flash & AUTH_WPA2PSK)
			is_psk = 1;			
	}
	else {
		if (pCtx->assigned_auth_type & AUTH_WPAPSK || pCtx->assigned_auth_type & AUTH_WPA2PSK)
			is_psk = 1;
	}

	//if (is_psk) 	
	fputs("WPA_AUTH=2\n", fp);
	//else		
		//fputs("WPA_AUTH=1\n", fp);

	if (is_psk) {
		if (is_local) {
			sprintf(tmpbuf, "WPA_PSK=\"%s\"\n", pCtx->network_key);
			fputs(tmpbuf, fp);
			if (strlen(pCtx->network_key) == 64)
				fputs("PSK_FORMAT=1\n", fp);
			else
				fputs("PSK_FORMAT=0\n", fp);	
		}
		else {
			sprintf(tmpbuf, "WPA_PSK=\"%s\"\n", pCtx->assigned_network_key);
			fputs(tmpbuf, fp);
			if (strlen(pCtx->assigned_network_key) == 64)
				fputs("PSK_FORMAT=1\n", fp);
			else
				fputs("PSK_FORMAT=0\n", fp);	
		}
	}

	if (is_local) {
		sprintf(tmpbuf, "WSC_PSK=\"%s\"\n", pCtx->network_key);
		fputs(tmpbuf, fp);
	}
	else {
		sprintf(tmpbuf, "WSC_PSK=\"%s\"\n", pCtx->assigned_network_key);
		fputs(tmpbuf, fp);
	}

	cipher = 0;
	if (is_local) {
		if (pCtx->encrypt_type_flash & ENCRYPT_TKIP)
			cipher |= 1;
		
		if (pCtx->encrypt_type_flash & ENCRYPT_AES)
			cipher |= 2;		
	}
	else {
		if(pCtx->is_ap){
			if (pCtx->assigned_encrypt_type & ENCRYPT_TKIP)
				cipher |= 1;

			if (pCtx->assigned_encrypt_type & ENCRYPT_AES)
				cipher |= 2;
		}else{
			if (pCtx->assigned_encrypt_type & ENCRYPT_TKIP)
				cipher = 1;
			
			if (pCtx->assigned_encrypt_type & ENCRYPT_AES)
				cipher = 2;
		}
	}
	sprintf(tmpbuf, "WPA_CIPHER_SUITE=%d\n", cipher);
	fputs(tmpbuf, fp);				
	sprintf(tmpbuf, "WPA2_CIPHER_SUITE=%d\n", cipher);
	fputs(tmpbuf, fp);				

	cipher = 0;
	if (is_local)
		cipher = pCtx->encrypt_type_flash;
	else {
		cipher = pCtx->assigned_encrypt_type;
		if (pCtx->assigned_encrypt_type == ENCRYPT_WEP) {
			sprintf(tmpbuf, "WEP=%d\n", pCtx->assigned_wep_key_len);
			fputs(tmpbuf, fp);
			if (pCtx->assigned_wep_key_len == WEP64) {
				sprintf(tmpbuf, "WEP64_KEY1=%s\n", pCtx->assigned_wep_key_1);
				fputs(tmpbuf, fp);
				sprintf(tmpbuf, "WEP64_KEY2=%s\n", pCtx->assigned_wep_key_2);
				fputs(tmpbuf, fp);
				sprintf(tmpbuf, "WEP64_KEY3=%s\n", pCtx->assigned_wep_key_3);
				fputs(tmpbuf, fp);
				sprintf(tmpbuf, "WEP64_KEY4=%s\n", pCtx->assigned_wep_key_4);
				fputs(tmpbuf, fp);
			}
			else {
				sprintf(tmpbuf, "WEP128_KEY1=%s\n", pCtx->assigned_wep_key_1);
				fputs(tmpbuf, fp);
				sprintf(tmpbuf, "WEP128_KEY2=%s\n", pCtx->assigned_wep_key_2);
				fputs(tmpbuf, fp);
				sprintf(tmpbuf, "WEP128_KEY3=%s\n", pCtx->assigned_wep_key_3);
				fputs(tmpbuf, fp);
				sprintf(tmpbuf, "WEP128_KEY4=%s\n", pCtx->assigned_wep_key_4);
				fputs(tmpbuf, fp);
			}
			sprintf(tmpbuf, "WEP_DEFAULT_KEY=%d\n", pCtx->assigned_wep_transmit_key-1);
			fputs(tmpbuf, fp);
			sprintf(tmpbuf, "WEP_KEY_TYPE=%d\n", pCtx->assigned_wep_key_format);
			fputs(tmpbuf, fp);
		}
		else {
			sprintf(tmpbuf, "WEP=%d\n", WEP_DISABLED);
			fputs(tmpbuf, fp);
		}
	}
	sprintf(tmpbuf, "WSC_ENC=%d\n", cipher);
	fputs(tmpbuf, fp);	

	if (pCtx->is_ap) {
		if (is_local) {
			sprintf(tmpbuf, "WSC_CONFIGBYEXTREG=%d\n", CONFIG_BY_INTERNAL_REGISTRAR);
			fputs(tmpbuf, fp);
		}
		else {
			sprintf(tmpbuf, "WSC_CONFIGBYEXTREG=%d\n", CONFIG_BY_EXTERNAL_REGISTRAR);
			fputs(tmpbuf, fp);
		}
		/*if (pCtx->manual_config) {
			sprintf(tmpbuf, "WSC_MANUAL_ENABLED=%d\n", 0);
			fputs(tmpbuf, fp);
		}
		sprintf(tmpbuf, "WSC_CONFIGURED=%d\n", 1);
		fputs(tmpbuf, fp);	*/

	}
		
	sprintf(tmpbuf, "WSC_CONFIGURED=%d\n", 1);
	fputs(tmpbuf, fp);
		
	fclose(fp);

	if (pCtx->No_ifname_for_flash_set != 2) {
		sprintf(tmpbuf, "%s -param_file %s %s", WRITE_FLASH_PROG, 
				pCtx->wlan_interface_name, PARAM_TEMP_FILE);
		system(tmpbuf);

#ifdef FOR_DUAL_BAND		// sync configured state
		if(pCtx->is_ap && !pCtx->wlan1_wsc_disabled){
			#if 0	/*no mater local(auto gener) or foreign(from ER) ;we just apply	*/ 
			if(is_local){
				/*is config is autogenerate my self;just apply to my self ,don't apply to the other one
				but configured state need sync*/ 
				sprintf(tmpbuf, "flash set WLAN1_WSC_CONFIGURED 1\n");
				system(tmpbuf);
			}else
			#endif
			{
				/*is config is assigned by ER ;apply config to both two inteface*/ 
				sprintf(tmpbuf, "%s -param_file %s %s", WRITE_FLASH_PROG, 
				pCtx->wlan_interface_name2, PARAM_TEMP_FILE);
				UTIL_DEBUG("issue cmd \"%s\"\n",tmpbuf);				
				system(tmpbuf);
			}		
		}
#endif
	}
	return REINIT_SYS;
}

#ifdef FOR_DUAL_BAND
int write_param_to_flash2(CTX_Tp pCtx, int is_local)
{
	char tmpbuf[120];
	FILE *fp;	
	int encrypt, cipher, is_psk=0;


	#ifdef FULL_SECURITY_CLONE	
	int  cipher2 = 0; 
	#endif
	
	UTIL_DEBUG("\n\n\n");

	
	fp = fopen(PARAM_TEMP_FILE, "w");

	if (is_local)
		sprintf(tmpbuf, "SSID=\"%s\"\n", pCtx->SSID2);		
	else
		sprintf(tmpbuf, "SSID=\"%s\"\n", pCtx->assigned_ssid);
	fputs(tmpbuf, fp);	

	if (is_local)
		sprintf(tmpbuf, "WSC_SSID=\"%s\"\n", pCtx->SSID2);		
	else
		sprintf(tmpbuf, "WSC_SSID=\"%s\"\n", pCtx->assigned_ssid);
	fputs(tmpbuf, fp);	
			
	encrypt = 0;
	if (is_local) {
		if (pCtx->auth_type_flash2 & AUTH_WPAPSK)			
			encrypt |= 2;
		if (pCtx->auth_type_flash2 & AUTH_WPA2PSK)			
			encrypt |= 4;		
	}
	else {
		sprintf(tmpbuf, "AUTH_TYPE=%d\n", 2);
		
		if(pCtx->is_ap){
			if (pCtx->assigned_auth_type & AUTH_WPAPSK)			
				encrypt |= 2;

			if (pCtx->assigned_auth_type & AUTH_WPA2PSK)			
				encrypt |= 4;
		}else{
			if (pCtx->assigned_auth_type & AUTH_WPAPSK) 		
				encrypt = 2;
			
			if (pCtx->assigned_auth_type & AUTH_WPA2PSK)			
				encrypt = 4;
		}
		
		if (pCtx->assigned_auth_type == AUTH_OPEN && pCtx->assigned_encrypt_type == ENCRYPT_WEP)
			encrypt = 1;
		else if (pCtx->assigned_auth_type == AUTH_SHARED && pCtx->assigned_encrypt_type == ENCRYPT_WEP) {
			encrypt = 1;
			sprintf(tmpbuf, "AUTH_TYPE=%d\n", 1);
		}
		fputs(tmpbuf, fp);
	}
	sprintf(tmpbuf, "ENCRYPT=%d\n", encrypt);
	fputs(tmpbuf, fp);			

	encrypt = 0;
	if (is_local){
		encrypt = pCtx->auth_type_flash2;
	}else{ 
		if(pCtx->is_ap){
			encrypt = pCtx->assigned_auth_type;
		}else{

			/*under client mode just choose highest security set*/
			encrypt = pCtx->assigned_auth_type;
			
			if(pCtx->assigned_auth_type == AUTH_WPA2PSKMIXED)
				encrypt = AUTH_WPA2PSK ;
		}
	}
	sprintf(tmpbuf, "WSC_AUTH=%d\n", encrypt);
	fputs(tmpbuf, fp);

	if (is_local) {
		if (pCtx->auth_type_flash2 & AUTH_WPAPSK || pCtx->auth_type_flash2 & AUTH_WPA2PSK)
			is_psk = 1;			
	}
	else {
		if (pCtx->assigned_auth_type & AUTH_WPAPSK || pCtx->assigned_auth_type & AUTH_WPA2PSK)
			is_psk = 1;
	}


	fputs("WPA_AUTH=2\n", fp);

	if (is_psk) {
		if (is_local) {
			sprintf(tmpbuf, "WPA_PSK=\"%s\"\n", pCtx->network_key2);
			fputs(tmpbuf, fp);
			if (strlen(pCtx->network_key2) == 64)
				fputs("PSK_FORMAT=1\n", fp);
			else
				fputs("PSK_FORMAT=0\n", fp);	
		}
		else {
			sprintf(tmpbuf, "WPA_PSK=\"%s\"\n", pCtx->assigned_network_key);
			fputs(tmpbuf, fp);
			if (strlen(pCtx->assigned_network_key) == 64)
				fputs("PSK_FORMAT=1\n", fp);
			else
				fputs("PSK_FORMAT=0\n", fp);	
		}
	}

	if (is_local) {
		sprintf(tmpbuf, "WSC_PSK=\"%s\"\n", pCtx->network_key2);
		fputs(tmpbuf, fp);
	}
	else {
		sprintf(tmpbuf, "WSC_PSK=\"%s\"\n", pCtx->assigned_network_key);
		fputs(tmpbuf, fp);
	}

	cipher = 0;
	if (is_local) {
		if (pCtx->encrypt_type_flash2 & ENCRYPT_TKIP)
			cipher |= 1;
		if (pCtx->encrypt_type_flash2 & ENCRYPT_AES)
			cipher |= 2;		
	}
	else {
		if(pCtx->is_ap){	
			if (pCtx->assigned_encrypt_type & ENCRYPT_TKIP)
				cipher |= 1;

			if (pCtx->assigned_encrypt_type & ENCRYPT_AES)
				cipher |= 2;
		}else{
			if (pCtx->assigned_encrypt_type & ENCRYPT_TKIP)
				cipher = 1;
			
			if (pCtx->assigned_encrypt_type & ENCRYPT_AES)
				cipher = 2;
		}		
	}


		sprintf(tmpbuf, "WPA_CIPHER_SUITE=%d\n", cipher);
		fputs(tmpbuf, fp);				
		sprintf(tmpbuf, "WPA2_CIPHER_SUITE=%d\n", cipher);
		fputs(tmpbuf, fp);						



	

	cipher = 0;
	if (is_local)
		cipher = pCtx->encrypt_type_flash2;
	else {
		cipher = pCtx->assigned_encrypt_type;
		if (pCtx->assigned_encrypt_type == ENCRYPT_WEP) {
			sprintf(tmpbuf, "WEP=%d\n", pCtx->assigned_wep_key_len);
			fputs(tmpbuf, fp);
			if (pCtx->assigned_wep_key_len == WEP64) {
				sprintf(tmpbuf, "WEP64_KEY1=%s\n", pCtx->assigned_wep_key_1);
				fputs(tmpbuf, fp);
				sprintf(tmpbuf, "WEP64_KEY2=%s\n", pCtx->assigned_wep_key_2);
				fputs(tmpbuf, fp);
				sprintf(tmpbuf, "WEP64_KEY3=%s\n", pCtx->assigned_wep_key_3);
				fputs(tmpbuf, fp);
				sprintf(tmpbuf, "WEP64_KEY4=%s\n", pCtx->assigned_wep_key_4);
				fputs(tmpbuf, fp);
			}
			else {
				sprintf(tmpbuf, "WEP128_KEY1=%s\n", pCtx->assigned_wep_key_1);
				fputs(tmpbuf, fp);
				sprintf(tmpbuf, "WEP128_KEY2=%s\n", pCtx->assigned_wep_key_2);
				fputs(tmpbuf, fp);
				sprintf(tmpbuf, "WEP128_KEY3=%s\n", pCtx->assigned_wep_key_3);
				fputs(tmpbuf, fp);
				sprintf(tmpbuf, "WEP128_KEY4=%s\n", pCtx->assigned_wep_key_4);
				fputs(tmpbuf, fp);
			}
			sprintf(tmpbuf, "WEP_DEFAULT_KEY=%d\n", pCtx->assigned_wep_transmit_key-1);
			fputs(tmpbuf, fp);
			sprintf(tmpbuf, "WEP_KEY_TYPE=%d\n", pCtx->assigned_wep_key_format);
			fputs(tmpbuf, fp);
		}
		else {
			sprintf(tmpbuf, "WEP=%d\n", WEP_DISABLED);
			fputs(tmpbuf, fp);
		}
	}


		sprintf(tmpbuf, "WSC_ENC=%d\n", cipher);
		fputs(tmpbuf, fp);	


	#ifdef FULL_SECURITY_CLONE
	if(!pCtx->is_ap){
		/*
		UTIL_DEBUG("Switch to AP mode and reinit...\n");	
		sprintf(tmpbuf, "WLAN0_MODE=%d\n", 0);
		fputs(tmpbuf, fp);
		*/		
		sprintf(tmpbuf, "WLAN0_BAND=%d\n", pCtx->ss_status.bssdb[pCtx->join_idx].network);
		fputs(tmpbuf, fp);			
	}
	#endif
	
		
	if (pCtx->is_ap) {
		if (is_local) {
			sprintf(tmpbuf, "WSC_CONFIGBYEXTREG=%d\n", CONFIG_BY_INTERNAL_REGISTRAR);
			fputs(tmpbuf, fp);
		}
		else {
			sprintf(tmpbuf, "WSC_CONFIGBYEXTREG=%d\n", CONFIG_BY_EXTERNAL_REGISTRAR);
			fputs(tmpbuf, fp);
		}
		/*if (pCtx->manual_config) {
			sprintf(tmpbuf, "WSC_MANUAL_ENABLED=%d\n", 0);
			fputs(tmpbuf, fp);
		}
		sprintf(tmpbuf, "WSC_CONFIGURED=%d\n", 1);
		fputs(tmpbuf, fp);	*/

	}
	
	sprintf(tmpbuf, "WSC_CONFIGURED=%d\n", 1);
	fputs(tmpbuf, fp);
		
	fclose(fp);

	if (pCtx->No_ifname_for_flash_set != 2) {
		sprintf(tmpbuf, "%s -param_file %s %s", WRITE_FLASH_PROG, 
				pCtx->wlan_interface_name2, PARAM_TEMP_FILE);
		system(tmpbuf);

#ifdef FOR_DUAL_BAND		/*sync configured state to another interface wlan0*/
		if(pCtx->is_ap && !pCtx->wlan0_wsc_disabled){
			#if 0	// no mater local(auto gener) or foreign(from ER) we just apply
			if(is_local){
				/*case 1:is config is autogenerate my self;
				just apply to my self ,don't apply to the other one
				but configured state need sync*/ 
				sprintf(tmpbuf, "flash set WLAN0_WSC_CONFIGURED 1\n");
				system(tmpbuf);
			}else
			#endif
			{	
				/*case2:is config is assigned by ER ;apply config to both two inteface*/ 
				sprintf(tmpbuf, "%s -param_file %s %s", WRITE_FLASH_PROG, 
				pCtx->wlan_interface_name, PARAM_TEMP_FILE);
				UTIL_DEBUG("issue cmd \"%s\"\n",tmpbuf);
				system(tmpbuf);
			}		
		}
#endif
	}
	return REINIT_SYS;
}

#endif

#else

int write_param_to_flash(CTX_Tp pCtx, int is_local)
{
	char tmpbuf[100];
	//FILE *fp;	
	int encrypt, cipher, is_psk=0, len=0;
	unsigned char byte;
#ifdef FOR_DUAL_BAND
//	int orig_wlan_idx = wlan_idx;
#endif //FOR_DUAL_BAND
	//prevent rebooting if assigned setting is the same as local one
	//the only exception is WEP

	if (!is_local && pCtx->is_ap) {
		
// Reset manual flag when configered by external registrar
		//if (pCtx->manual_config) {
			//pCtx->manual_config = 0;
			//sprintf(tmpbuf, "flash set %s WSC_MANUAL_ENABLED 0", pCtx->wlan_interface_name);
			//system(tmpbuf);			
		//}
//--------------------------------- david+2007-01-26

		if ((pCtx->assigned_auth_type != AUTH_OPEN) ||
			(pCtx->assigned_auth_type == AUTH_OPEN &&
			pCtx->assigned_encrypt_type == ENCRYPT_NONE)) {
			if ((len = strlen(pCtx->SSID)) == strlen(pCtx->assigned_ssid) &&
				(len = strlen(pCtx->network_key)) == strlen(pCtx->assigned_network_key) &&
					!memcmp(pCtx->SSID, pCtx->assigned_ssid, strlen(pCtx->SSID)) &&
					!memcmp(pCtx->network_key, pCtx->assigned_network_key, strlen(pCtx->network_key)) &&
					pCtx->auth_type_flash == pCtx->assigned_auth_type &&
					pCtx->encrypt_type_flash == pCtx->assigned_encrypt_type) {
				if (pCtx->config_state == CONFIG_STATE_UNCONFIGURED) {
					//sprintf(tmpbuf, "flash set %s WSC_CONFIGURED 1", pCtx->wlan_interface_name);
					//andrew sprintf(tmpbuf, "flash set WSC_CONFIGURED 1");
					//andrew system(tmpbuf);				
					byte = 1;
					MIB_SET(gMIB_WSC_CONFIGURED, &byte);
				}
				//sprintf(tmpbuf, "flash set %s WSC_CONFIGBYEXTREG %d", pCtx->wlan_interface_name, CONFIG_BY_EXTERNAL_REGISTRAR);
				//system(tmpbuf);	
				return SYNC_FLASH_PARAMETER;
			}
		}
	}

#ifdef FOR_DUAL_BAND
//	wlan_idx = 0;
#endif //FOR_DUAL_BAND
	//fp = fopen(PARAM_TEMP_FILE, "w");

	if (is_local)
		//sprintf(tmpbuf, "SSID=%s\n", pCtx->SSID);		
		MIB_SET(gMIB_WLAN_SSID, pCtx->SSID);
	else
		//sprintf(tmpbuf, "SSID=%s\n", );
		MIB_SET(gMIB_WLAN_SSID, pCtx->assigned_ssid);
	//fputs(tmpbuf, fp);	

	if (is_local)
		//sprintf(tmpbuf, "WSC_SSID=%s\n", pCtx->SSID);	
		MIB_SET(gMIB_WSC_SSID, pCtx->SSID);
	else
		//sprintf(tmpbuf, "WSC_SSID=%s\n", pCtx->assigned_ssid);
		MIB_SET(gMIB_WSC_SSID, pCtx->assigned_ssid);
	//fputs(tmpbuf, fp);	
			
	encrypt = 0;
	if (is_local) {
		if (pCtx->auth_type_flash & AUTH_WPAPSK)			
			encrypt |= 2;
		if (pCtx->auth_type_flash & AUTH_WPA2PSK)			
			encrypt |= 4;		
	}
	else {
		sprintf(tmpbuf, "AUTH_TYPE=%d\n", 2);
		
		if(pCtx->is_ap){
			if (pCtx->assigned_auth_type & AUTH_WPAPSK)		
				encrypt |= 2;
			
			if (pCtx->assigned_auth_type & AUTH_WPA2PSK)
				encrypt |= 4;
		}else{
			if (pCtx->assigned_auth_type & AUTH_WPAPSK)
				encrypt = 2;
			
			if (pCtx->assigned_auth_type & AUTH_WPA2PSK)	
				encrypt = 4;
		}
		
		if (pCtx->assigned_auth_type == AUTH_OPEN && pCtx->assigned_encrypt_type == ENCRYPT_WEP)
			encrypt = 1;
		else if (pCtx->assigned_auth_type == AUTH_SHARED && pCtx->assigned_encrypt_type == ENCRYPT_WEP) {
			encrypt = 1;
			//sprintf(tmpbuf, "AUTH_TYPE=%d\n", 1);
			byte = 1;
			MIB_SET(gMIB_WLAN_AUTH_TYPE, &byte);
		}
		//fputs(tmpbuf, fp);
	}
	//sprintf(tmpbuf, "WLAN_ENCRYPT=%d\n", encrypt);
	//fputs(tmpbuf, fp);			
	byte = encrypt;
	MIB_SET(gMIB_WLAN_ENCRYPT, &byte);

	encrypt = 0;
	if (is_local){
		encrypt = pCtx->auth_type_flash;
	}else{ 
		if(pCtx->is_ap){			
			encrypt = pCtx->assigned_auth_type;
		}else{
			/*under client mode just choose highest security set*/
			encrypt = pCtx->assigned_auth_type;
			
			if(pCtx->assigned_auth_type == AUTH_WPA2PSKMIXED)
				encrypt = AUTH_WPA2PSK ;			
		}
	}
	//sprintf(tmpbuf, "WSC_AUTH=%d\n", encrypt);
	//fputs(tmpbuf, fp);
	byte = encrypt;
	MIB_SET(gMIB_WSC_AUTH, &byte);

	if (is_local) {
		if (pCtx->auth_type_flash & AUTH_WPAPSK || pCtx->auth_type_flash & AUTH_WPA2PSK)
			is_psk = 1;			
	}
	else {
		if (pCtx->assigned_auth_type & AUTH_WPAPSK || pCtx->assigned_auth_type & AUTH_WPA2PSK)
			is_psk = 1;
	}

	//if (is_psk) 	
	//fputs("WLAN_WPA_AUTH=2\n", fp);
	byte = 2;
	MIB_SET(gMIB_WLAN_WPA_AUTH, &byte);
	//else		
		//fputs("WLAN_WPA_AUTH=1\n", fp);

	if (is_psk) {
		if (is_local) {
			//sprintf(tmpbuf, "WLAN_WPA_PSK=%s\n", pCtx->network_key);
			//fputs(tmpbuf, fp);
			MIB_SET(gMIB_WLAN_WPA_PSK, pCtx->network_key);
			if (strlen(pCtx->network_key) == 64) {
				//fputs("WLAN_PSK_FORMAT=1\n", fp);
				byte = 1;
				MIB_SET(gMIB_WLAN_WPA_PSK_FORMAT, &byte);				
			} else {
				//fputs("WLAN_PSK_FORMAT=0\n", fp);	
				byte = 0;
				MIB_SET(gMIB_WLAN_WPA_PSK_FORMAT, &byte);
			}
		}
		else {
			//sprintf(tmpbuf, "WLAN_WPA_PSK=%s\n", pCtx->assigned_network_key);
			//fputs(tmpbuf, fp);
			MIB_SET(gMIB_WLAN_WPA_PSK, pCtx->assigned_network_key);
			if (strlen(pCtx->assigned_network_key) == 64) {
				//fputs("WLAN_PSK_FORMAT=1\n", fp);
				byte = 1;
				MIB_SET(gMIB_WLAN_WPA_PSK_FORMAT, &byte);
			} else {
				//fputs("WLAN_PSK_FORMAT=0\n", fp);	
				byte = 0;
				MIB_SET(gMIB_WLAN_WPA_PSK_FORMAT, &byte);
			}
		}
	}

	if (is_local) {
		//sprintf(tmpbuf, "WSC_PSK=%s\n", pCtx->network_key);
		//fputs(tmpbuf, fp);
		MIB_SET(gMIB_WSC_PSK, pCtx->network_key);
	}
	else {
		//sprintf(tmpbuf, "WSC_PSK=%s\n", pCtx->assigned_network_key);
		//fputs(tmpbuf, fp);
		MIB_SET(gMIB_WSC_PSK, pCtx->assigned_network_key);
	}

	cipher = 0;
	if (is_local) {
		if (pCtx->encrypt_type_flash & ENCRYPT_TKIP)
			cipher |= 1;
		if (pCtx->encrypt_type_flash & ENCRYPT_AES)
			cipher |= 2;		
	}
	else {
		if (pCtx->assigned_encrypt_type & ENCRYPT_TKIP)
			cipher |= 1;
		if (pCtx->assigned_encrypt_type & ENCRYPT_AES)
			cipher |= 2;
	}
	//sprintf(tmpbuf, "WLAN_WPA_CIPHER_SUITE=%d\n", cipher);
	//fputs(tmpbuf, fp);			
#ifndef WPA2_MIXED_2MODE_ONLY	
	byte = cipher;
	MIB_SET(gMIB_WLAN_WPA_CIPHER_SUITE, &byte);
	//sprintf(tmpbuf, "WPA2_CIPHER_SUITE=%d\n", cipher);
	//fputs(tmpbuf, fp);				
	MIB_SET(gMIB_WLAN_WPA2_CIPHER_SUITE, &byte);
#else
	byte = 1;		//tkip
	MIB_SET(gMIB_WLAN_WPA_CIPHER_SUITE, &byte);
	byte = 2;		//aes
	MIB_SET(gMIB_WLAN_WPA2_CIPHER_SUITE, &byte);
#endif

	cipher = 0;
	if (is_local)
		cipher = pCtx->encrypt_type_flash;
	else {
		cipher = pCtx->assigned_encrypt_type;
		if (pCtx->assigned_encrypt_type == ENCRYPT_WEP) {
			unsigned char wepkey[20];
			//sprintf(tmpbuf, "WEP=%d\n", pCtx->assigned_wep_key_len);
			//fputs(tmpbuf, fp);
			byte = pCtx->assigned_wep_key_len;
			MIB_SET(gMIB_WLAN_WEP, &byte);
			if (pCtx->assigned_wep_key_len == WEP64) {				
				string_to_hex(pCtx->assigned_wep_key_1, wepkey, 10);
				MIB_SET(gMIB_WLAN_WEP64_KEY1, wepkey);
				string_to_hex(pCtx->assigned_wep_key_2, wepkey, 10);
				MIB_SET(gMIB_WLAN_WEP64_KEY1, wepkey);
				string_to_hex(pCtx->assigned_wep_key_3, wepkey, 10);
				MIB_SET(gMIB_WLAN_WEP64_KEY1, wepkey);
				string_to_hex(pCtx->assigned_wep_key_4, wepkey, 10);
				MIB_SET(gMIB_WLAN_WEP64_KEY1, wepkey);
			}
			else {
				string_to_hex(pCtx->assigned_wep_key_1, wepkey, 26);
				MIB_SET(gMIB_WLAN_WEP128_KEY1, wepkey);
				string_to_hex(pCtx->assigned_wep_key_2, wepkey, 26);
				MIB_SET(gMIB_WLAN_WEP128_KEY1, wepkey);
				string_to_hex(pCtx->assigned_wep_key_3, wepkey, 26);
				MIB_SET(gMIB_WLAN_WEP128_KEY1, wepkey);
				string_to_hex(pCtx->assigned_wep_key_4, wepkey, 26);
				MIB_SET(gMIB_WLAN_WEP128_KEY1, wepkey);
			}
			byte = pCtx->assigned_wep_transmit_key-1;
			MIB_SET(gMIB_WLAN_WEP_DEFAULT_KEY, &byte);			
			byte = pCtx->assigned_wep_key_format;
			MIB_SET(gMIB_WLAN_WEP_KEY_TYPE, &byte);
		}
		else {
			byte = WEP_DISABLED;
			MIB_SET(gMIB_WLAN_WEP, &byte);
		}
	}
	byte = cipher;
	MIB_SET(gMIB_WSC_ENC, &byte);

	if (pCtx->is_ap) {
		if (is_local) {
			byte = CONFIG_BY_INTERNAL_REGISTRAR;
			MIB_SET(gMIB_WSC_CONFIG_BY_EXT_REG, &byte);
		}
		else {
			byte = CONFIG_BY_EXTERNAL_REGISTRAR;
			MIB_SET(gMIB_WSC_CONFIG_BY_EXT_REG, &byte);
		}
		//if (pCtx->manual_config) {
			//sprintf(tmpbuf, "WSC_MANUAL_ENABLED=%d\n", 0);
			//fputs(tmpbuf, fp);
		//}
		//sprintf(tmpbuf, "WSC_CONFIGURED=%d\n", 1);
		//fputs(tmpbuf, fp);
	}
		
	byte = 1;
	MIB_SET(gMIB_WSC_CONFIGURED, &byte);

	fprintf(stderr, "finsished\n");
	mib_update_all();
#ifdef FOR_DUAL_BAND
//	wlan_idx = orig_wlan_idx;
#endif //FOR_DUAL_BAND

	return REINIT_SYS;
}

#ifdef FOR_DUAL_BAND
int write_param_to_flash2(CTX_Tp pCtx, int is_local)
{
	char tmpbuf[120];// orig =100
	//FILE *fp;	
	int encrypt, cipher, is_psk=0;
	unsigned char byte;

	#ifdef FULL_SECURITY_CLONE	
	int  cipher2 = 0; 
	#endif

	int orig_wlan_idx = wlan_idx;
	wlan_idx = 1;

	//UTIL_DEBUG("\n\n\n");

	
	//fp = fopen(PARAM_TEMP_FILE, "w");

	if (is_local)
		//sprintf(tmpbuf, "SSID=%s\n", pCtx->SSID2);
		MIB_SET(gMIB_WLAN_SSID, pCtx->SSID2);
	else
		//sprintf(tmpbuf, "SSID=%s\n", pCtx->assigned_ssid);
		MIB_SET(gMIB_WLAN_SSID, pCtx->assigned_ssid);
	//fputs(tmpbuf, fp);	

	if (is_local)
		//sprintf(tmpbuf, "WSC_SSID=%s\n", pCtx->SSID2);
		MIB_SET(gMIB_WSC_SSID, pCtx->SSID2);
	else
		//sprintf(tmpbuf, "WSC_SSID=%s\n", pCtx->assigned_ssid);
		MIB_SET(gMIB_WSC_SSID, pCtx->assigned_ssid);
	//fputs(tmpbuf, fp);	
			
	encrypt = 0;
	if (is_local) {
		if (pCtx->auth_type_flash2 & AUTH_WPAPSK)			
			encrypt |= 2;
		if (pCtx->auth_type_flash2 & AUTH_WPA2PSK)			
			encrypt |= 4;		
	}
	else {
		//sprintf(tmpbuf, "AUTH_TYPE=%d\n", 2);
		
		if(pCtx->is_ap){
			if (pCtx->assigned_auth_type & AUTH_WPAPSK)			
				encrypt |= 2;

			if (pCtx->assigned_auth_type & AUTH_WPA2PSK)			
				encrypt |= 4;
		}else{
			if (pCtx->assigned_auth_type & AUTH_WPAPSK) 		
				encrypt = 2;
			
			if (pCtx->assigned_auth_type & AUTH_WPA2PSK)			
				encrypt = 4;
		}

		#ifdef FULL_SECURITY_CLONE
		if(pCtx->TagAPConfigStat==2)
			EncryptTypeChk(pCtx , &encrypt);
		#endif
		
		if (pCtx->assigned_auth_type == AUTH_OPEN && pCtx->assigned_encrypt_type == ENCRYPT_WEP)
			encrypt = 1;
		else if (pCtx->assigned_auth_type == AUTH_SHARED && pCtx->assigned_encrypt_type == ENCRYPT_WEP) {
			encrypt = 1;
			//sprintf(tmpbuf, "AUTH_TYPE=%d\n", 1);
			byte = 1;
			MIB_SET(gMIB_WLAN_AUTH_TYPE, &byte);
		}
		//fputs(tmpbuf, fp);
	}
	//sprintf(tmpbuf, "ENCRYPT=%d\n", encrypt);
	//fputs(tmpbuf, fp);			
	byte = encrypt;
	MIB_SET(gMIB_WLAN_ENCRYPT, &byte);

	encrypt = 0;
	if (is_local){
		encrypt = pCtx->auth_type_flash2;
	}else{ 
		if(pCtx->is_ap){
			encrypt = pCtx->assigned_auth_type;
		}else{

			/*under client mode just choose highest security set*/
			encrypt = pCtx->assigned_auth_type;
			
			if(pCtx->assigned_auth_type == AUTH_WPA2PSKMIXED)
				encrypt = AUTH_WPA2PSK ;
		}
	}
	//sprintf(tmpbuf, "WSC_AUTH=%d\n", encrypt);
	//fputs(tmpbuf, fp);
	byte = encrypt;
	MIB_SET(gMIB_WSC_AUTH, &byte);
	if (is_local) {
		if (pCtx->auth_type_flash2 & AUTH_WPAPSK || pCtx->auth_type_flash2 & AUTH_WPA2PSK)
			is_psk = 1;			
	}
	else {
		if (pCtx->assigned_auth_type & AUTH_WPAPSK || pCtx->assigned_auth_type & AUTH_WPA2PSK)
			is_psk = 1;
	}


	//fputs("WPA_AUTH=2\n", fp);
	byte = 2;
	MIB_SET(gMIB_WLAN_WPA_AUTH, &byte);

	if (is_psk) {
		if (is_local) {
			//sprintf(tmpbuf, "WPA_PSK=%s\n", pCtx->network_key2);
			//fputs(tmpbuf, fp);
			MIB_SET(gMIB_WLAN_WPA_PSK, pCtx->network_key2);
			if (strlen(pCtx->network_key2) == 64)
				//fputs("PSK_FORMAT=1\n", fp);
				byte = 1;
			else
				//fputs("PSK_FORMAT=0\n", fp);	
				byte = 0;
		}
		else {
			//sprintf(tmpbuf, "WPA_PSK=%s\n", pCtx->assigned_network_key);
			//fputs(tmpbuf, fp);
			MIB_SET(gMIB_WLAN_WPA_PSK, pCtx->assigned_network_key);
			if (strlen(pCtx->assigned_network_key) == 64)
				//fputs("PSK_FORMAT=1\n", fp);
				byte = 1;
			else
				//fputs("PSK_FORMAT=0\n", fp);	
				byte = 0;
		}
		MIB_SET(gMIB_WLAN_WPA_PSK_FORMAT, &byte);
	}

	if (is_local) {
		//sprintf(tmpbuf, "WSC_PSK=%s\n", pCtx->network_key2);
		//fputs(tmpbuf, fp);
		MIB_SET(gMIB_WSC_PSK, pCtx->network_key2);
	}
	else {
		//sprintf(tmpbuf, "WSC_PSK=%s\n", pCtx->assigned_network_key);
		//fputs(tmpbuf, fp);
		MIB_SET(gMIB_WSC_PSK, pCtx->assigned_network_key);
	}

	cipher = 0;
	if (is_local) {
		if (pCtx->encrypt_type_flash2 & ENCRYPT_TKIP)
			cipher |= 1;
		if (pCtx->encrypt_type_flash2 & ENCRYPT_AES)
			cipher |= 2;		
	}
	else {
		if (pCtx->assigned_encrypt_type & ENCRYPT_TKIP)
			cipher |= 1;
		if (pCtx->assigned_encrypt_type & ENCRYPT_AES)
			cipher |= 2;
	}


	//sprintf(tmpbuf, "WPA_CIPHER_SUITE=%d\n", cipher);
	//fputs(tmpbuf, fp);				
	//sprintf(tmpbuf, "WPA2_CIPHER_SUITE=%d\n", cipher);
	//fputs(tmpbuf, fp);						
#ifndef WPA2_MIXED_2MODE_ONLY	
	byte = cipher;
	MIB_SET(gMIB_WLAN_WPA_CIPHER_SUITE, &byte);
	//sprintf(tmpbuf, "WPA2_CIPHER_SUITE=%d\n", cipher);
	//fputs(tmpbuf, fp);				
	MIB_SET(gMIB_WLAN_WPA2_CIPHER_SUITE, &byte);
#else
	byte = 1;		//tkip
	MIB_SET(gMIB_WLAN_WPA_CIPHER_SUITE, &byte);
	byte = 2;		//aes
	MIB_SET(gMIB_WLAN_WPA2_CIPHER_SUITE, &byte);
#endif

	cipher = 0;
	if (is_local)
		cipher = pCtx->encrypt_type_flash2;
	else {
		cipher = pCtx->assigned_encrypt_type;
		if (pCtx->assigned_encrypt_type == ENCRYPT_WEP) {
			unsigned char wepkey[32];
			//sprintf(tmpbuf, "WEP=%d\n", pCtx->assigned_wep_key_len);
			//fputs(tmpbuf, fp);
			byte = pCtx->assigned_wep_key_len;
			MIB_SET(gMIB_WLAN_WEP, &byte);
			if (pCtx->assigned_wep_key_len == WEP64) {
#if 0
				sprintf(tmpbuf, "WEP64_KEY1=%s\n", pCtx->assigned_wep_key_1);
				fputs(tmpbuf, fp);
				sprintf(tmpbuf, "WEP64_KEY2=%s\n", pCtx->assigned_wep_key_2);
				fputs(tmpbuf, fp);
				sprintf(tmpbuf, "WEP64_KEY3=%s\n", pCtx->assigned_wep_key_3);
				fputs(tmpbuf, fp);
				sprintf(tmpbuf, "WEP64_KEY4=%s\n", pCtx->assigned_wep_key_4);
				fputs(tmpbuf, fp);
#else
				string_to_hex(pCtx->assigned_wep_key_1, wepkey, 10);
				MIB_SET(gMIB_WLAN_WEP64_KEY1, wepkey);
				string_to_hex(pCtx->assigned_wep_key_2, wepkey, 10);
				MIB_SET(gMIB_WLAN_WEP64_KEY1, wepkey);
				string_to_hex(pCtx->assigned_wep_key_3, wepkey, 10);
				MIB_SET(gMIB_WLAN_WEP64_KEY1, wepkey);
				string_to_hex(pCtx->assigned_wep_key_4, wepkey, 10);
				MIB_SET(gMIB_WLAN_WEP64_KEY1, wepkey);
#endif
			}
			else {
#if 0
				sprintf(tmpbuf, "WEP128_KEY1=%s\n", pCtx->assigned_wep_key_1);
				fputs(tmpbuf, fp);
				sprintf(tmpbuf, "WEP128_KEY2=%s\n", pCtx->assigned_wep_key_2);
				fputs(tmpbuf, fp);
				sprintf(tmpbuf, "WEP128_KEY3=%s\n", pCtx->assigned_wep_key_3);
				fputs(tmpbuf, fp);
				sprintf(tmpbuf, "WEP128_KEY4=%s\n", pCtx->assigned_wep_key_4);
				fputs(tmpbuf, fp);
#else
				string_to_hex(pCtx->assigned_wep_key_1, wepkey, 26);
				MIB_SET(gMIB_WLAN_WEP128_KEY1, wepkey);
				string_to_hex(pCtx->assigned_wep_key_2, wepkey, 26);
				MIB_SET(gMIB_WLAN_WEP128_KEY1, wepkey);
				string_to_hex(pCtx->assigned_wep_key_3, wepkey, 26);
				MIB_SET(gMIB_WLAN_WEP128_KEY1, wepkey);
				string_to_hex(pCtx->assigned_wep_key_4, wepkey, 26);
				MIB_SET(gMIB_WLAN_WEP128_KEY1, wepkey);
#endif
			}
			//sprintf(tmpbuf, "WEP_DEFAULT_KEY=%d\n", pCtx->assigned_wep_transmit_key-1);
			//fputs(tmpbuf, fp);
			byte = pCtx->assigned_wep_transmit_key-1;
			MIB_SET(gMIB_WLAN_WEP_DEFAULT_KEY, &byte);
			//sprintf(tmpbuf, "WEP_KEY_TYPE=%d\n", pCtx->assigned_wep_key_format);
			//fputs(tmpbuf, fp);
			byte = pCtx->assigned_wep_key_format;
			MIB_SET(gMIB_WLAN_WEP_KEY_TYPE, &byte);
		}
		else {
			//sprintf(tmpbuf, "WEP=%d\n", WEP_DISABLED);
			//fputs(tmpbuf, fp);
			byte = WEP_DISABLED;
			MIB_SET(gMIB_WLAN_WEP, &byte);
		}
	}


	//sprintf(tmpbuf, "WSC_ENC=%d\n", cipher);
	//fputs(tmpbuf, fp);	
	byte = cipher;
	MIB_SET(gMIB_WSC_ENC, &byte);

	#ifdef FULL_SECURITY_CLONE
	if(!pCtx->is_ap){
		/*
		UTIL_DEBUG("Switch to AP mode and reinit...\n");	
		sprintf(tmpbuf, "WLAN0_MODE=%d\n", 0);
		fputs(tmpbuf, fp);
		*/		
		sprintf(tmpbuf, "WLAN0_BAND=%d\n", pCtx->ss_status.bssdb[pCtx->join_idx].network);
		fputs(tmpbuf, fp);			
	}
	#endif
	
		
	if (pCtx->is_ap) {
		if (is_local) {
			//sprintf(tmpbuf, "WSC_CONFIGBYEXTREG=%d\n", CONFIG_BY_INTERNAL_REGISTRAR);
			//fputs(tmpbuf, fp);
			byte = CONFIG_BY_INTERNAL_REGISTRAR;
			MIB_SET(gMIB_WSC_CONFIG_BY_EXT_REG, &byte);
		}
		else {
			//sprintf(tmpbuf, "WSC_CONFIGBYEXTREG=%d\n", CONFIG_BY_EXTERNAL_REGISTRAR);
			//fputs(tmpbuf, fp);
			byte = CONFIG_BY_EXTERNAL_REGISTRAR;
			MIB_SET(gMIB_WSC_CONFIG_BY_EXT_REG, &byte);
		}
		/*if (pCtx->manual_config) {
			sprintf(tmpbuf, "WSC_MANUAL_ENABLED=%d\n", 0);
			fputs(tmpbuf, fp);
		}
		sprintf(tmpbuf, "WSC_CONFIGURED=%d\n", 1);
		fputs(tmpbuf, fp);	*/

	}
	
	//sprintf(tmpbuf, "WSC_CONFIGURED=%d\n", 1);
	//fputs(tmpbuf, fp);
		
	//fclose(fp);
	byte = 1;
	MIB_SET(gMIB_WSC_CONFIGURED, &byte);

	fprintf(stderr, "finsished\n");
	mib_update_all();

	wlan_idx = orig_wlan_idx;

	return REINIT_SYS;
}

#endif //FOR_DUAL_BAND

#endif //CONFIG_RTL867x


#ifdef	WINDOW7
void func_off_wlan_tx(CTX_Tp pCtx , unsigned char *interfacename)
{
	/*before issue system ReInit; hold wlan0's tx */
	char tmpbuf[100];
	unsigned int func_off_mibValue=0;
	if(wlioctl_get_mib(interfacename , "func_off" ,&func_off_mibValue)==0){
		func_off_mibValue |= 1<<1 ;
	}
	sprintf(tmpbuf,"iwpriv %s set_mib func_off=%d", interfacename ,func_off_mibValue);		

	UTIL_DEBUG("%s\n",tmpbuf);
	
	system(tmpbuf);	

}
void func_on_wlan_tx(CTX_Tp pCtx , unsigned char *interfacename)
{
	char tmpbuf[100];
	unsigned int func_off_mibValue=0;
	if(wlioctl_get_mib(interfacename , "func_off" ,&func_off_mibValue)==0){
		func_off_mibValue &= ~(1<<1) ;
		if(func_off_mibValue > 1){
			WSC_DEBUG("func_off_mibValue = %x\n",func_off_mibValue);			
			printf("Warning!! func_off_mibValue!=0 tx of %s will be holded\n",interfacename);
		}
	}
	sprintf(tmpbuf,"iwpriv %s set_mib func_off=%d", interfacename ,func_off_mibValue);		
	UTIL_DEBUG("%s\n",tmpbuf);	

	system(tmpbuf);	
}
#endif	//WINDOW7

#ifdef WPS2DOTX
void func_off_wlan_acl(CTX_Tp pCtx, unsigned char *interfacename)
{
	/*when WPS2.X disable ACL */
	char tmpbuf[100];
	unsigned int func_off_mibValue=0;
	sprintf(tmpbuf,"iwpriv %s set_mib aclmode=%d", interfacename ,func_off_mibValue);		
	system(tmpbuf);	
	UTIL_DEBUG("%s\n",tmpbuf);		
}	
#endif

void signal_webs(int condition)
{

#ifdef CONFIG_RTL867x
	// To-Do: We need to inform BoA (or configd) that the config has changed.
	//system("reboot");
	//cathy
	CTX_Tp pCtx = pGlobalCtx;
	fprintf(stderr, "signal_webs(%d)\n", condition);
	if (REINIT_SYS == condition) {
		//sleep(3);
		fprintf(stderr, "restart WLAN\n");
		//config_WLAN(3);
		// Kaohj --- should have been done in write_param_to_flash()
		//mib_update_all();
		//cathy
		pCtx->restart_timeout = 3;
		//system("/bin/reboot");
	}

#else // CONFIG_RTL867x

	char tmpbuf[100];
#ifdef OUTPUT_LOG
	if(outlog_fp){
		UTIL_DEBUG("close outlog_fp\n");
		fclose(outlog_fp);	
	}
#endif	
	
	if (condition == SYNC_FLASH_PARAMETER) {
		sprintf(tmpbuf, "echo 1 > %s", REINIT_WEB_FILE);
		system(tmpbuf);		
	}	
	
#if defined(CONFIG_RTL8186_KB) || defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196C_EC) || defined(CONFIG_RTL8198_AP_HCM)
	if (condition == REINIT_SYS ) {
//#ifdef CONFIG_RTL865X_KLD
#if 1
		sprintf(tmpbuf, "echo 2 > %s", REINIT_WEB_FILE);
		system(tmpbuf);		
#else
		{		
		//	sleep(1); // wait a while till status has been got by web, david+2008-06-11
		//	system("reboot");
			sprintf(tmpbuf, "echo 3 > %s", REINIT_WEB_FILE);
			system(tmpbuf);					
		}
#endif
	}
#else
	FILE *fp;
	char line[100];
	pid_t pid;
	char pidcase[30];	
#ifdef REINIT_VIA_RELOAD_DAEMON
	strcpy(pidcase,"reload daemon");
	if ((fp = fopen("/var/run/rc.pid", "r")) != NULL) 
#else
	strcpy(pidcase,"web server daemon");
	if ((fp = fopen(WEB_PID_FILENAME, "r")) != NULL) 
#endif		
	{
		fgets(line, sizeof(line), fp);
		if ( sscanf(line, "%d", &pid) ) {
			if (pid > 1) {			
				UTIL_DEBUG("Start Reinit via(%s)(pid=%d)\n",pidcase , pid );
				kill(pid, SIGUSR1);		
			}
		}
		fclose(fp);
	}
	else{
		UTIL_DEBUG("open pid(%s) fail\n",pidcase );
	}
#endif

#endif //CONFIG_RTL867x
}

int validate_pin_code(unsigned long code)
{
	unsigned long accum = 0;
	
	accum += 3 * ((code / 10000000) % 10); 
	accum += 1 * ((code / 1000000) % 10); 
	accum += 3 * ((code / 100000) % 10); 
	accum += 1 * ((code / 10000) % 10);
	accum += 3 * ((code / 1000) % 10);
	accum += 1 * ((code / 100) % 10);
	accum += 3 * ((code / 10) % 10); 
	accum += 1 * ((code / 1) % 10);
	
	return (0 == (accum % 10));	
}

DH *generate_dh_parameters(int prime_len, unsigned char *data, int generator)
{
	BIGNUM *p=NULL;
	DH *ret=NULL;
	int g;

	ret=DH_new();
	if (ret == NULL) {
		DEBUG_ERR("DH_new() return NULL\n");
		return NULL;
	}
	
	g = generator;
	if ((p=BN_new()) == NULL) {
		DEBUG_ERR("BN_new() return NULL!\n");
		DH_free(ret);		
		ret = NULL;
	}
	
	if (!BN_bin2bn(data, prime_len/8, p)) {
		DEBUG_ERR("BN_bin2bn() return error!\n");
		DH_free(ret);			
		ret = NULL;
	}

	ret->p=p;
	ret->g=BN_new();
	if (ret->g == NULL) {
		DEBUG_ERR("BN_new() return NULL!!\n");
		DH_free(ret);			
		ret = NULL;		
	}
	
	if (!BN_set_word(ret->g, g)) {
		DEBUG_ERR("BN_set_word() return error!!\n");
		DH_free(ret);		
		ret = NULL;		
	}
	return ret;	
}

void reset_sta_UPnP(CTX_Tp pCtx, STA_CTX_Tp pSta)
{
	unsigned int reset_size=0;
		
	if (pSta == NULL)
		return;
	
	if (pSta->dh_enrollee){
		DH_free(pSta->dh_enrollee);
		pSta->dh_enrollee = NULL;
	}
	if (pSta->dh_registrar){
		DH_free(pSta->dh_registrar);
		pSta->dh_registrar = NULL;
	}
	reset_size = ((unsigned int)&pSta->tx_size) - ((unsigned int)&pSta->state);
	memset(&pSta->state, '\0', reset_size);
	reset_size = ((unsigned int)&pSta->nonce_enrollee) - ((unsigned int)&pSta->reg_timeout);
	memset(&pSta->reg_timeout, '\0', reset_size);
	reset_size = ((unsigned int)&pSta->last_tx_msg_buffer) - ((unsigned int)&pSta->r_s1);
	memset(&pSta->r_s1, '\0', reset_size);
	reset_size = sizeof(STA_CTX) - ( ((unsigned int)&pSta->auth_type_flags) - ((unsigned int)pSta));
	memset(&pSta->auth_type_flags, '\0', reset_size);
#ifdef SUPPORT_UPNP
	DEBUG_PRINT("%s %d UPnP ctrl point = %s\n",
							__FUNCTION__, __LINE__, pSta->ip_addr);
#endif
	
	if (pCtx->sta_invoke_reg == pSta && pCtx->registration_on >= 1) {
		pCtx->registration_on = 0;
		pCtx->sta_invoke_reg = NULL;
		pCtx->role = pCtx->original_role;
		if (pCtx->pin_assigned || pCtx->pb_pressed) {
			DEBUG_PRINT("%s %d pCtx->pin_assigned = %d; pCtx->pb_pressed = %d\n", __FUNCTION__, __LINE__, pCtx->pin_assigned, pCtx->pb_pressed);
			reset_ctx_state(pCtx);
		}
	}
	DEBUG_PRINT("%s %d pCtx->registration_on = %d\n", __FUNCTION__, __LINE__, pCtx->registration_on);
}

void reset_sta(CTX_Tp pCtx, STA_CTX_Tp pSta, int need_free)
{
	int i;
	unsigned char allow_reconnect_count=0, ap_role=0;
	
	if (pSta == NULL)
		return;

	if (pSta->dh_enrollee){
		DH_free(pSta->dh_enrollee);
		pSta->dh_enrollee = NULL;
	}
	if (pSta->dh_registrar){
		DH_free(pSta->dh_registrar);
		pSta->dh_registrar = NULL;
	}
	
	if (pCtx->sta_invoke_reg == pSta && pCtx->registration_on >= 1) {
		pCtx->registration_on = 0;
		pCtx->sta_invoke_reg = NULL;
		pCtx->role = pCtx->original_role;
	}
	DEBUG_PRINT2("pCtx->registration_on = %d\n", pCtx->registration_on);
	
	if (pCtx->is_ap) {
		if (!need_free && (pSta->ap_role != ENROLLEE)) {
			allow_reconnect_count = pSta->allow_reconnect_count;
			ap_role = pSta->ap_role;
		}
	}
#ifdef CLIENT_MODE
	else {
		if (pCtx->wait_reinit) {
			pCtx->start = 0;
			//pCtx->wait_reinit = 0;
		}
		else {
			if (!pSta->do_not_rescan) {
				if (pCtx->pb_timeout || pCtx->pin_timeout) {
					DEBUG_PRINT("%s %d set pCtx->connect_fail = 1\n", __FUNCTION__, __LINE__);
					pCtx->connect_fail = 1;
				}
			}
		}
	}
#endif

	if (!(pSta->used & IS_UPNP_CONTROL_POINT)) {
		//UTIL_DEBUG("sta = %02x:%02x:%02x:%02x:%02x:%02x\n",	pSta->addr[0], pSta->addr[1], pSta->addr[2],
		//					pSta->addr[3], pSta->addr[4], pSta->addr[5]);
		
		memset(pSta, '\0', sizeof(STA_CTX));
		pCtx->num_sta--;
		if (pCtx->num_sta < 0) {
			UTIL_DEBUG("number of station = %d, CHK!!!\n", pCtx->num_sta);
			pCtx->num_sta = 0;
		}else{
			DEBUG_PRINT2("number of station = %d\n",pCtx->num_sta);
		}
	}
	else {
		unsigned int reset_size = sizeof(STA_CTX) - ( ((unsigned int)&pSta->state) - ((unsigned int)pSta));
		memset(&pSta->state, '\0', reset_size);
#ifdef SUPPORT_UPNP
		UTIL_DEBUG("UPnP ctrl point = %s\n", pSta->ip_addr);
#endif
	}

	if (need_free) {
		for (i=0; i<MAX_STA_NUM; i++) {
			if (pSta == pCtx->sta[i]) {
				if (pSta->used & IS_UPNP_CONTROL_POINT) {

					if(pCtx->num_ext_registrar)
						pCtx->num_ext_registrar--;
					
					UTIL_DEBUG("ER count=%d\n", pCtx->num_ext_registrar);
				}
				free(pSta);
				pCtx->sta[i] = NULL;			
			}		
		}		
	}	
	else {
		if (pCtx->is_ap && (ap_role != ENROLLEE)) {
			pSta->allow_reconnect_count = allow_reconnect_count;
			pSta->ap_role = ap_role;
		}
	}
}

void clear_SetSelectedRegistrar_flag(CTX_Tp pCtx)
{
	unsigned char tmpbuf[400];
	int len;

	DEBUG_PRINT("\n\n	Clear SetSelectedRegistrar flag!!\n\n");
	len = build_beacon_ie(pCtx, 0, 0, 0, tmpbuf);
	if (pCtx->encrypt_type == ENCRYPT_WEP) // add provisioning service ie
		len += build_provisioning_service_ie((unsigned char *)(tmpbuf+len));

#ifdef FOR_DUAL_BAND
	if(!pCtx->wlan0_wsc_disabled)
#endif
	{
		if (wlioctl_set_wsc_ie(pCtx->wlan_interface_name, tmpbuf, len, 
				DOT11_EVENT_WSC_SET_IE, SET_IE_FLAG_BEACON) < 0) {
			DEBUG_ERR("wlioctl_set_wsc_ie() error!\n");
		}
	}
#ifdef FOR_DUAL_BAND
	if(!pCtx->wlan1_wsc_disabled){
		if (wlioctl_set_wsc_ie(pCtx->wlan_interface_name2, tmpbuf, len, 
			DOT11_EVENT_WSC_SET_IE, SET_IE_FLAG_BEACON) < 0) {
			DEBUG_ERR("wlioctl_set_wsc_ie() error!\n");
		}
	}
#endif //FOR_DUAL_BAND
	
	len = build_probe_rsp_ie(pCtx, 0, 0, 0, tmpbuf);

	if ((pCtx->encrypt_type == ENCRYPT_WEP) && (pCtx->current_wps_version == WPS_VERSION_V1)) // add provisioning service ie
		len += build_provisioning_service_ie((unsigned char *)(tmpbuf+len));

	
	if (len > MAX_WSC_IE_LEN) {
		DEBUG_ERR("Length of IE exceeds %d\n", MAX_WSC_IE_LEN);
	}

#ifdef FOR_DUAL_BAND
	if(!pCtx->wlan0_wsc_disabled)
#endif
	{
		if (wlioctl_set_wsc_ie(pCtx->wlan_interface_name, tmpbuf, len, 
			DOT11_EVENT_WSC_SET_IE, SET_IE_FLAG_PROBE_RSP) < 0) {
			DEBUG_ERR("wlioctl_set_wsc_ie() error!\n");
		}
	}
#ifdef FOR_DUAL_BAND
	if(!pCtx->wlan1_wsc_disabled){
		if (wlioctl_set_wsc_ie(pCtx->wlan_interface_name2, tmpbuf, len, 
				DOT11_EVENT_WSC_SET_IE, SET_IE_FLAG_PROBE_RSP) < 0) {
			DEBUG_ERR("wlioctl_set_wsc_ie() error!\n");
		}
	}
#endif //FOR_DUAL_BAND
}

void reset_ctx_state(CTX_Tp pCtx)
{
#ifdef CLIENT_MODE
	unsigned char tmpbuf[100];
#endif

#ifdef CONFIG_RTL8186_TR
	struct stat buf;
#endif

	_DEBUG_PRINT("%s %d \n", __FUNCTION__, __LINE__);

#ifdef FOR_DUAL_BAND
	pCtx->inter0only=0;
	pCtx->inter1only=0;
	UTIL_DEBUG("clean inter0only & inter1only\n\n");
#endif
	
	//TODO: need check if session timeout happen, then clear LED
#ifdef CONFIG_RTL8186_TR
	if (stat(LED_ON_10S_FILE, &buf) != 0)
#endif
	
	if (wlioctl_set_led(pCtx->wlan_interface_name, LED_WSC_END) < 0) {
		DEBUG_ERR("issue wlan ioctl set_led error!\n");	
	}

#ifdef CLIENT_MODE
	if (!pCtx->is_ap) {
		if (wlioctl_set_wsc_ie(pCtx->wlan_interface_name, tmpbuf, 0, 
				DOT11_EVENT_WSC_SET_IE, SET_IE_FLAG_PROBE_REQ) < 0) {
			DEBUG_ERR("Reset probe request IE failed\n");
		}

		if(pCtx->STAmodeSuccess){
			pCtx->STAmodeSuccess = 0;
			client_set_WlanDriver_WscEnable(pCtx, 6);
		}else{
			client_set_WlanDriver_WscEnable(pCtx, 0);
		}
		pCtx->connect_fail = 0;
		
		//pCtx->wait_assoc_ind = 0;
		
		pCtx->SPEC_SSID[0]='\0';
		memset(pCtx->SPEC_MAC , 0 , sizeof(pCtx->SPEC_MAC));
	}
	else
#endif	
	{
	
		pCtx->wps_triggered = 0;//add by peteryu for WZC in WEP
	}
	
	pCtx->pb_pressed = 0;
	pCtx->pin_assigned = 0;
	pCtx->pb_timeout = 0;
	pCtx->pin_timeout = 0;
	pCtx->setSelectedRegTimeout = 0;
	//cathy
	pCtx->restart_timeout = 0;
#ifdef SUPPORT_UPNP
	memset(pCtx->SetSelectedRegistrar_ip, 0, IP_ADDRLEN);
#endif	
	memcpy(pCtx->pin_code, pCtx->original_pin_code, PIN_LEN+1);
	DEBUG_PRINT("set pCtx->pin_code = %s\n", pCtx->pin_code);
	
	if (pCtx->is_ap && pCtx->use_ie)
		clear_SetSelectedRegistrar_flag(pCtx);
	
#ifdef BLOCKED_ROGUE_STA
	if (pCtx->is_ap && pCtx->blocked_expired_time)
		disassociate_blocked_list(pCtx);
#endif
}

void hmac_sha256(unsigned char *text, int text_len, unsigned char *key, int key_len, unsigned char *digest, int *digest_len)
{
    unsigned int    outlen;
    unsigned char   out[EVP_MAX_MD_SIZE];
    const EVP_MD   *md;
//    int x;
	md= EVP_sha256();
	HMAC(md,key,key_len,text,text_len,out,&outlen);
//              for(x=0;x<outlen;x++)
//                  printf("%02x  ",out[x]);
//                  printf("\n");
	memcpy(digest,out,outlen);
	*digest_len=outlen;
}

void Encrypt_aes_128_cbc(unsigned char *key, unsigned char *iv, unsigned char *plaintext, unsigned int plainlen, unsigned char *ciphertext, unsigned int *cipherlen)
{  
#ifdef USE_PORTING_OPENSSL
	AES_KEY aes_ks1;
	unsigned char iv_tmp[BYTE_LEN_128B];
	unsigned char cipher_tmp[BYTE_LEN_128B];
	unsigned char *plaintext_tmp=NULL;
	unsigned char *ciphertext_tmp=NULL;
	unsigned int remainder=0;
	
	memset(&aes_ks1, 0, sizeof(struct aes_key_st));
	if (AES_set_encrypt_key(key, 128, &aes_ks1) < 0) {
		DEBUG_ERR("AES_set_encrypt_key error!\n");
		return;
	}

	// preserve the original value
	memcpy(iv_tmp, iv, BYTE_LEN_128B);
	plaintext_tmp = (unsigned char *)malloc(plainlen);
	if (plaintext_tmp == NULL) {
		DEBUG_ERR("Not enough memory!\n");
		return;
	}
	memcpy(plaintext_tmp, plaintext, plainlen);

	remainder = plainlen % AES_BLOCK_SIZE;
	if (remainder)
		plainlen -= remainder;
	*cipherlen = plainlen;
	AES_cbc_encrypt(plaintext, ciphertext, plainlen, &aes_ks1, iv, AES_ENCRYPT);

	ciphertext_tmp = (unsigned char *)malloc(*cipherlen);
	if (ciphertext_tmp == NULL) {
		DEBUG_ERR("Not enough memory!\n");
		free(plaintext_tmp);
		return;
	}
	memcpy(ciphertext_tmp, ciphertext, *cipherlen);

	if (remainder) {
		memcpy(cipher_tmp, plaintext+plainlen, remainder);
		memset(cipher_tmp+remainder, BYTE_LEN_128B-remainder, BYTE_LEN_128B-remainder); //add padding
	}
	else
		memset(cipher_tmp, 0x10, BYTE_LEN_128B);
	AES_cbc_encrypt(cipher_tmp, ciphertext, BYTE_LEN_128B, &aes_ks1, iv, AES_ENCRYPT);
	
	memcpy(ciphertext, ciphertext_tmp, *cipherlen);
	memcpy(ciphertext+(*cipherlen), iv, BYTE_LEN_128B);
	*cipherlen += BYTE_LEN_128B;

	memcpy(iv, iv_tmp, BYTE_LEN_128B);
	if (remainder)
		plainlen += remainder;
	memcpy(plaintext, plaintext_tmp, plainlen);

	free(plaintext_tmp);
	free(ciphertext_tmp);
	//DEBUG_ERR("plainlen = %d\n", plainlen);
	//DEBUG_ERR("cipherlen = %d\n", *cipherlen);
#else
	unsigned int tlen;
	EVP_CIPHER_CTX ctx;
	
	EVP_CIPHER_CTX_init(&ctx);
	EVP_EncryptInit(&ctx, EVP_aes_128_cbc(), key, iv);
	EVP_EncryptUpdate(&ctx, ciphertext, cipherlen, plaintext, plainlen);
	//debug_out("ciphertext  before EVP_EncryptFinal", ciphertext, *cipherlen);
	DEBUG_ERR("Encrypt:olen=%d\n",*cipherlen);
	//debug_out("ciphertext + cipherlen  before EVP_EncryptFinal", ciphertext +(* cipherlen), 48);
	
	EVP_EncryptFinal(&ctx, ciphertext +(* cipherlen), &tlen) ;
	//debug_out("ciphertext  after EVP_EncryptFinal", ciphertext, *cipherlen);
	//debug_out("ciphertext + cipherlen  after EVP_EncryptFinal", ciphertext +(* cipherlen), 48);
	DEBUG_ERR("Encrypt:tlen:%d\n",tlen);
	
	EVP_CIPHER_CTX_cleanup(&ctx);
	*cipherlen=(*cipherlen)+tlen;

	//DEBUG_ERR("plainlen = %d\n", plainlen);
	//DEBUG_ERR("cipherlen = %d\n", *cipherlen);
#endif
}

void Decrypt_aes_128_cbc(unsigned char *key,  unsigned char *iv, unsigned char *plaintext, unsigned int *plainlen, unsigned char *ciphertext, unsigned int cipherlen)
{
#ifdef USE_PORTING_OPENSSL
	AES_KEY aes_ks1;
	unsigned char *ciphertext_tmp=NULL;
	unsigned char iv_tmp[BYTE_LEN_128B];

	memset(&aes_ks1, 0, sizeof(struct aes_key_st));
	if (AES_set_decrypt_key(key, 128, &aes_ks1) < 0) {
		DEBUG_ERR("AES_set_decrypt_key error!\n");
		return;
	}

	memcpy(iv_tmp, iv, BYTE_LEN_128B);
	ciphertext_tmp = (unsigned char *)malloc(cipherlen);
	if (ciphertext_tmp == NULL) {
		DEBUG_ERR("Not enough memory!\n");
		return;
	}
	memcpy(ciphertext_tmp, ciphertext, cipherlen);

	AES_cbc_encrypt(ciphertext, plaintext, cipherlen, &aes_ks1, iv, AES_DECRYPT);
	if (cipherlen % AES_BLOCK_SIZE)
		*plainlen = cipherlen + (AES_BLOCK_SIZE - (cipherlen % AES_BLOCK_SIZE));
	else
		*plainlen = cipherlen;

	memcpy(iv, iv_tmp, BYTE_LEN_128B);
	memcpy(ciphertext, ciphertext_tmp, cipherlen);
	free(ciphertext_tmp);
#else
	unsigned int tlen;
	int ret;
	EVP_CIPHER_CTX ctx;

	EVP_CIPHER_CTX_init(&ctx);
	EVP_DecryptInit(&ctx, EVP_aes_128_cbc(), key, iv);
	EVP_DecryptUpdate(&ctx, plaintext, plainlen, ciphertext, cipherlen);
	//debug_out("plaintext  before EVP_DecryptUpdate", plaintext, *plainlen);
	//debug_out("plaintext + plainlen  before EVP_DecryptUpdatel", plaintext +(* plainlen), 48);
	DEBUG_ERR("Decrypt:olen=%d\n",*plainlen);
	
	ret=EVP_DecryptFinal(&ctx, plaintext + (*plainlen), &tlen) ;
	//debug_out("plaintext  after EVP_DecryptFinal", plaintext, *plainlen);
	//debug_out("plaintext+ plainlen  after EVP_DecryptFinal", plaintext +(* plainlen), 48);
	DEBUG_ERR("Decrypt:tlen=%d\n",tlen);
	
	 (*plainlen)=(*plainlen)+tlen;
	EVP_CIPHER_CTX_cleanup(&ctx);

	//DEBUG_ERR("plainlen = %d\n", *plainlen);
	//DEBUG_ERR("cipherlen = %d\n", cipherlen);
#endif
}

void wsc_kdf(
	unsigned char*  key,                // pointer to authentication key 
	int             key_len,            // length of authentication key 
	unsigned char*  text,               // pointer to data stream 
	int	text_len,           // length of data stream 
	int 	expect_key_len,   //expect total key length in bit number
	unsigned char*  digest             // caller digest to be filled in 
	)
{
	int i, len;
	unsigned long longVal;	
	unsigned char result[1024];
	unsigned char md[EVP_MAX_MD_SIZE];
	unsigned int md_len=0;
	unsigned char text_buf[256];
	int IterationNum = ((expect_key_len/8) + PRF_DIGEST_SIZE - 1)/PRF_DIGEST_SIZE;
	
	//printf("loop count:%d\n",IterationNum);
	memset(result, 0, 256);
	memset(text_buf, 0, 256);
	memset(md, 0, EVP_MAX_MD_SIZE);

	memcpy(&text_buf[4] ,text, text_len);
	longVal = htonl(expect_key_len);
	memcpy(&text_buf[4+text_len], &longVal, 4);
			
	for(i=1,len=0; i<=IterationNum; i++)
	{	
		longVal = htonl(i);
		memcpy(&text_buf[0], &longVal, 4);
		hmac_sha256(text_buf, 4+text_len+4, key, key_len, md, &md_len);
		memcpy(&result[len], md, md_len);
		len += md_len;		
	}
	memcpy(digest, result, expect_key_len/8);	
}

int build_beacon_ie(CTX_Tp pCtx, unsigned char selected, unsigned short passid, 
				unsigned short method, unsigned char *data)
{
	unsigned char *pMsg;
	unsigned char byteVal;
	unsigned short shortVal;
	int len;
	
	data[0] = WSC_IE_ID;
	memcpy(&data[2], WSC_IE_OUI, sizeof(WSC_IE_OUI));

	pMsg = &data[2+sizeof(WSC_IE_OUI)];
	byteVal = WSC_VER;
	pMsg = add_tlv(pMsg, TAG_VERSION, 1, (void *)&byteVal);

	byteVal = (unsigned char)pCtx->config_state;
	pMsg = add_tlv(pMsg, TAG_SIMPLE_CONFIG_STATE, 1, (void *)&byteVal);	

	if (pCtx->lock
#ifdef	AUTO_LOCK_DOWN
			|| (pCtx->auto_lock_down > 0)
#endif
	) {			
		byteVal = 1; // 1=locked ; 0 =unlocked ;  pCtx->lock;
		UTIL_DEBUG("	!!!add TAG_AP_SETUP_LOCKED to beacon\n");
		pMsg = add_tlv(pMsg, TAG_AP_SETUP_LOCKED, 1, &byteVal);		
	}

	//pMsg = add_tlv(pMsg, TAG_DEVICE_NAME, strlen(pCtx->device_name), (void *)pCtx->device_name);
	//shortVal = htons(pCtx->config_method);
	//pMsg = add_tlv(pMsg, TAG_CONFIG_METHODS, 2, &shortVal);

	if (selected) {
		pMsg = add_tlv(pMsg, TAG_SELECTED_REGITRAR, 1, &selected);

		shortVal = htons(passid);			
		pMsg = add_tlv(pMsg, TAG_DEVICE_PASSWORD_ID, 2, &shortVal);
	
		if(pCtx->setSelectedRegTimeout){
			UTIL_DEBUG("include ER's config method \n");				
			shortVal = htons(pCtx->config_method|method);
		}
		else{
			shortVal = htons(pCtx->config_method);		
		}
		pMsg = add_tlv(pMsg, TAG_SEL_REG_CONFIG_METHODS, 2, &shortVal);
	}

#ifdef FOR_DUAL_BAND
	if(pCtx->wlan1_wsc_disabled==0 && pCtx->wlan0_wsc_disabled==0
		&& pCtx->inter0only==0 && pCtx->inter1only==0){

		//UTIL_DEBUG("Dual band concurrent!!\n");
		pMsg = add_tlv(pMsg, TAG_UUID_E, UUID_LEN, (void *)pCtx->uuid);	
	
		byteVal = 0x3;
		pMsg = add_tlv(pMsg, TAG_RF_BAND, 1, (void *)&byteVal);
	}
#endif	

#ifdef WPS2DOTX	
	if (pCtx->current_wps_version == WPS_VERSION_V2) {
		if (selected) {		
			int vendorDatalen = add_v2andAuthTag(pCtx);
			pMsg = add_tlv(pMsg, TAG_VENDOR_EXT, vendorDatalen, (void *)pCtx->VENDOR_DTAT);
		}else{
			pMsg = add_tlv(pMsg, TAG_VENDOR_EXT, 6, (void *)WSC_VENDOR_V2);
		}
	}
#endif
	len = (int)(((unsigned long)pMsg)-((unsigned long)data)) -2;
	data[1] = (unsigned char)len;

	return len+2;
}

int build_probe_rsp_ie(CTX_Tp pCtx, unsigned char selected, unsigned short passid, 
				unsigned short method, unsigned char *data)	
{
	unsigned char *pMsg;
	unsigned char byteVal;
	unsigned short shortVal;
	unsigned char tmpbuf[100];
	int len = 0;
	
	data[0] = WSC_IE_ID;
	memcpy(&data[2], WSC_IE_OUI, sizeof(WSC_IE_OUI));

	pMsg = &data[2+sizeof(WSC_IE_OUI)];
	byteVal = WSC_VER;
	pMsg = add_tlv(pMsg, TAG_VERSION, 1, (void *)&byteVal);

#if	defined(WPS2DOTX) && defined(WSC_IE_FRAGMENT_APSIDE)
	if (pCtx->probeRsp_need_wscIE_frag && (pCtx->current_wps_version == WPS_VERSION_V2)){
		len = (int)(((unsigned long)pMsg)-((unsigned long)data)) - 2;
		data[1] = (unsigned char)len;	
		UTIL_DEBUG("\n!AP mode do probe_Rsp fragment(1) , len =%d\n",len);			
		pMsg[0] = WSC_IE_ID ;
		pMsg+=2;
		memcpy(pMsg, WSC_IE_OUI, sizeof(WSC_IE_OUI));		
		pMsg+=4;
		
	}
#endif


	byteVal = (unsigned char)pCtx->config_state;
	pMsg = add_tlv(pMsg, TAG_SIMPLE_CONFIG_STATE, 1, (void *)&byteVal);
	if ( pCtx->lock
#ifdef	AUTO_LOCK_DOWN
			|| (pCtx->auto_lock_down > 0)
#endif
		) 
	{			
		byteVal = 1 ;
		UTIL_DEBUG("	!!!add TAG_AP_SETUP_LOCKED to probe_rsp\n");		
		pMsg = add_tlv(pMsg, TAG_AP_SETUP_LOCKED, 1, &byteVal);		
	}

	if (selected) {
		pMsg = add_tlv(pMsg, TAG_SELECTED_REGITRAR, 1, &selected);			
		shortVal = htons(passid);		
		pMsg = add_tlv(pMsg, TAG_DEVICE_PASSWORD_ID, 2, &shortVal);

		if(pCtx->setSelectedRegTimeout){
			UTIL_DEBUG("include ER's config method \n");				
			shortVal = htons(pCtx->config_method|method);
		}
		else{
			shortVal = htons(pCtx->config_method);		
		}
		pMsg = add_tlv(pMsg, TAG_SEL_REG_CONFIG_METHODS, 2, &shortVal);
	}

	if (pCtx->is_ap)
		byteVal = RSP_TYPE_AP;
	else
		byteVal = ((pCtx->role == REGISTRAR) ? RSP_TYPE_REG : RSP_TYPE_ENR);		
	
	pMsg = add_tlv(pMsg, TAG_RESPONSE_TYPE, 1, (void *)&byteVal);	
	
	if (byteVal == RSP_TYPE_REG && !pCtx->is_ap)
		pMsg = add_tlv(pMsg, TAG_UUID_R, UUID_LEN, (void *)pCtx->uuid);
	else
		pMsg = add_tlv(pMsg, TAG_UUID_E, UUID_LEN, (void *)pCtx->uuid);	

	pMsg = add_tlv(pMsg, TAG_MANUFACTURER, strlen(pCtx->manufacturer), (void *)pCtx->manufacturer);
	pMsg = add_tlv(pMsg, TAG_MODEL_NAME, strlen(pCtx->model_name), (void *)pCtx->model_name);	
	pMsg = add_tlv(pMsg, TAG_MODEL_NUMBER, strlen(pCtx->model_num), (void *)pCtx->model_num);
	pMsg = add_tlv(pMsg, TAG_SERIAL_NUM, strlen(pCtx->serial_num), (void *)pCtx->serial_num);

	shortVal = htons(((unsigned short)pCtx->device_category_id));
	memcpy(tmpbuf, &shortVal, 2);
	memcpy(&tmpbuf[2], pCtx->device_oui, OUI_LEN);
	shortVal = htons(((unsigned short)pCtx->device_sub_category_id));
	memcpy(&tmpbuf[6], &shortVal, 2);	
	pMsg = add_tlv(pMsg, TAG_PRIMARY_DEVICE_TYPE, 8, (void *)tmpbuf);
	pMsg = add_tlv(pMsg, TAG_DEVICE_NAME, strlen(pCtx->device_name), (void *)pCtx->device_name);

	shortVal = (pCtx->config_method & ~(CONFIG_METHOD_VIRTUAL_PBC | CONFIG_METHOD_PHYSICAL_PBC)) ;
	UTIL_DEBUG("config method(0x1008)=%x\n",shortVal);	
	shortVal = htons(shortVal);
	pMsg = add_tlv(pMsg, TAG_CONFIG_METHODS, 2, &shortVal);

#ifdef FOR_DUAL_BAND	
	if(pCtx->wlan1_wsc_disabled==0 && pCtx->wlan0_wsc_disabled==0
		&& pCtx->inter0only==0 && pCtx->inter1only==0){
		//UTIL_DEBUG("Dual band concurrent!!!\n");		
		byteVal = 0x3;
		pMsg = add_tlv(pMsg, TAG_RF_BAND, 1, (void *)&byteVal);
	}
#endif		

	
#ifdef WPS2DOTX	
	if (pCtx->current_wps_version == WPS_VERSION_V2) {
		if (selected) {		
			int vendorDatalen = add_v2andAuthTag(pCtx);
			pMsg = add_tlv(pMsg, TAG_VENDOR_EXT, vendorDatalen, (void *)pCtx->VENDOR_DTAT);
		}else{
			pMsg = add_tlv(pMsg, TAG_VENDOR_EXT, 6, (void *)WSC_VENDOR_V2);
		}
	}
#endif

#if	defined(WPS2DOTX) && defined(WSC_IE_FRAGMENT_APSIDE)
	if(pCtx->probeRsp_need_wscIE_frag && (pCtx->current_wps_version == WPS_VERSION_V2)){
		int len2 = (int)(((unsigned long)pMsg)-((unsigned long)data))- 2 - len - 2 ;
		UTIL_DEBUG("\n!AP mode do probe_Rsp fragment(2),len =%d \n",len2);
		data[len+2+1] = (unsigned char)len2;	
		int totalLen = (int)(((unsigned long)pMsg)-((unsigned long)data));
		debug_out("ProbeRsp wsc ie",data,totalLen);
		return totalLen;
		
	}else
#endif	
	{
		len = (int)(((unsigned long)pMsg)-((unsigned long)data)) -2;
		data[1] = (unsigned char)len;
	}
	
	return len+2;
}

#ifdef CLIENT_MODE
int build_probe_request_ie(CTX_Tp pCtx, unsigned short passid, 
				unsigned char *data)	
{
	unsigned char *pMsg;
	unsigned char byteVal;
	unsigned short shortVal;
	unsigned char tmpbuf[100];
	int len = 0;
#if defined(WPS2DOTX) && defined(WSC_IE_FRAGMENT_STASIDE)
	int len2;//wps2x	
#endif	
	data[0] = WSC_IE_ID;
	memcpy(&data[2], WSC_IE_OUI, sizeof(WSC_IE_OUI));

	pMsg = &data[2+sizeof(WSC_IE_OUI)];
	byteVal = WSC_VER;
	
#if	defined(WPS2DOTX) && defined(WSC_IE_FRAGMENT_STASIDE)
	if(pCtx->probeReq_need_wscIE_frag && (pCtx->current_wps_version == WPS_VERSION_V2)){
		pMsg[0]=0x10;
		pMsg++;
		len = sizeof(WSC_IE_OUI)+1;
		data[1] = (unsigned char)len;	
		UTIL_DEBUG("\n!STA mode do probe_Req fragment1 , len =%d\n",len);	


		
		pMsg[0] = WSC_IE_ID ;
		pMsg+=2;
		
		memcpy(pMsg, WSC_IE_OUI, sizeof(WSC_IE_OUI));		
		pMsg+=4;
		
		pMsg[0]=0x4a;
		pMsg[1]=0x00;
		pMsg[2]=0x01;
		pMsg[3]=0x10;		
		pMsg+=4;
	} else
#endif
	{
		pMsg = add_tlv(pMsg, TAG_VERSION, 1, (void *)&byteVal);
	}
	

	byteVal = ((pCtx->role == REGISTRAR) ? REQ_TYPE_REG : REQ_TYPE_ENR);		
	pMsg = add_tlv(pMsg, TAG_REQUEST_TYPE, 1, (void *)&byteVal);
	shortVal = htons(pCtx->config_method);
	pMsg = add_tlv(pMsg, TAG_CONFIG_METHODS, 2, &shortVal);
	if (pCtx->is_ap)
		pMsg = add_tlv(pMsg, TAG_UUID_E, UUID_LEN, (void *)pCtx->uuid);
	else {
		if (pCtx->role == REGISTRAR)
			pMsg = add_tlv(pMsg, TAG_UUID_R, UUID_LEN, (void *)pCtx->uuid);
		else
			pMsg = add_tlv(pMsg, TAG_UUID_E, UUID_LEN, (void *)pCtx->uuid);
	}
	
	shortVal = htons(((unsigned short)pCtx->device_category_id));
	memcpy(tmpbuf, &shortVal, 2);
	memcpy(&tmpbuf[2], pCtx->device_oui, OUI_LEN);
	shortVal = htons(((unsigned short)pCtx->device_sub_category_id));
	memcpy(&tmpbuf[6], &shortVal, 2);	
	pMsg = add_tlv(pMsg, TAG_PRIMARY_DEVICE_TYPE, 8, (void *)tmpbuf);
	byteVal = (unsigned char)pCtx->rf_band;	//TODO . when STA mode need take care
	pMsg = add_tlv(pMsg, TAG_RF_BAND, 1, (void *)&byteVal);
	shortVal = ASSOC_STATE_NOT_ASSOC; // TODO:
	shortVal = htons((unsigned short)shortVal);
	pMsg = add_tlv(pMsg, TAG_ASSOC_STATE, 2, (void *)&shortVal);
	shortVal = htons((unsigned short)pCtx->config_err);
	pMsg = add_tlv(pMsg, TAG_CONFIG_ERR, 2, (void *)&shortVal);
	shortVal = htons(passid);			
	pMsg = add_tlv(pMsg, TAG_DEVICE_PASSWORD_ID, 2, &shortVal);
#ifdef WPS2DOTX		// add  below attributes in probe request
	if (pCtx->current_wps_version == WPS_VERSION_V2) {
		pMsg = add_tlv(pMsg, TAG_MANUFACTURER, strlen(pCtx->manufacturer) , (void *)pCtx->manufacturer);
		pMsg = add_tlv(pMsg, TAG_MODEL_NAME, strlen(pCtx->model_name), (void *)pCtx->model_name);		
		pMsg = add_tlv(pMsg, TAG_MODEL_NUMBER, strlen(pCtx->model_num), (void *)pCtx->model_num);
		pMsg = add_tlv(pMsg, TAG_DEVICE_NAME, strlen(pCtx->device_name), (void *)pCtx->device_name);
		pMsg = add_tlv(pMsg, TAG_VENDOR_EXT, 6, (void *)WSC_VENDOR_V2);
	}
#endif

#if	defined(WPS2DOTX) && defined(WSC_IE_FRAGMENT_STASIDE)
	if(pCtx->probeReq_need_wscIE_frag && (pCtx->current_wps_version == WPS_VERSION_V2)){
		len2 = (int)((((unsigned long)pMsg)-((unsigned long)data) - 2 )- len - 2 );
		UTIL_DEBUG("\n!STA mode do probe_Req fragment2 , len2 =%d(0x%x)\n",len2,len2);			
		data[len+2+1] = (unsigned char)len2;	

		return (int)(((unsigned long)pMsg)-((unsigned long)data));
		
	}else
#endif	
	{
		len = (int)(((unsigned long)pMsg)-((unsigned long)data)) -2;
		data[1] = (unsigned char)len;
	}
	
	if (len >= MAX_WSC_IE_LEN)
		return 0;
	else
		return len+2;
}

int build_assoc_request_ie(CTX_Tp pCtx, unsigned char *data)
{
	unsigned char *pMsg;
	unsigned char byteVal;
	int len;

	data[0] = WSC_IE_ID;
	memcpy(&data[2], WSC_IE_OUI, sizeof(WSC_IE_OUI));

	pMsg = &data[2+sizeof(WSC_IE_OUI)];
	byteVal = WSC_VER;
	pMsg = add_tlv(pMsg, TAG_VERSION, 1, (void *)&byteVal);
	byteVal = ((pCtx->role == REGISTRAR) ? REQ_TYPE_REG : REQ_TYPE_ENR);		
	pMsg = add_tlv(pMsg, TAG_REQUEST_TYPE, 1, (void *)&byteVal);

#ifdef WPS2DOTX
	if (pCtx->current_wps_version == WPS_VERSION_V2)
		pMsg = add_tlv(pMsg, TAG_VENDOR_EXT, 6, (void *)WSC_VENDOR_V2);
#endif

	len = (int)(((unsigned long)pMsg)-((unsigned long)data)) -2;
	data[1] = (unsigned char)len;
	if (len >= MAX_WSC_IE_LEN)
		return 0;
	else
		return len+2;
}

int getWlJoinRequest(char *interface, pBssDscr pBss, unsigned char *res)
{
    int skfd;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
	pBss->bdType |= WIFI_WPS;
	strcpy(wrq.ifr_name, interface);	
    wrq.u.data.pointer = (caddr_t)pBss;
    wrq.u.data.length = sizeof(BssDscr);
	if (ioctl(skfd, SIOCGIWRTLJOINREQ, &wrq) < 0)
	{
		perror("ioctl[SIOCGIWRTLJOINREQ]");
		close(skfd);
		return -1;
	}
	close(skfd);
   
    *res = *((unsigned char *)wrq.u.data.pointer);

    return 0;
}

int getWlJoinResult(char *interface, unsigned char *res)
{
    int skfd;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);

	strcpy(wrq.ifr_name, interface);	
    wrq.u.data.pointer = (caddr_t)res;
    wrq.u.data.length = 1;
	if (ioctl(skfd, SIOCGIWRTLJOINREQSTATUS, &wrq) < 0)
	{
		perror("ioctl[SIOCGIWRTLJOINREQSTATUS]");
		close(skfd);
		return -1;
	}
    close(skfd);

    return 0;
}
#endif // CLIENT_MODE

#ifdef P2P_SUPPORT

int ReportWPSstate(char *interface, unsigned char *res)
{
    int skfd;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd < 0)
		return -1;

	strcpy(wrq.ifr_name, interface);	
    wrq.u.data.pointer = (caddr_t)res;
    wrq.u.data.length = 1;

	if (ioctl(skfd, SIOCP2P_WSC_REPORT_STATE, &wrq) < 0)
	{
		perror("ioctl[SIOCP2P_WSC_REPORT_STATE]");
		close(skfd);
		return -1;
	}
    close(skfd);

    return 0;
}


#endif

int build_provisioning_service_ie(unsigned char *data)
{
	data[0] = WSC_IE_ID;
	data[1] = 5;
	memcpy(&data[2], Provisioning_Service_IE_OUI, sizeof(Provisioning_Service_IE_OUI));
	data[6] = 0;
	
	return 7;
}

int build_assoc_response_ie(CTX_Tp pCtx, unsigned char *data)
{
	unsigned char *pMsg;
	unsigned char byteVal;
	int len;

	data[0] = WSC_IE_ID;
	memcpy(&data[2], WSC_IE_OUI, sizeof(WSC_IE_OUI));

	pMsg = &data[2+sizeof(WSC_IE_OUI)];
	byteVal = WSC_VER;
	pMsg = add_tlv(pMsg, TAG_VERSION, 1, (void *)&byteVal);
	byteVal = RSP_TYPE_AP;
	pMsg = add_tlv(pMsg, TAG_RESPONSE_TYPE, 1, (void *)&byteVal);

#ifdef WPS2DOTX
	if (pCtx->current_wps_version == WPS_VERSION_V2)
		pMsg = add_tlv(pMsg, TAG_VENDOR_EXT, 6, (void *)WSC_VENDOR_V2);
#endif

	len = (int)(((unsigned long)pMsg)-((unsigned long)data)) -2;
	data[1] = (unsigned char)len;
	if (len >= MAX_WSC_IE_LEN)
		return 0;
	else
		return len+2;
}

unsigned char *search_tag(unsigned char *data, unsigned short id, int len, int *out_len)
{
	unsigned short tag, tag_len;
	int size;

	while (len > 0) {
		memcpy(&tag, data, 2);
		memcpy(&tag_len, data+2, 2);
		tag = ntohs(tag);
		tag_len = ntohs(tag_len);

		if (id == tag) {
			if (len >= (4 + tag_len)) {
				//Fix, makes wscd silently killed running on TI platform
				//*out_len = (int)tag_len;
				memset(out_len,0,sizeof(int));
				memcpy((unsigned char *)out_len+2,&tag_len,sizeof(unsigned short));
				
				return (&data[4]);
			}
			else {
				DEBUG_ERR("Found tag [0x%x], but invalid length!\n", id);
				break;
			}
		}
		size = 4 + tag_len;
		data += size;
		len -= size;
	}

	return NULL;
}



#ifdef WPS2DOTX
unsigned char *search_VendorExt_tag(unsigned char *data, unsigned char id, int len, int *out_len)
{
	unsigned char tag, tag_len;
	int size;

	//skip WFA_VENDOR_LEN
	data +=3;
	len	-=3;
	while (len > 0) {
		memcpy(&tag, data, 1);
		memcpy(&tag_len, data+1, 1);
		if (id == tag) {
			if (len >= (2 + tag_len)) {
				*out_len = (int)tag_len;
				return (&data[2]);
			}
			else {
				UTIL_DEBUG("Found VE tag [0x%x], but invalid length!\n", id);
				break;
			}
		}
		size = 2 + tag_len;
		data += size;
		len -= size;
	}

	return NULL;
}
#endif


#if defined(SUPPORT_UPNP) && !defined(USE_MINI_UPNP)
static int get_sockfd(void)
{
	static int sockfd = -1;

	if (sockfd == -1) {
		if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) == -1) {
			perror("user: socket creation failed");
			return(-1);
		}
	}
	return sockfd;
}

IPCon IPCon_New(char * ifname)
{
	IPCon ipcon=NULL;

	ipcon = (IPCon)malloc(sizeof(_IPCon));
	if (!ipcon) { 
		printf("Error in IPCon_New:Cannot allocate memory\n");
		return NULL;
	}

	ipcon->ifname = ifname;
	return (ipcon);
}


IPCon IPCon_Destroy(IPCon this)
{
	if (!this) 
		return (NULL);

	free(this);
	return (NULL);
}

struct in_addr *IPCon_GetIpAddr(IPCon this)
{
    struct ifreq ifr;
	struct sockaddr_in *saddr;
    int fd, ret;

    fd = get_sockfd();
    if (fd >= 0) {
	    strcpy(ifr.ifr_name, this->ifname);
		ifr.ifr_addr.sa_family = AF_INET;
		ret = ioctl(fd, SIOCGIFADDR, &ifr);
		close(fd);
		if (ret == 0) {
			saddr = (struct sockaddr_in *)&ifr.ifr_addr;
			return &saddr->sin_addr;
		} else {
			return NULL;
		}
	}
	return NULL;
}


char *IPCon_GetIpAddrByStr(IPCon this)
{
	struct in_addr *adr;

	adr = IPCon_GetIpAddr(this);
	if (adr == NULL) {
		return NULL;
	} else {
		return inet_ntoa(*adr);
	}
}
#endif // SUPPORT_UPNP

// deal with Vista's bug
static unsigned char wep_format(unsigned char *msg, int msg_len, unsigned char *msg_out)
{
	int i, ret=KEY_HEX;

	for (i=0; i<msg_len; i++) {
		if (msg[i] >= 0x21 && msg[i] <= 0x7e) {
			if (i >= msg_len-1) // ASCII case
				ret = KEY_ASCII;
		}
		else
			break;
	}

	convert_bin_to_str(msg, msg_len, msg_out);
	msg_out[msg_len*2] = '\0';
	
	return ret;
}

int check_wep_key_format(unsigned char *msg, int msg_len, unsigned char *key_format, unsigned char *key_len, unsigned char *msg_out, int *msg_out_len)
{
	if (msg_len == 5) {
		*key_format = wep_format(msg, msg_len, msg_out);
		*msg_out_len = 10;
		*key_len = WEP64;
	}
	else if (msg_len == 10) {
		memcpy(msg_out, msg, msg_len);
		*msg_out_len = msg_len;
		msg_out[msg_len] = '\0';
		*key_format = KEY_HEX;
		*key_len = WEP64;
	}
	else if (msg_len == 13) {
		*key_format = wep_format(msg, msg_len, msg_out);
		*msg_out_len = 26;
		*key_len = WEP128;
	}
	else if (msg_len == 26) {
		memcpy(msg_out, msg, msg_len);
		*msg_out_len = msg_len;
		msg_out[msg_len] = '\0';
		*key_format = KEY_HEX;
		*key_len = WEP128;
	}
	else {
		DEBUG_ERR("Invalid WEP key length = %d\n", msg_len);
		return -1;
	}
	return 0;
}

#ifdef MUL_PBC_DETECTTION
void search_active_pbc_sta(CTX_Tp pCtx, unsigned char *addr, unsigned char *uuid)
{
	pbc_node_ptr node=NULL;
	pbc_node_ptr previous_node=NULL;
#ifdef DEBUG
	unsigned int count=0;
#endif

	previous_node = pCtx->active_pbc_staList;
	node = pCtx->active_pbc_staList->next_pbc_sta;
	while (node) {
#ifdef DEBUG
		count++;
		if (count >= 10000)
			_DEBUG_PRINT("ERROR : Infinite loop!\n");
#endif
		// for Buffalo station 

		/*for fit dual band AP maybe active at both 5g and 2.4g , 
		  so we need chk if UUID is the same ; 2010-10-21 */  
		if (!memcmp(node->addr, addr, ETHER_ADDRLEN) || !memcmp(node->uuid, uuid, UUID_LEN)) {
		//if (!memcmp(node->addr, addr, ETHER_ADDRLEN)) {
#ifdef DEBUG
			_DEBUG_PRINT("\n\n");
			UTIL_DEBUG("An active PBC STA be found: ");
			MAC_PRINT(addr);
			UUID_PRINT(uuid);		
			UTIL_DEBUG("Active pbc sta count = %d\n\n", pCtx->active_pbc_sta_count);
#endif
			node->time_stamp = time(NULL);
		
			return;
		}
		previous_node = node;
		node = node->next_pbc_sta;
	}

	node = (struct pbc_node_context *) malloc(sizeof(struct pbc_node_context));
	if (node == NULL) {
		DEBUG_ERR("%s %d Not enough memory\n", __FUNCTION__, __LINE__);
		return;
	}
	else {
		_DEBUG_PRINT("\n\n");

		UTIL_DEBUG("Add an active PBC STA: ");
		MAC_PRINT(addr);
		UUID_PRINT(uuid);						
		memset(node, 0, sizeof(struct pbc_node_context));
		pCtx->active_pbc_sta_count++;
		UTIL_DEBUG("Active pbc sta count = %d\n\n", pCtx->active_pbc_sta_count);
		
		memcpy(node->addr, addr, ETHER_ADDRLEN);
		memcpy(node->uuid, uuid, UUID_LEN);
		node->time_stamp = time(NULL);
		previous_node->next_pbc_sta = node;
			
		return;
	}
}

void remove_active_pbc_sta(CTX_Tp pCtx, STA_CTX_Tp pSta, unsigned char mode)
{
	pbc_node_ptr node=NULL;
	pbc_node_ptr previous_node=NULL;
#ifdef DEBUG
	unsigned char sta_found=0;
	unsigned char sta_count=0;
	unsigned int count=0;
#endif
	unsigned char remove_sta=0;

	if (mode && pSta == NULL)
		return;
	
	if (pCtx->active_pbc_staList->next_pbc_sta == NULL) {
#ifdef DEBUG
		if (pCtx->active_pbc_sta_count != 0 && !mode)
			DEBUG_ERR("active_pbc_staList && active_pbc_sta_count mismatched!\n");
		else if (mode)
			DEBUG_ERR("Null active_pbc_staList!\n");
#endif

		return;
	}
	else {
		node = pCtx->active_pbc_staList->next_pbc_sta;
		previous_node = pCtx->active_pbc_staList;
	}
	
	while (node) {
#ifdef DEBUG
		count++;
		if (count >= 10000)
			_DEBUG_PRINT("ERROR : Infinite loop!\n");
#endif
		// for Buffalo station 
		//if (((!memcmp(node->addr, pSta->addr, ETHER_ADDRLEN) && !memcmp(node->uuid, pSta->uuid, UUID_LEN)) && mode) ||
		//if (((!memcmp(node->addr, pSta->addr, ETHER_ADDRLEN)) && mode) ||
			//((difftime(time(NULL), node->time_stamp) >= PBC_WALK_TIME) && !mode))
		remove_sta = 0;
		if (mode == 0) {
			int walk_time;
			if (pCtx->is_ap)
				walk_time = PBC_WALK_TIME;
			else
				walk_time = 5;
			//cathy, difftime return a double value, our LIB has problem to access float or double value
			//if (difftime(time(NULL), node->time_stamp) >= walk_time)
			if ((time(NULL) - node->time_stamp) >= walk_time)
				remove_sta = 1;
		}
		else {
			if (!memcmp(node->addr, pSta->addr, ETHER_ADDRLEN))
				remove_sta = 1;
		}
		
		if (remove_sta) {
			pCtx->active_pbc_sta_count--;

			UTIL_DEBUG("An active PBC STA be removed: ");
			MAC_PRINT(node->addr);
			UUID_PRINT(node->uuid);						
			UTIL_DEBUG("active pbc sta count = %d\n\n", pCtx->active_pbc_sta_count);
			
			previous_node->next_pbc_sta = node->next_pbc_sta;
			free(node);
			node = previous_node;

#ifdef DEBUG
			if (mode) {
				sta_found=1;
				sta_count++;
			}
#endif
		}
		previous_node = node;
		node = node->next_pbc_sta;
	}

#ifdef DEBUG
	if (mode) {
		if (!sta_found)
			DEBUG_ERR("Error : no PBC station has been removed!\n");
		else {
			if (sta_count > 1)
				DEBUG_ERR("Error : more than one PBC station have been removed!\n");
		}
	}
#endif
}

void SwitchSessionOverlap_LED_On(CTX_Tp pCtx)
{
	pCtx->SessionOverlapTimeout = SESSION_OVERLAP_TIME;

	_DEBUG_PRINT("Detected Overlaping \n"); // for DET debug
	report_WPS_STATUS(PROTOCOL_PBC_OVERLAPPING);
	reset_ctx_state(pCtx);
	
#ifdef CLIENT_MODE
	if (!pCtx->is_ap && pCtx->role == ENROLLEE)
		pCtx->start = 0;
#endif

	if (wlioctl_set_led(pCtx->wlan_interface_name, LED_PBC_OVERLAPPED) < 0)
		DEBUG_ERR("issue wlan ioctl set_led error!\n");
	//	else
	//		pCtx->SessionOverlapTimeout = SESSION_OVERLAP_TIME;
}
#endif

#ifdef BLOCKED_ROGUE_STA
unsigned char search_blocked_list(CTX_Tp pCtx, unsigned char *addr)
{
	int i;

	if (addr) {
		for (i=0; i<MAX_BLOCKED_STA_NUM; i++) {
			if (!pCtx->blocked_sta_list[i].used)
				continue;
			else {
				if (!memcmp(pCtx->blocked_sta_list[i].addr, addr, ETHER_ADDRLEN)) {

					UTIL_DEBUG("A Sta has been found in the blocked list\n");
					MAC_PRINT(pCtx->blocked_sta_list[i].addr);							
					return 1;
				}
			}
		}
		return 0;
	}
	else
		return 0;
}

struct blocked_sta *add_into_blocked_list(CTX_Tp pCtx, STA_CTX_Tp pSta)
{
	int i;

	if (pSta->addr) {
		for (i=0; i<MAX_BLOCKED_STA_NUM; i++) {
			if (pCtx->blocked_sta_list[i].used == 0) {
				pSta->blocked = 1;
				pCtx->blocked_sta_list[i].used = 1;
				pCtx->blocked_sta_list[i].expired_time = pCtx->blocked_expired_time;				
				memcpy(pCtx->blocked_sta_list[i].addr, pSta->addr, ETHER_ADDRLEN);
				_DEBUG_PRINT("Adds a sta [%02x:%02x:%02x:%02x:%02x:%02x] into the blocked list\n",
					pCtx->blocked_sta_list[i].addr[0], pCtx->blocked_sta_list[i].addr[1],
					pCtx->blocked_sta_list[i].addr[2], pCtx->blocked_sta_list[i].addr[3], 
					pCtx->blocked_sta_list[i].addr[4], pCtx->blocked_sta_list[i].addr[5]);
				return (&pCtx->blocked_sta_list[i]);
			}
		}
		_DEBUG_PRINT("Warning : blocked_list table full!\n");
		return NULL; // table full
	}
	else {
		DEBUG_ERR("Error : NULL address for blocked_list!\n");
		return NULL; // null address
	}
}

void disassociate_blocked_list(CTX_Tp pCtx)
{
	int i, count=0;

	for (i=0; i<MAX_BLOCKED_STA_NUM; i++) {
		if (pCtx->blocked_sta_list[i].used) {
			count++;
			DEBUG_PRINT("Disassociates a sta [%02x:%02x:%02x:%02x:%02x:%02x] from the blocked list\n",
				pCtx->blocked_sta_list[i].addr[0], pCtx->blocked_sta_list[i].addr[1],
				pCtx->blocked_sta_list[i].addr[2], pCtx->blocked_sta_list[i].addr[3], 
				pCtx->blocked_sta_list[i].addr[4], pCtx->blocked_sta_list[i].addr[5]);
			// Reason code 1 : Unspecified reason
			UTIL_DEBUG("IssueDisconnect\n");			
			IssueDisconnect(pCtx->wlan_interface_name, pCtx->blocked_sta_list[i].addr, 1);
		}
	}
	
	if (count)
		memset(pCtx->blocked_sta_list, 0, (MAX_BLOCKED_STA_NUM * sizeof(struct blocked_sta)));
}

void countdown_blocked_list(CTX_Tp pCtx)
{
	int i;
	for (i=0; i<MAX_BLOCKED_STA_NUM; i++) {
		if (pCtx->blocked_sta_list[i].used && pCtx->blocked_sta_list[i].expired_time > 0) {
			if (--pCtx->blocked_sta_list[i].expired_time <= 0) {
				DEBUG_PRINT("Blocked STA [%02x:%02x:%02x:%02x:%02x:%02x] expired!\n",
					pCtx->blocked_sta_list[i].addr[0], pCtx->blocked_sta_list[i].addr[1],
					pCtx->blocked_sta_list[i].addr[2], pCtx->blocked_sta_list[i].addr[3], 
					pCtx->blocked_sta_list[i].addr[4], pCtx->blocked_sta_list[i].addr[5]);
				// Reason code 1 : Unspecified reason
				UTIL_DEBUG("IssueDisconnect\n");
				IssueDisconnect(pCtx->wlan_interface_name, pCtx->blocked_sta_list[i].addr, 1);
				memset(&pCtx->blocked_sta_list[i], 0, sizeof(struct blocked_sta));				
			}
		}
	}	
}
#endif // BLOCKED_ROGUE_STA


#ifdef CONNECT_PROXY_AP
unsigned char search_blocked_ap_list(CTX_Tp pCtx, int idx, int selected)
{
	int i;
	BssDscr *pBss;
	pBss = &pCtx->ss_status.bssdb[idx];


	for (i=0; i<MAX_BLOCKED_AP_NUM; i++) {

		if (!pCtx->blocked_ap_list[i].used)
			continue;
		else {
			if (!memcmp(pCtx->blocked_ap_list[i].addr, pBss->bdBssId, ETHER_ADDRLEN)) {
				_DEBUG_PRINT("An AP has been found in the blocked list\n");
				MAC_PRINT(pCtx->blocked_ap_list[i].addr);			
				if(selected == 0)
					{
						//printf("_Eric used_unselected = %d \n", pCtx->blocked_ap_list[i].used_unselected);
						if((pCtx->blocked_ap_list[i].used_unselected) <= MAX_RETRY_AP_TIME)
							continue;
					}
				
				return 1;
			}
		}
	}
	
	return 0;

}

void add_into_blocked_ap_list(CTX_Tp pCtx, int idx, int selected)
{
	int i;
	BssDscr *pBss;
	pBss = &pCtx->ss_status.bssdb[idx];

	for (i=0; i<MAX_BLOCKED_AP_NUM; i++) {
		if (pCtx->blocked_ap_list[i].used == 0) {
			pCtx->blocked_ap_list[i].used = 1;

			if(selected == 0)
				{
					pCtx->blocked_ap_list[i].used_unselected = 1;
					pCtx->blocked_unselected_ap ++ ;
				}
			
			memcpy(pCtx->blocked_ap_list[i].addr, pBss->bdBssId, ETHER_ADDRLEN);
			_DEBUG_PRINT("Add an AP to the blocked list\n");
			MAC_PRINT(pCtx->blocked_ap_list[i].addr);
			return ;
		}
		else
		{
			if(selected == 0)
				{
				   if (!memcmp(pCtx->blocked_ap_list[i].addr, pBss->bdBssId, ETHER_ADDRLEN))
				   	{
				   		//printf("_Eric used_unselected ++ \n");
				   		pCtx->blocked_ap_list[i].used_unselected ++;
						return ;
				   	}

				}		

		}
	}
	
	_DEBUG_PRINT("Warning : blocked_list table full!\n");
	return ; // table full

}


void clear_blocked_ap_list(CTX_Tp pCtx)
{
	int i;
	//printf("_Eric clear_blocked_ap_list +++ \n");
	pCtx->blocked_unselected_ap = 0;
	for (i=0; i<MAX_BLOCKED_AP_NUM; i++) {			
		memset(&pCtx->blocked_ap_list[i], 0, sizeof(struct blocked_ap));				
	}	
}
#endif


void enable_WPS_LED(void)
{
	system("echo E > /proc/gpio");
}

void report_WPS_STATUS(int status)
{
	char tmp[40];

	sprintf(tmp, "echo %d > %s", status, WSCD_CONFIG_STATUS);
	system(tmp);
}

static int _is_hex(char c)
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
		if ( !_is_hex(tmpBuf[0]) || !_is_hex(tmpBuf[1]))
			return 0;

		key[ii++] = (unsigned char) strtol(tmpBuf, (char**)NULL, 16);
	}
	return 1;
}

int is_zero_ether_addr(const unsigned char *a)
{
	return !(a[0] | a[1] | a[2] | a[3] | a[4] | a[5]);
}

void show_auth_encry_help(void)
{
#ifdef DEBUG
	printf("Auth: OPEN=1, WPAPSK=2, SHARED=4, WPA2PSK=0x20, MIXED=0x22\n");	
	printf("Encry: NONE=1, WEP=2, TKIP=4, AES=8, TKIPAES=12\n");
#endif	
}

#ifdef PIXIE_DUST_ATTACK

#include <openssl/hmac.h>

typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8 ;   

#define POOL_WORDS 32
#define POOL_WORDS_MASK (POOL_WORDS - 1)
#define POOL_TAP1 26
#define POOL_TAP2 20
#define POOL_TAP3 14
#define POOL_TAP4 7
#define POOL_TAP5 1
#define EXTRACT_LEN 16
//#define MIN_READY_MARK 2

static u32 pool[POOL_WORDS];
static unsigned int input_rotate = 0;
static unsigned int pool_pos = 0;
static u8 dummy_key[20];

//static unsigned int own_pool_ready = 0;
//#define RANDOM_ENTROPY_SIZE 20
//static char *random_entropy_file = NULL;
//static int random_entropy_file_read = 0;
//#define MIN_COLLECT_ENTROPY 1000
//static unsigned int entropy = 0;
//static unsigned int total_collected = 0;
#define SHA1_MAC_LEN 20

static u32 __ROL32(u32 x, u32 y)
{
	return (x << (y & 31)) | (x >> (32 - (y & 31)));
}

static void random_mix_pool(const void *buf, size_t len)
{
	static const u32 twist[8] = {
		0x00000000, 0x3b6e20c8, 0x76dc4190, 0x4db26158,
		0xedb88320, 0xd6d6a3e8, 0x9b64c2b0, 0xa00ae278
	};
	const u8 *pos = buf;
	u32 w;

	//wpa_hexdump_key(MSG_EXCESSIVE, "random_mix_pool", buf, len);

	while (len--) {
		w = __ROL32(*pos++, input_rotate & 31);
		input_rotate += pool_pos ? 7 : 14;
		pool_pos = (pool_pos - 1) & POOL_WORDS_MASK;
		w ^= pool[pool_pos];
		w ^= pool[(pool_pos + POOL_TAP1) & POOL_WORDS_MASK];
		w ^= pool[(pool_pos + POOL_TAP2) & POOL_WORDS_MASK];
		w ^= pool[(pool_pos + POOL_TAP3) & POOL_WORDS_MASK];
		w ^= pool[(pool_pos + POOL_TAP4) & POOL_WORDS_MASK];
		w ^= pool[(pool_pos + POOL_TAP5) & POOL_WORDS_MASK];
		pool[pool_pos] = (w >> 3) ^ twist[w & 7];
	}
}

static int openssl_hmac_vector(const EVP_MD *type, const u8 *key,
			       size_t key_len, size_t num_elem,
			       const u8 *addr[], const size_t *len, u8 *mac,
			       unsigned int mdlen)
{
	HMAC_CTX ctx;
	size_t i;
	int res;

	HMAC_CTX_init(&ctx);
#if OPENSSL_VERSION_NUMBER < 0x00909000
	HMAC_Init_ex(&ctx, key, key_len, type, NULL);
#else /* openssl < 0.9.9 */
	if (HMAC_Init_ex(&ctx, key, key_len, type, NULL) != 1)
		return -1;
#endif /* openssl < 0.9.9 */

	for (i = 0; i < num_elem; i++)
		HMAC_Update(&ctx, addr[i], len[i]);

#if OPENSSL_VERSION_NUMBER < 0x00909000
	HMAC_Final(&ctx, mac, &mdlen);
	res = 1;
#else /* openssl < 0.9.9 */
	res = HMAC_Final(&ctx, mac, &mdlen);
#endif /* openssl < 0.9.9 */
	HMAC_CTX_cleanup(&ctx);

	return res == 1 ? 0 : -1;
}

int hmac_sha1_vector(const u8 *key, size_t key_len, size_t num_elem,
		     const u8 *addr[], const size_t *len, u8 *mac)
{
	return openssl_hmac_vector(EVP_sha1(), key, key_len, num_elem, addr,
				   len, mac, 20);
}

int hmac_sha1(const u8 *key, size_t key_len, const u8 *data, size_t data_len,
	       u8 *mac)
{
	return hmac_sha1_vector(key, key_len, 1, &data, &data_len, mac);
}

static void random_extract(u8 *out)
{
	unsigned int i;
	u8 hash[SHA1_MAC_LEN];
	u32 *hash_ptr;
	u32 buf[POOL_WORDS / 2];

	/* First, add hash back to pool to make backtracking more difficult. */
	hmac_sha1(dummy_key, sizeof(dummy_key), (const u8 *) pool,
		  sizeof(pool), hash);
	random_mix_pool(hash, sizeof(hash));
	/* Hash half the pool to extra data */
	for (i = 0; i < POOL_WORDS / 2; i++)
		buf[i] = pool[(pool_pos - i) & POOL_WORDS_MASK];
	hmac_sha1(dummy_key, sizeof(dummy_key), (const u8 *) buf,
		  sizeof(buf), hash);

	/*
	 * Fold the hash to further reduce any potential output pattern.
	 * Though, compromise this to reduce CPU use for the most common output
	 * length (32) and return 16 bytes from instead of only half.
	 */
	hash_ptr = (u32 *) hash;
	hash_ptr[0] ^= hash_ptr[4];
	memcpy(out, hash, EXTRACT_LEN);
}


#if defined(WPA_TRACE_BFD) && defined(CONFIG_TESTING_OPTIONS)
#define TEST_FAIL() testing_test_fail()
int testing_test_fail(void);
#else
#define TEST_FAIL() 0
#endif

int os_get_random(unsigned char *buf, size_t len)
{
	FILE *f;
	size_t rc;

	if (TEST_FAIL())
		return -1;

	f = fopen("/dev/urandom", "rb");
	if (f == NULL) {
		printf("Could not open /dev/urandom.\n");
		return -1;
	}

	rc = fread(buf, 1, len, f);
	fclose(f);

	return rc != len ? -1 : 0;
}

int crypto_get_random(void *buf, size_t len)
{
	if (RAND_bytes(buf, len) != 1)
		return -1;
	return 0;
}

/*parameter:len (unit:bytes)*/
unsigned char *generate_random(unsigned char *data, int len)
{

    int ret;
	int idx;

	unsigned char tmp[EXTRACT_LEN];
	size_t left;  
    size_t size1;    
	u8 *bytes = data;
    
	/* Start with assumed strong randomness from OS */
	ret = os_get_random(data, len);    
    #ifdef DEBUG_PIXIE_DUST_ATTACK
    printf("strong randomness from OS\n");    
	for (idx = 0; idx < len; idx++){
        printf("[%02x]",data[idx]);        
    }
    printf("\n");
    #endif


    /* Mix in additional entropy extracted from the internal pool */
	bytes = data;        
    left=len;
    while (left) {

        random_extract(tmp);

   		size1 = left > EXTRACT_LEN ? EXTRACT_LEN : left;
        for (idx = 0; idx < size1; idx++){
            *bytes++ ^= tmp[idx];            
        }
        left -= size1;
    }
    
    #ifdef DEBUG_PIXIE_DUST_ATTACK    
    printf("entropy extracted from the internal pool\n");        
	for (idx = 0; idx < len; idx++){
        printf("[%02x]",data[idx]);        
    }
    printf("\n");
    #endif


    
	/* Mix in additional entropy from the crypto module */

	bytes = data;        
    left=len;
    while (left) {
        crypto_get_random(tmp, sizeof(tmp));
   		size1 = left > EXTRACT_LEN ? EXTRACT_LEN : left;
        for (idx = 0; idx < size1; idx++){
            *bytes++ ^= tmp[idx];            
        }
        left -= size1;
    }
    
    #ifdef    DEBUG_PIXIE_DUST_ATTACK
     printf("Mix in additional entropy from the crypto module \n");  
     for (idx = 0; idx < len; idx++){
         printf("[%02x]",data[idx]);        
     }
     printf("\n");
     #endif


	return data+len;
}
#else

/*parameter:len (unit:bytes)*/
unsigned char *generate_random(unsigned char *data, int len)
{
	struct timeval tod;
	int i, num;

	gettimeofday(&tod , NULL);
	srand(tod.tv_usec);

	for (i=0; i<(len/4); i++) {
		num = rand();
		//*((int *)data) = num;
		memcpy(data,&num,sizeof(num));
		data += 4;
	}

	i = len % 4;
	if (i > 0) {
		num = rand();
		memcpy(data, &num, i);
	}
	return data+len;
}
#endif  


