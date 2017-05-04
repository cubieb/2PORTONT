/*
 *  Receive packet handler for WiFi Simple-Config
 *
 *	Copyright (C)2006, Realtek Semiconductor Corp. All rights reserved.
 *
 *	$Id: rxpkt.c,v 1.9 2012/02/23 07:19:45 cathy Exp $
 */

/*================================================================*/
/* Include Files */

#include "wsc.h"

// WPS2DOTX
extern unsigned char WSC_VENDOR_OUI[3];
/*================================================================*/

#ifdef	AUTO_LOCK_DOWN

void	record_and_check_AuthFail(CTX_Tp pCtx)
{
	int i;
//	struct sysinfo info ;
	unsigned int time_offset;
	
	/*if now rdy lock stats , need not check and record*/ 
	if(pCtx->auto_lock_down)
		return;

	_DEBUG_PRINT("\n<<%s>>\n", __FUNCTION__);
	
#ifdef ALD_BRUTEFORCE_ATTACK_MITIGATION
	pCtx->ADL_pin_attack_count++;
	RX_DEBUG("	consecutive attempts times [%d]\n",pCtx->ADL_pin_attack_count);
	
	if(pCtx->ADL_pin_attack_count >= ALD_INDEFINITE_TH){
		pCtx->auto_lock_down = 0x7FFFFFFF;
		InOut_auto_lock_down(pCtx,1);
		pCtx->ald_virgin = 0;
		return;
	}
#endif
	
	if(pCtx->ald_virgin == 0){
		
		for(i=0 ; i<AUTH_FAIL_TIMES ;i++){
			pCtx->ald_timestamp[i] = 0 ;
		}
		
		pCtx->ald_h = 0; // head
		pCtx->ald_t = 0; // tail		 
//		sysinfo(&info);
//		pCtx->ald_timestamp[pCtx->ald_t] = (unsigned long)info.uptime;
		pCtx->ald_timestamp[pCtx->ald_t] = time(NULL);
		pCtx->ald_t++; 
		pCtx->ald_virgin = 1;
		return ;
	}

//	sysinfo(&info);
//	pCtx->ald_timestamp[pCtx->ald_t] = (unsigned long)info.uptime;
	pCtx->ald_timestamp[pCtx->ald_t] = time(NULL);

	if(( ( pCtx->ald_h - pCtx->ald_t) == 1 ) || 
		( (pCtx->ald_t - pCtx->ald_h) == ( AUTH_FAIL_TIMES - 1)) )
	{


		//cathy, difftime return a double value, our LIB has problem to access float or double value
		//time_offset = difftime(pCtx->ald_timestamp[pCtx->ald_t], pCtx->ald_timestamp[pCtx->ald_h]);
		time_offset = pCtx->ald_timestamp[pCtx->ald_t] - pCtx->ald_timestamp[pCtx->ald_h];
		
		if(time_offset < AUTH_FAIL_TIME_TH){
			pCtx->auto_lock_down = AUTO_LOCKED_DOWN_TIME;
			InOut_auto_lock_down(pCtx,1);
			pCtx->ald_virgin = 0;
			return ; 
		}
	}

	/*circle array; when array is full ,
	  last-timeStamp replace first-timeStamp*/ 	
	pCtx->ald_t++;
	pCtx->ald_t %= AUTH_FAIL_TIMES;
	if(pCtx->ald_t == pCtx->ald_h){
		pCtx->ald_h++;
		pCtx->ald_h %= AUTH_FAIL_TIMES;
	}
	
}
#endif

/* Implementation Routines */
#if 0
int isUpnpSubscribed(CTX_Tp pCtx)
{
	int i;

	for (i=0; i<MAX_STA_NUM; i++) {
		if (pCtx->sta[i].used && 
			(pCtx->sta[i].used & IS_UPNP_CONTROL_POINT))
			return 1;
	}
	//DEBUG_PRINT("No UPnP external registrar subscribes!\n");
	return 0;
}
#endif

static unsigned char *check_tag(CTX_Tp pCtx, unsigned char *pMsg, int msg_len, int tag, int check_len, 
	char *name, int type, int *o_len)
{
	unsigned char *pData;
	int tag_len;

	pData = search_tag(pMsg, tag, msg_len, &tag_len);
	if (pData == NULL) {
		DEBUG_ERR("\n can't find %s tag!\n", name);
		return NULL;
	}
	if (check_len) {
		if (check_len & NOT_GREATER_THAN_MASK) {
			if (tag_len > (check_len&~NOT_GREATER_THAN_MASK)) {
				DEBUG_ERR("Invalid tag length of %s [%d]!\n", name, tag_len);
				return NULL;
			}			
		}
		else { // equal
			if (tag_len != check_len) {
				DEBUG_ERR("Invalid tag length of %s [%d]!\n", name, tag_len);
				return NULL;
			}
		}		
	}	
#ifdef DEBUG
	if (pCtx->debug2) {
		unsigned short usVal;
		unsigned long ulVal;
		char tmp[512];
		
		if (type == TYPE_BYTE) {
			printf("%s: 0x%x\n", name, pData[0]);
		}
		else if (type == TYPE_WORD) {
			memcpy(&usVal, pData, 2);
			printf("%s: 0x%x\n", name, ntohs(usVal));			
		}
		else if (type == TYPE_DWORD) {
			memcpy(&ulVal, pData, 4);
			printf("%s: 0x%x\n", name, (int)ntohl(ulVal));			
		}
		else if (type == TYPE_STR) {
			memcpy(tmp, pData, tag_len);
			if (tmp[tag_len-1] != 0)
				tmp[tag_len] = '\0';
			printf("%s: %s\n", name, tmp);
		}
		else // TYPE_BIN
		{
			debug_out(name, pData, tag_len);
		}
	}
#endif
	*o_len = tag_len;
	return pData;
}

static int check_authenticator_attr(STA_CTX_Tp pSta, unsigned char *pMsg, int msg_len)
{
	unsigned char *pData;
	int tag_len, size;
	char tmp[100];

	pData = search_tag(pMsg, TAG_AUTHENTICATOR, msg_len, &tag_len);
	if (pData == NULL) {
		DEBUG_ERR("Can't find TAG_AUTHENTICATOR!\n");
		return -1;
	}
	if (tag_len != BYTE_LEN_64B) {
		DEBUG_ERR("Invalid length of Authenticator [%d]!\n", tag_len);
		return -1;
	}
	
#ifdef DEBUG
//	if (pCtx->debug) 
//		debug_out("Authenticator", pData, tag_len);	
#endif
	size = (int)(((unsigned long)pData) - ((unsigned long)pMsg) - 4);
	append(&pSta->last_tx_msg_buffer[pSta->last_tx_msg_size], pMsg, size);
	hmac_sha256(pSta->last_tx_msg_buffer, pSta->last_tx_msg_size+size, pSta->auth_key, BYTE_LEN_256B, tmp, &size);
	if (memcmp(tmp, pData, BYTE_LEN_64B)) {
		DEBUG_ERR("hmac value of Authenticator mismatched!\n");
		report_WPS_STATUS(HMAC_FAIL);
		return -1;
	}
	return 0;
}

static int decrypt_attr(CTX_Tp pCtx, STA_CTX_Tp pSta, unsigned char *pMsg, int msg_len, char *out)
{
	unsigned char *pData;
	int tag_len, size;
#ifdef DEBUG
	unsigned char tmp[200];
#endif

	pData = search_tag(pMsg, TAG_ENCRYPT_SETTINGS, msg_len, &tag_len);
	if (pData == NULL) {
		DEBUG_ERR("Can't find TAG_ENCRYPT_SETTINGS\n");
		return -1;
	}

	if (tag_len < (BYTE_LEN_128B+4+NONCE_LEN+4+BYTE_LEN_64B)) {
		DEBUG_ERR("Invalid length (of EncryptedSettings [%d]!\n", tag_len);
		return -1;		
	}

#ifdef DEBUG
//	if (pCtx->debug)
//		debug_out("IV", pData, BYTE_LEN_128B);
#endif

	Decrypt_aes_128_cbc(pSta->key_wrap_key, pData, out, &size, &pData[BYTE_LEN_128B], tag_len-BYTE_LEN_128B);

#ifdef DEBUG
	if (pCtx->debug2) {
		sprintf(tmp, "Plaintext of EncryptedSettings: len=%d", size);
		debug_out(tmp, out, size);		
	}
#endif

	pData = check_tag(pCtx, out, size, TAG_KEY_WRAP_AUTH, BYTE_LEN_64B, "KeyWrapAuthenticator", TYPE_BIN, &tag_len);
	if (pData == NULL)
		return -1;

	size = size-BYTE_LEN_64B-4;
#if 0
	hmac_sha256(out, size, pSta->auth_key, BYTE_LEN_256B, tmp, &tag_len);
	if (memcmp(&out[size-BYTE_LEN_64B], tmp, BYTE_LEN_64B)) {
		DEBUG_ERR("hmac value of KWA mismatched!\n");
		return -1;		
	}
#endif	
	return (size);
}

