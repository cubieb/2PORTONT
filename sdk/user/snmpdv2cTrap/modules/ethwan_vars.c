#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>

#include "ctypes.h"
#include "error.h"
#include "local.h"
#include "adsl_vars.h"
#include "mix.h"
#include "mis.h"
#include "asn.h"

#include "mib_tool.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include "ethwan_vars.h"

/**
ethwan_snmp use these global values
**/
static long glb_ethwanDotvarsDotc_last_scan = 0;
unsigned int glb_ethwanNum = 0;

static struct mib_oid_tbl ethwan_mib_oid_tbl;

//-----------------------------------------------

//MACRO
#define SNMP_ENABLE 	1
#define SNMP_DISABLE	2

#define SNMP_FIX_MODE  1
#define SNMP_DHCP_MODE 2

#define MAX_WAN_NUM 		16

#define CHECK_NAS_IDX(a) (a >> 16)
#define min(a,b)  (((a) < (b)) ? (a) : (b))


//
enum snmp_ethwantable
{
	//----common ---
	ETHWANIFINDEX = 1,
#if defined(NEW_PORTMAPPING) || defined(CONFIG_USER_BRIDGE_GROUPING)

	ETHWANENVLAN,
	/*		|
			|---ENABLE
			|	|
			|	|---ETHVLANID
			|	|---ETHWAN8021P
			|
			|---DISANBLE
				|
				|--- Nothing at all
	*/
		ETHWANVLANID,
		ETHWAN8021P,
	ETHWANPORTMP,
#endif
	ETHWANADMINSTU,
	ETHWANMODE,
	
	
	//----FOR IP/PPPOE mode---
	ETHWANNAPT,
#ifdef CONFIG_IGMPPROXY_MULTIWAN

	ETHWANIGMPPRXY,
#endif
#ifdef CONFIG_USER_IP_QOS	
	ETHWANQOS,
#endif
	ETHWANDFTROUT,
	ETHWANIPVER,

	//---only for IP mode ----
	ETHWANIPTYPE,
	/*		|
			|---FIXED
			|	|
			|	|---ETHWANIPADDR
			|	|---ETHWANREMADDR
			|	|---ETHWANNETMASK
			|
			|---DHCP
				|
				|--- Nothing at all
	*/
		ETHWANIPADDR,
		ETHWANREMADDR,
		ETHWANNETMASK,

	//---only for PPP mode ----
	ETHWANPPP_USERNAME,
	ETHWANPPP_PASSWD,
	ETHWANPPP_CTYPE,// connect on demand
	/*		|
			|---Continuis
			|	
			|
			|---Connect on demand
			|	|
			|	|--- ETHWANPPP_IDLETIME
			|
			|---Manual
	*/

		ETHWANPPP_IDLETIME,

    //---THE END----
	ETHWANTABLE_END,
};

enum OID_PTR_INDEX
{
	ETHWANTBL_SUB_OID_SEQ = 0, 	// in the oid it is the  "/2"
	ETHWANENTRY_SUB_OID_SEQ,		// in the oid it is the  "/1"
	ETHWANMEMB_SUB_OID_SEQ,		//
	ETHWANMEMB_ARRAY_TAIL,		//for that we have more than one ethwan
	ETHWAN_COMPARE_LENGTH,		// compare length, for the origin bad compare algth!!!
	ETHWANINDEX_FOURTH_OCTET = ETHWAN_COMPARE_LENGTH,
	ETHWANINDEX_THIRD_OCTET,
	
	SUB_OID_SEQ_END,
};


enum ETHWAN_CONNECT_MODE
{
	ETHWAN_PPPOE  = 1,
	ETHWAN_BRIDGE  = 2,
	ETHWAN_ROUTE = 3,
};

enum ETHWAN_PPP_CONNECT_MODE
{
	PPP_CONN_CONTINUOUS = 0,
	PPP_CONN_ON_DEMAND = 1,
	PPP_CONN_ON_MANNUAL = 2,
};

