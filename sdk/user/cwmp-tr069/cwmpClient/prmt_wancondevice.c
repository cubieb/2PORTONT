#include <stdint.h>
#include <signal.h>
#include <linux/atm.h>
#include <linux/atmdev.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <sys/sysinfo.h>
#include <sys/ioctl.h>
#include "cwmpc_utility.h"

#include "prmt_wancondevice.h"
#include "prmt_wanatmf5loopback.h"
#if defined(_PRMT_X_CT_COM_DDNS_) && defined(CONFIG_USER_DDNS)
#include "prmt_ddns.h"
#endif
#ifdef WLAN_SUPPORT
#include "prmt_landevice_wlan.h"
#endif
#ifdef _PRMT_X_CT_EXT_ENABLE_
#include "prmt_ctcom_wanext.h"
#endif
#ifdef CONFIG_BHS
#include "prmt_telefonica_ipv6.h"
#endif

#include <rtk/subr_dhcpv6.h>

unsigned int WANDEVNUM_ATM = 1;
unsigned int WANDEVNUM_PTM = 2;
unsigned int WANDEVNUM_ETH = 3;

//#ifdef _USE_RSDK_WRAPPER_
unsigned int getSentDHCPOptInstNum( char *name );
unsigned int getReqDHCPOptInstNum( char *name );
//#endif //_USE_RSDK_WRAPPER_

int update_WANDEVNUM(void)
{
	int atm_status = -1, ptm_status = -1, nas_status = -1, ret = 0;
	unsigned int new_WANDEVNUM_ATM, new_WANDEVNUM_PTM, new_WANDEVNUM_ETH;

#ifdef CONFIG_DEV_xDSL
	if (WAN_MODE & MODE_ATM) {
		atm_status = get_net_link_status("atm0");
	}
#ifdef CONFIG_PTMWAN
	if (WAN_MODE & MODE_PTM) {
		ptm_status = get_net_link_status("ptm0");
	}
#endif
#endif
#ifdef CONFIG_ETHWAN
	if (WAN_MODE & MODE_Ethernet)
		nas_status = get_net_link_status("nas0");
#endif

	if (atm_status == 1) {
		new_WANDEVNUM_ATM = 1;
		if (ptm_status == 1) {
			new_WANDEVNUM_PTM = 2;
			new_WANDEVNUM_ETH = 3;
		} else {
			if (nas_status == 1) {
				new_WANDEVNUM_PTM = 3;
				new_WANDEVNUM_ETH = 2;
			} else {
				new_WANDEVNUM_PTM = 2;
				new_WANDEVNUM_ETH = 3;
			}
		}
	} else {
		if (ptm_status == 1) {
			new_WANDEVNUM_PTM = 1;
			if (nas_status == 1) {
				new_WANDEVNUM_ATM = 3;
				new_WANDEVNUM_ETH = 2;
			} else {
				new_WANDEVNUM_ATM = 2;
				new_WANDEVNUM_ETH = 3;
			}
		} else {
			if (nas_status == 1) {
				new_WANDEVNUM_ATM = 2;
				new_WANDEVNUM_PTM = 3;
				new_WANDEVNUM_ETH = 1;
			} else {
				new_WANDEVNUM_ATM = 1;
				new_WANDEVNUM_PTM = 2;
				new_WANDEVNUM_ETH = 3;
			}
		}
	}

	if (WANDEVNUM_ATM != new_WANDEVNUM_ATM
			|| WANDEVNUM_PTM != new_WANDEVNUM_PTM
			|| WANDEVNUM_ETH != new_WANDEVNUM_ETH)
		ret = 1;

	WANDEVNUM_ATM = new_WANDEVNUM_ATM;
	WANDEVNUM_PTM = new_WANDEVNUM_PTM;
	WANDEVNUM_ETH = new_WANDEVNUM_ETH;

	return ret;
}

#ifdef E8B_NEW_DIAGNOSE
void writePVCFile(int vpi, int vci, char *action, char *servertype, char *wantype)
{
	FILE *fp;
	char buf[32];
	int fileid;
	pvc_status_entry entry;

	memset(&entry, 0, sizeof(entry));
	entry.vpi = vpi;
	entry.vci = vci;
	strncpy(entry.action, action, sizeof(entry.action));
	strncpy(entry.servertype, servertype, sizeof(entry.servertype));
	strncpy(entry.wantype, wantype, sizeof(entry.wantype));

	if (!strcmp(action, "add")) {
		cleardelpvcfile();
	}
	fileid = findpvcfileid(entry);
	sprintf(buf, "%s%d", PVC_FILE, fileid);
	fp = fopen(buf, "w");
	if (fp) {
		fwrite(&entry, sizeof(pvc_status_entry), 1, fp);
		fclose(fp);

		fp = fopen(NEW_SETTING, "r+");
		if (fp) {
			fgets(buf, sizeof(buf), fp);
			if (strcmp(buf, PVC_FILE)) {
				clearpvcfile();
			} else {
				rewind(fp);
				ftruncate(fileno(fp), 0);
				fprintf(fp, "%s", PVC_FILE);
			}
			fclose(fp);
		}

		fp = fopen(REMOTE_SETSAVE_FILE, "r");
		if (fp) {
			fclose(fp);
			unlink(REMOTE_SETSAVE_FILE);
#ifdef CONFIG_USER_FLATFSD_XXX
			va_cmd("/bin/flatfsd", 1, 1, "-s");
#endif
		}
	}
}
#endif

extern void notify_set_wan_changed(void);
extern int getInterfaceStat(
	char *ifname,
	unsigned long *bs,
	unsigned long *br,
	unsigned long *ps,
	unsigned long *pr );
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
extern int getInterfaceStat1(
	char *ifname,
	unsigned long *es, unsigned long *er,
	unsigned long *ups, unsigned long *upr,
	unsigned long *dps, unsigned long *dpr,
	unsigned long *mps, unsigned long *mpr,
	unsigned long *bps, unsigned long *bpr,
	unsigned long *uppr);
#endif
/*ping_zhang:20081217 END*/
extern int getChainID( struct CWMP_LINKNODE *ctable, int num );

//int getObjectID( struct sCWMP_ENTITY *ctable, int chainid );
unsigned int getNewIfIndex(int cmode, int my_condev_instnum, int media_type, int chainidx);
unsigned int getWANDevInstNum( char *name );
unsigned int getWANConDevInstNum( char *name );
unsigned int getWANPPPConInstNum( char *name );
unsigned int getWANIPConInstNum( char *name );
unsigned int getWANPortMapInstNum( char *name );
unsigned int getInstNum( char *name, char *objname );

/*****port mapping api****/
#ifdef VIRTUAL_SERVER_SUPPORT
unsigned int getPortMappingMaxInstNum( unsigned int ifindex );
int getPortMappingCount( unsigned int ifindex );
int getPortMappingByID( unsigned int ifindex, int id, MIB_VIRTUAL_SVR_T *c, unsigned int *chainID );
int getPortMappingByInstNum( unsigned int ifindex, unsigned int instnum, MIB_VIRTUAL_SVR_T *c, unsigned int *chainID );
#elif defined PORT_FORWARD_GENERAL
unsigned int getPortMappingMaxInstNum( unsigned int ifindex );
int getPortMappingCount( unsigned int ifindex );
int getPortMappingByID( unsigned int ifindex, int id, MIB_CE_PORT_FW_T *c, unsigned int *chainID );
int getPortMappingByInstNum( unsigned int ifindex, unsigned int instnum, MIB_CE_PORT_FW_T *c, unsigned int *chainID );
#endif
/*****endi port mapping api****/

int getATMCellCnt( unsigned char vpi, unsigned short vci, unsigned int *txcnt, unsigned int *rxcnt );

int getRIPInfo( unsigned int ifIndex, unsigned char *ripmode );
int updateRIPInfo( unsigned int ifIndex, unsigned char newripmode );

#ifdef CONFIG_PPP
/*ppp utilities*/
static int getPPPUptime( char *pppname, int ppptype, unsigned int *uptime );
#endif

int getATMVCEntry(char *, MIB_CE_ATM_VC_T *, unsigned int *);
int resetATMVCConnection( MIB_CE_ATM_VC_T *p );

//char gSharedWanName[256+1]="";


struct CWMP_OP tWANCONSTATSLeafOP = { getWANCONSTATS, NULL };
struct CWMP_PRMT tWANCONSTATSLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"EthernetBytesSent",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANCONSTATSLeafOP},
{"EthernetBytesReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANCONSTATSLeafOP},
{"EthernetPacketsSent",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANCONSTATSLeafOP},
{"EthernetPacketsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANCONSTATSLeafOP},
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{"EthernetErrorsSent",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANCONSTATSLeafOP},
{"EthernetErrorsReceived",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANCONSTATSLeafOP},
{"EthernetUnicastPacketsSent",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANCONSTATSLeafOP},
{"EthernetUnicastPacketsReceived",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANCONSTATSLeafOP},
{"EthernetDiscardPacketsSent",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANCONSTATSLeafOP},
{"EthernetDiscardPacketsReceived",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANCONSTATSLeafOP},
{"EthernetMulticastPacketsSent",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANCONSTATSLeafOP},
{"EthernetMulticastPacketsReceived",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANCONSTATSLeafOP},
{"EthernetBroadcastPacketsSent",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANCONSTATSLeafOP},
{"EthernetBroadcastPacketsReceived",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANCONSTATSLeafOP},
{"EthernetUnknownProtoPacketsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANCONSTATSLeafOP}
#endif
/*ping_zhang:20081217 END*/
};
enum eWANCONSTATSLeaf
{
	eEthernetBytesSent,
	eEthernetBytesReceived,
	eEthernetPacketsSent,
	eEthernetPacketsReceived,
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	eEthernetErrorsSent,
	eEthernetErrorsReceived,
	eEthernetUnicastPacketsSent,
	eEthernetUnicastPacketsReceived,
	eEthernetDiscardPacketsSent,
	eEthernetDiscardPacketsReceived,
	eEthernetMulticastPacketsSent,
	eEthernetMulticastPacketsReceived,
	eEthernetBroadcastPacketsSent,
	eEthernetBroadcastPacketsReceived,
	eEthernetUnknownProtoPacketsReceived
#endif
/*ping_zhang:20081217 END*/
};
struct CWMP_LEAF tWANCONSTATSLeaf[] =
{
{ &tWANCONSTATSLeafInfo[eEthernetBytesSent] },
{ &tWANCONSTATSLeafInfo[eEthernetBytesReceived] },
{ &tWANCONSTATSLeafInfo[eEthernetPacketsSent] },
{ &tWANCONSTATSLeafInfo[eEthernetPacketsReceived] },
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{ &tWANCONSTATSLeafInfo[eEthernetErrorsSent] },
{ &tWANCONSTATSLeafInfo[eEthernetErrorsReceived] },
{ &tWANCONSTATSLeafInfo[eEthernetUnicastPacketsSent] },
{ &tWANCONSTATSLeafInfo[eEthernetUnicastPacketsReceived] },
{ &tWANCONSTATSLeafInfo[eEthernetDiscardPacketsSent] },
{ &tWANCONSTATSLeafInfo[eEthernetDiscardPacketsReceived] },
{ &tWANCONSTATSLeafInfo[eEthernetMulticastPacketsSent] },
{ &tWANCONSTATSLeafInfo[eEthernetMulticastPacketsReceived] },
{ &tWANCONSTATSLeafInfo[eEthernetBroadcastPacketsSent] },
{ &tWANCONSTATSLeafInfo[eEthernetBroadcastPacketsReceived] },
{ &tWANCONSTATSLeafInfo[eEthernetUnknownProtoPacketsReceived] },
#endif
/*ping_zhang:20081217 END*/
{ NULL }
};

/*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
/*InternetGatewayDevice.WANDevice.{i}.WANConnectionDevice.{i}.WANIPConnection.{i}.DHCPClient.SentDHCPOption.{i}.*/
struct CWMP_OP tDHCPClientSentENTITYLeafOP = { getDHCPClientOptENTITY, setDHCPClientOptENTITY };
struct CWMP_PRMT tDHCPCLientSentENTITYLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tDHCPClientSentENTITYLeafOP},
{"Tag",			eCWMP_tUINT,		CWMP_WRITE|CWMP_READ,	&tDHCPClientSentENTITYLeafOP},
{"Value",			eCWMP_tBASE64,	CWMP_WRITE|CWMP_READ,	&tDHCPClientSentENTITYLeafOP}
};
enum eDHCPCLientSentENTITYLeaf
{
	eSentEnable,
	eSentTag,
	eSentValue
};
struct CWMP_LEAF tDHCPCLientSentENTITYLeaf[] =
{
{ &tDHCPCLientSentENTITYLeafInfo[eSentEnable] },
{ &tDHCPCLientSentENTITYLeafInfo[eSentTag] },
{ &tDHCPCLientSentENTITYLeafInfo[eSentValue] },
{ NULL }
};

struct CWMP_PRMT tDHCPClientSentObjectInfo[] =
{
/*(name,			type,		flag,					op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eDHCPClientSentObjectInfo
{
	eDHCPCLientSent0
};
struct CWMP_LINKNODE tDHCPClientSentObject[] =
{
/*info,  				leaf,			next,		sibling,		instnum)*/
{&tDHCPClientSentObjectInfo[eDHCPCLientSent0],	tDHCPCLientSentENTITYLeaf,	NULL,		NULL,			0},
};

/*InternetGatewayDevice.WANDevice.{i}.WANConnectionDevice.{i}.WANIPConnection.{i}.DHCPClient.ReqDHCPOption.{i}.*/
struct CWMP_OP tDHCPClientReqENTITYLeafOP = { getDHCPClientOptENTITY, setDHCPClientOptENTITY };
struct CWMP_PRMT tDHCPCLientReqENTITYLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tDHCPClientReqENTITYLeafOP},
{"Order",			eCWMP_tUINT,		CWMP_WRITE|CWMP_READ,	&tDHCPClientReqENTITYLeafOP},
{"Tag",			eCWMP_tUINT,		CWMP_WRITE|CWMP_READ,	&tDHCPClientReqENTITYLeafOP},
{"Value",			eCWMP_tBASE64,	CWMP_READ,				&tDHCPClientReqENTITYLeafOP}
};
enum eDHCPCLientReqENTITYLeaf
{
	eReqEnable,
	eReqOrder,
	eReqTag,
	eReqValue
};
struct CWMP_LEAF tDHCPCLientReqENTITYLeaf[] =
{
{ &tDHCPCLientReqENTITYLeafInfo[eReqEnable] },
{ &tDHCPCLientReqENTITYLeafInfo[eReqOrder] },
{ &tDHCPCLientReqENTITYLeafInfo[eReqTag] },
{ &tDHCPCLientReqENTITYLeafInfo[eReqValue] },
{ NULL }
};

struct CWMP_PRMT tDHCPClientReqObjectInfo[] =
{
/*(name,			type,		flag,					op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eDHCPClientReqObjectInfo
{
	eDHCPCLientReq0
};
struct CWMP_LINKNODE tDHCPClientReqObject[] =
{
/*info,  				leaf,			next,		sibling,		instnum)*/
{&tDHCPClientReqObjectInfo[eDHCPCLientReq0],	tDHCPCLientReqENTITYLeaf,	NULL,		NULL,			0},
};
#endif
/*ping_zhang:20080919 END*/

#if defined(VIRTUAL_SERVER_SUPPORT) || defined(PORT_FORWARD_GENERAL)
struct CWMP_OP tPORTMAPENTITYLeafOP = { getPORMAPTENTITY, setPORMAPTENTITY };
struct CWMP_PRMT tPORTMAPENTITYLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"PortMappingEnabled",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tPORTMAPENTITYLeafOP},
{"PortMappingLeaseDuration",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ|CWMP_DENY_ACT,&tPORTMAPENTITYLeafOP},
{"RemoteHost",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tPORTMAPENTITYLeafOP},
{"ExternalPort",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tPORTMAPENTITYLeafOP},
{"InternalPort",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tPORTMAPENTITYLeafOP},
{"PortMappingProtocol",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tPORTMAPENTITYLeafOP},
{"InternalClient",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tPORTMAPENTITYLeafOP},
{"PortMappingDescription",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tPORTMAPENTITYLeafOP}
};
enum ePORTMAPENTITYLeaf
{
	ePortMappingEnabled,
	ePortMappingLeaseDuration,
	eRemoteHost,
	eExternalPort,
	eInternalPort,
	ePortMappingProtocol,
	eInternalClient,
	ePortMappingDescription
};
struct CWMP_LEAF tPORTMAPENTITYLeaf[] =
{
{ &tPORTMAPENTITYLeafInfo[ePortMappingEnabled] },
{ &tPORTMAPENTITYLeafInfo[ePortMappingLeaseDuration] },
{ &tPORTMAPENTITYLeafInfo[eRemoteHost] },
{ &tPORTMAPENTITYLeafInfo[eExternalPort] },
{ &tPORTMAPENTITYLeafInfo[eInternalPort] },
{ &tPORTMAPENTITYLeafInfo[ePortMappingProtocol] },
{ &tPORTMAPENTITYLeafInfo[eInternalClient] },
{ &tPORTMAPENTITYLeafInfo[ePortMappingDescription] },
{ NULL }
};


struct CWMP_PRMT tWANPORTMAPObjectInfo[] =
{
/*(name,			type,		flag,					op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eWANPORTMAPObject
{
	eWANPORTMAP0
};
struct CWMP_LINKNODE tWANPORTMAPObject[] =
{
/*info,  				leaf,			next,		sibling,		instnum)*/
{&tWANPORTMAPObjectInfo[eWANPORTMAP0],	tPORTMAPENTITYLeaf,	NULL,		NULL,			0},
};
#endif


struct CWMP_OP tWANPPPCONENTITYLeafOP = { getWANPPPCONENTITY, setWANPPPCONENTITY };
struct CWMP_PRMT tWANPPPCONENTITYLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP},
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{"Reset",				eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP},
#endif
/*ping_zhang:20081217 END*/
{"ConnectionStatus",		eCWMP_tSTRING,	CWMP_READ,		&tWANPPPCONENTITYLeafOP},
{"PossibleConnectionTypes",	eCWMP_tSTRING,	CWMP_READ,		&tWANPPPCONENTITYLeafOP},
{"ConnectionType",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP},
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{"PPPoESessionID",		eCWMP_tUINT,	CWMP_READ,		&tWANPPPCONENTITYLeafOP},
{"DefaultGateway",		eCWMP_tSTRING,	CWMP_READ,		&tWANPPPCONENTITYLeafOP},
#endif
/*ping_zhang:20081217 END*/
{"Name",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP},
{"Uptime",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANPPPCONENTITYLeafOP},
{"LastConnectionError",		eCWMP_tSTRING,	CWMP_READ,		&tWANPPPCONENTITYLeafOP},
{"AutoDisconnectTime",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP},
{"IdleDisconnectTime",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP},
{"WarnDisconnectDelay",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP},
{"RSIPAvailable",		eCWMP_tBOOLEAN,	CWMP_READ,		&tWANPPPCONENTITYLeafOP},
{"NATEnabled",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP},
{"Username",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP},
{"Password",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP},
{"PPPEncryptionProtocol",	eCWMP_tSTRING,	CWMP_READ,		&tWANPPPCONENTITYLeafOP},
{"PPPCompressionProtocol",	eCWMP_tSTRING,	CWMP_READ,		&tWANPPPCONENTITYLeafOP},
{"PPPAuthenticationProtocol",	eCWMP_tSTRING,	CWMP_READ,		&tWANPPPCONENTITYLeafOP},
{"ExternalIPAddress",		eCWMP_tSTRING,	CWMP_READ,		&tWANPPPCONENTITYLeafOP},
{"RemoteIPAddress",		eCWMP_tSTRING,	CWMP_READ,		&tWANPPPCONENTITYLeafOP},
{"MaxMRUSize",			eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP},
{"CurrentMRUSize",		eCWMP_tUINT,	CWMP_READ,		&tWANPPPCONENTITYLeafOP},
{"DNSEnabled",			eCWMP_tBOOLEAN,	CWMP_READ,		&tWANPPPCONENTITYLeafOP},
{"DNSOverrideAllowed",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP},
{"DNSServers",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP},
{"MACAddress",			eCWMP_tSTRING,	CWMP_READ,		&tWANPPPCONENTITYLeafOP},
/*MACAddressOverride*/
{"TransportType",		eCWMP_tSTRING,	CWMP_READ,		&tWANPPPCONENTITYLeafOP},
{"PPPoEACName",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP},
{"PPPoEServiceName",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP},
{"ConnectionTrigger",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP},
{"RouteProtocolRx",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP},
#ifdef _PRMT_X_RTK_
{"X_RTK_IgmpProxy",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP},
{"X_RTK_ServiceType", 		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP},
#endif
{"PPPLCPEcho",			eCWMP_tUINT,	CWMP_READ,		&tWANPPPCONENTITYLeafOP},
{"PPPLCPEchoRetry",		eCWMP_tUINT,	CWMP_READ,		&tWANPPPCONENTITYLeafOP},
{"PortMappingNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,		&tWANPPPCONENTITYLeafOP},
};
enum eWANPPPCONENTITYLeaf
{
	ePPP_Enable,
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	ePPP_Reset,
#endif
/*ping_zhang:20081217 END*/
	ePPP_ConnectionStatus,
	ePPP_PossibleConnectionTypes,
	ePPP_ConnectionType,
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	ePPP_PPPoESessionID,
	ePPP_DefaultGateway,
#endif
/*ping_zhang:20081217 END*/
	ePPP_Name,
	ePPP_Uptime,
	ePPP_LastConnectionError,
	ePPP_AutoDisconnectTime,
	ePPP_IdleDisconnectTime,
	ePPP_WarnDisconnectDelay,
	ePPP_RSIPAvailable,
	ePPP_NATEnabled,
	ePPP_Username,
	ePPP_Password,
	ePPP_PPPEncryptionProtocol,
	ePPP_PPPCompressionProtocol,
	ePPP_PPPAuthenticationProtocol,
	ePPP_ExternalIPAddress,
	ePPP_RemoteIPAddress,
	ePPP_MaxMRUSize,
	ePPP_CurrentMRUSize,
	ePPP_DNSEnabled,
	ePPP_DNSOverrideAllowed,
	ePPP_DNSServers,
	ePPP_MACAddress,
	/*MACAddressOverride*/
	ePPP_TransportType,
	ePPP_PPPoEACName,
	ePPP_PPPoEServiceName,
	ePPP_ConnectionTrigger,
	ePPP_RouteProtocolRx,
#ifdef _PRMT_X_RTK_
	ePPP_X_RTK_IgmpProxy,
	ePPP_X_RTK_ServiceType,
#endif
	ePPP_PPPLCPEcho,
	ePPP_PPPLCPEchoRetry,
	ePPP_PortMappingNumberOfEntries,
};
struct CWMP_LEAF tWANPPPCONENTITYLeaf[] =
{
{ &tWANPPPCONENTITYLeafInfo[ePPP_Enable] },
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{ &tWANPPPCONENTITYLeafInfo[ePPP_Reset] },
#endif
/*ping_zhang:20081217 END*/
{ &tWANPPPCONENTITYLeafInfo[ePPP_ConnectionStatus] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_PossibleConnectionTypes] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_ConnectionType] },
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{ &tWANPPPCONENTITYLeafInfo[ePPP_PPPoESessionID] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_DefaultGateway] },
#endif
/*ping_zhang:20081217 END*/
{ &tWANPPPCONENTITYLeafInfo[ePPP_Name] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_Uptime] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_LastConnectionError] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_AutoDisconnectTime] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_IdleDisconnectTime] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_WarnDisconnectDelay] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_RSIPAvailable] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_NATEnabled] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_Username] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_Password] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_PPPEncryptionProtocol] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_PPPCompressionProtocol] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_PPPAuthenticationProtocol] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_ExternalIPAddress] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_RemoteIPAddress] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_MaxMRUSize] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_CurrentMRUSize] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_DNSEnabled] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_DNSOverrideAllowed] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_DNSServers] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_MACAddress] },
/*MACAddressOverride*/
{ &tWANPPPCONENTITYLeafInfo[ePPP_TransportType] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_PPPoEACName] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_PPPoEServiceName] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_ConnectionTrigger] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_RouteProtocolRx] },
#ifdef _PRMT_X_RTK_
{ &tWANPPPCONENTITYLeafInfo[ePPP_X_RTK_IgmpProxy] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_X_RTK_ServiceType] },
#endif
{ &tWANPPPCONENTITYLeafInfo[ePPP_PPPLCPEcho] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_PPPLCPEchoRetry] },
#ifdef _PRMT_X_CT_COM_PPPOE_PROXY_
{ &tCTWANPPPCONENTITYLeafInfo[ePPP_X_CTCOM_ProxyEnable] },
{ &tCTWANPPPCONENTITYLeafInfo[ePPP_X_CTCOM_MAXUser] },
#endif //_PRMT_X_CT_COM_PPPOE_PROXY_
#ifdef _PRMT_X_CT_COM_WANEXT_
{ &tCTWANPPPCONENTITYLeafInfo[ePPP_X_CTCOM_LanInterface] },
{ &tCTWANPPPCONENTITYLeafInfo[ePPP_X_CTCOM_ServiceList] },
{ &tCTWANPPPCONENTITYLeafInfo[ePPP_X_CTCOM_LanInterface_DHCPEnable] },
{ &tCTWANPPPCONENTITYLeafInfo[ePPP_X_CTCOM_IPForwardList] },
#ifdef CONFIG_MCAST_VLAN
{ &tCTWANPPPCONENTITYLeafInfo[ePPP_X_CTCOM_MulticastVlan] },
#endif
#endif //_PRMT_X_CT_COM_WANEXT_
#ifdef _PRMT_X_CT_COM_IPv6_
{ &tCTWANPPPCONENTITYLeafInfo[ePPP_X_CTCOM_IPMode] },
{ &tCTWANPPPCONENTITYLeafInfo[ePPP_X_CTCOM_IPv6ConnectionStatus] },
{ &tCTWANPPPCONENTITYLeafInfo[ePPP_X_CTCOM_IPv6IPAddress] },
{ &tCTWANPPPCONENTITYLeafInfo[ePPP_X_CTCOM_IPv6IPAddressAlias] },
{ &tCTWANPPPCONENTITYLeafInfo[ePPP_X_CTCOM_IPv6IPAddressOrigin] },
{ &tCTWANPPPCONENTITYLeafInfo[ePPP_X_CTCOM_IPv6DNSServers] },
{ &tCTWANPPPCONENTITYLeafInfo[ePPP_X_CTCOM_IPv6PrefixDelegationEnabled] },
{ &tCTWANPPPCONENTITYLeafInfo[ePPP_X_CTCOM_IPv6PrefixAlias] },
{ &tCTWANPPPCONENTITYLeafInfo[ePPP_X_CTCOM_IPv6PrefixOrigin] },
{ &tCTWANPPPCONENTITYLeafInfo[ePPP_X_CTCOM_IPv6Prefix] },
{ &tCTWANPPPCONENTITYLeafInfo[ePPP_X_CTCOM_IPv6PrefixPltime] },
{ &tCTWANPPPCONENTITYLeafInfo[ePPP_X_CTCOM_IPv6PrefixVltime] },
{ &tCTWANPPPCONENTITYLeafInfo[ePPP_X_CTCOM_DefaultIPv6Gateway] },
{ &tCTWANPPPCONENTITYLeafInfo[ePPP_X_CTCOM_IPv6DomainName] },
{ &tCTWANPPPCONENTITYLeafInfo[ePPP_X_CTCOM_Dslite_Enable] },
{ &tCTWANPPPCONENTITYLeafInfo[ePPP_X_CTCOM_AftrMode] },
{ &tCTWANPPPCONENTITYLeafInfo[ePPP_X_CTCOM_Aftr] },
#endif	//_PRMT_X_CT_COM_IPv6_
{ &tWANPPPCONENTITYLeafInfo[ePPP_PortMappingNumberOfEntries] },
#ifdef CONFIG_BHS
{ &tBHSWANPPPCONENTITYLeafInfo[ePPP_X_TELEFONICA_ES_IPv6Enabled] },
{ &tBHSWANPPPCONENTITYLeafInfo[ePPP_X_TELEFONICA_ES_IPv4Enabled] },
{ &tBHSWANPPPCONENTITYLeafInfo[ePPP_X_TELEFONICA_ES_UnnumberedModel] },
{ &tBHSWANPPPCONENTITYLeafInfo[ePPP_X_TELEFONICA_ES_IPv6ConnStatus] },
{ &tBHSWANPPPCONENTITYLeafInfo[ePPP_X_TELEFONICA_ES_IPv6PppUp] },
{ &tBHSWANPPPCONENTITYLeafInfo[ePPP_X_TELEFONICA_ES_IPv6AddressingType] },
{ &tBHSWANPPPCONENTITYLeafInfo[ePPP_X_TELEFONICA_ES_DHCP6cForAddress] },
{ &tBHSWANPPPCONENTITYLeafInfo[ePPP_X_TELEFONICA_ES_ExternalIPv6Address] },
{ &tBHSWANPPPCONENTITYLeafInfo[ePPP_X_TELEFONICA_ES_DefaultIPv6Gateway] },
{ &tBHSWANPPPCONENTITYLeafInfo[ePPP_X_TELEFONICA_ES_IPv6DNSServers] },
{ &tBHSWANPPPCONENTITYLeafInfo[ePPP_X_TELEFONICA_ES_IPv6PrefixDelegationEnabled] },
{ &tBHSWANPPPCONENTITYLeafInfo[ePPP_X_TELEFONICA_ES_IPv6SitePrefix] },
{ &tBHSWANPPPCONENTITYLeafInfo[ePPP_X_TELEFONICA_ES_IPv6SitePrefixPltime] },
{ &tBHSWANPPPCONENTITYLeafInfo[ePPP_X_TELEFONICA_ES_IPv6SitePrefixVLtime] },
{ &tBHSWANPPPCONENTITYLeafInfo[ePPP_X_TELEFONICA_ES_ExternalIPv6AddressPrefixLength] },
#endif
{ NULL }
};

#if defined(VIRTUAL_SERVER_SUPPORT) || defined(PORT_FORWARD_GENERAL)
struct CWMP_OP tWAN_PortMapping_OP = { NULL, objWANPORTMAPPING };
#endif
#if defined(_PRMT_X_CT_COM_DDNS_) && defined(CONFIG_USER_DDNS)
struct CWMP_OP tWAN_DDNSConf_OP = { NULL, objDDNS };
#endif
struct CWMP_PRMT tWANPPPCONENTITYObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
#if defined(VIRTUAL_SERVER_SUPPORT) || defined(PORT_FORWARD_GENERAL)
{"PortMapping",			eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,	&tWAN_PortMapping_OP},
#endif
{"Stats",			eCWMP_tOBJECT,	CWMP_READ,		NULL},
#if defined(_PRMT_X_CT_COM_DDNS_) && defined(CONFIG_USER_DDNS)
{"X_CT-COM_DDNSConfiguration",	eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,	&tWAN_DDNSConf_OP},
#endif
};
enum eWANPPPCONENTITYObject
{
	ePPP_PortMapping,
	ePPP_Stats,
#if defined(_PRMT_X_CT_COM_DDNS_) && defined(CONFIG_USER_DDNS)
	ePPP_X_CTCOM_DDNSConfiguration
#endif
};
struct CWMP_NODE tWANPPPCONENTITYObject[] =
{
/*info,  							leaf,			node)*/
{&tWANPPPCONENTITYObjectInfo[ePPP_PortMapping],			NULL,			NULL},
{&tWANPPPCONENTITYObjectInfo[ePPP_Stats],			tWANCONSTATSLeaf,	NULL},
#if defined(_PRMT_X_CT_COM_DDNS_) && defined(CONFIG_USER_DDNS)
{&tWANPPPCONENTITYObjectInfo[ePPP_X_CTCOM_DDNSConfiguration],	NULL,			NULL},
#endif
{NULL,								NULL,			NULL}
};


struct CWMP_PRMT tWANPPPCONObjectInfo[] =
{
/*(name,			type,		flag,					op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eWANPPPCONObject
{
	WANPPPCON0
};
struct CWMP_LINKNODE tWANPPPCONObject[] =
{
/*info,  				leaf,			next,				sibling,		instnum)*/
{&tWANPPPCONObjectInfo[WANPPPCON0],	tWANPPPCONENTITYLeaf,	tWANPPPCONENTITYObject,		NULL,			0}
};


