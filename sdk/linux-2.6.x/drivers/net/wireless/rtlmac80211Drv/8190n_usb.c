
#define R8190N_USB_C

#ifdef __KERNEL__
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/in.h>
#include <linux/if.h>
#include <linux/ip.h>
#include <asm/io.h>
#include <linux/skbuff.h>
#include <linux/socket.h>
#include <linux/fs.h>
#include <linux/major.h>
#include <linux/fcntl.h>
#include <linux/signal.h>
#include <asm/uaccess.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#endif

#include <linux/nl80211.h>
#include <net/mac80211.h>


#include "8190n_cfg.h"
#include "8190n.h"
#include "8190n_hw.h"
#include "8190n_headers.h"
#include "8190n_debug.h"

#include "farray.h"

#include "8190n_usb.h"



#define FWBUFF_ALIGN_SZ 4
#define MAX_DUMP_FWSZ	49152 /*default = 49152 (48k)*/

#define MAX_RECVBUF_SZ (32768) // 32k

int ffaddr2pipehdl(struct rtl8190_hw *pintfpriv, u32 addr)
{
	int pipe=0;
	struct usb_device *pusbd = pintfpriv->udev;
	
	if(pintfpriv->nr_endpoint == 11)
	{		
		switch(addr)
		{	    
			case RTL8712_DMA_BKQ:
			 	pipe=usb_sndbulkpipe(pusbd, 0x07);
				break;
	     		case RTL8712_DMA_BEQ:
		 		pipe=usb_sndbulkpipe(pusbd, 0x06);
				break;	     		
	     		case RTL8712_DMA_VIQ:
		 		pipe=usb_sndbulkpipe(pusbd, 0x05);
				break;
	    		case RTL8712_DMA_VOQ:
				pipe=usb_sndbulkpipe(pusbd, 0x04);
				break;					
                     case RTL8712_DMA_BCNQ:	
				pipe=usb_sndbulkpipe(pusbd, 0x0a);
				break;	 	
			case RTL8712_DMA_BMCQ:	//HI Queue
				pipe=usb_sndbulkpipe(pusbd, 0x0b);
				break;	
			case RTL8712_DMA_MGTQ:				
		 		pipe=usb_sndbulkpipe(pusbd, 0x0c);
				break;
                     case RTL8712_DMA_RX0FF:
				pipe=usb_rcvbulkpipe(pusbd, 0x03);//in
				break;	 	
			case RTL8712_DMA_C2HCMD:		 	
				pipe=usb_rcvbulkpipe(pusbd, 0x09);//in
				break;
			case RTL8712_DMA_H2CCMD:
				pipe=usb_sndbulkpipe(pusbd, 0x0d);
				break;	
				
		}

	}
	else if(pintfpriv->nr_endpoint == 6)
	{
		switch(addr)
		{	    
	     		case RTL8712_DMA_BKQ:
			 	pipe=usb_sndbulkpipe(pusbd, 0x07);
				break;
	     		case RTL8712_DMA_BEQ:
		 		pipe=usb_sndbulkpipe(pusbd, 0x06);
				break;	     		
	     		case RTL8712_DMA_VIQ:
		 		pipe=usb_sndbulkpipe(pusbd, 0x05);
				break;
	    		case RTL8712_DMA_VOQ:
				pipe=usb_sndbulkpipe(pusbd, 0x04);
				break;					
                    case RTL8712_DMA_RX0FF:
			case RTL8712_DMA_C2HCMD:		 	
				pipe=usb_rcvbulkpipe(pusbd, 0x03);//in
				break;
			case RTL8712_DMA_H2CCMD:
			case RTL8712_DMA_BCNQ:					
			case RTL8712_DMA_BMCQ:	
			case RTL8712_DMA_MGTQ:			
				pipe=usb_sndbulkpipe(pusbd, 0x0d);
				break;	
				
		}

	}
	else if(pintfpriv->nr_endpoint == 4)
	{
		switch(addr)
		{		       
	     		case RTL8712_DMA_BEQ:
		 	//case RTL8712_DMA_BKQ:
	 			//pipe=usb_sndbulkpipe(pusbd, 0x05);
			 	pipe=usb_sndbulkpipe(pusbd, 0x06);	
				break;			
	     		//case RTL8712_DMA_VIQ:
		 	case RTL8712_DMA_VOQ:					
		 		pipe=usb_sndbulkpipe(pusbd, 0x04);
				break;
			case RTL8712_DMA_RX0FF:
			case RTL8712_DMA_C2HCMD:		 	
				pipe=usb_rcvbulkpipe(pusbd, 0x03);//in
				break;
			case RTL8712_DMA_H2CCMD:	
			case RTL8712_DMA_BCNQ:					
			case RTL8712_DMA_BMCQ:	
			case RTL8712_DMA_MGTQ:				
				pipe=usb_sndbulkpipe(pusbd, 0x0d);
				break;	
		}
	
	}
	else
	{
	   	printk("ffaddr2pipehdl():nr_endpoint=%d error!\n", pintfpriv->nr_endpoint);
	   	pipe = 0;
	}
		
	return pipe;

}

u32 get_ff_hwaddr(struct rtl8190_hw *pintfpriv, unsigned int tid, unsigned int qsel)
{
	u32 addr = 0;

	if(pintfpriv->nr_endpoint == 6)
	{
		switch(tid)
		{
			case 0:
			case 3:
				addr = RTL8712_DMA_BEQ;
			 	break;
			case 1:
			case 2:
				addr = RTL8712_DMA_BKQ;
				break;				
			case 4:
			case 5:
				addr = RTL8712_DMA_VIQ;
				break;		
			case 6:
			case 7:
				addr = RTL8712_DMA_VOQ;
				break;	

			case 0x10:				
			case 0x11:				
			case 0x12:
			case 0x13://
				addr = RTL8712_DMA_H2CCMD;
				break;					

			default:
				addr = RTL8712_DMA_BEQ;
				break;		
				
		}
		
	}
	else if(pintfpriv->nr_endpoint == 4)
	{		
		switch(qsel)
		{
			case 0:
			case 3:				
			case 1:
			case 2:
				
				addr = RTL8712_DMA_BEQ;//RTL8712_EP_LO;
				
				break;		
			case 4:
			case 5:					
			case 6:
			case 7:

				addr = RTL8712_DMA_VOQ;//RTL8712_EP_HI;
				
				break;	
			
			case 0x10:				
			case 0x11:				
			case 0x12:	
			case 0x13://
				addr = RTL8712_DMA_H2CCMD;;
				break;
			default:
				addr = RTL8712_DMA_BEQ;//RTL8712_EP_LO;
				break;		
				
		}
	

	}

	return addr;
	
}

int usb_configure(struct priv_shared_hw *pshared_hw)
{
	int i;	
	
	struct	usb_device_descriptor 		*pdev_desc;

	struct	usb_host_config			*phost_conf;
	struct	usb_config_descriptor 		*pconf_desc;

	struct	usb_host_interface			*phost_iface;
	struct	usb_interface_descriptor	*piface_desc;
	
	struct	usb_host_endpoint			*phost_endp;
	struct	usb_endpoint_descriptor	*pendp_desc;	

	struct rtl8190_hw *phw_intf = pshared_hw->phw;
	struct usb_device *pusbd = pshared_hw->udev;	
	struct usb_interface *puintf = phw_intf->pusb_intf;
	
	
	pdev_desc = &pusbd->descriptor;
	
#if 0
	printk("\n8192s_usb_device_descriptor:\n");
	printk("bLength=%x\n", pdev_desc->bLength);
	printk("bDescriptorType=%x\n", pdev_desc->bDescriptorType);
	printk("bcdUSB=%x\n", pdev_desc->bcdUSB);
	printk("bDeviceClass=%x\n", pdev_desc->bDeviceClass);
	printk("bDeviceSubClass=%x\n", pdev_desc->bDeviceSubClass);
	printk("bDeviceProtocol=%x\n", pdev_desc->bDeviceProtocol);
	printk("bMaxPacketSize0=%x\n", pdev_desc->bMaxPacketSize0);
	printk("idVendor=%x\n", pdev_desc->idVendor);
	printk("idProduct=%x\n", pdev_desc->idProduct);
	printk("bcdDevice=%x\n", pdev_desc->bcdDevice);	
	printk("iManufacturer=%x\n", pdev_desc->iManufacturer);
	printk("iProduct=%x\n", pdev_desc->iProduct);
	printk("iSerialNumber=%x\n", pdev_desc->iSerialNumber);
	printk("bNumConfigurations=%x\n", pdev_desc->bNumConfigurations);
#endif	

	phost_conf = pusbd->actconfig;
	pconf_desc = &phost_conf->desc;

#if 0	
	printk("\n8192s_usb_configuration_descriptor:\n");
	printk("bLength=%x\n", pconf_desc->bLength);
	printk("bDescriptorType=%x\n", pconf_desc->bDescriptorType);
	printk("wTotalLength=%x\n", pconf_desc->wTotalLength);
	printk("bNumInterfaces=%x\n", pconf_desc->bNumInterfaces);
	printk("bConfigurationValue=%x\n", pconf_desc->bConfigurationValue);
	printk("iConfiguration=%x\n", pconf_desc->iConfiguration);
	printk("bmAttributes=%x\n", pconf_desc->bmAttributes);
	printk("bMaxPower=%x\n", pconf_desc->bMaxPower);
#endif
	
	//printk("\n/****** num of altsetting = (%d) ******/\n", pintf->num_altsetting);
		
	phost_iface = &puintf->altsetting[0];
	piface_desc = &phost_iface->desc;

#if 0
	printk("\n8192s_usb_interface_descriptor:\n");
	printk("bLength=%x\n", piface_desc->bLength);
	printk("bDescriptorType=%x\n", piface_desc->bDescriptorType);
	printk("bInterfaceNumber=%x\n", piface_desc->bInterfaceNumber);
	printk("bAlternateSetting=%x\n", piface_desc->bAlternateSetting);
	printk("bNumEndpoints=%x\n", piface_desc->bNumEndpoints);
	printk("bInterfaceClass=%x\n", piface_desc->bInterfaceClass);
	printk("bInterfaceSubClass=%x\n", piface_desc->bInterfaceSubClass);
	printk("bInterfaceProtocol=%x\n", piface_desc->bInterfaceProtocol);
	printk("iInterface=%x\n", piface_desc->iInterface);	
#endif
	
	phw_intf->nr_endpoint = piface_desc->bNumEndpoints;
	

	//printk("\ndump 8192s_usb_endpoint_descriptor:\n");

	for(i=0; i<phw_intf->nr_endpoint; i++)
	{
		phost_endp = phost_iface->endpoint+i;
		if(phost_endp)
		{
			pendp_desc = &phost_endp->desc;
		
			printk("\n8192s_usb_endpoint_descriptor(%d):\n", i);
			printk("bLength=%x\n",pendp_desc->bLength);
			printk("bDescriptorType=%x\n",pendp_desc->bDescriptorType);
			printk("bEndpointAddress=%x\n",pendp_desc->bEndpointAddress);
			//printk("bmAttributes=%x\n",pendp_desc->bmAttributes);
			//printk("wMaxPacketSize=%x\n",pendp_desc->wMaxPacketSize);
			printk("wMaxPacketSize=%x\n",le16_to_cpu(pendp_desc->wMaxPacketSize));
			printk("bInterval=%x\n",pendp_desc->bInterval);
			//printk("bRefresh=%x\n",pendp_desc->bRefresh);
			//printk("bSynchAddress=%x\n",pendp_desc->bSynchAddress);	
		}	

	}

	printk("\n");
	
	if (pusbd->speed==USB_SPEED_HIGH)
	{
              phw_intf->ishighspeed = _TRUE;
              printk("8192su : USB_SPEED_HIGH\n");
	}
	else
	{
		phw_intf->ishighspeed = _FALSE;
		printk("8192s: NON USB_SPEED_HIGH\n");
	}
	  	

	printk("8192su:nr_endpoint=%d\n", phw_intf->nr_endpoint);	  	
	

	return 0;
	
}

