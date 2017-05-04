/*-----------------------------------------------------------------------------
	This file is for PCI related functions.
------------------------------------------------------------------------------*/
#ifndef _8190N_PCI_C_
#define _8190N_PCI_C_

#ifdef __KERNEL__
#include <linux/pci.h>
#include "./8190n_cfg.h"
#include "./8190n_headers.h"
#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
#include <asm/mips16_lib.h>
#endif
#if !defined(CONFIG_NET_PCI) && defined(CONFIG_RTL_819X)
#include <bsp/bspchip.h>
#endif

#if (!defined(CONFIG_NET_PCI) && defined(CONFIG_RTL_819X)) || defined(NEW_MAC80211_DRV)
#define MAX_PAYLOAD_SIZE_128B    0x00

#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
__NOMIPS16
#endif
void rtl8196b_pci_reset(unsigned long conf_addr)
{
   /* If PCI needs to be reset, put code here.
    * Note:
    *    Software may need to do hot reset for a period of time, say ~100us.
    *    Here we put 2ms.
    */

	//Modified for PCIE PHY parameter due to RD center suggestion by Jason 12252009
	WRITE_MEM32(0xb8000044, 0x9);//Enable PCIE PLL
	delay_ms(10);
	REG32(0xb8000010)=REG32(0xb8000010)|(0x500); //Active LX & PCIE Clock in 8196B system register
	delay_ms(10);
	WRITE_MEM32(0xb800003C, 0x1);//PORT0 PCIE PHY MDIO Reset
	delay_ms(10);
	WRITE_MEM32(0xb800003C, 0x3);//PORT0 PCIE PHY MDIO Reset
	delay_ms(10);
	WRITE_MEM32(0xb8000040, 0x1);//PORT1 PCIE PHY MDIO Reset
	delay_ms(10);
	WRITE_MEM32(0xb8000040, 0x3);//PORT1 PCIE PHY MDIO Reset
	delay_ms(10);
	WRITE_MEM32(0xb8b01008, 0x1);// PCIE PHY Reset Close:Port 0
	delay_ms(10);
	WRITE_MEM32(0xb8b01008, 0x81);// PCIE PHY Reset On:Port 0
	delay_ms(10);
#ifdef PIN_208
	WRITE_MEM32(0xb8b21008, 0x1);// PCIE PHY Reset Close:Port 1
	delay_ms(10);
	WRITE_MEM32(0xb8b21008, 0x81);// PCIE PHY Reset On:Port 1
	delay_ms(10);
#endif
#ifdef OUT_CYSTALL
	WRITE_MEM32(0xb8b01000, 0xcc011901);// PCIE PHY Reset On:Port 0
	delay_ms(10);
#ifdef PIN_208
	WRITE_MEM32(0xb8b21000, 0xcc011901);// PCIE PHY Reset On:Port 1
	delay_ms(10);
#endif
#endif
	REG32(0xb8000010)=REG32(0xb8000010)|(0x01000000); //PCIE PHY Reset On:Port 0
	delay_ms(10);

   WRITE_MEM32(BSP_PCIE0_H_PWRCR, READ_MEM32(BSP_PCIE0_H_PWRCR) & 0xFFFFFF7F);
#ifdef PIN_208
   WRITE_MEM32(BSP_PCIE1_H_PWRCR, READ_MEM32(BSP_PCIE1_H_PWRCR) & 0xFFFFFF7F);
#endif
   delay_ms(100);
   WRITE_MEM32(BSP_PCIE0_H_PWRCR, READ_MEM32(BSP_PCIE0_H_PWRCR) | 0x00000080);
#ifdef PIN_208
   WRITE_MEM32(BSP_PCIE1_H_PWRCR, READ_MEM32(BSP_PCIE1_H_PWRCR) | 0x00000080);
#endif

   delay_ms(10);

   // Enable PCIE host
	WRITE_MEM32(BSP_PCIE0_H_CFG + 0x04, 0x00100007);
	WRITE_MEM8(BSP_PCIE0_H_CFG + 0x78, (READ_MEM8(conf_addr + 0x78) & (~0xE0)) | MAX_PAYLOAD_SIZE_128B);  // Set MAX_PAYLOAD_SIZE to 128B
}
#endif // !defined(CONFIG_NET_PCI) && defined(CONFIG_RTL_819X)