static int GetNetworkProfile(const CTX_Tp pCtx, const unsigned char *pMsg, const int msg_len)
{
	unsigned char *pData, *pVal, *pMsg_start=NULL;
	int size, tag_len;
	unsigned short sVal;
	unsigned char key_index=0, num_of_assigned_wep_key=0;
	unsigned char pre_wep_key_format=0, pre_wep_key_len=0;
	unsigned char wep_key_tmp[MAX_WEP_KEY_LEN+1];
	int assigned_wep_len=0;
	
	pData = (unsigned char *)pMsg;
	size = msg_len;

#ifdef WPS2DOTX
	if (pCtx->current_wps_version == WPS_VERSION_V2) {
		// TAG_NETWORKKEY_SHAREABLE
		pVal = check_tag(pCtx, pData, size, TAG_NETWORKKEY_SHAREABLE,1 , "network key shareable", TYPE_BYTE, &tag_len);
		if (pVal){
				RX_DEBUG("network key shareable set to %d\n",pVal[0]);
		}
	}	
#endif

	RX_DEBUG("Profile from Registrar:\n");	
	/*get SSID*/
	pVal = check_tag(pCtx, pData, size, TAG_SSID, 
				NOT_GREATER_THAN_MASK|MAX_SSID_LEN, "SSID", TYPE_STR, &tag_len);
	if (pVal == NULL || tag_len < 1) {
		DEBUG_ERR("Invalid SSID!\n");
		return -1;
	}
	memcpy(pCtx->assigned_ssid, pVal, tag_len);
	pCtx->assigned_ssid[tag_len] = '\0';
	RX_DEBUG("	SSID:\"%s\"\n",pCtx->assigned_ssid);


	/*get member mac addr*/
	if (check_tag(pCtx, pData, size, TAG_MAC_ADDRESS, NOT_GREATER_THAN_MASK|ETHER_ADDRLEN, 
		"MAC Address", TYPE_BIN, &tag_len) == NULL)
		return CONFIG_ERR_CANNOT_CONNECT_TO_REG;
	
	/*get Auth Type*/	
	pVal = check_tag(pCtx, pData, size, TAG_AUTH_TYPE, 2, "AuthenticationType", TYPE_WORD, &tag_len);
	if (pVal == NULL)
		return -1;
	memcpy(&sVal, pVal, 2);	
	pCtx->assigned_auth_type = ntohs(sVal);	

	if (!(pCtx->assigned_auth_type & pCtx->auth_type_flags)) {
		DEBUG_ERR("Invalid assigned_auth_type = %d; not supported\n", pCtx->assigned_auth_type);
		return CONFIG_ERR_CANNOT_CONNECT_TO_REG;
	}
	RX_DEBUG("	Auth_type :0x%02X\n",pCtx->assigned_auth_type);
	RX_DEBUG("	OPEN=1,WPAPSK=2,SHARED=4,WPA2PSK=0x20,WPA2PSKMIXED=0x22\n");	


	/*get Encrypt Type*/	
	pVal = check_tag(pCtx, pData, size, TAG_ENCRYPT_TYPE, 2, "EncryptionType", TYPE_WORD, &tag_len);
	if (pVal == NULL)
		return -1;
	memcpy(&sVal, pVal, 2);	
	pCtx->assigned_encrypt_type = ntohs(sVal);
	if (!(pCtx->assigned_encrypt_type & pCtx->encrypt_type_flags)) {
		DEBUG_ERR("Invalid assigned_encrypt_type = %d; not supported\n", pCtx->assigned_encrypt_type);
		return CONFIG_ERR_CANNOT_CONNECT_TO_REG;
	}

#ifdef WPS2DOTX
	if (pCtx->current_wps_version == WPS_VERSION_V2) {
		/* handle When 1.0 ER want to set WPA+TKIP security mode ; test plan 4.1.12*/ 
		if (pCtx->assigned_auth_type == AUTH_WPAPSK &&
			 pCtx->assigned_encrypt_type == ENCRYPT_TKIP)
		{
			pCtx->assigned_auth_type = AUTH_WPA2PSKMIXED;
			pCtx->assigned_encrypt_type = ENCRYPT_TKIPAES;
			RX_DEBUG(".....!!!under WPS2.0 ;when the (WPA-PSK+TKIP )case setting to Mixed mode!!!\n\n");

		}	

		/* habdle When 1.0 ER want to set WEP security mode ;return NACK ; test plan 4.1.10*/ 
		if(	pCtx->assigned_encrypt_type == ENCRYPT_WEP ){
			RX_DEBUG(".......!!!under WPS2.0 ; reject the  Encrpty== WEP case!!! \n\n");
			if (pCtx->pin_timeout>3) {
				pCtx->pin_timeout=3;
			}
			if (pCtx->pb_timeout>3) {
				pCtx->pb_timeout=3;
			}
			
			return CONFIG_ERR_CANNOT_CONNECT_TO_REG;
		}
	}
#endif	
	RX_DEBUG("	Encry_type :%d\n",pCtx->assigned_encrypt_type);	
	RX_DEBUG("	(NONE=1,WEP=2,TKIP=4,AES=8,TKIPAES=12)\n");	

	// Add WPA2-TKIP support for WLK v1.2, david+2008-05-27
	#if 0
	//#ifdef CONFIG_RTL8186_KB
	if (pCtx->assigned_auth_type == AUTH_WPA2PSK&& 
		pCtx->assigned_encrypt_type == ENCRYPT_TKIP)
		return CONFIG_ERR_CANNOT_CONNECT_TO_REG;
	#endif

	if ((pCtx->assigned_auth_type == AUTH_OPEN && pCtx->assigned_encrypt_type > ENCRYPT_WEP) ||
		((pCtx->assigned_auth_type != AUTH_OPEN && pCtx->assigned_auth_type != AUTH_SHARED)
			&& pCtx->assigned_encrypt_type <= ENCRYPT_WEP)) {
		DEBUG_ERR("Invalid assigned_auth_type = %d and assigned_encrypt_type = %d\n", pCtx->assigned_auth_type, pCtx->assigned_encrypt_type);
		return CONFIG_ERR_CANNOT_CONNECT_TO_REG;
	}

	while (1) {	
		//currently only support multiple keys in wep mode
		if (pCtx->assigned_encrypt_type == ENCRYPT_WEP) {

#ifdef WPS2DOTX
			if (pCtx->current_wps_version == WPS_VERSION_V2) {
				RX_DEBUG("WEP under WPS2.0 is be deprecated\n");
				break;
			}
#endif

			pMsg_start = pData;
			pVal = check_tag(pCtx, pData, size, TAG_NETWORK_KEY_INDEX, 1, "NetworkKeyIndex", TYPE_BYTE, &tag_len);
			
			if (pVal == NULL) {
				if (!num_of_assigned_wep_key) { //no TAG_NETWORK_KEY_INDEX tag; default to 1
					key_index = 1;
				}
				else if (num_of_assigned_wep_key == 1) {

					if(strlen(pCtx->assigned_wep_key_1)){
						strcpy(pCtx->assigned_wep_key_2, pCtx->assigned_wep_key_1);
						strcpy(pCtx->assigned_wep_key_3, pCtx->assigned_wep_key_1);
						strcpy(pCtx->assigned_wep_key_4, pCtx->assigned_wep_key_1);

					}else if(strlen(pCtx->assigned_wep_key_2)){
						strcpy(pCtx->assigned_wep_key_1, pCtx->assigned_wep_key_2);
						strcpy(pCtx->assigned_wep_key_3, pCtx->assigned_wep_key_2);
						strcpy(pCtx->assigned_wep_key_4, pCtx->assigned_wep_key_2);

					}
					else if(strlen(pCtx->assigned_wep_key_3)){
						strcpy(pCtx->assigned_wep_key_1, pCtx->assigned_wep_key_3);
						strcpy(pCtx->assigned_wep_key_2, pCtx->assigned_wep_key_3);
						strcpy(pCtx->assigned_wep_key_4, pCtx->assigned_wep_key_3);

					}
					else if(strlen(pCtx->assigned_wep_key_4)){
						strcpy(pCtx->assigned_wep_key_1, pCtx->assigned_wep_key_4);
						strcpy(pCtx->assigned_wep_key_2, pCtx->assigned_wep_key_4);
						strcpy(pCtx->assigned_wep_key_3, pCtx->assigned_wep_key_4);

					}					
					DEBUG_PRINT("pCtx->assigned_wep_key_len = %d\n", pCtx->assigned_wep_key_len);
					DEBUG_PRINT("pCtx->assigned_wep_key_format = %d\n", pCtx->assigned_wep_key_format);
					break;
				}
				else if (num_of_assigned_wep_key == 2 || num_of_assigned_wep_key == 3) {
					
#ifdef DEBUG					
					RX_DEBUG("only 2/3 WEP key \n");

					if(!strlen(pCtx->assigned_wep_key_1))
						RX_DEBUG("WEP key 1=NULL\n");

					if(!strlen(pCtx->assigned_wep_key_2))
						RX_DEBUG("WEP key 2=NULL\n");

					if(!strlen(pCtx->assigned_wep_key_3))
						RX_DEBUG("WEP key 3=NULL\n");

					if(!strlen(pCtx->assigned_wep_key_4))
						RX_DEBUG("WEP key 4=NULL\n");
#endif

					break;
				}
				else if (num_of_assigned_wep_key == 4) {
					break;
				}
				else {
					DEBUG_ERR("Multiple wep keys not supported if number of provided keys is %d\n", num_of_assigned_wep_key);
					return CONFIG_ERR_CANNOT_CONNECT_TO_REG;
				}
			}
			else{
				key_index = *pVal;
				RX_DEBUG("get key index =%d \n",key_index);
			}

			num_of_assigned_wep_key++;
			if (num_of_assigned_wep_key > 4) {
				DEBUG_ERR("Multiple wep keys not supported if number of provided keys is greater than %d\n", num_of_assigned_wep_key);
				return CONFIG_ERR_CANNOT_CONNECT_TO_REG;
			}
		}

		pVal = check_tag(pCtx, pData, size, TAG_NETWORK_KEY, 
			NOT_GREATER_THAN_MASK|MAX_NETWORK_KEY_LEN, "NetworkKey", TYPE_BIN, &tag_len);
		
		/*2011-0519 found at Gemtek,when ER is Intel and encrypt is NONE intel will not include this attri*/		
		if (pCtx->assigned_encrypt_type != ENCRYPT_NONE) {		
			if (pVal == NULL){
				RX_DEBUG("!!!network key == NULL \n");
				return -1;
			}
		}

		if (pCtx->assigned_encrypt_type == ENCRYPT_NONE) {
			memset(pCtx->assigned_network_key, 0, MAX_NETWORK_KEY_LEN+1);
#if 0
			memcpy(pCtx->assigned_network_key, pVal, tag_len);
			pCtx->assigned_network_key[tag_len] = '\0';
			if (strlen(pCtx->assigned_network_key) > 0) {
				DEBUG_ERR("Error! auth type = %d, encrypt type = %d, network key = %s\n",
				pCtx->assigned_auth_type, pCtx->assigned_encrypt_type, pCtx->assigned_network_key);
					return -1;
			}
#endif
			break;
		}
		else if (pCtx->assigned_encrypt_type == ENCRYPT_WEP) {

#ifdef WPS2DOTX
			if (pCtx->current_wps_version == WPS_VERSION_V2) {
				RX_DEBUG("WEP under WPS2.0 is be deprecated\n");
				break;
			}
#endif
			
			#if 0
			if (key_index != num_of_assigned_wep_key) {
				DEBUG_ERR("Not supported: invalid wep key index = %d; num_of_assigned_wep_key = %d!\n", key_index, num_of_assigned_wep_key);
				return CONFIG_ERR_CANNOT_CONNECT_TO_REG;
			}
			#endif

			if (check_wep_key_format(pVal, tag_len, &pCtx->assigned_wep_key_format, &pCtx->assigned_wep_key_len, wep_key_tmp, &assigned_wep_len) < 0)
				return CONFIG_ERR_CANNOT_CONNECT_TO_REG;
			else {
				#if 0				
				if (key_index > 1 && (pre_wep_key_format != pCtx->assigned_wep_key_format ||
					pre_wep_key_len != pCtx->assigned_wep_key_len)) {
					DEBUG_ERR("Format or length mismatch among assigned keys\n");
					return CONFIG_ERR_CANNOT_CONNECT_TO_REG;
				}
				#endif				
				pre_wep_key_format = pCtx->assigned_wep_key_format;
				pre_wep_key_len = pCtx->assigned_wep_key_len;
			}
				
			RX_DEBUG("KEY = \"%s\" , index = %d\n",wep_key_tmp , key_index);	
				
			switch (key_index)
			{
				case 1:
					memcpy(pCtx->assigned_wep_key_1, wep_key_tmp, assigned_wep_len);
					pCtx->assigned_wep_key_1[assigned_wep_len] = '\0';
					break;
				case 2:
					memcpy(pCtx->assigned_wep_key_2, wep_key_tmp, assigned_wep_len);
					pCtx->assigned_wep_key_2[assigned_wep_len] = '\0';
					break;
				case 3:
					memcpy(pCtx->assigned_wep_key_3, wep_key_tmp, assigned_wep_len);
					pCtx->assigned_wep_key_3[assigned_wep_len] = '\0';
					break;
				case 4:
					memcpy(pCtx->assigned_wep_key_4, wep_key_tmp, assigned_wep_len);
					pCtx->assigned_wep_key_4[assigned_wep_len] = '\0';
					break;
				default: //should not go in here; just in case
					DEBUG_ERR("Error: invalid wep key index = %d!\n", key_index);
					return CONFIG_ERR_CANNOT_CONNECT_TO_REG;
			}

			pData = pVal + tag_len;
			size -= (pData - pMsg_start);
		}
		else {
			memcpy(pCtx->assigned_network_key, pVal, tag_len);
			pCtx->assigned_network_key[tag_len] = '\0';
			if (strlen(pCtx->assigned_network_key) < 8) {
				DEBUG_ERR("Error! network key too short [%s]\n", pCtx->assigned_network_key);
				return -1;
			}

			break; //not support multiple keys yet
		}
	}

	RX_DEBUG("	key :\"%s\"\n",pCtx->assigned_network_key);	
		
	if (pCtx->assigned_encrypt_type == ENCRYPT_WEP) {
		if ((pVal = check_tag(pCtx, pData, size, TAG_WEP_TRANSMIT_KEY, 1, "WEPTransmitKey", TYPE_BYTE, &tag_len)) == NULL){
			//no TAG_NETWORK_KEY_INDEX tag; default to 1			
			//pCtx->assigned_wep_transmit_key = 1;
			pCtx->assigned_wep_transmit_key = key_index;
		}else{
			pCtx->assigned_wep_transmit_key = *pVal;
		}
		
		RX_DEBUG("wep_transmit_key = %d\n", pCtx->assigned_wep_transmit_key);
	}
	
	return 0;
}

static int decrypt_setting(CTX_Tp pCtx, STA_CTX_Tp pSta, unsigned char *pMsg, int msg_len)
{
	unsigned char tmpbuf[1024], *pData;
	int size, tag_len;
	
	size = decrypt_attr(pCtx, pSta, pMsg, msg_len, tmpbuf);
	if (size < 0)
		return -1;

	pData = search_tag(tmpbuf, TAG_CREDENTIAL, size, &tag_len);
	if (pData == NULL) 
		pData = tmpbuf;
	else
		size = tag_len;

#ifdef WPS2DOTX
	/*under WPS2.0 deprecated NetworkKetIndex*/
	if (pCtx->current_wps_version == WPS_VERSION_V1)
#endif
	{
		if (!pCtx->is_ap) {
			if (check_tag(pCtx, pData, size, TAG_NETWORK_INDEX, 1, "NetworkIndex", TYPE_BYTE, &tag_len) == NULL)
				return -1;
		}
	}

	return GetNetworkProfile(pCtx, pData, size);
}

static int check_nonce(unsigned char *pMsg, int msg_len, int tag, unsigned char *nonce, char *name)
{
	unsigned char *pData;
	int tag_len;

	pData = search_tag(pMsg, tag, msg_len, &tag_len);
	if (pData == NULL) {
		DEBUG_ERR("Can't find %s tag!\n", name);
		return -1;
	}
	if (tag_len != NONCE_LEN) {
		DEBUG_ERR("Invalid length of %s [%d]!\n", name, tag_len);
		return -1;
	}	
#ifdef DEBUG
//	if (pCtx->debug) 
//		debug_out("Enrollee Nonce", pData, tag_len);	
#endif

	if (memcmp(pData, nonce, NONCE_LEN)) {
		DEBUG_ERR("%s mismatch!\n", name);
		return -1;
	}
	return 0;
}

