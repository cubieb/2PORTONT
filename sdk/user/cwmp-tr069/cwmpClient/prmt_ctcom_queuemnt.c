#include "prmt_ctcom_queuemnt.h"
#include "prmt_queuemnt.h"

int getClassIndex();

#ifdef _PRMT_X_STD_QOS_

extern unsigned int getInstNum( char *name, char *objname );
unsigned int getQueueInstNum( char *name );
unsigned int getClassInstNum( char *name );
unsigned int findClassInstNum(void);
int getClassEntryByInstNum(unsigned int instnum, MIB_CE_IP_QOS_T *p, unsigned int *id);
int getQWaninterface( char *name, int instnum );
int getClassQueue(MIB_CE_IP_QOS_T* p);
int getqosnum();
int getQueueIfindex();

#define MAX_CLASS_RULE 256
#define MAX_QUEUE_NUM  256


struct CWMP_OP tQueueEntityLeafOP = { getQueueEntity, setQueueEntity };
struct CWMP_PRMT tQueueEntityLeafInfo[] =
{
/*(name,		type,		flag,			op)*/
{"QueueKey",		eCWMP_tUINT,	CWMP_READ,		&tQueueEntityLeafOP},
{"QueueEnable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tQueueEntityLeafOP},
{"QueueStatus",		eCWMP_tSTRING,	CWMP_READ,		&tQueueEntityLeafOP},
{"QueueInterface",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tQueueEntityLeafOP},
{"QueueBufferLength",	eCWMP_tUINT,	CWMP_READ,		&tQueueEntityLeafOP},
//{"QueueWeight",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tQueueEntityLeafOP},
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
	eQE_QueueKey,
	eQE_QueueEnable,
	eQE_QueueStatus,
	eQE_QueueInterface,
	eQE_QueueBufferLength,
	eQE_QueuePrecedence,
	eQE_DropAlgorithm,
	eQE_SchedulerAlgorithm
};
struct CWMP_LEAF tQueueEntityLeaf[] =
{
{ &tQueueEntityLeafInfo[eQE_QueueKey] },
{ &tQueueEntityLeafInfo[eQE_QueueEnable] },
{ &tQueueEntityLeafInfo[eQE_QueueStatus] },
{ &tQueueEntityLeafInfo[eQE_QueueInterface] },
{ &tQueueEntityLeafInfo[eQE_QueueBufferLength] },
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


#if 0
struct sCWMP_ENTITY tPolicerEntity[] =
{
/*(name,		type,		flag,			accesslist,	getvalue,	setvalue,	next_table,	sibling)*/
{"PolicerKey",		eCWMP_tUINT,	CWMP_READ,		NULL,		getPolicerEntity,NULL,		NULL,		NULL},
{"PolicerEnable",	eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	NULL,		getPolicerEntity,setPolicerEntity,NULL,		NULL},
{"PolicerStatus",	eCWMP_tSTRING,	CWMP_READ,		NULL,		getPolicerEntity,NULL,		NULL,		NULL},
{"CommittedRate",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	NULL,		getPolicerEntity,setPolicerEntity,NULL,		NULL},
{"CommittedBurstSize",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	NULL,		getPolicerEntity,setPolicerEntity,NULL,		NULL},
/*ExcessBurstSize*/
/*PeakRate*/
/*PeakBurstSize*/
{"MeterType",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	NULL,		getPolicerEntity,setPolicerEntity,NULL,		NULL},
{"PossibleMeterTypes",	eCWMP_tSTRING,	CWMP_READ,		NULL,		getPolicerEntity,NULL,		NULL,		NULL},
{"ConformingAction",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	NULL,		getPolicerEntity,setPolicerEntity,NULL,		NULL},
/*PartialConformingAction*/
{"NonConformingAction",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	NULL,		getPolicerEntity,setPolicerEntity,NULL,		NULL},
{"CountedPackets",	eCWMP_tUINT,	CWMP_READ,		NULL,		getPolicerEntity,NULL,		NULL,		NULL},
{"CountedBytes",	eCWMP_tUINT,	CWMP_READ,		NULL,		getPolicerEntity,NULL,		NULL,		NULL},
{"",			eCWMP_tNONE,	0,			NULL,		NULL,		NULL,		NULL,		NULL}
};

struct sCWMP_ENTITY tPolicer[] =
{
/*(name,			type,		flag,					accesslist,	getvalue,	setvalue,	next_table,	sibling)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL,		NULL,		NULL,		tPolicerEntity,	NULL}
//{"1",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL,		NULL,		NULL,		tPolicerEntity,	NULL}
};
#endif


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
//{"SourceMACAddress",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
/*SourceMACMask*/
//{"SourceMACExclude",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
//{"DestMACAddress",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
/*DestMACMask*/
//{"DestMACExclude",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
/*Ethertype, EthertypeExclude*/
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
	eQC_SourceIP,
	eQC_SourceMask,
	eQC_Protocol,
//	eQC_ProtocolExclude,
	eQC_DestPort,
	eQC_DestPortRangeMax,
	eQC_SourcePort,
	eQC_SourcePortRangeMax,
	eQC_DSCPCheck,
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
{ &tClassEntityLeafInfo[eQC_SourceIP] },
{ &tClassEntityLeafInfo[eQC_SourceMask] },
{ &tClassEntityLeafInfo[eQC_Protocol] },
//{ &tClassEntityLeafInfo[eQC_ProtocolExclude] },
{ &tClassEntityLeafInfo[eQC_DestPort] },
{ &tClassEntityLeafInfo[eQC_DestPortRangeMax] },
{ &tClassEntityLeafInfo[eQC_SourcePort] },
{ &tClassEntityLeafInfo[eQC_SourcePortRangeMax] },
{ &tClassEntityLeafInfo[eQC_DSCPCheck] },
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
{ &tQueueMntLeafInfo[eQ_DefaultEthernetPriorityMark] },
{ &tQueueMntLeafInfo[eQ_AvailableAppList] },
{ NULL }
};


struct CWMP_OP tQM_Class_OP = { NULL, objClass };
//struct CWMP_OP tQM_Policer_OP = { NULL, objPolicer };
struct CWMP_OP tQM_Queue_OP = { NULL, objQueue };
struct CWMP_PRMT tQueueMntObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Classification",		eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,	&tQM_Class_OP},
//{"Policer",			eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,	&tQM_Policer_OP},
{"Queue",			eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,	&tQM_Queue_OP}
};
enum tQueueMntObject
{
	eQ_Classification,
	//eQ_Policer,
	eQ_Queue
};
struct CWMP_NODE tQueueMntObject[] =
{
/*info,  					leaf,			node)*/
{&tQueueMntObjectInfo[eQ_Classification],	NULL,			NULL},
{&tQueueMntObjectInfo[eQ_Queue],		NULL,			NULL},
{NULL,						NULL,			NULL}
};


int getQueueEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	char 	buff[256]={0};
	unsigned char vChar=0;
	unsigned int  qinst=0;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	qinst = getQueueInstNum( name );
	if(qinst==0) return ERR_9005;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "QueueKey" )==0 )
	{
		*data=uintdup( qinst );
	}else if( strcmp( lastname, "QueueEnable" )==0 )
	{
		 *data=booldup( 1 );
	}else if( strcmp( lastname, "QueueStatus" )==0 )
	{
		 *data=strdup( "Enabled" );
	}else if( strcmp( lastname, "QueueInterface" )==0 )
	{
		char buf[256];
		int ret=0;
		ret=getQWaninterface(buf,qinst);
		if(ret==0)
			 *data=strdup( buf );
		else
			*data=strdup("");
	}else if( strcmp( lastname, "QueueBufferLength" )==0 )
	{
		 *data=uintdup( 8*2048 );
//	}else if( strcmp( lastname, "QueueWeight" )==0 )
//	{
//		 *data=uintdup( 0 );
	}else if( strcmp( lastname, "QueuePrecedence" )==0 )
	{
		 *data=uintdup( ((qinst-1)%IPQOS_NUM_PRIOQ)+1 );
//	}else if( strcmp( lastname, "REDThreshold" )==0 )
//	{
//		 *data=uintdup( 0 );
//	}else if( strcmp( lastname, "REDPercentage" )==0 )
//	{
//		 *data=uintdup( 0 );
	}else if( strcmp( lastname, "DropAlgorithm" )==0 )
	{
		 *data=strdup( "DT" );
	}else if( strcmp( lastname, "SchedulerAlgorithm" )==0 )
	{
		unsigned char policy=0;
		mib_get(MIB_QOS_POLICY, &policy);
		if(policy==0)
		 	*data=strdup( "SP" );
		else
			*data=strdup( "WRR" );
//	}else if( strcmp( lastname, "ShapingRate" )==0 )
//	{
//		 *data=intdup( -1 );
//	}else if( strcmp( lastname, "ShapingBurstSize" )==0 )
//	{
//		 *data=uintdup( 0 );
	}else{
		return ERR_9005;
	}

	return 0;
}


int setQueueEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned int  qinst=0;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	qinst = getQueueInstNum( name );
	if(qinst==0) return ERR_9005;

	if( strcmp( lastname, "QueueEnable" )==0 )
	{
		int *i = data;

		if(i==NULL) return ERR_9007;
		if(*i==0) return ERR_9001;
		return 0;
	}else if( strcmp( lastname, "QueueInterface" )==0 )
	{
		if(buf==NULL) return ERR_9007;
		if( strlen(buf)!=0 ) return ERR_9001;
		return 0;
//	}else if( strcmp( lastname, "QueueWeight" )==0 )
//	{
	}else if( strcmp( lastname, "QueuePrecedence" )==0 )
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
	}else if( strcmp( lastname, "SchedulerAlgorithm" )==0 )
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
//	}else if( strcmp( lastname, "ShapingRate" )==0 )
//	{
//	}else if( strcmp( lastname, "ShapingBurstSize" )==0 )
//	{
	}else{
		return ERR_9005;
	}

	return 0;
}