#ifdef NEW_MAC80211_DRV
extern int MDL_DEVINIT rtl8190_init_one(struct pci_dev *,
                  const struct pci_device_id *);
#else
extern int MDL_DEVINIT rtl8190_init_one(struct pci_dev *,
                  const struct pci_device_id *, struct _device_info_ *, int );
#endif

//#if defined(CONFIG_NET_PCI) || defined(NEW_MAC80211_DRV)
#if defined(NEW_MAC80211_DRV)
//int pci_dev_init(struct pci_dev *pdev, struct mac80211_shared_priv *priv, void __iomem *mem)
int pci_dev_init(struct pci_dev *pdev, struct mac80211_shared_priv *priv, int *mem_len, int *io_len)
#else
int pci_dev_init(struct pci_dev *pdev, struct rtl8190_priv *priv, struct net_device *dev)
#endif
{
	void *regs = NULL;
#if defined(NEW_MAC80211_DRV)
	unsigned long io_addr, mem_addr;
#endif

#ifndef USE_IO_OPS
        u32 pciaddr;
#endif
	u32 pmem_len;
	int rc=0;

#ifdef CONFIG_NET_PCI
#ifndef USE_IO_OPS
    pciaddr=0;
#endif
	pmem_len;
#endif

	rc = pci_enable_device(pdev);
	if (rc)
		goto err_out_free;
#ifndef USE_IO_OPS
	rc = pci_request_regions(pdev, DRV_NAME);
#endif
	if (rc)
		goto err_out_disable;

	if (pdev->irq < 2) {
		rc = -EIO;
#ifdef __LINUX_2_6__
		printk(KERN_ERR "invalid irq (%d) for pci dev\n", pdev->irq);
#else
		printk(KERN_ERR "invalid irq (%d) for pci dev %s\n", pdev->irq, pdev->slot_name);
#endif
		goto err_out_res;
	}

#ifdef NEW_MAC80211_DRV
	io_addr = pci_resource_start(pdev, 0);
	*io_len = pci_resource_len(pdev, 0);
	
	
	mem_addr = pci_resource_start(pdev, 1);
	*mem_len = pci_resource_len(pdev, 1);

	
	if (*mem_len < RTL8190_REGS_SIZE) {
			printk(KERN_ERR "%s (rtl8192se): %x/%x Too short PCI resources\n",
				   pci_name(pdev), (unsigned int)*mem_len, (unsigned int)*io_len);
			rc = -ENOMEM;
			goto err_out_res;
	}

#ifdef CONFIG_X86
	if ((rc = pci_set_dma_mask(pdev, 0xFFFFFF00ULL)) ||
		(rc = pci_set_consistent_dma_mask(pdev, 0xFFFFFF00ULL))) {
		printk(KERN_ERR "%s (rtl8192se): No suitable DMA available\n",
			   pci_name(pdev));
		goto err_out_res;
	}
#endif
#else	// !NEW_MAC80211_DRV
#ifdef USE_IO_OPS
	{
		unsigned long pio_start, pio_len, pio_flags;

		pio_start = (unsigned long)pci_resource_start(pdev, 0);
		pio_len = (unsigned long)pci_resource_len(pdev, 0);
		pio_flags = (unsigned long)pci_resource_flags(pdev, 0);

		if (!(pio_flags & IORESOURCE_IO)) {
			rc = -EIO;
#ifdef __LINUX_2_6__
			printk(KERN_ERR "pci: region #0 not a PIO resource, aborting\n");
#else
			printk(KERN_ERR "%s: region #0 not a PIO resource, aborting\n", pdev->slot_name);
#endif
			goto err_out_res;
		}

		if (!request_region(pio_start, pio_len, DRV_NAME)) {
			rc = -EIO;
			printk(KERN_ERR "request_region failed!\n");
			goto err_out_res;
		}

		if (pio_len < RTL8190_REGS_SIZE) {
			rc = -EIO;
#ifdef __LINUX_2_6__
			printk(KERN_ERR "PIO resource (%lx) too small on pci dev\n", pio_len);
#else
			printk(KERN_ERR "PIO resource (%lx) too small on pci dev %s\n", pio_len, pdev->slot_name);
#endif
			goto err_out_res;
		}

		dev->base_addr = pio_start;
		priv->pshare->ioaddr = pio_start; // device I/O address
	}
#else	// USE_IO_OPS
#ifdef IO_MAPPING
	pciaddr = pci_resource_start(pdev, 0);
#else
	pciaddr = pci_resource_start(pdev, 1);
#endif
	if (!pciaddr) {
		rc = -EIO;
#ifdef __LINUX_2_6__
		printk(KERN_ERR "no MMIO resource for pci dev");
#else
		printk(KERN_ERR "no MMIO resource for pci dev %s\n", pdev->slot_name);
#endif
		goto err_out_res;
	}

	if ((pmem_len = pci_resource_len(pdev, 1)) < RTL8190_REGS_SIZE) {
		rc = -EIO;
#ifdef __LINUX_2_6__
		printk(KERN_ERR "MMIO resource () too small on pci dev\n");
#else
		printk(KERN_ERR "MMIO resource (%lx) too small on pci dev %s\n", (unsigned long)pmem_len, pdev->slot_name);
#endif
		goto err_out_res;
	}

	regs = ioremap_nocache(pciaddr, pmem_len);
	if (!regs) {
		rc = -EIO;
#ifdef __LINUX_2_6__
		printk(KERN_ERR "Cannot map PCI MMIO () on pci dev \n");
#else
		printk(KERN_ERR "Cannot map PCI MMIO (%lx@%lx) on pci dev %s\n", (unsigned long)pmem_len, (long)pciaddr, pdev->slot_name);
#endif
		goto err_out_res;
	}

	dev->base_addr = (unsigned long)regs;
	priv->pshare->ioaddr = (UINT)regs;
#endif // ifndef USE_IO_OPS
#endif	//ifdef NEW_MAC80211_DRV
err_out_free:
err_out_disable:
err_out_res:
	return rc;
}
//#endif	//NEW_MAC80211_DRV


