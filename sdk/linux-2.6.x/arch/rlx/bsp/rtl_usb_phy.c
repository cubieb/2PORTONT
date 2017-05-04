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
#include <asm/delay.h>
#include <bspchip.h>
#include <rtl_usb_phy.h>

#define USB2_PHY_DELAY __delay(200)

void xhci_usb3_phy_write(unsigned char page, unsigned char reg, unsigned short data)
{
	printk("XHCI is not supported!\n");

	return;
}
EXPORT_SYMBOL(xhci_usb3_phy_write);

unsigned short xhci_usb3_phy_read(unsigned char page, unsigned char reg)
{
	printk("XHCI is not supported!\n");

	return 0;
}
EXPORT_SYMBOL(xhci_usb3_phy_read);

void xhci_usb2_phy_write(unsigned char reg, unsigned char val)
{
	printk("XHCI is not supported!\n");
	
	return;
}
EXPORT_SYMBOL(xhci_usb2_phy_write);

unsigned char xhci_usb2_phy_read(unsigned char reg)
{
	printk("XHCI is not supported!\n");
	
	return 0;
}
EXPORT_SYMBOL(xhci_usb2_phy_read);

void ehci_phy_write(unsigned char phy_port, unsigned char reg, unsigned char val)
{
	unsigned int tmp = REG32(BSP_USB_PHY_CTRL);

	if ((phy_port != 0) && (phy_port != 1)) {
		printk("EHCI: phy port should be 0 or 1!\n");
		return;
	}

	if ((reg < 0xE0) || (reg > 0xF6) || ((reg>0xE7)&&(reg<0xF0))) {
		printk("EHCI: Wrong register address: 0x%02x\n", reg);
		return;
	}

	if (phy_port == 0) {	//port 0
		tmp = tmp & 0xFF00FF00;
		REG32(BSP_USB_PHY_CTRL) = val | tmp; USB2_PHY_DELAY;
		REG32(BSP_EHCI_UTMI_CTRL) = ((reg & 0x0F) << 16) | 0x00300000; USB2_PHY_DELAY;
		REG32(BSP_EHCI_UTMI_CTRL) = ((reg & 0x0F) << 16) | 0x00200000; USB2_PHY_DELAY;
		REG32(BSP_EHCI_UTMI_CTRL) = ((reg & 0x0F) << 16) | 0x00300000; USB2_PHY_DELAY;
		REG32(BSP_EHCI_UTMI_CTRL) = ((reg & 0xF0) << 12) | 0x00300000; USB2_PHY_DELAY;
		REG32(BSP_EHCI_UTMI_CTRL) = ((reg & 0xF0) << 12) | 0x00200000; USB2_PHY_DELAY;
		REG32(BSP_EHCI_UTMI_CTRL) = ((reg & 0xF0) << 12) | 0x00300000; USB2_PHY_DELAY;
	}
	else {	//port 1
		tmp = tmp & 0xFF00FF00;
		REG32(BSP_USB_PHY_CTRL) = (val << 16) | tmp; USB2_PHY_DELAY;
		REG32(BSP_EHCI_UTMI_CTRL) = ((reg & 0x0F) << 16) | 0x00500000; USB2_PHY_DELAY;
		REG32(BSP_EHCI_UTMI_CTRL) = ((reg & 0x0F) << 16) | 0x00400000; USB2_PHY_DELAY;
		REG32(BSP_EHCI_UTMI_CTRL) = ((reg & 0x0F) << 16) | 0x00500000; USB2_PHY_DELAY;
		REG32(BSP_EHCI_UTMI_CTRL) = ((reg & 0xF0) << 12) | 0x00500000; USB2_PHY_DELAY;
		REG32(BSP_EHCI_UTMI_CTRL) = ((reg & 0xF0) << 12) | 0x00400000; USB2_PHY_DELAY;
		REG32(BSP_EHCI_UTMI_CTRL) = ((reg & 0xF0) << 12) | 0x00500000; USB2_PHY_DELAY;
	}

	return;
}
EXPORT_SYMBOL(ehci_phy_write);

