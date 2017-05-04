#ifndef _PMLIST_H_
	#define _PMLIST_H_
#define CDROUTER_PATCH
extern char extIpAddress[16];     // Server internal ip address
struct portMap
{
   int m_PortMappingEnabled;
#ifdef CDROUTER_PATCH
   char m_PortMappingLeaseDuration[10];
#else
   long int m_PortMappingLeaseDuration;
#endif
   char m_RemoteHost[16];
   char m_ExternalPort[6];
   char m_InternalPort[6];
   char m_PortMappingProtocol[4];
   char m_InternalClient[16];
   char m_PortMappingDescription[50];

	struct portMap* next;
	struct portMap* prev;
} *pmlist_Head, *pmlist_Tail, *pmlist_Current;

//struct portMap* pmlist_NewNode(void);
#ifdef CDROUTER_PATCH
struct portMap* pmlist_NewNode(int enabled, char *remoteHost,
			char *externalPort, char *internalPort, 
			char *protocol, char *internalClient, char *desc,char *leaseTime);
#else
struct portMap* pmlist_NewNode(int enabled, int duration, char *remoteHost,
			char *externalPort, char *internalPort, 
			char *protocol, char *internalClient, char *desc);
#endif

struct portMap* pmlist_Find(char *externalPort, char *proto, char *internalClient);
struct portMap* pmlist_FindByIndex(int index);
struct portMap* pmlist_FindSpecific(char *externalPort, char *protocol);
int pmlist_IsEmtpy(void);
int pmlist_Size(void);
int pmlist_FreeList(void);
int pmlist_PushBack(struct portMap* item);
int pmlist_Delete(struct portMap* item);
#ifdef CDROUTER_PATCH
int pmlist_AddPortMapping (char *protocol,
		char *externalPort, char *internalClient, char *internalPort, char *remoteIp, char *leaseTime);
#else
int pmlist_AddPortMapping (char *protocol,
		char *externalPort, char *internalClient, char *internalPort);
#endif
int pmlist_DeletePortMapping(char *protocol, 
		char *externalPort, char *internalClient, char *internalPort);

#endif // _PMLIST_H_
