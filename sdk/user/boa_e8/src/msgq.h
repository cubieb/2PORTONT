/*
 * msgq.h -- System V Message Queue Framework Header
 * --- By Kaohj
 */

#ifndef _h_MSGQ
#define _h_MSGQ 1

#include <sys/types.h>
#ifdef EMBED
#include <config/autoconf.h>
#include <rtk/options.h>
#else
#include "../../../config/autoconf.h"
#include "LINUX/options.h"
#endif
//ql
//#include "LINUX/options.h"

#ifdef VOIP_SUPPORT
#define MAX_SEND_SIZE	(7*1024)
#else /*VOIP_SUPPORT*/
#define MAX_SEND_SIZE	4096
#endif /*VOIP_SUPPORT*/
#define BOA_MAX_SEND_SIZE	5000
// Kaohj -- Shared memory size
//#define SHM_SIZE	16384
//#define SHM_SIZE	20480
#define SHM_SIZE	32768
#define USE_SHM

#define MQ_CREATE	0
#define MQ_GET		1
#define MSG_SUCC	0
#define MSG_FAIL	1
#define MSG_MIB_LOCKED	2
#define KILL_PROCESS_OFF 0
#define KILL_PROCESS_ON  1

/* Command Type */
#define		CMD_START		0
#define		CMD_MIB_LOCK		1
#define		CMD_MIB_UNLOCK		2
#define		CMD_MIB_UPDATE		3
#define		CMD_MIB_GET		4
#define		CMD_MIB_SET		5
#define		CMD_MIB_RELOAD		6
#define		CMD_MIB_SIZE		7
#define		CMD_MIB_INFO_ID		8
#define		CMD_MIB_INFO_INDEX	9
#define		CMD_MIB_BACKUP		10
#define		CMD_MIB_RESTORE		11
#define		CMD_MIB_GET_DEFAULT	12
#define		CMD_MIB_INFO_TOTAL   13
#define		CMD_MIB_SWAP		14
#define		CMD_MIB_FLASH_TO_DEFAULT	15
#define		CMD_MIB_TO_DEFAULT	16

#define		CMD_CHAIN_TOTAL		21
#define		CMD_CHAIN_GET		22
#define		CMD_CHAIN_ADD		23
#define		CMD_CHAIN_DELETE	24
#define		CMD_CHAIN_CLEAR		25
#define		CMD_CHAIN_UPDATE	26
#define		CMD_CHAIN_INFO_ID	27
#define		CMD_CHAIN_INFO_INDEX	28
#define		CMD_CHAIN_INFO_NAME	29
#define		CMD_CHAIN_DESC_ID	30
#define		CMD_CHAIN_SWAP		31

#define		CMD_CHECK_DESC		41

#ifdef CONFIG_IPV6
#if defined(CONFIG_USER_DHCPV6_ISC_DHCP411)
#define		CMD_GET_PD_PREFIX_LEN	42
#define		CMD_STOP_DELEGATION		43
#endif
#if defined(CONFIG_USER_DHCPV6_ISC_DHCP411) && defined(CONFIG_USER_RADVD)
#define		CMD_DELEGATION			44
#endif
#endif

#define		CMD_REBOOT		45
#define		CMD_UPLOAD		47
#define		CMD_KILLPROC		48
#define 	CMD_CHECK_IMAGE    	49
#ifdef CONFIG_USER_DDNS
#define 	CMD_DDNS_CTRL		50
#endif

//#ifdef CONFIG_USER_PPPOE_PROXY
#if defined(CONFIG_USER_PPPOE_PROXY) || defined(CONFIG_USER_PPTP_CLIENT_PPTP)
#define           CMD_ADD_POLICY_TABLE	52
#define           CMD_DEL_POLICY_TABLE	53
#define           CMD_ADD_POLICY_RULE	54
#define           CMD_DEL_POLICY_RULE	55
#define           CMD_NO_ADSLLINK_PPP	56
#define           CMD_POLL_SWITCH_PORT	57
#endif //end of CONFIG_USER_PPTP_CLIENT_PPTP
//#ifdef	RESERVE_KEY_SETTING
#define		CMD_MIB_RETRIVE_TABLE	58
#define		CMD_MIB_RETRIVE_CHAIN	59
//#endif
#define		CMD_START_AUTOHUNT	62
#define		CMD_FILE2XML		63
#define		CMD_XML2FILE		64
#define		CMD_MIB_SET_FLASH	65
#define		CMD_CHAIN_ADD_FLASH	66
#define		CMD_UPDATE_PPPOE_SESSION	67
#define		CMD_MIB_SAVE_PPPOE	68
#define		CMD_MIB_UPDATE_FROM_RAW	69
#define		CMD_MIB_READ_TO_RAW	70
#define		CMD_MIB_READ_HEADER	71
#ifdef CONFIG_IPV6
#if defined(CONFIG_USER_DHCPV6_ISC_DHCP411)
#define		CMD_GET_PD_PREFIX_IP	72
#endif
#endif
#define CMD_SET_DNS_CONFIG		73
#if (defined VOIP_SUPPORT) && (defined CONFIG_USER_XMLCONFIG)
#define         CMD_MIB_VOIP_TO_DEFAULT 74
#endif
#if defined(CONFIG_USER_CWMP_TR069)
#define CMD_SET_ACSURL_ROUTE                   75
#endif


#if defined(CONFIG_IPV6) && defined(DUAL_STACK_LITE) &&defined(CONFIG_USER_DHCPV6_ISC_DHCP411)
#define		CMD_GOT_AFTR	76
#endif
#define		CMD_END			77



typedef struct msgInfo {
	int	cmd;
	int	arg1;
	int	arg2;
	char	mtext[MAX_SEND_SIZE];
} MSG_T;

struct mymsgbuf {
        long mtype;			// Message type
        long request;			// Request ID/Status code
        long tgid;			// thread group tgid
	MSG_T msg;
};

typedef struct twoFile {
	char fromName[32];
	char toName[32];
} MSGFile_T;

extern int	open_queue( key_t keyval, int flag );
extern void	send_message(int qid, long type, long req, long tgid, MSG_T *msg);
extern int	read_message(int qid, struct mymsgbuf *qbuf, long type);
extern int	peek_message(int qid, long type);
extern void	remove_queue(int qid);

#endif /* _h_MSGQ */
