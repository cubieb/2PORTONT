#include <unistd.h>
#include <netinet/in.h>

const char *xtables_ip6addr_to_numeric(const struct in6_addr *addrp)
{
	/* 0000:0000:0000:0000:0000:000.000.000.000
	 * 0000:0000:0000:0000:0000:0000:0000:0000 */
	static char buf[50+1];
	return inet_ntop(AF_INET6, addrp, buf, sizeof(buf));
}

struct in6_addr *xtables_numeric_to_ip6addr(const char *num)
{
	static struct in6_addr ap;
	int err;

	if ((err = inet_pton(AF_INET6, num, &ap)) == 1)
		return &ap;
#ifdef DEBUG
	fprintf(stderr, "\nnumeric2addr: %d\n", err);
#endif
	return NULL;
}
