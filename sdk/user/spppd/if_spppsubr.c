/*
 * Synchronous PPP/Cisco link level subroutines.
 * Keepalive protocol implemented in both Cisco and PPP modes.
 *
 * Copyright (C) 1994-1996 Cronyx Engineering Ltd.
 * Author: Serge Vakulenko, <vak@cronyx.ru>
 *
 * Heavily revamped to conform to RFC 1661.
 * Copyright (C) 1997, 2001 Joerg Wunsch.
 *
 * This software is distributed with NO WARRANTIES, not even the implied
 * warranties for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Authors grant any other persons or organisations permission to use
 * or modify this software as long as this message is kept with the software,
 * all derivative works or modified versions.
 *
 * From: Version 2.4, Thu Apr 30 17:17:21 MSD 1997
 *
 * $FreeBSD: src/sys/net/if_spppsubr.c,v 1.59.2.13 2002/07/03 15:44:41 joerg Exp $
 */

#include "if_sppp.h"
#ifdef USE_LIBMD5
#include <libmd5wrapper.h>
#else
#include "md5.h"
#endif //USE_LIBMD5
#include "pppoe.h"
#include <rtk/utility.h>

// Added by Mason Yu for PPP LED
#include <signal.h>
#include <config/autoconf.h>
#include "chap_ms.h"
//#include "ccp.h"
//#include "mppe.h"
#ifdef CONFIG_USER_L2TPD_L2TPD
#include "l2tp_main.h"
#endif//endof CONFIG_USER_L2TPD_L2TPD

#ifdef CONFIG_PPP_MPPE_MPPC
#include "mppe.h"
#include "chap_ms.h"
#endif

#ifdef ENABLE_PPP_SYSLOG
#define addlog		dbg_printf
#else
#define addlog		printf
#endif
//#define addlog		printf

#define MHLEN	1500
int time_second=0;
int hz = 1;

// Jenny, for PPP uptime
#include <sys/sysinfo.h>

long starttime[N_SPPP] = {0};
long totaluptime[N_SPPP] = {0};

#define FAILMSG "Failed..."
#define SUCCMSG "Welcome!"


//struct sppp sppp_softc[8];


#define TIMEOUT(fun, arg1, arg2, handle) 	timeout(fun,arg1,arg2, &handle)
#define UNTIMEOUT(fun, arg, handle)			untimeout(&handle)

#define IOCTL_CMD_T							u_long

#define MAXALIVECNT     6               /* max. alive packets */
#define KEEPALIVE_INTERVAL	20	/* send LCP echo-request every 20 seconds */

void sppp_callout_handle_init(struct callout *handle)
{

}

/*
 * Interface flags that can be set in an ifconfig command.
 *
 * Setting link0 will make the link passive, i.e. it will be marked
 * as being administrative openable, but won't be opened to begin
 * with.  Incoming calls will be answered, or subsequent calls with
 * -link1 will cause the administrative open of the LCP layer.
 *
 * Setting link1 will cause the link to auto-dial only as packets
 * arrive to be sent.
 *
 * Setting IFF_DEBUG will syslog the option negotiation and state
 * transitions at level kern.debug.  Note: all logs consistently look
 * like
 *
 *   <if-name><unit>: <proto-name> <additional info...>
 *
 * with <if-name><unit> being something like "bppp0", and <proto-name>
 * being one of "lcp", "ipcp", "cisco", "chap", "pap", etc.
 */

#define IFF_PASSIVE	IFF_MASTER	/* wait passively for connection */
#define IFF_AUTO	IFF_SLAVE	/* auto-dial on output */

#define PPP_ALLSTATIONS 0xff		/* All-Stations broadcast address */
#define PPP_UI		0x03		/* Unnumbered Information */
#define PPP_IP		0x0021		/* Internet Protocol */
#define PPP_VJ_COMP	0x002d		/* VJ compressed TCP/IP */
#define PPP_VJ_UCOMP	0x002f		/* VJ uncompressed TCP/IP */
#define PPP_IPV6	0x0057		/* Internet Protocol Version 6 */
#define PPP_LCP		0xc021		/* Link Control Protocol */
#define PPP_PAP		0xc023		/* Password Authentication Protocol */
#define PPP_CHAP	0xc223		/* Challenge-Handshake Auth Protocol */
#define PPP_IPCP	0x8021		/* Internet Protocol Control Protocol */
#define PPP_IPV6CP	0x8057		/* IPv6 Control Protocol */
#define PPP_CCP		0x80fd		/* Compression Control Protocol */
#define	PPP_COMP	0x00fd	/* Compressed Datagram */

#define CONF_REQ	1		/* PPP configure request */
#define CONF_ACK	2		/* PPP configure acknowledge */
#define CONF_NAK	3		/* PPP configure negative ack */
#define CONF_REJ	4		/* PPP configure reject */
#define TERM_REQ	5		/* PPP terminate request */
#define TERM_ACK	6		/* PPP terminate acknowledge */
#define CODE_REJ	7		/* PPP code reject */
#define PROTO_REJ	8		/* PPP protocol reject */
#define ECHO_REQ	9		/* PPP echo request */
#define ECHO_REPLY	10		/* PPP echo reply */
#define DISC_REQ	11		/* PPP discard request */

#define LCP_OPT_MRU		1	/* maximum receive unit */
#define LCP_OPT_ASYNC_MAP	2	/* async control character map */
#define LCP_OPT_AUTH_PROTO	3	/* authentication protocol */
#define LCP_OPT_QUAL_PROTO	4	/* quality protocol */
#define LCP_OPT_MAGIC		5	/* magic number */
#define LCP_OPT_RESERVED	6	/* reserved */
#define LCP_OPT_PROTO_COMP	7	/* protocol field compression */
#define LCP_OPT_ADDR_COMP	8	/* address/control field compression */

#define IPCP_OPT_ADDRESSES		0x01	/* both IP addresses; deprecated */
#define IPCP_OPT_COMPRESSION	0x02	/* IP compression protocol (VJ) */
#define IPCP_OPT_ADDRESS		0x03	/* local IP address */
#define IPCP_OPT_DNS_PRIMARY	0x81	/* primary DNS address */
#define IPCP_OPT_DNS_SECOND		0x83	/* second DNS address */

#ifdef CONFIG_PPP_MPPE_MPPC

#define CCP_Microsoft_PPC		0x12	/* Microsoft PPC */
#define CI_MPPE                18      /* config option for MPPE */
#define CILEN_MPPE		6	/* length of config. option */
#define MPPE_STATELESS          0x01   /* configuration bit H */
#define MPPE_40BIT              0x20   /* configuration bit L */
#define MPPE_56BIT              0x80   /* configuration bit M */
#define MPPE_128BIT             0x40   /* configuration bit S */
#define MPPC                    0x01    /* configuration bit C */
#define MPPE_MPPC               (MPPC|MPPE_128BIT|MPPE_56BIT|MPPE_40BIT)
//#define MPPE_MPPC               (MPPC|MPPE_40BIT)
//#define MPPE_MPPC               MPPC
//#define MPPE_MPPC               (MPPC|MPPE_128BIT)

#define BCOPY(s, d, l)		memcpy(d, s, l)

#endif


#define IPV6CP_OPT_IFID	1	/* interface identifier */
#define IPV6CP_OPT_COMPRESSION	2	/* IPv6 compression protocol */

#define IPCP_COMP_VJ		0x2d	/* Code for VJ compression */

#define PAP_REQ			1	/* PAP name/password request */
#define PAP_ACK			2	/* PAP acknowledge */
#define PAP_NAK			3	/* PAP fail */

#define CHAP_CHALLENGE		1	/* CHAP challenge request */
#define CHAP_RESPONSE		2	/* CHAP challenge response */
#define CHAP_SUCCESS		3	/* CHAP response ok */
#define CHAP_FAILURE		4	/* CHAP response failed */

#define CHAP_MD5		5	/* hash algorithm - MD5 */

/* states are named and numbered according to RFC 1661 */
#define STATE_INITIAL	0
#define STATE_STARTING	1
#define STATE_CLOSED	2
#define STATE_STOPPED	3
#define STATE_CLOSING	4
#define STATE_STOPPING	5
#define STATE_REQ_SENT	6
#define STATE_ACK_RCVD	7
#define STATE_ACK_SENT	8
#define STATE_OPENED	9

struct ppp_header {
	u_char address;
	u_char control;
	u_short protocol;
};
#define PPP_HEADER_LEN          sizeof (struct ppp_header)

struct lcp_header {
	u_char type;
	u_char ident;
	u_short len;
};
#define LCP_HEADER_LEN          sizeof (struct lcp_header)


/*
 * We follow the spelling and capitalization of RFC 1661 here, to make
 * it easier comparing with the standard.  Please refer to this RFC in
 * case you can't make sense out of these abbreviation; it will also
 * explain the semantics related to the various events and actions.
 */
struct cp {
	u_short	proto;		/* PPP control protocol number */
	u_char protoidx;	/* index into state table in struct sppp */
	u_char flags;
#define CP_LCP		0x01	/* this is the LCP */
#define CP_AUTH		0x02	/* this is an authentication protocol */
#define CP_NCP		0x04	/* this is a NCP */
#define CP_QUAL		0x08	/* this is a quality reporting protocol */
	const char *name;	/* name of this control protocol */
	/* event handlers */
	void	(*Up)(struct sppp *sp);
	void	(*Down)(struct sppp *sp);
	void	(*Open)(struct sppp *sp);
	void	(*Close)(struct sppp *sp);
	void	(*TO)(void *sp);
	int	(*RCR)(struct sppp *sp, struct lcp_header *h, int len);
	void	(*RCN_rej)(struct sppp *sp, struct lcp_header *h, int len);
	void	(*RCN_nak)(struct sppp *sp, struct lcp_header *h, int len);
	/* actions */
	void	(*tlu)(struct sppp *sp);
	void	(*tld)(struct sppp *sp);
	void	(*tls)(struct sppp *sp);
	void	(*tlf)(struct sppp *sp);
	void	(*scr)(struct sppp *sp);
};

struct sppp *spppq = NULL;
static struct callout keepalive_ch;
struct callout idle_ch;
char ppp_term_flag = 0;

#define	SPP_FMT		"%s "
#define	SPP_ARGS(sp)	(sp)->if_name

/*
 * The following disgusting hack gets around the problem that IP TOS
 * can't be set yet.  We want to put "interactive" traffic on a high
 * priority queue.  To decide if traffic is interactive, we check that
 * a) it is TCP and b) one of its ports is telnet, rlogin or ftp control.
 *
 * XXX is this really still necessary?  - joerg -
 */
static u_short interactive_ports[8] = {
	0,	513,	0,	0,
	0,	21,	0,	23,
};
#define INTERACTIVE(p) (interactive_ports[(p) & 7] == (p))

/* almost every function needs these */
//#define STDDCL							\
//	int debug = sp->pp_flags & IFF_DEBUG
#define STDDCL	int debug = sp->debug;
//#define STDDCL	int debug = 1;

//static int sppp_output(struct ifnet *ifp, struct mbuf *m,
//		       struct sockaddr *dst, struct rtentry *rt);

static void sppp_cp_input(const struct cp *cp, struct sppp *sp,
			  void *data, int len);
static void sppp_cp_send(struct sppp *sp, u_short proto, u_char type,
			 u_char ident, u_short len, void *data);
static void sppp_cp_down(struct sppp *sp);

/* static void sppp_cp_timeout(void *arg); */
static void sppp_cp_change_state(const struct cp *cp, struct sppp *sp,
				 int newstate);
static void sppp_auth_send(const struct cp *cp,
			   struct sppp *sp, unsigned int type, unsigned int id,
			   ...);

static void sppp_up_event(const struct cp *cp, struct sppp *sp);
static void sppp_down_event(const struct cp *cp, struct sppp *sp);
static void sppp_open_event(const struct cp *cp, struct sppp *sp);
static void sppp_close_event(const struct cp *cp, struct sppp *sp);
static void sppp_to_event(const struct cp *cp, struct sppp *sp);

static void sppp_null(struct sppp *sp);

static void sppp_lcp_init(struct sppp *sp);
static void sppp_lcp_up(struct sppp *sp);
static void sppp_lcp_down(struct sppp *sp);
static void sppp_lcp_open(struct sppp *sp);
static void sppp_lcp_close(struct sppp *sp);
static void sppp_lcp_TO(void *sp);
static int sppp_lcp_RCR(struct sppp *sp, struct lcp_header *h, int len);
static void sppp_lcp_RCN_rej(struct sppp *sp, struct lcp_header *h, int len);
static void sppp_lcp_RCN_nak(struct sppp *sp, struct lcp_header *h, int len);
static void sppp_lcp_tlu(struct sppp *sp);
static void sppp_lcp_tld(struct sppp *sp);
static void sppp_lcp_tls(struct sppp *sp);
static void sppp_lcp_tlf(struct sppp *sp);
static void sppp_lcp_scr(struct sppp *sp);
static void sppp_lcp_check_and_close(struct sppp *sp);
static int sppp_ncp_check(struct sppp *sp);

static void sppp_ipcp_init(struct sppp *sp);
static void sppp_ipcp_up(struct sppp *sp);
static void sppp_ipcp_down(struct sppp *sp);
static void sppp_ipcp_open(struct sppp *sp);
static void sppp_ipcp_close(struct sppp *sp);
static void sppp_ipcp_TO(void *sp);
static int sppp_ipcp_RCR(struct sppp *sp, struct lcp_header *h, int len);
static void sppp_ipcp_RCN_rej(struct sppp *sp, struct lcp_header *h, int len);
static void sppp_ipcp_RCN_nak(struct sppp *sp, struct lcp_header *h, int len);
static void sppp_ipcp_tlu(struct sppp *sp);
static void sppp_ipcp_tld(struct sppp *sp);
static void sppp_ipcp_tls(struct sppp *sp);
static void sppp_ipcp_tlf(struct sppp *sp);
static void sppp_ipcp_scr(struct sppp *sp);

#ifdef CONFIG_PPP_MPPE_MPPC

static void sppp_ccp_init(struct sppp *sp);
static void sppp_ccp_up(struct sppp *sp);
static void sppp_ccp_down(struct sppp *sp);
static void sppp_ccp_open(struct sppp *sp);
static void sppp_ccp_close(struct sppp *sp);
static void sppp_ccp_TO(void *sp);
static int sppp_ccp_RCR(struct sppp *sp, struct lcp_header *h, int len);
static void sppp_ccp_RCN_rej(struct sppp *sp, struct lcp_header *h, int len);
static void sppp_ccp_RCN_nak(struct sppp *sp, struct lcp_header *h, int len);
static void sppp_ccp_tlu(struct sppp *sp);
static void sppp_ccp_tld(struct sppp *sp);
static void sppp_ccp_tls(struct sppp *sp);
static void sppp_ccp_tlf(struct sppp *sp);
static void sppp_ccp_scr(struct sppp *sp);

#endif

//#ifdef CONFIG_IPV6
static void sppp_ipv6cp_init(struct sppp *sp);
static void sppp_ipv6cp_up(struct sppp *sp);
static void sppp_ipv6cp_down(struct sppp *sp);
static void sppp_ipv6cp_open(struct sppp *sp);
static void sppp_ipv6cp_close(struct sppp *sp);
static void sppp_ipv6cp_TO(void *sp);
static int sppp_ipv6cp_RCR(struct sppp *sp, struct lcp_header *h, int len);
static void sppp_ipv6cp_RCN_rej(struct sppp *sp, struct lcp_header *h, int len);
static void sppp_ipv6cp_RCN_nak(struct sppp *sp, struct lcp_header *h, int len);
static void sppp_ipv6cp_tlu(struct sppp *sp);
static void sppp_ipv6cp_tld(struct sppp *sp);
static void sppp_ipv6cp_tls(struct sppp *sp);
static void sppp_ipv6cp_tlf(struct sppp *sp);
static void sppp_ipv6cp_scr(struct sppp *sp);
//#endif

static void sppp_pap_input(struct sppp *sp, void *data, int len);
static void sppp_pap_init(struct sppp *sp);
static void sppp_pap_up(struct sppp *sp);
static void sppp_pap_open(struct sppp *sp);
static void sppp_pap_close(struct sppp *sp);
static void sppp_pap_TO(void *sp);
static void sppp_pap_my_TO(void *sp);
static void sppp_pap_tlu(struct sppp *sp);
static void sppp_pap_tld(struct sppp *sp);
static void sppp_pap_scr(struct sppp *sp);

static void sppp_chap_input(struct sppp *sp, void *data, int len);
static void sppp_chap_init(struct sppp *sp);
static void sppp_chap_up(struct sppp *sp);
static void sppp_chap_open(struct sppp *sp);
static void sppp_chap_close(struct sppp *sp);
static void sppp_chap_TO(void *sp);
static void sppp_chap_my_TO(void *sp);
static void sppp_chap_tlu(struct sppp *sp);
static void sppp_chap_tld(struct sppp *sp);
static void sppp_chap_scr(struct sppp *sp);

static const char *sppp_auth_type_name(u_short proto, u_char type);
static const char *sppp_cp_type_name(u_char type);
static const char *sppp_dotted_quad(u_long addr);
static const char *sppp_ipcp_opt_name(u_char opt);
#ifdef CONFIG_IPV6
static const char *ip6_sprintf(char *ip6buf, void *src);
static const char *sppp_ipv6cp_opt_name(u_char opt);
#endif
static const char *sppp_lcp_opt_name(u_char opt);
const char *sppp_phase_name(enum ppp_phase phase);
static const char *sppp_proto_name(u_short proto);
static const char *sppp_state_name(int state);
static const int sppp_proto_idx(u_short proto);
static int sppp_params(struct sppp *sp, u_long cmd, void *data);
static int sppp_strnlen(u_char *p, int max);
static void sppp_get_ip_addrs(struct sppp *sp, u_long *src, u_long *dst,
			      u_long *srcmask);
static void sppp_keepalive(void *dummy);
static void sppp_phase_network(struct sppp *sp);
//static void sppp_print_bytes(const u_char *p, u_short len);
//static void sppp_print_string(const char *p, u_short len);
static void sppp_bytes(const char *result, const u_char *p, u_short len);
static void sppp_string(const char *result, const char *p, u_short len);
static void sppp_set_ip_addr(struct sppp *sp, u_long src);
static void sppp_auth_log(int unit, int value);
#ifdef CONFIG_IPV6
static void sppp_get_ip6_addrs(struct sppp *sp, struct in6_addr *src,
			       struct in6_addr *dst, struct in6_addr *srcmask);
#ifdef IPV6CP_MYIFID_DYN
static void sppp_set_ip6_addr(struct sppp *sp, const struct in6_addr *src);
static void sppp_gen_ip6_addr(struct sppp *sp, const struct in6_addr *src);
#endif
static void sppp_suggest_ip6_addr(struct sppp *sp, struct in6_addr *src);
#endif // of CONFIG_IPV6
#ifdef _CWMP_MIB_
static void sppp_auto_disconnect_timeout(void *sp);
static void sppp_cp_down_delay(void *sp);
#endif
#ifdef CONFIG_USER_PPPOE_PROXY
//static int add_policy_routing_table(int wan_uint);
#endif
#ifdef CONFIG_USER_PPPOE_PROXY
#define PMAP_PPP_START	0x10
#define PMAP_NAS_PPP_START 0x30


void add_policy_routing_table(struct sppp * lan, struct sppp * wan, int users){
	int lan_unit,wan_unit;
	char ext_ifname[32];
	char primary_dns[32];
	char second_dns[32];

	lan_unit = lan->unit;
	wan_unit = wan->unit;
	DEBUG_PRINT("add_policy_routing_table, lan %d, wan %d, users %d\n",lan_unit, wan_unit, users);
#ifdef CONFIG_PPPOE_PROXY_IF_NAME
	char wan_pppif[IF_LANPPP_NAME_SIZE];
	char wan_unitif[IF_LANPPP_NAME_SIZE];
	char lan_pppif[IF_LANPPP_NAME_SIZE];
#else
	char wan_pppif[6];
	char wan_unitif[6];
	char lan_pppif[6];
#endif

	if(sameInterfaceType(wan, ALIASNAME_VC))
		snprintf(wan_unitif, 6, "%u", PMAP_PPP_START + wan_unit);
	else if(sameInterfaceType(wan, ALIASNAME_NAS))
		snprintf(wan_unitif, 6, "%u", PMAP_NAS_PPP_START + wan_unit);

#ifdef CONFIG_PPPOE_PROXY_IF_NAME
	if (wan_unit >= N_SPPP)
		snprintf(wan_pppif, IF_LANPPP_NAME_SIZE, "%s%u", IF_LANPPP_NAME_HR, wan_unit);
	else
        snprintf(wan_pppif, 6, "%s%u",ALIASNAME_PPP, wan_unit);
#else
    snprintf(wan_pppif, 6, "%s%u",ALIASNAME_PPP, wan_unit);
#endif
	//printf("wan_pppif %s, wan_unitif %s\n",wan_pppif, wan_unitif);
	if(users == 1){
		snprintf(ext_ifname, 32, "/var/ppp/ifup_%s", wan->if_name);
		va_cmd(ext_ifname, 0, 1);
	}
	//va_cmd("/bin/ip", 7, 1, "route", "add", "default", "dev", wan_pppif, "table", wan_unitif);

#ifdef CONFIG_PPPOE_PROXY_IF_NAME
	if (lan_unit >= N_SPPP)
		snprintf(lan_pppif, IF_LANPPP_NAME_SIZE, "%s%u", IF_LANPPP_NAME_HR, lan_unit); 
	else
		return;
#else
    snprintf(lan_pppif, 6, "%s%u",ALIASNAME_PPP, lan_unit);    	 
#endif
	//ip route add dns_server/32 dev ppp*
	if(wan->ipcp.primary_dns){
		sprintf(primary_dns, "%s/32", inet_ntoa(wan->ipcp.primary_dns));
		va_cmd("/bin/ip", 5, 1, "route", "add", primary_dns, "dev", wan_pppif);
	}
	if(wan->ipcp.second_dns){
		sprintf(second_dns, "%s/32", inet_ntoa(wan->ipcp.second_dns));
		va_cmd("/bin/ip", 5, 1, "route", "add", second_dns, "dev", wan_pppif);
	}
	va_cmd("/bin/ip", 6, 1, "rule", "add", "iif", lan_pppif, "table", caculate_tblid(wan->if_unit));
	va_cmd ("/bin/iptables", 6, 1, (char *) "-I", (char *) "ipfilter", (char *) "-i", lan_pppif, "-j", (char *) "RETURN");
	
	va_cmd("/bin/ip", 5, 1, "route", "add", "default","dev", wan_pppif);
}

void del_policy_routing_table(struct sppp * lan, struct sppp * wan, int users){
	int lan_unit,wan_unit;
	char ext_ifname[32];

	lan_unit = lan->unit;
	wan_unit = wan->unit;
	
#ifdef CONFIG_PPPOE_PROXY_IF_NAME
	char wan_pppif[IF_LANPPP_NAME_SIZE];
	char wan_unitif[IF_LANPPP_NAME_SIZE];
	char lan_pppif[IF_LANPPP_NAME_SIZE];
#else
	char wan_pppif[6];
	char wan_unitif[6];
	char lan_pppif[6];
#endif

	if(sameInterfaceType(wan, ALIASNAME_VC))
		snprintf(wan_unitif, 6, "%u", PMAP_PPP_START + wan_unit);
	else if(sameInterfaceType(wan, ALIASNAME_NAS))
		snprintf(wan_unitif, 6, "%u", PMAP_NAS_PPP_START + wan_unit);
	
#ifdef CONFIG_PPPOE_PROXY_IF_NAME
	if (wan_unit >= N_SPPP)
		snprintf(wan_pppif, IF_LANPPP_NAME_SIZE, "%s%u", IF_LANPPP_NAME_HR, wan_unit);
	else
        snprintf(wan_pppif, 6, "%s%u",ALIASNAME_PPP,wan_unit);
#else
    snprintf(wan_pppif, 6, "%s%u",ALIASNAME_PPP,wan_unit);

#endif
	if(users == 0){
		snprintf(ext_ifname, 32, "/var/ppp/ifdown_%s", wan->if_name);
		va_cmd(ext_ifname, 0, 1);
	}
	//va_cmd("/bin/ip", 7, 1, "route", "del", "default", "dev", wan_pppif, "table", wan_unitif);

#ifdef CONFIG_PPPOE_PROXY_IF_NAME
		if (lan_unit >= N_SPPP)
			snprintf(lan_pppif, IF_LANPPP_NAME_SIZE, "%s%u", IF_LANPPP_NAME_HR, lan_unit); 
		else
			return;
#else
		snprintf(lan_pppif, 6, "%s%u",ALIASNAME_PPP, lan_unit); 	 
#endif

	va_cmd("/bin/ip", 6, 1, "rule", "del", "iif", lan_pppif, "table", caculate_tblid(wan->if_unit));
	va_cmd ("/bin/iptables", 6, 1, (char *) "-D", (char *) "ipfilter", (char *) "-i", lan_pppif, "-j", (char *) "RETURN");
}
#endif

void ccp_flags_set (int unit, int isopen, int isup);


/* our control protocol descriptors */
static const struct cp lcp = {
	PPP_LCP, IDX_LCP, CP_LCP, "LCP",
	sppp_lcp_up, sppp_lcp_down, sppp_lcp_open, sppp_lcp_close,
	sppp_lcp_TO, sppp_lcp_RCR, sppp_lcp_RCN_rej, sppp_lcp_RCN_nak,
	sppp_lcp_tlu, sppp_lcp_tld, sppp_lcp_tls, sppp_lcp_tlf,
	sppp_lcp_scr
};

static const struct cp ipcp = {
	PPP_IPCP, IDX_IPCP, CP_NCP, "IPCP",
	sppp_ipcp_up, sppp_ipcp_down, sppp_ipcp_open, sppp_ipcp_close,
	sppp_ipcp_TO, sppp_ipcp_RCR, sppp_ipcp_RCN_rej, sppp_ipcp_RCN_nak,
	sppp_ipcp_tlu, sppp_ipcp_tld, sppp_ipcp_tls, sppp_ipcp_tlf,
	sppp_ipcp_scr
};
#ifdef CONFIG_PPP_MPPE_MPPC

static const struct cp ccp = {
	PPP_CCP, IDX_CCP, CP_NCP, "CCP",
	sppp_ccp_up, sppp_ccp_down, sppp_ccp_open, sppp_ccp_close,
	sppp_ccp_TO, sppp_ccp_RCR, sppp_ccp_RCN_rej, sppp_ccp_RCN_nak,
	sppp_ccp_tlu, sppp_ccp_tld, sppp_ccp_tls, sppp_ccp_tlf,
	sppp_ccp_scr
};
#endif

//#ifdef CONFIG_IPV6
static const struct cp ipv6cp = {
	PPP_IPV6CP, IDX_IPV6CP,
#ifdef CONFIG_IPV6	/*don't run IPv6CP if there's no IPv6 support*/
	CP_NCP,
#else
	0,
#endif
	"IPV6CP",
	sppp_ipv6cp_up, sppp_ipv6cp_down, sppp_ipv6cp_open, sppp_ipv6cp_close,
	sppp_ipv6cp_TO, sppp_ipv6cp_RCR, sppp_ipv6cp_RCN_rej, sppp_ipv6cp_RCN_nak,
	sppp_ipv6cp_tlu, sppp_ipv6cp_tld, sppp_ipv6cp_tls, sppp_ipv6cp_tlf,
	sppp_ipv6cp_scr
};
//#endif

static const struct cp pap = {
	PPP_PAP, IDX_PAP, CP_AUTH, "PAP",
	sppp_pap_up, sppp_null, sppp_pap_open, sppp_pap_close,
	sppp_pap_TO, 0, 0, 0,
	sppp_pap_tlu, sppp_pap_tld, sppp_null, sppp_null,
	sppp_pap_scr
};

static const struct cp chap = {
	PPP_CHAP, IDX_CHAP, CP_AUTH, "CHAP",
	sppp_chap_up, sppp_null, sppp_chap_open, sppp_chap_close,
	sppp_chap_TO, 0, 0, 0,
	sppp_chap_tlu, sppp_chap_tld, sppp_null, sppp_null,
	sppp_chap_scr
};

static const struct cp *cps[IDX_COUNT] = {
	&lcp,			/* IDX_LCP */
	&ipcp,			/* IDX_IPCP */
//#ifdef CONFIG_IPV6
	&ipv6cp,		/* IDX_IPV6CP */
//#endif
	&pap,			/* IDX_PAP */
	&chap,			/* IDX_CHAP */
	#ifdef CONFIG_PPP_MPPE_MPPC
    &ccp,			/* IDX_CCP */
	#endif
};

#if 0
// Added by Mason Yu for PPP LED
int do_cmd(const char *filename, char *argv [], int dowait)
{
	pid_t pid, wpid;
	int stat, st;
	
	if((pid = vfork()) == 0) {
		/* the child */
		char *env[3];
		
		signal(SIGINT, SIG_IGN);
		argv[0] = (char *)filename;
		env[0] = "PATH=/bin:/usr/bin:/etc:/sbin:/usr/sbin";
		env[1] = NULL;

		execve(filename, argv, env);

		printf("exec %s failed\n", filename);
		_exit(2);
	} else if(pid > 0) {
		if (!dowait)
			stat = 0;
		else {
			/* parent, wait till rc process dies before spawning */
			while ((wpid = wait(&stat)) != pid)
				if (wpid == -1 && errno == ECHILD) { /* see wait(2) manpage */
					stat = 0;
					break;
				}
		}
	} else if(pid < 0) {
		printf("fork of %s failed\n", filename);
		stat = -1;
	}
	//st = WEXITSTATUS(stat);
	return st;
}

void va_cmd(const char *cmd, int num, int dowait, ...)
{
	va_list ap;
	int k;
	char *s;
	char *argv[19];
	
	//TRACE(STA_SCRIPT, "%s ", cmd);
	va_start(ap, dowait);
	
	for (k=0; k<num; k++)
	{
		s = va_arg(ap, char *);
		argv[k+1] = s;
		//TRACE(STA_SCRIPT|STA_NOTAG, "%s ", s);
	}
	
	//TRACE(STA_SCRIPT|STA_NOTAG, "\n");
	argv[k+1] = NULL;
	do_cmd(cmd, argv, dowait);
	va_end(ap);
}
#endif



/*
 * Exported functions, comprising our interface to the lower layer.
 */

/*
 * Process the received packet.
 */
void
sppp_input(struct sppp *sp, void *data, int len)
{
	struct ppp_header *h;
	u_char *ppp_hdr;
	struct ifqueue *inq = 0;
    int	isr;
	int s;
	//struct sppp *sp = (struct sppp *)ifp;
	u_char *iphdr;
	int hlen, vjlen, do_account = 0;
	u_short	protocol;
	STDDCL;


	if (len <= PPP_HEADER_LEN) {
		/* Too small packet, drop it. */
		if (debug)
			addlog(
			    SPP_FMT "input packet is too small, %d bytes\n",
			    SPP_ARGS(sp), len);
drop:
		return;
	}

	/* Get PPP header. */

	if (sp->pp_flags & PP_ADDRCOMP) {
		ppp_hdr = (u_char*)data;
		protocol = *(u_short*)ppp_hdr;
		data+=2;
		len -= 2;
	}
	else 
	{
		h = (struct ppp_header*)data;
		data += PPP_HEADER_LEN;
		len -= PPP_HEADER_LEN;

		switch (h->address) {
		case PPP_ALLSTATIONS:
			if (h->control != PPP_UI)
				goto invalid;
			protocol = ntohs (h->protocol);
			break;
		default:        /* Invalid PPP packet. */
		invalid:
			if (debug)
				addlog(
				    SPP_FMT "invalid input packet "
			    	"<addr=0x%x ctrl=0x%x proto=0x%x>\n",
			    	SPP_ARGS(sp),
			   		h->address, h->control, ntohs(h->protocol));
			goto drop;
		}
	}				
    
	switch (protocol) {

#ifdef CONFIG_PPP_MPPE_MPPC
//            case PPP_COMP:
//              printf("PPP_COMP\n");               
//            return;
	case PPP_CCP: 
		if(sp->myauth.enc_type) 
		{
			if (sp->pp_phase == PHASE_NETWORK){
				sppp_cp_input(&ccp, sp, data, len);
			}
			return;
		}

#endif	

	
	default:
		if (sp->pp_flags & PP_ADDRCOMP) {
			if (debug)
				addlog(SPP_FMT "rejecting protocol <proto=0x%x>\n",
		    		SPP_ARGS(sp), protocol);
			if (sp->state[IDX_LCP] == STATE_OPENED)
				sppp_cp_send (sp, PPP_LCP, PROTO_REJ,
					++sp->pp_seq[IDX_LCP], len + 2,	ppp_hdr);
		}
		else {
		if (debug)
			addlog(
		    	SPP_FMT "rejecting protocol "
		    	"<addr=0x%x ctrl=0x%x proto=0x%x>\n",
		    	SPP_ARGS(sp),
		    	h->address, h->control, ntohs(h->protocol));
		if (sp->state[IDX_LCP] == STATE_OPENED)
				sppp_cp_send (sp, PPP_LCP, PROTO_REJ,
				++sp->pp_seq[IDX_LCP], len + 2,
				&h->protocol);
			
		}
		//++ifp->if_noproto;
		goto drop;
	case PPP_LCP:
		sppp_cp_input(&lcp, sp, data, len);
		return;
	case PPP_PAP:
		if (sp->pp_phase >= PHASE_AUTHENTICATE)
			sppp_pap_input(sp, data, len);
		return;
	case PPP_CHAP:
		if (sp->pp_phase >= PHASE_AUTHENTICATE)
			sppp_chap_input(sp, data, len);
		return;
	case PPP_IPCP:
		// Kaohj -- 0: ipv4, 1: ipv6, 2: both
		if (sp->ipType != 0 && sp->ipType != 2)
			return;
		if (sp->pp_phase == PHASE_NETWORK)
			sppp_cp_input(&ipcp, sp, data, len);
		return;
	case PPP_IP:
		if (sp->state[IDX_IPCP] == STATE_OPENED) {
			printf("received IP packet!!\n");
		}
		do_account++;
		break;
#ifdef CONFIG_IPV6
	case PPP_IPV6CP:
		// Kaohj -- 0: ipv4, 1: ipv6, 2: both
		if (sp->ipType != 1 && sp->ipType != 2)
			return;
		if (sp->pp_phase == PHASE_NETWORK)
		    sppp_cp_input(&ipv6cp, sp, data, len);
		return;

	case PPP_IPV6:
		if (sp->state[IDX_IPV6CP] == STATE_OPENED)
			printf("received IPv6 packet!!\n");
		do_account++;
		break;
#endif
	}

	//if (! (sp->pp_flags & IFF_UP) || ! inq)
	//	goto drop;

	/* send to upper layer interface */
	

	if (do_account)
		/*
		 * Do only account for network packets, not for control
		 * packets.  This is used by some subsystems to detect
		 * idle lines.
		 */
		sp->pp_last_recv = time_second;

}