static void usbctrl_vendorreq_async_callback(struct urb *urb)
{
	if(urb)
	{
		if(urb->context)
			kfree(urb->context);

		usb_free_urb(urb);		
	}	
}
int usbctrl_vendorreq_async_write(struct usb_device *udev, u8 request, u16 value, u16 index, void *pdata, u16 len, u8 requesttype)
{
	int rc;
	unsigned int pipe;	
	u8 reqtype;
	struct usb_ctrlrequest *dr;
	struct urb *urb;
	struct rtl819x_async_write_data {
		u8 data[4];
		struct usb_ctrlrequest dr;
	} *buf;
	
				
	if (requesttype == 0x01)
	{
		pipe = usb_rcvctrlpipe(udev, 0);//read_in
		reqtype =  RTL871X_VENQT_READ;		
	} 
	else 
	{
		pipe = usb_sndctrlpipe(udev, 0);//write_out
		reqtype =  RTL871X_VENQT_WRITE;		
	}		
	
	buf = kmalloc(sizeof(*buf), GFP_ATOMIC);
	if (!buf)
		return -ENOMEM;

	urb = usb_alloc_urb(0, GFP_ATOMIC);
	if (!urb) {
		kfree(buf);
		return -ENOMEM;
	}

	dr = &buf->dr;

	dr->bRequestType = reqtype;
	dr->bRequest = request;
	dr->wValue = cpu_to_le16(value);
	dr->wIndex = cpu_to_le16(index);
	dr->wLength = cpu_to_le16(len);

	memcpy(buf, pdata, len);

	usb_fill_control_urb(urb, udev, pipe,
			     (unsigned char *)dr, buf, len,
			     usbctrl_vendorreq_async_callback, buf);
	
	//usb_anchor_urb(urb, &priv->anchored);
	
	rc = usb_submit_urb(urb, GFP_ATOMIC);
	if (rc < 0) {
		kfree(buf);
		//usb_unanchor_urb(urb);
	}
	
	//usb_free_urb(urb);
	
	return rc;

}


int usbctrl_vendorreq(struct usb_device *udev, u8 request, u16 value, u16 index, void *pdata, u16 len, u8 requesttype)
{
	unsigned int pipe;
	int status;
	u8 reqtype;
				
	if (requesttype == 0x01)
	{
		pipe = usb_rcvctrlpipe(udev, 0);//read_in
		reqtype =  RTL871X_VENQT_READ;		
	} 
	else 
	{
		pipe = usb_sndctrlpipe(udev, 0);//write_out
		reqtype =  RTL871X_VENQT_WRITE;		
	}		
	
	status = usb_control_msg(udev, pipe, request, reqtype, value, index, pdata, len, HZ/2);
	
	if (status < 0)
       {
		printk(KERN_ERR "reg 0x%x, usbctrl_vendorreq TimeOut! status:0x%x value=0x%x\n", value, status, *(u32*)pdata);		
       }

	return status;

}

u8 usb_read8(struct priv_shared_hw *pshared_hw, u32 addr)
{
	u8 request;
	u8 requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	u32 data;
	
	request = 0x05;
	requesttype = 0x01;//read_in
	index = 0;//n/a

	wvalue = (u16)(addr&0x0000ffff);
	len = 1;	
	
	usbctrl_vendorreq(pshared_hw->udev, request, wvalue, index, &data, len, requesttype);

	return (u8)(le32_to_cpu(data)&0x0ff);
		
}
u16 usb_read16(struct priv_shared_hw *pshared_hw, u32 addr)
{       
	u8 request;
	u8 requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	u32 data;
	
	request = 0x05;
	requesttype = 0x01;//read_in
	index = 0;//n/a

	wvalue = (u16)(addr&0x0000ffff);
	len = 2;	
	
	usbctrl_vendorreq(pshared_hw->udev, request, wvalue, index, &data, len, requesttype);

	return (u16)(le32_to_cpu(data)&0xffff);
	
}
u32 usb_read32(struct priv_shared_hw *pshared_hw, u32 addr)
{
	u8 request;
	u8 requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	u32 data;	

	request = 0x05;
	requesttype = 0x01;//read_in
	index = 0;//n/a

	wvalue = (u16)(addr&0x0000ffff);
	len = 4;	
	
	usbctrl_vendorreq(pshared_hw->udev, request, wvalue, index, &data, len, requesttype);

	return le32_to_cpu(data);
	
}
void usb_write8(struct priv_shared_hw *pshared_hw, u32 addr, u8 val)
{
	u8 request;
	u8 requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	u32 data;
	
	request = 0x05;
	requesttype = 0x00;//write_out
	index = 0;//n/a

	wvalue = (u16)(addr&0x0000ffff);
	len = 1;
	
	data = val;
	data = cpu_to_le32(data&0x000000ff);
	
	usbctrl_vendorreq(pshared_hw->udev, request, wvalue, index, &data, len, requesttype);
		
}
void usb_write16(struct priv_shared_hw *pshared_hw, u32 addr, u16 val)
{	
	u8 request;
	u8 requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	u32 data;
	
	request = 0x05;
	requesttype = 0x00;//write_out
	index = 0;//n/a

	wvalue = (u16)(addr&0x0000ffff);
	len = 2;
	
	data = val;
	data = cpu_to_le32(data&0x0000ffff);
	
	usbctrl_vendorreq(pshared_hw->udev, request, wvalue, index, &data, len, requesttype);
		
}

void usb_write32(struct priv_shared_hw *pshared_hw, u32 addr, u32 val)
{
	u8 request;
	u8 requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	u32 data;
	
	request = 0x05;
	requesttype = 0x00;//write_out
	index = 0;//n/a

	wvalue = (u16)(addr&0x0000ffff);
	len = 4;
	data = cpu_to_le32(val);	
	
	usbctrl_vendorreq(pshared_hw->udev, request, wvalue, index, &data, len, requesttype);
		
}
void usb_write8_async(struct priv_shared_hw *pshared_hw, u32 addr, u8 val)
{
	u8 request;
	u8 requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	u32 data;
	
	request = 0x05;
	requesttype = 0x00;//write_out
	index = 0;//n/a

	wvalue = (u16)(addr&0x0000ffff);
	len = 1;
	
	data = val;
	data = cpu_to_le32(data&0x000000ff);
	
	usbctrl_vendorreq_async_write(pshared_hw->udev, request, wvalue, index, &data, len, requesttype);
		
}
void usb_write16_async(struct priv_shared_hw *pshared_hw, u32 addr, u16 val)
{	
	u8 request;
	u8 requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	u32 data;
	
	request = 0x05;
	requesttype = 0x00;//write_out
	index = 0;//n/a

	wvalue = (u16)(addr&0x0000ffff);
	len = 2;
	
	data = val;
	data = cpu_to_le32(data&0x0000ffff);
	
	usbctrl_vendorreq_async_write(pshared_hw->udev, request, wvalue, index, &data, len, requesttype);
		
}
void usb_write32_async(struct priv_shared_hw *pshared_hw, u32 addr, u32 val)
{
	u8 request;
	u8 requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	u32 data;
	
	request = 0x05;
	requesttype = 0x00;//write_out
	index = 0;//n/a

	wvalue = (u16)(addr&0x0000ffff);
	len = 4;
	data = cpu_to_le32(val);	
	
	usbctrl_vendorreq_async_write(pshared_hw->udev, request, wvalue, index, &data, len, requesttype);

}

static void usb_bulkout_zero_complete(struct urb *purb)
{	
	unsigned char *pbuf = (unsigned char *)purb->context;

	//printk("+usb_bulkout_zero_complete\n");
	
	if(pbuf)
	{		
		kfree(pbuf);
	}

	if(purb)
	{
		usb_free_urb(purb);		
	}
	
}

static void usb_bulkout_zero(struct priv_shared_hw *pshared_hw, u32 addr)
{	
	int pipe, status, len;	
	unsigned char *pbuf;	
	struct urb *purb;		
	struct usb_device *pusbd = pshared_hw->udev;
	struct rtl8190_hw *phw_intf = pshared_hw->phw;

	//printk("+usb_bulkout_zero\n");
		
	pbuf = (unsigned char *)kmalloc(sizeof(int), GFP_ATOMIC);	
    	purb = usb_alloc_urb(0, GFP_ATOMIC);
      	
	len = 0;
		
	//translate DMA FIFO addr to pipehandle
	pipe = ffaddr2pipehdl(phw_intf, addr);	

	usb_fill_bulk_urb(purb, pusbd, pipe, 
       				pbuf, len,
              			usb_bulkout_zero_complete,
              			pbuf);

	status = usb_submit_urb(purb, GFP_ATOMIC);

	if (!status)
	{		
		
	}
	else
	{
	
	}

}
static void usb_write_mem_complete(struct urb *purb)
{	
	struct completion *completed = (struct completion *)purb->context;

	printk("%s\n", __FUNCTION__);
		
	if(purb->status==0)
	{
		
	}
	else
	{
		printk("usb_write_mem_complete : purb->status(%d) != 0 \n", purb->status);
		
		if(purb->status == (-ESHUTDOWN))
		{
			printk("usb_write_mem_complete: ESHUTDOWN\n");			
		}
		else
		{			
			
		}
	}

	complete(completed);

}
static void usb_write_mem(struct priv_shared_hw *pshared_hw, u32 addr, u32 sz, u8 *pmem)
{	
	int pipe, status;
	u8 bwritezero;	
	struct urb *purb;
	struct usb_device *pusbd = pshared_hw->udev;
	struct rtl8190_hw *phw_intf = pshared_hw->phw;
		
	DECLARE_COMPLETION_ONSTACK(complete);	
	
	purb = usb_alloc_urb(0, GFP_ATOMIC);
	if(!purb)
	{
		return;
	}
	
	//translate DMA FIFO addr to pipehandle
	pipe = ffaddr2pipehdl(phw_intf, addr);
	if(pipe==0)
	{	   	
		return;
	}


	bwritezero = _FALSE;
	if(phw_intf->ishighspeed)
	{
		if(sz> 0 && sz%512 == 0)
		{			
			bwritezero = _TRUE;//printk("ishighspeed, cnt=%d\n", cnt);
		}	
	}
	else
	{
		if(sz > 0 && sz%64 == 0)
		{			
			bwritezero = _TRUE;//printk("cnt=%d\n", cnt);	
		}	
	}	

	
	usb_fill_bulk_urb(purb, pusbd, pipe, 
       			    	pmem, sz,
              		    	usb_write_mem_complete,
              		    	&complete);
	

	status = usb_submit_urb(purb, GFP_ATOMIC);
	
	if (!status)
	{		
		
	}
	else
	{
		printk("usb_write_mem(): usb_submit_urb err, status=%x\n", status);
	}

	if(bwritezero == _TRUE)
	{
		usb_bulkout_zero(pshared_hw, addr);
	}

	wait_for_completion(&complete);	

}

u8 read8(struct mac80211_shared_priv *priv, u32 addr)
{
	u8 r_val;	
	struct priv_shared_hw *pshared_hw = (struct priv_shared_hw *)priv->pshare_hw;

	r_val = usb_read8(pshared_hw, addr);
	
	
	return r_val;
}

