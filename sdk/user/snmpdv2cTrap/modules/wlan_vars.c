
#include	<stdio.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<time.h>

#include	"ctypes.h"
#include	"error.h"
#include	"local.h"
#include	"wlan_vars.h"
#include	"mix.h"
#include	"mis.h"
#include	"asn.h"

#include 	"mib_tool.h"


//const char WLANIF[] = "wlan0";
const char IWPRIV[] = "/bin/iwpriv";
const char BAND[] = "net_work_type:";
const char SSID[] = "dot11DesiredSSID:";
const char CHANNEL[] = "dot11channel:";
const char AUTHTYPE[] = "dot11AuthAlgrthm:";
const char RTSTHRES[] = "dot11RTSThreshold:";
const char FRAGTHRES[] = "dot11FragmentationThreshold:";
const char BEACONINTERVAL[] = "dot11BeaconPeriod:";
const char DATARATE[] = "autoRate:";
const char PREAMBLE[] = "shortpreamble:";
const char HIDDENAP[] = "hiddenAP:";
const char ENCRYPTION[] = "dot11PrivacyAlgrthm:";
const char WLANKEY1[] = "keytype[0].skey:";
const char WLANKEY2[] = "keytype[1].skey:";
const char WLANKEY3[] = "keytype[2].skey:";
const char WLANKEY4[] = "keytype[3].skey:";
const char WLANKEYINDEX[] = "dot11PrivacyKeyIndex:";
const char WLANPASSPHRASE[] = "dot11PassPhrase:";
const char WLANTXPOWERCCK[] = "pwrlevelCCK:";
const char WLANTXPOWEXOFDM[] = "pwrlevelOFDM:";

static struct mib_oid_tbl wlan_mib_oid_tbl;
static long wlan_last_scan = 0;
static int TXPOWER = 2;  // 2: 60mW  1: 30mW  2: 15mW
static unsigned char CCKValue[15];
static unsigned char OFDMValue[15];

void Wlan_Mib_Init(void)
{
	struct timeval now;
	int i;
	
	gettimeofday(&now, 0);

//	printf("Now:%u:%u\n",now.tv_sec, now.tv_usec);

	if((now.tv_sec - wlan_last_scan) > 10)	//  10 sec
	{		
		
		unsigned int oid_tbl_size;
		struct mib_oid * oid_ptr;

		wlan_last_scan = now.tv_sec;			

		// create oid table
		free_mib_tbl(&wlan_mib_oid_tbl);
		
		// Wlan MIB has 18 object
		oid_tbl_size = 18;
		create_mib_tbl(&wlan_mib_oid_tbl,  oid_tbl_size, oid_tbl_size); 
		oid_ptr = wlan_mib_oid_tbl.oid;
		
		for (i=1; i<=oid_tbl_size; i++) {
			oid_ptr->length = 1;
			oid_ptr->name[0] = i;						
			oid_ptr++;	
		}	
		
	}
}

