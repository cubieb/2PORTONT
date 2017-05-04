
#include	<stdio.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<time.h>

#include	"ctypes.h"
#include	"error.h"
#include	"local.h"
#include	"adsl2_vars.h"
#include	"mix.h"
#include	"mis.h"
#include	"asn.h"

#include "mib_tool.h"

struct adsl2_mib {
	unsigned int adslAturPerfStatSesL;	    // adslMIB.1.19.1.1
	unsigned int adslAturPerfStatUasL;     // 19.1.2
	unsigned int adslAturPerfCurr15MinSesL;// 19.1.3
	unsigned int adslAturPerfCurr1DaySesL; // 19.1.5
	
	int adslConfProfileLineType;		// adslMIB.1.20.1.1
	
    struct adsl2_mib   *if_next;
 };

extern unsigned int Adsl_Interface_Idx(void);	// retrieve adsl interface id

static struct adsl2_mib *adsl2_mib_list = NULL;
static struct mib_oid_tbl adsl2_mib_oid_tbl;
static unsigned int adsl2_list_num;

static long adsl2_last_scan = 0;

void Adsl2_Mib_Get(unsigned int if_idx)
{
	struct adsl2_mib *nnew;		
	/*
	 * free old list: 
	 */
	while (adsl2_mib_list) {
	    struct adsl2_mib   *old = adsl2_mib_list;
	    adsl2_mib_list = adsl2_mib_list->if_next;
	    free(old);
	}
	
	nnew = (struct adsl2_mib *) calloc(1, sizeof(struct adsl2_mib));
	if (nnew == NULL)
	{
		adsl2_mib_list = NULL;
		return;              /* alloc error */
	}

	adsl2_mib_list = nnew;
	nnew->if_next = NULL;

// Mason Yu.  20130207
//#ifdef __uClinux__
#if	1		
	//adslAturPerfDataExtTable 94.3.1.19, adslAturPerfStatSesL(1)
	{
		Modem_DSLConfigStatus vStat;

		if(adsl_drv_get(RLCM_GET_DSL_STAT_TOTAL, (void *)&vStat, TR069_STAT_SIZE)) {			
			nnew->adslAturPerfStatSesL = vStat.SES;						
		} else {			
			goto Adsl2_Mib_Get_Fail;
		}
	}

	//adslAturPerfDataExtTable 94.3.1.19, adslAturPerfStatUasL(2)
	{
		Modem_MgmtCounter counter; 

		if(adsl_drv_get(RLCM_GET_DS_ERROR_COUNT, (void *)&counter, sizeof(counter))) {			
			nnew->adslAturPerfStatUasL = counter.uas;						
		} else {			
			goto Adsl2_Mib_Get_Fail;
		}
	}
	
	//adslAturPerfDataExtTable 94.3.1.19, adslAturPerfCurr15MinSesL(3)
	{
		Modem_DSLConfigStatus vStat;

		if(adsl_drv_get(RLCM_GET_DSL_STAT_15MIN, (void *)&vStat, TR069_STAT_SIZE)) {			
			nnew->adslAturPerfCurr15MinSesL = vStat.SES;					
		} else {			
			goto Adsl2_Mib_Get_Fail;
		}
	}
	
	//adslAturPerfDataExtTable 94.3.1.19, adslAturPerfCurr1DaySesL(5)
	{
		Modem_DSLConfigStatus vStat;

		if(adsl_drv_get(RLCM_GET_DSL_STAT_1DAY, (void *)&vStat, TR069_STAT_SIZE)) {			
			nnew->adslAturPerfCurr1DaySesL = vStat.SES;						
		} else {			
			goto Adsl2_Mib_Get_Fail;
		}
	}
	
	//adslConfProfileExtTable 94.3.1.20, adslConfProfileLineType(1)
	{
		int intVal;

		if(adsl_drv_get(RLCM_GET_CHANNEL_MODE, (void *)&intVal, sizeof(int))) {			
			nnew->adslConfProfileLineType = intVal+1;						
		} else {			
			goto Adsl2_Mib_Get_Fail;
		}
	}
	return;
	
Adsl2_Mib_Get_Fail:
	// handle ioctl error
	while (adsl2_mib_list) {
	    struct adsl2_mib   *old = adsl2_mib_list;
	    adsl2_mib_list = adsl2_mib_list->if_next;
	    free(old);
	}

	adsl2_mib_list = NULL;

#else
	nnew->adslLineCoding = 2;	
#endif	
}

