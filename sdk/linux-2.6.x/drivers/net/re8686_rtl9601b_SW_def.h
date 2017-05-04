#ifndef __RE8686_RTL601B_SW_DEF_H__
#define __RE8686_RTL601B_SW_DEF_H__

#include <common/type.h>

typedef struct hal_register_s
{
    uint32 reg_addr;
    uint32 write_bit;
    uint32 reset_val;
}hal_register_t;

extern hal_register_t reg_map_def[];

#define REG_MAP_ENTRY_NUM  105

#endif /* __RE8686_RTL601B_SW_DEF_H__ */