#if defined(CONFIG_NET_PCI) || defined(NEW_MAC80211_DRV)
int MDL_DEVINIT rtl8190_init_pci(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	int ret;
#if defined(NEW_MAC80211_DRV)
	ret = rtl8190_init_one(pdev, ent);
	return ret;
#else
	static int dev_num=0;
#ifdef MBSSID
	int i;
#endif

#ifdef RTL8190_VARIABLE_USED_DMEM
	/* For D-MEM allocation system's initialization : It would before ALL processes */
	rtl8190_dmem_init();
#endif

	if(dev_num >= (sizeof(wlan_device)/sizeof(struct _device_info_))){
		printk("PCI device %d can't be support\n", dev_num);
		return -1;
	}
	else {
		ret = rtl8190_init_one(pdev, ent, &wlan_device[dev_num++], -1);
#ifdef UNIVERSAL_REPEATER
		if (ret == 0) {
			ret = rtl8190_init_one(pdev, ent, &wlan_device[--dev_num], -1);
			dev_num++;
		}
#endif
#ifdef MBSSID
		if (ret == 0) {
			dev_num--;
			for (i=0; i<RTL8190_NUM_VWLAN; i++) {
				ret = rtl8190_init_one(pdev, ent, &wlan_device[dev_num], i);
				if (ret != 0) {
					printk("Init fail!\n");
					return ret;
				}
			}
			dev_num++;
		}
#endif
		return ret;
	}
#endif	//ifdef NEW_MAC80211_DRV
}

#ifdef CONFIG_PM
int rtl8190_suspend(struct pci_dev *pdev, pm_message_t state)
{
	pci_save_state(pdev);
	pci_set_power_state(pdev, pci_choose_state(pdev, state));
	return 0;
}

int rtl8190_resume(struct pci_dev *pdev)
{
	pci_set_power_state(pdev, PCI_D0);
	pci_restore_state(pdev);
	return 0;
}
#endif /* CONFIG_PM */

void MDL_DEVEXIT rtk_remove_one(struct pci_dev *pdev)
{
    struct net_device *dev = pci_get_drvdata(pdev);
    struct rtl8190_priv *priv = dev->priv;

    if (!dev)
        BUG();

    //pci_iounmap((void *)priv->dev->base_addr);
    pci_release_regions(pdev);
    pci_disable_device(pdev);
    pci_set_drvdata(pdev, NULL);
}

#ifdef CONFIG_NET_PCI
MODULE_DEVICE_TABLE(pci, rtl8190_pci_tbl);
#endif

#endif // CONFIG_NET_PCI

#endif //__KERNEL__

#endif //_8190N_PCI_C_
