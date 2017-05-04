

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

#include "../../boa/src/LINUX/utility.h"
#include <semaphore.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/if_ether.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>


unsigned int entryNumRMIP;
static struct mib_oid_tbl rmip_mib_oid_tbl;
static long rmip_last_scan = 0;

enum snmp_cpe_iptable
{ 
   CPEIPINDEX = 1,
   CPEIPLOWERIFINDEX,
   CPEIPADDRESS,
   CPEIPNETMASK,
   CPEIPGATEWAY,
   CPEIPROWSTATUS,   
   CPE_IPTABLE_END 
};

	

static	AsnIdType	rmipRetrieveMibValue (unsigned int mibIdx, unsigned long ifindex)
{
	struct mib_oid * oid_ptr;	
	MIB_CE_ATM_VC_T Entry;	
	MIB_CE_ATM_VC_T tmp_Entry;
	char vpi[6], vci[6];	
	char ifname[6];
	int flags, strStatus;
	CONN_T conn_status;
	int i;	
	struct in_addr inAddr, outAddr;
	char ipAddr[20], remoteIp[20], netmask[20];
	char *temp;	
	unsigned char buffer[64];
	unsigned char strbuf[64];
	char p1[4];
	
	oid_ptr = rmip_mib_oid_tbl.oid;
	oid_ptr += mibIdx;		
	
	// If ifindex=10000, this interface is LAN Interface
	if ( ifindex == 10000 )
		goto LAN_IF;
	
	// To find the correct pEntry by pEntry->cpeIpIndex		
	for (i=0; i<entryNumRMIP; i++){
		//tmp_pEntry = (MIB_CE_ATM_VC_Tp) mib_chain_get(MIB_ATM_VC_TBL, i);
		mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&tmp_Entry);			
				
		if ( tmp_Entry.cpeIpIndex != ifindex  )
			continue;
		else if ( tmp_Entry.cpeIpIndex == ifindex  ){
			//pEntry = tmp_pEntry;	
			memcpy(&Entry, &tmp_Entry, sizeof(tmp_Entry));	
			break;
		}	
	
	}		
	
	
	// Retrieve VPI/VCI
	//snprintf(vpi, 6, "%u", Entry.vpi);
	//snprintf(vci, 6, "%u", Entry.vci);
	//printf("rmipRetrieveMibValue: atoi(vpi)=%d\n", atoi(vpi) );
	//printf("rmipRetrieveMibValue: atoi(vci)=%d\n", atoi(vci) );		
	
	
	// Retrieve AdminStatus and OperStatus
	if (Entry.cmode == CHANNEL_MODE_PPPOE || Entry.cmode == CHANNEL_MODE_PPPOA){
		snprintf(ifname, 6, "ppp%u", PPP_INDEX(Entry.ifIndex));
		
		// ipAddr
		if (getInAddr( ifname, IP_ADDR, (void *)&inAddr) == 1)
		{
			temp = inet_ntoa(inAddr);
			strcpy(ipAddr, temp);		
			
		}
		else
			strcpy(ipAddr, "");		
					
		// remoteIp
		if (getInAddr( ifname, DST_IP_ADDR, (void *)&inAddr) == 1)
		{
			temp = inet_ntoa(inAddr);
			strcpy(remoteIp, temp);			
			
		}
		else
			strcpy(remoteIp, "");
                
                // netmask
		if (getInAddr( ifname, SUBNET_MASK, (void *)&inAddr) == 1)	// Jenny, subnet mask
		{
			temp = inet_ntoa(inAddr);
			strcpy(netmask, temp);			
			
		}
		else
				strcpy(netmask, "");

		
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
					//conn_status = CONN_NOT_EXIST;
					conn_status = CONN_DOWN;
				}
			}
		}
		else
		{
			strStatus = 1;
			//conn_status = CONN_NOT_EXIST;
			conn_status = CONN_DOWN;
		}	
		
		
	}else {
		snprintf(ifname, 5, "vc%u", VC_INDEX(Entry.ifIndex));
		
		if (Entry.ipDhcp == (char)DHCP_DISABLED)
		{
			// static IP address
			temp = inet_ntoa(*((struct in_addr *)Entry.ipAddr));
			strcpy(ipAddr, temp);			
			
			
			temp = inet_ntoa(*((struct in_addr *)Entry.remoteIpAddr));
			strcpy(remoteIp, temp);
			

			temp = inet_ntoa(*((struct in_addr *)Entry.netMask));	// Jenny, subnet mask			
			strcpy(netmask, temp);
			
		}
		else
		{
			// DHCP enabled

			if (getInAddr( ifname, IP_ADDR, (void *)&inAddr) == 1)
			{
				temp = inet_ntoa(inAddr);
				strcpy(ipAddr, temp);				
			}
			else
				strcpy(ipAddr, "");


			if (getInAddr( ifname, DST_IP_ADDR, (void *)&inAddr) == 1)
			{
				temp = inet_ntoa(inAddr);
				strcpy(remoteIp, temp);				
			}
			else
				strcpy(remoteIp, "");
				

			if (getInAddr( ifname, SUBNET_MASK, (void *)&inAddr) == 1)	// Jenny, subnet mask
			{
				temp = inet_ntoa(inAddr);
				strcpy(netmask, temp);				
			}
			else
				strcpy(netmask, "");
		}

		if (Entry.ipunnumbered)
		{
			strcpy(ipAddr, "");
			strcpy(netmask, "");
			strcpy(remoteIp, "");
		}
		
		if (Entry.cmode == CHANNEL_MODE_BRIDGE)
		{
			strcpy(ipAddr, "");
			strcpy(netmask, "");
			strcpy(remoteIp, "");			
		}
		
		
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



LAN_IF:
	
	switch (oid_ptr->name[0])
	{
		
	case CPEIPINDEX:
		if ( ifindex != 10000)				 
			return asnUnsl (asnClassApplication, (AsnTagType) 2, Entry.cpeIpIndex ); 		
		else {			
			return asnUnsl (asnClassApplication, (AsnTagType) 2, 10000 );
		}	
			
			
	case CPEIPLOWERIFINDEX:	
		if ( ifindex != 10000)									
			return asnUnsl (asnClassUniversal, (AsnTagType) 2, 150000+VC_INDEX(Entry.ifIndex) );  		
		else
			return asnUnsl (asnClassUniversal, (AsnTagType) 2, 10000 );	
			
		
		
	case CPEIPADDRESS:
		if ( ifindex != 10000)	{
			inet_aton(ipAddr, &outAddr);
			p1[0]=*(char *)(&(outAddr.s_addr)); 
			p1[1]=*((char *)(&(outAddr.s_addr)) + 1); 
			p1[2]=*((char *)(&(outAddr.s_addr)) + 2);
			p1[3]=*((char *)(&(outAddr.s_addr)) + 3);
			
			return asnOctetString (asnClassApplication, (AsnTagType) 0, 
						(CBytePtrType) p1, 
						(AsnLengthType) 4);						
			
		}	
		else {
			if(!mib_get( MIB_ADSL_LAN_IP, (void *)buffer))
				return ((AsnIdType) 0);
			sprintf(strbuf, "%s", inet_ntoa(*((struct in_addr *)buffer)));
						
			inet_aton(strbuf, &outAddr);
			p1[0]=*(char *)(&(outAddr.s_addr)); 
			p1[1]=*((char *)(&(outAddr.s_addr)) + 1); 
			p1[2]=*((char *)(&(outAddr.s_addr)) + 2);
			p1[3]=*((char *)(&(outAddr.s_addr)) + 3);
			
			return asnOctetString (asnClassApplication, (AsnTagType) 0, 
						(CBytePtrType) p1, 
						(AsnLengthType) 4);
		}				
		
	case CPEIPNETMASK:
		if ( ifindex != 10000)	{
			
			inet_aton(netmask, &outAddr);
			p1[0]=*(char *)(&(outAddr.s_addr)); 
			p1[1]=*((char *)(&(outAddr.s_addr)) + 1); 
			p1[2]=*((char *)(&(outAddr.s_addr)) + 2);
			p1[3]=*((char *)(&(outAddr.s_addr)) + 3);
			
			return asnOctetString (asnClassApplication, (AsnTagType) 0, 
						(CBytePtrType) p1, 
						(AsnLengthType) 4);						
			
		}		
		else {
			if(!mib_get( MIB_ADSL_LAN_SUBNET, (void *)buffer))
				return ((AsnIdType) 0);
			sprintf(strbuf, "%s", inet_ntoa(*((struct in_addr *)buffer)));
			
			inet_aton(strbuf, &outAddr);
			p1[0]=*(char *)(&(outAddr.s_addr)); 
			p1[1]=*((char *)(&(outAddr.s_addr)) + 1); 
			p1[2]=*((char *)(&(outAddr.s_addr)) + 2);
			p1[3]=*((char *)(&(outAddr.s_addr)) + 3);
			return asnOctetString (asnClassApplication, (AsnTagType) 0, 
						(CBytePtrType) p1, 
						(AsnLengthType) 4);					
			
		}				
					
	case CPEIPGATEWAY:
		if ( ifindex != 10000)	{
			
			inet_aton(remoteIp, &outAddr);
			p1[0]=*(char *)(&(outAddr.s_addr)); 
			p1[1]=*((char *)(&(outAddr.s_addr)) + 1); 
			p1[2]=*((char *)(&(outAddr.s_addr)) + 2);
			p1[3]=*((char *)(&(outAddr.s_addr)) + 3);
			return asnOctetString (asnClassApplication, (AsnTagType) 0, 
						(CBytePtrType) p1, 
						(AsnLengthType) 4);			
			
		}		
		else {
			p1[0]=0x00; 
			p1[1]=0x00; 
			p1[2]=0x00;
			p1[3]=0x00;
			return asnOctetString (asnClassApplication, (AsnTagType) 0, 
						(CBytePtrType) p1, 
						(AsnLengthType) 4);				
			
		}
			
	case CPEIPROWSTATUS:	
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, 4);  		
	
	// Mason Yu.
	// I do not know what is this.But it will be used for EOC channel for Set and get Next Function.	
	case 7:
	 	return asnUnsl (asnClassUniversal, (AsnTagType) 2, 2); 	
	 																													
	default:
		//return ((AsnIdType) 0);
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, 1);
				
	}
	
}



