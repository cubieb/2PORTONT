#include <init_define.h>
#include <uart/uart.h>
#include <util.h>

#define STACK_GUIDE     0xcafecafe

#define SOC_A0         0x0371
#define SOC_B0         0x6266
#define CHIP_ID_REG		 0xBB010004
#define BOND_CHIP_MODE 0xBB010008	/* bond chip information */
		#define PAD_BOND_DMY_0 (1 << 0) //reserved
		#define PAD_BOND_DMY_1 (1 << 1)
		#define PAD_BOND_DMY_2 (1 << 2)
		#define PAD_BOND_CHIP_MODE_0	(1 << 3)
		#define PAD_BOND_CHIP_MODE_1	(1 << 4)
        #define CHIP_901      (PAD_BOND_DMY_0|PAD_BOND_DMY_1)
        #define CHIP_906_1    (PAD_BOND_DMY_0|PAD_BOND_DMY_1|PAD_BOND_DMY_2)
        #define CHIP_906_2    (PAD_BOND_DMY_0|PAD_BOND_CHIP_MODE_0)
        #define CHIP_907      (PAD_BOND_CHIP_MODE_0|PAD_BOND_DMY_0|PAD_BOND_DMY_1)
        #define CHIP_902      (PAD_BOND_CHIP_MODE_0|PAD_BOND_DMY_0|PAD_BOND_DMY_2)
        #define CHIP_903      (PAD_BOND_CHIP_MODE_0|PAD_BOND_DMY_0|PAD_BOND_DMY_1|PAD_BOND_DMY_2)
        #define CHIP_96       (PAD_BOND_CHIP_MODE_1|PAD_BOND_DMY_0|PAD_BOND_DMY_1)
        #define CHIP_98B      (PAD_BOND_CHIP_MODE_1|PAD_BOND_DMY_0|PAD_BOND_DMY_1|PAD_BOND_DMY_2)
		#define CHIP_2510	(PAD_BOND_CHIP_MODE_0 | PAD_BOND_CHIP_MODE_1 | PAD_BOND_DMY_0)
        #define CHIP_96P      (PAD_BOND_CHIP_MODE_0 | PAD_BOND_CHIP_MODE_1 |PAD_BOND_DMY_1 | PAD_BOND_DMY_0)
        #define CHIP_DBG      (PAD_BOND_CHIP_MODE_0|PAD_BOND_CHIP_MODE_1|PAD_BOND_DMY_0|PAD_BOND_DMY_1|PAD_BOND_DMY_2)

u32_t util_ms_accumulator SECTION_SDATA =0;

// message
const char __msg_load_page_fail[] SECTION_SDATA = {"loading virtual page %d fail\n"};
static char _banner_msg[] SECTION_RECYCLE_DATA = {"PRELOADER.%x.%x\n"};


struct soc_reg_remap_t {
	volatile u32_t *SOC_IO_MODE_EN; /* IO_MODE_EN : Enable interface IO */
	volatile u32_t *SOC_GPIO_CTRL_2;
};

const struct soc_reg_remap_t soc_reg_remap_a0 = {
	.SOC_IO_MODE_EN=(volatile u32_t *)0xBB023020,
	.SOC_GPIO_CTRL_2=(volatile u32_t *)0xBB000118,
	/*add remap register at here*/
};

/*soc 6266  address map table*/
const struct soc_reg_remap_t soc_reg_remap_b0 = {
	.SOC_IO_MODE_EN=(volatile u32_t *)0xBB023018,
	.SOC_GPIO_CTRL_2=(volatile u32_t *)0xBB0000F0,
	/*add remap register at here*/
};

static struct soc_reg_remap_t soc_reg_remap;

#define IO_MODE_EN (soc_reg_remap.SOC_IO_MODE_EN)
#define GPIO_CTRL_2 (soc_reg_remap.SOC_GPIO_CTRL_2)

