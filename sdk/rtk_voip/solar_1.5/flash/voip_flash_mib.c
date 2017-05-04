/*
 * voip_flash_mib.c: VoIP MIB File
 *
 * Authors: Rock <shaofu@realtek.com.tw>
 *
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
	#include <io.h>
	#include <winsock2.h>
#else
	#include <unistd.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
#endif

#include "voip_flash.h"
#include "voip_flash_mib.h"
#include "voip_flash_tool.h"

typedef struct {
	char name[16];
	int size;
} V_TYPE_INFO;

const V_TYPE_INFO v_types[V_END] = {
	{"V_INT",			sizeof(int)},
	{"V_UINT",			sizeof(int)},
	// 1 byte
	{"V_CHAR", 			sizeof(char)},
	{"V_BYTE",			sizeof(char)},
	// 2 byte
	{"V_SHORT",			sizeof(short)},
	{"V_WORD",			sizeof(short)},
	// 4 byte
	{"V_LONG",			sizeof(long)},
	{"V_DWORD",			sizeof(long)},
	// other
	{"V_IP4",			4},
	{"V_MAC6",			6},
	{"V_MIB",			0},	// length is decided by mib_tbl
	// array
	{"V_STRING",		sizeof(char)},
	{"V_BYTE_ARRAY",	sizeof(char)},
	// list
	{"V_INT_LIST",		sizeof(int)},
	{"V_UINT_LIST",		sizeof(int)},
	{"V_CHAR_LIST",		sizeof(char)},	
	{"V_BYTE_LIST",		sizeof(char)},
	{"V_SHORT_LIST",	sizeof(short)},
	{"V_WORD_LIST",		sizeof(short)},
	{"V_LONG_LIST",		sizeof(long)},
	{"V_DWORD_LIST",	sizeof(long)},
	{"V_IP4_LIST",		4},
	{"V_MAC6_LIST",		6},
	{"V_MIB_LIST",		0}	// length is decided by mib_tbl
};

/*
 * MIB Name rule:
 *	- Append a keyword "DEF_" in ahead of mib name when access default setting MIB.
 *  - Append a keyword "PORTx_" in ahead of mib name when access VoIP port setting.
 *  - Append a keyword "DEF_PORTx_" in ahead of mib name when access default VoIP port setting.
 *	- Append "_x" behind the mib name when access array type mib. 
 *
 * Examples:
 *	1. DEF_PORT0_SPEED_DIAL0_NAME
 *	2. DEF_PORT1_SPEED_DIAL9_NAME
 *	3. DEF_PORT0_
 */

voipMibEntry_T mibtbl_speed_dial[] = {
	{MIB_VOIP_SPEED_DIAL_NAME,	"NAME",	V_STRING,	VOIP_SPEED_DIAL_STRING_FIELD(name)},
	{MIB_VOIP_SPEED_DIAL_URL,	"URL",	V_STRING,	VOIP_SPEED_DIAL_STRING_FIELD(url)},
	{0}
};

voipMibEntry_T mibtbl_proxies[] = {
	// proxy setting - general
	{MIB_VOIP_PROXY_DISPLAY_NAME,	"DISPLAY_NAME",	V_STRING,	VOIP_PROXY_STRING_FIELD(display_name)},
	{MIB_VOIP_PROXY_NUMBER,			"NUMBER",		V_STRING,	VOIP_PROXY_STRING_FIELD(number)},
	{MIB_VOIP_PROXY_LOGIN_ID,		"LOGIN_ID",		V_STRING,	VOIP_PROXY_STRING_FIELD(login_id)},
	{MIB_VOIP_PROXY_PASSWORD,		"PASSWORD",		V_STRING,	VOIP_PROXY_STRING_FIELD(password)},
	// proxy setting - register server
	{MIB_VOIP_PROXY_ENABLE,			"ENABLE",		V_WORD,		VOIP_PROXY_FIELD(enable)},
	{MIB_VOIP_PROXY_ADDR,			"ADDR",			V_STRING,	VOIP_PROXY_STRING_FIELD(addr)},
	{MIB_VOIP_PROXY_PORT,			"PORT",			V_WORD,		VOIP_PROXY_FIELD(port)},
	{MIB_VOIP_PROXY_DOMAIN_NAME,	"DOMAIN_NAME",	V_STRING,	VOIP_PROXY_STRING_FIELD(domain_name)},
	{MIB_VOIP_PROXY_REG_EXPIRE,		"REG_EXPIRE",	V_UINT,		VOIP_PROXY_FIELD(reg_expire)},
	// proxy setting - nat traversal server
	{MIB_VOIP_PROXY_OUTBOUND_ENABLE,	"OUTBOUND_ENABLE",	V_WORD,		VOIP_PROXY_FIELD(outbound_enable)},
	{MIB_VOIP_PROXY_OUTBOUND_ADDR,		"OUTBOUND_ADDR",	V_STRING,	VOIP_PROXY_STRING_FIELD(outbound_addr)},
	{MIB_VOIP_PROXY_OUTBOUND_PORT,		"OUTBOUND_PORT",	V_WORD,		VOIP_PROXY_FIELD(outbound_port)},
	// sip tls enable
	{MIB_VOIP_TLS_ENABLE,		"SIPTLS_ENABLE",	V_WORD,		VOIP_PROXY_FIELD(siptls_enable)},
	{0}
};

voipMibEntry_T mibtbl_abbreviated_dial[] = {
	{MIB_VOIP_ABBREVIATED_DIAL_NAME,	"NAME",	V_STRING,	VOIP_ABBREVIATED_DIAL_STRING_FIELD(name)},
	{MIB_VOIP_ABBREVIATED_DIAL_URL,		"URL",	V_STRING,	VOIP_ABBREVIATED_DIAL_STRING_FIELD(url)},
	{0}
};

