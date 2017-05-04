/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2006  Ralf Baechle <ralf@linux-mips.org>
 *
 */
#ifndef __ASM_MACH_GENERIC_DMA_COHERENCE_H
#define __ASM_MACH_GENERIC_DMA_COHERENCE_H

struct device;

#if !defined(CONFIG_ARCH_LUNA_SLAVE) && defined(CONFIG_HIGHMEM) \
&& ( defined(CONFIG_LUNA_MEMORY_AUTO_DETECTION) || CONFIG_RTL8686_CPU_MEM_SIZE > 0x10000000 )

/********************************************************************
 * Reference to "Dynamic DMA mapping Guide" of Linux Documentation
 * - The DMA address of Device should be "Bus Address"
 * - When usg HIGHMEM (Cpu0's Mem_size >256MB), we use CPU0 ZONE2_OFFSET to shift it.
 *  #define ZONE2_OFF     (0x80000000 + ZONE2_PHY - ZONE2_BASE)
 *    ( ZONE2_BASE = 0x2000_0000 is constant. It shound not be changed.)
 *    ( ZONE2_PHY  = 0x1000_0000. User can change it to map the 512MB_Addr of CPU_PHY_Addr_Space 
 *      to any address of Physical DRAM.
               CPU                  CPU                  Bus
             Virtual              Physical             Address
             Address              Address               Space
              Space                Space

            +-------+             +------+             +------+
            |       |             |MMIO  |   Offset    |      |
            |       |  Virtual    |Space |   applied   |      |
          C +-------+ --------> B +------+ ----------> +------+ A
            |       |  mapping    |      |   by host   |      |
  +-----+   |       |             |      |   bridge    |      |   +--------+
  |     |   |       |             +------+             |      |   |        |
  | CPU |   |       |             | RAM  |             |      |   | Device |
  |     |   |       |             |      |             |      |   |        |
  +-----+   +-------+             +------+             +------+   +--------+
            |       |  Virtual    |Buffer|   Mapping   |      |
          X +-------+ --------> Y +------+ <---------- +------+ Z
            |       |  mapping    | RAM  |   by IOMMU
            |       |             |      |
            |       |             |      |
            +-------+             +------+
**********************************************************************/
#include <bsp_automem.h>
#ifdef DEBUG
#define DEBUG_CPU2BUS_PTK(fmt, args...)   printk(KERN_DEBUG "DMA_Address for Highmem: " fmt, ## args)
#else
#define DEBUG_CPU2BUS_PTK(fmt, args...)
#endif
static inline dma_addr_t cpu_to_baddr(unsigned long cpu_addr){
        if(cpu_addr > ZONE2_BASE){
	  DEBUG_CPU2BUS_PTK("[%s-%d]cpu_address=0x%08lx, bus_address=0x%08lx\n", __func__, __LINE__, cpu_addr, cpu_addr - ZONE2_PHY);
	  return (cpu_addr - ZONE2_PHY);
	}
	else{
	  return cpu_addr;
	}
}

static inline unsigned long baddr_to_cpu(dma_addr_t dma_addr){
        if(dma_addr > ZONE2_PHY){
	  DEBUG_CPU2BUS_PTK("[%s-%d]dma_address=0x%08x ; cpu_address=0x%08x\n", __func__, __LINE__, dma_addr, dma_addr + ZONE2_PHY);
	  return (dma_addr + ZONE2_PHY);
	}
	else{
	  return dma_addr;
	}
}

static inline dma_addr_t plat_map_dma_mem(struct device *dev, void *addr,
	size_t size)
{
	dma_addr_t pa = cpu_to_baddr(virt_to_phys(addr));

	return pa;
}

static inline dma_addr_t plat_map_dma_mem_page(struct device *dev,
	struct page *page)
{
	dma_addr_t pa = cpu_to_baddr(page_to_phys(page));

	return pa;
}
static inline unsigned long plat_dma_addr_to_phys(dma_addr_t dma_addr)
{
	unsigned long pa = baddr_to_cpu(dma_addr);

	return pa;
}

#else //Not select  ( CONFIG_HIGHMEM and CONFIG_LUNA_MEMORY_AUTO_DETECTION)
static inline dma_addr_t plat_map_dma_mem(struct device *dev, void *addr,
	size_t size)
{
	return virt_to_phys(addr);
}

static inline dma_addr_t plat_map_dma_mem_page(struct device *dev,
	struct page *page)
{
	return page_to_phys(page);
}

static inline unsigned long plat_dma_addr_to_phys(dma_addr_t dma_addr)
{
	return dma_addr;
}
#endif
static inline void plat_unmap_dma_mem(struct device *dev, dma_addr_t dma_addr)
{
}

static inline int plat_dma_supported(struct device *dev, u64 mask)
{
	/*
	 * we fall back to GFP_DMA when the mask isn't all 1s,
	 * so we can't guarantee allocations that must be
	 * within a tighter range than GFP_DMA..
	 */
	if (mask < DMA_BIT_MASK(24))
		return 0;

	return 1;
}

static inline void plat_extra_sync_for_device(struct device *dev)
{
	return;
}

static inline int plat_dma_mapping_error(struct device *dev,
					 dma_addr_t dma_addr)
{
	return 0;
}

static inline int plat_device_is_coherent(struct device *dev)
{
#ifdef CONFIG_DMA_COHERENT
	return 1;
#endif
#ifdef CONFIG_DMA_NONCOHERENT
	return 0;
#endif
}

#endif /* __ASM_MACH_GENERIC_DMA_COHERENCE_H */
