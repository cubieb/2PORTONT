
#include	<stdio.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<time.h>

#include	"ctypes.h"
#include	"error.h"
#include	"local.h"
#include	"atm_vars.h"
#include	"mix.h"
#include	"mis.h"
#include	"asn.h"

#include "mib_tool.h"

#define ATMINTERFACEADMINADDRESS_LEN			41
#define ATMINTERFACEMYNEIGHBORIFNAME_LEN		256
#define ATMINTERFACESUBSCRADDRESS_LEN			41

struct atmInterfaceConfTable_mib {
	unsigned int atmInterfaceMaxVpcs;
	unsigned int atmInterfaceMaxVccs;
	unsigned int atmInterfaceConfVpcs;
	unsigned int atmInterfaceConfVccs;
	unsigned int atmInterfaceMaxActiveVpiBits;
	unsigned int atmInterfaceMaxActiveVciBits;
	unsigned int atmInterfaceIlmiVpi;
	unsigned int atmInterfaceIlmiVci;
	unsigned int atmInterfaceAddressType;
	char atmInterfaceAdminAddress[ATMINTERFACEADMINADDRESS_LEN];
	char atmInterfaceMyNeighborIpAddress[4];
	char atmInterfaceMyNeighborIfName[ATMINTERFACEMYNEIGHBORIFNAME_LEN];
	unsigned int atmInterfaceCurrentMaxVpiBits;
	unsigned int atmInterfaceCurrentMaxVciBits;
	char atmInterfaceSubscrAddress[ATMINTERFACESUBSCRADDRESS_LEN];
};

struct atmTrafficDescrParamTable_mib {
	unsigned int atmTrafficDescrParamIndex;
	unsigned int atmTrafficDescrType;
	unsigned int atmTrafficDescrParam1;
	unsigned int atmTrafficDescrParam2;
	unsigned int atmTrafficDescrParam3;
	unsigned int atmTrafficDescrParam4;
	unsigned int atmTrafficDescrParam5;
	unsigned int atmTrafficQoSClass;
	unsigned int atmTrafficDescrRowStatus;
	unsigned int atmServiceCategory;
	unsigned int atmTrafficFrameDiscard;
};

struct atmVclTable_mib {
	unsigned int atmVclVpi;
	unsigned int atmVclVci;
	unsigned int atmVclAdminStatus;
	unsigned int atmVclOperStatus;
	unsigned int atmVclLastChange;
	unsigned int atmVclReceiveTrafficDescrIndex;
	unsigned int atmVclTransmitTrafficDescrIndex;
	unsigned int atmVccAalType;
	unsigned int atmVccAal5CpcsTransmitSduSize;
	unsigned int atmVccAal5CpcsReceiveSduSize;
	unsigned int atmVccAal5EncapsType;
	unsigned int atmVclCrossConnectIdentifier;
	unsigned int atmVclRowStatus;
	unsigned int atmVclCastType;
	unsigned int atmVclConnKind;
};

struct aal5VccTable_mib {
	unsigned int aal5VccVpi;
	unsigned int aal5VccVci;
	unsigned int aal5VccCrcErrors;
	unsigned int aal5VccSarTimeOuts;
	unsigned int aal5VccOverSizedSDUs;
};


struct atm_mib {
	unsigned int 	if_idx;
	struct atmInterfaceConfTable_mib	atmInterfaceConfTable;

	unsigned int atmTrafficDescrParamTable_num;
	struct atmTrafficDescrParamTable_mib *atmTrafficDescrParamTable_ptr;

	unsigned int atmVclTable_num;
	struct atmVclTable_mib *atmVclTable_ptr;

	unsigned int aal5VccTable_num;
	struct aal5VccTable_mib *aal5VccTable_ptr;

	struct atm_mib   *if_next;
 };


#define MAX_ATMTRAFFICDESCRPARAMTABLE_NUM	16
#define MAX_ATM_VCL_TBL_NUM					16

static struct atm_mib *atm_mib_list = NULL;
static struct mib_oid_tbl atm_mib_oid_tbl;

static unsigned int atm_if_num;
static unsigned int atmTrafficDescrParamTable_num;
static unsigned int atmVclTable_num;
static unsigned int aal5VccTable_num;

static unsigned int atmTrafficDescrParamIndex_Tbl[MAX_ATMTRAFFICDESCRPARAMTABLE_NUM];
static unsigned int atmVclTable_ifIndex[MAX_ATM_VCL_TBL_NUM];
static unsigned int atmVclVpi_Tbl[MAX_ATM_VCL_TBL_NUM];
static unsigned int atmVclVci_Tbl[MAX_ATM_VCL_TBL_NUM];
static unsigned int aal5VccTable_ifIndex[MAX_ATM_VCL_TBL_NUM];
static unsigned int aal5VccTable_Tbl[MAX_ATM_VCL_TBL_NUM];
static unsigned int aal5VccTable_Tbl[MAX_ATM_VCL_TBL_NUM];

