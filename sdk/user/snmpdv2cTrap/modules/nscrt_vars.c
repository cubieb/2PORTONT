
//#include <net/if.h>
#include	<stdio.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<time.h>
#include <math.h>

#include	"ctypes.h"
#include	"error.h"
#include	"local.h"
#include	"nscrt_vars.h"
#include	"mix.h"
#include	"mis.h"
#include	"asn.h"

#include "mib_tool.h"



#include "rtk/ponmac.h"
#include "rtk/gpon.h"
#include "rtk/epon.h"
#include "rtk/stat.h"
#include "rtk/switch.h"
#include "common/util/rt_util.h"

static struct mib_oid_tbl nscrt_mib_oid_tbl;

static long nscrt_last_scan = 0;


void nscrt_mib_Init(void)
{
	struct timeval now;
	gettimeofday(&now, 0);

	if((now.tv_sec - nscrt_last_scan) > 10)	//  10 sec
	{
		unsigned int if_idx;

		nscrt_last_scan = now.tv_sec;

		unsigned int oid_tbl_size;
		unsigned int idx;
		struct mib_oid *oid_ptr;
		// create oid table
		free_mib_tbl(&nscrt_mib_oid_tbl);

		oid_tbl_size = 8;
				
		create_mib_tbl(&nscrt_mib_oid_tbl,  oid_tbl_size, 6);
		oid_ptr = nscrt_mib_oid_tbl.oid;

		unsigned int oid_idx;
		
		//1.3.1.13 NSCRTCOMMONINTERNALTEMPERATURE
		oid_ptr->length = 3;
		oid_ptr->name[0] = 3;
		oid_ptr->name[1] = 1;
		oid_ptr->name[2] = 13;
		oid_ptr++;			
		
		//1.9.3.1.2 NSCRTDORINPUTPOWER
		oid_ptr->length = 4;
		oid_ptr->name[0] = 9;
		oid_ptr->name[1] = 3;
		oid_ptr->name[2] = 1;
		oid_ptr->name[3] = 2;
		oid_ptr++;		
		
		//1.9.5.1.4 NSCRTDOROUTPUTLEVEL
		oid_ptr->length = 4;
		oid_ptr->name[0] = 9;
		oid_ptr->name[1] = 5;
		oid_ptr->name[2] = 1;
		oid_ptr->name[3] = 4;
		oid_ptr++;			

		//1.9.8.1.2 NSCRTDORDCPOWERVOLTAGE
		oid_ptr->length = 4;
		oid_ptr->name[0] = 9;
		oid_ptr->name[1] = 8;
		oid_ptr->name[2] = 1;
		oid_ptr->name[3] = 2;
		oid_ptr++;			

		//1.9.11.4.1.2 gain control type
		oid_ptr->length = 5;
		oid_ptr->name[0] = 9;
		oid_ptr->name[1] = 11;
		oid_ptr->name[2] = 4;
		oid_ptr->name[3] = 1;
		oid_ptr->name[4] = 2;
		oid_ptr++;			
		
		//1.9.11.4.1.3 gain control upper bound
		oid_ptr->length = 5;
		oid_ptr->name[0] = 9;
		oid_ptr->name[1] = 11;
		oid_ptr->name[2] = 4;
		oid_ptr->name[3] = 1;
		oid_ptr->name[4] = 3;
		oid_ptr++;		
		
		//1.9.11.4.1.4 gain control range
		oid_ptr->length = 5;
		oid_ptr->name[0] = 9;
		oid_ptr->name[1] = 11;
		oid_ptr->name[2] = 4;
		oid_ptr->name[3] = 1;
		oid_ptr->name[4] = 4;
		oid_ptr++;		

		//1.9.11.4.1.5 manual gain value
		oid_ptr->length = 5;
		oid_ptr->name[0] = 9;
		oid_ptr->name[1] = 11;
		oid_ptr->name[2] = 4;
		oid_ptr->name[3] = 1;
		oid_ptr->name[4] = 5;
		oid_ptr++;		
	
		//print_mib_tbl(&nscrt_mib_oid_tbl);
	}
}