unsigned char oid_ifNumber[] = "\1\0";
unsigned char oid_ifEntry[]  = "\2\1";//, "\2\2", "\2\3", "\2\4", "\2\5", "\2\6", "\2\7", "\2\8"};
unsigned char oid_ifEntry_item[ETHWANTABLE_END] = 
{ 

	//----common ---
	ETHWANIFINDEX,
#if defined(NEW_PORTMAPPING)  || defined(CONFIG_USER_BRIDGE_GROUPING)

	ETHWANENVLAN,
	/*		|
			|---ENABLE
			|	|
			|	|---ETHVLANID
			|	|---ETHWAN8021P
			|
			|---DISANBLE
				|
				|--- Nothing at all
	*/
		ETHWANVLANID,
		ETHWAN8021P,
	ETHWANPORTMP,
#endif
	ETHWANADMINSTU,
	ETHWANMODE,
	
	
	//----FOR IP/PPPOE mode---
	ETHWANNAPT,
#ifdef CONFIG_IGMPPROXY_MULTIWAN

	ETHWANIGMPPRXY,
#endif
#ifdef CONFIG_USER_IP_QOS
	ETHWANQOS,
#endif
	ETHWANDFTROUT,
	ETHWANIPVER,

	//---only for IP mode ----
	ETHWANIPTYPE,
	/*		|
			|---FIXED
			|	|
			|	|---ETHWANIPADDR
			|	|---ETHWANREMADDR
			|	|---ETHWANNETMASK
			|
			|---DHCP
				|
				|--- Nothing at all
	*/
		ETHWANIPADDR,
		ETHWANREMADDR,
		ETHWANNETMASK,

	//---only for PPP mode ----
	ETHWANPPP_USERNAME,
	ETHWANPPP_PASSWD,
	ETHWANPPP_CTYPE,// connect on demand
	/*		|
			|---Continuis
			|	
			|
			|---Connect on demand
			|	|
			|	|--- ETHWANPPP_IDLETIME
			|
			|---Manual
	*/

		ETHWANPPP_IDLETIME,

	ETHWANTABLE_END,

};


static	AsnIdType ethwanRetrieveMibValue (unsigned int mibIdx);//, const unsigned char hi_idx, const unsigned char low_idx);

void Ethwan_Mib_Init(void);


static	MixStatusType	ethwanRelease (MixCookieType cookie)
{
	cookie = cookie;
	return (smpErrorGeneric);
}

static	MixStatusType ethwanCreate (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen, AsnIdType asn)
{
	cookie = cookie;
	name = name;
	namelen = namelen;
	asn = asn;
	return (smpErrorGeneric);
}

static	MixStatusType	ethwanDestroy (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen)
{
	cookie = cookie;
	name = name;
	namelen = namelen;
	return (smpErrorGeneric);
}


static	AsnIdType	ethwanNext (MixCookieType cookie, MixNamePtrType name, MixLengthPtrType namelenp)
{
	unsigned int idx=0;	

	AUG_SNMP_PRT("the namelen is %d. name is 0x%x 0x%x 0x%x 0x%x\n",*namelenp, name[0],name[1],name[2],name[3]);

	Ethwan_Mib_Init();		
	
	cookie = cookie;	
		
	if(snmp_oid_getnext(&ethwan_mib_oid_tbl, name, *namelenp, &idx))
	{		
		struct mib_oid * oid_ptr = ethwan_mib_oid_tbl.oid;

		AUG_SNMP_PRT("The idx is %d\n", idx);
		
		oid_ptr += idx;

		if(0 == idx)
		{
			memcpy((unsigned char *)name, oid_ptr->name, oid_ptr->length);		
			*namelenp = oid_ptr->length;				
		}
		else
			memcpy((unsigned char *)name, oid_ptr->name, ETHWAN_COMPARE_LENGTH);		
			*namelenp = ETHWAN_COMPARE_LENGTH;

			
		return ethwanRetrieveMibValue(idx);
	}

	return ((AsnIdType) 0);
}

