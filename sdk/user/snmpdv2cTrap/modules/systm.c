
#include	<sys/param.h>
#include	<sys/types.h>
#include	<sys/time.h>
#include	<stdio.h>
#include	<netdb.h>
#include	<unistd.h>
#include 	<sys/file.h>

#include	"ctypes.h"
#include	"local.h"
#include	"debug.h"
#include	"miv.h"
#include	"asn.h"
#include	"mix.h"
#include	"systm.h"
#include	"oid.h"
// Added by Mason Yu for SystemName
#include 	<rtk/sysconfig.h>
#include 	<sys/sysinfo.h>

#define			systmStringSize		(2 * MAXHOSTNAMELEN)

#ifdef NeTtel
#define PRIVATENUM	6492
#else
#define PRIVATENUM	14358 /* SnapGear Pty Ltd */
#endif

static	CCharType	systmDescrText[systmStringSize];		//*****Change to HOSTANME + version
static	CCharType	systmHostName[(MAXHOSTNAMELEN + 1)];
static	MivStrType	systmDescrStr;
static	CCharType	systmContText[systmStringSize];
static	MivStrType	systmContStr;
static	CCharType	systmNameText[systmStringSize];
static	MivStrType	systmNameStr;
static	CCharType	systmLocText[systmStringSize];
static	MivStrType	systmLocStr;

static	CIntsType	systmServInt;

static	CByteType	systmObjId [ systmStringSize ];
static	MivStrType	systmIdStr;

static	struct	timeval	systmBootTime;
// Added by Mason Yu for SystemName
static  char		name_str[256];

static	MixStatusType	systmTimeRelease (mix)

MixCookieType		mix;

{
	mix = mix;
	return (smpErrorGeneric);
}


static	MixStatusType	systmTimeNoSet (mix, name, namelen, value)

MixCookieType		mix;
MixNamePtrType		name;
MixLengthType		namelen;
AsnIdType		value;

{
	mix = mix;
	name = name;
	namelen = namelen;
	value = value;
	return (smpErrorReadOnly);
}

static	MixStatusType	systmTimeCreate (mix, name, namelen, value)

MixCookieType		mix;
MixNamePtrType		name;
MixLengthType		namelen;
AsnIdType		value;

{
	mix = mix;
	name = name;
	namelen = namelen;
	value = value;
	return (smpErrorGeneric);
}

static	MixStatusType	systmTimeDestroy (mix, name, namelen)

MixCookieType		mix;
MixNamePtrType		name;
MixLengthType		namelen;

{
	mix = mix;
	name = name;
	namelen = namelen;
	return (smpErrorGeneric);
}

static	AsnIdType	systmTimeGet (mix, name, namelen)

MixCookieType		mix;
MixNamePtrType		name;
MixLengthType		namelen;

{
	struct	timeval		now;
	CUnslType		uptime;
	struct sysinfo info;

	mix = mix;
	if ((namelen != (MixLengthType) 1) ||
		(*name != (MixNameType) 0)) {
		return ((AsnIdType) 0);
	}
	else {		
		// Modified by Mason Yu. If the system time is updated by NTP server, the system uptime geted by gettimeofday() is not correct.
#if 0
		(void) gettimeofday (& now, (struct timezone *) 0);		
		uptime = (CUnslType) (now.tv_sec - systmBootTime.tv_sec) *
			(CUnslType) 100;
#else
		sysinfo(&info);
		uptime = (CUnslType)(info.uptime * 100 );
#endif
		return (asnUnsl (asnClassApplication, (AsnTagType) 3, uptime));
	}
}

static	AsnIdType	systmTimeNext (mix, name, namelenp)

MixCookieType		mix;
MixNamePtrType		name;
MixLengthPtrType	namelenp;

