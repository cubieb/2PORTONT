#include <common.h>
#include <asm/arch/bspchip.h>

phys_size_t initdram(int board_type)
{
	u32 MCR;
	u32 buswid, rows, cols, banks;
	
	MCR = REG32(BSP_MC_MTCR0);
	
	switch ((MCR & 0x30000000) >> 28) {
	case 0: buswid = 1; break;
	case 1: buswid = 2; break;
	case 2: buswid = 4; break;
	default:
		goto DEFAULT;
	}
		
	switch ((MCR & 0x6000000) >> 25) {
	case 0: rows = 1024 * 2; break;
	case 1: rows = 1024 * 4; break;
	case 2: rows = 1024 * 8; break;
	case 3: rows = 1024 * 16; break;	
	}
	
	switch ((MCR & 0x1C00000) >> 22) {
	case 0: cols = 256; break;
	case 1: cols = 512; break;
	case 2: cols = 1024; break;
	case 3: cols = 1024 * 2; break;
	case 4: cols = 1024 * 4; break;	
	default:
		goto DEFAULT;
	}
	
	banks = (MCR & 0x80000) ? 4 : 2;
	
	return buswid * cols * rows * banks;
	
DEFAULT:	
	return 16 * 1024 * 1024;
}

int checkboard (void)
{
	printf("Board: Realtek RTL8676 demo\n");
	
	return 0;
}