int objQueue(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);

	switch( type )
	{
	case eCWMP_tINITOBJ:
	     {
	     	int num=0,MaxInstNum=0,i,j;
	     	struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;
		int total;

	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		total=getqosnum();
		printf("queue num=%d",total);

	     	num=IPQOS_NUM_PRIOQ;
		for( i=0; i<(num*total);i++ )
		{
			MaxInstNum = i+1;
			if( create_Object( c, tQueueObject, sizeof(tQueueObject), 1, MaxInstNum ) < 0 )
				return -1;
		}
		add_objectNum( name, MaxInstNum );
		return 0;
	     }
	case eCWMP_tADDOBJ:
	     {
		return ERR_9001;
	     }
	case eCWMP_tDELOBJ:
	     {
		return ERR_9001;
	     }
	case eCWMP_tUPDATEOBJ:
		{
	     	int num=0,i,MaxInstNum=0;
	     	struct CWMP_LINKNODE *old_table;
		int total;

		total=getqosnum();
	     	num=IPQOS_NUM_PRIOQ;

	     	old_table = (struct CWMP_LINKNODE *)entity->next;
	     	entity->next = NULL;
		for( i=0; i<(num*total);i++ )
		{
			MaxInstNum = i+1;
			add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tQueueObject, sizeof(tQueueObject), &MaxInstNum );
		}
		if( old_table )
	     		destroy_ParameterTable( (struct CWMP_NODE *)old_table );

	     	return 0;
	     }
	}
	return -1;
}

#if 0
int getPolicerEntity(char *name, struct sCWMP_ENTITY *entity, int *type, void **data)
{
	char	*lastname = entity->name;
	unsigned char vChar=0;
	struct in_addr ipAddr;
	char buff[256]={0};

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->type;
	*data = NULL;
	if( strcmp( lastname, "PolicerKey" )==0 )
	{
		*data=uintdup( 0 );
	}else if( strcmp( lastname, "PolicerEnable" )==0 )
	{
		 *data=booldup( 0 );
	}else if( strcmp( lastname, "PolicerStatus" )==0 )
	{
		 *data=strdup( "Disabled" );
	}else if( strcmp( lastname, "CommittedRate" )==0 )
	{
		 *data=uintdup( 0 );
	}else if( strcmp( lastname, "CommittedBurstSize" )==0 )
	{
		 *data=uintdup( 0 );
	}else if( strcmp( lastname, "MeterType" )==0 )
	{
		 *data=strdup( "SimpleTokenBucket" );
	}else if( strcmp( lastname, "PossibleMeterTypes" )==0 )
	{
		 *data=strdup( "SimpleTokenBucket,SingleRateThreeColor,TwoRateThreeColor" );
	}else if( strcmp( lastname, "ConformingAction" )==0 )
	{
		 *data=strdup( "Null" );
	}else if( strcmp( lastname, "NonConformingAction" )==0 )
	{
		 *data=strdup( "Drop" );
	}else if( strcmp( lastname, "CountedPackets" )==0 )
	{
		 *data=uintdup( 0 );
	}else if( strcmp( lastname, "CountedBytes" )==0 )
	{
		 *data=uintdup( 0 );
	}else{
		return ERR_9005;
	}

	return 0;
}

int setPolicerEntity(char *name, struct sCWMP_ENTITY *entity, int type, void *data)
{
	char	*lastname = entity->name;
	char	*buf=data;
	unsigned char vChar=0;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->type!=type ) return ERR_9006;

	if( strcmp( lastname, "PolicerEnable" )==0 )
	{
	}else if( strcmp( lastname, "CommittedRate" )==0 )
	{
	}else if( strcmp( lastname, "CommittedBurstSize" )==0 )
	{
	}else if( strcmp( lastname, "MeterType" )==0 )
	{
	}else if( strcmp( lastname, "ConformingAction" )==0 )
	{
	}else if( strcmp( lastname, "NonConformingAction" )==0 )
	{
	}else{
		return ERR_9005;
	}

	return 0;
}




int objPolicer(char *name, struct sCWMP_ENTITY *entity, int type, void *data)
{
	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);

	switch( type )
	{
	case eCWMP_tINITOBJ:
	     {
		return 0;
	     }
	case eCWMP_tADDOBJ:
	     {
		return 0;
	     }
	case eCWMP_tDELOBJ:
	     {
		return 0;
	     }
	case eCWMP_tUPDATEOBJ:
	     {
	     	return 0;
	     }
	}
	return -1;
}
#endif

int getClassEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar=0;
	unsigned int  instnum=0,chainid=0;
	char buf[256]={0};
	MIB_CE_IP_QOS_T qos_entity, *p = &qos_entity;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	instnum=getClassInstNum(name);
	if(instnum==0) return ERR_9005;
	if(getClassEntryByInstNum( instnum, p, &chainid )<0) return ERR_9002;

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "ClassificationKey" )==0 )
	{
		*data = uintdup( p->InstanceNum );
	}else if( strcmp( lastname, "ClassificationEnable" )==0 )
	{
		 *data=booldup( p->enable );
	}else if( strcmp( lastname, "ClassificationStatus" )==0 )
	{
		if( p->enable==0 )
		 	*data=strdup( "Disabled" );
		else
			*data=strdup( "Enabled" );
	}else if( strcmp( lastname, "ClassificationOrder" )==0 )
	{
		 *data=uintdup( chainid+1 );
	}else if( strcmp( lastname, "ClassInterface" )==0 )
	{
		if( p->phyPort==0 )
			*data=strdup( "LAN" );
#if (defined(CONFIG_RTL_MULTI_LAN_DEV)  && defined(IP_QOS_VPORT))
		else if( p->phyPort==1 )
			*data=strdup("InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.1");
		else if( p->phyPort==2 )
			*data=strdup("InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.2");
		else if( p->phyPort==3 )
			*data=strdup("InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.3");
		else if( p->phyPort==4 )
			*data=strdup("InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.4");
#else
		else if( p->phyPort==1 )
			*data=strdup("InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.1");
#endif
		else
			return ERR_9002;
	}else if( strcmp( lastname, "DestIP" )==0 )
	{
		sprintf(buf, "%s", inet_ntoa(*((struct in_addr *)p->dip)));
		*data = strdup( buf );
	}else if( strcmp( lastname, "DestMask" )==0 )
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
//	}else if( strcmp( lastname, "DestIPExclude" )==0 )
//	{
//		 *data=booldup( 0 );
	}else if( strcmp( lastname, "SourceIP" )==0 )
	{
		sprintf(buf, "%s", inet_ntoa(*((struct in_addr *)p->sip)));
		*data = strdup( buf );
	}else if( strcmp( lastname, "SourceMask" )==0 )
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
//	}else if( strcmp( lastname, "SourceIPExclude" )==0 )
//	{
//		 *data=booldup( 0 );
	}else if( strcmp( lastname, "Protocol" )==0 )
	{
		if( p->protoType==PROTO_NONE )
			*data=intdup( -1 );
		else if( p->protoType==2 )//TCP
			*data=intdup( 6 );
		else if( p->protoType==3 )//UDP
			*data=intdup( 17 );
		else if( p->protoType==1 )//ICMP
			*data=intdup( 1 );
		else
			return ERR_9002;
//	}else if( strcmp( lastname, "ProtocolExclude" )==0 )
//	{
//		 *data=booldup( 0 );
	}else if( strcmp( lastname, "DestPort" )==0 )
	{
		if( p->dPort==0 )
			*data=intdup( -1 );
		else
			*data=intdup( p->dPort );
	}else if( strcmp( lastname, "DestPortRangeMax" )==0 )
	{
		if( p->dPort < p->dPortRangeMax )
		 	*data=intdup( p->dPortRangeMax-p->dPort );
		else
			*data=intdup(-1);

//	}else if( strcmp( lastname, "DestPortExclude" )==0 )
//	{
//		 *data=booldup( 0 );
	}else if( strcmp( lastname, "SourcePort" )==0 )
	{
		if( p->sPort==0 )
			*data=intdup( -1 );
		else
			*data=intdup( p->sPort );
	}else if( strcmp( lastname, "SourcePortRangeMax" )==0 )
	{
		 if( p->sPort < p->sPortRangeMax )
		 	*data=intdup( p->sPortRangeMax-p->sPort );
		else
			*data=intdup(-1);
//	}else if( strcmp( lastname, "SourcePortExclude" )==0 )
//	{
//		 *data=booldup( 0 );
//	}else if( strcmp( lastname, "SourceMACAddress" )==0 )
//	{
//		 *data=strdup( "" );
//	}else if( strcmp( lastname, "SourceMACExclude" )==0 )
//	{
//		 *data=booldup( 0 );
//	}else if( strcmp( lastname, "DestMACAddress" )==0 )
//	{
//		 *data=strdup( "" );
//	}else if( strcmp( lastname, "DestMACExclude" )==0 )
//	{
//		 *data=booldup( 0 );
//	}else if( strcmp( lastname, "DSCPCheck" )==0 )
//	{
//		 *data=intdup( -1 );
//	}else if( strcmp( lastname, "DSCPExclude" )==0 )
//	{
//		 *data=booldup( 0 );
//	}else if( strcmp( lastname, "DSCPMark" )==0 )
//	{
//		 *data=intdup( -1 );
#ifdef QOS_DIFFSERV
	}else if( strcmp( lastname, "DSCPCheck" )==0 )
	{
		if( p->qosDscp==0 )
		 	*data=intdup( -1 );
		else
			*data=intdup( ((int)p->qosDscp-1)/4 );
#endif
	}else if( strcmp( lastname, "DSCPMark" )==0 )
	{
		if( p->m_dscp==0 )
		 	*data=intdup( -1 );
		else
			*data=intdup( ((int)p->m_dscp-1)/4 );
	}else if( strcmp( lastname, "EthernetPriorityCheck" )==0 )
	{
		if( p->vlan1p==0 )
		 	*data=intdup( -1 );
		else
			*data=intdup( (int)p->vlan1p-1 );
//	}else if( strcmp( lastname, "EthernetPriorityExclude" )==0 )
//	{
//		 *data=booldup( 0 );
	}else if( strcmp( lastname, "EthernetPriorityMark" )==0 )
	{
		if( p->m_1p==0 )
		 	*data=intdup( -1 );
		else
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
		int queue=getClassQueue(p);
		if(queue==-1)
			return ERR_9002;
		 *data=intdup( queue);
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
	unsigned int  instnum=0,chainid=0;
	MIB_CE_IP_QOS_T *p, qos_entity, old_qos_entity;
	struct in_addr in;
	char	*pzeroip="0.0.0.0";

	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	instnum=getClassInstNum(name);
	if(instnum==0) return ERR_9005;
	p = &qos_entity;
	if(getClassEntryByInstNum( instnum, p, &chainid )<0) return ERR_9002;
	memcpy( &old_qos_entity, &qos_entity, sizeof(MIB_CE_IP_QOS_T) );

	if( strcmp( lastname, "ClassificationEnable" )==0 )
	{
		int *i = data;

		if(i==NULL) return ERR_9007;
		p->enable = (*i==0) ? 0:1;
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QOS_T) );
		return 0;
	}else if( strcmp( lastname, "ClassificationOrder" )==0 )
	{
		unsigned int *i = data;

		if(i==NULL) return ERR_9007;
		if( *i==0 ) return ERR_9007;
		if( *i!=(chainid+1) ) return ERR_9001;
		return 0;
	}else if( strcmp( lastname, "ClassInterface" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strcmp( buf, "LAN" )==0 )
			p->phyPort=0xff;
#if (defined(CONFIG_RTL_MULTI_LAN_DEV)  && defined(IP_QOS_VPORT))
		else if( strcmp( buf, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.4" )==0 )
			p->phyPort=4;
		else if( strcmp( buf, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.3" )==0 )
			p->phyPort=3;
		else if( strcmp( buf, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.2" )==0 )
			p->phyPort=2;
		else if( strcmp( buf, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.1" )==0 )
			p->phyPort=1;
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
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QOS_T) );
		return 0;
	}else if( strcmp( lastname, "DestIP" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) buf=pzeroip;
		if( inet_aton( buf, &in )==0 ) //the ip address is error.
			return ERR_9007;
		memcpy( p->dip, &in, sizeof(in) );
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QOS_T) );
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
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QOS_T) );
		return 0;
//	}else if( strcmp( lastname, "DestIPExclude" )==0 )
//	{
	}else if( strcmp( lastname, "SourceIP" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) buf=pzeroip;
		if( inet_aton( buf, &in )==0 ) //the ip address is error.
			return ERR_9007;
		memcpy( p->sip, &in, sizeof(in) );
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QOS_T) );
		return 0;
	}else if( strcmp( lastname, "SourceMask" )==0 )
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
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QOS_T) );
		return 0;
