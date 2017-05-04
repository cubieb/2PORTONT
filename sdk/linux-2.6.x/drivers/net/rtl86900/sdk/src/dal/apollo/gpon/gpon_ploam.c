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
 * $Revision: 57014 $
 * $Date: 2015-03-20 15:34:23 +0800 (Fri, 20 Mar 2015) $
 *
 * Purpose : GPON MAC Driver PLOAM Processor
 *
 * Feature : GPON MAC Driver PLOAM Processor
 *
 */
#include <dal/apollo/gpon/gpon_defs.h>
#include <dal/apollo/gpon/gpon_fsm.h>
#include <dal/apollo/gpon/gpon_res.h>
#include <dal/apollo/gpon/gpon_ploam.h>
#include <dal/apollo/raw/apollo_raw_gpon.h>

extern gpon_drv_obj_t *g_gponmac_drv;

static uint8 gReiTxCnt = 0;

typedef enum gpon_ploam_dsType_e{
    GPON_PLOAM_DS_OVERHEAD = 0x01,
    GPON_PLOAM_DS_SNMASK = 0x02,/* deprecated in G.984.3(2007)*/
    GPON_PLOAM_DS_ASSIGNONUID = 0x03,
    GPON_PLOAM_DS_RANGINGTIME = 0x04,
    GPON_PLOAM_DS_DEACTIVEONU = 0x05,
    GPON_PLOAM_DS_DISABLESN = 0x06,
    GPON_PLOAM_DS_CFG_VPVC = 0x07,
    GPON_PLOAM_DS_ENCRYPTPORT = 0x08,
    GPON_PLOAM_DS_REQUESTPASSWORD = 0x09,
    GPON_PLOAM_DS_ASSIGNEDALLOCID = 0x0A,
    GPON_PLOAM_DS_NOMESSAGE = 0x0B,
    GPON_PLOAM_DS_POPUP = 0x0C,
    GPON_PLOAM_DS_REQUESTKEY = 0x0D,
    GPON_PLOAM_DS_CONFIGPORT = 0x0E,
    GPON_PLOAM_DS_PEE = 0x0F,
    GPON_PLOAM_DS_POWERLEVEL = 0x10,
    GPON_PLOAM_DS_PST = 0x11,
    GPON_PLOAM_DS_BER_INTERVAL = 0x12,
    GPON_PLOAM_DS_SWITCHINGKEY = 0x13,
    GPON_PLOAM_DS_EXT_BURSTLENGTH = 0x14,
}gpon_ploam_dsType_t;

typedef enum gpon_ploam_usType_e{
    GPON_PLOAM_US_SN = 0x01,
    GPON_PLOAM_US_PASSWORD = 0x02,
    GPON_PLOAM_US_DYINGGASP = 0x03,
    GPON_PLOAM_US_NOMESSAGE = 0x04,
    GPON_PLOAM_US_ENCRYPTKEY = 0x05,
    GPON_PLOAM_US_PEE = 0x06,
    GPON_PLOAM_US_PST = 0x07,
    GPON_PLOAM_US_REI = 0x08,
    GPON_PLOAM_US_ACKNOWLEDGE = 0x09
}gpon_ploam_usType_t;

static int32 gpon_ploam_sn_equal(uint8* str, rtk_gpon_serialNumber_t *sn)
{
    uint8 i;

    for(i=0;i<RTK_GPON_VENDOR_ID_LEN;i++)
    {
        if(str[i]!=sn->vendor[i])
        {
            return FALSE;
        }
    }
    for(i=0;i<RTK_GPON_VENDOR_SPECIFIC_LEN;i++)
    {
        if(str[i+RTK_GPON_VENDOR_ID_LEN]!=sn->specific[i])
        {
            return FALSE;
        }
    }
    return TRUE;
}

static void gpon_ploam_ack_tx(gpon_dev_obj_t* obj, rtk_gpon_ploam_t* ploam)
{
    uint8 msg[3+RTK_GPON_PLOAM_MSG_LEN] = {0};
    msg[0] = obj->onuid;
    msg[1] = GPON_PLOAM_US_ACKNOWLEDGE;
    msg[2] = ploam->type;
    msg[3] = ploam->onuid;
    msg[4] = ploam->type;
    osal_memcpy(msg+5,ploam->msg,7);

    GPON_OS_Log(GPON_LOG_LEVEL_PLOAM,"Send PLOAM %02x (ONU %d) %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x [baseaddr:%p]",
        msg[1],msg[0],msg[2],msg[3],msg[4],msg[5],msg[6],msg[7],msg[8],msg[9],msg[10],msg[11],obj->base_addr);
    if(apollo_raw_gpon_usPloam_write(TRUE,msg))
    {
        GPON_OS_Log(GPON_LOG_LEVEL_WARNING,"Send PLOAM Error");
    }
    else
    {
        obj->cnt_cpu_ploam_tx++;
        obj->cnt_cpu_ploam_tx_nor++;
    }
}

