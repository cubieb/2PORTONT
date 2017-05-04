#include <soc.h>
#include <soc_exam_util/bus_traffic_generator.h>
#include <soc_exam_util/mt_ram_test.h> 



/* Provided outside. */
#ifndef BTG_LX0_BASE
    #undef BTG_LX0_ADDR_PHY
    #define BTG_LX0_BASE      (0)
    #define BTG_LX0_ADDR_PHY (0)
#endif
#ifndef BTG_LX1_BASE
    #undef BTG_LX1_ADDR_PHY
    #define BTG_LX1_BASE      (0)
    #define BTG_LX1_ADDR_PHY (0)
#endif
#ifndef BTG_LX2_BASE
    #undef BTG_LX2_ADDR_PHY
    #define BTG_LX2_BASE      (0)
    #define BTG_LX2_ADDR_PHY (0)
#endif
#ifndef BTG_LX3_BASE
    #undef BTG_LX3_ADDR_PHY
    #define BTG_LX3_BASE      (0)
    #define BTG_LX3_ADDR_PHY (0)
#endif
#ifdef PROJECT_WITH_PBO_LX_BUS
    #ifndef BTG_LXUW_BASE
        #undef BTG_LXUW_ADDR_PHY
        #define BTG_LXUW_BASE      (0)
        #define BTG_LXUW_ADDR_PHY (0)
    #endif
    #ifndef BTG_LXUR_BASE
        #define BTG_LXUR_BASE      (0)
    #endif
    #ifndef BTG_LXDW_BASE
        #undef BTG_LXDW_ADDR_PHY
        #define BTG_LXDW_BASE      (0)
        #define BTG_LXDW_ADDR_PHY (0)
    #endif
    #ifndef BTG_LXDR_BASE
        #define BTG_LXDR_BASE      (0)
    #endif
#endif //#ifdef PROJECT_WITH_PBO_LX_BUS

#define END_BTG_DATA (0xEEEE)
#define NULL_BTG_DATA (0x0)

#define ISBTGAVAL(x) ((x->gbase & 0xb8000000) == 0xb8000000)
#define ISBTGNULL(x) ((x->gbase & 0xFFFFF000) == NULL_BTG_DATA)
#define ISBTGEND(x)  (x->gbase == END_BTG_DATA)


#define BURST_LEN   (3)    /* 0: 16B; 1: 32B, 2: 64B, 3: 128B */
#define PACKAGE_LEN (8160) /* from 32B ~ 8160B in a unit of 32B. */

