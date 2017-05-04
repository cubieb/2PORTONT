/* dhcpd.c
 *
 * udhcp Server
 * Copyright (C) 1999 Matthew Ramsay <matthewr@moreton.com.au>
 *			Chris Trew <ctrew@moreton.com.au>
 *
 * Rewrite by Russ Dill <Russ.Dill@asu.edu> July 2001
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

#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/time.h>

#include "debug.h"
#include "dhcpd.h"
#include "arpping.h"
#include "socket.h"
#include "options.h"
#include "files.h"
#include "leases.h"
#include "packet.h"
#include "serverpacket.h"
#include "pidfile.h"
#include "../boa/src/LINUX/mib.h"

#if 0
#ifdef CONFIG_CTC_E8_CLIENT_LIMIT
#include <signal.h>
#include <sys/time.h>
#endif
#endif

/* globals */
struct dhcpOfferedAddr *leases;
struct server_config_t server_config;
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
struct server_config_t* p_serverpool_config;
#endif
static int signal_pipe[2];
//ql
unsigned int serverpool;

/* Exit and cleanup */
static void exit_server(int retval)
{
	pidfile_delete(server_config.pidfile);
	CLOSE_LOG();
	clear_all_lease();
// Kaohj --- TR111 Part 1
#ifdef _CWMP_TR111_
	clear_all_deviceId();
#endif
#ifdef IP_BASED_CLIENT_TYPE
	//ql, clear clientrange
	{
		struct client_category_t *p;
		while (server_config.clientRange)
		{
			p = server_config.clientRange;

			server_config.clientRange = server_config.clientRange->next;
			free(p);
		}
	}
#endif //IP_BASED_CLIENT_TYPE
    /* remove_force_portal(); */
	exit(retval);
}


/* Signal handler */
static void signal_handler(int sig)
{
	if (send(signal_pipe[1], &sig, sizeof(sig), MSG_DONTWAIT) < 0) {
		LOG(LOG_ERR, "Could not send signal: %s",
			strerror(errno));
	}
}

#if 0
#ifdef CONFIG_CTC_E8_CLIENT_LIMIT
// int maxPCClients=1;//MAXPCCLIENTS;
// int maxCameraClients=1;//MAXCAMERACLIENTS;
 struct itimerval read_proc_interval;
//struct CTC_Clients accepted_PC_Clients[MAXPCCLIENTS];
//struct CTC_Clients accepted_Camera_Clients[MAXCAMERACLIENTS];
static void read_proc_handler(int dummy)
{
	FILE *fd;
	int totallen;
	int parsedlen=0;
	int i;
	char buffer[1024];
	char *cur_ptr=buffer;
	char *str_end=buffer;
	int str_found;
	char cmd_str[256];
	//printf("read proc \n");
	fd=fopen(CLIENTSMONITOR, "r");
	if(fd==NULL)
	{
		printf("file %s open failed\n", CLIENTSMONITOR);
		return -1;
	}
	memset(buffer, 0, sizeof(buffer));
	totallen=fread(buffer, 1, 1024, fd);
	if(totallen!=0)
		printf("read size=%d, buffer=%s\n", totallen, buffer);
	if(totallen < 0)
		goto err;

	while(1)
	{
		str_found=0;
		memset(cmd_str, 0, sizeof(cmd_str));
		for(i=0; i<totallen-parsedlen; i++)
		{
			if(*str_end=='\n')
			{
				str_found=1;
				break;
			}
			str_end++;
		}
		if(str_found)
		{
			memcpy(cmd_str, cur_ptr, (unsigned)str_end-(unsigned)cur_ptr);
			printf("cmd parsed out: %s", cmd_str);
			system(cmd_str);//execute it....
			parsedlen+=i;
			str_end+=1;
			cur_ptr=str_end;

		}else
		{
			break;
		}
	}
err:
	fclose(fd);
	return;
}
#endif
#endif
#ifdef COMBINED_BINARY
int udhcpd_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
	fd_set rfds;
	struct timeval tv;
	int server_socket = -1;
	int bytes, retval;
	struct dhcpMessage packet;
	unsigned char *state;
	unsigned char *server_id, *requested;
	u_int32_t server_id_align, requested_align;
	unsigned long timeout_end;
	struct option_set *option;
	struct dhcpOfferedAddr *lease;
	int pid_fd, pid;
	int max_sock;
	int sig;
	unsigned long num_ips;
	struct itimerval stTimerInter;
	unsigned char *classVendor;
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	struct server_config_t* p_servingpool_tmp;
	//DHCPS_SERVING_POOL_T *pDHCPSPEntry, DHCPSPEntry;
	//int entry_num, i;
	//char isDroped = 0;
