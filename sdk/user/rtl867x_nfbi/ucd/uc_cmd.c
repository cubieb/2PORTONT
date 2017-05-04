/*================================================================*/
/* System Include Files */

#include <stdio.h> 
#include <string.h>
#include "uc_mib.h"
#include "uc_cmd.h"
#include "uc_udp.h"
#include "uc_xdsl.h"
#include "uc_sys.h"


static int do_xdsl_init=0;
static int do_xdsl_setup=0;


/*================================================================*/
static int uc_cmd_get_mib(char *cmd, int cmd_len, int maxlen);
static int uc_cmd_set_mib(char *cmd, int cmd_len, int maxlen);
static int uc_cmd_get_mib_byid(char *cmd, int cmd_len, int maxlen);
static int uc_cmd_set_mib_byid(char *cmd, int cmd_len, int maxlen);
static int uc_cmd_sysinit(char *cmd, int cmd_len, int maxlen);
static int uc_cmd_syscmd(char *cmd, int cmd_len, int maxlen);
static int uc_cmd_dslioc(char *cmd, int cmd_len, int maxlen);

static struct uc_cmd_entry cmd_table[]=
{ 
	{ UC_CMD_SET_MIB,		"set_mib", 		uc_cmd_set_mib },
	{ UC_CMD_GET_MIB,		"get_mib", 		uc_cmd_get_mib },
	{ UC_CMD_SET_MIB_BYID,	"set_mib_byid", uc_cmd_set_mib_byid },
	{ UC_CMD_GET_MIB_BYID,	"get_mib_byid", uc_cmd_get_mib_byid },
	{ UC_CMD_SYS_INIT,		"sysinit", 		uc_cmd_sysinit },
	{ UC_CMD_SYS_CMD,		"syscmd", 		uc_cmd_syscmd },
	{ UC_CMD_DSL_IOC,		"dslioc", 		uc_cmd_dslioc },
	{ 0,					NULL, 			NULL}
};


/*================================================================*/
static int uc_cmd_syscmd(char *cmd, int cmd_len, int maxlen) 
{
	FILE *fp;
	int resp_len=0;
	
	cmd[cmd_len]='\0';
	fp = popen(cmd, "r");
	if (fp==NULL)
	{
		perror( "uc_cmd_syscmd(): popen" );
		return -1; //error reply in do_cmd
	}

	while (!feof(fp)) 
	{
		resp_len += fread( &cmd[resp_len], sizeof(char), maxlen-resp_len, fp);
		if(resp_len>=maxlen)
		{
			printf("%s(): result is too large!!!\n", __FUNCTION__);
			break;
		}
	}

	pclose(fp);
	return resp_len;
}

static int uc_cmd_set_mib(char *cmd, int cmd_len, int maxlen)
{
	char *param,*val;	
	int ret;

	cmd[cmd_len]='\0';
	param=cmd;
	val=strchr(param,'=');
	if( !val )
	{
		printf("%s(): invalid set mib format:%s\n", __FUNCTION__, cmd);
		return -1;
	}
	*val=0;
	val++;
	cmd = strchr(val,'\n');
	if(cmd) *cmd = '\0';

	if( uc_mib_set_by_name( param,val ) ) //OK
		ret=0;
	else
		ret=-1;

	return ret;
}

static int uc_cmd_set_mib_byid(char *cmd, int cmd_len, int maxlen)
{
	UC_DATA_HDR *pdata;
	int size;

	pdata=(UC_DATA_HDR*)cmd;
	if(cmd_len<UC_DATA_HDR_SIZE)
	{
		printf( "%s(): cmd_len(%d) is too short\n", 
						__FUNCTION__, cmd_len);
		return -1;
	}
	if(cmd_len<(UC_DATA_HDR_SIZE+pdata->len))
	{
		printf( "%s(): cmd_len(%d,%d) is too short for data\n", 
						__FUNCTION__, cmd_len, pdata->len);
		return -1;
	}
	size=uc_mib_get_size_by_id(pdata->id);
	if(pdata->len!=size)
	{
		printf( "%s(): check size(%d,%d) is not the same\n", 
						__FUNCTION__, pdata->len, size );
		return -1;
	}

	if(uc_mib_set(pdata->id,pdata->data))
	{
		return 0;
	}

	return -1;
}

