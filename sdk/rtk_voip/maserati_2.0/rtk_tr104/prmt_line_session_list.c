#include "prmt_limit.h"
#include "prmt_line_session_list.h"
#include "mib_def.h"
#include "mib_tr104.h"
#include "str_mib.h"
#include "str_utility.h"


//******* VoiceService.{i}.VoiceProfile.{i}.Line.{i}Session.{i} EntityLEAF*******
struct CWMP_OP tLineSessionListEntityLeafOP = { getLineSessionListEntity, setLineSessionListEntity};

struct CWMP_PRMT tLineSessionListEntityLeafInfo[] =
{
    /*(name,                  type,           flag,                 op)*/
    { "SessionStartTime",	eCWMP_tDATETIME,	CWMP_READ,		&tLineSessionListEntityLeafOP},
    { "SessionDuration",		eCWMP_tUINT,		CWMP_READ,		&tLineSessionListEntityLeafOP},
    { "FarEndIPAddress",		eCWMP_tSTRING,		CWMP_READ,		&tLineSessionListEntityLeafOP},
    { "FarEndUDPPort",		eCWMP_tUINT,  		CWMP_READ,		&tLineSessionListEntityLeafOP},
    { "LocalUDPPort",		eCWMP_tUINT, 		CWMP_READ,		&tLineSessionListEntityLeafOP}
};

enum eLineSessionListEntityLeaf
{
    eListSessionStartTime,
    eListSessionDuration,
    eListFarEndIPAddress,
    eListFarEndUDPPort,
    eListLocalUDPPort
};

struct CWMP_LEAF tLineSessionListEntityLeaf[] =
{
    { &tLineSessionListEntityLeafInfo[eListSessionStartTime]  },
    { &tLineSessionListEntityLeafInfo[eListSessionDuration]  },
    { &tLineSessionListEntityLeafInfo[eListFarEndIPAddress]  },
    { &tLineSessionListEntityLeafInfo[eListFarEndUDPPort]  },
    { &tLineSessionListEntityLeafInfo[eListLocalUDPPort]  },
    { NULL }
};


//******* VoiceService.{i}.VoiceProfile.{i}.Line.{i}.Session.{i}  *******
struct CWMP_PRMT tLineSessionListObjectInfo[] =
{
    /*(name,  type,           flag,		               op)*/
    {"0",     eCWMP_tOBJECT,  CWMP_READ|CWMP_LNKLIST,  NULL}
};
enum eLineSessionListObject
{
    eVPLineSessionList0
};
struct CWMP_LINKNODE tLineSessionListObject[] =
{
    /*info,  			                            leaf,                      next,  sibling,  instnum)*/
    {&tLineSessionListObjectInfo[eVPLineSessionList0],  tLineSessionListEntityLeaf,  NULL,  NULL,     0}
};







int objLineSessionList(char *name, struct CWMP_LEAF *e, int type, void *data)
{
    struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
    
	switch( type ) {
	case eCWMP_tINITOBJ:
	{
		int num=0,i;
		struct CWMP_LINKNODE **ptable = (struct CWMP_LINKNODE **)data;

		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		num = PROFILE_MAX_SESSION;	
		for( i=1; i<=num; i++ )
		{
			if( create_Object( ptable, tLineSessionListObject, sizeof(tLineSessionListObject), num, 1) < 0 )
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
            
            num = PROFILE_MAX_SESSION;
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
                        tLineSessionListObject, sizeof(tLineSessionListObject), &MaxInstNum );
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
int getLineSessionListEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.SIP.Line.{i}.Session.{i}. */
	unsigned int nVoiceProfileInstNum;
	unsigned int nSipLineInstNum;
	unsigned int nSessionListInstNum;
	unsigned int nNumberOfLine;
	union {
		unsigned int SessionDuration;
		char FarEndIPAddress[ 256 ];
		unsigned int FarEndUDPPort;
		unsigned int LocalUDPPort;
	} s;

	const char *pszLastname = entity->info->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
	
	if( !GetOneBasedInstanceNumber_VoiceProfile_Line_Session( name, &nVoiceProfileInstNum, &nSipLineInstNum, &nSessionListInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	if( !mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &nNumberOfLine ) )
		return -1;
	
	if( -- nSipLineInstNum >= nNumberOfLine )	/* convert to zero based */
		return -1;
	
	if( -- nSessionListInstNum >= PROFILE_MAX_SESSION )	/* convert to zero based */
		return -1;
	
	*type = entity->info->type;

    CWMPDBG( 3, ( stderr, "<%s:%d> get %s value \n", __FUNCTION__, __LINE__, name) );

	if( strcmp( pszLastname, "SessionStartTime" ) == 0 ) {
		*data=timedup( 1 );
	} else if( strcmp( pszLastname, "SessionDuration" ) == 0 ) {
		mib_get_type3( MIB_VOICE_PROFILE__LINE__SESSION__SESSIONDURATION, 
					   nVoiceProfileInstNum, nSipLineInstNum, nSessionListInstNum,
					   &s.SessionDuration );
		*data = uintdup( s.SessionDuration);
	} else if( strcmp( pszLastname, "FarEndIPAddress" ) == 0 ) {
		mib_get_type3( MIB_VOICE_PROFILE__LINE__SESSION__FARENDIPADDRESS, 
					   nVoiceProfileInstNum, nSipLineInstNum, nSessionListInstNum,
					   &s.FarEndIPAddress );
		*data = strdup(s.FarEndIPAddress);
	} else if( strcmp( pszLastname, "FarEndUDPPort" ) == 0 ) {
		mib_get_type3( MIB_VOICE_PROFILE__LINE__SESSION__FARENDUDPPORT, 
					   nVoiceProfileInstNum, nSipLineInstNum, nSessionListInstNum,
					   &s.FarEndUDPPort );
		*data = uintdup( s.FarEndUDPPort );
	} else if( strcmp( pszLastname, "LocalUDPPort" ) == 0 ) {
		mib_get_type3( MIB_VOICE_PROFILE__LINE__SESSION__LOCALUDPPORT, 
					   nVoiceProfileInstNum, nSipLineInstNum, nSessionListInstNum,
					   &s.LocalUDPPort );
		*data = uintdup( s.LocalUDPPort );
	} else {
		*data = NULL;
		return ERR_9005;
	}

	return 0;
}

int setLineSessionListEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
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
	
	if( -- nCodecListInstNum >= PROFILE_MAX_SESSION )	/* convert to zero based */
		return -1;

	if( entity->info->type != type )
		return ERR_9006;

    CWMPDBG( 3, ( stderr, "<%s:%d> set %s value \n", __FUNCTION__, __LINE__, name) );

	if( strcmp( pszLastname, "SessionStartTime" ) == 0 ) {
		return ERR_9008;
	} else if( strcmp( pszLastname, "SessionDuration" ) == 0 ) {
		return ERR_9008;
	} else if( strcmp( pszLastname, "FarEndIPAddress" ) == 0 ) {
		return ERR_9008;
	} else if( strcmp( pszLastname, "FarEndUDPPort" ) == 0 ) {
		return ERR_9008;
	} else if( strcmp( pszLastname, "LocalUDPPort" ) == 0 ) {
		return ERR_9008;
	} else {
		return ERR_9005;
	}

	return res;
}


