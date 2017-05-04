#include "prmt_queuemnt_3.h"

extern unsigned int getInstNum( char *name, char *objname );
unsigned int getQueueInstNum( char *name );
unsigned int getClassInstNum( char *name );
unsigned int findClassInstNum(void);
int getClassEntryByInstNum(unsigned int instnum, MIB_CE_IP_QOS_T *p, unsigned int *id);
int getQueueStatsEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setQueueStatsEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);
int objQueueStats(char *name, struct CWMP_LEAF *e, int type, void *data);

unsigned int getQueueStatsInstNum( char *name );
struct CWMP_OP tQueueEntityLeafOP = { getQueueEntity, setQueueEntity };
struct CWMP_PRMT tQueueEntityLeafInfo[] =
{
/*(name,		type,		flag,			op)*/
//{"QueueKey",		eCWMP_tUINT,	CWMP_READ,		&tQueueEntityLeafOP},
{"QueueEnable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tQueueEntityLeafOP},
{"QueueStatus",		eCWMP_tSTRING,	CWMP_READ,		&tQueueEntityLeafOP},
{"QueueInterface",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tQueueEntityLeafOP},
{"QueueBufferLength",	eCWMP_tUINT,	CWMP_READ,		&tQueueEntityLeafOP},
{"QueueWeight",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tQueueEntityLeafOP},
{"QueuePrecedence",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tQueueEntityLeafOP},
//{"REDThreshold",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tQueueEntityLeafOP},
//{"REDPercentage",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tQueueEntityLeafOP},
{"DropAlgorithm",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tQueueEntityLeafOP},
{"SchedulerAlgorithm",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tQueueEntityLeafOP},
//{"ShapingRate",		eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tQueueEntityLeafOP},
//{"ShapingBurstSize",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tQueueEntityLeafOP}
};
enum eQueueEntityLeaf
{
//	eQE_QueueKey,
	eQE_QueueEnable,
	eQE_QueueStatus,
	eQE_QueueInterface,
	eQE_QueueBufferLength,
	eQE_QueueWeight,
	eQE_QueuePrecedence,
	eQE_DropAlgorithm,
	eQE_SchedulerAlgorithm,
};
struct CWMP_LEAF tQueueEntityLeaf[] =
{
//{ &tQueueEntityLeafInfo[eQE_QueueKey] },
{ &tQueueEntityLeafInfo[eQE_QueueEnable] },
{ &tQueueEntityLeafInfo[eQE_QueueStatus] },
{ &tQueueEntityLeafInfo[eQE_QueueInterface] },
{ &tQueueEntityLeafInfo[eQE_QueueBufferLength] },
{ &tQueueEntityLeafInfo[eQE_QueueWeight] },
{ &tQueueEntityLeafInfo[eQE_QueuePrecedence] },
{ &tQueueEntityLeafInfo[eQE_DropAlgorithm] },
{ &tQueueEntityLeafInfo[eQE_SchedulerAlgorithm] },
{ NULL }
};

struct CWMP_PRMT tQueueObjectInfo[] =
{
/*(name,			type,		flag,					op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eQueueObject
{
	eQueueObject0
};
struct CWMP_LINKNODE tQueueObject[] =
{
/*info,  				leaf,			next,		sibling,		instnum)*/
{&tQueueObjectInfo[eQueueObject0],	tQueueEntityLeaf,	NULL,		NULL,			0},
};

struct CWMP_OP tQueueStatsEntityLeafOP = { getQueueStatsEntity, setQueueStatsEntity };
struct CWMP_PRMT tQueueStatsEntityLeafInfo[] =
{
/*(name,		type,		flag,			op)*/
{"Enable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tQueueStatsEntityLeafOP},
{"Status",		eCWMP_tSTRING,	CWMP_READ,		&tQueueStatsEntityLeafOP},
{"Queue",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tQueueStatsEntityLeafOP},
{"Interface",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tQueueStatsEntityLeafOP},
{"OutputPackets",	eCWMP_tUINT,	CWMP_READ,		&tQueueStatsEntityLeafOP},
{"OutputBytes",		eCWMP_tUINT,	CWMP_READ,		&tQueueStatsEntityLeafOP},
{"DroppedPackets",	eCWMP_tUINT,	CWMP_READ,		&tQueueStatsEntityLeafOP},
{"DroppedBytes",	eCWMP_tUINT,	CWMP_READ,		&tQueueStatsEntityLeafOP},
{"QueueOccupancyPackets",eCWMP_tUINT,	CWMP_READ,		&tQueueStatsEntityLeafOP},
{"QueueOccupancyPercentage",eCWMP_tUINT,CWMP_READ,		&tQueueStatsEntityLeafOP},
};
enum eQueueStatsEntityLeaf
{
	eQESta_Enable,
	eQESta_Status,
	eQESta_Queue,
	eQESta_Interface,
	eQESta_OutputPackets,
	eQESta_OutputBytes,
	eQESta_DroppedPackets,
	eQESta_DroppedBytes,
	eQESta_QueueOccupancyPackets,
	eQESta_QueueOccupancyPercentage
};
struct CWMP_LEAF tQueueStatsEntityLeaf[] =
{
{ &tQueueStatsEntityLeafInfo[eQESta_Enable] },
{ &tQueueStatsEntityLeafInfo[eQESta_Status] },
{ &tQueueStatsEntityLeafInfo[eQESta_Queue] },
{ &tQueueStatsEntityLeafInfo[eQESta_Interface] },
{ &tQueueStatsEntityLeafInfo[eQESta_OutputPackets] },
{ &tQueueStatsEntityLeafInfo[eQESta_OutputBytes] },
{ &tQueueStatsEntityLeafInfo[eQESta_DroppedPackets] },
{ &tQueueStatsEntityLeafInfo[eQESta_DroppedBytes] },
{ &tQueueStatsEntityLeafInfo[eQESta_QueueOccupancyPackets] },
{ &tQueueStatsEntityLeafInfo[eQESta_QueueOccupancyPercentage] },
{ NULL }
};

struct CWMP_PRMT tQueueStatsObjectInfo[] =
{
/*(name,			type,		flag,					op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eQueueStatsObject
{
	eQueueStatsObject0
};
struct CWMP_LINKNODE tQueueStatsObject[] =
{
/*info,  				leaf,			next,		sibling,		instnum)*/
{&tQueueStatsObjectInfo[eQueueObject0],	tQueueStatsEntityLeaf,	NULL,		NULL,			0},
};


struct CWMP_OP tClassEntityLeafOP = { getClassEntity, setClassEntity };
struct CWMP_PRMT tClassEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"ClassificationKey",		eCWMP_tUINT,	CWMP_READ,		&tClassEntityLeafOP},
{"ClassificationEnable",	eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"ClassificationStatus",	eCWMP_tSTRING,	CWMP_READ,		&tClassEntityLeafOP},
{"ClassificationOrder",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"ClassInterface",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"DestIP",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"DestMask",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
//{"DestIPExclude",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"SourceIP",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"SourceMask",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
//{"SourceIPExclude",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"Protocol",			eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
//{"ProtocolExclude",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"DestPort",			eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"DestPortRangeMax",		eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
//{"DestPortExclude",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"SourcePort",			eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"SourcePortRangeMax",		eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
//{"SourcePortExclude",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"SourceMACAddress",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
/*SourceMACMask*/
//{"SourceMACExclude",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"DestMACAddress",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
/*DestMACMask*/
//{"DestMACExclude",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"Ethertype",				eCWMP_tINT, CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
//{"EthertypeExclude",		eCWMP_tBOOLEAN, CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
/*SSAP, SSAPExclude*/
/*DSAP, DSAPExclude*/
/*LLCControl, LLCControlExclude*/
/*SNAPOUI, SNAPOUIExclude*/
/*SourceVendorClassID, SourceVendorClassIDExclude*/
/*DestVendorClassID, DestVendorClassIDExclude*/
/*SourceClientID, SourceClientIDExclude*/
/*DestClientID, DestClientIDExclude*/
/*SourceUserClassID, SourceUserClassIDExclude*/
/*DestUserClassID, DestUserClassIDExclude*/
/*TCPACK, TCPACKExclude*/
/*IPLengthMin, IPLengthMax, IPLengthExclude*/
#ifdef QOS_DIFFSERV
{"DSCPCheck",			eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
#endif
//{"DSCPExclude",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"DSCPMark",			eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"EthernetPriorityCheck",	eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
//{"EthernetPriorityExclude",	eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"EthernetPriorityMark",	eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
//{"VLANIDCheck",			eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
//{"VLANIDExclude",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
//{"ForwardingPolicy",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
//{"ClassPolicer",		eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"ClassQueue",			eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP}
/*ClassApp*/
};
enum eClassEntityLeaf
{
	eQC_ClassificationKey,
	eQC_ClassificationEnable,
	eQC_ClassificationStatus,
	eQC_ClassificationOrder,
	eQC_ClassInterface,
	eQC_DestIP,
	eQC_DestMask,
//	eQC_DestIPExclude,
	eQC_SourceIP,
	eQC_SourceMask,
//	eQC_SourceIPExclude,
	eQC_Protocol,
//	eQC_ProtocolExclude,
	eQC_DestPort,
	eQC_DestPortRangeMax,
//	eQC_DestPortExclude,
	eQC_SourcePort,
	eQC_SourcePortRangeMax,
//	eQC_SourcePortExclude,
	eQC_SourceMACAddress,
//	eQC_SourceMACExclude,
	eQC_DestMACAddress,
//	eQC_DestMACExclude,
	eQC_Ethertype,
//	eQC_EthertypeExclude,
	eQC_DSCPCheck,
//	eQC_DSCPExclude,
	eQC_DSCPMark,
	eQC_EthernetPriorityCheck,
	eQC_EthernetPriorityMark,
	eQC_ClassQueue
};
struct CWMP_LEAF tClassEntityLeaf[] =
{
{ &tClassEntityLeafInfo[eQC_ClassificationKey] },
{ &tClassEntityLeafInfo[eQC_ClassificationEnable] },
{ &tClassEntityLeafInfo[eQC_ClassificationStatus] },
{ &tClassEntityLeafInfo[eQC_ClassificationOrder] },
{ &tClassEntityLeafInfo[eQC_ClassInterface] },
{ &tClassEntityLeafInfo[eQC_DestIP] },
{ &tClassEntityLeafInfo[eQC_DestMask] },
//{ &tClassEntityLeafInfo[eQC_DestIPExclude] },
{ &tClassEntityLeafInfo[eQC_SourceIP] },
{ &tClassEntityLeafInfo[eQC_SourceMask] },
//{ &tClassEntityLeafInfo[eQC_SourceIPExclude] },
{ &tClassEntityLeafInfo[eQC_Protocol] },
//{ &tClassEntityLeafInfo[eQC_ProtocolExclude] },
{ &tClassEntityLeafInfo[eQC_DestPort] },
{ &tClassEntityLeafInfo[eQC_DestPortRangeMax] },
//{ &tClassEntityLeafInfo[eQC_DestPortExclude] },
{ &tClassEntityLeafInfo[eQC_SourcePort] },
{ &tClassEntityLeafInfo[eQC_SourcePortRangeMax] },
//{ &tClassEntityLeafInfo[eQC_SourcePortExclude] },
{ &tClassEntityLeafInfo[eQC_SourceMACAddress] },
//{ &tClassEntityLeafInfo[eQC_SourceMACExclude] },
{ &tClassEntityLeafInfo[eQC_DestMACAddress] },
//{ &tClassEntityLeafInfo[eQC_DestMACExclude] },
{ &tClassEntityLeafInfo[eQC_Ethertype] },
//{ &tClassEntityLeafInfo[eQC_EthertypeExclude] },
{ &tClassEntityLeafInfo[eQC_DSCPCheck] },
//{ &tClassEntityLeafInfo[eQC_DSCPExclude] },
{ &tClassEntityLeafInfo[eQC_DSCPMark] },
{ &tClassEntityLeafInfo[eQC_EthernetPriorityCheck] },
{ &tClassEntityLeafInfo[eQC_EthernetPriorityMark] },
{ &tClassEntityLeafInfo[eQC_ClassQueue] },
{ NULL }
};

struct CWMP_PRMT tClassObjectInfo[] =
{
/*(name,			type,		flag,					op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eClassObject
{
	eClassObject0
};
struct CWMP_LINKNODE tClassObject[] =
{
/*info,  				leaf,			next,		sibling,		instnum)*/
{&tClassObjectInfo[eClassObject0],	tClassEntityLeaf,	NULL,		NULL,			0},
};



struct CWMP_OP tQueueMntLeafOP = { getQueueMnt,	setQueueMnt };
struct CWMP_PRMT tQueueMntLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tQueueMntLeafOP},
{"MaxQueues",			eCWMP_tUINT,	CWMP_READ,		&tQueueMntLeafOP},
{"MaxClassificationEntries",	eCWMP_tUINT,	CWMP_READ,		&tQueueMntLeafOP},
{"ClassificationNumberOfEntries",eCWMP_tUINT,	CWMP_READ,		&tQueueMntLeafOP},
{"MaxAppEntries",		eCWMP_tUINT,	CWMP_READ,		&tQueueMntLeafOP},
{"AppNumberOfEntries",		eCWMP_tUINT,	CWMP_READ,		&tQueueMntLeafOP},
{"MaxFlowEntries",		eCWMP_tUINT,	CWMP_READ,		&tQueueMntLeafOP},
{"FlowNumberOfEntries",		eCWMP_tUINT,	CWMP_READ,		&tQueueMntLeafOP},
{"MaxPolicerEntries",		eCWMP_tUINT,	CWMP_READ,		&tQueueMntLeafOP},
{"PolicerNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,		&tQueueMntLeafOP},
{"MaxQueueEntries",		eCWMP_tUINT,	CWMP_READ,		&tQueueMntLeafOP},
{"QueueNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,		&tQueueMntLeafOP},
{"QueueStatsNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,		&tQueueMntLeafOP},
//{"DefaultForwardingPolicy",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tQueueMntLeafOP},
//{"DefaultPolicer",		eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tQueueMntLeafOP},
//{"DefaultQueue",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tQueueMntLeafOP},
//{"DefaultDSCPMark",		eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tQueueMntLeafOP},
{"DefaultEthernetPriorityMark",	eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tQueueMntLeafOP},
{"AvailableAppList",		eCWMP_tSTRING,	CWMP_READ,		&tQueueMntLeafOP}
};
enum eQueueMntLeaf
{
	eQ_Enable,
	eQ_MaxQueues,
	eQ_MaxClassificationEntries,
	eQ_ClassificationNumberOfEntries,
	eQ_MaxAppEntries,
	eQ_AppNumberOfEntries,
	eQ_MaxFlowEntries,
	eQ_FlowNumberOfEntries,
	eQ_MaxPolicerEntries,
	eQ_PolicerNumberOfEntries,
	eQ_MaxQueueEntries,
	eQ_QueueNumberOfEntries,
	eQ_QueueStatsNumberOfEntries,
	eQ_DefaultQueue,
	eQ_DefaultEthernetPriorityMark,
	eQ_AvailableAppList
};
struct CWMP_LEAF tQueueMntLeaf[] =
{
{ &tQueueMntLeafInfo[eQ_Enable] },
{ &tQueueMntLeafInfo[eQ_MaxQueues] },
{ &tQueueMntLeafInfo[eQ_MaxClassificationEntries] },
{ &tQueueMntLeafInfo[eQ_ClassificationNumberOfEntries] },
{ &tQueueMntLeafInfo[eQ_MaxAppEntries] },
{ &tQueueMntLeafInfo[eQ_AppNumberOfEntries] },
{ &tQueueMntLeafInfo[eQ_MaxFlowEntries] },
{ &tQueueMntLeafInfo[eQ_FlowNumberOfEntries] },
{ &tQueueMntLeafInfo[eQ_MaxPolicerEntries] },
{ &tQueueMntLeafInfo[eQ_PolicerNumberOfEntries] },
{ &tQueueMntLeafInfo[eQ_MaxQueueEntries] },
{ &tQueueMntLeafInfo[eQ_QueueNumberOfEntries] },
{ &tQueueMntLeafInfo[eQ_QueueStatsNumberOfEntries] },
{ &tQueueMntLeafInfo[eQ_DefaultQueue] },
{ &tQueueMntLeafInfo[eQ_DefaultEthernetPriorityMark] },
{ &tQueueMntLeafInfo[eQ_AvailableAppList] },
{ NULL }
};


struct CWMP_OP tQM_Class_OP = { NULL, objClass };
//struct CWMP_OP tQM_Policer_OP = { NULL, objPolicer };
struct CWMP_OP tQM_Queue_OP = { NULL, objQueue };
struct CWMP_OP tQM_QueueStats_OP = { NULL, objQueueStats };
struct CWMP_PRMT tQueueMntObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Classification",		eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,	&tQM_Class_OP},
//{"Policer",			eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,	&tQM_Policer_OP},
{"Queue",			eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,	&tQM_Queue_OP},
{"QueueStats",		eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,	&tQM_QueueStats_OP}
};
enum tQueueMntObject
{
	eQ_Classification,
	//eQ_Policer,
	eQ_Queue
	, eQ_QueueStats
};
struct CWMP_NODE tQueueMntObject[] =
{
/*info,  					leaf,			node)*/
{&tQueueMntObjectInfo[eQ_Classification],	NULL,			NULL},
{&tQueueMntObjectInfo[eQ_Queue],		NULL,			NULL},
{&tQueueMntObjectInfo[eQ_QueueStats],		NULL,			NULL},
{NULL,						NULL,			NULL}
};

int getQueueEntryByQinst(unsigned int qinst, MIB_CE_IP_QOS_QUEUE_T *p, unsigned int *id)
{
	int ret=-1;
	unsigned int i,num;

	if( (qinst==0) || (p==NULL) || (id==NULL) )
		return ret;

	num = mib_chain_total(MIB_IP_QOS_QUEUE_TBL);
	for(i=0; i<num; i++)
	{
		if( !mib_chain_get( MIB_IP_QOS_QUEUE_TBL, i, (void*)p ) )
			continue;

		if(p->QueueInstNum == qinst)
		{
			*id = i;
			ret = 0;
			break;
		}
	}
	return ret;
}

int getATMVCEntryByIfindex( unsigned int ifindex, MIB_CE_ATM_VC_T *p, unsigned int *id )
{
	int ret=-1;
	unsigned int i,num;
	
	if( (ifindex==0) || (p==NULL) || (id==NULL) )
		return ret;
		
	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)p ) )
			continue;

		if( p->ifIndex == ifindex)
		{
			*id = i;
			ret = 0;
			break;
		}
	}	
	return ret;	
}

