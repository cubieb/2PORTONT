#include <linux/sched.h>
#include <linux/errno.h>

#include "usb.h"
#include "initializers.h"
#include "debug.h"
#include "transport.h"

/* eject media from CD-ROM to switch mode */
int usb_stor_eject_media_init(struct us_data *us)
{
	struct bulk_cb_wrap *bcb = (struct bulk_cb_wrap*) us->iobuf;
	struct bulk_cs_wrap *bcs = (struct bulk_cs_wrap*) us->iobuf;
	int res, act_len;

	bcb->Signature = cpu_to_le32(US_BULK_CB_SIGN);
	bcb->Tag = 0;
	bcb->DataTransferLength = cpu_to_le32(0);
	bcb->Flags = bcb->Lun = 0;
	bcb->Length = 6;
	memset(bcb->CDB, 0, sizeof(bcb->CDB));
	bcb->CDB[0] = 0x1b;	//start stop unit
	bcb->CDB[4] = 0x02;	//eject media

	res = usb_stor_bulk_transfer_buf(us, us->send_bulk_pipe, bcb,
			US_BULK_CB_WRAP_LEN, &act_len);
	if(res)
		return res;

	res = usb_stor_bulk_transfer_buf(us, us->recv_bulk_pipe, bcs,
			US_BULK_CS_WRAP_LEN, &act_len);

	return -1;
}

/* have to eject media two times for some strange devices */
int usb_stor_eject_media_init2(struct us_data *us)
{
	usb_stor_eject_media_init(us);
	return usb_stor_eject_media_init(us);
}

/* support Huawei E1756 to switch 3G mode */
int usb_stor_huawei_e1756_init(struct us_data *us)
{
	struct bulk_cb_wrap *bcb = (struct bulk_cb_wrap*) us->iobuf;
	struct bulk_cs_wrap *bcs = (struct bulk_cs_wrap*) us->iobuf;
	int res, act_len;

	bcb->Signature = cpu_to_le32(US_BULK_CB_SIGN);
	bcb->Tag = 0;
	bcb->DataTransferLength = cpu_to_le32(0);
	bcb->Flags = bcb->Lun = 0;
	bcb->Length = 0;
	memset(bcb->CDB, 0, sizeof(bcb->CDB));
	bcb->CDB[0] = 0x11;
	bcb->CDB[1] = 0x06;

	res = usb_stor_bulk_transfer_buf(us, us->send_bulk_pipe, bcb,
			US_BULK_CB_WRAP_LEN, &act_len);
	if(res)
		return res;

	res = usb_stor_bulk_transfer_buf(us, us->recv_bulk_pipe, bcs,
			US_BULK_CS_WRAP_LEN, &act_len);

	return -1;
}

/* support ZTE AC8710-AC2726-AC2736 to switch 3G mode */
int usb_stor_zte_init(struct us_data *us)
{
	struct bulk_cb_wrap *bcb = (struct bulk_cb_wrap*) us->iobuf;
	struct bulk_cs_wrap *bcs = (struct bulk_cs_wrap*) us->iobuf;
	int res, act_len;
	unsigned char buf[512];

	bcb->Signature = cpu_to_le32(US_BULK_CB_SIGN);
	bcb->Tag = 0;
	bcb->DataTransferLength = cpu_to_le32(0xc0);
	bcb->Flags = 0x80;	//data in
	bcb->Lun = 0;
	bcb->Length = 6;
	memset(bcb->CDB, 0, sizeof(bcb->CDB));
	bcb->CDB[0] = 0x9f;
	bcb->CDB[1] = 0x01;

	res = usb_stor_bulk_transfer_buf(us, us->send_bulk_pipe, bcb,
			US_BULK_CB_WRAP_LEN, &act_len);
	if(res)
		return res;

	res = usb_stor_bulk_transfer_buf(us, us->recv_bulk_pipe, buf,
			sizeof(buf), &act_len);

	if(!res && (act_len == US_BULK_CS_WRAP_LEN))
		return -2;
	
	res = usb_stor_bulk_transfer_buf(us, us->recv_bulk_pipe, bcs,
			US_BULK_CS_WRAP_LEN, &act_len);

	return -1;
}

