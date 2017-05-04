#if 0
#include <stdio.h> 
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include "nfbi_api.h"
#include "nfbi_frame.h"


//#define NC_USER_DEBUG	1
#ifdef NC_USER_DEBUG
#define NC_PRINT	printf
#else  /*NC_USER_DEBUG*/
#define NC_PRINT(...)	while(0){}
#endif /*NC_USER_DEBUG*/


static struct nc_cmd_table_t nc_cmd_table[]=
{
//	{name,		id,		mode,	type,		size},
	{"netinfo",	0x01,	NC_RW,	NC_ARRAY,	14}, //mac[6]+ip[4]+mask[4]
#if 0
	{"ibyte",	0xf0,	NC_RW,	NC_BYTE,	1},
	{"ishort",	0xf1,	NC_RW,	NC_WORD,	2},
	{"iint",	0xf2,	NC_RW,	NC_DWORD,	4},
	{"istr",	0xf3,	NC_RW,	NC_STR,		32},
	{"iarray",	0xf4,	NC_RW,	NC_ARRAY,	10},
#endif
	{NULL,		0,		0,			0,		0}
};

static void nc_debug_out(unsigned char *label, unsigned char *data, int data_length)
{
#ifdef NC_USER_DEBUG
    int i,j;
    int num_blocks;
    int block_remainder;

    num_blocks = data_length >> 4;
    block_remainder = data_length & 15;

	if (label) 
	{
	    printf("%s\n", label);
	}

	if (data==NULL || data_length==0)
		return;

    for (i=0; i<num_blocks; i++)
    {
        printf("\t");
        for (j=0; j<16; j++)
        {
            printf("%02x ", data[j + (i<<4)]);
        }
        printf("\n");
    }

    if (block_remainder > 0)
    {
        printf("\t");
        for (j=0; j<block_remainder; j++)
        {
            printf("%02x ", data[j+(num_blocks<<4)]);
        }
        printf("\n");
    }
#endif // NC_USER_DEBUG
}

static int nc_tx_frame(unsigned char cmd_type, unsigned char id, unsigned char *cmd, int cmd_len)
{
	unsigned char buf[NFBI_BUFSIZE*2];
	int num, count;

	if( cmd_len<0 )	return -1;
	if( cmd_len>(NFBI_BUFSIZE-6) ) return -1;
	if( (cmd_len>0)&&(cmd==NULL) ) return -1;
	nc_debug_out("nc_tx_frame(): data", cmd, cmd_len);

	buf[0]=cmd_type;
	buf[1]=id;
	buf[2]=0;
	buf[3]=cmd_len;
	for(num=0;num<cmd_len; num++)
	{
		buf[4+num*2]=0;
		buf[4+num*2+1]=cmd[num];
	}
	num=4+num*2;

	nc_debug_out("nc_tx_frame(): raw data", buf, num);
	count = write(nfbi_fd, buf, num);
	if (count!=num) 
	{
		if (errno == ETIME) 
			printf("nc_tx_frame(): write timeout\n");
		else
			printf("nc_tx_frame(): write error\n");
		return -1;
	}
	return 0;
}

static int nc_rx_frame(unsigned char id, unsigned char *cmd, int cmd_maxlen)
{
	unsigned char buf[NFBI_BUFSIZE*2];
	int count, cmd_len, i, ret;
	
	count = read(nfbi_fd, buf, sizeof(buf));
	if(count<=0) 
	{
		if (errno == ETIME)
			printf("nc_rx_frame(): read timeout\n");
		else
			printf("nc_rx_frame(): read error\n");
		return -1;
	}

	nc_debug_out("nc_rx_frame(): raw data", buf, count);
	if(count<4) 
	{
		printf("%s(): Invalid cmd len [%d] !\n", __FUNCTION__, count);
		return -1;
	}	
    if(buf[1]!=id)
	{
    	printf("incorrect response(%02x) for the command(%02x)\n", buf[1], id);
    	return -1;
    }
	if(buf[2]!=0)
	{
		printf("%s(): data[2](%u)!=0 !\n", __FUNCTION__, buf[2]);
		return -1;
	}	
	if(count!=(buf[3]*2+4)) 
	{
		printf("%s(): cmd length not matched [%d, %d] !\n", __FUNCTION__, count, buf[3]);
		return -1;
	}
	cmd_len=buf[3];
	if(cmd_len>cmd_maxlen)
	{
		printf("CMD length too big [%d]!\n", cmd_len);
		return -1;
	}
	
    if(buf[0]==TAG_GOOD_STATUS)
	{
		memset(cmd, '\0', cmd_maxlen);
		for(i=0; i<cmd_len; i++)
			cmd[i] = buf[i*2+5];

		ret=cmd_len;
		nc_debug_out("nc_rx_frame(): data", cmd, cmd_len);
    }else{
    	if(cmd_len==1)
			printf( "%s(): got error code=%u\n", __FUNCTION__, buf[5] );
		else
			printf( "%s(): unkown error format\n", __FUNCTION__);

		ret=-1;
    }

	return ret;
}