static unsigned long wlanRetrieveMibValueFromFile(const char * compareStr, int offset)
{
	FILE *fp;
	char temps[0x100];
	unsigned long value;
	char *str, *endptr;
	
	if ((fp = fopen("/proc/wlan0/mib_all", "r")) == NULL)
	{
		printf("Open file /proc/wlan0/mib_all fail !\n");
		return -1;
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


static char * wlanRetrieveMibStringFromFile(const char * compareStr, int offset)
{
	FILE *fp;
	char temps[0x100];
	static unsigned char string[100];
	char *str, *endptr;
	
	if ((fp = fopen("/proc/wlan0/mib_all", "r")) == NULL)
	{
		printf("Open file /proc/wlan0/mib_all fail !\n");
		return -1;
	}	
	
	while (fgets(temps,0x100,fp))
	{
		if (temps[strlen(temps)-1]=='\n')
			temps[strlen(temps)-1] = 0;
		
		if (str=strstr(temps, compareStr))
		{			
			//printf("The string is %s\n", str+offset);			
			strcpy(string, str+offset);						
							
			
		} else {					
			continue;
			
	        }
		
	}
	
	fclose(fp);
	return string;	
	
}


static	AsnIdType	wlanRetrieveMibValue (unsigned int mibIdx)
{
	struct mib_oid * oid_ptr;
	unsigned long value;
	char *string;		
	
	// Mason Yu 123
	//unsigned char value2[32];
	//mib_get(20, (void *)value2);
	
	oid_ptr = wlan_mib_oid_tbl.oid;
	oid_ptr += mibIdx;			
	
	switch (oid_ptr->name[0])
	{
	case WLANBAND:	
		string = wlanRetrieveMibStringFromFile(BAND, strlen(BAND)+1);
		if (strcmp(string,"BG") == 0 )
			return asnUnsl (asnClassUniversal, (AsnTagType) 2, 3);
		else if (strcmp(string,"G") == 0 )
			return asnUnsl (asnClassUniversal, (AsnTagType) 2, 2);	
		else if (strcmp(string,"B") == 0 )
			return asnUnsl (asnClassUniversal, (AsnTagType) 2, 1);	
					
		
	case WLANSSID:	
		string = wlanRetrieveMibStringFromFile(SSID, 25);
		return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
				(CBytePtrType) string, 
				(AsnLengthType) strlen ((char *) string));			
		
	case WLANCHAN:	
		value = wlanRetrieveMibValueFromFile(CHANNEL, strlen(CHANNEL)+1);	
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, value);			
	
	case WLANRADIOPOWER:		
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, TXPOWER);
		break;
		
	case WLANAUTHTYPE:
		value = wlanRetrieveMibValueFromFile(AUTHTYPE, strlen(AUTHTYPE)+1);	
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, value);			
					
	case WLANFRAGTHRES:				
		value = wlanRetrieveMibValueFromFile(FRAGTHRES, strlen(FRAGTHRES)+1);	
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, value);			
		
	case WLANRTSTHRES:	
		value = wlanRetrieveMibValueFromFile(RTSTHRES, strlen(RTSTHRES)+1);
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, value);				
	
	case WLANBCNINT:
		value = wlanRetrieveMibValueFromFile(BEACONINTERVAL, strlen(BEACONINTERVAL)+1);	
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, value);			
	
	case WLANDATARATE:
		value = wlanRetrieveMibValueFromFile(DATARATE, strlen(DATARATE)+1);	
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, value);				
		
	case WLANPREABBLE:	
		value = wlanRetrieveMibValueFromFile(PREAMBLE, strlen(PREAMBLE)+1);
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, value);			
	
	case WLANBROADCASTSSID:	
		value = wlanRetrieveMibValueFromFile(HIDDENAP, strlen(HIDDENAP)+1);	
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, value);						
	
	case WLANENCRYPTION:
		value = wlanRetrieveMibValueFromFile(ENCRYPTION, strlen(ENCRYPTION)+1);	
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, value);
		
	case WLANENCRYKEY1:
		string = wlanRetrieveMibStringFromFile(WLANKEY1, strlen(WLANKEY1)+1);
		return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
				(CBytePtrType) string, 
				(AsnLengthType) strlen ((char *) string));
	
	case WLANENCRYKEY2:
		string = wlanRetrieveMibStringFromFile(WLANKEY2, strlen(WLANKEY2)+1);
		return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
				(CBytePtrType) string, 
				(AsnLengthType) strlen ((char *) string));
				
	case WLANENCRYKEY3:
		string = wlanRetrieveMibStringFromFile(WLANKEY3, strlen(WLANKEY3)+1);
		return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
				(CBytePtrType) string, 
				(AsnLengthType) strlen ((char *) string));
				
	case WLANENCRYKEY4:
		string = wlanRetrieveMibStringFromFile(WLANKEY4, strlen(WLANKEY4)+1);
		return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
				(CBytePtrType) string, 
				(AsnLengthType) strlen ((char *) string));
	
	case WLANDEFAULTTXKEY:
		value = wlanRetrieveMibValueFromFile(WLANKEYINDEX, strlen(WLANKEYINDEX)+1);	
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, value);
	
	case WLANPRESHAREDKEY:
		string = wlanRetrieveMibStringFromFile(WLANPASSPHRASE, strlen(WLANPASSPHRASE)+1);
		return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
				(CBytePtrType) string, 
				(AsnLengthType) strlen ((char *) string));														
	
	default:
		return ((AsnIdType) 0);
				
	}
	
}

