
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/usb.h>
//#include <asm/mach-realtek/rtl8672/platform.h>	
#include <bspchip.h>
#include "otg_dev_driver.h"

/* Version Information */
#define DRIVER_VERSION "v0.1.0 (2010/03/29)"
#define DRIVER_DESC "usb-ethernet driver for dwc OTG device"

#define	INTBUFSIZE		8
#define	OTG_TX_TIMEOUT	(HZ)
#define	RX_SKB_POOL_SIZE	4
#define	OTG_DEV_MTU	1540

#define warn printk

#ifdef CONFIG_USB_EHCI_HCD
void ehci_hcd_init(void);
void ehci_hcd_cleanup(void);
void ehci_shutdown_rtl8672 ( void );
void ehci_stop_rtl8672 ( void );
void ehci_start_rtl8672 ( void );
#endif

#ifndef print_ft2
#define print_ft2 printk
#endif

int TX_SIZE=1400;
u8 TEST_PKT_NUM=200;
u8 loop=0;
u8 ep1rec=0;
u8 ep3rec[2]={0};
u8 is_EHCI=0;
u8 EHCI_up=0;
int open_time=-1;
u8 port1_disconnect=0;

static u8 pass_out[2];
static u8 pass_in[2];
static u8 pass_IPT[2];
#ifdef	CONFIG_EQC_COMMON
#include <linux/eqc_common.h>
extern unsigned int check_ft2_ready;                  //for RTL8672 356P EQC test, kevinchung
#endif

extern unsigned eqc_pass_count;                  //for RTL8672 356P EQC test, kevinchung
extern void eqc_fail_display(void);			//for RTL8672 356P EQC test, kevinchung
extern void eqc_pass_display(void);			//for RTL8672 356P EQC test, kevinchung
extern void ext_miiar_write(unsigned char phyaddr,unsigned char regaddr,unsigned short value);		//for RTL8672 356P EQC test, kevinchung
extern void ext_miiar_read(unsigned char phyaddr,unsigned char regaddr,unsigned short *value);	//for RTL8672 356P EQC test, kevinchung
extern void WatchdogEventActive(void);		//for RTL8672 356P EQC test, kevinchung


struct otg_dev {
	unsigned long flags;
	struct usb_device *udev;
	struct net_device_stats stats;
	struct net_device *netdev;
	struct urb *rx_urb, *tx_urb, *intr_urb, *ctrl_urb;
	struct sk_buff *tx_skb, *rx_skb;
	struct sk_buff *rx_skb_pool[RX_SKB_POOL_SIZE];
	spinlock_t rx_pool_lock;
	struct usb_ctrlrequest dr;
	int intr_interval;
	__le16 rx_creg;
	u8 *intr_buff;
	unsigned char *bulkin_buff;
	unsigned char *bulkout_buff;
	unsigned int bulkin_buff_offset;
	unsigned int bulkout_buff_offset;
	unsigned char bulkin_class;
	//unsigned char bulkout_class;
	unsigned char bulkin_start_data;
	unsigned char bulkout_start_data;
	int bulkin_len;
	int bulkout_len;
};
typedef struct otg_dev otg_dev_t;




#define VENDOR_ID_REALTEK		0x0bda
#define VENDOR_ID_REALTEK2		0x0bdd
#define PRODUCT_ID_RTL8672		0x8672

/* table of devices that work with this driver */
static struct usb_device_id otg_dev_table[] = {
	{USB_DEVICE(VENDOR_ID_REALTEK, PRODUCT_ID_RTL8672)},
	{USB_DEVICE(VENDOR_ID_REALTEK2, PRODUCT_ID_RTL8672)},		
	{}
};

static int otg_dev_probe(struct usb_interface *intf,
			   const struct usb_device_id *id);
static void otg_dev_disconnect(struct usb_interface *intf);

static int otg_dev_start_xmit(struct net_device *netdev);

static const char driver_name [] = "otg_dev";

static struct usb_driver otg_dev_driver = {
	.name =		driver_name,
	.probe =	otg_dev_probe,
	.disconnect =	otg_dev_disconnect,
	.id_table =	otg_dev_table,
	//.suspend =	rtl8150_suspend,
	//.resume =	rtl8150_resume
};

#if 0
static void otg_dev_tx_timeout(struct net_device *netdev)
{
	otg_dev_t *dev = netdev_priv(netdev);
	warn("%s: Tx timeout.", netdev->name);
	usb_unlink_urb(dev->tx_urb);
	dev->stats.tx_errors++;
}
#endif

