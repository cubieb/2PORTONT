#ifndef __DNS_THREAD_H
#define __DNS_THREAD_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "rcm_sim_defs.h" //SD6, darren_shiue, rcm simulation

#define DISABLE_SRV

#define DNS_ERROR		0
#define DNS_INFO		1
#define DNS_TRACE		2
#define DNS_CURRENT		DNS_ERROR

#ifdef __mips__
#define DNS_SERVER_NAME "/bin/dns_task"
#define DNS_CLIENT_NAME "/bin/solar"
#elif defined(RCM_SIMULATION)	//SD6, darren_shiue, rcm simulation
#define DNS_SERVER_NAME "/bin/dns_task"
#define DNS_CLIENT_NAME "/bin/solar"
#else
#define DNS_SERVER_NAME "./dns_task"
#define DNS_CLIENT_NAME "./dns_test"
#endif

#define DNS_TASK_VER	0x01

#define MAX_DNS_THREAD	1
#define MAX_DNS_QUERY	5	// num of query in one thread
#define MAX_DNS_RECORD	10	// num of record in one query
#define MAX_DNS_CACHE	10	// max dns cache
#define MAX_DNS_TIME	120	// max dns cache time
#define MIN_DNS_TIME	30	// min dns cache time: if request time smaller than it, don't dns lookup really

#define DNS_MAX_HOST	40

#define ARRAY_LEN(x)	(sizeof(x)/sizeof(x[0]))

#define MAX_DNS_MSGSIZE (sizeof(TDNS_Message) + sizeof(TDNS_AddrInfo) * MAX_DNS_RECORD)

enum {
	// client
	DNS_QUERY_REQUEST = 0,
	DNS_QUERY_REQUEST2,
	DNS_QUERY_REQUEST3,
	DNS_QUERY_UPDATE,
	// server
	DNS_QUERY_RESPONSE,
	DNS_QUERY_BUSY,
	DNS_QUERY_TRYING,
};

enum {
	DNS_FINDING = 0,
	DNS_FOUND,
	DNS_UPDATING,
};

typedef struct {
	int ai_flags;
	int ai_family;
	int ai_socktype;
	int ai_protocol;
	size_t ai_addrlen;
	union {
		struct sockaddr_in ai_addr_in;
		struct sockaddr_in6 ai_addr_in6;
	} ai_addr;
} TDNS_AddrInfo;

typedef struct {
	union {
		struct in_addr h_addr_in;
		struct in6_addr h_addr_in6;
	} _h_addr; // WARNING: don't use h_addr, it has defined in netdb.h
} TDNS_HostInfo;

typedef struct {
	int ai_count;
	TDNS_AddrInfo addrinfos[0];
} TDNS_ServiceRecord;

typedef struct {
	int h_addrtype;
	int h_length;
	int h_count;
	TDNS_HostInfo hostinfos[0];
} TDNS_HostRecord;

typedef struct {
	// query
	char hostname[DNS_MAX_HOST];
	int port; // query host record if port = 0
	// response
	union {
		TDNS_ServiceRecord service;
		TDNS_HostRecord host;
	};
} TDNS_Record;

typedef struct {
	long int msg_key;
	int status;
	int _errno;
	TDNS_Record record;
} TDNS_Message;

typedef struct {
	int status;
	int _errno;
	struct timeval expire_time;
	TDNS_Record record;
} TDNS_Cache;

typedef struct SDNS_Task TDNS_Task;

typedef struct {
	TDNS_Task *parent;
	int index;
	pthread_t m_thread;
	pthread_cond_t m_cond;
	pthread_mutex_t m_mutex;
	TDNS_Message dns_query_fifo[MAX_DNS_QUERY];
	int dns_query_in, dns_query_out;
} TDNS_Thread;

struct SDNS_Task {
	int server_qid; // server queue id
	int client_qid; // client queue id
	TDNS_Cache *dns_caches[MAX_DNS_CACHE];
	TDNS_Thread threads[MAX_DNS_THREAD];
};


int dns_extra_record_size(TDNS_Record const *record);
#define dns_record_size(r) (sizeof(TDNS_Record) + dns_extra_record_size(r))
#define dns_msg_size(m) (sizeof(TDNS_Message) + dns_extra_record_size(&(m)->record))
#define dns_msg_new(r) ((TDNS_Message *) malloc(sizeof(TDNS_Message) + dns_extra_record_size(r)))
#define dns_cache_size(c) (sizeof(TDNS_Cache) + dns_extra_record_size(&(c)->record))
#define dns_cache_new(r) ((TDNS_Cache *) malloc(sizeof(TDNS_Cache) + dns_extra_record_size(r)))

int dns_task_init(TDNS_Task *task);
void dns_task_destroy(TDNS_Task *task);

int dns_thread_init(TDNS_Task *task, const int index);
int dns_thread_query(TDNS_Task *task, const TDNS_Message *msg, TDNS_Cache *cache);
void dns_thread_destroy(TDNS_Task *task, const int index);
void dns_thread_dump(TDNS_Thread *thread);

int dns_msg_recv(TDNS_Task *task, TDNS_Message *msg);
int dns_msg_send(TDNS_Task *task, const TDNS_Message *msg);

TDNS_Cache *dns_caches_find(TDNS_Task *task, const TDNS_Record *record);
int dns_caches_add(TDNS_Task *task, const TDNS_Record *record, int status, int _errno);
void dns_caches_remove(TDNS_Task *task, int index);
void dns_caches_update(TDNS_Task *task);
void dns_cache_dump(TDNS_Task *task);

#endif
