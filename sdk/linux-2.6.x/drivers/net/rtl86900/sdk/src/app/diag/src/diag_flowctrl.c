 /*
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: 39101 $
 * $Date: 2013-05-03 17:35:27 +0800 (Fri, 03 May 2013) $
 *
 * Purpose : Definition those XXX command and APIs in the SDK diagnostic shell.
 *
 * Feature : The file have include the following module and sub-modules
 *
 */

/*
 * Include Files
 */
#include <stdio.h>
#include <string.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <diag_util.h>
#include <diag_str.h>
#include <parser/cparser_priv.h>
#if defined(CONFIG_SDK_APOLLO)
#include <dal/apollo/raw/apollo_raw_flowctrl.h>
#endif

#if defined(CONFIG_SDK_APOLLOMP)
#include <dal/apollomp/raw/apollomp_raw_flowctrl.h>
#endif
/*
 * flow control show rule
 */


/*
* flowctrl dump threshold  ( switch | pon )
*/
cparser_result_t
cparser_cmd_flowctrl_dump_threshold_switch_pon(
cparser_context_t *context)
{
    apollo_raw_flowctrl_type_t type;
    rtk_enable_t enable;
    apollo_raw_flowctrl_jumbo_size_t size;
    uint32 threshold, onThresh, offThresh;
    diag_portlist_t portlist;
    rtk_port_t port;
    diag_mask_t  queuelist;
    rtk_qid_t queue;
    int32 ret = RT_ERR_FAILED;
    diag_mask_t  idxlist;
    uint32 idx;
    uint32 index[8];

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if('s'==TOKEN_CHAR(3,0))
    {
        /*call API*/
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_type_get(&type), ret);
        diag_util_printf("Flow Control Type is  %s\n",diagStr_flowCtrlType[type]);
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboMode_get(&enable), ret);
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboSize_get(&size), ret);
        diag_util_printf("Jumbo Mode is %s\n",diagStr_enable[enable]);
        diag_util_printf("Jumbo Size is %s\n",diagStr_flowCtrlJumboSize[size]);
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_dropAllThreshold_get(&threshold), ret);
        diag_util_printf("Drop All Threshold is %u\n", threshold);
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_pauseAllThreshold_get(&threshold), ret);
        diag_util_printf("Pause All Threshold is %u\n", threshold);

        diag_util_printf("\nFlow Control On-Off Threshold\n");
        diag_util_printf("                        On Threshold     Off-Threshold\n");
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconHighThreshold_get(&onThresh, &offThresh), ret);
        diag_util_printf("Global High FC-On            %-4u              %-4u\n",onThresh,offThresh);
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconLowThreshold_get(&onThresh, &offThresh), ret);
        diag_util_printf("Global Low FC-On             %-4u              %-4u\n",onThresh,offThresh);
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFconHighThreshold_get(&onThresh, &offThresh), ret);
        diag_util_printf("Port High FC-On              %-4u              %-4u\n",onThresh,offThresh);
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFconLowThreshold_get(&onThresh, &offThresh), ret);
        diag_util_printf("Port Low FC-On               %-4u              %-4u\n",onThresh,offThresh);
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffHighThreshold_get(&onThresh, &offThresh), ret);
        diag_util_printf("Global High FC-Off           %-4u              %-4u\n",onThresh,offThresh);
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffLowThreshold_get(&onThresh, &offThresh), ret);
        diag_util_printf("Global Low FC-Off            %-4u              %-4u\n",onThresh,offThresh);
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFcoffHighThreshold_get(&onThresh, &offThresh), ret);
        diag_util_printf("Port High FC-Off             %-4u              %-4u\n",onThresh,offThresh);
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFcoffLowThreshold_get(&onThresh, &offThresh), ret);
        diag_util_printf("Port Low FC-Off              %-4u              %-4u\n",onThresh,offThresh);
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboGlobalHighThreshold_get(&onThresh, &offThresh), ret);
        diag_util_printf("Jumbo Global High            %-4u              %-4u\n",onThresh,offThresh);
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboGlobalLowThreshold_get(&onThresh, &offThresh), ret);
        diag_util_printf("Jumbo Global Low             %-4u              %-4u\n",onThresh,offThresh);
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboPortHighThreshold_get(&onThresh, &offThresh), ret);
        diag_util_printf("Jumbo Port High              %-4u              %-4u\n",onThresh,offThresh);
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboPortLowThreshold_get(&onThresh, &offThresh), ret);
        diag_util_printf("Jumbo Port Low               %-4u              %-4u\n",onThresh,offThresh);
        diag_util_printf("\n");
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portEegressGapThreshold_get(&threshold), ret);
        diag_util_printf("Port Egress Gap Threshold is %u \n", threshold);
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_queueEegressGapThreshold_get(&threshold), ret);
        diag_util_printf("Queue Egress Gap Threshold is %u \n", threshold);

        portlist.min =0;
        portlist.max = 6;
        portlist.portmask.bits[0] = 0x7F;
        diag_util_printf("Egress-Port       Drop-Threshold \n");
        diag_util_printf("-----------------------------\n");
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portEegressDropThreshold_get(port, &threshold), ret);
            diag_util_printf("%-18u  %u\n", port, threshold);
        }
        diag_util_printf("\n");
        queuelist.min = 0;
        queuelist.max =  APOLLO_QUEUENO -1;
        queuelist.mask.bits[0] = 0xFF;
        diag_util_printf("Egress-Queue       Drop-Threshold \n");
        diag_util_printf("---------------------------------\n");
        DIAG_UTIL_MASK_SCAN(queuelist, queue)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_queueEegressDropThreshold_get(queue, &threshold), ret);
            diag_util_printf("%-18u  %u\n", queue, threshold);
        }

        diag_util_printf("ueue Egress Drop Enable \n");
        diag_util_printf("            ");
        for(queue=0;queue<8; queue++)
        {
            diag_util_printf("Q%d    ",queue);
        }
        diag_util_printf("\n");
        diag_util_printf("---------------------------------------------------------\n");
        for(port=0;port<7; port++)
        {
            diag_util_printf("Port %2u ", port);
            for(queue=0;queue<8; queue++)
            {
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_egressDropEnable_get(port, queue, &enable), ret);
                    diag_util_printf("   %3s",enable?"En":"Dis");
            }
            diag_util_printf("\n");
        }

    }
    else if('p'==TOKEN_CHAR(3,0))
    {
        diag_util_printf("\nPON Flow Control On-Off Threshold\n");
        diag_util_printf("                        On Threshold     Off-Threshold\n");
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponGlobalHighThreshold_get(&onThresh, &offThresh), ret);
        diag_util_printf("Global High                  %-4u              %-4u\n",onThresh,offThresh);
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponGlobalLowThreshold_get(&onThresh, &offThresh), ret);
        diag_util_printf("Global Low                   %-4u              %-4u\n",onThresh,offThresh);
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponPortHighThreshold_get(&onThresh, &offThresh), ret);
        diag_util_printf("Port High                    %-4u              %-4u\n",onThresh,offThresh);
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponPortLowThreshold_get(&onThresh, &offThresh), ret);
        diag_util_printf("Port Low                     %-4u              %-4u\n",onThresh,offThresh);

        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponQueueEegressGapThreshold_get(&threshold), ret);
        diag_util_printf("PON MAC Egress Queue Gap Threshold is %u \n", threshold);
        diag_util_printf("\n");
        idxlist.min = 0;
        idxlist.max =  APOLLO_PON_QUEUE_INDEX_MAX;
        idxlist.mask.bits[0] = 0xFF;
        diag_util_printf("PON  Egress Queue Drop Index Threshold  \n");
        diag_util_printf("Index           Drop-Threshold \n");
        diag_util_printf("---------------------------------\n");
        DIAG_UTIL_MASK_SCAN(idxlist, idx)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponQueueEegressDropThreshold_get(idx, &threshold), ret);
            diag_util_printf("%-18u  %u\n", idx, threshold);
        }

        diag_util_printf("Queue Egress Drop Threshold Index \n");
        diag_util_printf("--------------------------------------------------\n");
        for(queue=0;queue<8; queue++)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponQueueEegressDropIndex_get(queue, &index[(queue%8)]), ret);
        }

        diag_util_printf("QID:       0     1     2     3     4     5     6     7    \n");

        diag_util_printf("Index:     ");
        for(queue=0;queue<8; queue++)
        {
            diag_util_printf("%-6u", index[queue]);
    	      if(queue==7)
                diag_util_printf("\n");
        }

        diag_util_printf("---------------------------------------------------\n");

        for(queue=8;queue<16; queue++)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponQueueEegressDropIndex_get(queue, &index[(queue%8)]), ret);
        }

        diag_util_printf("QID:       8     9    10    11    12    13    14    15    \n");

        diag_util_printf("Index:     ");
        for(queue=0;queue<8; queue++)
        {
            diag_util_printf("%-6u", index[queue]);
    	      if(queue==7)
                diag_util_printf("\n");
        }

        diag_util_printf("---------------------------------------------------\n");

        for(queue=16;queue<24; queue++)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponQueueEegressDropIndex_get(queue, &index[(queue%8)]), ret);
        }

        diag_util_printf("QID:      16    17    18    19    20    21    22    23    \n");

        diag_util_printf("Index:     ");
        for(queue=0;queue<8; queue++)
        {
            diag_util_printf("%-6u", index[queue]);
    	      if(queue==7)
                diag_util_printf("\n");
        }

        diag_util_printf("---------------------------------------------------\n");

        for(queue=24;queue<32; queue++)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponQueueEegressDropIndex_get(queue, &index[(queue%8)]), ret);
        }

        diag_util_printf("QID:      24    25    26    27    28    29    30    31    \n");

        diag_util_printf("Index:     ");
        for(queue=0;queue<8; queue++)
        {
            diag_util_printf("%-6u", index[queue]);
    	      if(queue==7)
                diag_util_printf("\n");
        }

        diag_util_printf("---------------------------------------------------\n");

        for(queue=32;queue<40; queue++)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponQueueEegressDropIndex_get(queue, &index[(queue%8)]), ret);
        }

        diag_util_printf("QID:      32    33    34    35    36    37    38    39    \n");

        diag_util_printf("Index:     ");
        for(queue=0;queue<8; queue++)
        {
            diag_util_printf("%-6u", index[queue]);
    	      if(queue==7)
                diag_util_printf("\n");
        }

        diag_util_printf("---------------------------------------------------\n");

        for(queue=40;queue<48; queue++)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponQueueEegressDropIndex_get(queue, &index[(queue%8)]), ret);
        }

        diag_util_printf("QID:      40    41    42    43    44    45    46    47    \n");

        diag_util_printf("Index:     ");
        for(queue=0;queue<8; queue++)
        {
            diag_util_printf("%-6u", index[queue]);
    	      if(queue==7)
                diag_util_printf("\n");
        }

        diag_util_printf("---------------------------------------------------\n");

        for(queue=48;queue<56; queue++)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponQueueEegressDropIndex_get(queue, &index[(queue%8)]), ret);
        }

        diag_util_printf("QID:      48    49    50    51    52    53    54    55    \n");

        diag_util_printf("Index:     ");
        for(queue=0;queue<8; queue++)
        {
            diag_util_printf("%-6u", index[queue]);
    	      if(queue==7)
                diag_util_printf("\n");
        }

        diag_util_printf("---------------------------------------------------\n");

        for(queue=56;queue<64; queue++)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponQueueEegressDropIndex_get(queue, &index[(queue%8)]), ret);
        }

        diag_util_printf("QID:      56    57    58    59    60    61    62    63    \n");

        diag_util_printf("Index:     ");
        for(queue=0;queue<8; queue++)
        {
            diag_util_printf("%-6u", index[queue]);
    	      if(queue==7)
                diag_util_printf("\n");
        }

        diag_util_printf("---------------------------------------------------\n");

        for(queue=64;queue<72; queue++)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponQueueEegressDropIndex_get(queue, &index[(queue%8)]), ret);
        }

        diag_util_printf("QID:      64    65    66    67    68    69    70    71    \n");

        diag_util_printf("Index:     ");
        for(queue=0;queue<8; queue++)
        {
            diag_util_printf("%-6u", index[queue]);
    	      if(queue==7)
                diag_util_printf("\n");
        }

        diag_util_printf("---------------------------------------------------\n");

        for(queue=72;queue<80; queue++)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponQueueEegressDropIndex_get(queue, &index[(queue%8)]), ret);
        }

        diag_util_printf("QID:      72    73    74    75    76    77    78    79    \n");

        diag_util_printf("Index:     ");
        for(queue=0;queue<8; queue++)
        {
            diag_util_printf("%-6u", index[queue]);
    	      if(queue==7)
                diag_util_printf("\n");
        }

        diag_util_printf("---------------------------------------------------\n");

        for(queue=80;queue<88; queue++)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponQueueEegressDropIndex_get(queue, &index[(queue%8)]), ret);
        }

        diag_util_printf("QID:      80    81    82    83    84    85    86    87    \n");

        diag_util_printf("Index:     ");
        for(queue=0;queue<8; queue++)
        {
            diag_util_printf("%-6u", index[queue]);
    	      if(queue==7)
                diag_util_printf("\n");
        }

        diag_util_printf("---------------------------------------------------\n");

        for(queue=88;queue<96; queue++)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponQueueEegressDropIndex_get(queue, &index[(queue%8)]), ret);
        }

        diag_util_printf("QID:      88    89    90    91    92    93    94    95    \n");

        diag_util_printf("Index:     ");
        for(queue=0;queue<8; queue++)
        {
            diag_util_printf("%-6u", index[queue]);
    	      if(queue==7)
                diag_util_printf("\n");
        }

        diag_util_printf("---------------------------------------------------\n");

        for(queue=96;queue<104; queue++)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponQueueEegressDropIndex_get(queue, &index[(queue%8)]), ret);
        }

        diag_util_printf("QID:      96    97    98    99    100   101   102   103    \n");

        diag_util_printf("Index:     ");
        for(queue=0;queue<8; queue++)
        {
            diag_util_printf("%-6u", index[queue]);
    	      if(queue==7)
                diag_util_printf("\n");
        }

        diag_util_printf("---------------------------------------------------\n");

        for(queue=104;queue<112; queue++)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponQueueEegressDropIndex_get(queue, &index[(queue%8)]), ret);
        }

        diag_util_printf("QID:      104   105   106   107   108   109   110   111    \n");

        diag_util_printf("Index:     ");
        for(queue=0;queue<8; queue++)
        {
            diag_util_printf("%-6u", index[queue]);
    	      if(queue==7)
                diag_util_printf("\n");
        }

        diag_util_printf("---------------------------------------------------\n");

        for(queue=112;queue<120; queue++)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponQueueEegressDropIndex_get(queue, &index[(queue%8)]), ret);
        }

        diag_util_printf("QID:      112   113   114   115   116   117   118   119    \n");

        diag_util_printf("Index:     ");
        for(queue=0;queue<8; queue++)
        {
            diag_util_printf("%-6u", index[queue]);
    	      if(queue==7)
                diag_util_printf("\n");
        }

        diag_util_printf("---------------------------------------------------\n");

        for(queue=120;queue<128; queue++)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponQueueEegressDropIndex_get(queue, &index[(queue%8)]), ret);
        }

        diag_util_printf("QID:      120   121   122   123   124   125   126   127    \n");

        diag_util_printf("Index:     ");
        for(queue=0;queue<8; queue++)
        {
            diag_util_printf("%-6u", index[queue]);
    	      if(queue==7)
                diag_util_printf("\n");
        }

    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_dump_threshold_switch_pon */


