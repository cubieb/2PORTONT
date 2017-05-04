#include "usb.h"
#include "transport.h"

/* eject media from CD-ROM to switch mode */
int usb_stor_eject_media_init(struct us_data *us);

/* have to eject media two times for some strange devices */
int usb_stor_eject_media_init2(struct us_data *us);

/* support Huawei E1756 to switch 3G mode */
int usb_stor_huawei_e1756_init(struct us_data *us);

/* support ZTE AC8710-AC2726-AC2736 to switch 3G mode */
int usb_stor_zte_init(struct us_data *us);

/* support ZTE MU350 AC560-AC580 to switch 3G mode */
int usb_stor_zte_init2(struct us_data *us);

/* support Qisda to switch 3G mode */
int usb_stor_qisda_init(struct us_data *us);

/* support DLINK DWM-162-U5CDROM or SIMCOM SIM5250 to switch 3G mode */
int usb_stor_check_vendor_init(struct us_data *us);

