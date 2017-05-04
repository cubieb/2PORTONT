/*!
*  \file       tcx_eep.c
*  \brief      EEPROM functionality for the test application
*  \Author     sergiym
*
*  @(#)  %filespec: tcx_eep.c~1 %
*
*******************************************************************************
*  \par  History
*  \n==== History =============================================================\n
*  date        name     version   action                                       \n
*  ----------------------------------------------------------------------------\n
*   23-Apr-09  sergiym    1.0        Initial version                             \n
*******************************************************************************/

#include <stdio.h>

#include "cmbs_api.h"
#include "tcx_eep.h"


FILE * g_fpEepFile = NULL;


//    ========== tcx_EepOpen ===========
/*!
      \brief             Open the eeprom binary file
      \param[in]         pszEepFileName The name of the logfile
      \return            1 if successful, 0 if there was an error

*/
int tcx_EepOpen(char * psz_EepFileName)
{
   g_fpEepFile = fopen(psz_EepFileName, "r+b");

   if(g_fpEepFile == NULL)
   {
      return 0;
   }
   return 1;
}


//    ========== tcx_EepRead ===========
/*!
      \brief                Read data from EEPROM file
      \param[out]       pu8_OutBuf    Pointer to output data buffer
      \param[in]         u32_Offset     Offset in the EEPROM file
      \param[in]         u32_Size        Length of the data
      \return            none

*/
void tcx_EepRead(u8* pu8_OutBuf, u32 u32_Offset, u32 u32_Size)

{
   size_t  res = 0;
   
   if (g_fpEepFile != NULL)
   {
     fseek(g_fpEepFile, u32_Offset, SEEK_SET);
     res = fread(pu8_OutBuf, 1, u32_Size, g_fpEepFile);
   }

   if (res != u32_Size)
   {
     printf("ERROR: Can't read EEPROM file\n");
   }
}


//    ========== tcx_EepWrite ===========
/*!
      \brief                Write data to EEPROM file
      \param[out]       pu8_InBuf      Pointer to input data buffer
      \param[in]         u32_Offset     Offset in the EEPROM file
      \param[in]         u32_Size        Length of the data
      \return            none

*/
void tcx_EepWrite(u8* pu8_InBuf, u32 u32_Offset, u32 u32_Size)

{
   size_t  res = 0;
   
   if (g_fpEepFile != NULL)
   {
     fseek(g_fpEepFile, u32_Offset, SEEK_SET);
     res = fwrite(pu8_InBuf, 1, u32_Size, g_fpEepFile);
     fflush(g_fpEepFile);
   }

   if (res != u32_Size)
   {
     printf("ERROR: Can't write EEPROM file\n");
   }
}


//    ========== tcx_EepClose ===========
/*!
      \brief             Close the EEPROM file
      \return            none

*/
void tcx_EepClose(void)
{
   if( g_fpEepFile != NULL)
   {
      fclose(g_fpEepFile);
      g_fpEepFile = NULL;
   }
}


//    ========== tcx_EepSize ===========
/*!
      \brief             Return size of the EEPROM file
      \return           Size of the EEPROM file

*/
u32 tcx_EepSize(void)
{
   long size = 0;
   
   if( g_fpEepFile != NULL)
   {
      fseek(g_fpEepFile, 0, SEEK_END);
      size = ftell(g_fpEepFile);
   }
   return (size > 0) ? (u32)size : 0;
}

//*/
