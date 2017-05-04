/*!
*	\file			cmbs_api.c
*	\brief			
*	\Author		kelbch 
*
*	@(#)	%filespec: cmbs_api.c~DMZD53#3 %
*
*******************************************************************************
*	\par	History
*	\n==== History ============================================================\n
*	date			name		version	 action                                          \n
*	----------------------------------------------------------------------------\n
*	06-Feb-09   stein		1		Initialize \n
*	14-Feb-09   kelbch   2		Restruction \n
*  25-Feb-09   stein    61    Restructuration                             \n

*******************************************************************************/

#if defined(__arm)
# include "tclib.h"
# include "embedded.h"
#else
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
#endif

#include "cmbs_int.h"
#include "cfr_ie.h"

#if defined( CMBS_API_TARGET )
#include "tapp_log.h"
#endif

#include "cfr_debug.h"

/*****************************************************************************
 * API Maintenance functions
 *****************************************************************************/

E_CMBS_RC         cmbs_api_Init( E_CMBS_API_MODE e_Mode, ST_CMBS_DEV * pst_DevCtl, ST_CMBS_DEV * pst_DevMedia )
{
   int            nRetVal = CMBS_RC_OK;
   
   if( pst_DevCtl )
   {
      CFR_DBG_INFO( "cmbs_api_Init: Mode:%d DevType:%d Port:%d\n", e_Mode,
                  pst_DevCtl->e_DevType,
                  pst_DevCtl->u_Config.pUartCfg->u8_Port );
   }
               
   if( (nRetVal = cmbs_int_EnvCreate( e_Mode, pst_DevCtl, pst_DevMedia )) != CMBS_RC_OK )
   {
      CFR_DBG_ERROR( "cmbs_api_Init Error:%d\n", nRetVal );
   }

   return nRetVal;
}


void              cmbs_api_UnInit( void )
{   
   cmbs_int_EnvDestroy();
}


void *            cmbs_api_RegisterCb( void * pv_AppRef, PFN_CMBS_API_CB pfn_api_Cb, u16 u16_bcdVersion )
{   
   return cmbs_int_RegisterCb( pv_AppRef, pfn_api_Cb, u16_bcdVersion );
}


void              cmbs_api_UnregisterCb( void * pv_AppRefHandle )
{
   cmbs_int_UnregisterCb( pv_AppRefHandle );
}


u16               cmbs_api_ModuleVersionGet( void )
{
   return cmbs_int_ModuleVersionGet();
}


//*/
