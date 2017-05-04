/*--------------------------------
		Includes
---------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <stdint.h>
#include <linux/atm.h>
#include <linux/atmdev.h>
#include <string.h>

/*--------------------------------
		Definitions
---------------------------------*/

#define reg(address)	(*(volatile uint32 *)((uint32)address))
#define	REG32(reg)		(*(volatile uint32 *)(reg))
#define	REG16(reg)		(*(volatile uint16 *)(reg))
#define	REG8(reg)		(*(volatile uint8 *)(reg))

#define	uint8	unsigned char
#define	uint16	unsigned short
#define	uint32	unsigned long
#define	int8	signed char
#define	int16	signed short
#define	int32	signed long


#define	UINT8	uint8
#define	UINT16	uint16
#define	UINT32	uint32
#define	INT8	int8
#define	INT16	int16
#define	INT32	int32
#define	UCHAR	uint8
#define	BOOL	uint8

#define	VC_CREATED	1
#define	VC_NOT_CREATED	0

#define	TRUE	0
#define	FALSE	1

// for ATM device (RFC1483)
#define SAR_GET_MODEMSTATE		SIOCDEVPRIVATE
#define SAR_GET_STATS			(SIOCDEVPRIVATE+1)
#define SAR_ENABLE			(SIOCDEVPRIVATE+2)
#define SAR_DISABLE			(SIOCDEVPRIVATE+3)
#define SAR_GET_CONFIG 			(SIOCDEVPRIVATE+4)
#define SAR_SET_CONFIG 			(SIOCDEVPRIVATE+5)
#define SAR_ATM_OAM_SET_ID		(SIOCDEVPRIVATE+6)
#define SAR_ATM_OAM_START		(SIOCDEVPRIVATE+7)
#define SAR_ATM_OAM_STOP		(SIOCDEVPRIVATE+8)
#define SAR_ATM_OAM_STATUS		(SIOCDEVPRIVATE+9)
#define SAR_ATM_OAM_STATUS_FE		(SIOCDEVPRIVATE+10)
#define SAR_ATM_OAM_RPT_LB		(SIOCDEVPRIVATE+11)
#define SAR_ATM_OAM_STOP_LB		(SIOCDEVPRIVATE+12)
#define SAR_CREATE_VC			(SIOCDEVPRIVATE+13)
#define SAR_DELETE_VC			(SIOCDEVPRIVATE+14)
#define SAR_ENABLE_UTOPIA		(SIOCDEVPRIVATE+15)
#define	SAR_UTOPIA_FAST			(SIOCDEVPRIVATE+16)
#define	SAR_UTOPIA_SLOW			(SIOCDEVPRIVATE+17)
#define	SAR_SETMAC			(SIOCDEVPRIVATE+18)
#define	SAR_EnableLOOPBACK			(SIOCDEVPRIVATE+19)
#define	SAR_DisableLOOPBACK			(SIOCDEVPRIVATE+20)
#define	SAR_SET_PVC_NUMBER			(SIOCDEVPRIVATE+21)
#define	SAR_READ_MEM				(SIOCDEVPRIVATE+22)
#define	SAR_WRITE_MEM				(SIOCDEVPRIVATE+23)
#ifdef AUTO_PVC_SEARCH
#define	SENT_STARTUP_PID			(SIOCDEVPRIVATE+25)
#endif
#define 	SENT_TR069_PID			(SIOCDEVPRIVATE+27)
#define	QoS_UBR		0
#define	QoS_CBR		1
#define	QoS_nrtVBR		2
#define	QoS_rtVBR		3

//ql
#if 0
#define SAR_SET_TRAFFIC_CTL			(SIOCDEVPRIVATE+29)
#endif
#define QOS_ENABLE_IMQ					(SIOCDEVPRIVATE+29)

#define SAR_SET_BRIDGE_MODE			(SIOCDEVPRIVATE+30)
#ifdef CONFIG_RTL8672
#define SAR_SET_SARHDR		 		(SIOCDEVPRIVATE+31)
#define SAR_SET_PKTA		 		(SIOCDEVPRIVATE+32)
#endif
#define	SENT_IPTV_INTF				(SIOCDEVPRIVATE+34)
#define PVC_QOS_TYPE				(SIOCDEVPRIVATE+35)

