#include "voip_manager.h"
#include "prmt_limit.h"
#include "prmt_voice_profile_line.h"
#include "prmt_line_codec_list.h"
#include "prmt_capabilities.h"	/* for lstCodecs */
#include "mib_def.h"
#include "mib_tr104.h"
#include "str_mib.h"
#include "cwmp_main_tr104.h"
#include "str_utility.h"
#include "cwmpevt.h"

//******* VoiceService.{i}.VoiceProfile.{i}.Line.{i}.SIP. EntityLEAF*******
struct CWMP_OP tVPLineSIPEntityLeafOP = { getLineSipEntity, setLineSipEntity};

struct CWMP_PRMT tVPLineSIPEntityLeafInfo[] =
{
    /*(name,           type,           flag,                  op)*/
	{ "AuthUserName",  eCWMP_tSTRING,  CWMP_READ|CWMP_WRITE,  &tVPLineSIPEntityLeafOP},
	{ "AuthPassword",  eCWMP_tSTRING,  CWMP_READ|CWMP_WRITE,  &tVPLineSIPEntityLeafOP},
	{ "URI",           eCWMP_tSTRING,  CWMP_READ|CWMP_WRITE,  &tVPLineSIPEntityLeafOP},
	{ "X_Standby-AuthUserName",  eCWMP_tSTRING,  CWMP_READ|CWMP_WRITE,  &tVPLineSIPEntityLeafOP},
	{ "X_Standby-AuthPassword",  eCWMP_tSTRING,  CWMP_READ|CWMP_WRITE,  &tVPLineSIPEntityLeafOP},
	{ "X_Standby-URI",           eCWMP_tSTRING,  CWMP_READ|CWMP_WRITE,  &tVPLineSIPEntityLeafOP}	
};

enum eVPLineSIPEntityLeaf
{
    eSIPAuthUserName,
    eSIPAuthPassword,
    eSIPURI,
    eSIP_XCT_AuthUserName,
    eSIP_XCT_AuthPassword,
    eSIP_XCT_URI    
};

struct CWMP_LEAF tVPLineSIPEntityLeaf[] =
{
    { &tVPLineSIPEntityLeafInfo[eSIPAuthUserName]  },
    { &tVPLineSIPEntityLeafInfo[eSIPAuthPassword]  },
    { &tVPLineSIPEntityLeafInfo[eSIPURI]  },
    { &tVPLineSIPEntityLeafInfo[eSIP_XCT_AuthUserName]  },
    { &tVPLineSIPEntityLeafInfo[eSIP_XCT_AuthPassword]  },
    { &tVPLineSIPEntityLeafInfo[eSIP_XCT_URI]  },    
    { NULL }
};

//******* VoiceService.{i}.VoiceProfile.{i}.Line.{i}.Stats. EntityLEAF*******
struct CWMP_OP tVPLineStatsEntityLeafOP = { getLineStatsEntity, setLineStatsEntity};

struct CWMP_PRMT tVPLineStatsEntityLeafInfo[] =
{
    /*(name,                     type,         flag,                  op)*/
    { "PacketsSent",             eCWMP_tUINT,  CWMP_READ,             &tVPLineStatsEntityLeafOP},
    { "PacketsReceived",         eCWMP_tUINT,  CWMP_READ,             &tVPLineStatsEntityLeafOP},
    { "BytesSent",               eCWMP_tUINT,  CWMP_READ,             &tVPLineStatsEntityLeafOP},
    { "BytesReceived",           eCWMP_tUINT,  CWMP_READ,             &tVPLineStatsEntityLeafOP},
    { "PacketsLost",             eCWMP_tUINT,  CWMP_READ,             &tVPLineStatsEntityLeafOP},
    { "IncomingCallsReceived",   eCWMP_tUINT,  CWMP_READ,             &tVPLineStatsEntityLeafOP},
    { "IncomingCallsAnswered",   eCWMP_tUINT,  CWMP_READ,             &tVPLineStatsEntityLeafOP},
    { "IncomingCallsConnected",  eCWMP_tUINT,  CWMP_READ,             &tVPLineStatsEntityLeafOP},
    { "IncomingCallsFailed",     eCWMP_tUINT,  CWMP_READ,             &tVPLineStatsEntityLeafOP},
    { "OutgoingCallsAttempted",  eCWMP_tUINT,  CWMP_READ,             &tVPLineStatsEntityLeafOP},
    { "OutgoingCallsAnswered",   eCWMP_tUINT,  CWMP_READ,             &tVPLineStatsEntityLeafOP},
    { "OutgoingCallsConnected",  eCWMP_tUINT,  CWMP_READ,             &tVPLineStatsEntityLeafOP},
    { "OutgoingCallsFailed",     eCWMP_tUINT,  CWMP_READ,             &tVPLineStatsEntityLeafOP},
    { "ResetStatistics",         eCWMP_tUINT,  CWMP_READ|CWMP_WRITE,  &tVPLineStatsEntityLeafOP},
    { "TotalCallTime",           eCWMP_tUINT,  CWMP_READ,             &tVPLineStatsEntityLeafOP},
};

enum eVPLineStatsEntityLeaf
{
    eStatsPacketsSent,
    eStatsPacketsReceived,
    eStatsBytesSent,
    eStatsBytesReceived,
    eStatsPacketsLost,
    eStatsIncomingCallsReceived,
    eStatsIncomingCallsAnswered,
    eStatsIncomingCallsConnected,
    eStatsIncomingCallsFailed,
    eStatsOutgoingCallsAttempted,
    eStatsOutgoingCallsAnswered,
    eStatsOutgoingCallsConnected,
    eStatsOutgoingCallsFailed,
	eResetStatistics,
	eTotalCallTime
};

struct CWMP_LEAF tVPLineStatsEntityLeaf[] =
{
    { &tVPLineStatsEntityLeafInfo[eStatsPacketsSent]  },
    { &tVPLineStatsEntityLeafInfo[eStatsPacketsReceived]  },
    { &tVPLineStatsEntityLeafInfo[eStatsBytesSent]  },
    { &tVPLineStatsEntityLeafInfo[eStatsBytesReceived]  },
    { &tVPLineStatsEntityLeafInfo[eStatsPacketsLost]  },
    { &tVPLineStatsEntityLeafInfo[eStatsIncomingCallsReceived]  },
    { &tVPLineStatsEntityLeafInfo[eStatsIncomingCallsAnswered]  },
    { &tVPLineStatsEntityLeafInfo[eStatsIncomingCallsConnected]  },
    { &tVPLineStatsEntityLeafInfo[eStatsIncomingCallsFailed]  },
    { &tVPLineStatsEntityLeafInfo[eStatsOutgoingCallsAttempted]  },
    { &tVPLineStatsEntityLeafInfo[eStatsOutgoingCallsAnswered]  },
    { &tVPLineStatsEntityLeafInfo[eStatsOutgoingCallsConnected]  },
    { &tVPLineStatsEntityLeafInfo[eStatsOutgoingCallsFailed]  },
	{ &tVPLineStatsEntityLeafInfo[eResetStatistics]  },
    { &tVPLineStatsEntityLeafInfo[eTotalCallTime]  },
    { NULL }
};

//******* VoiceService.{i}.VoiceProfile.{i}.Line.{i}.Codec. EntityObject*******
struct CWMP_OP tVPLineCodecOP = { NULL, objLineCodecList};

struct CWMP_PRMT tVPLineCodecEntityObjectInfo[] =
{
    /*(name,    type,           flag,       op)*/
	{ "List",   eCWMP_tOBJECT,  CWMP_READ,  &tVPLineCodecOP}
};
enum eVPLineCodecEntityObject
{
	eCodecList
};
struct CWMP_NODE tVPLineCodecEntityObject[] =
{
    /*info,                                      leaf,  next)*/
    {&tVPLineCodecEntityObjectInfo[eCodecList],  NULL,  NULL},
    {NULL,                                       NULL,  NULL}
};


//******* VoiceService.{i}.VoiceProfile.{i}.Line.{i}.Codec. EntityLEAF *******

struct CWMP_OP tVPLineCodecEntityLeafOP = { getLineCodecEntity, setLineCodecEntity};

struct CWMP_PRMT tVPLineCodecEntityLeafInfo[] =
{
    /*(name,                     type,         flag,                  op)*/
    { "TransmitCodec",        eCWMP_tSTRING,  CWMP_READ,             &tVPLineCodecEntityLeafOP},
    { "ReceiveCodec",         eCWMP_tSTRING,  CWMP_READ,             &tVPLineCodecEntityLeafOP},
  
};

enum eVPLineCodecEntityLeaf
{
    eCodecTransmitCodec,
    eCodecReceiveCodec,
 
};

struct CWMP_LEAF tVPLineCodecEntityLeaf[] =
{
    { &tVPLineCodecEntityLeafInfo[eCodecTransmitCodec]  },
    { &tVPLineCodecEntityLeafInfo[eCodecReceiveCodec]  },
     { NULL }
};



//******* VoiceService.{i}.VoiceProfile.{i}.Line.{i}.VoiceProcessing EntityLEAF*******
struct CWMP_OP tVPLineVoiceProcessingEntityLeafOP = 
  { getLineVoiceProcessingEntity, setLineVoiceProcessingEntity};

struct CWMP_PRMT tVPLineVoiceProcessingEntityLeafInfo[] =
{
    /*(name,                    type,           flag,                  op)*/
	{ "TransmitGain",           eCWMP_tINT,     CWMP_READ|CWMP_WRITE,  &tVPLineVoiceProcessingEntityLeafOP},
	{ "ReceiveGain",            eCWMP_tINT,     CWMP_READ|CWMP_WRITE,  &tVPLineVoiceProcessingEntityLeafOP},
	{ "EchoCancellationEnable", eCWMP_tBOOLEAN, CWMP_READ|CWMP_WRITE,  &tVPLineVoiceProcessingEntityLeafOP},
	{ "EchoCancellationInUse",  eCWMP_tBOOLEAN, CWMP_READ,             &tVPLineVoiceProcessingEntityLeafOP},
    { "EchoCancellationTail",   eCWMP_tUINT,    CWMP_READ,             &tVPLineVoiceProcessingEntityLeafOP}
};

enum eVPLineVoiceProcessingEntityLeaf
{
    eVoiceProcessingTransmitGain,
    eVoiceProcessingReceiveGain,
    eVoiceProcessingEchoCancellationEnable,
    eVoiceProcessingEchoCancellationInUse,
    eVoiceProcessingEchoCancellationTail
};

