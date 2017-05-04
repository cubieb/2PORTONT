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

static struct mib_oid_tbl rmadslLine_mib_oid_tbl;
static long rmadslLine_last_scan = 0;


static	AsnIdType	rmadslLineRetrieveMibValue (unsigned int mibIdx)
{
	struct mib_oid * oid_ptr;			
	int i;		
	unsigned char vChar;
	unsigned char buffer[64];
	unsigned char strbuf[64];
	Modem_Config vMc;
	int xmode;
	int vInt1;
	unsigned char vInt2;
	
	oid_ptr = rmadslLine_mib_oid_tbl.oid;
	oid_ptr += mibIdx;			

	
	switch (oid_ptr->name[0])
	{
		
	case 1:	
		//printf("Get ADSL Line Trellis\n");
		if(adsl_drv_get(RLCM_MODEM_READ_CONFIG, (void *)&vMc, RLCM_MODEM_CONFIG_SIZE)) {
		 	if ( vMc.TrellisEnable == 1 )
            			return asnUnsl (asnClassUniversal, (AsnTagType) 2, 1);   // enable:1 , Disable:2 
            		else	       	            				
				return asnUnsl (asnClassUniversal, (AsnTagType) 2, 2);   // enable:1 , Disable:2
		}	
		
	case 2:
		//printf("Get ADSL Line HandShak\n");	
#ifdef CONFIG_VDSL
{
		int msgval[4],mval;;

		vInt1=0;
		dsl_msg_get_array(GetHskXdslMode,msgval);
		//printf("Get pmdmode=0x%08x, profile=0x%08x\n", msgval[0], msgval[1]);
		mval=msgval[0];
		if( mval&(MODE_ANSI|MODE_ETSI) )
			vInt1 |= 0x1; // T1.413
		if( (mval&MODE_MASK)&~(MODE_ANSI|MODE_ETSI) )
			vInt1 |= 0x2; // G.dmt
}
#else
		adsl_drv_get(RLCM_GET_ADSL_MODE, (void *)&vInt1, 4);
		adsl_drv_get(RLCM_GET_XDSL_MODE, (void *)&vInt2, 1);
		
		//printf("vInt1=%d vInt2=%d\n", vInt1, vInt2);
		
		if ( vInt2 > 1 )
			vInt1= vInt1 | 0x02;
			
		if ( (vInt2 & 1) == 0 )
			vInt1= vInt1 & 0x2;

			
		vInt1 = vInt1 & 3;
#endif /*CONFIG_VDSL*/

		if ( vInt1 == 1 )
			return asnUnsl (asnClassUniversal, (AsnTagType) 2, 4);   // T1.413
		else if ( vInt1 == 2 ) 	
			return asnUnsl (asnClassUniversal, (AsnTagType) 2, 5);   // G.dmt
		else
			return asnUnsl (asnClassUniversal, (AsnTagType) 2, 7); 	 // Autosense			
		   
	
	case 3:
		if (adsl_drv_get(RLCM_GET_OLR_TYPE, (void *)&xmode, 8)) {
			//printf("xmode=%d\n", xmode);
			if ( xmode  != 0 )
				return asnUnsl (asnClassUniversal, (AsnTagType) 2, 1);   // enable:1 , Disable:2
			else
				return asnUnsl (asnClassUniversal, (AsnTagType) 2, 2);   // enable:1 , Disable:2	
		}	
	 	return asnUnsl (asnClassUniversal, (AsnTagType) 2, 2);   // enable:1 , Disable:2
	 																														
	default:
		//return ((AsnIdType) 0);
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, 1);
				
	}
	
}



void rmadslLine_Mib_Init(void)
{
	struct timeval now;	
	
	gettimeofday(&now, 0);

//	printf("Now:%u:%u\n",now.tv_sec, now.tv_usec);

	if((now.tv_sec - rmadslLine_last_scan) > 10)	//  10 sec
	{		
		
		unsigned int oid_tbl_size;
		struct mib_oid * oid_ptr;
		
		rmadslLine_last_scan = now.tv_sec;			

		// create oid table
		free_mib_tbl(&rmadslLine_mib_oid_tbl);		
		
		// rmadslLine MIB has 3 objects
		oid_tbl_size = 3;
		
		create_mib_tbl(&rmadslLine_mib_oid_tbl,  oid_tbl_size, 2); 
		oid_ptr = rmadslLine_mib_oid_tbl.oid;								
																			
		oid_ptr->length = 2;
		oid_ptr->name[0] = 1;
		oid_ptr->name[1] = 0; 		
		oid_ptr++;
		
		oid_ptr->length = 2;
		oid_ptr->name[0] = 2;
		oid_ptr->name[1] = 0; 	
		oid_ptr++;
		
		oid_ptr->length = 2;
		oid_ptr->name[0] = 3;
		oid_ptr->name[1] = 0;
		
		oid_ptr++;	
	}
}



