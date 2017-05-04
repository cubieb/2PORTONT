#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "memwatch.h"
#include "dns_api.h"

void dbgprintf(const int level, const char *format, ...)
{
	va_list args;

	if (level > DBG_CURRENT)
		return;

	fprintf(stderr, "%d :- ", getpid());
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
}

int main(int argc, char *argv[])
{
	int res;
	struct addrinfo *addrinfo, *next;
	int port;
	char hbuf[NI_MAXHOST];
	struct hostent *hptr = NULL;

	if (argc != 2 && argc != 3)
	{
		fprintf(stderr, "Usage: %s hostname [port]\n", argv[0]);
		exit(0);
	}

	if (my_getaddrinfo_init(dbgprintf) != 0)
	{
		fprintf(stderr, "my_getaddrinfo_init failed\n");
		return -1;
	}

	if (argc == 3)
		port = atoi(argv[2]);
	else
		port = 0;

	if (strstr(argv[0], "dns_test2"))
	{
		fprintf(stderr, "\nmy_getaddrinfo2 %s\n", argv[1]);
		if (port)
		{
			res = my_getaddrinfo2(&addrinfo, argv[1], port);
		}
		else
		{
			hptr = my_gethostbyname2(argv[1]);
			res = (hptr == NULL);
		}
	}
	else if (strstr(argv[0], "dns_test3"))
	{
		fprintf(stderr, "\nmy_getaddrinfo3 %s\n", argv[1]);
		if (port)
		{
			res = my_getaddrinfo3(&addrinfo, argv[1], port);
		}
		else
		{
			hptr = my_gethostbyname3(argv[1]);
			res = (hptr == NULL);
		}
	}
	else if (strstr(argv[0], "dns_test1"))
	{
		fprintf(stderr, "\nmy_getaddrinfo1 %s\n", argv[1]);
		if (port)
		{
			res = my_getaddrinfo1(&addrinfo, argv[1], port);
		}
		else
		{
			hptr = my_gethostbyname1(argv[1]);
			res = (hptr == NULL);
		}
	}
	else
	{
		fprintf(stderr, "\nmy_getaddrinfo %s\n", argv[1]);
		if (port)
		{
			res = my_getaddrinfo(argv[1], argv[2], NULL, &addrinfo);
		}
		else
		{
			hptr = my_gethostbyname(argv[1]);
			res = (hptr == NULL);
		}
	}

	if (res != 0)
	{
		if (port)
			fprintf(stderr, "mygetaddrinfo failed\n");
		else
			fprintf(stderr, "mygethostbyname failed\n");
	}
	else if (port)
	{
		if (addrinfo == NULL)
		{
			fprintf(stderr, "addrinfo is NULL!\n");
			return -1;
		}

		next = addrinfo;
		while (next)
		{
			res = getnameinfo(next->ai_addr, next->ai_addrlen, 
				hbuf, sizeof(hbuf), NULL, 0, NI_NUMERICHOST);

			if (res == 0)
				fprintf(stderr, "resolve %s is %s\n", argv[1], hbuf);

			next = next->ai_next;
		}

		my_freeaddrinfo(addrinfo);
	}
	else
	{
		char **pptr;
		char str[32];

		pptr = hptr->h_addr_list;
		for(; *pptr!=NULL; pptr++)
		{
			fprintf(stderr, "resolve %s is %s\n", 
				argv[1], inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str)));
		}
	}

	my_getaddrinfo_destroy();
	return 0;
}

