#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "bfdlib.h"
#include "bfdsocket.h"
#include "bfdtimeout.h"
#include "bfdutil.h"

#if 0
#include <rtk/utility.h>
#else
#define BUFFER_SIZE	256

struct data_to_pass_st {
	int	id;
	char data[BUFFER_SIZE];
};
#endif

//#define BFDCFG_DEBUG
#ifdef BFDCFG_DEBUG
unsigned char bfdcfg_dbgflag=1;
#else
unsigned char bfdcfg_dbgflag=0;
#endif


/*************************************************************************************/
#define BFDCFG_DBGLOG		"/var/bfd/bfd_dbg_log"
struct bfdcfg_dbglog_t
{
	struct bfdcfg_dbglog_t	*next;
	unsigned char		debug;
	unsigned char		ifname[16];
};
struct bfdcfg_dbglog_t		*gbfdcfg_dbglog=NULL;

static void bfdcfg_dbglog_write(void)
{
	FILE *fp;
	
	fp=fopen(BFDCFG_DBGLOG, "w");
	if(fp)
	{
		struct bfdcfg_dbglog_t *p;
		
		p=gbfdcfg_dbglog;
		while(p)
		{
			fprintf( fp, "%s %u\n", p->ifname, p->debug  );
			p=p->next;
		}
		fclose(fp);
	}
}

static void bfdcfg_dbglog_set( unsigned char *ifname, unsigned char debug )
{
	
	if(ifname && ifname[0])
	{
		struct bfdcfg_dbglog_t *p;

		if(bfdcfg_dbgflag) printf( "%s> ifname=%s, debug=%u\n", __FUNCTION__, ifname, debug );
		
		p=gbfdcfg_dbglog;
		while(p)
		{
			if( strcmp(p->ifname, ifname)==0 )
				break;
			p=p->next;
		}
		
		if(p)
		{
			p->debug=debug;
		}else{
			p=malloc( sizeof(struct bfdcfg_dbglog_t) );
			if(p)
			{
				memset( p, 0, sizeof(struct bfdcfg_dbglog_t) );
				p->debug=debug;
				strcpy( p->ifname, ifname );
				p->next=gbfdcfg_dbglog;
				gbfdcfg_dbglog=p;
			}else{
				if(bfdcfg_dbgflag) printf( "%s> malloc() failed\n", __FUNCTION__ );
			}
		}
		
		bfdcfg_dbglog_write();
	}
}

/*************************************************************************************/
#define BFD_SERVER_FIFO_NAME "/tmp/bfd_serv_fifo"
static int server_fifo_fd;
static int bfdcfg_init_msg (void)
{
  	
	mkfifo(BFD_SERVER_FIFO_NAME, 0777);
	server_fifo_fd = open(BFD_SERVER_FIFO_NAME, O_RDONLY);
	if (server_fifo_fd == -1) {
		printf("%s> open fifo error errno=%d(%s)\n", __FUNCTION__, errno, strerror(errno));
		return -1;
	}

	return 0;
}

static int bfdcfg_poll_cc(int fd)
{
	int bytesToRead = 0;
	(void)ioctl(fd, FIONREAD, (int)&bytesToRead);
	return bytesToRead;
}

static int bfdcfg_recv_msg( unsigned char *b, int len )
{
	int read_res=-1;
  	if(b && bfdcfg_poll_cc(server_fifo_fd)) 
  	{
		read_res = read(server_fifo_fd, b, len);
	}	
	return read_res;
}
/*************************************************************************************/
static int bfdcfg_is_hex(char c)
{
    return (((c >= '0') && (c <= '9')) ||
            ((c >= 'A') && (c <= 'F')) ||
            ((c >= 'a') && (c <= 'f')));
}

static int bfdcfg_hexstr2binstr(char *string, int len, unsigned char *key, int *keylen)
{
	char tmpBuf[4];
	int idx, ii=0;

	if( (string==NULL) || (key==NULL) || (keylen==NULL) )
		return -1;
	
	*keylen=0;
	for (idx=0; idx<len; idx+=2) 
	{
		tmpBuf[0] = string[idx];
		tmpBuf[1] = string[idx+1];
		tmpBuf[2] = 0;
		if ( !bfdcfg_is_hex(tmpBuf[0]) || !bfdcfg_is_hex(tmpBuf[1]))
			return -1;

		key[ii++] = (unsigned char) strtol(tmpBuf, (char**)NULL, 16);
	}
	*keylen=ii;
	return 0;
}
/*************************************************************************************/
#define BFDCMD_NONE	0
#define BFDCMD_SHOW	1
#define BFDCMD_ADD	2
#define BFDCMD_DEL	3
#define BFDCMD_DBG	4

