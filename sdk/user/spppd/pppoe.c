/*
 * PPPOE.C [RFC 2516]
 *
 */



#include "if_sppp.h"
#include "pppoe.h"
#include <rtk/utility.h>
#include <linux/pkt_sched.h>
#include "md5.h"
#include "arpping.h"
#include <stdlib.h>


#define OK 0
#define ERROR -1
#define TRUE	1
#define FALSE	0

#ifdef CONFIG_USER_PPPOE_PROXY
/* Structures used by PPPoE server */
struct ClientSession {
	unsigned int sess;		/* Session number */
	unsigned char ip[4];	/* IP address of peer */
	unsigned char hwaddr[Eid_SZ]; /* Peer's Ethernet address */
	unsigned char isUsingNow;	  /* Now is using this session. */
};

char *IFName = "br0";
char *ACName = "RTL_ADSL";
unsigned char localhwAddr[Eid_SZ];
unsigned int LocalIP;

unsigned char ServerIP[4] = {192, 168, 100, 1};
unsigned char ClientIP[4] = {192, 168, 100, 100}; /* Counter STARTS here */

#define DEFAULT_MAX_SESSIONS 32
struct ClientSession *Sessions = NULL;

int pppoe_server_socket = -1;
fd_set pppConnSet;
int maxpppConnFd;

/* Random seed for cookie generation */
#define SEED_LEN 16
unsigned char CookieSeed[SEED_LEN];

#define IPCP_OPT_ADDRESSES		0x01	/* both IP addresses; deprecated */
#define IPCP_OPT_COMPRESSION	0x02	/* IP compression protocol (VJ) */
#define IPCP_OPT_ADDRESS		0x03	/* local IP address */
#define IPCP_OPT_DNS_PRIMARY	0x81	/* primary DNS address */
#define IPCP_OPT_DNS_SECOND		0x83	/* second DNS address */

#endif

//extern int dbg_printf(const char * fmt, ...);
#ifdef ENABLE_PPP_SYSLOG
int dbg_printf(const char * fmt, ...)
{
	va_list args;
	unsigned char vChar, buff[256];

	va_start(args, fmt);
	vsprintf(buff, fmt, args);
	va_end(args);
	fflush(NULL);

	mib_get(MIB_SYSLOG, (void *)&vChar);
	if (vChar==1 && dbg_syslog == 1)
		syslog(LOG_INFO, buff);
	else
		printf(buff);
	return 0;
}
#endif

#ifdef ENABLE_PPP_SYSLOG
#define addlog		dbg_printf
#else
#define addlog		printf
#endif
#define	PPPOE_FMT		"%s%d : "
#define	PPPOE_OPT		"%s%d :    "
#define	PPPOE_ARGS(unit)	"pppoe", unit

// Kaohj
#define RECV_LEN	1024


//#define TIMEOUT(fun, arg1, arg2, handle) 	timeout(fun,arg1,arg2,&handle)
//#define UNTIMEOUT(fun, arg, handle)			untimeout(&handle)

//2014/3/4 Some PPPoE server do not accept End-Of-List Tag, so revmoe it.
//         According to PPPoE RFC 2516, End-Of-List Tag is not required.
//#define PPPOE_EOL_TAG 1

//N_SPPP has defined in if_sppp.h
//#define N_SPPP 8
unsigned int disc_counter[N_SPPP] = {0};	// Jenny, for discovery timeout counter
char pre_PADT_flag[N_SPPP] = {0};
char sendPADR_flag[N_SPPP] = {0};	// Jenny, for PADR flag
#ifdef ENABLE_PPP_SYSLOG
int dbg_syslog = 0;	// Jenny, for debug message output ; 1: syslog, 0: console
#endif
int pppoe_output(PPPOE_DRV_CTRL *p, void *buf, int data_len);

#define PPPOE_LOWER_SEND(p,m,l)    pppoe_output(p,m,l)
#define PPPOE_UPPER_INPUT(ifp,m)

//static void _pppoe_print_mac(const unsigned char *mac);
static void _pppoe_mac(const char *result, const unsigned char *mac);
const char *_pppoe_state_name(int state);
//static void pppoe_session_update(PPPOE_DRV_CTRL *p);
void pre_release_pppoe(char* interface_name);
void pppoe_up_log(int ifno, int value);

