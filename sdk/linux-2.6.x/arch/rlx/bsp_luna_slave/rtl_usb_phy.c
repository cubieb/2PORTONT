/*
 * Copyright 2011, Realtek Semiconductor Corp.
 *
 * arch/rlx/bsp/rtl_usb_phy.c
 *
 * $Author: cathy $
 * 
 * USB PHY access and USB MAC identification
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <asm/delay.h>
#include <bspchip.h>
#include <rtl_usb_phy.h>

#define USB2_PHY_DELAY __delay(200)

inline int mdio_ready(union usb3_mdio *mdio)
{
	int retry = 1000;

	do {
		mdio->d32 = REG32(BSP_USB3_MDIO);
		if ((mdio->b.status == MDIO_IDLE) && (mdio->b.ready == MDIO_READY) && (mdio->b.reset != RESET_ACTIVE))
			return 1;

		__delay(1000);
	} while (--retry);

	return 0;
}

void xhci_usb3_phy_write(unsigned char page, unsigned char reg, unsigned short data)
{
	union usb3_mdio mdio = {.d32 = 0};

	if (unlikely(!mdio_ready(&mdio)))
		goto err;

	mdio.b.data = data;
	mdio.b.page = page;
	mdio.b.reg = reg;
	mdio.b.cmd = MDIO_WRITE;
	REG32(BSP_USB3_MDIO) = mdio.d32;

	if (unlikely(!mdio_ready(&mdio)))
		goto err;

	return;

err:
	printk("%s: write fail!\n", __func__);
	return;
}
EXPORT_SYMBOL(xhci_usb3_phy_write);

unsigned short xhci_usb3_phy_read(unsigned char page, unsigned char reg)
{
	union usb3_mdio mdio = {.d32 = 0};

	if (unlikely(!mdio_ready(&mdio)))
		goto err;

	mdio.b.page = page;
	mdio.b.reg = reg;
	mdio.b.cmd = MDIO_READ;
	REG32(BSP_USB3_MDIO) = mdio.d32;

	if (unlikely(!mdio_ready(&mdio)))
		goto err;

	printk("%s: page:0x%02x, reg:0x%02x, data:0x%04x\n", __func__, page, reg, mdio.b.data);
	return mdio.b.data;

err:
	printk("%s: read fail! port 0, page:0x%02x, reg:0x%02x\n", __func__, page, reg);
	return 0;
}
EXPORT_SYMBOL(xhci_usb3_phy_read);

void xhci_usb2_phy_write(unsigned char reg, unsigned char val)
{
	unsigned int tmp = REG32(BSP_USB_PHY_CTRL);

	if ((reg < 0xE0) || (reg > 0xF6) || ((reg>0xE7)&&(reg<0xF0))) {
		printk("XHCI_USB2: Wrong register address: 0x%02x\n", reg);
		return;
	}

	//XHCI is on PHY0
	tmp = tmp & 0xFF00FF00;
	REG32(BSP_USB_PHY_CTRL) = val | tmp; USB2_PHY_DELAY;
	REG32(BSP_XHCI_USB2_PHY_CTRL) = ((reg & 0x0F) << 16) | 0x00000002; USB2_PHY_DELAY;
	REG32(BSP_XHCI_USB2_PHY_CTRL) = ((reg & 0xF0) << 12) | 0x00000003; USB2_PHY_DELAY;
	
	return;
}
EXPORT_SYMBOL(xhci_usb2_phy_write);

unsigned char xhci_usb2_phy_read(unsigned char reg)
{
	unsigned char val;

	if ((reg < 0xE0) || (reg > 0xF6) || ((reg>0xE7)&&(reg<0xF0))) {
		printk("XHCI_USB2: Wrong register address: 0x%02x\n", reg);
		return 0;
	}

	reg = reg - 0x20;

	REG32(BSP_XHCI_USB2_PHY_CTRL) = ((reg & 0x0F) << 16) | 0x00000002; USB2_PHY_DELAY;
	REG32(BSP_XHCI_USB2_PHY_CTRL) = ((reg & 0xF0) << 12) | 0x00000003; USB2_PHY_DELAY;
	REG32(BSP_XHCI_USB2_PHY_CTRL) = 0x00000002; USB2_PHY_DELAY;

	val = (REG32(BSP_XHCI_USB2_PHY_CTRL) & 0xFF000000) >> 24;

	//XHCI is on PHY0
	printk("XHCI_USB2: phy port 0, phy(0x%02x) = 0x%02x\n\n", reg + 0x20, val);

	return val;
}
EXPORT_SYMBOL(xhci_usb2_phy_read);

void ehci_phy_write(unsigned char phy_port, unsigned char reg, unsigned char val)
{
	unsigned int tmp = REG32(BSP_USB_PHY_CTRL);

	if (phy_port != 1) {
		printk("EHCI: phy port should be 1!\n");
		return;
	}

	if ((reg < 0xE0) || (reg > 0xF6) || ((reg>0xE7)&&(reg<0xF0))) {
		printk("EHCI: Wrong register address: 0x%02x\n", reg);
		return;
	}

	//ehci macPort 0 at PHY1
	tmp = tmp & 0xFF00FF00;
	REG32(BSP_USB_PHY_CTRL) = (val << 16) | tmp; USB2_PHY_DELAY;
	REG32(BSP_EHCI_UTMI_CTRL) = ((reg & 0x0F) << 16) | 0x00300000; USB2_PHY_DELAY;
	REG32(BSP_EHCI_UTMI_CTRL) = ((reg & 0x0F) << 16) | 0x00200000; USB2_PHY_DELAY;
	REG32(BSP_EHCI_UTMI_CTRL) = ((reg & 0x0F) << 16) | 0x00300000; USB2_PHY_DELAY;
	REG32(BSP_EHCI_UTMI_CTRL) = ((reg & 0xF0) << 12) | 0x00300000; USB2_PHY_DELAY;
	REG32(BSP_EHCI_UTMI_CTRL) = ((reg & 0xF0) << 12) | 0x00200000; USB2_PHY_DELAY;
	REG32(BSP_EHCI_UTMI_CTRL) = ((reg & 0xF0) << 12) | 0x00300000; USB2_PHY_DELAY;

	return;
}
EXPORT_SYMBOL(ehci_phy_write);

unsigned char ehci_phy_read(unsigned char phy_port, unsigned char reg)
{
	unsigned char val;

	if (phy_port != 1) {
		printk("EHCI: phy port should be 1!\n");
		return 0;
	}

	if ((reg < 0xE0) || (reg > 0xF6) || ((reg>0xE7)&&(reg<0xF0))) {
		printk("EHCI: Wrong register address: 0x%02x\n", reg);
		return 0;
	}

	reg = reg - 0x20;

	//ehci mac port 0 at PHY1
	REG32(BSP_EHCI_UTMI_CTRL) = ((reg & 0x0F) << 16) | 0x00300000; USB2_PHY_DELAY;
	REG32(BSP_EHCI_UTMI_CTRL) = ((reg & 0x0F) << 16) | 0x00200000; USB2_PHY_DELAY;
	REG32(BSP_EHCI_UTMI_CTRL) = ((reg & 0x0F) << 16) | 0x00300000; USB2_PHY_DELAY;
	REG32(BSP_EHCI_UTMI_CTRL) = ((reg & 0xF0) << 12) | 0x00300000; USB2_PHY_DELAY;
	REG32(BSP_EHCI_UTMI_CTRL) = ((reg & 0xF0) << 12) | 0x00200000; USB2_PHY_DELAY;
	REG32(BSP_EHCI_UTMI_CTRL) = ((reg & 0xF0) << 12) | 0x00300000; USB2_PHY_DELAY;

	val = (REG32(BSP_EHCI_UTMI_CTRL) & 0xFF000000) >> 24;
	printk("EHCI: phy port %d, phy(0x%02x) = 0x%02x\n\n", phy_port, reg + 0x20, val);

	return val;
}
EXPORT_SYMBOL(ehci_phy_read);

void dwc_otg_phy_write(unsigned char reg, unsigned char val)
{
	printk("OTG is not supported!\n");

	return;
}
EXPORT_SYMBOL(dwc_otg_phy_write);

unsigned char dwc_otg_phy_read(unsigned char reg)
{
	printk("OTG is not supported!\n");
	
	return 0;
}
EXPORT_SYMBOL(dwc_otg_phy_read);

unsigned char usb_mac_identify(unsigned char phy_port)
{
	unsigned char mac;

	/* PHY0: XHCI, PHY1: EHCI */
	if ((phy_port != 0) && (phy_port != 1)) {
		printk("%s: incorrect port:%d\n", __func__, phy_port);
		return 0;
	}

	if (phy_port == 1)
		mac = USB_MAC_EHCI_P0;
	else
		mac = USB_MAC_XHCI;

	return mac;
}
EXPORT_SYMBOL(usb_mac_identify);

