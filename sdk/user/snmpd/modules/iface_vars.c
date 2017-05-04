
#include <net/if.h>
#include	<stdio.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<time.h>

#include	"ctypes.h"
#include	"error.h"
#include	"local.h"
#include	"iface_vars.h"
#include	"mix.h"
#include	"mis.h"
#include	"asn.h"

#include "mib_tool.h"

// for Interface Scan
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/if_ether.h>

#include <sys/ioctl.h>

// for Interface Mib Init
#include <sys/time.h>

#define IFACE_MAXTYPE 1
#define IFACE_MAXTYPE21 22
static	CUnslType ifaceAddr;

struct ifnet {
	char	*if_name;		/* name, e.g. ``en1'' or ``lo'' */
  /* 	short	if_unit;		/ * sub-unit for lower level driver */
	short	if_mtu;			/* maximum transmission unit */
	short	if_flags;		/* up/down, broadcast, etc. */
	int	if_metric;		/* routing metric (external only) */
	char    if_hwaddr [6];		/* ethernet address */
	int	if_type;		/* interface type: 1=generic,
					   28=slip, ether=6, loopback=24,
					   7=802.3, 23=ppp */
	int	if_speed;		/* interface speed: in bits/sec */

	struct sockaddr if_addr;	/* interface's address */
	struct sockaddr ifu_broadaddr;	/* broadcast address */
	struct sockaddr ia_subnetmask; 	/* interface's mask */

	struct	ifqueue {
		int	ifq_len;
		int	ifq_drops;
	} if_snd;			/* output queue */

        /* ibytes and obytes added for cmu snmp linux v3.4: */
        unsigned long if_ibytes;	/* # of bytes received */
        unsigned long if_obytes;	/* # of bytes sent */

        unsigned long if_ipackets;	/* packets received on interface */
	unsigned long if_opackets;	/* packets sent on interface */

	unsigned long if_ierrors;	/* input errors on interface */
	unsigned long if_oerrors;	/* output errors on interface */

        /* note: collisions are filled but not used */
	unsigned long if_collisions;	/* collisions on csma interfaces */

	unsigned long if_idrop;		/* discard on input */
	unsigned long if_odrop;		/* discard on output */
/* end statistics */
	struct	ifnet *if_next;
};

struct mib_ifEntry {
    long    ifIndex;	    /* index of this interface	*/
    char    ifDescr[32];    /* english description of interface	*/
    long    ifType;	    /* network type of device	*/
    long    ifMtu;	    /* size of largest packet in bytes	*/
    u_long  ifSpeed;	    /* bandwidth in bits/sec	*/
    u_char  ifPhysAddress[11];	/* interface's address */
    u_char  PhysAddrLen;    /* length of physAddr */
    long    ifAdminStatus;  /* desired state of interface */
    long    ifOperStatus;   /* current operational status */
    u_long  ifLastChange;   /* value of sysUpTime when current state entered */
    u_long  ifInOctets;	    /* number of octets received on interface */
    u_long  ifInUcastPkts;  /* number of unicast packets delivered */
    u_long  ifInNUcastPkts; /* number of broadcasts or multicasts */
    u_long  ifInDiscards;   /* number of packets discarded with no error */
    u_long  ifInErrors;	    /* number of packets containing errors */
    u_long  ifInUnknownProtos;	/* number of packets with unknown protocol */
    u_long  ifOutOctets;    /* number of octets transmitted */
    u_long  ifOutUcastPkts; /* number of unicast packets sent */
    u_long  ifOutNUcastPkts;/* number of broadcast or multicast pkts */
    u_long  ifOutDiscards;  /* number of packets discarded with no error */
    u_long  ifOutErrors;    /* number of pkts discarded with an error */
    u_long  ifOutQLen;	    /* number of packets in output queue */
};

static struct ifnet *ifnetaddr_list;
static unsigned int ifnetaddr_list_num;
static struct mib_oid_tbl interface_mib_oid_tbl;

/*
 * if_type_from_name
 * Return interface type using the interface name as a clue.
 * Returns 1 to imply "other" type if name not recognized. 
 */
