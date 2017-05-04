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
 * $Revision: 46475 $
 * $Date: 2014-02-14 11:03:12 +0800 (?±ä?, 14 äºŒæ? 2014) $
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

#include <rtk/trap.h>
#include <rtk/epon.h>
#include <rtk/oam.h>
#include <hal/common/halctrl.h>

#include "epon_oam_config.h"
#include "epon_oam_rx.h"
#include "epon_oam_db.h"
#include "epon_oam_msgq.h"
#include "epon_oam_dbg.h"
#include "epon_oam_err.h"

#include "ctc_oam.h"

/* 
 * Symbol Definition 
 */
#define EPON_OAM_STATEKEEPER_SIG_START          (SIGRTMIN+3)
#define EPON_OAM_KEEPALIVE_TIME                 (5000) /* In unit of ms */

/*  
 * Data Declaration  
 */
static int msgQId;
static timer_t skKeepalive[EPON_OAM_SUPPORT_LLID_NUM];
static unsigned int failoverTime;
static struct timespec failoverTimeSpec;
static unsigned int backoffTime;
static struct timespec backoffTimeSpec;

/* 
 * Macro Definition 
 */
#define EPON_TIMER_SET(timerId,ms,ret)              \
{                                                   \
    struct itimerspec its;                          \
                                                    \
    its.it_value.tv_sec = ms/1000;                  \
    its.it_value.tv_nsec = (ms%1000) * 1000000;     \
    its.it_interval.tv_sec = its.it_value.tv_sec;   \
    its.it_interval.tv_nsec = its.it_value.tv_nsec; \
    ret = timer_settime(timerId, 0, &its, NULL);    \
}

/*  
 * Function Declaration  
 */
/* Function Name:
 *      epon_oam_user_init
 * Description:
 *      Execute user specific initializations
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      EPON_OAM_ERR_OK
 *      ...
 * Note:
 *      All user initialization function should put in here
 */
int epon_oam_user_init(void)
{
    /* For CTC OAM initialization */
    ctc_oam_init();
}

/* Function Name:
 *      epon_oam_userDb_init
 * Description:
 *      Execute user specific database initializations
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      EPON_OAM_ERR_OK
 *      ...
 * Note:
 *      All user database initialization function should put in here
 *      This initialization function will be call every time the specific LLID
 *      is link down
 */
int epon_oam_userDb_init(
    unsigned char llidIdx)
{
    /* For CTC OAM initialization */
    ctc_oam_db_init(llidIdx);
}

/* ------------------------------------------------------------------------- */
/* Internal APIs */
int epon_oam_event_send(
    unsigned char llidIdx,
    unsigned int eventId)
{
    int ret;
    oam_msgqEventData_t event;

    event.mtype = eventId;
    event.msgqData.llidIdx = llidIdx;
    event.msgqData.dataSize = 0;
    ret = msgsnd(msgQId, (void *)&event, sizeof(event.msgqData), IPC_NOWAIT);
    if(-1 == ret)
    {
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
            "[OAM:%s:%d] msgsnd failed %d\n", __FILE__, __LINE__, errno);
        return EPON_OAM_ERR_MSGQ;
    }

    return EPON_OAM_ERR_OK;
}

int epon_oam_eventData_send(
    unsigned char llidIdx,
    unsigned int eventId,
    unsigned char *data,
    unsigned short dataLen)
{
    int ret;
    oam_msgqEventData_t eventData;

    if(dataLen > EPON_OAM_EVENTDATA_MAX)
    {
        return EPON_OAM_ERR_PARAM;
    }

    eventData.mtype = eventId;
    eventData.msgqData.llidIdx = llidIdx;
    eventData.msgqData.dataSize = dataLen;
    memcpy(eventData.msgqData.data, data, dataLen);
    ret = msgsnd(msgQId, (void *)&eventData, sizeof(eventData.msgqData), IPC_NOWAIT);
    if(-1 == ret)
    {
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
            "[OAM:%s:%d] msgsnd failed %d\n", __FILE__, __LINE__, errno);
        return EPON_OAM_ERR_MSGQ;
    }

    return EPON_OAM_ERR_OK;
}