/*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
struct CWMP_OP tWANIPConDHCPClientLeafOP = { getWANIPConDHCPClientENTITY, NULL };
struct CWMP_PRMT tWANIPConDHCPClientLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"SentDHCPOptionNumberOfEntries",		eCWMP_tUINT,		CWMP_READ,	&tWANIPConDHCPClientLeafOP},
{"ReqDHCPOptionNumberOfEntries",		eCWMP_tUINT,		CWMP_READ,	&tWANIPConDHCPClientLeafOP}
};
enum eWANIPConDHCPClientLeaf
{
	eSentDHCPOptionNumberOfEntries,
	eReqDHCPOptionNumberOfEntries
};
struct CWMP_LEAF tWANIPConDHCPClientLeaf[] =
{
{ &tWANIPConDHCPClientLeafInfo[eSentDHCPOptionNumberOfEntries] },
{ &tWANIPConDHCPClientLeafInfo[eReqDHCPOptionNumberOfEntries] },
{ NULL }
};

struct CWMP_OP tDHCPClientSentOpt_OP = { NULL, objDHCPClientSentOpt};
struct CWMP_OP tDHCPClientReqOpt_OP = { NULL, objDHCPClientReqOpt};
struct CWMP_PRMT tWANIPConDHCPClientObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"SentDHCPOption",		eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,		&tDHCPClientSentOpt_OP},
{"ReqDHCPOption",			eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,		&tDHCPClientReqOpt_OP},
};
enum eWANIPConDHCPClientObjectInfo
{
	eSentDHCPOption,
	eReqDHCPOption
};
struct CWMP_NODE tWANIPConDHCPClientObject[] =
{
/*info,  							leaf,			node)*/
{&tWANIPConDHCPClientObjectInfo[eSentDHCPOption],	NULL,	NULL},
{&tWANIPConDHCPClientObjectInfo[eReqDHCPOption],	NULL,	NULL},
{NULL,								NULL,			NULL}
};
#endif
/*ping_zhang:20080919 END*/

struct CWMP_OP tWANIPCONENTITYLeafOP = { getWANIPCONENTITY, setWANIPCONENTITY };
struct CWMP_PRMT tWANIPCONENTITYLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",				eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWANIPCONENTITYLeafOP},
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{"Reset",				eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWANIPCONENTITYLeafOP},
#endif
/*ping_zhang:20081217 END*/
{"ConnectionStatus",			eCWMP_tSTRING,	CWMP_READ,		&tWANIPCONENTITYLeafOP},
{"PossibleConnectionTypes",		eCWMP_tSTRING,	CWMP_READ,		&tWANIPCONENTITYLeafOP},
{"ConnectionType",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANIPCONENTITYLeafOP},
{"Name",				eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANIPCONENTITYLeafOP},
{"Uptime",				eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANIPCONENTITYLeafOP},
{"LastConnectionError",			eCWMP_tSTRING,	CWMP_READ,		&tWANIPCONENTITYLeafOP},
/*AutoDisconnectTime*/
/*IdleDisconnectTime*/
/*WarnDisconnectDelay*/
{"RSIPAvailable",			eCWMP_tBOOLEAN,	CWMP_READ,		&tWANIPCONENTITYLeafOP},
{"NATEnabled",				eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWANIPCONENTITYLeafOP},
{"AddressingType",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANIPCONENTITYLeafOP},
{"ExternalIPAddress",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANIPCONENTITYLeafOP},
{"SubnetMask",				eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANIPCONENTITYLeafOP},
{"DefaultGateway",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANIPCONENTITYLeafOP},
{"DNSEnabled",				eCWMP_tBOOLEAN,	CWMP_READ,		&tWANIPCONENTITYLeafOP},
{"DNSOverrideAllowed",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWANIPCONENTITYLeafOP},
{"DNSServers",				eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANIPCONENTITYLeafOP},
{"MaxMTUSize",				eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tWANIPCONENTITYLeafOP},
{"MACAddress",				eCWMP_tSTRING,	CWMP_READ,		&tWANIPCONENTITYLeafOP},
/*MACAddressOverride*/
{"ConnectionTrigger",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANIPCONENTITYLeafOP},
{"RouteProtocolRx",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANIPCONENTITYLeafOP},
#ifdef _PRMT_X_RTK_
{"X_RTK_IgmpProxy",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWANIPCONENTITYLeafOP},
{"X_RTK_ServiceType",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tWANIPCONENTITYLeafOP},
#endif
{"PortMappingNumberOfEntries",		eCWMP_tUINT,	CWMP_READ,		&tWANIPCONENTITYLeafOP},
};
enum eWANIPCONENTITYLeaf
{
	eIP_Enable,
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	eIP_Reset,
#endif
/*ping_zhang:20081217 END*/
	eIP_ConnectionStatus,
	eIP_PossibleConnectionTypes,
	eIP_ConnectionType,
	eIP_Name,
	eIP_Uptime,
	eIP_LastConnectionError,
	/*AutoDisconnectTime*/
	/*IdleDisconnectTime*/
	/*WarnDisconnectDelay*/
	eIP_RSIPAvailable,
	eIP_NATEnabled,
	eIP_AddressingType,
	eIP_ExternalIPAddress,
	eIP_SubnetMask,
	eIP_DefaultGateway,
	eIP_DNSEnabled,
	eIP_DNSOverrideAllowed,
	eIP_DNSServers,
	eIP_MaxMTUSize,
	eIP_MACAddress,
	/*MACAddressOverride*/
	eIP_ConnectionTrigger,
	eIP_RouteProtocolRx,
#ifdef _PRMT_X_RTK_
	eIP_X_RTK_IgmpProxy,
	eIP_X_RTK_ServiceType,
#endif
	eIP_PortMappingNumberOfEntries,
};
struct CWMP_LEAF tWANIPCONENTITYLeaf[] =
{
{ &tWANIPCONENTITYLeafInfo[eIP_Enable] },
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{ &tWANIPCONENTITYLeafInfo[eIP_Reset] },
#endif
/*ping_zhang:20081217 END*/
{ &tWANIPCONENTITYLeafInfo[eIP_ConnectionStatus] },
{ &tWANIPCONENTITYLeafInfo[eIP_PossibleConnectionTypes] },
{ &tWANIPCONENTITYLeafInfo[eIP_ConnectionType] },
{ &tWANIPCONENTITYLeafInfo[eIP_Name] },
{ &tWANIPCONENTITYLeafInfo[eIP_Uptime] },
{ &tWANIPCONENTITYLeafInfo[eIP_LastConnectionError] },
/*AutoDisconnectTime*/
/*IdleDisconnectTime*/
/*WarnDisconnectDelay*/
{ &tWANIPCONENTITYLeafInfo[eIP_RSIPAvailable] },
{ &tWANIPCONENTITYLeafInfo[eIP_NATEnabled] },
{ &tWANIPCONENTITYLeafInfo[eIP_AddressingType] },
{ &tWANIPCONENTITYLeafInfo[eIP_ExternalIPAddress] },
{ &tWANIPCONENTITYLeafInfo[eIP_SubnetMask] },
{ &tWANIPCONENTITYLeafInfo[eIP_DefaultGateway] },
{ &tWANIPCONENTITYLeafInfo[eIP_DNSEnabled] },
{ &tWANIPCONENTITYLeafInfo[eIP_DNSOverrideAllowed] },
{ &tWANIPCONENTITYLeafInfo[eIP_DNSServers] },
{ &tWANIPCONENTITYLeafInfo[eIP_MaxMTUSize] },
{ &tWANIPCONENTITYLeafInfo[eIP_MACAddress] },
/*MACAddressOverride*/
{ &tWANIPCONENTITYLeafInfo[eIP_ConnectionTrigger] },
{ &tWANIPCONENTITYLeafInfo[eIP_RouteProtocolRx] },
#ifdef _PRMT_X_RTK_
{ &tWANIPCONENTITYLeafInfo[eIP_X_RTK_IgmpProxy] },
{ &tWANIPCONENTITYLeafInfo[eIP_X_RTK_ServiceType] },
#endif
#ifdef _PRMT_X_CT_COM_WANEXT_
{ &tCTWANIPCONENTITYLeafInfo[eIP_X_CTCOM_LanInterface] },
{ &tCTWANIPCONENTITYLeafInfo[eIP_X_CTCOM_ServiceList] },
{ &tCTWANIPCONENTITYLeafInfo[eIP_X_CTCOM_LanInterface_DHCPEnable] },
{ &tCTWANIPCONENTITYLeafInfo[eIP_X_CTCOM_IPForwardList] },
{ &tCTWANIPCONENTITYLeafInfo[eIP_X_CTCOM_IPMode] },
{ &tCTWANIPCONENTITYLeafInfo[eIP_X_CTCOM_IPv6ConnectionStatus] },
{ &tCTWANIPCONENTITYLeafInfo[eIP_X_CTCOM_LanInterface_DHCPEnable] },
{ &tCTWANIPCONENTITYLeafInfo[eIP_X_CTCOM_IPv6IPAddress] },
{ &tCTWANIPCONENTITYLeafInfo[eIP_X_CTCOM_IPv6IPAddressAlias] },
{ &tCTWANIPCONENTITYLeafInfo[eIP_X_CTCOM_IPv6IPAddressOrigin] },
{ &tCTWANIPCONENTITYLeafInfo[eIP_X_CTCOM_IPv6DNSServers] },
{ &tCTWANIPCONENTITYLeafInfo[eIP_X_CTCOM_IPv6PrefixDelegationEnabled] },
{ &tCTWANIPCONENTITYLeafInfo[eIP_X_CTCOM_IPv6PrefixAlias] },
{ &tCTWANIPCONENTITYLeafInfo[eIP_X_CTCOM_IPv6PrefixOrigin] },
{ &tCTWANIPCONENTITYLeafInfo[eIP_X_CTCOM_IPv6Prefix] },
{ &tCTWANIPCONENTITYLeafInfo[eIP_X_CTCOM_IPv6PrefixPltime] },
{ &tCTWANIPCONENTITYLeafInfo[eIP_X_CTCOM_IPv6PrefixVltime] },
{ &tCTWANIPCONENTITYLeafInfo[eIP_X_CTCOM_DefaultIPv6Gateway] },
{ &tCTWANIPCONENTITYLeafInfo[eIP_X_CTCOM_IPv6DomainName] },
{ &tCTWANIPCONENTITYLeafInfo[eIP_X_CTCOM_Dslite_Enable] },
{ &tCTWANIPCONENTITYLeafInfo[eIP_X_CTCOM_AftrMode] },
{ &tCTWANIPCONENTITYLeafInfo[eIP_X_CTCOM_Aftr] },
#ifdef CONFIG_MCAST_VLAN
{ &tCTWANIPCONENTITYLeafInfo[eIP_X_CTCOM_MulticastVlan] },
#endif
#endif //_PRMT_X_CT_COM_WANEXT_
{ &tWANIPCONENTITYLeafInfo[eIP_PortMappingNumberOfEntries] },
#ifdef CONFIG_BHS
{ &tBHSWANIPCONENTITYLeafInfo[eIP_X_TELEFONICA_ES_IPv6Enabled] },
{ &tBHSWANIPCONENTITYLeafInfo[eIP_X_TELEFONICA_ES_IPv4Enabled] },
{ &tBHSWANIPCONENTITYLeafInfo[eIP_X_TELEFONICA_ES_UnnumberedModel] },
{ &tBHSWANIPCONENTITYLeafInfo[eIP_X_TELEFONICA_ES_IPv6ConnStatus] },
{ &tBHSWANIPCONENTITYLeafInfo[eIP_X_TELEFONICA_ES_IPv6AddressingType] },
{ &tBHSWANIPCONENTITYLeafInfo[eIP_X_TELEFONICA_ES_DHCP6cForAddress] },
{ &tBHSWANIPCONENTITYLeafInfo[eIP_X_TELEFONICA_ES_ExternalIPv6Address] },
{ &tBHSWANIPCONENTITYLeafInfo[eIP_X_TELEFONICA_ES_DefaultIPv6Gateway] },
{ &tBHSWANIPCONENTITYLeafInfo[eIP_X_TELEFONICA_ES_IPv6DNSServers] },
{ &tBHSWANIPCONENTITYLeafInfo[eIP_X_TELEFONICA_ES_IPv6PrefixDelegationEnabled] },
{ &tBHSWANIPCONENTITYLeafInfo[eIP_X_TELEFONICA_ES_IPv6SitePrefix] },
{ &tBHSWANIPCONENTITYLeafInfo[eIP_X_TELEFONICA_ES_IPv6SitePrefixPltime] },
{ &tBHSWANIPCONENTITYLeafInfo[eIP_X_TELEFONICA_ES_IPv6SitePrefixVLtime] },
{ &tBHSWANIPCONENTITYLeafInfo[eIP_X_TELEFONICA_ES_ExternalIPv6AddressPrefixLength] },
#endif
{ NULL }
};

struct CWMP_PRMT tWANIPCONENTITYObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
/*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
{"DHCPClient",			eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif
/*ping_zhang:20080919 END*/
#if defined(VIRTUAL_SERVER_SUPPORT) || defined(PORT_FORWARD_GENERAL)
{"PortMapping",			eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,	&tWAN_PortMapping_OP},
#endif
{"Stats",			eCWMP_tOBJECT,	CWMP_READ,		NULL},
#if defined(_PRMT_X_CT_COM_DDNS_) && defined(CONFIG_USER_DDNS)
{"X_CT-COM_DDNSConfiguration",	eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,	&tWAN_DDNSConf_OP},
#endif
#ifdef _PRMT_X_CT_COM_DHCP_
{"X_CT-COM_DHCPOPTION60",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"X_CT-COM_DHCPOPTION125",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"X_CT-COM_DHCPv6OPTION16",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"X_CT-COM_DHCPv6OPTION17", eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif
};
enum eWANIPCONENTITYObject
{
/*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	eIP_DHCPClient,
#endif
/*ping_zhang:20080919 END*/
	eIP_PortMapping,
	eIP_Stats,
#if defined(_PRMT_X_CT_COM_DDNS_) && defined(CONFIG_USER_DDNS)
	eIP_X_CTCOM_DDNSConfiguration,
#endif
#ifdef _PRMT_X_CT_COM_DHCP_
	eIP_X_CTCOM_DHCPOPTION60,
	eIP_X_CTCOM_DHCPOPTION125,
	eIP_X_CTCOM_DHCPV6OPTION16,
	eIP_X_CTCOM_DHCPV6OPTION17,
#endif
};
struct CWMP_NODE tWANIPCONENTITYObject[] =
{
/*info,  							leaf,			node)*/
/*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
{&tWANIPCONENTITYObjectInfo[eIP_DHCPClient],			tWANIPConDHCPClientLeaf,			tWANIPConDHCPClientObject},
#endif
/*ping_zhang:20080919 END*/
{&tWANIPCONENTITYObjectInfo[eIP_PortMapping],			NULL,			NULL},
{&tWANIPCONENTITYObjectInfo[eIP_Stats],				tWANCONSTATSLeaf,	NULL},
#if defined(_PRMT_X_CT_COM_DDNS_) && defined(CONFIG_USER_DDNS)
{&tWANIPCONENTITYObjectInfo[eIP_X_CTCOM_DDNSConfiguration],	NULL,			NULL},
#endif
#ifdef _PRMT_X_CT_COM_DHCP_
{&tWANIPCONENTITYObjectInfo[eIP_X_CTCOM_DHCPOPTION60],	NULL,	tCTDhcpOpt60Object},
{&tWANIPCONENTITYObjectInfo[eIP_X_CTCOM_DHCPOPTION125],	NULL,	tCTDhcpOpt125Object},
{&tWANIPCONENTITYObjectInfo[eIP_X_CTCOM_DHCPV6OPTION16],	NULL,	tCTDhcpv6Opt16Object},
{&tWANIPCONENTITYObjectInfo[eIP_X_CTCOM_DHCPV6OPTION17],	NULL,	tCTDhcpv6Opt17Object},
#endif
{NULL,								NULL,			NULL}
};


struct CWMP_PRMT tWANIPCONObjectInfo[] =
{
/*(name,			type,		flag,					op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eWANIPCONObject
{
	eWANIPCON0
};
struct CWMP_LINKNODE tWANIPCONObject[] =
{
/*info,  				leaf,			next,				sibling,		instnum)*/
{&tWANIPCONObjectInfo[eWANIPCON0],	tWANIPCONENTITYLeaf,	tWANIPCONENTITYObject,		NULL,			0}
};


#ifdef CONFIG_DEV_xDSL
struct CWMP_OP tDSLLNKCONFLeafOP = { getDSLLNKCONF, setDSLLNKCONF };
struct CWMP_PRMT tDSLLNKCONFLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tDSLLNKCONFLeafOP},
{"LinkStatus",			eCWMP_tSTRING,	CWMP_READ,		&tDSLLNKCONFLeafOP},
{"LinkType",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tDSLLNKCONFLeafOP},
{"AutoConfig",			eCWMP_tBOOLEAN,	CWMP_READ,		&tDSLLNKCONFLeafOP},
{"ModulationType",		eCWMP_tSTRING,	CWMP_READ,		&tDSLLNKCONFLeafOP},
{"DestinationAddress",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tDSLLNKCONFLeafOP},
{"ATMEncapsulation",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tDSLLNKCONFLeafOP},
/*FCSPreserved*/
/*VCSearchList*/
{"ATMAAL",			eCWMP_tSTRING,	CWMP_READ,		&tDSLLNKCONFLeafOP},
{"ATMTransmittedBlocks",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tDSLLNKCONFLeafOP},
{"ATMReceivedBlocks",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tDSLLNKCONFLeafOP},
{"ATMQoS",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tDSLLNKCONFLeafOP},
{"ATMPeakCellRate",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tDSLLNKCONFLeafOP},
{"ATMMaximumBurstSize",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tDSLLNKCONFLeafOP},
{"ATMSustainableCellRate",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tDSLLNKCONFLeafOP},
{"AAL5CRCErrors",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tDSLLNKCONFLeafOP},
{"ATMCRCErrors",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tDSLLNKCONFLeafOP}
/*ATMHECErrors*/
};
enum eDSLLNKCONFLeaf
{
	eDL_Enable,
	eDL_LinkStatus,
	eDL_LinkType,
	eDL_AutoConfig,
	eDL_ModulationType,
	eDL_DestinationAddress,
	eDL_ATMEncapsulation,
	eDL_ATMAAL,
	eDL_ATMTransmittedBlocks,
	eDL_ATMReceivedBlocks,
	eDL_ATMQoS,
	eDL_ATMPeakCellRate,
	eDL_ATMMaximumBurstSize,
	eDL_ATMSustainableCellRate,
	eDL_AAL5CRCErrors,
	eDL_ATMCRCErrors
};
struct CWMP_LEAF tDSLLNKCONFLeaf[] =
{
{ &tDSLLNKCONFLeafInfo[eDL_Enable] },
{ &tDSLLNKCONFLeafInfo[eDL_LinkStatus] },
{ &tDSLLNKCONFLeafInfo[eDL_LinkType] },
{ &tDSLLNKCONFLeafInfo[eDL_AutoConfig] },
{ &tDSLLNKCONFLeafInfo[eDL_ModulationType] },
{ &tDSLLNKCONFLeafInfo[eDL_DestinationAddress] },
{ &tDSLLNKCONFLeafInfo[eDL_ATMEncapsulation] },
#ifdef _PRMT_X_CT_COM_WANEXT_
{ &tCTDSLLNKCONFLeafInfo[eDL_X_CTCOM_VLAN] },
#endif
{ &tDSLLNKCONFLeafInfo[eDL_ATMAAL] },
{ &tDSLLNKCONFLeafInfo[eDL_ATMTransmittedBlocks] },
{ &tDSLLNKCONFLeafInfo[eDL_ATMReceivedBlocks] },
{ &tDSLLNKCONFLeafInfo[eDL_ATMQoS] },
{ &tDSLLNKCONFLeafInfo[eDL_ATMPeakCellRate] },
{ &tDSLLNKCONFLeafInfo[eDL_ATMMaximumBurstSize] },
{ &tDSLLNKCONFLeafInfo[eDL_ATMSustainableCellRate] },
{ &tDSLLNKCONFLeafInfo[eDL_AAL5CRCErrors] },
{ &tDSLLNKCONFLeafInfo[eDL_ATMCRCErrors] },
{ NULL }
};


#ifdef CONFIG_PTMWAN
struct CWMP_OP tPTMLNKSTATSLeafOP = { getPTMLNKSTATS, NULL };
struct CWMP_PRMT tPTMLNKSTATSLeafInfo[] =
{
/*(name,						type,			flag,			op)*/
{"BytesSent",				eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tPTMLNKSTATSLeafOP},
{"BytesReceived",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tPTMLNKSTATSLeafOP},
{"FramesSent",				eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tPTMLNKSTATSLeafOP},
{"FramesReceived",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tPTMLNKSTATSLeafOP},
{"OOSNearEnd",				eCWMP_tBOOLEAN,	CWMP_READ|CWMP_DENY_ACT,&tPTMLNKSTATSLeafOP},
{"OOSFarEnd",				eCWMP_tBOOLEAN,	CWMP_READ|CWMP_DENY_ACT,&tPTMLNKSTATSLeafOP},
{"ErrorsSent",				eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tPTMLNKSTATSLeafOP},
{"ErrorsReceived",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tPTMLNKSTATSLeafOP},
{"UnicastPacketsSent",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tPTMLNKSTATSLeafOP},
{"UnicastPacketsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tPTMLNKSTATSLeafOP},
{"DiscardPacketsSent",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tPTMLNKSTATSLeafOP},
{"DiscardPacketsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tPTMLNKSTATSLeafOP},
{"MulticastPacketsSent",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tPTMLNKSTATSLeafOP},
{"MulticastPacketsReceived",eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tPTMLNKSTATSLeafOP},
{"BroadcastPacketsSent",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tPTMLNKSTATSLeafOP},
{"BroadcastPacketsReceived",eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tPTMLNKSTATSLeafOP},
{"UnknownProtoPacketsReceived",eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tPTMLNKSTATSLeafOP}
};
enum ePTMLNKSTATSLeaf
{
	eBytesSent,
	eBytesReceived,
	eFramesSent,
	eFramesReceived,
	eOOSNearEnd,
	eOOSFarEnd,
	eErrorsSent,
	eErrorsReceived,
	eUnicastPacketsSent,
	eUnicastPacketsReceived,
	eDiscardPacketsSent,
	eDiscardPacketsReceived,
	eMulticastPacketsSent,
	eMulticastPacketsReceived,
	eBroadcastPacketsSent,
	eBroadcastPacketsReceived,
	eUnknownProtoPacketsReceived
};
struct CWMP_LEAF tPTMLNKSTATSLeaf[] =
{
{ &tPTMLNKSTATSLeafInfo[eBytesSent] },
{ &tPTMLNKSTATSLeafInfo[eBytesReceived] },
{ &tPTMLNKSTATSLeafInfo[eFramesSent] },
{ &tPTMLNKSTATSLeafInfo[eFramesReceived] },
{ &tPTMLNKSTATSLeafInfo[eOOSNearEnd] },
{ &tPTMLNKSTATSLeafInfo[eOOSFarEnd] },
{ &tPTMLNKSTATSLeafInfo[eErrorsSent] },
{ &tPTMLNKSTATSLeafInfo[eErrorsReceived] },
{ &tPTMLNKSTATSLeafInfo[eUnicastPacketsSent] },
{ &tPTMLNKSTATSLeafInfo[eUnicastPacketsReceived] },
{ &tPTMLNKSTATSLeafInfo[eDiscardPacketsSent] },
{ &tPTMLNKSTATSLeafInfo[eDiscardPacketsReceived] },
{ &tPTMLNKSTATSLeafInfo[eMulticastPacketsSent] },
{ &tPTMLNKSTATSLeafInfo[eMulticastPacketsReceived] },
{ &tPTMLNKSTATSLeafInfo[eBroadcastPacketsSent] },
{ &tPTMLNKSTATSLeafInfo[eBroadcastPacketsReceived] },
{ &tPTMLNKSTATSLeafInfo[eUnknownProtoPacketsReceived] },
{ NULL }
};

struct CWMP_PRMT tPTMLNKCONFObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Stats",		eCWMP_tOBJECT,	CWMP_READ,		NULL},
};
enum ePTMLNKCONFObject
{
	ePL_Stats,
};
struct CWMP_NODE tPTMLNKCONFObject[] =
{
/*info,  							leaf,			node)*/
{&tPTMLNKCONFObjectInfo[ePL_Stats],	tPTMLNKSTATSLeaf,	NULL},
{NULL,								NULL,				NULL}
};

struct CWMP_OP tPTMLNKCONFLeafOP = { getPTMLNKCONF, setPTMLNKCONF };
struct CWMP_PRMT tPTMLNKCONFLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tPTMLNKCONFLeafOP},
{"LinkStatus",	eCWMP_tSTRING,	CWMP_READ,				&tPTMLNKCONFLeafOP},
{"MACAddress",	eCWMP_tSTRING,	CWMP_READ,				&tPTMLNKCONFLeafOP}
};
enum ePTMLNKCONFLeaf
{
	ePL_Enable,
	ePL_LinkStatus,
	ePL_MACAddress,
};
struct CWMP_LEAF tPTMLNKCONFLeaf[] =
{
{ &tPTMLNKCONFLeafInfo[ePL_Enable] },
{ &tPTMLNKCONFLeafInfo[ePL_LinkStatus] },
{ &tPTMLNKCONFLeafInfo[ePL_MACAddress] },
{ NULL }
};
#endif /*CONFIG_PTMWAN*/
#endif /*CONFIG_DEV_xDSL*/

#ifdef CONFIG_ETHWAN
struct CWMP_OP tWANETHLINKFLeafOP = { getWANETHLINKCONF, NULL };
struct CWMP_PRMT tWANETHLINKLeafInfo[] =
{
	/*(name,			type,		flag,			op)*/
	{"EthernetLinkStatus", eCWMP_tSTRING, CWMP_READ,	&tWANETHLINKFLeafOP}
};
enum eWANETHLINKLeaf
{
	eEthernetLinkStatus
};
struct CWMP_LEAF tWANETHLINKLeaf[] =
{
#ifdef _PRMT_X_CT_COM_ETHLINK_
	{ &tCTWANETHLINKLeafInfo[eX_CTCOM_Enable] },
	{ &tCTWANETHLINKLeafInfo[eX_CTCOM_Mode] },
	{ &tCTWANETHLINKLeafInfo[eX_CTCOM_VLANIDMark] },
	{ &tCTWANETHLINKLeafInfo[eX_CTCOM_8021pMark] },
#endif
	{ &tWANETHLINKLeafInfo[eEthernetLinkStatus] },
	{ NULL }
};
#endif // #ifdef CONFIG_ETHWAN

struct CWMP_OP tWANCONDEVENTITYLeafOP = { getWANCONDEVENTITY, NULL };
struct CWMP_PRMT tWANCONDEVENTITYLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"WANIPConnectionNumberOfEntries",eCWMP_tUINT,	CWMP_READ,		&tWANCONDEVENTITYLeafOP},
{"WANPPPConnectionNumberOfEntries",eCWMP_tUINT,	CWMP_READ,		&tWANCONDEVENTITYLeafOP}
};
enum eWANCONDEVENTITYLeaf
{
	eWANIPConnectionNumberOfEntries,
	eWANPPPConnectionNumberOfEntries
};
struct CWMP_LEAF tWANCONDEVENTITYLeaf[] =
{
{ &tWANCONDEVENTITYLeafInfo[eWANIPConnectionNumberOfEntries] },
{ &tWANCONDEVENTITYLeafInfo[eWANPPPConnectionNumberOfEntries] },
{ NULL }
};

struct CWMP_OP tWAN_WANIPConnection_OP = { NULL, objWANIPConn };
struct CWMP_OP tWAN_WANPPPConnection_OP = { NULL, objWANPPPConn };
struct CWMP_PRMT tWANCONDEVENTITYObjectInfo[] =
{
	/*(name,			type,		flag,			op)*/
#ifdef CONFIG_DEV_xDSL
	{"WANDSLLinkConfig",		eCWMP_tOBJECT,	CWMP_READ,		NULL},
#ifdef CONFIG_PTMWAN
	{"WANPTMLinkConfig",		eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif /*CONFIG_PTMWAN*/
#endif
#ifdef CONFIG_ETHWAN
	{"WANEthernetLinkConfig",		eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
	{"WANATMF5LoopbackDiagnostics",	eCWMP_tOBJECT,	CWMP_READ,		NULL},
	{"WANIPConnection",		eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,	&tWAN_WANIPConnection_OP},
	{"WANPPPConnection",		eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,	&tWAN_WANPPPConnection_OP},
#if defined(_PRMT_X_CT_COM_WANEXT_) && defined(CONFIG_VDSL)
	{"X_CT-COM_WANVdslLinkConfig",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif
#if defined(_PRMT_X_CT_COM_WANEXT_) && defined(CONFIG_GPON_FEATURE)
	{"X_CT-COM_WANGponLinkConfig",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif
#if defined(_PRMT_X_CT_COM_WANEXT_) && defined(CONFIG_EPON_FEATURE)
	{"X_CT-COM_WANEponLinkConfig",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif
};
enum eWANCONDEVENTITYObject
{
#ifdef CONFIG_DEV_xDSL
	eWANDSLLinkConfig,
#ifdef CONFIG_PTMWAN
	eWANPTMLinkConfig,
#endif /*CONFIG_PTMWAN*/
#endif
#ifdef CONFIG_ETHWAN
	eWANEthernetLinkConfig,
#endif
	eWANATMF5LoopbackDiagnostics,
	eWANIPConnection,
	eWANPPPConnection,
#if defined(_PRMT_X_CT_COM_WANEXT_) && defined(CONFIG_VDSL)
	eX_CTCOM_WANVdslLinkConfig,
#endif
#if defined(_PRMT_X_CT_COM_WANEXT_) && defined(CONFIG_GPON_FEATURE)
	eX_CTCOM_WANGponLinkConfig,
#endif
#if defined(_PRMT_X_CT_COM_WANEXT_) && defined(CONFIG_EPON_FEATURE)
	eX_CTCOM_WANEponLinkConfig,
#endif

};

#ifdef CONFIG_DEV_xDSL
struct CWMP_NODE tWANCONDEVENTITYObjectDSL[] =
{
	/*info,  							leaf,			node)*/
	{&tWANCONDEVENTITYObjectInfo[eWANDSLLinkConfig], tDSLLNKCONFLeaf, NULL},
	{&tWANCONDEVENTITYObjectInfo[eWANATMF5LoopbackDiagnostics],	 tWANATMF5LBLeaf, NULL},
	{&tWANCONDEVENTITYObjectInfo[eWANIPConnection], NULL, NULL},
	{&tWANCONDEVENTITYObjectInfo[eWANPPPConnection], NULL, NULL},
	{NULL, NULL, NULL}
};
#ifdef CONFIG_PTMWAN
struct CWMP_NODE tWANCONDEVENTITYObjectPTM[] =
{
	/*info,  							leaf,			node)*/
	{&tWANCONDEVENTITYObjectInfo[eWANPTMLinkConfig], tPTMLNKCONFLeaf, tPTMLNKCONFObject},
	{&tWANCONDEVENTITYObjectInfo[eWANIPConnection], NULL, NULL},
	{&tWANCONDEVENTITYObjectInfo[eWANPPPConnection], NULL, NULL},
#if defined(_PRMT_X_CT_COM_WANEXT_) && defined(CONFIG_VDSL)
	{&tWANCONDEVENTITYObjectInfo[eX_CTCOM_WANVdslLinkConfig], tCT_VDSLLinkLeaf, NULL},
#endif
	{NULL, NULL, NULL}
};
#endif /*CONFIG_PTMWAN*/
#endif

#ifdef CONFIG_ETHWAN
struct CWMP_NODE tWANCONDEVENTITYObjectEth[] =
{
	/*info,  							leaf,			node)*/
	{&tWANCONDEVENTITYObjectInfo[eWANEthernetLinkConfig], tWANETHLINKLeaf, NULL},
	//{&tWANCONDEVENTITYObjectInfo[eWANATMF5LoopbackDiagnostics],	 tWANATMF5LBLeaf, NULL},
	{&tWANCONDEVENTITYObjectInfo[eWANIPConnection], NULL, NULL},
	{&tWANCONDEVENTITYObjectInfo[eWANPPPConnection], NULL, NULL},
	{NULL, NULL, NULL}
};
#endif

#if defined(_PRMT_X_CT_COM_WANEXT_) && defined(CONFIG_GPON_FEATURE)
struct CWMP_NODE tWANCONDEVENTITYObjectGPON[] =
{
	/*info, 							leaf,			node)*/
	{&tWANCONDEVENTITYObjectInfo[eX_CTCOM_WANGponLinkConfig], tCT_XPONLinkLeaf, NULL},
	{&tWANCONDEVENTITYObjectInfo[eWANIPConnection], NULL, NULL},
	{&tWANCONDEVENTITYObjectInfo[eWANPPPConnection], NULL, NULL},
	{NULL, NULL, NULL}
};
#endif

#if defined(_PRMT_X_CT_COM_WANEXT_) && defined(CONFIG_EPON_FEATURE)
struct CWMP_NODE tWANCONDEVENTITYObjectEPON[] =
{
	/*info, 							leaf,			node)*/
	{&tWANCONDEVENTITYObjectInfo[eX_CTCOM_WANEponLinkConfig], tCT_XPONLinkLeaf, NULL},
	{&tWANCONDEVENTITYObjectInfo[eWANIPConnection], NULL, NULL},
	{&tWANCONDEVENTITYObjectInfo[eWANPPPConnection], NULL, NULL},
	{NULL, NULL, NULL}
};
#endif


struct CWMP_PRMT tWANCONDEVObjectInfo[] =
{
/*(name,			type,		flag,					op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eWANCONDEVObject
{
	eWANCONDEV0
};

#ifdef CONFIG_DEV_xDSL
struct CWMP_LINKNODE tWANCONDEVObjectDSL[] =
{
/*info,  				leaf,			next,			sibling,		instnum)*/
{&tWANCONDEVObjectInfo[eWANCONDEV0], tWANCONDEVENTITYLeaf, tWANCONDEVENTITYObjectDSL, NULL, 0},
};
#ifdef CONFIG_PTMWAN
struct CWMP_LINKNODE tWANCONDEVObjectPTM[] =
{
/*info,  				leaf,			next,			sibling,		instnum)*/
{&tWANCONDEVObjectInfo[eWANCONDEV0], tWANCONDEVENTITYLeaf, tWANCONDEVENTITYObjectPTM, NULL, 0},
};
#endif /*CONFIG_PTMWAN*/
#endif

#ifdef CONFIG_ETHWAN
struct CWMP_LINKNODE tWANCONDEVObjectEth[] =
{
/*info,  				leaf,			next,			sibling,		instnum)*/
{&tWANCONDEVObjectInfo[eWANCONDEV0], tWANCONDEVENTITYLeaf, tWANCONDEVENTITYObjectEth, NULL, 0},
};
#endif

#if defined(_PRMT_X_CT_COM_WANEXT_) && defined(CONFIG_GPON_FEATURE)
struct CWMP_LINKNODE tWANCONDEVObjectGPON[] =
{
/*info,  				leaf,			next,			sibling,		instnum)*/
{&tWANCONDEVObjectInfo[eWANCONDEV0], tWANCONDEVENTITYLeaf, tWANCONDEVENTITYObjectGPON, NULL, 0},
};
#endif

#if defined(_PRMT_X_CT_COM_WANEXT_) && defined(CONFIG_EPON_FEATURE)
struct CWMP_LINKNODE tWANCONDEVObjectEPON[] =
{
/*info,  				leaf,			next,			sibling,		instnum)*/
{&tWANCONDEVObjectInfo[eWANCONDEV0], tWANCONDEVENTITYLeaf, tWANCONDEVENTITYObjectEPON, NULL, 0},
};
#endif



struct CWMP_OP tCONSERENTITYLeafOP = { getCONSERENTITY, NULL };
struct CWMP_PRMT tCONSERENTITYLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"WANConnectionDevice",		eCWMP_tSTRING,	CWMP_READ,		&tCONSERENTITYLeafOP},
{"WANConnectionService",	eCWMP_tSTRING,	CWMP_READ,		&tCONSERENTITYLeafOP},
{"DestinationAddress",		eCWMP_tSTRING,	CWMP_READ,		&tCONSERENTITYLeafOP},
{"LinkType",			eCWMP_tSTRING,	CWMP_READ,		&tCONSERENTITYLeafOP},
{"ConnectionType",		eCWMP_tSTRING,	CWMP_READ,		&tCONSERENTITYLeafOP},
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{"Name",		eCWMP_tSTRING,	CWMP_READ,		&tCONSERENTITYLeafOP},
#endif
/*ping_zhang:20081217 END*/
};
enum eCONSERENTITYLeaf
{
	eCS_WANConnectionDevice,
	eCS_WANConnectionService,
	eCS_DestinationAddress,
	eCS_LinkType,
	eCS_ConnectionType,
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	eCS_Name,
#endif
/*ping_zhang:20081217 END*/
};
struct CWMP_LEAF tCONSERENTITYLeaf[] =
{
{ &tCONSERENTITYLeafInfo[eCS_WANConnectionDevice] },
{ &tCONSERENTITYLeafInfo[eCS_WANConnectionService] },
{ &tCONSERENTITYLeafInfo[eCS_DestinationAddress] },
{ &tCONSERENTITYLeafInfo[eCS_LinkType] },
{ &tCONSERENTITYLeafInfo[eCS_ConnectionType] },
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{ &tCONSERENTITYLeafInfo[eCS_Name] },
#endif
/*ping_zhang:20081217 END*/
{ NULL }
};




struct CWMP_PRMT tCONSERVICEObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_LNKLIST,	NULL}
};
enum eCONSERVICEObject
{
	eCONSERVICE0
};
struct CWMP_LINKNODE tCONSERVICEObject[] =
{
/*info,  				leaf,			next,		sibling,		instnum)*/
{&tCONSERVICEObjectInfo[eCONSERVICE0],	tCONSERENTITYLeaf,	NULL,		NULL,			0},
};


struct CWMP_OP tWANDSLCNTMNGLeafOP = { getWANDSLCNTMNG,NULL };
struct CWMP_PRMT tWANDSLCNTMNGLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"ConnectionServiceNumberOfEntries",eCWMP_tUINT,CWMP_READ,		&tWANDSLCNTMNGLeafOP}
};
enum eWANDSLCNTMNGLeaf
{
	eConnectionServiceNumberOfEntries
};
struct CWMP_LEAF tWANDSLCNTMNGLeaf[]=
{
{ &tWANDSLCNTMNGLeafInfo[eConnectionServiceNumberOfEntries]},
{ NULL }
};
struct CWMP_OP tWAN_ConnectionService_OP = { NULL, objConService };
struct CWMP_PRMT tWANDSLCNTMNGObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"ConnectionService",		eCWMP_tOBJECT,	CWMP_READ,		&tWAN_ConnectionService_OP}
};
enum eWANDSLCNTMNGObject
{
	eConnectionService
};
struct CWMP_NODE tWANDSLCNTMNGObject[] =
{
/*info,  					leaf,			node)*/
{&tWANDSLCNTMNGObjectInfo[eConnectionService],	NULL,			NULL},
{NULL,						NULL,			NULL}
};


