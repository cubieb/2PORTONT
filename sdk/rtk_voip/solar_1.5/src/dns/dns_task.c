
#include <stdio.h>
#include <stdarg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "memwatch.h"
#include "dns_task.h"

static char *pidfile = "/var/run/dns_task.pid";
static TDNS_Task dns_task;

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

void dns_task_abort(int sig)
{
	dns_task_destroy(&dns_task);
//	signal(SIGINT, SIG_DFL);
	exit(0);
}

static int pidfile_acquire(char *pidfile)
{
	int pid_fd;

	if(pidfile == NULL)
		return -1;

	pid_fd = open(pidfile, O_CREAT | O_WRONLY, 0644);
	if (pid_fd < 0) 
		printf("Unable to open pidfile %s\n", pidfile);
	else 
		lockf(pid_fd, F_LOCK, 0);

	return pid_fd;
}

static void pidfile_write_release(int pid_fd)
{
	FILE *out;

	if(pid_fd < 0)
		return;

	if((out = fdopen(pid_fd, "w")) != NULL) {
		fprintf(out, "%d\n", getpid());
		fclose(out);
	}
	lockf(pid_fd, F_UNLCK, 0);
	close(pid_fd);
}

int main(int argc, char *argv[])
{
	TDNS_Message *msg;
	TDNS_Cache *cache;

	// destroy old process and create a PID file
	{
		int pid_fd;
		FILE *fp;
		char line[20];
		pid_t pid;

		if ((fp = fopen(pidfile, "r")) != NULL) {
			fgets(line, sizeof(line), fp);
			if (sscanf(line, "%d", &pid)) {
				if (pid > 1)
					kill(pid, SIGTERM);
			}
			fclose(fp);
		}

		pid_fd = pidfile_acquire(pidfile);
		if (pid_fd < 0)
			return 0;
		pidfile_write_release(pid_fd);
	}

#ifdef MALLOC_DEBUG
	mwInit("memwatch_dns.log");
	mwDoFlush(1);
#endif

	msg = (TDNS_Message *) malloc(MAX_DNS_MSGSIZE);
	if (msg == NULL)
	{
		dbgprintf(DBG_ERROR, "dns_msg_new failed\n");
		return -1;
	}

	if (dns_task_init(&dns_task) != 0)
		goto dns_task_tone;

	signal(SIGINT, dns_task_abort);

	while (1)
	{
		// wait dns message
		if (dns_msg_recv(&dns_task, msg) != 0)
			goto dns_task_tone;

		// dns_task command
		if (strcmp(msg->record.hostname, "exit") == 0)
		{
			dbgprintf(DBG_INFO, "exit dns_task\n");
			goto dns_task_tone;
		}
		else if (strcmp(msg->record.hostname, "flush") == 0)
		{
			int i;

			for (i=0; i<ARRAY_LEN(dns_task.dns_caches); i++)
				dns_caches_remove(&dns_task, i);
			msg->status = DNS_QUERY_RESPONSE;
			dns_msg_send(&dns_task, msg);
			continue;
		}
		else if (strcmp(msg->record.hostname, "dump") == 0)
		{
			dns_cache_dump(&dns_task);
			msg->status = DNS_QUERY_RESPONSE;
			dns_msg_send(&dns_task, msg);
			continue;
		}

		switch (msg->status)
		{
		case DNS_QUERY_REQUEST:  // no cache + blocking 
		case DNS_QUERY_REQUEST2: // cache + blocking
		case DNS_QUERY_REQUEST3: // cache + non-blocking
			dbgprintf(DBG_INFO, "resolving %s:%d, key=%d\n", 
				msg->record.hostname, msg->record.port, msg->msg_key);

			dns_caches_update(&dns_task);
			cache = dns_caches_find(&dns_task, &msg->record);
			if (cache == NULL)
			{
				if (msg->status == DNS_QUERY_REQUEST3)
				{
					// non-blocking behavior
					msg->status = DNS_QUERY_TRYING;
					dns_msg_send(&dns_task, msg);
					// use dns thread to update DNS
					msg->status = DNS_QUERY_UPDATE;
					dns_thread_query(&dns_task, msg, cache);
					continue;
				}

				if (dns_thread_query(&dns_task, msg, cache) != 0)
				{
					msg->status = DNS_QUERY_BUSY;
					dns_msg_send(&dns_task, msg);
				}
				// else wait response
			}
			else if (cache->status == DNS_FOUND)
			{
				struct timeval now;

				if (msg->status == DNS_QUERY_REQUEST)
				{
					// no cache behavior
					cache->status = DNS_FINDING;
					if (dns_thread_query(&dns_task, msg, cache) != 0)
					{
						msg->status = DNS_QUERY_BUSY;
						dns_msg_send(&dns_task, msg);
					}
					continue;
				}

				// cache found! response dns cache directly
				memcpy(&msg->record, &cache->record, dns_record_size(&cache->record));
				msg->status = DNS_QUERY_RESPONSE;
				dns_msg_send(&dns_task, msg);

				// use dns thread to update DNS
				gettimeofday(&now, NULL);
				if ((cache->_errno == 0) &&
					(MAX_DNS_TIME - (cache->expire_time.tv_sec - now.tv_sec) <= MIN_DNS_TIME))
				{
					// skip dns query
					dbgprintf(DBG_INFO, "resolving %s:%d is skipped: query time is too short\n", 
						msg->record.hostname, msg->record.port);
				}
				else
				{
					dbgprintf(DBG_INFO, "updating %s:%d\n", msg->record.hostname, msg->record.port);
					msg->status = DNS_QUERY_UPDATE;
					cache->status = DNS_UPDATING;
					dns_thread_query(&dns_task, msg, cache);
				}
			}
			else // DNS_FINDING or DNS_UPDATING
			{
				if (msg->status == DNS_QUERY_REQUEST)
				{
					// no cache behavior
					if (dns_thread_query(&dns_task, msg, cache) != 0)
					{
						msg->status = DNS_QUERY_BUSY;
						dns_msg_send(&dns_task, msg);
					}

					continue;
				}

				if (cache->status == DNS_UPDATING)
				{
					// cache found! response old dns cache directly
					dbgprintf(DBG_INFO, "resolving %s:%d is skipped: updating\n", 
						msg->record.hostname, msg->record.port);
					memcpy(&msg->record, &cache->record, dns_record_size(&cache->record));
					msg->status = DNS_QUERY_RESPONSE;
					dns_msg_send(&dns_task, msg);
				}
				else
				{
					// skip dns query
					dbgprintf(DBG_INFO, "resolving %s:%d is skipped: finding\n", 
						msg->record.hostname, msg->record.port);
					msg->status = DNS_QUERY_TRYING;
					dns_msg_send(&dns_task, msg);
				}
			}
			break;
		case DNS_QUERY_RESPONSE: // blocking response
			dbgprintf(DBG_INFO, "%s is resolved.\n", msg->record.hostname);
			dns_caches_add(&dns_task, &msg->record, DNS_FOUND, msg->_errno);
			dns_msg_send(&dns_task, msg);	
			break;
		case DNS_QUERY_UPDATE:
			dbgprintf(DBG_INFO, "%s is resolved.\n", msg->record.hostname);
			dns_caches_add(&dns_task, &msg->record, DNS_FOUND, msg->_errno);
			break;
		}
	}

dns_task_tone:
	dns_task_destroy(&dns_task);
	free(msg);
#ifdef MALLOC_DEBUG
	mwTerm();
#endif
	return 0;
}