/*
* flowctrl dump used-page ( switch | pon )
*/
cparser_result_t
cparser_cmd_flowctrl_dump_used_page_switch_pon(
cparser_context_t *context)
{
    apollo_raw_flowctrl_type_t type;
    rtk_enable_t enable;
    apollo_raw_flowctrl_jumbo_size_t size;
    uint32 threshold, current, maximum;
    diag_portlist_t portlist;
    rtk_port_t port, tmp_port;
    diag_mask_t  queuelist;
    rtk_qid_t queue,idx, tmp_queue;
    uint32 qcurrent[8], qmaximum[8];
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    if('s'==TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_totalUsedPageCnt_get(&current, &maximum), ret);
        diag_util_printf("Total page Count: Current  %u maximum  %u\n", current, maximum);
        diag_util_printf("\n");
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_publicUsedPageCnt_get(&current, &maximum), ret);
        diag_util_printf("Public page Count: Current  %u maximum  %u\n", current, maximum);
        diag_util_printf("\n");
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_publicFcoffUsedPageCnt_get(&current, &maximum), ret);
        diag_util_printf("Public page Count for FC-off: Current  %u maximum  %u\n", current, maximum);
        diag_util_printf("\n");
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_publicJumboUsedPageCnt_get(&current, &maximum), ret);
        diag_util_printf("Public page Count for jumbo frame: Current  %u maximum  %u\n", current, maximum);

        diag_util_printf("ingress-port     current-page      maximum-page\n");
        diag_util_printf("-----------------------------------------------------\n");
        for(port=0;port<APOLLO_PORTNO; port++)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portUsedPageCnt_get(port, &current, &maximum), ret);
            diag_util_printf("    %-18u %-18u%u\n", port, current, maximum);
        }

        diag_util_printf("egress-port     current-page      maximum-page\n");
        diag_util_printf("-----------------------------------------------------\n");
        for(port=0;port<APOLLO_PORTNO; port++)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portEgressUsedPageCnt_get(port, &current, &maximum), ret);
            diag_util_printf("    %-18u %-18u%u\n", port, current, maximum);
        }

        diag_util_printf("Egress Page Count \n");
        for(port=0;port<APOLLO_PORTNO; port++)
        {
            if (HAL_GET_PON_PORT() != port)
            {
                if (port>HAL_GET_PON_PORT() )
                    tmp_port = port-1;
                else
                    tmp_port = port;

                diag_util_printf("          P%u     Q0     Q1     Q2     Q3     Q4     Q5     Q6     Q7  \n", port);
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portEgressUsedPageCnt_get(tmp_port, &current, &maximum), ret);

                for(queue=0;queue<APOLLO_QUEUENO; queue++)
                {
                        DIAG_UTIL_ERR_CHK( apollo_raw_flowctrl_queueEgressUsedPageCnt_get(tmp_port, queue, &qcurrent[queue], &qmaximum[queue]), ret);
                }
                diag_util_printf("Current : %-5u  ", current);

                for(queue=0;queue<APOLLO_QUEUENO; queue++)
                {
			 if (queue == 7)
			 	diag_util_printf("%-5u\n", qcurrent[queue]);
			 else
                         diag_util_printf("%-5u  ", qcurrent[queue]);
                }

                diag_util_printf("Maximum : %-5u  ", current);

                for(queue=0;queue<APOLLO_QUEUENO; queue++)
                {
			 if (queue == 7)
			 	diag_util_printf("%-5u\n", qmaximum[queue]);
			 else
                         diag_util_printf("%-5u  ", qmaximum[queue]);
                }
                diag_util_printf("-------------------------------------------------------------------\n");
            }

        }

    }
    else if('p'==TOKEN_CHAR(3,0))
    {


        diag_util_printf("Get PON  Egress Queue Used Page Count\n");
        diag_util_printf("-------------------------------------------------------------\n");

	 for 	(idx=0;idx<16;idx++)
	 {
	      diag_util_printf("QID:          ");
            for(queue=0;queue<8; queue++)
            {
                tmp_queue = (idx<<3) | queue;
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponUsedPage_get(tmp_queue, &qcurrent[(tmp_queue%8)], &qmaximum[(tmp_queue%8)]), ret);
                if (queue == 7)
                    diag_util_printf("%-5u\n", tmp_queue);
                else
                    diag_util_printf("%-5u   ", tmp_queue);
            }
           diag_util_printf("Current :     ");
           for(queue=0;queue<APOLLO_QUEUENO; queue++)
           {
               tmp_queue = (idx<<3) | queue;
               if (queue == 7)
                   diag_util_printf("%-5u\n", qcurrent[queue]);
               else
                   diag_util_printf("%-5u   ", qcurrent[queue]);
           }
           diag_util_printf("Maximum :     ");
           for(queue=0;queue<APOLLO_QUEUENO; queue++)
           {
           if (queue == 7)
               diag_util_printf("%-5u\n", qmaximum[queue]);
           else
               diag_util_printf("%-5u   ", qmaximum[queue]);
           }
           diag_util_printf("-------------------------------------------------------------------\n");

	 }


    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_dump_used_page_switch_pon */



/*
 * flowctrl set type ( ingress | egress )
 */