#endif
/*ping_zhang:20090316 START:Fix the DHCP_VENDOR string bugs*/
	unsigned char len = 0;
	unsigned char classVendorStr[256] = {0};
/*ping_zhang:20090316 END*/

	OPEN_LOG("udhcpd");
	LOG(LOG_INFO, "udhcp server (v%s) started", VERSION);

	memset(&server_config, 0, sizeof(struct server_config_t));

#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	p_serverpool_config=NULL;
	//pDHCPSPEntry = &DHCPSPEntry;
#endif

	if (argc < 2)
		read_config(DHCPD_CONF_FILE);
	else read_config(argv[1]);

	pid_fd = pidfile_acquire(server_config.pidfile);
	pidfile_write_release(pid_fd);


#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	p_servingpool_tmp=p_serverpool_config;
	while(p_servingpool_tmp!=NULL){
		unsigned int serverip;
		if ((option = find_option(p_servingpool_tmp->options, DHCP_LEASE_TIME))) {
			memcpy(&(p_servingpool_tmp->lease), option->data + 2, 4);
			p_servingpool_tmp->lease = ntohl(p_servingpool_tmp->lease);
		}
		else p_servingpool_tmp->lease = LEASE_TIME;

	#if 1
		p_servingpool_tmp->max_leases = 254; /*every pool shares the same lease num/structure*/
	#else
		num_ips = ntohl(p_servingpool_tmp->end) - ntohl(p_servingpool_tmp->start) + 1;
		if (p_servingpool_tmp->max_leases > num_ips) {
			p_servingpool_tmp->max_leases = num_ips;
		}
	#endif
		//LOG(LOG_INFO, "p_servingpool_tmp->max_leases=%d", p_servingpool_tmp->max_leases);

		if (read_interface(p_servingpool_tmp->interface, &p_servingpool_tmp->ifindex,
			   &serverip, p_servingpool_tmp->arp) < 0)
			exit_server(1);
		//ql add
		if (0 == p_servingpool_tmp->server) {
			p_servingpool_tmp->server = serverip;
		}

		p_servingpool_tmp=p_servingpool_tmp->next;
	}
	memcpy(&server_config,p_serverpool_config,sizeof(struct server_config_t));

	/*every pool shares the same lease num/structure*/
	//LOG(LOG_INFO, "create leases server_config.max_leases=%d, size=%d", server_config.max_leases, sizeof(struct dhcpOfferedAddr) * server_config.max_leases );
	leases = xmalloc(sizeof(struct dhcpOfferedAddr) * server_config.max_leases);
	memset(leases, 0, sizeof(struct dhcpOfferedAddr) * server_config.max_leases);
	read_leases(server_config.lease_file);
