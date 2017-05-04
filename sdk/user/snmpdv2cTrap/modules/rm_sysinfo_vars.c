
#include 	<net/if.h>
#include	<stdio.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<time.h>

#include	"ctypes.h"
#include	"error.h"
#include	"local.h"
//#include	"iface_vars.h"
#include	"mix.h"
#include	"mis.h"
#include	"asn.h"

#include 	"mib_tool.h"

//#include "../../boa/src/LINUX/mib.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/if_ether.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>

#include "../../boa/src/LINUX/options.h"
#include "../../boa/src/defs.h"


char ConfigID[10]="0000";
char *productID = "Realtek1000";
char *vendorID = "Realtek200";
char *systemVersion = "Realtek-ADSL-V1";
char *mibVersion = "V2.00";
char *serialNumber = "23456";
char manufactureOUI[4];
//char *manufactureOUI = "123";



static struct mib_oid_tbl sysinfo_mib_oid_tbl;
static long sysinfo_last_scan = 0;


static	AsnIdType	sysinfoRetrieveMibValue (unsigned int mibIdx)
{
	struct mib_oid * oid_ptr;
	unsigned long value;
	char *string;		
	int fd;
	struct ifreq    ifrq;	
	struct sysinfo info;
	
	
	oid_ptr = sysinfo_mib_oid_tbl.oid;
	oid_ptr += mibIdx;			
	
	//printf("sysinfoGet: oid_ptr->name[0] = %d\n", oid_ptr->name[0]);
	
	switch (oid_ptr->name[0])
	{
	case 1:	
		sysinfo(&info);						
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, (int) info.uptime);  
				
		
	case 2:
		return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
						(CBytePtrType) (ConfigID), 
						(AsnLengthType) strlen ((char *) (ConfigID)));	
	case 3:
	{
		FILE *fp;
		char strVer[64], *strBld;			
			
		strVer[0]=0;
		strBld = 0;
		fp = fopen("/etc/version", "r");
		if (fp!=NULL) {
			fgets(strVer, sizeof(strVer), fp);  //main version
			fclose(fp);
			strBld = strchr(strVer, ' ');
			*strBld=0;
		}	
			
		return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
						(CBytePtrType) (strVer), 
						(AsnLengthType) strlen ((char *) (strVer)));	
	}
	case 4:
		return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
						(CBytePtrType) (productID), 
						(AsnLengthType) strlen ((char *) (productID)));			
	case 5:
		return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
						(CBytePtrType) (vendorID), 
						(AsnLengthType) strlen ((char *) (vendorID)));
	case 6:
		return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
						(CBytePtrType) (systemVersion), 
						(AsnLengthType) strlen ((char *) (systemVersion)));	
	case 7:
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, 2);  // router
								 
	case 8:
		return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
						(CBytePtrType) (mibVersion), 
						(AsnLengthType) strlen ((char *) (mibVersion)));
	 
	case 9:
		return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
						(CBytePtrType) (serialNumber), 
						(AsnLengthType) strlen ((char *) (serialNumber)));
	case 10:		
			
		return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
						(CBytePtrType) (manufactureOUI), 
						(AsnLengthType)3);
			
		
		/*			
		return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
						(CBytePtrType) (manufactureOUI), 
						(AsnLengthType)strlen ((char *) (manufactureOUI)));
		*/				
																																			
	default:
		return ((AsnIdType) 0);
		//return asnUnsl (asnClassUniversal, (AsnTagType) 2, 2);
				
	}
	
}