cparser_result_t
cparser_cmd_flowctrl_set_type_ingress_egress(
    cparser_context_t *context)
{
    apollo_raw_flowctrl_type_t type;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    if('i'==TOKEN_CHAR(3,0))
        type = RAW_FLOWCTRL_TYPE_INGRESS;
    else if('e'==TOKEN_CHAR(3,0))
        type = RAW_FLOWCTRL_TYPE_EGRESS;

    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_type_set(type), ret);

    diag_util_mprintf("Set Flow Control Type to  %s\n",diagStr_flowCtrlType[type]);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_type_ingress_egress */

/*
 * flowctrl get type
 */
cparser_result_t
cparser_cmd_flowctrl_get_type(
    cparser_context_t *context)
{
    apollo_raw_flowctrl_type_t type;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    /*call API*/
    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_type_get(&type), ret);

    diag_util_mprintf("Flow Control Type is  %s\n",diagStr_flowCtrlType[type]);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_type */

/*
 * flowctrl set jumbo mode ( enable | disable )
 */
cparser_result_t
cparser_cmd_flowctrl_set_jumbo_mode_enable_disable(
    cparser_context_t *context)
{
    rtk_enable_t enable;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    if('e'==TOKEN_CHAR(4,0))
        enable = ENABLED;
    else if('d'==TOKEN_CHAR(4,0))
        enable = DISABLED;

    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboMode_set(enable), ret);

    diag_util_mprintf("Set Flow Control Jumbo Mode to %s\n",diagStr_enable[enable]);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_jumbo_mode_enable_disable */

/*
 * flowctrl get jumbo mode
 */
cparser_result_t
cparser_cmd_flowctrl_get_jumbo_mode(
    cparser_context_t *context)
{
    rtk_enable_t enable;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    /*call API*/
    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboMode_get(&enable), ret);

    diag_util_mprintf("Flow Control Jumbo Mode is %s\n",diagStr_enable[enable]);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_jumbo_mode */

/*
 * flowctrl set jumbo size ( 3k | 4k | 6k | max )
 */
cparser_result_t
cparser_cmd_flowctrl_set_jumbo_size_3k_4k_6k_max(
    cparser_context_t *context)
{
    apollo_raw_flowctrl_jumbo_size_t size;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    if('3'==TOKEN_CHAR(4,0))
        size = RAW_FLOWCTRL_JUMBO_3K;
    else if('4'==TOKEN_CHAR(4,0))
        size = RAW_FLOWCTRL_JUMBO_4K;
    else if('6'==TOKEN_CHAR(4,0))
        size = RAW_FLOWCTRL_JUMBO_6K;
    else if('m'==TOKEN_CHAR(4,0))
        size = RAW_FLOWCTRL_JUMBO_MAX;

    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboSize_set(size), ret);

    diag_util_mprintf("Set Flow Control Jumbo size to %s\n",diagStr_flowCtrlJumboSize[size]);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_jumbo_size_3k_4k_6k_max */

/*
 * flowctrl get jumbo size
 */
cparser_result_t
cparser_cmd_flowctrl_get_jumbo_size(
    cparser_context_t *context)
{
    apollo_raw_flowctrl_jumbo_size_t size;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboSize_get(&size), ret);

    diag_util_mprintf("Flow Control Jumbo size is %s\n",diagStr_flowCtrlJumboSize[size]);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_jumbo_size */

/*
 * flowctrl set drop-all <UINT:thresh>
 */
cparser_result_t
cparser_cmd_flowctrl_set_drop_all_thresh(
    cparser_context_t *context,
    uint32_t  *thresh_ptr)
{
    int32 ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_dropAllThreshold_set(*thresh_ptr), ret);
    diag_util_mprintf("Set Drop All Threshold to %u\n", *thresh_ptr);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_drop_all_thresh */

/*
 * flowctrl get drop-all
 */
cparser_result_t
cparser_cmd_flowctrl_get_drop_all(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    uint32 threshold;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_dropAllThreshold_get(&threshold), ret);
    diag_util_mprintf("Drop All Threshold is %u\n", threshold);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_drop_all */

/*
 * flowctrl set pause-all <UINT:thresh>
 */
cparser_result_t
cparser_cmd_flowctrl_set_pause_all_thresh(
    cparser_context_t *context,
    uint32_t  *thresh_ptr)
{
    int32 ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_pauseAllThreshold_set(*thresh_ptr), ret);
    diag_util_mprintf("Set Pause All Threshold to %u\n", *thresh_ptr);


    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_pause_all_thresh */

/*
 * flowctrl get pause-all
 */
cparser_result_t
cparser_cmd_flowctrl_get_pause_all(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    uint32 threshold;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_pauseAllThreshold_get(&threshold), ret);
    diag_util_mprintf("Pause All Threshold is %u\n", threshold);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_pause_all */

/*
 * flowctrl set ingress global ( fc-off-thresh | fc-on-thresh ) ( high-off | high-on | low-off | low-on ) <UINT:thresh>
 */
cparser_result_t
cparser_cmd_flowctrl_set_ingress_global_fc_off_thresh_fc_on_thresh_high_off_high_on_low_off_low_on_thresh(
    cparser_context_t *context,
    uint32_t  *thresh_ptr)
{
    uint32 onThresh, offThresh;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    if('f'==TOKEN_CHAR(4,4))
    {
        if(!osal_strcmp(TOKEN_STR(5),"high-off"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffHighThreshold_get(&onThresh, &offThresh), ret);
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffHighThreshold_set(onThresh, *thresh_ptr), ret);
            diag_util_mprintf("Set Flow Control Global FC-Off High Off Threshold to %u\n",*thresh_ptr);
        }
        else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffHighThreshold_get(&onThresh, &offThresh), ret);
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffHighThreshold_set(*thresh_ptr, offThresh), ret);
            diag_util_mprintf("Set Flow Control Global FC-Off High On Threshold to %u\n",*thresh_ptr);
        }
        else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffLowThreshold_get(&onThresh, &offThresh), ret);
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffLowThreshold_set(onThresh, *thresh_ptr), ret);
            diag_util_mprintf("Set Flow Control Global FC-Off Low Off Threshold to %u\n",*thresh_ptr);
        }
        else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffLowThreshold_get(&onThresh, &offThresh), ret);
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffLowThreshold_set(*thresh_ptr, offThresh), ret);
            diag_util_mprintf("Set Flow Control Global FC-Off Low On Threshold to %u\n",*thresh_ptr);
        }
    }
    else if('n'==TOKEN_CHAR(4,4))
    {
        if(!osal_strcmp(TOKEN_STR(5),"high-off"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconHighThreshold_get(&onThresh, &offThresh), ret);
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconHighThreshold_set(onThresh, *thresh_ptr), ret);
            diag_util_mprintf("Set Flow Control Global FC-On High Off Threshold to %u\n",*thresh_ptr);
        }
        else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconHighThreshold_get(&onThresh, &offThresh), ret);
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconHighThreshold_set(*thresh_ptr, offThresh), ret);
            diag_util_mprintf("Set Flow Control Global FC-On High On Threshold to %u\n",*thresh_ptr);
        }
        else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconLowThreshold_get(&onThresh, &offThresh), ret);
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconLowThreshold_set(onThresh, *thresh_ptr), ret);
            diag_util_mprintf("Set Flow Control Global FC-On Low Off Threshold to %u\n",*thresh_ptr);
        }
        else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconLowThreshold_get(&onThresh, &offThresh), ret);
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconLowThreshold_set(*thresh_ptr, offThresh), ret);
            diag_util_mprintf("Set Flow Control Global FC-On Low On Threshold to %u\n",*thresh_ptr);
        }
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_ingress_global_fc_off_thresh_fc_on_thresh_high_off_high_on_low_off_low_on_thresh */

/*
 * flowctrl get ingress global ( fc-off-thresh | fc-on-thresh ) ( high-off | high-on | low-off | low-on )
 */
cparser_result_t
cparser_cmd_flowctrl_get_ingress_global_fc_off_thresh_fc_on_thresh_high_off_high_on_low_off_low_on(
    cparser_context_t *context)
{
    uint32 onThresh, offThresh;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if('f'==TOKEN_CHAR(4,4))
    {
        if(!osal_strcmp(TOKEN_STR(5),"high-off"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffHighThreshold_get(&onThresh, &offThresh), ret);
            diag_util_mprintf("Flow Control Global FC-Off High Off Threshold is %u\n",offThresh);
        }
        else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffHighThreshold_get(&onThresh, &offThresh), ret);
            diag_util_mprintf("Flow Control Global FC-Off High On Threshold is %u\n",onThresh);
        }
        else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffLowThreshold_get(&onThresh, &offThresh), ret);
            diag_util_mprintf("Flow Control Global FC-Off Low Off Threshold is %u\n",offThresh);
        }
        else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffLowThreshold_get(&onThresh, &offThresh), ret);
            diag_util_mprintf("Flow Control Global FC-Off Low On Threshold is %u\n",onThresh);
        }
    }
    else if('n'==TOKEN_CHAR(4,4))
    {
        if(!osal_strcmp(TOKEN_STR(5),"high-off"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconHighThreshold_get(&onThresh, &offThresh), ret);
            diag_util_mprintf("Flow Control Global FC-On High Off Threshold is %u\n",offThresh);
        }
        else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconHighThreshold_get(&onThresh, &offThresh), ret);
            diag_util_mprintf("Flow Control Global FC-On High On Threshold is %u\n",onThresh);
        }
        else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconLowThreshold_get(&onThresh, &offThresh), ret);
            diag_util_mprintf("Flow Control Global FC-On Low Off Threshold is %u\n",offThresh);
        }
        else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconLowThreshold_get(&onThresh, &offThresh), ret);
            diag_util_mprintf("Flow Control Global FC-On Low On Threshold is %u\n",onThresh);
        }
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_ingress_global_fc_off_thresh_fc_on_thresh_high_off_high_on_low_off_low_on */

/*
 * flowctrl set ingress port ( fc-off-thresh | fc-on-thresh ) ( high-off | high-on | low-off | low-on ) <UINT:thresh>
 */
