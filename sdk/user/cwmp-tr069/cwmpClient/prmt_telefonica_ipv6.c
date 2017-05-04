#include <signal.h>
#include "prmt_telefonica_ipv6.h"
#include "iptunnel.h"


struct CWMP_OP tIPTunnelEntityLeafOP = { getIPTunnelEntity, setIPTunnelEntity };

struct CWMP_PRMT tIPTunnelEntityLeafInfo[] =
{
/* name,		type,		flag,			op */
{"Activated",		eCWMP_tBOOLEAN,	CWMP_READ | CWMP_WRITE,	&tIPTunnelEntityLeafOP},
{"TunnelName",		eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	&tIPTunnelEntityLeafOP},
{"TunnelCounter",	eCWMP_tINT,	CWMP_READ,		&tIPTunnelEntityLeafOP},
{"Mode",		eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	&tIPTunnelEntityLeafOP},
{"AssociatedWanIfName",	eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	&tIPTunnelEntityLeafOP},
{"AssociatedLanIfName",	eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	&tIPTunnelEntityLeafOP},
};

enum eIPTunnelEntityLeaf
{
	eActivated,
	eTunnelName,
	eTunnelCounter,
	eMode,
	eAssociatedWanIfName,
	eAssociatedLanIfName,
};

/* InternetGateWayDevice.X_TELEFONICA-ES_IPTunnel.{i}.* */
struct CWMP_LEAF tIPTunnelEntityLeaf[] =
{
{ &tIPTunnelEntityLeafInfo[eActivated] },
{ &tIPTunnelEntityLeafInfo[eTunnelName] },
{ &tIPTunnelEntityLeafInfo[eTunnelCounter] },
{ &tIPTunnelEntityLeafInfo[eMode] },
{ &tIPTunnelEntityLeafInfo[eAssociatedWanIfName] },
{ &tIPTunnelEntityLeafInfo[eAssociatedLanIfName] },
{ NULL },
};


struct CWMP_OP t4in6TunnelEntityLeafOP = { get4in6TunnelEntity, set4in6TunnelEntity };

struct CWMP_PRMT t4in6TunnelEntityLeafInfo[] =
{
/* name,		type,		flag,			op */
{"ConnStatus",		eCWMP_tSTRING,	CWMP_READ,		&t4in6TunnelEntityLeafOP},
{"Mechanism",		eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	&t4in6TunnelEntityLeafOP},
{"Dynamic",		eCWMP_tBOOLEAN,	CWMP_READ | CWMP_WRITE,	&t4in6TunnelEntityLeafOP},
{"RemoteIpv6Address",	eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	&t4in6TunnelEntityLeafOP},
};

enum e4in6TunnelEntityLeaf
{
	eConnStatus,
	eMechanism,
	eDynamic,
	eRemoteIpv6Address,
};

/* InternetGateWayDevice.X_TELEFONICA-ES_IPTunnel.{i}.4in6Tunnel.{i}.* */
struct CWMP_LEAF t4in6TunnelEntityLeaf[] =
{
{ &t4in6TunnelEntityLeafInfo[eConnStatus] },
{ &t4in6TunnelEntityLeafInfo[eMechanism] },
{ &t4in6TunnelEntityLeafInfo[eDynamic] },
{ &t4in6TunnelEntityLeafInfo[eRemoteIpv6Address] },
{ NULL },
};


struct CWMP_PRMT t4in6TunnelObjectInfo[] =
{
/* name,	type,		flag,					op */
{"0",		eCWMP_tOBJECT,	CWMP_READ | CWMP_WRITE | CWMP_LNKLIST,	NULL},
};

enum e4in6TunnelObject
{
	e4in6Tunnel0,
};

/* InternetGateWayDevice.X_TELEFONICA-ES_IPTunnel.{i}.4in6Tunnel.{i}. */
struct CWMP_LINKNODE t4in6TunnelObject[] =
{
/* info,				leaf,			next,	sibling,	instnum */
{ &t4in6TunnelObjectInfo[e4in6Tunnel0],	t4in6TunnelEntityLeaf,	NULL,	NULL,		0 },
};


struct CWMP_OP t6in4TunnelEntityLeafOP = { get6in4TunnelEntity, set6in4TunnelEntity };

struct CWMP_PRMT t6in4TunnelEntityLeafInfo[] =
{
/* name,		type,		flag,			op */
{"ConnStatus",		eCWMP_tSTRING,	CWMP_READ,		&t6in4TunnelEntityLeafOP},
{"Mechanism",		eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	&t6in4TunnelEntityLeafOP},
{"Dynamic",		eCWMP_tBOOLEAN,	CWMP_READ | CWMP_WRITE,	&t6in4TunnelEntityLeafOP},
{"IPv4MaskLen",		eCWMP_tINT,	CWMP_READ | CWMP_WRITE,	&t6in4TunnelEntityLeafOP},
{"Prefix",		eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	&t6in4TunnelEntityLeafOP},
{"BorderRelayAddress",	eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	&t6in4TunnelEntityLeafOP},
};

enum e6in4TunnelEntityLeaf
{
	eIPv4MaskLen = 3,
	ePrefix,
	eBorderRelayAddress,
};

/* InternetGateWayDevice.X_TELEFONICA-ES_IPTunnel.{i}.6in4Tunnel.{i}.* */
struct CWMP_LEAF t6in4TunnelEntityLeaf[] =
{
{ &t6in4TunnelEntityLeafInfo[eConnStatus] },
{ &t6in4TunnelEntityLeafInfo[eMechanism] },
{ &t6in4TunnelEntityLeafInfo[eDynamic] },
{ &t6in4TunnelEntityLeafInfo[eIPv4MaskLen] },
{ &t6in4TunnelEntityLeafInfo[ePrefix] },
{ &t6in4TunnelEntityLeafInfo[eBorderRelayAddress] },
{ NULL },
};


struct CWMP_PRMT t6in4TunnelObjectInfo[] =
{
/* name,	type,		flag,					op */
{"0",		eCWMP_tOBJECT,	CWMP_READ | CWMP_WRITE | CWMP_LNKLIST,	NULL},
};

enum e6in4TunnelObject
{
	e6in4Tunnel0,
};

/* InternetGateWayDevice.X_TELEFONICA-ES_IPTunnel.{i}.6in4Tunnel.{i}. */
struct CWMP_LINKNODE t6in4TunnelObject[] =
{
/* info,				leaf,			next,	sibling,	instnum */
{ &t6in4TunnelObjectInfo[e6in4Tunnel0],	t6in4TunnelEntityLeaf,	NULL,	NULL,		0 },
};


struct CWMP_OP t4in6TunnelOP = { NULL, obj4in6Tunnel };
struct CWMP_OP t6in4TunnelOP = { NULL, obj6in4Tunnel };

struct CWMP_PRMT tIPTunnelEntityObjectInfo[] =
{
/* name,	type,		flag,			op */
{"4in6Tunnel",	eCWMP_tOBJECT,	CWMP_READ | CWMP_WRITE,	&t4in6TunnelOP},
{"6in4Tunnel",	eCWMP_tOBJECT,	CWMP_READ | CWMP_WRITE,	&t6in4TunnelOP},
};

enum eIPTunnelEntityObject
{
	e4in6Tunnel,
	e6in4Tunnel,
};

/* InternetGateWayDevice.X_TELEFONICA-ES_IPTunnel.{i}.*. */
struct CWMP_NODE tIPTunnelEntityObject[] =
{
/* info,  					leaf,		next */
{ &tIPTunnelEntityObjectInfo[e4in6Tunnel],	NULL,		NULL},
{ &tIPTunnelEntityObjectInfo[e6in4Tunnel],	NULL,		NULL},
{ NULL, NULL, NULL },
};


struct CWMP_PRMT tIPTunnelObjectInfo[] =
{
/* name,	type,		flag,					op */
{"0",		eCWMP_tOBJECT,	CWMP_READ | CWMP_WRITE | CWMP_LNKLIST,	NULL},
};

enum eIPTunnelObject
{
	eIPTunnel0,
};

/* InternetGateWayDevice.X_TELEFONICA-ES_IPTunnel.{i}. */
struct CWMP_LINKNODE tIPTunnelObject[] =
{
/* info,				leaf,			next,			sibling,	instnum */
{ &tIPTunnelObjectInfo[eIPTunnel0],	tIPTunnelEntityLeaf,	tIPTunnelEntityObject,	NULL,		0 },
};


int getIPTunnelEntity(char *name, struct CWMP_LEAF *entity, int *type,
			void **data)
{
	char *lastname = entity->info->name;
	char buf[256];

	struct ip46_tunnel_parm ai46tp[8];
	int nr, ret;
	unsigned int instNum;
	struct ip46_tunnel_parm *pi46tp;

	if (name == NULL || entity == NULL || type == NULL || data == NULL)
		return -1;

	nr = do_tunnels_list(ai46tp, sizeof(ai46tp) / sizeof(ai46tp[0]));
	sscanf(name, "InternetGatewayDevice.X_TELEFONICA-ES_IPTunnel.%u.%*s", &instNum);
	pi46tp = ai46tp + instNum - 1;

	*type = entity->info->type;
	*data = NULL;
	if (strcmp(lastname, "Activated") == 0) {
		int flags;

		ret = getInFlags(pi46tp->uitp.p4.name, &flags);
		*data = booldup(ret && flags & IFF_UP);
	} else if (strcmp(lastname, "TunnelName") == 0) {
		*data = strdup(pi46tp->uitp.p4.name);
	} else if (strcmp(lastname, "TunnelCounter") == 0) {
		*data = intdup(1);
	} else if (strcmp(lastname, "Mode") == 0) {
		switch (pi46tp->family == AF_INET ? 
				pi46tp->uitp.p4.iph.protocol : pi46tp->uitp.p6.proto) {
		case IPPROTO_IPIP:
			strcpy(buf, "4");
			break;
		case IPPROTO_GRE:
			strcpy(buf, "GRE");
			break;
		case IPPROTO_IPV6:
			strcpy(buf, "6");
			break;
		default:
			strcpy(buf, "?");
			break;
		}

		strcat(buf, pi46tp->family == AF_INET ? "in4" : "in6");

		*data = strdup(buf);
	} else if (strcmp(lastname, "AssociatedWanIfName") == 0) {
		const char *str;
		
		str = tnl_ioctl_get_ifname(pi46tp->uitp.p4.link, pi46tp->family);
		*data = strdup(str ? str : "");
	} else if (strcmp(lastname, "AssociatedLanIfName") == 0) {
		*data = strdup("br0");
	} else {
		return ERR_9005;
	}

	return 0;
}

int setIPTunnelEntity(char *name, struct CWMP_LEAF *entity, int type,
			void *data)
{
	char *lastname = entity->info->name;
	int *pInt = data;
	char *buf = data;

	struct ip46_tunnel_parm ai46tp[8];
	int nr, ret;
	unsigned int instNum;
	struct ip46_tunnel_parm *pi46tp;

	if (name == NULL || entity == NULL || data == NULL)
		return -1;

	if (entity->info->type != type)
		return ERR_9006;

	nr = do_tunnels_list(ai46tp, sizeof(ai46tp) / sizeof(ai46tp[0]));
	sscanf(name, "InternetGatewayDevice.X_TELEFONICA-ES_IPTunnel.%u.%*s", &instNum);
	pi46tp = ai46tp + instNum - 1;

	if (strcmp(lastname, "Activated") == 0) {
		int flags;

		ret = getInFlags(pi46tp->uitp.p4.name, &flags);
		if (ret) {
			flags = *pInt ? flags | IFF_UP : flags & ~IFF_UP;
		} else {
			return ERR_9003;
		}
		ret = setInFlags(pi46tp->uitp.p4.name, flags);

		if (!ret)
			return ERR_9002;
	} else if (strcmp(lastname, "TunnelName") == 0) {
		const char *basedev;

		ret = tnl_ioctl(SIOCDELTUNNEL, pi46tp->uitp.p4.name, &pi46tp->uitp, pi46tp->family);
		if (ret)
			return ERR_9003;

		/* 
		 * construct the base device name
		 */
		if (pi46tp->family == AF_INET) {
			switch (pi46tp->uitp.p4.iph.protocol) {
			case IPPROTO_IPIP:
			default:
				basedev = basedevs[tunl0];
				break;
			case IPPROTO_GRE:
				basedev = basedevs[gre0];
				break;
			case IPPROTO_IPV6:
				basedev = basedevs[sit0];
				break;
			}
		} else {
			basedev = basedevs[ip6tnl0];
		}

		/* copy the new name */
		strncpy(pi46tp->uitp.p4.name, buf, IFNAMSIZ);
		pi46tp->uitp.p4.name[IFNAMSIZ - 1] = '\0';

		ret = tnl_ioctl(SIOCADDTUNNEL, basedev, &pi46tp->uitp, pi46tp->family);
		if (ret)
			return ERR_9002;
	} else if (strcmp(lastname, "TunnelCounter") == 0) {
		return ERR_9008;
	} else if (strcmp(lastname, "Mode") == 0) {
		struct ifreq *pifr;
		int inner, outer;

		sscanf(buf, "%uin%u", &inner, &outer);

		pifr = tnl_ioctl_get_ifreq(SIOCGIFHWADDR, pi46tp->uitp.p4.name, pi46tp->family);

		if (pifr == NULL)
			return ERR_9003;

		if (outer == 4) {
			pi46tp->family = AF_INET;

			if (inner == 4) {
				pi46tp->uitp.p4.iph.protocol = IPPROTO_IPIP;
				pifr->ifr_addr.sa_family = ARPHRD_TUNNEL;
			} else {
				pi46tp->uitp.p4.iph.protocol = IPPROTO_IPV6;
				pifr->ifr_addr.sa_family = ARPHRD_SIT;
			}

			pifr->ifr_addr.sa_family;
		} else {
			pi46tp->family = AF_INET6;

			pi46tp->uitp.p6.proto = inner == 4 ? IPPROTO_IPIP : IPPROTO_IPV6;
			pifr->ifr_addr.sa_family = ARPHRD_TUNNEL6;
		}

		ret = tnl_ioctl_set_ifreq(SIOCSIFHWADDR, pifr, pi46tp->family);
		if (ret)
			return ERR_9002;

		ret = tnl_ioctl(SIOCCHGTUNNEL, pi46tp->uitp.p4.name, 
				&pi46tp->uitp, pi46tp->family);
		if (ret)
			return ERR_9002;
	} else if (strcmp(lastname, "AssociatedWanIfName") == 0) {
		struct ifreq *pifr;

		pifr = tnl_ioctl_get_ifreq(SIOCGIFINDEX, buf, pi46tp->family);

		if (pifr == NULL)
			return ERR_9003;

		pi46tp->uitp.p4.link = pifr->ifr_ifindex;

		ret = tnl_ioctl(SIOCCHGTUNNEL, pi46tp->uitp.p4.name, 
				&pi46tp->uitp, pi46tp->family);
		if (ret)
			return ERR_9002;
	} else if (strcmp(lastname, "AssociatedLanIfName") == 0) {
		/* ignored */
	} else {
		return ERR_9005;
	}

	return 0;
}

