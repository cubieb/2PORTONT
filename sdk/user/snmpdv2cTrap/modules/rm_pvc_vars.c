

#include 	<net/if.h>
#include	<stdio.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<time.h>

#include	"ctypes.h"
#include	"error.h"
#include	"local.h"
#include	"mix.h"
#include	"mis.h"
#include	"asn.h"

#include 	"mib_tool.h"

//#include "../../boa/src/LINUX/mib.h"
#include <semaphore.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/if_ether.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>


unsigned int entryNumPVC;
struct eoc_create_pvcEncap eocAddPvcUpEncap;
extern int g_initCpePvcTable;

enum snmp_cpe_pvctable
{ 
   CPEPVCID = 1,
   CPEPVCIFINDEX = 3,
   CPEPVCUPPERENCAPSULATION,
   CPEPVCADMINSTATUS,
   CPEPVCOPERSTATUS,
   CPEPVCBRIDGEMODE,
   CPEPVCMACLEARNMODE,
   CPEPVCIGMPMODE,
   CPEDHCPCLIENTMODE,
   CPEPVCNATMODE,
   CPE_PVCTABLE_END 
};


static struct mib_oid_tbl pvc_mib_oid_tbl;
static long pvc_last_scan = 0;	

static	AsnIdType	pvcRetrieveMibValue (unsigned int mibIdx, int inputvpi, int inputvci)
{
	struct mib_oid * oid_ptr;
	unsigned long value;
	char *string;		
	int fd;
	struct ifreq    ifrq;	
	struct sysinfo info;
	//MIB_CE_ATM_VC_Tp pEntry, tmp_pEntry;	
	MIB_CE_ATM_VC_T Entry, tmp_Entry;
	char vpi[6], vci[6];
	char tmp_pEntry_vpi[6], tmp_pEntry_vci[6];
	char ifname[6];
	int flags, strStatus;
	CONN_T conn_status;
	int i, mode, natmode, dhcpcmode;
	unsigned char igmpEnable;
	unsigned int igmpItf;	
	
	oid_ptr = pvc_mib_oid_tbl.oid;
	oid_ptr += mibIdx;			
	
	//printf("pvcGet: inputvpi = %d  inputvci = %d\n", inputvpi, inputvci);
	// To find the correct pEntry by VPI, VCI	
	for (i=0; i<entryNumPVC; i++){
		//tmp_pEntry = (MIB_CE_ATM_VC_Tp) mib_chain_get(MIB_ATM_VC_TBL, i);
		mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&tmp_Entry);	
		snprintf(tmp_pEntry_vpi, 6, "%u", tmp_Entry.vpi);
		snprintf(tmp_pEntry_vci, 6, "%u", tmp_Entry.vci);		
		
		if ( (atoi(tmp_pEntry_vpi) != inputvpi) || (atoi(tmp_pEntry_vci) != inputvci)  )
			continue;
		else if ( (atoi(tmp_pEntry_vpi) == inputvpi) && (atoi(tmp_pEntry_vci) == inputvci)  ){
			//pEntry = tmp_pEntry;			
			memcpy(&Entry, &tmp_Entry, sizeof(MIB_CE_ATM_VC_T));
			break;
		}	
	
	}		
	
	
	// Retrieve VPI/VCI
	snprintf(vpi, 6, "%u", Entry.vpi);
	snprintf(vci, 6, "%u", Entry.vci);
	
	// Retrive UpperEncapsulation
	mode=0;
	if (Entry.cmode == CHANNEL_MODE_PPPOE){
		mode = 2;		
	}else if (Entry.cmode == CHANNEL_MODE_PPPOA)
		mode = 1;
	else if (Entry.cmode == CHANNEL_MODE_BRIDGE)
		mode = 4;
	else if (Entry.cmode == CHANNEL_MODE_IPOE)
		mode = 5;
	else if (Entry.cmode == CHANNEL_MODE_RT1483)
		mode = 3;
	
	
	
	// Retrieve AdminStatus and OperStatus
	if (Entry.cmode == CHANNEL_MODE_PPPOE || Entry.cmode == CHANNEL_MODE_PPPOA){
		snprintf(ifname, 6, "ppp%u", PPP_INDEX(Entry.ifIndex));
		
		// set status flag
		if (Entry.enable == 0)
		{	
			strStatus = 2;		
			conn_status = CONN_DOWN;
		}
		else
		if (getInFlags( ifname, &flags) == 1)
		{
			if (flags & IFF_UP)			
			{
				strStatus = 1;
				conn_status = CONN_UP;
			}
			else
			{
#ifdef CONFIG_PPP
				if (find_ppp_from_conf(ifname))
				{
					strStatus = 1;
					conn_status = CONN_DOWN;
				}
				else
#endif
				{
					strStatus = 1;
					conn_status = CONN_NOT_EXIST;
				}
			}
		}
		else
		{
			strStatus = 1;
			conn_status = CONN_NOT_EXIST;
		}
		
		
	}else {
		snprintf(ifname, 5, "vc%u", VC_INDEX(Entry.ifIndex));
		
		// set status flag
		if (Entry.enable == 0)		
		{	
			strStatus = 2;				
			conn_status = CONN_DOWN;
		}
		else
		if (getInFlags( ifname, &flags) == 1)
		{
			if (flags & IFF_UP)
			{
				strStatus = 1;
				conn_status = CONN_UP;
			}
			else
			{
				strStatus = 1;
				conn_status = CONN_DOWN;
			}
		}
		else
		{
			strStatus = 1;
			conn_status = CONN_NOT_EXIST;
		}

	}		
	
	
	// Retrieve NAT Mode
	if (Entry.napt == 0 || Entry.cmode == CHANNEL_MODE_BRIDGE )
		natmode = 2;   // Disable
	else
		natmode = 1;   // Enable		
	
	
	// Retrieve DHCP Client Mode
	if (Entry.ipDhcp == (char)DHCP_CLIENT)
		dhcpcmode = 1;     // Enable
	else
		dhcpcmode = 2;	   // Disable
	
	
	
	switch (oid_ptr->name[0])
	{
		
	case CPEPVCID:									
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, VC_INDEX(Entry.ifIndex) );  		
		
	case CPEPVCIFINDEX:		
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, 150000+VC_INDEX(Entry.ifIndex) );	
		
	case CPEPVCUPPERENCAPSULATION:
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, mode);
		
	case CPEPVCADMINSTATUS:
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, strStatus);
					
	case CPEPVCOPERSTATUS:
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, conn_status);
			
	case CPEPVCBRIDGEMODE:
		if ( Entry.cmode != CHANNEL_MODE_BRIDGE ) 	
			return asnUnsl (asnClassUniversal, (AsnTagType) 2, 2);   // enable:1 , Disable:2
		else if ( Entry.brmode != BRIDGE_ETHERNET )
			return asnUnsl (asnClassUniversal, (AsnTagType) 2, 2);   // enable:1 , Disable:2
		else
			return asnUnsl (asnClassUniversal, (AsnTagType) 2, 1);   // enable:1 , Disable:2
			
	case CPEPVCMACLEARNMODE:
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, 1);   // enable:1 , Disable:2
		
	case CPEPVCIGMPMODE:
		if (mib_get(MIB_IGMP_PROXY, (void *)&igmpEnable) != 0)
		{
			if (igmpEnable != 1)
				return asnUnsl (asnClassUniversal, (AsnTagType) 2, 2);   // enable:1 , Disable:2
			else {
				if (mib_get(MIB_IGMP_PROXY_ITF, (void *)&igmpItf) != 0)
				{
					if (igmpItf != DUMMY_IFINDEX){
						if ( igmpItf == Entry.ifIndex )
							return asnUnsl (asnClassUniversal, (AsnTagType) 2, 1);   // enable:1 , Disable:2
						else
							return asnUnsl (asnClassUniversal, (AsnTagType) 2, 2);   // enable:1 , Disable:2				
					}
					else
						return asnUnsl (asnClassUniversal, (AsnTagType) 2, 2);   // enable:1 , Disable:2
				}
			}								
		}	
		//return asnUnsl (asnClassUniversal, (AsnTagType) 2, 1);   // enable:1 , Disable:2
		
	case CPEDHCPCLIENTMODE:
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, dhcpcmode);   // enable:1 , Disable:2
		
	case CPEPVCNATMODE:	
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, natmode);   // enable:1 , Disable:2
	 																														
	default:
		//return ((AsnIdType) 0);
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, 1);
				
	}
	
}