btg_para_v3_t btg_parameter[] = {
    {
		.gbase = BTG_LX0_BASE + 0x100,
		.lxid  = 0,
		.feat  = BTG_WRITE,
        .rec_latest = 1,
        .burstlen = BURST_LEN, /* 0: 16B; 1: 32B, 2: 64B, 3: 128B */
        .pktlen = PACKAGE_LEN,  
        .resp_time = 0x80,  /* in cycles. */
        .perid_time = 0x80, /* in cycles. */
        .addr_base = BTG_LX0_ADDR_PHY,
        .addr_mask = BTG_SIZE_W,
        .addr_gap = PACKAGE_LEN,
	}, 
	{
		.gbase = BTG_LX0_BASE + 0x200,
		.lxid  = 0,
		.feat  = BTG_READ,
        .rec_latest = 1,
        .rchkmode = 1, /* 0: NONE; 1: data gen.; 2: checksum; 3: undefined. */
        .burstlen = BURST_LEN, /* 0: 16B; 1: 32B, 2: 64B, 3: 128B */
        .pktlen = PACKAGE_LEN,  /* from 32B ~ 8160B in a unit of 32B. */
        .resp_time = 0x80,  /* in cycles. */
        .perid_time = 0x80, /* in cycles. */
        .addr_base = BTG_LX0_ADDR_PHY,
        .addr_mask = BTG_SIZE_R,
        .addr_gap = PACKAGE_LEN,
	},
    {
		.gbase = BTG_LX1_BASE + 0x100,
		.lxid  = 1,
		.feat  = BTG_WRITE,
        .rec_latest = 1,
        .burstlen = BURST_LEN, /* 0: 16B; 1: 32B, 2: 64B, 3: 128B */
        .pktlen = PACKAGE_LEN,  /* from 32B ~ 8160B in a unit of 32B. */
        .resp_time = 132000,  /* in cycles. */
        .perid_time = 0x80, /* in cycles. */
        .addr_base = BTG_LX1_ADDR_PHY,
        .addr_mask = BTG_SIZE_W,
        .addr_gap = PACKAGE_LEN,
	}, 
	{
		.gbase = BTG_LX1_BASE + 0x200,
		.lxid  = 1,
		.feat  = BTG_READ,
        .rec_latest = 1,
        .rchkmode = 1, /* 0: NONE; 1: data gen.; 2: checksum; 3: undefined. */
        .burstlen = BURST_LEN, /* 0: 16B; 1: 32B, 2: 64B, 3: 128B */
        .pktlen = PACKAGE_LEN,  /* from 32B ~ 8160B in a unit of 32B. */
        .resp_time = 0x80,  /* in cycles. */
        .perid_time = 0x80, /* in cycles. */
        .addr_base = BTG_LX1_ADDR_PHY,
        .addr_mask = BTG_SIZE_R,
        .addr_gap = PACKAGE_LEN,
	},
	{
		.gbase = BTG_LX2_BASE + 0x100,
		.lxid  = 2,
		.feat  = BTG_WRITE,
        .rec_latest = 1,
        .burstlen = BURST_LEN, /* 0: 16B; 1: 32B, 2: 64B, 3: 128B */
        .pktlen = PACKAGE_LEN,  /* from 32B ~ 8160B in a unit of 32B. */
        .resp_time = 0x80,  /* in cycles. */
        .perid_time = 0x80, /* in cycles. */
        .addr_base = BTG_LX2_ADDR_PHY,
        .addr_mask = BTG_SIZE_W,
        .addr_gap = PACKAGE_LEN,
	}, 
	{
		.gbase = BTG_LX2_BASE + 0x200,
		.lxid  = 2,
		.feat  = BTG_READ,
        .rec_latest = 1,
        .rchkmode = 1, /* 0: NONE; 1: data gen.; 2: checksum; 3: undefined. */
        .burstlen = BURST_LEN, /* 0: 16B; 1: 32B, 2: 64B, 3: 128B */
        .pktlen = PACKAGE_LEN,  /* from 32B ~ 8160B in a unit of 32B. */
        .resp_time = 0x80,  /* in cycles. */
        .perid_time = 0x80, /* in cycles. */
        .addr_base = BTG_LX2_ADDR_PHY,
        .addr_mask = BTG_SIZE_R,
        .addr_gap = PACKAGE_LEN,
	},
    {
        .gbase = BTG_LX3_BASE + 0x100,
        .lxid  = 3,
        .feat  = BTG_WRITE,
        .rec_latest = 1,
        .burstlen = BURST_LEN, /* 0: 16B; 1: 32B, 2: 64B, 3: 128B */
        .pktlen = PACKAGE_LEN,  /* from 32B ~ 8160B in a unit of 32B. */
        .resp_time = 0x80,  /* in cycles. */
        .perid_time = 0x80, /* in cycles. */
        .addr_base = BTG_LX3_ADDR_PHY,
        .addr_mask = BTG_SIZE_W,
        .addr_gap = PACKAGE_LEN,
    }, 
    {
        .gbase = BTG_LX3_BASE + 0x200,
        .lxid  = 3,
        .feat  = BTG_READ,
        .rec_latest = 1,
        .rchkmode = 1, /* 0: NONE; 1: data gen.; 2: checksum; 3: undefined. */
        .burstlen = BURST_LEN, /* 0: 16B; 1: 32B, 2: 64B, 3: 128B */
        .pktlen = PACKAGE_LEN,  /* from 32B ~ 8160B in a unit of 32B. */
        .resp_time = 0x80,  /* in cycles. */
        .perid_time = 0x80, /* in cycles. */
        .addr_base = BTG_LX3_ADDR_PHY,
        .addr_mask = BTG_SIZE_R,
        .addr_gap = PACKAGE_LEN,
    },
#ifdef PROJECT_WITH_PBO_LX_BUS
    {
		.gbase = BTG_LXUW_BASE + 0x100,
		.lxid  = 3,
		.feat  = BTG_WRITE,
        .rec_latest = 1,
        .burstlen = BURST_LEN, /* 0: 16B; 1: 32B, 2: 64B, 3: 128B */
        .pktlen = PACKAGE_LEN,  /* from 32B ~ 8160B in a unit of 32B. */
        .resp_time = 600,  /* in cycles. */
        .perid_time = 0x80, /* in cycles. */
        .addr_base = BTG_LXUS_ADDR_PHY,
        .addr_mask = BTG_SIZE_W,
        .addr_gap = PACKAGE_LEN,
	}, 
	{
		.gbase = BTG_LXUR_BASE + 0x200,
		.lxid  = 4,
		.feat  = BTG_READ,
        .rec_latest = 1,
        .rchkmode = 1, /* 0: NONE; 1: data gen.; 2: checksum; 3: undefined. */
        .burstlen = BURST_LEN, /* 0: 16B; 1: 32B, 2: 64B, 3: 128B */
        .pktlen = PACKAGE_LEN,  /* from 32B ~ 8160B in a unit of 32B. */
        .resp_time = 0x80,  /* in cycles. */
        .perid_time = 0x80, /* in cycles. */
        .addr_base = BTG_LXUS_ADDR_PHY,
        .addr_mask = BTG_SIZE_R,
        .addr_gap = PACKAGE_LEN,
	},

	{
		.gbase = BTG_LXDW_BASE + 0x100,
		.lxid  = 5,
		.feat  = BTG_WRITE,
        .rec_latest = 1,
        .burstlen = BURST_LEN, /* 0: 16B; 1: 32B, 2: 64B, 3: 128B */
        .pktlen = PACKAGE_LEN,  /* from 32B ~ 8160B in a unit of 32B. */
        .resp_time = 600,  /* in cycles. */
        .perid_time = 0x80, /* in cycles. */
        .addr_base = BTG_LXDS_ADDR_PHY,
        .addr_mask = BTG_SIZE_W,
        .addr_gap = PACKAGE_LEN,
	}, 
	{
		.gbase = BTG_LXDR_BASE + 0x200,
		.lxid  = 6,
		.feat  = BTG_READ,
        .rec_latest = 1,
        .rchkmode = 1, /* 0: NONE; 1: data gen.; 2: checksum; 3: undefined. */
        .burstlen = BURST_LEN, /* 0: 16B; 1: 32B, 2: 64B, 3: 128B */
        .pktlen = PACKAGE_LEN,  /* from 32B ~ 8160B in a unit of 32B. */
        .resp_time = 0x80,  /* in cycles. */
        .perid_time = 0x80, /* in cycles. */
        .addr_base = BTG_LXDS_ADDR_PHY,
        .addr_mask = BTG_SIZE_R,
        .addr_gap = PACKAGE_LEN,
	},
#endif //PROJECT_WITH_PBO_LX_BUS
	{
		/* Sentinal entry guards end of list. */
		.gbase = 0xEEEE,
	}
};


