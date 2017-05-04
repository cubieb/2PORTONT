/*
*modified from GNU PPPoE source code
*/

#ifndef _PPPOE_H_
#define _PPPOE_H_

#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <syslog.h>
#include <stdio.h>		/* stdio               */
#include <stdlib.h>		/* strtoul(), realloc() */
#include <getopt.h>

#include <sys/types.h>
#include <asm/types.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <net/if_arp.h>
#include <netinet/in.h>
#include <net/ethernet.h>
#include <linux/if.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_pppox.h>
#include "timeout.h"
#ifdef EMBED
#include <rtk/msgq.h>
#else
#include "../../include/rtk/msgq.h"
#endif

#define SPPP_PPPOE	1
#define ENABLE_PPP_SYSLOG	1


/* Ethernet frame types */
#define ETH_PPPOE_DISCOVERY 0x8863
#define ETH_PPPOE_SESSION   0x8864

/* PPPoE codes */
#define CODE_PADI           0x09
#define CODE_PADO           0x07
#define CODE_PADR           0x19
#define CODE_PADS           0x65
#define CODE_PADT           0xA7
#define CODE_SESS           0x00

/* PPPoE Tags */
#define TAG_END_OF_LIST        0x0000
#define TAG_SERVICE_NAME       0x0101
#define TAG_AC_NAME            0x0102
#define TAG_HOST_UNIQ          0x0103
#define TAG_AC_COOKIE          0x0104
#define TAG_VENDOR_SPECIFIC    0x0105
#define TAG_RELAY_SESSION_ID   0x0110
#define TAG_SERVICE_NAME_ERROR 0x0201
#define TAG_AC_SYSTEM_ERROR    0x0202
#define TAG_GENERIC_ERROR      0x0203

/* Discovery phase states */
#define PPPOE_STATE_IDLE				0
#define PPPOE_STATE_SENT_PADI			(1<<0)
#define PPPOE_STATE_RECEIVED_PADO	(1<<1)
#define PPPOE_STATE_SENT_PADR			(1<<2)
#define PPPOE_STATE_SESSION			(1<<3)
#define PPPOE_STATE_TERMINATED		(1<<4)

#define STATE_DISCOVERY			(PPPOE_STATE_SENT_PADI|PPPOE_STATE_RECEIVED_PADO|PPPOE_STATE_SENT_PADR)

#if 0
	#define DEBUG_PRINT(fmt, args...) printf(fmt, ## args)
#else
	#define DEBUG_PRINT(fmt, args...)
#endif


/* How many PADI/PADS attempts? */
#define MAX_PADI_ATTEMPTS 10

/* Initial timeout for PADO/PADS */
#define PADI_TIMEOUT 1 //sec

#define STR_LEN  128

extern char pre_PADT_flag[];
extern unsigned int disc_counter[];
extern char sendPADR_flag[];
#ifdef ENABLE_PPP_SYSLOG
extern int dbg_syslog;
#endif


/* We assume that the compiler will not place any padding into the following
   structure.  (If it did, structures would be pretty much useless.)
*/

typedef struct ether_addr_s {
	unsigned char octet[ETHER_ADDR_LEN];
} ether_addr_t;

struct Ehdr {			/* Ethernet header */
    ether_addr_t to;
    ether_addr_t from;
    unsigned short type;
};
#define Ehdr_SZ 14          /* size of header */
#define ET_MAXLEN	1500
/* A PPPoE Packet, including Ethernet headers */
struct PPPoEPacket {
    struct Ehdr ehdr;		/* Ethernet header */
    unsigned char ver_type;	/* PPPoE Version (must be 1) */
							/* PPPoE Type (must be 1) */
    unsigned char code;		/* PPPoE code */
    unsigned short session;	/* PPPoE session */
    unsigned short length;	/* Payload length */
    unsigned char payload[ET_MAXLEN]; /* A bit of room to spare */
};

/* Header size of a PPPoE packet */
#define PPPOE_OVERHEAD 6  /* type, code, session, length */
#define PPPOE_HDR_SIZE (sizeof(struct Ehdr) + PPPOE_OVERHEAD)

/* PPPoE Tag */

struct PPPoETag {
    unsigned int type:16;	/* tag type */
    unsigned int length:16;	/* Length of payload */
    unsigned char payload[ET_MAXLEN]; /* A LOT of room to spare */
};


typedef struct pppoe_s_info {
	unsigned int	uifno;			/* index of device */
	unsigned short session;				/* Identifier for our session */
	struct sockaddr_ll remote;
} PPPOE_SESSION_INFO;


/* Header size of a PPPoE tag */
#define TAG_HDR_SIZE 4
#define Eid_SZ	6