static AsnIdType nscrtGetTemperature(void)
{
	double tmp = 0;
	rtk_transceiver_data_t transceiver;
	rtk_ponmac_transceiver_get(RTK_TRANSCEIVER_PARA_TYPE_TEMPERATURE, &transceiver);
	 
	if (128 >= transceiver.buf[0]) //MSB: pSrcData->buf[0]; LSB: pSrcData->buf[1]
	{
		tmp = (-1)*((~(transceiver.buf[0]))+1)+((double)(transceiver.buf[1])*1/256);
	}else{
		tmp = transceiver.buf[0]+((double)(transceiver.buf[1])*1/256);
	}
	
	return asnUnsl (asnClassApplication, (AsnTagType) 1, tmp);
}

static AsnIdType nscrtGetRxPower(void) // inputlevel
{
	double tmp , tmp2;
	rtk_transceiver_data_t transceiver;
	rtk_ponmac_transceiver_get(RTK_TRANSCEIVER_PARA_TYPE_RX_POWER, &transceiver);
	tmp = __log10(((double)((transceiver.buf[0] << 8) | transceiver.buf[1])*1/10000))*10;
	tmp2 = pow(10,(tmp/10));
	tmp2 /= 0.1;
	return asnUnsl (asnClassApplication, (AsnTagType) 1, tmp2);
}

static AsnIdType nscrtGetOutputLevel(void) // output level
{
	return asnUnsl (asnClassApplication, (AsnTagType) 1, 123);
}

static	AsnIdType	nscrtRetrieveMibValue (unsigned int mibIdx, MixNamePtrType oid_name, MixLengthType oid_namelen)
{
	int i = 0;
	//printf("oid_namelen:%d \n" , oid_namelen);
	//for(i = 0 ; i < oid_namelen; i++)
	//	printf("oid_name[%d]=%d\n" , i , oid_name[i]);

	if(oid_namelen <= 0)
	{
		printf("OID len : %d \n" , oid_namelen);
		return ((AsnIdType) 0);
	}
	else 
	{
		// list table 
		if(oid_name[0] == NSCRTPROPERTY)
		{
		}
		else if(oid_name[0] == NSCRTALARMS)
		{
		}
		else if(oid_name[0] == NSCRTCOMMON)
		{
			if(oid_name[1] == 1 && oid_name[2] == 13)
			{
				return nscrtGetTemperature();
			}
		}
		else if(oid_name[0] == NSCRTDOR)
		{
			if(oid_namelen >= 4 && oid_name[1] == 3 && oid_name[2] == 1 && oid_name[3] == 2) // input power (rx power)
			{
				return nscrtGetRxPower();
			}
			else if(oid_namelen >= 4 && oid_name[1] == 5 && oid_name[2] == 1 && oid_name[3] == 4) // output level
			{
				return nscrtGetOutputLevel();
			}
			else if(oid_namelen >= 4 && oid_name[1] == 8 && oid_name[2] == 1 && oid_name[3] == 2) // voltage
			{
				char str[4] = {'\0'};
				strcpy(str, "5V");
				return asnOctetString (asnClassUniversal, (AsnTagType) 4, (CBytePtrType) (str), (AsnLengthType) strlen ((char *) (str)));
			}
			else if(oid_namelen >= 5 && oid_name[1] == 11 && oid_name[2] == 4 && oid_name[3] == 1 && oid_name[4] == 2) // gain control
			{
				char str[16] = {'\0'};
				strcpy(str, "¹âAGC");
				return asnOctetString (asnClassUniversal, (AsnTagType) 4, (CBytePtrType) (str), (AsnLengthType) strlen ((char *) (str)));				
			}
			else if(oid_namelen >= 5 && oid_name[1] == 11 && oid_name[2] == 4 && oid_name[3] == 1 && oid_name[4] == 3)// gain control upper bound
			{
				char str[16] = {'\0'};
				strcpy(str, "-2dBm");
				return asnOctetString (asnClassUniversal, (AsnTagType) 4, (CBytePtrType) (str), (AsnLengthType) strlen ((char *) (str)));				
			}
			else if(oid_namelen >= 5 && oid_name[1] == 11 && oid_name[2] == 4 && oid_name[3] == 1 && oid_name[4] == 4)// gain control range
			{
				char str[16] = {'\0'};
				strcpy(str, "-2dBm~-12dBm");
				return asnOctetString (asnClassUniversal, (AsnTagType) 4, (CBytePtrType) (str), (AsnLengthType) strlen ((char *) (str)));				
			}
			else if(oid_namelen >= 5 && oid_name[1] == 11 && oid_name[2] == 4 && oid_name[3] == 1 && oid_name[4] == 5)// manual gain control
			{
				char str[4] = {'\0'};
				str[0] = '0';
				return asnOctetString (asnClassUniversal, (AsnTagType) 4, (CBytePtrType) (str), (AsnLengthType) strlen ((char *) (str)));				
			}
		}
		else if(oid_name[0] == NSCRTFN)
		{
		}
		else //not table 
		{
			switch (oid_name[0])
			{
				default:
					return ((AsnIdType) 0);
			}
		}
	}
	

	return ((AsnIdType) 0);
}