/*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
int checkandmodify_reqoption_order(unsigned int order, int chainid, unsigned int ifIndex)
{
	int ret=-1;
	int num,i;
	int maxorder;
	MIB_CE_DHCP_OPTION_T *p,pentry;

	p=&pentry;
	maxorder=findMaxDHCPReqOptionOrder(ifIndex);
	if(order>maxorder+1)
		goto checkresult;
	else{
		num = mib_chain_total( MIB_DHCP_CLIENT_OPTION_TBL );
		for( i=0; i<num;i++ )
		{
			if(i==chainid)
				continue;
			if( !mib_chain_get( MIB_DHCP_CLIENT_OPTION_TBL, i, (void*)p ))
				continue;
			if(p->usedFor!=eUsedFor_DHCPClient_Req)
				continue;
			if(p->ifIndex != ifIndex)
					continue;
			if(p->order>=order){
				(p->order)++;
				mib_chain_update(MIB_DHCP_CLIENT_OPTION_TBL,(void*)p,i);
			}
		}
		ret=0;
	}

checkresult:
	return ret;
}

unsigned int DHCPClientReservedOption[]={
	53,  //DHCP Message type
	54,  //Server id
	50,  //Requested IP Addr
	0
};//star: I think some important options can't be changed by user, because they are necessary or are managed by other parameters.
int checkDHCPClientOptionTag(unsigned int tagvalue)
{
	//int i;
	unsigned int *tmp=DHCPClientReservedOption;

	while(*tmp!=0){
		if(tagvalue == *tmp)
			return -1;
		tmp++;
	}

	return 0;
}

int getWANIPConDHCPClientENTITY(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char usedFor;
	MIB_CE_ATM_VC_T pvcentry;
	unsigned int pvcchainidx;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;

	if( getATMVCEntry(name, &pvcentry, &pvcchainidx) < 0 )
		return ERR_9002;

	if( strcmp( lastname, "SentDHCPOptionNumberOfEntries" )==0 )
	{
		usedFor = eUsedFor_DHCPClient_Sent;
		*data = uintdup(findDHCPOptionNum(usedFor, pvcentry.ifIndex));
	}
	else if( strcmp( lastname, "ReqDHCPOptionNumberOfEntries" )==0 )
	{
		usedFor = eUsedFor_DHCPClient_Req;
		*data = uintdup(findDHCPOptionNum(usedFor,pvcentry.ifIndex));
	}
	else
	{
		return ERR_9005;
	}
	return 0;
}

int getDHCPClientOptENTITY(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	MIB_CE_DHCP_OPTION_T *pDHCPOptEntry, DhcpOptEntry;
	unsigned int sentDhcpOptNum,reqDhcpOptNum;
	MIB_CE_ATM_VC_T pvcentry;
	unsigned char usedFor;
	unsigned int chainid,pvcchainidx;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	sentDhcpOptNum = getSentDHCPOptInstNum( name );
	reqDhcpOptNum = getReqDHCPOptInstNum( name );

	if( sentDhcpOptNum==0 && reqDhcpOptNum ==0 ) return ERR_9005;

	pDHCPOptEntry = &DhcpOptEntry;

	if( getATMVCEntry(name, &pvcentry, &pvcchainidx) < 0 )
		return ERR_9002;

	if(sentDhcpOptNum != 0)   	//for IGD.LANDevice.{i}.WANDevice.{i}.WANIPConnectionDevice.{i}.DHCPClient.SentDHCPOption.{i}.
	{
		usedFor = eUsedFor_DHCPClient_Sent;
		if( getDHCPClientOptionByOptInstNum(sentDhcpOptNum, pvcentry.ifIndex,  usedFor, pDHCPOptEntry, &chainid) < 0)
			return ERR_9002;
	}
	else if(reqDhcpOptNum !=0)	//for IGD.LANDevice.{i}.WANDevice.{i}.WANIPConnectionDevice.{i}.DHCPClient.ReqDHCPOption.{i}.
	{
		usedFor = eUsedFor_DHCPClient_Req;
		if( getDHCPClientOptionByOptInstNum(reqDhcpOptNum, pvcentry.ifIndex, usedFor,pDHCPOptEntry, &chainid) < 0 )
			return ERR_9002;
	}

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "Enable" )==0 )
	{
		if(pDHCPOptEntry->enable)
			*data = booldup(1);
		else
			*data = booldup(0);
	}
	else if( strcmp( lastname, "Order")==0)
	{
		if(usedFor != eUsedFor_DHCPClient_Req)
			return ERR_9005;
		*data = uintdup(pDHCPOptEntry->order);
	}
	else if( strcmp( lastname, "Tag" )==0 )
	{
		*data = uintdup(pDHCPOptEntry->tag);
	}
	else if( strcmp( lastname, "Value" )==0 )
	{
		struct SOAP_ENC__base64 tmp;
		tmp.__ptr=pDHCPOptEntry->value;
		tmp.__size=pDHCPOptEntry->len;
		*data=base64dup( tmp );
	}
	else
	{
		return ERR_9005;
	}
	return 0;
}

int setDHCPClientOptENTITY(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char *buf = data;
	MIB_CE_DHCP_OPTION_T *pDHCPOptEntry, DhcpOptEntry;
	unsigned int sentDhcpOptNum,reqDhcpOptNum;
	MIB_CE_ATM_VC_T pvcentry;
	unsigned char usedFor;
	unsigned int chainid, pvcchainidx;

	if( (name==NULL) || (type==0) || (data==NULL) || (entity==NULL))
		return -1;

	sentDhcpOptNum = getSentDHCPOptInstNum( name );
	reqDhcpOptNum = getReqDHCPOptInstNum( name );

	if (sentDhcpOptNum==0 && reqDhcpOptNum ==0 ) return ERR_9005;

	pDHCPOptEntry = &DhcpOptEntry;
	if( getATMVCEntry(name, &pvcentry, &pvcchainidx) < 0 )
		return ERR_9002;

	if(sentDhcpOptNum != 0)   	//for IGD.LANDevice.{i}.WANDevice.{i}.WANIPConnectionDevice.{i}.DHCPClient.SentDHCPOption.{i}.
	{
		usedFor = eUsedFor_DHCPClient_Sent;
		if( getDHCPClientOptionByOptInstNum(sentDhcpOptNum, pvcentry.ifIndex, usedFor, pDHCPOptEntry, &chainid) < 0)
		return ERR_9002;
	}
	else if(reqDhcpOptNum !=0)	//for IGD.LANDevice.{i}.WANDevice.{i}.WANIPConnectionDevice.{i}.DHCPClient.ReqDHCPOption.{i}.
	{
		usedFor = eUsedFor_DHCPClient_Req;
		if( getDHCPClientOptionByOptInstNum(reqDhcpOptNum, pvcentry.ifIndex, usedFor,pDHCPOptEntry, &chainid) < 0 )
		return ERR_9002;
	}

	if(usedFor==eUsedFor_DHCPClient_Sent){
		if(checkDHCPClientOptionTag(pDHCPOptEntry->tag)<0)
			return ERR_9001;
	}

	gWanBitMap |= (1U << pvcchainidx);

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i=data;
		if(i==NULL) return ERR_9007;
		pDHCPOptEntry->enable = (*i==0) ? 0:1;
		mib_chain_update( MIB_DHCP_CLIENT_OPTION_TBL, (unsigned char*)pDHCPOptEntry, chainid );
		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "Order" )==0 )
	{
		unsigned int *i=data;
		if(i==NULL) return ERR_9007;
		if(*i<1 ) return ERR_9007;
		if(pDHCPOptEntry->usedFor != eUsedFor_DHCPClient_Req) return ERR_9005;
		if(checkandmodify_reqoption_order(*i,chainid,pvcentry.ifIndex)<0) return ERR_9007;
		pDHCPOptEntry->order = *i;
		mib_chain_update( MIB_DHCP_CLIENT_OPTION_TBL, (unsigned char*)pDHCPOptEntry, chainid );
		compact_reqoption_order(pvcentry.ifIndex);
		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "Tag" )==0 )
	{
		int *i=data;
		if(i==NULL) return ERR_9007;
		if(*i<1 || *i>254) return ERR_9007;
		if(usedFor==eUsedFor_DHCPClient_Sent){
			if(checkDHCPClientOptionTag(*i)<0)
				return ERR_9001;
		}
		pDHCPOptEntry->tag = *i;
		mib_chain_update( MIB_DHCP_CLIENT_OPTION_TBL, (unsigned char*)pDHCPOptEntry, chainid );
		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "Value" )==0 )
	{
		if( buf==NULL ) return ERR_9007;

		if(pDHCPOptEntry->usedFor == eUsedFor_DHCPClient_Req)
			return ERR_9001;

		if(data)
		{
			   int i;
			   struct SOAP_ENC__base64 *b=data;
			   fprintf( stderr, "<SOAP_ENC__base64:size %d>", b->__size );
			   for( i=0; i<b->__size; i++ )
			   {
			    fprintf( stderr, "%u(%c) ", b->__ptr[i], b->__ptr[i]  );
			   }
			   fprintf( stderr, "\n" );
			    if(b->__size>DHCP_OPT_VAL_LEN) return ERR_9001;
			   pDHCPOptEntry->len=b->__size;
			   memcpy(pDHCPOptEntry->value,b->__ptr,b->__size);
		}
		mib_chain_update( MIB_DHCP_CLIENT_OPTION_TBL, (unsigned char*)pDHCPOptEntry, chainid );
		return CWMP_NEED_RESTART_WAN;
	}
	else
	{
		return ERR_9005;
	}
	return CWMP_APPLIED;
}

//for IGD.LANDevice.{i}.WANDevice.{i}.WANIPConnectionDevice.{i}.DHCPClient.SentDHCPOption.{i}.
int objDHCPClientSentOpt(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	MIB_CE_DHCP_OPTION_T *pDHCPOptEntry, DhcpOptEntry;
	unsigned int num,i;
	unsigned char usedFor;
	//unsigned int chainid;
	MIB_CE_ATM_VC_T pvcentry;
	unsigned int pvcchainidx;

	usedFor = eUsedFor_DHCPClient_Sent;

	if( getATMVCEntry(name, &pvcentry, &pvcchainidx) < 0 )
		return ERR_9002;

	switch( type )
	{
	case eCWMP_tINITOBJ:
		{
			int MaxInstNum;
			struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			MaxInstNum = findMaxDHCPClientOptionInstNum(usedFor, pvcentry.ifIndex);
			num = mib_chain_total( MIB_DHCP_CLIENT_OPTION_TBL );
			for( i=0; i<num;i++ )
			{
				pDHCPOptEntry = &DhcpOptEntry;
				if( !mib_chain_get( MIB_DHCP_CLIENT_OPTION_TBL, i, (void*)pDHCPOptEntry ))
					continue;

				if(pDHCPOptEntry->usedFor != usedFor ||  pDHCPOptEntry->ifIndex != pvcentry.ifIndex)
					continue;
				if( pDHCPOptEntry->dhcpOptInstNum==0 ) //maybe createn by web or cli
				{
					MaxInstNum++;
					pDHCPOptEntry->dhcpOptInstNum = MaxInstNum;
					mib_chain_update( MIB_DHCP_CLIENT_OPTION_TBL, (unsigned char*)pDHCPOptEntry, i );
				}
				{
					if( create_Object( c, tDHCPClientSentObject, sizeof(tDHCPClientSentObject), 1, pDHCPOptEntry->dhcpOptInstNum ) < 0 )
						return -1;
				}
			}
			add_objectNum( name, MaxInstNum );
			return CWMP_APPLIED;
		}
	case eCWMP_tADDOBJ:
		{
			int ret;//found=0;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tDHCPClientSentObject, sizeof(tDHCPClientSentObject), data );
			if( ret >= 0 )
			{
				MIB_CE_DHCP_OPTION_T entry;
				memset( &entry, 0, sizeof( MIB_CE_DHCP_OPTION_T ) );
				{ //default values for this new entry
					entry.enable = 0;
					entry.usedFor = usedFor;
					entry.dhcpOptInstNum = *(int *)data;
					entry.dhcpConSPInstNum = 0;
					entry.ifIndex = pvcentry.ifIndex;
				}

				mib_chain_add( MIB_DHCP_CLIENT_OPTION_TBL, (unsigned char*)&entry);
			}
			return ret;
		}
	case eCWMP_tDELOBJ:
		{
			int ret, num, i;
			int found = 0;
			unsigned int *pUint=data;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			num = mib_chain_total( MIB_DHCP_CLIENT_OPTION_TBL );
			for( i=num-1; i>=0;i-- )
			{
				pDHCPOptEntry = &DhcpOptEntry;
				if( !mib_chain_get( MIB_DHCP_CLIENT_OPTION_TBL, i, (void*)pDHCPOptEntry ) )
					continue;
				if(pDHCPOptEntry->usedFor == usedFor && pDHCPOptEntry->ifIndex == pvcentry.ifIndex
					&& pDHCPOptEntry->dhcpOptInstNum==*pUint)
				{
					found =1;
					mib_chain_delete( MIB_DHCP_CLIENT_OPTION_TBL, i );
					break;
				}
			}

			if(found==0) return ERR_9005;
			ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next, *(int*)data );
			if( ret==0 )	ret=1;
			return ret;
		}
	case eCWMP_tUPDATEOBJ:
		{
			int num,i;
			struct CWMP_LINKNODE *old_table;

			num = mib_chain_total( MIB_DHCP_CLIENT_OPTION_TBL );
			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;
			for( i=0; i<num;i++ )
			{
				struct CWMP_LINKNODE *remove_entity=NULL;
				pDHCPOptEntry = &DhcpOptEntry;
				if( !mib_chain_get( MIB_DHCP_CLIENT_OPTION_TBL, i, (void*)pDHCPOptEntry ))
					continue;

				if( (pDHCPOptEntry->usedFor == usedFor) && ( pDHCPOptEntry->ifIndex == pvcentry.ifIndex) && (pDHCPOptEntry->dhcpOptInstNum!=0))
				{
					remove_entity = remove_SiblingEntity( &old_table, pDHCPOptEntry->dhcpOptInstNum );
					if( remove_entity!=NULL )
					{
						add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
					}
					else
					{
						if( find_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, pDHCPOptEntry->dhcpOptInstNum )==NULL )
						{
							unsigned int MaxInstNum = pDHCPOptEntry->dhcpOptInstNum;
							add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tDHCPClientSentObject, sizeof(tDHCPClientSentObject), &MaxInstNum );
						}//else already in next_table
					}
				}
			}

			if( old_table )
				destroy_ParameterTable( (struct CWMP_NODE *)old_table );

			return 0;
		}
	}
	return -1;
}

//for IGD.LANDevice.{i}.WANDevice.{i}.WANIPConnectionDevice.{i}.DHCPClient.ReqDHCPOption.{i}.
int objDHCPClientReqOpt(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	MIB_CE_DHCP_OPTION_T *pDHCPOptEntry, DhcpOptEntry;
	unsigned int num,i;
	unsigned char usedFor;
	//unsigned int chainid;
	MIB_CE_ATM_VC_T pvcentry;
	unsigned int pvcchainidx;

	usedFor = eUsedFor_DHCPClient_Req;

	if( getATMVCEntry(name, &pvcentry, &pvcchainidx) < 0 )
		return ERR_9002;

	switch( type )
	{
	case eCWMP_tINITOBJ:
		{
			int MaxInstNum;
			struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			MaxInstNum = findMaxDHCPClientOptionInstNum(usedFor,pvcentry.ifIndex);
			num = mib_chain_total( MIB_DHCP_CLIENT_OPTION_TBL );
			for( i=0; i<num;i++ )
			{
				pDHCPOptEntry = &DhcpOptEntry;
				if( !mib_chain_get( MIB_DHCP_CLIENT_OPTION_TBL, i, (void*)pDHCPOptEntry ))
					continue;
				if(pDHCPOptEntry->usedFor != usedFor || pDHCPOptEntry->ifIndex!=pvcentry.ifIndex)
					continue;
				if( pDHCPOptEntry->dhcpOptInstNum==0 ) //maybe createn by web or cli
				{
					MaxInstNum++;
					pDHCPOptEntry->dhcpOptInstNum = MaxInstNum;
					mib_chain_update( MIB_DHCP_CLIENT_OPTION_TBL, (unsigned char*)pDHCPOptEntry, i );
				}
				{
					if( create_Object( c, tDHCPClientReqObject, sizeof(tDHCPClientReqObject), 1, pDHCPOptEntry->dhcpOptInstNum ) < 0 )
						return -1;
				}
			}
			add_objectNum( name, MaxInstNum );
			return 0;
		}
	case eCWMP_tADDOBJ:
		{
			int ret;//found=0;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tDHCPClientReqObject, sizeof(tDHCPClientReqObject), data );
			if( ret >= 0 )
			{
				MIB_CE_DHCP_OPTION_T entry;
				memset( &entry, 0, sizeof( MIB_CE_DHCP_OPTION_T ) );
				{ //default values for this new entry
					entry.enable = 0;
					entry.usedFor = usedFor;
					entry.dhcpOptInstNum = *(int *)data;
					entry.dhcpConSPInstNum = 0;
					entry.ifIndex = pvcentry.ifIndex;
					entry.order = findMaxDHCPReqOptionOrder(pvcentry.ifIndex)+1;
				}
				mib_chain_add( MIB_DHCP_CLIENT_OPTION_TBL, (unsigned char*)&entry);
			}
			return ret;
		}
	case eCWMP_tDELOBJ:
		{
			int ret, num, i;
			int found = 0;
			unsigned int *pUint=data;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			num = mib_chain_total( MIB_DHCP_CLIENT_OPTION_TBL );
			for( i=num-1; i>=0;i-- )
			{
				pDHCPOptEntry = &DhcpOptEntry;
				if( !mib_chain_get( MIB_DHCP_CLIENT_OPTION_TBL, i, (void*)pDHCPOptEntry ) )
					continue;
				if(pDHCPOptEntry->usedFor == usedFor && pDHCPOptEntry->ifIndex == pvcentry.ifIndex
					&& pDHCPOptEntry->dhcpOptInstNum==*pUint)
				{
					found =1;
					mib_chain_delete( MIB_DHCP_CLIENT_OPTION_TBL, i );
					compact_reqoption_order(pvcentry.ifIndex);
					break;
				}
			}

			if(found==0) return ERR_9005;
			ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next, *(int*)data );
			if( ret==0 )	ret=1;
			return ret;
		}
	case eCWMP_tUPDATEOBJ:
		{
			int num,i;
			struct CWMP_LINKNODE *old_table;

			num = mib_chain_total( MIB_DHCP_CLIENT_OPTION_TBL );
			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;
			for( i=0; i<num;i++ )
			{
				struct CWMP_LINKNODE *remove_entity=NULL;

				pDHCPOptEntry = &DhcpOptEntry;
				if( !mib_chain_get( MIB_DHCP_CLIENT_OPTION_TBL, i, (void*)pDHCPOptEntry ))
					continue;

				if( (pDHCPOptEntry->usedFor == usedFor) && (pDHCPOptEntry->ifIndex == pvcentry.ifIndex) && (pDHCPOptEntry->dhcpOptInstNum!=0))
				{
					remove_entity = remove_SiblingEntity( &old_table, pDHCPOptEntry->dhcpOptInstNum );
					if( remove_entity!=NULL )
					{
						add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
					}
					else
					{
						if( find_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, pDHCPOptEntry->dhcpOptInstNum )==NULL )
						{
							unsigned int MaxInstNum = pDHCPOptEntry->dhcpOptInstNum;
							add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tDHCPClientReqObject, sizeof(tDHCPClientReqObject), &MaxInstNum );
						}//else already in next_table
					}
				}
			}

			if( old_table )
			destroy_ParameterTable( (struct CWMP_NODE *)old_table );

			return 0;
		}
	}
	return -1;
}
#endif
/*ping_zhang:20080919 END*/

#ifdef VIRTUAL_SERVER_SUPPORT
int getPORMAPTENTITY(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned int 	mapnum, chainidx;
	int port_chainid=0;
	MIB_CE_ATM_VC_T	*pEntry, vc_entity;
	MIB_VIRTUAL_SVR_T *pPort, port_entity;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	mapnum  = getWANPortMapInstNum( name );

	if(mapnum == 0) return ERR_9005;

	pEntry = &vc_entity;

	if( getATMVCEntry(name, pEntry, &chainidx) < 0 )
		return ERR_9002;

	pPort = &port_entity;
	if( getPortMappingByInstNum( 0, mapnum, pPort, &port_chainid ) )
		return ERR_9002;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "PortMappingEnabled" )==0 )
	{
		*data = booldup(1);
	}else if( strcmp( lastname, "PortMappingLeaseDuration" )==0 )
	{
		*data = booldup(0);
	}else if( strcmp( lastname, "RemoteHost" )==0 )
	{
		*data = strdup( "" );
	}else if( strcmp( lastname, "ExternalPort" )==0 )
	{
//		*data=uintdup( pPort->externalport );
		*data=uintdup( pPort->wanEndPort );
	}else if( strcmp( lastname, "InternalPort" )==0 )
	{
		*data=uintdup( pPort->lanPort );
	}else if( strcmp( lastname, "PortMappingProtocol" )==0 )
	{
		if( pPort->protoType==PROTO_TCP )
			*data = strdup( "TCP" );
		else if( pPort->protoType==PROTO_UDP )
			*data = strdup( "UDP" );
		else if (pPort->protoType==0 )
			*data = strdup("TCPandUDP");
		else /*PROTO_NONE or PROTO_ICMP*/
			*data = strdup( "" );
	}else if( strcmp( lastname, "InternalClient" )==0 )
	{
		char *tmp;
		tmp = inet_ntoa(*((struct in_addr *)&(pPort->serverIp)));
		if(tmp)
			*data = strdup( tmp );
		else
			*data = strdup( "" );
	}else if( strcmp( lastname, "PortMappingDescription" )==0 )
	{
		*data = strdup( pPort->svrName );
	}else{
		return ERR_9005;
	}

	return 0;
}

int setPORMAPTENTITY(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	unsigned int 	mapnum;
	unsigned int	chainidx;
	int		port_chainid=0;
	char		*buf=data;
	MIB_CE_ATM_VC_T	*pEntry, vc_entity;
	MIB_VIRTUAL_SVR_T *pPort, port_entity, OldPort_Entity;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( data==NULL ) return ERR_9007;
	if( entity->info->type!=type ) return ERR_9006;

	mapnum  = getWANPortMapInstNum( name );
	if(mapnum==0) return ERR_9005;

	pEntry = &vc_entity;

	if( getATMVCEntry(name, pEntry, &chainidx) < 0 )
		return ERR_9002;

	pPort = &port_entity;
	if( getPortMappingByInstNum( 0, mapnum, pPort, &port_chainid ) )
		return ERR_9002;

	memcpy( &OldPort_Entity, pPort, sizeof(MIB_VIRTUAL_SVR_T) );


	if( strcmp( lastname, "PortMappingEnabled" )==0 )
	{
		return CWMP_APPLIED;
	}else if( strcmp( lastname, "PortMappingLeaseDuration" )==0 )
	{
		unsigned int *i = data;

		//only support value 0
		if( *i!=0 ) return ERR_9001;
		return CWMP_APPLIED;
	}else if( strcmp( lastname, "RemoteHost" )==0 )
	{
		return CWMP_APPLIED;

	}else if( strcmp( lastname, "ExternalPort" )==0 )
	{
		unsigned int *i = data;

		if(i==NULL) return ERR_9007;
		if( *i> 65535) return ERR_9007;
//		pPort->externalport= *i;
		pPort->wanEndPort= *i;
		pPort->wanStartPort = *i;
		mib_chain_update( MIB_VIRTUAL_SVR_TBL, (unsigned char*)pPort, port_chainid );

		apply_add( CWMP_PRI_N, apply_PortForwarding, CWMP_RESTART, port_chainid, &OldPort_Entity, sizeof(MIB_VIRTUAL_SVR_T) );
		return CWMP_APPLIED;
	}else if( strcmp( lastname, "InternalPort" )==0 )
	{
		unsigned int *i = data;

		if(i==NULL) return ERR_9007;
		if( *i> 65535) return ERR_9007;
		pPort->lanPort= *i;
		mib_chain_update( MIB_VIRTUAL_SVR_TBL, (unsigned char*)pPort, port_chainid );

		apply_add( CWMP_PRI_N, apply_PortForwarding, CWMP_RESTART, port_chainid, &OldPort_Entity, sizeof(MIB_VIRTUAL_SVR_T) );
		return CWMP_APPLIED;
	}else if( strcmp( lastname, "PortMappingProtocol" )==0 )
	{
		if( strlen(buf)==0 ) return ERR_9007;
		if( strcmp( buf, "TCP" )==0 )
			pPort->protoType = PROTO_TCP;
		else if( strcmp( buf, "UDP" )==0 )
			pPort->protoType = PROTO_UDP;
		else if ( strcmp( buf, "TCPandUDP" )==0 )
			pPort->protoType = 0;
		else
			return ERR_9007;
		mib_chain_update( MIB_VIRTUAL_SVR_TBL, (unsigned char*)pPort, port_chainid );

		apply_add( CWMP_PRI_N, apply_PortForwarding, CWMP_RESTART, port_chainid, &OldPort_Entity, sizeof(MIB_VIRTUAL_SVR_T) );
		return CWMP_APPLIED;
	}else if( strcmp( lastname, "InternalClient" )==0 )
	{
		if( strlen(buf)==0 ) return ERR_9007; //can't be empty

		if ( !inet_aton(buf, (struct in_addr *)&(pPort->serverIp)) )
			return ERR_9007;
		mib_chain_update( MIB_VIRTUAL_SVR_TBL, (unsigned char*)pPort, port_chainid );

		apply_add( CWMP_PRI_N, apply_PortForwarding, CWMP_RESTART, port_chainid, &OldPort_Entity, sizeof(MIB_VIRTUAL_SVR_T) );
		return CWMP_APPLIED;
	}else if( strcmp( lastname, "PortMappingDescription" )==0 )
	{
		strncpy( pPort->svrName, buf, 60-1 );
		pPort->svrName[60-1]=0;
		mib_chain_update( MIB_VIRTUAL_SVR_TBL, (unsigned char*)pPort, port_chainid );
		return CWMP_APPLIED;
	}else{
		return ERR_9005;
	}

	return CWMP_APPLIED;
}