int dns_extra_record_size(TDNS_Record const *record)
{
	return (record->port ? 
			record->service.ai_count * sizeof(TDNS_AddrInfo) :
			record->host.h_count * sizeof(TDNS_HostInfo));
}

TDNS_Cache *dns_caches_find(TDNS_Task *task, const TDNS_Record *record)
{
	int i;

	for (i=0; i<ARRAY_LEN(task->dns_caches); i++)
	{
		if (task->dns_caches[i] == NULL)
			continue;
	
		if ((strcmp(task->dns_caches[i]->record.hostname, record->hostname) == 0) &&
			(task->dns_caches[i]->record.port == record->port))
			return task->dns_caches[i];
	}

	return NULL;
}

void dns_caches_remove(TDNS_Task *task, int index)
{
	if (task->dns_caches[index])
	{
		free(task->dns_caches[index]);
		task->dns_caches[index] = NULL;
	}
}

int dns_caches_add(TDNS_Task *task, const TDNS_Record *record, int status, int _errno)
{
	int i, idx_empty;
	int oldest_cahce;

	idx_empty = -1;
	oldest_cahce = -1;
	for (i=0; i<ARRAY_LEN(task->dns_caches); i++)
	{
		if (task->dns_caches[i] == NULL)
		{
			if (idx_empty == -1) idx_empty = i;
			continue;
		}

		if ((strcmp(task->dns_caches[i]->record.hostname, record->hostname) == 0) &&
			(task->dns_caches[i]->record.port == record->port))
		{
			// found in cache, update only
			dns_caches_remove(task, i);
			idx_empty = i;
			break;
		}

		// find oldest cache if force update
		if (oldest_cahce == -1) 
			oldest_cahce = i;
		else if (timercmp(&task->dns_caches[oldest_cahce]->expire_time, &task->dns_caches[i]->expire_time, >))
			oldest_cahce = i;
	}

	if (idx_empty == -1)
	{
		// no empty cache, force update oldest cache
		dns_caches_remove(task, oldest_cahce);
		idx_empty = oldest_cahce;
	}

	task->dns_caches[idx_empty] = dns_cache_new(record);
	if (task->dns_caches[idx_empty] == NULL)
	{
		dbgprintf(DBG_ERROR, "dns_cache_new failed!\n");
		return -1;
	}
	memcpy(&task->dns_caches[idx_empty]->record, record, dns_record_size(record));
	gettimeofday(&task->dns_caches[idx_empty]->expire_time, NULL);
	task->dns_caches[idx_empty]->expire_time.tv_sec += MAX_DNS_TIME;
	task->dns_caches[idx_empty]->status = status;
	task->dns_caches[idx_empty]->_errno = _errno;

	return idx_empty;
}

