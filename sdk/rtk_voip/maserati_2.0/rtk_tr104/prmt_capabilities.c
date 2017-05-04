#include "prmt_limit.h"
#include "prmt_capabilities.h"
#include "str_utility.h"

//******* VoiceService.Capabilities.SIP. LEAF *******
struct CWMP_OP tVSCapSIPLeafOP = { getCapabilitiesSipEntity, NULL };
struct CWMP_PRMT tVSCapSIPLeafInfo[] =
{
    /*(name,         type,           flag,       op)*/
	{ "Role",        eCWMP_tSTRING,  CWMP_READ,	 &tVSCapSIPLeafOP},
	{ "Extensions",  eCWMP_tSTRING,	 CWMP_READ,	 &tVSCapSIPLeafOP},
	{ "Transports",  eCWMP_tSTRING,	 CWMP_READ,	 &tVSCapSIPLeafOP},
	{ "URISchemes",  eCWMP_tSTRING,	 CWMP_READ,	 &tVSCapSIPLeafOP},
};
enum eVSCapSIPLeaf
{
	eVSCapRole,
	eVSCapExtensions,
	eVSCapTransports,
	eVSCapURISchemes,
};
struct CWMP_LEAF tVSCapSIPLeaf[] =
{
    { &tVSCapSIPLeafInfo[eVSCapRole]  },
    { &tVSCapSIPLeafInfo[eVSCapExtensions]  },
    { &tVSCapSIPLeafInfo[eVSCapTransports]  },
    { &tVSCapSIPLeafInfo[eVSCapURISchemes]  },
    { NULL }
};

//******* VoiceService.{i}.Capabilities.Codecs.{i}. LEAF *******
struct CWMP_OP tVSCapCodecsLeafOP = { getCapabilitiesCodecsEntity, NULL };
struct CWMP_PRMT tVSCapCodecsLeafInfo[] =
{
    /*(name,                  type,            flag,        op)*/
	{ "EntryID",              eCWMP_tUINT,     CWMP_READ,   &tVSCapCodecsLeafOP},
	{ "Codec",                eCWMP_tSTRING,   CWMP_READ,   &tVSCapCodecsLeafOP},
	{ "BitRate",              eCWMP_tUINT,     CWMP_READ,   &tVSCapCodecsLeafOP},
	{ "PacketizationPeriod",  eCWMP_tSTRING,   CWMP_READ,   &tVSCapCodecsLeafOP},
	{ "SilenceSuppression",   eCWMP_tBOOLEAN,  CWMP_READ,   &tVSCapCodecsLeafOP},
};
enum eVSCapCodecsLeaf
{
	eVSCapEntryID,
	eVSCapCodec,
	eVSCapBitRate,
	eVSCapPacketizationPeriod,
	eVSCapSilenceSuppression,
};
struct CWMP_LEAF tVSCapCodecsLeaf[] =
{
    { &tVSCapCodecsLeafInfo[eVSCapEntryID]  },
    { &tVSCapCodecsLeafInfo[eVSCapCodec]  },
    { &tVSCapCodecsLeafInfo[eVSCapBitRate]  },
    { &tVSCapCodecsLeafInfo[eVSCapPacketizationPeriod]  },
    { &tVSCapCodecsLeafInfo[eVSCapSilenceSuppression]  },
    { NULL }
};

//******* VoiceService.{i}.Capabilities.Codecs.{i} Object *******
struct CWMP_PRMT tVSCapCodecsObjectInfo[] =
{
    /*(name,     type,           flag,                    op)*/
    { "0",       eCWMP_tOBJECT,  CWMP_READ|CWMP_LNKLIST,  NULL},
};
enum eVSCapCodecsObject
{
	eVSCapCodecs0
};
struct CWMP_LINKNODE tVSCapCodecsObject[] =
{
    /*info,                                   leaf,              next,  sibling,  instnum)*/
    {&tVSCapCodecsObjectInfo[eVSCapCodecs0],  tVSCapCodecsLeaf,  NULL,  NULL,     0}
};