unsigned char ehci_phy_read(unsigned char phy_port, unsigned char reg)
{
	unsigned char val;

	if ((phy_port != 0) && (phy_port != 1)) {
		printk("EHCI: Port should be 0 or 1!\n");
		return 0;
	}

	if ((reg < 0xE0) || (reg > 0xF6) || ((reg>0xE7)&&(reg<0xF0))) {
		printk("EHCI: Wrong register address: 0x%02x\n", reg);
		return 0;
	}

	reg = reg - 0x20;

	if (phy_port == 0) {	//port 0
		REG32(BSP_EHCI_UTMI_CTRL) = ((reg & 0x0F) << 16) | 0x00300000; USB2_PHY_DELAY;
		REG32(BSP_EHCI_UTMI_CTRL) = ((reg & 0x0F) << 16) | 0x00200000; USB2_PHY_DELAY;
		REG32(BSP_EHCI_UTMI_CTRL) = ((reg & 0x0F) << 16) | 0x00300000; USB2_PHY_DELAY;
		REG32(BSP_EHCI_UTMI_CTRL) = ((reg & 0xF0) << 12) | 0x00300000; USB2_PHY_DELAY;
		REG32(BSP_EHCI_UTMI_CTRL) = ((reg & 0xF0) << 12) | 0x00200000; USB2_PHY_DELAY;
		REG32(BSP_EHCI_UTMI_CTRL) = ((reg & 0xF0) << 12) | 0x00300000; USB2_PHY_DELAY;
	}
	else {	//port 1
		REG32(BSP_EHCI_UTMI_CTRL) = ((reg & 0x0F) << 16) | 0x00500000; USB2_PHY_DELAY;
		REG32(BSP_EHCI_UTMI_CTRL) = ((reg & 0x0F) << 16) | 0x00400000; USB2_PHY_DELAY;
		REG32(BSP_EHCI_UTMI_CTRL) = ((reg & 0x0F) << 16) | 0x00500000; USB2_PHY_DELAY;
		REG32(BSP_EHCI_UTMI_CTRL) = ((reg & 0xF0) << 12) | 0x00500000; USB2_PHY_DELAY;
		REG32(BSP_EHCI_UTMI_CTRL) = ((reg & 0xF0) << 12) | 0x00400000; USB2_PHY_DELAY;
		REG32(BSP_EHCI_UTMI_CTRL) = ((reg & 0xF0) << 12) | 0x00500000; USB2_PHY_DELAY;
	}

	val = (REG32(BSP_EHCI_UTMI_CTRL) & 0xFF000000) >> 24;
	printk("EHCI: phy port %d, phy(0x%02x) = 0x%02x\n\n", phy_port, reg + 0x20, val);

	return val;
}
EXPORT_SYMBOL(ehci_phy_read);

void dwc_otg_phy_write(unsigned char reg, unsigned char val)
{
	unsigned int tmp = REG32(BSP_USB_PHY_CTRL);

	if ((reg < 0xE0) || (reg > 0xF6) || ((reg>0xE7)&&(reg<0xF0))) {
		printk("DWC_OTG: Wrong register address: 0x%02x\n", reg);
		return;
	}
	
	tmp = tmp & 0xFF00FF00;
	REG32(BSP_USB_PHY_CTRL) = (val << 16) | tmp; USB2_PHY_DELAY;	//OTG is on PHY1
	REG32(BSP_DWC_OTG_PHY_CTRL) = ((reg & 0x0F) << 16) | 0x00004002; USB2_PHY_DELAY;
	REG32(BSP_DWC_OTG_PHY_CTRL) = ((reg & 0x0F) << 16) | 0x00004000; USB2_PHY_DELAY;
	REG32(BSP_DWC_OTG_PHY_CTRL) = ((reg & 0x0F) << 16) | 0x00004002; USB2_PHY_DELAY;
	REG32(BSP_DWC_OTG_PHY_CTRL) = ((reg & 0xF0) << 12) | 0x00004002; USB2_PHY_DELAY;
	REG32(BSP_DWC_OTG_PHY_CTRL) = ((reg & 0xF0) << 12) | 0x00004000; USB2_PHY_DELAY;
	REG32(BSP_DWC_OTG_PHY_CTRL) = ((reg & 0xF0) << 12) | 0x00004002; USB2_PHY_DELAY;

	return;
}
EXPORT_SYMBOL(dwc_otg_phy_write);