unsigned int findMaxQueueInstNum(void)
{
	unsigned int ret=0;
	int i,num;
	MIB_CE_IP_QOS_QUEUE_T queue_entry;

	num = mib_chain_total(MIB_IP_QOS_QUEUE_TBL);
	for(i=0; i<num; i++)
	{
		if( !mib_chain_get( MIB_IP_QOS_QUEUE_TBL, i, (void*)&queue_entry ) )
			continue;

		if(queue_entry.QueueInstNum > ret)
		{
			ret = queue_entry.QueueInstNum;
		}
	}
	return ret;
}

int getQWaninterface( char *name, int instnum )
{
	int total,i;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;
	int qidx=-1,qnum;

	if( name==NULL ) return -1;
	name[0]=0;

	qnum = (instnum-1)/IPQOS_NUM_PRIOQ;

	total = mib_chain_total(MIB_ATM_VC_TBL);
	for( i=0; i<total; i++ )
	{
		pEntry = &vc_entity;
		if( !mib_chain_get(MIB_ATM_VC_TBL, i, (void*)pEntry ) )
			continue;
		if(pEntry->enableIpQos==0)
			continue;
		qidx++;
		if(qidx==qnum)
		{
			char strfmt[]="InternetGatewayDevice.WANDevice.1.WANConnectionDevice.%d.%s.%d"; //wt-121v8-3.33, no trailing dot
			char ipstr[]="WANIPConnection";
			char pppstr[]="WANPPPConnection";
			char *pconn=NULL;
			unsigned int instnum=0;

			if( (pEntry->cmode==CHANNEL_MODE_PPPOE) ||
#ifdef PPPOE_PASSTHROUGH
			    ((pEntry->cmode==CHANNEL_MODE_BRIDGE)&&(pEntry->cmode==BRIDGE_PPPOE)) ||
#endif
			    (pEntry->cmode==CHANNEL_MODE_PPPOA) )
			{
				pconn = pppstr;
				instnum = pEntry->ConPPPInstNum;
			}else{
				pconn = ipstr;
				instnum = pEntry->ConIPInstNum;
			}

			if( pEntry->connDisable==0 )
			{
				sprintf( name, strfmt, pEntry->ConDevInstNum , pconn, instnum );
				break;
			}else
				return -1;

		}
	}
	//name = InternetGatewayDevice.WANDevice.1.WANConnection.2.WANPPPConnection.1.
	return 0;
}

int getQueueEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	int chainid;
	MIB_CE_IP_QOS_QUEUE_T *pEntry, queue_entity;
	char 	buff[256]={0};
	unsigned char vChar=0;
	unsigned int  qinst=0;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	qinst = getQueueInstNum( name );
	if(qinst==0) return ERR_9005;

	pEntry = &queue_entity;
	if(getQueueEntryByQinst(qinst, pEntry,&chainid) < 0)
		return ERR_9002;
	
	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "QueueEnable" )==0 )
	{
		 if(pEntry->enable)
			*data = booldup(1);
		else
			*data = booldup(0);
	}
	else if( strcmp( lastname, "QueueStatus" )==0 )
	{
		 if(pEntry->enable)
		 	*data=strdup( "Enabled" );
		else
		 	*data=strdup( "Disabled" );
	}
	else if( strcmp( lastname, "QueueInterface" )==0 )
	{
		char buf[256];
		int ret=0;
		ret=getQWaninterface(buf,qinst);
		if(ret==0)
			 *data=strdup( buf );
		else
			*data=strdup("");
	}
	else if( strcmp( lastname, "QueueBufferLength" )==0 )
	{
		 *data=uintdup( 8*2048 );
	}
	else if( strcmp( lastname, "QueueWeight" )==0 )
	{
		 *data=uintdup(pEntry->weight);
	}
	else if( strcmp( lastname, "QueuePrecedence" )==0 )
	{
		*data = uintdup( ((qinst-1)%IPQOS_NUM_PRIOQ)+1 );
	}
//	else if( strcmp( lastname, "REDThreshold" )==0 )
//	{
//		 *data=uintdup( 0 );
//	}else if( strcmp( lastname, "REDPercentage" )==0 )
//	{
//		 *data=uintdup( 0 );
//	}
	else if( strcmp( lastname, "DropAlgorithm" )==0 )
	{
		 *data=strdup( "DT" );
	}
	else if( strcmp( lastname, "SchedulerAlgorithm" )==0 )
	{
		unsigned char policy=0;
		mib_get(MIB_QOS_POLICY, &policy);
		if(policy==0)
		 	*data=strdup( "SP" );
		else
			*data=strdup( "WRR" );
	}
//	else if( strcmp( lastname, "ShapingRate" )==0 )
//	{
//		 *data=intdup( -1 );
//	}else if( strcmp( lastname, "ShapingBurstSize" )==0 )
//	{
//		 *data=uintdup( 0 );
//	}
	else{
		return ERR_9005;
	}
	
	return 0;
}


int setQueueEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	int chainid;
	MIB_CE_IP_QOS_QUEUE_T *pEntry, queue_entity;
	unsigned int  qinst=0;
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	qinst = getQueueInstNum( name );
	if(qinst==0) return ERR_9005;

	pEntry = &queue_entity;
	if(getQueueEntryByQinst(qinst, pEntry,&chainid) < 0)
		return ERR_9002;
	
	if( strcmp( lastname, "QueueEnable" )==0 )
	{
		int *i=data;
		if(i==NULL) return ERR_9007;
		pEntry->enable = (*i == 0) ? 0 : 1;

		mib_chain_update(MIB_IP_QOS_QUEUE_TBL, (void *)pEntry, chainid);
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "QueueInterface" )==0 )
	{
		if(buf==NULL) return ERR_9007;
		if( strlen(buf)!=0 ) return ERR_9001;
		return 0;
	}
	else if( strcmp( lastname, "QueueWeight" )==0 )
	{
		unsigned int *i = data;

		if(*i > 255) return ERR_9007;
		pEntry->weight = *i;

		mib_chain_update(MIB_IP_QOS_QUEUE_TBL, (unsigned char*)pEntry, chainid);
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "QueuePrecedence" )==0 )
	{
		unsigned int *i = data;

		if(i==NULL) return ERR_9007;
		if(*i!=((qinst-1)%IPQOS_NUM_PRIOQ+1)) return ERR_9001;
		return 0;
//	}else if( strcmp( lastname, "REDThreshold" )==0 )
//	{
//	}else if( strcmp( lastname, "REDPercentage" )==0 )
//	{
	}else if( strcmp( lastname, "DropAlgorithm" )==0 )
	{
		if(buf==NULL) return ERR_9007;
		if( strcmp( buf, "DT" )!=0 ) return ERR_9001;
		return 0;
	}
	else if( strcmp( lastname, "SchedulerAlgorithm" )==0 )
	{
		unsigned char policy=0;
		if(buf==NULL) return ERR_9007;
		if( strcmp( buf, "SP" )==0 ) policy=0;
		else if( strcmp( buf, "WRR" )==0 ) policy=1;
		else return ERR_9001;
		if(!mib_set(MIB_QOS_POLICY, &policy))
			return ERR_9002;

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
//	else if( strcmp( lastname, "ShapingRate" )==0 )
//	{
//	}else if( strcmp( lastname, "ShapingBurstSize" )==0 )
//	{
//	}
	else{
		return ERR_9005;
	}
	
	return 0;
}


