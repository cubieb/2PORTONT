

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
#include <rtk/sysconfig.h>

unsigned int entryNumATM;
struct eoc_create_aal5Encap eocAddaal5Encap;
int g_initCpePvcTable = 0;

enum snmp_atm_vcl
{
   ATMVCLVPI = 1,
   ATMVCLVCI,
   ATMVCLADMINSTATUS,
   ATMVCLOPERSTATUS,
   ATMVCCAALTYPE = 8,
   ATMVCCAAL5ENCAPSTYPE =11,
   ATMVCLROWSTATUS = 13,
   ATM_VCL_END
	
};


static struct mib_oid_tbl atmvcl_mib_oid_tbl;
static long atmvcl_last_scan = 0;


static	AsnIdType	atmvclRetrieveMibValue (unsigned int mibIdx, int inputvpi, int inputvci)
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
	char ifname[6];
	int flags;
	CONN_T conn_status;
	int i, strStatus;
	char tmp_pEntry_vpi[6], tmp_pEntry_vci[6];		
	
	oid_ptr = atmvcl_mib_oid_tbl.oid;
	oid_ptr += mibIdx;		
	
	
	// To find the correct pEntry by VPI, VCI	
	for (i=0; i<entryNumATM; i++){
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

#if 0	
	// mibIdx % entryNumATM = the parameter i of mib_chain_get(MIB_ATM_VC_TBL, i). Mason Yu	
	pEntry = (MIB_CE_ATM_VC_Tp) mib_chain_get(MIB_ATM_VC_TBL, mibIdx%entryNumATM );
#endif	


	// Retrieve VPI/VCI
	snprintf(vpi, 6, "%u", Entry.vpi);
	snprintf(vci, 6, "%u", Entry.vci);
	
	
	// Retrieve VclAdminStatus and VclOperStatus
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

	
	
	switch (oid_ptr->name[0])
	{		
	case ATMVCLVPI:									
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, atoi(vpi));  		
		
	case ATMVCLVCI:		
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, atoi(vci));	
		
	case ATMVCLADMINSTATUS:
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, strStatus);
		
	case ATMVCLOPERSTATUS:
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, conn_status);
					
	case ATMVCCAALTYPE:
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, 3);   // aal5
	case ATMVCCAAL5ENCAPSTYPE:
		if ( Entry.encap == ENCAP_VCMUX )	
			return asnUnsl (asnClassUniversal, (AsnTagType) 2, 1);   // VC_MUX
		else if ( Entry.encap == ENCAP_LLC )
			return asnUnsl (asnClassUniversal, (AsnTagType) 2, 7);   // llcEncapsulation
			
	case ATMVCLROWSTATUS:		
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, 1);           // active
																															
	default:
		//return ((AsnIdType) 0);
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, 1);
				
	}
	
}



void atmvcl_Mib_Init(void)
{
	struct timeval now;
	int j;
	int i, k;	
	//MIB_CE_ATM_VC_Tp pEntry;
	MIB_CE_ATM_VC_T Entry;	
	char vpi[6], vci[6];
	int initvpi;
	int initvci;
	int h;
	
	gettimeofday(&now, 0);

//	printf("Now:%u:%u\n",now.tv_sec, now.tv_usec);
	//if((now.tv_sec - atmvcl_last_scan) > 10 )
	if((now.tv_sec - atmvcl_last_scan) > 10 || g_initCpePvcTable == 1 )	//  10 sec
	{		
		
		unsigned int oid_tbl_size;
		struct mib_oid * oid_ptr;
		
		atmvcl_last_scan = now.tv_sec;			

		// create oid table
		free_mib_tbl(&atmvcl_mib_oid_tbl);
		
		entryNumATM = mib_chain_total(MIB_ATM_VC_TBL);		
		
		
		// atmvcl MIB has 13*entryNumATM objects
		oid_tbl_size = 13;
		
		create_mib_tbl(&atmvcl_mib_oid_tbl,  oid_tbl_size*entryNumATM, 6); 
		oid_ptr = atmvcl_mib_oid_tbl.oid;
					
		for (j=1; j<=oid_tbl_size; j++) {
			initvpi = 0;
			initvci = 0;
			
			for (i=0; i<entryNumATM; i++){
				//pEntry=vclTableSort(initvpi, initvci);	
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
				oid_ptr++;				
			}	
		}
		
		
				
	}
}



static	MixStatusType	atmvclRelease ()
{
	
	return (smpErrorReadOnly);
}

static	MixStatusType	atmvclCreate ()
{
	
	return (smpErrorReadOnly);
}

