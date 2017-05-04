#include <soc.h>

void rlx5281_disable_non_blocking_load(void) {
#define CCTL0_nbl_ON		(1<<28)
#define CCTL0_nbl_OFF		(1<<29)
	volatile u32_t cctl;
	cctl = asm_mfc0(CCTL_REG);
	cctl &= cctl & (~(CCTL0_nbl_ON | CCTL0_nbl_OFF));
	asm_mtc0(cctl, CCTL_REG);
	if (_soc.cid == 0x0639) {
		cctl |= CCTL0_nbl_OFF;
	} else {
		cctl |= CCTL0_nbl_ON;
	}
	asm_mtc0(cctl, CCTL_REG);
	return;
}
//REG_INIT_FUNC(rlx5281_disable_non_blocking_load, 29);