void rmip_Mib_Init(void)
{
	struct timeval now;
	int j;
	int i, k;	
	//MIB_CE_ATM_VC_Tp pEntry;	
	MIB_CE_ATM_VC_T Entry;		
	unsigned long initifindex;
	
	gettimeofday(&now, 0);

//	printf("Now:%u:%u\n",now.tv_sec, now.tv_usec);

	//if((now.tv_sec - rmip_last_scan) > 10)	//  10 sec
	{		
		
		unsigned int oid_tbl_size;
		struct mib_oid * oid_ptr;
		
		rmip_last_scan = now.tv_sec;			

		// create oid table
		free_mib_tbl(&rmip_mib_oid_tbl);
		
		entryNumRMIP = mib_chain_total(MIB_ATM_VC_TBL);		
		
		
		// rmip MIB has 7*entryNumrmip objects
		oid_tbl_size = 7;
		
		create_mib_tbl(&rmip_mib_oid_tbl,  oid_tbl_size*(entryNumRMIP+1), 3); 
		oid_ptr = rmip_mib_oid_tbl.oid;			
	
	    	
		for (j=1; j<=oid_tbl_size; j++) {			
			initifindex = 0;			
			
			// Register the first OID for LAN Interface 
			oid_ptr->length = 3;
			oid_ptr->name[0] = j;
			oid_ptr->name[1] = 206;                                
			oid_ptr->name[2] = 16;              					          												
			oid_ptr++;
			
			
			// Register other OIDs for WAN Interfaces
			for (i=0; i<entryNumRMIP; i++){				
				//pEntry = ipTableSort(initifindex);
				ipTableSort(initifindex, &Entry);		
				
				initifindex = Entry.cpeIpIndex;				
				
				if ((Entry.cmode != CHANNEL_MODE_BRIDGE) && initifindex!=0xff ){					
								
					k= 16;                                                  // (2)Hex:10															
					oid_ptr->length = 3;
					oid_ptr->name[0] = j;
					oid_ptr->name[1] = 206;                                 // (1) Hex= ce
					oid_ptr->name[2] = k+(initifindex-10000);               // The (1)0xce, (2)0x10 value is means 10000					          												
					oid_ptr++;
				}else
					continue;							
			}	
		}				
	}
}