int objQueue(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	MIB_CE_IP_QOS_QUEUE_T *pEntry, queue_entity;
	unsigned int num=0;
	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);

	switch( type )
	{
	case eCWMP_tINITOBJ:
	     {
	     	int num=0,MaxInstNum=0,i;
	     	struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;
	     	
	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		MaxInstNum = findMaxQueueInstNum();
		pEntry = &queue_entity;
	     	num=mib_chain_total(MIB_IP_QOS_QUEUE_TBL);
		for( i=0; i<num;i++ )
		{
			if( !mib_chain_get( MIB_IP_QOS_QUEUE_TBL, i, (void*)pEntry ))
				continue;
			if(pEntry->QueueInstNum==0)
			{
				MaxInstNum++;
				pEntry->QueueInstNum=MaxInstNum;
				mib_chain_update(MIB_IP_QOS_QUEUE_TBL, (unsigned char*)pEntry, i);
			}
			if( create_Object( c, tQueueObject, sizeof(tQueueObject), 1, pEntry->QueueInstNum ) < 0 )
				return -1;
		}
		add_objectNum( name, MaxInstNum );
		return 0;
	     }
	case eCWMP_tADDOBJ:
	     {
		int ret;

		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		if(getQoSQueueNum()>=MAX_QOS_QUEUE_NUM) return ERR_9004;
		
		ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tQueueObject, sizeof(tQueueObject), data );
		if( ret >= 0 )
		{
			MIB_CE_IP_QOS_QUEUE_T entry;
			memset( &entry, 0, sizeof( MIB_CE_IP_QOS_QUEUE_T ) );
			{
				entry.outif = DUMMY_IFINDEX;
				entry.prior = 0;
				entry.enable = 0;
				entry.QueueInstNum = *(int*)data;
			}
			mib_chain_add( MIB_IP_QOS_QUEUE_TBL, (unsigned char*)&entry );
		}
		return ret;
	     }
	case eCWMP_tDELOBJ:
	     {
		int ret, num, i,found=0;
		unsigned int *pUint=data;

		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		num = mib_chain_total( MIB_IP_QOS_QUEUE_TBL );
		for( i=num-1; i>=0;i-- )
		{
			pEntry = &queue_entity;
			if( !mib_chain_get( MIB_IP_QOS_QUEUE_TBL, i, (void*)pEntry ) )
				continue;
			if(pEntry->QueueInstNum==*pUint )
			{
				found =1;
				mib_chain_delete( MIB_IP_QOS_QUEUE_TBL, i );	
			}
		}

		if(found==0) return ERR_9005;
		ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next, *(int*)data );
		if( ret==0 )	ret=1;
		return ret;
	     }
	case eCWMP_tUPDATEOBJ:	
	     {
		int num,i;
		struct CWMP_LINKNODE *old_table;


		num = mib_chain_total( MIB_IP_QOS_QUEUE_TBL );
		old_table = (struct CWMP_LINKNODE *)entity->next;
		entity->next = NULL;
		for( i=0; i<num;i++ )
		{
			struct CWMP_LINKNODE *remove_entity=NULL;

			pEntry = &queue_entity;
			if( !mib_chain_get( MIB_IP_QOS_QUEUE_TBL, i, (void*)pEntry ))
				continue;

			remove_entity = remove_SiblingEntity( &old_table, pEntry->QueueInstNum );
			if( remove_entity!=NULL )
			{
				add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
			}
			else
			{
				if( find_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, pEntry->QueueInstNum )==NULL ) 
				{
					unsigned int MaxInstNum = pEntry->QueueInstNum;					
					add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tQueueObject, sizeof(tQueueObject), &MaxInstNum );
					if(MaxInstNum!=pEntry->QueueInstNum)
					{
						pEntry->QueueInstNum = MaxInstNum;
						mib_chain_update( MIB_IP_QOS_QUEUE_TBL, (unsigned char*)pEntry, i );
					}
				}//else already in next_table
			}	
		}

		if( old_table )
			destroy_ParameterTable( (struct CWMP_NODE *)old_table );	     	

		return 0;
		}
	}
	return -1;
}

QUEUE_STATS *queueStatsTable = NULL;
unsigned int getQueueStatsCount(void)
{
	unsigned int num=0;
	QUEUE_STATS *queue_stats;
	
	for(queue_stats=queueStatsTable;queue_stats;queue_stats=queue_stats->next) {
		num++;
	}
	return num;
}

unsigned int getQueueStatsMaxInstNum(void)
{
	unsigned int maxInstNum=0;
	QUEUE_STATS *queue_stats;
	
	for(queue_stats=queueStatsTable;queue_stats;queue_stats=queue_stats->next) {
		if(queue_stats->instanceNum >= maxInstNum)
			maxInstNum = queue_stats->instanceNum;
	}
	return maxInstNum;
}

unsigned int findUnusedQueueStatsInstNum()
{
	int i=0;
	unsigned int num[MAX_QOS_QUEUE_NUM]={0};
	QUEUE_STATS *queue_stats;

	for(queue_stats=queueStatsTable;queue_stats;queue_stats=queue_stats->next) {
		num[queue_stats->instanceNum] = 1;
	}

	for(i=0; i<MAX_QOS_QUEUE_NUM; i++)	{
		if(num[i]==0)
			return i+1;
	}
	return 0;

}

QUEUE_STATS* getQueueStatsByInstNum(unsigned int InstNum)
{
	QUEUE_STATS *queue_stats;

	for(queue_stats=queueStatsTable;queue_stats;queue_stats=queue_stats->next) {
		if(queue_stats->instanceNum == InstNum) {
			return queue_stats;
		}
	}
	return NULL;
}

unsigned int removeQueueStatsByInstNum(unsigned int instNum)
{
	unsigned int maxInstNum=0;
	QUEUE_STATS *queue_stats,*queue_stats_prev;
	
	for(queue_stats_prev=queue_stats=queueStatsTable;queue_stats;queue_stats_prev=queue_stats,queue_stats=queue_stats->next) {
		if(queue_stats->instanceNum == instNum) {
			if(queue_stats_prev == queue_stats)/*remove the head*/
				queueStatsTable = queue_stats->next;
			else
				queue_stats_prev->next= queue_stats->next;
			free(queue_stats);
			return 1;
		}
	}
	return 0;
}


int getQueueStatsEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	QUEUE_STATS *queue_stats;
	MIB_CE_IP_QOS_QUEUE_T queue_entry;
	int chainid,prior=-1;
	unsigned int  qinst=0, outputPkts=0, outputBytes=0, dropPkts=0, dropBytes=0, occupyPkts=0, occupyPerct=0;
	unsigned char isValidQueue=0;
	char tmp[1024],ifname[6];
	FILE *fp;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	qinst = getQueueStatsInstNum( name );
	if(qinst==0) return ERR_9005;

	if((queue_stats = getQueueStatsByInstNum(qinst)) ==NULL)
		return ERR_9005;
	if(getQueueEntryByQinst(queue_stats->queueInstNum, &queue_entry,&chainid) == 0) {
		isValidQueue = 1;
		ifGetName(queue_entry.outif, ifname, sizeof(ifname));

		sprintf(tmp, "tc -s -d qdisc show dev %s > /tmp/qstats", ifname);
		va_cmd("/bin/sh", 2, 1, "-c", tmp);
		/*/tmp/qstats looks like:
			 qdisc prio 1: bands 4 priomap  2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2
			 Sent 930230 bytes 8304 pkts (dropped 4014, overlimits 0) 
			 
			 qdisc pfifo 2: parent 1:1 limit 10p
			 Sent 0 bytes 0 pkts (dropped 0, overlimits 0) 
			 
			 qdisc pfifo 3: parent 1:2 limit 10p
			 Sent 785428 bytes 8018 pkts (dropped 3988, overlimits 0) 
			 
			 qdisc pfifo 4: parent 1:3 limit 10p
			 Sent 144802 bytes 286 pkts (dropped 26, overlimits 0) 
			 
			 qdisc pfifo 5: parent 1:4 limit 10p
			 Sent 0 bytes 0 pkts (dropped 0, overlimits 0)  */
		if (fp = fopen("/tmp/qstats","r"))
		{	
			while (fgets(tmp, sizeof(tmp), fp))
			{
				if(strstr(tmp,"Sent"))
					prior ++;
				if(prior == queue_entry.prior)
				{
					sscanf(tmp, " Sent %u bytes %u pkts (dropped %u, overlimits", &outputPkts, &outputBytes, &dropPkts);
					break;
				}
			}
			fclose(fp);
		}	
	}
	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		 if(queue_stats->enable)
			*data = booldup(1);
		else
			*data = booldup(0);
	}else if( strcmp( lastname, "Status" )==0 )
	{
		 if(queue_stats->enable) {
		 	if(isValidQueue)
				*data=strdup("Enabled");
			else
				*data=strdup("Error");
		 }
		else
		 	*data=strdup( "Disabled" );
	}else if( strcmp( lastname, "Queue" )==0 )
	{
		if(isValidQueue)
			*data=uintdup(queue_entry.QueueInstNum);
		else
			*data=uintdup(0);
	}else if( strcmp( lastname, "Interface" )==0 )
	{
		char pathName[128];
		 if(isValidQueue) {
		 	transfer2PathName(queue_entry.outif, pathName);
			*data=strdup(pathName);

		 }
		else
			*data=strdup("");
	}else if( strcmp( lastname, "OutputPackets" )==0 )
	{
		 if(isValidQueue)
			*data=uintdup(outputPkts);
		else
			*data=uintdup(0);
	}else if( strcmp( lastname, "OutputBytes" )==0 )
	{
		 if(isValidQueue)
			*data=uintdup(outputBytes);
		else
			*data=uintdup(0);
	}else if( strcmp( lastname, "DroppedPackets" )==0 )
	{
		 if(isValidQueue)
			*data=uintdup(dropPkts);
		else
			*data=uintdup(0);
	}else if( strcmp( lastname, "DroppedBytes" )==0 )
	{
		 if(isValidQueue)
			*data=uintdup(dropBytes);
		else
			*data=uintdup(0);
	}else if( strcmp( lastname, "QueueOccupancyPackets" )==0 )
	{
		 if(isValidQueue)
			*data=uintdup(occupyPkts);
		else
			*data=uintdup(0);
	}else if( strcmp( lastname, "QueueOccupancyPercentage" )==0 )
	{
		 if(isValidQueue)
			*data=uintdup(occupyPerct);
		else
			*data=uintdup(0);
	}else{
		return ERR_9005;
	}
	
	return 0;
}


int setQueueStatsEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned int *pUint=data;
	QUEUE_STATS *queue_stats;
	int chainid;
	MIB_CE_IP_QOS_QUEUE_T *pEntry, queue_entity;
	unsigned int  qinst=0;
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	qinst = getQueueStatsInstNum( name );
	if(qinst==0) return ERR_9005;

	if((queue_stats = getQueueStatsByInstNum(qinst)) ==NULL)
		return ERR_9005;
	
	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i=data;
		if(i==NULL) return ERR_9007;
		queue_stats->enable = (*i==0) ? 0 : 1;		
		return 0;
	}else if( strcmp( lastname, "Queue" )==0 )
	{
		MIB_CE_IP_QOS_QUEUE_T  queue_entry;
		int chainid;
		if(getQueueEntryByQinst(*pUint, &queue_entry, &chainid))
			return ERR_9007;
		queue_stats->queueInstNum = *pUint;
		return 0;
	}else if( strcmp( lastname, "Interface" )==0 )
	{
		MIB_CE_IP_QOS_QUEUE_T  queue_entry;
		int chainid;
		if(getQueueEntryByQinst(queue_stats->queueInstNum, &queue_entry, &chainid))
			return ERR_9007;
		if(transfer2IfIndex(buf)!=queue_entry.outif)
			return ERR_9007;
		return 0;
	}else{
		return ERR_9005;
	}
	
	return 0;
}


int objQueueStats(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	QUEUE_STATS *queue_stats;
	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);

	switch( type )
	{
	case eCWMP_tINITOBJ:
	     {
	     	int num=0,MaxInstNum=0,i;
	     	struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;
	     	
	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		MaxInstNum = getQueueStatsMaxInstNum();
		for(queue_stats=queueStatsTable;queue_stats;queue_stats=queue_stats->next) {
			if( create_Object( c, tQueueStatsObject, sizeof(tQueueStatsObject), 1, queue_stats->instanceNum) < 0 )
				return -1;
		}
		add_objectNum( name, MaxInstNum );
		return 0;
	     }
	case eCWMP_tADDOBJ:
	     {
		int ret;

		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		if(getQueueStatsCount()>=getQoSQueueNum()) return ERR_9004;
		
		ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tQueueStatsObject, sizeof(tQueueStatsObject), data );
		if( ret >= 0 )
		{
			queue_stats = malloc(sizeof(QUEUE_STATS));
			memset( queue_stats, 0, sizeof( QUEUE_STATS ) );
			{
				queue_stats->enable = 0;
				queue_stats->queueInstNum= 0;
				queue_stats->instanceNum = *(unsigned int*)data;//findUnusedQueueStatsInstNum();
			}
			queue_stats->next = queueStatsTable;
			queueStatsTable = queue_stats;
		}
		return ret;
	     }
	case eCWMP_tDELOBJ:
	     {
		int ret;
		unsigned int *pUint=data;

		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		if((queue_stats = getQueueStatsByInstNum(*pUint))==NULL)
			return ERR_9005;

		removeQueueStatsByInstNum(queue_stats->instanceNum);

		ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next, *(int*)data );
		//if( ret==0 )	ret=1;
		return ret;
	     }
	case eCWMP_tUPDATEOBJ:	
	     {
		struct CWMP_LINKNODE *old_table;

		old_table = (struct CWMP_LINKNODE *)entity->next;
		entity->next = NULL;
		for(queue_stats=queueStatsTable;queue_stats;queue_stats=queue_stats->next) {
			struct CWMP_LINKNODE *remove_entity=NULL;

			remove_entity = remove_SiblingEntity( &old_table, queue_stats->instanceNum);
			if( remove_entity!=NULL )
			{
				add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
			}
			else
			{
				if( find_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, queue_stats->instanceNum )==NULL ) 
				{
					unsigned int MaxInstNum = queue_stats->instanceNum;					
					add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tQueueStatsObject, sizeof(tQueueStatsObject), &MaxInstNum );
				}//else already in next_table
			}	
		}

		if( old_table )
			destroy_ParameterTable( (struct CWMP_NODE *)old_table );	     	

		return 0;
		}
	}
	return -1;
}


int getClassEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar=0;
	unsigned int  instnum=0,chainid=0;
	char buf[256]={0};
	MIB_CE_IP_QOS_T *p, qos_entity;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	instnum=getClassInstNum(name);
	if(instnum==0) return ERR_9005;
	p = &qos_entity;
	if(getClassEntryByInstNum( instnum, p, &chainid )<0) return ERR_9002;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "ClassificationKey" )==0 )
	{
		*data = uintdup( p->InstanceNum );
	}
	else if( strcmp( lastname, "ClassificationEnable" )==0 )
	{
		 *data=booldup( p->enable );
	}
	else if( strcmp( lastname, "ClassificationStatus" )==0 )
	{
		if( p->enable==0 )
		 	*data=strdup( "Disabled" );
		else
			*data=strdup( "Enabled" );
	}
	else if( strcmp( lastname, "ClassificationOrder" )==0 )
	{
		 *data=uintdup( chainid+1 );
	}
	else if( strcmp( lastname, "ClassInterface" )==0 )
	{
		switch(p->phyPort)
		{
		case 0:
			*data=strdup( "" );
			break;
		case 1:
			*data=strdup("InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.1");
			break;
#if (defined(CONFIG_RTL_MULTI_LAN_DEV)  && defined(IP_QOS_VPORT))
		case 2:
			*data=strdup("InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.2");
			break;
		case 3:
			*data=strdup("InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.3");
			break;
		case 4:
			*data=strdup("InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.4");
			break;
#endif
#ifdef WLAN_SUPPORT
		case 5:
			*data=strdup("InternetGatewayDevice.LANDevice.1.WLANConfiguration.1");
			break;
#if 0 //def WLAN_MBSSID
		case 6:
			*data=strdup("InternetGatewayDevice.LANDevice.1.WLANConfiguration.2");
			break;
		case 7:
			*data=strdup("InternetGatewayDevice.LANDevice.1.WLANConfiguration.3");
			break;
		case 8:
			*data=strdup("InternetGatewayDevice.LANDevice.1.WLANConfiguration.4");
			break;
		case 9:
			*data=strdup("InternetGatewayDevice.LANDevice.1.WLANConfiguration.5");
			break;
#endif //WLAN_MBSSID
#endif //WLAN_SUPPORT
		default:
			return ERR_9002;
		}
	}
	else if( strcmp( lastname, "DestIP" )==0 )
	{
		sprintf(buf, "%s", inet_ntoa(*((struct in_addr *)&p->dip)));
		*data = strdup( buf );
	}
	else if( strcmp( lastname, "DestMask" )==0 )
	{
		unsigned long mask=0;
		int i,mbit;
		mbit = p->dmaskbit;
		for(i=0;i<32;i++)
		{
			mask = mask << 1;
			if(mbit)
			{ 
				mask = mask | 0x1;
				mbit--;
			}
		}
		mask = ntohl(mask);
		sprintf(buf, "%s", inet_ntoa(*((struct in_addr *)&mask)));
		*data = strdup( buf );
	}
#if 0
	else if( strcmp( lastname, "DestIPExclude" )==0 )
	{
		if(p->flags & EXC_DESTIP)
		 	*data = booldup( 1 );
		else
			*data = booldup( 0 );
	}
#endif
	else if( strcmp( lastname, "SourceIP" )==0 )
	{
		sprintf(buf, "%s", inet_ntoa(*((struct in_addr *)&p->sip)));
		*data = strdup( buf );
	}
	else if( strcmp( lastname, "SourceMask" )==0 )
	{
		unsigned long mask=0;
		int i,mbit;
		mbit = p->smaskbit;
		for(i=0;i<32;i++)
		{
			mask = mask << 1;
			if(mbit)
			{ 
				mask = mask | 0x1;
				mbit--;
			}
		}
		mask = ntohl(mask);
		sprintf(buf, "%s", inet_ntoa(*((struct in_addr *)&mask)));
		*data = strdup( buf );
	}
#if 0
	else if( strcmp( lastname, "SourceIPExclude" )==0 )
	{
		 if(p->flags & EXC_SOURCEIP)
		 	*data = booldup( 1 );
		else
			*data = booldup( 0 );
	}
#endif
	else if( strcmp( lastname, "Protocol" )==0 )
	{
		if( p->protoType==PROTO_NONE )
			*data=intdup( -1 );
		else if( p->protoType==PROTO_TCP )
			*data=intdup( 6 );
		else if( p->protoType==PROTO_UDP )
			*data=intdup( 17 );
		else if( p->protoType==PROTO_ICMP )
			*data=intdup( 1 );
		else
			return ERR_9002;	
	}
