 /*
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: 63513 $
 * $Date: 2015-11-19 13:09:17 +0800 (Thu, 19 Nov 2015) $
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
#include <dal/apollo/raw/apollo_raw_flowctrl.h>
#include <dal/apollomp/raw/apollomp_raw_flowctrl.h>
#include <dal/rtl9601b/dal_rtl9601b_flowctrl.h>
#include <dal/rtl9602c/dal_rtl9602c_flowctrl.h>
#include <dal/rtl9607b/dal_rtl9607b_flowctrl.h>

/*
 * flowctrl dump threshold ( switch | pon )
 */
cparser_result_t
cparser_cmd_flowctrl_dump_threshold_switch_pon(
    cparser_context_t *context)
{
    rtk_enable_t enable;
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
        switch(DIAG_UTIL_CHIP_TYPE)
        {
#ifdef CONFIG_SDK_APOLLO
            case APOLLO_CHIP_ID:
            {
                apollo_raw_flowctrl_type_t type;
                apollo_raw_flowctrl_jumbo_size_t size;
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
            break;
#endif
#ifdef CONFIG_SDK_APOLLOMP
            case APOLLOMP_CHIP_ID:
	     {
                apollomp_raw_flowctrl_type_t type;
                apollomp_raw_flowctrl_jumbo_size_t size;
                /*call API*/
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_type_get(&type), ret);
                diag_util_printf("Flow Control Type is  %s\n",diagStr_flowCtrlType[type]);
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_jumboMode_get(&enable), ret);
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_jumboSize_get(&size), ret);
                diag_util_printf("Jumbo Mode is %s\n",diagStr_enable[enable]);
                diag_util_printf("Jumbo Size is %s\n",diagStr_flowCtrlJumboSize[size]);
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_dropAllThreshold_get(&threshold), ret);
                diag_util_printf("Drop All Threshold is %u\n", threshold);
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_pauseAllThreshold_get(&threshold), ret);
                diag_util_printf("Pause All Threshold is %u\n", threshold);

                diag_util_printf("\nFlow Control On-Off Threshold\n");
                diag_util_printf("                        On Threshold     Off-Threshold\n");
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFconHighThreshold_get(&onThresh, &offThresh), ret);
                diag_util_printf("Global High FC-On            %-4u              %-4u\n",onThresh,offThresh);
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFconLowThreshold_get(&onThresh, &offThresh), ret);
                diag_util_printf("Global Low FC-On             %-4u              %-4u\n",onThresh,offThresh);
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portFconHighThreshold_get(&onThresh, &offThresh), ret);
                diag_util_printf("Port High FC-On              %-4u              %-4u\n",onThresh,offThresh);
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portFconLowThreshold_get(&onThresh, &offThresh), ret);
                diag_util_printf("Port Low FC-On               %-4u              %-4u\n",onThresh,offThresh);
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                diag_util_printf("Global High FC-Off           %-4u              %-4u\n",onThresh,offThresh);
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                diag_util_printf("Global Low FC-Off            %-4u              %-4u\n",onThresh,offThresh);
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                diag_util_printf("Port High FC-Off             %-4u              %-4u\n",onThresh,offThresh);
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                diag_util_printf("Port Low FC-Off              %-4u              %-4u\n",onThresh,offThresh);
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_jumboGlobalHighThreshold_get(&onThresh, &offThresh), ret);
                diag_util_printf("Jumbo Global High            %-4u              %-4u\n",onThresh,offThresh);
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_jumboGlobalLowThreshold_get(&onThresh, &offThresh), ret);
                diag_util_printf("Jumbo Global Low             %-4u              %-4u\n",onThresh,offThresh);
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_jumboPortHighThreshold_get(&onThresh, &offThresh), ret);
                diag_util_printf("Jumbo Port High              %-4u              %-4u\n",onThresh,offThresh);
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_jumboPortLowThreshold_get(&onThresh, &offThresh), ret);
                diag_util_printf("Jumbo Port Low               %-4u              %-4u\n",onThresh,offThresh);
                diag_util_printf("\n");
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portEegressGapThreshold_get(&threshold), ret);
                diag_util_printf("Port Egress Gap Threshold is %u \n", threshold);
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_queueEegressGapThreshold_get(&threshold), ret);
                diag_util_printf("Queue Egress Gap Threshold is %u \n", threshold);

                portlist.min =0;
                portlist.max = 6;
                portlist.portmask.bits[0] = 0x7F;
                diag_util_printf("Egress-Port       Drop-Threshold \n");
                diag_util_printf("-----------------------------\n");
                DIAG_UTIL_PORTMASK_SCAN(portlist, port)
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portEegressDropThreshold_get(port, &threshold), ret);
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
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_queueEegressDropThreshold_get(queue, &threshold), ret);
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
                        DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_egressDropEnable_get(port, queue, &enable), ret);
                            diag_util_printf("   %3s",enable?"En":"Dis");
                    }
                    diag_util_printf("\n");
                }
            }
            break;
#endif
#ifdef CONFIG_SDK_RTL9601B
            case RTL9601B_CHIP_ID:
	     {
                /*call API*/
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_dropAllThreshold_get(&threshold), ret);
                diag_util_printf("Drop All Threshold is %u\n", threshold);
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_pauseAllThreshold_get(&threshold), ret);
                diag_util_printf("Pause All Threshold is %u\n", threshold);

                diag_util_printf("\nFlow Control On-Off Threshold\n");
                diag_util_printf("                        On Threshold     Off-Threshold\n");
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_globalFconHighThreshold_get(&onThresh, &offThresh), ret);
                diag_util_printf("Global High FC-On            %-4u              %-4u\n",onThresh,offThresh);
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_globalFconLowThreshold_get(&onThresh, &offThresh), ret);
                diag_util_printf("Global Low FC-On             %-4u              %-4u\n",onThresh,offThresh);
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_portFconHighThreshold_get(&onThresh, &offThresh), ret);
                diag_util_printf("Port High FC-On              %-4u              %-4u\n",onThresh,offThresh);
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_portFconLowThreshold_get(&onThresh, &offThresh), ret);
                diag_util_printf("Port Low FC-On               %-4u              %-4u\n",onThresh,offThresh);
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_globalFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                diag_util_printf("Global High FC-Off           %-4u              %-4u\n",onThresh,offThresh);
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_globalFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                diag_util_printf("Global Low FC-Off            %-4u              %-4u\n",onThresh,offThresh);
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_portFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                diag_util_printf("Port High FC-Off             %-4u              %-4u\n",onThresh,offThresh);
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_portFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                diag_util_printf("Port Low FC-Off              %-4u              %-4u\n",onThresh,offThresh);
                diag_util_printf("\n");
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_portEegressGapThreshold_get(&threshold), ret);
                diag_util_printf("Port Egress Gap Threshold is %u \n", threshold);
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_queueEegressGapThreshold_get(&threshold), ret);
                diag_util_printf("Queue Egress Gap Threshold is %u \n", threshold);

                portlist.min =0;
                portlist.max = 6;
                portlist.portmask.bits[0] = 0x7;
                diag_util_printf("Egress-Port       Drop-Threshold \n");
                diag_util_printf("-----------------------------\n");
                DIAG_UTIL_PORTMASK_SCAN(portlist, port)
                {
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_portEegressDropThreshold_get(port, &threshold), ret);
                    diag_util_printf("%-18u  %u\n", port, threshold);
                }
                diag_util_printf("\n");
                queuelist.min = 0;
                queuelist.max =  7;
                queuelist.mask.bits[0] = 0xFF;
                diag_util_printf("Egress-Queue       Drop-Threshold \n");
                diag_util_printf("---------------------------------\n");
                DIAG_UTIL_MASK_SCAN(queuelist, queue)
                {
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_queueEegressDropThreshold_get(queue, &threshold), ret);
                    diag_util_printf("%-18u  %u\n", queue, threshold);
                }

                diag_util_printf("Queue Egress Drop Enable \n");
                diag_util_printf("            ");
                for(queue=0;queue<8; queue++)
                {
                    diag_util_printf("Q%d    ",queue);
                }
                diag_util_printf("\n");
                diag_util_printf("---------------------------------------------------------\n");
                for(port=0;port<=2; port++)
                {
                    diag_util_printf("Port %2u ", port);
                    for(queue=0;queue<8; queue++)
                    {
                        DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_egressDropEnable_get(port, queue, &enable), ret);
                            diag_util_printf("   %3s",enable?"En":"Dis");
                    }
                    diag_util_printf("\n");
                }
            }
            break;
#endif
#ifdef CONFIG_SDK_RTL9602C
			case RTL9602C_CHIP_ID:
		 {
				/*call API*/
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_dropAllThreshold_get(&threshold), ret);
				diag_util_printf("Drop All Threshold is %u\n", threshold);
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_pauseAllThreshold_get(&threshold), ret);
				diag_util_printf("Pause All Threshold is %u\n", threshold);

				diag_util_printf("\nFlow Control On-Off Threshold\n");
				diag_util_printf("						  On Threshold	   Off-Threshold\n");
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_globalFconHighThreshold_get(&onThresh, &offThresh), ret);
				diag_util_printf("Global High FC-On 		   %-4u 			 %-4u\n",onThresh,offThresh);
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_globalFconLowThreshold_get(&onThresh, &offThresh), ret);
				diag_util_printf("Global Low FC-On			   %-4u 			 %-4u\n",onThresh,offThresh);
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_portFconHighThreshold_get(&onThresh, &offThresh), ret);
				diag_util_printf("Port High FC-On			   %-4u 			 %-4u\n",onThresh,offThresh);
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_portFconLowThreshold_get(&onThresh, &offThresh), ret);
				diag_util_printf("Port Low FC-On			   %-4u 			 %-4u\n",onThresh,offThresh);
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_globalFcoffHighThreshold_get(&onThresh, &offThresh), ret);
				diag_util_printf("Global High FC-Off		   %-4u 			 %-4u\n",onThresh,offThresh);
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_globalFcoffLowThreshold_get(&onThresh, &offThresh), ret);
				diag_util_printf("Global Low FC-Off 		   %-4u 			 %-4u\n",onThresh,offThresh);
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_portFcoffHighThreshold_get(&onThresh, &offThresh), ret);
				diag_util_printf("Port High FC-Off			   %-4u 			 %-4u\n",onThresh,offThresh);
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_portFcoffLowThreshold_get(&onThresh, &offThresh), ret);
				diag_util_printf("Port Low FC-Off			   %-4u 			 %-4u\n",onThresh,offThresh);
				diag_util_printf("\n");

				portlist.min =0;
				portlist.max = 6;
				portlist.portmask.bits[0] = 0x7;
				diag_util_printf("Egress-Port		Drop-Threshold \n");
				diag_util_printf("-----------------------------\n");
				DIAG_UTIL_PORTMASK_SCAN(portlist, port)
				{
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_portEegressDropThreshold_get(port, &threshold), ret);
					diag_util_printf("%-18u  %u\n", port, threshold);
				}
				diag_util_printf("\n");
				queuelist.min = 0;
				queuelist.max =  7;
				queuelist.mask.bits[0] = 0xFF;
				diag_util_printf("Egress-Queue		 Drop-Threshold \n");
				diag_util_printf("---------------------------------\n");
				DIAG_UTIL_MASK_SCAN(queuelist, queue)
				{
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_queueEegressDropThreshold_get(queue, &threshold), ret);
					diag_util_printf("%-18u  %u\n", queue, threshold);
				}

				diag_util_printf("Queue Egress Drop Enable \n");
				diag_util_printf("			  ");
				DIAG_UTIL_MASK_SCAN(queuelist, queue)
				{
					diag_util_printf("Q%d	 ",queue);
				}
				diag_util_printf("\n");
				diag_util_printf("---------------------------------------------------------\n");
				DIAG_UTIL_PORTMASK_SCAN(portlist, port)
				{
					diag_util_printf("Port %2u ", port);
					DIAG_UTIL_MASK_SCAN(queuelist, queue)
					{
						DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_egressDropEnable_get(port, queue, &enable), ret);
							diag_util_printf("	 %3s",enable?"En":"Dis");
					}
					diag_util_printf("\n");
				}

				diag_util_printf("\nQueue Egress Drop Enable for PON\n");
				diag_util_printf("			  ");
				DIAG_UTIL_MASK_SCAN(queuelist, queue)
				{
					diag_util_printf("Q%d	 ",queue);
				}
				diag_util_printf("\n");
				diag_util_printf("---------------------------------------------------------\n");
				DIAG_UTIL_PORTMASK_SCAN(portlist, port)
				{
					diag_util_printf("Port %2u ", port);
					DIAG_UTIL_MASK_SCAN(queuelist, queue)
					{
						DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_ponEgressDropEnable_get(port, queue, &enable), ret);
							diag_util_printf("	 %3s",enable?"En":"Dis");
					}
					diag_util_printf("\n");
				}
			}
			break;
#endif
#ifdef CONFIG_SDK_RTL9607B
            case RTL9607B_CHIP_ID:
            {
                /*call API*/
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_dropAllThreshold_get(&threshold), ret);
                diag_util_printf("Drop All Threshold is %u\n", threshold);
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_pauseAllThreshold_get(&threshold), ret);
                diag_util_printf("Pause All Threshold is %u\n", threshold);

                diag_util_printf("\nFlow Control On-Off Threshold\n");
                diag_util_printf("                        On Threshold     Off-Threshold\n");
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_globalFconHighThreshold_get(&onThresh, &offThresh), ret);
                diag_util_printf("Global High FC-On            %-4u              %-4u\n",onThresh,offThresh);
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_globalFconLowThreshold_get(&onThresh, &offThresh), ret);
                diag_util_printf("Global Low FC-On             %-4u              %-4u\n",onThresh,offThresh);
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_portFconHighThreshold_get(&onThresh, &offThresh), ret);
                diag_util_printf("Port High FC-On              %-4u              %-4u\n",onThresh,offThresh);
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_portFconLowThreshold_get(&onThresh, &offThresh), ret);
                diag_util_printf("Port Low FC-On               %-4u              %-4u\n",onThresh,offThresh);
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_globalFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                diag_util_printf("Global High FC-Off           %-4u              %-4u\n",onThresh,offThresh);
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_globalFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                diag_util_printf("Global Low FC-Off            %-4u              %-4u\n",onThresh,offThresh);
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_portFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                diag_util_printf("Port High FC-Off             %-4u              %-4u\n",onThresh,offThresh);
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_portFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                diag_util_printf("Port Low FC-Off              %-4u              %-4u\n",onThresh,offThresh);
                diag_util_printf("\n");

                portlist.min = HAL_GET_MIN_PORT();
                portlist.max = HAL_GET_MAX_PORT();
                HAL_GET_ALL_PORTMASK(portlist.portmask);
                diag_util_printf("Egress-Port       Drop-Threshold \n");
                diag_util_printf("-----------------------------\n");
                DIAG_UTIL_PORTMASK_SCAN(portlist, port)
                {
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_portEegressDropThreshold_get(port, &threshold), ret);
                    diag_util_printf("%-18u  %u\n", port, threshold);
                }
                diag_util_printf("\n");
                queuelist.min = HAL_MIN_NUM_OF_QUEUE() - 1;
                queuelist.max =  HAL_MAX_NUM_OF_QUEUE() - 1;
                queuelist.mask.bits[0] = 0xFF;
                //DIAG_UTIL_EXTRACT_QUEUEMASK(queuelist, 7);
                diag_util_printf("Egress-Queue       Drop-Threshold \n");
                diag_util_printf("---------------------------------\n");
                DIAG_UTIL_MASK_SCAN(queuelist, queue)
                {
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_queueEegressDropThreshold_get(queue, &threshold), ret);
                    diag_util_printf("%-18u  %u\n", queue, threshold);
                }

                diag_util_printf("Queue Egress Drop Enable \n");
                diag_util_printf("            ");
                DIAG_UTIL_MASK_SCAN(queuelist, queue)
                {
                    diag_util_printf("Q%d    ",queue);
                }
                diag_util_printf("\n");
                diag_util_printf("---------------------------------------------------------\n");
                DIAG_UTIL_PORTMASK_SCAN(portlist, port)
                {
                    diag_util_printf("Port %2u ", port);
                    DIAG_UTIL_MASK_SCAN(queuelist, queue)
                    {
                        DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_egressDropEnable_get(port, queue, &enable), ret);
                            diag_util_printf("   %3s",enable?"En":"Dis");
                    }
                    diag_util_printf("\n");
                }

                diag_util_printf("\nQueue Egress Drop Enable for PON\n");
                diag_util_printf("            ");
                DIAG_UTIL_MASK_SCAN(queuelist, queue)
                {
                    diag_util_printf("Q%d    ",queue);
                }
                diag_util_printf("\n");
                diag_util_printf("---------------------------------------------------------\n");
            }
            break;
#endif


            default:
                diag_util_mprintf("%s\n",DIAG_STR_NOTSUPPORT);
                return CPARSER_NOT_OK;
            break;
        }
    }
    else if('p'==TOKEN_CHAR(3,0))
    {
        switch(DIAG_UTIL_CHIP_TYPE)
        {
#if defined(CONFIG_SDK_APOLLO)
            case APOLLO_CHIP_ID:
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
            break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
            case APOLLOMP_CHIP_ID:
            {
                diag_util_printf("\nPON Flow Control On-Off Threshold\n");
                diag_util_printf("                        On Threshold     Off-Threshold\n");
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponGlobalHighThreshold_get(&onThresh, &offThresh), ret);
                diag_util_printf("Global High                  %-4u              %-4u\n",onThresh,offThresh);
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponGlobalLowThreshold_get(&onThresh, &offThresh), ret);
                diag_util_printf("Global Low                   %-4u              %-4u\n",onThresh,offThresh);
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponPortHighThreshold_get(&onThresh, &offThresh), ret);
                diag_util_printf("Port High                    %-4u              %-4u\n",onThresh,offThresh);
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponPortLowThreshold_get(&onThresh, &offThresh), ret);
                diag_util_printf("Port Low                     %-4u              %-4u\n",onThresh,offThresh);

                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponQueueEegressGapThreshold_get(&threshold), ret);
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
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponQueueEegressDropThreshold_get(idx, &threshold), ret);
                    diag_util_printf("%-18u  %u\n", idx, threshold);
                }

                diag_util_printf("Queue Egress Drop Threshold Index \n");
                diag_util_printf("--------------------------------------------------\n");
                for(queue=0;queue<8; queue++)
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponQueueEegressDropIndex_get(queue, &index[(queue%8)]), ret);
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
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponQueueEegressDropIndex_get(queue, &index[(queue%8)]), ret);
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
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponQueueEegressDropIndex_get(queue, &index[(queue%8)]), ret);
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
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponQueueEegressDropIndex_get(queue, &index[(queue%8)]), ret);
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
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponQueueEegressDropIndex_get(queue, &index[(queue%8)]), ret);
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
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponQueueEegressDropIndex_get(queue, &index[(queue%8)]), ret);
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
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponQueueEegressDropIndex_get(queue, &index[(queue%8)]), ret);
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
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponQueueEegressDropIndex_get(queue, &index[(queue%8)]), ret);
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
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponQueueEegressDropIndex_get(queue, &index[(queue%8)]), ret);
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
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponQueueEegressDropIndex_get(queue, &index[(queue%8)]), ret);
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
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponQueueEegressDropIndex_get(queue, &index[(queue%8)]), ret);
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
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponQueueEegressDropIndex_get(queue, &index[(queue%8)]), ret);
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
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponQueueEegressDropIndex_get(queue, &index[(queue%8)]), ret);
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
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponQueueEegressDropIndex_get(queue, &index[(queue%8)]), ret);
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
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponQueueEegressDropIndex_get(queue, &index[(queue%8)]), ret);
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
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponQueueEegressDropIndex_get(queue, &index[(queue%8)]), ret);
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
            break;
#endif
#if defined(CONFIG_SDK_RTL9601B)
            case RTL9601B_CHIP_ID:
            {
                diag_util_printf("\nPON Flow Control On-Off Threshold\n");
                diag_util_printf("                        On Threshold     Off-Threshold\n");
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_ponGlobalHighThreshold_get(&onThresh, &offThresh), ret);
                diag_util_printf("Global High                  %-4u              %-4u\n",onThresh,offThresh);
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_ponGlobalLowThreshold_get(&onThresh, &offThresh), ret);
                diag_util_printf("Global Low                   %-4u              %-4u\n",onThresh,offThresh);
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_ponPortHighThreshold_get(&onThresh, &offThresh), ret);
                diag_util_printf("Port High                    %-4u              %-4u\n",onThresh,offThresh);
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_ponPortLowThreshold_get(&onThresh, &offThresh), ret);
                diag_util_printf("Port Low                     %-4u              %-4u\n",onThresh,offThresh);

                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_ponQueueEegressGapThreshold_get(&threshold), ret);
                diag_util_printf("PON MAC Egress Queue Gap Threshold is %u \n", threshold);
                diag_util_printf("\n");
                idxlist.min = 0;
                idxlist.max =  RTL9601B_PON_QUEUE_INDEX_MAX;
                idxlist.mask.bits[0] = 0xFF;
                diag_util_printf("PON  Egress Queue Drop Index Threshold  \n");
                diag_util_printf("Index           Drop-Threshold \n");
                diag_util_printf("---------------------------------\n");
                DIAG_UTIL_MASK_SCAN(idxlist, idx)
                {
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_ponQueueEegressDropThreshold_get(idx, &threshold), ret);
                    diag_util_printf("%-18u  %u\n", idx, threshold);
                }
            }
            break;
#endif
#if defined(CONFIG_SDK_RTL9602C)
			case RTL9602C_CHIP_ID:
			{
				diag_util_printf("\nPON Flow Control On-Off Threshold\n");
				diag_util_printf("						  On Threshold	   Off-Threshold\n");
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_ponGlobalHighThreshold_get(&onThresh, &offThresh), ret);
				diag_util_printf("Global High				   %-4u 			 %-4u\n",onThresh,offThresh);
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_ponGlobalLowThreshold_get(&onThresh, &offThresh), ret);
				diag_util_printf("Global Low				   %-4u 			 %-4u\n",onThresh,offThresh);
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_ponPortHighThreshold_get(&onThresh, &offThresh), ret);
				diag_util_printf("Port High 				   %-4u 			 %-4u\n",onThresh,offThresh);
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_ponPortLowThreshold_get(&onThresh, &offThresh), ret);
				diag_util_printf("Port Low					   %-4u 			 %-4u\n",onThresh,offThresh);
			}
			break;
#endif
#if defined(CONFIG_SDK_RTL9607B)
            case RTL9607B_CHIP_ID:
            {
                diag_util_printf("\nPON Flow Control On-Off Threshold\n");
                diag_util_printf("                        On Threshold     Off-Threshold\n");
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_ponGlobalHighThreshold_get(&onThresh, &offThresh), ret);
                diag_util_printf("Global High                  %-4u              %-4u\n",onThresh,offThresh);
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_ponGlobalLowThreshold_get(&onThresh, &offThresh), ret);
                diag_util_printf("Global Low                   %-4u              %-4u\n",onThresh,offThresh);
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_ponPortHighThreshold_get(&onThresh, &offThresh), ret);
                diag_util_printf("Port High                    %-4u              %-4u\n",onThresh,offThresh);
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_ponPortLowThreshold_get(&onThresh, &offThresh), ret);
                diag_util_printf("Port Low                     %-4u              %-4u\n",onThresh,offThresh);
            }
            break;
