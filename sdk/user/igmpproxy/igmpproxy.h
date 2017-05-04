#ifndef _IGMPPROXY_H_
#define _IGMPPROXY_H_

#include "mclab.h"
#include "timeout.h"
#include <fcntl.h>
#include <signal.h>
#include <rtk/options.h>


#ifdef CONFIG_IGMPPROXY_MULTIWAN
#define  MAXWAN 8
#else 
#define  MAXWAN 1
#endif

// Send Group-specific query periodically (keepalive polling)
#define PERIODICAL_SPECIFIC_QUERY
// Send Group-specific query on leave (fast leave)
//#define LEAVE_SPECIFIC_QUERY
// Maintain the group members in order to do immediately leave
#define KEEP_GROUP_MEMBER
// Send IGMP General Query periodically
#define SEND_GENERAL_QUERY

//#define SEND_UNSOLICIT_REPORT

/* IGMP timer and default values */
#define LAST_MEMBER_QUERY_INTERVAL	1	// second
#define LAST_MEMBER_QUERY_COUNT		2
// Kaohj --- group-specific query in periodical
#ifdef PERIODICAL_SPECIFIC_QUERY
#define MEMBER_QUERY_INTERVAL		30	// second
#define MEMBER_QUERY_COUNT		3
#endif
#define GENERAL_QUERY_INTERVAL		15	// second
#define UNSOLT_REPORT_INTERVAL   30  //second

#define TIMER_GENERAL_QUERY		1
#define TIMER_DELAY_QUERY		2
#define TIMER_UNSOLT_REPORT  3

/* IGMP group address */
#define ALL_SYSTEMS			htonl(0xE0000001)	// General Query - 224.0.0.1
#define ALL_ROUTERS			htonl(0xE0000002)	// Leave - 224.0.0.2	
#define ALL_ROUTERS_V3			htonl(0xE0000016)	// Leave - 224.0.0.22
#define ALL_PRINTER			htonl(0xEFFFFFFA)	// notify all printer - 239.255.255.250
#define CLASS_D_MASK			0xE0000000		// the mask that defines IP Class D
#define IPMULTI_MASK			0x007FFFFF		// to get the low-order 23 bits

/* header length */
#define MIN_IP_HEADER_LEN		20
#define IGMP_MINLEN			8

/* IGMP v3 type */
#define IGMP_HOST_V3_MEMBERSHIP_REPORT	0x22


// group member entry
struct mbr_entry {
	struct mbr_entry	*next;
	__u32			user_addr;
};

struct igmp_timer {
	int		type;			// timer type

	union
	{
		int		retry_left;		// retry counts left
		int   unsol_report_count;   //unsolicit report count 
	};
	struct callout	ch;
#ifdef _SUPPORT_IGMPV3_
	unsigned int	lefttime;
#endif /*_SUPPORT_IGMPV3_*/
};

#ifdef _SUPPORT_IGMPV3_
struct src_entry {
	struct src_entry	*next;
	__u32			srcaddr;
	struct igmp_timer	timer;
};
#endif /*_SUPPORT_IGMPV3_*/

struct mcft_entry {
	struct mcft_entry 	*next;
	__u32			grp_addr;
	__u32			user_count;	// group member count
	struct mbr_entry 	*grp_mbr;
	struct igmp_timer	timer;
#ifdef _SUPPORT_IGMPV3_
	int			igmp_ver;
	int			filter_mode;
	struct src_entry	*srclist;
	int			mrt_state; //0:disable, 1: enable all sources
#endif /*_SUPPORT_IGMPV3_*/
};


#ifdef _SUPPORT_IGMPV3_
/*IGMP version*/
#define IGMP_VER_2		2
#define IGMP_VER_3		3
extern int igmpv3_accept(int recvlen, struct IfDesc *dp);
extern void igmpv3_timer(void);
#endif /*_SUPPORT_IGMPV3_*/

extern char igmp_down_if_name[IFNAMSIZ];
extern char igmp_down_if_idx;

#ifdef CONFIG_IGMPPROXY_MULTIWAN
extern char igmp_up_if_name[MAXWAN][IFNAMSIZ];
#else
extern char igmp_up_if_name[IFNAMSIZ];
#endif

#ifdef CONFIG_IGMPPROXY_MULTIWAN
extern char igmp_up_if_idx[MAXWAN];
#else
extern char igmp_up_if_idx;
#endif

#ifdef CONFIG_IGMPPROXY_MULTIWAN
extern int igmp_up_if_num;
#endif

extern char *recv_buf;
extern char *send_buf;

extern struct mcft_entry *mcpq;

extern int igmp_query(__u32 dst, __u32 grp,__u8 mrt);

extern struct mcft_entry * add_mcft(__u32 grp_addr, __u32 src_addr);
extern int del_mcft(__u32 grp_addr);
extern int chk_mcft(__u32 grp_addr);
extern struct mcft_entry * get_mcft(__u32 grp_addr);


#endif /*_IGMPPROXY_H_*/