#if 0
	else if( strcmp( lastname, "ProtocolExclude" )==0 )
	{
		 if(p->flags & EXC_PROTOCOL)
		 	*data = booldup( 1 );
		else
			*data = booldup( 0 );
	}
#endif
	else if( strcmp( lastname, "DestPort" )==0 )
	{
		if( p->dPort==0 )
			*data=intdup( -1 );
		else
			*data=intdup( p->dPort );
	}
	else if( strcmp( lastname, "DestPortRangeMax" )==0 )
	{
		if(p->dPortRangeMax == 0)
			*data=intdup( -1 );
		else
			*data=intdup( p->dPortRangeMax);
	}
#if 0
	else if( strcmp( lastname, "DestPortExclude" )==0 )
	{
		 if(p->flags & EXC_DESTPORT)
		 	*data = booldup( 1 );
		else
			*data = booldup( 0 );
	}
#endif
	else if( strcmp( lastname, "SourcePort" )==0 )
	{
		if( p->sPort==0 )
			*data=intdup( -1 );
		else
			*data=intdup( p->sPort );
	}
	else if( strcmp( lastname, "SourcePortRangeMax" )==0 )
	{
		if( p->sPortRangeMax == 0 )
			*data=intdup( -1 );
		else
			*data=intdup(p->sPortRangeMax);
	}
#if 0
	else if( strcmp( lastname, "SourcePortExclude" )==0 )
	{
		if(p->flags & EXC_SOURCEPORT)
		 	*data = booldup( 1 );
		else
			*data = booldup( 0 );
	}
#endif
	else if( strcmp( lastname, "SourceMACAddress" )==0 )
	{
		sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x",
			p->smac[0], p->smac[1], p->smac[2], p->smac[3], p->smac[4], p->smac[5]);

		*data = strdup(buf);
	}
#if 0
	else if( strcmp( lastname, "SourceMACExclude" )==0 )
	{
		if(p->flags & EXC_SOURCEMAC)
		 	*data = booldup( 1 );
		else
			*data = booldup( 0 );
	}
#endif
	else if( strcmp( lastname, "DestMACAddress" )==0 )
	{
		sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x",
			p->dmac[0], p->dmac[1], p->dmac[2], p->dmac[3], p->dmac[4], p->dmac[5]);

		*data = strdup(buf);
	}
#if 0
	else if( strcmp( lastname, "DestMACExclude" )==0 )
	{
		if(p->flags & EXC_DESTMAC)
		 	*data = booldup( 1 );
		else
			*data = booldup( 0 );
	}
#endif
#ifndef CONFIG_E8B
	else if( strcmp( lastname, "Ethertype" )==0 )
	{
		if(p->ethType == 0)
			*data=intdup(-1);
		else
			*data=intdup((int)p->ethType);
	}
#endif
#if 0
	else if( strcmp( lastname, "EthertypeExclude" )==0 )
	{
		if(p->flags & EXC_ETHTYPE)
		 	*data = booldup( 1 );
		else
			*data = booldup( 0 );
	}
#endif
#ifdef QOS_DIFFSERV
	else if( strcmp( lastname, "DSCPCheck" )==0 )
	{
		 *data=intdup(p->qosDscp >> 2);
	}
#endif
#if 0
	else if( strcmp( lastname, "DSCPExclude" )==0 )
	{
		if(p->flags & EXC_DSCP)
		 	*data = booldup( 1 );
		else
			*data = booldup( 0 );
	}
#endif
	else if( strcmp( lastname, "DSCPMark" )==0 )
	{
		 *data=intdup(p->m_dscp >> 2);
	}
	else if( strcmp( lastname, "EthernetPriorityCheck" )==0 )
	{
		*data=intdup( (int)p->vlan1p - 1);
	}
#if 0
	else if( strcmp( lastname, "EthernetPriorityExclude" )==0 )
	{
		if(p->flags & EXC_ETHERNETPRIORITY)
		 	*data = booldup( 1 );
		else
			*data = booldup( 0 );
	}