{
	struct	timeval		now;
	CUnslType		uptime;
	struct sysinfo info;

	mix = mix;
	if (*namelenp != (MixLengthType) 0) {
		return ((AsnIdType) 0);
	}
	else {
		*namelenp = (MixLengthType) 1;
		*name = (MixNameType) 0;
		
		// Modified by Mason Yu. If the system time is updated by NTP server, the system uptime geted by gettimeofday()is not correct.
#if 0
		DEBUG2 ("systmTimeNext: boot %d %d\n",
			systmBootTime.tv_sec,
			systmBootTime.tv_usec);
		(void) gettimeofday (& now, (struct timezone *) 0);
		DEBUG2 ("systmTimeNext: now %d %d\n",
			now.tv_sec,
			now.tv_usec);
		uptime = (CUnslType) (now.tv_sec - systmBootTime.tv_sec) *
			(CUnslType) 100;
		DEBUG1 ("systmTimeNext: uptime %d\n", uptime);
#else
		sysinfo(&info);
		uptime = (CUnslType)(info.uptime * 100 );
#endif			
		return (asnUnsl (asnClassApplication, (AsnTagType) 3, uptime));
	}
}

static	MixOpsType	systmTimeOps	= {

			systmTimeRelease,
			systmTimeCreate,
			systmTimeDestroy,
			systmTimeNext,
			systmTimeGet,
			systmTimeNoSet

		};
		
// Added by Mason Yu for System Name. Start
static	MixStatusType	systmNameRelease (mix)

MixCookieType		mix;

{
	mix = mix;
	return (smpErrorGeneric);
}


static	MixStatusType	systmNameSet (mix, name, namelen, value)

MixCookieType		mix;
MixNamePtrType		name;
MixLengthType		namelen;
AsnIdType		value;

{
	AsnLengthType		k;
	unsigned char strvalue[256];
	
	mix = mix;
	name = name;
	namelen = namelen;
	value = value;	
	
	k = asnLength (value);		
	(void) asnContents (value, strvalue, k);	
	strvalue[k]=NULL;	
	
	if ( !mib_set(MIB_SNMP_SYS_NAME, (void *)strvalue)) {			
		return (smpErrorGeneric);
	}	
	
	return (smpErrorNone);
}

static	MixStatusType	systmNameCreate (mix, name, namelen, value)

MixCookieType		mix;
MixNamePtrType		name;
MixLengthType		namelen;
AsnIdType		value;

{
	mix = mix;
	name = name;
	namelen = namelen;
	value = value;
	return (smpErrorGeneric);
}

static	MixStatusType	systmNameDestroy (mix, name, namelen)

MixCookieType		mix;
MixNamePtrType		name;
MixLengthType		namelen;

{
	mix = mix;
	name = name;
	namelen = namelen;
	return (smpErrorGeneric);
}

static	AsnIdType	systmNameGet (mix, name, namelen)

MixCookieType		mix;
MixNamePtrType		name;
MixLengthType		namelen;

{
	struct	timeval		now;
	CUnslType		uptime;	
	
	mix = mix;
	if ((namelen != (MixLengthType) 1) ||
		(*name != (MixNameType) 0)) {
		return ((AsnIdType) 0);
	}
	else {			
		if ( !mib_get(MIB_SNMP_SYS_NAME, (void *)name_str)) {			
			return (smpErrorGeneric);
		}
		
		return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
				(CBytePtrType) (name_str), 
				(AsnLengthType) strlen ((char *) (name_str)));
	}
}

static	AsnIdType	systmNameNext (mix, name, namelenp)

MixCookieType		mix;
MixNamePtrType		name;
MixLengthPtrType	namelenp;

{
	struct	timeval		now;
	CUnslType		uptime;

	mix = mix;
	if (*namelenp != (MixLengthType) 0) {
		return ((AsnIdType) 0);
	}
	else {		
		*namelenp = (MixLengthType) 1;
		*name = (MixNameType) 0;
		
		if ( !mib_get(MIB_SNMP_SYS_NAME, (void *)name_str)) {			
			return (smpErrorGeneric);
		}
		
		return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
				(CBytePtrType) (name_str), 
				(AsnLengthType) strlen ((char *) (name_str)));
	}
}