#if 0
/*
 * Enqueue transmit packet.
 */
static int
sppp_output(struct ifnet *ifp, struct mbuf *m,
	    struct sockaddr *dst, struct rtentry *rt)
{
	struct sppp *sp = (struct sppp*) ifp;
	struct ppp_header *h;
	struct ifqueue *ifq = NULL;
	int s,rv = 0;
	int ipproto = PPP_IP;
	int ppp_header_len;
	STDDCL;

	s = splimp();

	if ((sp->pp_flags & IFF_UP) == 0 ||
	    (sp->pp_flags & (IFF_RUNNING | IFF_AUTO)) == 0) {
		m_freem (m);
		splx (s);
		return (ENETDOWN);
	}

	if ((sp->pp_flags & (IFF_RUNNING | IFF_AUTO)) == IFF_AUTO) {
		/*
		 * Interface is not yet running, but auto-dial.  Need
		 * to start LCP for it.
		 */
		sp->pp_flags |= IFF_RUNNING;
		splx(s);
		lcp.Open(sp);
		s = splimp();
	}

	ifq = &ifp->if_snd;
	if (dst->sa_family == AF_INET) {
		/* XXX Check mbuf length here? */
		struct ip *ip = mtod (m, struct ip*);
		struct tcphdr *tcp = (struct tcphdr*) ((long*)ip + ip->ip_hl);

		/*
		 * When using dynamic local IP address assignment by using
		 * 0.0.0.0 as a local address, the first TCP session will
		 * not connect because the local TCP checksum is computed
		 * using 0.0.0.0 which will later become our real IP address
		 * so the TCP checksum computed at the remote end will
		 * become invalid. So we
		 * - don't let packets with src ip addr 0 thru
		 * - we flag TCP packets with src ip 0 as an error
		 */

		if(ip->ip_src.s_addr == INADDR_ANY)	/* -hm */
		{
			m_freem(m);
			splx(s);
			if(ip->ip_p == IPPROTO_TCP)
				return(EADDRNOTAVAIL);
			else
				return(0);
		}

		/*
		 * Put low delay, telnet, rlogin and ftp control packets
		 * in front of the queue.
		 */
#if 0
		if (IF_QFULL (&sp->pp_fastq))
			;
		else if (ip->ip_tos & IPTOS_LOWDELAY)
			ifq = &sp->pp_fastq;
		else if (m->m_len < sizeof *ip + sizeof *tcp)
			;
		else if (ip->ip_p != IPPROTO_TCP)
			;
		else if (INTERACTIVE (ntohs (tcp->th_sport)))
			ifq = &sp->pp_fastq;
		else if (INTERACTIVE (ntohs (tcp->th_dport)))
			ifq = &sp->pp_fastq;
#endif

		/*
		 * Do IP Header compression
		 */
	}

	/*
	 * Prepend general data packet PPP header. For now, IP only.
 	 */
	ppp_header_len = ((sp->pp_flags & PP_ADDRCOMP) ? 2: PPP_HEADER_LEN);
	M_PREPEND (m, ppp_header_len, M_DONTWAIT);
	if (! m) {
		if (debug)
			addlog(SPP_FMT "no memory for transmit header\n",
				SPP_ARGS(sp));
		++ifp->if_oerrors;
		splx (s);
		return (ENOBUFS);
	}
	
	switch (dst->sa_family) {
	case AF_INET:   /* Internet Protocol */
		/*
		 * Don't choke with an ENETDOWN early.  It's
		 * possible that we just started dialing out,
		 * so don't drop the packet immediately.  If
		 * we notice that we run out of buffer space
		 * below, we will however remember that we are
		 * not ready to carry IP packets, and return
		 * ENETDOWN, as opposed to ENOBUFS.
		 */
		if (sp->pp_flags & PP_ADDRCOMP)
			*mtod(m,u_short*) = htons(ipproto);
		else
		{
			h = mtod (m, struct ppp_header*);
			h->address = PPP_ALLSTATIONS;        /* broadcast address */
			h->control = PPP_UI;                 /* Unnumbered Info */
			h->protocol = htons(ipproto);
		}
		if (sp->state[IDX_IPCP] != STATE_OPENED)
			rv = ENETDOWN;
		break;
	default:
		m_freem (m);
		++ifp->if_oerrors;
		splx (s);

	}

	/*
	 * Queue message on interface, and start output if interface
	 * not yet active.
	 */
	if (IF_QFULL (ifq)) {
		IF_DROP (&ifp->if_snd);
		m_freem (m);
		++ifp->if_oerrors;
		splx (s);
		return (rv? rv: ENOBUFS);
	}
	IF_ENQUEUE (ifq, m);
    ifp->if_opackets++;
    ifp->if_obytes += m->m_pkthdr.len;
	if (! (sp->pp_flags & IFF_OACTIVE))
		(*ifp->if_start) (ifp);

	/*
	 * Count output packets and bytes.
	 * The packet length includes header, FCS and 1 flag,
	 * according to RFC 1333.
	 */
	ifp->if_obytes += m->m_pkthdr.len + 3;
	splx (s);
	/*
	 * Unlike in sppp_input(), we can always bump the timestamp
	 * here since sppp_output() is only called on behalf of
	 * network-layer traffic; control-layer traffic is handled
	 * by sppp_cp_send().
	 */
	sp->pp_last_sent = time_second;
	return (0);
}
#endif



void
sppp_init(struct sppp *sp)
{
	sppp_lcp_init(sp);
	sppp_ipcp_init(sp);
#ifdef CONFIG_IPV6
	sppp_ipv6cp_init(sp);
#endif
	sppp_pap_init(sp);
	sppp_chap_init(sp);
#ifdef CONFIG_PPP_MPPE_MPPC	
	sppp_ccp_init(sp);
#endif

}

extern int ppp_down_flag[N_SPPP];
void sppp_term_send()
{
	struct sppp *sp;

	va_cmd("/bin/sarctl", 1, 1, "ppp_term");
	for (sp=spppq; sp; sp=sp->pp_next) {
		#if 0
		if (sp->over == SPPP_PPPOE)
			va_cmd("/bin/sarctl", 1, 1, "ppp_term");
		else if (sp->over == SPPP_PPPOATM) {
			pkt_len = 6;

			*(u_short*)pkt_buf = htons(PPP_LCP);
			lh = (struct lcp_header*)((u_char*)pkt_buf + 2);

			lh->type = TERM_REQ;
			lh->ident = (u_char)(0x00);
			lh->len = htons (LCP_HEADER_LEN);

			if (sp->debug) {
				addlog(SPP_FMT "%s: O %s [%s] id=0x%x len=%d <",
				    SPP_ARGS(sp),
				    sppp_proto_name(PPP_LCP),
				    sppp_cp_type_name (lh->type),
				    sppp_state_name(sp->state[sppp_proto_idx(PPP_LCP)]), 
				    lh->ident,
				    ntohs (lh->len));
				sppp_print_bytes ((u_char*) (lh+1), pkt_len);
				addlog(">\n");
			}
			/* send to lower layer interface */
				output(sp, pkt_buf, pkt_len);
			if (!ppp_down_flag[sp->unit])
				sp->pp_up(sp);
		}
		#endif
		
		// do not start dial-on-demand ppp here, it is triggered in dial_start()
		if (sp->over == SPPP_PPPOATM && !ppp_down_flag[sp->unit] && !sp->timeout)
			sp->pp_up(sp);
	}
	ppp_term_flag = 1;
}

void
sppp_attach(struct sppp *sp)
{
	
	/* Initialize keepalive handler. */
	if (! spppq)
	{
		TIMEOUT(sppp_keepalive, 0, hz * 10, keepalive_ch);
	}
//	TIMEOUT(sppp_keepalive, 0, hz * 10, keepalive_ch);

	/* Insert new entry into the keepalive list. */
	sp->pp_next = spppq;
	spppq = sp;

 	sp->pp_flags = PP_ADDRCOMP | PP_KEEPALIVE;
 	if(sp->timeout) {
 		sp->idletime = 0;
 	}
	sp->pp_loopcnt = 0;
	sp->pp_alivecnt = 0;
	bzero((char*)&sp->pp_seq[0], sizeof(sp->pp_seq));
	bzero((char*)&sp->pp_rseq[0], sizeof(sp->pp_rseq));
	sp->pp_phase = PHASE_DEAD;
	sp->pp_up = lcp.Up;
	//sp->pp_down = lcp.Down;
	sp->pp_down = sppp_cp_down;
	sp->pp_last_recv = sp->pp_last_sent = time_second;
	
	sppp_init(sp);

}

void
sppp_detach(struct sppp *sp)
{
	struct sppp **q, *p;
	int i;
	int status;

	/* Remove the entry from the keepalive list. */
	for (q = &spppq; (p = *q); q = &p->pp_next)
		if (p == sp) {
			*q = p->pp_next;
			break;
		}

	/* Stop keepalive handler. */
	if (! spppq) {
		UNTIMEOUT(sppp_keepalive, 0, keepalive_ch);
	}

	for (i = 0; i < IDX_COUNT; i++)
		UNTIMEOUT((cps[i])->TO, (void *)sp, sp->ch[i]);
	UNTIMEOUT(sppp_pap_my_TO, (void *)sp, sp->pap_my_to_ch);
	UNTIMEOUT(sppp_chap_my_TO, (void *)sp, sp->chap_my_to_ch);
#ifdef _CWMP_MIB_
	UNTIMEOUT(sppp_auto_disconnect_timeout, 0, sp->autodisconnect_ch);
	UNTIMEOUT(sppp_cp_down_delay, 0, sp->disconnectdelay_ch);
#endif
	UNTIMEOUT(authTimeout, 0, sp->auth_ch);
	if (sp->timeout) {
		UNTIMEOUT(sppp_idle, 0, idle_ch);
		sp->timeout = 0;
	}
//#ifdef CONFIG_USER_PPPOE_PROXY
#if 0
	if(sp->enable_pppoe_proxy)
		{
		//printf("\033[0;32mcallout %p shouldbeen deleted!!!\033[0m\n",&(sp->link_ch));
		UNTIMEOUT(linkTimeout, 0, sp->link_ch);
		}
#endif
}

//#if 0
// Jenny, ppp interface down
void sppp_if_down(struct sppp *sp)
{
	int i;

	for (i = 0; i < IDX_COUNT; i++)
		UNTIMEOUT((cps[i])->TO, (void *)sp, sp->ch[i]);
	UNTIMEOUT(sppp_pap_my_TO, (void *)sp, sp->pap_my_to_ch);
	UNTIMEOUT(sppp_chap_my_TO, (void *)sp, sp->chap_my_to_ch);
#ifdef _CWMP_MIB_
	UNTIMEOUT(sppp_auto_disconnect_timeout, 0, sp->autodisconnect_ch);
	UNTIMEOUT(sppp_cp_down_delay, 0, sp->disconnectdelay_ch);
#endif
	UNTIMEOUT(authTimeout, 0, sp->auth_ch);
	if (sp->timeout) {
		UNTIMEOUT(sppp_idle, 0, idle_ch);
		//sp->timeout = 0;
	}

 	sp->pp_flags = PP_ADDRCOMP | PP_KEEPALIVE;
 	if(sp->timeout) {
 		sp->idletime = 0;
 	}
	sp->pp_loopcnt = 0;
	sp->pp_alivecnt = 0;
	bzero((char*)&sp->pp_seq[0], sizeof(sp->pp_seq));
	bzero((char*)&sp->pp_rseq[0], sizeof(sp->pp_rseq));
	sp->pp_phase = PHASE_DEAD;
	sp->pp_up = lcp.Up;
	sp->pp_down = sppp_cp_down;
	sp->pp_last_recv = sp->pp_last_sent = time_second;
	
	sppp_init(sp);
}
//#endif

#ifdef _CWMP_MIB_
// Jenny, set to 1 when in "pending" state
int ppp_pending_flag[N_SPPP] = {0};
#endif
// Jenny, check on-demand timeout
void sppp_idle(void *dummy)
{
	struct sppp *sp;
	
	for (sp=spppq; sp; sp=sp->pp_next) {

		/* channel down? */
		if (!(sp->pp_flags & IFF_RUNNING))
			continue;

		/* LCP not opened yet. */
		if (sp->pp_phase < PHASE_AUTHENTICATE)
			continue;

		if(sp->pp_phase == PHASE_NETWORK)
			if(sp->timeout) {
				if(chk_ifxmtrcv(sp)) {
					const struct cp *cp = &lcp;
#ifdef _CWMP_MIB_
					if (ppp_pending_flag[sp->if_unit])
						return;
					ppp_pending_flag[sp->if_unit] = 1;
					if (sp->warnDisconnectDelay)
						printf (SPP_FMT "will down in %d seconds!\n", SPP_ARGS(sp), sp->warnDisconnectDelay);
					TIMEOUT(sppp_cp_down_delay, (void *)sp, (sp->warnDisconnectDelay)* hz, sp->disconnectdelay_ch);
#else
					(cp->tld)(sp);
					sp->rst_counter[cp->protoidx] = 0;
					sppp_cp_change_state(cp, sp, STATE_STOPPING);
#endif
					sppp_last_connection_error(sp->if_unit, ERROR_IDLE_DISCONNECT);
					syslog(LOG_INFO, "spppd: %s (connect on-demand)): idle %d seconds, disconnected\n", sp->if_name, sp->timeout);
					return;
				}
			}
	}
	TIMEOUT(sppp_idle, 0, 1, idle_ch);
}

#if 0
/*
 * Process an ioctl request.  Called on low priority level.
 */
int
sppp_ioctl(struct ifnet *ifp, IOCTL_CMD_T cmd, void *data)
{
	struct ifreq *ifr = (struct ifreq*) data;
	struct sppp *sp = (struct sppp*) ifp;
	int s, rv, going_up, going_down, newmode;

	s = splimp();
	rv = 0;
	switch (cmd) {
	case SIOCAIFADDR:
	case SIOCSIFDSTADDR:
		break;

	case SIOCSIFADDR:
		/* set the interface "up" when assigning an IP address */
		sp->pp_flags |= IFF_UP;
		/* fall through... */

	case SIOCSIFFLAGS:
		going_up = sp->pp_flags & IFF_UP &&
			(sp->pp_flags & IFF_RUNNING) == 0;
		going_down = (sp->pp_flags & IFF_UP) == 0 &&
			sp->pp_flags & IFF_RUNNING;

		newmode = sp->pp_flags & IFF_PASSIVE;
		if (!newmode)
			newmode = sp->pp_flags & IFF_AUTO;
		sp->pp_flags &= ~(IFF_PASSIVE | IFF_AUTO);
		sp->pp_flags |= newmode;

		if (newmode != sp->pp_mode) {
			going_down = 1;
			if (!going_up)
				going_up = sp->pp_flags & IFF_RUNNING;
		}

		if (going_down) {
			lcp.Close(sp);
			sppp_flush(ifp);
			sp->pp_flags &= ~IFF_RUNNING;
			sp->pp_mode = newmode;
		}

		if (going_up) {
			lcp.Close(sp);
			sp->pp_mode = newmode;
			if (sp->pp_mode == 0) {
				sp->pp_flags |= IFF_RUNNING;
				lcp.Open(sp);
			}
		}

		break;

#ifdef SIOCSIFMTU
#ifndef ifr_mtu
#define ifr_mtu ifr_metric
#endif
	case SIOCSIFMTU:
		if (ifr->ifr_mtu < 128 || ifr->ifr_mtu > sp->lcp.their_mru)
			return (EINVAL);
		ifp->if_mtu = ifr->ifr_mtu;
		break;
#endif
#ifdef SLIOCSETMTU
	case SLIOCSETMTU:
		if (*(short*)data < 128 || *(short*)data > sp->lcp.their_mru)
			return (EINVAL);
		ifp->if_mtu = *(short*)data;
		break;
#endif
#ifdef SIOCGIFMTU
	case SIOCGIFMTU:
		ifr->ifr_mtu = ifp->if_mtu;
		break;
#endif
#ifdef SLIOCGETMTU
	case SLIOCGETMTU:
		*(short*)data = ifp->if_mtu;
		break;
#endif
	case SIOCADDMULTI:
	case SIOCDELMULTI:
		break;

	case SIOCGIFGENERIC:
	case SIOCSIFGENERIC:
		rv = sppp_params(sp, cmd, data);
		break;

	default:
		rv = ENOTTY;
	}
	splx(s);
	return rv;
}
#endif

int get_ipcpstate(struct sppp *sp){
	if(sp->state[ipcp.protoidx] == STATE_OPENED)
		return 1;
	else return 0;
}
int get_lcpstate(struct sppp *sp){
	if(sp->state[lcp.protoidx] == STATE_STOPPED || sp->state[lcp.protoidx] == STATE_CLOSED)
		return 0;
	else return 1;
}

/*
 * PPP protocol implementation.
 */

/*
 * Send PPP control protocol packet.
 */
static void
sppp_cp_send(struct sppp *sp, u_short proto, u_char type,
	     u_char ident, u_short len, void *data)
{
	STDDCL;
	//u_char pkt_buf[256];
	u_char pkt_buf[MHLEN];
	u_short pkt_len;
	struct ppp_header *h;
	struct lcp_header *lh;
	int	ppp_header_len = (sp->pp_flags & PP_ADDRCOMP) ? 2 : PPP_HEADER_LEN;

	if (len > MHLEN - ppp_header_len - LCP_HEADER_LEN)
		len = MHLEN - ppp_header_len - LCP_HEADER_LEN;

	pkt_len = ppp_header_len + LCP_HEADER_LEN + len;

	if (sp->pp_flags & PP_ADDRCOMP) {
		*(u_short*)pkt_buf = htons(proto);
		lh = (struct lcp_header*)((u_char*)pkt_buf + 2);
	} else {
		h = (struct ppp_header*)pkt_buf;
		h->address = PPP_ALLSTATIONS;        /* broadcast address */
		h->control = PPP_UI;                 /* Unnumbered Info */
		h->protocol = htons (proto);         /* Link Control Protocol */
		lh = (struct lcp_header*) (h + 1);
	}

	lh->type = type;
	lh->ident = ident;
	lh->len = htons (LCP_HEADER_LEN + len);
	if (len)
		bcopy ((char*)data, (char*)(lh+1), len);

	if (debug) {
		/*addlog(SPP_FMT "%s: O %s [%s] id=0x%x len=%d <",
		    SPP_ARGS(sp),
		    sppp_proto_name(proto),
		    sppp_cp_type_name (lh->type),
		    sppp_state_name(sp->state[sppp_proto_idx(proto)]), 
		    lh->ident,
		    ntohs (lh->len));
		sppp_print_bytes ((u_char*) (lh+1), len);
		addlog(">\n");*/
		//char bytes[256];
		char bytes[MHLEN];
		bytes[0] = 0;
		sppp_bytes(bytes, (u_char*)(lh+1), len);
		addlog(SPP_FMT "%s: O %s [%s] id=0x%x len=%d <%s>\n",
		    SPP_ARGS(sp),
		    sppp_proto_name(proto),
		    sppp_cp_type_name (lh->type),
		    sppp_state_name(sp->state[sppp_proto_idx(proto)]), 
		    lh->ident,
		    ntohs (lh->len), bytes);
	}

	/* send to lower layer interface */
	output(sp, pkt_buf, pkt_len);
	/* */
	
}

static void 
sppp_cp_down(struct sppp *sp)
{
	STDDCL;
	const struct cp *cp = &lcp;

	switch (sp->state[cp->protoidx]) {
	case STATE_ACK_RCVD:
	case STATE_ACK_SENT:
		sppp_cp_change_state(cp, sp, STATE_REQ_SENT);
		break;
	case STATE_OPENED:
#ifdef _CWMP_MIB_
		if (ppp_pending_flag[sp->if_unit])
			return;
		ppp_pending_flag[sp->if_unit] = 1;
		if (sp->warnDisconnectDelay) {
			printf (SPP_FMT "will down in %d seconds!\n", SPP_ARGS(sp), sp->warnDisconnectDelay);
		TIMEOUT(sppp_cp_down_delay, (void *)sp, (sp->warnDisconnectDelay)* hz, sp->disconnectdelay_ch);
		}
		else {
			(cp->tld)(sp);
			sp->rst_counter[cp->protoidx] = sp->lcp.max_terminate;
			sppp_cp_send(sp, cp->proto, TERM_REQ, ++sp->pp_seq[cp->protoidx], 0, 0);
			sppp_cp_change_state(cp, sp, STATE_CLOSING);
		}
#else
		#if 0
		(cp->tld)(sp);
		sp->rst_counter[cp->protoidx] = 0;
		sppp_cp_change_state(cp, sp, STATE_STOPPING);
		#endif
		(cp->tld)(sp);
		sp->rst_counter[cp->protoidx] = sp->lcp.max_terminate;
		sppp_cp_send(sp, cp->proto, TERM_REQ, ++sp->pp_seq[cp->protoidx], 0, 0);
		sppp_cp_change_state(cp, sp, STATE_CLOSING);

#endif
		break;
	case STATE_CLOSED:
	case STATE_STOPPED:
	case STATE_CLOSING:
	case STATE_STOPPING:
	case STATE_REQ_SENT:
	default:
		break;
	}
	return;
}

unsigned int auth_fail_counter[N_SPPP] = {0};	// Jenny, for max authentication fail counter
/*
 * Handle incoming PPP control protocol packets.
 */
static void
sppp_cp_input(const struct cp *cp, struct sppp *sp, void *data, int len)
{
	STDDCL;
	struct lcp_header *h;
	int rv;
	u_char *p;

	if (len < 4) {
		if (debug)
			addlog(SPP_FMT "%s invalid packet length: %d bytes\n",
			    SPP_ARGS(sp), cp->name, len);
		return;
	}
	
	h = (struct lcp_header*)data;
	if (debug) {
		/*addlog(SPP_FMT "%s: I %s [%s] id=0x%x len=%d <",
		    SPP_ARGS(sp), cp->name,
		    sppp_cp_type_name (h->type), 
   		    sppp_state_name(sp->state[cp->protoidx]),
			h->ident, ntohs(h->len));
		sppp_print_bytes ((u_char*) (h+1), ntohs(h->len)-4);
		addlog(">\n");*/
		//char bytes[256];
		char bytes[MHLEN];
		bytes[0] = 0;
		sppp_bytes(bytes, (u_char*)(h+1), ntohs(h->len)-4);
		addlog(SPP_FMT "%s: I %s [%s] id=0x%x len=%d <%s>\n",
		    SPP_ARGS(sp), cp->name,
		    sppp_cp_type_name (h->type), 
   		    sppp_state_name(sp->state[cp->protoidx]),
			h->ident, ntohs(h->len), bytes);
	}
	if (len > ntohs (h->len))
		len = ntohs (h->len);
	p = (u_char *)(h + 1);

	//if (cp == &ipcp)
	//	printf("recive ipcp [type=%d], state=%d\n", h->type, sp->state[cp->protoidx]);
	
	switch (h->type) {
	case CONF_REQ:
		if (len < 4) {
			printf("len < 4!!\n");
			if (debug)
				addlog(SPP_FMT "%s invalid conf-req length %d\n",
				       SPP_ARGS(sp), cp->name,
				       len);
			//++ifp->if_ierrors;
			break;
		}
		/* handle states where RCR doesn't get a SCA/SCN */
		switch (sp->state[cp->protoidx]) {
		case STATE_CLOSING:
		case STATE_STOPPING:
			return;
		case STATE_CLOSED:
			sppp_cp_send(sp, cp->proto, TERM_ACK, h->ident,
				     0, 0);
			return;
		}
		rv = (cp->RCR)(sp, h, len);
		switch (sp->state[cp->protoidx]) {
		case STATE_OPENED:
			(cp->tld)(sp);
			(cp->scr)(sp);
			/* fall through... */
		case STATE_ACK_SENT:
		case STATE_REQ_SENT:
			/*
			 * sppp_cp_change_state() have the side effect of
			 * restarting the timeouts. We want to avoid that
			 * if the state don't change, otherwise we won't
			 * ever timeout and resend a configuration request
			 * that got lost.
			 */
#ifdef DIAGNOSTIC_TEST
			if (sp->pp_phase == PHASE_ESTABLISH)
				sppp_diag_log(sp->if_unit, 1);	// Jenny, diagnostic test, PPP server connection pass
#endif
			if (sp->state[cp->protoidx] == (rv ? STATE_ACK_SENT:
			    STATE_REQ_SENT))
				break;
			sppp_cp_change_state(cp, sp, rv?
					     STATE_ACK_SENT: STATE_REQ_SENT);
			break;
		case STATE_STOPPED:
			sp->rst_counter[cp->protoidx] = sp->lcp.max_configure;
			(cp->scr)(sp);
			sppp_cp_change_state(cp, sp, rv?
					     STATE_ACK_SENT: STATE_REQ_SENT);
			break;
		case STATE_ACK_RCVD:
			if (rv) {
				sppp_cp_change_state(cp, sp, STATE_OPENED);
				//if (debug)
				//	addlog(SPP_FMT "%s tlu\n",
				//	    SPP_ARGS(sp),
				//	    cp->name);
				(cp->tlu)(sp);
			} else
				sppp_cp_change_state(cp, sp, STATE_ACK_RCVD);
			break;
		default:
			printf(SPP_FMT "%s illegal %s in state %s\n",
			       SPP_ARGS(sp), cp->name,
			       sppp_cp_type_name(h->type),
			       sppp_state_name(sp->state[cp->protoidx]));
			//++ifp->if_ierrors;
		}
		break;
	case CONF_ACK:
		// Casey, for Slow RedBack
		//if (h->ident != sp->confid[cp->protoidx]) {
		if (h->ident > sp->confid[cp->protoidx]) {
			if (debug)
				addlog(SPP_FMT "%s id mismatch 0x%x != 0x%x\n",
				       SPP_ARGS(sp), cp->name,
				       h->ident, sp->confid[cp->protoidx]);
			//++ifp->if_ierrors;
			break;
		}
		if (h->ident != sp->confid[cp->protoidx]) {	// Jenny, discard mismatch CONF-ACK from server
			if (debug)
				addlog(SPP_FMT "%s id mismatch 0x%x != 0x%x\n",
				       SPP_ARGS(sp), cp->name,
				       h->ident, sp->confid[cp->protoidx]);
			break;
		}
		switch (sp->state[cp->protoidx]) {
		case STATE_CLOSED:
		case STATE_STOPPED:
			sppp_cp_send(sp, cp->proto, TERM_ACK, h->ident, 0, 0);
			break;
		case STATE_CLOSING:
		case STATE_STOPPING:
			break;
		case STATE_REQ_SENT:
#if 0//def CONFIG_PPP_MPPE_MPPC
			if ((&ccp == cp) && (sp->myauth.enc_type == MPPE_TYPE)) {
				/* 
				 * The kernel ppp driver needs the session key 
				 * which is not sent via CCP :( 
				 */
				unsigned int keysize;
				unsigned char opt_buf[64];
				opt_buf[0] = CI_MPPE;
				opt_buf[1] = CILEN_MPPE;
				opt_buf[2] = ((p[2] & MPPE_STATELESS) ? 1 : 0);
				opt_buf[3] = 0;
				opt_buf[4] = 0;
				opt_buf[5] = p[5]&MPPE_MPPC;
				/* push in our send/receive keys */
				if(p[5] & MPPE_40BIT) {
					keysize = 8;
					BCOPY(mppe_master_send_key_40, opt_buf+6, keysize);
					BCOPY(mppe_master_recv_key_40, opt_buf+14, keysize);
				} else if(p[5] & MPPE_128BIT) {
					keysize = 16;
					BCOPY(mppe_master_send_key_128, opt_buf+6, keysize);
					BCOPY(mppe_master_recv_key_128, opt_buf+22, keysize);
				} else {
					printf("unknown ccp format.\n");
				}
				/* call ioctl and pass this nasty stuff to the kernel */
				if (ccp_test(sp->dev_fd, opt_buf, (2*keysize)+3, 0) <= 0){
					printf("%s %d ccp_test fail.\n", __func__, __LINE__);
				}
			}
#endif
			sp->rst_counter[cp->protoidx] = sp->lcp.max_configure;
			sppp_cp_change_state(cp, sp, STATE_ACK_RCVD);
			break;
		case STATE_OPENED:
			(cp->tld)(sp);
			/* fall through */
		case STATE_ACK_RCVD:
			(cp->scr)(sp);
			sppp_cp_change_state(cp, sp, STATE_REQ_SENT);
			break;
		case STATE_ACK_SENT:
/*#ifdef DIAGNOSTIC_TEST
			sppp_diag_log(sp->if_unit, 1);	// Jenny, diagnostic test, PPP server connection pass
			if (sp->pp_phase == PHASE_NETWORK)
				sppp_diag_log(sp->if_unit, 3);	// Jenny, diagnostic test, assign IP address pass
#endif*/
			sp->rst_counter[cp->protoidx] = sp->lcp.max_configure;
			sppp_cp_change_state(cp, sp, STATE_OPENED);
			//if (debug)
			//	addlog(SPP_FMT "%s tlu\n",
			//	       SPP_ARGS(sp), cp->name);
			(cp->tlu)(sp);
			break;
		default:
			printf(SPP_FMT "%s illegal %s in state %s\n",
			       SPP_ARGS(sp), cp->name,
			       sppp_cp_type_name(h->type),
			       sppp_state_name(sp->state[cp->protoidx]));
			//++ifp->if_ierrors;
		}
		break;
	case CONF_NAK:
	case CONF_REJ:
		if (h->ident != sp->confid[cp->protoidx]) {
			if (debug)
				addlog(SPP_FMT "%s id mismatch 0x%x != 0x%x\n",
				       SPP_ARGS(sp), cp->name,
				       h->ident, sp->confid[cp->protoidx]);
			//++ifp->if_ierrors;
			break;
		}
		if (h->type == CONF_NAK)
			(cp->RCN_nak)(sp, h, len);
		else /* CONF_REJ */
			(cp->RCN_rej)(sp, h, len);

#ifdef CONFIG_USER_PPPOMODEM
		/*when booting & adsl-handshaking (some packets may be lost),
		  ppp client & server have strange negotiation.
		  ppp server tries to reject IPCP_OPT_ADDRESS&DNS opt*/
		if( (sp->over==SPPP_PPPOMODEM) &&
		    (cp->proto==PPP_IPCP) &&
		    (h->type==CONF_REJ) &&
		    (!(sp->ipcp.opts&(1<<IPCP_OPT_ADDRESS))) )
		{
			if (debug)
				addlog(SPP_FMT "%s: reject Address (over pppom) => change to closing state\n",
				       SPP_ARGS(sp), cp->name);
			sppp_cp_change_state(cp, sp, STATE_CLOSING);
		}
#endif //CONFIG_USER_PPPOMODEM	

#ifdef CONFIG_SPPPD_STATICIP
		// Jenny, close the connection if COF_NAK/CON_REJ received (based on windows pppoe client)
		if ((cp->proto == PPP_IPCP) && (sp->staticip == 1)) {
			switch (*p) {
			case IPCP_OPT_ADDRESS:
				sppp_cp_change_state(cp, sp, STATE_CLOSING);
				break;
			}
		}
#endif

		switch (sp->state[cp->protoidx]) {
		case STATE_CLOSED:
		case STATE_STOPPED:
			sppp_cp_send(sp, cp->proto, TERM_ACK, h->ident, 0, 0);
			break;
		case STATE_REQ_SENT:
		case STATE_ACK_SENT:
			sp->rst_counter[cp->protoidx] = sp->lcp.max_configure;
			if (1 == sp->role)
				break;
			/*
			 * Slow things down a bit if we think we might be
			 * in loopback. Depend on the timeout to send the
			 * next configuration request.
			 */
			if (sp->pp_loopcnt)
				break;
			(cp->scr)(sp);
			break;
		case STATE_OPENED:
			(cp->tld)(sp);
			/* fall through */
		case STATE_ACK_RCVD:
			sppp_cp_change_state(cp, sp, STATE_REQ_SENT);
			(cp->scr)(sp);
			break;
		case STATE_CLOSING:
		case STATE_STOPPING:
			break;
		default:
			printf(SPP_FMT "%s illegal %s in state %s\n",
			       SPP_ARGS(sp), cp->name,
			       sppp_cp_type_name(h->type),
			       sppp_state_name(sp->state[cp->protoidx]));
			//++ifp->if_ierrors;
		}
		break;

	case TERM_REQ:
		switch (sp->state[cp->protoidx]) {
		case STATE_ACK_RCVD:
		case STATE_ACK_SENT:
			sppp_cp_change_state(cp, sp, STATE_REQ_SENT);
			/* fall through */
		case STATE_CLOSED:
		case STATE_STOPPED:
		case STATE_CLOSING:
		case STATE_STOPPING:
		case STATE_REQ_SENT:
		  sta:
			/* Send Terminate-Ack packet. */
			if (debug)
				addlog(SPP_FMT "%s send terminate-ack\n",
				    SPP_ARGS(sp), cp->name);
			sppp_cp_send(sp, cp->proto, TERM_ACK, h->ident, 0, 0);
			break;
		case STATE_OPENED:
			/* Send Terminate-Ack packet. */
			if (debug)
				addlog(SPP_FMT "%s send terminate-ack\n",
				    SPP_ARGS(sp), cp->name);
		
			sppp_cp_send(sp, cp->proto, TERM_ACK, h->ident, 0, 0);
			if (sp->pp_phase == PHASE_AUTHENTICATE) {	// Jenny, for authentication fail counter
				auth_fail_counter[sp->unit] ++;
				syslog(LOG_INFO, "spppd: %s: Authentication failed\n", sp->if_name);
			}
			else if (sp->pp_phase == PHASE_NETWORK) {
				sppp_last_connection_error(sp->if_unit, ERROR_ISP_DISCONNECT);
				syslog(LOG_INFO, "spppd: %s: ISP disconnected\n", sp->if_name);
			}
			
			(cp->tld)(sp);
			sp->rst_counter[cp->protoidx] = 0;
			sppp_cp_change_state(cp, sp, STATE_STOPPING);
			//goto sta;
			break;
		default:
			printf(SPP_FMT "%s illegal %s in state %s\n",
			       SPP_ARGS(sp), cp->name,
			       sppp_cp_type_name(h->type),
			       sppp_state_name(sp->state[cp->protoidx]));
			//++ifp->if_ierrors;
		}
		break;
	case TERM_ACK:
		switch (sp->state[cp->protoidx]) {
		case STATE_CLOSED:
		case STATE_STOPPED:
		case STATE_REQ_SENT:
		case STATE_ACK_SENT:
			break;
		case STATE_CLOSING:
			sppp_cp_change_state(cp, sp, STATE_CLOSED);
			(cp->tlf)(sp);
			break;
		case STATE_STOPPING:
			sppp_cp_change_state(cp, sp, STATE_STOPPED);
			(cp->tlf)(sp);
			break;
		case STATE_ACK_RCVD:
			sppp_cp_change_state(cp, sp, STATE_REQ_SENT);
			break;
		case STATE_OPENED:
			(cp->tld)(sp);
			(cp->scr)(sp);
			sppp_cp_change_state(cp, sp, STATE_ACK_RCVD);
			break;
		default:
			printf(SPP_FMT "%s illegal %s in state %s\n",
			       SPP_ARGS(sp), cp->name,
			       sppp_cp_type_name(h->type),
			       sppp_state_name(sp->state[cp->protoidx]));
			//++ifp->if_ierrors;
		}
		break;
	case CODE_REJ:
		/* XXX catastrophic rejects (RXJ-) aren't handled yet. */
		addlog(SPP_FMT "%s: ignoring RXJ (%s) for proto 0x%x, "
		    "danger will robinson\n",
		    SPP_ARGS(sp), cp->name,
		    sppp_cp_type_name(h->type), ntohs(*((u_short *)p)));
		switch (sp->state[cp->protoidx]) {
		case STATE_CLOSED:
		case STATE_STOPPED:
		case STATE_REQ_SENT:
		case STATE_ACK_SENT:
		case STATE_CLOSING:
		case STATE_STOPPING:
		case STATE_OPENED:
			break;
		case STATE_ACK_RCVD:
			sppp_cp_change_state(cp, sp, STATE_REQ_SENT);
			break;
		default:
			printf(SPP_FMT "%s illegal %s in state %s\n",
			       SPP_ARGS(sp), cp->name,
			       sppp_cp_type_name(h->type),
			       sppp_state_name(sp->state[cp->protoidx]));
			//++ifp->if_ierrors;
		}
		break;
	case PROTO_REJ:
	    {
		int catastrophic;
		const struct cp *upper;
		int i;
		unsigned short proto;

		catastrophic = 0;
		upper = NULL;
		proto = ntohs(*((unsigned short *)p));
		for (i = 0; i < IDX_COUNT; i++) {
			if (cps[i]->proto == proto) {
				upper = cps[i];
				break;
			}
		}
		if (upper == NULL)
			catastrophic++;

		if (catastrophic || debug)
			addlog(SPP_FMT "%s: RXJ%c (%s) for proto 0x%x (%s/%s)\n",
			    SPP_ARGS(sp), cp->name, catastrophic ? '-' : '+',
			    sppp_cp_type_name(h->type), proto,
			    upper ? upper->name : "unknown",
			    upper ? sppp_state_name(sp->state[upper->protoidx]) : "?");

		/*
		 * if we got RXJ+ against conf-req, the peer does not implement
		 * this particular protocol type.  terminate the protocol.
		 */
		if (upper && !catastrophic) {
			if (sp->state[upper->protoidx] == STATE_REQ_SENT) {
				upper->Close(sp);
				break;
			}
		}

		/* XXX catastrophic rejects (RXJ-) aren't handled yet. */
		switch (sp->state[cp->protoidx]) {
		case STATE_CLOSED:
		case STATE_STOPPED:
		case STATE_REQ_SENT:
		case STATE_ACK_SENT:
		case STATE_CLOSING:
		case STATE_STOPPING:
		case STATE_OPENED:
			break;
		case STATE_ACK_RCVD:
			sppp_cp_change_state(cp, sp, STATE_REQ_SENT);
			break;
		default:
			printf(SPP_FMT "%s illegal %s in state %s\n",
			       SPP_ARGS(sp), cp->name,
			       sppp_cp_type_name(h->type),
			       sppp_state_name(sp->state[cp->protoidx]));
			//++ifp->if_ierrors;
		}
		break;
	    }
	case DISC_REQ:
		if (cp->proto != PPP_LCP)
			goto illegal;
		/* Discard the packet. */
		break;
	case ECHO_REQ:
		if (cp->proto != PPP_LCP)
			goto illegal;
		if (sp->state[cp->protoidx] != STATE_OPENED) {
			if (debug)
				addlog(SPP_FMT "lcp echo req but lcp closed\n",
				       SPP_ARGS(sp));
			//++ifp->if_ierrors;
			break;
		}
		if (len < 8) {
			if (debug)
				addlog(SPP_FMT "invalid lcp echo request "
				       "packet length: %d bytes\n",
				       SPP_ARGS(sp), len);
			break;
		}
		if ((sp->lcp.opts & (1 << LCP_OPT_MAGIC)) &&
		    ntohl (*(long*)(h+1)) == sp->lcp.magic) {
			/* Line loopback mode detected. */
			printf(SPP_FMT "loopback\n", SPP_ARGS(sp));
			sp->pp_loopcnt = MAXALIVECNT * 5;
			//if_down (ifp);
			//sppp_qflush (&sp->pp_cpq);

			/* Shut down the PPP link. */
			/* XXX */
			lcp.Down(sp);
			// Casey
			//lcp.Up(sp);
			break;
		}
		*(long*)(h+1) = htonl (sp->lcp.magic);
		
		//Since receive Server's ECHO_REQUEST, means server alive. 
		//		So reset keep alive counter.
		sp->pp_alivecnt = 0;
		sppp_cp_send (sp, PPP_LCP, ECHO_REPLY, h->ident, len-4, h+1);
		break;
	case ECHO_REPLY:
		if (cp->proto != PPP_LCP)
			goto illegal;
		if (h->ident != sp->lcp.echoid) {
			//++ifp->if_ierrors;
			break;
		}
		if (len < 8) {
			if (debug)
				addlog(SPP_FMT "lcp invalid echo reply "
				       "packet length: %d bytes\n",
				       SPP_ARGS(sp), len);
			break;
		}
		if (!(sp->lcp.opts & (1 << LCP_OPT_MAGIC)) ||
		    ntohl (*(long*)(h+1)) != sp->lcp.magic)
			sp->pp_alivecnt = 0;
		break;
	default:
		/* Unknown packet type -- send Code-Reject packet. */
	  illegal:
		if (debug)
			addlog(SPP_FMT "%s send code-rej for 0x%x\n",
			       SPP_ARGS(sp), cp->name, h->type);
		sppp_cp_send(sp, cp->proto, CODE_REJ,
			     ++sp->pp_seq[cp->protoidx], len, h);
		//++ifp->if_ierrors;
	}
}