voipMibEntry_T mibtbl_voip_port[] = {
	// port setting - proxies
	{MIB_VOIP_PROXIES,			"PROXIES",			V_MIB_LIST,	VOIP_PORT_MIB_LIST_FIELD(proxies, mibtbl_proxies)},
	{MIB_VOIP_DEFAULT_PROXY,	"DEFAULT_PROXY",	V_BYTE,		VOIP_PORT_FIELD(default_proxy)},

	// port setting - NAT Traversal
	{MIB_VOIP_STUN_ENABLE,	"STUN_ENABLE",		V_BYTE,		VOIP_PORT_FIELD(stun_enable)},
	{MIB_VOIP_STUN_ADDR,	"STUN_ADDR",		V_STRING,	VOIP_PORT_STRING_FIELD(stun_addr)},
	{MIB_VOIP_STUN_PORT,	"STUN_PORT",		V_WORD,		VOIP_PORT_FIELD(stun_port)},

	// port setting - advanced
	{MIB_VOIP_SIP_PORT,				"SIP_PORT",				V_WORD,		VOIP_PORT_FIELD(sip_port)},
	{MIB_VOIP_MEDIA_PORT,			"MEDIA_PORT",			V_WORD,		VOIP_PORT_FIELD(media_port)},
	{MIB_VOIP_DTMF_MODE,			"DTMF_MODE",			V_BYTE,		VOIP_PORT_FIELD(dtmf_mode)},
	{MIB_VOIP_PAYLOAD_TYPE,			"PAYLOAD_TYPE",			V_WORD,		VOIP_PORT_FIELD(payload_type)},
	{MIB_VOIP_SIP_INFO_DURATION,	"SIP_INFO_DURATION",	V_WORD,		VOIP_PORT_FIELD(sip_info_duration)},
	{MIB_VOIP_CALL_WAITING_ENABLE,	"CALL_WAITING_ENABLE",	V_BYTE,		VOIP_PORT_FIELD(call_waiting_enable)},
	{MIB_VOIP_DIRECT_IP_CALL,		"DIRECT_IP_CALL",		V_BYTE,		VOIP_PORT_FIELD(direct_ip_call)},

	// port setting - forward
	{MIB_VOIP_UC_FORWARD_ENABLE,	"UC_FORWARD_ENABLE",	V_BYTE,		VOIP_PORT_FIELD(uc_forward_enable)},
	{MIB_VOIP_UC_FORWARD,			"UC_FORWARD",			V_STRING,	VOIP_PORT_STRING_FIELD(uc_forward)},	
	{MIB_VOIP_BUSY_FORWARD_ENABLE,	"BUSY_FORWARD_ENABLE",	V_BYTE,		VOIP_PORT_FIELD(busy_forward_enable)},
	{MIB_VOIP_BUSY_FORWARD,			"BUSY_FORWARD",			V_STRING,	VOIP_PORT_STRING_FIELD(busy_forward)},
	{MIB_VOIP_NA_FORWARD_ENABLE,	"NA_FORWARD_ENABLE",	V_BYTE,		VOIP_PORT_FIELD(na_forward_enable)},
	{MIB_VOIP_NA_FORWARD,			"NA_FORWARD",			V_STRING,	VOIP_PORT_STRING_FIELD(na_forward)},
	{MIB_VOIP_NA_FORWARD_TIME,		"NA_FORWARD_TIME",		V_WORD,		VOIP_PORT_FIELD(na_forward_time)},

	// port setting - speed dial
	{MIB_VOIP_SPEED_DIAL,	"SPEED_DIAL",	V_MIB_LIST,	VOIP_PORT_MIB_LIST_FIELD(speed_dial, mibtbl_speed_dial)},

//#ifdef CONFIG_RTK_VOIP_DIALPLAN
	// port setting - dial plan
	{MIB_VOIP_REPLACE_RULE_OPTION,	"REPLACE_RULE_OPTION",	V_BYTE,		VOIP_PORT_FIELD(replace_rule_option)},
	{MIB_VOIP_REPLACE_RULE_SOURCE,	"REPLACE_RULE_SOURCE",	V_STRING,	VOIP_PORT_STRING_FIELD(replace_rule_source)},
	{MIB_VOIP_REPLACE_RULE_TARGET,	"REPLACE_RULE_TARGET",	V_STRING,	VOIP_PORT_STRING_FIELD(replace_rule_target)},
	{MIB_VOIP_DIAL_PLAN,			"DIAL_PLAN",			V_STRING,	VOIP_PORT_STRING_FIELD(dialplan)},
	{MIB_VOIP_AUTO_PREFIX,			"AUTO_PREFIX",			V_STRING,	VOIP_PORT_STRING_FIELD(auto_prefix)},
	{MIB_VOIP_PREFIX_UNSET_PLAN,	"PREFIX_UNSET_PLAN",	V_STRING,	VOIP_PORT_STRING_FIELD(prefix_unset_plan)},
	// {MIB_VOIP_DIAL_PLAN_TIME,		"DIAL_PLAN_TIME",		V_WORD,		VOIP_PORT_FIELD(dialplan_time)},
//#endif
	// port setting - codec
	{MIB_VOIP_FRAME_SIZE,	"FRAME_SIZE",	V_BYTE_LIST,	VOIP_PORT_LIST_FIELD(frame_size)},
	{MIB_VOIP_PRECEDENCE,	"PRECEDENCE",	V_BYTE_LIST,	VOIP_PORT_LIST_FIELD(precedence)},
	{MIB_VOIP_VAD,			"VAD",			V_BYTE,			VOIP_PORT_FIELD(vad)},
	{MIB_VOIP_VAD_THRESHOLD,	"VAD_THRESHOLD",	V_BYTE,			VOIP_PORT_FIELD(vad_thr)},
	{MIB_VOIP_CNG,			"CNG",			V_BYTE,			VOIP_PORT_FIELD(cng)},
	{MIB_VOIP_CNG_THRESHOLD,	"CNG_THRESHOLD",	V_BYTE,			VOIP_PORT_FIELD(cng_thr)},
	{MIB_VOIP_CNG_MODE,		"CNG_CONFIG_MODE",	V_BYTE,			VOIP_PORT_FIELD(cng_gainmode)},
	{MIB_VOIP_CNG_LEVEL,		"CNG_LEVEL_VALUE",	V_BYTE,			VOIP_PORT_FIELD(cng_noiselevel)},
	{MIB_VOIP_CNG_GAIN,		"CNG_GAIN_VALUE",	V_BYTE,			VOIP_PORT_FIELD(cng_noisegain)},
	{MIB_VOIP_PLC,			"PLC",			V_BYTE,			VOIP_PORT_FIELD(PLC)},
	{MIB_VOIP_RTCP_INTERVAL,"RTCP_INTERVAL",		V_BYTE,			VOIP_PORT_FIELD(RTCP_Interval)},
	{MIB_VOIP_RTCPXR,		"RTCPXR",		V_BYTE,			VOIP_PORT_FIELD(RTCP_XR)},
	{MIB_VOIP_G7231_RATE,	"G7231_RATE",	V_BYTE,			VOIP_PORT_FIELD(g7231_rate)},
	{MIB_VOIP_ILBC_MODE,	"ILBC_MODE",	V_BYTE,			VOIP_PORT_FIELD(iLBC_mode)},
	{MIB_VOIP_SPEEX_NB_RATE,	"SPEEX_NB_RATE",	V_BYTE,			VOIP_PORT_FIELD(speex_nb_rate)},
	{MIB_VOIP_G726_PACK,	"G726_PACK",	V_BYTE,			VOIP_PORT_FIELD(g726_packing)},

	// port setting - RTP redundant 
	{MIB_VOIP_RTP_RED_PAYLOAD_TYPE,	"RTP_RED_PAYLOAD_TYPE",	V_BYTE,	VOIP_PORT_FIELD(rtp_redundant_payload_type)},
	{MIB_VOIP_RTP_RED_CODEC,		"RTP_RED_CODEC",		V_BYTE,	VOIP_PORT_FIELD(rtp_redundant_codec)},

	// port setting - DSP
	{MIB_VOIP_SLIC_TX_VOLUME,	"SLIC_TX_VOLUME",	V_BYTE,	VOIP_PORT_FIELD(slic_txVolumne)},
	{MIB_VOIP_SLIC_RX_VOLUME,	"SLIC_RX_VOLUME",	V_BYTE,	VOIP_PORT_FIELD(slic_rxVolumne)},
	{MIB_VOIP_JITTER_DELAY,		"JITTER_DELAY",		V_BYTE,	VOIP_PORT_FIELD(jitter_delay)},
	{MIB_VOIP_MAX_DELAY,		"MAX_DELAY",		V_BYTE,	VOIP_PORT_FIELD(maxDelay)},
	{MIB_VOIP_JITTER_FACTOR,	"JITTER_FACTOR",	V_BYTE,	VOIP_PORT_FIELD(jitter_factor)},
	{MIB_VOIP_ECHO_LEC,		"LEC",			V_BYTE,	VOIP_PORT_FIELD(lec)},
	{MIB_VOIP_ECHO_NLP,		"NLP",			V_BYTE,	VOIP_PORT_FIELD(nlp)},
	{MIB_VOIP_ECHO_TAIL,		"ECHO_TAIL",		V_BYTE,	VOIP_PORT_FIELD(echoTail)},
	{MIB_VOIP_CALLER_ID_MODE,	"CALLER_ID_MODE",	V_BYTE,	VOIP_PORT_FIELD(caller_id_mode)},
	{MIB_VOIP_CALL_WAITING_CID,	"CALL_WAITING_CID",	V_BYTE,	VOIP_PORT_FIELD(call_waiting_cid)},
	{MIB_VOIP_CID_DTMF_MODE,	"CID_DTMF_MODE",	V_BYTE, VOIP_PORT_FIELD(cid_dtmf_mode)},
	{MIB_VOIP_SPEAKERAGC,		"SPEAKERAGC",		V_BYTE, VOIP_PORT_FIELD(speaker_agc)},
	{MIB_VOIP_SPK_AGC_LVL,		"SPK_AGC_LVL",		V_BYTE, VOIP_PORT_FIELD(spk_agc_lvl)},
	{MIB_VOIP_SPK_AGC_GU,		"SPK_AGC_GU",		V_BYTE, VOIP_PORT_FIELD(spk_agc_gu)},
	{MIB_VOIP_SPK_AGC_GD,		"SPK_AGC_GD",		V_BYTE, VOIP_PORT_FIELD(spk_agc_gd)},
	{MIB_VOIP_MICAGC,		"MICAGC",		V_BYTE, VOIP_PORT_FIELD(mic_agc)},
	{MIB_VOIP_MIC_AGC_LVL,		"MIC_AGC_LVL",		V_BYTE, VOIP_PORT_FIELD(mic_agc_lvl)},
	{MIB_VOIP_MIC_AGC_GU,		"MIC_AGC_GU",		V_BYTE, VOIP_PORT_FIELD(mic_agc_gu)},
	{MIB_VOIP_MIC_AGC_GD,		"MIC_AGC_GD",		V_BYTE, VOIP_PORT_FIELD(mic_agc_gd)},
	{MIB_VOIP_FSK_GEN_MODE,		"FSK_GEN_MODE",		V_BYTE, VOIP_PORT_FIELD(cid_fsk_gen_mode)},
	{MIB_VOIP_SPK_VOICE_GAIN,	"SPK_VOICE_GAIN",	V_BYTE, VOIP_PORT_FIELD(spk_voice_gain)},
	{MIB_VOIP_MIC_VOICE_GAIN,	"MIC_VOICE_GAIN",	V_BYTE, VOIP_PORT_FIELD(mic_voice_gain)},
	{MIB_VOIP_ANSTONE,		"ANSTONE",		V_DWORD, VOIP_PORT_FIELD(anstone)},
	{MIB_VOIP_FAX_MODEM_RFC2833,	"FAX_MODEM_RFC2833",	V_BYTE, VOIP_PORT_FIELD(faxmodem_rfc2833)},
	// QoS
	{MIB_VOIP_VOICE_QOS,		"VOICE_QOS",		V_BYTE,	VOIP_PORT_FIELD(voice_qos)},
	
	//T.38
	{MIB_VOIP_T38_USET38,		"T38_ENABLE",		V_BYTE,	VOIP_PORT_FIELD(useT38)},
	{MIB_VOIP_T38_PORT,		"T38_PORT",		V_WORD,	VOIP_PORT_FIELD(T38_port)},
	{MIB_VOIP_FAX_MODEM_DET,	"FAX_MODEM_DET",	V_BYTE, VOIP_PORT_FIELD(fax_modem_det)},
	
	//T.38 parameters 
	{MIB_VOIP_T38_PARAM_ENABLE,	"T38_PARAM_ENABLE",		V_BYTE,	VOIP_PORT_FIELD(T38ParamEnable)},
	{MIB_VOIP_T38_MAX_BUFFER,	"T38_MAX_BUFFER",		V_SHORT,	VOIP_PORT_FIELD(T38MaxBuffer)},
	{MIB_VOIP_T38_RATE_MGT,		"FAX_RATE_MGT",		V_BYTE, VOIP_PORT_FIELD(T38RateMgt)},
	{MIB_VOIP_T38_MAX_RATE,		"T38_MAX_RATE",		V_BYTE,	VOIP_PORT_FIELD(T38MaxRate)},
	{MIB_VOIP_T38_ENABLE_ECM,	"T38_ENABLE_ECM",	V_BYTE,	VOIP_PORT_FIELD(T38EnableECM)},
	{MIB_VOIP_T38_ECC_SIGNAL,	"T38_ECC_SIGNAL",	V_BYTE, VOIP_PORT_FIELD(T38ECCSignal)},
	{MIB_VOIP_T38_ECC_DATA,		"T38_ECC_DATA",		V_BYTE,	VOIP_PORT_FIELD(T38ECCData)},
	
	// V.152
	{MIB_VOIP_USE_V152,				"V152_ENABLE",		V_BYTE,	VOIP_PORT_FIELD(useV152)},
	{MIB_VOIP_V152_PAYLOAD_TYPE,	"V152_PAYLOAD_TYPE",V_BYTE,	VOIP_PORT_FIELD(v152_payload_type)},
	{MIB_VOIP_V152_CODEC_TYPE,		"V152_CODEC_TYPE",	V_BYTE, VOIP_PORT_FIELD(v152_codec_type)},

	// Hot Line
	{MIB_VOIP_HOTLINE_ENABLE,		"HOTLINE_ENABLE",		V_BYTE,		VOIP_PORT_FIELD(hotline_enable)},
	{MIB_VOIP_HOTLINE_NUMBER,		"HOTLINE_NUMBER",		V_STRING,	VOIP_PORT_STRING_FIELD(hotline_number)},

	// DND
	{MIB_VOIP_DND_MODE,			"DND_MODE",			V_BYTE,		VOIP_PORT_FIELD(dnd_mode)},
	{MIB_VOIP_DND_FROM_HOUR,	"DND_FROM_HOUR",	V_BYTE,		VOIP_PORT_FIELD(dnd_from_hour)},
	{MIB_VOIP_DND_FROM_MIN,		"DND_FROM_MIN",		V_BYTE,		VOIP_PORT_FIELD(dnd_from_min)},
	{MIB_VOIP_DND_TO_HOUR,		"DND_TO_HOUR",		V_BYTE,		VOIP_PORT_FIELD(dnd_to_hour)},
	{MIB_VOIP_DND_TO_MIN,		"DND_TO_MIN",		V_BYTE,		VOIP_PORT_FIELD(dnd_to_min)},

	// flash hook time
	{MIB_VOIP_FLASH_HOOK_TIME,	"FLASH_HOOK_TIME",	V_WORD,	VOIP_PORT_FIELD(flash_hook_time)},
	{MIB_VOIP_FLASH_HOOK_TIME_MIN,	"FLASH_HOOK_TIME_MIN",	V_WORD,	VOIP_PORT_FIELD(flash_hook_time_min)},

	// security
	/* +++++Add by Jack for VoIP security 240108+++++ */
//#ifdef CONFIG_RTK_VOIP_SRTP
	{MIB_VOIP_SECURITY_ENABLE,	"SECURITY_ENABLE",	V_BYTE,	VOIP_PORT_FIELD(security_enable)},
	{MIB_VOIP_KEY_EXCHANGE_MODE,		"SECURITY_KEY_EXCHANGE_MODE", V_BYTE, VOIP_PORT_FIELD(key_exchange_mode)},
//#endif /*CONFIG_RTK_VOIP_SRTP*/
	/*-----end-----*/

	// auth
	{MIB_VOIP_OFFHOOK_PASSWD,	"OFFHOOK_PASSWD",	V_STRING,	VOIP_PORT_STRING_FIELD(offhook_passwd)},

	// port setting - abbreviated dial
	{MIB_VOIP_ABBREVIATED_DIAL,	"ABBREVIATED_DIAL",	V_MIB_LIST,	VOIP_PORT_MIB_LIST_FIELD(abbr_dial, mibtbl_abbreviated_dial)},

	// port setting - alarm 
	{MIB_VOIP_ALARM_ENABLE,			"ALARM_ENABLE",			V_BYTE,		VOIP_PORT_FIELD(alarm_enable)},
	{MIB_VOIP_ALARM_TIME_HH,		"ALARM_TIME_HH",		V_BYTE,		VOIP_PORT_FIELD(alarm_time_hh)},
	{MIB_VOIP_ALARM_TIME_MM,		"ALARM_TIME_MM",		V_BYTE,		VOIP_PORT_FIELD(alarm_time_mm)},
#if 0
	{MIB_VOIP_ALARM_RING_LAST_DAY,	"ALARM_RING_LAST_DAY",	V_BYTE,		VOIP_PORT_FIELD(alarm_ring_last_day)},
	{MIB_VOIP_ALARM_RING_DEFER,		"ALARM_RING_DEFER",		V_BYTE,		VOIP_PORT_FIELD(alarm_ring_defer)},
#endif

	// port setting - PSTN routing prefix  
	{MIB_VOIP_PSTN_ROUTING_PREFIX,	"PSTN_ROUTING_PREFIX",	V_STRING,	VOIP_PORT_STRING_FIELD(PSTN_routing_prefix)},

	{0}
};