#endif

            default:
                diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
                return CPARSER_NOT_OK;
                break;
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
        switch(DIAG_UTIL_CHIP_TYPE)
        {
#if defined(CONFIG_SDK_APOLLO)
            case APOLLO_CHIP_ID:
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

                        diag_util_printf("Maximum : %-5u  ", maximum);

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
                break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
            case APOLLOMP_CHIP_ID:
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_totalUsedPageCnt_get(&current, &maximum), ret);
                diag_util_printf("Total page Count: Current  %u maximum  %u\n", current, maximum);
                diag_util_printf("\n");
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_publicUsedPageCnt_get(&current, &maximum), ret);
                diag_util_printf("Public page Count: Current  %u maximum  %u\n", current, maximum);
                diag_util_printf("\n");
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_publicFcoffUsedPageCnt_get(&current, &maximum), ret);
                diag_util_printf("Public page Count for FC-off: Current  %u maximum  %u\n", current, maximum);
                diag_util_printf("\n");
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_publicJumboUsedPageCnt_get(&current, &maximum), ret);
                diag_util_printf("Public page Count for jumbo frame: Current  %u maximum  %u\n", current, maximum);

                diag_util_printf("ingress-port     current-page      maximum-page\n");
                diag_util_printf("-----------------------------------------------------\n");
                for(port=0;port<APOLLO_PORTNO; port++)
                {
                    if(!HAL_IS_PORT_EXIST(port))
                        continue;
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portUsedPageCnt_get(port, &current, &maximum), ret);
                    diag_util_printf("    %-18u %-18u%u\n", port, current, maximum);
                }

                diag_util_printf("egress-port     current-page      maximum-page\n");
                diag_util_printf("-----------------------------------------------------\n");
                for(port=0;port<APOLLO_PORTNO; port++)
                {
                    if(!HAL_IS_PORT_EXIST(port))
                        continue;
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portEgressUsedPageCnt_get(port, &current, &maximum), ret);
                    diag_util_printf("    %-18u %-18u%u\n", port, current, maximum);
                }

                diag_util_printf("Egress Page Count \n");
                for(port=0;port<APOLLO_PORTNO; port++)
                {
                    if(!HAL_IS_PORT_EXIST(port))
                        continue;
                    if (HAL_GET_PON_PORT() != port)
                    {
                        if (port>HAL_GET_PON_PORT() )
                            tmp_port = port-1;
                        else
                            tmp_port = port;

                        diag_util_printf("          P%u     Q0     Q1     Q2     Q3     Q4     Q5     Q6     Q7  \n", port);
                        DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portEgressUsedPageCnt_get(tmp_port, &current, &maximum), ret);

                        for(queue=0;queue<APOLLO_QUEUENO; queue++)
                        {
                                DIAG_UTIL_ERR_CHK( apollomp_raw_flowctrl_queueEgressUsedPageCnt_get(tmp_port, queue, &qcurrent[queue], &qmaximum[queue]), ret);
                        }
                        diag_util_printf("Current : %-5u  ", current);

                        for(queue=0;queue<APOLLO_QUEUENO; queue++)
                        {
        			 if (queue == 7)
        			 	diag_util_printf("%-5u\n", qcurrent[queue]);
        			 else
                                 diag_util_printf("%-5u  ", qcurrent[queue]);
                        }

                        diag_util_printf("Maximum : %-5u  ", maximum);

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
                break;
#endif
#if defined(CONFIG_SDK_RTL9601B)
            case RTL9601B_CHIP_ID:
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_totalUsedPageCnt_get(&current, &maximum), ret);
                diag_util_printf("Total page Count: Current  %u maximum  %u\n", current, maximum);
                diag_util_printf("\n");
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_publicUsedPageCnt_get(&current, &maximum), ret);
                diag_util_printf("Public page Count: Current  %u maximum  %u\n", current, maximum);
                diag_util_printf("\n");
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_publicFcoffUsedPageCnt_get(&current, &maximum), ret);
                diag_util_printf("Public page Count for FC-off: Current  %u maximum  %u\n", current, maximum);
                diag_util_printf("\n");

                diag_util_printf("ingress-port     current-page      maximum-page\n");
                diag_util_printf("-----------------------------------------------------\n");
                for(port=0;port<=2; port++)
                {
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_portUsedPageCnt_get(port, &current, &maximum), ret);
                    diag_util_printf("    %-18u %-18u%u\n", port, current, maximum);
                }

                diag_util_printf("egress-port     current-page      maximum-page\n");
                diag_util_printf("-----------------------------------------------------\n");
                for(port=0;port<=2; port++)
                {
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_portEgressUsedPageCnt_get(port, &current, &maximum), ret);
                    diag_util_printf("    %-18u %-18u%u\n", port, current, maximum);
                }

                diag_util_printf("Egress Page Count \n");
                for(port=0;port<=2; port++)
                {
                    diag_util_printf("          P%u     Q0     Q1     Q2     Q3     Q4     Q5     Q6     Q7  \n", port);
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_portEgressUsedPageCnt_get(port, &current, &maximum), ret);

                    for(queue=0;queue<APOLLO_QUEUENO; queue++)
                    {
                            DIAG_UTIL_ERR_CHK( rtl9601b_raw_flowctrl_queueEgressUsedPageCnt_get(port, queue, &qcurrent[queue], &qmaximum[queue]), ret);
                    }
                    diag_util_printf("Current : %-5u  ", current);

                    for(queue=0;queue<APOLLO_QUEUENO; queue++)
                    {
            			 if (queue == 7)
            			 	diag_util_printf("%-5u\n", qcurrent[queue]);
            			 else
                             diag_util_printf("%-5u  ", qcurrent[queue]);
                    }

                    diag_util_printf("Maximum : %-5u  ", maximum);

                    for(queue=0;queue<APOLLO_QUEUENO; queue++)
                    {
            			 if (queue == 7)
            			    diag_util_printf("%-5u\n", qmaximum[queue]);
            			 else
                            diag_util_printf("%-5u  ", qmaximum[queue]);
                    }
                    diag_util_printf("-------------------------------------------------------------------\n");

                }
                break;
#endif
#if defined(CONFIG_SDK_RTL9602C)
			case RTL9602C_CHIP_ID:
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_totalUsedPageCnt_get(&current, &maximum), ret);
				diag_util_printf("Total page Count: Current  %u maximum  %u\n", current, maximum);
				diag_util_printf("\n");
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_publicUsedPageCnt_get(&current, &maximum), ret);
				diag_util_printf("Public page Count: Current  %u maximum  %u\n", current, maximum);
				diag_util_printf("\n");
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_publicFcoffUsedPageCnt_get(&current, &maximum), ret);
				diag_util_printf("Public page Count for FC-off: Current  %u maximum  %u\n", current, maximum);
				diag_util_printf("\n");

				diag_util_printf("ingress-port     current-page      maximum-page\n");
				diag_util_printf("-----------------------------------------------------\n");
				portlist.min =0;
				portlist.max = 3;
				portlist.portmask.bits[0] = 0xF;
				DIAG_UTIL_PORTMASK_SCAN(portlist, port)
				{
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_portUsedPageCnt_get(port, &current, &maximum), ret);
					diag_util_printf("    %-18u %-18u%u\n", port, current, maximum);
				}

				diag_util_printf("egress-port     current-page      maximum-page\n");
				diag_util_printf("-----------------------------------------------------\n");
				DIAG_UTIL_PORTMASK_SCAN(portlist, port)
				{
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_portEgressUsedPageCnt_get(port, &current, &maximum), ret);
					diag_util_printf("    %-18u %-18u%u\n", port, current, maximum);
				}
				queuelist.min = 0;
				queuelist.max =  7;
				queuelist.mask.bits[0] = 0xFF;

				diag_util_printf("Egress Page Count \n");
				DIAG_UTIL_PORTMASK_SCAN(portlist, port)
				{
					diag_util_printf("          P%u     Q0     Q1     Q2     Q3     Q4     Q5     Q6     Q7  \n", port);
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_portEgressUsedPageCnt_get(port, &current, &maximum), ret);

					DIAG_UTIL_MASK_SCAN(queuelist, queue)
					{
							DIAG_UTIL_ERR_CHK( rtl9602c_raw_flowctrl_queueEgressUsedPageCnt_get(port, queue, &qcurrent[queue], &qmaximum[queue]), ret);
					}
					diag_util_printf("Current : %-5u  ", current);

					DIAG_UTIL_MASK_SCAN(queuelist, queue)
					{
						 if (queue == 7)
							diag_util_printf("%-5u\n", qcurrent[queue]);
						 else
							 diag_util_printf("%-5u  ", qcurrent[queue]);
					}

					diag_util_printf("Maximum : %-5u  ", maximum);

					DIAG_UTIL_MASK_SCAN(queuelist, queue)
					{
						 if (queue == 7)
							diag_util_printf("%-5u\n", qmaximum[queue]);
						 else
							diag_util_printf("%-5u  ", qmaximum[queue]);
					}
					diag_util_printf("-------------------------------------------------------------------\n");

				}
				break;
#endif
#if defined(CONFIG_SDK_RTL9607B)
                case RTL9607B_CHIP_ID:
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_totalUsedPageCnt_get(&current, &maximum), ret);
                    diag_util_printf("Total page Count: Current  %u maximum  %u\n", current, maximum);
                    diag_util_printf("\n");
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_publicUsedPageCnt_get(&current, &maximum), ret);
                    diag_util_printf("Public page Count: Current  %u maximum  %u\n", current, maximum);
                    diag_util_printf("\n");
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_publicFcoffUsedPageCnt_get(&current, &maximum), ret);
                    diag_util_printf("Public page Count for FC-off: Current  %u maximum  %u\n", current, maximum);
                    diag_util_printf("\n");

                    diag_util_printf("ingress-port     current-page      maximum-page\n");
                    diag_util_printf("-----------------------------------------------------\n");

                    portlist.min = HAL_GET_MIN_PORT();
                    portlist.max = HAL_GET_MAX_PORT();
                    HAL_GET_ALL_PORTMASK(portlist.portmask);

                    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
                    {
                        DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_portUsedPageCnt_get(port, &current, &maximum), ret);
                        diag_util_printf("    %-18u %-18u%u\n", port, current, maximum);
                    }

                    diag_util_printf("egress-port     current-page      maximum-page\n");
                    diag_util_printf("-----------------------------------------------------\n");
                    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
                    {
                        DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_portEgressUsedPageCnt_get(port, &current, &maximum), ret);
                        diag_util_printf("    %-18u %-18u%u\n", port, current, maximum);
                    }
                    queuelist.min = HAL_MIN_NUM_OF_QUEUE() - 1;
                    queuelist.max =  HAL_MAX_NUM_OF_QUEUE() - 1;
                    queuelist.mask.bits[0] = 0xFF;
                    //DIAG_UTIL_EXTRACT_QUEUEMASK(queuelist, 7);

                    diag_util_printf("Egress Page Count \n");
                    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
                    {
                        diag_util_printf("          P%u     Q0     Q1     Q2     Q3     Q4     Q5     Q6     Q7  \n", port);
                        DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_portEgressUsedPageCnt_get(port, &current, &maximum), ret);

                        DIAG_UTIL_MASK_SCAN(queuelist, queue)
                        {
                            DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_queueEgressUsedPageCnt_get(port, queue, &qcurrent[queue], &qmaximum[queue]), ret);
                        }
                        diag_util_printf("Current : %-5u  ", current);

                        DIAG_UTIL_MASK_SCAN(queuelist, queue)
                        {
                             if (queue == 7)
                                diag_util_printf("%-5u\n", qcurrent[queue]);
                             else
                                 diag_util_printf("%-5u  ", qcurrent[queue]);
                        }

                        diag_util_printf("Maximum : %-5u  ", maximum);

                        DIAG_UTIL_MASK_SCAN(queuelist, queue)
                        {
                             if (queue == 7)
                                diag_util_printf("%-5u\n", qmaximum[queue]);
                             else
                                diag_util_printf("%-5u  ", qmaximum[queue]);
                        }
                        diag_util_printf("-------------------------------------------------------------------\n");

                    }
                    break;
#endif


            default:
                diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
                return CPARSER_NOT_OK;
                break;
        }
    }
    else if('p'==TOKEN_CHAR(3,0))
    {
        switch(DIAG_UTIL_CHIP_TYPE)
        {
#if defined(CONFIG_SDK_APOLLO)
            case APOLLO_CHIP_ID:
            {
                diag_util_printf("Get PON  Egress Queue Used Page Count\n");
                diag_util_printf("-------------------------------------------------------------\n");

                for (idx=0;idx<16;idx++)
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
            break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
            case APOLLOMP_CHIP_ID:
            {
                diag_util_printf("Get PON  Egress Queue Used Page Count\n");
                diag_util_printf("-------------------------------------------------------------\n");

                for (idx=0;idx<16;idx++)
                {
          	      diag_util_printf("QID:          ");
                      for(queue=0;queue<8; queue++)
                      {
                          tmp_queue = (idx<<3) | queue;
                          DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponUsedPage_get(tmp_queue, &qcurrent[(tmp_queue%8)], &qmaximum[(tmp_queue%8)]), ret);
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
            break;
#endif
#if defined(CONFIG_SDK_RTL9601B)
            case RTL9601B_CHIP_ID:
            {
                diag_util_printf("Get PON  Egress SID Used Page Count\n");
                diag_util_printf("-------------------------------------------------------------\n");

                for (idx=0;idx<5;idx++)
                {
                    diag_util_printf("SID:          ");
                    for(queue=0;queue<8; queue++)
                    {
                        if(idx==4&&queue>0)
                            break;

                        tmp_queue = (idx<<3) | queue;
                        DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_ponUsedPage_get(tmp_queue, &qcurrent[queue], &qmaximum[queue]), ret);
                        if ((queue == 7) || (idx==4 && queue==0))
                            diag_util_printf("%-5u\n", tmp_queue);
                        else
                            diag_util_printf("%-5u   ", tmp_queue);
                    }
                    diag_util_printf("Current :     ");
                    for(queue=0;queue<APOLLO_QUEUENO; queue++)
                    {
                        if(idx==4&&queue>0)
                            break;

                        if ((queue == 7) || (idx==4 && queue==0))
                            diag_util_printf("%-5u\n", qcurrent[queue]);
                        else
                            diag_util_printf("%-5u   ", qcurrent[queue]);
                    }
                    diag_util_printf("Maximum :     ");
                    for(queue=0;queue<APOLLO_QUEUENO; queue++)
                    {
                        if(idx==4&&queue>0)
                            break;

                        if ((queue == 7) || (idx==4 && queue==0))
                            diag_util_printf("%-5u\n", qmaximum[queue]);
                        else
                            diag_util_printf("%-5u   ", qmaximum[queue]);
                    }
                    diag_util_printf("-------------------------------------------------------------------\n");

                }
            }
            break;
#endif
#if defined(CONFIG_SDK_RTL9602C)
			case RTL9602C_CHIP_ID:
			{
				diag_util_printf("Get PON  Egress SID Used Page Count\n");
				diag_util_printf("-------------------------------------------------------------\n");

				for (idx=0;idx<9;idx++)
				{
					diag_util_printf("SID:          ");
					for(queue=0;queue<8; queue++)
					{
                        if(idx==8&&queue>0)
                            break;

						tmp_queue = (idx<<3) | queue;
						DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_ponUsedPage_get(tmp_queue, &qcurrent[queue], &qmaximum[queue]), ret);
						if ((queue == 7) || (idx==8 && queue==0))
							diag_util_printf("%-5u\n", tmp_queue);
						else
							diag_util_printf("%-5u   ", tmp_queue);
					}
					diag_util_printf("Current :     ");
					for(queue=0;queue<8; queue++)
					{
                        if(idx==8&&queue>0)
                            break;

						if ((queue == 7) || (idx==8 && queue==0))
							diag_util_printf("%-5u\n", qcurrent[queue]);
						else
							diag_util_printf("%-5u   ", qcurrent[queue]);
					}
					diag_util_printf("Maximum :     ");
					for(queue=0;queue<8; queue++)
					{
                        if(idx==8&&queue>0)
                            break;

						if ((queue == 7) || (idx==8 && queue==0))
							diag_util_printf("%-5u\n", qmaximum[queue]);
						else
							diag_util_printf("%-5u   ", qmaximum[queue]);
					}
					diag_util_printf("-------------------------------------------------------------------\n");

				}
			}
			break;
#endif
#if defined(CONFIG_SDK_RTL9607B)
            case RTL9607B_CHIP_ID:
            {
                diag_util_printf("Get PON  Egress SID Used Page Count\n");
                diag_util_printf("-------------------------------------------------------------\n");
                rtk_qid_t idx_end = (HAL_CLASSIFY_SID_NUM() / 8) + ((0 == (HAL_CLASSIFY_SID_NUM() % 8)) ? 0 : 1);

                for (idx = 0; idx < idx_end + 1; idx++)
                {
                    diag_util_printf("SID:          ");
                    for(queue=0;queue<8; queue++)
                    {
                        if(idx == idx_end &&queue>0)
                            break;

                        tmp_queue = (idx<<3) | queue;
                        DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_ponUsedPage_get(tmp_queue, &qcurrent[queue], &qmaximum[queue]), ret);
                        if ((queue == 7) || (idx == idx_end && queue==0))
                            diag_util_printf("%-5u\n", tmp_queue);
                        else
                            diag_util_printf("%-5u   ", tmp_queue);
                    }
                    diag_util_printf("Current :     ");
                    for(queue=0;queue<8; queue++)
                    {
                        if(idx == idx_end && queue>0)
                            break;

                        if ((queue == 7) || (idx == idx_end && queue==0))
                            diag_util_printf("%-5u\n", qcurrent[queue]);
                        else
                            diag_util_printf("%-5u   ", qcurrent[queue]);
                    }
                    diag_util_printf("Maximum :     ");
                    for(queue=0;queue<8; queue++)
                    {
                        if(idx== idx_end && queue > 0)
                            break;

                        if ((queue == 7) || (idx== idx_end && queue==0))
                            diag_util_printf("%-5u\n", qmaximum[queue]);
                        else
                            diag_util_printf("%-5u   ", qmaximum[queue]);
                    }
                    diag_util_printf("-------------------------------------------------------------------\n");

                }
            }
            break;
#endif

            default:
                diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
                return CPARSER_NOT_OK;
                break;
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
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
            apollo_raw_flowctrl_type_t type;
            if('i'==TOKEN_CHAR(3,0))
                type = RAW_FLOWCTRL_TYPE_INGRESS;
            else if('e'==TOKEN_CHAR(3,0))
                type = RAW_FLOWCTRL_TYPE_EGRESS;
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_type_set(type), ret);
            diag_util_printf("Set Flow Control Type to  %s\n",diagStr_flowCtrlType[type]);
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            apollomp_raw_flowctrl_type_t type;
            if('i'==TOKEN_CHAR(3,0))
                type = RAW_FLOWCTRL_TYPE_INGRESS;
            else if('e'==TOKEN_CHAR(3,0))
                type = RAW_FLOWCTRL_TYPE_EGRESS;
            DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_type_set(type), ret);
            diag_util_printf("Set Flow Control Type to  %s\n",diagStr_flowCtrlType[type]);
        }
        break;
#endif
        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_type_ingress_egress */

/*
 * flowctrl get type
 */
cparser_result_t
cparser_cmd_flowctrl_get_type(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
            apollo_raw_flowctrl_type_t type;
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_type_get(&type), ret);
            diag_util_mprintf("Flow Control Type is  %s\n",diagStr_flowCtrlType[type]);
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            apollomp_raw_flowctrl_type_t type;
            DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_type_get(&type), ret);
            diag_util_mprintf("Flow Control Type is  %s\n",diagStr_flowCtrlType[type]);
        }
        break;
#endif
        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_type */

/*
 * flowctrl set drop-all threshold <UINT:threshold>
 */
cparser_result_t
cparser_cmd_flowctrl_set_drop_all_threshold_threshold(
    cparser_context_t *context,
    uint32_t  *threshold_ptr)
{
    int32 ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_dropAllThreshold_set(*threshold_ptr), ret);
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_dropAllThreshold_set(*threshold_ptr), ret);
        }
        break;
#endif

#if defined(CONFIG_SDK_RTL9601B)
        case RTL9601B_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_dropAllThreshold_set(*threshold_ptr), ret);
        }
        break;
#endif
#if defined(CONFIG_SDK_RTL9602C)
		case RTL9602C_CHIP_ID:
		{
			DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_dropAllThreshold_set(*threshold_ptr), ret);
		}
		break;
#endif
#if defined(CONFIG_SDK_RTL9607B)
        case RTL9607B_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_dropAllThreshold_set(*threshold_ptr), ret);
        }
        break;
#endif

        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }
    diag_util_mprintf("Set Drop All Threshold to %u\n", *threshold_ptr);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_drop_all_threshold_threshold */

/*
 * flowctrl get drop-all threshold
 */
cparser_result_t
cparser_cmd_flowctrl_get_drop_all_threshold(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    uint32 threshold;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_dropAllThreshold_get(&threshold), ret);
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_dropAllThreshold_get(&threshold), ret);
        }
        break;
#endif
#if defined(CONFIG_SDK_RTL9601B)
        case RTL9601B_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_dropAllThreshold_get(&threshold), ret);
        }
        break;
#endif
#if defined(CONFIG_SDK_RTL9602C)
		case RTL9602C_CHIP_ID:
		{
			DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_dropAllThreshold_get(&threshold), ret);
		}
		break;
#endif
#if defined(CONFIG_SDK_RTL9607B)
        case RTL9607B_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_dropAllThreshold_get(&threshold), ret);
        }
        break;
#endif

        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }
    diag_util_mprintf("Drop All Threshold is %u\n", threshold);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_drop_all_threshold */

/*
 * flowctrl set pause-all threshold <UINT:threshold>
 */
cparser_result_t
cparser_cmd_flowctrl_set_pause_all_threshold_threshold(
    cparser_context_t *context,
    uint32_t  *threshold_ptr)
{
    int32 ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_pauseAllThreshold_set(*threshold_ptr), ret);
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_pauseAllThreshold_set(*threshold_ptr), ret);
        }
        break;
#endif
#if defined(CONFIG_SDK_RTL9601B)
        case RTL9601B_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_pauseAllThreshold_set(*threshold_ptr), ret);
        }
        break;
#endif
#if defined(CONFIG_SDK_RTL9602C)
		case RTL9602C_CHIP_ID:
		{
			DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_pauseAllThreshold_set(*threshold_ptr), ret);
		}
		break;
#endif
#if defined(CONFIG_SDK_RTL9607B)
        case RTL9607B_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_pauseAllThreshold_set(*threshold_ptr), ret);
        }
        break;
#endif

        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }

    diag_util_mprintf("Set Pause All Threshold to %u\n", *threshold_ptr);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_pause_all_threshold_threshold */

/*
 * flowctrl get pause-all threshold
 */
cparser_result_t
cparser_cmd_flowctrl_get_pause_all_threshold(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    uint32 threshold;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_pauseAllThreshold_get(&threshold), ret);
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_pauseAllThreshold_get(&threshold), ret);
        }
        break;
#endif
#if defined(CONFIG_SDK_RTL9601B)
        case RTL9601B_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_pauseAllThreshold_get(&threshold), ret);
        }
        break;
#endif
#if defined(CONFIG_SDK_RTL9602C)
		case RTL9602C_CHIP_ID:
		{
			DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_pauseAllThreshold_get(&threshold), ret);
		}
		break;