int get4in6TunnelEntity(char *name, struct CWMP_LEAF *entity, int *type,
			void **data)
{
	char *lastname = entity->info->name;
	char buf[256];

	struct ip46_tunnel_parm ai46tp[8];
	int nr, ret;
	unsigned int instNum;
	struct ip46_tunnel_parm *pi46tp;

	if (name == NULL || entity == NULL || type == NULL || data == NULL)
		return -1;

	nr = do_tunnels_list(ai46tp, sizeof(ai46tp) / sizeof(ai46tp[0]));
	sscanf(name, "InternetGatewayDevice.X_TELEFONICA-ES_IPTunnel.%u.4in6Tunnel.1.%*s", &instNum);
	pi46tp = ai46tp + instNum - 1;

	*type = entity->info->type;
	*data = NULL;
	if (strcmp(lastname, "ConnStatus") == 0) {
		int flags;

		ret = getInFlags(pi46tp->uitp.p4.name, &flags);
		*data = strdup((ret && flags & IFF_RUNNING) ? "Connected" : "Disconnected");
	} else if (strcmp(lastname, "Mechanism") == 0) {
		*data = strdup("DualStackLite");
	} else if (strcmp(lastname, "Dynamic") == 0) {
		/* always static for now */
		*data = booldup(0);
	} else if (strcmp(lastname, "RemoteIpv6Address") == 0) {
		inet_ntop(AF_INET6, &pi46tp->uitp.p6.raddr, buf, sizeof(buf));
		*data = strdup(buf);
	} else {
		return ERR_9005;
	}

	return 0;
}

int set4in6TunnelEntity(char *name, struct CWMP_LEAF *entity, int type,
			void *data)
{
	char *lastname = entity->info->name;
	int *pInt = data;
	char *buf = data;

	struct ip46_tunnel_parm ai46tp[8];
	int nr, ret;
	unsigned int instNum;
	struct ip46_tunnel_parm *pi46tp;

	if (name == NULL || entity == NULL || data == NULL)
		return -1;

	if (entity->info->type != type)
		return ERR_9006;

	nr = do_tunnels_list(ai46tp, sizeof(ai46tp) / sizeof(ai46tp[0]));
	sscanf(name, "InternetGatewayDevice.X_TELEFONICA-ES_IPTunnel.%u.4in6Tunnel.1.%*s", &instNum);
	pi46tp = ai46tp + instNum - 1;

	if (strcmp(lastname, "ConnStatus") == 0) {
		return ERR_9008;
	} else if (strcmp(lastname, "Mechanism") == 0) {
		/* ignored */
	} else if (strcmp(lastname, "Dynamic") == 0) {
		/* ignored */
	} else if (strcmp(lastname, "RemoteIpv6Address") == 0) {
		inet_pton(AF_INET6, buf, &pi46tp->uitp.p6.raddr);
	} else {
		return ERR_9005;
	}

	ret = tnl_ioctl(SIOCCHGTUNNEL, pi46tp->uitp.p4.name, 
			&pi46tp->uitp, pi46tp->family);
	if (ret)
		return ERR_9002;

	return 0;
}

int get6in4TunnelEntity(char *name, struct CWMP_LEAF *entity, int *type,
			void **data)
{
	char *lastname = entity->info->name;
	int vInt;
	char buf[256];

	struct ip46_tunnel_parm ai46tp[8];
	int nr, ret;
	unsigned int instNum;
	struct ip46_tunnel_parm *pi46tp;
	struct ip_tunnel_6rd ip6rd;

	if (name == NULL || entity == NULL || type == NULL || data == NULL)
		return -1;

	nr = do_tunnels_list(ai46tp, sizeof(ai46tp) / sizeof(ai46tp[0]));
	sscanf(name, "InternetGatewayDevice.X_TELEFONICA-ES_IPTunnel.%u.6in4Tunnel.1.%*s", &instNum);
	pi46tp = ai46tp + instNum - 1;
	ret = tnl_ioctl(SIOCGET6RD, pi46tp->uitp.p4.name, &ip6rd, pi46tp->family);
	if (ret)
		return ERR_9002;

	*type = entity->info->type;
	*data = NULL;
	if (strcmp(lastname, "ConnStatus") == 0) {
		int flags;

		ret = getInFlags(pi46tp->uitp.p4.name, &flags);
		*data = strdup((ret && flags & IFF_RUNNING) ? "Connected" : "Disconnected");
	} else if (strcmp(lastname, "Mechanism") == 0) {
		/* always 6rd for now */
		*data = strdup("Ipv6RapidDeployment");
	} else if (strcmp(lastname, "Dynamic") == 0) {
		/* always static for now */
		*data = booldup(0);
	} else if (strcmp(lastname, "IPv4MaskLen") == 0) {
		*data = intdup(ip6rd.relay_prefixlen);
	} else if (strcmp(lastname, "Prefix") == 0) {
		char tmp_buf[128];

		inet_ntop(AF_INET6, &ip6rd.prefix, buf, sizeof(buf));
		snprintf(tmp_buf, sizeof(tmp_buf), "/%u", ip6rd.prefixlen);
		strncat(buf, tmp_buf, sizeof(buf));

		*data = strdup(buf);
	} else if (strcmp(lastname, "BorderRelayAddress") == 0) {
		/* to be fixed */
		inet_ntop(AF_INET, &ip6rd.relay_prefix, buf, sizeof(buf));
		*data = strdup(buf);
	} else {
		return ERR_9005;
	}

	return 0;
}

int set6in4TunnelEntity(char *name, struct CWMP_LEAF *entity, int type,
			void *data)
{
	char *lastname = entity->info->name;
	int *pInt = data;
	char *buf = data;

	struct ip46_tunnel_parm ai46tp[8];
	int nr, ret;
	unsigned int instNum;
	struct ip46_tunnel_parm *pi46tp;
	struct ip_tunnel_6rd ip6rd;

	if (name == NULL || entity == NULL || data == NULL)
		return -1;

	if (entity->info->type != type)
		return ERR_9006;

	nr = do_tunnels_list(ai46tp, sizeof(ai46tp) / sizeof(ai46tp[0]));
	sscanf(name, "InternetGatewayDevice.X_TELEFONICA-ES_IPTunnel.%u.6in4Tunnel.1.%*s", &instNum);
	pi46tp = ai46tp + instNum - 1;
	ret = tnl_ioctl(SIOCGET6RD, pi46tp->uitp.p4.name, &ip6rd, pi46tp->family);
	if (ret)
		return ERR_9002;

	if (strcmp(lastname, "ConnStatus") == 0) {
		return ERR_9008;
	} else if (strcmp(lastname, "Mechanism") == 0) {
		/* ignored */
	} else if (strcmp(lastname, "Dynamic") == 0) {
		/* ignored */
	} else if (strcmp(lastname, "IPv4MaskLen") == 0) {
		if (*pInt < 0)
			return ERR_9003;
		ip6rd.relay_prefixlen = *pInt;
	} else if (strcmp(lastname, "Prefix") == 0) {
		char *ptr;

		ptr = strrchr(buf, '/');

		if (ptr) {
			*ptr++ = '\0';
			ip6rd.prefixlen = strtoul(ptr, NULL, 10);
		}
		inet_pton(AF_INET6, buf, &ip6rd.prefix);
	} else if (strcmp(lastname, "BorderRelayAddress") == 0) {
		/* to be fixed */
		ret = inet_pton(AF_INET, buf, &ip6rd.relay_prefix);
		if (ret != 1)
			return ERR_9002;
	} else {
		return ERR_9005;
	}

	ret = tnl_ioctl(SIOCCHG6RD, pi46tp->uitp.p4.name, &ip6rd, pi46tp->family);
	if (ret)
		return ERR_9002;

	return 0;
}

int obj4in6Tunnel(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	int ret = 0;
	struct CWMP_NODE *entity = (struct CWMP_NODE *)e;
	unsigned int i;

	struct ip46_tunnel_parm ai46tp[8];
	int nr;
	unsigned int instNum;
	struct ip46_tunnel_parm *pi46tp;

	if (name == NULL || entity == NULL)
		return -1;

	nr = do_tunnels_list(ai46tp, sizeof(ai46tp) / sizeof(ai46tp[0]));
	sscanf(name, "InternetGatewayDevice.X_TELEFONICA-ES_IPTunnel.%u.4in6Tunnel.", &instNum);
	pi46tp = ai46tp + instNum - 1;
	if (pi46tp->family == AF_INET6 && pi46tp->uitp.p6.proto == IPPROTO_IPIP)
		nr = 1;
	else
		nr = 0;

	switch (type) {
	case eCWMP_tINITOBJ:
		if (data == NULL)
			return -1;

		if (create_Object(data, t4in6TunnelObject,
				  sizeof(t4in6TunnelObject), nr,
				  1) < 0)
			return -1;

		ret = add_objectNum(name, nr);

		break;
	case eCWMP_tADDOBJ:
		return -1;
	case eCWMP_tDELOBJ:
		return -1;
	case eCWMP_tUPDATEOBJ:{
			struct CWMP_LINKNODE *old_table;

			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;

			for (i = 1; i <= nr; i++) {
				struct CWMP_LINKNODE *remove_entity = NULL;

				remove_entity =
				    remove_SiblingEntity(&old_table, i);

				if (remove_entity != NULL) {
					/* it exists both in the System and the parameter tree */
					add_SiblingEntity((struct CWMP_LINKNODE
							   **)&entity->next,
							  remove_entity);
				} else {
					/* it exists only in the System,
					 * so we add it into the parameter tree */
					instNum = i;
					add_Object(name,
						   (struct CWMP_LINKNODE **)
						   &entity->next,
						   t4in6TunnelObject,
						   sizeof(t4in6TunnelObject),
						   &instNum);
				}
			}

			if (old_table) {
				/* it exists only in the parameter tree,
				 * so we remove it from the parameter tree */
				destroy_ParameterTable((struct CWMP_NODE *)
						       old_table);
			}

			break;
		}
	}

	return ret;
}

int obj6in4Tunnel(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	int ret = 0;
	struct CWMP_NODE *entity = (struct CWMP_NODE *)e;
	unsigned int i;

	struct ip46_tunnel_parm ai46tp[8];
	int nr;
	unsigned int instNum;
	struct ip46_tunnel_parm *pi46tp;

	if (name == NULL || entity == NULL)
		return -1;

	nr = do_tunnels_list(ai46tp, sizeof(ai46tp) / sizeof(ai46tp[0]));
	sscanf(name, "InternetGatewayDevice.X_TELEFONICA-ES_IPTunnel.%u.6in4Tunnel.", &instNum);
	pi46tp = ai46tp + instNum - 1;
	if (pi46tp->family == AF_INET && pi46tp->uitp.p4.iph.protocol == IPPROTO_IPV6)
		nr = 1;
	else
		nr = 0;

	switch (type) {
	case eCWMP_tINITOBJ:
		if (data == NULL)
			return -1;

		if (create_Object(data, t6in4TunnelObject,
				  sizeof(t6in4TunnelObject), nr,
				  1) < 0)
			return -1;

		ret = add_objectNum(name, nr);

		break;
	case eCWMP_tADDOBJ:
		return -1;
	case eCWMP_tDELOBJ:
		return -1;
	case eCWMP_tUPDATEOBJ:{
			struct CWMP_LINKNODE *old_table;

			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;

			for (i = 1; i <= nr; i++) {
				struct CWMP_LINKNODE *remove_entity = NULL;

				remove_entity =
				    remove_SiblingEntity(&old_table, i);

				if (remove_entity != NULL) {
					/* it exists both in the System and the parameter tree */
					add_SiblingEntity((struct CWMP_LINKNODE
							   **)&entity->next,
							  remove_entity);
				} else {
					/* it exists only in the System,
					 * so we add it into the parameter tree */
					instNum = i;
					add_Object(name,
						   (struct CWMP_LINKNODE **)
						   &entity->next,
						   t6in4TunnelObject,
						   sizeof(t6in4TunnelObject),
						   &instNum);
				}
			}

			if (old_table) {
				/* it exists only in the parameter tree,
				 * so we remove it from the parameter tree */
				destroy_ParameterTable((struct CWMP_NODE *)
						       old_table);
			}

			break;
		}
	}

	return ret;
}

int objIPTunnel(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	int ret = 0;
	struct CWMP_NODE *entity = (struct CWMP_NODE *)e;
	unsigned int i;

	struct ip46_tunnel_parm ai46tp[8];
	int nr;
	unsigned int instNum;

	if (name == NULL || entity == NULL)
		return -1;

	nr = do_tunnels_list(ai46tp, sizeof(ai46tp) / sizeof(ai46tp[0]));

	switch (type) {
	case eCWMP_tINITOBJ:
		if (data == NULL)
			return -1;

		if (create_Object(data, tIPTunnelObject,
				  sizeof(tIPTunnelObject), nr, 1) < 0)
			return -1;

		ret = add_objectNum(name, nr);

		break;
	case eCWMP_tADDOBJ:{
			struct ip46_tunnel_parm i46tp;

			if (data == NULL)
				return -1;

			if (add_Object
			    (name, (struct CWMP_LINKNODE **)&entity->next,
			     tIPTunnelObject, sizeof(tIPTunnelObject),
			     data) < 0)
				return -1;

			memset(&i46tp, 0, sizeof(i46tp));
			i46tp.family = AF_INET;
			i46tp.uitp.p4.iph.version = 4;
			i46tp.uitp.p4.iph.ihl = 5;
			/* Flag: "Don't Fragment" */
			i46tp.uitp.p4.iph.frag_off = htons(0x4000);
			i46tp.uitp.p4.iph.protocol = IPPROTO_IPV6;
			/* tunnels with the same mode can not have same local/remote pairs */
			i46tp.uitp.p4.iph.daddr = *(int *)data;

			ret = tnl_ioctl(SIOCADDTUNNEL, basedevs[sit0],
					&i46tp.uitp, i46tp.family);

			break;
		}
	case eCWMP_tDELOBJ:{
			struct ip46_tunnel_parm *pi46tp;

			if (data == NULL)
				return -1;

			if (del_Object(name, (struct CWMP_LINKNODE **)
				       &entity->next, *(int *)data) < 0)
				return -1;

			pi46tp = ai46tp + *(int *)data - 1;
			ret = tnl_ioctl(SIOCDELTUNNEL, pi46tp->uitp.p4.name,
					&pi46tp->uitp, pi46tp->family);

			break;
		}
	case eCWMP_tUPDATEOBJ:{
			struct CWMP_LINKNODE *old_table;

			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;

			for (i = 1; i <= nr; i++) {
				struct CWMP_LINKNODE *remove_entity = NULL;

				remove_entity =
				    remove_SiblingEntity(&old_table, i);

				if (remove_entity != NULL) {
					/* it exists both in the System and the parameter tree */
					add_SiblingEntity((struct CWMP_LINKNODE
							   **)&entity->next,
							  remove_entity);
				} else {
					/* it exists only in the System,
					 * so we add it into the parameter tree */
					instNum = i;
					add_Object(name,
						   (struct CWMP_LINKNODE **)
						   &entity->next,
						   tIPTunnelObject,
						   sizeof(tIPTunnelObject),
						   &instNum);
				}
			}

			if (old_table) {
				/* it exists only in the parameter tree,
				 * so we remove it from the parameter tree */
				destroy_ParameterTable((struct CWMP_NODE *)
						       old_table);
			}

			break;
		}
	}

	return ret;
}


struct CWMP_OP tIPv6Layer3ForwardingLeafOP = { getIPv6Layer3Forwarding, setIPv6Layer3Forwarding };