static	MixOpsType	systmNameOps	= {

			systmNameRelease,
			systmNameCreate,
			systmNameDestroy,
			systmNameNext,
			systmNameGet,
			systmNameSet

			};
// Added by Mason Yu for SystemName. End

static	MixStatusType	systmDescrRelease (mix)

MixCookieType		mix;

{
	mix = mix;
	return (smpErrorGeneric);
}


static	MixStatusType	systmDescrSet (mix, name, namelen, value)

MixCookieType		mix;
MixNamePtrType		name;
MixLengthType		namelen;
AsnIdType		value;

{
	AsnLengthType		k;
	unsigned char strvalue[256];
	
	mix = mix;
	name = name;
	namelen = namelen;
	value = value;	

#if 0	
	k = asnLength (value);		
	(void) asnContents (value, strvalue, k);	
	strvalue[k]=NULL;	
	
	if ( !mib_set(MIB_SNMP_SYS_DESCR, (void *)strvalue)) {			
		return (smpErrorGeneric);
	}	
	
	return (smpErrorNone);
#else	
	return (smpErrorReadOnly);
#endif
}

static	MixStatusType	systmDescrCreate (mix, name, namelen, value)

MixCookieType		mix;
MixNamePtrType		name;
MixLengthType		namelen;
AsnIdType		value;

{
	mix = mix;
	name = name;
	namelen = namelen;
	value = value;
	return (smpErrorGeneric);
}

static	MixStatusType	systmDescrDestroy (mix, name, namelen)

MixCookieType		mix;
MixNamePtrType		name;
MixLengthType		namelen;

{
	mix = mix;
	name = name;
	namelen = namelen;
	return (smpErrorGeneric);
}

static	AsnIdType	systmDescrGet (mix, name, namelen)

MixCookieType		mix;
MixNamePtrType		name;
MixLengthType		namelen;

{
	struct	timeval		now;
	CUnslType		uptime;	
	
	mix = mix;
	if ((namelen != (MixLengthType) 1) ||
		(*name != (MixNameType) 0)) {
		return ((AsnIdType) 0);
	}
	else {			
		if ( !mib_get(MIB_SNMP_SYS_DESCR, (void *)name_str)) {			
			return (smpErrorGeneric);
		}
		
		return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
				(CBytePtrType) (name_str), 
				(AsnLengthType) strlen ((char *) (name_str)));
	}
}

static	AsnIdType	systmDescrNext (mix, name, namelenp)

MixCookieType		mix;
MixNamePtrType		name;
MixLengthPtrType	namelenp;

{
	struct	timeval		now;
	CUnslType		uptime;

	mix = mix;
	if (*namelenp != (MixLengthType) 0) {
		return ((AsnIdType) 0);
	}
	else {		
		*namelenp = (MixLengthType) 1;
		*name = (MixNameType) 0;
		
		if ( !mib_get(MIB_SNMP_SYS_DESCR, (void *)name_str)) {			
			return (smpErrorGeneric);
		}
		
		return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
				(CBytePtrType) (name_str), 
				(AsnLengthType) strlen ((char *) (name_str)));
	}
}

static	MixOpsType	systmDescrOps	= {

			systmDescrRelease,
			systmDescrCreate,
			systmDescrDestroy,
			systmDescrNext,
			systmDescrGet,
			systmDescrSet

			};

static	MixStatusType	systmContRelease (mix)

MixCookieType		mix;

{
	mix = mix;
	return (smpErrorGeneric);
}


static	MixStatusType	systmContSet (mix, name, namelen, value)

MixCookieType		mix;
MixNamePtrType		name;
MixLengthType		namelen;
AsnIdType		value;

