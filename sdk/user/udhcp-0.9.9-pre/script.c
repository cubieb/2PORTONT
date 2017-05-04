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
	
	envp = xmalloc((num_options + 5) * sizeof(char *));
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
		}
#ifdef CONFIG_MIDDLEWARE
		if (options[i].code == DHCP_DNS_SERVER) {
			if(isMgtChannel())
				setDhcpMgtDNS(temp);	
		}
#endif
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

#ifdef _WT_146_
#define BFD_CONF_PREFIX "/var/bfd/bfdconf_"
extern int wt146_dbglog_get(unsigned char *ifname);
static void handle_bfdsession(struct dhcpMessage *packet, const char *name)
{
	if( name)
	{
		unsigned char buf[128];
		static unsigned int cur_ip=0;
		static unsigned int cur_gw=0;

		if(strcmp(name, "deconfig")==0)
		{
			sprintf( buf, "/bin/bfdctl del %s", client_config.interface );
			LOG(LOG_INFO, "(%s) exec: %s", name, buf);
			system( buf );
			cur_ip=0;
			cur_gw=0;
		}else if( (strcmp(name, "bound")==0)||(strcmp(name, "renew")==0)  )
		{
			if(packet)
			{
				unsigned char *pr;
				struct in_addr temp_addr;
				unsigned int *pgw;
				
				pr=get_option(packet, DHCP_ROUTER);
				pgw=(unsigned int *)pr;
				if((packet->yiaddr==cur_ip)&&(pgw!=NULL)&&(*pgw==cur_gw))
				{
					//LOG(LOG_INFO, "packet->yiaddr(0x%08x) == cur_ip(0x%08x), pgw(0x%08x) == cur_gw(0x%08x)", 
					//	packet->yiaddr, cur_ip, *pgw, cur_gw);
				}else{
					//try to del first
					sprintf( buf, "/bin/bfdctl del %s", client_config.interface );
					LOG(LOG_INFO, "(%s) exec: %s", name, buf);
					system( buf );
					cur_ip=0;
					cur_gw=0;

					if(pgw&&*pgw&&packet->yiaddr)
					{
						sprintf( buf, "/bin/bfdctl add %s file %s%s", 
								client_config.interface, BFD_CONF_PREFIX, client_config.interface);
						temp_addr.s_addr = packet->yiaddr;
						sprintf( &buf[strlen(buf)], " LocalIP %s", inet_ntoa(temp_addr));
						temp_addr.s_addr = *pgw;
						sprintf( &buf[strlen(buf)], " RemoteIP %s", inet_ntoa(temp_addr));
						//sprintf( &buf[strlen(buf)], " RemoteIP %s", "192.168.2.200");
						sprintf( &buf[strlen(buf)], " dhcpc %d", getpid() );
						sprintf( &buf[strlen(buf)], " debug %d", wt146_dbglog_get(client_config.interface)?1:0 );
						LOG(LOG_INFO, "(%s) exec: %s", name, buf);
						system( buf );

						cur_ip=packet->yiaddr;
						cur_gw=*pgw;
						//LOG(LOG_INFO, "set cur_ip=0x%08x, cur_gw=0x%08x", cur_ip, cur_gw);
					}
				}
			}
		}
	}
}
#endif //_WT_146_


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
#ifdef _WT_146_
		handle_bfdsession(packet, name);
#endif //_WT_146_
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
