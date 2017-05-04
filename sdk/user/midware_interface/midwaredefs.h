#ifndef _MIDWAREDEFS_H_
#define _MIDWAREDEFS_H_

/* UNIX socket file */
#define MW_DOMAIN_FILE		"/var/ct/tmp/interface2sock"
//#define MW_DOMAIN_FILE		"/var/run/interface2sock"

/*packet len */
#define MAX_MIDWARE_PKTLEN		2048

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

#define BIT_SUCCESS				0x1
#define BIT_PART_SUCCESS			0x2
#define BIT_SUCCESS_REBOOT		0x4
#define BIT_FAILED				0x8
#define BIT_NAME_PWD_ERR		0x10
#define BIT_CONNSERVER_ERR		0x20
#define BIT_NOFILE_ONSERVER		0x40
#define BIT_PARA_NOT_EXIST		0x80
#define BIT_PKT_TOO_LONG			0x100
#define BIT_INVALID_PARAVALUE	0x200
#define BIT_SET_UNWRITABLE_PARA		0x400
#define BIT_ATTRI_CANNOT_SET		0x800
#define BIT_UNSUPPORT_PROTO		0x1000

#define RET_BITSET(retcode,bit_value)	(retcode |= bit_value)		
#define RET_BTICLR(retcode,bit_value)	(retcode &= ~bit_value)
#define RET_CLEAR(retcode)				(retcode = 0)
/*propetry*/
#define D0	0x1		
#define D1	0x2	/* D1|D0-- 0: don't report;1:report to ITMS passively;2:report to ITMS actively;3:undefined*/
#define D2	0x4  /* D2--0:cannot write;1:can write*/
#define D3	0x8  /* D3--0:don't report to middleware; 1:repor to middleware*/

#define NTF_ITMS_PAS	1
#define NTF_ITMS_ACT	2
#define NTF_MIDWARE		1

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
#define OP_MWExit					89
#define OP_MWExitRet				90


/*the following OP_*** is defined for internal process*/
#define OP_SaveReboot				101
#define OP_RegisterFirmware			102
#define OP_SetDefaultFlag			103
#define OP_informKeyPara			104
#define OP_informEvent				105


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

#define MAX_MWNAME_LEN	30
#define MAX_TRNAME_LEN	80
/*type*/
#define OBJECT_NAME		1
#define LEAF_NAME		2
/*flag*/
#define NAME_END			0x1	/* denote that to this level,the paraName should be exchanged between trName and mwName*/

typedef struct paraNameNode{
 char * mwName;
 char * trName;
 int type;
 int flag;
 struct paraNameNode * nextLevel;
} PARA_NAME_NODE;

struct setParaValueResult{
	char * result;
	int successCount;
};

struct mwMsg {
	int	msg_type;
	int	msg_datatype;
	char	msg_data[MAX_MIDWARE_PKTLEN+4];
};

#define MW_MSG_SIZE  		(sizeof(struct mwMsg) - sizeof(int))

/* others*/
#define R_CHAR2WORD(addr)		(((uint16)*((uint8 *)(addr)) << 8) | ((uint16)*((uint8 *)(addr)+1)))
#define W_WORD2CHAR(add, val)		{ *((uint8 *)((add)+1)) = ((val) & 0xff); \
    								*((uint8 *)(add))   = (((val) >> 8) & 0xff);}

#define MIDWARE_RUNFILE	"/var/run/midware.pid"
#define MWINTF_MAIN_RUNFILE "/var/run/midintfmain.pid"
#define MW_INTF_RUNFILE	"/var/run/mwintf.pid"
#define CWMP_MIDPROC_RUNFILE	"/var/run/cwmpmidproc.pid"
#define MGT_DNS_FILE		"/var/mgtdns.conf"

#define SENDPKT2CWMP	1 
#define PROCESS_OK		2
#define PROCESS_ERROR	-1

#define MSG_SEND_OK 		0
#define MSG_SEND_ERR		-1

#define MSG_RECV_ERR		-1
#define PACKET_ERR		0
#define PACKET_OK		1

#define MW_SUCCESS		0
#define MW_ERR_9001		-9001	/*invlaid packet*/
#define MW_ERR_9002		-9002	/*parameter name error*/
#define MW_ERR_9003		-9003	/*parameter type error*/
#define MW_ERR_9004		-9904	/*parameter data error*/

#define CMD_ERROR		-1 	/*command error*/
#define CMD_FROM_TR069	0	/* command from tr069*/
#define CMD_FROM_WEB	1	/* command from web*/
#define CMD_FROM_MW		2	/*command from midware*/

enum {
	MSG_MIDWARE = 30,
	MSG_END
};

struct inform_node{
	struct inform_node * next;
	struct cwmp__ParameterValueStruct * data;
	int __size;
};

struct inform_queue{
	int status;
	int length;
	struct inform_node * head;
};

#define INFORM_IDLE			0
#define INFORM_EMPTY			0x1	/*queue is empty*/
#define INFORM_SENT			0x2	/*have sent a inform packet,but not receive reply packet*/
#define INFORM_REPLY			0x4	/*have been replied by midware*/
#define INFORM_WAIT			0x8 /*if this flag is set,inform will be sent next timer out*/

#define INFORM_QUEUE_MAXLEN		100

#define INFORM_CHECK_INTERVAL	20
#define MWSURL_TRANS_INTERVAL	60
#define MW_EXIT_WAIT_TIME		10

/*ct event define*/
#define CTEVENT_ACCOUNTCHANGE	0x01
#define CTEVENT_BIND			0x02
#define CTEVENT_SEND_INFORM		0x03
#define KEYEVENT_WLANSHORT		0x10
#define KEYEVENT_WLANLONG		0x20
#define KEYEVENT_WLANDOUBLE		0x30
#define KEYEVENT_WPSSHORT		0x40
#define KEYEVENT_WPSLONG		0x50
#define KEYEVENT_WPSDOUBLE		0x60

/*midware flag define*/
#define MIDWARE_DEBUG_FLAG		0x01

#endif