static void gpon_ploam_pwd_tx(gpon_dev_obj_t* obj)
{
    uint8 msg[3+RTK_GPON_PLOAM_MSG_LEN] = {0};
    msg[0] = obj->onuid;
    msg[1] = GPON_PLOAM_US_PASSWORD;
    osal_memcpy(msg+2,obj->password.password,RTK_GPON_PASSWORD_LEN);

    GPON_OS_Log(GPON_LOG_LEVEL_PLOAM,"Send PLOAM %02x (ONU %d) %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x [baseaddr:%p]",
        msg[1],msg[0],msg[2],msg[3],msg[4],msg[5],msg[6],msg[7],msg[8],msg[9],msg[10],msg[11],obj->base_addr);
    if(apollo_raw_gpon_usPloam_write(TRUE,msg))
    {
        GPON_OS_Log(GPON_LOG_LEVEL_WARNING,"Send PLOAM Error");
    }
    else
    {
        obj->cnt_cpu_ploam_tx++;
        obj->cnt_cpu_ploam_tx_nor++;
    }

    if(apollo_raw_gpon_usPloam_write(TRUE,msg))
    {
        GPON_OS_Log(GPON_LOG_LEVEL_WARNING,"Send PLOAM Error");
    }
    else
    {
        obj->cnt_cpu_ploam_tx++;
        obj->cnt_cpu_ploam_tx_nor++;
    }

    if(apollo_raw_gpon_usPloam_write(TRUE,msg))
    {
        GPON_OS_Log(GPON_LOG_LEVEL_WARNING,"Send PLOAM Error");
    }
    else
    {
        obj->cnt_cpu_ploam_tx++;
        obj->cnt_cpu_ploam_tx_nor++;
    }
}

static void gpon_ploam_key_tx(gpon_dev_obj_t* obj)
{
    int i;
    uint8 msg[3+RTK_GPON_PLOAM_MSG_LEN] = {0};
    msg[0] = obj->onuid;
    msg[1] = GPON_PLOAM_US_ENCRYPTKEY;
    msg[2] = obj->key_index;

    if(obj->aeskey_callback)
    {
        (*obj->aeskey_callback)(&obj->aes_key);
    }
    else
    {
        for(i=0; i < RTK_GPON_AES_KEY_LEN; i++)
        {
            obj->aes_key.key[i] = obj->key_index;
        }
    }
    GPON_OS_Log(GPON_LOG_LEVEL_NORMAL,"Get New AES KEY[%d]: %02x %02x %02x %02x %02x %02x %02x %02x, %02x %02x %02x %02x %02x %02x %02x %02x [baseaddr:%p]",obj->key_index,
        obj->aes_key.key[0],obj->aes_key.key[1],obj->aes_key.key[2],obj->aes_key.key[3],obj->aes_key.key[4],obj->aes_key.key[5],obj->aes_key.key[6],obj->aes_key.key[7],
        obj->aes_key.key[8],obj->aes_key.key[9],obj->aes_key.key[10],obj->aes_key.key[11],obj->aes_key.key[12],obj->aes_key.key[13],obj->aes_key.key[14],obj->aes_key.key[15],obj->base_addr);
#if 1
    for(i=0; i<3; i++)
    {
        msg[3] = 0;
        osal_memcpy(msg+4,obj->aes_key.key,RTK_GPON_AES_KEY_LEN/2);

        GPON_OS_Log(GPON_LOG_LEVEL_PLOAM,"Send PLOAM %02x (ONU %d) %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x [baseaddr:%p]",
            msg[1],msg[0],msg[2],msg[3],msg[4],msg[5],msg[6],msg[7],msg[8],msg[9],msg[10],msg[11],obj->base_addr);

        if(apollo_raw_gpon_usPloam_write(TRUE,msg))
        {
            GPON_OS_Log(GPON_LOG_LEVEL_WARNING,"Send PLOAM Error");
        }
        else
        {
            obj->cnt_cpu_ploam_tx++;
            obj->cnt_cpu_ploam_tx_nor++;
        }
        msg[3] = 1;
        osal_memcpy(msg+4,obj->aes_key.key+RTK_GPON_AES_KEY_LEN/2,RTK_GPON_AES_KEY_LEN/2);

        GPON_OS_Log(GPON_LOG_LEVEL_PLOAM,"Send PLOAM %02x (ONU %d) %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x [baseaddr:%p]",
            msg[1],msg[0],msg[2],msg[3],msg[4],msg[5],msg[6],msg[7],msg[8],msg[9],msg[10],msg[11],obj->base_addr);

        if(apollo_raw_gpon_usPloam_write(TRUE,msg))
        {
            GPON_OS_Log(GPON_LOG_LEVEL_WARNING,"Send PLOAM Error");
        }
        else
        {
            obj->cnt_cpu_ploam_tx++;
            obj->cnt_cpu_ploam_tx_nor++;
        }
    }
#else
    msg[3] = 0;
    osal_memcpy(msg+4,obj->aes_key.key,RTK_GPON_AES_KEY_LEN/2);

    GPON_OS_Log(GPON_LOG_LEVEL_PLOAM,"Send PLOAM %02x (ONU %d) %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x [baseaddr:%p]",
        msg[1],msg[0],msg[2],msg[3],msg[4],msg[5],msg[6],msg[7],msg[8],msg[9],msg[10],msg[11],obj->base_addr);
    if(apollo_raw_gpon_usPloam_write(TRUE,msg))
    {
        GPON_OS_Log(GPON_LOG_LEVEL_WARNING,"Send PLOAM Error");
    }
    else
    {
        obj->cnt_cpu_ploam_tx++;
        obj->cnt_cpu_ploam_tx_nor++;
    }

    if(apollo_raw_gpon_usPloam_write(TRUE,msg))
    {
        GPON_OS_Log(GPON_LOG_LEVEL_WARNING,"Send PLOAM Error");
    }
    else
    {
        obj->cnt_cpu_ploam_tx++;
        obj->cnt_cpu_ploam_tx_nor++;
    }

    if(apollo_raw_gpon_usPloam_write(TRUE,msg))
    {
        GPON_OS_Log(GPON_LOG_LEVEL_WARNING,"Send PLOAM Error");
    }
    else
    {
        obj->cnt_cpu_ploam_tx++;
        obj->cnt_cpu_ploam_tx_nor++;
    }

    msg[3] = 1;
    osal_memcpy(msg+4,obj->aes_key.key+RTK_GPON_AES_KEY_LEN/2,RTK_GPON_AES_KEY_LEN/2);

    GPON_OS_Log(GPON_LOG_LEVEL_PLOAM,"Send PLOAM %02x (ONU %d) %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x [baseaddr:%p]",
        msg[1],msg[0],msg[2],msg[3],msg[4],msg[5],msg[6],msg[7],msg[8],msg[9],msg[10],msg[11],obj->base_addr);
    if(apollo_raw_gpon_usPloam_write(TRUE,msg))
    {
        GPON_OS_Log(GPON_LOG_LEVEL_WARNING,"Send PLOAM Error");
    }
    else
    {
        obj->cnt_cpu_ploam_tx++;
        obj->cnt_cpu_ploam_tx_nor++;
    }

    if(apollo_raw_gpon_usPloam_write(TRUE,msg))
    {
        GPON_OS_Log(GPON_LOG_LEVEL_WARNING,"Send PLOAM Error");
    }
    else
    {
        obj->cnt_cpu_ploam_tx++;
        obj->cnt_cpu_ploam_tx_nor++;
    }

    if(apollo_raw_gpon_usPloam_write(TRUE,msg))
    {
        GPON_OS_Log(GPON_LOG_LEVEL_WARNING,"Send PLOAM Error");
    }
    else
    {
        obj->cnt_cpu_ploam_tx++;
        obj->cnt_cpu_ploam_tx_nor++;
    }
#endif

    obj->key_index++;
    apollo_raw_gpon_aes_keyword_write(obj->aes_key.key);
}