cparser_result_t
cparser_cmd_flowctrl_set_ingress_port_fc_off_thresh_fc_on_thresh_high_off_high_on_low_off_low_on_thresh(
    cparser_context_t *context,
    uint32_t  *thresh_ptr)
{
    uint32 onThresh, offThresh;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    if('f'==TOKEN_CHAR(4,4))
    {
        if(!osal_strcmp(TOKEN_STR(5),"high-off"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFcoffHighThreshold_get(&onThresh, &offThresh), ret);
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFcoffHighThreshold_set(onThresh, *thresh_ptr), ret);
            diag_util_mprintf("Set Flow Control Port FC-Off High Off Threshold to %u\n",*thresh_ptr);
        }
        else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFcoffHighThreshold_get(&onThresh, &offThresh), ret);
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFcoffHighThreshold_set(*thresh_ptr, offThresh), ret);
            diag_util_mprintf("Set Flow Control Port FC-Off High On Threshold to %u\n",*thresh_ptr);
        }
        else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFcoffLowThreshold_get(&onThresh, &offThresh), ret);
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFcoffLowThreshold_set(onThresh, *thresh_ptr), ret);
            diag_util_mprintf("Set Flow Control Port FC-Off Low Off Threshold to %u\n",*thresh_ptr);
        }
        else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFcoffLowThreshold_get(&onThresh, &offThresh), ret);
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFcoffLowThreshold_set(*thresh_ptr, offThresh), ret);
            diag_util_mprintf("Set Flow Control Port FC-Off Low On Threshold to %u\n",*thresh_ptr);
        }
    }
    else if('n'==TOKEN_CHAR(4,4))
    {
        if(!osal_strcmp(TOKEN_STR(5),"high-off"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFconHighThreshold_get(&onThresh, &offThresh), ret);
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFconHighThreshold_set(onThresh, *thresh_ptr), ret);
            diag_util_mprintf("Set Flow Control Port FC-On High Off Threshold to %u\n",*thresh_ptr);
        }
        else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFconHighThreshold_get(&onThresh, &offThresh), ret);
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFconHighThreshold_set(*thresh_ptr, offThresh), ret);
            diag_util_mprintf("Set Flow Control Port FC-On High On Threshold to %u\n",*thresh_ptr);
        }
        else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFconLowThreshold_get(&onThresh, &offThresh), ret);
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFconLowThreshold_set(onThresh, *thresh_ptr), ret);
            diag_util_mprintf("Set Flow Control Port FC-On Low Off Threshold to %u\n",*thresh_ptr);
        }
        else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFconLowThreshold_get(&onThresh, &offThresh), ret);
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFconLowThreshold_set(*thresh_ptr, offThresh), ret);
            diag_util_mprintf("Set Flow Control Port FC-On Low On Threshold to %u\n",*thresh_ptr);
        }
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_ingress_port_fc_off_thresh_fc_on_thresh_high_off_high_on_low_off_low_on_thresh */

/*
 * flowctrl get ingress port ( fc-off-thresh | fc-on-thresh ) ( high-off | high-on | low-off | low-on )
 */
cparser_result_t
cparser_cmd_flowctrl_get_ingress_port_fc_off_thresh_fc_on_thresh_high_off_high_on_low_off_low_on(
    cparser_context_t *context)
{
    uint32 onThresh, offThresh;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if('f'==TOKEN_CHAR(4,4))
    {
        if(!osal_strcmp(TOKEN_STR(5),"high-off"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFcoffHighThreshold_get(&onThresh, &offThresh), ret);
            diag_util_mprintf("Flow Control Port FC-Off High Off Threshold is %u\n",offThresh);
        }
        else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFcoffHighThreshold_get(&onThresh, &offThresh), ret);
            diag_util_mprintf("Flow Control Port FC-Off High On Threshold is %u\n",onThresh);
        }
        else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFcoffLowThreshold_get(&onThresh, &offThresh), ret);
            diag_util_mprintf("Flow Control Port FC-Off Low Off Threshold is %u\n",offThresh);
        }
        else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFcoffLowThreshold_get(&onThresh, &offThresh), ret);
            diag_util_mprintf("Flow Control Port FC-Off Low On Threshold is %u\n",onThresh);
        }
    }
    else if('n'==TOKEN_CHAR(4,4))
    {
        if(!osal_strcmp(TOKEN_STR(5),"high-off"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFconHighThreshold_get(&onThresh, &offThresh), ret);
            diag_util_mprintf("Flow Control Port FC-On High Off Threshold is %u\n",offThresh);
        }
        else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFconHighThreshold_get(&onThresh, &offThresh), ret);
            diag_util_mprintf("Flow Control Port FC-On High On Threshold is %u\n",onThresh);
        }
        else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFconLowThreshold_get(&onThresh, &offThresh), ret);
            diag_util_mprintf("Flow Control Port FC-On Low Off Threshold is %u\n",offThresh);
        }
        else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFconLowThreshold_get(&onThresh, &offThresh), ret);
            diag_util_mprintf("Flow Control Port FC-On Low On Threshold is %u\n",onThresh);
        }
    }


    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_ingress_port_fc_off_thresh_fc_on_thresh_high_off_high_on_low_off_low_on */

/*
 * flowctrl set ingress jumbo-global ( high-off | high-on | low-off | low-on ) <UINT:thresh>
 */
cparser_result_t
cparser_cmd_flowctrl_set_ingress_jumbo_global_high_off_high_on_low_off_low_on_thresh(
    cparser_context_t *context,
    uint32_t  *thresh_ptr)
{
    uint32 onThresh, offThresh;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    if(!osal_strcmp(TOKEN_STR(4),"high-off"))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboGlobalHighThreshold_get(&onThresh, &offThresh), ret);
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboGlobalHighThreshold_set(onThresh, *thresh_ptr), ret);
        diag_util_mprintf("Set Flow Control Jumbo Global High Off Threshold to %u\n",*thresh_ptr);
    }
    else if(!osal_strcmp(TOKEN_STR(4),"high-on"))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboGlobalHighThreshold_get(&onThresh, &offThresh), ret);
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboGlobalHighThreshold_set(*thresh_ptr, offThresh), ret);
        diag_util_mprintf("Set Flow Control Jumbo Global High On Threshold to %u\n",*thresh_ptr);
    }
    else if(!osal_strcmp(TOKEN_STR(4),"low-off"))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboGlobalLowThreshold_get(&onThresh, &offThresh), ret);
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboGlobalLowThreshold_set(onThresh, *thresh_ptr), ret);
        diag_util_mprintf("Set Flow Control Jumbo Global Low Off Threshold to %u\n",*thresh_ptr);
    }
    else if(!osal_strcmp(TOKEN_STR(4),"low-on"))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboGlobalLowThreshold_get(&onThresh, &offThresh), ret);
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboGlobalLowThreshold_set(*thresh_ptr, offThresh), ret);
        diag_util_mprintf("Set Flow Control Jumbo Global Low On Threshold to %u\n",*thresh_ptr);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_ingress_jumbo_global_high_off_high_on_low_off_low_on_thresh */

/*
 * flowctrl get ingress jumbo-global ( high-off | high-on | low-off | low-on )
 */
cparser_result_t
cparser_cmd_flowctrl_get_ingress_jumbo_global_high_off_high_on_low_off_low_on(
    cparser_context_t *context)
{
    uint32 onThresh, offThresh;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if(!osal_strcmp(TOKEN_STR(4),"high-off"))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboGlobalHighThreshold_get(&onThresh, &offThresh), ret);
        diag_util_mprintf("Flow Control Jumbo Global High Off Threshold is %u\n",offThresh);
    }
    else if(!osal_strcmp(TOKEN_STR(4),"high-on"))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboGlobalHighThreshold_get(&onThresh, &offThresh), ret);
        diag_util_mprintf("Flow Control Jumbo Global High On Threshold is %u\n",onThresh);
    }
    else if(!osal_strcmp(TOKEN_STR(4),"low-off"))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboGlobalLowThreshold_get(&onThresh, &offThresh), ret);
        diag_util_mprintf("Flow Control Jumbo Global Low Off Threshold is %u\n",offThresh);
    }
    else if(!osal_strcmp(TOKEN_STR(4),"low-on"))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboGlobalLowThreshold_get(&onThresh, &offThresh), ret);
        diag_util_mprintf("Flow Control Jumbo Global Low On Threshold is %u\n",onThresh);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_ingress_jumbo_global_high_off_high_on_low_off_low_on */

/*
 * flowctrl set ingress jumbo-port ( high-off | high-on | low-off | low-on ) <UINT:thresh>
 */
cparser_result_t
cparser_cmd_flowctrl_set_ingress_jumbo_port_high_off_high_on_low_off_low_on_thresh(
    cparser_context_t *context,
    uint32_t  *thresh_ptr)
{
    uint32 onThresh, offThresh;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    if(!osal_strcmp(TOKEN_STR(4),"high-off"))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboPortHighThreshold_get(&onThresh, &offThresh), ret);
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboPortHighThreshold_set(onThresh, *thresh_ptr), ret);
        diag_util_mprintf("Set Flow Control Jumbo Port High Off Threshold to %u\n",*thresh_ptr);
    }
    else if(!osal_strcmp(TOKEN_STR(4),"high-on"))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboPortHighThreshold_get(&onThresh, &offThresh), ret);
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboPortHighThreshold_set(*thresh_ptr, offThresh), ret);
        diag_util_mprintf("Set Flow Control Jumbo Port High On Threshold to %u\n",*thresh_ptr);
    }
    else if(!osal_strcmp(TOKEN_STR(4),"low-off"))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboPortLowThreshold_get(&onThresh, &offThresh), ret);
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboPortLowThreshold_set(onThresh, *thresh_ptr), ret);
        diag_util_mprintf("Set Flow Control Jumbo Port Low Off Threshold to %u\n",*thresh_ptr);
    }
    else if(!osal_strcmp(TOKEN_STR(4),"low-on"))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboPortLowThreshold_get(&onThresh, &offThresh), ret);
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboPortLowThreshold_set(*thresh_ptr, offThresh), ret);
        diag_util_mprintf("Set Flow Control Jumbo Port Low On Threshold to %u\n",*thresh_ptr);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_ingress_jumbo_port_high_off_high_on_low_off_low_on_thresh */

/*
 * flowctrl get ingress jumbo-port ( high-off | high-on | low-off | low-on )
 */
