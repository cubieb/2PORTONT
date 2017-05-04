#include <common.h>
#include <asm/arch/bspchip.h>
#include <asm/addrspace.h>
#include <asm/io.h>
#include <asm/mipsregs.h>
#include <asm/arch/memctl.h>
#include <asm/otto_pll.h>

#define MIPS_COMPANY_ID (0x01<<16)
#define PROCESSOR_ID_4KE (0x90<<8)
#define PROCESSOR_ID_RLX5281 (0xdc02)
#define PROCESSOR_ID_RLX4281 (0xdc01)


//extern u32 board_DRAM_freq_mhz(void);
//extern u32 board_CPU_freq_mhz(void);
extern u32 board_CPU_freq_hz(void);
extern u32 board_DSP_freq_hz(void);
extern u32 board_LX1_freq_hz(void);
extern u32 board_LX_freq_hz(void);
extern void board_DRAM_check(void);
extern void show_cpu_config(void);



phys_size_t initdram(int board_type)
{
	return (long int)memctlc_dram_size();
}

int checkboard (void)
{
    u32 proc_id;
    u32 cpu_clk;
    u32 dsp_clk;
    u32 mem_clk;
    u32 lx0_clk;
    u32 lx1_clk;
    u32 lx2_clk;


    proc_id = read_c0_prid();

    /* Probe Memory Clock */
    mem_clk = board_DRAM_freq_mhz();

    /* Probe CPU Clock*/
    cpu_clk = board_CPU_freq_hz();

    /* Probe DSP Clock*/
    dsp_clk = board_DSP_freq_hz();

    /* Probe LX bus Clock*/
    lx1_clk = board_LX_freq_hz();
    lx0_clk = board_LX_freq_mhz();
    lx2_clk = board_LX_freq_mhz();


    printf ("Board: LUNA\n");
    if(PROCESSOR_ID_RLX5281==proc_id){
        printf ("CPU: RLX5281 %u.%02u MHz, ", cpu_clk/1000000, ((cpu_clk%1000000)*100)/1000000 );
        printf ("DSP: RLX5181 %u.%02u MHz, ", dsp_clk/1000000, ((dsp_clk%1000000)*100)/1000000);
	//show_cpu_config();
        if(memctls_is_DDR()){
        	printf(", DDR%03d", mem_clk*2);
        }
        else if (memctls_is_DDR2())
        {
		printf(", DDR2 %03dMHz", mem_clk);
        }
	else if (memctls_is_DDR3())
	{
		printf(", DDR3 %03dMHz", mem_clk);

	}else{
		printf(", UNKONW DRAM_TYPE %03dMHz", mem_clk);
	}
	printf(", LX:%u.%02u MHz \n", lx1_clk/1000000, ((lx1_clk%1000000)*100)/1000000);
    }
    else
    {
        printf ("Unsupported cpu %d, proc_id=0x%x\n", proc_id >> 24, proc_id);
    }

    /* Wait for GPIO ports to become stable */
    udelay(5000); /* FIXME */

    return 0;
}
