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
 * Purpose : For linux kernel mode
 *           MAC debug APIs in the SDK
 *
 * Feature : MAC debug APIs
 *
 */

/*
 * Include Files
 */
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <common/error.h>
#include <common/util.h>
#include <rtk/rtusr/rtusr_pkt.h>
#include <linux/slab.h>

/*
 * Symbol Definition
 */

/*
 * Macro Declaration
 */
#define MAC_DEBUG_CONTINUOUSLY_TX_CNT       100

/*
 * Data Declaration
 */
static struct task_struct *pPktTxThread = NULL;
static uint8 *pktData = NULL;
static uint16 pktLen = 0;
static struct pkt_dbg_tx_info pktInfo;
static uint32 continueTxCnt = MAC_DEBUG_CONTINUOUSLY_TX_CNT;

/*
 * Function Declaration
 */
int mac_debug_pktTx_thread(void *data)
{
    int i;
    int re8686_tx_with_Info_dbg(unsigned char *pPayload, unsigned short length, void *pInfo);

    while(!kthread_should_stop())
    {
        /* It is possible to wake up earlier */
        set_current_state(TASK_INTERRUPTIBLE);
        schedule_timeout(1 * HZ / 1000);

        pktInfo.opts2.bit.cputag = 1;
        if(likely(pktData != NULL))
        {
            for(i = 0;i < continueTxCnt;i++)
            {
                re8686_tx_with_Info_dbg(pktData, pktLen, &pktInfo);
            }
        }
    }

    return 0;
}

/* Function Name:
 *      mac_debug_rxPkt_enable
 * Description:
 *      Due to the rx packet debug might impact the performance, this API is to
 *      enable/disable the rx packet debug function
 * Input:
 *      enable - enable or disable the rx packet storage
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 * Note:
 *      None
 */
 int32 
 mac_debug_rxPkt_enable(uint32 enable)
{
#if defined(CONFIG_SOC_DEPEND_FEATURE)
    int re8686_rx_pktDump_set(unsigned int enable);
    
    re8686_rx_pktDump_set(enable);
#endif

    return RT_ERR_OK;
}

/* Function Name:
 *      mac_debug_rxPkt_get
 * Description:
 *      Get the last received packet
 * Input:
 *      pPayload   - entire packet to be sent
 *      buf_length - specified the maximum length to be dumped
 *      pkt_length - get the actul length of packet
 *      pInfo      - rx descriptor of packet
 *      pEnable    - get the dump copy state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 * Note:
 *      Dump command always success
 */
int32 
mac_debug_rxPkt_get(uint8 *pPayload, uint16 buf_length, uint16 *pPkt_len, struct pkt_dbg_rx_info *pInfo, unsigned int *pEnable)
{
#if defined(CONFIG_SOC_DEPEND_FEATURE)
    int re8686_rx_pktDump_get(unsigned char *pPayload, unsigned short buf_length, unsigned short *pPkt_len, void *pInfo, unsigned int *pEnable);

    re8686_rx_pktDump_get(pPayload, buf_length, pPkt_len, pInfo, pEnable);
#endif

    return RT_ERR_OK;
}

/* Function Name:
 *      mac_debug_rxPktFifo_get
 * Description:
 *      Get the last received packet
 * Input:
 *      fifo_idx   - FIFO index to be dumped
 *      pPayload   - entire packet to be sent
 *      buf_length - specified the maximum length to be dumped
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 * Note:
 */
int32 
mac_debug_rxPktFifo_get(uint16 fifo_idx, uint8 *pPayload, uint16 buf_length)
{
#if defined(CONFIG_SOC_DEPEND_FEATURE)
    int re8686_rx_pktFifoDump_get(unsigned int fifo_idx, unsigned char *pPayload, unsigned short buf_length);

    return re8686_rx_pktFifoDump_get(fifo_idx, pPayload, buf_length);
#endif
}

/* Function Name:
 *      mac_debug_rxPkt_clear
 * Description:
 *      Clear the last received packet storage buffer
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 * Note:
 *      Clear command always success
 */
