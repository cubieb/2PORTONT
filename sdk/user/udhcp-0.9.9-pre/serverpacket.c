/* serverpacket.c
 *
 * Constuct and send DHCP server packets
 *
 * Russ Dill <Russ.Dill@asu.edu> July 2001
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>

#include "packet.h"
#include "debug.h"
#include "dhcpd.h"
#include "options.h"
#include "leases.h"
#include "files.h"

#include "../boa/src/LINUX/mib.h"
// Added by Mason Yu for Half Bridge
#include <net/if.h>
#include <linux/sockios.h>
#if defined(IP_BASED_CLIENT_TYPE)
#include <stdio.h>
#endif
#include <stdlib.h>


//#ifdef _USE_RSDK_WRAPPER_
void addDhcpdOption(struct dhcpMessage *packet);
//#endif //_USE_RSDK_WRAPPER_


#ifdef CTC_DHCP_OPTION43
#define CONFIGVERSTR0     'C'
#define CONFIGVERSTR1     'T'
#define CONFIGVERSTR2     'C'
#define CONFIGVERSTR3     '0'
char option43[1+1+254]=
	{43, 6, 1, 4, CONFIGVERSTR0, CONFIGVERSTR1, CONFIGVERSTR2, CONFIGVERSTR3,0,0};
#endif
#ifdef CTC_DHCP_OPTION60
char option60[1+1+254]=
	{60, 2, 0, 0,}; // FOR dhcp ack, we just send enterprise code to clients.
const char IPTABLES[] = "/bin/iptables";
#endif

extern int serverpool;

#if defined(IP_BASED_CLIENT_TYPE)
static char *g_apszCTCDeviceName[] = 
{
    "Computer",
    "Camera",
    "HGW",
    "STB",
    "Phone",
    NULL
};
#endif

// Kaohj --- TR111 Part 1
#ifdef _CWMP_TR111_
struct device_id_t opt125_deviceId;

//#define _TR111_TEST_
#define TR111_DEVICEFILE	"/var/udhcpd/tr111device.txt"

/* Dump the option 125 device Identity to file
 * 0: fail
 * 1: successful
 */
int dump_deviceId()
{
	FILE *fp;
	int i;
	char tmp[160];
	struct device_id_t *pdevId;
	
	fp = fopen( TR111_DEVICEFILE, "w" );
	if (!fp)
		return 0;
	
	pdevId = opt125_deviceId.next;
	
	i = 1;
	while (pdevId) {
		sprintf( tmp, "%d %s", i, pdevId->oui );
		if (pdevId->productClass[0])
			sprintf( tmp, "%s?%s?%s\n", tmp, pdevId->productClass, pdevId->serialNo);
		else
			sprintf( tmp, "%s?%s\n", tmp, pdevId->serialNo);
		fwrite( tmp, 1, strlen(tmp), fp );
		i++;
		pdevId = pdevId->next;
	}
	
	fclose(fp);
	return 1;
}
/* Device with option 125 has been cached, add device ID into option 125 device list.
 * 0: fail
 * 1: replace
 * 2: add
 */
int add_deviceId(struct device_id_t *deviceId)
{
	struct device_id_t *pdevId;
	
	pdevId = opt125_deviceId.next;
	// find deviceId
	while (pdevId) {
		if (pdevId->yiaddr == deviceId->yiaddr)
			break;
		pdevId = pdevId->next;
	}
	
	if (pdevId) { // found, replace it
		//printf("replace deviceId: ip 0x%x\n", pdevId->yiaddr);
		memcpy(pdevId->oui, deviceId->oui, 7);
		memcpy(pdevId->serialNo, deviceId->serialNo, 65);
		memcpy(pdevId->productClass, deviceId->productClass, 65);
		return 1;
	}
	else { // add a new one
		//printf("add new deviceId: ip 0x%x\n", deviceId->yiaddr);
		pdevId = xmalloc(sizeof(struct device_id_t));
		if (pdevId) {
			memcpy(pdevId, deviceId, sizeof(struct device_id_t));
			pdevId->next = opt125_deviceId.next;
			opt125_deviceId.next = pdevId;
			return 2;
		}
		else
			printf("%s: xmalloc fail\n", __FUNCTION__);
	}
	
	return 0;
}

/* Device iaddr is out, remove its device ID from option 125 device list
 * 0: fail
 * 1: successful
 */
int del_deviceId(u_int32_t iaddr)
{
	struct device_id_t *preId, *curId;
	
	preId = &opt125_deviceId;
	curId = opt125_deviceId.next;
	// find deviceId
	while (curId) {
		if (curId->yiaddr == iaddr)
			break;
		preId = curId;
		curId = curId->next;
	}
	
	if (curId) { // found
		preId->next = curId->next;
		free(curId);
		return 1;
	}
	
	return 0;
}

void clear_all_deviceId()
{
	struct device_id_t *curId, *tmpId;
	
	curId = opt125_deviceId.next;
	
	while (curId) {
		tmpId = curId;
		curId = curId->next;
		free(tmpId);
	}
}

int handle_tr111(struct dhcpMessage *oldpacket, u_int32_t iaddr)
{
	int ret=0;
	//static int ins_num=1;
	unsigned char *pOpt125=NULL;
#ifndef _TR111_TEST_
	pOpt125 = get_option(oldpacket, DHCP_VI_VENSPEC);
#else
      #if 0
	unsigned char testbuf[]={ 0x00, 0x00, 0x0d, 0xe9, 0x18, 0x01, 0x06, 0x30,
				  0x30, 0x65, 0x30, 0x64, 0x34, 0x02, 0x09, 0x30,
				  0x30, 0x30, 0x30, 0x2d, 0x30, 0x30, 0x30, 0x31,
				  0x03, 0x03, 0x49, 0x47, 0x44, 0x00};
      #else
	unsigned char testbuf[]={
	0x00 , 0x00 , 0x0d , 0xe9 , 0x2d , 0x01 , 0x06 , 0x30 , 0x30 , 0x30,
	0x31 , 0x30 , 0x32 , 0x02 , 0x10 , 0x30 , 0x30 , 0x30 , 0x31 , 0x30 , 0x32 , 0x2d , 0x34 , 0x32 , 0x38 , 0x32,
	0x38 , 0x38 , 0x38 , 0x32 , 0x39 , 0x03 , 0x11 , 0x43 , 0x44 , 0x52 , 0x6f , 0x75 , 0x74 , 0x65 , 0x72 , 0x20,
	0x56 , 0x6f , 0x49 , 0x50 , 0x20 , 0x41 , 0x54 , 0x41 };
      #endif
	pOpt125 = testbuf;
#endif
	struct device_id_t cur_devId={iaddr, 3561, "", "", "", 0};


	if(pOpt125)
	{
		unsigned int ent_num, *pUInt;
		unsigned short data_len;
		char *GW_OUI=cur_devId.oui, *GW_SN=cur_devId.serialNo, *GW_CLASS=cur_devId.productClass;
		
		pUInt = (unsigned int*)pOpt125;
		ent_num = ntohl( *pUInt );
		data_len = (unsigned short)pOpt125[4];

		//if more than one enterprise-number in the same option125??
		if( ent_num==3561 )
		{
			unsigned char *pStart;
			
			//sub-option
			pStart = &pOpt125[5];
			while( data_len>0 )
			{
				unsigned char sub_code, sub_len, *sub_data;
				
				sub_code = pStart[0];
				sub_len = pStart[1];
				sub_data = &pStart[2];
				
				if( data_len < sub_len+2 )
					break;
					
				switch( sub_code )
				{
				case 1://DeviceManufacturerOUI
					if( sub_len<7 )
					{
						strncpy( GW_OUI, sub_data, sub_len );
						GW_OUI[sub_len]=0;
					}
					break;
				case 2://DeviceSerialNumber
					if( sub_len<65 )
					{
						strncpy( GW_SN, sub_data, sub_len );
						GW_SN[sub_len]=0;
					}
					break;
				case 3://DeviceProductClass
					if( sub_len<65 )
					{
						strncpy( GW_CLASS, sub_data, sub_len );
						GW_CLASS[sub_len]=0;
					}
					break;
				default:
					//unknown suboption
					break;
				}
				
				
				pStart = pStart+2+sub_len;
				data_len = data_len-sub_len-2;
			}

			
			//LOG(LOG_ERR,  "%d: oui:%s sn:%s class:%s ", __LINE__,GW_OUI,GW_SN,GW_CLASS  );
			if( *GW_OUI && *GW_SN && *GW_CLASS )
			{
				//sprintf( buf, "%s?%s?%s", GW_OUI,GW_CLASS,GW_SN );
				add_deviceId(&cur_devId);
				dump_deviceId();
			}else if( *GW_OUI && *GW_SN  )
			{
				//sprintf( buf, "%s?%s", GW_OUI,GW_SN );
				add_deviceId(&cur_devId);
				dump_deviceId();
			}else{
				//error
			}			
		}
	}
	
	return ret;
}
#endif // _CWMP_TR111_

