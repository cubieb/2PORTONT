#ifndef	__IP_CLIENT_LIMIT_H__
#define	__IP_CLIENT_LIMIT_H__

#define	MODULE_NAME	"Client Limit"

#define	ipaddr_t __u32

#define LIMIT_NONE        0
#define LIMIT_GLOBAL      1
#define LIMIT_BY_DEV_TYPE 2

#define DETECT_TIME       3
#define DETECT_INTERVAL   10 /* 10s */

#define CLIENT_LIMIT_CONFIG_FILE "ClientsMonitor"
#define CLIENT_LIMIT_DEV_FILE    "ClientsDev"

enum DeviceType
{
	CTC_Computer = 0,
	CTC_Camera,
	CTC_HGW,
	CTC_STB,
	CTC_PHONE,
	CTC_MAX_DEV,
};

struct dev_entry /* Device found by dhcp */
{
	struct list_head list;
	int dev_type;
	unsigned char dev_mac[ETH_ALEN];
};

struct client_entry 
{
	struct list_head list;
	struct list_head list_same_devtype;
    ipaddr_t client_ip;
    int polling_succ;
    int polling_time;
	unsigned char client_mac[ETH_ALEN];
};

struct arpreq_by_client_limit 
{
    ipaddr_t ip;
    int arp_flags;
    unsigned long confirmed;
    unsigned char mac[ETH_ALEN];
};

#define CLIENTS_POLL_TIMER 1

#if 0
#define DEBUGP printk
#define DUMP_CONTENT(dptr, length) \
	{\
	  int i;\
	  DEBUGP("\n*************************DUMP***********************\n");\
	  for (i=0;i<length;i++)\
	    DEBUGP("%c",dptr[i]);\
	  DEBUGP("*************************DUMP OVER******************\n");\
	}
#else
#define DEBUGP(format, args...)
#define DUMP_CONTENT(dptr, length) 
#endif




#endif
