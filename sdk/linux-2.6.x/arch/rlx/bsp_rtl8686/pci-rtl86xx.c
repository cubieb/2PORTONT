
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <asm/rlxregs.h>
#include "pci-rtl86xx.h"

#ifdef CONFIG_GENERIC_RTL86XX_PCIE_SLOT0
static struct resource rtl86xx_pcie0_io_resource = {
   .name   = "RTL86xx PCIe0 IO",
   .flags  = IORESOURCE_IO,
   .start  = PADDR(BSP_PCIE0_D_IO),
   .end    = PADDR(BSP_PCIE0_D_IO + 0xFFFF)
};

static struct resource rtl86xx_pcie0_mem_resource = {
   .name   = "RTL86xx PCIe0 MEM",
   .flags  = IORESOURCE_MEM,
   .start  = PADDR(BSP_PCIE0_D_MEM),
   .end    = PADDR(BSP_PCIE0_D_MEM + 0xFFFFFF)
};
#endif

#ifdef CONFIG_GENERIC_RTL86XX_PCIE_SLOT1
static struct resource rtl86xx_pcie1_io_resource = {
   .name   = "RTL86xx PCIe1 IO",
   .flags  = IORESOURCE_IO,
   .start  = PADDR(BSP_PCIE1_D_IO),
   .end    = PADDR(BSP_PCIE1_D_IO + 0xFFFF)
};

static struct resource rtl86xx_pcie1_mem_resource = {
   .name   = "RTL86xx PCIe1 MEM",
   .flags  = IORESOURCE_MEM,
   .start  = PADDR(BSP_PCIE1_D_MEM),
   .end    = PADDR(BSP_PCIE1_D_MEM + 0xFFFFFF)
};
#endif