void dns_caches_update(TDNS_Task *task)
{
	int i;
	struct timeval now;

	// check timeout
	gettimeofday(&now, NULL);
	for (i=0; i<ARRAY_LEN(task->dns_caches); i++)
	{
		if (task->dns_caches[i] == NULL)
			continue;

		// don't remove dns cache if finding or updating (still in thread fifo)
		if (task->dns_caches[i]->status == DNS_FOUND &&
			timercmp(&now, &task->dns_caches[i]->expire_time, >))
		{
			dbgprintf(DBG_INFO, "%s in cache is expired\n", task->dns_caches[i]->record.hostname);
			dns_caches_remove(task, i);
		}
	}
}

void dns_cache_dump(TDNS_Task *task)
{
	int i;
	struct timeval now;
	char hbuf[NI_MAXHOST];

	gettimeofday(&now, NULL);
	dbgprintf(DBG_ERROR, "==== dns cache table (now=%d) \n", now.tv_sec);
	for (i=0; i<ARRAY_LEN(task->dns_caches); i++)
	{
		if (task->dns_caches[i] == NULL)
			continue;

		hbuf[0] = 0;
		if (task->dns_caches[i]->record.port)
		{
			if (task->dns_caches[i]->record.service.ai_count &&
				task->dns_caches[i]->record.service.addrinfos[0].ai_addrlen)
			{
				getnameinfo(
					(struct sockaddr *) &task->dns_caches[i]->record.service.addrinfos[0].ai_addr,
					task->dns_caches[i]->record.service.addrinfos[0].ai_addrlen,
					hbuf, sizeof(hbuf), NULL, 0, NI_NUMERICHOST);
			}
		}
		else
		{
			if (task->dns_caches[i]->record.host.h_count &&
				task->dns_caches[i]->record.host.h_length)
			{
				inet_ntop(task->dns_caches[i]->record.host.h_addrtype, 
					&task->dns_caches[i]->record.host.hostinfos[0]._h_addr, hbuf, sizeof(hbuf));
			}
		}

		dbgprintf(DBG_ERROR, "idx:%d, status: %d, expire:%d, host:%s, ip0:%s:%d, err=%d\n",
			i,
			task->dns_caches[i]->status,
			task->dns_caches[i]->expire_time.tv_sec - now.tv_sec,
			task->dns_caches[i]->record.hostname,
			hbuf[0] ? hbuf : "(null)",
			task->dns_caches[i]->record.port,
			task->dns_caches[i]->_errno
			);
	}

	for (i=0; i<MAX_DNS_THREAD; i++)
		dns_thread_dump(&task->threads[i]);
}