static	MixStatusType	atmvclDestroy ()
{
	
	return (smpErrorReadOnly);
}




static	AsnIdType	atmvclNext (MixCookieType cookie, MixNamePtrType name, MixLengthPtrType namelenp)
{
	unsigned int idx=0;	
	
	
	atmvcl_Mib_Init();	
	
	cookie = cookie;	
		
	if(snmp_oid_getnext(&atmvcl_mib_oid_tbl, name, *namelenp, &idx))
	{		
		struct mib_oid * oid_ptr = atmvcl_mib_oid_tbl.oid;
		oid_ptr += idx;

		memcpy((unsigned char *)name, oid_ptr->name, oid_ptr->length);		
		*namelenp = oid_ptr->length;				
		
		return atmvclRetrieveMibValue(idx, oid_ptr->name[4], oid_ptr->name[5]);
	}

	return ((AsnIdType) 0);
}


static	AsnIdType	atmvclGet (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen)
{
	unsigned int idx;
	int i;

	//for ( i=0; i<namelen; i++)
	//		printf("atmvclGet: name[%d] = %d\n", i, name[i]);
			
	atmvcl_Mib_Init();
	
	cookie = cookie;	
	
	if(snmp_oid_get(&atmvcl_mib_oid_tbl, name, namelen, &idx))
	{			
		return atmvclRetrieveMibValue(idx, name[4], name[5]);		
	}
	// Mason Yu. If set RowStatus with CreateAndGo(4), snmp agent can not get this OID. 
	// So we should retuen a valid value to pass the predure to atmvclSet().
	else if ( name[0] == 13){
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, 1);	
	}
	// Mason Yu. If set Encap with LLC(7)and snmp agent can not get this OID. 
	// So we should retuen a valid value to pass the predure to atmvclSet().
	else if ( name[0] == 11){
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, 1);	
	}
	else
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, 1);
		
	return ((AsnIdType) 0);
	
}