static void epon_oam_keepalive_timeout(
    int sig,
    siginfo_t *si,
    void *uc)
{
    epon_oam_event_send(sig - EPON_OAM_STATEKEEPER_SIG_START, EPON_OAM_EVENT_KEEPALIVE_TIMEOUT);
    return;
}

static void epon_oam_holdover_timeout(
    int sig)
{
    /* MODIFY ME - do something */
    return;
}

static void epon_stateKeeper_timer_init(void)
{
    int i;
    sigset_t sigMask;
    struct sigaction sa;
    struct sigevent sevp;

    /* Create EPON_OAM_SUPPORT_LLID_NUM timers for each LLID
     * Each of the timer uses a signal to tell the difference
     * TODO - try single signal implementation
     */
    for(i = 0;i < EPON_OAM_SUPPORT_LLID_NUM;i++)
    {
        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = epon_oam_keepalive_timeout;
        sigemptyset(&sa.sa_mask);
        /* Linux thread imeplementation uses SIGRTMIN ~ SIGRTMIN + 2, skip them */
        if(-1 == sigaction(EPON_OAM_STATEKEEPER_SIG_START + i, &sa, NULL))
        {
            EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
                "[OAM:%s:%d] signal action set failed %d %d\n", __FILE__, __LINE__, errno, i);
            continue;
        }

        sevp.sigev_notify = SIGEV_SIGNAL;
        sevp.sigev_signo = EPON_OAM_STATEKEEPER_SIG_START + i;
        sevp.sigev_value.sival_ptr = &skKeepalive[i];
        if(-1 == timer_create(CLOCK_REALTIME, &sevp, &skKeepalive[i]))
        {
            EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
                "[OAM:%s:%d] keepalive timer create failed %d\n", __FILE__, __LINE__, errno);
            return;
        }
    }
}

static int epon_oam_triggerMpcp_reg(
    unsigned char llidIdx)
{
    int ret;
    rtk_epon_llid_entry_t llidEntry;
    rtk_epon_regReq_t regEntry;
    
    /* Trigger register */
    llidEntry.llidIdx = llidIdx;
    ret = rtk_epon_llid_entry_get(&llidEntry);
    if(ret)
    {
        return ret;
    }

    llidEntry.valid = DISABLED;
    llidEntry.llid = 0x7fff;
    ret = rtk_epon_llid_entry_set(&llidEntry);
    if(ret)
    {
        return ret;
    }

    ret = rtk_epon_registerReq_get(&regEntry);
    if(ret)
    {
        return ret;
    }
    regEntry.llidIdx = llidIdx;
    regEntry.doRequest = ENABLED;
    ret = rtk_epon_registerReq_set(&regEntry);
    if(ret)
    {
        return ret;
    }

    return 0;
}

static int epon_oam_llidInfo_clear(
    unsigned char llidIdx)
{
    int ret;
    rtk_epon_llid_entry_t llidEntry;
    
    llidEntry.llidIdx = llidIdx;
    ret = rtk_epon_llid_entry_get(&llidEntry);
    if(RT_ERR_OK != ret)
    {
        return ret;
    }

    llidEntry.valid = DISABLED;
    llidEntry.llid = 0x7fff;
    ret = rtk_epon_llid_entry_set(&llidEntry);
    if(RT_ERR_OK != ret)
    {
        return ret;
    }

    return 0;
}