#define MAX_ARGS	32
#define MAX_ARG_LEN	64

struct command
{
	int	needs_arg;
	int	num_string_arg;/*unused*/
	char	*name;
	int	(*func)(char *arg);
};
static struct command commands[];
static struct bfdcfg cfg;
static struct bfdsessiondata *pbfdlist=NULL;

static int bfdcfg_show(char *arg)
{
	cfg.bfdcmd=BFDCMD_SHOW;
	if(arg && arg[0])
		strcpy( cfg.bfddata.Interface, arg );
	else
		return -1;

	return 0;
}

static int bfdcfg_add(char *arg)
{
	cfg.bfdcmd=BFDCMD_ADD;
	if(arg && arg[0])
	{
		struct bfdsessiondata *c=pbfdlist;
		while(c)
		{
			if( strcmp( c->Interface, arg)==0 )
			{
				printf( "%s> this interface(%s) exists in the bfdlist!!!\n", __FUNCTION__,  arg );
				return -1;
			}
			c=c->next;
		}

		strcpy( cfg.bfddata.Interface, arg );
		
	}else
		return -1;

	return 0;
}

static int bfdcfg_del(char *arg)
{
	cfg.bfdcmd=BFDCMD_DEL;
	if(arg && arg[0])
		strcpy( cfg.bfddata.Interface, arg );
	else
		return -1;

	return 0;
}

static int bfdcfg_dbg(char *arg)
{
	cfg.bfdcmd=BFDCMD_DBG;
	if(arg && arg[0])
		strcpy( cfg.bfddata.Interface, arg );
	else
		return -1;

	return 0;
}

static int bfdcfg_file(char *arg)
{
	int ret=-1;
	if(arg && arg[0])
	{
		FILE *fp;
		
		fp=fopen( arg, "r" );
		if(fp)
		{
			char buf[128];
			
			ret=0;
			while( fgets(buf, sizeof(buf)-1, fp) )
			{
				char *p, *pcmd, *parg;
				int i,c;
				
				buf[sizeof(buf)-1]=0;
				p=strtok( buf, "\n\r" );
				if(bfdcfg_dbgflag) printf( "%s> got line=>%s\n", __FUNCTION__, p?p:"(NULL)" );
				if(p==NULL) continue;
				pcmd=strtok( p, "=");
				parg=strtok( NULL, "\n\r" );
				if(pcmd) pcmd=strtok( pcmd, " \t");
				if(parg) parg=strtok( parg, " \t" );
				if(bfdcfg_dbgflag) printf( "%s> got cmd=%s, arg=%s.\n", __FUNCTION__, pcmd?pcmd:"(NULL)", parg?parg:"(NULL)" );
				if(pcmd==NULL) continue;

				for(c=0; commands[c].name!=NULL; c++) 
				{
					if(!strcmp(pcmd, commands[c].name)) 
					{
						if(commands[c].needs_arg)
						{
							i++;
							ret=commands[c].func(parg);
						}else{
							ret=commands[c].func(NULL);
						}
						
						//error, stop processiong this message
						if(ret)
						{ 
							printf( "%s> handle cmd:'%s' return error\n", __FUNCTION__, commands[c].name );
							break;
						}
					}
				}
				//if pcmd is not one of commands[], skip it.

				//error, stop processiong this message
				if(ret) break;	
			}		
			fclose(fp);
		}else{
			perror("fopen");
		}
	}

	return ret;
}

static int bfdcfg_localip(char *arg)
{
	int ret=-1;
	if(arg && arg[0])
	{
		struct bfdsessiondata *pb=&cfg.bfddata;
		struct in_addr ina;
		if( inet_aton(arg, &ina) )
		{
			strncpy( pb->LocalIP, arg, 15 );
			pb->LocalIP[15]=0;
			if(bfdcfg_dbgflag) printf( "%s> set localip=%s\n",  __FUNCTION__, pb->LocalIP );
			ret=0;
		}
	}	
	return ret;
}

static int bfdcfg_remoteip(char *arg)
{
	int ret=-1;
	if(arg && arg[0])
	{
		struct bfdsessiondata *pb=&cfg.bfddata;
		struct in_addr ina;
		if( inet_aton(arg, &ina) )
		{
			strncpy( pb->RemoteIP, arg, 15 );
			pb->RemoteIP[15]=0;
			strcpy( pb->EchoRemoteIP, pb->RemoteIP );
			if(bfdcfg_dbgflag) printf( "%s> set remoteip=%s\n",  __FUNCTION__, pb->RemoteIP );
			ret=0;
		}
	}	
	return ret;
}