#ifdef CONFIG_PPP_MPPE_MPPC
static void ccp_up_event(const struct cp *cp, struct sppp *sp)
{
	if (cp != &ccp)
		return;
	
	if (STATE_STARTING == sp->state[cp->protoidx])
	{
		sp->rst_counter[cp->protoidx] = sp->lcp.max_configure;
		(cp->scr)(sp);
		sppp_cp_change_state(cp, sp, STATE_REQ_SENT);
	}
}
#endif

/*
 * The generic part of all Up/Down/Open/Close/TO event handlers.
 * Basically, the state transition handling in the automaton.
 */
static void
sppp_up_event(const struct cp *cp, struct sppp *sp)
{
	STDDCL;

	if (debug)
		addlog(SPP_FMT "%s: Up, State is %s\n",
		    SPP_ARGS(sp), cp->name,
		    sppp_state_name(sp->state[cp->protoidx]));

	switch (sp->state[cp->protoidx]) {
	case STATE_INITIAL:
		sppp_cp_change_state(cp, sp, STATE_CLOSED);
		break;
	case STATE_STARTING:
#ifdef CONFIG_PPP_MPPE_MPPC
		if (cp == &ccp) {
			//if authentication mode is "AUTO", and auto negotiatiated to MS-CHAP-V2 mode, then send ccp request after receive ccp-req.
			if (sp->myauth.aumode == 0)
				break;
		}
#endif
		sp->rst_counter[cp->protoidx] = sp->lcp.max_configure;
		(cp->scr)(sp);
		sppp_cp_change_state(cp, sp, STATE_REQ_SENT);
		break;
	default:
		printf(SPP_FMT "%s illegal up in state %s\n",
		       SPP_ARGS(sp), cp->name,
		       sppp_state_name(sp->state[cp->protoidx]));
	}
}

static void
sppp_down_event(const struct cp *cp, struct sppp *sp)
{
	STDDCL;

	if (debug)
		addlog(SPP_FMT "%s: Down, State is %s\n",
		    SPP_ARGS(sp), cp->name,
		    sppp_state_name(sp->state[cp->protoidx]));

	switch (sp->state[cp->protoidx]) {
	case STATE_CLOSED:
	case STATE_CLOSING:
		sppp_cp_change_state(cp, sp, STATE_INITIAL);
		break;
	case STATE_STOPPED:
		sppp_cp_change_state(cp, sp, STATE_STARTING);
		(cp->tls)(sp);
		break;
	case STATE_STOPPING:
	case STATE_REQ_SENT:
	case STATE_ACK_RCVD:
	case STATE_ACK_SENT:
		sppp_cp_change_state(cp, sp, STATE_STARTING);
		break;
	case STATE_OPENED:
		(cp->tld)(sp);
		sppp_cp_change_state(cp, sp, STATE_STARTING);
		break;
	default:
		printf(SPP_FMT "%s illegal down in state %s\n",
		       SPP_ARGS(sp), cp->name,
		       sppp_state_name(sp->state[cp->protoidx]));
	}
}


static void
sppp_open_event(const struct cp *cp, struct sppp *sp)
{
	STDDCL;

	if (debug)
		addlog(SPP_FMT "%s: Up, State is %s\n",
		    SPP_ARGS(sp), cp->name,
		    sppp_state_name(sp->state[cp->protoidx]));
	switch (sp->state[cp->protoidx]) {
	case STATE_INITIAL:
		sppp_cp_change_state(cp, sp, STATE_STARTING);
		(cp->tls)(sp);
		break;
	case STATE_STARTING:
		break;
	case STATE_CLOSED:
		sp->rst_counter[cp->protoidx] = sp->lcp.max_configure;
		(cp->scr)(sp);
		sppp_cp_change_state(cp, sp, STATE_REQ_SENT);
		break;
	case STATE_STOPPED:
		/*
		 * Try escaping stopped state.  This seems to bite
		 * people occasionally, in particular for IPCP,
		 * presumably following previous IPCP negotiation
		 * aborts.  Somehow, we must have missed a Down event
		 * which would have caused a transition into starting
		 * state, so as a bandaid we force the Down event now.
		 * This effectively implements (something like the)
		 * `restart' option mentioned in the state transition
		 * table of RFC 1661.
		 */
		sppp_cp_change_state(cp, sp, STATE_STARTING);
		(cp->tls)(sp);
		break;
	case STATE_STOPPING:
	case STATE_REQ_SENT:
	case STATE_ACK_RCVD:
	case STATE_ACK_SENT:
	case STATE_OPENED:
		break;
	case STATE_CLOSING:
		sppp_cp_change_state(cp, sp, STATE_STOPPING);
		break;
	}

}


static void
sppp_close_event(const struct cp *cp, struct sppp *sp)
{
	STDDCL;

	if (debug)
		addlog(SPP_FMT "%s: Close, State is %s\n",
		    SPP_ARGS(sp), cp->name,
		    sppp_state_name(sp->state[cp->protoidx]));

	switch (sp->state[cp->protoidx]) {
	case STATE_INITIAL:
	case STATE_CLOSED:
	case STATE_CLOSING:
		break;
	case STATE_STARTING:
		sppp_cp_change_state(cp, sp, STATE_INITIAL);
		(cp->tlf)(sp);
		break;
	case STATE_STOPPED:
		sppp_cp_change_state(cp, sp, STATE_CLOSED);
		break;
	case STATE_STOPPING:
		sppp_cp_change_state(cp, sp, STATE_CLOSING);
		break;
	case STATE_OPENED:
		(cp->tld)(sp);
		/* fall through */
	case STATE_REQ_SENT:
	case STATE_ACK_RCVD:
	case STATE_ACK_SENT:
		sp->rst_counter[cp->protoidx] = sp->lcp.max_terminate;
		sppp_cp_send(sp, cp->proto, TERM_REQ,
			     ++sp->pp_seq[cp->protoidx], 0, 0);
		sppp_cp_change_state(cp, sp, STATE_CLOSING);
		break;
	}
}

static void
sppp_to_event(const struct cp *cp, struct sppp *sp)
{
	STDDCL;
	//int s;

	//s = splimp();
	if (debug)
		addlog(SPP_FMT "%s: TO, State is %s. rst_counter = %d\n",
		    SPP_ARGS(sp), cp->name,
		    sppp_state_name(sp->state[cp->protoidx]),
		    sp->rst_counter[cp->protoidx]);

	if (--sp->rst_counter[cp->protoidx] < 0)
		/* TO- event */
		switch (sp->state[cp->protoidx]) {
		case STATE_CLOSING:
			sppp_cp_change_state(cp, sp, STATE_CLOSED);
			(cp->tlf)(sp);
			break;
		case STATE_STOPPING:
			sppp_cp_change_state(cp, sp, STATE_STOPPED);
			(cp->tlf)(sp);
			break;
		case STATE_REQ_SENT:
		case STATE_ACK_RCVD:
		case STATE_ACK_SENT:
			sppp_cp_change_state(cp, sp, STATE_STOPPED);
			(cp->tlf)(sp);
			break;
		}
	else
		/* TO+ event */
		switch (sp->state[cp->protoidx]) {
		case STATE_CLOSING:
		case STATE_STOPPING:
			sppp_cp_send(sp, cp->proto, TERM_REQ,
				     ++sp->pp_seq[cp->protoidx], 0, 0);
			TIMEOUT(cp->TO, (void *)sp, sp->lcp.timeout,
			    sp->ch[cp->protoidx]);
			break;
		case STATE_REQ_SENT:
		case STATE_ACK_RCVD:
			(cp->scr)(sp);
			/* sppp_cp_change_state() will restart the timer */
			sppp_cp_change_state(cp, sp, STATE_REQ_SENT);
			break;
		case STATE_ACK_SENT:
			// Casey 20040329
			(cp->scr)(sp);
			TIMEOUT(cp->TO, (void *)sp, sp->lcp.timeout,
			    sp->ch[cp->protoidx]);
			break;
		}

	//splx(s);
}

/*
 * Change the state of a control protocol in the state automaton.
 * Takes care of starting/stopping the restart timer.
 */
void
sppp_cp_change_state(const struct cp *cp, struct sppp *sp, int newstate)
{
	sp->state[cp->protoidx] = newstate;
	UNTIMEOUT(cp->TO, (void *)sp, sp->ch[cp->protoidx]);
	switch (newstate) {
	case STATE_INITIAL:
	case STATE_STARTING:
	case STATE_CLOSED:
	case STATE_STOPPED:
	case STATE_OPENED:
		break;
	//case STATE_CLOSING:
	//case STATE_STOPPING:
	case STATE_REQ_SENT:
	case STATE_ACK_RCVD:
	case STATE_ACK_SENT:
		TIMEOUT(cp->TO, (void *)sp, sp->lcp.timeout,
		    sp->ch[cp->protoidx]);
		break;
	case STATE_CLOSING:
	case STATE_STOPPING:
		cp->TO(sp);
		break;
	}
}

/*
 *--------------------------------------------------------------------------*
 *                                                                          *
 *                         The LCP implementation.                          *
 *                                                                          *
 *--------------------------------------------------------------------------*
 */
static void
sppp_lcp_init(struct sppp *sp)
{
	sp->lcp.opts = (1 << LCP_OPT_MAGIC) | (1<< LCP_OPT_MRU);
	sp->lcp.magic = 0;
	sp->state[IDX_LCP] = STATE_INITIAL;
	sp->fail_counter[IDX_LCP] = 0;
	sp->pp_seq[IDX_LCP] = 0;
	sp->pp_rseq[IDX_LCP] = 0;
	sp->lcp.protos = 0;
//	sp->lcp.mru = sp->lcp.their_mru = PP_MTU;
	sp->lcp.their_mru = PP_MTU;

	/* Note that these values are  relevant for all control protocols */
	//sp->lcp.timeout = 3 * hz;
	sp->lcp.timeout = 3 * 1000 * hz;
	sp->lcp.max_terminate = 2;
	#ifndef CONFIG_USER_PPPOE_CTC_DELAY_INT
	sp->lcp.max_configure = 10;
	#endif
	//sp->lcp.max_failure = 10;
	sp->lcp.max_failure = 5;
	sppp_callout_handle_init(&sp->ch[IDX_LCP]);
#ifdef _CWMP_MIB_
	sppp_lcp_echo_log();
#endif
}

static void
sppp_lcp_up(struct sppp *sp)
{
	STDDCL;

	sp->pp_alivecnt = 0;
	sp->lcp.opts = (1 << LCP_OPT_MAGIC);
	if (1 == sp->role)
	{
		sp->lcp.opts |= (1<<LCP_OPT_MRU);
		// Mason Yu. start.  Don't request  (1) protocol field compression (2) address/control field compression 
		//if(sp->over != SPPP_PPPOE)
		//	sp->lcp.opts |= (1<<LCP_OPT_PROTO_COMP) | (1<<LCP_OPT_ADDR_COMP);
		// Mason Yu. End
		
		//memcpy(&sp->hisauth, &sp->myauth, sizeof(sp->myauth));
		//if (0 == sp->hisauth.proto) {
		//	sp->hisauth.proto = PPP_CHAP;
		//	sp->hisauth.chal_type = CHAP_MICROSOFT_V2;
		//}
		if (sp->myauth.proto != 0) {
			sp->hisauth = sp->myauth;
			sp->lcp.opts |= (1<<LCP_OPT_AUTH_PROTO);
		}
	}
#ifdef CONFIG_PPP_MPPE_MPPC
	//QL to improve performance.
	//if (MPPE_TYPE == sp->myauth.enc_type)
	//	sp->lcp.opts |= (1<<LCP_OPT_PROTO_COMP) | (1<<LCP_OPT_ADDR_COMP);
#endif
	sp->lcp.magic = 0;
	sp->lcp.protos = 0;
//	sp->lcp.mru = sp->lcp.their_mru = PP_MTU;
	sp->lcp.their_mru = PP_MTU;
#ifdef CONFIG_USER_PPPOMODEM
	/* lcp option, accm*/
	if(sp->over==SPPP_PPPOMODEM){
		//printf( "\nsppp_lcp_up: set ayncmap\n" );
		sp->lcp.opts |= (1 << LCP_OPT_ASYNC_MAP);
		ppp_send_accm_config( sp, DEF_ACCM_VALUE);
		ppp_recv_accm_config( sp, DEF_ACCM_VALUE);
		sp->lcp.neg_xaccm=0;//default 0. With accm option from peer, set to 1
		sp->lcp.xaccm=0;
		sp->lcp.neg_raccm=1;//default 1. When reject,  set to 0
		sp->lcp.raccm=0;
	}
#endif //CONFIG_USER_PPPOMODEM

#ifdef CONFIG_PPP_MPPE_MPPC
	if (sp->over == SPPP_PPTP) {
		if (sp->lcp.opts & ((1<<LCP_OPT_PROTO_COMP) | (1<<LCP_OPT_ADDR_COMP))) {
			int flags;
			if (ioctl(sp->fd, PPPIOCGFLAGS, (caddr_t) &flags) < 0) {
				printf("getflags fail.\n");
			}
			flags = (sp->lcp.opts & (1<<LCP_OPT_PROTO_COMP))?(flags | SC_COMP_PROT):(flags & ~SC_COMP_PROT);
			flags = (sp->lcp.opts & (1<<LCP_OPT_ADDR_COMP))?(flags | SC_COMP_AC):(flags & ~SC_COMP_AC);
			if (ioctl(sp->fd, PPPIOCSFLAGS, (caddr_t) &flags) < 0) {
				printf("setflags fail.\n");
			}

			//test
			ioctl(sp->fd, PPPIOCGFLAGS, (caddr_t) &flags);
			//printf("ppp_flags:0x%x\n", flags);
		}
	}
#endif

	/*
	 * If this interface is passive or dial-on-demand, and we are
	 * still in Initial state, it means we've got an incoming
	 * call.  Activate the interface.
	 */
	if ((sp->pp_flags & (IFF_AUTO | IFF_PASSIVE)) != 0) {
		if (debug)
			addlog(SPP_FMT "Up event", SPP_ARGS(sp));
		sp->pp_flags |= IFF_RUNNING;
		if (sp->state[IDX_LCP] == STATE_INITIAL) {
			if (debug)
				addlog("(incoming call)\n");
			sp->pp_flags |= PP_CALLIN;
			lcp.Open(sp);
		} else if (debug)
			addlog("\n");
	} else if ((sp->pp_flags & (IFF_AUTO | IFF_PASSIVE)) == 0 &&
		   (sp->state[IDX_LCP] == STATE_INITIAL)) {
		sp->pp_flags |= IFF_RUNNING;
		lcp.Open(sp);
	}

	sppp_up_event(&lcp, sp);
	
}

static void
sppp_lcp_down(struct sppp *sp)
{
	STDDCL;

	sppp_down_event(&lcp, sp);

	/*
	 * If this is neither a dial-on-demand nor a passive
	 * interface, simulate an ``ifconfig down'' action, so the
	 * administrator can force a redial by another ``ifconfig
	 * up''.  XXX For leased line operation, should we immediately
	 * try to reopen the connection here?
	 */
	if ((sp->pp_flags & (IFF_AUTO | IFF_PASSIVE)) == 0) {
		addlog(SPP_FMT "PPP: Down event, taking interface down.\n",
		    SPP_ARGS(sp));
		//if_down(ifp);
	} else {
		if (debug)
			addlog(SPP_FMT "PPP: Down event (carrier loss)\n",
			    SPP_ARGS(sp));
		sp->pp_flags &= ~PP_CALLIN;
		if (sp->state[IDX_LCP] != STATE_INITIAL)
			lcp.Close(sp);
		sp->pp_flags &= ~IFF_RUNNING;
	}

	
	/* Notify lower layer if desired. */
	//if (sp->pp_tlf)
	//	(sp->pp_tlf)(sp);

}

static void
sppp_lcp_open(struct sppp *sp)
{
	
	/*
	 * If we are authenticator, negotiate LCP_AUTH
	 */
	if (sp->hisauth.proto != 0)
		sp->lcp.opts |= (1 << LCP_OPT_AUTH_PROTO);
	else
		sp->lcp.opts &= ~(1 << LCP_OPT_AUTH_PROTO);
	sp->pp_flags &= ~PP_NEEDAUTH;
	sppp_open_event(&lcp, sp);
}

static void
sppp_lcp_close(struct sppp *sp)
{
	sppp_close_event(&lcp, sp);
}

static void
sppp_lcp_TO(void *cookie)
{
	sppp_to_event(&lcp, (struct sppp *)cookie);
}

/*
 * Analyze a configure request.  Return true if it was agreeable, and
 * caused action sca, false if it has been rejected or nak'ed, and
 * caused action scn.  (The return value is used to make the state
 * transition decision in the state automaton.)
 */
static int
sppp_lcp_RCR(struct sppp *sp, struct lcp_header *h, int len)
{
	STDDCL;
	u_char *buf, *r, *p;
	int origlen, rlen;
	u_long nmagic;
	u_short authproto;
	char strbuff[256];

	len -= 4;
	origlen = len;
	buf = r = malloc(len);
	if (! buf)
		return (0);

	strbuff[0] = 0;
	if (debug)
		sprintf(strbuff, "%s LCP:   parse opts: ", SPP_ARGS(sp));
		//addlog(SPP_FMT "LCP:   parse opts: ",
		//    SPP_ARGS(sp));
	//printf("sppp_lcp_RCR %s\n", SPP_ARGS(sp));
	/* pass 1: check for things that need to be rejected */
	p = (void*) (h+1);
	for (rlen=0; len>1 && p[1]; len-=p[1], p+=p[1]) {
		if (debug)
			sprintf(strbuff, "%s %s ", strbuff, sppp_lcp_opt_name(*p));
			//addlog(" %s ", sppp_lcp_opt_name(*p));
		switch (*p) {
		case LCP_OPT_MAGIC:
			/* Magic number. */
			if (len >= 6 && p[1] == 6)
				continue;
			if (debug)
				sprintf(strbuff, "%s[invalid] ", strbuff);
				//addlog("[invalid] ");
			break;
		case LCP_OPT_ASYNC_MAP:
			/* Async control character map. */
			if (len >= 6 && p[1] == 6)
				continue;
			if (debug)
				sprintf(strbuff, "%s[invalid] ", strbuff);
				//addlog("[invalid] ");
			break;
		case LCP_OPT_MRU:
			/* Maximum receive unit. */
			if (len >= 4 && p[1] == 4)
				continue;
			if (debug)
				sprintf(strbuff, "%s[invalid] ", strbuff);
				//addlog("[invalid] ");
			break;
		case LCP_OPT_AUTH_PROTO:
#ifdef CONFIG_USER_PPPOMODEM
			//SPPP_PPPOMODEM may not need to auth.
			if( (sp->over==SPPP_PPPOMODEM) && (sp->myauth.name[0]==0) )
			{
				if (debug)
					printf( "%s:%d: over=SPPP_PPPOMODEM and no username, skip auth, reject LCP_OPT_AUTH_PROTO\n", __FUNCTION__, __LINE__ );
				if (debug)
					sprintf(strbuff, "%s[rej] ", strbuff);

				sp->pp_flags &= ~PP_NEEDAUTH;
				break;
			}
#endif //CONFIG_USER_PPPOMODEM
			if (len < 4) {
				if (debug)
					sprintf(strbuff, "%s[invalid] ", strbuff);
					//addlog("[invalid] ");
				break;
			}
			authproto = (p[2] << 8) + p[3];
			if (authproto == PPP_CHAP && p[1] != 5) {
				if (debug)
					sprintf(strbuff, "%s[invalid chap len] ", strbuff);
					//addlog("[invalid chap len] ");
				break;
			}
			/*
			 * Remote want us to authenticate, remember this,
			 * so we stay in PHASE_AUTHENTICATE after LCP got
			 * up.
			 */
			sp->pp_flags |= PP_NEEDAUTH;
			continue;
#ifdef CONFIG_PPP_MPPE_MPPC
		case LCP_OPT_ADDR_COMP:
			if (sp->lcp.opts & (1<<LCP_OPT_ADDR_COMP)) {
				if (len >= 2 && p[1] == 2) {
					sp->pp_flags |= PP_ADDRCOMP;
					continue;
				}
			}
			break;
		case LCP_OPT_PROTO_COMP:
			if (sp->lcp.opts & (1<<LCP_OPT_PROTO_COMP)) {
				if (len >= 2 && p[1] == 2)
					continue;
			}
			break;
#endif//endof CONFIG_PPP_MPPE_MPPC
		default:
			/* Others not supported. */
			if (debug)
				sprintf(strbuff, "%s[rej] ", strbuff);
				//addlog("[rej] ");
			break;
		}
		/* Add the option to rejected list. */
		bcopy (p, r, p[1]);
		r += p[1];
		rlen += p[1];
	}
	if (rlen) {
		if (debug)
			addlog("%s send conf-rej\n", strbuff);
			//addlog(" send conf-rej\n");
		sppp_cp_send (sp, PPP_LCP, CONF_REJ, h->ident, rlen, buf);
		return 0;
	} else if (debug)
		addlog("%s\n", strbuff);
		//addlog("\n");

	/*
	 * pass 2: check for option values that are unacceptable and
	 * thus require to be nak'ed.
	 */
	if (debug)
		sprintf(strbuff, "%s LCP:   parse opt values: ", SPP_ARGS(sp));
		//addlog(SPP_FMT "LCP:   parse opt values: ",
		//       SPP_ARGS(sp));

	p = (void*) (h+1);
	len = origlen;
	for (rlen=0; len>1 && p[1]; len-=p[1], p+=p[1]) {
		if (debug)
			sprintf(strbuff, "%s%s ", strbuff, sppp_lcp_opt_name(*p));
			//addlog("%s ", sppp_lcp_opt_name(*p));
		switch (*p) {
		case LCP_OPT_MAGIC:
			/* Magic number -- extract. */
			nmagic = (u_long)p[2] << 24 |
				(u_long)p[3] << 16 | p[4] << 8 | p[5];
			if (nmagic != sp->lcp.magic) {
				sp->pp_loopcnt = 0;
				if (debug)
					sprintf(strbuff, "%s0x%lx ", strbuff, nmagic);
					//addlog("0x%lx ", nmagic);
				continue;
			}
			if (debug && sp->pp_loopcnt < MAXALIVECNT*5)
				sprintf(strbuff, "%s[glitch] ", strbuff);
				//addlog("[glitch] ");
			++sp->pp_loopcnt;
			/*
			 * We negate our magic here, and NAK it.  If
			 * we see it later in an NAK packet, we
			 * suggest a new one.
			 */
			nmagic = ~sp->lcp.magic;
			/* Gonna NAK it. */
			p[2] = nmagic >> 24;
			p[3] = nmagic >> 16;
			p[4] = nmagic >> 8;
			p[5] = nmagic;
			break;

		case LCP_OPT_ASYNC_MAP:
			/*
			 * Async control character map -- just ignore it.
			 *
			 * Quote from RFC 1662, chapter 6:
			 * To enable this functionality, synchronous PPP
			 * implementations MUST always respond to the
			 * Async-Control-Character-Map Configuration
			 * Option with the LCP Configure-Ack.  However,
			 * acceptance of the Configuration Option does
			 * not imply that the synchronous implementation
			 * will do any ACCM mapping.  Instead, all such
			 * octet mapping will be performed by the
			 * asynchronous-to-synchronous converter.
			 */
#ifdef CONFIG_USER_PPPOMODEM
			/* lcp option, accm*/
			if(sp->over==SPPP_PPPOMODEM){
				sp->lcp.neg_xaccm=1;
				sp->lcp.xaccm = (u_long)p[2] << 24 |
								(u_long)p[3] << 16 | p[4] << 8 | p[5];
				if (debug)
					sprintf(strbuff, "%s0x%lx ", strbuff, sp->lcp.xaccm);
			}
#endif //CONFIG_USER_PPPOMODEM
			continue;

		case LCP_OPT_MRU:
			/*
			 * Maximum receive unit.  Always agreeable,
			 * but ignored by now.
			 */
			sp->lcp.their_mru = p[2] * 256 + p[3];
			if (debug)
				sprintf(strbuff, "%s%lu ", strbuff, sp->lcp.their_mru);
				//addlog("%lu ", sp->lcp.their_mru);
			continue;

		case LCP_OPT_AUTH_PROTO:
			authproto = (p[2] << 8) + p[3];
			sprintf(strbuff, "%s[mine %s auth %s ] his %s ", strbuff, 
				sppp_proto_name(sp->myauth.proto), sppp_proto_name(authproto), sppp_proto_name(sp->hisauth.proto));
			if (sp->myauth.proto == authproto && authproto == PPP_CHAP) {
				if (sp->myauth.chal_type == p[4]) {
					continue;
				}
				else {
					if (sp->myauth.chal_type != 0) {
						if (debug)
							sprintf(strbuff, "%s[chap algorithm(0x%x) not acceptable, acquired 0x%x] ", strbuff, p[4], sp->myauth.chal_type);
						p[4] = sp->myauth.chal_type;
						break;
					}
					else {
						if (p[4]==CHAP_MICROSOFT || p[4]==CHAP_MICROSOFT_V2 || p[4]==CHAP_MD5) {
							sp->myauth.chal_type = p[4];
							continue;
						}
						else {
							p[4] = CHAP_MD5;
							break;
						}
					}
				}
			}
			
			//printf("%s\n", strbuff);
			if ((authproto!=PPP_PAP&&authproto!=PPP_CHAP) ||
				(sp->myauth.proto!=0x0&&sp->myauth.proto!=authproto)){
				/* not agreed, nak */
				if (debug)
					sprintf(strbuff, "%s[mine %s != auth %s ] his %s ", strbuff, 
						sppp_proto_name(sp->myauth.proto), sppp_proto_name(authproto), sppp_proto_name(sp->hisauth.proto));
					//addlog("[mine %s != his %s] ",
						//sppp_proto_name(sp->hisauth.proto),
					/*addlog("[mine %s != auth %s ] his %s ",
						sppp_proto_name(sp->myauth.proto),
						sppp_proto_name(authproto), sppp_proto_name(sp->hisauth.proto));*/
					       //sppp_proto_name(authproto));
				p[2] = sp->myauth.proto >> 8;
				p[3] = sp->myauth.proto;
				break;
			}
			if (sp->myauth.proto == 0x0) {
				if (authproto == PPP_CHAP) {
					if (p[4] == CHAP_MICROSOFT) {
						sp->myauth.proto = authproto;
						sp->myauth.chal_type = CHAP_MICROSOFT;
						continue;
					}
					if (p[4] == CHAP_MICROSOFT_V2) {
						sp->myauth.proto = authproto;
						sp->myauth.chal_type = CHAP_MICROSOFT_V2;
						continue;
					}
				}
			}
			if (authproto == PPP_CHAP && p[4] != CHAP_MD5) {
				if (debug)
					sprintf(strbuff, "%s[chap not MD5] ", strbuff);
					//addlog("[chap not MD5] ");
				p[4] = CHAP_MD5;
				break;
			}
			if (sp->myauth.proto != authproto) {
				const struct cp *cp = &lcp;
				//kill the timer for the previous auth proto
				if (sp->myauth.proto == PPP_CHAP)
					UNTIMEOUT(sppp_chap_my_TO, (void *)sp, sp->chap_my_to_ch);
				if (sp->myauth.proto == PPP_PAP)
					UNTIMEOUT(sppp_pap_my_TO, (void *)sp, sp->pap_my_to_ch);
		//		sppp_cp_change_state(cp, sp, STATE_REQ_SENT); //in ppp state machine, should not change state here.

				/* follow the peer auth */
				sp->myauth.proto = authproto;
			}
			if (debug)
				sprintf(strbuff, "%s%s ", strbuff, sppp_proto_name(authproto));
				//addlog("%s ", sppp_proto_name(authproto));
			continue;
#ifdef CONFIG_PPP_MPPE_MPPC
		case LCP_OPT_ADDR_COMP:
		case LCP_OPT_PROTO_COMP:
			continue;
#endif
		}
		/* Add the option to nak'ed list. */
		bcopy (p, r, p[1]);
		r += p[1];
		rlen += p[1];
	}
	if (rlen) {
		/*
		 * Local and remote magics equal -- loopback?
		 */
		if (sp->pp_loopcnt >= MAXALIVECNT*5) {
			if (sp->pp_loopcnt == MAXALIVECNT*5)
				printf (SPP_FMT "loopback\n",
					SPP_ARGS(sp));
			if (sp->pp_flags & IFF_UP) {
				//if_down(ifp);
				//sppp_qflush(&sp->pp_cpq);
				/* XXX ? */
				lcp.Down(sp);
				// Casey
				//lcp.Up(sp);
			}
		} else if (++sp->fail_counter[IDX_LCP] >= sp->lcp.max_failure) {
			if (debug)
				addlog("%s max_failure (%d) exceeded, send conf-rej\n", strbuff, sp->lcp.max_failure);
				/*addlog(" max_failure (%d) exceeded, "
				       "send conf-rej\n",
				       sp->lcp.max_failure);*/
			sppp_cp_send(sp, PPP_LCP, CONF_REJ, h->ident, rlen, buf);
		} else {
			if (debug)
				addlog("%s send conf-nak\n", strbuff);
				//addlog(" send conf-nak\n");
			sppp_cp_send (sp, PPP_LCP, CONF_NAK, h->ident, rlen, buf);
		}
	} else {
		if (debug)
			addlog("%s send conf-ack\n", strbuff);
			//addlog(" send conf-ack\n");
		sp->fail_counter[IDX_LCP] = 0;
		sp->pp_loopcnt = 0;
		sppp_cp_send (sp, PPP_LCP, CONF_ACK,
			      h->ident, origlen, h+1);
	}

	free (buf);
	return (rlen == 0);
}

