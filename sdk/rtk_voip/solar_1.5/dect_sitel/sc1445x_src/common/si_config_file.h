/*
 *-----------------------------------------------------------------------------
 *                                                                             
 *               @@@@@@@      *   @@@@@@@@@           *                                     
 *              @       @             @               *                            
 *              @             *       @      ****     *                                 
 *               @@@@@@@      *       @     *    *    *                              
 *        ___---        @     *       @     ******    *                                 
 *  ___---      @       @     *       @     *         *                             
 *   -_          @@@@@@@  _   *       @      ****     *                               
 *     -_                 _ -                                                     
 *       -_          _ -                                                       
 *         -_   _ -        s   e   m   i   c   o   n   d   u   c   t   o   r 
 *           -                                                                    
 *                                                                              
 * (C) Copyright SiTel Semiconductor BV, unpublished work.
 * This computer program includes Confidential, Proprietary Information and
 * is a Trade Secret of SiTel Semiconductor BV.
 * All use, disclosure, and/or reproduction is prohibited unless authorized
 * in writing. All Rights Reserved.
 *
 *-----------------------------------------------------------------------------
 * File:			si_config_file.h
 * Purpose:		
 * Created:		29/11/2007
 * By:			  KF
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef SI_CONFIG_FILE_H
#define SI_CONFIG_FILE_H

#include "operation_mode_defs.h"

//#include "../../../../../../linux-2.6.x/include/linux/autoconf.h" 
#include "si_linux_autoconf.h"
#include "si_phbook_api.h"
/*========================== Include files ==================================*/


/*========================== Local macro definitions & typedefs =============*/
#if ((defined CONFIG_SC1445x_DECT_SUPPORT) && (!(defined CONFIG_SC1445x_DECT_HEADSET_SUPPORT)))
	#define MAX_SUPPORTED_ACCOUNTS			7
#elif defined CONFIG_SC1445x_LEGERITY_890_SUPPORT
	#ifdef SUPPORT_3_CHANNELS
		#define MAX_SUPPORTED_ACCOUNTS		3
	#elif defined SUPPORT_4_CHANNELS
		#define MAX_SUPPORTED_ACCOUNTS		4
	#endif
#else 
	#define MAX_SUPPORTED_ACCOUNTS			3
#endif

#ifdef SUPPORT_4_CHANNELS

		#define MAX_CHANNELS_SUPPORTED		4
		#define MAX_PCM_PORTS_SUPPORTED		4
		#define MCU_SUPPORTED_CHANNELS		4

	#if (defined CONFIG_LMX4180_DECT_SUPPORT)
		#if (defined CONFIG_ATA_1_FXS_NO_FXO)
			#define MAX_ADPCM_PORTS_SUPPORTED 5 //In case of ata+natalie, natalie ports start at "ata fxs initialized ports + 1"
			#define MCU_SUPPORTED_PORTS				5
		#elif (defined CONFIG_ATA_2_FXS_NO_FXO)
			#define MAX_ADPCM_PORTS_SUPPORTED 6 //In case of ata+natalie, natalie ports start at "ata fxs initialized ports + 1"
			#define MCU_SUPPORTED_PORTS				6
		#elif (defined CONFIG_ATA_3_FXS_NO_FXO)
			#define MAX_ADPCM_PORTS_SUPPORTED 6 //We cannot go more than 6 due to account number limitations. 7th account is the broadcast account.
			#define MCU_SUPPORTED_PORTS				6
		#elif (defined CONFIG_ATA_4_FXS_NO_FXO)
			#define MAX_ADPCM_PORTS_SUPPORTED 6 //We cannot go more than 6 due to account number limitations. 7th account is the broadcast account.
			#define MCU_SUPPORTED_PORTS				6
		#else 
			#define MAX_ADPCM_PORTS_SUPPORTED 4 
			#define MCU_SUPPORTED_PORTS				4
		#endif
	#else
		#define MAX_ADPCM_PORTS_SUPPORTED 4 
		#define MCU_SUPPORTED_PORTS				4
	#endif

#elif defined SUPPORT_3_CHANNELS

	#define MAX_CHANNELS_SUPPORTED		3
	#define MAX_PCM_PORTS_SUPPORTED		3
	#define MCU_SUPPORTED_CHANNELS		3
	#define MCU_SUPPORTED_PORTS				3
	#define MAX_ADPCM_PORTS_SUPPORTED 3 

#endif 

#define MAX_SUPPORTED_CODECS			7
#define MAX_NUM_OF_STATIONS				10

#define DISABLED	0
#define ENABLED		1

#define RSSFEED_TOPICS_WEATHER	0
#define RSSFEED_TOPICS_NEWS		1

#define TRANSPORT_UDP 0
#define TRANSPORT_TCP 1
#define TRANSPORT_TLS 2

#define IN_BAND				0
#define SIP_INFO			1
#define RFC_2833			2

#define NAT_OFF				0
#define NAT_STUN			1
#define NAT_STATIC		2

#define SUBSCRIBE_OFF 0
#define SUBSCRIBE_ON	1

#define SUBSCRIBE_STATUS_OFF 0
#define SUBSCRIBE_STATUS_ON  1

#define NEVER				  0
#define ON_NO_ANSWER	1
#define ON_BUSY				2
#define ALWAYS				3

#define OFF		0
#define SOFT	1
#define LOUD	2
#define _12_HOUR	1
#define _24_HOUR	2
#define ENGLISH	1
#define STAR_CODES_NUMBER	17
#define AUTO_DETECT		0
#define PASS_THROUGH	1

#define IPModev4 0
#define IPModev6 1