#endif
#if defined(CONFIG_SDK_RTL9607B)
        case RTL9607B_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_pauseAllThreshold_get(&threshold), ret);
        }
        break;
#endif

        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }

    diag_util_mprintf("Pause All Threshold is %u\n", threshold);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_pause_all_threshold */

/*
 * flowctrl set ingress system ( flowctrl-threshold | drop-threshold ) ( high-off | high-on | low-off | low-on ) threshold <UINT:threshold>
 */
cparser_result_t
cparser_cmd_flowctrl_set_ingress_system_flowctrl_threshold_drop_threshold_high_off_high_on_low_off_low_on_threshold_threshold(
    cparser_context_t *context,
    uint32_t  *threshold_ptr)
{
    uint32 onThresh, offThresh;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
            if('d'==TOKEN_CHAR(4,0))
            {
                if(!osal_strcmp(TOKEN_STR(5),"high-off"))
                {
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffHighThreshold_set(onThresh, *threshold_ptr), ret);
                    diag_util_printf("Set Flow Control Global FC-Off High Off Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
                {
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffHighThreshold_set(*threshold_ptr, offThresh), ret);
                    diag_util_printf("Set Flow Control Global FC-Off High On Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
                {
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffLowThreshold_set(onThresh, *threshold_ptr), ret);
                    diag_util_printf("Set Flow Control Global FC-Off Low Off Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
                {
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffLowThreshold_set(*threshold_ptr, offThresh), ret);
                    diag_util_printf("Set Flow Control Global FC-Off Low On Threshold to %u\n",*threshold_ptr);
                }
            }
            else if('f'==TOKEN_CHAR(4,0))
            {
                if(!osal_strcmp(TOKEN_STR(5),"high-off"))
                {
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconHighThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconHighThreshold_set(onThresh, *threshold_ptr), ret);
                    diag_util_printf("Set Flow Control Global FC-On High Off Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
                {
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconHighThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconHighThreshold_set(*threshold_ptr, offThresh), ret);
                    diag_util_printf("Set Flow Control Global FC-On High On Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
                {
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconLowThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconLowThreshold_set(onThresh, *threshold_ptr), ret);
                    diag_util_printf("Set Flow Control Global FC-On Low Off Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
                {
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconLowThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconLowThreshold_set(*threshold_ptr, offThresh), ret);
                    diag_util_printf("Set Flow Control Global FC-On Low On Threshold to %u\n",*threshold_ptr);
                }
            }
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            if('d'==TOKEN_CHAR(4,0))
            {
                if(!osal_strcmp(TOKEN_STR(5),"high-off"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFcoffHighThreshold_set(onThresh, *threshold_ptr), ret);
                    diag_util_printf("Set Flow Control Global FC-Off High Off Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFcoffHighThreshold_set(*threshold_ptr, offThresh), ret);
                    diag_util_printf("Set Flow Control Global FC-Off High On Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFcoffLowThreshold_set(onThresh, *threshold_ptr), ret);
                    diag_util_printf("Set Flow Control Global FC-Off Low Off Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFcoffLowThreshold_set(*threshold_ptr, offThresh), ret);
                    diag_util_printf("Set Flow Control Global FC-Off Low On Threshold to %u\n",*threshold_ptr);
                }
            }
            else if('f'==TOKEN_CHAR(4,0))
            {
                if(!osal_strcmp(TOKEN_STR(5),"high-off"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFconHighThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFconHighThreshold_set(onThresh, *threshold_ptr), ret);
                    diag_util_printf("Set Flow Control Global FC-On High Off Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFconHighThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFconHighThreshold_set(*threshold_ptr, offThresh), ret);
                    diag_util_printf("Set Flow Control Global FC-On High On Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFconLowThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFconLowThreshold_set(onThresh, *threshold_ptr), ret);
                    diag_util_printf("Set Flow Control Global FC-On Low Off Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFconLowThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFconLowThreshold_set(*threshold_ptr, offThresh), ret);
                    diag_util_printf("Set Flow Control Global FC-On Low On Threshold to %u\n",*threshold_ptr);
                }
            }

        }
        break;
#endif
#if defined(CONFIG_SDK_RTL9601B)
        case RTL9601B_CHIP_ID:
        {
            if('d'==TOKEN_CHAR(4,0))
            {
                if(!osal_strcmp(TOKEN_STR(5),"high-off"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_globalFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_globalFcoffHighThreshold_set(onThresh, *threshold_ptr), ret);
                    diag_util_printf("Set Flow Control Global FC-Off High Off Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_globalFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_globalFcoffHighThreshold_set(*threshold_ptr, offThresh), ret);
                    diag_util_printf("Set Flow Control Global FC-Off High On Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_globalFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_globalFcoffLowThreshold_set(onThresh, *threshold_ptr), ret);
                    diag_util_printf("Set Flow Control Global FC-Off Low Off Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_globalFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_globalFcoffLowThreshold_set(*threshold_ptr, offThresh), ret);
                    diag_util_printf("Set Flow Control Global FC-Off Low On Threshold to %u\n",*threshold_ptr);
                }
            }
            else if('f'==TOKEN_CHAR(4,0))
            {
                if(!osal_strcmp(TOKEN_STR(5),"high-off"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_globalFconHighThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_globalFconHighThreshold_set(onThresh, *threshold_ptr), ret);
                    diag_util_printf("Set Flow Control Global FC-On High Off Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_globalFconHighThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_globalFconHighThreshold_set(*threshold_ptr, offThresh), ret);
                    diag_util_printf("Set Flow Control Global FC-On High On Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_globalFconLowThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_globalFconLowThreshold_set(onThresh, *threshold_ptr), ret);
                    diag_util_printf("Set Flow Control Global FC-On Low Off Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_globalFconLowThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_globalFconLowThreshold_set(*threshold_ptr, offThresh), ret);
                    diag_util_printf("Set Flow Control Global FC-On Low On Threshold to %u\n",*threshold_ptr);
                }
            }

        }
        break;
#endif
#if defined(CONFIG_SDK_RTL9602C)
		case RTL9602C_CHIP_ID:
		{
			if('d'==TOKEN_CHAR(4,0))
			{
				if(!osal_strcmp(TOKEN_STR(5),"high-off"))
				{
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_globalFcoffHighThreshold_get(&onThresh, &offThresh), ret);
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_globalFcoffHighThreshold_set(onThresh, *threshold_ptr), ret);
					diag_util_printf("Set Flow Control Global FC-Off High Off Threshold to %u\n",*threshold_ptr);
				}
				else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
				{
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_globalFcoffHighThreshold_get(&onThresh, &offThresh), ret);
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_globalFcoffHighThreshold_set(*threshold_ptr, offThresh), ret);
					diag_util_printf("Set Flow Control Global FC-Off High On Threshold to %u\n",*threshold_ptr);
				}
				else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
				{
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_globalFcoffLowThreshold_get(&onThresh, &offThresh), ret);
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_globalFcoffLowThreshold_set(onThresh, *threshold_ptr), ret);
					diag_util_printf("Set Flow Control Global FC-Off Low Off Threshold to %u\n",*threshold_ptr);
				}
				else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
				{
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_globalFcoffLowThreshold_get(&onThresh, &offThresh), ret);
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_globalFcoffLowThreshold_set(*threshold_ptr, offThresh), ret);
					diag_util_printf("Set Flow Control Global FC-Off Low On Threshold to %u\n",*threshold_ptr);
				}
			}
			else if('f'==TOKEN_CHAR(4,0))
			{
				if(!osal_strcmp(TOKEN_STR(5),"high-off"))
				{
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_globalFconHighThreshold_get(&onThresh, &offThresh), ret);
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_globalFconHighThreshold_set(onThresh, *threshold_ptr), ret);
					diag_util_printf("Set Flow Control Global FC-On High Off Threshold to %u\n",*threshold_ptr);
				}
				else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
				{
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_globalFconHighThreshold_get(&onThresh, &offThresh), ret);
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_globalFconHighThreshold_set(*threshold_ptr, offThresh), ret);
					diag_util_printf("Set Flow Control Global FC-On High On Threshold to %u\n",*threshold_ptr);
				}
				else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
				{
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_globalFconLowThreshold_get(&onThresh, &offThresh), ret);
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_globalFconLowThreshold_set(onThresh, *threshold_ptr), ret);
					diag_util_printf("Set Flow Control Global FC-On Low Off Threshold to %u\n",*threshold_ptr);
				}
				else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
				{
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_globalFconLowThreshold_get(&onThresh, &offThresh), ret);
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_globalFconLowThreshold_set(*threshold_ptr, offThresh), ret);
					diag_util_printf("Set Flow Control Global FC-On Low On Threshold to %u\n",*threshold_ptr);
				}
			}

		}
		break;