#if defined(IP_BASED_CLIENT_TYPE)
int parse_CTC_Vendor_Class(struct dhcpMessage *packet, unsigned char* option60, struct dhcp_ctc_client_info *pstClientInfo)
{
	unsigned short enterprise_code=(*option60<<8)+*(option60+1);
	unsigned char *fieldtype;
	unsigned char fieldlength;
	int iOptlen, i;
    
	DEBUG_CHN_TEL("__FUNCTION__, enterprise_code=%04x\n", enterprise_code);

	pstClientInfo->iCategory = NULL;
    
	if(enterprise_code!=0x0000)
	{
		//ql 20090119 START: for imagenio STB string should be "[IAL]"
/*ping_zhang:20090312 START:add STB checkbox for designate STB device type*/
	#if 0 //mark for STB with different option60
		if (strstr(option60, "[IAL]"))
			pstClientInfo->category = CTC_STB;
		else
			pstClientInfo->category = CTC_UNKNOWN;
	#endif
/*ping_zhang:20090312 END*/
#ifdef IMAGENIO_IPTV_SUPPORT
		struct client_category_t *pDhcp;
		for (pDhcp=server_config.clientRange; pDhcp; pDhcp=pDhcp->next) {
			if (strstr(option60, pDhcp->option60)) {
/*ping_zhang:20090312 START:add STB checkbox for designate STB device type*/
				pstClientInfo->category = pDhcp->device_type;
/*ping_zhang:20090312 END*/
				pstClientInfo->iCategory = pDhcp;
				
				return 0;
			}
		}
#endif
		//ql 20090119 END
		return -1;// not china telecom enterprise code.. 
	}
    
	fieldtype=(option60+2);
    iOptlen = *(option60-1);
    while (iOptlen > 0)
    {
        fieldlength = *(fieldtype + 1);
        switch (*fieldtype)
        {
            case Vendor:
                if ((fieldlength < DHCP_CTC_MIN_FIELD_LEN)
                    || (fieldlength > DHCP_CTC_MAX_FIELD_LEN))
                {
                    return -1;
                }

                memcpy(pstClientInfo->szVendor, fieldtype + 2, fieldlength);
                pstClientInfo->szVendor[fieldlength] = 0;
                break;

            case Category:
                if ((fieldlength < DHCP_CTC_MIN_FIELD_LEN)
                    || (fieldlength > DHCP_CTC_MAX_FIELD_LEN))
                {
                    return -1;
                }
                
		struct client_category_t *pDhcp;
		for (pDhcp=server_config.clientRange; pDhcp; pDhcp=pDhcp->next) {
			if (strstr(fieldtype + 2, pDhcp->option60)) {
				pstClientInfo->iCategory = pDhcp;
				
                for (i = 0; g_apszCTCDeviceName[i]; i++)
                {
                    if ((fieldlength == strlen(g_apszCTCDeviceName[i]))
                        && (0 == memcmp(fieldtype + 2, g_apszCTCDeviceName[i], fieldlength)))
                    {
		                        pstClientInfo->category = i;
                        break;
                    }
                }
                break;
			}
		}
		pstClientInfo->iCategory = NULL;

                break;

            case Model:
                if ((fieldlength < DHCP_CTC_MIN_FIELD_LEN)
                    || (fieldlength > DHCP_CTC_MAX_FIELD_LEN))
                {
                    return -1;
                }

                memcpy(pstClientInfo->szModel, fieldtype + 2, fieldlength);
                pstClientInfo->szModel[fieldlength] = 0;
                break;

            case Version:
                if ((fieldlength < DHCP_CTC_MIN_FIELD_LEN)
                    || (fieldlength > DHCP_CTC_MAX_FIELD_LEN))
                {
                    return -1;
                }

                memcpy(pstClientInfo->szVersion, fieldtype + 2, fieldlength);
                pstClientInfo->szVersion[fieldlength] = 0;
                break;

            case ProtocolType:
                memcpy((char *)(&pstClientInfo->stPortForwarding.usProtocol),
                        fieldtype + 2,
                        sizeof(unsigned short));
                memcpy((char *)(&pstClientInfo->stPortForwarding.usPort),
                        fieldtype + 4,
                        sizeof(unsigned short));
                break;

            default:
                break;
        }

        iOptlen -= fieldlength + 2;
        fieldtype += fieldlength + 2;
    }

	return 0;
}

//int check_type(u_int32_t addr, enum DeviceType devicetype)
int check_type(u_int32_t addr, struct client_category_t *deviceCategory)
{
	int ret=0;

	if (serverpool)
	{
		if ((addr<ntohl(server_config.start)) || (addr>ntohl(server_config.end)))
			ret = 1;
	}
	else
	{
		if (deviceCategory == NULL)
		{
			if ((addr<ntohl(server_config.start)) || (addr>ntohl(server_config.end)))
				ret = 1;
		}
		else
		{
			if ((addr<ntohl(deviceCategory->ipstart)) || 
				(addr>ntohl(deviceCategory->ipend)))
				ret = 1;
		}
	}
#if 0
	switch(devicetype)
	{
		case CTC_Computer:
			if(addr<ntohl(server_config.pcstart)||addr>ntohl(server_config.pcend))
				ret=1;
			break;
		case CTC_Camera:
			if(addr<ntohl(server_config.cmrstart)||addr>ntohl(server_config.cmrend))
				ret=1;
			break;
		case CTC_STB:
			if(addr<ntohl(server_config.stbstart)||addr>ntohl(server_config.stbend))
				ret=1;
			break;
		case CTC_PHONE:
			if(addr<ntohl(server_config.phnstart)||addr>ntohl(server_config.phnend))
				ret=1;
			break;
		case CTC_HGW:
			if(addr<ntohl(server_config.hgwstart)||addr>ntohl(server_config.hgwend))
				ret=1;
			break;
		case CTC_UNKNOWN:
			if(addr<ntohl(server_config.start)||addr>ntohl(server_config.end))
				ret=1;
			if((addr>ntohl(server_config.pcstart)&&addr<ntohl(server_config.pcend))||
			   	(addr>ntohl(server_config.cmrstart)&&addr<ntohl(server_config.cmrend))||
				(addr>ntohl(server_config.stbstart)&&addr<ntohl(server_config.stbend))||
				(addr>ntohl(server_config.phnstart)&&addr<ntohl(server_config.phnend)))
				ret=1;
			break;
		default:
			break;
	}
#endif
	
	return ret;
}

#endif
//added by jim luo to support china telecom e8 spec.
#ifdef CONFIG_CTC_E8_CLIENT_LIMIT
#if 0
/*
static int acceptedAllowedNumbers(struct CTC_Clients *clients, int range);
static void blockingClient(unsigned int ip)
{
	unsigned char buffer[128];
		//va_cmd("/bin/iptables"  6, 1, "-I", "FORWARD", "-s", ip, "-j", "DROP");
	sprintf(buffer, "/bin/iptables -I FORWARD -s %d.%d.%d.%d -j DROP", 
					(ip>>24)&0xFF, (ip>>16)&0xFF, (ip>>8)&0xFF, (ip>>0)&0xFF);

	system(buffer);
	printf("blocking client %08x\n", ip);
}
static void unblockingClient(unsigned int ip)
{
	unsigned char buffer[128];
	//va_cmd("/bin/iptables"  6, 1, "-D", "FORWARD", "-s", ip, "-j", "DROP");
	sprintf(buffer, "/bin/iptables -D FORWARD -s %d.%d.%d.%d -j DROP", 
					(ip>>24)&0xFF, (ip>>16)&0xFF, (ip>>8)&0xFF, (ip>>0)&0xFF);

	system(buffer);
	printf("unblocking client %08x\n", ip);
}

//push out the unused elements in list.....
static void reorderClientsList(struct CTC_Clients *clients, int range)
{
	int i;
	for(i=0;  i<range; i++)
	{
		if(clients[i].ip==0)
		{	//copress
			int j,k;
			j=i;
			for(k=0; k<range-i-1; k++, j++)
				memcpy(&clients[j], &clients[j+1], sizeof(struct CTC_Clients)); 
		}
	}
}
static void removeUnexistClients(struct CTC_Clients *clients, int range)
{
	int i;
	for(i=0; i< range; i++)
	{
		if((clients[i].ip!=0)&&(clients[i].detected==0))
		{	// the blocked connection is not detected, then unblock it from iptables.
			if(clients[i].state==0) //blocked
				unblockingClient(clients[i].ip); //remove the iptables blocking entry
			clients[i].ip=0;
			clients[i].state=0;
		}
		if(clients[i].ip==0)
		{	//copress
			int j,k;
			j=i;
			for(k=0; k<range-i-1; k++, j++)
				memcpy(&clients[j], &clients[j+1], sizeof(struct CTC_Clients)); 
		}
		
	}
	return ; 
}
static int updateClientsState(struct CTC_Clients *clients, int range, int maxclients)
{
	int clientsAllowedAlready;
	int clientsCanAllowed;
	int clientsAllowedFromBlocking=0;
	int i,j;
	clientsAllowedAlready=acceptedAllowedNumbers(clients, range);
	clientsCanAllowed=maxclients-clientsAllowedAlready;   //we can allow some blocked connections to be allowed to access WAN..
	for(i=0,j=0; i< clientsCanAllowed; i++)
	{
		for( ; j<range; j++)
		{
			if(clients[j].ip && clients[j].state==0)  //blocked state
			{
				clients[j].state=1; // allowed state.
				unblockingClient(clients[j].ip);
				clientsAllowedFromBlocking++;
				break;
			}
		}
	}
	return clientsAllowedAlready+clientsAllowedFromBlocking; // return the updated allow/block connection nunbers to caller.
}
static int clientInList(struct CTC_Clients *clients, int range,  unsigned long ip, int * state)
{
	int i;
	for(i=0; i< range; i++)
	{
		if(clients[i].ip==0)
			return -1;
		if(clients[i].ip==ip)
		{
			*state=clients[i].state;
			//clients[i].detected=1; // set detected flags.
			return i; // found..
		}
	}
	return -1; //not found..
}
static int acceptedAllowedNumbers(struct CTC_Clients *clients, int range)
{
	int i;
	int numbers=0;
	for(i=0; i<range; i++)
	{
		if( ( clients[i].ip!=0) &&(clients[i].state==1)) //unblocking
			numbers++;
	}
	return numbers;
}
static int insertClients(struct CTC_Clients  *clients, int range, unsigned int ip, int state)
{
	int i;
	for(i=0; i<range; i++)
	{
		if(clients[i].ip==ip || clients[i].ip==0)
		{
			clients[i].ip=ip;
			clients[i].state=state;
			return 0;
		}
	}
	return -1;
}
static struct CTC_Clients *getNextClient(struct CTC_Clients * clients, int *range)
{
	int i;
	if(*range==0)
		return NULL;
	for(i=0;i<*range; i++)
	{
		if(clients->ip!=0)
		{
			*range=*range-i-1;
			return clients;
		}
		clients++;
	}
	return NULL;
}


static int checkClients(unsigned int ip, enum DeviceType devtype)
{
	//using Arp request to detect the accepted-clients is still here.
	struct CTC_Clients * client;
	unsigned char buffer[1024];
	unsigned char recv_buffer[1024];
	unsigned char *curptr;
	int len,i;
	unsigned int ipparts[4];
	int range;
	int pcNum=0;
	int cameraNum=0;
	int pcNumAllowed=0;
	int pcNumBlocked=0;
	int cameraNumAllowed=0;
	int cameraNumBlocked=0;
	FILE * fd;
	curptr=buffer;
	//printf("checkClients in\n");
	fd=fopen(CLIENTSMONITOR, "r+");
	if(fd==NULL)
		return -1;
	//printf("checkClients in 2\n");
	memset(buffer, 0, sizeof(buffer));
	// send Arp Request to eavy accepted-clients
	client=accepted_PC_Clients;
	range=MAXPCCLIENTS;
	len=sprintf(curptr, "1 "); // set detect flag...
	if(len < 0)
		goto err;
	curptr+=len;
	
	while(client=getNextClient(client, &range))
	{
		//sendArpRequest(*client);
		len=sprintf(curptr, "%d.%d.%d.%d ", (client->ip>>24)&0xFF, (client->ip>>16)&0xFF,
										(client->ip>>8)&0xFF, (client->ip>>0)&0xFF); // set ip to detect...
		if(len < 0)
			goto err;
		curptr+=len;
		client++; //next client
	}
	//printf("checkClients in 3\n");
	client=accepted_Camera_Clients;
	range=MAXCAMERACLIENTS;
	while(client=getNextClient(client, &range))
	{
		//sendArpRequest(*client);
		len=sprintf(curptr, "%d.%d.%d.%d ", (client->ip>>24)&0xFF, (client->ip>>16)&0xFF,
										(client->ip>>8)&0xFF, (client->ip>>0)&0xFF); // set ip to detect...
		if(len < 0)
			goto err;
		curptr+=len;
		client++;
	}
	//printf("checkClients in 4\n");
	// before gather detected clients info, zero detected flag...
	for(i=0; i< MAXPCCLIENTS; i++)
	{
		accepted_PC_Clients[i].detected=0;
	}
	for(i=0; i< MAXCAMERACLIENTS; i++)
	{
		accepted_Camera_Clients[i].detected=0;
	}


// do 3 times try detect...
{
	int localcount=0;
retry:
	//printf("retry count\n");
	fclose(fd);
	fd=fopen(CLIENTSMONITOR, "r+");
	fwrite(buffer, 1, strlen(buffer),fd);
	fflush(fd);
	//though china telecom spec need 3 times detect to determined if still existed, but one time is enough...
	usleep(200000);// wait arp reply
	memset(recv_buffer, 0, sizeof(recv_buffer));
	curptr=recv_buffer;
	//printf("checkclients: read\n");
	len=fread(curptr, 1,  1024, fd);
	//fscanf(fd, "%-16s 0x%-8x0x%-6x%s"
	//						"   *     %s\n", 
	if(len < 0)
		goto err;
	//printf("checkClients in 5\n");
	printf("%s\n", curptr);

	for(i=0; i< MAXCAMERACLIENTS+MAXPCCLIENTS; i++)
	{
		unsigned long ip;
		int state;
		int index;
		if(sscanf(curptr, "%d.%d.%d.%d", &ipparts[0],&ipparts[1],&ipparts[2],&ipparts[3])<=0)
			break;
		ip=(ipparts[0]<<24) |( ipparts[1]<<16) |( ipparts[2]<<8) |( ipparts[3]<<0);
		if((index=clientInList(accepted_PC_Clients, MAXPCCLIENTS, ip, &state))>=0)
		{
			accepted_PC_Clients[index].detected=1;
		}else if((index=clientInList(accepted_Camera_Clients, MAXCAMERACLIENTS, ip, &state))>=0)
		{
			accepted_Camera_Clients[index].detected=1;
		}
		//search next ip.
		while((*curptr != ' ') && (*curptr != 0))
		{
			curptr++;
		}
		if(*curptr==0)
			break;
		curptr++;
	}
	localcount++;
	if(localcount<3)
		goto retry;
}
	// now all the clients-connected have been polled. 
	removeUnexistClients(accepted_PC_Clients,MAXPCCLIENTS);
	removeUnexistClients(accepted_Camera_Clients, MAXCAMERACLIENTS);
	pcNumAllowed=updateClientsState(accepted_PC_Clients,MAXPCCLIENTS, maxPCClients);
	cameraNumAllowed=updateClientsState(accepted_Camera_Clients,MAXCAMERACLIENTS, maxCameraClients);
	if(devtype == CTC_Carema)
	{
		int state;
		if(cameraNumAllowed<maxCameraClients)
			state=1;   //allowed
		else
		{
			state=0;
			blockingClient(ip);
		}
		insertClients(accepted_Camera_Clients,MAXCAMERACLIENTS, ip, state);
		
		
	}else // Computers device default
	{
		int state;
		if(pcNumAllowed<maxPCClients)
			state=1;   //allowed
		else
		{
			state=0;
			blockingClient(ip);
		}
		insertClients(accepted_PC_Clients,MAXPCCLIENTS, ip, state);
	}

//reset arp-detecting to idle
	memset(buffer, 0, sizeof(buffer));
	curptr=buffer;
	len=sprintf(curptr, "0 "); // set detect flag...
	if(len < 0)
		goto err;
	//printf("checkClients in 6\n");
	fwrite(buffer, 1, strlen(buffer),fd);
	fclose(fd);
	return 0;
err:
	//printf("checkClients err out\n");
	fclose(fd);
	return -1;
	
	
}*/

