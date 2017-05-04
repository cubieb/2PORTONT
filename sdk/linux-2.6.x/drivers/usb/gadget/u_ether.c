/*
 * u_ether.c -- Ethernet-over-USB link layer utilities for Gadget stack
 *
 * Copyright (C) 2003-2005,2008 David Brownell
 * Copyright (C) 2003-2004 Robert Schwebel, Benedikt Spranger
 * Copyright (C) 2008 Nokia Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* #define VERBOSE_DEBUG */

#include <linux/kernel.h>
#include <linux/utsname.h>
#include <linux/device.h>
#include <linux/ctype.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#ifdef CONFIG_DWC_OTG_DEVICE_ONLY
#include <bspchip.h>
#ifdef TEST_MODE
#include "../../net/usb/otg_dev_driver.h"
#endif
#endif

#include "u_ether.h"


/*
 * This component encapsulates the Ethernet link glue needed to provide
 * one (!) network link through the USB gadget stack, normally "usb0".
 *
 * The control and data models are handled by the function driver which
 * connects to this code; such as CDC Ethernet, "CDC Subset", or RNDIS.
 * That includes all descriptor and endpoint management.
 *
 * Link level addressing is handled by this component using module
 * parameters; if no such parameters are provided, random link level
 * addresses are used.  Each end of the link uses one address.  The
 * host end address is exported in various ways, and is often recorded
 * in configuration databases.
 *
 * The driver which assembles each configuration using such a link is
 * responsible for ensuring that each configuration includes at most one
 * instance of is network link.  (The network layer provides ways for
 * this single "physical" link to be used by multiple virtual links.)
 */

#define UETH__VERSION	"29-May-2008"

struct eth_dev {
	/* lock is held while accessing port_usb
	 * or updating its backlink port_usb->ioport
	 */
	spinlock_t		lock;
	struct gether		*port_usb;

	struct net_device	*net;
	struct usb_gadget	*gadget;

	spinlock_t		req_lock;	/* guard {rx,tx}_reqs */
	struct list_head	tx_reqs, rx_reqs;
	atomic_t		tx_qlen;

	unsigned		header_len;
	struct sk_buff		*(*wrap)(struct sk_buff *skb);
	int			(*unwrap)(struct sk_buff *skb);

	struct work_struct	work;

	unsigned long		todo;
#define	WORK_RX_MEMORY		0

	bool			zlp;
	u8			host_mac[ETH_ALEN];
#ifdef TEST_MODE
	unsigned int bulkout_buff_offset;
	unsigned char bulkout_start_data;
	int bulkout_len;
#endif
};

/*-------------------------------------------------------------------------*/

#define RX_EXTRA	20	/* bytes guarding against rx overflows */

#define DEFAULT_QLEN	2	/* double buffering by default */


#ifdef CONFIG_USB_GADGET_DUALSPEED