inline static u32_t gdma_get_base(btg_para_v3_t *req) {
	return (req->gbase & 0xfffff000);
}

inline static u32_t btg_get_base(btg_para_v3_t *req) {
	return req->gbase;
}

void _btg_setup(btg_para_v3_t *req) {
	u32_t infinite = 0;
	u32_t pktlen = req->pktlen;

	if (pktlen > 8160) {
		puts("WW: packet length is capped to 8160B\n");
		pktlen = 8160;
		req->pktlen = pktlen;
	}

	/* if (pktlen & 0x1F) { */
	/* 	puts("WW: packet length is changed to 32B-alignment\n"); */
	/* 	pktlen &= (~(0x1F)); */
		req->pktlen = pktlen;
	/* } */

	if (req->iter == 0) {
		infinite = 1;
	}

	BTGREG(req, BTGCRo)  = ((req->rchkmode << 24) | (req->rec_latest << 22) |
	                        (req->precise  << 21) | (infinite << 20)        |
	                        (req->burstlen << 16) | pktlen);
	BTGREG(req, BTGIRo)  = req->iter;
	BTGREG(req, BTGRTRo) = req->resp_time;
	BTGREG(req, BTGPRo)  = req->perid_time;
	BTGREG(req, BTGBARo) = req->addr_base &(0x1FFFFFFF); /* get physical address. */
	BTGREG(req, BTGAMRo) = req->addr_mask;


    //Aligned to 4-Byte
    req->addr_gap &= 0xFFFFFFF0;

    
	if (req->addr_gap < 0) {
		req->addr_gap *= -1;
		BTGREG(req, BTGGRo) = 0x80000000 | req->addr_gap;
	} else  {
		BTGREG(req, BTGGRo) = req->addr_gap;
	}

	/* Input Vector setup */
	BTGIVEC(req, 0)  = 0xa5a55a5a;
	BTGIVEC(req, 1)  = 0xffff0000;
	BTGIVEC(req, 2)  = 0x0000ffff;
	BTGIVEC(req, 3)  = 0xff00ff00;
	BTGIVEC(req, 4)  = 0x00ff00ff;
	BTGIVEC(req, 5)  = 0x5a5aa5a5;
	BTGIVEC(req, 6)  = 0x01234567;
	BTGIVEC(req, 7)  = 0x89abcdef;
	BTGIVEC(req, 8)  = 0xaaaa5555;
	BTGIVEC(req, 9)  = 0x5555aaaa;
	BTGIVEC(req, 10) = 0xa5a55a5a;
	BTGIVEC(req, 11) = 0xffff0000;
	BTGIVEC(req, 12) = 0x0000ffff;
	BTGIVEC(req, 13) = 0xff00ff00;
	BTGIVEC(req, 14) = 0x00ff00ff;
	BTGIVEC(req, 15) = 0x5a5aa5a5;

	return;
}