static void handle_clients_limit(struct dhcpMessage *packet,unsigned long yiaddr)
{
	unsigned char *classVendor;
	enum DeviceType      devicetype;
	char pass_buffer[256];
	char *curptr=pass_buffer;
	int len;
	FILE * fd;
    struct dhcp_ctc_client_info stClientInfo;
/*ping_zhang:20090313 START:Telefonica DHCP option new request*/
	unsigned char vendor_len = 0;
	unsigned char classVendorStr[256] = {0};
/*ping_zhang:20090313 END*/
	//printf("checkClients in\n");
	
	//check the device type: PC or Camera...
	printf("handle_clients_limit in\n");
	if(!(classVendor=get_option(packet, DHCP_VENDOR)))
		//default : PC clients....
		devicetype=CTC_Computer;
	else
	{
/*ping_zhang:20090313 START:Telefonica DHCP option new request*/
		vendor_len=*(unsigned char*)(classVendor-OPT_LEN);
		memcpy(classVendorStr,classVendor,vendor_len);
		classVendorStr[vendor_len]=0;
/*ping_zhang:20090313 END*/
        	memset(&stClientInfo, 0, sizeof(struct dhcp_ctc_client_info));
/*ping_zhang:20090313 START:Telefonica DHCP option new request*/
		//parse_CTC_Vendor_Class(packet, classVendor, &stClientInfo);
		parse_CTC_Vendor_Class(packet, classVendorStr, &stClientInfo);
/*ping_zhang:20090313 END*/
        devicetype = (enum DeviceType)(stClientInfo.iCategory);
	}
	// send it to kernel...
	fd=fopen(CLIENTSMONITOR, "r+");
	if(fd==NULL)
	{
		printf("no such proc file %s\n", CLIENTSMONITOR);
		return;
	}
	memset(pass_buffer, 0, sizeof(pass_buffer));

//debug management path through proc/ClientsMonitor.....
if(0) // test pass, below path is OK....
{
	len=sprintf(curptr, "flagLimitOnAll "); // set ip to detect...
	if(len < 0)
		goto err;
	curptr+=len;
	len=sprintf(curptr, "%d\n", 0);
	curptr+=len;
	
	len=sprintf(curptr, "limitOnAll "); // set ip to detect...
	if(len < 0)
		goto err;
	curptr+=len;
	len=sprintf(curptr, "%d\n", 252);
	curptr+=len;

	len=sprintf(curptr, "computerLimitEnable "); // set ip to detect...
	if(len < 0)
		goto err;
	curptr+=len;
	len=sprintf(curptr, "%d\n", 0);
	curptr+=len;
	
	len=sprintf(curptr, "limitOnComputer "); // set ip to detect...
	if(len < 0)
		goto err;
	curptr+=len;
	len=sprintf(curptr, "%d\n", 201);
	curptr+=len;
	
	len=sprintf(curptr, "cameraLimitEnable "); // set ip to detect...
	if(len < 0)
		goto err;
	curptr+=len;
	len=sprintf(curptr, "%d\n", 1);
	curptr+=len;
	
	len=sprintf(curptr, "limitOnCamera "); // set ip to detect...
	if(len < 0)
		goto err;
	curptr+=len;
	len=sprintf(curptr, "%d\n", 39);
	curptr+=len;
}
	//the format should conformance with kernel arp.c
	len=sprintf(curptr, "IP              Types\n"); // set ip to detect...
	if(len < 0)
		goto err;
	curptr+=len;
	len=sprintf(curptr, "%d.%d.%d.%d %d\n", (yiaddr>>24)&0xFF, (yiaddr>>16)&0xFF,
										(yiaddr>>8)&0xFF, (yiaddr>>0)&0xFF, (int)devicetype); // set ip to detect...
	if(len < 0)
		goto err;
	curptr+=len;
	len=fwrite(pass_buffer, 1, 2+(unsigned)curptr-(unsigned)pass_buffer, fd);	//some trail zero should be included.	
	fclose(fd);
	printf("handle_clients_limit exit len=%d\n", len);	
	if(1)
	{
			printf("proc wirrten: %s", pass_buffer);
	}
	return;
err:
	fclose(fd);
	return;
	
}
#endif
#endif
/* send a packet to giaddr using the kernel ip stack */
static int send_packet_to_relay(struct dhcpMessage *payload)
{
	DEBUG(LOG_INFO, "Forwarding packet to relay");

	return kernel_packet(payload, server_config.server, SERVER_PORT,
			payload->giaddr, SERVER_PORT);
}


/* send a packet to a specific arp address and ip address by creating our own ip packet */
static int send_packet_to_client(struct dhcpMessage *payload, int force_broadcast)
{
	unsigned char *chaddr;
	u_int32_t ciaddr;
	
	if (force_broadcast) {
		DEBUG(LOG_INFO, "broadcasting packet to client (NAK)");
		ciaddr = INADDR_BROADCAST;
		chaddr = MAC_BCAST_ADDR;
	} else if (payload->ciaddr) {
		DEBUG(LOG_INFO, "unicasting packet to client ciaddr");
		ciaddr = payload->ciaddr;
		chaddr = payload->chaddr;
	} else if (ntohs(payload->flags) & BROADCAST_FLAG) {
		DEBUG(LOG_INFO, "broadcasting packet to client (requested)");
		ciaddr = INADDR_BROADCAST;
		chaddr = MAC_BCAST_ADDR;
	} else {
		DEBUG(LOG_INFO, "unicasting packet to client yiaddr");
		ciaddr = payload->yiaddr;
		chaddr = payload->chaddr;
	}
	return raw_packet(payload, server_config.server, SERVER_PORT, 
			ciaddr, CLIENT_PORT, chaddr, server_config.ifindex);
}


/* send a dhcp packet, if force broadcast is set, the packet will be broadcast to the client */
static int send_packet(struct dhcpMessage *payload, int force_broadcast)
{
	int ret;

	if (payload->giaddr)
		ret = send_packet_to_relay(payload);
	else ret = send_packet_to_client(payload, force_broadcast);
	return ret;
}


