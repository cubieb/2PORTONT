#include <common.h>
#include "bus_traffic_generator.h"

#define MAX_BTG_NUM 2

#define BTG_WRITE 0
#define BTG_READ  1

#define BTG_RCHK_NONE 0
#define BTG_RCHK_DGEN 1
#define BTG_RCHK_CSUM 2

#define BTG_REC_FIRST  0
#define BTG_REC_LATEST 1

#define BTG_IMPRECISE 0
#define BTG_PRECISE   1

#define BTG_FINITE   0
#define BTG_INFINITE 1

#define BTG_BLEN_16  0
#define BTG_BLEN_32  1
#define BTG_BLEN_64  2
#define BTG_BLEN_128 3

#define GDMAREG(req, oft) (*((volatile uint32_t *)(gdma_get_base(req) + oft)))
#define BTGREG(req, oft)  (*((volatile uint32_t *)(btg_get_base(req) + oft)))
#define BTGIVEC(req, n)   (*((volatile uint32_t *)(btg_get_base(req) + 0x30 + (n*4))))

btg_para_t req[MAX_BTG_NUM] = {
                {
                  .btg_id		 = 0,
				  .cmd			 = BTG_READ,
				  .rec_latest	 = BTG_REC_FIRST,
				  .precise		 = BTG_IMPRECISE,
				  .rchkmode		 = BTG_RCHK_DGEN,
				  .burstlen		 = BTG_BLEN_128,
				  .pktlen		 = 4096,
				  .iter			 = 0,
				  .response_time = 0x1FF0,
				  .period_time	 = 0x2000,
				  .addr_base	 = 0x00000000,
				  .addr_mask	 = 0x001FFFFF,
				  .addr_gap		 = 4096
			    },{
				  .btg_id		 = 1,
                  .cmd           = BTG_READ,
                  .rec_latest    = BTG_REC_FIRST,
                  .precise       = BTG_IMPRECISE,
                  .rchkmode      = BTG_RCHK_DGEN,
                  .burstlen      = BTG_BLEN_128,
                  .pktlen        = 4096,
                  .iter          = 0,
                  .response_time = 0x1FF0,
                  .period_time   = 0x2000,
				  .addr_base	 = 0x00000000,
				  .addr_mask	 = 0x001FFFFF,
				  .addr_gap		 = 4096
				}};

void puts(const char *s);
void btg_chk_reset(void);
void btg_chk_basic_func(void);
void btg_chk_timing_debug_reg(void);
void btg_chk_read_dgen_debug_reg(void);
void btg_chk_read_csum_debug_reg(void);
void btg_chk_concurrent_rw(void);
void btg_chk_experiment_single(void);

inline static uint32_t gdma_get_base(btg_para_t *req) {
#if 0 
	uint32_t gdma_base = 0xb800a000;

	gdma_base += req->btg_id * 0xe000;

	return gdma_base;
#else
	if (req) {
		if (1==req->btg_id) {
			return 0xB8018000;
		} else {
			return 0xB800A000;
		}
	}
	return 0;
#endif
}

inline static uint32_t btg_get_base(btg_para_t *req) {
	return gdma_get_base(req) + (0x100 * (req->cmd + 1));
}

inline void reset_mem_region(void) {
	uint32_t i;
	uint32_t *region_base0 = (uint32_t *)0xa1000000;
	uint32_t *region_base1 = (uint32_t *)0xa2000000;

	for (i=0; i<(0x100000/4); i++) {
		*region_base0 = 0x05D650C0;
		*region_base1 = 0x05D650C0;
		region_base0++;
		region_base1++;
	}

	return;
}

void btg_reset(btg_para_t *req) {
	uint32_t tmp_cmd, id;

	//for (id=0; id<2; id++) {
		assert(req->btg_id == id);

		printf("II: Resetting BTG#%d...", id);

		/* Reset GDMA and BTG */
		GDMAREG(req, GDMACRo) = 0x00000000;
		GDMAREG(req, GDMACRo) = 0x10000000;
		GDMAREG(req, GDMACRo) = 0x90000000;

		/* Reset Interrupt bits */
		GDMAREG(req, GDMAIMRo) = 0x07300000;
		GDMAREG(req, GDMAISRo) = 0x07300000;
		GDMAREG(req, GDMAISRo) = 0x00000000;

		/* Reset BTG */
		tmp_cmd = req->cmd;

		req->cmd = BTG_WRITE;
		BTGREG(req, BTGCRo) = 0;

		req->cmd = BTG_READ;
		BTGREG(req, BTGCRo) = 0;

		req->cmd = tmp_cmd;

		puts(" OK\n");
	//}

	return;
}

