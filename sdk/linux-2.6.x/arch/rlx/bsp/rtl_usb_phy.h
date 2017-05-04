#ifndef _RTL_USB_PHY_H_
#define _RTL_USB_PHY_H_

union usb3_mdio {
	unsigned int d32;
	struct {
		/* b31:b16, MDIO read data or write data */
		unsigned data:16;

		/* b15:13, MDIO phy page addr */
		unsigned page:3;

		/* b12:8, MDIO register address */
		unsigned reg:5;

		/* b7, reserved */
		unsigned reserved:1;

		/* b6:5, MDIO status for debug checking, 0/others means idle/busy */
#define MDIO_IDLE	0
		unsigned status:2;

		/* b4, MDIO ready for debug checking */
#define MDIO_READY	1
		unsigned ready:1;

		/* b3:2, MDIO clock rate */
#define LX_DIV_32	0
#define LX_DIV_16	1
#define LX_DIV_8	2
#define LX_DIV_4	3
		unsigned rate:2;

		/* b1, MDIO soft reset */
#define RESET_ACTIVE	1
		unsigned reset:1;

		/* b0, MDIO read/write command */
#define MDIO_READ	0
#define MDIO_WRITE	1
		unsigned cmd:1;
	} b;
};


/* Prototypes */
void xhci_usb3_phy_write(unsigned char page, unsigned char reg, unsigned short data);
unsigned short xhci_usb3_phy_read(unsigned char page, unsigned char reg);
void xhci_usb2_phy_write(unsigned char reg, unsigned char val);
unsigned char xhci_usb2_phy_read(unsigned char reg);
void ehci_phy_write(unsigned char phy_port, unsigned char reg, unsigned char val);
unsigned char ehci_phy_read(unsigned char phy_port, unsigned char reg);
void dwc_otg_phy_write(unsigned char reg, unsigned char val);
unsigned char dwc_otg_phy_read(unsigned char reg);
unsigned char usb_mac_identify(unsigned char phy_port);
unsigned char usb_get_phy_port(const char* desc, unsigned char mac_port);
void set_usbphy(void);
void set_usbphy_xhci(void);
void usb_enable_IP(void);
#endif
