/* script.c
 *
 * Functions to call the DHCP client notification scripts
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
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#include "options.h"
#include "dhcpd.h"
#include "dhcpc.h"
#include "packet.h"
#include "options.h"
#include "debug.h"
#include <config/autoconf.h>
#include <rtk/options.h>
#include <rtk/utility.h>
#ifdef CONFIG_MIDDLEWARE
#include <rtk/sysconfig.h>
#include <rtk/midwaredefs.h>

int isMgtChannel()
{
	MIB_CE_ATM_VC_T Entry,*pEntry;
	unsigned int i,num;
	char wanif[5];

	pEntry = &Entry;

	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)pEntry ) ){
			continue;
		}

		if(pEntry->cmode != CHANNEL_MODE_RT1483 && pEntry->cmode != CHANNEL_MODE_IPOE){
			continue;
		}

		if(!(pEntry->ServiceList & X_CT_SRV_TR069))	{
			continue;
		}

		snprintf(wanif, 5, "vc%d", VC_INDEX(pEntry->ifIndex));
		if(!strcmp(wanif,client_config.interface)){
			return 1;
		}
	}

	return 0;
}
void setDhcpMgtDNS(unsigned char * dns)
{
	FILE * fp;
	char buff[32];

	memset(buff,0,32);
	sprintf(buff, "%d.%d.%d.%d\n", dns[0],dns[1],dns[2],dns[3]);
	fp = 	fopen(MGT_DNS_FILE, "w");
	fputs(buff, fp);
	fclose(fp);
}
#endif

/* get a rough idea of how long an option will be (rounding up...) */
static int max_option_length[] = {
	[OPTION_IP] =		sizeof("255.255.255.255 "),
	[OPTION_IP_PAIR] =	sizeof("255.255.255.255 ") * 2,
	[OPTION_STRING] =	1,
	[OPTION_BOOLEAN] =	sizeof("yes "),
	[OPTION_U8] =		sizeof("255 "),
	[OPTION_U16] =		sizeof("65535 "),
	[OPTION_S16] =		sizeof("-32768 "),
	[OPTION_U32] =		sizeof("4294967295 "),
	[OPTION_S32] =		sizeof("-2147483684 "),
#ifdef _CWMP_TR111_
	[OPTION_125] =	1,
#endif
};


static int upper_length(int length, struct dhcp_option *option)
{
	return max_option_length[option->flags & TYPE_MASK] *
	       (length / option_lengths[option->flags & TYPE_MASK]);
}


static int sprintip(char *dest, char *pre, unsigned char *ip)
{
	return sprintf(dest, "%s%d.%d.%d.%d", pre, ip[0], ip[1], ip[2], ip[3]);
}


/* really simple implementation, just count the bits */
static int mton(struct in_addr *mask)
{
	int i;
	unsigned long bits = ntohl(mask->s_addr);
	/* too bad one can't check the carry bit, etc in c bit
	 * shifting */
	for (i = 0; i < 32 && !((bits >> i) & 1); i++);
	return 32 - i;
}

//wpeng 20120411 added for parsestatic router option
#ifdef CONFIG_USER_DHCP_OPT_33_249
static int sprintpartialip(char *dest, int masklen, int octets, unsigned char *ip)
{
	if(octets == 4)
		return sprintf(dest, "%d.%d.%d.%d/%d", ip[0], ip[1], ip[2], ip[3], masklen);
	else if(octets == 3)
		return sprintf(dest, "%d.%d.%d.%d/%d", ip[0], ip[1], ip[2], 0, masklen);
	else if(octets == 2)
		return sprintf(dest, "%d.%d.%d.%d/%d", ip[0], ip[1], 0, 0, masklen);
	else if(octets == 1)
		return sprintf(dest, "%d.%d.%d.%d/%d", ip[0], 0, 0, 0, masklen);
	else if(octets == 0)
		return sprintf(dest, "%d.%d.%d.%d/%d", 0, 0, 0, 0, masklen);
}

#define PMAP_VC_START	1
#define PMAP_PPP_START	0x10
#define PMAP_NAS_START 0x20
#define PMAP_NAS_PPP_START 0x30

#define CHECK_NAS_IDX(a) (a >> 16)