static int bfdcfg_opmode(char *arg)
{
	int ret=-1;
	if(arg && arg[0])
	{
		struct bfdsessiondata *pb=&cfg.bfddata;
		int type;
		type=atoi(arg);
		if(type==BFD_DEMAND_MODE)
			pb->session.DemandMode=1;
		else 
			pb->session.DemandMode=0;
		if(bfdcfg_dbgflag) printf( "%s> set DemandMode=%u\n",  __FUNCTION__, pb->session.DemandMode );
		ret=0;
	}	
	return ret;
}

static int bfdcfg_role(char *arg)
{
	int ret=-1;
	if(arg && arg[0])
	{
		struct bfdsessiondata *pb=&cfg.bfddata;
		int type;
		type=atoi(arg);
		if(type==BFD_PASSIVE_ROLE)
			pb->Role=BFD_PASSIVE_ROLE;
		else 
			pb->Role=BFD_ACTIVE_ROLE;
		if(bfdcfg_dbgflag) printf( "%s> set Role=%u\n",  __FUNCTION__, pb->Role );
		ret=0;
	}	
	return ret;
}

static int bfdcfg_detectmult(char *arg)
{
	int ret=-1;
	if(arg && arg[0])
	{
		struct bfdsessiondata *pb=&cfg.bfddata;
		int type;
		type=atoi(arg);
		if( (type>=1)&&(type<=255) )
		{
			pb->session.DetectMult=(unsigned char)type;
			if(bfdcfg_dbgflag) printf( "%s> set DetectMult=%u\n",  __FUNCTION__, pb->session.DetectMult );
			ret=0;
		}
	}	
	return ret;
}

static int bfdcfg_mintxint(char *arg)
{
	int ret=-1;
	if(arg && arg[0])
	{
		struct bfdsessiondata *pb=&cfg.bfddata;
		unsigned int value;
		value=strtoul(arg, (char**)NULL, 10);
		if( (value==ULONG_MAX)&&(errno==ERANGE) )
		{
			if(bfdcfg_dbgflag) printf( "%s> set DesiredMinTxInterval error\n",  __FUNCTION__ );
		}else{
			pb->session.DesiredMinTxInterval=value;
			pb->session.UserDesiredMinTxInterval=pb->session.DesiredMinTxInterval;
			if(bfdcfg_dbgflag) printf( "%s> set DesiredMinTxInterval=%u\n",  __FUNCTION__, pb->session.DesiredMinTxInterval );
			ret=0;
		}
	}	
	return ret;
}

static int bfdcfg_minrxint(char *arg)
{
	int ret=-1;
	if(arg && arg[0])
	{
		struct bfdsessiondata *pb=&cfg.bfddata;
		unsigned int value;
		value=strtoul(arg, (char**)NULL, 10);
		if( (value==ULONG_MAX)&&(errno==ERANGE) )
		{
			if(bfdcfg_dbgflag) printf( "%s> set RequiredMinRxInterval error\n",  __FUNCTION__ );
		}else{
			pb->session.RequiredMinRxInterval=value;
			if(bfdcfg_dbgflag) printf( "%s> set RequiredMinRxInterval=%u\n",  __FUNCTION__, pb->session.RequiredMinRxInterval );
			ret=0;
		}
	}	
	return ret;
}

static int bfdcfg_minechorxint(char *arg)
{
	int ret=-1;
	if(arg && arg[0])
	{
		struct bfdsessiondata *pb=&cfg.bfddata;
		unsigned int value;
		value=strtoul(arg, (char**)NULL, 10);
		if( (value==ULONG_MAX)&&(errno==ERANGE) )
		{
			if(bfdcfg_dbgflag) printf( "%s> set LocalEchoRxInterval error\n",  __FUNCTION__ );
		}else{
			pb->session.LocalEchoRxInterval=value;
			if(bfdcfg_dbgflag) printf( "%s> set LocalEchoRxInterval=%u\n",  __FUNCTION__, pb->session.LocalEchoRxInterval );
			ret=0;
		}
	}	
	return ret;
}

