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
 * $Date: 2013-07-01 15:36:16 +0800 (?±ä?, 01 ä¸ƒæ? 2013) $
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
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>

#include "epon_oam_config.h"
#include "epon_oam_err.h"
#include "epon_oam_msgq.h"
#include "epon_oam_cli.h"

/* 
 * Symbol Definition 
 */
#define EPON_OAM_CLI_BRANCH     0x01
#define EPON_OAM_CLI_LEAF       0x02
#define EPON_OAM_CLI_END        0x03

typedef struct oam_cli_tree_s {
    unsigned char cmd[16];
    unsigned char type;
    union {
        struct oam_cli_tree_s *pSubTree;
        int (*cli_proc)(int argc, char *argv[], oam_cli_t *pCli);
    } u;
} oam_cli_tree_t;


/*  
 * Data Declaration  
 */
static int msgQId;
static oam_cli_tree_t cliRootClear[] = {
    {
        "counter",
        EPON_OAM_CLI_LEAF,
        { .cli_proc = epon_oam_cli_counter_clear }
    },
    {
        "",
        EPON_OAM_CLI_END,
        { NULL }
    }
};

static oam_cli_tree_t cliRootGetCtc[] = {
    {
        "loid",
        EPON_OAM_CLI_LEAF,
        { .cli_proc = epon_oam_cli_ctcLoid_get }
    },
    {
        "",
        EPON_OAM_CLI_END,
        { NULL }
    }
};

static oam_cli_tree_t cliRootGet[] = {
    {
        "config",
        EPON_OAM_CLI_LEAF,
        { .cli_proc = epon_oam_cli_config_get }
    },
    {
        "ctc",
        EPON_OAM_CLI_BRANCH,
        { cliRootGetCtc }
    },
    {
        "counter",
        EPON_OAM_CLI_LEAF,
        { .cli_proc = epon_oam_cli_counter_get }
    },
    {
        "dbgext",
        EPON_OAM_CLI_LEAF,
        { .cli_proc = epon_oam_cli_dbgExt_get }
    },
    {
        "dbgflag",
        EPON_OAM_CLI_LEAF,
        { .cli_proc = epon_oam_cli_dbgFlag_get }
    },
    {
        "failover",
        EPON_OAM_CLI_LEAF,
        { .cli_proc = epon_oam_cli_failover_get }
    },
    {
        "oaminfo",
        EPON_OAM_CLI_LEAF,
        { .cli_proc = epon_oam_cli_oamInfo_get }
    },
    {
        "oamstate",
        EPON_OAM_CLI_LEAF,
        { .cli_proc = epon_oam_cli_oamState_get }
    },
    {
        "version",
        EPON_OAM_CLI_LEAF,
        { .cli_proc = epon_oam_cli_version_get }
    },
    {
        "",
        EPON_OAM_CLI_END,
        { NULL }
    }
};

static oam_cli_tree_t cliRootSetConfig[] = {
    {
        "autoreg",
        EPON_OAM_CLI_LEAF,
        { .cli_proc = epon_oam_cli_configAutoReg_set }
    },
    {
        "event",
        EPON_OAM_CLI_LEAF,
        { .cli_proc = epon_oam_cli_configEvent_set }
    },
    {
        "holdover",
        EPON_OAM_CLI_LEAF,
        { .cli_proc = epon_oam_cli_configHoldover_set }
    },
    {
        "mac",
        EPON_OAM_CLI_LEAF,
        { .cli_proc = epon_oam_cli_configMac_set }
    },
    {
        "oam",
        EPON_OAM_CLI_LEAF,
        { .cli_proc = epon_oam_cli_configOam_set }
    },
    {
        "",
        EPON_OAM_CLI_END,
        { NULL }
    }
};

static oam_cli_tree_t cliRootSetOamInfo[] = {
    {
        "venderoui",
        EPON_OAM_CLI_LEAF,
        { .cli_proc = epon_oam_cli_oamInfoVenderOui_set }
    },
    {
        "",
        EPON_OAM_CLI_END,
        { NULL }
    }
};

