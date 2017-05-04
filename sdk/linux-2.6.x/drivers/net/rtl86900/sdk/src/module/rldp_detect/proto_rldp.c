/*
 * Copyright (C) 2010 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: 65149 $
 * $Date: 2016-01-15 16:09:00 +0800 (15 Jan 2016) $
 *
 * Purpose : RLDP
 *
 * Feature : RLDP
 *
 */
#include "proto_rldp.h"

extern int drv_nic_register_rxhook ( int portmask, int priority, p2rfunc_t rx );

static int32 _tx_thread ( void* pInput );
static int32 _rx_thread ( void );
static int32 _recovery_thread ( void );
static int32 _trunkEvent ( uint32 event, char* pData );
static int rldp_link_change_mon ( intrBcasterMsg_t* pMsgData );
static int32 _model_pkt_init ( void );
static int32 _port_blink ( rtk_port_t port, rtk_enable_t blink );
static int32 _global_blink ( rtk_enable_t blink );

#define RLDP_LOCK(lock)         \
do{                             \
    if( RT_ERR_OK != osal_sem_mutex_take(lock, OSAL_SEM_WAIT_FOREVER))  \
    {                                                               \
        printk("Get lock faild\n");                               \
        return SYS_ERR_FAILED;                                      \
    }                                                               \
}while(0)

#define RLDP_UNLOCK(lock) osal_sem_mutex_give(lock);                 \

#define MAX_NUM_OF_ACL_RULE_ENTRY() ((dev_info.capacityInfo.max_num_of_acl_rule_entry != 0) ? (dev_info.capacityInfo.max_num_of_acl_rule_entry - 1): 0)

#define RTK_PON_PORT (dev_info.ether.max)

struct proc_dir_entry* rldp_proc_dir = NULL; 
struct proc_dir_entry* rldp_enable_entry = NULL;
struct proc_dir_entry* rldp_tx_interval_entry = NULL;
struct proc_dir_entry* rldp_port_sts_entry = NULL;
struct proc_dir_entry* rldp_port_stat_clr_entry = NULL; 
static rtk_switch_devInfo_t dev_info;

static 
rldp_frame_t 
_model_pkt = {
    .da = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
    .eth_type = { 0x88, 0x99 },
};

static 
rldp_cfg_t 
_rldp_cfg = {
    .enable             =    DISABLED,
    .tx_interval        =    20,
    .recovery_interval  =    60,
};


static 
intrBcasterNotifier_t 
rldpLinkChngNotifier = {
    .notifyType = MSG_TYPE_LINK_CHANGE,
    .notifierCb = rldp_link_change_mon,
}; 


static struct task_struct* __tx_thread = NULL;
static struct task_struct* __recovery_thread = 0;
static rldp_port_sts_t* _ports = NULL;
static rtk_portmask_t _trapPortMask;
static uint8 is_boot = DISABLED;
static int32 FWD_REASON_ACL_RLDP = -1; 

static 
int32 
_tx_thread ( 
    void* pInput )
{
    extern int re8686_tx_with_Info(unsigned char* pPayload, unsigned short length, void* pInfo);
    struct tx_info txInfo;
    rtk_port_t uni_port;
    rldp_port_sts_t* port_sts = NULL;
    char* data = (char*)&(_model_pkt.da);
    rldp_frame_t rldp_frame;
    
    printk ("RLDP TX thread is created!!!\n");
    while (TRUE) {
        set_current_state (TASK_UNINTERRUPTIBLE);
        if (_rldp_cfg.enable == DISABLED) {
            schedule_timeout (HZ);
            continue;
        }

        memset(&txInfo, 0x0, sizeof(struct tx_info));

        FOR_EACH_ETHER_PORT(uni_port) {
            if (uni_port == RTK_PON_PORT) {
                continue;
            }

            port_sts = &_ports[uni_port];
            if ((ENABLED == port_sts->enable) && 
                    (PORT_LINKUP == port_sts->isLinkup) && 
                        RTK_PORTMASK_IS_PORT_SET(_trapPortMask, uni_port)) {
                    port_sts->tx_count++;
                    _model_pkt.tx_port = uni_port;
                    memcpy(&rldp_frame, data, sizeof(rldp_frame_t));

                    txInfo.opts1.bit.dislrn = 1;
                    txInfo.opts1.bit.keep   = 1;
                    txInfo.opts2.bit.cputag = 1;
                    txInfo.opts3.bit.tx_portmask |= (1 << uni_port);
            }
        }

        re8686_tx_with_Info(
            (unsigned char*)&rldp_frame,
            sizeof(rldp_frame_t),
            &txInfo);

        osal_time_sleep(_rldp_cfg.tx_interval);
    }

    return (SYS_ERR_OK);
}

