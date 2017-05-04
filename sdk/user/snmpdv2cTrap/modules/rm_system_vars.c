

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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/if_ether.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>


char cpeRsvdString[256];
int sysRestore = 1;

static struct mib_oid_tbl RMsystem_mib_oid_tbl;
static long RMsystem_last_scan = 0;

extern int g_reboot_flag;
extern int g_save_flag;
extern int g_reboot2def_flag;
extern btrap_t 	g_snmpBootTrapResponse;
extern int deleteAllPvc();

static	AsnIdType	RMsystemRetrieveMibValue (unsigned int mibIdx)
{
	struct mib_oid * oid_ptr;
	unsigned long value;
	char *string;		
	int fd;
	struct ifreq    ifrq;	
	struct sysinfo info;
	
	
	oid_ptr = RMsystem_mib_oid_tbl.oid;
	oid_ptr += mibIdx;	
	
	
	switch (oid_ptr->name[0])
	{
	case 1:							
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, 1);  		
		
	case 2:
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, 1);	
	case 3:
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, sysRestore);
	case 4:
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, 1);
					
	case 5:
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, g_snmpBootTrapResponse);
	case 6:			
		return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
						(CBytePtrType) (cpeRsvdString), 
						(AsnLengthType) strlen ((char *) (cpeRsvdString)));																													
	default:
		return ((AsnIdType) 0);		
		//return asnUnsl (asnClassUniversal, (AsnTagType) 2, 2);
				
	}
	
}



void RMsystem_Mib_Init(void)
{
	struct timeval now;
	int i;
	
	gettimeofday(&now, 0);

//	printf("Now:%u:%u\n",now.tv_sec, now.tv_usec);

	if((now.tv_sec - RMsystem_last_scan) > 10)	//  10 sec
	{		
		
		unsigned int oid_tbl_size;
		struct mib_oid * oid_ptr;
		
		RMsystem_last_scan = now.tv_sec;			

		// create oid table
		free_mib_tbl(&RMsystem_mib_oid_tbl);
		
		// RMsystem MIB has 6 object
		oid_tbl_size = 6;
		create_mib_tbl(&RMsystem_mib_oid_tbl,  oid_tbl_size, 2); 
		oid_ptr = RMsystem_mib_oid_tbl.oid;
		
		for (i=1; i<=oid_tbl_size; i++) {
			oid_ptr->length = 2;
			oid_ptr->name[0] = i;
			oid_ptr->name[1] = 0;						
			oid_ptr++;	
		}				
		
	}
}



static	MixStatusType	RMsystemRelease ()
{
	
	return (smpErrorReadOnly);
}

static	MixStatusType	RMsystemCreate ()
{
	
	return (smpErrorReadOnly);
}

static	MixStatusType	RMsystemDestroy ()
{
	
	return (smpErrorReadOnly);
}




static	AsnIdType	RMsystemNext (MixCookieType cookie, MixNamePtrType name, MixLengthPtrType namelenp)
{
	unsigned int idx;

	RMsystem_Mib_Init();	
	
	cookie = cookie;
	
	if(snmp_oid_getnext(&RMsystem_mib_oid_tbl, name, *namelenp, &idx))
	{
		struct mib_oid * oid_ptr = RMsystem_mib_oid_tbl.oid;
		oid_ptr += idx;
		
		memcpy((unsigned char *)name, oid_ptr->name, oid_ptr->length);
		*namelenp = oid_ptr->length;
		
		return RMsystemRetrieveMibValue(idx);
	}
	
	
	return ((AsnIdType) 0);
}


static	AsnIdType	RMsystemGet (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen)
{
	unsigned int idx;

	RMsystem_Mib_Init();
	
	cookie = cookie;
	if(snmp_oid_get(&RMsystem_mib_oid_tbl, name, namelen, &idx))
	{		
		return RMsystemRetrieveMibValue(idx);		
	}	
	
	return ((AsnIdType) 0);
	
}


static	MixStatusType	RMsystemSet  (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen, AsnIdType asn)
{
	unsigned int idx;	
	
	RMsystem_Mib_Init();	
	cookie = cookie;
	name = name;
	namelen = namelen;
	asn = asn;	
	
	
	
	if(snmp_oid_get(&RMsystem_mib_oid_tbl, name, namelen, &idx))
	{	
		struct mib_oid * oid_ptr;				
		AsnLengthType		k;
		int option;
		long i;		
		unsigned char strvalue[256];
	
		oid_ptr = RMsystem_mib_oid_tbl.oid;
		oid_ptr += idx;

		//printf("RMsystemSet: oid_ptr->name[0] = %d\n", oid_ptr->name[0]);
		
		switch (oid_ptr->name[0])
		{
		case 1:
			i =  asnNumber (asnValue (asn), asnLength (asn));
			if ( i == 1 ) {
				g_reboot_flag = 1;
			}						
			return (smpErrorNone);
			
		case 2:
			i =  asnNumber (asnValue (asn), asnLength (asn));
			if ( i == 1 ) {
				g_save_flag = 1;
				return (smpErrorNone);
			}
			
			
		case 3:	
			i =  asnNumber (asnValue (asn), asnLength (asn));
			
			if ( i == 1 ) {				
				printf("RMsystemSet: Delete all PVC configuration(1)\n");
				sysRestore = 1;				
				deleteAllPvc();
			}else if (i == 2){	
				//printf("RMsystemSet: Reboot to default\n");	
				sysRestore = 2;
				g_reboot2def_flag = 1;					
			}					
			return (smpErrorNone);
			
		case 4:
			return (smpErrorNone);
			
		case 5:
			i =  asnNumber (asnValue (asn), asnLength (asn));
			g_snmpBootTrapResponse = i;
			return (smpErrorNone);
			
		case 6:
			k = asnLength (asn);		
			(void) asnContents (asn, strvalue, k);	
			strvalue[k]=NULL;
			strcpy(cpeRsvdString, strvalue);
			return (smpErrorNone);
				
		default:
			return (smpErrorReadOnly);			
		}
		
			
		
	}	
	
}

static	MixOpsType	RMsystemOps = {

			RMsystemRelease,
			RMsystemCreate,
			RMsystemDestroy,
			RMsystemNext,
			RMsystemGet,			
			RMsystemSet

			};

CVoidType		RMsystemInit (void)
{
	(void) misExport ((MixNamePtrType) "\53\6\1\4\1\201\204\114\2\1",
		(MixLengthType) 10, & RMsystemOps, (MixCookieType) 0);
}
