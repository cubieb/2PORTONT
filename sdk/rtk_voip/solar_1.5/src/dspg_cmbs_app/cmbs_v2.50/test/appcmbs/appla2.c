/*!
*  \file       appla2.c
*	\brief		handles CAT-iq list access functioality
*	\Author		lebastard
*
*	@(#)	%filespec: appla2.c=3 %
*
*******************************************************************************
*	\par	History
*	\n==== History ============================================================
*	date			name		version	 action                                        
*      2 feb 2010    G.Lebastard                    get and add only necessary info for day/time 
*      4 feb 2010    G.Lebastard                    replace printf by PRINT to avoid debug printf 
*	----------------------------------------------------------------------------
*/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if ! defined ( WIN32 )
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/time.h>
#include <signal.h>
#endif

#include "cmbs_api.h"
#include "cmbs_int.h"
#include "cfr_ie.h"
#include "cfr_mssg.h"
#include "cmbs_str.h"

#include "appcmbs.h"

#include "appla2.h"

#define PRINT //printf

#define REMOVE_MSB(x) (x & 0x7F)
#define ADD_MSB(x) (x | 0x80)

u8 *         cmbs_util_LA_EntryLenght_EntryIdentifierAdd( u8 *p_entry, u16 u16_index, u16 u16_EntryID)
{
   u16 u16_entry_len = u16_index - 2;  // remove 2 bytes reserved for EntryID

if (u16_entry_len<0x81)
	u16_entry_len--;  // if entry length only need 7bits to be coded


if (u16_entry_len < 0x80)
{
	p_entry[3] = ADD_MSB(u16_entry_len);
	if (u16_EntryID < 0x80)
		{
	           p_entry[2] = ADD_MSB(u16_EntryID);
			   return (&p_entry[2]);
		}
	else
		{
	           p_entry[1] = REMOVE_MSB(u16_EntryID>>7);
	           p_entry[2] = ADD_MSB(u16_EntryID);
			   return (&p_entry[1]);
		}
		
}
else
{
	p_entry[2] = REMOVE_MSB(u16_entry_len>>7);
	p_entry[3] = ADD_MSB(u16_entry_len);
	if (u16_EntryID < 0x80)
		{
	           p_entry[1] = ADD_MSB(u16_EntryID);
			   return (&p_entry[1]);
		}
	else
		{
	           p_entry[0] =REMOVE_MSB(u16_EntryID>>7);
	           p_entry[1] = ADD_MSB(u16_EntryID);
			   return (&p_entry[0]);
		}
}

}


void cmbs_util_LA_EntryLenght_EntryIdentifierGet(u8 *pu8_Data, u16 *pu16_EntryIdentifier, u16 *pu16_EntryLength, u16 *pu16_Index) 
{

cmbs_util_LA_GetInfo_ShiftIndex(pu8_Data, pu16_EntryIdentifier, pu16_Index); // in case that info may be 0..16383
PRINT("pu16_EntryIdentifier=%x\n", *pu16_EntryIdentifier);
cmbs_util_LA_GetInfo_ShiftIndex(pu8_Data, pu16_EntryLength, pu16_Index); // in case that info may be 0..16383
PRINT("pu16_EntryLength=%x\n", *pu16_EntryLength);

}



E_CMBS_RC         cmbs_util_LA_Fld_NumberAdd( u8 * pv_buff, ST_UTIL_LA_FLD_NUMBER * pst_LA_Fld_Number, u16 *pindex )
{
   return cmbs_util_LA_Fld_Type3_Add( pv_buff, pst_LA_Fld_Number, CMBS_UTIL_LA_FLD_NUMBER, pindex  );
}


E_CMBS_RC         cmbs_util_LA_Fld_NumberGet( u8 * pv_buff, ST_UTIL_LA_FLD_NUMBER * pst_LA_Fld_Number )
{
   return cmbs_util_LA_Fld_Type3_Get( pv_buff, pst_LA_Fld_Number, CMBS_UTIL_LA_FLD_NUMBER );
}


E_CMBS_RC         cmbs_util_LA_Fld_NameAdd( u8 * pv_buff, ST_UTIL_LA_FLD_NAME * pst_LA_Fld_Name, u16 *pindex  )
{
   return cmbs_util_LA_Fld_Type3_Add( pv_buff, pst_LA_Fld_Name, CMBS_UTIL_LA_FLD_NAME, pindex  );
}


E_CMBS_RC         cmbs_util_LA_Fld_NameGet( u8 * pv_buff, ST_UTIL_LA_FLD_NAME * pst_LA_Fld_Name )
{
   return cmbs_util_LA_Fld_Type3_Get( pv_buff, pst_LA_Fld_Name, CMBS_UTIL_LA_FLD_NAME );
}