#ifdef SUPPORT_REGISTRAR
static int msgHandler_M7(CTX_Tp pCtx, STA_CTX_Tp pSta, unsigned char *pMsg, int msg_len)
{
	unsigned char *pData, *ptr;
	int tag_len, size, ret;
	char tmpbuf[1024];
	char tmp1[200], tmp2[200], tmp[200];

	DBFENTER;

	_DEBUG_PRINT("\n<< Receive EAP WSC_MSG M7\n\n");

	report_WPS_STATUS(RECV_M7);

	if (pSta->state != ST_WAIT_M7) {
		RX_DEBUG("Invalid state [%d]!\n", pSta->state);
		return 0;
	}

	if (check_authenticator_attr(pSta, pMsg, msg_len) < 0)
		return -1;

	if (check_nonce(pMsg, msg_len, TAG_REGISTRAR_NONCE,  pSta->nonce_registrar, "RegistarNonce") < 0)
		return -1;

	size = decrypt_attr(pCtx, pSta, pMsg, msg_len, tmpbuf);
	if (size < 0)
		return -1;

	pData = check_tag(pCtx, tmpbuf, size, TAG_E_SNONCE2, NONCE_LEN, "E-S2", TYPE_BIN, &tag_len);
	if (pData == NULL)
		return -1;
	memcpy(pSta->e_s2, pData, tag_len);

#if defined(CLIENT_MODE) && defined(SUPPORT_REGISTRAR)
	if (!pCtx->is_ap && pCtx->role == REGISTRAR && 
		pSta->config_state == CONFIG_STATE_CONFIGURED) {
		ret = GetNetworkProfile(pCtx, tmpbuf, size);
		if (ret != 0) {
			memset(pCtx->assigned_ssid, 0, ((unsigned long)pCtx->sta - (unsigned long)pCtx->assigned_ssid));
			return ret;
		}
		else {
			send_wsc_nack(pCtx, pSta, CONFIG_ERR_NO_ERR);
			pCtx->start = 0;
			pCtx->wait_reinit = write_param_to_flash(pCtx, 0);
			return 0;
		}
	}
#endif

	/*check E-Hash1*/ 
	hmac_sha256(pCtx->peer_pin_code, strlen(pCtx->peer_pin_code)/2, pSta->auth_key, BYTE_LEN_256B, tmp, &size);
	memcpy(tmpbuf, pSta->e_s1, NONCE_LEN);
	ptr = append(&tmpbuf[BYTE_LEN_128B], tmp, BYTE_LEN_128B);

	BN_bn2bin(pSta->dh_enrollee->p, tmp1);
	ptr = append(ptr, tmp1, PUBLIC_KEY_LEN);

	BN_bn2bin(pSta->dh_registrar->pub_key, tmp2);
	ptr = append(ptr, tmp2, PUBLIC_KEY_LEN);
	
	size = (int)(((unsigned long)ptr) - ((unsigned long)tmpbuf));
	hmac_sha256(tmpbuf, size, pSta->auth_key, BYTE_LEN_256B, tmp, &size);	
	
	if (memcmp(tmp, pSta->e_h1, BYTE_LEN_256B))
	{
#ifdef DEBUG
		if (pCtx->debug2) {
			RX_DEBUG("E-Hash1 mismatched!\n");		
			debug_out("E-Hash1(enrollee)", tmp, BYTE_LEN_256B);
			debug_out("E-Hash1(my keep)", pSta->e_h1, BYTE_LEN_256B);		
		}
#endif
		report_WPS_STATUS(HASH_FAIL);
		return -1;
	}
		
	/*check E-Hash2*/ 
	hmac_sha256(&pCtx->peer_pin_code[strlen(pCtx->peer_pin_code)/2], strlen(pCtx->peer_pin_code)/2, pSta->auth_key, BYTE_LEN_256B, tmp, &size);
	memcpy(tmpbuf, pSta->e_s2, NONCE_LEN);
	ptr = append(&tmpbuf[BYTE_LEN_128B], tmp, BYTE_LEN_128B);
	ptr = append(ptr, tmp1, PUBLIC_KEY_LEN);
	ptr = append(ptr, tmp2, PUBLIC_KEY_LEN);

	size = (int)(((unsigned long)ptr) - ((unsigned long)tmpbuf));
	hmac_sha256(tmpbuf, size, pSta->auth_key, BYTE_LEN_256B, tmp, &size);

	if (memcmp(tmp, pSta->e_h2, BYTE_LEN_256B))
	{
#ifdef DEBUG
		if (pCtx->debug2){
			RX_DEBUG("E-Hash2 mismatched!\n");
			debug_out("E-Hash2(Enrollee)", tmp, BYTE_LEN_256B);			
			debug_out("E-Hash2(my keep)", pSta->e_h2, BYTE_LEN_256B);
		}
#endif
		report_WPS_STATUS(HASH_FAIL);
		return -1;
	}

	pSta->tx_timeout = pCtx->tx_timeout;
	ret = send_wsc_M8(pCtx, pSta);
#ifdef WPS2DOTX
	if ((pSta->state == ST_WAIT_EAPOL_FRAG_ACK_M8) && (pCtx->current_wps_version == WPS_VERSION_V2)) {

	}else
#endif
	{
		if (ret < 0) {
			if (pSta->invoke_security_gen)
				pSta->invoke_security_gen = 0;
			return -1;
		}

		pSta->state = ST_WAIT_DONE;
		pSta->tx_timeout = pCtx->tx_timeout;	
		pSta->retry = 0;
	}
	return 0;
}

static int msgHandler_M5(CTX_Tp pCtx, STA_CTX_Tp pSta, unsigned char *pMsg, int msg_len)
{
	unsigned char *pData;
	int tag_len, size;
	char tmpbuf[1024];

	DBFENTER;

	_DEBUG_PRINT("\n<< Receive EAP WSC_MSG M5\n\n");

        report_WPS_STATUS(RECV_M5);

	if (pSta->state != ST_WAIT_M5) {
		RX_DEBUG("Invalid state [%d]!\n", pSta->state);
		return 0;
	}

	if (check_authenticator_attr(pSta, pMsg, msg_len) < 0)
		return -1;

	if (check_nonce(pMsg, msg_len, TAG_REGISTRAR_NONCE,  pSta->nonce_registrar, "RegistarNonce") < 0)
		return -1;

	size = decrypt_attr(pCtx, pSta, pMsg, msg_len, tmpbuf);
	if (size < 0)
		return -1;

	pData = check_tag(pCtx, tmpbuf, size, TAG_E_SNONCE1, NONCE_LEN, "E-S1", TYPE_BIN, &tag_len);
	if (pData == NULL)
		return -1;

	// got E-S1
	memcpy(pSta->e_s1, pData, tag_len);

	pSta->tx_timeout = pCtx->tx_timeout;
	send_wsc_M6(pCtx, pSta);
	
#ifdef WPS2DOTX
	if((pSta->state == ST_WAIT_EAPOL_FRAG_ACK_M6) && (pCtx->current_wps_version == WPS_VERSION_V2)){

	}else
#endif
	{
		pSta->state = ST_WAIT_M7;
		pSta->tx_timeout = pCtx->tx_timeout;	
		pSta->retry = 0;
	}
	return 0;
}

static int msgHandler_M3(CTX_Tp pCtx, STA_CTX_Tp pSta, unsigned char *pMsg, int msg_len)
{
	unsigned char *pData;
	int tag_len;

	DBFENTER;

	_DEBUG_PRINT("\n<< Receive EAP WSC_MSG M3\n\n");

        report_WPS_STATUS(RECV_M3);

	if (pSta->state != ST_WAIT_M3) {
		RX_DEBUG("Invalid state [%d]!\n", pSta->state);
		return 0;
	}

	if (check_authenticator_attr(pSta, pMsg, msg_len) < 0)
		return -1;

	if (check_nonce(pMsg, msg_len, TAG_REGISTRAR_NONCE,  pSta->nonce_registrar, "RegistarNonce") < 0)
		return -1;

	pData = check_tag(pCtx, pMsg, msg_len, TAG_E_HASH1, BYTE_LEN_256B, "E-Hash1", TYPE_BIN, &tag_len);
	if (pData == NULL)
		return -1;
	memcpy(pSta->e_h1, pData, tag_len);

	pData = check_tag(pCtx, pMsg, msg_len, TAG_E_HASH2, BYTE_LEN_256B, "E-Hash2", TYPE_BIN, &tag_len);
	if (pData == NULL)
		return -1;
	memcpy(pSta->e_h2, pData, tag_len);

	pSta->tx_timeout = pCtx->tx_timeout;
	
	send_wsc_M4(pCtx, pSta);
	
#ifdef WPS2DOTX
	if((pSta->state == ST_WAIT_EAPOL_FRAG_ACK_M4) && (pCtx->current_wps_version == WPS_VERSION_V2)) {

	}else
#endif
	{
		pSta->state = ST_WAIT_M5;
		pSta->tx_timeout = pCtx->tx_timeout;
		pSta->retry = 0;
	}

	return 0;
}

static int msgHandler_M1(CTX_Tp pCtx, STA_CTX_Tp pSta, unsigned char *pMsg, int msg_len)
{
	unsigned char *pData;
	int tag_len, ret;
	unsigned short sVal;

	DBFENTER;

	_DEBUG_PRINT("\n<< Receive EAP WSC_MSG M1\n\n");

        report_WPS_STATUS(RECV_M1);

	if (pSta->state != ST_WAIT_M1) {
		RX_DEBUG("Invalid state [%d]!\n", pSta->state);
		return 0;
	}

	pData = check_tag(pCtx, pMsg, msg_len, TAG_UUID_E, UUID_LEN, "UUID-E", TYPE_BIN, &tag_len);
	if (pData == NULL)
		return -1;
	memcpy(pSta->uuid, pData, UUID_LEN);

	pData = check_tag(pCtx, pMsg, msg_len, TAG_MAC_ADDRESS, ETHER_ADDRLEN, "MACAddress", TYPE_BIN, &tag_len);
	if (pData == NULL){		
		return -1;
	}
#if 0  // for Intel SDK
	if (memcmp(pData, pSta->addr, ETHER_ADDRLEN)) {
		DEBUG_ERR("MAC address is mismatched![%02x:%02x:%02x:%02x:%02x:%02x]\n",
			pData[0],pData[1],pData[2],pData[3],pData[4],pData[5]);
		return -1;		
	}
#else
	memcpy(pSta->msg_addr, pData, ETHER_ADDRLEN);
#endif

#if	0	//def WPS2DOTX
	pData = check_tag(pCtx, pMsg, msg_len, TAG_REQ_TO_ENROLL, 1, "Req to Enroll", TYPE_BYTE, &tag_len);	
	
	if(pData){
		Ptr = pSta->msg_addr;
		RX_DEBUG("found REQ_TO_ENROLL [from %02x%02x%02x-%02x%02x%02x]\n",Ptr[0],Ptr[1],Ptr[2],Ptr[3],Ptr[4],Ptr[5]);
	}
#endif

	pData = check_tag(pCtx, pMsg, msg_len, TAG_EROLLEE_NONCE, NONCE_LEN, "EnrolleeNonce", TYPE_BIN, &tag_len);
	if (pData == NULL)
		return -1;
	memcpy(pCtx->nonce_enrollee, pData, tag_len);
	memcpy(pSta->nonce_enrollee, pData, tag_len);

	pData = check_tag(pCtx, pMsg, msg_len, TAG_PUB_KEY, PUBLIC_KEY_LEN, "PublicKey", TYPE_BIN, &tag_len);
	if (pData == NULL)
		return -1;
	
	pSta->dh_enrollee = generate_dh_parameters(PUBLIC_KEY_LEN*8, pData, DH_GENERATOR_2);
	if (pSta->dh_enrollee == NULL)
		return -1;

	if(pCtx->debug2)
		debug_out("dh_enrollee" ,(void *)pSta->dh_enrollee , sizeof(struct dh_st));
	
	if ((pData = check_tag(pCtx, pMsg, msg_len, TAG_AUTH_TYPE_FLAGS, 2, "AuthenticationTypeFlags", TYPE_WORD, &tag_len)) == NULL)
		return CONFIG_ERR_NET_AUTH_FAIL;
	pSta->auth_type_flags = ntohs(*((unsigned short *)pData));

#if 0	// sometimes STA's Auth is wrong , for IOT let it by pass
	if (!(pSta->auth_type_flags & pCtx->auth_type_flags)) {
		DEBUG_ERR("Enrollee uses auth_type_flags= %d; not supported by current setting\n", pSta->auth_type_flags);
		return -1;
	}
#endif
	if ((pData = check_tag(pCtx, pMsg, msg_len, TAG_ENCRYPT_TYPE_FLAGS, 2, "EncryptionTypeFlags", TYPE_WORD, &tag_len)) == NULL)
		return CONFIG_ERR_NET_AUTH_FAIL;
	pSta->encrypt_type_flags = ntohs(*((unsigned short *)pData));
	
#ifdef DEBUG
	RX_DEBUG("STA support Auth:0x%02X\n",pSta->auth_type_flags);
	RX_DEBUG("STA support Encrypt: %d\n",pSta->encrypt_type_flags);
	show_auth_encry_help();
#endif	

#if 0	// sometimes STA's Encry is wrong , for IOT let it by pass
	if (!(pSta->encrypt_type_flags & pCtx->encrypt_type_flags)) {
		DEBUG_ERR("Enrollee uses encrypt_type_flags= %d; not supported by current setting\n", pSta->encrypt_type_flags);
		return -1;
	}
#endif

#ifdef WPS2DOTX
	if ((pSta->encrypt_type_flags==ENCRYPT_WEP) && (pCtx->current_wps_version == WPS_VERSION_V2)) {
		RX_DEBUG("Enrollee only support WEP Encrypt_type (%d) not supported by 2.0 AP\n", 
			pSta->encrypt_type_flags);
		return -1;
	}	
#endif
	
	if (check_tag(pCtx, pMsg, msg_len, TAG_CONNECT_TYPE_FLAGS, 1, "ConnectionTypeFlags", TYPE_BYTE, &tag_len) == NULL)
		return -1;
	pData = check_tag(pCtx, pMsg, msg_len, TAG_CONFIG_METHODS, 2, "ConfigMethods", TYPE_WORD, &tag_len);
	if (pData == NULL)
		return -1;
	memcpy(&sVal, pData, 2);
	sVal = ntohs(sVal);	
	pSta->config_method = sVal;

	pData = check_tag(pCtx, pMsg, msg_len, TAG_SIMPLE_CONFIG_STATE, 1, "SimpleConfigState", TYPE_BYTE, &tag_len);
	if (pData == NULL)
		return -1;

#if defined(CLIENT_MODE) && defined(SUPPORT_REGISTRAR)
	if (!pCtx->is_ap && pCtx->role == REGISTRAR) {
		pSta->config_state = (unsigned char)(*pData);
        #ifdef DEBUG_UPNP
		RX_DEBUG("(ER)Enrollee's state = %d\n", (int)pSta->config_state);
        #endif
	}
#endif

	if (check_tag(pCtx, pMsg, msg_len, TAG_MANUFACTURER, 
			NOT_GREATER_THAN_MASK|MAX_MANUFACT_LEN, "Manufacture", TYPE_STR, &tag_len) == NULL)
		return -1;
	if (check_tag(pCtx, pMsg, msg_len, TAG_MODEL_NAME, 
			NOT_GREATER_THAN_MASK|MAX_MODEL_NAME_LEN, "ModelName", TYPE_STR, &tag_len) == NULL)
		return -1;
	if (check_tag(pCtx, pMsg, msg_len, TAG_MODEL_NUMBER, 
			NOT_GREATER_THAN_MASK|MAX_MODEL_NUM_LEN, "ModelNumber", TYPE_STR, &tag_len) == NULL)
		return -1;
	if (check_tag(pCtx, pMsg, msg_len, TAG_SERIAL_NUM, 
			NOT_GREATER_THAN_MASK|MAX_SERIAL_NUM_LEN, "SerailNumber", TYPE_STR, &tag_len) == NULL)
		return -1;

	pData = search_tag(pMsg, TAG_PRIMARY_DEVICE_TYPE, msg_len, &tag_len);
	if (pData == NULL) {
		DEBUG_ERR("Can't find TAG_PRIMARY_DEVICE_TYPE\n");
		return -1;
	}

	if(pCtx->debug2)
	DEBUG_PRINT("Primary Device Type: len=%d, category_id=0x%x, oui=%02x%02x%02x%02x, sub_category_id=0x%x\n",
		tag_len, ntohs(*((unsigned short *)pData)), pData[2],pData[3],pData[4],pData[5],ntohs(*((unsigned short *)&pData[6])));

	pData = check_tag(pCtx, pMsg, msg_len, TAG_DEVICE_NAME, 
		NOT_GREATER_THAN_MASK|MAX_DEVICE_NAME_LEN, "DeviceName", TYPE_STR, &tag_len);
	if ( pData == NULL){
		return -1;
	}
	if (check_tag(pCtx, pMsg, msg_len, TAG_RF_BAND, 1, "RFBand", TYPE_BYTE, &tag_len) == NULL)
		return -1;
	if (check_tag(pCtx, pMsg, msg_len, TAG_ASSOC_STATE, 2, "AssociationState", TYPE_WORD, &tag_len) == NULL)
		return -1;
	pData = check_tag(pCtx, pMsg, msg_len, TAG_DEVICE_PASSWORD_ID, 2, "DevicePasswordID", TYPE_WORD, &tag_len);
	if (pData == NULL)
		return -1;
	memcpy(&sVal, pData, 2);
	pSta->device_password_id = ntohs(sVal);

	if(pCtx->debug)
		RX_DEBUG("pSta->device_password_id=%d \n",pSta->device_password_id);

#ifdef MUL_PBC_DETECTTION
	if (pCtx->is_ap && IS_PBC_METHOD(pCtx->config_method) && 
		pSta->device_password_id == PASS_ID_PB &&
		!pCtx->disable_MulPBC_detection) {
		WSC_pthread_mutex_lock(&pCtx->PBCMutex);
		//DEBUG_PRINT("%s %d Lock PBC mutex\n", __FUNCTION__, __LINE__);
		search_active_pbc_sta(pCtx, pSta->addr, pSta->uuid);
		WSC_pthread_mutex_unlock(&pCtx->PBCMutex);
		//DEBUG_PRINT("%s %d unlock PBC mutex\n", __FUNCTION__, __LINE__);
	}
#endif
	
	if (check_tag(pCtx, pMsg, msg_len, TAG_CONFIG_ERR, 2, "ConfigurationError", TYPE_WORD, &tag_len) == NULL)
		return -1;
	if (check_tag(pCtx, pMsg, msg_len, TAG_OS_VERSION, 4, "OSVersion", TYPE_DWORD, &tag_len) == NULL)
		return -1;

	pSta->tx_timeout = pCtx->tx_timeout;
	ret = send_wsc_M2(pCtx, pSta);

#ifdef WPS2DOTX
	if ((pSta->state == ST_WAIT_EAPOL_FRAG_ACK_M2) && (pCtx->current_wps_version == WPS_VERSION_V2)) {

	}else
#endif	
	{
		if (ret < 0) {
			DEBUG_ERR("send_wsc_M2() error!\n");
			return -1;
		}
		if (ret == MSG_TYPE_M2)
			pSta->state = ST_WAIT_M3;
		else
			pSta->state = ST_WAIT_ACK;
		pSta->tx_timeout = pCtx->tx_timeout;
		pSta->retry = 0;
	}

	return 0;
}
#endif // SUPPORT_REGISTRAR