static void gpon_berInterval_expire(uint32 data)
{
    uint32 ret;
    uint32 bipError;
    apollo_raw_gpon_dsGtc_pmMiscType_t index;
    rtk_gpon_ploam_t msg;
    uint32 interval;

    if(data);

    /* stop timer first */
    if(g_gponmac_drv->dev->ber_timer)
    {
        GPON_OS_StopTimer(g_gponmac_drv->dev->ber_timer);
        g_gponmac_drv->dev->ber_timer = 0;
    }

    /* get bip error counter */
    memset(&msg,0,sizeof(rtk_gpon_ploam_t));
    index = GPON_REG_DSGTCPMMISC_BIP_BLOCK_ERR;
    apollo_raw_gpon_dsGtc_miscCnt_read(index,&bipError);

 //   printk("get bip error blocks %d  TXCnt %d \r\n",bipError,gReiTxCnt);
    msg.msg[0] = (uint8)((bipError >> 24) & 0xFF);
    msg.msg[1] = (uint8)((bipError >> 16) & 0xFF);
    msg.msg[2] = (uint8)((bipError >> 8) & 0xFF);
    msg.msg[3] = (uint8)(bipError);
    msg.msg[4] = gReiTxCnt;
    msg.type = GPON_PLOAM_US_REI;
    msg.onuid = g_gponmac_drv->dev->onuid;

    gReiTxCnt++;
    if(gReiTxCnt > 0xF) gReiTxCnt = 0;
    /* send REI ploam*/
    ret = gpon_ploam_tx(g_gponmac_drv->dev,0,&msg);

    /* start timer again */
    gpon_dev_berInterval_get(g_gponmac_drv->dev, &interval);
    g_gponmac_drv->dev->ber_timer = GPON_OS_StartTimer(interval,FALSE,0,gpon_berInterval_expire);

}

