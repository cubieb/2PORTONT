
#ifdef CONFIG_EUROPA_MODULE
#include <linux/module.h>  /* Needed by all modules */
#include <linux/kernel.h>  /* Needed for KERN_ALERT */
#include <linux/delay.h>
#include <linux/spinlock.h>

#include <module/europa/common.h>
#include <module/europa/europa.h>
//#include <module/europa/laser_drv.h>
#include <module/europa/ddmi.h>
#include <module/europa/init.h>
#include <module/europa/utility.h>
#include <module/europa/interrupt.h>
//#include <system/include/ioal/mem32.h>

#include <module/europa/rtl8290_api.h>


#include <rtk/i2c.h>
#include <hal/chipdef/apollomp/rtk_apollomp_reg_struct.h>
#include <hal/mac/reg.h>
#include <common/error.h>
#include <rtk/gpio.h>
#include <rtk/irq.h>

#if 0   // tempest -- move to include/module/europa.h
#define EUROPA_PARAMETER_LOCATION	"/var/config/europa.data"
#define EUROPA_PARAMETER_SIZE	                                1600
#define EUROPA_DDMIA0_ADDR                                      0
#define EUROPA_DDMIA2_ADDR                                      0x100
#define EUROPA_A4_ADDR                                              0x200
#define EUROPA_A4_ADDR_BASE                                   (EUROPA_A4_ADDR+1)
#define EUROPA_APD_LUT_ADDR                                    0x300
#define EUROPA_LASER_LUT_ADDR                                 0x400
#define EUROPA_DCL_P0_ADDR                                       1346
#define EUROPA_DCL_P1_ADDR                                       1347
#define EUROPA_DCL_PAVG_ADDR                                  1348
#define EUROPA_W57_APCDIG_DAC_BIAS_ADDR           1349
#define EUROPA_RX_A_ADDR                                          1350
#define EUROPA_RX_B_ADDR                                          1354
#define EUROPA_RX_C_ADDR                                          1358
#define EUROPA_RSSI_V0_ADDR                                     1362
#define EUROPA_MPD0_ADDR                                          1366
#endif

rtl8290_param_t europa_param;

extern uint8 flash_data[];
extern uint8 flash_update;