void _btg_start(btg_para_v3_t *req) {
	volatile u32_t *region;

	printf("II: BTG-LX%d(%s) ", req->lxid, (req->feat == BTG_WRITE) ? "WR" : "RD");

	if (!ISBTGAVAL(req)) {
		puts("is NOT avaiable\n");
	} else {
		printf("@ 0x%08x: 0x%08x ~ 0x%08x... ",
		       req->gbase, req->addr_base, (req->addr_base | req->addr_mask));

		if (req->feat == BTG_WRITE) {
			region = (u32_t *)(req->addr_base | 0xa0000000);
			while ((u32_t)region < (req->addr_base | req->addr_mask | 0xa0000000)) {
				*region++ = 0x77777777;
			}
		}

		BTGREG(req, BTGCRo) |= 0x80000000;
		puts("enabled\n");
	}
	return;
}

void _btg_stop(btg_para_v3_t *req) {
	printf("II: BTG-LX%d(%s) ", req->lxid, (req->feat == BTG_WRITE) ? "WR" : "RD");

	if (!ISBTGAVAL(req)) {
		puts("is NOT avaiable\n");
	} else {
		printf("@ 0x%08x: 0x%08x ~ 0x%08x... ",
		       req->gbase, req->addr_base, (req->addr_base | req->addr_mask));
		BTGREG(req, BTGCRo) &= 0x7FFFFFFF;
		puts("stopped\n");
	}

	return;
}

