#ifndef _UDP_ECHO_SERVER_LIB_H_
#define _UDP_ECHO_SERVER_LIB_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

#define MAXLINE 4096
#define ECHOTOK "/bin/udpechoserver"

struct ECHOPLUS
{
	unsigned int	TestGenSN;
	unsigned int	TestRespSN;
	unsigned int	TestRespRecvTimeStamp;
	unsigned int	TestRespReplyTimeStamp;
	unsigned int	TestRespReplyFailureCount;
};

struct ECHORESULT
{
	unsigned int	TestRespSN;
	unsigned int	TestRespReplyFailureCount;
	unsigned int	PacketsReceived;
	unsigned int	PacketsResponded;
	unsigned int	BytesReceived;
	unsigned int	BytesResponded;
	struct timeval	TimeFirstPacketReceived;
	struct timeval	TimeLastPacketReceived;
};

extern struct ECHORESULT *gEchoResult;

extern int initShmem( void **t, int s, char *name );
extern int getShmem( void **t, int s, char *name );
extern int detachShmem( void *t );

#endif //_UDP_ECHO_SERVER_LIB_H_
