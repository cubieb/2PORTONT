/*
 * xHCI host controller driver PCI Bus Glue.
 *
 * Copyright (C) 2008 Intel Corp.
 *
 * Author: Sarah Sharp
 * Some code borrowed from the Linux EHCI driver.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include "bspchip.h"

#include "xhci.h"

/* Device for a quirk */
#define PCI_VENDOR_ID_FRESCO_LOGIC	    0x1b73
#define PCI_DEVICE_ID_FRESCO_LOGIC_PDK	0x1000

extern void set_usb2phy_xhci(void);
static const char hcd_name[] = "xhci_hcd";



static void get_mdio_phy(unsigned int addr,unsigned int *value);
static void set_mdio_phy(unsigned int addr,unsigned int value);
static u32 read_reg(unsigned char *addr);
static void write_reg(unsigned char *addr,u32 value);
static void U3_PhyReset(int Reset_n);

static int u3_reg_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	return 0;
}
static int u3_reg_write(struct file *file, const char *buff, unsigned long len, void *data)
{
	char 		tmpbuf[64];
	unsigned int	mem_addr, mem_data, mem_len;
	char		*strptr, *cmd_addr;
	char		*tokptr;

	if (buff && !copy_from_user(tmpbuf, buff, len)) {
		tmpbuf[len] = '\0';
		strptr=tmpbuf;
		cmd_addr = strsep(&strptr," ");
		if (cmd_addr==NULL)
		{
			goto errout;
		}

		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}

		if (!memcmp(cmd_addr, "r", 1))
		{
			mem_addr=simple_strtol(tokptr, NULL, 0);


            if(mem_addr==0)
            {
                printk("read reg :0x%x \n", mem_addr);
                goto errout;
            }
            mem_len=read_reg((unsigned char *)mem_addr);

            
		}
		else if (!memcmp(cmd_addr, "w", 1))
		{
			mem_addr=simple_strtol(tokptr, NULL, 0);
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			mem_data=simple_strtol(tokptr, NULL, 0);

            write_reg((unsigned char *)mem_addr, mem_data);
            mem_len=read_reg((unsigned char *)mem_addr);

		}
		else
		{
			goto errout;
		}
	}
	else
	{
errout:
		printk("Memory operation only support \"r\" and \"w\" as the first parameter\n");
		printk("Read format:	\"r mem_addr length\"\n");
		printk("Write format:	\"w mem_addr mem_data\"\n");
	}

	return len;
}


static int u3_mdio_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	return 0;
}
static int u3_mdio_write(struct file *file, const char *buff, unsigned long len, void *data)
{
	char 		tmpbuf[64];
	unsigned int	mem_addr, mem_data;
	char		*strptr, *cmd_addr;
	char		*tokptr;

	if (buff && !copy_from_user(tmpbuf, buff, len)) {
		tmpbuf[len] = '\0';
		strptr=tmpbuf;
		cmd_addr = strsep(&strptr," ");
		if (cmd_addr==NULL)
		{
			goto errout;
		}
		////printk("cmd %s\n", cmd_addr);
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			////goto errout;
            int i;
            for(i=0;i<0x30;i++)
			{
                get_mdio_phy(i,&mem_data);
			    printk("read mdio addr:0x%x val:0x%x\n",i, mem_data);
			}
	        return len;
		}

		if (!memcmp(cmd_addr, "r", 1))
		{
			mem_addr=simple_strtol(tokptr, NULL, 0);

            get_mdio_phy(mem_addr,&mem_data);
			printk("read mdio addr:0x%x val:0x%x\n", mem_addr, mem_data);
			////memDump(mem_addr, mem_len, "");
		}
		else if (!memcmp(cmd_addr, "w", 1))
		{
			mem_addr=simple_strtol(tokptr, NULL, 0);
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			mem_data=simple_strtol(tokptr, NULL, 0);      
			////WRITE_MEM32(mem_addr, mem_data);
            set_mdio_phy(mem_addr,mem_data);
			printk("Write mdio addr:0x%x val:0x%x\n", mem_addr, mem_data);         
		}
		else
		{
			goto errout;
		}
	}
	else
	{
errout:
		printk("mido operation only support \"r\" and \"w\" as the first parameter\n");
		printk("Read format:	\"r addr\"\n");
		printk("Write format:	\"w addr val\"\n");
	}

	return len;
}

