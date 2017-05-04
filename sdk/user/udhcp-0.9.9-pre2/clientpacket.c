/* clientpacket.c
 *
 * Packet generation and dispatching functions for the DHCP client.
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

#include <string.h>
#include <sys/socket.h>
#include <features.h>
#if __GLIBC__ >=2 && __GLIBC_MINOR >= 1
#include <netpacket/packet.h>
#include <net/ethernet.h>
#else
#include <asm/types.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#endif
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "dhcpd.h"
#include "packet.h"
#include "options.h"
#include "dhcpc.h"
#include "debug.h"
/*ql:20080925 START: get option and request from mibtbl*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
#include <rtk/mib.h>
#include <rtk/sysconfig.h>
#include <rtk/utility.h>
#endif
/*ql:20080925 END*/

/* Create a random xid */
unsigned long random_xid(void)
{
	static int initialized;
	if (!initialized) {
		int fd;
		unsigned long seed;

		fd = open("/dev/urandom", 0);
		if (fd < 0 || read(fd, &seed, sizeof(seed)) < 0) {
			LOG(LOG_WARNING, "Could not load seed from /dev/urandom: %s",
				strerror(errno));
			seed = time(0);
		}
		if (fd >= 0) close(fd);
		srand(seed);
		initialized++;
	}
	return rand();
}


/* initialize a packet with the proper defaults */
static void init_packet(struct dhcpMessage *packet, char type)
{
	/*ql: 20080925 START: move below option to mibtbl*/
#ifndef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	struct vendor  {
		char vendor, length;
		char str[sizeof("udhcp "VERSION)];
	} vendor_id = { DHCP_VENDOR,  sizeof("udhcp "VERSION) - 1, "udhcp "VERSION};
#endif
	/*ql: 20080925 END*/

	init_header(packet, type);
	memcpy(packet->chaddr, client_config.arp, 6);
	/*ql: 20080925 START: move below option to mibtbl*/
#ifndef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	add_option_string(packet->options, client_config.clientid);
	add_option_string(packet->options, (unsigned char *) &vendor_id);
#endif
	/*ql: 20080925 END*/
	if (client_config.hostname) add_option_string(packet->options, client_config.hostname);
}

#ifndef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
/* Add a paramater request list for stubborn DHCP servers. Pull the data
 * from the struct in options.c. Don't do bounds checking here because it
 * goes towards the head of the packet. */
static void add_requests(struct dhcpMessage *packet)
{
	int end = end_option(packet->options);
	int i, len = 0;

	packet->options[end + OPT_CODE] = DHCP_PARAM_REQ;
	for (i = 0; options[i].code; i++)
		if (options[i].flags & OPTION_REQ)
			packet->options[end + OPT_DATA + len++] = options[i].code;
	//wpeng 20120411 added for static router
#ifdef CONFIG_USER_DHCP_OPT_33_249
	packet->options[end + OPT_DATA + len++] = DHCP_STATIC_RT;
	packet->options[end + OPT_DATA + len++] = DHCP_CLASSLESS_RT;
	packet->options[end + OPT_DATA + len++] = DHCP_OPT_249;
#endif
	//end of add
	packet->options[end + OPT_LEN] = len;
	packet->options[end + OPT_DATA + len] = DHCP_END;

}
#endif

