#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "simplecfg_upnp.h"
#include "wsc.h"

static const char *known_service_types[] =
{
	"upnp:rootdevice",
	"urn:schemas-wifialliance-org:device:WFADevice:",
	"urn:schemas-wifialliance-org:service:WFAWLANConfig:",
	0
};

/* The amount of time before advertisements
   will expire */
static const int default_advr_expire = 1800;

WSC_FunPtr WSCCallBack = NULL;
char *user_priv_data = NULL;
unsigned char WSCCallBack_registered = 0;
OpMode upnp_op_mode = WSC_AP_MODE;
OpStatus upnp_op_status = WSC_INITIAL;

static int gen_simplecfg_xml(char *IP, int port, char *docpath, char *outfile, struct WSC_profile *profile)
{
	FILE *fpo;
	char *patho=NULL;
	char *buffo=NULL;
	char uuid[2*UPNP_UUID_LEN+4];

	patho = (char *) malloc(256);
	if (patho == NULL)
		return WSC_UPNP_FAIL;
	buffo = (char *) malloc(256);
	if (buffo == NULL) {
		free(patho);
		return WSC_UPNP_FAIL;
	}

	sprintf(patho, "%s%s", docpath, outfile);
	if ((fpo = fopen(patho,"w")) == NULL) {
		free(buffo);
		free(patho);
		DEBUG_ERR("output file can not open\n");
		return WSC_UPNP_FAIL;
	}
	memset(buffo, 0, 256);

	fputs("<?xml version=\"1.0\"?>\n" , fpo);
	fputs("<root xmlns=\"urn:schemas-upnp-org:device-1-0\">\n" , fpo);
		fputs("\t<specVersion>\n" , fpo);
			fputs("\t\t<major>1</major>\n" , fpo);
			fputs("\t\t<minor>0</minor>\n" , fpo);
		fputs("\t</specVersion>\n" , fpo);
		sprintf(buffo, "\t<URLBase>http://%s:%u</URLBase>\n", IP, port);
		fputs(buffo, fpo); memset(buffo, 0, 256);
		fputs("\t<device>\n" , fpo);
			fputs("\t\t<deviceType>urn:schemas-wifialliance-org:device:WFADevice:1</deviceType>\n" , fpo);
			if (profile->device_name == NULL)
				fputs("\t\t<friendlyName>RTL8186 WFA Device</friendlyName>\n", fpo);
			else {
				sprintf(buffo, "\t\t<friendlyName>%s</friendlyName>\n", profile->device_name);
				fputs(buffo, fpo); memset(buffo, 0, 256);
			}
			if (profile->manufacturer == NULL)
				fputs("\t\t<manufacturer>Realtek Semiconductor</manufacturer>\n", fpo);
			else {
				sprintf(buffo, "\t\t<manufacturer>%s</manufacturer>\n", profile->manufacturer);
				fputs(buffo, fpo); memset(buffo, 0, 256);
			}
			if (profile->manufacturerURL == NULL)
				fputs("\t\t<manufacturerURL>http://www.realtek.com.tw</manufacturerURL>\n" , fpo);
			else {
				sprintf(buffo, "\t\t<manufacturerURL>%s</manufacturerURL>\n", profile->manufacturerURL);
				fputs(buffo, fpo); memset(buffo, 0, 256);
			}
			if (profile->modelDescription == NULL)
				fputs("\t\t<modelDescription>Simple Config UPnP Proxy</modelDescription>\n" , fpo);
			else {
				sprintf(buffo, "\t\t<modelDescription>%s</modelDescription>\n", profile->modelDescription);
				fputs(buffo, fpo); memset(buffo, 0, 256);
			}
			if (profile->model_name == NULL)
				fputs("\t\t<modelName>Simple Config UPnP Proxy Version 1.0</modelName>\n", fpo);
			else {
				sprintf(buffo, "\t\t<modelName>%s</modelName>\n", profile->model_name);
				fputs(buffo, fpo); memset(buffo, 0, 256);
			}
			if (profile->model_num == NULL)
				fputs("\t\t<modelNumber>RTL8186</modelNumber>\n", fpo);
			else {
				sprintf(buffo, "\t\t<modelNumber>%s</modelNumber>\n", profile->model_num);
				fputs(buffo, fpo); memset(buffo, 0, 256);
			}
			if (profile->modelURL == NULL)
				fputs("\t\t<modelURL>http://www.realtek.com.tw</modelURL>\n" , fpo);
			else {
				sprintf(buffo, "\t\t<modelURL>%s</modelURL>\n", profile->modelURL);
				fputs(buffo, fpo); memset(buffo, 0, 256);
			}
			if (profile->serial_num == NULL)
				fputs("\t\t<serialNumber>12345678</serialNumber>\n" , fpo);
			else {
				sprintf(buffo, "\t\t<serialNumber>%s</serialNumber>\n", profile->serial_num);
				fputs(buffo, fpo); memset(buffo, 0, 256);
			}

			sprintf(uuid, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
				profile->uuid[0],
				profile->uuid[1],profile->uuid[2],profile->uuid[3],profile->uuid[4],profile->uuid[5],
				profile->uuid[6],profile->uuid[7],profile->uuid[8],profile->uuid[9],profile->uuid[10],
				profile->uuid[11],profile->uuid[12],profile->uuid[13],profile->uuid[14],profile->uuid[15]);
			sprintf(buffo, "\t\t<UDN>uuid:%s</UDN>\n", uuid);
			fputs(buffo, fpo);
			memset(buffo, 0, 256);

			if (profile->UPC == NULL)
				fputs("\t\t<UPC>112233445566</UPC>\n" , fpo); //must be 12 digit
			else {
				sprintf(buffo, "\t\t<UPC>%s</UPC>\n", profile->UPC);
				fputs(buffo, fpo); memset(buffo, 0, 256);
			}
#if 0
			fputs("\t\t<iconList>\n" , fpo);
				fputs("\t\t\t<icon>\n" , fpo);
					fputs("\t\t\t\t<mimetype>image/gif</mimetype>\n" , fpo);
					fputs("\t\t\t\t<width>118</width>\n" , fpo);
					fputs("\t\t\t\t<height>119</height>\n" , fpo);
					fputs("\t\t\t\t<depth>8</depth>\n" , fpo);
					fputs("\t\t\t\t<url>/ligd.gif</url>\n" , fpo);
				fputs("\t\t\t</icon>\n" , fpo);
			fputs("\t\t</iconList>\n" , fpo);
#endif
			fputs("\t\t<serviceList>\n" , fpo);
				fputs("\t\t\t<service>\n" , fpo);
					fputs("\t\t\t\t<serviceType>urn:schemas-wifialliance-org:service:WFAWLANConfig:1</serviceType>\n" , fpo);
					fputs("\t\t\t\t<serviceId>urn:wifialliance-org:serviceId:WFAWLANConfig1</serviceId>\n" , fpo);
					fputs("\t\t\t\t<SCPDURL>/simplecfgservice.xml</SCPDURL>\n" , fpo);
					fputs("\t\t\t\t<controlURL>/upnp/control/WFAWLANConfig1</controlURL>\n" , fpo);
					fputs("\t\t\t\t<eventSubURL>/upnp/event/WFAWLANConfig1</eventSubURL>\n" , fpo);
				fputs("\t\t\t</service>\n" , fpo);
			fputs("\t\t</serviceList>\n" , fpo);
#if 0
			fputs("\t\t<deviceList>\n" , fpo);
			fputs("\t\t</deviceList>\n" , fpo);
			sprintf(buffo, "\t\t<presentationURL>http://%s/</presentationURL>\n", IP);
			fputs(buffo, fpo);
#endif
		fputs("\t</device>\n" , fpo);
	fputs("</root>\n" , fpo);

	fclose(fpo);
	free(buffo);
	free(patho);

	return WSC_UPNP_SUCCESS;
}

