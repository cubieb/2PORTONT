#include <soc.h>

SECTION_RECYCLE
void led_power_on(void) {
	REG32((0xBB000048 + 0x4)) |= (1<<4);
	REG32((0xB8003308 + 0x1c)) |= (1<<4);
	REG32((0xB800330C + 0x1c)) &= ~(1 << 4);
}

REG_INIT_FUNC(led_power_on, 1);