cparser_result_t
cparser_cmd_flowctrl_get_ingress_jumbo_port_high_off_high_on_low_off_low_on(
    cparser_context_t *context)
{
    uint32 onThresh, offThresh;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if(!osal_strcmp(TOKEN_STR(4),"high-off"))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboPortHighThreshold_get(&onThresh, &offThresh), ret);
        diag_util_mprintf("Flow Control Jumbo Global High Off Threshold is %u\n",offThresh);
    }
    else if(!osal_strcmp(TOKEN_STR(4),"high-on"))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboPortHighThreshold_get(&onThresh, &offThresh), ret);
        diag_util_mprintf("Flow Control Jumbo Global High On Threshold is %u\n",onThresh);
    }
    else if(!osal_strcmp(TOKEN_STR(4),"low-off"))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboPortLowThreshold_get(&onThresh, &offThresh), ret);
        diag_util_mprintf("Flow Control Jumbo Global Low Off Threshold is %u\n",offThresh);
    }
    else if(!osal_strcmp(TOKEN_STR(4),"low-on"))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboPortLowThreshold_get(&onThresh, &offThresh), ret);
        diag_util_mprintf("Flow Control Jumbo Global Low On Threshold is %u\n",onThresh);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_ingress_jumbo_port_high_off_high_on_low_off_low_on */

/*
 * flowctrl set ingress egress-drop port ( <PORT_LIST:ports> | all ) <UINT:thresh>
 */
cparser_result_t
cparser_cmd_flowctrl_set_ingress_egress_drop_port_ports_all_thresh(
    cparser_context_t *context,
    char * *ports_ptr,
    uint32_t  *thresh_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    diag_util_mprintf("Set Ingress Flow Control Port Egress Drop Threshold \n");
    diag_util_mprintf("Egress-Port       Drop-Threshold \n");
    diag_util_mprintf("-----------------------------\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portEegressDropThreshold_set(port, *thresh_ptr), ret);
        diag_util_mprintf("%-18u  %u\n", port, *thresh_ptr);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_ingress_egress_drop_port_ports_all_thresh */

/*
 * flowctrl get ingress egress-drop port ( <PORT_LIST:ports> | all ) drop-thresh
 */
cparser_result_t
cparser_cmd_flowctrl_get_ingress_egress_drop_port_ports_all_drop_thresh(
    cparser_context_t *context,
    char * *ports_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    uint32 threshold;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    diag_util_mprintf("Get Ingress Flow Control Port Egress Drop Threshold \n");
    diag_util_mprintf("Egress-Port       Drop-Threshold \n");
    diag_util_mprintf("-----------------------------\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portEegressDropThreshold_get(port, &threshold), ret);
        diag_util_mprintf("%-18u  %u\n", port, threshold);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_ingress_egress_drop_port_ports_all_drop_thresh */

/*
 * flowctrl set ingress egress-drop queue ( <PORT_LIST:queues> | all ) <UINT:thresh>
 */
cparser_result_t
cparser_cmd_flowctrl_set_ingress_egress_drop_queue_queues_all_thresh(
    cparser_context_t *context,
    char * *queues_ptr,
    uint32_t  *thresh_ptr)
{
    diag_mask_t  queuelist;
    rtk_qid_t queue;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_QUEUEMASK(queuelist, 5), ret);

    diag_util_mprintf("Set Ingress Flow Control Queue Egress Drop Threshold \n");
    diag_util_mprintf("Egress-Queue       Drop-Threshold \n");
    diag_util_mprintf("---------------------------------\n");
    DIAG_UTIL_MASK_SCAN(queuelist, queue)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_queueEegressDropThreshold_set(queue, *thresh_ptr), ret);
        diag_util_mprintf("%-18u  %u\n", queue, *thresh_ptr);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_ingress_egress_drop_queue_queues_all_thresh */

/*
 * flowctrl get ingress egress-drop queue ( <PORT_LIST:queues> | all ) drop-thresh
 */
cparser_result_t
cparser_cmd_flowctrl_get_ingress_egress_drop_queue_queues_all_drop_thresh(
    cparser_context_t *context,
    char * *queues_ptr)
{
    diag_mask_t  queuelist;
    rtk_qid_t queue;
    uint32 threshold;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_QUEUEMASK(queuelist, 5), ret);

    diag_util_mprintf("Get Ingress Flow Control Queue Egress Drop Threshold \n");
    diag_util_mprintf("Egress-Queue       Drop-Threshold \n");
    diag_util_mprintf("---------------------------------\n");
    DIAG_UTIL_MASK_SCAN(queuelist, queue)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_queueEegressDropThreshold_get(queue, &threshold), ret);
        diag_util_mprintf("%-18u  %u\n", queue, threshold);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_ingress_egress_drop_queue_queues_all_drop_thresh */

/*
 * flowctrl set ingress egress-drop ( port-gap | queue-gap ) <UINT:thresh>
 */
cparser_result_t
cparser_cmd_flowctrl_set_ingress_egress_drop_port_gap_queue_gap_thresh(
    cparser_context_t *context,
    uint32_t  *thresh_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    if('p'==TOKEN_CHAR(4,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portEegressGapThreshold_set(*thresh_ptr), ret);
        diag_util_mprintf("Set Ingress Flow Control Port Egress Gap Threshold to %u \n", *thresh_ptr);
    }
    else if('q'==TOKEN_CHAR(4,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_queueEegressGapThreshold_set(*thresh_ptr), ret);
        diag_util_mprintf("Set Ingress Flow Control Queue Egress Gap Threshold to %u \n", *thresh_ptr);
    }
    else
        return RT_ERR_FAILED;

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_ingress_egress_drop_port_gap_queue_gap_thresh */

/*
 * flowctrl get ingress egress-drop ( port-gap | queue-gap ) drop-thresh
 */
cparser_result_t
cparser_cmd_flowctrl_get_ingress_egress_drop_port_gap_queue_gap_drop_thresh(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    uint32 threshold;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if('p'==TOKEN_CHAR(4,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portEegressGapThreshold_get(&threshold), ret);
        diag_util_mprintf("Ingress Flow Control Port Egress Gap Threshold is %u \n", threshold);
    }
    else if('q'==TOKEN_CHAR(4,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_queueEegressGapThreshold_get(&threshold), ret);
        diag_util_mprintf("Ingress Flow Control Queue Egress Gap Threshold is %u \n", threshold);
    }
    else
        return RT_ERR_FAILED;

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_ingress_egress_drop_port_gap_queue_gap_drop_thresh */

/*
 * flowctrl set ingress egress-drop port ( <PORT_LIST:ports> | all ) queue ( <PORT_LIST:queues> | all ) drop ( enable | disable )
 */
cparser_result_t
cparser_cmd_flowctrl_set_ingress_egress_drop_port_ports_all_queue_queues_all_drop_enable_disable(
    cparser_context_t *context,
    char * *ports_ptr,
    char * *queues_ptr)
{
    diag_portlist_t portlist;
    diag_mask_t  queuelist;
    rtk_port_t port;
    rtk_qid_t queue;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_QUEUEMASK(queuelist, 7), ret);

    diag_util_mprintf("Set Ingress Flow Control Queue Egress Drop Enable \n");
    diag_util_printf("            ");
    DIAG_UTIL_MASK_SCAN(queuelist, queue)
    {
        diag_util_printf("Q%d    ",queue);
    }
    diag_util_mprintf("\n");
    diag_util_mprintf("---------------------------------------------------------\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_printf("Port%2u  ", port);
        DIAG_UTIL_MASK_SCAN(queuelist, queue)
        {
            if('e'==TOKEN_CHAR(9,0))
            {
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_egressDropEnable_set(port, queue, ENABLED), ret);
                diag_util_printf("    En");
            }
            else if('d'==TOKEN_CHAR(9,0))
            {
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_egressDropEnable_set(port, queue, DISABLED), ret);
                diag_util_printf("   Dis");
            }
	      else
		   return RT_ERR_FAILED;
        }
        diag_util_mprintf("\n");
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_ingress_egress_drop_port_ports_all_queue_queues_all_drop_enable_disable */

/*
 * flowctrl get ingress egress-drop port ( <PORT_LIST:ports> | all ) queue ( <PORT_LIST:queues> | all ) drop
 */
cparser_result_t
cparser_cmd_flowctrl_get_ingress_egress_drop_port_ports_all_queue_queues_all_drop(
    cparser_context_t *context,
    char * *ports_ptr,
    char * *queues_ptr)
{
    diag_portlist_t portlist;
    diag_mask_t  queuelist;
    rtk_port_t port;
    rtk_qid_t queue;
    rtk_enable_t enable;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_QUEUEMASK(queuelist, 7), ret);

    diag_util_mprintf("Get Ingress Flow Control Queue Egress Drop Enable \n");
    DIAG_UTIL_MASK_SCAN(queuelist, queue)
    {
        diag_util_printf("Q%d    ",queue);
    }
    diag_util_mprintf("\n");
    diag_util_mprintf("---------------------------------------------------------\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_printf("Port %2u ", port);
        DIAG_UTIL_MASK_SCAN(queuelist, queue)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_egressDropEnable_get(port, queue, &enable), ret);
                diag_util_printf("   %3s",enable?"En":"Dis");
        }
        diag_util_mprintf("\n");
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_ingress_egress_drop_port_ports_all_queue_queues_all_drop */

/*
 * flowctrl set ingress pon global ( high-off | high-on | low-off | low-on ) <UINT:thresh>
 */
cparser_result_t
cparser_cmd_flowctrl_set_ingress_pon_global_high_off_high_on_low_off_low_on_thresh(
    cparser_context_t *context,
    uint32_t  *thresh_ptr)
{
    uint32 onThresh, offThresh;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if(!osal_strcmp(TOKEN_STR(5),"high-off"))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponGlobalHighThreshold_get(&onThresh, &offThresh), ret);
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponGlobalHighThreshold_set(onThresh, *thresh_ptr), ret);
        diag_util_mprintf("Set PON MAC Global High Off Threshold to %u\n",*thresh_ptr);
    }
    else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponGlobalHighThreshold_get(&onThresh, &offThresh), ret);
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponGlobalHighThreshold_set(*thresh_ptr, offThresh), ret);
        diag_util_mprintf("Set PON MAC Global High On Threshold to %u\n",*thresh_ptr);
    }
    else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponGlobalLowThreshold_get(&onThresh, &offThresh), ret);
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponGlobalLowThreshold_set(onThresh, *thresh_ptr), ret);
        diag_util_mprintf("Set PON MAC Global Low Off Threshold to %u\n",*thresh_ptr);
    }
    else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponGlobalLowThreshold_get(&onThresh, &offThresh), ret);
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponGlobalLowThreshold_set(*thresh_ptr, offThresh), ret);
        diag_util_mprintf("Set PON MAC Global Low On Threshold to %u\n",*thresh_ptr);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_ingress_pon_global_high_off_high_on_low_off_low_on_thresh */