struct CWMP_PRMT tIPv6Layer3ForwardingLeafInfo[] =
{
/* name,			type,		flag,			op */
{"DefaultConnectionService",	eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	&tIPv6Layer3ForwardingLeafOP},
{"ForwardNumberOfEntries",	eCWMP_tUINT,	CWMP_READ | CWMP_WRITE,	&tIPv6Layer3ForwardingLeafOP},
};

enum eIPv6Layer3ForwardingLeaf
{
	eDefaultConnectionService,
	eForwardNumberOfEntries,
};

/* InternetGatewayDevice.X_TELEFONICA-ES_IPv6Layer3Forwarding.* */
struct CWMP_LEAF tIPv6Layer3ForwardingLeaf[] =
{
{ &tIPv6Layer3ForwardingLeafInfo[eDefaultConnectionService] },
{ &tIPv6Layer3ForwardingLeafInfo[eForwardNumberOfEntries] },
{ NULL },
};


struct CWMP_OP tIPv6ForwardingEntityOP = { getIPv6ForwardingEntity, setIPv6ForwardingEntity };

struct CWMP_PRMT tIPv6ForwardingEntityLeafInfo[] =
{
/* name,		type,		flag,			op */
{"Enable",		eCWMP_tBOOLEAN,	CWMP_READ | CWMP_WRITE,	&tIPv6ForwardingEntityOP},
{"Status",		eCWMP_tSTRING,	CWMP_READ,		&tIPv6ForwardingEntityOP},
{"Type",		eCWMP_tSTRING,	CWMP_READ,		&tIPv6ForwardingEntityOP},
{"DestIPv6Address",	eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	&tIPv6ForwardingEntityOP},
{"SourceIPv6Address",	eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	&tIPv6ForwardingEntityOP},
{"SourceIfName",	eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	&tIPv6ForwardingEntityOP},
{"GatewayIPv6Address",	eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	&tIPv6ForwardingEntityOP},
{"Interface",		eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	&tIPv6ForwardingEntityOP},
{"ForwardingMetric",	eCWMP_tINT,	CWMP_READ | CWMP_WRITE,	&tIPv6ForwardingEntityOP},
{"MTU",			eCWMP_tINT,	CWMP_READ | CWMP_WRITE,	&tIPv6ForwardingEntityOP},
};

enum eIPv6ForwardingEntityLeaf
{
	eEnable,
	eStatus,
	eType,
	eDestIPv6Address,
	eSourceIPv6Address,
	eSourceIfName,
	eGatewayIPv6Address,
	eInterface,
	eForwardingMetric,
	eMTU,
};

/* InternetGatewayDevice.X_TELEFONICA-ES_IPv6Layer3Forwarding.IPv6Forwarding.{i}.* */
struct CWMP_LEAF tIPv6ForwardingEntityLeaf[] =
{
{ &tIPv6ForwardingEntityLeafInfo[eEnable] },
{ &tIPv6ForwardingEntityLeafInfo[eStatus] },
{ &tIPv6ForwardingEntityLeafInfo[eType] },
{ &tIPv6ForwardingEntityLeafInfo[eDestIPv6Address] },
{ &tIPv6ForwardingEntityLeafInfo[eSourceIPv6Address] },
{ &tIPv6ForwardingEntityLeafInfo[eSourceIfName] },
{ &tIPv6ForwardingEntityLeafInfo[eGatewayIPv6Address] },
{ &tIPv6ForwardingEntityLeafInfo[eInterface] },
{ &tIPv6ForwardingEntityLeafInfo[eForwardingMetric] },
{ &tIPv6ForwardingEntityLeafInfo[eMTU] },
{ NULL },
};


struct CWMP_PRMT tIPv6ForwardingObjectInfo[] =
{
/* name,	type,		flag,					op */
{"0",		eCWMP_tOBJECT,	CWMP_READ | CWMP_WRITE | CWMP_LNKLIST,	NULL},
};

enum eIPv6ForwardingObject
{
	eIPv6Forwarding0,
};

/* InternetGatewayDevice.X_TELEFONICA-ES_IPv6Layer3Forwarding.IPv6Forwarding.{i}. */
struct CWMP_LINKNODE tIPv6ForwardingObject[] =
{
/* info,					leaf,				next,		sibling,	instnum */
{ &tIPv6ForwardingObjectInfo[eIPv6Forwarding0],	tIPv6ForwardingEntityLeaf,	NULL,		NULL,		0 },
};


struct CWMP_OP tIPv6ForwardingOP = { NULL, objIPv6Forwarding };

struct CWMP_PRMT tIPv6Layer3ForwardingObjectInfo[] =
{
/* name,		type,		flag,			op */
{"IPv6Forwarding",	eCWMP_tOBJECT,	CWMP_READ | CWMP_WRITE,	&tIPv6ForwardingOP},
};

enum eIPv6Layer3ForwardingObject
{
	eIPv6Forwarding,
};

/* InternetGatewayDevice.X_TELEFONICA-ES_IPv6Layer3Forwarding.*. */
struct CWMP_NODE tIPv6Layer3ForwardingObject[] =
{
/* info,						leaf,		next */
{ &tIPv6Layer3ForwardingObjectInfo[eIPv6Forwarding],	NULL,		NULL },
{ NULL,	NULL, NULL },
};


int getIPv6Layer3Forwarding(char *name, struct CWMP_LEAF *entity, int *type,
			void **data)
{
	char *lastname = entity->info->name;
	unsigned int vUint;
	char buf[256];

	if (name == NULL || entity == NULL || type == NULL || data == NULL)
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if (strcmp(lastname, "DefaultConnectionService") == 0) {
		*data = strdup(buf);
	} else if (strcmp(lastname, "ForwardNumberOfEntries") == 0) {
		*data = uintdup(vUint);
	} else {
		return ERR_9005;
	}

	return 0;
}

int setIPv6Layer3Forwarding(char *name, struct CWMP_LEAF *entity, int type,
			void *data)
{
	char *lastname = entity->info->name;
	unsigned int *pUint;
	char *buf = data;

	if (name == NULL || entity == NULL || data == NULL)
		return -1;

	if (entity->info->type != type)
		return ERR_9006;

	if (strcmp(lastname, "DefaultConnectionService") == 0) {
	} else if (strcmp(lastname, "ForwardNumberOfEntries") == 0) {
	} else {
		return ERR_9005;
	}

	return 0;
}

int getIPv6ForwardingEntity(char *name, struct CWMP_LEAF *entity, int *type,
			void **data)
{
	char *lastname = entity->info->name;
	int vInt;
	char buf[256];

	if (name == NULL || entity == NULL || type == NULL || data == NULL)
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if (strcmp(lastname, "Enable") == 0) {
		*data = booldup(vInt != 0);
	} else if (strcmp(lastname, "Status") == 0) {
		*data = strdup(buf);
	} else if (strcmp(lastname, "Type") == 0) {
		*data = strdup(buf);
	} else if (strcmp(lastname, "DestIPv6Address") == 0) {
		*data = strdup(buf);
	} else if (strcmp(lastname, "SourceIPv6Address") == 0) {
		*data = strdup(buf);
	} else if (strcmp(lastname, "SourceIfName") == 0) {
		*data = strdup(buf);
	} else if (strcmp(lastname, "GatewayIPv6Address") == 0) {
		*data = strdup(buf);
	} else if (strcmp(lastname, "Interface") == 0) {
		*data = strdup(buf);
	} else if (strcmp(lastname, "ForwardingMetric") == 0) {
		*data = intdup(vInt);
	} else if (strcmp(lastname, "MTU") == 0) {
		*data = intdup(vInt);
	} else {
		return ERR_9005;
	}

	return 0;
}

int setIPv6ForwardingEntity(char *name, struct CWMP_LEAF *entity, int type,
			void *data)
{
	char *lastname = entity->info->name;
	int *pInt = data;
	char *buf = data;

	if (name == NULL || entity == NULL || data == NULL)
		return -1;

	if (entity->info->type != type)
		return ERR_9006;

	if (strcmp(lastname, "Enable") == 0) {
	} else if (strcmp(lastname, "Status") == 0) {
	} else if (strcmp(lastname, "Type") == 0) {
	} else if (strcmp(lastname, "DestIPv6Address") == 0) {
	} else if (strcmp(lastname, "SourceIPv6Address") == 0) {
	} else if (strcmp(lastname, "SourceIfName") == 0) {
	} else if (strcmp(lastname, "GatewayIPv6Address") == 0) {
	} else if (strcmp(lastname, "Interface") == 0) {
	} else if (strcmp(lastname, "ForwardingMetric") == 0) {
	} else if (strcmp(lastname, "MTU") == 0) {
	} else {
		return ERR_9005;
	}

	return 0;
}

int objIPv6Forwarding(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	int ret = 0;
	struct CWMP_NODE *entity = (struct CWMP_NODE *)e;
	unsigned int i, num, maxInstNum = 0;

	if (name == NULL || entity == NULL)
		return -1;

	switch (type) {
	case eCWMP_tINITOBJ:
		if (data == NULL)
			return -1;

		if (create_Object(data, tIPv6ForwardingObject,
				  sizeof(tIPv6ForwardingObject), 1,
				  ++maxInstNum) < 0)
			return -1;

		add_objectNum(name, maxInstNum);

		break;
	case eCWMP_tADDOBJ:
		if (data == NULL)
			return -1;

		if (add_Object(name,
			       (struct CWMP_LINKNODE **)&entity->next,
			       tIPv6ForwardingObject,
			       sizeof(tIPv6ForwardingObject), data) < 0)
			return -1;

		break;
	case eCWMP_tDELOBJ:
		if (data == NULL)
			return -1;

		if (del_Object(name, (struct CWMP_LINKNODE **)
			       &entity->next, *(int *)data) < 0)
			return -1;

		break;
	case eCWMP_tUPDATEOBJ:
		{
			return -1;

			struct CWMP_LINKNODE *old_table;

			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;

			for (i = 0; i < num; i++) {
				struct CWMP_LINKNODE *remove_entity = NULL;

				remove_entity =
				    remove_SiblingEntity(&old_table, i);

				if (remove_entity != NULL) {
					/* it exists both in the MIB and the parameter tree */
					add_SiblingEntity((struct CWMP_LINKNODE
							   **)&entity->next,
							  remove_entity);
				} else {
					/* it exists only in the MIB,
					 * so we add it into the parameter tree */
					add_Object(name,
						   (struct CWMP_LINKNODE **)
						   &entity->next,
						   tIPv6ForwardingObject,
						   sizeof
						   (tIPv6ForwardingObject), &i);
					/*mib_chain_update(CWMP_CT_ALARM_TBL, */
					/*pAlarm, i); */
				}
			}

			if (old_table) {
				/* it exists only in the parameter tree,
				 * so we remove it from the parameter tree */
				destroy_ParameterTable((struct CWMP_NODE *)
						       old_table);
			}

			break;
		}
	}

	return ret;
}



/*****************************************************************************
 * Telefonica IPv6 LAN Parameters
 *****************************************************************************/

struct CWMP_OP tIPv6LANHostConfigManagementLeafOP = { getIPv6LANHostConfigManagement, setIPv6LANHostConfigManagement };

struct CWMP_PRMT tIPv6LANHostConfigManagementLeafInfo[] =
{
/* name,			 type,		 flag,			 op */
{"AutoConfigurationAddress",	 eCWMP_tBOOLEAN, CWMP_READ | CWMP_WRITE, &tIPv6LANHostConfigManagementLeafOP},
{"IPv6DNSWanConnection",	 eCWMP_tSTRING,  CWMP_READ | CWMP_WRITE, &tIPv6LANHostConfigManagementLeafOP},
};

enum eIPv6LANHostConfigManagementLeaf
{
 eAutoConfigurationAddress,
 eIPv6DNSWanConnection,
};

/* InternetGatewayDevice.LANDevice.{i}.X_TELEFONICA-ES_IPv6LANHostConfigManagement.* */
struct CWMP_LEAF tIPv6LANHostConfigManagementLeaf[] =
{
/* info */
{ &tIPv6LANHostConfigManagementLeafInfo[eAutoConfigurationAddress]},
{ &tIPv6LANHostConfigManagementLeafInfo[eIPv6DNSWanConnection]},
{ NULL },
};


struct CWMP_OP tRADVDLeafOP = { getRADVDLeaf, setRADVDLeaf };

struct CWMP_PRMT tRADVDLeafInfo[] =
{
/* name,	 type,		 flag,			 op */
{"Enabled", eCWMP_tBOOLEAN, CWMP_READ | CWMP_WRITE, &tRADVDLeafOP},
};

enum eRADVDLeaf
{
 eEnabled,
};

/* InternetGatewayDevice.LANDevice.{i}.X_TELEFONICA-ES_IPv6LANHostConfigManagement.RADVD.* */
struct CWMP_LEAF tRADVDLeaf[] =
{
/* info */
{ &tRADVDLeafInfo[eEnabled]},
{ NULL },
};


struct CWMP_OP tIPv6SitePrefixConfigTypeLeafOP = { getIPv6SitePrefixConfigType, setIPv6SitePrefixConfigType };

struct CWMP_PRMT tIPv6SitePrefixConfigTypeLeafInfo[] =
{
/* name,		 type,		 flag,			 op */
{"StaticDelegated", eCWMP_tBOOLEAN, CWMP_READ | CWMP_WRITE, &tIPv6SitePrefixConfigTypeLeafOP},
};

enum eIPv6SitePrefixConfigTypeLeaf
{
 eStaticDelegated,
};

/* InternetGatewayDevice.LANDevice.{i}.X_TELEFONICA-ES_IPv6LANHostConfigManagement.IPv6SitePrefixConfigType.* */
struct CWMP_LEAF tIPv6SitePrefixConfigTypeLeaf[] =
{
/* info */
{ &tIPv6SitePrefixConfigTypeLeafInfo[eStaticDelegated]},
{ NULL },
};


struct CWMP_OP tIPv6PDWanConnectionLeafOP = { getIPv6PDWanConnection, setIPv6PDWanConnection };

struct CWMP_PRMT tIPv6PDWanConnectionLeafInfo[] =
{
/* name,	 type,		 flag,			 op */
{"Interface",	 eCWMP_tSTRING,  CWMP_READ | CWMP_WRITE, &tIPv6PDWanConnectionLeafOP},
};

enum eIPv6PDWanConnectionLeaf
{
	ePDWan_Interface,
};

/* InternetGatewayDevice.LANDevice.{i}.X_TELEFONICA-ES_IPv6LANHostConfigManagement.IPv6PDWanConnection.* */
struct CWMP_LEAF tIPv6PDWanConnectionLeaf[] =
{
/* info */
{ &tIPv6PDWanConnectionLeafInfo[ePDWan_Interface]},
{ NULL },
};


struct CWMP_OP tIPv6InterfaceNumberOfEntriesLeafOP = { getIPv6InterfaceNumberOfEntries, setIPv6InterfaceNumberOfEntries };

struct CWMP_PRMT tIPv6InterfaceNumberOfEntriesLeafInfo[] =
{
/* name,	 type,		 flag,			 op */
{"Nentries",	 eCWMP_tUINT,	 CWMP_READ | CWMP_WRITE, &tIPv6InterfaceNumberOfEntriesLeafOP},
};

enum eIPv6InterfaceNumberOfEntriesLeaf
{
 eNentries,
};

