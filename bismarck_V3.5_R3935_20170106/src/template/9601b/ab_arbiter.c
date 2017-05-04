#include <soc.h>
#include <register_map.h>

static void _haa_config_af_for_lxp(void) {
	u32_t i;

	RMOD_ARB_WRITE_MSK(arb_dec_write_msk, 0);

	for (i=0; i<8; i++) {
		RMOD_ARB_DATA(arb_dec_data, 0xffffffff);
		RMOD_ARB_CTRL(arb_dec_dir, 1,
		              arb_dec_idx, i);
		RMOD_ARB_CTRL(arb_dec_issue, 1);
		while (RFLD_ARB_CTRL(arb_dec_issue));
	}

	return;
}

void arbitor_setup(void) {
	if (_soc.cid == 0x0639) {
		/* Configure Arbitration Func. */
		_haa_config_af_for_lxp();

		/* Reset arbiter. */
		RMOD_HASR(arb_clear, 0);
		RMOD_HASR(arb_clear, 1);

		/* Reset & enable debug counters. */
		ARB_CTRLrv = 0x0;
		RMOD_ARB_CTRL(idle_cnt_rst, 1,
		              lxo_cnt_rst,  1,
		              lxp_cnt_rst,  1,
		              ocp_cnt_rst,  1);
		RMOD_ARB_CTRL(idle_cnt_en, 1,
		              lxo_cnt_en,  1,
		              lxp_cnt_en,  1,
		              ocp_cnt_en,  1);

		/* Enable HAA. */
		RMOD_HASR(arb_new_arb_en, 1);
	} else {
		GIAR0rv  = 0xffffffff;
		GIAR1rv  = 0x00000000;
		LXIAR0rv = 0xffffffff;
		LXIAR1rv = 0x42942942;
		LXIAR2rv = 0xffffffff;
		LXIAR3rv = 0xBD6BD6BD;
		DMCRrv   = 0x00130000;
	}

	return;
}
REG_INIT_FUNC(arbitor_setup, 16);