static int
if_type_from_name(const char *pcch)
{
    typedef struct _match_if {
        int             mi_type;
        const char     *mi_name;
    }              *pmatch_if, match_if;

    static match_if lmatch_if[] = {
        {24, "lo"},
        {6, "eth"},
        {9, "tr"},
        {23, "ppp"},
        {28, "sl"},
        {0, 0}                  /* end of list */
    };

    int             ii, len;
    register pmatch_if pm;

    for (ii = 0, pm = lmatch_if; pm->mi_name; pm++) {
        len = strlen(pm->mi_name);
        if (0 == strncmp(pcch, pm->mi_name, len)) {
            return (pm->mi_type);
        }
    }
    return (1);                 /* in case search fails */
}

/**
* Determines network interface speed. It is system specific. Only linux
* realization is made. 
*/
unsigned int getIfSpeed(int fd, struct ifreq ifr){
	unsigned int retspeed = 10000000;
/* the code is based on mii-diag utility by Donald Becker
* see ftp://ftp.scyld.com/pub/diag/mii-diag.c
*/
	ushort *data = (ushort *)(&ifr.ifr_data);
	unsigned int *data32 = (unsigned int *)(&ifr.ifr_data);
	unsigned phy_id=1;
	unsigned char new_ioctl_nums = 0;
	int mii_reg, i;
	ushort mii_val[32];
	ushort bmcr, bmsr, new_bmsr, nway_advert, lkpar;
	const unsigned int media_speeds[] = {10000000, 10000000, 100000000, 100000000, 10000000, 0};	
/* It corresponds to "10baseT", "10baseT-FD", "100baseTx", "100baseTx-FD", "100baseT4", "Flow-control", 0, */


	if (ioctl(fd, 0x8947, &ifr) >= 0) {
		phy_id = data32[1];
		new_ioctl_nums = 1;
	} else if (ioctl(fd, SIOCDEVPRIVATE, &ifr) >= 0) {
		new_ioctl_nums = 0;
	} else {
		printf("mibII/interfaces: SIOCGMIIPHY on %s failed\n", ifr.ifr_name);
		return retspeed;
	}
/* Begin getting mii register values */
	for (mii_reg = 0; mii_reg < 8; mii_reg++){
		data[0] = phy_id;
		data[1] = mii_reg;
		if(ioctl(fd, new_ioctl_nums ? 0x8948 : SIOCDEVPRIVATE+1, &ifr) <0){
			printf("mibII/interfaces: SIOCGMIIREG on %s failed\n", ifr.ifr_name);
		}
		mii_val[mii_reg] = data[3];		
	}
/*Parsing of mii values*/
/*Invalid basic mode control register*/
	if (mii_val[0] == 0xffff  ||  mii_val[1] == 0x0000) {
		printf("mibII/interfaces: No MII transceiver present!.\n");
		return retspeed;
	}
	/* Descriptive rename. */
	bmcr = mii_val[0]; 	  /*basic mode control register*/
	bmsr = mii_val[1]; 	  /* basic mode status register*/
	nway_advert = mii_val[4]; /* autonegotiation advertisement*/
	lkpar = mii_val[5]; 	  /*link partner ability*/
	
/*Check for link existence, returns 0 if link is absent*/
	if ((bmsr & 0x0016) != 0x0004){
//		printf("mibII/interfaces: No link...\n");
		retspeed = 0;
		return retspeed;
	}
	
	if(!(bmcr & 0x1000) ){
//		printf("mibII/interfaces: Auto-negotiation disabled.\n");
		retspeed = bmcr & 0x2000 ? 100000000 : 10000000;
		return retspeed;
	}
/* Link partner got our advertised abilities */	
	if (lkpar & 0x4000) {
		int negotiated = nway_advert & lkpar & 0x3e0;
		int max_capability = 0;
		/* Scan for the highest negotiated capability, highest priority
		   (100baseTx-FDX) to lowest (10baseT-HDX). */
		int media_priority[] = {8, 9, 7, 6, 5}; 	/* media_names[i-5] */
		for (i = 0; media_priority[i]; i++){
			if (negotiated & (1 << media_priority[i])) {
				max_capability = media_priority[i];
				break;
			}
		}
		if (max_capability)
			retspeed = media_speeds[max_capability - 5];
		else
			printf("mibII/interfaces: No common media type was autonegotiated!\n");
	}
	return retspeed;
}