static int bfdcfg_authtype(char *arg)
{
	int ret=-1;
	if(arg && arg[0])
	{
		struct bfdsessiondata *pb=&cfg.bfddata;
		int type;
		type=atoi(arg);
		if( (type>=AUTH_NONE) && (type<=AUTH_METI_KEYED_SHA1) )
		{
			pb->session.AuthType=(unsigned char)type;
			if(bfdcfg_dbgflag) printf( "%s> set AuthType=%u\n",  __FUNCTION__, pb->session.AuthType );
			ret=0;
		}
	}	
	return ret;
}

static int bfdcfg_authkeyid(char *arg)
{
	int ret=-1;
	if(arg && arg[0])
	{
		struct bfdsessiondata *pb=&cfg.bfddata;
		int type;
		type=atoi(arg);
		if( (type>=0) && (type<=255) )
		{
			pb->AuthKeyID=(unsigned char)type;
			if(bfdcfg_dbgflag) printf( "%s> set AuthType=%u\n",  __FUNCTION__, pb->AuthKeyID );
			ret=0;
		}
	}	
	return ret;
}

static int bfdcfg_authkey(char *arg)
{
	int ret=-1;

	if(arg && arg[0])
	{
		struct bfdsessiondata *pb=&cfg.bfddata;
		int len, newlen, i;
		len=strlen(arg);

		if( (len&0x1)==0x1 ) 
			return ret;

		if( len> (sizeof(pb->AuthKey)*2) ) 
			return ret;
		if( bfdcfg_hexstr2binstr(arg, len, pb->AuthKey, &newlen )==0 )
		{
			pb->AuthKeyLen=(unsigned char)newlen;
			if(bfdcfg_dbgflag) 
			{
				printf( "%s> set AuthKeyLen=%u\n",  __FUNCTION__, pb->AuthKeyLen );
				printf( "%s> set AuthKey=",  __FUNCTION__);
					for(i=0; i<newlen; i++) printf( "%02x", pb->AuthKey[i] );
					printf( "\n");
			}
			ret=0;
		}
	}	
	return ret;
}

static int bfdcfg_dscp(char *arg)
{
	int ret=-1;
	if(arg && arg[0])
	{
		struct bfdsessiondata *pb=&cfg.bfddata;
		unsigned int type;
		type=atoi(arg);
		if( (type>=0) && (type<=63) )
		{
			pb->dscp=type;
			if(bfdcfg_dbgflag) printf( "%s> set dscp=%u\n",  __FUNCTION__, pb->dscp );
			ret=0;
		}
	}	
	return ret;
}

static int bfdcfg_ethprio(char *arg)
{
	int ret=-1;
	if(arg && arg[0])
	{
		struct bfdsessiondata *pb=&cfg.bfddata;
		unsigned int type;
		type=atoi(arg);
		if( (type>=0) && (type<=7) )
		{
			pb->ethprio=type;
			if(bfdcfg_dbgflag) printf( "%s> set ethprio=%u\n",  __FUNCTION__, pb->ethprio );
			ret=0;
		}
	}	
	return ret;
}

static int bfdcfg_debug(char *arg)
{
	int ret=-1;
	if(arg && arg[0])
	{
		struct bfdsessiondata *pb=&cfg.bfddata;
		int type;
		type=atoi(arg);
		if(type==0)
			pb->debug=0;
		else
			pb->debug=1;
		if(bfdcfg_dbgflag) printf( "%s> set debug=%u\n",  __FUNCTION__, pb->debug );
		ret=0;
	}	
	return ret;
}

static int bfdcfg_dhcpc(char *arg)
{
	int ret=-1;
	if(arg && arg[0])
	{
		struct bfdsessiondata *pb=&cfg.bfddata;
		int pid;
		pid=atoi(arg);
		if(pid)
		{
			pb->dhcpc_pid=pid;
			ret=0;
			if(bfdcfg_dbgflag) printf( "%s> set dhcpc_pid=%u\n",  __FUNCTION__, pb->dhcpc_pid );
		}
	}	
	return ret;
}

static struct command commands[] = 
{
	//cmd
	{1, 0, "show",	bfdcfg_show},
	{1, 0, "add",	bfdcfg_add},
	{1, 0, "del",	bfdcfg_del},
	{1, 0, "dbg",	bfdcfg_dbg},
	