#if 1
static void memDump (void *start, u32 size, char * strHeader)
{
	int row, column, index, index2, max;
//	uint32 buffer[5];
	u8 *buf, *line, ascii[17];
	char empty = ' ';

	if(!start ||(size==0))
		return;
	line = (u8*)start;

	/*
	16 bytes per line
	*/
	if (strHeader)
		printk("%s", strHeader);
	column = size % 16;
	row = (size / 16) + 1;
	for (index = 0; index < row; index++, line += 16) 
	{
		buf = line;

		memset (ascii, 0, 17);

		max = (index == row - 1) ? column : 16;
		if ( max==0 ) break; /* If we need not dump this line, break it. */

		printk("\n%08x ", (u32) line);
		
		//Hex
		for (index2 = 0; index2 < max; index2++)
		{
			if (index2 == 8)
			printk("  ");
			printk("%02x ", (u8) buf[index2]);
			ascii[index2] = ((u8) buf[index2] < 32) ? empty : buf[index2];
		}

		if (max != 16)
		{
			if (max < 8)
				printk("  ");
			for (index2 = 16 - max; index2 > 0; index2--)
				printk("   ");
		}

		//ASCII
		printk("  %s", ascii);
	}
	printk("\n");
	return;
}
#endif
static int alloc_all_urbs(otg_dev_t * dev)
{
	dev->rx_urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!dev->rx_urb)
		return 0;
	
	dev->tx_urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!dev->tx_urb) {
		usb_free_urb(dev->rx_urb);
		return 0;
	}
	dev->tx_urb->transfer_flags |= URB_ZERO_PACKET;
	
	dev->intr_urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!dev->intr_urb) {
		usb_free_urb(dev->rx_urb);
		usb_free_urb(dev->tx_urb);
		return 0;
	}
	dev->ctrl_urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!dev->ctrl_urb) {
		usb_free_urb(dev->rx_urb);
		usb_free_urb(dev->tx_urb);
		usb_free_urb(dev->intr_urb);
		return 0;
	}

	return 1;
}

static void free_all_urbs(otg_dev_t * dev)
{
	usb_free_urb(dev->rx_urb);
	usb_free_urb(dev->tx_urb);
	usb_free_urb(dev->intr_urb);
	usb_free_urb(dev->ctrl_urb);
}

static void unlink_all_urbs(otg_dev_t * dev)
{
	usb_kill_urb(dev->rx_urb);
	usb_kill_urb(dev->tx_urb);
	usb_kill_urb(dev->intr_urb);
	usb_kill_urb(dev->ctrl_urb);
}

static void fill_skb_pool(otg_dev_t *dev)
{
	struct sk_buff *skb;
	int i;

	for (i = 0; i < RX_SKB_POOL_SIZE; i++) {
		if (dev->rx_skb_pool[i])
			continue;
		skb = dev_alloc_skb(OTG_DEV_MTU + 2);
		if (!skb) {
			return;
		}
		skb->dev = dev->netdev;
		//skb_reserve(skb, 2);	//cathy, for usb dma alignment problem
		dev->rx_skb_pool[i] = skb;
	}
}

static void free_skb_pool(otg_dev_t *dev)
{
	int i;

	for (i = 0; i < RX_SKB_POOL_SIZE; i++)
		if (dev->rx_skb_pool[i])
			dev_kfree_skb(dev->rx_skb_pool[i]);
}

static inline struct sk_buff *pull_skb(otg_dev_t *dev)
{
	struct sk_buff *skb;
	int i;

	for (i = 0; i < RX_SKB_POOL_SIZE; i++) {
		if (dev->rx_skb_pool[i]) {
			skb = dev->rx_skb_pool[i];
			dev->rx_skb_pool[i] = NULL;
			return skb;
		}
	}
	return NULL;
}

