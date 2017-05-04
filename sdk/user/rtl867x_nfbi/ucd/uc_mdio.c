#include <stdio.h> 
#include <fcntl.h> 
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "linux-2.6.x/drivers/char/rtl867x_nfbi/rtl867x_mdio.h"
#include "uc_udp.h"
#include "uc_mdio.h"


#define MDIO_DEV_NAME		("/dev/rtl_mdio")
static int mdio_fd=-1;


//#define MDIO_USER_DEBUG	1
#ifdef MDIO_USER_DEBUG
#define MDIO_PRINT	printf
#else  /*MDIO_USER_DEBUG*/
#define MDIO_PRINT(...)	while(0){}
#endif /*MDIO_USER_DEBUG*/


static void mdio_debug_out(unsigned char *label, unsigned char *data, int data_length)
{
#ifdef MDIO_USER_DEBUG
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
#endif // MDIO_USER_DEBUG
}


#ifdef _USE_NFBI_WAY_
extern int ser_handle_udp(char* mesg, int len, int maxlen);
extern int ser_handle_action(void);
static void mdio_process_host_cmd(unsigned char *data, int len, int maxlen)
{
	int ret;
	len=ser_handle_udp( data, len, maxlen );
	if(len>0)
	{
		ret=write(mdio_fd, data, len);
		if(ret!=len)
		{
			printf( "mdio_process_host_cmd(): write error %d/%d\n", ret, len );
		}
	}else{
		printf( "mdio_process_host_cmd(): call ser_handle_udp() failed\n" );
	}
	ser_handle_action();		
}


#else /*_USE_NFBI_WAY_*/


/*command byte definition*/
#define MDIO_CMD_FIRST_MASK	0x80
//request
#define MDIO_CMD_MIB_MASK	0x40
#define MDIO_CMD_LOOP_MASK	0x20
//response
#define MDIO_RSP_STAT_MASK	0x40
//mib get/set
#define MDIO_CMD_MIB_SET	0x00
#define MDIO_CMD_MIB_GET	MDIO_CMD_MIB_MASK
//response ok/failed
#define MDIO_RSP_STAT_OK	0x00
#define MDIO_RSP_STAT_FAIL	MDIO_RSP_STAT_MASK

//id definition
#define ID_NETIFNO			0x01

static unsigned char netinfo[14]={0};//mac[6]+ip[4]+mask[4]
static int get_netinfo=0;
#if 0
//for test
static unsigned char ibyte=0;
static unsigned short ishort=0;
static unsigned int iint=0;
static unsigned char *istr=NULL;
static unsigned char iarray[10]={0};
#endif

static int mdio_unpack_req_frame(unsigned char *data, int len, int *cmd_type, int *id, unsigned char *cmd, int cmd_maxlen)
{
	int cmd_len, i;

	if( !data || !cmd_type || !id || !cmd ) return -1;

	if(len<4) 
	{
		printf("%s(): Invalid cmd len [%d] !\n", __FUNCTION__, len);
		return -1;
	}
	if(data[2]!=0)
	{
		printf("%s(): data[2](%u)!=0 !\n", __FUNCTION__, data[2]);
		return -1;
	}	
	if(len!=(data[3]*2+4)) 
	{
		printf("%s(): cmd length not matched [%d, %d] !\n", __FUNCTION__, len, data[3]);
		return -1;
	}
	cmd_len=data[3];
	if(cmd_len>cmd_maxlen)
	{
		printf("%s(): cmd_len is too big [%d, %d]!\n", __FUNCTION__, cmd_len, cmd_maxlen);
		return -1;
	}
	
	*cmd_type=data[0];
	*id = data[1];
	memset(cmd, 0x00, cmd_maxlen);
	for(i=0; i<cmd_len; i++)
		cmd[i] = data[i*2+5];

	return cmd_len;
}

static int mdio_pack_rsp_frame(unsigned char st, unsigned char cmd_id, int len, unsigned char *in, unsigned char *out)
{
	int i;
	out[0] = MDIO_CMD_FIRST_MASK | st;
	out[1] = cmd_id;
	out[2] = 0x00;
	out[3] = (unsigned char)len;
	for (i=0; i<len; i++) 
	{
		out[4+i*2] = 0x00;
		out[4+i*2+1] = in[i];
	}
	return (4+i*2);
}

static void mdio_write_data_to_driver(unsigned char *data, int len)
{
	if(len>MDIO_BUFSIZE) 
	{
		printf("%s(): write length(%d) > MDIO_BUFSIZE(%d)!\n", __FUNCTION__, len, MDIO_BUFSIZE );
		return;
	}
	write(mdio_fd, data, len);
}