int caculate_tblid(uint32_t ifid)
{
	int tbl_id;

	uint32_t ifindex;
	
	ifindex = ifid;

	//ifindex of nas0_* is 0x1ff01, 0x1ff02
	//ifindex of ppp* over nas0 is 0x10000, 0x10001
#ifdef CONFIG_ETHWAN
	if(CHECK_NAS_IDX(ifindex))
	{
		if(PPP_INDEX(ifindex) == DUMMY_PPP_INDEX)
			tbl_id = VC_INDEX(ifindex) + PMAP_NAS_START;
		else
			tbl_id = PPP_INDEX(ifindex) + PMAP_NAS_PPP_START;
	}
	else
#endif
	{
	//ifindex of vc* is 0xff01, 0xff02, ...
	//ifindex of ppp* is 0x00, 0x0101, 0x0202 ...
	if (PPP_INDEX(ifindex) == DUMMY_PPP_INDEX)
		tbl_id = VC_INDEX(ifindex) + PMAP_VC_START;
	else
		tbl_id = PPP_INDEX(ifindex) + PMAP_PPP_START;
	}

	return tbl_id;
}

static int aug_itoa(int digital, char* string, uint32_t num)
{
	int rt;

	if(NULL  == string)
		return -1;

	if(0 == num || num > 10)
		return -2;

	rt = snprintf(string, num, "%d", digital);

	return rt;
}

static void addStaticRT(unsigned char *data, int masklen, int octets){
	unsigned char ip_mask[18];
	unsigned char nxthp[15];
	int i, entrynum;
	MIB_CE_ATM_VC_T Entry;
	int32_t tbl_id;
	char str_tblid[10];
	char ext_ifname[IFNAMSIZ];
	
	sprintpartialip(ip_mask, masklen, octets, data);
	data += octets;
	sprintip(nxthp, "", data);
	data += 4;
	
	entrynum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<entrynum; i++){
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry) || !Entry.enable)
			continue;
		
		ifGetName(Entry.ifIndex,ext_ifname,sizeof(ext_ifname));
		if(!strcmp(ext_ifname, client_config.interface))
			break;
		/*
		if (Entry.ifIndex == client_config.interface)
			break;
		*/	
	}
	if(i == entrynum){
		printf("[%s %d]can not find the interface %s", __func__, __LINE__, client_config.interface);
		return;
	}
#ifdef NEW_PORTMAPPING
	if(Entry.itfGroup == 0){
		//printf("/bin/ip route add %s via %s dev %s\n", ip_mask, nxthp, client_config.interface);
		va_cmd("/bin/ip", 7, 1, "route", "add", ip_mask, "via", nxthp, "dev", client_config.interface);
	}else{
		tbl_id = caculate_tblid(Entry.ifIndex);
		aug_itoa(tbl_id, str_tblid, sizeof(str_tblid));
		//printf("/bin/ip route add %s via %s dev %s table %s\n", ip_mask, nxthp, client_config.interface, str_tblid);
		va_cmd("/bin/ip", 9, 1, "route", "add", ip_mask, "via", nxthp, "dev", client_config.interface, "table", str_tblid);
	}
#else
	//printf("/bin/ip route add %s via %s dev %s\n", ip_mask, nxthp, client_config.interface);
	va_cmd("/bin/ip", 7, 1, "route", "add", ip_mask, "via", nxthp, "dev", client_config.interface);
#endif
}


void dealStaticRTOption(unsigned char *data, int code){
	int optlen = 0, masklen = 0, octets = 0;
	optlen = data[OPT_LEN - 2];
	
	//printf("[%s %d]code = %d\n", __func__, __LINE__, code);
	
	if(DHCP_CLASSLESS_RT == code || DHCP_OPT_249 == code){
		while(optlen > 0){
			masklen = data[0];
			octets = masklen>24?4:(masklen>16?3:(masklen>8?2:(masklen>0?1:0)));
			data++;
			addStaticRT(data, masklen, octets);
			data += (4+octets);
			optlen -= (5+octets);
		}
	}else{
		while(optlen > 0){
			addStaticRT(data, 32, 4);
			data += 8;
			optlen -= 8;
		}
	}
}
#endif
//end of add

