/* $Id: upnpdescgen.h,v 1.5 2012/08/01 04:27:13 adsmt Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2006-2008 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#ifndef __UPNPDESCGEN_H__
#define __UPNPDESCGEN_H__

#include "config.h"

/* for the root description
 * The child list reference is stored in "data" member using the
 * INITHELPER macro with index/nchild always in the
 * same order, whatever the endianness */
struct XMLElt {
	const char * eltname;	/* begin with '/' if no child */
	const char * data;	/* Value */
};

/* for service description */
struct serviceDesc {
	const struct action * actionList;
	const struct stateVar * serviceStateTable;
};

struct action {
	const char * name;
	const struct argument * args;
};

struct argument {
	const char * name;
	unsigned short dir;
	unsigned short relatedVar;
};

struct stateVar {
	const char * name;
	unsigned char itype;	/* MSB is sendEvent flag */
	unsigned char idefault;
	unsigned char iallowedlist;
};

/* little endian
 * The code has now be tested on big endian architecture */
#define INITHELPER(i, n) ((char *)((n<<16)|i))

/* char * genRootDesc(int *);
 * returns: NULL on error, string allocated on the heap */
char *genRootDesc(int * len);
char *genWANDSLLink(int * len);
/* for the two following functions */
char *genWANIPCn(int * len);
char *genWANCfg(int * len);

#ifdef ENABLE_L3F_SERVICE
char *genL3F(int * len);
#endif

#ifdef CONFIG_TR_064
char *genDeviceInfo(int *len);
char *genDeviceConfig(int *len);
char *genLayer3Forwarding(int *len);
char *genLANConfigSecurity(int *len);
char *genManagementServer(int *len);
char *genTime(int *len);

char *genLANHostConfigManagement(int *len);
char *genLANEthernetInterfaceConfig(int *len);
char *genWLANConfiguration(int *len);
char *genHosts(int *len);

char *genWANDSLConnectionManagement(int *len);
char *genWANDSLInterfaceConfig(int *len);
char *genWANDSLLinkConfig(int *len);
char *genWANEthernetInterfaceConfig(int *len);
char *genWANPPPCn(int *len);
#endif

#endif