//	}else if( strcmp( lastname, "SourceIPExclude" )==0 )
//	{
	}else if( strcmp( lastname, "Protocol" )==0 )
	{
		int *pro = data;

		if(pro==NULL) return ERR_9007;
		switch(*pro)
		{
		case -1:
			p->protoType=PROTO_NONE;
			break;
		case 1:
			p->protoType=1; //ICMP
			break;
		case 6:
			p->protoType=2; //TCP
			break;
		case 17:
			p->protoType=3; //UDP
			break;
		default:
			return ERR_9001;
		}
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QOS_T) );
		return 0;
//	}else if( strcmp( lastname, "ProtocolExclude" )==0 )
//	{
	}else if( strcmp( lastname, "DestPort" )==0 )
	{
		int *dport = data;

		if( dport==NULL ) return ERR_9007;
		if( *dport==-1 )
			p->dPort = p->dPortRangeMax = 0;
		else if( (*dport>=1) && (*dport<=65535) )
			p->dPort = p->dPortRangeMax = *dport;
		else
			return ERR_9007;
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QOS_T) );
		return 0;
	}else if( strcmp( lastname, "DestPortRangeMax" )==0 )
	{
		int *dport = data;

		if( dport==NULL ) return ERR_9007;
		if( *dport==-1)
			p->dPortRangeMax=p->dPort;
		else if( (*dport>=1) && (*dport<=65535) )
			p->dPortRangeMax=p->dPort+*dport;
		else
			return ERR_9007;
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QOS_T) );
		return 0;
//	}else if( strcmp( lastname, "DestPortExclude" )==0 )
//	{
	}else if( strcmp( lastname, "SourcePort" )==0 )
	{
		int *sport = data;

		if( sport==NULL ) return ERR_9007;
		if( *sport==-1 )
			p->sPort = p->sPortRangeMax = 0;
		else if( (*sport>=1) && (*sport<=65535) )
			p->sPort = p->sPortRangeMax = *sport;
		else
			return ERR_9007;
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QOS_T) );
		return 0;
	}else if( strcmp( lastname, "SourcePortRangeMax" )==0 )
	{
		int *dport = data;

		if( dport==NULL ) return ERR_9007;
		if( *dport==-1)
			p->sPortRangeMax=p->sPort;
		else if( (*dport>=1) && (*dport<=65535) )
			p->sPortRangeMax=p->sPort+*dport;
		else
			return ERR_9007;
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QOS_T) );
		return 0;
//	}else if( strcmp( lastname, "SourcePortExclude" )==0 )
//	{
//	}else if( strcmp( lastname, "SourceMACAddress" )==0 )
//	{
//	}else if( strcmp( lastname, "SourceMACExclude" )==0 )
//	{
//	}else if( strcmp( lastname, "DestMACAddress" )==0 )
//	{
//	}else if( strcmp( lastname, "DestMACExclude" )==0 )
//	{
#ifdef QOS_DIFFSERV
	}else if( strcmp( lastname, "DSCPCheck" )==0 )
	{
		int *i = data;

		if(i==NULL) return ERR_9007;
		if( *i<0  || *i>63 ) return ERR_9007;
		p->qosDscp = (unsigned char)((*i)*4+1);
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QOS_T) );
		return 0;
#endif
//	}else if( strcmp( lastname, "DSCPExclude" )==0 )
//	{
	}else if( strcmp( lastname, "DSCPMark" )==0 )
	{
		int *i = data;

		if(i==NULL) return ERR_9007;
		if( *i<0  || *i>63 ) return ERR_9007;
		p->m_dscp = (unsigned char)((*i)*4+1);
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QOS_T) );
		return 0;
	}else if( strcmp( lastname, "EthernetPriorityCheck" )==0 )
	{

		int *i = data;

		if(i==NULL) return ERR_9007;
		if( *i<-1 || *i>7 ) return ERR_9007;
		p->vlan1p = (unsigned char)(*i+1);
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QOS_T) );
		return 0;
//	}else if( strcmp( lastname, "EthernetPriorityExclude" )==0 )
//	{
	}else if( strcmp( lastname, "EthernetPriorityMark" )==0 )
	{

		int *i = data;

		if(i==NULL) return ERR_9007;
		if( *i<-1 || *i>7 ) return ERR_9007;
		p->m_1p = (unsigned char)(*i+1);
		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QOS_T) );
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

		int idx,num;
		int *i = data;

		num=getqosnum();
		if(i==NULL) return ERR_9007;
		if( *i<1 || *i>(IPQOS_NUM_PRIOQ*num) ) return ERR_9007;
		p->prior = (unsigned char)((*i-1)%IPQOS_NUM_PRIOQ+1);
		idx=(*i)/IPQOS_NUM_PRIOQ;
		printf("\nnum=%d,*i=%d,idx=%d\n",num,*i,idx);
		if(getQueueIfindex(idx,p)<0)
			return ERR_9002;
		printf("\noutif=%d\n",p->outif);

		mib_chain_update( MIB_IP_QOS_TBL, (unsigned char*)p, chainid );

		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QOS_T) );
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
#ifdef _PRMT_X_CT_COM_QOS_
			if(p->modeTr69!=MODEINTERNET)
				continue;
