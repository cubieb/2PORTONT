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
#include <rtk/mib.h>

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

//ql
unsigned int serverpool;
char g_dhcp_mode=0; // Mason Yu
char g_server_ip[16]="1.1.1.1";

/* Exit and cleanup */
//static void exit_server(int retval)   // try123
void exit_server(int retval)
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

void usage(char *s)
{
   printf("usage: %s [-h] {-S|-R|-B} {ServerIP|configFile}\n"
	  "\t-h:print this help\n"
	  "\t-S:run in the Server function\n"
	  "\t-R:run in the Relay fuction\n"
	  "\t-B:run in the Server and Relay function\n"
	  "\tServerIP:It is an IP Address. The CPE will relay packet to this IP\n"
	  "\tAddress.\n"
	  "\tconfigFile:It is config File for DHCP Server\n"
	  ,s);
   exit(1);
}

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
	int c;
	extern char *optarg;
	extern int optind;

	OPEN_LOG("udhcpd");
	LOG(LOG_INFO, "udhcp server/relay (v%s) started", VERSION);

	memset(&server_config, 0, sizeof(struct server_config_t));

	// Because we do not input config file for Relay, so we should set the server_config.pidfile with "/var/run/udhcpd.pid" for Relay.
	// After read_config() on Server , the server_config.pidfile will be set again.
	server_config.pidfile = strdup("/var/run/udhcpd.pid");
	pid_fd = pidfile_acquire(server_config.pidfile);
	pidfile_write_release(pid_fd);

	if (argc>2) {
    		while ((c = getopt(argc, argv,"B:S:R:")) !=EOF) {
    			//printf("optind: %d\n", optind);
      			switch (c) {
      				case 'S':
					g_dhcp_mode = LAN_SERVER;
					printf("Run Server function only\n");
					if ( optarg != NULL ) {
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
						p_serverpool_config=NULL;
						//pDHCPSPEntry = &DHCPSPEntry;
#endif
						read_config(optarg);
					} else
						usage(argv[0]);
					break;
     				case 'R':
					g_dhcp_mode = LAN_RELAY;
					printf("Run Relay function only\n");
					if ( optarg != NULL ) {
						strncpy(g_server_ip, optarg, strlen(optarg));
						g_server_ip[strlen(optarg)] = '\0';
					} else
						usage(argv[0]);
					break;
				case 'B':
					g_dhcp_mode = LAN_SERVER_AND_RELAY;
					printf("Run Server and Relay functions.\n");
					if ( optarg != NULL ) {
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
						p_serverpool_config=NULL;
						//pDHCPSPEntry = &DHCPSPEntry;
#endif
						read_config(optarg);
					} else
						usage(argv[0]);
					break;
      				case 'h':
      				default:
					usage(argv[0]);
      			}
      		}
      	}
  	else
    		usage(argv[0]);

    	if (argc != optind) {
    		usage(argv[0]);
    	}

	if ( g_dhcp_mode == LAN_SERVER || g_dhcp_mode == LAN_SERVER_AND_RELAY ) {
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
#if 0
			if (read_interface(p_servingpool_tmp->interface, &p_servingpool_tmp->ifindex,
				   &serverip, p_servingpool_tmp->arp) < 0)
				exit_server(1);
#else
			while (read_interface(p_servingpool_tmp->interface, &p_servingpool_tmp->ifindex,
				   &serverip, p_servingpool_tmp->arp) < 0) {
				printf("DHCPv4 Server: Interface %s is not ready\n", p_servingpool_tmp->interface);
				sleep(1);
			}
#endif
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
#if 0
		if (read_interface(server_config.interface, &server_config.ifindex,
				   &serverip, server_config.arp) < 0)
			exit_server(1);
#else
		while (read_interface(server_config.interface, &server_config.ifindex,
			&serverip, server_config.arp) < 0) {
			printf("DHCPv4 Server: Interface %s is not ready\n", server_config.interface);
			sleep(1);
		}
#endif
		if (0 == server_config.server)
			server_config.server = serverip;
	}
	#endif //_PRMT_X_TELEFONICA_ES_DHCPOPTION_
	} // if ( g_dhcp_mode == LAN_SERVER || g_dhcp_mode == LAN_SERVER_AND_RELAY ) {

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


		// If created by tty, stdin, stdout and stderr may be associated to tty.
		freopen("/dev/null", "r", stdin);
		freopen("/dev/console", "w", stdout);
		freopen("/dev/console", "w", stderr);

		stTimerInter.it_value.tv_sec = DHCP_LEASE_TIMER_INTERVAL;
		stTimerInter.it_value.tv_usec = 0;
		stTimerInter.it_interval = stTimerInter.it_value;
		setitimer(ITIMER_REAL, &stTimerInter, NULL);

		if ( g_dhcp_mode == LAN_SERVER || g_dhcp_mode == LAN_SERVER_AND_RELAY ) {
			timeout_end = time(0) + server_config.auto_time;
		}

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

	// Mason Yu. signal123
	listen_from_relay();

	return 0;
}

