/*
 * xHCI host controller driver Bus Glue for DWC.
 *
 * Copyright (C) 2009-2011 Tony Wu (tonywu@realtek.com)
 */

#include <linux/platform_device.h>

#include "xhci.h"

static const char hcd_name_xxxx[] = "xhci_dwc";

#define DWC_DCFG_DEVSPD_BITS    0x000007
#define DWC_DCFG_DEVSPD_SHIFT   0

/* called during probe() after chip reset completes */
static int xhci_dwc_setup(struct usb_hcd *hcd)
{
	struct xhci_hcd	*xhci = hcd_to_xhci(hcd);
	int retval;

	xhci->cap_regs = hcd->regs;
	xhci->op_regs = hcd->regs +
		HC_LENGTH(xhci_readl(xhci, &xhci->cap_regs->hc_capbase));
	xhci->run_regs = hcd->regs +
		(xhci_readl(xhci, &xhci->cap_regs->run_regs_off) & RTSOFF_MASK);

	/* Cache read-only capability registers */
	xhci->hcs_params1 = xhci_readl(xhci, &xhci->cap_regs->hcs_params1);
	xhci->hcs_params2 = xhci_readl(xhci, &xhci->cap_regs->hcs_params2);
	xhci->hcs_params3 = xhci_readl(xhci, &xhci->cap_regs->hcs_params3);
	xhci->hcc_params = xhci_readl(xhci, &xhci->cap_regs->hcc_params);
	xhci_print_registers(xhci);

	/* Make sure the HC is halted. */
	retval = xhci_halt(xhci);
	if (retval)
		return retval;

	xhci_dbg(xhci, "Resetting HCD\n");

	/* Reset the internal HC memory state and registers. */
	retval = xhci_reset(xhci);
	if (retval)
		return retval;
	xhci_dbg(xhci, "Reset complete\n");

	xhci_dbg(xhci, "Calling HCD init\n");
	/* Initialize HCD and host controller data structures. */
	retval = xhci_init(hcd);
	if (retval)
		return retval;
	xhci_dbg(xhci, "Called HCD init\n");

	return retval;
}

static const struct hc_driver xhci_dwc_hc_driver = {
	.description =		hcd_name_xxxx,////leroy////hcd_name,
	.product_desc =		"DWC xHCI Host Controller",
	.hcd_priv_size =	sizeof(struct xhci_hcd),

	/*
	 * generic hardware linkage
	 */
	.irq =			xhci_irq,
	.flags =		HCD_MEMORY | HCD_USB3,

	/*
	 * basic lifecycle operations
	 */
	.reset =		xhci_dwc_setup,
	.start =		xhci_run,
	/* suspend and resume implemented later */
	.stop =			xhci_stop,
	.shutdown =		xhci_shutdown,

	/*
	 * managing i/o requests and associated device resources
	 */
	.urb_enqueue =		xhci_urb_enqueue,
	.urb_dequeue =		xhci_urb_dequeue,
	.alloc_dev =		xhci_alloc_dev,
	.free_dev =		xhci_free_dev,
	.add_endpoint =		xhci_add_endpoint,
	.drop_endpoint =	xhci_drop_endpoint,
	.endpoint_reset =	xhci_endpoint_reset,
	.check_bandwidth =	xhci_check_bandwidth,
	.reset_bandwidth =	xhci_reset_bandwidth,
	.address_device =	xhci_address_device,

	/*
	 * scheduling support
	 */
	.get_frame_number =	xhci_get_frame,

	/* Root hub support */
	.hub_control =		xhci_hub_control,
	.hub_status_data =	xhci_hub_status_data,
};

/*-------------------------------------------------------------------------*/

int xhci_dwc_probe(struct platform_device *pdev)
{
	struct hc_driver *phcd;
	struct usb_hcd *hcd;
	struct resource *res;
	int retval;
	u32 dwc_usb3_irq; 

	phcd = (struct hc_driver *) &xhci_dwc_hc_driver;

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (res == NULL) {
		printk (KERN_ERR __FILE__ ": get irq resource failed!\n");
		retval = -ENOMEM;
		return retval;
	}

	dwc_usb3_irq = res->start;

	hcd = usb_create_hcd(phcd, &pdev->dev, dev_name(&pdev->dev));
	if (hcd == NULL) {
		retval = -ENOMEM;
		return retval;
	}

	if (hcd->driver->flags & HCD_MEMORY) {
		res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
		if (res == NULL) {
			printk(KERN_ERR __FILE__ ": get memory resource failed!\n");
			retval = -ENOMEM;
			return retval;
		}

		hcd->rsrc_start = res->start;
		hcd->rsrc_len = res->end - res->start + 1;
		if (request_mem_region(hcd->rsrc_start,
				       hcd->rsrc_len, "dwc_usb3") == NULL) {
			printk(KERN_ERR __FILE__ ": request_mem_region failed\n");
			retval = -ENOMEM;
			return retval;
		}

		hcd->regs = (void *) ioremap_nocache(hcd->rsrc_start, hcd->rsrc_len);
		if (hcd->regs == NULL) {
			printk(KERN_ERR __FILE__ ": ioremap failed\n");
			release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
			retval = -ENOMEM;
			return retval;
		}
	}

	retval = usb_add_hcd(hcd, dwc_usb3_irq, IRQF_DISABLED | IRQF_SHARED);
	if (retval != 0) {
		if (hcd->driver->flags & HCD_MEMORY) {
			iounmap(hcd->regs);
			release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
		} else
			release_region(hcd->rsrc_start, hcd->rsrc_len);
	}

	return retval;
}

static int xhci_dwc_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd;

	hcd = platform_get_drvdata (pdev);
	usb_remove_hcd (hcd);
	if (hcd->driver->flags & HCD_MEMORY) {
		iounmap (hcd->regs);
		release_mem_region (hcd->rsrc_start, hcd->rsrc_len);
	} else
		release_region (hcd->rsrc_start, hcd->rsrc_len);

	usb_put_hcd (hcd);
	return 0;
}

/*-------------------------------------------------------------------------*/

struct platform_driver xhci_dwc_driver = {
	.probe = xhci_dwc_probe,
	.remove = xhci_dwc_remove,
	.shutdown = usb_hcd_platform_shutdown,
	.driver = {
		.name = "dwc_usb3",
	},
};