void
Interface_Scan(void)
{
	int fd;
	FILE           *devin;
	char            line[256];
	struct ifnet  **ifnetaddr_ptr;

	const char     *scan_line_to_use =
		"%lu %lu %lu %lu %*lu %*lu %*lu %*lu %lu %lu %lu %lu";

	/*
	 * free old list: 
	 */
	while (ifnetaddr_list) {
	    struct ifnet   *old = ifnetaddr_list;
	    ifnetaddr_list = ifnetaddr_list->if_next;
	    free(old->if_name);
//	    free(old->if_unit);
	    free(old);
	}
	ifnetaddr_ptr = &ifnetaddr_list;
	ifnetaddr_list_num = 0;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("Error: socket open failure in Interface_Scan_Init\n");
		return;
	}

	if (!(devin = fopen("/proc/net/dev", "r"))) {
		close(fd);
		printf("Error: cannot open /proc/net/dev - continuing...\n");
		return;
	}

	fgets(line, sizeof(line), devin);
	fgets(line, sizeof(line), devin);

	if (strstr(line, "compressed") == NULL) {
		fclose(devin);
		close(fd);
		printf("Error: /proc/net/dev NOT compressed...\n");
		return;
	}

	while (fgets(line, sizeof(line), devin)) {
		struct ifnet *nnew;
    		struct ifreq    ifrq;

		char ifname_buf[64];
		char *ifname, *ptr;
		unsigned long   rec_pkt, rec_oct, rec_err, rec_drop;
		unsigned long   snd_pkt, snd_oct, snd_err, snd_drop;

		char *stats;
		char *ifstart = line;

		// find the start character
		while (*ifstart && *ifstart == ' ')
			ifstart++;
		stats = strrchr(ifstart, ':');

		// copy interface name
		*stats = 0;
		strncpy(ifname_buf, ifstart, sizeof(ifname_buf));
		ifname_buf[ sizeof(ifname_buf)-1 ] = 0;
		*stats++ = ':';

		// find the start item
		while (*stats == ' ')
			stats++;

		if(sscanf(stats, scan_line_to_use,
			&rec_oct, &rec_pkt, &rec_err, &rec_drop,
			&snd_oct, &snd_pkt, &snd_err, &snd_drop) != 8)
		{
			fclose(devin);
			close(fd);
			printf("Error: No statistics available\n");
			return;
		}

		nnew = (struct ifnet *) calloc(1, sizeof(struct ifnet));
		if (nnew == NULL)
		    break;              /* alloc error */

		/*
		 * chain in: 
		 */
		*ifnetaddr_ptr = nnew;
		ifnetaddr_ptr = &nnew->if_next;
		ifnetaddr_list_num++;

		/*
		 * linux previous to 1.3.~13 may miss transmitted loopback pkts: 
		 */
		if (!strcmp(ifname_buf, "lo") && rec_pkt > 0 && !snd_pkt)
			snd_pkt = rec_pkt;

		nnew->if_ibytes = rec_oct;
		nnew->if_ipackets = rec_pkt;
		nnew->if_ierrors = rec_err;
		nnew->if_idrop = rec_drop;
		
		nnew->if_obytes = snd_oct;
		nnew->if_opackets = snd_pkt;
		nnew->if_oerrors = snd_err;
		nnew->if_odrop = snd_drop;

		/*
		 * ifnames are given as ``   eth0'': split in ``eth'' and ``0'': 
		 */
		for (ifname = ifname_buf; *ifname && *ifname == ' '; ifname++);

		/*
		 * set name and interface# : 
		 */
		nnew->if_name = (char *) strdup(ifname);
#if 0		
		for (ptr = nnew->if_name; *ptr && (*ptr < '0' || *ptr > '9');
		     ptr++);
		nnew->if_unit = strdup(*ptr ? ptr : "");
		*ptr = 0;
#endif

		strncpy(ifrq.ifr_name, ifname, sizeof(ifrq.ifr_name));
		ifrq.ifr_name[ sizeof(ifrq.ifr_name)-1 ] = 0;
		if (ioctl(fd, SIOCGIFADDR, &ifrq) < 0)
		    memset((char *) &nnew->if_addr, 0, sizeof(nnew->if_addr));
		else
		    nnew->if_addr = ifrq.ifr_addr;

		strncpy(ifrq.ifr_name, ifname, sizeof(ifrq.ifr_name));
		ifrq.ifr_name[ sizeof(ifrq.ifr_name)-1 ] = 0;
		if (ioctl(fd, SIOCGIFBRDADDR, &ifrq) < 0)
		    memset((char *) &nnew->ifu_broadaddr, 0,
		           sizeof(nnew->ifu_broadaddr));
		else
		    nnew->ifu_broadaddr = ifrq.ifr_broadaddr;

		strncpy(ifrq.ifr_name, ifname, sizeof(ifrq.ifr_name));
		ifrq.ifr_name[ sizeof(ifrq.ifr_name)-1 ] = 0;
		if (ioctl(fd, SIOCGIFNETMASK, &ifrq) < 0)
		    memset((char *) &nnew->ia_subnetmask, 0,
		           sizeof(nnew->ia_subnetmask));
		else
		    nnew->ia_subnetmask = ifrq.ifr_netmask;

		strncpy(ifrq.ifr_name, ifname, sizeof(ifrq.ifr_name));
		ifrq.ifr_name[ sizeof(ifrq.ifr_name)-1 ] = 0;
		nnew->if_flags = ioctl(fd, SIOCGIFFLAGS, &ifrq) < 0
		    ? 0 : ifrq.ifr_flags;

		nnew->if_type = 0;

		strncpy(ifrq.ifr_name, ifname, sizeof(ifrq.ifr_name));
		ifrq.ifr_name[ sizeof(ifrq.ifr_name)-1 ] = 0;
		if (ioctl(fd, SIOCGIFHWADDR, &ifrq) < 0)
		    memset(nnew->if_hwaddr, (0), 6);
		else {
		    memcpy(nnew->if_hwaddr, ifrq.ifr_hwaddr.sa_data, 6);

            		switch (ifrq.ifr_hwaddr.sa_family) {
				case ARPHRD_ETHER:
				    nnew->if_type = 6;
				    break;
				case ARPHRD_PPP:
				    nnew->if_type = 23;
				    break;          /* ppp */
				case ARPHRD_LOOPBACK:
				    nnew->if_type = 24;
				    break;          /* softwareLoopback */
				case ARPHRD_ATM:
				    nnew->if_type = 37;
				    break;
				default:
				    break;
            		}
		}

		strncpy(ifrq.ifr_name, ifname, sizeof(ifrq.ifr_name));
		ifrq.ifr_name[ sizeof(ifrq.ifr_name)-1 ] = 0;
		nnew->if_mtu = (ioctl(fd, SIOCGIFMTU, &ifrq) < 0)
		    ? 0 : ifrq.ifr_mtu;

		if (!nnew->if_type)
		    nnew->if_type = if_type_from_name(nnew->if_name);
		nnew->if_speed = nnew->if_type == 6 ? getIfSpeed(fd, ifrq) :
		    nnew->if_type == 24 ? 10000000 :
		    nnew->if_type == 9 ? 4000000 : 0;
		/*Zero speed means link problem*/
		if(nnew->if_speed == 0 && nnew->if_flags & IFF_UP){
		    nnew->if_flags &= ~IFF_RUNNING;
		}
/*
 		printf("Line: %s\n", line);
		printf("Name:%s, Type:%d, MTU:%d, Speed:%d, PHY:%2x-%2x-%2x-%2x-%2x-%2x\n"
			,nnew->if_name, nnew->if_type, nnew->if_mtu, nnew->if_speed
			,(unsigned char)nnew->if_hwaddr[0] ,(unsigned char)nnew->if_hwaddr[1] ,(unsigned char)nnew->if_hwaddr[2] ,(unsigned char)nnew->if_hwaddr[3] ,(unsigned char)nnew->if_hwaddr[4] ,(unsigned char)nnew->if_hwaddr[5]);
			
		printf("Admin:%d, Oper:%d, InO:%u, InP:%u, InE:%u, InD:%u\n"
			,nnew->if_flags ,nnew->if_flags ,nnew->if_ibytes ,nnew->if_ipackets ,nnew->if_ierrors ,nnew->if_iqdrops);

		printf("OutO:%u, OutP:%u, OutE:%u OutD:%u\n"
			,nnew->if_obytes ,nnew->if_opackets ,nnew->if_oerrors ,nnew->if_collisions);
*/ 		
	}