#endif
#if defined(CONFIG_SDK_RTL9607B)
        case RTL9607B_CHIP_ID:
        {
            if('d'==TOKEN_CHAR(4,0))
            {
                if(!osal_strcmp(TOKEN_STR(5),"high-off"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_globalFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_globalFcoffHighThreshold_set(onThresh, *threshold_ptr), ret);
                    diag_util_printf("Set Flow Control Global FC-Off High Off Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_globalFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_globalFcoffHighThreshold_set(*threshold_ptr, offThresh), ret);
                    diag_util_printf("Set Flow Control Global FC-Off High On Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_globalFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_globalFcoffLowThreshold_set(onThresh, *threshold_ptr), ret);
                    diag_util_printf("Set Flow Control Global FC-Off Low Off Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_globalFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_globalFcoffLowThreshold_set(*threshold_ptr, offThresh), ret);
                    diag_util_printf("Set Flow Control Global FC-Off Low On Threshold to %u\n",*threshold_ptr);
                }
            }
            else if('f'==TOKEN_CHAR(4,0))
            {
                if(!osal_strcmp(TOKEN_STR(5),"high-off"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_globalFconHighThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_globalFconHighThreshold_set(onThresh, *threshold_ptr), ret);
                    diag_util_printf("Set Flow Control Global FC-On High Off Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_globalFconHighThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_globalFconHighThreshold_set(*threshold_ptr, offThresh), ret);
                    diag_util_printf("Set Flow Control Global FC-On High On Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_globalFconLowThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_globalFconLowThreshold_set(onThresh, *threshold_ptr), ret);
                    diag_util_printf("Set Flow Control Global FC-On Low Off Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_globalFconLowThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_globalFconLowThreshold_set(*threshold_ptr, offThresh), ret);
                    diag_util_printf("Set Flow Control Global FC-On Low On Threshold to %u\n",*threshold_ptr);
                }
            }

        }
        break;
#endif

        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_ingress_system_flowctrl_threshold_drop_threshold_high_off_high_on_low_off_low_on_threshold_threshold */

/*
 * flowctrl get ingress system (  flowctrl-threshold | drop-threshold ) ( high-off | high-on | low-off | low-on ) threshold
 */
cparser_result_t
cparser_cmd_flowctrl_get_ingress_system_flowctrl_threshold_drop_threshold_high_off_high_on_low_off_low_on_threshold(
    cparser_context_t *context)
{
    uint32 onThresh, offThresh;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
            if('d'==TOKEN_CHAR(4,0))
            {
                if(!osal_strcmp(TOKEN_STR(5),"high-off"))
                {
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_printf("Flow Control Global FC-Off High Off Threshold is %u\n",offThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
                {
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_printf("Flow Control Global FC-Off High On Threshold is %u\n",onThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
                {
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_printf("Flow Control Global FC-Off Low Off Threshold is %u\n",offThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
                {
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_printf("Flow Control Global FC-Off Low On Threshold is %u\n",onThresh);
                }
            }
            else if('f'==TOKEN_CHAR(4,0))
            {
                if(!osal_strcmp(TOKEN_STR(5),"high-off"))
                {
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconHighThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_printf("Flow Control Global FC-On High Off Threshold is %u\n",offThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
                {
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconHighThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_printf("Flow Control Global FC-On High On Threshold is %u\n",onThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
                {
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconLowThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_printf("Flow Control Global FC-On Low Off Threshold is %u\n",offThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
                {
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconLowThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_printf("Flow Control Global FC-On Low On Threshold is %u\n",onThresh);
                }
            }
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            if('d'==TOKEN_CHAR(4,0))
            {
                if(!osal_strcmp(TOKEN_STR(5),"high-off"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_printf("Flow Control Global FC-Off High Off Threshold is %u\n",offThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_printf("Flow Control Global FC-Off High On Threshold is %u\n",onThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_printf("Flow Control Global FC-Off Low Off Threshold is %u\n",offThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_printf("Flow Control Global FC-Off Low On Threshold is %u\n",onThresh);
                }
            }
            else if('f'==TOKEN_CHAR(4,0))
            {
                if(!osal_strcmp(TOKEN_STR(5),"high-off"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFconHighThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_printf("Flow Control Global FC-On High Off Threshold is %u\n",offThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFconHighThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_printf("Flow Control Global FC-On High On Threshold is %u\n",onThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFconLowThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_printf("Flow Control Global FC-On Low Off Threshold is %u\n",offThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFconLowThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_printf("Flow Control Global FC-On Low On Threshold is %u\n",onThresh);
                }
            }

        }
        break;
#endif
#if defined(CONFIG_SDK_RTL9601B)
        case RTL9601B_CHIP_ID:
        {
            if('d'==TOKEN_CHAR(4,0))
            {
                if(!osal_strcmp(TOKEN_STR(5),"high-off"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_globalFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_printf("Flow Control Global FC-Off High Off Threshold is %u\n",offThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_globalFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_printf("Flow Control Global FC-Off High On Threshold is %u\n",onThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_globalFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_printf("Flow Control Global FC-Off Low Off Threshold is %u\n",offThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_globalFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_printf("Flow Control Global FC-Off Low On Threshold is %u\n",onThresh);
                }
            }
            else if('f'==TOKEN_CHAR(4,0))
            {
                if(!osal_strcmp(TOKEN_STR(5),"high-off"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_globalFconHighThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_printf("Flow Control Global FC-On High Off Threshold is %u\n",offThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_globalFconHighThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_printf("Flow Control Global FC-On High On Threshold is %u\n",onThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_globalFconLowThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_printf("Flow Control Global FC-On Low Off Threshold is %u\n",offThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_globalFconLowThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_printf("Flow Control Global FC-On Low On Threshold is %u\n",onThresh);
                }
            }

        }
        break;
#endif
#if defined(CONFIG_SDK_RTL9602C)
		case RTL9602C_CHIP_ID:
		{
			if('d'==TOKEN_CHAR(4,0))
			{
				if(!osal_strcmp(TOKEN_STR(5),"high-off"))
				{
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_globalFcoffHighThreshold_get(&onThresh, &offThresh), ret);
					diag_util_printf("Flow Control Global FC-Off High Off Threshold is %u\n",offThresh);
				}
				else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
				{
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_globalFcoffHighThreshold_get(&onThresh, &offThresh), ret);
					diag_util_printf("Flow Control Global FC-Off High On Threshold is %u\n",onThresh);
				}
				else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
				{
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_globalFcoffLowThreshold_get(&onThresh, &offThresh), ret);
					diag_util_printf("Flow Control Global FC-Off Low Off Threshold is %u\n",offThresh);
				}
				else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
				{
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_globalFcoffLowThreshold_get(&onThresh, &offThresh), ret);
					diag_util_printf("Flow Control Global FC-Off Low On Threshold is %u\n",onThresh);
				}
			}
			else if('f'==TOKEN_CHAR(4,0))
			{
				if(!osal_strcmp(TOKEN_STR(5),"high-off"))
				{
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_globalFconHighThreshold_get(&onThresh, &offThresh), ret);
					diag_util_printf("Flow Control Global FC-On High Off Threshold is %u\n",offThresh);
				}
				else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
				{
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_globalFconHighThreshold_get(&onThresh, &offThresh), ret);
					diag_util_printf("Flow Control Global FC-On High On Threshold is %u\n",onThresh);
				}
				else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
				{
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_globalFconLowThreshold_get(&onThresh, &offThresh), ret);
					diag_util_printf("Flow Control Global FC-On Low Off Threshold is %u\n",offThresh);
				}
				else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
				{
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_globalFconLowThreshold_get(&onThresh, &offThresh), ret);
					diag_util_printf("Flow Control Global FC-On Low On Threshold is %u\n",onThresh);
				}
			}

		}
		break;
#endif
#if defined(CONFIG_SDK_RTL9607B)
        case RTL9607B_CHIP_ID:
        {
            if('d'==TOKEN_CHAR(4,0))
            {
                if(!osal_strcmp(TOKEN_STR(5),"high-off"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_globalFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_printf("Flow Control Global FC-Off High Off Threshold is %u\n",offThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_globalFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_printf("Flow Control Global FC-Off High On Threshold is %u\n",onThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_globalFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_printf("Flow Control Global FC-Off Low Off Threshold is %u\n",offThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_globalFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_printf("Flow Control Global FC-Off Low On Threshold is %u\n",onThresh);
                }
            }
            else if('f'==TOKEN_CHAR(4,0))
            {
                if(!osal_strcmp(TOKEN_STR(5),"high-off"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_globalFconHighThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_printf("Flow Control Global FC-On High Off Threshold is %u\n",offThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_globalFconHighThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_printf("Flow Control Global FC-On High On Threshold is %u\n",onThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_globalFconLowThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_printf("Flow Control Global FC-On Low Off Threshold is %u\n",offThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_globalFconLowThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_printf("Flow Control Global FC-On Low On Threshold is %u\n",onThresh);
                }
            }

        }
        break;
#endif

        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_ingress_system_flowctrl_threshold_drop_threshold_high_off_high_on_low_off_low_on_threshold */

/*
 * flowctrl set ingress port ( flowctrl-threshold | drop-threshold ) ( high-off | high-on | low-off | low-on ) threshold <UINT:threshold>
 */
cparser_result_t
cparser_cmd_flowctrl_set_ingress_port_flowctrl_threshold_drop_threshold_high_off_high_on_low_off_low_on_threshold_threshold(
    cparser_context_t *context,
    uint32_t  *threshold_ptr)
{
    uint32 onThresh, offThresh;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
            if('d'==TOKEN_CHAR(4,0))
            {
                if(!osal_strcmp(TOKEN_STR(5),"high-off"))
                {
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFcoffHighThreshold_set(onThresh, *threshold_ptr), ret);
                    diag_util_printf("Set Flow Control Port FC-Off High Off Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
                {
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFcoffHighThreshold_set(*threshold_ptr, offThresh), ret);
                    diag_util_printf("Set Flow Control Port FC-Off High On Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
                {
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFcoffLowThreshold_set(onThresh, *threshold_ptr), ret);
                    diag_util_printf("Set Flow Control Port FC-Off Low Off Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
                {
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFcoffLowThreshold_set(*threshold_ptr, offThresh), ret);
                    diag_util_printf("Set Flow Control Port FC-Off Low On Threshold to %u\n",*threshold_ptr);
                }
            }
            else if('f'==TOKEN_CHAR(4,0))
            {
                if(!osal_strcmp(TOKEN_STR(5),"high-off"))
                {
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFconHighThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFconHighThreshold_set(onThresh, *threshold_ptr), ret);
                    diag_util_printf("Set Flow Control Port FC-On High Off Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
                {
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFconHighThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFconHighThreshold_set(*threshold_ptr, offThresh), ret);
                    diag_util_printf("Set Flow Control Port FC-On High On Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
                {
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFconLowThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFconLowThreshold_set(onThresh, *threshold_ptr), ret);
                    diag_util_printf("Set Flow Control Port FC-On Low Off Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
                {
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFconLowThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portFconLowThreshold_set(*threshold_ptr, offThresh), ret);
                    diag_util_printf("Set Flow Control Port FC-On Low On Threshold to %u\n",*threshold_ptr);
                }
            }
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            if('d'==TOKEN_CHAR(4,0))
            {
                if(!osal_strcmp(TOKEN_STR(5),"high-off"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portFcoffHighThreshold_set(onThresh, *threshold_ptr), ret);
                    diag_util_printf("Set Flow Control Port FC-Off High Off Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portFcoffHighThreshold_set(*threshold_ptr, offThresh), ret);
                    diag_util_printf("Set Flow Control Port FC-Off High On Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portFcoffLowThreshold_set(onThresh, *threshold_ptr), ret);
                    diag_util_printf("Set Flow Control Port FC-Off Low Off Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portFcoffLowThreshold_set(*threshold_ptr, offThresh), ret);
                    diag_util_printf("Set Flow Control Port FC-Off Low On Threshold to %u\n",*threshold_ptr);
                }
            }
            else if('f'==TOKEN_CHAR(4,0))
            {
                if(!osal_strcmp(TOKEN_STR(5),"high-off"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portFconHighThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portFconHighThreshold_set(onThresh, *threshold_ptr), ret);
                    diag_util_printf("Set Flow Control Port FC-On High Off Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portFconHighThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portFconHighThreshold_set(*threshold_ptr, offThresh), ret);
                    diag_util_printf("Set Flow Control Port FC-On High On Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portFconLowThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portFconLowThreshold_set(onThresh, *threshold_ptr), ret);
                    diag_util_printf("Set Flow Control Port FC-On Low Off Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portFconLowThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portFconLowThreshold_set(*threshold_ptr, offThresh), ret);
                    diag_util_printf("Set Flow Control Port FC-On Low On Threshold to %u\n",*threshold_ptr);
                }
            }
        }
        break;
#endif
#if defined(CONFIG_SDK_RTL9601B)
        case RTL9601B_CHIP_ID:
        {
            if('d'==TOKEN_CHAR(4,0))
            {
                if(!osal_strcmp(TOKEN_STR(5),"high-off"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_portFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_portFcoffHighThreshold_set(onThresh, *threshold_ptr), ret);
                    diag_util_printf("Set Flow Control Port FC-Off High Off Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_portFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_portFcoffHighThreshold_set(*threshold_ptr, offThresh), ret);
                    diag_util_printf("Set Flow Control Port FC-Off High On Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_portFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_portFcoffLowThreshold_set(onThresh, *threshold_ptr), ret);
                    diag_util_printf("Set Flow Control Port FC-Off Low Off Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_portFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_portFcoffLowThreshold_set(*threshold_ptr, offThresh), ret);
                    diag_util_printf("Set Flow Control Port FC-Off Low On Threshold to %u\n",*threshold_ptr);
                }
            }
            else if('f'==TOKEN_CHAR(4,0))
            {
                if(!osal_strcmp(TOKEN_STR(5),"high-off"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_portFconHighThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_portFconHighThreshold_set(onThresh, *threshold_ptr), ret);
                    diag_util_printf("Set Flow Control Port FC-On High Off Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_portFconHighThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_portFconHighThreshold_set(*threshold_ptr, offThresh), ret);
                    diag_util_printf("Set Flow Control Port FC-On High On Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_portFconLowThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_portFconLowThreshold_set(onThresh, *threshold_ptr), ret);
                    diag_util_printf("Set Flow Control Port FC-On Low Off Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_portFconLowThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_portFconLowThreshold_set(*threshold_ptr, offThresh), ret);
                    diag_util_printf("Set Flow Control Port FC-On Low On Threshold to %u\n",*threshold_ptr);
                }
            }
        }
        break;
#endif
#if defined(CONFIG_SDK_RTL9602C)
		case RTL9602C_CHIP_ID:
		{
			if('d'==TOKEN_CHAR(4,0))
			{
				if(!osal_strcmp(TOKEN_STR(5),"high-off"))
				{
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_portFcoffHighThreshold_get(&onThresh, &offThresh), ret);
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_portFcoffHighThreshold_set(onThresh, *threshold_ptr), ret);
					diag_util_printf("Set Flow Control Port FC-Off High Off Threshold to %u\n",*threshold_ptr);
				}
				else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
				{
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_portFcoffHighThreshold_get(&onThresh, &offThresh), ret);
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_portFcoffHighThreshold_set(*threshold_ptr, offThresh), ret);
					diag_util_printf("Set Flow Control Port FC-Off High On Threshold to %u\n",*threshold_ptr);
				}
				else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
				{
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_portFcoffLowThreshold_get(&onThresh, &offThresh), ret);
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_portFcoffLowThreshold_set(onThresh, *threshold_ptr), ret);
					diag_util_printf("Set Flow Control Port FC-Off Low Off Threshold to %u\n",*threshold_ptr);
				}
				else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
				{
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_portFcoffLowThreshold_get(&onThresh, &offThresh), ret);
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_portFcoffLowThreshold_set(*threshold_ptr, offThresh), ret);
					diag_util_printf("Set Flow Control Port FC-Off Low On Threshold to %u\n",*threshold_ptr);
				}
			}
			else if('f'==TOKEN_CHAR(4,0))
			{
				if(!osal_strcmp(TOKEN_STR(5),"high-off"))
				{
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_portFconHighThreshold_get(&onThresh, &offThresh), ret);
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_portFconHighThreshold_set(onThresh, *threshold_ptr), ret);
					diag_util_printf("Set Flow Control Port FC-On High Off Threshold to %u\n",*threshold_ptr);
				}
				else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
				{
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_portFconHighThreshold_get(&onThresh, &offThresh), ret);
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_portFconHighThreshold_set(*threshold_ptr, offThresh), ret);
					diag_util_printf("Set Flow Control Port FC-On High On Threshold to %u\n",*threshold_ptr);
				}
				else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
				{
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_portFconLowThreshold_get(&onThresh, &offThresh), ret);
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_portFconLowThreshold_set(onThresh, *threshold_ptr), ret);
					diag_util_printf("Set Flow Control Port FC-On Low Off Threshold to %u\n",*threshold_ptr);
				}
				else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
				{
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_portFconLowThreshold_get(&onThresh, &offThresh), ret);
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_portFconLowThreshold_set(*threshold_ptr, offThresh), ret);
					diag_util_printf("Set Flow Control Port FC-On Low On Threshold to %u\n",*threshold_ptr);
				}
			}
		}
		break;
#endif
#if defined(CONFIG_SDK_RTL9607B)
        case RTL9607B_CHIP_ID:
        {
            if('d'==TOKEN_CHAR(4,0))
            {
                if(!osal_strcmp(TOKEN_STR(5),"high-off"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_portFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_portFcoffHighThreshold_set(onThresh, *threshold_ptr), ret);
                    diag_util_printf("Set Flow Control Port FC-Off High Off Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_portFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_portFcoffHighThreshold_set(*threshold_ptr, offThresh), ret);
                    diag_util_printf("Set Flow Control Port FC-Off High On Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_portFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_portFcoffLowThreshold_set(onThresh, *threshold_ptr), ret);
                    diag_util_printf("Set Flow Control Port FC-Off Low Off Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_portFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_portFcoffLowThreshold_set(*threshold_ptr, offThresh), ret);
                    diag_util_printf("Set Flow Control Port FC-Off Low On Threshold to %u\n",*threshold_ptr);
                }
            }
            else if('f'==TOKEN_CHAR(4,0))
            {
                if(!osal_strcmp(TOKEN_STR(5),"high-off"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_portFconHighThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_portFconHighThreshold_set(onThresh, *threshold_ptr), ret);
                    diag_util_printf("Set Flow Control Port FC-On High Off Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_portFconHighThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_portFconHighThreshold_set(*threshold_ptr, offThresh), ret);
                    diag_util_printf("Set Flow Control Port FC-On High On Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_portFconLowThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_portFconLowThreshold_set(onThresh, *threshold_ptr), ret);
                    diag_util_printf("Set Flow Control Port FC-On Low Off Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_portFconLowThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_portFconLowThreshold_set(*threshold_ptr, offThresh), ret);
                    diag_util_printf("Set Flow Control Port FC-On Low On Threshold to %u\n",*threshold_ptr);
                }
            }
        }
        break;
#endif

        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_ingress_port_flowctrl_threshold_drop_threshold_high_off_high_on_low_off_low_on_threshold_threshold */

/*
 * flowctrl get ingress port ( flowctrl-threshold | drop-threshold ) ( high-off | high-on | low-off | low-on ) threshold
 */
cparser_result_t
cparser_cmd_flowctrl_get_ingress_port_flowctrl_threshold_drop_threshold_high_off_high_on_low_off_low_on_threshold(
    cparser_context_t *context)
{
    uint32 onThresh, offThresh;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
            if('d'==TOKEN_CHAR(4,0))
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
            else if('f'==TOKEN_CHAR(4,0))
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
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            if('d'==TOKEN_CHAR(4,0))
            {
                if(!osal_strcmp(TOKEN_STR(5),"high-off"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_mprintf("Flow Control Port FC-Off High Off Threshold is %u\n",offThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_mprintf("Flow Control Port FC-Off High On Threshold is %u\n",onThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_mprintf("Flow Control Port FC-Off Low Off Threshold is %u\n",offThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_mprintf("Flow Control Port FC-Off Low On Threshold is %u\n",onThresh);
                }
            }
            else if('f'==TOKEN_CHAR(4,0))
            {
                if(!osal_strcmp(TOKEN_STR(5),"high-off"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portFconHighThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_mprintf("Flow Control Port FC-On High Off Threshold is %u\n",offThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portFconHighThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_mprintf("Flow Control Port FC-On High On Threshold is %u\n",onThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portFconLowThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_mprintf("Flow Control Port FC-On Low Off Threshold is %u\n",offThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portFconLowThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_mprintf("Flow Control Port FC-On Low On Threshold is %u\n",onThresh);
                }
            }
        }
        break;
#endif
#if defined(CONFIG_SDK_RTL9601B)
        case RTL9601B_CHIP_ID:
        {
            if('d'==TOKEN_CHAR(4,0))
            {
                if(!osal_strcmp(TOKEN_STR(5),"high-off"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_portFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_mprintf("Flow Control Port FC-Off High Off Threshold is %u\n",offThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_portFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_mprintf("Flow Control Port FC-Off High On Threshold is %u\n",onThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_portFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_mprintf("Flow Control Port FC-Off Low Off Threshold is %u\n",offThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_portFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_mprintf("Flow Control Port FC-Off Low On Threshold is %u\n",onThresh);
                }
            }
            else if('f'==TOKEN_CHAR(4,0))
            {
                if(!osal_strcmp(TOKEN_STR(5),"high-off"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_portFconHighThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_mprintf("Flow Control Port FC-On High Off Threshold is %u\n",offThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_portFconHighThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_mprintf("Flow Control Port FC-On High On Threshold is %u\n",onThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_portFconLowThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_mprintf("Flow Control Port FC-On Low Off Threshold is %u\n",offThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_portFconLowThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_mprintf("Flow Control Port FC-On Low On Threshold is %u\n",onThresh);
                }
            }
        }
        break;
#endif
#if defined(CONFIG_SDK_RTL9602C)
		case RTL9602C_CHIP_ID:
		{
			if('d'==TOKEN_CHAR(4,0))
			{
				if(!osal_strcmp(TOKEN_STR(5),"high-off"))
				{
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_portFcoffHighThreshold_get(&onThresh, &offThresh), ret);
					diag_util_mprintf("Flow Control Port FC-Off High Off Threshold is %u\n",offThresh);
				}
				else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
				{
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_portFcoffHighThreshold_get(&onThresh, &offThresh), ret);
					diag_util_mprintf("Flow Control Port FC-Off High On Threshold is %u\n",onThresh);
				}
				else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
				{
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_portFcoffLowThreshold_get(&onThresh, &offThresh), ret);
					diag_util_mprintf("Flow Control Port FC-Off Low Off Threshold is %u\n",offThresh);
				}
				else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
				{
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_portFcoffLowThreshold_get(&onThresh, &offThresh), ret);
					diag_util_mprintf("Flow Control Port FC-Off Low On Threshold is %u\n",onThresh);
				}
			}
			else if('f'==TOKEN_CHAR(4,0))
			{
				if(!osal_strcmp(TOKEN_STR(5),"high-off"))
				{
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_portFconHighThreshold_get(&onThresh, &offThresh), ret);
					diag_util_mprintf("Flow Control Port FC-On High Off Threshold is %u\n",offThresh);
				}
				else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
				{
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_portFconHighThreshold_get(&onThresh, &offThresh), ret);
					diag_util_mprintf("Flow Control Port FC-On High On Threshold is %u\n",onThresh);
				}
				else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
				{
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_portFconLowThreshold_get(&onThresh, &offThresh), ret);
					diag_util_mprintf("Flow Control Port FC-On Low Off Threshold is %u\n",offThresh);
				}
				else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
				{
					DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_portFconLowThreshold_get(&onThresh, &offThresh), ret);
					diag_util_mprintf("Flow Control Port FC-On Low On Threshold is %u\n",onThresh);
				}
			}
		}
		break;
#endif
#if defined(CONFIG_SDK_RTL9607B)
        case RTL9607B_CHIP_ID:
        {
            if('d'==TOKEN_CHAR(4,0))
            {
                if(!osal_strcmp(TOKEN_STR(5),"high-off"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_portFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_mprintf("Flow Control Port FC-Off High Off Threshold is %u\n",offThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_portFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_mprintf("Flow Control Port FC-Off High On Threshold is %u\n",onThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_portFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_mprintf("Flow Control Port FC-Off Low Off Threshold is %u\n",offThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_portFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_mprintf("Flow Control Port FC-Off Low On Threshold is %u\n",onThresh);
                }
            }
            else if('f'==TOKEN_CHAR(4,0))
            {
                if(!osal_strcmp(TOKEN_STR(5),"high-off"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_portFconHighThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_mprintf("Flow Control Port FC-On High Off Threshold is %u\n",offThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_portFconHighThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_mprintf("Flow Control Port FC-On High On Threshold is %u\n",onThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_portFconLowThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_mprintf("Flow Control Port FC-On Low Off Threshold is %u\n",offThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
                {
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_portFconLowThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_mprintf("Flow Control Port FC-On Low On Threshold is %u\n",onThresh);
                }
            }
        }
        break;
#endif

        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_ingress_port_flowctrl_threshold_drop_threshold_high_off_high_on_low_off_low_on_threshold */

/*
 * flowctrl set ingress egress-drop port ( <PORT_LIST:ports> | all ) threshold <UINT:threshold>
 */
cparser_result_t
cparser_cmd_flowctrl_set_ingress_egress_drop_port_ports_all_threshold_threshold(
    cparser_context_t *context,
    char * *ports_ptr,
    uint32_t  *threshold_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    diag_util_printf("Set Ingress Flow Control Port Egress Drop Threshold \n");
    diag_util_printf("Egress-Port       Drop-Threshold \n");
    diag_util_printf("-----------------------------\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        switch(DIAG_UTIL_CHIP_TYPE)
        {
#if defined(CONFIG_SDK_APOLLO)
            case APOLLO_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portEegressDropThreshold_set(port, *threshold_ptr), ret);
            }
            break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
            case APOLLOMP_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portEegressDropThreshold_set(port, *threshold_ptr), ret);
            }
            break;
#endif
#if defined(CONFIG_SDK_RTL9601B)
            case RTL9601B_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_portEegressDropThreshold_set(port, *threshold_ptr), ret);
            }
            break;
#endif
#if defined(CONFIG_SDK_RTL9602C)
			case RTL9602C_CHIP_ID:
			{
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_portEegressDropThreshold_set(port, *threshold_ptr), ret);
			}
			break;
#endif
#if defined(CONFIG_SDK_RTL9607B)
            case RTL9607B_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_portEegressDropThreshold_set(port, *threshold_ptr), ret);
            }
            break;
#endif

            default:
                diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
                return CPARSER_NOT_OK;
            break;
        }
        diag_util_mprintf("%-18u  %u\n", port, *threshold_ptr);
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_ingress_egress_drop_port_ports_all_threshold_threshold */

/*
 * flowctrl get ingress egress-drop port ( <PORT_LIST:ports> | all ) threshold
 */
cparser_result_t
cparser_cmd_flowctrl_get_ingress_egress_drop_port_ports_all_threshold(
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
        switch(DIAG_UTIL_CHIP_TYPE)
        {
    #if defined(CONFIG_SDK_APOLLO)
            case APOLLO_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portEegressDropThreshold_get(port, &threshold), ret);
            }
            break;
    #endif
    #if defined(CONFIG_SDK_APOLLOMP)
            case APOLLOMP_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portEegressDropThreshold_get(port, &threshold), ret);
            }
            break;
    #endif
    #if defined(CONFIG_SDK_RTL9601B)
            case RTL9601B_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_portEegressDropThreshold_get(port, &threshold), ret);
            }
            break;
    #endif
	#if defined(CONFIG_SDK_RTL9602C)
            case RTL9602C_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_portEegressDropThreshold_get(port, &threshold), ret);
            }
            break;
	#endif
    #if defined(CONFIG_SDK_RTL9607B)
            case RTL9607B_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_portEegressDropThreshold_get(port, &threshold), ret);
            }
            break;
    #endif
            default:
                diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
                return CPARSER_NOT_OK;
            break;
        }
        diag_util_mprintf("%-18u  %u\n", port, threshold);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_ingress_egress_drop_port_ports_all_threshold */

/*
 * flowctrl set ingress egress-drop queue-id ( <MASK_LIST:qid> | all ) threshold <UINT:threshold>
 */
cparser_result_t
cparser_cmd_flowctrl_set_ingress_egress_drop_queue_id_qid_all_threshold_threshold(
    cparser_context_t *context,
    char * *qid_ptr,
    uint32_t  *threshold_ptr)
{
    diag_mask_t  queuelist;
    rtk_qid_t queue;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_QUEUEMASK(queuelist, 5), ret);

    diag_util_printf("Set Ingress Flow Control Queue Egress Drop Threshold \n");
    diag_util_printf("Egress-Queue       Drop-Threshold \n");
    diag_util_printf("---------------------------------\n");
    DIAG_UTIL_MASK_SCAN(queuelist, queue)
    {
        switch(DIAG_UTIL_CHIP_TYPE)
        {
    #if defined(CONFIG_SDK_APOLLO)
            case APOLLO_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_queueEegressDropThreshold_set(queue, *threshold_ptr), ret);
            }
            break;
    #endif
    #if defined(CONFIG_SDK_APOLLOMP)
            case APOLLOMP_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_queueEegressDropThreshold_set(queue, *threshold_ptr), ret);
            }
            break;
    #endif
    #if defined(CONFIG_SDK_RTL9601B)
            case RTL9601B_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_queueEegressDropThreshold_set(queue, *threshold_ptr), ret);
            }
            break;
    #endif

	#if defined(CONFIG_SDK_RTL9602C)
            case RTL9602C_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_queueEegressDropThreshold_set(queue, *threshold_ptr), ret);
            }
            break;
	#endif

    #if defined(CONFIG_SDK_RTL9607B)
            case RTL9607B_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_queueEegressDropThreshold_set(queue, *threshold_ptr), ret);
            }
            break;
    #endif
            default:
                diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
                return CPARSER_NOT_OK;
            break;
        }

        diag_util_printf("%-18u  %u\n", queue, *threshold_ptr);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_ingress_egress_drop_queue_id_qid_all_threshold_threshold */

/*
 * flowctrl get ingress egress-drop queue-id ( <MASK_LIST:qid> | all ) threshold
 */
cparser_result_t
cparser_cmd_flowctrl_get_ingress_egress_drop_queue_id_qid_all_threshold(
    cparser_context_t *context,
    char * *qid_ptr)
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
        switch(DIAG_UTIL_CHIP_TYPE)
        {
    #if defined(CONFIG_SDK_APOLLO)
            case APOLLO_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_queueEegressDropThreshold_get(queue, &threshold), ret);
            }
            break;
    #endif
    #if defined(CONFIG_SDK_APOLLOMP)
            case APOLLOMP_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_queueEegressDropThreshold_get(queue, &threshold), ret);
            }
            break;
    #endif
    #if defined(CONFIG_SDK_RTL9601B)
            case RTL9601B_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_queueEegressDropThreshold_get(queue, &threshold), ret);
            }
            break;
    #endif

	#if defined(CONFIG_SDK_RTL9602C)
            case RTL9602C_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_queueEegressDropThreshold_get(queue, &threshold), ret);
            }
            break;
	#endif

    #if defined(CONFIG_SDK_RTL9607B)
            case RTL9607B_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_queueEegressDropThreshold_get(queue, &threshold), ret);
            }
            break;
    #endif
            default:
                diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
                return CPARSER_NOT_OK;
            break;
        }
        diag_util_mprintf("%-18u  %u\n", queue, threshold);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_ingress_egress_drop_queue_id_qid_all_threshold */

/*
 * flowctrl set ingress egress-drop ( port-gap | queue-gap ) threshold <UINT:threshold>
 */
cparser_result_t
cparser_cmd_flowctrl_set_ingress_egress_drop_port_gap_queue_gap_threshold_threshold(
    cparser_context_t *context,
    uint32_t  *threshold_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
            if('p'==TOKEN_CHAR(4,0))
            {
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portEegressGapThreshold_set(*threshold_ptr), ret);
                diag_util_printf("Set Ingress Flow Control Port Egress Gap Threshold to %u \n", *threshold_ptr);
            }
            else if('q'==TOKEN_CHAR(4,0))
            {
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_queueEegressGapThreshold_set(*threshold_ptr), ret);
                diag_util_printf("Set Ingress Flow Control Queue Egress Gap Threshold to %u \n", *threshold_ptr);
            }
            else
                return RT_ERR_FAILED;
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            if('p'==TOKEN_CHAR(4,0))
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portEegressGapThreshold_set(*threshold_ptr), ret);
                diag_util_printf("Set Ingress Flow Control Port Egress Gap Threshold to %u \n", *threshold_ptr);
            }
            else if('q'==TOKEN_CHAR(4,0))
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_queueEegressGapThreshold_set(*threshold_ptr), ret);
                diag_util_printf("Set Ingress Flow Control Queue Egress Gap Threshold to %u \n", *threshold_ptr);
            }
            else
                return RT_ERR_FAILED;
        }
        break;
#endif
#if defined(CONFIG_SDK_RTL9601B)
        case RTL9601B_CHIP_ID:
        {
            if('p'==TOKEN_CHAR(4,0))
            {
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_portEegressGapThreshold_set(*threshold_ptr), ret);
                diag_util_printf("Set Ingress Flow Control Port Egress Gap Threshold to %u \n", *threshold_ptr);
            }
            else if('q'==TOKEN_CHAR(4,0))
            {
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_queueEegressGapThreshold_set(*threshold_ptr), ret);
                diag_util_printf("Set Ingress Flow Control Queue Egress Gap Threshold to %u \n", *threshold_ptr);
            }
            else
                return RT_ERR_FAILED;
        }
        break;
#endif
        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_ingress_egress_drop_port_gap_queue_gap_threshold_threshold */

/*
 * flowctrl get ingress egress-drop ( port-gap | queue-gap ) threshold
 */
cparser_result_t
cparser_cmd_flowctrl_get_ingress_egress_drop_port_gap_queue_gap_threshold(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    uint32 threshold;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
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
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            if('p'==TOKEN_CHAR(4,0))
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portEegressGapThreshold_get(&threshold), ret);
                diag_util_mprintf("Ingress Flow Control Port Egress Gap Threshold is %u \n", threshold);
            }
            else if('q'==TOKEN_CHAR(4,0))
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_queueEegressGapThreshold_get(&threshold), ret);
                diag_util_mprintf("Ingress Flow Control Queue Egress Gap Threshold is %u \n", threshold);
            }
            else
                return RT_ERR_FAILED;
        }
        break;
#endif
#if defined(CONFIG_SDK_RTL9601B)
        case RTL9601B_CHIP_ID:
        {
            if('p'==TOKEN_CHAR(4,0))
            {
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_portEegressGapThreshold_get(&threshold), ret);
                diag_util_mprintf("Ingress Flow Control Port Egress Gap Threshold is %u \n", threshold);
            }
            else if('q'==TOKEN_CHAR(4,0))
            {
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_queueEegressGapThreshold_get(&threshold), ret);
                diag_util_mprintf("Ingress Flow Control Queue Egress Gap Threshold is %u \n", threshold);
            }
            else
                return RT_ERR_FAILED;
        }
        break;
#endif
        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_ingress_egress_drop_port_gap_queue_gap_threshold */

/*
 * flowctrl set ingress egress-drop port ( <PORT_LIST:ports> | all ) queue-id ( <MASK_LIST:qid> | all ) drop ( enable | disable )
 */
cparser_result_t
cparser_cmd_flowctrl_set_ingress_egress_drop_port_ports_all_queue_id_qid_all_drop_enable_disable(
    cparser_context_t *context,
    char * *ports_ptr,
    char * *qid_ptr)
{
    diag_portlist_t portlist;
    diag_mask_t  queuelist;
    rtk_port_t port;
    rtk_qid_t queue;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_QUEUEMASK(queuelist, 7), ret);

    diag_util_printf("Set Ingress Flow Control Queue Egress Drop Enable \n");
    diag_util_printf("            ");
    DIAG_UTIL_MASK_SCAN(queuelist, queue)
    {
        diag_util_printf("Q%d    ",queue);
    }
    diag_util_printf("\n");
    diag_util_printf("---------------------------------------------------------\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_printf("Port%2u  ", port);
        DIAG_UTIL_MASK_SCAN(queuelist, queue)
        {
            switch(DIAG_UTIL_CHIP_TYPE)
            {
        #if defined(CONFIG_SDK_APOLLO)
                case APOLLO_CHIP_ID:
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
                break;
        #endif
        #if defined(CONFIG_SDK_APOLLOMP)
                case APOLLOMP_CHIP_ID:
                {
                    if('e'==TOKEN_CHAR(9,0))
                    {
                        DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_egressDropEnable_set(port, queue, ENABLED), ret);
                        diag_util_printf("    En");
                    }
                    else if('d'==TOKEN_CHAR(9,0))
                    {
                        DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_egressDropEnable_set(port, queue, DISABLED), ret);
                        diag_util_printf("   Dis");
                    }
                    else
                        return RT_ERR_FAILED;
                }
                break;
        #endif
        #if defined(CONFIG_SDK_RTL9601B)
                case RTL9601B_CHIP_ID:
                {
                    if('e'==TOKEN_CHAR(9,0))
                    {
                        DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_egressDropEnable_set(port, queue, ENABLED), ret);
                        diag_util_printf("    En");
                    }
                    else if('d'==TOKEN_CHAR(9,0))
                    {
                        DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_egressDropEnable_set(port, queue, DISABLED), ret);
                        diag_util_printf("   Dis");
                    }
                    else
                        return RT_ERR_FAILED;
                }
                break;
        #endif

		#if defined(CONFIG_SDK_RTL9602C)
                case RTL9602C_CHIP_ID:
                {
                    if('e'==TOKEN_CHAR(9,0))
                    {
                        DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_egressDropEnable_set(port, queue, ENABLED), ret);
                        diag_util_printf("    En");
                    }
                    else if('d'==TOKEN_CHAR(9,0))
                    {
                        DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_egressDropEnable_set(port, queue, DISABLED), ret);
                        diag_util_printf("   Dis");
                    }
                    else
                        return RT_ERR_FAILED;
                }
                break;
		#endif

        #if defined(CONFIG_SDK_RTL9607B)
                case RTL9607B_CHIP_ID:
                {
                    if('e'==TOKEN_CHAR(9,0))
                    {
                        DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_egressDropEnable_set(port, queue, ENABLED), ret);
                        diag_util_printf("    En");
                    }
                    else if('d'==TOKEN_CHAR(9,0))
                    {
                        DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_egressDropEnable_set(port, queue, DISABLED), ret);
                        diag_util_printf("   Dis");
                    }
                    else
                        return RT_ERR_FAILED;
                }
                break;
        #endif
                default:
                    diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
                    return CPARSER_NOT_OK;
                break;
            }
        }
        diag_util_printf("\n");
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_ingress_egress_drop_port_ports_all_queue_id_qid_all_drop_enable_disable */

/*
 * flowctrl get ingress egress-drop port ( <PORT_LIST:ports> | all ) queue-id ( <MASK_LIST:qid> | all ) drop
 */
cparser_result_t
cparser_cmd_flowctrl_get_ingress_egress_drop_port_ports_all_queue_id_qid_all_drop(
    cparser_context_t *context,
    char * *ports_ptr,
    char * *qid_ptr)
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
            switch(DIAG_UTIL_CHIP_TYPE)
            {
        #if defined(CONFIG_SDK_APOLLO)
                case APOLLO_CHIP_ID:
                {
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_egressDropEnable_get(port, queue, &enable), ret);
                }
                break;
        #endif
        #if defined(CONFIG_SDK_APOLLOMP)
                case APOLLOMP_CHIP_ID:
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_egressDropEnable_get(port, queue, &enable), ret);
                }
                break;
        #endif
        #if defined(CONFIG_SDK_RTL9601B)
                case RTL9601B_CHIP_ID:
                {
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_egressDropEnable_get(port, queue, &enable), ret);
                }
                break;
        #endif

		#if defined(CONFIG_SDK_RTL9602C)
                case RTL9602C_CHIP_ID:
                {
                    DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_egressDropEnable_get(port, queue, &enable), ret);
                }
                break;
		#endif

        #if defined(CONFIG_SDK_RTL9607B)
                case RTL9607B_CHIP_ID:
                {
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_egressDropEnable_get(port, queue, &enable), ret);
                }
                break;
        #endif
                default:
                    diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
                    return CPARSER_NOT_OK;
                break;
            }
            diag_util_printf("   %3s",enable?"En":"Dis");
        }
        diag_util_mprintf("\n");
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_ingress_egress_drop_port_ports_all_queue_id_qid_all_drop */