// Mason Yu. try123
int server_func(struct dhcpMessage packet)
{
	unsigned char *state;
	unsigned char *server_id, *requested;
	u_int32_t server_id_align, requested_align;
	struct option_set *option;
	struct dhcpOfferedAddr *lease;
	unsigned char *classVendor;
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	//struct server_config_t* p_servingpool_tmp;
	DHCPS_SERVING_POOL_T *pDHCPSPEntry, DHCPSPEntry;
	int entry_num, i;
	char isDroped = 0;
#endif
/*ping_zhang:20090316 START:Fix the DHCP_VENDOR string bugs*/
	unsigned char len = 0;
	unsigned char classVendorStr[256] = {0};
/*ping_zhang:20090316 END*/

#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	pDHCPSPEntry = &DHCPSPEntry;
#endif

	if ((state = get_option(&packet, DHCP_MESSAGE_TYPE)) == NULL) {
		DEBUG(LOG_ERR, "couldn't get option from packet, ignoring");
		//continue;
		return 0;
	}

// Mason Yu. try123
// Remove to got_one()
#if 0
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
		find_match_serving_pool(&packet);

		if( server_config.locallyserved==0 )
		{
			//LOG(LOG_INFO, "should be handled by relay, skip this DHCP message(0x%x)!", server_config.dhcpserver );
			//continue;
			return 0;
		}
#endif
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

// Magician: TR-069 DHCP Serving Pool.
#if 0 // Mason Yu. try123
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
						//return 0;

					if(strstr(classVendorStr, pDHCPSPEntry->vendorclass))
					{
						isDroped = !pDHCPSPEntry->localserved;
						break;
					}
				}
			}

			if(!isDroped)
#endif
#endif  // try123
// The end of TR-069 DHCP Serving Pool.
			if (sendOffer(&packet) < 0) {
				LOG(LOG_ERR, "send OFFER failed");
			}
			break;

 		case DHCPREQUEST:
// Magician: TR-069 DHCP Serving Pool.
#if 0 // Mason Yu. try123
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
						//return 0;

					if(strstr(classVendorStr, pDHCPSPEntry->vendorclass))
					{
						isDroped = !pDHCPSPEntry->localserved;
						break;
					}
				}
			}

			if(isDroped)
				//continue;
				return 0;
#endif
#endif
			// The end of TR-069 DHCP Serving Pool.
			DEBUG(LOG_INFO, "received REQUEST");
			requested = get_option(&packet, DHCP_REQUESTED_IP);
			server_id = get_option(&packet, DHCP_SERVER_ID);

#ifdef DHCP_OPTION_43_ENABLE
			OPT43_VendorSpecificInformation(get_option(&packet, DHCP_VENDOR_SPECIFIC_INFO));
#endif
#ifdef DHCP_OPTION_82_ENABLE
			OPT82_RelayAgentInformation(get_option(&packet, DHCP_RELAY_AGENT_INFO));
