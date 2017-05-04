

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


unsigned int entryNumPPPSS;
static struct mib_oid_tbl pppss_mib_oid_tbl;
static long pppss_last_scan = 0;

enum snmp_ppp_sec
{   
   PPPSECURITYSECRETSLINK = 1, 
   PPPSECURITYSECRETSIDINDEX,
   PPPSECURITYSECRETSDIRECTION,
   PPPSECURITYSECRETSPROTOCOL,
   PPPSECURITYSECRETSIDENTITY,
   PPPSECURITYSECRETSSECRET,
   PPPSECURITYSECRETSSTATUS,
   PPP_SEC_END 
};


	

static	AsnIdType	pppssRetrieveMibValue (unsigned int mibIdx, unsigned long ifindex)
{
	struct mib_oid * oid_ptr;	
	//MIB_CE_ATM_VC_Tp pEntry, tmp_pEntry;	
	MIB_CE_ATM_VC_T Entry, tmp_Entry;
	char vpi[6], vci[6];	
	char ifname[6];
	int flags, strStatus;
	CONN_T conn_status;
	int i;	
	MixNameType local_oid[10];
	
	oid_ptr = pppss_mib_oid_tbl.oid;
	oid_ptr += mibIdx;			
	
	//printf("pppssGet: inputvpi = %d  inputvci = %d\n", inputvpi, inputvci);
	// To find the correct pEntry by VC_INDEX(pEntry->ifIndex)	
	for (i=0; i<entryNumPPPSS; i++){
		//tmp_pEntry = (MIB_CE_ATM_VC_Tp) mib_chain_get(MIB_ATM_VC_TBL, i);
		mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&tmp_Entry);			
				
		if ( tmp_Entry.cpePppIfIndex != ifindex  )
			continue;
		else if ( tmp_Entry.cpePppIfIndex == ifindex  ){
			//pEntry = tmp_pEntry;	
			memcpy(&Entry, &tmp_Entry, sizeof(MIB_CE_ATM_VC_T));		
			break;
		}	
	
	}		
	
	
	// Retrieve VPI/VCI
	//snprintf(vpi, 6, "%u", Entry.vpi);
	//snprintf(vci, 6, "%u", Entry.vci);
	//printf("pppssRetrieveMibValue: atoi(vpi)=%d\n", atoi(vpi) );
	//printf("pppssRetrieveMibValue: atoi(vci)=%d\n", atoi(vci) );		
	
	
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
		
	}	
	
	
	switch (oid_ptr->name[0])
	{
		
	case PPPSECURITYSECRETSLINK:									
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, Entry.cpePppIfIndex );  		
		
	case PPPSECURITYSECRETSIDINDEX:		
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, 1 );	
		
	case PPPSECURITYSECRETSDIRECTION:
		if ( Entry.pppCtype == CONNECT_ON_DEMAND )
			return asnUnsl (asnClassUniversal, (AsnTagType) 2, 2);    // remote to local
		else 
			return asnUnsl (asnClassUniversal, (AsnTagType) 2, 1);    // local to remote
		
		
	case PPPSECURITYSECRETSPROTOCOL:
		if ( Entry.pppAuth==PPP_AUTH_CHAP ) {						
			memcpy(local_oid, "\53\6\1\2\1\12\27\2\1\2", 10);			
			return asnObjectId (asnClassUniversal, (AsnTagType) 6, local_oid, 10);
			
		}else if ( Entry.pppAuth==PPP_AUTH_PAP ){
			memcpy(local_oid, "\53\6\1\2\1\12\27\2\1\1", 10);			
			return asnObjectId (asnClassUniversal, (AsnTagType) 6, local_oid, 10);
		} else {
			memcpy(local_oid, "\53\6\1\2\1\12\27\2\1\1", 10);			
			return asnObjectId (asnClassUniversal, (AsnTagType) 6, local_oid, 10);
		}	
			
										
	case PPPSECURITYSECRETSIDENTITY:		
		return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
							(CBytePtrType) (Entry.pppUsername), 
							(AsnLengthType) strlen (Entry.pppUsername));
			
	case PPPSECURITYSECRETSSECRET:	
		return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
							(CBytePtrType) (Entry.pppPassword), 
							(AsnLengthType) strlen (Entry.pppPassword));  
		
	case PPPSECURITYSECRETSSTATUS:
		if ( strStatus == 1)  // Enable this Entry
			return asnUnsl (asnClassUniversal, (AsnTagType) 2, 2);    // valid
		else
			return asnUnsl (asnClassUniversal, (AsnTagType) 2, 1);    // invalid	
	 																														
	default:
		//return ((AsnIdType) 0);
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, 1);
				
	}
	
}



