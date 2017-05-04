// this version is supporting TCP and UDP
#include    "connect.h"
#include    <stdio.h>
#include    <sys/types.h>
#include    <sys/socket.h>
#include    <netinet/in.h>
#include    <netdb.h>
#include    <sys/time.h> 
#include    <sys/types.h>
#include    <sys/ioctl.h>
#include    <arpa/inet.h>
#include    <string.h>
#include    <strings.h>
#include    <unistd.h>
#include    <stdlib.h>

#ifndef     INADDR_NONE
#define     INADDR_NONE 0xffffffff
#endif      

#define TCP_PROTO_NUM  6

int sock_error;
/*
unsigned short htons ();
unsigned char inet_addr ();
*/
int 
connectsocket(const char *host, const char *service, const char *protocol) {
	struct hostent *phe;
	struct servent *pse;
	struct protoent *ppe;
	struct sockaddr_in sin;	
	int s, type;

	bzero( (char*)&sin,sizeof(sin));
	sin.sin_family = AF_INET;
	if ( (pse = getservbyname(service,protocol)))
		sin.sin_port = pse->s_port;
	else if ( ( sin.sin_port = htons (( unsigned short) atoi (service))) == 0 )
		{ sock_error = Eservice; return(-1);}

	if ( (phe = gethostbyname(host)) )
		bcopy(phe->h_addr,(char*)&sin.sin_addr,phe->h_length);
	else if ((sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE)
		{ sock_error = Ehostname; return (-1);}

//	if ((ppe = getprotobyname(protocol )) == 0) 
//		{ sock_error = Eprotocol; return (-1);}

	if (strcmp(protocol,"tcp") == 0)
		type = SOCK_STREAM;
	else 
		type = SOCK_DGRAM;

//	s = socket (PF_INET, type , ppe->p_proto);
	s = socket (PF_INET, type , TCP_PROTO_NUM);
	if (s < 0) { sock_error = Eopensocket; return(-1);}

	if (type==SOCK_STREAM)
	if (connect(s,(struct sockaddr *)&sin,sizeof(sin)) < 0 )
		{ sock_error = Econnect; return(-1);}
	return(s);
}
int 
passivesocket(const char *service, const char *protocol, int qlen) {
	struct protoent *ppe;
	struct sockaddr_in sin;	
	int s, type;
	int i=1;

	bzero( (char*)&sin,sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	if ( ( sin.sin_port = htons (( unsigned short) atoi (service))) == 0 )
		{ sock_error = Eservice; return(-1);}

//	if ((ppe = getprotobyname(protocol )) == 0) 
//		{ sock_error = Eprotocol; return (-1);}

	if (strcmp(protocol,"tcp") == 0)
		type = SOCK_STREAM;
	else 
		type = SOCK_DGRAM;

//	s = socket (PF_INET, type , ppe->p_proto);
	s = socket (PF_INET, type , TCP_PROTO_NUM);
	if (s < 0) { sock_error = Eopensocket; return(-1);}

	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const void*)&i, sizeof(int));

	if (bind(s,(struct sockaddr *)&sin,sizeof(sin)) < 0 )
		{ sock_error = Ebind; return(-1);}
	if (type == SOCK_STREAM && listen (s,qlen) < 0 )
		{ sock_error = Elisten; return (-1);}
	return(s);
}
