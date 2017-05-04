/*
 * options.c -- DHCP server option packet tools
 * Rewrite by Russ Dill <Russ.Dill@asu.edu> July 2001
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "dhcpd.h"
#include "files.h"
#include "options.h"
#include "leases.h"
#include <rtk/sysconfig.h>
#ifdef CONFIG_USER_RTK_VOIP
#include <errno.h>
#include <fcntl.h>
#include "voip_manager.h"
voipCfgParam_t *voip_pVoIPCfg = NULL;
int voip_modify_flag=0;
#define FIFO_SOLAR "/var/run/solar_control.fifo"
#endif


/* supported options are easily added here */
struct dhcp_option options[] = {
	/* name[10]	flags					code */
	{"subnet",	OPTION_IP | OPTION_REQ,			0x01},
	{"timezone",	OPTION_S32,				0x02},
	{"router",	OPTION_IP | OPTION_LIST | OPTION_REQ,	0x03},
	{"timesvr",	OPTION_IP | OPTION_LIST,		0x04},
	{"namesvr",	OPTION_IP | OPTION_LIST,		0x05},
	{"dns",		OPTION_IP | OPTION_LIST | OPTION_REQ,	0x06},
	{"logsvr",	OPTION_IP | OPTION_LIST,		0x07},
	{"cookiesvr",	OPTION_IP | OPTION_LIST,		0x08},
	{"lprsvr",	OPTION_IP | OPTION_LIST,		0x09},
	{"hostname",	OPTION_STRING | OPTION_REQ,		0x0c},
	{"bootsize",	OPTION_U16,				0x0d},
	{"domain",	OPTION_STRING | OPTION_REQ,		0x0f},
	{"swapsvr",	OPTION_IP,				0x10},
	{"rootpath",	OPTION_STRING,				0x11},
	{"ipttl",	OPTION_U8,				0x17},
	{"mtu",		OPTION_U16,				0x1a},
	{"broadcast",	OPTION_IP | OPTION_REQ,			0x1c},
	{"ntpsrv",	OPTION_IP | OPTION_LIST,		0x2a},
#ifdef CTC_DHCP_OPTION43
	{"vendorinfo",	OPTION_STRING,		0x2b},
#endif
	{"wins",	OPTION_IP | OPTION_LIST,		0x2c},
	{"requestip",	OPTION_IP,				0x32},
	{"lease",	OPTION_U32,				0x33},
	{"dhcptype",	OPTION_U8,				0x35},
	{"serverid",	OPTION_IP,				0x36},
	{"message",	OPTION_STRING,				0x38},
#ifdef CTC_DHCP_OPTION60
	{"vendorclassid",	OPTION_STRING,		0x3c},
#endif
	{"tftp",	OPTION_STRING, 66},
	{"bootfile",	OPTION_STRING, 67},
	{"tzstring",	OPTION_STRING, 100},
	{"sipsrv",	OPTION_120, 120},
#ifdef _CWMP_TR111_
	{"venspec",	OPTION_125,				0x7d},
#endif
	{"",		0x00,				0x00}
};

/* Lengths of the different option types */
int option_lengths[] = {
	[OPTION_IP] =		4,
	[OPTION_IP_PAIR] =	8,
	[OPTION_BOOLEAN] =	1,
	[OPTION_STRING] =	1,
	[OPTION_U8] =		1,
	[OPTION_U16] =		2,
	[OPTION_S16] =		2,
	[OPTION_U32] =		4,
	[OPTION_S32] =		4,
	[OPTION_120] =		1
#ifdef _CWMP_TR111_
	,[OPTION_125] =		1
#endif
};



/* get an option with bounds checking (warning, not aligned). */
unsigned char *get_option(struct dhcpMessage *packet, int code)
{
	int i, length;
	unsigned char *optionptr;
	int over = 0, done = 0, curr = OPTION_FIELD;

	optionptr = packet->options;
	i = 0;
	length = 308;
	while (!done) {
		if (i >= length) {
			LOG(LOG_WARNING, "bogus packet, option fields too long.");
			return NULL;
		}
		if (optionptr[i + OPT_CODE] == code) {
			if (i + 1 + optionptr[i + OPT_LEN] >= length) {
				LOG(LOG_WARNING, "bogus packet, option fields too long.");
				return NULL;
			}
			return optionptr + i + 2;
		}
		switch (optionptr[i + OPT_CODE]) {
		case DHCP_PADDING:
			i++;
			break;
		case DHCP_OPTION_OVER:
			if (i + 1 + optionptr[i + OPT_LEN] >= length) {
				LOG(LOG_WARNING, "bogus packet, option fields too long.");
				return NULL;
			}
			over = optionptr[i + 3];
			i += optionptr[OPT_LEN] + 2;
			break;
		case DHCP_END:
			if (curr == OPTION_FIELD && over & FILE_FIELD) {
				optionptr = packet->file;
				i = 0;
				length = 128;
				curr = FILE_FIELD;
			} else if (curr == FILE_FIELD && over & SNAME_FIELD) {
				optionptr = packet->sname;
				i = 0;
				length = 64;
				curr = SNAME_FIELD;
			} else done = 1;
			break;
		default:
			i += optionptr[OPT_LEN + i] + 2;
		}
	}
	return NULL;
}