static	AsnIdType	ethwanGet (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen)
{
	unsigned int idx;

	AUG_SNMP_PRT("the namelen is %d. name is 0x%x 0x%x 0x%x 0x%x\n",namelen, name[0],name[1],name[2],name[3]);

	Ethwan_Mib_Init();

	cookie = cookie;
	if(snmp_oid_get(&ethwan_mib_oid_tbl, name, namelen, &idx))
	{
		AUG_SNMP_PRT("the idx is %d\n", idx);
		return ethwanRetrieveMibValue(idx);
	}

	return ((AsnIdType) 0);
}


void Ethwan_Mib_Init(void)
{
	struct timeval now;
	gettimeofday(&now, 0);

	if((now.tv_sec - glb_ethwanDotvarsDotc_last_scan) > 10)	//  10 sec
	{
		unsigned int wanNum, ethwanNum, ret;
		int i, j;

		MIB_CE_ATM_VC_T entry;
		uint32_t ifIndex;

		unsigned int ifIdx_Array[MAX_WAN_NUM];
		unsigned int idx, item, oid_tbl_size;

		struct mib_oid * oid_ptr;

		glb_ethwanNum = 0;

		wanNum = ethwanNum = 0;
		memset(ifIdx_Array, 0, sizeof(int) * MAX_WAN_NUM);

		glb_ethwanDotvarsDotc_last_scan = now.tv_sec;

	
		free_mib_tbl(&ethwan_mib_oid_tbl);		

		wanNum = mib_chain_total(MIB_ATM_VC_TBL);
		for(i = 0; i < wanNum; ++i)
		{
			if(mib_chain_get(MIB_ATM_VC_TBL, i, (void*)&entry))
			{
				ifIndex = entry.ifIndex;
				if(CHECK_NAS_IDX(ifIndex))// && (PPP_INDEX(ifIndex) == DUMMY_PPP_INDEX))
				{	
					ifIdx_Array[ethwanNum] = ifIndex;
					ethwanNum++;
					glb_ethwanNum++;
				}
			}
		}

		AUG_SNMP_PRT("the glb_ethwanNum is %d\n", glb_ethwanNum);

		oid_tbl_size = 1 + ETHWANTABLE_END*ethwanNum; // IFNUMBER + N * IFENTRY

		AUG_SNMP_PRT("oid_tbl_size is %d\n", oid_tbl_size);

		ret = create_mib_tbl(&ethwan_mib_oid_tbl,  oid_tbl_size, SUB_OID_SEQ_END); 
		oid_ptr = ethwan_mib_oid_tbl.oid;

		if(!oid_ptr)
			AUG_SNMP_PRT("oid_ptr is  NULL !!!!!!!!\n");


		oid_ptr->length = sizeof(oid_ifNumber) - 1;
		memcpy(oid_ptr->name, oid_ifNumber, oid_ptr->length);
		oid_ptr++;		


		for(item = 0;item < ETHWANTABLE_END; item++)
		{
			for(idx = 0; idx < ethwanNum; idx++)
			{

					oid_ptr->length = SUB_OID_SEQ_END;
					memcpy(oid_ptr->name, oid_ifEntry, sizeof(oid_ifEntry) - 1);

					//as oid_ifEntry is 2 bytes, we set the enum OID_PTR_INDEX has 2 sub oid ;

					oid_ptr->name[ETHWANMEMB_SUB_OID_SEQ] 	= oid_ifEntry_item[item];
					oid_ptr->name[ETHWANMEMB_ARRAY_TAIL] 	= idx + 1;
					oid_ptr->name[ETHWANINDEX_FOURTH_OCTET] = (ifIdx_Array[idx] & 0xff);
					oid_ptr->name[ETHWANINDEX_THIRD_OCTET] 	= ((ifIdx_Array[idx] >> 8) & 0xff );

					

					oid_ptr++;
			}
		}
		AUG_SNMP_PRT("finish\n");
	}
}