static void read_bulk_callback(struct urb *urb)
{
	otg_dev_t *dev;
	struct net_device *netdev;
	int status;
	__u8 *d;
	unsigned int i;
	unsigned char data;

	dev = urb->context;
	if (!dev)
		return;
	
	netdev = dev->netdev;
	d = urb->transfer_buffer;
	if(urb->actual_length == 0){
		//printk("%s-%d  urb->actual_length=0\n",__func__,__LINE__);
		goto again;
	}
	//if(urb->actual_length != dev->bulkin_len)
	//	printk("urb->actual_length = %d, dev->bulkin_len = %d\n", urb->actual_length, dev->bulkin_len);

	//memDump(d, urb->actual_length, "urb->transfer_buffer:");
	//printk("[%d] dev->bulkin_start_data = %02x, urb->actual_length= %d, dev->bulkin_len=%d\n",__LINE__, dev->bulkin_start_data, urb->actual_length, dev->bulkin_len);
	dev->stats.rx_packets++;
	dev->stats.rx_bytes += urb->actual_length;
	data = dev->bulkin_start_data;
	for(i=0; i<urb->actual_length; i++) {
		if(d[i] != data) {
			printk("%s i = %d,data=%x, dev->bulkin_len=%d, actual_length=%d\n",__func__, i,data,dev->bulkin_len,urb->actual_length);
			memDump(d, urb->actual_length, "urb->transfer_buffer:");
			dev->stats.rx_errors++;
			dev->stats.rx_length_errors++;
			break;
		}
		data++;
	}

	if(dev->bulkin_len >= END_DATA_LEN) {
		if(dev->bulkin_buff_offset >= END_DMA_ADDR) {	//finish testing at last starting address
			//printk("bulk in test finish!\n");
			print_ft2("PASS_%s_in\n",netdev->name);	
#ifdef	CONFIG_EQC_COMMON
			if(!strncmp(netdev->name,"usb10",5)){
				eqc_pass_count |= USB0_BIN_EQC_TEST;
			}else{
				eqc_pass_count |= USB1_BIN_EQC_TEST;
			}
			check_ft2_ready++;
			printk("eqc_pass_count=%d [bulk in]\n",eqc_pass_count);
#endif			
			dev->bulkin_buff_offset = START_DMA_ADDR;
			dev->bulkin_start_data = START_DATA;
			dev->bulkin_len = START_DATA_LEN;
			goto out;
		}
		else {
			switch(dev->bulkin_class) {
				case ALIGNED_4N:
				case ALIGNED_4N_1:
				case ALIGNED_4N_2:
				case ALIGNED_4N_3:
					dev->bulkin_buff_offset+=4;
					if(dev->bulkin_buff_offset > END_DMA_ADDR)
						return;
					break;					
				case ALIGNED_NO:
				default:
					dev->bulkin_buff_offset++;
					break;
			}
			//printk("[%d], bulkin_len=%d, bulkin_start_data=%02x\n",__LINE__,dev->bulkin_len,dev->bulkin_start_data);					
			dev->bulkin_len = START_DATA_LEN;
			dev->bulkin_start_data++;	//change starting data
		}
	}
	else {	//next data length for next tx testing at the same starting address
		//printk("[%d], bulkin_len=%d, bulkin_start_data=%02x\n",__LINE__,dev->bulkin_len,dev->bulkin_start_data);		
		dev->bulkin_len++;
		dev->bulkin_start_data++;	//change starting data
	}
#if 0	
	if((d[0] != ep1rec) || (urb->status !=0)) {
		print_ft2("F_usb0-%d: usb ep1  <%s,%d> urb->status = %d, d[0] = %d, ep1rec=%d\n", open_time, __func__, __LINE__, urb->status, d[0], ep1rec);
		eqc_fail_display();
		while(1);
	}
	for(i=1; i<(TX_SIZE-1); i++) {
		if(d[i] != i%0xFF) {
			print_ft2("F_usb1-%d: usb ep1  <%s,%d> d[%d] = %d\n", open_time, __func__, __LINE__, i, d[i]);
			eqc_fail_display();
			while(1);
		}
	}
	if(d[TX_SIZE-1]!=0xFF) {
		print_ft2("F_usb6-%d: usb cable plugs in wrong port!\n", open_time);
		eqc_fail_display();
		while(1);
	}
	ep1rec++;

	if(ep1rec==TEST_PKT_NUM) {
		print_ft2("P_usb0-%d\n", open_time);
		//print_ft2("(P_usb0)END USB time slice %lu\n", jiffies);				//Unit : 10 mini second ....,addition by kevinchung
		eqc_pass_count++;
		return;
	}
#endif	

	otg_dev_start_xmit(netdev);
again:
	usb_fill_bulk_urb(dev->rx_urb, dev->udev, usb_rcvbulkpipe(dev->udev, 1),
		      dev->bulkin_buff+dev->bulkin_buff_offset, END_DATA_LEN+512, read_bulk_callback, dev);
	dma_cache_wback_inv((unsigned long)(dev->bulkin_buff+dev->bulkin_buff_offset), END_DATA_LEN+512);	      
	status = usb_submit_urb(dev->rx_urb, GFP_ATOMIC);
	if (status == -ENODEV)
		netif_device_detach(dev->netdev);

	
out:	
	return;

}

