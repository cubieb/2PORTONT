#ifdef __LUNA_KERNEL__
#include <linux/module.h>
#include "nor_spif_core.h"
#include "register_map.h"
#define puts(...) printk(__VA_ARGS__)
#else
#include <nor_spif_core.h>
#include <register_map.h>
#endif

#define NORSF_CMN_SZ_MB      (16)
#define NORSF_CMN_ERASE_SZ_B (4096)

SECTION_NOR_SPIF_GEN2_CORE
s32_t flash_unit_erase_g2(const u32_t, const u32_t);
SECTION_NOR_SPIF_GEN2_CORE
s32_t flash_unit_write_g2(const u32_t, u32_t, u32_t, const void *);
SECTION_NOR_SPIF_GEN2_MISC
s32_t flash_read_g2(const u32_t, u32_t, const u32_t, void *);

/* Tested on 0xc84018 (GD). */
plr_nor_spi_info_g2_t nor_spif_common_info = {
	.id             = 0xffffff,
	.num_chips      = 1,
	.wr_boundary_b  = 256,
	.erase_unit_b   = 4096,
	.size_per_chip_b= 16 * 1024 * 1024,
	.addr_len_b     = 3,
	._nor_spi_erase = flash_unit_erase_g2,
	._nor_spi_read  = flash_read_g2,
	._nor_spi_write = flash_unit_write_g2,
};

/* !!!!This function be allocated on SRAM!!!! */
SECTION_NOR_SPIF_GEN2_CORE
static void spif_wip_block(u32_t cs) {
	nsf_write_phase_t wp = {
		.cmd  = 0x05,
		.addr = 0,
	};
	nsf_trx_attr_t attr = {
		.wmode  = norsf_sio,
		.wlen_b = 1,
		.dummy_ck = 0,
		.pdir   = norsf_prx,
		.pmode  = norsf_sio,
		.plen_b = 1,
	};
	u8_t payload = 0x3;

	while ((payload & 0x3) != 0) {
		spif_trx(cs, &wp, &payload, attr, VZERO);
	}

	return;
}

SECTION_NOR_SPIF_GEN2_CORE
s32_t flash_unit_erase_g2(const u32_t cid, const u32_t offset) {
	const nsf_write_phase_t wp = {
		.cmd  = 0x20,
		.addr = offset & (~(NORSF_CMN_ERASE_SZ_B - 1)),
	};
	nsf_trx_attr_t attr = {
		.plen_b = 0,
		.write_en = 1,
		.wlen_b = 1 + nor_spif_common_info.addr_len_b,
		.dummy_ck = 0,
		.pdir  = norsf_ptx,
		.wmode = norsf_sio,
		.pmode = norsf_sio,
	};

	spif_trx(cid, &wp, VZERO, attr, spif_wip_block);

	return 0;
}

SECTION_NOR_SPIF_GEN2_CORE
s32_t flash_unit_write_g2(const u32_t cid, u32_t offset, u32_t leng, const void *buf) {
	nsf_write_phase_t wp = {
		.cmd  = 0x02,
		.addr = offset,
	};
	nsf_trx_attr_t attr = {
		.plen_b = leng,
		.write_en = 1,
		.wlen_b = 1 + nor_spif_common_info.addr_len_b,
		.dummy_ck = 0,
		.pdir  = norsf_ptx,
		.wmode = norsf_sio,
		.pmode = norsf_sio,
	};

	spif_trx(cid, &wp, (void *)buf, attr, spif_wip_block);

	return 0;
}

SECTION_NOR_SPIF_GEN2_MISC
s32_t flash_read_g2(const u32_t cid, u32_t offset, const u32_t leng, void *buf) {
	nsf_write_phase_t wp = {
		.cmd  = 0x03,
		.addr = offset,
	};
	nsf_trx_attr_t attr = {
		.plen_b = leng,
		.write_en = 0,
		.wlen_b = 1 + nor_spif_common_info.addr_len_b,
		.dummy_ck = 0,
		.pdir  = norsf_prx,
		.wmode = norsf_sio,
		.pmode = norsf_sio,
	};

	spif_trx(cid, &wp, buf, attr, VZERO);

	return 0;
}

SECTION_NOR_SPIF_GEN2_MISC
plr_nor_spi_info_g2_t * nor_spif_common_probe(void) {
	u32_t sz_mb = nor_spif_common_info.size_per_chip_b / 1024 / 1024;
	u32_t sz_factor = 0;

	while (sz_mb) {
		sz_factor++;
		sz_mb >>= 1;
	}

	if (RFLD_SFCR2(mmio_4b_addr_en) == 1) {
		puts("4B NOR SPI-F is not supported yet... ");
	} else {
		RMOD_SFCR2(sfsize, sz_factor + 2);

		puts("Generic 3B NOR SPI-F detected... ");

		return &nor_spif_common_info;
	}

	return VZERO;
}
REG_NOR_SPIF_PROBE_FUNC(nor_spif_common_probe);