static void gpon_ploam_rx_process(gpon_dev_obj_t* obj, rtk_gpon_ploam_t* ploam)
{
    uint32 alloc;
    uint32 framecnt;
    uint32 time;
    uint32 enable_port;
    uint32 gemport;
    uint32 aesEn;
    uint32 berInterval;
    uint32 ret;
    rtk_gpon_dsFlow_attr_t ds_attr;
    rtk_gpon_usFlow_attr_t us_attr;
    uint32 i;


    switch(ploam->type)
    {
        case GPON_PLOAM_DS_OVERHEAD:
        {
            if(obj->status==RTK_GPONMAC_FSM_STATE_O2)
            {
                if(ploam->onuid==GPON_DEV_DEFAULT_ONU_ID)
                {
                     /* ploam callback */
                    if(obj->ploam_callback)
                    {
                        (*obj->ploam_callback)(ploam);
                    }

                    /* configure overhead */
                    obj->burst_overhead.guard_bits = ploam->msg[0];
                    obj->burst_overhead.type1_bits = ploam->msg[1];
                    obj->burst_overhead.type2_bits = ploam->msg[2];
                    obj->burst_overhead.type3_ptn = ploam->msg[3];
                    obj->burst_overhead.type3_preranged = 0;
                    obj->burst_overhead.type3_ranged = 0;
                    obj->burst_overhead.delimiter[0] = ploam->msg[4];
                    obj->burst_overhead.delimiter[1] = ploam->msg[5];
                    obj->burst_overhead.delimiter[2] = ploam->msg[6];
                    gpon_dev_upstreamOverhead_calculate(obj);
                    gpon_dev_burstHead_preRanged_set(obj);

                    /* configure the extra sn */
                    GPON_OS_Log(GPON_LOG_LEVEL_NORMAL,"set extra SN %d [baseaddr:%p]",(ploam->msg[7]>>2)&0x03,obj->base_addr);
                    apollo_raw_gpon_extraSN_write((ploam->msg[7]>>2)&0x03);

                    /* configure pre_eqd */
                    if((ploam->msg[7]>>5)&0x01)
                    {
                        time = (((ploam->msg[8]<<8)|(ploam->msg[9]<<0))*32*8);
                        GPON_OS_Log(GPON_LOG_LEVEL_NORMAL,"set pre-eqd(%d) [baseaddr:%p]",time , obj->base_addr);
                        apollo_raw_gpon_eqd_write(time);
                        obj->pre_eqd = time;
                    }
                    else
                    {
                        GPON_OS_Log(GPON_LOG_LEVEL_NORMAL,"disable pre-eqd [baseaddr:%p]",obj->base_addr);
                        apollo_raw_gpon_eqd_write(0);
                        obj->pre_eqd = 0;
                    }

                    /* the sn mask is deprecated in G.984.3(2007) */

                    /* the power level mode should be processed by the applications */

                    /* notify FSM */
                    gpon_fsm_event(obj,GPON_FSM_EVENT_RX_UPSTREAM);
                }
                else
                    GPON_OS_Log(GPON_LOG_LEVEL_NORMAL,"B_OVERHEAD: onuid[%d]!=GPON_DEV_DEFAULT_ONU_ID \n\r", ploam->onuid);
            }
            else
                    GPON_OS_Log(GPON_LOG_LEVEL_NORMAL,"B_OVERHEAD: status[%d]!=RTK_GPONMAC_FSM_STATE_O2\n\r", obj->status);
            break;
        }
        case GPON_PLOAM_DS_SNMASK:
        {
            /*deprecated in G.984.3(2007)*/
            break;
        }
        case GPON_PLOAM_DS_ASSIGNONUID:
        {
            if(obj->status==RTK_GPONMAC_FSM_STATE_O3)
            {
                if(ploam->onuid==GPON_DEV_DEFAULT_ONU_ID)
                {
                    if(gpon_ploam_sn_equal(ploam->msg+1,&obj->serial_number))
                    {
                        /* ploam callback */
                        if(obj->ploam_callback)
                        {
                            (*obj->ploam_callback)(ploam);
                        }

                        /* configure ONU ID */
                        obj->onuid = ploam->msg[0];
                        gpon_dev_onuid_set(obj,obj->onuid);

                        /* notify FSM */
                        gpon_fsm_event(obj,GPON_FSM_EVENT_RX_ONUID);
                    }
                }
            }
            break;
        }
        case GPON_PLOAM_DS_RANGINGTIME:
        {
            if(obj->status==RTK_GPONMAC_FSM_STATE_O4 || obj->status==RTK_GPONMAC_FSM_STATE_O5)
            {
                if(ploam->onuid==obj->onuid)
                {
                     /* ploam callback */
                    if(obj->ploam_callback)
                    {
                        (*obj->ploam_callback)(ploam);
                    }

                    time = (ploam->msg[1]<<24)|(ploam->msg[2]<<16)|(ploam->msg[3]<<8)|(ploam->msg[4]<<0);

                    /* Only Main path EqD is accepted */
                    if((ploam->msg[0]&0x01)==0)
                    {
                        /* configure EqD */
                        GPON_OS_Log(GPON_LOG_LEVEL_NORMAL,"set main path eqd(%d) [baseaddr:%p]",time, obj->base_addr);
                        apollo_raw_gpon_eqd_write(time);

                        /* notify FSM */
                        gpon_fsm_event(obj,GPON_FSM_EVENT_RX_EQD);
                    }
                    else
                    {
                        GPON_OS_Log(GPON_LOG_LEVEL_INFO,"protect path eqd %d is not supported to configure [baseaddr:%p]",time,obj->base_addr);
                    }
                }
            }
            break;
        }
        case GPON_PLOAM_DS_DEACTIVEONU:
        {
            if(obj->status==RTK_GPONMAC_FSM_STATE_O4
                || obj->status==RTK_GPONMAC_FSM_STATE_O5
                || obj->status==RTK_GPONMAC_FSM_STATE_O6)
            {
                if(ploam->onuid==obj->onuid || ploam->onuid==GPON_DEV_DEFAULT_ONU_ID)
                {
                    /* ploam callback */
                    if(obj->ploam_callback)
                    {
                        (*obj->ploam_callback)(ploam);
                    }

                    /* notify FSM */
                    gpon_fsm_event(obj,GPON_FSM_EVENT_RX_DEACTIVATE);
                }
            }
            break;
        }
        case GPON_PLOAM_DS_DISABLESN:
        {
            if(obj->status!=RTK_GPONMAC_FSM_STATE_O1)
            {
                if(ploam->onuid==GPON_DEV_DEFAULT_ONU_ID)
                {
                     /* ploam callback */
                    if(obj->ploam_callback)
                    {
                        (*obj->ploam_callback)(ploam);
                    }

                    if(ploam->msg[0]==0x0F)
                    {
                        /* notify FSM */
                        if(obj->status==RTK_GPONMAC_FSM_STATE_O7)
                        {
                            gpon_fsm_event(obj,GPON_FSM_EVENT_RX_ENABLE);
                        }
                    }
                    else
                    {
                        if(gpon_ploam_sn_equal(ploam->msg+1,&obj->serial_number))
                        {
                            if(ploam->msg[0]==0xFF)
                            {
                                /* notify FSM */
                                gpon_fsm_event(obj,GPON_FSM_EVENT_RX_DISABLE);
                            }
                            else if(ploam->msg[0]==0x00)
                            {
                                /* notify FSM */
                                gpon_fsm_event(obj,GPON_FSM_EVENT_RX_ENABLE);
                            }
                        }
                    }
                }
            }
            break;
        }
        case GPON_PLOAM_DS_CFG_VPVC:
        {
            if(ploam->onuid==obj->onuid)
            {
                 /* ploam callback */
                if(obj->ploam_callback)
                {
                    (*obj->ploam_callback)(ploam);
                }

                /* this ploam is not supported since it configures a ATM connection for OMCI */

                gpon_ploam_ack_tx(obj,ploam);
            }
            break;
        }
        case GPON_PLOAM_DS_ENCRYPTPORT:
        {
            if(ploam->onuid==obj->onuid)
            {
                 /* ploam callback */
                if(obj->ploam_callback)
                {
                    (*obj->ploam_callback)(ploam);
                }

                /* this ploam should be processed by applications */
                gemport = (ploam->msg[1]<<4)|(ploam->msg[2]>>4);
        		aesEn = (ploam->msg[0] & 0x01);
        		memset(&ds_attr,0,sizeof(rtk_gpon_usFlow_attr_t));
                for(i=0; i<GPON_DEV_MAX_FLOW_NUM; i++)
                {
        		    if(gpon_dev_dsFlow_get(obj,i,&ds_attr) == RT_ERR_OK)
                    {
                        if(ds_attr.gem_port_id == gemport)
                        {
                            ds_attr.aes_en = aesEn;
                            gpon_dev_dsFlow_add(obj, i, &ds_attr);
                            break;
                        }
                    }
                }

                gpon_ploam_ack_tx(obj,ploam);
            }
            break;
        }
        case GPON_PLOAM_DS_REQUESTPASSWORD:
        {
            if(ploam->onuid==obj->onuid)
            {
                /* ploam callback */
                if(obj->ploam_callback)
                {
                    (*obj->ploam_callback)(ploam);
                }

                gpon_ploam_pwd_tx(obj);
            }
            break;
        }
        case GPON_PLOAM_DS_ASSIGNEDALLOCID:
        {
            if(ploam->onuid==obj->onuid)
            {
                 /* ploam callback */
                if(obj->ploam_callback)
                {
                    (*obj->ploam_callback)(ploam);
                }

                alloc = (ploam->msg[0]<<4)|(ploam->msg[1]>>4);

                /* de-allocate this alloc-id */
                if(ploam->msg[2]==0xFF)
                {
                    gpon_dev_tcont_physical_del(obj,alloc);
                }
                /* allocate this ATM alloc-id */
                else if(ploam->msg[2]==0)
                {
                    /* not supported now */
                }
                /* allocate this GEM alloc-id */
                else if(ploam->msg[2]==1)
                {
                    gpon_dev_tcont_physical_add(obj,alloc);
                }
                /* allocate this DBA alloc-id */
                else if(ploam->msg[2]==2)
                {
#if 0 /* the DBA alloc-id is removed in G.984.3(2008) */
                    gpon_dev_dba_tcont_add(obj,alloc);
#endif
                }
                gpon_ploam_ack_tx(obj,ploam);
            }
            break;
        }
        case GPON_PLOAM_DS_NOMESSAGE:
        {
            /* generally, this ploam is not sent to CPU */
            break;
        }
        case GPON_PLOAM_DS_POPUP:
        {
             /* ploam callback */
            if(obj->ploam_callback)
            {
                (*obj->ploam_callback)(ploam);
            }

            if(obj->status==RTK_GPONMAC_FSM_STATE_O6)
            {
                if(ploam->onuid==obj->onuid)
                {
                    gpon_fsm_event(obj,GPON_FSM_EVENT_RX_DIRECT_POPUP);
                }
                if(ploam->onuid==GPON_DEV_DEFAULT_ONU_ID)
                {
                    gpon_fsm_event(obj,GPON_FSM_EVENT_RX_BC_POPUP);
                }
            }
            break;
        }
        case GPON_PLOAM_DS_REQUESTKEY:
        {
            if(ploam->onuid==obj->onuid)
            {
                 /* ploam callback */
                if(obj->ploam_callback)
                {
                    (*obj->ploam_callback)(ploam);
                }

                gpon_ploam_key_tx(obj);
            }
            break;
        }
        case GPON_PLOAM_DS_CONFIGPORT:
        {
            if(ploam->onuid==obj->onuid)
            {
                 /* ploam callback */
                if(obj->ploam_callback)
                {
                    (*obj->ploam_callback)(ploam);
                }

                /* this ploam should be processed by applications */
                enable_port = (ploam->msg[0] & 0x1);
                ds_attr.gem_port_id = (ploam->msg[1]<<4)|(ploam->msg[2]>>4);
                ds_attr.type = RTK_GPON_FLOW_TYPE_OMCI;
                ds_attr.multicast = 0;
                ds_attr.aes_en = 0;

                us_attr.gem_port_id = (ploam->msg[1]<<4)|(ploam->msg[2]>>4);
                us_attr.type = RTK_GPON_FLOW_TYPE_OMCI;
                us_attr.tcont_id = GPON_OMCI_TCONT_ID;
                us_attr.channel = 0;

                if(enable_port)
                {
                    gpon_dev_dsFlow_add(obj, GPON_OMCI_FLOW_ID, &ds_attr);
                    gpon_dev_usFlow_add(obj, GPON_OMCI_FLOW_ID, &us_attr);
                }
                else
                {
                    gpon_dev_dsFlow_del(obj, GPON_OMCI_FLOW_ID);
                    gpon_dev_usFlow_del(obj, GPON_OMCI_FLOW_ID);
                }

                gpon_ploam_ack_tx(obj,ploam);
            }
            break;
        }
        case GPON_PLOAM_DS_PEE:
        {
             /* ploam callback */
            if(ploam->onuid==GPON_DEV_DEFAULT_ONU_ID)
            {
                if(obj->ploam_callback)
                {
                    (*obj->ploam_callback)(ploam);
                }

                /* this ploam should be processed by applications */

            }
            break;
        }
        case GPON_PLOAM_DS_POWERLEVEL:
        {
            /* ploam callback */
            if(ploam->onuid==obj->onuid || ploam->onuid==GPON_DEV_DEFAULT_ONU_ID)
            {
                if(obj->ploam_callback)
                {
                    (*obj->ploam_callback)(ploam);
                }

                /* this ploam should be processed by applications */

            }
            break;
        }
        case GPON_PLOAM_DS_PST:
        {
            /* ploam callback */
            if(ploam->onuid==obj->onuid || ploam->onuid==GPON_DEV_DEFAULT_ONU_ID)
            {
                if(obj->ploam_callback)
                {
                    (*obj->ploam_callback)(ploam);
                }

                /* this ploam should be processed by applications */

            }
            break;
        }
        case GPON_PLOAM_DS_BER_INTERVAL:
        {
            if(ploam->onuid==obj->onuid || ploam->onuid==GPON_DEV_DEFAULT_ONU_ID)
            {
                /* ploam callback */
                if(obj->ploam_callback)
                {
                    (*obj->ploam_callback)(ploam);
                }

                /* this ploam should be processed by applications */
                berInterval = (uint32)(ploam->msg[0] << 24) + (uint32)(ploam->msg[1] << 16) +
                                (uint32)(ploam->msg[2] << 8) + (uint32)(ploam->msg[3]);
                /* set db */
                ret = gpon_dev_berInterval_set(g_gponmac_drv->dev, berInterval);

                if(g_gponmac_drv->dev->ber_timer)
                {
                    GPON_OS_StopTimer(g_gponmac_drv->dev->ber_timer);
                    g_gponmac_drv->dev->ber_timer = 0;
                    gReiTxCnt = 0;
                }
                g_gponmac_drv->dev->ber_timer = GPON_OS_StartTimer(berInterval,FALSE,0,gpon_berInterval_expire);

                gpon_ploam_ack_tx(obj,ploam);
            }
            break;
        }
        case GPON_PLOAM_DS_SWITCHINGKEY:
        {
            if(obj->status==RTK_GPONMAC_FSM_STATE_O5)
            {
                if(ploam->onuid==obj->onuid || ploam->onuid==GPON_DEV_DEFAULT_ONU_ID)
                {
                    /* ploam callback */
                    if(obj->ploam_callback)
                    {
                        (*obj->ploam_callback)(ploam);
                    }

                    /* configure switch key */
                    framecnt = (ploam->msg[0]&0x3F)<<24;
                    framecnt |= ploam->msg[1]<<16;
                    framecnt |= ploam->msg[2]<<8;
                    framecnt |= ploam->msg[3]<<0;

                    if(obj->aes_key_switch_time!=framecnt)
                    {
                        GPON_OS_Log(GPON_LOG_LEVEL_NORMAL,"key switch @ 0x%04x [baseaddr:%p]",framecnt,obj->base_addr);
                        obj->aes_key_switch_time = framecnt;
                        apollo_raw_gpon_aes_keyswitch_write(obj->aes_key_switch_time);
                    }
                    gpon_ploam_ack_tx(obj,ploam);
                }
            }
            break;
        }
        case GPON_PLOAM_DS_EXT_BURSTLENGTH:
        {
            if(obj->status==RTK_GPONMAC_FSM_STATE_O3)
            {
                if(ploam->onuid==GPON_DEV_DEFAULT_ONU_ID)
                {
                    /* ploam callback */
                    if(obj->ploam_callback)
                    {
                        (*obj->ploam_callback)(ploam);
                    }

                    /* configure overhead */
                    obj->burst_overhead.type3_preranged = ploam->msg[0];
                    obj->burst_overhead.type3_ranged = ploam->msg[1];
                    gpon_dev_upstreamOverhead_calculate(obj);
                    gpon_dev_burstHead_preRanged_set(obj);
                }
            }
            break;
        }

        default:
        {
            break;
        }
    }
}