/*ql:20080925 START: get option and request from mibtbl*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
enum e_dhcpcTrack
{
	DHCPC_DISCOVER,
	DHCPC_REQUEST,
	DHCPC_RENEW,
	DHCPC_RELEASE
};

/*ql:20080925 START: init MIB_DHCP_CLIENT_OPTION_TBL here*/
typedef struct initial_option {
	unsigned int order;
	unsigned int tag;
	unsigned char len;//ql add: recored the len of value
	unsigned char value[DHCP_OPT_VAL_LEN];
	unsigned char usedFor;
	unsigned int dhcpOptInstNum;
} __PACK__ INITIAL_OPTION_T;
INITIAL_OPTION_T dhcpc_option_table[] = {
	//SendDHCPOption
	{0, DHCP_MESSAGE_TYPE, 1, {0x03, 0}, eUsedFor_DHCPClient_Sent,1},
	{0, DHCP_CLIENT_ID, 0, "\0", eUsedFor_DHCPClient_Sent,2},
#ifdef _CONFIG_DHCPC_OPTION43_ACSURL_
	{0, DHCP_VENDOR, sizeof("dslforum.org"), "dslforum.org", eUsedFor_DHCPClient_Sent,3},
#else
	{0, DHCP_VENDOR, sizeof("udhcp 0.9.9-pre"), "udhcp 0.9.9-pre", eUsedFor_DHCPClient_Sent,3},
#endif	
	{0, DHCP_REQUESTED_IP, 0, "\0", eUsedFor_DHCPClient_Sent,4},
	{0, DHCP_SERVER_ID, 0, "\0", eUsedFor_DHCPClient_Sent,5},
	//ReqDHCPOption
	{1, DHCP_SUBNET, 0, "\0", eUsedFor_DHCPClient_Req,1},
	{2, DHCP_ROUTER, 0, "\0", eUsedFor_DHCPClient_Req,2},
	{3, DHCP_DNS_SERVER, 0, "\0", eUsedFor_DHCPClient_Req,3},
	{4, DHCP_HOST_NAME, 0, "\0", eUsedFor_DHCPClient_Req,4},
	{5, DHCP_DOMAIN_NAME, 0, "\0", eUsedFor_DHCPClient_Req,5},
	{6, DHCP_BROADCAST, 0, "\0", eUsedFor_DHCPClient_Req,6}
	//wpeng 20120411 added for required option
#ifdef CONFIG_USER_DHCP_OPT_33_249
	,{7, DHCP_STATIC_RT, 0, "\0", eUsedFor_DHCPClient_Req,7}
	,{8, DHCP_CLASSLESS_RT, 0, "\0", eUsedFor_DHCPClient_Req,8}
	,{9, DHCP_OPT_249, 0, "\0", eUsedFor_DHCPClient_Req,9}
#endif
#ifdef RECEIVE_DHCP_OPTION_125
	,{10, DHCP_VI_VENSPEC, 0, "\0", eUsedFor_DHCPClient_Req,10}
#endif
#ifdef _CONFIG_DHCPC_OPTION43_ACSURL_
	,{11, DHCP_VENDOR_SPECIFIC_INFO, 0, "\0", eUsedFor_DHCPClient_Req,11}
#endif
	,{12, DHCP_SIP_SERVER, 0, "\0", eUsedFor_DHCPClient_Req,12}
	,{13, DHCP_NTP_SERVER, 0, "\0", eUsedFor_DHCPClient_Req,13}
	,{14, DHCP_TFTP_SERVER, 0, "\0", eUsedFor_DHCPClient_Req,14}
	,{15, DHCP_POSIX_TZ_STRING, 0, "\0", eUsedFor_DHCPClient_Req,15}
	,{16, DHCP_SOURCE_ROUTE, 0, "\0", eUsedFor_DHCPClient_Req,16}
	,{17, DHCP_BOOT_FILENAME, 0, "\0", eUsedFor_DHCPClient_Req,17}
	//end of add
};
/*ql:20080925 END*/


void addDhcpcOption(struct dhcpMessage *packet, int state)
{
	MIB_CE_DHCP_OPTION_T entry;
	int i, entrynum;
	unsigned char option[DHCP_OPT_VAL_LEN+2];

	entrynum = mib_chain_total(MIB_DHCP_CLIENT_OPTION_TBL);
	for (i=0; i<entrynum; i++)
	{
		if (!mib_chain_get(MIB_DHCP_CLIENT_OPTION_TBL, i, (void *)&entry) || !entry.enable)
			continue;

		if (entry.ifIndex != wan_ifIndex)
			continue;

		if (entry.usedFor != eUsedFor_DHCPClient_Sent)
			continue;

		if ((state == DHCPC_DISCOVER) || (state == DHCPC_RENEW)) {
			if ((entry.tag == DHCP_REQUESTED_IP) || (entry.tag == DHCP_SERVER_ID))
				continue;
		}

#ifdef _PRMT_X_CT_COM_DHCP_
		if(entry.tag == DHCP_VENDOR)
		{
			int total_len=0;
			total_len = appendOption60(entry.value);
			//printf("total_len=%d\n", total_len);

			/* All dhcp option 60 entry is disabled. */
			if(total_len == 2)
				continue;

			entry.len = total_len;

			mib_chain_update(MIB_DHCP_CLIENT_OPTION_TBL, (void *)&entry, i);
		}
#endif
		option[OPT_CODE] = (unsigned char)entry.tag;
		option[OPT_LEN] = entry.len;
		memcpy(option+OPT_DATA, entry.value, entry.len);

		add_option_string(packet->options, option);
	}
}