#ifdef SUPPORT_ENROLLEE
static int msgHandler_M8(CTX_Tp pCtx, STA_CTX_Tp pSta, unsigned char *pMsg, int msg_len)
{
	int ret = 0;
	
	DBFENTER;
	
	_DEBUG_PRINT("\n<< Receive EAP WSC_MSG M8\n\n");
	report_WPS_STATUS(RECV_M8);

	if (pSta->state != ST_WAIT_M8) {
		RX_DEBUG("Invalid state [%d]!\n", pSta->state);
		return 0;
	}

	if (check_authenticator_attr(pSta, pMsg, msg_len) < 0)
		return -1;

	if (check_nonce(pMsg, msg_len, TAG_EROLLEE_NONCE,  pSta->nonce_enrollee, "EnrolleeNonce") < 0)
		return -1;

	ret = decrypt_setting(pCtx, pSta, pMsg, msg_len);
	if (ret != 0) {
		memset(pCtx->assigned_ssid, 0, ((unsigned long)pCtx->sta - (unsigned long)pCtx->assigned_ssid));
		return ret;
	}

#ifdef P2P_SUPPORT
if(pCtx->p2p_trigger_type==P2P_PRE_CLIENT){
	RX_DEBUG("\n\n");
	strcpy(pCtx->p2p_peers_ssid,pCtx->assigned_ssid);
	strcpy(pCtx->p2p_peers_psk,pCtx->assigned_network_key); 	   
}else
#endif 
{
#ifdef FOR_DUAL_BAND
	if(pCtx->is_ap){ 
		if(pCtx->InterFaceComeIn == COME_FROM_WLAN0)
			pCtx->wait_reinit = write_param_to_flash(pCtx, 0);
		else if(pCtx->InterFaceComeIn == COME_FROM_WLAN1)
			pCtx->wait_reinit = write_param_to_flash2(pCtx, 0);  //  1001
	}
	else{
		pCtx->wait_reinit = write_param_to_flash(pCtx, 0);
	}
#else
	pCtx->wait_reinit = write_param_to_flash(pCtx, 0);
#endif
}

	if (pCtx->wait_reinit != REINIT_SYS)
		memset(pCtx->assigned_ssid, 0, ((unsigned long)pCtx->sta - (unsigned long)pCtx->assigned_ssid));

	send_wsc_done(pCtx, pSta);
	report_WPS_STATUS(PROTOCOL_SUCCESS);

	if (pCtx->is_ap)
	reset_ctx_state(pCtx);

	if (wlioctl_set_led(pCtx->wlan_interface_name, LED_WSC_SUCCESS) < 0) {
		DEBUG_ERR("issue wlan ioctl set_led error!\n");	
	}


#ifdef MUL_PBC_DETECTTION
	if (pCtx->is_ap && IS_PBC_METHOD(pCtx->config_method) && 
		pSta->device_password_id == PASS_ID_PB &&
		!pCtx->disable_MulPBC_detection) {
		WSC_pthread_mutex_lock(&pCtx->PBCMutex);
		DEBUG_PRINT("%s %d Lock PBC mutex\n", __FUNCTION__, __LINE__);
		remove_active_pbc_sta(pCtx, pSta, 1);
		WSC_pthread_mutex_unlock(&pCtx->PBCMutex);
		//DEBUG_PRINT("%s %d unlock PBC mutex\n", __FUNCTION__, __LINE__);
	}
#endif

#ifdef SUPPORT_UPNP
	if (pSta->used & IS_UPNP_CONTROL_POINT) {
		pCtx->upnp_wait_reboot_timeout = UPNP_WAIT_REBOOT;
		pSta->tx_timeout = 0;
		pCtx->status_changed = 1;
	}
	else
#endif
	{	if (pCtx->is_ap)
		pSta->state = ST_WAIT_ACK;
		else
			pSta->state = ST_WAIT_EAP_FAIL;
		pSta->tx_timeout = pCtx->tx_timeout;
	}
	pSta->retry = 0;

#if	0	//def CLIENT_MODE ; no need  ; remove
	if (!pCtx->is_ap && pCtx->wait_reinit){
		RX_DEBUG("signal_webs\n");		
		signal_webs(pCtx->wait_reinit);		
	}
#endif	
	
	return 0;
}

static int msgHandler_M6(CTX_Tp pCtx, STA_CTX_Tp pSta, unsigned char *pMsg, int msg_len)
{
	unsigned char *pData, *ptr;
	int tag_len, size;
	char tmpbuf[1024];
	char tmp1[200], tmp[200], tmp2[200];

	DBFENTER;
	
	_DEBUG_PRINT("\n<< Receive EAP WSC_MSG M6\n\n");
        report_WPS_STATUS(RECV_M6);

	if (pSta->state != ST_WAIT_M6) {
		RX_DEBUG("Invalid state [%d]!\n", pSta->state);
		return 0;
	}

	if (check_authenticator_attr(pSta, pMsg, msg_len) < 0)
		return -1;

	if (check_nonce(pMsg, msg_len, TAG_EROLLEE_NONCE,  pSta->nonce_enrollee, "EnrolleeNonce") < 0)
		return -1;

	size = decrypt_attr(pCtx, pSta, pMsg, msg_len, tmpbuf);
	if (size < 0)
		return -1;

	pData = check_tag(pCtx, tmpbuf, size, TAG_R_SNONCE2, NONCE_LEN, "R-S2", TYPE_BIN, &tag_len);
	if (pData == NULL)
		return -1;
	memcpy(pSta->r_s2, pData, tag_len);

	/* check R-Hash2 */ 
	hmac_sha256(&pCtx->pin_code[strlen(pCtx->pin_code)/2], strlen(pCtx->pin_code)/2, pSta->auth_key, BYTE_LEN_256B, tmp, &size);
	memcpy(tmpbuf, pSta->r_s2, NONCE_LEN);
	ptr = append(&tmpbuf[BYTE_LEN_128B], tmp, BYTE_LEN_128B);
	BN_bn2bin(pSta->dh_enrollee->pub_key, tmp1);
	ptr = append(ptr, tmp1, PUBLIC_KEY_LEN);
	BN_bn2bin(pSta->dh_registrar->p, tmp2);
	ptr = append(ptr, tmp2, PUBLIC_KEY_LEN);
	size = (int)(((unsigned long)ptr) - ((unsigned long)tmpbuf));
	hmac_sha256(tmpbuf, size, pSta->auth_key, BYTE_LEN_256B, tmp, &size);
	if (memcmp(tmp, pSta->r_h2, BYTE_LEN_256B)) {
		RX_DEBUG("M6 R-Hash2 mismatched!\n");
		debug_out("My keep:", pSta->r_h2, BYTE_LEN_256B);
		debug_out("from STA:", tmp, BYTE_LEN_256B);		

		report_WPS_STATUS(HASH_FAIL);
#ifdef	AUTO_LOCK_DOWN
		record_and_check_AuthFail(pCtx);
#endif
		return CONFIG_ERR_DEV_PASS_AUTH_FAIL;
	}
#ifdef	AUTO_LOCK_DOWN
#ifdef ALD_BRUTEFORCE_ATTACK_MITIGATION				
	/*if even PIN is correct recount it*/
	pCtx->ADL_pin_attack_count=0;	
	RX_DEBUG("Reset pin fail count to 0\n");
#endif
#endif

	send_wsc_M7(pCtx, pSta);

#ifdef WPS2DOTX	
	if ((pSta->state == ST_WAIT_EAPOL_FRAG_ACK_M7) && (pCtx->current_wps_version == WPS_VERSION_V2)) {
		return 0;
	} 
#endif

	pSta->state = ST_WAIT_M8;
	if (!(pSta->used & IS_UPNP_CONTROL_POINT))
		pSta->tx_timeout = pCtx->tx_timeout;
	else
		pSta->tx_timeout = 15;
	pSta->retry = 0;

	return 0;
}