//******* VoiceService.{i}.Capabilities. LEAF *******
struct CWMP_OP tVSCapabilitiesLeafOP = { getCapabilitiesEntity, NULL };
struct CWMP_PRMT tVSCapabilitiesLeafInfo[] =
{
    /*(name,                  type,           flag,       op)*/
    { "MaxProfileCount",      eCWMP_tUINT,    CWMP_READ,  &tVSCapabilitiesLeafOP},
    { "MaxSessionCount",      eCWMP_tUINT,    CWMP_READ,  &tVSCapabilitiesLeafOP},
    { "SignalingProtocols",   eCWMP_tSTRING,  CWMP_READ,  &tVSCapabilitiesLeafOP},
    { "FaxT38",               eCWMP_tBOOLEAN, CWMP_READ,  &tVSCapabilitiesLeafOP},
    { "FaxPassThrough",       eCWMP_tBOOLEAN, CWMP_READ,  &tVSCapabilitiesLeafOP},
    { "ModemPassThrough",     eCWMP_tBOOLEAN, CWMP_READ,  &tVSCapabilitiesLeafOP},
    { "ToneGeneration",       eCWMP_tBOOLEAN, CWMP_READ,  &tVSCapabilitiesLeafOP},
    { "RingGeneration",       eCWMP_tBOOLEAN, CWMP_READ,  &tVSCapabilitiesLeafOP},
    { "VoicePortTests",       eCWMP_tBOOLEAN, CWMP_READ,  &tVSCapabilitiesLeafOP},
    { "DigitMap",             eCWMP_tBOOLEAN, CWMP_READ,  &tVSCapabilitiesLeafOP},
    { "X_CT-COM_Heartbeat",   eCWMP_tBOOLEAN, CWMP_READ,  &tVSCapabilitiesLeafOP}
};
enum eVSCapabilitiesLeaf
{
	eVSCapMaxProfileCount,
	eVSCapMaxSessionCount,
	eVSCapSignalingProtocol,
	eVSCapFaxT38,
	eVSCapFaxPassThrough,
	eVSCapModemPassThrough,
	eVSCapToneGeneration,
	eVSCapRingGeneration,
	eVSCapVoicePortTests,
	eVSDigitMap,
	eVSXCT_Heartbeat
};
struct CWMP_LEAF tVSCapabilitiesLeaf[] =
{
    { &tVSCapabilitiesLeafInfo[eVSCapMaxProfileCount]  },
    { &tVSCapabilitiesLeafInfo[eVSCapMaxSessionCount]  },
    { &tVSCapabilitiesLeafInfo[eVSCapSignalingProtocol]  },
    { &tVSCapabilitiesLeafInfo[eVSCapFaxT38]  },
    { &tVSCapabilitiesLeafInfo[eVSCapFaxPassThrough]  },
    { &tVSCapabilitiesLeafInfo[eVSCapModemPassThrough]  },
    { &tVSCapabilitiesLeafInfo[eVSCapToneGeneration]  },
    { &tVSCapabilitiesLeafInfo[eVSCapRingGeneration]  },
    { &tVSCapabilitiesLeafInfo[eVSCapVoicePortTests]  },
    { &tVSCapabilitiesLeafInfo[eVSDigitMap]  },
    { &tVSCapabilitiesLeafInfo[eVSXCT_Heartbeat]  },
    { NULL	}
};


//******* VoiceService.{i}.Capabilities.  *******
struct CWMP_OP tVSCapabilitiesCodecsOP = { NULL, objCapabilitiesCodecs };
struct CWMP_PRMT tVSCapabilitiesObjectInfo[] =
{
    /*(name,    type,           flag,		op)*/
    {"SIP",     eCWMP_tOBJECT,  CWMP_READ,  NULL},
    {"Codecs",  eCWMP_tOBJECT,  CWMP_READ,	&tVSCapabilitiesCodecsOP}
};
enum eVSCapabilitiesObject
{
    eVSCapSIP,
    eVSCapCodecs
};
struct CWMP_NODE tVSCapabilitiesObject[] =
{
    /*info,  			                        leaf,           next)*/
    {&tVSCapabilitiesObjectInfo[eVSCapSIP],     tVSCapSIPLeaf,  NULL},
    {&tVSCapabilitiesObjectInfo[eVSCapCodecs],  NULL,           NULL},
    {NULL,                                      NULL,           NULL}
};