void addDhcpcRequest(struct dhcpMessage *packet)
{
	MIB_CE_DHCP_OPTION_T entry;
	int i, entrynum;
	unsigned int order=1;
	int len = 0;
	int end = end_option(packet->options);

	packet->options[end + OPT_CODE] = DHCP_PARAM_REQ;
	//for (i = 0; options[i].code; i++)
	//	if (options[i].flags & OPTION_REQ)
	//		packet->options[end + OPT_DATA + len++] = options[i].code;

	entrynum = mib_chain_total(MIB_DHCP_CLIENT_OPTION_TBL);	
	while (1)
	{
		for (i=0; i<entrynum; i++)
		{
			if (!mib_chain_get(MIB_DHCP_CLIENT_OPTION_TBL, i, (void *)&entry) || !entry.enable)
				continue;

			if (entry.ifIndex != wan_ifIndex)
				continue;

			if (entry.usedFor != eUsedFor_DHCPClient_Req)
				continue;

			if (entry.order == order)//conform to the order of tr-069 configuration.
			{
				packet->options[end + OPT_DATA + len++] = (unsigned char)entry.tag;

				order++;
				break;
			}
		}

		if (i >= entrynum)
			break;
	}

	packet->options[end + OPT_LEN] = len;
	packet->options[end + OPT_DATA + len] = DHCP_END;
}

void initialDhcpcOptionTbl(void)
{
	MIB_CE_DHCP_OPTION_T entry, init_entry;
	int i, j, entrynum;
	int optNum;
	int needInit=1, needDelete=1, needUpdate=0;
	int count=0;	
	int maxorder;
	
	entrynum = mib_chain_total(MIB_DHCP_CLIENT_OPTION_TBL);
	optNum = sizeof(dhcpc_option_table)/sizeof(INITIAL_OPTION_T);		
	printf("%s: existing rule num:%d\n", __FUNCTION__, entrynum);
	
	// Add new entry into MIB
	for (i=0; i<optNum; i++)
	{	
		needInit = 1;
		for (j=0; j<entrynum; j++)
		{
			if (!mib_chain_get(MIB_DHCP_CLIENT_OPTION_TBL, j, (void *)&entry))
				continue;
	
			if (entry.ifIndex == wan_ifIndex && entry.tag == dhcpc_option_table[i].tag) {
				needInit = 0;
				break;
			}
		}

		if (needInit)
		{
			// find max order in MIB_DHCP_CLIENT_OPTION_TBL
			maxorder=findMaxDHCPReqOptionOrder(wan_ifIndex);
			memset(&init_entry, 0, sizeof(MIB_CE_DHCP_OPTION_T));
			init_entry.ifIndex = wan_ifIndex;
			init_entry.enable = 1;
			//init_entry.order = dhcpc_option_table[i].order;
			init_entry.order = maxorder+1;
			init_entry.tag = dhcpc_option_table[i].tag;
			init_entry.len = dhcpc_option_table[i].len;
			if (init_entry.len)
				memcpy(init_entry.value, dhcpc_option_table[i].value, init_entry.len);
	
			init_entry.usedFor = dhcpc_option_table[i].usedFor;
			init_entry.dhcpOptInstNum = dhcpc_option_table[i].dhcpOptInstNum;
	
			mib_chain_add(MIB_DHCP_CLIENT_OPTION_TBL, (void *)&init_entry);			
			printf("%s: add new rule to interface 0x%x with %d option\n", __FUNCTION__, wan_ifIndex, dhcpc_option_table[i].tag);
			count++;
			needUpdate = 1;
		}
	}
	printf("%s: add %d rule to interface 0x%x\n", __FUNCTION__, count, wan_ifIndex);
	
	// delete old entry from MIB
	entrynum = mib_chain_total(MIB_DHCP_CLIENT_OPTION_TBL);
	for( i=0; i<entrynum;i++ )
	{
		if (!mib_chain_get(MIB_DHCP_CLIENT_OPTION_TBL, i, (void *)&entry))
				continue;
				
		if (entry.ifIndex != wan_ifIndex)
			continue;
			
		needDelete = 1;
		for (j=0; j<optNum; j++)
		{
			if (entry.ifIndex == wan_ifIndex && entry.tag == dhcpc_option_table[j].tag)
			{
				needDelete = 0;
				break;
			}
		}
		
		if (needDelete)
		{
			mib_chain_delete(MIB_DHCP_CLIENT_OPTION_TBL, i);
			printf("%s: delete old rule from interface 0x%x with %d option\n", __FUNCTION__, wan_ifIndex, entry.tag);
			needUpdate = 1;
			// reset order value by seguence.	
			compact_reqoption_order(wan_ifIndex);
		}
	}	
	
	if (needUpdate)
	{		
#ifdef COMMIT_IMMEDIATELY
		Commit();
#endif
		//printf("%s: write value into flash\n", __FUNCTION__);
	}
}