#ifdef CONFIG_GENERIC_RTL86XX_PCIE_SLOT0
extern struct pci_ops rtl86xx_pcie0_ops;
#endif
#ifdef CONFIG_GENERIC_RTL86XX_PCIE_SLOT1
extern struct pci_ops rtl86xx_pcie1_ops;
#endif
 
 
 struct pcie_para rtl86xx_ePHY[][29] = {
     {  {0, 1, 0x0003},         {0, 2, 0x2d18}, {0, 3, 0x4d09}, {0, 4, 0x5c3f},
        {0, 0, 0x1046},         {0, 6, 0x9048}, {0, 5, 0x2213}, {0, 7, 0x31ff},
        {0, 8, 0x18d7},         {0, 9, 0x539c},         {0, 0xa, 0x00e8},       {0, 0xb, 0x0711},
        {0, 0xc, 0x0828},       {0, 0xd, 0x17a6},       {0, 0xe, 0x98c5},       {0, 0xf, 0x0f0f},
        {0, 0x10, 0x000c},      {0, 0x11, 0x3c00},      {0, 0x12, 0xfc00},      {0, 0x13, 0x0c81},
        {0, 0x14, 0xde01},      {0, 0x19, 0xfce0},      {0, 0x1a, 0x7c00},      {0, 0x1b, 0xfc00},
        {0, 0x1c, 0xfc00},      {0, 0x1d, 0xa0eb},      {0, 0x1e, 0xc280},      {0, 0x1f, 0x0600},
        {0xff,0xff,0xffff}}, //8676 35.328M clk
     {  {0, 1, 0x0003},         {0, 2, 0x2d18}, {0, 3, 0x4d09}, {0, 4, 0x5000},
        {0, 0, 0x1047},         {0, 6, 0x9148}, {0, 5, 0x23cb}, {0, 7, 0x31ff},
        {0, 8, 0x18d7},         {0, 9, 0x539c},         {0, 0xa, 0x00e8},       {0, 0xb, 0x0711},
        {0, 0xc, 0x0828},       {0, 0xd, 0x17a6},       {0, 0xe, 0x98c5},       {0, 0xf, 0x0f0f},
        {0, 0x10, 0x000c},      {0, 0x11, 0x3c00},      {0, 0x12, 0xfc00},      {0, 0x13, 0x0c81},
        {0, 0x14, 0xde01},      {0, 0x19, 0xfce0},      {0, 0x1a, 0x7c00},      {0, 0x1b, 0xfc00},
        {0, 0x1c, 0xfc00},      {0, 0x1d, 0xa0eb},      {0, 0x1e, 0xc280},      {0, 0x1f, 0x0600},
        {0xff,0xff,0xffff}}, //8676 40M clk
     {  {0, 0, 0x1086}, {0, 4, 0x5800}, {0, 5, 0x05d3}, {0, 6, 0xf048},
        {0, 0xb, 0x0711},       {0, 0xd, 0x1766},       {0, 0xf, 0x0a00},       {0, 0x1d, 0xa0eb},
        {0xff,0xff,0xffff}},//8686 40M clk
     {  {0, 6, 0xf848}, {0, 0xb, 0x0711},       {0, 0xd, 0x1766},       {0, 0xf, 0x0a00},
        {0, 0x01d, 0xa0eb},
        {1, 6, 0xf848}, {1, 0xb, 0x0711},       {1, 0xd, 0x1766},       {1, 0xf, 0x0a00},
        {1, 0x01d, 0xa0eb},
        {0xff,0xff,0xffff}},//8686 25M clk
     {  {0, 1, 0x0002}, {0, 2, 0x4300}, {0, 3, 0x0400}, {0, 4, 0x4644},
        {0, 0, 0x0000}, {0, 6, 0x30c0}, {0, 5, 0x8101}, {0, 7, 0x7440},
        {0, 8, 0x901c}, {0, 9, 0x0c9c}, {0, 0xa, 0x4037},       {0, 0xb, 0x2bb0},
        //{0, 0xc, 0x0261},
        {0xff,0xff,0xffff}}, //8676S 35.328M clk
     {  {0, 1, 0x0002}, {0, 2, 0x4300}, {0, 3, 0x0400}, {0, 4, 0x4644},
        {0, 0, 0x0000}, {0, 6, 0x30c0}, {0, 5, 0x8101}, {0, 7, 0x7440},
        {0, 8, 0x901c}, {0, 9, 0x0c9c}, {0, 0xa, 0x4037},       {0, 0xb, 0x2bb0},
        //{0, 0xc, 0x0261},
        {0xff,0xff,0xffff}}, //8676S 40M clk
     {  {0, 1, 0x06a3}, {0, 2, 0x4300}, {0, 3, 0x0400}, {0, 4, 0xd546},
        {0, 0, 0x0000}, {0, 6, 0xb880}, {0, 5, 0x8101}, {0, 7, 0x7c40},
        {0, 8, 0x901c}, {0, 9, 0x0c9c}, {0, 0xa, 0x4037},       {0, 0xb, 0x03b0},
        {0, 0xc, 0x0261},
        {0xff,0xff,0xffff}}, //0562 35.328M clk
     {  {0, 1, 0x06a3}, {0, 2, 0x4300}, {0, 3, 0x0400}, {0, 4, 0xd546},
        {0, 0, 0x0000}, {0, 6, 0xb880}, {0, 5, 0x8101}, {0, 7, 0x7c40},
        {0, 8, 0x901c}, {0, 9, 0x0c9c}, {0, 0xa, 0x4037},       {0, 0xb, 0x03b0},
        {0, 0xc, 0x0261},
        {0xff,0xff,0xffff}}, //0562 40M clk
     {  {0, 0, 0x404c}, {0, 1, 0x16a3}, {0, 2, 0x6340}, {0, 3, 0x370d},
        {0, 4, 0x856a}, {0, 5, 0x8109}, {0, 6, 0x6081}, {0, 7, 0x5400},
        {0, 8, 0x9000}, {0, 9, 0x0ccc}, {0, 0xa, 0x4437},       {0, 0xb, 0x0230},
        {0, 0xc, 0x0021},       {0, 0xd, 0x0000},       {0, 0xe, 0x0000},       {0, 0x1f, 0x0000},
        {0xff,0xff,0xffff}}, //8685 25M clk
     {  {1, 0, 0x404c}, {1, 1, 0x16a3}, {1, 2, 0x6340}, {1, 3, 0x370d},
        {1, 4, 0x856a}, {1, 5, 0x8109}, {1, 6, 0x6081}, {1, 7, 0x5400},
        {1, 8, 0x9000}, {1, 9, 0x0ccc}, {1, 0xa, 0x4437},       {1, 0xb, 0x0230},
        {1, 0xc, 0x0021},       {1, 0xd, 0x0000},       {1, 0xe, 0x0000},       {1, 0x1f, 0x0000},
        {0xff,0xff,0xffff}} //8685 25M clk
};

 

 

