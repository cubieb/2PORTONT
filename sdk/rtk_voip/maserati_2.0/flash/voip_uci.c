#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <signal.h>
#include <errno.h>
#include "voip_flash.h"
#include "voip_types.h"
#include "voip_params.h"
#include "voip_control.h"
#include "voip_feature.h"
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "voip_flash_mib.h"

#include "voip_flash_client.h"

#include "voip_ioctl.h"
//support uci
#include <uci.h>
#include "voip_default.h"


#if 1
typedef struct voip_option
{
	const char *name;
	int type;
	int offset;
	int size;
}voip_option_t;
#define _OFFSET_CE(type, field)		((int)(long *)&(((type *)0)->field))
#define _SIZE_CE(type, field)		sizeof(((type *)0)->field)
#undef MIBDEF
#undef MIBARY
#undef MIBLST
#undef MIBSTR
#define _VOIP_DESC(name) DESC_##name


#define MIBDEF(_ctype,  _cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{#_mib_name,  _mib_type,  _OFFSET_CE(_mib_parents_ctype, _cname),  _SIZE_CE(_mib_parents_ctype, _cname) },

#define MIBARY(_ctype,  _cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{#_mib_name,  _mib_type,  _OFFSET_CE(_mib_parents_ctype, _cname),  _SIZE_CE(_mib_parents_ctype, _cname) }, 

#define MIBLST(_ctype,  _cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{#_mib_name,  _mib_type,  _OFFSET_CE(_mib_parents_ctype, _cname),  _SIZE_CE(_mib_parents_ctype, _cname)},  