#ifdef RECEIVE_DHCP_OPTION_125
int dealOption125(unsigned char *data){
	// V-I Vendor-specific Information
	int ret=1;     // default is accept
	int optlen = 0, datalen = 0;	
	MIB_CE_ATM_VC_T entry = {0};
	int i, j, option=1;
	unsigned char *backup = data;	
	
	if(getWanEntrybyindex(&entry, wan_ifIndex) < 0)
	{		
		DEBUG(LOG_ERR, "0x%x is not a WAN interface. Do not check CT-COM option 125", wan_ifIndex);
		return 1;
	}
	
	for(i = 0 ; i < 4 ; i++) {		
		option = 1;
		data = backup;
		optlen = data[OPT_LEN - 2];		
		
		// match sub_code + sub_data
		if(entry.dhcp_opt125_enable[i] && entry.dhcp_opt125_type[i] == 1)
		{	
			ret = 0;
			while(optlen > 0){
				datalen = data[0+2];  // enterprise(2bytes)
				//printf("match sub_code : option %d, datalen=%d\n", option, datalen);
				
				data +=3;  // enterprise(2bytes) + datalen(1bytes)				
				if (strlen(entry.dhcp_opt125_sub_data[i]) == data[1]) {
					if(!strncmp(data+2, entry.dhcp_opt125_sub_data[i], strlen(entry.dhcp_opt125_sub_data[i]))) {
						printf("Find the V-I Vendor-specific by matching sub_code & sub_data. Accept it.\n");
						ret = 1;
						return ret;
					}		
				}				
				data += datalen;
				optlen -= (3+datalen);  // enterprise(2bytes) + datalen(1bytes)
				option++;
			}
		}
	}
	
	for(i = 0 ; i < 4 ; i++) {		
		option = 1;
		data = backup;
		optlen = data[OPT_LEN - 2];		
		
		// match value
		if(entry.dhcp_opt125_enable[i] && entry.dhcp_opt125_type[i] == 2)
		{	
			ret = 0;
			while(optlen > 0){
				datalen = data[0+2];  // enterprise(2bytes)
				//printf("match value: option %d, datalen=%d\n", option, datalen);
				
				data +=3;  // enterprise(2bytes) + datalen(1bytes)				
				if (strlen(entry.dhcp_opt125_value[i]) == datalen) {
					if(!strncmp(data, entry.dhcp_opt125_value[i], strlen(entry.dhcp_opt125_value[i]))) {
						printf("Find the V-I Vendor-specific by matching value. Accept it.\n");
						ret = 1;
						return ret;
					}		
				}				
				data += datalen;
				optlen -= (3+datalen);  // enterprise(2bytes) + datalen(1bytes)
				option++;
			}
		}
	}
	
	return ret;
}
#endif

#ifdef _CONFIG_DHCPC_OPTION43_ACSURL_
int dealOption43(unsigned char *data){	
	int i;     
	int optlen = 0;		
	char acsurl[256+1]={0};
	unsigned char cwmp_ProvisioningCode[64];	
	
	for (i=0; i<2; i++)
	{
		optlen = data[1];
		if (data[0] == 1 )      // for ACS URL
		{
			memcpy(acsurl, data+2, optlen);
			//printf("acsurl=%s\n", acsurl);
			if(!mib_set(CWMP_ACS_URL, (void *)acsurl))
			{
				printf("Set CWMP_ACS_URL error!\n");
				return 0;
			}
		}
		else if (data[0] == 2 )  // for ProvisioningCode
		{
			memcpy(cwmp_ProvisioningCode, data+2, optlen);
			//printf("cwmp_ProvisioningCode=%s\n", cwmp_ProvisioningCode);
			if(!mib_set(CWMP_PROVISIONINGCODE, (void *)cwmp_ProvisioningCode))
			{
				printf("Set CWMP_PROVISIONINGCODE error!\n");
				return 0;
			}
		}
			
		data +=	2+optlen;
	}	

	Commit();
	return 1;
}
#endif


int dealOption20(unsigned char *data){	 // Enable/Disable source routing
	int temp=data[0];
	char tmpStr[64];

	sprintf(tmpStr, "/bin/echo %d > /proc/sys/net/ipv4/conf/all/accept_source_route", temp);
	system(tmpStr);

	return 1;
}


