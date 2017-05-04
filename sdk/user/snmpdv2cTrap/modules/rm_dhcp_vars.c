

#include 	<net/if.h>
#include	<stdio.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<time.h>
#if 1
#include	"ctypes.h"
#include	"error.h"
#include	"local.h"
#include	"mix.h"
#include	"mis.h"
#include	"asn.h"
#endif
#include 	"mib_tool.h"

//#include "../../boa/src/LINUX/utility.h"
//#include <semaphore.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/if_ether.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>


enum snmp_cpe_landhcp
{	   
   CPELANDHCPADMINSTATUS = 1,
   CPE_LANDHCP_END 
};

static struct mib_oid_tbl rmdhcp_mib_oid_tbl;
static long rmdhcp_last_scan = 0;
	

static	AsnIdType	rmdhcpRetrieveMibValue (unsigned int mibIdx)
{
	struct mib_oid * oid_ptr;			
	int i;		
	unsigned char vChar;
	
	oid_ptr = rmdhcp_mib_oid_tbl.oid;
	oid_ptr += mibIdx;			

	
	switch (oid_ptr->name[0])
	{
		
#ifdef CONFIG_USER_DHCP_SERVER
	case CPELANDHCPADMINSTATUS:		
		mib_get( MIB_DHCP_MODE, (void *)&vChar);
		
		
		if ( vChar == 2 )	
			return asnUnsl (asnClassUniversal, (AsnTagType) 2,2 );	// DHCP Server enable
		else
			return asnUnsl (asnClassUniversal, (AsnTagType) 2,1 );	// DHCP Server disable
#endif
	 																														
	default:
		//return ((AsnIdType) 0);
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, 1);
				
	}
	
}



void rmdhcp_Mib_Init(void)
{
	struct timeval now;	
	
	gettimeofday(&now, 0);

//	printf("Now:%u:%u\n",now.tv_sec, now.tv_usec);

	if((now.tv_sec - rmdhcp_last_scan) > 10)	//  10 sec
	{		
		
		unsigned int oid_tbl_size;
		struct mib_oid * oid_ptr;
		
		rmdhcp_last_scan = now.tv_sec;			

		// create oid table
		free_mib_tbl(&rmdhcp_mib_oid_tbl);		
		
		// rmdhcp MIB has 1 objects
		oid_tbl_size = 1;
		
		create_mib_tbl(&rmdhcp_mib_oid_tbl,  oid_tbl_size, 2); 
		oid_ptr = rmdhcp_mib_oid_tbl.oid;								
																			
		oid_ptr->length = 2;
		oid_ptr->name[0] = 1;
		oid_ptr->name[1] = 0; 		
				
	}
}



static	MixStatusType	rmdhcpRelease ()
{
	
	return (smpErrorReadOnly);
}

static	MixStatusType	rmdhcpCreate ()
{
	
	return (smpErrorReadOnly);
}

static	MixStatusType	rmdhcpDestroy ()
{
	
	return (smpErrorReadOnly);
}




static	AsnIdType	rmdhcpNext (MixCookieType cookie, MixNamePtrType name, MixLengthPtrType namelenp)
{
	unsigned int idx=0;
		
	
	rmdhcp_Mib_Init();	
	
	cookie = cookie;	
	
	if(snmp_oid_getnext(&rmdhcp_mib_oid_tbl, name, *namelenp, &idx))
	{		
		struct mib_oid * oid_ptr = rmdhcp_mib_oid_tbl.oid;
		oid_ptr += idx;

		memcpy((unsigned char *)name, oid_ptr->name, oid_ptr->length);		
		*namelenp = oid_ptr->length;		
		
		return rmdhcpRetrieveMibValue(idx);
	}
	
	return ((AsnIdType) 0);
}


static	AsnIdType	rmdhcpGet (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen)
{
	unsigned int idx;	
	
			
	rmdhcp_Mib_Init();
	
	cookie = cookie;
	
	if(snmp_oid_get(&rmdhcp_mib_oid_tbl, name, namelen, &idx))
	{			
		if ( namelen != 0 ){			       		       	
			return rmdhcpRetrieveMibValue(idx);  
		}else	
			return ((AsnIdType) 0);	
			
	}		
	
	printf("rmdhcpGet: Cannot find the OID\n");
	return ((AsnIdType) 0);
	
}


static	MixStatusType	rmdhcpSet  (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen, AsnIdType asn)
{
	unsigned int idx;	
	long i;		
	
	
	rmdhcp_Mib_Init();	
	
	cookie = cookie;
	name = name;
	namelen = namelen;
	asn = asn;	
	
	//for ( i=0; i<6; i++)
	//		printf("rmdhcpSet: name[%d] = %d\n", i, name[i]);
			
	if(snmp_oid_get(&rmdhcp_mib_oid_tbl, name, namelen, &idx))
	{	
		struct mib_oid * oid_ptr;		
		unsigned char vChar;
		DHCP_TYPE_T dtmode;
		
		oid_ptr = rmdhcp_mib_oid_tbl.oid;
		oid_ptr += idx;		
		
		
		switch (oid_ptr->name[0])
		{
			
#ifdef CONFIG_USER_DHCP_SERVER
		case CPELANDHCPADMINSTATUS:						
			i =  asnNumber (asnValue (asn), asnLength (asn));
			//printf("rmdhcpSet: i=0x%x\n", i);
			
			if ( i == 2) {				
				dtmode = DHCP_LAN_SERVER;   // Enable DHCP Server
			}else
				dtmode = DHCP_LAN_NONE;     // Disable DHCP Server
			
			vChar = (unsigned char) dtmode;	
			mib_set(MIB_DHCP_MODE, (void *)&vChar);			
			return (smpErrorNone);	
#endif
		
				
		default:
			return (smpErrorReadOnly);			
		}
				
	}		
	
		
	
}

static	MixOpsType	rmdhcpOps = {

			rmdhcpRelease,
			rmdhcpCreate,
			rmdhcpDestroy,
			rmdhcpNext,
			rmdhcpGet,			
			rmdhcpSet

			};

CVoidType		rmdhcpInit (void)
{
	
	(void) misExport ((MixNamePtrType) "\53\6\1\4\1\201\204\114\2\6",
		(MixLengthType) 10, & rmdhcpOps, (MixCookieType) 0);
		
		
}
