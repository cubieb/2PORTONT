

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

#if 0
//#include "../../boa/src/LINUX/mib.h"
#include <semaphore.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/if_ether.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>
#endif

struct eoc_create_PppMss eocAddPppMSS;
struct eoc_create_PppDisconnectTime eocAddPppDisconnectTime;
unsigned int entryNumPPP;
unsigned long g_pppTestRseult=1;
static struct mib_oid_tbl ppp_mib_oid_tbl;
static long ppp_last_scan = 0;

extern int	g_pppTestFlag;
extern unsigned long 	g_cpePppIfIndex;
extern int 	g_pppTestAdmin;

enum snmp_cpe_ppptable
{ 
   CPEPPPIFINDEX = 1,
   CPEPPPPVCIFINDEX,
   CPEPPPSERVICENAME,
   CPEPPPDISCONNECTTIMEOUT,
   CPEPPPMSS,
   CPEPPPROWSTATUS,
   CPEPPPADMINSTATUS,
   CPEPPPOPERSTATUS,
   CPEPPPTESTRESULT,
   CPE_PPPTABLE_END 
};


unsigned long RMPppRetrieveValueFromFile(const char * compareStr, int offset)
{
	FILE *fp;
	char temps[0x100];
	unsigned long value;
	char *str, *endptr;
	
	value = 0;
	
	if ((fp = fopen("/tmp/ppp_auth_log", "r")) == NULL)
	{
		//printf("Open file /var/ftpStatus.txt fail !\n");
		return 0;
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

	

static	AsnIdType	pppRetrieveMibValue (unsigned int mibIdx, unsigned long ifindex)
{
	struct mib_oid * oid_ptr;	
	//MIB_CE_ATM_VC_Tp pEntry, tmp_pEntry;	
	MIB_CE_ATM_VC_T Entry, tmp_Entry;
	char vpi[6], vci[6];	
	char ifname[6];
	int flags, strStatus;
	CONN_T conn_status;
	int i;
	unsigned long pppAuth;
	char pppifname[6];
	
	
	oid_ptr = ppp_mib_oid_tbl.oid;
	oid_ptr += mibIdx;			
	
	//printf("pppGet: inputvpi = %d  inputvci = %d\n", inputvpi, inputvci);
	// To find the correct pEntry by VC_INDEX(pEntry->ifIndex)	
	for (i=0; i<entryNumPPP; i++){
		//tmp_pEntry = (MIB_CE_ATM_VC_Tp) mib_chain_get(MIB_ATM_VC_TBL, i);
		mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&tmp_Entry);			
				
		if ( tmp_Entry.cpePppIfIndex != ifindex  )
			continue;
		else if ( tmp_Entry.cpePppIfIndex == ifindex  ){
			//pEntry = tmp_pEntry;
			memcpy(&Entry, &tmp_Entry, sizeof(MIB_CE_ATM_VC_T));			
			break;
		}	
	
	}		
	
	
	// Retrieve VPI/VCI
	//snprintf(vpi, 6, "%u", Entry.vpi);
	//snprintf(vci, 6, "%u", Entry.vci);
	//printf("pppRetrieveMibValue: atoi(vpi)=%d\n", atoi(vpi) );
	//printf("pppRetrieveMibValue: atoi(vci)=%d\n", atoi(vci) );		
	
	
	// Retrieve AdminStatus and OperStatus
	if (Entry.cmode == CHANNEL_MODE_PPPOE || Entry.cmode == CHANNEL_MODE_PPPOA){
		snprintf(ifname, 6, "ppp%u", PPP_INDEX(Entry.ifIndex));
		
		// set status flag
		if (Entry.enable == 0)
		{	
			strStatus = 2;		
			conn_status = CONN_DOWN;
		}
		else
		if (getInFlags( ifname, &flags) == 1)
		{
			if (flags & IFF_UP)			
			{
				strStatus = 1;
				conn_status = CONN_UP;
			}
			else
			{
#ifdef CONFIG_PPP
				if (find_ppp_from_conf(ifname))
				{
					strStatus = 1;
					conn_status = CONN_DOWN;
				}
				else
#endif
				{
					strStatus = 1;
					//conn_status = CONN_NOT_EXIST;
					conn_status = CONN_DOWN;
				}
			}
		}
		else
		{
			strStatus = 1;
			//conn_status = CONN_NOT_EXIST;
			conn_status = CONN_DOWN;
		}	
		
	}	
	
	
	switch (oid_ptr->name[0])
	{
		
	case CPEPPPIFINDEX:									
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, Entry.cpePppIfIndex );  		
		
	case CPEPPPPVCIFINDEX:		
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, 150000+VC_INDEX(Entry.ifIndex) );	
		
	case CPEPPPSERVICENAME:
		if (Entry.cmode == CHANNEL_MODE_PPPOE){
			return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
						(CBytePtrType) ("PPPoE"), 
						(AsnLengthType) strlen ("PPPoE"));
		}else {
			return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
						(CBytePtrType) ("PPPoA"), 
						(AsnLengthType) strlen ("PPPoA"));
		}
		
		
	case CPEPPPDISCONNECTTIMEOUT:
		if ( Entry.pppCtype == CONNECT_ON_DEMAND )
			return asnUnsl (asnClassUniversal, (AsnTagType) 2, Entry.pppIdleTime);
		else 
			return asnUnsl (asnClassUniversal, (AsnTagType) 2, 0);
					
	case CPEPPPMSS:	
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, Entry.mtu);		
			
	case CPEPPPROWSTATUS:	
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, 4);   
		
	case CPEPPPADMINSTATUS:
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, strStatus);     // enable:1 , Disable:2
		
	case CPEPPPOPERSTATUS:
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, conn_status);   // up:1 , down:2
		
	case CPEPPPTESTRESULT:
		if ( conn_status == CONN_UP ) {
			g_pppTestRseult = 1;
			return asnUnsl (asnClassUniversal, (AsnTagType) 2, g_pppTestRseult);             // Testing Success
		}	            
		else {
			snprintf(pppifname, 6, "%u:", PPP_INDEX(Entry.ifIndex));
			pppAuth=RMPppRetrieveValueFromFile(pppifname, strlen(pppifname));
			
			if ( pppAuth == 3 ){
				g_pppTestRseult = 3;
				return asnUnsl (asnClassUniversal, (AsnTagType) 2, g_pppTestRseult);     // Testing AuthFail
			}else{
				g_pppTestRseult = 4;
				return asnUnsl (asnClassUniversal, (AsnTagType) 2, g_pppTestRseult);     // Testing Failure
			}	
		}
	
	 																														
	default:
		//return ((AsnIdType) 0);
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, 1);
				
	}
	
}