static void intr_callback(struct urb *urb)
{
	otg_dev_t *dev;
	__u8 *d;
	int status;
	int count;
	dev = urb->context;
	if (!dev)
		return;
	
	d = urb->transfer_buffer;	
	if(!strncmp(dev->netdev->name,"usb10",5))
		count = 0;
	else
		count = 1;	
	
	if((d[0] != ep3rec[count]) || (urb->status !=0)) {
		print_ft2("FAIL_usb2-%d: usb ep3  <%s,%d> urb->status = %d, d[0] = %d, ep3rec[%d]=%d :::%s\n", open_time, __func__, __LINE__, urb->status, d[0],count, ep3rec[count],dev->netdev->name);
#ifdef	CONFIG_EQC_COMMON
		eqc_fail_display();
		//while(1);
#endif
	}

	ep3rec[count]++;
	if( ep3rec[count]== TEST_PKT_NUM) {
		//print_ft2("P_usb_intr1-%d\n", open_time);
		pass_IPT[count] = 1;
		print_ft2("PASS_%s_IPT\n",dev->netdev->name);
		//print_ft2("(P_usb1)END USB time slice %lu\n", jiffies);				//Unit : 10 mini second ....,addition by kevinchung
#ifdef	CONFIG_EQC_COMMON
		if(!strncmp(dev->netdev->name,"usb10",5)){
			eqc_pass_count |= USB0_IPT_EQC_TEST;
		}else{
			eqc_pass_count |= USB1_IPT_EQC_TEST;
		}
		check_ft2_ready++;
		printk("eqc_pass_count=%d [intr1]\n",eqc_pass_count);
#endif		
		return;
	}
#if 1	
	usb_fill_int_urb(dev->intr_urb, dev->udev, usb_rcvintpipe(dev->udev, 3),
		 dev->intr_buff, INTBUFSIZE, intr_callback,
		 dev, dev->intr_interval);
	dma_cache_wback_inv((unsigned long)dev->intr_buff, INTBUFSIZE);
	status = usb_submit_urb (urb, GFP_ATOMIC);
	if (status == -ENODEV)
		netif_device_detach(dev->netdev);
	else if (status)
		err ("can't resubmit intr, %s-%s/input0, status %d",
				dev->udev->bus->bus_name,
				dev->udev->devpath, status);
#else
	usb_fill_int_urb(dev->intr_urb, dev->udev, usb_rcvintpipe(dev->udev, 3),
		 dev->intr_buff, INTBUFSIZE, intr_callback,
		 dev, dev->intr_interval);
	dma_cache_wback_inv((unsigned long)dev->intr_buff, INTBUFSIZE);
	status = usb_submit_urb(dev->intr_urb, GFP_KERNEL);
	if (status == -ENODEV)
		netif_device_detach(dev->netdev);
}
#endif
}