/*
 * Analyze the LCP Configure-Reject option list, and adjust our
 * negotiation.
 */
static void
sppp_lcp_RCN_rej(struct sppp *sp, struct lcp_header *h, int len)
{
	STDDCL;
	u_char *buf, *p;
	char strbuff[256];

	len -= 4;
	buf = malloc(len);
	if (!buf)
		return;

	strbuff[0] = 0;
	if (debug)
		sprintf(strbuff, "%slcp rej opts: ", SPP_ARGS(sp));
		//addlog(SPP_FMT "lcp rej opts: ",
		//    SPP_ARGS(sp));

	p = (void*) (h+1);
	for (; len > 1 && p[1]; len -= p[1], p += p[1]) {
		if (debug)
			sprintf(strbuff, "%s %s ", strbuff, sppp_lcp_opt_name(*p));
			//addlog(" %s ", sppp_lcp_opt_name(*p));
		switch (*p) {
		case LCP_OPT_MAGIC:
			/* Magic number -- can't use it, use 0 */
			sp->lcp.opts &= ~(1 << LCP_OPT_MAGIC);
			sp->lcp.magic = 0;
			break;
		case LCP_OPT_MRU:
			/*
			 * Should not be rejected anyway, since we only
			 * negotiate a MRU if explicitly requested by
			 * peer.
			 */
			sp->lcp.opts &= ~(1 << LCP_OPT_MRU);
			break;
		case LCP_OPT_AUTH_PROTO:
			/*
			 * Peer doesn't want to authenticate himself,
			 * deny unless this is a dialout call, and
			 * AUTHFLAG_NOCALLOUT is set.
			 */
			if ((sp->pp_flags & PP_CALLIN) == 0 &&
			    (sp->hisauth.flags & AUTHFLAG_NOCALLOUT) != 0) {
				if (debug)
					sprintf(strbuff, "%s[don't insist on auth for callout]", strbuff);
					//addlog("[don't insist on auth "
					//       "for callout]");
				sp->lcp.opts &= ~(1 << LCP_OPT_AUTH_PROTO);
				break;
			}
			if (debug)
				addlog("%s[access denied]\n", strbuff);
				//addlog("[access denied]\n");
			lcp.Close(sp);
			break;
#ifdef CONFIG_USER_PPPOMODEM
		case LCP_OPT_ASYNC_MAP:
			/* lcp option, accm*/
			if(sp->over==SPPP_PPPOMODEM){
				sp->lcp.opts &= ~(1 << LCP_OPT_ASYNC_MAP);
				sp->lcp.neg_raccm=0;
			}
			break;
#endif //CONFIG_USER_PPPOMODEM
		case LCP_OPT_PROTO_COMP:
			if (1 == sp->role)
				sp->lcp.opts &= ~(1<<LCP_OPT_PROTO_COMP);
			break;
		case LCP_OPT_ADDR_COMP:
			if (1 == sp->role)
				sp->lcp.opts &= ~(1<<LCP_OPT_ADDR_COMP);
			break;
		}
	}
	if (debug)
		addlog("%s\n", strbuff);
		//addlog("\n");
	free (buf);
	return;
}

/*
 * Analyze the LCP Configure-NAK option list, and adjust our
 * negotiation.
 */
static void
sppp_lcp_RCN_nak(struct sppp *sp, struct lcp_header *h, int len)
{
	STDDCL;
	u_char *buf, *p;
	u_long magic;
	u_short authproto;
	char strbuff[256];

	len -= 4;
	buf = malloc(len);
	if (!buf)
		return;

	strbuff[0] = 0;
	if (debug)
		sprintf(strbuff, "%slcp nak opts: ", SPP_ARGS(sp));
		//addlog(SPP_FMT "lcp nak opts: ",
		//    SPP_ARGS(sp));

	p = (void*) (h+1);
	for (; len > 1 && p[1]; len -= p[1], p += p[1]) {
		if (debug)
			sprintf(strbuff, "%s %s ", strbuff, sppp_lcp_opt_name(*p));
			//addlog(" %s ", sppp_lcp_opt_name(*p));
		switch (*p) {
		case LCP_OPT_MAGIC:
			/* Magic number -- renegotiate */
			if ((sp->lcp.opts & (1 << LCP_OPT_MAGIC)) &&
			    len >= 6 && p[1] == 6) {
				magic = (u_long)p[2] << 24 |
					(u_long)p[3] << 16 | p[4] << 8 | p[5];
				/*
				 * If the remote magic is our negated one,
				 * this looks like a loopback problem.
				 * Suggest a new magic to make sure.
				 */
				if (magic == ~sp->lcp.magic) {
					if (debug)
						sprintf(strbuff, "%smagic glitch ", strbuff);
						//addlog("magic glitch ");
					sp->lcp.magic = rand();
				} else {
					sp->lcp.magic = magic;
					if (debug)
						sprintf(strbuff, "%s%lu ", strbuff, magic);
						//addlog("%lu ", magic);
				}
			}
			break;
		case LCP_OPT_MRU:
			/*
			 * Peer wants to advise us to negotiate an MRU.
			 * Agree on it if it's reasonable, or use
			 * default otherwise.
			 */
			if (len >= 4 && p[1] == 4) {
				u_int mru = p[2] * 256 + p[3];
				if (debug)
					sprintf(strbuff, "%s%d ", strbuff, mru);
					//addlog("%d ", mru);
				if (mru < PP_MTU || mru > PP_MAX_MRU)
					mru = PP_MTU;
				sp->lcp.mru = mru;
				sp->lcp.opts |= (1 << LCP_OPT_MRU);
			}
			break;
		case LCP_OPT_AUTH_PROTO:
			if (1 == sp->role) {
				authproto = (p[2] << 8) + p[3];

				sprintf(strbuff, "%s[mine %s auth %s ] his %s ", strbuff, 
					sppp_proto_name(sp->myauth.proto), sppp_proto_name(authproto), sppp_proto_name(sp->hisauth.proto));
				printf("%s\n", strbuff);

				if (sp->myauth.proto == 0x0) {/* follow the peer auth */
					if ((authproto == PPP_CHAP) && p[4] == CHAP_MICROSOFT_V2)
					{
						sp->myauth.proto = authproto;
						sp->myauth.chal_type = CHAP_MICROSOFT_V2;
						continue;
					}
					else
						sp->myauth.proto = authproto;
				}
				sp->hisauth = sp->myauth;
			}

			/*
			 * Peer doesn't like our authentication method,
			 * deny.
			 */
			if (debug)
				addlog("%s[access denied]\n", strbuff);
				//addlog("[access denied]\n");
			lcp.Close(sp);
			break;
#ifdef CONFIG_USER_PPPOMODEM
		case LCP_OPT_ASYNC_MAP:
			if (len >= 6 && p[1] == 6) {
				/* lcp option, accm*/
				if(sp->over==SPPP_PPPOMODEM){
					u_long raccm;
					raccm =	(u_long)p[2] << 24 |
							(u_long)p[3] << 16 | p[4] << 8 | p[5];
					sp->lcp.opts |= (1 << LCP_OPT_ASYNC_MAP);
					sp->lcp.neg_raccm=1;
					sp->lcp.raccm |=raccm;//default value OR the value from peer
				}
			}
			break;
#endif //CONFIG_USER_PPPOMODEM
		}
	}
	if (debug)
		addlog("%s\n", strbuff);
		//addlog("\n");
	free (buf);
	return;
}

static void
sppp_lcp_tlu(struct sppp *sp)
{
	STDDCL;
	int i;
	u_long mask;

	
#ifdef CONFIG_PPP_MPPE_MPPC
	if (1 == sp->role) {
		if (sp->lcp.opts & ((1<<LCP_OPT_PROTO_COMP) | (1<<LCP_OPT_ADDR_COMP))) {
			int flags;
			if (ioctl(sp->fd, PPPIOCGFLAGS, (caddr_t) &flags) < 0) {
				printf("getflags fail.\n");
			}
			flags = (sp->lcp.opts & (1<<LCP_OPT_PROTO_COMP))?(flags | SC_COMP_PROT):(flags & ~SC_COMP_PROT);
			flags = (sp->lcp.opts & (1<<LCP_OPT_ADDR_COMP))?(flags | SC_COMP_AC):(flags & ~SC_COMP_AC);
			if (ioctl(sp->fd, PPPIOCSFLAGS, (caddr_t) &flags) < 0) {
				printf("setflags fail.\n");
			}

			//test
			ioctl(sp->fd, PPPIOCGFLAGS, (caddr_t) &flags);
			//printf("ppp_flags:0x%x\n", flags);
		}
	}
#endif

#ifdef CONFIG_USER_PPPOMODEM
	/* lcp option, accm*/
	if(sp->over==SPPP_PPPOMODEM){
		//printf( "\nsppp_lcp_tlu: set ayncmap\n" );
		ppp_send_accm_config( sp, sp->lcp.neg_xaccm?sp->lcp.xaccm:DEF_ACCM_VALUE);
		ppp_recv_accm_config( sp, sp->lcp.neg_raccm?sp->lcp.raccm:DEF_ACCM_VALUE);
	}
#endif //CONFIG_USER_PPPOMODEM

	/* XXX ? */
	if (! (sp->pp_flags & IFF_UP) &&
	    (sp->pp_flags & IFF_RUNNING)) {
		/* Coming out of loopback mode. */
		//if_up(ifp);
		//printf (SPP_FMT "up\n", SPP_ARGS(sp));
	}

	for (i = 0; i < IDX_COUNT; i++)
		if ((cps[i])->flags & CP_QUAL)
			(cps[i])->Open(sp);

	if ((sp->lcp.opts & (1 << LCP_OPT_AUTH_PROTO)) != 0 ||
	    (sp->pp_flags & PP_NEEDAUTH) != 0) {
		sp->pp_phase = PHASE_AUTHENTICATE;
		sppp_auth_log(sp->if_unit, 3);	// Jenny
		//sppp_last_connection_error(sp->if_unit, ERROR_AUTHENTICATION_FAILURE);
	}
	else
		sp->pp_phase = PHASE_NETWORK;

	if (debug)
		addlog(SPP_FMT "PPP: Phase is %s\n", SPP_ARGS(sp),
		    sppp_phase_name(sp->pp_phase));

	/*
	 * Open all authentication protocols.  This is even required
	 * if we already proceeded to network phase, since it might be
	 * that remote wants us to authenticate, so we might have to
	 * send a PAP request.	Undesired authentication protocols
	 * don't do anything when they get an Open event.
	 */
	if (1 == sp->role) {
		for (i=0; i<IDX_COUNT; i++) {
			if ((cps[i])->flags & CP_AUTH)
				(cps[i])->Up(sp);
		}
	}
	else {
		for (i = 0; i < IDX_COUNT; i++)
			if ((cps[i])->flags & CP_AUTH)
				(cps[i])->Open(sp);
	}

	if (sp->pp_phase == PHASE_NETWORK) {
		/* Notify all NCPs. */
		for (i = 0; i < IDX_COUNT; i++)
			if ((cps[i])->flags & CP_NCP)
				(cps[i])->Open(sp);
	}

	//printf("%s lcp.protos:0x%x\n", __func__, sp->lcp.protos);
	/* Send Up events to all started protos. */
	for (i = 0, mask = 1; i < IDX_COUNT; i++, mask <<= 1)
		if ((sp->lcp.protos & mask) && ((cps[i])->flags & CP_LCP) == 0)
			(cps[i])->Up(sp);

	/* notify low-level driver of state change */
	if (sp->pp_chg)
		sp->pp_chg(sp, (int)sp->pp_phase);
	
	if (sp->pp_phase == PHASE_NETWORK)
		/* if no NCP is starting, close down */
		sppp_lcp_check_and_close(sp);
}

static void
sppp_lcp_tld(struct sppp *sp)
{
	STDDCL;
	int i;
	u_long mask;
#ifdef CONFIG_USER_PPPOE_PROXY	
	struct sppp *sq;
	PPPOEPROXY_DRV_CTRL *proxy_p;
#endif

	sp->pp_phase = PHASE_TERMINATE;

	if (debug)
		addlog(SPP_FMT "PPP: Phase is %s\n", SPP_ARGS(sp),
		    sppp_phase_name(sp->pp_phase));
//	printf("\033[0;31m set sp->myauth.proto from %s to %s\n\033[0m", sppp_proto_name(sp->myauth.proto), sppp_proto_name(sp->myauth.aumode));
	sp->myauth.proto = sp->myauth.aumode;
	
#ifdef CONFIG_USER_PPPOE_PROXY
	if(sp->enable_pppoe_proxy == 1){
		if(sp->role == 0 && sp->currUser >= 0){
			sq = spppq;
			while(sq && (sq->role == 1) && (sq->over == SPPP_PPPOE)){
				proxy_p = sq->pp_lowerp;
				if(sp == proxy_p->sp){
					sendServerPADT(sq);
					lcp.tld(sq);
				}
				sq = sq->pp_next;
			}
			sp->currUser = 0;
		}
	}
#endif
	/*
	 * Take upper layers down.  We send the Down event first and
	 * the Close second to prevent the upper layers from sending
	 * ``a flurry of terminate-request packets'', as the RFC
	 * describes it.
	 */
	for (i = 0, mask = 1; i < IDX_COUNT; i++, mask <<= 1)
		if ((sp->lcp.protos & mask) && ((cps[i])->flags & CP_LCP) == 0) {
			(cps[i])->Down(sp);
			(cps[i])->Close(sp);
		}
	
}

static void
sppp_lcp_tls(struct sppp *sp)
{
	STDDCL;

	sp->pp_phase = PHASE_ESTABLISH;

	if (debug)
		addlog(SPP_FMT "PPP: Phase is %s\n", SPP_ARGS(sp),
		    sppp_phase_name(sp->pp_phase));

	/* Notify lower layer if desired. */
	if (sp->pp_tls){
		(sp->pp_tls)(sp);
	}
}

static void
sppp_lcp_tlf(struct sppp *sp)
{
#ifdef CONFIG_USER_PPPOE_PROXY
	PPPOEPROXY_DRV_CTRL *proxy_p = sp->pp_lowerp;
	struct sppp *sq;
#endif
	STDDCL;
	sp->pp_phase = PHASE_DEAD;
	if (debug)
		addlog(SPP_FMT "PPP: Phase is %s\n", SPP_ARGS(sp),
		    sppp_phase_name(sp->pp_phase));

	/* Notify lower layer if desired. */
	if (sp->pp_tlf && sp->role != 1)
		(sp->pp_tlf)(sp);

#ifdef CONFIG_USER_PPPOE_PROXY
	if (sp->role == 1)
	{
		printf("free unit=%d, session=%d\n",sp->if_unit,proxy_p->session);
		freeSession(proxy_p->session);
		memset(proxy_p->peerEthAddr, 0, sizeof(proxy_p->peerEthAddr));
		sq = proxy_p->sp;
		sq->currUser--; 
	}
#endif
}

static void
sppp_lcp_scr(struct sppp *sp)
{
#ifdef CONFIG_USER_PPPOMODEM
	char opt[6 /* magicnum */ + 4 /* mru */ + 5 /* chap */ + 6 /* accm */];
#elif defined(CONFIG_USER_PPTPD_PPTPD) || defined(CONFIG_USER_L2TPD_LNS)
	char opt[6 /* magicnum */ + 4 /* mru */ + 5 /* chap */ + 6 /* accm */];
#else
	char opt[6 /* magicnum */ + 4 /* mru */ + 5 /* chap */];
#endif //CONFIG_USER_PPPOMODEM
	int i = 0;
	u_short authproto;

	//printf("%s sp->over=%d sp->lcp.opts=0x%x\n", __func__, sp->over, sp->lcp.opts);
#ifdef CONFIG_USER_PPPOMODEM
	/* lcp option, accm*/
	if(sp->over==SPPP_PPPOMODEM){
		if (sp->lcp.opts & (1 << LCP_OPT_ASYNC_MAP)) {
			opt[i++] = LCP_OPT_ASYNC_MAP;
			opt[i++] = 6;
			opt[i++] = sp->lcp.raccm >> 24;
			opt[i++] = sp->lcp.raccm >> 16;
			opt[i++] = sp->lcp.raccm >> 8;
			opt[i++] = sp->lcp.raccm;
		}
	}
#endif //CONFIG_USER_PPPOMODEM
#ifdef CONFIG_PPP_MPPE_MPPC
	if (sp->over == SPPP_PPTP) {
		opt[i++] = LCP_OPT_ASYNC_MAP;
		opt[i++] = 6;
		opt[i++] = sp->lcp.raccm >> 24;
		opt[i++] = sp->lcp.raccm >> 16;
		opt[i++] = sp->lcp.raccm >> 8;
		opt[i++] = sp->lcp.raccm;
	}
#endif//endof CONFIG_PPP_MPPE_MPPC
	if (sp->lcp.opts & (1 << LCP_OPT_MAGIC)) {
		if (! sp->lcp.magic)
			sp->lcp.magic = rand();
		opt[i++] = LCP_OPT_MAGIC;
		opt[i++] = 6;
		opt[i++] = sp->lcp.magic >> 24;
		opt[i++] = sp->lcp.magic >> 16;
		opt[i++] = sp->lcp.magic >> 8;
		opt[i++] = sp->lcp.magic;
	}

	if (sp->lcp.opts & (1 << LCP_OPT_MRU)) {
		opt[i++] = LCP_OPT_MRU;
		opt[i++] = 4;
		opt[i++] = sp->lcp.mru >> 8;
		opt[i++] = sp->lcp.mru;
	}

	if (sp->lcp.opts & (1 << LCP_OPT_AUTH_PROTO)) {
		authproto = sp->hisauth.proto;
		opt[i++] = LCP_OPT_AUTH_PROTO;
		opt[i++] = authproto == PPP_CHAP? 5: 4;
		opt[i++] = authproto >> 8;
		opt[i++] = authproto;
		if (authproto == PPP_CHAP) {
			if (sp->myauth.chal_type == CHAP_MICROSOFT_V2)
				opt[i++] = CHAP_MICROSOFT_V2;
			else
				opt[i++] = CHAP_MD5;
		}
	}

#ifdef CONFIG_PPP_MPPE_MPPC
	if (sp->lcp.opts & (1 << LCP_OPT_PROTO_COMP)) {
		opt[i++] = LCP_OPT_PROTO_COMP;
		opt[i++] = 2;
	}

	if (sp->lcp.opts & (1<< LCP_OPT_ADDR_COMP)) {
		opt[i++] = LCP_OPT_ADDR_COMP;
		opt[i++] = 02;
	}
#endif

	sp->confid[IDX_LCP] = ++sp->pp_seq[IDX_LCP];
	sppp_cp_send (sp, PPP_LCP, CONF_REQ, sp->confid[IDX_LCP], i, &opt);
}

/*
 * Check the open NCPs, return true if at least one NCP is open.
 */
static int
sppp_ncp_check(struct sppp *sp)
{
	int i, mask;

	for (i = 0, mask = 1; i < IDX_COUNT; i++, mask <<= 1)
		if ((sp->lcp.protos & mask) && (cps[i])->flags & CP_NCP)
			return 1;
	return 0;
}

/*
 * Re-check the open NCPs and see if we should terminate the link.
 * Called by the NCPs during their tlf action handling.
 */
static void
sppp_lcp_check_and_close(struct sppp *sp)
{

	if (sp->pp_phase < PHASE_NETWORK)
		/* don't bother, we are already going down */
		return;

	if (sppp_ncp_check(sp))
		return;

	lcp.Close(sp);
}

#ifdef CONFIG_PPP_MPPE_MPPC

/*
 *--------------------------------------------------------------------------*
 *                                                                          *
 *                        The CCP implementation.                          *
 *                                                                          *
 *--------------------------------------------------------------------------*
 */

static void
sppp_ccp_init(struct sppp *sp)
{
//    printf("[%s %d]\n",__func__,__LINE__);
    ccp_flags_set(sp->dev_fd, 1, 0);
//    sp->ccp.neg_mppe_mppc=MPPE_MPPC;    
    sp->ccp.neg_mppe_mppc=sp->myauth.enc_type;
    sp->ccp.zero_byte=0x00;
    sp->ccp.neg_stateless=MPPE_STATELESS;
	sppp_callout_handle_init(&sp->ch[IDX_CCP]);
}


static void set_flags (int fd, int flags)
{    
  //  SYSDEBUG ((LOG_DEBUG, "set flags = %x\n", flags));

    if (ioctl(fd, PPPIOCSFLAGS, (caddr_t) &flags) < 0) {
    printf("ioctl(PPPIOCSFLAGS, %x): %m(%d)", flags, errno) ;
//	if (! ok_error (errno) )
//	    fatal("ioctl(PPPIOCSFLAGS, %x): %m(%d)", flags, errno);
    }
}

static int get_flags (int fd)
{    
    int flags;

    if (ioctl(fd, PPPIOCGFLAGS, (caddr_t) &flags) < 0) {
    printf("ioctl(PPPIOCGFLAGS): %m");
    /*
	if ( ok_error (errno) )
	    flags = 0;
	else
	    fatal("ioctl(PPPIOCGFLAGS): %m");
*/	    
    }

  //  SYSDEBUG ((LOG_DEBUG, "get flags = %x\n", flags));
    return flags;
}


void ccp_flags_set (int unit, int isopen, int isup)
{
   // if (still_ppp()) {
	int x = get_flags(unit);
	x = isopen? x | SC_CCP_OPEN : x &~ SC_CCP_OPEN;
	x = isup?   x | SC_CCP_UP   : x &~ SC_CCP_UP;
	set_flags (unit, x);
    //}
}


static void
sppp_ccp_up(struct sppp *sp)
{
	//printf("[%s %d]\n",__func__,__LINE__);

//	struct sysinfo info;
	sppp_up_event(&ccp, sp);
//	sysinfo(&info);
//	starttime[sp->if_unit] = (int)info.uptime;	// Jenny, PPP start time
	// Added by Mason Yu for PPP LED on
	va_cmd("/bin/ethctl",2,1,"ppp","on");	
#ifdef _CWMP_MIB_
	/* Jenny, support auto disconnect, set timer */
	if (sp->autoDisconnectTime)
		TIMEOUT(sppp_auto_disconnect_timeout, (void *)sp, (sp->autoDisconnectTime)* hz, sp->autodisconnect_ch);
#endif

}

static void
sppp_ccp_down(struct sppp *sp)
{

struct sysinfo info;
	sppp_down_event(&ccp, sp);
	sysinfo(&info);
	if (starttime[sp->if_unit])
		totaluptime[sp->if_unit] += (int)info.uptime - starttime[sp->if_unit];
	starttime[sp->if_unit] = 0;
	va_cmd("/bin/ethctl",2,1,"ppp","off");
#ifdef IP_PASSTHROUGH
	if (sp->ippt_flag ==1) {
		unlink("/tmp/PPPHalfBridge");	
	}
#endif
}

static void
sppp_ccp_open(struct sppp *sp)
{
	STDDCL;

	//printf("%s %d\n", __func__, __LINE__);
	
	if (1 == sp->role) {
		//printf("%s enc_type:%x proto:%x aumode:%x\n", __func__, 
		//		sp->myauth.enc_type, sp->myauth.proto, sp->myauth.aumode);
		if ((0 == sp->myauth.enc_type) && (sp->myauth.aumode != 0))
			return;
	}

	sppp_open_event(&ccp, sp);
}

static void
sppp_ccp_close(struct sppp *sp)
{
	sppp_close_event(&ccp, sp);
}

static void
sppp_ccp_TO(void *cookie)
{
	sppp_to_event(&ccp, (struct sppp *)cookie);
}






int ccp_test (int unit, u_char *opt_ptr, int opt_len, int for_transmit)
{
    struct ppp_option_data data;

    memset (&data, '\0', sizeof (data));
    data.ptr      = opt_ptr;
    data.length   = opt_len;
    data.transmit = for_transmit;

    if (ioctl(unit, PPPIOCSCOMPRESS, (caddr_t) &data) >= 0)
	return 1;

    return (errno == ENOBUFS)? 0: -1;
}

//#define BCOPY(s, d, l)		memcpy(d, s, l)



/*
 * Analyze a configure request.  Return true if it was agreeable, and
 * caused action sca, false if it has been rejected or nak'ed, and
 * caused action scn.  (The return value is used to make the state
 * transition decision in the state automaton.)
 */
static int
sppp_ccp_RCR(struct sppp *sp, struct lcp_header *h, int len)
{
	STDDCL;
	u_char *buf, *r, *p;
	int rlen, origlen;
//	u_long hisaddr, desiredaddr;
//	int gotmyaddr = 0;
//	int desiredcomp;
	char strbuff[256];

	len -= 4;
	origlen = len;
	/*
	 * Make sure to allocate a buf that can at least hold a
	 * conf-nak with an `address' option.  We might need it below.
	 */
	buf = r = malloc((len < 6? 6: len));
	if (! buf)
		return (0);

	memset (buf, 0, (len < 6? 6: len));
	memset (r, 0, (len < 6? 6: len));
	strbuff[0] = 0;
	
	/* pass 1: see if we can recognize them */
	if (debug)
		sprintf(strbuff, "%s CCP:   parse opts: ", SPP_ARGS(sp));
		//addlog(SPP_FMT "IPCP:   parse opts: ",
		//    SPP_ARGS(sp));
	p = (void*) (h+1);
	for (rlen=0; len>1 && p[1]; len-=p[1], p+=p[1]) {
		if (debug)
			sprintf(strbuff, "%s %s ", strbuff, sppp_ipcp_opt_name(*p));
			//addlog(" %s ", sppp_ipcp_opt_name(*p));
		switch (*p) {
//check CCP REQ option
        case CCP_Microsoft_PPC:

                   if (len >= CILEN_MPPE && p[1] == CILEN_MPPE)    
                            continue;
                        if (debug)
                            sprintf(strbuff, "%s[invalid] ", strbuff);
                
                        break;

		default:
			/* Others not supported. */
			if (debug)
				sprintf(strbuff, "%s[rej] ", strbuff);
				//addlog("[rej] ");
			break;
		}
		/* Add the option to rejected list. */
		bcopy (p, r, p[1]);
		r += p[1];
		rlen += p[1];
	}
	if (rlen) {
		if (debug)
			addlog("%s send conf-rej\n", strbuff);
			//addlog(" send conf-rej\n");	
		sppp_cp_send (sp, PPP_CCP, CONF_REJ, h->ident, rlen, buf);            
		return 0;
	} else if (debug)
		addlog("\n");

	/* pass 2: parse option values */
	if (debug)
		sprintf(strbuff, "%s CCP:   parse opt values: ", SPP_ARGS(sp));
		//addlog(SPP_FMT "IPCP:   parse opt values: ",
		//       SPP_ARGS(sp));
	p = (void*) (h+1);
	len = origlen;
	for (rlen=0; len>1 && p[1]; len-=p[1], p+=p[1]) {
		if (debug)
			sprintf(strbuff, "%s %s ", strbuff, sppp_ipcp_opt_name(*p));
			//addlog(" %s ", sppp_ipcp_opt_name(*p));
		switch (*p) {

        case CCP_Microsoft_PPC:
	        if(sp->myauth.enc_type) 
	        {
	            if((p[2]==sp->ccp.neg_stateless)&&(p[3]==sp->ccp.zero_byte)&&(p[4]==sp->ccp.zero_byte)&&(p[5]==sp->ccp.neg_mppe_mppc))
	            {
	            //option correct then send ack 
	            //printf("option correct then send ack\n");
	            	if (sp->myauth.aumode == 0)
	            		ccp_up_event(&ccp, sp);
	                continue;
	            }
	            else
	            {
	            	
					//if auth mode is auto, we should adjust encrypt mode dynamically.
					if ((sp->myauth.aumode == 0) &&
						(p[2]==sp->ccp.neg_stateless)&&(p[3]==sp->ccp.zero_byte)&&(p[4]==sp->ccp.zero_byte))
					{
						sp->ccp.neg_mppe_mppc = p[5];
						
						if (sp->myauth.aumode == 0)
							ccp_up_event(&ccp, sp);
						continue;
					}

	            //set CCP option then send Nak
	            //printf("set CCP option then send Nak\n");
	                p[2]=sp->ccp.neg_stateless;
	                p[3]=sp->ccp.zero_byte;
	                p[4]=sp->ccp.zero_byte;
	                p[5]=sp->ccp.neg_mppe_mppc;
	            
	            }

	        }

			if (sp->myauth.aumode == 0)
	    		ccp_up_event(&ccp, sp);
			
	        break;
		}
		/* Add the option to nak'ed list. */
		bcopy (p, r, p[1]);
		r += p[1];
		rlen += p[1];
	}


	if (rlen) { 
            	sppp_cp_send (sp, PPP_CCP, CONF_NAK, h->ident, rlen, buf);
		
	} else {
		if (debug)
			addlog("%s send conf-ack\n", strbuff);
			//addlog(" send conf-ack\n");

        sppp_cp_send (sp, PPP_CCP, CONF_ACK,
			      h->ident, origlen, h+1);
	}

	free (buf);
	return (rlen == 0);
}

/*
 * Analyze the CCP Configure-Reject option list, and adjust our
 * negotiation.
 */
static void
sppp_ccp_RCN_rej(struct sppp *sp, struct lcp_header *h, int len)
{
	u_char *buf, *p;
	char strbuff[256];
	STDDCL;

	len -= 4;
	buf = malloc(len);
	if (!buf)
		return;

	strbuff[0] = 0;
	if (debug)
		sprintf(strbuff, "%s ccp rej opts: ", SPP_ARGS(sp));
		//addlog(SPP_FMT "ipcp rej opts: ",
		//    SPP_ARGS(sp));

	p = (void*) (h+1);
	for (; len > 1 && p[1]; len -= p[1], p += p[1]) {
		if (debug)
			sprintf(strbuff, "%s %s ", strbuff, (*p));
			//addlog(" %s ", sppp_ipcp_opt_name(*p));
		switch (*p) {
		
		}
	}
	if (debug)
		addlog("%s\n", strbuff);
		//addlog("\n");
	free (buf);
	return;
}

/*
 * Analyze the CCP Configure-NAK option list, and adjust our
 * negotiation.
 */
static void
sppp_ccp_RCN_nak(struct sppp *sp, struct lcp_header *h, int len)
{
	u_char *buf, *p;
	STDDCL;
	char strbuff[256];

	len -= 4;
	buf = malloc(len);
	if (!buf)
		return;

	strbuff[0] = 0;
	if (debug)
		sprintf(strbuff, "%s CCP:   nak opts: ", SPP_ARGS(sp));
		//addlog(SPP_FMT "IPCP:   nak opts: ",
		//    SPP_ARGS(sp));

	p = (void*) (h+1);
	for (; len > 1 && p[1]; len -= p[1], p += p[1]) {
		if (debug)
			sprintf(strbuff, "%s %s ", strbuff, sppp_ipcp_opt_name(*p));
			//addlog(" %s ", sppp_ipcp_opt_name(*p));
		switch (*p) {


         case CCP_Microsoft_PPC:
        if(sp->myauth.enc_type) 
        {
            sp->ccp.neg_mppe_mppc=p[5];
//            sp->ccp.neg_mppe_mppc=MPPC;
            sp->ccp.neg_stateless=p[2];

        }
		break;
		}
	}
	if (debug)
		addlog("%s\n", strbuff);
		//addlog("\n");
	free (buf);
	return;
}

static void
sppp_ccp_tlu(struct sppp *sp)
{
	//printf("%s %d role=%d neg_mppe_mppc=0x%x\n", __func__, __LINE__, 
	//			sp->role, sp->ccp.neg_mppe_mppc);
	
//char ext_ifname[32];
	
	/* we are up - notify isdn daemon */
	if (sp->pp_con)
		sp->pp_con(sp, IDX_CCP);
		
	// Mason Yu--- run ifup script	
//	snprintf(ext_ifname, 32, "/var/ppp/ifupv6_%s", sp->if_name);
//	va_cmd(ext_ifname, 0, 1);

}


static void
sppp_ccp_tld(struct sppp *sp)
{

}

static void
sppp_ccp_tls(struct sppp *sp)
{
	/* indicate to CCP that it must stay alive */
	sp->lcp.protos |= (1 << IDX_CCP);
}

static void
sppp_ccp_tlf(struct sppp *sp)
{
}

static void
sppp_ccp_scr(struct sppp *sp)
{
    char opt[64 /* compression */ ];
	int i = 0;
	
	// send CCP REQ to server
	if(sp->myauth.enc_type)
	{
		opt[i++] = CCP_Microsoft_PPC;
		opt[i++] = CILEN_MPPE;
		opt[i++] = sp->ccp.neg_stateless;
		opt[i++] = sp->ccp.zero_byte;
		opt[i++] = sp->ccp.zero_byte;
		opt[i++] = sp->ccp.neg_mppe_mppc;

		if (sp->ccp.neg_mppe_mppc) {//this is client
			if(sp->ccp.neg_mppe_mppc==MPPC)		
			{
				if (ccp_test(sp->dev_fd, opt, 6, 1) <= 0)
					printf("%s %d ccp_test fail.\n", __func__, __LINE__);

				if (ccp_test(sp->dev_fd, opt, 6, 0) <= 0)
					printf("%s %d ccp_test fail.\n", __func__, __LINE__);
			}
			else
			{
				/* 
				 * The kernel ppp driver needs the session key 
				 * which is not sent via CCP :( 
				 */
				unsigned int keysize;
				unsigned char opt_buf[64];
				opt_buf[0] = CI_MPPE;
				opt_buf[1] = CILEN_MPPE;
				opt_buf[2] = sp->ccp.neg_stateless & MPPE_STATELESS;
				opt_buf[3] = 0;
				opt_buf[4] = 0;
				opt_buf[5] = sp->ccp.neg_mppe_mppc & MPPE_MPPC;
				/* push in our send/receive keys */
				if(opt_buf[5] & MPPE_40BIT) {
					keysize = 8;
					BCOPY(mppe_master_send_key_40, opt_buf+6, keysize);
					BCOPY(mppe_master_recv_key_40, opt_buf+14, keysize);
				} else if(opt_buf[5] & MPPE_128BIT) {
					keysize = 16;
					BCOPY(mppe_master_send_key_128, opt_buf+6, keysize);
					BCOPY(mppe_master_recv_key_128, opt_buf+22, keysize);
				} else {
					printf("unknown ccp format.\n");
				}
				/* call ioctl and pass this nasty stuff to the kernel */
				if (ccp_test(sp->dev_fd, opt_buf, (2*keysize)+6, 1) <= 0){
					printf("%s %d ccp_test fail.\n", __func__, __LINE__);
				}

				/* push in our send/receive keys */
				if(opt_buf[5] & MPPE_40BIT) {
					keysize = 8;
					BCOPY(mppe_master_recv_key_40, opt_buf+6, keysize);
					BCOPY(mppe_master_send_key_40, opt_buf+14, keysize);
				} else if(opt_buf[5] & MPPE_128BIT) {
					keysize = 16;
					BCOPY(mppe_master_recv_key_128, opt_buf+6, keysize);
					BCOPY(mppe_master_send_key_128, opt_buf+22, keysize);
				}
				if (ccp_test(sp->dev_fd, opt_buf, (2*keysize)+6, 0) <= 0){
					printf("%s %d ccp_test fail.\n", __func__, __LINE__);
				}
			}
		}

		sp->confid[IDX_CCP] = ++sp->pp_seq[IDX_CCP];
		sppp_cp_send(sp, PPP_CCP, CONF_REQ, sp->confid[IDX_CCP], i, &opt);
	}
}
#endif