typedef struct pppoe_s {
	int	ifno;			/* index of device */
	int uifno;          /* unique id of this device for storing into MIB PPPoE Session table */
	char name[IFNAMSIZ];		/*dev name */
	struct sockaddr_ll local;
	struct sockaddr_ll remote;
	struct sockaddr_ll recv;
	struct sockaddr_pppox sp;
	int fd;				/* fd of PPPoE socket */
	int disc_sock;

	int DiscoveryState;
	int optUseHostUnique;   		    /* Use Host-Unique tag for multiple sessions */
	char ServiceName[STR_LEN];			/* Desired service name */
	char DesiredACName[STR_LEN];	    /* Desired access concentrator */
	char ACName[STR_LEN];	    		/* access concentrator */
	unsigned char ACEthAddr[Eid_SZ];	/* Destination hardware address */
	unsigned char MyEthAddr[Eid_SZ];    /* My hardware address */
	unsigned short session;				/* Identifier for our session */
	struct PPPoETag cookie;				/* We have to send this if we get it */
	struct PPPoETag relayId;			/* Ditto */
	struct PPPoETag hostUniq;			/* We have to send this if we get it */
	struct	callout ch; 				/* PPPoE timeout handler */
	int	timeout;
	int debug;
	unsigned short vpi;				/* Jenny, to distinguish between several PPPoE connection */
	unsigned short vci;
	int in_pppoe_disc;				/* Jenny, if in discovery phase */
	struct callout disc_ch;
	int disc_timeout;
	char NoHardware_NAT;

#ifdef CONFIG_USER_PPPOE_2_INTERVALS_RETRY
	int retry_threshold;
	int retry_delay;
	int retry_delay_long;
#endif
} PPPOE_DRV_CTRL;

typedef struct pppoeproxy_s {
	struct sppp *sp;
	char name[IFNAMSIZ];		/*dev name */
	struct sockaddr_ll client;
	struct sockaddr_pppox localaddr;
	struct sockaddr_pppox peeraddr;
	int fd;				/* fd of PPPoE socket */

	char ServiceName[STR_LEN];			/* Desired service name */
	char ACName[STR_LEN];	    		/* access concentrator */
	unsigned char localEthAddr[Eid_SZ];	/* My hardware address */
	unsigned char peerEthAddr[Eid_SZ];    /*Destination hardware address  */
	unsigned short session;				/* Identifier for our session */
	struct PPPoETag cookie;				/* We have to send this if we get it */
	struct PPPoETag relayId;			/* Ditto */
	struct PPPoETag hostUniq;			/* We have to send this if we get it */
	struct	callout ch; 				/* PPPoE timeout handler */
	int	timeout;
	int debug;
	unsigned short vpi;				/* Jenny, to distinguish between several PPPoE connection */
	unsigned short vci;
	int in_pppoe_disc;				/* Jenny, if in discovery phase */
	struct callout disc_ch;
	int disc_timeout;

} PPPOEPROXY_DRV_CTRL;

/* Structure used to determine acceptable PADO or PADS packet */
struct PacketCriteria {
    int acNameOK;
    int serviceNameOK;
};

struct pppoe_param_s {
	int DiscoveryState;
	unsigned short session;				/* Identifier for our session */
	char dev_name[IFNAMSIZ];
	char ACName[STR_LEN];	    		/* access concentrator */
	char ServiceName[STR_LEN];	    		/* Jenny, Service name */
	unsigned char ACEthAddr[Eid_SZ];	/* Destination hardware address */
	unsigned char MyEthAddr[Eid_SZ];    /* My hardware address */
	unsigned short vpi;				/* Jenny, to distinguish between several PPPoE connection */
	unsigned short vci;
	char NoHardware_NAT;
#ifdef CONFIG_USER_PPPOE_2_INTERVALS_RETRY
	int retry_threshold;
	int retry_delay;
	int retry_delay_long;
#endif

};

#ifdef CONFIG_USER_PPPOE_PROXY
#define SIOCGSWPORT 0x89a4

struct SW_INFO{
	unsigned char hwaddr[ETH_ALEN];
	unsigned short swport;
};

#endif

/* Format for parsing long device-name */
#define _STR(x) #x
#define FMTSTRING(size) "%x:%x:%x:%x:%x:%x/%x/%" _STR(size) "s"

#define MAX_PPPOE_PAYLOAD (ET_MAXLEN - PPPOE_OVERHEAD)
#define CHECK_ROOM(cursor, start, len) \
do {\
    if (((cursor)-(start))+(len) > MAX_PPPOE_PAYLOAD) { \
        syslog(LOG_ERR, "Would create too-long packet"); \
        return; \
    } \
} while(0)

#if 0
int pppoeInit(int unit,
			char *pDevice,
			int dunit,
			char *local_addr,
			char *remote_addr,
			char *username,
			char *secret);

int pppoeDelete(int unit, char *pDevice, int dunit);
int pppoeDebug(int unit, int debug);
int pppoeRestart(int unit);
int pppoeStatus(int unit, pppoe_param_p pParam);
int pppoeDebugGet(char *out_buf, int len);

int pppoeShow(int unit);
void pppoeDebugFlush(void);
#endif



#endif