#define U3DBG_PROC_DIR_NAME "U3DBG"
static struct proc_dir_entry *u3_dbg_proc_dir_t=NULL;
static struct proc_dir_entry *u3_mido, *u3_reg;

static void rtl8686_proc_debug_init(void){
	if(u3_dbg_proc_dir_t==NULL)
		u3_dbg_proc_dir_t = proc_mkdir(U3DBG_PROC_DIR_NAME,NULL);
	if(u3_dbg_proc_dir_t)
	{
		u3_mido = create_proc_entry("mdio",0,u3_dbg_proc_dir_t);
		if(u3_mido != NULL)
		{
			u3_mido->read_proc = u3_mdio_read;
			u3_mido->write_proc= u3_mdio_write;			
		}
		else
		{
			printk("can't create proc entry for u3_mido\n");
		}

		u3_reg = create_proc_entry("reg",0,u3_dbg_proc_dir_t);
		if(u3_reg != NULL)
		{
			u3_reg->read_proc = u3_reg_read;
			u3_reg->write_proc= u3_reg_write;			
		}
		else
		{
			printk("can't create proc entry for u3_reg\n");
		}
	}
}


#if 0
/* called after powerup, by probe or system-pm "wakeup" */
static int xhci_pci_reinit(struct xhci_hcd *xhci, struct pci_dev *pdev)
{
	/*
	 * TODO: Implement finding debug ports later.
	 * TODO: see if there are any quirks that need to be added to handle
	 * new extended capabilities.
	 */

	/* PCI Memory-Write-Invalidate cycle support is optional (uncommon) */
	if (!pci_set_mwi(pdev))
		xhci_dbg(xhci, "MWI active\n");

	xhci_dbg(xhci, "Finished xhci_pci_reinit\n");
	return 0;
}
#endif

/* called during probe() after chip reset completes */
#if 0
static int xhci_pci_setup(struct usb_hcd *hcd)
#else
static int xhci_setup(struct usb_hcd *hcd)
#endif
{
	struct xhci_hcd		*xhci = hcd_to_xhci(hcd);
//	struct pci_dev		*pdev = to_pci_dev(hcd->self.controller);
	int			retval;

	hcd->self.sg_tablesize = TRBS_PER_SEGMENT - 1;

	xhci->cap_regs = hcd->regs;
	xhci->op_regs  = hcd->regs +
		HC_LENGTH(xhci_readl(xhci, &xhci->cap_regs->hc_capbase));
	xhci->run_regs = hcd->regs +
		(xhci_readl(xhci, &xhci->cap_regs->run_regs_off) & RTSOFF_MASK);
	/* Cache read-only capability registers */
	xhci->hcs_params1 = xhci_readl(xhci, &xhci->cap_regs->hcs_params1);
#if 0	
	xhci->hcs_params2 = xhci_readl(xhci, &xhci->cap_regs->hcs_params2);
#else //tysu: force disable sc
	xhci->hcs_params2 = 0;
#endif
	xhci->hcs_params3 = xhci_readl(xhci, &xhci->cap_regs->hcs_params3);
	xhci->hcc_params = xhci_readl(xhci, &xhci->cap_regs->hc_capbase);
	xhci->hci_version = HC_VERSION(xhci->hcc_params);
	xhci->hcc_params = xhci_readl(xhci, &xhci->cap_regs->hcc_params);
	xhci_print_registers(xhci);

#if 0
	/* Look for vendor-specific quirks */
	if (pdev->vendor == PCI_VENDOR_ID_FRESCO_LOGIC &&
			pdev->device == PCI_DEVICE_ID_FRESCO_LOGIC_PDK &&
			pdev->revision == 0x0) {
			xhci->quirks |= XHCI_RESET_EP_QUIRK;
			xhci_dbg(xhci, "QUIRK: Fresco Logic xHC needs configure"
					" endpoint cmd after reset endpoint\n");
	}
#endif	

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

#if 0
	pci_read_config_byte(pdev, XHCI_SBRN_OFFSET, &xhci->sbrn);
#else
	xhci->sbrn=0x30;	//version 3.0
#endif
	xhci_dbg(xhci, "Got SBRN %u\n", (unsigned int) xhci->sbrn);

#if 0
	/* Find any debug ports */
	return xhci_pci_reinit(xhci, pdev);
#else
	return 0;
#endif
}

