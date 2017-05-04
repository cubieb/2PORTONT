#ifndef _BSP_AUTOMEM_H_
#define _BSP_AUTOMEM_H_

#define ZONE1_SIZE    0x10000000
#define ZONE2_BASE    0x20000000
#define ZONE2_PHY     0x10000000
#define ZONE2_OFF     (0x80000000 + ZONE2_PHY - ZONE2_BASE)
#define ZONE2_MAX     ((256<<20) -1)


#if defined(CONFIG_LUNA_MEMORY_AUTO_DETECTION)
#undef CONFIG_RTL8686_CPU_MEM_BASE
#undef CONFIG_RTL8686_IPC_MEM_BASE
#undef CONFIG_RTL8686_DSP_MEM_BASE

struct bsp_mem_map_s {
unsigned long BSP_IPC_MEM_BASE;
unsigned long BSP_DSP_MEM_BASE;
unsigned long padding[2];
};
extern struct bsp_mem_map_s bsp_mem_map;

#define CONFIG_RTL8686_DSP_MEM_BASE   bsp_mem_map.BSP_DSP_MEM_BASE
#define CONFIG_RTL8686_IPC_MEM_BASE   bsp_mem_map.BSP_IPC_MEM_BASE

#endif
#endif /* _BSP_AUTOMEM_H_ */
#ifndef _BSP_AUTOMEM_H_
#define _BSP_AUTOMEM_H_

#define ZONE1_SIZE    0x10000000
#define ZONE2_BASE    0x20000000
#define ZONE2_PHY     0x10000000
#define ZONE2_OFF     (0x80000000 + ZONE2_PHY - ZONE2_BASE)
#define ZONE2_MAX     ((256<<20) -1)


#if defined(CONFIG_LUNA_MEMORY_AUTO_DETECTION)
#undef CONFIG_RTL8686_CPU_MEM_BASE
#undef CONFIG_RTL8686_IPC_MEM_BASE
#undef CONFIG_RTL8686_DSP_MEM_BASE

struct bsp_mem_map_s {
unsigned long BSP_IPC_MEM_BASE;
unsigned long BSP_DSP_MEM_BASE;
unsigned long padding[2];
};
extern struct bsp_mem_map_s bsp_mem_map;

#define CONFIG_RTL8686_DSP_MEM_BASE   bsp_mem_map.BSP_DSP_MEM_BASE
#define CONFIG_RTL8686_IPC_MEM_BASE   bsp_mem_map.BSP_IPC_MEM_BASE

#endif
#endif /* _BSP_AUTOMEM_H_ */