void ppp_Mib_Init(void)
{
	struct timeval now;
	int j;
	int i, k;	
	//MIB_CE_ATM_VC_Tp pEntry;
	MIB_CE_ATM_VC_T Entry;			
	unsigned long initifindex;
	
	gettimeofday(&now, 0);

//	printf("Now:%u:%u\n",now.tv_sec, now.tv_usec);

	//if((now.tv_sec - ppp_last_scan) > 10)	//  10 sec
	{		
		
		unsigned int oid_tbl_size;
		struct mib_oid * oid_ptr;
		
		ppp_last_scan = now.tv_sec;			

		// create oid table
		free_mib_tbl(&ppp_mib_oid_tbl);
		
		entryNumPPP = mib_chain_total(MIB_ATM_VC_TBL);		
		
		
		// ppp MIB has 9*entryNumPPP objects
		oid_tbl_size = 9;
		
		create_mib_tbl(&ppp_mib_oid_tbl,  oid_tbl_size*entryNumPPP, 4); 
		oid_ptr = ppp_mib_oid_tbl.oid;	
		
		
		for (j=1; j<=oid_tbl_size; j++) {			
			initifindex = 0;			
			
			for (i=0; i<entryNumPPP; i++){				
				//pEntry = pppTableSort(initifindex);									
				pppTableSort(initifindex, &Entry);				
				initifindex = Entry.cpePppIfIndex;								
				
				if ((Entry.cmode == CHANNEL_MODE_PPPOE || Entry.cmode == CHANNEL_MODE_PPPOA) && initifindex!=0xff ){					
								
					k= 80;                                                  // Hex:50															
					oid_ptr->length = 4;
					oid_ptr->name[0] = j;
					oid_ptr->name[1] = 140;                                 // (1) Hex= 8c
					oid_ptr->name[2] = 232;                                 // (2) Hex= e8
					oid_ptr->name[3] = k+(initifindex-210000);              // (3) Hex= 50  the (1)0x8c, (2)0xe8, (3)0x50 value is means 210000												
					oid_ptr++;
				}else
					continue;												
			}	
		}
		
		
				
	}
}



