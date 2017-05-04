/*  
 *   *  hello-1.c - The simplest kernel module.
 *    */
#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_ALERT */
#include "wapi_entry.h"

int init_module(void)
{
	wapiInit_hook=wapiInit;
	wapiExit_hook=wapiExit;	
	wapiStationInit_hook=wapiStationInit;
	wapiSetIE_hook=wapiSetIE;
	wapiReleaseFragementQueue_hook=wapiReleaseFragementQueue;
	DOT11_Process_WAPI_Info_hook=DOT11_Process_WAPI_Info;
	wapiHandleRecvPacket_hook=wapiHandleRecvPacket;
	wapiIEInfoInstall_hook=wapiIEInfoInstall;
	wapiReqActiveCA_hook=wapiReqActiveCA;
	wapiSetBK_hook=wapiSetBK;
	wapiSendUnicastKeyAgrementRequeset_hook=wapiSendUnicastKeyAgrementRequeset;
	wapiUpdateUSK_hook=wapiUpdateUSK;
	wapiUpdateMSK_hook=wapiUpdateMSK;
	return 0;
}

void cleanup_module(void)
{
	wapiInit_hook=NULL;
	wapiExit_hook=NULL;
	wapiStationInit_hook=NULL;
	wapiSetIE_hook=NULL;
	wapiReleaseFragementQueue_hook=NULL;
	DOT11_Process_WAPI_Info_hook=NULL;
	wapiHandleRecvPacket_hook=NULL;
	wapiIEInfoInstall_hook=NULL;
	wapiReqActiveCA_hook=NULL;
	wapiSetBK_hook=NULL;
	wapiSendUnicastKeyAgrementRequeset_hook=NULL;
	wapiUpdateUSK_hook=NULL;
	wapiUpdateMSK_hook=NULL;	
	
       return;	   
}

MODULE_LICENSE("Dual BSD/GPL");