static int nc_name2index(char *name)
{
    int i;
    
    i = 0;
    while (nc_cmd_table[i].name != NULL) 
	{
        if(strcmp(name, nc_cmd_table[i].name)==0)
            return i;
        i++;
    }
    return -1;
}

void nc_print_cmd_list(unsigned char m)
{
    int i;
    
    i = 0;
    while (nc_cmd_table[i].name != NULL) 
	{
        if(nc_cmd_table[i].mode&m)
        {
			if(m&NC_R)
            	printf("nfbi getcmd %s\n", nc_cmd_table[i].name);
			if(m&NC_W)
            	printf("nfbi setcmd %s\n", nc_cmd_table[i].name);
        }
        i++;
	}
}

int nc_handle_setcmd(char *name, unsigned char *value)
{
	unsigned char b[NFBI_BUFSIZE], id;
	int len, ret, index;
	unsigned int ui;
	
	if(!value) return -1;
	if(!name) return -1;
    index = nc_name2index(name);
    if(index<0) return -1;
    if( (nc_cmd_table[index].mode&NC_W)==0 )
		return -1;

	id=nc_cmd_table[index].id;
	switch(nc_cmd_table[index].type)
	{
	case NC_BYTE:
		{
			unsigned char ub;
			ui=atoi(value);
			ub=(unsigned char)ui;
			len=1;
			memcpy( b, &ub, len );
		}
		break;
	case NC_WORD:
		{
			unsigned short us;
			ui=atoi(value);
			us=(unsigned short)ui;
			len=2;
			memcpy( b, &us, len );
		}
		break;
	case NC_DWORD:
		{
			ui=atoi(value);
			len=4;
			memcpy( b, &ui, len );
		}
		break;
	case NC_STR:
		{
			len=strlen(value);
			memcpy( b, value, len );
		}
		break;
	case NC_ARRAY:
		{
			int i;
			unsigned char tmp[3];

			len=nc_cmd_table[index].size;
			if( strlen(value)!=(len*2) )
			{
				printf( "len error(%d, %d)\n", strlen(value), len*2 );
				return -1;
			}
			for (i=0; i<len; i++) 
			{
				strncpy(tmp, value+i*2, 2);
				tmp[2] = '\0'; 
				b[i] = _atoi(tmp ,16);
			}
		}
		break;
	default:
		printf( "unkown data type\n" );
		return -1;
		break;
	}

	ret=nc_tx_frame( TAG_SET_COMMAND, id, b, len );
	if(ret<0)
	{
		printf( "call nc_tx_frame failed\n" );
		return -1;
	}
	ret=nc_rx_frame( id, b, sizeof(b) );
	if(ret<0)
	{
		printf( "call nc_rx_frame failed\n" );
		return -1;
	}

	return 0;
}

int nc_handle_getcmd(char *name)
{
	unsigned char b[NFBI_BUFSIZE], id;
	int ret,index;

	if(!name) return -1;
    index = nc_name2index(name);
    if(index<0) return -1;
    if( (nc_cmd_table[index].mode&NC_R)==0 )
		return -1;
	
	id=nc_cmd_table[index].id;
	ret=nc_tx_frame( TAG_GET_COMMAND, id, NULL, 0 );
	if(ret<0)
	{
		printf( "call nc_tx_frame failed\n" );
		return -1;
	}
	ret=nc_rx_frame( id, b, sizeof(b) );
	if(ret<0)
	{
		printf( "call nc_rx_frame failed\n" );
		return -1;
	}

	switch(nc_cmd_table[index].type)
	{
	case NC_BYTE:
		{
			if(ret!=1)
			{
				printf( "return len error (%d,%d)\n", ret, 1 );
				return -1;
			}
			printf( "%s=%u\n", nc_cmd_table[index].name, b[0] );
		}
		break;
	case NC_WORD:
		{
			unsigned short *pUS=(unsigned short *)b;
			if(ret!=2)
			{
				printf( "return len error (%d,%d)\n", ret, 2 );
				return -1;
			}
			printf( "%s=%u\n", nc_cmd_table[index].name, *pUS );
		}
		break;
	case NC_DWORD:
		{
			unsigned int *pUI=(unsigned int *)b;
			if(ret!=4)
			{
				printf( "return len error (%d,%d)\n", ret, 4 );
				return -1;
			}
			printf( "%s=%u\n", nc_cmd_table[index].name, *pUI );
		}
		break;
	case NC_STR:
		{
			//check size??  ret < max size
			b[ret]=0;
			printf( "%s=%s\n", nc_cmd_table[index].name, b );
		}
		break;
	case NC_ARRAY:
		{
			int i;
			if(ret!=nc_cmd_table[index].size)
			{
				printf( "return len error (%d,%d)\n", ret, nc_cmd_table[index].size );
				return -1;
			}
			printf( "%s=", nc_cmd_table[index].name);
			for(i=0;i<ret;i++)
				printf( "%02x", b[i] );
			printf( "\n");
		}
		break;
	default:
		printf( "unkown data type\n" );
		return -1;
		break;
	}

	return 0;

}
#endif