static	MixStatusType	rmipRelease ()
{
	
	return (smpErrorReadOnly);
}

static	MixStatusType	rmipCreate ()
{
	
	return (smpErrorReadOnly);
}

static	MixStatusType	rmipDestroy ()
{
	
	return (smpErrorReadOnly);
}




static	AsnIdType	rmipNext (MixCookieType cookie, MixNamePtrType name, MixLengthPtrType namelenp)
{
	unsigned int idx=0;
		
	
	rmip_Mib_Init();	
	
	cookie = cookie;	
	
	if(snmp_oid_getnext(&rmip_mib_oid_tbl, name, *namelenp, &idx))
	{		
		struct mib_oid * oid_ptr = rmip_mib_oid_tbl.oid;
		oid_ptr += idx;

		memcpy((unsigned char *)name, oid_ptr->name, oid_ptr->length);		
		*namelenp = oid_ptr->length;			
		
		//name[2]-16+10000 = cpeIpIndex(10000 ~ 10009)
		return rmipRetrieveMibValue(idx, name[2]-16+10000);
	}
	
	return ((AsnIdType) 0);
}


static	AsnIdType	rmipGet (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen)
{
	unsigned int idx;	
	
			
	rmip_Mib_Init();
	
	cookie = cookie;
	
	if(snmp_oid_get(&rmip_mib_oid_tbl, name, namelen, &idx))
	{			
		if ( namelen != 0 ){	
		        //name[2]-16+10000 = cpeIpIndex(10000 ~ 10009)		       	
			return rmipRetrieveMibValue(idx, name[2]-16+10000);  
		}else	
			return ((AsnIdType) 0);	
			
	}
	// Mason Yu. If do RowStatus with CreateAndGo(4), snmp agent will not get this OID. 
	// So we should retuen a valid value to pass the predure to rmipSet().
	else if ( name[0] == 6){
		//printf("rmipGet: (1) Cannot find the OID(name[0]=%d)\n", name[0]);		
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, 1);	
	// Added by Mason Yu.
	// If Ifindex = 0, it mean LAN interface's index on True Lib.	
	} else if ( name[0] == 3 && name[1] == 0 ) {
		struct in_addr inAddr, outAddr;
		unsigned char buffer[64];
		unsigned char strbuf[64];
		char p1[4];
		
		printf("rmipGet: Get LAN IP Address on True Lib\n");	
		
		if(!mib_get( MIB_ADSL_LAN_IP, (void *)buffer))
			return ((AsnIdType) 0);
		sprintf(strbuf, "%s", inet_ntoa(*((struct in_addr *)buffer)));
					
		inet_aton(strbuf, &outAddr);
		p1[0]=*(char *)(&(outAddr.s_addr)); 
		p1[1]=*((char *)(&(outAddr.s_addr)) + 1); 
		p1[2]=*((char *)(&(outAddr.s_addr)) + 2);
		p1[3]=*((char *)(&(outAddr.s_addr)) + 3);
		
		return asnOctetString (asnClassApplication, (AsnTagType) 0, 
			(CBytePtrType) p1, 
			(AsnLengthType) 4);
	
	// Added by Mason Yu.
	// If Ifindex = 0, it mean LAN interface's index on True Lib.	
	} else if ( name[0] == 4 && name[1] == 0 ) {
		struct in_addr inAddr, outAddr;
		unsigned char buffer[64];
		unsigned char strbuf[64];
		char p1[4];
		
		printf("rmipGet: Get LAN NetMask on True Lib\n");	
		
		if(!mib_get( MIB_ADSL_LAN_SUBNET, (void *)buffer))
			return ((AsnIdType) 0);
		sprintf(strbuf, "%s", inet_ntoa(*((struct in_addr *)buffer)));
					
		inet_aton(strbuf, &outAddr);
		p1[0]=*(char *)(&(outAddr.s_addr)); 
		p1[1]=*((char *)(&(outAddr.s_addr)) + 1); 
		p1[2]=*((char *)(&(outAddr.s_addr)) + 2);
		p1[3]=*((char *)(&(outAddr.s_addr)) + 3);
		
		return asnOctetString (asnClassApplication, (AsnTagType) 0, 
			(CBytePtrType) p1, 
			(AsnLengthType) 4);					
	}else {
		//printf("rmipGet: (2) Cannot find the OID(name[0]=%d)\n", name[0]);
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, 1);
	}
	
	return ((AsnIdType) 0);
	
}


