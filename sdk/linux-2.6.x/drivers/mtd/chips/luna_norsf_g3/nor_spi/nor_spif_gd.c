#include <cross_env.h>
#include <nor_spi/nor_spif_core.h>

static int norsf_gd_xread_en(const norsf_g2_info_t *ni, const u32_t cs);
static int norsf_gd_xread_ex(const norsf_g2_info_t *ni, const u32_t cs);

norsf_erase_cmd_t gd25q128c_erase_cmds[] SECTION_NOR_SPIF_GEN2_PARAM =
	{{.a.cmd       = 0xd8,
	  .a.write_en  = 1,
	  .a.pdir      = norsf_ptx,
	  .a.cmode     = norsf_sio,
	  .a.amode     = norsf_sio,
	  .a.pmode     = norsf_sio,
	  .sz_b        = 64*1024,
	  .offset_lmt  = 0,
	  .to_us       = 1200*1000},
	 {.a.cmd       = 0x52,
	  .a.write_en  = 1,
	  .a.pdir      = norsf_ptx,
	  .a.cmode     = norsf_sio,
	  .a.amode     = norsf_sio,
	  .a.pmode     = norsf_sio,
	  .sz_b        = 32*1024,
	  .offset_lmt  = 0,
	  .to_us       = 1000*1000},
	 {.a.cmd       = 0x20,
	  .a.write_en  = 1,
	  .a.pdir      = norsf_ptx,
	  .a.cmode     = norsf_sio,
	  .a.amode     = norsf_sio,
	  .a.pmode     = norsf_sio,
	  .sz_b        = 4*1024,
	  .offset_lmt  = 0,
	  .to_us       = 400*1000}};

norsf_erase_cmd_t gd25q64c_erase_cmds[] SECTION_NOR_SPIF_GEN2_PARAM =
	{{.a.cmd       = 0xd8,
	  .a.write_en  = 1,
	  .a.pdir      = norsf_ptx,
	  .a.cmode     = norsf_sio,
	  .a.amode     = norsf_sio,
	  .a.pmode     = norsf_sio,
	  .sz_b        = 64*1024,
	  .offset_lmt  = 0,
	  .to_us       = 2000*1000},
	 {.a.cmd       = 0x52,
	  .a.write_en  = 1,
	  .a.pdir      = norsf_ptx,
	  .a.cmode     = norsf_sio,
	  .a.amode     = norsf_sio,
	  .a.pmode     = norsf_sio,
	  .sz_b        = 32*1024,
	  .offset_lmt  = 0,
	  .to_us       = 1600*1000},
	 {.a.cmd       = 0x20,
	  .a.write_en  = 1,
	  .a.pdir      = norsf_ptx,
	  .a.cmode     = norsf_sio,
	  .a.amode     = norsf_sio,
	  .a.pmode     = norsf_sio,
	  .sz_b        = 4*1024,
	  .offset_lmt  = 0,
	  .to_us       = 300*1000}};

norsf_read_cmd_t gd25q128c_64c_read_cmd SECTION_NOR_SPIF_GEN2_PARAM = {
#if (NORSF_XREAD_EN == 1)
	.a.cmd      = 0xEB,
	.a.write_en = 0,
	.a.dummy_ck = 4,
	.a.pdir     = norsf_prx,
	.a.cmode    = norsf_sio,
	.a.amode    = norsf_qio,
	.a.pmode    = norsf_qio,

	.xread_en   = norsf_gd_xread_en,
	.xread_ex   = norsf_gd_xread_ex,
#else
	.a.cmd      = 0x03,
	.a.write_en = 0,
	.a.dummy_ck = 0,
	.a.pdir     = norsf_prx,
	.a.cmode    = norsf_sio,
	.a.amode    = norsf_sio,
	.a.pmode    = norsf_sio,

	.xread_en   = VZERO,
	.xread_ex   = VZERO,
#endif
};

norsf_cmd_info_t gd25q128c_cmd_info SECTION_NOR_SPIF_GEN2_PARAM = {
	.cerase = gd25q128c_erase_cmds,

	.cerase_cmd_num = sizeof(gd25q128c_erase_cmds)/sizeof(norsf_erase_cmd_t),

	.cprog_attr.cmd      = 0x02,
	.cprog_attr.pdir     = norsf_ptx,
	.cprog_attr.cmode    = norsf_sio,
	.cprog_attr.amode    = norsf_sio,
	.cprog_attr.pmode    = norsf_sio,
	.cprog_attr.write_en = 1,
	.cprog_attr.dummy_ck = 0,
	.cprog_to_us         = 2400,
	.cprog_lim_b         = 256,

	.cread = &gd25q128c_64c_read_cmd,
};