voipMibEntry_T mibtbl_cust_tone[] = {
	{MIB_VOIP_CUST_TONE_TYPE,			"CUST_TONE_TYPE",			V_DWORD,	VOIP_CUST_TONE_FIELD(toneType)},
	{MIB_VOIP_CUST_TONE_CYCLE,			"CUST_TONE_CYCLE",			V_WORD,		VOIP_CUST_TONE_FIELD(cycle)},
	{MIB_VOIP_CUST_TONE_CAD_NUM,		"CUST_TONE_CAD_NUM",		V_WORD,		VOIP_CUST_TONE_FIELD(cadNUM)},
	{MIB_VOIP_CUST_TONE_CAD_ON0,		"CUST_TONE_CAD_ON0",		V_DWORD,	VOIP_CUST_TONE_FIELD(CadOn0)},
	{MIB_VOIP_CUST_TONE_CAD_ON1,		"CUST_TONE_CAD_ON1",		V_DWORD,	VOIP_CUST_TONE_FIELD(CadOn1)},
	{MIB_VOIP_CUST_TONE_CAD_ON2,		"CUST_TONE_CAD_ON2",		V_DWORD,	VOIP_CUST_TONE_FIELD(CadOn2)},
	{MIB_VOIP_CUST_TONE_CAD_ON3,		"CUST_TONE_CAD_ON3",		V_DWORD,	VOIP_CUST_TONE_FIELD(CadOn3)},
	{MIB_VOIP_CUST_TONE_CAD_OFF0,		"CUST_TONE_CAD_OFF0",		V_DWORD,	VOIP_CUST_TONE_FIELD(CadOff0)},
	{MIB_VOIP_CUST_TONE_CAD_OFF1,		"CUST_TONE_CAD_OFF1",		V_DWORD,	VOIP_CUST_TONE_FIELD(CadOff1)},
	{MIB_VOIP_CUST_TONE_CAD_OFF2,		"CUST_TONE_CAD_OFF2",		V_DWORD,	VOIP_CUST_TONE_FIELD(CadOff2)},
	{MIB_VOIP_CUST_TONE_CAD_OFF3,		"CUST_TONE_CAD_OFF3",		V_DWORD,	VOIP_CUST_TONE_FIELD(CadOff3)},
	{MIB_VOIP_CUST_TONE_PATTERN_OFF,	"CUST_TONE_PATTERN_OFF",	V_DWORD,	VOIP_CUST_TONE_FIELD(PatternOff)},
	{MIB_VOIP_CUST_TONE_NUM,			"CUST_TONE_NUM",			V_DWORD,	VOIP_CUST_TONE_FIELD(ToneNUM)},
	{MIB_VOIP_CUST_TONE_FREQ1,			"CUST_TONE_FREQ1",			V_DWORD,	VOIP_CUST_TONE_FIELD(Freq1)},
	{MIB_VOIP_CUST_TONE_FREQ2,			"CUST_TONE_FREQ2",			V_DWORD,	VOIP_CUST_TONE_FIELD(Freq2)},
	{MIB_VOIP_CUST_TONE_FREQ3,			"CUST_TONE_FREQ3",			V_DWORD,	VOIP_CUST_TONE_FIELD(Freq3)},
	{MIB_VOIP_CUST_TONE_FREQ4,			"CUST_TONE_FREQ4",			V_DWORD,	VOIP_CUST_TONE_FIELD(Freq4)},
	{MIB_VOIP_CUST_TONE_GAIN1,			"CUST_TONE_GAIN1",			V_LONG,		VOIP_CUST_TONE_FIELD(Gain1)},
	{MIB_VOIP_CUST_TONE_GAIN2,			"CUST_TONE_GAIN2",			V_LONG,		VOIP_CUST_TONE_FIELD(Gain2)},
	{MIB_VOIP_CUST_TONE_GAIN3,			"CUST_TONE_GAIN3",			V_LONG,		VOIP_CUST_TONE_FIELD(Gain3)},
	{MIB_VOIP_CUST_TONE_GAIN4,			"CUST_TONE_GAIN4",			V_LONG,		VOIP_CUST_TONE_FIELD(Gain4)},
	//{MIB_VOIP_CUST_TONE_DUMMY,			"CUST_TONE_DUMMY",			V_LONG,		VOIP_CUST_TONE_FIELD(dummy)},
	{0}
};

