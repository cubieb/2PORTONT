#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include "iptunnel.h"

const char * const basedevs[] = {
	"tunl0",
	"gre0",
	"sit0",
	"ip6tnl0",
};

const char *tnl_ioctl_get_ifname(int idx, int family)
{
	static struct ifreq ifr;
	int fd;
	int err;

	ifr.ifr_ifindex = idx;
	fd = socket(family, SOCK_DGRAM, 0);
	err = ioctl(fd, SIOCGIFNAME, &ifr);
	if (err)
		perror("ioctl");
	close(fd);

	return err ? NULL : ifr.ifr_name;
}

struct ifreq *tnl_ioctl_get_ifreq(int cmd, const char *name, int family)
{
	static struct ifreq ifr;
	int fd;
	int err;

	strncpy(ifr.ifr_name, name, IFNAMSIZ);
	fd = socket(family, SOCK_DGRAM, 0);
	err = ioctl(fd, cmd, &ifr);
	if (err)
		perror("ioctl");
	close(fd);

	return err ? NULL : &ifr;
}

int tnl_ioctl_set_ifreq(int cmd, struct ifreq *pifr, int family)
{
	int fd;
	int err;

	fd = socket(family, SOCK_DGRAM, 0);
	err = ioctl(fd, cmd, pifr);
	if (err)
		perror("ioctl");
	close(fd);

	return err;
}

int tnl_ioctl(int cmd, const char *name, void *p, int family)
{
	struct ifreq ifr;
	int fd;
	int err;

	strncpy(ifr.ifr_name, (name && name[0]) ? name : p, IFNAMSIZ);
	ifr.ifr_ifru.ifru_data = p;
	fd = socket(family, SOCK_DGRAM, 0);
	err = ioctl(fd, cmd, &ifr);
	if (err)
		perror("ioctl");
	close(fd);

	return err;
}

/**
 * do_tunnels_list - list tunnels within the system
 * @ai46tp: the array of structures storing tunnel parameters
 * @nmemb: the number of elements of the above array
 * returns the number of tunnels found in the system
 */
int do_tunnels_list(struct ip46_tunnel_parm *const ai46tp, const size_t nmemb)
{
	char name[IFNAMSIZ];
	int type, i;
	struct ip46_tunnel_parm *pi46tp;
	void *p;

	struct ifreq *pifr;

	char buf[512], *ptr;
	size_t nr;
	FILE *fp = fopen("/proc/net/dev", "r");
	if (fp == NULL) {
		perror("fopen");
		return -1;
	}

	/* skip two lines at the begenning of the file */
	fgets(buf, sizeof(buf), fp);
	fgets(buf, sizeof(buf), fp);

	for (nr = 0; nr < nmemb && fgets(buf, sizeof(buf), fp) != NULL;) {
		buf[sizeof(buf) - 1] = 0;
		if ((ptr = strchr(buf, ':')) == NULL ||
		    (*ptr++ = '\0', sscanf(buf, "%s", name) != 1)) {
			fprintf(stderr,
				"Wrong format of /proc/net/dev. Sorry.\n");
			nr = -1;
			goto ret;
		}

		/* skip the base devices */
		for (i = 0; i < nr_basedevs; i++) {
			if (strcmp(name, basedevs[i]) == 0)
				break;
		}
		if (i < nr_basedevs)
			continue;

		pifr = tnl_ioctl_get_ifreq(SIOCGIFHWADDR, name, AF_INET);
		if (pifr == NULL) {
			fprintf(stderr, "Failed to get type of [%s]\n", name);
			continue;
		}
		type = pifr->ifr_addr.sa_family;
		if (type != ARPHRD_TUNNEL && type != ARPHRD_IPGRE
		    && type != ARPHRD_SIT && type != ARPHRD_TUNNEL6)
			continue;

		pi46tp = ai46tp + nr;
		memset(pi46tp, 0, sizeof(*pi46tp));

		switch (type) {
		case ARPHRD_TUNNEL:
		case ARPHRD_IPGRE:
		case ARPHRD_SIT:
			pi46tp->family = AF_INET;
			p = &pi46tp->uitp.p4;
			break;
		case ARPHRD_TUNNEL6:
			pi46tp->family = AF_INET6;
			p = &pi46tp->uitp.p6;
			break;
		default:
			continue;
		}

		if (tnl_ioctl(SIOCGETTUNNEL, name, p, pi46tp->family))
			continue;

		nr++;
	}

ret:
	fclose(fp);

	return nr;
}

