#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

#include "dns_task.h"
#include "memwatch.h"

static int server_qid = -1;
static int client_qid = -1;

static void (*dbgprintf)(const int level, const char *fmt, ...) = NULL;
#define DBG_PRINTF(...) do { if (dbgprintf) dbgprintf(__VA_ARGS__); } while (0)

enum {
	DNS_RESPONSE_FAILED = -1,
	DNS_RESPONSE_OK = 0,
	DNS_RESPONSE_BUSY,
	DNS_RESPONSE_TRYING
};

void my_freeaddrinfo(struct addrinfo *addrinfo);

int my_getaddrinfo_init(void (*my_printf)(const int level, const char *fmt, ...))
{
	int nRetry;
	key_t key_server, key_client;

	key_server = ftok(DNS_SERVER_NAME, DNS_TASK_VER);
	if (key_server == -1)
	{
		fprintf(stderr, "ftok failed: %s\n", strerror(errno));
		return -1;
	}

	key_client = ftok(DNS_CLIENT_NAME, DNS_TASK_VER);
	if (key_client == -1)
	{
		fprintf(stderr, "ftok failed: %s\n", strerror(errno));
		return -1;
	}

	nRetry = 0;
	while (1)
	{
		server_qid = msgget(key_server, 0666);
		if (server_qid == -1 && errno != ENOENT)
		{
			fprintf(stderr, "msgget failed(%s)\n", strerror(errno));
			return -1;
		}

		client_qid = msgget(key_client, 0666);
		if (client_qid == -1 && errno != ENOENT)
		{
			fprintf(stderr, "msgget failed(%s)\n", strerror(errno));
			return -1;
		}

		if (server_qid != -1 && client_qid != -1)
			break;

		fprintf(stdout, "waiting dns task start... (%d)\n", nRetry);
		if (nRetry++ < 3)
			sleep(1);
		else
			sleep(3);
	}

	dbgprintf = my_printf;
	return 0;
}

void my_getaddrinfo_destroy(void)
{
	dbgprintf = NULL;
}

static int send_dns_request(const int status, const char *hostname, const int port)
{
	TDNS_Message msg;

	memset(&msg, 0, sizeof(msg));
	msg.msg_key = getpid();
	msg.status = status;
	msg._errno = 0;
	strncpy(msg.record.hostname, hostname, sizeof(msg.record.hostname) - 1);
	msg.record.port = port;

	DBG_PRINTF(DBG_TRACE, "sending message to queue %d, key=%d\n",
		server_qid, msg.msg_key);

	if (msgsnd(server_qid, (void *) &msg, sizeof(msg), 0) == -1)
	{
		DBG_PRINTF(DBG_ERROR, "send_dns_request failed in %d lines of %s\n", __LINE__, __FILE__);
		return -1;
	}

	return 0;
}