voipMibEntry_T mibtbl_voip[] = {
	// voip flash info
	{MIB_VOIP_SIGNATURE,		"SIGNATURE",		V_DWORD,	VOIP_FIELD(signature)},
	{MIB_VOIP_VERSION,			"VERSION",			V_WORD,		VOIP_FIELD(version)},
	{MIB_VOIP_FEATURE,			"FEATURE",			V_DWORD,	VOIP_FIELD(feature)},
	{MIB_VOIP_EXTEND_FEATURE,	"EXTEND_FEATURE",	V_DWORD,	VOIP_FIELD(extend_feature)},
	{MIB_VOIP_MIB_VERSION,		"MIB_VERSION",		V_WORD,		VOIP_FIELD(mib_version)},

	// RFC flags
	{MIB_VOIP_RFC_FLAGS,	"RFC_FLAGS",	V_UINT,	VOIP_FIELD(rfc_flags)},

	// tone
	{MIB_VOIP_TONE_OF_COUNTRY,		"TONE_OF_COUNTRY",		V_BYTE,	VOIP_FIELD(tone_of_country)},
	{MIB_VOIP_TONE_OF_CUSTDIAL,		"TONE_OF_CUSTDIAL",		V_BYTE,	VOIP_FIELD(tone_of_custdial)},
	{MIB_VOIP_TONE_OF_CUSTRING,		"TONE_OF_CUSTRING",		V_BYTE,	VOIP_FIELD(tone_of_custring)},
	{MIB_VOIP_TONE_OF_CUSTBUSY,		"TONE_OF_CUSTBUSY",		V_BYTE,	VOIP_FIELD(tone_of_custbusy)},
	{MIB_VOIP_TONE_OF_CUSTWAITING,	"TONE_OF_CUSTWAITING",	V_BYTE,	VOIP_FIELD(tone_of_custwaiting)},
	{MIB_VOIP_TONE_OF_CUSTOMIZE,	"TONE_OF_CUSTOMIZE",	V_BYTE,	VOIP_FIELD(tone_of_customize)},

	// customize tone
	{MIB_VOIP_CUST_TONE,	"CUST_TONE",	V_MIB_LIST,	VOIP_MIB_LIST_FIELD(cust_tone_para, mibtbl_cust_tone)},

	// disconnect tone det
	{MIB_VOIP_DISTONE_NUM,		"DISTONE_NUM",		V_BYTE,	VOIP_FIELD(distone_num)},
	{MIB_VOIP_D1FREQNUM,		"D1FREQNUM",		V_BYTE,	VOIP_FIELD(d1freqnum)},
	{MIB_VOIP_D1FREQ1,		"D1FREQ1",		V_WORD,	VOIP_FIELD(d1Freq1)},
	{MIB_VOIP_D1FREQ2,		"D1FREQ2",		V_WORD,	VOIP_FIELD(d1Freq2)},
	{MIB_VOIP_D1ACCUR,		"D1ACCUR",		V_BYTE,	VOIP_FIELD(d1Accur)},
	{MIB_VOIP_D1LEVEL,		"D1LEVEL",		V_WORD,	VOIP_FIELD(d1Level)},
	{MIB_VOIP_D1ONUP,		"D1ONUP",		V_WORD,	VOIP_FIELD(d1ONup)},
	{MIB_VOIP_D1ONLOW,		"D1ONLOW",		V_WORD,	VOIP_FIELD(d1ONlow)},
	{MIB_VOIP_D1OFFUP,		"D1OFFUP",		V_WORD,	VOIP_FIELD(d1OFFup)},
	{MIB_VOIP_D1OFFLOW,		"D1OFFLOW",		V_WORD,	VOIP_FIELD(d1OFFlow)},
	{MIB_VOIP_D2FREQNUM,		"D2FREQNUM",		V_BYTE,	VOIP_FIELD(d2freqnum)},
	{MIB_VOIP_D2FREQ1,		"D2FREQ1",		V_WORD,	VOIP_FIELD(d2Freq1)},
	{MIB_VOIP_D2FREQ2,		"D2FREQ2",		V_WORD,	VOIP_FIELD(d2Freq2)},
	{MIB_VOIP_D2ACCUR,		"D2ACCUR",		V_BYTE,	VOIP_FIELD(d2Accur)},
	{MIB_VOIP_D2LEVEL,		"D2LEVEL",		V_WORD,	VOIP_FIELD(d2Level)},
	{MIB_VOIP_D2ONUP,		"D2ONUP",		V_WORD,	VOIP_FIELD(d2ONup)},
	{MIB_VOIP_D2ONLOW,		"D2ONLOW",		V_WORD,	VOIP_FIELD(d2ONlow)},
	{MIB_VOIP_D2OFFUP,		"D2OFFUP",		V_WORD,	VOIP_FIELD(d2OFFup)},
	{MIB_VOIP_D2OFFLOW,		"D2OFFLOW",		V_WORD,	VOIP_FIELD(d2OFFlow)},

	// ring
	{MIB_VOIP_RING_CAD,			"RING_CAD",			V_BYTE,			VOIP_FIELD(ring_cad)},
	{MIB_VOIP_RING_GROUP,		"RING_GROUP",		V_BYTE,			VOIP_FIELD(ring_group)},
	{MIB_VOIP_RING_PHONE_NUM,	"RING_PHONE_NUM",	V_UINT_LIST,	VOIP_LIST_FIELD(ring_phone_num)},
	{MIB_VOIP_RING_CADENCE_USE,	"RING_CADENCE_USE",	V_BYTE_LIST,	VOIP_LIST_FIELD(ring_cadence_use)},
	{MIB_VOIP_RING_CADENCE_SEL,	"RING_CADENCE_SEL",	V_BYTE,			VOIP_FIELD(ring_cadence_sel)},
	{MIB_VOIP_RING_CAD_ON,		"RING_CAD_ON",		V_WORD_LIST,	VOIP_LIST_FIELD(ring_cadon)},
	{MIB_VOIP_RING_CAD_OFF,		"RING_CAD_OFF",		V_WORD_LIST,	VOIP_LIST_FIELD(ring_cadoff)},

	// function key
	{MIB_VOIP_FUNCKEY_PSTN,		"FUNCKEY_PSTN",		V_STRING,	VOIP_STRING_FIELD(funckey_pstn)},
	{MIB_VOIP_FUNCKEY_TRANSFER,	"FUNCKEY_TRANSFER",	V_STRING,	VOIP_STRING_FIELD(funckey_transfer)},

	// other
	{MIB_VOIP_AUTO_DIAL,		"AUTO_DIAL",		V_WORD,	VOIP_FIELD(auto_dial)},
	{MIB_VOIP_OFF_HOOK_ALARM,	"OFF_HOOK_ALARM",	V_WORD,	VOIP_FIELD(off_hook_alarm)},
	{MIB_VOIP_CALLER_ID_AUTO_DET_SELECT,	"CALLER_ID_AUTO_DET_SELECT",	V_WORD,	VOIP_FIELD(cid_auto_det_select)},
	{MIB_VOIP_CALLER_ID_DET_MODE,	"CALLER_ID_DET_MODE",	V_WORD,	VOIP_FIELD(caller_id_det_mode)},
	{MIB_VOIP_ONE_STAGE_DIAL,	"ONE_STAGE_DIAL",	V_BYTE, VOIP_FIELD(one_stage_dial)},
	{MIB_VOIP_TWO_STAGE_DIAL,	"TWO_STAGE_DIAL",	V_BYTE, VOIP_FIELD(two_stage_dial)},
	{MIB_VOIP_AUTO_BYPASS_RELAY,	"AUTO_BYPASS_RELAY",	V_BYTE, VOIP_FIELD(auto_bypass_relay)},
	{MIB_VOIP_PULSE_DIAL_GEN,	"PULSE_DIAL_GEN",	V_BYTE,	VOIP_FIELD(pulse_dial_gen)},
	{MIB_VOIP_PULSE_GEN_PPS,	"PULSE_GEN_PPS",	V_BYTE,	VOIP_FIELD(pulse_gen_pps)},
	{MIB_VOIP_PULSE_GEN_MAKE_TIME,	"PULSE_GEN_MAKE_TIME",	V_BYTE,	VOIP_FIELD(pulse_gen_make_time)},
	{MIB_VOIP_PULSE_GEN_INTERDIGIT_PAUSE,	"PULSE_GEN_INTERDIGIT_PAUSE",	V_WORD,	VOIP_FIELD(pulse_gen_interdigit_pause)},
	{MIB_VOIP_PULSE_DIAL_DET,	"PULSE_DIAL_DET",	V_BYTE,	VOIP_FIELD(pulse_dial_det)},
	{MIB_VOIP_PULSE_DET_PAUSE,	"PULSE_DET_PAUSE",	V_WORD,	VOIP_FIELD(pulse_det_pause)},

	// auto config
	{MIB_VOIP_AUTO_CFG_VER,			"AUTO_CFG_VER",			V_WORD,		VOIP_FIELD(auto_cfg_ver)},
	{MIB_VOIP_AUTO_CFG_MODE,		"AUTO_CFG_MODE",		V_BYTE,		VOIP_FIELD(auto_cfg_mode)},
	{MIB_VOIP_AUTO_CFG_HTTP_ADDR,	"AUTO_CFG_HTTP_ADDR",	V_STRING,	VOIP_STRING_FIELD(auto_cfg_http_addr)},
	{MIB_VOIP_AUTO_CFG_HTTP_PORT,	"AUTO_CFG_HTTP_PORT",	V_WORD,		VOIP_FIELD(auto_cfg_http_port)},
/*+++++added by Jack for auto provision for tftp and ftp+++++*/
	{MIB_VOIP_AUTO_CFG_TFTP_ADDR,	"AUTO_CFG_TFTP_ADDR",	V_STRING,	VOIP_STRING_FIELD(auto_cfg_tftp_addr)},
	{MIB_VOIP_AUTO_CFG_FTP_ADDR,	"AUTO_CFG_FTP_ADDR",	V_STRING,	VOIP_STRING_FIELD(auto_cfg_ftp_addr)},
	{MIB_VOIP_AUTO_CFG_FTP_USER,	"AUTO_CFG_FTP_USER",	V_STRING,	VOIP_STRING_FIELD(auto_cfg_ftp_user)},
	{MIB_VOIP_AUTO_CFG_FTP_PASSWD,	"AUTO_CFG_FTP_PASSWD",	V_STRING,	VOIP_STRING_FIELD(auto_cfg_ftp_passwd)},
/*-----end-----*/
	{MIB_VOIP_AUTO_CFG_FILE_PATH,	"AUTO_CFG_FILE_PATH",	V_STRING,	VOIP_STRING_FIELD(auto_cfg_file_path)},
	{MIB_VOIP_AUTO_CFG_EXPIRE,		"AUTO_CFG_EXPIRE",		V_WORD,		VOIP_FIELD(auto_cfg_expire)},

	// Fw update setting
	{MIB_VOIP_FW_UPDATE_MODE,		"FW_UPDATE_MODE",		V_BYTE,		VOIP_FIELD(fw_update_mode)},
	{MIB_VOIP_FW_UPDATE_TFTP_ADDR,		"FW_UPDATE_TFTP_ADDR",		V_STRING,	VOIP_STRING_FIELD(fw_update_tftp_addr)},
	{MIB_VOIP_FW_UPDATE_HTTP_ADDR,		"FW_UPDATE_HTTP_ADDR",		V_STRING,	VOIP_STRING_FIELD(fw_update_http_addr)},
	{MIB_VOIP_FW_UPDATE_HTTP_PORT,		"FW_UPDATE_HTTP_PORT",		V_WORD,		VOIP_FIELD(fw_update_http_port)},
	{MIB_VOIP_FW_UPDATE_FTP_ADDR,		"FW_UPDATE_FTP_ADDR",		V_STRING,	VOIP_STRING_FIELD(fw_update_ftp_addr)},
	{MIB_VOIP_FW_UPDATE_FTP_USER,		"FW_UPDATE_FTP_USER",		V_STRING,	VOIP_STRING_FIELD(fw_update_ftp_user)},
	{MIB_VOIP_FW_UPDATE_FTP_PASSWD,		"FW_UPDATE_FTP_PASSWD",		V_STRING,	VOIP_STRING_FIELD(fw_update_ftp_passwd)},
	{MIB_VOIP_FW_UPDATE_FILE_PATH,		"FW_UPDATE_FILE_PATH",		V_STRING,	VOIP_STRING_FIELD(fw_update_file_path)},
	{MIB_VOIP_FW_UPDATE_POWER_ON,		"FW_UPDATE_POWER_ON",		V_BYTE,		VOIP_FIELD(fw_update_power_on)},
	{MIB_VOIP_FW_UPDATE_SCHEDULING_DAY,	"FW_UPDATE_SCHEDULING_DAY",	V_WORD,		VOIP_FIELD(fw_update_scheduling_day)},
	{MIB_VOIP_FW_UPDATE_SCHEDULING_TIME,	"FW_UPDATE_SCHEDULING_TIME",	V_BYTE,		VOIP_FIELD(fw_update_scheduling_time)},
	{MIB_VOIP_FW_UPDATE_AUTO,		"FW_UPDATE_AUTO",		V_BYTE,		VOIP_FIELD(fw_update_auto)},
	{MIB_VOIP_FW_UPDATE_FILE_PREFIX,	"FW_UPDATE_FILE_PREFIX",	V_STRING,	VOIP_STRING_FIELD(fw_update_file_prefix)},
	{MIB_VOIP_FW_UPDATE_NEXT_TIME,		"FW_UPDATE_NEXT_TIME",		V_DWORD,	VOIP_FIELD(fw_update_next_time)},
	{MIB_VOIP_FW_UPDATE_FW_VERSION,		"FW_UPDATE_FW_VERSION",		V_STRING,	VOIP_STRING_FIELD(fw_update_fw_version)},

	// VLAN setting
	{MIB_VOIP_WAN_VLAN_ENABLE,		"WAN_VLAN_ENABLE",		V_BYTE,		VOIP_FIELD(wanVlanEnable)},
	// VLAN for Voice
	{MIB_VOIP_WAN_VLAN_ID_VOICE,		"WAN_VLAN_ID_VOICE",		V_WORD,		VOIP_FIELD(wanVlanIdVoice)},
	{MIB_VOIP_WAN_VLAN_PRIORITY_VOICE,	"WAN_VLAN_PRIORITY_VOICE",	V_BYTE,		VOIP_FIELD(wanVlanPriorityVoice)},
	{MIB_VOIP_WAN_VLAN_CFI_VOICE,		"WAN_VLAN_CFI_VOICE",		V_BYTE,		VOIP_FIELD(wanVlanCfiVoice)},
	// VLAN for Data
	{MIB_VOIP_WAN_VLAN_ID_DATA,		"WAN_VLAN_ID_DATA",		V_WORD,		VOIP_FIELD(wanVlanIdData)},
	{MIB_VOIP_WAN_VLAN_PRIORITY_DATA,	"WAN_VLAN_PRIORITY_DATA",	V_BYTE,		VOIP_FIELD(wanVlanPriorityData)},
	{MIB_VOIP_WAN_VLAN_CFI_DATA,		"WAN_VLAN_CFI_DATA",		V_BYTE,		VOIP_FIELD(wanVlanCfiData)},
	// VLAN for Video
	{MIB_VOIP_WAN_VLAN_ID_VIDEO,		"WAN_VLAN_ID_VIDEO",		V_WORD,		VOIP_FIELD(wanVlanIdVideo)},
	{MIB_VOIP_WAN_VLAN_PRIORITY_VIDEO,	"WAN_VLAN_PRIORITY_VIDEO",	V_BYTE,		VOIP_FIELD(wanVlanPriorityVideo)},
	{MIB_VOIP_WAN_VLAN_CFI_VIDEO,		"WAN_VLAN_CFI_VIDEO",		V_BYTE,		VOIP_FIELD(wanVlanCfiVideo)},


#define MIB_VOIP_VLAN_ENABLE             229
#define MIB_VOIP_VLAN_TAG                230
#define MIB_VOIP_VLAN_BRIDGE_ENABLE      231
#define MIB_VOIP_VLAN_BRIDGE_TAG         232
#define MIB_VOIP_VLAN_BRIDGE_PORT        233
#define MIB_VOIP_VLAN_BRIDGE_STREAM_ENABLE 234
#define MIB_VOIP_VLAN_BRIDGE_STREAM_TAG    235
#define MIB_VOIP_HOST_ENABLE          236
#define MIB_VOIP_HOST_TAG          237
#define MIB_VOIP_HOST_PRI          238
#define MIB_VOIP_WIFI_ENABLE       239
#define MIB_VOIP_WIFI_TAG          240
#define MIB_VOIP_WIFI_PRI          241
#define MIB_VOIP_WIFI_VAP0_ENABLE  242
#define MIB_VOIP_WIFI_VAP0_TAG     243
#define MIB_VOIP_WIFI_VAP0_PRI     244
#define MIB_VOIP_WIFI_VAP1_ENABLE  245
#define MIB_VOIP_WIFI_VAP1_TAG     246
#define MIB_VOIP_WIFI_VAP1_PRI     247
#define MIB_VOIP_WIFI_VAP2_ENABLE  248
#define MIB_VOIP_WIFI_VAP2_TAG     249
#define MIB_VOIP_WIFI_VAP2_PRI     250
#define MIB_VOIP_WIFI_VAP3_ENABLE  251
#define MIB_VOIP_WIFI_VAP3_TAG     252
#define MIB_VOIP_WIFI_VAP3_PRI     253


    {MIB_VOIP_VLAN_ENABLE,              "VLAN_ENABLE",          V_BYTE,         VOIP_FIELD(vlan_enable)},
    {MIB_VOIP_VLAN_TAG,                 "VLAN_TAG",              V_WORD,         VOIP_FIELD(vlan_tag)},
    {MIB_VOIP_VLAN_BRIDGE_ENABLE,       "VLAN_BRIDGE_ENABLE",           V_BYTE,         VOIP_FIELD(vlan_bridge_enable)},
    {MIB_VOIP_VLAN_BRIDGE_TAG,          "VLAN_BRIDGE_TAG",       V_WORD,         VOIP_FIELD(vlan_bridge_tag)},
    {MIB_VOIP_VLAN_BRIDGE_PORT,         "VLAN_BRIDGE_PORT",      V_WORD,         VOIP_FIELD(vlan_bridge_port)},
    {MIB_VOIP_VLAN_BRIDGE_MULTICAST_ENABLE,"VLAN_BRIDGE_MULTICAST_ENABLE",       V_BYTE,         VOIP_FIELD(vlan_bridge_multicast_enable)},
    {MIB_VOIP_VLAN_BRIDGE_MULTICAST_TAG,   "VLAN_BRIDGE_MULTICAST_TAG",       V_WORD,         VOIP_FIELD(vlan_bridge_multicast_tag)},
	
	{MIB_VOIP_VLAN_HOST_ENABLE,        "VLAN_HOST_ENABLE",          V_BYTE,         VOIP_FIELD(vlan_host_enable)},
	{MIB_VOIP_VLAN_HOST_TAG,           "VLAN_HOST_TAG",             V_WORD, VOIP_FIELD(vlan_host_tag)},
	{MIB_VOIP_VLAN_HOST_PRI,           "VLAN_HOST_PRI",             V_WORD,  VOIP_FIELD(vlan_host_pri)},

	{MIB_VOIP_VLAN_WIFI_ENABLE,        "VLAN_WIFI_ENABLE",          V_BYTE,         VOIP_FIELD(vlan_wifi_enable)},
	{MIB_VOIP_VLAN_WIFI_TAG,           "VLAN_WIFI_TAG",             V_WORD, VOIP_FIELD(vlan_wifi_tag)},
	{MIB_VOIP_VLAN_WIFI_PRI,           "VLAN_WIFI_PRI",             V_WORD,  VOIP_FIELD(vlan_wifi_pri)},

	{MIB_VOIP_VLAN_WIFI_VAP0_ENABLE,        "VLAN_WIFI_VAP0_ENABLE",          V_BYTE, VOIP_FIELD(vlan_wifi_vap0_enable)},
	{MIB_VOIP_VLAN_WIFI_VAP0_TAG,           "VLAN_WIFI_VAP0_TAG",             V_WORD, VOIP_FIELD(vlan_wifi_vap0_tag)},
	{MIB_VOIP_VLAN_WIFI_VAP0_PRI,           "VLAN_WIFI_VAP0_PRI",             V_WORD, VOIP_FIELD(vlan_wifi_vap0_pri)},
	{MIB_VOIP_VLAN_WIFI_VAP1_ENABLE,        "VLAN_WIFI_VAP1_ENABLE",          V_BYTE, VOIP_FIELD(vlan_wifi_vap1_enable)},
	{MIB_VOIP_VLAN_WIFI_VAP1_TAG,           "VLAN_WIFI_VAP1_TAG",             V_WORD, VOIP_FIELD(vlan_wifi_vap1_tag)},
	{MIB_VOIP_VLAN_WIFI_VAP1_PRI,           "VLAN_WIFI_VAP1_PRI",             V_WORD, VOIP_FIELD(vlan_wifi_vap1_pri)},
	{MIB_VOIP_VLAN_WIFI_VAP2_ENABLE,        "VLAN_WIFI_VAP2_ENABLE",          V_BYTE, VOIP_FIELD(vlan_wifi_vap2_enable)},
	{MIB_VOIP_VLAN_WIFI_VAP2_TAG,           "VLAN_WIFI_VAP2_TAG",             V_WORD, VOIP_FIELD(vlan_wifi_vap2_tag)},
	{MIB_VOIP_VLAN_WIFI_VAP2_PRI,           "VLAN_WIFI_VAP2_PRI",             V_WORD, VOIP_FIELD(vlan_wifi_vap2_pri)},
	{MIB_VOIP_VLAN_WIFI_VAP3_ENABLE,        "VLAN_WIFI_VAP3_ENABLE",          V_BYTE, VOIP_FIELD(vlan_wifi_vap3_enable)},
	{MIB_VOIP_VLAN_WIFI_VAP3_TAG,           "VLAN_WIFI_VAP3_TAG",             V_WORD, VOIP_FIELD(vlan_wifi_vap3_tag)},
	{MIB_VOIP_VLAN_WIFI_VAP3_PRI,           "VLAN_WIFI_VAP3_PRI",             V_WORD, VOIP_FIELD(vlan_wifi_vap3_pri)},


	// HWNAT enable
	{MIB_VOIP_HWNAT_ENABLE,			"HWNAT_ENABLE",			V_BYTE,		VOIP_FIELD(hwnat_enable)},

	//Port Bandwidth Mgr
	{MIB_VOIP_BANDWIDTH_LANPORT0_EGRESS,	"BANDWIDTH_LANPORT0_EGRES",	V_WORD,		VOIP_FIELD(bandwidth_LANPort0_Egress)},
	{MIB_VOIP_BANDWIDTH_LANPORT1_EGRESS,	"BANDWIDTH_LANPORT1_EGRES",	V_WORD,		VOIP_FIELD(bandwidth_LANPort1_Egress)},
	{MIB_VOIP_BANDWIDTH_LANPORT2_EGRESS,	"BANDWIDTH_LANPORT2_EGRES",	V_WORD,		VOIP_FIELD(bandwidth_LANPort2_Egress)},
	{MIB_VOIP_BANDWIDTH_LANPORT3_EGRESS,	"BANDWIDTH_LANPORT3_EGRES",	V_WORD,		VOIP_FIELD(bandwidth_LANPort3_Egress)},
	{MIB_VOIP_BANDWIDTH_WANPORT_EGRESS,	"BANDWIDTH_WANPORT_EGRES",	V_WORD,		VOIP_FIELD(bandwidth_WANPort_Egress)},
	{MIB_VOIP_BANDWIDTH_LANPORT0_INGRESS,	"BANDWIDTH_LANPORT0_INGRES",	V_WORD,		VOIP_FIELD(bandwidth_LANPort0_Ingress)},
	{MIB_VOIP_BANDWIDTH_LANPORT1_INGRESS,	"BANDWIDTH_LANPORT1_INGRES",	V_WORD,		VOIP_FIELD(bandwidth_LANPort1_Ingress)},
	{MIB_VOIP_BANDWIDTH_LANPORT2_INGRESS,	"BANDWIDTH_LANPORT2_INGRES",	V_WORD,		VOIP_FIELD(bandwidth_LANPort2_Ingress)},
	{MIB_VOIP_BANDWIDTH_LANPORT3_INGRESS,	"BANDWIDTH_LANPORT3_INGRES",	V_WORD,		VOIP_FIELD(bandwidth_LANPort3_Ingress)},
	{MIB_VOIP_BANDWIDTH_WANPORT_INGRESS,	"BANDWIDTH_WANPORT_INGRES",	V_WORD,		VOIP_FIELD(bandwidth_WANPort_Ingress)},

	// FXO volume
	{MIB_VOIP_DAA_TX_VOLUME,	"DAA_TX_VOLUME",	V_BYTE,	VOIP_FIELD(daa_txVolumne)},
	{MIB_VOIP_DAA_RX_VOLUME,	"DAA_RX_VOLUME",	V_BYTE,	VOIP_FIELD(daa_rxVolumne)},

	// DSCP
	{MIB_VOIP_RTP_DSCP,		"RTP_DSCP",		V_BYTE,		VOIP_FIELD(rtpDscp)},
	{MIB_VOIP_SIP_DSCP,		"SIP_DSCP",		V_BYTE,		VOIP_FIELD(sipDscp)},

	// port setting
	{MIB_VOIP_PORT,	"PORT",	V_MIB_LIST,	VOIP_MIB_LIST_FIELD(ports, mibtbl_voip_port)},

	{0}
};