/*
 * flowctrl get ingress pon global ( high-off | high-on | low-off | low-on )
 */
cparser_result_t
cparser_cmd_flowctrl_get_ingress_pon_global_high_off_high_on_low_off_low_on(
    cparser_context_t *context)
{
    uint32 onThresh, offThresh;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if(!osal_strcmp(TOKEN_STR(5),"high-off"))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponGlobalHighThreshold_get(&onThresh, &offThresh), ret);
        diag_util_mprintf("PON MAC Global High Off Threshold is %u\n",offThresh);
    }
    else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponGlobalHighThreshold_get(&onThresh, &offThresh), ret);
        diag_util_mprintf("PON MAC Global High On Threshold is %u\n",onThresh);
    }
    else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponGlobalLowThreshold_get(&onThresh, &offThresh), ret);
        diag_util_mprintf("PON MAC Global Low Off Threshold is %u\n",offThresh);
    }
    else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponGlobalLowThreshold_get(&onThresh, &offThresh), ret);
        diag_util_mprintf("PON MAC Global Low On Threshold is %u\n",onThresh);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_ingress_pon_global_high_off_high_on_low_off_low_on */

/*
 * flowctrl set ingress pon port ( high-off | high-on | low-off | low-on ) <UINT:thresh>
 */
cparser_result_t
cparser_cmd_flowctrl_set_ingress_pon_port_high_off_high_on_low_off_low_on_thresh(
    cparser_context_t *context,
    uint32_t  *thresh_ptr)
{
    uint32 onThresh, offThresh;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if(!osal_strcmp(TOKEN_STR(5),"high-off"))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponPortHighThreshold_get(&onThresh, &offThresh), ret);
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponPortHighThreshold_set(onThresh, *thresh_ptr), ret);
        diag_util_mprintf("Set PON MAC Port High Off Threshold to %u\n",*thresh_ptr);
    }
    else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponPortHighThreshold_get(&onThresh, &offThresh), ret);
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponPortHighThreshold_set(*thresh_ptr, offThresh), ret);
        diag_util_mprintf("Set PON MAC Port High On Threshold to %u\n",*thresh_ptr);
    }
    else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponPortLowThreshold_get(&onThresh, &offThresh), ret);
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponPortLowThreshold_set(onThresh, *thresh_ptr), ret);
        diag_util_mprintf("Set PON MAC Port Low Off Threshold to %u\n",*thresh_ptr);
    }
    else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponPortLowThreshold_get(&onThresh, &offThresh), ret);
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponPortLowThreshold_set(*thresh_ptr, offThresh), ret);
        diag_util_mprintf("Set PON MAC Port Low On Threshold to %u\n",*thresh_ptr);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_ingress_pon_port_high_off_high_on_low_off_low_on_thresh */

/*
 * flowctrl get ingress pon port ( high-off | high-on | low-off | low-on )
 */
cparser_result_t
cparser_cmd_flowctrl_get_ingress_pon_port_high_off_high_on_low_off_low_on(
    cparser_context_t *context)
{
    uint32 onThresh, offThresh;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if(!osal_strcmp(TOKEN_STR(5),"high-off"))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponPortHighThreshold_get(&onThresh, &offThresh), ret);
        diag_util_mprintf("PON MAC Port High Off Threshold is %u\n",offThresh);
    }
    else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponPortHighThreshold_get(&onThresh, &offThresh), ret);
        diag_util_mprintf("PON MAC Port High On Threshold is %u\n",onThresh);
    }
    else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponPortLowThreshold_get(&onThresh, &offThresh), ret);
        diag_util_mprintf("PON MAC Port Low Off Threshold is %u\n",offThresh);
    }
    else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponPortLowThreshold_get(&onThresh, &offThresh), ret);
        diag_util_mprintf("PON MAC Port Low On Threshold is %u\n",onThresh);
    }


    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_ingress_pon_port_high_off_high_on_low_off_low_on */

/*
 * flowctrl set ingress pon egress-drop queue ( <PORT_LIST:queues> | all ) thresh-idx <UINT:idx>
 */
cparser_result_t
cparser_cmd_flowctrl_set_ingress_pon_egress_drop_queue_queues_all_thresh_idx_idx(
    cparser_context_t *context,
    char * *queues_ptr,
    uint32_t  *idx_ptr)
{
    diag_mask_t  queuelist;
    rtk_qid_t queue;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK128(queuelist, 6), ret);
    DIAG_UTIL_MASK_SCAN(queuelist, queue)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponQueueEegressDropIndex_set(queue, *idx_ptr), ret);
    }
    diag_util_mprintf("Queues:%s  Threshold index:%u\n",context->parser->tokens[(6)].buf, *idx_ptr);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_ingress_pon_egress_drop_queue_queues_all_thresh_idx_idx */

/*
 * flowctrl get ingress pon egress-drop queue ( <PORT_LIST:queues> | all )
 */
cparser_result_t
cparser_cmd_flowctrl_get_ingress_pon_egress_drop_queue_queues_all(
    cparser_context_t *context,
    char * *queues_ptr)
{
    diag_mask_t  queuelist;
    rtk_qid_t queue;
    uint32 idx;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK128(queuelist, 6), ret);

    diag_util_mprintf("Get Ingress Flow Control Queue Egress Drop Threshold Index \n");
    diag_util_mprintf("Egress-Queue       Drop-Threshold-Index \n");
    diag_util_mprintf("---------------------------------\n");
    DIAG_UTIL_MASK_SCAN(queuelist, queue)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponQueueEegressDropIndex_get(queue, &idx), ret);
        diag_util_mprintf("%-18u  %u\n", queue, idx);
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_ingress_pon_egress_drop_queue_queues_all */

/*
 * flowctrl set ingress pon egress-drop queue-thresh-idx ( <PORT_LIST:idx> | all ) <UINT:thresh>
 */
cparser_result_t
cparser_cmd_flowctrl_set_ingress_pon_egress_drop_queue_thresh_idx_idx_all_thresh(
    cparser_context_t *context,
    char * *idx_ptr,
    uint32_t  *thresh_ptr)
{
    diag_mask_t  idxlist;
    uint32  idx;
    uint8   buf[UTIL_PORT_MASK_BUFFER_LENGTH];
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK8(idxlist, 6), ret);
    DIAG_UTIL_MASK_SCAN(idxlist, idx)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponQueueEegressDropThreshold_set(idx, *thresh_ptr), ret);
    }
    diag_util_mask2str (buf, idxlist.mask.bits[0]);
    diag_util_mprintf("Threshold Index:%s Threshold:%u\n",buf, *thresh_ptr);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_ingress_pon_egress_drop_queue_thresh_idx_idx_all_thresh */

/*
 * flowctrl get ingress pon egress-drop queue-thresh-idx ( <PORT_LIST:idx> | all )
 */
cparser_result_t
cparser_cmd_flowctrl_get_ingress_pon_egress_drop_queue_thresh_idx_idx_all(
    cparser_context_t *context,
    char * *idx_ptr)
{
    diag_mask_t  idxlist;
    uint32 idx;
    uint32 threshold;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK8(idxlist, 6), ret);

    diag_util_mprintf("Get Ingress Flow Control Queue Egress Drop Threshold  \n");
    diag_util_mprintf("Index           Drop-Threshold \n");
    diag_util_mprintf("---------------------------------\n");
    DIAG_UTIL_MASK_SCAN(idxlist, idx)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponQueueEegressDropThreshold_get(idx, &threshold), ret);
        diag_util_mprintf("%-18u  %u\n", idx, threshold);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_ingress_pon_egress_drop_queue_thresh_idx_idx_all */

/*
 * flowctrl set ingress pon egress-drop queue-gap <UINT:thresh>
 */
cparser_result_t
cparser_cmd_flowctrl_set_ingress_pon_egress_drop_queue_gap_thresh(
    cparser_context_t *context,
    uint32_t  *thresh_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponQueueEegressGapThreshold_set(*thresh_ptr), ret);
    diag_util_mprintf("Set PON MAC Queue Egress Gap Threshold to %u \n", *thresh_ptr);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_ingress_pon_egress_drop_queue_gap_thresh */

/*
 * flowctrl get ingress pon egress-drop queue-gap
 */
cparser_result_t
cparser_cmd_flowctrl_get_ingress_pon_egress_drop_queue_gap(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    uint32 threshold;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponQueueEegressGapThreshold_get(&threshold), ret);
    diag_util_mprintf("Ingress Flow Control PON MAC Egress Gap Threshold is %u \n", threshold);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_ingress_pon_egress_drop_queue_gap */

/*
 * flowctrl set max-page-clear egress-port ( <PORT_LIST:ports> | all )
 */
cparser_result_t
cparser_cmd_flowctrl_set_max_page_clear_egress_port_ports_all(
    cparser_context_t *context,
    char * *ports_ptr)
{
    diag_portlist_t  portlist;
    rtk_bmp_t  mask;
    rtk_port_t port;
    uint8   buf[UTIL_PORT_MASK_BUFFER_LENGTH];
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    mask.bits[0] = 0;
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_debugCtrl_set(port, ENABLED, mask), ret);
    }
    diag_util_lPortMask2str(buf, &portlist.portmask);
    diag_util_mprintf("Port:%s Egress Port Max Page Clear.\n",buf);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_max_page_clear_egress_port_ports_all */

/*
 * flowctrl set total-pktcnt-clear
 */
cparser_result_t
cparser_cmd_flowctrl_set_total_pktcnt_clear(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_clrTotalPktCnt_set(ENABLED), ret);

    diag_util_mprintf("Set Total Packet Count Clear.\n");

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_total_pktcnt_clear_ports_all */

/*
 * flowctrl set max-page-clear used-page
 */
cparser_result_t
cparser_cmd_flowctrl_set_max_page_clear_used_page(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_clrMaxUsedPageCnt_set(ENABLED), ret);
    diag_util_mprintf("Maximum Used Paged Count is cleared\n");
    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_max_page_clear_used_page */

/*
 * flowctrl set max-page-clear egress-queue port ( <PORT_LIST:ports> | all ) queue ( <MASK_LIST:queues> | all )
 */