void epon_oam_cli_proc(
    oam_cli_t *pCli)
{
    unsigned int flag;
    unsigned int value;
    unsigned short oamPduFlag;
    oam_config_t oamConfig;
    oam_oamInfo_t oamInfo;
    ctc_onuAuthLoid_t authLoid;

    switch(pCli->cliType)
    {
    case EPON_OAM_CLI_DBG_SET:
        epon_oam_dbgFlag_get(&flag);
        printf("EPON OAM debug flag 0x%08x -> ", flag);
        epon_oam_dbgFlag_set(pCli->u.cliDbg.flag);
        epon_oam_dbgFlag_get(&flag);
        printf("0x%08x\n", flag);
        break;
    case EPON_OAM_CLI_DBG_GET:
        epon_oam_dbgFlag_get(&flag);
        printf("EPON OAM debug flag 0x%08x\n", flag);
        break;
    case EPON_OAM_CLI_CFGOAM_SET:
        epon_oam_config_get(pCli->u.cliEnable.llidIdx, &oamConfig);
        oamConfig.oamEnabled = pCli->u.cliEnable.enable;
        epon_oam_config_set(pCli->u.cliEnable.llidIdx, &oamConfig);
        break;
    case EPON_OAM_CLI_CFGMAC_SET:
        epon_oam_config_get(pCli->u.cliMac.llidIdx, &oamConfig);
        memcpy(oamConfig.macAddr, pCli->u.cliMac.mac, 6);
        epon_oam_config_set(pCli->u.cliMac.llidIdx, &oamConfig);
        break;
    case EPON_OAM_CLI_CFGAUTOREG_SET:
        epon_oam_config_get(pCli->u.cliAutoReg.llidIdx, &oamConfig);
        oamConfig.autoRegEnable = pCli->u.cliAutoReg.autoRegEnable;
        oamConfig.autoRegTime = pCli->u.cliAutoReg.autoRegTime;
        epon_oam_config_set(pCli->u.cliAutoReg.llidIdx, &oamConfig);
        break;
    case EPON_OAM_CLI_CFGHOLDOVER_SET:
        epon_oam_config_get(pCli->u.cliHoldover.llidIdx, &oamConfig);
        oamConfig.holdoverEnable = pCli->u.cliHoldover.holdoverEnable;
        oamConfig.holdoverTime = pCli->u.cliHoldover.holdoverTime;
        epon_oam_config_set(pCli->u.cliHoldover.llidIdx, &oamConfig);
        break;
    case EPON_OAM_CLI_CFGEVENT_SET:
        epon_oam_config_get(pCli->u.cliEvent.llidIdx, &oamConfig);
        oamConfig.eventRepCnt = pCli->u.cliEvent.eventRepCnt;
        oamConfig.eventRepIntvl = pCli->u.cliEvent.eventRepIntvl;
        epon_oam_config_set(pCli->u.cliEvent.llidIdx, &oamConfig);
        break;
    case EPON_OAM_CLI_CFG_GET:
        epon_oam_config_get(pCli->u.cliLlidIdx.llidIdx, &oamConfig);
        printf("OAM configs for LLID %u\n", pCli->u.cliLlidIdx.llidIdx);
        printf("OAM state: %s\n", oamConfig.oamEnabled ? "enable" : "disable");
        printf("MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
            oamConfig.macAddr[0],oamConfig.macAddr[1],oamConfig.macAddr[2],
            oamConfig.macAddr[3],oamConfig.macAddr[4],oamConfig.macAddr[5]);
        printf("autoReg enable: %s\n", oamConfig.autoRegEnable ? "enable" : "disable");
        printf("autoReg time: %u ms\n", oamConfig.autoRegTime);
        printf("holdover enable: %s\n", oamConfig.holdoverEnable ? "enable" : "disable");
        printf("holdover time: %u ms\n", oamConfig.holdoverTime);
        printf("event repeat count: %u\n", oamConfig.eventRepCnt);
        printf("event repeat interval: %u\n", oamConfig.eventRepIntvl);
        break;
    case EPON_OAM_CLI_COUNTER_CLEAR:
        epon_oam_counter_init(pCli->u.cliEvent.llidIdx, EPON_OAM_COUNTERTYPE_TX);
        epon_oam_counter_init(pCli->u.cliEvent.llidIdx, EPON_OAM_COUNTERTYPE_RX);
        epon_oam_counter_init(pCli->u.cliEvent.llidIdx, EPON_OAM_COUNTERTYPE_TXINFO);
        epon_oam_counter_init(pCli->u.cliEvent.llidIdx, EPON_OAM_COUNTERTYPE_RXINFO);
        epon_oam_counter_init(pCli->u.cliEvent.llidIdx, EPON_OAM_COUNTERTYPE_TXORGSPEC);
        epon_oam_counter_init(pCli->u.cliEvent.llidIdx, EPON_OAM_COUNTERTYPE_RXORGSPEC);
        epon_oam_counter_init(pCli->u.cliEvent.llidIdx, EPON_OAM_COUNTERTYPE_DROP);
        epon_oam_counter_init(pCli->u.cliEvent.llidIdx, EPON_OAM_COUNTERTYPE_LOSTLINK);
        break;
    case EPON_OAM_CLI_COUNTER_GET:
        printf("OAM counters for LLID %u\n", pCli->u.cliLlidIdx.llidIdx);
        value = epon_oam_counter_get(pCli->u.cliLlidIdx.llidIdx, EPON_OAM_COUNTERTYPE_TX);
        printf("Tx: %u\n", value);
        value = epon_oam_counter_get(pCli->u.cliLlidIdx.llidIdx, EPON_OAM_COUNTERTYPE_RX);
        printf("Rx: %u\n", value);
        value = epon_oam_counter_get(pCli->u.cliLlidIdx.llidIdx, EPON_OAM_COUNTERTYPE_TXINFO);
        printf("Tx Info: %u\n", value);
        value = epon_oam_counter_get(pCli->u.cliLlidIdx.llidIdx, EPON_OAM_COUNTERTYPE_RXINFO);
        printf("Rx Info: %u\n", value);
        value = epon_oam_counter_get(pCli->u.cliLlidIdx.llidIdx, EPON_OAM_COUNTERTYPE_TXORGSPEC);
        printf("Tx Orgnization Specific: %u\n", value);
        value = epon_oam_counter_get(pCli->u.cliLlidIdx.llidIdx, EPON_OAM_COUNTERTYPE_RXORGSPEC);
        printf("Rx Orgnization Specific: %u\n", value);
        value = epon_oam_counter_get(pCli->u.cliLlidIdx.llidIdx, EPON_OAM_COUNTERTYPE_DROP);
        printf("Drop: %u\n", value);
        value = epon_oam_counter_get(pCli->u.cliLlidIdx.llidIdx, EPON_OAM_COUNTERTYPE_LOSTLINK);
        printf("Lost Link: %u\n", value);
        break;
    case EPON_OAM_CLI_DBGEXT_SET:
        epon_oam_dbgExt_get(&flag);
        printf("EPON OAM extended flag 0x%08x -> ", flag);
        epon_oam_dbgExt_set(pCli->u.cliDbg.flag);
        epon_oam_dbgExt_get(&flag);
        printf("0x%08x\n", flag);
        break;
    case EPON_OAM_CLI_DBGEXT_GET:
        epon_oam_dbgExt_get(&flag);
        printf("EPON OAM extended flag 0x%08x\n", flag);
        break;
    case EPON_OAM_CLI_FAILOVER_SET:
        failoverTime = pCli->u.cliFailover.granularity;
        failoverTimeSpec.tv_sec = failoverTime / 1000;
        failoverTimeSpec.tv_nsec = (failoverTime % 1000) * 1000 * 1000;
        backoffTime = pCli->u.cliFailover.backoff;
        backoffTimeSpec.tv_sec = backoffTime / 1000;
        backoffTimeSpec.tv_nsec = (backoffTime % 1000) * 1000 * 1000;
        break;
    case EPON_OAM_CLI_FAILOVER_GET:
        printf("EPON OAM failover parameters\n");
        printf("Granularity: %u ms\n", failoverTime);
        printf("backoff: %u ms\n", backoffTime);
        break;
    case EPON_OAM_CLI_OAMINFOOUI_SET:
        epon_oam_localInfo_get(pCli->u.cliMac.llidIdx, &oamInfo);
        memcpy(oamInfo.oui, pCli->u.cliMac.mac, 3);
        epon_oam_localInfo_set(pCli->u.cliMac.llidIdx, &oamInfo);
        break;
    case EPON_OAM_CLI_OAMINFO_GET:
        epon_oam_localInfo_get(pCli->u.cliMac.llidIdx, &oamInfo);
        printf("OAM Info for LLID %u\n", pCli->u.cliLlidIdx.llidIdx);
        printf("OUI: %02x:%02x:%02x\n",
            oamInfo.oui[0],oamInfo.oui[1],oamInfo.oui[2]);
        break;
    case EPON_OAM_CLI_OAMSTATE_GET:
        oamPduFlag = epon_oam_oamPduFlag_get(pCli->u.cliLlidIdx.llidIdx);
        printf("OAMPDU Satisfication\n");
        printf("       Eval Stable\n");
        printf(" Local %4u %6u\n",
            (oamPduFlag & EPON_OAM_FLAG_LOCAL_EVAL) ? 1 : 0,
            (oamPduFlag & EPON_OAM_FLAG_LOCAL_STABLE) ? 1 : 0);
        printf("Remote %4u %6u\n",
            (oamPduFlag & EPON_OAM_FLAG_REMOTE_EVAL) ? 1 : 0,
            (oamPduFlag & EPON_OAM_FLAG_REMOTE_STABLE) ? 1 : 0);
        break;
    case EPON_OAM_CLI_REG_TRIGGER:
        epon_oam_triggerMpcp_reg(pCli->u.cliLlidIdx.llidIdx);
        break;
    case CTC_OAM_CLI_LOID_GET:
        ctc_oam_onuAuthLoid_get(pCli->u.cliCtcLoid.llidIdx, &authLoid);
        printf("LOID for LLID %d\n", pCli->u.cliCtcLoid.llidIdx);
        printf("    LOID: %s\n", authLoid.loid);
        printf("password: %s\n", authLoid.password);
        break;
    case CTC_OAM_CLI_LOID_SET:
        strcpy(authLoid.loid, pCli->u.cliCtcLoid.loid);
        strcpy(authLoid.password, pCli->u.cliCtcLoid.password);
        ctc_oam_onuAuthLoid_set(pCli->u.cliCtcLoid.llidIdx, &authLoid);
        break;
    default:
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_WARN,
                "[OAM:%s:%d] unsupported CLI type %u\n", __FILE__, __LINE__, pCli->cliType);
        break;
    }
}