static int voip_mibname_check(const char *mib_name)
{
	int i, j;
	char mib_name_reserved[] = { // characters except alphanumber
		'-', '_', 
		0
	};

	for (i=0; mib_name[i]; i++)
	{
		// allow alpha only if first char
		if (i == 0 && !isalpha(mib_name[i]))
			return -1;

		if (!isalnum(mib_name[i]))
		{
			for (j=0; j<mib_name_reserved[j]; j++)
			{
				if (mib_name[i] == mib_name_reserved[j])
					break;
			}

			if (mib_name_reserved[j] == 0)
			{
				return -1;	// not reserved character
			}
		}
	}

	return 0;
}

static int _voip_mibtbl_check(const voipMibEntry_T *mib_tbl, int *size);

static int voip_mibentry_check(const voipMibEntry_T *mib)
{
	int size, mib_size;

	if (mib->type < V_INT || mib->type >= V_END)
	{
		fprintf(stderr, "VoIP MIB Error: unknown type %d in %s\n", 
			mib->type, mib->name);
		return -1;
	}

	if (mib->type >= V_INT && mib->type < V_STRING)	
	{
		// not Array/List type
		if (mib->entry_size)
		{
			fprintf(stderr, "VoIP MIB (%s, %d, %d) Error: entry size should be zero\n",
				mib->name, mib->size, mib->entry_size);
			return -1;
		}

		mib_size = mib->size;
	}
	else
	{
		// Array/List type
		if (mib->entry_size == 0)
		{
			fprintf(stderr, "VoIP MIB (%s, %d, %d) Error: entry size should not be zero\n",
				mib->name, mib->size, mib->entry_size);
			return -1;
		}

		mib_size = mib->entry_size;
	}

	if (mib->type == V_MIB || mib->type == V_MIB_LIST)
	{
		// get variable size from mib_tbl
		if (_voip_mibtbl_check(mib->mib_tbl, &size) != 0)
		{
			fprintf(stderr, "VoIP MIB (%s, %d, %d) Error: check mib_tbl failed\n",
				mib->name, mib->size, mib->entry_size);
			return -1;
		}
	}
	else 
	{
		// fixed size
		size = v_types[mib->type].size;
	}

	if (mib_size != size)
	{
		fprintf(stderr, "VoIP MIB (%s, %d, %d) Error: size not match %d\n",
			mib->name, mib->size, mib->entry_size, size);
		return -1;
	}

	if (voip_mibname_check(mib->name) != 0)
	{
		fprintf(stderr, "VoIP MIB (%s, %d, %d) Error: mib name is incorrect\n",
			mib->name, mib->size, mib->entry_size);
		return -1;
	}

	return 0;
}

