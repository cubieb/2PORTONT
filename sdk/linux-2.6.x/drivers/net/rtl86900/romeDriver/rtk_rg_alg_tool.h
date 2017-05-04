#ifndef RTK_RG_ALG_TOOL_H
#define RTK_RG_ALG_TOOL_H

//#include <rtk_rg_struct.h>
#include <rtk_rg_internal.h>
#include <rtk_rg_apollo_liteRomeDriver.h>	//FIXME: temporary incldue for calling rtk_rg_apollo_*

//siyuan add for alg function
#define MAX_ALG_CONNECTION_SIZE		64
#define MAX_ALG_EXPECT_SIZE			64
#define MAX_ALG_EXPECT_HASH_SIZE	48
#define MAX_ALG_EXPECT_LIST_SIZE	(MAX_ALG_EXPECT_SIZE - MAX_ALG_EXPECT_HASH_SIZE)
#define ALG_EXPECT_INACTIVE			0x2
#define ALG_EXPECT_GET_FROM_LIST 	0x4

typedef enum rtk_rg_alg_connType_e
{
	ALG_CONNTYPE_START=0,
	ALG_CONNTYPE_H323,
	ALG_CONNTYPE_SIP,
	ALG_CONNTYPE_PPTP,
}rtk_rg_alg_connType_t;

//siyuan add for alg function
typedef struct rtk_rg_alg_newPort_s{
	uint16 newPort;
	uint16 newExtPort;
} rtk_rg_alg_newPort_t;

typedef struct rtk_rg_alg_newAddress_s{
	uint32 newIp;
	uint16 newPort;
} rtk_rg_alg_newAddress_t;

typedef struct rtk_rg_alg_h323_s{
	// one for outbound packet and the other for inbound packet
	unsigned short tpktLen[2];
	// if only get 4 bytes data of TPKT header, 
	// set it to 0 to wait for Q931 data in another packet
	int receiveData[2]; 
	//used for RAS message registrationRequest and registrationComfirm
	uint16 signal_port[2];
} rtk_rg_alg_h323_t;

typedef struct rtk_rg_alg_sip_s{
#ifdef __KERNEL__
	struct hlist_head expects; 	/*expects added by the same connection */
#endif
	unsigned int invite_cseq;
	unsigned int register_cseq;
} rtk_rg_alg_sip_t;

typedef struct rtk_rg_alg_pptp_s 
{
	unsigned short remoteCallID;
	unsigned short externalCallID;
	unsigned short internalCallID;
}rtk_rg_alg_pptp_t;

union rtk_rg_alg_addr {
	uint32      all[4];
	uint32		ip;
	uint32		ip6[4];
};

typedef struct rtk_rg_alg_tuple_s{
    //must store ipv4 address,port information of host byte order
    //ipv6 address is network byte order
	int isTcp;
	int isIp6;
	union rtk_rg_alg_addr extIp; 
	uint16 extPort;
	union rtk_rg_alg_addr internalIp;
	uint16 internalPort;
	union rtk_rg_alg_addr remoteIp;
	uint16 remotePort;
}rtk_rg_alg_tuple_t;

typedef struct rtk_rg_alg_connection_s {	
	int valid;
	rtk_rg_alg_tuple_t tuple;
	int (*appHandler)(int direct, int after, unsigned char *pSkb,unsigned char * pPktInfo, unsigned char * pConnInfo);
	unsigned char * skb;
	unsigned char * pPktHdr;
	unsigned int appOff; /*offset of application layer */
	/*used to sync inbound and outbound acknowledgement*/
	int inDelta;	
	int outDelta;
	
	int oldInDelta;	
	int oldOutDelta;
	unsigned int oldInSeq;		//record for sequence number
	unsigned int oldOutSeq;		//record for sequence number
	
	int direct; 
	union{
		rtk_rg_alg_h323_t h323;
		rtk_rg_alg_sip_t sip;
		rtk_rg_alg_pptp_t pptp;
	}app;
	rtk_rg_alg_connType_t appType;	//indicate which app this conn stands for
	struct rtk_rg_alg_connection_s *pNext,*pPrev;
}rtk_rg_alg_connection_t;

