#include <soc.h>

#ifndef SECTION_EFUSE
#define SECTION_EFUSE
#endif

#define RD_EFUSE(x)    ((*((volatile u32_t *)(0xbb000144 + (x*4)))) & 0xffff)

#define SWR1C4         (*((volatile u32_t *)0xbb0001c4))
#define SWR1C8         (*((volatile u32_t *)0xbb0001c8))
#define SWR1CC         (*((volatile u32_t *)0xbb0001cc))

#define GPHY_IND_WD    (*((volatile u32_t *)0xbb000098))
#define GPHY_IND_CMD   (*((volatile u32_t *)0xbb00009C))
#define GPHY_IND_RD    (*((volatile u32_t *)0xbb0000A0))
#define GPHY98 GPHY_IND_WD
#define GPHY9C GPHY_IND_CMD
#define GPHYA0 GPHY_IND_RD

#define SDS_IND_WD    (*((volatile u32_t *)0xbb00011C))
#define SDS_IND_CMD   (*((volatile u32_t *)0xbb000120))
#define SDS_IND_RD    (*((volatile u32_t *)0xbb000124))
#define SDS11C SDS_IND_WD
#define SDS120 SDS_IND_CMD
#define SDS124 SDS_IND_RD

#define PATCH_BREAK    (0x00000000)
#define PATCH_CONTINUE (0x0000ffff)

#define WAIT_GPHY_RDY()   do {	  \
		while (GPHY_IND_RD & (0x00010000)) { \
			; \
		} \
	} while (0)

#define WAIT_SDS_RDY()   do {	  \
		while (SDS_IND_RD & (0x00010000)) { \
			; \
		} \
	} while (0)


#define CHK_BREAK(x) do {	  \
		if ((x & 0xffff) == PATCH_BREAK) { \
			puts("break\n"); \
			return; \
		} \
	} while (0)

#define IS_CONTINUE(x) ((x & PATCH_CONTINUE) == PATCH_CONTINUE)