static int _voip_mibtbl_check(const voipMibEntry_T *mib_tbl, int *size)
{
	int i, j;

	*size = 0;
	for (i=0; mib_tbl[i].id; i++)
	{
		if (voip_mibentry_check(&mib_tbl[i]) != 0)
			return -1;

		*size += mib_tbl[i].size;
		// check duplicate mib id or duplicate mib name
		for (j=i+1; mib_tbl[j].id; j++)
		{
			if (mib_tbl[i].id == mib_tbl[j].id)
			{
				fprintf(stderr, "VoIP MIB Error: %s detect duplicate id in %s\n",
					mib_tbl[i].name, mib_tbl[j].name);
				return -1;
			}

			if (strcmp(mib_tbl[i].name, mib_tbl[j].name) == 0)
			{
				fprintf(stderr, "VoIP MIB Error: detect duplicate name in id%d and id%d\n",
					mib_tbl[i].id, mib_tbl[j].id);
				return -1;
			}
		}
	}

	return 0;
}

int voip_mibtbl_check(int *size)
{
	int i;

	for (i=V_INT; i<V_END; i++)
	{
		// check type size
		if (((i == V_MIB || i == V_MIB_LIST) && v_types[i].size) ||
			((i != V_MIB && i != V_MIB_LIST) && v_types[i].size == 0))
			break;
	}

	if (i != V_END)
	{
		fprintf(stderr, "VoIP MIB Type (%s, %d) Error: size not match\n",
			v_types[i].name, v_types[i].size);
		return -1;
	}

	return _voip_mibtbl_check(mibtbl_voip, size);
}

int bin2hex(const unsigned char *bin, char *hex, const int len)
{
	int i, idx;
	char hex_char[] = "0123456789ABCDEF";

	for (i=0, idx=0; i<len; i++)
	{
		hex[idx++] = hex_char[(bin[i] & 0xf0) >> 4];
		hex[idx++] = hex_char[bin[i] & 0x0f];
	}

	hex[idx] = 0;
	return 0;
}