static void init_packet(struct dhcpMessage *packet, struct dhcpMessage *oldpacket, char type)
{
	init_header(packet, type);
	packet->xid = oldpacket->xid;
	memcpy(packet->chaddr, oldpacket->chaddr, 16);
	packet->flags = oldpacket->flags;
	packet->giaddr = oldpacket->giaddr;
	packet->ciaddr = oldpacket->ciaddr;
	add_simple_option(packet->options, DHCP_SERVER_ID, server_config.server);
}


/* add in the bootp options */
static void add_bootp_options(struct dhcpMessage *packet)
{
	packet->siaddr = server_config.siaddr;
	if (server_config.sname)
		strncpy(packet->sname, server_config.sname, sizeof(packet->sname) - 1);
	if (server_config.boot_file)
		strncpy(packet->file, server_config.boot_file, sizeof(packet->file) - 1);
}


// Added by Mason Yu for MAC Base assignment
unsigned long find_IP_by_Mac(unsigned char * packet_chaddr)
{
	char tmp_mac[6][3];
	char chaddr[19];
	FILE *fp;
	char temps[0x100];
	unsigned long value;
	char *str, *endptr;
	struct in_addr matchIp;
	int offset;				
	int i;
	
	for (i=0; i<6; i++) {
		if ( packet_chaddr[i] <= 0xf )
			sprintf(tmp_mac[i], "0%x",  packet_chaddr[i]);
		else
			sprintf(tmp_mac[i], "%x",  packet_chaddr[i]);	        		
	}
	
	sprintf(chaddr, "%s-%s-%s-%s-%s-%s:", tmp_mac[0], tmp_mac[1], tmp_mac[2], tmp_mac[3], tmp_mac[4], tmp_mac[5]);
	//printf("chaddr=%s\n", chaddr);	        		
	
	value = 0;
	
	if ((fp = fopen("/var/dhcpdMacBase.txt", "r")) == NULL)
	{
		printf("Open file /var/dhcpdMacBase.txt fail !\n");
		return 0;
	}				
	
	while (fgets(temps,0x100,fp))
	{
		if (temps[strlen(temps)-1]=='\n')
			temps[strlen(temps)-1] = 0;
		
		if (str=strstr(temps, chaddr))
		{							
			offset = strlen(chaddr)+1;			
			//printf("The string is %s\n", str+offset);
									
			inet_aton(str+offset, &matchIp);	
			value = matchIp.s_addr;	
			//printf("value=0x%x\n", value);						
			break;	
			
		} else {					
			continue;
			
	        }
		
	}	
	
  fclose(fp);
	return value;							        		
}

//star add: for static ip based Mac
int find_Mac_by_IP(u_int32_t ipaddr)
{
	struct in_addr matchIp;
	char ip[20];
	char * str=0;
	FILE *fp;
	int value=0;
	char temps[0x100];

	matchIp.s_addr = ipaddr;
	strcpy(ip,inet_ntoa(matchIp));

	if ((fp = fopen("/var/dhcpdMacBase.txt", "r")) == NULL)
	{
		printf("Open file /var/dhcpdMacBase.txt fail !\n");
		return 0;
	}	

	while (fgets(temps,0x100,fp))
	{
		if (temps[strlen(temps)-1]=='\n')
			temps[strlen(temps)-1] = 0;
		
		if (str=strstr(temps, ip))
		{							
			value = 1;				
			break;	
			
		} else {					
			continue;
		}	
	}		
	//jim: we should release all resource, fixed by jim
	fclose(fp);
	return value;
	
}

#ifdef SUPPORT_DHCP_RESERVED_IPADDR
static const char DHCPReservedIPAddrFile[] = "/var/udhcpd/DHCPReservedIPAddr.txt";
int isReservedIPAddress(u_int32_t ipaddr)
{
	FILE *fp;
	int ret=0, step=0;
	char buf[32];
	unsigned int instnum=0;
	
	fp=fopen( DHCPReservedIPAddrFile, "r" );
	if(!fp) return ret;

#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	instnum = server_config.cwmpinstnum;
#endif //_PRMT_X_TELEFONICA_ES_DHCPOPTION_

	//LOG(LOG_INFO, "isReservedIPAddress(): ipaddr=0x%08x, instnum=%u", ipaddr, instnum );
	while( fgets(buf, 32, fp) )
	{
		char *p;
		
		p=strchr( buf, '\n' );
		if(p) *p=0;
		p=strchr( buf, '\r' );
		if(p) *p=0;

		//LOG(LOG_INFO, "isReservedIPAddress(): got (buf=%s)", buf );
		if(step==0)
		{
			if( (strncmp( buf, "START ", 6 )==0) && ( strlen(&buf[6])>0 ) )
			{
				unsigned int cur_instnum;				
				cur_instnum=atoi( &buf[6] );
				if(cur_instnum==instnum)
					step=1;
			}
		}else{
			struct in_addr matchIP;
			
			if( strncmp( buf, "END", 3 )==0 ) break;
			if( (inet_aton(buf,&matchIP)!=0) && (matchIP.s_addr==ipaddr) )
			{
				//LOG(LOG_INFO, "isReservedIPAddress(): found ipaddr=0x%08x", ipaddr );
				ret=1;
				break;
			}
		}
	}
	fclose(fp);	
	//LOG(LOG_INFO, "isReservedIPAddress(): end ret=%d", ret );	
	return ret;
}
#endif //SUPPORT_DHCP_RESERVED_IPADDR
	
// Added by Mason Yu for PPP Half Bridge
int PPP_HALFBRIDGE_GET_WANIP = FALSE;
//int PPP_HALFBRIDGE_GET_WANIP = TRUE;
int IPOA_HALFBRIDGE_GET_WANIP = FALSE;
int FIRST_SEND_PUBLIC_IP = FALSE;	
int FIRST_SEND_ACK = TRUE;
int RENEW_PUBLIC_IP = FALSE;

/* send a DHCP OFFER to a DHCP DISCOVER */
int sendOffer(struct dhcpMessage *oldpacket)
{
	struct dhcpMessage packet;
	struct dhcpOfferedAddr *lease = NULL;
	u_int32_t req_align, lease_time_align = server_config.lease;
	unsigned char *req, *lease_time;
	struct option_set *curr;
	struct in_addr addr;

    	// Added by Mason Yu for Half Bridge
    	// Added by Mason Yu. Access internet fail.
    	unsigned long myipaddr, hisipaddr, dnsipaddr;
    	FILE *fp;
    	struct ifreq ifr;
    	int i;       
    	static int skfd = -1;
    	//struct sockaddr_in *addrtemp;
    	//struct in_addr pAddr;
    	//char incoming_route[100];	
	
    	init_packet(&packet, oldpacket, DHCPOFFER);    
    
    	// Added by Mason Yu for Half Bridge, Start
    	DEBUG_CHN_TEL("server_config.ippt = %d\n", server_config.ippt);
    	if ( server_config.ippt )  {
        	if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			perror("creat socket fail");
			return (0);
		}
             
         	// If we can get WAN IP, that is the WAN interface is up.  
         	/*                 
         	if (skfd >= 0) {
             		strcpy(ifr.ifr_name, "ppp0");
             		ifr.ifr_addr.sa_family = AF_INET;
             		if (ioctl(skfd, SIOCGIFADDR, &ifr) == 0) {
             			PPP_HALFBRIDGE_GET_WANIP = TRUE;
            		 }
         	}
         	*/            
         	 
         	// Read global variables from sppp process.
	 	fp = fopen ("/tmp/PPPHalfBridge", "r");
         	if (fp) {
	 	       fread(&myipaddr, 4, 1, fp);	 	         
	 	       fread(&hisipaddr, 4, 1, fp);
	 	       // Added by Mason Yu. Access internet fail.
	 	       fread(&dnsipaddr, 4, 1, fp);        	   
	 	       fclose(fp);
	 	       PPP_HALFBRIDGE_GET_WANIP = TRUE;
         	} else {
         		PPP_HALFBRIDGE_GET_WANIP = FALSE;
         	}   
         
         	// Read global variables from goahead process.
	 	fp = fopen ("/tmp/IPoAHalfBridge", "r");
         	if (fp) {
	 	       fread(&myipaddr, 4, 1, fp);	  	        
	 	       fread(&hisipaddr, 4, 1, fp);	          	   
	 	       fclose(fp);	        
	 	       IPOA_HALFBRIDGE_GET_WANIP = TRUE;
         	} else {
         		IPOA_HALFBRIDGE_GET_WANIP = FALSE;
         	}
         
         	// If we can not find public IP in the release table, we should distribute the public IP to DHCP client.      
         	if (( (!(lease = find_lease_by_yiaddr(myipaddr)) ||
         	      /* or it expired and we are checking for expired leases */
	 	    	  (lease_expired(lease))) &&
         	
	 	    	  /* and it isn't on the network */
	 	          (!check_ip(myipaddr))  ) ||
	 	          ((lease = find_lease_by_yiaddr(myipaddr))&&
	 	          !memcmp(lease->chaddr,packet.chaddr,16))) {
  //       		  printf("No host use the public IP\n"); 
         		  FIRST_SEND_PUBLIC_IP = TRUE;
         	} 	
    	}
#ifdef IMAGENIO_IPTV_SUPPORT
	enum DeviceType devicetype;
	devicetype = getDeviceType();
#elif defined(IP_BASED_CLIENT_TYPE)
	enum DeviceType devicetype;
	struct client_category_t *deviceCategory;
	unsigned char *classVendor;
	struct dhcp_ctc_client_info stClientInfo;
	/*ping_zhang:20090313 START:Telefonica DHCP option new request*/
	unsigned char len = 0;
	unsigned char classVendorStr[256] = {0};
	/*ping_zhang:20090313 END*/

	if(!(classVendor=get_option(oldpacket, DHCP_VENDOR))) {
		//default : PC clients....
		devicetype = CTC_Computer;
		deviceCategory=NULL;
	}
	else
	{
		/*ping_zhang:20090313 START:Telefonica DHCP option new request*/
		len=*(unsigned char*)(classVendor-OPT_LEN);
		memcpy(classVendorStr,classVendor,len);
		classVendorStr[len]=0;
		/*ping_zhang:20090313 END*/
		memset(&stClientInfo, 0, sizeof(struct dhcp_ctc_client_info));
		/*ping_zhang:20090313 START:Telefonica DHCP option new request*/
		//parse_CTC_Vendor_Class(oldpacket, classVendor, &stClientInfo);
		parse_CTC_Vendor_Class(oldpacket, classVendorStr, &stClientInfo);
		/*ping_zhang:20090313 END*/
		/*ping_zhang:20090319 START:replace ip range with serving pool of tr069*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
		devicetype = (enum DeviceType)(stClientInfo.category);
#else
		devicetype = getDeviceType();
#endif
		/*ping_zhang:20090319 END*/
		deviceCategory = stClientInfo.iCategory;
	}
