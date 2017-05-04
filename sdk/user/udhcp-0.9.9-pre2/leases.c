/*
 * leases.c -- tools to manage DHCP leases
 * Russ Dill <Russ.Dill@asu.edu> July 2001
 */

#include <time.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "debug.h"
#include "dhcpd.h"
#include "files.h"
#include "options.h"
#include "leases.h"
#include "arpping.h"

// Mason Yu
#include <errno.h>

unsigned char blank_chaddr[] = {[0 ... 15] = 0};

extern int serverpool;

void lease_timer()
{
    int i, iUpdateForcePortal = 0;

	for (i = 0; i < server_config.max_leases; i++)
    {
		if ((leases[i].yiaddr != 0)
            && (lease_expired(&(leases[i]))))
        {
        	// Kaohj
#ifdef _CWMP_TR111_
		// Remove option 125 device when lease expired
		if (del_deviceId(leases[i].yiaddr))
			dump_deviceId();
#endif
            if (NULL != leases[i].stClientInfo.pfLeaseExpired)
            {
                leases[i].stClientInfo.pfLeaseExpired(&leases[i]);
            }

            if (FP_InvalidDevice != leases[i].stClientInfo.ulDevice)
            {
                iUpdateForcePortal = 1;
            }

            //LOG(LOG_INFO, "lease_timer of %s",  inet_ntoa(*((struct in_addr *)&(leases[i].yiaddr))));
            memset(&leases[i].stClientInfo, 0, sizeof(struct dhcpClientInfo));
            delete_hosts( inet_ntoa(*((struct in_addr *)&(leases[i].yiaddr))));
		}
	}

    if (iUpdateForcePortal)
    {
        /* update_force_portal(); */
    }

    return;
}

void clear_all_lease()
{
    int i;

	for (i = 0; i < server_config.max_leases; i++)
    {
		if (leases[i].yiaddr != 0)
        {
            if (NULL != leases[i].stClientInfo.pfLeaseExpired)
            {
                leases[i].stClientInfo.pfLeaseExpired(&leases[i]);
            }
		}
	}

    return;
}

void clear_one_lease(struct dhcpOfferedAddr *pstLease)
{
    int iUpdateForcePortal = 0;

    if (NULL != pstLease->stClientInfo.pfLeaseExpired)
    {
        pstLease->stClientInfo.pfLeaseExpired(pstLease);
    }

    if (FP_InvalidDevice != pstLease->stClientInfo.ulDevice)
    {
        iUpdateForcePortal = 1;
    }

    memset(pstLease, 0, sizeof(struct dhcpOfferedAddr));

    if (iUpdateForcePortal)
    {
        /* update_force_portal(); */
    }

    return;
}

/* clear every lease out that chaddr OR yiaddr matches and is nonzero */
void clear_lease(u_int8_t *chaddr, u_int32_t yiaddr)
{
	unsigned int i, j;

	for (j = 0; j < 16 && !chaddr[j]; j++);

	for (i = 0; i < server_config.max_leases; i++)
		if ((j != 16 && !memcmp(leases[i].chaddr, chaddr, 16)) ||
		    (yiaddr && leases[i].yiaddr == yiaddr)) {
			clear_one_lease(&(leases[i]));
		}
}


/* add a lease into the table, clearing out any old ones */
struct dhcpOfferedAddr *add_lease(u_int8_t *chaddr, u_int32_t yiaddr, unsigned long lease)
{
	struct dhcpOfferedAddr *oldest;

	/* clean out any old ones */
	clear_lease(chaddr, yiaddr);

	oldest = oldest_expired_lease();

	if (oldest) {
        clear_one_lease(oldest);
		memcpy(oldest->chaddr, chaddr, 16);
		oldest->yiaddr = yiaddr;
		// Mason Yu
		if ( lease == 0xffffffff ){
			//printf("DHCPD Lease time is overflow!!\n");
			oldest->expires = 0xffffffff;
		}else
			oldest->expires = time(0) + lease;
	}

	return oldest;
}


/* true if a lease has expired */
int lease_expired(struct dhcpOfferedAddr *lease)
{
	return (lease->expires < (unsigned long) time(0));
}


/* Find the oldest expired lease, NULL if there are no expired leases */
struct dhcpOfferedAddr *oldest_expired_lease(void)
{
	struct dhcpOfferedAddr *oldest = NULL;
	unsigned long oldest_lease = time(0);
	unsigned int i;