int32 
rldp_rx (
    struct re_private* cp,
    struct sk_buff*    skb,
    struct rx_info*    pRxInfo)
{

    int ret = RE8670_RX_STOP;
    rtk_port_t port;
    rldp_port_sts_t* rx_port = NULL;
    rldp_port_sts_t* tx_port = NULL;
    rldp_frame_t* rldp_pkt = NULL;
    if (pRxInfo->opts3.bit.reason != FWD_REASON_ACL_RLDP) {
        return (RE8670_RX_CONTINUE);
    }

     /* Filter only spa = UNI and RLDP frames */
     if (skb->data [14] != RLDP_PROTOCOL_ID) {
         ret = RE8670_RX_CONTINUE;
         goto ERROR;
     }

     rldp_pkt = (rldp_frame_t*) skb->data;
     port = pRxInfo->opts3.bit.src_port_num;
     tx_port = _ports + rldp_pkt->tx_port;
     rx_port = _ports + port;
     rx_port->rx_count++;

     if (DISABLED == _rldp_cfg.enable) {
         printk (KERN_DEBUG "%s: RLDP not enable\n", __FUNCTION__);
         ret = RE8670_RX_CONTINUE;
         goto ERROR;
     }

     if (DISABLED == rx_port->enable) {
         printk (
             KERN_DEBUG 
             "%s: RLDP port: %u not enable.\n", __FUNCTION__,
             port);
         ret = RE8670_RX_CONTINUE;
         goto ERROR;
     }

     if (0 != memcmp(rldp_pkt->sa, _model_pkt.sa, ETHER_ADDR_LEN)) {
         printk(
             KERN_DEBUG
             "%s: smac not same pkt is %x:%x:%x:%x:%x:%x, "
             "device is %x:%x:%x:%x:%x:%x\n",
             __FUNCTION__,
             rldp_pkt->sa[0], rldp_pkt->sa[1], rldp_pkt->sa[2],
             rldp_pkt->sa[3], rldp_pkt->sa[4], rldp_pkt->sa[5],
             _model_pkt.sa[0], _model_pkt.sa[1], _model_pkt.sa[2],
             _model_pkt.sa[3], _model_pkt.sa[4], _model_pkt.sa[5]);
         ret = RE8670_RX_CONTINUE;
         goto ERROR; 
     }

     if (0 != memcmp(rldp_pkt->random_id, _model_pkt.random_id, 6)) {
         printk(
             KERN_DEBUG
             "random id not same pkt is %x%x%x%x%x%x, dut is %x%x%x%x%x%x\n",
             rldp_pkt->random_id[0], rldp_pkt->random_id[1], 
             rldp_pkt->random_id[2], rldp_pkt->random_id[3], 
             rldp_pkt->random_id[4], rldp_pkt->random_id[5],
             _model_pkt.random_id[0], _model_pkt.random_id[1], 
             _model_pkt.random_id[2], _model_pkt.random_id[3], 
             _model_pkt.random_id[4], _model_pkt.random_id[5]);
         ret = RE8670_RX_CONTINUE;
         goto ERROR;
     }

     rx_port->loop_count++;