static const struct hc_driver xhci_driver_rtl8672 = {
	.description =		hcd_name,
	.product_desc =		"xHCI Host Controller",
	.hcd_priv_size =	sizeof(struct xhci_hcd),

	/*
	 * generic hardware linkage
	 */
	.irq =			xhci_irq,
	.flags =		HCD_MEMORY | HCD_USB3,

	/*
	 * basic lifecycle operations
	 */
	.reset =		xhci_setup,
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
	.update_hub_device =	xhci_update_hub_device,
	.reset_device =         xhci_reset_device,

	/*
	 * scheduling support
	 */
	.get_frame_number =	xhci_get_frame,

	/* Root hub support */
	.hub_control =		xhci_hub_control,
	.hub_status_data =	xhci_hub_status_data,
};


#define USB2_PHY_DELAY { mdelay(10); }

//#define REG32(reg)   (*(volatile unsigned int *)((unsigned int)reg))

#if 0
static int read_mdio_phy(unsigned int addr)
{
	int readback;
	readback=REG32(0xb8140000);
	USB2_PHY_DELAY;
	REG32(0xb8140000)=(addr<<8);
	USB2_PHY_DELAY;
	readback=REG32(0xb8140000);
	USB2_PHY_DELAY;
	return (readback >> 16)&0xffff;
}
#endif

static void set_mdio_phy(unsigned int addr,unsigned int value)
{
	int readback;

	REG32(0xb8140000)=(addr<<8);
	USB2_PHY_DELAY;
	readback=REG32(0xb8140000);
	USB2_PHY_DELAY;
	REG32(0xb8140000)=(value<<16)|(addr<<8)|1;
	USB2_PHY_DELAY;

	readback=REG32(0xb8140000);
	USB2_PHY_DELAY;
	REG32(0xb8140000)=(addr<<8);
	USB2_PHY_DELAY;
	readback=REG32(0xb8140000);
	USB2_PHY_DELAY;
	
	if(readback!=((value<<16)|(addr<<8)|0x10)) { printk("usb phy set error (addr=%x, value=%x read=%x)\n",addr,value,readback); }
}


static void get_mdio_phy(unsigned int addr,unsigned int *value)
{
	int readback;
	int readback2;
	REG32(0xb8140000)=(addr<<8);
	USB2_PHY_DELAY;
	readback=REG32(0xb8140000);
	USB2_PHY_DELAY;
	////readback=REG32(0xb8140000);
	USB2_PHY_DELAY;
	REG32(0xb8140000)=(addr<<8);
	USB2_PHY_DELAY;
	readback2=REG32(0xb8140000);
	USB2_PHY_DELAY;
    *value=((readback2>>16)&0xFFFF);
}

//-------------------------------------------------------
static void U3_PhyReset(int Reset_n)  //wei add
{
	#define U3_IPCFG 0xb8140008
	//U3_PhyReset
	REG32(U3_IPCFG) |=   (1<<9);  //control by sys reg.

	if(Reset_n==0)     REG32(U3_IPCFG) &=~(1<<10);
	else				    REG32(U3_IPCFG) |=   (1<<10);	
  
	mdelay(100);
	
}