void stringtoByte(unsigned char * str, unsigned char * value)
{
	int i;
	
	for (i=0; i<28; i++)
	{
		if ( i%2 == 0 ) {
			if (str[i]>=48 && str[i]<=57) {				
				value[i/2]=(str[i] - 48) * 16;
			}else {				
				value[i/2]=(str[i] - 97 + 10) * 16;
			}		
		}else if ( i%2 == 1 ){
			if (str[i]>=48 && str[i]<=57) {				
				value[i/2]=(str[i] - 48) + value[i/2];
			}else {				
				value[i/2]=(str[i] - 97 + 10) + value[i/2];
			}	
		}
		//printf("value[%d]=%d\n", i/2, value[i/2]);
		
	}
	
}


static void setTxPowerValue(int mode, char * parm, unsigned char * value)
{
	int i;
	
	if ( TXPOWER == 2 ) {
		if ( mode == 2 ) {				
			for(i=0; i<=13; i++)
			{			
				sprintf(parm, "%s%02x", parm, value[i]);
			}
		}else if (mode == 1 ) {				
			for(i=0; i<=13; i++)
			{	
				value[i]-=3;		
				sprintf(parm, "%s%02x", parm, value[i]);
			}
		}else if (mode == 0 ) {				
			for(i=0; i<=13; i++)
			{	
				value[i]-=6;		
				sprintf(parm, "%s%02x", parm, value[i]);
			}
		}						
	}else if ( TXPOWER == 1 ) {
		if ( mode == 2 ) {				
			for(i=0; i<=13; i++)
			{	
				value[i]+=3;		
				sprintf(parm, "%s%02x", parm, value[i]);
			}
		}else if (mode == 1 ) {				
			for(i=0; i<=13; i++)
			{								
				sprintf(parm, "%s%02x", parm, value[i]);
			}
		}else if (mode == 0 ) {				
			for(i=0; i<=13; i++)
			{	
				value[i]-=3;		
				sprintf(parm, "%s%02x", parm, value[i]);
			}
		}				
	}else if ( TXPOWER == 0 ) {
		if ( mode == 2 ) {				
			for(i=0; i<=13; i++)
			{	
				value[i]+=6;		
				sprintf(parm, "%s%02x", parm, value[i]);
			}
		}else if (mode == 1 ) {				
			for(i=0; i<=13; i++)
			{	
				value[i]+=3;							
				sprintf(parm, "%s%02x", parm, value[i]);
			}
		}else if (mode == 0 ) {				
			for(i=0; i<=13; i++)
			{									
				sprintf(parm, "%s%02x", parm, value[i]);
			}
		}
		
	}
}



static	MixStatusType	wlanRelease ()
{
	
	return (smpErrorReadOnly);
}

static	MixStatusType	wlanCreate ()
{
	
	return (smpErrorReadOnly);
}

static	MixStatusType	wlanDestroy ()
{
	
	return (smpErrorReadOnly);
}

static	AsnIdType	wlanNext (MixCookieType cookie, MixNamePtrType name, MixLengthPtrType namelenp)
{
	unsigned int idx;

	Wlan_Mib_Init();	
	
	cookie = cookie;
	if(snmp_oid_getnext(&wlan_mib_oid_tbl, name, *namelenp, &idx))
	{
		struct mib_oid * oid_ptr = wlan_mib_oid_tbl.oid;
		oid_ptr += idx;

		memcpy((unsigned char *)name, oid_ptr->name, oid_ptr->length);
		*namelenp = oid_ptr->length;
	
		return wlanRetrieveMibValue(idx);
	}

	return ((AsnIdType) 0);
}


static	AsnIdType	wlanGet (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen)
{
	unsigned int idx;
	char *argv[6];		
	
	Wlan_Mib_Init();	
	cookie = cookie;		
	
	if(snmp_oid_get(&wlan_mib_oid_tbl, name, namelen, &idx))
	{		
		//printf("wlanGet: Find the OID  cookie=%x name=%c idx=%d\n", cookie, name, idx);
		return wlanRetrieveMibValue(idx);		
	}
	
	printf("wlanGet: Cannot find the OID\n");	
	return ((AsnIdType) 0);
	
}