static void WFAGetDeviceInfo(struct upnphttp * h)
{
	char *result_str=NULL;
	struct WSC_packet *packet=NULL;
	char *body=NULL;
	unsigned char *NewDeviceInfo_Base64=NULL;
	int NewDeviceInfo_Base64Length=0, TotalLen=0;

	if (upnp_op_status == WSC_LOCKED) {
		_DEBUG_PRINT("Status : locked\n");
		return;
	}

	packet = (struct WSC_packet *)malloc(sizeof(struct WSC_packet));
	if (packet == NULL) {
		DEBUG_ERR("Not enough memory!\n");
		goto error_handle;
	}
	memset(packet, 0, sizeof(struct WSC_packet));
	packet->EventType = WSC_NOT_PROXY;
	packet->EventID = WSC_GETDEVINFO;
	memcpy(packet->IP, h->IP, strlen(h->IP));
	packet->IP[strlen(h->IP)] = '\0';

	_DEBUG_PRINT("WFAGetDeviceInfo\n");
	if (WSCCallBack(packet, user_priv_data) != WSC_UPNP_SUCCESS) {
		DEBUG_ERR("WSCCallBack Fail!\n");
		goto error_handle;
	}

	if ((packet->tx_size > MAX_MSG_LEN) || (packet->tx_size <= 0)) {
		DEBUG_ERR("Unreasonable tx length!\n");
		goto error_handle;
	}

#ifdef DEBUG
	debug_out("M1", packet->tx_buffer, packet->tx_size);
#endif
	NewDeviceInfo_Base64Length = ILibBase64Encode(packet->tx_buffer, packet->tx_size, &NewDeviceInfo_Base64);
	body = (char *)malloc(32 + NewDeviceInfo_Base64Length);
	if (body == NULL) {
		DEBUG_ERR("Not enough memory!\n");
		goto error_handle;
	}
	sprintf(body,"<NewDeviceInfo>%s</NewDeviceInfo>", NewDeviceInfo_Base64);

	TotalLen = strlen("<u:%sResponse xmlns:u=\"%s\">\n%s\n</u:%sResponse>") +
		strlen("urn:schemas-wifialliance-org:WFAWLANConfig:1") +
		2 * strlen("GetDeviceInfo") + (32 + NewDeviceInfo_Base64Length);
	result_str = (char *)malloc(TotalLen);
	if (result_str == NULL) {
		DEBUG_ERR("Not enough memory!\n");
		goto error_handle;
	}
	memset(result_str, 0, TotalLen);

      	TotalLen = sprintf(result_str, "<u:%sResponse xmlns:u=\"%s\">\n%s\n</u:%sResponse>", "GetDeviceInfo",
               "urn:schemas-wifialliance-org:WFAWLANConfig:1",
                body,
                "GetDeviceInfo");

	BuildSendAndCloseSoapResp(h, result_str, TotalLen);

error_handle:
	if (packet)
		free(packet);
	if (body)
		free(body);
	if (NewDeviceInfo_Base64)
		free(NewDeviceInfo_Base64);
	if (result_str)
		free(result_str);

       return;
}

static void SendMsgToSM_Dir_In(struct upnphttp * h,
							WSC_EventID eid,
							char *tag, char *InMsgName)
{
	char *result_str=NULL;
	char *p_NewInMessage=NULL;
	struct WSC_packet *packet=NULL;
	int p_NewInMessageLength=0;
	unsigned char* _NewInMessage=NULL;
	int _NewInMessageLength=0;
	struct NameValueParserData data;
	int TotalLen=0;

	memset(&data, 0, sizeof(struct NameValueParserData));
	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);

	p_NewInMessage = GetValueFromNameValueList(&data, InMsgName);
	if (p_NewInMessage == NULL) {
		DEBUG_ERR("%s : No %s!\n", tag, InMsgName);
		ClearNameValueList(&data);
		return;
	}
	p_NewInMessageLength = strlen(p_NewInMessage);
	_NewInMessageLength = ILibBase64Decode(p_NewInMessage, p_NewInMessageLength,&_NewInMessage);
	if (_NewInMessageLength > MAX_MSG_LEN || _NewInMessageLength <= 0 || _NewInMessage == NULL) {
		DEBUG_ERR("Unreasonable rx length!\n");
		goto error_handle;
	}

	packet = (struct WSC_packet *)malloc(sizeof(struct WSC_packet));
	if (packet == NULL) {
		DEBUG_ERR("Not enough memory!\n");
		goto error_handle;
	}
	memset(packet, 0, sizeof(struct WSC_packet));
	packet->EventID = eid;

	memcpy(packet->IP, h->IP, strlen(h->IP));
	packet->IP[strlen(h->IP)] = '\0';
	_DEBUG_PRINT("Receive Upnp message from IP : %s\n", packet->IP);

	packet->rx_size = _NewInMessageLength;
	memcpy(packet->rx_buffer, _NewInMessage, _NewInMessageLength);
#ifdef DEBUG
	debug_out(tag, packet->rx_buffer, packet->rx_size);