#else

	if ((option = find_option(server_config.options, DHCP_LEASE_TIME))) {
		memcpy(&server_config.lease, option->data + 2, 4);
		server_config.lease = ntohl(server_config.lease);
	}
	else server_config.lease = LEASE_TIME;

	/* Sanity check */
	// Kaohj -- num_ips should include both start and end ips
	//num_ips = ntohl(server_config.end) - ntohl(server_config.start);
	num_ips = ntohl(server_config.end) - ntohl(server_config.start) + 1;
	if (server_config.max_leases > num_ips) {
// david, disable message. 2003-5-21
//		LOG(LOG_ERR, "max_leases value (%lu) not sane, setting to %lu instead",
//			server_config.max_leases, num_ips);
		server_config.max_leases = num_ips;
	}

	leases = xmalloc(sizeof(struct dhcpOfferedAddr) * server_config.max_leases);
	memset(leases, 0, sizeof(struct dhcpOfferedAddr) * server_config.max_leases);
	read_leases(server_config.lease_file);

{
	unsigned int serverip;
	if (read_interface(server_config.interface, &server_config.ifindex,
			   &serverip, server_config.arp) < 0)
		exit_server(1);
	if (0 == server_config.server)
		server_config.server = serverip;
}
#endif //_PRMT_X_TELEFONICA_ES_DHCPOPTION_

#if 0
#ifndef DEBUGGING
	pid_fd = pidfile_acquire(server_config.pidfile); /* hold lock during fork. */
	if (daemon(0, 0) == -1) {
		perror("fork");
		exit_server(1);
	}
	pidfile_write_release(pid_fd);
#endif
#endif

	socketpair(AF_UNIX, SOCK_STREAM, 0, signal_pipe);
	signal(SIGUSR1, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGALRM, signal_handler);
    	// Kaohj -- get SIGHUP, associat stdin, stdout and stderr
    	signal(SIGHUP, signal_handler);
    	// If created by tty, stdin, stdout and stderr may be associated to tty.
    	freopen("/dev/null", "r", stdin);
    	freopen("/dev/console", "w", stdout);
    	freopen("/dev/console", "w", stderr);

    stTimerInter.it_value.tv_sec = DHCP_LEASE_TIMER_INTERVAL;
    stTimerInter.it_value.tv_usec = 0;
    stTimerInter.it_interval = stTimerInter.it_value;
    setitimer(ITIMER_REAL, &stTimerInter, NULL);

	timeout_end = time(0) + server_config.auto_time;

//jim luo
#if 0
#ifdef CONFIG_CTC_E8_CLIENT_LIMIT
	//memset(accepted_PC_Clients, 0, sizeof(accepted_PC_Clients));
	//memset(accepted_Camera_Clients, 0, sizeof(accepted_Camera_Clients));
	printf("install signal SIGALRM hander read_proc_handler\n");
	{
		pid_t pid;
		pid=fork();
		if(pid==-1)
		{
			printf("fork error\n");
			return;
		}
		if(pid==0)
		{	//child process
			//no need to
			/*
			signal(SIGALRM, read_proc_handler);
			read_proc_interval;
			read_proc_interval.it_value.tv_sec=1;
			read_proc_interval.it_value.tv_usec=0;
			read_proc_interval.it_interval=read_proc_interval.it_value;
			setitimer(ITIMER_REAL, &read_proc_interval, NULL );
			while(1);*/
			while(1)
			{
				sleep(1);
				read_proc_handler(0);
			}
		}
		if(pid)
		{
			printf("child process pid=%d created in dhcpd\n ", pid);
		}
	}