int objWANPORTMAPPING(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	unsigned int chainidx;
	MIB_CE_ATM_VC_T	*pEntry, vc_entity;

	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);fflush(NULL);
	if( (name==NULL) || (entity==NULL) ) return -1;

	pEntry = &vc_entity;

	if( getATMVCEntry(name, pEntry, &chainidx) < 0 )
		return ERR_9002;

	switch( type )
	{
	case eCWMP_tINITOBJ:
	{
		int num=0,i;
		struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;
		MIB_VIRTUAL_SVR_T *p, port_entity;
		unsigned int port_chainID=0, MaxInstNum=0;

		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		MaxInstNum = getPortMappingMaxInstNum( 0);
		num = getPortMappingCount( 0 );
		for( i=0; i<num;i++ )
		{
			p = &port_entity;
			if( getPortMappingByID(0, i, p, &port_chainID )<0 )
				continue;

			if( p->InstanceNum==0 ) //maybe createn by web or cli
			{
				MaxInstNum++;
				p->InstanceNum = MaxInstNum;
				mib_chain_update( MIB_VIRTUAL_SVR_TBL, (unsigned char*)p, port_chainID );
			}

			if( create_Object( c, tWANPORTMAPObject, sizeof(tWANPORTMAPObject), 1, p->InstanceNum ) < 0 )
				return -1;
		}
		add_objectNum( name, MaxInstNum );
		return CWMP_APPLIED;
	     }
	case eCWMP_tADDOBJ:
	     {
	     	int ret;

	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tWANPORTMAPObject, sizeof(tWANPORTMAPObject), data );
		if( ret >= 0 )
		{
			MIB_VIRTUAL_SVR_T entry;
		     	char vCh=0;

			memset( &entry, 0, sizeof( MIB_VIRTUAL_SVR_T ) );
			{ //default values for this new entry
				//entry.ifIndex=pEntry->ifIndex;
				entry.InstanceNum= *(int*)data;
				entry.protoType = PROTO_TCP;
			}
			mib_chain_add( MIB_VIRTUAL_SVR_TBL, (unsigned char*)&entry );

			mib_get(MIB_PORT_FW_ENABLE, (void *)&vCh);
			if( vCh!=1 )
			{
				vCh=1;
				mib_set( MIB_PORT_FW_ENABLE, (void *)&vCh);
			}
		}
		return ret;
	     }
	case eCWMP_tDELOBJ:
	     {
	     	int ret;
		MIB_VIRTUAL_SVR_T *p, port_entity;
		unsigned int port_chainID=0;

	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		p = &port_entity;
		if( getPortMappingByInstNum( 0, *(unsigned int*)data , p, &port_chainID )<0 )
			return ERR_9005;
     		mib_chain_delete( MIB_VIRTUAL_SVR_TBL, port_chainID );
		apply_PortForwarding( CWMP_STOP, port_chainID, p );

		ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next, *(int*)data );
		return ret;
	     	break;
	     }

	case eCWMP_tUPDATEOBJ:
	     {
	     	int num=0,i;
	     	struct CWMP_LINKNODE *old_table;
	     	unsigned int port_chainID=0;

	     	//CWMPDBG( 0, ( stderr, "<%s:%d>action=eCWMP_tUPDATEOBJ(name=%s)\n", __FUNCTION__, __LINE__, name ) );
	     	num = getPortMappingCount( 0);
	     	old_table = (struct CWMP_LINKNODE *)entity->next;
	     	entity->next = NULL;
	     	for( i=0; i<num;i++ )
	     	{
	     		struct CWMP_LINKNODE *remove_entity=NULL;
			MIB_VIRTUAL_SVR_T *p, port_entity;

			p = &port_entity;
			if( getPortMappingByID( 0, i, p, &port_chainID )<0 )
				continue;

			remove_entity = remove_SiblingEntity( &old_table, p->InstanceNum );
			if( remove_entity!=NULL )
			{
				add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
			}else{
				unsigned int MaxInstNum=p->InstanceNum;

				add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tWANPORTMAPObject, sizeof(tWANPORTMAPObject), &MaxInstNum );
				if(MaxInstNum!=p->InstanceNum)
				{
					p->InstanceNum = MaxInstNum;
					mib_chain_update( MIB_VIRTUAL_SVR_TBL, (unsigned char*)p, port_chainID );
				}
			}
	     	}

	     	if( old_table )
	     		destroy_ParameterTable( (struct CWMP_NODE *)old_table );
	     	return 0;
	     }
	}

	return -1;
}

#elif defined PORT_FORWARD_GENERAL
/*InternetGatewayDevice.WANDevice.{i}.WANConnectionDevice.{i}.WANIP/PPPConnection.PortMapping*/
int getPORMAPTENTITY(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned int 	mapnum, chainidx;
	unsigned int	port_chainid=0;
	MIB_CE_ATM_VC_T	*pEntry, vc_entity;
	MIB_CE_PORT_FW_T *pPort, port_entity;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	mapnum  = getWANPortMapInstNum( name );
	if(mapnum==0) return ERR_9005;

	pEntry = &vc_entity;
	if( getATMVCEntry(name, pEntry, &chainidx) < 0 )
		return ERR_9002;

	pPort = &port_entity;
	if( getPortMappingByInstNum( pEntry->ifIndex, mapnum, pPort, &port_chainid ) )
		return ERR_9002;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "PortMappingEnabled" )==0 )
	{
		if( pPort->enable==0 )
			*data = booldup(0);
		else
			*data = booldup(1);
	}else if( strcmp( lastname, "PortMappingLeaseDuration" )==0 )
	{
		*data = uintdup( pPort->leaseduration );
	}else if( strcmp( lastname, "RemoteHost" )==0 )
	{
		char *tmp;
		tmp = inet_ntoa(*((struct in_addr *)pPort->remotehost));
		if(tmp)
			*data = strdup( tmp );
		else
			*data = strdup( "" );
	}else if( strcmp( lastname, "ExternalPort" )==0 )
	{
//		*data=uintdup( pPort->externalport );
		*data=uintdup( pPort->externaltoport );
	}else if( strcmp( lastname, "InternalPort" )==0 )
	{
		*data=uintdup( pPort->toPort );
	}else if( strcmp( lastname, "PortMappingProtocol" )==0 )
	{
		if( pPort->protoType==PROTO_TCP )
			*data = strdup( "TCP" );
		else if( pPort->protoType==PROTO_UDP )
			*data = strdup( "UDP" );
		else if (pPort->protoType==PROTO_UDPTCP )
			*data = strdup("TCPandUDP");
		else /*PROTO_NONE or PROTO_ICMP*/
			*data = strdup( "" );
	}else if( strcmp( lastname, "InternalClient" )==0 )
	{
		char *tmp;
		tmp = inet_ntoa(*((struct in_addr *)pPort->ipAddr));
		if(tmp)
			*data = strdup( tmp );
		else
			*data = strdup( "" );
	}else if( strcmp( lastname, "PortMappingDescription" )==0 )
	{
		*data = strdup( (char*)pPort->comment );
	}else{
		return ERR_9005;
	}

	return 0;
}

int setPORMAPTENTITY(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	unsigned int 	mapnum, chainidx;
	unsigned int	port_chainid=0;
	char		*buf=data;
	MIB_CE_ATM_VC_T	*pEntry, vc_entity;
	MIB_CE_PORT_FW_T *pPort, port_entity, OldPort_Entity;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( data==NULL ) return ERR_9007;
	if( entity->info->type!=type ) return ERR_9006;

	mapnum  = getWANPortMapInstNum( name );
	if(mapnum==0) return ERR_9005;

	pEntry = &vc_entity;
	if( getATMVCEntry(name, pEntry, &chainidx) < 0 )
		return ERR_9002;

	pPort = &port_entity;
	if( getPortMappingByInstNum( pEntry->ifIndex, mapnum, pPort, &port_chainid ) )
		return ERR_9002;

	memcpy( &OldPort_Entity, pPort, sizeof(MIB_CE_PORT_FW_T) );


	if( strcmp( lastname, "PortMappingEnabled" )==0 )
	{
		int *i = data;

		pPort->enable = (*i==0)?0:1;
		mib_chain_update( MIB_PORT_FW_TBL, (unsigned char*)pPort, port_chainid );

		apply_add( CWMP_PRI_N, apply_PortForwarding, CWMP_RESTART, port_chainid, &OldPort_Entity, sizeof(MIB_CE_PORT_FW_T) );
		return CWMP_APPLIED;
	}else if( strcmp( lastname, "PortMappingLeaseDuration" )==0 )
	{
		unsigned int *i = data;

		//only support value 0
		if( *i!=0 ) return ERR_9001;
		return CWMP_APPLIED;
	}else if( strcmp( lastname, "RemoteHost" )==0 )
	{
		char *zeroip="0.0.0.0";
		if( strlen(buf)==0 )
			buf = zeroip;
		if ( !inet_aton(buf, (struct in_addr *)&pPort->remotehost) )
			return ERR_9007;
		mib_chain_update( MIB_PORT_FW_TBL, (unsigned char*)pPort, port_chainid );

		apply_add( CWMP_PRI_N, apply_PortForwarding, CWMP_RESTART, port_chainid, &OldPort_Entity, sizeof(MIB_CE_PORT_FW_T) );
		return CWMP_APPLIED;
	}else if( strcmp( lastname, "ExternalPort" )==0 )
	{
		unsigned int *i = data;

		if(i==NULL) return ERR_9007;
		if( *i> 65535) return ERR_9007;
//		pPort->externalport= *i;
		pPort->externalfromport= *i;
		pPort->externaltoport = *i;
		mib_chain_update( MIB_PORT_FW_TBL, (unsigned char*)pPort, port_chainid );

		apply_add( CWMP_PRI_N, apply_PortForwarding, CWMP_RESTART, port_chainid, &OldPort_Entity, sizeof(MIB_CE_PORT_FW_T) );
		return CWMP_APPLIED;
	}else if( strcmp( lastname, "InternalPort" )==0 )
	{
		unsigned int *i = data;

		if(i==NULL) return ERR_9007;
		if( *i> 65535) return ERR_9007;
		pPort->toPort= *i;
		pPort->fromPort = *i;
		mib_chain_update( MIB_PORT_FW_TBL, (unsigned char*)pPort, port_chainid );

		apply_add( CWMP_PRI_N, apply_PortForwarding, CWMP_RESTART, port_chainid, &OldPort_Entity, sizeof(MIB_CE_PORT_FW_T) );
		return CWMP_APPLIED;
	}else if( strcmp( lastname, "PortMappingProtocol" )==0 )
	{
		if( strlen(buf)==0 ) return ERR_9007;
		if( strcmp( buf, "TCP" )==0 )
			pPort->protoType = PROTO_TCP;
		else if( strcmp( buf, "UDP" )==0 )
			pPort->protoType = PROTO_UDP;
		else if ( strcmp( buf, "TCPandUDP" )==0 )
			pPort->protoType = PROTO_UDPTCP;
		else
			return ERR_9007;
		mib_chain_update( MIB_PORT_FW_TBL, (unsigned char*)pPort, port_chainid );

		apply_add( CWMP_PRI_N, apply_PortForwarding, CWMP_RESTART, port_chainid, &OldPort_Entity, sizeof(MIB_CE_PORT_FW_T) );
		return CWMP_APPLIED;
	}else if( strcmp( lastname, "InternalClient" )==0 )
	{
		if( strlen(buf)==0 ) return ERR_9007; //can't be empty

		if ( !inet_aton(buf, (struct in_addr *)&pPort->ipAddr) )
			return ERR_9007;
		mib_chain_update( MIB_PORT_FW_TBL, (unsigned char*)pPort, port_chainid );

		apply_add( CWMP_PRI_N, apply_PortForwarding, CWMP_RESTART, port_chainid, &OldPort_Entity, sizeof(MIB_CE_PORT_FW_T) );
		return CWMP_APPLIED;
	}else if( strcmp( lastname, "PortMappingDescription" )==0 )
	{
		strncpy( (char*)pPort->comment, buf, COMMENT_LEN-1 );
		pPort->comment[COMMENT_LEN-1]=0;
		mib_chain_update( MIB_PORT_FW_TBL, (unsigned char*)pPort, port_chainid );
		return CWMP_APPLIED;
	}else{
		return ERR_9005;
	}

	return 0;
}

int objWANPORTMAPPING(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	unsigned int chainidx;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;

	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);fflush(NULL);
	if( (name==NULL) || (entity==NULL) ) return -1;

	pEntry = &vc_entity;
	if( getATMVCEntry(name, pEntry, &chainidx) < 0 )
		return ERR_9002;

	switch( type )
	{
	case eCWMP_tINITOBJ:
	{
		int num=0,i;
		struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;
		MIB_CE_PORT_FW_T *p, port_entity;
		unsigned int port_chainID=0, MaxInstNum=0;

		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		MaxInstNum = getPortMappingMaxInstNum( pEntry->ifIndex );
		num = getPortMappingCount( pEntry->ifIndex );
		for( i=0; i<num;i++ )
		{
			p = &port_entity;
			if( getPortMappingByID( pEntry->ifIndex, i, p, &port_chainID )<0 )
				continue;

			if( p->InstanceNum==0 ) //maybe createn by web or cli
			{
				MaxInstNum++;
				p->InstanceNum = MaxInstNum;
				mib_chain_update( MIB_PORT_FW_TBL, (unsigned char*)p, port_chainID );
			}

			if( create_Object( c, tWANPORTMAPObject, sizeof(tWANPORTMAPObject), 1, p->InstanceNum ) < 0 )
				return -1;
		}
		add_objectNum( name, MaxInstNum );
		return 0;
	     }
	case eCWMP_tADDOBJ:
	     {
	     	int ret;

	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tWANPORTMAPObject, sizeof(tWANPORTMAPObject), data );
		if( ret >= 0 )
		{
			MIB_CE_PORT_FW_T entry;
		     	char vCh=0;

			memset( &entry, 0, sizeof( MIB_CE_PORT_FW_T ) );
			{ //default values for this new entry
				entry.ifIndex=pEntry->ifIndex;
				entry.InstanceNum= *(int*)data;
				entry.protoType = PROTO_TCP;
			}
			mib_chain_add( MIB_PORT_FW_TBL, (unsigned char*)&entry );

			mib_get(MIB_PORT_FW_ENABLE, (void *)&vCh);
			if( vCh!=1 )
			{
				vCh=1;
				mib_set( MIB_PORT_FW_ENABLE, (void *)&vCh);
			}
		}
		return ret;
	     }
	case eCWMP_tDELOBJ:
	     {
	     	int ret;
		MIB_CE_PORT_FW_T *p, port_entity;
		unsigned int port_chainID=0;

	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		p = &port_entity;
		if( getPortMappingByInstNum( pEntry->ifIndex, *(unsigned int*)data , p, &port_chainID )<0 )
			return ERR_9005;
		/* created by UPnP */
		if (p->dynamic)
			return ERR_9001;

		apply_PortForwarding( CWMP_STOP, port_chainID, p );

		mib_chain_delete( MIB_PORT_FW_TBL, port_chainID );
		ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next, *(int*)data );
		return ret;
	     	break;
	     }

	case eCWMP_tUPDATEOBJ:
	     {
	     	int num=0,i;
	     	struct CWMP_LINKNODE *old_table;
	     	unsigned int port_chainID=0;

	     	//CWMPDBG( 0, ( stderr, "<%s:%d>action=eCWMP_tUPDATEOBJ(name=%s)\n", __FUNCTION__, __LINE__, name ) );
	     	num = getPortMappingCount( pEntry->ifIndex );
	     	old_table = (struct CWMP_LINKNODE *)entity->next;
	     	entity->next = NULL;
	     	for( i=0; i<num;i++ )
	     	{
	     		struct CWMP_LINKNODE *remove_entity=NULL;
			MIB_CE_PORT_FW_T *p, port_entity;

			p = &port_entity;
			if( getPortMappingByID( pEntry->ifIndex, i, p, &port_chainID )<0 )
				continue;

			remove_entity = remove_SiblingEntity( &old_table, p->InstanceNum );
			if( remove_entity!=NULL )
			{
				add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
			}else{
				unsigned int MaxInstNum=p->InstanceNum;

				add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tWANPORTMAPObject, sizeof(tWANPORTMAPObject), &MaxInstNum );
				if(MaxInstNum!=p->InstanceNum)
				{
					p->InstanceNum = MaxInstNum;
					mib_chain_update( MIB_PORT_FW_TBL, (unsigned char*)p, port_chainID );
				}
			}
	     	}

	     	if( old_table )
	     		destroy_ParameterTable( (struct CWMP_NODE *)old_table );
	     	return 0;
	     }
	}

	return -1;
}
#endif

/*InternetGatewayDevice.WANDevice.{i}.WANConnectionDevice.{i}.WANIP/PPPConnection.Stats*/
int getWANCONSTATS(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned int	chainidx;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;
	char	ifname[16];
	//char	buf[256];
	struct net_device_stats nds;
	int		iserror=0;
	struct ethtool_stats *stats = NULL;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	pEntry = &vc_entity;
	if( getATMVCEntry(name, pEntry, &chainidx) < 0 )
		return ERR_9002;
	if( ifGetName(pEntry->ifIndex, ifname, sizeof(ifname)) )
	{
		if(get_net_device_stats(ifname, &nds) < 0)
			iserror=1;
		stats = ethtool_gstats(ifname);
	}else{
		iserror=1;
	}
	if(iserror) memset( &nds, 0, sizeof(nds) );


	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "EthernetBytesSent" )==0 )
	{
		*data = uintdup(stats ? stats->data[TX_OCTETS] : nds.tx_bytes);
	}else if( strcmp( lastname, "EthernetBytesReceived" )==0 )
	{
		*data = uintdup(stats ? stats->data[RX_OCTETS] : nds.rx_bytes);
	}else if( strcmp( lastname, "EthernetPacketsSent" )==0 )
	{
		*data = uintdup(stats ? stats->data[TX_UCAST_PACKETS]
				+ stats->data[TX_MCAST_PACKETS]
				+ stats->data[TX_BCAST_PACKETS]
				: nds.tx_packets);
	}else if( strcmp( lastname, "EthernetPacketsReceived" )==0 )
	{
		*data = uintdup(stats ? stats->data[RX_UCAST_PACKETS]
				+ stats->data[RX_MCAST_PACKETS]
				+ stats->data[RX_BCAST_PACKETS]
				: nds.rx_packets);

/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	}else if( strcmp( lastname, "EthernetErrorsSent" )==0 )
	{
		*data = uintdup(nds.tx_errors);
	}else if( strcmp( lastname, "EthernetErrorsReceived" )==0 )
	{
		*data = uintdup(nds.rx_errors);
	}else if( strcmp( lastname, "EthernetUnicastPacketsSent" )==0 )
	{
		*data = uintdup(stats ? stats->data[TX_UCAST_PACKETS] : 0);
	}else if( strcmp( lastname, "EthernetUnicastPacketsReceived" )==0 )
	{
		*data = uintdup(stats ? stats->data[RX_UCAST_PACKETS] : 0);
	}else if( strcmp( lastname, "EthernetDiscardPacketsSent" )==0 )
	{
		*data = uintdup(stats ? stats->data[TX_DISCARDS] : nds.tx_dropped);
	}else if( strcmp( lastname, "EthernetDiscardPacketsReceived" )==0 )
	{
		*data = uintdup(stats ? stats->data[RX_DISCARDS] : nds.rx_dropped);
	}else if( strcmp( lastname, "EthernetMulticastPacketsSent" )==0 )
	{
		*data = uintdup(stats ? stats->data[TX_MCAST_PACKETS] : 0);
	}else if( strcmp( lastname, "EthernetMulticastPacketsReceived" )==0 )
	{
		*data = uintdup(stats ? stats->data[RX_MCAST_PACKETS] : nds.multicast);
	}else if( strcmp( lastname, "EthernetBroadcastPacketsSent" )==0 )
	{
		*data = uintdup(stats ? stats->data[TX_BCAST_PACKETS] : 0);
	}else if( strcmp( lastname, "EthernetBroadcastPacketsReceived" )==0 )
	{
		*data = uintdup(stats ? stats->data[RX_BCAST_PACKETS] : 0);
	}else if( strcmp( lastname, "EthernetUnknownProtoPacketsReceived" )==0 )
	{
		*data = uintdup( 0 );
#endif
/*ping_zhang:20081217 END*/
	}else{
		free(stats);
		return ERR_9005;
	}
	free(stats);

	return 0;
}

static void get_dns_by_wan(char *buf, int len, MIB_CE_ATM_VC_T *pEntry)
{
	char dns1[32] = {0}, dns2[32] = {0};
	unsigned char zero[IP_ADDR_LEN] = {0};

	if ( ((pEntry->cmode == CHANNEL_MODE_RT1483) || (pEntry->cmode == CHANNEL_MODE_IPOE)) && (pEntry->dnsMode == REQUEST_DNS_NONE) )
	{
		//manual
		if(memcmp(zero, pEntry->v4dns1, IP_ADDR_LEN) != 0)
		{
			strncpy(dns1, inet_ntoa(*((struct in_addr *)pEntry->v4dns1)), 32);
			strncpy(buf, dns1, len);
		}
		if(memcmp(zero, pEntry->v4dns2, IP_ADDR_LEN) != 0)
		{
			strncpy(dns2, inet_ntoa(*((struct in_addr *)pEntry->v4dns2)), 32);
			if(strlen(dns1) != 0)
				strncat(buf, ",", len);
			strncat(buf, dns2, len);
		}
	}
	else
	{
		FILE *infdns = NULL;
		char line[128] = {0};
		char ifname[IFNAMSIZ] = {0};

		ifGetName(pEntry->ifIndex,ifname,sizeof(ifname));
		if ((DHCP_T)pEntry->ipDhcp == DHCP_CLIENT)
			snprintf(line, 64, "%s.%s", (char *)DNS_RESOLV, ifname);
		if (pEntry->cmode == CHANNEL_MODE_PPPOE || pEntry->cmode == CHANNEL_MODE_PPPOA)
			snprintf(line, 64, "%s.%s", (char *)PPP_RESOLV, ifname);

		infdns=fopen(line,"r");
		if(infdns)
		{
			char *p = NULL;
			int cnt = 0;

			while(fgets(line,sizeof(line),infdns))
			{
				if((strlen(line)==0))
					continue;

				p = strchr(line, '@');
				//strncpy(dns1, line, p - line);
				if (p) {
					memcpy(dns1, line, p-line);
					dns1[p-line] = '\0';
				}
				if(cnt != 0)
					strncat(buf, ",", len);
				strncat(buf, dns1, len);
				cnt++;
			}
			fclose(infdns);
		}
	}
}

static int set_dns_by_wan(char *buf, MIB_CE_ATM_VC_T * pEntry)
{
	int i;
	size_t len;
	char *tmpbuf, *token;

	if (((pEntry->cmode == CHANNEL_MODE_RT1483) || (pEntry->cmode == CHANNEL_MODE_IPOE))
	    && (pEntry->dnsMode == REQUEST_DNS_NONE)) {
		/* prevent to use buf as the input to strtok() */
		len = strlen(buf);
		tmpbuf = malloc(len + 1);
		if (tmpbuf == NULL)
			goto ret;
		strcpy(tmpbuf, buf);

		memset(pEntry->v4dns1, 0, sizeof(pEntry->v4dns1));
		memset(pEntry->v4dns2, 0, sizeof(pEntry->v4dns2));
		i = 0;
		token = strtok(tmpbuf, " ,");
		while (token) {
			i++;
			if (i == 1)
				inet_aton(token, (struct in_addr *)pEntry->v4dns1);
			else if (i == 2)
				inet_aton(token, (struct in_addr *)pEntry->v4dns2);
			else
				break;

			token = strtok(NULL, " ,");
		}

		free(tmpbuf);

		return 0;
	}

ret:
	return 1;
}


/*InternetGatewayDevice.WANDevice.{i}.WANConnectionDevice.{i}.WANPPPConnection.*/
int getWANPPPCONENTITY(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	//char	*tok;
	unsigned int	chainidx;
	MIB_CE_ATM_VC_T *pEntry = NULL, vc_entity = {0};
	char	ifname[16] = {0};
	char	buf[512]="";
	unsigned char vChar=0;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	pEntry = &vc_entity;
	if( getATMVCEntry(name, pEntry, &chainidx) < 0 )
		return ERR_9002;

	if(ifGetName( pEntry->ifIndex, ifname, 16 )==0) return ERR_9002;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		if(pEntry->enable)
			*data = booldup(1);
		else
			*data = booldup(0);
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	}else if( strcmp( lastname, "Reset" )==0 )
	{
		/*while read, this parameter always returns False.*/
		*data = booldup(0);
#endif
/*ping_zhang:20081217 END*/
	}else if( strcmp( lastname, "ConnectionStatus" )==0 )
	{
		char pppstatus[32]={0};
#ifdef PPPOE_PASSTHROUGH
		int flags;

		if(pEntry->cmode==CHANNEL_MODE_BRIDGE)
		{
			if(pEntry->enable==0)
				*data = strdup( "Disconnected" );
			else if(getInFlags( ifname, &flags) == 1)
			{
				if (flags & IFF_RUNNING)
					*data = strdup( "Connected" );
				else
					*data = strdup( "Disconnected" );
			}else
				*data = strdup( "Disconnected" );
		}else
#endif
#ifdef CONFIG_PPP
		if( getPPPConStatus( ifname, pppstatus )==0 )
			*data = strdup( pppstatus );
		else
#endif
			*data = strdup( "Disconnected" );
	}else if( strcmp( lastname, "PossibleConnectionTypes" )==0 )
	{
#ifdef PPPOE_PASSTHROUGH
		*data = strdup( "IP_Routed,PPPoE_Bridged" );
#else
		*data = strdup( "IP_Routed" );
#endif
	}else if( strcmp( lastname, "ConnectionType" )==0 )
	{
#ifdef PPPOE_PASSTHROUGH
		if(pEntry->cmode==CHANNEL_MODE_BRIDGE)
			*data = strdup( "PPPoE_Bridged" );
		else
#endif
			*data = strdup( "IP_Routed" );
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	}else if( strcmp( lastname, "PPPoESessionID" )==0 )
	{
		unsigned char totalEntry;
		MIB_CE_PPPOE_SESSION_T Entry;
		unsigned int i,sessionID=0;//,found=0;

		totalEntry = mib_chain_total(MIB_PPPOE_SESSION_TBL); /* get chain record size */
		for (i=0; i<totalEntry; i++) {
			if (!mib_chain_get(MIB_PPPOE_SESSION_TBL, i, (void *)&Entry)) {
				return ERR_9002;
			}
			if (Entry.uifno == pEntry->ifIndex)
			{
				sessionID = Entry.sessionId;
				break;
			}
		}
		if(sessionID < 1)
			*data = intdup(0);
		else
			*data = intdup(sessionID);
	}else if( strcmp( lastname, "DefaultGateway" )==0 )
	{
		char *temp=NULL;
		struct in_addr inAddr;

		if (getInAddr( ifname, DST_IP_ADDR, (void *)&inAddr) == 1)
			temp = inet_ntoa(inAddr);
		if(temp)
			*data=strdup(temp);
		else
			*data=strdup("");
#endif
/*ping_zhang:20081217 END*/
	}else if( strcmp( lastname, "Name" )==0 )
	{
		if(*(pEntry->WanName))
			*data = strdup( (char*)pEntry->WanName );
#ifdef CTC_WAN_NAME
		else
		{//if not set by ACS. then generate automaticly.
			char wanname[40];
			memset(wanname, 0, sizeof(wanname));
			generateWanName(pEntry, wanname);
			*data = strdup( wanname );
		}
#endif
	}else if( strcmp( lastname, "Uptime" )==0 )
	{
#ifdef CONFIG_PPP
		unsigned int uptime;
#endif
#ifdef PPPOE_PASSTHROUGH
		if(pEntry->cmode==CHANNEL_MODE_BRIDGE)
		{
			struct sysinfo info;
			sysinfo(&info);
			*data = uintdup( info.uptime );
			return 0;
		}
#endif
#ifdef CONFIG_PPP
		if( getPPPUptime( ifname, pEntry->cmode, &uptime )==0 )
			*data = uintdup( uptime );
		else
#endif
			*data = uintdup( 0 );
	}else if( strcmp( lastname, "LastConnectionError" )==0 )
	{
#ifdef PPPOE_PASSTHROUGH
		if(pEntry->cmode==CHANNEL_MODE_BRIDGE)
			*data = strdup( "ERROR_NONE" );
		else
#endif
			*data = strdup(getLastConnectionError(pEntry->ifIndex));	// Jenny
	}else if( strcmp( lastname, "AutoDisconnectTime" )==0 )
	{
//		*data = uintdup( 0 ); //only 0:the connection is not to be shut down automatically
		*data = uintdup( pEntry->autoDisTime );		// Jenny
	}else if( strcmp( lastname, "IdleDisconnectTime" )==0 )
	{
		*data = uintdup( pEntry->pppIdleTime );
	}else if( strcmp( lastname, "WarnDisconnectDelay" )==0 )	// Jenny
	{
		*data = uintdup( pEntry->warnDisDelay );
	}else if( strcmp( lastname, "RSIPAvailable" )==0 )
	{
		*data = booldup( 0 );
	}else if( strcmp( lastname, "NATEnabled" )==0 )
	{
		if(pEntry->napt)
			*data = booldup( 1 );
		else
			*data = booldup( 0 );
	}else if( strcmp( lastname, "Username" )==0 )
	{
		*data = strdup( (char*)pEntry->pppUsername );
	}else if( strcmp( lastname, "Password" )==0 )
	{
#if 1
#ifdef CONFIG_MIDDLEWARE
		mib_get(CWMP_TR069_ENABLE,(void *)&vChar);
		if(vChar == 0){
			*data = strdup( pEntry->pppPassword );
		}else
#endif	//end of CONFIG_MIDDLEWARE
		*data = strdup( "" ); /*return an empty string*/
#else
		*data = strdup( pEntry->pppPassword );
#endif
	}else if( strcmp( lastname, "PPPEncryptionProtocol" )==0 )	// Jenny
	{
		*data = strdup( "None" );
	}else if( strcmp( lastname, "PPPCompressionProtocol" )==0 )	// Jenny
	{
		*data = strdup( "None" );
	}else if( strcmp( lastname, "PPPAuthenticationProtocol" )==0 )
	{
		if(pEntry->pppAuth==PPP_AUTH_PAP)
			*data = strdup( "PAP" );
		else if(pEntry->pppAuth==PPP_AUTH_CHAP)
			*data = strdup( "CHAP" );
		else if(pEntry->pppAuth==PPP_AUTH_AUTO)
			*data = strdup( "PAPandCHAP" );
		else
			return ERR_9002;
	}else if( strcmp( lastname, "ExternalIPAddress" )==0 )
	{
		char *temp=NULL;
		struct in_addr inAddr;

		if (getInAddr( ifname, IP_ADDR, (void *)&inAddr) == 1)
			temp = inet_ntoa(inAddr);
		if(temp)
			*data=strdup(temp);
		else
			*data=strdup("");
	}else if( strcmp( lastname, "RemoteIPAddress" )==0 )
	{
		char *temp=NULL;
		struct in_addr inAddr;

		if (getInAddr( ifname, DST_IP_ADDR, (void *)&inAddr) == 1)
			temp = inet_ntoa(inAddr);
		if(temp)
			*data=strdup(temp);
		else
			*data=strdup("");
	}else if( strcmp( lastname, "MaxMRUSize" )==0 )	// Jenny
	{
		*data = uintdup( pEntry->mtu );
	}else if( strcmp( lastname, "CurrentMRUSize" )==0 )	// Jenny
	{
#ifdef CONFIG_PPP
		unsigned int cmru;
		if( getPPPCurrentMRU( ifname, pEntry->cmode, &cmru )==0 )
			*data = uintdup( cmru );
		else
#endif
			*data = uintdup( pEntry->mtu );
	}else if( strcmp( lastname, "DNSEnabled" )==0 )
	{
		*data = booldup( 1 );
	}else if( strcmp( lastname, "DNSOverrideAllowed" )==0 )
	{
		if ( ((pEntry->cmode == CHANNEL_MODE_RT1483) || (pEntry->cmode == CHANNEL_MODE_IPOE)) && (pEntry->dnsMode == REQUEST_DNS_NONE) )
			*data = booldup(1);
		else
			*data = booldup(0);
	}else if( strcmp( lastname, "DNSServers" )==0 )
	{
		get_dns_by_wan(buf, sizeof(buf), pEntry);
		*data = strdup( buf );
	}else if( strcmp( lastname, "MACAddress" )==0 )
	{
		if( pEntry->cmode==CHANNEL_MODE_PPPOA )
		{
			*data = strdup( "" );
		}
		else
		{
			//struct in_addr inAddr;
			char wanif[IFNAMSIZ];
			char macadd[MAC_ADDR_LEN] = {0};
			struct sockaddr sa;

			ifGetName(PHY_INTF(pEntry->ifIndex), wanif, sizeof(wanif));

			if(!getInAddr(wanif, HW_ADDR, (void *)&sa))
			{
				//if this entry is disabled, getInAddr() will return failed
				//return ERR_9002;
				*data = strdup("");
			}else{
				memcpy( macadd, sa.sa_data, MAC_ADDR_LEN );
				sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", macadd[0]&0xff, macadd[1]&0xff, macadd[2]&0xff, macadd[3]&0xff, macadd[4]&0xff, macadd[5]&0xff);
				*data = strdup(buf);
			}
		}
	}else if( strcmp( lastname, "TransportType" )==0 )
	{
		if( pEntry->cmode == CHANNEL_MODE_PPPOA )
			*data = strdup("PPPoA");
		else
			*data = strdup("PPPoE");
	}else if( strcmp( lastname, "PPPoEACName" )==0 )
	{
		if( pEntry->cmode==CHANNEL_MODE_PPPOE )
			*data = strdup( (char*)pEntry->pppACName );
		else
			*data = strdup( "" );
	}else if( strcmp( lastname, "PPPoEServiceName" )==0 )
	{
//		*data = strdup( "" );
		if( pEntry->cmode==CHANNEL_MODE_PPPOE )	// Jenny
			*data = strdup( (char*)pEntry->pppServiceName );
		else
			*data = strdup( "" );
	}else if( strcmp( lastname, "ConnectionTrigger" )==0 )
	{
#ifdef PPPOE_PASSTHROUGH
		if( pEntry->cmode==CHANNEL_MODE_BRIDGE )
			*data = strdup( "AlwaysOn" );
		else
#endif
		if( pEntry->pppCtype==CONTINUOUS )
			*data = strdup( "AlwaysOn" );
		else if( pEntry->pppCtype==CONNECT_ON_DEMAND )
			*data = strdup( "OnDemand" );
		else
			*data = strdup( "Manual" );
	}else if( strcmp( lastname, "RouteProtocolRx" )==0 )
	{
		unsigned char ripVer=RIP_NONE;
		getRIPInfo( pEntry->ifIndex, &ripVer );
		switch( ripVer )
		{
		case RIP_NONE:
			*data=strdup( "Off" );
			break;
		case RIP_V1:
			*data=strdup( "RIPv1" );
			break;
		case RIP_V2:
			*data=strdup( "RIPv2" );
			break;
		case RIP_V1_V2:
			*data=strdup( "RIPv1andRIPv2" );
			break;
		default:
			return ERR_9002;
		}
	}
