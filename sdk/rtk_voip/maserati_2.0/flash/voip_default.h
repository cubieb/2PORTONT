#include "rcm_customize.h"

#ifdef CONFIG_RTK_VOIP_G729AB
#define G729AB_DEF_FRAME_SIZE	( 2 - 1 ),
#else
#define G729AB_DEF_FRAME_SIZE
#endif

#ifdef CONFIG_RTK_VOIP_G7231
#define G723_DEF_FRAME_SIZE		( 1 - 1 ),
#else
#define G723_DEF_FRAME_SIZE
#endif

#ifdef CONFIG_RTK_VOIP_G726
#define G726_16_DEF_FRAME_SIZE	( 2 - 1 ),
#define G726_24_DEF_FRAME_SIZE	( 2 - 1 ),
#define G726_32_DEF_FRAME_SIZE	( 2 - 1 ),
#define G726_40_DEF_FRAME_SIZE	( 2 - 1 ),
#else
#define G726_16_DEF_FRAME_SIZE
#define G726_24_DEF_FRAME_SIZE
#define G726_32_DEF_FRAME_SIZE
#define G726_40_DEF_FRAME_SIZE
#endif

#ifdef CONFIG_RTK_VOIP_GSMFR
#define GSMFR_DEF_FRAME_SIZE	( 1 - 1 ),
#else
#define GSMFR_DEF_FRAME_SIZE
#endif

#ifdef CONFIG_RTK_VOIP_ILBC
#define ILBC_DEF_FRAME_SIZE		( 1 - 1 ),
#else
#define ILBC_DEF_FRAME_SIZE
#endif

#ifdef CONFIG_RTK_VOIP_G722
#define G722_DEF_FRAME_SIZE		( 2 - 1 ),
#else
#define G722_DEF_FRAME_SIZE
#endif

#ifdef CONFIG_RTK_VOIP_SPEEX_NB
#define SPEEX_NB_DEF_FRAME_SIZE		( 1 - 1 ),
#else
#define SPEEX_NB_DEF_FRAME_SIZE
#endif

#ifdef CONFIG_RTK_VOIP_G7111
#define G711U_WB_DEF_FRAME_SIZE		( 1 - 1 ),
#define G711A_WB_DEF_FRAME_SIZE		( 1 - 1 ),
#else
#define G711U_WB_DEF_FRAME_SIZE
#define G711A_WB_DEF_FRAME_SIZE
#endif

#define	M_DEFAULT_FRAME_SIZE_LIST								\
	( 2 - 1 ),				/* SUPPORTED_CODEC_G711U = 0, */	\
	( 2 - 1 ),				/* SUPPORTED_CODEC_G711A, */		\
	G729AB_DEF_FRAME_SIZE	/* SUPPORTED_CODEC_G729, */			\
	G723_DEF_FRAME_SIZE		/* SUPPORTED_CODEC_G723, */			\
	G726_16_DEF_FRAME_SIZE	/* SUPPORTED_CODEC_G726_16, */		\
	G726_24_DEF_FRAME_SIZE	/* SUPPORTED_CODEC_G726_24, */		\
	G726_32_DEF_FRAME_SIZE	/* SUPPORTED_CODEC_G726_32, */		\
	G726_40_DEF_FRAME_SIZE	/* SUPPORTED_CODEC_G726_40, */		\
	GSMFR_DEF_FRAME_SIZE	/* SUPPORTED_CODEC_GSMFR, */		\
	ILBC_DEF_FRAME_SIZE		/* SUPPORTED_CODEC_ILBC, */			\
	G722_DEF_FRAME_SIZE		/* SUPPORTED_CODEC_G722, */			\
	SPEEX_NB_DEF_FRAME_SIZE	/* SUPPORTED_CODEC_SPEEX_NB, */		\
	G711U_WB_DEF_FRAME_SIZE /* SUPPORTED_CODEC_G711U_WB, */		\
	G711A_WB_DEF_FRAME_SIZE /* SUPPORTED_CODEC_G711A_WB, */		\



#define DEFAULT_TONE_CUSTOMER_1			\
	{0, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, 7, 7, 0, 0}

#define DEFAULT_TONE_CUSTOMER_2			\
	{0, 0, 2, 2000, 4000, 2000, 4000, 0, 0, 0, 0, 0, 2, 440, 480, 0, 0, 7, 7, 0, 0}

#define DEFAULT_TONE_CUSTOMER_3			\
	{0, 0, 2, 500, 500, 500, 500, 0, 0, 0, 0, 0, 2, 480, 620, 0, 0, 7, 7, 0, 0}