#endif
#endif
	while(1) { /* loop until universe collapses */
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
		memcpy(&server_config,p_serverpool_config,sizeof(struct server_config_t));
#endif

		if (server_socket < 0)
			if ((server_socket = listen_socket(INADDR_ANY, SERVER_PORT, server_config.interface)) < 0) {
				LOG(LOG_ERR, "FATAL: couldn't create server socket, %s", strerror(errno));
				exit_server(0);
			}

		FD_ZERO(&rfds);
		FD_SET(server_socket, &rfds);
		FD_SET(signal_pipe[0], &rfds);
		if (server_config.auto_time) {
			tv.tv_sec = timeout_end - time(0);
			tv.tv_usec = 0;
		}
		if (!server_config.auto_time || tv.tv_sec > 0) {
			max_sock = server_socket > signal_pipe[0] ? server_socket : signal_pipe[0];
			retval = select(max_sock + 1, &rfds, NULL, NULL,
					server_config.auto_time ? &tv : NULL);
		} else retval = 0; /* If we already timed out, fall through */
		if (retval == 0) {
			write_leases();
			timeout_end = time(0) + server_config.auto_time;
			continue;
		} else if (retval < 0 && errno != EINTR) {
			DEBUG(LOG_INFO, "error on select");
			continue;
		}

		if (FD_ISSET(signal_pipe[0], &rfds)) {
			if (read(signal_pipe[0], &sig, sizeof(sig)) < 0)
				continue; /* probably just EINTR */
			switch (sig) {
			case SIGUSR1:
//				LOG(LOG_INFO, "Received a SIGUSR1");
				write_leases();
				/* why not just reset the timeout, eh */
				timeout_end = time(0) + server_config.auto_time;
				continue;
			case SIGTERM:
				//LOG(LOG_INFO, "Received a SIGTERM");
				exit_server(0);
			case SIGHUP:
				//LOG(LOG_INFO, "Received a SIGHUP");
				// re-create socket pair
				socketpair(AF_UNIX, SOCK_STREAM, 0, signal_pipe);
				continue;

			case SIGALRM:
				/* Check whether there are expired leases. */
				lease_timer	();
				continue;
			}
		}

		if ((bytes = get_packet(&packet, server_socket)) < 0) { /* this waits for a packet - idle */
			if (bytes == -1 && errno != EINTR) {
				DEBUG(LOG_INFO, "error on read, %s, reopening socket", strerror(errno));
				close(server_socket);
				server_socket = -1;
			}
			continue;
		}

		if ((state = get_option(&packet, DHCP_MESSAGE_TYPE)) == NULL) {
			DEBUG(LOG_ERR, "couldn't get option from packet, ignoring");
			continue;
		}

#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
		find_match_serving_pool(&packet);

		if( server_config.locallyserved==0 )
		{
			//LOG(LOG_INFO, "should be handled by relay, skip this DHCP message(0x%x)!", server_config.dhcpserver );
			continue;		
		}
#endif
		/* ADDME: look for a static lease */
		lease = find_lease_by_chaddr(packet.chaddr);
		//jim: added by star_zhang
		u_int32_t ip_addr;
		switch (state[0]) {
		case DHCPDISCOVER:
			DEBUG(LOG_INFO,"received DISCOVER");
			//tylo, wait for 1 sec. to force PC get IP from WAN prior to CPE if there is DHCP server in WAN side
			// Kaohj -- not for general release (should be customized as necessary)
			//sleep(1);

#if 0
// Magician: TR-069 DHCP Serving Pool.
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
			isDroped = 0;

			if(classVendor=get_option(&packet, DHCP_VENDOR))
			{
				len=*(unsigned char*)(classVendor-OPT_LEN);
				memcpy(classVendorStr,classVendor,len);
				classVendorStr[len]=0;

				entry_num = mib_chain_total(MIB_DHCPS_SERVING_POOL_TBL);
				for( i = 0; i < entry_num; i++ )
				{
					if( !mib_chain_get(MIB_DHCPS_SERVING_POOL_TBL, i, (void*)pDHCPSPEntry) )
						continue;

					if(strstr(classVendorStr, pDHCPSPEntry->vendorclass))
					{
						isDroped = !pDHCPSPEntry->localserved;
						break;
					}
				}
			}

			if(!isDroped)
#endif
// The end of TR-069 DHCP Serving Pool.
#endif
				if (sendOffer(&packet) < 0)
					LOG(LOG_ERR, "send OFFER failed");

			break;
 		case DHCPREQUEST:
#if 0
// Magician: TR-069 DHCP Serving Pool.
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
			isDroped = 0;

			if(classVendor=get_option(&packet, DHCP_VENDOR))
			{
				len=*(unsigned char*)(classVendor-OPT_LEN);
				memcpy(classVendorStr,classVendor,len);
				classVendorStr[len]=0;

				entry_num = mib_chain_total(MIB_DHCPS_SERVING_POOL_TBL);
				for( i = 0; i < entry_num; i++ )
				{
					if( !mib_chain_get(MIB_DHCPS_SERVING_POOL_TBL, i, (void*)pDHCPSPEntry) )
						continue;

					if(strstr(classVendorStr, pDHCPSPEntry->vendorclass))
					{
						isDroped = !pDHCPSPEntry->localserved;
						break;
					}
				}
			}

			if(isDroped)
				continue;
#endif
// The end of TR-069 DHCP Serving Pool.
#endif
			DEBUG(LOG_INFO, "received REQUEST");
			requested = get_option(&packet, DHCP_REQUESTED_IP);
			server_id = get_option(&packet, DHCP_SERVER_ID);
#ifdef IP_BASED_CLIENT_TYPE
			enum DeviceType devicetype;
			struct client_category_t *deviceCategory;
			struct dhcp_ctc_client_info stClientInfo;

			if(!(classVendor=get_option(&packet, DHCP_VENDOR))) {
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
				//parse_CTC_Vendor_Class(&packet, classVendor, &stClientInfo);
				parse_CTC_Vendor_Class(&packet, classVendorStr, &stClientInfo);
/*ping_zhang:20090316 END*/
				devicetype = (enum DeviceType)(stClientInfo.category);
				deviceCategory = stClientInfo.iCategory;
			}
#endif

			if (requested) memcpy(&requested_align, requested, 4);
			if (server_id) memcpy(&server_id_align, server_id, 4);

			if (lease) { /*ADDME: or static lease */
				if (server_id) {
					/* SELECTING State */
					DEBUG(LOG_INFO, "server_id = %08x", ntohl(server_id_align));
					if (server_id_align == server_config.server && requested &&
					    requested_align == lease->yiaddr) {
						sendACK(&packet, lease->yiaddr);
					}
				} else {
					ip_addr = find_IP_by_Mac(packet.chaddr);
					if (requested) {
						/* INIT-REBOOT State */
						if ((lease->yiaddr == requested_align)
							&& (!(find_Mac_by_IP(requested_align)))
							&& ((ip_addr ==0) || ((ip_addr != 0) && (ip_addr == lease->yiaddr)))
#ifdef SUPPORT_DHCP_RESERVED_IPADDR
							&& ((ip_addr != 0) || ((ip_addr == 0) && (isReservedIPAddress(lease->yiaddr)==0)))
#endif //SUPPORT_DHCP_RESERVED_IPADDR
							&&  !check_ip(requested_align)
				#ifdef IP_BASED_CLIENT_TYPE
							&& ((ip_addr != 0) || (!check_type(lease->yiaddr, deviceCategory)))
/*star:20080927 START add for dhcp ip range check*/
				#else
							&& ((ip_addr !=0) || (lease->yiaddr >= server_config.start && lease->yiaddr <= server_config.end))
/*star:20080927 END*/
				#endif
						)
							sendACK(&packet, lease->yiaddr);
						else
							sendNAK(&packet);
					} else {
						/* RENEWING or REBINDING State */
						if ((lease->yiaddr == packet.ciaddr)
						&& (!(find_Mac_by_IP(packet.ciaddr)))
						&& ((ip_addr ==0) || ((ip_addr != 0) && (ip_addr == lease->yiaddr)))
#ifdef SUPPORT_DHCP_RESERVED_IPADDR
						&& ((ip_addr != 0) || ((ip_addr == 0) && (isReservedIPAddress(lease->yiaddr)==0)))
#endif //SUPPORT_DHCP_RESERVED_IPADDR
				#ifdef IP_BASED_CLIENT_TYPE
						&& ((ip_addr != 0) || (!check_type(lease->yiaddr, deviceCategory)))
/*star:20080927 START add for dhcp ip range check*/
				#else
						&& ((ip_addr !=0) || (lease->yiaddr >= server_config.start && lease->yiaddr <= server_config.end))
/*star:20080927 END*/
				#endif
						)
							sendACK(&packet, lease->yiaddr);
						else {
							/* don't know what to do!!!! */
							sendNAK(&packet);
						}
					}
				}

			/* what to do if we have no record of the client */
			}
			else if (server_id) {
				/* SELECTING State */

			}
			else if (requested) {
				/* INIT-REBOOT State */
				ip_addr = find_IP_by_Mac(packet.chaddr);
				if ((lease = find_lease_by_yiaddr(requested_align))) {
					if (lease_expired(lease)) {
						/* probably best if we drop this lease */
						clear_one_lease(lease);
					/* make some contention for this address */
					} else
						sendNAK(&packet);
				}
				//jim: added by star_zhang
				else if(ip_addr != 0){
					if(ip_addr!=requested_align)
						sendNAK(&packet);
				}
				else if (
#ifdef IP_BASED_CLIENT_TYPE
					    (ip_addr ==0) && (check_type(requested_align, deviceCategory))
#else
					    (ip_addr ==0) && (requested_align < server_config.start || requested_align > server_config.end)
#endif
				) {
					sendNAK(&packet);
				} /* else remain silent */

				else if(find_Mac_by_IP(requested_align))
					sendNAK(&packet);
#ifdef SUPPORT_DHCP_RESERVED_IPADDR
				else if( isReservedIPAddress(requested_align) )
					sendNAK(&packet);
#endif //SUPPORT_DHCP_RESERVED_IPADDR
			}
			else {
				 /* RENEWING or REBINDING State */
//jim: modified by starzhang
//				sendACK(&packet, packet.ciaddr);	// Jenny, ack to client renewal request after CPE reboot
				ip_addr = find_IP_by_Mac(packet.chaddr);  //star, for static ip based Mac
				if(((ip_addr == 0) || ((ip_addr != 0) &&(ip_addr == packet.ciaddr)))
					&& (!(find_Mac_by_IP(packet.ciaddr)))
#ifdef SUPPORT_DHCP_RESERVED_IPADDR
					&& ((ip_addr != 0) || ((ip_addr == 0) && (isReservedIPAddress(packet.ciaddr)==0)))
#endif //SUPPORT_DHCP_RESERVED_IPADDR
				#ifdef IP_BASED_CLIENT_TYPE
					&& ((ip_addr != 0) || (!check_type(packet.ciaddr, deviceCategory)))
/*star:20080927 START add for dhcp ip range check*/
				#else
					&& ((ip_addr !=0) || (packet.ciaddr >= server_config.start && packet.ciaddr <= server_config.end))
/*star:20080927 END*/
				#endif
				)
					sendACK(&packet, packet.ciaddr);
				else
					sendNAK(&packet);

			}
			break;
		case DHCPDECLINE:
			DEBUG(LOG_INFO,"received DECLINE");
			if (lease) {
				memset(lease->chaddr, 0, 16);
				lease->expires = time(0) + server_config.decline_time;
			}
			break;
		case DHCPRELEASE:
			DEBUG(LOG_INFO,"received RELEASE");
			if (lease) lease->expires = time(0);
#ifdef _CWMP_TR111_
			if (lease) {
				del_deviceId(lease->yiaddr);
				dump_deviceId();
			}
#endif
			break;
		case DHCPINFORM:
			DEBUG(LOG_INFO,"received INFORM");
			send_inform(&packet);
			break;
		default:
			LOG(LOG_WARNING, "unsupported DHCP message (%02x) -- ignoring", state[0]);
		}
	}

	return 0;
}