struct CWMP_LEAF tVPLineVoiceProcessingEntityLeaf[] =
{
    { &tVPLineVoiceProcessingEntityLeafInfo[eVoiceProcessingTransmitGain]  },
    { &tVPLineVoiceProcessingEntityLeafInfo[eVoiceProcessingReceiveGain]  },
    { &tVPLineVoiceProcessingEntityLeafInfo[eVoiceProcessingEchoCancellationEnable]  },
    { &tVPLineVoiceProcessingEntityLeafInfo[eVoiceProcessingEchoCancellationInUse]  },
    { &tVPLineVoiceProcessingEntityLeafInfo[eVoiceProcessingEchoCancellationTail]  },
    { NULL }
};

//******* VoiceService.{i}.VoiceProfile.{i}.Line.{i}.X_IMS EntityLEAF*******
struct CWMP_OP tVPLineXCT_IMSEntityLeafOP = { getLineXCT_IMSEntity, setLineXCT_IMSEntity};

struct CWMP_PRMT tVPLineXCT_IMSEntityLeafInfo[] =
{
    /*(name,                    type,        flag,                 op)*/
	{ "dial-tone-pattern",    eCWMP_tUINT,   CWMP_READ,            &tVPLineXCT_IMSEntityLeafOP},
	{ "mcid-service",         eCWMP_tUINT,   CWMP_READ,            &tVPLineXCT_IMSEntityLeafOP},
	{ "no-dialing-behaviour", eCWMP_tUINT,   CWMP_READ,            &tVPLineXCT_IMSEntityLeafOP},
	{ "hold-service",         eCWMP_tUINT,   CWMP_READ,            &tVPLineXCT_IMSEntityLeafOP},
	{ "three-party-service",  eCWMP_tUINT,   CWMP_READ,            &tVPLineXCT_IMSEntityLeafOP},
	{ "conference-uri",       eCWMP_tSTRING, CWMP_READ|CWMP_WRITE, &tVPLineXCT_IMSEntityLeafOP},
	{ "hotline-uri",          eCWMP_tSTRING, CWMP_READ|CWMP_WRITE, &tVPLineXCT_IMSEntityLeafOP},
	{ "cw-service",           eCWMP_tUINT,   CWMP_READ,            &tVPLineXCT_IMSEntityLeafOP}
};

enum eVPLineXCT_IMSEntityLeaf
{
    eXCT_IMSdialTonePattern,
    eXCT_IMSmcidService,
    eXCT_IMSnoDialingBehaviour,
    eXCT_IMSholdService,
    eXCT_IMSthreePartyService,
    eXCT_IMSconferenceUri,
    eXCT_IMShotlineUri,
    eXCT_IMScwService
};

struct CWMP_LEAF tVPLineXCT_IMSEntityLeaf[] =
{
    { &tVPLineXCT_IMSEntityLeafInfo[eXCT_IMSdialTonePattern]  },
    { &tVPLineXCT_IMSEntityLeafInfo[eXCT_IMSmcidService]  },
    { &tVPLineXCT_IMSEntityLeafInfo[eXCT_IMSnoDialingBehaviour]  },
    { &tVPLineXCT_IMSEntityLeafInfo[eXCT_IMSholdService]  },
    { &tVPLineXCT_IMSEntityLeafInfo[eXCT_IMSthreePartyService]  },
    { &tVPLineXCT_IMSEntityLeafInfo[eXCT_IMSconferenceUri]  },
    { &tVPLineXCT_IMSEntityLeafInfo[eXCT_IMShotlineUri]  },
    { &tVPLineXCT_IMSEntityLeafInfo[eXCT_IMScwService]  },
    { NULL }
};


//******* VoiceService.{i}.VoiceProfile.{i}.Line.{i}.CallingFeatures EntityLEAF*******
 
struct CWMP_OP tVPLineCallingFeaturesEntityLeafOP = 
  { getLineCallingFeaturesEntity, setLineCallingFeaturesEntity};

struct CWMP_PRMT tVPLineCallingFeaturesEntityLeafInfo[] =
{
    /*(name,                    type,           flag,                  op)*/
	{ "CallerIDName",           eCWMP_tSTRING,     CWMP_READ|CWMP_WRITE,  &tVPLineCallingFeaturesEntityLeafOP},
	{ "CallWaitingEnable",      eCWMP_tBOOLEAN,     CWMP_READ|CWMP_WRITE,  &tVPLineCallingFeaturesEntityLeafOP},
	{ "X_Stanby-CallerIDName",           eCWMP_tSTRING,     CWMP_READ|CWMP_WRITE,  &tVPLineCallingFeaturesEntityLeafOP},

};

enum eVPLineCallingFeaturesEntityLeaf
{
    eCallingFeaturesCallerIDName,
    eCallingFeaturesCallWaitingEnable,
    eCallingFeatures_XCT_CallerIDName,
};

struct CWMP_LEAF tVPLineCallingFeaturesEntityLeaf[] =
{
    { &tVPLineCallingFeaturesEntityLeafInfo[eCallingFeaturesCallerIDName]  },
    { &tVPLineCallingFeaturesEntityLeafInfo[eCallingFeaturesCallWaitingEnable]  },
	{ &tVPLineCallingFeaturesEntityLeafInfo[eCallingFeatures_XCT_CallerIDName]  },    
    { NULL }
};


//******* VoiceService.{i}.VoiceProfile.{i}.Line.{i}. EntityLEAF*******
struct CWMP_OP tVPLineEntityLeafOP = { getVoiceProfileLineEntity, setVoiceProfileLineEntity};	//FIXME

struct CWMP_PRMT tVPLineEntityLeafInfo[] =
{
    /*(name,              type,            flag,                  op)*/
	{ "Enable",           eCWMP_tSTRING,  CWMP_READ|CWMP_WRITE,  &tVPLineEntityLeafOP},
	{ "DirectoryNumber",  eCWMP_tSTRING,  CWMP_READ|CWMP_WRITE,  &tVPLineEntityLeafOP},
	{ "Status",           eCWMP_tSTRING,  CWMP_READ,             &tVPLineEntityLeafOP},
	{ "X_Standby-Enable", 		  eCWMP_tSTRING,  CWMP_READ|CWMP_WRITE,  &tVPLineEntityLeafOP},
	{ "X_Standby-DirectoryNumber",  eCWMP_tSTRING,  CWMP_READ|CWMP_WRITE,  &tVPLineEntityLeafOP},

};

enum eVPLineEntityLeaf
{
    eLineEnable,
    eLineDirectoryNumber,
    eLineStatus,
    eX_STB_LineEnable,
    eX_STB_LineDirectoryNumber,    
};

struct CWMP_LEAF tVPLineEntityLeaf[] =
{
    { &tVPLineEntityLeafInfo[eLineEnable]  },
    { &tVPLineEntityLeafInfo[eLineDirectoryNumber]  },
    { &tVPLineEntityLeafInfo[eLineStatus]  },
    { &tVPLineEntityLeafInfo[eX_STB_LineEnable]  },
    { &tVPLineEntityLeafInfo[eX_STB_LineDirectoryNumber]  },    
    { NULL }
};


//******* VoiceService.{i}.VoiceProfile.{i}.Line.{i}. EntityObject*******
struct CWMP_PRMT tVPLineEntityObjectInfo[] =
{
    /*(name,             type,           flag,       op)*/
	{ "SIP",             eCWMP_tOBJECT,  CWMP_READ,  NULL},
	{ "Stats",           eCWMP_tOBJECT,  CWMP_READ,  NULL},
	{ "Codec",           eCWMP_tOBJECT,  CWMP_READ,  NULL},
    { "VoiceProcessing", eCWMP_tOBJECT,  CWMP_READ,  NULL},
    { "X_IMS",    eCWMP_tOBJECT,  CWMP_READ,  NULL},
	{ "CallingFeatures", eCWMP_tOBJECT,  CWMP_READ,	 NULL}
};
enum eVPLineEntityObject
{
	eLineSIP,
	eLineStats,
	eLineCodec,
	eLineVoiceProcessing,
	eLineXCTIMS,
	eLineCallingFeatures
};
struct CWMP_NODE tVPLineEntityObject[] =
{
    /*info,                                          leaf,                             next)*/
    {&tVPLineEntityObjectInfo[eLineSIP],             tVPLineSIPEntityLeaf,             NULL},	
    {&tVPLineEntityObjectInfo[eLineStats],           tVPLineStatsEntityLeaf,           NULL},
    {&tVPLineEntityObjectInfo[eLineCodec],           tVPLineCodecEntityLeaf,     tVPLineCodecEntityObject},
    {&tVPLineEntityObjectInfo[eLineVoiceProcessing], tVPLineVoiceProcessingEntityLeaf, NULL},   
    {&tVPLineEntityObjectInfo[eLineXCTIMS],          tVPLineXCT_IMSEntityLeaf,         NULL},
    {&tVPLineEntityObjectInfo[eLineCallingFeatures], tVPLineCallingFeaturesEntityLeaf,NULL},   
    {NULL,                                           NULL,                             NULL}
};


//******* VoiceService.{i}.VoiceProfile.{i}.Line.{i}.  *******
struct CWMP_PRMT tVPLineObjectInfo[] =
{
    /*(name,  type,           flag,		                          op)*/
    {"0",     eCWMP_tOBJECT,  CWMP_READ|CWMP_LNKLIST,  NULL}
};
enum eVPLineObject
{
    eVPLine0
};
struct CWMP_LINKNODE tVPLineObject[] =
{
    /*info,  			            leaf,               next,                 sibling,  instnum)*/
    {&tVPLineObjectInfo[eVPLine0],  tVPLineEntityLeaf,  tVPLineEntityObject,  NULL,     0}
};