static oam_cli_tree_t cliRootSetCtc[] = {
    {
        "loid",
        EPON_OAM_CLI_LEAF,
        { .cli_proc = epon_oam_cli_ctcLoid_set }
    },
    {
        "",
        EPON_OAM_CLI_END,
        { NULL }
    }
};

static oam_cli_tree_t cliRootSet[] = {
    {
        "config",
        EPON_OAM_CLI_BRANCH,
        { cliRootSetConfig }
    },
    {
        "ctc",
        EPON_OAM_CLI_BRANCH,
        { cliRootSetCtc }
    },
    {
        "dbgext",
        EPON_OAM_CLI_LEAF,
        { .cli_proc = epon_oam_cli_dbgExt_set }
    },
    {
        "dbgflag",
        EPON_OAM_CLI_LEAF,
        { .cli_proc = epon_oam_cli_dbgFlag_set }
    },
    {
        "failover",
        EPON_OAM_CLI_LEAF,
        { .cli_proc = epon_oam_cli_failover_set }
    },
    {
        "oaminfo",
        EPON_OAM_CLI_BRANCH,
        { cliRootSetOamInfo }
    },
    {
        "",
        EPON_OAM_CLI_END,
        { NULL }
    }
};

static oam_cli_tree_t cliRootTrigger[] = {
    {
        "register",
        EPON_OAM_CLI_LEAF,
        { .cli_proc = epon_oam_cli_register_trigger }
    },
    {
        "",
        EPON_OAM_CLI_END,
        { NULL }
    }
};

static oam_cli_tree_t cliRoot[] = {
    {
        "clear",
        EPON_OAM_CLI_BRANCH,
        { cliRootClear }
    },
    {
        "get",
        EPON_OAM_CLI_BRANCH,
        { cliRootGet }
    },
    {
        "set",
        EPON_OAM_CLI_BRANCH,
        { cliRootSet }
    },
    {
        "trigger",
        EPON_OAM_CLI_BRANCH,
        { cliRootTrigger }
    },
    {
        "",
        EPON_OAM_CLI_END,
        { NULL }
    }
};


/* 
 * Macro Definition 
 */

/*  
 * Function Declaration  
 */

/* ------------------------------------------------------------------------- */
/* Internal APIs */
static int epon_oam_cli_configAutoReg_set(
    int argc,
    char *argv[],
    oam_cli_t *pCli)
{
    unsigned int llidIdx;
    
    if(argc >= 3)
    {
        pCli->cliType = EPON_OAM_CLI_CFGAUTOREG_SET;
        llidIdx = atoi(argv[1]);
        if(llidIdx >= EPON_OAM_SUPPORT_LLID_NUM)
        {
            printf("<llidIdx>: %u~%u\n", 0, EPON_OAM_SUPPORT_LLID_NUM - 1);
            return -1;
        }
        pCli->u.cliAutoReg.llidIdx = llidIdx;

        if(strcmp(argv[2], "enable") == 0)
        {
            if(argc == 4)
            {
                pCli->u.cliAutoReg.autoRegEnable = 1;
                pCli->u.cliAutoReg.autoRegTime = atoi(argv[3]);
            }
            else
            {
                printf("[enable <autoreg-interval>|disable]\n");
            }
        }
        else if(strcmp(argv[2], "disable") == 0)
        {
            if(argc == 3)
            {
                pCli->u.cliAutoReg.autoRegEnable = 0;
            }
            else
            {
                printf("[enable <autoreg-interval>|disable]\n");
            }
        }
        else
        {
            printf("%s <llidIdx> [enable <autoreg-interval>|disable]\n", argv[0]);
            return -1;
        }
        return 0;
    }
    else
    {
        printf("%s <llidIdx> [enable <autoreg-interval>|disable]\n", argv[0]);
        return -1;
    }
}