static	MixStatusType	atmvclSet  (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen, AsnIdType asn)
{
	unsigned int idx, ifMap;	
	long i;
	MIB_CE_ATM_VC_T entry;
	MIB_CE_ATM_VC_T tmp_Entry;
	MIB_CE_ATM_VC_T Entry;
	int cnt;
	unsigned long atmIfIndex;	
	struct channel_conf_para para;
	
	atmvcl_Mib_Init();	
	
	cookie = cookie;
	name = name;
	namelen = namelen;
	asn = asn;
		
			
	if(snmp_oid_get(&atmvcl_mib_oid_tbl, name, namelen, &idx))
	{	
		struct mib_oid * oid_ptr;				
		AsnLengthType		k;
		int option;				
		unsigned char strvalue[256];
	
		oid_ptr = atmvcl_mib_oid_tbl.oid;
		oid_ptr += idx;		
		
		
		switch (oid_ptr->name[0])
		{
		case ATMVCLROWSTATUS:
			//printf("atmvclSet: To do Rowstatus Delete PVC\n");
			i =  asnNumber (asnValue (asn), asnLength (asn));
			
			if ( i == 6 ) {   // Destroy
				//printf("atmvclSet: Delete AtmVc\n");
				deleteAtmVc(name[namelen-2], name[namelen-1]);	
				
				// Set Init cpePvcTable flag = 1 for avoid get PVC so long
				g_initCpePvcTable = 1;	
			}
			return (smpErrorNone);
		
		case ATMVCLADMINSTATUS:
			// We can do nothing.
			//printf("Set ATMVCLADMINSTATUS: We can do nothing\n");
			return (smpErrorNone);
			
		
		case ATMVCCAAL5ENCAPSTYPE: 
			//printf("Set ATMVCCAAL5ENCAPSTYPE\n");
			i =  asnNumber (asnValue (asn), asnLength (asn));			
			
			para.inputvpi=name[namelen-2];
			para.inputvci=name[namelen-1];
			para.cmode=0;
			para.admin=0;
			para.natmode=0;
			para.dhcpmode=0;
			para.pppIfIndex=0;	
			para.IpIndex=0;	
			para.encap=i;	
			para.brmode=100;
			
			modifyChannelConf(&para);			
			return (smpErrorNone);
		
		case ATMVCCAALTYPE:
			// We can do nothing. This value always is aal5(3).
			//printf("Set ATMVCAALTYPE: We can do nothing\n");
			return (smpErrorNone);
			
		default:
			return (smpErrorNone);			
		}
		
			
		
	}
	else if ( name[0] == 13 ){		
		i =  asnNumber (asnValue (asn), asnLength (asn));				
		printf("atmvclSet: This is RowStatus and value=%d.\n", i);
		
		
		if (i == 4 ){  // CreateAndGo
			entry.enable = 1;  // Enable
			entry.cmode = CHANNEL_MODE_BRIDGE;
			entry.encap = ENCAP_LLC;		
			entry.mtu = 1500;
			entry.pppIdleTime = 0;
			entry.pppCtype = CONTINUOUS;
			entry.brmode = 0;

			// Mason Yu. We have not modified the OID(140000) for ATM ifindex in ifTable(RFC1213).						
			entry.vpi = name[namelen-2];
			entry.vci = name[namelen-1];			
			
			// Mason Yu. To avoid creating the same PVC
			if ( searchpEntrybyVpiVci(name[namelen-2], name[namelen-1], (void *)&Entry) == 1 ) {
				printf("This PVC is exist, can not create this PVC !!!!!\n");
				return (smpErrorReadOnly);
			}			
			
			// check if connection exists
			ifMap = 0;
			cnt=0;
		
		        for (i=0; i<entryNumATM; i++) {
		        	//pEntry = (MIB_CE_ATM_VC_Tp) mib_chain_get(MIB_ATM_VC_TBL, i); /* get the specified chain record */
		        	mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&tmp_Entry); /* get the specified chain record */
		        	
		        	//if (tmp_Entry == NULL)
		        	//{	  	        		
		        	//	return (smpErrorReadOnly);
		        	//}		        	
                        
				ifMap |= 1 << VC_INDEX(tmp_Entry.ifIndex);	// vc map
				ifMap |= (1 << 16) << PPP_INDEX(tmp_Entry.ifIndex);	// PPP map
		        }
			
			entry.ifIndex = if_find_index(entry.cmode, ifMap);
			
			
			if (mib_chain_add(MIB_ATM_VC_TBL, (unsigned char*)&entry) != 1){
				printf("atmvclSet: Error! Add chain record.");
				return (smpErrorReadOnly);
				
			}	
			
			// Really Set aal5Encap via EOC channel
			if ( eocAddaal5Encap.aal5EncapFlag == 1 ) {
				//printf("Really Set aal5Encap(vpi/vci=%d/%d) via EOC channel\n", eocAddaal5Encap.vpi, eocAddaal5Encap.vci);
				para.inputvpi=eocAddaal5Encap.vpi;
				para.inputvci=eocAddaal5Encap.vci;
				para.cmode=0;
				para.admin=0;
				para.natmode=0;
				para.dhcpmode=0;
				para.pppIfIndex=0;	
				para.IpIndex=0;	
				para.encap=eocAddaal5Encap.aal5EncapValue;	
				para.brmode=100;
				
				modifyChannelConf(&para);
				eocAddaal5Encap.aal5EncapFlag=0;
			}	
			
			// Set Init cpePvcTable flag = 1 for avoid get PVC so long
			g_initCpePvcTable = 1;		
			return (smpErrorNone);
		}
		
		if ( i == 6 ) {   // Destroy
			//printf("atmvclSet: Can not get OID and to do Delete AtmVc(vpi/vci=%d/%d) \n", name[namelen-2], name[namelen-1]);			
			deleteAtmVc(name[namelen-2], name[namelen-1]);	
			
			// Set Init cpePvcTable flag = 1 for avoid get PVC so long
			g_initCpePvcTable = 1;			
			return (smpErrorNone);			
		}			
			
	}
	else if ( name[0] == 11 ){		
		i =  asnNumber (asnValue (asn), asnLength (asn));			
					
		// Save atmEncapFlag and atmEncapValue for create PVC via EOC chnnel
		eocAddaal5Encap.vpi=name[namelen-2];
		eocAddaal5Encap.vci=name[namelen-1];
		eocAddaal5Encap.aal5EncapFlag=1;
		eocAddaal5Encap.aal5EncapValue=i;
						
		//printf("atmvclSet: Setting aal5Encap(vpi/vci=%d/%d) and can not get OID. value=%d\n", name[namelen-2], name[namelen-1], i);		
		return (smpErrorNone);			
	}
	else
		return (smpErrorNone);

		
	
}

static	MixOpsType	atmvclOps = {

			atmvclRelease,
			atmvclCreate,
			atmvclDestroy,
			atmvclNext,
			atmvclGet,			
			atmvclSet

			};

CVoidType		atmvclInit (void)
{
	eocAddaal5Encap.aal5EncapFlag=0;
	
	(void) misExport ((MixNamePtrType) "\53\6\1\2\1\45\1\7\1",
		(MixLengthType) 9, & atmvclOps, (MixCookieType) 0);
}