cparser_result_t
cparser_cmd_flowctrl_set_max_page_clear_egress_queue_port_ports_all_queue_queues_all(
    cparser_context_t *context,
    char * *ports_ptr)
{
    diag_portlist_t  portlist;
    diag_mask_t  queuelist;
    rtk_port_t port;
    uint32 idx;
    uint8   buf[UTIL_PORT_MASK_BUFFER_LENGTH];
    uint8   buf2[UTIL_PORT_MASK_BUFFER_LENGTH];
    int32 ret = RT_ERR_FAILED;

    osal_memset(&queuelist, 0, sizeof(diag_mask_t));
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_QUEUEMASK(queuelist, 7) , ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_debugCtrl_set(port, DISABLED, queuelist.mask), ret);
    }
    diag_util_lPortMask2str(buf, &portlist.portmask);
    diag_util_mask2str(buf2, queuelist.mask.bits[0]);
    diag_util_mprintf("Port:%s Egress queue: %s Max Page Clear.\n",buf, buf2);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_max_page_clear_egress_queue_ports_all */

/*
 * flowctrl get used-page-cnt ( ingress | egress ) port ( <PORT_LIST:ports> | all )
 */
cparser_result_t
cparser_cmd_flowctrl_get_used_page_cnt_ingress_egress_port_ports_all(
    cparser_context_t *context,
    char * *ports_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    uint32 current, maximum;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    if(!osal_strcmp(TOKEN_STR(3),"ingress"))
    {
        diag_util_mprintf("ingress-port     current-page      maximum-page\n");
        diag_util_mprintf("-----------------------------------------------------\n");

        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portUsedPageCnt_get(port, &current, &maximum), ret);
            diag_util_mprintf("    %-18u %-18u%u\n", port, current, maximum);
        }
    }
    else if(!osal_strcmp(TOKEN_STR(3),"egress"))
    {
        diag_util_mprintf("egress-port     current-page      maximum-page\n");
        diag_util_mprintf("-----------------------------------------------------\n");
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portEgressUsedPageCnt_get(port, &current, &maximum), ret);
            diag_util_mprintf("    %-18u %-18u%u\n", port, current, maximum);
        }
    }


    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_used_page_cnt_ingress_egress_port_ports_all */

/*
 * flowctrl get used-page-cnt egress-queue port ( <PORT_LIST:ports> | all ) queue ( <PORT_LIST:queues> | all )
 */
cparser_result_t
cparser_cmd_flowctrl_get_used_page_cnt_egress_queue_port_ports_all_queue_queues_all(
    cparser_context_t *context,
    char * *ports_ptr,
    char * *queues_ptr)
{
    diag_portlist_t portlist;
    diag_mask_t  queuelist;
    rtk_port_t port;
    rtk_qid_t queue;
    uint32 current, maximum[8];
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_QUEUEMASK(queuelist, 7), ret);

    diag_util_mprintf("Get Ingress Flow Control Queue Egress Drop Queue Page Count \n");
    diag_util_printf("                     ");
    DIAG_UTIL_MASK_SCAN(queuelist, queue)
    {
        diag_util_printf("Q%d      ",queue);
    }
    diag_util_mprintf("\n");
    diag_util_mprintf("------------------------------------------------------------------------------------\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_printf("Port%2u  Current   ", port);
        DIAG_UTIL_MASK_SCAN(queuelist, queue)
        {
                DIAG_UTIL_ERR_CHK( apollo_raw_flowctrl_queueEgressUsedPageCnt_get(port, queue, &current, &maximum[queue]), ret);
                diag_util_printf("%5u   ", current);
        }
        diag_util_mprintf("\n");
        diag_util_printf("        Maximum   ");
        DIAG_UTIL_MASK_SCAN(queuelist, queue)
        {
                diag_util_printf("%5u   ", maximum[queue]);
        }
        diag_util_mprintf("\n");

    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_used_page_cnt_egress_queue_port_ports_all_queue_queues_all */

/*
 * flowctrl get total-page-cnt
 */
cparser_result_t
cparser_cmd_flowctrl_get_total_page_cnt(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    uint32 count;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_totalPageCnt_get(&count), ret);
    diag_util_mprintf("Total page Count for Packet: Current  %u\n", count);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_total_page_cnt */

/*
 * flowctrl get used-page-cnt ( total | public | public-off | public-jumbo )
 */
cparser_result_t
cparser_cmd_flowctrl_get_used_page_cnt_total_public_public_off_public_jumbo(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    uint32 current, maximum;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if(!osal_strcmp(TOKEN_STR(3),"total"))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_totalUsedPageCnt_get(&current, &maximum), ret);
        diag_util_mprintf("Total page Count: Current  %u maximum  %u\n", current, maximum);
    }
    else if(!osal_strcmp(TOKEN_STR(3),"public"))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_publicUsedPageCnt_get(&current, &maximum), ret);
        diag_util_mprintf("Public page Count: Current  %u maximum  %u\n", current, maximum);
    }
    else if(!osal_strcmp(TOKEN_STR(3),"public-off"))
    {
        diag_util_mprintf("-----------------------------------------------------\n");
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_publicFcoffUsedPageCnt_get(&current, &maximum), ret);
        diag_util_mprintf("Public page Count for FC-off: Current  %u maximum  %u\n", current, maximum);
    }
    else if(!osal_strcmp(TOKEN_STR(3),"public-jumbo"))
    {
        diag_util_mprintf("-----------------------------------------------------\n");
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_publicJumboUsedPageCnt_get(&current, &maximum), ret);
        diag_util_mprintf("Public page Count for jumbo frame: Current  %u maximum  %u\n", current, maximum);
    }


    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_used_page_cnt_total_public_public_off_public_jumbo */

/*
 * flowctrl get used-page-cnt packet ( <PORT_LIST:ports> | all )
 */
cparser_result_t
cparser_cmd_flowctrl_get_used_page_cnt_packet_ports_all(
    cparser_context_t *context,
    char * *ports_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    uint32 count;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

        diag_util_mprintf("ingress-port     page    \n");
        diag_util_mprintf("-------------------------------\n");

        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portPacketPageCnt_get(port, &count), ret);
            diag_util_mprintf("    %-18u %u\n", port, count);
        }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_used_page_cnt_packet_ports_all */


/*
 * flowctrl get used-page-cnt pon queue ( <PORT_LIST:queues> | all )
 */
cparser_result_t
cparser_cmd_flowctrl_get_used_page_cnt_pon_queue_queues_all(
    cparser_context_t *context,
    char * *queues_ptr)
{
    diag_mask_t  queuelist;
    rtk_qid_t queue;
    uint32 current, maximum;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK128(queuelist, 5), ret);
    diag_util_mprintf("PON queue        current-page      maximum-page\n");
    diag_util_mprintf("-----------------------------------------------------\n");
    DIAG_UTIL_MASK_SCAN(queuelist, queue)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponUsedPage_get(queue, &current, &maximum), ret);
        diag_util_mprintf("%-18u %-18u%u\n", queue, current, maximum);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_used_page_cnt_pon_queue_queues_all */


/*
 * flowctrl set max-page-clear pon queue <UINT:qid>
 */
cparser_result_t
cparser_cmd_flowctrl_set_max_page_clear_pon_queue_qid(
    cparser_context_t *context,
    uint32_t  *qid_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponUsedPageCtrl_set(*qid_ptr, ENABLED), ret);
    diag_util_mprintf("Maximum PON queue %u is cleared\n",*qid_ptr);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_max_page_clear_pon_queue_qid */

/*
 * flowctrl set prefetch <UINT:thresh>
 */
cparser_result_t
cparser_cmd_flowctrl_set_prefetch_thresh(
    cparser_context_t *context,
    uint32_t  *thresh_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_txPrefet_set(*thresh_ptr), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_prefetch_thresh */

/*
 * flowctrl get prefetch
 */
cparser_result_t
cparser_cmd_flowctrl_get_prefetch(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    uint32 threshold;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_txPrefet_get(&threshold), ret);
    diag_util_mprintf("Prefet Threshold is %u\n", threshold);

    return CPARSER_OK;

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_prefetch */

/*
 * flowctrl set low-queue <UINT:thresh>
 */
cparser_result_t
cparser_cmd_flowctrl_set_low_queue_thresh(
    cparser_context_t *context,
    uint32_t  *thresh_ptr)
{
    int32 ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();
    
    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_lowQueueThreshold_set(*thresh_ptr), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_low_queue_thresh */

/*
 * flowctrl get low-queue
 */
cparser_result_t
cparser_cmd_flowctrl_get_low_queue(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    uint32 threshold;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_lowQueueThreshold_get(&threshold), ret);
    diag_util_mprintf("Low Queue Threshold is %u\n", threshold);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_low_queue */

/*
 * flowctrl set egress global ( fc-on | fc-off ) ( high-off | high-on | low-off | low-on ) <UINT:thresh>
 */
cparser_result_t
cparser_cmd_flowctrl_set_egress_global_fc_on_fc_off_high_off_high_on_low_off_low_on_thresh(
    cparser_context_t *context,
    uint32_t  *thresh_ptr)
{
    uint32 onThresh, offThresh;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    if('f'==TOKEN_CHAR(4,4))
    {
        if(!osal_strcmp(TOKEN_STR(5),"high-off"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffHighThreshold_get(&onThresh, &offThresh), ret);
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffHighThreshold_set(onThresh, *thresh_ptr), ret);
            diag_util_mprintf("Set Flow Control Global FC-Off High Off Threshold to %u\n",*thresh_ptr);
        }
        else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffHighThreshold_get(&onThresh, &offThresh), ret);
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffHighThreshold_set(*thresh_ptr, offThresh), ret);
            diag_util_mprintf("Set Flow Control Global FC-Off High On Threshold to %u\n",*thresh_ptr);
        }
        else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffLowThreshold_get(&onThresh, &offThresh), ret);
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffLowThreshold_set(onThresh, *thresh_ptr), ret);
            diag_util_mprintf("Set Flow Control Global FC-Off Low Off Threshold to %u\n",*thresh_ptr);
        }
        else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffLowThreshold_get(&onThresh, &offThresh), ret);
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffLowThreshold_set(*thresh_ptr, offThresh), ret);
            diag_util_mprintf("Set Flow Control Global FC-Off Low On Threshold to %u\n",*thresh_ptr);
        }
    }
    else if('n'==TOKEN_CHAR(4,4))
    {
        if(!osal_strcmp(TOKEN_STR(5),"high-off"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconHighThreshold_get(&onThresh, &offThresh), ret);
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconHighThreshold_set(onThresh, *thresh_ptr), ret);
            diag_util_mprintf("Set Flow Control Global FC-On High Off Threshold to %u\n",*thresh_ptr);
        }
        else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconHighThreshold_get(&onThresh, &offThresh), ret);
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconHighThreshold_set(*thresh_ptr, offThresh), ret);
            diag_util_mprintf("Set Flow Control Global FC-On High On Threshold to %u\n",*thresh_ptr);
        }
        else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconLowThreshold_get(&onThresh, &offThresh), ret);
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconLowThreshold_set(onThresh, *thresh_ptr), ret);
            diag_util_mprintf("Set Flow Control Global FC-On Low Off Threshold to %u\n",*thresh_ptr);
        }
        else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconLowThreshold_get(&onThresh, &offThresh), ret);
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconLowThreshold_set(*thresh_ptr, offThresh), ret);
            diag_util_mprintf("Set Flow Control Global FC-On Low On Threshold to %u\n",*thresh_ptr);
        }
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_egress_global_fc_on_fc_off_high_off_high_on_low_off_low_on_thresh */