     if (FALSE == rx_port->loop_status) {
         rx_port->loop_status = TRUE;
         rx_port->timer = _rldp_cfg.recovery_interval;

         queue_broadcast(
             MSG_TYPE_RLDP_LOOP_STATE_CHNG, 
             RLDP_STS_LOOP_DETECTED, 
             rx_port->port, 
             ENABLED); 

         printk(
             KERN_DEBUG
             "%s: Loop is detected from the port %u\n",
             __FUNCTION__,
             rx_port->port);
         if (rx_port->action == RLDP_ACTION_LOG || rx_port->action == RLDP_ACTION_BOTH) {
             if (rx_port->action == RLDP_ACTION_LED || rx_port->action == RLDP_ACTION_BOTH) {
                 _global_blink(ENABLED);
                 _port_blink(port, ENABLED);
             }
         }
     }
ERROR:
    /* Left the skb to be free by caller */
    return (ret);
}

static 
int32 
_recovery_thread ( 
    void )
{
    rldp_port_sts_t* port_sts = NULL;
    rtk_port_t port;
    static sys_enable_t flag = DISABLED;
    sys_enable_t tmp = DISABLED;
    printk ("Recovery thread is created!!!\n");
    while (TRUE) {
        osal_time_sleep(1);

        if (DISABLED == _rldp_cfg.enable) {
            continue;
        }

        tmp = DISABLED;

        FOR_EACH_ETHER_PORT(port) {
            if (!RTK_PORTMASK_IS_PORT_SET(_trapPortMask, port)) {
                continue;
            }

            port_sts = &_ports[port];

            if ((TRUE == port_sts->loop_status)) {
                port_sts->timer--;
                if (0 == port_sts->timer) {
                    _port_blink(port, DISABLED);
                    port_sts->loop_status = FALSE;
                    queue_broadcast(
                        MSG_TYPE_RLDP_LOOP_STATE_CHNG,
                        RLDP_STS_LOOP_REMOVED,
                        port_sts->port,
                        ENABLED); 
                    printk (
                        KERN_DEBUG
                        "%s: Loop is removed from the port %u\n",
                        __FUNCTION__,
                        port_sts->port);

                } else {
                    tmp = ENABLED;
                }
            }
        }

        if (tmp != flag) {
            _global_blink(tmp);
        }

        flag = tmp;
    }
    return (SYS_ERR_OK);
}

static 
int
rldp_link_change_mon ( 
    intrBcasterMsg_t* pMsgData )
{
    if (!pMsgData) {
        return (-1);
    }

    printk(
        KERN_DEBUG
        "rldp_link_change_mon, port %u, status %u",
        pMsgData->intrBitMask,
        pMsgData->intrSubType);

    if (pMsgData->intrBitMask >= RTK_PON_PORT) {
        return (-1);
    }

    RLDP_LOCK(_ports[pMsgData->intrBitMask].lock);
    if (pMsgData->intrSubType == INTR_STATUS_LINKUP) {
        _ports [pMsgData->intrBitMask].isLinkup = TRUE;
        RLDP_UNLOCK(_ports[pMsgData->intrBitMask].lock);
        return (-1);
    }

    _ports[pMsgData->intrBitMask].isLinkup = FALSE;
    if (_ports [pMsgData->intrBitMask].loop_status) {
        _ports[pMsgData->intrBitMask].loop_status = FALSE;
        _ports[pMsgData->intrBitMask].timer       = 0;

        queue_broadcast(
            MSG_TYPE_RLDP_LOOP_STATE_CHNG,
            RLDP_STS_LOOP_REMOVED,
            pMsgData->intrBitMask,
            ENABLED);
    }

    RLDP_UNLOCK(_ports[pMsgData->intrBitMask].lock);

}