	for (i = 0; i < server_config.max_leases; i++)
		if (oldest_lease > leases[i].expires) {
			oldest_lease = leases[i].expires;
			oldest = &(leases[i]);
		}
	return oldest;

}


/* Find the first lease that matches chaddr, NULL if no match */
struct dhcpOfferedAddr *find_lease_by_chaddr(u_int8_t *chaddr)
{
	unsigned int i;

	for (i = 0; i < server_config.max_leases; i++)
		if (!memcmp(leases[i].chaddr, chaddr, 16)) return &(leases[i]);

	return NULL;
}


/* Find the first lease that matches yiaddr, NULL is no match */
struct dhcpOfferedAddr *find_lease_by_yiaddr(u_int32_t yiaddr)
{
	unsigned int i;

	for (i = 0; i < server_config.max_leases; i++)
		if (leases[i].yiaddr == yiaddr) return &(leases[i]);

	return NULL;
}

// Mason Yu
/* Find the first lease that matches chaddr and yiaddr, NULL if no match */
struct dhcpOfferedAddr *find_lease_by_chaddr_and_yiaddr(u_int8_t *chaddr, u_int32_t yiaddr)
{
	unsigned int i, j;

	for (i = 0; i < server_config.max_leases; i++) {
		if (!memcmp(leases[i].chaddr, chaddr, 6) && leases[i].yiaddr == yiaddr) {
			return &(leases[i]);
		}
	}

	return NULL;
}

// Mason Yu
int arpping_for_Server(u_int32_t yiaddr, u_int32_t ip, unsigned char *mac, char *interface)
{

//	int	timeout = 2;
	int   timeout = 1;
	int 	optval = 1;
	int	s;			/* socket */
	int	rv = 1;			/* return value */
	struct sockaddr addr;		/* for interface name */
	struct arpMsg	arp;
	fd_set		fdset;
	struct timeval	tm;
	time_t		prevTime;


	if ((s = socket (PF_PACKET, SOCK_PACKET, htons(ETH_P_ARP))) == -1) {
		LOG(LOG_ERR, "Could not open raw socket");
		return -1;
	}

	if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval)) == -1) {
		LOG(LOG_ERR, "Could not setsocketopt on raw socket");
		close(s);
		return -1;
	}

	/* send arp request */
	memset(&arp, 0, sizeof(arp));
	memcpy(arp.ethhdr.h_dest, MAC_BCAST_ADDR, 6);	/* MAC DA */
	memcpy(arp.ethhdr.h_source, mac, 6);		/* MAC SA */
	arp.ethhdr.h_proto = htons(ETH_P_ARP);		/* protocol type (Ethernet) */
	arp.htype = htons(ARPHRD_ETHER);		/* hardware type */
	arp.ptype = htons(ETH_P_IP);			/* protocol type (ARP message) */
	arp.hlen = 6;					/* hardware address length */
	arp.plen = 4;					/* protocol address length */
	arp.operation = htons(ARPOP_REQUEST);		/* ARP op code */
	memcpy(arp.sInaddr, &ip, sizeof(ip));		/* source IP address */
	memcpy(arp.sHaddr, mac, 6);			/* source hardware address */
	memcpy(arp.tInaddr, &yiaddr, sizeof(yiaddr));	/* target IP address */

	memset(&addr, 0, sizeof(addr));
	strcpy(addr.sa_data, interface);
	if (sendto(s, &arp, sizeof(arp), 0, &addr, sizeof(addr)) < 0)
		rv = 0;

	/* wait arp reply, and check it */
	tm.tv_usec = 0;
	time(&prevTime);
	while (timeout > 0) {
		FD_ZERO(&fdset);
		FD_SET(s, &fdset);
		tm.tv_sec = timeout;
		if (select(s + 1, &fdset, (fd_set *) NULL, (fd_set *) NULL, &tm) < 0) {
			DEBUG(LOG_ERR, "Error on ARPING request: %s", strerror(errno));
			if (errno != EINTR) rv = 0;
		} else if (FD_ISSET(s, &fdset)) {
			if (recv(s, &arp, sizeof(arp), 0) < 0 ) rv = 0;
			if (arp.operation == htons(ARPOP_REPLY) &&
			    bcmp(arp.tHaddr, mac, 6) == 0 &&
			    // Mason Yu
			    //*((u_int *) arp.sInaddr) == yiaddr) {
			    *((u_int *) arp.sInaddr) == yiaddr && find_lease_by_chaddr_and_yiaddr(arp.sHaddr, yiaddr)==NULL) {
			    printf("\nconflict address mac=%x-%x-%x-%x-%x-%x\n",arp.sHaddr[0],arp.sHaddr[1],arp.sHaddr[2],arp.sHaddr[3],arp.sHaddr[4],arp.sHaddr[5]);
				DEBUG(LOG_INFO, "Valid arp reply receved for this address");
				rv = 0;
				break;
			}
		}
		timeout -= time(NULL) - prevTime;
		time(&prevTime);
	}
	close(s);
	DEBUG(LOG_INFO, "%salid arp replies for this address", rv ? "No v" : "V");
	return rv;
}