unsigned char dwc_otg_phy_read(unsigned char reg)
{
	unsigned char val = 0;

	if ((reg < 0xE0) || (reg > 0xF6) || ((reg>0xE7)&&(reg<0xF0))) {
		printk("DWC_OTG: Wrong register address: 0x%02x\n", reg);
		return 0;
	}
	
	reg = reg - 0x20;
	REG32(BSP_DWC_OTG_PHY_CTRL) = ((reg & 0x0F) << 16) | 0x00004002; USB2_PHY_DELAY;
	REG32(BSP_DWC_OTG_PHY_CTRL) = ((reg & 0x0F) << 16) | 0x00004000; USB2_PHY_DELAY;
	REG32(BSP_DWC_OTG_PHY_CTRL) = ((reg & 0x0F) << 16) | 0x00004002; USB2_PHY_DELAY;
	REG32(BSP_DWC_OTG_PHY_CTRL) = ((reg & 0xF0) << 12) | 0x00004002; USB2_PHY_DELAY;
	REG32(BSP_DWC_OTG_PHY_CTRL) = ((reg & 0xF0) << 12) | 0x00004000; USB2_PHY_DELAY;
	REG32(BSP_DWC_OTG_PHY_CTRL) = ((reg & 0xF0) << 12) | 0x00004002; USB2_PHY_DELAY;

	val = (REG32(BSP_DWC_OTG_PHY_CTRL) & 0xFF000000) >> 24;
	printk("DWC_OTG: phy(0x%02x) = 0x%02x\n", reg + 0x20, val);
	
	return val;
}
EXPORT_SYMBOL(dwc_otg_phy_read);

unsigned char usb_mac_identify(unsigned char phy_port)
{
	unsigned char mac;
	unsigned int pinsr = REG32(BSP_MISC_PINSR);

	/*
	*	Note:
	*	1. RL6085 has OTG only
	*	2. port 0 is only bonded with EHCI MAC 
	*	3. if port 1 is EHCI, ENUSBOTG bit is 0
	*	4. OTG is always at port 1
	*/
	if ((phy_port != 0) && (phy_port != 1)) {
		printk("%s: incorrect port:%d\n", __func__, phy_port);
		return 0;
	}

	if (IS_RTL8676) {
		unsigned int ipSel = REG32(BSP_IP_SEL);
		if (phy_port == 0 && (ipSel & BSP_EN_USB_HOST))
			mac = USB_MAC_EHCI_P0;
		else if (phy_port == 1 && (ipSel & BSP_EN_USB_OTG))
			mac = USB_MAC_OTG;
		else if (phy_port == 1 && (ipSel & BSP_EN_USB_HOST))
			mac = USB_MAC_EHCI_P1;
		else {
			printk("%s: incorrect ipSel = 0x%08x\n", __func__, ipSel);
			return 0;
		}
	}
	else if (IS_6085) {
		mac = USB_MAC_OTG;
	}
	else {
		if (!phy_port)
			mac = USB_MAC_EHCI_P0;
		else if (pinsr & BSP_EN_USB_OTG)
			mac = USB_MAC_OTG;
		else
			mac = USB_MAC_EHCI_P1;
	}

	return mac;
}
EXPORT_SYMBOL(usb_mac_identify);

unsigned char usb_get_phy_port(const char* desc, unsigned char mac_port)
{
	if (mac_port == 1)	/* EHCI port 1 on phy 1 */
		return 1;
	else if (!strcmp(desc, "ehci_hcd") || !strcmp(desc, "ohci_hcd"))	/* EHCI port 0 on phy 0 */
		return 0;
	else	/* OTG on phy 1 */
		return 1;
}
EXPORT_SYMBOL(usb_get_phy_port);