{
	AsnLengthType		k;
	unsigned char strvalue[256];
	
	mix = mix;
	name = name;
	namelen = namelen;
	value = value;	
	
	k = asnLength (value);		
	(void) asnContents (value, strvalue, k);	
	strvalue[k]=NULL;	
	
	if ( !mib_set(MIB_SNMP_SYS_CONTACT, (void *)strvalue)) {			
		return (smpErrorGeneric);
	}	
	
	return (smpErrorNone);
}

static	MixStatusType	systmContCreate (mix, name, namelen, value)

MixCookieType		mix;
MixNamePtrType		name;
MixLengthType		namelen;
AsnIdType		value;

{
	mix = mix;
	name = name;
	namelen = namelen;
	value = value;
	return (smpErrorGeneric);
}

static	MixStatusType	systmContDestroy (mix, name, namelen)

MixCookieType		mix;
MixNamePtrType		name;
MixLengthType		namelen;

{
	mix = mix;
	name = name;
	namelen = namelen;
	return (smpErrorGeneric);
}

static	AsnIdType	systmContGet (mix, name, namelen)

MixCookieType		mix;
MixNamePtrType		name;
MixLengthType		namelen;

{
	struct	timeval		now;
	CUnslType		uptime;	
	
	mix = mix;
	if ((namelen != (MixLengthType) 1) ||
		(*name != (MixNameType) 0)) {
		return ((AsnIdType) 0);
	}
	else {			
		if ( !mib_get(MIB_SNMP_SYS_CONTACT, (void *)name_str)) {			
			return (smpErrorGeneric);
		}
		
		return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
				(CBytePtrType) (name_str), 
				(AsnLengthType) strlen ((char *) (name_str)));
	}
}

static	AsnIdType	systmContNext (mix, name, namelenp)

MixCookieType		mix;
MixNamePtrType		name;
MixLengthPtrType	namelenp;

{
	struct	timeval		now;
	CUnslType		uptime;

	mix = mix;
	if (*namelenp != (MixLengthType) 0) {
		return ((AsnIdType) 0);
	}
	else {		
		*namelenp = (MixLengthType) 1;
		*name = (MixNameType) 0;
		
		if ( !mib_get(MIB_SNMP_SYS_CONTACT, (void *)name_str)) {			
			return (smpErrorGeneric);
		}
		
		return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
				(CBytePtrType) (name_str), 
				(AsnLengthType) strlen ((char *) (name_str)));
	}
}

static	MixOpsType	systmContOps	= {

			systmContRelease,
			systmContCreate,
			systmContDestroy,
			systmContNext,
			systmContGet,
			systmContSet

			};

static	MixStatusType	systmLocRelease (mix)

MixCookieType		mix;

{
	mix = mix;
	return (smpErrorGeneric);
}


static	MixStatusType	systmLocSet (mix, name, namelen, value)

MixCookieType		mix;
MixNamePtrType		name;
MixLengthType		namelen;
AsnIdType		value;

{
	AsnLengthType		k;
	unsigned char strvalue[256];
	
	mix = mix;
	name = name;
	namelen = namelen;
	value = value;	
	
	k = asnLength (value);		
	(void) asnContents (value, strvalue, k);	
	strvalue[k]=NULL;	
	
	if ( !mib_set(MIB_SNMP_SYS_LOCATION, (void *)strvalue)) {			
		return (smpErrorGeneric);
	}	
	
	return (smpErrorNone);
}

static	MixStatusType	systmLocCreate (mix, name, namelen, value)

MixCookieType		mix;
MixNamePtrType		name;
MixLengthType		namelen;
AsnIdType		value;

{
	mix = mix;
	name = name;
	namelen = namelen;
	value = value;
	return (smpErrorGeneric);
}

static	MixStatusType	systmLocDestroy (mix, name, namelen)

MixCookieType		mix;
MixNamePtrType		name;
MixLengthType		namelen;

{
	mix = mix;
	name = name;
	namelen = namelen;
	return (smpErrorGeneric);
}