/*
 *--------------------------------------------------------------------------*
 *                                                                          *
 *                        The IPCP implementation.                          *
 *                                                                          *
 *--------------------------------------------------------------------------*
 */

static void
sppp_ipcp_init(struct sppp *sp)
{
	sp->ipcp.opts = 0;
#ifdef CONFIG_SPPPD_STATICIP
	if (sp->staticip == 0)
		sp->ipcp.myipaddr = 0;
#else
	sp->ipcp.myipaddr = 0;
#endif
	sp->ipcp.hisipaddr = 0;
	sp->ipcp.dns_opts = IPCP_OPT_DNS_SECOND;
	sp->ipcp.primary_dns = 0;
	sp->ipcp.second_dns = 0;
	sp->ipcp.flags = 0;
	sp->state[IDX_IPCP] = STATE_INITIAL;
	sp->fail_counter[IDX_IPCP] = 0;
	sp->pp_seq[IDX_IPCP] = 0;
	sp->pp_rseq[IDX_IPCP] = 0;

	sppp_callout_handle_init(&sp->ch[IDX_IPCP]);
}

static void
sppp_ipcp_up(struct sppp *sp)
{
//	struct sysinfo info;
	//printf("%s %d\n", __func__, __LINE__);
	sppp_up_event(&ipcp, sp);
//	sysinfo(&info);
//	starttime[sp->if_unit] = (int)info.uptime;	// Jenny, PPP start time
	// Added by Mason Yu for PPP LED on
	va_cmd("/bin/ethctl",2,1,"ppp","on");	
#ifdef _CWMP_MIB_
	/* Jenny, support auto disconnect, set timer */
	if (sp->autoDisconnectTime)
		TIMEOUT(sppp_auto_disconnect_timeout, (void *)sp, (sp->autoDisconnectTime)* hz, sp->autodisconnect_ch);
#endif
}

static void
sppp_ipcp_down(struct sppp *sp)
{
	char ext_ifname[32];
	struct sysinfo info;
	sppp_down_event(&ipcp, sp);
	sysinfo(&info);
	if (starttime[sp->if_unit])
		totaluptime[sp->if_unit] += (int)info.uptime - starttime[sp->if_unit];
	starttime[sp->if_unit] = 0;
	// Added by Mason Yu for PPP LED off
	va_cmd("/bin/ethctl",2,1,"ppp","off");
//endif ppp term

//#ifdef CONFIG_USER_PPPOE_PROXY
#if 0
	if (sp->enable_pppoe_proxy) {
		/*ping_zhang:20080806 START:disable no_adsl_link_wan_ppp action after disconnect pppoe proxy connection actively.*/
		//printf("%s:%d sp->unit=%d\n",__FUNCTION__, __LINE__,sp->unit);			
		//no_adsl_link_wan_ppp(sp->unit);
		return;
	}
#endif
#ifdef CONFIG_NO_REDIAL		
		if(no_redial){
			
			sp->is_noredial =1;
		}	
		else 
			sp->is_noredial =0;
#endif
	// set holdoff delay (delay to re-initiating link)
	if (holdoff_interval) {
		// Specifies timer to wait before re-initiating the link after it terminates.
		sp->holdoff_delay = holdoff_interval;
	}

#ifdef IP_PASSTHROUGH
	// Mason Yu. for Half Bridge
	if (sp->ippt_flag ==1) {
		unlink("/tmp/PPPHalfBridge");	
	}
#endif
#ifdef CONFIG_USER_PPPOE_PROXY	
	if(sp->role == 0 && !sp->enable_pppoe_proxy)
#endif
	{
		// Kaohj --- run ifdown script
		snprintf(ext_ifname, 32, "/var/ppp/ifdown_%s", sp->if_name);
		printf("run %s\n", ext_ifname);
		va_cmd(ext_ifname, 0, 1);
	}
}

static void
sppp_ipcp_open(struct sppp *sp)
{
	STDDCL;
	u_long myaddr, hisaddr;

	// Kaohj -- 0: ipv4, 1: ipv6, 2: both
	if (sp->ipType != 0 && sp->ipType != 2)
		return;

	if (1 == sp->role) {
#if defined(CONFIG_USER_PPTPD_PPTPD) || defined(CONFIG_USER_L2TPD_LNS)
		/* assigned peer IP now */
		if(sp->over == SPPP_PPTPD || sp->over == SPPP_L2TPD){
			if (assignedIPToPppdClient(sp->over, &sp->ipcp.myipaddr, &sp->ipcp.hisipaddr, &sp->ipcp.primary_dns) < 0)
				printf("%s server is malformed.\n", __func__);
			else {
				if (sp->ipcp.hisipaddr)
					sp->ipcp.opts = (1 << IPCP_OPT_ADDRESS);
				sp->ipcp.dns_opts = 0;
			}
		}
#endif
		sppp_get_ip_addrs(sp, &myaddr, &hisaddr, 0);
		if (myaddr)
			sp->ipcp.flags |= IPCP_MYADDR_SEEN;
		if (hisaddr)
			sp->ipcp.flags |= IPCP_HISADDR_SEEN;

		sppp_open_event(&ipcp, sp);
		return;
	}
	
	sp->ipcp.flags &= ~(IPCP_HISADDR_SEEN | IPCP_MYADDR_SEEN |
			    IPCP_MYADDR_DYN | IPCP_VJ);
	sp->ipcp.opts = 0;

	sppp_get_ip_addrs(sp, &myaddr, &hisaddr, 0);
	/*
	 * If we don't have his address, this probably means our
	 * interface doesn't want to talk IP at all.  (This could
	 * be the case if somebody wants to speak only IPX, for
	 * example.)  Don't open IPCP in this case.
	 */
#if 0 /* not support this feature */
	if (hisaddr == 0L) {
		/* XXX this message should go away */
		if (debug)
			addlog(SPP_FMT "ipcp_open(): no IP interface\n",
			    SPP_ARGS(sp));
		return;
	}
#endif

	if (myaddr == 0L) {
		/*
		 * I don't have an assigned address, so i need to
		 * negotiate my address.
		 */
		sp->ipcp.flags |= IPCP_MYADDR_DYN;
		sp->ipcp.opts |= (1 << IPCP_OPT_ADDRESS);
	} else {
		sp->ipcp.flags |= IPCP_MYADDR_SEEN;
#ifdef CONFIG_SPPPD_STATICIP
		if (sp->staticip == 1)	// Jenny, send static ip option
			sp->ipcp.opts |= (1 << IPCP_OPT_ADDRESS);
#endif
	}

	sppp_open_event(&ipcp, sp);
}

static void
sppp_ipcp_close(struct sppp *sp)
{
	sppp_close_event(&ipcp, sp);
	/*
	 * My address was dynamic, clear it again.
	 */
	// Casey
	if (sp->ipcp.flags & IPCP_MYADDR_DYN)
		sppp_set_ip_addr(sp, 0L);
}

static void
sppp_ipcp_TO(void *cookie)
{
	sppp_to_event(&ipcp, (struct sppp *)cookie);
}

/*
 * Analyze a configure request.  Return true if it was agreeable, and
 * caused action sca, false if it has been rejected or nak'ed, and
 * caused action scn.  (The return value is used to make the state
 * transition decision in the state automaton.)
 */
static int
sppp_ipcp_RCR(struct sppp *sp, struct lcp_header *h, int len)
{
	STDDCL;
	u_char *buf, *r, *p;
	int rlen, origlen;
	u_long hisaddr, desiredaddr, primarydns;
	int gotmyaddr = 0;
	int desiredcomp;
	char strbuff[256];

	len -= 4;
	origlen = len;
	/*
	 * Make sure to allocate a buf that can at least hold a
	 * conf-nak with an `address' option.  We might need it below.
	 */
	buf = r = malloc((len < 6? 6: len));
	if (! buf)
		return (0);

	memset (buf, 0, (len < 6? 6: len));
	memset (r, 0, (len < 6? 6: len));
	strbuff[0] = 0;
	
	/* pass 1: see if we can recognize them 
	 *  for server, should check for things that need to be rejected 
	 */
	if (debug)
		sprintf(strbuff, "%s IPCP:   parse opts: ", SPP_ARGS(sp));
		//addlog(SPP_FMT "IPCP:   parse opts: ",
		//    SPP_ARGS(sp));
	p = (void*) (h+1);
	for (rlen=0; len>1 && p[1]; len-=p[1], p+=p[1]) {
		if (debug)
			sprintf(strbuff, "%s %s ", strbuff, sppp_ipcp_opt_name(*p));
			//addlog(" %s ", sppp_ipcp_opt_name(*p));
		switch (*p) {
		case IPCP_OPT_ADDRESS:
		case IPCP_OPT_DNS_PRIMARY:
		case IPCP_OPT_DNS_SECOND:
			if (1 == sp->role) {//PPPD don't support second DNS
				if (*p == IPCP_OPT_DNS_SECOND)
					break;
			}
			
			if (len >= 6 && p[1] == 6) {
				/* correctly formed address option */
				continue;
			}
			if (debug)
				sprintf(strbuff, "%s[invalid] ", strbuff);
				//addlog("[invalid] ");
			break;
		default:
			/* Others not supported. */
			if (debug)
				sprintf(strbuff, "%s[rej] ", strbuff);
				//addlog("[rej] ");
			break;
		}
		/* Add the option to rejected list. */
		bcopy (p, r, p[1]);
		r += p[1];
		rlen += p[1];
	}
	if (rlen) {
		if (debug)
			addlog("%s send conf-rej\n", strbuff);
			//addlog(" send conf-rej\n");
		sppp_cp_send (sp, PPP_IPCP, CONF_REJ, h->ident, rlen, buf);            
		
		return 0;
	} else if (debug)
		addlog("\n");

	/* pass 2: parse option values */
	if (debug)
		sprintf(strbuff, "%s IPCP:   parse opt values: ", SPP_ARGS(sp));
		//addlog(SPP_FMT "IPCP:   parse opt values: ",
		//       SPP_ARGS(sp));
	p = (void*) (h+1);
	len = origlen;
	for (rlen=0; len>1 && p[1]; len-=p[1], p+=p[1]) {
		if (debug)
			sprintf(strbuff, "%s %s ", strbuff, sppp_ipcp_opt_name(*p));
			//addlog(" %s ", sppp_ipcp_opt_name(*p));
		switch (*p) {
		case IPCP_OPT_ADDRESS:
			if (1 == sp->role)
			{
				desiredaddr = p[2] << 24 | p[3] << 16 | p[4] << 8 | p[5];
				hisaddr = htonl(sp->ipcp.hisipaddr);
				if (hisaddr != desiredaddr)
				{
					p[2] = hisaddr >> 24;
					p[3] = hisaddr >> 16;
					p[4] = hisaddr >> 8;
					p[5] = hisaddr;
					break;
				}
				continue;
			}
			
			// Casey
			sppp_get_ip_addrs(sp, 0, &hisaddr, 0);
			if(hisaddr == 0)
			{
				/* record that we've seen it already */
				sp->ipcp.flags |= IPCP_HISADDR_SEEN;
				sp->ipcp.hisipaddr = p[2] << 24 | p[3] << 16 |
									p[4] << 8 | p[5];
				if (debug)
					sprintf(strbuff, "%s[hisaddr %s] ", strbuff, sppp_dotted_quad(sp->ipcp.hisipaddr));
					//addlog("[hisaddr %s] ",
					//      sppp_dotted_quad(sp->ipcp.hisipaddr));
				continue;
			}
			/* This is the address he wants in his end */
			desiredaddr = p[2] << 24 | p[3] << 16 |
				p[4] << 8 | p[5];
			if (desiredaddr == hisaddr ||
			    (hisaddr >= 1 && hisaddr <= 254 && desiredaddr != 0)) {
				/*
				 * Peer's address is same as our value,
				 * or we have set it to 0.0.0.* to
				 * indicate that we do not really care,
				 * this is agreeable.  Gonna conf-ack
				 * it.
				 */
				if (debug)
					sprintf(strbuff, "%s%s [ack] ", strbuff, sppp_dotted_quad(hisaddr));
					//addlog("%s [ack] ",
					//	sppp_dotted_quad(hisaddr));
				/* record that we've seen it already */
				sp->ipcp.flags |= IPCP_HISADDR_SEEN;
				continue;
			}
			/*
			 * The address wasn't agreeable.  This is either
			 * he sent us 0.0.0.0, asking to assign him an
			 * address, or he send us another address not
			 * matching our value.  Either case, we gonna
			 * conf-nak it with our value.
			 * XXX: we should "rej" if hisaddr == 0
			 */
			if (debug) {
				if (desiredaddr == 0)
					sprintf(strbuff, "%s[addr requested] ", strbuff);
					//addlog("[addr requested] ");
				else
					sprintf(strbuff, "%s%s [not agreed] ", strbuff, sppp_dotted_quad(desiredaddr));
					//addlog("%s [not agreed] ",
					//	sppp_dotted_quad(desiredaddr));

			}
			p[2] = hisaddr >> 24;
			p[3] = hisaddr >> 16;
			p[4] = hisaddr >> 8;
			p[5] = hisaddr;
			break;
		case IPCP_OPT_DNS_PRIMARY:
			if (1 == sp->role) {
				primarydns = p[2] << 24 | p[3] << 16 | p[4] << 8 | p[5];
				if (sp->ipcp.primary_dns != primarydns) {
					p[2] = sp->ipcp.primary_dns >> 24;
					p[3] = sp->ipcp.primary_dns >> 16;
					p[4] = sp->ipcp.primary_dns >> 8;
					p[5] = sp->ipcp.primary_dns;
					break;
				}
				continue;
			}
			sp->ipcp.primary_dns = p[2] << 24 | p[3] << 16 |
				p[4] << 8 | p[5];
			if (debug)
				sprintf(strbuff, "%s[primary dns %s] ", strbuff, sppp_dotted_quad(sp->ipcp.primary_dns));
				//addlog("[primary dns %s] ", sppp_dotted_quad(sp->ipcp.primary_dns));
			break;
		case IPCP_OPT_DNS_SECOND:
			sp->ipcp.second_dns = p[2] << 24 | p[3] << 16 |
				p[4] << 8 | p[5];
			if (debug)
				sprintf(strbuff, "%s[secondary dns %s] ", strbuff, sppp_dotted_quad(sp->ipcp.second_dns));
				//addlog("[secondary dns %s] ", sppp_dotted_quad(sp->ipcp.second_dns));
			break;
		}
		/* Add the option to nak'ed list. */
		bcopy (p, r, p[1]);
		r += p[1];
		rlen += p[1];
	}

	/*
	 * If we are about to conf-ack the request, but haven't seen
	 * his address so far, gonna conf-nak it instead, with the
	 * `address' option present and our idea of his address being
	 * filled in there, to request negotiation of both addresses.
	 *
	 * XXX This can result in an endless req - nak loop if peer
	 * doesn't want to send us his address.  Q: What should we do
	 * about it?  XXX  A: implement the max-failure counter.
	 */
#ifdef CONFIG_USER_PPPOMODEM
    //SPPP_PPPOMODEM may not bring "hisaddr", skip checking
    if(sp->over != SPPP_PPPOMODEM) 
    {
#endif //CONFIG_USER_PPPOMODEM
	if (rlen == 0 && !(sp->ipcp.flags & IPCP_HISADDR_SEEN) && !gotmyaddr) {
		sppp_get_ip_addrs(sp, 0, &hisaddr, 0);
		buf[0] = IPCP_OPT_ADDRESS;
		buf[1] = 6;
		buf[2] = hisaddr >> 24;
		buf[3] = hisaddr >> 16;
		buf[4] = hisaddr >> 8;
		buf[5] = hisaddr;
		rlen = 6;
		if (debug)
			sprintf(strbuff, "%sstill need hisaddr ", strbuff);
			//addlog("still need hisaddr ");
	}
#ifdef CONFIG_USER_PPPOMODEM
    }//if(sp->over != SPPP_PPPOMODEM)
#endif //CONFIG_USER_PPPOMODEM

	if (rlen) {

        
		if (htonl(sp->ipcp.primary_dns) == 0 && htonl(sp->ipcp.second_dns) == 0) {
			if (debug)
				addlog("%s send conf-rej\n", strbuff);
				//addlog(" send conf-rej\n");
			sppp_cp_send (sp, PPP_IPCP, CONF_REJ, h->ident, rlen, buf);
			return 0;
		}
		if (debug)
			addlog("%s send conf-nak\n", strbuff);
			//addlog(" send conf-nak\n");
		sppp_cp_send (sp, PPP_IPCP, CONF_NAK, h->ident, rlen, buf);
		
	} else {
		if (debug)
			addlog("%s send conf-ack\n", strbuff);
			//addlog(" send conf-ack\n");			
		sppp_cp_send (sp, PPP_IPCP, CONF_ACK,
			      h->ident, origlen, h+1);
	}

	free (buf);
	return (rlen == 0);
}

/*
 * Analyze the IPCP Configure-Reject option list, and adjust our
 * negotiation.
 */
static void
sppp_ipcp_RCN_rej(struct sppp *sp, struct lcp_header *h, int len)
{
	u_char *buf, *p;
	char strbuff[256];
	STDDCL;

	len -= 4;
	buf = malloc(len);
	if (!buf)
		return;

	strbuff[0] = 0;
	if (debug)
		sprintf(strbuff, "%sipcp rej opts: ", SPP_ARGS(sp));
		//addlog(SPP_FMT "ipcp rej opts: ",
		//    SPP_ARGS(sp));

	p = (void*) (h+1);
	for (; len > 1 && p[1]; len -= p[1], p += p[1]) {
		if (debug)
			sprintf(strbuff, "%s %s ", strbuff, sppp_ipcp_opt_name(*p));
			//addlog(" %s ", sppp_ipcp_opt_name(*p));
		switch (*p) {
		case IPCP_OPT_COMPRESSION:
			sp->ipcp.opts &= ~(1 << IPCP_OPT_COMPRESSION);
			break;
		case IPCP_OPT_ADDRESS:
			/*
			 * Peer doesn't grok address option.  This is
			 * bad.  XXX  Should we better give up here?
			 * XXX We could try old "addresses" option...
			 */
			sp->ipcp.opts &= ~(1 << IPCP_OPT_ADDRESS);
			break;
		case IPCP_OPT_DNS_PRIMARY:
			sp->ipcp.dns_opts = 0;
			break;
		case IPCP_OPT_DNS_SECOND:
			if(sp->ipcp.dns_opts)
				sp->ipcp.dns_opts = IPCP_OPT_DNS_PRIMARY;
			break;
		}
	}
	if (debug)
		addlog("%s\n", strbuff);
		//addlog("\n");
	free (buf);
	return;
}

/*
 * Analyze the IPCP Configure-NAK option list, and adjust our
 * negotiation.
 */
static void
sppp_ipcp_RCN_nak(struct sppp *sp, struct lcp_header *h, int len)
{
	u_char *buf, *p;
	STDDCL;
	int desiredcomp;
	u_long wantaddr;
	char strbuff[256];

	len -= 4;
	buf = malloc(len);
	if (!buf)
		return;

	strbuff[0] = 0;
	if (debug)
		sprintf(strbuff, "%s IPCP:   nak opts: ", SPP_ARGS(sp));
		//addlog(SPP_FMT "IPCP:   nak opts: ",
		//    SPP_ARGS(sp));

	p = (void*) (h+1);
	for (; len > 1 && p[1]; len -= p[1], p += p[1]) {
		if (debug)
			sprintf(strbuff, "%s %s ", strbuff, sppp_ipcp_opt_name(*p));
			//addlog(" %s ", sppp_ipcp_opt_name(*p));
		switch (*p) {
		case IPCP_OPT_ADDRESS:
			/*
			 * Peer doesn't like our local IP address.  See
			 * if we can do something for him.  We'll drop
			 * him our address then.
			 */
			if (len >= 6 && p[1] == 6) {
				wantaddr = p[2] << 24 | p[3] << 16 |
					p[4] << 8 | p[5];
				sp->ipcp.opts |= (1 << IPCP_OPT_ADDRESS);
				if (debug)
					sprintf(strbuff, "%s[wantaddr %s] ", strbuff, sppp_dotted_quad(wantaddr));
					//addlog("[wantaddr %s] ",
					//       sppp_dotted_quad(wantaddr));
				/*
				 * When doing dynamic address assignment,
				 * we accept his offer.  Otherwise, we
				 * ignore it and thus continue to negotiate
				 * our already existing value.
			 	 * XXX: Bogus, if he said no once, he'll
				 * just say no again, might as well die.
				 */
				if (sp->ipcp.flags & IPCP_MYADDR_DYN) {
					sppp_set_ip_addr(sp, wantaddr);
					if (debug)
						sprintf(strbuff, "%s[agree] ", strbuff);
						//addlog("[agree] ");
					sp->ipcp.flags |= IPCP_MYADDR_SEEN;
				}
			}
			break;
		case IPCP_OPT_DNS_PRIMARY:
			if (len >= 6 && p[1] == 6)
				sp->ipcp.primary_dns = p[2] << 24 | p[3] << 16 | \
					p[4] << 8 | p[5];
			if (debug)
				sprintf(strbuff, "%s[%s] ", strbuff, sppp_dotted_quad(sp->ipcp.primary_dns));
				//addlog("[%s] ",
				//       sppp_dotted_quad(sp->ipcp.primary_dns));
			break;
		case IPCP_OPT_DNS_SECOND:
			if (len >= 6 && p[1] == 6)
				sp->ipcp.second_dns = p[2] << 24 | p[3] << 16 | \
					p[4] << 8 | p[5];
			if (debug)
				sprintf(strbuff, "%s[%s] ", strbuff, sppp_dotted_quad(sp->ipcp.second_dns));
				//addlog("[%s] ",
				//       sppp_dotted_quad(sp->ipcp.second_dns));
			break;
		}
	}
	if (debug)
		addlog("%s\n", strbuff);
		//addlog("\n");
	free (buf);
	return;
}

static void
sppp_ipcp_tlu(struct sppp *sp)
{
        int my_pid, dhcrelay_pid;
        char * argv[8];        
	struct sysinfo info;
	char ext_ifname[32];
#ifdef CONFIG_USER_IGMPPROXY
	unsigned int igmpItf;
	int flags;
#endif
	char if_name[IFNAMSIZ];
	
	// Mason Yu. Test
	//printf("On sppp_ipcp_tlu\n");

#ifdef CONFIG_IGMPPROXY_MULTIWAN
	int entryNum;
	unsigned char igmpRun = 0; //check if igmp should run or kill
	MIB_CE_ATM_VC_T vcEntry;	
	int i;
#endif

#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)
        unsigned char upnpEnable, upnpItf;
#endif      

#ifdef CONFIG_USER_PPPOE_PROXY
	PPPOEPROXY_DRV_CTRL *proxy_p;
	struct sppp *spmaster;
#endif

	/* we are up - notify isdn daemon */
	if (sp->pp_con)
		sp->pp_con(sp, IDX_IPCP);
#ifdef DIAGNOSTIC_TEST
	sppp_diag_log(sp->if_unit, 3);	// Jenny, diagnostic test, assign IP address pass
#endif
	sysinfo(&info);
	starttime[sp->if_unit] = (int)info.uptime;	// Jenny, PPP start time
	if (sp->over == SPPP_PPPOE)	// PPPoE
	{
		PPPOE_DRV_CTRL * p = (PPPOE_DRV_CTRL *)sp->pp_lowerp;
		PPPOE_SESSION_INFO p_session_info;
		p_session_info.uifno = p->uifno;
		memcpy(&p_session_info.remote,(void *) &p->remote, sizeof(struct sockaddr_ll));
		p_session_info.session= p->session;
		mib_set_PPPoE(CMD_MIB_SAVE_PPPOE, &p_session_info, sizeof(PPPOE_SESSION_INFO));
	}
	else if (sp->over == SPPP_PPPOATM)	// for only PPPoA connection (without any PPPoE)
		pppoe_up_log(sp->if_unit, 1);
#ifdef CONFIG_USER_PPPOMODEM
	else if (sp->over == SPPP_PPPOMODEM) //???
		pppoe_up_log(sp->if_unit, 1);
#endif //CONFIG_USER_PPPOMODEM

#ifdef ROUTING
	route_ppp_ifup(sp->ipcp.hisipaddr);
#endif
      
#ifdef CONFIG_USER_IGMPPROXY
	my_pid = read_pid("/var/run/igmp_pid");
	if (my_pid >= 1) {
		// Kick to sync the multicast virtual interfaces
		if (kill(my_pid, SIGUSR1) != 0) {
			printf("*******igmp proxy Could not kill pid '%d'", my_pid);
		}
		// Kick to reset multicast MFC, Igmpproxy will send IGMP general Query
		// in order to retrieve multicast streams
#ifdef CONFIG_IGMPPROXY_MULTIWAN
		entryNum = mib_chain_total(MIB_ATM_VC_TBL);
		for(i=0; i<entryNum; i++)
		{
			mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&vcEntry);
			if(vcEntry.enableIGMP) {
				if ((igmpItf=vcEntry.ifIndex) != DUMMY_IFINDEX)
				{
					if (PPP_INDEX(igmpItf) != DUMMY_PPP_INDEX) {
						if (sp->if_unit == PPP_INDEX(igmpItf)) {
							kill(my_pid, SIGUSR2);
						}
					}
				}
				else
				{
					printf("Error: IGMP proxy interface:%x not available !\n",vcEntry.ifIndex);
				}
			}
		}
#else
		if (mib_get(MIB_IGMP_PROXY_ITF, (void *)&igmpItf) != 0)
		{
			if (igmpItf != DUMMY_IFINDEX)
			{
				if (sp->if_unit == PPP_INDEX(igmpItf)) {
					kill(my_pid, SIGUSR2);
				}
			}
			else
			{
				printf("Error: IGMP proxy interface not available !\n");
			}
		}
#endif
	}
#endif // of CONFIG_USER_IGMPPROXY
	
	// Mason Yu. kick dhcrelay to sync the interface
#ifdef COMBINE_DHCPD_DHCRELAY
	dhcrelay_pid = read_pid("/var/run/udhcpd.pid");
#else
	dhcrelay_pid = read_pid("/var/run/dhcrelay.pid");
#endif
	if (dhcrelay_pid > 0) {
		printf("sppp_ipcp_tlu: kick dhcrelay to sync the interface\n");		
		kill(dhcrelay_pid, SIGUSR2);
	}
	
#ifdef CONFIG_USER_PPPOE_PROXY
	if(sp->enable_pppoe_proxy == 1)
		sifdown(sp);
	else if((1 == sp->role) && (SPPP_PPPOE == sp->over)){
		proxy_p = sp->pp_lowerp;
		spmaster = proxy_p->sp;
		if(spmaster->currUser >= 1)  //May multiple proxy user connect at the same time, so currUser >=1
			sifup(spmaster);
		add_policy_routing_table(sp, spmaster, spmaster->currUser);
	}else
#endif
	if(0 == sp->role){
		// Kaohj --- run ifup script
		snprintf(ext_ifname, 32, "/var/ppp/ifup_%s", sp->if_name);
		va_cmd(ext_ifname, 0, 1);
	}
//	printf("\033[0;31m set sp->myauth.proto from %s to %s\n\033[0m", sppp_proto_name(sp->myauth.proto), sppp_proto_name(sp->myauth.aumode));
	sp->myauth.proto = sp->myauth.aumode;
}


static void
sppp_ipcp_tld(struct sppp *sp)
{

}

static void
sppp_ipcp_tls(struct sppp *sp)
{
	/* indicate to LCP that it must stay alive */
	sp->lcp.protos |= (1 << IDX_IPCP);
}

static void
sppp_ipcp_tlf(struct sppp *sp)
{
	/* we no longer need LCP */
	sp->lcp.protos &= ~(1 << IDX_IPCP);
	sppp_lcp_check_and_close(sp);
}

static void
sppp_ipcp_scr(struct sppp *sp)
{
	char opt[6 /* compression */ + 6 /* address */];
	u_long ouraddr;
	u_long primary_dns;
	u_long second_dns;
	int i = 0;

	if (sp->ipcp.opts & (1 << IPCP_OPT_COMPRESSION)) {
		opt[i++] = IPCP_OPT_COMPRESSION;
		opt[i++] = 6;
		opt[i++] = IPCP_COMP_VJ >> 8;
		opt[i++] = IPCP_COMP_VJ;
		opt[i++] = sp->ipcp.max_state;
		opt[i++] = sp->ipcp.compress_cid;
	}
	if (sp->ipcp.opts & (1 << IPCP_OPT_ADDRESS)) {
		sppp_get_ip_addrs(sp, &ouraddr, 0, 0);
		opt[i++] = IPCP_OPT_ADDRESS;
		opt[i++] = 6;
		opt[i++] = ouraddr >> 24;
		opt[i++] = ouraddr >> 16;
		opt[i++] = ouraddr >> 8;
		opt[i++] = ouraddr;
	}
	if ((sp->ipcp.dns_opts & IPCP_OPT_DNS_PRIMARY) == IPCP_OPT_DNS_PRIMARY) {
		primary_dns = sp->ipcp.primary_dns;
		opt[i++] = IPCP_OPT_DNS_PRIMARY;
		opt[i++] = 6;
		opt[i++] = primary_dns >> 24;
		opt[i++] = primary_dns >> 16;
		opt[i++] = primary_dns >> 8;
		opt[i++] = primary_dns;
	}
	if ((sp->ipcp.dns_opts & IPCP_OPT_DNS_SECOND) == IPCP_OPT_DNS_SECOND) {
		second_dns = sp->ipcp.second_dns;
		opt[i++] = IPCP_OPT_DNS_SECOND;
		opt[i++] = 6;
		opt[i++] = second_dns >> 24;
		opt[i++] = second_dns >> 16;
		opt[i++] = second_dns >> 8;
		opt[i++] = second_dns;
	}

	sp->confid[IDX_IPCP] = ++sp->pp_seq[IDX_IPCP];
	sppp_cp_send(sp, PPP_IPCP, CONF_REQ, sp->confid[IDX_IPCP], i, &opt);

}

/*
 *--------------------------------------------------------------------------*
 *                                                                          *
 *                      The IPv6CP implementation.                          *
 *                                                                          *
 *--------------------------------------------------------------------------*
 */

#ifdef CONFIG_IPV6
static void
sppp_ipv6cp_init(struct sppp *sp)
{
	sp->ipv6cp.opts = 0;
	sp->ipv6cp.flags = 0;
	sp->state[IDX_IPV6CP] = STATE_INITIAL;
	sp->fail_counter[IDX_IPV6CP] = 0;
	sp->pp_seq[IDX_IPV6CP] = 0;
	sp->pp_rseq[IDX_IPV6CP] = 0;
 	//callout_init(&sp->ch[IDX_IPV6CP], CALLOUT_MPSAFE);
}

static void
sppp_ipv6cp_up(struct sppp *sp)
{
	sppp_up_event(&ipv6cp, sp);
	va_cmd("/bin/ethctl",2,1,"ppp","on");	
#ifdef _CWMP_MIB_
	/* Jenny, support auto disconnect, set timer */
	if (sp->autoDisconnectTime)
		TIMEOUT(sppp_auto_disconnect_timeout, (void *)sp, (sp->autoDisconnectTime)* hz, sp->autodisconnect_ch);
#endif
}

static void
sppp_ipv6cp_down(struct sppp *sp)
{
	struct sysinfo info;
	sppp_down_event(&ipv6cp, sp);
	sysinfo(&info);
	if (starttime[sp->if_unit])
		totaluptime[sp->if_unit] += (int)info.uptime - starttime[sp->if_unit];
	starttime[sp->if_unit] = 0;
	va_cmd("/bin/ethctl",2,1,"ppp","off");
#ifdef IP_PASSTHROUGH
	if (sp->ippt_flag ==1) {
		unlink("/tmp/PPPHalfBridge");	
	}
#endif
	
#ifdef CONFIG_USER_DHCPV6_ISC_DHCP411
	{
		unsigned char pidfile[30], leasefile[30];	
		int dhcpcpid;
	
		// Stop DHCPv6 client					
		snprintf(pidfile, 30, "/var/run/%s%s.pid", DHCPCV6STR, sp->if_name);
		dhcpcpid = read_pid(pidfile);
       	
		if(dhcpcpid > 0) {
			kill(dhcpcpid, 15);
			snprintf(leasefile, 30, "/var/%s%s.leases", DHCPCV6STR, sp->if_name);
			unlink(pidfile);
			unlink(leasefile);			
		}
	}
#endif
}

static void
sppp_ipv6cp_open(struct sppp *sp)
{
	STDDCL;
	struct in6_addr myaddr, hisaddr;

	// Kaohj -- 0: ipv4, 1: ipv6, 2: both
	if (sp->ipType != 1 && sp->ipType != 2)
		return;
#ifdef IPV6CP_MYIFID_DYN
	sp->ipv6cp.flags &= ~(IPV6CP_MYIFID_SEEN|IPV6CP_MYIFID_DYN);
#else
	sp->ipv6cp.flags &= ~IPV6CP_MYIFID_SEEN;
#endif

	sppp_get_ip6_addrs(sp, &myaddr, &hisaddr, 0);
	/*
	 * If we don't have our address, this probably means our
	 * interface doesn't want to talk IPv6 at all.  (This could
	 * be the case if somebody wants to speak only IPX, for
	 * example.)  Don't open IPv6CP in this case.
	 */
	//if (IN6_IS_ADDR_UNSPECIFIED(&myaddr)) {  //pppx won't have IPv6 address
		/* XXX this message should go away */
		#if 0
		if (debug)
			addlog(SPP_FMT "ipv6cp_open(): no IPv6 interface\n",
			    SPP_ARGS(sp));
		return;
		#endif
#ifdef IPV6CP_MYIFID_DYN
		sp->ipv6cp.flags |= IPV6CP_MYIFID_DYN;
		sp->ipv6cp.opts |= (1 << IPV6CP_OPT_IFID);
#endif
	//}
	//else
	//	sp->ipv6cp.flags |= IPV6CP_MYIFID_SEEN;
	sppp_open_event(&ipv6cp, sp);
}

