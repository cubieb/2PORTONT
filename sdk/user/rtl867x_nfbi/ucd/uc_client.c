#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include "uc_udp.h"
#include "uc_mib.h"


#define UC_SER_IP "127.0.0.1"


struct client_cmd_table_t
{
	const char *cmd;
	int (*func)(int index,int argc, char *argv[]);
	void (*print_func)(char *para,char *data,int len);
	const char *msg;
};


static void print_read(char *para,char *data,int len);
static void print_read_byid(char *para,char *data,int len);
static void print_syscmd(char *para,char *data,int len);
static void print_dslioc(char *para,char *data,int len);
static int cmd_write(int index,int argc, char *argv[]);
static int cmd_read(int index,int argc, char *argv[]);
static int cmd_write_byid(int index,int argc, char *argv[]);
static int cmd_read_byid(int index,int argc, char *argv[]);
static int cmd_sysinit(int index,int argc, char *argv[]);
static int cmd_syscmd(int index,int argc, char *argv[]);
static int cmd_dslioc(int index,int argc, char *argv[]);


static struct client_cmd_table_t cmd_table[]=
{
    {"set_mib",			cmd_write,		NULL,				"ucc set_mib mibA=valA"},
    {"get_mib",			cmd_read,		print_read,			"ucc get_mib mibA"},
    {"set_mib_byid",	cmd_write_byid,	NULL,				"ucc set_mib_byid id len value"},
    {"get_mib_byid",	cmd_read_byid,	print_read_byid,	"ucc get_mib_byid id"},
    {"sysinit",			cmd_sysinit,		NULL,			"ucc sysinit dsl-init"},
    {"syscmd",			cmd_syscmd,		print_syscmd,		"ucc syscmd \'cmd_string\'"},
    {"dslioc",			cmd_dslioc,		print_dslioc,		"ucc dslioc id len (id starts from 184832)"},
    {NULL, NULL,NULL,NULL},
};


static int cmd_write(int index,int argc, char *argv[])
{
	int len,count,ret=0;
	char b[UC_MAX_SIZE], *data;

	data=b+UC_HDR_SIZE;	
	sprintf(data,"%s",argv[2]); 
	len = strlen(data);	
	count=uc_udp_sendrecv( UC_SER_IP, UC_CMD_SET_MIB, b, len, sizeof(b) );
	if(count >=0 )
		ret=0;
	else
		ret=-1;   

	return ret;    	
}

static int cmd_write_byid(int index,int argc, char *argv[])
{
	char b[UC_MAX_SIZE], *data;
	UC_DATA_HDR *p;
	int len,count,ret;
	
	if(argc<5) return -1;
	data=b+UC_HDR_SIZE;	
	p=(UC_DATA_HDR*)data;
	p->id=atoi(argv[2]);
	p->len=atoi(argv[3]);
	//test case
	if(p->len==1)
	{
		unsigned char d;
		d=(unsigned char)atoi(argv[4]);
		memcpy( p->data, &d, p->len);
	}else if(p->len==2)
	{
		unsigned short d;
		d=(unsigned short)atoi(argv[4]);
		memcpy( p->data, &d, p->len);
	}else if(p->len==4)
	{
		unsigned int d;
		d=(unsigned int)atoi(argv[4]);
		memcpy( p->data, &d, p->len);
	}else{
		printf( "cmd_write_byid(): un-supported len\n" );
		return -1;
	}
	len=UC_DATA_HDR_SIZE+p->len;
	count=uc_udp_sendrecv( UC_SER_IP, UC_CMD_SET_MIB_BYID, b, len, sizeof(b) );
	if(count >=0 )
		ret=0;
	else
		ret=-1;   

	return ret;
}


static void print_read(char *para,char *data,int len)
{
	unsigned char *value;

	value =(unsigned char *)strchr(data,'=');
	if(value)
	{
		*value=0;
		value++;
		data[len] = '\0';
		printf("print_cfgread: %s=%s\n",data,value);
	}else{
		data[len] = '\0';
		printf("print_cfgread: format error \"%s\"\n",data);
	}
}

static int cmd_read(int index,int argc, char *argv[])
{
	int len,count,ret=0;
	char b[UC_MAX_SIZE], *data;

	data=b+UC_HDR_SIZE;	
	sprintf(data,"%s",argv[2]);
	len = strlen(data);
	count=uc_udp_sendrecv( UC_SER_IP, UC_CMD_GET_MIB, b, len, sizeof(b) );
	if(count >=0 )
	{
		if(cmd_table[index].print_func)
			cmd_table[index].print_func(NULL,data,count);
		ret=0;
	}
	else
		ret=-1;   

   return ret;    
}

static void print_read_byid(char *para,char *data,int len)
{
	UC_DATA_HDR *p;

	printf( "print_read_byid: " );
	if( data&&(len>=UC_DATA_HDR_SIZE) )
	{
		p=(UC_DATA_HDR*)data;
		printf( "id=%d, ", p->id );
		printf( "len=%d, ", p->len );
		if(len<(UC_DATA_HDR_SIZE+p->len))
		{
			printf( "(len is too short)" );
		}else{
			int i;
			printf( "data=0x" );
			for( i=0;i<p->len;i++ ) printf("%02x", (unsigned char)p->data[i]);		
		}
	}
	printf( "\n" );
}