unsigned char usb_get_phy_port(const char* desc, unsigned char mac_port)
{
	/* PHY0: XHCI, PHY1: EHCI */
	if (!strcmp(desc, "ehci_hcd") || !strcmp(desc, "ohci_hcd"))
		return 1;
	else
		return 0;
}
EXPORT_SYMBOL(usb_get_phy_port);

extern unsigned int SOC_ID;

/* called by EHCI or OHCI */
void set_usbphy(void)
{
	unsigned int clk = REG32(BSP_SYS_CLK_SEL);

 	if(SOC_ID != 0x0371){
		REG32(BSP_IP_SEL) |= ((BSP_USBPHY_P1_EN)|(BSP_USBPHY_P0_EN));
	}

	//andrew, mantis 6066, bug 1
	REG32(0xb8021094) = 0x80008000; 

	/* EHCI is on PHY1 */
	ehci_phy_write(1, 0xE1, 0xA8);
	ehci_phy_write(1, 0xE2, 0x99);
	ehci_phy_write(1, 0xE5, 0x9D);
	ehci_phy_write(1, 0xE7, 0x3D);
	if (clk & BSP_SYS_CLK_SRC_40MHZ) {	/* clk src is 40MHz */
		ehci_phy_write(1, 0xF5, 0xC1);
	}

	return;
}
EXPORT_SYMBOL(set_usbphy);

/* called by XHCI */
void set_usbphy_xhci(void)
{
	unsigned int clk = REG32(BSP_SYS_CLK_SEL);

	/* XHCI is on PHY0 */
	xhci_usb2_phy_write(0xE1, 0xA8);
	xhci_usb2_phy_write(0xE2, 0x99);
	xhci_usb2_phy_write(0xE5, 0x9D);
	if (clk & BSP_SYS_CLK_SRC_40MHZ) {	/* clk src is 40MHz */
		xhci_usb2_phy_write(0xF5, 0xC1);
	}

	return;
}
EXPORT_SYMBOL(set_usbphy_xhci);

void usb_enable_IP(void)
{
	REG32(BSP_IP_SEL) |= (BSP_EN_USB_PHY0_USB2_USB3 | BSP_EN_USB_PHY1);

	return;
}
EXPORT_SYMBOL(usb_enable_IP);