static	MixStatusType	pppRelease ()
{
	
	return (smpErrorReadOnly);
}

static	MixStatusType	pppCreate ()
{
	
	return (smpErrorReadOnly);
}

static	MixStatusType	pppDestroy ()
{
	
	return (smpErrorReadOnly);
}




static	AsnIdType	pppNext (MixCookieType cookie, MixNamePtrType name, MixLengthPtrType namelenp)
{
	unsigned int idx=0;
		
	
	ppp_Mib_Init();	
	
	cookie = cookie;	
	
	if(snmp_oid_getnext(&ppp_mib_oid_tbl, name, *namelenp, &idx))
	{		
		struct mib_oid * oid_ptr = ppp_mib_oid_tbl.oid;
		oid_ptr += idx;

		memcpy((unsigned char *)name, oid_ptr->name, oid_ptr->length);		
		*namelenp = oid_ptr->length;				
		
		//name[3]-80+210000 = cpePppIfIndex(210000 ~ 210008)
		return pppRetrieveMibValue(idx, name[3]-80+210000);
	}	
	
	return ((AsnIdType) 0);
}


static	AsnIdType	pppGet (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen)
{
	unsigned int idx;	
	
			
	ppp_Mib_Init();
	
	cookie = cookie;
	
	if(snmp_oid_get(&ppp_mib_oid_tbl, name, namelen, &idx))
	{			
		if ( namelen != 0 )	
		        //name[3]-80+210000 = cpePppIfIndex(210000 ~ 210008)		
			return pppRetrieveMibValue(idx, name[3]-80+210000);  
		else	
			return ((AsnIdType) 0);	
			
	}
	// Mason Yu. If do RowStatus with CreateAndGo(4), snmp agent will not get this OID. 
	// So we should retuen a valid value to pass the predure to pppSet().
	else if ( name[0] == 6){		
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, 1);	
	}
	else
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, 1);
		
	
	printf("pppGet: Cannot find the OID\n");
	return ((AsnIdType) 0);
	
}


