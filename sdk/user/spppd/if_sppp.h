/*
 * Defines for synchronous PPP/Cisco link level subroutines.
 *
 * Copyright (C) 1994 Cronyx Ltd.
 * Author: Serge Vakulenko, <vak@cronyx.ru>
 *
 * Heavily revamped to conform to RFC 1661.
 * Copyright (C) 1997, Joerg Wunsch.
 *
 * This software is distributed with NO WARRANTIES, not even the implied
 * warranties for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Authors grant any other persons or organizations permission to use
 * or modify this software as long as this message is kept with the software,
 * all derivative works or modified versions.
 *
 * From: Version 2.0, Fri Oct  6 20:39:21 MSK 1995
 *
 * $FreeBSD: src/sys/net/if_sppp.h,v 1.16.2.3 2002/04/24 18:45:25 joerg Exp $
 */

#ifndef _NET_IF_SPPP_H_
#define _NET_IF_SPPP_H_ 1

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

#include <linux/if.h>		/* ifreq struct         */
#include <net/if_arp.h>
#include <net/route.h>
#include <netinet/in.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_pppox.h>

#include <linux/ppp_defs.h>
#include <linux/if_ppp.h>
#include <linux/version.h>

#include "timeout.h"
#include <linux/config.h>
#include <config/autoconf.h>
#include <rtk/options.h>
#ifdef CONFIG_USER_PPPOE_PROXY
#include <rtk/msgq.h>  

extern int Enable_pppoe_proxy; 
#endif


#define SPPP_PPPOATM 0
#define SPPP_PPPOE 1
#define SPPP_PPPOMODEM 2
#define SPPP_PPTP	3
#define SPPP_L2TP	4
#define SPPP_PPTPD	5
#define SPPP_L2TPD	6

#define PPTPD_AUTH_IDX	0
#define L2TPD_AUTH_IDX	1
#define SPPPD_AUTH_NUM		3
#define PPTP_PORT 1723
#define L2TP_PORT 1701



extern char ppp_term_flag;

// Mason Yu. Add VPN ifIndex
// unit declarations for ppp
// (1) 0 ~ 7: pppoe/pppoa, (2) 8: 3G, (3) 9 ~ 10: PPTP, (4) 11 ~12: L2TP
// #define MAX_IPIP_NUM=2, MAX_PPTP_NUM=2, MAX_L2TP_NUM=2 on mib.h
#if defined(CONFIG_USER_PPTP_CLIENT_PPTP) || defined(CONFIG_USER_L2TPD_L2TPD)	// Mason Yu. Add VPN ifIndex
#define N_SPPP_EXTRA	5
#else
#ifdef CONFIG_USER_PPPOMODEM
#define N_SPPP_EXTRA	1
#else
#define N_SPPP_EXTRA	0
#endif //CONFIG_USER_PPPOMODEM
#endif
#if defined(CONFIG_USER_PPTP_CLIENT_PPTP) || defined(CONFIG_USER_L2TPD_L2TPD)
#define N_SPPP_PPTP		8
#else //end of CONFIG_USER_PPTP_CLIENT_PPTP || CONFIG_USER_L2TPD_L2TPD
#define N_SPPP_PPTP		0
#endif

#ifdef CONFIG_USER_PPPOE_PROXY
#define N_SPPP_PPPOE_PROXY 32
#else
#define N_SPPP_PPPOE_PROXY 0
#endif

#define N_SPPP_EXT	(8+N_SPPP_EXTRA)
#define N_SPPP (8+N_SPPP_EXTRA+N_SPPP_PPTP+N_SPPP_PPPOE_PROXY)

//alex_huang
//#define CONFIG_SPPPD_STATICIP 
#define IDX_LCP 0		/* idx into state table */

//krammer
#define CHAP_MICROSOFT      0x80
#define CHAP_MICROSOFT_V2   0x81
#define MPPE_TYPE   0x40
#define MPPC_TYPE   0x01
#define MPPE_MPPC_TYPE   (MPPE_TYPE|MPPC_TYPE)