#define NEW_PHY_PARAM 1
////#define TINA_20110901 1
static void set_usbphy_rle0371(void) //just for rle0371
{

#if 0  // mark old U3 phy parameters ,  2013/10/31 
    //enable USB3 MAC/PHY power
    REG32(0xb8000600)|=0xc000201c; //0xc000ffff
    //REG32(0xb8000600)|=0xc000ffff; //0xc000ffff

        //mdc/mdio setting

#if 0
//new parameter from RDC +
    set_mdio_phy(0x00, 0x1278);
    set_mdio_phy(0x01, 0x0003);
    set_mdio_phy(0x02, 0x2D18); //RX CCO= LC-VCO
    set_mdio_phy(0x03, 0x6D70);
    set_mdio_phy(0x04, 0x7000);
    set_mdio_phy(0x05, 0x0304);
    set_mdio_phy(0x06, 0xB054);
    set_mdio_phy(0x07, 0x4CC1);
    set_mdio_phy(0x08, 0x31D2);
    set_mdio_phy(0x09, 0x923C);
    //enable debug port, LFPS: DPHY period is most like U55
    set_mdio_phy(0x0a, 0x9240);
    set_mdio_phy(0x0b, 0xC51D);
    set_mdio_phy(0x0c, 0x68AB);
    set_mdio_phy(0x0d, 0x27A6);
    set_mdio_phy(0x0e, 0x9B01);
    set_mdio_phy(0x0f, 0x051A);
    set_mdio_phy(0x10, 0x000C);

    set_mdio_phy(0x11, 0x4C00);
    set_mdio_phy(0x12, 0xFC00);
    set_mdio_phy(0x13, 0x0C81);
    set_mdio_phy(0x14, 0xDE01);
    set_mdio_phy(0x15, 0x0000);
    set_mdio_phy(0x16, 0x0000);
    set_mdio_phy(0x17, 0x0000);
    set_mdio_phy(0x18, 0x0000);
    set_mdio_phy(0x19, 0xA000);
    set_mdio_phy(0x1a, 0x6DE1);
    set_mdio_phy(0x1b, 0xA027);
    set_mdio_phy(0x1c, 0xC300);
    //set_mdio_phy(0x1c, 0xC81C); //RX negtive edge & slew rate slow $ 4mA
    set_mdio_phy(0x1d, 0xA03E);
    set_mdio_phy(0x1e, 0xC2A0);
    
    set_mdio_phy(0x1f, 0x0000);
    
    set_mdio_phy(0x20, 0xE39F);
    
    set_mdio_phy(0x21, 0xD51B);
    set_mdio_phy(0x22, 0x0836);
    set_mdio_phy(0x23, 0x4FA2);
    set_mdio_phy(0x24, 0x13F1);
    set_mdio_phy(0x25, 0x03DD);
    set_mdio_phy(0x26, 0x64CA);
    set_mdio_phy(0x27, 0x00F9);
    set_mdio_phy(0x28, 0x48B0);
    set_mdio_phy(0x29, 0x0000);////???
    set_mdio_phy(0x2a, 0x3080);
    set_mdio_phy(0x2b, 0x2018);
    set_mdio_phy(0x2c, 0x0000);////???
    set_mdio_phy(0x2d, 0x0000);////???
    set_mdio_phy(0x2e, 0x0000);////???
    set_mdio_phy(0x2f, 0x0000);////???

    set_mdio_phy(0x04, 0x7000);
 

    set_mdio_phy(0x1c, 0x0000);////???
    set_mdio_phy(0x0a, 0x0000);////???
    set_mdio_phy(0x1c, 0x0000);////???
    set_mdio_phy(0x0a, 0x0000);////???

    set_mdio_phy(0x0a, 0x9A40);
    set_mdio_phy(0x0a, 0x9A44);
    set_mdio_phy(0x0a, 0x9240);
    set_mdio_phy(0x09, 0x923C);
    set_mdio_phy(0x09, 0x903C);
    set_mdio_phy(0x09, 0x923C);
////new parameter from RDC -

#else
 
#ifdef NEW_PHY_PARAM
        set_mdio_phy(0x00, 0x1218);
#else
        set_mdio_phy(0x00, 0x1278);
#endif
        set_mdio_phy(0x01, 0x0003);
        set_mdio_phy(0x02, 0x2D18);
        set_mdio_phy(0x03, 0x6D66); //org
//      set_mdio_phy(0x03, 0x6D64); //test
        set_mdio_phy(0x04, 0x5000);
        set_mdio_phy(0x05, 0x0304);
        set_mdio_phy(0x06, 0x6054);
        set_mdio_phy(0x07, 0x4CC1);
        set_mdio_phy(0x08, 0x31D2); //org
//      set_mdio_phy(0x08, 0x31D6); //debug test
        set_mdio_phy(0x09, 0x921C);
        set_mdio_phy(0x0a, 0x5280);
        set_mdio_phy(0x0b, 0xC51D);
        set_mdio_phy(0x0c, 0xA8AB);
        set_mdio_phy(0x0d, 0x27A6);
        set_mdio_phy(0x0e, 0x94C5);
        set_mdio_phy(0x0f, 0x051A);
        set_mdio_phy(0x10, 0x000C);
        set_mdio_phy(0x11, 0x4C00);
        set_mdio_phy(0x12, 0xFC00);
        set_mdio_phy(0x13, 0x0C81);
        set_mdio_phy(0x14, 0xDE01);
        set_mdio_phy(0x15, 0x0000);
        set_mdio_phy(0x16, 0x0000);
        set_mdio_phy(0x17, 0x0000);
        set_mdio_phy(0x18, 0x0000);
        set_mdio_phy(0x19, 0xa000);
        set_mdio_phy(0x1a, 0x6DE1);
        set_mdio_phy(0x1b, 0x3F00);
        set_mdio_phy(0x1c, 0xC3C0);
        set_mdio_phy(0x1d, 0xA03E);
        set_mdio_phy(0x1e, 0xC2A0);
        set_mdio_phy(0x1f, 0x0000);
        set_mdio_phy(0x20, 0xE39F);
        set_mdio_phy(0x21, 0xD51B);
        set_mdio_phy(0x22, 0x0836);
        set_mdio_phy(0x23, 0x4FA2);
        set_mdio_phy(0x24, 0x13F1);
        set_mdio_phy(0x25, 0x03DD);
        set_mdio_phy(0x26, 0x65BA);
        set_mdio_phy(0x27, 0x00F9);
        set_mdio_phy(0x28, 0x48B0);
//      set_mdio_phy(0x29, 0x0);
        set_mdio_phy(0x2a, 0x3000);
        set_mdio_phy(0x2b, 0x0000);
//      set_mdio_phy(0x2c, 0x0000);
//      set_mdio_phy(0x2d, 0x0000);
//      set_mdio_phy(0x2e, 0x0000);
        set_mdio_phy(0x2f, 0x0000);
        set_mdio_phy(0x04, 0x7000);
 
#ifdef NEW_PHY_PARAM
        set_mdio_phy(0x1c, 0xc301);
        set_mdio_phy(0x0a, 0x1ae0);
        set_mdio_phy(0x1c, 0xc300);
        set_mdio_phy(0x0a, 0x1a80);
#endif

        set_mdio_phy(0x0a, 0x5A80);
        set_mdio_phy(0x0a, 0x5A84);
        set_mdio_phy(0x0a, 0x5280);
        set_mdio_phy(0x0a, 0x5280);
        set_mdio_phy(0x0a, 0x5280);
        set_mdio_phy(0x09, 0x921C);
        set_mdio_phy(0x09, 0x901C);
        set_mdio_phy(0x09, 0x921C);

 #endif
#else
//=== Start of 2013/10/31, Got new U3 phy parameter for RLE0371 from RD Center  ===== 

		
#if 0 
		//Sugguestion from PHY designer, reset the mac , until phy parameters are written.
		tmp = read_reg(0xb8000600);
		printk("[%s:%d] read_reg(0xb8000600)=0x%x, tmp=0x%x\n",__func__,__LINE__, read_reg(0xb8000600),tmp);
		tmp|=0x8000201C; 
		tmp &= ~(1<<30) ;	
		write_reg(0xb8000600, tmp);  //Disable bit 30 , until set phy paramteters , 0xb800_0600 bit[30] POW_USB3
#else
		REG32(0xb8000600)|=0xc000201c; //0xc000ffff
#endif
		printk("[%s] read_reg(0xb8000600)=0x%x\n",__func__, read_reg(0xb8000600));
		printk("Start of setting U3 Phy parameters\n");
		set_mdio_phy(0x00,0x1218); //0x1278->0x1218, Recommand by RD-Center, 2013/11/22
		set_mdio_phy(0x01,0x0003);
		set_mdio_phy(0x02,0x2D18);
		set_mdio_phy(0x03,0x6D70);
		set_mdio_phy(0x04,0x5000);  //0x04, should disable first
		set_mdio_phy(0x05,0x0304);
		set_mdio_phy(0x06,0xB054);
		set_mdio_phy(0x07,0x4CC1);
		set_mdio_phy(0x08,0x31D2);
		set_mdio_phy(0x09,0x923C);
		set_mdio_phy(0x0A,0x9240);
		set_mdio_phy(0x0B,0xC51D);
		set_mdio_phy(0x0C,0x68AB);
		set_mdio_phy(0x0D,0x27A6);
		set_mdio_phy(0x0E,0x9B01);
		set_mdio_phy(0x0F,0x051A);
		set_mdio_phy(0x10,0x000C);
		set_mdio_phy(0x11,0x4C00);
		set_mdio_phy(0x12,0xFC00);
		set_mdio_phy(0x13,0x0C81);
		set_mdio_phy(0x14,0xDE01);
		
		set_mdio_phy(0x19,0xA000);
		set_mdio_phy(0x1A,0x6DE1);
		set_mdio_phy(0x1B,0xA027);
		set_mdio_phy(0x1C,0xC300);
		set_mdio_phy(0x1D,0xA03E);
		set_mdio_phy(0x1E,0xC2A0);
		
		set_mdio_phy(0x20,0xE39F);
		set_mdio_phy(0x21,0xD51B);
		set_mdio_phy(0x22,0x0836);
		set_mdio_phy(0x23,0x4FA2);
		set_mdio_phy(0x24,0x13F1);
		set_mdio_phy(0x25,0x03DD);
		set_mdio_phy(0x26,0x64CA);
		set_mdio_phy(0x27,0x00F9);
		set_mdio_phy(0x28,0xF8B0); //0x48b0->0xf8b0 , Recommand by RD-Center, 2013/11/22
		set_mdio_phy(0x2A,0x3080);
		set_mdio_phy(0x2B,0x2018);

		set_mdio_phy(0x04,0x7000);  //0x04, then enable 
		
		set_mdio_phy(0x0A,0x9A40);
		set_mdio_phy(0x0A,0x9A44);
		set_mdio_phy(0x0A,0x9240);
		set_mdio_phy(0x09,0x923C);
		set_mdio_phy(0x09,0x903C);
		set_mdio_phy(0x09,0x923C);

		printk("End of setting U3 Phy parameters\n");
		REG32(0xb8000600)|=0xC000201C; //Enable bit 30 , after set phy paramteters
//=== End of 2013/10/31, Got new U3 phy parameter for RLE0371 from RD Center  ===== 
#endif

//	REG32(0xb8000600)=0x20a7ffff;
//	REG32(0xb8000600)=0x2007ffff; //bit 20~23 -> debug signal
//	REG32(0xb8000600)=0x2017ffff; //bit 20~23 -> debug signal
	printk("######### 0xb8000600=%x ###########\n",REG32(0xb8000600));  //debug port change

	//REG32(0xb8140008)= 0x0040006d;  //90
	REG32(0xb8140008)= 0x0080006d;  //180
	//REG32(0xb8140008)= 0x00c0006d;  //270
	mdelay(100);

//	write_reg(0xb804c2c0,read_reg(0xb804c2c0)|0x40000);  //for U2 debug 2011/3/15
//	mdelay(100);
//debug
//	REG32(0xb804c2c0)|= 0x80;
//	mdelay(100);	
//	REG32(0xb804c2c0)&= 0xffffff7f;
//	mdelay(100);		

	U3_PhyReset(0);
	U3_PhyReset(1);

	//Now config U2 Phy
	printk("Now config U2 PHY in XHCI\n");
	set_usb2phy_xhci();
}

