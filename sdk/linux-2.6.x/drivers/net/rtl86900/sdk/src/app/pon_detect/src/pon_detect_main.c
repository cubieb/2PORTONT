/*
 * Copyright (C) 2012 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: 40647 $
 * $Date: 2013-07-01 15:36:16 +0800 (Mon, 01 Jul 2013) $
 *
 * Purpose : Main function of the EPON OAM protocol stack user application
 *           It create two additional threads for packet Rx and state control
 *
 * Feature : Start point of the EPON OAM protocol stack. Use individual threads
 *           for packet Rx and state control
 *
 */

/*
 * Include Files
 */
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/time.h>

#include <common/error.h>
#include <rtk/ponmac.h>
#include <LINUX/mib.h>



/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */

/*
 * Macro Definition
 */
#define RESET_CDR_TIMES  1

/*
 * Function Declaration
 */
int
main(
	int argc,
	char *argv[])
{
#if defined(CONFIG_PON_DETECT_FEATURE)
    rtk_ponmac_mode_t check_mode, set_mode;
    unsigned int sd, sync;
    unsigned char buf[32];
	char *strMode=NULL;
	FILE *pFD;
    int reset_cnt=0;
    int ret;

    if((ret = rtk_ponmac_mode_get(&check_mode)) != RT_ERR_OK)
    {
        printf("pon auto detect: get pon mode fail 0x%x\n\r", ret);
        return 0;
    }

    while(1)
    {
        sleep(2);
        /* get pon link state */
        if((ret = rtk_ponmac_linkState_get(check_mode, &sd, &sync))!= RT_ERR_OK)
        {
            printf("pon auto detect: get link state faile 0x%x\n\r", ret);
            return 0;
        }
        /*printf("pon auto detect: pon mode=%d, get sd=%d, sync=%d\n\r",mib_pon_mode, sd, sync);*/
        if((sd == 0) && (sync == 0)) /* no link */
        {
            /*printf("pon auto detect: pon mode detect no link \n\r");*/
            reset_cnt = 0;
        }
        else
        if((sd == 1) && (sync == 0)) /* wrong pon mode */
        {
#if defined(CONFIG_PON_AUTO_SELECT_FEATURE)
            if(reset_cnt > RESET_CDR_TIMES)
                break;
#endif
            /* serdes cdr reset for re-lock */
            if((ret = rtk_ponmac_serdesCdr_reset()) != RT_ERR_OK)
            {
                printf("pon auto detect: serdes reset fail 0x%x\n\r", ret);
            }
            printf("pon auto detect: sd=1, sync=0, reset-cdr \n\r");
            reset_cnt++;
        }
        else
        if((sd == 1) && (sync == 1)) /* correct pon mode */
        {
            /*printf("pon auto detect: pon mode detect correct mode %d \n\r", check_mode);*/
            reset_cnt = 0;
        }
        else /* strange state */
        {
            printf("pon auto detect: pon mode detect strange!!! sd=%d, sync=%d!!\n\r", sd, sync);
        }
    }

#if defined(CONFIG_PON_AUTO_SELECT_FEATURE)
    printf("pon auto detect: pon mode detect wrong mode, set to mode %d \n\r", set_mode);
    if(check_mode == PONMAC_MODE_GPON)
    {
        set_mode = EPON_MODE;
    }
    else
    {
        set_mode = GPON_MODE;
    }

    printf("pon auto detect: MIB_PON_MODE set to mode=%d, and then reboot\n\r", set_mode);
    /* set pon mode */
    memset(buf, 0, 32);
    sprintf(buf, "flash set PON_MODE %d", set_mode);
    system(buf);

    /* reboot device */
    system("reboot -f");
#endif
#endif
    return 0;
}