norsf_cmd_info_t gd25q64c_cmd_info SECTION_NOR_SPIF_GEN2_PARAM = {
	.cerase = gd25q64c_erase_cmds,

	.cerase_cmd_num = sizeof(gd25q64c_erase_cmds)/sizeof(norsf_erase_cmd_t),

	.cprog_attr.cmd      = 0x02,
	.cprog_attr.pdir     = norsf_ptx,
	.cprog_attr.cmode    = norsf_sio,
	.cprog_attr.amode    = norsf_sio,
	.cprog_attr.pmode    = norsf_sio,
	.cprog_attr.write_en = 1,
	.cprog_attr.dummy_ck = 0,
	.cprog_to_us         = 2400,
	.cprog_lim_b         = 256,

	.cread = &gd25q128c_64c_read_cmd,
};

norsf_g2_info_t norsf_gd25q128c_info SECTION_NOR_SPIF_GEN2_PARAM = {
	.num_chips      = 1,
	.sec_sz_b       = 4*1024,   /* Set to the sector size for UBoot or Linux */
	.size_per_chip_b= 16777216, /* Set to 0 for auto-size detection. */
	.addr_mode      = norsf_3b_addr,
	.arch_wip       = norsf_cmn_arch_wip,
	.arch_erase     = norsf_cmn_arch_erase,
	.arch_read      = norsf_cmn_arch_read,
	.arch_prog      = norsf_cmn_arch_prog,
	.cmd_info       = &gd25q128c_cmd_info,
};

norsf_g2_info_t norsf_gd25q64c_info SECTION_NOR_SPIF_GEN2_PARAM = {
	.num_chips      = 1,
	.sec_sz_b       = 4*1024,  /* Set to the sector size for UBoot or Linux */
	.size_per_chip_b= 8388608, /* Set to 0 for auto-size detection. */
	.addr_mode      = norsf_3b_addr,
	.arch_wip       = norsf_cmn_arch_wip,
	.arch_erase     = norsf_cmn_arch_erase,
	.arch_read      = norsf_cmn_arch_read,
	.arch_prog      = norsf_cmn_arch_prog,
	.cmd_info       = &gd25q64c_cmd_info,
};

SECTION_NOR_SPIF_GEN2_CORE
static u8_t norsf_gd_rdsr(u32_t chip_sel) {
	u8_t _status;
	nsf_trx_attr_t attr = {
		.cmd      = 0x35,
		.cs       = chip_sel,
		.plen_b   = 1,
		.write_en = 0,
		.dummy_ck = 0,
		.pdir     = norsf_prx,
		.cmode    = norsf_sio,
		.amode    = norsf_sio,
		.pmode    = norsf_sio,
	};
	norsf_compound_cmd(-1, &_status, attr, VZERO);
	return _status;
}

SECTION_NOR_SPIF_GEN2_CORE
static void norsf_gd_wrsr(u32_t chip_sel, u8_t status) {
	nsf_trx_attr_t attr = {
		.cmd      = 0x31,
		.cs       = chip_sel,
		.plen_b   = 1,
		.write_en = 1,
		.dummy_ck = 0,
		.pdir     = norsf_ptx,
		.cmode    = norsf_sio,
		.amode    = norsf_sio,
		.pmode    = norsf_sio,
	};
	norsf_wip_info_t wi = {
		.to_c = -1,
		.wip = norsf_cmn_arch_wip,
	};
	norsf_compound_cmd(-1, &status, attr, &wi);
	return;
}

SECTION_NOR_SPIF_GEN2_CORE
__attribute__ ((unused))
static int norsf_gd_xread_en(const norsf_g2_info_t *ni,
                             const u32_t cs) {
	u8_t status;

	status = norsf_gd_rdsr(cs);

	/* enable quad-enable bit. */
	status |= 0x20;

	norsf_gd_wrsr(cs, status);

	return 0;
}

SECTION_NOR_SPIF_GEN2_CORE
__attribute__ ((unused))
static int norsf_gd_xread_ex(const norsf_g2_info_t *ni,
                             const u32_t cs) {
	u8_t status;

	status = norsf_gd_rdsr(cs);

	/* disable quad-enable bit. */
	status &= (~0x20);

	norsf_gd_wrsr(cs, status);

	return 0;
}

SECTION_NOR_SPIF_GEN2_MISC
norsf_g2_info_t *norsf_gd_probe(void) {
	u32_t norsf_id = norsf_rdid(0);
	norsf_g2_info_t *res = VZERO;

	switch (norsf_id) {
	case 0xc84018:
		res = &norsf_gd25q128c_info;
		break;
	case 0xc84017:
		res = &norsf_gd25q64c_info;
		break;
	default:
		break;
	}

	if (res) {
		puts("GD:");
	}
	return res;
}

REG_NOR_SPIF_PROBE_FUNC(norsf_gd_probe);