static	MixStatusType	rmadslLineRelease ()
{
	
	return (smpErrorReadOnly);
}

static	MixStatusType	rmadslLineCreate ()
{
	
	return (smpErrorReadOnly);
}

static	MixStatusType	rmadslLineDestroy ()
{
	
	return (smpErrorReadOnly);
}




static	AsnIdType	rmadslLineNext (MixCookieType cookie, MixNamePtrType name, MixLengthPtrType namelenp)
{
	unsigned int idx=0;
		
	
	rmadslLine_Mib_Init();	
	
	cookie = cookie;	
	
	if(snmp_oid_getnext(&rmadslLine_mib_oid_tbl, name, *namelenp, &idx))
	{		
		struct mib_oid * oid_ptr = rmadslLine_mib_oid_tbl.oid;
		oid_ptr += idx;

		memcpy((unsigned char *)name, oid_ptr->name, oid_ptr->length);		
		*namelenp = oid_ptr->length;		
		
		return rmadslLineRetrieveMibValue(idx);
	}
	
	return ((AsnIdType) 0);
}


static	AsnIdType	rmadslLineGet (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen)
{
	unsigned int idx;	
	
			
	rmadslLine_Mib_Init();
	
	cookie = cookie;
	
	if(snmp_oid_get(&rmadslLine_mib_oid_tbl, name, namelen, &idx))
	{			
		if ( namelen != 0 ){			       		       	
			return rmadslLineRetrieveMibValue(idx);  
		}else	
			return ((AsnIdType) 0);	
			
	}		
	
	printf("rmadslLineGet: Cannot find the OID\n");
	return ((AsnIdType) 0);
	
}


static	MixStatusType	rmadslLineSet  (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen, AsnIdType asn)
{
	unsigned int idx;		
	struct in_addr dns1, dns2;
	long i;
	
	
	rmadslLine_Mib_Init();	
	
	cookie = cookie;
	name = name;
	namelen = namelen;
	asn = asn;	
	
	//for ( i=0; i<6; i++)
	//		printf("rmadslLineSet: name[%d] = %d\n", i, name[i]);
			
	if(snmp_oid_get(&rmadslLine_mib_oid_tbl, name, namelen, &idx))
	{	
		struct mib_oid * oid_ptr;		
		unsigned char vChar;
		DHCP_TYPE_T dtmode;
		
		oid_ptr = rmadslLine_mib_oid_tbl.oid;
		oid_ptr += idx;		
		
		
		switch (oid_ptr->name[0])
		{
			
		case 1:
			i =  asnNumber (asnValue (asn), asnLength (asn));
			
			if ( i == 1 )
				adsl_drv_get(RLCM_ENABLE_TRELLIS, NULL, 0);
			else 
			 	adsl_drv_get(RLCM_DISABLE_TRELLIS, NULL, 0);	
			return (smpErrorNone);								
			
		
		case 2:			 					
			return (smpErrorNone);
		
		case 3 :
			i =  asnNumber (asnValue (asn), asnLength (asn));
			
			if ( i == 1 )
				adsl_drv_get(RLCM_ENABLE_BIT_SWAP, NULL, 0);
			else 
			 	adsl_drv_get(RLCM_DISABLE_BIT_SWAP, NULL, 0);	
			return (smpErrorNone);			
				
		default:
			return (smpErrorReadOnly);			
		}
				
	}		
	
		
	
}

static	MixOpsType	rmadslLineOps = {

			rmadslLineRelease,
			rmadslLineCreate,
			rmadslLineDestroy,
			rmadslLineNext,
			rmadslLineGet,			
			rmadslLineSet

			};

CVoidType		rmadslLineInit (void)
{
	
	(void) misExport ((MixNamePtrType) "\53\6\1\4\1\201\204\114\2\2",
		(MixLengthType) 10, & rmadslLineOps, (MixCookieType) 0);
		
		
}
