#ifndef _PRMT_WANCONDEVICE_H_
#define _PRMT_WANCONDEVICE_H_

#include "prmt_igd.h"

#ifdef __cplusplus
extern "C" {
#endif


/***************************************/
/*define wan device instance number & utilities*/
/***************************************/
extern unsigned int WANDEVNUM_ATM;
extern unsigned int WANDEVNUM_PTM;
extern unsigned int WANDEVNUM_ETH;
int update_WANDEVNUM(void);
static unsigned int getWanInstFromifIndex(unsigned int ifIndex);
static MEDIA_TYPE_T getMediaTypeFromWanInst(unsigned int instnum);
//int getWANDevMode(unsigned int instnum);
int isWANDevModeMatch(unsigned int instnum, unsigned int if_idx);
/***************************************/
unsigned int getInstNum( char *name, char *objname );
unsigned int getWANDevInstNum( char *name );
unsigned int getWANConDevInstNum( char *name );
unsigned int getWANPPPConInstNum( char *name );
unsigned int getWANIPConInstNum( char *name );
/***************************************/


extern struct CWMP_LEAF tWANCONSTATSLeaf[];
extern struct CWMP_LEAF tPORTMAPENTITYLeaf[];
extern struct CWMP_LINKNODE tWANPORTMAPObject[];
extern struct CWMP_LEAF tWANPPPCONENTITYLeaf[];
extern struct CWMP_NODE tWANPPPCONENTITYObject[];
extern struct CWMP_LINKNODE tWANPPPCONObject[];
extern struct CWMP_LEAF tWANIPCONENTITYLeaf[];
extern struct CWMP_NODE tWANIPCONENTITYObject[];
extern struct CWMP_LINKNODE tWANIPCONObject[];
extern struct CWMP_LEAF tDSLLNKCONFLeaf[];
extern struct CWMP_LEAF tWANCONDEVENTITYLeaf[];
extern struct CWMP_NODE tWANCONDEVENTITYObjectDSL[];
extern struct CWMP_LINKNODE tWANCONDEVObjectDSL[];
extern struct CWMP_NODE tWANCONDEVENTITYObjectEth[];
extern struct CWMP_LINKNODE tWANCONDEVObjectEth[];
extern struct CWMP_LEAF tCONSERENTITYLeaf[];
extern struct CWMP_LINKNODE tCONSERVICEObject[];
extern struct CWMP_LEAF tWANDSLCNTMNGLeaf[];
extern struct CWMP_NODE tWANDSLCNTMNGObject[];

int getPORMAPTENTITY(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setPORMAPTENTITY(char *name, struct CWMP_LEAF *entity, int type, void *data);
int objWANPORTMAPPING(char *name, struct CWMP_LEAF *entity, int type, void *data);

/*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
int getWANIPConDHCPClientENTITY(char *name, struct CWMP_LEAF *entity, int *type, void **data);

int getDHCPClientOptENTITY(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setDHCPClientOptENTITY(char *name, struct CWMP_LEAF *entity, int type, void *data);
int objDHCPClientSentOpt(char *name, struct CWMP_LEAF *e, int type, void *data);
int objDHCPClientReqOpt(char *name, struct CWMP_LEAF *e, int type, void *data);
#endif
/*ping_zhang:20080919 END*/

int getWANCONSTATS(char *name, struct CWMP_LEAF *entity, int *type, void **data);

int getWANPPPCONENTITY(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setWANPPPCONENTITY(char *name, struct CWMP_LEAF *entity, int type, void *data);
int objWANPPPConn(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getWANIPCONENTITY(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setWANIPCONENTITY(char *name, struct CWMP_LEAF *entity, int type, void *data);
int objWANIPConn(char *name, struct CWMP_LEAF *entity, int type, void *data);

#ifdef CONFIG_DEV_xDSL
int getDSLLNKCONF(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setDSLLNKCONF(char *name, struct CWMP_LEAF *entity, int type, void *data);
#ifdef CONFIG_PTMWAN
int getPTMLNKCONF(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setPTMLNKCONF(char *name, struct CWMP_LEAF *entity, int type, void *data);
int getPTMLNKSTATS(char *name, struct CWMP_LEAF *entity, int *type, void **data);
#endif /*CONFIG_PTMWAN*/
#endif
#ifdef CONFIG_ETHWAN
int getWANETHLINKCONF(char *name, struct CWMP_LEAF *entity, int *type, void **data);
#endif

int getWANCONDEVENTITY(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int objConDev(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getCONSERENTITY(char *name, struct CWMP_LEAF *entity, int *type, void **data);

int getWANDSLCNTMNG(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int objConService(char *name, struct CWMP_LEAF *entity, int type, void *data);

/*utilities*/
int getDefaultRoute( char *name );
int setDefaultRoute( char *name );
int transfer2IfName( char *name, char *ifname );
unsigned int transfer2IfIndex( char *name );
int transfer2PathName( unsigned int ifindex, char *name );
int transfer2PathNamefromItf( char *ifname, char *pathname );

/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
extern int gStartReset;
void cwmpStartReset();
#endif
/*ping_zhang:20081217 END*/
#ifdef __cplusplus
}
#endif

#endif /*_PRMT_WANCONDEVICE_H_*/
