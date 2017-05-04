/*
* Copyright c                  Realsil Semiconductor Corporation, 2006
* All rights reserved.
* 
* Program :  igmp snooping function
* Abstract : 
* Author :qinjunjie 
* Email:qinjunjie1980@hotmail.com
*
*/

#ifndef RTK_RG_IGMP_SNOOPING_H
#define RTK_RG_IGMP_SNOOPING_H

#include <rtk_rg_struct.h>

#ifdef __KERNEL__
#include <linux/list.h>
#endif

#define CONFIG_RG_NEW_IGMP_IMPLEMENTATION 1
#define CONFIG_RG_HARDWARE_MULTICAST 1
#define RG_IGMP_SNOOPING_MODULE_NAME "IGMP_SNOOP" 


extern rtk_rg_globalDatabase_t	rg_db;
#ifdef __KERNEL__
extern rtk_rg_globalKernel_t	rg_kernel;
#endif


#if 0
#define DBG_IGMPSNOOPING
#ifdef DBG_IGMPSNOOPING
#define DBG_IGMPSNOOPING_PRK printk
#else
#define DBG_IGMPSNOOPING_PRK(format, args...)
#endif
#endif


#ifdef CONFIG_RG_HARDWARE_MULTICAST
typedef struct rtl_multicast_index_mapping
{
	uint32 	groupIP[4];
	int		hwflow_index;
	int		ipVersion;
	int		vlanId;
	uint32	fwdmembr;
	struct list_head entry;

}rtl_multicast_index_mapping_t;
#endif


/***********************************Utilities************************************************/
#define IS_CLASSD_ADDR(ipv4addr)				((((uint32)(ipv4addr)) & 0xf0000000) == 0xe0000000)
#define RESERVE_MULTICAST_ADDR1 	0xEFFFFFFA
#define IN_MULTICAST_RESV1(addr)	((((uint32)(addr)) & 0xFFFFFF00) == 0xe0000000)	// 224.0.0.x
#define IN_MULTICAST_RESV2(addr)	((((uint32)(addr)) & 0xFF000000) == 0xEF000000)	// 239.0.0.0~239.255.255.255

#define IS_IPV6_MULTICAST_ADDRESS(ipv6addr)	((ipv6addr[0] & 0xFF000000)==0xff000000)
#define IS_IPV4_MULTICAST_ADDRESS(ipv4addr)	(IS_CLASSD_ADDR(ipv4addr[0]))		

#define PORT0_MASK 0x01
#define PORT1_MASK 0x02
#define PORT2_MASK 0x04
#define PORT3_MASK 0x08
#define PORT4_MASK 0x10
#define PORT5_MASK 0x20

#ifdef CONFIG_RG_NEW_IGMP_IMPLEMENTATION
#define MAX_SUPPORT_PORT_NUMBER 16
#else
#define MAX_SUPPORT_PORT_NUMBER 10
#endif
#define DEFAULT_HASH_TABLE_SIZE 64
#define MAX_HASH_TABLE_SIZE 1024
#define MAX_MCAST_MODULE_NUM 8

#define NON_PORT_MASK 0x00
#define IPV4_MCAST_MAC_MASK 0x007fffff
#define IPV6_MCAST_MAC_MASK 0x00ffffff



/* IGMP snooping default  parameters */
#define DEFAULT_MAX_GROUP_COUNT					     256	         /* default max group entry count **/
#define DEFAULT_MAX_SOURCE_COUNT                                  300         /*default max source entry count*/
#define DEFAULT_MAX_CLIENT_COUNT			256
#define DEFAULT_MAX_FLOW_COUNT			256

#define DEFAULT_GROUP_MEMBER_INTERVAL 260             /* IGMP group member interval, default is 260 seconds */
#define DEFAULT_LAST_MEMBER_INTERVAL  10             /* IGMP last member query time, default is 2 seconds */
#define DEFAULT_QUERIER_PRESENT_TIMEOUT 260             /* IGMP  querier present timeout, default is 260 seconds */

