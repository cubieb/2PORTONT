#include <9601b_pad_ctrl.h>

#ifndef SECTION_NOR_SPIF_GEN2_CORE
#define SECTION_NOR_SPIF_GEN2_CORE __attribute__ ((section (".sram_text"), noinline))
#endif

SECTION_NOR_SPIF_GEN2_CORE
void norsf_proj_4b_addr_en(int enable) {
	RMOD_PIN_STS(spi_flash_4b_en, enable);
	return;
}