static long atm_last_scan = 0;

static unsigned int atmTrafficDescrParamIndexNext = 0;

void	Atm_Mib_Get(unsigned int if_idx)
{
	struct atm_mib *nnew;

	/*
	 * free old list: 
	 */
	while (atm_mib_list) {
	    struct atm_mib   *old = atm_mib_list;
	    atm_mib_list = atm_mib_list->if_next;
	    if(old->atmTrafficDescrParamTable_ptr)
	    	free(old->atmTrafficDescrParamTable_ptr);
	    if(old->atmVclTable_ptr)
	    	free(old->atmVclTable_ptr);
	    if(old->aal5VccTable_ptr)
	    	free(old->aal5VccTable_ptr);
	    free(old);
	}

	nnew = (struct atm_mib *) calloc(1, sizeof(struct atm_mib));
	if (nnew == NULL)
	{
		atm_mib_list = NULL;
		return;              /* alloc error */
	}

	atm_mib_list = nnew;
	nnew->if_next = NULL;

	nnew->if_idx = if_idx;

	nnew->atmInterfaceConfTable.atmInterfaceMaxVpcs = 4096;
	nnew->atmInterfaceConfTable.atmInterfaceMaxVccs = 65536;
	nnew->atmInterfaceConfTable.atmInterfaceConfVpcs = 4096;
	nnew->atmInterfaceConfTable.atmInterfaceConfVccs = 65536;
	nnew->atmInterfaceConfTable.atmInterfaceMaxActiveVpiBits = 12;
	nnew->atmInterfaceConfTable.atmInterfaceMaxActiveVciBits = 16;
	nnew->atmInterfaceConfTable.atmInterfaceIlmiVpi = 4095;
	nnew->atmInterfaceConfTable.atmInterfaceIlmiVci = 65535;
	nnew->atmInterfaceConfTable.atmInterfaceAddressType = 1;
	strcpy(nnew->atmInterfaceConfTable.atmInterfaceAdminAddress, "ATM IF Admin Addr");
	nnew->atmInterfaceConfTable.atmInterfaceMyNeighborIpAddress[0] = 0xaa;
	nnew->atmInterfaceConfTable.atmInterfaceMyNeighborIpAddress[1] = 0xbb;
	nnew->atmInterfaceConfTable.atmInterfaceMyNeighborIpAddress[2] = 0xcc;
	nnew->atmInterfaceConfTable.atmInterfaceMyNeighborIpAddress[3] = 0xdd;
	strcpy(nnew->atmInterfaceConfTable.atmInterfaceMyNeighborIfName, "ATM IF Neighbor Name");
	nnew->atmInterfaceConfTable.atmInterfaceCurrentMaxVpiBits = 12;
	nnew->atmInterfaceConfTable.atmInterfaceCurrentMaxVciBits = 16;
	strcpy(nnew->atmInterfaceConfTable.atmInterfaceSubscrAddress, "ATM IF Subscr Addr");

	nnew->atmTrafficDescrParamTable_num = 1;	
	nnew->atmTrafficDescrParamTable_ptr = (struct atmTrafficDescrParamTable_mib *) calloc(1, sizeof(struct atmTrafficDescrParamTable_mib));
	if (nnew->atmTrafficDescrParamTable_ptr == NULL)
	{
		nnew->atmTrafficDescrParamTable_num = 0;	
	}
	else
	{

		nnew->atmTrafficDescrParamTable_ptr->atmTrafficDescrParamIndex = 10;
		nnew->atmTrafficDescrParamTable_ptr->atmTrafficDescrType = 7;
		nnew->atmTrafficDescrParamTable_ptr->atmTrafficDescrParam1 = 99;
		nnew->atmTrafficDescrParamTable_ptr->atmTrafficDescrParam2 = 999;
		nnew->atmTrafficDescrParamTable_ptr->atmTrafficDescrParam3 = 9999;
		nnew->atmTrafficDescrParamTable_ptr->atmTrafficDescrParam4 = 99999;
		nnew->atmTrafficDescrParamTable_ptr->atmTrafficDescrParam5 = 999999;
		nnew->atmTrafficDescrParamTable_ptr->atmTrafficQoSClass = 255;
		nnew->atmTrafficDescrParamTable_ptr->atmTrafficDescrRowStatus = 1; 
		nnew->atmTrafficDescrParamTable_ptr->atmServiceCategory = 2;
		nnew->atmTrafficDescrParamTable_ptr->atmTrafficFrameDiscard = 1;
		
	}

	nnew->atmVclTable_num = 2;	
	nnew->atmVclTable_ptr = (struct atmVclTable_mib *) calloc(2, sizeof(struct atmVclTable_mib));
	if (nnew->atmVclTable_ptr == NULL)
	{
		nnew->atmVclTable_num = 0;	
	}
	else
	{
		struct atmVclTable_mib *atmVclTable_ptr = nnew->atmVclTable_ptr;
		
		atmVclTable_ptr->atmVclVpi = 1;
		atmVclTable_ptr->atmVclVci = 1;
		atmVclTable_ptr->atmVclAdminStatus = 1;
		atmVclTable_ptr->atmVclOperStatus = 1;
		atmVclTable_ptr->atmVclLastChange = 888888;
		atmVclTable_ptr->atmVclReceiveTrafficDescrIndex = 10;
		atmVclTable_ptr->atmVclTransmitTrafficDescrIndex = 10;
		atmVclTable_ptr->atmVccAalType = 3;
		atmVclTable_ptr->atmVccAal5CpcsTransmitSduSize = 65535;
		atmVclTable_ptr->atmVccAal5CpcsReceiveSduSize = 65535;
		atmVclTable_ptr->atmVccAal5EncapsType = 1;
		atmVclTable_ptr->atmVclCrossConnectIdentifier = 2147483647;
		atmVclTable_ptr->atmVclRowStatus = 1;
		atmVclTable_ptr->atmVclCastType = 1;
		atmVclTable_ptr->atmVclConnKind = 1;		

		atmVclTable_ptr++;

		atmVclTable_ptr->atmVclVpi = 2;
		atmVclTable_ptr->atmVclVci = 2;
		atmVclTable_ptr->atmVclAdminStatus = 1;
		atmVclTable_ptr->atmVclOperStatus = 1;
		atmVclTable_ptr->atmVclLastChange = 888888;
		atmVclTable_ptr->atmVclReceiveTrafficDescrIndex = 10;
		atmVclTable_ptr->atmVclTransmitTrafficDescrIndex = 10;
		atmVclTable_ptr->atmVccAalType = 3;
		atmVclTable_ptr->atmVccAal5CpcsTransmitSduSize = 65535;
		atmVclTable_ptr->atmVccAal5CpcsReceiveSduSize = 65535;
		atmVclTable_ptr->atmVccAal5EncapsType = 1;
		atmVclTable_ptr->atmVclCrossConnectIdentifier = 2147483647;
		atmVclTable_ptr->atmVclRowStatus = 1;
		atmVclTable_ptr->atmVclCastType = 1;
		atmVclTable_ptr->atmVclConnKind = 1;		

	}

	nnew->aal5VccTable_num = 3;	
	nnew->aal5VccTable_ptr = (struct aal5VccTable_mib *) calloc(3, sizeof(struct aal5VccTable_mib));
	if (nnew->aal5VccTable_ptr == NULL)
	{
		nnew->aal5VccTable_num = 0;	
	}
	else
	{
		struct aal5VccTable_mib *aal5VccTable_ptr = nnew->aal5VccTable_ptr;

		aal5VccTable_ptr->aal5VccVpi = 10;
		aal5VccTable_ptr->aal5VccVci = 10;
		aal5VccTable_ptr->aal5VccCrcErrors = 10000;
		aal5VccTable_ptr->aal5VccSarTimeOuts = 20000;
		aal5VccTable_ptr->aal5VccOverSizedSDUs = 30000;

		aal5VccTable_ptr++;

		aal5VccTable_ptr->aal5VccVpi = 20;
		aal5VccTable_ptr->aal5VccVci = 20;
		aal5VccTable_ptr->aal5VccCrcErrors = 100000;
		aal5VccTable_ptr->aal5VccSarTimeOuts = 200000;
		aal5VccTable_ptr->aal5VccOverSizedSDUs = 300000;

		aal5VccTable_ptr++;

		aal5VccTable_ptr->aal5VccVpi = 30;
		aal5VccTable_ptr->aal5VccVci = 30;
		aal5VccTable_ptr->aal5VccCrcErrors = 1000000;
		aal5VccTable_ptr->aal5VccSarTimeOuts = 2000000;
		aal5VccTable_ptr->aal5VccOverSizedSDUs = 3000000;

	}
	
	// prepare other local variable
	nnew  = atm_mib_list;
	atm_if_num = 0;
	atmTrafficDescrParamTable_num = 0;
	atmVclTable_num = 0;
	aal5VccTable_num = 0;
	while(nnew)
	{
		atm_if_num++;

		if(nnew->atmTrafficDescrParamTable_num > 0)
		{
			unsigned int idx;
			struct atmTrafficDescrParamTable_mib *atmTrafficDescrParamTable_ptr = nnew->atmTrafficDescrParamTable_ptr;
			for(idx=0;idx<nnew->atmTrafficDescrParamTable_num;idx++)
			{
				atmTrafficDescrParamIndex_Tbl[atmTrafficDescrParamTable_num] = atmTrafficDescrParamTable_ptr->atmTrafficDescrParamIndex;
				atmTrafficDescrParamTable_ptr++;
				atmTrafficDescrParamTable_num++;
			}
		}

		if(nnew->atmVclTable_num > 0)
		{
			unsigned int idx;
			struct atmVclTable_mib *atmVclTable_ptr = nnew->atmVclTable_ptr;
			for(idx=0;idx<nnew->atmVclTable_num;idx++)
			{
				atmVclTable_ifIndex[atmVclTable_num] = nnew->if_idx;
				atmVclVpi_Tbl[atmVclTable_num] = atmVclTable_ptr->atmVclVpi;
				atmVclVci_Tbl[atmVclTable_num] = atmVclTable_ptr->atmVclVci;
				atmVclTable_ptr++;
				atmVclTable_num++;
			}
		}

		if(nnew->aal5VccTable_num > 0)
		{
			unsigned int idx;
			struct aal5VccTable_mib *aal5VccTable_ptr = nnew->aal5VccTable_ptr;
			for(idx=0;idx<nnew->aal5VccTable_num;idx++)
			{
				aal5VccTable_ifIndex[aal5VccTable_num] = nnew->if_idx;
				aal5VccTable_Tbl[aal5VccTable_num] = aal5VccTable_ptr->aal5VccVpi;
				aal5VccTable_Tbl[aal5VccTable_num] = aal5VccTable_ptr->aal5VccVci;
				aal5VccTable_ptr++;
				aal5VccTable_num++;
			}
		}
		
		nnew = nnew->if_next;
	}
}

