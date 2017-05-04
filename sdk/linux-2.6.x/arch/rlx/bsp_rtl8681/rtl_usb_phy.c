#include <linux/kernel.h>
#include <asm/delay.h>
#include <bspchip.h>

void ehci_phy_write(unsigned char port, unsigned char reg, unsigned char val)
{
	#define USB2_PHY_DELAY __delay(200) 
	int tmp = REG32(BSP_USB_PHY_CTRL);

	if((port != 0) && (port != 1)) {
		printk("EHCI: Port should be 0 or 1!\n");
		return;
	}
	if((reg < 0xE0) || (reg > 0xF6) || ((reg>0xE7)&&(reg<0xF0))) {
		printk("EHCI: Wrong register address: 0x%02x\n", reg);
		return;
	}

	if(port == 0) {	//port 0
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

unsigned char ehci_phy_read(unsigned char port, unsigned char reg)
{
	#define USB2_PHY_DELAY __delay(200) 
	unsigned char val;

	if((port != 0) && (port != 1)) {
		printk("EHCI: Port should be 0 or 1!\n");
		return 0;
	}
	if((reg < 0xE0) || (reg > 0xF6) || ((reg>0xE7)&&(reg<0xF0))) {
		printk("EHCI: Wrong register address: 0x%02x\n", reg);
		return 0;
	}

	reg = reg - 0x20;
	if(port == 0) {	//port 0
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
	printk("EHCI: port %d, phy(0x%02x) = 0x%02x\n\n", port, reg + 0x20, val);

	return val;
}

void dwc_otg_phy_write(unsigned char reg, unsigned char val)
{
	#define USB2_PHY_DELAY __delay(200) 
	int tmp = REG32(BSP_USB_PHY_CTRL);

	if((reg < 0xE0) || (reg > 0xF6) || ((reg>0xE7)&&(reg<0xF0))) {
		printk("DWC_OTG: Wrong register address: 0x%02x\n", reg);
		return;
	}
	
	tmp = tmp & 0xFF00FF00;
	REG32(BSP_USB_PHY_CTRL) = (val << 16) | tmp; USB2_PHY_DELAY;
	REG32(BSP_DWC_OTG_PHY_CTRL) = ((reg & 0x0F) << 16) | 0x00004002; USB2_PHY_DELAY;
	REG32(BSP_DWC_OTG_PHY_CTRL) = ((reg & 0x0F) << 16) | 0x00004000; USB2_PHY_DELAY;
	REG32(BSP_DWC_OTG_PHY_CTRL) = ((reg & 0x0F) << 16) | 0x00004002; USB2_PHY_DELAY;
	REG32(BSP_DWC_OTG_PHY_CTRL) = ((reg & 0xF0) << 12) | 0x00004002; USB2_PHY_DELAY;
	REG32(BSP_DWC_OTG_PHY_CTRL) = ((reg & 0xF0) << 12) | 0x00004000; USB2_PHY_DELAY;
	REG32(BSP_DWC_OTG_PHY_CTRL) = ((reg & 0xF0) << 12) | 0x00004002; USB2_PHY_DELAY;

	return;
}

unsigned char dwc_otg_phy_read(unsigned char reg)
{
	#define USB2_PHY_DELAY __delay(200) 
	unsigned char val = 0;

	if((reg < 0xE0) || (reg > 0xF6) || ((reg>0xE7)&&(reg<0xF0))) {
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