#define DEFAULT_TONE_CUSTOMER_4			\
	{0, 0, 3, 500, 500, 500, 500, 500, 8500, 0, 0, 0, 1, 440, 0, 0, 0, 7, 0, 0, 0}

#define DEFAULT_TONE_CUSTOMER_5			\
	{0, 0, 2, 100, 100, 100, 100, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, 7, 7, 0, 0}

#define DEFAULT_TONE_CUSTOMER_6			\
	{0, 0, 3, 100, 100, 100, 100, 100, 100, 0, 0, 0, 2, 350, 440, 0, 0, 7, 7, 0, 0}

#define DEFAULT_TONE_CUSTOMER_7			\
	{0, 0, 1, 240, 260, 0, 0, 0, 0, 0, 0, 0, 2, 480, 620, 0, 0, 7, 7, 0, 0}

#define DEFAULT_TONE_CUSTOMER_8			\
	{0, 0, 2, 1000, 1000, 1000, 1000, 0, 0, 0, 0, 0, 2, 1400, 2600, 0, 0, 7, 7, 0, 0}

voipCfgParam_t voipCfgParamDefault =
{
	VOIP_FLASH_SIGNATURE,
	VOIP_FLASH_VER,
	0/*VOIP_FLASH_FEATURE*/,		// use value of voip_system_feature_get()
	0/*VOIP_FLASH_EXT_FEATURE*/,	// use value of voip_system_feature_get()
	VOIP_FLASH_MIB_VER,
	0,					// rfc_flags
	TONE_TW,			// tone_of_country 
	TONE_CUSTOMER_1,	// tone_of_custdial
	TONE_CUSTOMER_2,	// tone_of_custring
	TONE_CUSTOMER_3,	// tone_of_custbusy
	TONE_CUSTOMER_4,	// tone_of_custwaiting
	TONE_CUSTOMER_1,	// tone_of_customize
	{DEFAULT_TONE_CUSTOMER_1, DEFAULT_TONE_CUSTOMER_2, DEFAULT_TONE_CUSTOMER_3, DEFAULT_TONE_CUSTOMER_4,
	 DEFAULT_TONE_CUSTOMER_5, DEFAULT_TONE_CUSTOMER_6, DEFAULT_TONE_CUSTOMER_7, DEFAULT_TONE_CUSTOMER_8},	// cust_tone_para
	DIS_CONNECT_TONE_1,	// disconnect tone detect number
	DIS_CONNECT_TONE_FREQ_2,// disconnect tone 1 parm
	480,
	620,
	16,
	800,
	29,
	21,
	29,
	21,
	DIS_CONNECT_TONE_FREQ_1,// disconnect tone 2 parm
	450,
	0,
	16,
	800,
	39,
	31,
	39,
	31,
	8,					// ring_cad
	RING_GROUP_1,		// ring_group
	{0},				// ring_phone_num
	{RING_CADENCE_1, RING_CADENCE_1, RING_CADENCE_1, RING_CADENCE_1},	// ring_cadence_use
	RING_CADENCE_1,		// ring_cadence_sel
	{2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000},	// ring_cadon
	{4000, 4000, 4000, 4000, 4000, 4000, 4000, 4000},	// ring_cadoff
	".0",				// funckey_pstn: default is *0
	".1",				// funckey_transfer: default is *1
	5,					// auto dial
	10,					// off hook alarm //e8c: 10s
	2,					// caller id auto detection selection  0: disable 1: enable(NTT) 2: enable (NOT NTT)
	CID_DTMF,			// caller id detection mode
	0,					// one_stage_dial
	0,					// two_stage_dial
	0,					// auto_bypass_relay
	0,					// pulse dial gen config: 0: disable 1: enable
	10,					// pulse_gen_pps
	40,					// pulse_gen_make_time: msec
	700,				// pulse_gen_interdigit_pause: msec
	0,					// pulse_dial_det: 0: disable 1: enable
	450,				// pulse_det_pause: msec
	0,					// auto_cfg_ver
	0,					// auto_cfg_mode
	{0},				// auto_cfg_http_addr
	80,					// auto_cfg_http_port
	{0},				// auto_cfg_tftp_addr
	{0},				// auto_cfg_ftp_addr
	{0},				// auto_cfg_ftp_user
	{0},				// auto_cfg_ftp_passwd
	{0},				// auto_cfg_file_path
	0,					// auto_cfg_expire

	0,					// fw_update_mode
	{0},				// fw_update_tftp_addr
	{0},				// fw_update_http_addr
	80,					// fw_update_http_port
	{0},				// fw_update_ftp_addr
	{0},				// fw_update_ftp_user
	{0},				// fw_update_ftp_passwd
	{0},				// fw_update_file_path
	0,				// fw_update_power_on
	0,				// fw_update_scheduling_day
	0,				//fw_update_scheduling_time
	0,				//fw_update_auto
	{0},				// fw_update_file_prefix
	0,				//fw_update_next_time
	{0},				//fw_update_fw_version

	0, 				// wanVlanEnable;
	1,				// wanVlanIdVocie;
	7,				// wanVlanPriorityVoice;
	0,				// wanVlanCfiVoice;
	2,				// wanVlanIdData;
	0,				// wanVlanPriorityData;
	0,				// wanVlanCfiData;
	3,				// wanVlanIdVideo;
	5,				// wanVlanPriorityVideo;
	0,				// wanVlanCfiVideo;
        0,                              //vlan_enable
        10,                             //vlan_tag
        0,                              //vlan_bridge_enable
        12,                             //vlan_bridge_tag
        0,                              //vlan_bridge_port
        0,                             //vlan_bridge_multicast_enable
        13,                             //vlan_bridge_multicast_tag
	 0,                           	//vlan_host_enable
     20,                         	//vlan_host_tag
     7,                         	//vlan_host_pri
   	0,                           	//vlan_wifi_enable
    30,                         	//vlan_wifi_tag
    3,                         		//vlan_wifi_pri	30,
	0,                           	//vlan_wifi_vap0_enable
    30,                         	//vlan_wifi_vap0_tag
    3,                         		//vlan_wifi_vap0_pri,
    0,                           	//vlan_wifi_vap1_enable
    30,                         	//vlan_wifi_vap1_tag
    3,                         		//vlan_wifi_vap1_pri,
    0,                           	//vlan_wifi_vap2_enable
    30,                         	//vlan_wifi_vap2_tag
    3,                         		//vlan_wifi_vap2_pri,
    0,                           	//vlan_wifi_vap3_enable
    30,                         	//vlan_wifi_vap3_tag
    3,                         		//vlan_wifi_vap3_pri,

	1,				// hwnat_enable
	0,				// bandwidth_LANPort0_Egress
	0,				// bandwidth_LANPort1_Egress
	0,				// bandwidth_LANPort2_Egress
	0,				// bandwidth_LANPort3_Egress
	0,				// bandwidth_WANPort_Egress
	0,				// bandwidth_LANPort0_Ingress
	0,				// bandwidth_LANPort1_Ingress
	0,				// bandwidth_LANPort2_Ingress
	0,				// bandwidth_LANPort3_Ingress
	0,				// bandwidth_WANPort_Ingress
	7,				// daa_txVolumne
	7,				// daa_rxVolumne
	8,				// rtpDscp
	3,				// sipDscp
	{0},	//voip_interface
	1,		//X_CT_servertype
	INTER_DIGIT_TIMER_LONG,		//InterDigitTimerLong
	60,		//HangingReminderToneTimer
	40,		//BusyToneTimer
	NOANSWER_TIMER,		//NoAnswerTimer
	// ports[VOIP_PORTS]
	{
		// port 0
		{
			// proxies
			{
				// proxies[0]
				{
					// account
					"\0",				// display_name
					"\0",				// number
					"\0",				// login_id
					"\0",				// password
					// register server
					0,					// enable
					"\0",				// addr
					DEF_SIP_PROXY_PORT,	// port
					"\0",				// domain_name
					3600,					// reg_expire
					// nat traversal server
					0,							// outbound_enable
					"\0",						// outbound_addr
					DEF_OUTBOUND_PROXY_PORT,	// outbound_port
					/*+++++added by Jack for sip TLS+++++*/
					0,			//TLS_enable
					0,			//e8c: option ping expire , HEARTBEAT_CYCLE
					3,			//e8c: option retry count, HEARTBEAT_COUNT
					1800,			//e8c: session expire time , 1800 (sec)
					90,			//e8c: man,backup proxy fail, next retry time (sec)
					
					/*---end---*/
				},
			#if (MAX_PROXY > 1)
				// proxies[0]
				{
					// account
					"\0",				// display_name
					"\0",				// number
					"\0",				// login_id
					"\0",				// password
					// register server
					0,					// enable
					"\0",				// addr
					DEF_SIP_PROXY_PORT,	// port
					"\0",				// domain_name
					3600,					// reg_expire
					// nat traversal server
					0,							// outbound_enable
					"\0",						// outbound_addr
					DEF_OUTBOUND_PROXY_PORT,	// outbound_port
					/*+++++added by Jack for sip TLS+++++*/
					0,			//TLS_enable
					0,			//e8c: option ping expire , HEARTBEAT_CYCLE
					3,			//e8c: option retry count, HEARTBEAT_COUNT
					1800,			//e8c: session expire time , 1800 (sec)
					90,			//e8c: man,backup proxy fail, next retry time (sec)
					
					/*---end---*/
				},
	
			#endif
			},
			0,							// default_proxy
			// NAT Traversal
			0,							// stun_enable
			"\0",						// stun_addr
			DEF_STUN_SERVER_PORT,		// stun_port
			// advanced
			DEF_SIP_LOCAL_PORT,	// sip_port
			DEF_RTP_PORT,		// media_port
		//	DTMF_RFC2833,		// dtmf_mode
			DTMF_INBAND,		// dtmf_mode
			96,					// DTMF RFC2833 payload_type
			10,					// DTMF RFC2833 packet interval (msec)
			1,					// Fax/Modem RFC2833 PT is the same to DTMF or not
			101,					// Fax/Modem RFC2833 payload_type
			10,					// Fax/Modem RFC2833 packet interval (msec)
			250,					// sip info duration
	//		1,					// call_waiting_enable
			0,					// call_waiting_disable
			0,					// direct_ip_call
			// forward
			0,					// uc_forward_enable
			"\0",				// uc_forward
			0,					// busy_forward_enable
			"\0",				// busy_forward
			0,					// na_forward_enable
			"\0",				// na_forward
			0,					// na_forward_time
			{{{0}}},			// speed_dial
			0,					// replace rule option
			"\0",				// replace rule source plan
			"\0",				// replace rule target
			"\0",				// dialplan
			0,				    // digitmap_enable
			"\0",				// auto prefix
			"\0",				// prefix unset plan
			// codec
			{M_DEFAULT_FRAME_SIZE_LIST},// frame_size
			{M_DEFAULT_PRECEDENACE_LIST}, 	// precedence
			0,					// vad
			63,					// vad threshold
			1,					// cng
			0,					// cng threshold
			0,					// sid_gainmode, disable noise level configuration
			70,					// sid_noiselevel, -70dBov
			0,					// sid_noisegain, not change
			1,					// PLC
			10,					// RTCP interval (10 seconds)
			1,					// RTCP XR 
			G7231_RATE63,		// g7231_rate
			ILBC_30MS,			// iLBC_mode
			SPEEX_RATE8,		// Speex_nb_rate
			G726_PACK_RIGHT,	// g726 packing
			{					// g711_wb_modes
				G7111_R3,
				G7111_R2B,
				G7111_R2A,
				G7111_R1
			},

			// RTP redundant
			121,					// rtp_redundant_payload_type
#ifdef SUPPORT_RTP_REDUNDANT_APP
			0,						// rtp_redundant_codec
#else
			-1,						// rtp_redundant_codec
#endif
			// DSP
			6,				// slic_txVolumne
			6,				// slic_rxVolumne
			4,					// jitter delay
			20,					// maxDelay
			1,					// jitter factor 
			1,					// lec on/off
			1,					// nlp on/off
			2,					// echoTail
			(CID_DTMF|0x08),			// caller_id_mode
			0,				//use call waiting caller ID or not.0: disable 1: enable
			8,				// cid_dtmf_mode
			0,				// speaker agc
			0,				// speaker agc require level
			5,				// speaker agc max gain up
			5,				// speaker agc max gain down
			0,				// mic agc
			0,				// mic agc require level
			5,				// mic agc max gain up
			5,				// mic agc max gain down
			1,				// fsk gen mode 0:hardware, 1: software dsp gen
			0,				// spk voice gain
			0,				// mic voice gain
			0x31212,			// answer tone detect, enable ANS/ANSAMBAR/V.21flag IP/TDM detect
			0x0,				// fax/modem rfc2833 support
			// QoS
			DSCP_EF,
			// T.38
			0,				//enable T.38
			DEF_T38_PORT,	//T.38 default port
			FAX_MODEM_DET_AUTO2,		//fax_modem_det:Auto
			// T.38 parameter
			0,				///< enable T.38 parameters customization
			500, 			///< default: 500. 200~600.
			2,				///< default: 2. 1: local TCF, 2: remote TCF
			5,				///< default: 5. 0~5: 2400, 4800, 7200, 9600, 12000, 14400
			1,				///< default: 1. 1: enable, 0: disable
			5, 				///< default: 5. 0~7
			2,				///< default: 2. 0~2
			1,				///< default: 1. 1: enable, 0: disable 
			0,				///< default: 0. 0~2
			// V.152
			0,				// enable V.152
			102,			// V.152 payload type
			0,				// V.152 codec type (u-law)
			0,				// hotline_enable: 0 is disable
			{0},			// hotline_number
			0,				// dnd mode: 0 is disable
			0,				// dnd_from_hour
			0,				// dnd_from_min
			0,				// dnd_to_hour
			0,				// dnd_to_min
			FLASH_HOOKTIME,			// flash_hook_time
			FLASH_HOOKTIME_MIN,				// flash_hook_time_min
/* +++++Add by Jack for VoIP security 240108+++++ */
//#ifdef CONFIG_RTK_VOIP_SRTP
			0,				// security_enable
			KEY_EXCHANGE_SDES,	//key_exchange_mode
//#endif /* CONFIG_RTK_VOIP_SRTP */
/*-----end-----*/
			"\0",			// offhook_passwd
			{{{0}}},		// abbreviated dial
			0, 				// alarm enable
			0,				// alarm time hh
			0, 				// alarm time mm
#if 0
			0, 				// alarm ring last day
			0,				// alarm ring defer
#endif
			{0},			// PSTN_routing_prefix
		},
#if (CON_CH_NUM > 1)
	//port 1
		{
					// proxies
					{
						// proxies[0]
						{
							// account
							"\0",				// display_name
							"\0",				// number
							"\0",				// login_id
							"\0",				// password
							// register server
							0,					// enable
							"\0",				// addr
							DEF_SIP_PROXY_PORT, // port
							"\0",				// domain_name
							3600,					// reg_expire
							// nat traversal server
							0,							// outbound_enable
							"\0",						// outbound_addr
							DEF_OUTBOUND_PROXY_PORT,	// outbound_port
							/*+++++added by Jack for sip TLS+++++*/
							0,			//TLS_enable
							0, 		//e8c: option ping expire , HEARTBEAT_CYCLE
							3,			//e8c: option retry count, HEARTBEAT_COUNT
							1800,			//e8c: session expire time , 1800 (sec)
							90, 		//e8c: man,backup proxy fail, next retry time (sec)
							
							/*---end---*/
						},
			#if (MAX_PROXY > 1)
						// proxies[0]
						{
							// account
							"\0",				// display_name
							"\0",				// number
							"\0",				// login_id
							"\0",				// password
							// register server
							0,					// enable
							"\0",				// addr
							DEF_SIP_PROXY_PORT, // port
							"\0",				// domain_name
							3600,					// reg_expire
							// nat traversal server
							0,							// outbound_enable
							"\0",						// outbound_addr
							DEF_OUTBOUND_PROXY_PORT,	// outbound_port
							/*+++++added by Jack for sip TLS+++++*/
							0,			//TLS_enable
							0, 		//e8c: option ping expire , HEARTBEAT_CYCLE
							3,			//e8c: option retry count, HEARTBEAT_COUNT
							1800,			//e8c: session expire time , 1800 (sec)
							90, 		//e8c: man,backup proxy fail, next retry time (sec)
							
							/*---end---*/
						},

			#endif
					},
					0,							// default_proxy
					// NAT Traversal
					0,							// stun_enable
					"\0",						// stun_addr
					DEF_STUN_SERVER_PORT,		// stun_port
					// advanced
					DEF_SIP_LOCAL_PORT, // sip_port
					DEF_RTP_PORT,		// media_port
			//		DTMF_RFC2833,		// dtmf_mode
					DTMF_INBAND,		// dtmf_mode
					96, 				// DTMF RFC2833 payload_type
					10, 				// DTMF RFC2833 packet interval (msec)
					1,					// Fax/Modem RFC2833 PT is the same to DTMF or not
					101,					// Fax/Modem RFC2833 payload_type
					10, 				// Fax/Modem RFC2833 packet interval (msec)
					250,					// sip info duration
		//			1,					// call_waiting_enable
					0,					// call_waiting_disable
					0,					// direct_ip_call
					// forward
					0,					// uc_forward_enable
					"\0",				// uc_forward
					0,					// busy_forward_enable
					"\0",				// busy_forward
					0,					// na_forward_enable
					"\0",				// na_forward
					0,					// na_forward_time
					{{{0}}},			// speed_dial
					0,					// replace rule option
					"\0",				// replace rule source plan
					"\0",				// replace rule target
					"\0",				// dialplan
					0,					// digitmap_enable
					"\0",				// auto prefix
					"\0",				// prefix unset plan
					// codec
					{M_DEFAULT_FRAME_SIZE_LIST},// frame_size
					{M_DEFAULT_PRECEDENACE_LIST},	// precedence
					0,					// vad
					63, 				// vad threshold
					1,					// cng
					0,					// cng threshold
					0,					// sid_gainmode, disable noise level configuration
					70, 				// sid_noiselevel, -70dBov
					0,					// sid_noisegain, not change
					1,					// PLC
					10, 				// RTCP interval (10 seconds)
					1,					// RTCP XR 
					G7231_RATE63,		// g7231_rate
					ILBC_30MS,			// iLBC_mode
					SPEEX_RATE8,		// Speex_nb_rate
					G726_PACK_RIGHT,	// g726 packing
					{					// g711_wb_modes
						G7111_R3,
						G7111_R2B,
						G7111_R2A,
						G7111_R1
					},
		
					// RTP redundant
					121,					// rtp_redundant_payload_type
#ifdef SUPPORT_RTP_REDUNDANT_APP
					0,						// rtp_redundant_codec
#else
					-1, 					// rtp_redundant_codec
#endif
					// DSP
					6,				// slic_txVolumne
					6,				// slic_rxVolumne
					4,					// jitter delay
					20, 				// maxDelay
					1,					// jitter factor 
					1,					// lec on/off
					1,					// nlp on/off
					2,					// echoTail
					(CID_DTMF|0x08),			// caller_id_mode
					0,				//use call waiting caller ID or not.0: disable 1: enable
					8,				// cid_dtmf_mode
					0,				// speaker agc
					0,				// speaker agc require level
					5,				// speaker agc max gain up
					5,				// speaker agc max gain down
					0,				// mic agc
					0,				// mic agc require level
					5,				// mic agc max gain up
					5,				// mic agc max gain down
					1,				// fsk gen mode 0:hardware, 1: software dsp gen
					0,				// spk voice gain
					0,				// mic voice gain
					0x31212,			// answer tone detect, enable ANS/ANSAMBAR/V.21flag IP/TDM detect
					0x0,				// fax/modem rfc2833 support
					// QoS
					DSCP_EF,
					// T.38
					0,				//enable T.38
					DEF_T38_PORT,	//T.38 default port
					FAX_MODEM_DET_AUTO2,		//fax_modem_det:Auto
					// T.38 parameter
					0,				///< enable T.38 parameters customization
					500,			///< default: 500. 200~600.
					2,				///< default: 2. 1: local TCF, 2: remote TCF
					5,				///< default: 5. 0~5: 2400, 4800, 7200, 9600, 12000, 14400
					1,				///< default: 1. 1: enable, 0: disable
					5,				///< default: 5. 0~7
					2,				///< default: 2. 0~2
					1,				///< default: 1. 1: enable, 0: disable 
					0,				///< default: 0. 0~2
					// V.152
					0,				// enable V.152
					102,			// V.152 payload type
					0,				// V.152 codec type (u-law)
					0,				// hotline_enable: 0 is disable
					{0},			// hotline_number
					0,				// dnd mode: 0 is disable
					0,				// dnd_from_hour
					0,				// dnd_from_min
					0,				// dnd_to_hour
					0,				// dnd_to_min
					FLASH_HOOKTIME,			// flash_hook_time
					FLASH_HOOKTIME_MIN,		// flash_hook_time_min
		/* +++++Add by Jack for VoIP security 240108+++++ */
		//#ifdef CONFIG_RTK_VOIP_SRTP
					0,				// security_enable
					KEY_EXCHANGE_SDES,	//key_exchange_mode
		//#endif /* CONFIG_RTK_VOIP_SRTP */
		/*-----end-----*/
					"\0",			// offhook_passwd
					{{{0}}},		// abbreviated dial
					0,				// alarm enable
					0,				// alarm time hh
					0,				// alarm time mm
#if 0
					0,				// alarm ring last day
					0,				// alarm ring defer
#endif
					{0},			// PSTN_routing_prefix
				}

#endif
	},
#ifdef CONFIG_RTK_VOIP_IP_PHONE
	{0}, 	// ui
#endif
	
};
