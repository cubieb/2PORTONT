#ifndef __STUN_CLIENT_H
#define __STUN_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif


#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>



void stun_SetServer(unsigned int addr, unsigned short port);

int stun_GetPublicAddr(unsigned int local_addr, unsigned short local_port,
	unsigned int *public_addr, unsigned short *public_port, int jfd, int sip_port, int qos);

#ifdef NEW_STUN
int stun_sendmessage(int fd, unsigned int ip, unsigned short port);
int stun_readmessage(int fd, char **buf, int *len);
int stun_handlemessage(char *buf, int len, unsigned int *mapIP, unsigned short *mapPort);
#endif

#if 0
int stun_test(void);
#endif




extern int stun_message_on;

static inline void stun_message(const char *fmt,...)
{
#if 1
  va_list args;

  if (!stun_message_on)
    return;

  fprintf(stdout,"STUN Message:");
  va_start (args, fmt);
  vfprintf (stdout, fmt, args);
  va_end (args);
  fprintf(stdout,"\n");
#endif
}

#ifdef __cplusplus
}
#endif

#endif