static	AsnIdType ethwanRetrieveMibValue (unsigned int mibIdx)
{
	struct mib_oid * oid_ptr;
	MIB_CE_ATM_VC_T Entry, tmp_Entry;
	int i, snmp_mode;
	unsigned int wanNum, hi_idx, low_idx, item;
	
	// the first oid_ptr is to get the ethwan num;
	if(mibIdx == 0)
	{
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, glb_ethwanNum);;	
	}
		
	wanNum = mib_chain_total(MIB_ATM_VC_TBL);
	if (mibIdx < ethwan_mib_oid_tbl.total)
	{
		oid_ptr = ethwan_mib_oid_tbl.oid;
		oid_ptr += mibIdx;	
		item 	= oid_ptr->name[ETHWANMEMB_SUB_OID_SEQ];	
		low_idx = oid_ptr->name[ETHWANINDEX_FOURTH_OCTET];
		hi_idx  = oid_ptr->name[ETHWANINDEX_THIRD_OCTET];
	}
	else
		return 0;
	

	// To find the correct pEntry by ifIndex	
	for (i = 0; i < wanNum; i++)
	{
		mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&tmp_Entry);

		//we only treat the ethwan interface;
		if(CHECK_NAS_IDX(tmp_Entry.ifIndex))
		{			
			if((tmp_Entry.ifIndex & 0xffff ) == (low_idx | (hi_idx << 8)))
			{
				memcpy(&Entry, &tmp_Entry, sizeof(MIB_CE_ATM_VC_T));
				break;
			}
		}
	}

	
	//if none of the entery meet the needs, return 0;
	if(i == wanNum)
		return 0; 

	AUG_SNMP_PRT("the Entry.ifIndex is 0x%x\n", Entry.ifIndex);
		

	snmp_mode=0;
	if (Entry.cmode == CHANNEL_MODE_PPPOE)
		snmp_mode = ETHWAN_PPPOE;		
	else if (Entry.cmode == CHANNEL_MODE_BRIDGE)
		snmp_mode = ETHWAN_BRIDGE;
	else if (Entry.cmode == CHANNEL_MODE_IPOE)
		snmp_mode = ETHWAN_ROUTE;

	switch(snmp_mode)
	{
		case ETHWAN_PPPOE:
			if((item >= ETHWANIPTYPE) && (item <= ETHWANNETMASK))
				return ((AsnIdType) 0);

			break;
		case ETHWAN_BRIDGE:
#if defined(NEW_PORTMAPPING) || defined(CONFIG_USER_BRIDGE_GROUPING)
			if(item > ETHWANPORTMP)
				return ((AsnIdType) 0);
#endif

			break;

		case ETHWAN_ROUTE:
			if((item >= ETHWANPPP_USERNAME) && (item <= ETHWANPPP_IDLETIME))
				return ((AsnIdType) 0);

			break;
				
		default:
			return ((AsnIdType) 0);
	}

	AUG_SNMP_PRT("the item is %d\n", item);
	
	switch (item)
	{
		case ETHWANIFINDEX:		
			return asnUnsl (asnClassUniversal, (AsnTagType) 2, Entry.ifIndex);	
#if defined(NEW_PORTMAPPING) || defined(CONFIG_USER_BRIDGE_GROUPING)		
		case ETHWANENVLAN:
			if(Entry.vlan)
				return asnUnsl (asnClassUniversal, (AsnTagType) 2, SNMP_ENABLE);
			else
				return asnUnsl (asnClassUniversal, (AsnTagType) 2, SNMP_DISABLE);
		case ETHWANVLANID:
		case ETHWAN8021P:
			if(Entry.vlan)
			{
				if(ETHWANVLANID == item)
					return asnUnsl (asnClassUniversal, (AsnTagType) 2, Entry.vid);
				else
					return asnUnsl (asnClassUniversal, (AsnTagType) 2, Entry.vprio);
			}
			else
				return 0;
		case ETHWANPORTMP:
			return asnUnsl (asnClassUniversal, (AsnTagType) 2, Entry.itfGroup);
#endif	
		case ETHWANADMINSTU:
			if(Entry.enable)
				return asnUnsl (asnClassUniversal, (AsnTagType) 2, SNMP_ENABLE);
			else
				return asnUnsl (asnClassUniversal, (AsnTagType) 2, SNMP_DISABLE);
				
		case ETHWANMODE:
			return asnUnsl (asnClassUniversal, (AsnTagType) 2, snmp_mode);

		case ETHWANNAPT:
			if(Entry.napt)
				return asnUnsl (asnClassUniversal, (AsnTagType) 2, SNMP_ENABLE);
			else
				return asnUnsl (asnClassUniversal, (AsnTagType) 2, SNMP_DISABLE);
#ifdef CONFIG_USER_IP_QOS				
		case ETHWANQOS:
			if(Entry.enableIpQos)
				return asnUnsl (asnClassUniversal, (AsnTagType) 2, SNMP_ENABLE);
			else
				return asnUnsl (asnClassUniversal, (AsnTagType) 2, SNMP_DISABLE);
#endif			
#ifdef CONFIG_IGMPPROXY_MULTIWAN

		case ETHWANIGMPPRXY:
			if(Entry.enableIGMP)
				return asnUnsl (asnClassUniversal, (AsnTagType) 2, SNMP_ENABLE);
			else
				return asnUnsl (asnClassUniversal, (AsnTagType) 2, SNMP_DISABLE);
#endif
		case ETHWANDFTROUT:
			if(Entry.dgw)
				return asnUnsl (asnClassUniversal, (AsnTagType) 2, SNMP_ENABLE);
			else
				return asnUnsl (asnClassUniversal, (AsnTagType) 2, SNMP_DISABLE);
		
		case ETHWANIPTYPE:
			if(DHCP_DISABLED == Entry.ipDhcp)
				return asnUnsl (asnClassUniversal, (AsnTagType) 2, SNMP_FIX_MODE);
			else
				return asnUnsl (asnClassUniversal, (AsnTagType) 2, SNMP_DHCP_MODE);

		case ETHWANIPADDR:
		case ETHWANREMADDR:
		case ETHWANNETMASK:
			{
				uint32_t _ipaddr;
				char netAddr[20];

				if(ETHWANIPADDR == item)
				{
					strncpy(netAddr, (const char *)inet_ntoa(*((struct in_addr *)Entry.ipAddr)), 20);
					return asnOctetString (asnClassUniversal, (AsnTagType)4, (CBytePtrType)netAddr, 
						(AsnLengthType) strlen ((char *)netAddr));
				}
				
				if(ETHWANREMADDR == item)
				{
					strncpy(netAddr, (const char *)inet_ntoa(*((struct in_addr *)Entry.remoteIpAddr)), 20);
					return asnOctetString (asnClassUniversal, (AsnTagType)4, (CBytePtrType)netAddr, 
						(AsnLengthType) strlen ((char *)netAddr));

				}

				if(ETHWANNETMASK == item)
				{
					strncpy(netAddr, (const char *)inet_ntoa(*((struct in_addr *)Entry.netMask)), 20);
					return asnOctetString (asnClassUniversal, (AsnTagType)4, (CBytePtrType)netAddr, 
						(AsnLengthType) strlen ((char *)netAddr));

				}
			}
		case ETHWANPPP_USERNAME:
			return asnOctetString (asnClassUniversal, (AsnTagType)4, (CBytePtrType)Entry.pppUsername, 
						(AsnLengthType) strlen ((char *)(Entry.pppUsername)));
		case ETHWANPPP_PASSWD:
			return asnOctetString (asnClassUniversal, (AsnTagType)4, (CBytePtrType)Entry.pppPassword, 
						(AsnLengthType) strlen ((char *)(Entry.pppPassword)));

		case ETHWANPPP_CTYPE:
		case ETHWANPPP_IDLETIME:
		{
			if(ETHWANPPP_CTYPE == item)
				return asnUnsl (asnClassUniversal, (AsnTagType) 2, Entry.pppCtype);
				
			if(PPP_CONN_ON_DEMAND == Entry.pppCtype ) // 1 means connect on demand
				return asnUnsl (asnClassUniversal, (AsnTagType) 2, Entry.pppIdleTime);
			else
				return 0;
		}																							
		default:
			/**
			if return 0 , the windows end software means return "no such"
			**/
			//return ((AsnIdType) 0);
			/**
			we return a value 0;
			**/
			return asnUnsl (asnClassUniversal, (AsnTagType) 2, 0);		
	}


	
}