#endif

	if (WSCCallBack(packet, user_priv_data) != WSC_UPNP_SUCCESS) {
		DEBUG_ERR("WSCCallBack Fail!\n");
		goto error_handle;
	}

	TotalLen = strlen("<u:%sResponse xmlns:u=\"%s\">\n%s\n</u:%sResponse>") +
		strlen("urn:schemas-wifialliance-org:WFAWLANConfig:1") +
		2 * strlen(tag) + 100;
	result_str = (char *)malloc(TotalLen);
	if (result_str == NULL) {
		DEBUG_ERR("Not enough memory!\n");
		goto error_handle;
	}
	memset(result_str, 0, TotalLen);

	TotalLen = sprintf(result_str, "<u:%sResponse xmlns:u=\"%s\"></u:%sResponse>", tag,
                "urn:schemas-wifialliance-org:WFAWLANConfig:1",
      	          tag);

	BuildSendAndCloseSoapResp(h, result_str, TotalLen);

error_handle:
	ClearNameValueList(&data);

	if (result_str)
		free(result_str);
	//if (p_NewInMessage)
		//free(p_NewInMessage);
	if (_NewInMessage)
		free(_NewInMessage);
	if (packet)
		free(packet);
}

static void SendMsgToSM_Dir_InOut(struct upnphttp * h,
							WSC_EventID eid,
							char *tag, char *InMsgName, char *OutMsgName)
{
	char *result_str=NULL;
	char *body=NULL;
	struct WSC_packet *packet=NULL;
	unsigned char* NewOutMessage_Base64=NULL;
	int NewOutMessage_Base64Length=0, TotalLen=0, body_len=0;
	char *p_NewInMessage=NULL;
	int p_NewInMessageLength=0;
	unsigned char* _NewInMessage=NULL;
	int _NewInMessageLength=0;
	struct NameValueParserData data;

	memset(&data, 0, sizeof(struct NameValueParserData));
	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);

	p_NewInMessage = GetValueFromNameValueList(&data, InMsgName);
	if (p_NewInMessage == NULL) {
		DEBUG_ERR("%s : No %s!\n", tag, InMsgName);
		ClearNameValueList(&data);
		return;
	}
	p_NewInMessageLength = strlen(p_NewInMessage);
	_NewInMessageLength = ILibBase64Decode(p_NewInMessage, p_NewInMessageLength, &_NewInMessage);
	if (_NewInMessageLength > MAX_MSG_LEN || _NewInMessageLength <= 0 || _NewInMessage == NULL) {
		DEBUG_ERR("Unreasonable rx length!\n");
		goto error_handle;
	}

	packet = (struct WSC_packet *)malloc(sizeof(struct WSC_packet));
	if (packet == NULL) {
		DEBUG_ERR("Not enough memory!\n");
		goto error_handle;
	}
	memset(packet, 0, sizeof(struct WSC_packet));

	packet->EventID = eid;

	memcpy(packet->IP, h->IP, strlen(h->IP));
	packet->IP[strlen(h->IP)] = '\0';
	_DEBUG_PRINT("Receive Upnp message from IP : %s\n", packet->IP);

	packet->rx_size = _NewInMessageLength;
	memcpy(packet->rx_buffer, _NewInMessage, _NewInMessageLength);
#ifdef DEBUG
	debug_out(tag, packet->rx_buffer, packet->rx_size);
#endif

	if (WSCCallBack(packet, user_priv_data) != WSC_UPNP_SUCCESS) {
		DEBUG_ERR("WSCCallBack Fail!\n");
		goto error_handle;
	}

	if ((packet->tx_size > MAX_MSG_LEN) || (packet->tx_size < 0)) {
		DEBUG_ERR("Unreasonable tx length!\n");
		goto error_handle;
	}

#ifdef DEBUG
	debug_out(tag, packet->tx_buffer, packet->tx_size);
#endif
	NewOutMessage_Base64Length = ILibBase64Encode(packet->tx_buffer, packet->tx_size, &NewOutMessage_Base64);
	body_len = 2*strlen(OutMsgName) + 5 + NewOutMessage_Base64Length;
	body = (char *)malloc(body_len);
	if (body == NULL) {
		DEBUG_ERR("Not enough memory!\n");
		goto error_handle;
	}

	if (NewOutMessage_Base64)
		sprintf(body,"<%s>%s</%s>", OutMsgName, NewOutMessage_Base64, OutMsgName);
	else
		sprintf(body,"<%s></%s>", OutMsgName, OutMsgName);

	TotalLen = strlen("<u:%sResponse xmlns:u=\"%s\">\n%s\n</u:%sResponse>") +
		strlen("urn:schemas-wifialliance-org:WFAWLANConfig:1") +
		2 * strlen(tag) + (body_len) + 100;
	result_str = (char *)malloc(TotalLen);
	if (result_str == NULL) {
		DEBUG_ERR("Not enough memory!\n");
		goto error_handle;
	}
	memset(result_str, 0, TotalLen);

	TotalLen = sprintf(result_str, "<u:%sResponse xmlns:u=\"%s\">\n%s\n</u:%sResponse>", tag,
                "urn:schemas-wifialliance-org:WFAWLANConfig:1",
                body,
      	          tag);

	BuildSendAndCloseSoapResp(h, result_str, TotalLen);

error_handle:
	ClearNameValueList(&data);

	if (body)
		free(body);
	if (NewOutMessage_Base64)
		free(NewOutMessage_Base64);
	if (result_str)
		free(result_str);
	if (_NewInMessage)
		free(_NewInMessage);
	if (packet)
		free(packet);
	//if (p_NewInMessage)
		//free(p_NewInMessage);
}

static void WFAPutMessage(struct upnphttp * h)
{
	if (upnp_op_status == WSC_LOCKED)
		_DEBUG_PRINT("Status : locked\n");
	else
		SendMsgToSM_Dir_InOut(h,
			WSC_M2M4M6M8, "PutMessage", "NewInMessage", "NewOutMessage");
}