static void write_bulk_callback(struct urb *urb)
{
	otg_dev_t *dev;
	unsigned char align_class;
#if defined (SKIP_64_REST) || defined (SKIP_CROSS_1K_4_7)
	int rest;
#endif

	dev = urb->context;
	if (!dev)
		return;

	if (!netif_device_present(dev->netdev))
		return;
		
	if (urb->status != 0) {
		printk("%s %d, urb->status=%d\n", __func__, __LINE__, urb->status);
		dev->stats.tx_errors++;
	}

	if(urb->actual_length != dev->bulkout_len)
		printk("urb->actual_length = %d, dev->bulkout_len = %d\n", urb->actual_length, dev->bulkout_len);

	//printk("[%d] bulkout_start_data:%02x, bulkout_len=%d actual len=%d\n",__LINE__,dev->bulkout_start_data,dev->bulkout_len,urb->actual_length);

	dev->stats.tx_packets++;
	dev->stats.tx_bytes += urb->actual_length;
	//memDump(urb->transfer_buffer, urb->actual_length, "data:");
	//netif_wake_queue(dev->netdev);
	//0923
	//usb_free_urb(urb);
	//dev->bulkout_buff_offset++;
	//return;
	align_class = BULKOUT_CLASS;
	if(dev->bulkout_len >= END_DATA_LEN) {	//finish testing data length from START_DATA_LEN to END_DATA_LEN at some starting address
		if(dev->bulkout_buff_offset >= END_DMA_ADDR) {	//finish testing at last starting address
			//printk("bulk out test finish!\n");
			print_ft2("PASS_%s_out\n",dev->netdev->name);	//[bulk out]
#ifdef	CONFIG_EQC_COMMON
			if(!strncmp(dev->netdev->name,"usb10",5)){
				eqc_pass_count |= USB0_BOUT_EQC_TEST;
			}else{
				eqc_pass_count |= USB1_BOUT_EQC_TEST;
			}
			check_ft2_ready++;
			printk("eqc_pass_count=%d [bulk out]\n",eqc_pass_count);
#endif			
			return;
		}
		else {	//choose next starting address and reset data length for next tx testing
			switch(align_class) {
				case ALIGNED_4N:
				case ALIGNED_4N_1:
				case ALIGNED_4N_2:
				case ALIGNED_4N_3:
					dev->bulkout_buff_offset+=4;
					if(dev->bulkout_buff_offset > END_DMA_ADDR)
						return;
					break;					
				case ALIGNED_NO:
				default:
					dev->bulkout_buff_offset++;
					break;
			}
			
			dev->bulkout_len = START_DATA_LEN;
		}
	}
	else {	//next data length for next tx testing at the same starting address
		dev->bulkout_len++;
	}

#ifdef SKIP_64_REST
check_length_again:
	rest = dev->bulkout_len % 64;

	if (align_class == ALIGNED_NO) {
		align_class = dev->bulkout_buff_offset & 0x3;
	}

	switch(align_class) {
		case ALIGNED_4N_1:
			if((rest >= 4) && (rest <= 6))
				dev->bulkout_len = dev->bulkout_len + 7 - rest;
			break;
		case ALIGNED_4N_2:
			if((rest >= 3) && (rest <= 5))
				dev->bulkout_len = dev->bulkout_len + 6 - rest;
			break;
		case ALIGNED_4N_3:
			if((rest >= 2) && (rest <= 4))
				dev->bulkout_len = dev->bulkout_len + 5 - rest;
			break;
		default:
			break;
	}
#endif	//SKIP_64_REST

#ifdef SKIP_CROSS_1K_4_7
	if((((unsigned int)dev->bulkout_buff) + dev->bulkout_buff_offset) & 0x3ff) {	//starting address is not at 1K boundary
		if (((((unsigned int)dev->bulkout_buff) + dev->bulkout_buff_offset + dev->bulkout_len) >> 10) !=
			((((unsigned int)dev->bulkout_buff) + dev->bulkout_buff_offset) >> 10)) { // over 1k boundary
			rest = ((((unsigned int)dev->bulkout_buff) + dev->bulkout_buff_offset + dev->bulkout_len)&0x3ff);
			//printk("rest = %d\n", rest);
			if ((rest >= 4) && (rest <= 7)) {
				dev->bulkout_len = dev->bulkout_len + 8 - rest;
#ifdef SKIP_64_REST				
				goto check_length_again;
#endif
			}
		}
	}
#endif	//SKIP_CROSS_1K_4_7

	dev->bulkout_start_data++;	//change starting data
#ifdef TX_ONLY
	otg_dev_start_xmit(dev->netdev);
#endif
}

static int otg_dev_start_xmit(struct net_device *netdev)
{
	otg_dev_t *dev = netdev_priv(netdev);
//	struct sk_buff *skb;
	int i=0, res;
	//0923
	//struct urb *urb;
#if 0
	if(loop == TEST_PKT_NUM)
		return 0;
	skb = dev_alloc_skb(OTG_DEV_MTU);	
	if (!skb) {
		print_ft2("F_usb4-%d: usb tx<%s, %d>allocate skb fail\n", open_time, __func__, __LINE__);
		eqc_fail_display();
		while(1);
		return -1;
	}
	skb->dev = dev->netdev;
	skb->data[0]=loop;
	loop++;
	
	for(i=1; i<TX_SIZE; i++) {
		skb->data[i]=i%0xFF;
	}

	dev->tx_skb = skb;
	
	//0923
	urb = usb_alloc_urb(0, GFP_ATOMIC);
	if (!urb){
		warn("failed to allocate urb %s\n", __func__);
		return -ENOMEM;
	}
	//0923
#endif
	for(i=0; i<dev->bulkout_len; i++) {
		dev->bulkout_buff[dev->bulkout_buff_offset+i] = (dev->bulkout_start_data+i) & 0xff;
	}
	
	usb_fill_bulk_urb(dev->tx_urb, dev->udev, usb_sndbulkpipe(dev->udev, 2),
		      dev->bulkout_buff+dev->bulkout_buff_offset, dev->bulkout_len, write_bulk_callback, dev);
	//usb_fill_bulk_urb(urb, dev->udev, usb_sndbulkpipe(dev->udev, 2),
    //                skb->data, TX_SIZE, write_bulk_callback, dev);
	dma_cache_wback_inv((unsigned long)(dev->bulkout_buff+dev->bulkout_buff_offset), dev->bulkout_len);
	//memDump(dev->bulkout_buff+dev->bulkout_buff_offset, dev->bulkout_len, "bulk out:");
	//0923
	if ((res = usb_submit_urb(dev->tx_urb, GFP_ATOMIC))) {
	//if ((res = usb_submit_urb(urb, GFP_ATOMIC))) {
		/* Can we get/handle EPIPE here? */
		if (res == -ENODEV)
			netif_device_detach(dev->netdev);
		else {
			warn("failed tx_urb %d\n", res);
			dev->stats.tx_errors++;
			netif_start_queue(netdev);
		}
	} else {
		netdev->trans_start = jiffies;
	}

	
	return 0;
}

