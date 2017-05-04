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
 * Purpose : Main function of SFU IGMP Application
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) IGMP configuration APIs
 */

/*
* Include Files
*/

#include <igmp/inc/proto_igmp.h>

static uint32 current_interval_index;
uint32        **pp_rx_counter_per_interval;
uint32        *p_igmp_us_rate_limit;

static void igmp_rate_calculate(int sig)
{
	uint32 new_interval_index;
	uint32 port;

	IGMP_DB_SEM_LOCK();
	new_interval_index = (current_interval_index + 1) % RATE_CHECKTIME_NUM;

	for (port = 0; port < igmpCtrl.sys_max_port_num - 2; port++)
	{
		/*If port no rate limit, continue to check other port*/
		if (0 == p_igmp_us_rate_limit[port])
			continue;

		pp_rx_counter_per_interval[port][current_interval_index] = p_rx_counter_current[port];

		/*new_interval_index is also the oldest interval_index*/
		p_rx_counter[port] = p_rx_counter[port] + pp_rx_counter_per_interval[port][current_interval_index] -
		                    pp_rx_counter_per_interval[port][new_interval_index];

		p_remain_packets[port] = p_igmp_us_rate_limit[port] - p_rx_counter[port];

		p_rx_counter_current[port] = 0;
	}

	current_interval_index = new_interval_index;
	IGMP_DB_SEM_UNLOCK();
}

static void mcast_igmp_rate_limit_thread(void *arg)
{
	uint32              ret, i;
	struct itimerval    tick;

	prctl(PR_SET_NAME, (unsigned long)((char *)arg));

    /*reset all value*/
	current_interval_index = 0;

	osal_memset(p_rx_counter_current, 0, sizeof(uint32) * (igmpCtrl.sys_max_port_num - 2));

	osal_memset(p_remain_packets, 0, sizeof(uint32) * (igmpCtrl.sys_max_port_num - 2));

	osal_memset(p_rx_counter, 0x0, sizeof(uint32) * (igmpCtrl.sys_max_port_num - 2));

    for (i = 0; i < igmpCtrl.sys_max_port_num - 2; i++)
    {
        osal_memset(&(pp_rx_counter_per_interval[i][0]), 0, sizeof(uint32) * RATE_CHECKTIME_NUM);
    }

	/*Set timer and signal*/
	signal(SIGALRM, igmp_rate_calculate);

	memset(&tick, 0, sizeof(tick));
	tick.it_value.tv_sec        = 0;
	tick.it_value.tv_usec       = RATE_CHECKTIME_UNIT;
	tick.it_interval.tv_sec     = 0;
	tick.it_interval.tv_usec    = RATE_CHECKTIME_UNIT;

	ret = setitimer(ITIMER_REAL, &tick, NULL);

  	while (1)
		pause();

}

int32 igmp_mld_rate_limit_timer_init(void)
{
    uint32  i;
    /* igmp rate resource alloc */
    if (!(p_mcast_total_rx_counter = (uint32 *)osal_alloc(sizeof(uint32) * (igmpCtrl.sys_max_port_num - 2))))
        goto alloc_failed;
	osal_memset(p_mcast_total_rx_counter, 0, sizeof(uint32) * (igmpCtrl.sys_max_port_num - 2));

    if (!(p_mcast_rx_drop_counter = (uint32 *)osal_alloc(sizeof(uint32) * (igmpCtrl.sys_max_port_num - 2))))
        goto alloc_failed;
	osal_memset(p_mcast_rx_drop_counter, 0x0, sizeof(uint32) * (igmpCtrl.sys_max_port_num - 2));

    if (!(p_igmp_us_rate_limit = (uint32 *)osal_alloc(sizeof(uint32) * (igmpCtrl.sys_max_port_num - 2))))
        goto alloc_failed;
	osal_memset(p_igmp_us_rate_limit, 0x0, sizeof(uint32) * (igmpCtrl.sys_max_port_num - 2));


    if (!(p_rx_counter_current = (uint32 *)osal_alloc(sizeof(uint32) * (igmpCtrl.sys_max_port_num - 2))))
        goto alloc_failed;
	osal_memset(p_rx_counter_current, 0, sizeof(uint32) * (igmpCtrl.sys_max_port_num - 2));

    if (!(p_remain_packets = (uint32 *)osal_alloc(sizeof(uint32) * (igmpCtrl.sys_max_port_num - 2))))
        goto alloc_failed;
	osal_memset(p_remain_packets, 0, sizeof(uint32) * (igmpCtrl.sys_max_port_num - 2));

    if (!( p_rx_counter = (uint32 *)osal_alloc(sizeof(uint32) * (igmpCtrl.sys_max_port_num - 2))))
        goto alloc_failed;
	osal_memset(p_rx_counter, 0x0, sizeof(uint32) * (igmpCtrl.sys_max_port_num - 2));

    if (!(pp_rx_counter_per_interval = (uint32 **)osal_alloc(sizeof(uint32 *) * (igmpCtrl.sys_max_port_num - 2))))
        goto alloc_failed;

    for (i = 0; i < igmpCtrl.sys_max_port_num - 2; i++)
    {
        if (!(pp_rx_counter_per_interval[i] = (uint32 *)osal_alloc(sizeof(uint32) * RATE_CHECKTIME_NUM)))
            goto alloc_failed;
        osal_memset(pp_rx_counter_per_interval[i], 0, sizeof(uint32) * RATE_CHECKTIME_NUM);
    }

    if ((osal_thread_t)NULL == (igmp_thread_create("IGMP RateLimit Thread", STACK_SIZE,
        SYS_THREAD_PRI_IGMP_RATE, (void *)mcast_igmp_rate_limit_thread, "IGMP_RATE")))
    {
        SYS_PRINTF("IGMP Tick Thread create failed\n");
        return SYS_ERR_FAILED;
    }
    return SYS_ERR_OK;

alloc_failed:

    if (p_mcast_total_rx_counter)
        osal_free(p_mcast_total_rx_counter);
    if (p_mcast_rx_drop_counter)
        osal_free(p_mcast_rx_drop_counter);
    if (p_rx_counter_current)
        osal_free(p_rx_counter_current);
    if (p_remain_packets)
        osal_free(p_remain_packets);
    if (p_rx_counter)
        osal_free(p_rx_counter);

    for (i = 0; i < igmpCtrl.sys_max_port_num - 2; i++)
    {
        if (pp_rx_counter_per_interval[i])
            osal_free(pp_rx_counter_per_interval[i]);
    }

    if (pp_rx_counter_per_interval)
        osal_free(pp_rx_counter_per_interval);

    return SYS_ERR_FAILED;
}