int hex2bin(const char *hex, unsigned char *bin, const int len)
{
	int i, idx;
	unsigned char bytes[2];

	for (i=0, idx=0; hex[i]; i++)
	{
		if (hex[i & 0x01] == 0)
			return -1; // hex length != even

		if (hex[i] >= '0' && hex[i] <= '9')
			bytes[i & 0x01] = hex[i] - '0';
		else if (hex[i] >= 'A' && hex[i] <= 'F')
			bytes[i & 0x01] = hex[i] - 'A' + 10;
		else if (hex[i] >= 'a' && hex[i] <= 'f')
			bytes[i & 0x01] = hex[i] - 'a' + 10;
		else
			return -1; // not hex

		if (i & 0x01)
		{
			if (idx >= len)
				return -1; // out of size

			bin[idx++] = (bytes[0] << 4) | bytes[1];
		}
	}

	return 0;
}

#define mac2str(mac, str) bin2hex((mac), (str), 6)
#define str2mac(str, mac) hex2bin((str), (mac), 6)

int voip_mibline_to(char *line, const char *name, const char *value)
{
	sprintf(line, "%s=%s\n", name, value);
	return 0;
}

int voip_mibline_from(const char *line, char *name, char *value)
{
	int i, idx;
	int mode;

	if (line[0] == ';')
	{
		name[0] = 0; // comment line
		return VOIP_CURRENT_SETTING;	
	}

	// parse name
	for (i=0; line[i]!='=' && line[i] && line[i]!='\n' && line[i]!='\r'; i++);

	mode = VOIP_NONE_SETTING;
	if (i && line[i] == '=')
	{
		if (strncmp(line, "VOIP.", 5) == 0)
		{
			memcpy(name, &line[5], i - 5);
			name[i - 5] = 0;
			mode = VOIP_CURRENT_SETTING;
		}
		else if (strncmp(line, "DEF_VOIP.", 9) == 0)
		{
			memcpy(name, &line[9], i - 9);
			name[i - 9] = 0;
			mode = VOIP_DEFAULT_SETTING;
		}
		else
		{
			return VOIP_NONE_SETTING;
		}
	}
	else
	{
		// couldn't find mib name
		return VOIP_NONE_SETTING;
	}

	// parse value
	i++;
	for (idx = 0; line[i] && line[i]!='\n' && line[i]!='\r'; i++)
		value[idx++] = line[i];
	value[idx] = 0;

	return mode;
}

static int voip_mibentry_to(const voipMibEntry_T *mib, const void *data, char *to)
{
	char *pChar;
	short vShort;
	int vInt;
	char *pBuf;
	char mac_str[13];
	char ip_str[32];

	switch (mib->type)
	{
	case V_INT:
	case V_LONG:
	case V_INT_LIST:
	case V_LONG_LIST:
		memcpy(&vInt, data, sizeof(vInt));
		sprintf(to, "%d", vInt);
		break;
	case V_UINT:
	case V_DWORD:
	case V_UINT_LIST:
	case V_DWORD_LIST:
		memcpy(&vInt, data, sizeof(vInt));
		sprintf(to, "%u", (unsigned int) vInt);
		break;
	case V_CHAR:
	case V_CHAR_LIST:
		pChar = (char *) data;
		sprintf(to, "%d", *pChar);
		break;
	case V_BYTE:
	case V_BYTE_LIST:
		pChar = (char *) data;
		sprintf(to, "%u", (unsigned char) *pChar);
		break;
	case V_SHORT:
	case V_SHORT_LIST:
		memcpy(&vShort, data, sizeof(vShort));
		sprintf(to, "%d", vShort);
		break;
	case V_WORD:
	case V_WORD_LIST:
		memcpy(&vShort, data, sizeof(vShort));
		sprintf(to, "%u", (unsigned short) vShort);
		break;
	case V_IP4:
	case V_IP4_LIST:
		if (inet_ntop(AF_INET, data, ip_str, sizeof(ip_str)) == NULL)
		{
			fprintf(stderr, "VoIP MIB Convert Error: inet_ntop failed.\n");
			return -1;
		}
		sprintf(to, "%s", ip_str);
		break;
	case V_MAC6:
	case V_MAC6_LIST:
		pChar = (char *) data;
		mac2str(pChar, mac_str);
		sprintf(to, "%s", mac_str);
		break;
	case V_STRING:
		pChar = (char *) data;
		sprintf(to, "%s", pChar);
		break;
	case V_BYTE_ARRAY:
		pChar = (char *) data;
		pBuf = malloc(mib->size * 2 + 1);
		if (pBuf == NULL)
		{
			fprintf(stderr, "VoIP MIB Convert Error: malloc %d failed\n", mib->size * 2 + 1);
			return -1;
		}
		bin2hex(pChar, pBuf, mib->size);
		sprintf(to, "%s", pBuf);
		free(pBuf);
		break;
	default:
		fprintf(stderr, "VoIP MIB Convert Error: Type %s is not support\n", 
			v_types[mib->type].name);
		return -1;
	}

	return 0;

#if 0
voip_mibentry_to_failed:
	fprintf(stderr, "VoIP MIB Convert Error: %s convert type %s to string failed\n", 
		mib->name, v_types[mib->type].name);
	return -1;
#endif	
}

static int voip_mibentry_from(const voipMibEntry_T *mib, void *data, const char *from)
{
	char *pChar;
	short vShort;
	int vInt;

	switch (mib->type)
	{
	case V_INT:
	case V_LONG:
	case V_INT_LIST:
	case V_LONG_LIST:
		if (sscanf(from, "%d", &vInt) != 1)
			goto voip_mibentry_from_failed;
		memcpy(data, &vInt, sizeof(vInt));
		break;
	case V_UINT:
	case V_DWORD:
	case V_UINT_LIST:
	case V_DWORD_LIST:
		if (sscanf(from, "%u", &vInt) != 1)
			goto voip_mibentry_from_failed;
		memcpy(data, &vInt, sizeof(vInt));
		break;
	case V_CHAR:
	case V_CHAR_LIST:
		if (sscanf(from, "%d", &vInt) != 1)
			goto voip_mibentry_from_failed;
		*(char *) data = (char) vInt;
		break;
	case V_BYTE:
	case V_BYTE_LIST:
		if (sscanf(from, "%u", &vInt) != 1)
			goto voip_mibentry_from_failed;
		*(char *) data = (char) vInt;
		break;
	case V_SHORT:
	case V_SHORT_LIST:
		if (sscanf(from, "%d", &vInt) != 1)
			goto voip_mibentry_from_failed;
		vShort = (short) vInt; // avoid endian issue
		memcpy(data, &vShort, sizeof(vShort)); // avoid alignment issue
		break;
	case V_WORD:
	case V_WORD_LIST:
		if (sscanf(from, "%u", &vInt) != 1)
			goto voip_mibentry_from_failed;
		vShort = (short) vInt; // avoid endian issue
		memcpy(data, &vShort, sizeof(vShort)); // avoid alignment issue
		break;
	case V_IP4:
	case V_IP4_LIST:
		if (inet_pton(AF_INET, from, data) <= 0)
			goto voip_mibentry_from_failed;
		break;
	case V_MAC6:
	case V_MAC6_LIST:
		pChar = (char *) data;
		if (str2mac(from, pChar) != 0)
			goto voip_mibentry_from_failed;
		break;
	case V_STRING:
		pChar = (char *) data;
		// Rock:
		// 	- skip length too long error
		// 	- ensure null terminated, 
		strncpy(pChar, from, mib->size - 1);
		pChar[mib->size - 1] = 0; 
		break;
	case V_BYTE_ARRAY:
		pChar = (char *) data;
		if (hex2bin(from, pChar, mib->size) != 0)
			goto voip_mibentry_from_failed;
		break;
	default:
		fprintf(stderr, "VoIP MIB Convert Error: Type %s is not support\n", 
			v_types[mib->type].name);
		return -1;
	}

	return 0;

voip_mibentry_from_failed:
	fprintf(stderr, "VoIP MIB Convert Error: %s can't be convert to %s\n", 
		from, v_types[mib->type].name);
	return -1;
}

static int _voip_mib_from(const voipMibEntry_T *mib_tbl, const char *name, 
	const voipMibEntry_T **ppmib, int *offset)
{
	int i, j, idx;
	char *ptr;
	const voipMibEntry_T *mib;

	j = 0;
	for (i=0; mib_tbl[i].id; i++)
	{
		for (j=0; mib_tbl[i].name[j]; j++)
			if (name[j] != mib_tbl[i].name[j])
				break;
		
		if ((mib_tbl[i].name[j] == 0) &&
			(name[j] == 0 || name[j] == '[' || name[j] == '.'))
			break; // found
	}

	if (mib_tbl[i].id == 0)
		return -1; // mib name not found 

	mib = &mib_tbl[i];
	if (name[j] == '[')
	{
		if (mib->type < V_INT_LIST || mib->type > V_MIB_LIST)
			return -1;	// have to xxxLIST type

		j++;
		for (i=j; name[j]!=']' && name[j] && name[j]!= '.'; j++);

		if (name[j] != ']')
			return -1;	// couldn't find match ']'

		idx = strtol(&name[i], &ptr, 10);
		if (ptr[0] != ']')
			return -1; // parse LIST index failed

		if (idx < 0 || idx >= (mib->size / mib->entry_size))
			return -1; // index out of range

		j++;
		if (name[j] != 0 && name[j] != '.')
			return -1; // not [%d]. or [%d] format

		*offset += mib->offset + mib->entry_size * idx;
	}
	else
	{
		if (mib->type >= V_INT_LIST && mib->type <= V_MIB_LIST)
			return -1;	// have not to xxxLIST type

		*offset += mib->offset;
	}
	
	if (name[j] == '.')
	{
		if (mib->type != V_MIB &&
			mib->type != V_MIB_LIST)
			return -1;  // Type must be V_MIBxxx if "." exist

		return _voip_mib_from(mib->mib_tbl, &name[j + 1], ppmib, offset);
	}
	else
	{
		if (mib->type == V_MIB ||
			mib->type == V_MIB_LIST)
			return -1;  // Type must not be V_MIBxxx if "." not exist

		*ppmib = mib;
		return 0;
	}
}