void rtl86xx_setmdio(unsigned int port_num , unsigned int reg_addr, unsigned short val)
{

	//HOST PCIE
	#define PCIE0_RC_EXT_BASE (0xb8b01000)
	#define PCIE1_RC_EXT_BASE (0xb8b21000)
	//RC Extended register
	#define PCIE0_MDIO (PCIE0_RC_EXT_BASE+0x00)
	#define PCIE1_MDIO (PCIE1_RC_EXT_BASE+0x00)
	//MDIO
	#define PCIE_MDIO_DATA_OFFSET (16)
	#define PCIE_MDIO_DATA_MASK (0xffff <<PCIE_MDIO_DATA_OFFSET)
	#define PCIE_MDIO_REG_OFFSET (8)
	#define PCIE_MDIO_RDWR_OFFSET (0)
	unsigned int mdioaddr;

	if(port_num == 0)          
		mdioaddr = PCIE0_MDIO;
	else if(port_num == 1)     
		mdioaddr = PCIE1_MDIO;
	else return;

	REG32(mdioaddr)= ( (reg_addr&0x1f)<<PCIE_MDIO_REG_OFFSET) |\
						((val&0xffff)<<PCIE_MDIO_DATA_OFFSET) |\
						(1<<PCIE_MDIO_RDWR_OFFSET) ; 
 
	mdelay(1);
}