u16 read16(struct mac80211_shared_priv *priv, u32 addr)
{
	u16 r_val;
	struct priv_shared_hw *pshared_hw = (struct priv_shared_hw *)priv->pshare_hw;
	

	r_val = usb_read16(pshared_hw, addr);
	
		
	return r_val;
}
	
u32 read32(struct mac80211_shared_priv *priv, u32 addr)
{
	u32 r_val;
	struct priv_shared_hw *pshared_hw = (struct priv_shared_hw *)priv->pshare_hw;

	
	r_val = usb_read32(pshared_hw, addr);
	
	
	return r_val;	
}

void write8(struct mac80211_shared_priv *priv, u32 addr, u8 val)
{
	struct priv_shared_hw *pshared_hw = (struct priv_shared_hw *)priv->pshare_hw;
	

	usb_write8(pshared_hw, addr, val);
	
}

void write16(struct mac80211_shared_priv *priv, u32 addr, u16 val)
{
	struct priv_shared_hw *pshared_hw = (struct priv_shared_hw *)priv->pshare_hw;
	

	usb_write16(pshared_hw, addr, val);

}

void write32(struct mac80211_shared_priv *priv, u32 addr, u32 val)
{
	struct priv_shared_hw *pshared_hw = (struct priv_shared_hw *)priv->pshare_hw;
	

	usb_write32(pshared_hw, addr, val);

}

void write8_async(struct mac80211_shared_priv *priv, u32 addr, u8 val)
{
	struct priv_shared_hw *pshared_hw = (struct priv_shared_hw *)priv->pshare_hw;
	

	usb_write8_async(pshared_hw, addr, val);
	
}

void write16_async(struct mac80211_shared_priv *priv, u32 addr, u16 val)
{
	struct priv_shared_hw *pshared_hw = (struct priv_shared_hw *)priv->pshare_hw;
	

	usb_write16_async(pshared_hw, addr, val);

}

void write32_async(struct mac80211_shared_priv *priv, u32 addr, u32 val)
{
	struct priv_shared_hw *pshared_hw = (struct priv_shared_hw *)priv->pshare_hw;
	

	usb_write32_async(pshared_hw, addr, val);

}

static void write_mem(struct mac80211_shared_priv *priv, u32 addr, u32 sz, u8 *pmem)
{	
	struct priv_shared_hw *pshared_hw = (struct priv_shared_hw *)priv->pshare_hw;
	
	usb_write_mem(pshared_hw, addr, sz, pmem);
}

unsigned int phy_QueryUsbBBReg(struct mac80211_shared_priv *priv, unsigned int RegAddr)
{	
	u32	ReturnValue = 0xffffffff;
	u8	PollingCnt = 50;
	u8	BBWaitCounter = 0;
	struct priv_shared_hw *pshared_hw = (struct priv_shared_hw *)priv->pshare_hw;
	struct rtl8190_hw *pintfpriv = pshared_hw->phw;	

#define HST_RDBUSY BIT(0)

	//
	// <Roger_Notes> Due to PASSIVE_LEVEL, so we ONLY simply busy waiting for a while here.
	// We have to make sure that previous BB I/O has been done. 
	// 2008.08.20.
	//
	while(pintfpriv->bChangeBBInProgress)
	{
		BBWaitCounter ++;
		//printk("phy_QueryUsbBBReg(): Wait 1 ms (%d times)...\n", BBWaitCounter);
		msleep(1); // 1 ms

		// Wait too long, return false to avoid to be stuck here.
		if((BBWaitCounter > 100) )
		{
			printk("phy_QueryUsbBBReg(): (%d) Wait too logn to query BB!!\n", BBWaitCounter);
			return ReturnValue;
		}
	}

	pintfpriv->bChangeBBInProgress = _TRUE;
	
	RTL_R32(RegAddr);	

	do
	{// Make sure that access could be done.
		if((RTL_R8(PHY_REG)&HST_RDBUSY) == 0)
			break;
	}while( --PollingCnt );

	if(PollingCnt == 0)
	{
		printk("Fail!!!phy_QueryUsbBBReg(): RegAddr(%#x) = %#x\n", RegAddr, ReturnValue);
	}
	else
	{
		// Data FW read back.
		ReturnValue = RTL_R32(PHY_REG_DATA);	
		//printk("phy_QueryUsbBBReg(): RegAddr(%#x) = %#x, PollingCnt(%d)\n", RegAddr, ReturnValue, PollingCnt);
	}
	
	pintfpriv->bChangeBBInProgress = _FALSE;

	return ReturnValue;
	
}


void phy_SetUsbBBReg(struct mac80211_shared_priv *priv, u32 RegAddr, u32 Data)
{	
	u8	BBWaitCounter = 0;
	struct priv_shared_hw *pshared_hw = (struct priv_shared_hw *)priv->pshare_hw;
	struct rtl8190_hw *pintfpriv = pshared_hw->phw;	

	//
	// <Roger_Notes> Due to PASSIVE_LEVEL, so we ONLY simply busy waiting for a while here.
	// We have to make sure that previous BB I/O has been done. 
	// 2008.08.20.
	//
	while(pintfpriv->bChangeBBInProgress)
	{
		BBWaitCounter ++;
		//printk("phy_SetUsbBBReg(): Wait 1 ms (%d times)...\n", BBWaitCounter);
		msleep(1); // 1 ms
	
		if((BBWaitCounter > 100))// || RT_USB_CANNOT_IO(Adapter))
		{
			printk("phy_SetUsbBBReg(): (%d) Wait too logn to query BB!!\n", BBWaitCounter);
			return;
		}
	}

	pintfpriv->bChangeBBInProgress = _TRUE;
	
	//printk("**************%s: RegAddr:%x Data:%x\n", __FUNCTION__,RegAddr, Data);	
	RTL_W32(RegAddr, Data);
	
	pintfpriv->bChangeBBInProgress = _FALSE;
	
}

unsigned int phy_QueryUsbRFReg(struct mac80211_shared_priv *priv, RF90_RADIO_PATH_E eRFPath, unsigned int Offset)
{	
	u32	ReturnValue = 0;	
	u8	PollingCnt = 50;
	u8	RFWaitCounter = 0;
	struct priv_shared_hw *pshared_hw = (struct priv_shared_hw *)priv->pshare_hw;
	struct rtl8190_hw *pintfpriv = pshared_hw->phw;	

	//
	// <Roger_Notes> Due to PASSIVE_LEVEL, so we ONLY simply busy waiting for a while here.
	// We have to make sure that previous RF I/O has been done. 
	// 2008.08.20.
	//
	
	while(pintfpriv->bChangeRFInProgress)
	{
		//down(&priv->rf_sem);

		RFWaitCounter ++;
		
		msleep(1); // 1 ms

		if((RFWaitCounter > 100)) 
		{			
			printk("phy_QueryUsbRFReg(): (%d) Wait too logn to query BB!!\n", RFWaitCounter);
			return 0xffffffff;
		}
		else
		{
			
		}
	}

	pintfpriv->bChangeRFInProgress = _TRUE;
	
	Offset &= 0x3f; //RF_Offset= 0x00~0x3F		

	//RF_Offset= 0x00~0x3F, //RF_Path = 0(A) or 1(B)
	RTL_W32(RF_BB_CMD_ADDR, 0xF0000002|(Offset<<8)|	(eRFPath<<16));	
								
	
	do
	{
		// Make sure that access could be done.
		if(RTL_R32(RF_BB_CMD_ADDR) == 0)
			break;
		
	}while( --PollingCnt );

	if(PollingCnt == 0)
	{		
		printk("phy_QueryUsbRFReg(): Query RegAddr(%#x) Fail!!!\n", Offset);
	}

	// Data FW read back.
	ReturnValue = RTL_R32(RF_BB_CMD_DATA);
	
	//up(&priv->rf_sem);
	
	pintfpriv->bChangeRFInProgress = _FALSE;
	
	return ReturnValue;

}

void phy_SetUsbRFReg(struct mac80211_shared_priv *priv, RF90_RADIO_PATH_E eRFPath, unsigned int RegAddr, unsigned int Data)
{	
	u8	PollingCnt = 50;
	u8	RFWaitCounter = 0;
	struct priv_shared_hw *pshared_hw = (struct priv_shared_hw *)priv->pshare_hw;
	struct rtl8190_hw *pintfpriv = pshared_hw->phw;	

	//
	// <Roger_Notes> Due to PASSIVE_LEVEL, so we ONLY simply busy waiting for a while here.
	// We have to make sure that previous BB I/O has been done. 
	// 2008.08.20.
	//
	
	while(pintfpriv->bChangeRFInProgress)
	{
		//down(&priv->rf_sem);

		RFWaitCounter ++;
		
		msleep(1); // 1 ms
	
		if((RFWaitCounter > 100))
		{			
			printk("phy_SetUsbRFReg(): (%d) Wait too logn to query BB!!\n", RFWaitCounter);
			return;
		}
		else
		{
			
		}
	}

	pintfpriv->bChangeRFInProgress = _TRUE;	

	
	RegAddr &= 0x3f; //RF_Offset= 0x00~0x3F
	
	RTL_W32(RF_BB_CMD_DATA, Data);	
	
	//RF_Offset= 0x00~0x3F, //RF_Path = 0(A) or 1(B)
	RTL_W32(RF_BB_CMD_ADDR, 0xF0000003|(RegAddr<<8)| (eRFPath<<16));  
	
	do
	{
		// Make sure that access could be done.
		if(RTL_R32(RF_BB_CMD_ADDR) == 0)
				break;
		
	}while( --PollingCnt );		

	if(PollingCnt == 0)
	{		
		printk("phy_SetUsbRFReg(): Set RegAddr(%#x) = %#x Fail!!!\n", RegAddr, Data);
	}
	
	//up(&priv->rf_sem);
	
	pintfpriv->bChangeRFInProgress = _FALSE;
	
}

size_t rtl8192s_open_fw(struct mac80211_shared_priv *priv, void **pphfwfile_hdl, u8 **ppmappedfw)
{
	size_t len;	
#if 1
	*ppmappedfw = f_array;
	len = sizeof(f_array);	
#else
	*ppmappedfw = Rtl819XFwImageArray;
	len = sizeof(Rtl819XFwImageArray);	
#endif

		   
	   
 	return len;
}

void rtl8192s_close_fw(struct mac80211_shared_priv *priv, void *phfwfile_hdl)
{

}

void fill_fwpriv(struct mac80211_shared_priv *priv, struct fw_priv *pfwpriv)
{
	struct priv_shared_hw *pshared_hw = (struct priv_shared_hw *)priv->pshare_hw;
	struct rtl8190_hw *pintfpriv = pshared_hw->phw;
	

enum RTL8712_RF_CONFIG{
	RTL8712_RF_1T1R,
	RTL8712_RF_1T2R,
	RTL8712_RF_2T2R
};

enum _RTL8712_HCI_TYPE_{
 RTL8712_HCI_TYPE_PCIE=0x01,
 RTL8712_HCI_TYPE_AP_PCIE=0x81,  
 RTL8712_HCI_TYPE_USB=0x02,
 RTL8712_HCI_TYPE_92USB=0x02,
 RTL8712_HCI_TYPE_AP_USB=0x82,   
 RTL8712_HCI_TYPE_72USB=0x12,
 RTL8712_HCI_TYPE_SDIO=0x04,
 RTL8712_HCI_TYPE_72SDIO=0x14
};

enum _RTL8712_RF_MIMO_CONFIG_{
 RTL8712_RFCONFIG_1T=0x10,
 RTL8712_RFCONFIG_2T=0x20,
 RTL8712_RFCONFIG_1R=0x01,
 RTL8712_RFCONFIG_2R=0x02,
 RTL8712_RFCONFIG_1T1R=0x11,
 RTL8712_RFCONFIG_1T2R=0x12,
 RTL8712_RFCONFIG_TURBO=0x92,
 RTL8712_RFCONFIG_2T2R=0x22
};
	
	memset(pfwpriv, 0, sizeof(struct fw_priv));

	pfwpriv->hci_sel =  RTL8712_HCI_TYPE_72USB;
		
	pfwpriv->usb_ep_num = (u8)pintfpriv->nr_endpoint;
	
	pfwpriv->bw_40MHz_en = 1;
	
	pfwpriv->rf_config = RTL8712_RFCONFIG_1T2R;

	pfwpriv->mp_mode =  0 ;

	pfwpriv->vcsType = 2; /* 0:off 1:on 2:auto */
	
	pfwpriv->vcsMode = 2; /* 1:RTS/CTS 2:CTS to self */

	pfwpriv->turboMode = 1;//default enable it
	
}

