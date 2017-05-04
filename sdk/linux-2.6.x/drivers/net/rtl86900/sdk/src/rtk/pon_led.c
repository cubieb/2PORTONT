/*
 * Copyright (C) 2014 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: 51563 $
 * $Date: 2014-09-23 10:56:10 +0800 (Tue, 23 Sep 2014) $
 *
 * Purpose : Definition those public PON LED APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           (1) PON LED settings
 */

/*
 * Include Files
 */
#include <common/rt_type.h>
#include <common/error.h>
#include <rtk/pon_led.h>
#include <rtk/pon_led_profile.h>
#include <rtk/led.h>
#include <rtk/ponmac.h>
#include <osal/lib.h>
#include <osal/print.h>
#ifdef CONFIG_SDK_KERNEL_LINUX
#include <linux/kthread.h>
#include <linux/sched.h>
#endif

/*
 * Symbol Definition
 */
/*
 * Data Declaration
 */
static rtk_pon_led_spec_type_t pon_led_spec_type=0;
#ifdef CONFIG_SDK_KERNEL_LINUX
static struct task_struct *pLedBlink;
#endif

/*
 * Function Declaration
 */
static int pon_led_gpon_spec_type_00_init(void)
{
#ifdef PON_LED_GROUP_PON_STATUS
{
    int ret;
    rtk_led_config_t ledConfig;
	rtk_led_force_mode_t forceMode;

	if((ret = rtk_led_parallelEnable_set(PON_LED_GROUP_PON_STATUS, ENABLED)) != RT_ERR_OK)
		return ret;

	osal_memset(&ledConfig,0x0,sizeof(rtk_led_config_t));
	ledConfig.ledEnable[LED_CONFIG_FORCE_MODE] = 1;
	if((ret = rtk_led_config_set(PON_LED_GROUP_PON_STATUS, LED_TYPE_PON, &ledConfig)) != RT_ERR_OK)
		return ret;

	forceMode = LED_FORCE_OFF;
    PON_LED_PON_STATUS_FORCEMODE_SET(forceMode);
}
#endif
    return RT_ERR_OK;
}

static int32 pon_led_gpon_spec_type_00(rtk_pon_led_pon_state_t ponStatus)
{
#ifdef CONFIG_SDK_KERNEL_LINUX
    static char init = 0;
    int32 ret;
	rtk_led_force_mode_t forceMode;
	rtk_led_blink_rate_t rate = 0;

    RT_PARAM_CHK((PON_LED_STATE_END <= ponStatus), RT_ERR_INPUT);

    /* Init all used LED groups */
    if(0 == init)
    {
        ret = pon_led_gpon_spec_type_00_init();
        if(ret)
            return ret;

        init = 1;
    }

	switch(ponStatus)
	{
	case PON_LED_STATE_O1:
		forceMode = LED_FORCE_OFF;
        PON_LED_PON_STATUS_FORCEMODE_SET(forceMode);
		break;
	case PON_LED_STATE_O5:
		forceMode = LED_FORCE_ON;
        PON_LED_PON_STATUS_FORCEMODE_SET(forceMode);
		break;
    case PON_LED_STATE_O2:
    case PON_LED_STATE_O3:
    case PON_LED_STATE_O4:
		forceMode = LED_FORCE_BLINK;
		rate = LED_BLINKRATE_512MS;
        PON_LED_PON_STATUS_FORCEMODE_SET(forceMode);
		if(RT_ERR_OK != rtk_led_blinkRate_set(LED_BLINK_GROUP_FORCE_MODE, rate))
			return ret;
		break;
	default:
        /* Don't know what to do for unknow case, don't do anything */
		break;
	}

	return ret;
#else
    ponStatus = ponStatus;

    return RT_ERR_OK;
#endif
}

static int pon_led_gpon_spec_type_01_init(void)
{
#ifdef PON_LED_GROUP_PON_STATUS
{
    int ret;
    rtk_led_config_t ledConfig;
	rtk_led_force_mode_t forceMode;

	if((ret = rtk_led_parallelEnable_set(PON_LED_GROUP_PON_STATUS, ENABLED)) != RT_ERR_OK)
		return ret;

	osal_memset(&ledConfig,0x0,sizeof(rtk_led_config_t));
	ledConfig.ledEnable[LED_CONFIG_FORCE_MODE] = 1;
	if((ret = rtk_led_config_set(PON_LED_GROUP_PON_STATUS, LED_TYPE_PON, &ledConfig)) != RT_ERR_OK)
		return ret;

	forceMode = LED_FORCE_OFF;
    PON_LED_PON_STATUS_FORCEMODE_SET(forceMode);
}
#endif
    return RT_ERR_OK;
}