int objVoiceProfileLine(char *name, struct CWMP_LEAF *e, int type, void *data)
{
    struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
    
	switch( type ) {
	case eCWMP_tINITOBJ:
	{
		int num=0,i;
		struct CWMP_LINKNODE **ptable = (struct CWMP_LINKNODE **)data;

		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		num = MAX_LINE_PER_PROFILE;
		for( i=1; i<=num;i++ )
		{			
			if( create_Object( ptable, tVPLineObject, sizeof(tVPLineObject), num, 1) < 0 )
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
            num = MAX_LINE_PER_PROFILE;
            old_table = (struct CWMP_LINKNODE*)entity->next;
            entity->next= NULL;
            for( i=1; i<=num;i++ ) {
                struct CWMP_LINKNODE *remove_entity=NULL;

                remove_entity = remove_SiblingEntity( &old_table, i );
                if( remove_entity!=NULL )
                {
                	add_SiblingEntity( (struct CWMP_LINKNODE**)&entity->next, remove_entity );
                }else{ 
    
                    add_Object( name, (struct CWMP_LINKNODE **)&entity->next,
                        tVPLineObject, sizeof(tVPLineObject), (unsigned int*)&num );
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

int getVoiceProfileLineEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.Line.{i}. */
	unsigned int nVoiceProfileInstNum;
	unsigned int nSipLineInstNum;
	unsigned int nNumberOfLine;
	union {
		enable_t Enable;
		char DirectoryNumber[ 32 ];
		line_status_t Status;
	} s;
	
	const char *pszLastname = entity->info->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile_Line( name, &nVoiceProfileInstNum, &nSipLineInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	if( !mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &nNumberOfLine ) )
		return -1;
	
	if( -- nSipLineInstNum >= nNumberOfLine )	/* convert to zero based */
		return -1;
	
	*type = entity->info->type;
    *data = NULL;

	if( strcmp( pszLastname, "Enable" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__ENABLE, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   &s.Enable );
		*data = strdup_Enable( s.Enable );
	}else if( strcmp( pszLastname, "X_Standby-Enable" ) == 0 ) {
			mib_get_type2( MIB_VOICE_PROFILE__LINE__XCT_ENABLE, 
						   nVoiceProfileInstNum, nSipLineInstNum,
						   &s.Enable );
			*data = strdup_Enable( s.Enable );
		
	} else if( strcmp( pszLastname, "DirectoryNumber" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__DIRECTORY_NUMBER, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   s.DirectoryNumber );
		*data = strdup( s.DirectoryNumber );
	} else if( strcmp( pszLastname, "X_Standby-DirectoryNumber" ) == 0 ) {
			mib_get_type2( MIB_VOICE_PROFILE__LINE__XCT_DIRECTORY_NUMBER, 
						   nVoiceProfileInstNum, nSipLineInstNum,
						   s.DirectoryNumber );
			*data = strdup( s.DirectoryNumber );
		
	} else if( strcmp( pszLastname, "Status" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__STATUS, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   &s.Status );
		*data = strdup_LineStatus( s.Status );
	} else {
		*data = NULL;
		return ERR_9005;
	}

	return 0;
}

int setVoiceProfileLineEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.Line.{i}. */
    int res = 0;
	unsigned int nVoiceProfileInstNum;
	unsigned int nSipLineInstNum;
	unsigned int nNumberOfLine;
	union {
		enable_t Enable;
		char DirectoryNumber[ 32 ];
		line_status_t Status;
	} s;
	
	const char *pszLastname = entity->info->name;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile_Line( name, &nVoiceProfileInstNum, &nSipLineInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	if( !mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &nNumberOfLine ) )
		return -1;
	
	if( -- nSipLineInstNum >= nNumberOfLine )	/* convert to zero based */
		return -1;
	
	if( entity->info->type != type )
		return ERR_9006;

    CWMPDBG( 3, ( stderr, "<%s:%d> set %s value \n", __FUNCTION__, __LINE__, name) );

	if( strcmp( pszLastname, "Enable" ) == 0 ) {

	/*add set value verify */
		if(strcmp(data,"Enabled")&&strcmp(data,"Disabled"))
			return ERR_9007;
			
		str2id_Enable( data, &s.Enable );
		res = mib_set_type2( MIB_VOICE_PROFILE__LINE__ENABLE, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   &s.Enable );


	}else if( strcmp( pszLastname, "X_Standby-Enable" ) == 0 ) {

	/*add set value verify */
		if(strcmp(data,"Enabled")&&strcmp(data,"Disabled"))
			return ERR_9007;
			
		str2id_Enable( data, &s.Enable );
		res = mib_set_type2( MIB_VOICE_PROFILE__LINE__XCT_ENABLE, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   &s.Enable );
		
	} else if( strcmp( pszLastname, "DirectoryNumber" ) == 0 ) {
		res = mib_set_type2( MIB_VOICE_PROFILE__LINE__DIRECTORY_NUMBER, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   data );
	} else if( strcmp( pszLastname, "X_Standby-DirectoryNumber" ) == 0 ) {
		res = mib_set_type2( MIB_VOICE_PROFILE__LINE__XCT_DIRECTORY_NUMBER, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   data );	
	} else if( strcmp( pszLastname, "Status" ) == 0 ) {
		return ERR_9008;
	} else {
		return ERR_9005;
	}

	return res;
}

int getLineStatsEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.Line.{i}.Stats. */
	unsigned int nVoiceProfileInstNum;
	unsigned int nSipLineInstNum;
	unsigned int nNumberOfLine;
	unsigned int chid;
	extern cwmpEvtMsg pEvtMsg;
	union {
		TstRtpRtcpStatistics rtpStatistics;
	} s;
    const char *pszLastname = entity->info->name;

	if(!gVoipReq){
        CWMPDBG( 3, ( stderr, "<%s:%d>INFO: Send Request from cwmp to solar", __FUNCTION__, __LINE__) );
        cwmpSendRequestToSolar();
        gVoipReq = EVT_VOICEPROFILE_LINE_GET_STATUS;
    }
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile_Line( name, &nVoiceProfileInstNum, &nSipLineInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	if( !mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &nNumberOfLine ) )
		return -1;
	
	if( -- nSipLineInstNum >= nNumberOfLine )	/* convert to zero based */
		return -1;
	
	*type = entity->info->type;
    *data = NULL;

	/*
	 * FIXME: Now, instance number of 'Line' is seens as channel ID,  
	 *        but specification indicate thta DirectoryNumber or PhyReferenceList 
	 *        is used to identify or associate with physical interface. 
	 */
	chid = nSipLineInstNum;
#if 1
	if( strcmp( pszLastname, "PacketsSent" ) == 0 ) {
		if( rtk_GetRtpRtcpStatistics( chid,0 ,0, &s.rtpStatistics ) )
			return ERR_9002;	/* Internal error */
		*data = uintdup( s.rtpStatistics.nTxPkts );
   		printf("@@@@@Line %d PacketsSent:%d\n",nVoiceProfileInstNum, s.rtpStatistics.nTxPkts);
	} else if( strcmp( pszLastname, "PacketsReceived" ) == 0 ) {
		if( rtk_GetRtpRtcpStatistics( chid, 0,0, &s.rtpStatistics ) )
			return ERR_9002;	/* Internal error */
		*data = uintdup( s.rtpStatistics.nRxPkts );
        printf("@@@@@Line %d PacketsReceived:%d\n",nVoiceProfileInstNum, s.rtpStatistics.nRxPkts);
	} else if( strcmp( pszLastname, "BytesSent" ) == 0 ) {
		if( rtk_GetRtpRtcpStatistics( chid,0, 0, &s.rtpStatistics ) )
			return ERR_9002;	/* Internal error */
		*data = uintdup( s.rtpStatistics.nTxBytes );
        printf("@@@@@Line %d BytesSent:%d\n",nVoiceProfileInstNum, s.rtpStatistics.nTxBytes);
	} else if( strcmp( pszLastname, "BytesReceived" ) == 0 ) {
		if( rtk_GetRtpRtcpStatistics( chid,0, 0, &s.rtpStatistics ) )
			return ERR_9002;	/* Internal error */
		*data = uintdup( s.rtpStatistics.nRxBytes );
        printf("@@@@@Line %d BytesReceived:%d\n",nVoiceProfileInstNum, s.rtpStatistics.nRxBytes);
	} else if( strcmp( pszLastname, "PacketsLost" ) == 0 ) {
		if( rtk_GetRtpRtcpStatistics( chid,0, 0, &s.rtpStatistics ) )
			return ERR_9002;	/* Internal error */
		*data = uintdup( s.rtpStatistics.nLost );
        printf("@@@@@Line %d PacketsLost:%d\n",nVoiceProfileInstNum, s.rtpStatistics.nLost);
	} else if( strcmp( pszLastname, "IncomingCallsReceived" ) == 0 ) {
		*data = uintdup( pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].incomingCallsReceived);
		printf("@@@@@Line %d IncomingCallsReceived:%d\n",nVoiceProfileInstNum, pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].incomingCallsReceived);
	} else if( strcmp( pszLastname, "IncomingCallsAnswered" ) == 0 ) {
		*data = uintdup( pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].incomingCallsAnswered);
		printf("@@@@@Line %d IncomingCallsAnswered:%d\n",nVoiceProfileInstNum, pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].incomingCallsAnswered);
	} else if( strcmp( pszLastname, "IncomingCallsConnected" ) == 0 ) {
		*data = uintdup( pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].incomingCallsConnected);
		printf("@@@@@Line %d IncomingCallsConnected:%d\n",nVoiceProfileInstNum, pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].incomingCallsConnected);
	} else if( strcmp( pszLastname, "IncomingCallsFailed" ) == 0 ) {
		*data = uintdup( pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].incomingCallsFailed);
		printf("@@@@@Line %d IncomingCallsFailed:%d\n",nVoiceProfileInstNum, pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].incomingCallsFailed);
	} else if( strcmp( pszLastname, "OutgoingCallsAttempted" ) == 0 ) {
		*data = uintdup( pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].outgoingCallsAttempted);
		printf("@@@@@Line %d OutgoingCallsAttempted:%d\n",nVoiceProfileInstNum, pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].outgoingCallsAttempted);
	} else if( strcmp( pszLastname, "OutgoingCallsAnswered" ) == 0 ) {
		*data = uintdup( pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].outgoingCallsAnswered);
		printf("@@@@@Line %d OutgoingCallsAnswered:%d\n",nVoiceProfileInstNum, pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].outgoingCallsAnswered);
	} else if( strcmp( pszLastname, "OutgoingCallsConnected" ) == 0 ) {
		*data = uintdup( pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].outgoingCallsConnected);
		printf("@@@@@Line %d OutgoingCallsConnected:%d\n",nVoiceProfileInstNum, pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].outgoingCallsConnected);
	} else if( strcmp( pszLastname, "OutgoingCallsFailed" ) == 0 ) {
		*data = uintdup( pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].outgoingCallsFailed);
		printf("@@@@@Line %d OutgoingCallsFailed:%d\n",nVoiceProfileInstNum, pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].outgoingCallsFailed);
	} else if( strcmp( pszLastname, "ResetStatistics" ) == 0 ) {
		*data = uintdup( pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].resetStatistics);
		printf("@@@@@Line %d ResetStatistics:%d\n",nVoiceProfileInstNum, pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].resetStatistics);
	} else if( strcmp( pszLastname, "TotalCallTime" ) == 0 ) {
		*data = uintdup( pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].totalCallTime);
		printf("@@@@@Line %d TotalCallTime:%d\n",nVoiceProfileInstNum, pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].totalCallTime);
	} else {
		*data = NULL;
		return ERR_9005;
	}