//#define x86

#ifdef x86
#define INT_BASE 0x620
#define DB_BASE 0x800
#define V_TO_P 0x3f
#else
#define INT_BASE 0x460
#define DB_BASE 0x480
#define V_TO_P 0xf
#endif

static u32 read_reg(unsigned char *addr)
{
	volatile u32 tmp;
	u32 addv=(u32)addr;
	tmp=REG32(addv);
	printk("read_reg(0x%x)=0x%x\n",(u32)addr,cpu_to_le32(tmp));
	return cpu_to_le32(tmp);
}

static void write_reg(unsigned char *addr,u32 value)
{
	u32 addv=(u32)addr;
	REG32(addv)=cpu_to_le32(value);
	printk("write_reg(0x%x)=0x%x\n",(u32)addr,value);
}

static int xhci_usb_hcd_probe_rtl8672 (struct hc_driver *driver, struct platform_device *pdev)
{
	struct usb_hcd	*hcd;
	struct resource	 *res;
	int			retval; 	
	u32 			rtl8672_usb_irq;
    rtl8686_proc_debug_init();

#if 1 //must enable in driver and test script
    {
    	u8 *base=(u8 *)0xb8040000;
//    	u32 csz=0; // HCCPARAMS:Context Size  1:32bits,2:64bits


    	set_usbphy_rle0371();
#if 0
    	while(1)
    	{
    		volatile u32 tmp;


    		// clear run bit
    		//write_reg(base+0x20,0);
    		//printk("%s %d staus=%x cmd=%x\n",__FUNCTION__,__LINE__,read_reg(base+0x24),read_reg(base+0x20));
    		//mdelay(200);


    		//REG32(0xb8140008)=0x10006d; //mac
    		//mdelay(500);
    		//REG32(0xb8140008)=0x26d; //phy
    		//mdelay(500);
    		//REG32(0xb8140008)=0x6d; // clear
    		//mdelay(500);		

    		// set reset
    		//write_reg(base+0x20,2);
    		//printk("%s %d\n",__FUNCTION__,__LINE__);
    		//mdelay(200);

    		//printk("plug-in device here....\n");
    		//mdelay(10000);	
    		printk("%s %d 0x430=%x\n",__FUNCTION__,__LINE__,read_reg(base+0x430));		
    		printk("%s %d 0x420=%x\n\n",__FUNCTION__,__LINE__,read_reg(base+0x420));

		if(read_reg(base+0x430)==0x2a0)  break;
		if((read_reg(base+0x430)&0x1000)==0x1000) break;	// SS device found!


    		//warm port reset for USB 3.0
		printk("WARM PORT RESET...\n");
    		tmp=read_reg(base+0x430);
    		tmp|=0x80000001;  // warm port reset and set CCS.
    		tmp&=0xfffffffd; // don't disable port
    		write_reg(base+0x430,tmp);		
    		mdelay(500);

		if(((read_reg(base+0x430)&0x1)==1)||((read_reg(base+0x430)&0xfff)==0x2a0))
		{

			printk("SS PortSC=%x\n",read_reg(base+0x430));
			if(read_reg(base+0x430)&(~(PORT_PE))) 
			{
				
				write_reg(base+0x430,read_reg(base+0x430)&(~(PORT_PE)));  //clear Warm port reset change bit.
				printk("clear WARM port reset status bit PortSC=%x\n",read_reg(base+0x430));			
				mdelay(200);
			}

			//try USB 2.0 Port Reset
			//write_reg(base+0x420,read_reg(base+0x420)|0x10);
			//mdelay(200);

			// stop USB 2.0 Port Enable
			//write_reg(base+0x420,read_reg(base+0x420)|0x2);
			//mdelay(200);

			break;
                }



            
    	}   
#endif   
    }
#endif
	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if(res == NULL)	printk("%s: get irq resource failed!\n", __FUNCTION__);
	rtl8672_usb_irq = res->start;
	
	hcd = usb_create_hcd(driver, &pdev->dev, pdev->dev.init_name);
	if(hcd == NULL) {
		retval = -ENOMEM;
		return retval;
	}
	
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if(res == NULL)	printk("%s: get memory resource failed!\n", __FUNCTION__);
	hcd->regs = (void *)(((u32)res->start)|0xb0000000);
	hcd->rsrc_start = ((u64)res->start)|0xb0000000;
	hcd->rsrc_len = res->end - res->start;
	printk("b8040000=%x b800300c=%x %s %d\n",REG32(0xb8040000),REG32(0xb800300c),__func__,__LINE__);

	retval = usb_add_hcd (hcd, rtl8672_usb_irq, IRQF_SHARED);
printk("b8040000=%x b800300c=%x %s %d retval=%d\n",REG32(0xb8040000),REG32(0xb800300c),__func__,__LINE__,retval);	
	return retval;
}