//	printf("Total:%d\n",ifnetaddr_list_num);

	fclose(devin);
	close(fd);
}

static long interface_last_scan = 0;

void Interface_Mib_Init(void)
{
	struct timeval now;
	
	gettimeofday(&now, 0);

//	printf("Now:%u:%u\n",now.tv_sec, now.tv_usec);

	if((now.tv_sec - interface_last_scan) > 10)	//  10 sec
	{
		struct ifnet *nnew;
		unsigned int idx, item;
		unsigned int oid_tbl_size;

		unsigned char oid_ifNumber[] = "\1\0";
		unsigned char oid_ifEntry[] = "\2\1";
		unsigned char oid_ifEntry_item[IFENTRYNUM] = 
			{ 	IFINDEX, 
				IFDESCR, 
				IFTYPE, 
				IFMTU, 
				IFSPEED, 
				IFPHYSADDRESS, 
				IFADMINSTATUS, 
				IFOPERSTATUS,
				IFINOCTETS, 
				IFINUCASTPKTS, 
				IFINDISCARDS, 
				IFINERRORS, 
				IFOUTOCTETS, 
				IFOUTUCASTPKTS, 
				IFOUTDISCARDS, 
				IFOUTERRORS		};

		struct mib_oid * oid_ptr;

		interface_last_scan = now.tv_sec;
		
		Interface_Scan();

		// create oid table
		free_mib_tbl(&interface_mib_oid_tbl);
		oid_tbl_size = 1 + IFENTRYNUM*ifnetaddr_list_num; // IFNUMBER + N * IFENTRY
		create_mib_tbl(&interface_mib_oid_tbl,  oid_tbl_size, 4);
		oid_ptr = interface_mib_oid_tbl.oid;

		// ifNumber
		oid_ptr->length = 2;
		memcpy(oid_ptr->name, oid_ifNumber, oid_ptr->length);
		oid_ptr++;

		// ifTable.ifEntry
		for(item=0;item<IFENTRYNUM;item++)
		{
			for(idx=1;idx<=ifnetaddr_list_num;idx++)
			{
				oid_ptr->length = 4;
				memcpy(oid_ptr->name, oid_ifEntry, oid_ptr->length-2);

				oid_ptr->name[2] = oid_ifEntry_item[item];
				oid_ptr->name[3] = (unsigned char)idx;

				oid_ptr++;
			}
		}
	}
}