int dealOption42(unsigned char *data){	
	int optlen = 0, tmplen=0;	
	char ipaddr[16]={0};
	
	optlen = data[OPT_LEN - 2];

	if (optlen > 0) {
		sprintf(ipaddr, "%d.%d.%d.%d", data[0],data[1],data[2],data[3]);
		if(!mib_set(MIB_NTP_SERVER_HOST2, (void *)ipaddr))
		{
			printf("Set MIB_NTP_SERVER_HOST2 error!\n");
			return 0;
		}
	}
	/* Allow to support multiple NTP server*/
	if (optlen > 4) {
		sprintf(ipaddr, "%d.%d.%d.%d", data[4],data[5],data[6],data[7]);

		if(!mib_set(MIB_NTP_SERVER_HOST1, (void *)ipaddr))
		{
			printf("Set MIB_NTP_SERVER_HOST1 error!\n");
			return 0;
		}
	}

	Commit();
	return 1;
}

int dealOption66(unsigned char *data){	
	int optlen = 0, tmplen=0;	
	char domainName[254+1]={0};
	
	optlen = data[OPT_LEN - 2];
	memcpy(domainName, data, optlen);
	domainName[optlen] = '\0';
	if(!mib_set(MIB_TFTP_SERVER_ADDR, (void *)domainName))
	{
		printf("Set TFTP_SERVER_ADDR error!\n");
		return 0;
	}
	Commit();
	return 1;
}

int dealOption67(unsigned char *data){	
	int optlen = 0, tmplen=0;	
	char domainName[254+1]={0};
	
	optlen = data[OPT_LEN - 2];
	memcpy(domainName, data, optlen);
	domainName[optlen] = '\0';
	if(!mib_set(MIB_BOOT_FILENAME, (void *)domainName))
	{
		printf("Set MIB_BOOT_FILENAME error!\n");
		return 0;
	}
	Commit();
	return 1;
}


int dealOption100(unsigned char *data){	
	int optlen = 0, tmplen=0;	
	char domainName[254+1]={0};
	
	optlen = data[OPT_LEN - 2];
	memcpy(domainName, data, optlen);
	domainName[optlen] = '\0';
	if(!mib_set(MIB_POSIX_TZ_STRING, (void *)domainName))
	{
		printf("Set Posix-TZ-String error!\n");
		return 0;
	}
	Commit();
	return 1;
}


#ifdef CONFIG_USER_RTK_VOIP
#define SIP_PROXY_NUM 2
int dealOption120(unsigned char *data)
{
	int optlen = 0, tmplen=0;	
	char url[SIP_PROXY_NUM][256+1]={"", ""};
	char ip[SIP_PROXY_NUM][IP_ADDR_LEN]={"", ""};
	char ipStr[SIP_PROXY_NUM][16]={"", ""};	
	char *current, *backup;
	int count=0, urllen, encode;
	
	optlen = data[OPT_LEN - 2];
	//  +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
	//  |120|27 | 0 | 7 |'e'|'x'|'a'|'m'|'p'|'l'|'e'| 3 |'c'|'o'|'m'| 0 |
	//  +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
	//  +---+---+---+---+---+---+---+---+---+---+---+---+---+ | 7
	//  |'e'|'x'|'a'|'m'|'p'|'l'|'e'| 3 |'n'|'e'|'t'| 0 | +---+---+---
	//  +---+---+---+---+---+---+---+---+---+---+
	// Get domain name
	if (data[0] == 0)
	{
		current = data+1;
		optlen = optlen-1;
		count = 0;
		encode = 0;
		
		while (optlen>0)
		{
			backup = current;
			urllen = 0;
			
			if (count >=SIP_PROXY_NUM)
				break;
				
			while (*current != 0)
			{
				tmplen = *current;
				current[0] = '.';
				current = current+tmplen+1;
				urllen = urllen+tmplen+1;
			}
			memcpy(url[count], &backup[1], urllen);
			optlen = optlen-urllen-1;
			count++;
			current++;
		}
	}
	
	// Code   Len   enc   Address 1               Address 2
	// +-----+-----+-----+-----+-----+-----+-----+-----+--
	// | 120 |  n  |  1  | a1  | a2  | a3  | a4  | a1  |  ...
	// +-----+-----+-----+-----+-----+-----+-----+-----+--
	// Get IP address
	if (data[0] == 1)
	{
		current = data+1;
		optlen = optlen-1;
		count = 0;
		encode = 1;
		
		while (optlen>0)
		{						
			if (count >=SIP_PROXY_NUM)
				break;
			memcpy(ip[count], current, IP_ADDR_LEN);
			strncpy(ipStr[count], inet_ntoa(*((struct in_addr *)ip[count])), 16);
			ipStr[count][15] = '\0';
			optlen = optlen-IP_ADDR_LEN;
			count++;
			current = current+IP_ADDR_LEN;
		}
	}
	
	if (encode == 0)
		voip_SetDhcpOption120(url);
	else if (encode == 1)
		voip_SetDhcpOption120(ipStr);

	return 1;
}
#endif