static void WFAPutWLANResponse(struct upnphttp * h)
{
	struct WSC_packet *packet=NULL;
	char *p_NewInMessage=NULL;
	int p_NewInMessageLength=0;
	unsigned char* _NewInMessage=NULL;
	int _NewInMessageLength=0;
	unsigned char EType=0;
	struct NameValueParserData data;
	int TotalLen=0;
	char *result_str=NULL;

	if (upnp_op_mode != WSC_AP_MODE) {
		DEBUG_ERR("Not AP mode!\n");
		return;
	}

	memset(&data, 0, sizeof(struct NameValueParserData));
	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);

	p_NewInMessage = GetValueFromNameValueList(&data, "NewMessage");
	if (p_NewInMessage == NULL) {
		DEBUG_ERR("No NewMessage!\n");
		goto error_handle;
	}

	p_NewInMessageLength = strlen(p_NewInMessage);
	_NewInMessageLength = ILibBase64Decode(p_NewInMessage, p_NewInMessageLength,&_NewInMessage);
	if (_NewInMessageLength > MAX_MSG_LEN || _NewInMessageLength <= 0 || _NewInMessage == NULL) {
		DEBUG_ERR("Unreasonable rx length!\n");
		goto error_handle;
	}

	packet = (struct WSC_packet *)malloc(sizeof(struct WSC_packet));
	if (packet == NULL) {
		DEBUG_ERR("Not enough memory!\n");
		goto error_handle;
	}
	memset(packet, 0, sizeof(struct WSC_packet));

	memcpy(packet->rx_buffer, _NewInMessage, _NewInMessageLength);
	packet->rx_size = _NewInMessageLength;

	p_NewInMessage = GetValueFromNameValueList(&data, "NewWLANEventType");
	if (p_NewInMessage != NULL)
		EType = atoi(p_NewInMessage);

	if (EType == WSC_8021XEAP_FRAME) {
#ifdef DEBUG
		debug_out("WFAPutWLANResponse : forward message to enrollee", packet->rx_buffer, packet->rx_size);
#endif
		packet->EventType = WSC_8021XEAP_FRAME;
	}
	else {
		DEBUG_ERR("Unknown event type!\n");
		goto error_handle;
	}

	p_NewInMessage = GetValueFromNameValueList(&data, "NewWLANEventMAC");
	if (p_NewInMessage == NULL) {
		DEBUG_ERR("No NewWLANEventMAC!\n");
		goto error_handle;
	}
	memcpy(packet->EventMac, p_NewInMessage, MACLEN);
	_DEBUG_PRINT("EventMac : %s\n", packet->EventMac);

	packet->EventID = WSC_PUTWLANRESPONSE;

	memcpy(packet->IP, h->IP, strlen(h->IP));
	packet->IP[strlen(h->IP)] = '\0';
	_DEBUG_PRINT("Receive Upnp message from IP : %s\n", packet->IP);

	if (WSCCallBack(packet, user_priv_data) != WSC_UPNP_SUCCESS) {
		DEBUG_ERR("WSCCallBack Fail!\n");
		goto error_handle;
	}

	TotalLen = strlen("<u:%sResponse xmlns:u=\"%s\">\n%s\n</u:%sResponse>") +
		strlen("urn:schemas-wifialliance-org:WFAWLANConfig:1") +
		2 * strlen("PutWLANResponse") + 100;
	result_str = (char *)malloc(TotalLen);
	if (result_str == NULL) {
		DEBUG_ERR("Not enough memory!\n");
		goto error_handle;
	}
	memset(result_str, 0, TotalLen);

	TotalLen = sprintf(result_str, "<u:%sResponse xmlns:u=\"%s\"></u:%sResponse>", "PutWLANResponse",
                "urn:schemas-wifialliance-org:WFAWLANConfig:1",
      	          "PutWLANResponse");

	BuildSendAndCloseSoapResp(h, result_str, TotalLen);


error_handle:
	ClearNameValueList(&data);

	if(result_str)
		free(result_str);
	if (packet)
		free(packet);
	//if (p_NewInMessage)
		//free(p_NewInMessage);
	if (_NewInMessage)
		free(_NewInMessage);
}

static int WFATxmitWLANEventToRegistra(struct WSC_packet *packet)
{
    	Upnp_Document PropSet=NULL;
	int ret=WSC_UPNP_FAIL, err_code=0;
	unsigned char *WLANEvent=NULL;
	unsigned char *WLANEvent_Base64=NULL;
	int WLANEvent_Base64Length=0;
	unsigned int TotalLen=0;
	CTX_Tp pCtx = (CTX_Tp)user_priv_data;

	if (upnp_op_mode != WSC_AP_MODE) {
		DEBUG_ERR("Not AP mode!\n");
		goto error_handle;
	}

	if (packet == NULL) {
		DEBUG_ERR("No message for WFATxmitWLANEventToRegistra!\n");
		goto error_handle;
	}
	else {
		if ((packet->tx_size > MAX_MSG_LEN) || (packet->tx_size <= 0)) {
			DEBUG_ERR("Unreasonable tx length!\n");
			goto error_handle;
		}
		else
			TotalLen += packet->tx_size;

		if ((packet->EventType == WSC_PROBE_FRAME) || (packet->EventType == WSC_8021XEAP_FRAME))
			TotalLen++;
		else {
			DEBUG_ERR("Unknown event type!\n");
			goto error_handle;
		}

		TotalLen += MACLEN; // Length of Mac address
		WLANEvent = (unsigned char *)malloc(TotalLen);
		if (WLANEvent == NULL) {
			DEBUG_ERR("Not enough memory!\n");
			goto error_handle;
		}

		WLANEvent[0] = packet->EventType;
		sprintf(WLANEvent+1, packet->EventMac);
		memcpy(WLANEvent+18, packet->tx_buffer, packet->tx_size);
#ifdef DEBUG
		if (WLANEvent[0] != WSC_PROBE_FRAME) {
			_DEBUG_PRINT("WLANEventType = %d\n", WLANEvent[0]);
			_DEBUG_PRINT("WLANEventMac = %s\n", packet->EventMac);
			debug_out("Forward WLANEvent to registra", WLANEvent+18, packet->tx_size);
		}
#endif
		WLANEvent_Base64Length = ILibBase64Encode(WLANEvent, TotalLen, &WLANEvent_Base64);
		if (WLANEvent_Base64 == NULL || WLANEvent_Base64Length <= 0) {
			DEBUG_ERR("ILibBase64Encode failed!\n");
			goto error_handle;
		}
		else {
			PropSet = CreatePropertySet();
			if (PropSet == NULL)
				goto error_handle;
			err_code = UpnpAddToPropertySet(PropSet, "WLANEvent", WLANEvent_Base64);
			if (err_code != UPNP_E_SUCCESS) {
				DEBUG_ERR("Error code %d : UpnpAddToPropertySet failed!\n", err_code);
				goto error_handle;
			}

			UpnpSendEventAll(PropSet, &pCtx->upnp_info.subscribe_list);
			ret = WSC_UPNP_SUCCESS;
		}
	}

error_handle:

	if (PropSet)
		UpnpDocument_free(PropSet);
	if (WLANEvent)
		free(WLANEvent);
	if (WLANEvent_Base64)
		free(WLANEvent_Base64);

    	return ret;
}

