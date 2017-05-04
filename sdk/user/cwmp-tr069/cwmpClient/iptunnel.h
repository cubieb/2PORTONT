#ifndef TUN_H
#define TUN_H
#include <netinet/in.h>
#include <linux/ip.h>
#include <linux/if_tunnel.h>
#include <linux/ip6_tunnel.h>
#include <sys/ioctl.h>
#include <net/if_arp.h>

struct ip46_tunnel_parm {
	int family;

	union {
		struct ip_tunnel_parm p4;
		struct ip6_tnl_parm p6;
	} uitp;
};

enum {
	tunl0,
	gre0,
	sit0,
	ip6tnl0,
	nr_basedevs,
};
extern const char *const basedevs[];

const char *tnl_ioctl_get_ifname(int idx, int family);
struct ifreq *tnl_ioctl_get_ifreq(int cmd, const char *name, int family);
int tnl_ioctl_set_ifreq(int cmd, struct ifreq *pifr, int family);
int tnl_ioctl(int cmd, const char *name, void *p, int family);
int do_tunnels_list(struct ip46_tunnel_parm *const ai46tp, const size_t nmemb);
#endif