#define MIBSTR(_ctype,  _cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
	{#_mib_name,  _mib_type,  _OFFSET_CE(_mib_parents_ctype, _cname),  _SIZE_CE(_mib_parents_ctype, _cname)},
	
voip_option_t voipCfgParam_table[]={

#define MIB_VOIP_CFG_IMPORT
#include "voip_mibdef.h"
#undef MIB_VOIP_CFG_IMPORT
{ }
};
voip_option_t voipCfgPortParam_table[]={

#define MIB_VOIP_PORT_IMPORT
#include "voip_mibdef.h"
#undef MIB_VOIP_PORT_IMPORT
{ }
};
voip_option_t voipCfgProxy_table[]={

#define MIB_VOIP_PROXY_IMPORT
#include "voip_mibdef.h"
#undef MIB_VOIP_PROXY_IMPORT
{ }
};
voip_option_t SpeedDialCfg_table[]={

#define MIB_VOIP_SPEED_DIAL_IMPORT
#include "voip_mibdef.h"
#undef MIB_VOIP_SPEED_DIAL_IMPORT
{ }
};
voip_option_t abbrDialCfg_table[]={

#define MIB_VOIP_ABBR_DIAL_IMPORT
#include "voip_mibdef.h"
#undef MIB_VOIP_ABBR_DIAL_IMPORT
{ }
};

voip_option_t st_ToneCfgParam_table[]={

#define MIB_VOIP_TONE_IMPORT
#include "voip_mibdef.h"
#undef MIB_VOIP_TONE_IMPORT
{ }
};
bool
parse_unsigned_char(void *ptr, const char *val)
{

	unsigned  char n = atoi(val);
	*((unsigned char *)ptr) = n;

	return true;
}
bool
parse_unsigned_int(void *ptr, const char *val)
{

	unsigned  int n = atoi(val);
	*((unsigned int *)ptr) = n;

	return true;
}
bool
parse_unsigned_long_int(void *ptr, const char *val)
{
	char *e;
	unsigned long int n = strtoul(val, &e, 0);

	if (e == val || *e)
		return false;

	*((unsigned long int *)ptr) = n;

	return true;
}
bool
parse_unsigned_short_int(void *ptr, const char *val)
{

	unsigned short int n =atoi(val); 

	*((unsigned short int *)ptr) = n;

	return true;
}
bool
parse_string(void *ptr, const char *val, int size)
{
	//*(( char **)ptr) = ( char *)val;
	//strncpy(ptr, val, strlen(val));
	strncpy(ptr, val, size);
	return true;
}
void voip_parse_option(void *s, const struct voip_option *opts, struct uci_section *section){
	
	struct voip_option *op=NULL;
	struct uci_element *option_e= NULL;
	struct uci_option *o= NULL;
	char *value=NULL;
	
	uci_foreach_element(&section->options, option_e)
	{
		o = uci_to_option(option_e);

		
		for (op = opts; op->name; op++)
		{
			

			//printf("voip_optons_table=%s option_e->name=%s\n",  op->name, option_e->name );
			if (strcmp(op->name, option_e->name))
				continue;

				
			if (o->type == UCI_TYPE_STRING)
			{

				switch (op->type)
				{
					case V_DWORD:
					
						if(!(parse_unsigned_long_int((char *)s + op->offset,o->v.string)))
						{
							fprintf(stdout, "V_DWORD parse error\n");
						}
						else
							printf("option name=%s value=%s\n",  option_e->name, o->v.string );
					
						break;
					case V_WORD:
						if(!(parse_unsigned_short_int((char *)s + op->offset,o->v.string )))
						{
							fprintf(stdout, "V_WORD parse error\n");
						}
						else
							printf("option name=%s value=%s\n",  option_e->name, o->v.string );
						break;
					case V_UINT:	
						if(!(parse_unsigned_int((char *)s + op->offset,o->v.string )))
						{
							fprintf(stdout, "V_UINT parse error\n");
						}
						else
							printf("option name=%s value=%s\n",  option_e->name, o->v.string );
						break;
					case V_BYTE:
						if(!(parse_unsigned_char((char *)s + op->offset,o->v.string )))
						{
							fprintf(stdout, "V_BYTE parse error\n");
						}
						else
							printf("option name=%s value=%s\n",  option_e->name, o->v.string );
						break;

					case V_STRING:
						if(!(parse_string((char *)s + op->offset,o->v.string , op->size)))
						{
							fprintf(stdout, "V_STRING parse error\n");
						}
						else
							fprintf(stdout ,"option name=%s value=%s\n",  option_e->name, o->v.string );

						break;

					default:
						break;
				}
				//printf("option name=%s value=%s\n",  option_e->name, o->v.string );
			}
		}
	}
}
void parse_voipSectionParam(void *Cfg, struct uci_package * p,voip_option_t* option_table, char* section_name){
	
	struct uci_section *s = NULL;	
	struct uci_element *section_e= NULL;
	//printf("section_name=%s\n", section_name);
	uci_foreach_element(&p->sections, section_e)
	{
		s = uci_to_section(section_e);
		if (strcmp(section_e->name, section_name))
			continue;
		
		voip_parse_option(Cfg, option_table,s );
	}
}
void voip_uci_config_get( voipCfgParam_t *pVoIPCfg )
{
	voipCfgPortParam_t* portCfg=NULL;


	const VoipFeature_t feature = g_VoIP_Feature;
	
	int voip_port ,proxy,speedDial,abbrDial, tone;
	char port_section_name[64]; 
	char proxy_section_name[64];
	char speedDial_section_name[64];
	char abbrDial_section_name[64];
	char tone_section_name[64];
	struct uci_context *uci= NULL;
	struct uci_package *p = NULL;

	memset(pVoIPCfg,0,(sizeof(voipCfgParam_t)));
	memcpy(pVoIPCfg, &voipCfgParamDefault, sizeof(voipCfgParam_t));

	pVoIPCfg->feature = VOIP_SYSTEM_2_FLASH_FEATURE( feature );
	pVoIPCfg->extend_feature = VOIP_SYSTEM_2_FLASH_EXT_FEATURE( feature );
	strcpy(pVoIPCfg->voip_interface,"br-lan");
	
	uci= uci_alloc_context();
	if (uci_load(uci, "rtkvoip", &p))
	{
		uci_perror(uci, NULL);
		
		fprintf(stdout, "Failed to load /etc/config/rtkvoip\n");
	}

	//voipCfgParam_t;
	parse_voipSectionParam(pVoIPCfg, p, voipCfgParam_table, "VOIP");


	//voipCfgPortParam_t
	for(voip_port=0;voip_port<VOIP_PORTS;voip_port++)
	{
		portCfg=&pVoIPCfg->ports[voip_port];
		
		sprintf(port_section_name, "VOIP_PORT%d", voip_port);
		parse_voipSectionParam(portCfg, p, voipCfgPortParam_table,port_section_name);


		//voipCfgProxy_t
		for(proxy=0;proxy<MAX_PROXY;proxy++)
		{

			sprintf(proxy_section_name, "%s_PROXY%d", port_section_name, proxy);
			parse_voipSectionParam(&portCfg->proxies[proxy], p, voipCfgProxy_table , proxy_section_name);
		}

		//SpeedDialCfg_t
		for(speedDial=0;speedDial<MAX_SPEED_DIAL;speedDial++)
		{
			sprintf(speedDial_section_name, "%s_SPEEDDIAL%d", port_section_name,speedDial);
			parse_voipSectionParam(&portCfg->speed_dial[speedDial], p, SpeedDialCfg_table,speedDial_section_name);
		}
		//abbrDialCfg_t
		for(abbrDial=0;abbrDial<MAX_ABBR_DIAL_NUM;abbrDial++)
		{
			sprintf(abbrDial_section_name, "%s_ABBRDIAL%d", port_section_name,abbrDial);
			parse_voipSectionParam(&portCfg->abbr_dial[abbrDial], p, abbrDialCfg_table, abbrDial_section_name);
		}

	}

	//st_ToneCfgParam
	for(tone=0;tone<TONE_CUSTOMER_MAX;tone++)
	{
		sprintf(tone_section_name, "VOIP_TONE%d", tone);
		parse_voipSectionParam(&pVoIPCfg->cust_tone_para[tone], p, st_ToneCfgParam_table, tone_section_name);
	}

	uci_unload(uci,p);
	uci_free_context(uci);
	uci=NULL;
}
#endif