/* return the position of the 'end' option (no bounds checking) */
int end_option(unsigned char *optionptr)
{
	int i = 0;

	while (optionptr[i] != DHCP_END) {
		if (optionptr[i] == DHCP_PADDING) i++;
		else i += optionptr[i + OPT_LEN] + 2;
	}
	return i;
}


/* add an option string to the options (an option string contains an option code,
 * length, then data) */
int add_option_string(unsigned char *optionptr, unsigned char *string)
{
	int end = end_option(optionptr);

	/* end position + string length + option code/length + end option */
	if (end + string[OPT_LEN] + 2 + 1 >= 308) {
		LOG(LOG_ERR, "Option 0x%02x did not fit into the packet!", string[OPT_CODE]);
		return 0;
	}
	DEBUG(LOG_INFO, "adding option 0x%02x", string[OPT_CODE]);
	memcpy(optionptr + end, string, string[OPT_LEN] + 2);
	optionptr[end + string[OPT_LEN] + 2] = DHCP_END;
	return string[OPT_LEN] + 2;
}


/* add a one to four byte option to a packet */
int add_simple_option(unsigned char *optionptr, unsigned char code, u_int32_t data)
{
	char length = 0;
	int i;
	unsigned char option[2 + 4];
	unsigned char *u8;
	u_int16_t *u16;
	u_int32_t *u32;
	u_int32_t aligned;
	u8 = (unsigned char *) &aligned;
	u16 = (u_int16_t *) &aligned;
	u32 = &aligned;

	for (i = 0; options[i].code; i++)
		if (options[i].code == code) {
			length = option_lengths[options[i].flags & TYPE_MASK];
		}

	if (!length) {
		DEBUG(LOG_ERR, "Could not add option 0x%02x", code);
		return 0;
	}

	option[OPT_CODE] = code;
	option[OPT_LEN] = length;

	switch (length) {
		case 1: *u8 =  data; break;
		case 2: *u16 = data; break;
		case 4: *u32 = data; break;
	}
	memcpy(option + 2, &aligned, length);
	return add_option_string(optionptr, option);
}

int add_option_61(struct dhcpMessage *oldpacket, struct dhcpMessage *packet)
{
	unsigned char clientid[256]={0};
	unsigned char *tmpstr;
	unsigned char len=0;
	unsigned char option[1+1+256];
	
	tmpstr=get_option(oldpacket, DHCP_CLIENT_ID);
	if(tmpstr!=NULL){
		len=*(unsigned char*)(tmpstr-OPT_LEN);
		memcpy(clientid,tmpstr,len);
		clientid[len]=0;
		
		option[OPT_CODE] = DHCP_CLIENT_ID;
		option[OPT_LEN] = len;
		memcpy(option+OPT_DATA, clientid, len);

		add_option_string(packet->options, option);
	}
	 return 1;
}

#include <rtk/mib.h>
#include <rtk/utility.h>
#include <signal.h>

int findClientIP_by_MAC(char *sMac, char *sIPaddr)
{
	FILE *fp;
	char buf[256];
	char arg1[20],arg2[20],arg4[20];
	int arg3;
	
	fp = fopen("/proc/net/arp", "r");
	if (fp == NULL){
		printf("read arp file fail!\n");
		return 0;
	}
    fgets(buf,256,fp);
	while(fgets(buf,256,fp)){
		sscanf(buf,"%s	%s	0x%x	%s",arg1,arg2,&arg3,arg4);
		if (!arg3) {
			continue;
		}
		
		if (!strcmp(sMac, arg4))
		{
			//printf("Found client, mac is %s\n", arg4);
			strcpy(sIPaddr, arg1);
			fclose(fp);
			return 1;
		}		
	}
	fclose(fp);
	return 0;
}