static int msgHandler_M4(CTX_Tp pCtx, STA_CTX_Tp pSta, unsigned char *pMsg, int msg_len)
{
	unsigned char *pData;
	unsigned char *ptr;	
	int tag_len, size;
	char tmpbuf[1024];
	char tmp1[200], tmp[200], tmp2[200];

	DBFENTER;
	
	_DEBUG_PRINT("\n<< Receive EAP WSC_MSG M4\n\n");
	report_WPS_STATUS(RECV_M4);

	if (pSta->state != ST_WAIT_M4 ) 
	{
		RX_DEBUG("Invalid state [%d]!\n", pSta->state);
		//brad modify for pass Vista WCN Error Handling and Tempering, return 0 will cause re-send M3 to Vista, the action 
		//can not pass the test
		//if (pCtx->is_ap) 
			return -1;
		//else
		//	return 0;
	}

	if (check_authenticator_attr(pSta, pMsg, msg_len) < 0)
		return -1;

	if (check_nonce(pMsg, msg_len, TAG_EROLLEE_NONCE,  pSta->nonce_enrollee, "EnrolleeNonce") < 0)
		return -1;

	pData = check_tag(pCtx, pMsg, msg_len, TAG_R_HASH1, BYTE_LEN_256B, "R-Hash1", TYPE_BIN, &tag_len);
	if (pData == NULL)
		return -1;
	memcpy(pSta->r_h1, pData, tag_len);

	pData = check_tag(pCtx, pMsg, msg_len, TAG_R_HASH2, BYTE_LEN_256B, "R-Hash2", TYPE_BIN, &tag_len);
	if (pData == NULL)
		return -1;
	memcpy(pSta->r_h2, pData, tag_len);

	size = decrypt_attr(pCtx, pSta, pMsg, msg_len, tmpbuf);
	if (size < 0)
		return -1;

	pData = check_tag(pCtx, tmpbuf, size, TAG_R_SNONCE1, NONCE_LEN, "R-S1", TYPE_BIN, &tag_len);
	if (pData == NULL)
		return -1;
	memcpy(pSta->r_s1, pData, tag_len);

	/* check R-Hash1 for detect PIN number left half Error*/ 
	hmac_sha256(pCtx->pin_code, strlen(pCtx->pin_code)/2, pSta->auth_key, BYTE_LEN_256B, tmp, &size);
	memcpy(tmpbuf, pSta->r_s1, NONCE_LEN);
	ptr = append(&tmpbuf[BYTE_LEN_128B], tmp, BYTE_LEN_128B);
	BN_bn2bin(pSta->dh_enrollee->pub_key, tmp1);
	ptr = append(ptr, tmp1, PUBLIC_KEY_LEN);
	BN_bn2bin(pSta->dh_registrar->p, tmp2);
	ptr = append(ptr, tmp2, PUBLIC_KEY_LEN);
	size = (int)(((unsigned long)ptr) - ((unsigned long)tmpbuf));
	hmac_sha256(tmpbuf, size, pSta->auth_key, BYTE_LEN_256B, tmp, &size);

	if (memcmp(tmp, pSta->r_h1, BYTE_LEN_256B)) {

		DEBUG_ERR("R-Hash1 mismatched!\n");
#ifdef DEBUG
		if(pCtx->debug2){
			debug_out("M4(My) R-Hash1", tmp, BYTE_LEN_256B);
			debug_out("M4(Reg) R-Hash1", pSta->r_h1, BYTE_LEN_256B);
		}
#endif

#ifdef	AUTO_LOCK_DOWN
		record_and_check_AuthFail(pCtx);
#endif
		return CONFIG_ERR_DEV_PASS_AUTH_FAIL ;		
	}

	send_wsc_M5(pCtx, pSta);
#ifdef WPS2DOTX	
	if ((pSta->state == ST_WAIT_EAPOL_FRAG_ACK_M5) && (pCtx->current_wps_version == WPS_VERSION_V2)) {
		return 0;
	}
#endif	

	
	pSta->state = ST_WAIT_M6;
	if (!(pSta->used & IS_UPNP_CONTROL_POINT))
		pSta->tx_timeout = pCtx->tx_timeout;
	else
		pSta->tx_timeout = 15;
	pSta->retry = 0;
	
	return 0;
}

static int msgHandler_M2(CTX_Tp pCtx, STA_CTX_Tp pSta, unsigned char *pMsg, int msg_len, int code)
{
	unsigned char *pData;
	int tag_len, is_m2=0, tmp;
	unsigned short sVal;

	DBFENTER;
	
	if (code == MSG_TYPE_M2)
		is_m2 = 1;

#ifdef AUTO_LOCK_DOWN
	if(pCtx->auto_lock_down)
		return CONFIG_ERR_SETUP_LOCKED;
#endif

        report_WPS_STATUS(((is_m2 == 1) ? RECV_M2 : RECV_M2D ));
	_DEBUG_PRINT("\n<< Receive EAP WSC_MSG %s\n\n", ((is_m2 == 1) ? "M2" : "M2D"));

	if (pSta->state != ST_WAIT_M2 && pSta->state != ST_WAIT_EAP_FAIL) {
		RX_DEBUG("Invalid state [%d]!\n", pSta->state);
		return CONFIG_ERR_OOB_INTERFACE_READ_ERR;	//WPS2.0
	}

	if (check_nonce(pMsg, msg_len, TAG_EROLLEE_NONCE, pSta->nonce_enrollee, "EnrolleeNonce") < 0)
		return CONFIG_ERR_OOB_INTERFACE_READ_ERR;	//WPS2.0

	pData = check_tag(pCtx, pMsg, msg_len, TAG_REGISTRAR_NONCE, NONCE_LEN, "RegistrarNonce", TYPE_BIN, &tag_len);
	if (pData == NULL)
		return CONFIG_ERR_OOB_INTERFACE_READ_ERR;	//WPS2.0
	
	memcpy(pSta->nonce_registrar, pData, tag_len);

	/*Disable PIN method*/
	if (!IS_PIN_METHOD(pCtx->config_method) && (pSta->device_password_id != PASS_ID_PB)) {
		DEBUG_ERR("Recv device_password_id=%d ,but PIN method is not configured!\n", pSta->device_password_id);
		return CONFIG_ERR_CANNOT_CONNECT_TO_REG;
	}
	
	/*Disable configured by ER*/
	if(pCtx->is_ap){
		/*whatever config or unconfig state all reject config by ER*/		
		if ((pCtx->disable_configured_by_exReg == 2))	 
		{
			RX_DEBUG("	Disable configured by ER!!\n\n");
			return CONFIG_ERR_CANNOT_CONNECT_TO_REG;
		}	
		else if ((pCtx->disable_configured_by_exReg == 1))	 
		{
			if(pCtx->config_state == CONFIG_STATE_CONFIGURED){
				RX_DEBUG("	Disable configured by ER!!\n\n");
				return CONFIG_ERR_CANNOT_CONNECT_TO_REG;
			}
			#ifdef DEBUG
			else{
				RX_DEBUG("under unconfigured state allow config by ER\n\n");
			}
			#endif
		}	
		
	}
	pData = check_tag(pCtx, pMsg, msg_len, TAG_UUID_R, UUID_LEN, "UUID-R", TYPE_BIN, &tag_len);
	if (pData == NULL)
		return CONFIG_ERR_OOB_INTERFACE_READ_ERR;	//WPS2.0

//#ifdef MUL_PBC_DETECTTION
	memcpy(pSta->uuid, pData, UUID_LEN);
//#endif

	if (is_m2) {
		pData = check_tag(pCtx, pMsg, msg_len, TAG_PUB_KEY, PUBLIC_KEY_LEN, "PublicKey", TYPE_BIN, &tag_len);
		if (pData == NULL)
			return CONFIG_ERR_OOB_INTERFACE_READ_ERR;	//WPS2.0

		pSta->dh_registrar= generate_dh_parameters(PUBLIC_KEY_LEN*8, pData, DH_GENERATOR_2);
		if (pSta->dh_registrar == NULL)
			return CONFIG_ERR_OOB_INTERFACE_READ_ERR;	//WPS2.0		
	}

	pData = check_tag(pCtx, pMsg, msg_len, TAG_AUTH_TYPE_FLAGS, 2, "AuthenticationTypeFlags", TYPE_WORD, &tag_len);
	if (pData == NULL)
		return CONFIG_ERR_NET_AUTH_FAIL;
	else {
		if (code == MSG_TYPE_M2) {
			tmp = ntohs(*((unsigned short *)pData));
			if (!(tmp & pCtx->auth_type_flags)) {
				DEBUG_ERR("Registrar Authentication Type Flags = %d; not supported!\n", tmp);
				return CONFIG_ERR_CANNOT_CONNECT_TO_REG;
			}
		}
	}
	pSta->auth_type_flags = ntohs(*((unsigned short *)pData));

	pData = check_tag(pCtx, pMsg, msg_len, TAG_ENCRYPT_TYPE_FLAGS, 2, "EncryptionTypeFlags", TYPE_WORD, &tag_len);
	if (pData == NULL)
		return CONFIG_ERR_NET_AUTH_FAIL;
	else {
		if (code == MSG_TYPE_M2) {
			tmp = ntohs(*((unsigned short *)pData));
			if (!(tmp & pCtx->encrypt_type_flags)) {
				DEBUG_ERR("Registrar Encryption Type Flags = %d; not supported!\n", tmp);
				return CONFIG_ERR_CANNOT_CONNECT_TO_REG;
			}
#ifdef WPS2DOTX
			if ((tmp ==ENCRYPT_WEP) && (pCtx->current_wps_version == WPS_VERSION_V2)) {
				RX_DEBUG("Enrollee uses Encrypt_type_flags= %d; not supported by  2.0\n", tmp);
				return -1;
			}			
#endif

		}
	}
	pSta->encrypt_type_flags = ntohs(*((unsigned short *)pData));
	
	if (check_tag(pCtx, pMsg, msg_len, TAG_CONNECT_TYPE_FLAGS, 1, "ConnectionTypeFlags", TYPE_BYTE, &tag_len) == NULL)
		return -1;

	pData = check_tag(pCtx, pMsg, msg_len, TAG_CONFIG_METHODS, 2, "ConfigMethods", TYPE_WORD, &tag_len);
	if (pData == NULL)
		return CONFIG_ERR_NET_AUTH_FAIL;
#if 0 // for Intel SDK
	else {
		if (code == MSG_TYPE_M2) {
			tmp = ntohs(*((unsigned short *)pData));
			if (pCtx->config_method & tmp) {
				int i=0;
				if (pCtx->config_method & CONFIG_METHOD_ETH)
					i = pCtx->config_method - CONFIG_METHOD_ETH;
				if (tmp & CONFIG_METHOD_ETH)
					tmp -= CONFIG_METHOD_ETH;
				if (!(i & tmp)) {
					DEBUG_ERR("Config method not supported!\n");
					return CONFIG_ERR_CANNOT_CONNECT_TO_REG;
				}
			}
			else {
				DEBUG_ERR("Config method not supported!\n");
				return CONFIG_ERR_CANNOT_CONNECT_TO_REG;
			}
		}
	}
#endif

	if (pCtx->is_ap) {
		if (code == MSG_TYPE_M2) {

			//tmp = ntohs(*((unsigned short *)pData));
			//Fix, makes wscd silently killed running on TI platform
			memcpy(&tmp,pData,sizeof(short));
			tmp = ntohs(tmp);
			if (tmp == CONFIG_METHOD_PBC) {
				DEBUG_ERR("PBC could not be supported when AP is configured by an external registrar!\n");
				return CONFIG_ERR_CANNOT_CONNECT_TO_REG;
			}
		}
	}

	if (check_tag(pCtx, pMsg, msg_len, TAG_MANUFACTURER, 
		NOT_GREATER_THAN_MASK|MAX_MANUFACT_LEN, "Manufacture", TYPE_STR, &tag_len) == NULL)
		return -1;
	
	if (check_tag(pCtx, pMsg, msg_len, TAG_MODEL_NAME, 
		NOT_GREATER_THAN_MASK|MAX_MODEL_NAME_LEN, "ModelName", TYPE_STR, &tag_len) == NULL)
		return -1;
	
	if (check_tag(pCtx, pMsg, msg_len, TAG_MODEL_NUMBER, 
		NOT_GREATER_THAN_MASK|MAX_MODEL_NUM_LEN, "ModelNumber", TYPE_STR, &tag_len) == NULL)
		return -1;
	if (check_tag(pCtx, pMsg, msg_len, TAG_SERIAL_NUM, 
		NOT_GREATER_THAN_MASK|MAX_SERIAL_NUM_LEN, "SerailNumber", TYPE_STR, &tag_len) == NULL)
		return -1;

	pData = search_tag(pMsg, TAG_PRIMARY_DEVICE_TYPE, msg_len, &tag_len);
	if (pData == NULL) {
		DEBUG_ERR("Can't find TAG_PRIMARY_DEVICE_TYPE\n");
		return -1;
	}

	if(pCtx->debug2)
		DEBUG_PRINT("Primary Device Type: len=%d, category_id=0x%x, oui=%02x%02x%02x%02x, sub_category_id=0x%x\n",
		tag_len, ntohs(*((unsigned short *)pData)), pData[2],pData[3],pData[4],pData[5],ntohs(*((unsigned short *)&pData[6])));

	if (check_tag(pCtx, pMsg, msg_len, TAG_DEVICE_NAME, 
		NOT_GREATER_THAN_MASK|MAX_DEVICE_NAME_LEN, "DeviceName", TYPE_STR, &tag_len) == NULL)
		return -1;
	if (check_tag(pCtx, pMsg, msg_len, TAG_RF_BAND, 1, "RFBand", TYPE_BYTE, &tag_len) == NULL)
		return -1;
	if (check_tag(pCtx, pMsg, msg_len, TAG_ASSOC_STATE, 2, "AssociationState", TYPE_WORD, &tag_len) == NULL)
		return -1;
	
	if (is_m2) {
		pData = check_tag(pCtx, pMsg, msg_len, TAG_DEVICE_PASSWORD_ID, 2, "DevicePasswordID", TYPE_WORD, &tag_len);
		if (pData == NULL)
			return -1;
		memcpy(&sVal, pData, 2);
		pSta->device_password_id = ntohs(sVal);
		
#ifdef MUL_PBC_DETECTTION
		if (pCtx->is_ap && IS_PBC_METHOD(pCtx->config_method) &&
			pSta->device_password_id == PASS_ID_PB &&
			!pCtx->disable_MulPBC_detection) {
			WSC_pthread_mutex_lock(&pCtx->PBCMutex);
			//DEBUG_PRINT("%s %d Lock PBC mutex\n", __FUNCTION__, __LINE__);
			
			search_active_pbc_sta(pCtx, pSta->addr, pSta->uuid);
			if (pCtx->active_pbc_sta_count > 1 && pCtx->pb_pressed) {
				RX_DEBUG("\n\n		!!Multiple PBC sessions [%d] detected!\n\n", pCtx->active_pbc_sta_count);
				WSC_pthread_mutex_unlock(&pCtx->PBCMutex);
				//DEBUG_PRINT("%s %d unlock PBC mutex\n", __FUNCTION__, __LINE__);
				
				SwitchSessionOverlap_LED_On(pCtx);
				return CONFIG_ERR_MUL_PBC_DETECTED;
			}
			
			WSC_pthread_mutex_unlock(&pCtx->PBCMutex);
			//DEBUG_PRINT("%s %d unlock PBC mutex\n", __FUNCTION__, __LINE__);
		}
#endif
	}
	if (check_tag(pCtx, pMsg, msg_len, TAG_CONFIG_ERR, 2, "ConfigurationError", TYPE_WORD, &tag_len) == NULL)
		return -1;
	if (check_tag(pCtx, pMsg, msg_len, TAG_OS_VERSION, 4, "OSVersion", TYPE_DWORD, &tag_len) == NULL)
		return -1;

	if (is_m2) {
		if (derive_key(pCtx, pSta) < 0)
			return -1;

		if (check_authenticator_attr(pSta, pMsg, msg_len) < 0)
			return -1;

		send_wsc_M3(pCtx, pSta);

#ifdef WPS2DOTX	
		if ((pSta->state == ST_WAIT_EAPOL_FRAG_ACK_M3) && (pCtx->current_wps_version == WPS_VERSION_V2)){
			return 0;
		}
#endif
		pSta->state = ST_WAIT_M4;
	}
	else {	/*handle M2D*/
		if (!(pSta->used & IS_UPNP_CONTROL_POINT)) {
			if (pCtx->is_ap){
				send_wsc_nack(pCtx, pSta, CONFIG_ERR_NO_ERR);
			}else {
				send_wsc_ack(pCtx, pSta);
				pSta->state = ST_WAIT_EAP_FAIL;
			}
		}
		else {
			 send_wsc_ack(pCtx, pSta);
			 pSta->state = ST_WAIT_ACK;
			 return -1;
		}
	}

	if (!(pSta->used & IS_UPNP_CONTROL_POINT))
		pSta->tx_timeout = pCtx->tx_timeout;
	else
		pSta->tx_timeout = 15;
	pSta->retry = 0;

	return 0;
}