void addDhcpcOptionSent(int tag, unsigned char *value, int len)
{
	MIB_CE_DHCP_OPTION_T entry;

	entry.ifIndex = wan_ifIndex;
	entry.enable = 1;
	entry.order = 0;
	entry.tag = tag;
	entry.len = len;
	if (entry.len)
		memcpy(entry.value, value, entry.len);

	entry.usedFor = eUsedFor_DHCPClient_Sent;
	entry.dhcpOptInstNum = findMaxDHCPClientOptionInstNum(eUsedFor_DHCPClient_Sent, wan_ifIndex) + 1;

	mib_chain_add(MIB_DHCP_CLIENT_OPTION_TBL, (void *)&entry);
}

int updateDhcpcOptionTbl(int tag, unsigned char *value, int len)
{
	MIB_CE_DHCP_OPTION_T entry;
	int i, entrynum;

	entrynum = mib_chain_total(MIB_DHCP_CLIENT_OPTION_TBL);
	for (i=0; i<entrynum; i++)
	{
		if (!mib_chain_get(MIB_DHCP_CLIENT_OPTION_TBL, i, (void *)&entry))
			continue;

		if (entry.ifIndex != wan_ifIndex)
			continue;

		if (entry.usedFor != eUsedFor_DHCPClient_Sent)
			continue;

		if (entry.tag == tag)
			break;
	}

	if (i >= entrynum) {
		printf("no DHCP_CLIENT_ID entry in MIB_DHCP_CLIENT_OPTION_TBL\n");
		return 0;
	}

	entry.len = len;
	memcpy(entry.value, value, len);

	mib_chain_update(MIB_DHCP_CLIENT_OPTION_TBL, &entry, i);

	return 1;	//updated
}

void updateReqDhcpcOptionValue(struct dhcpMessage *packet)
{
	MIB_CE_DHCP_OPTION_T entry;
	int i, entrynum;
	unsigned char *temp;

	entrynum = mib_chain_total(MIB_DHCP_CLIENT_OPTION_TBL);
	for (i=0; i<entrynum; i++)
	{
		if (!mib_chain_get(MIB_DHCP_CLIENT_OPTION_TBL, i, (void *)&entry) || !entry.enable)
			continue;

		if (entry.ifIndex != wan_ifIndex)
			continue;

		if (entry.usedFor != eUsedFor_DHCPClient_Req)
			continue;

		if((temp = get_option(packet, entry.tag))) {
			entry.len = *(unsigned char *)(temp-1);
			memcpy(entry.value, temp, entry.len);
			mib_chain_update(MIB_DHCP_CLIENT_OPTION_TBL, &entry, i);
		}
	}
}
/*
void stopDhcpc()
{
	MIB_CE_DHCP_OPTION_T entry;
	int i, entrynum;

	printf("stop DHCPC process on ifindex 0x%x\n", wan_ifIndex);

	entrynum = mib_chain_total(MIB_DHCP_CLIENT_OPTION_TBL);
	for (i=entrynum-1; i>=0; i--)
	{
		if (!mib_chain_get(MIB_DHCP_CLIENT_OPTION_TBL, i, (void *)&entry))
			continue;

		if (entry.ifIndex == wan_ifIndex)
			mib_chain_delete(MIB_DHCP_CLIENT_OPTION_TBL, i);
	}
}
*/
#endif
/*ql:20080925 END*/

