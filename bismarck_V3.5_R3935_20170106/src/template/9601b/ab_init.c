#include <soc.h>
#include <init_define.h>
#include <register_map.h>

void disable_timeout_monitor(void) {
	puts("II: Disabling OCP/LX timeout monitors... ");
	//Disable OCP timeout monitor
	RMOD_TO_CTRL(to_ctrl_en, 0);

	//Disable LX timeout monitor
	RMOD_BUS_TO_CTRL(to_en, 0);
	puts("done\n");
	return;
}
REG_INIT_FUNC(disable_timeout_monitor, 2);