static 
int32 
_model_pkt_init ( 
    void )
{
    rtnl_lock ();
    struct net_device* dev = __dev_get_by_name (&init_net, "eth0");
    if (!dev) {
        printk (KERN_ALERT "Failed to get the net device \"br0\"\n");
    } else {
        memcpy(_model_pkt.sa, dev->dev_addr, ETHER_ADDR_LEN);
        memcpy(_model_pkt.user_id, dev->dev_addr, ETHER_ADDR_LEN);
    }
    rtnl_unlock(); 
    printk(
        "dev mac: %x:%x:%x:%x:%x:%x\n", 
        dev->dev_addr [0],
        dev->dev_addr[1],
        dev->dev_addr[2],
        dev->dev_addr[3],
        dev->dev_addr[4],
        dev->dev_addr[5]);
    _model_pkt.proto_id = RLDP_PROTOCOL_ID;
    memset(_model_pkt.padding, 0x0, sizeof(_model_pkt.padding));

    return (SYS_ERR_OK);
}

static int32 _port_blink ( rtk_port_t port, sys_enable_t blink )
{
    sys_logic_portmask_t portm;

    FOR_EACH_PORT_IN_LOGIC_PORTMASK(port, portm) {
        if (DISABLED == blink) {
                //SYS_ERR_CHK(board_led_portSwCtrl_set(port, LED_PORT_ENTITY_0_1_2, LED_SWCTRL_ACT_DISABLE));
        } else {
                //SYS_ERR_CHK(board_led_portSwCtrl_set(port, LED_PORT_ENTITY_0_1_2, LED_SWCTRL_ACT_BLINK_1024MS));
        }
    }
    return (SYS_ERR_OK);
}

static int32 _global_blink ( rtk_enable_t blink )
{
    if (DISABLED == blink) {
            //SYS_ERR_CHK(board_led_sys_set(LED_SWCTRL_ACT_ON));
    } else {
            //SYS_ERR_CHK(board_led_sys_set(LED_SWCTRL_ACT_BLINK_1024MS));
    }

    return (SYS_ERR_OK);
}

static
int32
_l2_sw_acl_trap_set (
    rtk_pkt_type_t  pkt_type,
    rtk_portmask_t  port_mask,
    rtk_pkt_action_t action )
{
    uint32 ret = RT_ERR_FAILED;
    uint32 rule_idx;
    rtk_acl_ingress_entry_t acl_rule;

    memset(&acl_rule, 0x00, sizeof(rtk_acl_ingress_entry_t));
    acl_rule.templateIdx = 0;

    if (pkt_type == RTK_PKT_RLDP) {
        rtk_acl_field_t rldp_fld;

        memset(&rldp_fld, 0x00, sizeof(rtk_acl_field_t));

        rldp_fld.fieldType = ACL_FIELD_ETHERTYPE;
        rldp_fld.fieldUnion.data.value = 0x8899;
        rldp_fld.fieldUnion.data.mask  = 0xFFFF;
        rldp_fld.next                  = NULL;

        ret = rtk_acl_igrRuleField_add(&acl_rule, &rldp_fld);
        if (ret != RT_ERR_OK) {
            osal_printf("%s: Failed to add the field (0x8899) to the acl rule", __FUNCTION__);
            goto ERR;
        }
    } else {
        osal_printf("%s: the packet type is not supported!!!", __FUNCTION__);
        goto ERR;
    }

    if (action == DST_FE) {
        ret = rtk_acl_igrRuleEntry_del(rule_idx);
        if (ret != RT_ERR_OK) {
            osal_printf(
                "%s: Failed to delete the rule, idx %u, ret 0x%x, DST_FE\n",
                __FUNCTION__,
                rule_idx,
                ret);
        }
        goto ERR;
    }

    acl_rule.valid = ENABLED;
    RTK_PORTMASK_ASSIGN (acl_rule.activePorts, port_mask);
    rtk_port_t uni_port;
    FOR_EACH_ETHER_PORT (uni_port) {
        if (RTK_PORTMASK_IS_PORT_SET (port_mask, uni_port)) {
            ret = rtk_acl_igrState_set(uni_port, ENABLED);
            if (ret != RT_ERR_OK) {
                printk (
                    "Failed to enable the acl state for the port %u\n", 
                    uni_port);
            }
        }
    }

    acl_rule.act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLED;
    if (action == DST_DROP) {
        acl_rule.act.forwardAct.act = ACL_IGR_FORWARD_DROP_ACT;
    } else {
        acl_rule.act.forwardAct.act = ACL_IGR_FORWARD_TRAP_ACT;
    }
    rtk_acl_ingress_entry_t tmp;
    tmp.index = MAX_NUM_OF_ACL_RULE_ENTRY();
    while (1) {
        ret = rtk_acl_igrRuleEntry_get (&tmp);
        if (ret == RT_ERR_OK && !tmp.valid) {
            acl_rule.index  = tmp.index;
            rule_idx        = tmp.index;
            FWD_REASON_ACL_RLDP = 64 + rule_idx; 
            break;
        }
        if (tmp.index) {
            tmp.index--;
        } else {
            break;
        }
    }

    ret = rtk_acl_igrRuleEntry_add(&acl_rule);
    if (ret != RT_ERR_OK) {
        printk ("%s: Failed to add the acl rule\n", __FUNCTION__);
    }
    printk (
        "Add the rule to the no. %u entry in the acl table, reason %u\n", 
        acl_rule.index,
        FWD_REASON_ACL_RLDP);
ERR:
    return (ret);
}