static int32 pon_led_gpon_spec_type_01(rtk_pon_led_pon_state_t ponStatus)
{
#ifdef CONFIG_SDK_KERNEL_LINUX
    static char init = 0;
    int32 ret;
	rtk_led_force_mode_t forceMode;
	rtk_led_blink_rate_t rate = 0;

    RT_PARAM_CHK((PON_LED_STATE_END <= ponStatus), RT_ERR_INPUT);

    /* Init all used LED groups */
    if(0 == init)
    {
        ret = pon_led_gpon_spec_type_01_init();
        if(ret)
            return ret;

        init = 1;
    }

	switch(ponStatus)
	{
	case PON_LED_STATE_O1:
		forceMode = LED_FORCE_OFF;
        PON_LED_PON_STATUS_FORCEMODE_SET(forceMode);
		break;
	case PON_LED_STATE_O5:
		forceMode = LED_FORCE_ON;
        PON_LED_PON_STATUS_FORCEMODE_SET(forceMode);
		break;
    case PON_LED_STATE_O2:
    case PON_LED_STATE_O3:
    case PON_LED_STATE_O4:
		forceMode = LED_FORCE_BLINK;
		rate = LED_BLINKRATE_512MS;
        PON_LED_PON_STATUS_FORCEMODE_SET(forceMode);
		if(RT_ERR_OK != rtk_led_blinkRate_set(LED_BLINK_GROUP_FORCE_MODE, rate))
			return ret;
		break;
	default:
        /* Don't know what to do for unknow case, don't do anything */
		break;
	}

	return ret;
#else
    ponStatus = ponStatus;

    return RT_ERR_OK;
#endif
}

static int pon_led_epon_spec_type_00_init(void)
{
#ifdef PON_LED_GROUP_PON_STATUS
{
    int ret;
    rtk_led_config_t ledConfig;
	rtk_led_force_mode_t forceMode;

	if((ret = rtk_led_parallelEnable_set(PON_LED_GROUP_PON_STATUS, ENABLED)) != RT_ERR_OK)
		return ret;

	osal_memset(&ledConfig,0x0,sizeof(rtk_led_config_t));
	ledConfig.ledEnable[LED_CONFIG_FORCE_MODE] = 1;
	ledConfig.ledEnable[LED_CONFIG_RX_ACT] = 1;
	ledConfig.ledEnable[LED_CONFIG_TX_ACT] = 1;
	if((ret = rtk_led_config_set(PON_LED_GROUP_PON_STATUS, LED_TYPE_PON, &ledConfig)) != RT_ERR_OK)
		return ret;

	forceMode = LED_FORCE_OFF;
    PON_LED_PON_STATUS_FORCEMODE_SET(forceMode);
}
#endif
    return RT_ERR_OK;
}

static int32 pon_led_epon_spec_type_00(rtk_pon_led_pon_state_t ponStatus)
{
#ifdef CONFIG_SDK_KERNEL_LINUX
    static char init = 0;
    int32 ret;
	rtk_led_force_mode_t forceMode;

    RT_PARAM_CHK((PON_LED_STATE_END <= ponStatus), RT_ERR_INPUT);

    /* Init all used LED groups */
    if(0 == init)
    {
        ret = pon_led_epon_spec_type_00_init();
        if(ret)
            return ret;

        init = 1;
    }

	switch(ponStatus)
	{
	case PON_LED_STATE_EPONMAC_DOWN:
		forceMode = LED_FORCE_OFF;
        PON_LED_PON_STATUS_FORCEMODE_SET(forceMode);
		break;
	case PON_LED_STATE_EPONMAC_UP:
		forceMode = LED_FORCE_ON;
        PON_LED_PON_STATUS_FORCEMODE_SET(forceMode);
		break;
	default:
        /* Don't know what to do for unknow case, don't do anything */
		break;
	}

	return ret;
#else
    ponStatus = ponStatus;

    return RT_ERR_OK;
#endif
}