/* It must be listed as same as that of voip_flash.h */
const lstCodecs_t lstCodecs[] = {
	{ "G.711MuLaw",	80 * 100 * 8,		"10,20,30",	1 },
	{ "G.711ALaw",	80 * 100 * 8,		"10,20,30",	1 },
#ifdef  CONFIG_RTK_VOIP_G729AB
	{ "G.729",		10 * 100 * 8,		"10,20,30",	1 },
#endif /*CONFIG_RTK_VOIP_G729AB*/
#ifdef CONFIG_RTK_VOIP_G7231
	{ "G.723.1",	5300,				"30,60",	1 },
	{ "G.723.1",	6300,				"30,60",	1 },
#endif /*CONFIG_RTK_VOIP_G7231*/
#ifdef  CONFIG_RTK_VOIP_G726
	{ "G.726",		16 * 1000,			"10,20,30",	1 },
	{ "G.726",		24 * 1000,			"10,20,30",	1 },
	{ "G.726",		32 * 1000,			"10,20,30",	1 },
	{ "G.726",		40 * 1000,			"10,20,30",	1 },
#endif /*CONFIG_RTK_VOIP_G726*/
#ifdef CONFIG_RTK_VOIP_GSMFR
	{ "GSM-FR",		33 * 100 * 8 / 2,	"20,40",	0 },
#endif /*CONFIG_RTK_VOIP_GSMFR*/
#ifdef CONFIG_RTK_VOIP_ILBC
	{ "iLBC",	13330,		"30",		0 },
	{ "iLBC",	15200,		"20",		0 },
#endif /*CONFIG_RTK_VOIP_ILBC*/
#ifdef CONFIG_RTK_VOIP_G722
	{ "G.722",	64000,		"10,20,30",		1 },
#endif /*CONFIG_RTK_VOIP_G722*/
};

#define NUM_OF_LIST_CODEC		( sizeof( lstCodecs ) / sizeof( lstCodecs[ 0 ] ) )

CT_ASSERT( NUM_OF_LIST_CODEC == MAX_CODEC_LIST );

int getCapabilitiesCodecsEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	/* VoiceService.{i}.Capabilities.codecs.{i} */
    char *pszLastname = entity->info->name;
	unsigned int nObjectNum, nObjectIdx;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	if( !GetOneBasedInstanceNumber_Capabilities_Codecs( name, &nObjectNum ) )
		return ERR_9007;
	
	nObjectIdx = nObjectNum - 1;	/* to zero-based */
	
	if( nObjectIdx >= NUM_OF_LIST_CODEC )
		return ERR_9007;
			
	*type = entity->info->type;
    *data = NULL;

    CWMPDBG( 3,  ( stderr, "<%s:%d> get %s value \n", __FUNCTION__, __LINE__, name) );

	if( strcmp( pszLastname, "EntryID" ) == 0 ) {
		*data = uintdup( nObjectNum );
	} else if( strcmp( pszLastname, "Codec" ) == 0 ) {
		*data = strdup( lstCodecs[ nObjectIdx ].pszCodec );
	} else if( strcmp( pszLastname, "BitRate" ) == 0 ) {
		*data = uintdup( lstCodecs[ nObjectIdx ].nBitRate );
	} else if( strcmp( pszLastname, "PacketizationPeriod" ) == 0 ) {
		*data = strdup( lstCodecs[ nObjectIdx ].pszPacketizationPeriod );
	} else if( strcmp( pszLastname, "SilenceSuppression" ) == 0 ) {
		*data = booldup( lstCodecs[ nObjectIdx ].bSilenceSupression );
	} else {
		*data = NULL;
		return ERR_9005;
	}	

	return 0;
}