static int WFATxmitStatus(struct WSC_packet *packet)
{
	int ret=WSC_UPNP_FAIL;
	Upnp_Document PropSet=NULL;
	char *pstatus=NULL;
	int status=0;
	CTX_Tp pCtx = (CTX_Tp)user_priv_data;

	if (packet == NULL) {
		DEBUG_ERR("No message for WFATxmitStatus!\n");
		goto error_handle;
	}
	else {
		if ((packet->tx_size > MAX_MSG_LEN) || (packet->tx_size <= 0)) {
			DEBUG_ERR("Unreasonable tx length!\n");
			goto error_handle;
		}

		pstatus = (char *)malloc(packet->tx_size);
		if (pstatus == NULL) {
			DEBUG_ERR("Not enough memory!\n");
			goto error_handle;
		}
		memcpy(pstatus, packet->tx_buffer, packet->tx_size);
		status = atoi(pstatus);
	}

	switch (packet->EventID)
	{
		case WSC_AP_STATUS:
			if (upnp_op_mode != WSC_AP_MODE) {
				DEBUG_ERR("Not AP mode!\n");
				goto error_handle;
			}
			else {
				PropSet = CreatePropertySet();
				if (PropSet == NULL)
					goto error_handle;
				if (status == WSC_CONFIG_CHANGE)
					UpnpAddToPropertySet(PropSet, "APStatus", "1");
				else if (status == WSC_LOCKED)
					UpnpAddToPropertySet(PropSet, "APStatus", "2");
				else {
					DEBUG_ERR("Unknown status!\n");
					goto error_handle;
				}
			}
			break;
		case WSC_STA_STATUS:
			if (upnp_op_mode != WSC_STA_MODE) {
				DEBUG_ERR("Not STA mode!\n");
				goto error_handle;
			}
			else {
				PropSet = CreatePropertySet();
				if (PropSet == NULL)
					goto error_handle;
				if (status == WSC_CONFIG_CHANGE)
					UpnpAddToPropertySet(PropSet, "STAStatus", "1");
				else if (status == WSC_LOCKED)
					UpnpAddToPropertySet(PropSet, "STAStatus", "2");
				else {
					DEBUG_ERR("Unknown status!\n");
					goto error_handle;
				}
			}
			break;
		default:
			_DEBUG_PRINT("Unknown EventID in WFATxmitStatus!\n");
			goto error_handle;
	}

	UpnpSendEventAll(PropSet, &pCtx->upnp_info.subscribe_list);
	upnp_op_status = status;
	ret = WSC_UPNP_SUCCESS;

error_handle:

	if (PropSet)
		UpnpDocument_free(PropSet);

	if (pstatus)
		free(pstatus);

	return ret;
}

int WSCUpnpTxmit(struct WSC_packet *packet)
{
	switch (packet->EventID)
	{
		case WSC_PUTWLANREQUEST:
			return (WFATxmitWLANEventToRegistra(packet));
		case WSC_AP_STATUS:
		case WSC_STA_STATUS:
			return (WFATxmitStatus(packet));

		default:
			_DEBUG_PRINT("Unknown EventID in WSCUpnpTxmit!\n");
			return WSC_UPNP_FAIL;
	}
}

int WSCRegisterCallBackFunc(WSC_FunPtr Fun, void *Cookie)
{
	if (WSCCallBack_registered >= 1) {
		DEBUG_ERR("CallBack already registered!\n");
		return WSC_UPNP_FAIL;
	}

	if (Fun == NULL) {
		DEBUG_ERR("No Function handler!\n");
		return WSC_UPNP_FAIL;
	}
	else {
		WSCCallBack = Fun;
		WSCCallBack_registered = 1;
		user_priv_data = (char *)Cookie;
		_DEBUG_PRINT("WSCRegisterCallBackFunc successfully!\n");
	}

	return WSC_UPNP_SUCCESS;
}

static char *genWSCRootDesc(int *len)
{
	CTX_Tp pCtx = (CTX_Tp)user_priv_data;
	char *buf=NULL;

	buf = (unsigned char*)malloc(pCtx->upnp_info.rootXML_len);
	if (buf == NULL) {
		*len = 0;
		return NULL;
	}
	*len = pCtx->upnp_info.rootXML_len;
	memcpy(buf, pCtx->upnp_info.rootXML, *len);
	return buf;
}

static char *genWSCServiceDesc(int *len)
{
	CTX_Tp pCtx = (CTX_Tp)user_priv_data;
	char *buf=NULL;

	buf = (unsigned char*)malloc(pCtx->upnp_info.serviceXML_len);
	if (buf == NULL) {
		*len = 0;
		return NULL;
	}
	*len = pCtx->upnp_info.serviceXML_len;
	memcpy(buf, pCtx->upnp_info.serviceXML, *len);
	return buf;
}

static void WFASetSelectedRegistrar(struct upnphttp * h)
{
	SendMsgToSM_Dir_In(h,
			WSC_SETSELECTEDREGISTRA, "SetSelectedRegistrar", "NewMessage");
}

static const struct _soapMethods soapMethods[] =
{
//cathy, add "serviceName"
	{ 0, "GetDeviceInfo", WFAGetDeviceInfo },
	{ 0, "PutMessage", WFAPutMessage },
	{ 0, "SetSelectedRegistrar", WFASetSelectedRegistrar },
	{ 0, "PutWLANResponse", WFAPutWLANResponse },
	{ 0, 0, 0 }
};

static const struct _sendDesc sendDesc[] =
{
	{ "/simplecfg.xml", genWSCRootDesc, 0 },
	{ "/simplecfgservice.xml", genWSCServiceDesc, 0 },
	{ 0, .Impl.sendDescImpl=0, 0 }
};

static int Astrcmp(char *s1, char *s2)
{
  	int ret;

  	if (s1==NULL || s2==NULL)
    		return(1);
  	else
    	{
    		ret=strcmp(s1, s2);
    		return(ret);
    	}
}