void pvc_Mib_Init(void)
{
	struct timeval now;
	int j;
	int i, k;	
	//MIB_CE_ATM_VC_Tp pEntry;
	MIB_CE_ATM_VC_T Entry;	
	char vpi[6], vci[6];
	int initvpi;
	int initvci;
	
	
	gettimeofday(&now, 0);

//	printf("Now:%u:%u\n",now.tv_sec, now.tv_usec);

	//if((now.tv_sec - pvc_last_scan) > 10 )	//  10 sec
	if((now.tv_sec - pvc_last_scan) > 10 || g_initCpePvcTable == 1 )	//  10 sec
	{		
		
		unsigned int oid_tbl_size;
		struct mib_oid * oid_ptr;
		
		pvc_last_scan = now.tv_sec;			

		// create oid table
		free_mib_tbl(&pvc_mib_oid_tbl);		
		
		entryNumPVC = mib_chain_total(MIB_ATM_VC_TBL);			
		
		// pvc MIB has 11*entryNumPVC objects
		oid_tbl_size = 11;
		
		create_mib_tbl(&pvc_mib_oid_tbl,  oid_tbl_size*entryNumPVC, 6); 
		oid_ptr = pvc_mib_oid_tbl.oid;	
		
		
		for (j=1; j<=oid_tbl_size; j++) {
			initvpi = 0;
			initvci = 0;
			
			for (i=0; i<entryNumPVC; i++){
				//pEntry = vclTableSort(initvpi, initvci);
				vclTableSort(initvpi, initvci, &Entry);	
				snprintf(vpi, 6, "%u", Entry.vpi);
				snprintf(vci, 6, "%u", Entry.vci);				
				
				initvpi = atoi(vpi);
				initvci = atoi(vci);
								
				k= 96;                        // Hex:60
															
				oid_ptr->length = 6;
				oid_ptr->name[0] = j;
				oid_ptr->name[1] = 136;       // (1) Hex= 88
				oid_ptr->name[2] = 197;       // (2) Hex= c5
				oid_ptr->name[3] = k;         // (3) Hex= 60  the (1)0x88, (2)0xc5, (3)0x60 value is means 140000
				oid_ptr->name[4] = atoi(vpi);
				oid_ptr->name[5] = atoi(vci);	
				
				//for ( h=0; h<6; h++)
				//	printf("atmvcl_Mib_Init: oid_ptr->name[%d] = %d\n", h, oid_ptr->name[h]);
												
				oid_ptr++;				
			}	
		}
		
		// Set Init cpePvcTable flag = 0. for avoid get PVC so long
		if ( g_initCpePvcTable == 1 )
			g_initCpePvcTable = 0;	
		
				
	}//if((now.tv_sec - pvc_last_scan) > 10)	//  10 sec
}

