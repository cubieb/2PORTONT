#include <linux/kernel.h>
//#include <
#include "pcsc_io.h"
#include "ioal/mem32.h"
#include "ioal/io_mii.h"

#ifdef __KERNEL__
#define PCSCIO_PRINT printk
#else
#define PCSCIO_PRINT printf
#endif


unsigned int sc_read(uint32_t  address_ptr){
    uint32  mem = 0, value = 0;
	//int32   ret = 0;
	
    mem = address_ptr;

	value = 0x10000 | mem;
	
//	PCSCIO_PRINT("mem 0x%08x, value 0x%08x\n", mem, value);
	
	//DIAG_UTIL_ERR_CHK(ioal_mem32_write(0x40, value), ret);
	REG32(SC_IND_CMD) = value;

//	PCSCIO_PRINT("mem 0x%08x, value 0x%08x\n", mem, value);
	//DIAG_UTIL_ERR_CHK(ioal_mem32_read(0x44, &value), ret);
//	value = REG32(SC_IND_RD);

	while ( (value = REG32(SC_IND_RD)) & 0x100) {};

	//PCSCIO_PRINT("mem 0x%08x, value 0x%08x\n", mem, value);
	return value;
}

int sc_write(uint32_t  address_ptr, uint32_t  value_ptr){
    uint32  mem = 0, value = 0, reg;
    //int ret = 0;

    mem = address_ptr;
    value = value_ptr;
//	PCSCIO_PRINT("mem 0x%08x, value 0x%08x\n", mem, value);
	//DIAG_UTIL_ERR_CHK(ioal_mem32_write(0x3c, value), ret);
	REG32(SC_IND_WD) = value;
	//printk("SC_IND_WD 0x%x\n", REG32(SC_IND_WD));

	while ( (reg = REG32(SC_IND_RD)) & 0x100) {};
	
	value = 0x30000 | mem;
	//DIAG_UTIL_ERR_CHK(ioal_mem32_write(0x40, value), ret);
	//printk("value 0x%x\n", value);		
	REG32(SC_IND_CMD) = value;
	//printk("SC_IND_CMD 0x%x\n", REG32(SC_IND_CMD));	
	while ( (reg = REG32(SC_IND_RD)) & 0x100) {};

	return 1;
}