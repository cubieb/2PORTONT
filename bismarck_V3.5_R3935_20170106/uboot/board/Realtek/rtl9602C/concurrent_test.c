#include <common.h>
#include "bus_traffic_generator.h"


/* Provided outside. */
#define BTGLX0_BASE (0x00000000)
#define BTGLX1_BASE (0xb800a000)
#define BTGLX2_BASE (0xb8018000)


#define BTG_LX1_ADDR_PHY       (0x02000000)
#define BTG_LX2_ADDR_PHY       (0x03000000)
#define BTG_SIZE_W             (0x01000000-1)
#define BTG_SIZE_R             (0x01000000-1-0x100)

btg_para_v3_t btg_fpga[] = {
{
        .gbase = BTGLX1_BASE + 0x100,
        .lxid  = 1,
        .feat  = BTG_WRITE,
        .burstlen = 1, /* 0: 16B; 1: 32B, 2: 64B, 3: 128B */
        .pktlen = 0x20,  /* from 32B ~ 8160B in a unit of 32B. */
        .resp_time = 0x80,  /* in cycles. */
        .perid_time = 0x80, /* in cycles. */
        .addr_base = BTG_LX1_ADDR_PHY,
        .addr_mask = BTG_SIZE_W,
        .addr_gap = 0x20,
    },
    {
        .gbase = BTGLX1_BASE + 0x200,
        .lxid  = 1,
        .feat  = BTG_READ,
        .rchkmode = 1, /* 0: NONE; 1: data gen.; 2: checksum; 3: undefined. */
        .burstlen = 1, /* 0: 16B; 1: 32B, 2: 64B, 3: 128B */
        .pktlen = 0x20,  /* from 32B ~ 8160B in a unit of 32B. */
        .resp_time = 0x80,  /* in cycles. */
        .perid_time = 0x80, /* in cycles. */
        .addr_base = BTG_LX1_ADDR_PHY,
        .addr_mask = BTG_SIZE_R,
        .addr_gap = 0x20,
    },
    {
        .gbase = BTGLX2_BASE + 0x100,
        .lxid  = 2,
        .feat  = BTG_WRITE,
        .burstlen = 1, /* 0: 16B; 1: 32B, 2: 64B, 3: 128B */
        .pktlen = 0x20,  /* from 32B ~ 8160B in a unit of 32B. */
        .resp_time = 0x80,  /* in cycles. */
        .perid_time = 0x80, /* in cycles. */
        .addr_base = BTG_LX2_ADDR_PHY,
        .addr_mask = BTG_SIZE_W,
        .addr_gap = 0x20,
        },
    {
        .gbase = BTGLX2_BASE + 0x200,
        .lxid  = 2,
        .feat  = BTG_READ,
        .rchkmode = 1, /* 0: NONE; 1: data gen.; 2: checksum; 3: undefined. */
        .burstlen = 1, /* 0: 16B; 1: 32B, 2: 64B, 3: 128B */
        .pktlen = 0x20,  /* from 32B ~ 8160B in a unit of 32B. */
        .resp_time = 0x80,  /* in cycles. */
        .perid_time = 0x80, /* in cycles. */
        .addr_base = BTG_LX2_ADDR_PHY,
        .addr_mask = BTG_SIZE_R,
        .addr_gap = 0x20,
    },
    {
        /* Sentinal entry guards end of list. */
        .gbase = 0xffff,
    }
};

#define ISBTGAVAL(x) ((x->gbase & 0xb8000000) == 0xb8000000)
#define ISBTGEND(x)  (x->gbase == 0xffff)


void lx1_btg_start(void)
{
    btg_para_v3_t *btg_entry = &btg_fpga[0];
    _btg_start(btg_entry);
    btg_entry++;
    _btg_start(btg_entry);
}

void lx2_btg_start(void)
{
    btg_para_v3_t *btg_entry = &btg_fpga[2];
    while (!ISBTGEND(btg_entry)) {
        _btg_start(btg_entry);
                btg_entry++;
    }
}

void lx1_btg_stop(void)
{
    btg_para_v3_t *btg_entry = &btg_fpga[0];
    _btg_stop(btg_entry);
    btg_entry++;
    _btg_stop(btg_entry);
}

void lx2_btg_stop(void)
{
    btg_para_v3_t *btg_entry = &btg_fpga[2];
    while (!ISBTGEND(btg_entry)) {
        _btg_stop(btg_entry);
                btg_entry++;
    }
}

extern int dram_test (int flag, int argc, char *argv[]);
extern int _dram_test(u32_t dram_start, u32_t dram_size, u32_t area_size);
int concur_btg_test (int flag, int argc, char *argv[])
{
    u32_t bst_len,i;
    for(bst_len=0; bst_len<=3; bst_len++){
        for(i=0 ; i<4 ; i++){
            btg_fpga[i].burstlen = bst_len;
        }

        printf("\n\n [Uboot] BTG Initial Status\n");
        btg_controller_init(&btg_fpga[0]);

        btg_status(&btg_fpga[0]);

        printf(" [Uboot] Enable BTG ...................\n");
        lx1_btg_start();
        lx2_btg_start();

        //_con_dram_test();
        dram_test(0,0,0);
        flash_test(0,0,0);
        
        u32_t ram_addr_v   = 0xBF004000;
        u32_t size_per_pat = 8*1024;
        u32_t test_size    = 0x2000;
        _dram_test(ram_addr_v, size_per_pat, test_size);

        btg_status(&btg_fpga[0]);
        lx1_btg_stop();
        lx2_btg_stop();
    }

    printf("\n\n [Uboot] Finish bus_traffic_generator.c\n");
    return 1;
}