static	MixStatusType	pvcRelease ()
{
	
	return (smpErrorReadOnly);
}

static	MixStatusType	pvcCreate ()
{
	
	return (smpErrorReadOnly);
}

static	MixStatusType	pvcDestroy ()
{
	
	return (smpErrorReadOnly);
}




static	AsnIdType	pvcNext (MixCookieType cookie, MixNamePtrType name, MixLengthPtrType namelenp)
{
	unsigned int idx=0;	
	pvc_Mib_Init();		
	
	cookie = cookie;	
		
	if(snmp_oid_getnext(&pvc_mib_oid_tbl, name, *namelenp, &idx))
	{		
		struct mib_oid * oid_ptr = pvc_mib_oid_tbl.oid;
		oid_ptr += idx;

		memcpy((unsigned char *)name, oid_ptr->name, oid_ptr->length);		
		*namelenp = oid_ptr->length;				
		
		return pvcRetrieveMibValue(idx, name[4], name[5]);
	}

	return ((AsnIdType) 0);
}


static	AsnIdType	pvcGet (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen)
{
	unsigned int idx;
	int i;

	//for ( i=0; i<6; i++)
	//		printf("pvcGet: name[%d] = %d\n", i, name[i]);	
			
	pvc_Mib_Init();
	
	cookie = cookie;
	if(snmp_oid_get(&pvc_mib_oid_tbl, name, namelen, &idx))
	{				
		return pvcRetrieveMibValue(idx, name[4], name[5]);		
	}
	// Mason Yu. If do PvcEncap and snmp agent can not get this OID. 
	// So we should retuen a valid value to pass the predure to pvcSet().
	else if ( name[0] == 4){
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, 1);	
	}
	else
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, 1);		
	
	return ((AsnIdType) 0);
	
}


