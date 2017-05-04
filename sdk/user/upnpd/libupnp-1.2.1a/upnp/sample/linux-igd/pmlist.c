#include <stdlib.h>
//#include <syslog.h>
#include "globals.h"
#include "config.h"
#include "pmlist.h"
#include "gatedevice.h"
//#define RTL865X_ONLY 1

#ifdef RTL865X_ONLY
#include <re865x.h>
#include <linux/sockios.h>
#include <net/if.h>

#endif /* RTL865X_ONLY */
#define CDROUTER_PATCH

#ifdef RTL865X_ONLY
/*******************************/
/* RTL8651 specific function   */
static int device_ioctl(char *name,unsigned long arg0, unsigned long arg1, unsigned long arg2, unsigned long arg3)
{
	unsigned long args[4];
	struct ifreq ifr;
	int sockfd;

	args[0] = arg0;
	args[1] = arg1;
	args[2] = arg2;
	args[3] = arg3;

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		return -3;
	}
	//memset((char*)&ifr,0,sizeof(ifr));
	strcpy((char*)&ifr.ifr_name, name);
	((unsigned long *)(&ifr.ifr_data))[0] = (unsigned long)args;
	if (ioctl(sockfd, SIOCDEVPRIVATE, &ifr)<0)
	{
		perror("device ioctl:");
		close(sockfd);
		return -1;
	}
	close(sockfd);
	return 0;
}
#ifdef CDROUTER_PATCH
 int rtl8651_addNaptUpnpPortMapping(unsigned char isTcp, unsigned int extIpAddr, unsigned short extPort, unsigned int intIpAddr, unsigned short intPort,unsigned int remoteIpAddr,unsigned int leaseTime)
#else
static int rtl8651_addNaptUpnpPortMapping(unsigned char isTcp, unsigned int extIpAddr, unsigned short extPort, unsigned int intIpAddr, unsigned short intPort)
#endif
{
	unsigned long ret;
#ifdef CDROUTER_PATCH
	unsigned int ptr[7];
#else
	unsigned int ptr[5];
#endif
	ptr[0]=&isTcp;
	ptr[1]=&extIpAddr;
	ptr[2]=&extPort;
	ptr[3]=&intIpAddr;
	ptr[4]=&intPort;
#ifdef CDROUTER_PATCH
	ptr[5]=&remoteIpAddr;	
	ptr[6]=&leaseTime;
#endif
	device_ioctl("eth0",RTL8651_ADDNAPTUPNPPORTMAPPING,(unsigned int)(ptr),0,&ret);
	return ret;	
}

int rtl8651_queryUpnpMapTimeAge(unsigned int isTcp,unsigned int extIpAddr,unsigned short extPort, unsigned int intIpAddr,unsigned short intPort)
{
	unsigned int ret;
	unsigned int ptr[5];
	ptr[0]=&isTcp;
	ptr[1]=&extIpAddr;
	ptr[2]=&extPort;
	ptr[3]=&intIpAddr;
	ptr[4]=&intPort;
	device_ioctl("eth0",RTL8651_IOCTL_QUERYUPNPMAPTIMEAGE ,(unsigned int)(ptr),0,&ret);
	return ret;
}

static int rtl8651_delNaptUpnpPortMapping(unsigned char isTcp, unsigned int extIpAddr, unsigned short extPort, unsigned int intIpAddr, unsigned short intPort)
{
	unsigned long ret;
	unsigned int ptr[5];	
	ptr[0]=(unsigned int)&isTcp;
	ptr[1]=(unsigned int)&extIpAddr;
	ptr[2]=(unsigned int)&extPort;
	ptr[3]=(unsigned int)&intIpAddr;
	ptr[4]=(unsigned int)&intPort;
	device_ioctl("eth0",RTL8651_DELNAPTUPNPPORTMAPPING,(unsigned int)(ptr),0,&ret);
	return ret;		
}