struct slcp {
	u_long	opts;		/* LCP options to send (bitfield) */
	u_long  magic;          /* local magic number */
	u_long	mru;		/* our max receive unit */
	u_long	their_mru;	/* their max receive unit */
	u_long	protos;		/* bitmask of protos that are started */
	u_char  echoid;         /* id of last keepalive echo request */
	/* restart max values, see RFC 1661 */
	int	timeout;
	int	max_terminate;
	int	max_configure;
	int	max_failure;
#if defined(CONFIG_USER_PPPOMODEM) || defined(CONFIG_PPP_MPPE_MPPC)
#define DEF_ACCM_VALUE 0xffffffff
	/* lcp option, accm*/
	u_long xaccm; /*sending accm*/
	u_long raccm; /*receiving accm*/
	u_char neg_xaccm; /*negotiate xaccm*/
	u_char neg_raccm; /*negotiate raccm*/
#endif //CONFIG_USER_PPPOMODEM
};

#define IDX_IPCP 1		/* idx into state table */
#define IDX_IPV6CP 2		/* idx into state table */

struct sipcp {
	u_long	opts;		/* IPCP options to send (bitfield) */
	u_int	flags;
#define IPCP_HISADDR_SEEN 1	/* have seen his address already */
#define IPCP_MYADDR_DYN   2	/* my address is dynamically assigned */
#define IPCP_MYADDR_SEEN  4	/* have seen his address already */
#define IPV6CP_MYIFID_DYN 8	/* my ifid is dynamically assigned */
#define IPV6CP_MYIFID_SEEN 0x10	/* have seen his ifid already */
#define IPCP_VJ		0x20	/* can use VJ compression */
	int	max_state;	/* VJ: Max-Slot-Id */
	int	compress_cid;	/* VJ: Comp-Slot-Id */
	/*
	u_long	myipaddr;
	u_long	hisipaddr;
	*/
	union {
		u_long	_ul_myip;
		u_char	_ub_myip6[16];
	} _un_myip;
	union {
		u_long	_ul_hisip;
		u_char	_ub_hisip6[16];
	} _un_hisip;
	u_long	dns_opts;
	u_long	primary_dns;
	u_long	second_dns;
};
#define myipaddr _un_myip._ul_myip
#define hisipaddr _un_hisip._ul_hisip
#define myip6addr _un_myip._ub_myip6
#define hisip6addr _un_hisip._ub_hisip6

#define AUTHNAMELEN	64
//#define AUTHKEYLEN	16
#define AUTHKEYLEN	64

struct sauth {
	u_short	proto;			/* authentication protocol to use */
	u_short	aumode;		// Jenny, configured authentication mode
	u_short	flags;
#define AUTHFLAG_NOCALLOUT	1	/* do not require authentication on */
					/* callouts */
#define AUTHFLAG_NORECHALLENGE	2	/* do not re-challenge CHAP */
	u_char	name[AUTHNAMELEN];	/* system identification name */
	u_char	secret[AUTHKEYLEN];	/* secret password */
	u_char	challenge[AUTHKEYLEN];	/* random challenge */
	u_char chal_len;		/* challenge length */
    u_char chal_id;		/* ID of last challenge */
    u_char chal_type;		/* hash algorithm for challenges */
    u_char enc_type;		/* enctryption type*/
	int chal_transmits;
};

#define IDX_PAP		3
#define IDX_CHAP	4

#ifdef CONFIG_PPP_MPPE_MPPC

#define IDX_CCP 5

#define IDX_COUNT (IDX_CCP + 1) /* bump this when adding cp's! */
#else
#define IDX_COUNT (IDX_CHAP + 1)

#endif
/*
 * Don't change the order of this.  Ordering the phases this way allows
 * for a comparision of ``pp_phase >= PHASE_AUTHENTICATE'' in order to
 * know whether LCP is up.
 */
enum ppp_phase {
	PHASE_DEAD, PHASE_ESTABLISH, PHASE_TERMINATE,
	PHASE_AUTHENTICATE, PHASE_NETWORK
};

