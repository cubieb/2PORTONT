/***********************************************************************
 * Martin ZHU:
 * 	get http User-Agent filed and parser it to get lan host information,
 *	including: lan host OS, device type, brand and so on
 * 
 * 2016-01-07
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h> 

#include <signal.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h> 

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <sys/socket.h> 
#include <sys/param.h>

#include <linux/netlink.h> 
#include <linux/if_arp.h> 
#include <linux/if_ether.h>

#include <arpa/inet.h>

#include <rtk/mib.h>
#include <rtk/rtusr_rg_api.h>
#include <rtk_rg_struct.h>
#include <rtk_rg_define.h>

#include "lanNetInfo.h"

#if 0
#define DEBUG_TRACE(x, y) printf y
#else
#define DEBUG_TRACE(x, y) 
#endif

uint8 isZeroMac(uint8 *pMac) 
{
	return !(pMac[0]|pMac[1]|pMac[2]|pMac[3]|pMac[4]|pMac[5]);
}

/****************************************************************************
 *
 * mac match > first empty > first invalid
 *
 ****************************************************************************/
lanNetInfo_t *get_LanHost(uint8 *pMac)
{
	int first_invalid = -1, first_empty = -1, i;

	if( (!pMac) || isZeroMac(pMac) )
		return NULL;
	
	for(i=0; i < MAX_LAN_HOST_INFO_NUM; i++)
	{
		if( !lanNetInfoData[i].valid )
		{
			/* first invalid idx */
			if(first_invalid == -1)
				first_invalid = i;
			
			/* first invalid and empty idx  */
			if( (isZeroMac(lanNetInfoData[i].lanHost.mac.octet))&&(first_empty==-1) )
				first_empty = i;	
		}

		/* MAC match means not connect in fisrt time */
		if( !macAddrCmp(lanNetInfoData[i].lanHost.mac.octet, pMac) )
		{
			return &lanNetInfoData[i];
		}
	}

	/* table is full, all is valid */
	if(first_invalid == -1)
		return NULL;

	/* not find empty, so return first invalid */
	if(first_empty == -1)
		first_empty = first_invalid;

	/* recod new lan host idx in lanNetInfoData */
	newLanHostIdx[first_empty] = 1;
	return &lanNetInfoData[first_empty];
}

#define ARP_PKT_LEN		64
#define ARP_PKT_AGLIN	4

uint32 arpRequestPacket[ARP_PKT_LEN/ARP_PKT_AGLIN];
int arp_socket = -1;

int ARPRequestCreate(uint32 dstIp)
{
	uint32 i, lanIP, off;
	uint8 *pARPData = (uint8 *)arpRequestPacket;
	uint8 arp_hdr_len = sizeof(struct arphdr);
	struct ethhdr *pL2Hdr = (struct ethhdr *)pARPData;
	struct arphdr *arpHdr = (struct arphdr *)&pARPData[ETH_HLEN];
	uint8 sMac[ETH_ALEN];

	memset(pARPData, 0, ARP_PKT_LEN);

	if(!mib_get(MIB_ELAN_MAC_ADDR, (void *)sMac))
		return 1;
	
	/* set Ethernet Header  */
	memset(pL2Hdr->h_dest, 0xff, ETH_ALEN);
	memcpy(pL2Hdr->h_source, sMac, ETH_ALEN);
	pL2Hdr->h_proto = htons(ETH_P_ARP);

	/* set ARP Header  */
	arpHdr->ar_hrd = htons(ARPHRD_ETHER);
	arpHdr->ar_pro = htons(ETH_P_IP);

	arpHdr->ar_hln = ETH_ALEN;
	arpHdr->ar_pln = 4;//IP length
	arpHdr->ar_op = htons(ARPOP_REQUEST);
	off = ETH_HLEN+arp_hdr_len;
	
	memcpy(&pARPData[off], sMac, ETH_ALEN);
	off += ETH_ALEN;
	
	if(!mib_get(MIB_ADSL_LAN_IP, (void *)&lanIP))
		return 1;
	lanIP = htonl(lanIP);
	memcpy(&pARPData[off], &lanIP, 4);
	off += 4;
	
	memset(&pARPData[off], 0, ETH_ALEN);
	off += ETH_ALEN;

	dstIp = htonl(dstIp);
	memcpy(&pARPData[off], &dstIp, 4);
	DEBUG_TRACE(0, ("%s %d \n", __func__, __LINE__) );
	return 0;
}