int objCapabilitiesCodecs(char *name, struct CWMP_LEAF *e, int type, void *data)
{
    struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
    
	switch( type ) {	
	    case eCWMP_tINITOBJ:
		{
			int num=0,i;
			struct CWMP_LINKNODE **ptable = (struct CWMP_LINKNODE **)data;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			num = MAX_CODEC_LIST;
			for( i=1; i<=num; i++ )
			{
				if( create_Object( ptable, tVSCapCodecsObject, sizeof(tVSCapCodecsObject), num, 1) < 0 )
					return -1;
			}
	        add_objectNum( name, num );
			return 0;
		}
	    	break;
		case eCWMP_tADDOBJ:
		case eCWMP_tDELOBJ:
	         {
                CWMPDBG( 0, ( stderr, "<%s:%d> type %d unsupported now\n", __FUNCTION__, __LINE__, type) );
	            return 0;
	         }
	    	break;
	    case eCWMP_tUPDATEOBJ:
	         {
	            int num=0,i;
	            struct CWMP_LINKNODE *old_table;
	            
	            num = MAX_CODEC_LIST;
	            old_table = (struct CWMP_LINKNODE*)entity->next;
	            entity->next= NULL;
	            
	            for( i=1; i<=num;i++ ) {
	                struct CWMP_LINKNODE *remove_entity=NULL;

	                remove_entity = remove_SiblingEntity( &old_table, i );
	                if( remove_entity!=NULL )
	                {
	                	add_SiblingEntity( (struct CWMP_LINKNODE**)&entity->next, remove_entity );
	                }else{ 
	                	unsigned int MaxInstNum=i;
	                    add_Object( name, (struct CWMP_LINKNODE **)&entity->next,
	                        tVSCapCodecsObject, sizeof(tVSCapCodecsObject), &MaxInstNum );
	                }
	           }
	           if( old_table )  destroy_ParameterTable( (struct CWMP_NODE *)old_table );           
	           return 0;
	        }
		   	break;
        default:
            CWMPDBG( 0, ( stderr, "<%s:%d>Error: unknown type %d \n", __FUNCTION__, __LINE__, type) );
            break;
		}
	return -1;
}

int getCapabilitiesSipEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	/* VoiceService.{i}.Capabilities.SIP. */
	const char *pszLastname = entity->info->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
    *data = NULL;

    CWMPDBG( 3,  ( stderr, "<%s:%d> get %s value \n", __FUNCTION__, __LINE__, name) );
    
	if( strcmp( pszLastname, "Role" ) == 0 ) {
		*data = strdup( SIP_ROLE );
	} else if( strcmp( pszLastname, "Extensions" ) == 0 ) {
		*data = strdup( "" );	//FIXME, supported methods, ex: "INVITE, REFER, INFO,"
	} else if( strcmp( pszLastname, "Transports" ) == 0 ) {
		*data = strdup( SIP_TRANSPORTS );
	} else if( strcmp( pszLastname, "URISchemes" ) == 0 ) {
		*data = strdup(SIP_URI_SCHEMES);
	} else {
		*data = NULL;
		return ERR_9005;
	}

	return 0;
}

int getCapabilitiesEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	/* VoiceService.{i}.Capabilities */
	const char *pszLastname = entity->info->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
    *data = NULL;

	CWMPDBG( 3,  ( stderr, "<%s:%d>Debug: get %s value \n", __FUNCTION__, __LINE__, name) );
    
	if( strcmp( pszLastname, "MaxProfileCount" ) == 0 ) {
		*data = uintdup( MAX_PROFILE_COUNT );
	} else if( strcmp( pszLastname, "MaxSessionCount" ) == 0 ) {
		*data = uintdup( MAX_SESSION_COUNT );
	} else if( strcmp( pszLastname, "SignalingProtocols" ) == 0 ) {
		*data = strdup( SIGNALING_PROTOCOLS );	/* "SIP" */
	} else if( strcmp( pszLastname, "FaxT38" ) == 0 ) {
#ifdef CONFIG_RTK_VOIP_T38
		*data = booldup(TRUE);
	} else if( strcmp( pszLastname, "FaxPassThrough" ) == 0 ) {
		*data = booldup(TRUE);
	} else if( strcmp( pszLastname, "ModemPassThrough" ) == 0 ) {
		*data = booldup(TRUE);
#else
		*data = booldup(FALSE);
	} else if( strcmp( pszLastname, "FaxPassThrough" ) == 0 ) {
		*data = booldup(FALSE);
	} else if( strcmp( pszLastname, "ModemPassThrough" ) == 0 ) {
		*data = booldup(FALSE);
#endif
	} else if( strcmp( pszLastname, "ToneGeneration" ) == 0 ) {
		*data = booldup( SUPPORT_TONE_GENERATION_OBJECT );	/* Off -> No this object */
	} else if( strcmp( pszLastname, "RingGeneration" ) == 0 ) {
		*data = booldup( SUPPORT_RING_GENERATION_OBJECT );	/* Off -> No this object */
	} else if( strcmp( pszLastname, "VoicePortTests" ) == 0 ) {
		*data = booldup( SUPPORT_VOICE_LINE_TESTS_OBJECT );	/* Off -> No this object */
	} else if( strcmp( pszLastname, "DigitMap" ) == 0 ) {
		*data = booldup( SUPPORT_DIGIT_MAP_OBJECT );	//support digit_map  (dialplan)
	} else if( strcmp( pszLastname, "X_CT-COM_Heartbeat" ) == 0 ) {
		*data = booldup( SUPPORT_X_CT_COM_Heartbeat );	//support OPTION ping ,
	} else {
		*data = NULL;
		return ERR_9005;
	}

	return 0;
}

#if 0
static int getCapabilitiesCodecsEntity(char *name, struct sCWMP_ENTITY *entity, int *type, void **data);
static int objCapabilitiesCodecs(char *name, struct sCWMP_ENTITY *entity, int type, void *data);
static int getCapabilitiesSipEntity(char *name, struct sCWMP_ENTITY *entity, int *type, void **data);
static int getCapabilitiesEntity(char *name, struct sCWMP_ENTITY *entity, int *type, void **data);

struct sCWMP_ENTITY tCapabilitiesCodecsEntity[] = {
/*	{ name,					type,			flag,		accesslist,	getvalue,						setvalue,						next_table,	sibling } */
	{ "EntryId",			eCWMP_tUINT,	CWMP_READ,	NULL,		getCapabilitiesCodecsEntity, 	NULL,							NULL,		NULL },
	{ "Codec",				eCWMP_tSTRING,	CWMP_READ,	NULL,		getCapabilitiesCodecsEntity, 	NULL,							NULL,		NULL },
	{ "BitRate",			eCWMP_tUINT,	CWMP_READ,	NULL,		getCapabilitiesCodecsEntity, 	NULL,							NULL,		NULL },
	{ "PacketizationPeriod",eCWMP_tSTRING,	CWMP_READ,	NULL,		getCapabilitiesCodecsEntity, 	NULL,							NULL,		NULL },
	{ "SilenceSuppression",	eCWMP_tBOOLEAN,	CWMP_READ,	NULL,		getCapabilitiesCodecsEntity, 	NULL,							NULL,		NULL },
	{ "",					eCWMP_tNONE,	0,			NULL,		NULL,							NULL,							NULL,		NULL },
};

