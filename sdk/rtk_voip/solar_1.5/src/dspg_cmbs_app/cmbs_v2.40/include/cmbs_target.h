/*!
*	\file		   cmbs_target.h
*	\brief		It contains the relevant HW information for Cordless Module Base(CMBS)
*	\Author		kelbch 
*
*   This file contains the relevant information and changes between the DSPG
*   Development board and the CMBS UART/IOM Module.
*   The UART IOM module uses following GPIOs:
*   UART    Base GPIO   7
*   IOM/PCM Base GPIO   2
*   IIC Bus      GPIO   0,1
*
*******************************************************************************
*	\par	History
*	\n==== History ============================================================\n
*	date			name		version	 action                                          \n
*	----------------------------------------------------------------------------\n
*  05-Mar-09   D.Kelbch  1.0      Initialize
*******************************************************************************
*	COPYRIGHT DOSCH & AMAND RESEARCH GMBH & CO.KG
*	DOSCH & AMAND RESEARCH GMBH & CO.KG Confidential
*
*******************************************************************************/

#if	!defined( CMBS_TARGET_H )
#define	CMBS_TARGET_H

#if defined (CMBS_HW_MODULE)
/* The CMBS_UART IOM module uses according schematics 
   GPIO 7,8,9,10 for UART communication
   GPIO 2,3,4,5  for PCM/IOM communication 
*/
#define DR18_UART_GPIO 7
#define DR18_IOM_GPIO 2 
#endif

#if defined( __cplusplus )
extern "C"
{
#endif

#if defined( __cplusplus )
}
#endif

#endif	//	CMBS_TARGET_H
//*/