int32 
mac_debug_rxPkt_clear(void)
{
#if defined(CONFIG_SOC_DEPEND_FEATURE)
    int re8686_rx_pktDump_clear(void);
    
    re8686_rx_pktDump_clear();
#endif

    return RT_ERR_OK;
}

/* Function Name:
 *      mac_debug_txPkt_send
 * Description:
 *      Send the packet in the buffer
 * Input:
 *      pPayload - entire packet to be sent
 *      length   - packet length
 *      pInfo    - tx descriptor of packet
 *      count    - tx repeat count
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 * Note:
 *      Send command always success
 */
int32 
mac_debug_txPkt_send(uint8 *pPayload, uint16 length, struct pkt_dbg_tx_info *pInfo, uint32 count)
{
#if defined(CONFIG_SOC_DEPEND_FEATURE)
    int i;
    int re8686_tx_with_Info_dbg(unsigned char *pPayload, unsigned short length, void *pInfo);

    pInfo->opts2.bit.cputag = 1;
    for(i = 0; i < count; i ++)
    {
        re8686_tx_with_Info_dbg(pPayload, length, pInfo);
    }
#endif

    return RT_ERR_OK;
}

/* Function Name:
 *      mac_debug_txPktCont_send
 * Description:
 *      Send the packet in the buffer continuously
 * Input:
 *      pPayload - entire packet to be sent
 *      length   - packet length
 *      pInfo    - tx descriptor of packet
 *      enable   - enable/disable continous sending
 * Output:
 *      None
 * Return:
 *      Continue send command always success
 *      RT_ERR_OK
 */
int32 
mac_debug_txPktCont_send(uint8 *pPayload, uint16 length, struct pkt_dbg_tx_info *pInfo, uint8 enable)
{
#if defined(CONFIG_SOC_DEPEND_FEATURE)
    static uint8 state = 0;

    if(enable == state)
    {
        /* Woops? The same command twice? */
    }
    else
    {
        if(enable != 0)
        {
            /* 0 -> non-zero: start continuously packet xmit */
            if(pPktTxThread == NULL)
            {
                pPktTxThread = kthread_create(mac_debug_pktTx_thread, NULL, "dbgPktContTx");
            }

            if(IS_ERR(pPktTxThread))
            {
                printk("%s:%d mac_debug_pktTx_thread failed %ld!\n", __FILE__, __LINE__, PTR_ERR(pPktTxThread));
                return RT_ERR_OK;
            }
            else
            {
                if(pktData != NULL)
                {
                    kfree(pktData);
                }
                /* Thread create success, copy the necessary data for thread running */
                pktData = (uint8 *)kmalloc(sizeof(uint8) * length, GFP_KERNEL);
                /* If the malloc failed, NULL pointer will cause thread not sending anything */
                if(pktData)
                {
                    memcpy(pktData, pPayload, sizeof(uint8) * length);
                    pktLen = length;
                    pktInfo = *pInfo;
                    wake_up_process(pPktTxThread);
                }
                else
                {
                    /* Leave the created thread for next run and don't update the state */
                    return RT_ERR_OK;
                }
            }
        }
        else
        {
            /* non-zero -> 0: stop continuously packet xmit */
            if(pPktTxThread)
            {
                kthread_stop(pPktTxThread);
                pPktTxThread = NULL;
            }
            if(pktData)
            {
                kfree(pktData);
                pktData = NULL;
            }
        }

        /* state update */
        state = enable;
    }

#endif

    return RT_ERR_OK;
}

/* Function Name:
 *      mac_debug_txPktContCnt_set
 * Description:
 *      Set send count for each time sending thread wake up
 * Input:
 *      count    - tx repeat count
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 */
int32 
mac_debug_txPktContCnt_set(uint32 count)
{
    continueTxCnt = count;
    return RT_ERR_OK;
}

/* Function Name:
 *      mac_debug_txPktContCnt_get
 * Description:
 *      Get send count for each time sending thread wake up
 * Input:
 *      None
 * Output:
 *      pCount    - tx repeat count
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_INPUT
 */
int32 
mac_debug_txPktContCnt_get(uint32 *pCount)
{
    if(pCount != NULL)
    {
        *pCount = continueTxCnt;
        return RT_ERR_OK;
    }
    return RT_ERR_INPUT;
}