void update_fwhdr(struct fw_hdr *pfwhdr, u8* pmappedfw)
{
	pfwhdr->signature = le16_to_cpu(*(u16 *)pmappedfw);
	pfwhdr->version = le16_to_cpu(*(u16 *)(pmappedfw+2));
	
	pfwhdr->dmem_size = le32_to_cpu(*(uint *)(pmappedfw+4)); //define the size of boot loader
	
	pfwhdr->img_IMEM_size = le32_to_cpu(*(uint *)(pmappedfw+8)); //define the size of FW in IMEM
	
	pfwhdr->img_SRAM_size = le32_to_cpu(*(uint *)(pmappedfw+12));//define the size of FW in SRAM
	
	pfwhdr->fw_priv_sz = le32_to_cpu(*(uint *)(pmappedfw+16)); //define the size of DMEM variable 
	

	printk("update_fwhdr:sig=%x;ver=%x;dmem_size=%d;IMEMsz=%d;SRAMsz=%d;fwprivsz=%d;struct_fwprivsz=%d\n",
			pfwhdr->signature, pfwhdr->version,pfwhdr->dmem_size,pfwhdr->img_IMEM_size,pfwhdr->img_SRAM_size, pfwhdr->fw_priv_sz, sizeof(struct fw_priv));

}

int chk_fwhdr(struct fw_hdr *pfwhdr, u32 ulfilelength)
{
	u32	fwhdrsz, fw_sz;
	u8 intf, rfconf;

	//check signature
	if ((pfwhdr->signature != 0x8712) && (pfwhdr->signature != 0x8192))
	{
		printk("Signature does not match (Signature %x != 8712)! Issue complaints for fw coder\n", pfwhdr->signature);
		return -EINVAL;
	}

	//check fw_version
	printk("FW_VER=%X\n", pfwhdr->version&0x0FFF);
	
	//check interface
	intf = (u8)((pfwhdr->version&0x3000) >> 12);
	printk("Interface=%X\n", intf);

	//check rf_conf
	rfconf = (u8)((pfwhdr->version&0xC000) >> 14);
	printk("chk_fwhdr RF_Configure=%X\n", rfconf);

	//check fw_priv_sze & sizeof(struct fw_priv)
	if(pfwhdr->fw_priv_sz != sizeof(struct fw_priv))
	{
		printk("fw_priv size mismatch between fw(%d) and driver(%d)\n", pfwhdr->fw_priv_sz, sizeof(struct fw_priv));
		return -EINVAL;
	}
	

	//check fw_sz & image_fw_sz
	fwhdrsz = FIELD_OFFSET(struct fw_hdr, fwpriv) + pfwhdr->fw_priv_sz;	
	fw_sz =  fwhdrsz + pfwhdr->img_IMEM_size +pfwhdr->img_SRAM_size + pfwhdr->dmem_size;	
	if (fw_sz != ulfilelength)
	{			
		printk("FW image size dismatch! fw_sz=%d != image_fw_sz = %d!\n", fw_sz, ulfilelength);
		return -EINVAL;
	}

	return 0;
	
}

void write_fwimg(struct mac80211_shared_priv *priv, void *pbuf, void *src, size_t sz, int type)
{
	size_t w_sz, max_wsz;	
	struct rtl8192s_txdesc *pdesc = (struct rtl8192s_txdesc *)pbuf;
	size_t txdesc_sz = sizeof(struct rtl8192s_txdesc);
	u8 *pdata = (u8 *)pbuf + txdesc_sz;
	
	switch(type)
	{
		case LOAD_IMEM:
			max_wsz = MAX_DUMP_FWSZ;	
			break;
		case LOAD_EMEM:
			max_wsz = MAX_DUMP_FWSZ;
			break;
		case LOAD_DMEM:
			max_wsz = MAX_DUMP_FWSZ;
			break;
		default :
			return;			
	}		

	do{

		memset(pbuf, 0, txdesc_sz);
		
		if(sz >  max_wsz)
		{
			w_sz = max_wsz;
		}
		else
		{
			w_sz = sz;
			pdesc->txdw0 |= cpu_to_le32(BIT(28));				
		}	

		pdesc->txdw0 |= cpu_to_le32(w_sz&0x0000ffff); 

		memcpy(pdata, src, w_sz);		
		
		write_mem(priv, RTL8712_DMA_VOQ, w_sz+txdesc_sz, (u8*)pbuf);		

		src += w_sz;	
			
		sz -= w_sz;		
		
	}while(sz>0);

}

int rtl8192SU_Write_FWIMG(struct mac80211_shared_priv *priv, void *src, size_t sz, int type)
{	
	struct fw_priv fwpriv;
	u8 *pbuf_allocated = NULL, *pbuf;

	
	
	pbuf_allocated = (u8*)kmalloc(sz+ TXDESC_SIZE + FWBUFF_ALIGN_SZ, GFP_KERNEL);		
	if(pbuf_allocated==NULL)
	{
		printk("can't alloc resources when dl_fw\n");
		return _FALSE;		
	}
			
	pbuf = (pbuf_allocated + FWBUFF_ALIGN_SZ - ((u32 )(pbuf_allocated )&(FWBUFF_ALIGN_SZ-1)));

	if(type==LOAD_DMEM)
	{
		fill_fwpriv(priv,  &fwpriv);
		
		write_fwimg(priv, pbuf, &fwpriv, sz, type);
	}
	else		
	{
		if(src)
			write_fwimg(priv, pbuf, src, sz, type);
	}

	
	if(pbuf_allocated)
		kfree(pbuf_allocated);

	return _TRUE;

}

int check_img_ready(struct mac80211_shared_priv *priv, int type)
{
	switch(type)
	{
		case LOAD_IMEM:
			
			break;
		case LOAD_EMEM:
			
			break;
		case LOAD_DMEM:
			
			break;
		default :
			return -EINVAL;			
	}

	return 0;

}

int load_imem_img(struct mac80211_shared_priv *priv, void *pbuf, void *src, size_t sz)
{
	int ret, i = 10;
	u16 tmp16;
	
	
	write_fwimg(priv, pbuf, src, sz, LOAD_IMEM);
		
	ret = check_img_ready(priv, LOAD_IMEM);

	//check if imem ready	
	tmp16=read16(priv,TCR);
	printk("TCR val = %x\n", tmp16);
	while(((tmp16 & IMEM_CODE_DONE)==0) && (i>0))
	{		
		udelay(10);//delay
		
		tmp16=read16(priv,TCR);
		
		printk("TCR val = %x\n", tmp16);
		
		i--;
	}
	if(i==0)
	{
		printk("Error => Pollin _IMEM_CODE_DONE Fail\n");
		return -ETIMEDOUT;
	}
		
	if((tmp16 & IMEM_CHK_RPT) == 0) 
	{
		printk("_IMEM_CHK_RPT = 0\n");
		return -ETIMEDOUT;
	}

	return ret;
	
}
int load_emem_img(struct mac80211_shared_priv *priv, void *pbuf, void *src, size_t sz)
{
	int ret, i = 5;
	u16 tmp16;
	

	write_fwimg(priv, pbuf, src, sz, LOAD_EMEM);

	ret = check_img_ready(priv, LOAD_EMEM);

	//check if emem ready	
	tmp16=read16(priv,TCR);
	printk("TCR val = %x ", tmp16);
	while(((tmp16 & EMEM_CODE_DONE)==0) && ( i>0))
	{
		udelay(10);
		
		tmp16=read16(priv,TCR);
		
		printk("TCR val = %x\n", tmp16);
		
		i--;
	}
	if(i==0)
	{
		printk("Error => Pollin _EMEM_CODE_DONE Fail\n");
		return -ETIMEDOUT;
	}
		
	if((tmp16 & EMEM_CHK_RPT) == 0) 
	{		
		printk("_EMEM_CHK_RPT = 0\n");
		return -ETIMEDOUT;
	}

	return ret;
	
}
int load_dmem_img(struct mac80211_shared_priv *priv, void *pbuf, void *src, size_t sz)
{
	int ret, i = 100;
	u16 tmp16;	
	
	write_fwimg(priv, pbuf, src, sz, LOAD_DMEM);	
	
	ret = check_img_ready(priv, LOAD_DMEM);

	//polling dmem code done	
	tmp16=read16(priv, TCR);
	while(((tmp16 & DMEM_CODE_DONE)==0) && ( i>0))
	{
		udelay(1000);
		
		tmp16=read16(priv, TCR);
		
		i--;
	}
	if(i==0)
	{
		printk("Error => Pollin _DMEM_CODE_DONE Fail\n");
		return -ETIMEDOUT;
	}

	return ret;
	
}

int check_cpu_ready(struct mac80211_shared_priv *priv)
{
	int i = 100;
	u16 tmp16;

	
	// polling IMEM Ready
	printk("polling IMEM Ready\n");
	
	
	tmp16=read16(priv,TCR);
	while(((tmp16 & IMEM_RDY)==0) && ( i>0))
	{
		udelay(1000);
		
		tmp16=read16(priv,TCR);	
		
		i--;
	}	
	if(i==0)
	{
		printk("Error => Pollin _IMEM_RDY Fail\n");
		return -ETIMEDOUT;
			
	}

	return 0;
	
}

int enable_cpu(struct mac80211_shared_priv *priv)
{
	int ret;
	u8 tmp8, tmp8_a;
	u32 tmp32;
	

	tmp8 = read8(priv, SYS_CLKR);
	printk("WT SYS_CLKR to 0x%x(ori=0x%x)\n", (u32)(tmp8|BIT(2)), tmp8);

	write8(priv, SYS_CLKR, tmp8|BIT(2)); 
	
	tmp8_a = read8(priv, SYS_CLKR);
	if(tmp8_a !=(tmp8|BIT(2)))
	{
		printk("Error=> WT SYS_FUNC_EN fail; SYS_CLKR = %x;  target_val = %x \n", tmp8_a, tmp8);
		return -ETIMEDOUT;
	}
	

	tmp8 = read8(priv, SYS_FUNC_EN+1);
	printk("WT SYS_FUNC_EN+1 to 0x%x[ori=0x%x]\n",(u32)(tmp8|BIT(2)), tmp8);
	
	write8(priv, SYS_FUNC_EN+1, tmp8|BIT(2)); 
	
	tmp8_a= read8(priv, SYS_FUNC_EN+1); 
	if(tmp8_a !=(tmp8|BIT(2)))
	{
		printk("Error=> WT SYS_FUNC_EN fail; SYS_FUNC_EN = %x; target_val = %x\n", tmp8_a, tmp8);
		return -ETIMEDOUT;
	}
	
	tmp32= read32(priv, TCR); 
	printk("RD TCR = %x \n", tmp32);



	ret = check_cpu_ready(priv);//check imem ready
	

	return ret;
	
}