static int recv_dns_response(int port, struct addrinfo **res, struct hostent **host)
{
	TDNS_Message *msg;
	int i;
	struct addrinfo *addrinfo, *ai_last;
	// max entry: ipv6 = 16, ipv4 = 40 (320/8), +4 for null terminated
	static char host_buffer[sizeof(struct hostent) + 16 * (4 + 16) + 4]; 
	char **ppbuf = (void *) (host_buffer + sizeof(struct hostent));
	char *pbuf;

	msg = (TDNS_Message *) malloc(MAX_DNS_MSGSIZE);
	if (msg == NULL)
	{
		DBG_PRINTF(DBG_ERROR, "dns_msg_new failed in %d lines of %s\n", __LINE__, __FILE__);
		return DNS_RESPONSE_FAILED;
	}

	DBG_PRINTF(DBG_TRACE, "waiting message in queue %d\n", client_qid);
	if (msgrcv(client_qid, (void *) msg, MAX_DNS_MSGSIZE, getpid(), 0) == -1) 
	{
		DBG_PRINTF(DBG_ERROR, "msgrcv failed in %d lines of %s\n", __LINE__, __FILE__);
		free(msg);
		return DNS_RESPONSE_FAILED;
	}
	DBG_PRINTF(DBG_TRACE, "message is received in queue %d\n", client_qid);

	if (msg->status == DNS_QUERY_BUSY)
	{
		DBG_PRINTF(DBG_ERROR, "dns server is busy\n");
		free(msg);
		return DNS_RESPONSE_BUSY;
	}
	else if (msg->status == DNS_QUERY_TRYING)
	{
		DBG_PRINTF(DBG_INFO, "dns server is trying\n");
		free(msg);
		return DNS_RESPONSE_TRYING;
	}
	else if (msg->record.port && msg->record.service.ai_count == 0)
	{
		if (msg->_errno == EAI_AGAIN)
		{
			DBG_PRINTF(DBG_INFO, "dns server is trying\n");
			free(msg);
			return DNS_RESPONSE_TRYING;
		}

		DBG_PRINTF(DBG_INFO, "getaddrinfo error\n");
		free(msg);
		return DNS_RESPONSE_FAILED;
	}
	else if (msg->record.port == 0 && msg->record.host.h_count == 0)
	{
		if (msg->_errno == TRY_AGAIN)
		{
			DBG_PRINTF(DBG_INFO, "dns server is trying\n");
			free(msg);
			return DNS_RESPONSE_TRYING;
		}

		DBG_PRINTF(DBG_INFO, "gethostbyname error\n");
		free(msg);
		return DNS_RESPONSE_FAILED;
	}

	// gethostbyname
	if (port == 0)
	{
		if ((msg->record.host.h_addrtype == AF_INET && msg->record.host.h_count > 40) ||
			(msg->record.host.h_addrtype == AF_INET6 && msg->record.host.h_count > 16))
		{
			DBG_PRINTF(DBG_INFO, "gethostbyname error: too many entry (%d)\n", msg->record.host.h_count);
			free(msg);
			return DNS_RESPONSE_FAILED;
		}

		(*host) = (struct hostent *) (void *) host_buffer;
		(*host)->h_name = NULL;
		(*host)->h_aliases = NULL;
		(*host)->h_addrtype = msg->record.host.h_addrtype;
		(*host)->h_length = msg->record.host.h_length;
		(*host)->h_addr_list = ppbuf;
		ppbuf += msg->record.host.h_count + 1; // +1 for NULL terminated => (*host)->h_addr_list[i]
		for (i=0, pbuf = (void *) ppbuf; i<msg->record.host.h_count; i++, pbuf += msg->record.host.h_length)
		{
			memcpy(pbuf, &msg->record.host.hostinfos[i]._h_addr, msg->record.host.h_length);
			(*host)->h_addr_list[i] = pbuf;
		}
		(*host)->h_addr_list[i] = NULL;

		free(msg);	
		return DNS_RESPONSE_OK;
	}

	// getaddrinfo
	addrinfo = NULL;
	ai_last = NULL;
	for (i=msg->record.service.ai_count - 1; i>=0; i--)
	{
		addrinfo = (struct addrinfo *) malloc(sizeof(*addrinfo));
		if (addrinfo == NULL)
		{
			DBG_PRINTF(DBG_ERROR, "malloc addrinfo failed in %d lines of %s\n", __LINE__, __FILE__);
			my_freeaddrinfo(ai_last);
			free(msg);
			return DNS_RESPONSE_FAILED;
		}

		memset(addrinfo, 0, sizeof(*addrinfo));
		addrinfo->ai_flags = msg->record.service.addrinfos[i].ai_flags;
		addrinfo->ai_family = msg->record.service.addrinfos[i].ai_family;
		addrinfo->ai_socktype = msg->record.service.addrinfos[i].ai_socktype;
		addrinfo->ai_protocol = msg->record.service.addrinfos[i].ai_protocol;
		addrinfo->ai_addrlen = msg->record.service.addrinfos[i].ai_addrlen;
		addrinfo->ai_addr = (struct sockaddr *) malloc(addrinfo->ai_addrlen);
		if (addrinfo->ai_addr == NULL)
		{
			DBG_PRINTF(DBG_ERROR, "malloc ai_addr failed in %d lines of %s\n", __LINE__, __FILE__);
			free(addrinfo);
			my_freeaddrinfo(ai_last);
			free(msg);
			return DNS_RESPONSE_FAILED;
		}

		memcpy(addrinfo->ai_addr, &msg->record.service.addrinfos[i].ai_addr, addrinfo->ai_addrlen);
		addrinfo->ai_next = ai_last;
		ai_last = addrinfo;
	}

	*res = addrinfo;
	free(msg);	
	return DNS_RESPONSE_OK;
}

void my_freeaddrinfo(struct addrinfo *addrinfo)
{
	struct addrinfo *next;

	while (addrinfo)
	{
		if (addrinfo->ai_addr)
			free(addrinfo->ai_addr);

		if (addrinfo->ai_canonname)
			free(addrinfo->ai_canonname);

		next = addrinfo->ai_next;
		free(addrinfo);
		addrinfo = next;
	}
}

