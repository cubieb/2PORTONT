#ifndef _PRMT_LANDEVICE_USB_H_
#define _PRMT_LANDEVICE_USB_H_

#include "prmt_igd.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _PRMT_USB_ETH_
/***_PRMT_USB_ETH_*****************************************************************************************************************/
#define CWMP_LANUSBIFNO		1
extern struct CWMP_NODE tLANUSBConfObject[];
extern char *usb_name[];
int getLANUSBStats(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int getLANUSBConf(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setLANUSBConf(char *name, struct CWMP_LEAF *entity, int type, void *data);
/***end _PRMT_USB_ETH_*****************************************************************************************************************/
#elif defined(_PRMT_USB_)
/***_PRMT_USB_*****************************************************************************************************************/
#define CWMP_LANUSBIFNO		2
extern struct CWMP_NODE tLANUSBConfObject[];
int getLANUSBStats(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int getLANUSBConf(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setLANUSBConf(char *name, struct CWMP_LEAF *entity, int type, void *data);
/***end _PRMT_USB_*****************************************************************************************************************/
#endif
	
#ifdef __cplusplus
}
#endif

#endif /*_PRMT_LANDEVICE_USB_H_*/