static	MixStatusType	nscrtRelease (MixCookieType cookie)
{
	cookie = cookie;
	return (smpErrorGeneric);
}

static	MixStatusType nscrtCreate (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen, AsnIdType asn)
{
	cookie = cookie;
	name = name;
	namelen = namelen;
	asn = asn;
	return (smpErrorGeneric);
}

static	MixStatusType	nscrtDestroy (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen)
{
	printf("ifaceDestroy ()\n");    
	cookie = cookie;
	name = name;
	namelen = namelen;
	return (smpErrorGeneric);
}

static	AsnIdType	nscrtGet (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen)
{
	unsigned int idx;

	nscrt_mib_Init();

	cookie = cookie;
	//printf("nscrtGet \n");
	if(snmp_oid_get(&nscrt_mib_oid_tbl, name, namelen, &idx))
	{
		//printf("nscrtGet: Retrive Value \n");
		return nscrtRetrieveMibValue(idx, name , namelen);
	}
	//printf("nscrtGet: Can not get OID \n");
	
	
	return ((AsnIdType) 0);
}


static	MixStatusType	nscrtSet (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen, AsnIdType asn)
{
	unsigned int idx;

	cookie = cookie;
	name = name;
	namelen = namelen;
	asn = asn;

	return (smpErrorReadOnly);
}

static	AsnIdType	nscrtNext (MixCookieType cookie, MixNamePtrType name, MixLengthPtrType namelenp)
{
	unsigned int idx = 0;

	nscrt_mib_Init();

	cookie = cookie;
	
	// Mason Yu test
	//printf("nscrtNext \n");
	if(snmp_oid_getnext(&nscrt_mib_oid_tbl, name, *namelenp, &idx))
	{
		struct mib_oid * oid_ptr = nscrt_mib_oid_tbl.oid;
		oid_ptr += idx;

		memcpy((unsigned char *)name, oid_ptr->name, oid_ptr->length);
		*namelenp = oid_ptr->length;

		//printf("nscrtNext: Retrive Value \n");
		return nscrtRetrieveMibValue(idx, name , *namelenp);
		//return nscrtRetrieveMibValue(idx);
	}
	
	//printf("nscrtNext: Can not get OID \n");
	
	return ((AsnIdType) 0);
}

static	MixOpsType	nscrtOps = {

			nscrtRelease,
			nscrtCreate,
			nscrtDestroy,
			nscrtNext,
			nscrtGet,
			nscrtSet

			};


CVoidType		nscrtInit (void)
{
	(void) misExport ((MixNamePtrType) "\53\6\1\4\1\201\210\1\1",
		(MixLengthType) 9, & nscrtOps, (MixCookieType) 0);
		
}