uint8 local_a0_reg[0x100] = {
0x02, 0x04, 0x0B, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x03, 0x0C, 0x00, 0x14, 0xC8,
0x00, 0x00, 0x00, 0x00, 0x22, 0x72, 0x65, 0x61, 0x6C, 0x74, 0x65, 0x6B, 0x20, 0x69, 0x6E, 0x63,
0x2E, 0x22, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x22, 0x65, 0x75, 0x72, 0x6F, 0x70, 0x61, 0x22,
0x22, 0x65, 0x75, 0x72, 0x6F, 0x70, 0x61, 0x22, 0x72, 0x65, 0x76, 0x78, 0x05, 0x1E, 0x00, 0xFF,
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

uint8 local_a2_reg[0x100]={
0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x94, 0x70, 0x6D, 0x60, 0x8C, 0xA0, 0x75, 0x30,
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

#if 0
uint8 local_a4_reg[] = {
0x9E, 0x89, 0xE1, 0xE2, 0x8E, 0x32, 0x9B, 0x90, 0x00, 0x49, 0x9F, 0xFF, 0x23, 0x04, 0x78, 0x7F,
0xFF, 0x00, 0x02, 0x05, 0x00, 0x00, 0x01, 0xF6, 0xCE, 0x90, 0xC0, 0x00, 0x00, 0x38, 0x24, 0x40,
0x40, 0x00, 0x01, 0x13, 0xBC, 0x1E, 0x33, 0x10, 0xFB, 0x18, 0xE4, 0x80, 0xE0, 0x01, 0xB0, 0x44,
0x40, 0xAC, 0x03, 0x00, 0xF4, 0x00, 0x20, 0x40, 0x00, 0x7C, 0x20, 0x40, 0xFC, 0x30, 0x10, 0x00,
0x00, 0x00, 0x00, 0x00, 0xFF, 0xFC, 0x12, 0x0A, 0x8B, 0x04, 0x01, 0x33, 0x74, 0xA5, 0xA0, 0xE0,
0x00, 0x00, 0x00, 0x00, 0x81, 0x30, 0x00, 0x15, 0x00, 0xFF, 0x1F, 0x01, 0x00, 0xFF, 0x00, 0x32,
0xFF, 0x01, 0xFF, 0xFF, 0x05, 0x00, 0x60, 0x80, 0x00, 0x08, 0x10, 0xA0, 0xEC, 0xB0, 0xBF, 0xFE,
0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x10, 0x00, 0x00, 0x00, 0xFF, 0xA0, 0xC0
};
#endif

uint8 local_a4_reg[] = {
0x72, 0x89, 0xE1, 0xE2, 0x8E, 0x32, 0x9B, 0x90, 0x00, 0x49, 0x9F, 0xFF, 0x23, 0x04, 0x78, 0x7F,
0xFF, 0x00, 0x02, 0x05, 0x00, 0x00, 0x01, 0xF6, 0xCE, 0x90, 0xC0, 0x00, 0x00, 0x38, 0x24, 0x40,
0x40, 0x00, 0x01, 0x13, 0xBC, 0x1E, 0x33, 0x10, 0xFB, 0x18, 0xE4, 0x80, 0xE0, 0x01, 0xB0, 0x44,
0x40, 0xAC, 0x03, 0x00, 0xF4, 0x00, 0x20, 0x40, 0x00, 0x7C, 0x20, 0x40, 0xFC, 0x30, 0x10, 0x00,
0x00, 0x00, 0x00, 0x00, 0xFF, 0xFC, 0x12, 0x0A, 0x8B, 0x04, 0x01, 0x33, 0x74, 0xA5, 0xA0, 0xE0,
0x00, 0x00, 0x00, 0x00, 0x81, 0x30, 0x00, 0x15, 0x00, 0xFF, 0x1F, 0x01, 0x00, 0xFF, 0x00, 0x32,
0xFF, 0x01, 0xFF, 0xFF, 0x05, 0x00, 0x60, 0x80, 0x00, 0x08, 0x10, 0xA0, 0xEC, 0xB0, 0xBF, 0xFE,
0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x10, 0x00, 0x00, 0x00, 0xFF, 0xA0, 0xC0
};

uint8  internal;

uint16 apc_dcl_P0, apc_dcl_P1, apc_dcl_Pavg;

extern uint8 loopmode;
extern uint8 I2C_PORT;
extern uint8 INTR_PIN;
extern uint8 TXDIS_PIN;
extern uint8 TXPWR_PIN;
extern uint8 PON_MODE;


void ddmi_a0_init(uint8 flash_update, uint8 *flash_data[])
{
    uint16 addr;
    uint8 *ddmi_a0_reg;

    if (flash_update == 1)
        ddmi_a0_reg = (uint8*)flash_data;
    else
        ddmi_a0_reg=&local_a0_reg[0];

    printk("Update DDMI A0...., flash_update = %d, addr = 0x%x\n ", flash_update, (unsigned int)flash_data);

    for(addr=0; addr <= 0x7F ; addr++)
    {
        setReg(addr, ddmi_a0_reg[addr]);

        if (flash_update == 1)
        {
             if (addr%16==0)
                 printk(" addr = %d  ", addr);

             printk("%02X ",ddmi_a0_reg[addr] );

                if(addr%8 == 7)
                    printk("   ");
                if(addr%16 == 15)
                    printk( "\n");
        }
    }

}

void ddmi_a2_init(uint8 flash_update, uint8 *flash_data[])
{
    uint16 addr;
    uint8 *ddmi_a2_reg;

    if (flash_update == 1)
        ddmi_a2_reg = (uint8*)flash_data;
    else
        ddmi_a2_reg=&local_a2_reg[0];

    printk("Update DDMI A2...., flash_update = %x, addr = 0x%x\n ", flash_update, (unsigned int)flash_data);

    for(addr=0; addr <= 0x7F ; addr++)
    {
        setReg(addr+0x100, ddmi_a2_reg[addr]);

        if (flash_update == 1)
        {
             if (addr%16==0)
                 printk(" addr = %d  ", addr);

             printk("%02X ",ddmi_a2_reg[addr] );

                if(addr%8 == 7)
                    printk("   ");
                if(addr%16 == 15)
                    printk( "\n");
        }
    }

}

void patch_code_a4_init(uint8 flash_update, uint8 patch_len, uint8 *flash_data[])
{
    uint16 addr, length;
    uint8 *patch_code_a4_reg;

    if (flash_update == 1)
    {
        length = patch_len;
        patch_code_a4_reg= (uint8*)flash_data;
    }
    else
    {
        length = sizeof(local_a4_reg);
        patch_code_a4_reg=&local_a4_reg[0];
    }

    printk("Update A4...., flash_update = %x, length =0x%x, addr = 0x%x\n ", flash_update, length, (unsigned int)flash_data);

    for(addr=0; addr <length; addr++)
    {
        setReg(addr+0x200, patch_code_a4_reg[addr]);

        if (flash_update == 1)
        {
             if (addr%16==0)
                 printk(" addr = %d  ", addr);

             printk("%02X ",patch_code_a4_reg[addr] );

                if(addr%8 == 7)
                    printk("   ");
                if(addr%16 == 15)
                    printk( "\n");
        }
    }
    printk( "\n");
}


// FIXME  reg accessing should encap into a logical level API 
void apc_power_init(uint8 flash_update, uint8 *flash_data[])
{
    uint8 *apc_patch_reg;

    if (flash_update==1)
    {
        apc_patch_reg = flash_data;
        apc_dcl_P0 = apc_patch_reg[0];
        apc_dcl_P1 = apc_patch_reg[1];
        apc_dcl_Pavg = apc_patch_reg[2];
    }
    else
    {
        apc_dcl_P0 = 0x28;
        apc_dcl_P1 = 0x50;
        apc_dcl_Pavg = 0x50;
    }

    printk("Update P0 = 0x%x, P1 = 0x%x, Pavg = 0x%x., flash_update = %x,  addr = 0x%x\n ", apc_dcl_P0, apc_dcl_P1, apc_dcl_Pavg, flash_update, (unsigned int)flash_data);

    // FIXME
    // FIXME  20150709 tempest - sync parameters to reg  *dirty temp fix*
    // FIXME
    setReg(EUROPA_REG_W58_ADDR, apc_dcl_P0);
    setReg(EUROPA_REG_W59_ADDR, apc_dcl_P1);
    setReg(EUROPA_REG_W61_ADDR, apc_dcl_Pavg);
}

void parameter_init(void)
{
    //uint16 mode;

    //printk(KERN_EMERG "disable 8051 \n");
    printk(KERN_EMERG "disable DW ...\n");
    drv_setAsicRegBit(EUROPA_REG_CONTROL1_ADDR, EUROPA_REG_CONTROL1_DW8051_DISABLE_OFFSET, 1);
    printk(KERN_EMERG "[ init ddmi A0&A2, A4 ]\n");
    ddmi_a0_init(flash_update, &flash_data[EUROPA_DDMI_A0_ADDR]);
    ddmi_a2_init(flash_update, &flash_data[EUROPA_DDMI_A2_ADDR]);
    patch_code_a4_init(flash_update, flash_data[EUROPA_A4_PATCH_LEN_ADDR], &flash_data[EUROPA_PATCH_A4_ADDR]);

    internal = TRUE;

    apc_power_init(flash_update, &flash_data[EUROPA_DCL_P0_ADDR]);

}


void user_parameter_init(void)
{
    printk(KERN_EMERG "init user parameters ...\n");

    // setup crossing point adjustment +
    {
	uint8 cross_val, cross_mask;
	uint16 w48;

	cross_val = flash_data[EUROPA_CROSS_ADDR];

	cross_mask = (EUROPA_REG_W48_TX_OFFSET_MOD_EN_MASK |
	              EUROPA_REG_W48_TX_OFFSET_MOD_SIGN_MASK |
		      EUROPA_REG_W48_TX_OFFSET_MOD_STR_MASK);

	cross_val = cross_val & cross_mask;

	//w48 = 0;  /* W48 should be 0 here... */
	w48 = getReg(EUROPA_REG_W48_ADDR);

	w48 = (w48 & (0xff ^ cross_mask)) | cross_val;

	//printk("cross_val=0x%x w48=0x%x\n", cross_val, w48);

	setReg(EUROPA_REG_W48_ADDR, w48);
    }
    // setup crossing point adjustment

    // setup temperature interrupt scale +
    {
	uint8 temp_scale;

	temp_scale = flash_data[EUROPA_TEMP_INTR_SCALE_ADDR];

	if (0<temp_scale && 8>temp_scale)
	{
	   setReg(EUROPA_REG_TEMP_INT_OFFSET_ADDR, temp_scale);
	}
    }
    // setup temperature interrupt scale -

    // setup RX SD_TH & SD_DETH +
    {
	uint8 reg;

	reg = flash_data[EUROPA_RX_TH_ADDR];
	if (0 != reg) // FIXME  validate further more
	{
	   setReg(EUROPA_REG_RX_TH_ADDR, reg);
	}

	reg = flash_data[EUROPA_RX_DETH_ADDR];
	if (0 != reg) // FIXME  validate further more
	{
	   setReg(EUROPA_REG_RX_DE_TH_ADDR, reg);
	}
    }
    // setup RX SD_TH & SD_DETH -

    memcpy(&europa_param.rx_a,  &flash_data[EUROPA_RX_A_ADDR], sizeof(europa_param.rx_a));
    memcpy(&europa_param.rx_b,  &flash_data[EUROPA_RX_B_ADDR], sizeof(europa_param.rx_b));
    memcpy(&europa_param.rx_c,  &flash_data[EUROPA_RX_C_ADDR], sizeof(europa_param.rx_c));	
    memcpy(&europa_param.tx_a,  &flash_data[EUROPA_TX_A_ADDR], sizeof(europa_param.tx_a));
    memcpy(&europa_param.tx_b,  &flash_data[EUROPA_TX_B_ADDR], sizeof(europa_param.tx_b));
    memcpy(&europa_param.tx_c,  &flash_data[EUROPA_TX_C_ADDR], sizeof(europa_param.tx_c));		

    memcpy(&europa_param.temp_offset,  &flash_data[EUROPA_TEMP_OFFSET_ADDR], sizeof(europa_param.temp_offset));		

    europa_param.rx_windowcnt = 16;
    europa_param.rx_sum	=0;

}


void up_init(void)
{
#if 0
    /***************** TEST CODE FOR 8051 I-ROM READ/WRITE **************/
    char magic[9];

    //drv_setAsicRegBit(EUROPA_REG_CONTROL1_ADDR, EUROPA_REG_CONTROL1_DW8051_DISABLE_OFFSET, 1);

    printk(KERN_EMERG "[uP init]\n");
    printk(KERN_EMERG "  writing...\n");

    drv_setAsicRegBit(EUROPA_REG_IIC_S_ADDR, EUROPA_REG_IIC_S_IROM_RW_ENABLE_OFFSET, 1);

    setReg(0x6000, 'D');
    setReg(0x6001, 'e');
    setReg(0x6002, 'A');
    setReg(0x6003, 'd');
    setReg(0x6004, 'B');
    setReg(0x6005, 'e');
    setReg(0x6006, 'E');
    setReg(0x6007, 'f');

    drv_setAsicRegBit(EUROPA_REG_IIC_S_ADDR, EUROPA_REG_IIC_S_IROM_RW_ENABLE_OFFSET, 0);


    printk(KERN_EMERG "  reading...\n");

    drv_setAsicRegBit(EUROPA_REG_IIC_S_ADDR, EUROPA_REG_IIC_S_IROM_RW_ENABLE_OFFSET, 1);

    magic[0] = getReg(0x6000);
    magic[1] = getReg(0x6001);
    magic[2] = getReg(0x6002);
    magic[3] = getReg(0x6003);
    magic[4] = getReg(0x6004);
    magic[5] = getReg(0x6005);
    magic[6] = getReg(0x6006);
    magic[7] = getReg(0x6007);
    magic[8] = '\0';

    drv_setAsicRegBit(EUROPA_REG_IIC_S_ADDR, EUROPA_REG_IIC_S_IROM_RW_ENABLE_OFFSET, 0);
    printk(KERN_EMERG "  magic={%s}\n", magic);
    printk(KERN_EMERG "  uP test done.\n");
#endif
}

void europa_intrInit(void)
{
#ifdef CONFIG_EUROPA_INTERRUPT
    int32 retval;
    uint16 regData;
    uint16 mask1, mask2;

    retval = rtk_gpio_mode_set(INTR_PIN,GPIO_INPUT);	;
    if(retval != 0)
        printk(KERN_EMERG "rtk_gpio_mode_set fail  retval=%d \n",retval);

    retval = rtk_gpio_state_set(INTR_PIN, ENABLE);
    if(retval != 0)
        printk(KERN_EMERG "rtk_gpio_state_set fail  retval=%d \n",retval);

    retval = rtk_gpio_intr_set(INTR_PIN,
                               GPIO_INTR_ENABLE_FALLING_EDGE);
    if(retval != 0)
        printk(KERN_EMERG "rtk_gpio_intr_set fail  retval=%d \n",retval);

    retval = rtk_irq_gpioISR_register(INTR_PIN,
                                      laser_drv_intrHandle);
    if(retval != 0)
       printk(KERN_EMERG "rtk_irq_gpioISR_register fail  retval=%d \n",retval);

    printk(KERN_EMERG "europa_intrInit \n");

    /*Set Europa MCU Interrupt Enable*/
    setReg(EUROPA_REG_EXTRA_ADDR, 0x1);

    /*turn on Europa external interrupt*/
    //EX0 = 1;

    /*For clear interrupt status*/
    regData = getReg(EUROPA_REG_INT_STATUS1_ADDR);
    regData = getReg(EUROPA_REG_INT_STATUS2_ADDR);

    /*turn on interrupt */
    //mask1 =(~(EUROPA_REG_INT_MASK1_TEMP_CHANGE_INT_MASK_MASK|EUROPA_REG_INT_MASK1_RSSI_ER_INT_MASK_MASK|EUROPA_REG_INT_MASK1_LOST_INT_MASK_MASK|EUROPA_REG_INT_MASK1_FAULT_INT_MASK_MASK|EUROPA_REG_INT_MASK1_FAULT_RELEASE_INT_MASK_MASK|EUROPA_REG_INT_MASK1_DDMI_TX_DIS_INT_MASK_MASK|EUROPA_REG_INT_MASK1_TX_DIS_INT_MASK_MASK))&0xff;
    //mask2 = 0x5;
    mask1 = mask2 = 0xFF;


    // turn on temp change interrupt
    mask1 =(~(EUROPA_REG_INT_MASK1_TEMP_CHANGE_INT_MASK_MASK))&0xff;

    //mask2 =(~(EUROPA_REG_INT_STATUS2_TX_SD_TOGGLE_INT_MASK))&0xff;
    // TODO  EUROPA_REG_INT_STATUS2_UNDER_TX_DIS_RELEASE_INT_MASK

    laser_drv_intrEnable(mask1, mask2);
#endif
}


void europa_intrDeInit(void)
{
#ifdef CONFIG_EUROPA_INTERRUPT
    int32 retval;

    // disable interrupt
    laser_drv_intrDisable();

    retval = rtk_irq_gpioISR_unregister(INTR_PIN);
    if(retval != 0)
       printk(KERN_EMERG "rtk_irq_gpioISR_register fail  retval=%d \n",retval);

    printk(KERN_EMERG "europa_intrDeInit done.\n");
#endif
}


void apollo_init(void)
{
//#ifdef CONFIG_TX_DISABLE_GPIO_PIN
    printk(KERN_EMERG "config TX_DIS pin...\n");
    rtl8290_gpio_init(TXDIS_PIN);
//#endif

//#ifdef CONFIG_TX_POWER_GPO_PIN
    printk(KERN_EMERG "config TxPwr pin...\n");
    rtl8290_gpio_init(TXPWR_PIN);
//#endif

    printk(KERN_EMERG "Init I2C port ...\n");
    rtl8290_i2c_init(I2C_PORT);
}

void europa_init(void)
{
    uint8 *ptr_data;
    uint32 status;
    uint16 status2;

    uint8 dac_full_scale;  /* controls W57[4] BIAS_HALF bit */

    memset(&europa_param, 0, sizeof(rtl8290_param_t));

    rtl8290_reset(LDD_RESET_CHIP);
    udelay(500);
    parameter_init();

    rtl8290_powerOnStatus_get(&status);
    if (status !=0)
    {
        printk("rtk_europa_powerOnStatus_get error!!!!!!!status = 0x%x \n", status);
        return;
    }

#if 0
    printk("Get RSSI V0 !!!!!!!\n");
    rtl8290_rssiV0_get(100, &europa_param.rssi_v0);
    printk("SDADC RSSI V0 = %d!!!!\n", europa_param.rssi_v0);
#else
    // Note: RSSI_V0 is determined during optical calibration
    //       donot detect it every bootup since dark condition is not guarnteed
    //       SHOULD use unified param read/write API here!!
    printk("Read RSSI_V0 from flash...\n");
    memcpy(&europa_param.rssi_v0,
           &flash_data[EUROPA_RSSI_V0_ADDR], sizeof(europa_param.rssi_v0));
    printk("SDADC RSSI V0 = %d...\n", europa_param.rssi_v0);
#endif

    /* APC Enable flow*/
    printk("[ APC Enable flow ]\n");
    rtl8290_apcEnableFlow_set(LDD_APC_MCU_READY, loopmode);
    rtl8290_apcEnableFlow_set(LDD_APC_CHECK_READY, loopmode);
    rtl8290_apcEnableFlow_set(LDD_APC_TX_SD_THRESHOLD, loopmode);
    rtl8290_apcEnableFlow_set(LDD_APC_BIAS_POWER_ON, loopmode);
    rtl8290_apcEnableFlow_set(LDD_APC_POWER_SET, loopmode);
    // rtl8290_apcEnableFlow_set(LDD_APC_DIGITAL_POWER_ON, loop_mode);
    rtl8290_apcEnableFlow_set(LDD_APC_R_PROCESS_DETECT, loopmode);
    /*Richard suggest to move after R Process Detect*/
    rtl8290_apcEnableFlow_set(LDD_APC_DIGITAL_POWER_ON, loopmode);
    rtl8290_apcEnableFlow_set(LDD_APC_TX_RPD_RCODE_OUT, loopmode);
    rtl8290_apcEnableFlow_set(LDD_APC_OFFSET_CALIBR, loopmode);

    europa_param.mpd0 = 0;
    //rtl8290_mpd0_get(10, &europa_param.mpd0);
    rtl8290_mpd0_get_NoBackUpW48(10, &europa_param.mpd0);

    if (europa_param.mpd0 == 0)
    {
        printk("\nMPD0 Error in APC Enable flow!!!!!!!\n");
    }

    rtl8290_apcEnableFlow_set(LDD_APC_WAIT_CALIBR, loopmode);
    rtl8290_apcEnableFlow_set(LDD_APC_MOVE_CALIBR_RESULT, loopmode);


    //  set up DAC scale : Full or Half
    //
    dac_full_scale = flash_data[EUROPA_W57_APCDIG_DAC_BIAS_ADDR];
    {
       uint8 w57;

       w57 = getReg(EUROPA_REG_W57_ADDR) &
	            (0xFF ^ EUROPA_REG_W57_APCDIG_DAC_BIAS_HALF_MASK);
       w57 |= ((dac_full_scale^1)<<EUROPA_REG_W57_APCDIG_DAC_BIAS_HALF_OFFSET);

       setReg(EUROPA_REG_W57_ADDR, w57);

       if (1==dac_full_scale)
	       printk("DAC Full Scale!\n");
    }

    /* Tx Enable flow */
    printk("[ Tx Enable flow ]\n");
    udelay(100);
    ptr_data = &flash_data[EUROPA_LASER_LUT_ADDR];
    rtl8290_txEnableFlow_set(LDD_TX_BIAS_MOD_PROTECT, loopmode, ptr_data);
    rtl8290_txEnableFlow_set(LDD_TX_ENABLE_TX, loopmode, ptr_data);
    rtl8290_txEnableFlow_set(LDD_TX_BIAS_MOD_LUT, loopmode, ptr_data);
    rtl8290_txEnableFlow_set(LDD_TX_LOOP_MODE, loopmode, ptr_data);
    rtl8290_txEnableFlow_set(LDD_TX_BURST_MODE_CTRL, loopmode, ptr_data);
    rtl8290_txEnableFlow_set(LDD_TX_WAIT_500MS, loopmode, ptr_data);

    /* Rx Enable flow */
    printk("[ Rx Enable flow ]\n");
    //osal_time_mdelay(100);
    rtl8290_rxEnableFlow_set(LDD_RX_THRESHOLD);
    /* Enable Booster: set REG_W4<4> = 1 */
    rtl8290_rxEnableFlow_set(LDD_RX_BOOSTER);
    /* Enable RX: set REG_W41<4> = 0 */
    rtl8290_rxEnableFlow_set(LDD_RX_RX);
    /*Enable RSSI*/
    rtl8290_rxEnableFlow_set(LDD_RX_RSSI);
    /*Enable DAC*/
    rtl8290_rxEnableFlow_set(LDD_RX_DAC);

    /* Update APD lookup table */
    printk("[ Update APD     ]\n");
   // osal_time_mdelay(100);
    ptr_data = &flash_data[EUROPA_APD_LUT_ADDR];
    rtl8290_apdLut_set(ptr_data);

    printk("[ Enable TX SD   ]\n");
    //osal_time_mdelay(100);
    ptr_data = &flash_data[EUROPA_LASER_LUT_ADDR];
    rtl8290_txEnableFlow_set(LDD_TX_ENABLE_TX_SD, loopmode, ptr_data);
    rtl8290_txEnableFlow_set(LDD_TX_FAULT_ENABLE, loopmode, ptr_data);

    /* set threshold value */
    /* 1. REG_TEMP_TH   -- Get from DDMI A2 byte 0/1  alarm high */
    ddmi_temp_threshold_set();
    /* 2. REG_IMPD_TH   -- Read address (IMPD translate function 1 ) */
    //????????????????
    /* 3. REG_VOL_TH     -- Get from DDMI A2 byte 8/9 (voltage translate function 1) */
    ddmi_volt_threshold_set(internal);

    /* Fault enable, turn on all bits in REG_W53 */
    //drv_setAsicRegBits(EUROPA_REG_W53_ADDR, 0xff, 0xff);
    //drv_setAsicRegBits(EUROPA_REG_FAULT_CTL_ADDR, 0xff, 0x2);
    //drv_setAsicRegBits(EUROPA_REG_FAULT_INHIBIT_ADDR, 0xff, 0);

    /* Turn on Interrupt mask */
    //laser_drv_intrEnable(0x20, 0x5) ;

    /* wait no fault condition */
    drv_getAsicRegBits(EUROPA_REG_FAULT_STATUS_ADDR, 0xff, &status2);
    if(status2 != 0)
    {
        printk("\n!!! FAULT STATUS in Startup Procedure !!!\n");

        laser_drv_fault_status();

        drv_getAsicRegBits(EUROPA_REG_STATUS_2_ADDR,0xFF,&status2);
        if((EUROPA_REG_STATUS_2_FAULT_STATUS_MASK&status2)!=0 )
        {
             /* disable TX(REG_CONTROL2.TX_POW_CTL =0) */
            laser_drv_functionCtrl(EUROPA_LASER_FUN_TX,DISABLE);
            laser_drv_functionCtrl(EUROPA_LASER_FUN_VDD_LDX,DISABLE);
            laser_drv_functionCtrl(EUROPA_LASER_FUN_BOOSTER,DISABLE);
        }

    //laser_drv_intrDisable();
    }
    Check_C0_C1();

    user_parameter_init();

    up_init();


    /* enable SD/LOS at this moment when booster and threshold are ready,
     * in case wrongly behavior confuses PON MAC */
    printk("[ Enable RX_SD   ]\n");
    rtl8290_rxEnableFlow_set(LDD_RX_ENABLE_RX_SD);
}


rtk_ldd_mapper_t rtl8290_ldd_mapper =
{
    .ldd_i2c_init = rtl8290_i2c_init,
    .ldd_gpio_init = rtl8290_gpio_init,
    .ldd_reset = rtl8290_reset,
    .ldd_calibration_state_set = rtl8290_calibration_state_set,
    .ldd_parameter_set = rtl8290_parameter_set,
    .ldd_parameter_get = rtl8290_parameter_get,
    .ldd_loop_mode_set = rtl8290_loopMode_set,
    .ldd_laser_lut_set = rtl8290_laserLut_set,
    .ldd_apd_lut_set = rtl8290_apdLut_set,
    .ldd_apc_enable_set = rtl8290_apcEnableFlow_set,
    .ldd_tx_enable_set = rtl8290_txEnableFlow_set,
    .ldd_rx_enable_set = rtl8290_rxEnableFlow_set,
    .ldd_power_on_get = rtl8290_powerOnStatus_get,
    .ldd_tx_power_get = rtl8290_tx_power_get,
    .ldd_rx_power_get = rtl8290_rx_power_get,
    .ldd_rssi_voltage_get = rtl8290_rssiVoltage_get,
    .ldd_rssi_v0_get = rtl8290_rssiV0_get,
    .ldd_vdd_get = rtl8290_vdd_get,
    .ldd_mpd0_get = rtl8290_mpd0_get,
    .ldd_temperature_get = rtl8290_temperature_get,
    .ldd_tx_bias_get = rtl8290_tx_bias_get,
    .ldd_tx_mod_get = rtl8290_tx_mod_get,
    .ldd_tx_bias_set = rtl8290_tx_bias_set,
    .ldd_tx_mod_set = rtl8290_tx_mod_set,
    .ldd_driver_version_get = rtl8290_driver_version_get,
    .ldd_steering_mode_set = rtl8290_steering_mode_set,
    .ldd_integration_mode_set = rtl8290_integration_mode_set,
    .ldd_steering_mode_fixup = rtl8290_steering_mode_fixup,
    .ldd_integration_mode_fixup = rtl8290_integration_mode_fixup,
};

void ldd_mapper_init(void)
{
    rtk_lddMapper_attach(&rtl8290_ldd_mapper);
}

void ldd_mapper_exit(void)
{
    rtk_lddMapper_deattach();
}


#include <module/intr_bcaster/intr_bcaster.h>
static intrBcasterNotifier_t ploamStateChangeNotifier; 
static ldd_ploam_state = 1;

void ldd_ploamState_handler(intrBcasterMsg_t *pMsgData)
{
     if (MSG_TYPE_ONU_STATE == pMsgData->intrType)
     {
#if 1
        printk("[ploamState] O%d->O%d\n", ldd_ploam_state, pMsgData->intrSubType);
#endif
	/* O1->O2 */
	if (1==ldd_ploam_state && 2==pMsgData->intrSubType)
	{
	    rtl8290_steering_mode_set();	
	}
	/* O3->O2 */
	if (3==ldd_ploam_state && 2==pMsgData->intrSubType)
	{
	    rtl8290_steering_mode_fixup();	
	}
	/* O4->O5 */
	if (4==ldd_ploam_state && 5==pMsgData->intrSubType)
	{
	    rtl8290_integration_mode_set();	
	}

        ldd_ploam_state = pMsgData->intrSubType;
     }
}

static intrBcasterNotifier_t ploamStateChangeNotifier; 

void ldd_ploamState_handler_init(void)
{
    ploamStateChangeNotifier.notifyType = MSG_TYPE_ONU_STATE;
    ploamStateChangeNotifier.notifierCb = ldd_ploamState_handler;
    intr_bcaster_notifier_cb_register(&ploamStateChangeNotifier);
}

void ldd_ploamState_handler_exit(void)
{
    intr_bcaster_notifier_cb_unregister(&ploamStateChangeNotifier);
}


#endif