struct sockaddr_ll	my_etheraddr;
#define	BRIDGE_DEV	"br0"
int get_ifindex(char *dev)   
{   
    int s;     
    struct ifreq req;    
    int err;    
    strcpy(req.ifr_name, dev);    
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)   
    {   
        DEBUG_TRACE(0, ("socket()"));   
        exit(1);   
    }   
    err = ioctl(s, SIOCGIFINDEX, &req);    
    close(s);    
    if (err == -1)    
        return err;    
           
    return req.ifr_ifindex;   
} 


/* send arp request packet to lanNetInfoData[i] to test if this lan host online */
int sendARPRequest()
{
	int i, ret;
	if(arp_socket == -1)
	{
		/* Create sockaddr_ll( link layer ) struct */  
    	memset(&my_etheraddr, 0, sizeof(my_etheraddr));
    	my_etheraddr.sll_family = AF_PACKET;
    	my_etheraddr.sll_protocol = htons(ETH_P_ARP);
    	my_etheraddr.sll_ifindex = get_ifindex(BRIDGE_DEV);

		/* Create pf_packet socket  */
    	arp_socket = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
    	if (arp_socket < 0)
    	{
        	DEBUG_TRACE(0, ("open socket error\n"));
       	 	return -1;
    	}
		 /* bind to socket */    
    	if (bind(arp_socket, (struct sockaddr*)&my_etheraddr, sizeof(my_etheraddr)))   
    	{
        	DEBUG_TRACE(0, ("bind socket error\n"));
        	return -1;
    	}
	}
  
	for(i=0; i < MAX_LAN_HOST_INFO_NUM; i++)
	{
		if(lanNetInfoData[i].valid)
		{
			if( !ARPRequestCreate(lanNetInfoData[i].lanHost.ip) )
			{
				DEBUG_TRACE(0, ("send ARP Request to %x.\n", lanNetInfoData[i].lanHost.ip));
				ret = sendto(arp_socket, (void *)arpRequestPacket, ARP_PKT_LEN, 0, (struct sockaddr *)&my_etheraddr,sizeof(my_etheraddr));
				if(ret<=0)
				{
					DEBUG_TRACE(0, ("send ARP Request error.\n"));
					 return -1;
				}
			}
		}
	}
     return 0;
}

void updateOnlineTime()
{
	int i;

	for(i=0; i < MAX_LAN_HOST_INFO_NUM; i++)
	{
		if(lanNetInfoData[i].valid)
		{
			lanNetInfoData[i].lanHost.onLineTime++ ;
		}
	}
	
	return;
}

int32 isComeFromLan(rtk_rg_port_idx_t ingressPort)
{
	int idx, ret;
	rtk_rg_portmask_t port_mask;
	rtk_rg_intfInfo_t intf_info;
	
	if(ingressPort == RTK_RG_PORT_CPU)
		return 0;

	port_mask.portmask = 1<<ingressPort;
	for(idx=0; idx<MAX_NETIF_SW_TABLE_SIZE; idx++)
	{
		ret = rtk_rg_intfInfo_find(&intf_info, &idx);
		if( (ret == RT_ERR_RG_OK) && (0 == intf_info.is_wan) )
		{
			if( port_mask.portmask & intf_info.lan_intf.port_mask.portmask)
			{
				DEBUG_TRACE(0, ("The device is LAN device\n"));
				return 1;
			}
		}
	}
	return 0;
}


uint8 getlanNetInfoConPort(rtk_rg_port_idx_t port_idx)
{
	if(port_idx >= RTK_RG_EXT_PORT0)
		return Port_WIFI;
	else if (port_idx < RTK_RG_PORT_PON)
	{
#ifdef CONFIG_RTL9602C_SERIES
		return (port_idx+Port_LAN1);
#else
		//For YUEME(9607), port 2&3 exist only, corresponding to lan 1/2
		return (port_idx-Port_LAN1);
#endif
	}
	else
		return Port_LAN1;
}