int pktHandler_reqid(CTX_Tp pCtx, STA_CTX_Tp pSta, unsigned char id)
{
	DBFENTER;
	
        report_WPS_STATUS(SEND_EAP_IDREQ);
	
	_DEBUG_PRINT("\n<< Receive EAP REQUEST / Identity packet\n");

	//fix problem: when DUT is STA mode ;multiple EAP  id-reqs from AP only the first processed	
	if( pSta->state == ST_WAIT_START )
		pSta->state = ST_WAIT_REQ_ID;

	if (pSta->state != ST_WAIT_REQ_ID) {
		RX_DEBUG("Invalid state [%d], discard packet!\n", pSta->state);
		return 0;
	}
	pSta->eap_reqid = id;

	send_eap_rspid(pCtx, pSta);

	if (pCtx->role == ENROLLEE)
		pSta->state = ST_WAIT_START;
	else
		pSta->state = ST_WAIT_M1;

	pSta->tx_timeout = pCtx->tx_timeout;
	pSta->retry = 0;
	
	return 0;
}

int pktHandler_wsc_start(CTX_Tp pCtx, STA_CTX_Tp pSta)
{
	DBFENTER;

	_DEBUG_PRINT("\n<< Receive EAP WSC_Start\n");

	if (pSta->state != ST_WAIT_START) {
		RX_DEBUG("Invalid state [%d], discard packet!\n", pSta->state);
		return 0;
	}
        report_WPS_STATUS(RECV_EAPOL_START);

	send_wsc_M1(pCtx, pSta);

#ifdef WPS2DOTX
	if ((pSta->state == ST_WAIT_EAPOL_FRAG_ACK_M1) && (pCtx->current_wps_version == WPS_VERSION_V2)){
		return 0;
	} 
#endif

	pSta->state = ST_WAIT_M2;
	pSta->tx_timeout = pCtx->tx_timeout;
	pSta->retry = 0;

	return 0;
}
#endif // SUPPORT_ENROLLEE

int pktHandler_rspid(CTX_Tp pCtx, STA_CTX_Tp pSta, unsigned char *id, int len)
{
#if defined(DEBUG) || defined(PRINT_ERR)
	char eap_id[512];

	memcpy(eap_id, id, len);
	eap_id[len] = '\0';
#endif

	DBFENTER;

	_DEBUG_PRINT("<< Receive EAP RESPONSE / Identity packet>>; Rsp-id = %s\n", eap_id);

	if (pSta->state != ST_WAIT_RSP_ID) {
		RX_DEBUG("Invalid state [%d], discard packet!\n", pSta->state);
		return 0;
	}
        report_WPS_STATUS(RECV_EAP_IDRSP);

	//WSC_pthread_mutex_lock(&pCtx->RegMutex);
	//DEBUG_PRINT("%s %d Lock mutex\n", __FUNCTION__, __LINE__);
	
	if (pCtx->registration_on == 0) {
		pCtx->role = pCtx->original_role;
		DEBUG_PRINT("\nRoll back role to : %s\n", (pCtx->role==PROXY ? "Proxy" : (pCtx->role==ENROLLEE ? "Enrollee" : "Registrar")));
	}
	else {
		DEBUG_PRINT("%s %d Registration protocol is already in progress; ignore << Receive EAP RESPONSE / Identity packet>\n", __FUNCTION__, __LINE__);
		
		// Reason code 5 : Disassociated because AP is unable to handle all currently associated stations
		if ((len == strlen(EAP_ID_ENROLLEE) && !memcmp(id, EAP_ID_ENROLLEE, len)) ||
			(len == strlen(EAP_ID_REGISTRAR) && !memcmp(id, EAP_ID_REGISTRAR, len)) ||
			pSta->Assoc_wscIE_included){
			
			IssueDisconnect(pCtx->wlan_interface_name, pSta->addr, 5);
			RX_DEBUG("IssueDisconnect\n\n");
		}
		reset_sta(pCtx, pSta, 1);
		
		//WSC_pthread_mutex_unlock(&pCtx->RegMutex);
		//DEBUG_PRINT("%s %d unlock mutex\n", __FUNCTION__, __LINE__);
		
		return -1;
	}
		
	if (len == strlen(EAP_ID_ENROLLEE) && !memcmp(id, EAP_ID_ENROLLEE, len)) {
#ifdef SUPPORT_UPNP
		// Fix the issue of WLK v1.2 M1<->M2D proxy ---------------
		//	if ((pCtx->original_role == REGISTRAR || pCtx->original_role == PROXY) &&
		if (!pCtx->pb_pressed && !pCtx->pin_assigned && 
				pCtx->upnp && pCtx->TotalSubscriptions){
			pCtx->role = PROXY;
		}else		//------------------------------------ david+2008-05-27			
#endif
		{
			pCtx->role = REGISTRAR;
		}

		if(pCtx->debug)	
			RX_DEBUG("Role = %s\n", (pCtx->role==PROXY ? "Proxy" : (pCtx->role==ENROLLEE ? "Enrollee" : "Registrar")));

	}
	else if (len == strlen(EAP_ID_REGISTRAR) && !memcmp(id, EAP_ID_REGISTRAR, len)) {
		pCtx->role = ENROLLEE;
		pSta->ap_role = ENROLLEE;

		if(pCtx->debug)	
			RX_DEBUG("Role = %s\n", (pCtx->role==PROXY ? "Proxy" : (pCtx->role==ENROLLEE ? "Enrollee" : "Registrar")));
		
#if 0
		if (pCtx->is_ap && pCtx->disable_configured_by_exReg) {
			RX_DEBUG("	!!!disable_configured_by_exReg\n\n");
#ifdef BLOCKED_ROGUE_STA
			add_into_blocked_list(pCtx, pSta);
#endif
			reset_sta(pCtx, pSta, 1);
			return -1;
		}
#endif
	}
	else {
		DEBUG_ERR("Invalid EAP-Response ID = %s\n", eap_id);
		if (pSta->Assoc_wscIE_included) {
			// Reason code 1 : Unspecified reason
			IssueDisconnect(pCtx->wlan_interface_name, pSta->addr, 1);
			RX_DEBUG("IssueDisconnect\n\n");
		}
		reset_sta(pCtx, pSta, 1);

		//WSC_pthread_mutex_unlock(&pCtx->RegMutex);
		//DEBUG_PRINT("%s %d unlock mutex\n", __FUNCTION__, __LINE__);
		return -1;		
	}


	if (pCtx->role == REGISTRAR) {
		send_wsc_start(pCtx, pSta);
		pSta->state = ST_WAIT_M1;
	}
#ifdef SUPPORT_UPNP
	else if (pCtx->role == PROXY) {
		send_wsc_start(pCtx, pSta);
		pSta->state = ST_UPNP_WAIT_M1;
	}
#endif
	else {
#ifdef SUPPORT_ENROLLEE		
		send_wsc_M1(pCtx, pSta);
		pSta->state = ST_WAIT_M2;
#endif		
	}
		
	//WSC_pthread_mutex_unlock(&pCtx->RegMutex);
	//DEBUG_PRINT("%s %d unlock mutex\n", __FUNCTION__, __LINE__);
		
	pSta->tx_timeout = pCtx->tx_timeout;
	pSta->retry = 0;
	
	return 0;
}

int pktHandler_wsc_ack(CTX_Tp pCtx, STA_CTX_Tp pSta, struct eap_wsc_t *wsc)
{
	DBFENTER;

	_DEBUG_PRINT("\n\n<< Receive WSC_ACK packet\n");
        report_WPS_STATUS(PROC_EAP_ACK);

#ifdef SUPPORT_UPNP

	/*
	* for delay send eap-fail when ER > 1 ;search related code by 20101102 
	* if have >1  ER exist then there are >1 M2D will Rsp from ER , 
	* Enroll will Rsp for each M2D; AP(Proxy mode ) will send EAP-fail 
	* after last EAP-ACK(from Enroll) 
	*/
	if (pCtx->role == PROXY && pCtx->TotalSubscriptions  &&
			 pSta->state == ST_WAIT_ACK  
	){ 

		pSta->ER_RspM2D_delaytime = 2; 
		RX_DEBUG("	Rx ACK from STA (Rsp M2D)\n");
		return 0;
	}


	// Fix the issue of WLK v1.2 M1<->M2D proxy ---------------
	if (pCtx->role == PROXY && pCtx->TotalSubscriptions > 1 &&
					pSta->state != ST_UPNP_WAIT_DONE
					
	){ 
		return 0;
	}
#endif		
//------------------------------------ david+2008-05-27

	send_eap_fail(pCtx, pSta);

	sleep(1);
	
	// Reason code 1 : Unspecified reason
	if (pCtx->is_ap){
		IssueDisconnect(pCtx->wlan_interface_name, pSta->addr, 1);
		RX_DEBUG("IssueDisconnect\n\n");
	}
	
	reset_sta(pCtx, pSta, 1);

	return 0;	

}

int pktHandler_wsc_nack(CTX_Tp pCtx, STA_CTX_Tp pSta, struct eap_wsc_t *wsc)
{
	DBFENTER;

	_DEBUG_PRINT("\n<< Receive WSC_NACK packet\n");

	// Reason code 1 : Unspecified reason
	if (pCtx->is_ap) {
		send_eap_fail(pCtx, pSta);

#ifdef WPS2DOTX
		if (pCtx->is_ap && pCtx->use_ie && (pCtx->current_wps_version == WPS_VERSION_V2))
			clear_SetSelectedRegistrar_flag(pCtx);
#endif
		sleep(1);

#ifdef BLOCKED_ROGUE_STA
		if (pCtx->blocked_expired_time &&
			(pSta->state >= ST_WAIT_M4 && pSta->state <= ST_WAIT_M8) &&
			(pCtx->sta_invoke_reg == pSta && pCtx->registration_on >= 1) &&
			(pSta->ap_role != ENROLLEE)) {
			add_into_blocked_list(pCtx, pSta);
		}
		else
#endif
		{
			IssueDisconnect(pCtx->wlan_interface_name, pSta->addr, 1);
			RX_DEBUG("IssueDisconnect\n\n");
		}
		
		reset_sta(pCtx, pSta, 1);
	}
	else {
		send_wsc_nack(pCtx, pSta, CONFIG_ERR_NO_ERR);
		pSta->state = ST_WAIT_EAP_FAIL;
		pSta->tx_timeout = pCtx->tx_timeout;
		pSta->retry = 0;
	}

	return 0;	
}

