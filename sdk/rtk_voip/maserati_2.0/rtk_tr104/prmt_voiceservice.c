#include "prmt_limit.h"
#include "prmt_voiceservice.h"
#include "prmt_capabilities.h"
#include "prmt_voice_profile.h"
#include "prmt_phyinterface.h"

//******* .VoiceService.{i}. EntityLEAF*******
struct CWMP_OP tVSEntityOP = { getVSEntity, NULL };

struct CWMP_PRMT tVoiceServiceEntityLeafInfo[] =
{
    /*(name,                        type,         flag,       op)*/
   {"VoiceProfileNumberOfEntries",  eCWMP_tUINT,  CWMP_READ,  &tVSEntityOP}
};

enum eVoiceServiceEntityLeaf
{
    eVoiceProfileNumberOfEntries
};

struct CWMP_LEAF tVoiceServiceEntityLeaf[] =
{
    { &tVoiceServiceEntityLeafInfo[eVoiceProfileNumberOfEntries]  },
    { NULL }
};

//******* .VoiceService.{i}. EntityObject*******
struct CWMP_OP tVoiceProfileOP = { NULL, objVoiceProfile };
struct CWMP_OP tPhyInterfaceOP = { NULL, objPhyInterface };
struct CWMP_PRMT tVoiceServiceEntityObjectInfo[] =
{
    /*(name,          type,           flag,       op)*/
    {"Capabilities",  eCWMP_tOBJECT,  CWMP_READ,  NULL},
    {"VoiceProfile",  eCWMP_tOBJECT,  CWMP_READ,  &tVoiceProfileOP},
    {"PhyInterface",  eCWMP_tOBJECT,  CWMP_READ,  &tPhyInterfaceOP}
};
enum eVoiceServiceEntityObject
{
	eVSCapabilities,
	eVSVoiceProfile,
	eVSPhyInterface
};
struct CWMP_NODE tVoiceServiceEntityObject[] =
{
    /*info,                                             leaf,                 next)*/
    {&tVoiceServiceEntityObjectInfo[eVSCapabilities],   tVSCapabilitiesLeaf,  tVSCapabilitiesObject},
    {&tVoiceServiceEntityObjectInfo[eVSVoiceProfile],   NULL,                 NULL},
    {&tVoiceServiceEntityObjectInfo[eVSPhyInterface],   NULL,                 NULL},    
    {NULL,                                              NULL,                 NULL}
};

// ******* .VoiceService. Object******* 
struct CWMP_PRMT tVoiceServiceObjectInfo[] =
{
    /*(name,  type,           flag,                    op)*/
    {"0",     eCWMP_tOBJECT,  CWMP_READ|CWMP_LNKLIST,  NULL},
};

enum eVoiceServiceObject
{
	eVoiceService0
};

struct CWMP_LINKNODE tVoiceServiceObject[] =
{
    /*info,                                     leaf,                     next,                       sibling, instnum)*/
    {&tVoiceServiceObjectInfo[eVoiceService0],  tVoiceServiceEntityLeaf,  tVoiceServiceEntityObject,  NULL,    0},
};


int objVoiceService(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	/* .VoiceService. */
    struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
    
    switch( type )
    {
    case eCWMP_tINITOBJ:
         {
            int num=0;
            struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;
     
            if( (name==NULL) || (entity==NULL) || (data==NULL) ) 
              return -1;

            num = MAX_VOICE_SERVICE_COUNT;	

            if( create_Object( c, tVoiceServiceObject, sizeof(tVoiceServiceObject), 1, 1 ) < 0 ){
                return -1;
            }
            add_objectNum( name, num );
            return 0;
         }
            break;
	case eCWMP_tADDOBJ:
	case eCWMP_tDELOBJ:
         {
            CWMPDBG( 0, ( stderr, "<%s:%d> type %d unsupport now \n", __FUNCTION__, __LINE__, type) );
            return 0;
         }
            break;      
    case eCWMP_tUPDATEOBJ:
         {
            int num=0,i;
            struct CWMP_LINKNODE *old_table;
            
            num = MAX_VOICE_SERVICE_COUNT;
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
                        tVoiceServiceObject, sizeof(tVoiceServiceObject), (unsigned int*)&num );
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


int getVSEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;
    
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;

    CWMPDBG( 3, ( stderr, "<%s:%d> get %s value \n", __FUNCTION__, __LINE__, name) );
    
	if( strcmp( lastname, "VoiceProfileNumberOfEntries" )==0 )
	{
		*data = uintdup( MAX_PROFILE_COUNT );
	} else{
		return ERR_9005;
	}
	
	return 0;

}

#if 0
static int getVSEntity(char *name, struct sCWMP_ENTITY *entity, int *type, void **data);
struct sCWMP_ENTITY tVoiceServiceEntity[] = {
/*	{ name,								type,			flag,		accesslist,	getvalue,		setvalue,		next_table,				sibling } */
	{ "VoiceProfileNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,	NULL,		getVSEntity, 	NULL,			NULL,					NULL },
	{ "Capabilities",					eCWMP_tOBJECT,	CWMP_READ,	NULL,		NULL,			NULL,			tCapabilitiesEntity,	NULL },
	{ "VoiceProfile",					eCWMP_tOBJECT,	CWMP_READ,	NULL,		NULL,			objVoiceProfile,NULL,					NULL },
	{ "",								eCWMP_tNONE,	0,			NULL,		NULL,			NULL,			NULL,					NULL },
};

struct sCWMP_ENTITY tVoiceService[] = {
/*	{ name,		type,			flag,								accesslist,	getvalue,	setvalue,	next_table,				sibling } */
	{ "0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL,		NULL,		NULL,		tVoiceServiceEntity, 	NULL },
};

int objVoiceService(char *name, struct sCWMP_ENTITY *entity, int type, void *data)
{

	switch( type ) {
	case eCWMP_tINITOBJ:
	{
		int num=0;
		struct sCWMP_ENTITY **c = (struct sCWMP_ENTITY **)data;

		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		num = 1;	/* Now, we has one voice service only. */

                //create_Object( struct CWMP_LINKNODE **table, struct CWMP_LINKNODE ori_table[], int size, unsigned int num, unsigned int from);
        	if( create_Object( c, tVoiceService, sizeof(tVoiceService), 1, 1 ) < 0 )
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

static int getVSEntity(char *name, struct sCWMP_ENTITY *entity, int *type, void **data)
{
	*type = entity->type;
	*data = uintdup( MAX_PROFILE_COUNT );
	return 0;
}
#endif

