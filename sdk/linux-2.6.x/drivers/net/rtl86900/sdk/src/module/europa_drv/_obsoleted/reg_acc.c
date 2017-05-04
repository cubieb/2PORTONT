#ifdef CONFIG_EUROPA_MODULE
//#include <linux/module.h>  /* Needed by all modules */
#include <linux/kernel.h>  /* Needed for KERN_ALERT */
#include <module/europa/common.h>
#include <rtk/i2c.h>
#include <module/europa/europa_reg_definition.h>
#include <module/europa/utility.h>



#ifdef CONFIG_EUROPA_I2C_PORT_0
#define I2C_EUROPA_PORT 0
#elif defined(CONFIG_EUROPA_I2C_PORT_1) 
#define I2C_EUROPA_PORT 1
#else
#define I2C_EUROPA_PORT 0
#endif


void setReg(uint16 address, uint16 value)
{
    uint16 eeprom_addr=0, i2c_addr = 0;
    uint8 devId=0;
    uint32 status;
    int32   ret;
    
    if(address >= 0xc000) //EEPROM WRITE
    {
        eeprom_addr = address - 0xc000;
        devId = 0x55;
printk(KERN_EMERG "1 setReg eeprom_addr=%d  devId=%d\n",eeprom_addr, devId);
        rtk_i2c_write(I2C_EUROPA_PORT, devId, EUROPA_REG_INDACC_ADDR_LB_ADDR - 0x300, eeprom_addr & 0xff); //ADDR_LB
        rtk_i2c_write(I2C_EUROPA_PORT, devId, EUROPA_REG_INDACC_ADDR_HB_ADDR - 0x300, 0xc0+((eeprom_addr>>8) & 0xff)); //ADDR_HB
        rtk_i2c_write(I2C_EUROPA_PORT, devId, EUROPA_REG_INDACC_WDATA_0_ADDR - 0x300, value & 0xff); //WDATA_0
        while(1) //check EEPROM BUSY
        {
            rtk_i2c_read(I2C_EUROPA_PORT, devId, EUROPA_REG_INDACC_STATUS_ADDR - 0x300, &status); //STATUS
            if((status & 0x1) == 0)
                break;
        }
       ret =  rtk_i2c_write(I2C_EUROPA_PORT, devId, EUROPA_REG_INDACC_CONTROL_ADDR - 0x300, 0x13); //CONTROL(Write one byte)
    }
    else
    {
        if(address < 0x100) //0xa0
        {
            devId = 0x50;
            i2c_addr = address;
        }
        else if((address >= 0x100) && (address < 0x200)) //0xa1
        {
            devId = 0x51;
            i2c_addr = address - 0x100;
        }
        else if((address >= 0x200) && (address < 0x300)) //0xa4
        {
            devId = 0x54;
            i2c_addr = address - 0x200;
        }
        else if(address >= 0x300) //0xa5
        {
            devId = 0x55;
            i2c_addr = address - 0x300;
        }

        ret = rtk_i2c_write(I2C_EUROPA_PORT, devId, i2c_addr, value & 0xff);

		
    }
	
    if (ret !=0)
    {
       printk(KERN_EMERG "setReg FAULT: 0x%x!!!!\n", ret);	 
    }
}

uint16 getReg(uint16 address)
{
    uint16 eeprom_addr=0, i2c_addr = 0;
    uint16 devId=0;
    uint32 status, data;
    int32 ret;
    uint16 i;

    if(address >= 0xc000) //EEPROM READ
    {
        eeprom_addr = address - 0xc000;
        devId = 0x55;
printk(KERN_EMERG "1 getReg eeprom_addr=%d  devId=%d\n",eeprom_addr, devId);
       ret =  rtk_i2c_write(I2C_EUROPA_PORT, devId, EUROPA_REG_INDACC_ADDR_LB_ADDR - 0x300, eeprom_addr & 0xff); //ADDR_LB
       ret =  rtk_i2c_write(I2C_EUROPA_PORT, devId, EUROPA_REG_INDACC_ADDR_HB_ADDR - 0x300, 0xc0+((eeprom_addr>>8) & 0xff)); //ADDR_HB

//rtk_i2c_read(I2C_EUROPA_PORT, devId, EUROPA_REG_INDACC_ADDR_LB_ADDR - 0x300, &status); //STATUS
//printk(KERN_EMERG "1 getReg status=%d  \n",status);
//rtk_i2c_read(I2C_EUROPA_PORT, devId, EUROPA_REG_INDACC_ADDR_HB_ADDR - 0x300, &status); //STATUS
//printk(KERN_EMERG "2 getReg status=%d  \n",status);
#if 0
        while(1) //check EEPROM BUSY
        {
            rtk_i2c_read(I2C_EUROPA_PORT, devId, EUROPA_REG_INDACC_STATUS_ADDR - 0x300, &status); //STATUS
            if((status & 0x1) == 0)
                break;
        }
#endif        
        ret = rtk_i2c_write(I2C_EUROPA_PORT, devId, EUROPA_REG_INDACC_CONTROL_ADDR - 0x300, 0x11); //CONTROL(Read one byte)
        while(1) //check EEPROM BUSY
        {
            ret = rtk_i2c_read(I2C_EUROPA_PORT, devId, EUROPA_REG_INDACC_STATUS_ADDR - 0x300, &status); //STATUS
            if((status & 0x1) == 0)
                break;
        }
        busy_loop(0xffff);
        ret = rtk_i2c_read(I2C_EUROPA_PORT, devId, EUROPA_REG_INDACC_RDATA_0_ADDR - 0x300, &data); //Get data
    }
    else
    {
        if(address < 0x100) //0xa0
        {
            devId = 0x50;
            i2c_addr = address;
        }
        else if((address >= 0x100) && (address < 0x200)) //0xa1
        {
            devId = 0x51;
            i2c_addr = address - 0x100;
        }
        else if((address >= 0x200) && (address < 0x300)) //0xa4
        {
            devId = 0x54;
            i2c_addr = address - 0x200;
        }
        else if(address >= 0x300) //0xa5
        {
            devId = 0x55;
            i2c_addr = address - 0x300;
        }

        ret = rtk_i2c_read(I2C_EUROPA_PORT, devId, i2c_addr, &data); //Get data
//printk(KERN_EMERG "2 getReg devId=%d  address=%d  retval=%d \n", devId ,i2c_addr, retval);
    }

    if (ret !=0)
    {
       printk(KERN_EMERG "getReg FAULT: 0x%x!!!!\n", ret);	 
    }	
    return data & 0xff;
}
#endif