static void
sppp_ipv6cp_close(struct sppp *sp)
{
	sppp_close_event(&ipv6cp, sp);
}

static void
sppp_ipv6cp_TO(void *cookie)
{
	sppp_to_event(&ipv6cp, (struct sppp *)cookie);
}

/*
 * Analyze a configure request.  Return true if it was agreeable, and
 * caused action sca, false if it has been rejected or nak'ed, and
 * caused action scn.  (The return value is used to make the state
 * transition decision in the state automaton.)
 */
static int
sppp_ipv6cp_RCR(struct sppp *sp, struct lcp_header *h, int len)
{
	STDDCL;
	u_char *buf, *r, *p;
	//struct ifnet *ifp = SP2IFP(sp);
	//int rlen, origlen, debug = ifp->if_flags & IFF_DEBUG;
	int rlen, origlen;
	struct in6_addr myaddr, desiredaddr, suggestaddr;
	int ifidcount;
	int type;
	int collision, nohisaddr;
	char ip6buf[INET6_ADDRSTRLEN];

	len -= 4;
	origlen = len;
	/*
	 * Make sure to allocate a buf that can at least hold a
	 * conf-nak with an `address' option.  We might need it below.
	 */
	buf = r = malloc ((len < 6? 6: len));
	if (! buf)
		return (0);

	/* pass 1: see if we can recognize them */
	if (debug)
		addlog(SPP_FMT "ipv6cp parse opts:",
		    SPP_ARGS(sp));
	p = (void*) (h+1);
	ifidcount = 0;
	for (rlen=0; len >= 2 && p[1] >= 2 && len >= p[1];
	    len-=p[1], p+=p[1]) {
		if (debug)
			addlog(" %s", sppp_ipv6cp_opt_name(*p));
		switch (*p) {
		case IPV6CP_OPT_IFID:
			if (len >= 10 && p[1] == 10 && ifidcount == 0) {
				/* correctly formed address option */
				ifidcount++;
				continue;
			}
			if (debug)
				addlog(" [invalid]");
			break;
#ifdef notyet
		case IPV6CP_OPT_COMPRESSION:
			if (len >= 4 && p[1] >= 4) {
				/* correctly formed compress option */
				continue;
			}
			if (debug)
				addlog(" [invalid]");
			break;
#endif
		default:
			/* Others not supported. */
			if (debug)
				addlog(" [rej]");
			break;
		}
		/* Add the option to rejected list. */
		bcopy (p, r, p[1]);
		r += p[1];
		rlen += p[1];
	}
	if (rlen) {
		if (debug)
			addlog(" send conf-rej\n");
		sppp_cp_send (sp, PPP_IPV6CP, CONF_REJ, h->ident, rlen, buf);
		goto end;
	} else if (debug)
		addlog("\n");

	/* pass 2: parse option values */
	sppp_get_ip6_addrs(sp, &myaddr, 0, 0);
	if (debug)
		addlog(SPP_FMT "ipv6cp parse opt values: ",
		    SPP_ARGS(sp));
	p = (void*) (h+1);
	len = origlen;
	type = CONF_ACK;
	for (rlen=0; len >= 2 && p[1] >= 2 && len >= p[1];
	    len-=p[1], p+=p[1]) {
		if (debug)
			addlog(" %s", sppp_ipv6cp_opt_name(*p));
		switch (*p) {
#ifdef notyet
		case IPV6CP_OPT_COMPRESSION:
			continue;
#endif
		case IPV6CP_OPT_IFID:
			bzero(&desiredaddr, sizeof(desiredaddr));
			bcopy(&p[2], &desiredaddr.s6_addr[8], 8);
			collision = (bcmp(&desiredaddr.s6_addr[8],
					  &myaddr.s6_addr[8], 8) == 0);
			nohisaddr = IN6_IS_ADDR_UNSPECIFIED(&desiredaddr);

			desiredaddr.s6_addr16[0] = htons(0xfe80);
			// Kaohj
			//(void)in6_setscope(&desiredaddr, SP2IFP(sp), NULL);

			if (!collision && !nohisaddr) {
				/* no collision, hisaddr known - Conf-Ack */
				type = CONF_ACK;

				if (debug) {
					addlog(" %s [%s]",
					    ip6_sprintf(ip6buf, &desiredaddr),
					    sppp_cp_type_name(type));
				}
				continue;
			}

			bzero(&suggestaddr, sizeof(&suggestaddr));
			if (collision && nohisaddr) {
				/* collision, hisaddr unknown - Conf-Rej */
				type = CONF_REJ;
				bzero(&p[2], 8);
			} else {
				/*
				 * - no collision, hisaddr unknown, or
				 * - collision, hisaddr known
				 * Conf-Nak, suggest hisaddr
				 */
				type = CONF_NAK;
				sppp_suggest_ip6_addr(sp, &suggestaddr);
				bcopy(&suggestaddr.s6_addr[8], &p[2], 8);
			}
			if (debug)
				addlog(" %s [%s]",
				    ip6_sprintf(ip6buf, &desiredaddr),
				    sppp_cp_type_name(type));
			break;
		}
		/* Add the option to nak'ed list. */
		bcopy (p, r, p[1]);
		r += p[1];
		rlen += p[1];
	}

	if (rlen == 0 && type == CONF_ACK) {
		if (debug)
			addlog(" send %s\n", sppp_cp_type_name(type));
		// Kaohj --- it's acked, so save hisaddr.
		bcopy(&desiredaddr, sp->ipv6cp.hisip6addr, sizeof(sp->ipv6cp.hisip6addr));
		sppp_cp_send (sp, PPP_IPV6CP, type, h->ident, origlen, h+1);
	} else {
#ifdef DIAGNOSTIC
		if (type == CONF_ACK)
			panic("IPv6CP RCR: CONF_ACK with non-zero rlen");
#endif

		if (debug) {
			addlog(" send %s suggest %s\n",
			    sppp_cp_type_name(type),
			    ip6_sprintf(ip6buf, &suggestaddr));
		}
		sppp_cp_send (sp, PPP_IPV6CP, type, h->ident, rlen, buf);
	}

 end:
	free (buf);
	return (rlen == 0);
}

/*
 * Analyze the IPv6CP Configure-Reject option list, and adjust our
 * negotiation.
 */
static void
sppp_ipv6cp_RCN_rej(struct sppp *sp, struct lcp_header *h, int len)
{
	STDDCL;
	u_char *buf, *p;
	//struct ifnet *ifp = SP2IFP(sp);
	//int debug = ifp->if_flags & IFF_DEBUG;

	len -= 4;
	buf = malloc (len);
	if (!buf)
		return;

	if (debug)
		addlog(SPP_FMT "ipv6cp rej opts:",
		    SPP_ARGS(sp));

	p = (void*) (h+1);
	for (; len >= 2 && p[1] >= 2 && len >= p[1];
	    len -= p[1], p += p[1]) {
		if (debug)
			addlog(" %s", sppp_ipv6cp_opt_name(*p));
		switch (*p) {
		case IPV6CP_OPT_IFID:
			/*
			 * Peer doesn't grok address option.  This is
			 * bad.  XXX  Should we better give up here?
			 */
			sp->ipv6cp.opts &= ~(1 << IPV6CP_OPT_IFID);
			break;
#ifdef notyet
		case IPV6CP_OPT_COMPRESSION:
			sp->ipv6cp.opts &= ~(1 << IPV6CP_OPT_COMPRESSION);
			break;
#endif
		}
	}
	if (debug)
		addlog("\n");
	free (buf);
	return;
}

/*
 * Analyze the IPv6CP Configure-NAK option list, and adjust our
 * negotiation.
 */
static void
sppp_ipv6cp_RCN_nak(struct sppp *sp, struct lcp_header *h, int len)
{
	STDDCL;
	u_char *buf, *p;
	//struct ifnet *ifp = SP2IFP(sp);
	//int debug = ifp->if_flags & IFF_DEBUG;
	struct in6_addr suggestaddr;
	char ip6buf[INET6_ADDRSTRLEN];

	len -= 4;
	buf = malloc (len);
	if (!buf)
		return;

	if (debug)
		addlog(SPP_FMT "ipv6cp nak opts:",
		    SPP_ARGS(sp));

	p = (void*) (h+1);
	for (; len >= 2 && p[1] >= 2 && len >= p[1];
	    len -= p[1], p += p[1]) {
		if (debug)
			addlog(" %s", sppp_ipv6cp_opt_name(*p));
		switch (*p) {
		case IPV6CP_OPT_IFID:
			/*
			 * Peer doesn't like our local ifid.  See
			 * if we can do something for him.  We'll drop
			 * him our address then.
			 */
			if (len < 10 || p[1] != 10)
				break;
			bzero(&suggestaddr, sizeof(suggestaddr));
			suggestaddr.s6_addr16[0] = htons(0xfe80);
			//(void)in6_setscope(&suggestaddr, SP2IFP(sp), NULL);
			bcopy(&p[2], &suggestaddr.s6_addr[8], 8);

			sp->ipv6cp.opts |= (1 << IPV6CP_OPT_IFID);
			if (debug)
				addlog(" [suggestaddr %s]",
				       ip6_sprintf(ip6buf, &suggestaddr));
#ifdef IPV6CP_MYIFID_DYN
			/*
			 * When doing dynamic address assignment,
			 * we accept his offer.
			 */
			if (sp->ipv6cp.flags & IPV6CP_MYIFID_DYN) {
				struct in6_addr lastsuggest;
				/*
				 * If <suggested myaddr from peer> equals to
				 * <hisaddr we have suggested last time>,
				 * we have a collision.  generate new random
				 * ifid.
				 */
				sppp_suggest_ip6_addr(sp, &lastsuggest);
				if (IN6_ARE_ADDR_EQUAL(&suggestaddr,
						       &lastsuggest)) {
					if (debug)
						addlog(" [random]");
					sppp_gen_ip6_addr(sp, &suggestaddr);
				}
				sppp_set_ip6_addr(sp, &suggestaddr);
				if (debug)
					addlog(" [agree]");
				sp->ipv6cp.flags |= IPV6CP_MYIFID_SEEN;
			}
#else
			/*
			 * Since we do not do dynamic address assignment,
			 * we ignore it and thus continue to negotiate
			 * our already existing value.  This can possibly
			 * go into infinite request-reject loop.
			 *
			 * This is not likely because we normally use
			 * ifid based on MAC-address.
			 * If you have no ethernet card on the node, too bad.
			 * XXX should we use fail_counter?
			 */
#endif
			break;
#ifdef notyet
		case IPV6CP_OPT_COMPRESS:
			/*
			 * Peer wants different compression parameters.
			 */
			break;
#endif
		}
	}
	if (debug)
		addlog("\n");
	free (buf);
	return;
}
static void
sppp_ipv6cp_tlu(struct sppp *sp)
{
	char ext_ifname[32];
	
	/* we are up - notify isdn daemon */
	if (sp->pp_con)
		sp->pp_con(sp, IDX_IPV6CP);
		
	if ((sp->ipType == 1) &&sp->over == SPPP_PPPOE)	// IPv6 only and PPPoE,since if this connect is pure IPv6,
	{                                               // also need to store PPPoE session ID into MIB Table.
		PPPOE_DRV_CTRL * p = (PPPOE_DRV_CTRL *)sp->pp_lowerp;
		PPPOE_SESSION_INFO p_session_info;
		p_session_info.uifno = p->uifno;
		memcpy(&p_session_info.remote,(void *) &p->remote, sizeof(struct sockaddr_ll));
		p_session_info.session= p->session;
		mib_set_PPPoE(CMD_MIB_SAVE_PPPOE, &p_session_info, sizeof(PPPOE_SESSION_INFO));
	}

	// Mason Yu--- run ifup script	
	snprintf(ext_ifname, 32, "/var/ppp/ifupv6_%s", sp->if_name);
	va_cmd(ext_ifname, 0, 1);
}

static void
sppp_ipv6cp_tld(struct sppp *sp)
{
}

static void
sppp_ipv6cp_tls(struct sppp *sp)
{
	/* indicate to LCP that it must stay alive */
	sp->lcp.protos |= (1 << IDX_IPV6CP);
}

static void
sppp_ipv6cp_tlf(struct sppp *sp)
{

#if 0	/* need #if 0 to close IPv6CP properly */
	/* we no longer need LCP */
	sp->lcp.protos &= ~(1 << IDX_IPV6CP);
	sppp_lcp_check_and_close(sp);
#endif
}

static void
sppp_ipv6cp_scr(struct sppp *sp)
{
	char opt[10 /* ifid */ + 4 /* compression, minimum */];
	struct in6_addr ouraddr;
	int i = 0;

	if (sp->ipv6cp.opts & (1 << IPV6CP_OPT_IFID)) {
		sppp_get_ip6_addrs(sp, &ouraddr, 0, 0);
		sppp_set_ip6_addr(sp, &ouraddr);
		opt[i++] = IPV6CP_OPT_IFID;
		opt[i++] = 10;
		bcopy(&ouraddr.s6_addr[8], &opt[i], 8);
		i += 8;
	}

#ifdef notyet
	if (sp->ipv6cp.opts & (1 << IPV6CP_OPT_COMPRESSION)) {
		opt[i++] = IPV6CP_OPT_COMPRESSION;
		opt[i++] = 4;
		opt[i++] = 0;   /* TBD */
		opt[i++] = 0;   /* TBD */
		/* variable length data may follow */
	}
#endif

	sp->confid[IDX_IPV6CP] = ++sp->pp_seq[IDX_IPV6CP];
	sppp_cp_send(sp, PPP_IPV6CP, CONF_REQ, sp->confid[IDX_IPV6CP], i, &opt);
}
#else // CONFIG_IPV6
static void sppp_ipv6cp_init(struct sppp *sp)
{
}

static void sppp_ipv6cp_up(struct sppp *sp)
{
}

static void sppp_ipv6cp_down(struct sppp *sp)
{
}


static void sppp_ipv6cp_open(struct sppp *sp)
{
}

static void sppp_ipv6cp_close(struct sppp *sp)
{
}

static void sppp_ipv6cp_TO(void *sp)
{
}

static int sppp_ipv6cp_RCR(struct sppp *sp, struct lcp_header *h, int len)
{
	return 0;
}

static void sppp_ipv6cp_RCN_rej(struct sppp *sp, struct lcp_header *h, int len)
{
}

static void sppp_ipv6cp_RCN_nak(struct sppp *sp, struct lcp_header *h, int len)
{
}

static void sppp_ipv6cp_tlu(struct sppp *sp)
{
}

static void sppp_ipv6cp_tld(struct sppp *sp)
{
}

static void sppp_ipv6cp_tls(struct sppp *sp)
{
}

static void sppp_ipv6cp_tlf(struct sppp *sp)
{
}

static void sppp_ipv6cp_scr(struct sppp *sp)
{
}
#endif // of CONFIG_IPV6

/*
 *--------------------------------------------------------------------------*
 *                                                                          *
 *                        The CHAP implementation.                          *
 *                                                                          *
 *--------------------------------------------------------------------------*
 */

/*
 * The authentication protocols don't employ a full-fledged state machine as
 * the control protocols do, since they do have Open and Close events, but
 * not Up and Down, nor are they explicitly terminated.  Also, use of the
 * authentication protocols may be different in both directions (this makes
 * sense, think of a machine that never accepts incoming calls but only
 * calls out, it doesn't require the called party to authenticate itself).
 *
 * Our state machine for the local authentication protocol (we are requesting
 * the peer to authenticate) looks like:
 *
 *						    RCA-
 *	      +--------------------------------------------+
 *	      V					    scn,tld|
 *	  +--------+			       Close   +---------+ RCA+
 *	  |	   |<----------------------------------|	 |------+
 *   +--->| Closed |				TO*    | Opened	 | sca	|
 *   |	  |	   |-----+		       +-------|	 |<-----+
 *   |	  +--------+ irc |		       |       +---------+
 *   |	    ^		 |		       |	   ^
 *   |	    |		 |		       |	   |
 *   |	    |		 |		       |	   |
 *   |	 TO-|		 |		       |	   |
 *   |	    |tld  TO+	 V		       |	   |
 *   |	    |	+------->+		       |	   |
 *   |	    |	|	 |		       |	   |
 *   |	  +--------+	 V		       |	   |
 *   |	  |	   |<----+<--------------------+	   |
 *   |	  | Req-   | scr				   |
 *   |	  | Sent   |					   |
 *   |	  |	   |					   |
 *   |	  +--------+					   |
 *   | RCA- |	| RCA+					   |
 *   +------+	+------------------------------------------+
 *   scn,tld	  sca,irc,ict,tlu
 *
 *
 *   with:
 *
 *	Open:	LCP reached authentication phase
 *	Close:	LCP reached terminate phase
 *
 *	RCA+:	received reply (pap-req, chap-response), acceptable
 *	RCN:	received reply (pap-req, chap-response), not acceptable
 *	TO+:	timeout with restart counter >= 0
 *	TO-:	timeout with restart counter < 0
 *	TO*:	reschedule timeout for CHAP
 *
 *	scr:	send request packet (none for PAP, chap-challenge)
 *	sca:	send ack packet (pap-ack, chap-success)
 *	scn:	send nak packet (pap-nak, chap-failure)
 *	ict:	initialize re-challenge timer (CHAP only)
 *
 *	tlu:	this-layer-up, LCP reaches network phase
 *	tld:	this-layer-down, LCP enters terminate phase
 *
 * Note that in CHAP mode, after sending a new challenge, while the state
 * automaton falls back into Req-Sent state, it doesn't signal a tld
 * event to LCP, so LCP remains in network phase.  Only after not getting
 * any response (or after getting an unacceptable response), CHAP closes,
 * causing LCP to enter terminate phase.
 *
 * With PAP, there is no initial request that can be sent.  The peer is
 * expected to send one based on the successful negotiation of PAP as
 * the authentication protocol during the LCP option negotiation.
 *
 * Incoming authentication protocol requests (remote requests
 * authentication, we are peer) don't employ a state machine at all,
 * they are simply answered.  Some peers [Ascend P50 firmware rev
 * 4.50] react allergically when sending IPCP requests while they are
 * still in authentication phase (thereby violating the standard that
 * demands that these NCP packets are to be discarded), so we keep
 * track of the peer demanding us to authenticate, and only proceed to
 * phase network once we've seen a positive acknowledge for the
 * authentication.
 */


/*
 * Handle incoming CHAP packets.
 */
void
sppp_chap_input(struct sppp *sp, void *data, int len)
{
	STDDCL;
	struct lcp_header *h;
	int account_idx = 0;
	int retval;
	//int x;
	//u_char *value, *name, digest[AUTHKEYLEN], dsize;
	u_char *value, *name, digest[16], dsize;
	int value_len, name_len;
	MD5_CTX ctx;
	char bytes[256], string[256];
	bytes[0] = string[0] = 0;

	if (len < 4) {
		if (debug)
			addlog(SPP_FMT "chap invalid packet length: %d bytes\n",
			    SPP_ARGS(sp), len);
		sppp_auth_log(sp->if_unit, 3);	// Jenny, for failure authentication log
		sppp_last_connection_error(sp->if_unit, ERROR_AUTHENTICATION_FAILURE);
		return;
	}
	h = (struct lcp_header*)data;
	if (len > ntohs (h->len))
		len = ntohs (h->len);

	switch (h->type) {
	/* challenge, failure and success are his authproto */
	case CHAP_CHALLENGE:
		value = 1 + (u_char*)(h+1);
		value_len = value[-1];
		name = value + value_len;
		name_len = len - value_len - 5;
		if (name_len < 0) {
			if (debug) {
				/*addlog(SPP_FMT "chap corrupted challenge "
				    "<%s id=0x%x len=%d ",
				    SPP_ARGS(sp),
				    sppp_auth_type_name(PPP_CHAP, h->type),
				    h->ident, ntohs(h->len));
				sppp_print_bytes((u_char*) (h+1), len-4);
				addlog(">\n");*/
				sppp_bytes(bytes, (u_char*)(h+1), len-4);
				addlog(SPP_FMT "chap corrupted challenge "
				    "<%s id=0x%x len=%d %s>\n",
				    SPP_ARGS(sp),
				    sppp_auth_type_name(PPP_CHAP, h->type),
				    h->ident, ntohs(h->len), bytes);
			}
			sppp_auth_log(sp->if_unit, 3);	// Jenny, for failure authentication log
			sppp_last_connection_error(sp->if_unit, ERROR_AUTHENTICATION_FAILURE);
			break;
		}

		if (debug) {
			/*addlog(SPP_FMT "chap input <%s id=0x%x len=%d name=",
			    SPP_ARGS(sp),
			    sppp_auth_type_name(PPP_CHAP, h->type), h->ident,
			    ntohs(h->len));
			sppp_print_string((char*) name, name_len);
			addlog(" value-size=%d value=", value_len);
			sppp_print_bytes(value, value_len);
			addlog(">\n");*/
			sppp_string(string, (char*)name, name_len);
			sppp_bytes(bytes, value, value_len);
			addlog(SPP_FMT "chap input <%s [%s] id=0x%x len=%d name=%s value-size=%d value=%s>\n",
			    SPP_ARGS(sp),
			    sppp_auth_type_name(PPP_CHAP, h->type), sppp_state_name(sp->state[IDX_CHAP]), h->ident,
			    ntohs(h->len), string, value_len, bytes);
		} 

		sp->myauth.chal_id = h->ident;
		sp->myauth.chal_transmits = 0;

		if(sp->myauth.chal_type == CHAP_MICROSOFT) {
			int name_len;
			u_char md_len;
#ifdef CONFIG_PPP_MPPE_MPPC
			if(sp->myauth.enc_type) {				 
				ccp_flags_set(sp->dev_fd, 1, 1);
			}
#endif
			ChapMS(sp, value, value_len, sp->myauth.secret, sppp_strnlen(sp->myauth.secret, AUTHKEYLEN));
			
			md_len = sp->hisauth.chal_len;
			name_len = strlen(sp->myauth.name);    
			sppp_cp_change_state(&chap, sp, STATE_ACK_SENT);
			sppp_auth_send(&chap, sp, CHAP_RESPONSE, h->ident,
				/*	sizeof outlen, (const char *)&outlen,*/
				sizeof md_len, (const char *)&md_len,
				(size_t)sp->hisauth.chal_len,
				sp->hisauth.challenge,
				(size_t)name_len,sp->myauth.name,
				0); 
		} else
		if(sp->myauth.chal_type == CHAP_MICROSOFT_V2)
		{
			int name_len;
			u_char md_len;
#ifdef CONFIG_PPP_MPPE_MPPC
			if(sp->myauth.enc_type) {                
				ccp_flags_set(sp->dev_fd, 1, 1);
			}
#endif
			ChapMS_v2(sp, value, value_len, sp->myauth.secret, sppp_strnlen(sp->myauth.secret, AUTHKEYLEN));

			md_len = sp->hisauth.chal_len;
			name_len = strlen(sp->myauth.name);    
			sppp_cp_change_state(&chap, sp, STATE_ACK_SENT);
			sppp_auth_send(&chap, sp, CHAP_RESPONSE, h->ident,
				/*  sizeof outlen, (const char *)&outlen,*/
				sizeof md_len, (const char *)&md_len,
				(size_t)sp->hisauth.chal_len,
				sp->hisauth.challenge,
				(size_t)name_len,sp->myauth.name,
				0); 
		}
		else
		{
			/* Compute reply value. */
			MD5Init(&ctx);
			MD5Update(&ctx, &h->ident, 1);
			MD5Update(&ctx, sp->myauth.secret,sppp_strnlen(sp->myauth.secret, AUTHKEYLEN));
			MD5Update(&ctx, value, value_len);
			MD5Final(digest, &ctx);
			dsize = sizeof digest;

			sppp_cp_change_state(&chap, sp, STATE_ACK_SENT);
			sppp_auth_send(&chap, sp, CHAP_RESPONSE, h->ident,
				       sizeof dsize, (const char *)&dsize,
				       sizeof digest, digest,
				       (size_t)sppp_strnlen(sp->myauth.name, AUTHNAMELEN),
				       sp->myauth.name,
				       0);
		}
		TIMEOUT(sppp_chap_my_TO, (void *)sp, sp->lcp.timeout, sp->chap_my_to_ch);
		break;

	case CHAP_SUCCESS:       
		UNTIMEOUT(sppp_chap_my_TO, (void *)sp, sp->chap_my_to_ch);
		if (debug) {
			/*addlog(SPP_FMT "chap success",
			    SPP_ARGS(sp));
			if (len > 4) {
				addlog(": ");
				sppp_print_string((char*)(h + 1), len - 4);
			}
			addlog("\n");*/
			sprintf(string, ": [%s] ", sppp_state_name(sp->state[IDX_CHAP]));
			sppp_string(string, (char*)(h + 1), len - 4);
			addlog(SPP_FMT "chap success%s\n",
			    SPP_ARGS(sp), (len > 4)?string:"");
		}
		syslog(LOG_INFO, "spppd: %s: Authentication(CHAP) success\n", sp->if_name);
		//x = splimp();
/*		sppp_auth_log(sp->if_unit, 1);	// Jenny, for success authentication log
#ifdef DIAGNOSTIC_TEST
		sppp_diag_log(sp->if_unit, 2);	// Jenny, diagnostic test, authentication with ISP pass
#endif*/
		sp->pp_flags &= ~PP_NEEDAUTH;
		if (sp->myauth.proto == PPP_CHAP &&
		    (sp->lcp.opts & (1 << LCP_OPT_AUTH_PROTO)) &&
		    (sp->lcp.protos & (1 << IDX_CHAP)) == 0) {
			/*
			 * We are authenticator for CHAP but didn't
			 * complete yet.  Leave it to tlu to proceed
			 * to network phase.
			 */
			//splx(x);
			sppp_auth_log(sp->if_unit, 3);	// Jenny, for failure authentication log
			sppp_last_connection_error(sp->if_unit, ERROR_AUTHENTICATION_FAILURE);
			break;
		}
		//splx(x);
		sppp_phase_network(sp);
		break;

	case CHAP_FAILURE:
		UNTIMEOUT(sppp_chap_my_TO, (void *)sp, sp->chap_my_to_ch);
		if (debug) {
			/*addlog(SPP_FMT "chap failure",
			    SPP_ARGS(sp));
			if (len > 4) {
				addlog(": ");
				sppp_print_string((char*)(h + 1), len - 4);
			}
			addlog("\n");*/
			sprintf(string, ": [%s]", sppp_state_name(sp->state[IDX_CHAP]));
			sppp_string(string, (char*)(h + 1), len - 4);
			addlog(SPP_FMT "chap failure%s\n",
			    SPP_ARGS(sp), (len > 4)?string:"");
		} else
			addlog(SPP_FMT "chap failure\n",
			    SPP_ARGS(sp));
		//add by ramen to set the sp->myauth.proto to the default value
//		printf("\033[0;31m set sp->myauth.proto from %s to %s\n\033[0m", sppp_proto_name(sp->myauth.proto), sppp_proto_name(sp->myauth.aumode));
		sp->myauth.proto = sp->myauth.aumode;	// Jenny
		sppp_auth_log(sp->if_unit, 3);	// Jenny, for failure authentication log
		sppp_last_connection_error(sp->if_unit, ERROR_AUTHENTICATION_FAILURE);
		/* await LCP shutdown by authenticator */
		break;

	/* response is my authproto */
	case CHAP_RESPONSE:
		value = 1 + (u_char*)(h+1);
		value_len = value[-1];
		name = value + value_len;
		name_len = len - value_len - 5;
		if (name_len < 0) {
			if (debug) {
				/*addlog(SPP_FMT "chap corrupted response "
				    "<%s id=0x%x len=%d ",
				    SPP_ARGS(sp),
				    sppp_auth_type_name(PPP_CHAP, h->type),
				    h->ident, ntohs(h->len));
				sppp_print_bytes((u_char*)(h+1), len-4);
				addlog(">\n");*/
				sppp_bytes(bytes, (u_char*)(h+1), len-4);
				addlog(SPP_FMT "chap corrupted response "
				    "<%s id=0x%x len=%d %s>\n",
				    SPP_ARGS(sp),
				    sppp_auth_type_name(PPP_CHAP, h->type),
				    h->ident, ntohs(h->len), bytes);
			}
			sppp_auth_log(sp->if_unit, 3);	// Jenny, for failure authentication log
			sppp_last_connection_error(sp->if_unit, ERROR_AUTHENTICATION_FAILURE);
			break;
		}
		if (h->ident != sp->confid[IDX_CHAP]) {
			if (debug)
				addlog(SPP_FMT "chap dropping response for old ID "
				    "(got %d, expected %d)\n",
				    SPP_ARGS(sp),
				    h->ident, sp->confid[IDX_CHAP]);
			sppp_auth_log(sp->if_unit, 3);	// Jenny, for failure authentication log
			sppp_last_connection_error(sp->if_unit, ERROR_AUTHENTICATION_FAILURE);
			break;
		}

#if defined(CONFIG_USER_PPTPD_PPTPD) || defined(CONFIG_USER_L2TPD_LNS)
		if(sp->over == SPPP_PPTPD || sp->over == SPPP_L2TPD){
ACCOUNT_CHECK:
			retval = get_pppd_account_by_idx(sp, account_idx);
			if (0 == retval) {
				account_idx++;
				goto ACCOUNT_CHECK;
			}
			else if (retval < 0) {
				if (CHAP_MICROSOFT_V2 == sp->myauth.chal_type)
				{
					char *s;
					int i;
					memset(sp->hisauth.challenge, 0, AUTHKEYLEN);
					s = stpcpy(sp->hisauth.challenge, "E=691 R=0 C=");
					for (i=0; i<16; i++) {
						sprintf(s, "%x%x", sp->myauth.challenge[i]>>4,sp->myauth.challenge[i]&0xF);
						s += 2;
					}
					strcpy(s, " V=3 M=Access denied");
					sp->hisauth.chal_len = strlen(sp->hisauth.challenge);

					sppp_auth_send(&chap, sp, CHAP_FAILURE, h->ident,
							   sp->hisauth.chal_len, sp->hisauth.challenge,
							   0);
					chap.tld(sp);
					break;
				}
				else
				{
					/* action scn, tld */
					sppp_auth_send(&chap, sp, CHAP_FAILURE, h->ident,
							   sizeof(FAILMSG) - 1, (u_char *)FAILMSG,
							   0);
					chap.tld(sp);
					break;
				}
			}
			
			if (name_len != sppp_strnlen(sp->hisauth.name, AUTHNAMELEN)
			    || bcmp(name, sp->hisauth.name, name_len) != 0) {
			    #if 0
				/*addlog(SPP_FMT "chap response, his name ",
				    SPP_ARGS(sp));
				sppp_print_string(name, name_len);
				addlog(" != expected ");
				sppp_print_string(sp->hisauth.name,
						  sppp_strnlen(sp->hisauth.name, AUTHNAMELEN));
				addlog("\n");*/
				char str[256];
				str[0] = 0;
				sppp_string(string, name, name_len);
				sppp_string(str, sp->hisauth.name, sppp_strnlen(sp->hisauth.name, AUTHNAMELEN));
				addlog(SPP_FMT "chap response, his name %s != expected %s\n",
				    SPP_ARGS(sp), string, str);
				sppp_auth_log(sp->if_unit, 3);	// Jenny, for failure authentication log
				sppp_last_connection_error(sp->if_unit, ERROR_AUTHENTICATION_FAILURE);
				#else
				account_idx++;
				goto ACCOUNT_CHECK;
				#endif
			}
			if (debug) {
				/*addlog(SPP_FMT "CHAP: I %s [%s] id=0x%x len=%d name=",
				    SPP_ARGS(sp),
				    sppp_auth_type_name(PPP_CHAP, h->type),
				    sppp_state_name(sp->state[IDX_CHAP]),
				    h->ident, ntohs (h->len));
				sppp_print_string((char*)name, name_len);
				addlog(" value-size=%d value=", value_len);
				sppp_print_bytes(value, value_len);
				addlog("\n");*/
				sppp_string(string, (char*)name, name_len);
				sppp_bytes(bytes, value, value_len);
				addlog(SPP_FMT "CHAP: I %s [%s] id=0x%x len=%d name=%s value-size=%d value=%s\n",
				    SPP_ARGS(sp),
				    sppp_auth_type_name(PPP_CHAP, h->type),
				    sppp_state_name(sp->state[IDX_CHAP]),
				    h->ident, ntohs (h->len), string, value_len, bytes);
			}
			#if 0
			if (value_len != AUTHKEYLEN) {
				if (debug)
					addlog(SPP_FMT "chap bad hash value length: "
					    "%d bytes, should be %d\n",
					    SPP_ARGS(sp), value_len,
					    AUTHKEYLEN);
				sppp_auth_log(sp->if_unit, 3);	// Jenny, for failure authentication log
				sppp_last_connection_error(sp->if_unit, ERROR_AUTHENTICATION_FAILURE);
				break;
			}
			#endif

#if 1//def CHAPMS
			if(sp->myauth.chal_type == CHAP_MICROSOFT_V2)
			{
				//printf("%s value_len=%d\n", __func__, value_len);
				if(value_len != MS_CHAP_RESPONSE_LEN)
					break;
		        if(ChapMS_v2_Resp(sp,sp->hisauth.secret,sppp_strnlen(sp->hisauth.secret, AUTHKEYLEN),value,sp->hisauth.name) == 0)
			    {
			    	//code = CHAP_SUCCESS_R;
		    		ChapMS_v2_Auth(sp, sp->hisauth.secret,sppp_strnlen(sp->hisauth.secret, AUTHKEYLEN), value, sp->hisauth.name);
#ifdef CONFIG_PPP_MPPE_MPPC        
					if(sp->myauth.enc_type) {				 
						ccp_flags_set(sp->dev_fd, 1, 1);
					}
#endif        
			    }
				else {
					printf("%s account %s/%s emit.\n", __func__, sp->hisauth.name, sp->hisauth.secret);
					account_idx++;
					goto ACCOUNT_CHECK;
				}
			}
			else
#endif
			{
				MD5Init(&ctx);
				MD5Update(&ctx, &h->ident, 1);
				MD5Update(&ctx, sp->hisauth.secret,sppp_strnlen(sp->hisauth.secret, AUTHKEYLEN));
				MD5Update(&ctx, sp->myauth.challenge, AUTHKEYLEN);
				MD5Final(digest, &ctx);

				if (value_len != sizeof digest ||
				    bcmp(digest, value, value_len) != 0) {
				    #if 0
					/* action scn, tld */
					sppp_auth_send(&chap, sp, CHAP_FAILURE, h->ident,
						       sizeof(FAILMSG) - 1, (u_char *)FAILMSG,
						       0);
					chap.tld(sp);
					break;
					#else
					account_idx++;
					goto ACCOUNT_CHECK;
					#endif
				}
			}
		}
#endif

		/* action sca, perhaps tlu */
		if (sp->state[IDX_CHAP] == STATE_REQ_SENT ||
		    sp->state[IDX_CHAP] == STATE_OPENED)
		    if (CHAP_MICROSOFT_V2 == sp->myauth.chal_type)
	    	{
	    		sppp_auth_send(&chap, sp, CHAP_SUCCESS, h->ident,
					       sp->hisauth.chal_len, sp->hisauth.challenge,
					       0);
	    	}
			else {
				sppp_auth_send(&chap, sp, CHAP_SUCCESS, h->ident,
					       sizeof(SUCCMSG) - 1, (u_char *)SUCCMSG,
					       0);
			}
		if (sp->state[IDX_CHAP] == STATE_REQ_SENT) {
			sppp_cp_change_state(&chap, sp, STATE_OPENED);
			chap.tlu(sp);
		} 
		
		break;

	default:
		/* Unknown CHAP packet type -- ignore. */
		if (debug) {
			/*addlog(SPP_FMT "chap unknown input(%s) "
			    "<0x%x id=0x%xh len=%d ",
			    SPP_ARGS(sp),
			    sppp_state_name(sp->state[IDX_CHAP]),
			    h->type, h->ident, ntohs(h->len));
			sppp_print_bytes((u_char*)(h+1), len-4);
			addlog(">\n");*/
			sppp_bytes(bytes, (u_char*)(h+1), len-4);
			addlog(SPP_FMT "chap unknown input(%s) "
			    "<0x%x id=0x%xh len=%d %s>\n",
			    SPP_ARGS(sp),
			    sppp_state_name(sp->state[IDX_CHAP]),
			    h->type, h->ident, ntohs(h->len), bytes);
		}
		break;

	}
}