#endif
	
    	// Modified by Mason Yu for Half Bridge
    	// If user choose Half Bridge feature and the DHCP server send offer is the first time 
    	// after WAN interface is up, we should distribute the WAN IP address to DHCP client.       
    	if ( ((server_config.ippt == 1) && (PPP_HALFBRIDGE_GET_WANIP == TRUE) && (FIRST_SEND_PUBLIC_IP == TRUE)) ||
		((server_config.ippt == 1) && (IPOA_HALFBRIDGE_GET_WANIP == TRUE) && (FIRST_SEND_PUBLIC_IP == TRUE))) {           
    	     	/* Put the ip address into packet that WAN get from PPPoX Server  */ 
	     		packet.yiaddr = myipaddr;  
		               
    	} 
	else {	
	         /* ADDME: if static, short circuit */
	         /* the client is in our lease/offered table */
			 u_int32_t ip_addr;
			 ip_addr = find_IP_by_Mac(oldpacket->chaddr);
			 
			 if ((lease = find_lease_by_chaddr(oldpacket->chaddr))
			 	&& (!find_Mac_by_IP(lease->yiaddr))
			 	&& ((ip_addr == 0) || ((ip_addr != 0) && (ip_addr == lease->yiaddr)))
#ifdef SUPPORT_DHCP_RESERVED_IPADDR
			 	&& ((ip_addr != 0) || ((ip_addr == 0) && (isReservedIPAddress(lease->yiaddr)==0)))
#endif //SUPPORT_DHCP_RESERVED_IPADDR
			 	&& (!check_ip(lease->yiaddr))
#ifdef IP_BASED_CLIENT_TYPE
				&& ((lease->yiaddr==myipaddr)||((ip_addr != 0) || (!check_type(lease->yiaddr, deviceCategory))))
#endif
				// Mason Yu
				&& ( ntohl(lease->yiaddr) >= ntohl(server_config.start) )
				&& ( ntohl(lease->yiaddr) <= ntohl(server_config.end) )
				) {
				if (!lease_expired(lease)) 
	         			lease_time_align = lease->expires - time(0);	         	
	         		packet.yiaddr = lease->yiaddr;	         		
	         	
	        	 /* Or the client has a requested ip */
				// jim: starzhang added it to support ip/mac binding.
	         	} 	         	
#if defined(IP_BASED_CLIENT_TYPE) && defined(IMAGENIO_IPTV_SUPPORT)
			else if ((packet.yiaddr = find_IP_by_Mac(packet.chaddr)) && (!check_type(packet.yiaddr, deviceCategory)))
#else
	         	else if(packet.yiaddr = find_IP_by_Mac(packet.chaddr))
#endif
			/*ql 20090119 START: for ip/mac binding condition, if ip is not in right range, it should not be distributed.*/
//#ifndef IMAGENIO_IPTV_SUPPORT
//	         	else if(packet.yiaddr = find_IP_by_Mac(packet.chaddr))			
//#else
//			else if ((packet.yiaddr = find_IP_by_Mac(packet.chaddr)) && (!check_type(packet.yiaddr, deviceCategory)))
//#endif
	         	{	         		
	         		goto getip;
		   	}
			else if ((req = get_option(oldpacket, DHCP_REQUESTED_IP)) &&
             
		         	   /* Don't look here (ugly hackish thing to do) */
		         	   memcpy(&req_align, req, 4) &&
	             
		         	   /* and the ip is in the lease range */
		         	   ntohl(req_align) >= ntohl(server_config.start) &&
		         	   ntohl(req_align) <= ntohl(server_config.end) && 
		         	   (!find_Mac_by_IP(req_align)) &&
		         	   /*add by star, for static IP based Mac*/
		         	   /* and its not already taken/offered */ /* ADDME: check that its not a static lease */
#ifdef SUPPORT_DHCP_RESERVED_IPADDR
			 	   (isReservedIPAddress(req_align)==0) &&
#endif //SUPPORT_DHCP_RESERVED_IPADDR
		         	   ((!(lease = find_lease_by_yiaddr(req_align)) ||
		         	   
		         	   /* or its taken, but expired */ /* ADDME: or maybe in here */
		         	   lease_expired(lease)))
		         	   /* and it isn't on the network */
		    	     	     && !check_ip(req_align)
#ifdef IP_BASED_CLIENT_TYPE
					&& (!check_type(req_align, deviceCategory))
#endif
	    	     	     ) {	    	     	     		      	   	        
	         			packet.yiaddr = req_align; /* FIXME: oh my, is there a host using this IP? */
       
	         	/* otherwise, find a free IP */ /*ADDME: is it a static lease? */
	        	} 
			else {
				// Mason Yu. Find IP base on Mac
	//			packet.yiaddr = find_IP_by_Mac(packet.chaddr);
		        	
		        	// Modified by Mason Yu
		        	//packet.yiaddr = find_address(0);
	  	
		        	if (!packet.yiaddr){
#ifdef IP_BASED_CLIENT_TYPE
					 packet.yiaddr = find_address(0, deviceCategory);
#else
		        		 packet.yiaddr = find_address(0);	 
#endif
		         	}
		         	
		         	/* try for an expired lease */
		         	if (!packet.yiaddr){	
#ifdef IP_BASED_CLIENT_TYPE
					packet.yiaddr = find_address(1, deviceCategory);
#else
		         		packet.yiaddr = find_address(1);
#endif
		         	}	
	         	}
    	}

getip:
	
	if(!packet.yiaddr) {
		LOG(LOG_WARNING, "no IP addresses to give -- OFFER abandoned");
		return -1;
	}	

	if (!add_lease(packet.chaddr, packet.yiaddr, server_config.offer_time)) {
		LOG(LOG_WARNING, "lease pool is full -- OFFER abandoned");
		return -1;
	}	

	// Modified by Mason Yu for Half Bridge
	// Asigned Lease Time 
	if ( (((server_config.ippt == 1) && (PPP_HALFBRIDGE_GET_WANIP == TRUE) && (FIRST_SEND_PUBLIC_IP == TRUE)) || 
	      ((server_config.ippt == 1) && (PPP_HALFBRIDGE_GET_WANIP == TRUE) && (RENEW_PUBLIC_IP == TRUE))) ||
	     (((server_config.ippt == 1) && (IPOA_HALFBRIDGE_GET_WANIP == TRUE) && (FIRST_SEND_PUBLIC_IP == TRUE)) || 
	      ((server_config.ippt == 1) && (IPOA_HALFBRIDGE_GET_WANIP == TRUE) && (RENEW_PUBLIC_IP == TRUE))) )  {		
		// Asigned Lease Time for Half Bridge
            	//printf("server_config.ipptlt(sendOffer) = %d\n", server_config.ipptlt); 
            	//server_config.ipptlt = 30;            	 	    
            	add_simple_option(packet.options, DHCP_LEASE_TIME, htonl(server_config.ipptlt));
            
            	// Added by Mason Yu
            	if (!add_lease(packet.chaddr, packet.yiaddr, server_config.ipptlt)) {
			LOG(LOG_WARNING, "IPPT: lease pool is full -- OFFER abandoned");
			return -1;
	    	}
	    
    	}else {
	       if ((lease_time = get_option(oldpacket, DHCP_LEASE_TIME))) {
	       		memcpy(&lease_time_align, lease_time, 4);
	       		lease_time_align = ntohl(lease_time_align);
	       		if (lease_time_align > server_config.lease) 
	       			lease_time_align = server_config.lease;
	       }          

	       /* Make sure we aren't just using the lease time from the previous offer */
	       if (lease_time_align < server_config.min_lease) 
	       		lease_time_align = server_config.lease;
	       /* ADDME: end of short circuit */		
	       add_simple_option(packet.options, DHCP_LEASE_TIME, htonl(lease_time_align));
    	}

	curr = server_config.options;
	// Modified by Mason Yu for Half Bridge 
	// The following two situations, we should (1) set 0xffffffff as submet and
	// (2) set PPPoX server IP address as Gateway.
	// (1) The user choose Half Bridge feature and DHCP server send offer first time.
	// (2) The user choose Half Bridge feature and user renew public IP.	
	if ( (((server_config.ippt == 1) && (PPP_HALFBRIDGE_GET_WANIP == TRUE) && (FIRST_SEND_PUBLIC_IP == TRUE)) || 
	      ((server_config.ippt == 1) && (PPP_HALFBRIDGE_GET_WANIP == TRUE) && (RENEW_PUBLIC_IP == TRUE))) ||
	     (((server_config.ippt == 1) && (IPOA_HALFBRIDGE_GET_WANIP == TRUE) && (FIRST_SEND_PUBLIC_IP == TRUE)) || 
	      ((server_config.ippt == 1) && (IPOA_HALFBRIDGE_GET_WANIP == TRUE) && (RENEW_PUBLIC_IP == TRUE))) )  {
		//printf("Set gw and subnet by Half Bridge(sendoffer)\n");
		while (curr) {
	       		if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)  {
		        	//(1) Set 0xffffffff as submet of the option on this offer.
		        	if (curr->data[OPT_CODE] == DHCP_SUBNET) {			        
		                  	add_simple_option(packet.options, DHCP_SUBNET, 0xffffffff );               
		               
		        	// (2) Set WAN interface address as Gateway of the option on this offer.     
		        	}else if ( curr->data[OPT_CODE] == DHCP_ROUTER ) {	 		              
		                  	add_simple_option(packet.options, DHCP_ROUTER, hisipaddr ); 
		               	
		                // Added by Mason Yu. Access internet fail.
		                // (3) Set Dns Server of the option on this offer.     
		        	}else if ( (PPP_HALFBRIDGE_GET_WANIP) && (curr->data[OPT_CODE] == DHCP_DNS_SERVER) ) {	 		              
		                  	add_simple_option(packet.options, DHCP_DNS_SERVER, dnsipaddr );
		                  	            
		        	}else {
					add_option_string(packet.options, curr->data);
		        	}    			    
	        	}		
	        	curr = curr->next;
	      	}
	      
	      	FIRST_SEND_PUBLIC_IP = FALSE;
	      	RENEW_PUBLIC_IP = FALSE;
	      
	} else {
	      while (curr) {
		//ql 20090119 START: for imagenio service, dns must be 172.26.23.3
#ifndef IMAGENIO_IPTV_SUPPORT
		if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
	      		add_option_string(packet.options, curr->data);
#else
		if ((curr->data[OPT_CODE] != DHCP_LEASE_TIME) && 
		    ((devicetype != CTC_STB) || (curr->data[OPT_CODE] != DHCP_DNS_SERVER)))
		    add_option_string(packet.options, curr->data);
#endif
		//ql 20090119 END
	      	curr = curr->next;
	      }
    	}
