static void ehci_rtl8672_check_phy_timeout(unsigned long ptr)
{
	struct ehci_hcd *ehci = (struct ehci_hcd *)ptr;
	int port = ehci->port;
	unsigned int status = readl (&ehci->regs->port_status[port]);
	unsigned char tmp, phy_E6_val;
	static unsigned char reset[2] = {0, 0};

	printk("EHCI: port status=0x%08x\n", status);
	if((status & PORT_CONNECT) || (status & PORT_RESET)) {	//usb device is still connected or port reset is still in progress
		if(!(status&PORT_OWNER) && !(status&PORT_PE)) {		//if port owner is not ohci and port enable fails, increase disconnection threshold
			phy_E6_val = ehci_phy_read(port, 0xE6);
			if((phy_E6_val & 0xF0) != 0xF0) {
				if(!reset[port]) {
					reset[port] = phy_E6_val;		//backup original value of phy 0xE6
				}
				tmp = (phy_E6_val >> 4) + 1;
				phy_E6_val = (tmp << 4) | (phy_E6_val & 0x0F);
				writel(0, &ehci->regs->port_status[port]);	//turn off port power
				ehci_phy_write(port, 0xE6, phy_E6_val);	//increase reference voltage
				mdelay(10);
				writel(PORT_POWER, &ehci->regs->port_status[port]);	//turn on port power
				printk("EHCI: set phy(0xE6) to 0x%02x\n", phy_E6_val);
				mod_timer(&ehci->check_phy_timer, jiffies + 2*HZ);	//check again to avoid dead port reset condition
			}
			else {
				printk("EHCI: already reach highest ref. voltage\n");
			}
		}
	}
	else if(reset[port]) {	//restore the original value of phy 0xE6
		ehci_phy_write(port, 0xE6, reset[port]);
		printk("EHCI: reset phy(0xE6) to 0x%02x\n", reset[port]);
		reset[port] = 0;
	}

	return;
}

/* called during probe() after chip reset completes */
static int ehci_setup(struct usb_hcd *hcd)
{
	struct ehci_hcd		*ehci = hcd_to_ehci(hcd);
	int			retval = 0;

	ehci->caps = hcd->regs;
	ehci->regs = hcd->regs + HC_LENGTH(readl(&ehci->caps->hc_capbase));
	dbg_hcs_params(ehci, "reset");
	dbg_hcc_params(ehci, "reset");

	/* cache this readonly data; minimize chip reads */
	ehci->hcs_params = readl(&ehci->caps->hcs_params);
	retval = ehci_halt(ehci);
	if (retval)
		return retval;

	/* data structure init */
	retval = ehci_init(hcd);
	if (retval)
		return retval;

	ehci->port = 0;
	init_timer(&ehci->check_phy_timer);
	ehci->check_phy_timer.data = (unsigned long)ehci;
	ehci->check_phy_timer.function = ehci_rtl8672_check_phy_timeout;

	if (ehci_is_TDI(ehci))
		ehci_reset(ehci);

#ifdef	CONFIG_USB_SUSPEND
	/* REVISIT: the controller works fine for wakeup iff the root hub
	 * itself is "globally" suspended, but usbcore currently doesn't
	 * understand such things.
	 *
	 * System suspend currently expects to be able to suspend the entire
	 * device tree, device-at-a-time.  If we failed selective suspend
	 * reports, system suspend would fail; so the root hub code must claim
	 * success.  That's lying to usbcore, and it matters for for runtime
	 * PM scenarios with selective suspend and remote wakeup...
	 */
	if (ehci->no_selective_suspend && device_can_wakeup(&pdev->dev))
		ehci_warn(ehci, "selective suspend/wakeup unavailable\n");
#endif

	return retval;
}

static int ehci_usb_hcd_probe_rtl8672(struct hc_driver *driver, struct platform_device *pdev)
{
	struct usb_hcd *hcd;
	struct resource *res;
	int retval = 0;
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

void ehci_restart(struct usb_hcd *hcd)
{
	struct ehci_hcd *ehci = hcd_to_ehci(hcd);

	/* halt, reset EHCI, and cleanup EHCI memory */
	ehci_port_power(ehci, 0);
	ehci_halt(ehci);
	del_timer_sync(&ehci->watchdog);
	del_timer_sync(&ehci->check_phy_timer);
	spin_lock_irq(&ehci->lock);
	if(HC_IS_RUNNING(hcd->state))
		ehci_quiesce(ehci);
	ehci_reset(ehci);
	writel(0, &ehci->regs->intr_enable);
	remove_debug_files(ehci);
	if (ehci->async)
		ehci_work(ehci);
	spin_unlock_irq(&ehci->lock);
	ehci_mem_cleanup(ehci);

	/* allocate EHCI memory and start EHCI */
	ehci_setup(hcd);
	ehci_run(hcd);
	ehci_port_power (ehci, 1);

	return;
}
EXPORT_SYMBOL(ehci_restart);

struct hc_driver ehci_driver_rtl8672 = {
	.description =		hcd_name,
	.product_desc =		"EHCI Host Controller",
	.hcd_priv_size =	sizeof(struct ehci_hcd),
	/*
	 * generic hardware linkage
	 */
	.irq =			ehci_irq,
	.flags =		HCD_MEMORY | HCD_USB2,

	/*
	 * basic lifecycle operations
	 */
	.reset =		ehci_setup,
	.start =		ehci_run,
	.stop =			ehci_stop,
	.shutdown =		ehci_shutdown,

	/*
	 * managing i/o requests and associated device resources
	 */
	.urb_enqueue =		ehci_urb_enqueue,
	.urb_dequeue =		ehci_urb_dequeue,
	.endpoint_disable =	ehci_endpoint_disable,

	/*
	 * scheduling support
	 */
	.get_frame_number =	ehci_get_frame,

	/*
	 * root hub support
	 */
	.hub_status_data =	ehci_hub_status_data,
	.hub_control =		ehci_hub_control,
	.bus_suspend =		ehci_bus_suspend,
	.bus_resume =		ehci_bus_resume,
	.relinquish_port = ehci_relinquish_port,
	.port_handed_over = ehci_port_handed_over,
	.clear_tt_buffer_complete     = ehci_clear_tt_buffer_complete,
#ifdef CONFIG_USB_PATCH_RTL8672
	.rtl8672_dma_process = rtl8672_hcd_dma_process,
#endif	//CONFIG_USB_PATCH_RTL8672
};

static int ehci_rtl8672_drv_probe (struct platform_device *pdev)
{
	return ehci_usb_hcd_probe_rtl8672 (&ehci_driver_rtl8672, pdev);
}

static int ehci_rtl8672_drv_remove (struct platform_device *pdev)
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

out:
	return retval;
}

struct platform_driver platform_driver_rtl8672 = {
	.probe = ehci_rtl8672_drv_probe,
	.remove = ehci_rtl8672_drv_remove,
	.shutdown = usb_hcd_platform_shutdown,
	.driver = {
		.name = "rtl8672-ehci",
	},
};