static int otg_dev_open(struct net_device *netdev)
{
	otg_dev_t *dev = netdev_priv(netdev);
	int res,count;
	if(!strncmp(netdev->name,"usb10",5))
		count = 0;
	else
		count = 1;
	ep3rec[count] = 0;	
	pass_in[count] = 0;
	pass_out[count] = 0;	
	pass_IPT[count] = 0;
#if 0	
	loop=0;
	ep1rec=0;
	ep3rec=0;

	open_time++;
	if( open_time>2 && port1_disconnect==0 ){
		print_ft2("F_usb7-%d: usb cable plug in wrong port! <%s, %d>\n", open_time, __func__, __LINE__);
		eqc_fail_display();
                while(1);	
	}

	if (dev->rx_skb == NULL)
		dev->rx_skb = pull_skb(dev);
	if (!dev->rx_skb){
		print_ft2("F_usb5-%d: usb open <%s, %d>\n", open_time, __func__, __LINE__);
		eqc_fail_display();
		while(1);
		return -ENOMEM;
	}
	//set_registers(dev, IDR, 6, netdev->dev_addr);
#endif


	dev->bulkout_buff_offset = START_DMA_ADDR;
	dev->bulkout_start_data = START_DATA;
	dev->bulkout_len = START_DATA_LEN;


#ifndef TX_ONLY
	dev->bulkin_buff_offset = START_DMA_ADDR;
	dev->bulkin_start_data = START_DATA;
	dev->bulkin_len = START_DATA_LEN;
	dev->bulkin_class = BULKIN_CLASS;
#endif
	
	
	//dev->bulkin_class = 0;
	//dev->bulkout_class = ALIGNED_4N;
	
	
	
	

	//usb_fill_bulk_urb(dev->rx_urb, dev->udev, usb_rcvbulkpipe(dev->udev, 1),
	//	      dev->rx_skb->data, OTG_DEV_MTU, read_bulk_callback, dev);
	//dma_cache_wback_inv((unsigned long)dev->rx_skb->data, OTG_DEV_MTU);
#ifndef TX_ONLY
	usb_fill_bulk_urb(dev->rx_urb, dev->udev, usb_rcvbulkpipe(dev->udev, 1),
		      dev->bulkin_buff, END_DATA_LEN+512, read_bulk_callback, dev);
	dma_cache_wback_inv((unsigned long)dev->bulkin_buff, END_DATA_LEN+512);

	if ((res = usb_submit_urb(dev->rx_urb, GFP_KERNEL))) {
		if (res == -ENODEV)
			netif_device_detach(dev->netdev);
		warn("%s: rx_urb submit failed: %d", __FUNCTION__, res);
		//return res;
	}
#endif


#if 1	
	usb_fill_int_urb(dev->intr_urb, dev->udev, usb_rcvintpipe(dev->udev, 3),
		     dev->intr_buff, INTBUFSIZE, intr_callback,
		     dev, dev->intr_interval);
	dma_cache_wback_inv((unsigned long)dev->intr_buff, INTBUFSIZE);
	if ((res = usb_submit_urb(dev->intr_urb, GFP_KERNEL))) {
		if (res == -ENODEV)
			netif_device_detach(dev->netdev);
		warn("%s: intr_urb submit failed: %d", __FUNCTION__, res);
		usb_kill_urb(dev->rx_urb);
		return res;
	}
#endif


	otg_dev_start_xmit(netdev);


	return res;
}
#ifndef CONFIG_USB_EHCI_HCD
extern int nic_sar_test;
#endif
static int otg_dev_close(struct net_device *netdev)
{
	otg_dev_t *dev = netdev_priv(netdev);
	int res = 0, count;
	//u32 tmp;
	if(!strncmp(netdev->name,"usb10",5))
		count = 0;
	else
		count = 1;

	ep3rec[count] = 0;
	pass_in[count] = 0;
	pass_out[count] = 0;	
	pass_IPT[count] = 0;	
	netif_stop_queue(netdev);
				
	//if (!test_bit(RTL8150_UNPLUG, &dev->flags))
	//	disable_net_traffic(dev);
#ifdef CONFIG_USB_EHCI_HCD
	unlink_all_urbs(dev);
#if 0

	tmp = REG32(0xb8030804);

	REG32(0xb8030804) = tmp | 0x02000000;	//cathy, set soft disconnect in reg DCTL

	if( is_EHCI==0 ) {
		if(EHCI_up==0)
			ehci_hcd_init();
//		else {		
//			ehci_start_rtl8672();
//			REG32(0xb8030804) &= 0xFDFFFFFF;
//		}
		is_EHCI = 1;
		EHCI_up = 1;
	}
#endif	
#endif
//	else {
//		ehci_stop_rtl8672();
//		ehci_hcd_cleanup();
//		is_EHCI = 0;
//		REG32(0xb8030804) &= 0xFDFFFFFF;
//	}
	//REG32(0xb8030804) = tmp;	//cathy, let host redetect our device
#if 0	
	print_ft2("P_usb2-%d\n", open_time);
	//print_ft2("END USB time slice %lu\n", jiffies);				//Unit : 10 mini second ....,addition by kevinchung
	//print_ft2("eqc_pass_count=%d \n",eqc_pass_count);
	if(eqc_pass_count==9)
		eqc_pass_display();
	else if(eqc_pass_count==2){
			unsigned short value=0;
			ext_miiar_read(4,19,&value);
			if(value==0){
				unsigned eqc_i=0;
				*(volatile unsigned long *) 0xb8003508 = 0x800050;		//GPA4 ,GPA6 and GPC7 set output for relay control		
				*(volatile unsigned long *) 0xb800350c = 0x10;			//define GPA4 data equ 1 for USB Port0 and Port1 crossconnection
				for(eqc_i=0;eqc_i<6;eqc_i++){
					*(volatile unsigned long *) 0xb800350c = 0x800010;	//define GPA4 and GPC7 data equ 1 
					mdelay(1);
					*(volatile unsigned long *) 0xb800350c = 0x10;	//reset GPC7 data equ 0 
					mdelay(1);
					}
				ext_miiar_write(4,19,4096);
				mdelay(2);
				*(volatile unsigned long *) 0xb8003508 = 0x10800050;		//GPD4 ,GPA4 ,GPA6 and GPC7 set output for relay control	
				*(volatile unsigned long *) 0xb800350c = 0x10000010;		//define GPA4 and GPD4 data equ 1 for AP1534 shotdown
				}
			else if(value==4096){
				ext_miiar_write(4,19,8192);	//for RTL8672 EQC test,save test item to temp register
				mdelay(2);
				WatchdogEventActive();
*(volatile unsigned long *) 0xb8003508 = 0x50;			//GPA4 and GPA6 set output for relay control		
*(volatile unsigned long *) 0xb800350c = 0x40;			//define GPA4 data equ 1 for USB Port0 and Port1 crossconnection				
				*(volatile unsigned long *) 0xb8003508 = 0x10800050;		//GPD4 ,GPA4 ,GPA6 and GPC7 set output for relay control	
				*(volatile unsigned long *) 0xb800350c = 0x10000050;		//define GPA4 and GPD4 data equ 1 for AP1534 shotdown
				//*(volatile unsigned long *) 0xb800350c = 0x10000010;		//define GPA4 and GPD4 data equ 1 for AP1534 shotdown
				//print_ft2("test point1 \n");
				}
			else
				print_ft2("not define this test item for RTL8672 356p EQC test ........\n");
		}
	else
		eqc_fail_display();
#endif		
	return res;
}

