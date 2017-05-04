#include <common.h>
#include <command.h>
#include <asm/symb_define.h>

#define FT_SYMB_STATUS 0xF7E57F16
#define FT_SYMB_PUTC   0xF7E57C5L
#define OTTO_BIOS (((soc_t *)(0x9f000020))->bios)

uint32_t *plr_status;

uint32_t *intl_test_get_symbol(uint32_t symb) {
	const symbol_table_entry_t *ste;
	uint32_t *v = 0;

	ste = symb_retrive(symb,
	                   OTTO_BIOS.header.export_symb_list,
	                   OTTO_BIOS.header.end_of_export_symb_list);
	if (ste) {
		v = (u32_t *)ste->v.pvalue;
	}

	return v;
}

void dummy_putc(int c) {
	return;
}

void otto_uart_en(uint32_t uart_en) {
	if (plr_status &&
	    ((*plr_status & 0xfffff000) == 0xF7E57000)) {
		if (uart_en) {
			OTTO_BIOS.uart_putc = (fpv_s8_t *)intl_test_get_symbol(FT_SYMB_PUTC);
		} else {
			OTTO_BIOS.uart_putc = (fpv_s8_t *)dummy_putc;
		}
	}
	return;
}

int do_uart_en_shell(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]) {
	uint32_t uart_en;

	uart_en = argv[1][0] - 0x30;
	otto_uart_en(uart_en);

	return 0;
}

U_BOOT_CMD(
        uart_en, 2, 0, do_uart_en_shell,
        "Disable/enable UART output",
        "0: Disable, 1: Enable"
);

void intl_test_uboot(void) {
	plr_status = intl_test_get_symbol(FT_SYMB_STATUS);

	if (plr_status &&
	    ((*plr_status & 0xfffff000) == 0xF7E57000)) {
		otto_uart_en(1);

		/* Setting b8000044[0] to `1' so MAC can be activated. */
		*((volatile u32_t *)0xb8000044) = (*((volatile u32_t *)0xb8000044) | 0x1);
		printf("uboot pass\n");

		otto_uart_en(0);
	}

	return;
}
PATCH_REG(intl_test_uboot, 22);