static int pon_led_epon_spec_type_01_init(void)
{
#ifdef PON_LED_GROUP_PON_STATUS
{
    int ret;
    rtk_led_config_t ledConfig;
	rtk_led_force_mode_t forceMode;

	if((ret = rtk_led_parallelEnable_set(PON_LED_GROUP_PON_STATUS, ENABLED)) != RT_ERR_OK)
		return ret;

	osal_memset(&ledConfig,0x0,sizeof(rtk_led_config_t));
	ledConfig.ledEnable[LED_CONFIG_FORCE_MODE] = 1;
	if((ret = rtk_led_config_set(PON_LED_GROUP_PON_STATUS, LED_TYPE_PON, &ledConfig)) != RT_ERR_OK)
		return ret;

	forceMode = LED_FORCE_OFF;
    PON_LED_PON_STATUS_FORCEMODE_SET(forceMode);
}
#endif
    return RT_ERR_OK;
}

static int32 pon_led_epon_spec_type_01(rtk_pon_led_pon_state_t ponStatus)
{
#ifdef CONFIG_SDK_KERNEL_LINUX
    static char init = 0;
    int32 ret;
	rtk_led_force_mode_t forceMode;
	rtk_led_blink_rate_t rate = 0;

    RT_PARAM_CHK((PON_LED_STATE_END <= ponStatus), RT_ERR_INPUT);

    /* Init all used LED groups */
    if(0 == init)
    {
        ret = pon_led_epon_spec_type_01_init();
        if(ret)
            return ret;

        init = 1;
    }

	switch(ponStatus)
	{
	case PON_LED_STATE_EPONOAM_DOWN:
		forceMode = LED_FORCE_OFF;
        PON_LED_PON_STATUS_FORCEMODE_SET(forceMode);
		break;
	case PON_LED_STATE_EPONOAM_UP:
		forceMode = LED_FORCE_ON;
        PON_LED_PON_STATUS_FORCEMODE_SET(forceMode);
		break;
	case PON_LED_STATE_EPONOAM_TRYING:
		forceMode = LED_FORCE_BLINK;
		rate = LED_BLINKRATE_512MS;
        PON_LED_PON_STATUS_FORCEMODE_SET(forceMode);
		if(RT_ERR_OK != rtk_led_blinkRate_set(LED_BLINK_GROUP_FORCE_MODE, rate))
			return ret;
		break;
	default:
        /* Don't know what to do for unknow case, don't do anything */
		break;
	}

	return ret;
#else
    ponStatus = ponStatus;

    return RT_ERR_OK;
#endif
}

static int pon_led_others_spec_type_00_init(void)
{
#ifdef PON_LED_GROUP_LOS
{
    int ret;
    rtk_led_config_t ledConfig;
	rtk_led_force_mode_t forceMode;

	if((ret = rtk_led_parallelEnable_set(PON_LED_GROUP_LOS, ENABLED)) != RT_ERR_OK)
		return ret;

	osal_memset(&ledConfig,0x0,sizeof(rtk_led_config_t));
	ledConfig.ledEnable[LED_CONFIG_FORCE_MODE] = 1;
	if((ret = rtk_led_config_set(PON_LED_GROUP_LOS, LED_TYPE_PON, &ledConfig)) != RT_ERR_OK)
		return ret;

	forceMode = LED_FORCE_OFF;
    PON_LED_LOS_FORCEMODE_SET(forceMode);
}
#endif
    return RT_ERR_OK;
}

static int32 pon_led_others_spec_type_00(rtk_pon_led_pon_state_t ponStatus)
{
#ifdef CONFIG_SDK_KERNEL_LINUX
    static char init = 0;
    int32 ret;
	rtk_led_force_mode_t forceMode;
	rtk_led_blink_rate_t rate = 0;

    RT_PARAM_CHK((PON_LED_STATE_END <= ponStatus), RT_ERR_INPUT);

    /* Init all used LED groups */
    if(0 == init)
    {
        ret = pon_led_others_spec_type_00_init();
        if(ret)
            return ret;

        init = 1;
    }

	switch(ponStatus)
	{
	case PON_LED_STATE_LOS_OCCURRED:
		forceMode = LED_FORCE_BLINK;
		rate = LED_BLINKRATE_256MS;
        PON_LED_LOS_FORCEMODE_SET(forceMode);
		if(RT_ERR_OK != rtk_led_blinkRate_set(LED_BLINK_GROUP_FORCE_MODE, rate))
			return ret;
		break;
	case PON_LED_STATE_LOS_CLEARED:
		forceMode = LED_FORCE_OFF;
        PON_LED_LOS_FORCEMODE_SET(forceMode);
		break;
	default:
        /* Don't know what to do for unknow case, don't do anything */
		break;
	}

	return ret;
#else
    ponStatus = ponStatus;

    return RT_ERR_OK;
#endif
}

