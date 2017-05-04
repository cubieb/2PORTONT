//cathy
#include <linux/platform_device.h>

static int
ohci_reset (struct usb_hcd *hcd)
{
	struct ohci_hcd	*ohci = hcd_to_ohci (hcd);

	ohci_hcd_init (ohci);
	return ohci_init (ohci);
}

//__devinit//linux 2.6.19
static int 
ohci_start (struct usb_hcd *hcd)
{
	struct ohci_hcd	*ohci = hcd_to_ohci (hcd);
	int		ret;

	/* NOTE: there may have already been a first reset, to
	 * keep bios/smm irqs from making trouble
	 */
	if ((ret = ohci_run (ohci)) < 0) {
		ohci_err (ohci, "can't start\n");
		ohci_stop (hcd);
		return ret;
	}
	return 0;
}
#if 0
#ifdef	CONFIG_PM

static int ohci_pci_suspend (struct usb_hcd *hcd, pm_message_t message)
{
	struct ohci_hcd	*ohci = hcd_to_ohci (hcd);
	unsigned long	flags;
	int		rc = 0;

	/* Root hub was already suspended. Disable irq emission and
	 * mark HW unaccessible, bail out if RH has been resumed. Use
	 * the spinlock to properly synchronize with possible pending
	 * RH suspend or resume activity.
	 *
	 * This is still racy as hcd->state is manipulated outside of
	 * any locks =P But that will be a different fix.
	 */
	spin_lock_irqsave (&ohci->lock, flags);
	if (hcd->state != HC_STATE_SUSPENDED) {
		rc = -EINVAL;
		goto bail;
	}
	ohci_writel(ohci, OHCI_INTR_MIE, &ohci->regs->intrdisable);
	(void)ohci_readl(ohci, &ohci->regs->intrdisable);

	/* make sure snapshot being resumed re-enumerates everything */
	if (message.event == PM_EVENT_PRETHAW)
		ohci_usb_reset(ohci);

	clear_bit(HCD_FLAG_HW_ACCESSIBLE, &hcd->flags);
 bail:
	spin_unlock_irqrestore (&ohci->lock, flags);

	return rc;
}


static int ohci_pci_resume (struct usb_hcd *hcd)
{
	set_bit(HCD_FLAG_HW_ACCESSIBLE, &hcd->flags);
	usb_hcd_resume_root_hub(hcd);
	return 0;
}

#endif	/* CONFIG_PM */
#endif

/*-------------------------------------------------------------------------*/

struct hc_driver ohci_hc_driver_rtl8672 = {
	.description =		hcd_name,
	.product_desc =		"OHCI Host Controller",
	.hcd_priv_size =	sizeof(struct ohci_hcd),

	/*
	 * generic hardware linkage
	 */
	.irq =			ohci_irq,
	.flags =		HCD_MEMORY | HCD_USB11,

	/*
	 * basic lifecycle operations
	 */
	.reset =		ohci_reset,
	.start =		ohci_start,
	.stop =			ohci_stop,
	.shutdown =		ohci_shutdown,
#if 0
#ifdef	CONFIG_PM
	/* these suspend/resume entries are for upstream PCI glue ONLY */
	.suspend =		ohci_pci_suspend,
	.resume =		ohci_pci_resume,
#endif
#endif
	/*
	 * managing i/o requests and associated device resources
	 */
	.urb_enqueue =		ohci_urb_enqueue,
	.urb_dequeue =		ohci_urb_dequeue,
	.endpoint_disable =	ohci_endpoint_disable,

	/*
	 * scheduling support
	 */
	.get_frame_number =	ohci_get_frame,

	/*
	 * root hub support
	 */
	.hub_status_data =	ohci_hub_status_data,
	.hub_control =		ohci_hub_control,
	// .hub_irq_enable =	ohci_rhsc_enable,
#ifdef	CONFIG_PM
	.bus_suspend =		ohci_bus_suspend,
	.bus_resume =		ohci_bus_resume,
#endif
	.start_port_reset =	ohci_start_port_reset,

#ifdef CONFIG_USB_PATCH_RTL8672
	.rtl8672_dma_process = rtl8672_hcd_dma_process,
#endif	//CONFIG_USB_PATCH_RTL8672
};

/*-------------------------------------------------------------------------*/

int ohci_usb_hcd_probe_rtl8672 (struct hc_driver *driver, struct platform_device *pdev)
{
	struct usb_hcd *hcd;
	struct resource *res;
	int	retval = 0;
	unsigned int rtl8672_usb_irq;	//cathy for 8672

	set_usbphy();

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!res) {
		printk("%s: get irq resource failed!\n", __FUNCTION__);
		retval = -ENODEV;
		goto out1;
	}
	rtl8672_usb_irq = res->start;
	
	hcd = usb_create_hcd(driver, &pdev->dev, dev_name(&pdev->dev));
	if (!hcd) {
		printk("%s: create hcd failed!\n", __FUNCTION__);
		retval = -ENOMEM;
		goto out1;
	}
	
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		printk("%s: get memory resource failed!\n", __FUNCTION__);
		retval = -ENODEV;
		goto out2;
	}
	#ifdef CONFIG_PCI
	hcd->regs = (void *) (res->start|0xb0000000);
	hcd->rsrc_start = (res->start|0xb0000000);	
	#else
	hcd->regs = (void *) res->start;
        hcd->rsrc_start = res->start;
	#endif
	hcd->rsrc_len = res->end - res->start;
	
	retval = usb_add_hcd (hcd, rtl8672_usb_irq, IRQF_SHARED);
	if (retval != 0)
		goto out2;

	return retval;
	
out2:
	usb_put_hcd(hcd);
out1:
	return retval;
}


int ohci_rtl8672_drv_probe (struct platform_device *pdev)
{
	return ohci_usb_hcd_probe_rtl8672 (&ohci_hc_driver_rtl8672, pdev);
}

static int ohci_rtl8672_drv_remove (struct platform_device *pdev)
{
	struct usb_hcd *hcd;
	int	retval = 0;

	hcd = platform_get_drvdata (pdev);

	if (!hcd) {
		printk("%s: get hcd failed!\n", __FUNCTION__);
		retval = -ENODEV;
		goto out;
	}

	usb_remove_hcd (hcd);

	if (hcd->driver->flags & HCD_MEMORY) {
		iounmap (hcd->regs);
		release_mem_region (hcd->rsrc_start, hcd->rsrc_len);
	}
	else {
		release_region (hcd->rsrc_start, hcd->rsrc_len);
	}
	usb_put_hcd (hcd);
	//the_controller = NULL;

out:
	return retval;
}

 struct platform_driver platform_driver_rtl8672_ohci = {
	.probe = ohci_rtl8672_drv_probe,
	.remove = ohci_rtl8672_drv_remove,
	.shutdown = usb_hcd_platform_shutdown,
	.driver = {
	    .name = "rtl8672-ohci",
	},
};