#if 1
static int xhci_rtl8672_drv_probe(struct platform_device *pdev)
{
	return xhci_usb_hcd_probe_rtl8672 ((struct hc_driver *)&xhci_driver_rtl8672, pdev);
}

static int xhci_rtl8672_drv_remove(struct platform_device *pdev)
{
	struct usb_hcd		*hcd;

	hcd = platform_get_drvdata (pdev);
	usb_remove_hcd (hcd);
	if (hcd->driver->flags & HCD_MEMORY) {
		iounmap (hcd->regs);
		release_mem_region (hcd->rsrc_start, hcd->rsrc_len);
	} else {
		release_region (hcd->rsrc_start, hcd->rsrc_len);
	}
	usb_put_hcd (hcd);
	//the_controller = NULL;
	return 0;
}
#endif

#if 1

static struct platform_driver platform_xhci_driver_rtl8672 = {
	.probe = xhci_rtl8672_drv_probe,
	.remove =xhci_rtl8672_drv_remove,
	.shutdown = usb_hcd_platform_shutdown,
	.driver = {
		   .name = "rtl8672-xhci",
		   },
};
#endif

#if 0
static void usb_release(struct device *dev)
{
	/* normally not freed */
}
#endif

#define RTL8672_XHCI_BASE	0xb8040000