#endif /* RTL865X_ONLY */ 
#ifdef CDROUTER_PATCH
struct portMap* pmlist_NewNode(int enabled, char *remoteHost,
         char *externalPort, char *internalPort,
         char *protocol, char *internalClient, char *desc, char *leaseTime)
#else
struct portMap* pmlist_NewNode(int enabled, int duration, char *remoteHost,
         char *externalPort, char *internalPort,
         char *protocol, char *internalClient, char *desc)
#endif
{
	struct portMap* temp;
	temp = (struct portMap*) malloc(sizeof(struct portMap));
	temp->m_PortMappingEnabled = enabled;
#ifdef CDROUTER_PATCH
	temp->m_PortMappingLeaseDuration[0] = '0';
	temp->m_PortMappingLeaseDuration[1] = 0;
#else
	temp->m_PortMappingLeaseDuration = duration;
#endif
	
	if (strlen(remoteHost) < sizeof(temp->m_RemoteHost)) strcpy(temp->m_RemoteHost, remoteHost);
		else strcpy(temp->m_RemoteHost, "");
	if (strlen(externalPort) < sizeof(temp->m_ExternalPort)) strcpy(temp->m_ExternalPort, externalPort);
		else strcpy(temp->m_ExternalPort, "");
	if (strlen(internalPort) < sizeof(temp->m_InternalPort)) strcpy(temp->m_InternalPort, internalPort);
		else strcpy(temp->m_InternalPort, "");
	if (strlen(protocol) < sizeof(temp->m_PortMappingProtocol)) strcpy(temp->m_PortMappingProtocol, protocol);
		else strcpy(temp->m_PortMappingProtocol, "");
	if (strlen(internalClient) < sizeof(temp->m_InternalClient)) strcpy(temp->m_InternalClient, internalClient);
		else strcpy(temp->m_InternalClient, "");
	if (strlen(desc) < sizeof(temp->m_PortMappingDescription)) strcpy(temp->m_PortMappingDescription, desc);
		else strcpy(temp->m_PortMappingDescription, "");
#ifdef CDROUTER_PATCH
	if (strlen(leaseTime) < sizeof(temp->m_PortMappingLeaseDuration)) strcpy(temp->m_PortMappingLeaseDuration, leaseTime);
		else strcpy(temp->m_PortMappingLeaseDuration, "0");
#endif

	temp->next = NULL;
	temp->prev = NULL;
	
	return temp;
}
	
struct portMap* pmlist_Find(char *externalPort, char *proto, char *internalClient)
{
	struct portMap* temp;
	
	temp = pmlist_Head;
	if (temp == NULL)
		return NULL;
	
	do 
	{
		if ( (strcmp(temp->m_ExternalPort, externalPort) == 0) &&
				(strcmp(temp->m_PortMappingProtocol, proto) == 0) &&
				(strcmp(temp->m_InternalClient, internalClient) == 0) )
			return temp; // We found a match, return pointer to it
		else
			temp = temp->next;
	} while (temp != NULL);
	
	// If we made it here, we didn't find it, so return NULL
	return NULL;
}

//#ifdef CDROUTER_PATCH
#if 1
struct portMap* pmlist_Find2(char *externalPort, char *proto)
{
	struct portMap* temp;
	
	temp = pmlist_Head;
	if (temp == NULL)
		return NULL;
	
	do 
	{
		if ( (strcmp(temp->m_ExternalPort, externalPort) == 0) &&
				(strcmp(temp->m_PortMappingProtocol, proto) == 0)  )
			return temp; // We found a match, return pointer to it
		else
			temp = temp->next;
	} while (temp != NULL);
	
	// If we made it here, we didn't find it, so return NULL
	return NULL;
}
#endif

struct portMap* pmlist_FindByIndex(int index)
{
	int i=0;
	struct portMap* temp;

	temp = pmlist_Head;
	if (temp == NULL)
		return NULL;
	do
	{
		if (i == index)
			return temp;
		else
		{
			temp = temp->next;	
			i++;
		}
	} while (temp != NULL);

	return NULL;
}	

struct portMap* pmlist_FindSpecific(char *externalPort, char *protocol)
{
	struct portMap* temp;
	