static int epon_oam_cli_configEvent_set(
    int argc,
    char *argv[],
    oam_cli_t *pCli)
{
    unsigned int llidIdx;
    
    if(argc == 4)
    {
        pCli->cliType = EPON_OAM_CLI_CFGEVENT_SET;
        llidIdx = atoi(argv[1]);
        if(llidIdx >= EPON_OAM_SUPPORT_LLID_NUM)
        {
            printf("<llidIdx>: %u~%u\n", 0, EPON_OAM_SUPPORT_LLID_NUM - 1);
            return -1;
        }
        pCli->u.cliEvent.llidIdx = llidIdx;
        pCli->u.cliEvent.eventRepCnt = atoi(argv[2]);
        pCli->u.cliEvent.eventRepIntvl = atoi(argv[3]);

        return 0;
    }
    else
    {
        printf("%s <llidIdx> <repeat-cnt> <repeat-interval>\n", argv[0]);
        return -1;
    }
}

static int epon_oam_cli_configHoldover_set(
    int argc,
    char *argv[],
    oam_cli_t *pCli)
{
    unsigned int llidIdx;
    
    if(argc == 4)
    {
        pCli->cliType = EPON_OAM_CLI_CFGHOLDOVER_SET;
        llidIdx = atoi(argv[1]);
        if(llidIdx >= EPON_OAM_SUPPORT_LLID_NUM)
        {
            printf("<llidIdx>: %u~%u\n", 0, EPON_OAM_SUPPORT_LLID_NUM - 1);
            return -1;
        }

        if(strcmp(argv[2], "enable") == 0)
        {
            pCli->u.cliHoldover.holdoverEnable = 1;
        }
        else if(strcmp(argv[2], "disable") == 0)
        {
            pCli->u.cliHoldover.holdoverEnable = 0;
        }

        pCli->u.cliHoldover.llidIdx = llidIdx;
        pCli->u.cliHoldover.holdoverTime = atoi(argv[3]);

        return 0;
    }
    else
    {
        printf("%s <llidIdx> [enable|disable] <holdover-time>\n", argv[0]);
        return -1;
    }
}

static int epon_oam_cli_configMac_set(
    int argc,
    char *argv[],
    oam_cli_t *pCli)
{
    int i;
    char *endPtr, *pch;
    unsigned int llidIdx;
    
    if(argc == 3)
    {
        pCli->cliType = EPON_OAM_CLI_CFGMAC_SET;
        llidIdx = atoi(argv[1]);
        if(llidIdx >= EPON_OAM_SUPPORT_LLID_NUM)
        {
            printf("<llidIdx>: %u~%u\n", 0, EPON_OAM_SUPPORT_LLID_NUM - 1);
            return -1;
        }
        pCli->u.cliMac.llidIdx = llidIdx;

        /* Replace ':' with ' ' */
        pch = endPtr = argv[2];
        pch = strchr(pch, ':');
        while(pch != NULL)
        {
            *pch = ' ';
            pch = strchr(pch, ':');
        }

        for(i = 0;i < 6;i++)
        {
            pCli->u.cliMac.mac[i] =  strtol(endPtr, &endPtr, 16); 
        }

        return 0;
    }
    else
    {
        printf("%s <llidIdx> <mac-addr>\n", argv[0]);
        printf("Ex: %s 3 00:11:22:33:44:55\n", argv[0]);
        return -1;
    }
}

static int epon_oam_cli_configOam_set(
    int argc,
    char *argv[],
    oam_cli_t *pCli)
{
    unsigned int llidIdx;
    
    if(argc == 3)
    {
        pCli->cliType = EPON_OAM_CLI_CFGOAM_SET;
        llidIdx = atoi(argv[1]);
        if(llidIdx >= EPON_OAM_SUPPORT_LLID_NUM)
        {
            printf("<llidIdx>: %u~%u\n", 0, EPON_OAM_SUPPORT_LLID_NUM - 1);
            return -1;
        }
        pCli->u.cliEnable.llidIdx = llidIdx;

        if(strcmp(argv[2], "enable") == 0)
        {
            pCli->u.cliEnable.enable = 1;
        }
        else if(strcmp(argv[2], "disable") == 0)
        {
            pCli->u.cliEnable.enable = 0;
        }
        else
        {
            printf("[enable | disable]\n");
            return -1;
        }

        return 0;
    }
    else
    {
        printf("%s <llidIdx> [enable | disable]\n", argv[0]);
        return -1;
    }
}