static int pon_led_others_spec_type_01_init(void)
{
#ifdef PON_LED_GROUP_LOS
{
    int ret;
    rtk_led_config_t ledConfig;
	rtk_led_force_mode_t forceMode;

	if((ret = rtk_led_parallelEnable_set(PON_LED_GROUP_LOS, ENABLED)) != RT_ERR_OK)
		return ret;

	osal_memset(&ledConfig,0x0,sizeof(rtk_led_config_t));
	ledConfig.ledEnable[LED_CONFIG_FORCE_MODE] = 1;
	if((ret = rtk_led_config_set(PON_LED_GROUP_LOS, LED_TYPE_PON, &ledConfig)) != RT_ERR_OK)
		return ret;

	forceMode = LED_FORCE_OFF;
    PON_LED_LOS_FORCEMODE_SET(forceMode);
}
#endif
#ifdef PON_LED_GROUP_SYSTEM
{
    int ret;
    rtk_led_config_t ledConfig;
	rtk_led_force_mode_t forceMode;

	if((ret = rtk_led_parallelEnable_set(PON_LED_GROUP_SYSTEM, ENABLED)) != RT_ERR_OK)
		return ret;

	osal_memset(&ledConfig,0x0,sizeof(rtk_led_config_t));
	ledConfig.ledEnable[LED_CONFIG_FORCE_MODE] = 1;
	if((ret = rtk_led_config_set(PON_LED_GROUP_SYSTEM, LED_TYPE_PON, &ledConfig)) != RT_ERR_OK)
		return ret;

	forceMode = LED_FORCE_OFF;
    PON_LED_SYSTEM_FORCEMODE_SET(forceMode);
}
#endif
    return RT_ERR_OK;
}

static int32 pon_led_others_spec_type_01(rtk_pon_led_pon_state_t ponStatus)
{
#ifdef CONFIG_SDK_KERNEL_LINUX
    static char init = 0;
    static char cnt = 0, toggle = 0;
    int32 ret;
	rtk_led_force_mode_t forceMode;

    RT_PARAM_CHK((PON_LED_STATE_END <= ponStatus), RT_ERR_INPUT);

    /* Init all used LED groups */
    if(0 == init)
    {
        ret = pon_led_others_spec_type_01_init();
        if(ret)
            return ret;

        init = 1;
    }

	switch(ponStatus)
	{
	case PON_LED_STATE_LOS_OCCURRED:
		forceMode = LED_FORCE_ON;
        PON_LED_LOS_FORCEMODE_SET(forceMode);
		break;
	case PON_LED_STATE_LOS_CLEARED:
		forceMode = LED_FORCE_OFF;
        PON_LED_LOS_FORCEMODE_SET(forceMode);
		break;
    case PON_LED_STATE_TIMER_50MS:
        cnt++;
        if(cnt >= 20) /* blink every second: 20 * 50ms */
        {
            toggle = (toggle + 1) % 2;
            forceMode = (toggle == 1) ? LED_FORCE_ON : LED_FORCE_OFF;
            PON_LED_SYSTEM_FORCEMODE_SET(forceMode);
            cnt = 0;
        }
        break;
	default:
        /* Don't know what to do for unknow case, don't do anything */
		break;
	}

	return ret;
#else
    ponStatus = ponStatus;

    return RT_ERR_OK;
#endif
}
#ifdef CONFIG_SDK_KERNEL_LINUX
static int led_swBlink_thread(void *data)
{
    static rtk_enable_t lastLosState = RTK_ENABLE_END;
    int ret;
    rtk_enable_t losState;

    while(!kthread_should_stop())
    {
        /* It is NOT possible to wake up earlier */
        set_current_state(TASK_UNINTERRUPTIBLE);
        schedule_timeout(5 * HZ / 100); /* Wake up every 50 ms*/

        rtk_pon_led_status_set(PON_LED_PON_MODE_OTHERS, PON_LED_STATE_TIMER_50MS);

        if((ret = rtk_ponmac_losState_get(&losState)) == RT_ERR_OK)
        {
            if(lastLosState != losState)
            {
                /* LOS state changed or first time execute */
                if(ENABLED == losState)
                {
                    rtk_pon_led_status_set(PON_LED_PON_MODE_OTHERS, PON_LED_STATE_LOS_OCCURRED);
                }
                else
                {
                    rtk_pon_led_status_set(PON_LED_PON_MODE_OTHERS, PON_LED_STATE_LOS_CLEARED);
                }
                lastLosState = losState;
            }
        }
    }

    return 0;
}
#endif