void dns_thread_dump(TDNS_Thread *thread)
{
	int in, out;

	dbgprintf(DBG_ERROR, "==== thread%d fifo ====\n", thread->index);

	pthread_mutex_lock(&thread->m_mutex);
	in = thread->dns_query_in; 
	out = thread->dns_query_out;
	while (in != out)
	{
		dbgprintf(DBG_ERROR, "idx: %d, status: %d, host:%s:%d, err:%d\n", 
			out,
			thread->dns_query_fifo[out].status,
			thread->dns_query_fifo[out].record.hostname,
			thread->dns_query_fifo[out].record.port,
			thread->dns_query_fifo[out]._errno
		);
		out = (out + 1) % ARRAY_LEN(thread->dns_query_fifo);
	}
	pthread_mutex_unlock(&thread->m_mutex);
}

int dns_task_init(TDNS_Task *task)
{
	int i, res;
	key_t key_server, key_client;

	dbgprintf(DBG_INFO, "dns task starting\n");
	memset(task, 0, sizeof(*task));

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

	task->server_qid = msgget(key_server, 0666 | IPC_CREAT);
	if (task->server_qid == -1) 
	{
		dbgprintf(DBG_ERROR, "msgget server failed = %d\n", errno);
		return -1;
	}
	dbgprintf(DBG_TRACE, "server queue id is %d\n", task->server_qid);

	task->client_qid = msgget(key_client, 0666 | IPC_CREAT);
	if (task->client_qid == -1) 
	{
		dbgprintf(DBG_ERROR, "msgget client failed = %d\n", errno);
		return -1;
	}
	dbgprintf(DBG_TRACE, "client queue id is %d\n", task->client_qid);

	for (i=0; i<MAX_DNS_THREAD; i++)
	{
		res = dns_thread_init(task, i);
		if (res != 0) return -1;
	}

	return 0;
}

void dns_task_destroy(TDNS_Task *task)
{
	int i;

	dbgprintf(DBG_INFO, "dns task ending\n");
	for (i=0; i<MAX_DNS_THREAD; i++)
	{
		dns_thread_destroy(task, i);
	}

	if (task->client_qid != -1)
	{
		msgctl(task->client_qid, IPC_RMID, 0);
		task->client_qid = -1;
	}

	if (task->server_qid != -1)
	{
		msgctl(task->server_qid, IPC_RMID, 0);
		task->server_qid = -1;
	}

	for (i=0; i<ARRAY_LEN(task->dns_caches); i++)
		dns_caches_remove(task, i);
}

int dns_msg_recv(TDNS_Task *task, TDNS_Message *msg)
{
	dbgprintf(DBG_TRACE, "waiting message in queue %d\n", task->server_qid);
	if (msgrcv(task->server_qid, (void *) msg, MAX_DNS_MSGSIZE, 0, 0) == -1) 
	{
		if (errno == EINTR)
		{
			msg->status = DNS_QUERY_BUSY;
			return 0;
		}

		dbgprintf(DBG_ERROR, "msgrcv failed = %d (%s)\n", errno, strerror(errno));
		return -1;
	}

	dbgprintf(DBG_TRACE, "message is received in queue %d, key=%d, size=%d\n", task->server_qid,
		msg->msg_key, dns_msg_size(msg));

	return 0;
}