void _gdma_show_reg(btg_para_v3_t *req)
{
	if (!ISBTGAVAL(req)) {
		printf("II: BTG-LX%d(%s) is NOT available\n", req->lxid, (req->feat == BTG_WRITE) ? "WR" : "RD");
	} else {
		printf("II: BTG-LX%d (GDMA):\n", req->lxid);
		printf("II:   %p: 0x%08x 0x%08x 0x%08x\n",
		       gdma_get_base(req), GDMAREG(req, GDMACRo), GDMAREG(req, GDMAIMRo), GDMAREG(req, GDMAISRo));
	}
	return;
}

void _btg_show_reg(btg_para_v3_t *req) {
	volatile u32_t *btgreg = (volatile u32_t *)req->gbase;

	u32_t i = 0;

	if (!ISBTGAVAL(req)) {
		printf("II: BTG-LX%d(%s) is NOT available\n", req->lxid, (req->feat == BTG_WRITE) ? "WR" : "RD");
	} else {
		printf("II: BTG-LX%d(%s) @ 0x%08x:\n",
		       req->lxid,
		       (req->feat == BTG_WRITE) ? "WR" : "RD",
		       req->gbase);
		while (i < 28) {
			printf("II:   %p: 0x%08x 0x%08x 0x%08x 0x%08x\n",
			       btgreg,
			       *(btgreg+0), *(btgreg+1), *(btgreg+2), *(btgreg+3));
			btgreg += 4;
			i += 4;
		}
	}
	return;
}

void _btg_para_decode(btg_para_v3_t *req) {
	u32_t isr, isr_offset;

	if (!ISBTGAVAL(req)) {
		printf("II: BTG-LX%d(%s) is NOT available\n", req->lxid, (req->feat == BTG_WRITE) ? "WR" : "RD");
	} else {
		isr = GDMAREG(req, GDMAISRo);
		isr_offset = (req->feat == BTG_WRITE) ? 0 : 4;

		printf("\nII: BTG-LX%d(%s) @ 0x%08x Configuration:=========\n",
		       req->lxid,
		       (req->feat == BTG_WRITE) ? "WR" : "RD",
		       req->gbase);
		printf("II:   rec_latest: %d\tprecise: %d\n", req->rec_latest, req->precise);
		printf("II:   rchkmode:  %d\tburstlen: %d B\n", req->rchkmode, 1 << (4+req->burstlen));
		printf("II:   pktlen: %d B\titer: %d\n", req->pktlen, req->iter);
		printf("II:   resp_time: %d C\tperid_time: %d C\n", req->resp_time, req->perid_time);
		printf("II:   addr_base: %08x\taddr_mask: %08x\n", req->addr_base, req->addr_mask);
		printf("II:   addr_gap: %08x\n", req->addr_gap);
		printf("II:   covered range: %08x ~ %08x\n",
		       req->addr_base, (req->addr_base | req->addr_mask));
		puts("II: Status:\n");
		printf("II:   Int. Status Reg.: %08x\n", isr);
		printf("II:   In progress? %c\n", (BTGREG(req, BTGCRo) & 0x80000000) ? 'Y' : 'N');
		printf("II:   Failed response time: Intr.(%d);  Addr.(0x%08x)\n",
		       (isr >> (20 + isr_offset)) & 0x1, BTGREG(req, BTGRFARo));
		printf("II:   Failed period       : Intr.(%d);  Addr.(0x%08x)\n",
		       (isr >> (21 + isr_offset)) & 0x1, BTGREG(req, BTGPFARo));
		if (req->feat == BTG_READ) {
		    printf("II:   Failed verification : Intr.(%d);  Addr.(0x%08x)\n",
			       (isr >> 26) & 0x1, BTGREG(req, BTGVFARo));
		}
		printf("II:   Max response time: %d(0x%x) cycles\n",
		       BTGREG(req, BTGMRTRo), BTGREG(req, BTGMRTRo));
	}
	return;
}