int32 gpon_ploam_tx(gpon_dev_obj_t* obj, int32 urgent, rtk_gpon_ploam_t* ploam)
{
    int32 ret;
    uint8 msg[3+RTK_GPON_PLOAM_MSG_LEN] = {0};
    msg[0] = ploam->onuid;
    msg[1] = ploam->type;
    osal_memcpy(msg+2,ploam->msg,RTK_GPON_PLOAM_MSG_LEN);
    GPON_OS_Log(GPON_LOG_LEVEL_PLOAM,"Send PLOAM %02x (ONU %d) %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x [baseaddr:%p]",
        msg[1],msg[0],msg[2],msg[3],msg[4],msg[5],msg[6],msg[7],msg[8],msg[9],msg[10],msg[11],obj->base_addr);
    if(urgent)
    {
        obj->us_ploam_req_urg = TRUE;
    }
    else
    {
        obj->us_ploam_req_nrm = TRUE;
    }
    ret = apollo_raw_gpon_usPloam_write(TRUE,msg);

    if(ret == RT_ERR_OK)
    {
        obj->cnt_cpu_ploam_tx++;
        if(urgent)
        {
            obj->cnt_cpu_ploam_tx_urg++;
        }
        else
        {
            obj->cnt_cpu_ploam_tx_nor++;
        }
    }

    return ret;
}