#endif

#ifdef IP_BASED_CLIENT_TYPE
			enum DeviceType devicetype;
			struct client_category_t *deviceCategory;
			struct dhcp_ctc_client_info stClientInfo;

#ifdef _PRMT_X_CT_COM_DHCP_
			unsigned char check_opt_60;

			mib_get(CWMP_CT_DHCPS_CHECK_OPT_60, &check_opt_60);
			if(!check_opt_60 || !(classVendor=get_option(&packet, DHCP_VENDOR)))
#else
			if(!(classVendor=get_option(&packet, DHCP_VENDOR)))
#endif
			{
				struct server_config_t *pDhcp;
				//default : PC clients....
				devicetype = CTC_Computer;
				deviceCategory=NULL;
				// Magicia: Every device other than default type is set to Computer.
				for (pDhcp=&server_config; pDhcp; pDhcp=pDhcp->next)
				{
					if(pDhcp->vendorclass && !strcmp(pDhcp->vendorclass, "Computer"))
					{
						deviceCategory = pDhcp->clientRange;
						break;
					}
				}
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
				parse_CTC_Vendor_Class(&packet, classVendor, &stClientInfo);
//				parse_CTC_Vendor_Class(&packet, classVendorStr, &stClientInfo);
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
							// Mason Yu. Req
							&& ( (ip_addr ==0 && !(find_Mac_by_IP(packet.ciaddr)) ) || ((ip_addr != 0) && (ip_addr == lease->yiaddr)))
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
						else {
							sendNAK(&packet);
						}
					} else {
						/* RENEWING or REBINDING State */
						if ((lease->yiaddr == packet.ciaddr)
						// Mason Yu. Req
						&& ( (ip_addr ==0 && !(find_Mac_by_IP(packet.ciaddr)) ) || ((ip_addr != 0) && (ip_addr == lease->yiaddr)))
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
						// Mason Yu. Req
						sendACK(&packet, requested_align);
					/* make some contention for this address */
					} else
						sendNAK(&packet);
				}
				//jim: added by star_zhang
				else if(ip_addr != 0){
					if(ip_addr!=requested_align)
						sendNAK(&packet);
					// Mason Yu. Req
					else {
						sendACK(&packet, requested_align);
					}
				}
				else if (
#ifdef IP_BASED_CLIENT_TYPE
					(ip_addr == 0) && (check_type(requested_align, deviceCategory))
#else
					(ip_addr == 0) && (requested_align < server_config.start || requested_align > server_config.end)
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
				// Mason Yu. Req
				else if ( (ip_addr == 0) && (requested_align > server_config.start || requested_align < server_config.end) ) {
					sendACK(&packet, requested_align);
				}
				else{
					//LOG(LOG_INFO, "sending NACK!\n");
					sendNAK(&packet);
				}
			}
			else {
				 /* RENEWING or REBINDING State */
//jim: modified by starzhang
//				sendACK(&packet, packet.ciaddr);	// Jenny, ack to client renewal request after CPE reboot
				ip_addr = find_IP_by_Mac(packet.chaddr);  //star, for static ip based Mac
				// Mason Yu. Req
				if ( ((ip_addr == 0 && !(find_Mac_by_IP(packet.ciaddr))) || ((ip_addr != 0) && (ip_addr == packet.ciaddr)))

#ifdef SUPPORT_DHCP_RESERVED_IPADDR
					&& ((ip_addr != 0) || ((ip_addr == 0) && (isReservedIPAddress(packet.ciaddr)==0)))
#endif //SUPPORT_DHCP_RESERVED_IPADDR
				#ifdef IP_BASED_CLIENT_TYPE
					&& ((ip_addr != 0) || (!check_type(packet.ciaddr, deviceCategory)))
/*star:20080927 START add for dhcp ip range check*/
				#else
					&& ((ip_addr != 0) || (packet.ciaddr >= server_config.start && packet.ciaddr <= server_config.end))
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

		return 0;
}
