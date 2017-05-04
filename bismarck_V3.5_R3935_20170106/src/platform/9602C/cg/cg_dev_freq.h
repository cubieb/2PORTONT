#ifndef __CG_DEV_FREQ_H__
#define __CG_DEV_FREQ_H__

typedef struct {
    u16_t ocp_mhz;
    u16_t mem_mhz;
    u16_t lx_mhz;
    u16_t spif_mhz;
    #ifdef OTTO_FLASH_NOR
    u16_t nor_pll_mhz;
    #endif
} cg_dev_freq_t;

#define CG_QUERY_FREQUENCY(dev_type, dev_freq_sel) ({ \
    u32_t freq_ret; \
    switch(dev_type){ \
        case CG_DEV_OCP: \
            freq_ret = dev_freq_sel->ocp_mhz; break;\
        case CG_DEV_MEM: \
            freq_ret = dev_freq_sel->mem_mhz; break;\
        case CG_DEV_LX: \
            freq_ret = dev_freq_sel->lx_mhz; break;\
        case CG_DEV_SPIF: \
            freq_ret = dev_freq_sel->spif_mhz; break;\
        default: \
            freq_ret = (u32_t)(-1); \
    } \
    freq_ret; \
})

#endif //__CG_DEV_FREQ_H__

