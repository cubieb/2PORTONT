

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

extern char g_ftpIp[20];
extern char g_ftpUser[128];
extern char g_ftpPasswd[128];
extern char g_ftpFile[256];
extern int g_ftp_upgrade_flag;
extern int g_ftp_test_flag;
// Kaohj
extern int	g_ftp_operStatus;
extern int	g_ftp_totalSize;
extern int	g_ftp_doneSize;
extern int	g_ftp_elapseTime;

enum snmp_cpe_ftptable
{ 
   CPEFTPIP = 1,
   CPEFTPUSER,
   CPEFTPPASSWD,
   CPEFTPFILENAME,
   CPEFTPADMINSTATUS,
   CPEFTPOPERSTATUS,
   CPEFTPTOTALSIZE = 8,
   CPEFTPDONESIZE,
   CPEFTPELAPSETIME,
   CPE_FTPTABLE_END 
};

extern const char TOTALSIZE[];
extern const char ELAPSETIME[];
extern const char OPERSTATUS[];

static struct mib_oid_tbl RMftp_mib_oid_tbl;
static long RMftp_last_scan = 0;
extern long ftpAdmin;


#if 0
unsigned long RMFtpRetrieveValueFromFile(const char * compareStr, int offset)
{
	FILE *fp;
	char temps[0x100];
	unsigned long value;
	char *str, *endptr;
	
	value = 0;
	
	if ((fp = fopen("/var/ftpStatus.txt", "r")) == NULL)
	{		
		if ( strcmp("OperStatus:", compareStr) == 0 ) {			
			value = 1;
			return value;
		} else 	
			return value;
	}	
	
	
	while (fgets(temps,0x100,fp))
	{
		if (temps[strlen(temps)-1]=='\n')
			temps[strlen(temps)-1] = 0;
		
		if (str=strstr(temps, compareStr))
		{			
			//printf("The string is %s\n", str+offset);			
			value = strtoul(str+offset, &endptr, 10);				
			
		} else {					
			continue;
			
	        }
		
	}
	
	fclose(fp);
	return value;	
	
}
#endif


static	AsnIdType	RMftpRetrieveMibValue (unsigned int mibIdx)
{
	struct mib_oid * oid_ptr;
	unsigned long value;
	char *string;		
	int fd;
	struct ifreq    ifrq;	
	struct sysinfo info;
	char getfileName[100];
	long oper;
	FILE *fp;
	struct in_addr outAddr;
	char p1[4];
	
	oid_ptr = RMftp_mib_oid_tbl.oid;
	oid_ptr += mibIdx;	
	
	
	switch (oid_ptr->name[0])
	{
		
	case CPEFTPIP:	
		// Mason Yu
		inet_aton(g_ftpIp, &outAddr);
		p1[0]=*(char *)(&(outAddr.s_addr)); 
		p1[1]=*((char *)(&(outAddr.s_addr)) + 1); 
		p1[2]=*((char *)(&(outAddr.s_addr)) + 2);
		p1[3]=*((char *)(&(outAddr.s_addr)) + 3);
		
		// Return Data type is IpAddress
		return asnOctetString (asnClassApplication, (AsnTagType) 0, 
						(CBytePtrType) p1, 
						(AsnLengthType) 4);
		/*	
		// Return Data type is string									
		return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
						(CBytePtrType) (g_ftpIp), 
						(AsnLengthType) strlen (g_ftpIp)); 		
		*/
		
	case CPEFTPUSER:
		return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
						(CBytePtrType) (g_ftpUser), 
						(AsnLengthType) strlen ((char *) (g_ftpUser)));	
		
	case CPEFTPPASSWD:
		return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
						(CBytePtrType) (g_ftpPasswd), 
						(AsnLengthType) strlen ((char *) (g_ftpPasswd)));
		
	case CPEFTPFILENAME:
		return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
						(CBytePtrType) (g_ftpFile), 
						(AsnLengthType) strlen ((char *) (g_ftpFile)));
					
	case CPEFTPADMINSTATUS:
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, ftpAdmin);      
		
	case CPEFTPOPERSTATUS:
		// Kaohj
		//value=RMFtpRetrieveValueFromFile(OPERSTATUS, strlen(OPERSTATUS)+1);
		value = g_ftp_operStatus;
		
		/*
		if ( value == 2) {
			sprintf(getfileName, "/tmp/%s", g_ftpFile);
			if ( (fp = fopen(getfileName, "r")) == NULL )
			{				
				value=6;
			}
			else
				fclose(fp);
		}
		*/					
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, value);
		
	case CPEFTPTOTALSIZE:
		// Kaohj
		//value=RMFtpRetrieveValueFromFile(TOTALSIZE, strlen(TOTALSIZE)+1);
		value = g_ftp_totalSize;
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, value);
		
	case CPEFTPDONESIZE:
		// Kaohj
		//value=RMFtpRetrieveValueFromFile(TOTALSIZE, strlen(TOTALSIZE)+1);
		value = g_ftp_doneSize;
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, value);
		
	case CPEFTPELAPSETIME:
		// Kaohj
		//value=RMFtpRetrieveValueFromFile(ELAPSETIME, strlen(ELAPSETIME)+1);
		value = g_ftp_elapseTime;
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, value);			
																														
	default:
		//return ((AsnIdType) 0);		
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, 2);
				
	}
	
}



