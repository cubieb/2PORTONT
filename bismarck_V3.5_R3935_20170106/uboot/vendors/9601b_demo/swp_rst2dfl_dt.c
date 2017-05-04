#include <common.h>

#define PATCH_REG(x, lvl) \
	void_func * __swp_##x __attribute__ ((section (".soft_patch." #lvl))) = x

#define RESET_BUTTON_PRESSED (!(REG32(0xb800330c) & (0x1 << 16)))
#define BOOT_TIME_BEFORE_DETECTION (2) //Assume that DRAM calibration consumes about 2 seconds.
#define COUNT_DOWN_DEFAULT (10) //The target is 10 sec 
#define RST2DFL_FLAG_NAME "rst2dfl_flg"
void swp_reset_2_default_detection(void)
{
    unsigned int pressed_sec = BOOT_TIME_BEFORE_DETECTION;
    unsigned int cnt_dn;

    cnt_dn = getenv_ulong("rst2dfl_cnt_dn", 10, COUNT_DOWN_DEFAULT);

    while(RESET_BUTTON_PRESSED) {
        printf("RESET_BUTTON_PRESSED for %u sec\n", pressed_sec);
        if(pressed_sec >= cnt_dn) {
            puts("RESET TO FACTORY DEFAULT!!!\n");
            //The following parameter will be included into bootargs 
            setenv(RST2DFL_FLAG_NAME, "rst2dfl=1");
            break;
        }
        mdelay(1000);
        pressed_sec++;
    }
    
    /* Clean flag if it is accidentally saved into uboot environment */
    if(pressed_sec < cnt_dn) {
        if(NULL != getenv(RST2DFL_FLAG_NAME)) {
            puts("WARNING: " RST2DFL_FLAG_NAME " should not be saved, removing it\n");
            setenv(RST2DFL_FLAG_NAME, "");
            saveenv();
        }
    }
}


PATCH_REG(swp_reset_2_default_detection, 23);

