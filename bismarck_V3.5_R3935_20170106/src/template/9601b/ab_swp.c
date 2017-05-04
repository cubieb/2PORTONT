#include <soc.h>

void mc_dqs_half_clk_en(void) {
	if ((_soc.cid == 0x6422) &&
	    cg_query_freq(CG_DEV_MEM) > 300) {
		RMOD_DIDER(dqs0_en_hclk, 1,
		           dqs1_en_hclk, 1,
		           dqs0_en_tap,  0,
		           dqs1_en_tap,  0);
	};
	return;
}
REG_INIT_FUNC(mc_dqs_half_clk_en, 25);

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
REG_INIT_FUNC(rlx5281_disable_non_blocking_load, 29);