#define DEFAULT_MCAST_FLOW_EXPIRE_TIME 	15

#define DEFAULT_DVMRP_AGING_TIME		120           
#define DEFAULT_PIM_AGING_TIME		120           
#define DEFAULT_MOSPF_AGING_TIME		120

#define IP_VERSION4 4
#define IP_VERSION6 6
#define BOTH_IPV4_IPV6 0x46

#define CPUTAGPROTOCOL 0x09
#define VLAN_PROTOCOL_ID 0x8100

#define IPV4_ETHER_TYPE 0x0800
#define IPV6_ETHER_TYPE 0x86DD

#define PPPOE_ETHER_TYPE 0x8864
#define PPP_IPV4_PROTOCOL 0x0021
#define PPP_IPV6_PROTOCOL 0x0057
#define ROUTER_ALERT_OPTION 0x94040000

#define  HOP_BY_HOP_OPTIONS_HEADER 0
#define ROUTING_HEADER 43
#define  FRAGMENT_HEADER 44
#define DESTINATION_OPTION_HEADER 60
#define NO_NEXT_HEADER 59
#define ICMP_PROTOCOL 58

#define DVMRP_ADDR  0xE0000004
#define DVMRP_TYPE 0x13
#define DVMRP_PROTOCOL 3

#define PIM_PROTOCOL 103
#define IPV4_PIM_ADDR 0xE000000D
#define IS_IPV6_PIM_ADDR(ipv6addr) ((ipv6addr[0] == 0xFF020000)&&(ipv6addr[1] == 0x00000000)&&(ipv6addr[2] == 0x00000000)&&(ipv6addr[3] ==0x0000000D)) 
#define FF02_0000_0000_000D "\17\17\0\2\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\15"
#define IPV6_PIM_ADDR  FF02_0000_0000_000D

#define MOSPF_PROTOCOL 89
#define MOSPF_HELLO_TYPE 1
#define IPV4_MOSPF_ADDR1  0xE0000005
#define IPV4_MOSPF_ADDR2  0xE0000006
#define IS_IPV6_MOSPF_ADDR1(ipv6addr) ((ipv6addr[0] == 0xFF020000)&&(ipv6addr[1] == 0x00000000)&&(ipv6addr[2] == 0x00000000)&&(ipv6addr[3] ==0x00000005)) 		
#define IS_IPV6_MOSPF_ADDR2(ipv6addr) ((ipv6addr[0] == 0xFF020000)&&(ipv6addr[1] == 0x00000000)&&(ipv6addr[2] == 0x00000000)&&(ipv6addr[3] ==0x00000006)) 
#define FF02_0000_0000_0005 "\17\17\0\2\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\5"
#define IPV6_MOSPF_ADDR1 FF02_0000_0000_0005
#define FF02_0000_0000_0006 "\17\17\0\2\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\6"
#define IPV6_MOSPF_ADDR2 FF02_0000_0000_0006


#define IGMP_PROTOCOL 2
#define IGMP_ALL_HOSTS_ADDR	0xE0000001	/*general query address*/
#define IGMP_ALL_ROUTERS_ADDR	0xE0000002  /*leave report address*/
#define IGMPV3_REPORT_ADDR 0xE0000016


#define IS_MLD_ALL_HOSTS_ADDRESS(ipv6addr) ((ipv6addr[0] == 0xFF020000)&&(ipv6addr[1] == 0x00000000)&&(ipv6addr[2] == 0x00000000)&&(ipv6addr[3] ==0x00000001))

	
#define IS_MLD_ALL__ROUTER_ADDRESS(ipv6addr) ((ipv6addr[0] == 0xFF020000)&&(ipv6addr[1] ==0x00000000)&&(ipv6addr[2] ==0x00000000)&&(ipv6addr[3] == 0x00000002))

	
#define IS_MLDv2_REPORT_ADDRESS(ipv6addr) ((ipv6addr[0] == 0xFF020000)&&(ipv6addr[1] == 0x00000000)&&(ipv6addr[2] == 0x00000000)&&(ipv6addr[3] ==0x00000016))