#ifdef _PRMT_X_RTK_
	else if (strcmp(lastname, "X_RTK_IgmpProxy") == 0)
	{
		if (pEntry->enableIGMP == 1)
			*data = booldup(1);
		else
			*data = booldup(0);
	}
	else if (strcmp(lastname, "X_RTK_ServiceType") == 0)
	{
		// 1 bit: Internet, 2 bit: TR-069, 3 bit: VoIP
#ifdef CONFIG_USER_RTK_WAN_CTYPE
		unsigned int ServiceType = 0;

		if (pEntry->applicationtype & X_CT_SRV_INTERNET)
			ServiceType |= (1 << 0);
		if (pEntry->applicationtype & X_CT_SRV_TR069)
			ServiceType |= (1 << 1);
		if (pEntry->applicationtype & X_CT_SRV_VOICE)
			ServiceType |= (1 << 2);

		*data = uintdup(ServiceType);
#else
		*data = uintdup(0);
#endif
	}
#endif
	else if( strcmp( lastname, "PPPLCPEcho" )==0 )	// Jenny
	{
#ifdef CONFIG_PPP
		unsigned int echo;
		if( getPPPLCPEcho( ifname, pEntry->cmode, &echo )==0 )
			*data = uintdup( echo );
		else
#endif
			*data = uintdup( 0 );
	}else if( strcmp( lastname, "PPPLCPEchoRetry" )==0 )	// Jenny
	{
#ifdef CONFIG_PPP
		unsigned int retry;
		if( getPPPEchoRetry( ifname, pEntry->cmode, &retry )==0 )
			*data = uintdup( retry );
		else
#endif
			*data = uintdup( 0 );
	}else if( strcmp( lastname, "PortMappingNumberOfEntries" )==0 )
	{
#if defined(VIRTUAL_SERVER_SUPPORT) || defined(PORT_FORWARD_GENERAL)
		*data = uintdup( getPortMappingCount(pEntry->ifIndex) );
#else
		*data = uintdup( 0 );
#endif
	}else{
		return ERR_9005;
	}

	return 0;
}

int setWANPPPCONENTITY(char *name, struct CWMP_LEAF *entity, int type, void *data)
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

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;

		if(i==NULL) return ERR_9007;
		pEntry->enable = (*i==0) ? 0:1;
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
		return CWMP_NEED_RESTART_WAN;
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	}else if( strcmp( lastname, "Reset" )==0 )
	{
		int *i=data;
		if(i==NULL) return ERR_9007;
		if(*i==1)
		{
			resetChainID = chainidx;
			gStartReset = 1;
		}
		return 0;
#endif
/*ping_zhang:20081217 END*/
	}else if( strcmp( lastname, "ConnectionType" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( strcmp( buf, "IP_Routed" )==0 )
		{
#ifdef PPPOE_PASSTHROUGH
			unsigned int new_ifindex;
#ifdef E8B_NEW_DIAGNOSE
			FILE * fp1;
			int fileid;
			char tmpbuf[20];
			pvc_status_entry entry;
			memset(&entry, 0, sizeof(pvc_status_entry));
			entry.vpi = pEntry->vpi;
			entry.vci = pEntry->vci;
			fileid = findmatchpvcfileid(&entry);
			if (fileid > 0) {
				sprintf(tmpbuf, "%s%d", PVC_FILE, fileid);
				fp1 = fopen(tmpbuf, "wb+");
				if (fp1) {
					strcpy(entry.wantype, "路由");
					fwrite(&entry, sizeof(pvc_status_entry), 1, fp1);
					fclose(fp1);
				}
			}
#endif
			if( (pEntry->cmode==CHANNEL_MODE_PPPOA) ||
			    (pEntry->cmode==CHANNEL_MODE_PPPOE) )
			    return 0;

			//pppoe_bridge => ip_routed
			new_ifindex = getNewIfIndex(CHANNEL_MODE_PPPOE, pEntry->ConDevInstNum, MEDIA_INDEX(pEntry->ifIndex), chainidx);
			if( (new_ifindex==NA_VC)||(new_ifindex==NA_PPP) ) return ERR_9004;
			pEntry->cmode = CHANNEL_MODE_PPPOE;
			pEntry->ifIndex = TO_IFINDEX(MEDIA_INDEX(pEntry->ifIndex), PPP_INDEX(new_ifindex), VC_INDEX(pEntry->ifIndex));
			pEntry->mtu = 1492;
			pEntry->brmode=BRIDGE_DISABLE;
			pEntry->dnsMode = 1;
			if (pEntry->applicationtype&X_CT_SRV_INTERNET) {
				pEntry->napt=1;
#ifndef CONFIG_RTK_RG_INIT
				pEntry->dgw=1;
#endif
			}
			mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
			return CWMP_NEED_RESTART_WAN;
#else
			//PPPoE/A are IP_Routed
			return 0;
#endif
#ifdef PPPOE_PASSTHROUGH
		}else if( strcmp( buf, "PPPoE_Bridged" )==0 )
		{
			if( pEntry->cmode==CHANNEL_MODE_PPPOA )
				return ERR_9001;
			if( (pEntry->cmode!=CHANNEL_MODE_BRIDGE) ||
			    ((pEntry->cmode==CHANNEL_MODE_BRIDGE)&&(pEntry->brmode!=BRIDGE_PPPOE)) )
			{
				pEntry->ifIndex = TO_IFINDEX(MEDIA_INDEX(pEntry->ifIndex), DUMMY_PPP_INDEX, VC_INDEX(pEntry->ifIndex));
				pEntry->cmode=CHANNEL_MODE_BRIDGE;
				pEntry->brmode=BRIDGE_PPPOE;
				pEntry->dnsMode = 0;
			#ifdef CONFIG_E8B
				pEntry->IpProtocol = IPVER_IPV4_IPV6;
				pEntry->napt=0;
				pEntry->dgw=0;
				pEntry->applicationtype=X_CT_SRV_INTERNET;
			#ifdef _PRMT_X_CT_COM_WANEXT_
				pEntry->ServiceList = X_CT_SRV_INTERNET;
			#endif
			#endif
				mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
#ifdef E8B_NEW_DIAGNOSE
				FILE * fp1;
				int fileid;
				char tmpbuf[20];
				pvc_status_entry entry;
				memset(&entry, 0, sizeof(pvc_status_entry));
				entry.vpi = pEntry->vpi;
				entry.vci = pEntry->vci;
				fileid = findmatchpvcfileid(&entry);
				if (fileid > 0) {
					sprintf(tmpbuf, "%s%d", PVC_FILE, fileid);
					fp1 = fopen(tmpbuf, "wb+");
					if (fp1) {
						strcpy(entry.wantype, "桥接");
						fwrite(&entry, sizeof(pvc_status_entry), 1, fp1);
						fclose(fp1);
					}
				}
#endif
				return CWMP_NEED_RESTART_WAN;
			}
			return CWMP_APPLIED;
#endif
		}else
			return ERR_9007;
	}else if( strcmp( lastname, "Name" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)>=MAX_NAME_LEN ) return ERR_9007;
		strcpy( (char*)pEntry->WanName, buf );
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
		return 0;
	}else if( strcmp( lastname, "AutoDisconnectTime" )==0 )	// Jenny
	{
		unsigned int *disconntime = data;

		if(disconntime==NULL) return ERR_9007;
		pEntry->autoDisTime = (unsigned short) (*disconntime);
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
		return CWMP_NEED_RESTART_WAN;
	}else if( strcmp( lastname, "IdleDisconnectTime" )==0 )
	{
		unsigned int *idletime = data;

		if(idletime==NULL) return ERR_9007;
		pEntry->pppIdleTime = (unsigned short) (*idletime);
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
		return CWMP_NEED_RESTART_WAN;
	}else if( strcmp( lastname, "WarnDisconnectDelay" )==0 )	// Jenny
	{
		unsigned int *disconndelay = data;

		if(disconndelay==NULL) return ERR_9007;
		pEntry->warnDisDelay = (unsigned short) (*disconndelay);
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
		return CWMP_NEED_RESTART_WAN;
	}else if( strcmp( lastname, "NATEnabled" )==0 )
	{
		int *i = data;

		if(i==NULL) return ERR_9007;
		pEntry->napt = (*i==0) ? 0:1;
		//check bridge modes??
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
		return CWMP_NEED_RESTART_WAN;
	}else if( strcmp( lastname, "Username" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		strncpy( (char*)pEntry->pppUsername, buf, MAX_NAME_LEN-1 );
		pEntry->pppUsername[MAX_NAME_LEN-1]=0;
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
		return CWMP_NEED_RESTART_WAN;
	}else if( strcmp( lastname, "Password" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		strncpy( (char*)pEntry->pppPassword, buf, MAX_NAME_LEN-1 );
		pEntry->pppPassword[MAX_NAME_LEN-1]=0;
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
		return CWMP_NEED_RESTART_WAN;
	}else if( strcmp( lastname, "MaxMRUSize" )==0 )	// Jenny
	{
		unsigned int *mru = data;

		if (mru==NULL) return ERR_9007;
		if (*mru<1)	*mru = 1;
		else if (*mru>1540)	*mru = 1540;
		pEntry->mtu = *mru;
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "DNSOverrideAllowed" )==0 )
	{
		if (pUint == NULL)
			return ERR_9007;
		if (pEntry->cmode != CHANNEL_MODE_IPOE)
			return ERR_9001;
		pEntry->dnsMode = *pUint ? REQUEST_DNS_NONE : REQUEST_DNS;

		mib_chain_update(MIB_ATM_VC_TBL, (unsigned char *)pEntry, chainidx);
		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "DNSServers" )==0 )
	{
		if (set_dns_by_wan(buf, pEntry) == 0) {
			mib_chain_update(MIB_ATM_VC_TBL, pEntry, chainidx);
			cmd_set_dns_config(NULL);
			apply_add( CWMP_PRI_SL, apply_DNS, CWMP_RESTART, 0, NULL, 0 );
			return 0;
		} else {
			return ERR_9001;
		}
	}
	else if( strcmp( lastname, "PPPoEACName" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		//if( strlen(buf)==0 ) return ERR_9007;
		if( (pEntry->cmode==CHANNEL_MODE_PPPOE) && (strlen( buf )<MAX_NAME_LEN) )
			strcpy( (char*)pEntry->pppACName, buf );
		else
			return ERR_9001;

		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
		return CWMP_NEED_RESTART_WAN;
	}else if( strcmp( lastname, "PPPoEServiceName" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
//		if( strlen(buf)!=0 ) return ERR_9001;
		if( (pEntry->cmode==CHANNEL_MODE_PPPOE) && (strlen( buf )<MAX_NAME_LEN) )	// Jenny
			strcpy( (char*)pEntry->pppServiceName, buf );
		else
			return ERR_9001;

		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
		return CWMP_NEED_RESTART_WAN;
	}else if( strcmp( lastname, "ConnectionTrigger" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
#ifdef PPPOE_PASSTHROUGH
		if( pEntry->cmode==CHANNEL_MODE_BRIDGE )
		{
			if( strcmp( buf, "AlwaysOn" )==0 )
				return CWMP_APPLIED;
			else
				return ERR_9001;
		}
#endif
		if( strcmp( buf, "AlwaysOn" )==0 )
			pEntry->pppCtype=CONTINUOUS;
		else if( strcmp( buf, "OnDemand" )==0 )
			pEntry->pppCtype=CONNECT_ON_DEMAND;
		else if( strcmp( buf, "Manual" )==0 )
			pEntry->pppCtype=MANUAL;
		else
			return ERR_9007;
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
		return CWMP_NEED_RESTART_WAN;
	}else if( strcmp( lastname, "RouteProtocolRx" )==0 )
	{
		unsigned char newripmode=RIP_NONE;
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;

		if( strcmp( buf, "Off" )==0 )
			newripmode=RIP_NONE;
		else if( strcmp( buf, "RIPv1" )==0 )
			newripmode=RIP_V1;
		else if( strcmp( buf, "RIPv2" )==0 )
			newripmode=RIP_V2;
		else if( strcmp( buf, "RIPv1andRIPv2" )==0 )
			newripmode=RIP_V1_V2;
		else if( strcmp( buf, "OSPF" )==0 )
			return ERR_9001;
		else
			return ERR_9007;

		updateRIPInfo( pEntry->ifIndex, newripmode );

		apply_add( CWMP_PRI_N, apply_RIP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
#ifdef _PRMT_X_RTK_
	else if (strcmp(lastname, "X_RTK_IgmpProxy") == 0)
	{
		int *i = data;

		if (i == NULL) return ERR_9007;

		pEntry->enableIGMP = (*i == 0) ? 0:1;
		mib_chain_update(MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx);
		return CWMP_NEED_RESTART_WAN;
	}
	else if (strcmp(lastname, "X_RTK_ServiceType") == 0)
	{
		// 1 bit: Internet, 2 bit: TR-069, 3 bit: VoIP
#ifdef CONFIG_USER_RTK_WAN_CTYPE
		unsigned int *ServiceType = data;
		unsigned int apptype = 0;

		if(ServiceType == NULL) return ERR_9007;

		if(*ServiceType <= 0 || *ServiceType > 7) return ERR_9007;

		if(*ServiceType & 0x01)
			apptype |= X_CT_SRV_INTERNET;
		if(*ServiceType & 0x02)
			apptype |= X_CT_SRV_TR069;
		if(*ServiceType & 0x04)
			apptype |= X_CT_SRV_VOICE;

		pEntry->applicationtype = apptype;
		mib_chain_update(MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx);
		return CWMP_NEED_RESTART_WAN;
#else
		return 0;
#endif
	}
#endif
	else{
		return ERR_9005;
	}

	return 0;
}

int objWANPPPConn(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	//char	*tok;
	MIB_CE_ATM_VC_T *pEntry, vc_entity;
	unsigned int devnum, wandevnum;
	unsigned int num=0,i,maxnum=0,chainid=-1;

	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);fflush(NULL);
	//CWMPDBG( 1, ( stderr, "<%s:%d>name:%s(action:%d)\n", __FUNCTION__, __LINE__, name,type ) );
	if( (name==NULL) || (entity==NULL) ) return -1;

	wandevnum = getWANDevInstNum(name);
	devnum = getWANConDevInstNum(name);
	if(devnum==0 || wandevnum==0) return ERR_9005;

	switch( type )
	{
		case eCWMP_tINITOBJ:
		{
			struct CWMP_LINKNODE **ptable = (struct CWMP_LINKNODE **)data;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			maxnum = findMaxPPPConInstNum(getMediaTypeFromWanInst(wandevnum), devnum);
			num = mib_chain_total( MIB_ATM_VC_TBL );
			//CWMPDBG( 1, ( stderr, "<%s:%d>initobj, maxnum:%d, num:%d\n", __FUNCTION__, __LINE__, maxnum,num ) );
			for( i=0; i<num;i++ )
			{
				pEntry = &vc_entity;
				if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)pEntry ))
					continue;

				if(!isWANDevModeMatch(wandevnum, pEntry->ifIndex))
					continue;

				if( (pEntry->ConDevInstNum==devnum) && (pEntry->connDisable==0) )
				{
					if( (pEntry->cmode == CHANNEL_MODE_PPPOE) ||
	#ifdef PPPOE_PASSTHROUGH
					    ( (pEntry->cmode==CHANNEL_MODE_BRIDGE)&&(pEntry->brmode==BRIDGE_PPPOE) )  ||
	#endif
					    (pEntry->cmode == CHANNEL_MODE_PPPOA) )
					{
						if( create_Object( ptable, tWANPPPCONObject, sizeof(tWANPPPCONObject), 1, pEntry->ConPPPInstNum ) < 0 )
							return -1;
						//CWMPDBG( 1, ( stderr, "<%s:%d>add conppp:%d\n", __FUNCTION__, __LINE__, pEntry->ConPPPInstNum ) );
					}//if
				}//if
			}//for
			add_objectNum( name, maxnum );
			return 0;
			break;
		}
		case eCWMP_tADDOBJ:
		{
			int ret,cnt=0,found_first=-1, found_nocon=-1;
			MIB_CE_ATM_VC_T firstentity, noconentity;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			num = mib_chain_total( MIB_ATM_VC_TBL );
			for( i=0; i<num;i++ )
			{
				pEntry = &vc_entity;
				if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)pEntry ))
					continue;

				if(!isWANDevModeMatch(wandevnum, pEntry->ifIndex))
					continue;

				if(pEntry->ConDevInstNum==devnum)
				{
					cnt++;
					if(cnt==1)
					{
						found_first=i;
						memcpy( &firstentity, pEntry, sizeof(MIB_CE_ATM_VC_T) );
					}

					if(pEntry->connDisable==1)
					{
						found_nocon = i;
						memcpy( &noconentity, pEntry, sizeof(MIB_CE_ATM_VC_T) );
					}
				}
			}//for
			if(cnt==0) return ERR_9005;
			//connection > MAX_POE_PER_VC or ==MAX_POE_PER_VC with ip/ppp connection
			if( (cnt>MAX_POE_PER_VC) || ((cnt==MAX_POE_PER_VC) && (found_nocon==-1)) )
			    return ERR_9004;

			printf("\nfount=%d %d\n",found_nocon,found_first);
			if( found_nocon!=-1 ) //has one entry without pppconnection
			{
				i = found_nocon;
				pEntry = &noconentity;
			}
			else if( found_first!=-1 )  //create new one
			{
				unsigned int new_ifindex;
				new_ifindex = getNewIfIndex(CHANNEL_MODE_PPPOE, firstentity.ConDevInstNum, MEDIA_INDEX(firstentity.ifIndex), -1);

				CWMPDBP(2, "new ifIndex=%x\n", new_ifindex);
				if( (new_ifindex==NA_VC)||(new_ifindex==NA_PPP) ) return ERR_9001;

				i = found_first;
				pEntry = &firstentity;
				//set value to default;
				//pEntry->connDisable=1;
				//pEntry->ConPPPInstNum=0;
				resetATMVCConnection( pEntry );

				if(MEDIA_ATM == MEDIA_INDEX(pEntry->ifIndex))
					pEntry->ifIndex = TO_IFINDEX(MEDIA_ATM, PPP_INDEX(new_ifindex), VC_INDEX(pEntry->ifIndex));
				else if(MEDIA_ETH == MEDIA_INDEX(pEntry->ifIndex) || MEDIA_PTM == MEDIA_INDEX(pEntry->ifIndex))
					pEntry->ifIndex = new_ifindex;
			}else
				return ERR_9002;

			//wt-121v8 2.31, fail due to reaching the limit, return 9004
			if( pEntry->connDisable==0 ) /*already has one connection*/
				return ERR_9004;

#if 0 /*ct-com doesn't specify the mode first before creating the wanip/pppconnection */
			if( (pEntry->cmode != CHANNEL_MODE_PPPOE) && (pEntry->cmode != CHANNEL_MODE_PPPOA) )
				return ERR_9001;
#endif

			ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tWANPPPCONObject, sizeof(tWANPPPCONObject), data );
			if( ret >= 0 )
			{
				pEntry->connDisable=0;
				pEntry->ConPPPInstNum=*(int*)data;
				pEntry->ConIPInstNum=0;
				pEntry->enable=0;
				pEntry->dnsMode = 1;

#if 1 /*ct-com doesn't specify the mode first before creating the wanip/pppconnection */
				if( (pEntry->cmode != CHANNEL_MODE_PPPOE) && (pEntry->cmode != CHANNEL_MODE_PPPOA) )
				{
					unsigned int new_ifindex;
					if( found_nocon!=-1 )
						new_ifindex = getNewIfIndex(CHANNEL_MODE_PPPOE, pEntry->ConDevInstNum, MEDIA_INDEX(pEntry->ifIndex), i);
					else
						new_ifindex = getNewIfIndex(CHANNEL_MODE_PPPOE, pEntry->ConDevInstNum, MEDIA_INDEX(pEntry->ifIndex), -1);
					if( (new_ifindex==NA_VC)||(new_ifindex==NA_VC) ) return ERR_9001;
					CWMPDBP(2, "new ifIndex=%x\n", new_ifindex);
					pEntry->cmode = CHANNEL_MODE_PPPOE;

					if(MEDIA_ATM == MEDIA_INDEX(pEntry->ifIndex))
						pEntry->ifIndex = TO_IFINDEX(MEDIA_ATM, PPP_INDEX(new_ifindex), VC_INDEX(pEntry->ifIndex));
					else  //for MEDIA_ETH/CONFIG_ETHWAN, MEDIA_PTM/CONFIG_PTMWAN
						pEntry->ifIndex = new_ifindex;

					pEntry->mtu = 1492;
					pEntry->brmode=BRIDGE_DISABLE;
					if (pEntry->applicationtype&X_CT_SRV_INTERNET) {
						pEntry->napt=1;
#ifndef CONFIG_RTK_RG_INIT
						pEntry->dgw=1;
#endif
					}
#ifdef CONFIG_E8B
					//ipv6 default values
					pEntry->AddrMode = IPV6_WAN_DHCP;
					pEntry->Ipv6DhcpRequest |= 1;	//get address
#endif
				}
#endif
#if defined(CONFIG_LUNA) && defined(GEN_WAN_MAC)
				{
					//Interface index may be changed
					char macaddr[MAC_ADDR_LEN] = {0};
					mib_get(MIB_ELAN_MAC_ADDR, (void *)macaddr);
					macaddr[MAC_ADDR_LEN-1] += WAN_HW_ETHER_START_BASE + ETH_INDEX(pEntry->ifIndex);
					memcpy(pEntry->MacAddr, macaddr, MAC_ADDR_LEN);
				}
#endif
				if(found_nocon!=-1 )
					mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, i );
				else
					mib_chain_add( MIB_ATM_VC_TBL, (unsigned char*)pEntry );
#ifdef E8B_NEW_DIAGNOSE
				writePVCFile(pEntry->vpi, pEntry->vci, "add", "", "");
#endif
			}
			notify_set_wan_changed();
			//if( ret==0 ) ret=1;
			return ret;
			break;
		}
		case eCWMP_tDELOBJ:
		{
			int ret,cnt=0;
			#ifdef E8B_NEW_DIAGNOSE
			char s_appname[32];
			#endif

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			num = mib_chain_total( MIB_ATM_VC_TBL );
			for( i=0; i<num;i++ )
			{
				pEntry = &vc_entity;
				if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)pEntry ))
					continue;

				if(!isWANDevModeMatch(wandevnum, pEntry->ifIndex))
					continue;

				if(pEntry->ConDevInstNum==devnum)
				{
					cnt++;
					if(pEntry->ConPPPInstNum==*(unsigned int*)data)
					{
						chainid=i;
						break;
					}
				}
			}

			if(chainid == -1) return ERR_9005;

			ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next, *(int*)data );

			if( ret==0 )
			{
				//delete port-mappings of this connection.
#ifdef PORT_FORWARD_GENERAL
				delPortForwarding( pEntry->ifIndex );
#endif
#ifdef ROUTING
				delRoutingTable( pEntry->ifIndex );
#endif
#ifdef E8B_NEW_DIAGNOSE
				setWanName(s_appname, pEntry->applicationtype);
				writePVCFile(pEntry->vpi, pEntry->vci, "del", s_appname,
						(pEntry->cmode == CHANNEL_MODE_BRIDGE) ? "桥接" : "路由");
#endif
				//if the pppconnection is more than 1, delete the chain , not update it.
				if( cnt==1 )
				{
					//pEntry->connDisable=1;
					//reset ppp-related var.
					resetATMVCConnection( pEntry );
					mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainid );
				}else{
					mib_chain_delete( MIB_ATM_VC_TBL, chainid );
				}
				ret=1;
			}
			return ret;
			break;
		}
		case eCWMP_tUPDATEOBJ:
		{
			int has_new=0;
			struct CWMP_LINKNODE *old_table;

			/* WANDev instnum may be changed */
			clear_objectNum(name);

     	num = mib_chain_total( MIB_ATM_VC_TBL );
     	old_table = (struct CWMP_LINKNODE *)entity->next;
     	entity->next = NULL;
     	for( i=0; i<num;i++ )
     	{
     		struct CWMP_LINKNODE *remove_entity=NULL;

				pEntry = &vc_entity;
				if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)pEntry ))
					continue;

				if(!isWANDevModeMatch(wandevnum, pEntry->ifIndex))
					continue;

				if(pEntry->connDisable==1) continue;

				if( (pEntry->ConDevInstNum==devnum) && (pEntry->ConPPPInstNum!=0)  )
				{
					remove_entity = remove_SiblingEntity( &old_table, pEntry->ConPPPInstNum );
					if( remove_entity!=NULL )
					{
						add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
					}else{
						unsigned int MaxInstNum = pEntry->ConPPPInstNum;
						add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tWANPPPCONObject, sizeof(tWANPPPCONObject), &MaxInstNum );
						has_new=1;
					}
				}
			}

			if( old_table )
				destroy_ParameterTable( (struct CWMP_NODE *)old_table );
			if(has_new) notify_set_wan_changed();
				return 0;
		}
	}

	return -1;
}


/*InternetGatewayDevice.WANDevice.{i}.WANConnectionDevice.{i}.WANIPConnection.*/
int getWANIPCONENTITY(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned int	chainidx;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;
	char	ifname[16];
	char	buf[512]="";
	unsigned char vChar=0;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	pEntry = &vc_entity;
	if( getATMVCEntry(name, pEntry, &chainidx) < 0 )
		return ERR_9002;

	if(ifGetName( pEntry->ifIndex, ifname, 16 )==0) return ERR_9002;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		if(pEntry->enable)
			*data = booldup(1);
		else
			*data = booldup(0);
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	}else if( strcmp( lastname, "Reset" )==0 )
	{
		/*while read, this parameter always returns False.*/
		*data = booldup(0);
#endif
/*ping_zhang:20081217 END*/
	}else if( strcmp( lastname, "ConnectionStatus" )==0 )
	{
		int flags;

		if(pEntry->enable==0)
			*data = strdup( "Disconnected" );
		else if(getInFlags( ifname, &flags) == 1)
		{
			if (flags & IFF_RUNNING)
				*data = strdup( "Connected" );
			else
				*data = strdup( "Disconnected" );
		}else
			*data = strdup( "Disconnected" );
	}else if( strcmp( lastname, "PossibleConnectionTypes" )==0 )
	{
		*data = strdup( "IP_Routed,IP_Bridged" );
	}else if( strcmp( lastname, "ConnectionType" )==0 )
	{
		if (pEntry->cmode == CHANNEL_MODE_BRIDGE)
			*data = strdup("IP_Bridged");
#ifdef CONFIG_ATM_CLIP
		else if (pEntry->cmode == CHANNEL_MODE_IPOE || pEntry->cmode == CHANNEL_MODE_RT1483 || pEntry->cmode == CHANNEL_MODE_RT1577)
#else
		else if (pEntry->cmode == CHANNEL_MODE_IPOE || pEntry->cmode == CHANNEL_MODE_RT1483)
#endif
			*data = strdup("IP_Routed");
		else
			*data = strdup("Unconfigured");

	}else if( strcmp( lastname, "Name" )==0 )
	{
		if(*(pEntry->WanName))
			*data = strdup( (char*)pEntry->WanName );
#ifdef CTC_WAN_NAME
		else
		{//if not set by ACS. then generate automaticly.
			char wanname[40];
			memset(wanname, 0, sizeof(wanname));
			generateWanName(pEntry, wanname);
			*data = strdup( wanname );
		}
#endif
	}else if( strcmp( lastname, "Uptime" )==0 )
	{
		struct sysinfo info;
		sysinfo(&info);
		*data = uintdup( info.uptime );
	}else if( strcmp( lastname, "LastConnectionError" )==0 )
	{
		*data = strdup( "ERROR_NONE" );
	}else if( strcmp( lastname, "RSIPAvailable" )==0 )
	{
		*data = booldup( 0 );
	}else if( strcmp( lastname, "NATEnabled" )==0 )
	{
		if(pEntry->napt)
			*data = booldup( 1 );
		else
			*data = booldup( 0 );
	}else if( strcmp( lastname, "AddressingType" )==0 )
	{
		if(pEntry->ipDhcp == (char)DHCP_DISABLED)
			*data = strdup( "Static" );
		else
			*data = strdup( "DHCP" );
	}else if( strcmp( lastname, "ExternalIPAddress" )==0 )
	{
		char *temp=NULL;
		struct in_addr inAddr;
		if(pEntry->ipDhcp == (char)DHCP_DISABLED)
			temp = inet_ntoa(*((struct in_addr *)pEntry->ipAddr));
		else
			if (getInAddr( ifname, IP_ADDR, (void *)&inAddr) == 1)
				temp = inet_ntoa(inAddr);
		if(temp)
			*data=strdup(temp);
		else
			*data=strdup("");
	}else if( strcmp( lastname, "SubnetMask" )==0 )
	{
		char *temp=NULL;
		struct in_addr inAddr;
		if(pEntry->ipDhcp == (char)DHCP_DISABLED)
			temp = inet_ntoa(*((struct in_addr *)pEntry->netMask));
		else
			if (getInAddr( ifname, SUBNET_MASK, (void *)&inAddr) == 1)
				temp = inet_ntoa(inAddr);
		if(temp)
			*data=strdup(temp);
		else
			*data=strdup("");
	}else if( strcmp( lastname, "DefaultGateway" )==0 )
	{
		char *temp=NULL;
		struct in_addr inAddr;
		if(pEntry->ipDhcp == (char)DHCP_DISABLED)
			temp = inet_ntoa(*((struct in_addr *)pEntry->remoteIpAddr));
		else
			if (getInAddr( ifname, DST_IP_ADDR, (void *)&inAddr) == 1)
				temp = inet_ntoa(inAddr);
		if(temp)
			*data=strdup(temp);
		else
			*data=strdup("");
	}else if( strcmp( lastname, "DNSEnabled" )==0 )
	{
		*data = booldup(1);
	}else if( strcmp( lastname, "DNSOverrideAllowed" )==0 )
	{
		if ( ((pEntry->cmode == CHANNEL_MODE_RT1483) || (pEntry->cmode == CHANNEL_MODE_IPOE)) && (pEntry->dnsMode == REQUEST_DNS_NONE) )
			*data = booldup(1);
		else
			*data = booldup(0);
	}else if( strcmp( lastname, "DNSServers" )==0 )
	{
		get_dns_by_wan(buf, sizeof(buf), pEntry);
		*data = strdup( buf );
	}else if( strcmp( lastname, "MaxMTUSize" )==0 )
	{
		*data = uintdup(pEntry->mtu);
	}else if( strcmp( lastname, "MACAddress" )==0 )
	{
		//struct in_addr inAddr;
		char wanif[IFNAMSIZ];
		char macadd[MAC_ADDR_LEN] = {0};
		struct sockaddr sa;

		ifGetName(PHY_INTF(pEntry->ifIndex), wanif, sizeof(wanif));

		if(!getInAddr(wanif, HW_ADDR, (void *)&sa))
		{
			//if this entry is disabled, getInAddr() will return failed
			//return ERR_9002;
			*data = strdup("");
		}else{
			memcpy( macadd, sa.sa_data, MAC_ADDR_LEN );
			sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", macadd[0]&0xff, macadd[1]&0xff, macadd[2]&0xff, macadd[3]&0xff, macadd[4]&0xff, macadd[5]&0xff);
			*data = strdup(buf);
		}
	}else if( strcmp( lastname, "ConnectionTrigger" )==0 )
	{
		*data = strdup( "AlwaysOn" );
	}else if( strcmp( lastname, "RouteProtocolRx" )==0 )
	{
		unsigned char ripVer=RIP_NONE;
		getRIPInfo( pEntry->ifIndex, &ripVer );
		switch( ripVer )
		{
		case RIP_NONE:
			*data=strdup( "Off" );
			break;
		case RIP_V1:
			*data=strdup( "RIPv1" );
			break;
		case RIP_V2:
			*data=strdup( "RIPv2" );
			break;
		case RIP_V1_V2:
			*data=strdup( "RIPv1andRIPv2" );
			break;
		default:
			return ERR_9002;
		}
	}
#ifdef _PRMT_X_RTK_
	else if (strcmp(lastname, "X_RTK_IgmpProxy") == 0)
	{
		if (pEntry->enableIGMP == 1)
			*data = booldup(1);
		else
			*data = booldup(0);
	}
	else if (strcmp(lastname, "X_RTK_ServiceType") == 0)
	{
		// 1 bit: Internet, 2 bit: TR-069, 3 bit: VoIP
#ifdef CONFIG_USER_RTK_WAN_CTYPE
		unsigned int ServiceType = 0;

		if (pEntry->applicationtype & X_CT_SRV_INTERNET)
			ServiceType |= (1 << 0);
		if (pEntry->applicationtype & X_CT_SRV_TR069)
			ServiceType |= (1 << 1);
		if (pEntry->applicationtype & X_CT_SRV_VOICE)
			ServiceType |= (1 << 2);

		*data = uintdup(ServiceType);
#else
		*data = uintdup(0);
#endif
	}
#endif
	else if( strcmp( lastname, "PortMappingNumberOfEntries" )==0 )
	{
#if defined(VIRTUAL_SERVER_SUPPORT) || defined(PORT_FORWARD_GENERAL)
		*data = uintdup( getPortMappingCount(pEntry->ifIndex) );
#else
		*data = uintdup(0);
#endif
	}else{
		return ERR_9005;
	}

	return 0;
}