//PPPOE_DRV_CTRL pppoe_pool[N_PPPOE];
unsigned char bcastAddr[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
#ifdef CONFIG_USER_PPPOE_PROXY
int parsePADITags(PPPOEPROXY_DRV_CTRL *p, struct PPPoEPacket *packet)
{
	unsigned char *curTag, *tagValue;
	unsigned short tagType, tagLen;
	char temp[100];
	char *rpfcn = "[parsePADITags]";
	int debug = p->debug;
	int maxlen;

	memset(p, 0, sizeof(PPPOEPROXY_DRV_CTRL));
	memcpy(p->peerEthAddr, &packet->ehdr.from, ETH_ALEN);
	maxlen = (packet->length < (RECV_LEN-6)? packet->length:(RECV_LEN-6));
	if (maxlen < packet->length)
		printf("PPPoE payload overflow: %d\n", packet->length);

	curTag = packet->payload;
	while((curTag - packet->payload) < maxlen)
	{
		int mylen;
		tagType = (((unsigned short) curTag[0]) << 8) + (unsigned short) curTag[1];
		tagLen = (((unsigned short) curTag[2]) << 8) + (unsigned short) curTag[3];
		if (tagType == TAG_END_OF_LIST) return;
		tagValue = curTag+4;

		mylen = curTag+4+tagLen - packet->payload;
		if (mylen > maxlen) {
			break;
		}

		switch(tagType){
			case TAG_SERVICE_NAME:
				if (tagLen > 0 && tagLen < 100){
					memcpy(p->ServiceName, (char *)tagValue, tagLen);
				}
				break;
			case TAG_RELAY_SESSION_ID:
				if (tagLen > 0 && tagLen < 100){
					p->relayId.type = htons(TAG_RELAY_SESSION_ID);
					p->relayId.length = htons(tagLen);
					memcpy(p->relayId.payload, (char *)tagValue, tagLen);
				}
				break;
			case TAG_HOST_UNIQ:
				if (tagLen > 0 && tagLen < 100){
					p->hostUniq.type = htons(TAG_HOST_UNIQ);
					p->hostUniq.length = htons(tagLen);
					memcpy(p->hostUniq.payload, (char *)tagValue, tagLen);
				}
				break;
		}
		curTag = curTag + 4 + tagLen;
	}

}
#endif

/**********************************************************************
*%FUNCTION: parsePADOTags
*%ARGUMENTS:
* type -- tag type
* len -- tag length
* data -- tag data
* extra -- extra user data.  Should point to a PacketCriteria structure
*          which gets filled in according to selected AC name and service
*          name.
*%RETURNS: Nothing
*%DESCRIPTION: Picks interesting tags out of a PADO packet
***********************************************************************/

void parsePADOTags(PPPOE_DRV_CTRL *p, struct PPPoEPacket *packet, struct PacketCriteria *pc)
{
	unsigned char *curTag, *tagValue;
	unsigned short tagType, tagLen;
	char temp[100];
	char *rpfcn = "[parsePADOTags]";
	int debug = p->debug;
	unsigned char devAddr[MAC_ADDR_LEN];
	char ifname[16];
	// Kaohj
	int maxlen;

	// Kaohj ---- 6: pppoe header length
	maxlen = (packet->length < (RECV_LEN-Ehdr_SZ-6)? packet->length:(RECV_LEN-Ehdr_SZ-6));
	if (maxlen < packet->length)
		printf("PPPoE payload overflow: %d\n", packet->length);
	//printf("maxlen=%d\n", maxlen);
	curTag = packet->payload;
	// Kaohj
	//while((curTag - packet->payload) < packet->length)
	while((curTag - packet->payload) < maxlen)
	{
		// Kaohj
		int mylen;
		tagType = (((unsigned short) curTag[0]) << 8) + (unsigned short) curTag[1];
		tagLen = (((unsigned short) curTag[2]) << 8) + (unsigned short) curTag[3];
		if (tagType == TAG_END_OF_LIST) return;
		tagValue = curTag+4;
		// Kaohj
		mylen = curTag+4+tagLen - packet->payload;
		if (mylen > maxlen) {
			//printf("mylen exceeds\n");
			break;
		}

		switch(tagType)
		{
			case TAG_AC_NAME:
				if (tagLen > 0 && tagLen < 100)
				{
					memcpy(temp, (char *)tagValue, tagLen);
					temp[tagLen] = 0;
					strcpy(p->ACName, temp);
					if(debug)
						addlog(PPPOE_OPT"AC-Name: %s\n", PPPOE_ARGS(p->ifno), temp);
				}
//				if (strlen(p->ACName))
//					if (tagLen == strlen(p->ACName) && !strncmp((char *) tagValue, p->ACName, tagLen))
				if (strlen(p->DesiredACName)) {
					if (tagLen == strlen(p->DesiredACName) && !strncmp((char *) tagValue, p->DesiredACName, tagLen))
						pc->acNameOK = 1;
				}
				else
					pc->acNameOK = 1;
				break;
			case TAG_SERVICE_NAME:
				// Service name NULL issue
//				if (strlen(p->ServiceName) != 0) // Jenny
//					break;
				if (tagLen > 0 && tagLen < 100)
				{
					memcpy(temp, (char *)tagValue, tagLen);
					temp[tagLen] = 0;
					// Service name NULL issue
					if (!strlen(p->ServiceName))
						strcpy(p->ServiceName, temp);
					if(debug)
						addlog(PPPOE_OPT"Service-Name: %s\n", PPPOE_ARGS(p->ifno), temp);
				}
				if (strlen(p->ServiceName)) {
					if (tagLen == strlen(p->ServiceName) && !strncmp((char *) tagValue, p->ServiceName, tagLen))
						pc->serviceNameOK = 1;
				}
				else
					pc->serviceNameOK = 1;
				break;
			case TAG_AC_COOKIE:
				p->cookie.type = htons(tagType);
				p->cookie.length = htons(tagLen);
				memcpy(p->cookie.payload, tagValue, tagLen);
				break;
			case TAG_RELAY_SESSION_ID:
				p->relayId.type = htons(tagType);
				p->relayId.length = htons(tagLen);
				memcpy(p->relayId.payload, tagValue, tagLen);
				break;
			case TAG_SERVICE_NAME_ERROR:
				syslog(LOG_INFO, "spppd: ppp%d: PPPoE service name error: %.*s\n", p->ifno, (int) tagLen, tagValue);
				if(debug)
					addlog(PPPOE_OPT"Service-Name-Error: %.*s\n", PPPOE_ARGS(p->ifno), (int) tagLen, tagValue);
				return;
//				break;
			case TAG_AC_SYSTEM_ERROR:
				syslog(LOG_INFO, "spppd: ppp%d: PPPoE AC system error: %.*s\n", p->ifno, (int) tagLen, tagValue);
				if(debug)
					addlog(PPPOE_OPT"System-Error: %.*s\n", PPPOE_ARGS(p->ifno), (int) tagLen, tagValue);
				return;
//				break;
			case TAG_GENERIC_ERROR:
				syslog(LOG_INFO, "spppd: ppp%d: PPPoE generic error: %.*s\n", p->ifno, (int) tagLen, tagValue);
				if(debug)
					addlog(PPPOE_OPT"Generic-Error: %.*s\n", PPPOE_ARGS(p->ifno), (int) tagLen, tagValue);
				return;
//				break;
			case TAG_HOST_UNIQ:		// Jenny, parse TAG_HOST_UNIQ to verify packets
				memcpy(temp, (char *)tagValue, tagLen);
				temp[tagLen] = 0;
				memset(ifname,0,sizeof(ifname));
				mib_get(MIB_ELAN_MAC_ADDR, (void *)devAddr);
				memcpy(ifname,devAddr,MAC_ADDR_LEN);
				sprintf(ifname+MAC_ADDR_LEN, "%s%d", PPPOE_ARGS(p->ifno));
				if(debug)
					addlog(PPPOE_OPT"Host-Uniq: %s\n", PPPOE_ARGS(p->ifno), temp);
				if (memcmp(temp, ifname, tagLen))
					return;
		}
		curTag = curTag + 4 + tagLen;
	}
}

#ifdef CONFIG_USER_PPPOE_PROXY
int parsePADRTags(PPPOEPROXY_DRV_CTRL *p, struct PPPoEPacket *packet)
{
	unsigned char *curTag, *tagValue;
	unsigned short tagType, tagLen;
	char temp[100];
	char *rpfcn = "[parsePADRTags]";
	int debug = p->debug;
	int maxlen;

	memset(p, 0, sizeof(PPPOEPROXY_DRV_CTRL));
	memcpy(p->peerEthAddr, packet->ehdr.from.octet, ETH_ALEN);
	maxlen = (packet->length < (RECV_LEN-Ehdr_SZ-6)? packet->length:(RECV_LEN-Ehdr_SZ-6));
	if (maxlen < packet->length)
		printf("PPPoE payload overflow: %d\n", packet->length);

	curTag = packet->payload;
	while((curTag - packet->payload) < maxlen)
	{
		int mylen;
		tagType = (((unsigned short) curTag[0]) << 8) + (unsigned short) curTag[1];
		tagLen = (((unsigned short) curTag[2]) << 8) + (unsigned short) curTag[3];
		if (tagType == TAG_END_OF_LIST) return;
		tagValue = curTag+4;

		mylen = curTag+4+tagLen - packet->payload;
		if (mylen > maxlen) {
			break;
		}

		switch(tagType){
			case TAG_SERVICE_NAME:
				if (tagLen > 0 && tagLen < 100){
					memcpy(p->ServiceName, (char *)tagValue, tagLen);
				}
				break;
			case TAG_RELAY_SESSION_ID:
				if (tagLen > 0 && tagLen < 100){
					p->relayId.type = htons(TAG_RELAY_SESSION_ID);
					p->relayId.length = htons(tagLen);
					memcpy(p->relayId.payload, (char *)tagValue, tagLen);
				}
				break;
			case TAG_HOST_UNIQ:
				if (tagLen > 0 && tagLen < 100){
					p->hostUniq.type = htons(TAG_HOST_UNIQ);
					p->hostUniq.length = htons(tagLen);
					memcpy(p->hostUniq.payload, (char *)tagValue, tagLen);
				}
				break;
			case TAG_AC_COOKIE:
				if (tagLen > 0 && tagLen < 100){
					p->cookie.type = htons(TAG_AC_COOKIE);
					p->cookie.length = htons(tagLen);
					memcpy(p->cookie.payload, (char *)tagValue, tagLen);
				}
				break;
		}
		curTag = curTag + 4 + tagLen;
	}

}
#endif

/**********************************************************************
*%FUNCTION: parsePADSTags
*%ARGUMENTS:
* type -- tag type
* len -- tag length
* data -- tag data
* extra -- extra user data
*%RETURNS: 1 correct, 0 something error
*%DESCRIPTION: Picks interesting tags out of a PADS packet
***********************************************************************/

int parsePADSTags(PPPOE_DRV_CTRL *p, struct PPPoEPacket *packet)
{
	unsigned char *curTag, *tagValue;
	unsigned short tagType, tagLen;
	int debug = p->debug;
	char *rpfcn = "[parsePADSTags]";
	unsigned char devAddr[MAC_ADDR_LEN];
	char ifname[16], temp[100];
	// Kaohj
	int maxlen;
	int gotCorrectHostUniq = 0;

	// Kaohj ---- 6: pppoe header length
	maxlen = (packet->length < (RECV_LEN-Ehdr_SZ-6)? packet->length:(RECV_LEN-Ehdr_SZ-6));
	if (maxlen < packet->length)
		printf("PPPoE payload overflow: %d\n", packet->length);
	//printf("maxlen=%d\n", maxlen);
	curTag = packet->payload;
	// Kaohj
	//while((curTag - packet->payload) < packet->length)
	while((curTag - packet->payload) < maxlen)
	{
		// Kaohj
		int mylen;
		tagType = (((unsigned short) curTag[0]) << 8) + (unsigned short) curTag[1];
		tagLen = (((unsigned short) curTag[2]) << 8) + (unsigned short) curTag[3];
		if (tagType == TAG_END_OF_LIST)
		{
			if(gotCorrectHostUniq==0)
			{
				if(debug)
					addlog(PPPOE_OPT"Host-Uniq-Error: Not got Host-Uniq Tag\n", PPPOE_ARGS(p->ifno));
				return 0;
			}
			break;
		}

		tagValue = curTag+4;
		// Kaohj
		mylen = curTag+4+tagLen - packet->payload;
		if (mylen > maxlen) {
			//printf("mylen exceeds\n");
			break;
		}

		switch(tagType) {
			case TAG_AC_NAME:
				if(debug)
					addlog(PPPOE_OPT"AC-Name: %.*s\n", PPPOE_ARGS(p->ifno), (int) tagLen, tagValue);
				break;
			case TAG_SERVICE_NAME:
				if(debug)
					addlog(PPPOE_OPT"Service-Name: %.*s\n", PPPOE_ARGS(p->ifno), (int) tagLen, tagValue);
				break;
			case TAG_SERVICE_NAME_ERROR:
				syslog(LOG_INFO, "spppd: ppp%d: PPPoE service name error: %.*s\n", p->ifno, (int) tagLen, tagValue);
				if(debug)
					addlog(PPPOE_OPT"Service-Name-Error: %.*s\n", PPPOE_ARGS(p->ifno), (int) tagLen, tagValue);
				//curTag = curTag + 4 + tagLen;
				return 0;
				break;
			case TAG_AC_SYSTEM_ERROR:
				syslog(LOG_INFO, "spppd: ppp%d: PPPoE AC system error: %.*s\n", p->ifno, (int) tagLen, tagValue);
				if(debug)
					addlog(PPPOE_OPT"System-Error: %.*s\n", PPPOE_ARGS(p->ifno), (int) tagLen, tagValue);
				//curTag = curTag + 4 + tagLen;
				return 0;
				break;
			case TAG_GENERIC_ERROR:
				syslog(LOG_INFO, "spppd: ppp%d: PPPoE generic error: %.*s\n", p->ifno, (int) tagLen, tagValue);
				if(debug)
					addlog(PPPOE_OPT"Generic-Error: %.*s\n", PPPOE_ARGS(p->ifno), (int) tagLen, tagValue);
				//curTag = curTag + 4 + tagLen;
				return 0;
				break;
			case TAG_RELAY_SESSION_ID:
				p->relayId.type = htons(tagType);
				p->relayId.length = htons(tagLen);
				memcpy(p->relayId.payload, tagValue, tagLen);
				break;
			case TAG_HOST_UNIQ:		// Jenny, parse TAG_HOST_UNIQ to verify packets
				memcpy(temp, (char *)tagValue, tagLen);
				temp[tagLen] = 0;
				mib_get(MIB_ELAN_MAC_ADDR, (void *)devAddr);
				memset(ifname,0,sizeof(ifname));
				memcpy(ifname,devAddr,MAC_ADDR_LEN);
				sprintf(ifname+MAC_ADDR_LEN, "%s%d", PPPOE_ARGS(p->ifno));
				if(debug)
					addlog(PPPOE_OPT"Host-Uniq: %s\n", PPPOE_ARGS(p->ifno), temp);
				if (memcmp(temp, ifname, tagLen)) {
					//curTag = curTag + 4 + tagLen;
					if(debug)
						addlog(PPPOE_OPT"Host-Uniq not match: [%s] vs [%s]\n", PPPOE_ARGS(p->ifno), temp, ifname);
					return 0;
				}
				else
					gotCorrectHostUniq = 1;
				break;
		}
		curTag = curTag + 4 + tagLen;
	}
	return 1;
}

/**********************************************************************
*%FUNCTION: parseLogErrs
*%ARGUMENTS:
* type -- tag type
* len -- tag length
* data -- tag data
* extra -- extra user data
*%RETURNS: Nothing
*%DESCRIPTION: Picks error tags out of a packet and logs them.
***********************************************************************/
void parsePADTTags(PPPOE_DRV_CTRL *p, struct PPPoEPacket *packet)
{
	unsigned char *curTag, *tagValue;
	unsigned short tagType, tagLen;
	int debug = p->debug;
	char *rpfcn = "[parsePADTTags]";
	// Kaohj
	int maxlen;

	// Kaohj ---- 6: pppoe header length
	maxlen = (packet->length < (RECV_LEN-Ehdr_SZ-6)? packet->length:(RECV_LEN-Ehdr_SZ-6));
	if (maxlen < packet->length)
		printf("PPPoE payload overflow: %d\n", packet->length);
	//printf("maxlen=%d\n", maxlen);
	curTag = packet->payload;
	// Kaohj
	//while((curTag - packet->payload) < packet->length)
	while((curTag - packet->payload) < maxlen)
	{
		// Kaohj
		int mylen;
		tagType = (((unsigned short) curTag[0]) << 8) + (unsigned short) curTag[1];
		tagLen = (((unsigned short) curTag[2]) << 8) + (unsigned short) curTag[3];
		if (tagType == TAG_END_OF_LIST) return;
		tagValue = curTag+4;
		// Kaohj
		mylen = curTag+4+tagLen - packet->payload;
		if (mylen > maxlen) {
			//printf("mylen exceeds\n");
			break;
		}

		switch(tagType)
		{
			case TAG_SERVICE_NAME_ERROR:
				if(debug)
					addlog(PPPOE_OPT"Service-Name-Error: %.*s\n", PPPOE_ARGS(p->ifno), (int) tagLen, tagValue);
				return;
//				break;
			case TAG_AC_SYSTEM_ERROR:
				if(debug)
					addlog(PPPOE_OPT"System-Error: %.*s\n", PPPOE_ARGS(p->ifno), (int) tagLen, tagValue);
				return;
//				break;
			case TAG_GENERIC_ERROR:
				if(debug)
					addlog(PPPOE_OPT"Generic-Error: %.*s\n", PPPOE_ARGS(p->ifno), (int) tagLen, tagValue);
				return;
//				break;
		}
		curTag = curTag + 4 + tagLen;
	}
}

int pppoe_change_state(PPPOE_DRV_CTRL *p, int state)
{
	p->DiscoveryState = state;
	ppp_status();
	return 0;
}

#ifdef CONFIG_USER_PPPOE_PROXY
void genCookie(unsigned char const *peerEthAddr,
	  unsigned char const *myEthAddr,
	  unsigned char const *seed,
	  unsigned char *cookie)
{
    struct MD5Context ctx;

    MD5Init(&ctx);
    MD5Update(&ctx, peerEthAddr, ETH_ALEN);
    MD5Update(&ctx, myEthAddr, ETH_ALEN);
    MD5Update(&ctx, seed, SEED_LEN);
    MD5Final(cookie, &ctx);
}
#endif

/***********************************************************************
*%FUNCTION: sendPADI
*%ARGUMENTS: None
*%RETURNS: 0 success, 1 fail
*%DESCRIPTION: Sends a PADI packet
***********************************************************************/
void sendPADI(PPPOE_DRV_CTRL *p)
{
//	unsigned char pkt_buf[256];
	unsigned char pkt_buf[RECV_LEN];
	struct PPPoEPacket *packet;
	unsigned char *cursor;
	struct PPPoETag *svc;
#ifdef PPPOE_EOL_TAG
	struct PPPoETag eol;	//EOL
#endif
	unsigned short namelen = 0;
	unsigned short plen;
	int len;
	int debug = p->debug;
	char *rpfcn = "[sendPADI]";
	char ifname[16];

	packet = (struct PPPoEPacket *)pkt_buf;
	svc = (struct PPPoETag *)packet->payload;
	cursor = packet->payload;
//	svc = (struct PPPoETag *)(&packet->payload);

	if (p->ServiceName)	// Jenny
		namelen = (unsigned short)strlen(p->ServiceName);
	plen = TAG_HDR_SIZE + namelen;
	CHECK_ROOM(cursor, packet->payload, plen);	// Jenny

	memset(p->remote.sll_addr, 0xff, ETH_ALEN);
	memcpy(&packet->ehdr.to, bcastAddr, Eid_SZ);
	memcpy(&packet->ehdr.from, p->MyEthAddr, Eid_SZ);
//	bcopy((char*)bcastAddr, (char*)&packet->ehdr.to, Eid_SZ);
//	bcopy((char*)p->MyEthAddr, (char*)&packet->ehdr.from, Eid_SZ);
	packet->ehdr.type = htons(ETH_PPPOE_DISCOVERY);

	packet->ver_type = 0x11;
	packet->code = CODE_PADI;
	packet->session = 0;

	svc->type = TAG_SERVICE_NAME;
	svc->length = htons(namelen);
	CHECK_ROOM(cursor, packet->payload, namelen + TAG_HDR_SIZE);	// Jenny
	if (p->ServiceName)	// Jenny
		memcpy(svc->payload, p->ServiceName, namelen);
//	bcopy((char*)p->ServiceName, (char*)svc->payload, strlen(p->ServiceName));
	cursor += namelen + TAG_HDR_SIZE;

	// Jenny, add TAG_HOST_UNIQ to better support multiple PPPoE sessions
	{
		unsigned char devAddr[MAC_ADDR_LEN];
		struct PPPoETag hostUniq;
		unsigned short ifnamelen = 0;

		hostUniq.type = htons(TAG_HOST_UNIQ);
		mib_get(MIB_ELAN_MAC_ADDR, (void *)devAddr);
		memset(ifname,0,sizeof(ifname));
		memcpy(ifname,devAddr,MAC_ADDR_LEN);
		sprintf(ifname+MAC_ADDR_LEN, "%s%d", PPPOE_ARGS(p->ifno));
		ifnamelen = sizeof(ifname);
		hostUniq.length = (unsigned int)ifnamelen;
		memcpy(hostUniq.payload, ifname, ifnamelen);
		CHECK_ROOM(cursor, hostUniq.payload, ifnamelen + TAG_HDR_SIZE);
		memcpy(cursor, &hostUniq, ifnamelen + TAG_HDR_SIZE);
		cursor += ifnamelen + TAG_HDR_SIZE;
		plen += ifnamelen + TAG_HDR_SIZE;
	}

    // If we're using Host-Uniq, copy it over
    /*if (p->optUseHostUnique) {
		struct PPPoETag hostUniq;
		hostUniq.type = htons(TAG_HOST_UNIQ);
		hostUniq.length = htons(sizeof(cur_task));
		memcpy(hostUniq.payload, &cur_task, sizeof(cur_task));
		memcpy(cursor, &hostUniq, sizeof(cur_task) + TAG_HDR_SIZE);
		plen += sizeof(cur_task) + TAG_HDR_SIZE;
    }*/

#ifdef PPPOE_EOL_TAG	// EOL
    	// Jenny, support Tag End-Of-List
	eol.type = htons(TAG_END_OF_LIST);
	eol.length = htons(0);
	memcpy(cursor, &eol, TAG_HDR_SIZE);
	plen += TAG_HDR_SIZE;
#endif

	packet->length = htons(plen);
	len = (int) (plen + PPPOE_HDR_SIZE);
	PPPOE_LOWER_SEND(p, packet, len);

#if  !defined(CONFIG_USER_PPPOE_2_INTERVALS_RETRY) && !defined(CONFIG_USER_PPPOE_CTC_DELAY_INT)
	TIMEOUT(sendPADI, p, p->timeout, p->ch);
	p->timeout *= 2;
	if (p->timeout >= 16)
		p->timeout = 1;
#endif

	if (debug)
	{
		/*addlog(PPPOE_FMT"O PADI [%s] from ", PPPOE_ARGS(p->ifno), _pppoe_state_name(p->DiscoveryState));
		_pppoe_print_mac((char*)&packet->ehdr.from);
		addlog("\n");*/
		char mac[17];
		mac[0] = 0;
		_pppoe_mac(mac, (char*)&packet->ehdr.from);
		addlog(PPPOE_FMT"O PADI [%s] from %s\n", PPPOE_ARGS(p->ifno), _pppoe_state_name(p->DiscoveryState), mac);
		addlog(PPPOE_OPT"Service-Name: %s\n", PPPOE_ARGS(p->ifno), p->ServiceName);
		addlog(PPPOE_OPT"Host-Uniq: %s\n", PPPOE_ARGS(p->ifno), ifname);
	}
	pppoe_up_log(p->ifno, 0);	// Jenny
	pppoe_change_state(p, PPPOE_STATE_SENT_PADI);

	return;
}

#ifdef CONFIG_USER_PPPOE_PROXY
int sendPADO(PPPOEPROXY_DRV_CTRL *p, struct sockaddr_ll *addr)
{
	unsigned char pkt_buf[RECV_LEN];
	struct PPPoEPacket *packet;
	struct PPPoETag acname;
    struct PPPoETag servname;
    struct PPPoETag cookie;
    unsigned short acname_len;
	unsigned short servname_len;
	unsigned char *cursor;
	unsigned short plen = 0;
	unsigned short len = 0;
	char *rpfcn = "[sendPADO]";
	int ret;

	packet = (struct PPPoEPacket *)pkt_buf;
	cursor =  packet->payload;

	memcpy(&packet->ehdr.to, p->peerEthAddr, Eid_SZ);
	memcpy(&packet->ehdr.from, localhwAddr, Eid_SZ);

	packet->ehdr.type = htons(ETH_PPPOE_DISCOVERY);
	packet->ver_type = 0x11;
	packet->code = CODE_PADO;
	packet->session = 0;

	acname.type = htons(TAG_AC_NAME);
    acname_len = strlen(ACName);
    acname.length = htons(acname_len);
    memcpy(acname.payload, ACName, acname_len);

	if (p->ServiceName)
		servname_len = (unsigned short)strlen(p->ServiceName);
	servname.type = htons(TAG_SERVICE_NAME);
	servname.length = htons(servname_len);
	memcpy(servname.payload, p->ServiceName, servname_len);

	cookie.type = htons(TAG_AC_COOKIE);
    cookie.length = htons(16);		/* MD5 output is 16 bytes */
    genCookie(packet->ehdr.to.octet, localhwAddr, CookieSeed, cookie.payload);

	CHECK_ROOM(cursor, packet->payload, acname_len+TAG_HDR_SIZE);
    memcpy(cursor, &acname, acname_len + TAG_HDR_SIZE);
    cursor += acname_len + TAG_HDR_SIZE;
	plen += TAG_HDR_SIZE + acname_len;

    CHECK_ROOM(cursor, packet->payload, servname_len+TAG_HDR_SIZE);
    memcpy(cursor, &servname, TAG_HDR_SIZE);
    cursor += TAG_HDR_SIZE;
    plen += TAG_HDR_SIZE;

    CHECK_ROOM(cursor, packet->payload, TAG_HDR_SIZE + 16);
    memcpy(cursor, &cookie, TAG_HDR_SIZE + 16);
    cursor += TAG_HDR_SIZE + 16;
    plen += TAG_HDR_SIZE + 16;

	if (p->relayId.type) {
		CHECK_ROOM(cursor, packet->payload, ntohs(p->relayId.length) + TAG_HDR_SIZE);
		memcpy(cursor, &p->relayId, ntohs(p->relayId.length) + TAG_HDR_SIZE);
		cursor += ntohs(p->relayId.length) + TAG_HDR_SIZE;
		plen += ntohs(p->relayId.length) + TAG_HDR_SIZE;
    }

    if (p->hostUniq.type) {
		CHECK_ROOM(cursor, packet->payload, ntohs(p->hostUniq.length)+TAG_HDR_SIZE);
		memcpy(cursor, &p->hostUniq, ntohs(p->hostUniq.length) + TAG_HDR_SIZE);
		cursor += ntohs(p->hostUniq.length) + TAG_HDR_SIZE;
		plen += ntohs(p->hostUniq.length) + TAG_HDR_SIZE;
		unsigned char *c = cursor - 12;
    }

    packet->length = htons(plen);
	len = (int) (plen + PPPOE_HDR_SIZE);

	ret = sendto(pppoe_server_socket, packet, len+Ehdr_SZ, 0, (struct sockaddr*)addr, sizeof(struct sockaddr_ll));
	if(ret < 0){
		printf("send PADO error! ret = %d\n", ret);
		perror("Could not send data: ");
	}

}
#endif

/***********************************************************************
*%FUNCTION: sendPADR
*%ARGUMENTS: None
*%RETURNS: 0 success 1 fail
*%DESCRIPTION: Sends a PADR packet
***********************************************************************/
int sendPADR(PPPOE_DRV_CTRL *p)
{
//	unsigned char pkt_buf[256];
	unsigned char pkt_buf[RECV_LEN];
	struct PPPoEPacket *packet;
	struct PPPoETag *svc;
	unsigned char *cursor;
	unsigned short namelen = 0;
	unsigned short plen;
	int len;
	int debug = p->debug;
	char *rpfcn = "[sendPADR]";
	char ifname[16];

	packet = (struct PPPoEPacket *)pkt_buf;
	svc = (struct PPPoETag *)packet->payload;
	cursor =  packet->payload;
	if (p->ServiceName)	// Jenny
		namelen = (unsigned short)strlen(p->ServiceName);
	plen = TAG_HDR_SIZE + namelen;
	CHECK_ROOM(cursor, packet->payload, plen);	// Jenny

	memcpy(&packet->ehdr.to, p->ACEthAddr, Eid_SZ);
	memcpy(&packet->ehdr.from, p->MyEthAddr, Eid_SZ);
//	bcopy((char*)p->ACEthAddr, (char*)&packet->ehdr.to, Eid_SZ);
//	bcopy((char*)p->MyEthAddr, (char*)&packet->ehdr.from, Eid_SZ);

	packet->ehdr.type = htons(ETH_PPPOE_DISCOVERY);
	packet->ver_type = 0x11;
	packet->code = CODE_PADR;
	packet->session = 0;

	svc->type = TAG_SERVICE_NAME;
	svc->length = htons(namelen);
	if (p->ServiceName)	// Jenny
		memcpy(svc->payload, p->ServiceName, namelen);
//	bcopy((char*)p->ServiceName, (char*)svc->payload, strlen(p->ServiceName));

	cursor += namelen + TAG_HDR_SIZE;

	// Jenny, add TAG_HOST_UNIQ to better support multiple PPPoE sessions
	{
		unsigned char devAddr[MAC_ADDR_LEN];
		struct PPPoETag hostUniq;
		unsigned short ifnamelen = 0;

		hostUniq.type = htons(TAG_HOST_UNIQ);
		mib_get(MIB_ELAN_MAC_ADDR, (void *)devAddr);
		memset(ifname,0,sizeof(ifname));
		memcpy(ifname,devAddr,MAC_ADDR_LEN);
		sprintf(ifname+MAC_ADDR_LEN, "%s%d", PPPOE_ARGS(p->ifno));
		ifnamelen = sizeof(ifname);
		hostUniq.length = (unsigned int)ifnamelen;
		memcpy(hostUniq.payload, ifname, ifnamelen);
		CHECK_ROOM(cursor, hostUniq.payload, ifnamelen + TAG_HDR_SIZE);
		memcpy(cursor, &hostUniq, ifnamelen + TAG_HDR_SIZE);
		cursor += ifnamelen + TAG_HDR_SIZE;
		plen += ifnamelen + TAG_HDR_SIZE;
	}

    /* If we're using Host-Uniq, copy it over */
    /*if (p->optUseHostUnique) {
		struct PPPoETag hostUniq;
		hostUniq.type = htons(TAG_HOST_UNIQ);
		hostUniq.length = htons(sizeof(cur_task));
		memcpy(hostUniq.payload, &cur_task, sizeof(cur_task));
		memcpy(cursor, &hostUniq, sizeof(cur_task) + TAG_HDR_SIZE);
		plen += sizeof(cur_task) + TAG_HDR_SIZE;
    }*/

	/* Copy cookie and relay-ID if needed */
	if (p->cookie.type) {
		CHECK_ROOM(cursor, packet->payload, ntohs(p->cookie.length) + TAG_HDR_SIZE);	// Jenny
		memcpy(cursor, &p->cookie, ntohs(p->cookie.length) + TAG_HDR_SIZE);
		cursor += ntohs(p->cookie.length) + TAG_HDR_SIZE;
		plen += ntohs(p->cookie.length) + TAG_HDR_SIZE;
	}

	if (p->relayId.type) {
		CHECK_ROOM(cursor, packet->payload, ntohs(p->relayId.length) + TAG_HDR_SIZE);	// Jenny
		memcpy(cursor, &p->relayId, ntohs(p->relayId.length) + TAG_HDR_SIZE);
		cursor += ntohs(p->relayId.length) + TAG_HDR_SIZE;
		plen += ntohs(p->relayId.length) + TAG_HDR_SIZE;
	}

	packet->length = htons(plen);
	len = (int) (plen + PPPOE_HDR_SIZE);
	PPPOE_LOWER_SEND(p, packet, len);
	TIMEOUT(sendPADR, p, p->timeout, p->ch);

	if (debug)
	{
		/*addlog(PPPOE_FMT"O PADR [%s] from ", PPPOE_ARGS(p->ifno), _pppoe_state_name(p->DiscoveryState));
		_pppoe_print_mac((char*)&packet->ehdr.from);
		addlog(" to ");
		_pppoe_print_mac((char*)&packet->ehdr.to);
		addlog("\n");*/
		char mac1[17], mac2[17];
		mac1[0] = mac2[0] = 0;
		_pppoe_mac(mac1, (char*)&packet->ehdr.from);
		_pppoe_mac(mac2, (char*)&packet->ehdr.to);
		addlog(PPPOE_FMT"O PADR [%s] from %s to %s\n", PPPOE_ARGS(p->ifno), _pppoe_state_name(p->DiscoveryState), mac1, mac2);
		addlog(PPPOE_OPT"Service-Name: %s\n", PPPOE_ARGS(p->ifno), p->ServiceName);
		addlog(PPPOE_OPT"Host-Uniq: %s\n", PPPOE_ARGS(p->ifno), ifname);
	}

	sendPADR_flag[p->ifno] = 1;
	pppoe_up_log(p->ifno, 0);	// Jenny
	pppoe_change_state(p, PPPOE_STATE_SENT_PADR);

	p->timeout *= 2;
#if  !defined(CONFIG_USER_PPPOE_2_INTERVALS_RETRY) && !defined(CONFIG_USER_PPPOE_CTC_DELAY_INT)
	if (p->timeout >= 16) {
		p->timeout = 1;
		pppoe_change_state(p, PPPOE_STATE_IDLE);
		UNTIMEOUT(0, 0, p->ch);
		TIMEOUT(sendPADI, p, p->timeout, p->ch);
	}
#endif

	return 0;
}

#ifdef CONFIG_USER_PPPOE_PROXY
void sendErrorPADS(PPPOEPROXY_DRV_CTRL *p, struct sockaddr_ll *dst, int errorTag, char *errorMsg)
{
	unsigned char pkt_buf[RECV_LEN];
    struct PPPoEPacket *pads;
    unsigned char *cursor;
    unsigned short plen = 0;
	unsigned short len = 0;
    struct PPPoETag err;
	int ret;

    int elen = strlen(errorMsg);
	pads = (struct PPPoEPacket *)pkt_buf;
	cursor = pads->payload;
    memcpy(&pads->ehdr.to, p->peerEthAddr, Eid_SZ);
	memcpy(&pads->ehdr.from, localhwAddr, Eid_SZ);
	pads->ehdr.type = htons(ETH_PPPOE_DISCOVERY);
	pads->ver_type = 0x11;
    pads->code = CODE_PADS;

    pads->session = htons(0);
    plen = 0;

    err.type = htons(errorTag);
    err.length = htons(elen);

    memcpy(err.payload, errorMsg, elen);
    memcpy(cursor, &err, TAG_HDR_SIZE+elen);
    cursor += TAG_HDR_SIZE + elen;
    plen += TAG_HDR_SIZE + elen;

    if (p->relayId.type) {
		memcpy(cursor, &p->relayId, ntohs(p->relayId.length) + TAG_HDR_SIZE);
		cursor += ntohs(p->relayId.length) + TAG_HDR_SIZE;
		plen += ntohs(p->relayId.length) + TAG_HDR_SIZE;
    }
    if (p->hostUniq.type) {
		memcpy(cursor, &p->hostUniq, ntohs(p->hostUniq.length) + TAG_HDR_SIZE);
		cursor += ntohs(p->hostUniq.length) + TAG_HDR_SIZE;
		plen += ntohs(p->hostUniq.length) + TAG_HDR_SIZE;
    }

    pads->length = htons(plen);
	len = (int) (plen + PPPOE_HDR_SIZE);

	ret = sendto(pppoe_server_socket, pads, len+Ehdr_SZ, 0, (struct sockaddr*)dst, sizeof(struct sockaddr_ll));
	if(ret < 0){
		printf("send errorPADS error! ret = %d\n", ret);
		perror("Could not send data: ");
	}
}

int sendServerPADT(struct sppp *sp)
{
	unsigned char pkt_buf[RECV_LEN];
	struct PPPoEPacket *packet;
#ifdef PPPOE_EOL_TAG
	struct PPPoETag eol;	// EOL
#endif
	unsigned char *cursor;
	unsigned short plen = 0;
	int len;
	char *rpfcn = "[sendServerPADT]";
	int ret;
	PPPOEPROXY_DRV_CTRL *p = sp->pp_lowerp;

	if (!p->session) {
		return 1;
	}

	packet = (struct PPPoEPacket *)pkt_buf;
	cursor = packet->payload;

	memcpy(&packet->ehdr.to, p->peerEthAddr, Eid_SZ);
	memcpy(&packet->ehdr.from, localhwAddr, Eid_SZ);

	packet->ehdr.type = htons(ETH_PPPOE_DISCOVERY);
	packet->ver_type = 0x11;
	packet->code = CODE_PADT;
	packet->session = p->session;

	/* Reset Session to zero so there is no possibility of
       recursive calls to this function by any signal handler */
	p->session = 0;

	/* Copy cookie and relay-ID if needed */
	if (p->cookie.type) {
		CHECK_ROOM(cursor, packet->payload, ntohs(p->cookie.length) + TAG_HDR_SIZE);
		memcpy(cursor, &p->cookie, ntohs(p->cookie.length) + TAG_HDR_SIZE);
		cursor += ntohs(p->cookie.length) + TAG_HDR_SIZE;
		plen += ntohs(p->cookie.length) + TAG_HDR_SIZE;
	}

	if (p->relayId.type) {
		CHECK_ROOM(cursor, packet->payload, ntohs(p->relayId.length) + TAG_HDR_SIZE);
		memcpy(cursor, &p->relayId, ntohs(p->relayId.length) + TAG_HDR_SIZE);
		cursor += ntohs(p->relayId.length) + TAG_HDR_SIZE;
		plen += ntohs(p->relayId.length) + TAG_HDR_SIZE;
	}

#ifdef PPPOE_EOL_TAG
	eol.type = htons(TAG_END_OF_LIST);
	eol.length = htons(0);
	memcpy(cursor, &eol, TAG_HDR_SIZE);
	plen += TAG_HDR_SIZE;
#endif

	packet->length = htons(plen);
	len = (int) (plen + PPPOE_HDR_SIZE);

	ret = sendto(pppoe_server_socket, packet, len+Ehdr_SZ, 0, (struct sockaddr*)&p->client, sizeof(struct sockaddr_ll));
	if(ret < 0){
		printf("send serverPADT error! ret = %d\n", ret);
		perror("Could not send data: ");
	}

	p->session = PPPOE_STATE_IDLE;
	return 0;
}

int sendPADS(PPPOEPROXY_DRV_CTRL *p, unsigned int sess, struct sockaddr_ll *addr)
{

	unsigned char pkt_buf[RECV_LEN];
	struct PPPoEPacket *packet;
	struct PPPoETag servname;
	struct PPPoETag cookie;
	unsigned short servname_len;
	unsigned char *cursor;
	unsigned short plen = 0;
	unsigned short len = 0;
	char *rpfcn = "[sendPADS]";
	int ret;

	packet = (struct PPPoEPacket *)pkt_buf;
	cursor =  packet->payload;

	memcpy(&packet->ehdr.to, p->peerEthAddr, Eid_SZ);
	memcpy(&packet->ehdr.from, localhwAddr, Eid_SZ);

	packet->ehdr.type = htons(ETH_PPPOE_DISCOVERY);
	packet->ver_type = 0x11;
	packet->code = CODE_PADS;
	packet->session = sess;

	if (p->ServiceName)
		servname_len = (unsigned short)strlen(p->ServiceName);
	servname.type = htons(TAG_SERVICE_NAME);
	servname.length = htons(servname_len);
	memcpy(servname.payload, p->ServiceName, servname_len);

	cookie.type = htons(TAG_AC_COOKIE);
	cookie.length = htons(16);		/* MD5 output is 16 bytes */
	genCookie(packet->ehdr.to.octet, localhwAddr, CookieSeed, cookie.payload);

	CHECK_ROOM(cursor, packet->payload, servname_len+TAG_HDR_SIZE);
	memcpy(cursor, &servname, TAG_HDR_SIZE);
	cursor += TAG_HDR_SIZE + servname_len;
	plen += TAG_HDR_SIZE + servname_len;

	CHECK_ROOM(cursor, packet->payload, TAG_HDR_SIZE + 16);
	memcpy(cursor, &cookie, TAG_HDR_SIZE + 16);
	cursor += TAG_HDR_SIZE + 16;
	plen += TAG_HDR_SIZE + 16;

	if (p->relayId.type) {
		CHECK_ROOM(cursor, packet->payload, ntohs(p->relayId.length) + TAG_HDR_SIZE);
		memcpy(cursor, &p->relayId, ntohs(p->relayId.length) + TAG_HDR_SIZE);
		cursor += ntohs(p->relayId.length) + TAG_HDR_SIZE;
		plen += ntohs(p->relayId.length) + TAG_HDR_SIZE;
	}

	if (p->hostUniq.type) {
		CHECK_ROOM(cursor, packet->payload, ntohs(p->hostUniq.length)+TAG_HDR_SIZE);
		memcpy(cursor, &p->hostUniq, ntohs(p->hostUniq.length) + TAG_HDR_SIZE);
		cursor += ntohs(p->hostUniq.length) + TAG_HDR_SIZE;
		plen += ntohs(p->hostUniq.length) + TAG_HDR_SIZE;
	}

	packet->length = htons(plen);
	len = (int) (plen + PPPOE_HDR_SIZE);

	ret = sendto(pppoe_server_socket, packet, len+Ehdr_SZ, 0, (struct sockaddr*)addr, sizeof(struct sockaddr_ll));
	if(ret < 0){
		printf("send PADS error! ret = %d\n", ret);
		perror("Could not send data: ");
	}
}
#endif

/***********************************************************************
*%FUNCTION: sendPADT
*%ARGUMENTS: None
*%RETURNS: 0 success, 1 fail
*%DESCRIPTION: Sends a PADT packet
***********************************************************************/
int sendPADT(PPPOE_DRV_CTRL *p)
{
//	unsigned char pkt_buf[256];
	unsigned char pkt_buf[RECV_LEN];
	struct PPPoEPacket *packet;
#ifdef PPPOE_EOL_TAG
	struct PPPoETag eol;	// EOL
#endif
	unsigned char *cursor;
	unsigned short plen = 0;
	int len;
	int debug = p->debug;
	char *rpfcn = "[sendPADT]";

	if (!p->session)
	{
//		addlog(PPPOE_FMT"%s null session id\n", PPPOE_ARGS(p->ifno));
		return 1;
	}

	packet = (struct PPPoEPacket *)pkt_buf;
	cursor = packet->payload;

	memcpy(&packet->ehdr.to, p->ACEthAddr, Eid_SZ);
	memcpy(&packet->ehdr.from, p->MyEthAddr, Eid_SZ);
//	bcopy((char*)p->ACEthAddr, (char*)&packet->ehdr.to, Eid_SZ);
//	bcopy((char*)p->MyEthAddr, (char*)&packet->ehdr.from, Eid_SZ);

	packet->ehdr.type = htons(ETH_PPPOE_DISCOVERY);
	packet->ver_type = 0x11;
	packet->code = CODE_PADT;
	packet->session = p->session;

	/* Reset Session to zero so there is no possibility of
       recursive calls to this function by any signal handler */
	p->session = 0;

	/* Copy cookie and relay-ID if needed */
	if (p->cookie.type) {
		CHECK_ROOM(cursor, packet->payload, ntohs(p->cookie.length) + TAG_HDR_SIZE);	// Jenny
		memcpy(cursor, &p->cookie, ntohs(p->cookie.length) + TAG_HDR_SIZE);
//		bcopy((char*)&p->cookie, (char*)cursor, ntohs(p->cookie.length) + TAG_HDR_SIZE);
		cursor += ntohs(p->cookie.length) + TAG_HDR_SIZE;
		plen += ntohs(p->cookie.length) + TAG_HDR_SIZE;
	}

	if (p->relayId.type) {
		CHECK_ROOM(cursor, packet->payload, ntohs(p->relayId.length) + TAG_HDR_SIZE);	// Jenny
		memcpy(cursor, &p->relayId, ntohs(p->relayId.length) + TAG_HDR_SIZE);
//		bcopy((char*)&p->relayId, (char*)cursor, ntohs(p->relayId.length) + TAG_HDR_SIZE);
		cursor += ntohs(p->relayId.length) + TAG_HDR_SIZE;
		plen += ntohs(p->relayId.length) + TAG_HDR_SIZE;
	}

#ifdef PPPOE_EOL_TAG	// EOL
    	// Jenny, support Tag End-Of-List
	eol.type = htons(TAG_END_OF_LIST);
	eol.length = htons(0);
	memcpy(cursor, &eol, TAG_HDR_SIZE);
	plen += TAG_HDR_SIZE;
#endif

	packet->length = htons(plen);
	len = (int) (plen + PPPOE_HDR_SIZE);
	PPPOE_LOWER_SEND(p, packet, len);
	p->session = PPPOE_STATE_IDLE;
	pppoe_up_log(p->ifno, 1);	// Jenny

	if (debug)
		addlog(PPPOE_FMT"O PADT\n", PPPOE_ARGS(p->ifno));
	return 0;
}

void pppoe_up(PPPOE_DRV_CTRL *p)
{
	int debug = p->debug;

	if(debug)
		addlog(PPPOE_FMT"Phase is Discovery\n", PPPOE_ARGS(p->ifno));
	p->timeout = PADI_TIMEOUT;
	sendPADI(p);
}

void pppoe_to(PPPOE_DRV_CTRL *p)
{
	p->timeout = PADI_TIMEOUT;
	sendPADR(p);
}

//int pppoe_send(struct ifnet *ifp);
//void pppoe_if_up(struct sppp *sp);
//#define PPP_PAP		0xc023		/* Password Authentication Protocol */
//#define PPP_CHAP	0xc223		/* Challenge-Handshake Auth Protocol */
//#include "if_sppp.h"
//extern struct sppp sppp_softc[];

int pppoe_open(PPPOE_DRV_CTRL *p)
{
//struct sppp *sp = &sppp_softc[p->ifno];
int debug = p->debug;
int retval;

	if(debug)
		addlog(PPPOE_FMT"Phase is Session\n", PPPOE_ARGS(p->ifno));

    p->sp.sa_family = AF_PPPOX;
    p->sp.sa_protocol = PX_PROTO_OE;
    p->sp.sa_addr.pppoe.sid = p->session;
    memcpy(p->sp.sa_addr.pppoe.dev,p->name, IFNAMSIZ);
    memcpy(p->sp.sa_addr.pppoe.remote, p->remote.sll_addr, ETH_ALEN);

    //printf("Connecting PPPoE socket: %04x %s %p\n",
	//     p->sp.sa_addr.pppoe.sid,
	//     p->sp.sa_addr.pppoe.dev,p);

    retval = connect(p->fd, (struct sockaddr*)&p->sp,
		  sizeof(struct sockaddr_pppox));


    if( retval < 0 ){
		printf("Failed to connect PPPoE socket: %d %m",retval);
		return retval;
    }

#if defined(CONFIG_LUNA)
	//printf pppoe information for diag cmd user: Chuck
	printf("############PPPoE INFO:###############\n");
	printf("###SessionID: %d\n",p->session);
	printf("###dev_name: %s\n",p->name);

	printf("###gateway Mac: %02x:%02x:%02x:%02x:%02x:%02x\n",p->remote.sll_addr[0],p->remote.sll_addr[1],p->remote.sll_addr[2],p->remote.sll_addr[3],p->remote.sll_addr[4],p->remote.sll_addr[5]);
	printf("###  message@ %s(%s:%d)\n",__FILE__,__func__,__LINE__);
	printf("######################################\n");
#endif

	return 0;
}

void pppoe_down(PPPOE_DRV_CTRL *p)
{
	int debug = p->debug;

	UNTIMEOUT(0, 0, p->ch);
	UNTIMEOUT(0, 0, p->disc_ch);
	if(debug)
		addlog(PPPOE_FMT"State is Terminated\n", PPPOE_ARGS(p->ifno));
	disc_counter[p->ifno] = 0;
	pppoe_init(p);
//	pppoe_init(p->ifno, "");
	//sppp_init(p->ifno);
	pppoe_up(p);
}


extern struct sppp *spppq;
static struct sppp *find_sp(PPPOE_DRV_CTRL *p)
{
	struct sppp *sp;
	sp = spppq;
	while(sp) {
		if ((sp->over == SPPP_PPPOE) && sp->pp_lowerp == p)
			return sp;
		sp = sp->pp_next;
	}
	return sp;
}

void pppoe_cp_input(PPPOE_DRV_CTRL *p, struct PPPoEPacket *packet)
{

	struct PacketCriteria pc;
	int debug = p->debug;
	char *rpfcn = "[pppoe_cp_input]";
	char mac[17];
	MIB_CE_ATM_VC_T entry;
	unsigned int totalEntry; /* get chain record size */
	int i;

//	memset(&pc, 0, sizeof pc);
	memset(&pc, 0, sizeof(struct PacketCriteria));

	mac[0] = 0;
	_pppoe_mac(mac, (char*)&packet->ehdr.from);
	switch (packet->code)
	{
		case CODE_PADO:
			if (debug)
			{
				/*addlog(PPPOE_FMT"I PADO [%s] from ", PPPOE_ARGS(p->ifno), _pppoe_state_name(p->DiscoveryState));
				_pppoe_print_mac((char*)&packet->ehdr.from);
				addlog("\n");*/
				addlog(PPPOE_FMT"I PADO [%s] from %s\n", PPPOE_ARGS(p->ifno), _pppoe_state_name(p->DiscoveryState), mac);
			}
			// Jenny, check if PADR has been sent
			if (!sendPADR_flag[p->ifno]) {
				parsePADOTags(p, packet, &pc);
				if (pc.acNameOK && pc.serviceNameOK) {
					UNTIMEOUT(sendPADI, p, p->ch);
					pppoe_up_log(p->ifno, 0);	// Jenny
					pppoe_change_state(p, PPPOE_STATE_RECEIVED_PADO);
					memcpy(p->ACEthAddr, &packet->ehdr.from, Eid_SZ);
					memcpy(p->remote.sll_addr, &packet->ehdr.from, Eid_SZ);
					pppoe_to(p);
				}
			}
			break;
		case CODE_PADS:
			if (debug)
			{
				/*addlog(PPPOE_FMT"I PADS [%s] from ", PPPOE_ARGS(p->ifno), _pppoe_state_name(p->DiscoveryState));
				_pppoe_print_mac((char*)&packet->ehdr.from);
				addlog("\n");*/
				addlog(PPPOE_FMT"I PADS [%s] from %s\n", PPPOE_ARGS(p->ifno), _pppoe_state_name(p->DiscoveryState), mac);
			}
			if (memcmp(&packet->ehdr.from, p->ACEthAddr, Eid_SZ))
			{
				if(debug)
					addlog(PPPOE_OPT"Invalid source addr, discard\n", PPPOE_ARGS(p->ifno));
			}
			else
			{
				if (parsePADSTags(p, packet))
				{
					UNTIMEOUT(sendPADR, p, p->ch);
					p->session = packet->session;
					//pppoe_session_update(p);
					pppoe_up_log(p->ifno, 0);	// Jenny
					pppoe_change_state(p, PPPOE_STATE_SESSION);
					if (debug)
					{
						addlog(PPPOE_OPT"Session ID = %04X\n", PPPOE_ARGS(p->ifno), (int) ntohs(p->session));
						if (ntohs(p->session) == 0)
							addlog(PPPOE_FMT"Access concentrator used a session value of zero\n", \
								PPPOE_ARGS(p->ifno));
					}
					// for some broadband ras bug
					// it should delay msecs here
					pppoe_open(p);
				}
				else
				{
					pppoe_up_log(p->ifno, 1);	// Jenny
					pppoe_change_state(p, PPPOE_STATE_TERMINATED);
					pppoe_down(p);
				}
			}
			break;
		case CODE_PADT:
			if (debug)
			{
				/*addlog(PPPOE_FMT"I PADT [%s] sid = %04X from ", PPPOE_ARGS(p->ifno), \
					_pppoe_state_name(p->DiscoveryState), packet->session);
					_pppoe_print_mac((char*)&packet->ehdr.from);
				addlog("\n");*/
				addlog(PPPOE_FMT"I PADT [%s] sid = %04X from %s\n", PPPOE_ARGS(p->ifno), \
					_pppoe_state_name(p->DiscoveryState), packet->session, mac);
			}
			if (memcmp(&packet->ehdr.from, p->ACEthAddr, Eid_SZ))
			{
				if (debug)
					addlog(PPPOE_OPT"Invalid source addr, discard\n", PPPOE_ARGS(p->ifno));
			}
			else
			{
				if (packet->session != p->session)
				{
					if (debug)
						addlog(PPPOE_OPT"Error session ID, discard\n", PPPOE_ARGS(p->ifno));
				}
				else
				{
					// Receive PADT, force uplayer ppp down
					struct sppp *sp=0;
					sp = find_sp(p);
					if (sp) {
						sp->pp_down(sp);
					}
					parsePADTTags(p, packet);
					//pppoe_up_log(p->ifno, 1);	// Jenny
					pppoe_change_state(p, PPPOE_STATE_TERMINATED);
					pppoe_down(p);
					pppoe_up_log(p->ifno, 1);	// Jenny
				}
			}
			break;
	}
}

// 1 Invalid packet, 0 valid packet
int ErrorPacket(struct PPPoEPacket *packet, PPPOE_DRV_CTRL *p)
{
	int debug = p->debug;
	char *rpfcn = "[ErrorPacket]";

	if (packet->ver_type != 0x11)
	{
		if (debug)
			addlog(PPPOE_FMT"%s error ver and type =%02x\n", PPPOE_ARGS(p->ifno), packet->ver_type);
		return 1;
	}

	if (packet->length > ET_MAXLEN - 6) {
		if(debug)
			addlog(PPPOE_FMT"%s invalid PPPoE packet length %u\n", PPPOE_ARGS(p->ifno), packet->length);
		return 1;
	}

	if (memcmp(&packet->ehdr.to, p->MyEthAddr, Eid_SZ)) {
		if(debug)
			addlog(PPPOE_FMT"%s invalid dest addr %02x:%02x:%02x:%02x:%02x:%02x<->%02x:%02x:%02x:%02x:%02x:%02x\n\n", PPPOE_ARGS(p->ifno),
				   (unsigned) packet->ehdr.to.octet[0], (unsigned) packet->ehdr.to.octet[1], (unsigned) packet->ehdr.to.octet[2],
				   (unsigned) packet->ehdr.to.octet[3], (unsigned) packet->ehdr.to.octet[4], (unsigned) packet->ehdr.to.octet[5],
				   (unsigned) p->MyEthAddr[0], (unsigned) p->MyEthAddr[1], (unsigned) p->MyEthAddr[2],
				   (unsigned) p->MyEthAddr[3], (unsigned) p->MyEthAddr[4], (unsigned) p->MyEthAddr[5]);
		return 1;
	}

	return 0;
}


int pppoe_init(PPPOE_DRV_CTRL *pppoe_p)
{
	char *rpfcn = "[pppoe_init]";
//	int debug;

//	pppoe_p->debug = 1;
	pppoe_change_state(pppoe_p, PPPOE_STATE_IDLE);
	pppoe_p->optUseHostUnique = 0;
	pppoe_p->session = 0;
	strcpy(pppoe_p->ServiceName, "");
	strcpy(pppoe_p->DesiredACName, "");
	strcpy(pppoe_p->ACName, "");
	memset(pppoe_p->ACEthAddr, 0, Eid_SZ);
	pppoe_p->cookie.type = 0;               // Added by Jenny
	pppoe_p->cookie.length= 0;
	memset(pppoe_p->cookie.payload, 0, ET_MAXLEN);
	pppoe_p->relayId.type = 0;
	pppoe_p->relayId.length= 0;
	memset(pppoe_p->relayId.payload, 0, ET_MAXLEN);
	sendPADR_flag[pppoe_p->ifno] = 0;
	//pppoe_p->fd = -1;
	//pppoe_p->disc_sock = -1;
	//disc_counter[pppoe_p->ifno] = 0;

#ifdef CONFIG_USER_PPPOE_2_INTERVALS_RETRY
	pppoe_p->retry_delay = 30;
	pppoe_p->retry_delay_long = 300;
	pppoe_p->retry_threshold = 10;
#endif

	return 1;
}

int pppoe_shut(PPPOE_DRV_CTRL *pppoe_p)
{
	char *rpfcn = "[pppoe_shut]";

	sendPADT(pppoe_p);
	pppoe_change_state(pppoe_p, PPPOE_STATE_TERMINATED);
	return 1;
}

#if 0
PPPOE_DRV_CTRL* _pppoe_get_by_session(unsigned short session)
{
	int i;
	for(i=0; i<N_PPPOE; i++)
		if(pppoe_pool[i].used)
			if(pppoe_pool[i].session == session)
				return(&pppoe_pool[i]);
	/* not found */
	return NULL;
}

PPPOE_DRV_CTRL* _pppoe_get_by_discovery(void)
{
	int i;
	for(i=0; i<N_PPPOE; i++)
		if(pppoe_pool[i].used)
			if(pppoe_pool[i].DiscoveryState & STATE_DISCOVERY)
				return(&pppoe_pool[i]);
	/* not found */
	return NULL;
}
#endif

/*static void _pppoe_print_mac(const unsigned char *mac)
{
	addlog("%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}*/

static void _pppoe_mac(const char *result, const unsigned char *mac)
{
	sprintf(result, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

const char *
_pppoe_state_name(int state)
{
	switch (state) {
		case PPPOE_STATE_IDLE:				return "Idle";
		case PPPOE_STATE_SENT_PADI:			return "PADI-Sent";
		case PPPOE_STATE_RECEIVED_PADO:	return "PADO-Received";
		case PPPOE_STATE_SENT_PADR:		return "PADR-Sent";
		case PPPOE_STATE_SESSION:			return "Session";
		case PPPOE_STATE_TERMINATED:		return "Terminated";
	}
	return "illegal";
}


//extern fd_set	in_fds;
//extern int	max_in_fd;
// Kaohj
fd_set poe_in_fds;
int max_poe_in_fd;

int pppoe_recv(PPPOE_DRV_CTRL *pppoe_p)
{
	// Kaohj
	//unsigned char pkt_buf[256];
	unsigned char pkt_buf[RECV_LEN];
	struct Ehdr *ehdr;
	struct PPPoEPacket *packet;
	int	len;
	char *rpfcn = "[pppoe_recv]";


	packet = (struct PPPoEPacket *)pkt_buf;
	ehdr = &(packet->ehdr);

	{
		struct sockaddr_ll	from_addr;
		fd_set in;
		struct timeval tv;
		int ret;

		//in = in_fds;
		in = poe_in_fds;

		tv.tv_sec = 0;
		tv.tv_usec = 100000;

		//ret = select(max_in_fd+1, &in, NULL, NULL, &tv);
		ret = select(max_poe_in_fd+1, &in, NULL, NULL, &tv);

		if (ret<=0)
			return 0;

		len = sizeof(struct sockaddr_ll);

		if (!FD_ISSET( pppoe_p->disc_sock, &poe_in_fds))	// Jenny
			return 0;
		// Kaohj
		//ret = recvfrom( pppoe_p->disc_sock, pkt_buf+Ehdr_SZ, 256, 0,
		//	(struct sockaddr*)&from_addr, &len);
		ret = recvfrom( pppoe_p->disc_sock, pkt_buf+Ehdr_SZ, RECV_LEN-Ehdr_SZ, 0,
			(struct sockaddr*)&from_addr, &len);
		if (ret<0)		// Jenny
			return 0;

		memcpy(&packet->ehdr.from, from_addr.sll_addr, ETH_ALEN);
		ehdr->type = from_addr.sll_protocol;

	}



	//if (ErrorPacket(packet, pppoe_p))  goto RelBuf;

	switch (ehdr->type)
	{
		case ETH_PPPOE_DISCOVERY:
			// Casey
			//pppoe_p = _pppoe_get_by_discovery();
			//if(!pppoe_p && packet->code == CODE_PADT)
			//	pppoe_p = _pppoe_get_by_session(packet->session);
			//if(pppoe_p)
			//
			pppoe_cp_input(pppoe_p, packet);
			/* release buffer */
			goto pppoeReceiveError;

		case ETH_PPPOE_SESSION:
			if(packet->code != CODE_SESS)
				goto pppoeReceiveError;

			/* demux pppoe connection by session number */
			// Casey
			//pppoe_p = _pppoe_get_by_session(packet->session);
			//
			if(!pppoe_p)
				goto pppoeReceiveError;
			if(pppoe_p->DiscoveryState != PPPOE_STATE_SESSION)
				goto pppoeReceiveError;

			if (bcmp((char*)&packet->ehdr.from, (char*)pppoe_p->ACEthAddr, Eid_SZ)) {
				/* debug */
				if(pppoe_p->debug)
					addlog(PPPOE_FMT"Session packet, invalid source addr\n", PPPOE_ARGS(pppoe_p->ifno));
				goto pppoeReceiveError;
			}

			/* point to the ppp header, and adjust the length */
			len	-= (Ehdr_SZ + PPPOE_OVERHEAD);
			//PPPOE_UPPER_INPUT(packet->payload, len);
			printf("pass session packet to upper %x %d\n",packet->payload, len);
			break;

		default:
			addlog(PPPOE_FMT"Error ether type = 0x%04X\n", PPPOE_ARGS(pppoe_p->ifno), ehdr->type);
			goto pppoeReceiveError;
    }
    return (TRUE);

pppoeReceiveError:

    return (TRUE);

}



/**********************************************************************
* FUNCTION: pppoeInit
* ARGUMENTS:
* 	unit 		-- pppoe unit number
* 	pDevice		-- attach to ethernet device
* 	dunit 		-- attach to ethernet device number
* 	local_addr 	-- local ip address, 0 as dynamic
* 	remote_addr -- remote ip address, 0 as dynamic
* 	username	-- username for authentication
* 	secret      -- password for authentication
* RETURNS: STATUS
* DESCRIPTION: Initial PPPoE unit and do dial up
***********************************************************************/
int pppoeInit(char *dev_name)
{
}

int pppoe_client_init(struct sppp *sppp)
{
	struct pppoe_param_s *poer = sppp->dev;
	PPPOE_DRV_CTRL *p;
	int retval, flags;
	char dev[IFNAMSIZ+1];
	int addr[ETH_ALEN];
	int sid;
	struct ifreq ifr;
	int i=0;
	int sockbufpriority = TC_PRIO_CONTROL;


	p = (PPPOE_DRV_CTRL *)malloc(sizeof(PPPOE_DRV_CTRL));
	if (!p)
		return -1;
	memset (p, 0, sizeof(PPPOE_DRV_CTRL));

	p->fd = -1;
	p->disc_sock = -1;
	p->ifno = sppp->if_unit;
	pppoe_init(p);

	strcpy(p->name, poer->dev_name);
	strcpy(p->DesiredACName, poer->ACName);
	strcpy(p->ServiceName, poer->ServiceName);	// Jenny
	p->vpi = poer->vpi;
	p->vci = poer->vci;
#if defined(CONFIG_LUNA)
	p->NoHardware_NAT = poer->NoHardware_NAT;
#endif

#ifdef CONFIG_USER_PPPOE_2_INTERVALS_RETRY
	if(poer->retry_threshold)
		p->retry_threshold = poer->retry_threshold;
	if(poer->retry_delay)
		p->retry_delay = poer->retry_delay;
	if(poer->retry_delay_long)
		p->retry_delay_long = poer->retry_delay_long;
#endif
	sppp->pp_lowerp = p;

	p->disc_sock = socket(PF_PACKET, SOCK_DGRAM, 0);
	if (p->disc_sock < 0) {
		printf("Cannot create PF_PACKET socket for PPPoE discovery\n");
		return -1;
	}

	if (setsockopt(p->disc_sock, SOL_SOCKET, SO_PRIORITY, (void *) &sockbufpriority, sizeof(sockbufpriority)) == -1)	// Jenny
		printf("set PPPoE (SO_PRIORITY) error! ");

	fcntl(p->disc_sock, F_SETFD, fcntl(p->disc_sock, F_GETFD) | FD_CLOEXEC); // FD_CLOEXEC: Let child process won't inherit this fd
	flags = fcntl(p->disc_sock, F_GETFL);	// Jenny
	if (flags == -1 || fcntl(p->disc_sock, F_SETFL, flags | O_NONBLOCK) == -1)
	   	printf("Couldn't set PPPoE discovery socket to nonblock\n");

	{
		strncpy(ifr.ifr_name, p->name, sizeof(ifr.ifr_name));

		retval = ioctl(p->disc_sock , SIOCGIFINDEX, &ifr);
		if (retval < 0) {
			printf("Bad device name: %s  (%m)",p->name);
			return retval;
//			return 0;
	    	}

		p->local.sll_ifindex = ifr.ifr_ifindex;

		retval = ioctl (p->disc_sock, SIOCGIFHWADDR, &ifr);
		if (retval < 0) {
			printf("Bad device name: %s  (%m)",p->name);
			return retval;
//			return 0;
    		}

		if (ifr.ifr_hwaddr.sa_family != ARPHRD_ETHER) {
			printf("Interface %s is not Ethernet!", p->name);
			return -1;
//			return 0;
		}

		p->local.sll_family	= AF_PACKET;
		p->local.sll_protocol= ntohs(ETH_P_PPP_DISC);
		p->local.sll_hatype	= ARPHRD_ETHER;
		p->local.sll_pkttype = PACKET_BROADCAST;
		p->local.sll_hatype	= ETH_ALEN;
		memcpy(p->local.sll_addr, ifr.ifr_hwaddr.sa_data, ETH_ALEN);

		memcpy(&p->remote, &p->local, sizeof(struct sockaddr_ll));
		memset(p->remote.sll_addr, 0xff, ETH_ALEN);
	}
	memcpy(p->MyEthAddr, p->local.sll_addr, ETH_ALEN);

	retval = bind( p->disc_sock , (struct sockaddr*)&p->local, sizeof(struct sockaddr_ll));
	if (retval < 0) {
		printf("bind to PF_PACKET socket failed: %m");
		return retval;		// Jenny
	}

	p->fd = socket(AF_PPPOX,SOCK_STREAM,PX_PROTO_OE);
	if (p->fd < 0)	{
		printf("Failed to create PPPoE socket: %m");
		return p->fd;	// Jenny
	}

	fcntl(p->fd, F_SETFD, fcntl(p->fd, F_GETFD) | FD_CLOEXEC); // FD_CLOEXEC: Let child process won't inherit this fd
	p->sp.sa_family = AF_PPPOX;
	p->sp.sa_protocol = PX_PROTO_OE;
	p->sp.sa_addr.pppoe.sid = 0;
	memcpy(p->sp.sa_addr.pppoe.dev,p->name, IFNAMSIZ);

/*	retval = connect(p->fd, (struct sockaddr*)&p->sp, sizeof(struct sockaddr_pppox));
	if (retval < 0) {
		printf("Failed to connect PPPoE socket: %d %m",retval);
		return retval;
	}*/

	flags = fcntl(p->fd, F_GETFL);	// Jenny
	if (flags == -1 || fcntl(p->fd, F_SETFL, flags | O_NONBLOCK) == -1)
	   	printf("Couldn't set PPPoE socket to nonblock\n");

	disc_counter[sppp->if_unit] = 0;
	return p->fd;
}

// Kaohj
//static struct callout disc_ch;
//static int disc_timeout;
void discTimeout(PPPOE_DRV_CTRL *p)
{
//	disc_timeout = 1;
	//printf("pppoe%d discTimeout\n", p->ifno);
	p->disc_timeout = 1;
}

/*
 * pppoe_add_fd - add an fd to the set that wait_input waits for.
 */
void pppoe_add_fd(int fd)
{
	FD_SET(fd, &poe_in_fds);
	if (fd > max_poe_in_fd)
		max_poe_in_fd = fd;
}

/*
 * pppoe_remove_fd - remove an fd from the set that wait_input waits for.
 */
void pppoe_remove_fd(int fd)
{
	FD_CLR(fd, &poe_in_fds);
}

//extern int in_pppoe_disc;
// Jenny, change PPPoE discovery mechanism from sequential to simultaneous, to shorten multiple PPPoE connecting time

#ifdef CONFIG_USER_PPPOE_PROXY
int createPppoeServerSocket()
{
	int opt = 1;
	int optval=1;
	int flags;
	struct ifreq ifr;
	struct sockaddr_ll address;

	/* create the server socket and check it worked */
	if ((pppoe_server_socket = socket(PF_PACKET, SOCK_RAW, ETH_PPPOE_DISCOVERY)) < 0)
		return -1;

	if (setsockopt(pppoe_server_socket, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval)) < 0) {
		return -2;
    }

	fcntl(pppoe_server_socket, F_SETFD, fcntl(pppoe_server_socket, F_GETFD) | FD_CLOEXEC); // FD_CLOEXEC: Let child process won't inherit this fd
	flags = fcntl(pppoe_server_socket, F_GETFL);
	if (flags == -1 || fcntl(pppoe_server_socket, F_SETFL, flags | O_NONBLOCK) == -1)
	   	printf("Couldn't set pppoe server socket to nonblock\n");

	/* set up socket */
	memset(&address, 0, sizeof(address));

	address.sll_family	= AF_PACKET;
	address.sll_protocol= ntohs(ETH_P_PPP_DISC);
	address.sll_hatype	= ARPHRD_ETHER;
	address.sll_pkttype = PACKET_BROADCAST;
	address.sll_hatype	= ETH_ALEN;
	memcpy(address.sll_addr, localhwAddr, ETH_ALEN);

	/* bind the socket to the pptp port */
	if (bind(pppoe_server_socket, (struct sockaddr *) &address, sizeof(address)) < 0)
		return -3;

	FD_SET(pppoe_server_socket, &pppConnSet);
	maxpppConnFd = pppoe_server_socket;

	return 0;
}

void pppoe_server_init()
{
	int i;
	FILE *fp;

	if(-1 != pppoe_server_socket)
		return;

	//get server's ip and hwaddr
	read_interface(IFName, &LocalIP, localhwAddr);

	createPppoeServerSocket();

	//init CookieSeed
	fp = fopen("/dev/urandom", "r");
    if (fp) {
		fread(&CookieSeed, 1, SEED_LEN, fp);
		fclose(fp);
    } else {
		CookieSeed[0] = getpid() & 0xFF;
		CookieSeed[1] = (getpid() >> 8) & 0xFF;
		for (i=2; i<SEED_LEN; i++) {
	  		CookieSeed[i] = (rand() >> (i % 9)) & 0xFF;
		}
    }

	//init Session
	Sessions = calloc(DEFAULT_MAX_SESSIONS, sizeof(struct ClientSession));
	for(i = 0; i < DEFAULT_MAX_SESSIONS; i++){
		Sessions[i].sess = i+1;
		Sessions[i].isUsingNow = 0;
		memcpy(Sessions[i].ip, ClientIP, sizeof(ClientIP));

		/* Increment IP */
		ClientIP[3]++;
		if (!ClientIP[3] || ClientIP[3] == 255) {
		    ClientIP[3] = 1;
		    ClientIP[2]++;
		    if (!ClientIP[2] || ClientIP[2] == 255) {
				ClientIP[2] = 1;
				ClientIP[1]++;
				if (!ClientIP[1] || ClientIP[1] == 255) {
					ClientIP[1] = 1;
				    ClientIP[0]++;
				}
		    }
		}
	}
}

struct ClientSession * getSession(){
	int i;
	static int usedSession=0;

	for(i = usedSession; i < DEFAULT_MAX_SESSIONS; i++){
		usedSession++;
		if(usedSession>=DEFAULT_MAX_SESSIONS)
			usedSession=0;
		if (Sessions[i].isUsingNow != 1){
			printf("\nPrepared session id :%d, with IP 0x%08x.\n", i, *((int *)(&Sessions[i].ip)));
			return &Sessions[i];
		}
		else
			printf("\nThe IP 0x%08x has been used\n", *((int *)(&Sessions[i].ip)));
	}
	return NULL;
}

void freeSession(int sessionID){
	int i;

	for(i = 0; i < DEFAULT_MAX_SESSIONS; i++){
		if (Sessions[i].sess == sessionID)
		{
			Sessions[i].isUsingNow=0;
			printf("Free PPPoE session id :%d, with IP 0x%08x\n",sessionID,*((int *)(&Sessions[i].ip)));
		}
	}
}

void freeAllSession(int sessionID){
	int i;

	printf("Free All PPPoE proxy Session!\n");
	for(i = 0; i < DEFAULT_MAX_SESSIONS; i++)
			Sessions[i].isUsingNow=0;
}

void pppoe_server_rcv()
{
	struct sppp *sp, *sq, *spq;
	PPPOE_DRV_CTRL *pppoe_p;
	PPPOEPROXY_DRV_CTRL *proxy_p, proxy_req;
	fd_set in;
	struct timeval tv;
	unsigned char pkt_buf[RECV_LEN];
	struct Ehdr *ehdr;
	struct PPPoEPacket *packet;
	struct sockaddr_ll from_addr;
	unsigned char cookieBuffer[16];
	struct ClientSession *cliSession;
	struct ifreq ifreq;
	int	len;
	int ret;
	int i;
	int match = 0;
	int flags;
	int sock_fd;
	struct SW_INFO swinfo;

	in = pppConnSet;
	tv.tv_sec = 0;
	tv.tv_usec = 10000;

	if (select(maxpppConnFd + 1, &in, NULL, NULL, &tv) <= 0) {
		return;
	}

	if (FD_ISSET(pppoe_server_socket, &in)) {	/* A call came! */

		packet = (struct PPPoEPacket *)pkt_buf;
		ehdr = &(packet->ehdr);
		len = sizeof(struct sockaddr_ll);
		ret = recvfrom(pppoe_server_socket, pkt_buf, RECV_LEN, 0,
			(struct sockaddr*)&from_addr, &len);
		if (ret<0){		// Jenny
			perror("recvfrom error: ");
			return;
		}

		if (0x11 != packet->ver_type){
			printf("ver_type error! ver_type = 0x%x\n", packet->ver_type);
			return;
		}

		if(ETH_PPPOE_DISCOVERY != ehdr->type){
			printf("packet type should be discovery!, type = 0x%x\n", ehdr->type);
			return;
		}

		switch(packet->code) {
			case CODE_PADI:
				parsePADITags(&proxy_req, packet);
				sp = spppq;
				while(sp){
					if((1 == sp->enable_pppoe_proxy) && (sp->currUser < sp->maxUser)){
						sendPADO(&proxy_req, &from_addr);
						break;
					}
					sp = sp->pp_next;
				}
				break;
			case CODE_PADR:
				parsePADRTags(&proxy_req, packet);
				if(!proxy_req.cookie.type){
					printf("Received PADR packet without cookie tag\n");
					sendErrorPADS(&proxy_req, &from_addr, TAG_GENERIC_ERROR, "No cookie.  Me hungry!");
					return;
				}
				if (proxy_req.cookie.length != htons(16)) {
					printf("Received PADR packet with invalid cookie tag length\n");
 					sendErrorPADS(&proxy_req, &from_addr, TAG_GENERIC_ERROR, "Cookie wrong size.");
					return;
			    }

				genCookie(from_addr.sll_addr, localhwAddr, CookieSeed, cookieBuffer);
			    if (memcmp(proxy_req.cookie.payload, cookieBuffer, 16)){
					printf("Received PADR packet with invalid cookie tag\n");
					sendErrorPADS(&proxy_req, &from_addr, TAG_GENERIC_ERROR, "Bad cookie.  Me have tummy-ache.");
					return;
			    }

				sp = spppq;
				while(sp){
					if((1 == sp->role)){
						proxy_p = sp->pp_lowerp;
						if(!memcmp(proxy_p->peerEthAddr, from_addr.sll_addr, Eid_SZ))
							return; //already response PADS and start pppd
					}
					sp = sp->pp_next;
				}

#if !defined(CONFIG_LUNA) && !defined(CONFIG_RTK_RG_INIT)  //LUNA using RG to do port mapping
#ifdef NEW_PORTMAPPING
				sock_fd = socket(AF_INET, SOCK_DGRAM, 0);  //sock_fd for query swport
			    if (sock_fd < 0)
					printf("Couldn't create IP socket!\n");

				fcntl(sock_fd, F_SETFD, fcntl(sock_fd, F_GETFD) | FD_CLOEXEC); // FD_CLOEXEC: Let child process won't inherit this fd
				memcpy(ifreq.ifr_name, IFName, IFNAMSIZ);
				memcpy(swinfo.hwaddr, from_addr.sll_addr, Eid_SZ);
				swinfo.swport = 255;
				ifreq.ifr_data = (void *)&swinfo;
				if(ioctl(sock_fd, SIOCGSWPORT, &ifreq) < 0){
					printf("ioctl SIOCGSWPORT error!\n");
					return;
				}
				close(sock_fd);
#endif
#endif
				sp = spppq;
				while(sp){
					if((1 == sp->enable_pppoe_proxy) && (sp->currUser < sp->maxUser)){
						if((1 == get_ipcpstate(sp))
#if !defined(CONFIG_LUNA) && !defined(CONFIG_RTK_RG_INIT)  //LUNA using RG to do port mapping
#ifdef NEW_PORTMAPPING
						 && (sp->itfGroup == 0 || ((sp->itfGroup & (1<<swinfo.swport)) != 0))
#endif
#endif
						){
							match = 1;
							break;
						}
					}
					sp = sp->pp_next;
				}

				if(!match){
					printf("no server can response\n");
					return;
				}

				cliSession = getSession();
				if(NULL == cliSession){
					printf("Can not get session.\n");
					sendErrorPADS(&proxy_req, &from_addr, TAG_AC_SYSTEM_ERROR, "No session supplied");
					return;
			    }
				packet->session = cliSession->sess;

				sendPADS(&proxy_req, cliSession->sess, &from_addr);

				if((sq = add_pppd(SPPP_PPPOE)) != NULL){
					sq->ipcp.myipaddr = *((int *)(&ServerIP));
					sq->ipcp.hisipaddr = *((int *)(&cliSession->ip));
					sq->ipcp.opts = (1 << IPCP_OPT_ADDRESS);
					//sq->ipcp.primary_dns = LocalIP; //if PPPoE Proxy, remote end is 192.168.100.1, instead of 192.168.1.1
													  // and DNS relay may not work for 192.168.100.1, so just give the DNS
													  // IP addre got from ISP
					sq->ipcp.primary_dns = (sp->ipcp.primary_dns==0)?LocalIP:sp->ipcp.primary_dns;
//					sq->ipcp.second_dns = sp->ipcp.second_dns;
					sq->ipcp.dns_opts = IPCP_OPT_DNS_PRIMARY;
					sq->myauth.aumode = sp->myauth.aumode;
					sq->myauth.proto = sp->myauth.proto;
					if(0x0 == sq->myauth.proto)
						sq->myauth.proto = PPP_PAP;
					memcpy(sq->myauth.name, sp->myauth.name, AUTHNAMELEN);
					memcpy(sq->myauth.secret, sp->myauth.secret, AUTHNAMELEN);

					//init proxy_p
					proxy_p = (PPPOEPROXY_DRV_CTRL *)malloc(sizeof(PPPOEPROXY_DRV_CTRL));
					if (!proxy_p){
						printf("alloc proxy_p error!\n");
						return;
					}
					memset (proxy_p, 0, sizeof(PPPOEPROXY_DRV_CTRL));

					memcpy(proxy_p->peerEthAddr, from_addr.sll_addr, Eid_SZ);
					memcpy(&proxy_p->client, &from_addr, sizeof(struct sockaddr_ll));
					memcpy(&proxy_p->cookie, &proxy_req.cookie, sizeof(struct PPPoETag));
					memcpy(&proxy_p->relayId, &proxy_req.relayId, sizeof(struct PPPoETag));
					proxy_p->sp = sp;
					proxy_p->session = cliSession->sess;
					memcpy(proxy_p->name, IFName, IFNAMSIZ);
					sp->currUser++;
					sq->pp_lowerp = proxy_p;

					proxy_p->fd = socket(AF_PPPOX,SOCK_STREAM,PX_PROTO_OE);
					if (proxy_p->fd < 0)	{
						printf("Failed to create PPPOE server socket\n");
						return ;
					}

					fcntl(proxy_p->fd, F_SETFD, fcntl(proxy_p->fd, F_GETFD) | FD_CLOEXEC); // FD_CLOEXEC: Let child process won't inherit this fd
					proxy_p->localaddr.sa_family = AF_PPPOX;
				    proxy_p->localaddr.sa_protocol = PX_PROTO_OE;
				  	proxy_p->localaddr.sa_addr.pppoe.sid = proxy_p->session;
				    memcpy(proxy_p->localaddr.sa_addr.pppoe.dev, proxy_p->name, IFNAMSIZ);
				    memcpy(proxy_p->localaddr.sa_addr.pppoe.remote, localhwAddr, Eid_SZ);

					memcpy(&proxy_p->peeraddr, &proxy_p->localaddr, sizeof(struct sockaddr_pppox));
					memcpy(proxy_p->peeraddr.sa_addr.pppoe.remote, from_addr.sll_addr, Eid_SZ);

				    ret = connect(proxy_p->fd, (struct sockaddr*)&proxy_p->peeraddr, sizeof(struct sockaddr_pppox));
					if( ret < 0 ){
						printf("Failed to connect PPPoE socket, ret = %d",ret);
						perror("connect error: ");
						return ;
				    }

					flags = fcntl(proxy_p->fd, F_GETFL);
					if (flags == -1 || fcntl(proxy_p->fd, F_SETFL, flags | O_NONBLOCK) == -1)
						printf("Couldn't set PPPOE server socket to nonblock\n");

					sq->fd = proxy_p->fd;
					cliSession->isUsingNow = 1;
					start_pppd_real(sq);
				}

				break;
			default:
				break;
		}
	}
}
#endif

#ifdef CONFIG_USER_PPPOE_2_INTERVALS_RETRY
static int get_poe_timeout(PPPOE_DRV_CTRL *p)
{
	if(p == NULL)
		return 10;	//defualt value

	if(disc_counter[p->ifno] >= p->retry_threshold)
		return p->retry_delay_long;

	return p->retry_delay;
}
#endif


void process_poe()
{
	struct sppp *sp;
	PPPOE_DRV_CTRL *p;
	int InFlags;

#ifdef CONFIG_USER_PPPOE_PROXY
	if(-1 != pppoe_server_socket){
		pppoe_server_rcv();
	}
#endif


	sp = spppq;
	while(sp) {
		if ((sp->over != SPPP_PPPOE) || (sp->role == 1)) {
			sp = sp->pp_next;
			continue;
		}
		if (!(p = sp->pp_lowerp)) {
			sp = sp->pp_next;
			continue;
		}
		if (p->in_pppoe_disc) {
			if (p->DiscoveryState == PPPOE_STATE_SESSION) {
				p->in_pppoe_disc = 0;
				disc_counter[p->ifno] = 0;
				UNTIMEOUT(0, 0, p->disc_ch);
				//printf("pppoe on session: %s\n", sp->if_name);
				syslog(LOG_INFO, "spppd: %s: AC MAC address %02X:%02X:%02X:%02X:%02X:%02X\n"
					, sp->if_name, p->ACEthAddr[0], p->ACEthAddr[1], p->ACEthAddr[2], p->ACEthAddr[3], p->ACEthAddr[4], p->ACEthAddr[5]);
				syslog(LOG_INFO, "spppd: %s: PPPoE Session ID %d\n", sp->if_name, p->session);
				start_ppp_real(sp);
				return;
			}
			if (!FD_ISSET(p->disc_sock, &poe_in_fds)) {
			// pppoe been deleted
				p->in_pppoe_disc = 0;
				sp = sp->pp_next;
				continue;
			}
			if (p->disc_timeout) {
				pppoe_shut(p);
				pre_PADT_flag[p->ifno] = 0;
				sendPADR_flag[p->ifno] = 0;
				disc_counter[p->ifno] ++;	// Jenny, for counting discovery timeout
				//pre_release_pppoe();	// Jenny, send PADT to disconnect previous session after discovery timeout
				sppp_last_connection_error(sp->if_unit, ERROR_NO_ANSWER);	// Jenny, send PADI, no reponse from PPPoE server
				UNTIMEOUT(0, 0, p->ch);
				if (FD_ISSET(p->disc_sock, &poe_in_fds))
					pppoe_remove_fd(p->disc_sock);
				p->in_pppoe_disc = 0;
				//printf("disc timeout\n");
				return;
			}
			pppoe_recv(p);
		}
		else { // not in PPPoE Discovery stage
			InFlags = 1;
			getInFlags(p->name, &InFlags);
			if ((InFlags & IFF_RUNNING) && (p->DiscoveryState == PPPOE_STATE_IDLE || p->DiscoveryState == PPPOE_STATE_TERMINATED)) {
#ifdef CONFIG_NO_REDIAL
				if(sp->is_noredial)
				   	return ;
#endif
				if (pre_PADT_flag[sp->if_unit]) {
					int ifindex=0;
					if (!FD_ISSET(p->disc_sock, &poe_in_fds))
						pppoe_add_fd(p->disc_sock);
					p->in_pppoe_disc = 1;
					p->disc_timeout = 0;
#ifdef CONFIG_USER_PPPOE_2_INTERVALS_RETRY
					TIMEOUT(discTimeout, (void *)p, get_poe_timeout(p), p->disc_ch);
#elif defined(CONFIG_USER_PPPOE_CTC_DELAY_INT)
					TIMEOUT(discTimeout, (void *)p, get_poe_ctcto(sp,0), p->disc_ch);
#else
					TIMEOUT(discTimeout, (void *)p, 10, p->disc_ch);
#endif
					p->debug = sp->debug;
					p->ifno = sp->if_unit;

					if(strncmp(p->name,ALIASNAME_NAS,3)==0)
					{
						sscanf(p->name, "nas0_%d", &ifindex);
						p->uifno = TO_IFINDEX(MEDIA_ETH, sp->if_unit, ETH_INDEX(ifindex));
					}
					else if(strncmp(p->name,ALIASNAME_VC,2)==0)
					{
						sscanf(p->name, "vc%d", &ifindex);
						p->uifno = TO_IFINDEX(MEDIA_ATM, sp->if_unit, VC_INDEX(ifindex));
					}
					else if(strncmp(p->name,ALIASNAME_PTM,3)==0)
					{
						sscanf(p->name, "ptm%d", &ifindex);
						p->uifno = TO_IFINDEX(MEDIA_PTM, sp->if_unit, PTM_INDEX(ifindex));
					}
					else if(strncmp(p->name,"wlan", 4)==0)
					{
						//sscanf(p->name, "ptm%d", &ifindex);
						ifindex = 0;
						p->uifno = TO_IFINDEX(MEDIA_WLAN, sp->if_unit, ETH_INDEX(ifindex));
					}
					else
						printf("Error!, unknown device\n");

					pppoe_up(p);
					//printf("start disc: %s\n", sp->if_name);
					return;
				}
				else{
					pre_release_pppoe(p->name);
				}
			}
			else if ((InFlags & IFF_RUNNING) && (p->DiscoveryState == PPPOE_STATE_SESSION)) {
				// in case of receiving PADT in session
				pppoe_recv(p);
			}
		}
		sp = sp->pp_next;
	}
   	return;
}

#if 0
static struct sppp *cur_sp;
void process_poe()
{
	struct sppp *sp, *tmp_sp;
	PPPOE_DRV_CTRL *p;

	if (in_pppoe_disc) {
		// check if cur_sp valid
		for (sp=spppq; sp; sp=sp->pp_next) {
			if (cur_sp == sp)
				break;
		}
		if (!sp) {
			in_pppoe_disc = 0;
			return;
		}
		if (!cur_sp || !(p=cur_sp->pp_lowerp)) {
			in_pppoe_disc = 0;
			return;
		}
		// check if discovery on SESSION state
		if (p->DiscoveryState == PPPOE_STATE_SESSION) {
			in_pppoe_disc = 0;
			disc_counter[p->ifno] = 0;
			UNTIMEOUT(0, 0, disc_ch);
			//printf("pppoe on session: %s\n", sp->if_name);
			start_ppp_real(cur_sp);
			return;
		}
		// kaohj, check if pppoe has been deleted
		if (!FD_ISSET(p->disc_sock, &poe_in_fds)) {
			// pppoe been deleted
			in_pppoe_disc = 0;
			return;
		}
		// check if pppoe discovery timeouted
		if (disc_timeout) {
			struct sppp **q;

			pppoe_shut(p);
			pre_PADT_flag[p->ifno] = 0;
			disc_counter[p->ifno] ++;	// Jenny, for counting discovery timeout
			pre_release_pppoe();	// Jenny, send PADT to disconnect previous session after discovery timeout
			UNTIMEOUT(0, 0, p->ch);
			pppoe_remove_fd(p->disc_sock);
			in_pppoe_disc = 0;
			// move to the end
			/* Remove the entry from the keepalive list. */
			//sppp_detach(cur_sp);
			for (q = &spppq; (sp = *q); q = &sp->pp_next)
				if (sp == cur_sp) {
					*q = sp->pp_next;
					break;
				}
			/* attach to the end */
			tmp_sp=0;
			for (sp=spppq; sp; sp=sp->pp_next)
				tmp_sp = sp;
			if (tmp_sp)
				tmp_sp->pp_next = cur_sp;
			else
				spppq = cur_sp;
			cur_sp->pp_next = NULL;
			//printf("disc timeout\n");
			return;
		}
		pppoe_recv(p);
	}
	else {
		// find an idle or terminated one
		for (sp=spppq; sp; sp=sp->pp_next) {
			if (sp->over != SPPP_PPPOE)
				continue;
			p = sp->pp_lowerp;
			if (!p)
				continue;
			if ((p->DiscoveryState == PPPOE_STATE_IDLE || p->DiscoveryState == PPPOE_STATE_TERMINATED) && adsl_PADT_flag)
				if (pre_PADT_flag[sp->if_unit]) {
					p->debug = sp->debug;
					p->ifno = sp->if_unit;
					pppoe_up(p);
					pppoe_add_fd(p->disc_sock);
					in_pppoe_disc = 1;
					disc_timeout = 0;
					TIMEOUT(discTimeout, p, 20, disc_ch);
					cur_sp = sp;
					//printf("start disc: %s\n", sp->if_name);
					return;
				}
				else
					pre_release_pppoe();
#if 0
			if ((p->DiscoveryState == PPPOE_STATE_IDLE || p->DiscoveryState == PPPOE_STATE_TERMINATED) && pre_PADT_flag[sp->if_unit] && adsl_PADT_flag) {
				p->debug = sp->debug;
				p->ifno = sp->if_unit;
				pppoe_up(p);
				pppoe_add_fd(p->disc_sock);
				in_pppoe_disc = 1;
				disc_timeout = 0;
				TIMEOUT(discTimeout, p, 20, disc_ch);
				cur_sp = sp;
				//printf("start disc: %s\n", sp->if_name);
				return;
			}
#endif
		}
	}
}

int pppoe_client_connect(struct sppp *sppp)
{
PPPOE_DRV_CTRL *p = sppp->pp_lowerp;
int debug;
int retval;
char dev[IFNAMSIZ+1];
int addr[ETH_ALEN];
int sid;
struct ifreq ifr;
int i=0;
int if_index=0;

	if(!p)
		return -1;

	p->debug = sppp->debug;
	debug = p->debug;

	p->ifno = sppp->if_unit;
	if(strncmp(p->name,ALIASNAME_NAS,3)==0)
    {
		sscanf(p->name, "nas0_%d", &ifindex);
		p->uifno = TO_IFINDEX(MEDIA_ETH, sppp->if_unit, ETH_INDEX(ifindex));
    }
    else if(strncmp(p->name,ALIASNAME_VC,2)==0)
    {
		sscanf(p->name, "vc%d", &ifindex);
		p->uifno = TO_IFINDEX(MEDIA_ATM, sppp->if_unit, VC_INDEX(ifindex));
    }
    else if(strncmp(p->name,ALIASNAME_PTM,3)==0)
    {
		sscanf(p->name, "ptm%d", &ifindex);
		p->uifno = TO_IFINDEX(MEDIA_PTM, sppp->if_unit, PTM_INDEX(ifindex));
    }
	else
		printf("Error!, unknown device\n");

	pppoe_up(p);

	sid = p->disc_sock;
	add_fd(sid);

	while(p->DiscoveryState != PPPOE_STATE_SESSION) {
		recv_msg();
		calltimeout();
		// kaohj, check if pppoe has been deleted
		if (!FD_ISSET(sid, &in_fds)) {
			// pppoe deleted
			p = 0;
			break;
		}
		pppoe_recv(p);
	}

    /* Once the logging is fixed, print a message here indicating
       connection parameters */
	if (!p)
		return -1;
	if(p->DiscoveryState != PPPOE_STATE_SESSION)
		return -1;
	return p->fd;


}
#endif

/**********************************************************************
* FUNCTION: pppoeDelete
* ARGUMENTS:
* 	unit		-- pppoe unit number
* 	pDevice		-- attach to ethernet device
* 	dunit 		-- attach to ethernet device number
* RETURNS: STATUS
* DESCRIPTION: Delete and detach PPPoE unit
***********************************************************************/
int pppoeDelete(struct sppp *sp)
{
	PPPOE_DRV_CTRL *p = sp->pp_lowerp;
	int retval;

//	printf("pppoeDelete..\n");

	if (p != NULL) {
		/* send PADT will cause server terminate immediatly */
		sendPADT(p);

		UNTIMEOUT(0, 0, p->ch);
		UNTIMEOUT(0, 0, p->disc_ch);

		p->sp.sa_addr.pppoe.sid = 0;

	// Kaohj
    //retval = connect(p->fd, (struct sockaddr*)&p->sp,
	//	  sizeof(struct sockaddr_pppox));
		if(p->fd >=0){
		close(p->fd);
		p->fd = -1;
		}

	/* test direct mode */
		//if (FD_ISSET( p->disc_sock, &poe_in_fds) || p->disc_sock >= 0) {
		if (FD_ISSET( p->disc_sock, &poe_in_fds))
			pppoe_remove_fd(p->disc_sock);
		if (p->disc_sock >= 0) {
			close(p->disc_sock);
			p->disc_sock = -1;
		}
		free(p);
		sp->pp_lowerp = NULL;
	}
	//sp->pp_lowerp = NULL;
	//
	return (0);
}


int pppoe_output(PPPOE_DRV_CTRL *p, void *buf, int data_len)
{
	int error;
	error = sendto(p->disc_sock, buf+Ehdr_SZ, data_len, 0,
		   (struct sockaddr*) &p->remote,
		   sizeof(struct sockaddr_ll));
	return error;
}

// Jenny, get PPPoE session information
static void pppoe_session_get(PPPOE_DRV_CTRL *p)
{
	unsigned int totalEntry;
	MIB_CE_PPPOE_SESSION_T Entry;
	int i, found=0, tmpEntry=-1;

	p->session = 0;
	totalEntry = mib_chain_total(MIB_PPPOE_SESSION_TBL); /* get chain record size */
	for (i=0; i<totalEntry; i++) {
		if (!mib_chain_get(MIB_PPPOE_SESSION_TBL, i, (void *)&Entry)) {
			printf("Get chain record error!\n");
			return;
		}

		if (Entry.uifno == p->uifno ) {
   		    memcpy((unsigned char *)p->ACEthAddr, (unsigned char *)Entry.acMac, 6);
			p->session = Entry.sessionId;
			//printf("uifno = 0x%x, Got session id 0x%x\n",p->uifno,p->session);
			return;
		}
	}
}

// Jenny, send PADT packet to release existed PPPoE connection
void pre_release_pppoe(char* interface_name)
{
	PPPOE_DRV_CTRL *p;
	struct sppp *sp;
	unsigned char pkt_buf[256];
	struct PPPoEPacket *packet;
#ifdef PPPOE_EOL_TAG
	struct PPPoETag eol;	// EOL
	unsigned char *cursor;	// EOL
#endif
	unsigned short plen = 0;
	int len;
	unsigned int totalEntry;
	MIB_CE_PPPOE_SESSION_T Entry;
	int i,ifindex;
	unsigned int uifno;

	for (sp=spppq; sp; sp=sp->pp_next) {
		if (1 == sp->role)//VPN Server has nothing to do with wan interface link status
			continue;
		if (sp->over != SPPP_PPPOE)
			continue;
		if (pre_PADT_flag[sp->if_unit])
			continue;
		p = sp->pp_lowerp;
		if (!p)
			continue;
		if(strcmp(p->name, interface_name)) {
			//printf("%s %d\n", __func__, __LINE__);
			continue;
		}

		p->ifno = sp->if_unit;

		if(strncmp(p->name,ALIASNAME_NAS,3)==0)
		{
			sscanf(p->name, "nas0_%d", &ifindex);
			p->uifno = TO_IFINDEX(MEDIA_ETH, sp->if_unit, ETH_INDEX(ifindex));
		}
		else if(strncmp(p->name,ALIASNAME_VC,2)==0)
		{
			sscanf(p->name, "vc%d", &ifindex);
			p->uifno = TO_IFINDEX(MEDIA_ATM, sp->if_unit, VC_INDEX(ifindex));
		}
		else if(strncmp(p->name,ALIASNAME_PTM,3)==0)
		{
			sscanf(p->name, "ptm%d", &ifindex);
			p->uifno = TO_IFINDEX(MEDIA_PTM, sp->if_unit, PTM_INDEX(ifindex));
		}
		else if(strncmp(p->name,"wlan", 4)==0)
		{
			//sscanf(p->name, "wlan%d", &ifindex);
			ifindex = 0;
			p->uifno = TO_IFINDEX(MEDIA_WLAN, sp->if_unit, ETH_INDEX(ifindex));
		}
		else
			printf("Error!, unknown device\n");

		pppoe_session_get(p);

		packet = (struct PPPoEPacket *)pkt_buf;
#ifdef PPPOE_EOL_TAG
		cursor = packet->payload;	// EOL
#endif
		memcpy(&packet->ehdr.to, p->ACEthAddr, Eid_SZ);
		memcpy(&packet->ehdr.from, p->MyEthAddr, Eid_SZ);
		//Modify to send PADT by broadcast
		//memcpy(p->remote.sll_addr, &packet->ehdr.to, Eid_SZ);
		memset(p->remote.sll_addr, 0xff, Eid_SZ);

//		bcopy((char*)p->ACEthAddr, (char*)&packet->ehdr.to, Eid_SZ);
//		bcopy((char*)p->MyEthAddr, (char*)&packet->ehdr.from, Eid_SZ);
		packet->ehdr.type = htons(ETH_PPPOE_DISCOVERY);
		packet->ver_type = 0x11;
		packet->code = CODE_PADT;
		packet->session = p->session;
		p->session = 0;
#ifdef PPPOE_EOL_TAG	// EOL
		eol.type = htons(TAG_END_OF_LIST);
		eol.length = htons(0);
		memcpy(cursor, &eol, TAG_HDR_SIZE);
		plen += TAG_HDR_SIZE;
#endif

		packet->length = htons(plen);
		len = (int) (plen + PPPOE_HDR_SIZE);
		if (packet->session != 0)
		{
			//printf("Send PADT with session id 0x%x\n",packet->session);
			PPPOE_LOWER_SEND(p, packet, len);
		}
		pppoe_up_log(p->ifno, 1);	// Jenny

		p->session = PPPOE_STATE_IDLE;
		pre_PADT_flag[p->ifno] = 1;
		//sleep for 1 second. for inter-op with ISP, make sure previous session is closed.
		//sleep(1);
		if (sp->debug) {
			/*addlog(PPPOE_FMT"O PADT Session ID=%04X from ", PPPOE_ARGS(p->ifno), packet->session);
			_pppoe_print_mac((char*)&packet->ehdr.from);
			addlog(" to ");
			_pppoe_print_mac((char*)&packet->ehdr.to);
			addlog("\n");*/
			char mac1[17], mac2[17];
			mac1[0] = mac2[0] = 0;
			_pppoe_mac(mac1, (char*)&packet->ehdr.from);
			_pppoe_mac(mac2, (char*)&packet->ehdr.to);
			addlog(PPPOE_FMT"O PADT Session ID=%04X from %s to %s\n", PPPOE_ARGS(p->ifno), packet->session, mac1, mac2);
		}
	}
}

const char PPPOE_UPFLAG[] = "/tmp/ppp_up_log";
void pppoe_up_log(int ifno, int value)
{
	char buff[10]="";
	FILE *fp;
	struct sppp *sp;

	fp = fopen(PPPOE_UPFLAG, "w");

	sp = spppq;
	while(sp) {
		if (sp->over == SPPP_PPPOE) {	// sp->over, PPPoE:1, PPPoA: 0
			if(sp->if_unit ==  ifno) {
				sp->up_flag = value;
				sprintf(buff, "%d %d\n", sp->if_unit, value);
			}
			else
				sprintf(buff, "%d %d\n", sp->if_unit, sp->up_flag);
		}
		if(sp->over == SPPP_PPPOATM) {
			sp->up_flag = 1;
			sprintf(buff, "%d 1\n", sp->if_unit);
		}
#ifdef CONFIG_USER_PPPOMODEM
		if(sp->over == SPPP_PPPOMODEM) {
			sp->up_flag = 1;//????
			sprintf(buff, "%d 1\n", sp->if_unit);
		}
#endif //CONFIG_USER_PPPOMODEM
		fputs(buff, fp);
		sp = sp->pp_next;
	}
	fclose(fp);
}

