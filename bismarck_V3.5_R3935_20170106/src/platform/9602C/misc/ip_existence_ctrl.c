#include <util.h>

SECTION_RECYCLE void ctrl_ip_existence(void)
{
    u32_t reg_val = 0;
    reg_val |= ( (1<<1) | (1<<8) | (1<<9) );
    REG32(0xB8000600) = reg_val;
    return;
}

REG_INIT_FUNC(ctrl_ip_existence, 2);