int rtl86xx_PCIE_reset_procedure(int portnum, int Use_External_PCIE_CLK, int mdio_reset,unsigned long conf_addr)
{
	    extern void PCIE_reset_pin(int *reset);
#ifdef CONFIG_GENERIC_RTL86XX_PCIE_SLOT1
        extern void PCIE1_reset_pin(int *reset);
#endif

#define PCI_MISC           BSP_PCI_MISC
#define MISC_IP_SEL        BSP_IP_SEL
#define EN_PCIE            BSP_EN_PCIE
#define PCI_MISC           BSP_PCI_MISC
#define MISC_PINSR         BSP_MISC_PINSR
#define CLKSEL             BSP_CLKSEL


        int PCIE_gpio_RST, i, idx;
        unsigned int PCIE_D_CFG0, PCIE_H_CFG, PCIE_H_PWRCR;
        unsigned int ENABLE_PCIE = EN_PCIE;

        if (portnum==0) {
                PCIE_D_CFG0 = BSP_PCIE0_D_CFG0;
                PCIE_H_CFG = BSP_PCIE0_H_CFG;
                PCIE_H_PWRCR = BSP_PCIE0_H_PWRCR;
        } else if(portnum==1) {
                PCIE_D_CFG0 = BSP_PCIE1_D_CFG0;
                PCIE_H_CFG = BSP_PCIE1_H_CFG;
                PCIE_H_PWRCR = BSP_PCIE1_H_PWRCR;
                ENABLE_PCIE = BSP_ENABLE_PCIE1;
        } else {
                printk("Error: portnum=%d\n", portnum);
                return 1;
        }
#ifdef CONFIG_GENERIC_RTL86XX_PCIE_SLOT1
        if (portnum==1)
                PCIE1_reset_pin(&PCIE_gpio_RST);
        else
#endif
                PCIE_reset_pin(&PCIE_gpio_RST);
        // 0. Assert PCIE Device Reset
		printk("%s-%d PCIE_gpio_RST=%d\n",__func__,__LINE__,PCIE_gpio_RST);

        gpioClear(PCIE_gpio_RST);
        gpioConfig(PCIE_gpio_RST, GPIO_FUNC_OUTPUT);
        mdelay(10);

        // 1. PCIE phy mdio reset


        if(IS_8676S_6333 || IS_8676S_0562)
		{
			// donot reset mdio
		}
		else
		{
			if(mdio_reset)
			{
				REG32(PCI_MISC) = BSP_PCI_MDIO_RESET_ASSERT;
				REG32(PCI_MISC) = BSP_PCI_MDIO_RESET_RELEASE;
			}
		}

        // 2. PCIE MAC reset      
        REG32(MISC_IP_SEL) &= ~ENABLE_PCIE;
        REG32(MISC_IP_SEL) |= ENABLE_PCIE;

        if(mdio_reset)
        {
                //printk("Do MDIO_RESET\n");
                // 5.MDIO Reset          
				if(IS_8676S_6333 || IS_8676S_0562)
				{
					// donot reset mdio
				}
				else
					REG32(PCI_MISC) = BSP_PCI_MDIO_RESET_RELEASE;
        }

        // 6. PCIE PHY Reset		  

        REG32(PCIE_H_PWRCR) = 0x1; //bit7 PHY reset=0   bit0 Enable LTSSM=1
        REG32(PCIE_H_PWRCR) = 0x81;   //bit7 PHY reset=1   bit0 Enable LTSSM=1
        mdelay(100);

        //----------------------------------------
        
                if (IS_RTL8676 && !(REG32(MISC_PINSR) & CLKSEL))
                        idx = CLK35_328_8676;
                else if (IS_RTL8676 && (REG32(MISC_PINSR) & CLKSEL))
                        idx = CLK40_8676;
                else if (IS_RTL8686 && (REG32(MISC_PINSR) & CLKSEL)){
                        printk("8686 40Mhz\n");
                        idx = CLK40_8686;
                }
                else if(IS_RTL8686 && !(REG32(MISC_PINSR) & CLKSEL)){
                        printk("8686 25Mhz\n");
                        idx = CLK25_8686;
                }
                else if (IS_8676S_6333 && !(REG32(MISC_PINSR) & CLKSEL))
                        idx = CLK35_328_8676S;
                else if (IS_8676S_6333 && (REG32(MISC_PINSR) & CLKSEL))
                        idx = CLK40_8676S;
                else if (IS_8676S_0562 && !(REG32(MISC_PINSR) & CLKSEL))
                        idx = CLK35_328_0562;
                else if (IS_8676S_0562 && (REG32(MISC_PINSR) & CLKSEL))
                        idx = CLK40_0562;
                else if (IS_RTL8685){
                        if (portnum==0){
                                printk("8685 pcie port 0\n");
                                idx = CLK25_8685_P0;
                        }
                        else{
                                printk("8685 pcie port 1\n");
                                idx = CLK25_8685_P1;
                        }
                }
                else
                        idx = NOT_DEFINED_CLK;

                for (i = 0; NOT_DEFINED_CLK != idx; ) {
                        if(rtl86xx_ePHY[idx][i].port != 0xff){
                            if(portnum == rtl86xx_ePHY[idx][i].port)
								rtl86xx_setmdio(rtl86xx_ePHY[idx][i].port, rtl86xx_ePHY[idx][i].reg, rtl86xx_ePHY[idx][i].value);
							i++;
                        }
                        else
                            break;
                }
        

        // 7. PCIE Device Reset
        gpioSet(PCIE_gpio_RST);

        // wait for LinkUP
        i = 100;
        while(--i)
        {
                if((REG32(PCIE_H_CFG + 0x0728)&0x1f)==0x11)
                        break;
                mdelay(10);
        }
        if (i == 0)
        {
            printk("Warring!! Port %d WLan device PCIE Link Failed, State=0x%x\n", portnum, REG32(PCIE_H_CFG + 0x0728));
            printk("Reset PCIE Host PHY and try again...\n");

            REG32(PCIE_H_PWRCR) = 0x1; //bit7 PHY reset=0   bit0 Enable LTSSM=1
            REG32(PCIE_H_PWRCR) = 0x81;   //bit7 PHY reset=1   bit0 Enable LTSSM=1

            // wait for LinkUP
            i = 100;
            while(--i)
            {
                    if( (REG32(PCIE_H_CFG + 0x0728)&0x1f)==0x11)
                            break;
                    mdelay(100);
            }

			if (i == 0)
			{
				printk("%s[%d]: Error!! Port %d WLan device PCIE Link failed, State=0x%x\n", __FUNCTION__, __LINE__, portnum, REG32(PCIE_H_CFG + 0x0728));
				return 1;
			}
        }
        mdelay(100);

        // 8. Set BAR

        REG32(PCIE_D_CFG0 + 0x10) = 0x18c00001;
        REG32(PCIE_D_CFG0 + 0x18) = 0x19000004;
        REG32(PCIE_D_CFG0 + 0x04) = 0x00180007;
        REG32(PCIE_H_CFG + 0x04) = 0x00100007;

        printk("Find Port_num=%d, Vender_Device_ID=0x%X\n", portnum, REG32(PCIE_D_CFG0 + 0x00) );

        if(i==0)
        {
                printk("Cannot LinkUP (0x%08X)\n", REG32(PCIE_H_CFG + 0x0728));
                return 1;
        }
        // Enable PCIE host

        WRITE_MEM32(PCIE_H_CFG + 0x04, 0x00100007);
        WRITE_MEM8(PCIE_H_CFG + 0x78, (READ_MEM8(conf_addr + 0x78) & (~0xE0)) | 0);  // Set MAX_PAYLOAD_SIZE to 128B
		
        return 0;
}