#if defined(CTC_DHCP_OPTION43) 
		// jim should attach option  43 to dhcp ack packet.
	add_option_string(packet.options, option43);
#endif
#if defined(CTC_DHCP_OPTION60)
		// jim should attach option  60 to dhcp ack packet.
	add_option_string(packet.options, option60);
#endif
/*star:20080926 START: add for telefornica tr069 request*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	addDhcpdOption(&packet);
#endif
/*star:20080826 END*/

#if 0
/*ping_zhang:20090319 START:replace ip range with serving pool of tr069*/
#ifndef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	/*ql 20090119 START: add options 240 for STB Discover*/
#ifdef IMAGENIO_IPTV_SUPPORT
	if (CTC_STB == devicetype)
		addStbOption(&packet);
/*ping_zhang:20090313 START:Telefonica DHCP option new request*/
	else {
		if(deviceCategory && deviceCategory->optionCode!=DHCP_END)
			addReservedOption(&packet,deviceCategory);
	}
/*ping_zhang:20090313 END*/
#endif
	/*ql 20090119 END*/
#endif
/*ping_zhang:20090319 END*/
#endif

	add_bootp_options(&packet);

	addr.s_addr = packet.yiaddr;
	LOG(LOG_INFO, "sending OFFER of %s", inet_ntoa(addr));
	return send_packet(&packet, 0);
}


int sendNAK(struct dhcpMessage *oldpacket)
{
	struct dhcpMessage packet;

	init_packet(&packet, oldpacket, DHCPNAK);
	
	DEBUG(LOG_INFO, "sending NAK");
	return send_packet(&packet, 1);
}

#ifdef IP_BASED_CLIENT_TYPE
int set_CTC_port_forwarding(int iSet, unsigned long ulIp, unsigned short usProtocol, unsigned short usPort)
{
    char szAction[4];
    char szProtocol[4];
    unsigned char aucPort[16];
    unsigned char aucDstIP[32];        

    switch (usProtocol)
    {
        case PF_UDP:
        case PF_TCP:
            snprintf(szProtocol, sizeof(szProtocol), "%s", PF_UDP == usProtocol ? "udp" : "tcp");
            break;
            
        case PF_TCP_UDP:
            if (set_CTC_port_forwarding(iSet, ulIp, PF_UDP, usPort))
            {
                if (!set_CTC_port_forwarding(iSet, ulIp, PF_TCP, usPort))
                {
                    if (iSet)
                    {
                        /* undo udp setting */
                        set_CTC_port_forwarding(!iSet, ulIp, PF_UDP, usPort);
                    }
                }
                else
                {
                    /* success */
                    return 1;
                }
            }
            else
            {
                if (!iSet)
                {
                    /* Clear */
                    set_CTC_port_forwarding(iSet, ulIp, PF_TCP, usPort);
                }
            }
            return 0;

        default:
            return 0;
    }

    snprintf(szAction, sizeof(szAction), "%s", iSet ? "-A" : "-D");
    snprintf(aucPort, sizeof(aucPort), "%u", usPort);
	snprintf(aucDstIP, sizeof(aucDstIP), "%s:%s", inet_ntoa(*((struct in_addr *)(&ulIp))), aucPort);

	if (0 == va_cmd(IPTABLES, 15, 1, "-t", "nat", szAction, "PREROUTING", "-i", "!", 
                    server_config.interface, "-p", szProtocol, "--dport",
                    aucPort, "-j", "DNAT", "--to-destination", aucDstIP))
	{
        /* Set ip filter */
        snprintf(szAction, sizeof(szAction), "%s", iSet ? "-I" : "-D");
    	if (0 != va_cmd(IPTABLES, 13, 1, szAction, "ipfilter", "-i", "!", 
                        server_config.interface, "-o", server_config.interface,                         
                        "-p", szProtocol, "--dport", aucPort, "-j", "RETURN"))
    	{
            if (iSet)
            {
                /* undo nat setting */
                va_cmd (IPTABLES, 15, 1, "-t", "nat", "-D", "PREROUTING", "-i", "!", 
                        server_config.interface, "-p", szProtocol, "--dport",
                        aucPort, "-j", "DNAT", "--to-destination", aucDstIP);
            }
    	}
        else
        {
            /* success */
            return 1;
        }
	}
    else
    {
        if (!iSet)
        {
            /* go on clear ipfilter */
            va_cmd (IPTABLES, 13, 1, szAction, "ipfilter", "-i", "!", 
                    server_config.interface, "-o", server_config.interface,                         
                    "-p", szProtocol, "--dport", aucPort, "-j", "RETURN");
        }
    }

    return 0;
}

void CTC_lease_expired(struct dhcpOfferedAddr *pstLease)
{
    struct dhcp_ctc_client_info *pstClientInfo;

    pstClientInfo = (struct dhcp_ctc_client_info *)(pstLease->stClientInfo.pvDHCPClientData);
    if ((0 != pstClientInfo->stPortForwarding.usPort)
        && (pstClientInfo->stPortForwarding.iSet))
    {
        set_CTC_port_forwarding(0, pstLease->yiaddr, 
                                 pstClientInfo->stPortForwarding.usProtocol, 
                                 pstClientInfo->stPortForwarding.usPort);        
    }

    free(pstClientInfo);

    return;
}
#endif

int sendACK(struct dhcpMessage *oldpacket, u_int32_t yiaddr)
{
	struct dhcpMessage packet;
	struct option_set *curr;
	unsigned char *lease_time;
	u_int32_t lease_time_align = server_config.lease;
	struct in_addr addr;
    	// Added by Mason Yu for Half Bridge
    	// Added by Mason Yu. Access internet fail.
	unsigned long myipaddr, hisipaddr, dnsipaddr;  
	FILE *fp;
	struct dhcpOfferedAddr *pstLease;
	//ql 20090119 START: check if it is STB device
#ifdef IMAGENIO_IPTV_SUPPORT
	enum DeviceType devicetype;
#elif defined(IP_BASED_CLIENT_TYPE) //IMAGENIO_IPTV_SUPPORT
	enum DeviceType devicetype;
	struct client_category_t *deviceCategory;
	unsigned char *classVendor;
	struct dhcp_ctc_client_info stClientInfo;
/*ping_zhang:20090316 START:Fix the DHCP_VENDOR string bugs*/
	unsigned char len = 0;
	unsigned char classVendorStr[256] = {0};
/*ping_zhang:20090316 END*/
#endif
	//ql 20090119 END
	myipaddr = 0;
    
	// Added by Mason Yu for Half Bridge	
    if ( server_config.ippt )  {    	 
    	  
    	 fp = fopen ("/tmp/PPPHalfBridge", "r");
         if (fp) {
	        fread(&myipaddr, 4, 1, fp);	         
	        fread(&hisipaddr, 4, 1, fp);
	        // Added by Mason Yu. Access internet fail.
	 	fread(&dnsipaddr, 4, 1, fp);	         
	        
	        fclose(fp);
         } else {
         	PPP_HALFBRIDGE_GET_WANIP = FALSE;
         } 
         
         
         fp = fopen ("/tmp/IPoAHalfBridge", "r");
         if (fp) {
	        fread(&myipaddr, 4, 1, fp);	         
	        fread(&hisipaddr, 4, 1, fp);
	       	         
	        fclose(fp);
         } else {
         	 IPOA_HALFBRIDGE_GET_WANIP = FALSE;
         }
          
         
         if ( yiaddr == myipaddr )  {    	     
	         RENEW_PUBLIC_IP = TRUE;
	// Added by Mason Yu. Access internet fail.
        } else if ( !((ntohl(server_config.start) <= yiaddr) && (yiaddr<= ntohl(server_config.end))) ) {
        	printf("sendACK: This ip is not on DHCP Pool for IP Passthrough\n");         	
        	sendNAK(oldpacket);
         	return -1;
	}
	#if 0
	// Kaohj
	// ippt and bridged IP not yet leased
	else if (myipaddr != 0 && !(pstLease = find_lease_by_yiaddr(myipaddr))) {
		// reject it and let this client be able to discover the bridged IP
        	sendNAK(oldpacket);
         	return -1;
	}
	#endif
	goto init_ack;
	           
    }	
	
	// Added by Mason Yu
	if ( !((ntohl(server_config.start) <= yiaddr) && (yiaddr<= ntohl(server_config.end))) ) {
		printf("sendACK: This ip is not on DHCP Pool\n");         	
        	sendNAK(oldpacket);
         	return -1;
	}
		
init_ack:
	//ql 20090119 START: check if it is STB device
#ifdef IMAGENIO_IPTV_SUPPORT
	devicetype = getDeviceType();
#elif defined(IP_BASED_CLIENT_TYPE) //&& defined(IMAGENIO_IPTV_SUPPORT)
	if(!(classVendor=get_option(oldpacket, DHCP_VENDOR))) {
		//default : PC clients....
		devicetype = CTC_Computer;
		deviceCategory=NULL;
	}
	else
	{
/*ping_zhang:20090316 START:Fix the DHCP_VENDOR string bugs*/
		len=*(unsigned char*)(classVendor-OPT_LEN);
		memcpy(classVendorStr,classVendor,len);
		classVendorStr[len]=0;
/*ping_zhang:20090316 END*/
		memset(&stClientInfo, 0, sizeof(struct dhcp_ctc_client_info));
/*ping_zhang:20090316 START:Fix the DHCP_VENDOR string bugs*/
		//parse_CTC_Vendor_Class(oldpacket, classVendor, &stClientInfo);
		parse_CTC_Vendor_Class(oldpacket, classVendorStr, &stClientInfo);
/*ping_zhang:20090316 END*/
		devicetype = (enum DeviceType)(stClientInfo.category);
		deviceCategory = stClientInfo.iCategory;
	}
#endif
	//ql 20090119 END
	
	init_packet(&packet, oldpacket, DHCPACK);
	packet.yiaddr = yiaddr;
	
	// Modified by Mason Yu for Half Bridge 
	// Asigned Lease Time	
	if ( (((server_config.ippt == 1) && (PPP_HALFBRIDGE_GET_WANIP == TRUE) && (FIRST_SEND_ACK == TRUE)) ||
	      ((server_config.ippt == 1) && (PPP_HALFBRIDGE_GET_WANIP == TRUE) && (RENEW_PUBLIC_IP == TRUE))) ||
	     (((server_config.ippt == 1) && (IPOA_HALFBRIDGE_GET_WANIP == TRUE) && (FIRST_SEND_ACK == TRUE)) ||
	      ((server_config.ippt == 1) && (IPOA_HALFBRIDGE_GET_WANIP == TRUE) && (RENEW_PUBLIC_IP == TRUE))) )  {
            // Asigned Lease Time for Half Bridge
            //printf("server_config.ipptlt(sendAck) = %d\n", server_config.ipptlt);
            add_simple_option(packet.options, DHCP_LEASE_TIME, htonl(server_config.ipptlt));
            lease_time_align = server_config.ipptlt;
    }else {
	        if ((lease_time = get_option(oldpacket, DHCP_LEASE_TIME))) {
	       	     memcpy(&lease_time_align, lease_time, 4);
	       	     lease_time_align = ntohl(lease_time_align);
	       	     if (lease_time_align > server_config.lease) 
	       		     lease_time_align = server_config.lease;
	       	     else if (lease_time_align < server_config.min_lease) 
	       		     lease_time_align = server_config.lease;
	       }
	
	       add_simple_option(packet.options, DHCP_LEASE_TIME, htonl(lease_time_align));
	}
	
	curr = server_config.options;
	// Modified by Mason Yu for Half Bridge 	
	if ( (((server_config.ippt == 1) && (PPP_HALFBRIDGE_GET_WANIP == TRUE) && (FIRST_SEND_ACK == TRUE)) ||
	      ((server_config.ippt == 1) && (PPP_HALFBRIDGE_GET_WANIP == TRUE) && (RENEW_PUBLIC_IP == TRUE))) ||
	     (((server_config.ippt == 1) && (IPOA_HALFBRIDGE_GET_WANIP == TRUE) && (FIRST_SEND_ACK == TRUE)) ||
	      ((server_config.ippt == 1) && (IPOA_HALFBRIDGE_GET_WANIP == TRUE) && (RENEW_PUBLIC_IP == TRUE))) )  {
		  
		  while (curr) {
	        if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)  {
		        //(1) Set 0xffffffff as submet of the option on this offer.
		        if (curr->data[OPT_CODE] == DHCP_SUBNET) {		       
		                  add_simple_option(packet.options, DHCP_SUBNET, 0xffffffff );			                   
		                  
		        // (2) Set WAN interface address as Gateway of the option on this offer.     
		        } else if ( curr->data[OPT_CODE] == DHCP_ROUTER ) {			      
		                   add_simple_option(packet.options, DHCP_ROUTER, hisipaddr );
		                   
		        // Added by Mason Yu. Access internet fail.
		        // (3) Set Dns Server of the option on this offer.     
		        }else if ( (PPP_HALFBRIDGE_GET_WANIP) && (curr->data[OPT_CODE] == DHCP_DNS_SERVER) ) {	 		              
		                  add_simple_option(packet.options, DHCP_DNS_SERVER, dnsipaddr );
		                 	               
		        }else {
			               add_option_string(packet.options, curr->data);
		        }    			    
	        }		
	        curr = curr->next;
	      }
	      
	      FIRST_SEND_ACK = FALSE;
	      RENEW_PUBLIC_IP = FALSE;
	       
	} else  {
	      while (curr) {
		//ql 20090119 START: for imagenio service, dns must be 172.26.23.3
#ifndef IMAGENIO_IPTV_SUPPORT
		if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
	      		add_option_string(packet.options, curr->data);
#else
		if ((curr->data[OPT_CODE] != DHCP_LEASE_TIME) && 
		    ((devicetype != CTC_STB) || (curr->data[OPT_CODE] != DHCP_DNS_SERVER)))
		    add_option_string(packet.options, curr->data);
#endif
		//ql 20090119 END
	      	curr = curr->next;
	      }
	}