static	MixStatusType	ethwanSet (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen, AsnIdType asn)
{
	unsigned int idx;	
	int i, selected;
	MIB_CE_ATM_VC_T Entry;
	unsigned int ifIndex;
	int snmp_mode;
	int num  = 0;

	unsigned char strvalue[256];
	
	Ethwan_Mib_Init();	

	AUG_SNMP_PRT("the namelen is %d\n", namelen);
	AUG_SNMP_PRT("the name is %d %d %d %d\n", name[0], name[1], name[2], name[3]);
	
	if(snmp_oid_get(&ethwan_mib_oid_tbl, name, namelen, &idx))
	{
		struct mib_oid *oid_ptr;				
		AsnLengthType k;

		unsigned int wanNum, low_idx, item;
		unsigned int totalEntry;

		oid_ptr = ethwan_mib_oid_tbl.oid;
		oid_ptr += idx;		

		item  = oid_ptr->name[ETHWANMEMB_SUB_OID_SEQ];

		ifIndex =  	0x10000
					|	(oid_ptr->name[ETHWANINDEX_THIRD_OCTET] << 8)
					|	(oid_ptr->name[ETHWANINDEX_FOURTH_OCTET]);

		
		totalEntry = mib_chain_total(MIB_ATM_VC_TBL); /* get chain record size */
	
		for (i = 0; i < totalEntry; i++)
		{
			mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry); /* get the specified chain record */

			if(Entry.ifIndex == ifIndex)
			{
				selected = i;
				break;
			}
		}

		if(i == totalEntry)
			return 0;
		
		
		snmp_mode=0;
		if (Entry.cmode == CHANNEL_MODE_PPPOE)
			snmp_mode = ETHWAN_PPPOE;		
		else if (Entry.cmode == CHANNEL_MODE_BRIDGE)
			snmp_mode = ETHWAN_BRIDGE;
		else if (Entry.cmode == CHANNEL_MODE_IPOE)
			snmp_mode = ETHWAN_ROUTE;

		switch(snmp_mode)
		{
			case ETHWAN_PPPOE:
				if((item >= ETHWANIPTYPE) && (item <= ETHWANNETMASK))
					return smpErrorNoSuch;

				break;
			case ETHWAN_BRIDGE:
#if defined(NEW_PORTMAPPING) || defined(CONFIG_USER_BRIDGE_GROUPING)
				if(item > ETHWANPORTMP)
					return smpErrorNoSuch;
#endif
				break;

			case ETHWAN_ROUTE:
				if((item >= ETHWANPPP_USERNAME) && (item <= ETHWANPPP_IDLETIME))
					return smpErrorNoSuch;

				break;
					
			default:
				return smpErrorGeneric;
		}
		
		switch (item)
		{
#if defined(NEW_PORTMAPPING) || defined(CONFIG_USER_BRIDGE_GROUPING)		
			case ETHWANENVLAN:
				i =  asnNumber (asnValue (asn), asnLength (asn));

				if(SNMP_ENABLE == i)
					Entry.vlan = 1;
				else if(SNMP_DISABLE == i)
					Entry.vlan = 0;
				else
					return smpErrorBadValue;

				mib_chain_update(MIB_ATM_VC_TBL, (char *)&Entry, selected);
						
				return (smpErrorNone);

			case ETHWANVLANID:
				i =  asnNumber (asnValue (asn), asnLength (asn));

				if(i < 10)
					return smpErrorBadValue;

				Entry.vid = i;
				mib_chain_update(MIB_ATM_VC_TBL, (char *)&Entry, selected);
						
				return (smpErrorNone);

			case ETHWAN8021P:
				i =  asnNumber (asnValue (asn), asnLength (asn));

				if(i < 0 || i > 7)
					return smpErrorBadValue;

				
				Entry.vprio = i;
				mib_chain_update(MIB_ATM_VC_TBL, (char *)&Entry, selected);
						
				return (smpErrorNone);

			case ETHWANPORTMP:
				i =  asnNumber (asnValue (asn), asnLength (asn));
				Entry.itfGroup = i;
				mib_chain_update(MIB_ATM_VC_TBL, (char *)&Entry, selected);
				
				return (smpErrorNone);	
#endif
			case ETHWANADMINSTU:
				i =  asnNumber (asnValue (asn), asnLength (asn));

				if(SNMP_ENABLE == i)
					Entry.enable = 1;
				else if(SNMP_DISABLE == i)
					Entry.enable = 0;
				else
					return smpErrorBadValue;

				mib_chain_update(MIB_ATM_VC_TBL, (char *)&Entry, selected);
						
				return (smpErrorNone);			

			case ETHWANMODE:
				i =  asnNumber (asnValue (asn), asnLength (asn));

				if(ETHWAN_PPPOE == i)
					Entry.cmode = CHANNEL_MODE_PPPOE;
				else if(ETHWAN_BRIDGE == i)
					Entry.cmode = CHANNEL_MODE_BRIDGE;
				else if(ETHWAN_ROUTE == i)
					Entry.cmode = CHANNEL_MODE_IPOE;
				else
					return smpErrorBadValue;

				mib_chain_update(MIB_ATM_VC_TBL, (char *)&Entry, selected);
						
				return (smpErrorNone);	

			case ETHWANNAPT:
				i =  asnNumber (asnValue (asn), asnLength (asn));

				if(SNMP_ENABLE == i)
					Entry.napt= 1;
				else if(SNMP_DISABLE == i)
					Entry.napt= 0;
				else
					return smpErrorBadValue;

				mib_chain_update(MIB_ATM_VC_TBL, (char *)&Entry, selected);
				
				return (smpErrorNone);
				
#ifdef CONFIG_IGMPPROXY_MULTIWAN

			case ETHWANIGMPPRXY:
				i =  asnNumber (asnValue (asn), asnLength (asn));

				if(SNMP_ENABLE == i)
					Entry.enableIGMP = 1;
				else if(SNMP_DISABLE == i)
					Entry.enableIGMP = 0;
				else
					return smpErrorBadValue;

				mib_chain_update(MIB_ATM_VC_TBL, (char *)&Entry, selected);
				
				return (smpErrorNone);
#endif
#ifdef CONFIG_USER_IP_QOS				
			case ETHWANQOS:
				i =  asnNumber (asnValue (asn), asnLength (asn));

				if(SNMP_ENABLE == i)
					Entry.enableIpQos= 1;
				else if(SNMP_DISABLE == i)
					Entry.enableIpQos= 0;
				else
					return smpErrorBadValue;

				mib_chain_update(MIB_ATM_VC_TBL, (char *)&Entry, selected);
				
				return (smpErrorNone);	
#endif
			case ETHWANDFTROUT:
				i =  asnNumber (asnValue (asn), asnLength (asn));

				if(SNMP_ENABLE == i)
					Entry.dgw = 1;
				else if(SNMP_DISABLE == i)
					Entry.dgw = 0;
				else
					return smpErrorBadValue;

				mib_chain_update(MIB_ATM_VC_TBL, (char *)&Entry, selected);
				
				return (smpErrorNone);	

			case ETHWANIPTYPE:
				i =  asnNumber (asnValue (asn), asnLength (asn));

				if(SNMP_ENABLE == i)
					Entry.ipDhcp = 1;
				else if(SNMP_DISABLE == i)
					Entry.ipDhcp = 0;
				else
					return smpErrorBadValue;

				mib_chain_update(MIB_ATM_VC_TBL, (char *)&Entry, selected);
				
				return (smpErrorNone);	

			case ETHWANIPADDR:
			case ETHWANREMADDR:
			case ETHWANNETMASK:
				if(DHCP_DISABLED == Entry.ipDhcp)
				{
					k = asnLength (asn);		
					(void) asnContents (asn, strvalue, k);	
					strvalue[k]=NULL;

					if(ETHWANIPADDR == item)
						memcpy(Entry.ipAddr, strvalue, min(k, IP_ADDR_LEN));

					if(ETHWANREMADDR == item)
						memcpy(Entry.remoteIpAddr, strvalue, min(k, IP_ADDR_LEN));

					if(ETHWANNETMASK == item)
						memcpy(Entry.netMask, strvalue, min(k, IP_ADDR_LEN));
				
					mib_chain_update(MIB_ATM_VC_TBL, (char *)&Entry, selected);

					return smpErrorNone;
				}
				return smpErrorReadOnly;
				
			case ETHWANPPP_USERNAME:
				k = asnLength (asn);		
				(void) asnContents (asn, strvalue, k);	
				strvalue[k]=NULL;

				memcpy(Entry.pppUsername, strvalue, min(k,MAX_PPP_NAME_LEN+1));

				mib_chain_update(MIB_ATM_VC_TBL, (char *)&Entry, selected);
				return smpErrorNone;
				
			case ETHWANPPP_PASSWD:	
				k = asnLength (asn);		
				(void) asnContents (asn, strvalue, k);	
				strvalue[k]=NULL;

				memcpy(Entry.pppPassword, strvalue, min(k,MAX_NAME_LEN));

				mib_chain_update(MIB_ATM_VC_TBL, (char *)&Entry, selected);
				
				return smpErrorNone;

			case ETHWANPPP_CTYPE:
				i =  asnNumber (asnValue (asn), asnLength (asn));	

				if(i < 0 || i > PPP_CONN_ON_MANNUAL)
					return smpErrorBadValue;
				
				Entry.pppCtype = i;
				
				mib_chain_update(MIB_ATM_VC_TBL, (char *)&Entry, selected);
				return smpErrorNone;

			case ETHWANPPP_IDLETIME:	
				i =  asnNumber (asnValue (asn), asnLength (asn));	
				
				if(PPP_CONN_ON_DEMAND == Entry.pppCtype ) // 1 means connect on demand
				{
					Entry.pppIdleTime = i;

					mib_chain_update(MIB_ATM_VC_TBL, (char *)&Entry, selected);
					return smpErrorNone;
				}
				else
					return smpErrorNoSuch;
				
			default:
				return smpErrorNoSuch;
		}
	}
	return smpErrorNoSuch;
}


static	MixOpsType	ethwanOps = {

			ethwanRelease,
			ethwanCreate,
			ethwanDestroy,
			ethwanNext,
			ethwanGet,
			ethwanSet

			};


CVoidType		ethwanInit (void)
{
	(void) misExport ((MixNamePtrType) "\53\6\1\2\1\3",
		(MixLengthType)6, &ethwanOps, (MixCookieType) 0);
}

