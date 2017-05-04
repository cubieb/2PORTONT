/*  
 * Data Declaration  
 */
#ifndef _LAN_NET_INFO_H_
#define _LAN_NET_INFO_H_


/* ethernet address type */
typedef struct  lan_host_mac_s
{
    uint8 octet[MAC_ADDR_LEN];
} lan_host_mac_t;

#define NET_INFO_UPDATE_INTERVAL 	1 /*in unit of second*/

#define NET_INFO_ARP_INTERVAL 		2 /* in unit of second */
#define NET_INFO_AGEOUT_TIME 		30 /*in unit of second*/

#define MAX_LAN_HOST_INFO_NUM	256

static struct timeval queryTime;
static struct timeval interval;

int writeToLanHostFile = 0;
uint8 newLanHostIdx[MAX_LAN_HOST_INFO_NUM] = {0}; 

enum DeviceType
{
	CTC_PHONE=0,
	CTC_PAD,
	CTC_Computer,
	CTC_STB,
	CTC_OTHER
};

typedef struct lanHostInfo_s
{
	lan_host_mac_t	mac;
	char			devName[MAX_LANNET_DEV_NAME_LENGTH];
	uint8			devType;	/* 0-phone 1-pad 2-PC 3-STB 4-other  0xff-unknown */
	uint32			ip;
	uint8			connectionType;	/* 0- wired 1-wireless */
	uint8			port;	/* 0-wifi, 1- lan1, 2-lan2, 3-lan3, 4-lan4 */
	char			brand[MAX_LANNET_BRAND_NAME_LENGTH];
	char			os[MAX_LANNET_OS_NAME_LENGTH];
	uint32			onLineTime;
} lanHostInfo_t;

typedef struct lanNetInfo_s
{
	lanHostInfo_t 			lanHost;
	int32					idleSecs;
	uint64					startOnLine;
	uint8 					valid;	/* 0-invalid 1-valid */
} lanNetInfo_t;

lanNetInfo_t lanNetInfoData[MAX_LAN_HOST_INFO_NUM];

enum Port_Num
{
	Port_WIFI = 0,	//0:wifi, 1:lan1, 2:lan2, 3:lan3, 4:lan4
	Port_LAN1,
	Port_LAN2,
	Port_LAN3,
	Port_LAN4,
};

enum Host_InterfaceType
{
	Host_Ethernet = 0,
	Host_802_11,
};
#endif
