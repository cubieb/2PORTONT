#include "prmt_limit.h"
#include "prmt_line_codec_list.h"
#include "prmt_capabilities.h"	/* for lstCodecs */
#include "mib_def.h"
#include "mib_tr104.h"
#include "str_mib.h"
#include "str_utility.h"


//******* VoiceService.{i}.VoiceProfile.{i}.Line.{i}.Codec.List.{i} EntityLEAF*******
struct CWMP_OP tLineCodecListEntityLeafOP = { getLineCodecListEntity, setLineCodecListEntity};

struct CWMP_PRMT tLineCodecListEntityLeafInfo[] =
{
    /*(name,                  type,           flag,                 op)*/
    { "EntryID",              eCWMP_tUINT,    CWMP_READ,            &tLineCodecListEntityLeafOP},
    { "Codec",                eCWMP_tSTRING,  CWMP_READ,            &tLineCodecListEntityLeafOP},
    { "BitRate",              eCWMP_tUINT,    CWMP_READ,            &tLineCodecListEntityLeafOP},
    { "PacketizationPeriod",  eCWMP_tSTRING,  CWMP_READ|CWMP_WRITE, &tLineCodecListEntityLeafOP},
    { "SilenceSuppression",   eCWMP_tBOOLEAN, CWMP_READ|CWMP_WRITE, &tLineCodecListEntityLeafOP},
    { "Priority",             eCWMP_tUINT,    CWMP_READ|CWMP_WRITE, &tLineCodecListEntityLeafOP}
};

enum eLineCodecListEntityLeaf
{
    eListEntryID,
    eListCodec,
    eListBitRate,
    eListPacketizationPeriod,
    eListSilenceSuppression,
    eListPriority
};

struct CWMP_LEAF tLineCodecListEntityLeaf[] =
{
    { &tLineCodecListEntityLeafInfo[eListEntryID]  },
    { &tLineCodecListEntityLeafInfo[eListCodec]  },
    { &tLineCodecListEntityLeafInfo[eListBitRate]  },
    { &tLineCodecListEntityLeafInfo[eListPacketizationPeriod]  },
    { &tLineCodecListEntityLeafInfo[eListSilenceSuppression]  },
    { &tLineCodecListEntityLeafInfo[eListPriority]  },
    { NULL }
};


//******* VoiceService.{i}.VoiceProfile.{i}.Line.{i}.Codec.List.{i}  *******
struct CWMP_PRMT tLineCodecListObjectInfo[] =
{
    /*(name,  type,           flag,		               op)*/
    {"0",     eCWMP_tOBJECT,  CWMP_READ|CWMP_LNKLIST,  NULL}
};
enum eLineCodecListObject
{
    eVPLineCodecList0
};
struct CWMP_LINKNODE tLineCodecListObject[] =
{
    /*info,  			                            leaf,                      next,  sibling,  instnum)*/
    {&tLineCodecListObjectInfo[eVPLineCodecList0],  tLineCodecListEntityLeaf,  NULL,  NULL,     0}
};

int objLineCodecList(char *name, struct CWMP_LEAF *e, int type, void *data)
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
			if( create_Object( ptable, tLineCodecListObject, sizeof(tLineCodecListObject), num, 1) < 0 )
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
                        tLineCodecListObject, sizeof(tLineCodecListObject), &MaxInstNum );
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

int getLineCodecListEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
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

	const char *pszLastname = entity->info->name;

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
	
	*type = entity->info->type;

    CWMPDBG( 3, ( stderr, "<%s:%d> get %s value \n", __FUNCTION__, __LINE__, name) );

	if( strcmp( pszLastname, "EntryID" ) == 0 ) {
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

int setLineCodecListEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.Line.{i}.Codec.List.{i}. */
    int res = 0;
	unsigned int nVoiceProfileInstNum;
	unsigned int nSipLineInstNum;
	unsigned int nCodecListInstNum;
	unsigned int nNumberOfLine;

	const char *pszLastname = entity->info->name;

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

	if( entity->info->type != type )
		return ERR_9006;

    CWMPDBG( 3, ( stderr, "<%s:%d> set %s value \n", __FUNCTION__, __LINE__, name) );

	if( strcmp( pszLastname, "EntryID" ) == 0 ) {
		return ERR_9008;
	} else if( strcmp( pszLastname, "Codec" ) == 0 ) {
		return ERR_9008;
	} else if( strcmp( pszLastname, "BitRate" ) == 0 ) {
		return ERR_9008;
	} else if( strcmp( pszLastname, "PacketizationPeriod" ) == 0 ) {
		res = mib_set_type3( MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__PACKETIZATION_PERIOD, 
					   nVoiceProfileInstNum, nSipLineInstNum, nCodecListInstNum,
					   data );
	} else if( strcmp( pszLastname, "SilenceSuppression" ) == 0 ) {
		res = mib_set_type3( MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__SILENCE_SUPPRESSION, 
					   nVoiceProfileInstNum, nSipLineInstNum, nCodecListInstNum,
					   data );
	} else if( strcmp( pszLastname, "Priority" ) == 0 ) {
		res = mib_set_type3( MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__PRIORITY, 
					   nVoiceProfileInstNum, nSipLineInstNum, nCodecListInstNum,
					   data );
	} else {
		return ERR_9005;
	}

	return res;
}


