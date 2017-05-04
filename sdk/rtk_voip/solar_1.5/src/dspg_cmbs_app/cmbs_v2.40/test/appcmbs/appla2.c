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

u8 *         cmbs_util_LA_EntryLenght_EntryIdentifierAdd( u8 *p_entry, u16 u16_index, u16 u16_EntryID)
{
if (u16_index < 128)
{
	p_entry[3] = (u8) (u16_index |= 0x80);
	if (u16_EntryID < 128)
		{
	           p_entry[2] = (u8) (u16_EntryID |= 0x80);
			   return (&p_entry[2]);
		}
	else
		{
	           p_entry[1] = (u8) ((u16_EntryID>>8) & 0x7F);
	           p_entry[2] = (u8) ((u16_EntryID&0x7F) | 0x80);
			   return (&p_entry[1]);
		}
		
}
else
{
	p_entry[2] = (u8) ((u16_index>>8) & 0x7F);
	p_entry[3] = (u8) ((u16_index&0x7F) | 0x80);
	if (u16_EntryID < 128)
		{
	           p_entry[1] = (u8) (u16_EntryID | 0x80);
			   return (&p_entry[1]);
		}
	else
		{
	           p_entry[0] =(u8) ((u16_EntryID>>8) & 0x7F);
	           p_entry[1] = (u8) ((u16_EntryID&0x7F) | 0x80);
			   return (&p_entry[0]);
		}
}

}