#endif
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
		int ifindex=0;

	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;
		num = mib_chain_total( MIB_IP_QOS_TBL );
		if(num>=MAX_CLASS_RULE) return ERR_9004;

		//ifindex=getClassIfindex();
		//if(ifindex==-1)
			//return -1;

		ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tClassObject, sizeof(tClassObject), data );
		if( ret >= 0 )
		{
			MIB_CE_IP_QOS_T qos_entry;
			memset( &qos_entry, 0, sizeof( MIB_CE_IP_QOS_T ) );
			qos_entry.InstanceNum = *(unsigned int*)data;
			qos_entry.phyPort=0;
			qos_entry.prior=4;
			if(getClassIfindex(&qos_entry)<0)
				return ERR_9001;
			qos_entry.enable=0;
			strcpy(qos_entry.RuleName,"rule_name");
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
#ifdef _PRMT_X_CT_COM_QOS_
			if(p->modeTr69!=MODEINTERNET)
				continue;
#endif
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
		*data = booldup( 1 );
	}else if( strcmp( lastname, "MaxQueues" )==0 )
	{
		*data=uintdup( MAX_QUEUE_NUM );
	}else if( strcmp( lastname, "MaxClassificationEntries" )==0 )
	{
		*data=uintdup( MAX_CLASS_RULE );
	}else if( strcmp( lastname, "ClassificationNumberOfEntries" )==0 )
	{
		num = mib_chain_total(MIB_IP_QOS_TBL);
		*data=uintdup( num );
	}else if( strcmp( lastname, "MaxAppEntries" )==0 )
	{
		 *data=uintdup( 0 );
	}else if( strcmp( lastname, "AppNumberOfEntries" )==0 )
	{
		 *data=uintdup( 0 );
	}else if( strcmp( lastname, "MaxFlowEntries" )==0 )
	{
		 *data=uintdup( 0 );
	}else if( strcmp( lastname, "FlowNumberOfEntries" )==0 )
	{
		 *data=uintdup( 0 );
	}else if( strcmp( lastname, "MaxPolicerEntries" )==0 )
	{
		 *data=uintdup( 0 );
	}else if( strcmp( lastname, "PolicerNumberOfEntries" )==0 )
	{
		 *data=uintdup( 0 );
	}else if( strcmp( lastname, "MaxQueueEntries" )==0 )
	{
		 *data=uintdup( MAX_QUEUE_NUM );
	}else if( strcmp( lastname, "QueueNumberOfEntries" )==0 )
	{
		int total=getqosnum();
		 *data=uintdup( IPQOS_NUM_PRIOQ*total );
//	}else if( strcmp( lastname, "DefaultForwardingPolicy" )==0 )
//	{
//		 *data=uintdup( 0 );
//	}else if( strcmp( lastname, "DefaultPolicer" )==0 )
//	{
//		 *data=intdup( -1 );
//	}else if( strcmp( lastname, "DefaultQueue" )==0 )
//	{
//		 *data=uintdup( 0 );
//	}else if( strcmp( lastname, "DefaultDSCPMark" )==0 )
//	{
//		 *data=intdup( -1 );
	}else if( strcmp( lastname, "DefaultEthernetPriorityMark" )==0 )
	{
		 *data=intdup( -1 );
	}else if( strcmp( lastname, "AvailableAppList" )==0 )
	{
		 *data=strdup( "" );
	}else{
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
/*
		mib_get(MIB_MPMODE, (void *)&vChar);
		if(*i==0)
			vChar &= 0xfd;
		else
			vChar |= 0x02;
		mib_set(MIB_MPMODE, (void *)&vChar);

		//apply_IPQoS has higher priority than apply_IPQoSRule
		apply_add( CWMP_PRI_H, apply_IPQoS, CWMP_RESTART, 0, NULL, 0 );
		return 0;
*/
		return 0;
//	}else if( strcmp( lastname, "DefaultForwardingPolicy" )==0 )
//	{
//	}else if( strcmp( lastname, "DefaultPolicer" )==0 )
//	{
//	}else if( strcmp( lastname, "DefaultQueue" )==0 )
//	{
//	}else if( strcmp( lastname, "DefaultDSCPMark" )==0 )
//	{
	}else if( strcmp( lastname, "DefaultEthernetPriorityMark" )==0 )
	{
		int *i = data;

		if(i==NULL) return ERR_9007;
		if(*i!=-1) return ERR_9001;
		return 0;
	}else{
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
#ifdef _PRMT_X_CT_COM_QOS_
		if(p->modeTr69!=MODEINTERNET)
			continue;
#endif
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
#ifdef _PRMT_X_CT_COM_QOS_
		if(p->modeTr69!=MODEINTERNET)
			continue;
#endif
		if( p->InstanceNum == instnum )
		{
			*id = i;
			ret=0;
			break;
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

int getClassIfindex(MIB_CE_IP_QOS_T *p)
{
	int vcNum,i;
	MIB_CE_ATM_VC_T pvcEntry;
	int qidx=-1;

	vcNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<vcNum; i++)
	{
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&pvcEntry))
			continue;
		if(pvcEntry.enableIpQos==1){
			p->outif=pvcEntry.ifIndex;
#ifdef BR_ROUTE_ONEPVC
			p->cmode=pvcEntry.cmode;
#endif
			return 1;
		}
	}
	return -1;
}

int getQueueIfindex(int qnum,MIB_CE_IP_QOS_T *p)
{
	int vcNum,i;
	MIB_CE_ATM_VC_T pvcEntry;
	int qidx=-1;

	vcNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<vcNum; i++)
	{
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&pvcEntry))
			continue;
		if(pvcEntry.enableIpQos==0)
			continue;
		qidx++;
		printf("\nqidx=%d,qnum=%d,index=%d\n",qidx,qnum,pvcEntry.ifIndex);
		if(qidx==qnum){
			p->outif=pvcEntry.ifIndex;
#ifdef BR_ROUTE_ONEPVC
			p->cmode=pvcEntry.cmode;
#endif
			return 1;
		}
	}
	return -1;

}

int getClassQueue(MIB_CE_IP_QOS_T* p)
{
	int vcNum,i;
	MIB_CE_ATM_VC_T pvcEntry;
	int qidx=-1;

	vcNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<vcNum; i++)
	{
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&pvcEntry))
			continue;
		if(pvcEntry.enableIpQos==0)
			continue;
		qidx++;
		if(pvcEntry.ifIndex==p->outif)
			break;
	}
	if(i>=vcNum)
		return -1;

	return(qidx*IPQOS_NUM_PRIOQ+p->prior);

}

int getqosnum()
{
	int vcNum,i;
	MIB_CE_ATM_VC_T pvcEntry;
	int qidx=0;

	vcNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<vcNum; i++)
	{
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&pvcEntry))
			continue;
		if(pvcEntry.enableIpQos==1)
			qidx++;
	}
	return qidx;
}
#endif

#ifdef _PRMT_X_CT_COM_QOS_

#define IPQOS_APP_NUM  2

extern unsigned int getInstNum(char *name, char *objname);
static unsigned int getCT_AppInstNum(char *name);
static unsigned int getCT_QueueInstNum(char *name);
static unsigned int getCT_ClassInstNum(char *name);
static unsigned int getCT_TypeInstNum(char *name);
static int getCT_ClassEntryByInstNum(unsigned int instnum, MIB_CE_IP_QOS_T * p,
				     unsigned int *id);
static int getCT_AppEntryByInstNum(unsigned int instnum,
				   MIB_CE_IP_QOS_APP_T * p, unsigned int *id);
static int getCT_QueueEntryByInstNum(unsigned int instnum,
				     MIB_CE_IP_QOS_QUEUE_T * p,
				     unsigned int *id);


struct CWMP_OP tCT_QueueEntityLeafOP = { getCT_QueueEntity, setCT_QueueEntity };
struct CWMP_PRMT tCT_QueueEntityLeafInfo[] =
{
/*(name,		type,		flag,			op)*/
{"Enable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,		&tCT_QueueEntityLeafOP},
{"Priority",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tCT_QueueEntityLeafOP},
{"Weight",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,		&tCT_QueueEntityLeafOP},
};
enum eCT_QueueEntityLeaf
{
	eCT_QE_Enable,
	eCT_QE_Priority,
	eCT_QE_Weight,
};
struct CWMP_LEAF tCT_QueueEntityLeaf[] =
{
{ &tCT_QueueEntityLeafInfo[eCT_QE_Enable] },
{ &tCT_QueueEntityLeafInfo[eCT_QE_Priority] },
{ &tCT_QueueEntityLeafInfo[eCT_QE_Weight] },
{ NULL }
};

struct CWMP_PRMT tCT_QueueObjectInfo[] =
{
/*(name,			type,		flag,					op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eCT_QueueObject
{
	eCT_QueueObject0
};
struct CWMP_LINKNODE tCT_QueueObject[] =
{
/*info,  				leaf,			next,		sibling,		instnum)*/
{&tCT_QueueObjectInfo[eCT_QueueObject0],	tCT_QueueEntityLeaf,	NULL,		NULL,			0},
};


struct CWMP_OP tCT_ClassEntityLeafOP = { getCT_ClassEntity, setCT_ClassEntity };
struct CWMP_PRMT tCT_ClassEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"ClassQueue",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,		&tCT_ClassEntityLeafOP},
{"DSCPMarkValue",			eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tCT_ClassEntityLeafOP},
{"802-1_P_Value",			eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tCT_ClassEntityLeafOP},
};
enum eCT_ClassEntityLeaf
{
	eCT_QC_ClassQueue,
	eCT_QC_DSCPMarkValue,
	eCT_QC_8021_P_Value,
};
struct CWMP_LEAF tCT_ClassEntityLeaf[] =
{
{ &tCT_ClassEntityLeafInfo[eCT_QC_ClassQueue] },
{ &tCT_ClassEntityLeafInfo[eCT_QC_DSCPMarkValue] },
{ &tCT_ClassEntityLeafInfo[eCT_QC_8021_P_Value] },
{ NULL }
};


struct CWMP_OP tCT_TypeEntityLeafOP = { getCT_TypeEntity, setCT_TypeEntity };
struct CWMP_PRMT tCT_TypeEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Type",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_TypeEntityLeafOP},
{"Max",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,		&tCT_TypeEntityLeafOP},
{"Min",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_TypeEntityLeafOP},
{"ProtocolList",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_TypeEntityLeafOP},
};
enum eCT_TypeEntityLeaf
{
	eCT_QC_Type,
	eCT_QC_Max,
	eCT_QC_Min,
	eCT_QC_ProtocolList,
};
struct CWMP_LEAF tCT_TypeEntityLeaf[] =
{
{ &tCT_TypeEntityLeafInfo[eCT_QC_Type] },
{ &tCT_TypeEntityLeafInfo[eCT_QC_Max] },
{ &tCT_TypeEntityLeafInfo[eCT_QC_Min] },
{ &tCT_TypeEntityLeafInfo[eCT_QC_ProtocolList] },
{ NULL }
};

struct CWMP_PRMT tCT_TypeObjectInfo[] =
{
/*(name,			type,		flag,					op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eCT_TypeObject
{
	eCT_Type0
};
struct CWMP_LINKNODE tCT_TypeObject[] =
{
/*info,  				leaf,			next,		sibling,		instnum)*/
{&tCT_TypeObjectInfo[eCT_Type0],	tCT_TypeEntityLeaf,	NULL,		NULL,			0},
};



struct CWMP_OP tCT_Type_OP = { NULL, objCT_Type };

struct CWMP_PRMT tCT_ClassTypeObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"type",			eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,	&tCT_Type_OP},
};
enum eCT_ClassTypeObject
{
	eCT_ClassType,
};
struct CWMP_NODE tCT_ClassEntityObject[] =
{
/*info,  							leaf,			node)*/
{&tCT_ClassTypeObjectInfo[eCT_ClassType],			NULL,			NULL},
{NULL,								NULL,			NULL}
};

