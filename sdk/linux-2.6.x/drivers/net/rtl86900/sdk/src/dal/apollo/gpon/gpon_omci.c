/*
 * Copyright (C) 2011 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 *
 *
 * $Revision: 63949 $
 * $Date: 2015-12-08 17:48:39 +0800 (Tue, 08 Dec 2015) $
 *
 * Purpose : GMac Driver OMCI Processor
 *
 * Feature : GMac Driver OMCI Processor
 *
 */

#if defined(CONFIG_SDK_KERNEL_LINUX)
#include <linux/skbuff.h>
#include <linux/interrupt.h>
#endif

#include <dal/apollo/gpon/gpon_defs.h>
#include <dal/apollo/gpon/gpon_omci.h>
#include <dal/apollo/gpon/gpon_debug.h>
#include <dal/apollo/raw/apollo_raw_gpon.h>

#if defined(CONFIG_SDK_KERNEL_LINUX)
#if defined(OLD_FPGA_DEFINED)
#include <rtk/cpu_tag.h>
extern void apollo_nic_tx(struct sk_buff *skb);
#else
#if defined(CONFIG_RTL9600_SERIES)
#include "re8686.h"
#endif
#if defined(CONFIG_RTL9601B_SERIES)
#include "re8686_rtl9601b.h"
#endif
#if defined(CONFIG_RTL9602C_SERIES)
#include "re8686_rtl9602c.h"
#endif
#include <rtk/switch.h>
#endif
#endif

#if defined(OLD_FPGA_DEFINED)
int32 gpon_omci_rx_reg(gpon_dev_obj_t* obj)
{
    rtk_gpon_omci_msg_t omci;
    osal_memset(&omci,0,sizeof(rtk_gpon_omci_msg_t));

    while(apollo_raw_gpon_omci_read(omci.msg)==RT_ERR_OK)
    {
#if 0
        if(obj->status!=RTK_GPONMAC_FSM_STATE_O5 && obj->status!=RTK_GPONMAC_FSM_STATE_O6)
        {
            GPON_OS_Log(GPON_LOG_LEVEL_WARNING,"Rcv OMCI in state %s", gpon_dbg_fsm_status_str(obj->status));
        }
        else
        {
            obj->cnt_cpu_omci_rx++;
            GPON_OS_Log(GPON_LOG_LEVEL_OMCI,"Rcv OMCI: %02x%02x %02x %02x %02x%02x%02x%02x ",
                omci.msg[0],omci.msg[1],omci.msg[2],omci.msg[3],omci.msg[4],omci.msg[5],omci.msg[6],omci.msg[7]);
            GPON_OS_Log(GPON_LOG_LEVEL_OMCI,"%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x",
                omci.msg[8],omci.msg[9],omci.msg[10],omci.msg[11],omci.msg[12],omci.msg[13],omci.msg[14],omci.msg[15],
                omci.msg[16],omci.msg[17],omci.msg[18],omci.msg[19],omci.msg[20],omci.msg[21],omci.msg[22],omci.msg[23]);
            GPON_OS_Log(GPON_LOG_LEVEL_OMCI,"%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x",
                omci.msg[24],omci.msg[25],omci.msg[26],omci.msg[27],omci.msg[28],omci.msg[29],omci.msg[30],omci.msg[31],
                omci.msg[32],omci.msg[33],omci.msg[34],omci.msg[35],omci.msg[36],omci.msg[37],omci.msg[38],omci.msg[39]);
            GPON_OS_Log(GPON_LOG_LEVEL_OMCI,"%02x%02x %02x%02x %02x%02x%02x%02x ",
                omci.msg[40],omci.msg[41],omci.msg[42],omci.msg[43],omci.msg[44],omci.msg[45],omci.msg[46],omci.msg[47] );
            GPON_OS_Log(GPON_LOG_LEVEL_OMCI,"%02x%02x%02x%02x ",
                omci.msg[48],omci.msg[49],omci.msg[50],omci.msg[51]);
            if(obj->omci_callback)
            {
                (*obj->omci_callback)(&omci);
            }
        }
#endif
    }

    return RT_ERR_OK;
}
#endif

