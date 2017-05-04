/*!
*  \file       appfacility.h
*	\brief
*	\Author		stein
*
*	@(#)	%filespec: appfacility.h-2 %
*
*******************************************************************************
*	\par	History
*	\n==== History ============================================================\n
*	date			name		version	 action                                          \n
*	----------------------------------------------------------------------------\n
*******************************************************************************
*	COPYRIGHT DOSCH & AMAND RESEARCH GMBH & CO.KG
*	DOSCH & AMAND RESEARCH GMBH & CO.KG Confidential
*
*******************************************************************************/

#if	!defined( APPFACILITY_H )
#define	APPFACILITY_H


#if defined( __cplusplus )
extern "C"
{
#endif

E_CMBS_RC         app_FacilityVMWI( u16 u16_RequestId, u8 u8_LineId, u8 u8_Messages, char * psz_Handsets );
E_CMBS_RC         app_FacilityMissedCalls( u16 u16_RequestId, u8 u8_LineId, u8 u8_MissedCalls, char * psz_Handsets );
E_CMBS_RC         app_FacilityListChanged( u16 u16_RequestId, u8 u8_ListId, u8 u8_ListEntries, char * psz_Handsets );
E_CMBS_RC         app_FacilityWebContent( u16 u16_RequestId, u8 u8_NumOfWebCont, char * psz_Handsets );

E_CMBS_RC         app_FacilityPropEvent( u16 u16_RequestId, u16 u16_PropEvent, u8 * pu8_Data, u8 u8_DataLen, char * psz_Handsets );
E_CMBS_RC         app_FacilityDateTime( u16 u16_RequestId, ST_DATE_TIME * pst_DateTime, char * psz_Handsets );

#if defined( __cplusplus )
}
#endif

#endif	//APPFACILITY_H
//*/