static void feature_SOC_A0(u32_t bond_id, u32_t chip_ver) {
	switch(bond_id){
	default:
	case CHIP_901:
	case CHIP_906_2:
	case CHIP_907:
	case CHIP_902:
	case CHIP_903:
	case CHIP_96:
		*IO_MODE_EN = *IO_MODE_EN & ~(1<<3); //bit[3] is 0
		*GPIO_CTRL_2 = *GPIO_CTRL_2 | (1<<1); //EN_GPIO[1] is 1
		break;
	case CHIP_906_1:
	case CHIP_98B:
	case CHIP_DBG:
		*IO_MODE_EN = *IO_MODE_EN | (1<<3); //bit[3] is 1
		*GPIO_CTRL_2 = *GPIO_CTRL_2 & ~(1<<1); //EN_GPIO[1] is 0
		break;
	}
	return;
}

static void feature_SOC_B0(u32_t bond_id, u32_t chip_ver) {
	switch (chip_ver) {
	default:
	case 0x4:
		REG32(0xb8001074) |= (0x1 << 11); /* ADDRB29_LOCK_DIS */
	case 0x3:
		REG32(0xb8000108) |= (0x1 << 23); /* LX_ARB_DEL_EN */
		REG32(0xb8001074) |= (0x1 << 31); /* ARB_SEL */
	case 0x2:
	case 0x1:
		break;
	}

	/* For UART */
	switch(bond_id){
	default:
	case CHIP_901:
	case CHIP_906_2:
	case CHIP_907:
	case CHIP_902:
	case CHIP_903:
	case CHIP_96:
	case CHIP_2510:
	case CHIP_96P:
		*IO_MODE_EN = *IO_MODE_EN | (4<<3); //bit [5:3] is 100
		break;
	case CHIP_906_1:
	case CHIP_98B:
	case CHIP_DBG:
		*IO_MODE_EN = *IO_MODE_EN | (1<<3); //bit [5:3] is 001
		break;
	}

	return;
}

static u32_t soc_reg_init(void) {
	u32_t chip_ver, soc_id, bond_id;

	REG32(CHIP_ID_REG) = 0xa0000000;
	soc_id = (REG32(CHIP_ID_REG) & 0xffff);
	chip_ver = ((REG32(CHIP_ID_REG) >> 16) & 0x1f);

	REG32(BOND_CHIP_MODE) = 0xb0000000;
	bond_id = (REG32(BOND_CHIP_MODE) & 0xff);

	switch (soc_id) {
	default:
	case SOC_B0:
		soc_reg_remap = soc_reg_remap_b0;
		feature_SOC_B0(bond_id, chip_ver);
		break;
	case SOC_A0:
		soc_reg_remap = soc_reg_remap_a0;
		feature_SOC_A0(bond_id, chip_ver);
		break;
	}

	return chip_ver;
}

UTIL_FAR SECTION_UNS_TEXT void
puts(const char *s) {
	inline_puts(s);
}

SECTION_RECYCLE void
plr_init_utility(void) {
	u32_t chip_ver = soc_reg_init();

	// 2. init uart
	uart_init(uart_baud_rate, lx_bus_freq_mhz);
	_bios.uart_putc=uart_putc;
	_bios.uart_getc=uart_getc;
	_bios.uart_tstc=uart_tstc;

// 3. using printf showing the banner
	printf(_banner_msg, _soc_header.version, chip_ver);

	// 4. initial utility run-time value
	/* util_cpu_mhz = GET_CPU_HZ()/1000000; */

	// 5. put stack guide words
	/* extern u32_t farthest_stack_position; */
	/* u32_t *cur_sp; */
	/* __asm__ __volatile__  ("addiu %0, $29, -4": "=r"(cur_sp)); */
	/* u32_t *sp_end=(u32_t *)(0x9f000000 + 16*1024); */
	/* while (cur_sp != sp_end)  */
	/*     *(cur_sp--)=STACK_GUIDE; */
}

REG_INIT_FUNC(plr_init_utility, 1);