#endif
	return 0;
}

int setLineStatsEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.SIP.Line.{i}.SIP. */
    int res = 0;
	unsigned int nVoiceProfileInstNum;
	unsigned int nSipLineInstNum;
	unsigned int nNumberOfLine;
	/*union {
		TstRtpRtcpStatistics rtpStatistics;
	} s;*/

	const char *pszLastname = entity->info->name;
    extern cwmpEvtMsg pEvtMsg;

    if( (name==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile_Line( name, &nVoiceProfileInstNum, &nSipLineInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	if( !mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &nNumberOfLine ) )
		return -1;
	
	if( -- nSipLineInstNum >= nNumberOfLine )	/* convert to zero based */
		return -1;
	
	if( entity->info->type != type )
		return ERR_9006;

    CWMPDBG( 3, ( stderr, "<%s:%d>Debug: set %s value \n", __FUNCTION__, __LINE__, name) );

    if( strcmp( pszLastname, "ResetStatistics" ) == 0 ) {
		res = mib_set_type2( MIB_VOICE_PROFILE__LINE__STATS__RESET_STATISTICS, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   data );
        if(!gVoipReq){
            cwmpEvtMsg *pEvent =  cwmpEvtMsgNew();
            if(pEvent != NULL){
                pEvent->voiceProfileLineStatusMsg->line = nSipLineInstNum;
                pEvent->voiceProfileLineStatusMsg->profileID = nVoiceProfileInstNum;
                pEvent->event = gVoipReq = EVT_VOICEPROFILE_LINE_SET_STATUS;
                memcpy((void*)&pEvtMsg, (void*)pEvent,  sizeof(cwmpEvtMsg));
                free(pEvent);    
            }
        }
            
	} else if( strcmp( pszLastname, "PacketsSent" ) == 0 || 
               strcmp( pszLastname, "BytesSent" ) == 0 ||
               strcmp( pszLastname, "BytesReceived" ) == 0 ||
               strcmp( pszLastname, "PacketsLost" ) == 0 ||
               strcmp( pszLastname, "IncomingCallsReceived" ) == 0 ||
               strcmp( pszLastname, "IncomingCallsAnswered" ) == 0 ||
               strcmp( pszLastname, "IncomingCallsConnected" ) == 0 ||
               strcmp( pszLastname, "IncomingCallsFailed" ) == 0 ||
               strcmp( pszLastname, "OutgoingCallsAttempted" ) == 0 ||
               strcmp( pszLastname, "OutgoingCallsAnswered" ) == 0 ||
               strcmp( pszLastname, "OutgoingCallsConnected" ) == 0 ||
               strcmp( pszLastname, "OutgoingCallsFailed" ) == 0 ||
               strcmp( pszLastname, "TotalCallTime" ) == 0 ) {
		return ERR_9008;
	} else {
		return ERR_9005;
	}
    return res;
}


int getLineCodecEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.Line.{i}.codec. entry */
	unsigned int nVoiceProfileInstNum;
	unsigned int nSipLineInstNum;
	unsigned int nNumberOfLine;
	unsigned int chid;
	extern cwmpEvtMsg pEvtMsg;
	union {
		TstRtpRtcpStatistics rtpStatistics;
	} s;
    const char *pszLastname = entity->info->name;

	if(!gVoipReq){
        CWMPDBG( 3, ( stderr, "<%s:%d>INFO: Send Request from cwmp to solar", __FUNCTION__, __LINE__) );
        cwmpSendRequestToSolar();
        gVoipReq = EVT_VOICEPROFILE_LINE_GET_STATUS;
    }
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile_Line( name, &nVoiceProfileInstNum, &nSipLineInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	if( !mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &nNumberOfLine ) )
		return -1;
	
	if( -- nSipLineInstNum >= nNumberOfLine )	/* convert to zero based */
		return -1;
	
	*type = entity->info->type;
    *data = NULL;

	/*
	 * FIXME: Now, instance number of 'Line' is seens as channel ID,  
	 *        but specification indicate thta DirectoryNumber or PhyReferenceList 
	 *        is used to identify or associate with physical interface. 
	 */
	chid = nSipLineInstNum;

	if( strcmp( pszLastname, "TransmitCodec" ) == 0 ) {
		*data = strdup(  pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].codec_string);
			printf("@@@@@Line %d TransmitCodec:%s\n",nVoiceProfileInstNum, pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].codec_string);
	
	}else if( strcmp( pszLastname, "ReceiveCodec" ) == 0 ) {
		*data = strdup(  pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].codec_string);

	} else {
		*data = NULL;
		return ERR_9005;
	}

	return 0;
}

int setLineCodecEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.SIP.Line.{i}.SIP. */
    int res = 0;
	unsigned int nVoiceProfileInstNum;
	unsigned int nSipLineInstNum;
	unsigned int nNumberOfLine;
	/*union {
		TstRtpRtcpStatistics rtpStatistics;
	} s;*/

	const char *pszLastname = entity->info->name;
    extern cwmpEvtMsg pEvtMsg;

    if( (name==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile_Line( name, &nVoiceProfileInstNum, &nSipLineInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	if( !mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &nNumberOfLine ) )
		return -1;
	
	if( -- nSipLineInstNum >= nNumberOfLine )	/* convert to zero based */
		return -1;
	
	if( entity->info->type != type )
		return ERR_9006;

    CWMPDBG( 3, ( stderr, "<%s:%d>Debug: set %s value \n", __FUNCTION__, __LINE__, name) );

	if( strcmp( pszLastname, "TransmitCodec" ) == 0 || 
               strcmp( pszLastname, "ReceiveCodec" ) == 0 ) {
		return ERR_9008;
	} else {
		return ERR_9005;
	}
    return res;
}


int getLineSipEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.SIP.Line.{i}.SIP. */
	unsigned int nVoiceProfileInstNum;
	unsigned int nSipLineInstNum;
	unsigned int nNumberOfLine;
	union {
		char AuthUserName[ 128 ];
		char AuthPassword[ 128 ];
		char URI[ 389 ];
	} s;

	const char *pszLastname = entity->info->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile_Line( name, &nVoiceProfileInstNum, &nSipLineInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	if( !mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &nNumberOfLine ) )
		return -1;
	
	if( -- nSipLineInstNum >= nNumberOfLine )	/* convert to zero based */
		return -1;
	
	*type = entity->info->type;
    *data = NULL;

    CWMPDBG( 3, ( stderr, "<%s:%d> get %s value \n", __FUNCTION__, __LINE__, name) );

	if( strcmp( pszLastname, "AuthUserName" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__SIP__AUTH_USER_NAME, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   &s.AuthUserName );
		*data = strdup( s.AuthUserName );

	}else if( strcmp( pszLastname, "X_Standby-AuthUserName" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__SIP__XCT_AUTH_USER_NAME, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   &s.AuthUserName );
		*data = strdup( s.AuthUserName );
		
	} else if( strcmp( pszLastname, "AuthPassword" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__SIP__AUTH_PASSWORD, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   s.AuthPassword );
		*data = strdup( s.AuthPassword );
	} else if( strcmp( pszLastname, "X_Standby-AuthPassword" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__SIP__XCT_AUTH_PASSWORD, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   s.AuthPassword );
		*data = strdup( s.AuthPassword );

		
	} else if( strcmp( pszLastname, "URI" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__SIP__URI, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   &s.URI );
		*data = strdup( s.URI );
	} else if( strcmp( pszLastname, "X_Standby-URI" ) == 0 ) {
			mib_get_type2( MIB_VOICE_PROFILE__LINE__SIP__XCT_URI, 
						   nVoiceProfileInstNum, nSipLineInstNum,
						   &s.URI );
			*data = strdup( s.URI );
		
	} else {
		*data = NULL;
		return ERR_9005;
	}

	return 0;
}

int setLineSipEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.SIP.Line.{i}.SIP. */
    int res = 0;
	unsigned int nVoiceProfileInstNum;
	unsigned int nSipLineInstNum;
	unsigned int nNumberOfLine;

	const char *pszLastname = entity->info->name;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile_Line( name, &nVoiceProfileInstNum, &nSipLineInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	if( !mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &nNumberOfLine ) )
		return -1;
	
	if( -- nSipLineInstNum >= nNumberOfLine )	/* convert to zero based */
		return -1;

	if( entity->info->type != type )
		return ERR_9006;

    CWMPDBG( 3, ( stderr, "<%s:%d> set %s value \n", __FUNCTION__, __LINE__, name) );

	if( strcmp( pszLastname, "AuthUserName" ) == 0 ) {
		res = mib_set_type2( MIB_VOICE_PROFILE__LINE__SIP__AUTH_USER_NAME, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   data );
	}else if( strcmp( pszLastname, "X_Standby-AuthUserName" ) == 0 ) {
		res = mib_set_type2( MIB_VOICE_PROFILE__LINE__SIP__XCT_AUTH_USER_NAME, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   data );
		
	} else if( strcmp( pszLastname, "AuthPassword" ) == 0 ) {
		res = mib_set_type2( MIB_VOICE_PROFILE__LINE__SIP__AUTH_PASSWORD, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   data );
	} else if( strcmp( pszLastname, "X_Standby-AuthPassword" ) == 0 ) {
		res = mib_set_type2( MIB_VOICE_PROFILE__LINE__SIP__XCT_AUTH_PASSWORD, 
						   nVoiceProfileInstNum, nSipLineInstNum,
						   data );
		
	} else if( strcmp( pszLastname, "URI" ) == 0 ) {
		res = mib_set_type2( MIB_VOICE_PROFILE__LINE__SIP__URI, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   data );
	} else if( strcmp( pszLastname, "X_Standby-URI" ) == 0 ) {
		res = mib_set_type2( MIB_VOICE_PROFILE__LINE__SIP__XCT_URI, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   data );
		
	} else {
		return ERR_9005;
	}

	return res;
}

int getLineVoiceProcessingEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.SIP.Line.{i}.VoiceProcessing. */
	unsigned int nVoiceProfileInstNum;
	unsigned int nSipLineInstNum;
	unsigned int nNumberOfLine;
	union {
		int TransmitGain;
		int ReceiveGain;
		int EchoCancellationEnable;
		int  EchoCancellationInUse;
        unsigned int EchoCancellationTail;
	} s;

	const char *pszLastname = entity->info->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile_Line( name, &nVoiceProfileInstNum, &nSipLineInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	if( !mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &nNumberOfLine ) )
		return -1;
	
	if( -- nSipLineInstNum >= nNumberOfLine )	/* convert to zero based */
		return -1;
	
	*type = entity->info->type;
    *data = NULL;

    CWMPDBG( 3, ( stderr, "<%s:%d> get %s value \n", __FUNCTION__, __LINE__, name) );

	if( strcmp( pszLastname, "TransmitGain" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__VOIP_PROCESSING__TRANSMIT_GAIN, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   &s.TransmitGain );
		*data = intdup( s.TransmitGain );
	} else if( strcmp( pszLastname, "ReceiveGain" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__VOIP_PROCESSING__RECEIVE_GAIN, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   &s.ReceiveGain );
		*data = intdup( s.ReceiveGain );
	} else if( strcmp( pszLastname, "EchoCancellationEnable" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__VOIP_PROCESSING__ECHO_CANCELLATION_ENABLE, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   &s.EchoCancellationEnable );
		*data = booldup( s.EchoCancellationEnable );
	} else if( strcmp( pszLastname, "EchoCancellationInUse" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__VOIP_PROCESSING__ECHO_CANCELLATION_INUSE, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   &s.EchoCancellationInUse );
		*data = booldup( s.EchoCancellationInUse );
	} else if( strcmp( pszLastname, "EchoCancellationTail" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__VOIP_PROCESSING__ECHO_CANCELLATION_TAIL, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   &s.EchoCancellationTail );
		*data = uintdup( s.EchoCancellationTail );
	} else {
		*data = NULL;
		return ERR_9005;
	}

	return 0;
}

int setLineVoiceProcessingEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.Line.{i}.VoiceProcessing. */
    int res = 0;
	unsigned int nVoiceProfileInstNum;
	unsigned int nSipLineInstNum;
	unsigned int nNumberOfLine;

	const char *pszLastname = entity->info->name;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile_Line( name, &nVoiceProfileInstNum, &nSipLineInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	if( !mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &nNumberOfLine ) )
		return -1;
	
	if( -- nSipLineInstNum >= nNumberOfLine )	/* convert to zero based */
		return -1;

	if( entity->info->type != type )
		return ERR_9006;

    CWMPDBG( 3, ( stderr, "<%s:%d> set %s value \n", __FUNCTION__, __LINE__, name) );

	if( strcmp( pszLastname, "TransmitGain" ) == 0 ) {
		res = mib_set_type2( MIB_VOICE_PROFILE__LINE__VOIP_PROCESSING__TRANSMIT_GAIN, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   data );
	} else if( strcmp( pszLastname, "ReceiveGain" ) == 0 ) {
		res = mib_set_type2( MIB_VOICE_PROFILE__LINE__VOIP_PROCESSING__RECEIVE_GAIN, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   data );
	} else if( strcmp( pszLastname, "EchoCancellationEnable" ) == 0 ) {
		res = mib_set_type2( MIB_VOICE_PROFILE__LINE__VOIP_PROCESSING__ECHO_CANCELLATION_ENABLE, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   data );
	} else if( strcmp( pszLastname, "EchoCancellationInUse" ) == 0 ) {
		return ERR_9008;
	} else if( strcmp( pszLastname, "EchoCancellationTail" ) == 0 ) {
		return ERR_9008;
	} else {
		return ERR_9005;
	}

	return res;
}


int getLineCallingFeaturesEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.SIP.Line.{i}.CallingFeatures. */
	unsigned int nVoiceProfileInstNum;
	unsigned int nSipLineInstNum;
	unsigned int nNumberOfLine;
	union {
		char CallerIDName[ 40 ];
		int CallWaitingEnable;
	} s;

	const char *pszLastname = entity->info->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
	
	if( !GetOneBasedInstanceNumber_VoiceProfile_Line( name, &nVoiceProfileInstNum, &nSipLineInstNum ) )
		return -1;

	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;

	if( !mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &nNumberOfLine ) )
		return -1;

	if( -- nSipLineInstNum >= nNumberOfLine )	/* convert to zero based */
		return -1;

	*type = entity->info->type;
	*data = NULL;

	CWMPDBG( 3, ( stderr, "<%s:%d> get %s value \n", __FUNCTION__, __LINE__, name) );



	if( strcmp( pszLastname, "CallerIDName" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__CALLING_FEATURES_CALLERIDNAME, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   s.CallerIDName );
		*data = strdup( s.CallerIDName );
	}else if( strcmp( pszLastname, "X_Stanby-CallerIDName" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__XCT_CALLING_FEATURES_CALLERIDNAME, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   s.CallerIDName );
		*data = strdup( s.CallerIDName );
	}else if( strcmp( pszLastname, "CallWaitingEnable" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__CALLING_FEATURES_CALLWAITING_ENABLE, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   &s.CallWaitingEnable );
		*data = booldup( s.CallWaitingEnable );
	}


return 0;


}

int setLineCallingFeaturesEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
    int res = 0;
	unsigned int nVoiceProfileInstNum;
	unsigned int nSipLineInstNum;
	unsigned int nNumberOfLine;

	const char *pszLastname = entity->info->name;
	

	if( (name==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile_Line( name, &nVoiceProfileInstNum, &nSipLineInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	if( !mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &nNumberOfLine ) )
		return -1;
	
	if( -- nSipLineInstNum >= nNumberOfLine )	/* convert to zero based */
		return -1;

	if( entity->info->type != type )
		return ERR_9006;

    CWMPDBG( 3, ( stderr, "<%s:%d> set %s value \n", __FUNCTION__, __LINE__, name) );


	if( strcmp( pszLastname, "CallerIDName" ) == 0 ) {
		res = mib_set_type2( MIB_VOICE_PROFILE__LINE__CALLING_FEATURES_CALLERIDNAME, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   data );
	}else if( strcmp( pszLastname, "X_Stanby-CallerIDName" ) == 0 ) {
		res = mib_set_type2( MIB_VOICE_PROFILE__LINE__XCT_CALLING_FEATURES_CALLERIDNAME, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   data );
			
	}else if( strcmp( pszLastname, "CallWaitingEnable" ) == 0 ) {
		res = mib_set_type2( MIB_VOICE_PROFILE__LINE__CALLING_FEATURES_CALLWAITING_ENABLE, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   data );
	} 
	
	return res;

}


int getLineXCT_IMSEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.SIP.Line.{i}.X_IMS. */
	unsigned int nVoiceProfileInstNum;
	unsigned int nSipLineInstNum;
	unsigned int nNumberOfLine;
	union {
		unsigned int dialTonePattern;
		unsigned int mcidService;
        unsigned int noDialingBehaviour;
        unsigned int holdService;
        unsigned int threePartyService;
        char conferenceUri[64];
        char hotlineUri[64];
        unsigned int cwService;
	} s;

	const char *pszLastname = entity->info->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile_Line( name, &nVoiceProfileInstNum, &nSipLineInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	if( !mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &nNumberOfLine ) )
		return -1;
	
	if( -- nSipLineInstNum >= nNumberOfLine )	/* convert to zero based */
		return -1;
	
	*type = entity->info->type;
    *data = NULL;

    CWMPDBG( 3, ( stderr, "<%s:%d> get %s value \n", __FUNCTION__, __LINE__, name) );

	if( strcmp( pszLastname, "dial-tone-pattern" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__XCT_IMS__DIALTONEPATTERN, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   &s.dialTonePattern );
		*data = uintdup( s.dialTonePattern );
	} else if( strcmp( pszLastname, "mcid-service" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__XCT_IMS__MCIDSERVICE, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   &s.mcidService );
		*data = uintdup( s.mcidService );
	} else if( strcmp( pszLastname, "no-dialing-behaviour" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__XCT_IMS__NODIALINGBEHAVIOUR, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   &s.noDialingBehaviour );
		*data = uintdup( s.noDialingBehaviour );
	} else if( strcmp( pszLastname, "hold-service" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__XCT_IMS__HOLDSERVICE, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   &s.holdService );
		*data = uintdup( s.holdService );
	} else if( strcmp( pszLastname, "three-party-service" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__XCT_IMS__THREEPARTYSERVICE, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   &s.threePartyService );
		*data = uintdup( s.threePartyService );
	} else if( strcmp( pszLastname, "conference-uri" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__XCT_IMS__CONFERENCEURI, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   &s.conferenceUri );
		*data = strdup( s.conferenceUri );
	} else if( strcmp( pszLastname, "hotline-uri" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__XCT_IMS__HOTLINEURI, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   &s.hotlineUri );
		*data = strdup( s.hotlineUri );
	} else if( strcmp( pszLastname, "cw-service" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__XCT_IMS__CWSERVICE, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   &s.cwService );
		*data = uintdup( s.cwService );
	} else {
		*data = NULL;
		return ERR_9005;
	}

	return 0;
}

int setLineXCT_IMSEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.Line.{i}.X_IMS. */
    int res = 0;
	unsigned int nVoiceProfileInstNum;
	unsigned int nSipLineInstNum;
	unsigned int nNumberOfLine;

	const char *pszLastname = entity->info->name;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile_Line( name, &nVoiceProfileInstNum, &nSipLineInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	if( !mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &nNumberOfLine ) )
		return -1;
	
	if( -- nSipLineInstNum >= nNumberOfLine )	/* convert to zero based */
		return -1;

	if( entity->info->type != type )
		return ERR_9006;

    CWMPDBG( 3, ( stderr, "<%s:%d> set %s value \n", __FUNCTION__, __LINE__, name) );
 
	if( strcmp( pszLastname, "dial-tone-pattern" ) == 0 ) {
		return ERR_9008; //read only
	} else if( strcmp( pszLastname, "mcid-service" ) == 0 ) {
		return ERR_9008; //read only
	} else if( strcmp( pszLastname, "no-dialing-behaviour" ) == 0 ) {
		return ERR_9008; //read only
	} else if( strcmp( pszLastname, "hold-service" ) == 0 ) {
		return ERR_9008; //read only
	} else if( strcmp( pszLastname, "three-party-service" ) == 0 ) {
		return ERR_9008; //read only
	} else if( strcmp( pszLastname, "conference-uri" ) == 0 ) {
		res = mib_set_type2( MIB_VOICE_PROFILE__LINE__XCT_IMS__CONFERENCEURI, 
					   nVoiceProfileInstNum, nSipLineInstNum, data );
	} else if( strcmp( pszLastname, "hotline-uri" ) == 0 ) {
		res = mib_set_type2( MIB_VOICE_PROFILE__LINE__XCT_IMS__HOTLINEURI, 
					   nVoiceProfileInstNum, nSipLineInstNum, data );
	} else if( strcmp( pszLastname, "cw-service" ) == 0 ) {
		return ERR_9008; //read only
	} else {
		return ERR_9005;
	}

	return res;
}