void pppss_Mib_Init(void)
{
	struct timeval now;
	int j;
	int i, k;	
	//MIB_CE_ATM_VC_Tp pEntry;
	MIB_CE_ATM_VC_T Entry;			
	unsigned long initifindex;
	
	gettimeofday(&now, 0);

//	printf("Now:%u:%u\n",now.tv_sec, now.tv_usec);

	if((now.tv_sec - pppss_last_scan) > 10)	//  10 sec
	{		
		
		unsigned int oid_tbl_size;
		struct mib_oid * oid_ptr;
		
		pppss_last_scan = now.tv_sec;			

		// create oid table
		free_mib_tbl(&pppss_mib_oid_tbl);
		
		entryNumPPPSS = mib_chain_total(MIB_ATM_VC_TBL);		
		
		
		// pppss MIB has 7*entryNumPPPSS objects
		oid_tbl_size = 7;
		
		create_mib_tbl(&pppss_mib_oid_tbl,  oid_tbl_size*entryNumPPPSS, 5); 
		oid_ptr = pppss_mib_oid_tbl.oid;	
		
		
		for (j=1; j<=oid_tbl_size; j++) {			
			initifindex = 0;			
			
			for (i=0; i<entryNumPPPSS; i++){				
				//pEntry = pppTableSort(initifindex);		
				pppTableSort(initifindex, &Entry);
				
				initifindex = Entry.cpePppIfIndex;				
				
				if ((Entry.cmode == CHANNEL_MODE_PPPOE || Entry.cmode == CHANNEL_MODE_PPPOA) && initifindex!=0xff ){					
								
					k= 80;                                                  // Hex:50															
					oid_ptr->length = 5;
					oid_ptr->name[0] = j;
					oid_ptr->name[1] = 140;                                 // (1) Hex= 8c
					oid_ptr->name[2] = 232;                                 // (2) Hex= e8
					oid_ptr->name[3] = k+(initifindex-210000);              // (3) Hex= 50  the (1)0x8c, (2)0xe8, (3)0x50 value is means 210000												
					oid_ptr->name[4] = 0;                                   // EoC channel will put 0 value to modify PppSecuritySecret Table
					oid_ptr++;
				}else
					continue;
			
							
			}	
		}
		
		
				
	}
}



static	MixStatusType	pppssRelease ()
{
	
	return (smpErrorReadOnly);
}

static	MixStatusType	pppssCreate ()
{
	
	return (smpErrorReadOnly);
}

static	MixStatusType	pppssDestroy ()
{
	
	return (smpErrorReadOnly);
}




static	AsnIdType	pppssNext (MixCookieType cookie, MixNamePtrType name, MixLengthPtrType namelenp)
{
	unsigned int idx=0;
		
	
	pppss_Mib_Init();	
	
	cookie = cookie;	
	
	if(snmp_oid_getnext(&pppss_mib_oid_tbl, name, *namelenp, &idx))
	{		
		struct mib_oid * oid_ptr = pppss_mib_oid_tbl.oid;
		oid_ptr += idx;

		memcpy((unsigned char *)name, oid_ptr->name, oid_ptr->length);		
		*namelenp = oid_ptr->length;				
		
		//name[3]-80+210000 = cpePppIfIndex(210000 ~ 210008)		
		return pppssRetrieveMibValue(idx, name[3]-80+210000);
	}
	
	return ((AsnIdType) 0);
}


static	AsnIdType	pppssGet (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen)
{
	unsigned int idx;	
	
			
	pppss_Mib_Init();
	
	cookie = cookie;
	
	if(snmp_oid_get(&pppss_mib_oid_tbl, name, namelen, &idx))
	{			
		if ( namelen != 0 )	
		        //name[3]-80+210000 = cpePppIfIndex(210000 ~ 210008)		
			return pppssRetrieveMibValue(idx, name[3]-80+210000);  
		else	
			return ((AsnIdType) 0);	
			
	}
		
	
	printf("pppssGet: Cannot find the OID\n");
	return ((AsnIdType) 0);
	
}


static	MixStatusType	pppssSet  (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen, AsnIdType asn)
{
	unsigned int idx;	
	long i;	
	struct channel_conf_ppp_para para;
	unsigned long CpePppIfIndex;
	int entryNum;
	MIB_CE_ATM_VC_Tp pEntry;
	int j;
	
	
	pppss_Mib_Init();	
	
	cookie = cookie;
	name = name;
	namelen = namelen;
	asn = asn;	
	
	//for ( i=0; i<6; i++)
	//		printf("pppssSet: name[%d] = %d\n", i, name[i]);
			
	if(snmp_oid_get(&pppss_mib_oid_tbl, name, namelen, &idx))
	{	
		struct mib_oid * oid_ptr;				
		AsnLengthType		k;
		int option;				
		unsigned char strvalue[MAX_NAME_LEN];
	
		oid_ptr = pppss_mib_oid_tbl.oid;
		oid_ptr += idx;		
		
		
		switch (oid_ptr->name[0])
		{
			
		case PPPSECURITYSECRETSIDENTITY:
			//printf("pppssSet: To do PPPSECURITYSECRETSIDENTITY\n");
			k = asnLength (asn);		
			(void) asnContents (asn, strvalue, k);	
			strvalue[k]=NULL;			
			
			para.pppIfIndex=name[3]-80+210000;						
			para.admin=0;			
			para.IdleTime=0;
			strcpy(para.pppUsername, strvalue);
			strcpy(para.pppPassword, "");	
			para.mtu=0;		
						
			modifyChannelConfPPP(&para);
			return (smpErrorNone);
		
		case PPPSECURITYSECRETSSECRET:
			//printf("pppssSet: To do PPPSECURITYSECRETSSECRET\n");
			k = asnLength (asn);		
			(void) asnContents (asn, strvalue, k);	
			strvalue[k]=NULL;			
			
			para.pppIfIndex=name[3]-80+210000;						
			para.admin=0;			
			para.IdleTime=0;
			strcpy(para.pppUsername, "");
			strcpy(para.pppPassword, strvalue);
			para.mtu=0;			
						
			modifyChannelConfPPP(&para);						
			return (smpErrorNone);
					
		default:
			return (smpErrorReadOnly);			
		}
				
	}	
	
		
	
}

static	MixOpsType	pppssOps = {

			pppssRelease,
			pppssCreate,
			pppssDestroy,
			pppssNext,
			pppssGet,			
			pppssSet

			};

CVoidType		pppssInit (void)
{
	
	(void) misExport ((MixNamePtrType) "\53\6\1\2\1\12\27\2\3\1",
		(MixLengthType) 10, & pppssOps, (MixCookieType) 0);
		
		
}
