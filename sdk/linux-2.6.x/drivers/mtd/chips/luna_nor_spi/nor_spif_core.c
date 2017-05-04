#ifdef __LUNA_KERNEL__
#include <linux/kernel.h>
#include <linux/module.h>
#include "nor_spif_core.h"
#include "register_map.h"
#define puts(...)          printk(__VA_ARGS__)
#define inline_memcpy(...) memcpy(__VA_ARGS__)
#else
#include <nor_spif_core.h>
#include <register_map.h>
REG_INIT_FUNC(nor_spif_detect, 20);
#endif

#define alloca(x) __builtin_alloca(x)

static SFCSR_T sfcsr_rdy_n_idle_mask SECTION_NOR_SPIF_GEN2_COREDATA = {
	.f.spi_rdy  = 1,
	.f.spi_idle = 1,
};

static SFCSR_T sfcsr_deact_mask SECTION_NOR_SPIF_GEN2_COREDATA = {
	.f.spi_csb0_sts = 1,
	.f.spi_csb1_sts = 1,
	.f.spi_rdy      = 1,
	.f.spi_idle     = 1,
};

static SFCSR_T sfcsr_act_mask SECTION_NOR_SPIF_GEN2_COREDATA = {
	.f.spi_csb0_sts = 1,
	.f.spi_csb1_sts = 1,
};

static SFCSR_T sfcsr_wk SECTION_NOR_SPIF_GEN2_COREDATA;

plr_nor_spi_info_g2_t nor_spif_info SECTION_NOR_SPIF_GEN2_COREDATA;
#ifdef __LUNA_KERNEL__
#else
symb_pdefine(nor_spif_info, SP_NOR_SPIF_INFO, &nor_spif_info);
#endif

SECTION_NOR_SPIF_GEN2_CORE
static inline void CHECK_CTRL_READY(void) {
	u32_t err = 0;

	while ((sfcsr_rdy_n_idle_mask.v & SFCSRrv) != sfcsr_rdy_n_idle_mask.v) {
		/* Say each iteration of this loop takes 10 cycles,
		   on a 500MHz CPU, it polls for about 20ms. */
		if ((err++) > (1024*1024)) {
			puts("EE: NOR SPI flash controller does not respond.\n");
			while (1);
		}
	}

	return;
}

#define spif_cs_deactivate() do { \
	sfcsr_wk.f.spi_csb0 = 1; \
	sfcsr_wk.f.spi_csb1 = 1; \
	SFCSRrv = sfcsr_wk.v;    \
	while ((sfcsr_deact_mask.v & SFCSRrv) != sfcsr_deact_mask.v) { \
		; \
	} \
} while(0)

#define spif_cs_activate() do { \
	sfcsr_wk.f.spi_csb0 = 0; \
	sfcsr_wk.f.spi_csb1 = 0; \
	SFCSRrv = sfcsr_wk.v;    \
	while ((sfcsr_act_mask.v & SFCSRrv) != 0) { \
		; \
	} \
} while(0)

SECTION_NOR_SPIF_GEN2_CORE
u32_t spif_trx(const u32_t cs,
               const nsf_write_phase_t *_wp,
               u8_t *payload,
               nsf_trx_attr_t attr,
               nor_spi_wip_block *wip) {
	u32_t i;
	nsf_write_phase_t wp;
	u32_t uncached_payload[] = {
		(((u32_t)payload) | (1 << 29)),
		(((u32_t)payload) | (1 << 29)) + attr.plen_b - 1,
	};

	wp.cmd  = _wp->cmd;
	wp.addr = _wp->addr;

	/* Check payload location. */
	for (i=0; i<2; i++) {
		if (((uncached_payload[i] > (0xbfc00000 - 1)) &&
		     (uncached_payload[i] < (0xbfffffff + 1))) ||
		    ((uncached_payload[i] > (0xb4000000 - 1)) &&
		     (uncached_payload[i] < (0xbf000000)))) {
			puts("EE: payload is on flash.");
			return 1;
		}
	}

	/* Check write phase len */
	if ((attr.wlen_b == 0) ||
	    (attr.wlen_b == 1) ||
	    (attr.wlen_b == 4) ||
	    (attr.wlen_b == 5)) {
		/* these wlen are allowed. */
	} else {
		puts("EE: invalid wlen_b.");
		return 2;
	}

	if (attr.write_en == 1) {
		nsf_write_phase_t wp_wren = {
			.cmd  = 0x06,
			.addr = 0,
		};
		nsf_trx_attr_t attr_wren = {
			.plen_b = 0,
			.write_en = 0,
			.wlen_b = 1,
			.dummy_ck = 0,
			.pdir  = norsf_ptx,
			.wmode = norsf_sio,
			.pmode = norsf_sio,
		};

		spif_trx(cs, &wp_wren, VZERO, attr_wren, VZERO);
	}

	/* Reset phase */

	CHECK_CTRL_READY();
	spif_cs_deactivate();
	spif_cs_activate();
	spif_cs_deactivate();

	sfcsr_wk.v = SFCSRrv;

	/* Write phase: cmd */
	if (attr.wlen_b) {
		sfcsr_wk.f.len      = 0;
		sfcsr_wk.f.io_width = attr.wmode;
		sfcsr_wk.f.spi_csb0 = 0;
		SFCSRrv = sfcsr_wk.v;
		SFDRrv = wp.cmd << 24;
		attr.wlen_b--;
		CHECK_CTRL_READY();
	}

	/* Write phase: addr */
	if (attr.wlen_b) {
		sfcsr_wk.f.len      = attr.wlen_b - 1;
		sfcsr_wk.f.io_width = attr.wmode;
		sfcsr_wk.f.spi_csb0 = 0;
		SFCSRrv = sfcsr_wk.v;
		SFDRrv = wp.addr << ((4 - attr.wlen_b)*8);
		attr.wlen_b = 0;
		CHECK_CTRL_READY();
	}

	/* Payload phase */
	if (payload != VZERO) {
		sfcsr_wk.f.len      = 0;
		sfcsr_wk.f.io_width = attr.pmode;
		sfcsr_wk.f.spi_csb0 = 0;
		SFCSRrv = sfcsr_wk.v;
		while (attr.plen_b) {
			if (attr.pdir == norsf_prx) {
				*(payload++) = SFDRrv >> 24;
			} else {
				SFDRrv = *(payload++) << 24;
			}
			attr.plen_b--;
			CHECK_CTRL_READY();
		}
	}

	spif_cs_deactivate();

	/* Reset phase */
	CHECK_CTRL_READY();
	spif_cs_deactivate();
	spif_cs_activate();
	spif_cs_deactivate();

	if (wip) {
		wip(cs);
	}

	return 0;
}

extern nor_spi_probe_t *LS_start_of_nor_spif_probe_func;
extern nor_spi_probe_t *LS_end_of_nor_spif_probe_func;

SECTION_NOR_SPIF_GEN2_MISC
void nor_spif_detect(void) {
	nor_spi_probe_t **pf = &LS_start_of_nor_spif_probe_func;
	void *flash_info;

	puts("II: NOR SPI-F... ");

	while (pf != &LS_end_of_nor_spif_probe_func) {
		flash_info = (void *)(*pf)();
		if (flash_info != VZERO) {
			inline_memcpy(&nor_spif_info, flash_info, sizeof(plr_nor_spi_info_g2_t));
			break;
		}
		pf++;
	}

	puts("done\n");

	return;
}