void updateLanNetInfoFromRGARPTable()
{
	int ret;
	rtk_rg_arpInfo_t arpInfo;
	int arp_valid_idx=0;
	rtk_rg_macEntry_t macEntry;
	int mac_valid_idx;
	lanNetInfo_t *lanNetInfo=NULL;

	for( ; ; )
	{
		memset(&arpInfo, 0, sizeof(rtk_rg_arpInfo_t) );
    	ret = rtk_rg_arpEntry_find(&arpInfo, &arp_valid_idx);
		if(ret == RT_ERR_RG_OK)
    	{/* parse first */
			mac_valid_idx = arpInfo.arpEntry.macEntryIdx;
			memset(&macEntry, 0, sizeof(rtk_rg_macEntry_t));
			ret = rtk_rg_macEntry_find(&macEntry, &mac_valid_idx);
			if( (ret == RT_ERR_RG_OK) && (mac_valid_idx == arpInfo.arpEntry.macEntryIdx) )
			{
				if(!isComeFromLan(macEntry.port_idx))
				{
					arp_valid_idx++;
					continue;
				}
				
				lanNetInfo = get_LanHost(macEntry.mac.octet);
				if(lanNetInfo)
				{	
					if(arpInfo.idleSecs < NET_INFO_AGEOUT_TIME)
					{// update
						if(lanNetInfo->valid == 0)
						{
							gettimeofday(&interval,NULL);
							lanNetInfo->startOnLine = interval.tv_sec;
						}

						/*
						 Martin ZHU:
						 	patch for iphone only----Before get ip by DHCP, iphone send ARP Request with 192.168.1.12x as own ip,
						 	so after get IP by DHCP way, ARP table will have 2 entry point to 1 l2 entry(MAC is iphone MAC)
							2016-2-18
						*/
						if( (lanNetInfo->valid == 1)&&(arpInfo.arpEntry.ipv4Addr>=0xc0a80178)&& (arpInfo.arpEntry.ipv4Addr<=0xc0a80181) )
						{
							arp_valid_idx++;
							continue;
						}
						
						memcpy(lanNetInfo->lanHost.mac.octet, macEntry.mac.octet, MAC_ADDR_LEN);
						memcpy(lanNetInfo->lanHost.devName, arpInfo.lanNetInfo.dev_name, MAX_LANNET_DEV_NAME_LENGTH);
						memcpy(lanNetInfo->lanHost.brand, arpInfo.brandStr, MAX_LANNET_BRAND_NAME_LENGTH);
						memcpy(lanNetInfo->lanHost.os, arpInfo.osStr, MAX_LANNET_OS_NAME_LENGTH);
						lanNetInfo->lanHost.devType = arpInfo.lanNetInfo.dev_type;

						lanNetInfo->lanHost.ip = arpInfo.arpEntry.ipv4Addr;
						lanNetInfo->lanHost.port = getlanNetInfoConPort(macEntry.port_idx);
						lanNetInfo->lanHost.connectionType = (arpInfo.lanNetInfo.conn_type==RG_CONN_MAC_PORT) ? Host_Ethernet : Host_802_11;
						lanNetInfo->idleSecs = arpInfo.idleSecs;
						lanNetInfo->valid = 1;
						DEBUG_TRACE(0, ("%s %d ipv4Addr=%x, devType=%d, OS:%s, Brand:%s\n", __func__, __LINE__, arpInfo.arpEntry.ipv4Addr, lanNetInfo->lanHost.devType, lanNetInfo->lanHost.os, lanNetInfo->lanHost.brand));
					}
					else if(lanNetInfo->valid)
					{//delete this lanNetInfo
						DEBUG_TRACE(0, ("%s %d delete arpInfo.arpEntry.ipv4Addr=%x, arpInfo.idleSecs=%d\n", __func__, __LINE__, arpInfo.arpEntry.ipv4Addr, arpInfo.idleSecs));
						lanNetInfo->valid = 0;
						lanNetInfo->idleSecs = 0;
						lanNetInfo->lanHost.onLineTime = 0;
					}
				}
			}
			arp_valid_idx++;
    	}// end of if(ret == RT_ERR_RG_OK)
    	else
    	{
    		DEBUG_TRACE(0, ("%s %d break--ret=%x\n", __func__, __LINE__, ret));
			break;	//RT_ERR_RG_INDEX_OUT_OF_RANGE||RT_ERR_RG_NO_MORE_ENTRY_FOUND||RT_ERR_RG_NOT_INIT
    	}
	}

	return;
}

int flock_set(int fd, int type)  
{  
    struct flock lock;  
    lock.l_whence = SEEK_SET;  
    lock.l_start = 0;  
    lock.l_len = 0;  
    lock.l_type = type;  
    lock.l_pid = getpid();
  
    if((fcntl(fd, F_SETLKW, &lock)) < 0)  
    {  
        DEBUG_TRACE(0, ("Lock failed:type = %d\n", lock.l_type));  
        return 1;  
    }  
  
    return 0;  
}