int pktHandler_wsc_done(CTX_Tp pCtx, STA_CTX_Tp pSta)
{
	DBFENTER;
	
	_DEBUG_PRINT("\n<< Receive WSC_Done packet\n");
        report_WPS_STATUS(PROC_EAP_DONE);

#ifdef P2P_SUPPORT
	unsigned char Report2wlan ;
#endif

	if (pSta->state != ST_WAIT_DONE 
#ifdef SUPPORT_UPNP		
		&& pSta->state != ST_UPNP_WAIT_DONE
#endif		
		) {
		RX_DEBUG("Invalid state [%d]!\n", pSta->state);
		return 0;
	}
	
	if (pCtx->is_ap) {
		if (send_eap_fail(pCtx, pSta) < 0) {
			DEBUG_ERR("send_eap_fail() error!\n");
			return -1;
		}

#ifdef MUL_PBC_DETECTTION
		if (IS_PBC_METHOD(pCtx->config_method) && pSta->device_password_id == PASS_ID_PB &&
			!pCtx->disable_MulPBC_detection) {
			WSC_pthread_mutex_lock(&pCtx->PBCMutex);
			DEBUG_PRINT("%s %d Lock PBC mutex\n", __FUNCTION__, __LINE__);
			remove_active_pbc_sta(pCtx, pSta, 1);
			WSC_pthread_mutex_unlock(&pCtx->PBCMutex);
			//DEBUG_PRINT("%s %d unlock PBC mutex\n", __FUNCTION__, __LINE__);
		}
#endif

		report_WPS_STATUS(PROTOCOL_SUCCESS);
		DEBUG_PRINT("WPS protocol down(SUCCESS)\n");

		sleep(1); // wait a while till eap-fail is sent-out

		// Reason code 1 : Unspecified reason
		RX_DEBUG("IssueDisconnect\n");
		IssueDisconnect(pCtx->wlan_interface_name, pSta->addr, 1);

		reset_sta(pCtx, pSta, 1);
		reset_ctx_state(pCtx);	

#ifdef P2P_SUPPORT
		//indicate wlan driver that  WPS is success
		Report2wlan = GO_WPS_SUCCESS ;
		if (ReportWPSstate(GET_CURRENT_INTERFACE, &Report2wlan) < 0) {
			WSC_DEBUG("\n	Report p2p WPS state fail \n");
		}
				
		WSC_DEBUG("Report P2P GO WPS success\n");				
#endif
		/* start blinking when success*/
		if (wlioctl_set_led(pCtx->wlan_interface_name, LED_WSC_SUCCESS) < 0) {
			DEBUG_ERR("issue wlan ioctl set_led error!\n");	
		}

	}
	else {		
		if (send_wsc_ack(pCtx, pSta) < 0
#ifdef P2P_SUPPORT
		   || (pCtx->p2p_trigger_type==P2P_PRE_GO)
#endif
		) {
			if (pSta->invoke_security_gen)
				pSta->invoke_security_gen = 0;
			DEBUG_ERR("send_wsc_ack() error!\n");
			return -1;
		}

		if (pSta->invoke_security_gen) {
#ifdef FOR_DUAL_BAND
			if(pCtx->is_ap) {
				if(pCtx->InterFaceComeIn == COME_FROM_WLAN0)
					pCtx->wait_reinit = write_param_to_flash(pCtx, 1);
				else if(pCtx->InterFaceComeIn == COME_FROM_WLAN1)
					pCtx->wait_reinit = write_param_to_flash2(pCtx, 1);  //  1001
			}
			else {
				pCtx->wait_reinit = write_param_to_flash(pCtx, 1);
			}
#else //FOR_DUAL_BAND
			pCtx->wait_reinit = write_param_to_flash(pCtx, 1);
#endif //FOR_DUAL_BAND
			pSta->invoke_security_gen = 0;
		}
		else {
			if (pSta->state == ST_WAIT_DONE)
				pCtx->wait_reinit = REINIT_SYS;
		}
		
		pSta->state = ST_WAIT_EAP_FAIL;
		pSta->tx_timeout = pCtx->tx_timeout;
		pSta->retry = 0;		
	}
	return 0;
}

int pktHandler_eap_fail(CTX_Tp pCtx, STA_CTX_Tp pSta)
{
	DBFENTER;
	
	_DEBUG_PRINT("\n<< Receive EAP FAIL packet\n");

	if (pCtx->wait_reinit) {

		report_WPS_STATUS(PROTOCOL_SUCCESS);
		if (wlioctl_set_led(pCtx->wlan_interface_name, LED_WSC_SUCCESS) < 0) {
			DEBUG_ERR("issue wlan ioctl set_led error!\n");	
		}
		
		RX_DEBUG("	Success!, signal_webs wait to reinit\n");
		signal_webs(pCtx->wait_reinit);	
		pCtx->STAmodeSuccess = 1;
		reset_ctx_state(pCtx);

	}
#ifdef P2P_SUPPORT
	if(pCtx->p2p_trigger_type==P2P_PRE_CLIENT && strlen(pCtx->p2p_peers_ssid)>9){
	   RX_DEBUG("P2P_PRE_CLIENT do WPS DONE\n");
	   wlioctl_report_ssid_psk(GET_CURRENT_INTERFACE,pCtx->p2p_peers_ssid,pCtx->p2p_peers_psk);
	   reset_ctx_state(pCtx);				 
	   RX_DEBUG("p2p_peers_ssid[%s]\n",pCtx->p2p_peers_ssid);
	   RX_DEBUG("p2p_peers_psk[%s]\n",pCtx->p2p_peers_psk); 	   
	}
#endif 

	// Reason code 1 : Unspecified reason
	if (pCtx->is_ap) {//should not happen; just in case
#ifdef BLOCKED_ROGUE_STA
		if (pCtx->blocked_expired_time &&
			(pSta->state >= ST_WAIT_M4 && pSta->state <= ST_WAIT_M8) &&
			(pCtx->sta_invoke_reg == pSta && pCtx->registration_on >= 1) &&
			(pSta->ap_role != ENROLLEE)) {
			add_into_blocked_list(pCtx, pSta);
		}
		else
#endif
		{
			IssueDisconnect(pCtx->wlan_interface_name, pSta->addr, 1);
			RX_DEBUG("IssueDisconnect\n\n");
		}
	}

	reset_sta(pCtx, pSta, 1);
	RX_DEBUG("\n");		
	return 0;
}

int pktHandler_wsc_msg(CTX_Tp pCtx, STA_CTX_Tp pSta, struct eap_wsc_t * wsc, int len)
{
	unsigned char *pMsg, *pData;
	int msg_len, tag_len, ret=0;
	
	DBFENTER;

#ifdef FOR_DUAL_BAND
#ifdef DEBUG
	if(pCtx->debug2){
		if(pCtx->InterFaceComeIn == COME_FROM_WLAN1){
			RX_DEBUG("nego with wlan1\n");
		}else 	if(pCtx->InterFaceComeIn == COME_FROM_WLAN0){
			RX_DEBUG("nego with wlan0\n");
		}else{
			RX_DEBUG("nego with ?\n");
		}
	}
#endif
#endif
	
#ifdef SUPPORT_UPNP
	if (pSta->used & IS_UPNP_CONTROL_POINT) {
		msg_len = len;
		pMsg = ((struct WSC_packet *)wsc)->rx_buffer;		
	}
	else
#endif
	{
		msg_len = len - sizeof(struct eap_wsc_t);
		pMsg = (((unsigned char *)wsc) + sizeof(struct eap_wsc_t));
	}

#ifdef SUPPORT_UPNP
	if (pSta->state == ST_UPNP_WAIT_M1) { 
		pData = search_tag(pMsg, TAG_MSG_TYPE, msg_len, &tag_len);
		if (pData == NULL) {
			RX_DEBUG("Can't find TAG_MSG_TYPE\n");
			#ifdef DEBUG
			if(pCtx->debug2){
				debug_out("pmsg from Enroller",pMsg , msg_len);
			}
			#endif
			return -1;
		}
		if (pData[0] == MSG_TYPE_M1) {
			pSta->tx_timeout = 0;
			pSta->state = ST_UPNP_PROXY;
		}
		else {
			RX_DEBUG("Invalid Message Type [%d]! for UPnP-proxy\n", pData[0]);
			return -1;					
		}		
	}

	if (pSta->state == ST_UPNP_PROXY ||
		pSta->state == ST_UPNP_WAIT_DONE) { // UPnP msg, forward to ER
		
		struct WSC_packet packet;

		packet.EventType = WSC_8021XEAP_FRAME;
		packet.EventID = WSC_PUTWLANREQUEST;
		sprintf(packet.EventMac, "%02x:%02x:%02x:%02x:%02x:%02x",
			pSta->addr[0], pSta->addr[1], pSta->addr[2],
			pSta->addr[3], pSta->addr[4], pSta->addr[5]);
		packet.tx_buffer = pMsg;
		packet.tx_size = msg_len;		

		_DEBUG_PRINT("\n>> Forward STA's ( ");
#ifdef DEBUG
		pData = search_tag(pMsg, TAG_MSG_TYPE, msg_len, &tag_len);
		if(pData[0]==MSG_TYPE_M1){
			_DEBUG_PRINT("M1");
	 	}else if(pData[0]==MSG_TYPE_M3){
			_DEBUG_PRINT("M3");	
		}else if(pData[0]==MSG_TYPE_M5){
			_DEBUG_PRINT("M5");
		}else if(pData[0]==MSG_TYPE_M7){
			_DEBUG_PRINT("M7");				
		}else if(pData[0]==MSG_TYPE_ACK){
			_DEBUG_PRINT("ACK");				
		}else if(pData[0]==MSG_TYPE_NACK){
			_DEBUG_PRINT("NACK");
		}else if(pData[0]==MSG_TYPE_DONE){
			_DEBUG_PRINT("DONE");			
		}
#endif			
		_DEBUG_PRINT(" ) to ER\n");

		if (WSCUpnpTxmit(&packet) != WSC_UPNP_SUCCESS) {
			DEBUG_ERR("WSCUpnpTxmit() return error!\n");
			return -1;			
		}
		return 0;
	}
#ifdef DEBUG	
	else{	// 2011-0502 for debug 
		if(pSta->state == ST_WAIT_ACK){
			RX_DEBUG("Rx M2D before ,state change to ST_WAIT_ACK ,no forwarding EAP to Upnp\n");
			MAC_PRINT(pSta->addr);
		}
	}
#endif
#endif

	pData = search_tag(pMsg, TAG_VERSION, msg_len, &tag_len);
	if (pData == NULL) {
		DEBUG_ERR("Can't find TAG_VERSION\n");
		return -1;
	}

	DEBUG_PRINT2("Version: len=%d, val=0x%x\n", tag_len, pData[0]);

#ifdef WPS2DOTX
	if (pCtx->current_wps_version == WPS_VERSION_V2) {
		unsigned char *StmpPtr = pMsg;
		int msg_len_tmp = msg_len;
		int wvfound = 0;
		for (;;)
		{
			pData = search_tag(StmpPtr, TAG_VENDOR_EXT, msg_len_tmp, &tag_len);	
			if (pData != NULL) {
				if(!memcmp(pData , WSC_VENDOR_OUI ,3 ))
				{
					//RX_DEBUG("found WFA vendor OUI!!\n");
					#ifdef DEBUG
					if(pCtx->debug2){
						debug_out("verdor ext:",pData,tag_len );				
					}
					#endif
					
					wvfound = 1 ;
					break;
				}else{
					StmpPtr = pData + 4 + tag_len ;
					msg_len_tmp = msg_len - (int)((unsigned long)StmpPtr - (unsigned long)pMsg)  ;
				}				
			}
			else{
				break;
			}
		}
		if(wvfound){
			int lent2=0;
			StmpPtr = search_VendorExt_tag(pData ,VENDOR_VERSION2 , tag_len , &lent2);
			if(StmpPtr){
				RX_DEBUG("Rev version2(0x%x) EAP\n",StmpPtr[0]);
			}
		}
	}
#endif

	pData = search_tag(pMsg, TAG_MSG_TYPE, msg_len, &tag_len);
	if (pData == NULL) {
		DEBUG_ERR("Can't find TAG_MSG_TYPE\n");
		return -1;
	}

	DEBUG_PRINT2("Message Type: len=%d, val=0x%x\n", tag_len, pData[0]);

	pSta->last_rx_msg = pMsg;
	pSta->last_rx_msg_size = msg_len;

#ifdef SUPPORT_UPNP
	if ((pSta->used & IS_UPNP_CONTROL_POINT) &&
			(pData[0] != MSG_TYPE_M2 && pData[0] != MSG_TYPE_M4 &&
			pData[0] != MSG_TYPE_M6 && pData[0] != MSG_TYPE_M8 &&
			pData[0] != MSG_TYPE_NACK && pData[0] != MSG_TYPE_M2D)) {
		RX_DEBUG("Invalid Message Type [%d]! for UPnP\n", pData[0]);
		return -1;		
	}

	if ((pSta->used & IS_UPNP_CONTROL_POINT) && (pData[0] == MSG_TYPE_NACK)) {
		RX_DEBUG("\n>>Receive WSC NACK from UPnP\n");
		return CONFIG_ERR_CANNOT_CONNECT_TO_REG;
	}
#endif

	switch (pData[0]) {
#ifdef SUPPORT_REGISTRAR		
		case MSG_TYPE_M1:
			return msgHandler_M1(pCtx, pSta, pMsg, msg_len);

		case MSG_TYPE_M3:
			return msgHandler_M3(pCtx, pSta, pMsg, msg_len);
		
		case MSG_TYPE_M5:
			return msgHandler_M5(pCtx, pSta, pMsg, msg_len);

		case MSG_TYPE_M7:
			return msgHandler_M7(pCtx, pSta, pMsg, msg_len);
#endif // SUPPORT_REGISTRAR

#ifdef SUPPORT_ENROLLEE
		case MSG_TYPE_M2:
		case MSG_TYPE_M2D:
		{
			if (pData[0] == MSG_TYPE_M2) {
				WSC_pthread_mutex_lock(&pCtx->RegMutex);

				if (pCtx->registration_on >= 1 && pCtx->sta_invoke_reg != pSta) {
					RX_DEBUG("Registration protocol is already in progress; ignore M2\n");
					/*
						Reason code 5 : Disassociated because AP is unable to 
						handle all currently associated stations
					*/ 
					if (!(pSta->used & IS_UPNP_CONTROL_POINT) && pCtx->is_ap) {
						IssueDisconnect(pCtx->wlan_interface_name, pSta->addr, 5);
						RX_DEBUG("IssueDisconnect\n\n");
						reset_sta(pCtx, pSta, 1);
					}
					WSC_pthread_mutex_unlock(&pCtx->RegMutex);
					return CONFIG_ERR_DEV_BUSY;
				}
				else { //still possible for proxy ?
					if (pCtx->role != ENROLLEE) {
						pCtx->role = ENROLLEE;
						DEBUG_PRINT("Change role to Enrollee\n");
					}
					pCtx->registration_on = 1;
					pCtx->sta_invoke_reg = pSta;

					DEBUG_PRINT2("set registration_on to %d\n", pCtx->registration_on);

					
					if (pCtx->pb_pressed) {
						strcpy(pCtx->pin_code, "00000000");
						RX_DEBUG("set pCtx->pin_code = 00000000 due to PBC\n");
					}
				}
				WSC_pthread_mutex_unlock(&pCtx->RegMutex);
			}
			ret = msgHandler_M2(pCtx, pSta, pMsg, msg_len, (int)pData[0]);
			return ret;
		}
			
		case MSG_TYPE_M4:
			return msgHandler_M4(pCtx, pSta, pMsg, msg_len);
			
		case MSG_TYPE_M6:
			return msgHandler_M6(pCtx, pSta, pMsg, msg_len);
			
		case MSG_TYPE_M8:
			return msgHandler_M8(pCtx, pSta, pMsg, msg_len);			
#endif // SUPPORT_ENROLLEE

		default:
			DEBUG_ERR("Invalid Message Type [%d]!\n", pData[0]);
	}

	return 0;		
}		