static int epon_oam_cli_config_get(
    int argc,
    char *argv[],
    oam_cli_t *pCli)
{
    unsigned int llidIdx;

    if(argc == 2)
    {
        pCli->cliType = EPON_OAM_CLI_CFG_GET;
        llidIdx = atoi(argv[1]);
        if(llidIdx >= EPON_OAM_SUPPORT_LLID_NUM)
        {
            printf("<llidIdx>: %u~%u\n", 0, EPON_OAM_SUPPORT_LLID_NUM - 1);
            return -1;
        }
        pCli->u.cliLlidIdx.llidIdx = llidIdx;

        return 0;
    }
    else
    {
        printf("%s <llidIdx>\n", argv[0]);
        return -1;
    }

    return 0;
}

static int epon_oam_cli_counter_clear(
    int argc,
    char *argv[],
    oam_cli_t *pCli)
{
    unsigned int llidIdx;

    if(argc == 2)
    {
        pCli->cliType = EPON_OAM_CLI_COUNTER_CLEAR;
        llidIdx = atoi(argv[1]);
        if(llidIdx >= EPON_OAM_SUPPORT_LLID_NUM)
        {
            printf("<llidIdx>: %u~%u\n", 0, EPON_OAM_SUPPORT_LLID_NUM - 1);
            return -1;
        }
        pCli->u.cliLlidIdx.llidIdx = llidIdx;

        return 0;
    }
    else
    {
        printf("%s <llidIdx>\n", argv[0]);
        return -1;
    }

    return 0;
}

static int epon_oam_cli_counter_get(
    int argc,
    char *argv[],
    oam_cli_t *pCli)
{
    unsigned int llidIdx;

    if(argc == 2)
    {
        pCli->cliType = EPON_OAM_CLI_COUNTER_GET;
        llidIdx = atoi(argv[1]);
        if(llidIdx >= EPON_OAM_SUPPORT_LLID_NUM)
        {
            printf("<llidIdx>: %u~%u\n", 0, EPON_OAM_SUPPORT_LLID_NUM - 1);
            return -1;
        }
        pCli->u.cliLlidIdx.llidIdx = llidIdx;

        return 0;
    }
    else
    {
        printf("%s <llidIdx>\n", argv[0]);
        return -1;
    }

    return 0;
}

static int epon_oam_cli_dbgFlag_set(
    int argc,
    char *argv[],
    oam_cli_t *pCli)
{
    if(argc == 2)
    {
        pCli->cliType = EPON_OAM_CLI_DBG_SET;
        pCli->u.cliDbg.flag = strtoul(argv[1], NULL, 16);

        return 0;
    }
    else
    {
        printf("%s <hex-value>\n", argv[0]);
        printf("EPON_OAM_DBGFLAG_NONE       (0x00000000UL)\n");
        printf("EPON_OAM_DBGFLAG_INFO       (0x00000001UL)\n");
        printf("EPON_OAM_DBGFLAG_WARN       (0x00000002UL)\n");
        printf("EPON_OAM_DBGFLAG_ERROR      (0x00000004UL)\n");
        printf("EPON_OAM_DBGFLAG_DUMP       (0x00000008UL)\n");
        printf("EPON_OAM_DBGFLAG_CTC_INFO   (0x00000010UL)\n");
        printf("EPON_OAM_DBGFLAG_CTC_WARN   (0x00000020UL)\n");
        printf("EPON_OAM_DBGFLAG_CTC_ERROR  (0x00000040UL)\n");
        printf("EPON_OAM_DBGFLAG_CTC_DUMP   (0x00000080UL)\n");
        printf("EPON_OAM_DBGFLAG_TXOAM      (0x10000000UL)\n");
        printf("EPON_OAM_DBGFLAG_RXOAM      (0x20000000UL)\n");
        printf("EPON_OAM_DBGFLAG_DEBUG      (0x80000000UL)\n");
        printf("EPON_OAM_DBGFLAG_ALL        (0xFFFFFFFFUL)\n");
        return -1;
    }
}

