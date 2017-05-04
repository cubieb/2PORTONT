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
#if defined(CONFIG_RTL9600_SERIES)
#include "re8686.h"
#endif
#if defined(CONFIG_RTL9601B_SERIES)
#include "re8686_rtl9601b.h"
#endif
#if defined(CONFIG_RTL9602C_SERIES)
#include "re8686_rtl9602c.h"
#endif

#endif

#include <module/gpon/gpon_defs.h>
#include <module/gpon/gpon_omci.h>
#include <module/gpon/gpon_debug.h>
#include <module/gpon/gpon.h>

extern int re8686_tx_with_Info(unsigned char *pPayload, unsigned short length, void *pInfo);

int32 gpon_omci_tx(gpon_dev_obj_t* obj, rtk_gpon_omci_msg_t* omci)
{
    int32 ret=0;
    rtk_port_t ponPort;

    rtk_gpon_port_get(&ponPort);

    if(obj->us_omci_flow!=obj->scheInfo.max_flow)
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
      {
          struct tx_info ptxInfo;
          osal_memset(&ptxInfo, 0, sizeof(struct tx_info));

          ptxInfo.opts1.bit.cputag_psel = 1;
          ptxInfo.opts1.bit.dislrn = 1;
          ptxInfo.opts1.bit.keep = 1;
          ptxInfo.opts2.bit.cputag  = 1;
          ptxInfo.opts2.bit.efid  = 1;
          ptxInfo.opts2.bit.enhance_fid  = 0;
          ptxInfo.opts3.bit.tx_portmask  = (1 << ponPort);
          ptxInfo.opts3.bit.tx_dst_stream_id  = obj->scheInfo.omcc_flow;

          ret = re8686_tx_with_Info(omci->msg, RTK_GPON_OMCI_MSG_LEN_TX, &ptxInfo);
      }
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
    	osal_printf("flow_id=%d\n",obj->us_omci_flow);
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