void btg_wait(btg_para_t *req) {
	while (BTGREG(req, BTGCRo) & 0x80000000);

	return;
}

void btg_start(btg_para_t *req) {
	BTGREG(req, BTGCRo) |= 0x80000000;

	return;
}

void btg_stop(btg_para_t *req) {
	BTGREG(req, BTGCRo) &= 0x7FFFFFFF;

	return;
}

void btg_setup(btg_para_t *req) {
	uint32_t infinite = 0;
	uint32_t pktlen = req->pktlen;

	if (pktlen > 8160) {
		puts("WW: packet length is capped to 8160B.\n");
		pktlen = 8160;
		req->pktlen = pktlen;
	}

	if (pktlen & 0x1F) {
		puts("WW: packet length is changed to 32B-alignment.\n");
		pktlen &= (~(0x1F));
		req->pktlen = pktlen;
	}

	if (req->iter == 0) {
		infinite = 1;
	}

	BTGREG(req, BTGCRo)  = ((req->rchkmode << 24) | (req->rec_latest << 22) |
	                        (req->precise  << 21) | (infinite << 20)        |
	                        (req->burstlen << 16) | pktlen);
	BTGREG(req, BTGIRo)  = req->iter;
	BTGREG(req, BTGRTRo) = req->response_time;
	BTGREG(req, BTGPRo)  = req->period_time;
	BTGREG(req, BTGBARo) = req->addr_base & (~(0xF0000000)); /* get physical address. */
	BTGREG(req, BTGAMRo) = req->addr_mask;

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

void btg_para_decode(btg_para_t *req) {
	printf("II: rec_latest: %d\tprecise: %d\n", req->rec_latest, req->precise);
	printf("II: rchkmode:  %d\tburstlen: %d B\n", req->rchkmode, 1 << (4+req->burstlen));
	printf("II: pktlen: %d B\titer: %d\n", req->pktlen, req->iter);
	printf("II: response_time: %d C\n", req->response_time);
	printf("II: period_time: %d C\n", req->period_time);
	printf("II: addr_base: %08x\taddr_mask: %08x\n", req->addr_base, req->addr_mask);
	printf("II: covered range: %08x ~ %08x\n",
	       req->addr_base, (req->addr_base | req->addr_mask));
	return;
}

void btg_result(btg_para_t *req) {
	uint32_t isr = GDMAREG(req, GDMAISRo);
	uint32_t tmp_cmd = req->cmd;

	printf("II: GDMA#%d Int. Status Reg.: %08x\n", req->btg_id, isr);

	req->cmd = BTG_WRITE;
	printf("II: WRITE in progress? %c (current addr. 0x%08x)\n", (BTGREG(req, BTGCRo) & 0x80000000) ? 'Y' : 'N', BTGREG(req, BTGCARo));
	printf("II: Int. %d, Addr. (0x%08x) fails W response time\n",
	       (isr >> 20) & 0x1, BTGREG(req, BTGRFARo));
	printf("II: Int. %d, Addr. (0x%08x) fails W period\n",
	       (isr >> 21) & 0x1, BTGREG(req, BTGPFARo));
	printf("II: Max W response time: %d(0x%08x) cycles.\n",
	       BTGREG(req, BTGMRTRo), BTGREG(req, BTGMRTRo));

	req->cmd = BTG_READ;
	printf("II: READ in progress? %c (current addr. 0x%08x)\n", (BTGREG(req, BTGCRo) & 0x80000000) ? 'Y' : 'N', BTGREG(req, BTGCARo));
	printf("II: Int. %d, Addr. (0x%08x) fails R response time\n",
	       (isr >> 24) & 0x1, BTGREG(req, BTGRFARo));
	printf("II: Int. %d, Addr. (0x%08x) fails R period\n",
	       (isr >> 25) & 0x1, BTGREG(req, BTGPFARo));
	printf("II: Int. %d, Addr. (0x%08x) fails R verification\n",
	       (isr >> 26) & 0x1, BTGREG(req, BTGVFARo));
	printf("II: Max R response time: %d(0x%08x) cycles.\n",
	       BTGREG(req, BTGMRTRo), BTGREG(req, BTGMRTRo));

	req->cmd = tmp_cmd;
	return;
}

void btg_async_start(uint32_t id) {
	//uint32_t addr_base;

	//for (id=0; id<2; id++) {
		assert(req[id].btg_id==id);

		printf("II: Starting BTG#%d %s ",
			   id, (req[id].cmd == BTG_READ) ? "READ" : "WRITE");
#if 0
		if (req[id].cmd == BTG_READ) {
			req[id].addr_base -= id * 0x00400000;
		} else {
			req[id].addr_base += id * 0x00400000;
		}
#else
		req[id].addr_base = 0x0 + id * 0x00400000;
#endif

		printf("[%08x ~ %08x]...",
			   req[id].addr_base, (req[id].addr_base | req[id].addr_mask));

		btg_setup(&req[id]);
		btg_start(&req[id]);
		puts(" OK\n");
	//}

	return;
}

void btg_async_stop(uint32_t id) {
	int32_t op;
	uint32_t count;
	
	if (99==id) {
        count=MAX_BTG_NUM;
        id=count-1;
	} else count=1;


	//for (id=0; id<2; id++) {
	while(count>0) {
		assert(req[id].btg_id == id);

		for (op=1; op>=0; op--) {
			req[id].cmd = op;
			printf("II: Stopping BTG#%d %s... ",
			       id,
				   (req[id].cmd == BTG_READ) ? "READ" : "WRITE");
			btg_stop(&req[id]);
			puts("OK\n");
		}

		btg_result(&req[id]);
		count--;
		id--;
	};
	//}

	return;
}

#define ISSUBCMD(x)	(strcmp(x, argv[1]) == 0)
#define GETID()     (simple_strtoul(argv[2], NULL, 2))
#define ISOPT(x)    (strcmp(x, argv[3]) == 0)
#define SETIFVAR(i, x) do {						 \
		if (strcmp(#x, argv[3]) == 0) {				 \
			req[i].x = simple_strtoul(argv[4], NULL, 16); \
			return 0;                                  \
		}                                            \
	} while(0)

int btg_sub_sys(int argc, char * const argv[]) {
 /*     "  start <btg_id> r|w|wr|rw   - start specific BTG for READ/WRITE/WRITE&READ/READ&WRITE test\n"
        "  stop [<btg_id>]            - stop all BTG operations\n"
        "  info [<btg_id>]            - show BTG parameters\n"
        "  result [<btg_id>]          - show BTG status\n"
        "  set <btg_id> <arg> <val>   - set val to arg\n"
        "          precise: 0 for imprecise; 1 for precise\n"
        "         burstlen: 0 for 16, 1 for 32, 2 for 64, and 4 for 128 bytes\n"
        "           pktlen: 32 ~ 8160 bytes\n"
        "             iter: 0 for infinite; others for 1 ~ 4G times\n"
        "    response_time: 0 ~ 4G cycles\n"
        "      period_time: 0 ~ 4G cycles\n"
        "        addr_base: physical address BTG starts from\n"
        "        addr_mask: combine with addr_base to form a region\n"*/

	uint32_t id=99;

	if (ISSUBCMD("start")) {
		id = GETID();
		
		btg_reset(&req[id]);
		/*if (0==req.addr_base) {
		req.addr_base = initdram(0) - (8*1024*1024);
			printf("WW: address base modified as 0x%08x.\n", req.addr_base);
		}*/
		
		if (ISOPT("r")) {
			req[id].cmd = BTG_READ;
			btg_async_start(id);
			return 0;
		} else if (ISOPT("w")) {
			req[id].cmd = BTG_WRITE;
			btg_async_start(id);
			return 0;
		} else if (ISOPT("wr")) {
			req[id].cmd = BTG_WRITE;
			btg_async_start(id);
			req[id].cmd = BTG_READ;
			btg_async_start(id);
			return 0;
		} else if (ISOPT("rw")) {
			req[id].cmd = BTG_READ;
			btg_async_start(id);
			req[id].cmd = BTG_WRITE;
			btg_async_start(id);
			return 0;
		}
	} else if (ISSUBCMD("stop")) {
	    if (argc > 2) {id = GETID();}
		btg_async_stop(id);
		return 0;
	} else if (ISSUBCMD("set")) {
	    id = GETID();
		if (id>MAX_BTG_NUM)
		    goto err;
		SETIFVAR(id, precise);
		SETIFVAR(id, burstlen);
		SETIFVAR(id, pktlen);
		SETIFVAR(id, iter);
		SETIFVAR(id, response_time);
		SETIFVAR(id, period_time);
		SETIFVAR(id, addr_base);
		SETIFVAR(id, addr_mask);
		return 0;
	} else if (ISSUBCMD("info")) {
		btg_para_decode(&req[id]);
		return 0;
	} else if (ISSUBCMD("result")) {
	    if (argc > 2) {id = GETID();}
		if (0==id || 99==id) {
		    req[id].btg_id = 0;
		    btg_result(&req[id]);
	    }
		if (1==id || 99==id) {
		    req[id].btg_id = 1;
		    btg_result(&req[id]);
	}
	return 0;
	} 
err:	
	printf("WW: command incomplete! \n");
	return -1;
}