/* send Lan Host MAC and DevName to server if this lan host online first time */
int sendNewLanHostInform()
{
	int i;
	for(i=0; i<MAX_LAN_HOST_INFO_NUM ;i++)
	{
		if(newLanHostIdx[i])
		{
			DEBUG_TRACE(0, ("send New LanHost MAC(%2x:%2x:%2x:%2x:%2x:%2x) and DevName(%s) to server.\n",
				lanNetInfoData[i].lanHost.mac.octet[0], lanNetInfoData[i].lanHost.mac.octet[1],
				lanNetInfoData[i].lanHost.mac.octet[2], lanNetInfoData[i].lanHost.mac.octet[3],
				lanNetInfoData[i].lanHost.mac.octet[4], lanNetInfoData[i].lanHost.mac.octet[5],
				lanNetInfoData[i].lanHost.devName) );

		
		/*<-- add send Inform code here -->*/

			newLanHostIdx[i] = 0;
		}
	}

	return 0;
}

void signal2_handler(int sig)
{
	writeToLanHostFile = 1;
}

/* write lanNetInfoData to LANHOSTINFOFILE file */
#define LANNETINFOFILE	"/var/lannetinfo"
int writeLanHostInfo()
{
	int fd;
	int i;

	fd = open(LANNETINFOFILE, O_RDWR|O_CREAT, 0644);
	if (fd < 0) 
	{
		return -1;
	}

	if( !flock_set(fd, F_WRLCK) )
	{
		/* clear /var/lannetinfo */
		if ( !truncate(LANNETINFOFILE, 0) )
		{
			gettimeofday(&interval,NULL);
			for(i = 0; i < MAX_LAN_HOST_INFO_NUM; i++)
			{
				if(lanNetInfoData[i].valid)
				{// means online now
					lanNetInfoData[i].lanHost.onLineTime = interval.tv_sec - lanNetInfoData[i].startOnLine;
					write(fd, (void *)&lanNetInfoData[i].lanHost, sizeof(lanHostInfo_t));
				}
			}
			flock_set(fd, F_UNLCK);
			writeToLanHostFile = 0;
		}
	}

	close(fd);
	return 0;
}


#define LANNETINFO_RUNFILE	"/var/run/lannetinfo.pid"
static void log_pid()
{
	FILE *f;
	pid_t pid;
	char *pidfile = LANNETINFO_RUNFILE;

	pid = getpid();
	if((f = fopen(pidfile, "w")) == NULL)
	{
		DEBUG_TRACE(0, ("%s %d create /var/run/lannetinfo.pid error\n", __func__, __LINE__));
		return;
	}
	fprintf(f, "%d\n", pid);
	fclose(f);
}
 
static void clr_pid()
{
	FILE *f;
	char *pidfile = LANNETINFO_RUNFILE;

	if((f = fopen(pidfile, "r")) != NULL){	
		fclose(f);
		unlink(LANNETINFO_RUNFILE);	
	}
}

static void clr_lanNetInfoFile()
{
	FILE *f;

	if((f = fopen(LANNETINFOFILE, "r")) != NULL){	
		fclose(f);
		unlink(LANNETINFOFILE);	
	}
}

void main(int argc, char **argv)
{
	int ret, arp_send=0;
	struct timeval tv;
	
	log_pid();
	gettimeofday(&queryTime,NULL);
	signal(SIGUSR2, signal2_handler);

	/* reset lanNetInfoData and init rw_lock */
	memset(lanNetInfoData, 0, sizeof(lanNetInfo_t)*MAX_LAN_HOST_INFO_NUM);

	/* echo 1 > /proc/rg/gather_lanNetInfo */
	system("/bin/echo 1 > /proc/rg/gather_lanNetInfo");

	while(1)
	{
		/************************************************
	 	* 1.
	 	*	a.send ping packet to each lanHost each second
	  	*   b.update lanNetInfo DataBase(add/update/delte)
	  	*************************************************/ 
    	gettimeofday(&tv,NULL);

		if (tv.tv_sec>queryTime.tv_sec)
		{
			queryTime.tv_sec = tv.tv_sec + NET_INFO_UPDATE_INTERVAL;

			arp_send++;
			if(arp_send >= NET_INFO_ARP_INTERVAL)
			{
				sendARPRequest();
				arp_send = 0;
			}
			updateLanNetInfoFromRGARPTable();
		}

		/*************************************************
		*2. send New LAN Host Inform
		*************************************************/
		sendNewLanHostInform();
	
		/*************************************************
		*3. write to LAN Host File
		*************************************************/
		if(writeToLanHostFile)
			writeLanHostInfo();

		usleep(5000);//each 5ms exec a time
    }
	
	clr_lanNetInfoFile();
	clr_pid();
	
	return;
}