int check_fw_ready(struct mac80211_shared_priv *priv)
{
	int i;
	u8 tmp8;
	u16 tmp16;	
	
	tmp8 = read8(priv, CR9346);

	if(tmp8 & BIT(4))//When boot from EEPROM , FW need more time to read EEPROM 	
		i = 60;
	else //boot from EFUSE
		i = 30;
		
	tmp16=read16(priv, TCR);
	while(((tmp16 & FWRDY)==0) && ( i>0))
	{
		msleep(100);
		
		tmp16=read16(priv,TCR);
		
		i--;
	}		
	if(i==0)
	{
		printk("Error => Pollin _FWRDY Fail\n");
		return -ETIMEDOUT;
	}
		
	return 0;
	
}

//.1 pos init
int rtl819x_pos_init(struct mac80211_shared_priv *priv)
{	
	u8	val8 = 0;	
	u8	PollingCnt = 20;
		
	{
		//Initialization for power on sequence, Revised by Roger. 2008.09.03.

		//E-Fuse leakage prevention sequence
		write8(priv, 0x37, 0xb0);
		msleep(10);
		write8(priv, 0x37, 0x30);
		

		//
		//<Roger_Notes> Set control path switch to HW control and reset Digital Core,  CPU Core and 
		// MAC I/O to solve FW download fail when system from resume sate.
		// 2008.11.04.
		//
		val8 = read8(priv, SYS_CLKR+1);
		//DbgPrint("SYS_CLKR+1=0x%x\n", val8);
       	if(val8 & 0x80)
		{
       		val8 &= 0x3f;
              	write8(priv, SYS_CLKR+1, val8);
		}
	   
      		val8 = read8(priv, SYS_FUNC_EN+1);
		//DbgPrint("SYS_FUNC_EN+1=0x%x\n", val8);
       	val8 &= 0x73;
       	write8(priv, SYS_FUNC_EN+1, val8);
		
		udelay(1000);
		//msleep(100);

		//Revised POS, suggested by SD1 Alex, 2008.09.27.
		write8(priv, SPS0_CTRL+1, 0x53);
		write8(priv, SPS0_CTRL, 0x57);// Switching 18V to PWM.
		//DbgPrint("SPS0_CTRL+1=0x%x\n", read8(priv, SPS0_CTRL+1));
		//DbgPrint("SPS0_CTRL=0x%x\n", read8(priv, SPS0_CTRL));
		
		//Enable AFE Macro Block's Bandgap adn Enable AFE Macro Block's Mbias
		val8 = read8(priv, AFE_MISC);
		//DbgPrint("AFE_MISC=0x%x\n", val8);
		//write8(priv, AFE_MISC, (val8|AFE_MISC_BGEN|AFE_MISC_MBEN));
		write8(priv, AFE_MISC, (val8|AFE_BGEN)); //Bandgap
		write8(priv, AFE_MISC, (val8|AFE_BGEN|AFE_MBEN | BIT(3))); //Mbios
			
		//Enable LDOA15 block -> //Enable PLL Power (LDOA15V)
		val8 = read8(priv, LDOA15_CTRL);
		//DbgPrint("LDOA15_CTRL=0x%x\n", val8);
		write8(priv, LDOA15_CTRL, (val8|LDA15_EN));

		//val8 = read8(priv, SPS1_CTRL);	
		//write8(priv, SPS1_CTRL, (val8|SPS1_LDEN));

		//msleep(2);

		//Enable LDOV12D block
		val8 = read8(priv, LDOV12D_CTRL);
		//DbgPrint("LDOV12D_CTRL=0x%x\n", val8);
		write8(priv, LDOV12D_CTRL, (val8|BIT(0)));	
	

		//Enable Switch Regulator Block
		//val8 = read8(priv, SPS1_CTRL);	
		//write8(priv, SPS1_CTRL, (val8|SPS1_SWEN));

		//write32(priv, SPS1_CTRL, 0x00a7b267);//?
 	
		val8 = read8(priv, SYS_ISO_CTRL+1);
		//DbgPrint("SYS_ISO_CTRL+1=0x%x\n", val8);
		write8(priv, SYS_ISO_CTRL+1, (val8|0x08));

		//Engineer Packet CP test Enable
		val8 = read8(priv, SYS_FUNC_EN+1);
		//DbgPrint("SYS_FUNC_EN+1=0x%x\n", val8);
		write8(priv, SYS_FUNC_EN+1, (val8|0x20));

		
		//Support 64k IMEM, suggested by SD1 Alex.
		val8 = read8(priv, SYS_ISO_CTRL+1);
		//DbgPrint("SYS_ISO_CTRL+1=0x%x\n", val8);
		//write8(priv, SYS_ISO_CTRL+1, (val8& 0x6F));		
		write8(priv, SYS_ISO_CTRL+1, (val8&0x68));

		//Enable AFE clock
		val8 = read8(priv, AFE_XTAL_CTRL+1);
		//DbgPrint("AFE_XTAL_CTRL+1=0x%x\n", val8);
		write8(priv, AFE_XTAL_CTRL+1, (val8& 0xfb));

		//Enable AFE PLL Macro Block
		val8 = read8(priv, AFE_PLL_CTRL);
		//DbgPrint("AFE_PLL_CTRL=0x%x\n", val8);
		write8(priv, AFE_PLL_CTRL, (val8|0x11));

		//(20090928) for some sample will download fw failure
		udelay(1000);
		write8(priv, AFE_PLL_CTRL, (val8|0x51));
		udelay(100);
		write8(priv, AFE_PLL_CTRL, (val8|0x11));
		udelay(100);		

		//Attatch AFE PLL to MACTOP/BB/PCIe Digital
		val8 = read8(priv, SYS_ISO_CTRL);
		//DbgPrint("SYS_ISO_CTRL=0x%x\n", val8);
		write8(priv, SYS_ISO_CTRL, (val8&0xEE));

		// Switch to 40M clock
		write8(priv, SYS_CLKR, 0x00);

		//CPU Clock and 80M Clock SSC Disable to overcome FW download fail timing issue.
		val8 = read8(priv, SYS_CLKR);
		write8(priv, SYS_CLKR, (val8|0xa0));

		//Enable MAC clock
		val8 = read8(priv, SYS_CLKR+1);
		//DbgPrint("SYS_CLKR+1=0x%x\n", val8);
		write8(priv, SYS_CLKR+1, (val8|0x18));

		//Revised POS, suggested by SD1 Alex, 2008.09.27.
		write8(priv, PMC_FSM, 0x02);
		//DbgPrint("PMC_FSM=0x%x\n", read8(priv, PMC_FSM));
	
		//Enable Core digital and enable IOREG R/W
		val8 = read8(priv, SYS_FUNC_EN+1);
		//DbgPrint("SYS_FUNC_EN+1=0x%x\n", val8);
		write8(priv, SYS_FUNC_EN+1, (val8|0x08));

		//Enable REG_EN
		val8 = read8(priv, SYS_FUNC_EN+1);
		//DbgPrint("SYS_FUNC_EN+1=0x%x\n", val8);
		write8(priv, SYS_FUNC_EN+1, (val8|0x80));

		//Switch the control path to FW
		val8 = read8(priv, SYS_CLKR+1);
		//DbgPrint("SYS_CLKR+1=0x%x\n", val8);
		write8(priv, SYS_CLKR+1, (val8|0x80)& 0xBF);

		write8(priv, CMDR, 0xFC);		
		write8(priv, CMDR+1, 0x37);	

		//Fix the RX FIFO issue(usb error), 970410
		val8 = read8(priv, 0x1025FE5c);
		//DbgPrint("0x1025FE5c=0x%x\n", val8);
		write8(priv, 0x1025FE5c, (val8|BIT(7)));

#if 0	//fw will help set it depending on the fwpriv.
#define USE_SIX_USB_ENDPOINT		
#ifdef USE_SIX_USB_ENDPOINT
		val8 = read8(priv, 0x102500ab);	
		write8(priv, 0x102500ab, (val8|BIT(6)|BIT(7)));
#endif
#endif
	 	//For power save, used this in the bit file after 970621
		val8 = read8(priv, SYS_CLKR);	
		write8(priv, SYS_CLKR, val8&(~SYS_CPU_CLKSEL));
		//DbgPrint("SYS_CLKR=0x%x\n", read8(priv, SYS_CLKR));

		// Revised for 8051 ROM code wrong operation. Added by Roger. 2008.10.16. 
		write8(priv, 0x1025fe1c, 0x80);

		//
		// <Roger_EXP> To make sure that TxDMA can ready to download FW.
		// We should reset TxDMA if IMEM RPT was not ready.
		// Suggested by SD1 Alex. 2008.10.23.
		//
		do
		{
			val8 = read8(priv, TCR);
			if((val8 & (IMEM_CHK_RPT|EMEM_CHK_RPT)) == (IMEM_CHK_RPT|EMEM_CHK_RPT))
				break;	
			
			udelay(5);
			
		}while(PollingCnt--);	// Delay 1ms
	
		if(PollingCnt <= 0 )
		{
			//ERR_8712("MacConfigBeforeFwDownloadASIC(): Polling _TXDMA_INIT_VALUE timeout!! Current TCR(%#x)\n", val8);

			val8 = read8(priv, CMDR);	
			
			write8(priv, CMDR, val8&(~TXDMA_EN));
			
			udelay(2);
			
			write8(priv, CMDR, val8|TXDMA_EN);// Reset TxDMA
		}
		
	}

	return 0;
	
}		

//.2 download f/w
int rtl819x_fw_init(struct mac80211_shared_priv *priv)
{
	int ret = 0;
	struct fw_hdr  fwhdr;
	size_t ulfilelength, maxlen;
	size_t imem_sz, emem_sz, dmem_sz;
	void	*phfwfile_hdl = NULL;
	u8	*pmappedfw = NULL, *pbuf_allocated = NULL, *pbuf, *ptr;	
	size_t txdesc_sz = sizeof(struct rtl8192s_txdesc);
	
	
	ulfilelength = rtl8192s_open_fw(priv, &phfwfile_hdl, &pmappedfw);

	if((!pmappedfw) || (ulfilelength<=0))
		return -ENODATA;

		
	update_fwhdr(&fwhdr, pmappedfw);

	if(chk_fwhdr(&fwhdr, ulfilelength))
	{
		printk("CHK FWHDR fail!\n");
		ret = -EPERM;
		goto exit;
	}
	
	fill_fwpriv(priv, &fwhdr.fwpriv);
			
	//firmware check ok		
	printk("Downloading RTL8192S firmware major(%d)/minor(%d) version...\n", fwhdr.version >>8, fwhdr.version & 0xff);

	maxlen = (fwhdr.img_IMEM_size > fwhdr.img_SRAM_size)? fwhdr.img_IMEM_size : fwhdr.img_SRAM_size ;
	maxlen += txdesc_sz;
	
	pbuf_allocated = (u8*)kmalloc(maxlen + FWBUFF_ALIGN_SZ, GFP_KERNEL);		
	if(pbuf_allocated==NULL)
	{
		printk("can't alloc resources when dl_fw\n");
		ret = -ENOMEM;
		goto exit;
	}
			
	pbuf = (pbuf_allocated + FWBUFF_ALIGN_SZ - ((u32 )(pbuf_allocated )&(FWBUFF_ALIGN_SZ-1)));
	
	ptr = pmappedfw + FIELD_OFFSET(struct fw_hdr, fwpriv) + fwhdr.fw_priv_sz ;//+ fwhdr.dmem_size;
				
	//.1 load imem
	imem_sz = fwhdr.img_IMEM_size;
	ret = load_imem_img(priv, pbuf, ptr, imem_sz);
	if(ret)
	{
		printk("load_imem_img fail!\n");		
		goto exit;
	}
	
	ptr += imem_sz;
	//.2 load emem
	emem_sz = fwhdr.img_SRAM_size;
	ret = load_emem_img(priv, pbuf, ptr, emem_sz);
	if(ret)
	{
		printk("load_emem_img fail!\n");		
		goto exit;
	}
	
	ptr += emem_sz;
	
	//.3 enable cpu
	ret = enable_cpu(priv);
	if(ret)
	{
		printk("enable_cpu fail!\n");		
		goto exit;
	}

	
	//.4 load dmem
	ptr = (u8*)&fwhdr.fwpriv;
	dmem_sz = fwhdr.fw_priv_sz;
	ret = load_dmem_img(priv, pbuf, ptr, dmem_sz);
	if(ret)
	{
		printk("load_dmem_img fail!\n");		
		goto exit;
	}

	//.5 check fw ready
	ret = check_fw_ready(priv);
	if(ret)
	{
		printk("check_fw_ready() fail!\n");		
		goto exit;
	}
	else
	{
		printk("check_fw_ready() ok!\n");
	}

exit:

	rtl8192s_close_fw(priv, phfwfile_hdl);
	
	if(pbuf_allocated!=NULL)
		kfree(pbuf_allocated);

	return ret;
	
}