static int otg_dev_start_xmit2 (struct sk_buff *skb, struct net_device *net)
{
	//dev_kfree_skb_any (skb);
	return 0;
}

static struct net_device_stats *otg_dev_stats(struct net_device *dev)
{
	return &((otg_dev_t *)netdev_priv(dev))->stats;
}

static const struct net_device_ops dwc_otg_netdev_ops = {
	.ndo_open		= otg_dev_open,
	.ndo_stop		= otg_dev_close,
	//.ndo_do_ioctl		= rtl8150_ioctl,
	.ndo_start_xmit		= otg_dev_start_xmit2,
	//.ndo_tx_timeout 	= rtl8150_tx_timeout,
	//.ndo_set_multicast_list = rtl8150_set_multicast,
	//.ndo_set_mac_address	= rtl8150_set_mac_address,
	.ndo_get_stats		= otg_dev_stats,
	.ndo_change_mtu		= eth_change_mtu,
	//.ndo_validate_addr	= eth_validate_addr,
};

static int otg_dev_probe(struct usb_interface *intf,
			 const struct usb_device_id *id)
{
	struct usb_device *udev = interface_to_usbdev(intf);
	otg_dev_t *dev;
	struct net_device *netdev;
	struct usb_host_interface	*interface;
	
	netdev = alloc_etherdev(sizeof(otg_dev_t));
	if (!netdev) {
		err("Out of memory");
		return -ENOMEM;
	}

	interface = intf->cur_altsetting;
	dev = netdev_priv(netdev);
	memset(dev, 0, sizeof(otg_dev_t));

	//allocate buffer for interrupt in endpoint
	dev->intr_buff = kmalloc(INTBUFSIZE, GFP_KERNEL);
	if (!dev->intr_buff) {
		free_netdev(netdev);
		return -ENOMEM;
	}

	//allocate buffer for bulk in endpoint
	dev->bulkin_buff = kmalloc(BULKBUFSIZE, GFP_KERNEL);
	if (!dev->bulkin_buff) {
		free_netdev(netdev);
		return -ENOMEM;
	}
	printk("dev->bulkin_buff = 0x%p\n", dev->bulkin_buff);
	
	//allocate buffer for bulk out endpoint
	dev->bulkout_buff = kmalloc(BULKBUFSIZE, GFP_KERNEL);
	if (!dev->bulkout_buff) {
		free_netdev(netdev);
		return -ENOMEM;
	}
	printk("dev->bulkout_buff = 0x%p\n", dev->bulkout_buff);

	//tasklet_init(&dev->tl, rx_fixup, (unsigned long)dev);
	spin_lock_init(&dev->rx_pool_lock);
	
	dev->udev = udev;
	dev->netdev = netdev;	
	netdev->netdev_ops = &dwc_otg_netdev_ops;
	strcpy (netdev->name, "usb1%d");
	//netdev->mtu = RTL8150_MTU;
	//SET_ETHTOOL_OPS(netdev, &ops);
	dev->intr_interval = 1;	/* 100ms */
	//printk("intf->num_altsetting=%d, interface->desc.bInterfaceNumber = %d, interface->desc.bAlternateSetting= %d\n", intf->num_altsetting, interface->desc.bInterfaceNumber, interface->desc.bAlternateSetting);
	if(intf->num_altsetting == 2)
		usb_set_interface (udev, interface->desc.bInterfaceNumber, 1);	//choose bAlternateSetting=1
	if (!alloc_all_urbs(dev)) {
		err("out of memory");
		goto out;
	}

	//fill_skb_pool(dev);
	//set_ethernet_addr(dev);
	
	usb_set_intfdata(intf, dev);
	SET_NETDEV_DEV(netdev, &intf->dev);
	if (register_netdev(netdev) != 0) {
		err("couldn't register the device");
		goto out2;
	}

	//info("%s: otg_dev is detected", netdev->name);

	return 0;

out2:
	usb_set_intfdata(intf, NULL);
	//free_skb_pool(dev);
out1:
	free_all_urbs(dev);
out:
	kfree(dev->intr_buff);
	free_netdev(netdev);
	return -1;
}

static void otg_dev_disconnect(struct usb_interface *intf)
{
	otg_dev_t *dev = usb_get_intfdata(intf);

	printk("%s %d\n", __func__, __LINE__);

	usb_set_intfdata(intf, NULL);
	if (dev) {
		//set_bit(RTL8150_UNPLUG, &dev->flags);
		//tasklet_disable(&dev->tl);
		//tasklet_kill(&dev->tl);
		kfree(dev->bulkin_buff);
		kfree(dev->bulkout_buff);
		unregister_netdev(dev->netdev);
		unlink_all_urbs(dev);
		free_all_urbs(dev);
		free_skb_pool(dev);
		if (dev->rx_skb)
			dev_kfree_skb(dev->rx_skb);
		kfree(dev->intr_buff);
		free_netdev(dev->netdev);
	}
}

static int __init otg_dev_init(void)
{
	//info(DRIVER_DESC " " DRIVER_VERSION);
	return usb_register(&otg_dev_driver);
}

static void __exit otg_dev_exit(void)
{
	usb_deregister(&otg_dev_driver);
}

module_init(otg_dev_init);
module_exit(otg_dev_exit);
MODULE_DESCRIPTION(DRIVER_DESC);

