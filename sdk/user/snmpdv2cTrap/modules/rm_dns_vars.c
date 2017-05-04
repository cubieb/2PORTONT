

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

enum snmp_cpe_wandns
{ 
   CPEDNSPRIMARYADDRESS = 1,
   CPEDNSSECONDADDRESS,   
   CPE_WANDNS_END 
};

static struct mib_oid_tbl rmdns_mib_oid_tbl;
static long rmdns_last_scan = 0;
	

static	AsnIdType	rmdnsRetrieveMibValue (unsigned int mibIdx)
{
	struct mib_oid * oid_ptr;			
	int i;		
	unsigned char vChar;
	unsigned char buffer[64];
	unsigned char strbuf[64];
	char p1[4], p2[4];
	struct in_addr dns1, dns2;
	
	oid_ptr = rmdns_mib_oid_tbl.oid;
	oid_ptr += mibIdx;			

	
	switch (oid_ptr->name[0])
	{
		
	case CPEDNSPRIMARYADDRESS:	
		
		if(!mib_get( MIB_ADSL_WAN_DNS1, (void *)buffer))
			return ((AsnIdType) 0);
		sprintf(strbuf, "%s", inet_ntoa(*((struct in_addr *)buffer)));
		
		inet_aton(strbuf, &dns1);
		p1[0]=*(char *)(&(dns1.s_addr)); 
		p1[1]=*((char *)(&(dns1.s_addr)) + 1); 
		p1[2]=*((char *)(&(dns1.s_addr)) + 2);
		p1[3]=*((char *)(&(dns1.s_addr)) + 3);
		
		//strcpy(strbuf, "10.1.1.2");	
		return asnOctetString (asnClassApplication, (AsnTagType) 0, 
						(CBytePtrType) p1, 
						(AsnLengthType) 4);
		
		//return asnUnsl (asnClassUniversal, (AsnTagType) 0x40, 0xc0a80202);
		
		
	case CPEDNSSECONDADDRESS:		
		
		if(!mib_get( MIB_ADSL_WAN_DNS2, (void *)buffer))
			return ((AsnIdType) 0);
		sprintf(strbuf, "%s", inet_ntoa(*((struct in_addr *)buffer)));	
		
		inet_aton(strbuf, &dns2);
		p2[0]=*(char *)(&(dns2.s_addr)); 
		p2[1]=*((char *)(&(dns2.s_addr)) + 1); 
		p2[2]=*((char *)(&(dns2.s_addr)) + 2);
		p2[3]=*((char *)(&(dns2.s_addr)) + 3);
		return asnOctetString (asnClassApplication, (AsnTagType) 0, 
						(CBytePtrType) p2, 
						(AsnLengthType) 4);
		
		//return asnUnsl (asnClassUniversal, (AsnTagType) 0x40, 0xc0a80203);
	 																														
	default:
		//return ((AsnIdType) 0);
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, 1);
				
	}
	
}



void rmdns_Mib_Init(void)
{
	struct timeval now;	
	
	gettimeofday(&now, 0);

//	printf("Now:%u:%u\n",now.tv_sec, now.tv_usec);

	if((now.tv_sec - rmdns_last_scan) > 10)	//  10 sec
	{		
		
		unsigned int oid_tbl_size;
		struct mib_oid * oid_ptr;
		
		rmdns_last_scan = now.tv_sec;			

		// create oid table
		free_mib_tbl(&rmdns_mib_oid_tbl);		
		
		// rmdns MIB has 2 objects
		oid_tbl_size = 2;
		
		create_mib_tbl(&rmdns_mib_oid_tbl,  oid_tbl_size, 2); 
		oid_ptr = rmdns_mib_oid_tbl.oid;								
																			
		oid_ptr->length = 2;
		oid_ptr->name[0] = 1;
		oid_ptr->name[1] = 0; 		
		oid_ptr++;
		
		oid_ptr->length = 2;
		oid_ptr->name[0] = 2;
		oid_ptr->name[1] = 0; 	
		oid_ptr++;	
	}
}



static	MixStatusType	rmdnsRelease ()
{
	
	return (smpErrorReadOnly);
}

static	MixStatusType	rmdnsCreate ()
{
	
	return (smpErrorReadOnly);
}

static	MixStatusType	rmdnsDestroy ()
{
	
	return (smpErrorReadOnly);
}




static	AsnIdType	rmdnsNext (MixCookieType cookie, MixNamePtrType name, MixLengthPtrType namelenp)
{
	unsigned int idx=0;
		
	
	rmdns_Mib_Init();	
	
	cookie = cookie;	
	
	if(snmp_oid_getnext(&rmdns_mib_oid_tbl, name, *namelenp, &idx))
	{		
		struct mib_oid * oid_ptr = rmdns_mib_oid_tbl.oid;
		oid_ptr += idx;

		memcpy((unsigned char *)name, oid_ptr->name, oid_ptr->length);		
		*namelenp = oid_ptr->length;		
		
		return rmdnsRetrieveMibValue(idx);
	}
	
	return ((AsnIdType) 0);
}


static	AsnIdType	rmdnsGet (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen)
{
	unsigned int idx;	
	
			
	rmdns_Mib_Init();
	
	cookie = cookie;
	
	if(snmp_oid_get(&rmdns_mib_oid_tbl, name, namelen, &idx))
	{			
		if ( namelen != 0 ){			       		       	
			return rmdnsRetrieveMibValue(idx);  
		}else	
			return ((AsnIdType) 0);	
			
	}		
	
	printf("rmdnsGet: Cannot find the OID\n");
	return ((AsnIdType) 0);
	
}


static	MixStatusType	rmdnsSet  (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen, AsnIdType asn)
{
	unsigned int idx;		
	struct in_addr dns1, dns2;
	long i;
	
	
	rmdns_Mib_Init();	
	
	cookie = cookie;
	name = name;
	namelen = namelen;
	asn = asn;	
	
	//for ( i=0; i<6; i++)
	//		printf("rmdnsSet: name[%d] = %d\n", i, name[i]);
			
	if(snmp_oid_get(&rmdns_mib_oid_tbl, name, namelen, &idx))
	{	
		struct mib_oid * oid_ptr;		
		unsigned char vChar;
		DHCP_TYPE_T dtmode;
		
		oid_ptr = rmdns_mib_oid_tbl.oid;
		oid_ptr += idx;		
		
		
		switch (oid_ptr->name[0])
		{
			
		case CPEDNSPRIMARYADDRESS:				
			i =  asnNumber (asnValue (asn), asnLength (asn));
			
			dns1.s_addr = i;									
			mib_set( MIB_ADSL_WAN_DNS1, (void *)&dns1); 					
			return (smpErrorNone);	
		
		case CPEDNSSECONDADDRESS:				
			i =  asnNumber (asnValue (asn), asnLength (asn));
			
			dns2.s_addr = i;									
			mib_set( MIB_ADSL_WAN_DNS2, (void *)&dns2); 					
			return (smpErrorNone);
		
				
		default:
			return (smpErrorReadOnly);			
		}
				
	}		
	
		
	
}

static	MixOpsType	rmdnsOps = {

			rmdnsRelease,
			rmdnsCreate,
			rmdnsDestroy,
			rmdnsNext,
			rmdnsGet,			
			rmdnsSet

			};

CVoidType		rmdnsInit (void)
{
	
	(void) misExport ((MixNamePtrType) "\53\6\1\4\1\201\204\114\2\7",
		(MixLengthType) 10, & rmdnsOps, (MixCookieType) 0);
		
		
}