/*
 * flowctrl set ingress pon system ( high-off | high-on | low-off | low-on ) threshold <UINT:threshold>
 */
cparser_result_t
cparser_cmd_flowctrl_set_ingress_pon_system_high_off_high_on_low_off_low_on_threshold_threshold(
    cparser_context_t *context,
    uint32_t  *threshold_ptr)
{
    uint32 onThresh, offThresh;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
            if(!osal_strcmp(TOKEN_STR(5),"high-off"))
            {
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponGlobalHighThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponGlobalHighThreshold_set(onThresh, *threshold_ptr), ret);
                diag_util_printf("Set PON MAC Global High Off Threshold to %u\n",*threshold_ptr);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
            {
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponGlobalHighThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponGlobalHighThreshold_set(*threshold_ptr, offThresh), ret);
                diag_util_printf("Set PON MAC Global High On Threshold to %u\n",*threshold_ptr);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
            {
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponGlobalLowThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponGlobalLowThreshold_set(onThresh, *threshold_ptr), ret);
                diag_util_printf("Set PON MAC Global Low Off Threshold to %u\n",*threshold_ptr);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
            {
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponGlobalLowThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponGlobalLowThreshold_set(*threshold_ptr, offThresh), ret);
                diag_util_printf("Set PON MAC Global Low On Threshold to %u\n",*threshold_ptr);
            }
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            if(!osal_strcmp(TOKEN_STR(5),"high-off"))
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponGlobalHighThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponGlobalHighThreshold_set(onThresh, *threshold_ptr), ret);
                diag_util_printf("Set PON MAC Global High Off Threshold to %u\n",*threshold_ptr);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponGlobalHighThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponGlobalHighThreshold_set(*threshold_ptr, offThresh), ret);
                diag_util_printf("Set PON MAC Global High On Threshold to %u\n",*threshold_ptr);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponGlobalLowThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponGlobalLowThreshold_set(onThresh, *threshold_ptr), ret);
                diag_util_printf("Set PON MAC Global Low Off Threshold to %u\n",*threshold_ptr);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponGlobalLowThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponGlobalLowThreshold_set(*threshold_ptr, offThresh), ret);
                diag_util_printf("Set PON MAC Global Low On Threshold to %u\n",*threshold_ptr);
            }
        }
        break;
#endif
#if defined(CONFIG_SDK_RTL9601B)
        case RTL9601B_CHIP_ID:
        {
            if(!osal_strcmp(TOKEN_STR(5),"high-off"))
            {
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_ponGlobalHighThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_ponGlobalHighThreshold_set(onThresh, *threshold_ptr), ret);
                diag_util_printf("Set PON MAC Global High Off Threshold to %u\n",*threshold_ptr);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
            {
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_ponGlobalHighThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_ponGlobalHighThreshold_set(*threshold_ptr, offThresh), ret);
                diag_util_printf("Set PON MAC Global High On Threshold to %u\n",*threshold_ptr);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
            {
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_ponGlobalLowThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_ponGlobalLowThreshold_set(onThresh, *threshold_ptr), ret);
                diag_util_printf("Set PON MAC Global Low Off Threshold to %u\n",*threshold_ptr);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
            {
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_ponGlobalLowThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_ponGlobalLowThreshold_set(*threshold_ptr, offThresh), ret);
                diag_util_printf("Set PON MAC Global Low On Threshold to %u\n",*threshold_ptr);
            }
        }
        break;
#endif
#if defined(CONFIG_SDK_RTL9602C)
		case RTL9602C_CHIP_ID:
		{
			if(!osal_strcmp(TOKEN_STR(5),"high-off"))
			{
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_ponGlobalHighThreshold_get(&onThresh, &offThresh), ret);
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_ponGlobalHighThreshold_set(onThresh, *threshold_ptr), ret);
				diag_util_printf("Set PON MAC Global High Off Threshold to %u\n",*threshold_ptr);
			}
			else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
			{
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_ponGlobalHighThreshold_get(&onThresh, &offThresh), ret);
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_ponGlobalHighThreshold_set(*threshold_ptr, offThresh), ret);
				diag_util_printf("Set PON MAC Global High On Threshold to %u\n",*threshold_ptr);
			}
			else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
			{
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_ponGlobalLowThreshold_get(&onThresh, &offThresh), ret);
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_ponGlobalLowThreshold_set(onThresh, *threshold_ptr), ret);
				diag_util_printf("Set PON MAC Global Low Off Threshold to %u\n",*threshold_ptr);
			}
			else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
			{
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_ponGlobalLowThreshold_get(&onThresh, &offThresh), ret);
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_ponGlobalLowThreshold_set(*threshold_ptr, offThresh), ret);
				diag_util_printf("Set PON MAC Global Low On Threshold to %u\n",*threshold_ptr);
			}
		}
		break;
#endif
#if defined(CONFIG_SDK_RTL9607B)
        case RTL9607B_CHIP_ID:
        {
            if(!osal_strcmp(TOKEN_STR(5),"high-off"))
            {
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_ponGlobalHighThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_ponGlobalHighThreshold_set(onThresh, *threshold_ptr), ret);
                diag_util_printf("Set PON MAC Global High Off Threshold to %u\n",*threshold_ptr);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
            {
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_ponGlobalHighThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_ponGlobalHighThreshold_set(*threshold_ptr, offThresh), ret);
                diag_util_printf("Set PON MAC Global High On Threshold to %u\n",*threshold_ptr);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
            {
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_ponGlobalLowThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_ponGlobalLowThreshold_set(onThresh, *threshold_ptr), ret);
                diag_util_printf("Set PON MAC Global Low Off Threshold to %u\n",*threshold_ptr);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
            {
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_ponGlobalLowThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_ponGlobalLowThreshold_set(*threshold_ptr, offThresh), ret);
                diag_util_printf("Set PON MAC Global Low On Threshold to %u\n",*threshold_ptr);
            }
        }
        break;
#endif
        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_ingress_pon_system_high_off_high_on_low_off_low_on_threshold_threshold */

/*
 * flowctrl get ingress pon system ( high-off | high-on | low-off | low-on ) threshold
 */
cparser_result_t
cparser_cmd_flowctrl_get_ingress_pon_system_high_off_high_on_low_off_low_on_threshold(
    cparser_context_t *context)
{
    uint32 onThresh, offThresh;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
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
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            if(!osal_strcmp(TOKEN_STR(5),"high-off"))
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponGlobalHighThreshold_get(&onThresh, &offThresh), ret);
                diag_util_mprintf("PON MAC Global High Off Threshold is %u\n",offThresh);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponGlobalHighThreshold_get(&onThresh, &offThresh), ret);
                diag_util_mprintf("PON MAC Global High On Threshold is %u\n",onThresh);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponGlobalLowThreshold_get(&onThresh, &offThresh), ret);
                diag_util_mprintf("PON MAC Global Low Off Threshold is %u\n",offThresh);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponGlobalLowThreshold_get(&onThresh, &offThresh), ret);
                diag_util_mprintf("PON MAC Global Low On Threshold is %u\n",onThresh);
            }
        }
        break;
#endif
#if defined(CONFIG_SDK_RTL9601B)
        case RTL9601B_CHIP_ID:
        {
            if(!osal_strcmp(TOKEN_STR(5),"high-off"))
            {
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_ponGlobalHighThreshold_get(&onThresh, &offThresh), ret);
                diag_util_mprintf("PON MAC Global High Off Threshold is %u\n",offThresh);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
            {
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_ponGlobalHighThreshold_get(&onThresh, &offThresh), ret);
                diag_util_mprintf("PON MAC Global High On Threshold is %u\n",onThresh);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
            {
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_ponGlobalLowThreshold_get(&onThresh, &offThresh), ret);
                diag_util_mprintf("PON MAC Global Low Off Threshold is %u\n",offThresh);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
            {
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_ponGlobalLowThreshold_get(&onThresh, &offThresh), ret);
                diag_util_mprintf("PON MAC Global Low On Threshold is %u\n",onThresh);
            }
        }
        break;
#endif
#if defined(CONFIG_SDK_RTL9602C)
		case RTL9602C_CHIP_ID:
		{
			if(!osal_strcmp(TOKEN_STR(5),"high-off"))
			{
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_ponGlobalHighThreshold_get(&onThresh, &offThresh), ret);
				diag_util_mprintf("PON MAC Global High Off Threshold is %u\n",offThresh);
			}
			else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
			{
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_ponGlobalHighThreshold_get(&onThresh, &offThresh), ret);
				diag_util_mprintf("PON MAC Global High On Threshold is %u\n",onThresh);
			}
			else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
			{
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_ponGlobalLowThreshold_get(&onThresh, &offThresh), ret);
				diag_util_mprintf("PON MAC Global Low Off Threshold is %u\n",offThresh);
			}
			else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
			{
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_ponGlobalLowThreshold_get(&onThresh, &offThresh), ret);
				diag_util_mprintf("PON MAC Global Low On Threshold is %u\n",onThresh);
			}
		}
		break;
#endif
#if defined(CONFIG_SDK_RTL9607B)
        case RTL9607B_CHIP_ID:
        {
            if(!osal_strcmp(TOKEN_STR(5),"high-off"))
            {
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_ponGlobalHighThreshold_get(&onThresh, &offThresh), ret);
                diag_util_mprintf("PON MAC Global High Off Threshold is %u\n",offThresh);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
            {
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_ponGlobalHighThreshold_get(&onThresh, &offThresh), ret);
                diag_util_mprintf("PON MAC Global High On Threshold is %u\n",onThresh);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
            {
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_ponGlobalLowThreshold_get(&onThresh, &offThresh), ret);
                diag_util_mprintf("PON MAC Global Low Off Threshold is %u\n",offThresh);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
            {
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_ponGlobalLowThreshold_get(&onThresh, &offThresh), ret);
                diag_util_mprintf("PON MAC Global Low On Threshold is %u\n",onThresh);
            }
        }
        break;
#endif

        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_ingress_pon_system_high_off_high_on_low_off_low_on_threshold */

/*
 * flowctrl set ingress pon port ( high-off | high-on | low-off | low-on ) threshold <UINT:threshold>
 */
cparser_result_t
cparser_cmd_flowctrl_set_ingress_pon_port_high_off_high_on_low_off_low_on_threshold_threshold(
    cparser_context_t *context,
    uint32_t  *threshold_ptr)
{
    uint32 onThresh, offThresh;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
            if(!osal_strcmp(TOKEN_STR(5),"high-off"))
            {
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponPortHighThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponPortHighThreshold_set(onThresh, *threshold_ptr), ret);
                diag_util_printf("Set PON MAC Port High Off Threshold to %u\n",*threshold_ptr);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
            {
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponPortHighThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponPortHighThreshold_set(*threshold_ptr, offThresh), ret);
                diag_util_printf("Set PON MAC Port High On Threshold to %u\n",*threshold_ptr);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
            {
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponPortLowThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponPortLowThreshold_set(onThresh, *threshold_ptr), ret);
                diag_util_printf("Set PON MAC Port Low Off Threshold to %u\n",*threshold_ptr);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
            {
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponPortLowThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponPortLowThreshold_set(*threshold_ptr, offThresh), ret);
                diag_util_printf("Set PON MAC Port Low On Threshold to %u\n",*threshold_ptr);
            }
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            if(!osal_strcmp(TOKEN_STR(5),"high-off"))
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponPortHighThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponPortHighThreshold_set(onThresh, *threshold_ptr), ret);
                diag_util_printf("Set PON MAC Port High Off Threshold to %u\n",*threshold_ptr);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponPortHighThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponPortHighThreshold_set(*threshold_ptr, offThresh), ret);
                diag_util_printf("Set PON MAC Port High On Threshold to %u\n",*threshold_ptr);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponPortLowThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponPortLowThreshold_set(onThresh, *threshold_ptr), ret);
                diag_util_printf("Set PON MAC Port Low Off Threshold to %u\n",*threshold_ptr);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponPortLowThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponPortLowThreshold_set(*threshold_ptr, offThresh), ret);
                diag_util_printf("Set PON MAC Port Low On Threshold to %u\n",*threshold_ptr);
            }
        }
        break;
#endif
#if defined(CONFIG_SDK_RTL9601B)
        case RTL9601B_CHIP_ID:
        {
            if(!osal_strcmp(TOKEN_STR(5),"high-off"))
            {
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_ponPortHighThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_ponPortHighThreshold_set(onThresh, *threshold_ptr), ret);
                diag_util_printf("Set PON MAC Port High Off Threshold to %u\n",*threshold_ptr);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
            {
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_ponPortHighThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_ponPortHighThreshold_set(*threshold_ptr, offThresh), ret);
                diag_util_printf("Set PON MAC Port High On Threshold to %u\n",*threshold_ptr);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
            {
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_ponPortLowThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_ponPortLowThreshold_set(onThresh, *threshold_ptr), ret);
                diag_util_printf("Set PON MAC Port Low Off Threshold to %u\n",*threshold_ptr);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
            {
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_ponPortLowThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_ponPortLowThreshold_set(*threshold_ptr, offThresh), ret);
                diag_util_printf("Set PON MAC Port Low On Threshold to %u\n",*threshold_ptr);
            }
        }
        break;
#endif
#if defined(CONFIG_SDK_RTL9602C)
		case RTL9602C_CHIP_ID:
		{
			if(!osal_strcmp(TOKEN_STR(5),"high-off"))
			{
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_ponPortHighThreshold_get(&onThresh, &offThresh), ret);
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_ponPortHighThreshold_set(onThresh, *threshold_ptr), ret);
				diag_util_printf("Set PON MAC Port High Off Threshold to %u\n",*threshold_ptr);
			}
			else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
			{
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_ponPortHighThreshold_get(&onThresh, &offThresh), ret);
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_ponPortHighThreshold_set(*threshold_ptr, offThresh), ret);
				diag_util_printf("Set PON MAC Port High On Threshold to %u\n",*threshold_ptr);
			}
			else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
			{
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_ponPortLowThreshold_get(&onThresh, &offThresh), ret);
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_ponPortLowThreshold_set(onThresh, *threshold_ptr), ret);
				diag_util_printf("Set PON MAC Port Low Off Threshold to %u\n",*threshold_ptr);
			}
			else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
			{
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_ponPortLowThreshold_get(&onThresh, &offThresh), ret);
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_ponPortLowThreshold_set(*threshold_ptr, offThresh), ret);
				diag_util_printf("Set PON MAC Port Low On Threshold to %u\n",*threshold_ptr);
			}
		}
		break;
#endif
#if defined(CONFIG_SDK_RTL9607B)
        case RTL9607B_CHIP_ID:
        {
            if(!osal_strcmp(TOKEN_STR(5),"high-off"))
            {
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_ponPortHighThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_ponPortHighThreshold_set(onThresh, *threshold_ptr), ret);
                diag_util_printf("Set PON MAC Port High Off Threshold to %u\n",*threshold_ptr);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
            {
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_ponPortHighThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_ponPortHighThreshold_set(*threshold_ptr, offThresh), ret);
                diag_util_printf("Set PON MAC Port High On Threshold to %u\n",*threshold_ptr);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
            {
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_ponPortLowThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_ponPortLowThreshold_set(onThresh, *threshold_ptr), ret);
                diag_util_printf("Set PON MAC Port Low Off Threshold to %u\n",*threshold_ptr);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
            {
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_ponPortLowThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_ponPortLowThreshold_set(*threshold_ptr, offThresh), ret);
                diag_util_printf("Set PON MAC Port Low On Threshold to %u\n",*threshold_ptr);
            }
        }
        break;
#endif

        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_ingress_pon_port_high_off_high_on_low_off_low_on_threshold_threshold */

/*
 * flowctrl get ingress pon port ( high-off | high-on | low-off | low-on ) threshold
 */
cparser_result_t
cparser_cmd_flowctrl_get_ingress_pon_port_high_off_high_on_low_off_low_on_threshold(
    cparser_context_t *context)
{
    uint32 onThresh, offThresh;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
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
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            if(!osal_strcmp(TOKEN_STR(5),"high-off"))
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponPortHighThreshold_get(&onThresh, &offThresh), ret);
                diag_util_mprintf("PON MAC Port High Off Threshold is %u\n",offThresh);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponPortHighThreshold_get(&onThresh, &offThresh), ret);
                diag_util_mprintf("PON MAC Port High On Threshold is %u\n",onThresh);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponPortLowThreshold_get(&onThresh, &offThresh), ret);
                diag_util_mprintf("PON MAC Port Low Off Threshold is %u\n",offThresh);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponPortLowThreshold_get(&onThresh, &offThresh), ret);
                diag_util_mprintf("PON MAC Port Low On Threshold is %u\n",onThresh);
            }
        }
        break;
#endif
#if defined(CONFIG_SDK_RTL9601B)
        case RTL9601B_CHIP_ID:
        {
            if(!osal_strcmp(TOKEN_STR(5),"high-off"))
            {
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_ponPortHighThreshold_get(&onThresh, &offThresh), ret);
                diag_util_mprintf("PON MAC Port High Off Threshold is %u\n",offThresh);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
            {
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_ponPortHighThreshold_get(&onThresh, &offThresh), ret);
                diag_util_mprintf("PON MAC Port High On Threshold is %u\n",onThresh);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
            {
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_ponPortLowThreshold_get(&onThresh, &offThresh), ret);
                diag_util_mprintf("PON MAC Port Low Off Threshold is %u\n",offThresh);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
            {
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_ponPortLowThreshold_get(&onThresh, &offThresh), ret);
                diag_util_mprintf("PON MAC Port Low On Threshold is %u\n",onThresh);
            }
        }
        break;
#endif
#if defined(CONFIG_SDK_RTL9602C)
		case RTL9602C_CHIP_ID:
		{
			if(!osal_strcmp(TOKEN_STR(5),"high-off"))
			{
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_ponPortHighThreshold_get(&onThresh, &offThresh), ret);
				diag_util_mprintf("PON MAC Port High Off Threshold is %u\n",offThresh);
			}
			else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
			{
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_ponPortHighThreshold_get(&onThresh, &offThresh), ret);
				diag_util_mprintf("PON MAC Port High On Threshold is %u\n",onThresh);
			}
			else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
			{
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_ponPortLowThreshold_get(&onThresh, &offThresh), ret);
				diag_util_mprintf("PON MAC Port Low Off Threshold is %u\n",offThresh);
			}
			else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
			{
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_ponPortLowThreshold_get(&onThresh, &offThresh), ret);
				diag_util_mprintf("PON MAC Port Low On Threshold is %u\n",onThresh);
			}
		}
		break;
#endif
#if defined(CONFIG_SDK_RTL9607B)
        case RTL9607B_CHIP_ID:
        {
            if(!osal_strcmp(TOKEN_STR(5),"high-off"))
            {
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_ponPortHighThreshold_get(&onThresh, &offThresh), ret);
                diag_util_mprintf("PON MAC Port High Off Threshold is %u\n",offThresh);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
            {
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_ponPortHighThreshold_get(&onThresh, &offThresh), ret);
                diag_util_mprintf("PON MAC Port High On Threshold is %u\n",onThresh);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
            {
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_ponPortLowThreshold_get(&onThresh, &offThresh), ret);
                diag_util_mprintf("PON MAC Port Low Off Threshold is %u\n",offThresh);
            }
            else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
            {
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_ponPortLowThreshold_get(&onThresh, &offThresh), ret);
                diag_util_mprintf("PON MAC Port Low On Threshold is %u\n",onThresh);
            }
        }
        break;
#endif

        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_ingress_pon_port_high_off_high_on_low_off_low_on_threshold */

/*
 * flowctrl set ingress pon sid <UINT:sid> state ( enable | disable )
 */
cparser_result_t
cparser_cmd_flowctrl_set_ingress_pon_sid_sid_state_enable_disable(
    cparser_context_t *context,
    uint32_t  *sid_ptr)
{
    int32 ret = RT_ERR_FAILED;
    rtk_enable_t state;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if ('e' == TOKEN_CHAR(7,0))
    {
        state = ENABLED;
    }
    else if ('d' == TOKEN_CHAR(7,0))
    {
        state = DISABLED;
    }

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_RTL9601B)
        case RTL9601B_CHIP_ID:
        {
	    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_ponSidState_set(*sid_ptr, state), ret);
	    diag_util_mprintf("PON SID %u State %s \n", *sid_ptr, diagStr_enable[state]);
        }
        break;
