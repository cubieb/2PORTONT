#include "../../../includes/voip_manager.h"

#ifdef SA_CVM_NO_MAIN

int rtk_Si_SetDectPower( unsigned int power )
{
	return rtk_SetDectPower( power );
}

int rtk_Si_GetDectPower( void )
{
	return rtk_GetDectPower();
}

int rtk_Si_GetDectPage( void )
{
	return rtk_GetDectPage();
}

#endif /* SA_CVM_NO_MAIN */

