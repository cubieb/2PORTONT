#include <stdio.h>
#include <unistd.h>
#include "bfdlib.h"
#include "bfdsocket.h"
#include "bfdtimeout.h"

//#define TEST_SERVER

int main(int argc, char **argv)
{
	struct bfdsessiondata bsd;
	struct bfdsessiondata *pb;
	struct timeval tv;

	//bfd_test_md5hash();
	//bfd_test_sha1hash();
	
	pb = &bsd;
	bfd_session_init( pb );
	bfd_echo_init(pb);
	//pb->session.LocalEchoRxInterval=2000000;
#ifdef TEST_SERVER
    #ifdef _TEST_ON_ONE_PC_
	pb->LocalRecvPort=13784;
	pb->LocalSendPort=59512;
	pb->LocalSendFinalPort=59513;
	pb->EchoPort=13785;
    #endif //_TEST_ON_ONE_PC_
	pb->Role=BFD_ACTIVE_ROLE;
	pb->session.DemandMode=0;
#else
    #ifdef _TEST_ON_ONE_PC_
	pb->RemotePort=13784;
    #endif //_TEST_ON_ONE_PC_
	pb->Role=BFD_PASSIVE_ROLE;
	pb->session.DemandMode=1;
#endif

	pb->debug=BFD_ON;
	//pb->LocalIP
	if(argc>=2)
	{
		strncpy( pb->RemoteIP, argv[1], 15);
		pb->RemoteIP[15]=0;
	}else{
		strcpy( pb->RemoteIP, "172.21.146.1");
	}
	printf( "set pb->RemoteIP=%s\n", pb->RemoteIP );
	if(argc>=3)
	{
		strncpy( pb->Interface, argv[2], 15);
		pb->Interface[15]=0;
	}	
	printf( "set pb->Interface=%s\n", pb->Interface );

#define	_AUTHKEY_ "11223344556677889900"
	//auth part
	pb->session.AuthType=AUTH_NONE;//AUTH_KEYED_MD5;
	if(pb->session.AuthType!=AUTH_NONE)
	{
		pb->AuthKeyID=1;
		switch(pb->session.AuthType)
		{
		case AUTH_PASSWORD:
			pb->AuthKeyLen=4;/*1~16*/
			break;
		case AUTH_KEYED_MD5:
		case AUTH_METI_KEYED_MD5:
			pb->AuthKeyLen=16;
			break;
		case AUTH_KEYED_SHA1:
		case AUTH_METI_KEYED_SHA1:
			pb->AuthKeyLen=20;
			break;
		}
		memcpy( pb->AuthKey, _AUTHKEY_, pb->AuthKeyLen );
	}

	pb->LocalRecvSock = bfd_bind(pb->LocalRecvPort, pb->Interface);
	if(pb->LocalRecvSock<0)
	{
		printf( "bind pb->LocalRecvSock error\n" );
		return -1;
	}
	bfd_setsockopt( pb->LocalRecvSock, pb->dscp, pb->ttl, pb->ethprio );
	
	pb->LocalSendSock = bfd_bind(pb->LocalSendPort, pb->Interface);
	if(pb->LocalSendSock<0)
	{
		printf( "bind pb->LocalRecvSock error\n" );
		return -1;
	}
	bfd_setsockopt( pb->LocalSendSock, pb->dscp, pb->ttl, pb->ethprio );
	
	pb->LocalSendFinalSock = bfd_bind(pb->LocalSendFinalPort, pb->Interface);
	if(pb->LocalSendFinalSock<0)
	{
		printf( "bind pb->LocalSendFinalSock error\n" );
		return -1;
	}
	bfd_setsockopt( pb->LocalSendFinalSock, pb->dscp, pb->ttl, pb->ethprio );

	bfd_loop(pb);	
	
	return 0;
};