#endif
#if defined(CONFIG_SDK_RTL9602C)
		case RTL9602C_CHIP_ID:
		{
			DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_ponSidState_set(*sid_ptr, state), ret);
			diag_util_mprintf("PON SID %u State %s \n", *sid_ptr, diagStr_enable[state]);
		}
		break;
#endif
#if defined(CONFIG_SDK_RTL9607B)
        case RTL9607B_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_ponSidState_set(*sid_ptr, state), ret);
            diag_util_mprintf("PON SID %u State %s \n", *sid_ptr, diagStr_enable[state]);
        }
        break;
#endif

        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_ingress_pon_sid_sid_state_enable_disable */

/*
 * flowctrl get ingress pon sid <UINT:sid> state
 */
cparser_result_t
cparser_cmd_flowctrl_get_ingress_pon_sid_sid_state(
    cparser_context_t *context,
    uint32_t  *sid_ptr)
{
    int32 ret = RT_ERR_FAILED;
    rtk_enable_t state;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_RTL9601B)
        case RTL9601B_CHIP_ID:
        {
	    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_ponSidState_get(*sid_ptr, &state), ret);
	    if(ENABLED == state){
	    	diag_util_mprintf("PON SID %u State %s \n", *sid_ptr, diagStr_enable[state]);
	    } else {
	    	diag_util_mprintf("PON SID %u State %s \n", *sid_ptr, diagStr_enable[state]);
	    }
        }
        break;
#endif
#if defined(CONFIG_SDK_RTL9602C)
		case RTL9602C_CHIP_ID:
		{
			DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_ponSidState_get(*sid_ptr, &state), ret);
			if(ENABLED == state){
				diag_util_mprintf("PON SID %u State %s \n", *sid_ptr, diagStr_enable[state]);
			} else {
				diag_util_mprintf("PON SID %u State %s \n", *sid_ptr, diagStr_enable[state]);
			}
		}
		break;
#endif
#if defined(CONFIG_SDK_RTL9607B)
        case RTL9607B_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_ponSidState_get(*sid_ptr, &state), ret);
            if(ENABLED == state){
                diag_util_mprintf("PON SID %u State %s \n", *sid_ptr, diagStr_enable[state]);
            } else {
                diag_util_mprintf("PON SID %u State %s \n", *sid_ptr, diagStr_enable[state]);
            }
        }
        break;
#endif

        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_ingress_pon_sid_sid_state */

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
        switch(DIAG_UTIL_CHIP_TYPE)
        {
    #if defined(CONFIG_SDK_APOLLO)
            case APOLLO_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_debugCtrl_set(port, ENABLED, mask), ret);
            }
            break;
    #endif
    #if defined(CONFIG_SDK_APOLLOMP)
            case APOLLOMP_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_debugCtrl_set(port, ENABLED, mask), ret);
            }
            break;
    #endif
    #if defined(CONFIG_SDK_RTL9601B)
            case RTL9601B_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_debugCtrl_set(port, ENABLED, mask), ret);
            }
            break;
    #endif

	#if defined(CONFIG_SDK_RTL9602C)
            case RTL9602C_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_debugCtrl_set(port, ENABLED, mask), ret);
            }
            break;
	#endif

    #if defined(CONFIG_SDK_RTL9607B)
            case RTL9602C_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_debugCtrl_set(port, ENABLED, mask), ret);
            }
            break;
    #endif
            default:
                diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
                return CPARSER_NOT_OK;
            break;
        }
    }

    diag_util_lPortMask2str(buf, &portlist.portmask);
    diag_util_printf("Port:%s Egress Port Max Page Clear.\n",buf);

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

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_clrTotalPktCnt_set(ENABLED), ret);
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_clrTotalPktCnt_set(ENABLED), ret);
        }
        break;
#endif
#if defined(CONFIG_SDK_RTL9601B)
        case RTL9601B_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_clrTotalPktCnt_set(ENABLED), ret);
        }
        break;
#endif
#if defined(CONFIG_SDK_RTL9602C)
		case RTL9602C_CHIP_ID:
		{
			DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_clrTotalPktCnt_set(ENABLED), ret);
		}
		break;
#endif
#if defined(CONFIG_SDK_RTL9607B)
        case RTL9607B_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_clrTotalPktCnt_set(ENABLED), ret);
        }
        break;
#endif

        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }

    diag_util_printf("Set Total Packet Count Clear.\n");

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_total_pktcnt_clear */

/*
 * flowctrl set max-page-clear switch
 */
cparser_result_t
cparser_cmd_flowctrl_set_max_page_clear_switch(
    cparser_context_t *context)
{
    diag_portlist_t  portlist;
    diag_mask_t  queuelist;
    rtk_port_t port;
    int32 ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_RTL9601B)
        case RTL9601B_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_clrMaxUsedPageCnt_set(ENABLED), ret);
            DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_clrTotalPktCnt_set(ENABLED), ret);
            queuelist.mask.bits[0] = 0xFF;
            HAL_SCAN_ALL_PORT(port)
            {
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_debugCtrl_set(port, DISABLED, queuelist.mask), ret);
            }
        }
        break;
#endif
#if defined(CONFIG_SDK_RTL9602C)
		case RTL9602C_CHIP_ID:
		{
			DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_clrMaxUsedPageCnt_set(ENABLED), ret);
			DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_clrTotalPktCnt_set(ENABLED), ret);
			queuelist.mask.bits[0] = 0xFF;
			HAL_SCAN_ALL_PORT(port)
			{
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_debugCtrl_set(port, DISABLED, queuelist.mask), ret);
			}
		}
		break;
#endif
#if defined(CONFIG_SDK_RTL9607B)
            case RTL9607B_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_clrMaxUsedPageCnt_set(ENABLED), ret);
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_clrTotalPktCnt_set(ENABLED), ret);
                queuelist.mask.bits[0] = 0xFF;
                HAL_SCAN_ALL_PORT(port)
                {
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_debugCtrl_set(port, DISABLED, queuelist.mask), ret);
                }
            }
            break;
#endif

        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }


    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_max_page_clear_switch */




/*
 * flowctrl set max-page-clear used-page
 */
cparser_result_t
cparser_cmd_flowctrl_set_max_page_clear_used_page(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_clrMaxUsedPageCnt_set(ENABLED), ret);
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_clrMaxUsedPageCnt_set(ENABLED), ret);
        }
        break;
#endif
#if defined(CONFIG_SDK_RTL9601B)
        case RTL9601B_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_clrMaxUsedPageCnt_set(ENABLED), ret);
        }
        break;
#endif
#if defined(CONFIG_SDK_RTL9602C)
		case RTL9602C_CHIP_ID:
		{
			DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_clrMaxUsedPageCnt_set(ENABLED), ret);
		}
		break;
#endif
#if defined(CONFIG_SDK_RTL9607B)
        case RTL9607B_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_clrMaxUsedPageCnt_set(ENABLED), ret);
        }
        break;
#endif

        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }
    diag_util_printf("Maximum Used Paged Count is cleared\n");

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_max_page_clear_used_page */

/*
 * flowctrl set max-page-clear egress-queue port ( <PORT_LIST:ports> | all ) queue-id ( <MASK_LIST:qid> | all )
 */
cparser_result_t
cparser_cmd_flowctrl_set_max_page_clear_egress_queue_port_ports_all_queue_id_qid_all(
    cparser_context_t *context,
    char * *ports_ptr,
    char * *qid_ptr)
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
        switch(DIAG_UTIL_CHIP_TYPE)
        {
    #if defined(CONFIG_SDK_APOLLO)
            case APOLLO_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_debugCtrl_set(port, DISABLED, queuelist.mask), ret);
            }
            break;
    #endif
    #if defined(CONFIG_SDK_APOLLOMP)
            case APOLLOMP_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_debugCtrl_set(port, DISABLED, queuelist.mask), ret);
            }
            break;
    #endif
    #if defined(CONFIG_SDK_RTL9601B)
            case RTL9601B_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_debugCtrl_set(port, DISABLED, queuelist.mask), ret);
            }
            break;
    #endif

	#if defined(CONFIG_SDK_RTL9602C)
            case RTL9602C_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_debugCtrl_set(port, DISABLED, queuelist.mask), ret);
            }
            break;
	#endif

    #if defined(CONFIG_SDK_RTL9607B)
            case RTL9607B_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_debugCtrl_set(port, DISABLED, queuelist.mask), ret);
            }
            break;
    #endif
            default:
                diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
                return CPARSER_NOT_OK;
            break;
        }
    }
    diag_util_lPortMask2str(buf, &portlist.portmask);
    diag_util_mask2str(buf2, queuelist.mask.bits[0]);
    diag_util_printf("Port:%s Egress queue: %s Max Page Clear.\n",buf, buf2);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_max_page_clear_egress_queue_port_ports_all_queue_id_qid_all */

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
            switch(DIAG_UTIL_CHIP_TYPE)
            {
        #if defined(CONFIG_SDK_APOLLO)
                case APOLLO_CHIP_ID:
                {
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portUsedPageCnt_get(port, &current, &maximum), ret);
                }
                break;
        #endif
        #if defined(CONFIG_SDK_APOLLOMP)
                case APOLLOMP_CHIP_ID:
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portUsedPageCnt_get(port, &current, &maximum), ret);
                }
                break;
        #endif
        #if defined(CONFIG_SDK_RTL9601B)
                case RTL9601B_CHIP_ID:
                {
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_portUsedPageCnt_get(port, &current, &maximum), ret);
                }
                break;
        #endif

		#if defined(CONFIG_SDK_RTL9602C)
                case RTL9602C_CHIP_ID:
                {
                    DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_portUsedPageCnt_get(port, &current, &maximum), ret);
                }
                break;
		#endif

        #if defined(CONFIG_SDK_RTL9607B)
                case RTL9607B_CHIP_ID:
                {
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_portUsedPageCnt_get(port, &current, &maximum), ret);
                }
                break;
        #endif
                default:
                    diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
                    return CPARSER_NOT_OK;
                break;
            }
            diag_util_mprintf("    %-18u %-18u%u\n", port, current, maximum);
        }
    }
    else if(!osal_strcmp(TOKEN_STR(3),"egress"))
    {
        diag_util_mprintf("egress-port     current-page      maximum-page\n");
        diag_util_mprintf("-----------------------------------------------------\n");
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            switch(DIAG_UTIL_CHIP_TYPE)
            {
        #if defined(CONFIG_SDK_APOLLO)
                case APOLLO_CHIP_ID:
                {
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portEgressUsedPageCnt_get(port, &current, &maximum), ret);
                }
                break;
        #endif
        #if defined(CONFIG_SDK_APOLLOMP)
                case APOLLOMP_CHIP_ID:
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portEgressUsedPageCnt_get(port, &current, &maximum), ret);
                }
                break;
        #endif
        #if defined(CONFIG_SDK_RTL9601B)
                case RTL9601B_CHIP_ID:
                {
                    DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_portEgressUsedPageCnt_get(port, &current, &maximum), ret);
                }
                break;
        #endif

		#if defined(CONFIG_SDK_RTL9602C)
                case RTL9602C_CHIP_ID:
                {
                    DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_portEgressUsedPageCnt_get(port, &current, &maximum), ret);
                }
                break;
		#endif

        #if defined(CONFIG_SDK_RTL9607B)
                case RTL9607B_CHIP_ID:
                {
                    DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_portEgressUsedPageCnt_get(port, &current, &maximum), ret);
                }
                break;
        #endif
                default:
                    diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
                    return CPARSER_NOT_OK;
                break;
            }
            diag_util_mprintf("    %-18u %-18u%u\n", port, current, maximum);
        }
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_used_page_cnt_ingress_egress_port_ports_all */

/*
 * flowctrl get used-page-cnt egress-queue port ( <PORT_LIST:ports> | all ) queue-id ( <MASK_LIST:qid> | all )
 */
cparser_result_t
cparser_cmd_flowctrl_get_used_page_cnt_egress_queue_port_ports_all_queue_id_qid_all(
    cparser_context_t *context,
    char * *ports_ptr,
    char * *qid_ptr)
{
    diag_portlist_t portlist;
    diag_mask_t  queuelist;
    rtk_port_t port;
    rtk_qid_t queue;
    uint32 current, maximum[8];
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

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
            switch(DIAG_UTIL_CHIP_TYPE)
            {
        #if defined(CONFIG_SDK_APOLLO)
                case APOLLO_CHIP_ID:
                {
                    DIAG_UTIL_ERR_CHK( apollo_raw_flowctrl_queueEgressUsedPageCnt_get(port, queue, &current, &maximum[queue]), ret);
                }
                break;
        #endif
        #if defined(CONFIG_SDK_APOLLOMP)
                case APOLLOMP_CHIP_ID:
                {
                    DIAG_UTIL_ERR_CHK( apollomp_raw_flowctrl_queueEgressUsedPageCnt_get(port, queue, &current, &maximum[queue]), ret);
                }
                break;
        #endif
        #if defined(CONFIG_SDK_RTL9601B)
                case RTL9601B_CHIP_ID:
                {
                    DIAG_UTIL_ERR_CHK( rtl9601b_raw_flowctrl_queueEgressUsedPageCnt_get(port, queue, &current, &maximum[queue]), ret);
                }
                break;
        #endif

		#if defined(CONFIG_SDK_RTL9602C)
                case RTL9602C_CHIP_ID:
                {
                    DIAG_UTIL_ERR_CHK( rtl9602c_raw_flowctrl_queueEgressUsedPageCnt_get(port, queue, &current, &maximum[queue]), ret);
                }
                break;
		#endif

        #if defined(CONFIG_SDK_RTL9607B)
                case RTL9607B_CHIP_ID:
                {
                    DIAG_UTIL_ERR_CHK( rtl9607b_raw_flowctrl_queueEgressUsedPageCnt_get(port, queue, &current, &maximum[queue]), ret);
                }
                break;
        #endif
                default:
                    diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
                    return CPARSER_NOT_OK;
                break;
            }
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
}    /* end of cparser_cmd_flowctrl_get_used_page_cnt_egress_queue_port_ports_all_queue_id_qid_all */

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
    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_totalPageCnt_get(&count), ret);
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_totalPageCnt_get(&count), ret);
        }
        break;
#endif
#if defined(CONFIG_SDK_RTL9601B)
        case RTL9601B_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_totalPageCnt_get(&count), ret);
        }
        break;
#endif
#if defined(CONFIG_SDK_RTL9602C)
		case RTL9602C_CHIP_ID:
		{
			DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_totalPageCnt_get(&count), ret);
		}
		break;
#endif
#if defined(CONFIG_SDK_RTL9607B)
        case RTL9607B_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_totalPageCnt_get(&count), ret);
        }
        break;
#endif

        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }
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

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
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
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            if(!osal_strcmp(TOKEN_STR(3),"total"))
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_totalUsedPageCnt_get(&current, &maximum), ret);
                diag_util_mprintf("Total page Count: Current  %u maximum  %u\n", current, maximum);
            }
            else if(!osal_strcmp(TOKEN_STR(3),"public"))
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_publicUsedPageCnt_get(&current, &maximum), ret);
                diag_util_mprintf("Public page Count: Current  %u maximum  %u\n", current, maximum);
            }
            else if(!osal_strcmp(TOKEN_STR(3),"public-off"))
            {
                diag_util_mprintf("-----------------------------------------------------\n");
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_publicFcoffUsedPageCnt_get(&current, &maximum), ret);
                diag_util_mprintf("Public page Count for FC-off: Current  %u maximum  %u\n", current, maximum);
            }
            else if(!osal_strcmp(TOKEN_STR(3),"public-jumbo"))
            {
                diag_util_mprintf("-----------------------------------------------------\n");
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_publicJumboUsedPageCnt_get(&current, &maximum), ret);
                diag_util_mprintf("Public page Count for jumbo frame: Current  %u maximum  %u\n", current, maximum);
            }
        }
        break;
#endif
#if defined(CONFIG_SDK_RTL9601B)
        case RTL9601B_CHIP_ID:
        {
            if(!osal_strcmp(TOKEN_STR(3),"total"))
            {
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_totalUsedPageCnt_get(&current, &maximum), ret);
                diag_util_mprintf("Total page Count: Current  %u maximum  %u\n", current, maximum);
            }
            else if(!osal_strcmp(TOKEN_STR(3),"public"))
            {
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_publicUsedPageCnt_get(&current, &maximum), ret);
                diag_util_mprintf("Public page Count: Current  %u maximum  %u\n", current, maximum);
            }
            else if(!osal_strcmp(TOKEN_STR(3),"public-off"))
            {
                diag_util_mprintf("-----------------------------------------------------\n");
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_publicFcoffUsedPageCnt_get(&current, &maximum), ret);
                diag_util_mprintf("Public page Count for FC-off: Current  %u maximum  %u\n", current, maximum);
            }
            else
                diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
        }
        break;
#endif
#if defined(CONFIG_SDK_RTL9602C)
		case RTL9602C_CHIP_ID:
		{
			if(!osal_strcmp(TOKEN_STR(3),"total"))
			{
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_totalUsedPageCnt_get(&current, &maximum), ret);
				diag_util_mprintf("Total page Count: Current  %u maximum  %u\n", current, maximum);
			}
			else if(!osal_strcmp(TOKEN_STR(3),"public"))
			{
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_publicUsedPageCnt_get(&current, &maximum), ret);
				diag_util_mprintf("Public page Count: Current  %u maximum  %u\n", current, maximum);
			}
			else if(!osal_strcmp(TOKEN_STR(3),"public-off"))
			{
				diag_util_mprintf("-----------------------------------------------------\n");
				DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_publicFcoffUsedPageCnt_get(&current, &maximum), ret);
				diag_util_mprintf("Public page Count for FC-off: Current  %u maximum  %u\n", current, maximum);
			}
			else
				diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
		}
		break;
#endif
#if defined(CONFIG_SDK_RTL9607B)
        case RTL9607B_CHIP_ID:
        {
            if(!osal_strcmp(TOKEN_STR(3),"total"))
            {
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_totalUsedPageCnt_get(&current, &maximum), ret);
                diag_util_mprintf("Total page Count: Current  %u maximum  %u\n", current, maximum);
            }
            else if(!osal_strcmp(TOKEN_STR(3),"public"))
            {
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_publicUsedPageCnt_get(&current, &maximum), ret);
                diag_util_mprintf("Public page Count: Current  %u maximum  %u\n", current, maximum);
            }
            else if(!osal_strcmp(TOKEN_STR(3),"public-off"))
            {
                diag_util_mprintf("-----------------------------------------------------\n");
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_publicFcoffUsedPageCnt_get(&current, &maximum), ret);
                diag_util_mprintf("Public page Count for FC-off: Current  %u maximum  %u\n", current, maximum);
            }
            else
                diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
        }
        break;
#endif

        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
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
        switch(DIAG_UTIL_CHIP_TYPE)
        {
    #if defined(CONFIG_SDK_APOLLO)
            case APOLLO_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portPacketPageCnt_get(port, &count), ret);
            }
            break;
    #endif
    #if defined(CONFIG_SDK_APOLLOMP)
            case APOLLOMP_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portPacketPageCnt_get(port, &count), ret);
            }
            break;
    #endif
    #if defined(CONFIG_SDK_RTL9601B)
            case RTL9601B_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_portPacketPageCnt_get(port, &count), ret);
            }
            break;
    #endif

            default:
                diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
                return CPARSER_NOT_OK;
            break;
        }
        diag_util_mprintf("    %-18u %u\n", port, count);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_used_page_cnt_packet_ports_all */

/*
 * flowctrl get used-page-cnt pon queue-id ( <MASK_LIST:qid> | all )
 */
cparser_result_t
cparser_cmd_flowctrl_get_used_page_cnt_pon_queue_id_qid_all(
    cparser_context_t *context,
    char * *qid_ptr)
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
        switch(DIAG_UTIL_CHIP_TYPE)
        {
    #if defined(CONFIG_SDK_APOLLO)
            case APOLLO_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponUsedPage_get(queue, &current, &maximum), ret);
            }
            break;
    #endif
    #if defined(CONFIG_SDK_APOLLOMP)
            case APOLLOMP_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponUsedPage_get(queue, &current, &maximum), ret);
            }
            break;
    #endif
    #if defined(CONFIG_SDK_RTL9601B)
            case RTL9601B_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_ponUsedPage_get(queue, &current, &maximum), ret);
            }
            break;
    #endif
	#if defined(CONFIG_SDK_RTL9602C)
            case RTL9602C_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_ponUsedPage_get(queue, &current, &maximum), ret);
            }
            break;
    #endif
    #if defined(CONFIG_SDK_RTL9607B)
            case RTL9607B_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_ponUsedPage_get(queue, &current, &maximum), ret);
            }
            break;
    #endif
            default:
                diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
                return CPARSER_NOT_OK;
            break;
        }

        diag_util_mprintf("%-18u %-18u%u\n", queue, current, maximum);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_used_page_cnt_pon_queue_id_qid_all */

/*
 * flowctrl set max-page-clear pon queue-id <UINT:qid>
 */
cparser_result_t
cparser_cmd_flowctrl_set_max_page_clear_pon_queue_id_qid(
    cparser_context_t *context,
    uint32_t  *qid_ptr)
{
    int32 ret = RT_ERR_FAILED;
    uint32 i;

    DIAG_UTIL_PARAM_CHK();
    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponUsedPageCtrl_set(*qid_ptr, ENABLED), ret);
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
#if 0 //This function has something wrong
            DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponUsedPageCtrl_set(ENABLED), ret);
#endif

            DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponLatchMaxUsedPage_set(DISABLED), ret);
            for(i=0;i<1000;i++);
            DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponLatchMaxUsedPage_set(ENABLED), ret);

        }
        break;
#endif
#if defined(CONFIG_SDK_RTL9601B)
        case RTL9601B_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_ponUsedPageCtrl_set(*qid_ptr, ENABLED), ret);
        }
        break;
#endif
#if defined(CONFIG_SDK_RTL9602C)
		case RTL9602C_CHIP_ID:
		{
			DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_ponUsedPageCtrl_set(*qid_ptr, ENABLED), ret);
		}
		break;
#endif
#if defined(CONFIG_SDK_RTL9607B)
        case RTL9607B_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_ponUsedPageCtrl_set(*qid_ptr, ENABLED), ret);
        }
        break;
#endif
        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }

    diag_util_printf("Maximum PON queue %u is cleared\n",*qid_ptr);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_max_page_clear_pon_queue_id_qid */

/*
 * flowctrl set prefetch threshold <UINT:threshold>
 */
cparser_result_t
cparser_cmd_flowctrl_set_prefetch_threshold_threshold(
    cparser_context_t *context,
    uint32_t  *threshold_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_txPrefet_set(*threshold_ptr), ret);
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_txPrefet_set(*threshold_ptr), ret);
        }
        break;
#endif
#if defined(CONFIG_SDK_RTl9601B)
        case RTL9601B_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_txPrefet_set(*threshold_ptr), ret);
        }
        break;
#endif
#if defined(CONFIG_SDK_RTL9602C)
		case RTL9602C_CHIP_ID:
		{
			DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_txPrefet_set(*threshold_ptr), ret);
		}
		break;
#endif
#if defined(CONFIG_SDK_RTL9607B)
        case RTL9607B_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_txPrefet_set(*threshold_ptr), ret);
        }
        break;
#endif

        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_prefetch_threshold_threshold */

/*
 * flowctrl get prefetch threshold
 */