/* Function Name:
 *      rtk_pon_led_SpecType_set
 * Description:
 *      Set pon port spec type
 * Input:
 *      type   			- PON port LED spec type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 */
int32
rtk_pon_led_SpecType_set(rtk_pon_led_spec_type_t type)
{
    RT_PARAM_CHK((PON_LED_SPEC_TYPE_END <= type), RT_ERR_INPUT);
    /* function body */
    pon_led_spec_type = type;

#ifdef CONFIG_SDK_KERNEL_LINUX
    if(NULL == pLedBlink)
    {
        pLedBlink = kthread_create(led_swBlink_thread, NULL, "led_swBlink");
        if(IS_ERR(pLedBlink))
        {
            printk("%s:%d led sw blink init failed %ld!\n", __FILE__, __LINE__, PTR_ERR(pLedBlink));
            pLedBlink = NULL;
        }
        else
        {
            wake_up_process(pLedBlink);
            printk("%s:%d led sw blink init complete!\n", __FILE__, __LINE__);
        }
    }
#endif

    return RT_ERR_OK;
}   /* end of rtk_pon_led_SpecType_set */

/* Function Name:
 *      rtk_pon_led_status_set
 * Description:
 *      Set GPON/EPON pon port LED by pon status
 * Input:
 *      ponMode   			- GPON or EPON
 *      ponStatus           - GPON/EPON PON status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 */
int32
rtk_pon_led_status_set(rtk_pon_led_pon_mode_t ponMode, rtk_pon_led_pon_state_t ponStatus)
{
    int32   ret=RT_ERR_OK;

    /* function body */
    switch(ponMode)
    {
    case PON_LED_PON_MODE_GPON:
        switch(pon_led_spec_type)
        {
        case PON_LED_SPEC_TYPE_01:
            ret = pon_led_gpon_spec_type_01(ponStatus);
            break;

        case PON_LED_SPEC_TYPE_02:
            break;

        case PON_LED_SPEC_TYPE_03:
            break;

        case PON_LED_SPEC_TYPE_00:
        default:
            ret = pon_led_gpon_spec_type_00(ponStatus);
            break;
        }
        break;

    case PON_LED_PON_MODE_EPON:
        switch(pon_led_spec_type)
        {
        case PON_LED_SPEC_TYPE_01:
            ret = pon_led_epon_spec_type_01(ponStatus);
            break;

        case PON_LED_SPEC_TYPE_02:
            break;

        case PON_LED_SPEC_TYPE_03:
            break;

        case PON_LED_SPEC_TYPE_00:
        default:
            ret = pon_led_epon_spec_type_00(ponStatus);
            break;
        }
        break;

    case PON_LED_PON_MODE_OTHERS:
        switch(pon_led_spec_type)
        {
        case PON_LED_SPEC_TYPE_01:
            ret = pon_led_others_spec_type_01(ponStatus);
            break;

        case PON_LED_SPEC_TYPE_02:
            break;

        case PON_LED_SPEC_TYPE_03:
            break;

        case PON_LED_SPEC_TYPE_00:
        default:
            ret = pon_led_others_spec_type_00(ponStatus);
            break;
        }
        break;

    default:
        ret = RT_ERR_INPUT;
        break;
    }

    return ret;
}   /* end of rtk_pon_led_status_set */