void Adsl2_Mib_Init(void)
{
	struct timeval now;
	gettimeofday(&now, 0);

	if((now.tv_sec - adsl2_last_scan) > 10)	//  10 sec
	{
		unsigned int if_idx;

		adsl2_last_scan = now.tv_sec;

		if_idx = Adsl_Interface_Idx();		
		if(if_idx != 0)
		{
			unsigned int oid_tbl_size;
			struct mib_oid *oid_ptr;
			
			Adsl2_Mib_Get(if_idx);

			// only one adsl interface is support now
			adsl2_list_num = 1;

			// create oid table
			free_mib_tbl(&adsl2_mib_oid_tbl);
			
			oid_tbl_size = adsl2_list_num * (ADSLATURPERFDATAEXTTABLENUM+ADSLCONFPROFILEEXTTABLENUM);
			create_mib_tbl(&adsl2_mib_oid_tbl,  oid_tbl_size, 14);	// mib-2.10.94.3.1.15.1

			oid_ptr = adsl2_mib_oid_tbl.oid;
			
			// adslAturPerfDataExtTable 19.1
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLATURPERFDATAEXTTABLE;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATURPERFSTATSESL;   // 1
			oid_ptr->name[3] = if_idx;
			oid_ptr++;			
			
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLATURPERFDATAEXTTABLE;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATURPERFSTATUASL;  // 2
			oid_ptr->name[3] = if_idx;
			oid_ptr++;
			
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLATURPERFDATAEXTTABLE;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATURPERFCURR15MINSESL;  // 3
			oid_ptr->name[3] = if_idx;
			oid_ptr++;
			
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLATURPERFDATAEXTTABLE;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATURPERFCURR1DAYSESL;  // 5
			oid_ptr->name[3] = if_idx;
			oid_ptr++;
			
			// adslConfProfileExtTable 20.1
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLCONFPROFILEEXTTABLE;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLCONFPROFILELINETYPE;  // 1
			oid_ptr->name[3] = if_idx;			
		}	
	}
}

static	AsnIdType	adsl2RetrieveMibValue (unsigned int mibIdx)
{
	struct mib_oid * oid_ptr;

	if(adsl2_mib_list == NULL)
	{
		return ((AsnIdType) 0);
	}

	oid_ptr = adsl2_mib_oid_tbl.oid;

	oid_ptr += mibIdx;	

	if(oid_ptr->name[0] == ADSLATURPERFDATAEXTTABLE)
	{
		if (oid_ptr->name[2] == ADSLATURPERFSTATSESL)
		{
			return asnUnsl (asnClassApplication, (AsnTagType) 2, adsl2_mib_list->adslAturPerfStatSesL);	
		}
		if (oid_ptr->name[2] == ADSLATURPERFSTATUASL)
		{
			return asnUnsl (asnClassApplication, (AsnTagType) 2, adsl2_mib_list->adslAturPerfStatUasL);	
		}
		if (oid_ptr->name[2] == ADSLATURPERFCURR15MINSESL)
		{
			return asnUnsl (asnClassApplication, (AsnTagType) 2, adsl2_mib_list->adslAturPerfCurr15MinSesL);	
		}
		if (oid_ptr->name[2] == ADSLATURPERFCURR1DAYSESL)
		{
			return asnUnsl (asnClassApplication, (AsnTagType) 2, adsl2_mib_list->adslAturPerfCurr1DaySesL);	
		}
	}
	else if(oid_ptr->name[0] == ADSLCONFPROFILEEXTTABLE)
	{
		if (oid_ptr->name[2] == ADSLCONFPROFILELINETYPE)
		{
			return asnIntl (asnClassApplication, (AsnTagType) 2, adsl2_mib_list->adslConfProfileLineType);	
		}
	}
	
	return ((AsnIdType) 0);
}

static	MixStatusType	adsl2Release (MixCookieType cookie)
{
	cookie = cookie;
	return (smpErrorGeneric);
}

static	MixStatusType adsl2Create (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen, AsnIdType asn)
{
	//printf("ifaceCreate ()\n");    
	cookie = cookie;
	name = name;
	namelen = namelen;
	asn = asn;
	return (smpErrorGeneric);
}

static	MixStatusType	adsl2Destroy (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen)
{
	//printf("ifaceDestroy ()\n");    
	cookie = cookie;
	name = name;
	namelen = namelen;
	return (smpErrorGeneric);
}

static	AsnIdType	adsl2Get (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen)
{
	unsigned int idx;

	Adsl2_Mib_Init();

	cookie = cookie;
	if(snmp_oid_get(&adsl2_mib_oid_tbl, name, namelen, &idx))
	{
		return adsl2RetrieveMibValue(idx);
	}

	return ((AsnIdType) 0);
}

static	MixStatusType	adsl2Set (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen, AsnIdType asn)
{
	unsigned int idx;

	cookie = cookie;
	name = name;
	namelen = namelen;
	asn = asn;

	return (smpErrorNone);

}

static	AsnIdType	adsl2Next (MixCookieType cookie, MixNamePtrType name, MixLengthPtrType namelenp)
{
	unsigned int idx = 0;

	Adsl2_Mib_Init();
	
#if 0
	printf("adslNext %d\n", *namelenp);

	
	while(idx < *namelenp)
	{
		printf("adslNext %d %d\n",idx, *name++);		
		idx++;
	}
#endif

	cookie = cookie;
	if(snmp_oid_getnext(&adsl2_mib_oid_tbl, name, *namelenp, &idx))
	{
		struct mib_oid * oid_ptr = adsl2_mib_oid_tbl.oid;
		oid_ptr += idx;

		memcpy((unsigned char *)name, oid_ptr->name, oid_ptr->length);
		*namelenp = oid_ptr->length;
	
		return adsl2RetrieveMibValue(idx);
	}

	return ((AsnIdType) 0);
}

static	MixOpsType	adsl2Ops = {

			adsl2Release,
			adsl2Create,
			adsl2Destroy,
			adsl2Next,
			adsl2Get,
			adsl2Set

			};


CVoidType		adsl2Init (void)
{
	(void) misExport ((MixNamePtrType) "\53\6\1\2\1\12\136\3\1",
		(MixLengthType) 9, & adsl2Ops, (MixCookieType) 0);
}



