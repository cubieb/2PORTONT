

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


extern int g_ping_flag;
extern int g_PINGCOUNT;
extern int g_DEFDATALEN;   
extern char g_pingStr[64];
extern char g_xPingString[500];

enum snmp_cpe_ping
{    
   CPEPINGADDR = 1,
   CPEPINGPKGSIZE,
   CPEPINGCOUNT,
   CPEPINGADMINSTATUS,
   CPEPINGOPERSTATUS,
   CPEPINGRESULT,
   CPE_PING_END 
};

static int pingAdminStatus=1;
static struct mib_oid_tbl RMping_mib_oid_tbl;
static long RMping_last_scan = 0;
static long RMping_test_last_scan = 0;

int g_pingOperStatus=1;         // normal


static	AsnIdType	RMpingRetrieveMibValue (unsigned int mibIdx)
{
	struct mib_oid * oid_ptr;
	unsigned long value;
	char *string;		
	int fd;
	struct ifreq    ifrq;	
	struct sysinfo info;
	struct timeval now;
	
	
	oid_ptr = RMping_mib_oid_tbl.oid;
	oid_ptr += mibIdx;	
	
	
	switch (oid_ptr->name[0])
	{
		
	case CPEPINGADDR:							
		return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
						(CBytePtrType) (g_pingStr), 
						(AsnLengthType) strlen ((char *) (g_pingStr))); 		
		
	case CPEPINGPKGSIZE:
		// Kaohj
		//return asnUnsl (asnClassUniversal, (AsnTagType) 2, g_DEFDATALEN+8);	
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, g_DEFDATALEN);	
		
	case CPEPINGCOUNT:
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, g_PINGCOUNT);
		
	case CPEPINGADMINSTATUS:
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, pingAdminStatus);
					
	case CPEPINGOPERSTATUS:		
		gettimeofday(&now, 0);	
		
		if((now.tv_sec - RMping_test_last_scan) > 30) {   // 30 secs
			g_pingOperStatus = 1;
		}			
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, g_pingOperStatus);
		
	case CPEPINGRESULT:			
		return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
						(CBytePtrType) (g_xPingString), 
						(AsnLengthType) strlen ((char *) (g_xPingString)));
					
																														
	default:
		//return ((AsnIdType) 0);		
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, 2);
				
	}
	
}



void RMping_Mib_Init(void)
{
	struct timeval now;
	int i;
	
	gettimeofday(&now, 0);

//	printf("Now:%u:%u\n",now.tv_sec, now.tv_usec);

	if((now.tv_sec - RMping_last_scan) > 10)	//  10 sec
	{		
		
		unsigned int oid_tbl_size;
		struct mib_oid * oid_ptr;
		
		RMping_last_scan = now.tv_sec;			

		// create oid table
		free_mib_tbl(&RMping_mib_oid_tbl);
		
		// RMping MIB has 6 object
		oid_tbl_size = 6;
		create_mib_tbl(&RMping_mib_oid_tbl,  oid_tbl_size, 2); 
		oid_ptr = RMping_mib_oid_tbl.oid;
		
		for (i=1; i<=oid_tbl_size; i++) {
			oid_ptr->length = 2;
			oid_ptr->name[0] = i;
			oid_ptr->name[1] = 0;						
			oid_ptr++;	
		}				
		
	}
}



static	MixStatusType	RMpingRelease ()
{
	
	return (smpErrorReadOnly);
}

static	MixStatusType	RMpingCreate ()
{
	
	return (smpErrorReadOnly);
}

static	MixStatusType	RMpingDestroy ()
{
	
	return (smpErrorReadOnly);
}




static	AsnIdType	RMpingNext (MixCookieType cookie, MixNamePtrType name, MixLengthPtrType namelenp)
{
	unsigned int idx;

	RMping_Mib_Init();	
	
	cookie = cookie;
	
	if(snmp_oid_getnext(&RMping_mib_oid_tbl, name, *namelenp, &idx))
	{
		struct mib_oid * oid_ptr = RMping_mib_oid_tbl.oid;
		oid_ptr += idx;
		
		memcpy((unsigned char *)name, oid_ptr->name, oid_ptr->length);
		*namelenp = oid_ptr->length;
		
		return RMpingRetrieveMibValue(idx);
	}
	
	
	return ((AsnIdType) 0);
}


static	AsnIdType	RMpingGet (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen)
{
	unsigned int idx;
	
	//printf("RMpingGet\n");
	
	RMping_Mib_Init();
	
	cookie = cookie;
	if(snmp_oid_get(&RMping_mib_oid_tbl, name, namelen, &idx))
	{		
		return RMpingRetrieveMibValue(idx);		
	}	
	
	return ((AsnIdType) 0);
	
}


static	MixStatusType	RMpingSet  (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen, AsnIdType asn)
{
	unsigned int idx;	
	
	RMping_Mib_Init();	
	cookie = cookie;
	name = name;
	namelen = namelen;
	asn = asn;	
	
	
	
	if(snmp_oid_get(&RMping_mib_oid_tbl, name, namelen, &idx))
	{	
		struct mib_oid * oid_ptr;				
		AsnLengthType		k;
		int option;
		long i;		
		unsigned char strvalue[64];
		struct timeval now;
		
		
		oid_ptr = RMping_mib_oid_tbl.oid;
		oid_ptr += idx;

		//printf("RMpingSet: oid_ptr->name[0] = %d\n", oid_ptr->name[0]);
		
		switch (oid_ptr->name[0])
		{
		
		case CPEPINGADDR:
			//printf("RMpingSet: Set CPEPINGADDR\n");
			k = asnLength (asn);		
			(void) asnContents (asn, strvalue, k);	
			strvalue[k]=NULL;
			memset(g_pingStr, 0, sizeof(g_pingStr));
			strcpy(g_pingStr, strvalue);						
			return (smpErrorNone);
			
		case CPEPINGPKGSIZE:
			//printf("RMpingSet: Set CPEPINGPKGSIZE\n");
			i =  asnNumber (asnValue (asn), asnLength (asn));
			// Kaohj
			//g_DEFDATALEN = i - 8;			
			g_DEFDATALEN = i;
			return (smpErrorNone);			
			
		case CPEPINGCOUNT:
			//printf("RMpingSet: Set CPEPINGCOUNT\n");	
			i =  asnNumber (asnValue (asn), asnLength (asn));			
			g_PINGCOUNT = i;					
			return (smpErrorNone);

			
		case CPEPINGADMINSTATUS:
			//printf("RMpingSet: Set CPEPINGADMINSTATUS\n");
			i =  asnNumber (asnValue (asn), asnLength (asn));
			pingAdminStatus = i;
			
			if ( i == 2 ) {
				if (g_ping_flag == 0) {
					g_ping_flag = 1;
					//printf("RMpingSet: Do Ping Test\n");					
	                        	
					gettimeofday(&now, 0);
					RMping_test_last_scan = now.tv_sec;
				}
				
			}
			return (smpErrorNone);			
				
				
		default:
			return (smpErrorReadOnly);			
		}
		
			
		
	}	
	
}

static	MixOpsType	RMpingOps = {

			RMpingRelease,
			RMpingCreate,
			RMpingDestroy,
			RMpingNext,
			RMpingGet,			
			RMpingSet

			};

CVoidType		RMpingInit (void)
{
	(void) misExport ((MixNamePtrType) "\53\6\1\4\1\201\204\114\5\1",
		(MixLengthType) 10, & RMpingOps, (MixCookieType) 0);
}