int32 gpon_ploam_rx(gpon_dev_obj_t* obj)
{
    uint8 msg[3+RTK_GPON_PLOAM_MSG_LEN] = {0};
    rtk_gpon_ploam_t ploam;
    osal_memset(&ploam,0,sizeof(rtk_gpon_ploam_t));

    while(apollo_raw_gpon_dsPloam_read(msg)==RT_ERR_OK)
    {
        ploam.onuid = msg[0];
        ploam.type = msg[1];
        osal_memcpy(ploam.msg,msg+2,RTK_GPON_PLOAM_MSG_LEN);

        GPON_OS_Log(GPON_LOG_LEVEL_PLOAM,"Rcv PLOAM %02x (ONU %d) %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x [baseaddr:%p]",
            msg[1],msg[0],msg[2],msg[3],msg[4],msg[5],msg[6],msg[7],msg[8],msg[9],msg[10],msg[11],obj->base_addr);
        obj->cnt_cpu_ploam_rx++;
        gpon_ploam_rx_process(obj,&ploam);
    }

    return RT_ERR_OK;
}

int32 gpon_ploam_init(gpon_dev_obj_t* obj)
{
    uint8 msg[3+RTK_GPON_PLOAM_MSG_LEN];
    osal_memset(msg,0,sizeof(msg));

    /* Set Nomsg PLOAMu */
    msg[0] = 0xFF;
    msg[1] = GPON_PLOAM_US_NOMESSAGE;
    GPON_OS_Log(GPON_LOG_LEVEL_PLOAM,"Set U/S NoMsg PLOAM %02x (ONU %d) %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x [baseaddr:%p]",
        msg[1],msg[0],msg[2],msg[3],msg[4],msg[5],msg[6],msg[7],msg[8],msg[9],msg[10],msg[11],obj->base_addr);
    apollo_raw_gpon_usAutoPloam_write(GPON_REG_USAUTOPLOAM_NOMSG,msg);

    /* Set Nomsg PLOAMd MSG ID */
    apollo_raw_gpon_dsPloam_nomsg_write(GPON_PLOAM_DS_NOMESSAGE);

    /* Set DyingGasp PLOAMu */
    osal_memset(msg,0,sizeof(msg));
    msg[0] = 0xFF;
    msg[1] = GPON_PLOAM_US_DYINGGASP;
#if 1
    GPON_OS_Log(GPON_LOG_LEVEL_PLOAM,"Set U/S DyingGasp PLOAM %02x (ONU %d) %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x [baseaddr:%p]",
        msg[1],msg[0],msg[2],msg[3],msg[4],msg[5],msg[6],msg[7],msg[8],msg[9],msg[10],msg[11],obj->base_addr);
    apollo_raw_gpon_usAutoPloam_write(GPON_REG_USAUTOPLOAM_DYINGGASP,msg);
#endif
    return RT_ERR_OK;
}