int setWANIPCONENTITY(char *name, struct CWMP_LEAF *entity, int type, void *data)
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

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;

		if(i==NULL) return ERR_9007;
		pEntry->enable = (*i==0) ? 0:1;
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
		return CWMP_NEED_RESTART_WAN;
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	}else if( strcmp( lastname, "Reset" )==0 )
	{
		int *i=data;
		if(i==NULL) return ERR_9007;
		if(*i==1)
		{
			resetChainID = chainidx;
			gStartReset = 1;
		}
		return CWMP_APPLIED;
#endif
/*ping_zhang:20081217 END*/
	}else if( strcmp( lastname, "ConnectionType" )==0 )
	{
		//?????????????
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( strcmp( buf, "IP_Routed" )==0 )
		{
			if( (pEntry->cmode==CHANNEL_MODE_BRIDGE) ||
			    (pEntry->cmode==CHANNEL_MODE_IPOE) )  /*EoA*/
				pEntry->cmode=CHANNEL_MODE_IPOE;
			else if( pEntry->cmode==CHANNEL_MODE_RT1483 ) /*IPoA*/
				pEntry->cmode=CHANNEL_MODE_RT1483;
			pEntry->brmode=BRIDGE_DISABLE;
			if (pEntry->applicationtype&X_CT_SRV_INTERNET) {
				pEntry->napt=1;
#ifndef CONFIG_RTK_RG_INIT
				pEntry->dgw=1;
#endif
			}
		}else if( strcmp( buf, "IP_Bridged" )==0 )
		{
			if( (pEntry->cmode==CHANNEL_MODE_BRIDGE) ||
			    (pEntry->cmode==CHANNEL_MODE_IPOE) )  /*EoA*/
				pEntry->cmode=CHANNEL_MODE_BRIDGE;
			else if( pEntry->cmode==CHANNEL_MODE_RT1483 ) /*IPoA*/
				return ERR_9007;
				pEntry->brmode = BRIDGE_ETHERNET;
				pEntry->napt=0;
				pEntry->dgw=0;
				pEntry->applicationtype=X_CT_SRV_INTERNET;
			#ifdef _PRMT_X_CT_COM_WANEXT_
				pEntry->ServiceList = X_CT_SRV_INTERNET;
			#endif
		}else
			return ERR_9007;

#ifdef E8B_NEW_DIAGNOSE
		FILE * fp1;
		int fileid;
		char tmpbuf[20];
		pvc_status_entry entry;
		memset(&entry, 0, sizeof(pvc_status_entry));
		entry.vpi = pEntry->vpi;
		entry.vci = pEntry->vci;
		fileid = findmatchpvcfileid(&entry);
		if (fileid > 0) {
			sprintf(tmpbuf, "%s%d", PVC_FILE, fileid);
			fp1 = fopen(tmpbuf, "wb+");
			if (fp1) {
				if (strcmp( buf, "IP_Routed" )==0)
					strcpy(entry.wantype, "路由");
				else
					strcpy(entry.wantype, "桥接");
				fwrite(&entry, sizeof(pvc_status_entry), 1, fp1);
				fclose(fp1);
			}
		}
#endif
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
		return CWMP_NEED_RESTART_WAN;
	}else if( strcmp( lastname, "Name" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)>=MAX_NAME_LEN ) return ERR_9007;
		strcpy( (char*)pEntry->WanName, buf );
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
		return CWMP_APPLIED;
	}else if( strcmp( lastname, "NATEnabled" )==0 )
	{
		int *i = data;

		if(i==NULL) return ERR_9007;
		pEntry->napt = (*i==0) ? 0:1;
		//check bridge modes??
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
		return CWMP_NEED_RESTART_WAN;
	}else if( strcmp( lastname, "AddressingType" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( strcmp( buf, "DHCP" )==0 )
		{
			pEntry->ipDhcp = (char) DHCP_CLIENT;
			pEntry->dnsMode = 1;
		}
		else if( strcmp( buf, "Static" )==0 )
		{
			pEntry->ipDhcp = (char) DHCP_DISABLED;
			pEntry->dnsMode = 0;
		}
		else
			return ERR_9007;
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
		return CWMP_NEED_RESTART_WAN;
	}else if( strcmp( lastname, "ExternalIPAddress" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( pEntry->ipDhcp == (char) DHCP_DISABLED )
		{
			if ( !inet_aton(buf, (struct in_addr *)&pEntry->ipAddr) )
				return ERR_9007;

		}else
			return ERR_9001;
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
		return CWMP_NEED_RESTART_WAN;
	}else if( strcmp( lastname, "SubnetMask" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( pEntry->ipDhcp == (char) DHCP_DISABLED )
		{
			if ( !inet_aton(buf, (struct in_addr *)&pEntry->netMask) )
				return ERR_9007;
		}else
			return ERR_9001;
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
		return CWMP_NEED_RESTART_WAN;
	}else if( strcmp( lastname, "DefaultGateway" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( pEntry->ipDhcp == (char) DHCP_DISABLED )
		{
			if ( !inet_aton(buf, (struct in_addr *)&pEntry->remoteIpAddr) )
				return ERR_9007;
		}else
			return ERR_9001;
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "DNSOverrideAllowed" )==0 )
	{
		if (pUint == NULL)
			return ERR_9007;
		if (pEntry->cmode != CHANNEL_MODE_IPOE)
			return ERR_9001;
		pEntry->dnsMode = *pUint ? REQUEST_DNS_NONE : REQUEST_DNS;

		mib_chain_update(MIB_ATM_VC_TBL, (unsigned char *)pEntry, chainidx);
		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "DNSServers" )==0 )
	{
		if (set_dns_by_wan(buf, pEntry) == 0) {
			mib_chain_update(MIB_ATM_VC_TBL, pEntry, chainidx);
			cmd_set_dns_config(NULL);
			apply_add( CWMP_PRI_SL, apply_DNS, CWMP_RESTART, 0, NULL, 0 );
			return 0;
		} else {
			return ERR_9001;
		}
	}
	else if( strcmp( lastname, "MaxMTUSize" )==0 )
	{
		unsigned int *newmtu = data;

		if( newmtu==NULL ) return ERR_9007;
		if(*newmtu<1 || *newmtu>1540) return ERR_9007;
		pEntry->mtu = *newmtu;
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx );
		return CWMP_NEED_RESTART_WAN;
	}else if( strcmp( lastname, "ConnectionTrigger" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( strcmp( buf, "AlwaysOn" )!=0 ) return ERR_9007;
	}else if( strcmp( lastname, "RouteProtocolRx" )==0 )
	{
		unsigned char newripmode=RIP_NONE;
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;

		if( strcmp( buf, "Off" )==0 )
			newripmode=RIP_NONE;
		else if( strcmp( buf, "RIPv1" )==0 )
			newripmode=RIP_V1;
		else if( strcmp( buf, "RIPv2" )==0 )
			newripmode=RIP_V2;
		else if( strcmp( buf, "RIPv1andRIPv2" )==0 )
			newripmode=RIP_V1_V2;
		else if( strcmp( buf, "OSPF" )==0 )
			return ERR_9001;
		else
			return ERR_9007;

		updateRIPInfo( pEntry->ifIndex, newripmode );

		apply_add( CWMP_PRI_N, apply_RIP, CWMP_RESTART, 0, NULL, 0 );
		return CWMP_APPLIED;
	}
#ifdef _PRMT_X_RTK_
	else if (strcmp(lastname, "X_RTK_IgmpProxy") == 0)
	{
		int *i = data;

		if (i == NULL) return ERR_9007;

		pEntry->enableIGMP = (*i == 0) ? 0:1;
		mib_chain_update(MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx);
		return CWMP_NEED_RESTART_WAN;
	}
	else if (strcmp(lastname, "X_RTK_ServiceType") == 0)
	{
		// 1 bit: Internet, 2 bit: TR-069, 3 bit: VoIP
#ifdef CONFIG_USER_RTK_WAN_CTYPE
		unsigned int *ServiceType = data;
		unsigned int apptype = 0;

		if(ServiceType == NULL) return ERR_9007;

		if(*ServiceType <= 0 || *ServiceType > 7) return ERR_9007;

		if(*ServiceType & 0x01)
			apptype |= X_CT_SRV_INTERNET;
		if(*ServiceType & 0x02)
			apptype |= X_CT_SRV_TR069;
		if(*ServiceType & 0x04)
			apptype |= X_CT_SRV_VOICE;

		pEntry->applicationtype = apptype;
		mib_chain_update(MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainidx);
		return CWMP_NEED_RESTART_WAN;
#else
		return 0;
#endif
	}
#endif
	else
		return ERR_9005;

	return CWMP_APPLIED;
}

int objWANIPConn(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	MIB_CE_ATM_VC_T *pEntry, vc_entity;
	unsigned int devnum, wandevnum;
	unsigned int num=0,i,maxnum=0;

	//CWMPDBG( 1, ( stderr, "<%s:%d>name:%s(action:%d)\n", __FUNCTION__, __LINE__, name,type ) );
	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);fflush(NULL);
	if( (name==NULL) || (entity==NULL) ) return -1;

	wandevnum = getWANDevInstNum(name);
	devnum = getWANConDevInstNum(name);
	if(devnum==0 || wandevnum==0) return ERR_9005;


	switch( type )
	{
		case eCWMP_tINITOBJ:
		{
			struct CWMP_LINKNODE **ptable = (struct CWMP_LINKNODE **)data;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			maxnum = findMaxIPConInstNum(getMediaTypeFromWanInst(wandevnum), devnum);
			num = mib_chain_total( MIB_ATM_VC_TBL );

			for( i = 0; i < num; i++ )
			{
				pEntry = &vc_entity;

				if(!mib_chain_get( MIB_ATM_VC_TBL, i, (void*)pEntry))
					continue;

				if(!isWANDevModeMatch(wandevnum, pEntry->ifIndex))
					continue;

				if( (pEntry->ConDevInstNum==devnum) && (pEntry->connDisable==0) )
				{
					if( (pEntry->cmode == CHANNEL_MODE_IPOE) ||
#ifdef PPPOE_PASSTHROUGH
					    ( (pEntry->cmode==CHANNEL_MODE_BRIDGE)&&(pEntry->brmode!=BRIDGE_PPPOE) )  ||
#else
					    (pEntry->cmode == CHANNEL_MODE_BRIDGE)  ||
#endif
					    (pEntry->cmode == CHANNEL_MODE_RT1483) )
					{
						if( create_Object( ptable, tWANIPCONObject, sizeof(tWANIPCONObject), 1, pEntry->ConIPInstNum ) < 0 )
							return -1;
						//fprintf( stderr, "<%s:%d>add condev:%u ipcon:%u\n", __FUNCTION__, __LINE__, pEntry->ConDevInstNum, pEntry->ConIPInstNum);
					}//if
				}//if
			}//for
			add_objectNum( name, maxnum );
			return 0;
			break;
		}
		case eCWMP_tADDOBJ:
		{
			int ret;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			num = mib_chain_total( MIB_ATM_VC_TBL );
			for( i=0; i<num;i++ )
			{
				pEntry = &vc_entity;
				if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)pEntry ))
					continue;

				if(!isWANDevModeMatch(wandevnum, pEntry->ifIndex))
					continue;

				if(pEntry->ConDevInstNum==devnum)
					break;
			}

			if(i==num) return ERR_9005;

     	//wt-121v8 2.31, fail due to reaching the limit, return 9004
			if( pEntry->connDisable==0 ) /*already has one connection*/
				return ERR_9004;

#if 0 /*ct-com doesn't specify the mode first before creating the wanip/pppconnection */
			if( (pEntry->cmode != CHANNEL_MODE_BRIDGE) &&
			    (pEntry->cmode != CHANNEL_MODE_IPOE) &&
			    (pEntry->cmode != CHANNEL_MODE_RT1483) )
				return ERR_9001;
#endif

			ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tWANIPCONObject, sizeof(tWANIPCONObject), data );
			if( ret >= 0 )
			{
				pEntry->connDisable=0;
				pEntry->ConIPInstNum=*(int*)data;
				pEntry->ConPPPInstNum=0;
				pEntry->enable=0;
#if 1 /*ct-com doesn't specify the mode first before creating the wanip/pppconnection */
				if( ((pEntry->cmode != CHANNEL_MODE_BRIDGE) && (pEntry->cmode != CHANNEL_MODE_IPOE) && (pEntry->cmode != CHANNEL_MODE_RT1483))
#ifdef PPPOE_PASSTHROUGH
				    ||((pEntry->cmode==CHANNEL_MODE_BRIDGE) && (pEntry->brmode==BRIDGE_PPPOE))
#endif
			  )
				{
					unsigned int new_ifindex;
					int j;
					new_ifindex = getNewIfIndex(CHANNEL_MODE_BRIDGE, pEntry->ConDevInstNum, MEDIA_INDEX(pEntry->ifIndex), i);
					if( (new_ifindex==NA_VC)||(new_ifindex==NA_PPP) ) return ERR_9001;
					pEntry->cmode = CHANNEL_MODE_BRIDGE;
					pEntry->ifIndex = new_ifindex;
					pEntry->mtu = 1500;
					pEntry->brmode = BRIDGE_ETHERNET;
#ifdef CONFIG_E8B
					for(j= 0 ; j < 4 ; j++)
					{
						pEntry->dhcp_opt60_type[j] = 34;
						pEntry->dhcp_opt125_type[j] = 2;
						pEntry->dhcpv6_opt16_type[j] = 34;
						pEntry->dhcpv6_opt17_type[j] = 2;
					}

					//ipv6 default values
					pEntry->AddrMode = IPV6_WAN_DHCP;
					pEntry->Ipv6DhcpRequest |= 1;	//get address
#endif
				}
#endif
				mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, i );
				gWanBitMap |= (1U << i);

#ifdef E8B_NEW_DIAGNOSE
				writePVCFile(pEntry->vpi, pEntry->vci, "add", "", "");
#endif
			}
			notify_set_wan_changed();
			return ret;
			break;
		}
		case eCWMP_tDELOBJ:
		{
			int ret;
			#ifdef E8B_NEW_DIAGNOSE
			char s_appname[32];
			#endif
			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			num = mib_chain_total( MIB_ATM_VC_TBL );
			for( i=0; i<num;i++ )
			{
				pEntry = &vc_entity;
				if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)pEntry ))
					continue;

				if(!isWANDevModeMatch(wandevnum, pEntry->ifIndex))
					continue;

				if( (pEntry->ConDevInstNum==devnum) && (pEntry->ConIPInstNum==*(unsigned int*)data) )
					break;
			}//for

			if(i==num) return ERR_9005;
			gWanBitMap |= (1U << i);

			ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next, *(int*)data );
			if( ret==0 )
			{
				//delete port-mappings of this connection.
#ifdef PORT_FORWARD_GENERAL
				delPortForwarding( pEntry->ifIndex );
#endif
#ifdef ROUTING
				delRoutingTable( pEntry->ifIndex );
#endif
#ifdef E8B_NEW_DIAGNOSE
				setWanName(s_appname, pEntry->applicationtype);
				writePVCFile(pEntry->vpi, pEntry->vci, "del", s_appname, (pEntry->cmode == CHANNEL_MODE_BRIDGE) ? "桥接" : "路由");
#endif
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
				delDhcpcOption( pEntry->ifIndex );
#endif
				//if the ipconnection is more than 1, delete the chain , not update it.
				//pEntry->connDisable=1;
				resetATMVCConnection( pEntry );
				mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, i );
				ret=CWMP_NEED_RESTART_WAN;
			}
			return ret;
			break;
		}
		case eCWMP_tUPDATEOBJ:
		{
			struct CWMP_LINKNODE *old_table;
			int has_new=0;

			/* WANDev instnum may be changed */
			clear_objectNum(name);

			//CWMPDBG( 1, ( stderr, "<%s:%d>\n", __FUNCTION__, __LINE__ ) );
			num = mib_chain_total( MIB_ATM_VC_TBL );
			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;
			for( i=0; i<num;i++ )
			{
				struct CWMP_LINKNODE *remove_entity=NULL;

				pEntry = &vc_entity;
				if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)pEntry ))
					continue;

				if(!isWANDevModeMatch(wandevnum, pEntry->ifIndex))
					continue;

				if(pEntry->connDisable==1) continue;

				if( (pEntry->ConDevInstNum==devnum) && (pEntry->ConIPInstNum!=0) )
				{
					remove_entity = remove_SiblingEntity( &old_table, pEntry->ConIPInstNum );
					if( remove_entity!=NULL )
					{
						add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
					}
					else
					{
						unsigned int MaxInstNum=pEntry->ConIPInstNum;
						add_Object(name, (struct CWMP_LINKNODE **)&entity->next, tWANIPCONObject, sizeof(tWANIPCONObject), &MaxInstNum);
						has_new=1;
					}
				}
			}

			if( old_table )
				destroy_ParameterTable( (struct CWMP_NODE *)old_table );

			if(has_new) notify_set_wan_changed();
			return 0;
			}
		}
	return -1;
}

#ifdef CONFIG_DEV_xDSL
/*InternetGatewayDevice.WANDevice.{i}.WANConnectionDevice.{i}.WANDSLLinkConfig.*/
int getDSLLNKCONF(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	char	buf[256];
	unsigned int wandevnum, devnum,num,i;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	wandevnum = getWANDevInstNum(name);
	devnum = getWANConDevInstNum(name);
	if(devnum==0 || wandevnum == 0) return ERR_9005;

	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		pEntry = &vc_entity;
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)pEntry ))
			continue;

		if(!isWANDevModeMatch(wandevnum, pEntry->ifIndex))
			continue;

		if( pEntry->ConDevInstNum==devnum )
			break;
	}
	if(i==num) return ERR_9005;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		if(pEntry->enable)
			*data = booldup(1);
		else
			*data = booldup(0);
	}else if( strcmp( lastname, "LinkStatus" )==0 )
	{
		int 	flags;
		char	ifname[16];

		if(ifGetName( pEntry->ifIndex, ifname, 16 )==0) return ERR_9002;

		if(pEntry->enable==0)
			*data = strdup( "Down" );
		else if(getInFlags( ifname, &flags) == 1)
		{
			if (flags & IFF_RUNNING)
				*data = strdup( "Up" );
			else
				*data = strdup( "Down" );
		}else
			*data = strdup( "Unavailable" );
	}else if( strcmp( lastname, "LinkType" )==0 )
	{
		if( pEntry->cmode == CHANNEL_MODE_PPPOE )
			*data = strdup("EoA");//PPPoE => EoA
		else if (pEntry->cmode == CHANNEL_MODE_PPPOA)
			*data = strdup("PPPoA");
		else if (pEntry->cmode == CHANNEL_MODE_BRIDGE)
			*data = strdup("EoA");
		else if (pEntry->cmode == CHANNEL_MODE_IPOE)
			*data = strdup("EoA");
		else if (pEntry->cmode == CHANNEL_MODE_RT1483)
			*data = strdup("IPoA");
		else
			*data = strdup("Unconfigured");
	}else if( strcmp( lastname, "AutoConfig" )==0 )
	{
		*data = booldup( 0 );
	}else if( strcmp( lastname, "ModulationType" )==0 )
	{
		getAdslInfo(ADSL_GET_MODE, buf, 256);
		if( strncmp( buf, "T1.413", 6 )==0 )
			*data = strdup( "ADSL_ANSI_T1.413" );
		else if( strncmp( buf, "G.dmt", 5 )==0 )
			*data = strdup( "ADSL_G.dmt" );
		else if( strncmp( buf, "G.Lite", 6 )==0 )
			*data = strdup( "ADSL_G.lite" );
		else if( strncmp( buf, "ADSL2+", 6 )==0 )
			*data = strdup( "ADSL_2plus" );
		else if( strncmp( buf, "ADSL2", 5 )==0 )
			*data = strdup( "ADSL_G.dmt.bis" );
#ifdef CONFIG_VDSL
		else if( strncmp( buf, "VDSL2", 5 )==0 )
			*data = strdup( "VDSL2" );//spec doesn't define VDSL2
#endif /*CONFIG_VDSL*/
		else
			*data = strdup( "" );
	}else if( strcmp( lastname, "DestinationAddress" )==0 )
	{
		sprintf( buf, "PVC:%u/%u", pEntry->vpi, pEntry->vci );
		*data = strdup( buf );
	}else if( strcmp( lastname, "ATMEncapsulation" )==0 )
	{
		if( pEntry->encap==ENCAP_VCMUX )
			*data = strdup( "VCMUX" );
		else if( pEntry->encap==ENCAP_LLC )
			*data = strdup( "LLC" );
		else
			return ERR_9002;
	}else if( strcmp( lastname, "ATMAAL" )==0 )
	{
		*data = strdup( "AAL5" );
	}else if( strcmp( lastname, "ATMTransmittedBlocks" )==0 )
	{
		unsigned int txcnt=0, rxcnt=0;
		getATMCellCnt( pEntry->vpi, pEntry->vci, &txcnt, &rxcnt );
		*data = uintdup( txcnt );
	}else if( strcmp( lastname, "ATMReceivedBlocks" )==0 )
	{
		unsigned int txcnt=0, rxcnt=0;
		getATMCellCnt( pEntry->vpi, pEntry->vci, &txcnt, &rxcnt );
		*data = uintdup( rxcnt );
	}else if( strcmp( lastname, "ATMQoS" )==0 )
	{
		if( pEntry->qos==ATMQOS_UBR )
			*data = strdup( "UBR" );
		else if( pEntry->qos==ATMQOS_CBR )
			*data = strdup( "CBR" );
		else if( pEntry->qos==ATMQOS_VBR_NRT )
			*data = strdup( "VBR-nrt" );
		else if( pEntry->qos==ATMQOS_VBR_RT )
			*data = strdup( "VBR-rt" );
		else
			return ERR_9002;
	}else if( strcmp( lastname, "ATMPeakCellRate" )==0 )
	{
		*data = uintdup( pEntry->pcr );
	}else if( strcmp( lastname, "ATMMaximumBurstSize" )==0 )
	{
		*data = uintdup( pEntry->mbs );
	}else if( strcmp( lastname, "ATMSustainableCellRate" )==0 )
	{
		*data = uintdup( pEntry->scr );
	}else if( strcmp( lastname, "AAL5CRCErrors" )==0 ) /*the same value with ATMCRCErrors, wt121*/
	{
		unsigned int count;
		if( getAAL5CRCErrors( pEntry->vpi, pEntry->vci, &count )<0 ) return ERR_9002;
		*data = uintdup( count );
	}else if( strcmp( lastname, "ATMCRCErrors" )==0 ) /*the same value with AAL5CRCErrors, wt121*/
	{
		unsigned int count;
		if( getAAL5CRCErrors( pEntry->vpi, pEntry->vci, &count )<0 ) return ERR_9002;
		*data = uintdup( count );
	}else{
		return ERR_9005;
	}

	return 0;
}

int setDSLLNKCONF(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	//char	*tok;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;
	//char	buf[256];
	unsigned int wandevnum, devnum,num,chainid;
	int ret=CWMP_APPLIED;
	//unsigned int pppoe_ifindex=DUMMY_IFINDEX;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;
	if( data==NULL ) return ERR_9007;

	wandevnum = getWANDevInstNum(name);
	devnum = getWANConDevInstNum(name);
	if(devnum==0 || wandevnum==0) return ERR_9005;

	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( chainid=0; chainid<num;chainid++ )
	{
		pEntry = &vc_entity;
		if( !mib_chain_get( MIB_ATM_VC_TBL, chainid, (void*)pEntry ))
			continue;

		if(!isWANDevModeMatch(wandevnum, pEntry->ifIndex))
			continue;

		if( pEntry->ConDevInstNum==devnum )
		{
			gWanBitMap |= (1U << chainid);
			if( strcmp( lastname, "Enable" )==0 )
			{
				int *i = data;

				if( i==NULL ) return ERR_9007;
				pEntry->enable = (*i==0)?0:1;
				mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainid );
				ret=CWMP_NEED_RESTART_WAN;//return CWMP_NEED_RESTART_WAN;
			}else if( strcmp( lastname, "LinkType" )==0 )
			{
				char *tmp=data;
				unsigned int old_ifindex, new_ifindex;

				old_ifindex = pEntry->ifIndex;
				if( tmp==NULL ) return ERR_9007;
				if( strlen(tmp)==0 ) return ERR_9007;

				if( strcmp( "PPPoE", tmp )==0 )
				{
#if 1
					//use EoA to create PPPoE
					return ERR_9001;
#else
					if( pEntry->cmode != CHANNEL_MODE_PPPOE )
					{
						new_ifindex = getNewIfIndex( CHANNEL_MODE_PPPOE );
						if( (new_ifindex==NA_VC)||(new_ifindex==NA_VC) ) return ERR_9001;
						if(pppoe_ifindex==DUMMY_IFINDEX) pppoe_ifindex=new_ifindex;
						pEntry->cmode = CHANNEL_MODE_PPPOE;
						//use the same vc, only the pppoe has multi-seesion
						pEntry->ifIndex = TO_IFINDEX(MEDIA_ATM, PPP_INDEX(new_ifindex), VC_INDEX(pppoe_ifindex));
						pEntry->mtu = 1492;
					}
#endif
				}else if( strcmp( "PPPoA", tmp )==0 )
				{
					if(pEntry->cmode != CHANNEL_MODE_PPPOA)
					{
						new_ifindex = getNewIfIndex( CHANNEL_MODE_PPPOA, pEntry->ConDevInstNum, MEDIA_INDEX(pEntry->ifIndex), chainid);
						if( (new_ifindex==NA_VC)||(new_ifindex==NA_PPP) ) return ERR_9001;
						pEntry->cmode = CHANNEL_MODE_PPPOA;
						pEntry->ifIndex = new_ifindex;
						pEntry->mtu = 1500;
					}
				}else if( strcmp( "EoA", tmp )==0 )
				{
					if( (pEntry->cmode!=CHANNEL_MODE_BRIDGE) &&
					    (pEntry->cmode!=CHANNEL_MODE_IPOE) &&
					    (pEntry->cmode!=CHANNEL_MODE_PPPOE) )
					{
						new_ifindex = getNewIfIndex( CHANNEL_MODE_BRIDGE, pEntry->ConDevInstNum, MEDIA_INDEX(pEntry->ifIndex), chainid);
						if( (new_ifindex==NA_VC)||(new_ifindex==NA_PPP) ) return ERR_9001;
						pEntry->cmode = CHANNEL_MODE_BRIDGE;
						pEntry->ifIndex = new_ifindex;
						pEntry->mtu = 1500;
				#ifdef CONFIG_E8B
					#ifdef _PRMT_X_CT_COM_WANEXT_
						pEntry->ServiceList=X_CT_SRV_INTERNET;
					#endif
					#ifdef CTC_WAN_NAME
						pEntry->applicationtype = X_CT_SRV_INTERNET;
					#endif
				#endif
					}
				}else if( strcmp( "IPoA", tmp )==0 )
				{
					if(pEntry->cmode != CHANNEL_MODE_RT1483)
					{
						new_ifindex = getNewIfIndex( CHANNEL_MODE_RT1483, pEntry->ConDevInstNum, MEDIA_INDEX(pEntry->ifIndex), chainid);
						if( (new_ifindex==NA_VC)||(new_ifindex==NA_PPP) ) return ERR_9001;
						pEntry->cmode = CHANNEL_MODE_RT1483;
						pEntry->ifIndex = new_ifindex;
						pEntry->mtu = 1500;
					}
				}else
					return ERR_9007;

				if(pEntry->ifIndex!=old_ifindex)
				{
					//if ip -> ppp	or ppp->ip, destroy ppp/ip objects and reset values??
					mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainid );
#ifdef PORT_FORWARD_GENERAL
					updatePortForwarding( old_ifindex, pEntry->ifIndex );
#endif
#ifdef ROUTING
					updateRoutingTable( old_ifindex, pEntry->ifIndex );
#endif
					ret=CWMP_NEED_RESTART_WAN;//return CWMP_NEED_RESTART_WAN;;
				}
			}else if( strcmp( lastname, "DestinationAddress" )==0 )
			{
				char tmp1[64],*tmp=data, *sVPI, *sVCI;
				int vpi,vci;
				if( tmp==NULL ) return ERR_9007;
				if( strlen(tmp)==0 ) return ERR_9007;
				/*for loop, strtok will change the data */
				strncpy( tmp1,tmp,63 );
				tmp1[63]=0;
				tmp = tmp1;
				if( strncmp( tmp, "PVC", 3 )!=0 ) return ERR_9007;
				tmp = tmp + 3;
				sVPI = strtok( tmp, " :/" );
				sVCI = strtok( NULL, " :/" );
				if( (sVPI==NULL) || (sVCI==NULL) ) return ERR_9007;
				vpi = atoi( sVPI );
				vci = atoi( sVCI );
				if( (vpi<0) || (vci<0) ) return ERR_9007;
				//if (vpi,vci) conflicts with other channels??
				pEntry->vpi = vpi;
				pEntry->vci = vci;
				mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainid );
				ret=CWMP_NEED_RESTART_WAN;//return CWMP_NEED_RESTART_WAN;
			}else if( strcmp( lastname, "ATMEncapsulation" )==0 )
			{
				char *tmp=data;
				if( tmp==NULL ) return ERR_9007;
				if( strlen(tmp)==0 ) return ERR_9007;
				if( strcmp(tmp, "VCMUX")==0 )
					pEntry->encap=ENCAP_VCMUX;
				else if( strcmp(tmp, "LLC")==0 )
					pEntry->encap=ENCAP_LLC;
				else
					return ERR_9007;

				mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainid );
				ret=CWMP_NEED_RESTART_WAN;//return CWMP_NEED_RESTART_WAN;
			}else if( strcmp( lastname, "ATMQoS" )==0 )
			{
				char *tmp=data;
				if( tmp==NULL ) return ERR_9007;
				if( strlen(tmp)==0 ) return ERR_9007;
				if( strcmp(tmp, "UBR")==0 )
					pEntry->qos=ATMQOS_UBR;
				else if( strcmp(tmp, "CBR")==0 )
					pEntry->qos=ATMQOS_CBR;
				else if( strcmp(tmp, "VBR-nrt")==0 )
					pEntry->qos=ATMQOS_VBR_NRT;
				else if( strcmp(tmp, "VBR-rt")==0 )
					pEntry->qos=ATMQOS_VBR_RT;
				else
					return ERR_9007;

				mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainid );
				ret=CWMP_NEED_RESTART_WAN;//return CWMP_NEED_RESTART_WAN;
			}else if( strcmp( lastname, "ATMPeakCellRate" )==0 )
			{
				unsigned int *pUInt = data;

				if(pUInt==NULL) return ERR_9007;
				if( *pUInt<1 || *pUInt>ATM_MAX_US_PCR )	return ERR_9001;
				pEntry->pcr = *pUInt;
				mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainid );
				ret=CWMP_NEED_RESTART_WAN;//return CWMP_NEED_RESTART_WAN;
			}else if( strcmp( lastname, "ATMMaximumBurstSize" )==0 )
			{
				unsigned int *pUInt = data;

				if(pUInt==NULL) return ERR_9007;
				if( *pUInt>65535 ) return ERR_9001;
				pEntry->mbs = *pUInt;
				mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainid );
				ret=CWMP_NEED_RESTART_WAN;//return CWMP_NEED_RESTART_WAN;
			}else if( strcmp( lastname, "ATMSustainableCellRate" )==0 )
			{
				unsigned int *pUInt = data;

				if(pUInt==NULL) return ERR_9007;
				if( *pUInt<1 || *pUInt>ATM_MAX_US_PCR )	return ERR_9001;
				pEntry->scr = *pUInt;
				mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainid );
				ret=CWMP_NEED_RESTART_WAN;//return CWMP_NEED_RESTART_WAN;
			}else{
				return ERR_9005;
			}
		}//if
	}//for
	//if(chainid==num) return ERR_9005;

	return ret;
}


#ifdef CONFIG_PTMWAN
/*InternetGatewayDevice.WANDevice.{i}.WANConnectionDevice.{i}.WANPTMLinkConfig.*/
int getPTMLNKCONF(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	char	buf[256];

	//fprintf( stderr, "%s: name=%s\n", __FUNCTION__, name);fflush(NULL);
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
			*data = booldup(1);
	}else if( strcmp( lastname, "LinkStatus" )==0 )
	{
#ifdef CONFIG_VDSL
		int mval=0;

		getAdslInfo(ADSL_GET_STATE, buf, 256);
		if( strncmp( buf, "HANDSHAKING", 11 )==0 )
		{
			*data = strdup( "Initializing" );
		}else if( dsl_msg_get(GetPmdMode,&mval) )
		{
			if(mval&MODE_VDSL2)
			{
#endif /*CONFIG_VDSL*/
				int flags;
				if( (getInFlags(ALIASNAME_PTM0, &flags)==0) ||
					((flags&IFF_RUNNING)==0) )
				{
					*data = strdup("Down");
				}else{
					*data = strdup("Up");
				}
#ifdef CONFIG_VDSL
			}else
				*data = strdup( "Down" );
		}else
			*data = strdup( "Unavailable" );
#endif /*CONFIG_VDSL*/
	}else if( strcmp( lastname, "MACAddress" )==0 )
	{
		char wanif[IFNAMSIZ]=ALIASNAME_PTM0;
		char macadd[MAC_ADDR_LEN] = {0};
		struct sockaddr sa;

		if(!getInAddr(wanif, HW_ADDR, (void *)&sa))
		{
			*data = strdup("");
		}else{
			memcpy( macadd, sa.sa_data, MAC_ADDR_LEN );
			sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", macadd[0]&0xff, macadd[1]&0xff, macadd[2]&0xff, macadd[3]&0xff, macadd[4]&0xff, macadd[5]&0xff);
			*data = strdup(buf);
		}
	}else{
		return ERR_9005;
	}

	return 0;
}