typedef enum _crypto_options
{
 ID_CRYPTO_DISABLE,
 ID_CRYPTO_OPTIONAL, 
 ID_CRYPTO_MANDATORY,
 ID_CRYPTO_SUCCESS
}crypto_options;

typedef struct _NetworkSettingsInfo {
	unsigned char IP_address_info[16];
	unsigned char subnet_mask_info[16];
	unsigned char default_gateway_info[16];
	unsigned char primary_DNS_info[16];
	unsigned char secondary_DNS_info[16];
	char DHCP_info;
	unsigned char reserved[3];
} NetworkSettingsInfo;

typedef struct _NetworkSettings {
	unsigned char IP_address[64];
	unsigned char subnet_mask[64];
	unsigned char default_gateway[64];
	unsigned char primary_DNS[64];
	unsigned char secondary_DNS[64];
	char DHCP;
	char IPMode;
	char reserved[2];
} NetworkSettings;

typedef struct _NetApplications {
	char tftp_server_ip[16];
	char RSSFeedEnable;
	char RSSFeedTopics;
	char InternetRadio;
	char NTP_enable;
	char NTP_server_address[64];
	char presence;
	char reserved[3];
}NetApplications;

typedef struct _HardwareSettings {
	unsigned char MAC_address_1[6];
	unsigned char MAC_address_2[6];
	char product_model[32];
	char user_name[32];
	char user_password[32];
} HardwareSettings;

typedef struct _AudioSettings {
	int codec[MAX_SUPPORTED_CODECS];
	int ptime;
	int jitter;
	int VAD;
	int DTMF_mode;
	int SRTP_mode;
	int TOS;
} AudioSettings;

typedef struct _SIPSettings {
	char domain[64];
	char registrar[64];
 	char proxy_address[64];
	char registrarPort[8];
	char IP_address[16];
	char STUN_address[64];
	char STUN_port[8];
	char RTP_port_start[8];
	char RTP_port_stop[8];
	int expiry;
	char publicport[16];
	char NAT;
	char SessionTimer;
	char NAPTR;
 	char PRACK;
	char update;
 	char use_out_bound_proxy;
	char transport;
	char LocalSipPort[6]; 
	char reserved[3]; 
} SIPSettings;

typedef struct _SIPAccount {
	char user_id[64];
	char user_name[64];
	char user_password[64];
	char use_account;
	char reserved[3];
} SIPAccount;

typedef struct _CallSettings {
	char call_forward_number[64];
	char call_forward_mode;
	char call_forward_start_time;
	char call_waiting;
	char DND;
	char auto_answer;
	char hide_id;
	char reject_anonymous;
	char reserved;
} CallSettings;

typedef struct _SubscribeSettings {
	char subscribe_enable[MAX_NUM_OF_CONTACTS];
	char subscribe_name[MAX_NUM_OF_CONTACTS][64];
	char subscribe_number[MAX_NUM_OF_CONTACTS][64];
	char subscribe_status[MAX_NUM_OF_CONTACTS];
} SubscribeSettings;

typedef struct _PhoneSettings {
	char display_brightness;
	char backlight_timeout;
	char ring_tone[12];
	char ring_volume;
	char reserved;
	unsigned short speaker_volume;
	unsigned short handset_volume;
	unsigned short headset_volume;
	char keypad_tone;
	char date_year; // number of years after 1970
	char date_month;
	char date_day;
	char time_hour;
	char time_minute;
	char time_second;
	char time_format;
	char time_zone;
	char language;
} PhoneSettings;
  
typedef struct _ATA_settings {
  char bell_style;
	char fax_mode;
  char reserved[2];
	char dial_plan[200];
}ATA_Settings;

typedef struct _DialPlan_settings {
	char dp_enable;
	char dial_plan[255];
}DialPlan_settings;

typedef struct _star_code {
	char name[30];
	char local;
	char code[3];
	void (*star_code_func)(unsigned char, void*);
} star_code_t;

typedef struct _PrintLogSettings {	
	char print_enable;
	char print_level;
	char print_level_option;
	char reserved;
	char print_to[16];
	char syslog_server_ip[16];
	char print_file_path[256];
} PrintLogSettings;

typedef struct _InternetRadioSettings {	
	char station_name[80];
	char URL[255];
} InternetRadioSettings;

typedef struct _ConfigSettings 
{
	NetworkSettingsInfo m_NetworkSettings_info;
	NetworkSettings m_NetworkSettings;
	NetApplications m_NetApplications;
	HardwareSettings m_HardwareSettings;
	AudioSettings m_AudioSettings[MAX_SUPPORTED_ACCOUNTS];
	SIPSettings m_SIPSettings;
	SIPAccount m_SIPAccount[MAX_SUPPORTED_ACCOUNTS];
	CallSettings m_CallSettings[MAX_SUPPORTED_ACCOUNTS];
#if (!((defined ATA_ENABLED) || (defined DECT_ENABLED)))
 	PhoneSettings m_PhoneSettings;
#elif (defined ATA_ENABLED)
	ATA_Settings m_ATA_Settings;
#elif ((defined ATA_ENABLED) || (defined DECT_ENABLED))
	star_code_t StarCode[STAR_CODES_NUMBER];
#endif
	SubscribeSettings m_SubscribeSettings;
	DialPlan_settings m_DialPlan_settings;
	PrintLogSettings m_PrintLogSettings;
	InternetRadioSettings m_InternetRadioSettings[MAX_NUM_OF_STATIONS];
} ConfigSettings;

/*========================== Global variables ===============================*/


/*========================== Global function prototypes =====================*/

#endif /* SI_CONFIG_FILE_H */