/*            IGMP type              */
#define IGMP_QUERY 0x11
#define IGMPV1_REPORT 0x12
#define IGMPV2_REPORT 0x16
#define IGMPV2_LEAVE 0x17
#define IGMPV3_REPORT 0x22

/*             MLD type          */
#define MLD_QUERY 130
#define MLDV1_REPORT 131
#define MLDV1_DONE 132
#define MLDV2_REPORT 143
#define S_FLAG_MASK 0x08

#define MODE_IS_INCLUDE			                      1
#define MODE_IS_EXCLUDE			                      2
#define CHANGE_TO_INCLUDE_MODE	                      3
#define CHANGE_TO_EXCLUDE_MODE		               4
#define ALLOW_NEW_SOURCES                                  5
#define BLOCK_OLD_SOURCES			                      6

#define	FILTER_MODE_INCLUDE		0
#define	FILTER_MODE_EXCLUDE		1

		/*             IGMP version                       */
#define IGMP_V1					    1
#define IGMP_V2						2
#define IGMP_V3						3

#define MLD_V1 						4
#define MLD_V2						5


/*rtl_multicastSysTimerExpired() check type */
#define SYS_EXPIRED_NORMAL 			0
#define SYS_EXPIRED_USER_FORCE		1



#define IPV4_ROUTER_ALTER_OPTION 0x94040000
#define IPV6_ROUTER_ALTER_OPTION 0x05020000
#define IPV6_HEADER_LENGTH 40

/**********************IGMPv3 exponential field decoding ******************************/ 
#define	RG_IGMPV3_MASK(value, nb)		((nb)>=32 ? (value) : ((1<<(nb))-1) & (value))
#define	RG_IGMPV3_EXP(thresh, nbmant, nbexp, value) \
			((value) < (thresh) ? (value) : \
			((RG_IGMPV3_MASK(value, nbmant) | (1<<(nbmant))) << \
			(RG_IGMPV3_MASK((value) >> (nbmant), nbexp) + (nbexp))))		
			
#define	RG_IGMPV3_QQIC(value)			RG_IGMPV3_EXP(0x80, 4, 3, value)
#define	RG_IGMPV3_MRC(value)			RG_IGMPV3_EXP(0x80, 4, 3, value)
/********************************************************************************/


/**************************MLDv2 exponential field decoding ****************************/
#define   RG_MLDV2_MASK(value, nb) 	((nb)>=32 ? (value) : ((1<<(nb))-1) & (value))
#define   RG_MLDV2_EXP(thresh, nbmant, nbexp, value)\
	              ((value) < (thresh) ? (value) : \
			((RG_MLDV2_MASK(value, nbmant) | (1<<(nbmant))) << \
			(RG_MLDV2_MASK((value) >> (nbmant), nbexp) + (nbexp))))
			
#define   RG_MLDV2_QQIC(value)			RG_MLDV2_EXP(0x80, 4, 3, value)
#define   RG_MLDV2_MRC(value)			RG_MLDV2_EXP(0x8000, 12, 3, value)
/*------------------------------------------------------------------------------*/


/* multicast configuration*/
struct rtl_mCastSnoopingGlobalConfig
{
	uint32 maxGroupNum;
	uint32 maxClientNum;
	uint32 maxSourceNum;
	uint32 hashTableSize;
	
	uint32 groupMemberAgingTime;                
	uint32 lastMemberAgingTime;			
	uint32 querierPresentInterval;                   
	
	uint32 dvmrpRouterAgingTime;			
	uint32 mospfRouterAgingTime;                     
	uint32 pimRouterAgingTime;     
};