struct CWMP_PRMT tCT_ClassObjectInfo[] =
{
/*(name,			type,		flag,					op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eCT_ClassObject
{
	eCT_ClassObject0
};
struct CWMP_LINKNODE tCT_ClassObject[] =
{
/*info,  				leaf,			next,		sibling,		instnum)*/
{&tCT_ClassObjectInfo[eCT_ClassObject0],	tCT_ClassEntityLeaf,	tCT_ClassEntityObject,		NULL,			0},
};


struct CWMP_OP tCT_AppEntityLeafOP = { getCT_AppEntity, setCT_AppEntity };
struct CWMP_PRMT tCT_AppEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"AppName",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,		&tCT_AppEntityLeafOP},
{"ClassQueue",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tCT_AppEntityLeafOP},
};
enum eCT_AppEntityLeaf
{
	eCT_QA_AppName,
	eCT_QA_ClassQueue,
};
struct CWMP_LEAF tCT_AppEntityLeaf[] =
{
{ &tCT_AppEntityLeafInfo[eCT_QA_AppName] },
{ &tCT_AppEntityLeafInfo[eCT_QA_ClassQueue] },
{ NULL }
};

struct CWMP_PRMT tCT_AppObjectInfo[] =
{
/*(name,			type,		flag,					op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eCT_AppObject
{
	eCT_AppObject0
};
struct CWMP_LINKNODE tCT_AppObject[] =
{
/*info,  				leaf,			next,		sibling,		instnum)*/
{&tCT_AppObjectInfo[eCT_AppObject0],	tCT_AppEntityLeaf,	NULL,		NULL,			0},
};

struct CWMP_OP tCT_UplinkQoSLeafOP = { getCT_UplinkQoS,	setCT_UplinkQoS };
struct CWMP_PRMT tCT_UplinkQoSInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Mode",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_UplinkQoSLeafOP},
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_UplinkQoSLeafOP},
{"Bandwidth",			eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tCT_UplinkQoSLeafOP},
{"Plan",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_UplinkQoSLeafOP},
{"EnableForceWeight",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_UplinkQoSLeafOP},
{"EnableDSCPMark",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_UplinkQoSLeafOP},
{"Enable802-1_P",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tCT_UplinkQoSLeafOP},
};
enum eCT_UplinkQoSLeaf
{
	eCT_Q_Mode,
	eCT_Q_Enable,
	eCT_Q_Bandwidth,
	eCT_Q_Plan,
	eCT_Q_EnableForceWeight,
	eCT_Q_EnableDSCPMark,
	eCT_Q_Enable8021_P,
};
struct CWMP_LEAF tCT_UplinkQoSLeaf[] =
{
{ &tCT_UplinkQoSInfo[eCT_Q_Mode] },
{ &tCT_UplinkQoSInfo[eCT_Q_Enable] },
{ &tCT_UplinkQoSInfo[eCT_Q_Bandwidth] },
{ &tCT_UplinkQoSInfo[eCT_Q_Plan] },
{ &tCT_UplinkQoSInfo[eCT_Q_EnableForceWeight] },
{ &tCT_UplinkQoSInfo[eCT_Q_EnableDSCPMark] },
{ &tCT_UplinkQoSInfo[eCT_Q_Enable8021_P] },
{ NULL }
};

struct CWMP_OP tCT_QM_App_OP = { NULL, objCT_App };
struct CWMP_OP tCT_QM_Class_OP = { NULL, objCT_Class };
struct CWMP_OP tCT_QM_Queue_OP = { NULL, objCT_Queue };
struct CWMP_PRMT tCT_UplinkQoSObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"App",				eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,	&tCT_QM_App_OP},
{"Classification",		eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,	&tCT_QM_Class_OP},
{"PriorityQueue",		eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,	&tCT_QM_Queue_OP}
};
enum eCT_UplinkQoSObject
{
	eCT_Q_App,
	eCT_Q_Classification,
	eCT_Q_Queue
};
struct CWMP_NODE tCT_UplinkQoSObject[] =
{
/*info,  					leaf,			node)*/
{&tCT_UplinkQoSObjectInfo[eCT_Q_App],	NULL,			NULL},
{&tCT_UplinkQoSObjectInfo[eCT_Q_Classification],	NULL,			NULL},
{&tCT_UplinkQoSObjectInfo[eCT_Q_Queue],		NULL,			NULL},
{NULL,						NULL,			NULL}
};


int getCT_QueueEntity(char *name, struct CWMP_LEAF *entity, int *type,
		      void **data)
{
	char *lastname = entity->info->name;
	unsigned int instnum = 0, chainid = 0;
	MIB_CE_IP_QOS_QUEUE_T queue_entity, *p = &queue_entity;

	if ((name == NULL) || (type == NULL) || (data == NULL)
	    || (entity == NULL))
		return -1;

	instnum = getCT_QueueInstNum(name);
	if (instnum == 0)
		return ERR_9005;
	if (getCT_QueueEntryByInstNum(instnum, p, &chainid) < 0)
		return ERR_9001;

	*type = entity->info->type;
	*data = NULL;

	if (strcmp(lastname, "Enable") == 0) {
		*data = booldup(p->enable);
	} else if (strcmp(lastname, "Priority") == 0) {
		*data = uintdup(p->QueueInstNum);
	} else if (strcmp(lastname, "Weight") == 0) {
		*data = uintdup(p->weight);
	} else {
		return ERR_9005;
	}

	return 0;
}

int setCT_QueueEntity(char *name, struct CWMP_LEAF *entity, int type,
		      void *data)
{
	char *lastname = entity->info->name;
	char *buf = data;
	unsigned int instnum = 0, chainid = 0;
	MIB_CE_IP_QOS_QUEUE_T queue_entity, *p = &queue_entity;

	if ((name == NULL) || (data == NULL) || (entity == NULL))
		return -1;
	if (entity->info->type != type)
		return ERR_9006;

	instnum = getCT_QueueInstNum(name);
	if (instnum == 0)
		return ERR_9005;
	if (getCT_QueueEntryByInstNum(instnum, p, &chainid) < 0)
		return ERR_9001;

	if (strcmp(lastname, "Enable") == 0) {
		int *i = data;

		p->enable = *i ? 1 : 0;

		mib_chain_update(MIB_IP_QOS_QUEUE_TBL, p, chainid);

		apply_add(CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL,
			  sizeof(MIB_CE_IP_QOS_T));
		return 0;
	} else if (strcmp(lastname, "Priority") == 0) {
		unsigned int *i = data;

		if (*i != instnum)
			return ERR_9007;

		return 0;
	} else if (strcmp(lastname, "Weight") == 0) {
		unsigned int *i = data;

		if (*i > 100)
			return ERR_9007;

		p->weight = *i;

		mib_chain_update(MIB_IP_QOS_QUEUE_TBL, p, chainid);

		apply_add(CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL,
			  sizeof(MIB_CE_IP_QOS_T));
		return 0;
	} else {
		return ERR_9005;
	}

	return 0;
}

int objCT_Queue(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity = (struct CWMP_NODE *)e;
	int i, num;
	MIB_CE_IP_QOS_QUEUE_T queue_entity, *p = &queue_entity;

	if (name == NULL || entity == NULL)
		return -1;

	switch (type) {
	case eCWMP_tINITOBJ:
		{
			unsigned int QueueInstNum;
			struct CWMP_LINKNODE **c = data;

			if (data == NULL)
				return -1;

			num = mib_chain_total(MIB_IP_QOS_QUEUE_TBL);
			for (i = 0, QueueInstNum = 0; i < num; i++) {
				if (!mib_chain_get(MIB_IP_QOS_QUEUE_TBL, i, p))
					continue;

				if (create_Object
				    (c, tCT_QueueObject,
				     sizeof(tCT_QueueObject), 1,
				     ++QueueInstNum) < 0)
					return -1;

				if (p->QueueInstNum != QueueInstNum) {
					p->QueueInstNum = QueueInstNum;
					mib_chain_update(MIB_IP_QOS_QUEUE_TBL,
							 p, i);
				}
			}
			clear_objectNum(name);

			return 0;
		}
	case eCWMP_tADDOBJ:
		if (data == NULL)
			return -1;

		return ERR_9001;

	case eCWMP_tDELOBJ:
		if (data == NULL)
			return -1;

		return ERR_9001;

	case eCWMP_tUPDATEOBJ:
		{
			unsigned int QueueInstNum;
			struct CWMP_LINKNODE *old_table, *remove_entity;

			clear_objectNum(name);

			/* isolates entity->next to old_table */
			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;

			num = mib_chain_total(MIB_IP_QOS_QUEUE_TBL);
			for (i = 0; i < num; i++) {
				if (!mib_chain_get(MIB_IP_QOS_QUEUE_TBL, i, p))
					continue;
				/*
				 * pickes the needed entity from old_table, 
				 * and adds to entity->next
				 */
				remove_entity =
				    remove_SiblingEntity(&old_table,
							 p->QueueInstNum);
				if (remove_entity) {
					add_SiblingEntity((struct CWMP_LINKNODE
							   **)&entity->next,
							  remove_entity);
				} else {
					QueueInstNum = p->QueueInstNum;
					add_Object(name,
						   (struct CWMP_LINKNODE **)
						   &entity->next,
						   tCT_QueueObject,
						   sizeof(tCT_QueueObject),
						   &QueueInstNum);

					if (p->QueueInstNum != QueueInstNum) {
						p->QueueInstNum = QueueInstNum;
						mib_chain_update
						    (MIB_IP_QOS_QUEUE_TBL, p,
						     i);
					}
				}
			}

			if (old_table)
				destroy_ParameterTable((struct CWMP_NODE *)
						       old_table);
			return 0;
		}
	}
	return -1;
}

static unsigned int find_max_CT_Class_instanNum()
{
	int total = mib_chain_total(MIB_IP_QOS_TBL);
	int i = 0, max = 0;
	MIB_CE_IP_QOS_T qos_entity;

	for(i = 0 ; i < total ; i++)
	{
		if(!mib_chain_get(MIB_IP_QOS_TBL, i, &qos_entity))
			continue;

		if (qos_entity.modeTr69 == MODEINTERNET)
			continue;

		if(qos_entity.InstanceNum > max)
			max = qos_entity.InstanceNum;
	}

	return max;
}