void _gdma_reset(btg_para_v3_t *req)
{
	printf("II: Resetting GDMA-LX%d... (0x%08x)", req->lxid, gdma_get_base(req));

	/* Reset GDMA and BTG */
	GDMAREG(req, GDMACRo) = 0x00000000;
	GDMAREG(req, GDMACRo) = 0x10000000;
	GDMAREG(req, GDMACRo) = 0x90000000;

	/* Reset Interrupt bits */
	GDMAREG(req, GDMAIMRo) = 0x07300000;
	GDMAREG(req, GDMAISRo) = 0x07300000;
	GDMAREG(req, GDMAISRo) = 0x00000000;

	puts("done;\n");
	return;
}

void _btg_print_error(btg_para_v3_t *req)
{
	u32_t isr, isr_err;

	if (!ISBTGAVAL(req)) {
		printf("II: BTG-LX%d(%s) is NOT available\n", req->lxid, (req->feat == BTG_WRITE) ? "WR" : "RD");
	} else {
		isr = GDMAREG(req, GDMAISRo);

        if (req->feat == BTG_WRITE) {
            isr_err = (isr>>20)&0x1;
            if(isr_err) printf(" EE: BTG%d(%s) Failed write response time: Intr.(%d);  Addr.(0x%08x)\n", req->lxid, (req->feat == BTG_WRITE) ? "WR" : "RD",isr_err, BTGREG(req, BTGRFARo));

            isr_err = (isr>>21)&0x1;
            if(isr_err) printf(" EE: BTG%d(%s) Failed write period       : Intr.(%d);  Addr.(0x%08x)\n", req->lxid, (req->feat == BTG_WRITE) ? "WR" : "RD",isr_err, BTGREG(req, BTGPFARo));
        }


        if (req->feat == BTG_READ) {
            isr_err = (isr>>24)&0x1;
            if(isr_err) printf(" EE: BTG%d(%s) Failed read  response time: Intr.(%d);  Addr.(0x%08x)\n", req->lxid, (req->feat == BTG_WRITE) ? "WR" : "RD",isr_err, BTGREG(req, BTGRFARo));

            isr_err = (isr>>25)&0x1;
            if(isr_err) printf(" EE: BTG%d(%s) Failed read  period       : Intr.(%d);  Addr.(0x%08x)\n", req->lxid, (req->feat == BTG_WRITE) ? "WR" : "RD",isr_err, BTGREG(req, BTGPFARo));

            isr_err = (isr>>26)&0x1;
            if(isr_err){                    
                printf(" EE: BTG%d(%s) Failed read  verification : Intr.(%d);  Addr.(0x%08x)\n", req->lxid, (req->feat == BTG_WRITE) ? "WR" : "RD",isr_err, BTGREG(req, BTGVFARo));
                return;
            }
        }        
	}
    return;
}