static int epon_oam_cli_dbgFlag_get(
    int argc,
    char *argv[],
    oam_cli_t *pCli)
{
    pCli->cliType = EPON_OAM_CLI_DBG_GET;

    return 0;
}

static int epon_oam_cli_dbgExt_set(
    int argc,
    char *argv[],
    oam_cli_t *pCli)
{
    if(argc == 2)
    {
        pCli->cliType = EPON_OAM_CLI_DBGEXT_SET;
        pCli->u.cliDbg.flag = strtol(argv[1], NULL, 16);

        return 0;
    }
    else
    {
        printf("%s <hex-value>\n", argv[0]);
        return -1;
    }
}

static int epon_oam_cli_dbgExt_get(
    int argc,
    char *argv[],
    oam_cli_t *pCli)
{
    pCli->cliType = EPON_OAM_CLI_DBGEXT_GET;

    return 0;
}

static int epon_oam_cli_failover_set(
    int argc,
    char *argv[],
    oam_cli_t *pCli)
{
    int value;
    if(argc == 3)
    {
        pCli->cliType = EPON_OAM_CLI_FAILOVER_SET;
        value = atoi(argv[1]);
        if(value < 0)
        {
            return -1;
        }
        pCli->u.cliFailover.granularity = value;

        value = atoi(argv[2]);
        if(value < 0)
        {
            return -1;
        }
        pCli->u.cliFailover.backoff = value;

        return 0;
    }
    else
    {
        printf("%s <granularity-ms> <backoff-ms>\n", argv[0]);
        return -1;
    }
}

static int epon_oam_cli_failover_get(
    int argc,
    char *argv[],
    oam_cli_t *pCli)
{
    pCli->cliType = EPON_OAM_CLI_FAILOVER_GET;

    return 0;
}

static int epon_oam_cli_oamInfoVenderOui_set(
    int argc,
    char *argv[],
    oam_cli_t *pCli)
{
    int i;
    char *endPtr, *pch;
    unsigned int llidIdx;
    
    if(argc == 3)
    {
        pCli->cliType = EPON_OAM_CLI_OAMINFOOUI_SET;
        llidIdx = atoi(argv[1]);
        if(llidIdx >= EPON_OAM_SUPPORT_LLID_NUM)
        {
            printf("<llidIdx>: %u~%u\n", 0, EPON_OAM_SUPPORT_LLID_NUM - 1);
            return -1;
        }
        pCli->u.cliMac.llidIdx = llidIdx;

        /* Replace ':' with ' ' */
        pch = endPtr = argv[2];
        pch = strchr(pch, ':');
        while(pch != NULL)
        {
            *pch = ' ';
            pch = strchr(pch, ':');
        }

        for(i = 0;i < 3;i++)
        {
            pCli->u.cliMac.mac[i] =  strtol(endPtr, &endPtr, 16); 
        }

        return 0;
    }
    else
    {
        printf("%s <llidIdx> <vender-oui>\n", argv[0]);
        printf("Ex: %s 3 00:11:22\n", argv[0]);
        return -1;
    }
}

static int epon_oam_cli_oamInfo_get(
    int argc,
    char *argv[],
    oam_cli_t *pCli)
{
    unsigned int llidIdx;

    if(argc == 2)
    {
        pCli->cliType = EPON_OAM_CLI_OAMINFO_GET;
        llidIdx = atoi(argv[1]);
        if(llidIdx >= EPON_OAM_SUPPORT_LLID_NUM)
        {
            printf("<llidIdx>: %u~%u\n", 0, EPON_OAM_SUPPORT_LLID_NUM - 1);
            return -1;
        }
        pCli->u.cliLlidIdx.llidIdx = llidIdx;

        return 0;
    }
    else
    {
        printf("%s <llidIdx>\n", argv[0]);
        return -1;
    }

    return 0;
}
    