int getCT_ClassEntity(char *name, struct CWMP_LEAF *entity, int *type,
		      void **data)
{
	char *lastname = entity->info->name;
	unsigned int instnum = 0, chainid = 0;
	MIB_CE_IP_QOS_T qos_entity, *p = &qos_entity;

	if ((name == NULL) || (type == NULL) || (data == NULL)
	    || (entity == NULL))
		return -1;

	instnum = getCT_ClassInstNum(name);
	if (instnum == 0)
		return ERR_9005;
	if (getCT_ClassEntryByInstNum(instnum, p, &chainid) < 0)
		return ERR_9001;

	*type = entity->info->type;
	*data = NULL;

	if (strcmp(lastname, "ClassQueue") == 0) {
		*data = uintdup(p->prior);
	} else if (strcmp(lastname, "DSCPMarkValue") == 0) {
		if (p->m_dscp == 0)
			*data = uintdup(0);
		else
			*data = uintdup((p->m_dscp - 1) >> 2);
	} else if (strcmp(lastname, "802-1_P_Value") == 0) {
		if (p->m_1p == 0)
			*data = uintdup(0);
		else
			*data = uintdup(p->m_1p - 1);
	} else {
		return ERR_9005;
	}

	return 0;
}

int setCT_ClassEntity(char *name, struct CWMP_LEAF *entity, int type,
		      void *data)
{
	char *lastname = entity->info->name;
	char *buf = data;
	unsigned char vChar = 0;
	unsigned int instnum = 0, chainid = 0;
	MIB_CE_IP_QOS_T qos_entity, *p = &qos_entity;

	if ((name == NULL) || (data == NULL) || (entity == NULL))
		return -1;
	if (entity->info->type != type)
		return ERR_9006;

	instnum = getCT_ClassInstNum(name);
	if (instnum == 0)
		return ERR_9005;
	if (getCT_ClassEntryByInstNum(instnum, p, &chainid) < 0)
		return ERR_9001;

	if (strcmp(lastname, "ClassQueue") == 0) {
		unsigned int *i = data;

		if (i == NULL)
			return ERR_9007;

		p->prior = *i;

		mib_chain_update(MIB_IP_QOS_TBL, p, chainid);

		apply_add(CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL,
			  sizeof(MIB_CE_IP_QOS_T));
		return 0;
	} else if (strcmp(lastname, "DSCPMarkValue") == 0) {
		unsigned int *i = data;

		if (i == NULL)
			return ERR_9007;
		if (*i < 0 || *i > 63)
			return ERR_9007;

		p->m_dscp = (*i) << 2 + 1;

		mib_chain_update(MIB_IP_QOS_TBL, p, chainid);

		apply_add(CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL,
			  sizeof(MIB_CE_IP_QOS_T));
		return 0;
	} else if (strcmp(lastname, "802-1_P_Value") == 0) {
		unsigned int *i = data;

		if (i == NULL)
			return ERR_9007;
		if (*i < 0 || *i > 7)
			return ERR_9007;

		p->m_1p = *i + 1;

		mib_chain_update(MIB_IP_QOS_TBL, p, chainid);

		apply_add(CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL,
			  sizeof(MIB_CE_IP_QOS_T));
		return 0;
	} else
		return ERR_9005;
	return 0;
}

/* Must sync with boa_e8 fmqos_e8b.c */
#define QUEUE_RULE_NUM_MAX     256
int objCT_Class(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity = (struct CWMP_NODE *)e;
	int i, num;
	MIB_CE_IP_QOS_T qos_entity = {0}, *p = &qos_entity;

	if (name == NULL || entity == NULL)
		return -1;

	switch (type) {
	case eCWMP_tINITOBJ:
		{
			unsigned int InstanceNum;
			struct CWMP_LINKNODE **c = data;

			if (data == NULL)
				return -1;

			num = mib_chain_total(MIB_IP_QOS_TBL);
			for (i = 0, InstanceNum = 0; i < num; i++) {
				if (!mib_chain_get(MIB_IP_QOS_TBL, i, p))
					continue;
				if (p->modeTr69 != MODEIPTV
				    && p->modeTr69 != MODEOTHER)
					continue;

				if (create_Object
				    (c, tCT_ClassObject,
				     sizeof(tCT_ClassObject), 1,
				     ++InstanceNum) < 0)
					return -1;

				if (p->InstanceNum != InstanceNum) {
					p->InstanceNum = InstanceNum;
					mib_chain_update(MIB_IP_QOS_TBL, p, i);
				}
			}

			return 0;
		}
	case eCWMP_tADDOBJ:
		{
			int ret;
			int total = mib_chain_total(MIB_IP_QOS_TBL);

			if (data == NULL)
				return -1;

			if(total >= QUEUE_RULE_NUM_MAX)
				return ERR_9004;

			*(unsigned int *)data = find_max_CT_Class_instanNum() + 1;

			ret = add_Object(name, (struct CWMP_LINKNODE **)&entity->next,
				       tCT_ClassObject, sizeof(tCT_ClassObject), data);

			if (ret >= 0)
			{
				p->InstanceNum = *(unsigned int *)data;
				p->modeTr69 = MODEOTHER;
				p->IpProtocol = IPVER_IPV4;	//v6 need to set on GUI
				sprintf(p->RuleName, "rule_%d", p->InstanceNum);
				mib_chain_add(MIB_IP_QOS_TBL, p);
			}

			return 0;
		}
	case eCWMP_tDELOBJ:
		{
			int ret, chainid;

			if (data == NULL)
				return -1;

			i = *(unsigned int *)data;
			if(i == 0)
				return ERR_9003;

			if (getCT_ClassEntryByInstNum(i, p, &chainid) < 0)
				return ERR_9003;

			if (p->modeTr69 != MODEIPTV && p->modeTr69 != MODEOTHER)
				return ERR_9001;

			ret = del_Object(name, (struct CWMP_LINKNODE **)&entity->next, i);

			if (ret >= 0)
				mib_chain_delete(MIB_IP_QOS_TBL, chainid);

			apply_IPQoSRule( CWMP_RESTART, chainid, NULL );
			return 0;
		}
	case eCWMP_tUPDATEOBJ:
		{
			unsigned int InstanceNum;
			struct CWMP_LINKNODE *old_table, *remove_entity;

			clear_objectNum(name);

			/* isolates entity->next to old_table */
			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;

			num = mib_chain_total(MIB_IP_QOS_TBL);
			for (i = 0; i < num; i++) {
				if (!mib_chain_get(MIB_IP_QOS_TBL, i, p))
					continue;
				if (p->modeTr69 != MODEIPTV
				    && p->modeTr69 != MODEOTHER)
					continue;
				/*
				 * pickes the needed entity from old_table, 
				 * and adds to entity->next
				 */
				remove_entity =
				    remove_SiblingEntity(&old_table,
							 p->InstanceNum);
				if (remove_entity) {
					add_SiblingEntity((struct CWMP_LINKNODE
							   **)&entity->next,
							  remove_entity);
				} else {
					InstanceNum = p->InstanceNum;
					if(InstanceNum == 0)
						InstanceNum = find_max_CT_Class_instanNum() + 1;
					add_Object(name,
						   (struct CWMP_LINKNODE **)
						   &entity->next,
						   tCT_ClassObject,
						   sizeof(tCT_ClassObject),
						   &InstanceNum);

					if (p->InstanceNum != InstanceNum) {
						p->InstanceNum = InstanceNum;
						mib_chain_update(MIB_IP_QOS_TBL,
								 p, i);
					}
				}
			}

			if (old_table)
				destroy_ParameterTable((struct CWMP_NODE *)
						       old_table);
			return 0;
		}
	}
	return -1;
}

int getCT_TypeEntity(char *name, struct CWMP_LEAF *entity, int *type,
		     void **data)
{
	char *lastname = entity->info->name;
	char buf[256];
	unsigned int instnum = 0, chainid = 0, typeinst = 0;
	MIB_CE_IP_QOS_T qos_entity, *p = &qos_entity;

	if ((name == NULL) || (type == NULL) || (data == NULL)
	    || (entity == NULL))
		return -1;

	instnum = getCT_ClassInstNum(name);
	typeinst = getCT_TypeInstNum(name);
	if (instnum == 0 || typeinst <= 0 || typeinst > CT_TYPE_NUM)
		return ERR_9005;
	if (getCT_ClassEntryByInstNum(instnum, p, &chainid) < 0)
		return ERR_9001;
	if (p->cttypemap[typeinst - 1] == 0)
		return ERR_9005;

	*type = entity->info->type;
	*data = NULL;

	if (strcmp(lastname, "Type") == 0) {
		if (getcttype(buf, p, typeinst - 1) == 0)
			*data = strdup(buf);
		else
			*data = strdup("");
	} else if (strcmp(lastname, "Max") == 0) {
		if (getcttypevalue(buf, p, typeinst - 1, 1) == 0)
			*data = strdup(buf);
		else
			*data = strdup("");
	} else if (strcmp(lastname, "Min") == 0) {
		if (getcttypevalue(buf, p, typeinst - 1, 0) == 0)
			*data = strdup(buf);
		else
			*data = strdup("");
	} else if (strcmp(lastname, "ProtocolList") == 0) {
		switch (p->protoType) {
		case PROTO_TCP:
			strcpy(buf, "TCP");
			break;
		case PROTO_UDP:
			strcpy(buf, "UDP");
			break;
		case PROTO_ICMP:
			strcpy(buf, "ICMP");
			break;
		case PROTO_UDPTCP:
			strcpy(buf, "TCP,UDP");
			break;
		case PROTO_RTP:
			strcpy(buf, "RTP");
			break;
		default:
			strcpy(buf, "");
			break;
		}
		*data = strdup(buf);
	} else {
		return ERR_9005;
	}

	return 0;
}