struct rtl_mCastSnoopingLocalConfig
{
	uint8 enableSourceList;
	uint8 enableFastLeave;
	uint8   gatewayMac[6];
	uint32 gatewayIpv4Addr;
	uint32 gatewayIpv6Addr[4];
	uint32 unknownMcastFloodMap;
	uint32 staticRouterPortMask;
};

 struct rtl_mCastTimerParameters
 {
	uint32 disableExpire;
	uint32 groupMemberAgingTime;              
	uint32 lastMemberAgingTime;			   
	uint32 querierPresentInterval;                   
	
	uint32 dvmrpRouterAgingTime;				  /*DVMRP multicast router aging time*/
	uint32 mospfRouterAgingTime;                           /*MOSPF multicast router aging time*/
	uint32 pimRouterAgingTime;                          /*PIM-DM multicast router aging time*/
};

#define IP_VERSION4 4
#define IP_VERSION6 6

struct rtl_multicastDataInfo
{
	uint32 ipVersion;
	uint16 vlanId;
	uint32 sourceIp[4];
	uint32 groupAddr[4];

};

struct rtl_multicastFwdInfo
{
	uint8 unknownMCast;
	uint8 reservedMCast;
	uint16 cpuFlag;
	uint32 fwdPortMask;
	
};

struct rtl_groupInfo
{
	uint32 ownerMask;
};

typedef struct rtl_multicastDeviceInfo_s
{
	char devName[32];
	uint32 vlanId;
	uint32 portMask;
	uint32 swPortMask;
}rtl_multicastDeviceInfo_t;


typedef struct rtl_multicastEventContext_s
{
	char devName[16];
	uint32 moduleIndex;
	uint32 ipVersion;
	uint16 vlanId;
	uint32 groupAddr[4];
	uint32 sourceAddr[4];
	uint32 portMask;
}rtl_multicastEventContext_t;


typedef struct rtl_igmpPortInfo_s
{
	uint32 linkPortMask;
}rtl_igmpPortInfo_t;

/******************************************************
	Function called in the system initialization 
******************************************************/

int32 rtl_initMulticastSnooping(struct rtl_mCastSnoopingGlobalConfig mCastSnoopingGlobalConfig);
int32 rtl_exitMulticastSnooping(void);
void rtl_setMulticastParameters(struct rtl_mCastTimerParameters mCastTimerParameters);

int32 rtl_registerIgmpSnoopingModule(uint32 *moduleIndex);

int32 rtl_setIgmpSnoopingModuleDevInfo(uint32 moduleIndex,rtl_multicastDeviceInfo_t *devInfo);
int32 rtl_getIgmpSnoopingModuleDevInfo(uint32 moduleIndex,rtl_multicastDeviceInfo_t *devInfo);

int32 rtl_setIgmpSnoopingModuleStaticRouterPortMask(uint32 moduleIndex,uint32 staticRouterPortMask);
int32 rtl_getIgmpSnoopingModuleStaticRouterPortMask(uint32 moduleIndex,uint32 *staticRouterPortMask);

int32 rtl_getDeviceIgmpSnoopingModuleIndex(rtl_multicastDeviceInfo_t *devInfo,uint32 *moduleIndex);

int32 rtl_unregisterIgmpSnoopingModule(uint32 moduleIndex);

int32 rtl_configIgmpSnoopingModule(uint32 moduleIndex, struct rtl_mCastSnoopingLocalConfig *multicastSnoopingConfig);

int32 rtl_igmpMldProcess(uint32 moduleIndex, uint8 * macFrame,  rtk_rg_pktHdr_t * pPktHdr, uint32 portNum, uint32 *fwdPortMask);

int32 rtl_getMulticastDataFwdPortMask(uint32 moduleIndex, struct rtl_multicastDataInfo *multicastDataInfo, uint32 *fwdPortMask);

int32 rtl_getMulticastDataFwdInfo(uint32 moduleIndex, struct rtl_multicastDataInfo * multicastDataInfo, struct rtl_multicastFwdInfo *multicastFwdInfo);

int32 rtl_maintainMulticastSnoopingTimerList(uint32 currentSystemTime);

int32 rtl_compareMacAddr(uint8* macAddr1, uint8* macAddr2);
int32 rtl_compareIpv6Addr(uint32* ipv6Addr1, uint32* ipv6Addr2);
int32 rtl_checkMCastAddrMapping(uint32 ipVersion, uint32 *ipAddr, uint8* macAddr);