#if 0
static int getVoiceProfileLineCodecListEntity(char *name, struct sCWMP_ENTITY *entity, int *type, void **data);
static int setVoiceProfileLineCodecListEntity(char *name, struct sCWMP_ENTITY *entity, int type, void *data);
static int objVoiceProfileLineCodec(char *name, struct sCWMP_ENTITY *entity, int type, void *data);

static int getVoiceProfileLineStatusEntity(char *name, struct sCWMP_ENTITY *entity, int *type, void **data);
static int setVoiceProfileLineStatusEntity(char *name, struct sCWMP_ENTITY *entity, int type, void *data);

static int getVoiceProfileLineSipEntity(char *name, struct sCWMP_ENTITY *entity, int *type, void **data);
static int setLineSipEntity(char *name, struct sCWMP_ENTITY *entity, int type, void *data);

static int getVoiceProfileLineEntity(char *name, struct sCWMP_ENTITY *entity, int *type, void **data);
static int setVoiceProfileLineEntity(char *name, struct sCWMP_ENTITY *entity, int type, void *data);


struct sCWMP_ENTITY tVoiceProfileLineCodecListEntity[] = {
/*	{ name,					type,			flag,					accesslist,	getvalue,								setvalue,							next_table,	sibling } */
	{ "EntryId",			eCWMP_tUINT,	CWMP_READ,				NULL,		getVoiceProfileLineCodecListEntity, 	setVoiceProfileLineCodecListEntity,	NULL,		NULL },
	{ "Codec",				eCWMP_tSTRING,	CWMP_READ,				NULL,		getVoiceProfileLineCodecListEntity, 	setVoiceProfileLineCodecListEntity,	NULL,		NULL },
	{ "BitRate",			eCWMP_tUINT,	CWMP_READ,				NULL,		getVoiceProfileLineCodecListEntity, 	setVoiceProfileLineCodecListEntity,	NULL,		NULL },
	{ "PacketizationPeriod",eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileLineCodecListEntity, 	setVoiceProfileLineCodecListEntity,	NULL,		NULL },
	{ "SilenceSuppression",	eCWMP_tBOOLEAN,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileLineCodecListEntity, 	setVoiceProfileLineCodecListEntity,	NULL,		NULL },
	{ "Priority",			eCWMP_tUINT,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileLineCodecListEntity, 	setVoiceProfileLineCodecListEntity,	NULL,		NULL },
	{ "",					eCWMP_tNONE,	0,						NULL,		NULL,									NULL,								NULL,		NULL },
};