static struct subscription_info *search_sid_entry(CTX_Tp pCtx, char *Sid)
{
	int i, idx=-1;

	for (i=0; i<MAX_SUBSCRIPTION_NUM; i++) {
		if (pCtx->upnp_subscription_info[i].used == 0) {
			if (idx < 0)
				idx = i;
			continue;
		}
		if (Astrcmp(pCtx->upnp_subscription_info[i].Sid, Sid) == 0)
			break;
	}

	if ( i < MAX_SUBSCRIPTION_NUM)
		return (&pCtx->upnp_subscription_info[i]);

	if (idx >= 0)
		return (&pCtx->upnp_subscription_info[idx]);

	return NULL;
}

/********************************************************************************
 * PsimplecfgDeviceHandleSubscriptionRequest
 *
 * Description:
 *       Called during a subscription request callback.  If the
 *       subscription request is for this device and either its
 *       control service or picture service, then accept it.
 *
 * Parameters:
 *   sr_event -- The subscription request event structure
 *
 ********************************************************************************/
static void PsimplecfgDeviceHandleSubscriptionRequest(struct upnp_subscription_element *sub)
{
    	Upnp_Document PropSet=NULL;
	unsigned char WLANEvent[21];
	unsigned char *WLANEvent_Base64=NULL;
	int WLANEvent_Base64Length=0;
	struct subscription_info *subscription=NULL;
	CTX_Tp pCtx = (CTX_Tp)user_priv_data;

	if (sub == NULL)
		return;
	subscription = search_sid_entry(pCtx, sub->sid);
	if (subscription == NULL) {
		DEBUG_ERR("SID table full\n");
		return;
	}
	if (!subscription->used) {
		subscription->used = 1;
		pCtx->TotalSubscriptions++;
		memcpy(subscription->Sid, sub->sid, UPNP_SID_LEN);
		subscription->subscription_timeout = UPNP_EXTERNAL_REG_EXPIRED;
	}

	PropSet = CreatePropertySet();
	if (PropSet == NULL) {
		pCtx->TotalSubscriptions--;
		memset(subscription, 0, sizeof(struct subscription_info));
		return;
	}

	_DEBUG_PRINT("Total subscription is %d\n", pCtx->TotalSubscriptions);

	if (upnp_op_mode == WSC_STA_MODE)
		UpnpAddToPropertySet(PropSet, "STAStatus", "1");
	else {
		WLANEvent[0] = WSC_8021XEAP_FRAME;
		sprintf(WLANEvent+1, "00:01:02:03:04:05");
		memcpy(WLANEvent+18, "123", 3);
		WLANEvent_Base64Length = ILibBase64Encode(WLANEvent, 21, &WLANEvent_Base64);

		UpnpAddToPropertySet(PropSet, "APStatus", "1");
		UpnpAddToPropertySet(PropSet, "WLANEvent", WLANEvent_Base64);
	}
	UpnpSendEventSingle(PropSet, &pCtx->upnp_info.subscribe_list, sub);
	_DEBUG_PRINT("Subscription Sid = %s\n\n", sub->sid);
	UpnpDocument_free(PropSet);

	if (WLANEvent_Base64)
		free(WLANEvent_Base64);
}

/********************************************************************************
 * PsimplecfgDeviceHandleRenewalSubscriptionRequest
 *
 * Description:
 *       Called during a renewal subscription request callback.  If the
 *       renewal subscription request is for this device and either its
 *       control service or picture service, then accept it.
 *
 * Parameters:
 *   sr_event -- The renewal subscription request event structure
 *
 ********************************************************************************/
static void PsimplecfgDeviceHandleRenewalSubscriptionRequest(struct upnp_subscription_element *sub)
{
	int reset=0;
	CTX_Tp pCtx = (CTX_Tp)user_priv_data;
	struct subscription_info *subscription=NULL;

	if (sub == NULL)
		return;
	_DEBUG_PRINT("RenewalSubscription Sid = %s\n\n", sub->sid);
	subscription = search_sid_entry(pCtx, sub->sid);
	if (subscription) {
		if (!subscription->used) {
			if (pCtx->setSelectedRegTimeout == 0)
						reset = 1;
		}
		else
			subscription->subscription_timeout = UPNP_EXTERNAL_REG_EXPIRED;
	}
	else {
		DEBUG_ERR("Error : Sid table full or renewal subscription expired!\n");
	    	return;
	}

	if (reset) { //deal with Vista's bug
		memset(pCtx->upnp_subscription_info, 0, (MAX_SUBSCRIPTION_NUM * sizeof(struct subscription_info)));
		pCtx->TotalSubscriptions = 0;

#ifdef STAND_ALONE_MINIUPNP
		//sending byebye
		SendSSDPNotifies(pCtx->upnp_info.snotify, pCtx->upnp_info.lan_ip_address, pCtx->upnp_info.port,
			&pCtx->upnp_info.SSDP, 1, 1);
		_DEBUG_PRINT("Sending bye bye...\n");

		//sending alive
		SendSSDPNotifies(pCtx->upnp_info.snotify, pCtx->upnp_info.lan_ip_address, pCtx->upnp_info.port,
			&pCtx->upnp_info.SSDP, 0, pCtx->upnp_info.SSDP.max_age);
		_DEBUG_PRINT("Sending Advertisement...\n\n");

		pCtx->upnp_info.SSDP.alive_timeout = pCtx->upnp_info.SSDP.max_age/2;
#else
		FILE *fp;

		if ((fp = fopen(WSCD_BYEBYE_FILE,"w")) == NULL) {
			DEBUG_ERR("output file [%s] can not open\n", WSCD_BYEBYE_FILE);
		}
		else {
			fputs("1\n" , fp); // sending bye bye and then sending advertisement
			fclose(fp);
		}
#endif
	}
}

/********************************************************************************
 * PsimplecfgDeviceHandleUnSubscribeRequest
 *
 * Description:
 *       Called during an unsubscription request callback.  If the
 *       unsubscription request is for this device and either its
 *       control service or picture service, then accept it.
 *
 * Parameters:
 *   sr_event -- The unsubscription request event structure
 *
 ********************************************************************************/
