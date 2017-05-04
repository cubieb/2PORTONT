#include <soc.h>
#include <init_define.h>
#include <register_map.h>


void disable_timeout_monitor(void)
{
    //Disable OCP timeout monitor
    TO_CTRL_T ocp_to_mon;
    ocp_to_mon.v = TO_CTRLrv;
    ocp_to_mon.f.to_ctrl_en = 0;
    TO_CTRLrv = ocp_to_mon.v;
    puts("Disable OCP Timeout Monitor\n");

    //Disable LX timeout monitor
    BUS_TO_CTRL_T lx_to_mon;
    lx_to_mon.v = BUS_TO_CTRLrv;
    lx_to_mon.f.to_en = 0;
    BUS_TO_CTRLrv = lx_to_mon.v;
    puts("Disable LX Timeout Monitor\n");
}
REG_INIT_FUNC(disable_timeout_monitor, 2);