static	MixStatusType	wlanSet (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen, AsnIdType asn)
{
	unsigned int idx;
	char *argv[6];
	unsigned char strvalue[34];
	char parm[64];
	
	
	Wlan_Mib_Init();	
	cookie = cookie;
	name = name;
	namelen = namelen;
	asn = asn;	
	
	argv[1] = (char*)WLANIF[0];
	argv[2] = "set_mib";
	
	if(snmp_oid_get(&wlan_mib_oid_tbl, name, namelen, &idx))
	{	
		struct mib_oid * oid_ptr;				
		AsnLengthType		k;
		
		oid_ptr = wlan_mib_oid_tbl.oid;
		oid_ptr += idx;
		
		//printf("wlanSet: Find the OID  cookie=%x name=%c namelen=%d idx=%d asn=%d\n", cookie, name, namelen, idx, asnNumber (asnValue (asn), asnLength (asn)));			
		
		
		switch (oid_ptr->name[0])
		{
		case WLANBAND:						
			sprintf(parm, "band=%u", asnNumber (asnValue (asn), asnLength (asn)));
			argv[3] = parm;
			argv[4] = NULL;	
			break;			
			
		case WLANSSID:
			k = asnLength (asn);		
			(void) asnContents (asn, strvalue, k);	
			strvalue[k]=NULL;			
		
			sprintf(parm, "ssid=%s", strvalue);
			argv[3] = parm;
			argv[4] = NULL;	
			break;
			
		case WLANCHAN:	
			// 0: auto; 1: 1;  2: 2.....					
			sprintf(parm, "channel=%u", asnNumber (asnValue (asn), asnLength (asn)));
			argv[3] = parm;
			argv[4] = NULL;	
			break;
		
		case WLANRADIOPOWER:
		{
			char *str, string[29];
			int i, txpowermode;
			
			//TXPOWER = 2;  // 2: 60mW  1: 30mW  2: 15mW
			txpowermode = asnNumber (asnValue (asn), asnLength (asn));			
			
			// Get txpowerCCK			
			str = wlanRetrieveMibStringFromFile(WLANTXPOWERCCK, strlen(WLANTXPOWERCCK)+1);			
			str[28]='\0';
				
			stringtoByte(str, CCKValue);
			CCKValue[14]=NULL;
			
			sprintf(parm, "TxPowerCCK=");			
			setTxPowerValue(txpowermode, parm, CCKValue);
						
			argv[3] = parm;
			argv[4] = NULL;			
			do_cmd(IWPRIV, argv, 1);
			
			
			// Get txpowerOFDM
			str = wlanRetrieveMibStringFromFile(WLANTXPOWEXOFDM, strlen(WLANTXPOWEXOFDM)+1);			
			str[28]='\0';
				
			stringtoByte(str, OFDMValue);
			OFDMValue[14]=NULL;
			
			sprintf(parm, "TxPowerOFDM=");
			setTxPowerValue(txpowermode, parm, OFDMValue);
						
			argv[3] = parm;
			argv[4] = NULL;			
			
			// Save TXPOWER status
			TXPOWER = txpowermode;			
			break;
		}
			
		case WLANAUTHTYPE:						
			sprintf(parm, "authtype=%u", asnNumber (asnValue (asn), asnLength (asn)));
			argv[3] = parm;
			argv[4] = NULL;	
			break;	
			
		case WLANFRAGTHRES:				
			if ( asnNumber (asnValue (asn), asnLength (asn)) < 256 || 
			     asnNumber (asnValue (asn), asnLength (asn)) > 2346  ) {
				return (smpErrorReadOnly);
			}else { 			
				sprintf(parm, "fragthres=%u", asnNumber (asnValue (asn), asnLength (asn)));
				argv[3] = parm;
				argv[4] = NULL;	
				break;	
			}
			
		case WLANRTSTHRES:				
			if ( asnNumber (asnValue (asn), asnLength (asn)) < 0 || 
			     asnNumber (asnValue (asn), asnLength (asn)) > 2347  ) {
				return (smpErrorReadOnly);
			}else { 			
				sprintf(parm, "rtsthres=%u", asnNumber (asnValue (asn), asnLength (asn)));
				argv[3] = parm;
				argv[4] = NULL;	
				break;	
			}	
		
		case WLANBCNINT:				
			if ( asnNumber (asnValue (asn), asnLength (asn)) < 20 || 
			     asnNumber (asnValue (asn), asnLength (asn)) > 1024  ) {
				return (smpErrorReadOnly);
			}else { 			
				sprintf(parm, "bcnint=%u", asnNumber (asnValue (asn), asnLength (asn)));
				argv[3] = parm;
				argv[4] = NULL;	
				break;	
			}
		
		case WLANDATARATE:
			// 1: auto 2:1M						
			sprintf(parm, "autorate=%u", asnNumber (asnValue (asn), asnLength (asn)));
			argv[3] = parm;
			argv[4] = NULL;	
			break;	
			
		case WLANPREABBLE:
			// 0: long 1:short						
			sprintf(parm, "preamble=%u", asnNumber (asnValue (asn), asnLength (asn)));
			argv[3] = parm;
			argv[4] = NULL;	
			break;
		
		case WLANBROADCASTSSID:
			// 0: Enable 1:Disable						
			sprintf(parm, "hiddenAP=%u", asnNumber (asnValue (asn), asnLength (asn)));
			argv[3] = parm;
			argv[4] = NULL;	
			break;	
			
		case WLANENCRYPTION:
			// 0: None  1: WEP  2: WPA(TKIP)  4: WPA2(AES)  6: WPA2 Mixed 
			sprintf(parm, "encmode=%u", asnNumber (asnValue (asn), asnLength (asn)));
			argv[3] = parm;
			argv[4] = NULL;
			do_cmd(IWPRIV, argv, 1);
			
			if ( asnNumber (asnValue (asn), asnLength (asn))>= 2 ) {
				sprintf(parm, "psk_enable=%d", asnNumber (asnValue (asn), asnLength (asn))/2 );
				argv[3] = parm;
				argv[4] = NULL;		
			}	
			break;
		
		case WLANENCRYKEY1:
			k = asnLength (asn);		
			(void) asnContents (asn, strvalue, k);	
			strvalue[k]=NULL;		
			
			sprintf(parm, "wepkey1=%s", strvalue);
			argv[3] = parm;
			argv[4] = NULL;			
			break;
		
		case WLANENCRYKEY2:			
			k = asnLength (asn);		
			(void) asnContents (asn, strvalue, k);	
			strvalue[k]=NULL;			
			
			sprintf(parm, "wepkey2=%s", strvalue);
			argv[3] = parm;
			argv[4] = NULL;
			break;
			
		case WLANENCRYKEY3:			
			k = asnLength (asn);		
			(void) asnContents (asn, strvalue, k);	
			strvalue[k]=NULL;			
			
			sprintf(parm, "wepkey3=%s", strvalue);
			argv[3] = parm;
			argv[4] = NULL;
			break;
			
		case WLANENCRYKEY4:			
			k = asnLength (asn);		
			(void) asnContents (asn, strvalue, k);	
			strvalue[k]=NULL;			
			
			sprintf(parm, "wepkey4=%s", strvalue);
			argv[3] = parm;
			argv[4] = NULL;
			break;
			
		case WLANDEFAULTTXKEY:
			// 0: Key1  1: key2  2: key3  3: key4
			sprintf(parm, "wepdkeyid=%u", asnNumber (asnValue (asn), asnLength (asn)));
			argv[3] = parm;
			argv[4] = NULL;
			break;	
			
		case WLANPRESHAREDKEY:
			k = asnLength (asn);		
			(void) asnContents (asn, strvalue, k);	
			strvalue[k]=NULL;
			
			sprintf(parm, "passphrase=%s", strvalue);
			argv[3] = parm;
			argv[4] = NULL;
			break;	
									
		}	
			
		do_cmd(IWPRIV, argv, 1);		
		return (smpErrorNone);	
	}
	
	return (smpErrorReadOnly);
}

static	MixOpsType	wlanOps = {

			wlanRelease,
			wlanCreate,
			wlanDestroy,
			wlanNext,
			wlanGet,			
			wlanSet

		};	


CVoidType		wlanInit (void)
{
	unsigned char string[29], strvalue[15];	
	char *str;
	int i;	
	
	(void) misExport ((MixNamePtrType) "\53\6\1\2\1\64",
		(MixLengthType) 6, & wlanOps, (MixCookieType) 0);	
		
}