int setCT_TypeEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;
	char *buf = data;
	unsigned int instnum = 0, chainid = 0, typeinst = 0;
	MIB_CE_IP_QOS_T qos_entity, *p = &qos_entity;

	if ((name == NULL) || (data == NULL) || (entity == NULL))
		return -1;
	if (entity->info->type != type)
		return ERR_9006;

	instnum = getCT_ClassInstNum(name);
	typeinst = getCT_TypeInstNum(name);
	if (instnum == 0 || typeinst <= 0 || typeinst > CT_TYPE_NUM)
		return ERR_9005;
	if (getCT_ClassEntryByInstNum(instnum, p, &chainid) < 0)
		return ERR_9001;
	if (p->cttypemap[typeinst - 1] == 0)
		return ERR_9005;

	if (strcmp(lastname, "Type") == 0) {
		if (strlen(buf) == 0)
			return ERR_9007;

		if (setcttype(buf, p, typeinst - 1) == 0) {
			mib_chain_update(MIB_IP_QOS_TBL, p, chainid);
		} else
			return ERR_9001;

		apply_add(CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL,
			  sizeof(MIB_CE_IP_QOS_T));
		return 0;
	} else if (strcmp(lastname, "Max") == 0) {
		if (strlen(buf) == 0)
			return ERR_9007;

		if (setcttypevalue(buf, p, typeinst - 1, 1) == 0)
			mib_chain_update(MIB_IP_QOS_TBL, p, chainid);
		else
			return ERR_9007;

		apply_add(CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL,
			  sizeof(MIB_CE_IP_QOS_T));
		return 0;
	} else if (strcmp(lastname, "Min") == 0) {
		if (strlen(buf) == 0)
			return ERR_9007;

		if (setcttypevalue(buf, p, typeinst - 1, 0) == 0)
			mib_chain_update(MIB_IP_QOS_TBL, p, chainid);
		else
			return ERR_9007;

		apply_add(CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL,
			  sizeof(MIB_CE_IP_QOS_T));
		return 0;
	} else if (strcmp(lastname, "ProtocolList") == 0) {
		if (strstr(buf, "TCP") && strstr(buf, "UDP"))
			p->protoType = PROTO_UDPTCP;
		else if (strstr(buf, "TCP"))
			p->protoType = PROTO_TCP;
		else if (strstr(buf, "UDP"))
			p->protoType = PROTO_UDP;
		else if (strstr(buf, "ICMP"))
			p->protoType = PROTO_ICMP;
		else if (strstr(buf, "RTP"))
			p->protoType = PROTO_RTP;
		else
			p->protoType = PROTO_NONE;

		mib_chain_update(MIB_IP_QOS_TBL, p, chainid);

		apply_add(CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL,
			  sizeof(MIB_CE_IP_QOS_T));
		return 0;
	} else
		return ERR_9005;
	return 0;
}

int objCT_Type(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity = (struct CWMP_NODE *)e;
	unsigned int i;
	unsigned int instnum = 0, chainid = 0;
	MIB_CE_IP_QOS_T qos_entity, *p = &qos_entity;

	if (name == NULL || entity == NULL)
		return -1;

	instnum = getCT_ClassInstNum(name);
	if (instnum == 0)
		return ERR_9005;
	if (getCT_ClassEntryByInstNum(instnum, p, &chainid) < 0)
		return ERR_9001;

	switch (type) {
	case eCWMP_tINITOBJ:
		{
			struct CWMP_LINKNODE **c = data;

			if (data == NULL)
				return -1;

			for (i = 0; i < CT_TYPE_NUM; i++) {
				if (p->cttypemap[i] == 0)
					continue;

				if (create_Object
				    (c, tCT_TypeObject,
				     sizeof(tCT_TypeObject), 1, i + 1) < 0)
					return -1;
			}

			return 0;
		}
	case eCWMP_tADDOBJ:
		{
			int ret;

			if (data == NULL)
				return -1;

			for (i = 0; i < CT_TYPE_NUM; i++) {
				if (p->cttypemap[i] == 0)
					break;
			}

			if (i == CT_TYPE_NUM)
				return ERR_9004;

			*(unsigned int *)data = i + 1;

			ret =
			    add_Object(name,
				       (struct CWMP_LINKNODE **)&entity->next,
				       tCT_TypeObject, sizeof(tCT_TypeObject),
				       data);

			if (ret >= 0) {
				p->cttypemap[i] = 0xf;
				mib_chain_update(MIB_IP_QOS_TBL, p, chainid);
			}

			apply_add(CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0,
				  NULL, sizeof(MIB_CE_IP_QOS_T));
			return 0;
		}
	case eCWMP_tDELOBJ:
		{
			int ret;

			if (data == NULL)
				return -1;

			i = *(unsigned int *)data;
			if (i == 0 || i > CT_TYPE_NUM)
				return ERR_9001;

			ret =
			    del_Object(name,
				       (struct CWMP_LINKNODE **)&entity->next,
				       i);

			if (p->cttypemap[i - 1] == 0)
				return 0;

			if (ret >= 0 && delcttypevalue(p, i - 1) >= 0) {
				mib_chain_update(MIB_IP_QOS_TBL, p, chainid);
			}

			apply_IPQoSRule( CWMP_RESTART, chainid, NULL );
			return 0;
		}
	case eCWMP_tUPDATEOBJ:
		{
			unsigned int InstanceNum;
			struct CWMP_LINKNODE *old_table, *remove_entity;

			/* isolates entity->next to old_table */
			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;

			for (i = 0; i < CT_TYPE_NUM; i++) {
				if (p->cttypemap[i] == 0)
					continue;
				/*
				 * pickes the needed entity from old_table, 
				 * and adds to entity->next
				 */
				remove_entity =
				    remove_SiblingEntity(&old_table, i + 1);
				if (remove_entity) {
					add_SiblingEntity((struct
							   CWMP_LINKNODE **)
							  &entity->next,
							  remove_entity);
				} else {
					InstanceNum = i + 1;
					add_Object(name,
						   (struct CWMP_LINKNODE **)
						   &entity->next,
						   tCT_TypeObject,
						   sizeof
						   (tCT_TypeObject),
						   &InstanceNum);
				}

			}

			if (old_table)
				destroy_ParameterTable((struct CWMP_NODE *)
						       old_table);

			return 0;
		}
	}

	return -1;
}

int getCT_AppEntity(char *name, struct CWMP_LEAF *entity, int *type,
		    void **data)
{
	char *lastname = entity->info->name;
	char buf[256];
	unsigned int instnum = 0, chainid = 0;
	MIB_CE_IP_QOS_APP_T app_entity, *p = &app_entity;

	if ((name == NULL) || (type == NULL) || (data == NULL)
	    || (entity == NULL))
		return -1;

	instnum = getCT_AppInstNum(name);
	if (instnum == 0)
		return ERR_9005;
	if (getCT_AppEntryByInstNum(instnum, p, &chainid) < 0)
		return ERR_9001;

	*type = entity->info->type;
	*data = NULL;

	if (strcmp(lastname, "AppName") == 0) {
		switch (p->appName) {
		case 1:
			strcpy(buf, "VOIP");
			break;
		case 2:
			strcpy(buf, "TR069");
			break;
		default:
			strcpy(buf, "");
			break;
		}
		*data = strdup(buf);
	} else if (strcmp(lastname, "ClassQueue") == 0) {
		*data = uintdup(p->prior);
	} else {
		return ERR_9005;
	}

	return 0;
}

int setCT_AppEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;
	char *buf = data;
	unsigned char vChar = 0;
	unsigned int instnum = 0, chainid = 0;
	MIB_CE_IP_QOS_APP_T app_entity, *p = &app_entity;

	if ((name == NULL) || (data == NULL) || (entity == NULL))
		return -1;
	if (entity->info->type != type)
		return ERR_9006;

	instnum = getCT_AppInstNum(name);
	if (instnum == 0)
		return ERR_9005;
	if (getCT_AppEntryByInstNum(instnum, p, &chainid) < 0)
		return ERR_9001;

	if (strcmp(lastname, "AppName") == 0) {
		unsigned char modeTr69;

		if (buf == NULL)
			return ERR_9007;

		if (!strcmp(buf, "VOIP"))
			p->appName = 1;
		else if (!strcmp(buf, "TR069"))
			p->appName = 2;
		else
			p->appName = 0;

		mib_chain_update(MIB_IP_QOS_APP_TBL, p, chainid);

		apply_add(CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL,
			  sizeof(MIB_CE_IP_QOS_APP_T));
		return 0;
	} else if (strcmp(lastname, "ClassQueue") == 0) {
		unsigned int *i = data;

		if (i == NULL)
			return ERR_9007;

		p->prior = *i;

		mib_chain_update(MIB_IP_QOS_APP_TBL, p, chainid);

		apply_add(CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL,
			  sizeof(MIB_CE_IP_QOS_APP_T));
		return 0;
	} else
		return ERR_9005;

	return 0;
}

