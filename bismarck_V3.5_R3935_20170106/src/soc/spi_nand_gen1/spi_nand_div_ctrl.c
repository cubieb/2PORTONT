#include <soc.h>
#include <cg/cg.h>
#include <spi_nand/spi_nand_ctrl.h>

#ifndef SECTION_CG_CORE_INIT
    #define SECTION_CG_CORE_INIT
#endif

#ifndef SECTION_CG_MISC
    #define SECTION_CG_MISC
#endif

#ifndef SECTION_CG_MISC_DATA
    #define SECTION_CG_MISC_DATA
#endif

SECTION_CG_MISC_DATA
clk_div_sel_info_t sclk_divisor[] = {
    {
        .divisor     = 2,
        .div_to_ctrl = 0,
    },
    {
        .divisor     = 4,
        .div_to_ctrl = 1,
    },
    {
        .divisor     = 6,
        .div_to_ctrl = 2,
    },
    {
        .divisor     = 8,
        .div_to_ctrl = 3,
    },
    {
        .divisor     = 10,
        .div_to_ctrl = 4,
    },
    {
        .divisor     = 12,
        .div_to_ctrl = 5,
    },
    {
        .divisor     = 14,
        .div_to_ctrl = 6,
    },
    {
        .divisor     = 16,
        .div_to_ctrl = 7,
    },
    {   /* The end of structure */
        .divisor     = END_OF_INFO,
    },
};

SECTION_CG_MISC 
u32_t get_spi_ctrl_divisor(void)
{
    return RFLD_SNAFCFR(spi_clk_div);
}

#define DEFAULT_DIV_TO_CTRL (7)

SECTION_CG_MISC 
u32_t get_default_spi_ctrl_divisor(void)
{
    return DEFAULT_DIV_TO_CTRL;
}

SECTION_CG_CORE_INIT 
void set_spi_ctrl_divisor(u16_t clk_div, u16_t spif_mhz)
{
    RMOD_SNAFCFR(spi_clk_div, clk_div);
    if(100 == spif_mhz){
        RMOD_SNAFCFR(pipe_lat, 1);
    }
}