/*
 * flowctrl get egress global ( fc-on | fc-off ) ( high-off | high-on | low-off | low-on )
 */
cparser_result_t
cparser_cmd_flowctrl_get_egress_global_fc_on_fc_off_high_off_high_on_low_off_low_on(
    cparser_context_t *context)
{
    uint32 onThresh, offThresh;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if('f'==TOKEN_CHAR(4,4))
    {
        if(!osal_strcmp(TOKEN_STR(5),"high-off"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffHighThreshold_get(&onThresh, &offThresh), ret);
            diag_util_mprintf("Flow Control Global FC-Off High Off Threshold is %u\n",offThresh);
        }
        else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffHighThreshold_get(&onThresh, &offThresh), ret);
            diag_util_mprintf("Flow Control Global FC-Off High On Threshold is %u\n",onThresh);
        }
        else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffLowThreshold_get(&onThresh, &offThresh), ret);
            diag_util_mprintf("Flow Control Global FC-Off Low Off Threshold is %u\n",offThresh);
        }
        else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffLowThreshold_get(&onThresh, &offThresh), ret);
            diag_util_mprintf("Flow Control Global FC-Off Low On Threshold is %u\n",onThresh);
        }
    }
    else if('n'==TOKEN_CHAR(4,4))
    {
        if(!osal_strcmp(TOKEN_STR(5),"high-off"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconHighThreshold_get(&onThresh, &offThresh), ret);
            diag_util_mprintf("Flow Control Global FC-On High Off Threshold is %u\n",offThresh);
        }
        else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconHighThreshold_get(&onThresh, &offThresh), ret);
            diag_util_mprintf("Flow Control Global FC-On High On Threshold is %u\n",onThresh);
        }
        else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconLowThreshold_get(&onThresh, &offThresh), ret);
            diag_util_mprintf("Flow Control Global FC-On Low Off Threshold is %u\n",offThresh);
        }
        else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconLowThreshold_get(&onThresh, &offThresh), ret);
            diag_util_mprintf("Flow Control Global FC-On Low On Threshold is %u\n",onThresh);
        }
    }


    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_egress_global_fc_on_fc_off_high_off_high_on_low_off_low_on */

/*
 * flowctrl set egress queue ( <PORT_LIST:queues> | all ) <UINT:thresh>
 */
cparser_result_t
cparser_cmd_flowctrl_set_egress_queue_queues_all_thresh(
    cparser_context_t *context,
    char * *queues_ptr,
    uint32_t  *thresh_ptr)
{
    diag_mask_t  queuelist;
    rtk_qid_t queue;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_QUEUEMASK(queuelist, 4), ret);

    diag_util_mprintf("Set Egress Flow Control Queue  Threshold \n");
    diag_util_mprintf("Egress-Queue       Drop-Threshold \n");
    diag_util_mprintf("---------------------------------\n");
    DIAG_UTIL_MASK_SCAN(queuelist, queue)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_queueEegressDropThreshold_set(queue, *thresh_ptr), ret);
        diag_util_mprintf("%-18u  %u\n", queue, *thresh_ptr);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_egress_queue_queues_all_thresh */

/*
 * flowctrl get egress queue ( <PORT_LIST:queues> | all )
 */
cparser_result_t
cparser_cmd_flowctrl_get_egress_queue_queues_all(
    cparser_context_t *context,
    char * *queues_ptr)
{
    diag_mask_t  queuelist;
    rtk_qid_t queue;
    uint32 threshold;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_QUEUEMASK(queuelist, 4), ret);

    diag_util_mprintf("Get Egress Flow Control Queue Threshold \n");
    diag_util_mprintf("Egress-Queue       Drop-Threshold \n");
    diag_util_mprintf("---------------------------------\n");
    DIAG_UTIL_MASK_SCAN(queuelist, queue)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_queueEegressDropThreshold_get(queue, &threshold), ret);
        diag_util_mprintf("%-18u  %u\n", queue, threshold);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_egress_queue_queues_all */

/*
 * flowctrl set egress port ( <PORT_LIST:ports> | all ) <UINT:thresh>
 */
cparser_result_t
cparser_cmd_flowctrl_set_egress_port_ports_all_thresh(
    cparser_context_t *context,
    char * *ports_ptr,
    uint32_t  *thresh_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    diag_util_mprintf("Set Egress  Flow Control Port Threshold \n");
    diag_util_mprintf("Egress-Port       Drop-Threshold \n");
    diag_util_mprintf("-----------------------------\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portEegressDropThreshold_set(port, *thresh_ptr), ret);
        diag_util_mprintf("%-18u  %u\n", port, *thresh_ptr);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_egress_port_ports_all_thresh */

/*
 * flowctrl get egress port ( <PORT_LIST:ports> | all )
 */
cparser_result_t
cparser_cmd_flowctrl_get_egress_port_ports_all(
    cparser_context_t *context,
    char * *ports_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    uint32 threshold;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    diag_util_mprintf("Get Egress Flow Control Port Threshold \n");
    diag_util_mprintf("Egress-Port       Drop-Threshold \n");
    diag_util_mprintf("-----------------------------\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portEegressDropThreshold_get(port, &threshold), ret);
        diag_util_mprintf("%-18u  %u\n", port, threshold);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_egress_port_ports_all */

/*
 * flowctrl set egress port-gap <UINT:thresh>
 */
cparser_result_t
cparser_cmd_flowctrl_set_egress_port_gap_thresh(
    cparser_context_t *context,
    uint32_t  *thresh_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portEegressGapThreshold_set(*thresh_ptr), ret);
    diag_util_mprintf("Set Egress Flow Control Port Gap Threshold to %u \n", *thresh_ptr);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_egress_port_gap_thresh */

/*
 * flowctrl get egress port-gap
 */
cparser_result_t
cparser_cmd_flowctrl_get_egress_port_gap(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    uint32 threshold;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portEegressGapThreshold_get(&threshold), ret);
    diag_util_mprintf("Egress Flow Control Port  Gap Threshold is %u \n", threshold);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_egress_port_gap */

/*
 * flowctrl set egress queue-gap <UINT:thresh>
 */
cparser_result_t
cparser_cmd_flowctrl_set_egress_queue_gap_thresh(
    cparser_context_t *context,
    uint32_t  *thresh_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_queueEegressGapThreshold_set(*thresh_ptr), ret);
    diag_util_mprintf("Set Egress Flow Control Queue Gap Threshold to %u \n", *thresh_ptr);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_egress_queue_gap_thresh */

/*
 * flowctrl get egress queue-gap */
cparser_result_t
cparser_cmd_flowctrl_get_egress_queue_gap(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    uint32 threshold;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_queueEegressGapThreshold_get(&threshold), ret);
    diag_util_mprintf("Egress Flow Control Queue Gap Threshold is %u \n", threshold);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_egress_queue_gap */

/*
 * flowctrl set high-queue port ( <PORT_LIST:port> | all ) queue-mask ( <PORT_LIST:queue> | all )
 */
cparser_result_t
cparser_cmd_flowctrl_set_high_queue_port_port_all_queue_mask_queue_all(
    cparser_context_t *context,
    char * *port_ptr,
    char * *queue_ptr)
{
    int32           ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    diag_portlist_t    portlist;
    diag_mask_t    queueMask;
    uint8   buf1[UTIL_PORT_MASK_BUFFER_LENGTH];
    uint8   buf2[UTIL_PORT_MASK_BUFFER_LENGTH];

    osal_memset(&queueMask, 0, sizeof(diag_mask_t));
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    RT_PARAM_CHK((NULL == *queue_ptr), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((NULL == *port_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_QUEUEMASK(queueMask, 6), ret);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_highQueueMask_set(port, queueMask.mask), ret);
    }

    diag_util_lPortMask2str(buf1, &portlist.portmask);
    diag_util_mask2str (buf2, queueMask.mask.bits[0]);
    diag_util_mprintf("Set High Queue Mask: Port %s Queues:%s \n",buf1,buf2);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_high_queue_port_port_all_queue_mask_queue_all */

/*
 * flowctrl get high-queue port ( <PORT_LIST:port> | all )
 */
cparser_result_t
cparser_cmd_flowctrl_get_high_queue_port_port_all(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32               ret = RT_ERR_FAILED;
    rtk_port_t          port = 0;
    diag_portlist_t     portlist;
    rtk_bmp_t queueMask;

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((NULL == *port_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_OUTPUT_INIT();

    /* show specific port isolation info */
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    diag_util_mprintf("Get High Queue Mask\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_highQueueMask_get(port, &queueMask), ret);
        diag_util_mprintf("\tPort %2u : queue-mask = 0x%x\n", port, queueMask.bits[0]);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_high_queue_port_port_all */


/*
 * flowctrl set patch ( gpon-35m | fiber-35m | 20m )
 */
cparser_result_t
cparser_cmd_flowctrl_set_patch_gpon_35m_fiber_35m_20m(
    cparser_context_t *context)
{
    rtk_flowctrl_patch_t type;
    int32 ret;

    switch(TOKEN_CHAR(3,0))
    {
        case 'g':
            type = FLOWCTRL_PATCH_35M_GPON;
            break;
        case 'f':
            type = FLOWCTRL_PATCH_35M_FIBER;
            break;
        case '2':
            type = FLOWCTRL_PATCH_20M;
            break;
        default:
            return CPARSER_ERR_INVALID_PARAMS;
    }


    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_patch(type), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_patch_gpon_35m_fiber_35m_20m */