void Atm_Mib_Init(void)
{
	struct timeval now;
	gettimeofday(&now, 0);

	if((now.tv_sec - atm_last_scan) > 10)	//  10 sec
	{
		unsigned int if_idx;

		atm_last_scan = now.tv_sec;

		if_idx = Adsl_Interface_Idx();

		if(if_idx != 0)
		{
			unsigned int oid_tbl_size;
			unsigned int idx;
			struct mib_oid *oid_ptr;

			Atm_Mib_Get(if_idx);

			// create oid table
			free_mib_tbl(&atm_mib_oid_tbl);

			oid_tbl_size = atm_if_num * ATMINTERFACECONFTABLENUM + 
				atmTrafficDescrParamTable_num * ATMTRAFFICDESCRPARAMTABLENUM + 
				atmVclTable_num * ATMVCLTABLENUM + 
				aal5VccTable_num * AAL5VCCTABLENUM + 
				ATMTRAFFICDESCRPARAMINDEXNEXTNUM;
			create_mib_tbl(&atm_mib_oid_tbl,  oid_tbl_size, 6);

//			printf("oid_tbl_size  %d %d %d %d \n",oid_tbl_size, atmTrafficDescrParamTable_num, atmVclTable_num, aal5VccTable_num);

			oid_ptr = atm_mib_oid_tbl.oid;

			// atmInterfaceConfTable 2.1 (1~15)
			for(idx=0;idx<atm_if_num;idx++)
			{
				unsigned int oid_idx;
				for(oid_idx=ATMINTERFACEMAXVPCS;oid_idx<=ATMINTERFACESUBSCRADDRESS;oid_idx++)
				{
					oid_ptr->length = 4;
					oid_ptr->name[0] = ATMINTERFACECONFTABLE;
					oid_ptr->name[1] = 1;
					oid_ptr->name[2] = oid_idx;
					oid_ptr->name[3] = if_idx;
					oid_ptr++;			
				}
			}

			// atmTrafficDescrParamTable 5.1 (2~11)
			{
				unsigned int oid_idx;
				for(oid_idx=ATMTRAFFICDESCRTYPE;oid_idx<=ATMTRAFFICFRAMEDISCARD;oid_idx++)
				{
					for(idx=0;idx<atmTrafficDescrParamTable_num;idx++)
					{
						oid_ptr->length = 4;
						oid_ptr->name[0] = ATMTRAFFICDESCRPARAMTABLE;
						oid_ptr->name[1] = 1;
						oid_ptr->name[2] = oid_idx;
						oid_ptr->name[3] = atmTrafficDescrParamIndex_Tbl[idx];
						oid_ptr++;			
					}				
				}
			}

			// atmVclTable 7.1 (3~15)
			{
				unsigned int oid_idx;
				for(oid_idx=ATMVCLADMINSTATUS;oid_idx<=ATMVCLCONNKIND;oid_idx++)
				{
					for(idx=0;idx<atmVclTable_num;idx++)
					{
						oid_ptr->length = 6;
						oid_ptr->name[0] = ATMVCLTABLE;
						oid_ptr->name[1] = 1;
						oid_ptr->name[2] = oid_idx;
						oid_ptr->name[3] = atmVclTable_ifIndex[idx];
						oid_ptr->name[4] = atmVclVpi_Tbl[idx];
						oid_ptr->name[5] = atmVclVci_Tbl[idx];
						oid_ptr++;			
					}				
				}
			}

			// aal5VccTable 12.1 (3~5)
			{
				unsigned int oid_idx;
				for(oid_idx=AAL5VCCCRCERRORS;oid_idx<=AAL5VCCOVERSIZEDSDUS;oid_idx++)
				{
					for(idx=0;idx<aal5VccTable_num;idx++)
					{
						oid_ptr->length = 6;
						oid_ptr->name[0] = AAL5VCCTABLE;
						oid_ptr->name[1] = 1;
						oid_ptr->name[2] = oid_idx;
						oid_ptr->name[3] = aal5VccTable_ifIndex[idx];
						oid_ptr->name[4] = aal5VccTable_Tbl[idx];
						oid_ptr->name[5] = aal5VccTable_Tbl[idx];
						oid_ptr++;			
					}				
				}
			}

			// atmTrafficDescrParamIndexNext 13.0
			oid_ptr->length = 2;
			oid_ptr->name[0] = ATMTRAFFICDESCRPARAMINDEXNEXT;
			oid_ptr->name[1] = 0;

//			print_mib_tbl(&atm_mib_oid_tbl);

		}	
	}
}