void RMftp_Mib_Init(void)
{
	struct timeval now;
	int i;
	
	gettimeofday(&now, 0);

//	printf("Now:%u:%u\n",now.tv_sec, now.tv_usec);

	if((now.tv_sec - RMftp_last_scan) > 10)	//  10 sec
	{		
		
		unsigned int oid_tbl_size;
		struct mib_oid * oid_ptr;
		
		RMftp_last_scan = now.tv_sec;			

		// create oid table
		free_mib_tbl(&RMftp_mib_oid_tbl);
		
		// RMftp MIB has 10 object
		oid_tbl_size = 10;
		create_mib_tbl(&RMftp_mib_oid_tbl,  oid_tbl_size, 2); 
		oid_ptr = RMftp_mib_oid_tbl.oid;
		
		for (i=1; i<=oid_tbl_size; i++) {
			oid_ptr->length = 2;
			oid_ptr->name[0] = i;
			oid_ptr->name[1] = 0;						
			oid_ptr++;	
		}				
		
	}
}



static	MixStatusType	RMftpRelease ()
{
	
	return (smpErrorReadOnly);
}

static	MixStatusType	RMftpCreate ()
{
	
	return (smpErrorReadOnly);
}

static	MixStatusType	RMftpDestroy ()
{
	
	return (smpErrorReadOnly);
}




static	AsnIdType	RMftpNext (MixCookieType cookie, MixNamePtrType name, MixLengthPtrType namelenp)
{
	unsigned int idx;

	RMftp_Mib_Init();	
	
	cookie = cookie;
	
	if(snmp_oid_getnext(&RMftp_mib_oid_tbl, name, *namelenp, &idx))
	{
		struct mib_oid * oid_ptr = RMftp_mib_oid_tbl.oid;
		oid_ptr += idx;
		
		memcpy((unsigned char *)name, oid_ptr->name, oid_ptr->length);
		*namelenp = oid_ptr->length;
		
		return RMftpRetrieveMibValue(idx);
	}
	
	
	return ((AsnIdType) 0);
}


static	AsnIdType	RMftpGet (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen)
{
	unsigned int idx;
	
	//printf("RMftpGet\n");
	
	RMftp_Mib_Init();
	
	cookie = cookie;
	if(snmp_oid_get(&RMftp_mib_oid_tbl, name, namelen, &idx))
	{		
		return RMftpRetrieveMibValue(idx);		
	}	
	
	return ((AsnIdType) 0);
	
}


static	MixStatusType	RMftpSet  (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen, AsnIdType asn)
{
	unsigned int idx;	
	
	RMftp_Mib_Init();	
	cookie = cookie;
	name = name;
	namelen = namelen;
	asn = asn;	
	
	
	
	if(snmp_oid_get(&RMftp_mib_oid_tbl, name, namelen, &idx))
	{	
		struct mib_oid * oid_ptr;				
		AsnLengthType		k;
		int option;
		long i;			
		struct in_addr inAddr;
		char *temp;
		unsigned char strvalue[256];
		
		oid_ptr = RMftp_mib_oid_tbl.oid;
		oid_ptr += idx;

		//printf("RMftpSet: oid_ptr->name[0] = %d\n", oid_ptr->name[0]);
		
		switch (oid_ptr->name[0])
		{
		
		case CPEFTPIP:
			i =  asnNumber (asnValue (asn), asnLength (asn));			
			inAddr.s_addr = i;
			
			temp = inet_ntoa(inAddr);
			strcpy(g_ftpIp, temp);										
			return (smpErrorNone);
			
		case CPEFTPUSER:
			k = asnLength (asn);		
			(void) asnContents (asn, strvalue, k);	
			strvalue[k]=NULL;			
				
			strcpy(g_ftpUser, strvalue);			
			return (smpErrorNone);			
			
		case CPEFTPPASSWD:	
			k = asnLength (asn);		
			(void) asnContents (asn, strvalue, k);	
			strvalue[k]=NULL;			
				
			strcpy(g_ftpPasswd, strvalue);			
			return (smpErrorNone);

			
		case CPEFTPFILENAME:
			k = asnLength (asn);		
			(void) asnContents (asn, strvalue, k);	
			strvalue[k]=NULL;			
				
			strcpy(g_ftpFile, strvalue);			
			return (smpErrorNone);			
				
		case CPEFTPADMINSTATUS:
			i =  asnNumber (asnValue (asn), asnLength (asn));
			ftpAdmin = i;
			
			if ( i == 2 ) {
				if (g_ftp_test_flag == 0 && g_ftp_upgrade_flag == 0)
					g_ftp_upgrade_flag = 1;
				//printf("RMftpSet: Do FTP Upgrade(HI)\n");				
			}else if ( i == 3 ) {
				if (g_ftp_test_flag == 0 && g_ftp_upgrade_flag == 0)
					g_ftp_test_flag = 1;
			}	
			return (smpErrorNone);	
			
				
		default:
			return (smpErrorReadOnly);			
		}
		
			
		
	}	
	
}

static	MixOpsType	RMftpOps = {

			RMftpRelease,
			RMftpCreate,
			RMftpDestroy,
			RMftpNext,
			RMftpGet,			
			RMftpSet

			};

CVoidType		RMftpInit (void)
{
	(void) misExport ((MixNamePtrType) "\53\6\1\4\1\201\204\114\4",
		(MixLengthType) 9, & RMftpOps, (MixCookieType) 0);
}
