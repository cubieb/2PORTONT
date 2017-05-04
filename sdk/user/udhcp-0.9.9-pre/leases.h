/* leases.h */
#ifndef _LEASES_H
#define _LEASES_H

#define DHCP_LEASE_TIMER_INTERVAL 5 /* 5s */

struct dhcpOfferedAddr;
typedef void (*pfLeaseExpiredCallBack)(struct dhcpOfferedAddr *pstLease);

enum ForcePortalDeviceType
{
	FP_InvalidDevice = 0,
	FP_Computer,
	FP_STB,
	FP_MOBILE,
};

struct dhcpClientInfo
{
    unsigned long ulDevice;
    pfLeaseExpiredCallBack pfLeaseExpired;
    void *pvDHCPClientData;
};

struct dhcpOfferedAddr {
	u_int8_t chaddr[16];
	u_int32_t yiaddr;	/* network order */
	u_int32_t expires;	/* host order */
    struct dhcpClientInfo stClientInfo; /* Extra client info, such as option 60 */
};

extern unsigned char blank_chaddr[];

void clear_lease(u_int8_t *chaddr, u_int32_t yiaddr);
struct dhcpOfferedAddr *add_lease(u_int8_t *chaddr, u_int32_t yiaddr, unsigned long lease);
int lease_expired(struct dhcpOfferedAddr *lease);
struct dhcpOfferedAddr *oldest_expired_lease(void);
struct dhcpOfferedAddr *find_lease_by_chaddr(u_int8_t *chaddr);
struct dhcpOfferedAddr *find_lease_by_yiaddr(u_int32_t yiaddr);
void clear_all_lease();
void clear_one_lease(struct dhcpOfferedAddr *pstLease);
void lease_timer();

int check_ip(u_int32_t addr);


#endif