int dealOption15(unsigned char *data)
{
	int optlen = 0, tmplen=0;	
	char domainName[256+1]={0};
	
	optlen = data[OPT_LEN - 2];
	memcpy(domainName, data, optlen);
	domainName[optlen] = '\0';
	//printf("domainName=%s\n", domainName );


#ifdef CONFIG_USER_RTK_VOIP 
	//need check 
	voip_SetDhcpOption15(domainName);
#endif
	return 1;
}

#ifdef _PRMT_X_CT_COM_DHCP_
int appendOption60(unsigned char *value) {
	MIB_CE_ATM_VC_T entry = {0};
	int i;
	int total_len = 0;
	unsigned char field_len=0;
	unsigned char enterprise[2] = {0x00, 0x00};
	
	if(getWanEntrybyindex(&entry, wan_ifIndex) < 0)
	{		
		DEBUG(LOG_ERR, "appendOption60: 0x%x is not a WAN interface. Do not check CT-COM option 125", wan_ifIndex);
		return 1;
	}
	
	// (1) 2 bytes for Enterprise Code
	memcpy(value, enterprise, 2);
	total_len +=2;
	value +=2;
	
	// create data string with leading colon
	for (i = 0; i < 4; i++)
	{
		if(entry.dhcp_opt60_enable[i])
		{
			if(entry.dhcp_opt60_type[i] == 34 && entry.dhcp_opt60_value_mode[i] != 2)
				continue;

			//printf("%d option60 is enable\n", i+1);

			if(entry.dhcp_opt60_value_mode[i] == 0)
			{
				field_len = (unsigned char) strlen(entry.dhcp_opt60_value[i]);

				if(field_len > 0)
				{
					// (2) 1 bytes for Field type
					memcpy(value, &entry.dhcp_opt60_type[i], 1);
					total_len +=1;
					value +=1;

					// (3) 1 bytes for Field Length		
					memcpy(value, &field_len, 1);
					total_len +=1;
					value +=1;
			
					// (4) n bytes for Field Value
					memcpy(value, entry.dhcp_opt60_value[i], field_len);
					total_len += field_len;
					value += field_len;
				}
			}
			else if((entry.dhcp_opt60_type[i] == 34 && entry.dhcp_opt60_value_mode[i] == 2) ||
				(entry.dhcp_opt60_type[i] != 34 && entry.dhcp_opt60_value_mode[i] == 1))
			{
				char buf[80] = {0};

				field_len = gen_ctcom_dhcp_opt(entry.dhcp_opt60_type[i], buf, 80);

				if(field_len > 0)
				{
					// (2) 1 bytes for Field type
					memcpy(value, &entry.dhcp_opt60_type[i], 1);
					total_len +=1;
					value +=1;

					// (3) 1 bytes for Field Length
					memcpy(value, &field_len, 1);
					total_len +=1;
					value +=1;
				
					// (4) n bytes for Field Value
					memcpy(value, buf, field_len);
					total_len += field_len;
					value += field_len;
				}				
			}
		}
	}
	
	return total_len;	
}
#endif

