#ifndef LUNA_SLAVE_ADDR_H
#define LUNA_SLAVE_ADDR_H

#define luna_addr_v2l(x)  (((unsigned int)x) & 0x1fffffff)
#define luna_addr_l2v(x)  (((unsigned int)x) | 0x80000000)

#define luna_addr_l2p(x)  (((unsigned int)x) + CONFIG_RTL8686_DSP_MEM_BASE)
#define luna_addr_p2l(x)  (((unsigned int)x) - CONFIG_RTL8686_DSP_MEM_BASE)

#define luna_addr_v2p(x)  luna_addr_l2p(luna_addr_v2l(x))
#define luna_addr_p2v(x)  luna_addr_l2v(luna_addr_p2l(x))

#define luna_addr_v2uc(x) (((unsigned int)x) | 0x20000000)
#define luna_addr_l2uc(x) (((unsigned int)x) | 0xa0000000)
#define luna_addr_p2uc(x) luna_addr_l2uc(luna_addr_p2l(x))

#endif