/* Broadcast a DHCP discover packet to the network, with an optionally requested IP */
int send_discover(unsigned long xid, unsigned long requested)
{
	struct dhcpMessage packet;

	init_packet(&packet, DHCPDISCOVER);
	packet.xid = xid;
	/*ql: 20080925 START: move below option to mibtbl*/
#ifndef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	if (requested)
		add_simple_option(packet.options, DHCP_REQUESTED_IP, requested);

	add_requests(&packet);
#else
	char type = DHCPDISCOVER;
	updateDhcpcOptionTbl(DHCP_MESSAGE_TYPE, (unsigned char *)&type, 1);
	addDhcpcOption(&packet, DHCPC_DISCOVER);

	addDhcpcRequest(&packet);
#endif
	// Kaohj
#ifdef DHCP_OPTION_125
	add_option_string(packet.options, client_config.vi_vendorSpec);
#endif
	/*ql: 20080925 END*/

// david, disable message. 2003-5-21
//	LOG(LOG_DEBUG, "Sending discover...");
	return raw_packet(&packet, INADDR_ANY, CLIENT_PORT, INADDR_BROADCAST,
				SERVER_PORT, MAC_BCAST_ADDR, client_config.ifindex);
}


/* Broadcasts a DHCP request message */
int send_selecting(unsigned long xid, unsigned long server, unsigned long requested)
{
	struct dhcpMessage packet;
	struct in_addr addr;

	init_packet(&packet, DHCPREQUEST);
	packet.xid = xid;

	/*ql: 20080925 START: move below option to mibtbl*/
#ifndef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	add_simple_option(packet.options, DHCP_REQUESTED_IP, requested);
	add_simple_option(packet.options, DHCP_SERVER_ID, server);

	add_requests(&packet);
#else
	char type = DHCPREQUEST;
	updateDhcpcOptionTbl(DHCP_MESSAGE_TYPE, (unsigned char *)&type, 1);
	updateDhcpcOptionTbl(DHCP_REQUESTED_IP, (unsigned char *)&requested, 4);
	updateDhcpcOptionTbl(DHCP_SERVER_ID, (unsigned char *)&server, 4);

	addDhcpcOption(&packet, DHCPC_REQUEST);

	addDhcpcRequest(&packet);
#endif
	// Kaohj
#ifdef DHCP_OPTION_125
	add_option_string(packet.options, client_config.vi_vendorSpec);
#endif
	/*ql: 20080925 END*/
	addr.s_addr = requested;
	LOG(LOG_DEBUG, "Sending select for %s...", inet_ntoa(addr));
	return raw_packet(&packet, INADDR_ANY, CLIENT_PORT, INADDR_BROADCAST,
				SERVER_PORT, MAC_BCAST_ADDR, client_config.ifindex);
}


/* Unicasts or broadcasts a DHCP renew message */
int send_renew(unsigned long xid, unsigned long server, unsigned long ciaddr)
{
	struct dhcpMessage packet;
	int ret = 0;

	init_packet(&packet, DHCPREQUEST);
	packet.xid = xid;
	packet.ciaddr = ciaddr;

	/*ql: 20080925 START: move below option to mibtbl*/
#ifndef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	add_requests(&packet);
#else
	char type = DHCPREQUEST;
	updateDhcpcOptionTbl(DHCP_MESSAGE_TYPE, (unsigned char *)&type, 1);
	addDhcpcOption(&packet, DHCPC_RENEW);
	addDhcpcRequest(&packet);
#endif
	/*ql: 20080925 END*/
	LOG(LOG_DEBUG, "Sending renew...");
	if (server)
		ret = kernel_packet(&packet, ciaddr, CLIENT_PORT, server, SERVER_PORT);
	else ret = raw_packet(&packet, INADDR_ANY, CLIENT_PORT, INADDR_BROADCAST,
				SERVER_PORT, MAC_BCAST_ADDR, client_config.ifindex);
	return ret;
}


/* Unicasts a DHCP release message */
int send_release(unsigned long server, unsigned long ciaddr)
{
	struct dhcpMessage packet;

	init_packet(&packet, DHCPRELEASE);
	packet.xid = random_xid();
	packet.ciaddr = ciaddr;

	/*ql: 20080925 START: move below option to mibtbl*/
#ifndef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	add_simple_option(packet.options, DHCP_REQUESTED_IP, ciaddr);
	add_simple_option(packet.options, DHCP_SERVER_ID, server);
#else
	char type = DHCPRELEASE;
	updateDhcpcOptionTbl(DHCP_MESSAGE_TYPE, (unsigned char *)&type, 1);
	updateDhcpcOptionTbl(DHCP_REQUESTED_IP, (unsigned char *)&ciaddr, 4);
	updateDhcpcOptionTbl(DHCP_SERVER_ID, (unsigned char *)&server, 4);

	addDhcpcOption(&packet, DHCPC_RELEASE);

	addDhcpcRequest(&packet);
#endif
	/*ql: 20080925 END*/

	LOG(LOG_DEBUG, "Sending release...");
	return kernel_packet(&packet, ciaddr, CLIENT_PORT, server, SERVER_PORT);
}