/* find an assignable address, it check_expired is true, we check all the expired leases as well.
 * Maybe this should try expired leases by age... */
#ifdef IP_BASED_CLIENT_TYPE
//u_int32_t find_address(int check_expired, enum DeviceType devicetype )
u_int32_t find_address(int check_expired, struct client_category_t *deviceCategory )
#else
u_int32_t find_address(int check_expired)
#endif
{
	u_int32_t addr, ret;
	struct dhcpOfferedAddr *lease = NULL;

#ifdef IP_BASED_CLIENT_TYPE
	u_int32_t addrend;
	if (serverpool) {
			addr = ntohl(server_config.start);
			addrend = ntohl(server_config.end);
	} else {
		if (deviceCategory == NULL) {
			addr = ntohl(server_config.start);
			addrend = ntohl(server_config.end);
		} else {
			addr = ntohl(deviceCategory->ipstart);
			addrend = ntohl(deviceCategory->ipend);
		}
	}

	for (;addr <= addrend; addr++) {
		if (!serverpool) {
			if (deviceCategory == NULL) {
				struct client_category_t *pDhcp;
				for (pDhcp=server_config.clientRange; pDhcp; pDhcp=pDhcp->next) {
					if ((addr >= ntohl(pDhcp->ipstart)) && (addr <= ntohl(pDhcp->ipend)))
						break;
				}
				if (!pDhcp)
					continue;
			}
		}

		/* ie, 192.168.55.0 */
		if (!(addr & 0xFF)) continue;

		/* ie, 192.168.55.255 */
		if ((addr & 0xFF) == 0xFF) continue;
//jim added by star zhang
		if(find_Mac_by_IP(addr)) continue;// star add: for static ip based Mac
#ifdef SUPPORT_DHCP_RESERVED_IPADDR
		if( isReservedIPAddress(addr) ) continue;
#endif //SUPPORT_DHCP_RESERVED_IPADDR

		/* lease is not taken */
		ret = htonl(addr);
		if ((!(lease = find_lease_by_yiaddr(ret)) ||
			/* or it expired and we are checking for expired leases */
			(check_expired  && lease_expired(lease))) &&
			/* and it isn't on the network */
			!check_ip(ret)) {
				return ret;
				break;
		}
	}
#else  // IP_BASED_CLIENT_TYPE
	addr = ntohl(server_config.start); /* addr is in host order here */
	for (;addr <= ntohl(server_config.end); addr++) {

		/* ie, 192.168.55.0 */
		if (!(addr & 0xFF)) continue;

		/* ie, 192.168.55.255 */
		if ((addr & 0xFF) == 0xFF) continue;
//jim added by star zhang
		if(find_Mac_by_IP(addr)) continue;// star add: for static ip based Mac
#ifdef SUPPORT_DHCP_RESERVED_IPADDR
		if( isReservedIPAddress(addr) ) continue;
#endif //SUPPORT_DHCP_RESERVED_IPADDR

		/* lease is not taken */
		ret = htonl(addr);
		if ((!(lease = find_lease_by_yiaddr(ret)) ||

		     /* or it expired and we are checking for expired leases */
		     (check_expired  && lease_expired(lease))) &&

		     /* and it isn't on the network */
	    	     !check_ip(ret)) {
			return ret;
			break;
		}
	}
#endif
	return 0;
}


/* check is an IP is taken, if it is, add it to the lease table */
int check_ip(u_int32_t addr)
{
	struct in_addr temp;

	// Mason Yu
	//if (arpping(addr, server_config.server, server_config.arp, server_config.interface) == 0) {
	if (arpping_for_Server(addr, server_config.server, server_config.arp, server_config.interface) == 0) {
		temp.s_addr = addr;
	 	LOG(LOG_INFO, "%s belongs to someone, reserving it for %ld seconds",
	 		inet_ntoa(temp), server_config.conflict_time);
		add_lease(blank_chaddr, addr, server_config.conflict_time);
		return 1;
	} else return 0;
}

