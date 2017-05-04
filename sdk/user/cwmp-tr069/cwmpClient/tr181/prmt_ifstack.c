#include <stdio.h>
#include <stdlib.h>

#include <config/autoconf.h>
#include "prmt_ip_if.h"
#include "prmt_ifstack.h"

/****** Device.InterfaceStack.{i} ********************************************/
struct CWMP_OP tIfStackEntityOP = { NULL, objIfStack};
struct CWMP_OP tIfStackEntityLeafOP = { getIfStackEntity, NULL };

struct CWMP_PRMT tIfStackEntityLeafInfo[] =
{
/*(name,		type,		flag,			op)*/
{"HigherLayer",		eCWMP_tSTRING,	CWMP_READ,	&tIfStackEntityLeafOP},
{"LowerLayer",		eCWMP_tSTRING,	CWMP_READ,	&tIfStackEntityLeafOP},
};

enum eIfStackEntityLeaf
{
	eHigherLayer,
	eLowerLayer,
};

struct CWMP_LEAF tIfStackEntityLeaf[] =
{
{ &tIfStackEntityLeafInfo[eHigherLayer]  },
{ &tIfStackEntityLeafInfo[eLowerLayer]  },
{ NULL	}
};

/****** Device.Ethernet.Interface *********************************************/
struct CWMP_LINKNODE *gIfStackObjList = NULL;

struct CWMP_PRMT tIfStackObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"0",			eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL},
};

enum eIfStackObject
{
	eIfStack0,
};

struct CWMP_LINKNODE tIfStackObject[] =
{
/*info, 						leaf,				next,				sibling,	instnum)*/
{&tIfStackObjectInfo[eIfStack0], tIfStackEntityLeaf, NULL, NULL, 0},
};

/***** Utility Functions *****************************************************/
static int is_osi_layer1(char *path)
{
	if(strstr(path, "Device.DSL.Line")
		|| strstr(path, "Ethernet.Interface")
		|| strstr(path, "USB.Interface")
		|| strstr (path, "WiFi.Radio"))
		return 1;

	return 0;
}

static int is_duplicated_entity(const char *higher, const char *lower)
{
	struct CWMP_LINKNODE *node = gIfStackObjList;
	struct IfStackObjData *data = NULL;

	while(node)
	{
		data = (struct IfStackObjData *)node->obj_data;

		if(strcmp(data->higherLayer, higher) == 0
			&& strcmp(data->lowerLayer, lower) == 0)
			return 1;

		node = node->sibling;
	}

	return 0;
}

void free_if_stack_obj_data(void *obj_data)
{
	struct IfStackObjData *data = (struct IfStackObjData *)obj_data;

	if(data)
	{
		if(data->higherLayer)
			free(data->higherLayer);

		if(data->lowerLayer)
			free(data->lowerLayer);

		free(data);
	}
}

static int is_multiple_lowers(char *path)
{
	if(strstr(path, "Bridging.Bridge.1.Port.1"))
		return 1;
	else return 0;
}

