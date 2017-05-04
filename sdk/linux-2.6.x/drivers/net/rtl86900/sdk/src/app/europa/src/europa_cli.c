/*
 * Copyright (C) 2012 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: 40647 $
 * $Date: 2013-07-01 15:36:16 +0800 (?±ä?, 01 ä¸ƒæ? 2013) $
 *
 * Purpose : Main function of the EPON OAM protocol stack user application
 *           It create two additional threads for packet Rx and state control
 *
 * Feature : Start point of Europa. Use individual threads
 *           for packet Rx and state control
 *
 */

/*
 * Include Files
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>

//#include "epon_oam_config.h"
//#include "epon_oam_err.h"
//#include "epon_oam_msgq.h"
#include "europa_cli.h"

/* 
 * Symbol Definition 
 */
#define EUROPA_CLI_BRANCH     0x01
#define EUROPA_CLI_LEAF       0x02
#define EUROPA_CLI_END        0x03

#define EUROPA_PARAMETER_SIZE 0x1000
#define EUROPA_BLOCK_SIZE          0x100
#define EUROPA_FILE_SIZE 0x1000
#define EUROPA_FILE_LOCATION	"/var/config/europa.data"

#define EUROPA_STRING_SIZE 256

typedef struct europa_cli_tree_s {
    unsigned char cmd[16];
    unsigned char type;
    union {
        struct europa_cli_tree_s *pSubTree;
        int (*cli_proc)(int argc, char *argv[], FILE *fp);
    } u;
} europa_cli_tree_t;