static int rtl86xx_pci_reset(void)
{
   /* If PCI needs to be reset, put code here.
    * Note: 
    *    Software may need to do hot reset for a period of time, say ~100us.
    *    Here we put 2ms.
    */
#if 1
//Modified for PCIE PHY parameter due to RD center suggestion by Jason 12252009 
WRITE_MEM32(0xb8000044, 0x9);//Enable PCIE PLL
mdelay(10);
//WRITE_MEM32(0xb8000010, 0x00FFFFD6);//Active LX & PCIE Clock in 8196B system register
WRITE_MEM32(0xb8000010, READ_MEM32(0xb8000010)|(1<<8)|(1<<9)|(1<<10));
#ifdef ENABLE_SLOT_1
WRITE_MEM32(0xb8000010, READ_MEM32(0xb8000010)|(1<<12));
#endif
mdelay(10);
WRITE_MEM32(0xb800003C, 0x1);//PORT0 PCIE PHY MDIO Reset
mdelay(10);
WRITE_MEM32(0xb800003C, 0x3);//PORT0 PCIE PHY MDIO Reset
mdelay(10);
#ifdef ENABLE_SLOT_1
WRITE_MEM32(0xb8000040, 0x1);//PORT1 PCIE PHY MDIO Reset
mdelay(10);
WRITE_MEM32(0xb8000040, 0x3);//PORT1 PCIE PHY MDIO Reset
mdelay(10);
#endif
WRITE_MEM32(0xb8b01008, 0x1);// PCIE PHY Reset Close:Port 0
mdelay(10);
WRITE_MEM32(0xb8b01008, 0x81);// PCIE PHY Reset On:Port 0
mdelay(10);
#ifdef ENABLE_SLOT_1
WRITE_MEM32(0xb8b21008, 0x1);// PCIE PHY Reset Close:Port 1
mdelay(10);
WRITE_MEM32(0xb8b21008, 0x81);// PCIE PHY Reset On:Port 1
mdelay(10);
#endif
#ifdef OUT_CYSTALL
WRITE_MEM32(0xb8b01000, 0xcc011901);// PCIE PHY Reset On:Port 0
mdelay(10); 
#ifdef ENABLE_SLOT_1
WRITE_MEM32(0xb8b21000, 0xcc011901);// PCIE PHY Reset On:Port 1
mdelay(10); 
#endif
#endif
//WRITE_MEM32(0xb8000010, 0x01FFFFD6);// PCIE PHY Reset On:Port 1
WRITE_MEM32(0xb8000010, READ_MEM32(0xb8000010)|(1<<24));
mdelay(10);
#endif
   WRITE_MEM32(BSP_PCIE0_H_PWRCR, READ_MEM32(BSP_PCIE0_H_PWRCR) & 0xFFFFFF7F);
#ifdef ENABLE_SLOT_1
   WRITE_MEM32(BSP_PCIE1_H_PWRCR, READ_MEM32(BSP_PCIE1_H_PWRCR) & 0xFFFFFF7F);
#endif
   mdelay(100);
   WRITE_MEM32(BSP_PCIE0_H_PWRCR, READ_MEM32(BSP_PCIE0_H_PWRCR) | 0x00000080);
#ifdef ENABLE_SLOT_1
   WRITE_MEM32(BSP_PCIE1_H_PWRCR, READ_MEM32(BSP_PCIE1_H_PWRCR) | 0x00000080);
#endif
   return 0;
}