int32 gpon_ploam_deinit(gpon_dev_obj_t* obj)
{
    uint8 msg[3+RTK_GPON_PLOAM_MSG_LEN];
    osal_memset(msg,0,sizeof(msg));
    msg[0] = obj->onuid;
    msg[1] = GPON_PLOAM_US_DYINGGASP;
    GPON_OS_Log(GPON_LOG_LEVEL_PLOAM,"Send U/S DyingGasp PLOAM %02x (ONU %d) %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x [baseaddr:%p]",
        msg[1],msg[0],msg[2],msg[3],msg[4],msg[5],msg[6],msg[7],msg[8],msg[9],msg[10],msg[11],obj->base_addr);
    if(apollo_raw_gpon_usPloam_write(TRUE,msg))
    {
        GPON_OS_Log(GPON_LOG_LEVEL_WARNING,"Send PLOAM Error");
    }
    else
    {
        obj->cnt_cpu_ploam_tx++;
        obj->cnt_cpu_ploam_tx_urg++;
    }

    if(apollo_raw_gpon_usPloam_write(TRUE,msg))
    {
        GPON_OS_Log(GPON_LOG_LEVEL_WARNING,"Send PLOAM Error");
    }
    else
    {
        obj->cnt_cpu_ploam_tx++;
        obj->cnt_cpu_ploam_tx_urg++;
    }

    if(apollo_raw_gpon_usPloam_write(TRUE,msg))
    {
        GPON_OS_Log(GPON_LOG_LEVEL_WARNING,"Send PLOAM Error");
    }
    else
    {
        obj->cnt_cpu_ploam_tx++;
        obj->cnt_cpu_ploam_tx_urg++;
    }

    return RT_ERR_OK;
}

