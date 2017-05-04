#ifndef SUBR_WLAN_H
#define SUBR_WLAN_H

extern int wlan_idx;
extern const char *WLANIF[];
extern const char IWPRIV[];
extern const char AUTH[];
extern const char IWCONTROL[];
extern const char AUTH_PID[];
extern const char WLAN_AUTH_CONF[];
extern const char *wlan_encrypt[];
#ifdef WLAN_WDS
extern const char WDSIF[];
#endif
extern char *WLANAPIF[];

extern const char *wlan_band[];
extern const char *wlan_mode[];
extern const char *wlan_rate[];
extern const char *wlan_auth[];
extern const char *wlan_preamble[];
extern const char *wlan_encrypt[];
extern const char *wlan_pskfmt[];
extern const char *wlan_wepkeylen[];
extern const char *wlan_wepkeyfmt[];
extern const char *wlan_Cipher[];

#define SSID_LEN	32
#ifdef WLAN_CLIENT
#define	MAX_BSS_DESC	64
#define MESH_ID_LEN 32
typedef struct _OCTET_STRING {
    unsigned char *Octet;
    unsigned short Length;
} OCTET_STRING;
typedef enum _BssType {
    infrastructure = 1,
    independent = 2,
} BssType;
typedef	struct _IbssParms {
    unsigned short	atimWin;
} IbssParms;
typedef struct _BssDscr {
    unsigned char bdBssId[6];
    unsigned char bdSsIdBuf[SSID_LEN];
    OCTET_STRING  bdSsId;
    unsigned char	meshid[MESH_ID_LEN];
    unsigned char	*meshidptr;			// unused, for backward compatible
    unsigned short	meshidlen;
    BssType bdType;
    unsigned short bdBcnPer;			// beacon period in Time Units
    unsigned char bdDtimPer;			// DTIM period in beacon periods
    unsigned long bdTstamp[2];			// 8 Octets from ProbeRsp/Beacon
    IbssParms bdIbssParms;			// empty if infrastructure BSS
    unsigned short bdCap;				// capability information
    unsigned char ChannelNumber;			// channel number
    unsigned long bdBrates;
    unsigned long bdSupportRates;
    unsigned char bdsa[6];			// SA address
    unsigned char rssi, sq;			// RSSI and signal strength
    unsigned char network;			// 1: 11B, 2: 11G, 4:11G
    // P2P_SUPPORT
    unsigned char p2pdevname[33];
    unsigned char p2prole;
    unsigned short p2pwscconfig;
    unsigned char p2paddress[6];
} BssDscr, *pBssDscr;
typedef struct _sitesurvey_status {
    unsigned char number;
    unsigned char pad[3];
    BssDscr bssdb[MAX_BSS_DESC];
} SS_STATUS_T, *SS_STATUS_Tp;
typedef enum _Capability {
    cESS 		= 0x01,
    cIBSS		= 0x02,
    cPollable		= 0x04,
    cPollReq		= 0x01,
    cPrivacy		= 0x10,
    cShortPreamble	= 0x20,
} Capability;
typedef enum _Synchronization_Sta_State{
    STATE_Min		= 0,
    STATE_No_Bss	= 1,
    STATE_Bss		= 2,
    STATE_Ibss_Active	= 3,
    STATE_Ibss_Idle	= 4,
    STATE_Act_Receive	= 5,
    STATE_Pas_Listen	= 6,
    STATE_Act_Listen	= 7,
    STATE_Join_Wait_Beacon = 8,
    STATE_Max		= 9
} Synchronization_Sta_State;
#endif	// of WLAN_CLIENT


typedef enum _wlan_mac_state {
    STATE_DISABLED=0, STATE_IDLE, STATE_SCANNING, STATE_STARTED, STATE_CONNECTED, STATE_WAITFORKEY
} wlan_mac_state;

typedef struct _bss_info {
    unsigned char state;
    unsigned char channel;
    unsigned char txRate;
    unsigned char bssid[6];
    unsigned char rssi, sq;	// RSSI  and signal strength
    unsigned char ssid[SSID_LEN+1];
} bss_info;

#ifdef WLAN_WDS
typedef enum _wlan_wds_state {
    STATE_WDS_EMPTY=0, STATE_WDS_DISABLED, STATE_WDS_ACTIVE
} wlan_wds_state;

typedef struct _WDS_INFO {
	unsigned char	state;
	unsigned char	addr[6];
	unsigned long	tx_packets;
	unsigned long	rx_packets;
	unsigned long	tx_errors;
	unsigned char	txOperaRate;
} WDS_INFO_T, *WDS_INFO_Tp;

#endif //WLAN_WDS

struct _misc_data_ {
	unsigned char	mimo_tr_hw_support;
	unsigned char	mimo_tr_used;
	unsigned char	resv[30];
};

#ifdef WLAN_SUPPORT
int getWlStaInfo( char *interface,  WLAN_STA_INFO_Tp pInfo );
#endif
void restartWlan(void);
int config_WLAN( int action_type );
#ifdef CONFIG_USER_FON
int setFonFirewall();
#endif

extern int wl_isNband(unsigned char band);
extern void wl_updateSecurity(unsigned char band);
extern unsigned char wl_cipher2mib(unsigned char cipher);

char *getWlanIfName(void);
void getWscPidName(char *wscd_pid_name);
void getWispWanName(char *name);
void setWlanDevFlag(char *ifname, int set_wan);
int setup_wlan_block(void);

#endif

