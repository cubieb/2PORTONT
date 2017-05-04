#ifndef PCI_RTL86XX_H
#define PCI_RTL86XX_H

#include "bspchip.h"
#include "gpio.h"

#define MAX_NUM_DEV  4

//#define DEBUG_PRINTK 0



struct pcie_para{
        unsigned char port;
        unsigned char reg;
        unsigned short value;
};

enum clk_source{
        CLK35_328_8676,         //8676 35.328M clk
        CLK40_8676,                             //8676 40M clk
        CLK40_8686,                             //8686 40M clk
        CLK25_8686,                      //8686 25M clk
        CLK35_328_8676S,                //8676S 35.328M clk
        CLK40_8676S,                    //8676S 40M clk
        CLK35_328_0562,                 //0562 35.328M clk
        CLK40_0562,                             //0562 40M clk
        CLK25_8685_P0,                  //8685 25M clk
        CLK25_8685_P1,                  //8685 25M clk
        NOT_DEFINED_CLK
};




#endif