enum ppp_last_connection_error {
	ERROR_NONE=0, ERROR_ISP_TIME_OUT=1, ERROR_COMMAND_ABORTED=2,
	ERROR_NOT_ENABLED_FOR_INTERNET=3, ERROR_BAD_PHONE_NUMBER=4,
	ERROR_USER_DISCONNECT=5, ERROR_ISP_DISCONNECT=6, ERROR_IDLE_DISCONNECT=7,
	ERROR_FORCED_DISCONNECT=8, ERROR_SERVER_OUT_OF_RESOURCES=9,
	ERROR_RESTRICTED_LOGON_HOURS=10, ERROR_ACCOUNT_DISABLED=11,
	ERROR_ACCOUNT_EXPIRED=12, ERROR_PASSWORD_EXPIRED=13,
	ERROR_AUTHENTICATION_FAILURE=14, ERROR_NO_DIALTONE=15, ERROR_NO_CARRIER=16,
	ERROR_NO_ANSWER=17, ERROR_LINE_BUSY=18, ERROR_UNSUPPORTED_BITSPERSECOND=19,
	ERROR_TOO_MANY_LINE_ERRORS=20, ERROR_IP_CONFIGURATION=21, ERROR_UNKNOWN=22
};

#define PP_MTU      1500    /* default/minimal MRU */
#define PP_MAX_MRU	2048	/* maximal MRU we want to negotiate */

int keepalive_timer;	// Added by Jenny for keepalive timer

#ifdef CONFIG_USER_PPPOMODEM //paula, 3g backup PPP
int ppp_backup_flag;
int ppp_backup_timer;
int ppp_backup_unit;
int ppp_backup_debug;
#endif

/*
 * This is a cut down struct sppp (see below) that can easily be
 * exported to/ imported from userland without the need to include
 * dozens of kernel-internal header files.  It is used by the
 * SPPPIO[GS]DEFS ioctl commands below.
 */
struct sppp_parms {
	u_char	if_name[IFNAMSIZ];
	int unit;
	int	over;				/* PPPoE / PPPoA / PPTP / L2TP */
	int	mode;				/* direct / dial on demand */
	int	ipType;				/* 0: v4, 1: v6, 2: both */
	int dgw;				/* default gateway */
#ifdef IP_PASSTHROUGH
	int ippt_flag;          /* IP PassThrough Flag -- Added by Mason Yu for Half Bridge */
#endif
	int timeout;
#ifdef CONFIG_SPPPD_STATICIP
	int    staticip;           /*setting static ip flag*/
#endif
	int	debug;
	int	diska;		/* keepalive Setting -- Added by Jenny */
				
	int	enable_vj;			/* VJ header compression enabled */
	enum ppp_phase pp_phase;	/* phase we're currently in */
	struct slcp lcp;		/* LCP params */
	struct sipcp ipcp;		/* IPCP params */
#ifdef CONFIG_IPV6
	struct sipcp ipv6cp;		/* IPv6CP params */
#endif
	struct sauth myauth;	/* auth params, i'm peer */
	struct sauth hisauth;	/* auth params, i'm authenticator */

#ifdef _CWMP_MIB_
	int	autoDisconnectTime;		/* Jenny, for TR-069 support parameter */
	int	warnDisconnectDelay;		/* Jenny, for TR-069 support parameter */
#endif
#ifdef CONFIG_USER_PPPOE_PROXY
	int enable_pppoe_proxy;
	int itfGroup; /* bind lanport */
	int maxUser;  /* pppoe_proxy maxuser */

#endif
#ifdef CONFIG_TR069_DNS_ISOLATION
        int  isTr069_interface ;
#endif
};

/*
 * Definitions to pass struct sppp_parms data down into the kernel
 * using the SIOC[SG]IFGENERIC ioctl interface.
 *
 * In order to use this, create a struct spppreq, fill in the cmd
 * field with SPPPIOGDEFS, and put the address of this structure into
 * the ifr_data portion of a struct ifreq.  Pass this struct to a
 * SIOCGIFGENERIC ioctl.  Then replace the cmd field by SPPPIOSDEFS,
 * modify the defs field as desired, and pass the struct ifreq now
 * to a SIOCSIFGENERIC ioctl.
 */

#define SPPPIOGDEFS  ((caddr_t)(('S' << 24) + (1 << 16) +\
	sizeof(struct sppp_parms)))
#define SPPPIOSDEFS  ((caddr_t)(('S' << 24) + (2 << 16) +\
	sizeof(struct sppp_parms)))

#ifndef SIOCSIFGENERIC
#define SIOCSIFGENERIC	57	/* generic IF set op */
#endif

#ifndef SIOCGIFGENERIC
#define SIOCGIFGENERIC	58	/* generic IF get op */
#endif