// for DSL device
#define DSL_DEBUG_EOC_ENABLE			(SIOCDEVPRIVATE+21)
#define DSL_DEBUG_NUM2PRINT			(SIOCDEVPRIVATE+22)
#define DSL_DEBUG_STATUS			(SIOCDEVPRIVATE+23)
#define DSL_DEBUG_EOC_LBRX			(SIOCDEVPRIVATE+24)
#define DSL_DEBUG_EOC_LBTX			(SIOCDEVPRIVATE+25)

/*--------------------------------
		Structures
---------------------------------*/

#if 0  // moved to linux/atm.h
typedef struct ch_stat{

	/* RX program flow related */
	INT32	rcv_cnt;			/* count of Receive Functiont calls */
	INT32	rcv_ok;			/* packet return count */

	/* RX data statistics related */
	INT32	rx_desc_cnt;	/* count of descriptors that receive Functiont walks through */
	INT32	rx_byte_cnt;	/* received byte count */
	INT32	rx_pkt_cnt;
	INT32	rx_pkt_fail;

	INT32	rx_FS_cnt;		/* count of FS desc */
	INT32	rx_LS_cnt;		/* count of LS desc */
	INT32	rx_desc_ok_cnt;	/* rx ok descriptors */
	INT32 	rx_oam_count;	/* rx oam cell count */
	INT32	rx_buf_alloc;	/* rx buffer allocated */
	INT32	rx_buf_lack;		/* lack of rx buffer */
	INT32	rx_desc_fail;	/* rx desc errors */
	INT32	rx_crc_error;	/* rx AAL5 CRC error count */
	INT32	rx_lenb_error;	/* rx packet length too large */
	INT32	rx_lens_error;	/* rx packet length too small */

	INT32	rx_netif_cnt;	/* count for netif calls */

	/* TX program flow related */
	INT32	send_cnt;		/* count of Send function calls */
	INT32	send_ok;		/* count of Send function ok */
	INT32	send_fail;		/* count of Send function errors */
	INT32	send_desc_full;	/* descriptors full */
	INT32	send_desc_lack;	/* not enough free descriptors */

	/* TX data statistics related */
	INT32	tx_desc_ok_cnt;	/* tx ok descriptors (successfully transmitted) */
	INT32	tx_buf_free;		/* tx buffer freed */
	INT32	tx_pkt_ok_cnt;	/* tx ok packets (successfully transmitted)*/
	INT32	tx_pkt_fail_cnt;	/* tx fail packets (send into descritprtor ring failed) */
	INT32	tx_byte_cnt;		/* send byte count (send into descriptor ring) */

}ch_stat;

typedef struct Traffic_Manage{
	UINT32	tick_now;
	INT32	cell_cnt;
	INT8		Type;
	UINT16	PCR;
	UINT16	SCR;
	UINT8	MBS;
	UINT8	CRD;
	UINT32	CDVT;
}Traffic_Manage;

struct SAR_IOCTL_CFG {
	int8				ch_no;
	uint8			vpi;					//vpi number
	uint16			vci;					//vci number
	int				rfc;
	int				framing;
	int				created;
	int				loopback;
	uint8			MAC[6];
	Traffic_Manage	QoS;
	ch_stat			stat;
};
#endif

typedef struct {
    uint16      mac47_32;
    uint16      mac31_16;
    uint16      mac15_0;
} macaddr_t;

/*--------------------------------
			Routines
---------------------------------*/

int 		s2i(char *str_P);
void 	DumpStat(struct SAR_IOCTL_CFG cfg);
void 	DumpStat_2(struct SAR_IOCTL_CFG cfg);
void 	DumpCfg(struct SAR_IOCTL_CFG cfg);
void		AccumCfg(struct SAR_IOCTL_CFG *total_cfg, struct SAR_IOCTL_CFG cfg);
BOOL	Parsing_Cfg(struct SAR_IOCTL_CFG *cfg, char **spp);
void 	Dump(uint32 Buffer, int32 size);
int32 s2Mac(macaddr_t *mac_P, uint8 *str_P);