int add_hosts(struct dhcpMessage *oldpacket, struct dhcpMessage *packet, unsigned char *hostname)
{
	struct in_addr addr;
	FILE *fp;
	char domain[MAX_NAME_LEN];
	int dnsrelaypid=0;	
	
	char macStr[20]="\0";
	char sIPaddr[16]={0};	
	
	addr.s_addr = packet->yiaddr;
	
	// if the host is exist, ignore this action.
	//if(find_hosts(inet_ntoa(addr), hostname))
	//	return 1;
		
	// Mason Yu
	// Find client MAC	
	sprintf(macStr, "%02x:%02x:%02x:%02x:%02x:%02x", oldpacket->chaddr[0], oldpacket->chaddr[1],
			oldpacket->chaddr[2], oldpacket->chaddr[3], oldpacket->chaddr[4], oldpacket->chaddr[5]);    
    
	#if 0
	// Find client IP by Mac on /proc/net/arp	
	if(findClientIP_by_MAC(macStr, sIPaddr))
	{
		printf("Found client Mac is %s, IP is %s\n", macStr, sIPaddr);
		delete_hosts(sIPaddr);
	}
	#else
	delete_hosts( inet_ntoa(*((struct in_addr *)&addr)));
	#endif
	
	if ((fp = fopen(HOSTS, "a")) == NULL)
	{
		printf("Open file %s failed !\n", HOSTS);
		return 0;
	}

	// add DNS entry "hostname" for its own address	
	fprintf(fp, "%s\t", inet_ntoa(addr));
	
	mib_get(MIB_ADSL_LAN_DHCP_DOMAIN, (void *)domain);
	if (domain[0])
		fprintf(fp, "%s.%s ", hostname, domain);
	
	fprintf(fp, "%s\n", hostname);
	fclose(fp);
	
	dnsrelaypid = read_pid((char*)DNSRELAYPID);
	if(dnsrelaypid > 0)
		kill(dnsrelaypid, SIGHUP);
		
	return 1;
}

int find_hosts(unsigned char *yiaddr, unsigned char *hostname)
{	
	FILE *fp;	
	int dnsrelaypid=0;
	char temps[256+MAX_NAME_LEN]="", *pwd;
	char tmp1[20]="", tmp2[256+MAX_NAME_LEN]="", tmp3[256]="";	
	
	if ((fp = fopen(HOSTS, "r")) == NULL)
	{
		LOG(LOG_INFO, "Open file %s failed !\n", HOSTS);
		return 0;
	}
	
	fseek(fp, 0, SEEK_SET);	
	temps[0] = '\0';
	tmp1[0] = NULL;
	tmp2[0] = NULL;
	tmp3[0] = NULL;
	while (fgets(temps, 256+MAX_NAME_LEN, fp)) {		
		//LOG(LOG_INFO, "find_hosts: Get string is %s", temps);
		pwd = strstr(temps, yiaddr);
		if (pwd) {
			if (sscanf(temps, "%s%s%s", tmp1, tmp2, tmp3) == 3)			
			{
				//LOG(LOG_INFO, "find_hosts: Scan IP is %s, HostName is %s", tmp1, tmp3);
				if (!strcmp(tmp1, yiaddr) && !strcmp(tmp3, hostname)) {
					//LOG(LOG_INFO, "find_hosts: Match IP is %s, HostName is %s", tmp1, tmp3);
					fclose(fp);
					return 1;
				} 				
			}
		}
	}
	fclose(fp);	
	return 0;
}

int add_option_12(struct dhcpMessage *oldpacket, struct dhcpMessage *packet, char type)
{
	unsigned char hostname[256]={0};
	unsigned char *tmpstr;
	unsigned char len=0;
	unsigned char option[1+1+256];
	
	tmpstr=get_option(oldpacket, DHCP_HOST_NAME);
	if(tmpstr!=NULL){
		len=*(unsigned char*)(tmpstr-OPT_LEN);
		memcpy(hostname,tmpstr,len);
		hostname[len]=0;
		
		option[OPT_CODE] = DHCP_HOST_NAME;
		option[OPT_LEN] = len;
		memcpy(option+OPT_DATA, hostname, len);

		add_option_string(packet->options, option);
		
		if (type==DHCPACK)
			add_hosts(oldpacket, packet, hostname);
	}
	 return 1;
}

/* find option 'code' in opt_list */
struct option_set *find_option(struct option_set *opt_list, char code)
{
	while (opt_list && opt_list->data[OPT_CODE] < code)
		opt_list = opt_list->next;

	if (opt_list && opt_list->data[OPT_CODE] == code) return opt_list;
	else return NULL;
}