	//bfd-related arg
	{1, 0, "LocalIP",	bfdcfg_localip},
	{1, 0, "RemoteIP",	bfdcfg_remoteip},
	{1, 0, "OpMode",	bfdcfg_opmode},
	{1, 0, "Role",		bfdcfg_role},
	{1, 0, "DetectMult",	bfdcfg_detectmult},
	{1, 0, "MinTxInt",	bfdcfg_mintxint},
	{1, 0, "MinRxInt",	bfdcfg_minrxint},
	{1, 0, "MinEchoRxInt",	bfdcfg_minechorxint},
	{1, 0, "AuthType",	bfdcfg_authtype},
	{1, 0, "AuthKeyID",	bfdcfg_authkeyid},
	{1, 0, "AuthKey",	bfdcfg_authkey},
	{1, 0, "DSCP",		bfdcfg_dscp},
	{1, 0, "EthPrio",	bfdcfg_ethprio},
	
	//other arg
	{1, 0, "file",	bfdcfg_file},
	{1, 0, "debug",	bfdcfg_debug},
	{1, 0, "dhcpc",	bfdcfg_dhcpc},
	
	
	{0, 0, NULL,	NULL}
};
/*************************************************************************************/
static int bfdcfg_showbfd( struct bfdcfg *c )
{
	int ret=-1;
	
	if(c)
	{
		struct bfdsessiondata *pb=&c->bfddata;
		struct bfdsessiondata *p;
		unsigned char showall=0;
		
		if(bfdcfg_dbgflag) printf( "%s> search pb->Interface=%s\n", __FUNCTION__, pb->Interface );
		if(strcmp("all", pb->Interface)==0) showall=1;
		p=pbfdlist;
		while(p)
		{
			if(bfdcfg_dbgflag) printf( "%s> current p->Interface=%s\n", __FUNCTION__, p->Interface );
			if( (strcmp(p->Interface, pb->Interface)==0) || showall	 )
			{
				if(bfdcfg_dbgflag) printf( "%s> got match\n", __FUNCTION__);
				bfd_sessiondata_dump(p);
				ret=0;
				if(!showall) break;
			}
			p=p->next;
		}
	}
	
	return ret;
}