static	AsnIdType	atmRetrieveMibValue (unsigned int mibIdx)
{
	struct mib_oid * oid_ptr;

	if(atm_mib_list == NULL)
	{
		return ((AsnIdType) 0);
	}

	oid_ptr = atm_mib_oid_tbl.oid;
	oid_ptr += mibIdx;

	if(oid_ptr->name[0] == ATMTRAFFICDESCRPARAMINDEXNEXT)
	{

		/*
			This object contains an appropriate value to
       		be used for atmTrafficDescrParamIndex when
       		creating entries in the
       		atmTrafficDescrParamTable.
       		The value 0 indicates that no unassigned
       		entries are available. To obtain the
       		atmTrafficDescrParamIndex value for a new
      			entry, the manager issues a management
       		protocol retrieval operation to obtain the
       		current value of this object.  After each
       		retrieval, the agent should modify the value
       		to the next unassigned index.
       		After a manager retrieves a value the agent will
       		determine through its local policy when this index
       		value will be made available for reuse.
		*/
	
		atmTrafficDescrParamIndexNext++;
		return asnUnsl (asnClassUniversal, (AsnTagType) 2, atmTrafficDescrParamIndexNext);
	}
	else
	if(oid_ptr->name[0] == ATMINTERFACECONFTABLE)
	{
		// search for according to ifIndex
		struct atm_mib *nnew = atm_mib_list;
		while(nnew)
		{
			if(oid_ptr->name[3] == nnew->if_idx)
				break;
			
			nnew = nnew->if_next;
		}

		if(nnew)	// found
		{
			switch(oid_ptr->name[2])
			{
				case ATMINTERFACEMAXVPCS:
					return asnUnsl (asnClassUniversal, (AsnTagType) 2, nnew->atmInterfaceConfTable.atmInterfaceMaxVpcs);
				case ATMINTERFACEMAXVCCS:
					return asnUnsl (asnClassUniversal, (AsnTagType) 2, nnew->atmInterfaceConfTable.atmInterfaceMaxVccs);
				case ATMINTERFACECONFVPCS:
					return asnUnsl (asnClassUniversal, (AsnTagType) 2, nnew->atmInterfaceConfTable.atmInterfaceConfVpcs);
				case ATMINTERFACECONFVCCS:
					return asnUnsl (asnClassUniversal, (AsnTagType) 2, nnew->atmInterfaceConfTable.atmInterfaceConfVccs);
				case ATMINTERFACEMAXACTIVEVPIBITS:
					return asnUnsl (asnClassUniversal, (AsnTagType) 2, nnew->atmInterfaceConfTable.atmInterfaceMaxActiveVpiBits);
				case ATMINTERFACEMAXACTIVEVCIBITS:
					return asnUnsl (asnClassUniversal, (AsnTagType) 2, nnew->atmInterfaceConfTable.atmInterfaceMaxActiveVciBits);
				case ATMINTERFACEILMIVPI:
					return asnUnsl (asnClassUniversal, (AsnTagType) 2, nnew->atmInterfaceConfTable.atmInterfaceIlmiVpi);
				case ATMINTERFACEILMIVCI:
					return asnUnsl (asnClassUniversal, (AsnTagType) 2, nnew->atmInterfaceConfTable.atmInterfaceIlmiVci);
				case ATMINTERFACEADDRESSTYPE:
					return asnUnsl (asnClassUniversal, (AsnTagType) 2, nnew->atmInterfaceConfTable.atmInterfaceAddressType);
				case ATMINTERFACEADMINADDRESS:
					return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
						(CBytePtrType) (nnew->atmInterfaceConfTable.atmInterfaceAdminAddress), 
						(AsnLengthType) strlen ((char *) (nnew->atmInterfaceConfTable.atmInterfaceAdminAddress)));
				case ATMINTERFACEMYNEIGHBORIPADDRESS:
					return asnOctetString (asnClassApplication, (AsnTagType) 0, 
						(CBytePtrType) (nnew->atmInterfaceConfTable.atmInterfaceMyNeighborIpAddress), 
						(AsnLengthType) 4);
				case ATMINTERFACEMYNEIGHBORIFNAME:
					return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
						(CBytePtrType) (nnew->atmInterfaceConfTable.atmInterfaceMyNeighborIfName), 
						(AsnLengthType) strlen ((char *) (nnew->atmInterfaceConfTable.atmInterfaceMyNeighborIfName)));
				case ATMINTERFACECURRENTMAXVPIBITS:
					return asnUnsl (asnClassUniversal, (AsnTagType) 2, nnew->atmInterfaceConfTable.atmInterfaceCurrentMaxVpiBits);
				case ATMINTERFACECURRENTMAXVCIBITS:
					return asnUnsl (asnClassUniversal, (AsnTagType) 2, nnew->atmInterfaceConfTable.atmInterfaceCurrentMaxVciBits);
				case ATMINTERFACESUBSCRADDRESS:
					return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
						(CBytePtrType) (nnew->atmInterfaceConfTable.atmInterfaceSubscrAddress), 
						(AsnLengthType) strlen ((char *) (nnew->atmInterfaceConfTable.atmInterfaceSubscrAddress)));
				default:
					return ((AsnIdType) 0);			
			}
		}
	}
	else
	if(oid_ptr->name[0] == ATMTRAFFICDESCRPARAMTABLE)
	{
		// search for according to atmTrafficDescrParamIndex
		int  found = 0;
		struct atmTrafficDescrParamTable_mib *atmTrafficDescrParamTable_ptr = NULL;
		struct atm_mib *nnew = atm_mib_list;
		while(nnew && !found)
		{
			if(nnew->atmTrafficDescrParamTable_num > 0)
			{
				unsigned int idx;
				atmTrafficDescrParamTable_ptr = nnew->atmTrafficDescrParamTable_ptr;
				for(idx=0;idx<nnew->atmTrafficDescrParamTable_num;idx++)
				{
					if(oid_ptr->name[3] == nnew->atmTrafficDescrParamTable_ptr->atmTrafficDescrParamIndex)
					{
						found  = 1;
						break;
					}
					atmTrafficDescrParamTable_ptr++;
				}
			}
			
			nnew = nnew->if_next;
		}

		if(found)
		{
			switch(oid_ptr->name[2])
			{
				case ATMTRAFFICDESCRTYPE:
					{
						MixNameType local_oid[9];
						memcpy(local_oid, "\53\6\1\2\1\45\1\1", 8);
						local_oid[8] = (MixNameType) atmTrafficDescrParamTable_ptr->atmTrafficDescrType;
						return asnObjectId (asnClassUniversal, (AsnTagType) 6, local_oid, 9);
					}
					break;					
				case ATMTRAFFICDESCRPARAM1:
					return asnUnsl (asnClassUniversal, (AsnTagType) 2, atmTrafficDescrParamTable_ptr->atmTrafficDescrParam1);
				case ATMTRAFFICDESCRPARAM2:
					return asnUnsl (asnClassUniversal, (AsnTagType) 2, atmTrafficDescrParamTable_ptr->atmTrafficDescrParam2);
				case ATMTRAFFICDESCRPARAM3:
					return asnUnsl (asnClassUniversal, (AsnTagType) 2, atmTrafficDescrParamTable_ptr->atmTrafficDescrParam3);
				case ATMTRAFFICDESCRPARAM4:
					return asnUnsl (asnClassUniversal, (AsnTagType) 2, atmTrafficDescrParamTable_ptr->atmTrafficDescrParam4);
				case ATMTRAFFICDESCRPARAM5:
					return asnUnsl (asnClassUniversal, (AsnTagType) 2, atmTrafficDescrParamTable_ptr->atmTrafficDescrParam5);
				case ATMTRAFFICQOSCLASS:
					return asnUnsl (asnClassUniversal, (AsnTagType) 2, atmTrafficDescrParamTable_ptr->atmTrafficQoSClass);
				case ATMTRAFFICDESCRROWSTATUS:
					return asnUnsl (asnClassUniversal, (AsnTagType) 2, atmTrafficDescrParamTable_ptr->atmTrafficDescrRowStatus);
				case ATMSERVICECATEGORY:
					return asnUnsl (asnClassUniversal, (AsnTagType) 2, atmTrafficDescrParamTable_ptr->atmServiceCategory);
				case ATMTRAFFICFRAMEDISCARD:
					return asnUnsl (asnClassUniversal, (AsnTagType) 2, atmTrafficDescrParamTable_ptr->atmTrafficFrameDiscard);
				default:
					return ((AsnIdType) 0);			
			}
		}
		
	}
	else
	if(oid_ptr->name[0] == ATMVCLTABLE)
	{
		// search for according to ifIndex, atmVclVpi, atmVclVci
		int  found = 0;
		struct atmVclTable_mib *atmVclTable_ptr;
		struct atm_mib *nnew = atm_mib_list;
		
		// Mason Yu test
		printf("atmRetrieveMibValue: ATMVCLTABLE\n");
		
		while(nnew && !found)
		{
			if(nnew->atmVclTable_num > 0)
			{
				if(oid_ptr->name[3] == nnew->if_idx)
				{
					unsigned int idx;
					atmVclTable_ptr = nnew->atmVclTable_ptr;
					for(idx=0;idx<nnew->atmVclTable_num;idx++)
					{
						if((oid_ptr->name[4] == atmVclTable_ptr->atmVclVpi) && (oid_ptr->name[5] == atmVclTable_ptr->atmVclVci))
						{
							found  = 1;
							break;
						}
						atmVclTable_ptr++;
					}
				}
			}
			nnew = nnew->if_next;
		}

		if(found)
		{
			switch(oid_ptr->name[2])
			{
				case ATMVCLADMINSTATUS:
					return asnUnsl (asnClassUniversal, (AsnTagType) 2, atmVclTable_ptr->atmVclAdminStatus);
				case ATMVCLOPERSTATUS:
					return asnUnsl (asnClassUniversal, (AsnTagType) 2, atmVclTable_ptr->atmVclOperStatus);
				case ATMVCLLASTCHANGE:
					return asnUnsl (asnClassApplication, (AsnTagType) 3, atmVclTable_ptr->atmVclLastChange);
				case ATMVCLRECEIVETRAFFICDESCRINDEX:
					return asnUnsl (asnClassUniversal, (AsnTagType) 2, atmVclTable_ptr->atmVclReceiveTrafficDescrIndex);
				case ATMVCLTRANSMITTRAFFICDESCRINDEX:
					return asnUnsl (asnClassUniversal, (AsnTagType) 2, atmVclTable_ptr->atmVclTransmitTrafficDescrIndex);
				case ATMVCCAALTYPE:
					return asnUnsl (asnClassUniversal, (AsnTagType) 2, atmVclTable_ptr->atmVccAalType);
				case ATMVCCAAL5CPCSTRANSMITSDUSIZE:
					return asnUnsl (asnClassUniversal, (AsnTagType) 2, atmVclTable_ptr->atmVccAal5CpcsTransmitSduSize);
				case ATMVCCAAL5CPCSRECEIVESDUSIZE:
					return asnUnsl (asnClassUniversal, (AsnTagType) 2, atmVclTable_ptr->atmVccAal5CpcsReceiveSduSize);
				case ATMVCCAAL5ENCAPSTYPE:
					return asnUnsl (asnClassUniversal, (AsnTagType) 2, atmVclTable_ptr->atmVccAal5EncapsType);
				case ATMVCLCROSSCONNECTIDENTIFIER:
					return asnUnsl (asnClassUniversal, (AsnTagType) 2, atmVclTable_ptr->atmVclCrossConnectIdentifier);
				case ATMVCLROWSTATUS:
					return asnUnsl (asnClassUniversal, (AsnTagType) 2, atmVclTable_ptr->atmVclRowStatus);
				case ATMVCLCASTTYPE:
					return asnUnsl (asnClassUniversal, (AsnTagType) 2, atmVclTable_ptr->atmVclCastType);
				case ATMVCLCONNKIND:
					return asnUnsl (asnClassUniversal, (AsnTagType) 2, atmVclTable_ptr->atmVclConnKind);
				default:
					return ((AsnIdType) 0);			
			}
		}
	}
	else
	if(oid_ptr->name[0] == AAL5VCCTABLE)
	{
		// search for according to ifIndex, aal5VccVpi, aal5VccVci
		int  found = 0;
		struct aal5VccTable_mib *aal5VccTable_ptr;
		struct atm_mib *nnew = atm_mib_list;

		while(nnew && !found)
		{
			if(nnew->atmVclTable_num > 0)
			{
				if(oid_ptr->name[3] == nnew->if_idx)
				{
					unsigned int idx;
					aal5VccTable_ptr = nnew->aal5VccTable_ptr;
					for(idx=0;idx<nnew->aal5VccTable_num;idx++)
					{
						if((oid_ptr->name[4] == aal5VccTable_ptr->aal5VccVpi) && (oid_ptr->name[5] == aal5VccTable_ptr->aal5VccVci))
						{
							found  = 1;
							break;
						}
						aal5VccTable_ptr++;
					}
				}
			}
			nnew = nnew->if_next;
		}

		if(found)
		{
			switch(oid_ptr->name[2])
			{
				case AAL5VCCCRCERRORS:
					return asnUnsl (asnClassApplication, (AsnTagType) 1, aal5VccTable_ptr->aal5VccCrcErrors);
				case AAL5VCCSARTIMEOUTS:
					return asnUnsl (asnClassApplication, (AsnTagType) 1, aal5VccTable_ptr->aal5VccSarTimeOuts);
				case AAL5VCCOVERSIZEDSDUS:
					return asnUnsl (asnClassApplication, (AsnTagType) 1, aal5VccTable_ptr->aal5VccOverSizedSDUs);
				default:
					return ((AsnIdType) 0);			
			}
		}
	}

	return ((AsnIdType) 0);
}