//Create Device.InterfaceStack objects recursively
static int create_if_stack_list(char *higher, int *instNum, struct CWMP_LINKNODE **head, struct CWMP_LINKNODE **tail, int multiple_lower)
{
	struct IfStackObjData *data = NULL;
	char *lower = NULL;
	char prmt_lower_name[1024] = {0};
	int vtype;
	void *vdata;
	char *saveptr, *token;
	int err = 0;

	// Layer 1 interfaces have no lower layer,
	// success, but return an empty list;
	if(is_osi_layer1(higher))
		return 0;

	snprintf(prmt_lower_name, 1024, "%s.LowerLayers", higher);

	err = get_ParameterValue(prmt_lower_name, &vtype, &vdata);
	if(vdata == NULL)
	{
		fprintf(stderr, "<%s:%d> Get %s failed, err_no:%d\n", __FUNCTION__, __LINE__, prmt_lower_name, err);
		return -1;
	}

	lower = (char *)vdata;

	// No lower layer found and not layer 1,
	// let caller free collected nodes.
	if(strcmp(lower, "") == 0)
	{
		get_ParameterValueFree(vtype, vdata);
		return -1;
	}

	token = strtok_r(lower, ",", &saveptr);
	while(token)
	{
		struct CWMP_LINKNODE *node = NULL;
		struct CWMP_LINKNODE *tmp_head = NULL;
		struct CWMP_LINKNODE *tmp_tail = NULL;

		if(is_duplicated_entity(higher, token))
		{
			//get next lower layer
			token = strtok_r(NULL, ",", &saveptr);
			continue;
		}
			
		data = malloc(sizeof(struct IfStackObjData));
		if(data == NULL)
		{
			fprintf(stderr, "<%s:%d> malloc failed\n", __FUNCTION__, __LINE__);
			get_ParameterValueFree(vtype, vdata);
			return -1;
		}

		data->higherLayer = strdup(higher);
		data->lowerLayer = strdup(token);

		create_Object(&node, tIfStackObject, sizeof(tIfStackObject), 1, *instNum);
		node->obj_data = (void *)data;
		node->free_obj_data = free_if_stack_obj_data;

		add_SiblingEntity(head, node);
		*tail = node;
		(*instNum)++;

		if(create_if_stack_list(token, instNum, &tmp_head, &tmp_tail, is_multiple_lowers(token)) == 0)
		{
			if(tmp_head != NULL && tmp_tail != NULL)
			{
				(*tail)->sibling = tmp_head;
				*tail = tmp_tail;
			}
		}
		else
		{
			if(!multiple_lower)
			{
				*instNum -= ParameterEntityCount((struct CWMP_LEAF *)*head);
				destroy_ParameterTable((struct CWMP_NODE *)*head);
				*head = NULL;
				*tail = NULL;
			}
			return -1;
		}

		//get next lower layer
		token = strtok_r(NULL, ",", &saveptr);
	}
	get_ParameterValueFree(vtype, vdata);

	return 0;
}


/***** Operations ************************************************************/
int getIfStackEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	struct CWMP_LINKNODE *node = NULL;
	struct IfStackObjData *obj_data = NULL;
	char	*lastname = entity->info->name;
	unsigned int num;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	num = getInstNum(name, "InterfaceStack");
	
	node = find_SiblingEntity(&gIfStackObjList, num);

	if(node == NULL)
		return ERR_9005;

	obj_data = node->obj_data;
	if(obj_data == NULL)
	{
		CWMPDBG(1, (stderr, "<%s:%d> FIXME: obj_data should not be NULL\n"));
		return ERR_9002;
	}

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "HigherLayer" )==0 )
	{
		*data = strdup(obj_data->higherLayer);
	}
	else if( strcmp( lastname, "LowerLayer" )==0 )
	{
		*data = strdup( obj_data->lowerLayer);
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int objIfStack(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;

	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);
	
	switch( type )
	{
	case eCWMP_tINITOBJ:
		return 0;
	case eCWMP_tADDOBJ:
	case eCWMP_tDELOBJ:
		return ERR_9001;
	case eCWMP_tUPDATEOBJ:
		{
			int total_wan, i;
			struct CWMP_LINKNODE *ip_if_node = gIPIfEntityObjList;
			struct CWMP_LINKNODE *list_tail = NULL;
			unsigned int instNum = 1;
			struct node *if_stack_list = NULL;

			if( entity->next )
				destroy_ParameterTable(entity->next);
			entity->next = NULL;
			gIfStackObjList = NULL;

			//Handle WAN
			while (ip_if_node)
			{
				char prmt_ip_if[256] = {0};
				struct CWMP_LINKNODE *tmp_list_head = NULL;
				struct CWMP_LINKNODE *tmp_list_tail = NULL;

				snprintf(prmt_ip_if, 1024, "Device.IP.Interface.%d", ip_if_node->instnum);
				if(create_if_stack_list(prmt_ip_if, &instNum, &tmp_list_head, &tmp_list_tail, 0) == 0)
				{
					// success
					if(entity->next == NULL)
					{
						entity->next = (struct CWMP_NODE *)tmp_list_head;
						gIfStackObjList = tmp_list_head;
					}
					if(list_tail != NULL)
						list_tail->sibling = tmp_list_head;
						
					list_tail = tmp_list_tail;
				}
				else
				{
					printf("Failed on Device.IP.Interface.%d\n", ip_if_node->instnum);
				}

				// get next IP.Interface object
				ip_if_node = ip_if_node->sibling;
			}

			return 0;
		}
	}
	
	return -1;
}