#ifdef SUPPORT_UPNP
int pktHandler_upnp_select_msg(CTX_Tp pCtx, STA_CTX_Tp pSta, struct WSC_packet *packet)
{
	unsigned char *pData;
	int tag_len, len;
	unsigned char selectedReg;
	unsigned short passid=0, method=0;
	unsigned char tmpbuf[256];
#ifdef WPS2DOTX
	unsigned char tmpMac[6+1];	
#endif
	DBFENTER;
	
	_DEBUG_PRINT("\n<< Receive UPnP SetSelectedRegistrar msg>>\n\n");

	if (pCtx->registration_on >= 1 && pCtx->sta_invoke_reg != pSta) {
		DEBUG_PRINT("%s %d Registration protocol is already in progress; abort  UPnP SetSelectedRegistrar msg!\n", __FUNCTION__, __LINE__);
		return -1;
	}
		
	if (!( pCtx->is_ap && 
		   (pCtx->role == REGISTRAR || pCtx->role == PROXY) 
		  // &&   (pCtx->original_role != ENROLLEE)//For fix issue: unconfigured state can not pass DTM-case 651
		  )) 
	{
		DEBUG_PRINT("\n<<!!! Unable to set UPnP SetSelectedRegistrar flag>>\n");
		RX_DEBUG("pCtx->is_ap =%d \n",pCtx->is_ap);
		RX_DEBUG("pCtx->role =%d ; PROXY=0, ENROLLEE=1, REGISTRAR=2\n",pCtx->role);
		RX_DEBUG("pCtx->original_role =%d \n",pCtx->original_role);		
					
		return -1;
	}
			
	pData = check_tag(pCtx, packet->rx_buffer, packet->rx_size, TAG_VERSION, 1, "Version", TYPE_BYTE, &tag_len);
	if (pData == NULL){
		RX_DEBUG(" TAG_VERSION == NULL, return\n");
		return -1;
	}
	
	pData = check_tag(pCtx, packet->rx_buffer, packet->rx_size, TAG_SELECTED_REGITRAR, 1, "SelectedRegistrar", TYPE_BYTE, &tag_len);
	if (pData == NULL){
		RX_DEBUG(" TAG_SELECTED_REGITRAR == NULL, return\n");
		return -1;
	}

	selectedReg = pData[0];

#ifdef WPS2DOTX
	if (pCtx->current_wps_version == WPS_VERSION_V2) {

		unsigned char *StmpPtr=NULL ;
		//	int msg_len_tmp = packet->rx_size;
		int wvfound = 0;
		int lent2=0;
		
		// just for check if have some one include TAG_AUTHORIZED_MACs in EAP
		//pData = check_tag(pCtx, packet->rx_buffer, packet->rx_size, TAG_AUTHORIZED_MACs,
		//ETHER_ADDRLEN,"AuthorizedMACs", TYPE_BIN, &tag_len);
		//if(pData)
		//	RX_DEBUG("found TAG_AUTHORIZED_MACs at upnp SelReg packet\n");


		//pData = check_tag(pCtx, packet->rx_buffer, packet->rx_size, TAG_VENDOR_EXT,
		//		MAX_VENEXT_LEN,"VENDOR_EXT", TYPE_BYTE, &tag_len);

		pData = search_tag(packet->rx_buffer, TAG_VENDOR_EXT,  packet->rx_size, &tag_len);
		if (pData == NULL) {
			RX_DEBUG("\n can't find TAG_VENDOR_EXT!\n");
		}
		
		// just for debug
		//debug_out("message form ER:",packet->rx_buffer,packet->rx_size );

		if (pData != NULL) {
			if(!memcmp(pData , WSC_VENDOR_OUI ,3 ))
			{
				//RX_DEBUG("WFA vendor OUI!!\n");
				debug_out("verdor ext:",pData,tag_len );				
				wvfound = 1 ;

			}				
		}
		if(wvfound){	/* include VENDOR_EXT tag */ 

			StmpPtr = search_VendorExt_tag(pData ,VENDOR_VERSION2 , tag_len , &lent2);
			if(StmpPtr){
				RX_DEBUG("Rev version2(0x%x) EAP\n",StmpPtr[0]);
			}
			
			if(selectedReg){
				StmpPtr = search_VendorExt_tag(pData ,VENDOR_AUTHMAC , tag_len , &lent2);
				if(StmpPtr){
					RX_DEBUG("Include AuthMAC[%x:%x:%x:%x:%x:%x] from ER\n",
						StmpPtr[0],StmpPtr[1],StmpPtr[2],StmpPtr[3],StmpPtr[4],StmpPtr[5]);				
					registrar_add_authorized_mac(pCtx , StmpPtr);
				}else{
					/*no assigned ; add wildcard addr*/ 
					RX_DEBUG("no assigned AuthMac; add wildcard addr\n");				
					registrar_add_authorized_mac(pCtx , BroadCastMac);
				}		
			}
		}else{
			if(selectedReg){
				/*no assigned ; add wildcard addr*/ 
				RX_DEBUG("no assigned AuthMac; add wildcard addr\n");							
				registrar_add_authorized_mac(pCtx , BroadCastMac);
			}
		}
	}		
#endif




	if (selectedReg) {
		pData = check_tag(pCtx, packet->rx_buffer, packet->rx_size, TAG_DEVICE_PASSWORD_ID, 2, "DevicePasswordID", TYPE_WORD, &tag_len);
		if (pData == NULL){
			RX_DEBUG(" TAG_DEVICE_PASSWORD_ID == NULL, return\n");
			return -1;
		}
		
		memcpy(&passid, pData, 2);

		pData = check_tag(pCtx, packet->rx_buffer, packet->rx_size, TAG_SEL_REG_CONFIG_METHODS, 2, 
				"SelectedRegistrarConfigMethod", TYPE_WORD, &tag_len);
		if (pData == NULL){
			RX_DEBUG(" TAG_SEL_REG_CONFIG_METHODS == NULL, return\n");
			return -1;
		}
		memcpy(&method, pData, 2);

		pCtx->setSelectedRegTimeout = SETSELREG_WALK_TIME;
		memcpy(pCtx->SetSelectedRegistrar_ip, packet->IP, IP_ADDRLEN);

		if (wlioctl_set_led(pCtx->wlan_interface_name, LED_WSC_START) < 0) {
			DEBUG_ERR("issue wlan ioctl set_led error!\n");
		}
		
		if (pCtx->is_ap) 
			pCtx->wps_triggered = 1; 

#ifdef MUL_PBC_DETECTTION		
		if (pCtx->SessionOverlapTimeout) {
			DEBUG_PRINT("Clear session overlapping stuff!\n");
			pCtx->SessionOverlapTimeout = 0;
		}
#endif		
		if(pCtx->pb_timeout || pCtx->pb_pressed) {
			RX_DEBUG("Clear PBC stuff!\n");
			pCtx->pb_pressed = 0;
			pCtx->pb_timeout = 0;
			pCtx->pin_assigned = 0;
		}
		if (pCtx->pin_timeout) {
			DEBUG_PRINT("Clear PIN stuff!\n");
			pCtx->pin_timeout = 0; //clear PIN timeout
			pCtx->pin_assigned = 0;
		}
	}
	else{
		pCtx->setSelectedRegTimeout = 0;
#ifdef WPS2DOTX
		if (pCtx->current_wps_version == WPS_VERSION_V2)
			registrar_remove_authorized_mac(pCtx , tmpMac);		
#endif
	}

	if (pCtx->use_ie) {
		len = build_beacon_ie(pCtx, selectedReg, passid, method, tmpbuf);

	if ((pCtx->encrypt_type == ENCRYPT_WEP) && (pCtx->current_wps_version == WPS_VERSION_V1)) // add provisioning service ie
	{
		len += build_provisioning_service_ie((unsigned char *)(tmpbuf+len));
	}

#ifdef FOR_DUAL_BAND
		if(!pCtx->wlan0_wsc_disabled)
#endif
		{
			if (wlioctl_set_wsc_ie(pCtx->wlan_interface_name, tmpbuf, len,DOT11_EVENT_WSC_SET_IE, SET_IE_FLAG_BEACON) < 0)
				return -1;					
		}

#ifdef FOR_DUAL_BAND
		if(!pCtx->wlan1_wsc_disabled){
			if (wlioctl_set_wsc_ie(pCtx->wlan_interface_name2, tmpbuf, len, DOT11_EVENT_WSC_SET_IE, SET_IE_FLAG_BEACON) < 0)
				return -1;
		}
#endif //FOR_DUAL_BAND

		len = build_probe_rsp_ie(pCtx, selectedReg, passid, method, tmpbuf);

		if ((pCtx->encrypt_type == ENCRYPT_WEP) && (pCtx->current_wps_version == WPS_VERSION_V1)) // add provisioning service ie
		{
			len += build_provisioning_service_ie((unsigned char *)(tmpbuf+len));
		}
		
		if (len > MAX_WSC_IE_LEN) {
			DEBUG_ERR("Length of IE exceeds %d\n", MAX_WSC_IE_LEN);
			return -1;
		}

#ifdef FOR_DUAL_BAND
		if(!pCtx->wlan0_wsc_disabled)
#endif
		{
			if (wlioctl_set_wsc_ie(pCtx->wlan_interface_name, tmpbuf, len, 
					DOT11_EVENT_WSC_SET_IE, SET_IE_FLAG_PROBE_RSP) < 0)
				return -1;
		}
		
#ifdef FOR_DUAL_BAND
		if(!pCtx->wlan1_wsc_disabled){
			if (wlioctl_set_wsc_ie(pCtx->wlan_interface_name2, tmpbuf, len, 
					DOT11_EVENT_WSC_SET_IE, SET_IE_FLAG_PROBE_RSP) < 0)
				return -1;
		}
#endif //FOR_DUAL_BAND
	}

#ifdef BLOCKED_ROGUE_STA
	if (pCtx->is_ap && pCtx->blocked_expired_time)
		disassociate_blocked_list(pCtx);
#endif

	return 0;	
}
#endif // SUPPORT_UPNP