#endif
	else if( strcmp( lastname, "EthernetPriorityMark" )==0 )
	{
		*data=intdup( (int)p->m_1p-1 );
//	}else if( strcmp( lastname, "VLANIDCheck" )==0 )
//	{
//		 *data=intdup( -1 );
//	}else if( strcmp( lastname, "VLANIDExclude" )==0 )
//	{
//		 *data=booldup( 0 );
//	}else if( strcmp( lastname, "ForwardingPolicy" )==0 )
//	{
//		 *data=uintdup( 0 );
//	}else if( strcmp( lastname, "ClassPolicer" )==0 )
//	{
//		 *data=intdup( -1 );
	}else if( strcmp( lastname, "ClassQueue" )==0 )
	{
		*data=intdup(p->prior);
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setClassEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned char vChar=0;
	unsigned int  instnum=0,chainid=-1;
	MIB_CE_IP_QOS_T *p, qos_entity;
	struct in_addr in;
	char	*pzeroip="0.0.0.0";
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	instnum=getClassInstNum(name);
	if(instnum==0) return ERR_9005;
	p = &qos_entity;
	if(getClassEntryByInstNum( instnum, p, &chainid )<0) return ERR_9002;

	if( strcmp( lastname, "ClassificationEnable" )==0 )
	{
		int *i = data;

		if(i==NULL) return ERR_9007;
		p->enable = (*i==0) ? 0:1;
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}else if( strcmp( lastname, "ClassificationOrder" )==0 )
	{
		unsigned int *i = data;

		//cannot change order
		if(i==NULL) return ERR_9007;
		if( *i==0 ) return ERR_9007;
		if( *i!=(chainid+1) ) return ERR_9001;
		return 0;
	}else if( strcmp( lastname, "ClassInterface" )==0 )
	{
		if( buf==NULL ) return ERR_9007;

		if(strlen(buf) == 0)
		{
			p->phyPort = 0;
		}
		else if( strcmp( buf, "LAN" )==0 )
			p->phyPort=0;
#if (defined(CONFIG_RTL_MULTI_LAN_DEV)  && defined(IP_QOS_VPORT))
		else if( strcmp( buf, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.1" )==0 )
			p->phyPort=1;
		else if( strcmp( buf, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.2" )==0 )
			p->phyPort=2;
		else if( strcmp( buf, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.3" )==0 )
			p->phyPort=3;
		else if( strcmp( buf, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.4" )==0 )
			p->phyPort=4;
#else
		else if( strcmp( buf, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.1" )==0 )
			p->phyPort=1;
#endif
#ifdef WLAN_SUPPORT
		else if( strcmp( buf, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.1" )==0 )
			p->phyPort=5;
#if 0 //def WLAN_MBSSID
		else if( strcmp( buf, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.2" )==0 )
			p->phyPort=6;
		else if( strcmp( buf, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.3" )==0 )
			p->phyPort=7;
		else if( strcmp( buf, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.4" )==0 )
			p->phyPort=8;
		else if( strcmp( buf, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.5" )==0 )
			p->phyPort=9;
#endif //WLAN_MBSSID
#endif //WLAN_SUPPORT
		else
			return ERR_9007;

		p->ipqos_rule_type = 0;
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}else if( strcmp( lastname, "DestIP" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) buf=pzeroip;
		if( inet_aton( buf, &in )==0 ) //the ip address is error.
			return ERR_9007;
		memcpy( p->dip, &in, sizeof(struct in_addr) );
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL, 0);
		p->ipqos_rule_type = 2;
		return 0;
	}else if( strcmp( lastname, "DestMask" )==0 )
	{
		unsigned long mask;
		int intVal, i, mbit;
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) buf=pzeroip;
		if( inet_aton( buf, (struct in_addr *)&mask )==0 ) //the ip address is error.
			return ERR_9007;
		mask = htonl(mask);
		mbit=0; intVal=0;
		for (i=0; i<32; i++)
		{
			if (mask&0x80000000)
			{
				if (intVal) return ERR_9007;
				mbit++;
			}
			else
				intVal=1;
			mask <<= 1;
		}
		p->dmaskbit = mbit;
		p->ipqos_rule_type = 2;
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}
#if 0
	else if( strcmp( lastname, "DestIPExclude" )==0 ) {
		int *i = data;
		if(i==NULL) return ERR_9007;
		if(*i==0)
			p->flags &= ~EXC_DESTIP;
		else
			p->flags |= EXC_DESTIP;
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}
#endif
	else if( strcmp( lastname, "SourceIP" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) buf=pzeroip;
		if( inet_aton( buf, &in )==0 ) //the ip address is error.
			return ERR_9007;
		memcpy( p->sip, &in, sizeof(struct in_addr) );
		p->ipqos_rule_type = 2;
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}
	else if( strcmp( lastname, "SourceMask" )==0 )
	{
		unsigned long mask;
		int intVal, i, mbit;
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) buf=pzeroip;
		if( inet_aton( buf, (struct in_addr *)&mask )==0 ) //the ip address is error.
			return ERR_9007;
		mask = htonl(mask);
		mbit=0; intVal=0;
		for (i=0; i<32; i++)
		{
			if (mask&0x80000000)
			{
				if (intVal) return ERR_9007;
				mbit++;
			}
			else
				intVal=1;
			mask <<= 1;
		}
		p->smaskbit = mbit;
		p->ipqos_rule_type = 2;
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}
#if 0
	else if( strcmp( lastname, "SourceIPExclude" )==0 ) {
		int *i = data;
		if(i==NULL) return ERR_9007;
		if(*i==0)
			p->flags &= ~EXC_SOURCEIP;
		else
			p->flags |= EXC_SOURCEIP;
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}
#endif
	else if( strcmp( lastname, "Protocol" )==0 )
	{

		int *pro = data;

		if(pro==NULL) return ERR_9007;
		switch(*pro)
		{
		case -1:
			p->protoType=PROTO_NONE;
			break;
		case 1:
			p->protoType=PROTO_ICMP;
			break;
		case 6:
			p->protoType=PROTO_TCP;
			break;
		case 17:
			p->protoType=PROTO_UDP;
			break;
		default:
			return ERR_9001;
		}
		p->ipqos_rule_type = 2;
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}
#if 0
	else if( strcmp( lastname, "ProtocolExclude" )==0 ) 
	{
		int *i = data;
		if(i==NULL) return ERR_9007;
		if(*i==0)
			p->flags &= ~EXC_PROTOCOL;
		else
			p->flags |= EXC_PROTOCOL;
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}
#endif
	else if( strcmp( lastname, "DestPort" )==0 )
	{
		int *dport = data;

		if( dport==NULL ) return ERR_9007;
		if( *dport==-1 )
			p->dPort = 0;
		else if( (*dport>=1) && (*dport<=65535) )
			p->dPort = *dport;
		else
			return ERR_9007;
		p->ipqos_rule_type = 2;
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}
	else if( strcmp( lastname, "DestPortRangeMax" )==0 )
	{
		int *dportmax = data;

		if(dportmax==NULL) return ERR_9007;
		if(*dportmax > 65535) return ERR_9007;
		if(*dportmax < -2 || *dportmax == 0) return ERR_9007;

		if( *dportmax == -1 )
			p->dPortRangeMax = 0;
		else if( p->dPort > *dportmax)
			return ERR_9007;
		else
			p->dPortRangeMax = *dportmax;

		p->ipqos_rule_type = 2;
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}
#if 0
	else if( strcmp( lastname, "DestPortExclude" )==0 )
	{
		int *i = data;
		if(i==NULL) return ERR_9007;
		if(*i==0)
			p->flags &= ~EXC_DESTPORT;
		else
			p->flags |= EXC_DESTPORT;
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}
#endif
	else if( strcmp( lastname, "SourcePort" )==0 )
	{
		int *sport = data;

		if( sport==NULL ) return ERR_9007;
		if( *sport==-1 )
			p->sPort = 0;
		else if( (*sport>=1) && (*sport<=65535) )
			p->sPort = *sport;
		else
			return ERR_9007;
		p->ipqos_rule_type = 2;
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}
	else if( strcmp( lastname, "SourcePortRangeMax" )==0 )
	{
		int *sportmax = data;

		if(sportmax==NULL) return ERR_9007;
		if(*sportmax > 65535) return ERR_9007;
		if(*sportmax < -2 || *sportmax == 0) return ERR_9007;

		if( *sportmax==-1 )
			p->sPortRangeMax = 0;
		else if( p->sPort > *sportmax)
			return ERR_9007;
		else
			p->sPortRangeMax = *sportmax;

		p->ipqos_rule_type = 2;
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}
#if 0
	else if( strcmp( lastname, "SourcePortExclude" )==0 )
	{
		int *i = data;
		if(i==NULL) return ERR_9007;
		if(*i==0)
			p->flags &= ~EXC_SOURCEPORT;
		else
			p->flags |= EXC_SOURCEPORT;
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QOS_T) );
		return 0;
	}
#endif
	else if( strcmp( lastname, "SourceMACAddress" )==0 )
	{
		char *buf =data;
		int ret;

		if(strlen(buf) == 0)
			bzero(p->smac, 6);
		else
		{
			ret = sscanf(buf, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
				&p->smac[0], &p->smac[1], &p->smac[2], &p->smac[3], &p->smac[4], &p->smac[5]);

			if(ret != 6)
				return ERR_9007;
		}

		p->ipqos_rule_type = 3;
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}
#if 0
	else if( strcmp( lastname, "SourceMACExclude" )==0 )
	{
		int *i = data;
		if(i==NULL) return ERR_9007;
		if(*i==0)
			p->flags &= ~EXC_SOURCEMAC;
		else
			p->flags |= EXC_SOURCEMAC;
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QOS_T) );
		return 0;
	}
#endif
	else if( strcmp( lastname, "DestMACAddress" )==0 )
	{
		char *buf =data;
		int ret;

		if(strlen(buf) == 0)
			bzero(p->dmac, 6);
		else
		{
			ret = sscanf(buf, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
				&p->dmac[0], &p->dmac[1], &p->dmac[2], &p->dmac[3], &p->dmac[4], &p->dmac[5]);

			if(ret != 6)
				return ERR_9007;
		}

		p->ipqos_rule_type = 3;
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}
#if 0
	else if( strcmp( lastname, "DestMACExclude" )==0 )
	{
		int *i = data;
		if(i==NULL) return ERR_9007;
		if(*i==0)
			p->flags &= ~EXC_DESTMAC;
		else
			p->flags |= EXC_DESTMAC;
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QOS_T) );
		return 0;
	}
#endif
#ifndef CONFIG_E8B
	else if( strcmp( lastname, "Ethertype" )==0 )
	{
		int *i = data;

		if(*i == -1)
			p->ethType = 0;
		else if(*i >= 0)
			p->ethType = *i;
		else
			return ERR_9007;

		p->ipqos_rule_type = 1;
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}
#endif
#ifdef QOS_DIFFSERV
	else if( strcmp( lastname, "DSCPCheck" )==0 )
	{
		int *i = data;

		if(*i < -2 || *i > 63) return ERR_9007;
		if(*i == -2) return ERR_9001;	//We don't support.

		if(*i == -1)
			p->qosDscp = 0;
		else
			p->qosDscp = (*i << 2) + 1;
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}
#endif
#if 0
	else if( strcmp( lastname, "DSCPExclude" )==0 )
	{
		int *i = data;
		if(i==NULL) return ERR_9007;
		if(*i==0)
			p->flags &= ~EXC_DSCP;
		else
			p->flags |= EXC_DSCP;

		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}
#endif
	else if( strcmp( lastname, "DSCPMark" )==0 )
	{
		int *i = data;

		if(*i < -2 || *i > 63) return ERR_9007;
		if(*i == -2) return ERR_9001;	//We don't support.

		if(*i == -1)
			p->m_dscp = 0;
		else
			p->m_dscp = (*i << 2) + 1;
		
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}
	else if( strcmp( lastname, "EthernetPriorityCheck" )==0 )
	{
		int *i = data;

		if(i==NULL) return ERR_9007;
		if( *i<-1 || *i>7 ) return ERR_9007;
		p->vlan1p = (unsigned char)(*i+1);
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}
#if 0
	else if( strcmp( lastname, "EthernetPriorityExclude" )==0 )
	{
		int *i = data;
		if(i==NULL) return ERR_9007;
		if(*i==0)
			p->flags &= ~EXC_ETHTYPE;
		else
			p->flags |= EXC_ETHTYPE;
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QOS_T) );
		return 0;
	}
#endif
	else if( strcmp( lastname, "EthernetPriorityMark" )==0 )
	{
		int *i = data;

		if(i==NULL) return ERR_9007;
		if( *i<-1 || *i>7 ) return ERR_9007;
		p->m_1p = (unsigned char)(*i+1);
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL, 0);
		return 0;
//	}else if( strcmp( lastname, "VLANIDCheck" )==0 )
//	{
//	}else if( strcmp( lastname, "VLANIDExclude" )==0 )
//	{
//	}else if( strcmp( lastname, "ForwardingPolicy" )==0 )
//	{
//	}else if( strcmp( lastname, "ClassPolicer" )==0 )
//	{
	}else if( strcmp( lastname, "ClassQueue" )==0 )
	{
		int *i = data;

		int qchainid = -1;

		if(i==NULL) return ERR_9007;
		if( *i<1 || *i>findMaxQueueInstNum()) return ERR_9007;

		p->prior = *i;

		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL, 0);
		return 0;
	}else{
		return ERR_9005;
	}
	
	return 0;
}


int objClass(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);

	switch( type )
	{
	case eCWMP_tINITOBJ:
	     {
		unsigned int num=0,MaxInstNum=0,i;
		struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;
		MIB_CE_IP_QOS_T *p, qos_entity;

		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		MaxInstNum = findClassInstNum();
		num = mib_chain_total( MIB_IP_QOS_TBL );
		for( i=0; i<num;i++ )
		{
			p = &qos_entity;
			if( !mib_chain_get( MIB_IP_QOS_TBL, i, (void*)p ) )
				continue;
			
			if( p->InstanceNum==0 ) //maybe createn by web or cli
			{
				MaxInstNum++;
				p->InstanceNum = MaxInstNum;
				mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, i );
			}
			if( create_Object( c, tClassObject, sizeof(tClassObject), 1, p->InstanceNum ) < 0 )
				return -1;
			//c = & (*c)->sibling;
		}
		add_objectNum( name, MaxInstNum );
		return 0;	     		     	
	     }
	case eCWMP_tADDOBJ:
	     {
	     	int ret;
	     	unsigned int num=0;
	     	
	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;
		num = mib_chain_total( MIB_IP_QOS_TBL );
		if(num>=MAX_QOS_RULE) return ERR_9004;
		
		ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tClassObject, sizeof(tClassObject), data );
		if( ret >= 0 )
		{
			MIB_CE_IP_QOS_T qos_entry;
			memset( &qos_entry, 0, sizeof( MIB_CE_IP_QOS_T ) );
			qos_entry.InstanceNum = *(unsigned int*)data;
			qos_entry.prior=4;
			qos_entry.m_iptos=0xff;
			snprintf(qos_entry.RuleName, sizeof(qos_entry.RuleName), "rule_%d", qos_entry.InstanceNum);
			mib_chain_add( MIB_IP_QOS_TBL, (unsigned char*)&qos_entry );
		}
		return ret;
	     }
	case eCWMP_tDELOBJ:
	     {
		unsigned int i,num;
		int ret=0;
		MIB_CE_IP_QOS_T *p,qos_entity;
	     	
	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;
	
		num = mib_chain_total( MIB_IP_QOS_TBL );
		for( i=0; i<num;i++ )
		{
			p = &qos_entity;
			if( !mib_chain_get( MIB_IP_QOS_TBL, i, (void*)p ))
				continue;
			if( p->InstanceNum == *(unsigned int*)data )
				break;
		}	     	
		if(i==num) return ERR_9005;
		
		//why don't use CWMP_RESTART?
		//for IPQoS, use the chain index to count the mark number, get_classification_mark()
		//after deleting one rule, the mark numbers will change
		//hence, stop all rules=>delete one rule=>start the rest rules;
		apply_IPQoSRule( CWMP_STOP, -1, NULL );

     	mib_chain_delete( MIB_IP_QOS_TBL, i );
		ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next, *(int*)data );

		apply_IPQoSRule( CWMP_START, -1, NULL );
		return ret;
	     }
	case eCWMP_tUPDATEOBJ:	
	     {
	     	int num=0,i;
	     	struct CWMP_LINKNODE *old_table;
	     	
	     	num = mib_chain_total( MIB_IP_QOS_TBL );
	     	old_table = (struct CWMP_LINKNODE *)entity->next;
	     	entity->next = NULL;
	     	for( i=0; i<num;i++ )
	     	{
	     		struct CWMP_LINKNODE *remove_entity=NULL;
			MIB_CE_IP_QOS_T *p,qos_entity;

			p = &qos_entity;
			if( !mib_chain_get( MIB_IP_QOS_TBL, i, (void*)p ) )
				continue;
			
			remove_entity = remove_SiblingEntity( &old_table, p->InstanceNum );
			if( remove_entity!=NULL )
			{
				add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
			}else{ 
				unsigned int MaxInstNum=p->InstanceNum;
					
				add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tClassObject, sizeof(tClassObject), &MaxInstNum );
				if(MaxInstNum!=p->InstanceNum)
				{
					p->InstanceNum = MaxInstNum;
					mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, i );
				}
			}	
	     	}
	     	
	     	if( old_table )
	     		destroy_ParameterTable( (struct CWMP_NODE *)old_table );	     	

	     	return 0;
	     }
	}
	return -1;
}




int getQueueMnt(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	char 	buff[256]={0};
	unsigned char vChar=0;
	unsigned int num=0;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		mib_get(MIB_QOS_ENABLE_QOS, (void *)&vChar);
		*data = booldup( vChar != 0 );
	}
	else if( strcmp( lastname, "MaxQueues" )==0 )
	{
		 *data=uintdup(MAX_QOS_QUEUE_NUM);
	}
	else if( strcmp( lastname, "MaxClassificationEntries" )==0 )
	{
		*data=uintdup( MAX_QOS_RULE );
	}
	else if( strcmp( lastname, "ClassificationNumberOfEntries" )==0 )
	{
		num = mib_chain_total(MIB_IP_QOS_TBL);
		*data=uintdup( num );
	}
	else if( strcmp( lastname, "MaxAppEntries" )==0 )
	{
		 *data=uintdup( 0 );
	}
	else if( strcmp( lastname, "AppNumberOfEntries" )==0 )
	{
		 *data=uintdup( 0 );
	}
	else if( strcmp( lastname, "MaxFlowEntries" )==0 )
	{
		 *data=uintdup( 0 );
	}
	else if( strcmp( lastname, "FlowNumberOfEntries" )==0 )
	{
		 *data=uintdup( 0 );
	}
	else if( strcmp( lastname, "MaxPolicerEntries" )==0 )
	{
		 *data=uintdup( 0 );
	}
	else if( strcmp( lastname, "PolicerNumberOfEntries" )==0 )
	{
		 *data=uintdup( 0 );
	}
	else if( strcmp( lastname, "MaxQueueEntries" )==0 )
	{
		 *data=uintdup(MAX_QOS_QUEUE_NUM);
	}
	else if( strcmp( lastname, "QueueNumberOfEntries" )==0 )
	{
		 *data=uintdup(getQoSQueueNum());
	}
	else if( strcmp( lastname, "QueueStatsNumberOfEntries" )==0 )
	{
		 *data=uintdup(getQueueStatsCount());
	}
//	else if( strcmp( lastname, "DefaultForwardingPolicy" )==0 )
//	{
//		 *data=uintdup( 0 );
//	}
//	else if( strcmp( lastname, "DefaultPolicer" )==0 )
//	{
//		 *data=intdup( -1 );
//	}
//	else if( strcmp( lastname, "DefaultQueue" )==0 )
//	{
//	}
//	else if( strcmp( lastname, "DefaultDSCPMark" )==0 )
//	{
//		 *data=intdup( -1 );
//	}
	else if( strcmp( lastname, "DefaultEthernetPriorityMark" )==0 )
	{
		 *data=intdup( -1 );
	}
	else if( strcmp( lastname, "AvailableAppList" )==0 )
	{
		 *data=strdup( "" );
	}
	else{
		return ERR_9005;
	}
	
	return 0;
}