static void PsimplecfgDeviceHandleUnSubscribeRequest(struct upnp_subscription_element *sub)
{
	CTX_Tp pCtx = (CTX_Tp)user_priv_data;
	struct subscription_info *subscription=NULL;


	_DEBUG_PRINT("UnSubscribe Sid = %s\n\n", sub->sid);

	subscription = search_sid_entry(pCtx, sub->sid);
	if (subscription) {
		if (subscription->used) {
			_DEBUG_PRINT("Remove Sid [%s]\n", subscription->Sid);
			memset(subscription, 0, sizeof(struct subscription_info));
			pCtx->TotalSubscriptions--;
			_DEBUG_PRINT("Total subscription is %d\n", pCtx->TotalSubscriptions);
		}
	}
	else {
		DEBUG_ERR("Error : Sid table full or invalid unsubscription!\n");
	}
}

static void PsimplecfgDeviceCallbackEventHandler(struct upnp_subscription_element *sub)
{
	switch (sub->eventID)
	{
		case UPNP_EVENT_SUBSCRIPTION_REQUEST:
			PsimplecfgDeviceHandleSubscriptionRequest(sub);
			break;

		case UPNP_EVENT_RENEWAL_COMPLETE:
			PsimplecfgDeviceHandleRenewalSubscriptionRequest(sub);
			break;

		case UPNP_EVENT_UNSUBSCRIBE_COMPLETE:
			PsimplecfgDeviceHandleUnSubscribeRequest(sub);
			break;

		default:
			break;
	}
}

static int mini_UPnP_DeviceStateTableInit(CTX_Tp pCtx)
{
    	int ret = UPNP_E_SUCCESS;
	char *evnturl_ctrl = NULL, *ctrlurl_ctrl = NULL;
	char file_path[40];
	char *psimplecfg_udn=NULL;

	memset(file_path, 0, 40);
	sprintf(file_path, "%s%s.xml", PSIMPLECFG_INIT_CONF_DIR, PSIMPLECFG_INIT_DESC_DOC);
	if ((pCtx->upnp_info.rootXML = mini_UPnP_UploadXML(file_path)) == NULL) {
		ret = UPNP_E_INVALID_PARAM;
		goto error_handle;
	}
	pCtx->upnp_info.rootXML_len = strlen(pCtx->upnp_info.rootXML);
	//printf("\nmini_UPnP_DeviceStateTableInit\n%s\n", pCtx->upnp_info->rootXML);

	memset(file_path, 0, 40);
	sprintf(file_path, "%s%s.xml", PSIMPLECFG_INIT_CONF_DIR, PSIMPLECFG_SERVICE_DESC_DOC);
	if ((pCtx->upnp_info.serviceXML = mini_UPnP_UploadXML(file_path)) == NULL) {
		ret = UPNP_E_INVALID_PARAM;
		goto error_handle;
	}
	pCtx->upnp_info.serviceXML_len = strlen(pCtx->upnp_info.serviceXML);
	//printf("\nmini_UPnP_DeviceStateTableInit\n%s\n", pCtx->upnp_info->serviceXML);

    	if ((psimplecfg_udn = mini_UPnPGetFirstElement(pCtx->upnp_info.rootXML, pCtx->upnp_info.rootXML_len, "UDN", 3)) == NULL) {
		ret = UPNP_E_INVALID_PARAM;
		goto error_handle;
	}

	if (strlen(psimplecfg_udn) != 41) {
		ret = UPNP_E_INVALID_PARAM;
		goto error_handle;
	}
	memcpy(pCtx->upnp_info.SSDP.uuid, psimplecfg_udn, 41);
	free(psimplecfg_udn);
	pCtx->upnp_info.SSDP.known_service_types = (char **)known_service_types;
#ifdef DEBUG
	int i=0;
	while (pCtx->upnp_info.SSDP.known_service_types[i]) {
		_DEBUG_PRINT("\t%s%s\n", pCtx->upnp_info.SSDP.known_service_types[i],
			(i==0?"":"1"));
		i++;
	}
	_DEBUG_PRINT("\t%s\n", pCtx->upnp_info.SSDP.uuid);
#endif
	pCtx->upnp_info.SSDP.max_age = (unsigned int)default_advr_expire;
	pCtx->upnp_info.SSDP.alive_timeout = pCtx->upnp_info.SSDP.max_age/2;
	memcpy(pCtx->upnp_info.SSDP.root_desc_name, PSIMPLECFG_INIT_DESC_DOC, strlen(PSIMPLECFG_INIT_DESC_DOC));

	if ((evnturl_ctrl = mini_UPnPGetFirstElement(pCtx->upnp_info.rootXML, pCtx->upnp_info.rootXML_len, "eventSubURL", 11)) == NULL) {
		ret = UPNP_E_INVALID_PARAM;
		goto error_handle;
	}
	memcpy(pCtx->upnp_info.subscribe_list.event_url, evnturl_ctrl, strlen(evnturl_ctrl));
	free(evnturl_ctrl);
	_DEBUG_PRINT("\tevnturl_ctrl = %s\n", pCtx->upnp_info.subscribe_list.event_url);

	return ret;

error_handle:
	if (evnturl_ctrl)
		free(evnturl_ctrl);
	if (ctrlurl_ctrl)
		free(ctrlurl_ctrl);
	if (psimplecfg_udn)
		free(psimplecfg_udn);

    	return ret;
}

static int mini_UPnPInit(CTX_Tp pCtx)
{
	int shttpl;
#ifdef STAND_ALONE_MINIUPNP
	int sudp, snotify;

	/* socket d'ecoute pour le SSDP */
	sudp = OpenAndConfUdpSocket(pCtx->upnp_info.lan_ip_address);
	if (sudp < 0)
	{
		printf("Failed to open socket for SSDP. EXITING\n");
		return UPNP_E_INVALID_PARAM;
	}
	pCtx->upnp_info.sudp = sudp;

	/* open socket for sending notifications */
	snotify = OpenAndConfNotifySocket(pCtx->upnp_info.lan_ip_address);
	if (snotify < 0)
	{
		printf("Failed to open socket for SSDP notify messages\n");
		return UPNP_E_INVALID_PARAM;
	}
	pCtx->upnp_info.snotify = snotify;
#endif

	/* socket d'ecoute des connections HTTP */
	shttpl = OpenAndConfHTTPSocket(pCtx->upnp_info.lan_ip_address, pCtx->upnp_info.port);
	if (shttpl < 0)
	{
		printf("Failed to open socket for HTTP. EXITING\n");
		return UPNP_E_INVALID_PARAM;
	}
	pCtx->upnp_info.shttpl = shttpl;

	if (mini_UPnP_DeviceStateTableInit(pCtx) != UPNP_E_SUCCESS)
		return UPNP_E_INVALID_PARAM;

	return UPNP_E_SUCCESS;
}