void cmbs_util_LA_EntryLenght_EntryIdentifierGet(u8 *pu8_Data, u16 *pu16_EntryIdentifier, u16 *pu16_EntryLength, u16 *pu16_Index) 
{

cmbs_util_LA_GetInfo_ShiftIndex(pu8_Data, pu16_EntryIdentifier, pu16_Index); // in case that info may be 0..16383
cmbs_util_LA_GetInfo_ShiftIndex(pu8_Data, pu16_EntryLength, pu16_Index); // in case that info may be 0..16383

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


   pv_buff[*pindex]    = pst_LA_Fld_DateTime->u8_FieldId;
   pv_buff[*pindex+1]     = 0x87;
   pv_buff[*pindex+ 2] = pst_LA_Fld_DateTime->u8_Attribute;
   pv_buff[*pindex+ 3] = pst_LA_Fld_DateTime->u8_Year;
   pv_buff[*pindex+ 4] = pst_LA_Fld_DateTime->u8_Month;
   pv_buff[*pindex+ 5] = pst_LA_Fld_DateTime->u8_Day;
   pv_buff[*pindex+ 6] = pst_LA_Fld_DateTime->u8_Hours;
   pv_buff[*pindex+ 7] = pst_LA_Fld_DateTime->u8_Mins;
   pv_buff[*pindex+ 8] = pst_LA_Fld_DateTime->u8_Secs;

*pindex+= 9;

   return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_util_LA_Fld_DateTimeGet( u8 * pv_buff,
                                                   ST_UTIL_LA_FLD_DATETIME * pst_LA_Fld_DateTime )
{
   u8 * pu8_Buffer = (u8*)pv_buff;

   if( *pu8_Buffer == CMBS_UTIL_LA_FLD_DATETIME )
   {
      pst_LA_Fld_DateTime->u8_Attribute = pu8_Buffer[CFR_IE_SIZE_POS + 1];
      pst_LA_Fld_DateTime->u8_Year      = pu8_Buffer[CFR_IE_SIZE_POS + 2];
      pst_LA_Fld_DateTime->u8_Month     = pu8_Buffer[CFR_IE_SIZE_POS + 3];
      pst_LA_Fld_DateTime->u8_Day       = pu8_Buffer[CFR_IE_SIZE_POS + 4];
      pst_LA_Fld_DateTime->u8_Hours     = pu8_Buffer[CFR_IE_SIZE_POS + 5];
      pst_LA_Fld_DateTime->u8_Mins      = pu8_Buffer[CFR_IE_SIZE_POS + 6];
      pst_LA_Fld_DateTime->u8_Secs      = pu8_Buffer[CFR_IE_SIZE_POS + 7];

   	return CMBS_RC_OK;
   }

   return CMBS_RC_ERROR_PARAMETER;
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
   return cmbs_util_LA_Fld_Type2_Add( pv_buff, pst_LA_Fld_LineId, CMBS_UTIL_LA_FLD_LINE_ID, pindex  );
}


E_CMBS_RC         cmbs_util_LA_Fld_LineIdGet( u8 * pv_buff, ST_UTIL_LA_FLD_LINE_ID * pst_LA_Fld_LineId )
{
   return cmbs_util_LA_Fld_Type2_Get( pv_buff, pst_LA_Fld_LineId,  CMBS_UTIL_LA_FLD_LINE_ID);
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
/*
u16 CorrespondingTable[256];
u16 ii;
u16 count=1;

// Building Corresponding table;

for(ii=0; ii<256; ii++)
{
	if (count%8)
	{
	CorrespondingTable[ii]=count++;
	}
	else
	{
	count++;
	CorrespondingTable[ii]=count++;
	}
}


={
	1  , 2  , 3  , 4  , 5  , 6  , 7 ,
	9  , 10, 11, 12, 13, 14, 15,
	17, 18, 19, 20, 21, 22, 23,
	25, 26, 27, 28, 29, 30, 31,
	33, 34, 35, 36, 37, 38, 39,
	41, 42, 43, 44, 45, 46, 47,
	49, 50, 51, 52, 
	57,
	63,
*/

// NOT WORKING, NEED TO BE CHANGED 

     pv_buff[(*pindex)++]  = st_LA_Fld_AttachedHandsets->u8_FieldId;
     pv_buff[(*pindex)++]  = ((st_LA_Fld_AttachedHandsets->u16_DataLen) / 7 + 1) + 2;  // +2 is for Attrib and Nb of handset
     pv_buff[(*pindex)++]  = st_LA_Fld_AttachedHandsets->u8_Attribute;
     pv_buff[(*pindex)++]  = (u8) (st_LA_Fld_AttachedHandsets->u16_DataLen);  // len is max ~40
	 
   cfr_ie_ser_pu8(   &pv_buff[*pindex] , st_LA_Fld_AttachedHandsets->pu8_Data, ((st_LA_Fld_AttachedHandsets->u16_DataLen) / 7 + 1) );
   (*pindex)+=((st_LA_Fld_AttachedHandsets->u16_DataLen) / 7 + 1);
   
   return CMBS_RC_OK;
}

E_CMBS_RC               cmbs_util_LA_Fld_AttachedHandsetsGet(  u8 * pv_buff, ST_UTIL_LA_FLD_ATTACHED_HANDSETS *st_LA_Fld_AttachedHandsets)
{
// To be coded

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
     pv_buff[(*pindex)++]  = st_LA_Fld_PermanentCLIR->u8_Attribute;
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
	
pv_buff[u16_LengthIndex] = (*pindex) - u16_LengthIndex - 1;
   return CMBS_RC_OK;
	 
}


E_CMBS_RC               cmbs_util_LA_Fld_PermanentCLIRGet(  u8 * pv_buff, ST_UTIL_LA_FLD_PERMANENT_CLIR *st_LA_Fld_PermanentCLIR)
{
// To be developped
   return CMBS_RC_OK;

}


E_CMBS_RC               cmbs_util_LA_Fld_CallForwardingAdd(  u8 * pv_buff, ST_UTIL_LA_FLD_CALL_FORWARDING *st_LA_Fld_CallForwarding, u16 *pindex )
{
u16 u16_LengthIndex;

     pv_buff[(*pindex)++]  = st_LA_Fld_CallForwarding->u8_FieldId;
     u16_LengthIndex = (*pindex)++;
     pv_buff[(*pindex)++]  = st_LA_Fld_CallForwarding->u8_Attribute;
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

pv_buff[u16_LengthIndex] = (*pindex) - u16_LengthIndex - 1;
   return CMBS_RC_OK;
}


E_CMBS_RC               cmbs_util_LA_Fld_CallForwardingGet(  u8 * pv_buff, ST_UTIL_LA_FLD_CALL_FORWARDING *st_LA_Fld_CallForwarding)
{
//to be developped

   return CMBS_RC_OK;
}



E_CMBS_RC               cmbs_util_LA_Fld_CFNAAdd(  u8 * pv_buff, ST_UTIL_LA_FLD_CFNA *st_LA_Fld_CFNA,  u16 *pindex )
{
u16 u16_LengthIndex;

     pv_buff[(*pindex)++]  = st_LA_Fld_CFNA->u8_FieldId;
     u16_LengthIndex = (*pindex)++;
     pv_buff[(*pindex)++]  = st_LA_Fld_CFNA->u8_Attribute;
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

pv_buff[u16_LengthIndex] = (*pindex) - u16_LengthIndex - 1;
   return CMBS_RC_OK;
}


E_CMBS_RC               cmbs_util_LA_Fld_CFNAGet(  u8 * pv_buff, ST_UTIL_LA_FLD_CFNA *st_LA_Fld_CFNA)
	{
// to be developped
   return CMBS_RC_OK;
}

E_CMBS_RC               cmbs_util_LA_Fld_CFBAdd(  u8 * pv_buff, ST_UTIL_LA_FLD_CFB *st_LA_Fld_CFB, u16 *pindex )
{
u16 u16_LengthIndex;

     pv_buff[(*pindex)++]  = st_LA_Fld_CFB->u8_FieldId;
     u16_LengthIndex = (*pindex)++;
     pv_buff[(*pindex)++]  = st_LA_Fld_CFB->u8_Attribute;
	if(!(st_LA_Fld_CFB -> u8_NotSetByUSer))
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

pv_buff[u16_LengthIndex] = (*pindex) - u16_LengthIndex - 1;
   return CMBS_RC_OK;
}


E_CMBS_RC               cmbs_util_LA_Fld_CFBGet(  u8 * pv_buff, ST_UTIL_LA_FLD_CFB *st_LA_Fld_CFB )
{
// to be developped
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
   pv_buff[(*pindex)++]     = 0x81;  // Length
   pv_buff[(*pindex)++]     = pst_LA_Fld_Type1->u8_Attribute;
   

   return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_util_LA_Fld_Type1_Get( u8 * pv_buff, ST_UTIL_LA_FLD_TYPE_1 * pst_LA_Fld_Type1, E_CMBS_UTIL_LA_TYPE type )
{
   u8 * pu8_Buffer = (u8*)pv_buff;

   if( type == CMBS_UTIL_LA_FLD_NEW ||
       type == CMBS_UTIL_LA_FLD_CALL_TYPE )
   {
      pst_LA_Fld_Type1->u8_FieldId= pu8_Buffer[0];
      pst_LA_Fld_Type1->u8_Attribute = pu8_Buffer[2];
   	return CMBS_RC_OK;
   }

   return CMBS_RC_ERROR_PARAMETER;
}


E_CMBS_RC         cmbs_util_LA_Fld_Type2_Add( u8 * pv_buff,
                                                ST_UTIL_LA_FLD_TYPE_2 * pst_LA_Fld_Type2,
                                                E_CMBS_UTIL_LA_TYPE e_LATYPE,
                                                u16 *pindex  )
{

   if( e_LATYPE != CMBS_UTIL_LA_FLD_LINE_ID   &&
       e_LATYPE != CMBS_UTIL_LA_FLD_ASSOC_MDY &&
       e_LATYPE != CMBS_UTIL_LA_FLD_NR_OF_CALLS &&
       e_LATYPE != CMBS_UTIL_LA_FLD_FP_VOLUME &&
       e_LATYPE != CMBS_UTIL_LA_FLD_MULTI_CALL_MODE &&
       e_LATYPE != CMBS_UTIL_LA_FLD_INTRUSION_CALL_MODE)
   {
   //   CFR_DBG_ERROR( "cmbs_util_LA_Fld_Type2_Add ERROR: IE_TYPE %d no ST_UTIL_LA_FLD_TYPE_2\n", e_LATYPE );
      return CMBS_RC_ERROR_PARAMETER;
   }

   pv_buff[(*pindex)++]    = pst_LA_Fld_Type2->u8_FieldId;
    pv_buff[(*pindex)++]  = 0x82;  // Length

    pv_buff[(*pindex)++]  = pst_LA_Fld_Type2->u8_Attribute;
    pv_buff[(*pindex)++]  = pst_LA_Fld_Type2->u8_Value;

   return CMBS_RC_OK;
}


E_CMBS_RC         cmbs_util_LA_Fld_Type2_Get( u8 * pv_buff, ST_UTIL_LA_FLD_TYPE_2 * pst_LA_Fld_Type2, E_CMBS_UTIL_LA_TYPE type  )
{
   u8 * pu8_Buffer = (u8*)pv_buff;

   if( type == CMBS_UTIL_LA_FLD_LINE_ID   ||
       type == CMBS_UTIL_LA_FLD_ASSOC_MDY ||
       type == CMBS_UTIL_LA_FLD_NR_OF_CALLS ||
       type == CMBS_UTIL_LA_FLD_FP_VOLUME ||
       type == CMBS_UTIL_LA_FLD_MULTI_CALL_MODE ||
       type == CMBS_UTIL_LA_FLD_INTRUSION_CALL_MODE )
   {
      pst_LA_Fld_Type2->u8_FieldId= pu8_Buffer[0];
      pst_LA_Fld_Type2->u8_Attribute = pu8_Buffer[2];
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


if (pst_LA_Fld_Type3->u16_DataLen < 127)   // need to keep one for Attribute
{
	*pindex = (u8) ((pst_LA_Fld_Type3->u16_DataLen + 1) | 0x80);
	(*pindex)++;
}
else
{
	pv_buff[*pindex] = (u8) (((pst_LA_Fld_Type3->u16_DataLen + 1) >> 8) & 0x7F);
	(*pindex)++;
	pv_buff[*pindex] = (u8) (((pst_LA_Fld_Type3->u16_DataLen + 1) & 0x7F) | 0x80);
	(*pindex)++;
}

     pv_buff[(*pindex)++]  = pst_LA_Fld_Type3->u8_Attribute;

   cfr_ie_ser_pu8(   &pv_buff[*pindex] , pst_LA_Fld_Type3->pu8_Data, pst_LA_Fld_Type3->u16_DataLen );

   *pindex+=(1 + pst_LA_Fld_Type3->u16_DataLen + 2); // shift by lenght of data + 2

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

	  
      pst_LA_Fld_Type3->u8_Attribute   = pu8_Buffer[u16_index++];
      pst_LA_Fld_Type3->pu8_Data       = &pu8_Buffer[u16_index];

   	return CMBS_RC_OK;
   }

   return CMBS_RC_ERROR_PARAMETER;
}


/*****************************************************************************/


void		cmbs_util_la_ShiftIndexToNextFieldID(u8 *pu8_FieldId, u16 *u16_Index)
{
u16 u16_field_len;  // take into account FieldID size = 1
u16 Dummy_index;

    cmbs_util_LA_GetInfo_ShiftIndex(pu8_FieldId+1, &u16_field_len, &Dummy_index);  // +1 to point on length field
	  
(*u16_Index)+=(u16_field_len + 1);  // +1 to take FieldID into account

}

void cmbs_util_LA_GetInfo_ShiftIndex(u8 *pu8_buf, u16 *pu16_data_to_fill, u16 *pu16_Index)
{
if (cmbs_util_LA_extendlen(*pu8_buf))
	  	{
	  	*pu16_data_to_fill   = (*pu8_buf << 7) + (pu8_buf[1] & 0x7F);
		*pu16_Index+=2;
	  	}
	  else
	  	{
	  	*pu16_data_to_fill   = (*pu8_buf) & 0x7F;
		(*pu16_Index)++;
	  	}
}

u8 cmbs_util_LA_extendlen(u8 byte)
{
// if MSB==0 return 1
return ((byte>>7==0)?1:0); 
}