unsigned int Adsl_Interface_Idx(void)
{
	struct ifnet *if_ptr;
	unsigned int i;
	Interface_Mib_Init();

	if_ptr = ifnetaddr_list;
	i = 1;

	while(if_ptr)
	{
		// "eth1" is adsl interface
	
		if(strcmp(if_ptr->if_name, "eth1") == 0)
		{
			return i;
		}		
	
		if_ptr = if_ptr->if_next;
		i++;
	}

	return 0;
}


static	AsnIdType	ifaceRetrieveMibValue (unsigned int mibIdx)
{
	// ifNumber
	if(mibIdx == 0)
	{
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, ifnetaddr_list_num);;	
	}
	// ifTable.ifEntry
	else if (mibIdx < interface_mib_oid_tbl.total)
	{
		unsigned int i;
		unsigned int ifNum;
		unsigned int item;
		struct ifnet *if_ptr;
		struct mib_oid * oid_ptr;
		oid_ptr = interface_mib_oid_tbl.oid;

		oid_ptr += mibIdx;

		item = oid_ptr->name[2];
		ifNum = oid_ptr->name[3];

		if_ptr = ifnetaddr_list;
		i = ifNum - 1;
		while(i > 0)
		{
			if(if_ptr->if_next == NULL)
			{
				printf("Error: ifaceRetrieveMibValue (if_ptr->if_next == NULL) \n");
				return ((AsnIdType) 0);	
			}
			if_ptr = if_ptr->if_next;
			
			i --;
		}

//		printf("ifaceRetrieveMibValue ifNum %d item %d mibIdx %d\n",ifNum,item, mibIdx );

		switch(item)
		{
			case IFINDEX:
				return asnUnsl (asnClassUniversal, (AsnTagType) 2, ifNum);	
			case IFDESCR:
				return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
					(CBytePtrType) (if_ptr->if_name), 
					(AsnLengthType) strlen ((char *) (if_ptr->if_name)));
			case IFTYPE:
				return asnUnsl (asnClassUniversal, (AsnTagType) 2, if_ptr->if_type);;	
			case IFMTU:
				return asnUnsl (asnClassUniversal, (AsnTagType) 2, (CUnslType) (if_ptr->if_mtu));	
			case IFSPEED:
				return asnUnsl (asnClassApplication, (AsnTagType) 2, (CUnslType) (if_ptr->if_speed));	
			case IFPHYSADDRESS:
				return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
					(CBytePtrType) (if_ptr->if_hwaddr), 
					(AsnLengthType) 6);

			case IFADMINSTATUS:
				return asnUnsl (asnClassUniversal, (AsnTagType) 2, if_ptr->if_flags& IFF_UP ? 1 : 2);;	
			case IFOPERSTATUS:
				return asnUnsl (asnClassUniversal, (AsnTagType) 2, if_ptr->if_flags& IFF_UP ? 1 : 2);;	

			case IFINOCTETS:
				return asnUnsl (asnClassApplication, (AsnTagType) 1, if_ptr->if_ibytes);;	
			case IFINUCASTPKTS:
				return asnUnsl (asnClassApplication, (AsnTagType) 1, if_ptr->if_ipackets);;	
			case IFINDISCARDS:
				return asnUnsl (asnClassApplication, (AsnTagType) 1, if_ptr->if_idrop);;	
			case IFINERRORS:
				return asnUnsl (asnClassApplication, (AsnTagType) 1, if_ptr->if_ierrors);;	
			case IFOUTOCTETS:
				return asnUnsl (asnClassApplication, (AsnTagType) 1, if_ptr->if_obytes);;	
			case IFOUTUCASTPKTS:
				return asnUnsl (asnClassApplication, (AsnTagType) 1, if_ptr->if_opackets);;	
			case IFOUTDISCARDS:
				return asnUnsl (asnClassApplication, (AsnTagType) 1, if_ptr->if_odrop);;
			case IFOUTERRORS:
				return asnUnsl (asnClassApplication, (AsnTagType) 1, if_ptr->if_oerrors);;	

			default:
			return ((AsnIdType) 0);	
				
		}
	}

	return ((AsnIdType) 0);	
}