static int epon_oam_cli_oamState_get(
    int argc,
    char *argv[],
    oam_cli_t *pCli)
{
    unsigned int llidIdx;

    if(argc == 2)
    {
        pCli->cliType = EPON_OAM_CLI_OAMSTATE_GET;
        llidIdx = atoi(argv[1]);
        if(llidIdx >= EPON_OAM_SUPPORT_LLID_NUM)
        {
            printf("<llidIdx>: %u~%u\n", 0, EPON_OAM_SUPPORT_LLID_NUM - 1);
            return -1;
        }
        pCli->u.cliLlidIdx.llidIdx = llidIdx;

        return 0;
    }
    else
    {
        printf("%s <llidIdx>\n", argv[0]);
        return -1;
    }
}

static int epon_oam_cli_version_get(
    int argc,
    char *argv[],
    oam_cli_t *pCli)
{
    unsigned int llidIdx;

#ifdef EPON_OAM_VERSION
    printf("EPON OAM Version: %s\n", EPON_OAM_VERSION);
#else
    printf("EPON OAM Version: Unknown version\n");
#endif
    printf("Build Time: %s %s\n", __DATE__, __TIME__);
	return -1;
}

static int epon_oam_cli_register_trigger(
    int argc,
    char *argv[],
    oam_cli_t *pCli)
{
    unsigned int llidIdx;

    if(argc == 2)
    {
        pCli->cliType = EPON_OAM_CLI_REG_TRIGGER;
        llidIdx = atoi(argv[1]);
        if(llidIdx >= EPON_OAM_SUPPORT_LLID_NUM)
        {
            printf("<llidIdx>: %u~%u\n", 0, EPON_OAM_SUPPORT_LLID_NUM - 1);
            return -1;
        }
        pCli->u.cliLlidIdx.llidIdx = llidIdx;
    }
    else
    {
        printf("%s <llidIdx>\n", argv[0]);
        return -1;
    }

    return 0;
}

/* ------------------------------------------------------------------------- */
/* CTC APIs */
static int epon_oam_cli_ctcLoid_get(
    int argc,
    char *argv[],
    oam_cli_t *pCli)
{
    unsigned int llidIdx;

    if(argc == 2)
    {
        memset(pCli, 0, sizeof(oam_cli_t));
        pCli->cliType = CTC_OAM_CLI_LOID_GET;
        llidIdx = atoi(argv[1]);
        if(llidIdx >= EPON_OAM_SUPPORT_LLID_NUM)
        {
            printf("<llidIdx>: %u~%u\n", 0, EPON_OAM_SUPPORT_LLID_NUM - 1);
            return -1;
        }
        pCli->u.cliCtcLoid.llidIdx = llidIdx;
    }
    else
    {
        printf("%s <llidIdx>\n", argv[0]);
        return -1;
    }

    return 0;
}

static int epon_oam_cli_ctcLoid_set(
    int argc,
    char *argv[],
    oam_cli_t *pCli)
{
    unsigned int llidIdx;

    if(argc == 3)
    {
        memset(pCli, 0, sizeof(oam_cli_t));
        pCli->cliType = CTC_OAM_CLI_LOID_SET;
        llidIdx = atoi(argv[1]);
        if(llidIdx >= EPON_OAM_SUPPORT_LLID_NUM)
        {
            printf("<llidIdx>: %u~%u\n", 0, EPON_OAM_SUPPORT_LLID_NUM - 1);
            return -1;
        }
        pCli->u.cliCtcLoid.llidIdx = llidIdx;
        strcpy(pCli->u.cliCtcLoid.loid, argv[2]);
    }
    else if(argc == 4)
    {
        memset(pCli, 0, sizeof(oam_cli_t));
        pCli->cliType = CTC_OAM_CLI_LOID_SET;
        llidIdx = atoi(argv[1]);
        if(llidIdx >= EPON_OAM_SUPPORT_LLID_NUM)
        {
            printf("<llidIdx>: %u~%u\n", 0, EPON_OAM_SUPPORT_LLID_NUM - 1);
            return -1;
        }
        pCli->u.cliCtcLoid.llidIdx = llidIdx;
        strcpy(pCli->u.cliCtcLoid.loid, argv[2]);
        strcpy(pCli->u.cliCtcLoid.password, argv[3]);
    }
    else
    {
        printf("%s <llidIdx> <loid> {<password>}\n", argv[0]);
        return -1;
    }

    return 0;
}