struct spppreq {
	// Kaohj
	struct spppreq *next;
	int	cmd;
	struct sppp_parms ppp;
	void *dev;	/* point to pppoe or pppoa defs */
};


#ifdef CONFIG_PPP_MPPE_MPPC


struct sccp {	
	u_char neg_stateless; /*negotiate MPPE_STATELESS*/
	u_char zero_byte;
	u_char neg_mppe_mppc; /*negotiate MPPE_MPPC*/

};

#endif
struct sppp {
	struct  sppp *pp_next;  /* next interface in keepalive list */
	void	*dev;
	int		sock_fd;		/* interface fd */
#ifdef CONFIG_IPV6
	int		sock6_fd;		/* IPv6 interface fd */
#endif
	int		fd;				/* ppp fd */
	int		dev_fd;			/* ppp device fd */
	u_char id;			/* Current id */
	int		chindex;		/* channel index */
	int		if_unit;		/* interface unit */
	int		unit;			/* config unit */
	
	int		over;			/* pppoe / pppoa */
	int		role;			/* 1: server,  0: client*/
	int		mode;			/* direct / auto */
	int		ipType;			/* 0: v4, 1: v6, 2: both */
	int		dgw;			/* set as default route */
#ifdef IP_PASSTHROUGH
	int     	ippt_flag;      	/* IP PassThrough Flag. 1: Enable IPPT, 0:disable IPPT -- Added by Mason Yu for Half Bridge */
#endif
	u_long		previous_ippt_myip;	
	int 		timeout;
#ifdef CONFIG_SPPPD_STATICIP
	int     staticip;       /*static ip setting flag*/
#endif
	int		idletime;		
	int		debug;			/* debug flag */
	int		diska;		/* keepalive Setting -- Added by Jenny for disable keepalive*/
	int		auth_flag;		/* Authentication Flag -- Added by Jenny, 1: success, 3: failure*/
#ifdef DIAGNOSTIC_TEST
	int		diag_flag;		/* Diagnostic processing Flag -- Added by Jenny, 0: init, 1: server connected, 2: auth pass, 3: IP assigned pass*/
#endif
	int 		up_flag;			/* Jenny, for pppoe connect/disconnect flag, 1: could send PADI */
#ifdef _CWMP_MIB_
	int		autoDisconnectTime;		/* Jenny, for TR-069 support parameter, 0: the connection is not to be shut down automatically */
	int		warnDisconnectDelay;	/* Jenny, for TR-069 support parameter, 0: no delay */
	struct callout autodisconnect_ch;	/* Jenny, callout for auto disconnect timeout */
	struct callout disconnectdelay_ch;	/* Jenny, callout for disconnect delay */
#endif
	enum ppp_last_connection_error pp_error;	/* Jenny, last connection error */
		
	u_char	if_name[IFNAMSIZ];
	u_int   pp_mode;        /* major protocol modes (cisco/ppp/...) */
	u_int   pp_flags;       /* sub modes */
	u_short pp_alivecnt;    /* keepalive packets counter */
	u_short pp_loopcnt;     /* loopback detection counter */
	u_long  pp_seq[IDX_COUNT];	/* local sequence number */
	u_long  pp_rseq[IDX_COUNT];	/* remote sequence number */
	enum ppp_phase pp_phase;	/* phase we're currently in */
	int	state[IDX_COUNT];	/* state machine */
	u_char  confid[IDX_COUNT];	/* id of last configuration request */
	int	rst_counter[IDX_COUNT];	/* restart counter */
	int	fail_counter[IDX_COUNT]; /* negotiation failure counter */
	unsigned long long	pp_last_recv;	/* packets has been received */
	unsigned long long	pp_last_sent;	/* packets has been sent */
	struct callout	ch[IDX_COUNT]; /* per-proto and if callouts */
	struct callout	pap_my_to_ch; /* PAP needs one more... */
	struct callout	chap_my_to_ch; /* For CHAP timeout */
	struct callout auth_ch;	/* Jenny, callout for authentication fail restart timeout */
#ifdef CONFIG_USER_PPPOE_PROXY	                                       
//	struct callout link_ch; /*alex_huang ,callout for start check adsl link*/
	int enable_pppoe_proxy;
	int itfGroup;     /* for port mapping */
	int currUser;
	int maxUser;
#endif
#ifdef CONFIG_TR069_DNS_ISOLATION 
         int     isTr069_interface ;
#endif         
#ifdef CONFIG_NO_REDIAL
         int      is_noredial;
#endif
	int holdoff_delay;	/* time to wait before re-initiating the link after it terminates. */
	struct callout	holdoff_ch; /* For re-initiation delay */
	struct slcp 	lcp;		/* LCP params */
	struct sipcp 	ipcp;		/* IPCP params */
#ifdef CONFIG_IPV6
	struct sipcp	ipv6cp;		/* IPv6CP params */
#endif

#ifdef CONFIG_PPP_MPPE_MPPC
    struct sccp 	ccp;		/* LCP params */
#endif