int32 gpon_omci_tx(gpon_dev_obj_t* obj, rtk_gpon_omci_msg_t* omci)
{
#if defined(CONFIG_SDK_KERNEL_LINUX)
    rtk_gpon_omci_msg_t *tmp_omci;
    struct sk_buff *skb;
  	int32 length = sizeof(rtk_gpon_omci_msg_t);
    int32 pon_port;
#endif
    int32 ret=0;

    if(obj->us_omci_flow!=GPON_DEV_MAX_FLOW_NUM)
    {
        GPON_OS_Log(GPON_LOG_LEVEL_OMCI,"Send OMCI [port %d]: %02x%02x %02x %02x %02x%02x%02x%02x [baseaddr:%p]",
            obj->us_omci_flow,omci->msg[0],omci->msg[1],omci->msg[2],omci->msg[3],omci->msg[4],omci->msg[5],omci->msg[6],omci->msg[7],obj->base_addr);
        GPON_OS_Log(GPON_LOG_LEVEL_OMCI,"%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x",
            omci->msg[8],omci->msg[9],omci->msg[10],omci->msg[11],omci->msg[12],omci->msg[13],omci->msg[14],omci->msg[15],
            omci->msg[16],omci->msg[17],omci->msg[18],omci->msg[19],omci->msg[20],omci->msg[21],omci->msg[22],omci->msg[23]);
        GPON_OS_Log(GPON_LOG_LEVEL_OMCI,"%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x",
            omci->msg[24],omci->msg[25],omci->msg[26],omci->msg[27],omci->msg[28],omci->msg[29],omci->msg[30],omci->msg[31],
            omci->msg[32],omci->msg[33],omci->msg[34],omci->msg[35],omci->msg[36],omci->msg[37],omci->msg[38],omci->msg[39]);
        GPON_OS_Log(GPON_LOG_LEVEL_OMCI,"%02x%02x %02x%02x %02x%02x%02x%02x ",
            omci->msg[40],omci->msg[41],omci->msg[42],omci->msg[43],omci->msg[44],omci->msg[45],omci->msg[46],omci->msg[47] );

#if defined(CONFIG_SDK_KERNEL_LINUX)
    	skb = dev_alloc_skb(length);
    	if (!skb)
    		return RT_ERR_FAILED;

    	tmp_omci = (rtk_gpon_omci_msg_t *)skb_put(skb, length);
        osal_memcpy(tmp_omci->msg, omci->msg, RTK_GPON_OMCI_MSG_LEN);
  #if defined(OLD_FPGA_DEFINED)
        skb->tx_tag.cpu_tag = 1;
        skb->tx_tag.keep = 0;
        skb->tx_tag.v_sel = 0;
        skb->tx_tag.dis_lrn = 0;
        skb->tx_tag.cpu_l3cs = 0;
        skb->tx_tag.cpu_l4cs = 0;
        skb->tx_tag.as_pri = 0;
        skb->tx_tag.cpu_pri = 0;
        skb->tx_tag.efid = 1;
        skb->tx_tag.ehan_fid = 0;
        skb->tx_tag.tx_port_mask = (1 << 3);
        skb->tx_tag.tx_pon_sid = GPON_OMCI_FLOW_ID;

       ret = apollo_nic_tx(skb);
        //dev_kfree_skb_any(skb);
  #else
      {
          struct tx_info ptxInfo;
          osal_memset(&ptxInfo, 0, sizeof(struct tx_info));

          rtk_switch_phyPortId_get(RTK_PORT_PON, &pon_port);

          ptxInfo.opts1.bit.cputag_psel = 1;
          ptxInfo.opts1.bit.dislrn = 1;
          ptxInfo.opts1.bit.keep = 1;
          ptxInfo.opts2.bit.cputag  = 1;
          ptxInfo.opts2.bit.efid  = 1;
          ptxInfo.opts2.bit.enhance_fid  = 0;
          ptxInfo.opts3.bit.tx_portmask  = (1 << pon_port);
          ptxInfo.opts3.bit.tx_dst_stream_id  = GPON_OMCI_FLOW_ID;

          ret = re8686_send_with_txInfo(skb, &ptxInfo, 0);
      }
  #endif
#endif
        if(0 == ret)
        {
            obj->cnt_cpu_omci_tx++;
            obj->cnt_cpu_omci_tx_nor++;
        }
        else
        {
        }
        return ret;
    }
    else
    {
        return RT_ERR_INPUT;
    }
}

int32 gpon_omci_rx(gpon_dev_obj_t* obj, rtk_gpon_omci_msg_t *omci)
{
    if(obj->status!=RTK_GPONMAC_FSM_STATE_O5 && obj->status!=RTK_GPONMAC_FSM_STATE_O6)
    {
        GPON_OS_Log(GPON_LOG_LEVEL_WARNING,"Rcv OMCI in state %s", gpon_dbg_fsm_status_str(obj->status));
        return RT_ERR_OUT_OF_RANGE;
    }
    else
    {
        obj->cnt_cpu_omci_rx++;
        GPON_OS_Log(GPON_LOG_LEVEL_OMCI,"Rcv OMCI: %02x%02x %02x %02x %02x%02x%02x%02x ",
            omci->msg[0],omci->msg[1],omci->msg[2],omci->msg[3],omci->msg[4],omci->msg[5],omci->msg[6],omci->msg[7]);
        GPON_OS_Log(GPON_LOG_LEVEL_OMCI,"%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x",
            omci->msg[8],omci->msg[9],omci->msg[10],omci->msg[11],omci->msg[12],omci->msg[13],omci->msg[14],omci->msg[15],
            omci->msg[16],omci->msg[17],omci->msg[18],omci->msg[19],omci->msg[20],omci->msg[21],omci->msg[22],omci->msg[23]);
        GPON_OS_Log(GPON_LOG_LEVEL_OMCI,"%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x",
            omci->msg[24],omci->msg[25],omci->msg[26],omci->msg[27],omci->msg[28],omci->msg[29],omci->msg[30],omci->msg[31],
            omci->msg[32],omci->msg[33],omci->msg[34],omci->msg[35],omci->msg[36],omci->msg[37],omci->msg[38],omci->msg[39]);
        GPON_OS_Log(GPON_LOG_LEVEL_OMCI,"%02x%02x%02x%02x",
            omci->msg[40],omci->msg[41],omci->msg[42],omci->msg[43]);

        if(obj->omci_callback)
        {
            (*obj->omci_callback)(omci);
        }
    }
    return RT_ERR_OK;
}