/* InternetGatewayDevice.LANDevice.{i}.X_TELEFONICA-ES_IPv6LANHostConfigManagement.IPv6InterfaceNumberOfEntries.* */
struct CWMP_LEAF tIPv6InterfaceNumberOfEntriesLeaf[] =
{
/* info */
{ &tIPv6InterfaceNumberOfEntriesLeafInfo[eNentries]},
{ NULL },
};


struct CWMP_OP tULAPrefixInfoLeafOP = { getULAPrefixInfo, setULAPrefixInfo };

struct CWMP_PRMT tULAPrefixInfoLeafInfo[] =
{
/* name,		 type,		 flag,			 op */
{"Enable", 	 eCWMP_tBOOLEAN, CWMP_READ | CWMP_WRITE, &tULAPrefixInfoLeafOP},
{"Prefix", 	 eCWMP_tSTRING,  CWMP_READ | CWMP_WRITE, &tULAPrefixInfoLeafOP},
{"PreferredLifeTime",	 eCWMP_tINT, CWMP_READ | CWMP_WRITE, &tULAPrefixInfoLeafOP},
{"ValidLifeTime",	 eCWMP_tINT, CWMP_READ | CWMP_WRITE, &tULAPrefixInfoLeafOP},
};

enum eULAPrefixInfoLeaf
{
 eULAPrefix_Enable,
 eULAPrefix_Prefix,
 eULAPrefix_PreferredLifeTime,
 eULAPrefix_ValidLifeTime,
};

/* InternetGatewayDevice.LANDevice.{i}.X_TELEFONICA-ES_IPv6LANHostConfigManagement.RadVDConfigManagement.ULAPrefixInfo.* */
struct CWMP_LEAF tULAPrefixInfoLeaf[] =
{
/* info */
{ &tULAPrefixInfoLeafInfo[eULAPrefix_Enable]},
{ &tULAPrefixInfoLeafInfo[eULAPrefix_Prefix]},
{ &tULAPrefixInfoLeafInfo[eULAPrefix_PreferredLifeTime]},
{ &tULAPrefixInfoLeafInfo[eULAPrefix_ValidLifeTime]},
{ NULL },
};


struct CWMP_OP tPrefixInformationEntityLeafOP = { getPrefixInformationEntity, setPrefixInformationEntity };

struct CWMP_PRMT tPrefixInformationEntityLeafInfo[] =
{
/* name,		 type,		 flag,			 op */
{"Mode",		 eCWMP_tSTRING,  CWMP_READ | CWMP_WRITE, &tPrefixInformationEntityLeafOP},
{"DelegatedConnection", eCWMP_tSTRING,  CWMP_READ | CWMP_WRITE, &tPrefixInformationEntityLeafOP},
{"Prefix", 	 eCWMP_tSTRING,  CWMP_READ | CWMP_WRITE, &tPrefixInformationEntityLeafOP},
{"PreferredLifeTime",	 eCWMP_tSTRING,  CWMP_READ | CWMP_WRITE, &tPrefixInformationEntityLeafOP},
{"ValidLifeTime",	 eCWMP_tUINT,	 CWMP_READ | CWMP_WRITE, &tPrefixInformationEntityLeafOP},
};

enum ePrefixInformationEntityLeaf
{
 ePI_Mode,
 ePI_DelegatedConnection,
 ePI_Prefix,
 ePI_PreferredLifeTime,
 ePI_ValidLifeTime,
};

/* InternetGatewayDevice.LANDevice.{i}.X_TELEFONICA-ES_IPv6LANHostConfigManagement.RadVDConfigManagement.PrefixInformation.{i}.* */
struct CWMP_LEAF tPrefixInformationEntityLeaf[] =
{
{ &tPrefixInformationEntityLeafInfo[ePI_Mode] },
{ &tPrefixInformationEntityLeafInfo[ePI_DelegatedConnection] },
{ &tPrefixInformationEntityLeafInfo[ePI_Prefix] },
{ &tPrefixInformationEntityLeafInfo[ePI_PreferredLifeTime] },
{ &tPrefixInformationEntityLeafInfo[ePI_ValidLifeTime] },
{ NULL },
};


struct CWMP_PRMT tPrefixInformationObjectInfo[] =
{
/* name,	 type,		 flag,					 op */
{"0",		 eCWMP_tOBJECT,  CWMP_READ | CWMP_WRITE | CWMP_LNKLIST,  NULL},
};

enum ePrefixInformationObject
{
 ePrefixInformation0,
};

/* InternetGatewayDevice.LANDevice.{i}.X_TELEFONICA-ES_IPv6LANHostConfigManagement.RadVDConfigManagement.PrefixInformation.{i}. */
struct CWMP_LINKNODE tPrefixInformationObject[] =
{
/* info,						 leaf,				 next,	 sibling,	 instnum */
{ &tPrefixInformationObjectInfo[ePrefixInformation0],	 tPrefixInformationEntityLeaf,	 NULL,	 NULL,		 0 },
};


struct CWMP_OP tPrefixInformationOP = { NULL, objPrefixInformation };

struct CWMP_PRMT tRadVDConfigManagementObjectInfo[] =
{
/* name,		 type,		 flag,			 op */
{"ULAPrefixInfo",	 eCWMP_tOBJECT,  CWMP_READ, 	 NULL},
{"PrefixInformation",	 eCWMP_tOBJECT,  CWMP_READ | CWMP_WRITE, &tPrefixInformationOP},
};

enum eRadVDConfigManagementObject
{
 eULAPrefixInfo,
 ePrefixInformation,
};

/* InternetGatewayDevice.LANDevice.{i}.X_TELEFONICA-ES_IPv6LANHostConfigManagement.RadVDConfigManagement.*. */
struct CWMP_NODE tRadVDConfigManagementObject[] =
{
/* info,						 leaf,			 next */
{ &tRadVDConfigManagementObjectInfo[eULAPrefixInfo],	 tULAPrefixInfoLeaf, NULL},
{ &tRadVDConfigManagementObjectInfo[ePrefixInformation],NULL,			 NULL},
{ NULL, NULL, NULL },
};


struct CWMP_OP tServerTypeLeafOP = { getServerType, setServerType };

struct CWMP_PRMT tServerTypeLeafInfo[] =
{
/* name,	 type,		 flag,			 op */
{"Stateless",	 eCWMP_tBOOLEAN, CWMP_READ | CWMP_WRITE, &tServerTypeLeafOP},
};

enum eServerTypeLeaf
{
 eStateless,
};

/* InternetGatewayDevice.LANDevice.{i}.X_TELEFONICA-ES_IPv6LANHostConfigManagement.ServerType.* */
struct CWMP_LEAF tServerTypeLeaf[] =
{
{ &tServerTypeLeafInfo[eStateless] },
{ NULL },
};


struct CWMP_OP tServerTypeEntityLeafOP = { getServerTypeEntity, setServerTypeEntity };

struct CWMP_PRMT tServerTypeEntityLeafInfo[] =
{
/* name,		 type,		 flag,			 op */
{"MinInterfaceID",  eCWMP_tSTRING,  CWMP_READ | CWMP_WRITE, &tServerTypeEntityLeafOP},
{"MaxInterfaceID",  eCWMP_tSTRING,  CWMP_READ | CWMP_WRITE, &tServerTypeEntityLeafOP},
{"DHCPv6LeaseTime", eCWMP_tINT, CWMP_READ | CWMP_WRITE, &tServerTypeEntityLeafOP},
{"IPv6DomainName",  eCWMP_tSTRING,  CWMP_READ | CWMP_WRITE, &tServerTypeEntityLeafOP},
{"IPv6DNSConfigType",	 eCWMP_tSTRING,  CWMP_READ | CWMP_WRITE, &tServerTypeEntityLeafOP},
{"IPv6DNSServers",  eCWMP_tSTRING,  CWMP_READ | CWMP_WRITE, &tServerTypeEntityLeafOP},
};

enum tServerTypeEntityLeaf
{
 eMinInterfaceID,
 eMaxInterfaceID,
 eDHCPv6LeaseTime,
 eIPv6DomainName,
 eIPv6DNSConfigType,
 eIPv6DNSServers,
};

/* InternetGatewayDevice.LANDevice.{i}.X_TELEFONICA-ES_IPv6LANHostConfigManagement.ServerType.StatefullDHCPv6.* */
struct CWMP_LEAF tServerTypeEntityLeaf[] =
{
{ &tServerTypeEntityLeafInfo[eMinInterfaceID] },
{ &tServerTypeEntityLeafInfo[eMaxInterfaceID] },
{ &tServerTypeEntityLeafInfo[eDHCPv6LeaseTime] },
{ &tServerTypeEntityLeafInfo[eIPv6DomainName] },
{ &tServerTypeEntityLeafInfo[eIPv6DNSConfigType] },
{ &tServerTypeEntityLeafInfo[eIPv6DNSServers] },
{ NULL },
};


struct CWMP_PRMT tServerTypeObjectInfo[] =
{
/* name,		 type,		 flag,			 op */
{"StatefullDHCPv6", eCWMP_tOBJECT,  CWMP_READ, 	 NULL},
};

enum eServerTypeObject
{
 eStatefullDHCPv6,
};

/* InternetGatewayDevice.LANDevice.{i}.X_TELEFONICA-ES_IPv6LANHostConfigManagement.ServerType.*. */
struct CWMP_NODE tServerTypeObject[] =
{
/* info,					 leaf,			 next */
{ &tServerTypeObjectInfo[eStatefullDHCPv6], tServerTypeEntityLeaf,  NULL},
{ NULL, NULL, NULL },
};


struct CWMP_PRMT tIPv6LANHostConfigManagementObjectInfo[] =
{
/* name,			 type,		 flag,		 op */
{"RADVD",			 eCWMP_tOBJECT,  CWMP_READ,  NULL},
{"IPv6SitePrefixConfigType",	 eCWMP_tOBJECT,  CWMP_READ,  NULL},
{"IPv6PDWanConnection",	 eCWMP_tOBJECT,  CWMP_READ,  NULL},
{"IPv6InterfaceNumberOfEntries",eCWMP_tOBJECT,  CWMP_READ,  NULL},
{"RadVDConfigManagement",	 eCWMP_tOBJECT,  CWMP_READ,  NULL},
{"ServerType", 		 eCWMP_tOBJECT,  CWMP_READ,  NULL},
};

enum eIPv6LANHostConfigManagementObject
{
 eRADVD,
 eIPv6SitePrefixConfigType,
 eIPv6PDWanConnection,
 eIPv6InterfaceNumberOfEntries,
 eRadVDConfigManagement,
 eServerType,
};

/* InternetGatewayDevice.LANDevice.{i}.X_TELEFONICA-ES_IPv6LANHostConfigManagement.*. */
struct CWMP_NODE tIPv6LANHostConfigManagementObject[] =
{
/* info,									 leaf,					 next */
{ &tIPv6LANHostConfigManagementObjectInfo[eRADVD], 			 tRADVDLeaf,			 NULL },
{ &tIPv6LANHostConfigManagementObjectInfo[eIPv6SitePrefixConfigType],		 tIPv6SitePrefixConfigTypeLeaf, 	 NULL },
{ &tIPv6LANHostConfigManagementObjectInfo[eIPv6PDWanConnection],		 tIPv6PDWanConnectionLeaf,		 NULL },
{ &tIPv6LANHostConfigManagementObjectInfo[eIPv6InterfaceNumberOfEntries],	 tIPv6InterfaceNumberOfEntriesLeaf,  NULL },
{ &tIPv6LANHostConfigManagementObjectInfo[eRadVDConfigManagement], 	 NULL,					 tRadVDConfigManagementObject },
{ &tIPv6LANHostConfigManagementObjectInfo[eServerType],			 tServerTypeLeaf,			 tServerTypeObject },
{ NULL, NULL, NULL },
};


struct CWMP_OP tIPv6LanIntfAddressLeafOP = { getIPv6LanIntfAddress, setIPv6LanIntfAddress };

struct CWMP_PRMT tIPv6LanIntfAddressLeafInfo[] =
{
/* name,		 type,		 flag,			 op */
{"UniqueLocalAddress",  eCWMP_tSTRING,  CWMP_READ | CWMP_WRITE, &tIPv6LanIntfAddressLeafOP},
};

enum eIPv6LanIntfAddressLeaf
{
 eUniqueLocalAddress,
};

/* InternetGatewayDevice.LANDevice.{i}.LANHostConfigManagement.IPInterface.{i}.X_TELEFONICA-ES_IPv6LanIntfAddress.* */
struct CWMP_LEAF tIPv6LanIntfAddressLeaf[] =
{
{ &tIPv6LanIntfAddressLeafInfo[eUniqueLocalAddress] },
{ NULL },
};


struct CWMP_OP tDelegatedAddressEntityLeafOP = { getDelegatedAddressEntity, setDelegatedAddressEntity };

struct CWMP_PRMT tDelegatedAddressEntityLeafInfo[] =
{
/* name,		 type,		 flag,			 op */
{"Mode",		 eCWMP_tSTRING,  CWMP_READ, 	 &tDelegatedAddressEntityLeafOP},
{"DelegatedConnection", eCWMP_tSTRING,  CWMP_READ | CWMP_WRITE, &tDelegatedAddressEntityLeafOP},
{"IPv6InterfaceAddress",eCWMP_tSTRING,  CWMP_READ | CWMP_WRITE, &tDelegatedAddressEntityLeafOP},
{"IPv6PrefixAddress",	 eCWMP_tSTRING,  CWMP_READ | CWMP_WRITE, &tDelegatedAddressEntityLeafOP},
{"IPv6PrefixLength",	 eCWMP_tUINT,	 CWMP_READ | CWMP_WRITE, &tDelegatedAddressEntityLeafOP},
};

enum eDelegatedAddressEntityLeaf
{
 eDA_Mode,
 eDA_DelegatedConnection,
 eDA_IPv6InterfaceAddress,
 eDA_IPv6PrefixAddress,
 eDA_IPv6PrefixLength,
};

/* InternetGatewayDevice.LANDevice.{i}.LANHostConfigManagement.IPInterface.{i}.X_TELEFONICA-ES_IPv6LanIntfAddress.DelegatedAddress.{i}.* */
struct CWMP_LEAF tDelegatedAddressEntityLeaf[] =
{
{ &tDelegatedAddressEntityLeafInfo[eDA_Mode] },
{ &tDelegatedAddressEntityLeafInfo[eDA_DelegatedConnection] },
{ &tDelegatedAddressEntityLeafInfo[eDA_IPv6InterfaceAddress] },
{ &tDelegatedAddressEntityLeafInfo[eDA_IPv6PrefixAddress] },
{ &tDelegatedAddressEntityLeafInfo[eDA_IPv6PrefixLength] },
{ NULL },
};


struct CWMP_PRMT tDelegatedAddressObjectInfo[] =
{
/* name,	 type,		 flag,					 op */
{"0",		 eCWMP_tOBJECT,  CWMP_READ | CWMP_WRITE | CWMP_LNKLIST,  NULL},
};

enum eDelegatedAddressObject
{
 eDelegatedAddress0,
};

/* InternetGatewayDevice.LANDevice.{i}.LANHostConfigManagement.IPInterface.{i}.X_TELEFONICA-ES_IPv6LanIntfAddress.DelegatedAddress.{i}. */
struct CWMP_LINKNODE tDelegatedAddressObject[] =
{
/* info,						 leaf,				 next,	 sibling,	 instnum */
{ &tDelegatedAddressObjectInfo[eDelegatedAddress0], tDelegatedAddressEntityLeaf,	 NULL,	 NULL,		 0 },
};


