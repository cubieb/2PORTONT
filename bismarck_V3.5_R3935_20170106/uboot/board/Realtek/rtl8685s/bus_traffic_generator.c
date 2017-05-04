#include <common.h>
#include "bus_traffic_generator.h"

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

btg_para_t req = {.btg_id        = 0,
                  .cmd           = BTG_READ,
                  .rec_latest    = BTG_REC_FIRST,
                  .precise       = BTG_IMPRECISE,
                  .rchkmode      = BTG_RCHK_DGEN,
                  .burstlen      = BTG_BLEN_128,
                  .pktlen        = 4096,
                  .iter          = 0,
                  .response_time = 0x1FF0,
                  .period_time   = 0x2000,
                  .addr_base     = 0x07800000,
                  .addr_mask     = 0x003FFFFF,
                  .addr_gap      = 4096};

void puts(const char *s);
void btg_chk_reset(void);
void btg_chk_basic_func(void);
void btg_chk_timing_debug_reg(void);
void btg_chk_read_dgen_debug_reg(void);
void btg_chk_read_csum_debug_reg(void);
void btg_chk_concurrent_rw(void);
void btg_chk_experiment_single(void);

inline static uint32_t gdma_get_base(btg_para_t *req) {
	uint32_t gdma_base = 0xb800a000;

	gdma_base += req->btg_id * 0xe000;

	return gdma_base;
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
	uint32_t tmp_cmd, i;

	for (i=0; i<2; i++) {
		req->btg_id = i;

		printf("II: Resetting BTG#%d...", i);

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
	}

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
	printf("II: WRITE in progress? %c\n", (BTGREG(req, BTGCRo) & 0x80000000) ? 'Y' : 'N');
	printf("II: Int. %d, Addr. fails W response time: %08x\n",
	       (isr >> 20) & 0x1, BTGREG(req, BTGRFARo));
	printf("II: Int. %d, Addr. fails W period:        %08x\n",
	       (isr >> 21) & 0x1, BTGREG(req, BTGPFARo));
	printf("II: Max W response time: %d(0x%08x) cycles.\n",
	       BTGREG(req, BTGMRTRo), BTGREG(req, BTGMRTRo));

	req->cmd = BTG_READ;
	printf("II: READ in progress? %c\n", (BTGREG(req, BTGCRo) & 0x80000000) ? 'Y' : 'N');
	printf("II: Int. %d, Addr. fails R response time: %08x\n",
	       (isr >> 24) & 0x1, BTGREG(req, BTGRFARo));
	printf("II: Int. %d, Addr. fails R period:        %08x\n",
	       (isr >> 25) & 0x1, BTGREG(req, BTGPFARo));
	printf("II: Int. %d, Addr. fails R verification:  %08x\n",
	       (isr >> 26) & 0x1, BTGREG(req, BTGVFARo));
	printf("II: Max R response time: %d(0x%08x) cycles.\n",
	       BTGREG(req, BTGMRTRo), BTGREG(req, BTGMRTRo));

	req->cmd = tmp_cmd;
	return;
}

void btg_async_start(void) {
	uint32_t id;

	for (id=0; id<2; id++) {
		req.btg_id = id;

		printf("II: Starting BTG#%d %s ",
		       id,
		       (req.cmd == BTG_READ) ? "READ" : "WRITE");

		if (req.cmd == BTG_READ) {
			req.addr_base -= id * 0x00400000;
		} else {
			req.addr_base += id * 0x00400000;
		}

		printf("[%08x ~ %08x]...",
		       req.addr_base, (req.addr_base | req.addr_mask));

		btg_setup(&req);
		btg_start(&req);
		puts(" OK\n");
	}

	return;
}

void btg_async_stop(void) {
	uint32_t id;
	int32_t op;

	for (id=0; id<2; id++) {
		req.btg_id = id;

		for (op=1; op>=0; op--) {
			req.cmd = op;
			printf("II: Stopping BTG#%d %s... ",
			       id,
			       (req.cmd == BTG_READ) ? "READ" : "WRITE");
			btg_stop(&req);
			puts("OK\n");
		}

		btg_result(&req);
	}

	return;
}

#define ISCMD(x)    (strcmp(x, argv[1]) == 0)
#define ISSUBCMD(x) (strcmp(x, argv[2]) == 0)
#define SETIFVAR(x) do {                         \
		if (strcmp(#x, argv[2]) == 0) {              \
			req.x = simple_strtoul(argv[3], NULL, 16); \
			return 0;                                  \
		}                                            \
	} while(0)

int btg_sub_sys(int argc, char * const argv[]) {
	if (ISCMD("start")) {
		if (argc != 3) {
			return -1;
		}

		btg_reset(&req);
		req.addr_base = initdram(0) - (8*1024*1024);
		if (ISSUBCMD("r")) {
			req.cmd = BTG_READ;
			btg_async_start();
		} else if (ISSUBCMD("w")) {
			req.cmd = BTG_WRITE;
			btg_async_start();
		} else if (ISSUBCMD("wr")) {
			req.cmd = BTG_WRITE;
			btg_async_start();
			req.cmd = BTG_READ;
			btg_async_start();
		} else if (ISSUBCMD("rw")) {
			req.cmd = BTG_READ;
			btg_async_start();
			req.cmd = BTG_WRITE;
			btg_async_start();
		} else {
			return -1;
		}
	} else if (ISCMD("stop")) {
		btg_async_stop();
	} else if (ISCMD("set")) {
		SETIFVAR(precise);
		SETIFVAR(burstlen);
		SETIFVAR(pktlen);
		SETIFVAR(iter);
		SETIFVAR(response_time);
		SETIFVAR(period_time);
		SETIFVAR(addr_base);
		SETIFVAR(addr_mask);
		return -1;
	} else if (ISCMD("info")) {
		btg_para_decode(&req);
	} else if (ISCMD("result")) {
		req.btg_id = 0;
		btg_result(&req);
		req.btg_id = 1;
		btg_result(&req);
	}
	return 0;
}
