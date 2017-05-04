#ifndef __DNS_API_H
#define __DNS_API_H

#include <sys/socket.h>
#include <netdb.h>
#include "dns_task.h"

int my_getaddrinfo_init(void (*my_printf)(const int level, const char *fmt, ...));
void my_getaddrinfo_destroy(void);

int my_getaddrinfo(
	const char *node, 
	const char *service,
	const struct addrinfo *hints,
	struct addrinfo **res
	);

int my_getaddrinfo1(
	struct addrinfo **res,
	const char *node, 
	const int service
	);

int my_getaddrinfo2(
	struct addrinfo **res,
	const char *node, 
	const int service
	);

int my_getaddrinfo3(
	struct addrinfo **res,
	const char *node, 
	const int service
	);

void my_freeaddrinfo(struct addrinfo *addrinfo);

struct hostent *my_gethostbyname(const char *name);
struct hostent *my_gethostbyname1(const char *name);
struct hostent *my_gethostbyname2(const char *name);
struct hostent *my_gethostbyname3(const char *name);

#undef getaddrinfo
#undef freeaddrinfo
#define getaddrinfo my_getaddrinfo
#define freeaddrinfo my_freeaddrinfo

#endif