static	MixStatusType	atmRelease (MixCookieType cookie)
{
	cookie = cookie;
	return (smpErrorGeneric);
}

static	MixStatusType atmCreate (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen, AsnIdType asn)
{
	//printf("ifaceCreate ()\n");    
	cookie = cookie;
	name = name;
	namelen = namelen;
	asn = asn;
	return (smpErrorGeneric);
}

static	MixStatusType	atmDestroy (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen)
{
	//printf("ifaceDestroy ()\n");    
	cookie = cookie;
	name = name;
	namelen = namelen;
	return (smpErrorGeneric);
}

static	AsnIdType	atmGet (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen)
{
	unsigned int idx;

	Atm_Mib_Init();

	cookie = cookie;
	// Mason Yu test
	printf("atmGet \n");
	if(snmp_oid_get(&atm_mib_oid_tbl, name, namelen, &idx))
	{
		// Mason Yu test
		printf("atmGet: Retrive Value \n");
		return atmRetrieveMibValue(idx);
	}
	// Mason Yu test
	printf("atmGet: Can not get OID \n");
	return ((AsnIdType) 0);
}

static	MixStatusType	atmSet (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen, AsnIdType asn)
{
	unsigned int idx;

	cookie = cookie;
	name = name;
	namelen = namelen;
	asn = asn;

	return (smpErrorReadOnly);
}