#define PADDR(addr)  ((addr) & 0x1FFFFFFF)
static struct platform_device *rtl8672_xhci = NULL; //sd8


static int  xhci_rtl8672_init(void)
{
	struct resource r[2];
	int retval=0;
	if(rtl8672_xhci!=NULL)
	{	
        printk("xhci-rtl8672.c: XHCI device already init\n");
		return -1;
	}

    printk("%s:%d\n", __FUNCTION__,__LINE__);

	memset(&r, 0, sizeof(r));
	r[0].start = PADDR(RTL8672_XHCI_BASE);
	r[0].end   = PADDR(RTL8672_XHCI_BASE)+0x0000EFFF;
	r[0].flags = IORESOURCE_MEM; 	
	r[1].start = r[1].end = BSP_USB_H3_IRQ;
	r[1].flags = IORESOURCE_IRQ;
	rtl8672_xhci = platform_device_register_simple("rtl8672-xhci",0, r, 2);
	rtl8672_xhci->dev.coherent_dma_mask = 0xffffffffUL;
    rtl8672_xhci->dev.dma_mask = &rtl8672_xhci->dev.coherent_dma_mask;

	if (IS_ERR(rtl8672_xhci)) 
	{
		retval = PTR_ERR(rtl8672_xhci);
		rtl8672_xhci=NULL;  //wei add
        printk("xhci-rtl8672.c: XHCI fail ==>retval:%d\n",retval);
		return -1;
	}

    printk("%s:%d  xhci_rtl8672_init SUCCESS!!!\n", __FUNCTION__,__LINE__);
    return retval;
}