#ifdef CONFIG_PROC_FS
int igmp_show(struct seq_file *s, void *v);
int igmp_groupList_memDump(struct seq_file *s, void *v);
#endif

int32 rtl_getGroupInfo(uint32 groupAddr, struct rtl_groupInfo *groupInfo);
int32 rtl_flushAllIgmpRecord(int forceFlush);



struct ipv4Pkt 
{
	uint8	vhl;		/* version + Header Length */
	uint8	typeOfService;			
	uint16	length;		/* total length */
	
	uint16	identification;	/* identification */
	uint16	offset;
	
	uint8	ttl;		/* time to live */
	uint8	protocol;
	uint16	checksum;	/* Header Checksum */			
	uint32	sourceIp;
	uint32	destinationIp;
};

struct igmpPkt
{
	uint8 type;		/* type*/
	uint8 maxRespTime;	/*maximum response time,unit:0.1 seconds*/
	uint16 checksum;                   
	uint32 groupAddr;
};

struct igmpv1Pkt
{
	uint8 VersionType;	/*4bits: version, 4bits:type*/
	uint8 unused;
	uint16 checkSum;	/*IGMP packet checksum*/
	uint32 groupAddr;	/*multicast group address*/
};

struct igmpv2Pkt
{
	uint8 type;		/* type*/
	uint8 maxRespTime;	/*maximum response time,unit:0.1 seconds*/
	uint16 checkSum;                   
	uint32 groupAddr;
};

struct igmpv3Query
{
	uint8 type;		/*query type*/
	uint8 maxRespCode;	/*maximum response code*/
	uint16 checkSum;	/*IGMP checksum*/
	uint32 groupAddr;	/*multicast group address*/
	uint8 rsq;		/* 4bit: reserved, 1bit: suppress router-side processing, 3bit: querier's robustness variable*/
	uint8 qqic;		/* querier's query interval code */
	uint16 numOfSrc;	/* number of sources */
	uint32 srcList;		/* first entry of source list */	
};

struct groupRecord
{
	uint8	type;		/* Record Type */
	uint8	auxLen;		/* auxiliary data length, in uints of 32 bit words*/
	uint16	numOfSrc;	/* number of sources */
	uint32	groupAddr;	/* group address being reported */
	uint32	srcList;	/* first entry of source list */
};

struct igmpv3Report
{
	uint8	type;		/* Report Type */
	uint8	reserved1;
	uint16	checkSum;	/*IGMP checksum*/
	uint16	reserved2;
	uint16	numOfRecords;	/* number of Group records */
	struct	groupRecord recordList;/*first entry of group record list*/
};
 
struct ipv6Pkt
{
	uint32  vtf;		/*version(4bits),  traffic class(8bits), and flow label(20bits)*/
	
	uint16	payloadLenth;	/* payload length */
	uint8	nextHeader;	/* next header */
	uint8	hopLimit;	/* hop limit*/
	
	uint32	sourceAddr[4];	/*source address*/
	uint32	destinationAddr[4];	/* destination address */
};


struct mldv1Pkt
{
	uint8 type;                                   
	uint8 code;		/*initialize by sender, ignored by receiver*/
	uint16 checkSum;
	uint16 maxResDelay;	/*maximum response delay,unit:0.001 second*/ 
	uint16 reserved;
	uint32 mCastAddr[4];	/*ipv6 multicast address*/  
};


struct mldv2Query
{
	uint8 type;
	uint8 code;		/*initialize by sender, ignored by receiver*/
	uint16 checkSum;
	uint16 maxResCode;	/*maximum response code,unit:0.001 second*/ 
	uint16 reserved;
	uint32 mCastAddr[4];
	uint8 rsq;		/* 4bits: reserved, 1bit: suppress router-side processing, 3bits: querier's robustness variable*/
	uint8 qqic;		/* querier's query interval code */
	uint16 numOfSrc;	/* number of sources */
	uint32 srcList;
};