static unsigned qmult = 5;
module_param(qmult, uint, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(qmult, "queue length multiplier at high speed");

#else	/* full speed (low speed doesn't do bulk) */
#define qmult		1
#endif

/* for dual-speed hardware, use deeper queues at highspeed */
static inline int qlen(struct usb_gadget *gadget)
{
	if (gadget_is_dualspeed(gadget) && gadget->speed == USB_SPEED_HIGH)
		return qmult * DEFAULT_QLEN;
	else
		return DEFAULT_QLEN;
}

/*-------------------------------------------------------------------------*/

/* REVISIT there must be a better way than having two sets
 * of debug calls ...
 */

#undef DBG
#undef VDBG
#undef ERROR
#undef INFO

#define xprintk(d, level, fmt, args...) \
	printk(level "%s: " fmt , (d)->net->name , ## args)

#ifdef DEBUG
#undef DEBUG
#define DBG(dev, fmt, args...) \
	xprintk(dev , KERN_DEBUG , fmt , ## args)
#else
#define DBG(dev, fmt, args...) \
	do { } while (0)
#endif /* DEBUG */

#ifdef VERBOSE_DEBUG
#define VDBG	DBG
#else
#define VDBG(dev, fmt, args...) \
	do { } while (0)
#endif /* DEBUG */

#define ERROR(dev, fmt, args...) \
	xprintk(dev , KERN_ERR , fmt , ## args)
#define INFO(dev, fmt, args...) \
	xprintk(dev , KERN_INFO , fmt , ## args)

#ifdef TEST_MODE
static int eth_start_xmit(struct sk_buff *skb, struct net_device *net);
void memDump (void *start, u32 size, char * strHeader)
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

/*-------------------------------------------------------------------------*/

/* NETWORK DRIVER HOOKUP (to the layer above this driver) */

static int ueth_change_mtu(struct net_device *net, int new_mtu)
{
	struct eth_dev	*dev = netdev_priv(net);
	unsigned long	flags;
	int		status = 0;

	/* don't change MTU on "live" link (peer won't know) */
	spin_lock_irqsave(&dev->lock, flags);
	if (dev->port_usb)
		status = -EBUSY;
	else if (new_mtu <= ETH_HLEN || new_mtu > ETH_FRAME_LEN)
		status = -ERANGE;
	else
		net->mtu = new_mtu;
	spin_unlock_irqrestore(&dev->lock, flags);

	return status;
}

static void eth_get_drvinfo(struct net_device *net, struct ethtool_drvinfo *p)
{
	struct eth_dev	*dev = netdev_priv(net);

	strlcpy(p->driver, "g_ether", sizeof p->driver);
	strlcpy(p->version, UETH__VERSION, sizeof p->version);
	strlcpy(p->fw_version, dev->gadget->name, sizeof p->fw_version);
	strlcpy(p->bus_info, dev_name(&dev->gadget->dev), sizeof p->bus_info);
}

/* REVISIT can also support:
 *   - WOL (by tracking suspends and issuing remote wakeup)
 *   - msglevel (implies updated messaging)
 *   - ... probably more ethtool ops
 */

static struct ethtool_ops ops = {
	.get_drvinfo = eth_get_drvinfo,
	.get_link = ethtool_op_get_link,
};

static void defer_kevent(struct eth_dev *dev, int flag)
{
	if (test_and_set_bit(flag, &dev->todo))
		return;
	if (!schedule_work(&dev->work))
		ERROR(dev, "kevent %d may have been dropped\n", flag);
	else
		DBG(dev, "kevent %d scheduled\n", flag);
}

static void rx_complete(struct usb_ep *ep, struct usb_request *req);

static int
rx_submit(struct eth_dev *dev, struct usb_request *req, gfp_t gfp_flags)
{
	struct sk_buff	*skb;
	int		retval = -ENOMEM;
	size_t		size = 0;
	struct usb_ep	*out;
	unsigned long	flags;

	spin_lock_irqsave(&dev->lock, flags);
	if (dev->port_usb)
		out = dev->port_usb->out_ep;
	else
		out = NULL;
	spin_unlock_irqrestore(&dev->lock, flags);

	if (!out)
		return -ENOTCONN;


	/* Padding up to RX_EXTRA handles minor disagreements with host.
	 * Normally we use the USB "terminate on short read" convention;
	 * so allow up to (N*maxpacket), since that memory is normally
	 * already allocated.  Some hardware doesn't deal well with short
	 * reads (e.g. DMA must be N*maxpacket), so for now don't trim a
	 * byte off the end (to force hardware errors on overflow).
	 *
	 * RNDIS uses internal framing, and explicitly allows senders to
	 * pad to end-of-packet.  That's potentially nice for speed, but
	 * means receivers can't recover lost synch on their own (because
	 * new packets don't only start after a short RX).
	 */
	size += sizeof(struct ethhdr) + dev->net->mtu + RX_EXTRA;
	size += dev->port_usb->header_len;
	size += out->maxpacket - 1;
	size -= size % out->maxpacket;
#ifdef TEST_MODE
		size += 1000;
#endif

	skb = alloc_skb(size + NET_IP_ALIGN, gfp_flags);
	if (skb == NULL) {
		DBG(dev, "no rx skb\n");
		goto enomem;
	}

	/* Some platforms perform better when IP packets are aligned,
	 * but on at least one, checksumming fails otherwise.  Note:
	 * RNDIS headers involve variable numbers of LE32 values.
	 */
#ifndef CONFIG_DWC_OTG_DEVICE_ONLY
	skb_reserve(skb, NET_IP_ALIGN);
#endif

	req->buf = skb->data;
	req->length = size;
	req->complete = rx_complete;
	req->context = skb;

	retval = usb_ep_queue(out, req, gfp_flags);
	if (retval == -ENOMEM){
enomem:
		defer_kevent(dev, WORK_RX_MEMORY);
#ifdef CONFIG_DWC_OTG_DEVICE_ONLY
		//cathy
		spin_lock (&dev->req_lock);
		list_add (&req->list, &dev->rx_reqs);
		spin_unlock (&dev->req_lock);
		return retval;
#endif
	}
	if (retval) {
		DBG(dev, "rx submit --> %d\n", retval);
		if (skb)
			dev_kfree_skb_any(skb);
		spin_lock_irqsave(&dev->req_lock, flags);
		list_add(&req->list, &dev->rx_reqs);
		spin_unlock_irqrestore(&dev->req_lock, flags);
	}
	return retval;
}

static void rx_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct sk_buff	*skb = req->context;
	struct eth_dev	*dev = ep->driver_data;
	int		status = req->status;
#ifdef CONFIG_DWC_OTG_DEVICE_ONLY
#ifdef TEST_MODE
	//cathy
	u32 i;
	unsigned char align_class, data;
#if defined (SKIP_64_REST) || defined (SKIP_CROSS_1K_4_7)
	int rest;
#endif
#endif
#endif

	switch (status) {

	/* normal completion */
	case 0:
		skb_put(skb, req->actual);
		if (dev->unwrap)
			status = dev->unwrap(skb);
#ifndef TEST_MODE
		if (status < 0
				|| ETH_HLEN > skb->len
				|| skb->len > ETH_FRAME_LEN) {
			dev->net->stats.rx_errors++;
			dev->net->stats.rx_length_errors++;
			DBG(dev, "rx length %d\n", skb->len);
			break;
		}
#endif
		dev->net->stats.rx_packets++;
		dev->net->stats.rx_bytes += skb->len;

		/* no buffer copies needed, unless hardware can't
		 * use skb buffers.
		 */
#ifdef TEST_MODE
#ifndef TX_ONLY
		mdelay(1);
#endif
#ifndef RX_ONLY
		//data compare
		data = dev->bulkout_start_data;
		//printk("dev->bulkout_len = %d, skb->len = %d\n", dev->bulkout_len, skb->len);
		for(i=0; i<dev->bulkout_len; i++) {
			//if((skb->data[i] != data) && !err) {
			if(skb->data[i] != data) {
				//err = 1;
				printk("skb->data[%d] = 0x%02x, data = 0x%02x, start_data = 0x%02x, bulkout_buff_offset = %d, bulkout_len = %d, skb->len=%d\n", 
				i, skb->data[i], data, dev->bulkout_start_data, dev->bulkout_buff_offset, dev->bulkout_len, skb->len);
				memDump(skb->data, dev->bulkout_len, "otg rx:");
				break;
			}
			data++;
		}


		align_class = BULKOUT_CLASS;
		if(dev->bulkout_len >= END_DATA_LEN) {	//finish testing data length from START_DATA_LEN to END_DATA_LEN at some starting address
			if(dev->bulkout_buff_offset == END_DMA_ADDR) {	//finish testing at last starting address
				printk("bulk out test finish!\n");
				dev->bulkout_buff_offset = START_DMA_ADDR;
				dev->bulkout_start_data = START_DATA;
				dev->bulkout_len = START_DATA_LEN;
				/* 2010-10-21 krammer :  mark for transmit last packet,
				it will make host bulk in test finish normally*/
				//goto out;
			}
			else {
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
				dev->bulkout_start_data++;	//change starting data
			}
		}
		else {	//next data length for next tx testing at the same starting address
			dev->bulkout_len++;
			dev->bulkout_start_data++;	//change starting data
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
		if(dev->bulkout_buff_offset & 0x3ff) {	//starting address is not at 1K boundary
			if (((dev->bulkout_buff_offset + dev->bulkout_len) >> 10) !=
				(dev->bulkout_buff_offset >> 10)) { // over 1k boundary
				rest = ((dev->bulkout_buff_offset + dev->bulkout_len)&0x3ff);
				if ((rest >= 4) && (rest <= 7)) {
					dev->bulkout_len = dev->bulkout_len + 8 - rest;
#ifdef SKIP_64_REST				
					goto check_length_again;
#endif
				}
			}
		}
#endif	//SKIP_CROSS_1K_4_7
#endif	//RX_ONLY
#ifndef TX_ONLY
		eth_start_xmit(skb, dev->net);
		skb = NULL;
		break;
#endif	//TX_ONLY
#else	//TEST_MODE
		skb->protocol = eth_type_trans(skb, dev->net);
		status = netif_rx(skb);
		skb = NULL;
#endif
		break;

	/* software-driven interface shutdown */
	case -ECONNRESET:		/* unlink */
	case -ESHUTDOWN:		/* disconnect etc */
		VDBG(dev, "rx shutdown, code %d\n", status);
		goto quiesce;

	/* for hardware automagic (such as pxa) */
	case -ECONNABORTED:		/* endpoint reset */
		DBG(dev, "rx %s reset\n", ep->name);
		defer_kevent(dev, WORK_RX_MEMORY);
quiesce:
		dev_kfree_skb_any(skb);
		goto clean;

	/* data overrun */
	case -EOVERFLOW:
		dev->net->stats.rx_over_errors++;
		/* FALLTHROUGH */

	default:
		dev->net->stats.rx_errors++;
		DBG(dev, "rx status %d\n", status);
		break;
	}

	if (skb)
		dev_kfree_skb_any(skb);
	if (!netif_running(dev->net)) {
clean:
		spin_lock(&dev->req_lock);
		list_add(&req->list, &dev->rx_reqs);
		spin_unlock(&dev->req_lock);
		req = NULL;
	}
	if (req)
		rx_submit(dev, req, GFP_ATOMIC);
}

static int prealloc(struct list_head *list, struct usb_ep *ep, unsigned n)
{
	unsigned		i;
	struct usb_request	*req;

	if (!n)
		return -ENOMEM;

	/* queue/recycle up to N requests */
	i = n;
	list_for_each_entry(req, list, list) {
		if (i-- == 0)
			goto extra;
	}
	while (i--) {
		req = usb_ep_alloc_request(ep, GFP_ATOMIC);
		if (!req)
			return list_empty(list) ? -ENOMEM : 0;
		list_add(&req->list, list);
	}
	return 0;

extra:
	/* free extras */
	for (;;) {
		struct list_head	*next;

		next = req->list.next;
		list_del(&req->list);
		usb_ep_free_request(ep, req);

		if (next == list)
			break;

		req = container_of(next, struct usb_request, list);
	}
	return 0;
}

static int alloc_requests(struct eth_dev *dev, struct gether *link, unsigned n)
{
	int	status;

	spin_lock(&dev->req_lock);
	status = prealloc(&dev->tx_reqs, link->in_ep, n);
	if (status < 0)
		goto fail;
	status = prealloc(&dev->rx_reqs, link->out_ep, n);
	if (status < 0)
		goto fail;
	goto done;
fail:
	DBG(dev, "can't alloc requests\n");
done:
	spin_unlock(&dev->req_lock);
	return status;
}

static void rx_fill(struct eth_dev *dev, gfp_t gfp_flags)
{
	struct usb_request	*req;
	unsigned long		flags;

	/* fill unused rxq slots with some skb */
	spin_lock_irqsave(&dev->req_lock, flags);
	while (!list_empty(&dev->rx_reqs)) {
		req = container_of(dev->rx_reqs.next,
				struct usb_request, list);
		list_del_init(&req->list);
		spin_unlock_irqrestore(&dev->req_lock, flags);

		if (rx_submit(dev, req, gfp_flags) < 0) {
			defer_kevent(dev, WORK_RX_MEMORY);
			return;
		}

		spin_lock_irqsave(&dev->req_lock, flags);
	}
	spin_unlock_irqrestore(&dev->req_lock, flags);
}

static void eth_work(struct work_struct *work)
{
	struct eth_dev	*dev = container_of(work, struct eth_dev, work);

	if (test_and_clear_bit(WORK_RX_MEMORY, &dev->todo)) {
		if (netif_running(dev->net))
			rx_fill(dev, GFP_KERNEL);
	}

	if (dev->todo)
		DBG(dev, "work done, flags = 0x%lx\n", dev->todo);
}

static void tx_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct sk_buff	*skb = req->context;
	struct eth_dev	*dev = ep->driver_data;

	switch (req->status) {
	default:
		dev->net->stats.tx_errors++;
		VDBG(dev, "tx err %d\n", req->status);
		/* FALLTHROUGH */
	case -ECONNRESET:		/* unlink */
	case -ESHUTDOWN:		/* disconnect etc */
		break;
	case 0:
		dev->net->stats.tx_bytes += skb->len;
	}
	dev->net->stats.tx_packets++;

	spin_lock(&dev->req_lock);
	list_add(&req->list, &dev->tx_reqs);
	spin_unlock(&dev->req_lock);
	dev_kfree_skb_any(skb);

	atomic_dec(&dev->tx_qlen);
	if (netif_carrier_ok(dev->net))
		netif_wake_queue(dev->net);
}

static inline int is_promisc(u16 cdc_filter)
{
	return cdc_filter & USB_CDC_PACKET_TYPE_PROMISCUOUS;
}

static int eth_start_xmit(struct sk_buff *skb, struct net_device *net)
{
	struct eth_dev		*dev = netdev_priv(net);
	int			length = skb->len;
	int			retval;
	struct usb_request	*req = NULL;
	unsigned long		flags;
	struct usb_ep		*in;
	u16			cdc_filter;
#ifdef CONFIG_DWC_OTG_DEVICE_ONLY
	//cathy
	int offset=0;
	struct sk_buff *skb_new;
#endif

	spin_lock_irqsave(&dev->lock, flags);
	if (dev->port_usb) {
		in = dev->port_usb->in_ep;
		cdc_filter = dev->port_usb->cdc_filter;
	} else {
		in = NULL;
		cdc_filter = 0;
	}
	spin_unlock_irqrestore(&dev->lock, flags);

	if (!in) {
		dev_kfree_skb_any(skb);
		return 0;
	}

	/* apply outgoing CDC or RNDIS filters */
	if (!is_promisc(cdc_filter)) {
		u8		*dest = skb->data;

		if (is_multicast_ether_addr(dest)) {
			u16	type;

			/* ignores USB_CDC_PACKET_TYPE_MULTICAST and host
			 * SET_ETHERNET_MULTICAST_FILTERS requests
			 */
			if (is_broadcast_ether_addr(dest))
				type = USB_CDC_PACKET_TYPE_BROADCAST;
			else
				type = USB_CDC_PACKET_TYPE_ALL_MULTICAST;
			if (!(cdc_filter & type)) {
				dev_kfree_skb_any(skb);
				return 0;
			}
		}
		/* ignores USB_CDC_PACKET_TYPE_DIRECTED */
	}

	spin_lock_irqsave(&dev->req_lock, flags);
	/*
	 * this freelist can be empty if an interrupt triggered disconnect()
	 * and reconfigured the gadget (shutting down this queue) after the
	 * network stack decided to xmit but before we got the spinlock.
	 */
	if (list_empty(&dev->tx_reqs)) {
		spin_unlock_irqrestore(&dev->req_lock, flags);
		return 1;
	}

	req = container_of(dev->tx_reqs.next, struct usb_request, list);
	list_del(&req->list);

	/* temporarily stop TX queue when the freelist empties */
	if (list_empty(&dev->tx_reqs))
		netif_stop_queue(net);
	spin_unlock_irqrestore(&dev->req_lock, flags);

	/* no buffer copies needed, unless the network stack did it
	 * or the hardware can't use skb buffers.
	 * or there's not enough space for extra headers we need
	 */
	if (dev->wrap) {
		struct sk_buff	*skb_new;

		skb_new = dev->wrap(skb);
		if (!skb_new)
			goto drop;

		#ifdef SPEED_UP_TX
		if(skb_new != skb){
			dev_kfree_skb_any(skb);
		}
		#else
		dev_kfree_skb_any(skb);
		#endif
		skb = skb_new;
		length = skb->len;
	}
#ifdef CONFIG_DWC_OTG_DEVICE_ONLY
	//cathy, for usb dma address alignment problem
	offset = (u32)skb->data%4;
	if( offset ) {
		//printk("[%s %d] tx buffer is not 4 bytes aligned!\n", __func__, __LINE__);
		skb_new = skb_copy_expand(skb, skb_headroom(skb)+offset, 0, GFP_ATOMIC);
		req->buf = skb_new->data;
		req->context = skb_new;
		dev_kfree_skb(skb); 
		skb = skb_new;
	}
	else {
		req->buf = skb->data;
		req->context = skb;
	}
#else
	req->buf = skb->data;
	req->context = skb;
#endif
	req->complete = tx_complete;

	/* use zlp framing on tx for strict CDC-Ether conformance,
	 * though any robust network rx path ignores extra padding.
	 * and some hardware doesn't like to write zlps.
	 */
	req->zero = 1;
	if (!dev->zlp && (length % in->maxpacket) == 0)
		length++;

	req->length = length;

	/* throttle highspeed IRQ rate back slightly */
	if (gadget_is_dualspeed(dev->gadget))
		req->no_interrupt = (dev->gadget->speed == USB_SPEED_HIGH)
			? ((atomic_read(&dev->tx_qlen) % qmult) != 0)
			: 0;

	retval = usb_ep_queue(in, req, GFP_ATOMIC);
	switch (retval) {
	default:
		DBG(dev, "tx queue err %d\n", retval);
		break;
	case 0:
		net->trans_start = jiffies;
		atomic_inc(&dev->tx_qlen);
	}

	if (retval) {
drop:
		dev->net->stats.tx_dropped++;
		dev_kfree_skb_any(skb);
		spin_lock_irqsave(&dev->req_lock, flags);
		if (list_empty(&dev->tx_reqs))
			netif_start_queue(net);
		list_add(&req->list, &dev->tx_reqs);
		spin_unlock_irqrestore(&dev->req_lock, flags);
	}
	return 0;
}

/*-------------------------------------------------------------------------*/

static void eth_start(struct eth_dev *dev, gfp_t gfp_flags)
{
	DBG(dev, "%s\n", __func__);

	/* fill the rx queue */
	rx_fill(dev, gfp_flags);

	/* and open the tx floodgates */
	atomic_set(&dev->tx_qlen, 0);
	netif_wake_queue(dev->net);
}

static int eth_open(struct net_device *net)
{
	struct eth_dev	*dev = netdev_priv(net);
	struct gether	*link;
#ifdef CONFIG_DWC_OTG_DEVICE_ONLY
#define USB2_PHY_DELAY __delay(200)
	u32 tmp = REG32(BSP_USB_PHY_CTRL);
	//090929 cathy start
	USBLED_CONTROL_t	usbled;
	/////090929 cathy end
#endif

	DBG(dev, "%s\n", __func__);
	if (netif_carrier_ok(dev->net))
		eth_start(dev, GFP_KERNEL);

#ifdef CONFIG_DWC_OTG_DEVICE_ONLY
	tmp = tmp & 0xFF00FF00;
	REG32(BSP_USB_PHY_CTRL) = tmp; USB2_PHY_DELAY;	//cathy, set reg(0xf2)=0x00; to hang on pull-up resistor
	REG32(0xb8030034) = 0x00024002; USB2_PHY_DELAY;	
	REG32(0xb8030034) = 0x00024000; USB2_PHY_DELAY;	
	REG32(0xb8030034) = 0x00024002; USB2_PHY_DELAY;	
	REG32(0xb8030034) = 0x000F4002; USB2_PHY_DELAY;	
	REG32(0xb8030034) = 0x000F4000; USB2_PHY_DELAY;	
	REG32(0xb8030034) = 0x000F4002; USB2_PHY_DELAY;	

	//090929 cathy start
	usbled.d32 = REG32(USBLED);
	usbled.b.en_usb_led_1_n = USBLED_ENABLE;
	usbled.b.usb_led_bs_1 = USBLED_BLINK;
	usbled.b.usb_speed_sel_1 = USBLED_ALL_SPEED;
	if(IS_6085 || IS_RLE0315 || IS_6166 || IS_RTL8676) {
		usbled.b.s_usbotg_fs_termsel = HANG_UP_RPU;
	}	
	REG32(USBLED) = usbled.d32;
#ifdef TEST_MODE
		dev->bulkout_buff_offset = START_DMA_ADDR;
		dev->bulkout_start_data = START_DATA;
		dev->bulkout_len = START_DATA_LEN;
#endif
#endif
	#ifdef SKIP_64_REST 
	printk("SKIP_64_REST\n");
	#endif
	#ifdef SKIP_CROSS_1K_4_7
	printk("SKIP_CROSS_1K_4_7\n");
	#endif

	spin_lock_irq(&dev->lock);
	link = dev->port_usb;
	if (link && link->open)
		link->open(link);
	spin_unlock_irq(&dev->lock);

	return 0;
}

static int eth_stop(struct net_device *net)
{
	struct eth_dev	*dev = netdev_priv(net);
	unsigned long	flags;

	VDBG(dev, "%s\n", __func__);
	netif_stop_queue(net);

	DBG(dev, "stop stats: rx/tx %ld/%ld, errs %ld/%ld\n",
		dev->net->stats.rx_packets, dev->net->stats.tx_packets,
		dev->net->stats.rx_errors, dev->net->stats.tx_errors
		);

	/* ensure there are no more active requests */
	spin_lock_irqsave(&dev->lock, flags);
	if (dev->port_usb) {
		struct gether	*link = dev->port_usb;

		if (link->close)
			link->close(link);

		/* NOTE:  we have no abort-queue primitive we could use
		 * to cancel all pending I/O.  Instead, we disable then
		 * reenable the endpoints ... this idiom may leave toggle
		 * wrong, but that's a self-correcting error.
		 *
		 * REVISIT:  we *COULD* just let the transfers complete at
		 * their own pace; the network stack can handle old packets.
		 * For the moment we leave this here, since it works.
		 */
		usb_ep_disable(link->in_ep);
		usb_ep_disable(link->out_ep);
		if (netif_carrier_ok(net)) {
			DBG(dev, "host still using in/out endpoints\n");
			usb_ep_enable(link->in_ep, link->in);
			usb_ep_enable(link->out_ep, link->out);
		}
	}
	spin_unlock_irqrestore(&dev->lock, flags);

	return 0;
}

/*-------------------------------------------------------------------------*/

/* initial value, changed by "ifconfig usb0 hw ether xx:xx:xx:xx:xx:xx" */
static char *dev_addr;
module_param(dev_addr, charp, S_IRUGO);
MODULE_PARM_DESC(dev_addr, "Device Ethernet Address");

/* this address is invisible to ifconfig */
static char *host_addr;
module_param(host_addr, charp, S_IRUGO);
MODULE_PARM_DESC(host_addr, "Host Ethernet Address");


static u8 __init nibble(unsigned char c)
{
	if (isdigit(c))
		return c - '0';
	c = toupper(c);
	if (isxdigit(c))
		return 10 + c - 'A';
	return 0;
}

static int __init get_ether_addr(const char *str, u8 *dev_addr)
{
	if (str) {
		unsigned	i;

		for (i = 0; i < 6; i++) {
			unsigned char num;

			if ((*str == '.') || (*str == ':'))
				str++;
			num = nibble(*str++) << 4;
			num |= (nibble(*str++));
			dev_addr [i] = num;
		}
		if (is_valid_ether_addr(dev_addr))
			return 0;
	}
	random_ether_addr(dev_addr);
	return 1;
}

static struct eth_dev *the_dev;

#ifdef TEST_MODE
static int eth_start_xmit2 (struct sk_buff *skb, struct net_device *net)
{
	dev_kfree_skb_any (skb);
	return 0;
}
#endif

#ifdef CONFIG_DWC_OTG_DEVICE_ONLY
int eth_mac_addr2(struct net_device *dev, void *p)
{
	struct eth_dev		*host_dev;
	int result = eth_mac_addr(dev, p);
	if(!result){
		host_dev = netdev_priv(dev);
		memcpy(hostaddr, dev->dev_addr, ETH_ALEN);
		hostaddr[ETH_ALEN - 1] += 1;
		memcpy(host_dev->host_mac, hostaddr, ETH_ALEN);
		rndis_set_host_mac(rndis_config_num, hostaddr);
		//printk("set host mac %02x%02x%02x%02x%02x%02x\n", hostaddr[0], hostaddr[1], hostaddr[2], hostaddr[3], hostaddr[4], hostaddr[5]);
	}
	return result;
}

#endif

static const struct net_device_ops eth_netdev_ops = {
	.ndo_open		= eth_open,
	.ndo_stop		= eth_stop,
#ifdef TEST_MODE
	.ndo_start_xmit		= eth_start_xmit2,
#else
	.ndo_start_xmit		= eth_start_xmit,
#endif
	.ndo_change_mtu		= ueth_change_mtu,
#ifdef CONFIG_DWC_OTG_DEVICE_ONLY
	.ndo_set_mac_address 	= eth_mac_addr2,
#else
	.ndo_set_mac_address 	= eth_mac_addr,
#endif
	.ndo_validate_addr	= eth_validate_addr,
};

/**
 * gether_setup - initialize one ethernet-over-usb link
 * @g: gadget to associated with these links
 * @ethaddr: NULL, or a buffer in which the ethernet address of the
 *	host side of the link is recorded
 * Context: may sleep
 *
 * This sets up the single network link that may be exported by a
 * gadget driver using this framework.  The link layer addresses are
 * set up using module parameters.
 *
 * Returns negative errno, or zero on success
 */
int __init gether_setup(struct usb_gadget *g, u8 ethaddr[ETH_ALEN])
{
	struct eth_dev		*dev;
	struct net_device	*net;
	int			status;

	if (the_dev)
		return -EBUSY;

	net = alloc_etherdev(sizeof *dev);
	if (!net)
		return -ENOMEM;

	dev = netdev_priv(net);
	spin_lock_init(&dev->lock);
	spin_lock_init(&dev->req_lock);
	INIT_WORK(&dev->work, eth_work);
	INIT_LIST_HEAD(&dev->tx_reqs);
	INIT_LIST_HEAD(&dev->rx_reqs);

	/* network device setup */
	dev->net = net;
	strcpy(net->name, "usb%d");

	if (get_ether_addr(dev_addr, net->dev_addr))
		dev_warn(&g->dev,
			"using random %s ethernet address\n", "self");
	if (get_ether_addr(host_addr, dev->host_mac))
		dev_warn(&g->dev,
			"using random %s ethernet address\n", "host");

	if (ethaddr)
		memcpy(ethaddr, dev->host_mac, ETH_ALEN);

	net->netdev_ops = &eth_netdev_ops;

	SET_ETHTOOL_OPS(net, &ops);

	/* two kinds of host-initiated state changes:
	 *  - iff DATA transfer is active, carrier is "on"
	 *  - tx queueing enabled if open *and* carrier is "on"
	 */
	netif_stop_queue(net);
	netif_carrier_off(net);

	dev->gadget = g;
	SET_NETDEV_DEV(net, &g->dev);

	status = register_netdev(net);
	if (status < 0) {
		dev_dbg(&g->dev, "register_netdev failed, %d\n", status);
		free_netdev(net);
	} else {
		INFO(dev, "MAC %pM\n", net->dev_addr);
		INFO(dev, "HOST MAC %pM\n", dev->host_mac);

		the_dev = dev;
	}

	return status;
}

/**
 * gether_cleanup - remove Ethernet-over-USB device
 * Context: may sleep
 *
 * This is called to free all resources allocated by @gether_setup().
 */
void gether_cleanup(void)
{
	if (!the_dev)
		return;

	unregister_netdev(the_dev->net);
	free_netdev(the_dev->net);

	/* assuming we used keventd, it must quiesce too */
	flush_scheduled_work();

	the_dev = NULL;
}


/**
 * gether_connect - notify network layer that USB link is active
 * @link: the USB link, set up with endpoints, descriptors matching
 *	current device speed, and any framing wrapper(s) set up.
 * Context: irqs blocked
 *
 * This is called to activate endpoints and let the network layer know
 * the connection is active ("carrier detect").  It may cause the I/O
 * queues to open and start letting network packets flow, but will in
 * any case activate the endpoints so that they respond properly to the
 * USB host.
 *
 * Verify net_device pointer returned using IS_ERR().  If it doesn't
 * indicate some error code (negative errno), ep->driver_data values
 * have been overwritten.
 */
struct net_device *gether_connect(struct gether *link)
{
	struct eth_dev		*dev = the_dev;
	int			result = 0;

	if (!dev)
		return ERR_PTR(-EINVAL);

	link->in_ep->driver_data = dev;
	result = usb_ep_enable(link->in_ep, link->in);
	if (result != 0) {
		DBG(dev, "enable %s --> %d\n",
			link->in_ep->name, result);
		goto fail0;
	}

	link->out_ep->driver_data = dev;
	result = usb_ep_enable(link->out_ep, link->out);
	if (result != 0) {
		DBG(dev, "enable %s --> %d\n",
			link->out_ep->name, result);
		goto fail1;
	}

	if (result == 0)
		result = alloc_requests(dev, link, qlen(dev->gadget));

	if (result == 0) {
		dev->zlp = link->is_zlp_ok;
		DBG(dev, "qlen %d\n", qlen(dev->gadget));

		dev->header_len = link->header_len;
		dev->unwrap = link->unwrap;
		dev->wrap = link->wrap;

		spin_lock(&dev->lock);
		dev->port_usb = link;
		link->ioport = dev;
		if (netif_running(dev->net)) {
			if (link->open)
				link->open(link);
		} else {
			if (link->close)
				link->close(link);
		}
		spin_unlock(&dev->lock);

		netif_carrier_on(dev->net);
		if (netif_running(dev->net))
			eth_start(dev, GFP_ATOMIC);

	/* on error, disable any endpoints  */
	} else {
		(void) usb_ep_disable(link->out_ep);
fail1:
		(void) usb_ep_disable(link->in_ep);
	}
fail0:
	/* caller is responsible for cleanup on error */
	if (result < 0)
		return ERR_PTR(result);
	return dev->net;
}

/**
 * gether_disconnect - notify network layer that USB link is inactive
 * @link: the USB link, on which gether_connect() was called
 * Context: irqs blocked
 *
 * This is called to deactivate endpoints and let the network layer know
 * the connection went inactive ("no carrier").
 *
 * On return, the state is as if gether_connect() had never been called.
 * The endpoints are inactive, and accordingly without active USB I/O.
 * Pointers to endpoint descriptors and endpoint private data are nulled.
 */
void gether_disconnect(struct gether *link)
{
	struct eth_dev		*dev = link->ioport;
	struct usb_request	*req;

	WARN_ON(!dev);
	if (!dev)
		return;

	DBG(dev, "%s\n", __func__);

	netif_stop_queue(dev->net);
	netif_carrier_off(dev->net);

	/* disable endpoints, forcing (synchronous) completion
	 * of all pending i/o.  then free the request objects
	 * and forget about the endpoints.
	 */
	usb_ep_disable(link->in_ep);
	spin_lock(&dev->req_lock);
	while (!list_empty(&dev->tx_reqs)) {
		req = container_of(dev->tx_reqs.next,
					struct usb_request, list);
		list_del(&req->list);

		spin_unlock(&dev->req_lock);
		usb_ep_free_request(link->in_ep, req);
		spin_lock(&dev->req_lock);
	}
	spin_unlock(&dev->req_lock);
	link->in_ep->driver_data = NULL;
	link->in = NULL;

	usb_ep_disable(link->out_ep);
	spin_lock(&dev->req_lock);
	while (!list_empty(&dev->rx_reqs)) {
		req = container_of(dev->rx_reqs.next,
					struct usb_request, list);
		list_del(&req->list);

		spin_unlock(&dev->req_lock);
		usb_ep_free_request(link->out_ep, req);
		spin_lock(&dev->req_lock);
	}
	spin_unlock(&dev->req_lock);
	link->out_ep->driver_data = NULL;
	link->out = NULL;

	/* finish forgetting about this USB link episode */
	dev->header_len = 0;
	dev->unwrap = NULL;
	dev->wrap = NULL;

	spin_lock(&dev->lock);
	dev->port_usb = NULL;
	link->ioport = NULL;
	spin_unlock(&dev->lock);
}