static int mdio_handle_mib_set(int id, unsigned char *cmd, int cmd_len, int cmd_maxlen)
{
	int ret=-1;
	
	MDIO_PRINT( "%s(): enter\n", __FUNCTION__ );
	switch(id)
	{
	case ID_NETIFNO:
		{
			if( (cmd==NULL)||(cmd_len!=sizeof(netinfo)) )
			{
				printf( "netinfo: check len error\n" );
				return -1;
			}
			memcpy( netinfo, cmd, sizeof(netinfo) );
			get_netinfo=1;
			ret=0;
		}
		break;
#if 0
	case 0xf0://ibyte
		printf( "ibyte\n" );
		if( (cmd==NULL) || (cmd_len!=1) )
		{
			printf( "ibyte: check error\n" );
			return -1;
		}
		memcpy( &ibyte, cmd, 1 );
		ret=0;
		break;
	case 0xf1://ishort
		printf( "ishort\n" );
		if( (cmd==NULL) || (cmd_len!=2) )
		{
			printf( "ishort: check error\n" );
			return -1;
		}
		memcpy( &ishort, cmd, 2 );
		ret=0;
		break;
	case 0xf2://iint
		printf( "iint\n" );
		if( (cmd==NULL) || (cmd_len!=4) )
		{
			printf( "iint: check error\n" );
			return -1;
		}
		memcpy( &iint, cmd, 4 );
		ret=0;
		break;
	case 0xf3://istr
		printf( "istr\n" );
		if( cmd==NULL )
		{
			printf( "istr: check error\n" );
			return -1;
		}
		cmd[cmd_len]=0;
		if(istr) free(istr);
		istr=strdup(cmd);
		ret=0;
		break;
	case 0xf4: //iarray
		{
			printf( "iarray\n" );
			if( (cmd==NULL) || (cmd_len!=sizeof(iarray) ) )
			{
				printf( "iarray: check error\n" );
				return -1;
			}
			memcpy( iarray, cmd, sizeof(iarray) );
			ret=0;
		}
		break;
#endif
	default:	
		ret=-1;
		break;
	}

	return ret;
}

static int mdio_handle_mib_get(int id, unsigned char *cmd, int cmd_len, int cmd_maxlen)
{
	int ret=-1;

	MDIO_PRINT( "%s(): enter\n", __FUNCTION__ );
	switch(id)
	{
	case ID_NETIFNO:
		ret=sizeof(netinfo);
		memcpy( cmd, netinfo, ret );
		break;
#if 0
	case 0xf0: //ibyte
		ret=1;
		memcpy( cmd, &ibyte, ret );
		break;
	case 0xf1: //ishort
		ret=2;
		memcpy( cmd, &ishort, ret );
		break;
	case 0xf2: //iint
		ret=4;
		memcpy( cmd, &iint, ret );
		break;
	case 0xf3: //istr
		if(istr)
		{
			ret=strlen(istr);
			memcpy( cmd, istr, ret );
		}else
			ret=0;
		break;
	case 0xf4: //iarray
		ret=sizeof(iarray);
		memcpy( cmd, iarray, ret );
		break;
#endif
	default:	
			ret=-1;
		break;
	}

	return ret;
}

static int mdio_handle(int cmd_type, int id, unsigned char *cmd, int cmd_len, int cmd_maxlen)
{

	MDIO_PRINT( "enter mdio_handle\n" );
	if( (cmd_type&MDIO_CMD_MIB_MASK)==MDIO_CMD_MIB_SET )
	{
		return mdio_handle_mib_set(id, cmd, cmd_len, cmd_maxlen);
	}else if( (cmd_type&MDIO_CMD_MIB_MASK)==MDIO_CMD_MIB_GET )
	{
		return mdio_handle_mib_get(id, cmd, cmd_len, cmd_maxlen);
	}	
	MDIO_PRINT( "mdio_handle(): unsupported cmd_type=%d\n", cmd_type );
	return -1;
}

static void mdio_process_host_cmd(unsigned char *data, int len)
{
	unsigned char cmd[MDIO_BUFSIZE], rsp_packet[MDIO_BUFSIZE];
	int cmd_type, id, cmd_len, ret;
	unsigned char st;

	cmd_len=mdio_unpack_req_frame( data, len, &cmd_type, &id, cmd, sizeof(cmd) );
	if(cmd_len<0)
	{
		MDIO_PRINT( "mdio_unpack_req_frame() failed!\n" );
		return;
	}	
	if(cmd_type&MDIO_CMD_LOOP_MASK)
	{
		MDIO_PRINT("execute loopback command!\n");	
		mdio_write_data_to_driver(data, len);
		return;
	}
	MDIO_PRINT( "cmd_type=0x%x, id=%d, cmd_len=%d\n", cmd_type, id, cmd_len);
	mdio_debug_out("cmd data", cmd, cmd_len);

	ret = mdio_handle(cmd_type, id, cmd, cmd_len, sizeof(cmd) );
	if(ret<0)
	{
		cmd[0]=(unsigned char)( ~ret + 1);
		ret=1;
		st=MDIO_RSP_STAT_FAIL;
	}else if(ret == 0) { 
		cmd[0] = '\0';
		ret=1;
		st=MDIO_RSP_STAT_OK;
	}else{
		st=MDIO_RSP_STAT_OK;		
	}

	len = mdio_pack_rsp_frame(st, (unsigned char)id, ret, cmd, rsp_packet);		
	mdio_write_data_to_driver(rsp_packet, len);
	mdio_debug_out("rsp status", rsp_packet, len);
}