static	MixStatusType	pvcSet  (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen, AsnIdType asn)
{
	unsigned int idx;	
	long i;	
	struct channel_conf_para para;
	unsigned char proxy;
	unsigned int proxy_if;
	MIB_CE_ATM_VC_T Entry;
	unsigned int igmpItf;
	
	pvc_Mib_Init();	
	
	cookie = cookie;
	name = name;
	namelen = namelen;
	asn = asn;	
	
	//for ( i=0; i<6; i++)
	//	printf("pvcSet: name[%d] = %d\n", i, name[i]);
			
	if(snmp_oid_get(&pvc_mib_oid_tbl, name, namelen, &idx))
	{	
		struct mib_oid * oid_ptr;				
		AsnLengthType		k;
		int option;				
		unsigned char strvalue[256];
	
		oid_ptr = pvc_mib_oid_tbl.oid;
		oid_ptr += idx;		
		
		
		switch (oid_ptr->name[0])
		{
		case CPEPVCUPPERENCAPSULATION:
			//printf("pvcSet: To do CPEPVCUPPERENCAPSULATION\n");
			i =  asnNumber (asnValue (asn), asnLength (asn));
			
			para.inputvpi=name[namelen-2];
			para.inputvci=name[namelen-1];
			para.cmode=i;
			para.admin=0;
			para.natmode=0;
			para.dhcpmode=0;
			para.pppIfIndex=0;	
			para.IpIndex=0;
			para.encap=100;
			para.brmode=100;
					
			modifyChannelConf(&para);			
			return (smpErrorNone);
		
		case CPEPVCADMINSTATUS:
			//printf("pvcSet: To do 5 or 7\n");
			i =  asnNumber (asnValue (asn), asnLength (asn));						
			
			para.inputvpi=name[namelen-2];
			para.inputvci=name[namelen-1];
			para.cmode=0;
			para.admin=i;
			para.natmode=0;
			para.dhcpmode=0;
			para.pppIfIndex=0;
			para.IpIndex=0;	
			para.encap=100;		
			para.brmode=100;
			
			modifyChannelConf(&para);			
			return (smpErrorNone);
		
		case CPEPVCBRIDGEMODE:	
			i =  asnNumber (asnValue (asn), asnLength (asn));	
					
			//pEntry=searchpEntrybyVpiVci(name[namelen-2], name[namelen-1]);
			searchpEntrybyVpiVci(name[namelen-2], name[namelen-1], (void *)&Entry);
			
			if ( Entry.cmode != CHANNEL_MODE_BRIDGE)
				return (smpErrorNone);			
			
			if ( i == 1 )      // Enable
				i = 0;
			else if ( i == 2 ) // Disable
				i = 2;
			else
				return (smpErrorNone);
					
			para.inputvpi=name[namelen-2];
			para.inputvci=name[namelen-1];
			para.cmode=0;
			para.admin=0;
			para.natmode=0;
			para.dhcpmode=0;
			para.pppIfIndex=0;	
			para.IpIndex=0;	
			para.encap=100;	
			para.brmode=i;
			
			modifyChannelConf(&para);	
			return (smpErrorNone);
		
		case CPEDHCPCLIENTMODE:		
			//printf("pvcSet: To do CPEDHCPCLIENTMODE\n");
			i =  asnNumber (asnValue (asn), asnLength (asn));			
			
			para.inputvpi=name[namelen-2];
			para.inputvci=name[namelen-1];
			para.cmode=0;
			para.admin=0;
			para.natmode=0;
			para.dhcpmode=i;
			para.pppIfIndex=0;	
			para.IpIndex=0;	
			para.encap=100;	
			para.brmode=100;
			
			modifyChannelConf(&para);
			return (smpErrorNone);
			
		case CPEPVCNATMODE:
			//printf("pvcSet: To do 11\n");
			i =  asnNumber (asnValue (asn), asnLength (asn));			
			
			para.inputvpi=name[namelen-2];
			para.inputvci=name[namelen-1];
			para.cmode=0;
			para.admin=0;
			para.natmode=i;
			para.dhcpmode=0;
			para.pppIfIndex=0;	
			para.IpIndex=0;	
			para.encap=100;	
			para.brmode=100;
			
			modifyChannelConf(&para);
			
			// Really Set pvcUperEncap for create PVC via EOC channel
			if ( eocAddPvcUpEncap.pvcEncapFlag == 1 ) {
				para.inputvpi=eocAddPvcUpEncap.vpi;
				para.inputvci=eocAddPvcUpEncap.vci;
				para.cmode=eocAddPvcUpEncap.pvcEncapValue;
				para.admin=0;
				para.natmode=0;
				para.dhcpmode=0;
				para.pppIfIndex=0;	
				para.IpIndex=0;
				para.encap=100;
				para.brmode=100;
					
				modifyChannelConf(&para);
				eocAddPvcUpEncap.pvcEncapFlag = 0;
			}			
			return (smpErrorNone);		
		
			
		case CPEPVCIGMPMODE:
			//printf("pvcSet: To do CPEPVCIGMPMODE\n");
			
			i =  asnNumber (asnValue (asn), asnLength (asn));
			//pEntry=searchpEntrybyVpiVci(name[namelen-2], name[namelen-1]);
			searchpEntrybyVpiVci(name[namelen-2], name[namelen-1], (void *)&Entry);
			
			if ( i == 1 ) {  // Enable IGMPProxy			
				proxy = 1;
				if ( !mib_set(MIB_IGMP_PROXY, (void *)&proxy)) {
					printf("Set IGMP proxy error(Enable)!\n");					
				}				
				
				proxy_if = Entry.ifIndex;
				if ( !mib_set(MIB_IGMP_PROXY_ITF, (void *)&proxy_if)) {
					printf("Set IGMP proxy interface index error!\n");					
				}
				
			}else {	  // Disable IGMPProxy
				/*
				proxy = 0;
				if ( !mib_set(MIB_IGMP_PROXY, (void *)&proxy)) {
					printf("Set IGMP proxy error(Disable)!\n");					
				}
				*/
				
				if (mib_get(MIB_IGMP_PROXY_ITF, (void *)&igmpItf) != 0){
					if ( (igmpItf != DUMMY_IFINDEX) && (igmpItf == Entry.ifIndex) ) {
						proxy_if = DUMMY_IFINDEX;
						if ( !mib_set(MIB_IGMP_PROXY_ITF, (void *)&proxy_if)) {
							printf("Set IGMP proxy interface index error(Default)!\n");					
						}						
					}						
				}			
				
			}		
			
			return (smpErrorNone);			
			
					
		default:
			return (smpErrorReadOnly);			
		}		
		
	}
	else if ( name[0] == 4 ){		
		i =  asnNumber (asnValue (asn), asnLength (asn));			
		//printf("pvcSet: This is PVCEncap Setting and value=%d\n", i);
		
		// namelen != 6 , that is the request come from PVC Channel. His ifindex just has 1 byte.
		if ( namelen != 6 ) {
			printf("pvcSet: Create PVC(vpi/vci=%d/%d) via PVC channel\n", name[namelen-2], name[namelen-1]);
			para.inputvpi=name[namelen-2];
			para.inputvci=name[namelen-1];
			para.cmode=i;
			para.admin=0;
			para.natmode=0;
			para.dhcpmode=0;
			para.pppIfIndex=0;	
			para.IpIndex=0;
			para.encap=100;
			para.brmode=100;
					
			modifyChannelConf(&para);	
		} 
		// Save pvcEncapFlag and pvcEncapValue for create PVC(ifindex=140000) via EOC chnnel
		// name[1] = 0x88 && name[2] == 0xc5, that is namelen = 6. The request come from EOC Channel
		else if ( name[1] = 0x88 && name[2] == 0xc5) {
			eocAddPvcUpEncap.vpi = name[namelen-2];
			eocAddPvcUpEncap.vci = name[namelen-1];
			eocAddPvcUpEncap.pvcEncapFlag = 1;
			eocAddPvcUpEncap.pvcEncapValue = i;
		}			
		return (smpErrorNone);			
	}
	else if ( name[0] == 9 ){		
		i =  asnNumber (asnValue (asn), asnLength (asn));	
			
		//printf("pvcSet: IGMPMode Setting and value=%d via PVC channel\n", i);		
		//pEntry=searchpEntrybyVpiVci(name[namelen-2], name[namelen-1]);
		searchpEntrybyVpiVci(name[namelen-2], name[namelen-1], (void *)&Entry);
		
		if ( i == 1 ) {  // Enable IGMPProxy			
			proxy = 1;
			if ( !mib_set(MIB_IGMP_PROXY, (void *)&proxy)) {
				printf("Set IGMP proxy error(Enable)!\n");					
			}				
			
			proxy_if = Entry.ifIndex;
			if ( !mib_set(MIB_IGMP_PROXY_ITF, (void *)&proxy_if)) {
				printf("Set IGMP proxy interface index error(2)!\n");					
			}
			
		}else {	  // Disable IGMPProxy
			/*
			proxy = 0;
			if ( !mib_set(MIB_IGMP_PROXY, (void *)&proxy)) {
				printf("Set IGMP proxy error(Disable)!\n");					
			}
			*/
			
			if (mib_get(MIB_IGMP_PROXY_ITF, (void *)&igmpItf) != 0){
				if ( (igmpItf != DUMMY_IFINDEX) && (igmpItf == Entry.ifIndex) ) {
					proxy_if = DUMMY_IFINDEX;
					if ( !mib_set(MIB_IGMP_PROXY_ITF, (void *)&proxy_if)) {
						printf("Set IGMP proxy interface index error(Default-2)!\n");					
					}						
				}						
			}			
			
		}				
		return (smpErrorNone);			
	}
	else if ( name[0] == 10 ){		
		i =  asnNumber (asnValue (asn), asnLength (asn));			
		//printf("pvcSet: This is DHCPClient Setting and value=%d\n", i);
		
		//if ( name[0] == 0x0a && name[1] == 0x10 ) {
			printf("DHCPClient Setting via PVC(vpi/vci=%d/%d) channle\n", name[namelen-2], name[namelen-1]);
			
			para.inputvpi=name[namelen-2];
			para.inputvci=name[namelen-1];
			para.cmode=0;
			para.admin=0;
			para.natmode=0;
			para.dhcpmode=i;
			para.pppIfIndex=0;	
			para.IpIndex=0;	
			para.encap=100;	
			para.brmode=100;
			
			modifyChannelConf(&para);
		//}		
		return (smpErrorNone);			
	}
	else if ( name[0] == 11 ){		
		i =  asnNumber (asnValue (asn), asnLength (asn));			
		//printf("pvcSet: This is NAT Setting and value=%d\n", i);
		
		//if ( name[0] == 0x0b && name[1] == 0x10 ) {
			printf("NAT Setting via PVC(vpi/vci=%d/%d) channle\n", name[namelen-2], name[namelen-1]);
			
			para.inputvpi=name[namelen-2];
			para.inputvci=name[namelen-1];
			para.cmode=0;
			para.admin=0;
			para.natmode=i;
			para.dhcpmode=0;
			para.pppIfIndex=0;	
			para.IpIndex=0;	
			para.encap=100;	
			para.brmode=100;
			
			modifyChannelConf(&para);
		//}		
		return (smpErrorNone);			
	}	
	else 
		return (smpErrorNone);
		
		
	
}

static	MixOpsType	pvcOps = {

			pvcRelease,
			pvcCreate,
			pvcDestroy,
			pvcNext,
			pvcGet,			
			pvcSet

			};

CVoidType		pvcInit (void)
{
	// Init eocAddPvcUpEncap for create PVC via EOC channel
	eocAddPvcUpEncap.vpi = 0;
	eocAddPvcUpEncap.vci = 0;
	eocAddPvcUpEncap.pvcEncapFlag = 0;
	eocAddPvcUpEncap.pvcEncapValue = 0;
			
	(void) misExport ((MixNamePtrType) "\53\6\1\4\1\201\204\114\2\3\1",
		(MixLengthType) 11, & pvcOps, (MixCookieType) 0);
}
