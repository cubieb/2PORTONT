#ifndef INCLUDE_UC_UDP_H
#define INCLUDE_UC_UDP_H

#define _UC_USE_RELAY_
#define _USE_NFBI_WAY_

//command list
#define UC_CMD_LAST_ID		0x0000
#define UC_CMD_SET_MIB		0x0001
#define UC_CMD_GET_MIB		0x0002
#define UC_CMD_SYS_INIT		0x0003
#define UC_CMD_SYS_CMD		0x0004
#define UC_CMD_DSL_IOC		0x0005
#define UC_CMD_SET_MIB_BYID	0x0006
#define UC_CMD_GET_MIB_BYID	0x0007
#define UC_CMD_MASK			0x00ff
#define UC_CMD_NO_RES_MASK	0x4000
#define UC_CMD_RES_MASK		0x8000

//UC_CMD_NO_RES_MASK
#define UC_CMD_NEED_RES		0x0000
#define UC_CMD_NO_RES		0x4000

//UC_CMD_RES_MASK
#define UC_CMD_RES_OK		0x0000
#define UC_CMD_RES_ERR		0x8000

//for sysinit
#define UC_STR_DSL_INIT		"dsl-init"
#define UC_STR_DSL_SETUP	"dsl-setup"
#define UC_STR_MIB_DUMP		"mib-dump"
#define UC_STR_ETH_INIT		"eth-init"
#define UC_STR_ETH_DOWN		"eth-down"



typedef int (*uc_udp_cb)(char*, int, int);
typedef int (*uc_cmd_cb)(unsigned short, char*, int, int);
typedef int (*uc_act_cb)(void);


typedef struct uc_hdr
{
	unsigned int	magic;
	unsigned short	cmd;
	unsigned short	seq;
	unsigned short	code;
	unsigned short	len;
} UC_HDR;
#define UC_HDR_MAGIC	0x55434F4D
#define UC_HDR_SIZE		(sizeof(UC_HDR))
#define UC_DATA_SIZE	(4096+128) //RLCM_GET_VDSL2_DIAG_HLIN needs 4K
#define UC_MAX_SIZE		(UC_HDR_SIZE+UC_DATA_SIZE)
#define UC_PORT			56789
#define UC_TIMEOUT		3


typedef struct uc_data_hdr
{
	unsigned int	id;
	unsigned int	len;
	char			data[0];
}UC_DATA_HDR;
#define UC_DATA_HDR_SIZE	(sizeof(UC_DATA_HDR))


int uc_udp_handle_mesg(char *m, int len, int maxlen, uc_cmd_cb cb);
int uc_udp_bind(char *itf);
int uc_udp_loop(int sockfd, uc_udp_cb cb, uc_act_cb cmdcb);
int uc_udp_real_sendrecv(char *sip, char *data, int len, int maxlen);
int uc_udp_real_sendonly(char *sip, char *data, int len);
int uc_udp_sendrecv(char *sip, unsigned short cmd, char *data, int len, int maxlen);

#endif /*INCLUDE_UC_UDP_H*/