/* add an option to the opt_list */
void attach_option(struct option_set **opt_list, struct dhcp_option *option, char *buffer, int length)
{
	struct option_set *existing, *new, **curr;

	/* add it to an existing option */
	if ((existing = find_option(*opt_list, option->code))) {
		DEBUG(LOG_INFO, "Attaching option %s to existing member of list", option->name);
		if (option->flags & OPTION_LIST) {
			if (existing->data[OPT_LEN] + length <= 255) {
				existing->data = realloc(existing->data,
						existing->data[OPT_LEN] + length + 2);
				memcpy(existing->data + existing->data[OPT_LEN] + 2, buffer, length);
				existing->data[OPT_LEN] += length;
			} /* else, ignore the data, we could put this in a second option in the future */
		} /* else, ignore the new data */
	} else {
		DEBUG(LOG_INFO, "Attaching option %s to list", option->name);

		/* make a new option */
		new = xmalloc(sizeof(struct option_set));
		new->data = xmalloc(length + 2);
		new->data[OPT_CODE] = option->code;
		new->data[OPT_LEN] = length;
		memcpy(new->data + 2, buffer, length);

		curr = opt_list;
		while (*curr && (*curr)->data[OPT_CODE] < option->code)
			curr = &(*curr)->next;

		new->next = *curr;
		*curr = new;
	}
}

#ifdef DHCP_OPTION_82_ENABLE
int OPT82_AddSubOpt(unsigned char *opt82field, int subcode, const unsigned char *sub_data, int len)
{
	int sub_idx = OPT_DATA + opt82field[OPT_LEN];

	if(opt82field[OPT_LEN] + len > 256)
		return 0;

	opt82field[OPT_LEN] += 2 + len;
	opt82field[sub_idx] = subcode;
	opt82field[sub_idx+1] = len;
	memcpy(opt82field+sub_idx+2, sub_data, len);

	return 1;
}
#endif

#ifdef CONFIG_USER_RTK_VOIP
void restart_voip_callmanager(){
	int ret, h_pipe, res;
	FILE *fp;
	if (voip_modify_flag==0)
		return;
		
	if(voip_pVoIPCfg!=NULL){
		voip_flash_set(voip_pVoIPCfg);
		voip_flash_server_init_variables();
		ret= voip_flash_server_update();
		if (ret == -1)
			printf("voip flash server update fail in dhcp client\n");
	}


	if (access(FIFO_SOLAR, F_OK) !=0)
	{
		if (mkfifo(FIFO_SOLAR, 0755) == -1)
	        	printf("restart_voip_callmanager access %s failed: %s\n", FIFO_SOLAR, strerror(errno));
	}

	h_pipe = open(FIFO_SOLAR, O_RDWR);
	if (h_pipe == -1)
	{
		fprintf(stderr, "open %s failed\n", FIFO_SOLAR);
	}

	fprintf(stderr, "dhcp_restart_solar: restart...\n");
	res = write(h_pipe, "x\n", 2);		// restart solar
	if (res == -1)
	{
		fprintf(stderr, "write %s failed\n", FIFO_SOLAR);
		close(h_pipe);

	}


	close(h_pipe);
	
	voip_pVoIPCfg=NULL;
	voip_modify_flag=0;
	
	
}

void voip_SetDhcpOption120(char* proxy_addr)
{
	voipCfgPortParam_t *pCfg;
	int i,x;
	int ret;

	if(voip_pVoIPCfg==NULL){
		if (voip_flash_get(&voip_pVoIPCfg) != 0)
			return;
	}

	for( i=0 ; i<VOIP_PORTS ; i++ )
	{
		pCfg = &voip_pVoIPCfg->ports[i];
		
		for (x=0; x<MAX_PROXY; x++){

			if( proxy_addr != NULL )
			{

				//pCfg->proxies[x].outbound_enable = 1;
				//pCfg->proxies[x].outbound_port = 5060; // user hard code value first, user should modify manually if needed.
				strcpy(pCfg->proxies[x].addr, proxy_addr);
				voip_modify_flag=1;
			}
		}
	}
}



void voip_SetDhcpOption15(char* sip_domain)
{
#if 0 //only for rostelecom current not use this
	voipCfgPortParam_t *pCfg;
	int i,x;

	if(voip_pVoIPCfg==NULL){
		if (voip_flash_get(&voip_pVoIPCfg) != 0)
			return;
	}

	for( i=0 ; i<VOIP_PORTS; i++ )
	{
		pCfg = &voip_pVoIPCfg->ports[i];
		for (x=0; x<MAX_PROXY; x++){
			if( sip_domain != NULL ){
				strcpy(pCfg->proxies[x].domain_name, sip_domain);
				voip_modify_flag=1;
			}
		}
	}
#else
fprintf(stderr, "voip_SetDhcpOption15 current not use\n");
#endif
}

#endif