int setPTMLNKCONF(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	int ret=CWMP_APPLIED;

	//fprintf( stderr, "%s: name=%s\n", __FUNCTION__, name);fflush(NULL);
	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;
	if( data==NULL ) return ERR_9007;

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;
		if( i==NULL ) return ERR_9007;
		if(*i!=1) return ERR_9001;
	}else{
		return ERR_9005;
	}

	return ret;
}

/*InternetGatewayDevice.WANDevice.{i}.WANConnectionDevice.{i}.WANPTMLinkConfig.Stats.*/
int getPTMLNKSTATS(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	char	ifname[16]=ALIASNAME_PTM0;
	struct net_device_stats nds;
	struct ethtool_stats *stats = NULL;

	//fprintf( stderr, "%s: name=%s\n", __FUNCTION__, name);fflush(NULL);
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	if(get_net_device_stats(ifname, &nds) < 0)
	{
		memset(&nds, 0, sizeof(nds));
	}

	stats = ethtool_gstats(ifname);

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "BytesSent" )==0 )
	{
		*data = uintdup(stats ? stats->data[TX_OCTETS] : nds.tx_bytes);
	}else if( strcmp( lastname, "BytesReceived" )==0 )
	{
		*data = uintdup(stats ? stats->data[RX_OCTETS] : nds.rx_bytes);
	}else if( strcmp( lastname, "FramesSent" )==0 )
	{
		*data = uintdup(stats ? stats->data[TX_UCAST_PACKETS]
				+ stats->data[TX_MCAST_PACKETS]
				+ stats->data[TX_BCAST_PACKETS]
				: nds.tx_packets);
	}else if( strcmp( lastname, "FramesReceived" )==0 )
	{
		*data = uintdup(stats ? stats->data[RX_UCAST_PACKETS]
				+ stats->data[RX_MCAST_PACKETS]
				+ stats->data[RX_BCAST_PACKETS]
				: nds.rx_packets);
	}else if( strcmp( lastname, "OOSNearEnd" )==0 )
	{
		*data = booldup(0);
	}else if( strcmp( lastname, "OOSFarEnd" )==0 )
	{
		*data = booldup(0);
	}else if( strcmp( lastname, "ErrorsSent" )==0 )
	{
		*data = uintdup(nds.tx_errors);
	}else if( strcmp( lastname, "ErrorsReceived" )==0 )
	{
		*data = uintdup(nds.rx_errors);
	}else if( strcmp( lastname, "UnicastPacketsSent" )==0 )
	{
		*data = uintdup(stats ? stats->data[TX_UCAST_PACKETS] : 0);
	}else if( strcmp( lastname, "UnicastPacketsReceived" )==0 )
	{
		*data = uintdup(stats ? stats->data[RX_UCAST_PACKETS] : 0);
	}else if( strcmp( lastname, "DiscardPacketsSent" )==0 )
	{
		*data = uintdup(stats ? stats->data[TX_DISCARDS] : nds.tx_dropped);
	}else if( strcmp( lastname, "DiscardPacketsReceived" )==0 )
	{
		*data = uintdup(stats ? stats->data[RX_DISCARDS] : nds.rx_dropped);
	}else if( strcmp( lastname, "MulticastPacketsSent" )==0 )
	{
		*data = uintdup(stats ? stats->data[TX_MCAST_PACKETS] : 0);
	}else if( strcmp( lastname, "MulticastPacketsReceived" )==0 )
	{
		*data = uintdup(stats ? stats->data[RX_MCAST_PACKETS] : nds.multicast);
	}else if( strcmp( lastname, "BroadcastPacketsSent" )==0 )
	{
		*data = uintdup(stats ? stats->data[TX_BCAST_PACKETS] : 0);
	}else if( strcmp( lastname, "BroadcastPacketsReceived" )==0 )
	{
		*data = uintdup(stats ? stats->data[RX_BCAST_PACKETS] : 0);
	}else if( strcmp( lastname, "UnknownProtoPacketsReceived" )==0 )
	{
		*data = uintdup(0);
	}else{
		free(stats);
		return ERR_9005;
	}
	free(stats);

	return 0;
}
#endif /*CONFIG_PTMWAN*/
#endif  // #ifdef CONFIG_DEV_xDSL
#ifdef CONFIG_ETHWAN
/*InternetGatewayDevice.WANDevice.{i}.WANConnectionDevice.{i}.WANEthernetLinkConfig.*/
int getWANETHLINKCONF(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	//char	buf[256];
	unsigned int wandevnum, devnum,num,i;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	wandevnum = getWANDevInstNum(name);
	devnum = getWANConDevInstNum(name);
	if(devnum==0 || wandevnum ==0) return ERR_9005;

	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		pEntry = &vc_entity;
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)pEntry ))
			continue;

		if(!isWANDevModeMatch(wandevnum, pEntry->ifIndex))
			continue;

		if( pEntry->ConDevInstNum==devnum )
			break;
	}

	if(i==num) return ERR_9005;

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "EthernetLinkStatus" )==0 )
	{
		if(MEDIA_ETH == MEDIA_INDEX(pEntry->ifIndex))
		{
			struct ifreq ifr;
			int sockfd;
			char	ifname[16];

			sockfd = socket(AF_INET, SOCK_DGRAM, 0);
			bzero(&ifr, sizeof(ifr));

			ifGetName(pEntry->ifIndex, ifname, sizeof(ifname));

			strcpy(ifr.ifr_name, ifname);

			if(ioctl(sockfd, SIOCGIFFLAGS, &ifr) == -1)
			{
				*data = strdup("Unavailable");
				CWMPDBP(1, "Error! %s\n", strerror(errno));
			}
			else
			{
				if (ifr.ifr_flags & IFF_RUNNING)
					*data = strdup("Up");
				else
					*data = strdup("Down");
			}

			close(sockfd);
		}
		else
			*data = strdup("Unavailable");
	}
	else
		return ERR_9005;

	return 0;
}
#endif // #ifdef CONFIG_ETHWAN

/*InternetGatewayDevice.WANDevice.{i}.WANConnectionDevice.*/
int getWANCONDEVENTITY(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	MIB_CE_ATM_VC_T *p, vc_entity;
	unsigned int wandevnum, wancondevnum, i,num, ipcnt=0, pppcnt=0;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	wandevnum = getWANDevInstNum(name);
	wancondevnum = getWANConDevInstNum(name);
	if(wandevnum==0 || wancondevnum==0) return ERR_9005;

	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		p = &vc_entity;
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)p ))
			continue;

		if(!isWANDevModeMatch(wandevnum, p->ifIndex))
			continue;

		if( p->ConDevInstNum == wancondevnum )
		{
			switch( p->cmode )
			{
			case CHANNEL_MODE_BRIDGE:
#ifdef PPPOE_PASSTHROUGH
				if( p->connDisable==0 )
				{
					if(p->brmode==BRIDGE_PPPOE)
						pppcnt++;
					else
						ipcnt++;
				}
				break;
#endif
			case CHANNEL_MODE_IPOE:
			case CHANNEL_MODE_RT1483:
				if( p->connDisable==0 ) ipcnt++;
				break;
			case CHANNEL_MODE_PPPOE:
			case CHANNEL_MODE_PPPOA:
				if( p->connDisable==0 ) pppcnt++;
				break;
			}
		}
	}

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "WANIPConnectionNumberOfEntries" )==0 )
	{
		*data = uintdup( ipcnt );
	}else if( strcmp( lastname, "WANPPPConnectionNumberOfEntries" )==0 )
	{
		*data = uintdup( pppcnt );
	}else{
		return ERR_9005;
	}

	return 0;
}

int objConDev(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	unsigned int wandevnum;

	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);fflush(NULL);
	CWMPDBG( 0, ( stderr, "<%s:%d>name:%s(action:%d)\n", __FUNCTION__, __LINE__, name,type ) );

	wandevnum = getWANDevInstNum(name);

	if(wandevnum==0) return ERR_9005;

	switch( type )
	{
		case eCWMP_tINITOBJ:
		{
			int num, MaxInstNum, i, ret = 0;
			struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;
			MIB_CE_ATM_VC_T *p,vc_entity;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			MaxInstNum = findMaxConDevInstNum(getMediaTypeFromWanInst(wandevnum));
			num = mib_chain_total( MIB_ATM_VC_TBL );
			CWMPDBG(0, ( stderr, "<%s:%d>initobj, MaxInstNum:%d, num:%d\n", __FUNCTION__, __LINE__, MaxInstNum,num ) );

			for( i = 0; i < num; i++ )
			{
				int j, addit = 1;

				p = &vc_entity;
				if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)p ))
					continue;

				if(!isWANDevModeMatch(wandevnum, p->ifIndex))
					continue;

				//check if added
				for( j = 0; j < i; j++ )
				{
					MIB_CE_ATM_VC_T match_entity;
					if(!mib_chain_get(MIB_ATM_VC_TBL, j, (void*)&match_entity))
						continue;

					//if(!isWANDevModeMatch(wandevnum, p->ifIndex))
					if(!isWANDevModeMatch(wandevnum, match_entity.ifIndex))
						continue;

					CWMPDBP(0, "p->ConDevInstNum=%d, match_entity.ConDevInstNum=%d\n", p->ConDevInstNum, match_entity.ConDevInstNum);

					if( p->ConDevInstNum == match_entity.ConDevInstNum )
					{
						addit=0;
						break;
					}
				}

				if(addit)
				{
#ifdef CONFIG_DEV_xDSL
					if(wandevnum==WANDEVNUM_ATM)
					{
						if( create_Object( c, tWANCONDEVObjectDSL, sizeof(tWANCONDEVObjectDSL), 1, p->ConDevInstNum ) < 0 ) {
							ret = -1;
							break;
						}
						CWMPDBG(0, ( stderr, "<%s:%d>add ConDev:%d\n", __FUNCTION__, __LINE__, p->ConDevInstNum ) );
					}else
#ifdef CONFIG_PTMWAN
					if(wandevnum==WANDEVNUM_PTM)
					{
						if( create_Object( c, tWANCONDEVObjectPTM, sizeof(tWANCONDEVObjectPTM), 1, p->ConDevInstNum ) < 0 ) {
							ret = -1;
							break;
						}
						CWMPDBG(0, ( stderr, "<%s:%d>add ConDev:%d\n", __FUNCTION__, __LINE__, p->ConDevInstNum ) );
					}else
#endif /*CONFIG_PTMWAN*/
#endif /*CONFIG_DEV_xDSL*/
#ifdef CONFIG_ETHWAN
#if defined(_PRMT_X_CT_COM_WANEXT_) && (defined(CONFIG_EPON_FEATURE) || defined(CONFIG_GPON_FEATURE))
					if(wandevnum==WANDEVNUM_ETH)
					{
							int mode;

							mib_get(MIB_PON_MODE, &mode);
							if(mode == ETH_MODE)
							{
								if( create_Object( c, tWANCONDEVObjectEth, sizeof(tWANCONDEVObjectEth), 1, p->ConDevInstNum ) < 0 ) {
									ret = -1;
									break;
								}
								CWMPDBG(0, ( stderr, "<%s:%d>add ConDev:%d\n", __FUNCTION__, __LINE__, p->ConDevInstNum ) );
							}
						#ifdef  CONFIG_EPON_FEATURE
							else if(mode == EPON_MODE)
							{
								if( create_Object( c, tWANCONDEVObjectEPON, sizeof(tWANCONDEVObjectEPON), 1, p->ConDevInstNum ) < 0 ) {
									ret = -1;
									break;
								}
								CWMPDBG(0, ( stderr, "<%s:%d>add EPON ConDev:%d\n", __FUNCTION__, __LINE__, p->ConDevInstNum ) );
							}
						#endif
						#ifdef CONFIG_GPON_FEATURE
							else if(mode == GPON_MODE)
							{
								if( create_Object( c, tWANCONDEVObjectGPON, sizeof(tWANCONDEVObjectGPON), 1, p->ConDevInstNum ) < 0 ) {
									ret = -1;
									break;
								}
								CWMPDBG(0, ( stderr, "<%s:%d>add GPON ConDev:%d\n", __FUNCTION__, __LINE__, p->ConDevInstNum ) );
							}
						#endif
							else
								return ERR_9002;
					}
#else
					if(wandevnum==WANDEVNUM_ETH)
					{
						if( create_Object( c, tWANCONDEVObjectEth, sizeof(tWANCONDEVObjectEth), 1, p->ConDevInstNum ) < 0 ) {
							ret = -1;
							break;
						}
						CWMPDBG(0, ( stderr, "<%s:%d>add ConDev:%d\n", __FUNCTION__, __LINE__, p->ConDevInstNum ) );
					}else
#endif
#endif	//CONFIG_ETHWAN
					{
						//wandev does not match
						ret = -1;
						break;
					}
				}
			}

			if (ret == 0)
				add_objectNum( name, MaxInstNum );
			return ret;
		}
		case eCWMP_tADDOBJ:
		{
			int ret;
			unsigned int ifindex=0;
			unsigned int *pNextInstNum;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			//ifindex = getNewIfIndex(CHANNEL_MODE_BRIDGE, 0, getWANDevMode(wandevnum) == MODE_ADSL? MEDIA_ATM: MEDIA_ETH, -1);  //default: bridge mode
			ifindex = getNewIfIndex(CHANNEL_MODE_BRIDGE, 0, getMediaTypeFromWanInst(wandevnum), -1);  //default: bridge mode
			//wt-121v8 2.31, fail due to reaching the limit, return 9004
			if( ifindex==NA_VC ) return ERR_9004; //Maximum number of VC exceeds
			CWMPDBG( 1, ( stderr, "<%s:%d>addobj, ifindex:0x%08x\n", __FUNCTION__, __LINE__, ifindex ) );


			pNextInstNum = (unsigned int *)data;
			*pNextInstNum = 1 + findMaxConDevInstNum(getMediaTypeFromWanInst(wandevnum));

#ifdef CONFIG_DEV_xDSL
			if(wandevnum==WANDEVNUM_ATM)
			{
				ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next, tWANCONDEVObjectDSL, sizeof(tWANCONDEVObjectDSL), data );
			}else
#ifdef CONFIG_PTMWAN
			if(wandevnum==WANDEVNUM_PTM)
			{
				ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next, tWANCONDEVObjectPTM, sizeof(tWANCONDEVObjectPTM), data );
			}else
#endif /*CONFIG_PTMWAN*/
#endif /*CONFIG_DEV_xDSL*/
#ifdef CONFIG_ETHWAN
#if defined(_PRMT_X_CT_COM_WANEXT_) && (defined(CONFIG_EPON_FEATURE) || defined(CONFIG_GPON_FEATURE))
			if(wandevnum==WANDEVNUM_ETH)
			{
					int mode;

					mib_get(MIB_PON_MODE, &mode);
					if(mode == ETH_MODE)
						ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next, tWANCONDEVObjectEth, sizeof(tWANCONDEVObjectEth), data );
				#ifdef CONFIG_GPON_FEATURE
					else if(mode == GPON_MODE)
						ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next, tWANCONDEVObjectGPON, sizeof(tWANCONDEVObjectGPON), data );
				#endif
				#ifdef CONFIG_EPON_FEATURE
					else if(mode == EPON_MODE)
						ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next, tWANCONDEVObjectEPON, sizeof(tWANCONDEVObjectEPON), data );
				#endif
					else
						return ERR_9002;
			}else
#else
			if(wandevnum==WANDEVNUM_ETH)
			{
				ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next, tWANCONDEVObjectEth, sizeof(tWANCONDEVObjectEth), data );
			}else
#endif
#endif	//CONFIG_ETHWAN
			{
				//wandev does not match
				return ERR_9002;
			}

			if( ret >= 0 )
			{
				MIB_CE_ATM_VC_T entry;
				memset( &entry, 0, sizeof( MIB_CE_ATM_VC_T ) );
				{ //default values for this new entry
					entry.qos=0;
					entry.pcr=ATM_MAX_US_PCR;
					entry.encap=1;//LLC
					entry.mtu=1500;
					entry.ifIndex=ifindex;
					entry.connDisable=1; //0:enable, 1:disable
					entry.ConDevInstNum=*(int*)data;
#ifdef CONFIG_E8B
					entry.applicationtype=X_CT_SRV_INTERNET;
#ifdef _PRMT_X_CT_COM_WANEXT_
					entry.ServiceList=X_CT_SRV_INTERNET;
#endif
#endif
#ifdef CONFIG_USER_WT_146
					wt146_set_default_config( &entry );
#endif //CONFIG_USER_WT_146
#ifdef CONFIG_IPV6
					entry.IpProtocol=IPVER_IPV4;//ipv4 as default
#endif /*CONFIG_IPV6*/
#if defined(CONFIG_LUNA) && defined(GEN_WAN_MAC)
					{
						char macaddr[MAC_ADDR_LEN] = {0};
						mib_get(MIB_ELAN_MAC_ADDR, (void *)macaddr);
						macaddr[MAC_ADDR_LEN-1] += 3 + ETH_INDEX(entry.ifIndex);
						memcpy(entry.MacAddr, macaddr, MAC_ADDR_LEN);
					}
#endif
				}
				mib_chain_add( MIB_ATM_VC_TBL, (unsigned char*)&entry );
			}
			notify_set_wan_changed();
			return ret;
		}
		case eCWMP_tDELOBJ:
		{
			int ret, num, i;
			unsigned int *pUint=data;
			MIB_CE_ATM_VC_T *p, vc_entity;
			#ifdef E8B_NEW_DIAGNOSE
			char s_appname[32];
			#endif

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			num = mib_chain_total( MIB_ATM_VC_TBL );
			for( i=num-1; i>=0;i-- )
			{
				//delete port-mappings of this connectiondevice.
				p = &vc_entity;
				if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)p ) )
					continue;

				if(!isWANDevModeMatch(wandevnum, p->ifIndex))
					continue;

				if(p->ConDevInstNum==*pUint )
				{
#ifdef PORT_FORWARD_GENERAL
					delPortForwarding( p->ifIndex );
#endif
#ifdef ROUTING
					delRoutingTable( p->ifIndex );
#endif
					mib_chain_delete( MIB_ATM_VC_TBL, i );
#ifdef E8B_NEW_DIAGNOSE
					setWanName(s_appname, p->applicationtype);
					writePVCFile(p->vpi, p->vci, "del", s_appname, (p->cmode == CHANNEL_MODE_BRIDGE) ? "桥接" : "路由");
#endif
				}
			}
			ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next, *(int*)data );
			gWanBitMap = UINT_MAX;
			if( ret==0 )	ret=CWMP_NEED_RESTART_WAN;
			return ret;
		}
		case eCWMP_tUPDATEOBJ:
		{
			int num,i;
			struct CWMP_LINKNODE *old_table;
			int has_new=0;

			/* WANDev instnum may be changed */
			clear_objectNum(name);

			num = mib_chain_total( MIB_ATM_VC_TBL );
			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;

			for( i=0; i<num;i++ )
			{
				struct CWMP_LINKNODE *remove_entity=NULL;
				MIB_CE_ATM_VC_T *p,vc_entity;

				p = &vc_entity;
				if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)p ))
					continue;

				if(!isWANDevModeMatch(wandevnum, p->ifIndex))
					continue;

				remove_entity = remove_SiblingEntity( &old_table, p->ConDevInstNum );
				if( remove_entity!=NULL )
				{
					add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
				}
				else
				{
					if( find_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, p->ConDevInstNum )==NULL )
					{
						unsigned int MaxInstNum=p->ConDevInstNum;

#ifdef CONFIG_DEV_xDSL
						if(wandevnum==WANDEVNUM_ATM)
						{
							add_Object( name, (struct CWMP_LINKNODE **)&entity->next, tWANCONDEVObjectDSL, sizeof(tWANCONDEVObjectDSL), &MaxInstNum );
						}else
#ifdef CONFIG_PTMWAN
						if(wandevnum==WANDEVNUM_PTM)
						{
							add_Object( name, (struct CWMP_LINKNODE **)&entity->next, tWANCONDEVObjectPTM, sizeof(tWANCONDEVObjectPTM), &MaxInstNum );
						}else
#endif /*CONFIG_PTMWAN*/
#endif /*CONFIG_DEV_xDSL*/
#ifdef CONFIG_ETHWAN
#if defined(_PRMT_X_CT_COM_WANEXT_) && (defined(CONFIG_EPON_FEATURE) || defined(CONFIG_GPON_FEATURE))
						if(wandevnum==WANDEVNUM_ETH)
						{
								int mode;

								mib_get(MIB_PON_MODE, &mode);
								if(mode == ETH_MODE)
									add_Object( name, (struct CWMP_LINKNODE **)&entity->next, tWANCONDEVObjectEth, sizeof(tWANCONDEVObjectEth), &MaxInstNum );
							#ifdef CONFIG_GPON_FEATURE
								else if(mode == GPON_MODE)
									add_Object( name, (struct CWMP_LINKNODE **)&entity->next, tWANCONDEVObjectGPON, sizeof(tWANCONDEVObjectGPON), &MaxInstNum );
							#endif
							#ifdef CONFIG_EPON_FEATURE
								else if(mode == EPON_MODE)
									add_Object( name, (struct CWMP_LINKNODE **)&entity->next, tWANCONDEVObjectEPON, sizeof(tWANCONDEVObjectEPON), &MaxInstNum );
							#endif
								else
									return ERR_9002;

						}else
#else
						if(wandevnum==WANDEVNUM_ETH)
						{
							add_Object( name, (struct CWMP_LINKNODE **)&entity->next, tWANCONDEVObjectEth, sizeof(tWANCONDEVObjectEth), &MaxInstNum );
						}else
#endif
#endif	//CONFIG_ETHWAN
						{
							//wandev does not match
							return ERR_9002;
						}

						has_new=1;
					}//else already in next_table
				}
			}

			if( old_table )
			{
				destroy_ParameterTable( (struct CWMP_NODE *)old_table );
			}

			if(has_new) notify_set_wan_changed();
			{
				return 0;
			}
		}
	}
	return -1;
}


/*InternetGatewayDevice.WANDevice.{i}.WANDSLConnectionManagement.*/
int getCONSERENTITY(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	//char	*tok=NULL;
	int	chainid;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;
	char	buf[256];
	unsigned int conserid,wandevnum;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	wandevnum = getWANDevInstNum(name);
	if(wandevnum==0) return ERR_9005;
	conserid = getInstNum( name, "ConnectionService" );
	if(conserid==0) return ERR_9005;

	chainid = conserid -1;
	pEntry = &vc_entity;
	if( !mib_chain_get(MIB_ATM_VC_TBL, chainid, (void*)pEntry ) )
		return ERR_9002;
	if( MEDIA_INDEX(pEntry->ifIndex) != MEDIA_ATM )
		return ERR_9002;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "WANConnectionDevice" )==0 )
	{
		char strfmt[]="InternetGatewayDevice.WANDevice.%u.WANConnectionDevice.%u";//wt-121v8-3.33, no trailing dot
		sprintf( buf, strfmt, wandevnum, pEntry->ConDevInstNum );
		*data = strdup( buf );
	}else if( strcmp( lastname, "WANConnectionService" )==0 )
	{
		char strfmt[]="InternetGatewayDevice.WANDevice.%u.WANConnectionDevice.%u.%s.%u"; //wt-121v8-3.33, no trailing dot
		char ipstr[]="WANIPConnection";
		char pppstr[]="WANPPPConnection";

		if( pEntry->connDisable==1) //no connection with this connectiondevice
			*data = strdup("");
		else
		{
			if( (pEntry->cmode==CHANNEL_MODE_PPPOE) || (pEntry->cmode==CHANNEL_MODE_PPPOA) )
				sprintf( buf, strfmt, wandevnum, pEntry->ConDevInstNum, pppstr, pEntry->ConPPPInstNum );
			else
				sprintf( buf, strfmt, wandevnum, pEntry->ConDevInstNum, ipstr, pEntry->ConIPInstNum );
		}
		*data = strdup( buf );
	}else if( strcmp( lastname, "DestinationAddress" )==0 )
	{
		sprintf( buf, "PVC:%u/%u", pEntry->vpi, pEntry->vci );
		*data = strdup( buf );
	}else if( strcmp( lastname, "LinkType" )==0 )
	{
		if( pEntry->cmode == CHANNEL_MODE_PPPOE )
			*data = strdup("PPPoE");
		else if (pEntry->cmode == CHANNEL_MODE_PPPOA)
			*data = strdup("PPPoA");
		else if (pEntry->cmode == CHANNEL_MODE_BRIDGE)
			*data = strdup("EoA");
		else if (pEntry->cmode == CHANNEL_MODE_IPOE)
			*data = strdup("EoA");
		else if (pEntry->cmode == CHANNEL_MODE_RT1483)
			*data = strdup("IPoA");
		else
			*data = strdup("Unconfigured");
	}else if( strcmp( lastname, "ConnectionType" )==0 )
	{
		if( pEntry->connDisable==1) //don't create a connection with this connectiondevice
			*data = strdup("Unconfigured");
		else if( pEntry->cmode == CHANNEL_MODE_PPPOE )
			*data = strdup("IP_Routed");
		else if (pEntry->cmode == CHANNEL_MODE_PPPOA)
			*data = strdup("IP_Routed");
		else if (pEntry->cmode == CHANNEL_MODE_BRIDGE)
			*data = strdup("IP_Bridged");
		else if (pEntry->cmode == CHANNEL_MODE_IPOE)
			*data = strdup("IP_Routed");
		else if (pEntry->cmode == CHANNEL_MODE_RT1483)
			*data = strdup("IP_Routed");
		else
			*data = strdup("Unconfigured");
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	}else if( strcmp( lastname, "Name" )==0 )
	{
		if(*(pEntry->WanName))
			*data = strdup( (char*)pEntry->WanName );
		else
			*data = strdup("");
#endif
/*ping_zhang:20081217 END*/
	}else{
		return ERR_9005;
	}

	return 0;
}

int objConService(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	int num=0, i, num_of_entries;
	MIB_CE_ATM_VC_T pvcEntry;
	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);

	switch( type )
	{
		case eCWMP_tINITOBJ:
		{
			struct CWMP_LINKNODE **ptable = (struct CWMP_LINKNODE **)data;
			if(data==NULL) return -1;

			num_of_entries = mib_chain_total( MIB_ATM_VC_TBL );

			for( i  = 0; i < num_of_entries; i++ )
			{
				if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&pvcEntry))
					return  ERR_9002;

				if( MEDIA_INDEX(pvcEntry.ifIndex) == MEDIA_ATM )
				{
					//use chain index+1 as instance number
					create_Object( ptable, tCONSERVICEObject, sizeof(tCONSERVICEObject), 1, i+1 );
				}
			}

			return 0;
		}
		case eCWMP_tUPDATEOBJ:
		{
			struct CWMP_LINKNODE *old_table;

			num = mib_chain_total( MIB_ATM_VC_TBL );
			old_table = (struct CWMP_LINKNODE*)entity->next;
			entity->next = NULL;

			for( i=0; i<num;i++ )
			{
				struct CWMP_LINKNODE *remove_entity=NULL;

				if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&pvcEntry))
					return  ERR_9002;

				if(!(MEDIA_INDEX(pvcEntry.ifIndex) == MEDIA_ATM))
					continue;

				remove_entity = remove_SiblingEntity( &old_table, i+1 );
				if( remove_entity != NULL )
				{
					add_SiblingEntity( (struct CWMP_LINKNODE**)&entity->next, remove_entity );
				}else{
					unsigned int InstNum=i+1;
					add_Object( name, (struct CWMP_LINKNODE**)&entity->next,  tCONSERVICEObject, sizeof(tCONSERVICEObject), &InstNum );
				}
	    }

			if( old_table )
				destroy_ParameterTable( (struct CWMP_NODE *)old_table );

			return 0;
		}
	}

	return -1;
}


int getWANDSLCNTMNG(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	MIB_CE_ATM_VC_T pvcEntry;
	int i, num = 0;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "ConnectionServiceNumberOfEntries" )==0 )
	{
		int total;
		total = mib_chain_total(MIB_ATM_VC_TBL);

		for( i = 0; i < total; i++ )
		{
			if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&pvcEntry))
				return  ERR_9002;

			if( MEDIA_INDEX(pvcEntry.ifIndex) == MEDIA_ATM )
				num++;
		}

		*data = uintdup(num);
	}else{
		return ERR_9005;
	}

	return 0;
}
/**************************************************************************************/
/* utility functions*/
/**************************************************************************************/
#if 0
int getObjectID( struct sCWMP_ENTITY *ctable, int chainid )
{
	int i=-1;
	int objectid=0;

	if( chainid< 0 ) return objectid;

	while( ctable )
	{
		i++;
		if( i==chainid ) break;
		ctable = ctable->sibling;
	}

	if(ctable)
		sscanf( ctable->name, "%d", &objectid );

	return objectid;
}
#endif


static MEDIA_TYPE_T getMediaTypeFromWanInst(unsigned int instnum)
{
	MEDIA_TYPE_T type;

	if (instnum == WANDEVNUM_ETH)
		type = MEDIA_ETH;
	else if (instnum == WANDEVNUM_PTM)
		type = MEDIA_PTM;
	else			//default
		type = MEDIA_ATM;

	return type;
}

static unsigned int getWanInstFromMediaType(MEDIA_TYPE_T type)
{
	unsigned int wanInst;

	if (type == MEDIA_ETH)
		wanInst = WANDEVNUM_ETH;
	else if (type == MEDIA_PTM)
		wanInst = WANDEVNUM_PTM;
	else			//default
		wanInst = WANDEVNUM_ATM;

	return wanInst;
}

static unsigned int getWanInstFromifIndex(unsigned int ifIndex)
{
	return getWanInstFromMediaType(MEDIA_INDEX(ifIndex));
}

#if 0
int getWANDevMode(unsigned int instnum)
{
	if(WAN_MODE == (MODE_ADSL|MODE_Ethernet))
	{
			if(instnum == WANDEVNUM_ATM)
				return MODE_ADSL;
//#ifdef CONFIG_PTMWAN
			else if(instnum == WANDEVNUM_PTM)
				return MODE_ADSL;
//#endif /*CONFIG_PTMWAN*/
			else if(instnum == WANDEVNUM_ETH)
				return MODE_Ethernet;
			else
				return 0;//-1;
	}
	else
		return WAN_MODE;
}
#endif

int isWANDevModeMatch(unsigned int instnum, unsigned int if_idx)
{
	//int wandev_mode = getWANDevMode(instnum);

	if( (instnum==WANDEVNUM_ATM) &&
		/*(wandev_mode==MODE_ADSL) &&*/
		(MEDIA_ATM==MEDIA_INDEX(if_idx)) )
	{
		return CWMP_NEED_RESTART_WAN;
//#ifdef CONFIG_PTMWAN
	}else if((instnum==WANDEVNUM_PTM) &&
			/*(wandev_mode==MODE_ADSL) &&*/
			(MEDIA_PTM==MEDIA_INDEX(if_idx)) )
	{
		return CWMP_NEED_RESTART_WAN;
//#endif /*CONFIG_PTMWAN*/
	}else if((instnum==WANDEVNUM_ETH) &&
			/*(wandev_mode==MODE_Ethernet) &&*/
			(MEDIA_ETH==MEDIA_INDEX(if_idx)) )
	{
		return CWMP_NEED_RESTART_WAN;
	}else{
		return CWMP_APPLIED;
	}
}

/*copy from fmwan.c:if_find_index*/
unsigned int getNewIfIndex(int cmode, int my_condev_instnum, int media_type, int chainidx)
	{
	int i;
	unsigned int if_index;
	MEDIA_TYPE_T mType;

	unsigned int totalEntry;
	MIB_CE_ATM_VC_T *pEntry, vc_entity;
	unsigned int map = 0;	// high half for PPP bitmap, low half for vc bitmap

	totalEntry = mib_chain_total(MIB_ATM_VC_TBL); /* get chain record size */

	for (i=0; i<totalEntry; i++)
	{
		pEntry = &vc_entity;
		if(i == chainidx)
			continue;

		if( mib_chain_get(MIB_ATM_VC_TBL, i, (void*)pEntry ) ) /* get the specified chain record */
		{
			mType = MEDIA_INDEX(pEntry->ifIndex);

			//if(pEntry->ConDevInstNum != my_condev_instnum)
			if( (mType!=media_type) ||
				(pEntry->ConDevInstNum!=my_condev_instnum) ||
				(pEntry->cmode != cmode))
			{
				if (mType == MEDIA_ETH)
					map |= 1 << ETH_INDEX(pEntry->ifIndex);	// vc map
				else if (mType == MEDIA_ATM)
					map |= 1 << VC_INDEX(pEntry->ifIndex);	// vc map
#ifdef CONFIG_PTMWAN
				else if (mType == MEDIA_PTM)
					map |= 1 << PTM_INDEX(pEntry->ifIndex);	// vc map
#endif /*CONFIG_PTMWAN*/

				map |= (1 << 16) << PPP_INDEX(pEntry->ifIndex);	// PPP map
			}
		}
	}

	if_index = if_find_index(cmode, map);
	if( (if_index==NA_VC)||(if_index==NA_PPP) )
		return if_index;

	if(media_type == MEDIA_ETH)
		if_index = TO_IFINDEX(MEDIA_ETH, PPP_INDEX(if_index), ETH_INDEX(if_index));
#ifdef CONFIG_PTMWAN
	if(media_type == MEDIA_PTM)
		if_index = TO_IFINDEX(MEDIA_PTM, PPP_INDEX(if_index), PTM_INDEX(if_index));
#endif /*CONFIG_PTMWAN*/

	return if_index;
}