void *epon_oam_stateKeeper(void *argu)
{
    int ret;
    int permits;
    static unsigned short pushedLen = 0;
    unsigned char llidIdx;
    unsigned char dyingGaspBuf[EPON_OAM_DYINGGASP_BUF_LEN];
    key_t msgQKey = 1568;
    oam_msgqEventData_t eventData;
    struct itimerspec its;
    oam_oamInfo_t oamInfo;

    /* Create message queue for receiving the event from others */
    /* S_IRUSR | S_IWUSR | State keeper can read/write message
     * S_IRGRP | S_IWGRP | All others can read/write message
     * S_IROTH | S_IWOTH   All others can read/write message
     */
    permits = 00666;
    permits |= IPC_CREAT;
    msgQId = msgget(msgQKey, permits);
    if(-1 == msgQId)
    {
        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
            "[OAM:%s:%d] msgq create failed %d\n", __FILE__, __LINE__, errno);
        return NULL;
    }

    /* Init all timers used by state keeper */
    epon_stateKeeper_timer_init();

	while(1)
	{
		ret = msgrcv(msgQId, (void *) &eventData, sizeof(eventData.msgqData), 0, 0); /* Blocking call */
        if(-1 == ret)
        {
            if(EINTR == errno)
            {
                /* A signal is caught, just continue */
            }
            else
            {
                EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
                    "[OAM:%s:%d] msgq recv failed %d\n", __FILE__, __LINE__, errno);
            }
            continue;
        }

        llidIdx = eventData.msgqData.llidIdx;
        if(llidIdx >= EPON_OAM_SUPPORT_LLID_NUM)
        {
            /* Invalid llidIdx, just ignore it */
            continue;
        }

        EPON_OAM_PRINT(EPON_OAM_DBGFLAG_INFO, 
            "LLIDIdx %u EventType %u\n", llidIdx, eventData.mtype);
        switch(eventData.mtype)
        {
        case EPON_OAM_EVENT_DISCOVERY_COMPLETE:
            /* For the first registration, put a dying gasp packet into queue 127 */
        #if 0
            if(0 == pushedLen)
        #endif
            {
                epon_oam_dyingGasp_gen(llidIdx, dyingGaspBuf, EPON_OAM_DYINGGASP_BUF_LEN, &pushedLen);
                epon_oam_dyingGasp_send(llidIdx, dyingGaspBuf, pushedLen);
            }
            /* Start keepalive tiemr */
            EPON_TIMER_SET(skKeepalive[llidIdx], EPON_OAM_KEEPALIVE_TIME, ret)
            if(0 != ret)
            {
                EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
                    "[OAM:%s:%d] timer set failed %d\n", __FILE__, __LINE__, errno);
            }
            /* Set multiplexer to forward */
            if((ret = rtk_oam_multiplexerAction_set(HAL_GET_PON_PORT(),OAM_MULTIPLEXER_ACTION_FORWARD)) != RT_ERR_OK)
            {
                EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
                    "[OAM:%s:%d] OAM multiplexerAction set failed %d\n", __FILE__, __LINE__, ret);
            }
            break;
        case EPON_OAM_EVENT_KEEPALIVE_TIMEOUT:
        case EPON_OAM_EVENT_LOS:
            /* Stop timer */
            EPON_TIMER_SET(skKeepalive[llidIdx], 0, ret)
            if(0 != ret)
            {
                EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
                    "[OAM:%s:%d] timer set failed %d\n", __FILE__, __LINE__, errno);
            }
            /* Clear HW info */
            epon_oam_llidInfo_clear(llidIdx);
            /* Reset all the runtime database of the specified LLID index */
            epon_oam_discoveryLlid_init(llidIdx);
            epon_oam_oamDb_init(llidIdx);
            epon_oam_userDb_init(llidIdx);
            /* Clear remote OAM info */
            epon_oam_remoteInfo_init(llidIdx);
            /* Clear local OAM info revision */
            epon_oam_localInfo_get(llidIdx, &oamInfo);
            oamInfo.revision = 0;
            epon_oam_localInfo_set(llidIdx, &oamInfo);
            epon_oam_counter_inc(llidIdx, EPON_OAM_COUNTERTYPE_LOSTLINK);
            /* Set multiplexer to discard */
            if((ret = rtk_oam_multiplexerAction_set(HAL_GET_PON_PORT(),OAM_MULTIPLEXER_ACTION_DISCARD)) != RT_ERR_OK)
            {
                EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
                    "[OAM:%s:%d] OAM multiplexerAction set failed %d\n", __FILE__, __LINE__, ret);
            }
            break;
        case EPON_OAM_EVENT_KEEPALIVE_RESET:
            /* Valid OAMPDU received, reest the keepalive timer */
            EPON_TIMER_SET(skKeepalive[llidIdx], EPON_OAM_KEEPALIVE_TIME, ret)
            if(0 != ret)
            {
                EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
                    "[OAM:%s:%d] timer set failed %d\n", __FILE__, __LINE__, errno);
            }
            break;
        case EPON_OAM_EVENT_CLI:
            if(eventData.msgqData.dataSize)
            {
                epon_oam_cli_proc((oam_cli_t *) eventData.msgqData.data);
            }
            break;
        default:
            EPON_OAM_PRINT(EPON_OAM_DBGFLAG_WARN,
                "[OAM:%s:%d] Unsupported event %d\n", __FILE__, __LINE__, eventData.mtype);
            break;
        }
	}

	return NULL;
}