typedef struct rtk_rg_alg_expect_s {
#ifdef __KERNEL__
	struct hlist_node lnode; /* expect list member */
#endif
	int valid;
	int mask;
	unsigned int flags;
	int (*appHandler)(int direct, int after, unsigned char *pSkb,unsigned char * pPktInfo, unsigned char * pConnInfo);
	rtk_rg_alg_tuple_t tuple;
	unsigned long expire;
	int hash;
	int index;
}rtk_rg_alg_expect_t;

typedef struct rtk_rg_alg_expect_list_node_s {
	struct rtk_rg_alg_expect_list_node_s * pNext;
	int index;
}rtk_rg_alg_expect_list_node_t;


/* shared data structure */
//extern rtk_rg_globalDatabase_t	rg_db;
typedef struct rtk_rg_algDatabase_s
{
	//siyuan alg connection information
	rtk_rg_alg_connection_t * pAlgConnectionListHead;
	rtk_rg_alg_connection_t algConnectionList[MAX_ALG_CONNECTION_SIZE];
		
	//siyuan alg expection information
	rtk_rg_alg_expect_t * pAlgExpectListHead;
	rtk_rg_alg_expect_t algExpect[MAX_ALG_EXPECT_SIZE];
	rtk_rg_alg_expect_list_node_t * pAlgExpectFreeListHead;
	rtk_rg_alg_expect_list_node_t  algExpectFreeList[MAX_ALG_EXPECT_LIST_SIZE];
	rtk_rg_alg_expect_list_node_t * pAlgExpectHashList[MAX_ALG_EXPECT_HASH_SIZE>>2];

	uint32	algPPTPExtCallIDEnabled[65536/32]; // algPPTPExtCallIDEnabled[0] bit0==>callID 0 has been used, bit31==> callID 31 has been used, algPPTPExtCallIDEnabled[1] bit0==>callID 32 has been used...	
}rtk_rg_algDatabase_t;

//interfaces for romedriver to call
int rtk_rg_fwdEngine_connection_inbound_check(void *data,ipaddr_t *transIP,uint16 *transPort);

int rtk_rg_alg_expect_forward(int direct, int after, unsigned char *pSkb,unsigned char * pPktInfo);


//functions for alg inner call
int rtk_rg_alg_addr_cmp(const union rtk_rg_alg_addr *a1,const union rtk_rg_alg_addr *a2);

int _rtk_rg_alg_init_tuple(int direct, int after, rtk_rg_pktHdr_t *pPktHdr, rtk_rg_alg_tuple_t * pTuple);

rtk_rg_alg_connection_t *  _rtk_rg_alg_connection_find(rtk_rg_alg_tuple_t * pTuple);

rtk_rg_alg_connection_t *  _rtk_rg_alg_connection_add(rtk_rg_alg_tuple_t * pTuple);

rtk_rg_alg_expect_t * _rtk_rg_alg_expect_add(int direct, rtk_rg_alg_tuple_t * pTuple, int * isNew);

rtk_rg_alg_expect_t * _rtk_rg_alg_expect_find(int direct, rtk_rg_alg_tuple_t * pTuple);

void _rtk_rg_alg_expect_del(rtk_rg_alg_expect_t * pExpect);

int rtk_rg_skip_wspace_lws(const unsigned char * pData, unsigned int * pOff, unsigned int dataLen);

int _rtk_rg_mangle_packet(unsigned char ** ppData, unsigned int * pDataLen, rtk_rg_alg_connection_t * pConn,
				  unsigned int matchOff, unsigned int matchLen, const char *buffer, unsigned int bufLen);

int _rtk_rg_update_tcp_seqNdelta(int direct, rtk_rg_pktHdr_t * pPktHdr, rtk_rg_alg_connection_t * pConn);

int _rtk_rg_sync_tcp_ack_seq(int direct, rtk_rg_pktHdr_t * pPktHdr, rtk_rg_alg_connection_t * pConn);

int _rtk_rg_extPortPair_get(int isTcp, uint16 port);

int _rtk_rg_extPort_get(int isTcp, uint16 port);

int _rtk_rg_parse_addr(const char *cp, unsigned int * size, union rtk_rg_alg_addr * addr,
                      int dataLen, rtk_rg_alg_connection_t * pConn);

void _rtk_rg_alg_resetAllDataBase(void);
int _rtk_rg_alg_displayAllDataBase(struct seq_file *s, void *v);


#endif