/* Fill dest with the text of option 'option'. */
static void fill_options(char *dest, unsigned char *option, struct dhcp_option *type_p)
{
	int type, optlen;
	u_int16_t val_u16;
	int16_t val_s16;
	u_int32_t val_u32;
	int32_t val_s32;
	int len = option[OPT_LEN - 2];

	dest += sprintf(dest, "%s=", type_p->name);

	type = type_p->flags & TYPE_MASK;
	optlen = option_lengths[type];
	for(;;) {
		switch (type) {
		case OPTION_IP_PAIR:
			dest += sprintip(dest, "", option);
			*(dest++) = '/';
			option += 4;
			optlen = 4;
		case OPTION_IP:	/* Works regardless of host byte order. */
			dest += sprintip(dest, "", option);
 			break;
		case OPTION_BOOLEAN:
			dest += sprintf(dest, *option ? "yes" : "no");
			break;
		case OPTION_U8:
			dest += sprintf(dest, "%u", *option);
			break;
		case OPTION_U16:
			memcpy(&val_u16, option, 2);
			dest += sprintf(dest, "%u", ntohs(val_u16));
			break;
		case OPTION_S16:
			memcpy(&val_s16, option, 2);
			dest += sprintf(dest, "%d", ntohs(val_s16));
			break;
		case OPTION_U32:
			memcpy(&val_u32, option, 4);
			dest += sprintf(dest, "%lu", (unsigned long) ntohl(val_u32));
			break;
		case OPTION_S32:
			memcpy(&val_s32, option, 4);
			dest += sprintf(dest, "%ld", (long) ntohl(val_s32));
			break;
		case OPTION_STRING:
			memcpy(dest, option, len);
			dest[len] = '\0';
			return;	 /* Short circuit this case */
		}
		option += optlen;
		len -= optlen;
		if (len <= 0) break;
		dest += sprintf(dest, " ");
	}
}


static char *find_env(const char *prefix, char *defaultstr)
{
	extern char **environ;
	char **ptr;
	const int len = strlen(prefix);

	for (ptr = environ; *ptr != NULL; ptr++) {
		if (strncmp(prefix, *ptr, len) == 0)
			return *ptr;
	}
	return defaultstr;
}


/* put all the paramaters into an environment */
static char **fill_envp(struct dhcpMessage *packet)
{
	int num_options = 0;
	int i, j;
	char **envp;
	unsigned char *temp;
	struct in_addr subnet;
	char over = 0;

	if (packet == NULL)
		num_options = 0;
	else {
		for (i = 0; options[i].code; i++)
			if (get_option(packet, options[i].code))
				num_options++;
		if (packet->siaddr) num_options++;
		if ((temp = get_option(packet, DHCP_OPTION_OVER)))
			over = *temp;
		if (!(over & FILE_FIELD) && packet->file[0]) num_options++;
		if (!(over & SNAME_FIELD) && packet->sname[0]) num_options++;
	}

	envp = xmalloc((num_options + 7) * sizeof(char *));
	j = 0;
	envp[j++] = xmalloc(sizeof("interface=") + strlen(client_config.interface));
	sprintf(envp[0], "interface=%s", client_config.interface);
	envp[j++] = find_env("PATH", "PATH=/bin:/usr/bin:/sbin:/usr/sbin");
	envp[j++] = find_env("HOME", "HOME=/");

	if (packet == NULL) {
		envp[j++] = NULL;
		return envp;
	}

	envp[j] = xmalloc(sizeof("ip=255.255.255.255"));
	sprintip(envp[j++], "ip=", (unsigned char *) &packet->yiaddr);


	for (i = 0; options[i].code; i++) {
		if (!(temp = get_option(packet, options[i].code)))
			continue;
		envp[j] = xmalloc(upper_length(temp[OPT_LEN - 2], &options[i]) + strlen(options[i].name) + 2);
		fill_options(envp[j++], temp, &options[i]);

		/* Fill in a subnet bits option for things like /24 */
		if (options[i].code == DHCP_SUBNET) {
			envp[j] = xmalloc(sizeof("mask=32"));
			memcpy(&subnet, temp, 4);
			sprintf(envp[j++], "mask=%d", mton(&subnet));
			// Kaohj -- Add subnet IP. ex. 192.168.4.0
			envp[j] = xmalloc(sizeof("netip=255.255.255.255"));
			subnet.s_addr = packet->yiaddr&subnet.s_addr;
			sprintip(envp[j++], "netip=", (unsigned char *) &subnet.s_addr);
		}
#ifdef CONFIG_MIDDLEWARE
		if (options[i].code == DHCP_DNS_SERVER) {
			if(isMgtChannel())
				setDhcpMgtDNS(temp);
		}
#endif
		// For calulating lease time remains in cwmp-tr069
		if (options[i].code == DHCP_LEASE_TIME)
		{
			unsigned int lease = 0;
			envp[j] = xmalloc(sizeof("expire=") + 24);
			memcpy(&lease, temp, 4);
			sprintf(envp[j++], "expire=%ld", time(0) + (long int)lease);
		}
	}
	if (packet->siaddr) {
		envp[j] = xmalloc(sizeof("siaddr=255.255.255.255"));
		sprintip(envp[j++], "siaddr=", (unsigned char *) &packet->siaddr);
	}
	if (!(over & FILE_FIELD) && packet->file[0]) {
		/* watch out for invalid packets */
		packet->file[sizeof(packet->file) - 1] = '\0';
		envp[j] = xmalloc(sizeof("boot_file=") + strlen(packet->file));
		sprintf(envp[j++], "boot_file=%s", packet->file);
	}
	if (!(over & SNAME_FIELD) && packet->sname[0]) {
		/* watch out for invalid packets */
		packet->sname[sizeof(packet->sname) - 1] = '\0';
		envp[j] = xmalloc(sizeof("sname=") + strlen(packet->sname));
		sprintf(envp[j++], "sname=%s", packet->sname);
	}
	envp[j] = NULL;
	return envp;
}