static	MixStatusType	ifaceRelease (MixCookieType cookie)
{
	cookie = cookie;
	return (smpErrorGeneric);
}

static	MixStatusType	ifaceCreate (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen, AsnIdType asn)
{
	//printf("ifaceCreate ()\n");    
	cookie = cookie;
	name = name;
	namelen = namelen;
	asn = asn;
	return (smpErrorGeneric);
}

static	MixStatusType	ifaceDestroy (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen)
{
	//printf("ifaceDestroy ()\n");    
	cookie = cookie;
	name = name;
	namelen = namelen;
	return (smpErrorGeneric);
}

static	AsnIdType	ifaceGet (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen)
{
	unsigned int idx;

	Interface_Mib_Init();

	cookie = cookie;
	if(snmp_oid_get(&interface_mib_oid_tbl, name, namelen, &idx))
	{
		return ifaceRetrieveMibValue(idx);
	}

	return ((AsnIdType) 0);
}

static	MixStatusType	ifaceSet (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen, AsnIdType asn)
{
	unsigned int idx;

	Interface_Mib_Init();

	//printf("ifaceSet ()\n");    
	cookie = cookie;
	name = name;
	namelen = namelen;
	asn = asn;

	if(snmp_oid_get(&interface_mib_oid_tbl, name, namelen, &idx))
	{	// only interface.ifTable.ifEntry.ifAdminStatus is RW

		if ((idx > 0) && (idx < interface_mib_oid_tbl.total))
		{
			unsigned int i;
			unsigned int ifNum;
			unsigned int item;
			struct ifnet *if_ptr;
			struct mib_oid * oid_ptr;
			oid_ptr = interface_mib_oid_tbl.oid;

			oid_ptr += idx;

			item = oid_ptr->name[2];
			ifNum = oid_ptr->name[3];

			if_ptr = ifnetaddr_list;
			i = ifNum - 1;
			while(i > 0)
			{
				if(if_ptr->if_next == NULL)
				{
					printf("(if_ptr->if_next == NULL) \n");
					return ((AsnIdType) 0);	
				}
				if_ptr = if_ptr->if_next;
				
				i --;
			}

			if(item == IFADMINSTATUS)
			{
				if(asnType (asn) == asnTypeInteger)
				{
					long i =  asnNumber (asnValue (asn), asnLength (asn));

					if(i == 1)		// interface up
					{
						int fd;
						if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0) {
					    		struct ifreq    ifrq;
										
							strncpy(ifrq.ifr_name, if_ptr->if_name, sizeof(ifrq.ifr_name));
							ifrq.ifr_name[ sizeof(ifrq.ifr_name)-1 ] = 0;
							ifrq.ifr_flags = IFF_UP | IFF_RUNNING;
							
							ioctl(fd, SIOCSIFFLAGS, &ifrq);

							interface_last_scan = 0;	// force re-scan

							return (smpErrorNone);
						}
					}
					else
					if(i == 2) 	// interface down
					{
						int fd;
						if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0) {
					    		struct ifreq    ifrq;
										
							strncpy(ifrq.ifr_name, if_ptr->if_name, sizeof(ifrq.ifr_name));
							ifrq.ifr_name[ sizeof(ifrq.ifr_name)-1 ] = 0;
							ifrq.ifr_flags = 0;
							
							ioctl(fd, SIOCSIFFLAGS, &ifrq);

							interface_last_scan = 0;	// force re-scan

							return (smpErrorNone);
						}
					}
				}
				
				return (smpErrorGeneric);					
			}
		}
	}

	return (smpErrorReadOnly);
}

static	AsnIdType	ifaceNext (MixCookieType cookie, MixNamePtrType name, MixLengthPtrType namelenp)
{
	unsigned int idx;

	Interface_Mib_Init();

	cookie = cookie;
	if(snmp_oid_getnext(&interface_mib_oid_tbl, name, *namelenp, &idx))
	{
		struct mib_oid * oid_ptr = interface_mib_oid_tbl.oid;
		oid_ptr += idx;

		memcpy((unsigned char *)name, oid_ptr->name, oid_ptr->length);
		*namelenp = oid_ptr->length;
	
		return ifaceRetrieveMibValue(idx);
	}

	return ((AsnIdType) 0);
}

static	MixOpsType	ifaceOps = {

			ifaceRelease,
			ifaceCreate,
			ifaceDestroy,
			ifaceNext,
			ifaceGet,
			ifaceSet

			};

CVoidType		ifaceInit (void)
{
	(void) misExport ((MixNamePtrType) "\53\6\1\2\1\2",
		(MixLengthType) 6, & ifaceOps, (MixCookieType) 0);
}

