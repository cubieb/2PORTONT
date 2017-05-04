#ifndef __SCIO__
#define __SCIO__

#define DIAG_UTIL_ERR_CHK(op, ret)\
do {\
    if ((ret = (op)) != 0)\
    {\
        printk("0x%02x\n", ret);\
        return -1;\
    }\
} while(0)

#define SC_IND_WD	0xBB00003C
#define SC_IND_CMD	0xBB000040
#define SC_IND_RD	0xBB000044
#define REG32(reg)	(*(volatile unsigned int *)((unsigned int)reg))
unsigned int sc_read(uint32_t  address);	
int sc_write(uint32_t  address, uint32_t  value_ptr);
#endif