void set_usbphy(void)
{
	//andrew, mantis 6066, bug 1
	//if(IS_6028A || IS_6028B)
		REG32(0xb8021094) = 0x80008000; 

	/* port 0 */
	if (IS_RTL8676) {
		unsigned int ipSel = REG32(BSP_IP_SEL);

		if(ipSel & BSP_EN_USB_PHY0) {
			ehci_phy_write(0, 0xE2, 0x99);
			ehci_phy_write(0, 0xE5, 0x9D);
			if(!(REG32(BSP_MISC_PINSR) & BSP_CLKSEL))
				ehci_phy_write(0, 0xF5, 0xD1);	//clk:35.328MHZ
			else
				ehci_phy_write(0, 0xF5, 0xC1);	//clk:40MHZ
		}
		else {
			printk("USB PHY0 is disabled!\n");
		}
	}
	else if (IS_RLE0315 || IS_6166) {
		ehci_phy_write(0, 0xE0, 0xB8);
		ehci_phy_write(0, 0xE1, 0xA8);
		ehci_phy_write(0, 0xE2, 0x9A);
		ehci_phy_write(0, 0xE3, 0x41);
		ehci_phy_write(0, 0xE5, 0x91);
		ehci_phy_write(0, 0xE6, 0x88);
		ehci_phy_write(0, 0xF4, 0xE3);
	}
	else {
		ehci_phy_write(0, 0xE0, 0x91);
		ehci_phy_write(0, 0xE1, 0xA8);
		ehci_phy_write(0, 0xE2, 0x81);
		ehci_phy_write(0, 0xE5, 0x95);
		ehci_phy_write(0, 0xE6, 0xE8);
		ehci_phy_write(0, 0xE7, 0x38);
		ehci_phy_write(0, 0xF1, 0x8E);
	}

	/* port 1 */
	if (!usb_mac_identify(1)) {
		printk("USB phy 1 is routed to OTG, skip PHY 1 setting in set_usbphy\n");
		return;
	}
	
	if (IS_RTL8676) {
		unsigned int ipSel = REG32(BSP_IP_SEL);
		unsigned char val;

		if (ipSel & BSP_EN_USB_PHY1) {
			ehci_phy_write(1, 0xE2, 0x99);
			ehci_phy_write(1, 0xE5, 0x9D);
			if (!(REG32(BSP_MISC_PINSR) & BSP_CLKSEL))
				val = 0xD1;		//clk:35.328MHZ
			else
				val = 0xC1;		//clk:40MHZ
			if (!(ipSel & BSP_EN_USB_PHY0))
				ehci_phy_write(0, 0xF5, val);
			ehci_phy_write(1, 0xF5, val);
		}
		else {
			printk("USB PHY1 is disabled!\n");
		}
	}
	else if (IS_RLE0315 || IS_6166) {
		printk("disable USB phy 0\n");
		REG32(BSP_IP_SEL) &= ~BSP_EN_USB_PHY0;
		
		ehci_phy_write(1, 0xE0, 0xB8);
		ehci_phy_write(1, 0xE1, 0xA8);
		ehci_phy_write(1, 0xE2, 0x99);
		ehci_phy_write(1, 0xE3, 0x41);
		ehci_phy_write(1, 0xE5, 0x91);
		ehci_phy_write(1, 0xE6, 0x88);
		ehci_phy_write(1, 0xF4, 0xE3);
	}
	else {
		ehci_phy_write(1, 0xE0, 0x91);
		ehci_phy_write(1, 0xE1, 0xA8);
		ehci_phy_write(1, 0xE2, 0x81);
		ehci_phy_write(1, 0xE5, 0x95);
		ehci_phy_write(1, 0xE6, 0xE8);
		ehci_phy_write(1, 0xE7, 0x38);
		ehci_phy_write(1, 0xF1, 0x8E);
	}

	return;
}
EXPORT_SYMBOL(set_usbphy);

void usb_enable_IP(void)
{
	if (IS_RTL8676) {
		#define BSP_USB_PHY_CTRL2		(BSP_MISC_CR_BASE + 0x1C)
		#define BSP_USB_HOST_PGBNDRY_DISABLE (1 << 23)
		unsigned int ipSel = REG32(BSP_IP_SEL);
		unsigned int phyCtrl2 = REG32(BSP_USB_PHY_CTRL2);
#ifdef CONFIG_USB_OTG_IP_EN
		ipSel |= BSP_EN_USB_OTG;
#else //CONFIG_USB_OTG_IP_EN
		ipSel &= ~BSP_EN_USB_OTG;
#endif //CONFIG_USB_OTG_IP_EN
		ipSel |= BSP_EN_USB_PHY0 | BSP_EN_USB_PHY1 | BSP_EN_USB_HOST;

		REG32(BSP_IP_SEL) = ipSel;
		REG32(BSP_USB_PHY_CTRL2) = phyCtrl2 | BSP_USB_HOST_PGBNDRY_DISABLE;
		printk("usb_enable_IP: ipsel = 0x%08x, phyCtrl2 = 0x%08x\n", REG32(BSP_IP_SEL), REG32(BSP_USB_PHY_CTRL2));
	}

	return;
}
EXPORT_SYMBOL(usb_enable_IP);