//.3 mac init
int rtl819x_mac_init(struct mac80211_shared_priv *priv)
{
	u8 val8;
	u32 val32;
	int i, ret=0;
	
#if 0	
	//Set CMDR, Enable Tx/Rx
	val8 =(u8)(BBRSTn|BB_GLB_RSTn|SCHEDULE_EN|MACRXEN|MACTXEN|DDMA_EN|
				FW2HW_EN|RXDMA_EN|TXDMA_EN|HCI_RXDMA_EN|HCI_TXDMA_EN);
	
	write8(priv, CMDR, val8);
	
	//Loopback mode or Normal mode
	write8(priv, LBKMD_SEL, 0x00);//normal mode
	
	//Set RCR
	//write32(priv, RCR, priv->ReceiveConfig);
		

	//Set RQPN
	//FW will help do this

#if 0 //remove ?
	if (priv->bBootFromEfuse)
	{
		val8 = read_nic_byte(dev, SYS_ISO_CTRL+1); 
		val8 &= 0xFE;
		write_nic_byte(dev, SYS_ISO_CTRL+1, val8);
		
		// Change Program timing
		write_nic_byte(dev, EFUSE_CTRL+3, 0x72);		
	}
#endif

#if 0//temp remove
	// <Roger_Notes> Retrieve default FW Cmd IO map. 2009.05.08.
	priv->FwCmdIOMap = 	read_nic_word(dev, LBUS_MON_ADDR);
	priv->FwCmdIOParam = read_nic_dword(dev, LBUS_ADDR_MASK);
#endif

	ret = ConfigMACWithHeaderFile(priv);
	if(ret)
		return ret;


	//move to misc init
#if 0	
	for (i=0; i<4; i++)
	{
		write_nic_dword(dev,WDCAPARA_ADD[i], 0x5e4322);
	}	
	
	write_nic_byte(dev,AcmHwCtrl, 0x01);
#endif	
#endif

	val32 = 0;
	val32 = read32(priv, CMDR);
	write32(priv, CMDR, (val32&0x00FFFFFF));


	//
	RTL_W32(RCR, RCR_APPFCS |RCR_APP_PHYST_RXFF | RCR_APP_PHYST_STAFF | // Accept PHY status
			RCR_APWRMGT /*| RCR_ADD3*/ |RCR_AMF| RCR_ADF | RCR_APP_MIC | RCR_APP_ICV|
			RCR_AICV| /*RCR_ACRC32|*/	// Accept ICV error, CRC32 Error
			RCR_AB|RCR_AM|	// Accept Broadcast, Multicast
    			RCR_APM |		// Accept Physical match
     			/*RCR_AAP	|   */// Accept Destination Address packets     			
			(7/*pHalData->EarlyRxThreshold*/<<RCR_FIFO_OFFSET));
	

#ifdef CONFIG_RTL8712_TCP_CSUM_OFFLOAD_RX
	printk("1 RCR=0x%x \n",  read32(priv, RCR));
	val32 = read32(priv, RCR);//RCR
	write32(priv, RCR, (val32|BIT(26))); //Enable RX TCP Checksum offload	
	printk("2 RCR=0x%x \n",  read32(priv, RCR));
#endif	

#ifdef CONFIG_RTL8712_TCP_CSUM_OFFLOAD_TX
	printk("1 TCR=0x%x \n",  read32(priv, TCR));
	val32 = read32(priv, TCR);
	write32(priv, TCR, (val32|BIT(25))); //Enable TX TCP Checksum offload	
	printk("2 TCR=0x%x \n",  read32(priv, TCR));
#endif	
	
#ifdef CONFIG_SDIO_HCI
	write8(priv, 0x10250006, 0x3B);
	write8(priv, 0x10250040, 0xFC);
	write8(priv, 0x10250042, 0x00);
	write8(priv, RTL8712_SDIO_LOCAL_BASE+0xff, 0x0);
	write16(priv,SDIO_HIMR,0x3);	
#endif

//#ifdef CONFIG_USB_HCI
	//for usb rx aggregation	
	write8(priv, 0x102500B5, read8(priv, 0x102500B5)|BIT(0));//page = 128bytes
	write8(priv, 0x102500BD, read8(priv, 0x102500BD)|BIT(7));//enable usb rx aggregation
	//write8(padapter, 0x102500D9, 48);//TH = 48 pages, 6k
	write8(priv, 0x102500D9, 1);// TH=1 => means that invalidate  usb rx aggregation	
	//write8(padapter, 0x1025FE5B, 0x02);// 1.7ms/2
	write8(priv, 0x1025FE5B, 0x04);// 1.7ms/4

	// Fix the RX FIFO issue(USB error), Rivesed by Roger, 2008-06-14
	val8 = read8(priv, 0x1025fe5C);
	write8(priv, 0x1025fe5C, val8|BIT(7));
//#endif


	//RXFILTERMAP
	write16(priv, RXFILTERMAP, 0x0215);//currently, for sta_mode only

	
	for(i=0; i<6; i++)
	{
		write8(priv, MACIDR+i, priv->hwaddr[i]);
		//write8(priv, BSSIDR+i, priv->bssid[i]);
	
	}
	
	
	//for(i=0; i<6; i++)
	//{
	//	val8 = read8(priv, MACIDR+i);
		//priv->hwaddr[i] = val8;			
	//}
	
	printk("MACIDR:" MACSTR "\n", MAC2STR(priv->hwaddr));


	return ret;
	
}

//.4 bb init
int rtl819x_bb_init(struct mac80211_shared_priv *priv)
{
	int ret=0;
#if 0	
	u8 PathMap = 0, index = 0, rf_num = 0;
	struct rtl8192s_intf *pintfpriv = INTF_PRIV(priv);
	struct rtl8192s_hwinfo *phwinfo = HW_INFO(pintfpriv);
	struct rtl8192s_rfctrl *prf_ctrl = BBRF_CTRL(pintfpriv);
	
	
	phy_InitBBRFRegisterDefinition(priv);



	//
	// Config BB and AGC
	//
	ret = phy_BB8192S_Config_ParaFile(priv);

	// Check if the CCK HighPower is turned ON.
	// This is used to calculate PWDB.
	prf_ctrl->bCckHighPower = (u8)(rtl8192_QueryBBReg(priv, rFPGA0_XA_HSSIParameter2, 0x200));


	PathMap = (u8)(rtl8192_QueryBBReg(priv, rFPGA0_TxInfo, 0xf) |
					rtl8192_QueryBBReg(priv, rOFDM0_TRxPathEnable, 0xf));


	//priv->rf_pathmap = PathMap;	
	for(index=0; index<4; index++)
	{
		if((PathMap>>index)&0x1)
			rf_num++;
	}

	if((prf_ctrl->rf_type==RF_1T1R && rf_num!=1) ||
		(prf_ctrl->rf_type==RF_1T2R && rf_num!=2) ||
		(prf_ctrl->rf_type==RF_2T2R && rf_num!=2) ||
		(prf_ctrl->rf_type==RF_2T2R_GREEN && rf_num!=2) ||
		(prf_ctrl->rf_type==RF_2T4R && rf_num!=4))
	{	
		printk("RF_Type(%x) does not match RF_Num(%x)!!\n", prf_ctrl->rf_type, rf_num);
		ret = (-EPERM);
	}


	rtl8192_setBBreg(priv, rFPGA0_AnalogParameter2, 0xff, 0x58);//===>ok

	//
	// . Initialize RF related configurations.
	//
	// 2007/11/02 MH Before initalizing RF. We can not use FW to do RF-R/W.
	//priv->Rf_Mode = RF_OP_By_SW_3wire;
	
		
	RTL_W8(AFE_XTAL_CTRL+1, 0xDB);

	// <Roger_Notes> The following IOs are configured for each RF modules.
	// Enable RF module and reset RF and SDM module. 2008.11.17.
	if(phwinfo->chip_version == VERSION_8192SU_A)
		RTL_W8(SPS1_CTRL+3, (u8)(RF_EN|RF_RSTB|RF_SDMRSTB)); // Fix A-Cut bug.
	else
		RTL_W8(RF_CTRL, (u8)(RF_EN|RF_RSTB|RF_SDMRSTB));

#endif

	return ret;
	
}

//.5 rf init
int rtl819x_rf_init(struct mac80211_shared_priv *priv)
{
	int ret=0;
#if 0	
	struct rtl8192s_intf *pintfpriv = INTF_PRIV(priv);	
	struct rtl8192s_rfctrl *prf_ctrl = BBRF_CTRL(pintfpriv);

	//
	// RF config
	//
	switch(prf_ctrl->rf_chip)
	{
		case RF_8225:
		case RF_6052:
			ret = PHY_RF6052_Config(priv);
			break;
		
		case RF_8256:			
			//ret = PHY_RF8256_Config(priv);
			break;
		
		case RF_8258:
			break;
			
		case RF_PSEUDO_11N:
			//ret = PHY_RF8225_Config(priv);
			break;
			
        	default:
           		 break;
				 
	}
#endif

	return ret;
	
}

