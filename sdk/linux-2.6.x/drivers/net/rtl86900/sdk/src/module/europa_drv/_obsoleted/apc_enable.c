#ifdef CONFIG_EUROPA_MODULE
#include <linux/kernel.h>
#include <linux/delay.h>
#include <module/europa/europa.h>
#include <module/europa/common.h>
#include <module/europa/utility.h>

#include <module/europa/rtl8290_api.h>

//extern uint16 laser_lut_addr;
extern uint8 loop_mode;
extern uint16 apc_dcl_P0, apc_dcl_P1, apc_dcl_Pavg;
uint32 global_mpd0;

uint32 measure_mpd0(uint16 count ) 
{
    uint16 regData1, regData2, regData3, i;
    uint32 mpd, sum, temp;

    drv_setAsicRegBits(EUROPA_REG_W48_ADDR, 0xff, 0);

    sum = 0;
    for (i=0;i<count;i++)
    {
        drv_setAsicRegBits(EUROPA_REG_W18_ADDR, EUROPA_REG_W18_REG_W18_MASK, 0x02);
        udelay(DELAY_TIME);
        drv_setAsicRegBits(EUROPA_REG_W18_ADDR, EUROPA_REG_W18_REG_W18_MASK, 0x0A);
        udelay(DELAY_TIME);		
        drv_getAsicRegBits(EUROPA_REG_R11_ADDR, EUROPA_REG_R11_REG_R11_MASK, &regData1);
        drv_getAsicRegBits(EUROPA_REG_R12_ADDR, EUROPA_REG_R12_REG_R12_MASK, &regData2);
        drv_getAsicRegBits(EUROPA_REG_R13_ADDR, EUROPA_REG_R13_REG_R13_MASK, &regData3);
        temp = ((regData1 & 0xff) << 16) | ((regData2 & 0xff) << 8) | (regData3);

        printk("loop %d: MPD0 = 0x%x!!!!!\n", i, temp);	
		
	 sum +=temp;
    }
    mpd = sum/count;
    return mpd;	
 }  

	
