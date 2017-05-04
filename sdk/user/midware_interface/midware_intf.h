
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <sys/time.h>
#include <net/if.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>
#include <syslog.h>
#include <time.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <config/autoconf.h>
#include <rtk/midwaredefs.h>
#include <rtk/options.h>
#include <rtk/sysconfig.h>

#if 0
/*packet len */
#define MAX_MIDWARE_PKTLEN		2048

/* UNIX socket file */
//#define MW_DOMAIN_FILE		"/var/ct/tmp/interface2sock"
#define MW_DOMAIN_FILE		"/var/run/interface2sock"

/*return value*/
#define RET_SUCCESS				200
#define RET_PART_SUCCESS			202
#define RET_SUCCESS_REBOOT		203
#define RET_FAILED				400
#define RET_NAME_PWD_ERR		402
#define RET_CONNSERVER_ERR		405
#define RET_NOFILE_ONSERVER		407
#define RET_PARA_NOT_EXIST		412
#define RET_PKT_TOO_LONG		420
#define RET_INVALID_PARAVALUE	424
#define RET_SET_UNWRITABLE_PARA		425
#define RET_ATTRI_CANNOT_SET		426
#define RET_UNSUPPORT_PROTO		427

/*propetry*/
#define D0	0x1		
#define D1	0x2	/* D1|D0-- 0: don't report;1:report to ITMS passively;2:report to ITMS actively;3:undefined*/
#define D2	0x4  /* D2--0:cannot write;1:can write*/
#define D3	0x8  /* D3--0:don't report to middleware; 1:repor to middleware*/

/* opcode */
#define OP_Register				61
#define OP_RegisterOK				62
#define OP_ParameterSet			63
#define OP_ParameterSetRet		64
#define OP_ParaInformMW			65
#define OP_ParaInformMWRet		66
#define OP_ParameterGet			67
#define OP_ParameterGetRet		68
#define OP_ParaAttributeSet			69
#define OP_ParaAttributeSetRet		70
#define OP_ParaAttributeGet			71
#define OP_ParaAttributeGetRet		72
#define OP_ParaInformITMS			73
#define OP_ParaInformITMSRet		74
#define OP_Download				75
#define OP_DownloadRet			76
#define OP_Reboot					77
#define OP_OperationDone			78
#define OP_Upload					79
#define OP_UploadRet				80
#define OP_AddObject				81
#define OP_AddObjectRet			82
#define OP_DeleteObject			83
#define OP_DeleteObjectRet			84
#define OP_GetParanames			85
#define OP_GetParanamesRet		86
#define OP_SetDefault				87
#define OP_SetDefaultRet			88
/*the following OP_*** is defined for internal process*/
#define OP_SaveReboot				101
#define OP_RegisterFirmware		102

/* type*/
#define TYPE_ModName				0
#define TYPE_ParameterNames		4
#define TYPE_ParaValues			5
#define TYPE_Retcode				7
#define TYPE_ParaAttributes			8
#define TYPE_TWM					9
#define TYPE_Object				12
#define TYPE_Instance				13
#define TYPE_ParaList				15
#define TYPE_Operation				16

#define uint8		unsigned char
#define uint16		unsigned short

/*structure*/
typedef struct midware_hdr{
uint8 opcode;
uint8 num;
} MW_PKT_HDR;

typedef struct midware_data_hdr{
uint8 type;
uint8 length[2];
} MW_DATA_HDR;

/* others*/
#define R_CHAR2WORD(addr)		(((uint16)*((uint8 *)(addr)) << 8) | ((uint16)*((uint8 *)(addr)+1)))
#define W_WORD2CHAR(add, val)		{ *((uint8 *)((add)+1)) = ((val) & 0xff); \
    								*((uint8 *)(add))   = (((val) >> 8) & 0xff);}

#define SENDPKT2CWMP	1 
#define PROCESS_OK		2
#define PROCESS_ERROR	-1

#define MSG_SEND_OK 		0
#define MSG_SEND_ERR		-1

#define MSG_RECV_ERR		-1
#define PACKET_ERR		0
#define PACKET_OK		1

/*pid file*/
#define CWMP_MIDPROC_RUNFILE	"/var/run/cwmpmidproc.pid"
#define MW_INTF_RUNFILE	"/var/run/mwintf.pid"

/*message structure*/
struct mwMsg {
	int	msg_type;
	int	msg_datatype;
	char	msg_data[MAX_MIDWARE_PKTLEN+4];
};

#define MW_MSG_SIZE  		(sizeof(struct mwMsg) - sizeof(int))

enum {
	MSG_MIDWARE = 30,
	MSG_END
};
#endif