	temp = pmlist_Head;
	if (temp == NULL)
		return NULL;
	
	do
	{
		if ( (strcmp(temp->m_ExternalPort, externalPort) == 0) &&
				(strcmp(temp->m_PortMappingProtocol, protocol) == 0))
			return temp;
		else
			temp = temp->next;
	} while (temp != NULL);

	return NULL;
}

int pmlist_IsEmtpy(void)
{
	if (pmlist_Head)
		return 0;
	else
		return 1;
}

int pmlist_Size(void)
{
	struct portMap* temp;
	int size = 0;
	
	temp = pmlist_Head;
	if (temp == NULL)
		return 0;
	
	while (temp->next)
	{
		size++;
		temp = temp->next;
	}
	size++;
	return size;
}	

int pmlist_FreeList(void)
{
	struct portMap* temp;
	
	temp = pmlist_Head;
	if (temp) // We have a list
	{
		while(temp->next) // While there's another node left in the list
		{
	      pmlist_DeletePortMapping(temp->m_PortMappingProtocol, temp->m_ExternalPort,
            temp->m_InternalClient, temp->m_InternalPort);
			temp = temp->next; // Move to the next element.
 			free(temp->prev);
			temp->prev = NULL; // Probably unecessary, but i do it anyway. May remove.
		}
      pmlist_DeletePortMapping(temp->m_PortMappingProtocol, temp->m_ExternalPort,
            temp->m_InternalClient, temp->m_InternalPort);
		free(temp); // We're at the last element now, so delete ourselves.
		pmlist_Head = pmlist_Tail = NULL;
	}
	return 1;
}
		
int pmlist_PushBack(struct portMap* item)
{
	int action_succeeded = 0;

	if (pmlist_Tail) // We have a list, place on the end
	{
		pmlist_Tail->next = item;
		item->prev = pmlist_Tail;
		item->next = NULL;
		pmlist_Tail = item;
		action_succeeded = 1;
	}
	else // We obviously have no list, because we have no tail :D
	{
		pmlist_Head = pmlist_Tail = pmlist_Current = item;
		item->prev = NULL;
		item->next = NULL;
 		action_succeeded = 1;
	}
	if (action_succeeded == 1)
	{
#ifdef CDROUTER_PATCH	
		 pmlist_AddPortMapping(item->m_PortMappingProtocol,
         item->m_ExternalPort, item->m_InternalClient, item->m_InternalPort, item->m_RemoteHost,item->m_PortMappingLeaseDuration);	
#else
		 pmlist_AddPortMapping(item->m_PortMappingProtocol,
         item->m_ExternalPort, item->m_InternalClient, item->m_InternalPort);	
#endif
		return 1;
	}
	else
		return 0;
}

		
int pmlist_Delete(struct portMap* item)
{
	struct portMap *temp;
	int action_succeeded = 0;

	temp = pmlist_Find(item->m_ExternalPort, item->m_PortMappingProtocol, item->m_InternalClient);
	if (temp) // We found the item to delete
	{
		pmlist_DeletePortMapping(item->m_PortMappingProtocol, item->m_ExternalPort, 
				item->m_InternalClient, item->m_InternalPort);
		if (temp == pmlist_Head) // We are the head of the list
		{
			if (temp->next == NULL) // We're the only node in the list
			{
				pmlist_Head = pmlist_Tail = pmlist_Current = NULL;
				free (temp);
				action_succeeded = 1;
			}
			else // we have a next, so change head to point to it
			{
				pmlist_Head = temp->next;
				pmlist_Head->prev = NULL;
				free (temp);
				action_succeeded = 1;	
			}
		}
		else if (temp == pmlist_Tail) // We are the Tail, but not the Head so we have prev
		{
			pmlist_Tail = pmlist_Tail->prev;
			free (pmlist_Tail->next);
			pmlist_Tail->next = NULL;
			action_succeeded = 1;
		}
		else // We exist and we are between two nodes
		{
			temp->prev->next = temp->next;
			temp->next->prev = temp->prev;
			pmlist_Current = temp->next; // We put current to the right after a extraction
			free (temp);	
			action_succeeded = 1;
		}
	}
	else  // We're deleting something that's not there, so return 0
		action_succeeded = 0;

	if (action_succeeded == 1)
	{
		return 1;
	}
	else 
		return 0;
}