void *epon_oam_failRecover(void *argu)
{
    int ret;
    unsigned char isIdle, oam_discovery_state;
    rtk_enable_t state;
    unsigned short autoRegTime[EPON_OAM_SUPPORT_LLID_NUM] = { 0 };
    oam_config_t llidConf;
    struct timespec sleepTime, remainTime;

    /* Init variables */
    failoverTime = EPON_OAM_FAILOVER_GRANULARITY;
    failoverTimeSpec.tv_sec = EPON_OAM_FAILOVER_GRANULARITY / 1000;
    failoverTimeSpec.tv_nsec = (EPON_OAM_FAILOVER_GRANULARITY % 1000) * 1000 * 1000;
    backoffTime = EPON_OAM_FAILOVER_BACKOFF;
    backoffTimeSpec.tv_sec = EPON_OAM_FAILOVER_BACKOFF / 1000;
    backoffTimeSpec.tv_nsec = (EPON_OAM_FAILOVER_BACKOFF % 1000) * 1000 * 1000;

	while(1)
	{
        sleepTime.tv_sec = failoverTimeSpec.tv_sec;
        sleepTime.tv_nsec = failoverTimeSpec.tv_nsec;
FAIL_RECOVER_WAKE_UP1:
        ret = nanosleep(&sleepTime, &remainTime);
        if((ret == -1) && (EINTR == errno))
        {
            /* Interrupted by signal 
             * Uset the remain time to sleep again
             */
            sleepTime = remainTime;
            goto FAIL_RECOVER_WAKE_UP1;
        }
        else if(ret != 0)
        {
            EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
                "[OAM:%s:%d] nanosleep failed %d\n", __FILE__, __LINE__, ret);
        }

        /* Auto registration only support on primary LLID */
        epon_oam_config_get(0, &llidConf);
        epon_oam_discoveryIdleLlid_get(0, &isIdle, &oam_discovery_state);		
        if((!llidConf.autoRegEnable) ||
           (!llidConf.oamEnabled) ||
           (!isIdle))
        {
            /* Check software config first */
            continue;
        }

        /* Check if the physical fiber is connected */
        rtk_epon_losState_get(&state);
        if(DISABLED == state)
        {
            if(autoRegTime[0] < llidConf.autoRegTime)
            {
                autoRegTime[0] += failoverTime;
            }
            else
            {
                /* Time up, trigger register */
                epon_oam_triggerMpcp_reg(0);
                autoRegTime[0] = 0;

                /* Backoff before next retry */
                sleepTime.tv_sec = backoffTimeSpec.tv_sec;
                sleepTime.tv_nsec = backoffTimeSpec.tv_nsec;
                EPON_OAM_PRINT(EPON_OAM_DBGFLAG_INFO,
                    "[OAM:%s:%d] Trigger register\n", __FILE__, __LINE__);
FAIL_RECOVER_WAKE_UP2:
                ret = nanosleep(&sleepTime, &remainTime);
                if((ret == -1) && (EINTR == errno))
                {
                    /* Interrupted by signal 
                     * Uset the remain time to sleep again
                     */
                    sleepTime = remainTime;
                    goto FAIL_RECOVER_WAKE_UP2;
                }
                else if(ret != 0)
                {
                    EPON_OAM_PRINT(EPON_OAM_DBGFLAG_ERROR,
                        "[OAM:%s:%d] nanosleep failed %d\n", __FILE__, __LINE__, ret);
                }
            }
        }
	}

	return NULL;
}

int
main(
	int argc,
	char *argv[])
{
    int i;
	pthread_t th1, th2, th3;

    rtk_epon_init();

    rtk_core_init();

    /* Disable VLAN function by default */
    rtk_vlan_vlanFunctionEnable_set(DISABLED);

    /* Disable SVLAN function by default */
    rtk_svlan_svlanFunctionEnable_set(DISABLED);
    
    /* Allow OAM packet to be trapped */
    rtk_trap_oamPduAction_set(ACTION_TRAP2CPU);

    /* Init database */
    epon_oam_database_init();
    epon_oam_discovery_init();
    epon_oam_user_init();
    for(i = 0;i < EPON_OAM_SUPPORT_LLID_NUM;i++)
    {
        epon_oam_discoveryLlid_init(i);
        epon_oam_userDb_init(i);
    }

    pthread_create(&th1, NULL, &epon_oam_rxThread, NULL);
    pthread_create(&th2, NULL, &epon_oam_stateKeeper, NULL);
    pthread_create(&th3, NULL, &epon_oam_failRecover, NULL);
    while(1)
    {
        sleep(100);
    }

    return 0;
}