static
int
rldp_proc_enable_read (
    char* page,
    char** start,
    off_t off,
    int count,
    int* eof,
    void* data )
{
    int len = 0;

    len = sprintf(page, "rldp enable status: %u\n", _rldp_cfg.enable);

    return (len);
}

static
int
rldp_proc_enable_write (
    struct file* file,
    const char* buffer,
    unsigned long count,
    void* data )
{
    unsigned char tmp[3] = { 0 };
    int len = 2;

    if (buffer && !copy_from_user(tmp, buffer, len)) {
        _rldp_cfg.enable = simple_strtoul (tmp, NULL, 10);
        printk("\nwrite the rldp enable to %u\n", _rldp_cfg.enable);
        if (_rldp_cfg.enable >= 1) {
            rtk_port_t uni_port;
            _rldp_cfg.enable = 1;
            FOR_EACH_ETHER_PORT(uni_port) {
                _ports[uni_port].lock = osal_sem_mutex_create();
            }
            _model_pkt_init();
        } 
        rldp_set(&_rldp_cfg);
    }
    return (count);
}

static
int
rldp_proc_tx_interval_read (
    char* page,
    char** start,
    off_t off,
    int count,
    int* eof,
    void* data )
{
    int len = 0;

    len = sprintf(page, "rldp tx interval: %u\n", _rldp_cfg.tx_interval);

    return (len);
}

static
int
rldp_proc_tx_interval_write (
    struct file* file,
    const char* buffer,
    unsigned long count,
    void* data )
{
    unsigned char tmp[6] = { 0 };
    unsigned long tx_inr;
    if (count > 6) {
        printk("\nthe tx_interval is too large\n");
        goto ERR;
    }

    if (buffer && !copy_from_user(tmp, buffer, count)) {
        (((tx_inr = simple_strtoul(tmp, NULL, 10)) > 65535) ? 
            (_rldp_cfg.tx_interval = 65535) :(_rldp_cfg.tx_interval = tx_inr));
        printk("\nwrite the rldp tx interval to %u\n", _rldp_cfg.tx_interval);
        if (_rldp_cfg.enable) {
            wake_up_process(__tx_thread);
        }
    }

ERR:
    return (count);
}

static
int
rldp_proc_port_sts_read (
    char* page,
    char** start,
    off_t off,
    int count,
    int* eof,
    void* data ) 
{
    uint32 len = 0;
    rtk_port_t uni_port = 0;

    FOR_EACH_ETHER_PORT(uni_port) {
        if (uni_port == RTK_PON_PORT) {
            continue;
        }

        printk(
            "port %u\n  enable status: %u\n",
            uni_port,
            _ports[uni_port].enable);
        printk("  tx count: %u\n", _ports[uni_port].tx_count);
        printk("  rx count: %u\n", _ports[uni_port].rx_count);
        printk("  loop count: %u\n", _ports[uni_port].loop_count);
        printk("  loop status: %u\n", _ports[uni_port].loop_status);
        printk(
            "  port link status: %s\n",
            (_ports[uni_port].isLinkup ? "UP" : "DOWN"));
    }

    return (len);
}