	struct sauth 	myauth;		/* auth params, i'm peer */
	struct sauth 	hisauth;		/* auth params, i'm authenticator */
	/*
	 * These functions are filled in by sppp_attach(), and are
	 * expected to be used by the lower layer (hardware) drivers
	 * in order to communicate the (un)availability of the
	 * communication link.  Lower layer drivers that are always
	 * ready to communicate (like hardware HDLC) can shortcut
	 * pp_up from pp_tls, and pp_down from pp_tlf.
	 */
	void	(*pp_up)(struct sppp *sp);
	void	(*pp_down)(struct sppp *sp);
	/*
	 * These functions need to be filled in by the lower layer
	 * (hardware) drivers if they request notification from the
	 * PPP layer whether the link is actually required.  They
	 * correspond to the tls and tlf actions.
	 */
	void	(*pp_tls)(struct sppp *sp);
	void	(*pp_tlf)(struct sppp *sp);
	/*
	 * These (optional) functions may be filled by the hardware
	 * driver if any notification of established connections
	 * (currently: IPCP up) is desired (pp_con) or any internal
	 * state change of the interface state machine should be
	 * signaled for monitoring purposes (pp_chg).
	 */
	void	(*pp_con)(struct sppp *sp, int idx_ncp);
	void	(*pp_chg)(struct sppp *sp, int new_state);
	/* These two fields are for use by the lower layer */
	void    *pp_lowerp;
	int     pp_loweri;
#ifdef CONFIG_USER_PPPOE_CTC_DELAY_INT
	int 	ctc_retry_cnt;
#endif
	
};


/* bits for pp_flags */
#define PP_KEEPALIVE    0x01    /* use keepalive protocol */
				/* 0x04 was PP_TIMO */
#define PP_CALLIN	0x08	/* we are being called */
#define PP_NEEDAUTH	0x10	/* remote requested authentication */
#define PP_ADDRCOMP	0x20	/* address and control compressiom */

void sppp_idle(void *dummy);
//void sppp_if_down(struct sppp *sp);
//void stop_interface(void);
void stop_interface(char* interface_name);
void sppp_uptime(struct sppp *sp, char *uptime, char *tuptime);
#ifdef DIAGNOSTIC_TEST
void sppp_diag_log(int unit, int value);
#endif
void reconnect_ppp(int unit);
void sppp_last_connection_error(int unit, enum ppp_last_connection_error value);
//alex_huang
extern int  no_adsl_link_wan_ppp(int unit);
#ifdef _CWMP_MIB_
void sppp_lcp_echo_log(void);
// Jenny, set to 1 when in "pending" state
extern int ppp_pending_flag[N_SPPP];
#endif

extern int  no_redial ; 
extern int holdoff_interval;
#if 0
//void sppp_attach (struct ifnet *ifp);
//void sppp_detach (struct ifnet *ifp);
void sppp_attach (struct sppp *sp);
void sppp_detach (struct sppp *sp);
void sppp_input (struct sppp *sp, struct mbuf *m);
int sppp_ioctl (struct sppp *sp, u_long cmd, void *data);
struct mbuf *sppp_dequeue (struct sppp *sp);
struct mbuf *sppp_pick(struct sppp *sp);
int sppp_isempty (struct sppp *sp);
void sppp_flush (struct sppp *sp);
#endif
void sppp_debug_log(int unit, int value);

int get_poe_ctcto(struct sppp *sp,int state);

#endif /* _NET_IF_SPPP_H_ */
