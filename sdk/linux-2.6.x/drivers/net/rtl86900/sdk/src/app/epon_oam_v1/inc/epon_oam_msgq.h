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
 * $Revision: 40647 $
 * $Date: 2013-07-01 15:36:16 +0800 (週一, 01 七月 2013) $
 *
 * Purpose : Define the EPON OAM protcol stack's message queue structures
 *
 * Feature : 
 *
 */

#ifndef __EPON_OAM_MSGQ_H__
#define __EPON_OAM_MSGQ_H__

/* 
 * Symbol Definition 
 */
/* Maximum size of the event data length */
#define EPON_OAM_EVENTDATA_MAX              128
/* EPON OAM events */
#define EPON_OAM_EVENT_DISCOVERY_COMPLETE   0x0001
#define EPON_OAM_EVENT_KEEPALIVE_TIMEOUT    0x0002
#define EPON_OAM_EVENT_KEEPALIVE_RESET      0x0003
#define EPON_OAM_EVENT_LOS                  0x0004
#define EPON_OAM_EVENT_CLI                  0x0005

/* CLI types */
typedef enum oam_cliType_e {
    EPON_OAM_CLI_DBG_SET = 1,
    EPON_OAM_CLI_DBG_GET,
    EPON_OAM_CLI_CFGOAM_SET,
    EPON_OAM_CLI_CFGMAC_SET,
    EPON_OAM_CLI_CFGAUTOREG_SET,
    EPON_OAM_CLI_CFGHOLDOVER_SET,
    EPON_OAM_CLI_CFGEVENT_SET,
    EPON_OAM_CLI_CFG_GET,
    EPON_OAM_CLI_COUNTER_GET,
    EPON_OAM_CLI_COUNTER_CLEAR,
    EPON_OAM_CLI_DBGEXT_SET,
    EPON_OAM_CLI_DBGEXT_GET,
    EPON_OAM_CLI_FAILOVER_SET,
    EPON_OAM_CLI_FAILOVER_GET,
    EPON_OAM_CLI_OAMINFOOUI_SET,
    EPON_OAM_CLI_OAMINFO_GET,
    EPON_OAM_CLI_OAMSTATE_GET,
    EPON_OAM_CLI_REG_TRIGGER,
    CTC_OAM_CLI_LOID_SET,
    CTC_OAM_CLI_LOID_GET,
    EPON_OAM_CLI_END,
} oam_cliType_t;

typedef struct oam_msgqData_s {
    unsigned char llidIdx;
    unsigned short dataSize;
    unsigned char data[EPON_OAM_EVENTDATA_MAX];
} oam_msgqData_t;

typedef struct oam_msgqEventData_s {
    long mtype;
    oam_msgqData_t msgqData; 
} oam_msgqEventData_t;

/* Structure for CLI usage */
typedef struct oam_cliDbg_s {
    unsigned int flag;
} oam_cliDbg_t;

typedef struct oam_cliLlidIdx_s {
    unsigned char llidIdx;
} oam_cliLlidIdx_t;

typedef struct oam_cliEnable_s {
    unsigned char llidIdx;
    unsigned char enable;
} oam_cliEnable_t;

typedef struct oam_cliMac_s {
    unsigned char llidIdx;
    unsigned char mac[6];
} oam_cliMac_t;

typedef struct oam_cliFailover_s {
    unsigned short granularity;
    unsigned short backoff;
} oam_cliFailover_t;

typedef struct oam_cliAutoReg_s {
    unsigned char llidIdx;
    unsigned char autoRegEnable;
    unsigned short autoRegTime;
} oam_cliAutoReg_t;

typedef struct oam_cliHoldover_s {
    unsigned char llidIdx;
    unsigned char holdoverEnable;
    unsigned short holdoverTime;
} oam_cliHoldover_t;

typedef struct oam_cliEvent_s {
    unsigned char llidIdx;
    unsigned short eventRepCnt;
    unsigned short eventRepIntvl;
} oam_cliEvent_t;

typedef struct oam_cliCtcLoid_s {
    unsigned char llidIdx;
    unsigned char loid[24/* CTC_ORGSPEC_ONUAUTH_LOID_LEN */];
    unsigned char password[12/* CTC_ORGSPEC_ONUAUTH_PASSWORD_LEN */];
} oam_cliCtcLoid_t;

typedef struct oam_cli_s {
    oam_cliType_t cliType;
    union {
        oam_cliLlidIdx_t cliLlidIdx;
        oam_cliEnable_t cliEnable;
        oam_cliDbg_t cliDbg;
        oam_cliMac_t cliMac;
        oam_cliFailover_t cliFailover;
        oam_cliAutoReg_t cliAutoReg;
        oam_cliHoldover_t cliHoldover;
        oam_cliEvent_t cliEvent;
        oam_cliCtcLoid_t cliCtcLoid;
    } u;
} oam_cli_t;


/*
 * Function Declaration
 */
extern int epon_oam_event_send(
    unsigned char llidIdx,
    unsigned int eventId);
extern int epon_oam_eventData_send(
    unsigned char llidIdx,
    unsigned int eventId,
    unsigned char *data,
    unsigned short dataLen);

#endif /* __EPON_OAM_MSGQ_H__ */