static	MixStatusType	pppSet  (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen, AsnIdType asn)
{
	unsigned int idx;	
	long i;	
	struct channel_conf_ppp_para para;
	unsigned long CpePppIfIndex;
	int entryNum;
	//MIB_CE_ATM_VC_Tp pEntry;
	MIB_CE_ATM_VC_T Entry;
	int j;
	
	
	ppp_Mib_Init();	
	
	cookie = cookie;
	name = name;
	namelen = namelen;
	asn = asn;	
	
	//for ( i=0; i<6; i++)
	//		printf("pppSet: name[%d] = %d\n", i, name[i]);
			
	if(snmp_oid_get(&ppp_mib_oid_tbl, name, namelen, &idx))
	{	
		struct mib_oid * oid_ptr;				
		AsnLengthType		k;
		int option;				
		unsigned char strvalue[256];
	
		oid_ptr = ppp_mib_oid_tbl.oid;
		oid_ptr += idx;		
		
		
		switch (oid_ptr->name[0])
		{
			
		case CPEPPPADMINSTATUS:
			
			i =  asnNumber (asnValue (asn), asnLength (asn));
			//printf("pppSet: To do CPEPPPADMINSTATUS   i=%d\n", i);
			
			para.pppIfIndex=name[3]-80+210000;
			if ( i == 1 || i == 2 ) {						
				para.admin=i;			
				para.IdleTime=0;
				strcpy(para.pppUsername, "");
				strcpy(para.pppPassword, "");	
				para.mtu=0;					
				modifyChannelConfPPP(&para);
			}else {
				//printf("pppSet: Do PPP Testing\n");				
				g_cpePppIfIndex = name[3]-80+210000;
				g_pppTestAdmin = i;
				g_pppTestFlag = 1;
				
				para.admin=0;			
				para.IdleTime=0;
				strcpy(para.pppUsername, "");
				strcpy(para.pppPassword, "");
				para.mtu=0;
				modifyChannelConfPPP(&para);	
				
			}		
			
			return (smpErrorNone);
		
		case CPEPPPDISCONNECTTIMEOUT:
			//printf("pppSet: To do CPEPPPDISCONNECTTIMEOUT\n");
			i =  asnNumber (asnValue (asn), asnLength (asn));
			
			para.pppIfIndex=name[3]-80+210000;						
			para.admin=0;			
			para.IdleTime=i;
			strcpy(para.pppUsername, "");
			strcpy(para.pppPassword, "");
			para.mtu=0;
						
			modifyChannelConfPPP(&para);			
			return (smpErrorNone);
				
		case CPEPPPMSS:
			//printf("Set CPEPPPMSS: Do notthing\n");			
			i =  asnNumber (asnValue (asn), asnLength (asn));
			
			para.pppIfIndex=name[3]-80+210000;						
			para.admin=0;			
			para.IdleTime=0;
			strcpy(para.pppUsername, "");
			strcpy(para.pppPassword, "");
			para.mtu=i;
						
			modifyChannelConfPPP(&para);
			return (smpErrorNone);
		
		default:
			return (smpErrorNone);			
		}
				
	}else if ( name[0] == 6 ) {
		
		i =  asnNumber (asnValue (asn), asnLength (asn));		
		CpePppIfIndex = name[3]-80+210000;
		//printf("pppSet: This is RowStatus and value=%d. CpePppIfIndex=0x%x\n", i, CpePppIfIndex);
		
		if (i == 4 ){	// CreateAndGo	
			char vpi[6], vci[6];
			struct channel_conf_para paraIfIndex;
				
			for (j=0; j<entryNumPPP; j++){
				//pEntry = (MIB_CE_ATM_VC_Tp) mib_chain_get(MIB_ATM_VC_TBL, j);
				mib_chain_get(MIB_ATM_VC_TBL, j, (void *)&Entry);
				
				if ((Entry.cmode == CHANNEL_MODE_PPPOE || Entry.cmode == CHANNEL_MODE_PPPOA) && Entry.cpePppIfIndex == 0xff ){					
					//Entry.cpePppIfIndex = CpePppIfIndex;					
					snprintf(vpi, 6, "%u", Entry.vpi);
        				snprintf(vci, 6, "%u", Entry.vci);
        				
        				paraIfIndex.inputvpi=atoi(vpi);
        				paraIfIndex.inputvci=atoi(vci);
        				paraIfIndex.cmode=0;
        				paraIfIndex.admin=0;
        				paraIfIndex.natmode=0;
        				paraIfIndex.dhcpmode=0;        		
        				paraIfIndex.pppIfIndex=CpePppIfIndex;	
        				paraIfIndex.IpIndex=0;		
        				paraIfIndex.encap=100;
        				paraIfIndex.brmode=100;        				
        				modifyChannelConf(&paraIfIndex);					
					//printf("pppSet: Find the cpePppIfIndex(%d) and set it!\n", CpePppIfIndex);
					
					// Really Set CpePppMSS via EOC channel
					if ( eocAddPppMSS.PppMssFlag == 1 ) {
						para.pppIfIndex=eocAddPppMSS.pppIfIndex;						
						para.admin=0;			
						para.IdleTime=0;
						strcpy(para.pppUsername, "");
						strcpy(para.pppPassword, "");
						para.mtu=eocAddPppMSS.PppMssValue;
						
						modifyChannelConfPPP(&para);
						eocAddPppMSS.PppMssFlag=0;
					}
					
					// Really Set CpePppConnectTimeout via EOC channel
					if ( eocAddPppDisconnectTime.PppDisconnectTimeFlag == 1 ) {
						para.pppIfIndex=eocAddPppDisconnectTime.pppIfIndex;						
						para.admin=0;			
						para.IdleTime=eocAddPppDisconnectTime.PppDisconnectTimeValue;
						strcpy(para.pppUsername, "");
						strcpy(para.pppPassword, "");
						para.mtu=0;
						
						modifyChannelConfPPP(&para);
						eocAddPppDisconnectTime.PppDisconnectTimeFlag=0;
					}	
					return (smpErrorNone);			
					
				}else
					continue;
					
			}
				
		}	
		//return (smpErrorNone);	
	}
	else if ( name[0] == 2 ) {
		// We should save values for Setting via EOC channel	
		// We can do nothing	
		i =  asnNumber (asnValue (asn), asnLength (asn));		
		CpePppIfIndex = name[3]-80+210000;
		//printf("pppSet: CpePppIfIndex setting and value=0x%x. CpePppIfIndex=0x%x\n", i, CpePppIfIndex);			
		return (smpErrorNone);	
	}
	else if ( name[0] == 4 ) {	
		// We should save values for Setting via EOC channel	
		i =  asnNumber (asnValue (asn), asnLength (asn));		
		CpePppIfIndex = name[3]-80+210000;
		//printf("pppSet: CPEPPPDISCONNECTTIMEOUT setting and value=%d. CpePppIfIndex=0x%x\n", i, CpePppIfIndex);	
		
		if ( name[1] = 0x8c && name[2] == 0xe8) {
			eocAddPppDisconnectTime.pppIfIndex=name[3]-80+210000;
			eocAddPppDisconnectTime.PppDisconnectTimeFlag=1;
			eocAddPppDisconnectTime.PppDisconnectTimeValue=i;
		}		
		return (smpErrorNone);	
	}		
	else if ( name[0] == 7 ) {		
		i =  asnNumber (asnValue (asn), asnLength (asn));		
		para.pppIfIndex=name[3]-80+210000;
		//printf("pppSet: CpePppAdminStatus setting and value=%d. CpePppIfIndex=0x%x\n", i, para.pppIfIndex);
		
		if ( i == 1 || i == 2 ) {						
			para.admin=i;			
			para.IdleTime=0;
			strcpy(para.pppUsername, "");
			strcpy(para.pppPassword, "");
			para.mtu=0;
					
			modifyChannelConfPPP(&para);
		}else {
			//printf("Do PPP Testing\n");				
			g_cpePppIfIndex = name[3]-80+210000;
			g_pppTestAdmin = i;
			g_pppTestFlag = 1;
		
		}		
		
		return (smpErrorNone);		
	}
	else if ( name[0] == 5 ) {	
		// We should save values for Setting via EOC channel	
		i =  asnNumber (asnValue (asn), asnLength (asn));		
		para.pppIfIndex=name[3]-80+210000;
		//printf("pppSet: CpePppMSS setting and value=%d. CpePppIfIndex=0x%x\n", i, para.pppIfIndex);	
		
		if ( name[1] = 0x8c && name[2] == 0xe8) {
			eocAddPppMSS.pppIfIndex=name[3]-80+210000;
			eocAddPppMSS.PppMssFlag=1;
			eocAddPppMSS.PppMssValue=i;
		}				
		return (smpErrorNone);					
	}
		
	
}

static	MixOpsType	pppOps = {

			pppRelease,
			pppCreate,
			pppDestroy,
			pppNext,
			pppGet,			
			pppSet

			};

CVoidType		pppInit (void)
{
	eocAddPppMSS.PppMssFlag=0;
	eocAddPppDisconnectTime.PppDisconnectTimeFlag=0;
	
	(void) misExport ((MixNamePtrType) "\53\6\1\4\1\201\204\114\2\4\1",
		(MixLengthType) 11, & pppOps, (MixCookieType) 0);
		
		
}