int WSCUpnpStart(char *ifname, OpMode mode, struct WSC_profile *profile)
{
	CTX_Tp pCtx = (CTX_Tp)user_priv_data;
	int ret=0;
	char *desc_doc_name=NULL, *conf_dir_path=NULL;
    	char desc_doc_url[200];
	char profile_name[20];
	IPCon ipcon=NULL;
	struct timeval tod;

	if (WSCCallBack_registered != 1)
		return WSC_UPNP_FAIL;

	if (mode == WSC_AP_MODE || mode == WSC_STA_MODE)
		upnp_op_mode = mode;
	else
		return WSC_UPNP_FAIL;

	ipcon = IPCon_New(ifname);
	if (ipcon == NULL)
		return WSC_UPNP_FAIL;

	memset(&pCtx->upnp_info, 0, sizeof(mini_upnp_CTX_T));
	LIST_INIT(&pCtx->upnp_info.upnphttphead);
	LIST_INIT(&pCtx->upnp_info.subscribe_list.EvtResp_head);
	LIST_INIT(&pCtx->upnp_info.subscribe_list.subscription_head);
	pCtx->upnp_info.subscribe_list.max_subscription_num = MAX_SUBSCRIPTION_NUM;
	gettimeofday(&tod , NULL);
	srand(tod.tv_sec);
	pCtx->upnp_info.port = 50000 + (rand() % 10000);
	strcpy(pCtx->upnp_info.lan_ip_address, IPCon_GetIpAddrByStr(ipcon));
	IPCon_Destroy(ipcon);
	pCtx->upnp_info.subscribe_list.my_port = pCtx->upnp_info.port;
	memcpy(pCtx->upnp_info.subscribe_list.my_IP, pCtx->upnp_info.lan_ip_address, IP_ADDRLEN);
	pCtx->upnp_info.subscribe_list.max_subscription_time = MAX_SUBSCRIPTION_TIMEOUT;
	pCtx->upnp_info.subscribe_list.subscription_timeout = UPNP_EXTERNAL_REG_EXPIRED;
	pCtx->upnp_info.subscribe_list.EventCallBack = PsimplecfgDeviceCallbackEventHandler;
	pCtx->upnp_info.soapMethods = (struct _soapMethods *)soapMethods;
	pCtx->upnp_info.sendDesc = (struct _sendDesc *)sendDesc;

    	desc_doc_name = PSIMPLECFG_INIT_DESC_DOC;
    	conf_dir_path = PSIMPLECFG_INIT_CONF_DIR;
    	sprintf(desc_doc_url, "http://%s:%d/%s.xml", pCtx->upnp_info.lan_ip_address, pCtx->upnp_info.port, desc_doc_name);
    	_DEBUG_PRINT("Intializing Simple Config UPnP \n\tdesc_doc_url=%s\n", desc_doc_url);

	memset(profile_name, 0, 20);
	sprintf(profile_name, "%s.xml", desc_doc_name);

	if (gen_simplecfg_xml(pCtx->upnp_info.lan_ip_address, pCtx->upnp_info.port, conf_dir_path, profile_name, profile) != WSC_UPNP_SUCCESS)
	{
		printf("Error in gen_simplecfg_xml!\n");
		WSCUpnpStop();
		return WSC_UPNPINIT_FAIL;
	}

	if ((ret = mini_UPnPInit(pCtx)) != UPNP_E_SUCCESS) {
		printf("Error with UpnpInit -- %d\n", ret);
		WSCUpnpStop();
		return WSC_UPNPINIT_FAIL;
    	}

	return WSC_UPNP_SUCCESS;
}

void WSCUpnpStop(void)
{
	if (WSCCallBack_registered) {
		CTX_Tp pCtx = (CTX_Tp)user_priv_data;
		struct upnphttp * e = 0;
		struct upnphttp * next = 0;
		struct upnp_subscription_element *tmp=NULL;
		struct upnp_subscription_element *tmp_next=NULL;
		struct EvtRespElement *EvtResp=NULL;
		struct EvtRespElement *EvtResp_next=NULL;

#ifdef STAND_ALONE_MINIUPNP
		//sending byebye
		if (pCtx->upnp_info.snotify > 0)
			SendSSDPNotifies(pCtx->upnp_info.snotify, pCtx->upnp_info.lan_ip_address, pCtx->upnp_info.port,
				&pCtx->upnp_info.SSDP, 1, 10);
#else
		FILE *fp;

		if ((fp = fopen(WSCD_BYEBYE_FILE,"w")) == NULL) {
			DEBUG_ERR("output file [%s] can not open\n", WSCD_BYEBYE_FILE);
		}
		else {
			fputs("2\n" , fp); // sending bye bye only
			fclose(fp);
		}
#endif

		for(e = pCtx->upnp_info.upnphttphead.lh_first; e != NULL; )
		{
			next = e->entries.le_next;
			LIST_REMOVE(e, entries);
			Delete_upnphttp(e);
			e = next;
		}

		for (EvtResp = pCtx->upnp_info.subscribe_list.EvtResp_head.lh_first; EvtResp != NULL; )
		{
			EvtResp_next = EvtResp->entries.le_next;
			LIST_REMOVE(EvtResp, entries);
			if (EvtResp->socket >= 0)
				close(EvtResp->socket);
			free(EvtResp);
			EvtResp = EvtResp_next;
		}
		for (tmp = pCtx->upnp_info.subscribe_list.subscription_head.lh_first; tmp != NULL; )
		{
			tmp_next = tmp->entries.le_next;
			LIST_REMOVE(tmp, entries);
			free(tmp);
			tmp = tmp_next;
		}

#ifdef STAND_ALONE_MINIUPNP
		if (pCtx->upnp_info.sudp >= 0)
			close(pCtx->upnp_info.sudp);
		if (pCtx->upnp_info.snotify >= 0)
			close(pCtx->upnp_info.snotify);
#endif
		if (pCtx->upnp_info.shttpl >= 0)
			close(pCtx->upnp_info.shttpl);
		if (pCtx->upnp_info.rootXML)
			free(pCtx->upnp_info.rootXML);
		if (pCtx->upnp_info.serviceXML)
			free(pCtx->upnp_info.serviceXML);

		WSCCallBack_registered = 0;

		//to do: close more resourse if needed
#ifdef DEBUG
		closelog();
#endif
	}

	return;
}