//.6 misc init
int rtl819x_misc_init(struct mac80211_shared_priv *priv)
{
#if 0
	u8 val8;
	int i;
	int WDCAPARA_ADD[4] = {EDCAPARA_BE, EDCAPARA_BK, EDCAPARA_VI, EDCAPARA_VO};
	struct rtl8192s_intf *pintfpriv = INTF_PRIV(priv);
	struct rtl8192s_hwinfo *phwinfo = HW_INFO(pintfpriv);
	struct rtl8192s_rfctrl *prf_ctrl = BBRF_CTRL(pintfpriv);

	//.1
	for (i=0; i<4; i++)
	{
		write32(priv, WDCAPARA_ADD[i], 0x5e4322);
	}	
	
	write8(priv, ACMHWCTRL, 0x01);


	//
	// Joseph Note: Keep RfRegChnlVal for later use.
	//
	prf_ctrl->RfRegChnlVal[0] = rtl8192_phy_QueryRFReg(priv, (RF90_RADIO_PATH_E)0, RF_CHNLBW, bRFRegOffsetMask);
	prf_ctrl->RfRegChnlVal[1] = rtl8192_phy_QueryRFReg(priv, (RF90_RADIO_PATH_E)1, RF_CHNLBW, bRFRegOffsetMask);
	
	// Set CCK and OFDM Block "ON"
	rtl8192_setBBreg(priv, rFPGA0_RFMOD, bCCKEn, 0x1);
	rtl8192_setBBreg(priv, rFPGA0_RFMOD, bOFDMEn, 0x1);

	//
	// Turn off Radio B while RF type is 1T1R by SD3 Wilsion's request. 
	// Revised by Roger, 2008.12.18.
	//
	if(prf_ctrl->rf_type == RF_1T1R)
	{
		// This is needed for PHY_REG after 20081219
		rtl8192_setBBreg(priv, rFPGA0_RFMOD, 0xff000000, 0x03);
		// This is needed for PHY_REG before 20081219
		//PHY_SetBBReg(Adapter, rOFDM0_TRxPathEnable, bMaskByte0, 0x11);
	}

#ifdef WIFI_TEST
	rtl8192_setBBreg(priv, rFPGA0_XC_RFTiming, 0xffffffff, 0x002003c5);
#endif

#if 0//temp remove
#if (RTL8192SU_DISABLE_IQK==0)

		// For 1T2R IQK only currently.
		if (phwinfo->chip_version == VERSION_8192SU_B)
		{	
			PHY_IQCalibrateBcut(priv);
		}
		else if (phwinfo->chip_version == VERSION_8192SU_A)
		{
			PHY_IQCalibrate(priv);	
		}
#endif
#endif

	rtl8192SU_HwConfigureRTL8192SUsb(priv);


	//Security related.
	//-----------------------------------------------------------------------------
	// Set up security related. 070106, by rcnjko:
	// 1. Clear all H/W keys.
	// 2. Enable H/W encryption/decryption.
	//-----------------------------------------------------------------------------
	//
	CamResetAllEntry(priv);


	//User disable RF via registry. ???


	//rtl8192_rx_enable(dev); !!!


	//
	// Read EEPROM TX power index and PHY_REG_PG.txt to capture correct
	// TX power index for different rate set.
	//
	if(phwinfo->chip_version >= VERSION_8192SU_A)
	{
		// Get original hw reg values
		PHY_GetHWRegOriginalValue(priv);

		// Write correct tx power index for current default channel
		PHY_SetTxPowerLevel8192S(priv, priv->cur_ch);
	}

	// EEPROM R/W workaround
	val8 = RTL_R8(MAC_PINMUX_CFG);
	RTL_W8(MAC_PINMUX_CFG, val8&(~GPIOMUX_EN));

#if 0 //temp remove

	//
	// <Roger_Notes> The following FW CMDs are for DM initialization.
	//
	write_nic_dword(dev, WFM5, FW_IQK_ENABLE);
	ChkFwCmdIoDone(dev);


	//
	// <Roger_Notes> We enable high power mechanism after NIC initialized. 
	// 2008.11.27.
	//
	if(pFirmware->FirmwareVersion >= 0x35)
	{
		// Fw v.53 and later.
		priv->ieee80211->SetFwCmdHandler(dev,FW_CMD_RA_INIT);
	}
	else if(pFirmware->FirmwareVersion >= 0x34)
	{
		// Fw v.52 and later.
		write_nic_dword(dev, WFM5, FW_RA_INIT); 
		ChkFwCmdIoDone(dev);
	}
	else
	{
		// Compatible earlier FW version.
		write_nic_dword(dev, WFM5, FW_RA_RESET); 
		ChkFwCmdIoDone(dev);
		write_nic_dword(dev, WFM5, FW_RA_ACTIVE); 
		ChkFwCmdIoDone(dev);
		write_nic_dword(dev, WFM5, FW_RA_REFRESH); 
		ChkFwCmdIoDone(dev);
	}

	// Disable RF-B when 1T condition.
	if(priv->CustomerID == RT_CID_819x_CAMEO1)
	{
		write_nic_dword(dev, WFM5, FW_TXANT_SWITCH_DISABLE); 
		ChkFwCmdIoDone(dev);	
	}
	
#endif
#endif

	return 0;

}

#if 0
int rtl819x_init_hw_USB(struct mac80211_shared_priv *priv)
{
	int ret;

	//.0 check chip id
	// todo:


	//.1 pos init	
	ret = rtl819x_pos_init(priv);

	
	//.2 download f/w
	ret = rtl819x_fw_init(priv);	
	if(ret)
	{
		printk("rtl819x_init_hw: download f/w fail!\n");
		goto hw_init_fail;
	}
	

	//.3 mac init
	ret = rtl819x_mac_init(priv);
	
	
	//.4 bb init
	ret = rtl819x_bb_init(priv);

	
	//.5 rf init
	ret = rtl819x_rf_init(priv);
	

	//.6 misc init
	ret = rtl819x_misc_init(priv);


	return 0;
	
hw_init_fail:
	
	return ret;
	
}
#endif

static int rtl8192s_usb_recv_entry(struct mac80211_shared_priv *priv, struct sk_buff *skb)
{	
	u8 *pbuf;	
	unsigned int pkt_len, pkt_offset;
	int transfer_len;
	struct rtl8192s_rxdesc *prxstat;	
	u16 pkt_cnt, drvinfo_sz, rxdesc_sz=sizeof(struct rtl8192s_rxdesc);
	struct sk_buff *skb_rx;		
	struct ieee80211_hw *hw = priv->hw;
	struct ieee80211_rx_status rx_status = { 0 };

	//printk("+%s\n", __FUNCTION__);
	
	pbuf = (u8*)skb->data;
	
	prxstat = (struct rtl8192s_rxdesc *)pbuf;		
	
	pkt_cnt = (le32_to_cpu(prxstat->rxdw2)>>16)&0x3ffff;
	pkt_len =  le32_to_cpu(prxstat->rxdw0)&0x00003fff;	

	transfer_len = (int)skb->len;

#if 0 //temp remove when disable usb rx aggregation
	if((pkt_cnt > 10) || (pkt_cnt < 1) || (transfer_len<RXDESC_SIZE) ||(pkt_len<=0))
	{		
		return -EFAULT;
	}
#endif
	
	do{		

		prxstat = (struct rtl8192s_rxdesc *)pbuf;      
		pkt_len =  le32_to_cpu(prxstat->rxdw0)&0x00003fff;

		//RT_TRACE(_module_rtl871x_recv_c_,_drv_info_,("rxdesc : offsset0:0x%08x, offsset4:0x%08x, offsset8:0x%08x, offssetc:0x%08x\n", 
		//	prxstat->rxdw0, prxstat->rxdw1, prxstat->rxdw2, prxstat->rxdw4));

				
		drvinfo_sz = (le32_to_cpu(prxstat->rxdw0)&0x000f0000)>>16;
		drvinfo_sz = drvinfo_sz<<3;

		//RT_TRACE(_module_rtl871x_recv_c_,_drv_info_,("DRV_INFO_SIZE=%d\n", drvinfo_sz));

		if(pkt_len<=0)
		{	
			//RT_TRACE(_module_rtl871x_recv_c_,_drv_info_,("recvbuf2recvframe(), pkt_len<=0\n"));		
			goto  _exit_recv_entry;
		}		
	
				
		skb_rx = skb_clone(skb, GFP_ATOMIC);			

		skb_rx->len = 0;
		skb_rx->head = skb_rx->data = skb_rx->tail = skb->data;
		
		//pkt_offset = (u16)_RND512(pkt_len + drvinfo_sz + rxdesc_sz);
		//pkt_offset = (u16)_RND256(pkt_len + drvinfo_sz + rxdesc_sz);
		pkt_offset = (u16)_RND128(pkt_len + drvinfo_sz + rxdesc_sz);//TODO: need to check init. setting.

		skb_put(skb_rx, pkt_len + drvinfo_sz + rxdesc_sz);
		skb_pull(skb_rx, drvinfo_sz + rxdesc_sz);		


		//RT_TRACE(_module_rtl871x_recv_c_,_drv_info_,("recvbuf2recvframe(), transfer_len=%d, pkt_offset=%d, pkt_len=%d, ref_cnt=%d, precvbuf=0x%x\n",
		//		transfer_len, pkt_offset, pkt_len, precvbuf->ref_cnt, precvbuf));


		//because the endian issue, driver avoid reference to the rxstat after calling update_recvframe_attrib_from_recvstat();
		//update_recvframe_attrib_from_recvstat(&precvframe->u.hdr.attrib, prxstat);		

		//printk("%s: call ieee80211_rx_irqsafe\n", __FUNCTION__);
		
		ieee80211_rx_irqsafe(hw, skb_rx, &rx_status);
	
		transfer_len -= pkt_offset;
		pbuf += pkt_offset;
		pkt_cnt--;		

	}while((transfer_len>0) && (pkt_cnt>0));

	
_exit_recv_entry:

	dev_kfree_skb_any(skb);

	return 0;
	
}


static void rtl8192s_usb_recv_cb(struct urb *urb)
{
	unsigned int isevt, *pbuf;
	struct sk_buff *skb = (struct sk_buff *)urb->context;
	struct urb_cb_info *info = (struct urb_cb_info *)skb->cb;	
	struct mac80211_shared_priv *priv = info->priv;
	struct priv_shared_hw *pshared_hw = (struct priv_shared_hw *)priv->pshare_hw;
	struct rtl8190_hw *pintfpriv = pshared_hw->phw;
	struct ieee80211_hw *hw = priv->hw;
	struct ieee80211_rx_status rx_status = { 0 };
	int ret, rate, signal;
	u32 flags;
	u32 quality;
	unsigned long f;

	//printk("%s\n", __FUNCTION__);

	spin_lock_irqsave(&pintfpriv->rx_queue.lock, f);
	if (skb->next)
		__skb_unlink(skb, &pintfpriv->rx_queue);
	else {
		spin_unlock_irqrestore(&pintfpriv->rx_queue.lock, f);
		return;
	}
	spin_unlock_irqrestore(&pintfpriv->rx_queue.lock, f);
	skb_put(skb, urb->actual_length);

	if (unlikely(urb->status)) {
		dev_kfree_skb_irq(skb);
		return;
	}



	pbuf = (uint*)skb->data;
	if((isevt = le32_to_cpu(*(pbuf+1))&0x1ff) == 0x1ff)
	{			
		printk("usb_read_port_complete():rxcmd_event_hdl\n");

		//rxcmd_event_hdl(pintfpriv, pbuf);//rx c2h events

		skb_reset_tail_pointer(skb);// re-use skb
		
	}
	else
	{
		//ieee80211_rx_irqsafe(dev, skb, &rx_status);
		
		ret = rtl8192s_usb_recv_entry(priv, skb);

		if(!ret)
		{
			skb = dev_alloc_skb(MAX_RECVBUF_SZ);
			if (unlikely(!skb)) {
				/* TODO check rx queue length and refill *somewhere* */
				return;
			}
		}
		else // re-use skb
		{
			skb_reset_tail_pointer(skb);
		}
	}
	
	info = (struct urb_cb_info *)skb->cb;	
	info->urb = urb;	
	info->priv = priv;

	
	urb->transfer_buffer = skb_tail_pointer(skb);
	urb->context = skb;
	skb_queue_tail(&pintfpriv->rx_queue, skb);


	//printk("%s, submit_urb again\n", __FUNCTION__);
	
	usb_anchor_urb(urb, &pintfpriv->anchored);	
	if (usb_submit_urb(urb, GFP_ATOMIC)) {
		usb_unanchor_urb(urb);
		skb_unlink(skb, &pintfpriv->rx_queue);
		dev_kfree_skb_irq(skb);
	}	


}