int dns_msg_send(TDNS_Task *task, const TDNS_Message *msg)
{
	int size;

	size = dns_msg_size(msg);
	dbgprintf(DBG_TRACE, "sending message to queue %d, key=%d, size=%d\n",
		task->client_qid, msg->msg_key, size);	

	if (msgsnd(task->client_qid, (void *) msg, size, 0) == -1) 
	{
		dbgprintf(DBG_ERROR, "msgsnd failed = %d\n", errno);
		return -1;
	}

	return 0;
}

static int dns_msg_send_myself(TDNS_Task *task, const TDNS_Message *msg)
{
	dbgprintf(DBG_TRACE, "sending message to queue %d, key=%d, size=%d\n",
		task->server_qid, msg->msg_key, dns_msg_size(msg));
	if (msgsnd(task->server_qid, (void *) msg, dns_msg_size(msg), 0) == -1) 
	{
		dbgprintf(DBG_ERROR, "msgsnd failed = %d\n", errno);
		return -1;
	}

	return 0;
}

#define FIFO_EMPTY(fifo, in, out) ( (in) == (out) )
#define FIFO_FULL(fifo, in, out)  ( ((in) + 1) % (sizeof(fifo)/sizeof(fifo[0])) == (out) )
#define FIFO_LENGTH(fifo, in, out) ( (in) >= (out) ? (in) - (out) : (in) + (sizeof(fifo)/sizeof(fifo[0])) - (out) )

int dns_thread_query(TDNS_Task *task, const TDNS_Message *msg, TDNS_Cache *cache)
{
	static int index = 0;
	TDNS_Thread *thread;

	index = index % MAX_DNS_THREAD;
	thread = &task->threads[index];
	pthread_mutex_lock(&thread->m_mutex);

	if (FIFO_FULL(thread->dns_query_fifo, thread->dns_query_in, thread->dns_query_out))
	{
		dbgprintf(DBG_ERROR, "all fifo is full...(min:%d, in:%d, out:%d)\n", 
			index, thread->dns_query_in, thread->dns_query_out);
		pthread_mutex_unlock(&thread->m_mutex);
//		dns_thread_dump(thread);
		return -1;
	}
	dbgprintf(DBG_INFO, "use thread%d to query %s:%d, key=%d\n", 
		index, msg->record.hostname, msg->record.port, msg->msg_key);
	memcpy(&thread->dns_query_fifo[thread->dns_query_in], msg, sizeof(*msg));
	thread->dns_query_in = (thread->dns_query_in + 1) % ARRAY_LEN(thread->dns_query_fifo);
	// if no cache, init dns cache 
	if (cache == NULL) dns_caches_add(task, &msg->record, DNS_FINDING, 0);
	pthread_cond_signal(&thread->m_cond); // signal thread to start query
	dbgprintf(DBG_TRACE, "signal thread%d to wake up\n", index);

	pthread_mutex_unlock(&thread->m_mutex);
	index++;
	return 0;
}

struct hostent* _gethostbyname(const char *host)
{
	struct hostent *hostbuf, *hp;
	size_t hstbuflen;
	char *tmphstbuf;
	int res;
	int herr;

	hstbuflen = 1024;
	/* Allocate buffer, remember to free it to avoid memory leakage.  */
	hostbuf = (struct hostent *) malloc(sizeof(*hostbuf));
	tmphstbuf = malloc(hstbuflen);
	while ((res = gethostbyname_r (host, hostbuf, tmphstbuf, hstbuflen,
			&hp, &herr)) == ERANGE)
	{
		/* Enlarge the buffer.  */
		hstbuflen *= 2;
		tmphstbuf = realloc (tmphstbuf, hstbuflen);
	}

	/*  Check for errors.  */
	if (res || hp == NULL)
		return NULL;

	return hp;
}