static	MixStatusType	rmipSet  (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen, AsnIdType asn)
{
	unsigned int idx;	
	long i;	
	struct channel_conf_ip_para para;
	unsigned long CpeIpIndex;
	int entryNum;
	MIB_CE_ATM_VC_T Entry;
	int j;
	struct in_addr inIp, inMask;
	
	
	rmip_Mib_Init();	
	
	cookie = cookie;
	name = name;
	namelen = namelen;
	asn = asn;	
	
	//for ( i=0; i<6; i++)
	//		printf("rmipSet: name[%d] = %d\n", i, name[i]);
			
	if(snmp_oid_get(&rmip_mib_oid_tbl, name, namelen, &idx))
	{	
		struct mib_oid * oid_ptr;				
		AsnLengthType		k;
		int option;				
		unsigned char strvalue[256];
	
		oid_ptr = rmip_mib_oid_tbl.oid;
		oid_ptr += idx;		
		
		
		switch (oid_ptr->name[0])
		{
			
		case CPEIPADDRESS:
			//printf("rmipSet: To do CPEIPADDRESS\n");			
			i =  asnNumber (asnValue (asn), asnLength (asn));
			//printf("rmipSet(1): i=0x%x  IpIndex=%d\n", i, name[2]-16+10000);
			
			// This is not a LAN Interface
			if ( name[2]-16+10000 != 10000) {
				para.IpIndex = name[2]-16+10000;
				((struct in_addr *)para.ipAddr)->s_addr = i;									
				strcpy(para.remoteIpAddr, "");
				strcpy(para.netMask, "");
				
				modifyChannelConfIP(&para);
				
			// This is a LAN Interface	
			}else {				
				inIp.s_addr = i;									
				mib_set( MIB_ADSL_LAN_IP, (void *)&inIp); 
			}				
			
			return (smpErrorNone);
		
		case CPEIPNETMASK:
			//printf("rmipSet: To do CPEIPNETMASK\n");		
			i =  asnNumber (asnValue (asn), asnLength (asn));
			//printf("rmipSet(2): i=0x%x  IpIndex=%d\n", i, name[2]-16+10000);
			
			
			// This is not a LAN Interface
			if ( name[2]-16+10000 != 10000) {
				para.IpIndex = name[2]-16+10000;				
				strcpy(para.ipAddr, "");								
				strcpy(para.remoteIpAddr, "");
				((struct in_addr *)para.netMask)->s_addr = i;
						
				modifyChannelConfIP(&para);	
			
			// This is a LAN Interface	
			}else {				
				inMask.s_addr = i;									
				mib_set( MIB_ADSL_LAN_SUBNET, (void *)&inMask); 
			}			
					
			return (smpErrorNone);
		
		case CPEIPGATEWAY:
			//printf("rmipSet: To do CPEIPGATEWAY\n");		
			i =  asnNumber (asnValue (asn), asnLength (asn));
			//printf("rmipSet(3): i=0x%x  IpIndex=%d\n", i, name[2]-16+10000);
			
			para.IpIndex = name[2]-16+10000;				
			strcpy(para.ipAddr, "");								
			strcpy(para.netMask, "");
			((struct in_addr *)para.remoteIpAddr)->s_addr = i;
						
			modifyChannelConfIP(&para);		
			return (smpErrorNone);
		
		case CPEIPLOWERIFINDEX:
			//printf("Set CPEIPLOWERIFINDEX: We can do nothing\n");			
			return (smpErrorNone);
			
		case 7:
			//printf("Set 7(rmipSet): We can do nothing\n");			
			return (smpErrorNone);	
				
		default:
			return (smpErrorReadOnly);			
		}
				
	}else if ( name[0] == 6 ) {
		
		i =  asnNumber (asnValue (asn), asnLength (asn));		
		CpeIpIndex = name[2]-16+10000;
		printf("rmipSet: This is RowStatus and value=%d. CpeIpIndex=0x%x\n", i, CpeIpIndex);
		
		if (i == 4 ){	// CreateAndGo	
			char vpi[6], vci[6];
			struct channel_conf_para paraIfIndex;
				
			for (j=0; j<entryNumRMIP; j++){
				//pEntry = (MIB_CE_ATM_VC_Tp) mib_chain_get(MIB_ATM_VC_TBL, j);
				mib_chain_get(MIB_ATM_VC_TBL, j, (void *)&Entry);
				
				if ((Entry.cmode != CHANNEL_MODE_BRIDGE) && Entry.cpeIpIndex == 0xff ){						
					printf("rmipSet: Find the cpeIpIndex(%d) and set it!\n", CpeIpIndex);
					snprintf(vpi, 6, "%u", Entry.vpi);
        				snprintf(vci, 6, "%u", Entry.vci);
        				
        				paraIfIndex.inputvpi=atoi(vpi);
        				paraIfIndex.inputvci=atoi(vci);
        				paraIfIndex.cmode=0;
        				paraIfIndex.admin=0;
        				paraIfIndex.natmode=0;
        				paraIfIndex.dhcpmode=0;
        				paraIfIndex.pppIfIndex=0;			
        				paraIfIndex.IpIndex=CpeIpIndex;	
        				paraIfIndex.encap=100;	
        				paraIfIndex.brmode=100;	        				
        				modifyChannelConf(&paraIfIndex);					
					return (smpErrorNone);			
					
				}else
					continue;
					
			}
				
		}	
		//return (smpErrorNone);
		
	}else if ( name[0] == 7 ) {	
		return (smpErrorNone);	
	}else
		return (smpErrorReadOnly);
		
	
}

static	MixOpsType	rmipOps = {

			rmipRelease,
			rmipCreate,
			rmipDestroy,
			rmipNext,
			rmipGet,			
			rmipSet

			};

CVoidType		rmipInit (void)
{
	
	(void) misExport ((MixNamePtrType) "\53\6\1\4\1\201\204\114\2\5\1",
		(MixLengthType) 11, & rmipOps, (MixCookieType) 0);
		
		
}