struct CWMP_OP tDelegatedAddressOP = { NULL, objDelegatedAddress };

struct CWMP_PRMT tIPv6LanIntfAddressObjectInfo[] =
{
/* name,			 type,		 flag,			 op */
{"DelegatedAddress",		 eCWMP_tOBJECT,  CWMP_READ | CWMP_WRITE, &tDelegatedAddressOP},
};

enum eIPv6LanIntfAddressObject
{
 eDelegatedAddress,
};

/* InternetGatewayDevice.LANDevice.{i}.LANHostConfigManagement.IPInterface.{i}.X_TELEFONICA-ES_IPv6LanIntfAddress.*. */
struct CWMP_NODE tIPv6LanIntfAddressObject[] =
{
/* info,						 leaf,	 next */
{ &tIPv6LanIntfAddressObjectInfo[eDelegatedAddress],	 NULL,	 NULL},
{ NULL, NULL, NULL },
};


struct CWMP_OP tIPv6InterfaceAddressingTypeLeafOP = { getIPv6InterfaceAddressingType, setIPv6InterfaceAddressingType };

struct CWMP_PRMT tIPv6InterfaceAddressingTypeLeafInfo[] =
{
/* name,		 type,		 flag,			 op */
{"StaticDelegated", eCWMP_tBOOLEAN, CWMP_READ | CWMP_WRITE, &tIPv6InterfaceAddressingTypeLeafOP},
};

/*
enum eIPv6InterfaceAddressingTypeLeaf
{
 eStaticDelegated,
};
*/

/* InternetGatewayDevice.LANDevice.{i}.LANHostConfigManagement.IPInterface.{i}.X_TELEFONICA-ES_IPv6InterfaceAddressingType.* */
struct CWMP_LEAF tIPv6InterfaceAddressingTypeLeaf[] =
{
{ &tIPv6InterfaceAddressingTypeLeafInfo[eStaticDelegated] },
{ NULL },
};


struct CWMP_PRMT tIPInterfaceEntityObjectInfo[] =
{
/* name,					 type,		 flag,		 op */
{"X_TELEFONICA-ES_IPv6LanIntfAddress", 	 eCWMP_tOBJECT,  CWMP_READ,  NULL},
{"X_TELEFONICA-ES_IPv6InterfaceAddressingType", eCWMP_tOBJECT,  CWMP_READ,  NULL},
};

enum eIPInterfaceEntityObject
{
 eX_TELEFONICA_ES_IPv6LanIntfAddress,
 eX_TELEFONICA_ES_IPv6InterfaceAddressingType,
};

/* InternetGatewayDevice.LANDevice.{i}.LANHostConfigManagement.IPInterface.{i}.*. */
struct CWMP_NODE tIPInterfaceEntityObject[] =
{
/* info,									 leaf,					 next */
{ &tIPInterfaceEntityObjectInfo[eX_TELEFONICA_ES_IPv6LanIntfAddress],		 tIPv6LanIntfAddressLeaf,		 tIPv6LanIntfAddressObject},
{ &tIPInterfaceEntityObjectInfo[eX_TELEFONICA_ES_IPv6InterfaceAddressingType],  tIPv6InterfaceAddressingTypeLeaf,	 NULL},
{ NULL, NULL, NULL },
};


int getIPv6LANHostConfigManagement(char *name, struct CWMP_LEAF *entity, int *type,
			void **data)
{
	char *lastname = entity->info->name;
#ifdef CONFIG_IPV6
	char buf[256];

	int i, num;
	MIB_CE_ATM_VC_T vc_entity, *pEntry = &vc_entity;
#endif

	if (name == NULL || entity == NULL || type == NULL || data == NULL)
		return -1;

#ifdef CONFIG_IPV6
	num = mib_chain_total(MIB_ATM_VC_TBL);
	for (i = 0; i < num; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, pEntry))
			continue;

		if (pEntry->enable && pEntry->dgw 
				&& (pEntry->IpProtocol & IPVER_IPV6) && pEntry->Ipv6Dhcp)
			break;
	}
#endif

	*type = entity->info->type;
	*data = NULL;
	if (strcmp(lastname, "AutoConfigurationAddress") == 0) {
#ifdef CONFIG_IPV6
		*data = i < num ? booldup(1) : booldup(0);
#else
		*data = booldup(0);
#endif
	} else if (strcmp(lastname, "IPv6DNSWanConnection") == 0) {
#ifdef CONFIG_IPV6
		if (i < num) {
			if (pEntry->cmode == CHANNEL_MODE_PPPOE || pEntry->cmode == CHANNEL_MODE_PPPOA) {
				snprintf(buf, sizeof(buf), "ppp%u", PPP_INDEX(pEntry->ifIndex));
			} else {
				ifGetName(PHY_INTF(pEntry->ifIndex), buf, sizeof(buf));
			}
			*data = strdup(buf);
		} else {
			*data = strdup("");
		}
#else
		*data = strdup("");
#endif
	} else {
		return ERR_9005;
	}

	return 0;
}

int setIPv6LANHostConfigManagement(char *name, struct CWMP_LEAF *entity, int type,
			void *data)
{
	char *lastname = entity->info->name;
#ifdef CONFIG_IPV6
	int *pInt = data;
	char *buf = data;

	int i, num;
	MIB_CE_ATM_VC_T vc_entity, *pEntry = &vc_entity;
#endif

	if (name == NULL || entity == NULL || data == NULL)
		return -1;

	if (entity->info->type != type)
		return ERR_9006;

	if (strcmp(lastname, "AutoConfigurationAddress") == 0) {
#ifdef CONFIG_IPV6
		num = mib_chain_total(MIB_ATM_VC_TBL);
		for (i = 0; i < num; i++) {
			if (!mib_chain_get(MIB_ATM_VC_TBL, i, pEntry))
				continue;

			if (pEntry->enable && pEntry->dgw && (pEntry->IpProtocol & IPVER_IPV6))
				break;
		}

		if (i < num) {
			stopIP_PPP_for_V6(pEntry);
			pEntry->Ipv6Dhcp = *pInt != 0;
			mib_chain_update(MIB_ATM_VC_TBL, pEntry, i);
			startIP_for_V6(pEntry);
		} else {
			return ERR_9002;
		}
#else
		return ERR_9001;
#endif
	} else if (strcmp(lastname, "IPv6DNSWanConnection") == 0) {
#ifdef CONFIG_IPV6
		char (*ifnames)[IFNAMSIZ];
		char ifname[IFNAMSIZ];
		int ret, i_old = -1;

		/* get the array of Inteface Names */
		num = mib_chain_total(MIB_ATM_VC_TBL);
		ifnames = calloc(num, IFNAMSIZ);
		for (i = 0; i < num; i++) {
			if (!mib_chain_get(MIB_ATM_VC_TBL, i, pEntry))
				continue;

			if (i_old == -1 && pEntry->enable && pEntry->dgw
					&& (pEntry->IpProtocol & IPVER_IPV6) && pEntry->Ipv6Dhcp)
				i_old = i;

			if (pEntry->cmode == CHANNEL_MODE_PPPOE || pEntry->cmode == CHANNEL_MODE_PPPOA) {
				snprintf(ifname, sizeof(ifname), "ppp%u", 
						PPP_INDEX(pEntry->ifIndex));
			} else {
				ifGetName(PHY_INTF(pEntry->ifIndex), ifname, sizeof(ifname));
			}

			strncpy(ifnames[i], ifname, sizeof(ifnames[i]));
		}

		for (i = 0; i < num; i++) {
			if (strncmp(ifnames[i], buf, sizeof(ifnames[i])) == 0)
				break;
		}

		if (i < num) {
			if (i_old == -1)
				goto select_new;

			/* deselect the old interface */
			if (!mib_chain_get(MIB_ATM_VC_TBL, i_old, pEntry)) {
				ret = ERR_9002;
				goto fail;
			}
			stopIP_PPP_for_V6(pEntry);
			pEntry->dgw = pEntry->Ipv6Dhcp = 0;
			mib_chain_update(MIB_ATM_VC_TBL, pEntry, i_old);
			startIP_for_V6(pEntry);

select_new:
			/* select the new interface */
			if (!mib_chain_get(MIB_ATM_VC_TBL, i, pEntry)) {
				ret = ERR_9002;
				goto fail;
			}
			stopIP_PPP_for_V6(pEntry);
			pEntry->enable = pEntry->dgw = pEntry->Ipv6Dhcp = 1;
			pEntry->IpProtocol |= IPVER_IPV6;
			mib_chain_update(MIB_ATM_VC_TBL, pEntry, i);
			startIP_for_V6(pEntry);

			ret = 0;
		} else {
			ret = ERR_9003;
		}

fail:
		free(ifnames);
		return ret;
#else
		return ERR_9001;
#endif
	} else {
		return ERR_9005;
	}

	return 0;
}

int getRADVDLeaf(char *name, struct CWMP_LEAF *entity, int *type,
			void **data)
{
	char *lastname = entity->info->name;
	unsigned char vUchar;

	if (name == NULL || entity == NULL || type == NULL || data == NULL)
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if (strcmp(lastname, "Enabled") == 0) {
		*data = booldup(vUchar != 0);
	} else {
		return ERR_9005;
	}

	return 0;
}

int setRADVDLeaf(char *name, struct CWMP_LEAF *entity, int type,
			void *data)
{
	char *lastname = entity->info->name;
	char *buf = data;

	if (name == NULL || entity == NULL || data == NULL)
		return -1;

	if (entity->info->type != type)
		return ERR_9006;

	if (strcmp(lastname, "Enabled") == 0) {
	} else {
		return ERR_9005;
	}

	return 0;
}

int getIPv6SitePrefixConfigType(char *name, struct CWMP_LEAF *entity, int *type,
			void **data)
{
	char *lastname = entity->info->name;
#if defined(CONFIG_IPV6) && defined(CONFIG_USER_RADVD)
	unsigned char vUchar;
#endif

	if (name == NULL || entity == NULL || type == NULL || data == NULL)
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if (strcmp(lastname, "StaticDelegated") == 0) {
#if defined(CONFIG_IPV6) && defined(CONFIG_USER_RADVD)
		mib_get(MIB_V6_PREFIX_MODE, &vUchar);
		*data = booldup(vUchar != 0);
#else
		*data = booldup(1);
#endif
	} else {
		return ERR_9005;
	}

	return 0;
}

int setIPv6SitePrefixConfigType(char *name, struct CWMP_LEAF *entity, int type,
			void *data)
{
	char *lastname = entity->info->name;
#if defined(CONFIG_IPV6) && defined(CONFIG_USER_RADVD)
	int *pInt = data;
	unsigned char vUchar;

	int radvdpid;
#endif

	if (name == NULL || entity == NULL || data == NULL)
		return -1;

	if (entity->info->type != type)
		return ERR_9006;

	if (strcmp(lastname, "StaticDelegated") == 0) {
#if defined(CONFIG_IPV6) && defined(CONFIG_USER_RADVD)
		vUchar = *pInt != 0;
		mib_set(MIB_V6_PREFIX_MODE, &vUchar);

		radvdpid = read_pid((char *)RADVD_PID);
		if (*pInt) {
			if (radvdpid > 0) {
				kill(radvdpid, SIGTERM);
			}
		} else {
			if (radvdpid <= 0)
				va_cmd("/bin/radvd", 3, 0, "-s", "-C", (char *)RADVD_CONF);
		}
#else
		return ERR_9003;
#endif
	} else {
		return ERR_9005;
	}

	return 0;
}

int getIPv6PDWanConnection(char *name, struct CWMP_LEAF *entity, int *type,
			void **data)
{
	char *lastname = entity->info->name;
#ifdef CONFIG_IPV6
	char buf[256];

	int i, num;
	MIB_CE_ATM_VC_T vc_entity, *pEntry = &vc_entity;
#endif

	if (name == NULL || entity == NULL || type == NULL || data == NULL)
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if (strcmp(lastname, "Interface") == 0) {
#ifdef CONFIG_IPV6
		num = mib_chain_total(MIB_ATM_VC_TBL);
		for (i = 0; i < num; i++) {
			if (!mib_chain_get(MIB_ATM_VC_TBL, i, pEntry))
				continue;

			if (pEntry->enable && pEntry->dgw 
					&& (pEntry->IpProtocol & IPVER_IPV6) && pEntry->Ipv6Dhcp
					&& (pEntry->Ipv6DhcpRequest & 0x2))
				break;
		}

		if (i < num) {
			if (pEntry->cmode == CHANNEL_MODE_PPPOE || pEntry->cmode == CHANNEL_MODE_PPPOA) {
				snprintf(buf, sizeof(buf), "ppp%u", PPP_INDEX(pEntry->ifIndex));
			} else {
				ifGetName(PHY_INTF(pEntry->ifIndex), buf, sizeof(buf));
			}
			*data = strdup(buf);
		} else {
			*data = strdup("");
		}
#else
		*data = strdup("");
#endif
	} else {
		return ERR_9005;
	}

	return 0;
}

int setIPv6PDWanConnection(char *name, struct CWMP_LEAF *entity, int type,
			void *data)
{
	char *lastname = entity->info->name;
#ifdef CONFIG_IPV6
	int *pInt = data;
	char *buf = data;

	int i, num;
	MIB_CE_ATM_VC_T vc_entity, *pEntry = &vc_entity;
#endif

	if (name == NULL || entity == NULL || data == NULL)
		return -1;

	if (entity->info->type != type)
		return ERR_9006;

	if (strcmp(lastname, "Interface") == 0) {
#ifdef CONFIG_IPV6
		char (*ifnames)[IFNAMSIZ];
		char ifname[IFNAMSIZ];
		int ret, i_old = -1;

		/* get the array of Inteface Names */
		num = mib_chain_total(MIB_ATM_VC_TBL);
		ifnames = calloc(num, IFNAMSIZ);
		for (i = 0; i < num; i++) {
			if (!mib_chain_get(MIB_ATM_VC_TBL, i, pEntry))
				continue;

			if (i_old == -1 && pEntry->enable && pEntry->dgw 
					&& (pEntry->IpProtocol & IPVER_IPV6) && pEntry->Ipv6Dhcp
					&& (pEntry->Ipv6DhcpRequest & 0x2))
				i_old = i;

			if (pEntry->cmode == CHANNEL_MODE_PPPOE || pEntry->cmode == CHANNEL_MODE_PPPOA) {
				snprintf(ifname, sizeof(ifname), "ppp%u", 
						PPP_INDEX(pEntry->ifIndex));
			} else {
				ifGetName(PHY_INTF(pEntry->ifIndex), ifname, sizeof(ifname));
			}

			strncpy(ifnames[i], ifname, sizeof(ifnames[i]));
		}

		for (i = 0; i < num; i++) {
			if (strncmp(ifnames[i], buf, sizeof(ifnames[i])) == 0)
				break;
		}

		if (i < num) {
			if (i_old == -1)
				goto select_new;

			/* deselect the old interface */
			if (!mib_chain_get(MIB_ATM_VC_TBL, i_old, pEntry)) {
				ret = ERR_9002;
				goto fail;
			}
			stopIP_PPP_for_V6(pEntry);
			pEntry->dgw = pEntry->Ipv6Dhcp = 0;
			pEntry->Ipv6DhcpRequest &= ~0x2;
			mib_chain_update(MIB_ATM_VC_TBL, pEntry, i_old);
			startIP_for_V6(pEntry);

select_new:
			/* select the new interface */
			if (!mib_chain_get(MIB_ATM_VC_TBL, i, pEntry)) {
				ret = ERR_9002;
				goto fail;
			}
			stopIP_PPP_for_V6(pEntry);
			pEntry->enable = pEntry->dgw = pEntry->Ipv6Dhcp = 1;
			pEntry->Ipv6DhcpRequest |= 0x2;
			pEntry->IpProtocol |= IPVER_IPV6;
			mib_chain_update(MIB_ATM_VC_TBL, pEntry, i);
			startIP_for_V6(pEntry);

			ret = 0;
		} else {
			ret = ERR_9003;
		}

fail:
		free(ifnames);
		return ret;
#else
		return ERR_9001;
#endif
	} else {
		return ERR_9005;
	}

	return 0;
}