int32 gpon_ploam_snPloam_set(gpon_dev_obj_t* obj)
{
    uint8 msg[3+RTK_GPON_PLOAM_MSG_LEN];
    osal_memset(msg,0,sizeof(msg));
    msg[0] = 0xFF;
    msg[1] = GPON_PLOAM_US_SN;
    osal_memcpy(msg+2,obj->serial_number.vendor,RTK_GPON_VENDOR_ID_LEN);
    osal_memcpy(msg+6,obj->serial_number.specific,RTK_GPON_VENDOR_SPECIFIC_LEN);
    /* random delay is re-writed by the chipset */
    /* G bit (0x4) must be set */
    msg[11] = 0x4|(obj->power_level&0x03);
    GPON_OS_Log(GPON_LOG_LEVEL_PLOAM,"Set U/S SN PLOAM %02x (ONU %d) %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x [baseaddr:%p]",
        msg[1],msg[0],msg[2],msg[3],msg[4],msg[5],msg[6],msg[7],msg[8],msg[9],msg[10],msg[11],obj->base_addr);
    apollo_raw_gpon_usAutoPloam_write(GPON_REG_USAUTOPLOAM_SN,msg);
    return RT_ERR_OK;
}

void gpon_ploam_usUrgPloamQ_empty_report(gpon_dev_obj_t* obj)
{
    if(obj->us_ploam_req_urg)
    {
        obj->us_ploam_req_urg = FALSE;
        if(obj->usploam_urg_epty_callback)
        {
            (*obj->usploam_urg_epty_callback)();
        }
    }
}

void gpon_ploam_usNrmPloamQ_Empty_report(gpon_dev_obj_t* obj)
{
    if(obj->us_ploam_req_nrm)
    {
        obj->us_ploam_req_nrm = FALSE;
        if(obj->usploam_nrm_epty_callback)
        {
            (*obj->usploam_nrm_epty_callback)();
        }
    }
}