static	AsnIdType	systmLocGet (mix, name, namelen)

MixCookieType		mix;
MixNamePtrType		name;
MixLengthType		namelen;

{
	struct	timeval		now;
	CUnslType		uptime;	
	
	mix = mix;
	if ((namelen != (MixLengthType) 1) ||
		(*name != (MixNameType) 0)) {
		return ((AsnIdType) 0);
	}
	else {			
		if ( !mib_get(MIB_SNMP_SYS_LOCATION, (void *)name_str)) {			
			return (smpErrorGeneric);
		}
		
		return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
				(CBytePtrType) (name_str), 
				(AsnLengthType) strlen ((char *) (name_str)));
	}
}

static	AsnIdType	systmLocNext (mix, name, namelenp)

MixCookieType		mix;
MixNamePtrType		name;
MixLengthPtrType	namelenp;

{
	struct	timeval		now;
	CUnslType		uptime;

	mix = mix;
	if (*namelenp != (MixLengthType) 0) {
		return ((AsnIdType) 0);
	}
	else {		
		*namelenp = (MixLengthType) 1;
		*name = (MixNameType) 0;
		
		if ( !mib_get(MIB_SNMP_SYS_LOCATION, (void *)name_str)) {			
			return (smpErrorGeneric);
		}
		
		return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
				(CBytePtrType) (name_str), 
				(AsnLengthType) strlen ((char *) (name_str)));
	}
}

static	MixOpsType	systmLocOps	= {

			systmLocRelease,
			systmLocCreate,
			systmLocDestroy,
			systmLocNext,
			systmLocGet,
			systmLocSet

			};

static	MixStatusType	systmObjIDRelease (mix)

MixCookieType		mix;

{
	mix = mix;
	return (smpErrorGeneric);
}


static	MixStatusType	systmObjIDSet (mix, name, namelen, value)

MixCookieType		mix;
MixNamePtrType		name;
MixLengthType		namelen;
AsnIdType		value;

{
	AsnLengthType		k;
	unsigned char strvalue[256];
	
	mix = mix;
	name = name;
	namelen = namelen;
	value = value;	

#if 0	
	k = asnLength (value);		
	(void) asnContents (value, strvalue, k);	
	strvalue[k]=NULL;	
	
	if ( !mib_set(MIB_SNMP_SYS_OID, (void *)strvalue)) {			
		return (smpErrorGeneric);
	}	
	
	return (smpErrorNone);
#else
	return (smpErrorReadOnly);
#endif
}

static	MixStatusType	systmObjIDCreate (mix, name, namelen, value)

MixCookieType		mix;
MixNamePtrType		name;
MixLengthType		namelen;
AsnIdType		value;

{
	mix = mix;
	name = name;
	namelen = namelen;
	value = value;
	return (smpErrorGeneric);
}

static	MixStatusType	systmObjIDDestroy (mix, name, namelen)

MixCookieType		mix;
MixNamePtrType		name;
MixLengthType		namelen;

{
	mix = mix;
	name = name;
	namelen = namelen;
	return (smpErrorGeneric);
}

static	AsnIdType	systmObjIDGet (mix, name, namelen)

MixCookieType		mix;
MixNamePtrType		name;
MixLengthType		namelen;

{
	struct	timeval		now;
	CUnslType		uptime;	
	
	mix = mix;
	if ((namelen != (MixLengthType) 1) ||
		(*name != (MixNameType) 0)) {
		return ((AsnIdType) 0);
	}
	else {			
		if ( !mib_get(MIB_SNMP_SYS_OID, (void *)name_str)) {			
			return (smpErrorGeneric);
		}
		
		return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
				(CBytePtrType) (name_str), 
				(AsnLengthType) strlen ((char *) (name_str)));
	}
}

static	AsnIdType	systmObjIDNext (mix, name, namelenp)

MixCookieType		mix;
MixNamePtrType		name;
MixLengthPtrType	namelenp;