int getIPv6InterfaceNumberOfEntries(char *name, struct CWMP_LEAF *entity,
				    int *type, void **data)
{
	char *lastname = entity->info->name;
#ifdef CONFIG_IPV6
	unsigned int vUint;

	struct ifaddrs *ifaddr, *ifa;
#endif

	if (name == NULL || entity == NULL || type == NULL || data == NULL)
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if (strcmp(lastname, "Nentries") == 0) {
#ifdef CONFIG_IPV6
		FILE *f;
		char devname[IFNAMSIZ];

		f = fopen("/proc/net/if_inet6", "r");
		if (f == NULL)
			return ERR_9002;

		vUint = 0;

		while (fscanf(f, "%*32s %*08x %*02x %*02x %*02x %16s\n", devname) != EOF) {
			/* always "br0" now */
			if (!strcmp(devname, "br0")) {
				vUint++;
			}
		}
		fclose(f);

		*data = uintdup(vUint);
#else
		*data = uintdup(0);
#endif
	} else {
		return ERR_9005;
	}

	return 0;
}

int setIPv6InterfaceNumberOfEntries(char *name, struct CWMP_LEAF *entity, int type,
			void *data)
{
	char *lastname = entity->info->name;

	if (name == NULL || entity == NULL || data == NULL)
		return -1;

	if (entity->info->type != type)
		return ERR_9006;

	if (strcmp(lastname, "Nentries") == 0) {
		/* ignored */
	} else {
		return ERR_9005;
	}

	return 0;
}

int getULAPrefixInfo(char *name, struct CWMP_LEAF *entity, int *type,
			void **data)
{
	char *lastname = entity->info->name;
	unsigned char vUchar;
	int vInt;
	char buf[256];

	if (name == NULL || entity == NULL || type == NULL || data == NULL)
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if (strcmp(lastname, "Enable") == 0) {
		*data = booldup(vUchar != 0);
	} else if (strcmp(lastname, "Prefix") == 0) {
		*data = strdup(buf);
	} else if (strcmp(lastname, "PreferredLifeTime") == 0) {
		*data = intdup(vInt);
	} else if (strcmp(lastname, "ValidLifeTime") == 0) {
		*data = intdup(vInt);
	} else {
		return ERR_9005;
	}

	return 0;
}

int setULAPrefixInfo(char *name, struct CWMP_LEAF *entity, int type,
			void *data)
{
	char *lastname = entity->info->name;
	unsigned char *pUchar = data;
	int *pInt = data;
	char *buf = data;

	if (name == NULL || entity == NULL || data == NULL)
		return -1;

	if (entity->info->type != type)
		return ERR_9006;

	if (strcmp(lastname, "Enable") == 0) {
	} else if (strcmp(lastname, "Prefix") == 0) {
	} else if (strcmp(lastname, "PreferredLifeTime") == 0) {
	} else if (strcmp(lastname, "ValidLifeTime") == 0) {
	} else {
		return ERR_9005;
	}

	return 0;
}

int getPrefixInformationEntity(char *name, struct CWMP_LEAF *entity, int *type,
			void **data)
{
	char *lastname = entity->info->name;
	unsigned char vUchar;
	int vInt;
	char buf[256];

	if (name == NULL || entity == NULL || type == NULL || data == NULL)
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if (strcmp(lastname, "Mode") == 0) {
		*data = booldup(vUchar != 0);
	} else if (strcmp(lastname, "DelegatedConnection") == 0) {
		*data = strdup(buf);
	} else if (strcmp(lastname, "Prefix") == 0) {
		*data = strdup(buf);
	} else if (strcmp(lastname, "PreferredLifeTime") == 0) {
		*data = intdup(vInt);
	} else if (strcmp(lastname, "ValidLifeTime") == 0) {
		*data = intdup(vInt);
	} else {
		return ERR_9005;
	}

	return 0;
}

int setPrefixInformationEntity(char *name, struct CWMP_LEAF *entity, int type,
			void *data)
{
	char *lastname = entity->info->name;
	unsigned char *pUchar = data;
	int *pInt = data;
	char *buf = data;

	if (name == NULL || entity == NULL || data == NULL)
		return -1;

	if (entity->info->type != type)
		return ERR_9006;

	if (strcmp(lastname, "Mode") == 0) {
	} else if (strcmp(lastname, "DelegatedConnection") == 0) {
	} else if (strcmp(lastname, "Prefix") == 0) {
	} else if (strcmp(lastname, "PreferredLifeTime") == 0) {
	} else if (strcmp(lastname, "ValidLifeTime") == 0) {
	} else {
		return ERR_9005;
	}

	return 0;
}

int objPrefixInformation(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	int ret = 0;
	struct CWMP_NODE *entity = (struct CWMP_NODE *)e;
	unsigned int i, num, maxInstNum = 0;

	if (name == NULL || entity == NULL)
		return -1;

	switch (type) {
	case eCWMP_tINITOBJ:
		if (data == NULL)
			return -1;

		if (create_Object(data, tPrefixInformationObject,
				  sizeof(tPrefixInformationObject), 1, ++maxInstNum) < 0)
			return -1;

		add_objectNum(name, maxInstNum);

		break;
	case eCWMP_tADDOBJ:
		if (data == NULL)
			return -1;

		if (add_Object(name,
			       (struct CWMP_LINKNODE **)&entity->next,
			       tPrefixInformationObject,
			       sizeof(tPrefixInformationObject), data) < 0)
			return -1;

		break;
	case eCWMP_tDELOBJ:
		if (data == NULL)
			return -1;

		if (del_Object(name, (struct CWMP_LINKNODE **)
					 &entity->next, *(int *)data) < 0)
			return -1;

		break;
	case eCWMP_tUPDATEOBJ:
	{
		return -1;

		struct CWMP_LINKNODE *old_table;

		old_table = (struct CWMP_LINKNODE *)entity->next;
		entity->next = NULL;

		for (i = 0; i < num; i++) {
			struct CWMP_LINKNODE *remove_entity = NULL;

			remove_entity =
			    remove_SiblingEntity(&old_table, i);

			if (remove_entity != NULL) {
				/* it exists both in the MIB and the parameter tree */
				add_SiblingEntity((struct CWMP_LINKNODE
						   **)&entity->next,
						  remove_entity);
			} else {
				/* it exists only in the MIB,
				 * so we add it into the parameter tree */
				add_Object(name,
					   (struct CWMP_LINKNODE **)
					   &entity->next,
					   tPrefixInformationObject,
					   sizeof(tPrefixInformationObject),
					   &i);
				/*mib_chain_update(CWMP_CT_ALARM_TBL,*/
				/*pAlarm, i);*/
			}
		}

		if (old_table) {
			/* it exists only in the parameter tree,
			 * so we remove it from the parameter tree */
			destroy_ParameterTable((struct CWMP_NODE *)
					       old_table);
		}

		break;
	}
	}

	return ret;
}

int getServerType(char *name, struct CWMP_LEAF *entity, int *type,
			void **data)
{
	char *lastname = entity->info->name;
	unsigned char vUchar;

	if (name == NULL || entity == NULL || type == NULL || data == NULL)
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if (strcmp(lastname, "Stateless") == 0) {
		*data = booldup(vUchar != 0);
	} else {
		return ERR_9005;
	}

	return 0;
}

int setServerType(char *name, struct CWMP_LEAF *entity, int type,
			void *data)
{
	char *lastname = entity->info->name;
	unsigned char *pUchar;

	if (name == NULL || entity == NULL || data == NULL)
		return -1;

	if (entity->info->type != type)
		return ERR_9006;

	if (strcmp(lastname, "Stateless") == 0) {
	} else {
		return ERR_9005;
	}

	return 0;
}

int getServerTypeEntity(char *name, struct CWMP_LEAF *entity, int *type,
			void **data)
{
	char *lastname = entity->info->name;
#if defined(CONFIG_IPV6) && defined(CONFIG_USER_DHCPV6_ISC_DHCP411)
	unsigned char vUchar;
	unsigned int vUint;
	char buf[1024];
	struct in6_addr ip6Addr;
#endif

	if (name == NULL || entity == NULL || type == NULL || data == NULL)
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if (strcmp(lastname, "MinInterfaceID") == 0) {
#if defined(CONFIG_IPV6) && defined(CONFIG_USER_DHCPV6_ISC_DHCP411)
		mib_get(MIB_DHCPV6S_RANGE_START, &ip6Addr);
		inet_ntop(AF_INET6, &ip6Addr, buf, sizeof(buf));
		*data = strdup(buf);
#else
		*data = strdup("");
#endif
	} else if (strcmp(lastname, "MaxInterfaceID") == 0) {
#if defined(CONFIG_IPV6) && defined(CONFIG_USER_DHCPV6_ISC_DHCP411)
		mib_get(MIB_DHCPV6S_RANGE_END, &ip6Addr);
		inet_ntop(AF_INET6, &ip6Addr, buf, sizeof(buf));
		*data = strdup(buf);
#else
		*data = strdup("");
#endif
	} else if (strcmp(lastname, "DHCPv6LeaseTime") == 0) {
#if defined(CONFIG_IPV6) && defined(CONFIG_USER_DHCPV6_ISC_DHCP411)
		mib_get(MIB_DHCPV6S_DEFAULT_LEASE, &vUint);
		*data = uintdup(vUint);
#else
		*data = uintdup(0);
#endif
	} else if (strcmp(lastname, "IPv6DomainName") == 0) {
#if defined(CONFIG_IPV6) && defined(CONFIG_USER_DHCPV6_ISC_DHCP411)
		MIB_DHCPV6S_DOMAIN_SEARCH_T entry;
		int i, total;

		buf[0] = '\0';
		total = mib_chain_total(MIB_DHCPV6S_DOMAIN_SEARCH_TBL);
		for (i = 0; i < total; i++) {
			if (!mib_chain_get(MIB_DHCPV6S_DOMAIN_SEARCH_TBL, i, &entry)) {
				continue;
			}

			if (strlen(buf) != 0)
				strcat(buf, ", ");	

			strcat(buf, entry.domain);
		}

		*data = strdup(buf);
#else
		*data = strdup("");
#endif
	} else if (strcmp(lastname, "IPv6DNSConfigType") == 0) {
#if defined(CONFIG_IPV6) && defined(CONFIG_USER_DHCPV6_ISC_DHCP411)
		mib_get(MIB_DHCPV6_MODE, &vUchar);
		*data = strdup(vUchar == DHCP_LAN_SERVER_AUTO ? "DHCP" : "Static");
#else
		*data = strdup("");
#endif
	} else if (strcmp(lastname, "IPv6DNSServers") == 0) {
#if defined(CONFIG_IPV6) && defined(CONFIG_USER_DHCPV6_ISC_DHCP411)
		MIB_DHCPV6S_NAME_SERVER_T entry;
		int i, total;

		buf[0] = '\0';
		total = mib_chain_total(MIB_DHCPV6S_NAME_SERVER_TBL);
		for (i = 0; i < total; i++) {
			if (!mib_chain_get(MIB_DHCPV6S_NAME_SERVER_TBL, i, &entry)) {
				continue;
			}

			if (strlen(buf) != 0)
				strcat(buf, ", ");	

			strcat(buf, entry.nameServer);
		}

		*data = strdup(buf);
#else
		*data = strdup("");
#endif
	} else {
		return ERR_9005;
	}

	return 0;
}

int setServerTypeEntity(char *name, struct CWMP_LEAF *entity, int type,
			void *data)
{
	char *lastname = entity->info->name;
#if defined(CONFIG_IPV6) && defined(CONFIG_USER_DHCPV6_ISC_DHCP411)
	int *pUint = data, ret = 0;
	char *buf = data;
	struct in6_addr ip6Addr;
#endif

	if (name == NULL || entity == NULL || data == NULL)
		return -1;

	if (entity->info->type != type)
		return ERR_9006;

#if defined(CONFIG_IPV6) && defined(CONFIG_USER_DHCPV6_ISC_DHCP411)
	start_dhcpv6(0);
#endif

	if (strcmp(lastname, "MinInterfaceID") == 0) {
#if defined(CONFIG_IPV6) && defined(CONFIG_USER_DHCPV6_ISC_DHCP411)
		if (inet_pton(AF_INET6, buf, &ip6Addr) != 1) {
			ret = ERR_9003;
			goto ret;
		}

		if (mib_set(MIB_DHCPV6S_RANGE_START, &ip6Addr) != 1) {
			ret = ERR_9002;
		}
#else
		return ERR_9003;
#endif
	} else if (strcmp(lastname, "MaxInterfaceID") == 0) {
#if defined(CONFIG_IPV6) && defined(CONFIG_USER_DHCPV6_ISC_DHCP411)
		if (inet_pton(AF_INET6, buf, &ip6Addr) != 1) {
			ret = ERR_9003;
			goto ret;
		}

		if (mib_set(MIB_DHCPV6S_RANGE_END, &ip6Addr) != 1) {
			ret = ERR_9002;
		}
#else
		return ERR_9003;
#endif
	} else if (strcmp(lastname, "DHCPv6LeaseTime") == 0) {
#if defined(CONFIG_IPV6) && defined(CONFIG_USER_DHCPV6_ISC_DHCP411)
		if (mib_set(MIB_DHCPV6S_DEFAULT_LEASE, pUint) != 1) {
			ret = ERR_9002;
		}
#else
		return ERR_9003;
#endif
	} else if (strcmp(lastname, "IPv6DomainName") == 0) {
#if defined(CONFIG_IPV6) && defined(CONFIG_USER_DHCPV6_ISC_DHCP411)
		MIB_DHCPV6S_DOMAIN_SEARCH_T entry;
		char *ptr;

		if (mib_chain_clear(MIB_DHCPV6S_DOMAIN_SEARCH_TBL) != 1) {
			ret = ERR_9002;
			goto ret;
		}

		for (ptr = strtok(buf, " ,"); ptr; ptr = strtok(NULL, " ,")) {
			strncpy(entry.domain, ptr, sizeof(entry.domain));
			mib_chain_add(MIB_DHCPV6S_DOMAIN_SEARCH_TBL, &entry);
		}
#else
		return ERR_9003;
#endif
	} else if (strcmp(lastname, "IPv6DNSConfigType") == 0) {
#if defined(CONFIG_IPV6) && defined(CONFIG_USER_DHCPV6_ISC_DHCP411)
		unsigned char vUchar;

		if (!strcmp(buf, "Static")) {
			vUchar = DHCP_LAN_SERVER;
		} else if (!strcmp(buf, "DHCP")) {
			vUchar = DHCP_LAN_SERVER_AUTO;
		} else {
			ret = ERR_9003;
			goto ret;
		}

		if (mib_set(MIB_DHCPV6_MODE, &vUchar) != 1) {
			ret = ERR_9002;
		}
#else
		return ERR_9003;
#endif
	} else if (strcmp(lastname, "IPv6DNSServers") == 0) {
#if defined(CONFIG_IPV6) && defined(CONFIG_USER_DHCPV6_ISC_DHCP411)
		MIB_DHCPV6S_NAME_SERVER_T entry;
		char *ptr;

		if (mib_chain_clear(MIB_DHCPV6S_NAME_SERVER_TBL) != 1) {
			ret = ERR_9003;
			goto ret;
		}

		for (ptr = strtok(buf, " ,"); ptr; ptr = strtok(NULL, " ,")) {
			strncpy(entry.nameServer, ptr, sizeof(entry.nameServer));
			mib_chain_add(MIB_DHCPV6S_NAME_SERVER_TBL, &entry);
		}
#else
		return ERR_9003;
#endif
	} else {
		return ERR_9005;
	}

#if defined(CONFIG_IPV6) && defined(CONFIG_USER_DHCPV6_ISC_DHCP411)
ret:
	start_dhcpv6(1);
#endif

	return ret;
}