static int bfdcfg_addbfd( struct bfdcfg *c )
{
	int ret=-1;
	
	if(c)
	{
		struct bfdsessiondata *pb=&c->bfddata;
		struct bfdsessiondata *newpb;
		
		newpb=malloc( sizeof(struct bfdsessiondata) );
		if(newpb)
		{
			ret=0;
			bfd_session_init( newpb );
			bfd_echo_init(newpb);
			newpb->Role=BFD_PASSIVE_ROLE;
			newpb->session.DemandMode=1;
			//newpb->session.LocalEchoRxInterval=2000000;
#ifdef _TEST_ON_ONE_PC_
			newpb->RemotePort=13784;
#endif //_TEST_ON_ONE_PC_

			//copy the configuration from bfdcfg struct
			newpb->session.DemandMode=pb->session.DemandMode;
			newpb->Role=pb->Role;
			newpb->session.DetectMult=pb->session.DetectMult;
			newpb->session.DesiredMinTxInterval=pb->session.DesiredMinTxInterval;
			newpb->session.UserDesiredMinTxInterval=pb->session.UserDesiredMinTxInterval;
			newpb->session.RequiredMinRxInterval=pb->session.RequiredMinRxInterval;
			newpb->session.LocalEchoRxInterval=pb->session.LocalEchoRxInterval;
			strcpy( newpb->Interface, pb->Interface);
			strcpy( newpb->LocalIP, pb->LocalIP);
			strcpy( newpb->RemoteIP, pb->RemoteIP);
			strcpy( newpb->EchoRemoteIP, pb->EchoRemoteIP);
			strcpy( newpb->EchoSourceIP, "192.168.101.50" );
		#ifdef _LOCALWANTEST_
		{	//for test case
			//192.168.8.1  => 192.168.2.200
			//192.168.4.1  => 192.168.2.202
			if( strcmp(newpb->RemoteIP, "192.168.8.1")==0 )
			{
				printf( "%s> Change RemoteIP from %s to %s\n",  __FUNCTION__, newpb->RemoteIP, "192.168.2.200" );
				printf( "%s> EchoRemoteIP = %s\n",  __FUNCTION__, newpb->EchoRemoteIP );
				printf( "%s> EchoSourceIP = %s\n",  __FUNCTION__, newpb->EchoSourceIP );
				strcpy( newpb->RemoteIP, "192.168.2.200");
			}
			else if( strcmp(newpb->RemoteIP, "192.168.4.1")==0 )
			{
				printf( "%s> Change RemoteIP from %s to %s\n",  __FUNCTION__, newpb->RemoteIP, "192.168.2.202" );
				printf( "%s> EchoRemoteIP = %s\n",  __FUNCTION__, newpb->EchoRemoteIP );
				printf( "%s> EchoSourceIP = %s\n",  __FUNCTION__, newpb->EchoSourceIP );
				strcpy( newpb->RemoteIP, "192.168.2.202");
			}	
		}
		#endif //_LOCALWANTEST_
			newpb->debug=pb->debug;//debug
			newpb->dhcpc_pid=pb->dhcpc_pid;//dhcpc pid
			newpb->dscp=pb->dscp;//dscp
			newpb->ethprio=pb->ethprio;//ethprio
			//auth part
			newpb->session.AuthType=pb->session.AuthType;
			newpb->AuthKeyID=pb->AuthKeyID;
			newpb->AuthKeyLen=pb->AuthKeyLen;
			memcpy( newpb->AuthKey, pb->AuthKey, pb->AuthKeyLen );
			if(newpb->session.AuthType!=AUTH_NONE)
			{
				switch(newpb->session.AuthType)
				{
				case AUTH_PASSWORD:/*1~16*/
					if( (newpb->AuthKeyLen<1)||(newpb->AuthKeyLen>16) )
						ret=-1;
					break;
				case AUTH_KEYED_MD5:
				case AUTH_METI_KEYED_MD5:
					if(newpb->AuthKeyLen!=16)
						ret=-1;
					break;
				case AUTH_KEYED_SHA1:
				case AUTH_METI_KEYED_SHA1:
					if(newpb->AuthKeyLen!=20)
						ret=-1;
					break;
				}
				
				if(ret<0) printf( "%s> check key-length failed\n", __FUNCTION__ );
			}

			if(ret==0)
			{
				newpb->LocalRecvSock = bfd_bind(newpb->LocalRecvPort, newpb->Interface);
				//printf( "newpb->LocalRecvSock=%d\n", newpb->LocalRecvSock );
				if(newpb->LocalRecvSock<0)
				{
					printf( "%s> bind newpb->LocalRecvSock error\n", __FUNCTION__ );
					ret=-1;
				}else{
					bfd_setsockopt( newpb->LocalRecvSock, newpb->dscp, newpb->ttl, newpb->ethprio );
				}
			}
			if(ret==0)
			{
				newpb->LocalSendSock = bfd_bind(newpb->LocalSendPort, newpb->Interface);
				//if(bfdcfg_dbgflag) printf( "newpb->LocalSendSock=%d\n", newpb->LocalSendSock );
				if(newpb->LocalSendSock<0)
				{
					printf( "%s> bind newpb->LocalRecvSock error\n", __FUNCTION__ );
					ret=-1;
				}else{
					bfd_setsockopt( newpb->LocalSendSock, newpb->dscp, newpb->ttl, newpb->ethprio );
				}
			}

			if(ret==0)
			{
				newpb->LocalSendFinalSock = bfd_bind(newpb->LocalSendFinalPort, newpb->Interface);
				//if(bfdcfg_dbgflag) printf( "newpb->LocalSendFinalSock=%d\n", newpb->LocalSendFinalSock );
				if(newpb->LocalSendFinalSock<0)
				{
					printf( "%s> bind newpb->LocalSendFinalSock error\n", __FUNCTION__ );
					ret=-1;
				}else{
					bfd_setsockopt( newpb->LocalSendFinalSock, newpb->dscp, newpb->ttl, newpb->ethprio );
				}
			}
			
			if(ret) //error
			{
				if(newpb->LocalRecvSock>=0) bfd_close(newpb->LocalRecvSock);
				if(newpb->LocalSendSock>=0) bfd_close(newpb->LocalSendSock);
				if(newpb->LocalSendFinalSock>=0) bfd_close(newpb->LocalSendFinalSock);
				free(newpb);
				newpb=NULL;
			}else{
				struct timeval tv;
			
				if(bfdcfg_dbgflag) bfd_sessiondata_dump(newpb);
				
				//bfd_detect_update_interval(newpb);
				bfd_active_demand_mode(newpb);
				gettimeofday( &tv, NULL );
				timeradd( &tv, &newpb->DetectInterval, &newpb->DetectTimeout );

				//add to pbfdlist;
				newpb->next=pbfdlist;
				pbfdlist=newpb;
				if(bfdcfg_dbgflag) printf( "%s> Add OK\n", __FUNCTION__ );

				//setup iptables rules
				bfdutil_set_iprules( newpb->debug,  newpb->Interface, newpb->LocalIP, newpb->RemoteIP, newpb->EchoRemoteIP);
				
				bfdcfg_dbglog_set( newpb->Interface, newpb->debug );
			}
		}
	}	
	return ret;
}