static	AsnIdType	atmNext (MixCookieType cookie, MixNamePtrType name, MixLengthPtrType namelenp)
{
	unsigned int idx = 0;

#if 0	
	printf("atmNext %d\n", *namelenp);

	
	while(idx < *namelenp)
	{
		printf("atmNext %d %d\n",idx, *name++);		
		idx++;
	}
#endif

	Atm_Mib_Init();

	cookie = cookie;
	
	// Mason Yu test
	printf("atmNext \n");
	if(snmp_oid_getnext(&atm_mib_oid_tbl, name, *namelenp, &idx))
	{
		struct mib_oid * oid_ptr = atm_mib_oid_tbl.oid;
		oid_ptr += idx;

		memcpy((unsigned char *)name, oid_ptr->name, oid_ptr->length);
		*namelenp = oid_ptr->length;
		
		// Mason Yu test
		printf("atmNext: Retrive Value \n");
		return atmRetrieveMibValue(idx);
	}
	
	// Mason Yu test
	printf("atmNext: Can not get OID \n");
	
	return ((AsnIdType) 0);
}

static	MixOpsType	atmOps = {

			atmRelease,
			atmCreate,
			atmDestroy,
			atmNext,
			atmGet,
			atmSet

			};


CVoidType		atmInit (void)
{
	(void) misExport ((MixNamePtrType) "\53\6\1\2\1\45\1",
		(MixLengthType) 7, & atmOps, (MixCookieType) 0);
}