{
	struct	timeval		now;
	CUnslType		uptime;

	mix = mix;
	if (*namelenp != (MixLengthType) 0) {
		return ((AsnIdType) 0);
	}
	else {		
		*namelenp = (MixLengthType) 1;
		*name = (MixNameType) 0;
		
		if ( !mib_get(MIB_SNMP_SYS_OID, (void *)name_str)) {			
			return (smpErrorGeneric);
		}
		
		return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
				(CBytePtrType) (name_str), 
				(AsnLengthType) strlen ((char *) (name_str)));
	}
}

static	MixOpsType	systmObjIDOps	= {

			systmObjIDRelease,
			systmObjIDCreate,
			systmObjIDDestroy,
			systmObjIDNext,
			systmObjIDGet,
			systmObjIDSet

			};


CVoidType		systmInit ()

{
	long		hostid;
	unsigned char *ep;
	static int nr = 0;
	FILE *in;
 	char tmp [256];
	CIntfType	k;
	// Added by Mason Yu
	//unsigned char buffer1[100], buffer2[100], buffer3[100], buffer4[100], buffer5[100];
	FILE *fp;
	
 	if ((in = fopen ("/etc/version", "r")) == 0) {
   		in = fopen ("/proc/version", "r");
 	}
 	
 	if (in)
    	{
		if (fgets (tmp, 256, in) > 0)
	  	{ 
	      		tmp [strlen (tmp) - 1] = 0;
	      		strcpy (systmDescrText, tmp);
	  	}
		fclose (in);
    	}
  	else
    	{
		strcpy (systmDescrText, "Unknown");
		printf ("snmpd: cannot open /etc/version or /proc/version.\n");
    	}
    
    	// Mason Yu
#if 0   	
    	fp=fopen("/tmp/snmp", "r");
    	if ( fp ) {
    		fgets(buffer1, 100, fp); 
    		fgets(buffer2, 100, fp);    
    		fgets(buffer3, 100, fp); 
    		fgets(buffer4, 100, fp); 
    		fgets(buffer5, 100, fp);
    		fclose(fp);
    	}     		
#endif

	// Description
#if 0   	    	
	strcpy (systmDescrText, buffer1);
	
	systmDescrStr.mivStrData = (CBytePtrType) systmDescrText;
	systmDescrStr.mivStrMaxLen = (CUnsfType) systmStringSize;
	systmDescrStr.mivStrLen = (CUnsfType) strlen (systmDescrText);

	(void) mivStringRO ((MixNamePtrType) "\53\6\1\2\1\1\1",
		(MixLengthType) 7, & systmDescrStr);
#else
	(void) misExport ((MixNamePtrType) "\53\6\1\2\1\1\1",
		(MixLengthType) 7, & systmDescrOps, (MixCookieType) 0);
#endif
	/* REVISIT: Do we have a different MIB for each device type?
                    If so, this is probably not the way to find out */
        hostid = 1;
#ifdef NEVER
	ep = (unsigned char *) (0xf0006000);
	if ((ep[0] == 0xff) && (ep[1] == 0xff) && (ep[2] == 0xff) &&
	    (ep[3] == 0xff) && (ep[4] == 0xff) && (ep[5] == 0xff))
		hostid = 0x00;
	else
		//hostid = (ep[2]<<24)+(ep[3]<<16)+(ep[4]<<8)+ep[5];
		hostid = (ep[4]<<8)+ep[5];
	DEBUG1 ("systmInit: Hostid %d\n", hostid);
#endif

	// Modified by Mason Yu
	/** Object ID **/
#if 0
    	strncpy(tmp, buffer5, (strlen(buffer5)-1) ); 	
	tmp[strlen(buffer5)-1] = '\0';
    	
	//(void) sprintf (tmp, "1.3.6.1.4.1.%u.%lu", PRIVATENUM, hostid);	
	
	k = oidEncode (systmObjId, (CIntfType) systmStringSize, tmp);	
	
	if (k > (CIntfType) 0) {
		systmIdStr.mivStrData = systmObjId;
		systmIdStr.mivStrMaxLen = (CUnsfType) systmStringSize;
		systmIdStr.mivStrLen = (CUnsfType) k;

		(void) mivObjectIdRO ((MixNamePtrType) "\53\6\1\2\1\1\2",
			(MixLengthType) 7, & systmIdStr);
	}
#else
	(void) misExport ((MixNamePtrType) "\53\6\1\2\1\1\2",
		(MixLengthType) 7, & systmObjIDOps, (MixCookieType) 0);
#endif
	/** End Object ID **/
	
	/** SysupTime **/	
	(void) gettimeofday (&systmBootTime, (struct timezone *) 0);	
	(void) misExport ((MixNamePtrType) "\53\6\1\2\1\1\3",
		(MixLengthType) 7, & systmTimeOps, (MixCookieType) 0);
	/** End SysupmTime **/
	
	/**Contact**/
#if 0		
	strcpy(systmContText, buffer2);	
	
	systmContStr.mivStrData = (CBytePtrType) systmContText;
	systmContStr.mivStrMaxLen = (CUnsfType) systmStringSize;
	systmContStr.mivStrLen = (CUnsfType) strlen (systmContText);
	(void) mivStringRW ((MixNamePtrType) "\53\6\1\2\1\1\4",
		(MixLengthType) 7, & systmContStr);
#else
	(void) misExport ((MixNamePtrType) "\53\6\1\2\1\1\4",
		(MixLengthType) 7, & systmContOps, (MixCookieType) 0);
#endif
	/**End Contact**/

	/**Name**/
	// Modified by Mason Yu	    	    
        // Modified by Mason Yu for SystemName 
#if 0	    		
	strcpy (systmNameText, buffer3);
#if 0	
	if (gethostname((char *)systmHostName,(int)MAXHOSTNAMELEN) < 0) 
		{
		(void)sprintf(systmDescrText,"Unknown Hostname");
		}
	else 
		{
		systmHostName[MAXHOSTNAMELEN]=(CCharType)0;
		(void)sprintf(systmNameText, "%s",systmHostName);
		}	
#endif	
	systmNameStr.mivStrData = (CBytePtrType) systmNameText;
	systmNameStr.mivStrMaxLen = (CUnsfType) systmStringSize;
	systmNameStr.mivStrLen = (CUnsfType) strlen (systmNameText);
	(void) mivStringRW ((MixNamePtrType) "\53\6\1\2\1\1\5",
		(MixLengthType) 7, & systmNameStr);
#else		
	(void) misExport ((MixNamePtrType) "\53\6\1\2\1\1\5",
		(MixLengthType) 7, & systmNameOps, (MixCookieType) 0);	
	/**End Name**/
#endif

	/**Location**/
#if 0
	// Modified by Mason Yu
	//strcpy(systmLocText, "Unknown");	   	
    	//printf("***** buffer4= %s\n", buffer4);    	
	strcpy (systmLocText, buffer4);	
	
	systmLocStr.mivStrData = (CBytePtrType) systmLocText;
	systmLocStr.mivStrMaxLen = (CUnsfType) systmStringSize;
	systmLocStr.mivStrLen = (CUnsfType) strlen (systmLocText);
	(void) mivStringRW ((MixNamePtrType) "\53\6\1\2\1\1\6",
		(MixLengthType) 7, & systmLocStr);
#else
	(void) misExport ((MixNamePtrType) "\53\6\1\2\1\1\6",
		(MixLengthType) 7, & systmLocOps, (MixCookieType) 0);
#endif
	/**End Location**/

	/**Services**/
	systmServInt=(CIntsType)6;
	(void) mivIntlRO ((MixNamePtrType) "\53\6\1\2\1\1\7",
		(MixLengthType) 7, (CIntlPtrType)&systmServInt);
	/**End Services**/	

}