struct mCastAddrRecord
{
	uint8	type;		/* Record Type */
	uint8	auxLen;		/* auxiliary data length, in uints of 32 bit words*/
	uint16	numOfSrc;	/* number of sources */
	uint32	mCastAddr[4];	/* group address being reported */
	uint32	srcList;	/* first entry of source list */
};

struct mldv2Report
{
	uint8 type;
	uint8  reserved1;                                  
	uint16 checkSum;                           
	uint16 reserved2;                    
	uint16 numOfRecords;	/* number of multicast address records */
	struct mCastAddrRecord recordList;
};

struct ipv4MospfHdr
{
	uint8 version;
	uint8 type;
	uint16 pktLen;
	uint32 routerId;
	uint32 areaId;
	uint16 CheckSum;
	uint16 auType;
	uint32 authentication[2];	
};

struct ipv4MospfHello
{
	struct ipv4MospfHdr hdr;
	uint32 netWorkMask;
	uint16 helloInterVal;
	uint8  options;          /*  X-X-DC-EA-N/P-MC-E-X */
	uint8 routerPriority;
	uint32 routerDeadInterval;
	uint32 designatedRouter;
	uint32 backupDesignatedRouter;
	uint32 neighbor;
};

struct ipv6MospfHdr
{
	uint8 version; /*vesion is 3*/
	uint8 type;
	uint16 pktLen;
	uint32 routerId;
	uint32 areaId;
	uint16 CheckSum;
	uint8 instanceId;
	uint8 zeroData;
};

struct ipv6MospfHello
{
	struct ipv6MospfHdr hdr;
	uint32 interfaceId;
	uint8 routerPriority;
	uint8  options[3];          /*X-X-DC-R-N-MC-E-V6 */
	uint16 helloInterval;
	uint16 routerDeadInterval;
	uint32 designatedRouter;
	uint32 backupDesignatedRouter;
	uint32 neighbor;
};

struct ipv6PseudoHeader
{
	uint32 sourceAddr[4];
	uint32 destinationAddr[4];
	uint32 upperLayerPacketLength;
	uint8  zeroData[3];
	uint8  nextHeader;
};

struct ipv4PseudoHeader
{
	uint32 sourceAddr;
	uint32 destinationAddr;
	uint8 zero;
	uint8  protocol;
	uint16  payloadLen;
};


union pseudoHeader 
{
	struct ipv6PseudoHeader ipv6_pHdr;
	struct ipv4PseudoHeader ipv4_pHdr;
};

#ifdef CONFIG_RG_NEW_IGMP_IMPLEMENTATION
struct rtl_groupEntry
{
	struct rtl_groupEntry *previous;
	struct rtl_groupEntry  *next;             /*Pointer of next group entry*/
	uint32 ipVersion;
	uint16 vlanId;
	#ifdef CONFIG_RG_MLD_SNOOPING
	uint32 groupAddr[4];
	#else
	uint32 groupAddr[1];
	#endif

	struct	rtl_clientEntry *clientList;
};

struct rtl_clientEntry
{
	uint16 igmpVersion;
	uint16 portNum;
	struct rtl_clientEntry *previous;
	struct rtl_clientEntry  *next;	/*Pointer of next group entry*/
	#ifdef CONFIG_RG_MLD_SNOOPING
	uint32 clientAddr[4];		/*client ip address*/
	#else
	uint32 clientAddr[1];		/*client ip address*/
	#endif
	struct rtl_sourceEntry *sourceList;	/*this is the server source ip list*/
	uint32 groupFilterTimer;
	unsigned long int reportUpdateJiffies;	/*record the join/report packet received time*/	
};


struct rtl_sourceEntry
{
	#ifdef CONFIG_RG_MLD_SNOOPING
	uint32 sourceAddr[4];		/*D class IP multicast address*/
	#else
	uint32 sourceAddr[1];		/*D class IP multicast address*/
	#endif
	uint32 fwdState;
	struct rtl_sourceEntry *previous; 
	struct rtl_sourceEntry *next;	/*Pointer of next group entry*/
	uint32 portTimer;   
};