int objCT_App(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity = (struct CWMP_NODE *)e;
	int i, num;
	MIB_CE_IP_QOS_APP_T app_entity, *p = &app_entity;

	if (name == NULL || entity == NULL)
		return -1;

	switch (type) {
	case eCWMP_tINITOBJ:
		{
			unsigned int InstanceNum;
			struct CWMP_LINKNODE **c = data;

			if (data == NULL)
				return -1;

			num = mib_chain_total(MIB_IP_QOS_APP_TBL);
			for (i = 0, InstanceNum = 0; i < num; i++) {
				if (!mib_chain_get(MIB_IP_QOS_APP_TBL, i, p))
					continue;
				/* 0: None, 1: VOIP, 2: TR069 */
				if (p->appName != 1 && p->appName != 2)
					continue;

				if (create_Object
				    (c, tCT_AppObject,
				     sizeof(tCT_AppObject), 1,
				     ++InstanceNum) < 0)
					return -1;

				if (p->InstanceNum != InstanceNum) {
					p->InstanceNum = InstanceNum;
					mib_chain_update(MIB_IP_QOS_APP_TBL, p,
							 i);
				}
			}

			return 0;
		}
	case eCWMP_tADDOBJ:
		if (data == NULL)
			return -1;

		return ERR_9001;

	case eCWMP_tDELOBJ:
		if (data == NULL)
			return -1;

		return ERR_9001;

	case eCWMP_tUPDATEOBJ:
		{
			unsigned int InstanceNum;
			struct CWMP_LINKNODE *old_table, *remove_entity;

			clear_objectNum(name);

			/* isolates entity->next to old_table */
			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;

			num = mib_chain_total(MIB_IP_QOS_APP_TBL);
			for (i = 0; i < num; i++) {
				if (!mib_chain_get(MIB_IP_QOS_APP_TBL, i, p))
					continue;
				/* 0: None, 1: VOIP, 2: TR069 */
				if (p->appName != 1 && p->appName != 2)
					continue;
				/*
				 * pickes the needed entity from old_table, 
				 * and adds to entity->next
				 */
				remove_entity =
				    remove_SiblingEntity(&old_table,
							 p->InstanceNum);

				if (remove_entity) {
					add_SiblingEntity((struct CWMP_LINKNODE
							   **)&entity->next,
							  remove_entity);
				} else {
					InstanceNum = p->InstanceNum;
					add_Object(name,
						   (struct CWMP_LINKNODE **)
						   &entity->next,
						   tCT_AppObject,
						   sizeof(tCT_AppObject),
						   &InstanceNum);

					if (p->InstanceNum != InstanceNum) {
						p->InstanceNum = InstanceNum;
						mib_chain_update
						    (MIB_IP_QOS_APP_TBL, p, i);
					}
				}
			}

			if (old_table)
				destroy_ParameterTable((struct CWMP_NODE *)
						       old_table);
			return 0;
		}
	}
	return -1;
}

int getCT_UplinkQoS(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;
	char buff[256] = { 0 };
	unsigned char vChar = 0;
	unsigned int num = 0;

	if ((name == NULL) || (type == NULL) || (data == NULL)
	    || (entity == NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;

	if (strcmp(lastname, "Mode") == 0) {
		unsigned char modeflag;

		mib_get(CTQOS_MODE, buff);
		*data = strdup(buff);
	} else if (strcmp(lastname, "Enable") == 0) {
		unsigned char vUChar;

		mib_get(MIB_QOS_ENABLE_QOS, &vUChar);
		*data = booldup(vUChar);
	} else if (strcmp(lastname, "Bandwidth") == 0) {
		unsigned int totalbandwidth = 0;
		unsigned char totalbandwidthEn = 0;

		mib_get(MIB_TOTAL_BANDWIDTH_LIMIT_EN,
			(void *)&totalbandwidthEn);
		if (totalbandwidthEn == 0)
			*data = uintdup(0);
		else {
			mib_get(MIB_TOTAL_BANDWIDTH, (void *)&totalbandwidth);
			*data = uintdup(totalbandwidth);
		}
	} else if (strcmp(lastname, "Plan") == 0) {
		unsigned char policy = 0;
		mib_get(MIB_QOS_POLICY, &policy);
		if (policy == 0)
			*data = strdup("priority");
		else
			*data = strdup("weight");
	} else if (strcmp(lastname, "EnableForceWeight") == 0) {
		unsigned char vChar;
		mib_get(MIB_QOS_ENABLE_FORCE_WEIGHT, &vChar);
		*data = booldup(vChar);
	} else if (strcmp(lastname, "EnableDSCPMark") == 0) {
		unsigned char vChar;
		mib_get(MIB_QOS_ENABLE_DSCP_MARK, &vChar);
		*data = booldup(vChar);
	} else if (strcmp(lastname, "Enable802-1_P") == 0) {
		unsigned char vChar;
		mib_get(MIB_QOS_ENABLE_1P, &vChar);
		*data = uintdup(vChar);
	} else {
		return ERR_9005;
	}

	return 0;
}

int setCT_UplinkQoS(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;
	char *buf = data;
	unsigned char vChar = 0;

	if ((name == NULL) || (data == NULL) || (entity == NULL))
		return -1;
	if (entity->info->type != type)
		return ERR_9006;

	if (strcmp(lastname, "Mode") == 0) {
		int ret, num;
		size_t buflen;
		unsigned char modeflag = 0;
		unsigned int count = 0, instnum = 0, chainid = 0;
		MIB_CE_IP_QOS_QUEUE_T queue_entity, *p = &queue_entity;

		if (buf == NULL)
			return ERR_9007;

		//Save into MIB
		if (!mib_set(CTQOS_MODE, buf)){
			printf("mib set CTQOS_MODE fail!\n");
			return ERR_9002;
		}

		setMIBforQosMode(buf);	

		apply_add(CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL,
			  0);
		return 0;
	} else if (strcmp(lastname, "Enable") == 0) {
		unsigned char qosEnable;
		int *i = data;

		if (i == NULL)
			return ERR_9007;

		qosEnable = (*i == 0) ? 0 : 1;
		mib_set(MIB_QOS_ENABLE_QOS, &qosEnable);

apply_change_Enable:
		apply_add(CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL, 0);
		return 0;

	} else if (strcmp(lastname, "Bandwidth") == 0) {
		unsigned int totalbandwidth = 0;
		unsigned char totalbandwidthEn = 0;
		unsigned int *i = data;

		if (i == NULL)
			return ERR_9007;
		if (*i == 0) {
			totalbandwidth = 0;
			totalbandwidthEn = 0;
			mib_set(MIB_TOTAL_BANDWIDTH, (void *)&totalbandwidth);
		} else if (*i > 0) {
			totalbandwidth = *i;
			totalbandwidthEn = 1;
			mib_set(MIB_TOTAL_BANDWIDTH, (void *)&totalbandwidth);
		} else
			return ERR_9007;

		mib_set(MIB_TOTAL_BANDWIDTH_LIMIT_EN,
			(void *)&totalbandwidthEn);
		return 0;
	} else if (strcmp(lastname, "Plan") == 0) {
		unsigned char policy = 0;
		if (buf == NULL)
			return ERR_9007;
		if (strlen(buf) == 0)
			return ERR_9007;
		if (!strcmp(buf, "priority"))
			policy = 0;
		else if (!strcmp(buf, "weight"))
			policy = 1;
		else
			return ERR_9007;
		if (!mib_set(MIB_QOS_POLICY, &policy))
			return ERR_9002;
		apply_add(CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL,
			  0);
		return 0;
	} else if (strcmp(lastname, "EnableForceWeight") == 0) {
		int *i = data;

		if (i == NULL)
			return ERR_9007;
		unsigned char vChar = *i;
		if (!mib_set(MIB_QOS_ENABLE_FORCE_WEIGHT, &vChar))
			return ERR_9002;

		apply_add(CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL,
			  0);
		return 0;
	} else if (strcmp(lastname, "EnableDSCPMark") == 0) {
		int *i = data;

		if (i == NULL)
			return ERR_9007;
		unsigned char vChar = *i;
		if (!mib_set(MIB_QOS_ENABLE_DSCP_MARK, &vChar))
			return ERR_9002;

		apply_add(CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL,
			  0);
		return 0;
	} else if (strcmp(lastname, "Enable802-1_P") == 0) {
		unsigned int *i = data;

		if (i == NULL)
			return ERR_9007;
		unsigned char vChar = *i;
		if (!mib_set(MIB_QOS_ENABLE_1P, &vChar))
			return ERR_9002;
		apply_add(CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL,
			  0);
		return 0;
	} else {
		return ERR_9005;
	}

	return 0;
}

/**************************************************************************************/
/* utility functions*/
/**************************************************************************************/
static unsigned int getCT_AppInstNum(char *name)
{
	return getInstNum(name, "App");
}

static unsigned int getCT_QueueInstNum(char *name)
{
	return getInstNum(name, "PriorityQueue");
}

static unsigned int getCT_ClassInstNum(char *name)
{
	return getInstNum(name, "Classification");
}

static unsigned int getCT_TypeInstNum(char *name)
{
	return getInstNum(name, "type");
}

static int getCT_ClassEntryByInstNum(unsigned int instnum, MIB_CE_IP_QOS_T * p,
				     unsigned int *id)
{
	int ret = -1, i, num;

	if ((instnum == 0) || (p == NULL) || (id == NULL))
		return ret;

	num = mib_chain_total(MIB_IP_QOS_TBL);
	for (i = 0; i < num; i++) {
		if (!mib_chain_get(MIB_IP_QOS_TBL, i, p))
			continue;
		if (p->modeTr69 == MODEINTERNET)
			continue;
		if (p->InstanceNum == instnum) {
			*id = i;
			ret = 0;
			break;
		}
	}

	return ret;
}

static int getCT_AppEntryByInstNum(unsigned int instnum,
				      MIB_CE_IP_QOS_APP_T * p, unsigned int *id)
{
	int ret = -1, i, num;

	if ((instnum == 0) || (p == NULL) || (id == NULL))
		return ret;

	num = mib_chain_total(MIB_IP_QOS_APP_TBL);
	for (i = 0; i < num; i++) {
		if (!mib_chain_get(MIB_IP_QOS_APP_TBL, i, p))
			continue;
		if (p->appName != 1 && p->appName != 2)
			continue;
		if (p->InstanceNum == instnum) {
			*id = i;
			ret = 0;
			break;
		}
	}

	return ret;
}

static int getCT_QueueEntryByInstNum(unsigned int instnum,
				     MIB_CE_IP_QOS_QUEUE_T * p,
				     unsigned int *id)
{
	int ret = -1, i, num;

	if ((instnum == 0) || (p == NULL) || (id == NULL))
		return ret;

	num = mib_chain_total(MIB_IP_QOS_QUEUE_TBL);
	for (i = 0; i < num; i++) {
		if (!mib_chain_get(MIB_IP_QOS_QUEUE_TBL, i, p))
			continue;
		if (p->QueueInstNum == instnum) {
			*id = i;
			ret = 0;
			break;
		}
	}

	return ret;
}
#endif