struct sCWMP_ENTITY tVoiceProfileLineCodecList[] = {
/*	{ name,		type,			flag,								accesslist,	getvalue,	setvalue,	next_table,							sibling } */
	{ "0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL,		NULL,		NULL,		tVoiceProfileLineCodecListEntity, 	NULL },
};

struct sCWMP_ENTITY tVoiceProfileLineCodecEntity[] = {
/*	{ name,					type,			flag,		accesslist,	getvalue,					setvalue,					next_table,	sibling } */
	{ "List",				eCWMP_tOBJECT,	CWMP_READ,	NULL,		NULL, 						objVoiceProfileLineCodec,	NULL,		NULL },
	{ "",					eCWMP_tNONE,	0,			NULL,		NULL,						NULL,						NULL,		NULL },
};

struct sCWMP_ENTITY tVoiceProfileLineStatusEntity[] = {
/*	{ name,						type,			flag,		accesslist,	getvalue,							setvalue,							next_table,	sibling } */
	{ "PacketsSent",			eCWMP_tUINT,	CWMP_READ,	NULL,		getVoiceProfileLineStatusEntity, 	setVoiceProfileLineStatusEntity,	NULL,		NULL },
	{ "PacketsReceived",		eCWMP_tUINT,	CWMP_READ,	NULL,		getVoiceProfileLineStatusEntity, 	setVoiceProfileLineStatusEntity,	NULL,		NULL },
	{ "BytesSent",				eCWMP_tUINT,	CWMP_READ,	NULL,		getVoiceProfileLineStatusEntity, 	setVoiceProfileLineStatusEntity,	NULL,		NULL },
	{ "BytesReceived",			eCWMP_tUINT,	CWMP_READ,	NULL,		getVoiceProfileLineStatusEntity, 	setVoiceProfileLineStatusEntity,	NULL,		NULL },
	{ "PacketsLost",			eCWMP_tUINT,	CWMP_READ,	NULL,		getVoiceProfileLineStatusEntity, 	setVoiceProfileLineStatusEntity,	NULL,		NULL },
	{ "IncomingCallsReceived",	eCWMP_tUINT,	CWMP_READ,	NULL,		getVoiceProfileLineStatusEntity, 	setVoiceProfileLineStatusEntity,	NULL,		NULL },
	{ "IncomingCallsAnswered",	eCWMP_tUINT,	CWMP_READ,	NULL,		getVoiceProfileLineStatusEntity, 	setVoiceProfileLineStatusEntity,	NULL,		NULL },
	{ "IncomingCallsConnected",	eCWMP_tUINT,	CWMP_READ,	NULL,		getVoiceProfileLineStatusEntity, 	setVoiceProfileLineStatusEntity,	NULL,		NULL },
	{ "IncomingCallsFailed",	eCWMP_tUINT,	CWMP_READ,	NULL,		getVoiceProfileLineStatusEntity, 	setVoiceProfileLineStatusEntity,	NULL,		NULL },
	{ "OutgoingCallsAttempted",	eCWMP_tUINT,	CWMP_READ,	NULL,		getVoiceProfileLineStatusEntity, 	setVoiceProfileLineStatusEntity,	NULL,		NULL },
	{ "OutgoingCallsAnswered",	eCWMP_tUINT,	CWMP_READ,	NULL,		getVoiceProfileLineStatusEntity, 	setVoiceProfileLineStatusEntity,	NULL,		NULL },
	{ "OutgoingCallsConnected",	eCWMP_tUINT,	CWMP_READ,	NULL,		getVoiceProfileLineStatusEntity, 	setVoiceProfileLineStatusEntity,	NULL,		NULL },
	{ "OutgoingCallsFailed",	eCWMP_tUINT,	CWMP_READ,	NULL,		getVoiceProfileLineStatusEntity, 	setVoiceProfileLineStatusEntity,	NULL,		NULL },
	{ "",						eCWMP_tNONE,	0,			NULL,		NULL,								NULL,								NULL,		NULL },
};

struct sCWMP_ENTITY tVoiceProfileLineSipEntity[] = {
/*	{ name,					type,			flag,					accesslist,	getvalue,						setvalue,						next_table,	sibling } */
	{ "AuthUserName",		eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileLineSipEntity, 	setLineSipEntity,	NULL,		NULL },
	{ "AuthPassword",		eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileLineSipEntity, 	setLineSipEntity,	NULL,		NULL },
	{ "URI",				eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileLineSipEntity, 	setLineSipEntity,	NULL,		NULL },
	{ "",					eCWMP_tNONE,	0,						NULL,		NULL,							NULL,							NULL,		NULL },
};

struct sCWMP_ENTITY tVoiceProfileLineEntity[] = {
/*	{ name,					type,			flag,					accesslist,	getvalue,					setvalue,					next_table,						sibling } */
	{ "Enable",				eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileLineEntity, 	setVoiceProfileLineEntity,	NULL,							NULL },
	{ "DirectoryNumber",	eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileLineEntity, 	setVoiceProfileLineEntity,	NULL,							NULL },
	{ "Status",				eCWMP_tSTRING,	CWMP_READ,				NULL,		getVoiceProfileLineEntity, 	setVoiceProfileLineEntity,	NULL,							NULL },
	{ "SIP",				eCWMP_tOBJECT,	CWMP_READ,				NULL,		NULL, 						NULL,						tVoiceProfileLineSipEntity,		NULL },
	{ "Stats",				eCWMP_tOBJECT,	CWMP_READ,				NULL,		NULL, 						NULL,						tVoiceProfileLineStatusEntity,	NULL },
	{ "Codec",				eCWMP_tOBJECT,	CWMP_READ,				NULL,		NULL, 						NULL,						tVoiceProfileLineCodecEntity,	NULL },
	{ "",					eCWMP_tNONE,	0,						NULL,		NULL,						NULL,						NULL,		NULL },
};

struct sCWMP_ENTITY tVoiceProfileLine[] = {
/*	{ name,		type,			flag,								accesslist,	getvalue,	setvalue,	next_table,				sibling } */
	{ "0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL,		NULL,		NULL,		tVoiceProfileLineEntity, 	NULL },
};

static int getVoiceProfileLineCodecListEntity(char *name, struct sCWMP_ENTITY *entity, int *type, void **data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.SIP.Line.{i}.Codec.List.{i}. */
	unsigned int nVoiceProfileInstNum;
	unsigned int nSipLineInstNum;
	unsigned int nCodecListInstNum;
	unsigned int nNumberOfLine;
	union {
		char PacketizationPeriod[ 64 ];
		int SilenceSuppression;
		unsigned int Priority;
	} s;

	const char *pszLastname = entity ->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
	
	if( !GetOneBasedInstanceNumber_VoiceProfile_Line_List( name, &nVoiceProfileInstNum, &nSipLineInstNum, &nCodecListInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	if( !mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &nNumberOfLine ) )
		return -1;
	
	if( -- nSipLineInstNum >= nNumberOfLine )	/* convert to zero based */
		return -1;
	
	if( -- nCodecListInstNum >= MAX_CODEC_LIST )	/* convert to zero based */
		return -1;
	
	*type = entity ->type;

	if( strcmp( pszLastname, "EntryId" ) == 0 ) {
		*data = uintdup( nCodecListInstNum + 1 );	/* 1 based */
	} else if( strcmp( pszLastname, "Codec" ) == 0 ) {
		*data = strdup( lstCodecs[ nCodecListInstNum ].pszCodec );
	} else if( strcmp( pszLastname, "BitRate" ) == 0 ) {
		*data = uintdup( lstCodecs[ nCodecListInstNum ].nBitRate );
	} else if( strcmp( pszLastname, "PacketizationPeriod" ) == 0 ) {
		mib_get_type3( MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__PACKETIZATION_PERIOD, 
					   nVoiceProfileInstNum, nSipLineInstNum, nCodecListInstNum,
					   &s.PacketizationPeriod );
		*data = strdup( s.PacketizationPeriod );
	} else if( strcmp( pszLastname, "SilenceSuppression" ) == 0 ) {
		mib_get_type3( MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__SILENCE_SUPPRESSION, 
					   nVoiceProfileInstNum, nSipLineInstNum, nCodecListInstNum,
					   &s.SilenceSuppression );
		*data = booldup( s.SilenceSuppression );
	} else if( strcmp( pszLastname, "Priority" ) == 0 ) {
		mib_get_type3( MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__PRIORITY, 
					   nVoiceProfileInstNum, nSipLineInstNum, nCodecListInstNum,
					   &s.Priority );
		*data = uintdup( s.Priority );
	} else {
		*data = NULL;
		return ERR_9005;
	}

	return 0;
}

static int setVoiceProfileLineCodecListEntity(char *name, struct sCWMP_ENTITY *entity, int type, void *data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.Line.{i}.Codec.List.{i}. */
	unsigned int nVoiceProfileInstNum;
	unsigned int nSipLineInstNum;
	unsigned int nCodecListInstNum;
	unsigned int nNumberOfLine;

	const char *pszLastname = entity ->name;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
	
	if( !GetOneBasedInstanceNumber_VoiceProfile_Line_List( name, &nVoiceProfileInstNum, &nSipLineInstNum, &nCodecListInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	if( !mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &nNumberOfLine ) )
		return -1;
	
	if( -- nSipLineInstNum >= nNumberOfLine )	/* convert to zero based */
		return -1;
	
	if( -- nCodecListInstNum >= MAX_CODEC_LIST )	/* convert to zero based */
		return -1;

	if( entity ->type != type )
		return ERR_9006;

	if( strcmp( pszLastname, "EntryId" ) == 0 ) {
		return ERR_9008;
	} else if( strcmp( pszLastname, "Codec" ) == 0 ) {
		return ERR_9008;
	} else if( strcmp( pszLastname, "BitRate" ) == 0 ) {
		return ERR_9008;
	} else if( strcmp( pszLastname, "PacketizationPeriod" ) == 0 ) {
		mib_set_type3( MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__PACKETIZATION_PERIOD, 
					   nVoiceProfileInstNum, nSipLineInstNum, nCodecListInstNum,
					   data );
		return 1;
	} else if( strcmp( pszLastname, "SilenceSuppression" ) == 0 ) {
		mib_set_type3( MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__SILENCE_SUPPRESSION, 
					   nVoiceProfileInstNum, nSipLineInstNum, nCodecListInstNum,
					   data );
		return 1;
	} else if( strcmp( pszLastname, "Priority" ) == 0 ) {
		mib_set_type3( MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__PRIORITY, 
					   nVoiceProfileInstNum, nSipLineInstNum, nCodecListInstNum,
					   data );
		return 1;
	} else {
		return ERR_9005;
	}

	return 0;
}

static int objVoiceProfileLineCodec(char *name, struct sCWMP_ENTITY *entity, int type, void *data)
{
	switch( type ) {
	case eCWMP_tINITOBJ:
	{
		int num=0,i;
		struct sCWMP_ENTITY **c = (struct sCWMP_ENTITY **)data;
		//MIB_CE_IP_ROUTE_T *p,route_entity;

		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		num = 1;//mib_chain_total( MIB_IP_ROUTE_TBL );
		for( i=0; i<num;i++ )
		{
			//p = &route_entity;
			//if( !mib_chain_get( MIB_IP_ROUTE_TBL, i, (void*)p ) )
			//	continue;
			
			//if( p->InstanceNum==0 ) //maybe createn by web or cli
			//{
			//	MaxInstNum++;
			//	p->InstanceNum = MaxInstNum;
			//	mib_chain_update( MIB_IP_ROUTE_TBL, (unsigned char*)p, i );
			//}else
			//	MaxInstNum = p->InstanceNum;
			
			if( create_Object( c, tVoiceProfileLineCodecList, sizeof(tVoiceProfileLineCodecList), MAX_CODEC_LIST, 1 ) < 0 )
				return -1;
			//c = & (*c)->sibling;
		}
		add_objectNum( name, 1 );
		return 0;
	}
#if 0
	// TODO: Not implement add/del/update yet.  
	case eCWMP_tADDOBJ:
	     {
	     	int ret;
	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;
		ret = add_Object( name, &entity->next_table,  tForwarding, sizeof(tForwarding), data );
		if( ret >= 0 )
		{
			MIB_CE_IP_ROUTE_T fentry;
			memset( &fentry, 0, sizeof( MIB_CE_IP_ROUTE_T ) );
			fentry.InstanceNum = *(unsigned int*)data;
			fentry.FWMetric = -1;
			fentry.ifIndex = 0xff;
			mib_chain_add( MIB_IP_ROUTE_TBL, (unsigned char*)&fentry );
		}
		return ret;
	     }
	case eCWMP_tDELOBJ:
	     {
	     	int ret, id;
	     	
	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;
	     	
	     	id = getChainID( entity->next_table, *(int*)data  );
	     	if(id==-1) return ERR_9005;
	     	mib_chain_delete( MIB_IP_ROUTE_TBL, id );	
		ret = del_Object( name, &entity->next_table, *(int*)data );
		if( ret == 0 ) return 1;
		return ret;
	     }
	case eCWMP_tUPDATEOBJ:	
	     {     	int num=0,i;
	     	struct sCWMP_ENTITY *old_table;
	     	num = mib_chain_total( MIB_IP_ROUTE_TBL );
	     	old_table = entity->next_table;
	     	entity->next_table = NULL;
	     	for( i=0; i<num;i++ )
	     	{
	     		struct sCWMP_ENTITY *remove_entity=NULL;
			MIB_CE_IP_ROUTE_T *p,route_entity;

			p = &route_entity;
			if( !mib_chain_get( MIB_IP_ROUTE_TBL, i, (void*)p ) )
				continue;			
			remove_entity = remove_SiblingEntity( &old_table, p->InstanceNum );
			if( remove_entity!=NULL )
			{
				add_SiblingEntity( &entity->next_table, remove_entity );
			}else{ 
				unsigned int MaxInstNum=p->InstanceNum;			
				add_Object( name, &entity->next_table,  tForwarding,     							sizeof(tForwarding), &MaxInstNum );
				if(MaxInstNum!=p->InstanceNum)
				{     p->InstanceNum = MaxInstNum;
				      mib_chain_update( MIB_IP_ROUTE_TBL, (unsigned char*)p, i );
				}
			}	
	     	}
	     	if( old_table ) 	destroy_ParameterTable( old_table );	     	
	     	return 0;
	     }
#endif
	}
	return -1;
}

static int getVoiceProfileLineStatusEntity(char *name, struct sCWMP_ENTITY *entity, int *type, void **data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.SIP.Line.{i}.Stats. */
	unsigned int nVoiceProfileInstNum;
	unsigned int nSipLineInstNum;
	unsigned int nNumberOfLine;
	unsigned int chid;
	extern cwmpEvtMsg pEvtMsg;
	union {
		TstRtpRtcpStatistics rtpStatistics;
	} s;

	const char *pszLastname = entity ->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile_Line( name, &nVoiceProfileInstNum, &nSipLineInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	if( !mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &nNumberOfLine ) )
		return -1;
	
	if( -- nSipLineInstNum >= nNumberOfLine )	/* convert to zero based */
		return -1;
	
	*type = entity ->type;

	/*
	 * FIXME: Now, instance number of 'Line' is seens as channel ID,  
	 *        but specification indicate thta DirectoryNumber or PhyReferenceList 
	 *        is used to identify or associate with physical interface. 
	 */
	chid = nSipLineInstNum;

	if( strcmp( pszLastname, "PacketsSent" ) == 0 ) {
		if( rtk_GetRtpStatistics( chid, 0, &s.rtpStatistics ) )
			return ERR_9002;	/* Internal error */
		*data = uintdup( s.rtpStatistics.nTxRtpStatsCountPacket );
	} else if( strcmp( pszLastname, "PacketsReceived" ) == 0 ) {
		if( rtk_GetRtpStatistics( chid, 0, &s.rtpStatistics ) )
			return ERR_9002;	/* Internal error */
		*data = uintdup( s.rtpStatistics.nRxRtpStatsCountPacket );
	} else if( strcmp( pszLastname, "BytesSent" ) == 0 ) {
		if( rtk_GetRtpStatistics( chid, 0, &s.rtpStatistics ) )
			return ERR_9002;	/* Internal error */
		*data = uintdup( s.rtpStatistics.nTxRtpStatsCountByte );
	} else if( strcmp( pszLastname, "BytesReceived" ) == 0 ) {
		if( rtk_GetRtpStatistics( chid, 0, &s.rtpStatistics ) )
			return ERR_9002;	/* Internal error */
		*data = uintdup( s.rtpStatistics.nRxRtpStatsCountByte );
	} else if( strcmp( pszLastname, "PacketsLost" ) == 0 ) {
		if( rtk_GetRtpStatistics( chid, 0, &s.rtpStatistics ) )
			return ERR_9002;	/* Internal error */
		*data = uintdup( s.rtpStatistics.nRxRtpStatsLostPacket );
	} else if( strcmp( pszLastname, "IncomingCallsReceived" ) == 0 ) {
		*data = uintdup( pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].incomingCallsReceived);
		printf("@@@@@Line %d IncomingCallsReceived:%d\n",nVoiceProfileInstNum, pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].incomingCallsReceived);
	} else if( strcmp( pszLastname, "IncomingCallsAnswered" ) == 0 ) {
		*data = uintdup( pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].incomingCallsAnswered);
		printf("@@@@@Line %d IncomingCallsAnswered:%d\n",nVoiceProfileInstNum, pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].incomingCallsAnswered);
	} else if( strcmp( pszLastname, "IncomingCallsConnected" ) == 0 ) {
		*data = uintdup( pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].incomingCallsConnected);
		printf("@@@@@Line %d IncomingCallsConnected:%d\n",nVoiceProfileInstNum, pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].incomingCallsConnected);
	} else if( strcmp( pszLastname, "IncomingCallsFailed" ) == 0 ) {
		*data = uintdup( pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].incomingCallsFailed);
		printf("@@@@@Line %d IncomingCallsFailed:%d\n",nVoiceProfileInstNum, pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].incomingCallsFailed);
	} else if( strcmp( pszLastname, "OutgoingCallsAttempted" ) == 0 ) {
		*data = uintdup( pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].outgoingCallsAttempted);
		printf("@@@@@Line %d OutgoingCallsAttempted:%d\n",nVoiceProfileInstNum, pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].outgoingCallsAttempted);
	} else if( strcmp( pszLastname, "OutgoingCallsAnswered" ) == 0 ) {
		*data = uintdup( pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].outgoingCallsAnswered);
		printf("@@@@@Line %d OutgoingCallsAnswered:%d\n",nVoiceProfileInstNum, pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].outgoingCallsAnswered);
	} else if( strcmp( pszLastname, "OutgoingCallsConnected" ) == 0 ) {
		*data = uintdup( pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].outgoingCallsConnected);
		printf("@@@@@Line %d OutgoingCallsConnected:%d\n",nVoiceProfileInstNum, pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].outgoingCallsConnected);
	} else if( strcmp( pszLastname, "OutgoingCallsFailed" ) == 0 ) {
		*data = uintdup( pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].outgoingCallsFailed);
		printf("@@@@@Line %d OutgoingCallsFailed:%d\n",nVoiceProfileInstNum, pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].outgoingCallsFailed);
	} else {
		*data = NULL;
		return ERR_9005;
	}

	return 0;
}