// no cache, and blocking
int my_getaddrinfo1(
	struct addrinfo **res,
	const char *node, 
	const int service
	)
{
	int cnt;

	for (cnt=0;;cnt++)
	{
		if (send_dns_request(DNS_QUERY_REQUEST, node, service) != 0)
			return EAI_SYSTEM;

		switch (recv_dns_response(service, res, NULL))
		{
		case DNS_RESPONSE_OK:
			return 0;
		case DNS_RESPONSE_BUSY:
		case DNS_RESPONSE_TRYING:
			sleep(1);
			DBG_PRINTF(DBG_INFO, "resolve %s retry %d times...\n", node, cnt+1);
			continue; // try again after 1 second
		default:
			return EAI_SYSTEM;
		}
	}
	return 0;
}

// cache, and blocking
int my_getaddrinfo2(
	struct addrinfo **res,
	const char *node, 
	const int service
	)
{
	int cnt;

	for (cnt=0;;cnt++)
	{
		if (send_dns_request(DNS_QUERY_REQUEST2, node, service) != 0)
			return EAI_SYSTEM;

		switch (recv_dns_response(service, res, NULL))
		{
		case DNS_RESPONSE_OK:
			return 0;
		case DNS_RESPONSE_BUSY:
		case DNS_RESPONSE_TRYING:
			sleep(1);
			DBG_PRINTF(DBG_INFO, "resolve %s retry %d times...\n", node, cnt+1);
			continue; // try again after 1 second
		default:
			return EAI_SYSTEM;
		}
	}
	return 0;
}

// cache, and non-blocking
int my_getaddrinfo3(
	struct addrinfo **res,
	const char *node, 
	const int service
	)
{
	if (send_dns_request(DNS_QUERY_REQUEST3, node, service) != 0)
		return EAI_SYSTEM;

	switch (recv_dns_response(service, res, NULL))
	{
	case DNS_RESPONSE_OK:
		return 0;
	case DNS_RESPONSE_BUSY:
	case DNS_RESPONSE_TRYING:
		return EAI_AGAIN;
	default:
		return EAI_SYSTEM;
	}

	return EAI_SYSTEM;
}

static int g_errno = 0;

// no cache, and blocking
struct hostent *my_gethostbyname1(const char *name)
{
	int cnt;
	struct hostent *res;

	for (cnt=0;;cnt++)
	{
		if (send_dns_request(DNS_QUERY_REQUEST, name, 0) != 0)
		{
			g_errno = -1;
			return NULL;
		}

		switch (recv_dns_response(0, NULL, &res))
		{
		case DNS_RESPONSE_OK:
			g_errno = 0;
			return res;
		case DNS_RESPONSE_BUSY:
		case DNS_RESPONSE_TRYING:
			sleep(1);
			DBG_PRINTF(DBG_INFO, "resolve %s retry %d times...\n", name, cnt+1);
			continue; // try again after 1 second
		default:
			g_errno = -1;
			return NULL;
		}
	}

	g_errno = -1;
	return NULL;
}

// cache, and blocking
struct hostent *my_gethostbyname2(const char *name)
{
	int cnt;
	struct hostent *res;

	for (cnt=0;;cnt++)
	{
		if (send_dns_request(DNS_QUERY_REQUEST2, name, 0) != 0)
		{
			g_errno = -1;
			return NULL;
		}

		switch (recv_dns_response(0, NULL, &res))
		{
		case DNS_RESPONSE_OK:
			g_errno = 0;
			return res;
		case DNS_RESPONSE_BUSY:
		case DNS_RESPONSE_TRYING:
			sleep(1);
			DBG_PRINTF(DBG_INFO, "resolve %s retry %d times...\n", name, cnt+1);
			continue; // try again after 1 second
		default:
			g_errno = -1;
			return NULL;
		}
	}

	g_errno = -1;
	return NULL;
}

struct hostent *my_gethostbyname3(const char *name)
{
	struct hostent *res;

	if (send_dns_request(DNS_QUERY_REQUEST3, name, 0) != 0)
	{
		g_errno = -1;
		return NULL;
	}

	switch (recv_dns_response(0, NULL, &res))
	{
	case DNS_RESPONSE_OK:
		g_errno = 0;
		return res;
	case DNS_RESPONSE_BUSY:
	case DNS_RESPONSE_TRYING:
		g_errno = EAGAIN;
		return NULL;
	default:
		g_errno = -1;
		return NULL;
	}

	g_errno = -1;
	return NULL;
}

struct hostent *my_gethostbyname(const char *name)
{
	return my_gethostbyname3(name);
}

