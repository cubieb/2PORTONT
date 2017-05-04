#ifndef _PRMT_LANDEVICE_H_
#define _PRMT_LANDEVICE_H_

#include "prmt_igd.h"

#ifdef __cplusplus
extern "C" {
#endif

//extern struct sCWMP_ENTITY tForwardingEntity[];
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
extern struct CWMP_LEAF tLANInterfacesLeaf[];
#endif
/*ping_zhang:20081217 END*/
extern struct CWMP_LEAF tHostEntityLeaf[];
extern struct CWMP_LINKNODE tHostObject[];
extern struct CWMP_LEAF tHostsLeaf[];
extern struct CWMP_NODE tHostsObject[];
extern struct CWMP_LEAF tLANEthStatsLeaf[];
extern struct CWMP_LEAF tLANEthConfEntityLeaf[];
extern struct CWMP_NODE tLANEthConfEntityObject[];
extern struct CWMP_LEAF tIPInterfaceEntityLeaf[];
extern struct CWMP_NODE tIPInterfaceObject[];
extern struct CWMP_LEAF tLANHostConfLeaf[];
extern struct CWMP_NODE tLANHostConfObject[];
extern struct CWMP_LEAF tLANDeviceEntityLeaf[];
extern struct CWMP_NODE tLANDeviceEntityObject[];
extern struct CWMP_NODE tLANDeviceObject[];


/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
int getLANInterfaces(char *name, struct CWMP_LEAF *entity, int *type, void **data);
#endif
/*ping_zhang:20081217 END*/

int getHostEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);

int getHosts(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int objHosts(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getIPItfEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setIPItfEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

/*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
int getDHCPOptionEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setDHCPOptionEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);
int objDHCPOption(char *name, struct CWMP_LEAF *e, int type, void *data);
int getDHCPConSPEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setDHCPConSPEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);
int objDHCPConSP(char *name, struct CWMP_LEAF *e, int type, void *data);
#endif
/*ping_zhang:20080919 END*/

int getLANHostConf(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setLANHostConf(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getLDEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);

/*utility*/
int getInterfaceStat(
	char *ifname,
	unsigned long *bs,
	unsigned long *br,
	unsigned long *ps,
	unsigned long *pr );
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
int getInterfaceStat1(
	char *ifname,
	unsigned long *es, unsigned long *er,
	unsigned long *ups, unsigned long *upr, 
	unsigned long *dps, unsigned long *dpr, 
	unsigned long *mps, unsigned long *mpr, 
	unsigned long *bps, unsigned long *bpr, 
	unsigned long *uppr);
#endif
/*ping_zhang:20081217 END*/
	

#ifdef SUPPORT_DHCP_RESERVED_IPADDR
#define FILE4DHCPReservedIPAddr	"/tmp/DHCPReservedIPAddrTR069TMP.txt"
int getDHCPReservedIPAddr( unsigned int inst_num, char *pfilename );
int setDHCPReservedIPAddr( unsigned int inst_num, char *list );
#endif //SUPPORT_DHCP_RESERVED_IPADDR



#ifdef __cplusplus
}
#endif

#endif /*_PRMT_LANDEVICE_H_*/