static void
sppp_chap_init(struct sppp *sp)
{
	/* Chap doesn't have STATE_INITIAL at all. */
	sp->state[IDX_CHAP] = STATE_CLOSED;
	sp->fail_counter[IDX_CHAP] = 0;
	sp->pp_seq[IDX_CHAP] = 0;
	sp->pp_rseq[IDX_CHAP] = 0;
	sppp_callout_handle_init(&sp->ch[IDX_CHAP]);
	sppp_callout_handle_init(&sp->chap_my_to_ch);
}

static void 
sppp_chap_up(struct sppp *sp)
{
	if (sp->myauth.proto == PPP_CHAP &&
	    (sp->lcp.opts & (1 << LCP_OPT_AUTH_PROTO)) != 0) {
		/* we are authenticator for CHAP, start it */
		chap.scr(sp);
		sp->rst_counter[IDX_CHAP] = sp->lcp.max_configure;
		sppp_cp_change_state(&chap, sp, STATE_REQ_SENT);
	}
}

static void
sppp_chap_open(struct sppp *sp)
{
	if (sp->myauth.proto == PPP_CHAP &&
	    (sp->lcp.opts & (1 << LCP_OPT_AUTH_PROTO)) != 0) {
		/* we are authenticator for CHAP, start it */
		chap.scr(sp);
		sp->rst_counter[IDX_CHAP] = sp->lcp.max_configure;
		sppp_cp_change_state(&chap, sp, STATE_REQ_SENT);
	}
	/* nothing to be done if we are peer, await a challenge */
}

static void
sppp_chap_close(struct sppp *sp)
{
	if (sp->state[IDX_CHAP] != STATE_CLOSED)
		sppp_cp_change_state(&chap, sp, STATE_CLOSED);
}

static void
sppp_chap_TO(void *cookie)
{
	struct sppp *sp = (struct sppp *)cookie;
	STDDCL;
	int s;

	if (debug)
		addlog(SPP_FMT "chap TO(%s) rst_counter = %d\n",
		    SPP_ARGS(sp),
		    sppp_state_name(sp->state[IDX_CHAP]),
		    sp->rst_counter[IDX_CHAP]);

	sppp_auth_log(sp->if_unit, 3);	// Jenny, for failure authentication log
	sppp_last_connection_error(sp->if_unit, ERROR_AUTHENTICATION_FAILURE);
	if (--sp->rst_counter[IDX_CHAP] < 0)
		/* TO- event */
		switch (sp->state[IDX_CHAP]) {
		case STATE_REQ_SENT:
			chap.tld(sp);
			sppp_cp_change_state(&chap, sp, STATE_CLOSED);
			break;
		}
	else
		/* TO+ (or TO*) event */
		switch (sp->state[IDX_CHAP]) {
		case STATE_OPENED:
			/* TO* event */
			sp->rst_counter[IDX_CHAP] = sp->lcp.max_configure;
			/* fall through */
		case STATE_REQ_SENT:
			chap.scr(sp);
			/* sppp_cp_change_state() will restart the timer */
			sppp_cp_change_state(&chap, sp, STATE_REQ_SENT);
			break;
		}

}

static void
sppp_chap_my_TO(void *cookie)
{
	struct sppp *sp = (struct sppp *)cookie;
	STDDCL;

	if (debug)
		addlog(SPP_FMT "chap TO\n", SPP_ARGS(sp));

	sppp_auth_log(sp->if_unit, 3);	// Jenny, for failure authentication log
	sppp_last_connection_error(sp->if_unit, ERROR_AUTHENTICATION_FAILURE);
}

static void
sppp_chap_tlu(struct sppp *sp)
{
	STDDCL;
	int i, x;

	i = 0;
	sp->rst_counter[IDX_CHAP] = sp->lcp.max_configure;

	/*
	 * Some broken CHAP implementations (Conware CoNet, firmware
	 * 4.0.?) don't want to re-authenticate their CHAP once the
	 * initial challenge-response exchange has taken place.
	 * Provide for an option to avoid rechallenges.
	 */
	if ((sp->hisauth.flags & AUTHFLAG_NORECHALLENGE) == 0) {
		/*
		 * Compute the re-challenge timeout.  This will yield
		 * a number between 300 and 810 seconds.
		 */
		i = 300 + ((unsigned)(rand() & 0xff00) >> 7);
		TIMEOUT(chap.TO, (void *)sp, i * hz, sp->ch[IDX_CHAP]);
	}

	if (debug) {
		addlog(SPP_FMT "chap %s, ",
		    SPP_ARGS(sp),
		    sp->pp_phase == PHASE_NETWORK? "reconfirmed": "tlu");
		if ((sp->hisauth.flags & AUTHFLAG_NORECHALLENGE) == 0)
			addlog("next re-challenge in %d seconds\n", i);
		else
			addlog("re-challenging supressed\n");
	}

	/* indicate to LCP that we need to be closed down */
	sp->lcp.protos |= (1 << IDX_CHAP);

	if (sp->pp_flags & PP_NEEDAUTH) {
		/*
		 * Remote is authenticator, but his auth proto didn't
		 * complete yet.  Defer the transition to network
		 * phase.
		 */
		//splx(x);
		return;
	}

	/*
	 * If we are already in phase network, we are done here.  This
	 * is the case if this is a dummy tlu event after a re-challenge.
	 */
	if (sp->pp_phase != PHASE_NETWORK)
		sppp_phase_network(sp);
}

static void
sppp_chap_tld(struct sppp *sp)
{
	STDDCL;

	if (debug)
		addlog(SPP_FMT "chap tld\n", SPP_ARGS(sp));
	sppp_auth_log(sp->if_unit, 3);	// Jenny, for failure authentication log
	sppp_last_connection_error(sp->if_unit, ERROR_AUTHENTICATION_FAILURE);
	UNTIMEOUT(chap.TO, (void *)sp, sp->ch[IDX_CHAP]);
	UNTIMEOUT(sppp_chap_my_TO, (void *)sp, sp->chap_my_to_ch);
	sp->lcp.protos &= ~(1 << IDX_CHAP);

	lcp.Close(sp);
}

static void
sppp_chap_scr(struct sppp *sp)
{
	u_long *ch, seed;
	u_char clen;

	/* Compute random challenge. */
	ch = (u_long *)sp->myauth.challenge;
	seed = rand();
	ch[0] = seed ^ rand();
	ch[1] = seed ^ rand();
	ch[2] = seed ^ rand();
	ch[3] = seed ^ rand();
	
	if (CHAP_MICROSOFT_V2 == sp->myauth.chal_type)
	{
		clen = sp->myauth.chal_len = 16;

		sp->confid[IDX_CHAP] = ++sp->pp_seq[IDX_CHAP];

		sppp_cp_change_state(&chap, sp, STATE_ACK_SENT);
		//printf("%s chap CONF_REQ name %s!\n", __func__, sp->myauth.name);
		sppp_auth_send(&chap, sp, CHAP_CHALLENGE, sp->confid[IDX_CHAP],
			       sizeof clen, (const char *)&clen,
			       sp->myauth.chal_len, sp->myauth.challenge,
			       (size_t)sppp_strnlen(sp->myauth.name, AUTHNAMELEN),
			       sp->myauth.name,
			       0);
		return;
		
	}
	clen = AUTHKEYLEN;

	sp->confid[IDX_CHAP] = ++sp->pp_seq[IDX_CHAP];

	sppp_cp_change_state(&chap, sp, STATE_ACK_SENT);
	//printf("%s chap CONF_REQ name %s!\n", __func__, sp->myauth.name);
	sppp_auth_send(&chap, sp, CHAP_CHALLENGE, sp->confid[IDX_CHAP],
		       sizeof clen, (const char *)&clen,
		       (size_t)AUTHKEYLEN, sp->myauth.challenge,
		       (size_t)sppp_strnlen(sp->myauth.name, AUTHNAMELEN),
		       sp->myauth.name,
		       0);
}

/*
 *--------------------------------------------------------------------------*
 *                                                                          *
 *                        The PAP implementation.                           *
 *                                                                          *
 *--------------------------------------------------------------------------*
 */
/*
 * For PAP, we need to keep a little state also if we are the peer, not the
 * authenticator.  This is since we don't get a request to authenticate, but
 * have to repeatedly authenticate ourself until we got a response (or the
 * retry counter is expired).
 */

/*
 * Handle incoming PAP packets.  */
static void
sppp_pap_input(struct sppp *sp, void *data, int len)
{
	STDDCL;
	struct lcp_header *h;
	int x;
	u_char *name, *passwd, mlen;
	int name_len, passwd_len;
	char bytes[256], string[256];
	bytes[0] = string[0] = 0;

	if (len < 5) {
		if (debug)
			addlog(SPP_FMT "pap invalid packet length: %d bytes\n",
			    SPP_ARGS(sp), len);
		sppp_auth_log(sp->if_unit, 3);	// Jenny, for failure authentication log
		sppp_last_connection_error(sp->if_unit, ERROR_AUTHENTICATION_FAILURE);
		return;
	}
	h = (struct lcp_header*)data;
	if (len > ntohs (h->len))
		len = ntohs (h->len);
	sppp_bytes(bytes, (u_char*)(h+1), len-4);
	switch (h->type) {
	/* PAP request is my authproto */
	case PAP_REQ:
		name = 1 + (u_char*)(h+1);
		name_len = name[-1];
		passwd = name + name_len + 1;
		if (name_len > len - 6 ||
		    (passwd_len = passwd[-1]) > len - 6 - name_len) {
			if (debug) {
				/*addlog(SPP_FMT "pap corrupted input "
				    "<%s id=0x%x len=%d ",
				    SPP_ARGS(sp),
				    sppp_auth_type_name(PPP_PAP, h->type),
				    h->ident, ntohs(h->len));
				sppp_print_bytes((u_char*)(h+1), len-4);
				addlog(">\n");*/
				addlog(SPP_FMT "pap corrupted input "
				    "<%s id=0x%x len=%d %s>\n",
				    SPP_ARGS(sp),
				    sppp_auth_type_name(PPP_PAP, h->type),
				    h->ident, ntohs(h->len), bytes);
			}
			break;
		}
		if (debug) {
			/*addlog(SPP_FMT "PAP: I %s id=0x%x len=%d name=",
			    SPP_ARGS(sp),
			    sppp_auth_type_name(PPP_PAP, h->type),
			    //sppp_state_name(sp->state[IDX_PAP]),
			    h->ident, ntohs(h->len));
			sppp_print_string((char*)name, name_len);
			addlog(" passwd=");
			sppp_print_string((char*)passwd, passwd_len);
			addlog("\n");*/
			char str[256];
			str[0] = 0;
			sppp_string(string, (char*)name, name_len);
			sppp_string(str, (char*)passwd, passwd_len);
			addlog(SPP_FMT "PAP: I %s id=0x%x len=%d name=%s passwd=%s\n",
			    SPP_ARGS(sp),
			    sppp_auth_type_name(PPP_PAP, h->type),
			    h->ident, ntohs(h->len), string, str);
		}
		/* if we are server, then get an account for PAP check */
#if defined(CONFIG_USER_PPTPD_PPTPD) || defined(CONFIG_USER_L2TPD_LNS)
		if (1 == sp->role && sp->over != SPPP_PPPOE) {
			//printf("receive pap request.\n");
			if (get_pppd_account(sp, name, passwd, name_len, passwd_len) < 0)
			{
				mlen = sizeof(FAILMSG) - 1;
				sppp_auth_send(&pap, sp, PAP_NAK, h->ident,
					       sizeof mlen, (const char *)&mlen,
					       sizeof(FAILMSG) - 1, (u_char *)FAILMSG,
					       0);
				pap.tld(sp);
				break;
			}
			else {
				/* action sca, perhaps tlu */
				if (sp->state[IDX_PAP] == STATE_INITIAL) {
					mlen = sizeof(SUCCMSG) - 1;
					sppp_auth_send(&pap, sp, PAP_ACK, h->ident,
							   sizeof mlen, (const char *)&mlen,
							   sizeof(SUCCMSG) - 1, (u_char *)SUCCMSG,
							   0);
					sppp_cp_change_state(&pap, sp, STATE_OPENED);
					sp->pp_flags &= ~PP_NEEDAUTH;
					
					pap.tlu(sp);
				}
				
			}
		}
#endif
		if (name_len != sppp_strnlen(sp->hisauth.name, AUTHNAMELEN) ||
		    passwd_len != sppp_strnlen(sp->hisauth.secret, AUTHKEYLEN) ||
		    bcmp(name, sp->hisauth.name, name_len) != 0 ||
		    bcmp(passwd, sp->hisauth.secret, passwd_len) != 0) {
			/* action scn, tld */
			mlen = sizeof(FAILMSG) - 1;
			//printf("pap failed!\n");
			sppp_auth_send(&pap, sp, PAP_NAK, h->ident,
				       sizeof mlen, (const char *)&mlen,
				       sizeof(FAILMSG) - 1, (u_char *)FAILMSG,
				       0);
			pap.tld(sp);
			break;
		}
		/* action sca, perhaps tlu */
		if (sp->state[IDX_PAP] == STATE_REQ_SENT ||
		    sp->state[IDX_PAP] == STATE_OPENED ||
		    (sp->role ==1 && sp->state[IDX_PAP] == STATE_INITIAL)) {
			mlen = sizeof(SUCCMSG) - 1;
			//printf("pap successed!\n");
			sppp_auth_send(&pap, sp, PAP_ACK, h->ident,
				       sizeof mlen, (const char *)&mlen,
				       sizeof(SUCCMSG) - 1, (u_char *)SUCCMSG,
				       0);
		}
		if (sp->state[IDX_PAP] == STATE_REQ_SENT||
		    (sp->role ==1 && sp->state[IDX_PAP] == STATE_INITIAL)) {
			sppp_cp_change_state(&pap, sp, STATE_OPENED);
			pap.tlu(sp);
		}
		break;

	/* ack and nak are his authproto */
	case PAP_ACK:
		UNTIMEOUT(sppp_pap_my_TO, (void *)sp, sp->pap_my_to_ch);
		if (debug) {
			/*addlog(SPP_FMT "PAP: I %s id=0x%x len=%d ",
			    SPP_ARGS(sp),
			    sppp_auth_type_name(PPP_PAP, h->type),
			    //sppp_state_name(sp->state[IDX_PAP]),
			    h->ident, ntohs(h->len));
			name_len = *((char *)h);
			if (len > 5 && name_len) {
				addlog(": ");
				sppp_print_string((char*)(h+1), name_len);
			}
			addlog("\n");*/
			name_len = *((char *)h);
			sprintf(string, ": ");
			sppp_string(string, (char*)(h + 1), name_len);
			addlog(SPP_FMT "PAP: I %s id=0x%x len=%d %s\n",
			    SPP_ARGS(sp),
			    sppp_auth_type_name(PPP_PAP, h->type),
			    h->ident, ntohs(h->len), (len > 5 && name_len)?string:"");
		}
		syslog(LOG_INFO, "spppd: %s: Authentication(PAP) success\n", sp->if_name);
/*		sppp_auth_log(sp->if_unit, 1);	// Jenny, for success authentication log
#ifdef DIAGNOSTIC_TEST
		sppp_diag_log(sp->if_unit, 2);	// Jenny, diagnostic test, authentication with ISP pass
#endif*/
		//x = splimp();
		sp->pp_flags &= ~PP_NEEDAUTH;
		if (sp->myauth.proto == PPP_PAP &&
		    (sp->lcp.opts & (1 << LCP_OPT_AUTH_PROTO)) &&
		    (sp->lcp.protos & (1 << IDX_PAP)) == 0) {
			/*
			 * We are authenticator for PAP but didn't
			 * complete yet.  Leave it to tlu to proceed
			 * to network phase.
			 */
			//splx(x);
			break;
		}
		//splx(x);
		sppp_phase_network(sp);
		break;

	case PAP_NAK:
		UNTIMEOUT(sppp_pap_my_TO, (void *)sp, sp->pap_my_to_ch);
		if (debug) {
			/*addlog(SPP_FMT "PAP: I %s id=0x%x len=%d name=",
			    SPP_ARGS(sp),
			    sppp_auth_type_name(PPP_PAP, h->type),
			    //sppp_state_name(sp->state[IDX_PAP]),
			    h->ident, ntohs(h->len));
			name_len = *((char *)h);
			if (len > 5 && name_len) {
				sppp_print_string((char*)(h+1), name_len);
			}
			addlog("\n");*/
			name_len = *((char *)h);
			sppp_string(string, (char*)(h + 1), name_len);
			addlog(SPP_FMT "PAP: I %s id=0x%x len=%d name=%s\n",
			    SPP_ARGS(sp),
			    sppp_auth_type_name(PPP_PAP, h->type),
			    h->ident, ntohs(h->len), (len > 5 && name_len)?string:"");
		}
//		printf("\033[0;31m set sp->myauth.proto from %s to %s\n\033[0m", sppp_proto_name(sp->myauth.proto), sppp_proto_name(sp->myauth.aumode));
		sp->myauth.proto = sp->myauth.aumode;
		sppp_auth_log(sp->if_unit, 3);	// Jenny, for failure authentication log
		sppp_last_connection_error(sp->if_unit, ERROR_AUTHENTICATION_FAILURE);
		/* await LCP shutdown by authenticator */
		break;

	default:
		/* Unknown PAP packet type -- ignore. */
		if (debug) {
			/*addlog(SPP_FMT "pap corrupted input "
			    "<0x%x id=0x%x len=%d ",
			    SPP_ARGS(sp),
			    h->type, h->ident, ntohs(h->len));
			sppp_print_bytes((u_char*)(h+1), len-4);
			addlog(">\n");*/
			addlog(SPP_FMT "pap corrupted input "
			    "<0x%x id=0x%x len=%d %s>\n",
			    SPP_ARGS(sp),
			    h->type, h->ident, ntohs(h->len), bytes);
		}
		break;

	}
}

static void
sppp_pap_init(struct sppp *sp)
{
	/* PAP doesn't have STATE_INITIAL at all. */
	sp->state[IDX_PAP] = STATE_CLOSED;
	sp->fail_counter[IDX_PAP] = 0;
	sp->pp_seq[IDX_PAP] = 0;
	sp->pp_rseq[IDX_PAP] = 0;
	sppp_callout_handle_init(&sp->ch[IDX_PAP]);
	sppp_callout_handle_init(&sp->pap_my_to_ch);
}

static void 
sppp_pap_up(struct sppp *sp)
{
	//printf("%s proto:0x%x lcp->opts:0x%x\n", __func__, sp->hisauth.proto, sp->lcp.opts);
	if (sp->hisauth.proto == PPP_PAP &&
		(sp->lcp.opts & (1 << LCP_OPT_AUTH_PROTO)) != 0) {
		if (sp->state[IDX_PAP] == STATE_CLOSED)
		{
			sp->state[IDX_PAP] = STATE_INITIAL;
			TIMEOUT(pap.TO, (void *)sp, sp->lcp.timeout,
			    sp->ch[pap.protoidx]);
		}
	}
}

static void
sppp_pap_open(struct sppp *sp)
{
	if (sp->hisauth.proto == PPP_PAP &&
	    (sp->lcp.opts & (1 << LCP_OPT_AUTH_PROTO)) != 0) {
		/* we are authenticator for PAP, start our timer */
		sp->rst_counter[IDX_PAP] = sp->lcp.max_configure;
		sppp_cp_change_state(&pap, sp, STATE_REQ_SENT);
	}
	if (sp->myauth.proto == PPP_PAP) {
		/* we are peer, send a request, and start a timer */
		pap.scr(sp);
		TIMEOUT(sppp_pap_my_TO, (void *)sp, sp->lcp.timeout,
		    sp->pap_my_to_ch);
	}
}

static void
sppp_pap_close(struct sppp *sp)
{
	if (sp->state[IDX_PAP] != STATE_CLOSED)
		sppp_cp_change_state(&pap, sp, STATE_CLOSED);
}

/*
 * That's the timeout routine if we are authenticator.  Since the
 * authenticator is basically passive in PAP, we can't do much here.
 */
static void
sppp_pap_TO(void *cookie)
{
	struct sppp *sp = (struct sppp *)cookie;
	STDDCL;
	int s;

	//s = splimp();
	if (debug)
		addlog(SPP_FMT "pap TO(%s) rst_counter = %d\n",
		    SPP_ARGS(sp),
		    sppp_state_name(sp->state[IDX_PAP]),
		    sp->rst_counter[IDX_PAP]);

	if (1 == sp->role) {
		pap.Close(sp);
		lcp.Close(sp);
		return;
	}
	
	if (--sp->rst_counter[IDX_PAP] < 0)
		/* TO- event */
		switch (sp->state[IDX_PAP]) {
		case STATE_REQ_SENT:
			pap.tld(sp);
			sppp_cp_change_state(&pap, sp, STATE_CLOSED);
			break;
		}
	else
		/* TO+ event, not very much we could do */
		switch (sp->state[IDX_PAP]) {
		case STATE_REQ_SENT:
			sppp_auth_log(sp->if_unit, 3);	// Jenny, for failure authentication log
			sppp_last_connection_error(sp->if_unit, ERROR_AUTHENTICATION_FAILURE);
			/* sppp_cp_change_state() will restart the timer */
			sppp_cp_change_state(&pap, sp, STATE_REQ_SENT);
			break;
		}

	//splx(s);
}

/*
 * That's the timeout handler if we are peer.  Since the peer is active,
 * we need to retransmit our PAP request since it is apparently lost.
 * XXX We should impose a max counter.
 */
static void
sppp_pap_my_TO(void *cookie)
{
	struct sppp *sp = (struct sppp *)cookie;
	STDDCL;

	if (debug)
		addlog(SPP_FMT "pap peer TO\n",
		    SPP_ARGS(sp));

	sppp_auth_log(sp->if_unit, 3);	// Jenny, for failure authentication log
	sppp_last_connection_error(sp->if_unit, ERROR_AUTHENTICATION_FAILURE);
	pap.scr(sp);
}

static void
sppp_pap_tlu(struct sppp *sp)
{
	STDDCL;
	int x;

	sp->rst_counter[IDX_PAP] = sp->lcp.max_configure;

	//if (debug)
	//	addlog(SPP_FMT "%s tlu\n",
	//	    SPP_ARGS(sp), pap.name);

	//x = splimp();
	/* indicate to LCP that we need to be closed down */
	sp->lcp.protos |= (1 << IDX_PAP);
	//printf("%s lcp.protos:0x%x\n", __func__, sp->lcp.protos);

	if (sp->pp_flags & PP_NEEDAUTH) {
		/*
		 * Remote is authenticator, but his auth proto didn't
		 * complete yet.  Defer the transition to network
		 * phase.
		 */
		//splx(x);
		return;
	}
	//splx(x);
	sppp_phase_network(sp);
}

static void
sppp_pap_tld(struct sppp *sp)
{
	STDDCL;

	if (debug)
		addlog(SPP_FMT "pap tld\n", SPP_ARGS(sp));
	UNTIMEOUT(pap.TO, (void *)sp, sp->ch[IDX_PAP]);
	UNTIMEOUT(sppp_pap_my_TO, (void *)sp, sp->pap_my_to_ch);
	sp->lcp.protos &= ~(1 << IDX_PAP);

	sppp_auth_log(sp->if_unit, 3);	// Jenny, for failure authentication log
	sppp_last_connection_error(sp->if_unit, ERROR_AUTHENTICATION_FAILURE);
	lcp.Close(sp);
}

static void
sppp_pap_scr(struct sppp *sp)
{
	u_char idlen, pwdlen;

	sp->confid[IDX_PAP] = ++sp->pp_seq[IDX_PAP];
	pwdlen = sppp_strnlen(sp->myauth.secret, AUTHKEYLEN);
	idlen = sppp_strnlen(sp->myauth.name, AUTHNAMELEN);

	sppp_auth_send(&pap, sp, PAP_REQ, sp->confid[IDX_PAP],
		       sizeof idlen, (const char *)&idlen,
		       (size_t)idlen, sp->myauth.name,
		       sizeof pwdlen, (const char *)&pwdlen,
		       (size_t)pwdlen, sp->myauth.secret,
		       0);
}

/*
 * Random miscellaneous functions.
 */

/*
 * Send a PAP or CHAP proto packet.
 *
 * Varadic function, each of the elements for the ellipsis is of type
 * ``size_t mlen, const u_char *msg''.  Processing will stop iff
 * mlen == 0.
 * NOTE: never declare variadic functions with types subject to type
 * promotion (i.e. u_char). This is asking for big trouble depending
 * on the architecture you are on...
 */

static void
sppp_auth_send(const struct cp *cp, struct sppp *sp,
               unsigned int type, unsigned int id,
	       ...)
{
	STDDCL;
	struct ppp_header *h;
	struct lcp_header *lh;
	u_char *p;
	int len, pkt_len;
	unsigned int mlen;
	int ppp_header_len;
	const char *msg;
	va_list ap;

	u_char pkt_buf[256];

	if (sp->pp_flags & PP_ADDRCOMP) {
		*(u_short*)pkt_buf = htons(cp->proto);
		ppp_header_len = 2;
		lh = (struct lcp_header*)((u_char*)pkt_buf+2);
	} else {
		h = (struct ppp_header*)pkt_buf;
		h->address = PPP_ALLSTATIONS;		/* broadcast address */
		h->control = PPP_UI;			/* Unnumbered Info */
		h->protocol = htons(cp->proto);
		ppp_header_len = PPP_HEADER_LEN;

		lh = (struct lcp_header*)(h + 1);
	}

	lh->type = type;
	lh->ident = id;
	p = (u_char*) (lh+1);

  
	va_start(ap, id);
	len = 0;

	while ((mlen = (unsigned int)va_arg(ap, size_t)) != 0) {
		msg = va_arg(ap, const char *);
		len += mlen;
		if (len > MHLEN - ppp_header_len - LCP_HEADER_LEN) {		
			va_end(ap);
			return;
		}
		bcopy(msg, p, mlen);
		p += mlen;
	}
	va_end(ap);

	lh->len = htons (LCP_HEADER_LEN + len);
	pkt_len = ppp_header_len + LCP_HEADER_LEN + len;

	if (debug) {
		/*addlog(SPP_FMT "%s: O %s id=0x%x len=%d <",
		    SPP_ARGS(sp), cp->name,
		    sppp_auth_type_name(cp->proto, lh->type),
		    //sppp_state_name(sp->state[cp->protoidx]),
		    lh->ident, ntohs(lh->len));
		sppp_print_bytes((u_char*) (lh+1), len);
		addlog(">\n");*/
		char bytes[256];
		bytes[0] = 0;
		sppp_bytes(bytes, (u_char*)(lh+1), len);
		addlog(SPP_FMT "%s: O %s [%s] id=0x%x len=%d <%s>\n",
		    SPP_ARGS(sp), cp->name,
		    sppp_auth_type_name(cp->proto, lh->type), sppp_state_name(sp->state[IDX_CHAP]),
		    lh->ident, ntohs(lh->len), bytes);
	}
	/* send to lower interface */
	output(sp, pkt_buf, pkt_len);


}


/*
 * Send keepalive packets, every KEEPALIVE_INTERVAL seconds.
 */
static void
sppp_keepalive(void *dummy)
{
	struct sppp *sp;
	//int s;

	//s = splimp();
	// Jenny, for keepalive setting
	if (keepalive_timer == 0)
		keepalive_timer = KEEPALIVE_INTERVAL;
	
	for (sp=spppq; sp; sp=sp->pp_next) {
		//struct ifnet *ifp = &sp->pp_if;

		/* Keepalive mode disabled or channel down? */
		if (! (sp->pp_flags & PP_KEEPALIVE) ||
		    ! (sp->pp_flags & IFF_RUNNING))
			continue;

		/* No keepalive in PPP mode if LCP not opened yet. */
		if (sp->pp_phase < PHASE_AUTHENTICATE)
			continue;

		// Jenny, for keepalive packets enable
		if (!sp->diska) {
			if (sp->pp_alivecnt == MAXALIVECNT) {
			/* No keepalive packets got.  Stop the interface. */
				printf (SPP_FMT "down!n", SPP_ARGS(sp));
				syslog(LOG_INFO, "spppd: %s down: No keepalive packets got\n", sp->if_name);
				{
					const struct cp *cp = &lcp;
#ifdef CONFIG_USER_PPPOMODEM
					if(sp->over==SPPP_PPPOMODEM)
					{
						pppomodem_set_hangup(sp);
					}
#endif //CONFIG_USER_PPPOMODEM
					(cp->tld)(sp);
					sp->rst_counter[cp->protoidx] = 0;
					sppp_cp_change_state(cp, sp, STATE_STOPPING);
					// Kaohj -- continue to check the other links
					continue;
					//return;
				}			
			}
			if (sp->pp_alivecnt <= MAXALIVECNT)
				++sp->pp_alivecnt;
			if (sp->pp_phase >= PHASE_AUTHENTICATE) {
				long nmagic = htonl (sp->lcp.magic);
				sp->lcp.echoid = ++sp->pp_seq[IDX_LCP];
				sppp_cp_send (sp, PPP_LCP, ECHO_REQ,
					sp->lcp.echoid, 4, &nmagic);
			}
		}
		// Casey, for Dial on demain, check timeout
/* by Jenny
		if(sp->pp_phase == PHASE_NETWORK) {
			if(sp->timeout) {
				if(chk_ifxmtrcv(sp)) {
				struct cp *cp = &lcp;
					(cp->tld)(sp);
					sp->rst_counter[cp->protoidx] = 0;
					sppp_cp_change_state(cp, sp, STATE_STOPPING);
					// Jenny, continue to send keepalive packets after on-demand timeout (for more than one link)
					continue;
					//return;
				}
			}
		}*/
		//		
	}
	//splx(s);
	TIMEOUT(sppp_keepalive, 0, hz * keepalive_timer, keepalive_ch);
//	TIMEOUT(sppp_keepalive, 0, hz * KEEPALIVE_INTERVAL, keepalive_ch);
}



/*
 * Get both IP addresses.
 */
static void
sppp_get_ip_addrs(struct sppp *sp, u_long *src, u_long *dst, u_long *srcmask)
{
	if (dst)
		*dst = htonl(sp->ipcp.hisipaddr);
	if(src)
		*src = htonl(sp->ipcp.myipaddr);
}


/*
 * Set my IP address.  Must be called at splimp.
 */
static void
sppp_set_ip_addr(struct sppp *sp, u_long src)
{
	sp->ipcp.myipaddr = ntohl(src);	
}

#ifdef CONFIG_IPV6


/*
 *	convert Ethernet address to modified EUI-64
 *	src(6 octects):	mac address
 *	dst(8 octects):	target MEUI-64
 *	Return: 1 Success; 0 fail
 */
static int mac_meui64(char *src, char *dst)
{
	int i;
	
	memset(dst, 0, 8);
	memcpy(dst, src, 3);
	memcpy(dst + 5, src + 3, 3);
	dst[3] = 0xff;
	dst[4] = 0xfe;
	dst[0] ^= 0x02;
	return 1;
}

/*
 * Get both IPv6 addresses.
 */
static void
sppp_get_ip6_addrs(struct sppp *sp, struct in6_addr *src, struct in6_addr *dst,
		   struct in6_addr *srcmask)
{
	// kaohj
	if (dst)
		bcopy(sp->ipv6cp.hisip6addr, dst, sizeof(*dst));
	if (src)
	{
		unsigned char devAddr[MAC_ADDR_LEN];
		unsigned char meui64[8];
		char *addr = src;
		int i;
		
		mib_get(MIB_ELAN_MAC_ADDR, (void *)devAddr);
		mac_meui64(devAddr, meui64);

		//Make a IPv6 link local adddress
		memset(src,0,sizeof(*src));
		bcopy(meui64, addr+8, sizeof(meui64));
		addr[0]=0xfe;
		addr[1]=0x80;
		addr[15]+=(1+sp->unit)*7;
	}

	#if 0
	struct ifnet *ifp = SP2IFP(sp);
	struct ifaddr *ifa;
	struct sockaddr_in6 *si, *sm;
	struct in6_addr ssrc, ddst;