int epon_oam_event_send(
    unsigned char llidIdx,
    unsigned int eventId)
{
    int ret;
    oam_msgqEventData_t event;

    event.mtype = eventId;
    event.msgqData.llidIdx = llidIdx;
    event.msgqData.dataSize = 0;
    ret = msgsnd(msgQId, (void *)&event, 0, IPC_NOWAIT);
    if(-1 == ret)
    {
        printf("[OAM:%s:%d] msgsnd failed %d\n", __FILE__, __LINE__, errno);
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
        printf("[OAM:%s:%d] msgsnd failed %d\n", __FILE__, __LINE__, errno);
        return EPON_OAM_ERR_MSGQ;
    }

    return EPON_OAM_ERR_OK;
}

int epon_oam_cli_parse(
	int argc,
	char *argv[],
	oam_cli_tree_t *pCliTree,
	oam_cli_t *pCli)
{
    int inputLen, cmdLen;

    if(0 == argc)
    {
        /* No any argument for parsing */
        printf("available cmds:\n");
        while(pCliTree->type != EPON_OAM_CLI_END)
        {
            printf("%s\n", pCliTree->cmd);
            pCliTree += 1;
        }

        return -1;
    }

    if(NULL == pCliTree)
    {
        printf("incorrect command tree\n");
        return -2;
    }

    inputLen = strlen(argv[0]);
    while(pCliTree->type != EPON_OAM_CLI_END)
    {
        cmdLen = strlen(pCliTree->cmd);
        if(strncmp(argv[0], pCliTree->cmd, inputLen > cmdLen ? cmdLen : inputLen) == 0)
        {
            /* Search subtree or execute the command */
            if(pCliTree->type == EPON_OAM_CLI_BRANCH)
            {
                return epon_oam_cli_parse(argc - 1, &argv[1], pCliTree->u.pSubTree, pCli);
            }
            else if(pCliTree->type == EPON_OAM_CLI_LEAF)
            {
                if(NULL != pCliTree->u.cli_proc)
                {
                    return pCliTree->u.cli_proc(argc, argv, pCli);
                }
                else
                {
                    printf("incorrect command callback\n");
                    return -3;
                }
            }
            else
            {
                printf("incorrect command type\n");
                return -3;
            }
        }
        pCliTree = pCliTree + 1;
    }

    printf("incorrect command\n");
    return -4;
}

int
main(
	int argc,
	char *argv[])
{
    int ret;
    int permits;
    key_t msgQKey = 1568;
    oam_cli_t cli;

    /* Get message queue to send event to state keeper */
    /* S_IRUSR | S_IWUSR | State keeper can read/write message
     * S_IRGRP | S_IWGRP | All others can read/write message
     * S_IROTH | S_IWOTH   All others can read/write message
     */
    permits = 00666;
    permits |= IPC_CREAT;
    msgQId = msgget(msgQKey, permits);
    if(-1 == msgQId)
    {
        printf("[OAM:%s:%d] msgq create failed %d\n", __FILE__, __LINE__, errno);
        return -1;
    }

    memset(&cli, 0x0, sizeof(cli));
    ret = epon_oam_cli_parse(argc - 1, &argv[1], cliRoot, &cli);

    if(0 == ret)
    {
        epon_oam_eventData_send(0,
            EPON_OAM_EVENT_CLI,
            (unsigned char *)&cli,
            sizeof(cli));
    }

    return 0;
}