void sysinfo_Mib_Init(void)
{
	struct timeval now;
	int i;
	
	gettimeofday(&now, 0);

//	printf("Now:%u:%u\n",now.tv_sec, now.tv_usec);

	if((now.tv_sec - sysinfo_last_scan) > 10)	//  10 sec
	{		
		
		unsigned int oid_tbl_size;
		struct mib_oid * oid_ptr;
		
		sysinfo_last_scan = now.tv_sec;			

		// create oid table
		free_mib_tbl(&sysinfo_mib_oid_tbl);
		
		// sysinfo MIB has 10 object
		oid_tbl_size = 10;
		create_mib_tbl(&sysinfo_mib_oid_tbl,  oid_tbl_size, 2); 
		oid_ptr = sysinfo_mib_oid_tbl.oid;
		
		for (i=1; i<=oid_tbl_size; i++) {
			oid_ptr->length = 2;
			oid_ptr->name[0] = i;
			oid_ptr->name[1] = 0;						
			oid_ptr++;	
		}				
		
	}
}



static	MixStatusType	sysinfoRelease ()
{
	
	return (smpErrorReadOnly);
}

static	MixStatusType	sysinfoCreate ()
{
	
	return (smpErrorReadOnly);
}

static	MixStatusType	sysinfoDestroy ()
{
	
	return (smpErrorReadOnly);
}




static	AsnIdType	sysinfoNext (MixCookieType cookie, MixNamePtrType name, MixLengthPtrType namelenp)
{
	unsigned int idx;

	sysinfo_Mib_Init();	
	
	cookie = cookie;
	if(snmp_oid_getnext(&sysinfo_mib_oid_tbl, name, *namelenp, &idx))
	{
		struct mib_oid * oid_ptr = sysinfo_mib_oid_tbl.oid;
		oid_ptr += idx;

		memcpy((unsigned char *)name, oid_ptr->name, oid_ptr->length);
		*namelenp = oid_ptr->length;
	
		return sysinfoRetrieveMibValue(idx);
	}

	return ((AsnIdType) 0);
}


static	AsnIdType	sysinfoGet (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen)
{
	unsigned int idx;

	sysinfo_Mib_Init();
	
	cookie = cookie;
	if(snmp_oid_get(&sysinfo_mib_oid_tbl, name, namelen, &idx))
	{		
		return sysinfoRetrieveMibValue(idx);		
	}	
	
	printf("sysinfoGet: Cannot find the OID\n");
	return ((AsnIdType) 0);
	
}


static	MixStatusType	sysinfoSet  (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen, AsnIdType asn)
{
	unsigned int idx;
	unsigned char *strPtr;
	unsigned long strPtrAddr;
	AsnLengthType		k;
	unsigned char strvalue[10];
	
	sysinfo_Mib_Init();	
	cookie = cookie;
	name = name;
	namelen = namelen;
	asn = asn;	
	
	
	
	if(snmp_oid_get(&sysinfo_mib_oid_tbl, name, namelen, &idx))
	{	
		struct mib_oid * oid_ptr;				
		AsnLengthType		k;
		int option;
		
		oid_ptr = sysinfo_mib_oid_tbl.oid;
		oid_ptr += idx;

		//printf("sysinfoSet: oid_ptr->name[0] = %d\n", oid_ptr->name[0]);
		
		switch (oid_ptr->name[0])
		{
		case 2:			
			k = asnLength (asn);		
			(void) asnContents (asn, strvalue, k);	
			strvalue[k]=NULL;					
			
			//printf("Input String(1) is %s\n", strvalue);	
			strcpy(ConfigID, strvalue);			
			return (smpErrorNone);
		}
		
			
		return (smpErrorReadOnly);
	}	
	
}

static	MixOpsType	sysinfoOps = {

			sysinfoRelease,
			sysinfoCreate,
			sysinfoDestroy,
			sysinfoNext,
			sysinfoGet,			
			sysinfoSet

			};

CVoidType		sysinfoInit (void)
{
	
	manufactureOUI[0]= 0x0;
	manufactureOUI[1]= 0xe0;
	manufactureOUI[2]= 0x4d;
	manufactureOUI[3]= '\0';
	
	
	(void) misExport ((MixNamePtrType) "\53\6\1\4\1\201\204\114\1",
		(MixLengthType) 9, & sysinfoOps, (MixCookieType) 0);
}