#if defined(CTC_DHCP_OPTION43) 
		// jim should attach option  43 to dhcp ack packet.
	add_option_string(packet.options, option43);
#endif
#if defined(CTC_DHCP_OPTION60)
		// jim should attach option  60 to dhcp ack packet.
	add_option_string(packet.options, option60);
#endif
/*star:20080926 START: add for telefornica tr069 request*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	addDhcpdOption(&packet);
#endif
/*star:20080826 END*/

#if 0
/*ping_zhang:20090319 START:replace ip range with serving pool of tr069*/
#ifndef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	/*ql 20090119 START: add options 240 for STB Discover*/
#ifdef IMAGENIO_IPTV_SUPPORT
	if (CTC_STB == devicetype)
		addStbOption(&packet);
/*ping_zhang:20090313 START:Telefonica DHCP option new request*/
	else {
		if(deviceCategory && deviceCategory->optionCode!=DHCP_END)
			addReservedOption(&packet,deviceCategory);
	}
/*ping_zhang:20090313 END*/
#endif
	/*ql 20090119 END*/
#endif
/*ping_zhang:20090319 END*/
#endif

	add_bootp_options(&packet);

	addr.s_addr = packet.yiaddr;
	LOG(LOG_INFO, "sending ACK to %s", inet_ntoa(addr));

	if (send_packet(&packet, 0) < 0) 
		return -1;

	pstLease = add_lease(packet.chaddr, packet.yiaddr, lease_time_align);

    if (NULL != pstLease)
    {
        pstLease->stClientInfo.ulDevice = FP_Computer;
    
#ifdef IP_BASED_CLIENT_TYPE
        struct dhcp_ctc_client_info *pstClientInfo;
		unsigned char *classVendor;
/*ping_zhang:20090316 START:Fix the DHCP_VENDOR string bugs*/
		unsigned char len = 0;
		unsigned char classVendorStr[256] = {0};
/*ping_zhang:20090316 END*/
        
		if (NULL != (classVendor=get_option(oldpacket, DHCP_VENDOR)))
		{
/*ping_zhang:20090316 START:Fix the DHCP_VENDOR string bugs*/
		len=*(unsigned char*)(classVendor-OPT_LEN);
		memcpy(classVendorStr,classVendor,len);
		classVendorStr[len]=0;
/*ping_zhang:20090316 END*/
            pstClientInfo = malloc(sizeof(struct dhcp_ctc_client_info));
            if (NULL != pstClientInfo)
            {
                memset(pstClientInfo, 0, sizeof(struct dhcp_ctc_client_info));
/*ping_zhang:20090316 START:Fix the DHCP_VENDOR string bugs*/
                //parse_CTC_Vendor_Class(oldpacket, classVendor, pstClientInfo);
                parse_CTC_Vendor_Class(oldpacket, classVendorStr, pstClientInfo);
/*ping_zhang:20090316 END*/
                
                pstLease->stClientInfo.pfLeaseExpired = CTC_lease_expired;
                pstLease->stClientInfo.pvDHCPClientData = (void *)pstClientInfo;

                if ((0 != pstClientInfo->stPortForwarding.usPort)
                    && (!pstClientInfo->stPortForwarding.iSet))
                {
                    if (set_CTC_port_forwarding(1, pstLease->yiaddr, 
                                                 pstClientInfo->stPortForwarding.usProtocol, 
                                                 pstClientInfo->stPortForwarding.usPort))
                    {
                        pstClientInfo->stPortForwarding.iSet = 1;
                    }
                }

		switch (pstClientInfo->category)
                {
                    case CTC_STB:
                        pstLease->stClientInfo.ulDevice = FP_STB;
                        break;

                    case CTC_PHONE:
                        pstLease->stClientInfo.ulDevice = FP_MOBILE;
                        break;

                    default:
                        break;
                }
            }
		}

#ifdef CONFIG_CTC_E8_CLIENT_LIMIT
        update_client_limit(pstLease);
#endif
#endif

        /* update_force_portal(); */
        update_force_portal(pstLease);
    }

#ifdef _CWMP_TR111_
	handle_tr111( oldpacket, yiaddr );
#endif

#if 0
#ifdef CONFIG_CTC_E8_CLIENT_LIMIT
	handle_clients_limit(oldpacket,yiaddr);
#endif
#endif
	return 0;
}


int send_inform(struct dhcpMessage *oldpacket)
{
	struct dhcpMessage packet;
	struct option_set *curr;

	init_packet(&packet, oldpacket, DHCPACK);
	
	curr = server_config.options;
	while (curr) {
		if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
			add_option_string(packet.options, curr->data);
		curr = curr->next;
	}
#if defined(CTC_DHCP_OPTION43) 
		// jim should attach option  43 to dhcp ack packet.
	add_option_string(packet.options, option43);
#endif
#if defined(CTC_DHCP_OPTION60)
		// jim should attach option  60 to dhcp ack packet.
	add_option_string(packet.options, option60);
#endif
	add_bootp_options(&packet);

#ifdef _CWMP_TR111_
	handle_tr111( oldpacket, oldpacket->yiaddr );
#endif

	return send_packet(&packet, 0);
}

/*ql: 20090119 START: add for imagenio service*/
#ifdef IMAGENIO_IPTV_SUPPORT
void addStbOption(struct dhcpMessage *packet)
{
	unsigned char option[120];

	option[OPT_CODE] = DHCP_DNS_SERVER;
	option[OPT_LEN] = 4;
	memcpy(option+OPT_DATA, &server_config.stbdns1, 4);
	if (server_config.stbdns2) {
		memcpy(option+option[OPT_LEN]+2, &server_config.stbdns2, 4);
		option[OPT_LEN] = 8;
	}
	if (!add_option_string(packet->options, option))
		printf("add option DNS fail\n");

	option[OPT_CODE] = DHCP_OPT_240;
	sprintf(option+OPT_DATA, ":::::%s:%d", inet_ntoa(*(struct in_addr *)&server_config.opchaddr), server_config.opchport);
	option[OPT_LEN] = strlen(option+OPT_DATA);
	if (!add_option_string(packet->options, option))
		printf("add option 240 fail\n");
}