unsigned char* mdio_wait_netinfo(int sec)
{
	unsigned char *p=NULL;

	MDIO_PRINT( "%s(): enter\n", __FUNCTION__ );
	while(1)
	{
		sec=sleep(sec);
		if(get_netinfo)
		{
			MDIO_PRINT( "%s(): get netinfo\n", __FUNCTION__ );
			p=netinfo;
			get_netinfo=0;
			break;
		}
		if(sec==0)
		{
			printf( "%s(): timeout\n", __FUNCTION__ );
			break;
		}else{
			MDIO_PRINT( "%s(): rest %d sec\n", __FUNCTION__, sec );
		}
	}

	return p;
}
#endif  /*_USE_NFBI_WAY_*/

static void mdio_process_host_sysctl(unsigned int scr)
{
	MDIO_PRINT("%s(): enter\n", __FUNCTION__ );
	return;
}

void mdio_event_handler(int sig_no)
{
	struct evt_msg evt;

	if(mdio_fd==-1) return;
	
	while( read(mdio_fd, &evt, sizeof(evt))>0 ) 
	{
		MDIO_PRINT("%s(): evt: %s, len=%d\n", 
					__FUNCTION__, (evt.id == IND_CMD_EV ? "CMD" : "SYSCTL") , evt.len);
		mdio_debug_out("data", evt.buf, evt.len);			
		
		if (evt.id == IND_CMD_EV) 
		{
#ifdef _USE_NFBI_WAY_
			mdio_process_host_cmd(evt.buf, evt.len, sizeof(evt.buf));
#else /*_USE_NFBI_WAY_*/
			mdio_process_host_cmd(evt.buf, evt.len);			
#endif /*_USE_NFBI_WAY_*/
		}else if (evt.id == IND_SYSCTL_EV){ 
			mdio_process_host_sysctl(*((unsigned int *)evt.buf));	
		}else{
			MDIO_PRINT("%s(): Invalid evt id [%d]!\n", __FUNCTION__, evt.id);					
		}
	}
}

int mdio_open(void)
{
	mdio_fd=open(MDIO_DEV_NAME, O_RDWR);
	if(mdio_fd==-1)
	{
		perror("mdio_open(): open");
		return -1;
	}

	return 0;
}

void mdio_close(void)
{
	if(mdio_fd!=-1) close(mdio_fd);
	mdio_fd=-1;
}

int mdio_set_host_pid(void)
{	
	int pid;

	// Set daemon pid to driver. 
	// When this ioctl is set, driver will set AllSoftwareReady bit to 'CPU System Status' Register
	pid=getpid();
	if(ioctl(mdio_fd, MDIO_IOCTL_SET_HOST_PID, &pid)<0) 
	{
		perror("mdio_set_host_pid(): ioctl");
		mdio_close();
		return -1;
	}

	return 0;
}


#if 0
int main(int argc, char *argv[])
{ 	

	signal(SIGUSR1, mdio_event_handler);	
	if( mdio_open()<0 ) return -1;
	if( mdio_set_host_pid()<0 ) return -1;


	// pause daemon to wait signal 
	while (1) 
	{
        pause();
		if(get_netinfo)
		{
			printf( "get netinfo:\n" );
			printf( "mac=%02x:%02x:%02x:%02x:%02x:%02x\n", 
				netinfo[0],netinfo[1],netinfo[2],
				netinfo[3],netinfo[4],netinfo[5] );
			printf( "ip=%u.%u.%u.%u\n", 
				netinfo[6],netinfo[7],netinfo[8], netinfo[9] );
			printf( "mask=%u.%u.%u.%u\n", 
				netinfo[10],netinfo[11],netinfo[12], netinfo[13] );			
			get_netinfo=0;

			system( "ifconfig eth0 down" );
			system( "ifconfig eth0 hw ether 00e04c867701" );
			system( "ifconfig eth0 192.168.1.17 netmask 255.255.255.0" );
			system( "ifconfig eth0 up" );
		}
	}

	return 0;
}
#endif