int rtl8192s_usb_recv(struct mac80211_shared_priv *priv)
{
	 int pipe;
	struct urb *entry = NULL;
	struct sk_buff *skb;
	struct urb_cb_info *info;
	struct priv_shared_hw *pshared_hw = (struct priv_shared_hw *)priv->pshare_hw;
	struct rtl8190_hw *pintfpriv = pshared_hw->phw;
	int ret = 0;

	init_usb_anchor(&pintfpriv->anchored);
	
	while (skb_queue_len(&pintfpriv->rx_queue) < 4)
	{
		skb = __dev_alloc_skb(MAX_RECVBUF_SZ, GFP_KERNEL);
		if (!skb) {
			ret = -ENOMEM;
			goto err;
		}
		entry = usb_alloc_urb(0, GFP_KERNEL);
		if (!entry) {
			ret = -ENOMEM;
			goto err;
		}

		pipe = ffaddr2pipehdl(pintfpriv, RTL8712_DMA_RX0FF);
		if(pipe==0)
		{
			ret = -EPERM;
			goto err;
		}
		
		usb_fill_bulk_urb(entry, pintfpriv->udev,
				  		pipe, skb_tail_pointer(skb),
				  		MAX_RECVBUF_SZ, rtl8192s_usb_recv_cb, skb);
		
		info = (struct urb_cb_info *)skb->cb;
		info->urb = entry;
		info->priv = priv;
				
		skb_queue_tail(&pintfpriv->rx_queue, skb);				

		printk("usb_submit_urb, recv urb\n");
		
		usb_anchor_urb(entry, &pintfpriv->anchored);
		ret = usb_submit_urb(entry, GFP_KERNEL);
		if (ret)
		{			
			skb_unlink(skb, &pintfpriv->rx_queue);
			
			usb_unanchor_urb(entry);
			
			goto err;
		}
		
		usb_free_urb(entry);//???
		
	}
	
	return ret;

err:
	
	usb_free_urb(entry);
	
	kfree_skb(skb);
	
	usb_kill_anchored_urbs(&pintfpriv->anchored);
	
	return ret;	
	

}

unsigned int do_queue_select(struct rtl8190_hw *pintfpriv, unsigned int tid)
{
	unsigned int qsel=0;
	
	if(pintfpriv->nr_endpoint == 6)
	{		
		qsel = tid;	
	}	
	else if(pintfpriv->nr_endpoint == 4)
	{	
		qsel = tid;	
	}

	return qsel;

}

int update_txdesc(struct rtl8190_hw *pintfpriv, struct sk_buff *skb, struct tx_info *txinfo)
{
	u8 *qc;
	u16 fc;	
	unsigned int sz, mac_id, bmcst, tid=0, qsel=0;
	struct ieee80211_hdr *tx_hdr;
	struct rtl8192s_txdesc *ptxdesc;


	sz = skb->len;
	tx_hdr = (struct ieee80211_hdr *)(skb->data);
	fc = le16_to_cpu(tx_hdr->frame_control);
	bmcst = is_multicast_ether_addr(tx_hdr->addr1);
	

	// ----- fill tx desc -----	
	ptxdesc = (struct rtl8192s_txdesc *)skb_push(skb, sizeof(*ptxdesc));///!!!
	memset(ptxdesc, 0, sizeof(*ptxdesc));
	
	
	//offset 0	
	ptxdesc->txdw0 |= cpu_to_le32(sz&0x0000ffff); 
	ptxdesc->txdw0 |= cpu_to_le32(((TXDESC_SIZE+OFFSET_SZ)<<OFFSET_SHT)&0x00ff0000);//default = 32 bytes for TX Desc
	ptxdesc->txdw0 |= cpu_to_le32(OWN | FSG | LSG);


	if ((fc & IEEE80211_FCTL_FTYPE) == IEEE80211_FTYPE_DATA)
	{
		//offset 4
		mac_id = 0;//todo
		ptxdesc->txdw1 |= cpu_to_le32(mac_id&0x1f);//CAM_ID(MAC_ID)


		if (ieee80211_is_data_qos(tx_hdr->frame_control))
		{
			qc = ieee80211_get_qos_ctl(tx_hdr);
			tid = *qc & IEEE80211_QOS_CTL_TID_MASK;

			//tid = txinfo->priority;
			//txinfo->priority = tid;

			qsel =  do_queue_select(pintfpriv, tid);
		}
		else
		{
			qsel = tid = 0;			
			
			ptxdesc->txdw1 |= cpu_to_le32(BIT(16));//Non-QoS
		}

		qsel = qsel&0x0000001f;			
		ptxdesc->txdw1 |= cpu_to_le32((qsel<<QSEL_SHT)&0x00001f00);

#if 0	
		if(pattrib->encrypt	>0 && !pattrib->bswenc)
		{
			switch(pattrib->encrypt)
			{	//SEC_TYPE
			
				case _WEP40_:
				case _WEP104_:
					ptxdesc->txdw1 |= cpu_to_le32((0x01<<22)&0x00c00000);

					//KEY_ID when WEP is used;
					ptxdesc->txdw1 |= cpu_to_le32((psecuritypriv->dot11PrivacyKeyIndex<<17)&0x00060000);

					break;
				
				case _TKIP_:
				case _TKIP_WTMIC_:	
					ptxdesc->txdw1 |= cpu_to_le32((0x02<<22)&0x00c00000);
					break;

				case _AES_:
					ptxdesc->txdw1 |= cpu_to_le32((0x03<<22)&0x00c00000);
					break;
					
				case _NO_PRIVACY_:
				default:						
					break;

			}				
		}
#endif
		//offset 8				
		if(bmcst)	
		{
			ptxdesc->txdw2 |= cpu_to_le32(BMC);
		}	

		//offset 12
		//f/w will increase the seqnum by itself, driver pass the correct priority to fw
		//fw will check the correct priority for increasing the seqnum per tid.
		//about usb using 4-endpoint, qsel points out the correct mapping between AC&Endpoint,		
		//the purpose is that correct mapping let the MAC releases the AC Queue list correctly.		
		//ptxdesc->txdw3 = ((pattrib->seqnum<<SEQ_SHT)&0x0fff0000);
		ptxdesc->txdw3 = cpu_to_le32((tid<<SEQ_SHT)&0x0fff0000);

		//offset 16
		//offset 20
	

#if 0
		if( ( pattrib->ether_type != 0x888e ) && ( pattrib->ether_type != 0x0806 ) &&(pattrib->dhcp_pkt != 1) )
		{

			//Not EAP & ARP type data packet			
              	
#ifdef CONFIG_80211N_HT	
			if(phtpriv->ht_option==1) //B/G/N Mode
			{
				if(phtpriv->ampdu_enable != _TRUE)
				{
					ptxdesc->txdw2 |= cpu_to_le32(BK);
				}	
			
			
				//ptxdesc->txdw4 = cpu_to_le32(0x80000000);//driver uses data rate
				//ptxdesc->txdw4 |= cpu_to_le32(BIT(18)|BIT(20));
				//ptxdesc->txdw5 = cpu_to_le32(0x001f2600);// MCS7				
				//ptxdesc->txdw5 = cpu_to_le32(0x001f3600);// MCS15
			}
			else	
#endif		
			{
				//ptxdesc->txdw4 = 0x80000000;//driver uses data rate
                   		//ptxdesc->txdw5 = 0x001f1600;// 54M
		    		//ptxdesc->txdw5 = 0x001f2600;//MCS7						
		    		//ptxdesc->txdw5 = 0x001f9600;// 54M
			}
 		   
                }
		else
		{ 
                  	// EAP data packet and ARP packet.
                  // Use the 1M data rate to send the EAP/ARP packet.
                  // This will maybe make the handshake smooth.
                  
			ptxdesc->txdw4 = cpu_to_le32(0x80000000);//driver uses data rate
			ptxdesc->txdw5 = cpu_to_le32(0x001f8000);// 1M
		}
#else

		ptxdesc->txdw4 = cpu_to_le32(0x80000000);//driver uses data rate
		ptxdesc->txdw5 = cpu_to_le32(0x001f8000);// 1M
	
#endif
		
		
	}		
	else if((fc & IEEE80211_FCTL_FTYPE) == IEEE80211_FTYPE_MGMT)
	{		
		//offset 4
		ptxdesc->txdw1 |= (0x05)&0x1f;//CAM_ID(MAC_ID), default=5;
		ptxdesc->txdw1 |= cpu_to_le32(BIT(16));//Non-QoS

		qsel = tid = 0x06;
		qsel = qsel&0x0000001f;			
		ptxdesc->txdw1 |= cpu_to_le32((qsel<<QSEL_SHT)&0x00001f00);

		//offset 8			
		if(bmcst)	
		{
			ptxdesc->txdw2 |= cpu_to_le32(BMC);
		}	


		//offset 12		
		ptxdesc->txdw3 = cpu_to_le32((tid<<SEQ_SHT)&0x0fff0000);

		//offset 16		
		ptxdesc->txdw4 = cpu_to_le32(0x80002040);//gtest
		
		//offset 20		
		ptxdesc->txdw5 = cpu_to_le32(0x001f8000);//gtest//1M
		
		
	
	}
	else
	{
		
		
	}

	txinfo->priority = tid;

	return 0;

}

static void rtl8192s_usb_xmit_cb(struct urb *urb)
{
	struct sk_buff *skb = (struct sk_buff *)urb->context;
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
	struct mac80211_shared_priv *priv = (struct mac80211_shared_priv *)info->rate_driver_data[0];
	struct ieee80211_hw *hw = priv->hw;
	

	skb_pull(skb, sizeof(struct rtl8192s_txdesc));
					  
	ieee80211_tx_info_clear_status(info);

	if (!urb->status && !(info->flags & IEEE80211_TX_CTL_NO_ACK))
	{
		
		//printk("(1)%s\n", __FUNCTION__);
		ieee80211_tx_status_irqsafe(hw, skb);
		
	}
	else 
	{
		if (!(info->flags & IEEE80211_TX_CTL_NO_ACK) && !urb->status)
			info->flags |= IEEE80211_TX_STAT_ACK;
		

		//printk("(2)%s\n", __FUNCTION__);

		ieee80211_tx_status_irqsafe(hw, skb);
	}
	
}

int rtl8192s_usb_xmit(struct mac80211_shared_priv *priv, struct sk_buff *skb, struct tx_info *txinfo)
{	
	int rc, pipe;
	unsigned int tid, qsel;	
	struct urb *urb;	
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
	struct priv_shared_hw *pshared_hw = (struct priv_shared_hw *)priv->pshare_hw;
	struct rtl8190_hw *pintfpriv = pshared_hw->phw;	
	
	
	//printk("(1)%s\n", __FUNCTION__);

	urb = usb_alloc_urb(0, GFP_ATOMIC);
	if (!urb) {
		kfree_skb(skb);
		return NETDEV_TX_OK;
	}

	
	update_txdesc(pintfpriv, skb, txinfo);//will push skb->data pointer for fill txdesc;


	//translate DMA FIFO addr to pipehandle
	tid = txinfo->priority;
	qsel = do_queue_select(pintfpriv, tid);	
	pipe = ffaddr2pipehdl(pintfpriv, get_ff_hwaddr(pintfpriv, tid, qsel));	

	info->rate_driver_data[0] = priv;
	info->rate_driver_data[1] = urb;
	
	usb_fill_bulk_urb(urb, pintfpriv->udev, pipe,
			  skb->data, skb->len, rtl8192s_usb_xmit_cb, skb);
	
	urb->transfer_flags |= URB_ZERO_PACKET;
	usb_anchor_urb(urb, &pintfpriv->anchored);
	rc = usb_submit_urb(urb, GFP_ATOMIC);
	if (rc < 0) {
		usb_unanchor_urb(urb);
		kfree_skb(skb);
	}
	usb_free_urb(urb);


	return NETDEV_TX_OK;	
	
}