static int bfdcfg_delbfd( struct bfdcfg *c )
{
	int ret=-1;
	
	if(c)
	{
		struct bfdsessiondata *pb=&c->bfddata;
		struct bfdsessiondata **q, *p;
		
		q=&pbfdlist;
		p=pbfdlist;
		while(p)
		{
			if( strcmp(p->Interface, pb->Interface)==0 )
			{
				break;
			}
			
			q=&p->next;
			p=p->next;
		}
		
		if(p)//found
		{			
			*q=p->next;
			p->next=NULL;
			ret=0;

			//clear iptables rules
			bfdutil_clear_iprules( p->debug,  p->Interface, p->LocalIP, p->RemoteIP, p->EchoRemoteIP );

			//clean.....
			if(bfdcfg_dbgflag)
			{ 
				printf( "clean socket, p->LocalRecvSock=%d, p->LocalSendSock=%d, p->LocalSendFinalSock=%d\n", 
					p->LocalRecvSock, p->LocalSendSock, p->LocalSendFinalSock);
				printf( "clean socket, p->EchoSock=%d, p->EchoSendRawSock=%d\n", 
					p->EchoSock, p->EchoSendRawSock );
			}
			if(p->LocalRecvSock>=0)
			{
				if( bfd_close(p->LocalRecvSock)<0 )
					perror( "close-LocalRecvSock" );
			}
			if(p->LocalSendSock>=0)
			{
				if( bfd_close(p->LocalSendSock)<0 )
					perror( "close-LocalSendSock" );
			}
			if(p->LocalSendFinalSock>=0)
			{
				if( bfd_close(p->LocalSendFinalSock)<0 )
					perror( "close-LocalSendFinalSock" );
			}
			if(p->PeriodicTxState==BFD_ON) bfd_untimeout(&p->PeriodicTxTime);
			//echo-related
			if(p->EchoSendRawSock>=0)
			{
				if( bfd_close(p->EchoSendRawSock)<0 )
					perror( "close-EchoSendRawSock" );
			}
			if(p->EchoSock>=0)
			{
				if( bfd_close(p->EchoSock)<0 )
					perror( "close-EchoSock" );
			}
			if(p->EchoState==BFD_ON) bfd_untimeout(&p->EchoTxTime);
#ifdef PERIODIC_POLL_WHEN_DEMAND
			if(p->PeriodicPollState==BFD_ON) bfd_untimeout(&p->PeriodicPollTime);
#endif //PERIODIC_POLL_WHEN_DEMAND
			free(p);
			if(bfdcfg_dbgflag) printf( "%s> Del OK\n", __FUNCTION__ );
		}
	}
	return ret;
}

static int bfdcfg_dbgbfd( struct bfdcfg *c )
{
	int ret=-1;
	
	if(c)
	{
		struct bfdsessiondata *pb=&c->bfddata;
		struct bfdsessiondata **q, *p;
		
		q=&pbfdlist;
		p=pbfdlist;
		while(p)
		{
			if( strcmp(p->Interface, pb->Interface)==0 )
			{
				break;
			}
			
			q=&p->next;
			p=p->next;
		}
		
		if(p)//found
		{
			p->debug=pb->debug;
			if(bfdcfg_dbgflag) printf( "%s> set debug=%u\n", __FUNCTION__, p->debug );
			ret=0;
		}else if( strcmp("cmd", pb->Interface)==0 ) //a special argument to set bfdcfg_dbgflag by using dbg command
		{
			if(bfdcfg_dbgflag) printf( "%s> set bfdcfg_dbgflag=%u\n", __FUNCTION__, pb->debug );
			bfdcfg_dbgflag=pb->debug;
			ret=0;
		}

		bfdcfg_dbglog_set( pb->Interface, pb->debug );
	}
	return ret;
}