int setQueueMnt(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned char vChar=0;
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;

		if(i==NULL) return ERR_9007;

		vChar = (*i == 0) ? 0 : 1;
		mib_set(MIB_QOS_ENABLE_QOS, (void *)&vChar);

		//apply_IPQoS has higher priority than apply_IPQoSRule
		apply_add( CWMP_PRI_H, apply_IPQoS, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
//	else if( strcmp( lastname, "DefaultForwardingPolicy" )==0 )
//	{
//	}
//	else if( strcmp( lastname, "DefaultPolicer" )==0 )
//	{
//	}
//	else if( strcmp( lastname, "DefaultQueue" )==0 )
//	{
//	}
//	else if( strcmp( lastname, "DefaultDSCPMark" )==0 )
//	{
//	}
	else if( strcmp( lastname, "DefaultEthernetPriorityMark" )==0 )
	{
		int *i = data;

		if(i==NULL) return ERR_9007;
		if(*i!=-1) return ERR_9001;
		return 0;
	}
	else{
		return ERR_9005;
	}
	
	return 0;
}


/**************************************************************************************/
/* utility functions*/
/**************************************************************************************/

unsigned int getQueueInstNum( char *name )
{
	return getInstNum( name, "Queue" );
}

unsigned int getQueueStatsInstNum( char *name )
{
	return getInstNum( name, "QueueStats" );
}

unsigned int getClassInstNum( char *name )
{
	return getInstNum( name, "Classification" );
}

unsigned int findClassInstNum(void)
{
	unsigned int ret=0, i,num;
	MIB_CE_IP_QOS_T *p,qos_entity;
	
	num = mib_chain_total( MIB_IP_QOS_TBL );
	for( i=0; i<num;i++ )
	{
		p = &qos_entity;
		if( !mib_chain_get( MIB_IP_QOS_TBL, i, (void*)p ))
			continue;
		if( p->InstanceNum > ret )
			ret = p->InstanceNum;
	}
	
	return ret;
}

int getClassEntryByInstNum(unsigned int instnum, MIB_CE_IP_QOS_T *p, unsigned int *id)
{
	int ret=-1;
	unsigned int i,num;
	
	if( (instnum==0) || (p==NULL) || (id==NULL) ) return ret;
	
	num = mib_chain_total( MIB_IP_QOS_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_IP_QOS_TBL, i, (void*)p ))
			continue;
		if( p->InstanceNum == instnum )
		{
			*id = i;
			ret=0;
			break;
		}
	}
	
	return ret;
}