cparser_result_t
cparser_cmd_flowctrl_get_prefetch_threshold(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    uint32 threshold;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_PARAM_CHK();

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_txPrefet_get(&threshold), ret);
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_txPrefet_get(&threshold), ret);
        }
        break;
#endif
#if defined(CONFIG_SDK_RTl9601B)
        case RTL9601B_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(rtl9601b_raw_flowctrl_txPrefet_get(&threshold), ret);
        }
        break;
#endif
#if defined(CONFIG_SDK_RTL9602C)
		case RTL9602C_CHIP_ID:
		{
			DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_txPrefet_get(&threshold), ret);
		}
		break;
#endif
#if defined(CONFIG_SDK_RTL9607B)
        case RTL9607B_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(rtl9607b_raw_flowctrl_txPrefet_get(&threshold), ret);
        }
        break;
#endif

        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }

    diag_util_mprintf("Prefet Threshold is %u\n", threshold);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_prefetch_threshold */

#if (!defined(CONFIG_REDUCED_DIAG))

/*
 * flowctrl set jumbo state ( enable | disable )
 */
cparser_result_t
cparser_cmd_flowctrl_set_jumbo_state_enable_disable(
    cparser_context_t *context)
{
    rtk_enable_t enable;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    if('e'==TOKEN_CHAR(4,0))
        enable = ENABLED;
    else if('d'==TOKEN_CHAR(4,0))
        enable = DISABLED;

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboMode_set(enable), ret);
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_jumboMode_set(enable), ret);
        }
        break;
#endif
        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }
    diag_util_printf("Set Flow Control Jumbo Mode to %s\n",diagStr_enable[enable]);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_jumbo_state_enable_disable */

/*
 * flowctrl get jumbo state
 */
cparser_result_t
cparser_cmd_flowctrl_get_jumbo_state(
    cparser_context_t *context)
{
    rtk_enable_t enable;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboMode_get(&enable), ret);
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_jumboMode_get(&enable), ret);
        }
        break;
#endif
        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }

    diag_util_mprintf("Flow Control Jumbo Mode is %s\n",diagStr_enable[enable]);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_jumbo_state */

/*
 * flowctrl set jumbo size ( 3k | 4k | 6k | max )
 */
cparser_result_t
cparser_cmd_flowctrl_set_jumbo_size_3k_4k_6k_max(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
            apollo_raw_flowctrl_jumbo_size_t size;
            if('3'==TOKEN_CHAR(4,0))
                size = RAW_FLOWCTRL_JUMBO_3K;
            else if('4'==TOKEN_CHAR(4,0))
                size = RAW_FLOWCTRL_JUMBO_4K;
            else if('6'==TOKEN_CHAR(4,0))
                size = RAW_FLOWCTRL_JUMBO_6K;
            else if('m'==TOKEN_CHAR(4,0))
                size = RAW_FLOWCTRL_JUMBO_MAX;
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboSize_set(size), ret);
            diag_util_printf("Set Flow Control Jumbo size to %s\n",diagStr_flowCtrlJumboSize[size]);
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            apollomp_raw_flowctrl_jumbo_size_t size;
            if('3'==TOKEN_CHAR(4,0))
                size = RAW_FLOWCTRL_JUMBO_3K;
            else if('4'==TOKEN_CHAR(4,0))
                size = RAW_FLOWCTRL_JUMBO_4K;
            else if('6'==TOKEN_CHAR(4,0))
                size = RAW_FLOWCTRL_JUMBO_6K;
            else if('m'==TOKEN_CHAR(4,0))
                size = RAW_FLOWCTRL_JUMBO_MAX;
            DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_jumboSize_set(size), ret);
            diag_util_printf("Set Flow Control Jumbo size to %s\n",diagStr_flowCtrlJumboSize[size]);
        }
        break;
#endif
        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_jumbo_size_3k_4k_6k_max */

/*
 * flowctrl get jumbo size
 */
cparser_result_t
cparser_cmd_flowctrl_get_jumbo_size(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
            apollo_raw_flowctrl_jumbo_size_t size;
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboSize_get(&size), ret);
            diag_util_mprintf("Flow Control Jumbo size is %s\n",diagStr_flowCtrlJumboSize[size]);
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            apollomp_raw_flowctrl_jumbo_size_t size;
            DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_jumboSize_get(&size), ret);
            diag_util_mprintf("Flow Control Jumbo size is %s\n",diagStr_flowCtrlJumboSize[size]);
        }
        break;
#endif
        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_jumbo_size */


/*
 * flowctrl set ingress jumbo-global ( high-off | high-on | low-off | low-on ) threshold <UINT:threshold>
 */
cparser_result_t
cparser_cmd_flowctrl_set_ingress_jumbo_global_high_off_high_on_low_off_low_on_threshold_threshold(
    cparser_context_t *context,
    uint32_t  *threshold_ptr)
{
    uint32 onThresh, offThresh;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
            if(!osal_strcmp(TOKEN_STR(4),"high-off"))
            {
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboGlobalHighThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboGlobalHighThreshold_set(onThresh, *threshold_ptr), ret);
                diag_util_printf("Set Flow Control Jumbo Global High Off Threshold to %u\n",*threshold_ptr);
            }
            else if(!osal_strcmp(TOKEN_STR(4),"high-on"))
            {
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboGlobalHighThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboGlobalHighThreshold_set(*threshold_ptr, offThresh), ret);
                diag_util_printf("Set Flow Control Jumbo Global High On Threshold to %u\n",*threshold_ptr);
            }
            else if(!osal_strcmp(TOKEN_STR(4),"low-off"))
            {
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboGlobalLowThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboGlobalLowThreshold_set(onThresh, *threshold_ptr), ret);
                diag_util_printf("Set Flow Control Jumbo Global Low Off Threshold to %u\n",*threshold_ptr);
            }
            else if(!osal_strcmp(TOKEN_STR(4),"low-on"))
            {
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboGlobalLowThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboGlobalLowThreshold_set(*threshold_ptr, offThresh), ret);
                diag_util_printf("Set Flow Control Jumbo Global Low On Threshold to %u\n",*threshold_ptr);
            }
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            if(!osal_strcmp(TOKEN_STR(4),"high-off"))
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_jumboGlobalHighThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_jumboGlobalHighThreshold_set(onThresh, *threshold_ptr), ret);
                diag_util_printf("Set Flow Control Jumbo Global High Off Threshold to %u\n",*threshold_ptr);
            }
            else if(!osal_strcmp(TOKEN_STR(4),"high-on"))
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_jumboGlobalHighThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_jumboGlobalHighThreshold_set(*threshold_ptr, offThresh), ret);
                diag_util_printf("Set Flow Control Jumbo Global High On Threshold to %u\n",*threshold_ptr);
            }
            else if(!osal_strcmp(TOKEN_STR(4),"low-off"))
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_jumboGlobalLowThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_jumboGlobalLowThreshold_set(onThresh, *threshold_ptr), ret);
                diag_util_printf("Set Flow Control Jumbo Global Low Off Threshold to %u\n",*threshold_ptr);
            }
            else if(!osal_strcmp(TOKEN_STR(4),"low-on"))
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_jumboGlobalLowThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_jumboGlobalLowThreshold_set(*threshold_ptr, offThresh), ret);
                diag_util_printf("Set Flow Control Jumbo Global Low On Threshold to %u\n",*threshold_ptr);
            }

        }
        break;
#endif
        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_ingress_jumbo_global_high_off_high_on_low_off_low_on_threshold_threshold */

/*
 * flowctrl get ingress jumbo-global ( high-off | high-on | low-off | low-on ) threshold
 */
cparser_result_t
cparser_cmd_flowctrl_get_ingress_jumbo_global_high_off_high_on_low_off_low_on_threshold(
    cparser_context_t *context)
{
    uint32 onThresh, offThresh;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();


    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
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
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            if(!osal_strcmp(TOKEN_STR(4),"high-off"))
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_jumboGlobalHighThreshold_get(&onThresh, &offThresh), ret);
                diag_util_mprintf("Flow Control Jumbo Global High Off Threshold is %u\n",offThresh);
            }
            else if(!osal_strcmp(TOKEN_STR(4),"high-on"))
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_jumboGlobalHighThreshold_get(&onThresh, &offThresh), ret);
                diag_util_mprintf("Flow Control Jumbo Global High On Threshold is %u\n",onThresh);
            }
            else if(!osal_strcmp(TOKEN_STR(4),"low-off"))
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_jumboGlobalLowThreshold_get(&onThresh, &offThresh), ret);
                diag_util_mprintf("Flow Control Jumbo Global Low Off Threshold is %u\n",offThresh);
            }
            else if(!osal_strcmp(TOKEN_STR(4),"low-on"))
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_jumboGlobalLowThreshold_get(&onThresh, &offThresh), ret);
                diag_util_mprintf("Flow Control Jumbo Global Low On Threshold is %u\n",onThresh);
            }

        }
        break;
#endif
        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_ingress_jumbo_global_high_off_high_on_low_off_low_on_threshold */

/*
 * flowctrl set ingress jumbo-port ( high-off | high-on | low-off | low-on ) threshold <UINT:threshold>
 */
cparser_result_t
cparser_cmd_flowctrl_set_ingress_jumbo_port_high_off_high_on_low_off_low_on_threshold_threshold(
    cparser_context_t *context,
    uint32_t  *threshold_ptr)
{
    uint32 onThresh, offThresh;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
            if(!osal_strcmp(TOKEN_STR(4),"high-off"))
            {
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboPortHighThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboPortHighThreshold_set(onThresh, *threshold_ptr), ret);
                diag_util_printf("Set Flow Control Jumbo Port High Off Threshold to %u\n",*threshold_ptr);
            }
            else if(!osal_strcmp(TOKEN_STR(4),"high-on"))
            {
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboPortHighThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboPortHighThreshold_set(*threshold_ptr, offThresh), ret);
                diag_util_printf("Set Flow Control Jumbo Port High On Threshold to %u\n",*threshold_ptr);
            }
            else if(!osal_strcmp(TOKEN_STR(4),"low-off"))
            {
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboPortLowThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboPortLowThreshold_set(onThresh, *threshold_ptr), ret);
                diag_util_printf("Set Flow Control Jumbo Port Low Off Threshold to %u\n",*threshold_ptr);
            }
            else if(!osal_strcmp(TOKEN_STR(4),"low-on"))
            {
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboPortLowThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_jumboPortLowThreshold_set(*threshold_ptr, offThresh), ret);
                diag_util_printf("Set Flow Control Jumbo Port Low On Threshold to %u\n",*threshold_ptr);
            }
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            if(!osal_strcmp(TOKEN_STR(4),"high-off"))
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_jumboPortHighThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_jumboPortHighThreshold_set(onThresh, *threshold_ptr), ret);
                diag_util_printf("Set Flow Control Jumbo Port High Off Threshold to %u\n",*threshold_ptr);
            }
            else if(!osal_strcmp(TOKEN_STR(4),"high-on"))
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_jumboPortHighThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_jumboPortHighThreshold_set(*threshold_ptr, offThresh), ret);
                diag_util_printf("Set Flow Control Jumbo Port High On Threshold to %u\n",*threshold_ptr);
            }
            else if(!osal_strcmp(TOKEN_STR(4),"low-off"))
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_jumboPortLowThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_jumboPortLowThreshold_set(onThresh, *threshold_ptr), ret);
                diag_util_printf("Set Flow Control Jumbo Port Low Off Threshold to %u\n",*threshold_ptr);
            }
            else if(!osal_strcmp(TOKEN_STR(4),"low-on"))
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_jumboPortLowThreshold_get(&onThresh, &offThresh), ret);
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_jumboPortLowThreshold_set(*threshold_ptr, offThresh), ret);
                diag_util_printf("Set Flow Control Jumbo Port Low On Threshold to %u\n",*threshold_ptr);
            }
        }
        break;
#endif
        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_ingress_jumbo_port_high_off_high_on_low_off_low_on_threshold_threshold */

/*
 * flowctrl get ingress jumbo-port ( high-off | high-on | low-off | low-on ) threshold
 */
cparser_result_t
cparser_cmd_flowctrl_get_ingress_jumbo_port_high_off_high_on_low_off_low_on_threshold(
    cparser_context_t *context)
{
    uint32 onThresh, offThresh;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
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
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            if(!osal_strcmp(TOKEN_STR(4),"high-off"))
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_jumboPortHighThreshold_get(&onThresh, &offThresh), ret);
                diag_util_mprintf("Flow Control Jumbo Global High Off Threshold is %u\n",offThresh);
            }
            else if(!osal_strcmp(TOKEN_STR(4),"high-on"))
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_jumboPortHighThreshold_get(&onThresh, &offThresh), ret);
                diag_util_mprintf("Flow Control Jumbo Global High On Threshold is %u\n",onThresh);
            }
            else if(!osal_strcmp(TOKEN_STR(4),"low-off"))
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_jumboPortLowThreshold_get(&onThresh, &offThresh), ret);
                diag_util_mprintf("Flow Control Jumbo Global Low Off Threshold is %u\n",offThresh);
            }
            else if(!osal_strcmp(TOKEN_STR(4),"low-on"))
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_jumboPortLowThreshold_get(&onThresh, &offThresh), ret);
                diag_util_mprintf("Flow Control Jumbo Global Low On Threshold is %u\n",onThresh);
            }

        }
        break;
#endif
        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_ingress_jumbo_port_high_off_high_on_low_off_low_on_threshold */

/*
 * flowctrl set low-queue threshold <UINT:threshold>
 */
cparser_result_t
cparser_cmd_flowctrl_set_low_queue_threshold_threshold(
    cparser_context_t *context,
    uint32_t  *threshold_ptr)
{
    int32 ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();


    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_lowQueueThreshold_set(*threshold_ptr), ret);
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_lowQueueThreshold_set(*threshold_ptr), ret);
        }
        break;
#endif

        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }

	diag_util_mprintf("Set Low Queue Threshold to %u\n", *threshold_ptr);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_low_queue_threshold_threshold */

/*
 * flowctrl get low-queue threshold
 */
cparser_result_t
cparser_cmd_flowctrl_get_low_queue_threshold(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    uint32 threshold;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_lowQueueThreshold_get(&threshold), ret);
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_lowQueueThreshold_get(&threshold), ret);
        }
        break;
#endif
        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }
    diag_util_mprintf("Low Queue Threshold is %u\n", threshold);
    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_low_queue_threshold */

/*
 * flowctrl set egress system ( flowctrl-threshold | drop-threshold ) ( high-off | high-on | low-off | low-on ) threshold <UINT:threshold>
 */
cparser_result_t
cparser_cmd_flowctrl_set_egress_system_flowctrl_threshold_drop_threshold_high_off_high_on_low_off_low_on_threshold_threshold(
    cparser_context_t *context,
    uint32_t  *threshold_ptr)
{
    uint32 onThresh, offThresh;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
            if('d'==TOKEN_CHAR(4,0))
            {
                if(!osal_strcmp(TOKEN_STR(5),"high-off"))
                {
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffHighThreshold_set(onThresh, *threshold_ptr), ret);
                    diag_util_printf("Set Flow Control Global FC-Off High Off Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
                {
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffHighThreshold_set(*threshold_ptr, offThresh), ret);
                    diag_util_printf("Set Flow Control Global FC-Off High On Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
                {
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffLowThreshold_set(onThresh, *threshold_ptr), ret);
                    diag_util_printf("Set Flow Control Global FC-Off Low Off Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
                {
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFcoffLowThreshold_set(*threshold_ptr, offThresh), ret);
                    diag_util_printf("Set Flow Control Global FC-Off Low On Threshold to %u\n",*threshold_ptr);
                }
            }
            else if('f'==TOKEN_CHAR(4,0))
            {
                if(!osal_strcmp(TOKEN_STR(5),"high-off"))
                {
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconHighThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconHighThreshold_set(onThresh, *threshold_ptr), ret);
                    diag_util_printf("Set Flow Control Global FC-On High Off Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
                {
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconHighThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconHighThreshold_set(*threshold_ptr, offThresh), ret);
                    diag_util_printf("Set Flow Control Global FC-On High On Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
                {
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconLowThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconLowThreshold_set(onThresh, *threshold_ptr), ret);
                    diag_util_printf("Set Flow Control Global FC-On Low Off Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
                {
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconLowThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_globalFconLowThreshold_set(*threshold_ptr, offThresh), ret);
                    diag_util_printf("Set Flow Control Global FC-On Low On Threshold to %u\n",*threshold_ptr);
                }
            }
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            if('d'==TOKEN_CHAR(4,0))
            {
                if(!osal_strcmp(TOKEN_STR(5),"high-off"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFcoffHighThreshold_set(onThresh, *threshold_ptr), ret);
                    diag_util_printf("Set Flow Control Global FC-Off High Off Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFcoffHighThreshold_set(*threshold_ptr, offThresh), ret);
                    diag_util_printf("Set Flow Control Global FC-Off High On Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFcoffLowThreshold_set(onThresh, *threshold_ptr), ret);
                    diag_util_printf("Set Flow Control Global FC-Off Low Off Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFcoffLowThreshold_set(*threshold_ptr, offThresh), ret);
                    diag_util_printf("Set Flow Control Global FC-Off Low On Threshold to %u\n",*threshold_ptr);
                }
            }
            else if('f'==TOKEN_CHAR(4,0))
            {
                if(!osal_strcmp(TOKEN_STR(5),"high-off"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFconHighThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFconHighThreshold_set(onThresh, *threshold_ptr), ret);
                    diag_util_printf("Set Flow Control Global FC-On High Off Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFconHighThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFconHighThreshold_set(*threshold_ptr, offThresh), ret);
                    diag_util_printf("Set Flow Control Global FC-On High On Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFconLowThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFconLowThreshold_set(onThresh, *threshold_ptr), ret);
                    diag_util_printf("Set Flow Control Global FC-On Low Off Threshold to %u\n",*threshold_ptr);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFconLowThreshold_get(&onThresh, &offThresh), ret);
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFconLowThreshold_set(*threshold_ptr, offThresh), ret);
                    diag_util_printf("Set Flow Control Global FC-On Low On Threshold to %u\n",*threshold_ptr);
                }
            }

        }
        break;
#endif
        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_egress_system_flowctrl_threshold_drop_threshold_high_off_high_on_low_off_low_on_threshold_threshold */

/*
 * flowctrl get egress system ( flowctrl-threshold | drop-threshold ) ( high-off | high-on | low-off | low-on ) threshold
 */
cparser_result_t
cparser_cmd_flowctrl_get_egress_system_flowctrl_threshold_drop_threshold_high_off_high_on_low_off_low_on_threshold(
    cparser_context_t *context)
{
    uint32 onThresh, offThresh;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
            if('d'==TOKEN_CHAR(4,0))
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
            else if('f'==TOKEN_CHAR(4,0))
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
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            if('d'==TOKEN_CHAR(4,0))
            {
                if(!osal_strcmp(TOKEN_STR(5),"high-off"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_mprintf("Flow Control Global FC-Off High Off Threshold is %u\n",offThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFcoffHighThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_mprintf("Flow Control Global FC-Off High On Threshold is %u\n",onThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_mprintf("Flow Control Global FC-Off Low Off Threshold is %u\n",offThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFcoffLowThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_mprintf("Flow Control Global FC-Off Low On Threshold is %u\n",onThresh);
                }
            }
            else if('f'==TOKEN_CHAR(4,0))
            {
                if(!osal_strcmp(TOKEN_STR(5),"high-off"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFconHighThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_mprintf("Flow Control Global FC-On High Off Threshold is %u\n",offThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"high-on"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFconHighThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_mprintf("Flow Control Global FC-On High On Threshold is %u\n",onThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-off"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFconLowThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_mprintf("Flow Control Global FC-On Low Off Threshold is %u\n",offThresh);
                }
                else if(!osal_strcmp(TOKEN_STR(5),"low-on"))
                {
                    DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_globalFconLowThreshold_get(&onThresh, &offThresh), ret);
                    diag_util_mprintf("Flow Control Global FC-On Low On Threshold is %u\n",onThresh);
                }
            }

        }
        break;
#endif
        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_egress_system_flowctrl_threshold_drop_threshold_high_off_high_on_low_off_low_on_threshold */

/*
 * flowctrl set egress queue-id ( <MASK_LIST:qid> | all ) threshold <UINT:threshold>
 */
cparser_result_t
cparser_cmd_flowctrl_set_egress_queue_id_qid_all_threshold_threshold(
    cparser_context_t *context,
    char * *qid_ptr,
    uint32_t  *threshold_ptr)
{
    diag_mask_t  queuelist;
    rtk_qid_t queue;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_QUEUEMASK(queuelist, 4), ret);

    diag_util_printf("Set Egress Flow Control Queue  Threshold \n");
    diag_util_printf("Egress-Queue       Drop-Threshold \n");
    diag_util_printf("---------------------------------\n");
    DIAG_UTIL_MASK_SCAN(queuelist, queue)
    {
        switch(DIAG_UTIL_CHIP_TYPE)
        {
    #if defined(CONFIG_SDK_APOLLO)
            case APOLLO_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_queueEegressDropThreshold_set(queue, *threshold_ptr), ret);
            }
            break;
    #endif
    #if defined(CONFIG_SDK_APOLLOMP)
            case APOLLOMP_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_queueEegressDropThreshold_set(queue, *threshold_ptr), ret);
            }
            break;
    #endif
            default:
                diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
                return CPARSER_NOT_OK;
            break;
        }
        diag_util_printf("%-18u  %u\n", queue, *threshold_ptr);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_egress_queue_id_qid_all_threshold_threshold */

/*
 * flowctrl get egress queue-id ( <MASK_LIST:qid> | all ) threshold
 */
cparser_result_t
cparser_cmd_flowctrl_get_egress_queue_id_qid_all_threshold(
    cparser_context_t *context,
    char * *qid_ptr)
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
        switch(DIAG_UTIL_CHIP_TYPE)
        {
    #if defined(CONFIG_SDK_APOLLO)
            case APOLLO_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_queueEegressDropThreshold_get(queue, &threshold), ret);
            }
            break;
    #endif
    #if defined(CONFIG_SDK_APOLLOMP)
            case APOLLOMP_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_queueEegressDropThreshold_get(queue, &threshold), ret);
            }
            break;
    #endif
            default:
                diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
                return CPARSER_NOT_OK;
            break;
        }
        diag_util_mprintf("%-18u  %u\n", queue, threshold);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_egress_queue_id_qid_all_threshold */

/*
 * flowctrl set egress port ( <PORT_LIST:ports> | all ) threshold <UINT:threshold>
 */
cparser_result_t
cparser_cmd_flowctrl_set_egress_port_ports_all_threshold_threshold(
    cparser_context_t *context,
    char * *ports_ptr,
    uint32_t  *threshold_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    diag_util_printf("Set Egress  Flow Control Port Threshold \n");
    diag_util_printf("Egress-Port       Drop-Threshold \n");
    diag_util_printf("-----------------------------\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
         switch(DIAG_UTIL_CHIP_TYPE)
        {
    #if defined(CONFIG_SDK_APOLLO)
            case APOLLO_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portEegressDropThreshold_set(port, *threshold_ptr), ret);
            }
            break;
    #endif
    #if defined(CONFIG_SDK_APOLLOMP)
            case APOLLOMP_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portEegressDropThreshold_set(port, *threshold_ptr), ret);
            }
            break;
    #endif
            default:
                diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
                return CPARSER_NOT_OK;
            break;
        }
        diag_util_printf("%-18u  %u\n", port, *threshold_ptr);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_egress_port_ports_all_threshold_threshold */

/*
 * flowctrl get egress port ( <PORT_LIST:ports> | all ) threshold
 */
cparser_result_t
cparser_cmd_flowctrl_get_egress_port_ports_all_threshold(
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
        switch(DIAG_UTIL_CHIP_TYPE)
        {
    #if defined(CONFIG_SDK_APOLLO)
            case APOLLO_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portEegressDropThreshold_get(port, &threshold), ret);
            }
            break;
    #endif
    #if defined(CONFIG_SDK_APOLLOMP)
            case APOLLOMP_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portEegressDropThreshold_get(port, &threshold), ret);
            }
            break;
    #endif
            default:
                diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
                return CPARSER_NOT_OK;
            break;
        }
        diag_util_mprintf("%-18u  %u\n", port, threshold);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_egress_port_ports_all_threshold */

/*
 * flowctrl set egress port ( <PORT_LIST:ports> | all ) queue-id ( <MASK_LIST:qid> | all ) queue-drop state ( enable | disable )
 */
cparser_result_t
cparser_cmd_flowctrl_set_egress_port_ports_all_queue_id_qid_all_queue_drop_state_enable_disable(
    cparser_context_t *context,
    char * *ports_ptr,
    char * *qid_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {

        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {

        }
        break;
#endif
        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_egress_port_ports_all_queue_id_qid_all_queue_drop_state_enable_disable */

/*
 * flowctrl get egress port ( <PORT_LIST:ports> | all ) queue-id ( <MASK_LIST:qid> | all ) queue-drop state
 */
cparser_result_t
cparser_cmd_flowctrl_get_egress_port_ports_all_queue_id_qid_all_queue_drop_state(
    cparser_context_t *context,
    char * *ports_ptr,
    char * *qid_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("");
    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {

        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {

        }
        break;
#endif
        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_egress_port_ports_all_queue_id_qid_all_queue_drop_state */

/*
 * flowctrl set egress port-gap-threshold <UINT:threshold>
 */
cparser_result_t
cparser_cmd_flowctrl_set_egress_port_gap_threshold_threshold(
    cparser_context_t *context,
    uint32_t  *threshold_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portEegressGapThreshold_set(*threshold_ptr), ret);
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portEegressGapThreshold_set(*threshold_ptr), ret);
        }
        break;
#endif
        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }

    diag_util_mprintf("Set Egress Flow Control Port Gap Threshold to %u \n", *threshold_ptr);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_egress_port_gap_threshold_threshold */

/*
 * flowctrl get egress port-gap-threshold
 */
cparser_result_t
cparser_cmd_flowctrl_get_egress_port_gap_threshold(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    uint32 threshold;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_portEegressGapThreshold_get(&threshold), ret);
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_portEegressGapThreshold_get(&threshold), ret);
        }
        break;