static void bfdcfg_handle_msg (void)
{
	struct data_to_pass_st 	msg;
	int	read_res;
	int i, c;
  	int argc;
	char argv[MAX_ARGS][MAX_ARG_LEN+1];
  	char *arg_ptr;
	int arg_idx;
  	
  	
  	read_res=bfdcfg_recv_msg( (unsigned char*)&msg, sizeof(msg));
  	if( read_res<=0 ) return;

	argc = 0;
	arg_idx = 0;
	arg_ptr = msg.data;
	if(bfdcfg_dbgflag) printf( "%s> msg: %s\n", __FUNCTION__, arg_ptr );
	memset( argv, 0, sizeof(argv) );
	for(i=0; arg_ptr[i]!='\0'; i++) 
	{
		if(arg_ptr[i]==' ')
		{
			argv[argc][arg_idx]='\0';
			if(bfdcfg_dbgflag) printf( "%s> argc=%d, %s\n", __FUNCTION__, argc, argv[argc] );
			argc++;
			arg_idx=0;
		}else{
			if(arg_idx<MAX_ARG_LEN)
			{
				argv[argc][arg_idx]=arg_ptr[i];
				arg_idx++;
			}
		}
	}
	argv[argc][arg_idx]='\0';
	if(bfdcfg_dbgflag) printf( "%s> argc=%d, %s\n", __FUNCTION__, argc, argv[argc] );

	memset( &cfg, 0, sizeof(cfg) );
	cfg.bfdcmd=BFDCMD_NONE;
	for(c=0; commands[c].name!=NULL; c++) 
	{
		for(i=1; i<argc; i++) 
		{
			if(!strcmp(argv[i], commands[c].name)) 
			{
				int ret;
				if(commands[c].needs_arg)
				{
					i++;
					ret=commands[c].func(argv[i]);
				}else{
					ret=commands[c].func(NULL);
				}
				
				//error, stop processiong this message
				if(ret)
				{
					printf( "%s> handle cmd:'%s' return error\n", __FUNCTION__, commands[c].name );
					return;
				}
			}
		}
	}
	
	switch(cfg.bfdcmd)
	{
	case BFDCMD_SHOW:
		if(bfdcfg_dbgflag) printf( "%s> bfdcmd=BFDCMD_SHOW\n", __FUNCTION__ );
		bfdcfg_showbfd( &cfg );
		break;
	case BFDCMD_ADD:
		if(bfdcfg_dbgflag) printf( "%s> bfdcmd=BFDCMD_ADD\n", __FUNCTION__ );
		bfdcfg_addbfd( &cfg );
		break;
	case BFDCMD_DEL:
		if(bfdcfg_dbgflag) printf( "%s> bfdcmd=BFDCMD_DEL\n", __FUNCTION__ );
		bfdcfg_delbfd( &cfg );
		break;
	case BFDCMD_DBG:
		if(bfdcfg_dbgflag) printf( "%s> bfdcmd=BFDCMD_DBG\n", __FUNCTION__ );
		bfdcfg_dbgbfd( &cfg );
		break;
	case BFDCMD_NONE:
	default:
		printf( "%s> unknown bfdcmd=%d\n", __FUNCTION__, cfg.bfdcmd );
		break;
	}

	return;
}	

/*************************************************************************************/
#define BFDMAIN_RUNFILE "/var/run/bfdmain.pid"
static void bfdcfg_log_pid()
{
	FILE *f;
	pid_t pid;
	char *pidfile = BFDMAIN_RUNFILE;

	pid = getpid();
	if((f = fopen(pidfile, "w")) == NULL)
		return;
	fprintf(f, "%d\n", pid);
	fclose(f);
}
static void bfdcfg_clr_pid()
{
	FILE *f;
	char *pidfile = BFDMAIN_RUNFILE;

	if((f = fopen(pidfile, "r")) != NULL){
		fclose(f);
		unlink(pidfile);
	}
}

static void bfdcfg_handle_term()
{
	bfdcfg_clr_pid();
	exit(0);
}

/*************************************************************************************/
void bfdcfg_process_bfdlist(void)
{
	struct bfdsessiondata *p;
	p=pbfdlist;

	if(p)
	{
	 	while(p)
	 	{
	 		bfd_process(p);
	 		p=p->next;
	 	}
	}else{
		struct timespec stop_time={0, 20*1000*1000};
		nanosleep( &stop_time, NULL );
	}

}

int main(int argc, char **argv)
{
	bfdcfg_log_pid();

	signal( SIGTERM,bfdcfg_handle_term);
	signal( SIGHUP,SIG_IGN);
	signal( SIGINT,SIG_IGN);
	signal( SIGPIPE,SIG_IGN);
	signal( SIGALRM,SIG_IGN);
	signal( SIGUSR1,SIG_IGN);
	signal( SIGUSR2,SIG_IGN);

	//may need higher priority
	if(setpriority( PRIO_PROCESS, 0, -10 )<0)
		printf("%s> setpriority() error errno=%d(%s)\n", __FUNCTION__, errno, strerror(errno));
	
	bfdcfg_init_msg();
	while(1)
	{
		bfdcfg_handle_msg();
		bfdcfg_process_bfdlist();
		bfd_calltimeout();
	}
	return 0;
}