static unsigned char default_a0_reg[0x100] = {
0x02, 0x04, 0x0B, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x03, 0x0C, 0x00, 0x14, 0xC8,
0x00, 0x00, 0x00, 0x00, 0x52, 0x45, 0x41, 0x4C, 0x54, 0x45, 0x4B, 0x20, 0x20, 0x20, 0x20, 0x20,
0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x52, 0x54, 0x4C, 0x38, 0x32, 0x39, 0x30, 0x20,
0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x30, 0x30, 0x30, 0x31, 0x05, 0x1E, 0x00, 0xFF, 
0x00, 0x06, 0x00, 0x00, 0x76, 0x65, 0x6E, 0x64, 0x6F, 0x72, 0x70, 0x61, 0x72, 0x74, 0x6E, 0x75,
0x6D, 0x62, 0x65, 0x72, 0x32, 0x30, 0x31, 0x34, 0x30, 0x31, 0x32, 0x33, 0x68, 0x80, 0x02, 0xFF, 
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

static unsigned char default_a2_reg[0x100]={
0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF,/* 0x94, 0x70,*/0x8e, 0x94, 0x6D, 0x60, 0x8C, 0xA0, 0x75, 0x30, 
0x75, 0x30, 0x05, 0xDC, 0x61, 0xA8, 0x07, 0xD0, 0x00, 0x00, 0x0F, 0x8D, 0x00, 0x0A, 0x0C, 0x5A, 
0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 
0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

static unsigned char default_a4_reg[] = {
0x7d,
0x72, 0x89, 0xe1, 0xe2, 0x8e, 0x32, 0x9b, 0x90, 0x00, 0x49, 0x9f, 0xff, 0x23, 0x04, 0x78, 0x7f, 
0xff, 0x00, 0x02, 0x05, 0x00, 0x00, 0x01, 0xf6, 0xce, 0x90, 0xc0, 0x00, 0x00, 0x38, 0x24, 0x40, 
0x40, 0x00, 0x01, 0x13, 0xbc, 0x1e, 0x33, 0x10, 0xfb, 0x18, 0xe4, 0x80, 0xe0, 0x01, 0xb0, 0x44, 
0x40, 0xac, 0x03, 0x00, 0xf4, 0x00, 0x20, 0x40, 0x00, 0x7c, 0x20, 0x40, 0xfc, 0x30, 0x10, 0x00, 
0x00, 0x00, 0x00, 0x00, 0xff, 0xfc, 0x12, 0x0a, 0x8b, 0x04, 0x01, 0x33, 0x74, 0xa5, 0xa0, 0xe0, 
0x00, 0x00, 0x00, 0x00, 0x81, 0x30, 0x00, 0x15, 0x00, 0xff, 0x1f, 0x01, 0x00, 0xff, 0x00, 0x32, 
0xff, 0x01, 0xff, 0xff, 0x05, 0x00, 0x60, 0x80, 0x00, 0x08, 0x10, 0xa0, 0xec, 0xb0, 0xbf, 0xfe, 
0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x10, 0x00, 0x00, 0x00, 0xff, 0xa0, 0xc0
};

static unsigned char default_apd_lut[] = {
0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B,
0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C,
0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D,
0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E,
0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F,
0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21,
0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,
0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23,
0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24 
};

static unsigned char default_laser_lut[] = {
0x0A, 0x20, 0x0A, 0x20, 0x0A, 0x20, 0x0A, 0x20, 0x0A, 0x20, 0x0A, 0x20, 0x0A, 0x20, 0x0A, 0x20,
0x0A, 0x20, 0x0A, 0x20, 0x0C, 0x20, 0x0C, 0x20, 0x0C, 0x20, 0x0C, 0x20, 0x0C, 0x20, 0x0C, 0x20,
0x0C, 0x20, 0x0C, 0x20, 0x0C, 0x20, 0x0C, 0x20, 0x0E, 0x20, 0x0E, 0x20, 0x0E, 0x20, 0x0E, 0x20,
0x0E, 0x20, 0x0E, 0x20, 0x0E, 0x20, 0x0E, 0x20, 0x0E, 0x20, 0x0E, 0x20, 0x10, 0x20, 0x10, 0x20,
0x10, 0x20, 0x10, 0x20, 0x10, 0x20, 0x10, 0x20, 0x10, 0x20, 0x10, 0x20, 0x10, 0x20, 0x10, 0x20,
0x12, 0x20, 0x12, 0x20, 0x12, 0x20, 0x12, 0x20, 0x12, 0x20, 0x12, 0x20, 0x12, 0x20, 0x12, 0x20,
0x12, 0x20, 0x12, 0x20, 0x14, 0x32, 0x14, 0x32, 0x14, 0x32, 0x14, 0x32, 0x14, 0x32, 0x14, 0x32,
0x14, 0x32, 0x14, 0x32, 0x14, 0x32, 0x14, 0x32, 0x18, 0x34, 0x18, 0x34, 0x18, 0x34, 0x18, 0x34,
0x18, 0x34, 0x18, 0x34, 0x18, 0x34, 0x18, 0x34, 0x18, 0x34, 0x18, 0x34, 0x1C, 0x38, 0x1C, 0x38,
0x1C, 0x38, 0x1C, 0x38, 0x1C, 0x38, 0x1C, 0x38, 0x1C, 0x38, 0x1C, 0x38, 0x1C, 0x38, 0x1C, 0x38,
0x24, 0x40, 0x24, 0x40, 0x24, 0x40, 0x24, 0x40, 0x24, 0x40, 0x24, 0x40, 0x24, 0x40, 0x24, 0x40,
0x24, 0x40, 0x24, 0x40, 0x2C, 0x4C, 0x2C, 0x4C, 0x2C, 0x4C, 0x2C, 0x4C, 0x2C, 0x4C, 0x2C, 0x4C,
0x2C, 0x4C, 0x2C, 0x4C, 0x2C, 0x4C, 0x2C, 0x4C, 0x30, 0x54, 0x30, 0x54, 0x30, 0x54, 0x30, 0x54,
0x30, 0x54, 0x30, 0x54, 0x30, 0x54, 0x30, 0x54, 0x30, 0x54, 0x30, 0x54, 0x40, 0x5C, 0x40, 0x5C,
0x40, 0x5C, 0x40, 0x5C, 0x40, 0x5C, 0x40, 0x5C, 0x40, 0x5C, 0x40, 0x5C, 0x40, 0x5C, 0x40, 0x5C,
0x50, 0x60, 0x50, 0x60, 0x50, 0x60, 0x50, 0x60, 0x50, 0x60, 0x70, 0x68, 0x70, 0x68, 0x70, 0x68,
0x70, 0x68, 0x70, 0x68, 0x88, 0x6C, 0x88, 0x6C, 0x88, 0x6C, 0x88, 0x6C, 0x88, 0x6C, 0x88, 0x6C,
0x88, 0x6C, 0x88, 0x6C, 0x88, 0x6C, 0x88, 0x6C, 0x88, 0x6C, 0x88, 0x6C, 0x88, 0x6C, 0x88, 0x6C,
0x88, 0x6C, 0x88, 0x6C, 0x88, 0x6C, 0x88, 0x6C, 0x88, 0x6C, 0x88, 0x6C, 0x88, 0x6C
};

//P0, P1, Paverge, and others
static unsigned char default_param[]= {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x26, 0x50, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00
};

/*  
 * Data Declaration  
 */
//static int msgQId;
static europa_cli_tree_t cliRootOpen[] = {
    {
        "default",
        EUROPA_CLI_LEAF,
        { .cli_proc = europa_cli_param_init }
    },
    {
        "empty",
        EUROPA_CLI_LEAF,
        { .cli_proc = europa_cli_param_open }
    },
    {
        "",
        EUROPA_CLI_END,
        { NULL }
    }
};

static europa_cli_tree_t cliRootGet[] = {
    {
        "param",
        EUROPA_CLI_LEAF,
        { .cli_proc = europa_cli_param_get }
    },
    {
        "all",
        EUROPA_CLI_LEAF,
        { .cli_proc = europa_cli_all_get }
    },       
    {
        "ddmia0",
        EUROPA_CLI_LEAF,
        { .cli_proc = europa_cli_ddmi_a0_get }
    },    
    {
        "ddmia2",
        EUROPA_CLI_LEAF,
        { .cli_proc = europa_cli_ddmi_a2_get }
    },
    {
        "a4",
        EUROPA_CLI_LEAF,
        { .cli_proc = europa_cli_a4_get }
    },    
    {
        "apdlut",
        EUROPA_CLI_LEAF,
        { .cli_proc = europa_cli_apd_lut_get }
    },
    {
        "laserlut",
        EUROPA_CLI_LEAF,
        { .cli_proc = europa_cli_laser_lut_get }
    },    
    {
        "p0p1",
        EUROPA_CLI_LEAF,
        { .cli_proc = europa_cli_P0P1_get }
    },    
    {
        "vn", /* A0/20~35 Vendor Name */
        EUROPA_CLI_LEAF,
        { .cli_proc = europa_cli_VendorName_get }
    },         
    {
        "pn", /* A0/40~55 Vendor Part Number */
        EUROPA_CLI_LEAF,
        { .cli_proc = europa_cli_VendorPN_get }
    },         
    {
        "rev", /* A0/56~59 Vendor Rev */
        EUROPA_CLI_LEAF,
        { .cli_proc = europa_cli_VendorRev_get }
    },         
    {
        "",
        EUROPA_CLI_END,
        { NULL }
    }
};

static europa_cli_tree_t cliRootSet[] = {
    {
        "param",
        EUROPA_CLI_LEAF,
        { .cli_proc = europa_cli_param_set }
    },
    {
        "long",
        EUROPA_CLI_LEAF,
        { .cli_proc = europa_cli_long_set }
    },    
    {
        "ddmia0",
        EUROPA_CLI_LEAF,
        { .cli_proc = europa_cli_ddmi_a0_set }
    },    
    {
        "ddmia2",
        EUROPA_CLI_LEAF,
        { .cli_proc = europa_cli_ddmi_a2_set }
    },
    {
        "a4",
        EUROPA_CLI_LEAF,
        { .cli_proc = europa_cli_a4_set }
    },    
    {
        "apdlut",
        EUROPA_CLI_LEAF,
        { .cli_proc = europa_cli_apd_lut_set }
    },
    {
        "laserlut",
        EUROPA_CLI_LEAF,
        { .cli_proc = europa_cli_laser_lut_set }
    },    
    {
        "p0",
        EUROPA_CLI_LEAF,
        { .cli_proc = europa_cli_P0_set }
    },       
    {
        "p1",
        EUROPA_CLI_LEAF,
        { .cli_proc = europa_cli_P1_set }
    },         
    {
        "vn", /* A0/20~35 Vendor Name */
        EUROPA_CLI_LEAF,
        { .cli_proc = europa_cli_VendorName_set }
    },         
    {
        "pn", /* A0/40~55 Vendor Part Number */
        EUROPA_CLI_LEAF,
        { .cli_proc = europa_cli_VendorPN_set }
    },         
    {
        "rev", /* A0/56~59 Vendor Rev */
        EUROPA_CLI_LEAF,
        { .cli_proc = europa_cli_VendorRev_set }
    },         
    {
        "",
        EUROPA_CLI_END,
        { NULL }
    }
};

static europa_cli_tree_t cliRoot[] = {
    {
        "open",
        EUROPA_CLI_BRANCH,
        {cliRootOpen }
    },
     {
        "set",
        EUROPA_CLI_BRANCH,
        { cliRootSet }
    },   	
    {
        "get",
        EUROPA_CLI_BRANCH,
        { cliRootGet }
    },
    {
        "clear",
        EUROPA_CLI_LEAF,
        { .cli_proc = europa_cli_param_clear }
    },
    {
        "close",
        EUROPA_CLI_LEAF,
        { .cli_proc = europa_cli_param_close }
    },    
    {
        "delete",
        EUROPA_CLI_LEAF,
        { .cli_proc = europa_cli_delete }
    },    
    {
        "",
        EUROPA_CLI_END,
        { NULL }
    }       
};


/* 
 * Macro Definition 
 */

/*  
 * Function Declaration  
 */

/* ------------------------------------------------------------------------- */
/* Internal APIs */

static int _europa_param_init()
{
    FILE *fp;
    int length, start_addr, file_len;
    unsigned int input;
    unsigned int i;
           
    fp = fopen(EUROPA_FILE_LOCATION, "wb");  

    if (NULL ==fp)
    {
        printf("Create file in /var/config/ error!!!!!!!\n"); 
        return -1;           
    } 
	
    fseek(fp, 0, SEEK_SET); 
    fwrite(default_a0_reg, sizeof(unsigned char), sizeof(default_a0_reg),fp);
    fwrite(default_a2_reg, sizeof(unsigned char), sizeof(default_a2_reg),fp);
    fwrite(default_a4_reg, sizeof(unsigned char), sizeof(default_a4_reg),fp);	
    fseek(fp, 0x300, SEEK_SET); 
    fwrite(default_apd_lut, sizeof(unsigned char), sizeof(default_apd_lut),fp);	
    fseek(fp, 0x400, SEEK_SET); 	
    fwrite(default_laser_lut, sizeof(unsigned char), sizeof(default_laser_lut),fp);	
    fwrite(default_param, sizeof(unsigned char), sizeof(default_param),fp);		
    fclose(fp);
    printf("Create File Success!!!\n"); 

    return 0;    
}


static int europa_cli_delete(
    int argc,
    char *argv[],
    FILE *fp)
{
    char *ptr;
    unsigned int i; 

    printf("europa_cli_delete!!!!!!!\n");	

    unlink(EUROPA_FILE_LOCATION);  

    return 0;
}

static int europa_cli_param_init(
    int argc,
    char *argv[],
    FILE *fp)
{
    char *ptr; 
    unsigned int i; 

    fp = fopen(EUROPA_FILE_LOCATION,"rb");  
	
    if (NULL ==fp)
    {
        printf("europa.data does not exist. Create a new file with initial value.\n");       
        _europa_param_init(); 
    }	
    else
    {
        printf("File in /var/config/ is already exist!!!!!!!\n"); 

    }
	
    return 0;
}

static int europa_cli_param_open(
    int argc,
    char *argv[],
    FILE *fp)
{
    char *ptr; 
    unsigned int i; 

    fp = fopen(EUROPA_FILE_LOCATION,"rb");  

    if (NULL ==fp)
    {
        printf("europa.data does not exist. Create an empty file.\n");       
        fp = fopen(EUROPA_FILE_LOCATION,"wb");
        if (NULL ==fp)
        {
            printf("Create file in /var/config/ error!!!!!!!\n"); 
        return -1;           
    }	
        fclose(fp);			
    }	
    else
    {
        printf("File in /var/config/ is already exist!!!!!!!\n");  

    }

    return 0;
}

static int europa_cli_param_close(
    int argc,
    char *argv[],
    FILE *fp)
{
    
#if 0   
    fp = fopen("/var/config/europa.data","wb");
        
    if (NULL ==fp)
    {
        printf("Open file in /var/config/ error!!!!!!!\n"); 
        return -1;           
    }	    
    
    printf("europa_cli_param_close!!!!!!!\n");	
	
    fclose(fp);
#endif
    return 0;
}

static int europa_cli_param_clear(
    int argc,
    char *argv[],
    FILE *fp)
{
    //int length, start_addr;	
    //char *ptr; 
       	
    printf("europa_cli_param_clear!!!!!!!\n");	
	
    fp = fopen(EUROPA_FILE_LOCATION, "wb");
	
    if (NULL ==fp)
    {
        printf("Open file in /var/config/ error!!!!!!!\n"); 
        return -1;           
    }		
#if 0	
    if(argc >= 3)
    {
        start_addr = atoi(argv[1]); 
        length = atoi(argv[2]);
        
        printf("europa_cli_param_clear: length = %d, start address = %d\n", length, start_addr);       
              
        if ((length < 0) || (start_addr < 0))
        {
        	  printf("europa_cli_param_clear: length  or start address error!!!\n"); 
            return -1;
        }
        
        /* set the file pointer to start address */
        fseek(fp, start_addr, SEEK_SET); 
        ptr = malloc(length);
        
        /*Set all to 0*/
        memset(ptr, 0x0, sizeof(char)*length);
        fwrite(ptr,sizeof(char), length, fp);
    
        free(ptr);      

    }
    else if (argc<3)
    {
        printf("%s <start address> <length> \n", argv[0]);
        return -1;
    }  	
#endif

    fclose(fp);
        
    printf("Clear Success!!!\n"); 

    return 0;
}

static int _europa_load_data(
    unsigned char *ptr,
    FILE *fp,
    int *file_len)
{
    int i, output;

    fp = fopen(EUROPA_FILE_LOCATION, "rb");  
    if (NULL ==fp)
    {
        printf("Open file in /var/config/ error!!!!!!!\n"); 
        return -1;           
    }        
    fseek(fp, 0, SEEK_SET);  
    memset(ptr, 0x0, sizeof(char)*EUROPA_PARAMETER_SIZE);  
    for(i = 0; i < EUROPA_PARAMETER_SIZE; i++)
    {
        output = fgetc(fp);
        *file_len = i;        	        
        if (output == EOF)
        {
            printf("Original file length = %d\n", *file_len);
            break;	
        }
        else
        {
            *ptr = (unsigned char)(output&0xff);
            //printf("temp[%d] = 0x%x\n", i, *ptr);
        }
        ptr++;       
    }        
#if 0                     
        for(i = 0; i < EUROPA_PARAMETER_SIZE; i++)
        {
            file_len = i;
            if (feof(fp))
            {
                printf("file length = %d\n", i);
                break;  
            }
            else
            {        	
                input=fgetc(fp);
                temp[i] = input&0xff;
                printf("temp[%d] = %x\n", i, temp[i]);
            }
        } 
            
#endif        
        fclose(fp); 
	
}

static int europa_cli_param_set(
    int argc,
    char *argv[],
    FILE *fp)
{
    //FILE *fp;
    int length, start_addr, file_len;
    unsigned int input;
    unsigned int i;
    unsigned char temp[EUROPA_PARAMETER_SIZE];
    if(argc >= 3)
    {
        start_addr = atoi(argv[1]);
        length = atoi(argv[2]); 
        
        printf("europa_cli_param_set: start address = %d length = %d\n", start_addr, length);       
              
        if ((length < 0) || (start_addr < 0))
        {
        	  printf("europa_cli_param_set: length  or start address error!!!\n"); 
            return -1;
        }
        if ((length+start_addr)>EUROPA_PARAMETER_SIZE)
        {
        	  printf("europa_cli_param_set: full size should less than 4KB!!!\n"); 
            return -1;
        }        

        _europa_load_data(temp,fp,&file_len);  
           
        fp = fopen(EUROPA_FILE_LOCATION, "wb");  

        if (NULL ==fp)
        {
            printf("Open file in /var/config/ error!!!!!!!\n"); 
            return -1;           
        } 
        
        printf("Please input europa parameters with length %d:\n", length); 

        for(i = start_addr; i < (length+start_addr); i++)
        {
        	 scanf("%x",&input);
        	 temp[i] = input&0xFF;
        }
 
        if (file_len<(length+start_addr))
            file_len = length+start_addr;
 
        fseek(fp, 0, SEEK_SET); 
        fwrite(temp,sizeof(char), file_len,fp);
 
        fclose(fp);
        printf("Input Success!!!\n"); 

    }
    else if (argc<3)
    {
        printf("%s <start address> <length> \n", argv[0]);
        return -1;
    }  

    return 0;    
}

static int europa_cli_long_set(
    int argc,
    char *argv[],
    FILE *fp)
{
    //FILE *fp;
    int start_addr, file_len, length;
    unsigned int input;
    unsigned int i;
    unsigned char temp[EUROPA_PARAMETER_SIZE];
    if(argc >= 2)
    {
        start_addr = atoi(argv[1]);
        
        printf("europa_cli_long_set: start address = %d \n", start_addr);       
              
        if ((start_addr < 0) ||(start_addr>EUROPA_PARAMETER_SIZE))
        {
        	  printf("europa_cli_long_set: start address error!!!\n"); 
            return -1;
        }
#if 0        
        fp = fopen("/var/config/europa.data","rb");  
        if (NULL ==fp)
        {
            printf("Open file in /var/config/ error!!!!!!!\n"); 
            return -1;           
        }        
        fseek(fp, 0, SEEK_SET);  
        memset(temp, 0x0, sizeof(char)*EUROPA_PARAMETER_SIZE);  
        for(i = 0; i < EUROPA_PARAMETER_SIZE; i++)
        {
            input = fgetc(fp);
            file_len = i;        	        
            if (input == EOF)
            {
                printf("file length = %d\n", file_len);
                break;	
            }
            else
            {
                temp[i] = input&0xff;
                printf("temp[%d] = 0x%x\n", i, temp[i]);
            }        
        }        
#if 0                     
        for(i = 0; i < EUROPA_PARAMETER_SIZE; i++)
        {
            file_len = i;
            if (feof(fp))
            {
               printf("file length = %d\n", i);
               break;  
            }	        	 
            else
            {        	
                input=fgetc(fp);
                temp[i] = input&0xff;
                printf("temp[%d] = %x\n", i, temp[i]);
            }
        }          
            
#endif        
        fclose(fp);           
#endif

        _europa_load_data(temp,fp,&file_len);  
        
        fp = fopen(EUROPA_FILE_LOCATION, "wb");  

        if (NULL ==fp)
        {
            printf("Open file in /var/config/ error!!!!!!!\n"); 
            return -1;           
        }
                
        printf("The maximum file size is 4096. If you want to end the data inupt , please key in abcd.  \n");              
        printf("Please insert data:\n");       
       
        length = 0;
        for(i = start_addr; i < EUROPA_PARAMETER_SIZE; i++)
        {
            scanf("%x",&input);
            if (0xabcd != input)
            { 
                length++;           	
                temp[i] = input&0xFF;
            }
            else
            {
        	 	    break;	 	
            }
        }
        printf("Input europa parameters with length %d:\n", length);         
 
        if (file_len<(length+start_addr))
            file_len = length+start_addr;
 
        fseek(fp, 0, SEEK_SET); 
        fwrite(temp,sizeof(char), file_len,fp);
 
        fclose(fp);
        printf("europa long update success!!!\n"); 

    }
    else if (argc<3)
    {
        printf("%s <start address> \n", argv[0]);
        return -1;
    }  

    return 0;    
}

static int _europa_cli_block_set( FILE *fp, int start_addr, unsigned int size)
{
    int file_len, length;
    unsigned int input;
    unsigned int i;
    unsigned char temp[EUROPA_PARAMETER_SIZE];
    
    _europa_load_data(temp,fp,&file_len);  
           
    fp = fopen(EUROPA_FILE_LOCATION, "wb");  

    if (NULL ==fp)
    {
        printf("Open file in /var/config/ error!!!!!!!\n"); 
        return -1;           
    } 
        
    printf("The maximum file size is %d. If you want to end the data inupt less than maximum , please key in abcd.  \n", size);              
    printf("Please insert data:\n");       
       
    length = 0;
    for(i = start_addr; i < (start_addr + size); i++)
    {
        scanf("%x",&input);
        if (0xabcd != input)
        { 
            length++;               
            temp[i] = input&0xFF;
        }
        else
        {
            break;         
        }
    }      
    printf("Input Europa  data with length %d:\n", length);         
 
    if (file_len<(length+start_addr))
    {
        file_len = length+start_addr;
    }
    fseek(fp, 0, SEEK_SET); 
    fwrite(temp,sizeof(char), file_len,fp);
 
    fclose(fp);
        
    return 0;    
}


/* 
   set ASCII string type parameters in A0
   eg. A0/20~35 : Vendor Name
       A0/40~55 : Vendor Part Name
       A0/56~59 : Vendor Rev */
static int _europa_cli_string_set(FILE *fp, 
		                  int start_addr, 
				  unsigned int size,
				  char *str)
{
    int ret;

    printf("%s( %d, %d, \"%s\" )\n", __FUNCTION__, start_addr, size, str);


    fp = fopen(EUROPA_FILE_LOCATION, "rb+");  
    if (NULL ==fp)
    {
        printf("Open file in /var/config/ error!!!!!!!\n"); 
        return -1;           
    } 
 
    fseek(fp, start_addr, SEEK_SET); 
    ret = fwrite((void*)str, sizeof(char), size, fp);

    fclose(fp);
        
    return 0;    
}

/* 
   get ASCII string type parameters in A0
       A0/56~59 : Vendor Rev */
static int _europa_cli_string_get(FILE *fp, 
		                  int start_addr, 
				  unsigned int size,
				  char *str)
{
    fp = fopen(EUROPA_FILE_LOCATION, "rb");  
    if (NULL ==fp)
    {
        printf("Open file in /var/config/ error!!!!!!!\n"); 
        return -1;           
    } 
 
    fseek(fp, start_addr, SEEK_SET); 
    fread((void*)str, sizeof(char), size, fp);
 
    fclose(fp);
        
    return 0;    
}


static int europa_cli_ddmi_a0_set(
    int argc,
    char *argv[],
    FILE *fp)
{
    int start_addr, file_len, length;
    unsigned int input;
    unsigned int i;
    unsigned char temp[EUROPA_PARAMETER_SIZE];

    start_addr = 0;
        
    printf("europa_cli_ddmi_a0_set: start address = %d \n", start_addr);       
              
     _europa_cli_block_set( fp, start_addr, EUROPA_BLOCK_SIZE);
     
    printf("europa DDMI A0 update success!!!\n");  

    return 0;    
}

static int europa_cli_ddmi_a2_set(
    int argc,
    char *argv[],
    FILE *fp)
{
    int start_addr, file_len, length;
    unsigned int input;
    unsigned int i;
    unsigned char temp[EUROPA_PARAMETER_SIZE];

    start_addr = 0x100;
        
    printf("europa_cli_ddmi_a2_set: start address = %d \n", start_addr);       
              
     _europa_cli_block_set( fp, start_addr, EUROPA_BLOCK_SIZE);
     
    printf("europa DDMI A2 update success!!!\n");  

    return 0;    
}

static int europa_cli_a4_set(
    int argc,
    char *argv[],
    FILE *fp)
{
    int start_addr, file_len, length;
    unsigned int input;
    unsigned int i;
    unsigned char temp[EUROPA_PARAMETER_SIZE];

    start_addr = 0x200;
        
    printf("europa_cli_ddmi_a4_set: start address = %d \n", start_addr);       
              
     _europa_cli_block_set( fp, start_addr, EUROPA_BLOCK_SIZE);

    printf("europa DDMI A4 update success!!!\n");  

    return 0;    
}

static int europa_cli_apd_lut_set(
    int argc,
    char *argv[],
    FILE *fp)
{
    int start_addr, file_len, length;
    unsigned int input;
    unsigned int i;
    unsigned char temp[EUROPA_PARAMETER_SIZE];

    start_addr = 0x300;

     _europa_cli_block_set( fp, start_addr, EUROPA_BLOCK_SIZE);
        
    printf("europa APD lookup table update success!!!\n");  

    return 0;    
}

static int europa_cli_laser_lut_set(
    int argc,
    char *argv[],
    FILE *fp)
{
    int start_addr, file_len, length;
    unsigned int input;
    unsigned int i;
    unsigned char temp[EUROPA_PARAMETER_SIZE];

    start_addr = 0x400;
        
    printf("europa_cli_laser_lut_set: start address = %d \n", start_addr);       
              
     _europa_cli_block_set( fp, start_addr, 304);
     
    printf("europa Laser lookup table update success!!!\n");  

    return 0;    
}

static int europa_cli_P0_set(
    int argc,
    char *argv[],
    FILE *fp)
{
    int start_addr, file_len, length;
    unsigned int input;
    unsigned int i;
    unsigned char temp[EUROPA_PARAMETER_SIZE];

    start_addr = 1346;
        
    printf("europa_cli_P0_set: start address = %d \n", start_addr);       
              
     _europa_cli_block_set( fp, start_addr, 1);
     
    printf("europa P0 update success!!!\n");  

    return 0;    
}

static int europa_cli_P1_set(
    int argc,
    char *argv[],
    FILE *fp)
{
    int start_addr, file_len, length;
    unsigned int input;
    unsigned int i;
    unsigned char temp[EUROPA_PARAMETER_SIZE];

    start_addr = 1347;
        
    printf("europa_cli_P1_set: start address = %d \n", start_addr);       
              
     _europa_cli_block_set( fp, start_addr, 1);
     
    printf("europa P1 update success!!!\n");  

    return 0;    
}

static int europa_cli_VendorName_set(
    int argc,
    char *argv[],
    FILE *fp)
{
#define VENDOR_NAME_SZ 16
    int start_addr, length;
    char buf[VENDOR_NAME_SZ+1];
    char temp[EUROPA_STRING_SIZE];


    start_addr = 20;

    memset(buf, ' ', sizeof(buf));
    buf[VENDOR_NAME_SZ] = '\0';

    if (1==argc)
    {
       printf("Vendor Name : ");
       scanf("%s", temp);
    }
    else
    {
       strcpy(temp, argv[1]);
    }

    length = (strlen(temp)>=VENDOR_NAME_SZ) ? VENDOR_NAME_SZ : strlen(temp);
    memcpy(buf, temp, length);
        
    printf("%s: start address = %d \n", __FUNCTION__, start_addr);       
              
    _europa_cli_string_set( fp, start_addr, VENDOR_NAME_SZ, buf);
     
    printf("europa VendorName update success!!!\n");  

    return 0;    
#undef VENDOR_NAME_SZ
}

static int europa_cli_VendorPN_set(
    int argc,
    char *argv[],
    FILE *fp)
{
#define VENDOR_PN_SZ 16
    int start_addr, length;
    unsigned char buf[VENDOR_PN_SZ+1];
    char temp[EUROPA_STRING_SIZE];

    start_addr = 40;

    memset(buf, ' ', sizeof(buf));
    buf[VENDOR_PN_SZ] = '\0';

    if (1==argc)
    {
       printf("Part Number: ");
       scanf("%s", temp);
    }
    else
    {
       strcpy(temp, argv[1]);
    }

    length = (strlen(temp)>=VENDOR_PN_SZ) ? VENDOR_PN_SZ : strlen(temp);
    memcpy(buf, temp, length);
        
    printf("%s: start address = %d \n", __FUNCTION__, start_addr);       
              
    _europa_cli_string_set( fp, start_addr, VENDOR_PN_SZ, buf);
     
    printf("europa VendorPartNumber update success!!!\n");  

    return 0;    
#undef VENDOR_PN_SZ
}

static int europa_cli_VendorRev_set(
    int argc,
    char *argv[],
    FILE *fp)
{
#define VENDOR_REV_SZ 4
    int start_addr, length;
    unsigned char buf[VENDOR_REV_SZ+1];
    char temp[EUROPA_STRING_SIZE];

    start_addr = 56;

    memset(buf, ' ', sizeof(buf));
    buf[VENDOR_REV_SZ] = '\0';

    if (1==argc)
    {
       printf("Rev: ");
       scanf("%s", temp);
    }
    else
    {
       strcpy(temp, argv[1]);
    }
        
    length = (strlen(temp)>=VENDOR_REV_SZ) ? VENDOR_REV_SZ : strlen(temp);
    memcpy(buf, temp, length);

    printf("%s: start address = %d \n", __FUNCTION__, start_addr);       
              
    _europa_cli_string_set( fp, start_addr, VENDOR_REV_SZ, buf);
     
    printf("europa VendorRev update success!!!\n");  

    return 0;    
#undef VENDOR_REV_SZ
}



static int europa_cli_all_get(
    int argc,
    char *argv[],
    FILE *fp)
{
    //FILE *fp;
    int length, start_addr;
    int output,i;
  
    if(argc >= 1)
    {  
        printf("europa_cli_all_get:\n");       
                 
        fp = fopen(EUROPA_FILE_LOCATION, "rb");  

        if (NULL ==fp)
        {
            printf("Open file in /var/config/ error!!!!!!!\n"); 
            return -1;           
        }
        
        /* set the file pointer to start address */
        fseek(fp, 0, SEEK_SET); 
        
        for(i=0;i<EUROPA_PARAMETER_SIZE;i++)
        {
        	  output = fgetc(fp);
            if (output == EOF)
            {
                break;	
            }
            else
            {
                printf("0x%02x ", output);
                if(i%8 == 7)
                    printf("   ");
                if(i%16 == 15)
                    printf("\n");                    
            }
        }
        printf("\n");    
        fclose(fp);       
        printf("\nOutput Success!!!\n"); 

    }

    return 0; 
}

static int europa_cli_param_get(
    int argc,
    char *argv[],
    FILE *fp)
{
    //FILE *fp;
    int length, start_addr;
    int output,i;
  
    if(argc >= 3)
    {
    	  start_addr = atoi(argv[1]); 
        length = atoi(argv[2]);
   
        printf("europa_cli_param_get: length = %d, start address = %d\n", length, start_addr);       
              
        if ((length < 0) || (start_addr < 0))
        {
        	  printf("europa_cli_param_get: length  or start address error!!!\n"); 
            return -1;
        }
        
        fp = fopen(EUROPA_FILE_LOCATION, "rb");  

        if (NULL ==fp)
        {
            printf("Open file in /var/config/ error!!!!!!!\n"); 
            return -1;           
        }
        
        /* set the file pointer to start address */
        fseek(fp, start_addr, SEEK_SET); 
        
        for(i=0;i<length;i++)
        {
        	  output = fgetc(fp);
            if (output == EOF)
            {
                break;	
            }
            else
            {
                printf("0x%02x ", output);
                if(i%8 == 7)
                    printf("   ");
                if(i%16 == 15)
                    printf("\n");                    
            }
        }

        fclose(fp);
        printf("\n");    
        printf("\nOutput Success!!!\n"); 

    }
    else if (argc<3)
    {
        printf("%s <start address> <length>\n", argv[0]);
        return -1;
    }  

    return 0; 
}

static int _europa_cli_block_get(FILE *fp, int start_addr, int length)
{
    int output,i;
                      
    fp = fopen(EUROPA_FILE_LOCATION, "rb");  

    if (NULL ==fp)
    {
        printf("Open file in /var/config/ error!!!!!!!\n"); 
        return -1;           
    }

    /* set the file pointer to start address */
    fseek(fp, start_addr, SEEK_SET); 
        
    for(i=0;i<length;i++)
    {
        output = fgetc(fp);
        if (output == EOF)
        {
            break;    
        }
        else
        {
            printf("0x%02x ", output);
            if(i%8 == 7)
                printf("   ");
            if(i%16 == 15)
                printf("\n");                    
        }
    }

    fclose(fp);
    printf("\n");    

    return 0; 
}


static int europa_cli_ddmi_a0_get(
    int argc,
    char *argv[],
    FILE *fp)
{
    int start_addr, file_len, length;
    unsigned int input;
    unsigned int i;
    unsigned char temp[EUROPA_PARAMETER_SIZE];

    start_addr = 0;
        
    printf("europa_cli_ddmi_a0_get: start address = %d \n", start_addr);       
              
     _europa_cli_block_get( fp, start_addr, EUROPA_BLOCK_SIZE);
     
    printf("europa DDMI A0 get success!!!\n");  

    return 0;    
}

static int europa_cli_ddmi_a2_get(
    int argc,
    char *argv[],
    FILE *fp)
{
    int start_addr, file_len, length;
    unsigned int input;
    unsigned int i;
    unsigned char temp[EUROPA_PARAMETER_SIZE];

    start_addr = 0x100;
        
    printf("europa_cli_ddmi_a2_get: start address = %d \n", start_addr);       
              
     _europa_cli_block_get( fp, start_addr, EUROPA_BLOCK_SIZE);
     
    printf("europa DDMI A2 get success!!!\n");  

    return 0;    
}

static int europa_cli_a4_get(
    int argc,
    char *argv[],
    FILE *fp)
{
    int start_addr, file_len, length;
    unsigned int input;
    unsigned int i;
    unsigned char temp[EUROPA_PARAMETER_SIZE];

    start_addr = 0x200;
        
    printf("europa_cli_ddmi_a4_get: start address = %d \n", start_addr);       
              
     _europa_cli_block_get( fp, start_addr, EUROPA_BLOCK_SIZE);

    printf("europa DDMI A4 get success!!!\n");  

    return 0;    
}

static int europa_cli_apd_lut_get(
    int argc,
    char *argv[],
    FILE *fp)
{
    int start_addr, file_len, length;
    unsigned int input;
    unsigned int i;
    unsigned char temp[EUROPA_PARAMETER_SIZE];

    start_addr = 0x300;
    
    printf("europa_cli_apd_lut_get: start address = %d \n", start_addr);     
    
     _europa_cli_block_get( fp, start_addr, EUROPA_BLOCK_SIZE);
        
    printf("europa APD lookup table get success!!!\n");  

    return 0;    
}

static int europa_cli_laser_lut_get(
    int argc,
    char *argv[],
    FILE *fp)
{
    int start_addr, file_len, length;
    unsigned int input;
    unsigned int i;
    unsigned char temp[EUROPA_PARAMETER_SIZE];

    start_addr = 0x400;
        
    printf("europa_cli_laser_lut_get: start address = %d \n", start_addr);       
              
     _europa_cli_block_get( fp, start_addr, 304);
     
    printf("europa Laser lookup table get success!!!\n");  

    return 0;    
}

static int europa_cli_P0P1_get(
    int argc,
    char *argv[],
    FILE *fp)
{
    int start_addr, file_len, length;
    unsigned int input;
    unsigned int i;
    unsigned char temp[EUROPA_PARAMETER_SIZE];

    start_addr = 1346;
        
    printf("europa_cli_P0P1_get: start address = %d \n", start_addr);       
              
     _europa_cli_block_get( fp, start_addr, 2);
     
    printf("europa P0 P1 get  success!!!\n");  

    return 0;    
}

static int europa_cli_VendorName_get(
    int argc,
    char *argv[],
    FILE *fp)
{
#define VENDOR_NAME_SZ 16
    int start_addr;
    char buf[VENDOR_NAME_SZ+1];

    start_addr = 20;

    _europa_cli_string_get( fp, start_addr, VENDOR_NAME_SZ, buf);
    buf[VENDOR_NAME_SZ]='\0';

    printf("%s: start address [%d] = \"%s\" \n", __FUNCTION__, start_addr, buf);
    return 0;    
#undef VENDOR_NAME_SZ
}

static int europa_cli_VendorPN_get(
    int argc,
    char *argv[],
    FILE *fp)
{
#define VENDOR_PN_SZ 16
    int start_addr;
    char buf[VENDOR_PN_SZ+1];

    start_addr = 40;

    _europa_cli_string_get( fp, start_addr, VENDOR_PN_SZ, buf);
    buf[VENDOR_PN_SZ]='\0';

    printf("%s: start address [%d] = \"%s\" \n", __FUNCTION__, start_addr, buf);
    return 0;    
#undef VENDOR_PN_SZ
}

static int europa_cli_VendorRev_get(
    int argc,
    char *argv[],
    FILE *fp)
{
#define VENDOR_REV_SZ 4
    int start_addr;
    char buf[VENDOR_REV_SZ+1];

    start_addr = 56;

    _europa_cli_string_get( fp, start_addr, VENDOR_REV_SZ, buf);
    buf[VENDOR_REV_SZ]='\0';

    printf("%s: start address [%d] = \"%s\" \n", __FUNCTION__, start_addr, buf);
    return 0;    

#undef VENDOR_REV_SZ
}



static int europa_cli_parse(
	int argc,
	char *argv[],
	europa_cli_tree_t *pCliTree, 
	FILE *fp)
{
    int inputLen, cmdLen;

    if(0 == argc)
    {
        /* No any argument for parsing */
        printf("available cmds:\n");
        while(pCliTree->type != EUROPA_CLI_END)
        {
            printf("%s\n", pCliTree->cmd);
            pCliTree += 1;
        }

        return -1;
    }

    if(NULL == pCliTree)
    {
        printf("incorrect command tree\n");
        return -2;
    }

    inputLen = strlen(argv[0]);
    while(pCliTree->type != EUROPA_CLI_END)
    {
        cmdLen = strlen(pCliTree->cmd);
        if(strncmp(argv[0], pCliTree->cmd, inputLen > cmdLen ? cmdLen : inputLen) == 0)
        {
            /* Search subtree or execute the command */
            if(pCliTree->type == EUROPA_CLI_BRANCH)
            {
                return europa_cli_parse(argc - 1, &argv[1], pCliTree->u.pSubTree, fp);
            }
            else if(pCliTree->type == EUROPA_CLI_LEAF)
            {
                if(NULL != pCliTree->u.cli_proc)
                {
                    return pCliTree->u.cli_proc(argc, argv, fp);
                }
                else
                {
                    printf("incorrect command callback\n");
                    return -3;
                }
            }
            else
            {
                printf("incorrect command type\n");
                return -3;
            }
        }
        pCliTree = pCliTree + 1;
    }

    printf("incorrect command\n");
    return -4;
}

int
main( int argc,
      char *argv[])
{
    int ret;
    int permits;
    //key_t msgQKey = 1568;
    //oam_cli_t cli;
    FILE *fp;
    

    //memset(&cli, 0x0, sizeof(cli));
    ret = europa_cli_parse(argc - 1, &argv[1], cliRoot, fp);


    return ret;
}