#endif
        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }

    diag_util_mprintf("Egress Flow Control Port  Gap Threshold is %u \n", threshold);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_egress_port_gap_threshold */

/*
 * flowctrl set egress queue-gap-threshold <UINT:threshold>
 */
cparser_result_t
cparser_cmd_flowctrl_set_egress_queue_gap_threshold_threshold(
    cparser_context_t *context,
    uint32_t  *threshold_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_queueEegressGapThreshold_set(*threshold_ptr), ret);
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_queueEegressGapThreshold_set(*threshold_ptr), ret);
        }
        break;
#endif
        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }

    diag_util_printf("Set Egress Flow Control Queue Gap Threshold to %u \n", *threshold_ptr);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_egress_queue_gap_threshold_threshold */

/*
 * flowctrl get egress queue-gap-threshold
 */
cparser_result_t
cparser_cmd_flowctrl_get_egress_queue_gap_threshold(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    uint32 threshold;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_queueEegressGapThreshold_get(&threshold), ret);
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_queueEegressGapThreshold_get(&threshold), ret);
        }
        break;
#endif
        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }

    diag_util_mprintf("Egress Flow Control Queue Gap Threshold is %u \n", threshold);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_egress_queue_gap_threshold */

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
        switch(DIAG_UTIL_CHIP_TYPE)
        {
    #if defined(CONFIG_SDK_APOLLO)
            case APOLLO_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_highQueueMask_set(port, queueMask.mask), ret);
            }
            break;
    #endif
    #if defined(CONFIG_SDK_APOLLOMP)
            case APOLLOMP_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_highQueueMask_set(port, queueMask.mask), ret);
            }
            break;
    #endif
            default:
                diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
                return CPARSER_NOT_OK;
            break;
        }
    }

    diag_util_lPortMask2str(buf1, &portlist.portmask);
    diag_util_mask2str (buf2, queueMask.mask.bits[0]);
    diag_util_printf("Set High Queue Mask: Port %s Queues:%s \n",buf1,buf2);

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
         switch(DIAG_UTIL_CHIP_TYPE)
        {
    #if defined(CONFIG_SDK_APOLLO)
            case APOLLO_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_highQueueMask_get(port, &queueMask), ret);
            }
            break;
    #endif
    #if defined(CONFIG_SDK_APOLLOMP)
            case APOLLOMP_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_highQueueMask_get(port, &queueMask), ret);
            }
            break;
    #endif
            default:
                diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
                return CPARSER_NOT_OK;
            break;
        }
        diag_util_mprintf("\tPort %2u : queue-mask = 0x%x\n", port, queueMask.bits[0]);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_high_queue_port_port_all */

/*
 * flowctrl set egress port ( <PORT_LIST:ports> | all ) queue-id ( <MASK_LIST:qid> | all ) queue-drop state ( enable | disable )
 */
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
    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_patch(type), ret);
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_patch(type), ret);
        }
        break;
#endif
        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_patch_gpon_35m_fiber_35m_20m */


/*
 * flowctrl set pon max-page-cnt latch ( enable | disable )
 */
cparser_result_t
cparser_cmd_flowctrl_set_pon_max_page_cnt_latch_enable_disable(
    cparser_context_t *context)
{
    rtk_enable_t enable;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    if('e'==TOKEN_CHAR(5,0))
        enable = ENABLED;
    else if('d'==TOKEN_CHAR(5,0))
        enable = DISABLED;

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {

        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponLatchMaxUsedPage_set(enable), ret);
        }
        break;
#endif
        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }
    diag_util_mprintf("Set PON queue maximum latch to %s\n",diagStr_enable[enable]);

    return CPARSER_OK;

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_pon_max_page_cnt_latch_enable_disable */


/*
 * flowctrl get pon max-page-cnt latch
 */
cparser_result_t
cparser_cmd_flowctrl_get_pon_max_page_cnt_latch(
    cparser_context_t *context)
{
    rtk_enable_t enable;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {

        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponLatchMaxUsedPage_get(&enable), ret);
        }
        break;
#endif
        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }

    diag_util_mprintf("Maximum PON queue latch is %s\n",diagStr_enable[enable]);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_pon_max_page_cnt_latch */

/*
 * flowctrl set ingress pon egress-drop queue-id ( <MASK_LIST:qid> | all ) threshold-index <UINT:index>
 */
cparser_result_t
cparser_cmd_flowctrl_set_ingress_pon_egress_drop_queue_id_qid_all_threshold_index_index(
    cparser_context_t *context,
    char * *qid_ptr,
    uint32_t  *index_ptr)
{
    diag_mask_t  queuelist;
    rtk_qid_t queue;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK128(queuelist, 6), ret);
    DIAG_UTIL_MASK_SCAN(queuelist, queue)
    {
        switch(DIAG_UTIL_CHIP_TYPE)
        {
    #if defined(CONFIG_SDK_APOLLO)
            case APOLLO_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponQueueEegressDropIndex_set(queue, *index_ptr), ret);
            }
            break;
    #endif
    #if defined(CONFIG_SDK_APOLLOMP)
            case APOLLOMP_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponQueueEegressDropIndex_set(queue, *index_ptr), ret);
            }
            break;
    #endif
            default:
                diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
                return CPARSER_NOT_OK;
            break;
        }
    }
    diag_util_printf("Queues:%s  Threshold index:%u\n",context->parser->tokens[(6)].buf, *index_ptr);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_ingress_pon_egress_drop_queue_id_qid_all_threshold_index_index */

/*
 * flowctrl get ingress pon egress-drop queue-id ( <MASK_LIST:qid> | all ) threshold-index
 */
cparser_result_t
cparser_cmd_flowctrl_get_ingress_pon_egress_drop_queue_id_qid_all_threshold_index(
    cparser_context_t *context,
    char * *qid_ptr)
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
        switch(DIAG_UTIL_CHIP_TYPE)
        {
    #if defined(CONFIG_SDK_APOLLO)
            case APOLLO_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponQueueEegressDropIndex_get(queue, &idx), ret);
            }
            break;
    #endif
    #if defined(CONFIG_SDK_APOLLOMP)
            case APOLLOMP_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponQueueEegressDropIndex_get(queue, &idx), ret);
            }
            break;
    #endif
            default:
                diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
                return CPARSER_NOT_OK;
            break;
        }
        diag_util_mprintf("%-18u  %u\n", queue, idx);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_ingress_pon_egress_drop_queue_id_qid_all_threshold_index */

/*
 * flowctrl set ingress pon egress-drop queue-threshold-index ( <MASK_LIST:index> | all ) threshold <UINT:threshold>
 */
cparser_result_t
cparser_cmd_flowctrl_set_ingress_pon_egress_drop_queue_threshold_index_index_all_threshold_threshold(
    cparser_context_t *context,
    char * *index_ptr,
    uint32_t  *threshold_ptr)
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
        switch(DIAG_UTIL_CHIP_TYPE)
        {
    #if defined(CONFIG_SDK_APOLLO)
            case APOLLO_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponQueueEegressDropThreshold_set(idx, *threshold_ptr), ret);
            }
            break;
    #endif
    #if defined(CONFIG_SDK_APOLLOMP)
            case APOLLOMP_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponQueueEegressDropThreshold_set(idx, *threshold_ptr), ret);
            }
            break;
    #endif
            default:
                diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
                return CPARSER_NOT_OK;
            break;
        }
    }
    diag_util_mask2str (buf, idxlist.mask.bits[0]);
    diag_util_printf("Threshold Index:%s Threshold:%u\n",buf, *threshold_ptr);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_ingress_pon_egress_drop_queue_threshold_index_index_all_threshold_threshold */

/*
 * flowctrl get ingress pon egress-drop queue-threshold-index ( <MASK_LIST:idx> | all ) threshold
 */
cparser_result_t
cparser_cmd_flowctrl_get_ingress_pon_egress_drop_queue_threshold_index_idx_all_threshold(
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
        switch(DIAG_UTIL_CHIP_TYPE)
        {
    #if defined(CONFIG_SDK_APOLLO)
            case APOLLO_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponQueueEegressDropThreshold_get(idx, &threshold), ret);
            }
            break;
    #endif
    #if defined(CONFIG_SDK_APOLLOMP)
            case APOLLOMP_CHIP_ID:
            {
                DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponQueueEegressDropThreshold_get(idx, &threshold), ret);
            }
            break;
    #endif
            default:
                diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
                return CPARSER_NOT_OK;
            break;
        }
        diag_util_mprintf("%-18u  %u\n", idx, threshold);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_ingress_pon_egress_drop_queue_threshold_index_idx_all_threshold */



/*
 * flowctrl set ingress pon egress-drop queue-gap threshold <UINT:threshold>
 */
cparser_result_t
cparser_cmd_flowctrl_set_ingress_pon_egress_drop_queue_gap_threshold_threshold(
    cparser_context_t *context,
    uint32_t  *threshold_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponQueueEegressGapThreshold_set(*threshold_ptr), ret);
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponQueueEegressGapThreshold_set(*threshold_ptr), ret);
        }
        break;
#endif
        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }

    diag_util_printf("Set PON MAC Queue Egress Gap Threshold to %u \n", *threshold_ptr);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_ingress_pon_egress_drop_queue_gap_threshold_threshold */

/*
 * flowctrl get ingress pon egress-drop queue-gap threshold
 */
cparser_result_t
cparser_cmd_flowctrl_get_ingress_pon_egress_drop_queue_gap_threshold(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    uint32 threshold;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_flowctrl_ponQueueEegressGapThreshold_get(&threshold), ret);
        }
        break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
        {
            DIAG_UTIL_ERR_CHK(apollomp_raw_flowctrl_ponQueueEegressGapThreshold_get(&threshold), ret);
        }
        break;
#endif
        default:
            diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
        break;
    }

    diag_util_mprintf("Ingress Flow Control PON MAC Egress Gap Threshold is %u \n", threshold);

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_ingress_pon_egress_drop_queue_gap_threshold */

/*
 * flowctrl set ingress pon egress-drop port ( <PORT_LIST:ports> | all ) queue-id ( <MASK_LIST:qid> | all ) drop ( enable | disable )
 */
cparser_result_t
cparser_cmd_flowctrl_set_ingress_pon_egress_drop_port_ports_all_queue_id_qid_all_drop_enable_disable(
    cparser_context_t *context,
    char * *ports_ptr,
    char * *qid_ptr)
{
    diag_portlist_t portlist;
    diag_mask_t  queuelist;
    rtk_port_t port;
    rtk_qid_t queue;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 6), ret);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_QUEUEMASK(queuelist, 8), ret);

    diag_util_printf("Set Ingress Flow Control Queue Egress Drop Enable for PON \n");
    diag_util_printf("            ");
    DIAG_UTIL_MASK_SCAN(queuelist, queue)
    {
        diag_util_printf("Q%d    ",queue);
    }
    diag_util_printf("\n");
    diag_util_printf("---------------------------------------------------------\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_printf("Port%2u  ", port);
        DIAG_UTIL_MASK_SCAN(queuelist, queue)
        {
            switch(DIAG_UTIL_CHIP_TYPE)
            {
		#if defined(CONFIG_SDK_RTL9602C)
                case RTL9602C_CHIP_ID:
                {
                    if('e'==TOKEN_CHAR(10,0))
                    {
                        DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_ponEgressDropEnable_set(port, queue, ENABLED), ret);
                        diag_util_printf("    En");
                    }
                    else if('d'==TOKEN_CHAR(10,0))
                    {
                        DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_ponEgressDropEnable_set(port, queue, DISABLED), ret);
                        diag_util_printf("   Dis");
                    }
                    else
                        return RT_ERR_FAILED;
                }
                break;
		#endif
                default:
                    diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
                    return CPARSER_NOT_OK;
                break;
            }
        }
        diag_util_printf("\n");
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_ingress_pon_egress_drop_port_ports_all_queue_id_qid_all_drop_enable_disable */

/*
 * flowctrl get ingress pon egress-drop port ( <PORT_LIST:ports> | all ) queue-id ( <MASK_LIST:qid> | all ) drop
 */
cparser_result_t
cparser_cmd_flowctrl_get_ingress_pon_egress_drop_port_ports_all_queue_id_qid_all_drop(
    cparser_context_t *context,
    char * *ports_ptr,
    char * *qid_ptr)
{
    diag_portlist_t portlist;
    diag_mask_t  queuelist;
    rtk_port_t port;
    rtk_qid_t queue;
    rtk_enable_t enable;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 6), ret);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_QUEUEMASK(queuelist, 8), ret);

    diag_util_mprintf("Get Ingress Flow Control Queue Egress Drop Enable for PON \n");
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
            switch(DIAG_UTIL_CHIP_TYPE)
            {

		#if defined(CONFIG_SDK_RTL9602C)
                case RTL9602C_CHIP_ID:
                {
                    DIAG_UTIL_ERR_CHK(rtl9602c_raw_flowctrl_ponEgressDropEnable_get(port, queue, &enable), ret);
                }
                break;
		#endif
                default:
                    diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
                    return CPARSER_NOT_OK;
                break;
            }
            diag_util_printf("   %3s",enable?"En":"Dis");
        }
        diag_util_mprintf("\n");
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_get_ingress_pon_egress_drop_port_ports_all_queue_id_qid_all_drop */
#endif

/*
 *flowctrl set egress-drop port ( <PORT_LIST:port> | all ) state ( enable | disable )
 */
cparser_result_t
cparser_cmd_flowctrl_set_egress_drop_port_port_all_state_enable_disable(
    cparser_context_t *context,
    char * *ports_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    rtk_enable_t state;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if('d'==TOKEN_CHAR(6,0))
        state = DISABLED;
    else if('e'==TOKEN_CHAR(6,0))
        state = ENABLED;

	DIAG_UTIL_PORTMASK_SCAN(portlist, port)
	{
		switch(DIAG_UTIL_CHIP_TYPE)
		{
#ifdef CONFIG_SDK_RTL9602C
			case RTL9602C_CHIP_ID:

				if ((ret = reg_array_field_write(RTL9602C_OUTPUT_DROP_ENr, port, REG_ARRAY_INDEX_NONE, RTL9602C_ENf,&state)) != RT_ERR_OK)
				{
					RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
					return ret;
				}
				break;
#endif
#ifdef CONFIG_SDK_RTL9607B
            case RTL9607B_CHIP_ID:

                if ((ret = reg_array_field_write(RTL9607B_OUTPUT_DROP_ENr, port, REG_ARRAY_INDEX_NONE, RTL9607B_ENf,&state)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
                    return ret;
                }
                break;
#endif

			default:
				diag_util_mprintf("%s\n",DIAG_STR_NOTSUPPORT);
				return CPARSER_NOT_OK;
				break;
		}
	}


    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_egress_drop_port_port_all_state_enable_disable */

/*
 * flowctrl set egress-drop ( broadcast | multicast | unknown-unicast ) ( select | not )
 */
cparser_result_t
cparser_cmd_flowctrl_set_egress_drop_broadcast_multicast_unknown_unicast_select_not(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    rtk_enable_t sel;
	uint32 bitField;
    DIAG_UTIL_PARAM_CHK();

    if ('n' == TOKEN_CHAR(4,0))
    {
        sel = DISABLED;
    }
    else if ('s' == TOKEN_CHAR(4,0))
    {
        sel = ENABLED;
    }

	switch(DIAG_UTIL_CHIP_TYPE)
	{
#ifdef CONFIG_SDK_RTL9602C
		case RTL9602C_CHIP_ID:

		    if ('b' == TOKEN_CHAR(3,0))
		    {
		        bitField = RTL9602C_OD_BC_SELf;
		    }
		    else if ('m' == TOKEN_CHAR(3,0))
		    {
		        bitField = RTL9602C_OD_MC_SELf;
		    }
		    else if ('u' == TOKEN_CHAR(3,0))
		    {
		        bitField = RTL9602C_OD_UC_SELf;
		    }

			if ((ret = reg_field_write(RTL9602C_OUTPUT_DROP_CFGr, bitField,&sel)) != RT_ERR_OK)
			{
				RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
				return ret;
			}
			break;
#endif
#ifdef CONFIG_SDK_RTL9607B
        case RTL9607B_CHIP_ID:

            if ('b' == TOKEN_CHAR(3,0))
            {
                bitField = RTL9607B_OD_BC_SELf;
            }
            else if ('m' == TOKEN_CHAR(3,0))
            {
                bitField = RTL9607B_OD_MC_SELf;
            }
            else if ('u' == TOKEN_CHAR(3,0))
            {
                bitField = RTL9607B_OD_UC_SELf;
            }

            if ((ret = reg_field_write(RTL9607B_OUTPUT_DROP_CFGr, bitField,&sel)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
                return ret;
            }
            break;
#endif

		default:
			diag_util_mprintf("%s\n",DIAG_STR_NOTSUPPORT);
			return CPARSER_NOT_OK;
			break;
	}


    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_set_egress_drop_broadcast_multicast_unknown_unicast_select_not */

/*
 * flowctrl dump egress-drop
 */
cparser_result_t
cparser_cmd_flowctrl_dump_egress_drop(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    uint32 state,bcSel,mcSel,ucSel;
    diag_portlist_t portlist;
    rtk_port_t port;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	switch(DIAG_UTIL_CHIP_TYPE)
	{
#if defined(CONFIG_SDK_RTL9602C)
		case RTL9602C_CHIP_ID:
		{
			diag_util_mprintf("Drop state:\n");

			portlist.max = HAL_GET_MAX_PORT();
			portlist.min = HAL_GET_MIN_PORT();
			HAL_GET_ALL_PORTMASK(portlist.portmask);

			DIAG_UTIL_PORTMASK_SCAN(portlist, port)
			{
				if ((ret = reg_array_field_read(RTL9602C_OUTPUT_DROP_ENr, port, REG_ARRAY_INDEX_NONE, RTL9602C_ENf, &state)) != RT_ERR_OK)
				{
					RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
					return ret;
				}

				diag_util_mprintf("Port %d: %s\n",port,state?"Enable":"Disable");
			}

			if ((ret = reg_field_read(RTL9602C_OUTPUT_DROP_CFGr, RTL9602C_OD_BC_SELf,&bcSel)) != RT_ERR_OK)
			{
				RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
				return ret;
			}

			if ((ret = reg_field_read(RTL9602C_OUTPUT_DROP_CFGr, RTL9602C_OD_MC_SELf,&mcSel)) != RT_ERR_OK)
			{
				RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
				return ret;
			}

			if ((ret = reg_field_read(RTL9602C_OUTPUT_DROP_CFGr, RTL9602C_OD_UC_SELf,&ucSel)) != RT_ERR_OK)
			{
				RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
				return ret;
			}

			diag_util_mprintf("Broadcast: %s, Multicast: %s, Unicast: %s\n",
								bcSel?"Select":"Not",
								mcSel?"Select":"Not",
								ucSel?"Select":"Not");
		}

		break;
#endif
#if defined(CONFIG_SDK_RTL9607B)
        case RTL9607B_CHIP_ID:
        {
            diag_util_mprintf("Drop state:\n");

            portlist.max = HAL_GET_MAX_PORT();
            portlist.min = HAL_GET_MIN_PORT();
            HAL_GET_ALL_PORTMASK(portlist.portmask);

            DIAG_UTIL_PORTMASK_SCAN(portlist, port)
            {
                if ((ret = reg_array_field_read(RTL9607B_OUTPUT_DROP_ENr, port, REG_ARRAY_INDEX_NONE, RTL9607B_ENf,&state)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
                    return ret;
                }

                diag_util_mprintf("Port %d: %s\n",port,state?"Enable":"Disable");
            }

            if ((ret = reg_field_read(RTL9607B_OUTPUT_DROP_CFGr, RTL9607B_OD_BC_SELf,&bcSel)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
                return ret;
            }

            if ((ret = reg_field_read(RTL9607B_OUTPUT_DROP_CFGr, RTL9607B_OD_MC_SELf,&mcSel)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
                return ret;
            }

            if ((ret = reg_field_read(RTL9607B_OUTPUT_DROP_CFGr, RTL9607B_OD_UC_SELf,&ucSel)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
                return ret;
            }

            diag_util_mprintf("Broadcast: %s, Multicast: %s, Unicast: %s\n",
                                bcSel?"Select":"Not",
                                mcSel?"Select":"Not",
                                ucSel?"Select":"Not");
        }

        break;
#endif

		default:
			diag_util_printf("%s\n", DIAG_STR_NOTSUPPORT);
			return CPARSER_NOT_OK;
		break;
	}


    return CPARSER_OK;
}    /* end of cparser_cmd_flowctrl_dump_egress_drop */