int voip_mib_from(const voipMibEntry_T *mib_tbl, const char *name, 
	const voipMibEntry_T **ppmib, int *offset)
{
	*offset = 0;
	return _voip_mib_from(mib_tbl, name, ppmib, offset);
}

int voip_mib_read(void *data, const char *name, const char *value)
{
	const voipMibEntry_T *mib;
	int offset;

	// get mib and data offset via name
	if (voip_mib_from(mibtbl_voip, name, &mib, &offset) != 0)
	{
		fprintf(stderr, "VoIP MIB Read Error: couldn't find %s in mib table\n", name);
		return -1;
	}

	// write value by mib and data offset
	if (voip_mibentry_from(mib, (void *)((int) data + offset), value) != 0)
	{
		fprintf(stderr, "VoIP MIB Read Error: read %s failed in mib %s\n", value, name);
		return -1;
	}

	return 0;
}

int voip_mibtbl_read_line(voipCfgAll_t *cfg_all, char *line)
{
	int mode;
	char name[200];
	char value[200];

	// get name & value
	mode = voip_mibline_from(line, name, value);
	if (mode == VOIP_CURRENT_SETTING)
	{
		if (name[0] == 0) return 0; // comment line
		if (voip_mib_read(&cfg_all->current_setting, name, value) == 0)
		{
			cfg_all->mode |= VOIP_CURRENT_SETTING;
			return 0;
		}
	}
	else if (mode == VOIP_DEFAULT_SETTING)
	{
		if (voip_mib_read(&cfg_all->default_setting, name, value) == 0)
		{
			cfg_all->mode |= VOIP_DEFAULT_SETTING;
			return 0;
		}
	}

	fprintf(stderr, "VoIP MIB Read Error: parse \"%s\" failed\n", line);
	return -1;
}

static int voip_mib_to(const voipMibEntry_T *mib, const void *data, const char *name, 
	const int offset, FILE *fp)
{
	char line[600];
	char value[200];

	if (voip_mibentry_to(mib, (void *)((int) data + offset), value) != 0)
	{
		fprintf(stderr, "VoIP MIB Write Error: write failed in mib %s\n", name);
		return -1;
	}

	if (voip_mibline_to(line, name, value) != 0)
	{
		fprintf(stderr, "VoIP MIB Write Error: write %s failed in %s\n", value, name);
		return -1;
	}

	fputs(line, fp);
	return 0;
}

static int _voip_mibtbl_write(const voipMibEntry_T *mib_tbl, const void *data, 
	char *prefix, char *name, const int offset, FILE *fp)
{
	int i, j;
	const voipMibEntry_T *mib;
	char new_prefix[200];
	int new_offset;

	for (i=0; mib_tbl[i].id; i++)
	{
		mib = &mib_tbl[i];
		new_offset = offset + mib->offset;
		if (mib->type >= V_INT_LIST && mib->type <= V_MIB_LIST)
		{
			for (j=0; j<(mib->size / mib->entry_size); j++)
			{
				sprintf(name, "%s.%s[%d]", prefix, mib->name, j);
				if (mib->type == V_MIB_LIST)
				{
					strcpy(new_prefix, name);
					_voip_mibtbl_write(mib->mib_tbl, data, new_prefix, name, new_offset, fp);
				}
				else
				{
					voip_mib_to(mib, data, name, new_offset, fp);
				}

				new_offset += mib->entry_size;
			}
		}
		else if (mib->type == V_MIB)
		{
			sprintf(name, "%s.%s", prefix, mib->name);
			strcpy(new_prefix, name);
			_voip_mibtbl_write(mib->mib_tbl, data, new_prefix, name, new_offset, fp);
		}
		else
		{
			sprintf(name, "%s.%s", prefix, mib->name);
			voip_mib_to(mib, data, name, new_offset, fp);
		}
	}

	return 0;
}

int voip_mibtbl_write(const void *data, int fd, int mode)
{
	FILE *fp;
	char name[200];
	int err;

	fp = fdopen(dup(fd), "w");
	if (fp == NULL)
	{
		fprintf(stderr, "VoIP MIB Dump Error: fdopen failed\n");
		return -1;
	}

	err = 0;
	if (mode == VOIP_CURRENT_SETTING)
		err = _voip_mibtbl_write(mibtbl_voip, data, "VOIP", name, 0, fp);
	else if (mode == VOIP_DEFAULT_SETTING)
		err = _voip_mibtbl_write(mibtbl_voip, data, "DEF_VOIP", name, 0, fp);

	fclose(fp);
	return err;
}

int voip_mibtbl_get(const char *mib_name, voipCfgAll_t *cfg_all, int fd)
{
	FILE *fp;
	const voipMibEntry_T *mib;
	int offset;
	const char *pname;
	void *data;

	fp = fdopen(dup(fd), "w");
	if (fp == NULL)
	{
		fprintf(stderr, "VoIP MIB Get Error: fdopen failed\n");
		return -1;
	}

	if ((cfg_all->mode & VOIP_CURRENT_SETTING) && 
		(strncmp(mib_name, "VOIP.", 5) == 0))
	{
		pname = &mib_name[5];
		data = &cfg_all->current_setting;
	}
	else if ((cfg_all->mode & VOIP_DEFAULT_SETTING) && 
		(strncmp(mib_name, "DEF_VOIP.", 9) == 0))
	{
		pname = &mib_name[9];
		data = &cfg_all->default_setting;
	}
	else
	{
		pname = NULL;
		data = NULL;
	}

	// get mib entry and flash data offset by mib name
	offset = 0;
	if ((data == NULL) || (voip_mib_from(mibtbl_voip, pname, &mib, &offset) != 0))
	{
		fprintf(stderr, "VoIP MIB Get Error: couldn't find %s in mib table\n", mib_name);
		fclose(fp);
		return -1;
	}

	// output mib data by mib entry and flash data 
	if (voip_mib_to(mib, data, mib_name, offset, fp) != 0)
	{
		fprintf(stderr, "VoIP MIB Get Error: convert %s failed\n", mib_name);
		fclose(fp);
		return -1;
	}

	fclose(fp);
	return 0;
}

int voip_mibtbl_set(const char *mib_name, const char *mib_value, voipCfgAll_t *cfg_all, int fd)
{
	FILE *fp;
	const voipMibEntry_T *mib;
	int offset;
	const char *pname;
	void *data;

	fp = fdopen(dup(fd), "w");
	if (fp == NULL)
	{
		fprintf(stderr, "VoIP MIB Set Error: fdopen failed\n");
		return -1;
	}

	if ((cfg_all->mode & VOIP_CURRENT_SETTING) && 
		(strncmp(mib_name, "VOIP.", 5) == 0))
	{
		pname = &mib_name[5];
		data = &cfg_all->current_setting;
	}
	else if ((cfg_all->mode & VOIP_DEFAULT_SETTING) && 
		(strncmp(mib_name, "DEF_VOIP.", 9) == 0))
	{
		pname = &mib_name[9];
		data = &cfg_all->default_setting;
	}
	else
	{
		pname = NULL;
		data = NULL;
	}

	// get mib entry and flash data offset by mib name
	offset = 0;
	if ((data == NULL) || (voip_mib_from(mibtbl_voip, pname, &mib, &offset) != 0))
	{
		fprintf(stderr, "VoIP MIB Set Error: couldn't find %s in mib table\n", mib_name);
		fclose(fp);
		return -1;
	}

	// set mib data by mib, value and data offset
	if (voip_mibentry_from(mib, (void *)((int) data + offset), mib_value) != 0)
	{
		fprintf(stderr, "VoIP MIB Set Error: write %s failed in mib %s\n", mib_value, mib_name);
		fclose(fp);
		return -1;
	}

	fclose(fp);
	return 0;
}

int voip_mib_swap_value(const voipMibEntry_T *mib, void *data)
{
	short *pShort;
	int *pInt;

	switch (mib->type)
	{
	case V_INT:
	case V_LONG:
	case V_INT_LIST:
	case V_LONG_LIST:
	case V_UINT:
	case V_DWORD:
	case V_UINT_LIST:
	case V_DWORD_LIST:
		pInt = (int *) data;
		*pInt = htonl(*pInt);
		break;
	case V_SHORT:
	case V_SHORT_LIST:
	case V_WORD:
	case V_WORD_LIST:
		pShort = (short *) data;
		*pShort = htons(*pShort);
		break;
	}

	return 0;
}

int _voip_mibtbl_swap_value(const voipMibEntry_T *mib_tbl, void *data, int offset)
{
	int i, j;
	const voipMibEntry_T *mib;
	int new_offset;

	for (i=0; mib_tbl[i].id; i++)
	{
		mib = &mib_tbl[i];
		new_offset = offset + mib->offset;
		if (mib->type >= V_INT_LIST && mib->type <= V_MIB_LIST)
		{
			for (j=0; j<(mib->size / mib->entry_size); j++)
			{
				if (mib->type == V_MIB_LIST)
				{
					_voip_mibtbl_swap_value(mib->mib_tbl, data, new_offset);
				}
				else
				{
					voip_mib_swap_value(mib, (void *)((int) data + new_offset));
				}
				new_offset += mib->entry_size;
			}
		}
		else if (mib->type == V_MIB)
		{
			_voip_mibtbl_swap_value(mib->mib_tbl, data, new_offset);
		}
		else
		{
			voip_mib_swap_value(mib, (void *)((int) data + new_offset));
		}
	}

	return 0;
}

int voip_mibtbl_swap_value(void *data)
{
	return _voip_mibtbl_swap_value(mibtbl_voip, data, 0);
}