static void *dns_thread_func(void *arg)
{
	int i, err;
	TDNS_Thread *thread = (TDNS_Thread *) arg;
	TDNS_Task *task = thread->parent;
	TDNS_Message *request, *response;
	TDNS_Message active_querys[MAX_DNS_QUERY];
	int active_query_count;
	// service info
	struct addrinfo hints;
	struct addrinfo *aitop;
	struct addrinfo *ai;
	int ai_count;
	TDNS_AddrInfo *addrinfo;
	// host info
	struct hostent *h_top;
	char **_h_addr;
	int h_count;
	TDNS_HostInfo *hostinfo;

	while (1)
	{
		pthread_mutex_lock(&thread->m_mutex);

		dbgprintf(DBG_TRACE, "thread%d waiting message\n", thread->index);

		while (FIFO_EMPTY(thread->dns_query_fifo, thread->dns_query_in, thread->dns_query_out))
			pthread_cond_wait(&thread->m_cond, &thread->m_mutex);

		active_query_count = 0;
		while (!FIFO_EMPTY(thread->dns_query_fifo, thread->dns_query_in, thread->dns_query_out))
		{
			memcpy(&active_querys[active_query_count], &thread->dns_query_fifo[thread->dns_query_out],
				sizeof(TDNS_Message));
			thread->dns_query_out = (thread->dns_query_out + 1) % ARRAY_LEN(thread->dns_query_fifo);
			active_query_count++;
		}

		pthread_mutex_unlock(&thread->m_mutex);

		// do dns query
		dbgprintf(DBG_INFO, "thread%d process fifo, len=%d\n", thread->index, active_query_count);
		for (i=0; i<active_query_count; i++)
		{
			char sz_port[10];

			request = &active_querys[i];
			dbgprintf(DBG_TRACE, "thread%d resolving %s\n", thread->index, request->record.hostname);
			aitop = NULL;
			h_top = NULL;
			if (request->record.port)
			{
				memset(&hints, 0, sizeof(hints));
				hints.ai_family = PF_UNSPEC;
				hints.ai_socktype = SOCK_DGRAM;
				hints.ai_protocol = IPPROTO_UDP;
				sprintf(sz_port, "%d", request->record.port);
				err = getaddrinfo(request->record.hostname, sz_port, &hints, &aitop);
				if (err)
				{
					request->record.service.ai_count = 0;
					request->_errno = errno;
					dbgprintf(DBG_INFO, "getaddrinfo failed: errno = %d\n", errno);
				}
			}
			else
			{
#if 1
				h_top = gethostbyname(request->record.hostname);
#else
				h_top = _gethostbyname(request->record.hostname);
#endif
				err = (h_top == NULL);
				if (err)
				{
					request->record.host.h_count = 0;
					request->_errno = h_errno;
					dbgprintf(DBG_INFO, "gethostbyname failed: errno = %d\n", h_errno);
				}
			}
			dbgprintf(DBG_TRACE, "thread%d resolving %s done.\n", thread->index, request->record.hostname);

			if (err != 0)
			{
				dbgprintf(DBG_ERROR, "DNS resolve %s failed\n", request->record.hostname);
				switch (request->status)
				{
				case DNS_QUERY_REQUEST:
				case DNS_QUERY_REQUEST2:
					request->status = DNS_QUERY_RESPONSE;
					dns_msg_send_myself(task, request);
					break;
				case DNS_QUERY_UPDATE:
					dns_msg_send_myself(task, request);
					break;
				}

				continue;
			}
		
			if (request->record.port)
			{
				for (ai_count=0, ai=aitop; ai; ai=ai->ai_next)
				{
					if (ai->ai_addr == NULL || ai->ai_addrlen == 0)
					{
						dbgprintf(DBG_ERROR, "ai_addr is null!?\n");
						continue;
					}
					if (ai->ai_addrlen > sizeof(request->record.service.addrinfos[0].ai_addr))
					{
						dbgprintf(DBG_ERROR, "ai_addr(%d) is too big\n", ai->ai_addrlen);
						continue;
					}
					ai_count++;
				}

				request->record.service.ai_count = ai_count;
			}
			else
			{
				for(h_count=0, _h_addr=h_top->h_addr_list; *_h_addr; _h_addr++)
				{
					h_count++;
				}

				request->record.host.h_count = h_count;
			}

			response = dns_msg_new(&request->record);
			if (response == NULL)
			{
				dbgprintf(DBG_ERROR, "dns_msg_new failed!!\n");
				if (request->record.port)
					freeaddrinfo(aitop);
				else
					free(h_top);
				continue;
			}

			memcpy(response, request, sizeof(*request));

			if (request->record.port)
			{
				for (ai_count=0, ai=aitop; ai; ai=ai->ai_next)
				{
					if (ai->ai_addr == NULL || ai->ai_addrlen == 0 ||
						ai->ai_addrlen > sizeof(request->record.service.addrinfos[0].ai_addr))
						continue;
				
					addrinfo = &response->record.service.addrinfos[ai_count++];
					addrinfo->ai_flags = ai->ai_flags;
					addrinfo->ai_family = ai->ai_family;
					addrinfo->ai_socktype = ai->ai_socktype;
					addrinfo->ai_protocol = ai->ai_protocol;
					addrinfo->ai_addrlen = ai->ai_addrlen;
					memcpy(&addrinfo->ai_addr, ai->ai_addr, ai->ai_addrlen);
				}
				freeaddrinfo(aitop);
			}
			else
			{
				response->record.host.h_addrtype = h_top->h_addrtype;
				response->record.host.h_length = h_top->h_length;
				for(h_count=0, _h_addr=h_top->h_addr_list; *_h_addr; _h_addr++)
				{
					hostinfo = &response->record.host.hostinfos[h_count++];
					memcpy(&hostinfo->_h_addr, *_h_addr, h_top->h_length);
				}
#if 0
				freehostent(h_top);
#endif
			}

			switch (response->status)
			{
			case DNS_QUERY_REQUEST:
			case DNS_QUERY_REQUEST2:
				response->status = DNS_QUERY_RESPONSE;
				dns_msg_send_myself(task, response);
				break;
			case DNS_QUERY_UPDATE:
				dns_msg_send_myself(task, response);
				break;
			}

			free(response);
		}
	}

	dbgprintf(DBG_INFO, "thread%d out\n", thread->index);
	pthread_exit(NULL);
}