	sm = NULL;
	bzero(&ssrc, sizeof(ssrc));
	bzero(&ddst, sizeof(ddst));
	/*
	 * Pick the first link-local AF_INET6 address from the list,
	 * aliases don't make any sense on a p2p link anyway.
	 */
	si = NULL;
	if_addr_rlock(ifp);
	TAILQ_FOREACH(ifa, &ifp->if_addrhead, ifa_link)
		if (ifa->ifa_addr->sa_family == AF_INET6) {
			si = (struct sockaddr_in6 *)ifa->ifa_addr;
			sm = (struct sockaddr_in6 *)ifa->ifa_netmask;
			if (si && IN6_IS_ADDR_LINKLOCAL(&si->sin6_addr))
				break;
		}
	if (ifa) {
		if (si && !IN6_IS_ADDR_UNSPECIFIED(&si->sin6_addr)) {
			bcopy(&si->sin6_addr, &ssrc, sizeof(ssrc));
			if (srcmask) {
				bcopy(&sm->sin6_addr, srcmask,
				      sizeof(*srcmask));
			}
		}

		si = (struct sockaddr_in6 *)ifa->ifa_dstaddr;
		if (si && !IN6_IS_ADDR_UNSPECIFIED(&si->sin6_addr))
			bcopy(&si->sin6_addr, &ddst, sizeof(ddst));
	}

	if (dst)
		bcopy(&ddst, dst, sizeof(*dst));
	if (src)
		bcopy(&ssrc, src, sizeof(*src));
	if_addr_runlock(ifp);
	#endif
}

#ifdef IPV6CP_MYIFID_DYN
/*
 * Generate random ifid.
 */
static void
sppp_gen_ip6_addr(struct sppp *sp, const struct in6_addr *addr)
{
	/* TBD */
}

/*
 * Set my IPv6 address.  Must be called at splimp.
 */
static void
sppp_set_ip6_addr(struct sppp *sp, const struct in6_addr *src)
{
	// Kaohj
	bcopy(src, sp->ipv6cp.myip6addr, sizeof(*src));
	#if 0
	STDDCL;
	struct ifaddr *ifa;
	struct sockaddr_in6 *sin6;

	/*
	 * Pick the first link-local AF_INET6 address from the list,
	 * aliases don't make any sense on a p2p link anyway.
	 */

	sin6 = NULL;
	if_addr_rlock(ifp);
	TAILQ_FOREACH(ifa, &ifp->if_addrhead, ifa_link) {
		if (ifa->ifa_addr->sa_family == AF_INET6) {
			sin6 = (struct sockaddr_in6 *)ifa->ifa_addr;
			if (sin6 && IN6_IS_ADDR_LINKLOCAL(&sin6->sin6_addr)) {
				ifa_ref(ifa);
				break;
			}
		}
	}
	if_addr_runlock(ifp);

	if (ifa != NULL) {
		int error;
		struct sockaddr_in6 new_sin6 = *sin6;

		bcopy(src, &new_sin6.sin6_addr, sizeof(new_sin6.sin6_addr));
		error = in6_ifinit(ifp, ifatoia6(ifa), &new_sin6, 1);
		if (debug && error) {
			log(LOG_DEBUG, SPP_FMT "sppp_set_ip6_addr: in6_ifinit "
			    " failed, error=%d\n", SPP_ARGS(ifp), error);
		}
		ifa_free(ifa);
	}
	#endif
}
#endif

/*
 * Suggest a candidate address to be used by peer.
 */
static void
sppp_suggest_ip6_addr(struct sppp *sp, struct in6_addr *suggest)
{
	struct in6_addr myaddr;
	struct timeval tv;

	sppp_get_ip6_addrs(sp, &myaddr, 0, 0);

	myaddr.s6_addr[8] &= ~0x02;	/* u bit to "local" */
	// Kaohj -- use gettimeofday instead
	gettimeofday(&tv, 0);
	//microtime(&tv);
	if ((tv.tv_usec & 0xff) == 0 && (tv.tv_sec & 0xff) == 0) {
		myaddr.s6_addr[14] ^= 0xff;
		myaddr.s6_addr[15] ^= 0xff;
	} else {
		myaddr.s6_addr[14] ^= (tv.tv_usec & 0xff);
		myaddr.s6_addr[15] ^= (tv.tv_sec & 0xff);
	}
	if (suggest)
		bcopy(&myaddr, suggest, sizeof(myaddr));
}
#endif // of CONFIG_IPV6

#if 0
static int
sppp_params(struct sppp *sp, u_long cmd, void *data)
{
	u_long subcmd;
	struct ifreq *ifr = (struct ifreq *)data;
	struct spppreq *spr;
	int rv = 0;

	int dbg = 0;
	if ((spr = malloc(sizeof(struct spppreq))) == 0)
		return (EAGAIN);
	/*
	 * ifr->ifr_data is supposed to point to a struct spppreq.
	 * Check the cmd word first before attempting to fetch all the
	 * data.
	 */
	bcopy((char*)ifr->ifr_data, (char*)&subcmd, sizeof(u_long));
	bcopy((char*)ifr->ifr_data, (char*)spr, sizeof(struct spppreq));
	
	switch (subcmd) {
	case SPPPIOGDEFS:
		if (cmd != SIOCGIFGENERIC) {
			rv = EINVAL;
			break;
		}
		/*
		 * We copy over the entire current state, but clean
		 * out some of the stuff we don't wanna pass up.
		 * Remember, SIOCGIFGENERIC is unprotected, and can be
		 * called by any user.  No need to ever get PAP or
		 * CHAP secrets back to userland anyway.
		 */
		 memcpy(&spr->defs.pp_phase, &sp->pp_phase, sizeof(sp->pp_phase));
		 memcpy(&spr->defs.lcp, &sp->lcp, sizeof(spr->defs.lcp));
		 memcpy(&spr->defs.ipcp, &sp->ipcp, sizeof(sp->ipcp));
		 memcpy(&spr->defs.myauth, &sp->myauth, sizeof(sp->myauth));
		 memcpy(&spr->defs.hisauth, &sp->hisauth, sizeof(sp->hisauth));

		/*
		 * Fixup the LCP timeout value to milliseconds so
		 * spppcontrol doesn't need to bother about the value
		 * of "hz".  We do the reverse calculation below when
		 * setting it.
		 */
		spr->defs.lcp.timeout = sp->lcp.timeout * 1000 / hz;
		bcopy((char*)&spr->defs, (char*)ifr->ifr_data, sizeof(struct sppp_parms));		

		break;

	case SPPPIOSDEFS:
		if (cmd != SIOCSIFGENERIC) {
			rv = EINVAL;
			break;
		}
		/*
		 * We have a very specific idea of which fields we
		 * allow being passed back from userland, so to not
		 * clobber our current state.  For one, we only allow
		 * setting anything if LCP is in dead or establish
		 * phase.  Once the authentication negotiations
		 * started, the authentication settings must not be
		 * changed again.  (The administrator can force an
		 * ifconfig down in order to get LCP back into dead
		 * phase.)
		 *
		 * Also, we only allow for authentication parameters to be
		 * specified.
		 *
		 * XXX Should allow to set or clear pp_flags.
		 *
		 * Finally, if the respective authentication protocol to
		 * be used is set differently than 0, but the secret is
		 * passed as all zeros, we don't trash the existing secret.
		 * This allows an administrator to change the system name
		 * only without clobbering the secret (which he didn't get
		 * back in a previous SPPPIOGDEFS call).  However, the
		 * secrets are cleared if the authentication protocol is
		 * reset to 0.  */
		if (sp->pp_phase != PHASE_DEAD &&
		    sp->pp_phase != PHASE_ESTABLISH) {
			rv = EBUSY;
			break;
		}

		if ((spr->defs.myauth.proto != 0 && spr->defs.myauth.proto != PPP_PAP &&
		     spr->defs.myauth.proto != PPP_CHAP) ||
		    (spr->defs.hisauth.proto != 0 && spr->defs.hisauth.proto != PPP_PAP &&
		     spr->defs.hisauth.proto != PPP_CHAP)) {
			rv = EINVAL;
			break;
		}

		if (spr->defs.myauth.proto == 0)
			/* resetting myauth */
			bzero((char*)&sp->myauth, sizeof sp->myauth);
		else {
			/* setting/changing myauth */
			sp->myauth.proto = spr->defs.myauth.proto;
			bcopy(spr->defs.myauth.name, sp->myauth.name, AUTHNAMELEN);
			if (spr->defs.myauth.secret[0] != '\0')
				bcopy(spr->defs.myauth.secret, sp->myauth.secret,
				      AUTHKEYLEN);
		}
		if (spr->defs.hisauth.proto == 0)
			/* resetting hisauth */
			bzero((char*)&sp->hisauth, sizeof sp->hisauth);
		else {
			/* setting/changing hisauth */
			sp->hisauth.proto = spr->defs.hisauth.proto;
			sp->hisauth.flags = spr->defs.hisauth.flags;
			bcopy(spr->defs.hisauth.name, sp->hisauth.name, AUTHNAMELEN);
			if (spr->defs.hisauth.secret[0] != '\0')
				bcopy(spr->defs.hisauth.secret, sp->hisauth.secret,
				      AUTHKEYLEN);
		}
		/* set LCP restart timer timeout */
		if (spr->defs.lcp.timeout != 0)
			sp->lcp.timeout = spr->defs.lcp.timeout * hz / 1000;
		/* set ipcp parameter */
		sp->ipcp.myipaddr = spr->defs.ipcp.myipaddr;
		sp->ipcp.hisipaddr = spr->defs.ipcp.hisipaddr;
		sp->ipcp.primary_dns = spr->defs.ipcp.primary_dns;
		sp->ipcp.second_dns = spr->defs.ipcp.second_dns;
		break;

	default:
		rv = EINVAL;
	}

 quit:
	free(spr);

	return (rv);
}
#endif


static void
sppp_phase_network(struct sppp *sp)
{
	STDDCL;
	int i;
	u_long mask;

	sp->pp_phase = PHASE_NETWORK;

	if (debug)
		addlog(SPP_FMT "PPP: phase is %s\n", SPP_ARGS(sp),
		    sppp_phase_name(sp->pp_phase));

	sppp_auth_log(sp->if_unit, 1);	// Jenny, for success authentication log
#ifdef DIAGNOSTIC_TEST
	sppp_diag_log(sp->if_unit, 2);	// Jenny, diagnostic test, authentication with ISP pass
#endif

	/*
	 * Notify NCPs now.
	 * CCP should be notified firstly.
	 */
	for (i = IDX_COUNT - 1; i >= 0; i--)
		if ((cps[i])->flags & CP_NCP)
			(cps[i])->Open(sp);

	//printf("%s lcp.protos=0x%x\n", __func__, sp->lcp.protos);
	/* Send Up events to all NCPs. */
	if (1 == sp->role) {
		for (i = IDX_COUNT - 1, mask = 1<<(IDX_COUNT-1); i >= 0; i--, mask >>= 1)
			if ((sp->lcp.protos & mask) && ((cps[i])->flags & CP_NCP))
				(cps[i])->Up(sp);
	}
	else {
		for (i = 0, mask = 1; i < IDX_COUNT; i++, mask <<= 1)
			if ((sp->lcp.protos & mask) && ((cps[i])->flags & CP_NCP))
				(cps[i])->Up(sp);
	}

	/* if no NCP is starting, all this was in vain, close down */
	sppp_lcp_check_and_close(sp);
}


static const char *
sppp_cp_type_name(u_char type)
{
	static char buf[12];
	switch (type) {
	case CONF_REQ:   return "CONF-REQ";
	case CONF_ACK:   return "CONF-ACK";
	case CONF_NAK:   return "CONF-NAK";
	case CONF_REJ:   return "CONF-REJ";
	case TERM_REQ:   return "TERM-REQ";
	case TERM_ACK:   return "TERM-ACK";
	case CODE_REJ:   return "CODE-REJ";
	case PROTO_REJ:  return "PROTO-REJ";
	case ECHO_REQ:   return "ECHO-REQ";
	case ECHO_REPLY: return "ECHO-REPLY";
	case DISC_REQ:   return "DISCARD-REQ";
	}
	sprintf (buf, "cp/0x%x", type);
	return buf;
}

static const char *
sppp_auth_type_name(u_short proto, u_char type)
{
	static char buf[12];
	switch (proto) {
	case PPP_CHAP:
		switch (type) {
		case CHAP_CHALLENGE:	return "CHALLENGE";
		case CHAP_RESPONSE:	return "RESPONSE";
		case CHAP_SUCCESS:	return "SUCCESS";
		case CHAP_FAILURE:	return "FAILURE";
		}
	case PPP_PAP:
		switch (type) {
		case PAP_REQ:		return "AUTH-REQ";
		case PAP_ACK:		return "AUTH-ACK";
		case PAP_NAK:		return "AUTH-NAK";
		}
	}
	sprintf (buf, "auth/0x%x", type);
	return buf;
}

static const char *
sppp_lcp_opt_name(u_char opt)
{
	static char buf[12];
	switch (opt) {
	case LCP_OPT_MRU:		return "mru";
	case LCP_OPT_ASYNC_MAP:		return "Async-Map";
	case LCP_OPT_AUTH_PROTO:	return "Auth-Proto";
	case LCP_OPT_QUAL_PROTO:	return "Qual-Proto";
	case LCP_OPT_MAGIC:		return "Magic";
	case LCP_OPT_PROTO_COMP:	return "Proto-Comp";
	case LCP_OPT_ADDR_COMP:		return "Addr-Comp";
	}
	sprintf (buf, "lcp/0x%x", opt);
	return buf;
}

static const char *
sppp_ipcp_opt_name(u_char opt)
{
	static char buf[16];
	switch (opt) {
	case IPCP_OPT_ADDRESSES:	return "Addresses";
	case IPCP_OPT_COMPRESSION:	return "Compression";
	case IPCP_OPT_ADDRESS:		return "Address";
	case IPCP_OPT_DNS_PRIMARY:	return "Primary DNS";
	case IPCP_OPT_DNS_SECOND:	return "Secondary DNS";
	}
	sprintf (buf, "ipcp/0x%x", opt);
	return buf;
}

#ifdef CONFIG_IPV6
static const char *
sppp_ipv6cp_opt_name(u_char opt)
{
	static char buf[12];
	switch (opt) {
	case IPV6CP_OPT_IFID:		return "ifid";
	case IPV6CP_OPT_COMPRESSION:	return "compression";
	}
	sprintf (buf, "0x%x", opt);
	return buf;
}
#endif


static const char *
sppp_state_name(int state)
{
	switch (state) {
	case STATE_INITIAL:	return "Initial";
	case STATE_STARTING:	return "Starting";
	case STATE_CLOSED:	return "Closed";
	case STATE_STOPPED:	return "Stopped";
	case STATE_CLOSING:	return "Closing";
	case STATE_STOPPING:	return "Stopping";
	case STATE_REQ_SENT:	return "Req-Sent";
	case STATE_ACK_RCVD:	return "Ack-Rcvd";
	case STATE_ACK_SENT:	return "Ack-Sent";
	case STATE_OPENED:	return "Opened";
	}
	return "illegal";
}

const char *
sppp_phase_name(enum ppp_phase phase)
{
	switch (phase) {
	case PHASE_DEAD:	return "Dead";
	case PHASE_ESTABLISH:	return "Establish";
	case PHASE_TERMINATE:	return "Terminate";
	case PHASE_AUTHENTICATE: return "Authenticate";
	case PHASE_NETWORK:	return "Network";
	}
	return "illegal";
}

static const char *
sppp_proto_name(u_short proto)
{
	static char buf[12];
	switch (proto) {
	case PPP_LCP:	return "LCP";
	case PPP_IPCP:	return "IPCP";
	case PPP_PAP:	return "PAP";
	case PPP_CHAP:	return "CHAP";
	case PPP_IPV6CP: return "IPV6CP";
#ifdef CONFIG_PPP_MPPE_MPPC	
	case PPP_CCP:   return "CCP";
#endif	
	
	}
	snprintf(buf, sizeof(buf), "proto/0x%x", (unsigned)proto);
	return buf;
}

static const int
sppp_proto_idx(u_short proto)
{
	switch (proto) {
	case PPP_LCP:	return IDX_LCP;
	case PPP_IPCP:	return IDX_IPCP;
	case PPP_PAP:	return IDX_PAP;
	case PPP_CHAP:	return IDX_CHAP;
	case PPP_IPV6CP:	return IDX_IPV6CP;
#ifdef CONFIG_PPP_MPPE_MPPC	
	case PPP_CCP:   return IDX_CCP;
#endif	
	}
}

#if 0
static void
sppp_print_bytes(const u_char *p, u_short len)
{
#if 0
	if (len)
		addlog(" %*D", len, p, "-");
#endif
u_short i;
	for(i=0; i<len; i++)
		addlog("%02X", p[i]);
}

static void
sppp_print_string(const char *p, u_short len)
{
	u_char c;

	while (len-- > 0) {
		c = *p++;
		/*
		 * Print only ASCII chars directly.  RFC 1994 recommends
		 * using only them, but we don't rely on it.  */
		if (c < ' ' || c > '~')
			addlog("\\x%x", c);
		else
			addlog("%c", c);
	}
}
#endif

static void sppp_bytes(const char *result, const u_char *p, u_short len)
{
	u_short i;
	// todo: don't print too much to avoid memory violation.
	if (len>64) len=64;
	for (i=0; i<len; i++)
		sprintf(result, "%s%02X", result, p[i]);
}

static void sppp_string(const char *result, const char *p, u_short len)
{
	u_char c;

	while (len-- > 0) {
		c = *p++;
		if (c < ' ' || c > '~')
			sprintf(result, "%s\\x%x", result, c);
		else
			sprintf(result, "%s%c", result, c);
	}
}

static const char *
sppp_dotted_quad(u_long addr)
{
	static char s[16];
	sprintf(s, "%d.%d.%d.%d",
		(int)((addr >> 24) & 0xff),
		(int)((addr >> 16) & 0xff),
		(int)((addr >> 8) & 0xff),
		(int)(addr & 0xff));
	return s;
}

#ifdef CONFIG_IPV6
static const char *
ip6_sprintf(char *ip6buf, void *src)
{
	const char *s;
	
	s = (const char *)inet_ntop(PF_INET6, src, ip6buf, INET6_ADDRSTRLEN);
	return s;
}
#endif

static int
sppp_strnlen(u_char *p, int max)
{
	int len;

	for (len = 0; len < max && *p; ++p)
		++len;
	return len;
}

/* a dummy, used to drop uninteresting events */
static void
sppp_null(struct sppp *unused)
{
	/* do just nothing */
}

// Jenny, log authentication status
const char PPPD_AUTHLOG[] = "/tmp/ppp_auth_log";
static void sppp_auth_log(int unit, int value)
{
	struct sppp *sp;
	char buff[16];
	FILE *fp;
	
	fp=fopen(PPPD_AUTHLOG, "w");
	
	sp = spppq;
	while(sp) {
		if(sp->if_unit == unit)
			sp->auth_flag = value;
		if(sp->over == SPPP_PPPOATM || 
#ifdef CONFIG_USER_PPPOMODEM
		   sp->over == SPPP_PPPOMODEM ||
#endif //CONFIG_USER_PPPOMODEM
		   sp->over == SPPP_PPPOE) {	// PPPoE:1, PPPoA: 0
			sprintf(buff, "%d:%d\n", sp->if_unit, sp->auth_flag);
			fputs(buff, fp);
		}
		sp = sp->pp_next;
	}
	fclose(fp);
}

//#ifdef  CONFIG_USER_PPPOE_PROXY  
#if 0
#define CONF_SERVER_PIDFILE	"/var/run/configd.pid"

static void sendcmd(struct mymsgbuf* qbuf)
{
	key_t key;
	int   qid, cpid, spid;
	FILE *spidfile;

	/* Create unique key via call to ftok() */
	key = ftok("/bin/init", 'k');
	if ((qid = open_queue(key, MQ_GET)) == -1) {
		error("open_queue");
		return;
	}

	// get client pid
	cpid = (int)getpid();

	// get server pid
	if ((spidfile = fopen(CONF_SERVER_PIDFILE, "r"))) {
		fscanf(spidfile, "%d\n", &spid);
		fclose(spidfile);
	}
	else
		printf("server pidfile not exists\n");

	send_message(qid, spid, cpid, &qbuf->msg);
	while (!peek_message(qid, cpid));
	read_message(qid, qbuf, cpid);
}

int no_adsl_link_wan_ppp(int unit)
{
	struct mymsgbuf qbuf;
	MSG_T *mymsg;
	int k;
	int ret = 1;

	mymsg = &qbuf.msg;
	mymsg->cmd = CMD_NO_ADSLLINK_PPP;
	mymsg->arg1 = unit;
	sendcmd(&qbuf);
	if (qbuf.request != MSG_SUCC) {
		printf("no_adsl_link_wan_ppp failed\n");
		ret = 0;
	}
	return ret;
}

/*
static int add_policy_routing_table(int wan_uint)
{
       struct mymsgbuf qbuf;
	MSG_T *mymsg;
	int k;
	int ret=1;

	mymsg = &qbuf.msg;
	mymsg->cmd = CMD_ADD_POLICY_TABLE;
	mymsg->arg1 = wan_uint;

	sendcmd(&qbuf);
	if (qbuf.request != MSG_SUCC) {
		printf("add_policy_routing_table error\n");
		ret = 0;
	}

	return ret;
}
*/
#endif

#ifdef CONFIG_USER_PPPOMODEM
void stop_interface_modem(struct sppp *sp)
{
	if( sp && (sp->over==SPPP_PPPOMODEM) )
	{
		/* No link.  Stop the interface. */
		printf (SPP_FMT "down\n", SPP_ARGS(sp));
		{
			syslog(LOG_INFO, "spppd: %s down: Modem hangup\n", sp->if_name);

			const struct cp *cp = &lcp;
			(cp->tld)(sp);
			sp->rst_counter[cp->protoidx] = 0;
			sppp_cp_change_state(cp, sp, STATE_STOPPING);
			sppp_last_connection_error(sp->if_unit, ERROR_NO_CARRIER);
#ifdef DIAGNOSTIC_TEST
			sppp_diag_log(sp->if_unit, 0);	// Jenny, diagnostic test log init
#endif
		}	
	}
}
#endif //CONFIG_USER_PPPOMODEM

// Jenny, stop PPP interface if there is no ADSL link
void stop_interface(char* interface_name)
{
	struct sppp *sp;
	struct pppoe_param_s *poer;
	int i;

	for (sp=spppq; sp; sp=sp->pp_next) {
		if (1 == sp->role)//VPN Server has nothing to do with wan interface link status
			continue;
		
		if(sp->over == SPPP_PPPOE) {
			poer = sp->dev;
			//printf("%s %d: %s, %s %d\n", __func__, __LINE__, poer->dev_name, interface_name, sizeof(ALIASNAME_VC));
			if(strcmp(poer->dev_name, interface_name)) {
				//printf("%s %d\n", __func__, __LINE__);
				continue;
			}
		}
		else if(sp->over == SPPP_PPPOATM) {
			if(strcmp(interface_name, "atm0"))
				continue;
			else
				ppp_term_flag = 0;
		}
		else
			continue;
		
		pre_PADT_flag[sp->if_unit] = 0;
		sendPADR_flag[sp->if_unit] = 0;
		
		if (!(sp->pp_flags & IFF_RUNNING))
			continue;       

		if (sp->pp_phase  < PHASE_AUTHENTICATE)
			continue;
#if 1
		/* No ADSL link.  Stop the interface. */
		printf (SPP_FMT "down\n", SPP_ARGS(sp));
		{
			syslog(LOG_INFO, "spppd: %s down: No ADSL link\n", sp->if_name);
//#ifdef CONFIG_USER_PPPOE_PROXY
#if 0
			if (sp->enable_pppoe_proxy) {
				//no_adsl_link_wan_ppp(sp->unit);
				return;
			}
#endif
			const struct cp *cp = &lcp;
			(cp->tld)(sp);
			sp->rst_counter[cp->protoidx] = 0;
			sppp_cp_change_state(cp, sp, STATE_STOPPING);
			sppp_last_connection_error(sp->if_unit, ERROR_NO_CARRIER);
#ifdef DIAGNOSTIC_TEST
			sppp_diag_log(sp->if_unit, 0);	// Jenny, diagnostic test log init
#endif
			continue;
		}
#endif		
	}
}

//QL, force stop ppp interface
#ifdef CONFIG_USER_L2TPD_L2TPD
void stop_ppp_interface(int pppUnit)
{
	struct sppp *sp;
	L2TP_DRV_CTRL *p;
	int i;

	printf("%s enter!\n", __func__);
	for (sp=spppq; sp; sp=sp->pp_next)
	{
		if ((sp->over != SPPP_L2TP) || (sp->if_unit != pppUnit))
			continue;
		
		if (!(sp->pp_flags & IFF_RUNNING))
			continue;       

		if (sp->pp_phase  < PHASE_AUTHENTICATE)
			continue;

		p = sp->pp_lowerp;
		if (p->fd < 0)//tunnel has been removed
			continue;
		
		pre_PADT_flag[sp->if_unit] = 0;
		sendPADR_flag[sp->if_unit] = 0;
		
		/* No ADSL link.  Stop the interface. */
		printf ("%s " SPP_FMT " down!\n", __func__, SPP_ARGS(sp));
		{
			const struct cp *cp = &lcp;
			(cp->tld)(sp);
			sp->rst_counter[cp->protoidx] = 0;
			sppp_cp_change_state(cp, sp, STATE_STOPPING);
			sppp_last_connection_error(sp->if_unit, ERROR_NO_CARRIER);
#ifdef DIAGNOSTIC_TEST
			sppp_diag_log(sp->if_unit, 0);	// Jenny, diagnostic test log init
#endif
		}

		break;
	}
}
#endif

// Jenny, PPP uptime
void sppp_uptime(struct sppp *sp, char *uptime, char *tuptime)
{
	int updays, uphours, upminutes, upseconds, len;
	struct sysinfo info;
	long now;
	long pppuptime = 0;
	
	sysinfo(&info);
	now = (int)info.uptime;	// get current time
	uptime[0] = '\0';
	tuptime[0] = '\0';
	if (starttime[sp->if_unit]) {
		pppuptime = now - starttime[sp->if_unit];
		updays = (int) pppuptime / (60*60*24);
		if (updays)
			sprintf(uptime, "%dday%s,", updays, (updays != 1) ? "s" : "");
		len = strlen(uptime);
		upminutes = (int) pppuptime / 60;
		uphours = (upminutes / 60) % 24;
		upminutes %= 60;
		upseconds = (int) pppuptime % 60;
		sprintf(&uptime[len], "%02d:%02d:%02d", uphours, upminutes, upseconds);
	}
	else
		sprintf(uptime, "0sec");

	pppuptime += totaluptime[sp->if_unit];
	if (pppuptime) {
		updays = (int) pppuptime / (60*60*24);
		if (updays)
			sprintf(tuptime, "%dday%s,", updays, (updays != 1) ? "s" : "");
		len = strlen(tuptime);
		upminutes = (int) pppuptime / 60;
		uphours = (upminutes / 60) % 24;
		upminutes %= 60;
		upseconds = (int) pppuptime % 60;
		sprintf(&tuptime[len], "%02d:%02d:%02d", uphours, upminutes, upseconds);
		}
	else
		sprintf(tuptime, "0sec");
}

#ifdef DIAGNOSTIC_TEST
// Jenny, log diagnostic test needed information
const char PPPD_DIAGLOG[] = "/tmp/ppp_diag_log";
void sppp_diag_log(int unit, int value)
{
	struct sppp *sp;
	char buff[16];
	FILE *fp;
	
	fp=fopen(PPPD_DIAGLOG, "w");
	
	sp = spppq;
	while(sp) {
		if(sp->if_unit == unit)
			sp->diag_flag = value;
		if(sp->over == SPPP_PPPOATM || 
#ifdef CONFIG_USER_PPPOMODEM
		   sp->over == SPPP_PPPOMODEM ||
#endif //CONFIG_USER_PPPOMODEM
#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
		   sp->over == SPPP_PPTP ||
#endif //CONFIG_USER_PPTP_CLIENT_PPTP
#ifdef CONFIG_USER_L2TPD_L2TPD
		   sp->over == SPPP_L2TP ||
#endif //endof CONFIG_USER_L2TPD_L2TPD
		   sp->over == SPPP_PPPOE) {	// PPPoE:1, PPPoA: 0
			sprintf(buff, "%d:%d\n", sp->if_unit, sp->diag_flag);
			fputs(buff, fp);
		}
		sp = sp->pp_next;
	}
	fclose(fp);
}
#endif

#ifdef _CWMP_MIB_
// Jenny, check auto disconnect timeout
static void sppp_auto_disconnect_timeout(void *cookie)
{
	struct sppp *sp = (struct sppp *)cookie;
	if (sp->pp_phase == PHASE_NETWORK) {
		sppp_last_connection_error(sp->if_unit, ERROR_FORCED_DISCONNECT);
		syslog(LOG_INFO, "spppd: %s: forced disconnected\n", sp->if_name);
	}
	ppp_down(sp->if_unit);
}

// Jenny, warn disconnect delay
static void sppp_cp_down_delay(void *cookie)
{
	struct sppp *sp = (struct sppp *)cookie;
	STDDCL;
	struct cp *cp = &lcp;

	ppp_pending_flag[sp->if_unit] = 0;
	(cp->tld)(sp);
	//sp->rst_counter[cp->protoidx] = 0;
	sp->rst_counter[cp->protoidx] = sp->lcp.max_terminate;
	sppp_cp_send(sp, cp->proto, TERM_REQ, ++sp->pp_seq[cp->protoidx], 0, 0);
	sppp_cp_change_state(cp, sp, STATE_CLOSING);
	//sppp_cp_change_state(cp, sp, STATE_STOPPING);
}

// Jenny, log lcp echo information
const char PPPD_ECHO[] = "/tmp/ppp_lcp_echo";
const char PPPD_ECHO_RETRY[] = "/tmp/ppp_echo_retry";
void sppp_lcp_echo_log(void)
{
	FILE *fp;

	if (keepalive_timer == 0)
		keepalive_timer = KEEPALIVE_INTERVAL;
	fp = fopen(PPPD_ECHO, "w");
	fprintf(fp, "%d", keepalive_timer);
	fclose(fp);

	fp = fopen(PPPD_ECHO_RETRY, "w");
	fprintf(fp, "%d", MAXALIVECNT);
	fclose(fp);
}
#endif

// Jenny, log last connection error
const char PPPD_ERROR_LOG[] = "/tmp/ppp_error_log";
void sppp_last_connection_error(int unit, enum ppp_last_connection_error value)
{
	struct sppp *sp;
	char buff[16];
	int internet_flag = -1;
	FILE *fp;
	
	if ((fp = fopen("/proc/internet_flag", "r"))) {
		fscanf(fp, "%d\n", &internet_flag);
		fclose(fp);
	}

	fp=fopen(PPPD_ERROR_LOG, "w");
	
	sp = spppq;
	while(sp) {
		if(sp->if_unit == unit) {
			if ((internet_flag != 1) && sp->dgw && ((value == ERROR_AUTHENTICATION_FAILURE) || (value == ERROR_NO_ANSWER)
				|| (value == ERROR_NOT_ENABLED_FOR_INTERNET) || (value == ERROR_IP_CONFIGURATION))) {
				if (internet_flag != -1)
					//turn on INTERNET_RED_LED
					system("echo '2'>/proc/internet_flag");
			}
			sp->pp_error = value;
		}
		if(sp->over == SPPP_PPPOATM || 
#ifdef CONFIG_USER_PPPOMODEM
		   sp->over == SPPP_PPPOMODEM ||
#endif //CONFIG_USER_PPPOMODEM
#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
		   sp->over == SPPP_PPTP ||
#endif //CONFIG_USER_PPTP_CLIENT_PPTP
#ifdef CONFIG_USER_L2TPD_L2TPD
		   sp->over == SPPP_L2TP ||
#endif//endof CONFIG_USER_L2TPD_L2TPD
		   sp->over == SPPP_PPPOE) {	// PPPoE:1, PPPoA: 0
			sprintf(buff, "%d:%d\n", sp->if_unit, sp->pp_error);
			fputs(buff, fp);
		}
		sp = sp->pp_next;
	}
	
	fclose(fp);
}

void sppp_debug_log(int unit, int value)
{
	struct sppp *sp;
	char buff[16];
	FILE *fp;
	
	fp=fopen(PPP_DEBUG_LOG, "w");
	
	sp = spppq;
	while(sp) {
		if(sp->if_unit == unit)
			sp->debug = value;
		if(sp->over == SPPP_PPPOATM || 
#ifdef CONFIG_USER_PPPOMODEM
		   sp->over == SPPP_PPPOMODEM ||
#endif //CONFIG_USER_PPPOMODEM
		   sp->over == SPPP_PPPOE) {	// PPPoE:1, PPPoA: 0
			sprintf(buff, "%d:%d\n", sp->if_unit, sp->debug);
			fputs(buff, fp);
		}
		sp = sp->pp_next;
	}
	fclose(fp);
}

#if defined(CONFIG_USER_PPTPD_PPTPD) || defined(CONFIG_USER_L2TPD_LNS) || defined(CONFIG_USER_PPPOE_PROXY)
void
spppd_attach(struct sppp *sp)
{
	/* Insert new entry into the keepalive list. */
	sp->pp_next = spppq;
	spppq = sp;
	
 	sp->pp_flags = PP_ADDRCOMP | PP_KEEPALIVE;
 	if(sp->timeout) {
 		sp->idletime = 0;
 	}
	sp->pp_loopcnt = 0;
	sp->pp_alivecnt = 0;
	bzero((char*)&sp->pp_seq[0], sizeof(sp->pp_seq));
	bzero((char*)&sp->pp_rseq[0], sizeof(sp->pp_rseq));
	sp->pp_phase = PHASE_DEAD;
	sp->pp_up = lcp.Up;
	//sp->pp_down = lcp.Down;
	sp->pp_down = sppp_cp_down;
	sp->pp_last_recv = sp->pp_last_sent = time_second;
	
	sppp_init(sp);

}
#endif


#ifdef CONFIG_USER_PPPOE_CTC_DELAY_INT
int get_poe_ctcto(struct sppp *sp, int state)
{

	if(state == 1)
	{
		// if auth fail , the counter should minus 1 and return relative timeout value
		if(sp->ctc_retry_cnt == 1) return 5; // should change to 4 ?
		else if(sp->ctc_retry_cnt == 2) return 10;
		else if(sp->ctc_retry_cnt == 3) return 20;
		else if(sp->ctc_retry_cnt == 4) return 40;
		else if(sp->ctc_retry_cnt == 5) return 90;
		else return 90;
	}
	else
	{
		if(sp->ctc_retry_cnt == 0)
		{
			sp->ctc_retry_cnt = 1;
			return 5;
		}
		else if(sp->ctc_retry_cnt == 1)
		{
			sp->ctc_retry_cnt = 2;
			return 10;
		}
		else if(sp->ctc_retry_cnt == 2)
		{
			sp->ctc_retry_cnt = 3;
			return 20;
		}
		else if(sp->ctc_retry_cnt == 3)
		{
			sp->ctc_retry_cnt = 4;
			return 40;
		}
		else if(sp->ctc_retry_cnt >= 4)
		{
			sp->ctc_retry_cnt = 5;
			return 90;
		}
	}
}
#endif