#ifdef CONFIG_GENERIC_RTL86XX_PCIE_SLOT0
static struct pci_controller rtl86xx_pcie0_controller = {
   .pci_ops        = &rtl86xx_pcie0_ops,
   .mem_resource   = &rtl86xx_pcie0_mem_resource,
   .io_resource    = &rtl86xx_pcie0_io_resource,
};
#endif 

#ifdef CONFIG_GENERIC_RTL86XX_PCIE_SLOT1
static struct pci_controller rtl86xx_pcie1_controller = {
   .pci_ops        = &rtl86xx_pcie1_ops,
   .mem_resource   = &rtl86xx_pcie1_mem_resource,
   .io_resource    = &rtl86xx_pcie1_io_resource,
};
#endif

int pcibios_map_irq(struct pci_dev *dev, u8 slot, u8 pin)
{
   printk("PCIe Slot: %d\t Pin: %d\tDev->Bus->Number: %d\n", slot,pin,dev->bus->number);

   if (dev->bus->number < 2)
      return BSP_PCIE0_IRQ;
   else
      return BSP_PCIE1_IRQ;
}

/* Do platform specific device initialization at pci_enable_device() time */
int pcibios_plat_dev_init(struct pci_dev *dev)
{

   return 0;
}

static __init int rtl86xx_pcie_init(void)
{
#ifdef CONFIG_GENERIC_RTL86XX_PCIE_SLOT0
        if(rtl86xx_PCIE_reset_procedure(0,0,1,0xb9000000))
        {
                printk("PCIe 0 reset failed\n");
                return 0;
        }
        printk("<<<<<Register 1st PCI Controller>>>>>\n");
        register_pci_controller(&rtl86xx_pcie0_controller);
#endif
#ifdef CONFIG_GENERIC_RTL86XX_PCIE_SLOT1
		#ifndef CONFIG_GENERIC_RTL86XX_PCIE_SLOT0
		if(rtl86xx_PCIE_reset_procedure(1,0,1,0xba000000))
		#else
        if(rtl86xx_PCIE_reset_procedure(1,0,0,0xba000000))
		#endif
        {
                printk("PCIe 1 reset failed\n");
                return 0;
        }
        printk("<<<<<Register 2nd PCI Controller>>>>>\n");
        register_pci_controller(&rtl86xx_pcie1_controller);
#endif
        return 0;
}

arch_initcall(rtl86xx_pcie_init);