static int setVoiceProfileLineStatusEntity(char *name, struct sCWMP_ENTITY *entity, int type, void *data)
{
	return ERR_9008;	/* Parameters are read only */
}

static int getVoiceProfileLineSipEntity(char *name, struct sCWMP_ENTITY *entity, int *type, void **data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.SIP.Line.{i}.SIP. */
	unsigned int nVoiceProfileInstNum;
	unsigned int nSipLineInstNum;
	unsigned int nNumberOfLine;
	union {
		char AuthUserName[ 128 ];
		char AuthPassword[ 128 ];
		char URI[ 389 ];
	} s;

	const char *pszLastname = entity ->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile_Line( name, &nVoiceProfileInstNum, &nSipLineInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	if( !mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &nNumberOfLine ) )
		return -1;
	
	if( -- nSipLineInstNum >= nNumberOfLine )	/* convert to zero based */
		return -1;
	
	*type = entity ->type;

	if( strcmp( pszLastname, "AuthUserName" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__SIP__AUTH_USER_NAME, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   &s.AuthUserName );
		*data = strdup( s.AuthUserName );
	} else if( strcmp( pszLastname, "AuthPassword" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__SIP__AUTH_PASSWORD, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   s.AuthPassword );
		*data = strdup( s.AuthPassword );
	} else if( strcmp( pszLastname, "URI" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__SIP__URI, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   &s.URI );
		*data = strdup( s.URI );
	} else {
		*data = NULL;
		return ERR_9005;
	}

	return 0;
}

static int setLineSipEntity(char *name, struct sCWMP_ENTITY *entity, int type, void *data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.SIP.Line.{i}.SIP. */
	unsigned int nVoiceProfileInstNum;
	unsigned int nSipLineInstNum;
	unsigned int nNumberOfLine;

	const char *pszLastname = entity ->name;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile_Line( name, &nVoiceProfileInstNum, &nSipLineInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	if( !mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &nNumberOfLine ) )
		return -1;
	
	if( -- nSipLineInstNum >= nNumberOfLine )	/* convert to zero based */
		return -1;

	if( entity ->type != type )
		return ERR_9006;

	if( strcmp( pszLastname, "AuthUserName" ) == 0 ) {
		mib_set_type2( MIB_VOICE_PROFILE__LINE__SIP__AUTH_USER_NAME, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   data );
		return 1;
	} else if( strcmp( pszLastname, "AuthPassword" ) == 0 ) {
		mib_set_type2( MIB_VOICE_PROFILE__LINE__SIP__AUTH_PASSWORD, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   data );
		return 1;
	} else if( strcmp( pszLastname, "URI" ) == 0 ) {
		mib_set_type2( MIB_VOICE_PROFILE__LINE__SIP__URI, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   data );
		return 1;
	} else {
		return ERR_9005;
	}

	return 0;
}

static int getVoiceProfileLineEntity(char *name, struct sCWMP_ENTITY *entity, int *type, void **data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.SIP.Line.{i}. */
	unsigned int nVoiceProfileInstNum;
	unsigned int nSipLineInstNum;
	unsigned int nNumberOfLine;
	union {
		enable_t Enable;
		char DirectoryNumber[ 32 ];
		line_status_t Status;
	} s;
	
	const char *pszLastname = entity ->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile_Line( name, &nVoiceProfileInstNum, &nSipLineInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	if( !mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &nNumberOfLine ) )
		return -1;
	
	if( -- nSipLineInstNum >= nNumberOfLine )	/* convert to zero based */
		return -1;
	
	*type = entity ->type;

	if( strcmp( pszLastname, "Enable" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__ENABLE, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   &s.Enable );
		*data = strdup_Enable( s.Enable );
	} else if( strcmp( pszLastname, "DirectoryNumber" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__DIRECTORY_NUMBER, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   s.DirectoryNumber );
		*data = strdup( s.DirectoryNumber );
	} else if( strcmp( pszLastname, "Status" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__STATUS, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   &s.Status );
		*data = strdup_LineStatus( s.Status );
	} else {
		*data = NULL;
		return ERR_9005;
	}

	return 0;
}

static int setVoiceProfileLineEntity(char *name, struct sCWMP_ENTITY *entity, int type, void *data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.SIP.Line.{i}. */
	unsigned int nVoiceProfileInstNum;
	unsigned int nSipLineInstNum;
	unsigned int nNumberOfLine;
	union {
		enable_t Enable;
		char DirectoryNumber[ 32 ];
		line_status_t Status;
	} s;
	
	const char *pszLastname = entity ->name;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile_Line( name, &nVoiceProfileInstNum, &nSipLineInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	if( !mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &nNumberOfLine ) )
		return -1;
	
	if( -- nSipLineInstNum >= nNumberOfLine )	/* convert to zero based */
		return -1;
	
	if( entity ->type != type )
		return ERR_9006;

	if( strcmp( pszLastname, "Enable" ) == 0 ) {
		str2id_Enable( data, &s.Enable );
		mib_set_type2( MIB_VOICE_PROFILE__LINE__ENABLE, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   &s.Enable );
		return 1;
	} else if( strcmp( pszLastname, "DirectoryNumber" ) == 0 ) {
		mib_set_type2( MIB_VOICE_PROFILE__LINE__DIRECTORY_NUMBER, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   data );
		return 1;
	} else if( strcmp( pszLastname, "Status" ) == 0 ) {
		return ERR_9008;
	} else {
		return ERR_9005;
	}

	return 0;
}

int objVoiceProfileLine(char *name, struct sCWMP_ENTITY *entity, int type, void *data)
{
	switch( type ) {
	case eCWMP_tINITOBJ:
	{
		int num=0,i;
		struct sCWMP_ENTITY **c = (struct sCWMP_ENTITY **)data;
		//MIB_CE_IP_ROUTE_T *p,route_entity;

		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		num = 1;//mib_chain_total( MIB_IP_ROUTE_TBL );
		for( i=0; i<num;i++ )
		{
			//p = &route_entity;
			//if( !mib_chain_get( MIB_IP_ROUTE_TBL, i, (void*)p ) )
			//	continue;
			
			//if( p->InstanceNum==0 ) //maybe createn by web or cli
			//{
			//	MaxInstNum++;
			//	p->InstanceNum = MaxInstNum;
			//	mib_chain_update( MIB_IP_ROUTE_TBL, (unsigned char*)p, i );
			//}else
			//	MaxInstNum = p->InstanceNum;
			
			if( create_Object( c, tVoiceProfileLine, sizeof(tVoiceProfileLine), MAX_LINE_PER_PROFILE, 1 ) < 0 )
				return -1;
			//c = & (*c)->sibling;
		}
		add_objectNum( name, 1 );
		return 0;
	}
#if 0
	// TODO: Not implement add/del/update yet.  
	case eCWMP_tADDOBJ:
	     {
	     	int ret;
	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;
		ret = add_Object( name, &entity->next_table,  tForwarding, sizeof(tForwarding), data );
		if( ret >= 0 )
		{
			MIB_CE_IP_ROUTE_T fentry;
			memset( &fentry, 0, sizeof( MIB_CE_IP_ROUTE_T ) );
			fentry.InstanceNum = *(unsigned int*)data;
			fentry.FWMetric = -1;
			fentry.ifIndex = 0xff;
			mib_chain_add( MIB_IP_ROUTE_TBL, (unsigned char*)&fentry );
		}
		return ret;
	     }
	case eCWMP_tDELOBJ:
	     {
	     	int ret, id;
	     	
	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;
	     	
	     	id = getChainID( entity->next_table, *(int*)data  );
	     	if(id==-1) return ERR_9005;
	     	mib_chain_delete( MIB_IP_ROUTE_TBL, id );	
		ret = del_Object( name, &entity->next_table, *(int*)data );
		if( ret == 0 ) return 1;
		return ret;
	     }
	case eCWMP_tUPDATEOBJ:	
	     {     	int num=0,i;
	     	struct sCWMP_ENTITY *old_table;
	     	num = mib_chain_total( MIB_IP_ROUTE_TBL );
	     	old_table = entity->next_table;
	     	entity->next_table = NULL;
	     	for( i=0; i<num;i++ )
	     	{
	     		struct sCWMP_ENTITY *remove_entity=NULL;
			MIB_CE_IP_ROUTE_T *p,route_entity;

			p = &route_entity;
			if( !mib_chain_get( MIB_IP_ROUTE_TBL, i, (void*)p ) )
				continue;			
			remove_entity = remove_SiblingEntity( &old_table, p->InstanceNum );
			if( remove_entity!=NULL )
			{
				add_SiblingEntity( &entity->next_table, remove_entity );
			}else{ 
				unsigned int MaxInstNum=p->InstanceNum;			
				add_Object( name, &entity->next_table,  tForwarding,     							sizeof(tForwarding), &MaxInstNum );
				if(MaxInstNum!=p->InstanceNum)
				{     p->InstanceNum = MaxInstNum;
				      mib_chain_update( MIB_IP_ROUTE_TBL, (unsigned char*)p, i );
				}
			}	
	     	}
	     	if( old_table ) 	destroy_ParameterTable( old_table );	     	
	     	return 0;
	     }
#endif
	}
	return -1;
}
#endif