int getIPv6LanIntfAddress(char *name, struct CWMP_LEAF *entity, int *type,
			void **data)
{
	char *lastname = entity->info->name;
#if defined(CONFIG_IPV6) && defined(CONFIG_USER_RADVD)
	char buf[256];
	unsigned char vUchar;
	int i;
	char ula[INET6_ADDRSTRLEN];

	char ip6Addr[IP6_ADDR_LEN], devAddr[MAC_ADDR_LEN];
	char meui64[8];
#endif

	if (name == NULL || entity == NULL || type == NULL || data == NULL)
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if (strcmp(lastname, "UniqueLocalAddress") == 0) {
#if defined(CONFIG_IPV6) && defined(CONFIG_USER_RADVD)
		vUchar = 0;
		mib_get(MIB_V6_ULAPREFIX_ENABLE, &vUchar);
		if (vUchar == 0) {
			buf[0] = '\0';
			goto ret;
		}

		mib_get(MIB_V6_ULAPREFIX, ula);
		inet_pton(AF_INET6, ula, ip6Addr);	

		// get LAN ULA v6 IP address according the ULA prefix + EUI64. 
		mib_get(MIB_ELAN_MAC_ADDR, devAddr);
		mac_meui64(devAddr, meui64);
		for (i = 0; i < 8; i++)
			ip6Addr[i + 8] = meui64[i];
		inet_ntop(AF_INET6, ip6Addr, buf, sizeof(buf));

ret:
		*data = strdup(buf);
#else
		*data = strdup("");
#endif
	} else {
		return ERR_9005;
	}

	return 0;
}

int setIPv6LanIntfAddress(char *name, struct CWMP_LEAF *entity, int type,
			void *data)
{
	char *lastname = entity->info->name;

	if (name == NULL || entity == NULL || data == NULL)
		return -1;

	if (entity->info->type != type)
		return ERR_9006;

	if (strcmp(lastname, "UniqueLocalAddress") == 0) {
		return ERR_9008;
	} else {
		return ERR_9005;
	}

	return 0;
}

int getDelegatedAddressEntity(char *name, struct CWMP_LEAF *entity, int *type,
			void **data)
{
	char *lastname = entity->info->name;
	unsigned int vUint;
	char buf[256];

	if (name == NULL || entity == NULL || type == NULL || data == NULL)
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if (strcmp(lastname, "Mode") == 0) {
		*data = strdup(buf);
	} else if (strcmp(lastname, "DelegatedConnection") == 0) {
		*data = strdup(buf);
	} else if (strcmp(lastname, "IPv6InterfaceAddress") == 0) {
		*data = strdup(buf);
	} else if (strcmp(lastname, "IPv6PrefixAddress") == 0) {
		*data = strdup(buf);
	} else if (strcmp(lastname, "IPv6PrefixLength") == 0) {
		*data = uintdup(vUint);
	} else {
		return ERR_9005;
	}

	return 0;
}

int setDelegatedAddressEntity(char *name, struct CWMP_LEAF *entity, int type,
			void *data)
{
	char *lastname = entity->info->name;
	unsigned int *pUint = data;
	char *buf = data;

	if (name == NULL || entity == NULL || data == NULL)
		return -1;

	if (entity->info->type != type)
		return ERR_9006;

	if (strcmp(lastname, "Mode") == 0) {
	} else if (strcmp(lastname, "DelegatedConnection") == 0) {
	} else if (strcmp(lastname, "IPv6InterfaceAddress") == 0) {
	} else if (strcmp(lastname, "IPv6PrefixAddress") == 0) {
	} else if (strcmp(lastname, "IPv6PrefixLength") == 0) {
	} else {
		return ERR_9005;
	}

	return 0;
}

int objDelegatedAddress(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	int ret = 0;
	struct CWMP_NODE *entity = (struct CWMP_NODE *)e;
	unsigned int i, num, maxInstNum = 0;

	if (name == NULL || entity == NULL)
		return -1;

	switch (type) {
	case eCWMP_tINITOBJ:
		if (data == NULL)
			return -1;

		if (create_Object(data, tDelegatedAddressObject,
				  sizeof(tDelegatedAddressObject), 1, ++maxInstNum) < 0)
			return -1;

		add_objectNum(name, maxInstNum);

		break;
	case eCWMP_tADDOBJ:
		if (data == NULL)
			return -1;

		if (add_Object(name,
			       (struct CWMP_LINKNODE **)&entity->next,
			       tDelegatedAddressObject,
			       sizeof(tDelegatedAddressObject), data) < 0)
			return -1;

		break;
	case eCWMP_tDELOBJ:
		if (data == NULL)
			return -1;

		if (del_Object(name, (struct CWMP_LINKNODE **)
					 &entity->next, *(int *)data) < 0)
			return -1;

		break;
	case eCWMP_tUPDATEOBJ:
	{
		return -1;

		struct CWMP_LINKNODE *old_table;

		old_table = (struct CWMP_LINKNODE *)entity->next;
		entity->next = NULL;

		for (i = 0; i < num; i++) {
			struct CWMP_LINKNODE *remove_entity = NULL;

			remove_entity =
			    remove_SiblingEntity(&old_table, i);

			if (remove_entity != NULL) {
				/* it exists both in the MIB and the parameter tree */
				add_SiblingEntity((struct CWMP_LINKNODE
						   **)&entity->next,
						  remove_entity);
			} else {
				/* it exists only in the MIB,
				 * so we add it into the parameter tree */
				add_Object(name,
					   (struct CWMP_LINKNODE **)
					   &entity->next,
					   tDelegatedAddressObject,
					   sizeof(tDelegatedAddressObject),
					   &i);
				/*mib_chain_update(CWMP_CT_ALARM_TBL,*/
				/*pAlarm, i);*/
			}
		}

		if (old_table) {
			/* it exists only in the parameter tree,
			 * so we remove it from the parameter tree */
			destroy_ParameterTable((struct CWMP_NODE *)
					       old_table);
		}

		break;
	}
	}

	return ret;
}

int getIPv6InterfaceAddressingType(char *name, struct CWMP_LEAF *entity, int *type,
			void **data)
{
	char *lastname = entity->info->name;
#if defined(CONFIG_IPV6) && defined(CONFIG_USER_DHCPV6_ISC_DHCP411)
	unsigned char vUchar;
#endif

	if (name == NULL || entity == NULL || type == NULL || data == NULL)
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if (strcmp(lastname, "StaticDelegated") == 0) {
#if defined(CONFIG_IPV6) && defined(CONFIG_USER_DHCPV6_ISC_DHCP411)
		mib_get(MIB_DHCPV6_MODE, &vUchar);
		*data = booldup(vUchar != DHCP_LAN_SERVER_AUTO);
#else
		*data = booldup(1);
#endif
	} else {
		return ERR_9005;
	}

	return 0;
}

int setIPv6InterfaceAddressingType(char *name, struct CWMP_LEAF *entity, int type,
			void *data)
{
	char *lastname = entity->info->name;
#if defined(CONFIG_IPV6) && defined(CONFIG_USER_DHCPV6_ISC_DHCP411)
	int *pInt = data;
	unsigned char vUchar;
#endif

	if (name == NULL || entity == NULL || data == NULL)
		return -1;

	if (entity->info->type != type)
		return ERR_9006;

	if (strcmp(lastname, "StaticDelegated") == 0) {
#if defined(CONFIG_IPV6) && defined(CONFIG_USER_DHCPV6_ISC_DHCP411)
		start_dhcpv6(0);
		vUchar = *pInt == 0 ? DHCP_LAN_SERVER_AUTO : DHCP_LAN_SERVER;
		mib_set(MIB_DHCPV6_MODE, &vUchar);
		start_dhcpv6(1);
#else
		return ERR_9003;
#endif
	} else {
		return ERR_9005;
	}

	return 0;
}




/*****************************************************************************
 * InternetGatewayDevice.WANDevice.{i}.WANConnectionDevice.{i}.WANPPPConnection.{i}.
 *****************************************************************************/
struct CWMP_OP tBHSWANPPPCONENTITYLeafOP = { getBHSWANPPPCONENTITY, setBHSWANPPPCONENTITY };
struct CWMP_PRMT tBHSWANPPPCONENTITYLeafInfo[] =
{
{"X_TELEFONICA-ES_IPv6Enabled",	 eCWMP_tBOOLEAN, CWMP_READ | CWMP_WRITE,	 &tBHSWANPPPCONENTITYLeafOP},
{"X_TELEFONICA-ES_IPv4Enabled",	 eCWMP_tBOOLEAN, CWMP_READ | CWMP_WRITE,	 &tBHSWANPPPCONENTITYLeafOP},
{"X_TELEFONICA-ES_UnnumberedModel", eCWMP_tBOOLEAN, CWMP_READ | CWMP_WRITE,	 &tBHSWANPPPCONENTITYLeafOP},
{"X_TELEFONICA-ES_IPv6ConnStatus",  eCWMP_tSTRING,  CWMP_READ, 		 &tBHSWANPPPCONENTITYLeafOP},
{"X_TELEFONICA-ES_IPv6PppUp",		 eCWMP_tSTRING,  CWMP_READ, 		 &tBHSWANPPPCONENTITYLeafOP},
{"X_TELEFONICA-ES_IPv6AddressingType",  eCWMP_tSTRING,  CWMP_READ, 		 &tBHSWANPPPCONENTITYLeafOP},
{"X_TELEFONICA-ES_DHCP6cForAddress",	 eCWMP_tBOOLEAN, CWMP_READ | CWMP_WRITE,	 &tBHSWANPPPCONENTITYLeafOP},
{"X_TELEFONICA-ES_ExternalIPv6Address", eCWMP_tSTRING,  CWMP_READ | CWMP_WRITE,	 &tBHSWANPPPCONENTITYLeafOP},
{"X_TELEFONICA-ES_DefaultIPv6Gateway",  eCWMP_tSTRING,  CWMP_READ | CWMP_WRITE,	 &tBHSWANPPPCONENTITYLeafOP},
{"X_TELEFONICA-ES_IPv6DNSServers",  eCWMP_tSTRING,  CWMP_READ | CWMP_WRITE,	 &tBHSWANPPPCONENTITYLeafOP},
{"X_TELEFONICA-ES_IPv6PrefixDelegationEnabled", eCWMP_tBOOLEAN, CWMP_READ, 	 &tBHSWANPPPCONENTITYLeafOP},
{"X_TELEFONICA-ES_IPv6SitePrefix",  eCWMP_tSTRING,  CWMP_READ | CWMP_WRITE,	 &tBHSWANPPPCONENTITYLeafOP},
{"X_TELEFONICA-ES_IPv6SitePrefixPltime",eCWMP_tINT, CWMP_READ | CWMP_WRITE,	 &tBHSWANPPPCONENTITYLeafOP},
{"X_TELEFONICA-ES_IPv6SitePrefixVLtime",eCWMP_tINT, CWMP_READ | CWMP_WRITE,	 &tBHSWANPPPCONENTITYLeafOP},
{"X_TELEFONICA-ES_ExternalIPv6AddressPrefixLength", eCWMP_tUINT,	 CWMP_READ | CWMP_WRITE, &tBHSWANPPPCONENTITYLeafOP},
};