#ifdef CDROUTER_PATCH
int pmlist_AddPortMapping (char *protocol, char *externalPort, char *internalClient, char *internalPort,char *remoteIp,char *leaseTime)
#else
int pmlist_AddPortMapping (char *protocol, char *externalPort, char *internalClient, char *internalPort)
#endif
{

	char command[500];	
	
#ifndef RTL865X_ONLY	
	
	// Added by Mason Yu
	sprintf(command,"%s -I INPUT 1 -p %s --dport %s -j ACCEPT ", g_iptables, protocol,  externalPort);	
	system (command);
	
	sprintf(command, "%s -t nat -A %s -i %s -p %s --dport %s -j DNAT --to %s:%s", g_iptables, g_preroutingChainName, g_extInterfaceName, protocol, externalPort, internalClient, internalPort);
	if (g_debug) printf( command);
	system (command);
	if (g_forwardRules)
	{
	    sprintf(command,"%s -I %s -p %s -d %s --dport %s -j ACCEPT", g_iptables,g_forwardChainName, protocol, internalClient, internalPort);
	    if (g_debug) printf( command);
	    system(command);
	}
#else
	int isTCP;	
	unsigned short ExtPort,IntPort;
#ifdef CDROUTER_PATCH
	unsigned int lTime;
        struct in_addr rmIp;
#endif
	struct in_addr intIp,extIp;
	if(strcmp("TCP",protocol)==0) isTCP=1;
	else isTCP=0;	
	inet_aton(internalClient,&intIp);
	inet_aton(extIpAddress,&extIp);
#ifdef CDROUTER_PATCH
	if(remoteIp[0]==0) 
		rmIp.s_addr=0;
	else
		inet_aton(remoteIp,&rmIp);
	if(leaseTime[0]==0)
		lTime=0;
	else
		lTime=atoi(leaseTime);
#endif
	ExtPort=atoi(externalPort);
	IntPort=atoi(internalPort);
#ifdef CDROUTER_PATCH 
	rtl8651_addNaptUpnpPortMapping( isTCP, extIp.s_addr, ExtPort, intIp.s_addr, IntPort,rmIp.s_addr,lTime);
#else
	rtl8651_addNaptUpnpPortMapping( isTCP, extIp.s_addr, ExtPort, intIp.s_addr, IntPort);
#endif

#endif	

	return 1;
}

int pmlist_DeletePortMapping(char *protocol, char *externalPort, char *internalClient, char *internalPort)
{
#ifndef RTL865X_ONLY
	char command[500];
	
	sprintf(command, "%s -t nat -D %s -i %s -p %s --dport %s -j DNAT --to %s:%s",
			g_iptables, g_preroutingChainName, g_extInterfaceName, protocol, externalPort, internalClient, internalPort);
	if (g_debug) printf( command);
	system(command);
	if (g_forwardRules)
	{
	    sprintf(command,"%s -D %s -p %s -d %s --dport %s -j ACCEPT", g_iptables, g_forwardChainName, protocol, internalClient, internalPort);
	    if (g_debug) printf( command);
	    system(command);
	}
#else
	int isTCP;
	unsigned short ExtPort,IntPort;
	struct in_addr  intIp,extIp;	
	if(strcmp("TCP",protocol)==0) isTCP=1;
	else isTCP=0;	
	inet_aton(internalClient,&intIp);
	inet_aton(extIpAddress,&extIp);	
	ExtPort=atoi(externalPort);
	IntPort=atoi(internalPort);
	rtl8651_delNaptUpnpPortMapping( isTCP, extIp.s_addr, ExtPort, intIp.s_addr, IntPort);	
#endif	
	return 1;
}