E_CMBS_RC         cmbs_util_LA_Fld_DateTimeAdd( u8 * pv_buff,
                                                   ST_UTIL_LA_FLD_DATETIME *
						   pst_LA_Fld_DateTime , u16 *pindex )
{
u16 index_len;

   pv_buff[(*pindex)++]    = pst_LA_Fld_DateTime->u8_FieldId;
   index_len = (*pindex)++;
   pv_buff[(*pindex)++] = ADD_MSB(pst_LA_Fld_DateTime->u8_Attribute);
   pv_buff[(*pindex)++] = ((pst_LA_Fld_DateTime->u8_Coding)<<6)+(pst_LA_Fld_DateTime->u8_interpretation);
if ((pst_LA_Fld_DateTime->u8_Coding==CMBS_DATE) || (pst_LA_Fld_DateTime->u8_Coding==CMBS_DATE_TIME))
{
   pv_buff[(*pindex)++] = pst_LA_Fld_DateTime->u8_Year;
   pv_buff[(*pindex)++] = pst_LA_Fld_DateTime->u8_Month;
   pv_buff[(*pindex)++] = pst_LA_Fld_DateTime->u8_Day;
}
if ((pst_LA_Fld_DateTime->u8_Coding==CMBS_TIME) || (pst_LA_Fld_DateTime->u8_Coding==CMBS_DATE_TIME))
{
   pv_buff[(*pindex)++] = pst_LA_Fld_DateTime->u8_Hours;
   pv_buff[(*pindex)++] = pst_LA_Fld_DateTime->u8_Mins;
   pv_buff[(*pindex)++] = pst_LA_Fld_DateTime->u8_Secs;
   pv_buff[(*pindex)++] = pst_LA_Fld_DateTime->u8_TimeZone;
}
pv_buff[index_len]     = ADD_MSB(*pindex-index_len-1);
   return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_util_LA_Fld_DateTimeGet( u8 * pv_buff,
                                                   ST_UTIL_LA_FLD_DATETIME * pst_LA_Fld_DateTime )
{
u8 local_shift=2;
      pst_LA_Fld_DateTime->u8_Attribute = REMOVE_MSB(pv_buff[local_shift++]);
      pst_LA_Fld_DateTime->u8_Coding= pv_buff[local_shift]>>6;
      pst_LA_Fld_DateTime->u8_interpretation= pv_buff[local_shift++]&0x3F;
if ((pst_LA_Fld_DateTime->u8_Coding==CMBS_DATE) || (pst_LA_Fld_DateTime->u8_Coding==CMBS_DATE_TIME))
{
      pst_LA_Fld_DateTime->u8_Year      = pv_buff[local_shift++];
      pst_LA_Fld_DateTime->u8_Month     = pv_buff[local_shift++];
      pst_LA_Fld_DateTime->u8_Day       = pv_buff[local_shift++];
}
if ((pst_LA_Fld_DateTime->u8_Coding==CMBS_TIME) || (pst_LA_Fld_DateTime->u8_Coding==CMBS_DATE_TIME))
{
      pst_LA_Fld_DateTime->u8_Hours     = pv_buff[local_shift++];
      pst_LA_Fld_DateTime->u8_Mins      = pv_buff[local_shift++];
      pst_LA_Fld_DateTime->u8_Secs      = pv_buff[local_shift++];
      pst_LA_Fld_DateTime->u8_TimeZone= pv_buff[local_shift++];
}

   	return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_util_LA_Fld_NewAdd( u8 * pv_buff, ST_UTIL_LA_FLD_NEW * pst_LA_Fld_New , u16 *pindex )
{
   return cmbs_util_LA_Fld_Type1_Add( pv_buff, pst_LA_Fld_New, CMBS_UTIL_LA_FLD_NEW, pindex  );
}


E_CMBS_RC         cmbs_util_LA_Fld_NewGet( u8 * pv_buff, ST_UTIL_LA_FLD_NEW * pst_LA_Fld_New )
{
   return cmbs_util_LA_Fld_Type1_Get( pv_buff, pst_LA_Fld_New, CMBS_UTIL_LA_FLD_NEW );
}


E_CMBS_RC         cmbs_util_LA_Fld_LineNameAdd( u8 * pv_buff, ST_UTIL_LA_FLD_LINE_NAME * pst_LA_Fld_LineName, u16 *pindex  )
{
   return cmbs_util_LA_Fld_Type3_Add( pv_buff, pst_LA_Fld_LineName, CMBS_UTIL_LA_FLD_LINE_NAME, pindex  );
}


E_CMBS_RC         cmbs_util_LA_Fld_LineNameGet( u8 * pv_buff, ST_UTIL_LA_FLD_LINE_NAME * pst_LA_Fld_LineName )
{
   return cmbs_util_LA_Fld_Type3_Get( pv_buff, pst_LA_Fld_LineName , CMBS_UTIL_LA_FLD_LINE_NAME);
}


E_CMBS_RC         cmbs_util_LA_Fld_LineIdAdd( u8 * pv_buff, ST_UTIL_LA_FLD_LINE_ID * pst_LA_Fld_LineId , u16 *pindex )
{
u16 u16_LengthLocalIndex;
     pv_buff[(*pindex)++]  = pst_LA_Fld_LineId->u8_FieldId;
	 u16_LengthLocalIndex = (*pindex)++;  //suppose only 1 byte
     pv_buff[(*pindex)++]  = ADD_MSB(pst_LA_Fld_LineId->u8_Attribute);
     pv_buff[(*pindex)++]  = pst_LA_Fld_LineId->Subtype;
     cmbs_util_add_u16_and_shift(pv_buff, pst_LA_Fld_LineId->u16_LineNumber, pindex);
     pv_buff[u16_LengthLocalIndex] = ADD_MSB(*pindex - u16_LengthLocalIndex - 1); // Add the length now
  // return cmbs_util_LA_Fld_Type2_Add( pv_buff, pst_LA_Fld_LineId, CMBS_UTIL_LA_FLD_LINE_ID, pindex  );
   	return CMBS_RC_OK;

}


E_CMBS_RC         cmbs_util_LA_Fld_LineIdGet( u8 * pv_buff, ST_UTIL_LA_FLD_LINE_ID * pst_LA_Fld_LineId )
{
u16  u16_ind=4;
pst_LA_Fld_LineId->u8_FieldId= pv_buff[0];
      pst_LA_Fld_LineId->u8_Attribute = REMOVE_MSB(pv_buff[2]);
      pst_LA_Fld_LineId->Subtype     = pv_buff[3];
cmbs_util_LA_GetInfo_ShiftIndex(pv_buff, &(pst_LA_Fld_LineId->u16_LineNumber), &u16_ind); // in case that info may be 0..16383
   	return CMBS_RC_OK;

  // return cmbs_util_LA_Fld_Type2_Get( pv_buff, pst_LA_Fld_LineId, CMBS_UTIL_LA_FLD_NR_OF_CALLS);
}


E_CMBS_RC         cmbs_util_LA_Fld_NrOfCallsAdd( u8 * pv_buff, ST_UTIL_LA_FLD_NR_OF_CALLS * pst_LA_Fld_NrOfCalls, u16 *pindex  )
{
   return cmbs_util_LA_Fld_Type2_Add( pv_buff, pst_LA_Fld_NrOfCalls, CMBS_UTIL_LA_FLD_NR_OF_CALLS, pindex  );
}


E_CMBS_RC         cmbs_util_LA_Fld_NrOfCallsGet( u8 * pv_buff, ST_UTIL_LA_FLD_NR_OF_CALLS * pst_LA_Fld_NrOfCalls )
{
   return cmbs_util_LA_Fld_Type2_Get( pv_buff, pst_LA_Fld_NrOfCalls, CMBS_UTIL_LA_FLD_NR_OF_CALLS );
}


E_CMBS_RC         cmbs_util_LA_Fld_CallTypeAdd( u8 * pv_buff, ST_UTIL_LA_FLD_CALL_TYPE * pst_LA_Fld_CallType , u16 *pindex )
{
   return cmbs_util_LA_Fld_Type1_Add( pv_buff, pst_LA_Fld_CallType, CMBS_UTIL_LA_FLD_CALL_TYPE, pindex  );
}


E_CMBS_RC         cmbs_util_LA_Fld_CallTypeGet( u8 * pv_buff, ST_UTIL_LA_FLD_CALL_TYPE * pst_LA_Fld_CallType )
{
   return cmbs_util_LA_Fld_Type1_Get( pv_buff, pst_LA_Fld_CallType, CMBS_UTIL_LA_FLD_CALL_TYPE );
}


E_CMBS_RC         cmbs_util_LA_Fld_FirstNameAdd( u8 * pv_buff, ST_UTIL_LA_FLD_FIRST_NAME * pst_LA_Fld_FirstName, u16 *pindex  )
{
   return cmbs_util_LA_Fld_Type3_Add( pv_buff, pst_LA_Fld_FirstName, CMBS_UTIL_LA_FLD_FIRST_NAME, pindex  );
}


E_CMBS_RC         cmbs_util_LA_Fld_FirstNameGet( u8 * pv_buff, ST_UTIL_LA_FLD_FIRST_NAME * pst_LA_Fld_FirstName )
{
   return cmbs_util_LA_Fld_Type3_Get( pv_buff, pst_LA_Fld_FirstName, CMBS_UTIL_LA_FLD_FIRST_NAME );
}


E_CMBS_RC         cmbs_util_LA_Fld_ContactNrAdd( u8 * pv_buff, ST_UTIL_LA_FLD_CONTACT_NR * pst_LA_Fld_ContactNr , u16 *pindex )
{
   return cmbs_util_LA_Fld_Type3_Add( pv_buff, pst_LA_Fld_ContactNr, CMBS_UTIL_LA_FLD_CONTACT_NR, pindex  );
}


E_CMBS_RC         cmbs_util_LA_Fld_ContactNrGet( u8 * pv_buff, ST_UTIL_LA_FLD_CONTACT_NR * pst_LA_Fld_ContactNr )
{
   return cmbs_util_LA_Fld_Type3_Get( pv_buff, pst_LA_Fld_ContactNr, CMBS_UTIL_LA_FLD_CONTACT_NR );
}


E_CMBS_RC         cmbs_util_LA_Fld_AssocMdyAdd( u8 * pv_buff, ST_UTIL_LA_FLD_ASSOC_MDY * pst_LA_Fld_AssocMdy , u16 *pindex )
{
   return cmbs_util_LA_Fld_Type2_Add( pv_buff, pst_LA_Fld_AssocMdy, CMBS_UTIL_LA_FLD_ASSOC_MDY, pindex  );
}


E_CMBS_RC         cmbs_util_LA_Fld_AssocMdyGet( u8 * pv_buff, ST_UTIL_LA_FLD_ASSOC_MDY * pst_LA_Fld_AssocMdy )
{
   return cmbs_util_LA_Fld_Type2_Get( pv_buff, pst_LA_Fld_AssocMdy, CMBS_UTIL_LA_FLD_ASSOC_MDY );
}


E_CMBS_RC               cmbs_util_LA_Fld_AttachedHandsetsAdd(  u8 * pv_buff, ST_UTIL_LA_FLD_ATTACHED_HANDSETS *st_LA_Fld_AttachedHandsets,  u16 *pindex)
{

u16 CorrespondingTable[256];
u16 ii;
u16 count=0;
u8 bit, byte;
u8 *pu8_lastbyte;
u8 *pu8_firstbyte;
u16 u16_IndexLen;
// Building Corresponding table;
#if 0
for(ii=0; ii<256; ii++)
{
	if (!(count%8))
	{
	count++;
	}
	CorrespondingTable[ii]=count++;
}


     pv_buff[(*pindex)++]  = st_LA_Fld_AttachedHandsets->u8_FieldId;
     u16_IndexLen = (*pindex)++ ; 
     pv_buff[(*pindex)++]  = ADD_MSB(st_LA_Fld_AttachedHandsets->u8_Attribute);
     pv_buff[(*pindex)++]  = (u8) (st_LA_Fld_AttachedHandsets->u16_DataLen);  // number of handsets

pu8_firstbyte=pv_buff+*pindex;

memset(pu8_firstbyte, 0, 37);

for(byte=0;byte<32;byte++)
{
	for (bit=0;  bit<8;  bit++)
	{
		if(((st_LA_Fld_AttachedHandsets->pu8_Data[byte])>>bit) & 1)   // check if bit==1
		{
                   *(pv_buff+*pindex+CorrespondingTable[byte*8+bit]/8) |= ((u8) (1<<((CorrespondingTable[byte*8+bit]%8)-1))); // set the bit in catiq 2 buff
                   pu8_lastbyte = pv_buff+*pindex+CorrespondingTable[byte*8+bit]/8;
		}
	}
}

(*pu8_lastbyte) |= 0x80;   // set last bit of last byte to signal that it is the last byte

	pv_buff[u16_IndexLen] = (u8) ADD_MSB(pu8_lastbyte-pu8_firstbyte + 2 + 1);  // +2 is for Attrib and Nb of handset + for lastbyte shift
	 
   (*pindex)+= (( u8) (pu8_lastbyte-pu8_firstbyte + 1));
   
   return CMBS_RC_OK;
   #endif

	pv_buff[(*pindex)++]  = st_LA_Fld_AttachedHandsets->u8_FieldId;
	pv_buff[(*pindex)++]  = ADD_MSB(3);
	pv_buff[(*pindex)++]  = ADD_MSB(st_LA_Fld_AttachedHandsets->u8_Attribute);
	pv_buff[(*pindex)++]  = ADD_MSB( st_LA_Fld_AttachedHandsets->u16_DataLen);  // number of handsets
	pv_buff[(*pindex)++]  = ADD_MSB( st_LA_Fld_AttachedHandsets->pu8_Data[0]);  // bitmask of attached pps

   return CMBS_RC_OK;

   
}

E_CMBS_RC               cmbs_util_LA_Fld_AttachedHandsetsGet(  u8 * pv_buff, ST_UTIL_LA_FLD_ATTACHED_HANDSETS *st_LA_Fld_AttachedHandsets)
{

   u16 u16_index=0;
   u16 u16_FieldLen;
   u8 bit;
   u8 byte=0;
   u8* pu8_data;
   u8 ii=0;
      st_LA_Fld_AttachedHandsets->u8_FieldId     = pv_buff[u16_index++];
      cmbs_util_LA_GetInfo_ShiftIndex(pv_buff, &u16_FieldLen, &u16_index); // get length & shift index
      st_LA_Fld_AttachedHandsets->u8_Attribute   = REMOVE_MSB(pv_buff[u16_index++]);
      st_LA_Fld_AttachedHandsets->u16_DataLen= pv_buff[u16_index++];

pu8_data = st_LA_Fld_AttachedHandsets->pu8_Data;

u16_FieldLen-=2;
do
{
pv_buff[byte]=0;
	for (bit=0;  bit<7;  bit++)
	{
		if(((pv_buff[byte+u16_index])>>bit) & 1)   // check if bit==1
		{
                   *(pu8_data+(ii/8)) |= 1<<(ii%8); // set the bit in catiq 2 buff
		}
	ii++;
	}
	byte++;
}
while(byte<u16_FieldLen);

	  

   return CMBS_RC_OK;
}


E_CMBS_RC               cmbs_util_LA_Fld_DialPrefixeAdd(  u8 * pv_buff, ST_UTIL_LA_FLD_DIALING_PREFIX *st_LA_Fld_DialingPrefixe,  u16 *pindex)
{
   return cmbs_util_LA_Fld_Type3_Add( pv_buff, st_LA_Fld_DialingPrefixe, CMBS_UTIL_LA_FLD_NUMBER, pindex  );
}


E_CMBS_RC               cmbs_util_LA_Fld_DialPrefixeGet(  u8 * pv_buff, ST_UTIL_LA_FLD_DIALING_PREFIX *st_LA_Fld_DialingPrefixe)
{

   return cmbs_util_LA_Fld_Type3_Get( pv_buff, st_LA_Fld_DialingPrefixe, CMBS_UTIL_LA_FLD_NUMBER );

}


E_CMBS_RC               cmbs_util_LA_Fld_FPvolumeAdd(  u8 * pv_buff, ST_UTIL_LA_FLD_FP_VOLUME *st_LA_Fld_FPvolume,  u16 *pindex )
{
   return cmbs_util_LA_Fld_Type2_Add( pv_buff, st_LA_Fld_FPvolume, CMBS_UTIL_LA_FLD_FP_VOLUME, pindex  );
}

E_CMBS_RC               cmbs_util_LA_Fld_FPvolumeGet(  u8 * pv_buff, ST_UTIL_LA_FLD_FP_VOLUME *st_LA_Fld_FPvolume )
{
   return cmbs_util_LA_Fld_Type2_Get( pv_buff, st_LA_Fld_FPvolume, CMBS_UTIL_LA_FLD_FP_VOLUME );
}


E_CMBS_RC               cmbs_util_LA_Fld_BlockedNumberAdd(  u8 * pv_buff, ST_UTIL_LA_FLD_BLOCKED_NUMBER *st_LA_Fld_BlockedNumber,  u16 *pindex )
{
   return cmbs_util_LA_Fld_Type3_Add( pv_buff, st_LA_Fld_BlockedNumber, CMBS_UTIL_LA_FLD_NUMBER, pindex  );
}

E_CMBS_RC               cmbs_util_LA_Fld_BlockedNumberGet(  u8 * pv_buff, ST_UTIL_LA_FLD_BLOCKED_NUMBER *st_LA_Fld_BlockedNumber)
{

   return cmbs_util_LA_Fld_Type3_Get( pv_buff, st_LA_Fld_BlockedNumber, CMBS_UTIL_LA_FLD_NUMBER );

}


E_CMBS_RC               cmbs_util_LA_Fld_MultiCallModeAdd(  u8 * pv_buff, ST_UTIL_LA_FLD_MULTI_CALL_MODE *st_LA_Fld_MultiCallMode,  u16 *pindex )
{
   return cmbs_util_LA_Fld_Type2_Add( pv_buff, st_LA_Fld_MultiCallMode, CMBS_UTIL_LA_FLD_MULTI_CALL_MODE, pindex  );
}

E_CMBS_RC               cmbs_util_LA_Fld_MultiCallModeGet(  u8 * pv_buff, ST_UTIL_LA_FLD_MULTI_CALL_MODE *st_LA_Fld_MultiCallMode )
{
   return cmbs_util_LA_Fld_Type2_Get( pv_buff, st_LA_Fld_MultiCallMode, CMBS_UTIL_LA_FLD_MULTI_CALL_MODE );
}


E_CMBS_RC               cmbs_util_LA_Fld_IntrusionCallAdd(  u8 * pv_buff, ST_UTIL_LA_FLD_INTRUSION_CALL *st_LA_Fld_IntrusionCall,  u16 *pindex)
{
   return cmbs_util_LA_Fld_Type2_Add( pv_buff, st_LA_Fld_IntrusionCall, CMBS_UTIL_LA_FLD_INTRUSION_CALL_MODE, pindex  );
}

E_CMBS_RC               cmbs_util_LA_Fld_IntrusionCallGet(  u8 * pv_buff, ST_UTIL_LA_FLD_INTRUSION_CALL *st_LA_Fld_IntrusionCall)
{
   return cmbs_util_LA_Fld_Type2_Get( pv_buff, st_LA_Fld_IntrusionCall, CMBS_UTIL_LA_FLD_INTRUSION_CALL_MODE );
}


E_CMBS_RC               cmbs_util_LA_Fld_PermanentCLIRAdd(  u8 * pv_buff, ST_UTIL_LA_FLD_PERMANENT_CLIR *st_LA_Fld_PermanentCLIR, u16 *pindex )
{
u16 u16_LengthIndex;

     pv_buff[(*pindex)++]  = st_LA_Fld_PermanentCLIR->u8_FieldId;
     u16_LengthIndex = (*pindex)++;
     pv_buff[(*pindex)++]  = ADD_MSB(st_LA_Fld_PermanentCLIR->u8_Attribute);
     pv_buff[(*pindex)++]  = st_LA_Fld_PermanentCLIR->u8_Value;

if(st_LA_Fld_PermanentCLIR->u8_ActDataLen + st_LA_Fld_PermanentCLIR->u8_DesactDataLen)
{
     pv_buff[(*pindex)++]  = st_LA_Fld_PermanentCLIR->u8_ActDataLen;
    if(st_LA_Fld_PermanentCLIR->u8_ActDataLen)
    {
           cfr_ie_ser_pu8(   &pv_buff[*pindex] , st_LA_Fld_PermanentCLIR->pu8_ActData, st_LA_Fld_PermanentCLIR->u8_ActDataLen );
            (*pindex)+=st_LA_Fld_PermanentCLIR->u8_ActDataLen;
    }

    pv_buff[(*pindex)++]  = st_LA_Fld_PermanentCLIR->u8_DesactDataLen;
    if(st_LA_Fld_PermanentCLIR->u8_DesactDataLen)
    { 
     cfr_ie_ser_pu8(   &pv_buff[*pindex] , st_LA_Fld_PermanentCLIR->pu8_DesactData, st_LA_Fld_PermanentCLIR->u8_DesactDataLen );
     (*pindex)+=st_LA_Fld_PermanentCLIR->u8_DesactDataLen;
    }
}
	
pv_buff[u16_LengthIndex] = ADD_MSB((*pindex) - u16_LengthIndex - 1);
   return CMBS_RC_OK;
	 
}


E_CMBS_RC               cmbs_util_LA_Fld_PermanentCLIRGet(  u8 * pv_buff, ST_UTIL_LA_FLD_PERMANENT_CLIR *st_LA_Fld_PermanentCLIR)
{

   u8 * pu8_Buffer = (u8*)pv_buff;
   u16 u16_index=0;
   u16 u16_FieldLen;
   
      st_LA_Fld_PermanentCLIR->u8_FieldId     = pu8_Buffer[u16_index++];
      cmbs_util_LA_GetInfo_ShiftIndex(pu8_Buffer, &u16_FieldLen, &u16_index); // get length (unused) but shift index
      st_LA_Fld_PermanentCLIR->u8_Attribute   = REMOVE_MSB(pu8_Buffer[u16_index++]);
      st_LA_Fld_PermanentCLIR->u8_Value= pu8_Buffer[u16_index++];

	  if(u16_FieldLen>2)
	  	{
	  cmbs_util_LA_GetLengthAndDataShiftIndex(pu8_Buffer, 
	  	&st_LA_Fld_PermanentCLIR->u8_ActDataLen,
	  	st_LA_Fld_PermanentCLIR->pu8_ActData,
	  	&u16_index);

	  cmbs_util_LA_GetLengthAndDataShiftIndex(pu8_Buffer, 
	  	&st_LA_Fld_PermanentCLIR->u8_DesactDataLen,
	  	st_LA_Fld_PermanentCLIR->pu8_DesactData,
	  	&u16_index);
	  	}

   return CMBS_RC_OK;

}


E_CMBS_RC               cmbs_util_LA_Fld_CallForwardingAdd(  u8 * pv_buff, ST_UTIL_LA_FLD_CALL_FORWARDING *st_LA_Fld_CallForwarding, u16 *pindex )
{
u16 u16_LengthIndex;

     pv_buff[(*pindex)++]  = st_LA_Fld_CallForwarding->u8_FieldId;
     u16_LengthIndex = (*pindex)++;
     pv_buff[(*pindex)++]  = ADD_MSB(st_LA_Fld_CallForwarding->u8_Attribute);
	if(!(st_LA_Fld_CallForwarding -> u8_NotSetByUSer))
	{
	     pv_buff[(*pindex)++]  = st_LA_Fld_CallForwarding->u8_Value;

	     pv_buff[(*pindex)++]  = st_LA_Fld_CallForwarding->u8_ActDataLen;
	    if(st_LA_Fld_CallForwarding->u8_ActDataLen)
	    {
	           cfr_ie_ser_pu8(   &pv_buff[*pindex] , st_LA_Fld_CallForwarding->pu8_ActData, st_LA_Fld_CallForwarding->u8_ActDataLen );
	            (*pindex)+=st_LA_Fld_CallForwarding->u8_ActDataLen;
	    }

	    pv_buff[(*pindex)++]  = st_LA_Fld_CallForwarding->u8_DesactDataLen;
	    if(st_LA_Fld_CallForwarding->u8_DesactDataLen)
	    { 
	     cfr_ie_ser_pu8(   &pv_buff[*pindex] , st_LA_Fld_CallForwarding->pu8_DesactData, st_LA_Fld_CallForwarding->u8_DesactDataLen );
	     (*pindex)+=st_LA_Fld_CallForwarding->u8_DesactDataLen;
	    }
		
	    pv_buff[(*pindex)++]  = st_LA_Fld_CallForwarding->u8_NumberLen;
	    if(st_LA_Fld_CallForwarding->u8_NumberLen)
	    { 
	     cfr_ie_ser_pu8(   &pv_buff[*pindex] , st_LA_Fld_CallForwarding->pu8_Number, st_LA_Fld_CallForwarding->u8_NumberLen );
	     (*pindex)+=st_LA_Fld_CallForwarding->u8_NumberLen;
	    }

}

pv_buff[u16_LengthIndex] = ADD_MSB((*pindex) - u16_LengthIndex - 1);
   return CMBS_RC_OK;
}


E_CMBS_RC               cmbs_util_LA_Fld_CallForwardingGet(  u8 * pv_buff, ST_UTIL_LA_FLD_CALL_FORWARDING *st_LA_Fld_CallForwarding)
{
   u8 * pu8_Buffer = (u8*)pv_buff;
   u16 u16_index=0;
   u16 u16_DummyLen;
   
      st_LA_Fld_CallForwarding->u8_FieldId     = pu8_Buffer[u16_index++];
      cmbs_util_LA_GetInfo_ShiftIndex(pu8_Buffer, &u16_DummyLen, &u16_index); // get length (unused) but shift index
      st_LA_Fld_CallForwarding->u8_Attribute   = REMOVE_MSB(pu8_Buffer[u16_index++]);
      st_LA_Fld_CallForwarding->u8_Value= pu8_Buffer[u16_index++];
	  
	  cmbs_util_LA_GetLengthAndDataShiftIndex(pu8_Buffer, 
	  	&st_LA_Fld_CallForwarding->u8_ActDataLen,
	  	st_LA_Fld_CallForwarding->pu8_ActData,
	  	&u16_index);

	  cmbs_util_LA_GetLengthAndDataShiftIndex(pu8_Buffer, 
	  	&st_LA_Fld_CallForwarding->u8_DesactDataLen,
	  	st_LA_Fld_CallForwarding->pu8_DesactData,
	  	&u16_index);
	  
	  cmbs_util_LA_GetLengthAndDataShiftIndex(pu8_Buffer, 
	  	&st_LA_Fld_CallForwarding->u8_NumberLen,
	  	st_LA_Fld_CallForwarding->pu8_Number,
	  	&u16_index);

   return CMBS_RC_OK;
}



E_CMBS_RC               cmbs_util_LA_Fld_CFNAAdd(  u8 * pv_buff, ST_UTIL_LA_FLD_CFNA *st_LA_Fld_CFNA,  u16 *pindex )
{
u16 u16_LengthIndex;

     pv_buff[(*pindex)++]  = st_LA_Fld_CFNA->u8_FieldId;
     u16_LengthIndex = (*pindex)++;
     pv_buff[(*pindex)++]  = ADD_MSB(st_LA_Fld_CFNA->u8_Attribute);
if(!(st_LA_Fld_CFNA -> u8_NotSetByUSer))
{
     pv_buff[(*pindex)++]  = st_LA_Fld_CFNA->u8_Value;
     pv_buff[(*pindex)++]  = st_LA_Fld_CFNA->u8_NbSecond;

	     pv_buff[(*pindex)++]  = st_LA_Fld_CFNA->u8_ActDataLen;
	    if(st_LA_Fld_CFNA->u8_ActDataLen)
	    {
	           cfr_ie_ser_pu8(   &pv_buff[*pindex] , st_LA_Fld_CFNA->pu8_ActData, st_LA_Fld_CFNA->u8_ActDataLen );
	            (*pindex)+=st_LA_Fld_CFNA->u8_ActDataLen;
	    }

	    pv_buff[(*pindex)++]  = st_LA_Fld_CFNA->u8_DesactDataLen;
	    if(st_LA_Fld_CFNA->u8_DesactDataLen)
	    { 
	     cfr_ie_ser_pu8(   &pv_buff[*pindex] , st_LA_Fld_CFNA->pu8_DesactData, st_LA_Fld_CFNA->u8_DesactDataLen );
	     (*pindex)+=st_LA_Fld_CFNA->u8_DesactDataLen;
	    }
		
	    pv_buff[(*pindex)++]  = st_LA_Fld_CFNA->u8_NumberLen;
	    if(st_LA_Fld_CFNA->u8_NumberLen)
	    { 
	     cfr_ie_ser_pu8(   &pv_buff[*pindex] , st_LA_Fld_CFNA->pu8_Number, st_LA_Fld_CFNA->u8_NumberLen );
	     (*pindex)+=st_LA_Fld_CFNA->u8_NumberLen;
	    }

}

pv_buff[u16_LengthIndex] = ADD_MSB((*pindex) - u16_LengthIndex - 1);
   return CMBS_RC_OK;
}


E_CMBS_RC               cmbs_util_LA_Fld_CFNAGet(  u8 * pv_buff, ST_UTIL_LA_FLD_CFNA *st_LA_Fld_CFNA)
	{   
	u8 * pu8_Buffer = (u8*)pv_buff;
   u16 u16_index=0;
   u16 u16_DummyLen;
   
      st_LA_Fld_CFNA->u8_FieldId     = pu8_Buffer[u16_index++];
      cmbs_util_LA_GetInfo_ShiftIndex(pu8_Buffer, &u16_DummyLen, &u16_index); // get length (unused) but shift index
      st_LA_Fld_CFNA->u8_Attribute   = REMOVE_MSB(pu8_Buffer[u16_index++]);
      st_LA_Fld_CFNA->u8_Value= pu8_Buffer[u16_index++];
      st_LA_Fld_CFNA->u8_NbSecond= pu8_Buffer[u16_index++];
	  
	  cmbs_util_LA_GetLengthAndDataShiftIndex(pu8_Buffer, 
	  	&st_LA_Fld_CFNA->u8_ActDataLen,
	  	st_LA_Fld_CFNA->pu8_ActData,
	  	&u16_index);

	  cmbs_util_LA_GetLengthAndDataShiftIndex(pu8_Buffer, 
	  	&st_LA_Fld_CFNA->u8_DesactDataLen,
	  	st_LA_Fld_CFNA->pu8_DesactData,
	  	&u16_index);
	  
	  cmbs_util_LA_GetLengthAndDataShiftIndex(pu8_Buffer, 
	  	&st_LA_Fld_CFNA->u8_NumberLen,
	  	st_LA_Fld_CFNA->pu8_Number,
	  	&u16_index);

   return CMBS_RC_OK;
}

E_CMBS_RC               cmbs_util_LA_Fld_CFBAdd(  u8 * pv_buff, ST_UTIL_LA_FLD_CFB *st_LA_Fld_CFB, u16 *pindex )
{
u16 u16_LengthIndex;

     pv_buff[(*pindex)++]  = st_LA_Fld_CFB->u8_FieldId;
     u16_LengthIndex = (*pindex)++;
     pv_buff[(*pindex)++]  = ADD_MSB(st_LA_Fld_CFB->u8_Attribute);
	if(!(st_LA_Fld_CFB ->u8_NotSetByUSer))
	{
	     pv_buff[(*pindex)++]  = st_LA_Fld_CFB->u8_Value;

	     pv_buff[(*pindex)++]  = st_LA_Fld_CFB->u8_ActDataLen;
	    if(st_LA_Fld_CFB->u8_ActDataLen)
	    {
	           cfr_ie_ser_pu8(   &pv_buff[*pindex] , st_LA_Fld_CFB->pu8_ActData, st_LA_Fld_CFB->u8_ActDataLen );
	            (*pindex)+=st_LA_Fld_CFB->u8_ActDataLen;
	    }

	    pv_buff[(*pindex)++]  = st_LA_Fld_CFB->u8_DesactDataLen;
	    if(st_LA_Fld_CFB->u8_DesactDataLen)
	    { 
	     cfr_ie_ser_pu8(   &pv_buff[*pindex] , st_LA_Fld_CFB->pu8_DesactData, st_LA_Fld_CFB->u8_DesactDataLen );
	     (*pindex)+=st_LA_Fld_CFB->u8_DesactDataLen;
	    }
		
	    pv_buff[(*pindex)++]  = st_LA_Fld_CFB->u8_NumberLen;
	    if(st_LA_Fld_CFB->u8_NumberLen)
	    { 
	     cfr_ie_ser_pu8(   &pv_buff[*pindex] , st_LA_Fld_CFB->pu8_Number, st_LA_Fld_CFB->u8_NumberLen );
	     (*pindex)+=st_LA_Fld_CFB->u8_NumberLen;
	    }

}

pv_buff[u16_LengthIndex] = ADD_MSB((*pindex) - u16_LengthIndex - 1);
   return CMBS_RC_OK;
}


E_CMBS_RC               cmbs_util_LA_Fld_CFBGet(  u8 * pv_buff, ST_UTIL_LA_FLD_CFB *st_LA_Fld_CFB )
{
	u8 * pu8_Buffer = (u8*)pv_buff;
   u16 u16_index=0;
   u16 u16_DummyLen;
   
      st_LA_Fld_CFB->u8_FieldId     = pu8_Buffer[u16_index++];
      cmbs_util_LA_GetInfo_ShiftIndex(pu8_Buffer, &u16_DummyLen, &u16_index); // get length (unused) but shift index
      st_LA_Fld_CFB->u8_Attribute   = REMOVE_MSB(pu8_Buffer[u16_index++]);
      st_LA_Fld_CFB->u8_Value= pu8_Buffer[u16_index++];
	  
	  cmbs_util_LA_GetLengthAndDataShiftIndex(pu8_Buffer, 
	  	&st_LA_Fld_CFB->u8_ActDataLen,
	  	st_LA_Fld_CFB->pu8_ActData,
	  	&u16_index);

	  cmbs_util_LA_GetLengthAndDataShiftIndex(pu8_Buffer, 
	  	&st_LA_Fld_CFB->u8_DesactDataLen,
	  	st_LA_Fld_CFB->pu8_DesactData,
	  	&u16_index);
	  
	  cmbs_util_LA_GetLengthAndDataShiftIndex(pu8_Buffer, 
	  	&st_LA_Fld_CFB->u8_NumberLen,
	  	st_LA_Fld_CFB->pu8_Number,
	  	&u16_index);

   return CMBS_RC_OK;
}





/*****************************************************************************
 * List Access fields
 *****************************************************************************/

E_CMBS_RC         cmbs_util_LA_Fld_Type1_Add( u8 * pv_buff,
                                                ST_UTIL_LA_FLD_TYPE_1 * pst_LA_Fld_Type1,
                                                E_CMBS_UTIL_LA_TYPE e_LATYPE,
                                                u16 *pindex )
{

   if( e_LATYPE != CMBS_UTIL_LA_FLD_NEW &&
       e_LATYPE != CMBS_UTIL_LA_FLD_CALL_TYPE )
   {
  //    CFR_DBG_ERROR( "cmbs_util_LA_Fld_Type1_Add ERROR: IE_TYPE %d no ST_UTIL_LA_FLD_TYPE_1\n", e_LATYPE );
      return CMBS_RC_ERROR_PARAMETER;
   }

   pv_buff[(*pindex)++]    = pst_LA_Fld_Type1->u8_FieldId;
   pv_buff[(*pindex)++]     = ADD_MSB(1);  // Length
   pv_buff[(*pindex)++]     = ADD_MSB(pst_LA_Fld_Type1->u8_Attribute);
   

   return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_util_LA_Fld_Type1_Get( u8 * pv_buff, ST_UTIL_LA_FLD_TYPE_1 * pst_LA_Fld_Type1, E_CMBS_UTIL_LA_TYPE type )
{
   u8 * pu8_Buffer = (u8*)pv_buff;

   if( type == CMBS_UTIL_LA_FLD_NEW ||
       type == CMBS_UTIL_LA_FLD_CALL_TYPE )
   {
      pst_LA_Fld_Type1->u8_FieldId= pu8_Buffer[0];
      pst_LA_Fld_Type1->u8_Attribute = REMOVE_MSB(pu8_Buffer[2]);
   	return CMBS_RC_OK;
   }

   return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_util_LA_Fld_Type2_Add( u8 * pv_buff,
                                                ST_UTIL_LA_FLD_TYPE_2 * pst_LA_Fld_Type2,
                                                E_CMBS_UTIL_LA_TYPE e_LATYPE,
                                                u16 *pindex  )
{

   if( e_LATYPE != CMBS_UTIL_LA_FLD_ASSOC_MDY &&
       e_LATYPE != CMBS_UTIL_LA_FLD_NR_OF_CALLS &&
       e_LATYPE != CMBS_UTIL_LA_FLD_FP_VOLUME &&
       e_LATYPE != CMBS_UTIL_LA_FLD_MULTI_CALL_MODE &&
       e_LATYPE != CMBS_UTIL_LA_FLD_INTRUSION_CALL_MODE)
   {
   //   CFR_DBG_ERROR( "cmbs_util_LA_Fld_Type2_Add ERROR: IE_TYPE %d no ST_UTIL_LA_FLD_TYPE_2\n", e_LATYPE );
      return CMBS_RC_ERROR_PARAMETER;
   }

   pv_buff[(*pindex)++]    = pst_LA_Fld_Type2->u8_FieldId;
    pv_buff[(*pindex)++]  = ADD_MSB(2);  // Length

    pv_buff[(*pindex)++]  = ADD_MSB(pst_LA_Fld_Type2->u8_Attribute);
    pv_buff[(*pindex)++]  = pst_LA_Fld_Type2->u8_Value;

   return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_util_LA_Fld_Type2_Get( u8 * pv_buff, ST_UTIL_LA_FLD_TYPE_2 * pst_LA_Fld_Type2, E_CMBS_UTIL_LA_TYPE type  )
{
   u8 * pu8_Buffer = (u8*)pv_buff;

   if( type == CMBS_UTIL_LA_FLD_ASSOC_MDY ||
       type == CMBS_UTIL_LA_FLD_NR_OF_CALLS ||
       type == CMBS_UTIL_LA_FLD_FP_VOLUME ||
       type == CMBS_UTIL_LA_FLD_MULTI_CALL_MODE ||
       type == CMBS_UTIL_LA_FLD_INTRUSION_CALL_MODE )
   {
      pst_LA_Fld_Type2->u8_FieldId= pu8_Buffer[0];
      pst_LA_Fld_Type2->u8_Attribute = REMOVE_MSB(pu8_Buffer[2]);
      pst_LA_Fld_Type2->u8_Value     = pu8_Buffer[3];

   	return CMBS_RC_OK;
   }

   return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_util_LA_Fld_Type3_Add( u8 * pv_buff,
                                                ST_UTIL_LA_FLD_TYPE_3 * pst_LA_Fld_Type3,
                                                E_CMBS_UTIL_LA_TYPE e_LATYPE,
                                                u16 *pindex  )
{

   if( e_LATYPE != CMBS_UTIL_LA_FLD_NAME       &&
       e_LATYPE != CMBS_UTIL_LA_FLD_NUMBER     &&
       e_LATYPE != CMBS_UTIL_LA_FLD_LINE_NAME  &&
       e_LATYPE != CMBS_UTIL_LA_FLD_FIRST_NAME &&
       e_LATYPE != CMBS_UTIL_LA_FLD_CONTACT_NR )
   {
   //   CFR_DBG_ERROR( "cmbs_util_LA_Fld_Type3_Add ERROR: IE_TYPE %d no ST_UTIL_LA_FLD_TYPE_3\n", e_LATYPE );
      return CMBS_RC_ERROR_PARAMETER;
   }

     pv_buff[(*pindex)++]  = pst_LA_Fld_Type3->u8_FieldId;


cmbs_util_add_u16_and_shift(pv_buff, pst_LA_Fld_Type3->u16_DataLen + 1, pindex);


     pv_buff[(*pindex)++]  = ADD_MSB(pst_LA_Fld_Type3->u8_Attribute);

   cfr_ie_ser_pu8(   &pv_buff[*pindex] , pst_LA_Fld_Type3->pu8_Data, pst_LA_Fld_Type3->u16_DataLen );

   *pindex+=(pst_LA_Fld_Type3->u16_DataLen); // shift by lenght of data

   return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_util_LA_Fld_Type3_Get( u8 * pv_buff, ST_UTIL_LA_FLD_TYPE_3 * pst_LA_Fld_Type3 , E_CMBS_UTIL_LA_TYPE type )
{
   u8 * pu8_Buffer = (u8*)pv_buff;
   u16 u16_index=0;

   if( type == CMBS_UTIL_LA_FLD_NAME       ||
       type  == CMBS_UTIL_LA_FLD_NUMBER     ||
       type == CMBS_UTIL_LA_FLD_LINE_NAME  ||
       type == CMBS_UTIL_LA_FLD_FIRST_NAME ||
       type == CMBS_UTIL_LA_FLD_CONTACT_NR )
   {
      pst_LA_Fld_Type3->u8_FieldId     = pu8_Buffer[u16_index++];
	  
    cmbs_util_LA_GetInfo_ShiftIndex(pu8_Buffer, &(pst_LA_Fld_Type3->u16_DataLen), &u16_index);

		  (pst_LA_Fld_Type3->u16_DataLen)--; // remove attribute for the len of the datag
      pst_LA_Fld_Type3->u8_Attribute   = REMOVE_MSB(pu8_Buffer[u16_index++]);
      pst_LA_Fld_Type3->pu8_Data       = &pu8_Buffer[u16_index];

   	return CMBS_RC_OK;
   }

   return CMBS_RC_ERROR_PARAMETER;
}


/*****************************************************************************/

void cmbs_util_LA_GetLengthAndDataShiftIndex(u8 *pu8_Buffer, 
	  	u8* pu8_length,
	  	u8 * pu8_data,
	  	u16 *pu16_index)
{

    *pu8_length= pu8_Buffer[*pu16_index];
	if(*pu8_length)
		{
                 pu8_data       = pu8_Buffer+(++(*pu16_index));
	         (*pu16_index)+=*pu8_length;
		}
	else
		{
                 pu8_data = NULL;
		}
return;
}


void		cmbs_util_la_ShiftIndexToNextFieldID(u8 *pu8_FieldId, u16 *pu16_Index)
{
u16 u16_field_len;  // take into account FieldID size = 1
u16 Dummy_index=0;

    cmbs_util_LA_GetInfo_ShiftIndex(pu8_FieldId+1, &u16_field_len, &Dummy_index);  // +1 to point on length field
	  PRINT("cmbs_util_LA_GetInfo_ShiftIndex: u16_field_len=%x\n",u16_field_len);

	  if(u16_field_len<0x80)
		(*pu16_Index)+=(u16_field_len + 2);  // to take FieldID and field len into account 
	else
		(*pu16_Index)+=(u16_field_len + 3);  //to take FieldID and field len into account

}

void cmbs_util_LA_GetInfo_ShiftIndex(u8 *pu8_buf, u16 *pu16_data_to_fill, u16 *pu16_Index)
{
if (cmbs_util_LA_extendlen(pu8_buf[*pu16_Index]))
	  	{
	  	*pu16_data_to_fill   = (pu8_buf[*pu16_Index] << 7) + REMOVE_MSB(pu8_buf[*pu16_Index+1]);
		(*pu16_Index)++;
	  	}
	  else
	  	{
	  	*pu16_data_to_fill   = REMOVE_MSB(pu8_buf[*pu16_Index]);
	  	}
		(*pu16_Index)++;
}

void cmbs_util_add_u16_and_shift(u8 *pu8_buf, u16 u16_value_to_write, u16 *pu16_Index)
{
if (u16_value_to_write < 0x80)  
{
	pu8_buf[*pu16_Index] = (u8) ADD_MSB(u16_value_to_write);
}
else
{
	pu8_buf[*pu16_Index]  = (u8) REMOVE_MSB(u16_value_to_write >> 7);
	(*pu16_Index)++;
	pu8_buf[*pu16_Index] = ADD_MSB( u16_value_to_write);
}
(*pu16_Index)++;
}

u8 cmbs_util_LA_extendlen(u8 byte)
{
// if MSB==0 return 1
return ((byte>>7==0)?1:0); 
}

   
E_CMBS_RC cmbs_util_LA_AddEntryToFinalBuffer( void * pv_AppRef,
                                              u16 u16_SessionId,
                                              u8 * pu8_EntryBuff,
                                              u16 u16_sizeofbuff )
{
u16 i_SessionIndex = app_LaGetSessionIndex( u16_SessionId );

if (g_st_APP_LA_Session[i_SessionIndex].u16_SizeToSend+u16_sizeofbuff<CMBS_UTIL_LA_MAX_BUFFER)
{
PRINT("Add %d data\n",u16_sizeofbuff);
PRINT("index %x \n",g_st_APP_LA_Session[i_SessionIndex].u16_SizeToSend);
PRINT("data1 %x data\n",pu8_EntryBuff[0]);
PRINT("data2 %x data\n",pu8_EntryBuff[1]);
cfr_ie_ser_pu8( (g_st_APP_LA_Session[i_SessionIndex].u8_finalbuffer)+g_st_APP_LA_Session[i_SessionIndex].u16_SizeToSend, pu8_EntryBuff, u16_sizeofbuff );
g_st_APP_LA_Session[i_SessionIndex].u16_SizeToSend+=u16_sizeofbuff;
g_st_APP_LA_Session[i_SessionIndex].u16_Index=0;
     return CMBS_RC_OK;
}
else
    return CMBS_RC_ERROR_OUT_OF_MEM;


}


void    cmbs_util_LA_SendFinalBuffer( void * pv_AppRef,
                                              u16 u16_SessionId)
{
u8 IsLast_catiq;
u16 u16_LocalSize;
u16 i_SessionIndex = app_LaGetSessionIndex( u16_SessionId );

if (g_st_APP_LA_Session[i_SessionIndex].u16_SizeToSend<57)
{
u16_LocalSize=g_st_APP_LA_Session[i_SessionIndex].u16_SizeToSend;
IsLast_catiq = 1;
g_st_APP_LA_Session[i_SessionIndex].u16_SizeToSend=0;
}
else
{
u16_LocalSize=56;
IsLast_catiq=0;
g_st_APP_LA_Session[i_SessionIndex].u16_SizeToSend-=56;
}


PRINT("Sending %d data\n",u16_LocalSize);

cmbs_dsr_la_DataPacketSend( pv_AppRef,
                                  u16_SessionId,
                                  IsLast_catiq,
                                  g_st_APP_LA_Session[i_SessionIndex].u8_finalbuffer+g_st_APP_LA_Session[i_SessionIndex].u16_Index,
                                  u16_LocalSize);
if (IsLast_catiq)
{
	g_st_APP_LA_Session[i_SessionIndex].u16_Index=0;
}
else
{
g_st_APP_LA_Session[i_SessionIndex].u16_Index+=u16_LocalSize;
}
}