/* support ZTE MU350 AC560-AC580 to switch 3G mode */
int usb_stor_zte_init2(struct us_data *us)
{
	struct bulk_cb_wrap *bcb = (struct bulk_cb_wrap*) us->iobuf;
	struct bulk_cs_wrap *bcs = (struct bulk_cs_wrap*) us->iobuf;
	int res, act_len;
	unsigned char buf[512];

	bcb->Signature = cpu_to_le32(US_BULK_CB_SIGN);
	bcb->Tag = 0;
	bcb->DataTransferLength = cpu_to_le32(0x20);
	bcb->Flags = 0x80;	//data in
	bcb->Lun = 0;
	bcb->Length = 0x0c;
	memset(bcb->CDB, 0, sizeof(bcb->CDB));
	bcb->CDB[0] = 0x85;
	bcb->CDB[1] = bcb->CDB[2] = bcb->CDB[3] = bcb->CDB[5] = bcb->CDB[6] = bcb->CDB[7] = bcb->CDB[8] = bcb->CDB[9] = 0x01;
	bcb->CDB[4] = 0x18;

	res = usb_stor_bulk_transfer_buf(us, us->send_bulk_pipe, bcb,
			US_BULK_CB_WRAP_LEN, &act_len);
	if(res)
		return res;

	res = usb_stor_bulk_transfer_buf(us, us->recv_bulk_pipe, buf,
			sizeof(buf), &act_len);

	if(!res && (act_len == US_BULK_CS_WRAP_LEN))
		return -2;

	res = usb_stor_bulk_transfer_buf(us, us->recv_bulk_pipe, bcs,
			US_BULK_CS_WRAP_LEN, &act_len);

	return -1;
}

/* support Qisda H21 to switch 3G mode */
int usb_stor_qisda_init(struct us_data *us)
{
	int result;
	char data[16] = {0};
	data[9] = 0x01;
	result = usb_stor_control_msg(us, us->send_ctrl_pipe,
				      0x04,
				      (0x02<<5),
				      0x00, 0x0, data, sizeof(data), 1000);
	US_DEBUGP("usb_control_msg performing result is %d\n", result);
	return (result ? 0 : -1);
}


/* support DLINK DWM-162-U5CDROM or SIMCOM SIM5250 to switch 3G mode */
int usb_stor_check_vendor_init(struct us_data *us)
{
	struct usb_device *dev = us->pusb_dev;
	struct bulk_cb_wrap *bcb = (struct bulk_cb_wrap*) us->iobuf;
	struct bulk_cs_wrap *bcs = (struct bulk_cs_wrap*) us->iobuf;
	int res, act_len;

	if(!strncmp(dev->manufacturer, "Alpha", strlen("Alpha"))) {		//Alpha, SIMCOM SIM5250
		bcb->Signature = cpu_to_le32(US_BULK_CB_SIGN);
		bcb->Tag = 0;
		bcb->DataTransferLength = cpu_to_le32(0);
		bcb->Flags = bcb->Lun = 0;
		bcb->Length = 6;
		memset(bcb->CDB, 0, sizeof(bcb->CDB));
		bcb->CDB[0] = 0xbd;
		bcb->CDB[4] = 0x02;

		res = usb_stor_bulk_transfer_buf(us, us->send_bulk_pipe, bcb,
				US_BULK_CB_WRAP_LEN, &act_len);
		if(res)
			return res;

		res = usb_stor_bulk_transfer_buf(us, us->recv_bulk_pipe, bcs,
				US_BULK_CS_WRAP_LEN, &act_len);

		return -1;
	}
	else if(!strncmp(dev->manufacturer, "Qualcomm", strlen("Qualcomm"))) {	//Qualcomm, DLINK DWM-162-U5CDROM
		return usb_stor_eject_media_init(us);
	}
	else {
		printk("%s %d, no matched vendor name!\n", __func__, __LINE__);
		return 0;
	}
}