int32 
rldp_set ( 
    rldp_cfg_t* rldp )
{
    rtk_port_t port;

    _rldp_cfg.tx_interval = rldp->tx_interval;
    _rldp_cfg.recovery_interval = rldp->recovery_interval;

//    if (_rldp_cfg.enable != rldp->enable) {
//        SYS_ERR_CHK(rsd_acl_rsv_ruleEnable_set(ACL_RSV_RLDP, rldp->enable));

        if (DISABLED == rldp->enable) {
            _global_blink(DISABLED);

            FOR_EACH_LOGIC_PORT(port) {
                if (TRUE == _ports[port].loop_status) {
                    _port_blink(port, DISABLED);
                    _ports[port].loop_status = FALSE;
                    _ports[port].timer = 0;
                }
            }
        } else {
            char tmp[] = { 0, 0, 0, 0, 0, 0 };
            uint32 aaa = 0;

            if (0 == osal_memcmp(_model_pkt.random_id, tmp, sizeof(_model_pkt.random_id))) {
                aaa = jiffies;
                *(short*)&_model_pkt.random_id = jiffies % 54321;
                osal_time_usleep(jiffies % 1371);
                aaa *= jiffies;
                *(int*)(_model_pkt.random_id + 2) = aaa;
                printk (
                    KERN_DEBUG
                    "random_id: %x:%x:%x:%x:%x:%x\n", 
                    _model_pkt.random_id[0], _model_pkt.random_id[1], 
                    _model_pkt.random_id[2], _model_pkt.random_id[3], 
                    _model_pkt.random_id[4], _model_pkt.random_id[5]);
            }
        }

//        _rldp_cfg.enable = rldp->enable;
//   }

    return (SYS_ERR_OK);
}



int32 rldp_get ( rldp_cfg_t* rldp )
{
    memcpy(rldp, &_rldp_cfg, sizeof(rldp_cfg_t));

    return (SYS_ERR_OK);
}

int32 rldp_port_set ( 
    rldp_port_sts_t* port )
{
//    rsd_acl_rsv_field_t field;
    rldp_port_sts_t* old = &_ports[port->port];

    RLDP_LOCK(old->lock);

    if (old->enable != port->enable) {
        if (ENABLED == port->enable) {
            LOGIC_PORTMASK_SET_PORT(_trapPortMask, port->port);
        } else {
            LOGIC_PORTMASK_CLEAR_PORT(_trapPortMask, port->port);
            if (TRUE == old->loop_status) {
                _port_blink(port->port, DISABLED);
                old->loop_status = FALSE;
                old->timer = 0;
            }
        }
    }

    old->enable = port->enable;
    old->action = port->action;

//    field.fieldMask = ACL_RSV_FIELDMASK_SPM;
//    field.spm = _trapPortMask;
    //SYS_ERR_CHK(rsd_acl_rsv_rule_update(ACL_RSV_RLDP, &field, NULL, NULL));

    RLDP_UNLOCK(old->lock);

    return (SYS_ERR_OK);
}



int32 rldp_proc_port_stat_clr ( void )
{
    rtk_port_t port;

    printk ("clear statistics\n");

    FOR_EACH_ETHER_PORT(port) {
        if (port == RTK_PON_PORT) {
            continue;
        }
        RLDP_LOCK(_ports[port].lock);
        _ports[port].tx_count = 0;
        _ports[port].rx_count = 0;
        _ports[port].loop_count = 0;
        RLDP_UNLOCK(_ports[port].lock);
    }

    return (SYS_ERR_OK);
} 

