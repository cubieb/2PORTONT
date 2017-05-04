/*!
*  \file       tcx_eep.h
*  \brief
*  \Author     sergiym
*
*  @(#)  %filespec: tcx_eep.h~1 %
*
*******************************************************************************
*  \par  History
*  \n==== History ============================================================\n
*  date        name     version   action                                          \n
*  ----------------------------------------------------------------------------\n
*******************************************************************************/

#if   !defined( TCX_EEP_H )
#define  TCX_EEP_H


#if defined( __cplusplus )
extern "C"
{
#endif

int tcx_EepOpen(char * psz_EepFileName);

void tcx_EepRead(u8* pu8_OutBuf, u32 u32_Offset, u32 u32_Size);

void tcx_EepWrite(u8* pu8_InBuf, u32 u32_Offset, u32 u32_Size);

void tcx_EepClose(void);

u32 tcx_EepSize(void);

#if defined( __cplusplus )
}
#endif

#endif   // TCX_EEP_H
//*/