void apc_enable_flow(apc_enable_fun_t func) 
{
    uint16 tmp;
    uint16 tx_sd_threshold;
    uint16 cnt=0;
	
    switch(func)
    {
        case APC_EN_MCU_READY:
            //set MCU_ready
           drv_setAsicRegBit(EUROPA_REG_M0_ADDR, EUROPA_REG_M0_MCU_READY_STATUS_OFFSET, 0x1);	
            break;    
			
        case APC_EN_CHECK_READY:
            //---- check ready_Bit status---- (bit 2 == 1)
            while(1)
            {
                drv_getAsicRegBit(EUROPA_REG_STATUS_1_ADDR, EUROPA_REG_STATUS_1_READY_STATUS_OFFSET, &tmp);
                if(tmp  == 0x1)
                    break;
                busy_loop(0x50);
            }			
            break;
			
        case APC_EN_TX_SD_THRESHOLD:
            drv_setAsicRegBits(EUROPA_REG_W69_ADDR, 0xFF, 0x10);			
            // Set TX_SD threshold, wait Archy
            //drv_setAsicRegBits(EUROPA_REG_W62_ADDR, EUROPA_REG_W62_APCDIG_CODE_DACTXSD_MASK, 0x0f);		
            setReg(EUROPA_REG_W88_ADDR, 0x01);
            setReg(EUROPA_REG_W80_ADDR, 0x06);			
            setReg(EUROPA_REG_W71_ADDR, 0x05);
            setReg(EUROPA_REG_W72_ADDR, 0x86);
            setReg(EUROPA_REG_W57_ADDR, 0xFC);	
            drv_setAsicRegBit(EUROPA_REG_W74_ADDR, 3, 1);  	
            setReg(EUROPA_REG_W73_ADDR, 0x06);
            setReg(EUROPA_REG_W76_ADDR, 0x71);			
            if (loop_mode == APC_DOL_MODE)
            {
                drv_getAsicRegBit(EUROPA_REG_W57_ADDR, EUROPA_REG_W57_APCDIG_DAC_BIAS_HALF_OFFSET, &tmp);
                if(tmp  == 0x1) 
                    tx_sd_threshold = 0x8;
                else 
                    tx_sd_threshold = 0x10;				
            }
            else if (loop_mode == APC_DCL_MODE)
            {				
                //tmp = getReg(laser_lut_addr - 16 + 4); 	            
                //tx_sd_threshold = getReg(laser_lut_addr - 16 + 4)/2; 		            
                //tx_sd_threshold = getReg(laser_lut_addr - 16 + 4); 				
                tx_sd_threshold = apc_dcl_P0/2;
            }
            else
            {
                //tmp = getReg(laser_lut_addr - 16 + 6);
                //tx_sd_threshold = getReg(laser_lut_addr - 16 + 6)/2;      
                //tx_sd_threshold = getReg(laser_lut_addr - 16 + 6); 					
                tx_sd_threshold = apc_dcl_Pavg/2;
            }				
            drv_setAsicRegBits(EUROPA_REG_W62_ADDR, EUROPA_REG_W62_APCDIG_CODE_DACTXSD_MASK, tx_sd_threshold);  			
            // Rogue ONU Issue			
            setReg(EUROPA_REG_W50_ADDR, 0x0f);
            setReg(EUROPA_REG_W68_ADDR, 0xf8);				
            break;
			
        case APC_EN_APC_BIAS_POWER_ON:
            //Turn on APC  BIAS Power ;
            drv_setAsicRegBits(EUROPA_REG_W57_ADDR, 0xff, 0xFC);		
            break;   
			
        case APC_EN_APC_POWER_SET:
            //Set  APC power and config :set REG_W60=0xFC
            drv_setAsicRegBits(EUROPA_REG_W60_ADDR, 0xff, 0xFD);    
            break;
			
        case APC_EN_APC_DIGITAL_POWER_ON:
            //power on APC_Digital
            drv_setAsicRegBit(EUROPA_REG_APCDIG_RSTN_ADDR, EUROPA_REG_APCDIG_RSTN_APCDIG_RSTN_OFFSET, 1);
            busy_loop(0x300);    
            break;   
			
        case APC_EN_R_PROCESS_DETECT:
             /* Read REG_R31 TX_RPD_RCODE_OUT[1:0] and Write to REG_W50[7:6] / REG_W50[5:4] / REG_W73[4:3] */
            drv_getAsicRegBits(EUROPA_REG_R31_ADDR, EUROPA_REG_R31_TX_RPD_RCODE_OUT_MASK, &tmp);
            drv_setAsicRegBits(EUROPA_REG_W50_ADDR, EUROPA_REG_W50_TX_RLSEL_MASK, tmp);
            drv_setAsicRegBits(EUROPA_REG_W73_ADDR, EUROPA_REG_W73_APCDIG_DUAL_RLSEL_MASK, tmp);
            break;

        case APC_EN_TX_RPD_RCODE_OUT:
            drv_setAsicRegBit(EUROPA_REG_W60_ADDR,EUROPA_REG_W60_APCDIG_TIA_RLSEL_OFFSET,1);		
            break;
			
        case APC_EN_OFFSET_CALIBR:
            //Offset calibration make sure REG_W63/64/65/66/67's[7] and W76[6]/W76[5]=1 ,
            //Set REG_W76[7](APCDIG_OFFK_EN) = 0==> 1
            drv_setAsicRegBit(EUROPA_REG_W63_ADDR,7,1);
            drv_setAsicRegBit(EUROPA_REG_W64_ADDR,7,1);
            drv_setAsicRegBit(EUROPA_REG_W65_ADDR,7,1);
            drv_setAsicRegBit(EUROPA_REG_W66_ADDR,7,1);
            drv_setAsicRegBit(EUROPA_REG_W67_ADDR,7,1);
            drv_setAsicRegBit(EUROPA_REG_W76_ADDR,5,1);
            drv_setAsicRegBit(EUROPA_REG_W76_ADDR,6,1);
        	
            drv_setAsicRegBit(EUROPA_REG_W76_ADDR,7,0);
            busy_loop(0x300);
            drv_setAsicRegBit(EUROPA_REG_W76_ADDR,7,1);
            break;	

        case APC_EN_MEASURE_MPD0:
            global_mpd0 = measure_mpd0(20);
             printk("APC_EN_MEASURE_MPD0: MPD0 = 0x%x!!!!!\n", global_mpd0);				
            break;

        case APC_EN_WAIT_CALIBR:
            busy_loop(0x300);
            drv_setAsicRegBit(EUROPA_REG_W76_ADDR,7,1);
             //----wate 120us ,polling  REG_R30[7]= 1
            udelay(120);    
            while(1)
            {
                drv_getAsicRegBit(EUROPA_REG_R30_ADDR, EUROPA_REG_R30_APCDIG_RO_OFFK_DONE_OFFSET, &tmp);
                if((tmp  == 1)||cnt>=2000)
                {
                    if (cnt>=2000)
                    {
                        //SW user define fault!! useEUROPA_REG_SW_FAULT_STATUS1_ADDR bit 1
                        printk("APC Enable: APC_EN_WAIT_CALIBR over 2000 times!!!!!");	
                        drv_setAsicRegBit(EUROPA_REG_SW_FAULT_STATUS1_ADDR, 1,1);                    
                    }
				
                    break;
                }
                cnt++;
                busy_loop(0x550);
                
            } 
            drv_setAsicRegBit(EUROPA_REG_W76_ADDR,7,0);
            break;

        case APC_EN_MOVE_CALIBR_RESULT:
            //set REG_W74[7:4](=4~8) , readREG_R29[7:0]
             //move offset result to 36bits,REG_W63[6:0]~REG_W67[6:0]---- 
             //write W63 
             drv_setAsicRegBits(EUROPA_REG_W74_ADDR, EUROPA_REG_W74_APCDIG_RO_DATA_SEL_MASK, 0x4);
             drv_getAsicRegBits(EUROPA_REG_R29_ADDR, EUROPA_REG_R29_REG_R29_MASK, &tmp);
             tmp = tmp & 0x7F;
             drv_setAsicRegBits(EUROPA_REG_W63_ADDR, 0x7F, tmp);
             //write W64 
             drv_setAsicRegBits(EUROPA_REG_W74_ADDR, EUROPA_REG_W74_APCDIG_RO_DATA_SEL_MASK, 0x5);
             drv_getAsicRegBits(EUROPA_REG_R29_ADDR, EUROPA_REG_R29_REG_R29_MASK, &tmp);
             tmp = tmp & 0x7F;
             drv_setAsicRegBits(EUROPA_REG_W64_ADDR, 0x7F, tmp);
            //write W65 
             drv_setAsicRegBits(EUROPA_REG_W74_ADDR, EUROPA_REG_W74_APCDIG_RO_DATA_SEL_MASK, 0x6);
             drv_getAsicRegBits(EUROPA_REG_R29_ADDR, EUROPA_REG_R29_REG_R29_MASK, &tmp);
             tmp = tmp & 0x7F;
             drv_setAsicRegBits(EUROPA_REG_W65_ADDR, 0x7F, tmp);
             //write W66 
             drv_setAsicRegBits(EUROPA_REG_W74_ADDR, EUROPA_REG_W74_APCDIG_RO_DATA_SEL_MASK, 0x7);
             drv_getAsicRegBits(EUROPA_REG_R29_ADDR, EUROPA_REG_R29_REG_R29_MASK, &tmp);
             tmp = tmp & 0x7F;
             drv_setAsicRegBits(EUROPA_REG_W66_ADDR, 0x7F, tmp);
             //write W67 
             drv_setAsicRegBits(EUROPA_REG_W74_ADDR, EUROPA_REG_W74_APCDIG_RO_DATA_SEL_MASK, 0x8);
             drv_getAsicRegBits(EUROPA_REG_R29_ADDR, EUROPA_REG_R29_REG_R29_MASK, &tmp);
             tmp = tmp & 0x7F;
             drv_setAsicRegBits(EUROPA_REG_W67_ADDR, 0x7F, tmp);
            break;
			
        default:
            break;        
    }
}
#endif