void
rldp_proc_init ( 
    void ) 
{
    if (NULL == rldp_proc_dir) {
        rldp_proc_dir = proc_mkdir ("rldp_drv", NULL);
    }

    if (rldp_proc_dir) {
        rldp_enable_entry = create_proc_entry ("rldp_enable", 0, rldp_proc_dir);
        if (rldp_enable_entry) {
            rldp_enable_entry->read_proc = rldp_proc_enable_read;
            rldp_enable_entry->write_proc = rldp_proc_enable_write;
        }

        rldp_tx_interval_entry = create_proc_entry("tx_interval", 0, rldp_proc_dir);
        if (rldp_tx_interval_entry) {
            rldp_tx_interval_entry->read_proc = rldp_proc_tx_interval_read;
            rldp_tx_interval_entry->write_proc = rldp_proc_tx_interval_write;
        }
        
        rldp_port_sts_entry = create_proc_entry("port_sts", 0, rldp_proc_dir);
        if (rldp_port_sts_entry) {
            rldp_port_sts_entry->read_proc = rldp_proc_port_sts_read;
//            rldp_port_sts_entry->write_proc = rldp_proc_tx_interval_write;
        }
        rldp_port_stat_clr_entry = create_proc_entry("port_stat_clear", 0, rldp_proc_dir);
        if (rldp_port_stat_clr_entry) {
            rldp_port_stat_clr_entry->read_proc = rldp_proc_port_stat_clr;
        }
    }

    return;
}

int32
__init 
rldp_init ( 
    void )
{
    int32 ret = -1;
    rtk_port_t port;

    ret = rtk_switch_deviceInfo_get(&dev_info); 
    if (ret != RT_ERR_OK) {
        return ret; 
    }

    _model_pkt_init();

    _ports = osal_alloc ( 
                sizeof (rldp_port_sts_t) * dev_info.ether.portNum); 
    if (!_ports) {
        printk ("Failed to allocate the memory for the _ports\n");
        return -1;
    }

    memset(_ports, 0x00, sizeof(rldp_port_sts_t) * dev_info.ether.portNum);
    memset(&_trapPortMask, 0, sizeof(_trapPortMask));

    FOR_EACH_ETHER_PORT (port) {
        _ports[port].lock = osal_sem_mutex_create();
        _ports[port].port = port;
        (port == RTK_PON_PORT ? 
            (_ports[port].enable = DISABLED): (_ports [port].enable = ENABLED));
        _ports[port].action = RLDP_ACTION_NONE;

        if (DISABLED == _ports[port].enable) {
            continue;
        }

        RTK_PORTMASK_PORT_SET(_trapPortMask, port);
    }

    ret = _l2_sw_acl_trap_set(RTK_PKT_RLDP, _trapPortMask, DST_CPU);
    if (ret != RT_ERR_OK) {
        return (ret);
    }

    drv_nic_register_rxhook (
        _trapPortMask.bits [0],
        0,
        rldp_rx);

    __tx_thread = kthread_create (
                        (void*)_tx_thread,
                        NULL, 
                        "RLDP TX");
    if (!__tx_thread) {
        printk (KERN_ALERT "Create tx thread faild\n");
    } else {
        wake_up_process(__tx_thread);
    }
    __recovery_thread = kthread_create( 
                            (void*)_recovery_thread,
                            NULL,
                            "RLDP REC");
    if (!__recovery_thread) {
        printk (KERN_ALERT "KERN_ALERT Create recovery thread faild\n");
    } else {
        wake_up_process(__recovery_thread);
    }

    if (SYS_ERR_OK != 
            intr_bcaster_notifier_cb_register(&rldpLinkChngNotifier)) {
        printk (
            KERN_ALERT 
            " OMCI module register intr_bcaster module Error !! \n");
    }

    rldp_proc_init ();
    printk (KERN_ALERT "\nrldp_detect init ok!\n");
    return 0;
}

void
__exit
rldp_exit (
    void) {
    osal_free (_ports);
    kthread_stop (__tx_thread);
    kthread_stop(__recovery_thread);
    return;
}

MODULE_LICENSE ("GPL");
MODULE_DESCRIPTION ("RealTek RLDP Detect module");
module_init (rldp_init);
module_exit (rldp_exit); 


