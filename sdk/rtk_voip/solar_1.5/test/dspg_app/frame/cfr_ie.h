/*!
*	\file			cfr_ie.h
*	\brief			
*	\Author		kelbch 
*
*	@(#)	%filespec: cfr_ie.h-1 %
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

#if	!defined( CFR_IE_H )
#define	CFR_IE_H

#define		CFR_IE_TYPE_POS		0
#define		CFR_IE_SIZE_POS		1
#define		CFR_IE_HEADER_SIZE	2

typedef struct
{
	u16		u16_MaxSize;
	u16		u16_CurSize;
	u16		u16_CurIE;
	u8 *    pu8_Buffer;
} ST_CFR_IE_LIST, * PST_CFR_IE_LIST;

#if defined( __cplusplus )
extern "C"
{
#endif

void *			cfr_ie_ListNew( u8* pu8_Buffer, u16 u16_Size );
int				cfr_ie_ItemAdd( PST_CFR_IE_LIST p_List, u8 * pu8_Buffer, u16 u16_Size);
u8*				cfr_ie_ItemGet( PST_CFR_IE_LIST p_List );
u8* 				cfr_ie_ItemNextGet( PST_CFR_IE_LIST p_List );
u8 * 			    cfr_ie_ItemFirstGet ( PST_CFR_IE_LIST p_List );

void				cfr_ie_Serialize( PST_CFR_IE_LIST p_List, E_CMBS_IE_TYPE e_IE, void * pv_Data );

u16				cfr_ie_ser_u16 ( u8 * pu8_buffer, u16 u16_Value );
u16				cfr_ie_ser_u32 ( u8 * pu8_buffer, u32 u32_Value );
u16				cfr_ie_ser_pu8 ( u8 * pu8_buffer, u8 * pu8_Data, u16 u16_DataSize );

u16				cfr_ie_dser_u16( u8 *pu8_Buffer, u16 * pu16_Value );
u16				cfr_ie_dser_u32 ( u8 * pu8_Buffer, u32 * pu32_Value );

#if defined( __cplusplus )
}
#endif

#endif	//	CFR_IE_H
//*/