struct rtl_mcastFlowEntry
{
	struct rtl_mcastFlowEntry *previous;
	struct rtl_mcastFlowEntry  *next;             /*Pointer of next group entry*/
	uint32 ipVersion;
	#ifdef CONFIG_RG_MLD_SNOOPING
	uint32 serverAddr[4];
	uint32 groupAddr[4];
	#else
	uint32 serverAddr[1];
	uint32 groupAddr[1];
	#endif
	
//	uint32 fwdPortMask;
	struct rtl_multicastFwdInfo multicastFwdInfo;
	uint32 refreshTime;
};
#else
struct rtl_groupEntry
{
	struct rtl_groupEntry *previous;
	struct rtl_groupEntry  *next;             /*Pointer of next group entry*/
	struct rtl_sourceEntry *sourceList;
	uint32 ipVersion;
	#ifdef CONFIG_RG_MLD_SNOOPING
	uint32 groupAddr[4];
	#else
	uint32 groupAddr[1];
	#endif
	uint32 groupFilterTimer[MAX_SUPPORT_PORT_NUMBER];
};

struct rtl_sourceEntry
{
	#ifdef CONFIG_RG_MLD_SNOOPING
	uint32 sourceAddr[4];                       /*D class IP multicast address*/
	#else
	uint32 sourceAddr[1];                       /*D class IP multicast address*/
	#endif
	struct rtl_sourceEntry *previous; 
	struct rtl_sourceEntry *next;             /*Pointer of next group entry*/
	uint32 portTimer[MAX_SUPPORT_PORT_NUMBER];   
};
#endif

struct rtl_mcastRouter
{	
	uint32 portTimer[MAX_SUPPORT_PORT_NUMBER];
};


struct rtl_macFrameInfo
{
	uint8   ipVersion;

	uint32 srcIpAddr[4];
	uint32 dstIpAddr[4];

	uint8 *ipBuf;
	uint16   ipHdrLen;
	uint8   l3Protocol;
	uint8 checksumFlag;
	uint8 *l3PktBuf;
	uint16 l3PktLen;
	uint16 macFrameLen;
	uint16 vlanTagFlag;
	uint16 vlanTagID;
};


struct rtl_multicastRouters
{
	struct rtl_mcastRouter querier;
	struct rtl_mcastRouter dvmrpRouter;
	struct rtl_mcastRouter mospfRouter;
	struct rtl_mcastRouter pimRouter;
};
#ifdef CONFIG_RG_NEW_IGMP_IMPLEMENTATION
//#define CONFIG_RECORD_MCAST_FLOW
#endif

struct rtl_multicastModule
{
	uint8 enableSnooping;
#ifdef CONFIG_RG_NEW_IGMP_IMPLEMENTATION
#else
	uint8 enableSourceList;
#endif
	uint8 enableFastLeave;

	uint32 unknownMCastFloodMap;
	uint32 staticRouterPortMask;
	
	rtl_multicastDeviceInfo_t deviceInfo;

	struct rtl_multicastRouters rtl_ipv4MulticastRouters;
#ifdef CONFIG_RG_MLD_SNOOPING	
	struct rtl_multicastRouters rtl_ipv6MulticastRouters;
#endif

	/*gateway infomation*/
	uint8 rtl_gatewayMac[6];
	uint32 rtl_gatewayIpv4Addr;
	
#ifdef CONFIG_RG_MLD_SNOOPING	
	uint32 rtl_gatewayIpv6Addr[4];
#endif


	/*hash table definition*/
	struct rtl_groupEntry ** rtl_ipv4HashTable;

#ifdef CONFIG_RG_MLD_SNOOPING	
	struct rtl_groupEntry ** rtl_ipv6HashTable;
#endif

#ifdef CONFIG_RECORD_MCAST_FLOW
	struct rtl_mcastFlowEntry **flowHashTable;
#endif
#ifdef CONFIG_PROC_FS
	unsigned int expireEventCnt;
#endif

};

#endif 