/* Call a script with a par file and env vars */
void run_script(struct dhcpMessage *packet, const char *name)
{
	int pid;
	char **envp;

	if (client_config.script == NULL)
		return;

	/* call script */
	pid = fork();
	if (pid) {
		waitpid(pid, NULL, 0);
// Magician: Copy from Realsil E8B
		envp = fill_envp(packet);	

#if 0 // W.H. Hung: Don't need to send GWINFO
/*star:20090122 START message must be sent to boa after dhcpc script execute */
#ifdef IP_ROUTED_PVC_POLICY
		char *pRouter=NULL;
		char *pInterface=NULL;
		char *pIp=NULL;
		char *pMask=NULL;
		char **envpx=envp;
		while((*envpx)!=NULL)
		{
			printf("envpx=%s\n",*envpx);
			if(!pRouter) pRouter=strstr(*envpx,"router=");
			if(!pInterface) pInterface=strstr(*envpx,"interface=");
			if(!pIp) pIp=strstr(*envpx,"ip=");
			if(!pMask) pMask=strstr(*envpx,"subnet=");
			envpx++;
		}

		if(pRouter)
		{
			pRouter+=strlen("router=");
			printf("pRouter=%s\n",pRouter);
		}

		if(pInterface)
		{
			pInterface+=strlen("interface=");
			printf("pInterface=%s\n",pInterface);
		}

		if(pMask)
		{
			pMask+=strlen("subnet=");
			printf("pMask=%s\n",pMask);
		}

		if(pIp)
		{
			pIp+=strlen("ip=");
			printf("ip=%s\n",pIp);
		}

		if(pRouter&&pInterface&&pMask&&pIp)
		{
			GWINFO p;
			struct in_addr gwip;
			strcpy(p.name,pInterface);
			//gwip
			inet_aton(pRouter, &gwip);
			p.gwip=gwip.s_addr;
			//ip
			inet_aton(pIp, &gwip);
			p.ip=gwip.s_addr;
			//mask
			inet_aton(pMask, &gwip);
			p.mask=gwip.s_addr;
			sendMsg2Boa(CMD_PPPOE_DEFAULT_GW,& p, sizeof(GWINFO));
		}
#endif
/*star:20090122 END*/
#endif
// End Magician: Copy from Realsil E8B
		return;
	} else if (pid == 0) {
		envp = fill_envp(packet);

		/* close fd's? */

		/* exec script */
		DEBUG(LOG_INFO, "execle'ing %s", client_config.script);
		execle(client_config.script, client_config.script,
		       name, NULL, envp);
		LOG(LOG_ERR, "script %s failed: %s",
		    client_config.script, strerror(errno));
		exit(1);
	}
}