static void  xhci_rtl8672_cleanup(void)
{
	if(rtl8672_xhci==NULL)
	{
		printk("XHCI already cleanup\n");
		return;
	}
	
	platform_device_unregister(	rtl8672_xhci);
	rtl8672_xhci=NULL;
	
}


#if 0
/*-------------------------------------------------------------------------*/

/* PCI driver selection metadata; PCI hotplugging uses this */
static const struct pci_device_id pci_ids[] = { {
	/* handle any USB 3.0 xHCI controller */
	PCI_DEVICE_CLASS(PCI_CLASS_SERIAL_USB_XHCI, ~0),
	.driver_data =	(unsigned long) &xhci_pci_hc_driver,
	},
	{ /* end: all zeroes */ }
};
MODULE_DEVICE_TABLE(pci, pci_ids);

/* pci driver glue; this is a "new style" PCI driver module */
static struct pci_driver xhci_pci_driver = {
	.name =		(char *) hcd_name,
	.id_table =	pci_ids,

	.probe =	usb_hcd_pci_probe,
	.remove =	usb_hcd_pci_remove,
	/* suspend and resume implemented later */

	.shutdown = 	usb_hcd_pci_shutdown,
};

int xhci_register_pci()
{
	return pci_register_driver(&xhci_pci_driver);
}

void xhci_unregister_pci()
{
	pci_unregister_driver(&xhci_pci_driver);
}

#endif