struct sCWMP_ENTITY tCapabilitiesCodecs[] = {
/*	{ name,		type,			flag,								accesslist,	getvalue,	setvalue,	next_table,					sibling } */
	{ "0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL,		NULL,		NULL,		tCapabilitiesCodecsEntity,	NULL },
};

struct sCWMP_ENTITY tCapabilitiesSipEntity[] = {
/*	{ name,					type,			flag,		accesslist,	getvalue,					setvalue,					next_table,	sibling } */
	{ "Role",				eCWMP_tSTRING,	CWMP_READ,	NULL,		getCapabilitiesSipEntity, 	NULL,						NULL,		NULL },
	{ "Extensions",			eCWMP_tSTRING,	CWMP_READ,	NULL,		getCapabilitiesSipEntity, 	NULL,						NULL,		NULL },
	{ "Transports",			eCWMP_tSTRING,	CWMP_READ,	NULL,		getCapabilitiesSipEntity, 	NULL,						NULL,		NULL },
	{ "URISchemes",			eCWMP_tSTRING,	CWMP_READ,	NULL,		getCapabilitiesSipEntity, 	NULL,						NULL,		NULL },
	{ "",					eCWMP_tNONE,	0,			NULL,		NULL,						NULL,						NULL,		NULL },
};

struct sCWMP_ENTITY tCapabilitiesEntity[] = {
/*	{ name,					type,			flag,		accesslist,	getvalue,				setvalue,				next_table,				sibling } */
	{ "MaxProfileCount",	eCWMP_tUINT,	CWMP_READ,	NULL,		getCapabilitiesEntity, 	NULL,					NULL,					NULL },
	{ "MaxSessionCount",	eCWMP_tUINT,	CWMP_READ,	NULL,		getCapabilitiesEntity, 	NULL,					NULL,					NULL },
	{ "SignalingProtocols",eCWMP_tSTRING,	CWMP_READ,	NULL,		getCapabilitiesEntity, 	NULL,					NULL,					NULL },
	{ "FaxT38",				eCWMP_tBOOLEAN,	CWMP_READ,	NULL,		getCapabilitiesEntity, 	NULL,					NULL,					NULL },
	{ "FaxPassThrough",		eCWMP_tBOOLEAN,	CWMP_READ,	NULL,		getCapabilitiesEntity, 	NULL,					NULL,					NULL },
	{ "ModemPassThrough",	eCWMP_tBOOLEAN,	CWMP_READ,	NULL,		getCapabilitiesEntity, 	NULL,					NULL,					NULL },
	{ "ToneGeneration",		eCWMP_tBOOLEAN,	CWMP_READ,	NULL,		getCapabilitiesEntity, 	NULL,					NULL,					NULL },
	{ "RingGeneration",		eCWMP_tBOOLEAN,	CWMP_READ,	NULL,		getCapabilitiesEntity, 	NULL,					NULL,					NULL },
	{ "VoicePortTests",		eCWMP_tBOOLEAN,	CWMP_READ,	NULL,		getCapabilitiesEntity, 	NULL,					NULL,					NULL },
	{ "SIP",				eCWMP_tOBJECT,	CWMP_READ,	NULL,		NULL, 					NULL,					tCapabilitiesSipEntity,	NULL },
	{ "Codecs",				eCWMP_tOBJECT,	CWMP_READ,	NULL,		NULL, 					objCapabilitiesCodecs,	NULL,					NULL },
	{ "",					eCWMP_tNONE,	0,			NULL,		NULL,					NULL,					NULL,					NULL },
};

static int getCapabilitiesCodecsEntity(char *name, struct sCWMP_ENTITY *entity, int *type, void **data)
{
	const char *pszLastname = entity ->name;
	unsigned int nObjectNum, nObjectIdx;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	if( !GetOneBasedInstanceNumber_Capabilities_Codecs( name, &nObjectNum ) )
		return ERR_9007;
	
	nObjectIdx = nObjectNum - 1;	/* to zero-based */
	
	if( nObjectIdx >= NUM_OF_LIST_CODEC )
		return ERR_9007;
			
	*type = entity ->type;

	if( strcmp( pszLastname, "EntryId" ) == 0 ) {
		*data = uintdup( nObjectNum );
	} else if( strcmp( pszLastname, "Codec" ) == 0 ) {
		*data = strdup( lstCodecs[ nObjectIdx ].pszCodec );
	} else if( strcmp( pszLastname, "BitRate" ) == 0 ) {
		*data = uintdup( lstCodecs[ nObjectIdx ].nBitRate );
	} else if( strcmp( pszLastname, "PacketizationPeriod" ) == 0 ) {
		*data = strdup( lstCodecs[ nObjectIdx ].pszPacketizationPeriod );
	} else if( strcmp( pszLastname, "SilenceSuppression" ) == 0 ) {
		*data = booldup( lstCodecs[ nObjectIdx ].bSilenceSupression );
	} else {
		*data = NULL;
		return ERR_9005;
	}	

	return 0;
}


static int objCapabilitiesCodecs(char *name, struct sCWMP_ENTITY *entity, int type, void *data)
{
	switch( type ) {
	case eCWMP_tINITOBJ:
	{
		struct sCWMP_ENTITY **c = (struct sCWMP_ENTITY **)data;

		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		if( create_Object( c, tCodecsObject, sizeof(tCodecsObject), NUM_OF_LIST_CODEC, 1 ) < 0 )
			return -1;
			
		add_objectNum( name, 1 );
		return 0;
	}
		break;
		
	case eCWMP_tADDOBJ:
	case eCWMP_tDELOBJ:
	case eCWMP_tUPDATEOBJ:
		break;
	}
	
	return -1;
}

static int getCapabilitiesSipEntity(char *name, struct sCWMP_ENTITY *entity, int *type, void **data)
{
	const char *pszLastname = entity ->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity ->type;

	if( strcmp( pszLastname, "Role" ) == 0 ) {
		*data = strdup( SIP_ROLE );
	} else if( strcmp( pszLastname, "Extensions" ) == 0 ) {
		*data = strdup( "" );
	} else if( strcmp( pszLastname, "Transports" ) == 0 ) {
		*data = strdup( SIP_TRANSPORTS );
	} else if( strcmp( pszLastname, "URISchemes" ) == 0 ) {
		*data = strdup(SIP_URI_SCHEMES);
	} else {
		*data = NULL;
		return ERR_9005;
	}

	return 0;
}

static int getCapabilitiesEntity(char *name, struct sCWMP_ENTITY *entity, int *type, void **data)
{
	const char *pszLastname = entity ->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity ->type;

	if( strcmp( pszLastname, "MaxProfileCount" ) == 0 ) {
		*data = uintdup( MAX_PROFILE_COUNT );
	} else if( strcmp( pszLastname, "MaxSessionCount" ) == 0 ) {
		*data = uintdup( MAX_SESSION_COUNT );
	} else if( strcmp( pszLastname, "SignalingProtocols" ) == 0 ) {
		*data = strdup( SIGNALING_PROTOCOLS );	/* "SIP" */
	} else if( strcmp( pszLastname, "FaxT38" ) == 0 ) {
#ifdef CONFIG_RTK_VOIP_T38
		*data = booldup(TRUE);
	} else if( strcmp( pszLastname, "FaxPassThrough" ) == 0 ) {
		*data = booldup(TRUE);
	} else if( strcmp( pszLastname, "ModemPassThrough" ) == 0 ) {
		*data = booldup(TRUE);
#else
		*data = booldup(FALSE);
	} else if( strcmp( pszLastname, "FaxPassThrough" ) == 0 ) {
		*data = booldup(FALSE);
	} else if( strcmp( pszLastname, "ModemPassThrough" ) == 0 ) {
		*data = booldup(FALSE);
#endif
	} else if( strcmp( pszLastname, "ToneGeneration" ) == 0 ) {
		*data = booldup( SUPPORT_TONE_GENERATION_OBJECT );	/* Off -> No this object */
	} else if( strcmp( pszLastname, "RingGeneration" ) == 0 ) {
		*data = booldup( SUPPORT_RING_GENERATION_OBJECT );	/* Off -> No this object */
	} else if( strcmp( pszLastname, "VoicePortTests" ) == 0 ) {
		*data = booldup( SUPPORT_VOICE_LINE_TESTS_OBJECT );	/* Off -> No this object */
	} else {
		*data = NULL;
		return ERR_9005;
	}

	return 0;
}
#endif