/* return -1 on errors that are fatal for the socket, -2 for those that aren't */
int get_raw_packet(struct dhcpMessage *payload, int fd)
{
	int bytes;
	struct udp_dhcp_packet packet;
	u_int32_t source, dest;
	u_int16_t check;

	memset(&packet, 0, sizeof(struct udp_dhcp_packet));
	bytes = read(fd, &packet, sizeof(struct udp_dhcp_packet));
	if (bytes < 0) {
		DEBUG(LOG_INFO, "couldn't read on raw listening socket -- ignoring");
		usleep(500000); /* possible down interface, looping condition */
		return -1;
	}

	if (bytes < (int) (sizeof(struct iphdr) + sizeof(struct udphdr))) {
		DEBUG(LOG_INFO, "message too short, ignoring");
		return -2;
	}

	if (bytes < ntohs(packet.ip.tot_len)) {
		DEBUG(LOG_INFO, "Truncated packet");
		return -2;
	}

	/* ignore any extra garbage bytes */
	bytes = ntohs(packet.ip.tot_len);

	/* Make sure its the right packet for us, and that it passes sanity checks */
	if (packet.ip.protocol != IPPROTO_UDP || packet.ip.version != IPVERSION ||
	    packet.ip.ihl != sizeof(packet.ip) >> 2 || packet.udp.dest != htons(CLIENT_PORT) ||
	    bytes > (int) sizeof(struct udp_dhcp_packet) ||
	    ntohs(packet.udp.len) != (short) (bytes - sizeof(packet.ip))) {
	    	DEBUG(LOG_INFO, "unrelated/bogus packet");
	    	return -2;
	}

	/* check IP checksum */
	check = packet.ip.check;
	packet.ip.check = 0;
	if (check != checksum(&(packet.ip), sizeof(packet.ip))) {
		DEBUG(LOG_INFO, "bad IP header checksum, ignoring");
		return -1;
	}

	/* verify the UDP checksum by replacing the header with a psuedo header */
	source = packet.ip.saddr;
	dest = packet.ip.daddr;
	check = packet.udp.check;
	packet.udp.check = 0;
	memset(&packet.ip, 0, sizeof(packet.ip));

	packet.ip.protocol = IPPROTO_UDP;
	packet.ip.saddr = source;
	packet.ip.daddr = dest;
	packet.ip.tot_len = packet.udp.len; /* cheat on the psuedo-header */
	if (check && check != checksum(&packet, bytes)) {
		DEBUG(LOG_ERR, "packet with bad UDP checksum received, ignoring");
		return -2;
	}

	memcpy(payload, &(packet.data), bytes - (sizeof(packet.ip) + sizeof(packet.udp)));

	if (ntohl(payload->cookie) != DHCP_MAGIC) {
		LOG(LOG_ERR, "received bogus message (bad magic) -- ignoring");
		return -2;
	}
	DEBUG(LOG_INFO, "oooooh!!! got some!");
	return bytes - (sizeof(packet.ip) + sizeof(packet.udp));

}

/* Dick Tam, 2003-05-16, Microsoft AUTO IP procedure */
void rt_prepare_microsoft_auto_ip_dhcpack(struct dhcpMessage *packet, u_int32_t addr)
{
	init_header(packet, DHCPACK);

	memcpy(packet->chaddr, client_config.arp, 6);

	packet->yiaddr = addr;
	packet->ciaddr = addr;

	/*ql: 20080925 START: move below option to mibtbl*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	add_simple_option(packet->options, DHCP_MESSAGE_TYPE, DHCPACK);
#endif
	/*ql: 20080925 END*/
	add_simple_option(packet->options, DHCP_SUBNET, htonl(0xFFFF0000));

	add_simple_option(packet->options, DHCP_LEASE_TIME, htonl(86400));

//	Test only
//	raw_packet(packet, INADDR_ANY, CLIENT_PORT, INADDR_BROADCAST,
//				SERVER_PORT, MAC_BCAST_ADDR, client_config.ifindex);
}