int dns_thread_init(TDNS_Task *task, const int index)
{
	int i;
	TDNS_Thread *thread;

	thread = &task->threads[index];
	memset(thread, 0, sizeof(*thread));
	i = pthread_mutex_init(&thread->m_mutex, NULL);
	if (i != 0)
	{
		dbgprintf(DBG_ERROR, "pthread_mutex_init failed\n");
		return -1;
	}

	i = pthread_cond_init(&thread->m_cond, NULL);
	if (i != 0)
	{
		dbgprintf(DBG_ERROR, "pthread_cond_init failed\n");
		pthread_mutex_destroy(&thread->m_mutex);
		return -1;
	}

	thread->index = index;
	thread->parent = task;
	i = pthread_create(&thread->m_thread, NULL, dns_thread_func, (void *) thread);
	if (i != 0)
	{
		dbgprintf(DBG_ERROR, "pthread_create failed\n");
		thread->parent = NULL;
		thread->index = 0;
		pthread_cond_destroy(&thread->m_cond);
		pthread_mutex_destroy(&thread->m_mutex);
		return -1;
	}

	return 0;
}

void dns_thread_destroy(TDNS_Task *task, const int index)
{
	TDNS_Thread *thread;

	thread = &task->threads[index];
	if (thread->parent == NULL)
		return;

	thread->parent = NULL;
	thread->index = 0;
	pthread_cancel(thread->m_thread);
	pthread_join(thread->m_thread, NULL);
	pthread_cond_destroy(&thread->m_cond);
	pthread_mutex_destroy(&thread->m_mutex);
}