static int uc_cmd_get_mib(char *cmd, int cmd_len, int maxlen)
{
	unsigned char buffer[512];
	
	cmd[cmd_len]='\0';
	memset(buffer, 0x00 , 512);
	if(uc_mib_get_by_name(cmd, buffer) != 0)
	{
		strcat(cmd,"=");
		strcat(cmd,buffer);
		return strlen(cmd);
	}

	return -1;
}

static int uc_cmd_get_mib_byid(char *cmd, int cmd_len, int maxlen)
{
	UC_DATA_HDR *pdata;

	pdata=(UC_DATA_HDR*)cmd;
	if(cmd_len<UC_DATA_HDR_SIZE)
	{
		printf( "%s(): cmd_len(%d) is too short\n", __FUNCTION__, cmd_len);
		return -1;
	}
	if( cmd_len<(pdata->len+UC_DATA_HDR_SIZE) )
	{
		printf( "%s(): cmd_len(%d,%d) is too short for data\n", 
						__FUNCTION__, cmd_len, pdata->len);
		return -1;
	}
	if(uc_mib_get(pdata->id,pdata->data))
	{
		pdata->len=uc_mib_get_size_by_id(pdata->id);
		return (UC_DATA_HDR_SIZE+pdata->len);
	}
	return -1;
}

static int uc_cmd_sysinit(char *cmd, int cmd_len, int maxlen)
{
	cmd[cmd_len]='\0';
	if(!strcmp(cmd, UC_STR_DSL_INIT))
	{
		do_xdsl_init=1;
	}else if(!strcmp(cmd, UC_STR_DSL_SETUP))
	{
		do_xdsl_setup=1;
	}else if(!strcmp(cmd, UC_STR_MIB_DUMP))
	{
		uc_mib_dump();
	}else if(!strcmp(cmd, UC_STR_ETH_INIT))
	{
		uc_sys_eth_init();
	}else if(!strcmp(cmd, UC_STR_ETH_DOWN))
	{
		uc_sys_eth_down();
	}else
	{
		printf( "%s(): un-supported %s\n", __FUNCTION__, cmd);
		return -1;
	}

	return 0;
}

static int uc_cmd_dslioc(char *cmd, int cmd_len, int maxlen)
{
	UC_DATA_HDR *pdsl;
	
	pdsl=(UC_DATA_HDR *)cmd;
	if( cmd_len<UC_DATA_HDR_SIZE )
	{
		printf( "%s(): cmd_len(%d) is too short\n", __FUNCTION__, cmd_len);
		return -1;
	}	
	if( cmd_len<(pdsl->len+UC_DATA_HDR_SIZE) )
	{
		printf( "%s(): cmd_len(%d,%d) is too short for data\n", __FUNCTION__, cmd_len, pdsl->len);
		return -1;
	}

	if( uc_xdsl_drv_get( pdsl->id, pdsl->data, pdsl->len )==0 )
		return -1;

	return cmd_len;
}


/*================================================================*/
int uc_cmd_handle(int id , char *cmd ,int cmd_len, int maxlen)
{
	int i=0,ret=-1;

	while (cmd_table[i].id != 0) 
	{
		if ((cmd_table[i].id == id))	
		{
			ret = cmd_table[i].func(cmd,cmd_len, maxlen);
			break;
		}	
		i++;
	}

	return ret;
}

int uc_cmd_action(void)
{
	int ret=0;

	if(do_xdsl_init)
	{
		uc_xdsl_init();
		do_xdsl_init=0;
	}

	if(do_xdsl_setup)
	{
		uc_xdsl_setup();
		do_xdsl_setup=0;
	}

	return ret;
}