/*functions for get an object's number*/
unsigned int getWANDevInstNum( char *name )
{
	return getInstNum( name, "WANDevice" );
}

unsigned int getWANConDevInstNum( char *name )
{
	return getInstNum( name, "WANConnectionDevice" );
}

unsigned int getWANPPPConInstNum( char *name )
{
	return getInstNum( name, "WANPPPConnection" );
}

unsigned int getWANIPConInstNum( char *name )
{
	return getInstNum( name, "WANIPConnection" );
}


unsigned int getWANPortMapInstNum( char *name )
{
	return getInstNum( name, "PortMapping" );
}

/*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
unsigned int getSentDHCPOptInstNum( char *name )
{
	return getInstNum( name, "SentDHCPOption" );
}

unsigned int getReqDHCPOptInstNum( char *name )
{
	return getInstNum( name, "ReqDHCPOption" );
}
#endif
/*ping_zhang:20080919 END*/

#ifdef VIRTUAL_SERVER_SUPPORT
/*portmapping utilities*/
unsigned int getPortMappingMaxInstNum( unsigned int ifindex )
{
	unsigned int ret = 0;
	MIB_VIRTUAL_SVR_T *p, port_entity;
	unsigned int total,i;

	total = mib_chain_total( MIB_VIRTUAL_SVR_TBL );
	for( i=0;i<total;i++ )
	{
		p = &port_entity;
		if( mib_chain_get( MIB_VIRTUAL_SVR_TBL, i, (void*)p ) )
		{
			if(p->InstanceNum>ret) ret = p->InstanceNum;
		}
	}

	return ret;
}

int getPortMappingCount( unsigned int ifindex )
{
	int count = 0;
	MIB_VIRTUAL_SVR_T *p, port_entity;
	unsigned int total,i;

	total = mib_chain_total( MIB_VIRTUAL_SVR_TBL );
	for( i=0;i<total;i++ )
	{
		p = &port_entity;
		if( mib_chain_get( MIB_VIRTUAL_SVR_TBL, i, (void*)p ) )
			count++;
	}

	return count;
}

/*id starts from 0, 1, 2, ...*/
int getPortMappingByID( unsigned int ifindex, int id, MIB_VIRTUAL_SVR_T *c, unsigned int *chainID )
{
	unsigned int total,i;

	if( (id < 0) || (c==NULL) || (chainID==NULL) ) return -1;

	total = mib_chain_total( MIB_VIRTUAL_SVR_TBL );
	for( i=0;i<total;i++ )
	{
		if( !mib_chain_get( MIB_VIRTUAL_SVR_TBL, i, (void*)c ) )
			continue;

		id--;
		if(id==-1)
		{
			*chainID = i;
			return 0;
		}
	}

	return -1;
}

int getPortMappingByInstNum( unsigned int ifindex, unsigned int instnum, MIB_VIRTUAL_SVR_T *c, unsigned int *chainID )
{
	unsigned int total,i;

	if( (instnum == 0) || (c==NULL) || (chainID==NULL) ) return -1;

	total = mib_chain_total( MIB_VIRTUAL_SVR_TBL );
	for( i=0;i<total;i++ )
	{
		if( !mib_chain_get( MIB_VIRTUAL_SVR_TBL, i, (void*)c ) )
			continue;

		if( (c->InstanceNum==instnum) )
		{
			*chainID = i;
     			return 0;
     		}
	}

	return -1;
}
/*end portmapping utilities*/

#elif defined PORT_FORWARD_GENERAL
/*portmapping utilities*/
unsigned int getPortMappingMaxInstNum( unsigned int ifindex )
{
	unsigned int ret = 0;
	MIB_CE_PORT_FW_T *p, port_entity;
	unsigned int total,i;

	total = mib_chain_total( MIB_PORT_FW_TBL );
	for( i=0;i<total;i++ )
	{
		p = &port_entity;
		if( mib_chain_get( MIB_PORT_FW_TBL, i, (void*)p ) )
		{
			if(p->ifIndex==ifindex)
			{
				if(p->InstanceNum>ret) ret = p->InstanceNum;
			}
		}
	}

	return ret;
}

int getPortMappingCount( unsigned int ifindex )
{
	int count = 0;
	MIB_CE_PORT_FW_T *p, port_entity;
	unsigned int total,i;

	total = mib_chain_total( MIB_PORT_FW_TBL );
	for( i=0;i<total;i++ )
	{
		p = &port_entity;
		if( mib_chain_get( MIB_PORT_FW_TBL, i, (void*)p ) )
			if(p->ifIndex==ifindex) count++;
	}

	return count;
}

/*id starts from 0, 1, 2, ...*/
int getPortMappingByID( unsigned int ifindex, int id, MIB_CE_PORT_FW_T *c, unsigned int *chainID )
{
	unsigned int total,i;

	if( (id < 0) || (c==NULL) || (chainID==NULL) ) return -1;

	total = mib_chain_total( MIB_PORT_FW_TBL );
	for( i=0;i<total;i++ )
	{
		if( !mib_chain_get( MIB_PORT_FW_TBL, i, (void*)c ) )
			continue;

		if(c->ifIndex==ifindex)
		{
			id--;
			if(id==-1)
			{
				*chainID = i;
				return 0;
			}
		}
	}

	return -1;
}

int getPortMappingByInstNum( unsigned int ifindex, unsigned int instnum, MIB_CE_PORT_FW_T *c, unsigned int *chainID )
{
	unsigned int total,i;

	if( (instnum == 0) || (c==NULL) || (chainID==NULL) ) return -1;

	total = mib_chain_total( MIB_PORT_FW_TBL );
	for( i=0;i<total;i++ )
	{
		if( !mib_chain_get( MIB_PORT_FW_TBL, i, (void*)c ) )
			continue;

		if( (c->ifIndex==ifindex)&&(c->InstanceNum==instnum) )
		{
			*chainID = i;
     			return 0;
     		}
	}

	return -1;
}
/*end portmapping utilities*/
#endif

/*for DefaultConnectionService, set/get default route */
int getDefaultRoute( char *name )
{
	int total,i;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;

	if( name==NULL ) return -1;
	name[0]=0;

#ifdef DEFAULT_GATEWAY_V2
	{
		unsigned int dgw;
		if (mib_get(MIB_ADSL_WAN_DGW_ITF, (void *)&dgw) != 0)
		{
			transfer2PathName( dgw, name );
		}
	}
#else
	total = mib_chain_total(MIB_ATM_VC_TBL);
	for( i=0; i<total; i++ )
	{
		pEntry = &vc_entity;
		if( !mib_chain_get(MIB_ATM_VC_TBL, i, (void*)pEntry ) )
			continue;
		if(pEntry->dgw==1)
		{
			//char strfmt[]="InternetGatewayDevice.WANDevice.1.WANConnectionDevice.%d.%s.%d"; //wt-121v8-3.33, no trailing dot
			char strfmt[]="InternetGatewayDevice.WANDevice.%u.WANConnectionDevice.%u.%s.%u"; //wt-121v8-3.33, no trailing dot
			char ipstr[]="WANIPConnection";
			char pppstr[]="WANPPPConnection";
			char *pconn=NULL;
			unsigned int instnum=0;

			if( (pEntry->cmode==CHANNEL_MODE_PPPOE) ||
#ifdef PPPOE_PASSTHROUGH
			    ((pEntry->cmode==CHANNEL_MODE_BRIDGE)&&(pEntry->brmode==BRIDGE_PPPOE)) ||
#endif
			    (pEntry->cmode==CHANNEL_MODE_PPPOA) )
			{
				pconn = pppstr;
				instnum = pEntry->ConPPPInstNum;
			}else{
				pconn = ipstr;
				instnum = pEntry->ConIPInstNum;
			}

			if( pEntry->connDisable==0 )
			{
				unsigned int waninst;

				waninst=getWanInstFromifIndex(pEntry->ifIndex);
				//sprintf( name, strfmt, pEntry->ConDevInstNum , pconn, instnum );
				sprintf( name, strfmt, waninst, pEntry->ConDevInstNum , pconn, instnum );
				break;
			}else
				return -1;

		}
	}
#endif
	//name = InternetGatewayDevice.WANDevice.1.WANConnection.2.WANPPPConnection.1.
	return 0;
}

int setDefaultRoute( char *name )
{
	//name = InternetGatewayDevice.WANDevice.1.WANConnection.2.WANPPPConnection.1.
	MIB_CE_ATM_VC_T *pEntry, vc_entity;
	struct CWMP_LEAF *e=NULL;
	unsigned int wandevnum, devnum,ipnum, pppnum;
	int total,i;

	if(name==NULL) return -1;
	if( get_ParameterEntity( name, &e ) < 0 ) return -1;

#ifdef DEFAULT_GATEWAY_V2
	{
		unsigned int dgw;
		dgw = transfer2IfIndex( name );
		if( dgw==DUMMY_IFINDEX ) return -1;
		mib_set(MIB_ADSL_WAN_DGW_ITF, (void *)&dgw);
	}
#else
	wandevnum = getWANDevInstNum(name);
	devnum = getWANConDevInstNum( name );
	ipnum  = getWANIPConInstNum( name );
	pppnum = getWANPPPConInstNum( name );
	if( wandevnum==0 || (devnum==0) || ((ipnum==0)&&(pppnum==0)) ) return -1;

	//delete old/update new default route
	total = mib_chain_total(MIB_ATM_VC_TBL);
#ifdef CONFIG_E8B
	for( i=0; i<total; i++ )
	{
		pEntry = &vc_entity;
		if( !mib_chain_get(MIB_ATM_VC_TBL, i, (void*)pEntry ) )
			continue;

		if(!isWANDevModeMatch(wandevnum, pEntry->ifIndex))
			continue;

		if( (pEntry->ConDevInstNum==devnum) &&
		    (pEntry->ConIPInstNum==ipnum) &&
		    (pEntry->ConPPPInstNum==pppnum) ) //new default route
		{
			if(!(pEntry->applicationtype&X_CT_SRV_INTERNET)) // Only INTERNET connection can be default connection
				return -1;
		}
	}
#endif
	for( i=0; i<total; i++ )
	{
		pEntry = &vc_entity;
		if( !mib_chain_get(MIB_ATM_VC_TBL, i, (void*)pEntry ) )
			continue;

		//for different media_type, pEntry->dgw field won't be reset
		//if(!isWANDevModeMatch(wandevnum, pEntry->ifIndex))
		//	continue;

		if( (getMediaTypeFromWanInst(wandevnum)==MEDIA_INDEX(pEntry->ifIndex))&&
			(pEntry->ConDevInstNum==devnum) &&
		    (pEntry->ConIPInstNum==ipnum) &&
		    (pEntry->ConPPPInstNum==pppnum) ) //new default route
		{
			pEntry->dgw=1;
			mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, i );
		}else if(pEntry->dgw==1) //old default route
		{
			pEntry->dgw=0;
			mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, i );
		}
	}
#endif
	return 0;
}

int transfer2IfName( char *name, char *ifname )
{
	struct CWMP_LEAF *e=NULL;

	if( (name==NULL) || ( ifname==NULL ) ) return -1;
	if( get_ParameterEntity( name, &e ) < 0 ) return -1;
	ifname[0]=0;

	if( strcmp( name, "InternetGatewayDevice.LANDevice.1" )==0 )
		strcpy( ifname, "br0" );
	else if( strcmp( name, "InternetGatewayDevice.LANDevice.1.LANHostConfigManagement.IPInterface.1" )==0 )
		strcpy( ifname, "br0" );
#ifdef CONFIG_RTL_MULTI_LAN_DEV
#ifdef CONFIG_E8B
	else if( strcmp( name, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.4" )==0 )
		strcpy( ifname, "eth0.5" );
	else if( strcmp( name, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.3" )==0 )
		strcpy( ifname, "eth0.4" );
	else if( strcmp( name, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.2" )==0 )
		strcpy( ifname, "eth0.3" );
	else if( strcmp( name, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.1" )==0 )
		strcpy( ifname, "eth0.2" );
#else
	else if( strcmp( name, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.4" )==0 )
		strcpy( ifname, "eth0.2" );
	else if( strcmp( name, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.3" )==0 )
		strcpy( ifname, "eth0.3" );
	else if( strcmp( name, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.2" )==0 )
		strcpy( ifname, "eth0.4" );
	else if( strcmp( name, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.1" )==0 )
		strcpy( ifname, "eth0.5" );
#endif
#else
	else if( strcmp( name, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.1" )==0 )
		strcpy( ifname, "eth0" );
#endif
#ifdef WLAN_SUPPORT
	else if( strcmp( name, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.1" )==0 )
		strcpy( ifname, wlan_name[0] );
#ifdef WLAN_MBSSID
	else if( strcmp( name, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.2" )==0 )
		strcpy( ifname, wlan_name[1] );
	else if( strcmp( name, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.3" )==0 )
		strcpy( ifname, wlan_name[2] );
	else if( strcmp( name, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.4" )==0 )
		strcpy( ifname, wlan_name[3] );
	else if( strcmp( name, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.5" )==0 )
		strcpy( ifname, wlan_name[4] );
#endif //WLAN_MBSSID
#endif //WLAN_SUPPORT
	else //wan interface
	{
		unsigned int wandevnum, devnum,ipnum,pppnum;
		MIB_CE_ATM_VC_T *pEntry,vc_entity;
		int total,i;

		wandevnum = getWANDevInstNum(name);
		devnum = getWANConDevInstNum( name );
		ipnum  = getWANIPConInstNum( name );
		pppnum = getWANPPPConInstNum( name );

		if( wandevnum==0 || (devnum==0) || ((ipnum==0)&&(pppnum==0)) ) return -1;

		total = mib_chain_total(MIB_ATM_VC_TBL);
		for( i=0; i<total; i++ )
		{
			pEntry = &vc_entity;
			if( !mib_chain_get(MIB_ATM_VC_TBL, i, (void*)pEntry ) )
				continue;

			if(!isWANDevModeMatch(wandevnum, pEntry->ifIndex))
				continue;

			if( (pEntry->ConDevInstNum==devnum) &&
			    (pEntry->ConIPInstNum==ipnum) &&
			    (pEntry->ConPPPInstNum==pppnum) )
			{
				if(MEDIA_ATM==MEDIA_INDEX(pEntry->ifIndex))
				{
					if(pEntry->cmode == CHANNEL_MODE_PPPOE || pEntry->cmode == CHANNEL_MODE_PPPOA)
						sprintf( ifname, "ppp%u", PPP_INDEX(pEntry->ifIndex)  );
					else
						sprintf( ifname, "vc%u", VC_INDEX(pEntry->ifIndex)  );
				}else{ //for MEDIA_ETH/CONFIG_ETHWAN, MEDIA_PTM/CONFIG_PTMWAN
					ifGetName(pEntry->ifIndex, ifname, IFNAMSIZ);
				}
				break;
			}
		}
	}
	return 0;
}

unsigned int transfer2IfIndxfromIfName( char *ifname )
{
	MIB_CE_ATM_VC_T *pEntry,vc_entity;
	int total,i;
	unsigned int ifindex=DUMMY_IFINDEX;

	if(ifname==NULL) return ifindex;
	total = mib_chain_total(MIB_ATM_VC_TBL);
	for( i=0; i<total; i++ )
	{
		char tmp_ifname[32];
		pEntry = &vc_entity;
		if( !mib_chain_get(MIB_ATM_VC_TBL, i, (void*)pEntry ) )
			continue;

		//ifGetName(tmp_ifname, pEntry->ifIndex, sizeof(tmp_ifname));
		ifGetName(pEntry->ifIndex, tmp_ifname, sizeof(tmp_ifname));

		if( strcmp(ifname, tmp_ifname)==0 )
		{
			ifindex=pEntry->ifIndex;
			break;
		}
	}

	return ifindex;
}


int transfer2PathNamefromItf( char *ifname, char *pathname )
{
	//struct CWMP_LEAF *e=NULL;

	if( (ifname==NULL) || ( pathname==NULL ) ) return -1;
	pathname[0]=0;

	//if( strcmp( ifname, "br0" )==0 )
	//	strcpy( pathname, "InternetGatewayDevice.LANDevice.1" );
	//else
	if( strcmp( ifname, "br0" )==0 )
		strcpy( pathname, "InternetGatewayDevice.LANDevice.1.LANHostConfigManagement.IPInterface.1" );
#ifdef CONFIG_RTL_MULTI_LAN_DEV
  #ifdef CONFIG_E8B
	else if( strcmp( ifname, "eth0.5" )==0 )
		strcpy( pathname, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.4" );
	else if( strcmp( ifname, "eth0.4" )==0 )
		strcpy( pathname, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.3" );
	else if( strcmp( ifname, "eth0.3" )==0 )
		strcpy( pathname, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.2" );
	else if( strcmp( ifname,  "eth0.2" )==0 )
		strcpy( pathname, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.1" );
  #else  //CONFIG_E8B
	else if( strcmp( ifname, "eth0.2" )==0 )
		strcpy( pathname, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.4" );
	else if( strcmp( ifname, "eth0.3" )==0 )
		strcpy( pathname, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.3" );
	else if( strcmp( ifname, "eth0.4" )==0 )
		strcpy( pathname, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.2" );
	else if( strcmp( ifname,  "eth0.5" )==0 )
		strcpy( pathname, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.1" );
  #endif //CONFIG_E8B
#else
	else if( strcmp( ifname, "eth0" )==0 )
		strcpy( pathname, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.1" );
#endif
#ifdef WLAN_SUPPORT
	else if( strcmp( ifname, wlan_name[0])==0 )
		strcpy( pathname, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.1" );
#ifdef WLAN_MBSSID
	else if( strcmp( ifname, wlan_name[1])==0 )
		strcpy( pathname, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.2" );
	else if( strcmp( ifname, wlan_name[2])==0 )
		strcpy( pathname, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.3" );
	else if( strcmp( ifname, wlan_name[3])==0 )
		strcpy( pathname, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.4" );
	else if( strcmp( ifname, wlan_name[4])==0 )
		strcpy( pathname, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.5" );
#endif //WLAN_MBSSID
#endif //WLAN_SUPPORT
	else //wan interface
	{
		MIB_CE_ATM_VC_T *pEntry,vc_entity;
		int total,i;
		unsigned int ifindex=DUMMY_IFINDEX;

		total = mib_chain_total(MIB_ATM_VC_TBL);
		for( i=0; i<total; i++ )
		{
			char tmp_ifname[32];
			pEntry = &vc_entity;
			if( !mib_chain_get(MIB_ATM_VC_TBL, i, (void*)pEntry ) )
				continue;

			ifGetName(pEntry->ifIndex, tmp_ifname, sizeof(tmp_ifname));

			if( strcmp(ifname, tmp_ifname)==0 )
			{
				ifindex=pEntry->ifIndex;
				break;
			}
		}
		return transfer2PathName( ifindex, pathname );
	}
	return 0;
}

unsigned int transfer2IfIndex( char *name )
{
	struct CWMP_LEAF *e=NULL;
	unsigned int ret=DUMMY_IFINDEX;

	if( name==NULL ) return ret;
	if( get_ParameterEntity( name, &e ) < 0 ) return ret;

	{
		unsigned int wandevnum, devnum,ipnum,pppnum;
		MIB_CE_ATM_VC_T *pEntry,vc_entity;
		int total,i;

		wandevnum = getWANDevInstNum(name);
		devnum = getWANConDevInstNum( name );
		ipnum  = getWANIPConInstNum( name );
		pppnum = getWANPPPConInstNum( name );
		if( wandevnum==0 || (devnum==0) || ((ipnum==0)&&(pppnum==0)) ) return -1;

		total = mib_chain_total(MIB_ATM_VC_TBL);
		for( i=0; i<total; i++ )
		{
			pEntry = &vc_entity;
			if( !mib_chain_get(MIB_ATM_VC_TBL, i, (void*)pEntry ) )
				continue;

			if(!isWANDevModeMatch(wandevnum, pEntry->ifIndex))
				continue;

			if( (pEntry->ConDevInstNum==devnum) &&
			    (pEntry->ConIPInstNum==ipnum) &&
			    (pEntry->ConPPPInstNum==pppnum) )
			{
				ret = pEntry->ifIndex;
				break;
			}
		}
	}
	return ret;
}

int transfer2PathName( unsigned int ifindex, char *name )
{
	int total,i;
	MIB_CE_ATM_VC_T *pEntry, vc_entity;

	if( ifindex==DUMMY_IFINDEX ) return -1;
	if( name==NULL ) return -1;
	name[0]=0;

	total = mib_chain_total(MIB_ATM_VC_TBL);
	for( i=0; i<total; i++ )
	{
		pEntry = &vc_entity;
		if( !mib_chain_get(MIB_ATM_VC_TBL, i, (void*)pEntry ) )
			continue;
		if(pEntry->ifIndex==ifindex)
		{
			//char strfmt[]="InternetGatewayDevice.WANDevice.%d.WANConnectionDevice.%d.%s.%d"; //wt-121v8-3.33, no trailing dot
			char strfmt[]="InternetGatewayDevice.WANDevice.%u.WANConnectionDevice.%u.%s.%u"; //wt-121v8-3.33, no trailing dot
			char ipstr[]="WANIPConnection";
			char pppstr[]="WANPPPConnection";
			char *pconn=NULL;
			unsigned int instnum=0;

			if( (pEntry->cmode==CHANNEL_MODE_PPPOE) ||
#ifdef PPPOE_PASSTHROUGH
			    ((pEntry->cmode==CHANNEL_MODE_BRIDGE)&&(pEntry->brmode==BRIDGE_PPPOE)) ||
#endif
			    (pEntry->cmode==CHANNEL_MODE_PPPOA) )
			{
				pconn = pppstr;
				instnum = pEntry->ConPPPInstNum;
			}else{
				pconn = ipstr;
				instnum = pEntry->ConIPInstNum;
			}

			if( pEntry->connDisable==0 )
			{
				unsigned int waninst;
				waninst=getWanInstFromifIndex(pEntry->ifIndex);
				sprintf( name, strfmt, waninst, pEntry->ConDevInstNum , pconn, instnum );

				break;
			}else
				return -1;
		}
	}
	//name = InternetGatewayDevice.WANDevice.1.WANConnection.2.WANPPPConnection.1.
	return 0;
}

int getRIPInfo( unsigned int ifIndex, unsigned char *ripmode )
{

	unsigned char ripOn=0;

	if( ripmode==NULL ) return -1;

	*ripmode = RIP_NONE;
	mib_get(MIB_RIP_ENABLE, (void *)&ripOn);
	if( ripOn!=0 )
	{
		unsigned int entryNum=0, i=0;
		MIB_CE_RIP_T Entry;
		entryNum = mib_chain_total(MIB_RIP_TBL);
		for( i=0; i<entryNum; i++ )
		{
			if (!mib_chain_get(MIB_RIP_TBL, i, (void *)&Entry))
				continue;
			if(Entry.ifIndex==ifIndex)
			{
				//read receiveMode.
				*ripmode = Entry.receiveMode;

				break;
			}
		}
	}

	return 0;

}
int updateRIPInfo( unsigned int ifIndex, unsigned char newripmode )
{

	unsigned int entryNum=0, i=0, found=0;
	unsigned char vChar=0;
	MIB_CE_RIP_T Entry;
	entryNum = mib_chain_total(MIB_RIP_TBL);
	for( i=0; i<entryNum; i++ )
	{
		if (!mib_chain_get(MIB_RIP_TBL, i, (void *)&Entry))
			continue;
		if(Entry.ifIndex==ifIndex)
		{
			found = 1;
			break;
		}
	}

	if(found)
	{
		//receiveMode can only accept: none, v1, v2, v1_v2
		switch( newripmode )
		{
		case RIP_NONE:
			if(Entry.sendMode==RIP_NONE)
			{
				mib_chain_delete(MIB_RIP_TBL, i);
				entryNum = mib_chain_total(MIB_RIP_TBL);
				if(entryNum==0)
				{
					vChar = 0;
					mib_set(MIB_RIP_ENABLE, (void *)&vChar);
				}
			}else{
				// Mason Yu. Do not use version, set sendmode and receive mode together.
				Entry.sendMode = newripmode;
				Entry.receiveMode = newripmode;
				mib_chain_update( MIB_RIP_TBL, (unsigned char*)&Entry, i );
			}
			break;
		case RIP_V1:
		case RIP_V2:
		case RIP_V1_V2:
			// Mason Yu. Do not use version, set sendmode and receive mode together.
			Entry.sendMode = newripmode;
			Entry.receiveMode = newripmode;
			mib_chain_update( MIB_RIP_TBL, (unsigned char*)&Entry, i );
			vChar = 1;
			mib_set(MIB_RIP_ENABLE, (void *)&vChar);

			break;
		default:
			return -1;
		}
	}else{
		//receiveMode can only accept: none, v1, v2, v1_v2
		switch( newripmode )
		{
		case RIP_NONE:
			//nothing to do
			break;
		case RIP_V1:
		case RIP_V2:
		case RIP_V1_V2:
			memset( &Entry, 0, sizeof(Entry) );
			Entry.ifIndex = ifIndex;
			// Mason Yu. Do not use version, set sendmode and receive mode together.
			//Entry.sendMode = RIP_NONE;
			Entry.sendMode = newripmode;
			Entry.receiveMode = newripmode;
			mib_chain_add( MIB_RIP_TBL, (unsigned char*)&Entry );
			vChar = 1;
			mib_set(MIB_RIP_ENABLE, (void *)&vChar);

			break;
		default:
			return -1;
		}
	}

	return 0;

}

#ifdef CONFIG_PPP
/*ppp utilities*/
static int getPPPUptime(char *pppname, int ppptype, unsigned int *uptime)
{
	char buff[256], *fname;
	FILE *fp = NULL;
	char strif[6], tmpst[64] = { 0 };
	int spid = 0;

	if ((pppname == NULL) || (uptime == NULL) ||
	    ((ppptype != CHANNEL_MODE_PPPOE) && (ppptype != CHANNEL_MODE_PPPOA)))
		return -1;

	// get spppd pid
	if ((fp = fopen(PPP_PID, "r"))) {
		fscanf(fp, "%d\n", &spid);
		fclose(fp);
	} else {
		fprintf(stderr, "spppd pidfile not exists\n");
		return -1;
	}
	if (spid)
		kill(spid, SIGUSR2);
	usleep(500);

	*uptime = 0;
	if (ppptype == CHANNEL_MODE_PPPOE)
		fname = (char *)PPPOE_CONF;
	else
		fname = (char *)PPPOA_CONF;
	if (!(fp = fopen(fname, "r"))) {
		fprintf(stderr, "%s not exists.\n", fname);
		return -1;
	} else {
		fgets(buff, sizeof(buff), fp);
		while (fgets(buff, sizeof(buff), fp) != NULL) {
			if (ppptype == CHANNEL_MODE_PPPOE) {	//if      dev     phase           MAC          AC_MAC       uptime      totaluptime
				if (sscanf
				    (buff, "%s %*s %*s %*s %*s %s", strif,
				     tmpst) != 2) {
					fprintf(stderr,
						"Unsuported pppoe configuration format\n");
					break;
				}
			} else {	//if   dev     class  encaps qos  pcr   scr   mbs   uptime           totaluptime
				if (sscanf
				    (buff, "%s %*s %*s %*s %*s %*s %*s %*s %s",
				     strif, tmpst) != 2) {
					fprintf(stderr,
						"Unsuported pppoa configuration format\n");
					break;
				}
			}

			if (!strcmp(pppname, strif)) {
				break;
			}
		}
		fclose(fp);
	}

	{			//uptime format in the conf file    XXday(s),hh:mm:ss or 0sec
		if (strlen(tmpst) == 0)
			return -1;

		if (strcmp(tmpst, "0sec")) {
			int day = 0, hh = 0, mm = 0, ss = 0;
			char *tok1, *tok2;

			tok1 = strtok(tmpst, "days,");
			tok2 = strtok(NULL, "days,");
			if (tok2 != NULL)	//has days
				day = atoi(tok1);
			else
				tok2 = tok1;

			tok1 = strtok(tok2, ":");
			if (tok1)
				hh = atoi(tok1);
			tok1 = strtok(NULL, ":");
			if (tok1)
				mm = atoi(tok1);
			tok1 = strtok(NULL, ":");
			if (tok1)
				ss = atoi(tok1);

			//convert to seconds
			*uptime =
			    (unsigned int)((day * 24 + hh) * 60 + mm) * 60 + ss;
		} else
			*uptime = 0;

	}
	return 0;
}

/*end ppp utility*/
#endif

#ifdef CONFIG_DEV_xDSL
int getATMCellCnt( unsigned char vpi, unsigned short vci, unsigned int *txcnt, unsigned int *rxcnt )
{
	int skfd, i;
	struct atmif_sioc mysio;
	struct SAR_IOCTL_CFG cfg;
	//struct ch_stat stat;


	if( txcnt==NULL || rxcnt==NULL ) return -1;
	*txcnt=0; *rxcnt=0;

	// pvc statistics
	if((skfd = socket(PF_ATMPVC, SOCK_DGRAM, 0)) < 0){
		perror("socket open error");
		return -1;
	}

	mysio.number = 0;

	for (i=0; i < MAX_VC_NUM; i++)
	{
		cfg.ch_no = i;
		mysio.arg = (void *)&cfg;
		if(ioctl(skfd, ATM_SAR_GETSTAT, &mysio)<0)
		{
			(void)close(skfd);
			return -1;
		}

		if (cfg.created == 0)
			continue;


		if( cfg.vpi==vpi && cfg.vci==vci )
		{
			*txcnt = cfg.stat.tx_byte_cnt/48;
			*rxcnt = cfg.stat.rx_byte_cnt/48;
			break;
		}
	}
	(void)close(skfd);

	return 0;
}
#endif

int getFirstATMVCEntryByInstNum( unsigned int wandevnum, unsigned int condevnum, MIB_CE_ATM_VC_T *p, unsigned int *id )
{
	int ret=-1;
	unsigned int i,num;

	if( (wandevnum==0) || (condevnum==0) || (p==NULL) || (id==NULL) )
		return ret;

	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)p ) )
			continue;

		if(!isWANDevModeMatch(wandevnum, p->ifIndex))
			continue;

		if( p->ConDevInstNum==condevnum )
		{
			*id = i;
			ret = 0;
			break;
		}
	}
	return ret;
}

int getATMVCEntry(char *name, MIB_CE_ATM_VC_T *p, unsigned int *idx)
{
	int ret = -1;
	unsigned int i,num, wandev, devcon, ip, ppp;

	if( name == NULL )
		return ret;

	wandev = getWANDevInstNum(name);
	devcon= getWANConDevInstNum(name);
	ip = getWANIPConInstNum(name);
	ppp = getWANPPPConInstNum(name);

	if(wandev == 0 || devcon == 0 || (ip == 0 && ppp == 0))
		return ret;

	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)p ) )
			continue;

		if(!isWANDevModeMatch(wandev, p->ifIndex))
			continue;

		//fix for multi conppp/ip per ConDevInstNum
		//if((p->ConDevInstNum == devcon) && (p->ConPPPInstNum == ppp || p->ConIPInstNum == ip))
		if((p->ConDevInstNum == devcon) && (p->ConPPPInstNum == ppp) && (p->ConIPInstNum == ip))
		{
			*idx = i;
			ret = 0;
			break;
		}
	}
	return ret;
}

int resetATMVCConnection( MIB_CE_ATM_VC_T *p )
{
	MIB_CE_ATM_VC_T bkentry;

	if(p==NULL) return -1;

	memset( &bkentry, 0, sizeof(MIB_CE_ATM_VC_T) );
	bkentry.ifIndex = p->ifIndex;
	bkentry.vpi = p->vpi;
	bkentry.qos = p->qos;
	bkentry.vci = p->vci;
	bkentry.pcr = p->pcr;
	bkentry.scr = p->scr;
	bkentry.mbs = p->mbs;
	bkentry.cdvt = p->cdvt;
	bkentry.encap = p->encap;
	bkentry.cmode = p->cmode;
	bkentry.brmode = p->brmode;
	bkentry.mtu = p->mtu;
	bkentry.enable = p->enable;
	bkentry.vlan = p->vlan;
	bkentry.vid = p->vid;
	bkentry.connDisable = 1;
	bkentry.ConDevInstNum = p->ConDevInstNum;
#ifdef CONFIG_USER_WT_146
	wt146_copy_config( &bkentry, p );
#endif //CONFIG_USER_WT_146
#ifdef CONFIG_IPV6
	bkentry.IpProtocol=IPVER_IPV4;//ipv4 as default
#endif /*CONFIG_IPV6*/
	memcpy(bkentry.MacAddr, p->MacAddr, MAC_ADDR_LEN);

	memcpy( p, &bkentry, sizeof(MIB_CE_ATM_VC_T) );

	return 0;
}