int my_getaddrinfo(
	const char *node, 
	const char *service,
	const struct addrinfo *hints,
	struct addrinfo **res
	)
{
	struct in_addr addr;
	int port;
#ifdef DISABLE_SRV
	int i, count;
	struct hostent *hptr;
	char **pptr;
	struct addrinfo *addrinfo, *ai_last;
	struct sockaddr_in *address;
	struct sockaddr_in6 *address6;
#endif

	if (inet_pton(AF_INET, node, &addr)>0)
	{
		struct addrinfo *ai;
		struct addrinfo *addrinfo;
		int err;

		// ipv4 address detected, call getaddrinfo directly
		err = getaddrinfo(node, service, hints, &ai);
		if (err != 0)
		{
			DBG_PRINTF(DBG_ERROR, "getaddrinfo failed in %d lines of %s\n", __LINE__, __FILE__);
			return err;
		}

		addrinfo = (struct addrinfo *) malloc(sizeof(*addrinfo));
		if (addrinfo == NULL)
		{
			DBG_PRINTF(DBG_ERROR, "malloc addrinfo failed in %d lines of %s\n", __LINE__, __FILE__);
			return EAI_SYSTEM;
		}

		memset(addrinfo, 0, sizeof(*addrinfo));
		addrinfo->ai_flags = ai->ai_flags;
		addrinfo->ai_family = ai->ai_family;
		addrinfo->ai_socktype = ai->ai_socktype;
		addrinfo->ai_protocol = ai->ai_protocol;
		addrinfo->ai_addrlen = ai->ai_addrlen;
		addrinfo->ai_addr = (struct sockaddr *) malloc(addrinfo->ai_addrlen);
		if (addrinfo->ai_addr == NULL)
		{
			DBG_PRINTF(DBG_ERROR, "malloc ai_addr failed in %d lines of %s\n", __LINE__, __FILE__);
			free(addrinfo);
			freeaddrinfo(ai);
			return EAI_SYSTEM;
		}
		memcpy(addrinfo->ai_addr, ai->ai_addr, ai->ai_addrlen);
		freeaddrinfo(ai);
		*res = addrinfo;
		return 0;
	}

	port = 0;
	if (service)
		port = atoi(service);

#ifdef DISABLE_SRV
	hptr = my_gethostbyname(node);
	if (hptr == NULL)
	{
		if (g_errno == EAGAIN)
			return EAI_AGAIN;
		
		return EAI_SYSTEM;
	}

	pptr = hptr->h_addr_list;
	for (count=0; *pptr; count++, pptr++);

	addrinfo = NULL;
	ai_last = NULL;
	for (i=count - 1; i>=0; i--)
	{
		addrinfo = (struct addrinfo *) malloc(sizeof(*addrinfo));
		if (addrinfo == NULL)
		{
			DBG_PRINTF(DBG_ERROR, "malloc addrinfo failed in %d lines of %s\n", __LINE__, __FILE__);
			my_freeaddrinfo(ai_last);
			return EAI_SYSTEM;
		}

		memset(addrinfo, 0, sizeof(*addrinfo));

		if (hints)
		{
			addrinfo->ai_flags = hints->ai_flags;
			addrinfo->ai_family = hints->ai_family;
			addrinfo->ai_socktype = hints->ai_socktype;
			addrinfo->ai_protocol = hints->ai_protocol;
		}

		if (hptr->h_addrtype == AF_INET)
			addrinfo->ai_addrlen = sizeof(struct sockaddr_in);
		else
			addrinfo->ai_addrlen = sizeof(struct sockaddr_in6);

		addrinfo->ai_addr = (struct sockaddr *) malloc(addrinfo->ai_addrlen);
		if (addrinfo->ai_addr == NULL)
		{
			DBG_PRINTF(DBG_ERROR, "malloc ai_addr failed in %d lines of %s\n", __LINE__, __FILE__);
			free(addrinfo);
			my_freeaddrinfo(ai_last);
			return EAI_SYSTEM;
		}

		if (hptr->h_addrtype == AF_INET)
		{
			address = (struct sockaddr_in *) addrinfo->ai_addr;
			memset(address, 0, sizeof(struct sockaddr_in));
			address->sin_family = AF_INET;
			address->sin_port = (unsigned short int) port;
			memcpy(&address->sin_addr, hptr->h_addr_list[i], hptr->h_length);
		}
		else
		{
			address6 = (struct sockaddr_in6 *) addrinfo->ai_addr;
			memset(address6, 0, sizeof(struct sockaddr_in6));
			address6->sin6_family = AF_INET6;
			address6->sin6_port = (unsigned short int) port;
			memcpy(&address6->sin6_addr, hptr->h_addr_list[i], hptr->h_length);
		}

		addrinfo->ai_next = ai_last;
		ai_last = addrinfo;
	}

	*res = addrinfo;
	return 0;
#else	
	return my_getaddrinfo3(res, node, port);
#endif
}