static int cmd_read_byid(int index,int argc, char *argv[])
{
	char b[UC_MAX_SIZE], *data;
	UC_DATA_HDR *p;
	int len,count,ret;

	data=b+UC_HDR_SIZE;	
	p=(UC_DATA_HDR*)data;
	p->id=atoi(argv[2]);
	p->len=0;
	len=UC_DATA_HDR_SIZE+p->len;
	count=uc_udp_sendrecv( UC_SER_IP, UC_CMD_GET_MIB_BYID, b, len, sizeof(b) );
	if(count >=0 )
	{
		if(cmd_table[index].print_func)
			cmd_table[index].print_func(NULL,data,count);
		ret=0;
	}
	else
		ret=-1;   

	return ret;
}

static void print_syscmd(char *para,char *data,int len)
{
	if(len==UC_DATA_SIZE)
		data[len-1]=0;
	else
		data[len] = '\0';
	
	printf("print_syscmd():\n");
	printf("%s",data);
	printf("\n");

}

static int cmd_syscmd(int index,int argc, char *argv[])
{
	int len,count,ret=0;
	char b[UC_MAX_SIZE], *data;

	data=b+UC_HDR_SIZE;
	sprintf(data,"%s",argv[2]);
	len = strlen(data);
	count=uc_udp_sendrecv( UC_SER_IP, UC_CMD_SYS_CMD, b, len, sizeof(b) );
	if(count >=0 )
	{
		if(cmd_table[index].print_func)
			cmd_table[index].print_func(NULL,data,count);
		ret=0;
	}
	else
		ret=-1;   

   return ret;    
}

static int cmd_sysinit(int index,int argc, char *argv[])
{
	int len,count,ret=0;
	char b[UC_MAX_SIZE], *data;

	data=b+UC_HDR_SIZE;
	sprintf(data,"%s",argv[2]); 
	len = strlen(data);	
	count=uc_udp_sendrecv( UC_SER_IP, UC_CMD_SYS_INIT, b, len, sizeof(b) );
	if(count >=0 )
		ret=0;
	else
		ret=-1;   

	return ret;    	
}

#if 0
static int cmd_testsendonly(void)
{
	int len,count,ret=0;
	char b[UC_MAX_SIZE], *data;

	data=b+UC_HDR_SIZE;
	sprintf(data, "%s", UC_STR_MIB_DUMP); 
	len = strlen(data);	
	count=uc_udp_sendrecv( UC_SER_IP, UC_CMD_NO_RES_MASK|UC_CMD_SYS_INIT, b, len, sizeof(b) );
	if(count >=0 )
		ret=0;
	else
		ret=-1;   

	return ret;    	
}
#endif

static void print_dslioc(char *para,char *data,int len)
{
	UC_DATA_HDR *pdsl;

	if(len<UC_DATA_HDR_SIZE)
	{
		printf( "print_dslioc: format error\n" );
		return;
	}

	pdsl=(UC_DATA_HDR*)data;
	printf( "print_dslioc: id=0x%08x\n", pdsl->id );
	printf( "print_dslioc: len=%u\n", pdsl->len );
	printf( "print_dslioc: data\n" );

	{
		int i, datalen;
		datalen=len-UC_DATA_HDR_SIZE;
		for(i=0; i<datalen; i++)
		{
			unsigned char c=pdsl->data[i];
			
			printf( "%02x", c );
			if( c>31 && c<128 ) printf( "(%c)", c );
			else printf( "(.)");
			if( i && ((i&0x7)==0x7) ) printf( "\n" );
		}
		printf( "\n" );
	}
}

static int cmd_dslioc(int index,int argc, char *argv[])
{
	int len,count,ret=0;
	char b[UC_MAX_SIZE], *data;
	UC_DATA_HDR *pdsl;

	if(argc!=4) return -1;
	data=b+UC_HDR_SIZE;
	pdsl=(UC_DATA_HDR*)data;
	pdsl->id=atoi(argv[2]);
	pdsl->len=atoi(argv[3]);
	len=UC_DATA_HDR_SIZE+pdsl->len;
	count=uc_udp_sendrecv( UC_SER_IP, UC_CMD_DSL_IOC, b, len, sizeof(b) );
	if(count >=0 )
	{
		if(cmd_table[index].print_func)
			cmd_table[index].print_func(NULL,data,count);
		ret=0;
	}
	else
		ret=-1;   

   return ret;    
}

static void print_command_list(void)
{
    int i;

    printf("\n========== commands ============\n");
    i = 0;
    while (cmd_table[i].cmd != NULL) {         
            printf("%s\n", cmd_table[i].msg);
        i++;
	}
	printf("ucc mib_info\n");
}

int main(int argc, char **argv)
{
	int i, ret=-1;
	

	if( (argc==2)&&(0==strcmp(argv[1], "mib_info")) )
	{
		uc_mib_dump_info();
		return 0;
	}
	//if( (argc==2)&&(0==strcmp(argv[1], "sendonly")) )
	//{
	//	cmd_testsendonly();
	//	return 0;
	//}
	
	if(argc<3) 
	{
		print_command_list();
		return 0;
	}


    i = 0;
    while (cmd_table[i].cmd != NULL) 
	{
        if (0 == strcmp(argv[1], cmd_table[i].cmd)) 
		{
			if (cmd_table[i].func) 
			{
			    ret = cmd_table[i].func(i,argc, argv);
		        if (ret==0)
		            printf("OK\n");
		        else
		            printf("FAIL\n");
		    }
			break;
        }
        i++;
	}

	return ret;
}