SECTION_EFUSE
void efuse_decode(void) {
	u16_t efuse_data;
	u32_t swr_data;
	u32_t gphy_cmd, gphy_data;
	u32_t sds_cmd, sds_data;
	u32_t lxb_val;
	u16_t gphy_num, sds_num;
	u16_t curr_addr;
	u32_t i;
	u32_t lxb_addr;

	/* static */
	if (((RD_EFUSE(2) & 0xff00) == 0) &&
	    (RD_EFUSE(3) == 0) &&
	    (RD_EFUSE(4) == 0) &&
	    (RD_EFUSE(5) == 0)) {
		puts("DD: efuse static patch bypassed\n");
		goto patch_gphy;
	}

	efuse_data = RD_EFUSE(2);
	printf("DD: efuse2: %08x => ", efuse_data);
	/* SWR part */
	if (efuse_data & 0x000000f0) {
		SWR1C8 = 0x150000;
		swr_data = (SWR1C8 & 0xfff0) + ((efuse_data >> 4) & 0xf);
		printf("%04x =>", swr_data);
		SWR1C4 = swr_data;
		SWR1CC = 0xc;
		SWR1CC = 0xe;
		SWR1CC = 0x9;
		SWR1CC = 0xa;
	} else {
		puts("skipped");
	}

	printf(" 0x01c4; %08x => 0xbcc8... ", efuse_data >> 8);
	GPHY9C = 0x20bcc8;
	gphy_data = GPHYA0;
	WAIT_GPHY_RDY();
	GPHY98 = (gphy_data & 0xff00) + (efuse_data >> 8);
	GPHY9C = 0x60bcc8;
	WAIT_GPHY_RDY();
	puts("done\n");

	puts("DD: efuse3: ");
	efuse_data = RD_EFUSE(3);
	GPHY98 = efuse_data;
	GPHY9C = 0x60bcc4;
	printf("%08x => 0xbcc4... ", efuse_data);
	WAIT_GPHY_RDY();
	puts("done\n");

	puts("DD: efuse4: ");
	efuse_data = RD_EFUSE(4);
	GPHY98 = efuse_data;
	GPHY9C = 0x60bcdc;
	printf("%08x => 0xbcdc... ", efuse_data);
	WAIT_GPHY_RDY();
	GPHY9C = 0x60bcde;
	WAIT_GPHY_RDY();
	puts("done\n");

	puts("DD: efuse5: ");
	efuse_data = RD_EFUSE(5);
	GPHY98 = efuse_data;
	GPHY9C = 0x60bce0;
	printf("%08x => 0xbce0... ", efuse_data);
	WAIT_GPHY_RDY();
	GPHY9C = 0x60bce2;
	WAIT_GPHY_RDY();
	puts("done\n");

 patch_gphy:
	puts("DD: efuse6: ");
	efuse_data = RD_EFUSE(6);
	gphy_num   = (efuse_data & 0x000f);
	sds_num    = (efuse_data & 0x00f0) >> 4;
	printf("di: %02x; sds_num: %d; gphy_num: %d\n",
	       ((efuse_data >> 8)) & 0xff, sds_num, gphy_num);
	curr_addr  = 7;

#if 1   /* gphy */
	puts("DD: efuse gphy: ");
	i = 0;
	while ((curr_addr < 32) && (i < gphy_num))
		{
			efuse_data = RD_EFUSE(curr_addr);
			if (!IS_CONTINUE(efuse_data))
				{
					gphy_cmd = RD_EFUSE(curr_addr);
					gphy_data  = RD_EFUSE((curr_addr + 1));
					GPHY98    = gphy_data;
					GPHY9C    = 0x600000 + gphy_cmd;
					printf("[%08x => %08x] ", gphy_data, gphy_cmd);
					WAIT_GPHY_RDY();
					curr_addr += 2;
					i++;
				}
			else
				{
					curr_addr++;
				}
		}
	puts("done\n");
#endif

#if 1
	puts("DD: efuse sds: ");
	i = 0;
	while ((curr_addr < 32) && (i < sds_num))
		{
			efuse_data = RD_EFUSE(curr_addr);
			if (!IS_CONTINUE(efuse_data))
				{
					sds_cmd  = RD_EFUSE(curr_addr);
					sds_data = RD_EFUSE((curr_addr + 1));
					SDS11C   = sds_data;
					SDS120   = 0x30000 + sds_cmd;
					printf("[%08x => %08x] ", sds_data, sds_cmd);
					WAIT_GPHY_RDY();
					curr_addr += 2;
					i++;
				}
			else
				{
					curr_addr++;
				}
		}
	puts("done\n");
#endif

#if 1
	/* lxb */
	puts("DD: efuse lxb: ");
	i = 0;
	while (curr_addr < 32) {
		efuse_data = RD_EFUSE(curr_addr);
		CHK_BREAK(efuse_data);
		if (!IS_CONTINUE(efuse_data))
			{
				lxb_addr = (RD_EFUSE((curr_addr + 0)) << 16) | RD_EFUSE((curr_addr + 1));
				lxb_val  = (RD_EFUSE((curr_addr + 2)) << 16) | RD_EFUSE((curr_addr + 3));
				printf("[%08x => %08x] ", lxb_val, lxb_addr);
				*((volatile u32_t *)lxb_addr) = lxb_val;
				curr_addr += 4;
			}
		else
			{
				curr_addr++;
			}
	}
	puts("done\n");
#endif
	return;
}
REG_INIT_FUNC(efuse_decode, 3);

SECTION_EFUSE
u8_t efuse_6_rd(void) {
	u8_t e6_data = (RD_EFUSE(6) >> 12) & 0x7;
	if (e6_data > 3) {
		e6_data = 0;
	}
	return e6_data;
}