#ifdef PROJECT_WITH_PBO_LX_BUS
void btg_pbo_like_data_init(void)
{
    //LX0_BTG : 128 / 128 / 800 / 800 (Burst Len / PKT Len / WPeriod / RPeriod)
    btg_parameter[0].addr_gap  = btg_parameter[1].addr_gap  = 128;
    btg_parameter[0].pktlen    = btg_parameter[1].pktlen    = 128;
    btg_parameter[0].burstlen  = btg_parameter[1].burstlen  = 3;
    btg_parameter[0].perid_time= btg_parameter[1].perid_time= 800;

    // LX1_BTG : 64 / 65 / 700 /700 (Burst Len / PKT Len / WPeriod / RPeriod)
    btg_parameter[2].addr_gap  = btg_parameter[3].addr_gap  = 65;
    btg_parameter[2].pktlen    = btg_parameter[3].pktlen    = 65;
    btg_parameter[2].burstlen  = btg_parameter[3].burstlen  = 2;
    btg_parameter[2].perid_time= btg_parameter[3].perid_time= 700;

    // LX2_BTG : 32 / 20 / 900 / 900 (Burst Len / PKT Len / WPeriod / RPeriod)
    btg_parameter[4].addr_gap  = btg_parameter[5].addr_gap  = 20;
    btg_parameter[4].pktlen    = btg_parameter[5].pktlen    = 32;
    btg_parameter[4].burstlen  = btg_parameter[5].burstlen  = 1;
    btg_parameter[4].perid_time= btg_parameter[5].perid_time= 900;

    // LX3_BTG : 32 / 20 / 900 / 900 (Burst Len / PKT Len / WPeriod / RPeriod)
    btg_parameter[6].addr_gap  = btg_parameter[7].addr_gap  = 20;
    btg_parameter[6].pktlen    = btg_parameter[7].pktlen    = 32;
    btg_parameter[6].burstlen  = btg_parameter[7].burstlen  = 1;
    btg_parameter[6].perid_time= btg_parameter[7].perid_time= 900;

    // LXDS_W_BTG: 64 / 64 / 215 / N.A. (Burst Len / PKT Len / WPeriod / RPeriod)
    btg_parameter[8].addr_gap  = 64;
    btg_parameter[8].pktlen    = 64;
    btg_parameter[8].burstlen  = 2;
    btg_parameter[8].perid_time= 215;

    // LXDS_R_BTG: 64 / 192 / N.A. / 625 (Burst Len / PKT Len / WPeriod / RPeriod)
    btg_parameter[9].addr_gap  = 192;
    btg_parameter[9].pktlen    = 192;
    btg_parameter[9].burstlen  = 2;
    btg_parameter[9].perid_time= 625;

    // LXUS_W_BTG: 64 / 64 / 215 / N.A. (Burst Len / PKT Len / WPeriod / RPeriod)
    btg_parameter[10].addr_gap  = 64;
    btg_parameter[10].pktlen    = 64;
    btg_parameter[10].burstlen  = 2;
    btg_parameter[10].perid_time= 215;

    // LXUS_R_BTG: 64 / 64 / 215 / N.A. (Burst Len / PKT Len / WPeriod / RPeriod)
    btg_parameter[11].addr_gap  = 64;
    btg_parameter[11].pktlen    = 64;
    btg_parameter[11].burstlen  = 2;
    btg_parameter[11].perid_time= 215;
}
#endif


void _btg_func_4database(fpv_btg_t *func_ptr, btg_para_v3_t *req)
{
    while (!ISBTGEND(req)) {
        if(!ISBTGNULL(req)){
            func_ptr(req);
        }
        req++;
        udelay(5);
    }
}

void btg_check_exist(btg_para_v3_t *req)
{
    while (!ISBTGEND(req)) {
        if(ISBTGNULL(req)){
            printf("WW: BTG_LX_%d NOT defined.\n",req->lxid);
        }
		req++;
    }
}

s32_t btg_concur_mt_test(u32_t mt_loops)
{
    btg_para_v3_t *btg_entry = &btg_parameter[0];
    BTG_STOP_ALL(btg_entry);
    btg_check_exist(btg_entry);    
    GDMA_RESET_ALL(btg_entry);
    BTG_SETUP_ALL(btg_entry);
    BTG_START_ALL(btg_entry);
    BTG_PARA_DECODE_ALL(btg_entry);
    
    u32_t mt_cnt=1;
    s32_t ret=0;
    while(mt_cnt < mt_loops){
        printf("II: %d-runs mt_ram_test() \n",mt_cnt++);
        ret = mt_ram_test(MT_DRAM_ADDR_V, MT_DRAM_SIZE);

        BTG_PRI_ERR_ALL(btg_entry);
        if(ret != 0) {
            printf("EE: btg_concur failed.\n");
            return ret;
        }
    }
        
    puts("\n");
    BTG_SHOW_REG_ALL(btg_entry);
    BTG_STOP_ALL(btg_entry);
    return 0;
}