int getBHSWANPPPCONENTITY(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	//char	*tok;
	unsigned int	chainidx;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;
	char	ifname[16];
	char	buf[512]="";
	unsigned char vChar=0;
	DLG_INFO_T dlg_info;
	int ret;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	pEntry = &vc_entity;
	if( getATMVCEntry(name, pEntry, &chainidx) < 0 )
		return ERR_9002;

	if(ifGetName( pEntry->ifIndex, ifname, 16 )==0) return ERR_9002;

	*type = entity->info->type;
	*data = NULL;

	if (strcmp(lastname, "X_TELEFONICA-ES_IPv6Enabled") == 0)
	{
#ifdef CONFIG_IPV6
		*data = booldup((pEntry->IpProtocol & IPVER_IPV6) != 0);
#else
		*data = booldup(0);
#endif
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_IPv4Enabled") == 0) {
#ifdef CONFIG_IPV6
		*data = booldup(pEntry->IpProtocol == IPVER_IPV4_IPV6);
#else
		*data = booldup(0);
#endif
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_UnnumberedModel") == 0)
	{
		*data = booldup(pEntry->ipunnumbered);
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_IPv6ConnStatus") == 0)
	{
		int flags;

		if (pEntry->enable == 0 || getInFlags(ifname, &flags) == 0
				|| (flags & IFF_RUNNING) == 0) {
			*data = strdup("Disconnected");
		} else {
			*data = strdup("Connected");
		}
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_IPv6PppUp") == 0)
	{
		int flags;

		if (pEntry->enable == 0 || getInFlags(ifname, &flags) == 0
				|| (flags & IFF_UP) == 0) {
			*data = strdup("Down");
		} else {
			*data = strdup("Up");
		}
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_IPv6AddressingType") == 0)
	{
#ifdef CONFIG_IPV6
		buf[0] = '\0';

		if (pEntry->AddrMode & 1)
			strcat(buf, "SLACC, ");
		if (pEntry->AddrMode & 2)
			strcat(buf, "STATIC, ");
		if (pEntry->Ipv6Dhcp) {
			strcat(buf, "DHCP");
		} else if (pEntry->AddrMode) {
			/* clear up ", " */
			buf[strlen(buf) - 2] = '\0';
		}

		*data = strdup(buf);
#else
		*data = strdup("");
#endif
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_DHCP6cForAddress") == 0)
	{
#ifdef CONFIG_IPV6
		*data = booldup(pEntry->Ipv6Dhcp);
#else
		*data = booldup(0);
#endif
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_ExternalIPv6Address") == 0)
	{
#ifdef CONFIG_IPV6
		inet_ntop(AF_INET6, pEntry->Ipv6Addr, buf, sizeof(buf));
		*data = strdup(buf);
#else
		*data = strdup("");
#endif
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_DefaultIPv6Gateway") == 0) {
#ifdef CONFIG_IPV6
		inet_ntop(AF_INET6, pEntry->RemoteIpv6Addr, buf, sizeof(buf));
		*data = strdup(buf);
#else
		*data = strdup("");
#endif
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_IPv6DNSServers") == 0)
	{
		getNameServers(buf);
		*data = strdup(buf);
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_IPv6PrefixDelegationEnabled") == 0)
	{
		ret = getLeasesInfo(ifname, &dlg_info);
		*data = booldup(ret);
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_IPv6SitePrefix") == 0)
	{
		const char *dst;

		ret = getLeasesInfo(ifname, &dlg_info);
		dst = inet_ntop(AF_INET6, dlg_info.prefixIP, buf, sizeof(buf));
		*data = strdup(ret && dst ? dst : "");
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_IPv6SitePrefixPltime") == 0)
	{
		ret = getLeasesInfo(ifname, &dlg_info);
		*data = intdup(ret ? dlg_info.PLTime : 0);
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_IPv6SitePrefixVLtime") == 0)
	{
		ret = getLeasesInfo(ifname, &dlg_info);
		*data = intdup(ret ? dlg_info.MLTime : 0);
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_ExternalIPv6AddressPrefixLength") == 0)
	{
#ifdef CONFIG_IPV6
		*data = uintdup(pEntry->Ipv6AddrPrefixLen);
#else
		*data = uintdup(0);
#endif
	}
	else{
		return ERR_9005;
	}

	return 0;
}

int setBHSWANPPPCONENTITY(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	int *pInt = data;
	int *pUint = data;
	unsigned int	chainidx;
	MIB_CE_ATM_VC_T *pEntry, vc_entity;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	pEntry = &vc_entity;
	if( getATMVCEntry(name, pEntry, &chainidx) < 0 )
		return ERR_9002;

	gWanBitMap |= (1U << chainidx);

	if (strcmp(lastname, "X_TELEFONICA-ES_IPv6Enabled") == 0)
	{
#ifdef CONFIG_IPV6
		pEntry->IpProtocol = *pInt ? pEntry->IpProtocol | IPVER_IPV6 : pEntry->IpProtocol & ~IPVER_IPV6;
		mib_chain_update(MIB_ATM_VC_TBL, pEntry, chainidx);
		return CWMP_NEED_RESTART_WAN;
#else
		return ERR_9003;
#endif
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_IPv4Enabled") == 0)
	{
#ifdef CONFIG_IPV6
		pEntry->IpProtocol = *pInt ? IPVER_IPV4_IPV6 : pEntry->IpProtocol & ~IPVER_IPV4;
		mib_chain_update(MIB_ATM_VC_TBL, pEntry, chainidx);
		return CWMP_NEED_RESTART_WAN;
#else
		return ERR_9003;
#endif
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_UnnumberedModel") == 0)
	{
		pEntry->ipunnumbered = *pInt != 0;
		mib_chain_update(MIB_ATM_VC_TBL, pEntry, chainidx);
		return CWMP_NEED_RESTART_WAN;
	}
	if (strcmp(lastname, "X_TELEFONICA-ES_DHCP6cForAddress") == 0)
	{
#ifdef CONFIG_IPV6
		pEntry->Ipv6Dhcp = *pInt != 0;
		mib_chain_update(MIB_ATM_VC_TBL, pEntry, chainidx);
		return CWMP_NEED_RESTART_WAN;
#else
		return ERR_9003;
#endif
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_ExternalIPv6Address") == 0)
	{
#ifdef CONFIG_IPV6
		if ((pEntry->AddrMode & 0x2) == 0)
			return ERR_9001;
		inet_pton(AF_INET6, buf, pEntry->Ipv6Addr);
		mib_chain_update(MIB_ATM_VC_TBL, pEntry, chainidx);
		return CWMP_NEED_RESTART_WAN;
#else
		return ERR_9003;
#endif
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_DefaultIPv6Gateway") == 0)
	{
#ifdef CONFIG_IPV6
		inet_pton(AF_INET6, buf, pEntry->RemoteIpv6Addr);
		mib_chain_update(MIB_ATM_VC_TBL, pEntry, chainidx);
		return CWMP_NEED_RESTART_WAN;
#else
		return ERR_9003;
#endif
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_IPv6DNSServers") == 0)
	{
		setNameServers(buf);
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_IPv6SitePrefix") == 0)
	{
		/* ignored */
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_IPv6SitePrefixPltime") == 0)
	{
		/* ignored */
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_IPv6SitePrefixVLtime") == 0)
	{
		/* ignored */
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_ExternalIPv6AddressPrefixLength") == 0)
	{
#ifdef CONFIG_IPV6
		if ((pEntry->AddrMode & 0x1) == 0)
			return ERR_9001;
		pEntry->Ipv6AddrPrefixLen = *pUint;
		mib_chain_update(MIB_ATM_VC_TBL, pEntry, chainidx);
		return CWMP_NEED_RESTART_WAN;
#else
		return ERR_9003;
#endif
	}
	else{
		return ERR_9005;
	}

	return CWMP_APPLIED;
}





/*****************************************************************************
 * InternetGatewayDevice.WANDevice.{i}.WANConnectionDevice.{i}.WANIPConnection.{i}.
 *****************************************************************************/
struct CWMP_OP tBHSWANIPCONENTITYLeafOP = { getBHSWANIPCONENTITY, setBHSWANIPCONENTITY };
struct CWMP_PRMT tBHSWANIPCONENTITYLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"X_TELEFONICA-ES_IPv6Enabled",		eCWMP_tBOOLEAN,	CWMP_READ | CWMP_WRITE,		&tBHSWANIPCONENTITYLeafOP},
{"X_TELEFONICA-ES_IPv4Enabled",		eCWMP_tBOOLEAN,	CWMP_READ | CWMP_WRITE,		&tBHSWANIPCONENTITYLeafOP},
{"X_TELEFONICA-ES_UnnumberedModel",	eCWMP_tBOOLEAN,	CWMP_READ | CWMP_WRITE,		&tBHSWANIPCONENTITYLeafOP},
{"X_TELEFONICA-ES_IPv6ConnStatus",	eCWMP_tSTRING,	CWMP_READ,			&tBHSWANIPCONENTITYLeafOP},
{"X_TELEFONICA-ES_IPv6AddressingType",	eCWMP_tSTRING,	CWMP_READ,			&tBHSWANIPCONENTITYLeafOP},
{"X_TELEFONICA-ES_DHCP6cForAddress",	eCWMP_tBOOLEAN,	CWMP_READ | CWMP_WRITE,		&tBHSWANIPCONENTITYLeafOP},
{"X_TELEFONICA-ES_ExternalIPv6Address",	eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,		&tBHSWANIPCONENTITYLeafOP},
{"X_TELEFONICA-ES_DefaultIPv6Gateway",	eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,		&tBHSWANIPCONENTITYLeafOP},
{"X_TELEFONICA-ES_IPv6DNSServers",	eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,		&tBHSWANIPCONENTITYLeafOP},
{"X_TELEFONICA-ES_IPv6PrefixDelegationEnabled",	eCWMP_tBOOLEAN,	CWMP_READ,		&tBHSWANIPCONENTITYLeafOP},
{"X_TELEFONICA-ES_IPv6SitePrefix",	eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,		&tBHSWANIPCONENTITYLeafOP},
{"X_TELEFONICA-ES_IPv6SitePrefixPltime",eCWMP_tINT,	CWMP_READ | CWMP_WRITE,		&tBHSWANIPCONENTITYLeafOP},
{"X_TELEFONICA-ES_IPv6SitePrefixVLtime",eCWMP_tINT,	CWMP_READ | CWMP_WRITE,		&tBHSWANIPCONENTITYLeafOP},
{"X_TELEFONICA-ES_ExternalIPv6AddressPrefixLength",	eCWMP_tUINT,	CWMP_READ | CWMP_WRITE,	&tBHSWANIPCONENTITYLeafOP},
};

int getBHSWANIPCONENTITY(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned int	chainidx;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;
	char	ifname[16];
	char	buf[512]="";
	unsigned char vChar=0;
	DLG_INFO_T dlg_info;
	int ret;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	pEntry = &vc_entity;
	if( getATMVCEntry(name, pEntry, &chainidx) < 0 )
		return ERR_9002;

	if(ifGetName( pEntry->ifIndex, ifname, 16 )==0) return ERR_9002;

	*type = entity->info->type;
	*data = NULL;

	if (strcmp(lastname, "X_TELEFONICA-ES_IPv6Enabled") == 0)
	{
#ifdef CONFIG_IPV6
		*data = booldup((pEntry->IpProtocol & IPVER_IPV6) != 0);
#else
		*data = booldup(0);
#endif
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_IPv4Enabled") == 0)
	{
#ifdef CONFIG_IPV6
		*data = booldup(pEntry->IpProtocol == IPVER_IPV4_IPV6);
#else
		*data = booldup(0);
#endif
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_UnnumberedModel") == 0)
	{
		*data = booldup(pEntry->ipunnumbered);
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_IPv6ConnStatus") == 0)
	{
		int flags;

		if (pEntry->enable == 0 || getInFlags(ifname, &flags) == 0
				|| (flags & IFF_RUNNING) == 0) {
			*data = strdup("Disconnected");
		} else {
			*data = strdup("Connected");
		}
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_IPv6AddressingType") == 0)
	{
#ifdef CONFIG_IPV6
		buf[0] = '\0';

		if (pEntry->AddrMode & 1)
			strcat(buf, "SLACC, ");
		if (pEntry->AddrMode & 2)
			strcat(buf, "STATIC, ");
		if (pEntry->Ipv6Dhcp) {
			strcat(buf, "DHCP");
		} else if (pEntry->AddrMode) {
			/* clear up ", " */
			buf[strlen(buf) - 2] = '\0';
		}

		*data = strdup(buf);
#else
		*data = strdup("");
#endif
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_DHCP6cForAddress") == 0)
	{
#ifdef CONFIG_IPV6
		*data = booldup(pEntry->Ipv6Dhcp);
#else
		*data = booldup(0);
#endif
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_ExternalIPv6Address") == 0)
	{
#ifdef CONFIG_IPV6
		inet_ntop(AF_INET6, pEntry->Ipv6Addr, buf, sizeof(buf));
		*data = strdup(buf);
#else
		*data = strdup("");
#endif
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_DefaultIPv6Gateway") == 0)
	{
#ifdef CONFIG_IPV6
		inet_ntop(AF_INET6, pEntry->RemoteIpv6Addr, buf, sizeof(buf));
		*data = strdup(buf);
#else
		*data = strdup("");
#endif
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_IPv6DNSServers") == 0)
	{
		getNameServers(buf);
		*data = strdup(buf);
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_IPv6PrefixDelegationEnabled") == 0)
	{
		ret = getLeasesInfo(ifname, &dlg_info);
		*data = booldup(ret);
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_IPv6SitePrefix") == 0)
	{
		const char *dst;

		ret = getLeasesInfo(ifname, &dlg_info);
		dst = inet_ntop(AF_INET6, dlg_info.prefixIP, buf, sizeof(buf));
		*data = strdup(ret && dst ? dst : "");
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_IPv6SitePrefixPltime") == 0)
	{
		ret = getLeasesInfo(ifname, &dlg_info);
		*data = intdup(ret ? dlg_info.PLTime : 0);
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_IPv6SitePrefixVLtime") == 0)
	{
		ret = getLeasesInfo(ifname, &dlg_info);
		*data = intdup(ret ? dlg_info.MLTime : 0);
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_ExternalIPv6AddressPrefixLength") == 0)
	{
#ifdef CONFIG_IPV6
		*data = uintdup(pEntry->Ipv6AddrPrefixLen);
#else
		*data = uintdup(0);
#endif
	}
	else{
		return ERR_9005;
	}

	return 0;
}

int setBHSWANIPCONENTITY(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	int *pInt = data;
	int *pUint = data;
	unsigned int	chainidx;
	MIB_CE_ATM_VC_T *pEntry, vc_entity;
	DLG_INFO_T dlg_info;
	int ret;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	pEntry = &vc_entity;
	if( getATMVCEntry(name, pEntry, &chainidx) < 0 )
		return ERR_9002;

	gWanBitMap |= (1U << chainidx);

	if (strcmp(lastname, "X_TELEFONICA-ES_IPv6Enabled") == 0)
	{
#ifdef CONFIG_IPV6
		pEntry->IpProtocol = *pInt ? pEntry->IpProtocol | IPVER_IPV6 : pEntry->IpProtocol & ~IPVER_IPV6;
		mib_chain_update(MIB_ATM_VC_TBL, pEntry, chainidx);
		return CWMP_NEED_RESTART_WAN;
#else
		return ERR_9003;
#endif
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_IPv4Enabled") == 0)
	{
#ifdef CONFIG_IPV6
		pEntry->IpProtocol = *pInt ? IPVER_IPV4_IPV6 : pEntry->IpProtocol & ~IPVER_IPV4;
		mib_chain_update(MIB_ATM_VC_TBL, pEntry, chainidx);
		return CWMP_NEED_RESTART_WAN;
#else
		return ERR_9003;
#endif
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_UnnumberedModel") == 0)
	{
		pEntry->ipunnumbered = *pInt != 0;
		mib_chain_update(MIB_ATM_VC_TBL, pEntry, chainidx);
		return CWMP_NEED_RESTART_WAN;
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_DHCP6cForAddress") == 0)
	{
#ifdef CONFIG_IPV6
		pEntry->Ipv6Dhcp = *pInt != 0;
		mib_chain_update(MIB_ATM_VC_TBL, pEntry, chainidx);
		return CWMP_NEED_RESTART_WAN;
#else
		return ERR_9003;
#endif
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_ExternalIPv6Address") == 0)
	{
#ifdef CONFIG_IPV6
		if ((pEntry->AddrMode & 0x2) == 0)
			return ERR_9001;
		inet_pton(AF_INET6, buf, pEntry->Ipv6Addr);
		mib_chain_update(MIB_ATM_VC_TBL, pEntry, chainidx);
		return CWMP_NEED_RESTART_WAN;
#else
		return ERR_9003;
#endif
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_DefaultIPv6Gateway") == 0)
	{
#ifdef CONFIG_IPV6
		inet_pton(AF_INET6, buf, pEntry->RemoteIpv6Addr);
		mib_chain_update(MIB_ATM_VC_TBL, pEntry, chainidx);
		return CWMP_NEED_RESTART_WAN;
#else
		return ERR_9003;
#endif
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_IPv6DNSServers") == 0)
	{
		setNameServers(buf);
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_IPv6SitePrefix") == 0)
	{
		/* ignored */
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_IPv6SitePrefixPltime") == 0)
	{
		/* ignored */
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_IPv6SitePrefixVLtime") == 0)
	{
		/* ignored */
	}
	else if (strcmp(lastname, "X_TELEFONICA-ES_ExternalIPv6AddressPrefixLength") == 0)
	{
#ifdef CONFIG_IPV6
		if ((pEntry->AddrMode & 0x1) == 0)
			return ERR_9001;
		pEntry->Ipv6AddrPrefixLen = *pUint;
		mib_chain_update(MIB_ATM_VC_TBL, pEntry, chainidx);
		return CWMP_NEED_RESTART_WAN;
#else
		return ERR_9003;
#endif
	}
	else
		return ERR_9005;

	return CWMP_APPLIED;
}

