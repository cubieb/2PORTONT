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

#ifndef RTL865X_IGMP_SNOOPING_H
#define RTL865X_IGMP_SNOOPING_H

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

int32 rtl_igmpMldProcess(uint32 moduleIndex, uint8 * macFrame,  uint32 portNum, uint32 *fwdPortMask);

int32 rtl_getMulticastDataFwdPortMask(uint32 moduleIndex, struct rtl_multicastDataInfo *multicastDataInfo, uint32 *fwdPortMask);

int32 rtl_getMulticastDataFwdInfo(uint32 moduleIndex, struct rtl_multicastDataInfo * multicastDataInfo, struct rtl_multicastFwdInfo *multicastFwdInfo);

int32 rtl_maintainMulticastSnoopingTimerList(uint32 currentSystemTime);


#ifdef CONFIG_PROC_FS
int igmp_show(struct seq_file *s, void *v);
#endif

int32 rtl_getGroupInfo(uint32 groupAddr, struct rtl_groupInfo *groupInfo);
int32 rtl_flushAllIgmpRecord(void);
#endif 