#if 0
/*ping_zhang:20090313 START:Telefonica DHCP option new request*/
addReservedOption(struct dhcpMessage *packet,struct client_category_t *deviceCategory)
{
	unsigned char option[120];

	option[OPT_CODE] = deviceCategory->optionCode;
	strcpy(option+OPT_DATA, deviceCategory->optionStr);
	option[OPT_LEN] = strlen(option+OPT_DATA);
	if (!add_option_string(packet->options, option))
		printf("add option %d fail\n",deviceCategory->optionCode);
}
/*ping_zhang:20090313 END*/
#endif
#endif
/*ql 20090119 END*/

/*star:20080926 START: add for telefornica tr069 request*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
/*ping_zhang:20090319 START:replace ip range with serving pool of tr069*/
unsigned char getDeviceType()
{
	DHCPS_SERVING_POOL_T entry;
	unsigned int i,num;
	unsigned char deviceType = CTC_UNKNOWN;
		
	num = mib_chain_total( MIB_DHCPS_SERVING_POOL_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_DHCPS_SERVING_POOL_TBL, i, (void*)&entry ) )
			continue;

		if( entry.InstanceNum==server_config.cwmpinstnum) 
		{
			deviceType =  entry.deviceType;
			break;
		}
	}	
	return deviceType;
}
/*ping_zhang:20090319 END*/
	
void addDhcpdOption(struct dhcpMessage *packet)
{
	MIB_CE_DHCP_OPTION_T entry;
	int i, entrynum;
	unsigned char option[DHCP_OPT_VAL_LEN+2];

/*ping_zhang:20090319 START:replace ip range with serving pool of tr069*/
	//check if is STB device type
#ifdef IMAGENIO_IPTV_SUPPORT
	if(getDeviceType() == CTC_STB)
		addStbOption(packet);
#endif //IMAGENIO_IPTV_SUPPORT
/*ping_zhang:20090319 END*/

	entrynum = mib_chain_total(MIB_DHCP_SERVER_OPTION_TBL);
	for (i=0; i<entrynum; i++)
	{
		if (!mib_chain_get(MIB_DHCP_SERVER_OPTION_TBL, i, (void *)&entry) || !entry.enable)
			continue;
//#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_			
		if (entry.dhcpConSPInstNum!=server_config.cwmpinstnum)
			continue;
//#endif
		option[OPT_CODE] = (unsigned char)entry.tag;
		option[OPT_LEN] = entry.len;
		memcpy(option+OPT_DATA, entry.value, entry.len);
		
		add_option_string(packet->options, option);
	}
}

static int _is_hex(char c)
{
    return (((c >= '0') && (c <= '9')) ||
            ((c >= 'A') && (c <= 'F')) ||
            ((c >= 'a') && (c <= 'f')));
}

static int string_to_hex(char *string, unsigned char *key, int len)
{
	char tmpBuf[4];
	int idx, ii=0;
	for (idx=0; idx<len; idx+=2) {
		tmpBuf[0] = string[idx];
		tmpBuf[1] = string[idx+1];
		tmpBuf[2] = 0;
		if ( !_is_hex(tmpBuf[0]) || !_is_hex(tmpBuf[1]))
			return 0;

		key[ii++] = (unsigned char) strtol(tmpBuf, (char**)NULL, 16);
	}
	return 1;
}
void find_match_serving_pool(struct dhcpMessage *dhcppacket){

	struct server_config_t* p_servingpool_tmp=p_serverpool_config->next;
	unsigned char vendorclass[256]={0},clientid[256]={0},userclass[256]={0};
	unsigned char *tmpstr;
	unsigned char len=0;	 
	unsigned char vendormachflag=0,clientidmachflag=0,userclassmatchflag=0,chaddrmatchflag=0, srcIntfflag=0/*base on sourceinterface*/; 
	unsigned char chaddr[6],chaddrmask[6],chaddrmaskresult[6];
	int i;
	unsigned char srcIntf; // Base on sourceinterface

	serverpool = 1;
	
	tmpstr=get_option(dhcppacket, DHCP_VENDOR);
	if(tmpstr!=NULL){
		len=*(unsigned char*)(tmpstr-OPT_LEN);
		memcpy(vendorclass,tmpstr,len);
		vendorclass[len]=0;
	}
	tmpstr=get_option(dhcppacket, DHCP_CLIENT_ID);
	if(tmpstr!=NULL){
		len=*(unsigned char*)(tmpstr-OPT_LEN);
		memcpy(clientid,tmpstr,len);
		clientid[len]=0;
	}
	tmpstr=get_option(dhcppacket, DHCP_USER_ID);
	if(tmpstr!=NULL){
		len=*(unsigned char*)(tmpstr-OPT_LEN);
		memcpy(userclass,tmpstr,len);
		userclass[len]=0;
	}
	// Mason Yu. Base on sourceinterface
	tmpstr=get_option(dhcppacket, DHCP_SRC_INTF);
	if(tmpstr!=NULL){
		srcIntf = *tmpstr;		
	}	
	
	while(p_servingpool_tmp!=NULL){
		
		vendormachflag=0;
		clientidmachflag=0;
		userclassmatchflag=0;
		chaddrmatchflag=0;
		srcIntfflag=0; // Base on sourceinterface
		
		if(p_servingpool_tmp->vendorclass==NULL
			&& p_servingpool_tmp->sourceinterface==NULL // Base on sourceinterface
			&& p_servingpool_tmp->clientid==NULL
			&& p_servingpool_tmp->userclass==NULL
			&& p_servingpool_tmp->chaddr==NULL){
			p_servingpool_tmp=p_servingpool_tmp->next;
			continue;
		}		
		
		if(p_servingpool_tmp->vendorclass){
			if(p_servingpool_tmp->vendorclassflag==0){
				int classlen=0,tmplen=0;
				char *classtmp=0;
				if(!strcmp(p_servingpool_tmp->vendorclassmode,"Exact")){
					if(!strcmp(p_servingpool_tmp->vendorclass,vendorclass))
						vendormachflag=1;
				}else if(!strcmp(p_servingpool_tmp->vendorclassmode,"Prefix")){
					classtmp=strstr(vendorclass,p_servingpool_tmp->vendorclass);
					if(classtmp==vendorclass)
						vendormachflag=1;
				}else if(!strcmp(p_servingpool_tmp->vendorclassmode,"Suffix")){
					classlen=strlen(p_servingpool_tmp->vendorclass);
					classtmp=strstr(vendorclass,p_servingpool_tmp->vendorclass);
					tmplen=strlen(classtmp);
					if(tmplen==classlen)
						vendormachflag=1;
				}else{ //Substring
					classtmp=strstr(vendorclass,p_servingpool_tmp->vendorclass);
					if(classtmp!=NULL)
						vendormachflag=1;
				}
			}
			else{
				if(strcmp(p_servingpool_tmp->vendorclass,vendorclass))
					vendormachflag=1;
			}
		}else		
			vendormachflag=1;		
		
		if(p_servingpool_tmp->clientid){
			if(p_servingpool_tmp->clientidflag==0){
				if(!strcmp(p_servingpool_tmp->clientid,clientid))
					clientidmachflag=1;
			}
			else{
				if(strcmp(p_servingpool_tmp->clientid,clientid))
					clientidmachflag=1;
			}
		}else
			clientidmachflag=1;

		if(p_servingpool_tmp->userclass){
			if(p_servingpool_tmp->userclassflag==0){
				if(!strcmp(p_servingpool_tmp->userclass,userclass))
					userclassmatchflag=1;
			}
			else{
				if(strcmp(p_servingpool_tmp->userclass,userclass))
					userclassmatchflag=1;
			}
		}else
			userclassmatchflag=1;

		if(p_servingpool_tmp->chaddr){
			if(!p_servingpool_tmp->chaddrmask)
				memset(chaddrmask,0xff,6);
			else
				string_to_hex(p_servingpool_tmp->chaddrmask,chaddrmask,12);
			string_to_hex(p_servingpool_tmp->chaddr,chaddr,12);
			for(i=0;i<6;i++){	
				chaddrmaskresult[i]=(dhcppacket->chaddr[i])&chaddrmask[i];
			}
			if(p_servingpool_tmp->chaddrflag==0){
				if(!memcmp(chaddr,chaddrmaskresult,6))
					chaddrmatchflag=1;
			}
			else{
				if(memcmp(chaddr,chaddrmaskresult,6))
					chaddrmatchflag=1;
			}
		}else
			chaddrmatchflag=1;
		
		// Mason Yu. base on sourceinterface		
		if(p_servingpool_tmp->sourceinterface !=0 ){			
			if ( ( srcIntf!=0 && (p_servingpool_tmp->sourceinterface&srcIntf)==srcIntf) ) {	
				//printf("Find match source Interface. p_servingpool_tmp->sourceinterface=%d, srcIntf=%d\n", p_servingpool_tmp->sourceinterface, srcIntf);		
				srcIntfflag=1;
			}
		}else
			srcIntfflag=1;		
			
		if(vendormachflag==1 && clientidmachflag==1 && userclassmatchflag==1 && chaddrmatchflag==1 && srcIntfflag==1)
			break;

		p_servingpool_tmp=p_servingpool_tmp->next;
	}

	if(p_servingpool_tmp==NULL) {
		p_servingpool_tmp=p_serverpool_config;
		serverpool = 0;
	}

	DEBUG_CHN_TEL("\ndhcp server find servingpool:%s\n",p_servingpool_tmp->poolname);
	memcpy(&server_config,p_servingpool_tmp,sizeof(struct server_config_t));

}
/*
void dumpservingpool()
{
	struct server_config_t* p_servingpool_tmp=p_serverpool_config;

	while(p_servingpool_tmp!=NULL){
		printf("\ntmp=%x",p_servingpool_tmp);
		printf("\npoolname=%s",p_servingpool_tmp->poolname);
		printf("\nstart=%x",p_servingpool_tmp->start);
		printf("\nend=%x",p_servingpool_tmp->end);
		printf("\npoolorder=%d",p_servingpool_tmp->poolorder);
		printf("\nvendorclass=%s",p_servingpool_tmp->vendorclass);
		printf("\nvendorclassflag=%d",p_servingpool_tmp->vendorclassflag);
		printf("\nvendorclassmode=%s",p_servingpool_tmp->vendorclassmode);
		printf("\nclientid=%s",p_servingpool_tmp->clientid);
		printf("\nclientidflag=%d",p_servingpool_tmp->clientidflag);
		printf("\nuserclass=%s",p_servingpool_tmp->userclass);
		printf("\nuserclassflag=%d",p_servingpool_tmp->userclassflag);

		p_servingpool_tmp=p_servingpool_tmp->next;
	}

}
*/
#endif
/*star:20080926 END*/


