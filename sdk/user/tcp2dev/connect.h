#ifndef     CONNECT_H
#define     CONNECT_H

#define     Eopensocket 1
#define     Ehostname 2
#define     Eprotocol 3
#define     Eservice 4
#define     Econnect 5
#define     Ebind 6
#define     Elisten 7

#ifdef __cplusplus
    extern "C" {
#endif

int connectsocket(const char *host, const char *service, const char *protocol);
int passivesocket(const char *service, const char *protocol, int qlen);
extern int sock_error;

#ifdef __cplusplus
    }
#endif


#endif
